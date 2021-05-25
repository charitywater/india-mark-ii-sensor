/**************************************************************************************************
* \file     uart.h
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
#ifndef PERIPHERAL_DRIVERS_UART_H_
#define PERIPHERAL_DRIVERS_UART_H_

#include "stdint.h"

//add more if needed in the future
typedef enum
{
    LOG,
    CLI,
    CELLULAR,
    GPS_MODULE,
    SSM,
}UART_Periph_t;

extern void UART_initPeripherals(void);
extern void UART_deinitDebugPeripherals(void);
extern void UART_initCellUart(void);
extern void UART_initGpsUart(void);
extern void UART_deinitGpsUart(void);
extern void UART_initCliUart(void);
extern void UART_sendDataBlocking(UART_Periph_t device, uint8_t *pData, uint16_t bytesToSend);
extern uint32_t UART_sendDataNonBlocking(UART_Periph_t device, uint8_t *pData, uint16_t bytesToSend);
extern uint32_t UART_sendDataNonBlockingWithDma(UART_Periph_t device, uint8_t *pData, uint16_t bytesToSend);
extern void UART_recieveDataBlocking(UART_Periph_t device, uint8_t *pData, uint16_t bytesToRx);
extern void UART_recieveDataNonBlocking(UART_Periph_t device, uint8_t *pData, uint16_t bytesToRx);

//This function uses a set of HAL functions that are not called by any other uart peripheral,
//making them thread safe w/o taking a mutex
extern void UART_sendDataBlockingSsm(UART_Periph_t device, uint8_t *pData, uint16_t bytesToSend);

#endif /* PERIPHERAL_DRIVERS_UART_H_ */
