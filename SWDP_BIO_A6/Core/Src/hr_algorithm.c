/**
 * @file   hr_algorithm.c
 * @brief  Heart-rate estimation from PPG signal using CMSIS-DSP.
 *         Optimized for PPG_CIRC_SIZE = 256 and PPG_ACQ_SIZE = 128.
 */

#include "hr_algorithm.h"
#include "arm_math.h"
#include <math.h>
#include <stdint.h>
#include <string.h>

//TODO: tempo
#include "bluetooth.h"

/* ── Dimensioni del framework (Override o allineamento alle macro globali) ── */
#define PPG_CIRC_SIZE_LOCAL 256u
#define PPG_ACQ_SIZE_LOCAL  128u
#define PPG_FS_LOCAL        100.0f

#define BPF_NUM_STAGES  1u

/*
 * Filtro Butterworth Passa-Banda 2° ordine (0.5–4 Hz @ Fs = 100 Hz).
 * Convenzione CMSIS-DSP per il segno dei coefficienti ricorsivi:
 * y[n] = b0*x[n] + b1*x[n-1] + b2*x[n-2] + a1*y[n-1] + a2*y[n-2]
 */
static const float32_t bpf_coeffs[BPF_NUM_STAGES * 5u] = {
     0.09955f,   /* b0 */
     0.00000f,   /* b1 */
    -0.09955f,   /* b2 */
     1.79375f,   /* a1 : Segno invertito rispetto alla H(z) accademica */
    -0.80089f    /* a2 : Segno invertito rispetto alla H(z) accademica */
};

/* Stato del filtro: 4 float per ogni stage biquad */
static float32_t bpf_state[BPF_NUM_STAGES * 4u];
static arm_biquad_casd_df1_inst_f32 bpf_inst;

/* Buffer circolare simulato con spostamento lineare (256 campioni) */
static float32_t filtered_buf[PPG_CIRC_SIZE_LOCAL];

/* Stato della Peak Detection */
static uint32_t  total_sample_cnt;
static uint32_t  last_peak_abs_idx;
static uint8_t   peak_found_once;
static float32_t current_bpm;

/* Limiti fisiologici a Fs = 100 Hz */
#define PEAK_MIN_SAMPLES  30u   /* Corrisponde a circa 200 BPM */
#define PEAK_MAX_SAMPLES 200u   /* Corrisponde a circa 30 BPM  */

/* ── API Pubbliche ───────────────────────────────────────────────────────── */

void HR_Algorithm_Init(void)
{
    arm_biquad_cascade_df1_init_f32(&bpf_inst, BPF_NUM_STAGES, bpf_coeffs, bpf_state);
    memset(filtered_buf, 0, sizeof(filtered_buf));
    total_sample_cnt  = 0u;
    last_peak_abs_idx = 0u;
    peak_found_once   = 0u;
    current_bpm       = 0.0f;
}

float32_t HR_GetBPM(void)
{
    return current_bpm;
}

/**
 * @brief  Elabora una nuova finestra di campioni PPG e aggiorna il calcolo del BPM.
 * @param  circ_buf Puntatore al buffer raw da 256 elementi.
 *                  I primi 128 sono lo storico precedente, gli ultimi 128 sono i nuovi dati.
 */
float32_t HR_Process(const uint32_t *circ_buf)
{
    static float32_t input[PPG_ACQ_SIZE_LOCAL];

    /* 1. Cast dei soli 128 nuovi campioni acquisiti da DMA/Periferica */
    for (uint32_t i = 0u; i < PPG_ACQ_SIZE_LOCAL; i++) {
        input[i] = (float32_t)circ_buf[PPG_ACQ_SIZE_LOCAL + i];
    }

    /* 2. Shift a sinistra dello storico filtrato (libera gli ultimi 128 slot) */
    memcpy(filtered_buf,
           filtered_buf + PPG_ACQ_SIZE_LOCAL,
           PPG_ACQ_SIZE_LOCAL * sizeof(float32_t));

    /* 3. Filtraggio in place dei nuovi 128 campioni nella seconda metà del buffer */
    arm_biquad_cascade_df1_f32(&bpf_inst,
                               input,
                               filtered_buf + PPG_ACQ_SIZE_LOCAL,
                               PPG_ACQ_SIZE_LOCAL);

    // //TODO: temporary
    // uint32_t to_send[PPG_ACQ_SIZE_LOCAL];
    // for (uint32_t i = PPG_ACQ_SIZE_LOCAL; i < PPG_CIRC_SIZE_LOCAL; i++) {
    //     to_send[i - PPG_ACQ_SIZE_LOCAL] = (uint32_t)filtered_buf[i];
    // }
    // BLE_SendPacket_PPG(to_send, PPG_ACQ_SIZE_LOCAL);

    /* 4. Soglia adattativa basata sulla deviazione standard del segnale filtrato.
     *    La soglia è 1.5× la deviazione standard — i picchi di rumore raramente
     *    superano 1.5σ, mentre i picchi PPG sono tipicamente > 3σ.
     *    Lo squelch assoluto (σ < 500) scarta il rumore quando il dito non è
     *    appoggiato sul sensore.
     */
    float32_t mean = 0.0f;
    for (uint32_t i = 0u; i < PPG_CIRC_SIZE_LOCAL; i++) {
        mean += filtered_buf[i];
    }
    mean /= (float32_t)PPG_CIRC_SIZE_LOCAL;

    float32_t variance = 0.0f;
    for (uint32_t i = 0u; i < PPG_CIRC_SIZE_LOCAL; i++) {
        float32_t diff = filtered_buf[i] - mean;
        variance += diff * diff;
    }
    variance /= (float32_t)PPG_CIRC_SIZE_LOCAL;

    float32_t std_dev = sqrtf(variance);

    /* Soglia = 1.5× la deviazione standard del segnale filtrato */
    float32_t threshold = std_dev * 1.5f;

    /* Squelch assoluto: se σ è troppo bassa, non c'è dito sul sensore */
    if (std_dev < 500.0f) {
        total_sample_cnt += PPG_ACQ_SIZE_LOCAL;
        current_bpm = 0.0f;   /* Invalida la lettura precedente */
        return current_bpm;
    }
    /* 
     * 5. Peak Detection con scansione dell'area di Overlap.
     * Analizziamo partendo dal confine di giunzione (indice 127) per intercettare i picchi 
     * a cavallo tra la vecchia finestra e la nuova, estendendoci fino alla fine del buffer (indice 254).
     */
    for (uint32_t i = (PPG_ACQ_SIZE_LOCAL - 1u); i < (PPG_CIRC_SIZE_LOCAL - 1u); i++) {
        float32_t s = filtered_buf[i];

        /* Condizione di massimo locale e superamento della soglia dinamica */
        if (s > threshold &&
            s > filtered_buf[i - 1u] &&
            s > filtered_buf[i + 1u])
        {
            /* Calcolo della coordinata temporale assoluta del picco */
            uint32_t peak_abs = (total_sample_cnt - PPG_ACQ_SIZE_LOCAL) + i;

            if (peak_found_once) {
                uint32_t interval = peak_abs - last_peak_abs_idx;

                /* Verifica della congruenza fisiologica dell'intervallo R-R */
                if (interval >= PEAK_MIN_SAMPLES && interval <= PEAK_MAX_SAMPLES) {
                    float32_t instant_bpm = (60.0f * PPG_FS_LOCAL) / (float32_t)interval;
                    
                    /* IIR a singolo polo per stabilizzare la visualizzazione su App mobile */
                    if (current_bpm == 0.0f) {
                        current_bpm = instant_bpm;
                    } else {
                        current_bpm = (0.5f * current_bpm) + (0.5f * instant_bpm);
                    }
                    last_peak_abs_idx = peak_abs;
                } 
                else if (interval > PEAK_MAX_SAMPLES) {
                    /* Timeout: il sensore potrebbe essere stato rimosso. Reset della baseline temporale */
                    last_peak_abs_idx = peak_abs;
                }
            } else {
                last_peak_abs_idx = peak_abs;
                peak_found_once   = 1u;
            }
        }
    }

    /* Avanzamento del counter globale dei campioni integrati */
    total_sample_cnt += PPG_ACQ_SIZE_LOCAL;
    return current_bpm;
}