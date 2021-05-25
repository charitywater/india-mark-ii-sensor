/**************************************************************************************************
* \file     uC.c
* \brief    Microcontroller - Top level peripheral functionality.
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
* \date     01/29/2021
* \author   Twisthink
*
***************************************************************************************************/

#include "uC.h"
#include "uC_I2C.h"
#include "uC_UART.h"
#include "uC_TIME.h"
#include "uC_SPI.h"
#include "uC_ADC.h"

void uC_Init(void);


void uC_Init(void)
{
    uC_TIME_Init();
    uC_UART_Init();
    uC_SPI_Init();
    uC_I2C_Init();
    uC_ADC_Init();
}
