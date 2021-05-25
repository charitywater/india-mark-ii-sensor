/**************************************************************************************************
* \file     HW_RTC.h
* \brief    Real-time clock functionality for the M41T62Q6F part
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

#ifndef HW_RTC_H
#define HW_RTC_H

#include "HW.h"

#define HW_RTC_SLAVE_ADDR                       0x68
#define MAX_HOUR_VALUE                          23 //0-23 = 24 hours

typedef enum HW_RTC_DAY_OF_WEEK
{
    HW_RTC_SUN = 1u,
    HW_RTC_MON = 2u,
    HW_RTC_TUE = 3u,
    HW_RTC_WED = 4u,
    HW_RTC_THU = 5u,
    HW_RTC_FRI = 6u,
    HW_RTC_SAT = 7u
}HW_RTC_DAY_OF_WEEK_T;

typedef enum HW_RTC_MONTH
{
    HW_RTC_JAN = 1u,
    HW_RTC_FEB = 2u,
    HW_RTC_MAR = 3u,
    HW_RTC_APR = 4u,
    HW_RTC_MAY = 5u,
    HW_RTC_JUN = 6u,
    HW_RTC_JLY = 7u,
    HW_RTC_AUG = 8u,
    HW_RTC_SEP = 9u,
    HW_RTC_OCT = 10u,
    HW_RTC_NOV = 11u,
    HW_RTC_DEC = 12u
}HW_RTC_MONTH_T;

extern void HW_RTC_Init(void);
extern void HW_RTC_Monitor(void);
extern bool HW_RTC_SetTime(uint8_t _10_ms,
                           uint8_t seconds,
                           uint8_t minutes,
                           uint8_t hours,
                           HW_RTC_DAY_OF_WEEK_T day_of_week,
                           uint8_t date,
                           HW_RTC_MONTH_T month,
                           uint16_t year);
extern void HW_RTC_ReportTime(void);
extern bool HW_RTC_GetTime(uint8_t * _10_ms,
                    uint8_t * seconds,
                    uint8_t * minutes,
                    uint8_t * hours,
                    HW_RTC_DAY_OF_WEEK_T * day_of_week,
                    uint8_t * date,
                    HW_RTC_MONTH_T * month,
                    uint16_t * year);
extern void HW_RTC_GetHour(uint8_t * hour);
extern uint32_t HW_RTC_GetSecondsSinceMidnight(void);
extern void HW_RTC_GetSecToNextHour(uint16_t * secondsToNextHour);
extern uint32_t HW_RTC_GetEpochTime(void);
extern bool HW_RTC_SetTimeEpoch(uint32_t epoch_time);
extern bool HW_RTC_CheckValidTime(void);

#endif /* HW_RTC_H */
