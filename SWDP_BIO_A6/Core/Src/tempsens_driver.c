#include "tempsens_driver.h"
#include "arm_math.h"
#include "stm32u5xx_hal.h"
#include <math.h>
#include <stdint.h>

extern I2C_HandleTypeDef hi2c3;


static uint8_t temp_write_register(uint8_t reg_addr, uint8_t reg_data)
{
    uint8_t tx_buffer[] = { reg_addr, reg_data };

    if (HAL_I2C_Master_Transmit(&hi2c3, (MAX30205_I2C_ADDRESS << 1), tx_buffer, sizeof(tx_buffer), MAX30205_I2C_TIMEOUT) != HAL_OK) {
        return 0; 
    }

    return 1; 
}


/**
 * @brief Configura il sensore in modalità Shutdown durante il boot.
 */
uint8_t Temp_Init(void)
{
    // Scrive 0x01 nel registro di configurazione (Attiva il bit D0 di Shutdown)
    return temp_write_register(MAX30205_REG_CONF, 0x01);
}



/**
 * @brief Legge i 2 byte di temperatura e calcola il valore float in °C.
 * @param[out] temperature_celsius Puntatore alla variabile dove salvare la temperatura.
 * @return 1 in caso di successo, 0 in caso di errore I2C.
 */
float32_t MAX30205_GetTemperature(uint8_t *TempBuffer)
{
    // Ricostruisce il dato a 16 bit con segno (raw_data[0] è MSB, raw_data[1] è LSB)
    int16_t temp_raw = (int16_t)((TempBuffer[0] << 8) | TempBuffer[1]);

    return (float32_t)temp_raw * 0.00390625f; // Successo
}