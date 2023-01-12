/*
 * File: detectTransitions_types.h
 *
 * MATLAB Coder version            : 5.0
 * C/C++ source code generated on  : 27-Oct-2022 08:12:45
 */

#ifndef DETECTTRANSITIONS_TYPES_H
#define DETECTTRANSITIONS_TYPES_H

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

#ifndef typedef_ReasonCodes
#define typedef_ReasonCodes

typedef uint8_T ReasonCodes;

#endif                                 /*typedef_ReasonCodes*/

#ifndef ReasonCodes_constants
#define ReasonCodes_constants

/* enum ReasonCodes */
#define reason_code_none               ((ReasonCodes)0U)
#define water_calib_calibrated         ((ReasonCodes)1U)
#define water_calib_reset              ((ReasonCodes)2U)
#define water_calib_neg_delta          ((ReasonCodes)3U)
#define water_bad_sample               ((ReasonCodes)10U)
#define water_flow_standing_water      ((ReasonCodes)11U)
#define mag_calib_xy_cnt_low           ((ReasonCodes)12U)
#define mag_calib_xz_cnt_low           ((ReasonCodes)13U)
#define mag_calib_magnet_present       ((ReasonCodes)20U)
#define c_mag_calib_orientation_calibra ((ReasonCodes)21U)
#define mag_calib_offset_calibrated    ((ReasonCodes)22U)
#define mag_calib_present_reset        ((ReasonCodes)23U)
#define mag_calib_orientation_reset    ((ReasonCodes)24U)
#define mag_calib_major_change_reset   ((ReasonCodes)25U)
#define mag_calib_new_offset_value_l1  ((ReasonCodes)26U)
#define mag_calib_bad_placement_wobble ((ReasonCodes)27U)
#define stroke_trans_buffer_overflow   ((ReasonCodes)30U)
#define stroke_buffer_overflow         ((ReasonCodes)31U)
#endif                                 /*ReasonCodes_constants*/

#ifndef typedef_StrokeTransitionState
#define typedef_StrokeTransitionState

typedef uint8_T StrokeTransitionState;

#endif                                 /*typedef_StrokeTransitionState*/

#ifndef StrokeTransitionState_constants
#define StrokeTransitionState_constants

/* enum StrokeTransitionState */
#define no_activity                    ((StrokeTransitionState)0U)
#define finding_peak                   ((StrokeTransitionState)1U)
#define finding_valley                 ((StrokeTransitionState)2U)
#endif                                 /*StrokeTransitionState_constants*/

#ifndef typedef_TransitionType
#define typedef_TransitionType

typedef uint8_T TransitionType;

#endif                                 /*typedef_TransitionType*/

#ifndef TransitionType_constants
#define TransitionType_constants

/* enum TransitionType */
#define no_transition                  ((TransitionType)0U)
#define no_transition_activity         ((TransitionType)1U)
#define transition_peak                ((TransitionType)2U)
#define transition_valley              ((TransitionType)3U)
#endif                                 /*TransitionType_constants*/

#ifndef typedef_Window
#define typedef_Window

typedef uint8_T Window;

#endif                                 /*typedef_Window*/

#ifndef Window_constants
#define Window_constants

/* enum Window */
#define no_window                      ((Window)0U)
#define windowA                        ((Window)1U)
#define windowB                        ((Window)2U)
#endif                                 /*Window_constants*/

#ifndef typedef_WindowBlock
#define typedef_WindowBlock

typedef uint8_T WindowBlock;

#endif                                 /*typedef_WindowBlock*/

#ifndef WindowBlock_constants
#define WindowBlock_constants

/* enum WindowBlock */
#define b_blockA                       ((WindowBlock)1U)
#define b_blockB                       ((WindowBlock)2U)
#define b_blockOA                      ((WindowBlock)3U)
#define b_blockOB                      ((WindowBlock)4U)
#endif                                 /*WindowBlock_constants*/

#ifndef typedef_b_magBlock_t
#define typedef_b_magBlock_t

typedef struct {
  int16_T x_lsb[50];
  int16_T y_lsb[50];
  int16_T z_lsb[50];
} b_magBlock_t;

#endif                                 /*typedef_b_magBlock_t*/

#ifndef typedef_magBlock_t
#define typedef_magBlock_t

typedef struct {
  int16_T x_lsb[20];
  int16_T y_lsb[20];
  int16_T z_lsb[20];
} magBlock_t;

#endif                                 /*typedef_magBlock_t*/

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

#ifndef typedef_magWindows_t
#define typedef_magWindows_t

typedef struct {
  magBlock_t blockA;
  magBlock_t blockB;
  b_magBlock_t blockOA;
  b_magBlock_t blockOB;
  uint16_T write_idx;
  WindowBlock write_block;
  Window read_window;
  uint8_T process;
  uint8_T first_pass;
} magWindows_t;

#endif                                 /*typedef_magWindows_t*/

#ifndef typedef_strokeTransition_t
#define typedef_strokeTransition_t

typedef struct {
  TransitionType type;
  int16_T val;
  int16_T idx;
} strokeTransition_t;

#endif                                 /*typedef_strokeTransition_t*/

#ifndef typedef_strokeTransitionBuffer_t
#define typedef_strokeTransitionBuffer_t

typedef struct {
  uint8_T idx;
  strokeTransition_t buff[30];
} strokeTransitionBuffer_t;

#endif                                 /*typedef_strokeTransitionBuffer_t*/

#ifndef typedef_strokeTransitionInfo_t
#define typedef_strokeTransitionInfo_t

typedef struct {
  StrokeTransitionState state;
  uint8_T is_first;
  uint8_T downslope_cnt;
  int16_T downslope_sum;
  uint8_T upslope_cnt;
  int16_T upslope_sum;
  int16_T peak_val;
  int16_T peak_idx;
  uint8_T state_switch_cnt;
  int16_T prev_val;
} strokeTransitionInfo_t;

#endif                                 /*typedef_strokeTransitionInfo_t*/
#endif

/*
 * File trailer for detectTransitions_types.h
 *
 * [EOF]
 */
