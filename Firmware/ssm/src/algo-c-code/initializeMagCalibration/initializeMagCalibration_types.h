/*
 * File: initializeMagCalibration_types.h
 *
 * MATLAB Coder version            : 5.0
 * C/C++ source code generated on  : 27-Oct-2022 08:11:44
 */

#ifndef INITIALIZEMAGCALIBRATION_TYPES_H
#define INITIALIZEMAGCALIBRATION_TYPES_H

/* Include Files */
#include "rtwtypes.h"

/* Type Definitions */
#ifndef typedef_Orientation
#define typedef_Orientation

typedef uint8_T Orientation;

#endif                                 /*typedef_Orientation*/

#ifndef Orientation_constants
#define Orientation_constants

/* enum Orientation */
#define no_orientation                 ((Orientation)0U)
#define positive                       ((Orientation)1U)
#define negative                       ((Orientation)2U)
#endif                                 /*Orientation_constants*/

#ifndef typedef_windowCalibration_t
#define typedef_windowCalibration_t

typedef struct {
  uint16_T mag_pres_win_cnt;
  uint16_T mag_pres_pos_win_cnt;
  uint16_T mag_pres_neg_win_cnt;
  uint16_T orient_win_cnt;
  uint16_T orient_reset_cnt;
  uint16_T orient_cal_reset_cnt;
  uint32_T orient_xy_sync_window_cnt;
  uint32_T orient_xy_nosync_window_cnt;
  uint32_T orient_xz_sync_window_cnt;
  uint32_T orient_xz_nosync_window_cnt;
  uint16_T offset_win_cnt;
  int32_T offset_new_value_diff;
  int16_T x_max_val;
  int16_T x_min_val;
  int16_T y_max_val;
  int16_T y_min_val;
  int16_T z_max_val;
  int16_T z_min_val;
} windowCalibration_t;

#endif                                 /*typedef_windowCalibration_t*/

#ifndef typedef_magCalibration_t
#define typedef_magCalibration_t

typedef struct {
  Orientation x_orientation;
  Orientation y_orientation;
  Orientation z_orientation;
  int16_T x_offset;
  int16_T y_offset;
  int16_T z_offset;
  uint8_T orientation_calibrated;
  uint8_T offset_calibrated;
  int16_T max_val;
  int16_T min_val;
  uint8_T magnet_present;
  uint8_T calibration_changed;
  windowCalibration_t window_calib;
} magCalibration_t;

#endif                                 /*typedef_magCalibration_t*/
#endif

/*
 * File trailer for initializeMagCalibration_types.h
 *
 * [EOF]
 */
