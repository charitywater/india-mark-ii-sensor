/*
 * File: cliResetStrokeCount.c
 *
 * MATLAB Coder version            : 5.0
 * C/C++ source code generated on  : 18-May-2021 12:00:54
 */

/* Include Files */
#include "cliResetStrokeCount.h"

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
 * Return Type  : void
 */
void cliResetStrokeCount(accumStrokeCount_t *accum_stroke_count)
{
  /*  Reset calibration struct */
  accum_stroke_count->num_windows_processed = 0U;
  accum_stroke_count->wet_stroke_count_sum = 0U;
  accum_stroke_count->wet_percent_displacement_sum = 0UL;
  accum_stroke_count->dry_stroke_count_sum = 0U;
  accum_stroke_count->dry_percent_displacement_sum = 0UL;
  accum_stroke_count->mag_calibration_changed = 0U;
}

/*
 * File trailer for cliResetStrokeCount.c
 *
 * [EOF]
 */
