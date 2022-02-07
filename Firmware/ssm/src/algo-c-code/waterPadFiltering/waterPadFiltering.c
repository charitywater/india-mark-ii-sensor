/*
 * File: waterPadFiltering.c
 *
 * MATLAB Coder version            : 5.0
 * C/C++ source code generated on  : 18-May-2021 11:58:07
 */

/* Include Files */
#include "waterPadFiltering.h"

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
 * Arguments    : const padSample_t *pad_sample
 *                padFilteringData_t *pad_filtering_data
 *                padSample_t *filtered_pad_sample
 * Return Type  : void
 */
void waterPadFiltering(const padSample_t *pad_sample, padFilteringData_t
  *pad_filtering_data, padSample_t *filtered_pad_sample)
{
  int16_T buffer_1_sum;
  int16_T buffer_2_sum;
  int16_T buffer_3_sum;
  int16_T buffer_4_sum;
  int16_T buffer_5_sum;
  int16_T buffer_6_sum;
  int16_T buffer_7_sum;
  int16_T buffer_8_sum;
  int16_T i;
  int16_T b_i;

  /*  Check inputs */
  /*  Variable to store the filtered pad data */
  /*  variables to stores the buffer sums */
  buffer_1_sum = 0;
  buffer_2_sum = 0;
  buffer_3_sum = 0;
  buffer_4_sum = 0;
  buffer_5_sum = 0;
  buffer_6_sum = 0;
  buffer_7_sum = 0;
  buffer_8_sum = 0;

  /*  If the buffer index is at 7 we add the new point, average and shift the */
  /*  buffer and remain at index of 7 */
  if (pad_filtering_data->buffer_idx == 7) {
    /*  Add the newest point to the buffers */
    pad_filtering_data->pad_1_buffer[7] = pad_sample->pad1;
    pad_filtering_data->pad_2_buffer[7] = pad_sample->pad2;
    pad_filtering_data->pad_3_buffer[7] = pad_sample->pad3;
    pad_filtering_data->pad_4_buffer[7] = pad_sample->pad4;
    pad_filtering_data->pad_5_buffer[7] = pad_sample->pad5;
    pad_filtering_data->pad_6_buffer[7] = pad_sample->pad6;
    pad_filtering_data->pad_7_buffer[7] = pad_sample->pad7;
    pad_filtering_data->pad_8_buffer[7] = pad_sample->pad8;

    /*  Sum the data before bitshifting and shift the buffer */
    for (i = 0; i < 8; i++) {
      buffer_1_sum += pad_filtering_data->pad_1_buffer[i];
      buffer_2_sum += pad_filtering_data->pad_2_buffer[i];
      buffer_3_sum += pad_filtering_data->pad_3_buffer[i];
      buffer_4_sum += pad_filtering_data->pad_4_buffer[i];
      buffer_5_sum += pad_filtering_data->pad_5_buffer[i];
      buffer_6_sum += pad_filtering_data->pad_6_buffer[i];
      buffer_7_sum += pad_filtering_data->pad_7_buffer[i];
      buffer_8_sum += pad_filtering_data->pad_8_buffer[i];
      if (i + 1 > 1) {
        b_i = i - 1;
        pad_filtering_data->pad_1_buffer[b_i] = pad_filtering_data->
          pad_1_buffer[i];
        pad_filtering_data->pad_2_buffer[b_i] = pad_filtering_data->
          pad_2_buffer[i];
        pad_filtering_data->pad_3_buffer[b_i] = pad_filtering_data->
          pad_3_buffer[i];
        pad_filtering_data->pad_4_buffer[b_i] = pad_filtering_data->
          pad_4_buffer[i];
        pad_filtering_data->pad_5_buffer[b_i] = pad_filtering_data->
          pad_5_buffer[i];
        pad_filtering_data->pad_6_buffer[b_i] = pad_filtering_data->
          pad_6_buffer[i];
        pad_filtering_data->pad_7_buffer[b_i] = pad_filtering_data->
          pad_7_buffer[i];
        pad_filtering_data->pad_8_buffer[b_i] = pad_filtering_data->
          pad_8_buffer[i];
      }
    }

    /*  Use bitshifting to get the average datapoint */
    filtered_pad_sample->pad1 = (int16_T)((uint16_T)buffer_1_sum >> 3);

    /*  Inexpensive divide operation */
    filtered_pad_sample->pad2 = (int16_T)((uint16_T)buffer_2_sum >> 3);

    /*  Inexpensive divide operation        */
    filtered_pad_sample->pad3 = (int16_T)((uint16_T)buffer_3_sum >> 3);

    /*  Inexpensive divide operation        */
    filtered_pad_sample->pad4 = (int16_T)((uint16_T)buffer_4_sum >> 3);

    /*  Inexpensive divide operation        */
    filtered_pad_sample->pad5 = (int16_T)((uint16_T)buffer_5_sum >> 3);

    /*  Inexpensive divide operation        */
    filtered_pad_sample->pad6 = (int16_T)((uint16_T)buffer_6_sum >> 3);

    /*  Inexpensive divide operation        */
    filtered_pad_sample->pad7 = (int16_T)((uint16_T)buffer_7_sum >> 3);

    /*  Inexpensive divide operation        */
    filtered_pad_sample->pad8 = (int16_T)((uint16_T)buffer_8_sum >> 3);

    /*  Inexpensive divide operation        */
  } else {
    /*  Increment the buffer variable first */
    pad_filtering_data->buffer_idx++;

    /*  Add the newest point to the buffers */
    pad_filtering_data->pad_1_buffer[pad_filtering_data->buffer_idx - 1] =
      pad_sample->pad1;
    pad_filtering_data->pad_2_buffer[pad_filtering_data->buffer_idx - 1] =
      pad_sample->pad2;
    pad_filtering_data->pad_3_buffer[pad_filtering_data->buffer_idx - 1] =
      pad_sample->pad3;
    pad_filtering_data->pad_4_buffer[pad_filtering_data->buffer_idx - 1] =
      pad_sample->pad4;
    pad_filtering_data->pad_5_buffer[pad_filtering_data->buffer_idx - 1] =
      pad_sample->pad5;
    pad_filtering_data->pad_6_buffer[pad_filtering_data->buffer_idx - 1] =
      pad_sample->pad6;
    pad_filtering_data->pad_7_buffer[pad_filtering_data->buffer_idx - 1] =
      pad_sample->pad7;
    pad_filtering_data->pad_8_buffer[pad_filtering_data->buffer_idx - 1] =
      pad_sample->pad8;

    /*  Return current data without filtering */
    *filtered_pad_sample = *pad_sample;
  }
}

/*
 * File trailer for waterPadFiltering.c
 *
 * [EOF]
 */
