/*
 * File: computePumpHealth.c
 *
 * MATLAB Coder version            : 5.0
 * C/C++ source code generated on  : 27-Oct-2022 08:13:43
 */

/* Include Files */
#include "computePumpHealth.h"

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
 * Arguments    : const hourlyWaterInfo_t *hourly_water_info
 *                const hourlyStrokeInfo_t *hourly_stroke_info
 *                hourlyPumpHealthInfo_t *pump_health_info
 * Return Type  : void
 */
void computePumpHealth(const hourlyWaterInfo_t *hourly_water_info, const
  hourlyStrokeInfo_t *hourly_stroke_info, hourlyPumpHealthInfo_t
  *pump_health_info)
{
  uint32_T norm_stroke_count;
  uint32_T b_hourly_water_info;

  /*  Check inputs */
  pump_health_info->pump_capacity = 0UL;
  pump_health_info->quality_factor = 0L;
  if (hourly_stroke_info->mag_calibration_changed == 0) {
    /*  Only compute pump health if we have magnetometer calibration for the full hour */
    /*  Divide the stroke count by 2 to get to full strokes (full cycle */
    /*  down-up-down or up-down-up) */
    /*  Compute the normalized stroke count  */
    /*  NOTE: avg_displacement is in percent */
    norm_stroke_count = (uint32_T)((uint64_T)
      (hourly_stroke_info->wet_stroke_count >> 1) *
      hourly_stroke_info->wet_stroke_avg_displacement) / 100UL;

    /*  Compute the pump capacity */
    if ((int32_T)norm_stroke_count > 0L) {
      if ((uint32_T)hourly_water_info->volume < 4294967UL) {
        b_hourly_water_info = (uint32_T)((uint32_T)hourly_water_info->volume *
          1000ULL);
      } else {
        b_hourly_water_info = 4294967UL;
      }

      pump_health_info->pump_capacity = b_hourly_water_info / norm_stroke_count;

      /*  Will output pump capacity * 1000 (3 decimal places of precision) */
      pump_health_info->quality_factor = hourly_water_info->percent_pump_usage;
    } else {
      pump_health_info->pump_capacity = 0UL;
      pump_health_info->quality_factor = 0L;
    }
  }
}

/*
 * File trailer for computePumpHealth.c
 *
 * [EOF]
 */
