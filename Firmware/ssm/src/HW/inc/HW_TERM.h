/**************************************************************************************************
* \file     HW_TERM.h
* \brief    Hardware - Send and receive characters over UART for CLI and logging.
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

#ifndef HW_TERM_H
#define HW_TERM_H

#include "HW.h"
#include <stdint.h>
#include <stdbool.h>


extern void HW_TERM_Init(void);
extern void HW_TERM_Print(uint8_t * p_str);
//extern void HW_TERM_PrintColor(uint8_t * p_str, uint8_t * p_color);
extern void HW_TERM_GetCommand(uint8_t * p_buff);
extern bool HW_TERM_CommandRdy(void);
extern void HW_TERM_ReportPadValues(void);
extern void HW_TERM_RxByte(uint8_t byte);
extern void HW_TERM_DisableOrEnableLogging(bool isEnabled);
extern bool HW_TERM_IsLoggingEnabled(void);
extern void HW_TERM_PrintVersionInfo(void);

#define HW_TERM_RX_BUF_LEN  30

#endif /* HW_TERM_H */
