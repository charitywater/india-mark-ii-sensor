/*
 * File: detectStrokes_types.h
 *
 * MATLAB Coder version            : 5.0
 * C/C++ source code generated on  : 27-Oct-2022 08:13:00
 */

#ifndef DETECTSTROKES_TYPES_H
#define DETECTSTROKES_TYPES_H

/* Include Files */
#include "rtwtypes.h"

/* Type Definitions */
#ifndef typedef_AlgoState
#define typedef_AlgoState

typedef uint8_T AlgoState;

#endif                                 /*typedef_AlgoState*/

#ifndef AlgoState_constants
#define AlgoState_constants

/* enum AlgoState */
#define b_water_present                ((AlgoState)0U)
#define water_volume                   ((AlgoState)1U)
#endif                                 /*AlgoState_constants*/

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

#ifndef typedef_PresentType
#define typedef_PresentType

typedef uint8_T PresentType;

#endif                                 /*typedef_PresentType*/

#ifndef PresentType_constants
#define PresentType_constants

/* enum PresentType */
#define water_not_present              ((PresentType)0U)
#define water_draining                 ((PresentType)1U)
#define water_present                  ((PresentType)2U)
#endif                                 /*PresentType_constants*/

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

#ifndef typedef_StrokeType
#define typedef_StrokeType

typedef uint8_T StrokeType;

#endif                                 /*typedef_StrokeType*/

#ifndef StrokeType_constants
#define StrokeType_constants

/* enum StrokeType */
#define no_stroke                      ((StrokeType)0U)
#define rod_up                         ((StrokeType)1U)
#define rod_down                       ((StrokeType)2U)
#endif                                 /*StrokeType_constants*/

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

#ifndef typedef_accumStrokeCount_t
#define typedef_accumStrokeCount_t

typedef struct {
  uint16_T num_windows_processed;
  uint16_T wet_stroke_count_sum;
  uint32_T wet_percent_displacement_sum;
  uint16_T dry_stroke_count_sum;
  uint32_T dry_percent_displacement_sum;
  uint8_T mag_calibration_changed;
} accumStrokeCount_t;

#endif                                 /*typedef_accumStrokeCount_t*/

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

#ifndef typedef_strokeTransition_t
#define typedef_strokeTransition_t

typedef struct {
  TransitionType type;
  int16_T val;
  int16_T idx;
} strokeTransition_t;

#endif                                 /*typedef_strokeTransition_t*/

#ifndef typedef_stroke_t
#define typedef_stroke_t

typedef struct {
  StrokeType type;
  int16_T val;
  int16_T idx;
  int16_T duration;
  int16_T percent_displacement;
  uint8_T dry;
} stroke_t;

#endif                                 /*typedef_stroke_t*/

#ifndef typedef_padFilteringData_t
#define typedef_padFilteringData_t

typedef struct {
  uint8_T buffer_idx;
  int16_T pad_1_buffer[6];
  int16_T pad_2_buffer[6];
  int16_T pad_3_buffer[6];
  int16_T pad_4_buffer[6];
  int16_T pad_5_buffer[6];
  int16_T pad_6_buffer[6];
  int16_T pad_7_buffer[6];
  int16_T pad_8_buffer[6];
} padFilteringData_t;

#endif                                 /*typedef_padFilteringData_t*/

#ifndef typedef_padWaterState_t
#define typedef_padWaterState_t

typedef struct {
  PresentType present_type;
  uint8_T draining_count;
} padWaterState_t;

#endif                                 /*typedef_padWaterState_t*/

#ifndef typedef_strokeBuffer_t
#define typedef_strokeBuffer_t

typedef struct {
  uint8_T idx;
  stroke_t buff[30];
} strokeBuffer_t;

#endif                                 /*typedef_strokeBuffer_t*/

#ifndef typedef_strokeDetectInfo_t
#define typedef_strokeDetectInfo_t

typedef struct {
  uint8_T is_first;
  strokeTransition_t last_transition;
} strokeDetectInfo_t;

#endif                                 /*typedef_strokeDetectInfo_t*/

#ifndef typedef_strokeTransitionBuffer_t
#define typedef_strokeTransitionBuffer_t

typedef struct {
  uint8_T idx;
  strokeTransition_t buff[30];
} strokeTransitionBuffer_t;

#endif                                 /*typedef_strokeTransitionBuffer_t*/

#ifndef typedef_waterAlgoData_t
#define typedef_waterAlgoData_t

typedef struct {
  uint8_T present;
  uint8_T water_stopped;
  uint8_T pad_8_stop_flag;
  uint16_T present_start_idx;
  uint16_T present_stop_idx;
  padWaterState_t pad1_present;
  padWaterState_t pad2_present;
  padWaterState_t pad3_present;
  padWaterState_t pad4_present;
  padWaterState_t pad5_present;
  padWaterState_t pad6_present;
  padWaterState_t pad7_present;
  padWaterState_t pad8_present;
  int16_T pad1_OA;
  int16_T pad2_OA;
  int16_T pad3_OA;
  int16_T pad4_OA;
  int16_T pad5_OA;
  int16_T pad6_OA;
  int16_T pad7_OA;
  int16_T pad8_OA;
  int32_T water_int_value;
  int32_T water_volume_sum;
  int32_T no_change_counter;
  int32_T accum_processed_sample_cnt;
  int32_T accum_water_sample_cnt;
  int32_T session_sample_counter;
  uint16_T constant_height_counter;
  int32_T prev_water_height;
  uint8_T water_cal_error_count;
  uint16_T not_present_counter;
  uint16_T OA_counter;
  padFilteringData_t delta_buffer;
  AlgoState algo_state;
} waterAlgoData_t;

#endif                                 /*typedef_waterAlgoData_t*/
#endif

/*
 * File trailer for detectStrokes_types.h
 *
 * [EOF]
 */
