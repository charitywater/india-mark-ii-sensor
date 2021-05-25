/**************************************************************************************************
* \file     APP.h
* \brief    Top level application
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

#ifndef APP_H
#define APP_H

#include "HW.h"
#include "am-ssm-spi-protocol.h"

#define FIVE_MINUTES                    5*60

extern void APP_init(void);
extern void APP_handleAttnSourceRequest(void);
extern void APP_handleAttnSourceAck(asp_attn_source_payload_t *pMsg);
extern void APP_indicateActivation(void);
extern void APP_indicateError(uint32_t errorBit);
extern void APP_indicateNeedTime(void);
extern void APP_indicateCheckIn(void);
extern void APP_handleActivateCmd(void);
extern void APP_handleDeactivateCmd(void);
extern void APP_handleIncrementSensorDataCmd(void);
extern void APP_indicateInvalidSpiMsg(void);
extern app_state_t APP_getState(void);
extern reset_state_t APP_getResetState(void);
extern uint32_t APP_getErrorBits(void);
extern void APP_indicateErrorResolved(uint32_t errorBit);
extern void APP_periodic(void);
extern void APP_setPumpActive(bool active);
extern bool APP_getPumpActive(void);
extern void APP_setTimeUpdated(void);
extern void APP_setTimeFailed(void);
extern void APP_setTimeSyncStatus(uint8_t status);
extern void APP_handleHwResetCommand(void);
extern void APP_handleResetAlarmsCommand(void);
extern void APP_handleConfigs(uint32_t transmissRate, bool strokeAlgIsOn, uint16_t redFlagOnThresh, uint16_t redFlagOffThresh);
extern void APP_setNewRedFlagDetected(bool isNewRedFlagPresent);
extern void APP_indicateMagnetometerThresholdInterrupt(void);

#endif /* APP_H */
