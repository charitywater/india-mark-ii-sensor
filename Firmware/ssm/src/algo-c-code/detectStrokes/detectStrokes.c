/*
 * File: detectStrokes.c
 *
 * MATLAB Coder version            : 5.0
 * C/C++ source code generated on  : 27-Oct-2022 08:13:00
 */

/* Include Files */
#include "detectStrokes.h"

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
 * Arguments    : const strokeTransitionBuffer_t *transitions
 *                strokeBuffer_t *strokes
 *                strokeDetectInfo_t *state_info
 *                accumStrokeCount_t *accum_stroke_count
 *                const magCalibration_t *mag_calib
 *                const waterAlgoData_t *water_data
 * Return Type  : ReasonCodes
 */
ReasonCodes detectStrokes(const strokeTransitionBuffer_t *transitions,
  strokeBuffer_t *strokes, strokeDetectInfo_t *state_info, accumStrokeCount_t
  *accum_stroke_count, const magCalibration_t *mag_calib, const waterAlgoData_t *
  water_data)
{
  ReasonCodes reason_code;
  strokeTransition_t prev_trans;
  uint8_T b_idx;
  uint8_T stop;
  int16_T stroke_percent_displacement;
  uint8_T stroke_dry;
  boolean_T guard1 = false;
  boolean_T guard2 = false;
  int16_T i;
  StrokeType stroke_type;
  int16_T stroke_duration;
  int16_T i1;
  int16_T stroke_idx;
  int16_T stroke_val;
  stroke_t expl_temp;
  int16_T i2;
  int16_T b_stroke_percent_displacement;
  int16_T c_stroke_percent_displacement;

  /*  Initialize reason code */
  reason_code = reason_code_none;

  /*  Initialize the buffer */
  strokes->idx = 0U;
  if (transitions->idx > 0) {
    /*  Adjust the last stroke transition index to be in the current window */
    /*  range */
    if (state_info->is_first == 0) {
      state_info->last_transition.idx -= 70;
    }

    /*  Choose the starting index - if we have a previous transition, use */
    /*  this - otherwise use the first transition found in this window */
    if (state_info->last_transition.type == no_transition) {
      prev_trans = transitions->buff[0];
      b_idx = 2U;
    } else {
      prev_trans = state_info->last_transition;
      b_idx = 1U;
    }

    stop = 0U;
    while ((stop == 0) && (b_idx <= transitions->idx)) {
      stroke_percent_displacement = 0;
      stroke_dry = 0U;
      guard1 = false;
      guard2 = false;
      if (prev_trans.type == transition_peak) {
        i = b_idx - 1;
        if (transitions->buff[i].type == transition_valley) {
          stroke_type = rod_up;
          stroke_duration = transitions->buff[i].idx - prev_trans.idx;
          stroke_idx = (prev_trans.idx + transitions->buff[i].idx) >> 1;
          stroke_val = (prev_trans.val + transitions->buff[i].val) >> 1;

          /*  Given the current calibration, the displacement measured  */
          /*  should not be radically different than the calibrated range.  */
          /*  NOTE: Transitions will only be detected if the calibration has */
          /*  been complete... If there are no transitions, strokes will not be */
          /*  detected... */
          /*  Multiply by 100 to get to percentage */
          /*  Find the range of the  */
          i2 = mag_calib->max_val - mag_calib->min_val;
          if (i2 > 0) {
            stroke_percent_displacement = (int16_T)((prev_trans.val -
              transitions->buff[i].val) * 100L / i2);
          }

          if (((int32_T)stroke_idx < water_data->present_start_idx) ||
              ((uint16_T)stroke_idx > water_data->present_stop_idx)) {
            stroke_dry = 1U;
          }

          guard1 = true;
        } else {
          guard2 = true;
        }
      } else {
        guard2 = true;
      }

      if (guard2 && (prev_trans.type == transition_valley)) {
        i1 = b_idx - 1;
        if (transitions->buff[i1].type == transition_peak) {
          stroke_type = rod_down;
          stroke_duration = transitions->buff[i1].idx - prev_trans.idx;
          stroke_idx = (prev_trans.idx + transitions->buff[i1].idx) >> 1;
          stroke_val = (prev_trans.val + transitions->buff[i1].val) >> 1;

          /*  Given the current calibration, the displacement measured  */
          /*  should not be radically different than the calibrated range.  */
          /*  NOTE: Transitions will only be detected if the calibration has */
          /*  been complete... If there are no transitions, strokes will not be */
          /*  detected... */
          /*  Multiply by 100 to get to percentage */
          /*  Find the range of the  */
          if (mag_calib->max_val - mag_calib->min_val > 0) {
            stroke_percent_displacement = (int16_T)((prev_trans.val -
              transitions->buff[i1].val) * 100L / (mag_calib->max_val -
              mag_calib->min_val));
          }

          if (((int32_T)stroke_idx < water_data->present_start_idx) ||
              ((uint16_T)stroke_idx > water_data->present_stop_idx)) {
            stroke_dry = 1U;
          }

          guard1 = true;
        }
      }

      if (guard1) {
        if (strokes->idx < 30) {
          strokes->idx++;
          expl_temp.type = stroke_type;
          expl_temp.val = stroke_val;
          expl_temp.idx = stroke_idx;
          expl_temp.duration = stroke_duration;
          expl_temp.percent_displacement = stroke_percent_displacement;
          expl_temp.dry = stroke_dry;
          strokes->buff[strokes->idx - 1] = expl_temp;

          /*  Increment the accumlated stroked count */
          if (stroke_dry != 0) {
            accum_stroke_count->dry_stroke_count_sum++;
            if (stroke_percent_displacement < 0) {
              c_stroke_percent_displacement = -stroke_percent_displacement;
            } else {
              c_stroke_percent_displacement = stroke_percent_displacement;
            }

            accum_stroke_count->dry_percent_displacement_sum +=
              c_stroke_percent_displacement;
          } else {
            accum_stroke_count->wet_stroke_count_sum++;
            if (stroke_percent_displacement < 0) {
              b_stroke_percent_displacement = -stroke_percent_displacement;
            } else {
              b_stroke_percent_displacement = stroke_percent_displacement;
            }

            accum_stroke_count->wet_percent_displacement_sum +=
              b_stroke_percent_displacement;
          }
        } else {
          reason_code = stroke_buffer_overflow;
          stop = 1U;

          /*  Reached the maximum amount of strokes for this window */
        }
      }

      prev_trans = transitions->buff[b_idx - 1];
      b_idx++;
    }

    state_info->last_transition = prev_trans;
  }

  state_info->is_first = 0U;

  /*  Increment the windowed processed counter */
  accum_stroke_count->num_windows_processed++;
  if (mag_calib->calibration_changed != 0) {
    accum_stroke_count->mag_calibration_changed = 1U;

    /*  Lock onto this state if there is any calibration change within this hour */
  }

  return reason_code;
}

/*
 * File trailer for detectStrokes.c
 *
 * [EOF]
 */
