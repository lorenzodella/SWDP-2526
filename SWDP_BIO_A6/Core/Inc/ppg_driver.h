#ifndef __PPG_DRIVER_H__
#define __PPG_DRIVER_H__

/**
 * \file ppg_driver.h
 * \brief Driver library for the MAX30101 PPG sensor.
 */

#include "stdint.h"
#include "main.h"

// MAX30101 uses the 7-bit I2C address 0x57, corresponding to write/read
// addresses 0xAE and 0xAF.
#define MAX30101_I2C_ADDRESS    0x57
#define MAX30101_I2C_TIMEOUT    100

#define MAX30101_INT_EN_1_REG       0x02
#define MAX30101_FIFO_WR_PTR_REG    0x04
#define MAX30101_OVF_COUNTER_REG    0x05
#define MAX30101_FIFO_RD_PTR_REG    0x06
#define MAX30101_FIFO_CONFIG_REG    0x08
#define MAX30101_MODE_CONFIG_REG    0x09
#define MAX30101_SPO2_CONFIG_REG    0x0A
#define MAX30101_LED1_PA_REG        0x0C

#define MAX30101_SOFT_RESET         0x40
#define MAX30101_ENABLE_INT_A_FULL   0x80
#define MAX30101_ENABLE_INT_PPG_RDY  0x40
#define MAX30101_FIFO_A_FULL_15      0x0F
#define MAX30101_MODE_HEART_RATE     0x02
#define MAX30101_SPO2_CONFIG_0X27    0x27
#define MAX30101_LED1_PA_0X1F        0x1F

#define MAX30101_FIFO_DATA_REG       0x07

uint8_t PPG_Initialize(void);
uint8_t PPG_StartAcquisition(void);
uint8_t PPG_StopAcquisition(void);
// Read and parse FIFO samples. out_buf must have space for at least max_samples entries.
// Returns the number of samples written into out_buf.
uint16_t PPG_HandleInterrupt(uint32_t *out_buf, uint16_t max_samples);

#endif /* __PPG_DRIVER_H__ */
