#ifndef TEMPSENS_DRIVER_H
#define TEMPSENS_DRIVER_H

#include "stm32u5xx_hal.h"
#include "arm_math.h"

void TempSens_Init(void);
void TempSens_TimerCallback(TIM_HandleTypeDef *htim);
void TempSens_Process(void);
float32_t TempSens_GetLast(void);

#endif /* TEMPSENS_DRIVER_H */