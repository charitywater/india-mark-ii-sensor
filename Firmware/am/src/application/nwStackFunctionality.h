/**************************************************************************************************
* \file     nwStackFunctionality.c
* \brief    Init and handle uart routing of ppp/lwip, parse AT commands for responses
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

#ifndef APPLICATION_NWSTACKFUNCTIONALITY_H_
#define APPLICATION_NWSTACKFUNCTIONALITY_H_

extern void NW_processCellRxByte(void);
extern void NW_txComplete(void);
extern void NW_initLwip(void);
extern void NW_handleUartError(void);
extern void NW_initUart(void);
extern uint32_t NW_getRssiValue(void);
extern uint32_t NW_getCellOnTimeMs(void);
extern void ATcommandModeParsing_Task(void);
extern void NW_timeSyncRequested(bool flag);
extern uint64_t NW_getImeiOfModem(void);
#endif /* APPLICATION_NWSTACKFUNCTIONALITY_H_ */
