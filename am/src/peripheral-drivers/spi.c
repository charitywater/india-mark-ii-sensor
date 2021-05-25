/**************************************************************************************************
* \file     spi.c
* \brief    SPI API w/ CS control
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

#include "stdint.h"
#include "stddef.h"
#include "logTypes.h"
#include <stm32l4xx_hal.h>
#include "string.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "spi.h"
#include "task.h"
#include "ssm.h"
#include "am-ssm-spi-protocol.h"
#include "CLI.h"

#define MAX_SSM_WAIT_TIME_MS       600

SPI_HandleTypeDef hspi1;
SPI_HandleTypeDef hspi2;

SemaphoreHandle_t xSPITransferMutex;

static void xEnableNandCommunication(void);
static void xDisableNandCommunication(void);

static void xEnableSsmCommunication(void);
static void xDisableSsmCommunication(void);

static void xInitNandSpi(void);
static void xInitSsmSpi(void);

static bool WaitForSSMReady(uint16_t max_wait_ms);

/* init spi peripherals */
 void SPI_Init(void)
{
    xInitNandSpi();
    xInitSsmSpi();

    /* create a mutex */
    xSPITransferMutex = xSemaphoreCreateMutex();
}

/*******************************************************************************
Send data to the NAND flash chip over spi. Also receivev data is an rx len and
buffer is provided.
*******************************************************************************/
 spiStatus_t SPI_nandTransfer(const spiData_t* pDataToSend, spiData_t* pDataReceived,
         spiConfigOptions_t optAfter)
{
    uint8_t *dataSend;
    uint8_t *dataRecv;
    uint16_t rxLen = 0;
    uint16_t txLen = 0;
    HAL_StatusTypeDef stat = HAL_ERROR;

    txLen = pDataToSend->length;
    dataSend = pDataToSend->pChar;

    if (NULL != pDataReceived)
    {
        rxLen = pDataReceived->length;
        dataRecv = pDataReceived->pChar;
    }

    xEnableNandCommunication();

    /* Take mutex */
    if( xSemaphoreTake(xSPITransferMutex, ( TickType_t ) 200) == pdTRUE )
    {

        /* Send the spi command/data */
        stat = HAL_SPI_Transmit(&hspi1, dataSend, txLen, 0xffffff );

        if (stat == HAL_OK)
        {
            if (rxLen > 0)
            {
                // now read the actual data
                stat = HAL_SPI_Receive(&hspi1, dataRecv, rxLen, 0xffffff);
            }
        }

        /* Return mutex */
        xSemaphoreGive(xSPITransferMutex);
    }

    /* only clear cs if it is the end of a transfer */
    if ( optAfter == OpsEndTransfer )
    {
        xDisableNandCommunication();
    }

    /* check the spi code */
    if(stat != HAL_OK)
    {
        return spiError;
    }

    return spiSuccess;
}


/*******************************************************************************
Wait up to the specified mS for the SSM to assert WAKE_AP.  Return true
if the SSM is ready.
*******************************************************************************/
static bool WaitForSSMReady(uint16_t max_wait_ms)
{
    uint16_t ms_waited = 0u;
    bool ready = false;
    volatile GPIO_PinState wake_ap;

    do
    {
        vTaskDelay(1);
        ms_waited++;
        wake_ap = HAL_GPIO_ReadPin(SSM_RDY_PORT, SSM_RDY_PIN);

        if (wake_ap == GPIO_PIN_RESET)
        {
            ready = true;
        }

    }while((ms_waited < max_wait_ms) &&
           (ready == false));

    return ready;
}

 /*******************************************************************************
 Send data to the SSM over spi. Also receive data
 *******************************************************************************/
spiStatus_t SPI_ssmTransfer(const spiData_t* pDataToSend, spiData_t* pDataReceived,
      spiConfigOptions_t optAfter)
 {
     uint8_t *dataSend;
     uint8_t *dataRecv;
     uint16_t rxLen = 0;
     uint16_t txLen = 0;
     HAL_StatusTypeDef stat = HAL_ERROR;

     /* Take mutex */
     if( xSemaphoreTake(xSPITransferMutex, ( TickType_t ) 10000) == pdTRUE )
     {
         txLen = pDataToSend->length;
         dataSend = pDataToSend->pChar;

         if (NULL != pDataReceived)
         {
             rxLen = pDataReceived->length;
             dataRecv = pDataReceived->pChar;
         }

         xEnableSsmCommunication();

         /* Send the spi command/data */
         stat = HAL_SPI_Transmit(&hspi2, dataSend, txLen, 0xffff );

         if (stat == HAL_OK)
         {
             if (rxLen > 0)
             {

                vTaskDelay(1);

                // Usually these are fairly fast responses but there does some to be some responses after power up
                // likely due to capsense interrupts as the lib starts up.
               if (WaitForSSMReady(MAX_SSM_WAIT_TIME_MS) == true)
                {
                    // now read the actual data
                    stat = HAL_SPI_Receive(&hspi2, dataRecv, rxLen, 0xffff);

                    //check timeout on this receive to verify we received actual data.
                    if (stat != HAL_OK)
                    {
                        rxLen = 0u;
                        elogError("No response from SSM");
                    }
                }
               else
                {
                    //Application will decide how to handle this
                    elogError("No response from SSM - did not indicate ready to send");
                    rxLen = 0;  // Nothing came in so nothing to unpack.
                    stat = HAL_TIMEOUT;
                }
             }
         }

         /* Return mutex */
         xSemaphoreGive(xSPITransferMutex);
     }
     else
     {
         elogError("Failed to take mutex");
     }

     /* only clear cs if it is the end of a transfer */
     if ( optAfter == OpsEndTransfer )
     {
         xDisableSsmCommunication();
     }

     /* check the spi code - retry X number of times */
     if(stat != HAL_OK)
     {
         elogError("SPI status %d", stat);
         if ( stat == HAL_TIMEOUT )
             return spiTimeout;
         else
             return spiError;
     }

     return spiSuccess;
 }

 static void xInitNandSpi(void)
 {
     GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* SPI1 parameter configuration*/
    hspi1.Instance = SPI1;
    hspi1.Init.Mode = SPI_MODE_MASTER;
    hspi1.Init.Direction = SPI_DIRECTION_2LINES;
    hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
    hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
    hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
    hspi1.Init.NSS = SPI_NSS_SOFT;
    hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
    hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
    hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
    hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    hspi1.Init.CRCPolynomial = 0x0;
    hspi1.Init.NSSPMode = SPI_NSS_PULSE_DISABLE;
    if (HAL_SPI_Init(&hspi1) != HAL_OK)
    {
        // log error
    }

    /* init chip select */
    HAL_GPIO_WritePin(MT28F1_CS_PORT, MT28F1_CS_PIN, GPIO_PIN_SET);

    /*Configure GPIO pin : PA4 */
    GPIO_InitStruct.Pin = MT28F1_CS_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(MT28F1_CS_PORT, &GPIO_InitStruct);
 }

static void xInitSsmSpi(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* SPI1 parameter configuration*/
    hspi2.Instance = SPI2;
    hspi2.Init.Mode = SPI_MODE_MASTER;
    hspi2.Init.Direction = SPI_DIRECTION_2LINES;
    hspi2.Init.DataSize = SPI_DATASIZE_8BIT;
    hspi2.Init.CLKPolarity = SPI_POLARITY_LOW;
    hspi2.Init.CLKPhase = SPI_PHASE_1EDGE;
    hspi2.Init.NSS = SPI_NSS_SOFT;
    hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_256;
    hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
    hspi2.Init.TIMode = SPI_TIMODE_DISABLE;
    hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    hspi2.Init.CRCPolynomial = 0x0;
    hspi2.Init.NSSPMode = SPI_NSS_PULSE_DISABLE;
    if (HAL_SPI_Init(&hspi2) != HAL_OK)
    {
        // log error
    }

    /*Configure GPIO pin : PB12  as an input pin.
     * Not using CS as there is only 1 SPI slave */
    GPIO_InitStruct.Pin = SSM_RDY_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(SSM_RDY_PORT, &GPIO_InitStruct);
}

static void xEnableNandCommunication(void)
{
    /* clear cs */
    HAL_GPIO_WritePin(MT28F1_CS_PORT, MT28F1_CS_PIN, GPIO_LOW);
}

static void xDisableNandCommunication(void)
{
    /* set cs */
    HAL_GPIO_WritePin(MT28F1_CS_PORT, MT28F1_CS_PIN, GPIO_HIGH);
}

static void xEnableSsmCommunication(void)
{
    /* clear cs if a CS line is used */
}

static void xDisableSsmCommunication(void)
{
    /* set cs if a CS line is used */
}
