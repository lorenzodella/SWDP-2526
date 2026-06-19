#ifndef TEMPSENS_DRIVER_H
#define TEMPSENS_DRIVER_H

#include "arm_math.h"
#include "stdint.h"
#include "main.h"

#define MAX30205_I2C_ADDRESS    0x48  // Indirizzo base se A0,A1,A2 sono a GND
#define MAX30205_I2C_TIMEOUT    100   // Timeout in millisecondi// Indirizzi dei registri
#define MAX30205_REG_TEMP       0x00  // Registro temperatura (Read-only)
#define MAX30205_REG_CONF       0x01  // Registro configurazione (Read/Write)

uint8_t temp_write_register(uint8_t reg_addr, uint8_t reg_data);
uint8_t temp_read_register(uint8_t reg_addr, uint8_t *data, uint16_t data_len);
uint8_t Temp_Init(void);
uint8_t MAX30205_StartOneShot(void);
uint8_t MAX30205_GetTemperature(float *temperature_celsius);

#endif /* TEMPSENS_DRIVER_H */