/**************************************************************************************************
* \file     uC_TIME.h
* \brief    MSP timer peripheral driver
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

#ifndef UC_TIME_H
#define UC_TIME_H

#include "uC.h"
#include <stdint.h>
#include "stdbool.h"

#define MIN_PER_HOUR                    60lu
#define HOUR_PER_DAY                    24lu
#define DAY_PER_YEAR                    365lu
#define SEC_PER_MIN                     60lu
#define SEC_PER_HOUR                    (SEC_PER_MIN * MIN_PER_HOUR) // 3600
#define SEC_PER_DAY                     (SEC_PER_HOUR * HOUR_PER_DAY) // 86400
#define SEC_PER_YEAR                    (SEC_PER_DAY * DAY_PER_YEAR) // 3,153,600

#define MS_PER_S                        (1000u)

// Clock is running at 32768, meaning 3277 cycles for 100mS
#define UC_TIME_100_MS_IN_CYCLES        (3277u)
#define UC_TIME_10_MS_IN_CYCLES         (328u)
#define UC_TIME_1000_MS_IN_CYCLES       (32768u - 1)
#define UC_TIMER_TICK_TIME_MS           (10u)
#define UC_TIME_TICKS_PER_S             (MS_PER_S/UC_TIMER_TICK_TIME_MS)
#define UC_TIME_TICKS_PER_HR            (UC_TIME_TICKS_PER_S*SEC_PER_HOUR)
#define UC_TIME_TICKS_PER_100MS         (UC_TIME_TICKS_PER_S / 10u)
#define UC_TIME_TICKS_PER_50MS          (5u)

#define DATA_COLLECTION_FREQ_HZ         (1000/(UC_TIME_TICKS_PER_50MS * UC_TIMER_TICK_TIME_MS))

extern void uC_TIME_Init(void);
extern uint64_t uC_TIME_GetRuntimeTicks(void);
extern uint32_t uC_TIME_GetRuntimeSeconds(void);
extern void uC_TIME_SetRuntime(uint32_t seconds);
extern void HW_WatchdogStopKick(bool stop);
extern void uC_TIME_SetHourlyTimeAdjustSeconds(int32_t secs);
extern int32_t uC_TIME_GetHourlyTimeAdjustSeconds(void);

#endif /* UC_TIME_H */
