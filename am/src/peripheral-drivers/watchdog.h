/**************************************************************************************************
* \file     watchdog.h
* \brief    Driver to init and refresh the system watchdog (WWDG peripheral)
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

#ifndef PERIPHERAL_DRIVERS_WATCHDOG_H_
#define PERIPHERAL_DRIVERS_WATCHDOG_H_

extern void WD_init(void);
extern void WD_refresh(void);
extern bool WD_recoveredFromReset(void);
extern uint32_t WD_getRefreshRateMs(void);

#endif /* PERIPHERAL_DRIVERS_WATCHDOG_H_ */
