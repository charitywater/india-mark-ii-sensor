/*
 * File: getMaxUsageTime.c
 *
 * MATLAB Coder version            : 5.0
 * C/C++ source code generated on  : 18-May-2021 11:59:14
 */

/* Include Files */
#include "getMaxUsageTime.h"

/* Custom Source Code */

/* Copyright Notice
 * Copyright 2021 charity: water
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/* Function Definitions */

/*
 * Arguments    : const pumpUsage_t *pump_usage
 *                uint8_T day
 * Return Type  : uint8_T
 */
uint8_T getMaxUsageTime(const pumpUsage_t *pump_usage, uint8_T day)
{
  uint8_T hour;

  /*  Return the max usage hour if we have one */
  /*  It is based on the previous week's usage for this day so that we can */
  /*  proactive and hopefully turn on pump health computation during the hour */
  /*  that will have the maximum usage */
  hour = 8U;
  if (pump_usage->is_filling_hourly_usage == 0) {
    if (pump_usage->is_filling_daily_usage != 0) {
      hour = pump_usage->prev_day_max_usage_hour;
    } else {
      if (day < 7) {
        hour = pump_usage->daily_usage[day];
      }
    }
  }

  return hour;
}

/*
 * File trailer for getMaxUsageTime.c
 *
 * [EOF]
 */
