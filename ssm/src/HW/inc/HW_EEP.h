/**************************************************************************************************
* \file     HW_EEP.h
* \brief    EEPROM driver. Interfaces with the CAT24C512WI
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

#ifndef HW_EEP_H
#define HW_EEP_H

#include "HW.h"

#define HW_EEP_SLAVE_ADDR                       0x50
#define HW_EEP_START_ADDR                       0x0000
#define HW_EEP_END_ADDR                         0xFFFF
#define HW_EEP_LAST_AVAILABLE_ADDR              (HW_EEP_END_ADDR - 1)   // One byte at end if EEP is used for r/w test.

extern void HW_EEP_Init(void);
extern void HW_EEP_DoTest(void);
extern uint8_t HW_EEP_ReadByte(uint16_t addr);
extern void HW_EEP_WriteByte(uint16_t addr, uint8_t value);
extern void HW_EEP_EraseAll(void);
extern void HW_EEP_WriteBlock(uint16_t addr, uint8_t * p_data, uint8_t num_bytes);

#endif /* HW_EEP_H */
