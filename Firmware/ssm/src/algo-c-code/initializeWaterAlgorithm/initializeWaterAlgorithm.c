/*
 * File: initializeWaterAlgorithm.c
 *
 * MATLAB Coder version            : 5.0
 * C/C++ source code generated on  : 27-Oct-2022 08:09:38
 */

/* Include Files */
#include "initializeWaterAlgorithm.h"

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
 * Arguments    : waterAlgoData_t *Algorithm_Data
 *                waterCalibration_t *Calib_Data
 *                padFilteringData_t *Pad_Filter_Data
 *                pumpUsage_t *pump_usage
 * Return Type  : void
 */
void initializeWaterAlgorithm(waterAlgoData_t *Algorithm_Data,
  waterCalibration_t *Calib_Data, padFilteringData_t *Pad_Filter_Data,
  pumpUsage_t *pump_usage)
{
  int16_T i;

  /*  Reset the pad filtering buffers */
  Pad_Filter_Data->buffer_idx = 0U;

  /*  Reset the algorithm data */
  /*  Reset flags */
  Algorithm_Data->algo_state = b_water_present;
  Algorithm_Data->present_start_idx = 0U;
  Algorithm_Data->present_stop_idx = 0U;
  Algorithm_Data->present = 0U;
  Algorithm_Data->water_stopped = 0U;
  Algorithm_Data->pad_8_stop_flag = 0U;
  Algorithm_Data->not_present_counter = 0U;
  Algorithm_Data->constant_height_counter = 0U;
  Algorithm_Data->prev_water_height = 0L;
  Algorithm_Data->water_cal_error_count = 0U;

  /*  Reset the OA Counter back to 0  */
  /*  Reset the pad water state */
  Algorithm_Data->pad1_present.present_type = water_not_present;
  Algorithm_Data->pad1_present.draining_count = 0U;

  /*  Reset the pad water state */
  Algorithm_Data->pad2_present.present_type = water_not_present;
  Algorithm_Data->pad2_present.draining_count = 0U;

  /*  Reset the pad water state */
  Algorithm_Data->pad3_present.present_type = water_not_present;
  Algorithm_Data->pad3_present.draining_count = 0U;

  /*  Reset the pad water state */
  Algorithm_Data->pad4_present.present_type = water_not_present;
  Algorithm_Data->pad4_present.draining_count = 0U;

  /*  Reset the pad water state */
  Algorithm_Data->pad5_present.present_type = water_not_present;
  Algorithm_Data->pad5_present.draining_count = 0U;

  /*  Reset the pad water state */
  Algorithm_Data->pad6_present.present_type = water_not_present;
  Algorithm_Data->pad6_present.draining_count = 0U;

  /*  Reset the pad water state */
  Algorithm_Data->pad7_present.present_type = water_not_present;
  Algorithm_Data->pad7_present.draining_count = 0U;

  /*  Reset the pad water state */
  Algorithm_Data->pad8_present.present_type = water_not_present;
  Algorithm_Data->pad8_present.draining_count = 0U;
  Algorithm_Data->water_int_value = 0L;
  Algorithm_Data->session_sample_counter = 0L;
  Algorithm_Data->no_change_counter = 0L;

  /*  Reset the open air values */
  Algorithm_Data->pad1_OA = 0;
  Algorithm_Data->pad2_OA = 0;
  Algorithm_Data->pad3_OA = 0;
  Algorithm_Data->pad4_OA = 0;
  Algorithm_Data->pad5_OA = 0;
  Algorithm_Data->pad6_OA = 0;
  Algorithm_Data->pad7_OA = 0;
  Algorithm_Data->pad8_OA = 0;
  Algorithm_Data->delta_buffer.buffer_idx = 0U;
  for (i = 0; i < 6; i++) {
    Pad_Filter_Data->pad_1_buffer[i] = 0;
    Pad_Filter_Data->pad_2_buffer[i] = 0;
    Pad_Filter_Data->pad_3_buffer[i] = 0;
    Pad_Filter_Data->pad_4_buffer[i] = 0;
    Pad_Filter_Data->pad_5_buffer[i] = 0;
    Pad_Filter_Data->pad_6_buffer[i] = 0;
    Pad_Filter_Data->pad_7_buffer[i] = 0;
    Pad_Filter_Data->pad_8_buffer[i] = 0;
    Algorithm_Data->delta_buffer.pad_1_buffer[i] = 0;
    Algorithm_Data->delta_buffer.pad_2_buffer[i] = 0;
    Algorithm_Data->delta_buffer.pad_3_buffer[i] = 0;
    Algorithm_Data->delta_buffer.pad_4_buffer[i] = 0;
    Algorithm_Data->delta_buffer.pad_5_buffer[i] = 0;
    Algorithm_Data->delta_buffer.pad_6_buffer[i] = 0;
    Algorithm_Data->delta_buffer.pad_7_buffer[i] = 0;
    Algorithm_Data->delta_buffer.pad_8_buffer[i] = 0;
  }

  /*  Reset volume data */
  Algorithm_Data->water_volume_sum = 0L;
  Algorithm_Data->accum_processed_sample_cnt = 0L;
  Algorithm_Data->accum_water_sample_cnt = 0L;

  /*  For the first run we set OA counter to 300 to allow us to use the current */
  /*  open air values */
  Algorithm_Data->OA_counter = 300U;

  /*  Reset water calibration variables */
  Calib_Data->pad_1_calib[0] = 0;
  Calib_Data->pad_2_calib[0] = 0;
  Calib_Data->pad_3_calib[0] = 0;
  Calib_Data->pad_4_calib[0] = 0;
  Calib_Data->pad_5_calib[0] = 0;
  Calib_Data->pad_6_calib[0] = 0;
  Calib_Data->pad_7_calib[0] = 0;
  Calib_Data->pad_8_calib[0] = 0;
  Calib_Data->pad_1_calib[1] = 0;
  Calib_Data->pad_2_calib[1] = 0;
  Calib_Data->pad_3_calib[1] = 0;
  Calib_Data->pad_4_calib[1] = 0;
  Calib_Data->pad_5_calib[1] = 0;
  Calib_Data->pad_6_calib[1] = 0;
  Calib_Data->pad_7_calib[1] = 0;
  Calib_Data->pad_8_calib[1] = 0;
  Calib_Data->pad_1_calib_done = 0U;
  Calib_Data->pad_2_calib_done = 0U;
  Calib_Data->pad_3_calib_done = 0U;
  Calib_Data->pad_4_calib_done = 0U;
  Calib_Data->pad_5_calib_done = 0U;
  Calib_Data->pad_6_calib_done = 0U;
  Calib_Data->pad_7_calib_done = 0U;
  Calib_Data->pad_8_calib_done = 0U;

  /*  Reset pump usage data */
  pump_usage->is_first = 1U;
  pump_usage->first_hour = 0U;
  pump_usage->is_filling_hourly_usage = 0U;
  pump_usage->first_day = 0U;
  pump_usage->is_filling_daily_usage = 0U;
  pump_usage->prev_day_max_usage_hour = 0U;
}

/*
 * File trailer for initializeWaterAlgorithm.c
 *
 * [EOF]
 */
