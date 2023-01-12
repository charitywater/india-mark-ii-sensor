/*
 * File: initializeMagCalibration.c
 *
 * MATLAB Coder version            : 5.0
 * C/C++ source code generated on  : 27-Oct-2022 08:11:44
 */

/* Include Files */
#include "initializeMagCalibration.h"

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
 * Arguments    : magCalibration_t *mag_calib
 * Return Type  : void
 */
void initializeMagCalibration(magCalibration_t *mag_calib)
{
  /*  Reset calibration struct */
  mag_calib->x_orientation = no_orientation;
  mag_calib->y_orientation = no_orientation;
  mag_calib->z_orientation = no_orientation;
  mag_calib->x_offset = 0;
  mag_calib->y_offset = 0;
  mag_calib->z_offset = 0;
  mag_calib->orientation_calibrated = 0U;
  mag_calib->offset_calibrated = 0U;
  mag_calib->window_calib.mag_pres_pos_win_cnt = 0U;
  mag_calib->window_calib.mag_pres_neg_win_cnt = 0U;
  mag_calib->window_calib.mag_pres_win_cnt = 0U;
  mag_calib->window_calib.orient_win_cnt = 0U;
  mag_calib->window_calib.orient_reset_cnt = 0U;
  mag_calib->window_calib.orient_cal_reset_cnt = 0U;
  mag_calib->window_calib.orient_xy_sync_window_cnt = 0UL;
  mag_calib->window_calib.orient_xy_nosync_window_cnt = 0UL;
  mag_calib->window_calib.orient_xz_sync_window_cnt = 0UL;
  mag_calib->window_calib.orient_xz_nosync_window_cnt = 0UL;
  mag_calib->window_calib.offset_win_cnt = 0U;
  mag_calib->window_calib.offset_new_value_diff = 0L;
  mag_calib->window_calib.x_max_val = MIN_int16_T;
  mag_calib->window_calib.x_min_val = MAX_int16_T;
  mag_calib->window_calib.y_max_val = MIN_int16_T;
  mag_calib->window_calib.y_min_val = MAX_int16_T;
  mag_calib->window_calib.z_max_val = MIN_int16_T;
  mag_calib->window_calib.z_min_val = MAX_int16_T;
  mag_calib->max_val = 0;
  mag_calib->min_val = 0;
  mag_calib->magnet_present = 0U;
}

/*
 * File trailer for initializeMagCalibration.c
 *
 * [EOF]
 */
