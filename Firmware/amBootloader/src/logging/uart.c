/*
================================================================================================#=
Module:   UART Driver

Description:
    Driver to send and receive uart data. Manages all of the UART peripherals

Copyright 2021 Twisthink LLC
This code is licensed under Twisthink license.
See Twisthink-SoftwareLicenseAgreement-Auris.docx for details.
================================================================================================#=
*/

#include <stm32l4xx_hal.h>
#include "stdint.h"
#include "string.h"
#include "stdarg.h"
#include "stdio.h"
#include "stdbool.h"
#include "logger.h"
#include "logTypes.h"
#include <uart.h>

UART_HandleTypeDef hlpuart1;

static void xInitLpuart1(void);
static void xDeinitLpuart1(void);

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{

}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{

}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1)
    {
        //USART1
    }
}

void UART_initPeripherals(void)
{
    xInitLpuart1(); //log
}

void UART_deinitDebugPeripherals(void)
{
    xDeinitLpuart1();
}

void UART_sendDataBlocking(UART_Periph_t device, uint8_t *pData, uint16_t bytesToSend)
{

    if (device == LOG)
    {
        HAL_UART_Transmit(&hlpuart1, pData, bytesToSend, 0xffff);
    }
    else
    {
        elogError("Provide valid UART device");
    }
}


static void xDeinitLpuart1(void)
{
    HAL_UART_MspDeInit(&hlpuart1);
}

/**
  * @brief LPUART1 Initialization Function
  * @param None
  * @retval None
  */
static void xInitLpuart1(void)
{
    /* Init LPUSART 1 */
     hlpuart1.Instance = LPUART1;
     hlpuart1.Init.BaudRate = 9600;
     hlpuart1.Init.WordLength = UART_WORDLENGTH_8B;
     hlpuart1.Init.StopBits = UART_STOPBITS_1;
     hlpuart1.Init.Parity = UART_PARITY_NONE;
     hlpuart1.Init.Mode = UART_MODE_TX_RX;
     hlpuart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
     hlpuart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
     hlpuart1.Init.ClockPrescaler = UART_PRESCALER_DIV1;
     hlpuart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
     hlpuart1.FifoMode = UART_FIFOMODE_DISABLE;
     if (HAL_UART_Init(&hlpuart1) == HAL_OK)
     {
         HAL_UARTEx_SetTxFifoThreshold(&hlpuart1, UART_TXFIFO_THRESHOLD_1_8);
         HAL_UARTEx_SetRxFifoThreshold(&hlpuart1, UART_RXFIFO_THRESHOLD_1_8);
     }
}
