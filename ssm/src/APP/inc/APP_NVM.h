/**************************************************************************************************
* \file     APP_NVM.h
* \brief    Initialize, validate and update data stored in NVM
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

#ifndef APP_NVM_H
#define APP_NVM_H

#include "APP_NVM_Cfg.h"
#include "APP_NVM_Custom.h"

extern void APP_NVM_Init(void);
extern void APP_NVM_Validate(void);
extern void APP_NVM_UpdateCurrentEntry(uint8_t map_index, uint8_t * p_data_to_write, bool bump_addr);
extern void APP_NVM_ReadCurrentEntry(uint8_t map_index, uint8_t * p_data_buffer);
extern void APP_NVM_DefaultAll(void);
extern void APP_NVM_DefaultSensorDataLogs(void);
extern void APP_NVM_ReadSectionHeader(uint8_t map_index, APP_NVM_SECTION_HDR_T * p_hdr);
extern void APP_NVM_ReadBytes(uint16_t addr, uint16_t num_bytes, uint8_t * p_bytes);
extern uint8_t APP_NVM_ComputeChecksum(uint8_t * p_data, uint16_t num_bytes);
extern void APP_NVM_DefaultSection(uint8_t map_index);

#endif /* APP_NVM_H */
