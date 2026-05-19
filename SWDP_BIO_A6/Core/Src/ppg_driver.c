/**
 * \file ppg_driver.c
 * \brief Implementation file for the MAX30101 PPG driver.
 */

#include "ppg_driver.h"
#include "stm32u5xx_hal.h"

extern I2C_HandleTypeDef hi2c3;

static uint8_t ppg_write_register(uint8_t reg_addr, uint8_t reg_data);

static uint8_t ppg_read_register(uint8_t reg_addr, uint8_t *data, uint16_t data_len)
{
    // if (HAL_I2C_Master_Transmit(&hi2c3, MAX30101_I2C_ADDRESS << 1, &reg_addr, 1, MAX30101_I2C_TIMEOUT) != HAL_OK) {
    //     return 0;
    // }

    // if (HAL_I2C_Master_Receive(&hi2c3, MAX30101_I2C_ADDRESS << 1, data, data_len, MAX30101_I2C_TIMEOUT) != HAL_OK) {
    //     return 0;
    // }

    if (HAL_I2C_Mem_Read(&hi2c3, (MAX30101_I2C_ADDRESS << 1), reg_addr, I2C_MEMADD_SIZE_8BIT, data, data_len, MAX30101_I2C_TIMEOUT) != HAL_OK) {
        return 0;
    }

    return 1;
}

uint8_t PPG_Initialize(void)
{
    // reset the sensor
    if (!ppg_write_register(MAX30101_MODE_CONFIG_REG, MAX30101_SOFT_RESET)) {
        return 0;
    }

    // Wait for the reset to finish before writing the rest of the configuration
    HAL_Delay(10); // 10 ms is plenty

    // enable full and PPG ready interrupts
    if (!ppg_write_register(MAX30101_INT_EN_1_REG, MAX30101_ENABLE_INT_A_FULL)) {
        return 0;
    }

    // set FIFO_A_FULL threshold to 15 samples
    if (!ppg_write_register(MAX30101_FIFO_CONFIG_REG, MAX30101_FIFO_A_FULL_15)) {
        return 0;
    }

    // configure ADC range, sample rate and pulse width
    if (!ppg_write_register(MAX30101_SPO2_CONFIG_REG, MAX30101_SPO2_CONFIG_0X27)) {
        return 0;
    }

    // set LED1 current to 0x1F (6.2 mA)
    if (!ppg_write_register(MAX30101_LED1_PA_REG, MAX30101_LED1_PA_0X1F)) {
        return 0;
    }

    return 1;
}

uint8_t PPG_StartAcquisition(void)
{
    // reset FIFO pointers
    if (!ppg_write_register(MAX30101_FIFO_WR_PTR_REG, 0x00)) {
        return 0;
    }

    if (!ppg_write_register(MAX30101_OVF_COUNTER_REG, 0x00)) {
        return 0;
    }

    if (!ppg_write_register(MAX30101_FIFO_RD_PTR_REG, 0x00)) {
        return 0;
    }

    // set mode to heart rate mode (red LED only)
    if (!ppg_write_register(MAX30101_MODE_CONFIG_REG, MAX30101_MODE_HEART_RATE)) {
        return 0;
    }

    // clean up interrupts maybe
    uint8_t dummy;
    ppg_read_register(0x00, &dummy, 1);

    return 1;
}

uint8_t PPG_StopAcquisition(void)
{
    // Put the sensor in shutdown mode (SHDN bit = 1); stops sampling and INT generation
    if (!ppg_write_register(MAX30101_MODE_CONFIG_REG, 0x80)) {
        return 0;
    }

    // Clear any pending interrupt by reading the status register
    uint8_t dummy;
    ppg_read_register(0x00, &dummy, 1);

    return 1;
}

uint16_t PPG_HandleInterrupt(uint32_t *out_buf, uint16_t max_samples)
{
    uint8_t status = 0;
    // Read Status 1 to clear the interrupt flag and get the source
    ppg_read_register(0x00, &status, 1);

    // If it's not A_FULL (0x80) or PPG_RDY (0x40), return early without reading
    if ((status & 0x80) == 0) {
        return 0; 
    }

    uint8_t wr_ptr = 0, rd_ptr = 0;
    // Read FIFO write and read pointers
    if (!ppg_read_register(MAX30101_FIFO_WR_PTR_REG, &wr_ptr, 1)) return 0;
    if (!ppg_read_register(MAX30101_FIFO_RD_PTR_REG, &rd_ptr, 1)) return 0;

    uint8_t wr = wr_ptr & 0x1F;
    uint8_t rd = rd_ptr & 0x1F;
    uint16_t available = 0;
    if (wr >= rd) available = wr - rd;
    else available = (wr + 32) - rd; // handle wrap-around

    if (available == 0) return 0;

    if (available > max_samples) available = max_samples;

    uint16_t bytes_to_read = available * 3;
    uint8_t fifo_buf[96]; // up to 32 samples * 3 bytes

    // 5. Execute the Burst Read
    // The HAL handles the I2C clocking. The STM32 asks for 'bytes_to_read' from address 0x07.
    // The MAX30101 feeds the FIFO data through 0x07 and increments its internal pointer automatically.
    HAL_I2C_Mem_Read(&hi2c3, 
                    (MAX30101_I2C_ADDRESS << 1),       // I2C Address shifted left
                    MAX30101_FIFO_DATA_REG,              // FIFO_DATA register
                    I2C_MEMADD_SIZE_8BIT, 
                    fifo_buf,       // Destination buffer
                    bytes_to_read,     // Total bytes to pull
                    MAX30101_I2C_TIMEOUT);
                    
    // Reconstruct 18-bit left-justified data from each 3-byte group
    for (uint16_t i = 0; i < available; ++i) {
        uint8_t b1 = fifo_buf[i*3 + 0];
        uint8_t b2 = fifo_buf[i*3 + 1];
        uint8_t b3 = fifo_buf[i*3 + 2];
        uint32_t raw = (((uint32_t)b1 << 16) | ((uint32_t)b2 << 8) | (uint32_t)b3) & 0x03FFFF;
        out_buf[i] = raw;
    }


    return available;
}

static uint8_t ppg_write_register(uint8_t reg_addr, uint8_t reg_data)
{
    uint8_t tx_buffer[] = { reg_addr, reg_data };

    if (HAL_I2C_Master_Transmit(&hi2c3, MAX30101_I2C_ADDRESS << 1, tx_buffer, sizeof(tx_buffer), MAX30101_I2C_TIMEOUT) != HAL_OK) {
        return 0;
    }

    return 1;
}