/*
 * File: hourlyWaterVolume_types.h
 *
 * MATLAB Coder version            : 5.0
 * C/C++ source code generated on  : 18-May-2021 11:58:16
 */

#ifndef HOURLYWATERVOLUME_TYPES_H
#define HOURLYWATERVOLUME_TYPES_H

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

#ifndef typedef_hourlyWaterInfo_t
#define typedef_hourlyWaterInfo_t

typedef struct {
  int32_T volume;
  int32_T percent_pump_usage;
} hourlyWaterInfo_t;

#endif                                 /*typedef_hourlyWaterInfo_t*/

#ifndef typedef_padWaterState_t
#define typedef_padWaterState_t

typedef struct {
  PresentType present_type;
  uint8_T draining_count;
} padWaterState_t;

#endif                                 /*typedef_padWaterState_t*/

#ifndef typedef_pumpUsage_t
#define typedef_pumpUsage_t

typedef struct {
  int32_T hourly_usage[24];
  uint8_T daily_usage[7];
  uint8_T is_first;
  uint8_T first_hour;
  uint8_T is_filling_hourly_usage;
  uint8_T first_day;
  uint8_T is_filling_daily_usage;
  uint8_T prev_day_max_usage_hour;
} pumpUsage_t;

#endif                                 /*typedef_pumpUsage_t*/

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
 * File trailer for hourlyWaterVolume_types.h
 *
 * [EOF]
 */
