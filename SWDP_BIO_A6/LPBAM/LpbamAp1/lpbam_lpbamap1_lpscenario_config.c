/* USER CODE BEGIN Header */
/**
  **********************************************************************************************************************
  * @file    lpbam_lpbamap1_lpscenario_config.c
  * @author  MCD Application Team
  * @brief   Provide LPBAM LpbamAp1 application LPScenario configuration services.
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
/* Includes ----------------------------------------------------------------------------------------------------------*/
#include "lpbam_lpbamap1.h"

/* Private typedef ---------------------------------------------------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define --------------------------------------------------------------------------------------------------*/
#define TEMP_ACQ_Q_IDX (0U)
#define DMA_TIMEOUT_DURATION (0x1000U)

/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -----------------------------------------------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* External variables ------------------------------------------------------------------------------------------------*/
/* IP handler declaration */
extern I2C_HandleTypeDef hi2c3;
extern DMA_HandleTypeDef handle_LPDMA1_Channel0;
extern LPTIM_HandleTypeDef hlptim1;

/* LPBAM queues declaration */
extern DMA_QListTypeDef Temp_acq_Q;

/* USER CODE BEGIN EV */

/* USER CODE END EV */

/* Private function prototypes ---------------------------------------------------------------------------------------*/

/* LPBAM I2C3 management APIs */
static void MX_I2C3_Init(void);
static void MX_I2C3_MspInit(I2C_HandleTypeDef *hi2c3);
static void MX_I2C3_MspDeInit(I2C_HandleTypeDef *hi2c3);
static void MX_I2C3_DeInit(void);

/* LPBAM LPDMA1 management APIs */
static void MX_LPDMA1_Init(void);

/* LPBAM LPTIM1 management APIs */
static void MX_LPTIM1_Init(void);
static void MX_LPTIM1_MspInit(LPTIM_HandleTypeDef *hlptim1);
static void MX_LPTIM1_MspDeInit(LPTIM_HandleTypeDef *hlptim1);
static void MX_LPTIM1_DeInit(void);

/* LPBAM autonomous mode management APIs */
static void MX_AutonomousMode_Init(void);
static void MX_AutonomousMode_DeInit(void);

/* LPBAM queue linking/unlinking APIs */
static void MX_Temp_acq_Q_Link(DMA_HandleTypeDef *hdma);
static void MX_Temp_acq_Q_UnLink(DMA_HandleTypeDef *hdma);

/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Exported functions ----------------------------------------------------------------------------------------------*/
/**
  * @brief  LpbamAp1 application LPScenario scenario initialization
  * @param  None
  * @retval None
  */
void MX_LpbamAp1_LPScenario_Init(void)
{
  /* LPBAM I2C3 initialization */
  MX_I2C3_Init();

  /* LPBAM LPDMA1 initialization */
  MX_LPDMA1_Init();

  /* LPBAM LPTIM1 initialization */
  MX_LPTIM1_Init();

  /* Autonomous Mode initialization */
  MX_AutonomousMode_Init();

  /* USER CODE BEGIN LpbamAp1_LPScenario_Init */

  /* USER CODE END LpbamAp1_LPScenario_Init */
}

/**
  * @brief LpbamAp1 application LPScenario scenario de-initialization
  * @param None
  * @retval None
  */
void MX_LpbamAp1_LPScenario_DeInit(void)
{
  /* LPBAM I2C3 De-initialization */
  MX_I2C3_DeInit();

  /* LPBAM LPTIM1 De-initialization */
  MX_LPTIM1_DeInit();

  /* Autonomous mode de-initialization */
  MX_AutonomousMode_DeInit();

  /* USER CODE BEGIN LpbamAp1_LPScenario_DeInit */

  /* USER CODE END LpbamAp1_LPScenario_DeInit */
}

/**
  * @brief LpbamAp1 application LPScenario scenario link
  * @param None
  * @retval None
  */
void MX_LpbamAp1_LPScenario_Link(DMA_HandleTypeDef *hdma)
{
  /* USER CODE BEGIN LpbamAp1_LPScenario_Link 0 */

  /* USER CODE END LpbamAp1_LPScenario_Link 0 */

  /* Link Temp_acq queue to DMA channel */
  MX_Temp_acq_Q_Link(&hdma[TEMP_ACQ_Q_IDX]);

  /* USER CODE BEGIN LINK TEMP_ACQ_Q_IDX */

  /* USER CODE END LINK TEMP_ACQ_Q_IDX */

  /* USER CODE BEGIN LpbamAp1_LPScenario_Link 1 */

  /* USER CODE END LpbamAp1_LPScenario_Link 1 */
}

/**
  * @brief LpbamAp1 application LPScenario scenario unlink
  * @param hdma :Pointer to a DMA_HandleTypeDef structure that contains the configuration information for the specified
  *              DMA Channel
  * @retval None
  */
void MX_LpbamAp1_LPScenario_UnLink(DMA_HandleTypeDef *hdma)
{
  /* USER CODE BEGIN LpbamAp1_LPScenario_UnLink 0 */

  /* USER CODE END LpbamAp1_LPScenario_UnLink 0 */

  /* LPBAM unLink Temp_acq queue to DMA channel */
  MX_Temp_acq_Q_UnLink(&hdma[TEMP_ACQ_Q_IDX]);

  /* USER CODE BEGIN UNLINK TEMP_ACQ_Q_IDX */

  /* USER CODE END UNLINK TEMP_ACQ_Q_IDX */

  /* USER CODE BEGIN LpbamAp1_LPScenario_UnLink 1 */

  /* USER CODE END LpbamAp1_LPScenario_UnLink 1 */
}

/**
  * @brief LpbamAp1 application LPScenario scenario start
  * @retval None
  */
void MX_LpbamAp1_LPScenario_Start(DMA_HandleTypeDef *hdma)
{
  /* LPBAM start DMA channel in linked-list mode */
  if (HAL_DMAEx_List_Start(&hdma[TEMP_ACQ_Q_IDX]) != HAL_OK)
  {
    Error_Handler();
  }

  /* USER CODE BEGIN LpbamAp1_LPScenario_Start */

  /* USER CODE END LpbamAp1_LPScenario_Start */
}

/**
  * @brief LpbamAp1 application LPScenario scenario stop
  * @retval None
  */
void MX_LpbamAp1_LPScenario_Stop(DMA_HandleTypeDef *hdma)
{
  /* LPBAM stop DMA channel in linked-list mode */
  if ((hdma[TEMP_ACQ_Q_IDX].State == HAL_DMA_STATE_BUSY) && (hdma[TEMP_ACQ_Q_IDX].LinkedListQueue->FirstCircularNode != 0U))
  {
    if (HAL_DMA_Abort(&hdma[TEMP_ACQ_Q_IDX]) != HAL_OK)
    {
      Error_Handler();
    }
  }

  /* Check if DMA channel interrupt is enabled */
  if ((hdma[TEMP_ACQ_Q_IDX].State == HAL_DMA_STATE_BUSY) && (__HAL_DMA_GET_IT_SOURCE(&hdma[TEMP_ACQ_Q_IDX], DMA_IT_TC) == 0U))
  {
    if (HAL_DMA_PollForTransfer(&hdma[TEMP_ACQ_Q_IDX], HAL_DMA_FULL_TRANSFER, DMA_TIMEOUT_DURATION) != HAL_OK)
    {
      Error_Handler();
    }
  }

  /* USER CODE BEGIN LpbamAp1_LPScenario_Stop */

  /* USER CODE END LpbamAp1_LPScenario_Stop */
}

/**
  * @brief LpbamAp1 application LPScenario autonomous mode init
  * @param None
  * @retval None
  */
static void MX_AutonomousMode_Init(void)
{
  /* Enable LPDMA1 Sleep Clock */
  __HAL_RCC_LPDMA1_CLK_SLEEP_ENABLE();
  /* Enable LPDMA1 Autonomous Mode */
  __HAL_RCC_LPDMA1_CLKAM_ENABLE();

  /* Enable SRAM4 Sleep Clock */
  __HAL_RCC_SRAM4_CLK_SLEEP_ENABLE();
  /* Enable SRAM4 Autonomous Mode */
  __HAL_RCC_SRAM4_CLKAM_ENABLE();

  /* Enable I2C3 Sleep Clock */
  __HAL_RCC_I2C3_CLK_SLEEP_ENABLE();
  /* Enable I2C3 Autonomous Mode */
  __HAL_RCC_I2C3_CLKAM_ENABLE();

  /* Enable LPTIM1 Sleep Clock */
  __HAL_RCC_LPTIM1_CLK_SLEEP_ENABLE();
  /* Enable LPTIM1 Autonomous Mode */
  __HAL_RCC_LPTIM1_CLKAM_ENABLE();

  /* USER CODE BEGIN AutonomousMode_Init */

  /* USER CODE END AutonomousMode_Init */
}

/**
  * @brief  LpbamAp1 application LPScenario autonomous mode deinit
  * @param  None
  * @retval None
  */
static void MX_AutonomousMode_DeInit(void)
{
  /* Disable LPDMA1 Sleep Clock */
  __HAL_RCC_LPDMA1_CLK_SLEEP_DISABLE();
  /* Disable LPDMA1 Autonomous Mode */
  __HAL_RCC_LPDMA1_CLKAM_DISABLE();

  /* Disable SRAM4 Sleep Clock */
  __HAL_RCC_SRAM4_CLK_SLEEP_DISABLE();
  /* Disable SRAM4 Autonomous Mode */
  __HAL_RCC_SRAM4_CLKAM_DISABLE();

  /* Disable I2C3 Sleep Clock */
  __HAL_RCC_I2C3_CLK_SLEEP_DISABLE();
  /* Disable I2C3 Autonomous Mode */
  __HAL_RCC_I2C3_CLKAM_DISABLE();

  /* Disable LPTIM1 Sleep Clock */
  __HAL_RCC_LPTIM1_CLK_SLEEP_DISABLE();
  /* Disable LPTIM1 Autonomous Mode */
  __HAL_RCC_LPTIM1_CLKAM_DISABLE();

  /* USER CODE BEGIN AutonomousMode_DeInit */

  /* USER CODE END AutonomousMode_DeInit */
}

/**
  * @brief I2C3 initialization.
  * @param None
  * @retval None
  */
static void MX_I2C3_Init(void)
{
  /* USER CODE BEGIN I2C3_Init 0 */

  /* USER CODE END I2C3_Init 0 */

  /* USER CODE BEGIN I2C3_Init 1 */

  /* USER CODE END I2C3_Init 1 */

  /* Set I2C3 instance */
  hi2c3.Instance = I2C3;

  /* Register I2C msp callbacks */
  if (HAL_I2C_RegisterCallback(&hi2c3, HAL_I2C_MSPINIT_CB_ID, MX_I2C3_MspInit) != HAL_OK)
  {
    Error_Handler();
  }
  /* Register I2C msp callbacks */
  if (HAL_I2C_RegisterCallback(&hi2c3, HAL_I2C_MSPDEINIT_CB_ID, MX_I2C3_MspDeInit) != HAL_OK)
  {
    Error_Handler();
  }
  hi2c3.Instance = I2C3;
  hi2c3.Init.Timing = 0x00303D5B;
  hi2c3.Init.OwnAddress1 = 0;
  hi2c3.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c3.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c3.Init.OwnAddress2 = 0;
  hi2c3.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c3.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c3.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c3) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c3, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c3, 0) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configuration in LPBAM context
  */
  if (HAL_I2CEx_EnableWakeUp(&hi2c3) != HAL_OK)
  {
    Error_Handler();
  }
  if (ADV_LPBAM_I2C_EnableDMARequests(I2C3) != LPBAM_OK)
  {
    Error_Handler();
  }
  /** Register ISR
  */
  if (ADV_LPBAM_I2C_RegisterISR(&hi2c3) != LPBAM_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C3_Init 2 */

  /* USER CODE END I2C3_Init 2 */

}

/**
  * @brief I2C3 de-initialization.
  * @param None
  * @retval None
  */
static void MX_I2C3_DeInit(void)
{
  /* USER CODE BEGIN I2C3_DeInit 0 */

  /* USER CODE END I2C3_DeInit 0 */

  /* Set I2C3 instance */
  hi2c3.Instance = I2C3;

  /* UnRegister I2C msp callbacks */
  if (HAL_I2C_UnRegisterCallback(&hi2c3, HAL_I2C_MSPINIT_CB_ID) != HAL_OK)
  {
    Error_Handler();
  }
  /* Init I2C3 peripheral */
  if (HAL_I2C_DeInit(&hi2c3) != HAL_OK)
  {
    Error_Handler();
  }
  /* UnRegister I2C msp callbacks */
  if (HAL_I2C_UnRegisterCallback(&hi2c3, HAL_I2C_MSPDEINIT_CB_ID) != HAL_OK)
  {
    Error_Handler();
  }

  /* USER CODE BEGIN I2C3_DeInit 1 */

  /* USER CODE END I2C3_DeInit 1 */
}

/**
  * @brief I2C3 MSP initialization.
  * @retval None
  */
static void MX_I2C3_MspInit(I2C_HandleTypeDef* i2cHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};
  if(i2cHandle->Instance==I2C3)
  {
  /* USER CODE BEGIN I2C3_MspInit 0 */

  /* USER CODE END I2C3_MspInit 0 */

  /** Initializes the peripherals clock
  */
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_I2C3;
    PeriphClkInit.I2c3ClockSelection = RCC_I2C3CLKSOURCE_HSI;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_RCC_GPIOC_CLK_ENABLE();
    /**I2C3 GPIO Configuration
    PC0     ------> I2C3_SCL
    PC1     ------> I2C3_SDA
    */
    GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF4_I2C3;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    /* I2C3 clock enable */
    __HAL_RCC_I2C3_CLK_ENABLE();
  /* USER CODE BEGIN I2C3_MspInit 1 */

  /* USER CODE END I2C3_MspInit 1 */
  }
}

/**
  * @brief I2C3 MSP de-initialization.
  * @retval None
  */
static void MX_I2C3_MspDeInit(I2C_HandleTypeDef* i2cHandle)
{

  if (i2cHandle->Instance==I2C3)
  {
  /* USER CODE BEGIN I2C3_MspDeInit 0 */

  /* USER CODE END I2C3_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_I2C3_CLK_DISABLE();

    /**I2C3 GPIO Configuration
    PC0     ------> I2C3_SCL
    PC1     ------> I2C3_SDA
    */
    HAL_GPIO_DeInit(GPIOC, GPIO_PIN_0);

    HAL_GPIO_DeInit(GPIOC, GPIO_PIN_1);

  /* USER CODE BEGIN I2C3_MspDeInit 1 */

  /* USER CODE END I2C3_MspDeInit 1 */
  }
}

/**
  * @brief LPDMA1 initialization.
  * @param None
  * @retval None
  */
static void MX_LPDMA1_Init(void)
{
  /* USER CODE BEGIN LPDMA1_Init 0 */

  /* USER CODE END LPDMA1_Init 0 */

  /* Peripheral clock enable */
  __HAL_RCC_LPDMA1_CLK_ENABLE();

  /* USER CODE BEGIN LPDMA1_Init 1 */

  /* USER CODE END LPDMA1_Init 1 */

  /* Set LPDMA1 instance */
  handle_LPDMA1_Channel0.Instance = LPDMA1_Channel0;
  handle_LPDMA1_Channel0.InitLinkedList.Priority = DMA_HIGH_PRIORITY;
  handle_LPDMA1_Channel0.InitLinkedList.LinkStepMode = DMA_LSM_FULL_EXECUTION;
  handle_LPDMA1_Channel0.InitLinkedList.TransferEventMode = DMA_TCEM_LAST_LL_ITEM_TRANSFER;
  handle_LPDMA1_Channel0.InitLinkedList.LinkedListMode = DMA_LINKEDLIST_NORMAL;
  if (HAL_DMAEx_List_Init(&handle_LPDMA1_Channel0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN LPDMA1_Init 2 */

  /* USER CODE END LPDMA1_Init 2 */

}

/**
  * @brief LPTIM1 initialization.
  * @param None
  * @retval None
  */
static void MX_LPTIM1_Init(void)
{
  /* USER CODE BEGIN LPTIM1_Init 0 */

  /* USER CODE END LPTIM1_Init 0 */

  LPTIM_OC_ConfigTypeDef sConfig1 = {0};

  /* USER CODE BEGIN LPTIM1_Init 1 */

  /* USER CODE END LPTIM1_Init 1 */

  /* Set LPTIM1 instance */
  hlptim1.Instance = LPTIM1;

  /* Register LPTIM msp callbacks */
  if (HAL_LPTIM_RegisterCallback(&hlptim1, HAL_LPTIM_MSPINIT_CB_ID, MX_LPTIM1_MspInit) != HAL_OK)
  {
    Error_Handler();
  }
  /* Register LPTIM msp callbacks */
  if (HAL_LPTIM_RegisterCallback(&hlptim1, HAL_LPTIM_MSPDEINIT_CB_ID, MX_LPTIM1_MspDeInit) != HAL_OK)
  {
    Error_Handler();
  }
  hlptim1.Instance = LPTIM1;
  hlptim1.Init.Clock.Source = LPTIM_CLOCKSOURCE_APBCLOCK_LPOSC;
  hlptim1.Init.Clock.Prescaler = LPTIM_PRESCALER_DIV32;
  hlptim1.Init.Trigger.Source = LPTIM_TRIGSOURCE_SOFTWARE;
  hlptim1.Init.Period = 30000;
  hlptim1.Init.UpdateMode = LPTIM_UPDATE_IMMEDIATE;
  hlptim1.Init.CounterSource = LPTIM_COUNTERSOURCE_INTERNAL;
  hlptim1.Init.Input1Source = LPTIM_INPUT1SOURCE_GPIO;
  hlptim1.Init.Input2Source = LPTIM_INPUT2SOURCE_GPIO;
  hlptim1.Init.RepetitionCounter = 0;
  if (HAL_LPTIM_Init(&hlptim1) != HAL_OK)
  {
    Error_Handler();
  }
  sConfig1.Pulse = 15000;
  sConfig1.OCPolarity = LPTIM_OCPOLARITY_HIGH;
  if (HAL_LPTIM_OC_ConfigChannel(&hlptim1, &sConfig1, LPTIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN LPTIM1_Init 2 */

  /* USER CODE END LPTIM1_Init 2 */

}

/**
  * @brief LPTIM1 de-initialization.
  * @param None
  * @retval None
  */
static void MX_LPTIM1_DeInit(void)
{
  /* USER CODE BEGIN LPTIM1_DeInit 0 */

  /* USER CODE END LPTIM1_DeInit 0 */

  /* Set LPTIM1 instance */
  hlptim1.Instance = LPTIM1;

  /* UnRegister LPTIM msp callbacks */
  if (HAL_LPTIM_UnRegisterCallback(&hlptim1, HAL_LPTIM_MSPINIT_CB_ID) != HAL_OK)
  {
    Error_Handler();
  }
  /* Init LPTIM1 peripheral */
  if (HAL_LPTIM_DeInit(&hlptim1) != HAL_OK)
  {
    Error_Handler();
  }
  /* UnRegister LPTIM msp callbacks */
  if (HAL_LPTIM_UnRegisterCallback(&hlptim1, HAL_LPTIM_MSPDEINIT_CB_ID) != HAL_OK)
  {
    Error_Handler();
  }

  /* USER CODE BEGIN LPTIM1_DeInit 1 */

  /* USER CODE END LPTIM1_DeInit 1 */
}

/**
  * @brief LPTIM1 MSP initialization.
  * @retval None
  */
static void MX_LPTIM1_MspInit(LPTIM_HandleTypeDef* lptimHandle)
{

  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};
  if(lptimHandle->Instance==LPTIM1)
  {
  /* USER CODE BEGIN LPTIM1_MspInit 0 */

  /* USER CODE END LPTIM1_MspInit 0 */

  /** Initializes the peripherals clock
  */
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_LPTIM1;
    PeriphClkInit.Lptim1ClockSelection = RCC_LPTIM1CLKSOURCE_MSIK;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
    {
      Error_Handler();
    }

    /* LPTIM1 clock enable */
    __HAL_RCC_LPTIM1_CLK_ENABLE();
  /* USER CODE BEGIN LPTIM1_MspInit 1 */

  /* USER CODE END LPTIM1_MspInit 1 */
  }
}

/**
  * @brief LPTIM1 MSP de-initialization.
  * @retval None
  */
static void MX_LPTIM1_MspDeInit(LPTIM_HandleTypeDef* lptimHandle)
{

  if (lptimHandle->Instance==LPTIM1)
  {
  /* USER CODE BEGIN LPTIM1_MspDeInit 0 */

  /* USER CODE END LPTIM1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_LPTIM1_CLK_DISABLE();
  /* USER CODE BEGIN LPTIM1_MspDeInit 1 */

  /* USER CODE END LPTIM1_MspDeInit 1 */
  }
}

/**
  * @brief  Temp_acq queue link
  * @retval None
  */
static void MX_Temp_acq_Q_Link(DMA_HandleTypeDef *hdma)
{
  /* Enable LPDMA1 clock */
  __HAL_RCC_LPDMA1_CLK_ENABLE();

  hdma->InitLinkedList.Priority = DMA_LOW_PRIORITY_LOW_WEIGHT;
  hdma->InitLinkedList.LinkStepMode = DMA_LSM_FULL_EXECUTION;
  hdma->InitLinkedList.TransferEventMode = DMA_TCEM_LAST_LL_ITEM_TRANSFER;
  hdma->InitLinkedList.LinkedListMode = DMA_LINKEDLIST_NORMAL;
  if (HAL_DMAEx_List_Init(hdma) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_DMAEx_List_LinkQ(hdma, &Temp_acq_Q) != HAL_OK)
  {
    Error_Handler();
  }

  /* Register DMA channel error callbacks */
}

/**
  * @brief  Temp_acq queue unlink
  * @retval None
  */
static void MX_Temp_acq_Q_UnLink(DMA_HandleTypeDef *hdma)
{
  /* UnLink Temp_acq queue to DMA channel */
  if (HAL_DMAEx_List_UnLinkQ(hdma) != HAL_OK)
  {
    Error_Handler();
  }

  /* DMA linked list de-init */
  if (HAL_DMAEx_List_DeInit(hdma) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN LpbamAp1_LPScenario_Config */

/* USER CODE END LpbamAp1_LPScenario_Config */

