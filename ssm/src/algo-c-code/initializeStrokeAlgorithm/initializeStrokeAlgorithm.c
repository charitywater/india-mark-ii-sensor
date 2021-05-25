/*
 * File: initializeStrokeAlgorithm.c
 *
 * MATLAB Coder version            : 5.0
 * C/C++ source code generated on  : 18-May-2021 11:59:32
 */

/* Include Files */
#include "initializeStrokeAlgorithm.h"

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
 * Arguments    : strokeTransitionInfo_t *stroke_transition_info
 *                strokeDetectInfo_t *stroke_detect_info
 *                accumStrokeCount_t *accum_stroke_count
 * Return Type  : void
 */
void initializeStrokeAlgorithm(strokeTransitionInfo_t *stroke_transition_info,
  strokeDetectInfo_t *stroke_detect_info, accumStrokeCount_t *accum_stroke_count)
{
  /*  Reset stroke algorithm structures */
  stroke_transition_info->state = no_activity;
  stroke_transition_info->is_first = 1U;
  stroke_transition_info->downslope_cnt = 0U;
  stroke_transition_info->downslope_sum = 0;
  stroke_transition_info->upslope_cnt = 0U;
  stroke_transition_info->upslope_sum = 0;
  stroke_transition_info->peak_val = 0;
  stroke_transition_info->peak_idx = 0;
  stroke_transition_info->state_switch_cnt = 0U;
  stroke_transition_info->prev_val = 0;
  stroke_detect_info->is_first = 1U;
  stroke_detect_info->last_transition.type = no_transition;
  stroke_detect_info->last_transition.val = 0;
  stroke_detect_info->last_transition.idx = 0;
  accum_stroke_count->num_windows_processed = 0U;
  accum_stroke_count->wet_stroke_count_sum = 0U;
  accum_stroke_count->wet_percent_displacement_sum = 0UL;
  accum_stroke_count->dry_stroke_count_sum = 0U;
  accum_stroke_count->dry_percent_displacement_sum = 0UL;
  accum_stroke_count->mag_calibration_changed = 0U;
}

/*
 * File trailer for initializeStrokeAlgorithm.c
 *
 * [EOF]
 */
