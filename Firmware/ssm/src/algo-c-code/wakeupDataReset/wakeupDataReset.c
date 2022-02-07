/*
 * File: wakeupDataReset.c
 *
 * MATLAB Coder version            : 5.0
 * C/C++ source code generated on  : 18-May-2021 11:57:23
 */

/* Include Files */
#include "wakeupDataReset.h"

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
 * Arguments    : padWindows_t *pad_windows
 *                magWindows_t *mag_windows
 *                waterAlgoData_t *algo_data
 *                padFilteringData_t *Pad_Filter_Data
 * Return Type  : void
 */
void wakeupDataReset(padWindows_t *pad_windows, magWindows_t *mag_windows,
                     waterAlgoData_t *algo_data, padFilteringData_t
                     *Pad_Filter_Data)
{
  int16_T i;

  /*  Keep calibration but reset all windowsw, pad filter, and session info */
  /*  Reset windows */
  pad_windows->write_idx = 0U;
  pad_windows->write_block = b_blockOA;
  pad_windows->read_window = no_window;
  pad_windows->process = 0U;
  pad_windows->first_pass = 1U;
  mag_windows->write_idx = 0U;
  mag_windows->write_block = b_blockOA;
  mag_windows->read_window = no_window;
  mag_windows->process = 0U;
  mag_windows->first_pass = 1U;

  /*  Reset pad filtering buffers */
  Pad_Filter_Data->buffer_idx = 0U;
  for (i = 0; i < 8; i++) {
    Pad_Filter_Data->pad_1_buffer[i] = 0;
    Pad_Filter_Data->pad_2_buffer[i] = 0;
    Pad_Filter_Data->pad_3_buffer[i] = 0;
    Pad_Filter_Data->pad_4_buffer[i] = 0;
    Pad_Filter_Data->pad_5_buffer[i] = 0;
    Pad_Filter_Data->pad_6_buffer[i] = 0;
    Pad_Filter_Data->pad_7_buffer[i] = 0;
    Pad_Filter_Data->pad_8_buffer[i] = 0;
  }

  /*  Reset the session data - NOTE: an session should have ended before going */
  /*  to sleep... */
  algo_data->present = 0U;
  algo_data->water_stopped = 0U;
  algo_data->pad_8_stop_flag = 0U;
  algo_data->not_present_counter = 0U;
  algo_data->constant_height_counter = 0U;
  algo_data->prev_water_height = 0L;
  algo_data->water_cal_error_count = 0U;

  /*  Reset the OA Counter back to 0  */
  algo_data->OA_counter = 0U;

  /*  Reset the pad water state */
  algo_data->pad1_present.present_type = water_not_present;
  algo_data->pad1_present.draining_count = 0U;

  /*  Reset the pad water state */
  algo_data->pad2_present.present_type = water_not_present;
  algo_data->pad2_present.draining_count = 0U;

  /*  Reset the pad water state */
  algo_data->pad3_present.present_type = water_not_present;
  algo_data->pad3_present.draining_count = 0U;

  /*  Reset the pad water state */
  algo_data->pad4_present.present_type = water_not_present;
  algo_data->pad4_present.draining_count = 0U;

  /*  Reset the pad water state */
  algo_data->pad5_present.present_type = water_not_present;
  algo_data->pad5_present.draining_count = 0U;

  /*  Reset the pad water state */
  algo_data->pad6_present.present_type = water_not_present;
  algo_data->pad6_present.draining_count = 0U;

  /*  Reset the pad water state */
  algo_data->pad7_present.present_type = water_not_present;
  algo_data->pad7_present.draining_count = 0U;

  /*  Reset the pad water state */
  algo_data->pad8_present.present_type = water_not_present;
  algo_data->pad8_present.draining_count = 0U;
  algo_data->water_int_value = 0L;
  algo_data->session_sample_counter = 0L;
  algo_data->no_change_counter = 0L;

  /*  Go to water present state */
  algo_data->algo_state = b_water_present;
}

/*
 * File trailer for wakeupDataReset.c
 *
 * [EOF]
 */
