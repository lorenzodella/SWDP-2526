#include "tempsens_driver.h"
#include "arm_math.h"
#include "stm32u5xx_hal.h"
#include <math.h>

extern I2C_HandleTypeDef hi2c3;


uint8_t temp_write_register(uint8_t reg_addr, uint8_t reg_data)
{
    uint8_t tx_buffer[] = { reg_addr, reg_data };

    if (HAL_I2C_Master_Transmit(&hi2c3, (MAX30205_I2C_ADDRESS << 1), tx_buffer, sizeof(tx_buffer), MAX30205_I2C_TIMEOUT) != HAL_OK) {
        return 0; 
    }

    return 1; 
}

uint8_t temp_read_register(uint8_t reg_addr, uint8_t *data, uint16_t data_len)
{
    if (HAL_I2C_Mem_Read(&hi2c3, (MAX30205_I2C_ADDRESS << 1), reg_addr, I2C_MEMADD_SIZE_8BIT, data, data_len, MAX30205_I2C_TIMEOUT) != HAL_OK) {
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
 * @brief Sveglia il sensore per una singola conversione.
 */
uint8_t MAX30205_StartOneShot(void)
{
    // Write 0x81 (D7 One-Shot bit + D0 Shutdown bit set) 
    return temp_write_register(MAX30205_REG_CONF, 0x81);
}

/**
 * @brief Legge i 2 byte di temperatura e calcola il valore float in °C.
 * @param[out] temperature_celsius Puntatore alla variabile dove salvare la temperatura.
 * @return 1 in caso di successo, 0 in caso di errore I2C.
 */
uint8_t MAX30205_GetTemperature(float *temperature_celsius)
{
    uint8_t raw_data[2] = {0, 0};

    // Read 2 bytes starting from the temperature register (0x00) 
    if (!temp_read_register(MAX30205_REG_TEMP, raw_data, 2)) {
        return 0; // Errore di lettura
    }

    // Reconstruct the signed 16-bit value (raw_data[0] = MSB, raw_data[1] = LSB) 
    int16_t temp_raw = (int16_t)((raw_data[0] << 8) | raw_data[1]);

    // Convert to degrees Celsius using the sensor's resolution 
    *temperature_celsius = (float)temp_raw * 0.00390625f;

    return 1;
}