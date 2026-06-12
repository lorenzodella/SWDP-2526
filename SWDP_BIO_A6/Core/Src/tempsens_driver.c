/**
 * \file tempsens_driver.c
 * \brief MAX30101 internal die temperature driver.
 *
 * TIM2 interrupt fires at its configured period; TempSens_TimerCallback()
 * checks whether 30 s have elapsed (via HAL_GetTick) and sets a flag.
 * TempSens_Process(), called from the main loop, performs the I2C transfer
 * when the flag is set: it triggers a one-shot conversion (TEMP_EN), waits
 * the 29 ms acquisition time, then reads TINT + TFRAC.
 */

#include "tempsens_driver.h"
#include "ppg_driver.h"   /* MAX30101_I2C_ADDRESS, MAX30101_I2C_TIMEOUT */

extern I2C_HandleTypeDef hi2c3;
extern TIM_HandleTypeDef htim2;

/* MAX30101 temperature register addresses (datasheet Table, pp. 21-22) */
#define REG_INT_STATUS2   0x01   /* bit 1 = DIE_TEMP_RDY                 */
#define REG_TEMP_INTEGER  0x1F   /* TINT[7:0] — two's-complement, 1 °C/bit */
#define REG_TEMP_FRACTION 0x20   /* TFRAC[3:0] — 0.0625 °C/bit           */
#define REG_TEMP_CONFIG   0x21   /* bit 0 = TEMP_EN (self-clearing)       */

#define TEMP_EN_BIT       0x01u
#define TEMP_RDY_BIT      0x02u  /* DIE_TEMP_RDY in INT_STATUS_2          */
#define TEMP_INTERVAL_MS  30000u /* 30-second sample period               */
#define TEMP_CONV_MS      30u    /* ≥ 29 ms acquisition time (datasheet)  */

static volatile uint8_t temp_pending = 0;
static float32_t      last_temp    = 0.0f;
static uint32_t         last_tick    = 0;

/* ------------------------------------------------------------------ */
/* Internal helpers                                                     */
/* ------------------------------------------------------------------ */

static uint8_t write_reg(uint8_t reg, uint8_t val)
{
    uint8_t buf[2] = {reg, val};
    return HAL_I2C_Master_Transmit(&hi2c3,
                                   MAX30101_I2C_ADDRESS << 1,
                                   buf, sizeof(buf),
                                   MAX30101_I2C_TIMEOUT) == HAL_OK;
}

static uint8_t read_reg(uint8_t reg, uint8_t *out)
{
    return HAL_I2C_Mem_Read(&hi2c3,
                            MAX30101_I2C_ADDRESS << 1,
                            reg, I2C_MEMADD_SIZE_8BIT,
                            out, 1,
                            MAX30101_I2C_TIMEOUT) == HAL_OK;
}

/* ------------------------------------------------------------------ */
/* Public API                                                           */
/* ------------------------------------------------------------------ */

void TempSens_Init(void)
{
    last_tick = HAL_GetTick();

    /* TIM2 IRQ is routed in stm32u5xx_it.c but NVIC was never armed. */
    HAL_NVIC_SetPriority(TIM2_IRQn, 3, 0);
    HAL_NVIC_EnableIRQ(TIM2_IRQn);

    HAL_TIM_Base_Start_IT(&htim2);
}

/**
 * Call from HAL_TIM_PeriodElapsedCallback in main.c.
 * Sets temp_pending after every 30-second window; no I2C work here.
 */
void TempSens_TimerCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance != TIM2) return;

    uint32_t now = HAL_GetTick();
    if ((now - last_tick) >= TEMP_INTERVAL_MS) {
        last_tick    = now;
        temp_pending = 1;
    }
}

/**
 * Call from the main loop.
 * Triggers a one-shot conversion, waits for the ADC, reads TINT + TFRAC.
 * Blocks for ~30 ms when a reading is due; returns immediately otherwise.
 */
void TempSens_Process(void)
{
    if (!temp_pending) return;
    temp_pending = 0;

    /* Trigger a single conversion — TEMP_EN self-clears when done. */
    if (!write_reg(REG_TEMP_CONFIG, TEMP_EN_BIT)) return;

    /* Wait for acquisition (datasheet: T_T = 29 ms typ at 25 °C). */
    HAL_Delay(TEMP_CONV_MS);

    /* Read integer part (two's complement, 1 °C resolution). */
    uint8_t t_int = 0;
    if (!read_reg(REG_TEMP_INTEGER, &t_int)) return;

    /* Read fractional part (0.0625 °C per LSB, always positive).
     * Reading TFRAC also clears the DIE_TEMP_RDY interrupt flag. */
    uint8_t t_frac = 0;
    if (!read_reg(REG_TEMP_FRACTION, &t_frac)) return;

    last_temp = (float32_t)(int8_t)t_int + (float32_t)(t_frac & 0x0Fu) * 0.0625f;
}

float32_t TempSens_GetLast(void)
{
    return last_temp;
}
