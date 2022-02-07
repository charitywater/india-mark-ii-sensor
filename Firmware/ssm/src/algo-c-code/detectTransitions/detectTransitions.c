/*
 * File: detectTransitions.c
 *
 * MATLAB Coder version            : 5.0
 * C/C++ source code generated on  : 18-May-2021 12:00:09
 */

/* Include Files */
#include "detectTransitions.h"

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

/* Function Declarations */
static ReasonCodes addTransition(TransitionType b_type, int16_T b_val, int16_T
  b_idx, strokeTransitionBuffer_t *transitions);
static int16_T combineAxes(uint16_T b_idx, const int16_T
  mag_windows_blockA_x_lsb[20], const int16_T mag_windows_blockA_y_lsb[20],
  const int16_T mag_windows_blockA_z_lsb[20], const int16_T
  mag_windows_blockB_x_lsb[20], const int16_T mag_windows_blockB_y_lsb[20],
  const int16_T mag_windows_blockB_z_lsb[20], const int16_T
  mag_windows_blockOA_x_lsb[50], const int16_T mag_windows_blockOA_y_lsb[50],
  const int16_T mag_windows_blockOA_z_lsb[50], const int16_T
  mag_windows_blockOB_x_lsb[50], const int16_T mag_windows_blockOB_y_lsb[50],
  const int16_T mag_windows_blockOB_z_lsb[50], Window mag_windows_read_window,
  Orientation mag_calib_x_orientation, Orientation mag_calib_y_orientation,
  Orientation mag_calib_z_orientation, int16_T mag_calib_x_offset, int16_T
  mag_calib_y_offset, int16_T mag_calib_z_offset);

/* Function Definitions */

/*
 * Arguments    : TransitionType b_type
 *                int16_T b_val
 *                int16_T b_idx
 *                strokeTransitionBuffer_t *transitions
 * Return Type  : ReasonCodes
 */
static ReasonCodes addTransition(TransitionType b_type, int16_T b_val, int16_T
  b_idx, strokeTransitionBuffer_t *transitions)
{
  ReasonCodes reason_code;

  /*  Function to add transition */
  reason_code = reason_code_none;
  if (transitions->idx < 30) {
    transitions->idx++;
    transitions->buff[transitions->idx - 1].type = b_type;
    transitions->buff[transitions->idx - 1].val = b_val;
    transitions->buff[transitions->idx - 1].idx = b_idx;
  } else {
    /*  We have overflowed the buffer, we need to */
    /*  reset the state info and start over. Replace the last */
    /*  transition with a no-activity transition             */
    transitions->buff[transitions->idx - 1].type = no_transition_activity;
    transitions->buff[transitions->idx - 1].val = 0;
    transitions->buff[transitions->idx - 1].idx = b_idx;

    /*  Maintain the is_first state */
    /*  Reset the state info */
    /*  Set reason code */
    reason_code = stroke_trans_buffer_overflow;
  }

  return reason_code;
}

/*
 * Arguments    : uint16_T b_idx
 *                const int16_T mag_windows_blockA_x_lsb[20]
 *                const int16_T mag_windows_blockA_y_lsb[20]
 *                const int16_T mag_windows_blockA_z_lsb[20]
 *                const int16_T mag_windows_blockB_x_lsb[20]
 *                const int16_T mag_windows_blockB_y_lsb[20]
 *                const int16_T mag_windows_blockB_z_lsb[20]
 *                const int16_T mag_windows_blockOA_x_lsb[50]
 *                const int16_T mag_windows_blockOA_y_lsb[50]
 *                const int16_T mag_windows_blockOA_z_lsb[50]
 *                const int16_T mag_windows_blockOB_x_lsb[50]
 *                const int16_T mag_windows_blockOB_y_lsb[50]
 *                const int16_T mag_windows_blockOB_z_lsb[50]
 *                Window mag_windows_read_window
 *                Orientation mag_calib_x_orientation
 *                Orientation mag_calib_y_orientation
 *                Orientation mag_calib_z_orientation
 *                int16_T mag_calib_x_offset
 *                int16_T mag_calib_y_offset
 *                int16_T mag_calib_z_offset
 * Return Type  : int16_T
 */
static int16_T combineAxes(uint16_T b_idx, const int16_T
  mag_windows_blockA_x_lsb[20], const int16_T mag_windows_blockA_y_lsb[20],
  const int16_T mag_windows_blockA_z_lsb[20], const int16_T
  mag_windows_blockB_x_lsb[20], const int16_T mag_windows_blockB_y_lsb[20],
  const int16_T mag_windows_blockB_z_lsb[20], const int16_T
  mag_windows_blockOA_x_lsb[50], const int16_T mag_windows_blockOA_y_lsb[50],
  const int16_T mag_windows_blockOA_z_lsb[50], const int16_T
  mag_windows_blockOB_x_lsb[50], const int16_T mag_windows_blockOB_y_lsb[50],
  const int16_T mag_windows_blockOB_z_lsb[50], Window mag_windows_read_window,
  Orientation mag_calib_x_orientation, Orientation mag_calib_y_orientation,
  Orientation mag_calib_z_orientation, int16_T mag_calib_x_offset, int16_T
  mag_calib_y_offset, int16_T mag_calib_z_offset)
{
  int16_T x;
  int16_T y;
  int16_T z;
  int16_T sample_x_lsb_tmp;
  int16_T b_sample_x_lsb_tmp;
  int16_T c_sample_x_lsb_tmp;
  int16_T d_sample_x_lsb_tmp;
  int16_T e_sample_x_lsb_tmp;
  int16_T f_sample_x_lsb_tmp;
  int16_T sample_x_lsb;
  int16_T sample_y_lsb;
  int16_T sample_z_lsb;
  x = -500;
  y = -500;
  z = -500;
  if ((b_idx >= 1U) && (b_idx <= 120U) && (mag_windows_read_window != no_window))
  {
    /*  Find which block the index falls into */
    if (b_idx <= 50U) {
      /*  Read from the first overlap block */
      if (mag_windows_read_window == windowA) {
        b_sample_x_lsb_tmp = (int16_T)b_idx - 1;
        sample_x_lsb = mag_windows_blockOA_x_lsb[b_sample_x_lsb_tmp];
        sample_y_lsb = mag_windows_blockOA_y_lsb[b_sample_x_lsb_tmp];
        sample_z_lsb = mag_windows_blockOA_z_lsb[b_sample_x_lsb_tmp];
      } else {
        sample_x_lsb_tmp = (int16_T)b_idx - 1;
        sample_x_lsb = mag_windows_blockOB_x_lsb[sample_x_lsb_tmp];
        sample_y_lsb = mag_windows_blockOB_y_lsb[sample_x_lsb_tmp];
        sample_z_lsb = mag_windows_blockOB_z_lsb[sample_x_lsb_tmp];
      }
    } else if (b_idx <= 70U) {
      /*  Read from the non-overlap block */
      if (mag_windows_read_window == windowA) {
        f_sample_x_lsb_tmp = (int16_T)b_idx - 51;
        sample_x_lsb = mag_windows_blockA_x_lsb[f_sample_x_lsb_tmp];
        sample_y_lsb = mag_windows_blockA_y_lsb[f_sample_x_lsb_tmp];
        sample_z_lsb = mag_windows_blockA_z_lsb[f_sample_x_lsb_tmp];
      } else {
        e_sample_x_lsb_tmp = (int16_T)b_idx - 51;
        sample_x_lsb = mag_windows_blockB_x_lsb[e_sample_x_lsb_tmp];
        sample_y_lsb = mag_windows_blockB_y_lsb[e_sample_x_lsb_tmp];
        sample_z_lsb = mag_windows_blockB_z_lsb[e_sample_x_lsb_tmp];
      }
    } else {
      /*  Read from last overlap block */
      if (mag_windows_read_window == windowA) {
        d_sample_x_lsb_tmp = (int16_T)b_idx - 71;
        sample_x_lsb = mag_windows_blockOB_x_lsb[d_sample_x_lsb_tmp];
        sample_y_lsb = mag_windows_blockOB_y_lsb[d_sample_x_lsb_tmp];
        sample_z_lsb = mag_windows_blockOB_z_lsb[d_sample_x_lsb_tmp];
      } else {
        c_sample_x_lsb_tmp = (int16_T)b_idx - 71;
        sample_x_lsb = mag_windows_blockOA_x_lsb[c_sample_x_lsb_tmp];
        sample_y_lsb = mag_windows_blockOA_y_lsb[c_sample_x_lsb_tmp];
        sample_z_lsb = mag_windows_blockOA_z_lsb[c_sample_x_lsb_tmp];
      }
    }

    if (mag_calib_x_orientation == positive) {
      x = sample_x_lsb - mag_calib_x_offset;
    } else {
      x = mag_calib_x_offset - sample_x_lsb;
    }

    if (mag_calib_y_orientation == positive) {
      y = sample_y_lsb - mag_calib_y_offset;
    } else {
      y = mag_calib_y_offset - sample_y_lsb;
    }

    if (mag_calib_z_orientation == positive) {
      z = sample_z_lsb - mag_calib_z_offset;
    } else {
      z = mag_calib_z_offset - sample_z_lsb;
    }
  }

  return ((x + y) + z) + 1500;
}

/*
 * Arguments    : const magWindows_t *mag_windows
 *                const magCalibration_t *mag_calib
 *                strokeTransitionBuffer_t *transitions
 *                strokeTransitionInfo_t *state_info
 * Return Type  : ReasonCodes
 */
ReasonCodes detectTransitions(const magWindows_t *mag_windows, const
  magCalibration_t *mag_calib, strokeTransitionBuffer_t *transitions,
  strokeTransitionInfo_t *state_info)
{
  ReasonCodes reason_code;
  int16_T start_idx;
  int16_T i;
  int16_T curr_val;

  /*  Initialize reason code output */
  reason_code = reason_code_none;

  /*  Initialize the buffer */
  transitions->idx = 0U;
  if ((mag_calib->magnet_present != 0) && (mag_calib->orientation_calibrated !=
       0) && (mag_calib->offset_calibrated != 0)) {
    /*  Adjust the incoming peak idx - it should be from the previous window... */
    if (state_info->is_first == 0) {
      state_info->peak_idx -= 70;
    }

    /*  Determine the starting index into the window */
    if (state_info->is_first != 0) {
      start_idx = 2;
      state_info->prev_val = combineAxes(1U, mag_windows->blockA.x_lsb,
        mag_windows->blockA.y_lsb, mag_windows->blockA.z_lsb,
        mag_windows->blockB.x_lsb, mag_windows->blockB.y_lsb,
        mag_windows->blockB.z_lsb, mag_windows->blockOA.x_lsb,
        mag_windows->blockOA.y_lsb, mag_windows->blockOA.z_lsb,
        mag_windows->blockOB.x_lsb, mag_windows->blockOB.y_lsb,
        mag_windows->blockOB.z_lsb, mag_windows->read_window,
        mag_calib->x_orientation, mag_calib->y_orientation,
        mag_calib->z_orientation, mag_calib->x_offset, mag_calib->y_offset,
        mag_calib->z_offset);

      /*  We are starting in a NoActivity state on the first pass - log a */
      /*  no_activity transition */
      reason_code = addTransition(no_transition_activity, 0, 1, transitions);
    } else {
      start_idx = 51;
    }

    /*  Loop through the window looking for stroke transitions */
    for (i = start_idx; i < 121; i++) {
      curr_val = combineAxes((uint16_T)i, mag_windows->blockA.x_lsb,
        mag_windows->blockA.y_lsb, mag_windows->blockA.z_lsb,
        mag_windows->blockB.x_lsb, mag_windows->blockB.y_lsb,
        mag_windows->blockB.z_lsb, mag_windows->blockOA.x_lsb,
        mag_windows->blockOA.y_lsb, mag_windows->blockOA.z_lsb,
        mag_windows->blockOB.x_lsb, mag_windows->blockOB.y_lsb,
        mag_windows->blockOB.z_lsb, mag_windows->read_window,
        mag_calib->x_orientation, mag_calib->y_orientation,
        mag_calib->z_orientation, mag_calib->x_offset, mag_calib->y_offset,
        mag_calib->z_offset);
      switch (state_info->state) {
       case no_activity:
        state_info->state_switch_cnt = 0U;
        if (curr_val > state_info->prev_val) {
          /*  Reset the downslope counters */
          /*  Function to reset downslope counters */
          state_info->downslope_cnt = 0U;
          state_info->downslope_sum = 0;

          /*  Increment the upslope counters */
          state_info->upslope_cnt++;
          state_info->upslope_sum = (state_info->upslope_sum + curr_val) -
            state_info->prev_val;

          /*  If we've seen a substantial trend downward, switch to */
          /*  looking for a peak  */
          if ((state_info->upslope_cnt >= 3) && (state_info->upslope_sum >= 95))
          {
            state_info->state = finding_peak;

            /*  The current value serves as the new peak */
            state_info->peak_val = curr_val;
            state_info->peak_idx = i;

            /*  As we exit the no-activity zone, add a valley since we are now looking for a */
            /*  peak after a zone with no activity */
            reason_code = addTransition(transition_valley, state_info->prev_val,
              i - 1, transitions);
          }
        } else if (curr_val < state_info->prev_val) {
          /*  Reset the upslope counters */
          /*  Function to reset upslope counters */
          state_info->upslope_cnt = 0U;
          state_info->upslope_sum = 0;

          /*  Increment the downslope counters */
          state_info->downslope_cnt++;
          state_info->downslope_sum = (state_info->downslope_sum +
            state_info->prev_val) - curr_val;

          /*  If we've seen a substantial trend downward, switch to */
          /*  looking for a peak  */
          if ((state_info->downslope_cnt >= 3) && (state_info->downslope_sum >=
               95)) {
            state_info->state = finding_valley;

            /*  The current value serves as the new peak */
            state_info->peak_val = curr_val;
            state_info->peak_idx = i;

            /*  As we exit the no-activity zone, add a peak since we are now looking for a */
            /*  valley after a zone with no activity */
            reason_code = addTransition(transition_peak, state_info->prev_val, i
              - 1, transitions);
          }
        } else {
          /*  No change in value from the previous reset upslope and downslope counters */
          /*  Function to reset upslope counters */
          state_info->upslope_cnt = 0U;
          state_info->upslope_sum = 0;

          /*  Function to reset downslope counters */
          state_info->downslope_cnt = 0U;
          state_info->downslope_sum = 0;
        }
        break;

       case finding_peak:
        /*  Looking for a peak... */
        if (curr_val >= state_info->prev_val) {
          if (curr_val >= state_info->peak_val) {
            state_info->peak_val = curr_val;
            state_info->peak_idx = i;
          }

          /*  Each time we switch from negative slope to positive */
          /*  slope, increment the state switch count and reset the */
          /*  downslope counter */
          if (state_info->downslope_cnt > 0) {
            state_info->state_switch_cnt++;
          }

          /*  Function to reset downslope counters */
          state_info->downslope_cnt = 0U;
          state_info->downslope_sum = 0;
        } else {
          /*  Reset the up slope counters */
          /*  Function to reset upslope counters */
          state_info->upslope_cnt = 0U;
          state_info->upslope_sum = 0;

          /*  Increment the downslope counters */
          state_info->downslope_cnt++;
          state_info->downslope_sum = (state_info->downslope_sum +
            state_info->prev_val) - curr_val;

          /*  If we've seen a substantial trend downward, switch to */
          /*  looking for a valley */
          if ((state_info->downslope_cnt >= 3) && (state_info->downslope_sum >=
               95)) {
            /*  Save the peak information */
            reason_code = addTransition(transition_peak, state_info->peak_val,
              state_info->peak_idx, transitions);

            /*  We've seen a trend where the values are */
            /*  increasing and the total increase is a large */
            /*  enough magnitude to switch toward finding a peak */
            state_info->state = finding_valley;
            state_info->state_switch_cnt = 0U;

            /*  Record the current value as the new peak - this will */
            /*  get updated further in the find valley state */
            state_info->peak_val = curr_val;
            state_info->peak_idx = i;
          }
        }

        /*  If there hasn't been much activity for a while, move back to the */
        /*  flat state */
        if (state_info->state_switch_cnt >= 12) {
          state_info->state = no_activity;

          /*  Add the peak that we've been tracking - serves as a */
          /*  "transition" point to "close out" the previous stroke */
          addTransition(transition_peak, state_info->peak_val,
                        state_info->peak_idx, transitions);

          /*  Add an no-activity transition since we are moving to */
          /*  that state */
          reason_code = addTransition(no_transition_activity, 0, i, transitions);
        }
        break;

       default:
        /*  Looking for a valley... */
        if (curr_val <= state_info->prev_val) {
          if (curr_val <= state_info->peak_val) {
            state_info->peak_val = curr_val;
            state_info->peak_idx = i;
          }

          /*  Each time we switch from positive slope to negative */
          /*  slope, increment the state switch count and reset the */
          /*  upslope counter */
          if (state_info->upslope_cnt > 0) {
            state_info->state_switch_cnt++;
          }

          /*  Function to reset upslope counters */
          state_info->upslope_cnt = 0U;
          state_info->upslope_sum = 0;
        } else {
          /*  Reset the downslope counters */
          /*  Function to reset downslope counters */
          state_info->downslope_cnt = 0U;
          state_info->downslope_sum = 0;

          /*  Reset the upslope counters */
          state_info->upslope_cnt++;
          state_info->upslope_sum = (state_info->upslope_sum + curr_val) -
            state_info->prev_val;

          /*  If we've seen a substantial trend upward, switch to */
          /*  looking for a peak */
          if ((state_info->upslope_cnt >= 3) && (state_info->upslope_sum >= 95))
          {
            /*  Save the valley information */
            reason_code = addTransition(transition_valley, state_info->peak_val,
              state_info->peak_idx, transitions);

            /*  We've seen a trend where the values are */
            /*  increasing and the total increase is a large */
            /*  enough magnitude to switch toward finding a peak */
            state_info->state = finding_peak;
            state_info->state_switch_cnt = 0U;

            /*  Record the current value as the peak - this will */
            /*  get updated further in the find peak state */
            state_info->peak_val = curr_val;
            state_info->peak_idx = i;
          }
        }

        /*  If there hasn't been much activity for a while, move back to the */
        /*  flat state */
        if (state_info->state_switch_cnt >= 12) {
          state_info->state = no_activity;

          /*  Add the peak that we've been tracking - serves as a */
          /*  "transition" point to "close out" the previous stroke */
          addTransition(transition_valley, state_info->peak_val,
                        state_info->peak_idx, transitions);

          /*  Add an no-activity transition since we are moving to */
          /*  that state */
          reason_code = addTransition(no_transition_activity, 0, i, transitions);
        }
        break;
      }

      state_info->prev_val = curr_val;
    }

    state_info->is_first = 0U;
  }

  return reason_code;
}

/*
 * File trailer for detectTransitions.c
 *
 * [EOF]
 */
