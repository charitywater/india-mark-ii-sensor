/*
 * File: hourlyStrokeCount.c
 *
 * MATLAB Coder version            : 5.0
 * C/C++ source code generated on  : 27-Oct-2022 08:13:26
 */

/* Include Files */
#include "hourlyStrokeCount.h"

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
 * Arguments    : accumStrokeCount_t *accum_stroke_count
 *                hourlyStrokeInfo_t *hourly_stroke_info
 * Return Type  : void
 */
void hourlyStrokeCount(accumStrokeCount_t *accum_stroke_count,
  hourlyStrokeInfo_t *hourly_stroke_info)
{
  hourly_stroke_info->windows_processed =
    accum_stroke_count->num_windows_processed;
  hourly_stroke_info->mag_calibration_changed =
    accum_stroke_count->mag_calibration_changed;

  /*  Wet Strokes */
  hourly_stroke_info->wet_stroke_count =
    accum_stroke_count->wet_stroke_count_sum;
  if (accum_stroke_count->wet_stroke_count_sum > 0U) {
    hourly_stroke_info->wet_stroke_avg_displacement = (uint16_T)
      (accum_stroke_count->wet_percent_displacement_sum /
       accum_stroke_count->wet_stroke_count_sum);
  } else {
    hourly_stroke_info->wet_stroke_avg_displacement = 0U;
  }

  if (hourly_stroke_info->wet_stroke_avg_displacement > 200U) {
    hourly_stroke_info->wet_stroke_avg_displacement = 200U;
  }

  /*  Dry Strokes */
  hourly_stroke_info->dry_stroke_count =
    accum_stroke_count->dry_stroke_count_sum;
  if (accum_stroke_count->dry_stroke_count_sum > 0U) {
    hourly_stroke_info->dry_stroke_avg_displacement = (uint16_T)
      (accum_stroke_count->dry_percent_displacement_sum /
       accum_stroke_count->dry_stroke_count_sum);
  } else {
    hourly_stroke_info->dry_stroke_avg_displacement = 0U;
  }

  if (hourly_stroke_info->dry_stroke_avg_displacement > 200U) {
    hourly_stroke_info->dry_stroke_avg_displacement = 200U;
  }

  /*  Combined strokes */
  hourly_stroke_info->combined_stroke_count = (uint16_T)((uint32_T)
    accum_stroke_count->dry_stroke_count_sum +
    accum_stroke_count->wet_stroke_count_sum);
  if (hourly_stroke_info->combined_stroke_count > 0U) {
    hourly_stroke_info->c_combined_stroke_avg_displacem = (uint16_T)
      ((accum_stroke_count->dry_percent_displacement_sum +
        accum_stroke_count->wet_percent_displacement_sum) /
       hourly_stroke_info->combined_stroke_count);
  } else {
    hourly_stroke_info->c_combined_stroke_avg_displacem = 0U;
  }

  if (hourly_stroke_info->c_combined_stroke_avg_displacem > 200U) {
    hourly_stroke_info->c_combined_stroke_avg_displacem = 200U;
  }

  /*  Reset calibration struct */
  accum_stroke_count->num_windows_processed = 0U;
  accum_stroke_count->wet_stroke_count_sum = 0U;
  accum_stroke_count->wet_percent_displacement_sum = 0UL;
  accum_stroke_count->dry_stroke_count_sum = 0U;
  accum_stroke_count->dry_percent_displacement_sum = 0UL;
  accum_stroke_count->mag_calibration_changed = 0U;
}

/*
 * File trailer for hourlyStrokeCount.c
 *
 * [EOF]
 */
