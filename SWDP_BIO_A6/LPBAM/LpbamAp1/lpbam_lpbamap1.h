/* USER CODE BEGIN Header */
/**
  **********************************************************************************************************************
  * @file    lpbam_lpbamap1.h
  * @author  MCD Application Team
  * @brief   Header for LPBAM LpbamAp1 application
  **********************************************************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  **********************************************************************************************************************
  */
/* USER CODE END Header */
/* Define to prevent recursive inclusion -----------------------------------------------------------------------------*/
#ifndef LPBAM_LPBAMAP1_H
#define LPBAM_LPBAMAP1_H

/* Includes ----------------------------------------------------------------------------------------------------------*/
#include "main.h"
#include "stm32_lpbam.h"

/* Exported functions ------------------------------------------------------------------------------------------------*/
/* LpbamAp1 application initialization */
void MX_LpbamAp1_Init(void);

/* LpbamAp1 application LPScenario scenario initialization */
void MX_LpbamAp1_LPScenario_Init(void);

/* LpbamAp1 application LPScenario scenario de-initialization */
void MX_LpbamAp1_LPScenario_DeInit(void);

/* LpbamAp1 application LPScenario scenario build */
void MX_LpbamAp1_LPScenario_Build(void);

/* LpbamAp1 application LPScenario scenario link */
void MX_LpbamAp1_LPScenario_Link(DMA_HandleTypeDef *hdma);

/* LpbamAp1 application LPScenario scenario unlink */
void MX_LpbamAp1_LPScenario_UnLink(DMA_HandleTypeDef *hdma);

/* LpbamAp1 application LPScenario scenario start */
void MX_LpbamAp1_LPScenario_Start(DMA_HandleTypeDef *hdma);

/* LpbamAp1 application LPScenario scenario stop */
void MX_LpbamAp1_LPScenario_Stop(DMA_HandleTypeDef *hdma);

#endif /* LPBAM_LPBAMAP1_H */
