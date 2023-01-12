/*
 * File: magnetometerCalibration.c
 *
 * MATLAB Coder version            : 5.0
 * C/C++ source code generated on  : 27-Oct-2022 08:12:08
 */

/* Include Files */
#include "magnetometerCalibration.h"
#include "isPeakValley.h"

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

/* Type Definitions */
#ifndef typedef_magSample_t
#define typedef_magSample_t

typedef struct {
  int16_T x_lsb;
  int16_T y_lsb;
  int16_T z_lsb;
  int16_T temp_lsb;
  uint8_T status;
} magSample_t;

#endif                                 /*typedef_magSample_t*/

/* Function Declarations */
static void add_reason_code(ReasonCodes reason_code, ReasonCodes reason_codes[8]);
static void read_sample(uint16_T idx, const int16_T mag_windows_blockA_x_lsb[20],
  const int16_T mag_windows_blockA_y_lsb[20], const int16_T
  mag_windows_blockA_z_lsb[20], const int16_T mag_windows_blockB_x_lsb[20],
  const int16_T mag_windows_blockB_y_lsb[20], const int16_T
  mag_windows_blockB_z_lsb[20], const int16_T mag_windows_blockOA_x_lsb[50],
  const int16_T mag_windows_blockOA_y_lsb[50], const int16_T
  mag_windows_blockOA_z_lsb[50], const int16_T mag_windows_blockOB_x_lsb[50],
  const int16_T mag_windows_blockOB_y_lsb[50], const int16_T
  mag_windows_blockOB_z_lsb[50], Window mag_windows_read_window, uint8_T
  *success, magSample_t *sample);
static void reset_calibration(magCalibration_t *mag_calib);
static void set_offset_calibration(magCalibration_t *mag_calib);

/* Function Definitions */

/*
 * Arguments    : ReasonCodes reason_code
 *                ReasonCodes reason_codes[8]
 * Return Type  : void
 */
static void add_reason_code(ReasonCodes reason_code, ReasonCodes reason_codes[8])
{
  uint8_T found;
  int16_T r;
  int16_T b_r;
  boolean_T exitg1;

  /*  Does the reason code already exist in the list? */
  found = 0U;
  for (r = 0; r < 8; r++) {
    if (reason_code == reason_codes[r]) {
      found = 1U;
    }
  }

  /*  If it doesn't exist, add if there is a free slot */
  if (found == 0) {
    b_r = 0;
    exitg1 = false;
    while ((!exitg1) && (b_r < 8)) {
      if (reason_codes[b_r] == reason_code_none) {
        reason_codes[b_r] = reason_code;
        exitg1 = true;
      } else {
        b_r++;
      }
    }
  }
}

/*
 * Arguments    : uint16_T idx
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
 *                uint8_T *success
 *                magSample_t *sample
 * Return Type  : void
 */
static void read_sample(uint16_T idx, const int16_T mag_windows_blockA_x_lsb[20],
  const int16_T mag_windows_blockA_y_lsb[20], const int16_T
  mag_windows_blockA_z_lsb[20], const int16_T mag_windows_blockB_x_lsb[20],
  const int16_T mag_windows_blockB_y_lsb[20], const int16_T
  mag_windows_blockB_z_lsb[20], const int16_T mag_windows_blockOA_x_lsb[50],
  const int16_T mag_windows_blockOA_y_lsb[50], const int16_T
  mag_windows_blockOA_z_lsb[50], const int16_T mag_windows_blockOB_x_lsb[50],
  const int16_T mag_windows_blockOB_y_lsb[50], const int16_T
  mag_windows_blockOB_z_lsb[50], Window mag_windows_read_window, uint8_T
  *success, magSample_t *sample)
{
  int16_T sample_tmp;
  int16_T b_sample_tmp;
  int16_T c_sample_tmp;
  int16_T d_sample_tmp;
  int16_T e_sample_tmp;
  int16_T f_sample_tmp;
  *success = 1U;
  if ((idx < 1U) || (idx > 120U) || (mag_windows_read_window == no_window)) {
    *success = 0U;
  } else {
    /*  Find which block the index falls into */
    if (idx <= 50U) {
      /*  Read from the first overlap block */
      if (mag_windows_read_window == windowA) {
        b_sample_tmp = (int16_T)idx - 1;
        sample->x_lsb = mag_windows_blockOA_x_lsb[b_sample_tmp];
        sample->y_lsb = mag_windows_blockOA_y_lsb[b_sample_tmp];
        sample->z_lsb = mag_windows_blockOA_z_lsb[b_sample_tmp];
      } else {
        sample_tmp = (int16_T)idx - 1;
        sample->x_lsb = mag_windows_blockOB_x_lsb[sample_tmp];
        sample->y_lsb = mag_windows_blockOB_y_lsb[sample_tmp];
        sample->z_lsb = mag_windows_blockOB_z_lsb[sample_tmp];
      }
    } else if (idx <= 70U) {
      /*  Read from the non-overlap block */
      if (mag_windows_read_window == windowA) {
        f_sample_tmp = (int16_T)idx - 51;
        sample->x_lsb = mag_windows_blockA_x_lsb[f_sample_tmp];
        sample->y_lsb = mag_windows_blockA_y_lsb[f_sample_tmp];
        sample->z_lsb = mag_windows_blockA_z_lsb[f_sample_tmp];
      } else {
        e_sample_tmp = (int16_T)idx - 51;
        sample->x_lsb = mag_windows_blockB_x_lsb[e_sample_tmp];
        sample->y_lsb = mag_windows_blockB_y_lsb[e_sample_tmp];
        sample->z_lsb = mag_windows_blockB_z_lsb[e_sample_tmp];
      }
    } else {
      /*  Read from last overlap block */
      if (mag_windows_read_window == windowA) {
        d_sample_tmp = (int16_T)idx - 71;
        sample->x_lsb = mag_windows_blockOB_x_lsb[d_sample_tmp];
        sample->y_lsb = mag_windows_blockOB_y_lsb[d_sample_tmp];
        sample->z_lsb = mag_windows_blockOB_z_lsb[d_sample_tmp];
      } else {
        c_sample_tmp = (int16_T)idx - 71;
        sample->x_lsb = mag_windows_blockOA_x_lsb[c_sample_tmp];
        sample->y_lsb = mag_windows_blockOA_y_lsb[c_sample_tmp];
        sample->z_lsb = mag_windows_blockOA_z_lsb[c_sample_tmp];
      }
    }
  }
}

/*
 * Arguments    : magCalibration_t *mag_calib
 * Return Type  : void
 */
static void reset_calibration(magCalibration_t *mag_calib)
{
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

  /*  Each time the calibration is reset, indicate the calibration changed  */
  mag_calib->calibration_changed = 1U;
}

/*
 * Arguments    : magCalibration_t *mag_calib
 * Return Type  : void
 */
static void set_offset_calibration(magCalibration_t *mag_calib)
{
  int16_T x;
  int16_T b_x;
  int16_T y;
  int16_T b_y;
  int16_T z;
  int16_T b_z;
  int16_T b_mag_calib;
  int16_T c_mag_calib;
  int16_T d_mag_calib;
  int16_T e_mag_calib;
  int16_T f_mag_calib;
  int16_T g_mag_calib;

  /*  Set the offsets based on the peak types */
  if (mag_calib->x_orientation == positive) {
    if (mag_calib->x_offset != mag_calib->window_calib.x_min_val) {
      mag_calib->x_offset = mag_calib->window_calib.x_min_val;
    }

    x = mag_calib->window_calib.x_min_val;
    b_x = mag_calib->window_calib.x_max_val;
  } else {
    if (mag_calib->x_offset != mag_calib->window_calib.x_max_val) {
      mag_calib->x_offset = mag_calib->window_calib.x_max_val;
    }

    x = mag_calib->window_calib.x_max_val;
    b_x = mag_calib->window_calib.x_min_val;
  }

  if (mag_calib->y_orientation == positive) {
    if (mag_calib->y_offset != mag_calib->window_calib.y_min_val) {
      mag_calib->y_offset = mag_calib->window_calib.y_min_val;
    }

    y = mag_calib->window_calib.y_min_val;
    b_y = mag_calib->window_calib.y_max_val;
  } else {
    if (mag_calib->y_offset != mag_calib->window_calib.y_max_val) {
      mag_calib->y_offset = mag_calib->window_calib.y_max_val;
    }

    y = mag_calib->window_calib.y_max_val;
    b_y = mag_calib->window_calib.y_min_val;
  }

  if (mag_calib->z_orientation == positive) {
    if (mag_calib->z_offset != mag_calib->window_calib.z_min_val) {
      mag_calib->z_offset = mag_calib->window_calib.z_min_val;
    }

    z = mag_calib->window_calib.z_min_val;
    b_z = mag_calib->window_calib.z_max_val;
  } else {
    if (mag_calib->z_offset != mag_calib->window_calib.z_max_val) {
      mag_calib->z_offset = mag_calib->window_calib.z_max_val;
    }

    z = mag_calib->window_calib.z_max_val;
    b_z = mag_calib->window_calib.z_min_val;
  }

  mag_calib->offset_calibrated = 1U;

  /*  Compute the maximum and minimum field strength values that will */
  /*  be experienced given this calibration */
  if (mag_calib->x_orientation == positive) {
    b_mag_calib = x - mag_calib->x_offset;
  } else {
    b_mag_calib = mag_calib->x_offset - x;
  }

  if (mag_calib->y_orientation == positive) {
    c_mag_calib = y - mag_calib->y_offset;
  } else {
    c_mag_calib = mag_calib->y_offset - y;
  }

  if (mag_calib->z_orientation == positive) {
    d_mag_calib = z - mag_calib->z_offset;
  } else {
    d_mag_calib = mag_calib->z_offset - z;
  }

  mag_calib->min_val = ((b_mag_calib + c_mag_calib) + d_mag_calib) + 1500;
  if (mag_calib->x_orientation == positive) {
    e_mag_calib = b_x - mag_calib->x_offset;
  } else {
    e_mag_calib = mag_calib->x_offset - b_x;
  }

  if (mag_calib->y_orientation == positive) {
    f_mag_calib = b_y - mag_calib->y_offset;
  } else {
    f_mag_calib = mag_calib->y_offset - b_y;
  }

  if (mag_calib->z_orientation == positive) {
    g_mag_calib = b_z - mag_calib->z_offset;
  } else {
    g_mag_calib = mag_calib->z_offset - b_z;
  }

  mag_calib->max_val = ((e_mag_calib + f_mag_calib) + g_mag_calib) + 1500;
}

/*
 * Arguments    : const magWindows_t *mag_windows
 *                magCalibration_t *mag_calib
 *                const waterAlgoData_t *water_data
 *                ReasonCodes reason_codes[8]
 * Return Type  : void
 */
void magnetometerCalibration(const magWindows_t *mag_windows, magCalibration_t
  *mag_calib, const waterAlgoData_t *water_data, ReasonCodes reason_codes[8])
{
  int16_T r;
  int16_T x_max;
  int16_T x_min;
  int16_T y_max;
  int16_T y_min;
  int16_T z_max;
  int16_T z_min;
  uint32_T xy_sync_cnt;
  uint32_T xy_nosync_cnt;
  uint32_T xz_sync_cnt;
  uint32_T xz_nosync_cnt;
  int16_T i;
  int16_T b_i;
  int16_T e_idx;
  int16_T x;
  int16_T b_x;
  uint8_T buff_idx;
  int16_T c_x;
  uint8_T err;
  uint8_T axis_cnt;
  int16_T win_idx;
  int16_T d_x;
  uint8_T read_success;
  magSample_t samp;
  int16_T x_win[3];
  PeakType x_peak_type;
  int16_T x_val;
  int16_T x_win_tmp;
  int16_T y_win[3];
  PeakType y_peak_type;
  int16_T y_val;
  int16_T z_win[3];
  PeakType z_peak_type;
  int16_T z_val;
  int16_T e_x;
  int16_T f_x;
  int32_T g_x;
  int32_T h_x;
  uint16_T d;
  int32_T i_x;
  int32_T j_x;
  int32_T k_x;
  int32_T l_x;
  uint32_T total_xy_cnt;
  uint32_T total_xz_cnt;
  uint32_T b_mag_calib;
  uint32_T c_mag_calib;
  boolean_T guard1 = false;
  boolean_T guard2 = false;
  uint32_T d_mag_calib;
  uint32_T e_mag_calib;
  Orientation b_y_orientation;
  Orientation f_mag_calib;
  Orientation g_mag_calib;
  Orientation h_mag_calib;

  /*  MAGNETOMETERCALIBRATION Calibrates the magnetometer x, y, z axis so they can be */
  /*  combined into a single "vector" value */
  /*  Only calibrate or monitor calibration if water is present */
  /*  Means that if the pump is broken (no water), it can't be calibrated or */
  /*  re-calibrated and if there has been a change that would require an */
  /*  updated calibration, it won't find out about it until water is flowing */
  /*  again */
  /*  Initialize reason code */
  for (r = 0; r < 8; r++) {
    reason_codes[r] = reason_code_none;
  }

  mag_calib->calibration_changed = 0U;

  /*  Track whether or not the calibration has changed during this window */
  if (water_data->present != 0) {
    /*     %% Initialization */
    /*  For Debug */
    x_max = MIN_int16_T;
    x_min = MAX_int16_T;
    y_max = MIN_int16_T;
    y_min = MAX_int16_T;
    z_max = MIN_int16_T;
    z_min = MAX_int16_T;
    xy_sync_cnt = 0UL;
    xy_nosync_cnt = 0UL;
    xz_sync_cnt = 0UL;
    xz_nosync_cnt = 0UL;

    /*     %% Process Window and Collect Stats */
    for (i = 0; i < 118; i++) {
      b_i = (int16_T)(i + 1U);

      /*  Build peak/valley buffer */
      e_idx = (int16_T)(i + 3U);

      /*  Build peak/valley buffer */
      buff_idx = 1U;
      err = 0U;
      for (win_idx = b_i; win_idx <= e_idx; win_idx++) {
        read_sample((uint16_T)win_idx, mag_windows->blockA.x_lsb,
                    mag_windows->blockA.y_lsb, mag_windows->blockA.z_lsb,
                    mag_windows->blockB.x_lsb, mag_windows->blockB.y_lsb,
                    mag_windows->blockB.z_lsb, mag_windows->blockOA.x_lsb,
                    mag_windows->blockOA.y_lsb, mag_windows->blockOA.z_lsb,
                    mag_windows->blockOB.x_lsb, mag_windows->blockOB.y_lsb,
                    mag_windows->blockOB.z_lsb, mag_windows->read_window,
                    &read_success, &samp);
        if (read_success != 0) {
          x_win_tmp = buff_idx - 1;
          x_win[x_win_tmp] = samp.x_lsb;
          y_win[x_win_tmp] = samp.y_lsb;
          z_win[x_win_tmp] = samp.z_lsb;
          buff_idx++;
        } else {
          err = 1U;
        }
      }

      if (err == 0) {
        /*  Slope Direction */
        isPeakValley(x_win, 10, &x_peak_type, &x_val);
        isPeakValley(y_win, 6, &y_peak_type, &y_val);
        isPeakValley(z_win, 10, &z_peak_type, &z_val);

        /*  Max and min tracking */
        if (x_val > x_max) {
          x_max = x_val;
        }

        if (x_val < x_min) {
          x_min = x_val;
        }

        if (y_val > y_max) {
          y_max = y_val;
        }

        if (y_val < y_min) {
          y_min = y_val;
        }

        if (z_val > z_max) {
          z_max = z_val;
        }

        if (z_val < z_min) {
          z_min = z_val;
        }

        /*  Orientation tracking */
        if ((x_peak_type != no_peak) && (y_peak_type != no_peak)) {
          if (x_peak_type == y_peak_type) {
            if (xy_sync_cnt < MAX_uint32_T) {
              xy_sync_cnt++;
            }
          } else {
            if (xy_nosync_cnt < MAX_uint32_T) {
              xy_nosync_cnt++;
            }
          }
        }

        if ((x_peak_type != no_peak) && (z_peak_type != no_peak)) {
          if (x_peak_type == z_peak_type) {
            if (xz_sync_cnt < MAX_uint32_T) {
              xz_sync_cnt++;
            }
          } else {
            if (xz_nosync_cnt < MAX_uint32_T) {
              xz_nosync_cnt++;
            }
          }
        }
      }
    }

    /*     %% Magnet Present Stats */
    if (mag_calib->window_calib.mag_pres_win_cnt < MAX_uint16_T) {
      mag_calib->window_calib.mag_pres_win_cnt++;
    }

    x = x_max - x_min;
    b_x = y_max - y_min;
    c_x = z_max - z_min;
    axis_cnt = 0U;
    if (x < 0) {
      d_x = -x;
    } else {
      d_x = x;
    }

    if (d_x >= 20) {
      axis_cnt = 1U;
    }

    if (b_x < 0) {
      e_x = -b_x;
    } else {
      e_x = b_x;
    }

    if (e_x >= 20) {
      axis_cnt++;
    }

    if (c_x < 0) {
      f_x = -c_x;
    } else {
      f_x = c_x;
    }

    if (f_x >= 20) {
      axis_cnt++;
    }

    if (axis_cnt > 1) {
      /*  Increment postive window count */
      if (mag_calib->window_calib.mag_pres_pos_win_cnt < MAX_uint16_T) {
        mag_calib->window_calib.mag_pres_pos_win_cnt++;
      }
    } else {
      /*  Increment negative window count */
      if (mag_calib->window_calib.mag_pres_neg_win_cnt < MAX_uint16_T) {
        mag_calib->window_calib.mag_pres_neg_win_cnt++;
      }
    }

    /*     %% Magnet Calibration Logic */
    if (mag_calib->magnet_present == 0) {
      /*  THIS IS MAGNET CALIBRATION STATE */
      if (mag_calib->window_calib.mag_pres_win_cnt > 212U) {
        /*  NOTE: The logic above ensures that the sum of */
        /*  pos_win_cnt and neg_win_cnt will be equal to */
        /*  mag_pres_win_cnt - if this changes, this comparison */
        /*  should change as well... */
        if (mag_calib->window_calib.mag_pres_pos_win_cnt > (int32_T)
            (mag_calib->window_calib.mag_pres_neg_win_cnt + 10UL)) {
          /*  Magnet is present */
          mag_calib->magnet_present = 1U;
          add_reason_code(mag_calib_magnet_present, reason_codes);
        }

        /*  Start sampling period again */
        mag_calib->window_calib.mag_pres_win_cnt = 0U;
        mag_calib->window_calib.mag_pres_pos_win_cnt = 0U;
        mag_calib->window_calib.mag_pres_neg_win_cnt = 0U;
      }
    } else {
      /*  THIS IS MAGNET PRESENT MONITORING */
      if (mag_calib->window_calib.mag_pres_win_cnt > 212U) {
        /*  NOTE: The logic above ensures that the sum of */
        /*  pos_win_cnt and neg_win_cnt will be equal to */
        /*  mag_pres_win_cnt - if this changes, this comparison */
        /*  should change as well... */
        if (mag_calib->window_calib.mag_pres_neg_win_cnt >= 212U) {
          /*  Reset the calibration */
          reset_calibration(mag_calib);
          add_reason_code(mag_calib_present_reset, reason_codes);

          /*  For debug */
        }

        /*  Start sampling period again */
        mag_calib->window_calib.mag_pres_win_cnt = 0U;
        mag_calib->window_calib.mag_pres_pos_win_cnt = 0U;
        mag_calib->window_calib.mag_pres_neg_win_cnt = 0U;
      }
    }

    /*     %% -- Magnet is now present */
    if (mag_calib->magnet_present != 0) {
      /*         %% Orientation Calibration Stats */
      if (mag_calib->window_calib.orient_win_cnt < MAX_uint16_T) {
        mag_calib->window_calib.orient_win_cnt++;
      }

      /*  Accumulate XY sync and non-sync counts */
      if (mag_calib->window_calib.orient_xy_sync_window_cnt < MAX_uint32_T) {
        mag_calib->window_calib.orient_xy_sync_window_cnt += xy_sync_cnt;
      }

      if (mag_calib->window_calib.orient_xy_nosync_window_cnt < MAX_uint32_T) {
        mag_calib->window_calib.orient_xy_nosync_window_cnt += xy_nosync_cnt;
      }

      /*  Accumulate XZ sync and non-sync counts */
      if (mag_calib->window_calib.orient_xz_sync_window_cnt < MAX_uint32_T) {
        mag_calib->window_calib.orient_xz_sync_window_cnt += xz_sync_cnt;
      }

      if (mag_calib->window_calib.orient_xz_nosync_window_cnt < MAX_uint32_T) {
        mag_calib->window_calib.orient_xz_nosync_window_cnt += xz_nosync_cnt;
      }

      /*         %% Offset Calibration Stats */
      if (mag_calib->window_calib.offset_win_cnt < MAX_uint16_T) {
        mag_calib->window_calib.offset_win_cnt++;
      }

      /*  Update the max/min val - even if the calibration was just reset */
      /*  due to a major offset change */
      if (x_max > mag_calib->window_calib.x_max_val) {
        if (mag_calib->window_calib.x_max_val != MIN_int16_T) {
          g_x = (int32_T)x_max - mag_calib->window_calib.x_max_val;
          if (g_x < 0L) {
            d = (uint16_T)-g_x;
          } else {
            d = (uint16_T)g_x;
          }

          if (d > mag_calib->window_calib.offset_new_value_diff) {
            mag_calib->window_calib.offset_new_value_diff = d;
          }
        }

        mag_calib->window_calib.x_max_val = x_max;
      }

      if (x_min < mag_calib->window_calib.x_min_val) {
        if (mag_calib->window_calib.x_min_val != MAX_int16_T) {
          h_x = (int32_T)x_min - mag_calib->window_calib.x_min_val;
          if (h_x < 0L) {
            d = (uint16_T)-h_x;
          } else {
            d = (uint16_T)h_x;
          }

          if (d > mag_calib->window_calib.offset_new_value_diff) {
            mag_calib->window_calib.offset_new_value_diff = d;
          }
        }

        mag_calib->window_calib.x_min_val = x_min;
      }

      if (y_max > mag_calib->window_calib.y_max_val) {
        if (mag_calib->window_calib.y_max_val != MIN_int16_T) {
          i_x = (int32_T)y_max - mag_calib->window_calib.y_max_val;
          if (i_x < 0L) {
            d = (uint16_T)-i_x;
          } else {
            d = (uint16_T)i_x;
          }

          if (d > mag_calib->window_calib.offset_new_value_diff) {
            mag_calib->window_calib.offset_new_value_diff = d;
          }
        }

        mag_calib->window_calib.y_max_val = y_max;
      }

      if (y_min < mag_calib->window_calib.y_min_val) {
        if (mag_calib->window_calib.y_min_val != MAX_int16_T) {
          j_x = (int32_T)y_min - mag_calib->window_calib.y_min_val;
          if (j_x < 0L) {
            d = (uint16_T)-j_x;
          } else {
            d = (uint16_T)j_x;
          }

          if (d > mag_calib->window_calib.offset_new_value_diff) {
            mag_calib->window_calib.offset_new_value_diff = d;
          }
        }

        mag_calib->window_calib.y_min_val = y_min;
      }

      if (z_max > mag_calib->window_calib.z_max_val) {
        if (mag_calib->window_calib.z_max_val != MIN_int16_T) {
          k_x = (int32_T)z_max - mag_calib->window_calib.z_max_val;
          if (k_x < 0L) {
            d = (uint16_T)-k_x;
          } else {
            d = (uint16_T)k_x;
          }

          if (d > mag_calib->window_calib.offset_new_value_diff) {
            mag_calib->window_calib.offset_new_value_diff = d;
          }
        }

        mag_calib->window_calib.z_max_val = z_max;
      }

      if (z_min < mag_calib->window_calib.z_min_val) {
        if (mag_calib->window_calib.z_min_val != MAX_int16_T) {
          l_x = (int32_T)z_min - mag_calib->window_calib.z_min_val;
          if (l_x < 0L) {
            d = (uint16_T)-l_x;
          } else {
            d = (uint16_T)l_x;
          }

          if (d > mag_calib->window_calib.offset_new_value_diff) {
            mag_calib->window_calib.offset_new_value_diff = d;
          }
        }

        mag_calib->window_calib.z_min_val = z_min;
      }

      /*         %% Orientation Logic */
      if (mag_calib->orientation_calibrated == 0) {
        /*  THIS IS THE ORIENTATION CALIBRATION STATE */
        if (mag_calib->window_calib.orient_win_cnt > 212U) {
          /*  If we have enough windows of samples, determine the */
          /*  orientation */
          total_xy_cnt = mag_calib->window_calib.orient_xy_sync_window_cnt +
            mag_calib->window_calib.orient_xy_nosync_window_cnt;
          total_xz_cnt = mag_calib->window_calib.orient_xz_sync_window_cnt +
            mag_calib->window_calib.orient_xz_nosync_window_cnt;

          /*  Must have enough samples to make a orientation judgement */
          /*  call */
          if ((total_xy_cnt > 8000UL) && (total_xz_cnt > 8000UL)) {
            if (mag_calib->window_calib.orient_reset_cnt < MAX_uint16_T) {
              mag_calib->window_calib.orient_reset_cnt++;
            }

            /*  Compute the percent confidence in sync/non-sync */
            if (mag_calib->window_calib.orient_xy_sync_window_cnt >=
                mag_calib->window_calib.orient_xy_nosync_window_cnt) {
              c_mag_calib = mag_calib->window_calib.orient_xy_sync_window_cnt -
                mag_calib->window_calib.orient_xy_nosync_window_cnt;
            } else {
              c_mag_calib = mag_calib->window_calib.orient_xy_nosync_window_cnt
                - mag_calib->window_calib.orient_xy_sync_window_cnt;
            }

            guard1 = false;
            if ((int32_T)((uint32_T)(c_mag_calib * 100ULL) / total_xy_cnt) > 50L)
            {
              if (mag_calib->window_calib.orient_xz_sync_window_cnt >=
                  mag_calib->window_calib.orient_xz_nosync_window_cnt) {
                d_mag_calib = mag_calib->window_calib.orient_xz_sync_window_cnt
                  - mag_calib->window_calib.orient_xz_nosync_window_cnt;
              } else {
                d_mag_calib =
                  mag_calib->window_calib.orient_xz_nosync_window_cnt -
                  mag_calib->window_calib.orient_xz_sync_window_cnt;
              }

              if ((int32_T)((uint32_T)(d_mag_calib * 100ULL) / total_xz_cnt) >
                  50L) {
                if (mag_calib->window_calib.orient_xy_sync_window_cnt >
                    mag_calib->window_calib.orient_xy_nosync_window_cnt) {
                  b_y_orientation = positive;
                } else {
                  b_y_orientation = negative;
                }

                /*  Note: X and Z must have the same orientation based on */
                /*  the physics of the pump and magnet with normal */
                /*  magnet placement */
                if (mag_calib->window_calib.orient_xz_sync_window_cnt >
                    mag_calib->window_calib.orient_xz_nosync_window_cnt) {
                  g_mag_calib = positive;
                } else {
                  g_mag_calib = negative;
                }

                if (positive == g_mag_calib) {
                  mag_calib->x_orientation = positive;
                  mag_calib->y_orientation = b_y_orientation;
                  mag_calib->z_orientation = positive;
                  mag_calib->orientation_calibrated = 1U;
                  add_reason_code(c_mag_calib_orientation_calibra, reason_codes);

                  /*  Reset the window stats */
                  mag_calib->window_calib.orient_reset_cnt = 0U;
                  mag_calib->window_calib.orient_cal_reset_cnt = 0U;
                  mag_calib->window_calib.orient_xy_sync_window_cnt = 0UL;
                  mag_calib->window_calib.orient_xy_nosync_window_cnt = 0UL;
                  mag_calib->window_calib.orient_xz_sync_window_cnt = 0UL;
                  mag_calib->window_calib.orient_xz_nosync_window_cnt = 0UL;
                } else {
                  /*  Should never happen */
                }
              } else {
                guard1 = true;
              }
            } else {
              guard1 = true;
            }

            if (guard1 && (mag_calib->window_calib.orient_reset_cnt > 5U)) {
              /*  Record a reason code - the magnet placement is */
              /*  bad or the magnet is wobbling (or something else */
              /*  we don't understand) */
              add_reason_code(mag_calib_bad_placement_wobble, reason_codes);
            }
          } else {
            if (total_xy_cnt <= 8000UL) {
              add_reason_code(mag_calib_xy_cnt_low, reason_codes);
            }

            if (total_xz_cnt <= 8000UL) {
              add_reason_code(mag_calib_xz_cnt_low, reason_codes);
            }
          }

          /*  Start a new sampling period */
          mag_calib->window_calib.orient_win_cnt = 0U;
          if (mag_calib->window_calib.orient_reset_cnt > 5U) {
            mag_calib->window_calib.orient_reset_cnt = 0U;
            mag_calib->window_calib.orient_cal_reset_cnt = 0U;
            mag_calib->window_calib.orient_xy_sync_window_cnt = 0UL;
            mag_calib->window_calib.orient_xy_nosync_window_cnt = 0UL;
            mag_calib->window_calib.orient_xz_sync_window_cnt = 0UL;
            mag_calib->window_calib.orient_xz_nosync_window_cnt = 0UL;
          }
        }
      } else {
        /*  THIS IS ORIENTATION MONITORING */
        if (mag_calib->window_calib.orient_win_cnt > 212U) {
          /*  If we have enough windows of samples, determine the */
          /*  orientation */
          total_xy_cnt = mag_calib->window_calib.orient_xy_sync_window_cnt +
            mag_calib->window_calib.orient_xy_nosync_window_cnt;
          total_xz_cnt = mag_calib->window_calib.orient_xz_sync_window_cnt +
            mag_calib->window_calib.orient_xz_nosync_window_cnt;

          /*  Must have enough samples to make a orientation judgement */
          /*  call */
          if ((total_xy_cnt > 8000UL) && (total_xz_cnt > 8000UL)) {
            if (mag_calib->window_calib.orient_reset_cnt < MAX_uint16_T) {
              mag_calib->window_calib.orient_reset_cnt++;
            }

            /*  Compute the percent confidence in sync/non-sync */
            if (mag_calib->window_calib.orient_xy_sync_window_cnt >=
                mag_calib->window_calib.orient_xy_nosync_window_cnt) {
              b_mag_calib = mag_calib->window_calib.orient_xy_sync_window_cnt -
                mag_calib->window_calib.orient_xy_nosync_window_cnt;
            } else {
              b_mag_calib = mag_calib->window_calib.orient_xy_nosync_window_cnt
                - mag_calib->window_calib.orient_xy_sync_window_cnt;
            }

            guard1 = false;
            guard2 = false;
            if ((int32_T)((uint32_T)(b_mag_calib * 100ULL) / total_xy_cnt) > 50L)
            {
              if (mag_calib->window_calib.orient_xz_sync_window_cnt >=
                  mag_calib->window_calib.orient_xz_nosync_window_cnt) {
                e_mag_calib = mag_calib->window_calib.orient_xz_sync_window_cnt
                  - mag_calib->window_calib.orient_xz_nosync_window_cnt;
              } else {
                e_mag_calib =
                  mag_calib->window_calib.orient_xz_nosync_window_cnt -
                  mag_calib->window_calib.orient_xz_sync_window_cnt;
              }

              if ((int32_T)((uint32_T)(e_mag_calib * 100ULL) / total_xz_cnt) >
                  50L) {
                /*  Since we got here, reset the cal reset count */
                mag_calib->window_calib.orient_cal_reset_cnt = 0U;
                if (positive != mag_calib->x_orientation) {
                  guard2 = true;
                } else {
                  if (mag_calib->window_calib.orient_xy_sync_window_cnt >
                      mag_calib->window_calib.orient_xy_nosync_window_cnt) {
                    f_mag_calib = positive;
                  } else {
                    f_mag_calib = negative;
                  }

                  if (f_mag_calib != mag_calib->y_orientation) {
                    guard2 = true;
                  } else {
                    if (mag_calib->window_calib.orient_xz_sync_window_cnt >
                        mag_calib->window_calib.orient_xz_nosync_window_cnt) {
                      h_mag_calib = positive;
                    } else {
                      h_mag_calib = negative;
                    }

                    if (h_mag_calib != mag_calib->z_orientation) {
                      guard2 = true;
                    }
                  }
                }
              } else {
                guard1 = true;
              }
            } else {
              guard1 = true;
            }

            if (guard2) {
              /*  Reset the calibration */
              reset_calibration(mag_calib);
              add_reason_code(mag_calib_orientation_reset, reason_codes);

              /*  Reset the window stats */
              mag_calib->window_calib.orient_reset_cnt = 0U;
              mag_calib->window_calib.orient_cal_reset_cnt = 0U;
              mag_calib->window_calib.orient_xy_sync_window_cnt = 0UL;
              mag_calib->window_calib.orient_xy_nosync_window_cnt = 0UL;
              mag_calib->window_calib.orient_xz_sync_window_cnt = 0UL;
              mag_calib->window_calib.orient_xz_nosync_window_cnt = 0UL;

              /*  For debug */
            }

            if (guard1) {
              /*  Record a reason code - the magnet placement is */
              /*  bad or the magnet is wobbling (or something else */
              /*  we don't understand) */
              if (mag_calib->window_calib.orient_reset_cnt > 5U) {
                add_reason_code(mag_calib_bad_placement_wobble, reason_codes);
                if (mag_calib->window_calib.orient_cal_reset_cnt < MAX_uint16_T)
                {
                  mag_calib->window_calib.orient_cal_reset_cnt++;
                }
              }

              /*  If monitoring is not possible becaucase we are */
              /*  unable to get good ratios, we should reset the */
              /*  calibration since something has changed and the */
              /*  strokes are not inaccurate */
              if (mag_calib->window_calib.orient_cal_reset_cnt > 10U) {
                /*  Reset the calibration */
                reset_calibration(mag_calib);
                add_reason_code(mag_calib_orientation_reset, reason_codes);

                /*  Reset the window stats */
                mag_calib->window_calib.orient_reset_cnt = 0U;
                mag_calib->window_calib.orient_cal_reset_cnt = 0U;
                mag_calib->window_calib.orient_xy_sync_window_cnt = 0UL;
                mag_calib->window_calib.orient_xy_nosync_window_cnt = 0UL;
                mag_calib->window_calib.orient_xz_sync_window_cnt = 0UL;
                mag_calib->window_calib.orient_xz_nosync_window_cnt = 0UL;

                /*  For debug */
              }
            }
          }

          /*  Start a new sampling period */
          mag_calib->window_calib.orient_win_cnt = 0U;
          if (mag_calib->window_calib.orient_reset_cnt > 5U) {
            mag_calib->window_calib.orient_reset_cnt = 0U;
            mag_calib->window_calib.orient_xy_sync_window_cnt = 0UL;
            mag_calib->window_calib.orient_xy_nosync_window_cnt = 0UL;
            mag_calib->window_calib.orient_xz_sync_window_cnt = 0UL;
            mag_calib->window_calib.orient_xz_nosync_window_cnt = 0UL;
          }
        }
      }

      /*         %% -- Orientation is now calibrated */
      if (mag_calib->orientation_calibrated != 0) {
        /*             %% Offset Logic */
        /*  Only run this if magnet is present - this could have been reset */
        /*  with the major change logic above */
        if (mag_calib->offset_calibrated == 0) {
          /*  THIS IS THE OFFSET CALIBRATION STATE */
          if (mag_calib->window_calib.offset_win_cnt > 848U) {
            if ((mag_calib->orientation_calibrated != 0) &&
                (mag_calib->window_calib.x_max_val != MIN_int16_T) &&
                (mag_calib->window_calib.x_min_val != MAX_int16_T) &&
                (mag_calib->window_calib.y_max_val != MIN_int16_T) &&
                (mag_calib->window_calib.y_min_val != MAX_int16_T) &&
                (mag_calib->window_calib.z_max_val != MIN_int16_T) &&
                (mag_calib->window_calib.z_min_val != MAX_int16_T)) {
              /*  Set offset calibration */
              set_offset_calibration(mag_calib);
              add_reason_code(mag_calib_offset_calibrated, reason_codes);

              /*  Calibration is finished - indicate that the */
              /*  calibration has changed */
              mag_calib->calibration_changed = 1U;
            }

            /*  Reset the window count */
            mag_calib->window_calib.offset_win_cnt = 0U;
            mag_calib->window_calib.offset_new_value_diff = 0L;
          }
        } else {
          if ((mag_calib->offset_calibrated != 0) &&
              (mag_calib->window_calib.offset_win_cnt > 212U)) {
            /*  THIS IS OFFSET MONITORING */
            /*  Allow for some updates to the range in case there are */
            /*  new scenarios that occur after calibration */
            if (mag_calib->window_calib.offset_new_value_diff > 50L) {
              add_reason_code(mag_calib_new_offset_value_l1, reason_codes);
            } else {
              if (mag_calib->window_calib.offset_new_value_diff > 200L) {
                /*  Something major changed - reset the calibration */
                reset_calibration(mag_calib);
                add_reason_code(mag_calib_major_change_reset, reason_codes);

                /*  For debug */
              }
            }

            /*  Reset the window count */
            mag_calib->window_calib.offset_win_cnt = 0U;
            mag_calib->window_calib.offset_new_value_diff = 0L;
          }
        }
      }
    }
  }
}

/*
 * File trailer for magnetometerCalibration.c
 *
 * [EOF]
 */
