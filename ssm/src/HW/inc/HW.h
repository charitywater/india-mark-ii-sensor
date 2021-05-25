/**************************************************************************************************
* \file     HW.h
* \brief    Hardware - Top level functionality and initialization
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

#ifndef HW_H
#define HW_H

#include "CAPT_UserConfig.h"
#include "APP_NVM_Cfg_Shared.h"

#define BIT_0       0x01
#define BIT_1       0x02
#define BIT_2       0x04
#define BIT_3       0x08
#define BIT_4       0x10
#define BIT_5       0x20
#define BIT_6       0x40
#define BIT_7       0x80
#define BIT_8       0x100
#define BIT_9       0x200
#define BIT_10      0x400
#define BIT_11      0x800
#define BIT_12      0x1000
#define BIT_13      0x2000
#define BIT_14      0x4000
#define BIT_15      0x8000
#define BIT_16      0x10000
#define BIT_17      0x20000
#define BIT_18      0x40000

// If STREAM_ENGINEERING_DATA is defined, UART baud rate will change to 19200

 // Define this if you want engineering data to go to the terminal instead of EEPROM.
//#define STREAM_ENGINEERING_DATA

#ifdef STREAM_ENGINEERING_DATA
#ifndef ENGINEERING_DATA
#error "Must define ENGINEERING_DATA if using STREAM_ENGINEERING_DATA"
#endif
#endif

#if (defined(BUILD_HW_TERM) && (CAPT_INTERFACE == __CAPT_UART_INTERFACE__))
#error "Can't build HW_TERM and set CAPT_INTERFACE to __CAPT_UART_INTERFACE__"
#endif

extern void HW_Init(void);
extern void HW_PerformSwReset(void);

#endif /* HW_H */
