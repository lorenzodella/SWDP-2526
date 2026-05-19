/**
 * @file  hr_algorithm.c
 * @brief Heart-rate estimation from PPG signal using CMSIS-DSP.
 *
 * Pipeline
 * --------
 * 1. Each call to HR_Process() receives a 64-sample circular buffer whose
 *    second half (samples [32..63]) holds the newest PPG_ACQ_SIZE samples and
 *    whose first half holds the previous batch.
 * 2. The new samples are converted to float and passed through a persistent
 *    2nd-order Butterworth bandpass IIR filter (0.5 – 4 Hz @ 100 Hz).
 * 3. A 64-sample filtered history is maintained with the same 50 % overlap
 *    structure, so peak detection has context from the previous batch.
 * 4. Local maxima above an adaptive threshold are identified and the
 *    inter-peak interval is used to update the BPM estimate.
 *
 * Filter design (bilinear transform, pre-warped cutoffs)
 * ------------------------------------------------------
 *   Fs   = 100 Hz
 *   fl   = 0.5 Hz,  fh = 4 Hz
 *   Ω1   = 2·Fs·tan(π·fl/Fs) = 3.1424 rad/s
 *   Ω2   = 2·Fs·tan(π·fh/Fs) = 25.297 rad/s
 *   Ω0   = √(Ω1·Ω2)          = 8.9158 rad/s
 *   BW   = Ω2 − Ω1            = 22.154 rad/s
 *
 *   H(z) = 0.09955·(1 − z⁻²)
 *          ────────────────────────────────────────
 *          1 − 1.79375·z⁻¹ + 0.80089·z⁻²
 *
 * CMSIS-DSP DF1 difference equation (sign convention):
 *   y[n] = b0·x[n] + b1·x[n-1] + b2·x[n-2] + a1·y[n-1] + a2·y[n-2]
 *   {b0, b1, b2, a1, a2} = {0.09955, 0, −0.09955, 1.79375, −0.80089}
 *
 * Stability check: poles at z = 0.9560 and z = 0.8378 (both |z| < 1). ✓
 */

#include "hr_algorithm.h"
#include <string.h>

/* ── Filter instance ──────────────────────────────────────────────────────── */

#define BPF_NUM_STAGES  1u

/*
 * 2nd-order Butterworth bandpass 0.5–4 Hz at Fs = 100 Hz.
 * CMSIS-DSP coefficient order per stage: {b0, b1, b2, a1, a2}.
 */
static const float32_t bpf_coeffs[BPF_NUM_STAGES * 5u] = {
     0.09955f,   /* b0 */
     0.00000f,   /* b1 */
    -0.09955f,   /* b2 */
     1.79375f,   /* a1 – positive feedback */
    -0.80089f    /* a2 – negative feedback */
};

/* 4 state variables per biquad stage: {x[n-1], x[n-2], y[n-1], y[n-2]} */
static float32_t bpf_state[BPF_NUM_STAGES * 4u];
static arm_biquad_casd_df1_inst_f32 bpf_inst;

/* ── Filtered output history (same overlap structure as circ_buf) ─────────── */
static float32_t filtered_buf[PPG_CIRC_SIZE];

/* ── Peak-tracking state ──────────────────────────────────────────────────── */

/*
 * Absolute sample counter incremented by PPG_ACQ_SIZE every call.
 * Used to compute inter-peak interval in samples regardless of window
 * boundaries.
 */
static uint32_t  total_sample_cnt;
static uint32_t  last_peak_abs_idx;
static uint8_t   peak_found_once;
static float32_t current_bpm;

/* ── Minimum refractory period between peaks ──────────────────────────────── */

/* 200 BPM → 30 samples; 30 BPM → 200 samples */
#define PEAK_MIN_SAMPLES  30u
#define PEAK_MAX_SAMPLES 200u

/* ── Public API ───────────────────────────────────────────────────────────── */

void HR_Algorithm_Init(void)
{
    arm_biquad_cascade_df1_init_f32(&bpf_inst, BPF_NUM_STAGES,
                                     bpf_coeffs, bpf_state);
    memset(filtered_buf,  0, sizeof(filtered_buf));
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
 * @brief  Process one window of PPG data and update the BPM estimate.
 *
 * @param  circ_buf  Pointer to the 64-sample circular buffer.
 *                   circ_buf[0..31]  = previous batch (context)
 *                   circ_buf[32..63] = new batch (PPG_ACQ_SIZE fresh samples)
 * @return Latest BPM estimate (0 if no peak found yet).
 */
float32_t HR_Process(const uint32_t *circ_buf)
{
    /* ── 1. Convert new raw samples to float ─────────────────────────────── */
    float32_t input[PPG_ACQ_SIZE];
    for (uint32_t i = 0u; i < PPG_ACQ_SIZE; i++) {
        input[i] = (float32_t)circ_buf[PPG_ACQ_SIZE + i];
    }

    /* ── 2. Shift filtered history left, discard oldest PPG_ACQ_SIZE values ─ */
    memcpy(filtered_buf,
           filtered_buf + PPG_ACQ_SIZE,
           PPG_ACQ_SIZE * sizeof(float32_t));

    /* ── 3. Bandpass-filter the new samples (filter state is persistent) ──── */
    arm_biquad_cascade_df1_f32(&bpf_inst,
                                input,
                                filtered_buf + PPG_ACQ_SIZE,
                                PPG_ACQ_SIZE);

    /* ── 4. Adaptive threshold: peak fraction of max absolute amplitude ───── */
    float32_t max_amp = 0.0f;
    const float32_t *new_seg = filtered_buf + PPG_ACQ_SIZE;
    for (uint32_t i = 0u; i < PPG_ACQ_SIZE; i++) {
        float32_t av = (new_seg[i] >= 0.0f) ? new_seg[i] : -new_seg[i];
        if (av > max_amp) {
            max_amp = av;
        }
    }
    /* Require at least a 1 % full-scale amplitude to detect peaks – avoids
     * false triggers on a flat (noise-only) signal.  18-bit ADC → FS = 262143.
     * 0.01 × 262143 ≈ 2621 counts after filtering. */
    float32_t threshold = max_amp * 0.40f;
    /*if (threshold < 2621.0f) {
        total_sample_cnt += PPG_ACQ_SIZE;
        return current_bpm;  /* signal too weak – skip detection */
    //} 

    /* ── 5. Local-maxima peak detection on the new segment ──────────────── */
    /*
     * We skip the first and last sample of the new segment to allow a
     * symmetric 3-point comparison.  Interior indices: 1 … PPG_ACQ_SIZE-2.
     */
    for (uint32_t i = 1u; i < (PPG_ACQ_SIZE - 1u); i++) {
        float32_t s = new_seg[i];

        if (s > threshold &&
            s > new_seg[i - 1u] &&
            s > new_seg[i + 1u])
        {
            /* Absolute index of this peak within the infinite sample stream */
            uint32_t peak_abs = total_sample_cnt + i;

            if (peak_found_once) {
                uint32_t interval = peak_abs - last_peak_abs_idx;

                if (interval >= PEAK_MIN_SAMPLES &&
                    interval <= PEAK_MAX_SAMPLES)
                {
                    current_bpm = 60.0f * PPG_FS / (float32_t)interval;
                    last_peak_abs_idx = peak_abs;
                }
                /* If interval is out of range, ignore this candidate peak
                 * (likely a noise spike or a false local maximum). */
            } else {
                last_peak_abs_idx = peak_abs;
                peak_found_once   = 1u;
            }
        }
    }

    total_sample_cnt += PPG_ACQ_SIZE;
    return current_bpm;
}
