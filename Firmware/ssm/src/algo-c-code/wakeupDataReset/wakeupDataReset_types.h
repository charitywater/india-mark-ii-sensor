/*
 * File: wakeupDataReset_types.h
 *
 * MATLAB Coder version            : 5.0
 * C/C++ source code generated on  : 18-May-2021 11:57:23
 */

#ifndef WAKEUPDATARESET_TYPES_H
#define WAKEUPDATARESET_TYPES_H

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

#ifndef typedef_magBlock_t
#define typedef_magBlock_t

typedef struct {
  int16_T x_lsb[20];
  int16_T y_lsb[20];
  int16_T z_lsb[20];
} magBlock_t;

#endif                                 /*typedef_magBlock_t*/

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

#ifndef typedef_padFilteringData_t
#define typedef_padFilteringData_t

typedef struct {
  uint8_T buffer_idx;
  int16_T pad_1_buffer[8];
  int16_T pad_2_buffer[8];
  int16_T pad_3_buffer[8];
  int16_T pad_4_buffer[8];
  int16_T pad_5_buffer[8];
  int16_T pad_6_buffer[8];
  int16_T pad_7_buffer[8];
  int16_T pad_8_buffer[8];
} padFilteringData_t;

#endif                                 /*typedef_padFilteringData_t*/

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
 * File trailer for wakeupDataReset_types.h
 *
 * [EOF]
 */
