/**************************************************************************************************
* \file     i2c.c
* \brief    Driver to send and receive I2C data. Manages all of the I2C peripherals and is built on
*           top of the HAL i2c driver to make it thread safe
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

#include <stm32l4xx_hal.h>
#include "stddef.h"
#include "logTypes.h"
#include "string.h"
#include "stdbool.h"
#include "logTypes.h"
#include "ATECC608A.h"
#include "i2c.h"

#define REVOVER_BUS_TOGGLES         32
#define RECOVER_BUS_DELAY_TICKS     800

// these are used elsewhere so they are not static
I2C_HandleTypeDef hi2c2;

static bool i2cInitialized = false;

static void xinitAtecc608aBus(void);
static void xHandleBusError(I2C_Device_t device);

I2C_Err_t I2C_Init(void)
{
    xinitAtecc608aBus();

    i2cInitialized = true;

    return I2C_SUCCESS;
}

I2C_Err_t I2C_sendIsDeviceReady(I2C_Device_t device, uint16_t timeout, uint8_t tries)
{
    HAL_StatusTypeDef halResult = HAL_OK;
    I2C_Err_t i2cErr = I2C_SUCCESS;

    // need to take mutex here if more than 1 device is added
    if ( device == ATE )
    {
       halResult = HAL_I2C_IsDeviceReady(&hi2c2, 0x00, tries, timeout);
    }
    else
    {
        //invalid device name
        i2cErr = I2C_INV_DEVICE;
    }

   //check errors
    if ( halResult == HAL_ERROR )
    {
       xHandleBusError(device);
       i2cErr = I2C_ERROR;
    }
    else if ( halResult == HAL_TIMEOUT || halResult == HAL_BUSY )
    {
       i2cErr = I2C_TIMEOUT;
    }

   return i2cErr;
}

I2C_Err_t I2C_receive(I2C_Device_t device, uint8_t *buf, uint16_t memSize, uint16_t timeout)
{
    HAL_StatusTypeDef halResult = HAL_OK;
    I2C_Err_t i2cErr = I2C_SUCCESS;

    // need to take mutex here if more than 1 device is added
    if ( device == ATE )
    {
        halResult = HAL_I2C_Master_Receive(&hi2c2, ATECC_DEVICE_ID, buf, memSize, timeout);
    }
    else
    {
        //invalid device name
        i2cErr = I2C_INV_DEVICE;
    }

    if ( halResult == HAL_ERROR )
    {
        xHandleBusError(device);
        i2cErr = I2C_ERROR;
    }
    else if ( halResult == HAL_TIMEOUT || halResult == HAL_BUSY )
    {
        i2cErr = I2C_TIMEOUT;
    }

    return i2cErr;
}


I2C_Err_t I2C_transmit(I2C_Device_t device, uint8_t *buf, uint16_t memSize, uint16_t timeout)
{
    HAL_StatusTypeDef halResult = HAL_OK;
    I2C_Err_t i2cErr = I2C_SUCCESS;

    // need to take mutex here if more than 1 device is added
    if ( device == ATE )
    {
        halResult = HAL_I2C_Master_Transmit(&hi2c2, ATECC_DEVICE_ID, buf, memSize, timeout);
    }
    else
    {
        //invalid device name
        i2cErr = I2C_INV_DEVICE;
    }

    if ( halResult == HAL_ERROR )
    {
        xHandleBusError(device);
        i2cErr = I2C_ERROR;
    }
    else if ( halResult == HAL_TIMEOUT || halResult == HAL_BUSY )
    {
        i2cErr = I2C_TIMEOUT;
    }

    return i2cErr;
}

I2C_Err_t I2C_writeToMemAddr(I2C_Device_t device, uint16_t memAddress, uint16_t memSize, uint8_t *pDataToWrite, uint16_t dataLen)
{
    HAL_StatusTypeDef result;

    // need to take mutex here if more than 1 device is added
    if (device == ATE)
    {
         result = HAL_I2C_Mem_Write(&hi2c2, ATECC_DEVICE_ID, memAddress, memSize, pDataToWrite, dataLen,  0xffff);
    }
    else
    {
        //invalid device name
        result = HAL_ERROR;
    }

    if (result != HAL_OK)
    {
        return I2C_ERROR;
    }
    else
    {
        return I2C_SUCCESS;
    }
}

I2C_Err_t I2C_readFromMemAddr(I2C_Device_t device, uint16_t memAddress, uint16_t memSize, uint8_t *pReadBuffer, uint16_t readByteLen)
{
    HAL_StatusTypeDef result;

    // need to take mutex here if more than 1 device is added
    if (device == ATE)
    {
        result = HAL_I2C_Mem_Read(&hi2c2, ATECC_DEVICE_ID, memAddress, memSize, pReadBuffer, readByteLen,  0xffff);
    }
    else
    {
        //invalid device name
        result = HAL_ERROR;
    }

    if (result != HAL_OK)
    {
        return I2C_ERROR;
    }
    else
    {
        return I2C_SUCCESS;
    }
}


static void xHandleBusError(I2C_Device_t device)
{
    if ( device == ATE)
    {
        elogDebug("Resetting i2c 2 periph");
        HAL_I2C_DeInit(&hi2c2);
        HAL_I2C_Init(&hi2c2);
    }
    else
    {
        //invalid device
    }

}

static void xinitAtecc608aBus(void)
{
    uint8_t toggles = 0;
    uint16_t toggleDelay = 0;

    //Init the i2c pins first as GPIO - pin 11 is data line
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin =  GPIO_PIN_11;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    //pin 10 is output (clock line)
    GPIO_InitStruct.Pin =  GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    //Now read the data line, if it is LOW then the slave is holding it low
    if ( HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_11) == 0 )
    {
        //Manually toggle the clock line to get the i2c slave to release the
        //data line
        for(toggles = 0; toggles < REVOVER_BUS_TOGGLES; toggles++)
        {
            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, 0);
            //small delay
            for(toggleDelay = 0; toggleDelay< RECOVER_BUS_DELAY_TICKS; toggleDelay++) ;
            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, 1);
            //small delay
            for(toggleDelay =0; toggleDelay < RECOVER_BUS_DELAY_TICKS; toggleDelay++) ;
        }
    }

    //Now init as I2C - these settings were generated using STM32 CubeMX
    hi2c2.Instance = I2C2;
    hi2c2.Init.Timing = 0x00901954;
    hi2c2.Init.OwnAddress1 = 0x10;
    hi2c2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c2.Init.OwnAddress2 = 0x11;
    hi2c2.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
    hi2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c2.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
    if (HAL_I2C_Init(&hi2c2) != HAL_OK)
    {
        //TODO: pass up to app code
        elogError("Failed to init i2c for crypto");
    }

    /** Configure Analog filter
    */
    if (HAL_I2CEx_ConfigAnalogFilter(&hi2c2, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
    {
        elogError("Failed to enable i2c filter");
    }
}
