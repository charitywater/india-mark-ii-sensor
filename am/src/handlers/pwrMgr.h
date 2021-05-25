/**************************************************************************************************
* \file     pwrMgr.h
* \brief    Enable STANDY mode, wakeup source and any pre or post settings around this low power mode
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

#ifndef HANDLERS_PWRMGR_H_
#define HANDLERS_PWRMGR_H_

#include "stdbool.h"

extern void PWR_init(void);
extern void PWR_enterStandbyMode(void);
extern bool PWR_turnOnGpsPowerSupply(void);
extern bool PWR_turnOffGpsPowerSupply(void);
extern bool PWR_turnOnCellModemPowerSupply(void);
extern bool PWR_turnOffCellModemPowerSupply(void);

extern bool PWR_getGpsPwrState(void);
extern bool PWR_getCellPwrState(void);

#endif /* HANDLERS_PWRMGR_H_ */
