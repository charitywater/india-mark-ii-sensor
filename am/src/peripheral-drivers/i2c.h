/**************************************************************************************************
* \file     i2c.h
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

#ifndef PERIPHERAL_DRIVERS_I2C_H_
#define PERIPHERAL_DRIVERS_I2C_H_

#include "stdint.h"

//add more if needed in the future
typedef enum
{
    ATE,
}I2C_Device_t;

typedef enum
{
    I2C_SUCCESS,
    I2C_ERROR,
    I2C_TIMEOUT,
    I2C_INV_DEVICE,
}I2C_Err_t;

extern I2C_Err_t I2C_Init(void);
extern I2C_Err_t I2C_sendIsDeviceReady(I2C_Device_t device, uint16_t timeout, uint8_t tries);
extern I2C_Err_t I2C_writeToMemAddr(I2C_Device_t device, uint16_t memAddress, uint16_t memSize, uint8_t *pDataToWrite, uint16_t dataLen);
extern I2C_Err_t I2C_readFromMemAddr(I2C_Device_t device, uint16_t memAddress, uint16_t memSize, uint8_t *pReadBuffer, uint16_t readByteLen);
extern I2C_Err_t I2C_receive(I2C_Device_t device, uint8_t *buf, uint16_t memSize, uint16_t timeout);
extern I2C_Err_t I2C_transmit(I2C_Device_t device, uint8_t *buf, uint16_t memSize, uint16_t timeout);

#endif /* PERIPHERAL_DRIVERS_I2C_H_ */
