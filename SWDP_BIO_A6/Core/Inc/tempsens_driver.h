#ifndef TEMPSENS_DRIVER_H
#define TEMPSENS_DRIVER_H

#include "arm_math.h"
#include "stdint.h"
#include "main.h"
#include <math.h>

#define MAX30205_I2C_ADDRESS    0x48  // Indirizzo base se A0,A1,A2 sono a GND
#define MAX30205_I2C_TIMEOUT    100   // Timeout in millisecondi// Indirizzi dei registri
#define MAX30205_REG_TEMP       0x00  // Registro temperatura (Read-only)
#define MAX30205_REG_CONF       0x01  // Registro configurazione (Read/Write)

static uint8_t temp_write_register(uint8_t reg_addr, uint8_t reg_data);
uint8_t Temp_Init(void);
float32_t MAX30205_GetTemperature(uint8_t *TempBuffer);

#endif /* TEMPSENS_DRIVER_H */