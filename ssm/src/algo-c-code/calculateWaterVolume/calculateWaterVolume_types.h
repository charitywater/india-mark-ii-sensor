/*
 * File: calculateWaterVolume_types.h
 *
 * MATLAB Coder version            : 5.0
 * C/C++ source code generated on  : 18-May-2021 11:58:35
 */

#ifndef CALCULATEWATERVOLUME_TYPES_H
#define CALCULATEWATERVOLUME_TYPES_H

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

#ifndef typedef_b_padBlock_t
#define typedef_b_padBlock_t

typedef struct {
  uint16_T pad1[50];
  uint16_T pad2[50];
  uint16_T pad3[50];
  uint16_T pad4[50];
  uint16_T pad5[50];
  uint16_T pad6[50];
  uint16_T pad7[50];
  uint16_T pad8[50];
} b_padBlock_t;

#endif                                 /*typedef_b_padBlock_t*/

#ifndef typedef_padBlock_t
#define typedef_padBlock_t

typedef struct {
  uint16_T pad1[20];
  uint16_T pad2[20];
  uint16_T pad3[20];
  uint16_T pad4[20];
  uint16_T pad5[20];
  uint16_T pad6[20];
  uint16_T pad7[20];
  uint16_T pad8[20];
} padBlock_t;

#endif                                 /*typedef_padBlock_t*/

#ifndef typedef_padWaterState_t
#define typedef_padWaterState_t

typedef struct {
  PresentType present_type;
  uint8_T draining_count;
} padWaterState_t;

#endif                                 /*typedef_padWaterState_t*/

#ifndef typedef_padWindows_t
#define typedef_padWindows_t

typedef struct {
  padBlock_t blockA;
  padBlock_t blockB;
  b_padBlock_t blockOA;
  b_padBlock_t blockOB;
  uint16_T write_idx;
  WindowBlock write_block;
  Window read_window;
  uint8_T process;
  uint8_T first_pass;
} padWindows_t;

#endif                                 /*typedef_padWindows_t*/

#ifndef typedef_waterCalibration_t
#define typedef_waterCalibration_t

typedef struct {
  int16_T pad_1_calib[2];
  int16_T pad_2_calib[2];
  int16_T pad_3_calib[2];
  int16_T pad_4_calib[2];
  int16_T pad_5_calib[2];
  int16_T pad_6_calib[2];
  int16_T pad_7_calib[2];
  int16_T pad_8_calib[2];
  uint8_T pad_1_calib_done;
  uint8_T pad_2_calib_done;
  uint8_T pad_3_calib_done;
  uint8_T pad_4_calib_done;
  uint8_T pad_5_calib_done;
  uint8_T pad_6_calib_done;
  uint8_T pad_7_calib_done;
  uint8_T pad_8_calib_done;
} waterCalibration_t;

#endif                                 /*typedef_waterCalibration_t*/

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
 * File trailer for calculateWaterVolume_types.h
 *
 * [EOF]
 */
