/**************************************************************************************************
* \file     uart.c
* \brief    Driver to send and receive uart data. Manages all of the UART peripherals and is built on
*           top of the HAL uart driver to make it thread safe
*
* \par      Copyright Notice
*           Copyright 2021 charity: water
*
*           Licensed under the Apache License, Version 2.0 (the "License");
*           you may not use this file except in compliance with the License.
*           You may obtain a copy of the License at
*
*               http://www.apache.org/licenses/LICENSE-2.0
*
*           Unless required by applicable law or agreed to in writing, software
*           distributed under the License is distributed on an "AS IS" BASIS,
*           WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*           See the License for the specific language governing permissions and
*           limitations under the License.
*           
* \date     2/1/2021
* \author   Twisthink
*
***************************************************************************************************/

#include <uart.h>

#include <stm32l4xx_hal.h>
#include "stdint.h"
#include "string.h"
#include "stdarg.h"
#include "stdio.h"
#include "stdbool.h"
#include "ssm.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "logTypes.h"
#include "gpsManager.h"
#include "CLI.h"
#include "nwStackFunctionality.h"
#include "connectivity.h"

UART_HandleTypeDef hlpuart1;
UART_HandleTypeDef huart1;
UART_HandleTypeDef huart3;
UART_HandleTypeDef huart4;
UART_HandleTypeDef huart5;
DMA_HandleTypeDef hdma_uart5_tx;

SemaphoreHandle_t xUartTxMutex;
SemaphoreHandle_t xUartRxMutex;

static void xInitLpuart1(void);
static void xInitUart1(void);
static void xInitUart3(void);
static void xInitUart4(void);
static void xInitUart5(void);

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1)
    {
        CLI_charReceivedCb();
    }
    else if (huart->Instance == USART3)
    {
        GPS_processReceivedChars();
    }
    else if (huart->Instance == UART4)
    {

    }
    else if (huart->Instance == UART5)
    {
        NW_processCellRxByte();
    }
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == UART5)
    {
        NW_txComplete();
    }
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1)
    {
        //USART1
    }
    else if (huart->Instance == USART3)
    {
        GPS_HandleUartErr();
    }
    else if (huart->Instance == UART4)
    {

    }
    else if (huart->Instance == UART5)
    {
        NW_handleUartError();
    }
}

void UART_initPeripherals(void)
{
    /* create mutex */
    xUartTxMutex = xSemaphoreCreateMutex();
    xUartRxMutex = xSemaphoreCreateMutex();

    xInitLpuart1(); //log
    xInitUart4(); //ssm
}

void UART_deinitDebugPeripherals(void)
{
    //deinit these uart peripherals before going to sleep
    HAL_UART_DeInit(&hlpuart1);
    HAL_UART_DeInit(&huart1);
    HAL_UART_DeInit(&huart4);
}

void UART_initCellUart(void)
{
    xInitUart5();
}

void UART_initGpsUart(void)
{
    xInitUart3();
}

void UART_initCliUart(void)
{
    xInitUart1();
}

void UART_deinitGpsUart(void)
{
    if (HAL_UART_DeInit(&huart3) != HAL_OK)
    {
        elogError("FAILED TO DEINIT UART 3");
    }
}

void UART_sendDataBlocking(UART_Periph_t device, uint8_t *pData, uint16_t bytesToSend)
{
    //need to take mutexes here
    if( xSemaphoreTake(xUartTxMutex, ( TickType_t ) 1000) == pdTRUE )
    {
        if (device  == CLI)
        {
            HAL_UART_Transmit(&huart1, pData, bytesToSend, 0xffff);
        }
        else if (device == LOG)
        {
            HAL_UART_Transmit(&hlpuart1, pData, bytesToSend, 0xffff);
        }
        else if(device == CELLULAR )
        {
            HAL_UART_Transmit(&huart5, pData, bytesToSend, 0xff);
        }
        else if(device == GPS_MODULE )
        {
            HAL_UART_Transmit(&huart3, pData, bytesToSend, 0xff);
        }
        else if (device == SSM)
        {
            HAL_UART_Transmit(&huart4, pData, bytesToSend, 0xffff);
        }
        else
        {
            elogError("Invalid device");
        }

        xSemaphoreGive(xUartTxMutex);
    }
    else
    {
        elogError("Couldnt Take Mutex");
    }
}




void UART_sendDataBlockingSsm(UART_Periph_t device, uint8_t *pData, uint16_t bytesToSend)
{
    //double check that the caller is the SSM:
    if (device == SSM)
    {
        HAL_UART_Transmit_Uart4(&huart4, pData, bytesToSend, 0xffff);
    }
    else
    {
        elogError("Invalid device");
    }
}




uint32_t UART_sendDataNonBlocking(UART_Periph_t device, uint8_t *pData, uint16_t bytesToSend)
{
    HAL_StatusTypeDef stat = HAL_ERROR;

    //need to take mutexes here
    if( xSemaphoreTake(xUartTxMutex, ( TickType_t ) 200) == pdTRUE )
    {
        if (device  == CLI)
        {
            stat = HAL_UART_Transmit_IT(&huart1, pData, bytesToSend);
        }
        else if (device == LOG)
        {
            stat = HAL_UART_Transmit_IT(&hlpuart1, pData, bytesToSend);
        }
        else if(device == CELLULAR)
        {
            stat = HAL_UART_Transmit_IT(&huart5, pData, bytesToSend);
        }
        else if(device == GPS_MODULE)
        {
            stat = HAL_UART_Transmit_IT(&huart3, pData, bytesToSend);
        }
        else if (device == SSM)
        {
            stat = HAL_UART_Transmit_IT(&huart4, pData, bytesToSend);
        }
        else
        {
            elogError("Invalid device");
        }

        xSemaphoreGive(xUartTxMutex);
    }
    else
    {
        elogError("Couldnt Take Mutex");
    }

    return stat;
}

uint32_t UART_sendDataNonBlockingWithDma(UART_Periph_t device, uint8_t *pData, uint16_t bytesToSend)
{
    HAL_StatusTypeDef stat = HAL_ERROR;

    if (device == CELLULAR)
    {
        stat = HAL_UART_Transmit_DMA(&huart5, pData, bytesToSend);
    }
    else
    {
        elogError("No DMA channel for specified UART");
    }

    return stat;
}

void UART_recieveDataBlocking(UART_Periph_t device, uint8_t *pData, uint16_t bytesToRx)
{
    //add mutex if we ever add more devices to this function
    if (device == SSM)
    {
        //add ONE more to receive since we have a stop bit:
        HAL_UART_Receive_Uart4(&huart4, pData, bytesToRx+1, 1000);
    }
    else
    {
        elogError("UART driver is not set up for blocking calls on this Peripheral");
    }
}

void UART_recieveDataNonBlocking(UART_Periph_t device, uint8_t *pData, uint16_t bytesToRx)
{
    HAL_StatusTypeDef stat = HAL_OK;

    if (device  == CLI)
    {
        HAL_UART_Receive_IT(&huart1, pData, bytesToRx);
    }
    else if (device == LOG)
    {
        HAL_UART_Receive_IT(&hlpuart1, pData, bytesToRx);
    }
    else if(device == CELLULAR)
    {
        stat = HAL_UART_Receive_IT(&huart5, pData, bytesToRx);
        if( stat != HAL_OK)
        {
          NW_handleUartError();
        }
    }
    else if(device == GPS_MODULE)
    {
        HAL_UART_Receive_IT(&huart3, pData, bytesToRx);
    }
    else if (device == SSM)
    {
        HAL_UART_Receive_IT(&huart4, pData, bytesToRx);
    }
    else
    {
        elogError("Invalid Device ID");
    }
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
     if (HAL_UART_Init(&hlpuart1) != HAL_OK)
     {
         elogError("Failed to init UART");
     }
     if (HAL_UARTEx_SetTxFifoThreshold(&hlpuart1, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
     {
         elogError("Failed to init UART");
     }
     if (HAL_UARTEx_SetRxFifoThreshold(&hlpuart1, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
     {
         elogError("Failed to init UART");
     }
}

static void xInitUart1(void)
{
    /* Init USART 1 - for CLI */
    huart1.Instance = USART1;
    huart1.Init.BaudRate = 115200;
    huart1.Init.WordLength = UART_WORDLENGTH_8B;
    huart1.Init.StopBits = UART_STOPBITS_1;
    huart1.Init.Parity = UART_PARITY_NONE;
    huart1.Init.Mode = UART_MODE_TX_RX;
    huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart1.Init.OverSampling = UART_OVERSAMPLING_16;
    huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
    huart1.Init.ClockPrescaler = UART_PRESCALER_DIV1;
    huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;

    if (HAL_UART_Init(&huart1) != HAL_OK)
    {
        elogError("Failed to init UART");
    }
    if (HAL_UARTEx_SetTxFifoThreshold(&huart1, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
    {
        elogError("Failed to init UART");
    }
    if (HAL_UARTEx_SetRxFifoThreshold(&huart1, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
    {
        elogError("Failed to init UART");
    }
    if (HAL_UARTEx_DisableFifoMode(&huart1) != HAL_OK)
    {
        elogError("Failed to init UART");
    }
}

static void xInitUart5(void)
{
    /* DMA controller clock enable */
    __HAL_RCC_DMAMUX1_CLK_ENABLE();
    __HAL_RCC_DMA1_CLK_ENABLE();

    /* DMA interrupt init */
    /* DMA1_Channel1_IRQn interrupt configuration */
    HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);

    /* DMAMUX1_OVR_IRQn interrupt configuration */
    HAL_NVIC_SetPriority(DMAMUX1_OVR_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(DMAMUX1_OVR_IRQn);

    /* Init UART 5 */
    huart5.Instance = UART5;
    huart5.Init.BaudRate =  230400;
    huart5.Init.WordLength = UART_WORDLENGTH_8B;
    huart5.Init.StopBits = UART_STOPBITS_1;
    huart5.Init.Parity = UART_PARITY_NONE;
    huart5.Init.Mode = UART_MODE_TX_RX;
    huart5.Init.HwFlowCtl = UART_HWCONTROL_RTS_CTS;
    huart5.Init.OverSampling = UART_OVERSAMPLING_16;
    huart5.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
    huart5.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
    if (HAL_UART_Init(&huart5) != HAL_OK)
    {
        elogError("Failed to init UART");
    }

    /* Use DMA for transmitting */
    hdma_uart5_tx.Instance = DMA1_Channel1;
    hdma_uart5_tx.Init.Request = DMA_REQUEST_UART5_TX;
    hdma_uart5_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma_uart5_tx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_uart5_tx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_uart5_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_uart5_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_uart5_tx.Init.Mode = DMA_NORMAL;
    hdma_uart5_tx.Init.Priority = DMA_PRIORITY_MEDIUM;
    if (HAL_DMA_Init(&hdma_uart5_tx) != HAL_OK)
    {
        elogError("Failed to init UART");
    }

    //link DMA channel 1 to UART 5 tx interrupts
    __HAL_LINKDMA(&huart5, hdmatx, hdma_uart5_tx);
}

static void xInitUart3(void)
{
    /* Init USART 3 */
    huart3.Instance = USART3;
    huart3.Init.BaudRate = 9600; //default baud rate
    huart3.Init.WordLength = UART_WORDLENGTH_8B;
    huart3.Init.StopBits = UART_STOPBITS_1;
    huart3.Init.Parity = UART_PARITY_NONE;
    huart3.Init.Mode = UART_MODE_TX_RX;
    huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart3.Init.OverSampling = UART_OVERSAMPLING_8;
    huart3.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
    huart3.Init.ClockPrescaler = UART_PRESCALER_DIV1;
    huart3.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;

    if (HAL_UART_Init(&huart3) != HAL_OK)
    {
        elogError("Failed to init UART");
    }
    if (HAL_UARTEx_SetTxFifoThreshold(&huart3, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
    {
        elogError("Failed to init UART");
    }
    if (HAL_UARTEx_SetRxFifoThreshold(&huart3, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
    {
        elogError("Failed to init UART");
    }
    if (HAL_UARTEx_DisableFifoMode(&huart3) != HAL_OK)
    {
        elogError("Failed to init UART");
    }
}

static void xInitUart4(void)
{
    /* Init UART 4 */
    huart4.Instance = UART4;
    huart4.Init.BaudRate = 9600;
    huart4.Init.WordLength = UART_WORDLENGTH_9B;
    huart4.Init.StopBits = UART_STOPBITS_1;
    huart4.Init.Parity = UART_PARITY_EVEN;
    huart4.Init.Mode = UART_MODE_TX_RX;
    huart4.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart4.Init.OverSampling = UART_OVERSAMPLING_8;
    huart4.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
    huart4.Init.ClockPrescaler = UART_PRESCALER_DIV1;
    huart4.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;

    if (HAL_UART_Init(&huart4) != HAL_OK)
    {
        elogError("Failed to init UART");
    }
    if (HAL_UARTEx_SetTxFifoThreshold(&huart4, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
    {
        elogError("Failed to init UART");
    }
    if (HAL_UARTEx_SetRxFifoThreshold(&huart4, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
    {
        elogError("Failed to init UART");
    }
    if (HAL_UARTEx_DisableFifoMode(&huart4) != HAL_OK)
    {
        elogError("Failed to init UART");
    }
}


