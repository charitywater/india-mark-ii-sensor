/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : stm32l4xx_hal_msp.c
  * Description        : This file provides code for the MSP Initialization 
  *                      and de-Initialization codes.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include <main.h>

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN TD */

/* USER CODE END TD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN Define */
 
/* USER CODE END Define */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN Macro */

/* USER CODE END Macro */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* External functions --------------------------------------------------------*/
/* USER CODE BEGIN ExternalFunctions */

/* USER CODE END ExternalFunctions */

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */
/**
  * Initializes the Global MSP.
  */
void HAL_MspInit(void)
{
  /* USER CODE BEGIN MspInit 0 */

  /* USER CODE END MspInit 0 */

  __HAL_RCC_SYSCFG_CLK_ENABLE();
  __HAL_RCC_PWR_CLK_ENABLE();

  /* System interrupt init*/

  /* USER CODE BEGIN MspInit 1 */

  /* USER CODE END MspInit 1 */
}

/**
* @brief WWDG MSP Initialization
* This function configures the hardware resources used in this example
* @param hwwdg: WWDG handle pointer
* @retval None
*/
void HAL_WWDG_MspInit(WWDG_HandleTypeDef* hwwdg)
{
  if(hwwdg->Instance==WWDG)
  {
  /* USER CODE BEGIN WWDG_MspInit 0 */

  /* USER CODE END WWDG_MspInit 0 */
    /* Peripheral clock enable */
    __HAL_RCC_WWDG_CLK_ENABLE();
  /* USER CODE BEGIN WWDG_MspInit 1 */

  /* USER CODE END WWDG_MspInit 1 */
  }

}

/**
* @brief UART MSP Initialization
* This function configures the hardware resources used in this example
* @param huart: UART handle pointer
* @retval None
*/
void HAL_UART_MspInit(UART_HandleTypeDef* huart)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(huart->Instance==LPUART1)
  {
  /* USER CODE BEGIN LPUART1_MspInit 0 */

  /* USER CODE END LPUART1_MspInit 0 */
    /* Peripheral clock enable */
    __HAL_RCC_LPUART1_CLK_ENABLE();
  
    __HAL_RCC_GPIOC_CLK_ENABLE();
    /**LPUART1 GPIO Configuration    
    PC0     ------> LPUART1_RX
    PC1     ------> LPUART1_TX 
    */
    GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF8_LPUART1;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /* USER CODE BEGIN LPUART1_MspInit 1 */

  /* USER CODE END LPUART1_MspInit 1 */
  }
  else if(huart->Instance==USART1)
  {
  /* USER CODE BEGIN USART1_MspInit 0 */

  /* USER CODE END USART1_MspInit 0 */
    /* Peripheral clock enable */
    __HAL_RCC_USART1_CLK_ENABLE();
  
    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**USART1 GPIO Configuration    
    PA9     ------> USART1_TX
    PA10     ------> USART1_RX 
    */
    GPIO_InitStruct.Pin = GPIO_PIN_9|GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);


    HAL_NVIC_SetPriority(USART1_IRQn, 5, 0);
    HAL_NVIC_DisableIRQ(USART1_IRQn);

  /* USER CODE BEGIN USART1_MspInit 1 */

  /* USER CODE END USART1_MspInit 1 */
  }
  else if(huart->Instance==USART2)
  {
  /* USER CODE BEGIN USART1_MspInit 0 */

  /* USER CODE END USART1_MspInit 0 */
    /* Peripheral clock enable */
    __HAL_RCC_USART2_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**USART1 GPIO Configuration
    PA2     ------> USART2_TX
    PA3    ------> USART2_RX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_2|GPIO_PIN_3;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    HAL_NVIC_SetPriority(USART2_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(USART2_IRQn);

  /* USER CODE BEGIN USART1_MspInit 1 */

  /* USER CODE END USART1_MspInit 1 */
  }
  if(huart->Instance==UART5)
    {
    /* USER CODE BEGIN UART5_MspInit 0 */

    /* USER CODE END UART5_MspInit 0 */
      /* Peripheral clock enable */
      __HAL_RCC_UART5_CLK_ENABLE();

      __HAL_RCC_GPIOC_CLK_ENABLE();
      __HAL_RCC_GPIOD_CLK_ENABLE();
      __HAL_RCC_GPIOB_CLK_ENABLE();
      /**UART5 GPIO Configuration
      PC12     ------> UART5_TX
      PD2     ------> UART5_RX
      */
      GPIO_InitStruct.Pin = GPIO_PIN_12;
      GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
      GPIO_InitStruct.Pull = GPIO_PULLUP;
      GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
      GPIO_InitStruct.Alternate = GPIO_AF8_UART5;
      HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

      GPIO_InitStruct.Pin = GPIO_PIN_2;
      GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
      GPIO_InitStruct.Pull = GPIO_PULLUP;
      GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
      GPIO_InitStruct.Alternate = GPIO_AF8_UART5;
      HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

      GPIO_InitStruct.Pin = GPIO_PIN_5 | GPIO_PIN_4;
      GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
      GPIO_InitStruct.Pull = GPIO_PULLUP;
      GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
      GPIO_InitStruct.Alternate = GPIO_AF8_UART5;
      HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

      /* UART5 interrupt Init */
      HAL_NVIC_SetPriority(UART5_IRQn, 5, 0);
      HAL_NVIC_DisableIRQ(UART5_IRQn);

    /* USER CODE BEGIN UART5_MspInit 1 */

    /* USER CODE END UART5_MspInit 1 */
    }
    else if(huart->Instance==USART3)
    {
    /* USER CODE BEGIN USART3_MspInit 0 */

    /* USER CODE END USART3_MspInit 0 */
      /* Peripheral clock enable */
      __HAL_RCC_USART3_CLK_ENABLE();

      __HAL_RCC_GPIOC_CLK_ENABLE();
      /**USART3 GPIO Configuration
      PC4     ------> USART3_TX
      PC5     ------> USART3_RX
      */
      GPIO_InitStruct.Pin = GPIO_PIN_4|GPIO_PIN_5;
      GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
      GPIO_InitStruct.Pull = GPIO_NOPULL;
      GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
      GPIO_InitStruct.Alternate = GPIO_AF7_USART3;
      HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

      HAL_NVIC_SetPriority(USART3_IRQn, 0, 0);
      HAL_NVIC_EnableIRQ(USART3_IRQn);

    /* USER CODE BEGIN USART3_MspInit 1 */

    /* USER CODE END USART3_MspInit 1 */
    }
  if(huart->Instance==UART4)
    {
    /* USER CODE BEGIN UART4_MspInit 0 */

    /* USER CODE END UART4_MspInit 0 */
      /* Peripheral clock enable */
      __HAL_RCC_UART4_CLK_ENABLE();

      __HAL_RCC_GPIOA_CLK_ENABLE();
      /**UART4 GPIO Configuration
      PA0     ------> UART4_TX
      PA1     ------> UART4_RX
      */
      GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1;
      GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
      GPIO_InitStruct.Pull = GPIO_NOPULL;
      GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
      GPIO_InitStruct.Alternate = GPIO_AF8_UART4;
      HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

      HAL_NVIC_SetPriority(UART4_IRQn, 5, 0);
      HAL_NVIC_DisableIRQ(UART4_IRQn);

    /* USER CODE BEGIN UART4_MspInit 1 */

    /* USER CODE END UART4_MspInit 1 */
    }
}

/**
* @brief UART MSP De-Initialization
* This function freeze the hardware resources used in this example
* @param huart: UART handle pointer
* @retval None
*/
void HAL_UART_MspDeInit(UART_HandleTypeDef* huart)
{
  if(huart->Instance==LPUART1)
  {
  /* USER CODE BEGIN LPUART1_MspDeInit 0 */

  /* USER CODE END LPUART1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_LPUART1_CLK_DISABLE();
  
    /**LPUART1 GPIO Configuration    
    PC0     ------> LPUART1_RX
    PC1     ------> LPUART1_TX 
    */
    HAL_GPIO_DeInit(GPIOC, GPIO_PIN_0|GPIO_PIN_1);

  /* USER CODE BEGIN LPUART1_MspDeInit 1 */

  /* USER CODE END LPUART1_MspDeInit 1 */
  }
  else if(huart->Instance==USART1)
  {
  /* USER CODE BEGIN USART1_MspDeInit 0 */

  /* USER CODE END USART1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_USART1_CLK_DISABLE();
  
    /**USART1 GPIO Configuration    
    PA9     ------> USART1_TX
    PA10     ------> USART1_RX 
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_9|GPIO_PIN_10);

  /* USER CODE BEGIN USART1_MspDeInit 1 */

  /* USER CODE END USART1_MspDeInit 1 */
  }
  if(huart->Instance==UART5)
    {
    /* USER CODE BEGIN UART5_MspDeInit 0 */

    /* USER CODE END UART5_MspDeInit 0 */
      /* Peripheral clock disable */
      __HAL_RCC_UART5_CLK_DISABLE();

      /**UART5 GPIO Configuration
      PC12     ------> UART5_TX
      PD2     ------> UART5_RX
      */
      HAL_GPIO_DeInit(GPIOC, GPIO_PIN_12);

      HAL_GPIO_DeInit(GPIOD, GPIO_PIN_2);

    /* USER CODE BEGIN UART5_MspDeInit 1 */

    /* USER CODE END UART5_MspDeInit 1 */
    }
    else if(huart->Instance==USART3)
    {
    /* USER CODE BEGIN USART3_MspDeInit 0 */

    /* USER CODE END USART3_MspDeInit 0 */
      /* Peripheral clock disable */
      __HAL_RCC_USART3_CLK_DISABLE();

      /**USART3 GPIO Configuration
      PC4     ------> USART3_TX
      PC5     ------> USART3_RX
      */
      HAL_GPIO_DeInit(GPIOC, GPIO_PIN_4|GPIO_PIN_5);

    /* USER CODE BEGIN USART3_MspDeInit 1 */

    /* USER CODE END USART3_MspDeInit 1 */
    }
  if(huart->Instance==UART4)
    {
    /* USER CODE BEGIN UART4_MspDeInit 0 */

    /* USER CODE END UART4_MspDeInit 0 */
      /* Peripheral clock disable */
      __HAL_RCC_UART4_CLK_DISABLE();

      /**UART4 GPIO Configuration
      PA0     ------> UART4_TX
      PA1     ------> UART4_RX
      */
      HAL_GPIO_DeInit(GPIOA, GPIO_PIN_0|GPIO_PIN_1);

    /* USER CODE BEGIN UART4_MspDeInit 1 */

    /* USER CODE END UART4_MspDeInit 1 */
    }
}

/**
* @brief I2C MSP Initialization
* This function configures the hardware resources used in this example
* @param hi2c: I2C handle pointer
* @retval None
*/
void HAL_I2C_MspInit(I2C_HandleTypeDef* hi2c)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(hi2c->Instance==I2C1)
  {
  /* USER CODE BEGIN I2C1_MspInit 0 */

  /* USER CODE END I2C1_MspInit 0 */

    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**I2C1 GPIO Configuration
    PB6     ------> I2C1_SCL
    PB7     ------> I2C1_SDA
    */
    GPIO_InitStruct.Pin = GPIO_PIN_6|GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* Peripheral clock enable */
    __HAL_RCC_I2C1_CLK_ENABLE();
  /* USER CODE BEGIN I2C1_MspInit 1 */

  /* USER CODE END I2C1_MspInit 1 */
  }
  else if(hi2c->Instance==I2C2)
  {
  /* USER CODE BEGIN I2C1_MspInit 0 */

  /* USER CODE END I2C1_MspInit 0 */

    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**I2C2 GPIO Configuration
    PB10     ------> I2C2_SCL
    PB11     ------> I2C1_SDA
    */
    GPIO_InitStruct.Pin = GPIO_PIN_10|GPIO_PIN_11;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF4_I2C2;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* Peripheral clock enable */
    __HAL_RCC_I2C2_CLK_ENABLE();
  /* USER CODE BEGIN I2C1_MspInit 1 */

  /* USER CODE END I2C1_MspInit 1 */
  }

}

/**
* @brief I2C MSP De-Initialization
* This function freeze the hardware resources used in this example
* @param hi2c: I2C handle pointer
* @retval None
*/
void HAL_I2C_MspDeInit(I2C_HandleTypeDef* hi2c)
{
    if(hi2c->Instance==I2C1)
    {
        /* USER CODE BEGIN I2C1_MspDeInit 0 */

        /* USER CODE END I2C1_MspDeInit 0 */
        /* Peripheral clock disable */
        __HAL_RCC_I2C1_CLK_DISABLE();

        /**I2C1 GPIO Configuration
        PB6     ------> I2C1_SCL
        PB7     ------> I2C1_SDA
        */
        HAL_GPIO_DeInit(GPIOB, GPIO_PIN_6|GPIO_PIN_7);

        /* USER CODE BEGIN I2C1_MspDeInit 1 */

        /* USER CODE END I2C1_MspDeInit 1 */
    }
    else if(hi2c->Instance==I2C2)
    {
        /* USER CODE BEGIN I2C1_MspDeInit 0 */

        /* USER CODE END I2C1_MspDeInit 0 */
        /* Peripheral clock disable */
        __HAL_RCC_I2C2_CLK_DISABLE();

        /**I2C2 GPIO Configuration
        PB10     ------> I2C2_SCL
        PB11     ------> I2C1_SDA
         */
        HAL_GPIO_DeInit(GPIOB, GPIO_PIN_10|GPIO_PIN_11);

        /* USER CODE BEGIN I2C1_MspDeInit 1 */

        /* USER CODE END I2C1_MspDeInit 1 */
    }
}

/**
* @brief SPI MSP Initialization
* This function configures the hardware resources used in this example
* @param hspi: SPI handle pointer
* @retval None
*/
void HAL_SPI_MspInit(SPI_HandleTypeDef* hspi)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(hspi->Instance==SPI1)
  {
  /* USER CODE BEGIN SPI1_MspInit 0 */

  /* USER CODE END SPI1_MspInit 0 */
    /* Peripheral clock enable */
    __HAL_RCC_SPI1_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**SPI1 GPIO Configuration
    PA5     ------> SPI1_SCK
    PA6     ------> SPI1_MISO
    PA7     ------> SPI1_MOSI
    */
    GPIO_InitStruct.Pin = GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF5_SPI1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* USER CODE BEGIN SPI1_MspInit 1 */

  /* USER CODE END SPI1_MspInit 1 */
  }
  if(hspi->Instance==SPI2)
{
/* USER CODE BEGIN SPI2_MspInit 0 */

/* USER CODE END SPI2_MspInit 0 */
  /* Peripheral clock enable */
  __HAL_RCC_SPI2_CLK_ENABLE();

  __HAL_RCC_GPIOB_CLK_ENABLE();
  /**SPI2 GPIO Configuration
  PB13     ------> SPI2_SCK
  PB14     ------> SPI2_MISO
  PB15     ------> SPI2_MOSI
  */
  GPIO_InitStruct.Pin = GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF5_SPI2;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

/* USER CODE BEGIN SPI2_MspInit 1 */

/* USER CODE END SPI2_MspInit 1 */
}

}

/**
* @brief SPI MSP De-Initialization
* This function freeze the hardware resources used in this example
* @param hspi: SPI handle pointer
* @retval None
*/
void HAL_SPI_MspDeInit(SPI_HandleTypeDef* hspi)
{
  if(hspi->Instance==SPI1)
  {
  /* USER CODE BEGIN SPI1_MspDeInit 0 */

  /* USER CODE END SPI1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_SPI1_CLK_DISABLE();

    /**SPI1 GPIO Configuration
    PA5     ------> SPI1_SCK
    PA6     ------> SPI1_MISO
    PA7     ------> SPI1_MOSI
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7);

  /* USER CODE BEGIN SPI1_MspDeInit 1 */

  /* USER CODE END SPI1_MspDeInit 1 */
  }
  if(hspi->Instance==SPI2)
    {
    /* USER CODE BEGIN SPI2_MspDeInit 0 */

    /* USER CODE END SPI2_MspDeInit 0 */
      /* Peripheral clock disable */
      __HAL_RCC_SPI2_CLK_DISABLE();

      /**SPI2 GPIO Configuration
      PB13     ------> SPI2_SCK
      PB14     ------> SPI2_MISO
      PB15     ------> SPI2_MOSI
      */
      HAL_GPIO_DeInit(GPIOB, GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15);

    /* USER CODE BEGIN SPI2_MspDeInit 1 */

    /* USER CODE END SPI2_MspDeInit 1 */
    }
}

/**
  * Initializes the Global MSP.
  */

void HAL_RNG_MspInit(RNG_HandleTypeDef* hrng)
{
  if(hrng->Instance==RNG)
  {
      /* Peripheral clock enable */
    __HAL_RCC_RNG_CLK_ENABLE();
  }
}

void HAL_RNG_MspDeInit(RNG_HandleTypeDef* hrng)
{
  if(hrng->Instance==RNG)
  {
    /* Peripheral clock disable */
    __HAL_RCC_RNG_CLK_DISABLE();
  }
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
