/**************************************************************************************************
* \file     sara_u201.h
* \brief    API to issue commands to the Ublox Sara-u201 cellular module
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

#ifndef DEVICE_DRIVERS_SARA_U201_H_
#define DEVICE_DRIVERS_SARA_U201_H_

extern void SARA_initHardware(void);
extern void SARA_initDataMode(void);
extern void SARA_getSimId(void);
extern void SARA_getRssi(void);
extern void SARA_getImei(void);
extern void SARA_get_Iccid(void);
extern void SARA_getModemVersion(void);
extern void SARA_startTxTest(uint32_t channel, int8_t power);
extern void SARA_turnOnSequence(void);
extern void SARA_initAndSetApn(void);
extern void SARA_sendNwRegistrationCmd(void);

#endif /* DEVICE_DRIVERS_SARA_U201_H_ */
