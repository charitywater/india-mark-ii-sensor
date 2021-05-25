/*
 * File: magnetometerCalibration_types.h
 *
 * MATLAB Coder version            : 5.0
 * C/C++ source code generated on  : 18-May-2021 11:59:40
 */

#ifndef MAGNETOMETERCALIBRATION_TYPES_H
#define MAGNETOMETERCALIBRATION_TYPES_H

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

#ifndef typedef_PeakType
#define typedef_PeakType

typedef uint8_T PeakType;

#endif                                 /*typedef_PeakType*/

#ifndef PeakType_constants
#define PeakType_constants

/* enum PeakType */
#define no_peak                        ((PeakType)0U)
#define peak                           ((PeakType)1U)
#define valley                         ((PeakType)2U)
#endif                                 /*PeakType_constants*/

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
#define water_flow_clogged_pump        ((ReasonCodes)12U)
#define water_volume_capped            ((ReasonCodes)13U)
#define mag_calib_magnet_present       ((ReasonCodes)20U)
#define c_mag_calib_orientation_calibra ((ReasonCodes)21U)
#define mag_calib_offset_calibrated    ((ReasonCodes)22U)
#define mag_calib_present_reset        ((ReasonCodes)23U)
#define mag_calib_orientation_reset    ((ReasonCodes)24U)
#define mag_calib_major_change_reset   ((ReasonCodes)25U)
#define mag_calib_new_offset_value_l1  ((ReasonCodes)26U)
#define mag_calib_new_offset_value_l2  ((ReasonCodes)27U)
#define stroke_trans_buffer_overflow   ((ReasonCodes)30U)
#define stroke_buffer_overflow         ((ReasonCodes)31U)
#endif                                 /*ReasonCodes_constants*/

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
  uint16_T orient_xy_sync_window_cnt;
  uint16_T orient_xy_nosync_window_cnt;
  uint16_T orient_xz_sync_window_cnt;
  uint16_T orient_xz_nosync_window_cnt;
  uint16_T offset_win_cnt;
  uint8_T offset_major_change;
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

#ifndef typedef_padWaterState_t
#define typedef_padWaterState_t

typedef struct {
  PresentType present_type;
  uint8_T draining_count;
} padWaterState_t;

#endif                                 /*typedef_padWaterState_t*/

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
  AlgoState algo_state;
} waterAlgoData_t;

#endif                                 /*typedef_waterAlgoData_t*/
#endif

/*
 * File trailer for magnetometerCalibration_types.h
 *
 * [EOF]
 */
