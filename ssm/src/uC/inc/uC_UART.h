/**************************************************************************************************
* \file     uC_UART.h
* \brief    UART peripheral driver
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

#ifndef UC_UART_H
#define UC_UART_H

#include <stdint.h>
#include "uC.h"

extern void uC_UART_Init(void);
extern void uC_UART_Tx(uint8_t * p_buf, uint16_t len);

#endif /* UC_UART_H */
