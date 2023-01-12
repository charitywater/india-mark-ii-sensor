/*
 * File: calculateWaterVolume.c
 *
 * MATLAB Coder version            : 5.0
 * C/C++ source code generated on  : 27-Oct-2022 08:10:46
 */

/* Include Files */
#include "calculateWaterVolume.h"
#include "checkWaterCalibration.h"
#include "detectWaterChange.h"
#include "promotePadStates.h"
#include "waterCalibration.h"

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

/* Type Definitions */
#ifndef typedef_Pad
#define typedef_Pad

typedef uint8_T Pad;

#endif                                 /*typedef_Pad*/

#ifndef Pad_constants
#define Pad_constants

/* enum Pad */
#define b_pad1                         ((Pad)0U)
#define b_pad2                         ((Pad)1U)
#define b_pad3                         ((Pad)2U)
#define b_pad4                         ((Pad)3U)
#define b_pad5                         ((Pad)4U)
#define b_pad6                         ((Pad)5U)
#define b_pad7                         ((Pad)6U)
#define b_pad8                         ((Pad)7U)
#endif                                 /*Pad_constants*/

/* Function Declarations */
static void add_reason_code(ReasonCodes reason_code, ReasonCodes reason_codes[8]);
static void add_to_buffer(int16_T pad_sample_pad1, int16_T pad_sample_pad2,
  int16_T pad_sample_pad3, int16_T pad_sample_pad4, int16_T pad_sample_pad5,
  int16_T pad_sample_pad6, int16_T pad_sample_pad7, int16_T pad_sample_pad8,
  padFilteringData_t *pad_filtering_data, int16_T *pad_delta_data_pad1, int16_T *
  pad_delta_data_pad2, int16_T *pad_delta_data_pad3, int16_T
  *pad_delta_data_pad4, int16_T *pad_delta_data_pad5, int16_T
  *pad_delta_data_pad6, int16_T *pad_delta_data_pad7, int16_T
  *pad_delta_data_pad8);
static void read_sample(uint16_T b_index, const uint16_T pad_window_blockA_pad1
  [20], const uint16_T pad_window_blockA_pad2[20], const uint16_T
  pad_window_blockA_pad3[20], const uint16_T pad_window_blockA_pad4[20], const
  uint16_T pad_window_blockA_pad5[20], const uint16_T pad_window_blockA_pad6[20],
  const uint16_T pad_window_blockA_pad7[20], const uint16_T
  pad_window_blockA_pad8[20], const padBlock_t *pad_window_blockB, const
  b_padBlock_t *pad_window_blockOA, const b_padBlock_t *pad_window_blockOB,
  Window pad_window_read_window, uint8_T *success, int16_T *sample_pad1, int16_T
  *sample_pad2, int16_T *sample_pad3, int16_T *sample_pad4, int16_T *sample_pad5,
  int16_T *sample_pad6, int16_T *sample_pad7, int16_T *sample_pad8);

/* Function Definitions */

/*
 * Arguments    : ReasonCodes reason_code
 *                ReasonCodes reason_codes[8]
 * Return Type  : void
 */
static void add_reason_code(ReasonCodes reason_code, ReasonCodes reason_codes[8])
{
  uint8_T found;
  int16_T i;
  int16_T b_i;
  boolean_T exitg1;

  /*  Does the reason code already exist in the list? */
  found = 0U;
  for (i = 0; i < 8; i++) {
    if (reason_code == reason_codes[i]) {
      found = 1U;
    }
  }

  /*  If it doesn't exist, add if there is a free slot */
  if (found == 0) {
    b_i = 0;
    exitg1 = false;
    while ((!exitg1) && (b_i < 8)) {
      if (reason_codes[b_i] == reason_code_none) {
        reason_codes[b_i] = reason_code;
        exitg1 = true;
      } else {
        b_i++;
      }
    }
  }
}

/*
 * Arguments    : int16_T pad_sample_pad1
 *                int16_T pad_sample_pad2
 *                int16_T pad_sample_pad3
 *                int16_T pad_sample_pad4
 *                int16_T pad_sample_pad5
 *                int16_T pad_sample_pad6
 *                int16_T pad_sample_pad7
 *                int16_T pad_sample_pad8
 *                padFilteringData_t *pad_filtering_data
 *                int16_T *pad_delta_data_pad1
 *                int16_T *pad_delta_data_pad2
 *                int16_T *pad_delta_data_pad3
 *                int16_T *pad_delta_data_pad4
 *                int16_T *pad_delta_data_pad5
 *                int16_T *pad_delta_data_pad6
 *                int16_T *pad_delta_data_pad7
 *                int16_T *pad_delta_data_pad8
 * Return Type  : void
 */
static void add_to_buffer(int16_T pad_sample_pad1, int16_T pad_sample_pad2,
  int16_T pad_sample_pad3, int16_T pad_sample_pad4, int16_T pad_sample_pad5,
  int16_T pad_sample_pad6, int16_T pad_sample_pad7, int16_T pad_sample_pad8,
  padFilteringData_t *pad_filtering_data, int16_T *pad_delta_data_pad1, int16_T *
  pad_delta_data_pad2, int16_T *pad_delta_data_pad3, int16_T
  *pad_delta_data_pad4, int16_T *pad_delta_data_pad5, int16_T
  *pad_delta_data_pad6, int16_T *pad_delta_data_pad7, int16_T
  *pad_delta_data_pad8)
{
  int16_T pad1_max;
  int16_T pad1_min;
  int16_T pad2_max;
  int16_T pad2_min;
  int16_T pad3_max;
  int16_T pad3_min;
  int16_T pad4_max;
  int16_T pad4_min;
  int16_T pad5_max;
  int16_T pad5_min;
  int16_T pad6_max;
  int16_T pad6_min;
  int16_T pad7_max;
  int16_T pad7_min;
  int16_T pad8_max;
  int16_T pad8_min;
  int16_T i;
  int16_T b_i;
  int16_T c_i;

  /*         %% */
  /*  Note the buffer uses 1-7 to store the raw values and 8 to store the past delta point  */
  /*         %% */
  /*  If the buffer index is at 7 we add the new point, average and shift the */
  /*  buffer and remain at index of 7 */
  if (pad_filtering_data->buffer_idx == 5) {
    /*  Add the newest point to the buffers */
    pad_filtering_data->pad_1_buffer[4] = pad_sample_pad1;
    pad_filtering_data->pad_2_buffer[4] = pad_sample_pad2;
    pad_filtering_data->pad_3_buffer[4] = pad_sample_pad3;
    pad_filtering_data->pad_4_buffer[4] = pad_sample_pad4;
    pad_filtering_data->pad_5_buffer[4] = pad_sample_pad5;
    pad_filtering_data->pad_6_buffer[4] = pad_sample_pad6;
    pad_filtering_data->pad_7_buffer[4] = pad_sample_pad7;
    pad_filtering_data->pad_8_buffer[4] = pad_sample_pad8;

    /*  Set the initial values for the max and mins */
    pad1_max = pad_filtering_data->pad_1_buffer[0];
    pad1_min = pad_filtering_data->pad_1_buffer[0];
    pad2_max = pad_filtering_data->pad_2_buffer[0];
    pad2_min = pad_filtering_data->pad_2_buffer[0];
    pad3_max = pad_filtering_data->pad_3_buffer[0];
    pad3_min = pad_filtering_data->pad_3_buffer[0];
    pad4_max = pad_filtering_data->pad_4_buffer[0];
    pad4_min = pad_filtering_data->pad_4_buffer[0];
    pad5_max = pad_filtering_data->pad_5_buffer[0];
    pad5_min = pad_filtering_data->pad_5_buffer[0];
    pad6_max = pad_filtering_data->pad_6_buffer[0];
    pad6_min = pad_filtering_data->pad_6_buffer[0];
    pad7_max = pad_filtering_data->pad_7_buffer[0];
    pad7_min = pad_filtering_data->pad_7_buffer[0];
    pad8_max = pad_filtering_data->pad_8_buffer[0];
    pad8_min = pad_filtering_data->pad_8_buffer[0];

    /*  Loop through current array and keep track of max/min values */
    for (i = 0; i < 5; i++) {
      if (pad_filtering_data->pad_1_buffer[i] > pad1_max) {
        pad1_max = pad_filtering_data->pad_1_buffer[i];
      } else {
        if (pad_filtering_data->pad_1_buffer[i] < pad1_min) {
          pad1_min = pad_filtering_data->pad_1_buffer[i];
        }
      }

      if (pad_filtering_data->pad_2_buffer[i] > pad2_max) {
        pad2_max = pad_filtering_data->pad_2_buffer[i];
      } else {
        if (pad_filtering_data->pad_2_buffer[i] < pad2_min) {
          pad2_min = pad_filtering_data->pad_2_buffer[i];
        }
      }

      if (pad_filtering_data->pad_3_buffer[i] > pad3_max) {
        pad3_max = pad_filtering_data->pad_3_buffer[i];
      } else {
        if (pad_filtering_data->pad_3_buffer[i] < pad3_min) {
          pad3_min = pad_filtering_data->pad_3_buffer[i];
        }
      }

      if (pad_filtering_data->pad_4_buffer[i] > pad4_max) {
        pad4_max = pad_filtering_data->pad_4_buffer[i];
      } else {
        if (pad_filtering_data->pad_4_buffer[i] < pad4_min) {
          pad4_min = pad_filtering_data->pad_4_buffer[i];
        }
      }

      if (pad_filtering_data->pad_5_buffer[i] > pad5_max) {
        pad5_max = pad_filtering_data->pad_5_buffer[i];
      } else {
        if (pad_filtering_data->pad_5_buffer[i] < pad5_min) {
          pad5_min = pad_filtering_data->pad_5_buffer[i];
        }
      }

      if (pad_filtering_data->pad_6_buffer[i] > pad6_max) {
        pad6_max = pad_filtering_data->pad_6_buffer[i];
      } else {
        if (pad_filtering_data->pad_6_buffer[i] < pad6_min) {
          pad6_min = pad_filtering_data->pad_6_buffer[i];
        }
      }

      if (pad_filtering_data->pad_7_buffer[i] > pad7_max) {
        pad7_max = pad_filtering_data->pad_7_buffer[i];
      } else {
        if (pad_filtering_data->pad_7_buffer[i] < pad7_min) {
          pad7_min = pad_filtering_data->pad_7_buffer[i];
        }
      }

      if (pad_filtering_data->pad_8_buffer[i] > pad8_max) {
        pad8_max = pad_filtering_data->pad_8_buffer[i];
      } else {
        if (pad_filtering_data->pad_8_buffer[i] < pad8_min) {
          pad8_min = pad_filtering_data->pad_8_buffer[i];
        }
      }
    }

    /*  Calculate the delta using the max and the min values and */
    /*  average with the past delta value */
    /*             %% PAD 1 DELTA AND AVERAGING */
    if (pad_sample_pad1 - pad_filtering_data->pad_1_buffer[0] < 1) {
      *pad_delta_data_pad1 = (pad1_min - pad1_max) +
        pad_filtering_data->pad_1_buffer[5];
    } else {
      *pad_delta_data_pad1 = (pad1_max - pad1_min) +
        pad_filtering_data->pad_1_buffer[5];
    }

    if (*pad_delta_data_pad1 < 0) {
      *pad_delta_data_pad1 = 1 - *pad_delta_data_pad1;
      *pad_delta_data_pad1 = -(int16_T)((uint16_T)*pad_delta_data_pad1 >> 1);
    } else {
      *pad_delta_data_pad1 = (int16_T)((uint16_T)(*pad_delta_data_pad1 + 1) >> 1);
    }

    /*             %% PAD 2 DELTA AND AVERAGING */
    if (pad_sample_pad2 - pad_filtering_data->pad_2_buffer[0] < 1) {
      *pad_delta_data_pad2 = (pad2_min - pad2_max) +
        pad_filtering_data->pad_2_buffer[5];
    } else {
      *pad_delta_data_pad2 = (pad2_max - pad2_min) +
        pad_filtering_data->pad_2_buffer[5];
    }

    if (*pad_delta_data_pad2 < 0) {
      *pad_delta_data_pad2 = 1 - *pad_delta_data_pad2;
      *pad_delta_data_pad2 = (int16_T)((uint16_T)*pad_delta_data_pad2 >> 1);
      *pad_delta_data_pad2 = -*pad_delta_data_pad2;
    } else {
      *pad_delta_data_pad2 = (int16_T)((uint16_T)(*pad_delta_data_pad2 + 1) >> 1);
    }

    /*             %% PAD 3 DELTA AND AVERAGING                 */
    if (pad_sample_pad3 - pad_filtering_data->pad_3_buffer[0] < 1) {
      *pad_delta_data_pad3 = (pad3_min - pad3_max) +
        pad_filtering_data->pad_3_buffer[5];
    } else {
      *pad_delta_data_pad3 = (pad3_max - pad3_min) +
        pad_filtering_data->pad_3_buffer[5];
    }

    if (*pad_delta_data_pad3 < 0) {
      *pad_delta_data_pad3 = 1 - *pad_delta_data_pad3;
      *pad_delta_data_pad3 = (int16_T)((uint16_T)*pad_delta_data_pad3 >> 1);
      *pad_delta_data_pad3 = -*pad_delta_data_pad3;
    } else {
      *pad_delta_data_pad3 = (int16_T)((uint16_T)(*pad_delta_data_pad3 + 1) >> 1);
    }

    /*             %% PAD 4 DELTA AND AVERAGING */
    if (pad_sample_pad4 - pad_filtering_data->pad_4_buffer[0] < 1) {
      *pad_delta_data_pad4 = (pad4_min - pad4_max) +
        pad_filtering_data->pad_4_buffer[5];
    } else {
      *pad_delta_data_pad4 = (pad4_max - pad4_min) +
        pad_filtering_data->pad_4_buffer[5];
    }

    if (*pad_delta_data_pad4 < 0) {
      *pad_delta_data_pad4 = 1 - *pad_delta_data_pad4;
      *pad_delta_data_pad4 = (int16_T)((uint16_T)*pad_delta_data_pad4 >> 1);
      *pad_delta_data_pad4 = -*pad_delta_data_pad4;
    } else {
      *pad_delta_data_pad4 = (int16_T)((uint16_T)(*pad_delta_data_pad4 + 1) >> 1);
    }

    /*             %% PAD 5 DELTA AND AVERAGING */
    if (pad_sample_pad5 - pad_filtering_data->pad_5_buffer[0] < 1) {
      *pad_delta_data_pad5 = (pad5_min - pad5_max) +
        pad_filtering_data->pad_5_buffer[5];
    } else {
      *pad_delta_data_pad5 = (pad5_max - pad5_min) +
        pad_filtering_data->pad_5_buffer[5];
    }

    if (*pad_delta_data_pad5 < 0) {
      *pad_delta_data_pad5 = 1 - *pad_delta_data_pad5;
      *pad_delta_data_pad5 = (int16_T)((uint16_T)*pad_delta_data_pad5 >> 1);
      *pad_delta_data_pad5 = -*pad_delta_data_pad5;
    } else {
      *pad_delta_data_pad5 = (int16_T)((uint16_T)(*pad_delta_data_pad5 + 1) >> 1);
    }

    /*             %% PAD 6 DELTA AND AVERAGING */
    if (pad_sample_pad6 - pad_filtering_data->pad_6_buffer[0] < 1) {
      *pad_delta_data_pad6 = (pad6_min - pad6_max) +
        pad_filtering_data->pad_6_buffer[5];
    } else {
      *pad_delta_data_pad6 = (pad6_max - pad6_min) +
        pad_filtering_data->pad_6_buffer[5];
    }

    if (*pad_delta_data_pad6 < 0) {
      *pad_delta_data_pad6 = 1 - *pad_delta_data_pad6;
      *pad_delta_data_pad6 = (int16_T)((uint16_T)*pad_delta_data_pad6 >> 1);
      *pad_delta_data_pad6 = -*pad_delta_data_pad6;
    } else {
      *pad_delta_data_pad6 = (int16_T)((uint16_T)(*pad_delta_data_pad6 + 1) >> 1);
    }

    /*             %% PAD 7 DELTA AND AVERAGING */
    if (pad_sample_pad7 - pad_filtering_data->pad_7_buffer[0] < 1) {
      *pad_delta_data_pad7 = (pad7_min - pad7_max) +
        pad_filtering_data->pad_7_buffer[5];
    } else {
      *pad_delta_data_pad7 = (pad7_max - pad7_min) +
        pad_filtering_data->pad_7_buffer[5];
    }

    if (*pad_delta_data_pad7 < 0) {
      *pad_delta_data_pad7 = 1 - *pad_delta_data_pad7;
      *pad_delta_data_pad7 = (int16_T)((uint16_T)*pad_delta_data_pad7 >> 1);
      *pad_delta_data_pad7 = -*pad_delta_data_pad7;
    } else {
      *pad_delta_data_pad7 = (int16_T)((uint16_T)(*pad_delta_data_pad7 + 1) >> 1);
    }

    /*             %% PAD 2 DELTA AND AVERAGING */
    if (pad_sample_pad8 - pad_filtering_data->pad_8_buffer[0] < 1) {
      *pad_delta_data_pad8 = (pad8_min - pad8_max) +
        pad_filtering_data->pad_8_buffer[5];
    } else {
      *pad_delta_data_pad8 = (pad8_max - pad8_min) +
        pad_filtering_data->pad_8_buffer[5];
    }

    if (*pad_delta_data_pad8 < 0) {
      *pad_delta_data_pad8 = 1 - *pad_delta_data_pad8;
      *pad_delta_data_pad8 = (int16_T)((uint16_T)*pad_delta_data_pad8 >> 1);
      *pad_delta_data_pad8 = -*pad_delta_data_pad8;
    } else {
      *pad_delta_data_pad8 = (int16_T)((uint16_T)(*pad_delta_data_pad8 + 1) >> 1);
    }

    /*  Set the current delta to the final value in the buffer to */
    /*  average the next delta point */
    pad_filtering_data->pad_1_buffer[5] = *pad_delta_data_pad1;
    pad_filtering_data->pad_2_buffer[5] = *pad_delta_data_pad2;
    pad_filtering_data->pad_3_buffer[5] = *pad_delta_data_pad3;
    pad_filtering_data->pad_4_buffer[5] = *pad_delta_data_pad4;
    pad_filtering_data->pad_5_buffer[5] = *pad_delta_data_pad5;
    pad_filtering_data->pad_6_buffer[5] = *pad_delta_data_pad6;
    pad_filtering_data->pad_7_buffer[5] = *pad_delta_data_pad7;
    pad_filtering_data->pad_8_buffer[5] = *pad_delta_data_pad8;

    /*  Sum the data before bitshifting and shift the buffer */
    for (b_i = 0; b_i < 5; b_i++) {
      if (b_i + 1 > 1) {
        c_i = b_i - 1;
        pad_filtering_data->pad_1_buffer[c_i] = pad_filtering_data->
          pad_1_buffer[b_i];
        pad_filtering_data->pad_2_buffer[c_i] = pad_filtering_data->
          pad_2_buffer[b_i];
        pad_filtering_data->pad_3_buffer[c_i] = pad_filtering_data->
          pad_3_buffer[b_i];
        pad_filtering_data->pad_4_buffer[c_i] = pad_filtering_data->
          pad_4_buffer[b_i];
        pad_filtering_data->pad_5_buffer[c_i] = pad_filtering_data->
          pad_5_buffer[b_i];
        pad_filtering_data->pad_6_buffer[c_i] = pad_filtering_data->
          pad_6_buffer[b_i];
        pad_filtering_data->pad_7_buffer[c_i] = pad_filtering_data->
          pad_7_buffer[b_i];
        pad_filtering_data->pad_8_buffer[c_i] = pad_filtering_data->
          pad_8_buffer[b_i];
      }
    }
  } else {
    /*  Increment the buffer variable first */
    pad_filtering_data->buffer_idx++;

    /*  Add the newest point to the buffers */
    pad_filtering_data->pad_1_buffer[pad_filtering_data->buffer_idx - 1] =
      pad_sample_pad1;
    pad_filtering_data->pad_2_buffer[pad_filtering_data->buffer_idx - 1] =
      pad_sample_pad2;
    pad_filtering_data->pad_3_buffer[pad_filtering_data->buffer_idx - 1] =
      pad_sample_pad3;
    pad_filtering_data->pad_4_buffer[pad_filtering_data->buffer_idx - 1] =
      pad_sample_pad4;
    pad_filtering_data->pad_5_buffer[pad_filtering_data->buffer_idx - 1] =
      pad_sample_pad5;
    pad_filtering_data->pad_6_buffer[pad_filtering_data->buffer_idx - 1] =
      pad_sample_pad6;
    pad_filtering_data->pad_7_buffer[pad_filtering_data->buffer_idx - 1] =
      pad_sample_pad7;
    pad_filtering_data->pad_8_buffer[pad_filtering_data->buffer_idx - 1] =
      pad_sample_pad8;

    /*  Return current data without filtering */
    *pad_delta_data_pad1 = 0;
    *pad_delta_data_pad2 = 0;
    *pad_delta_data_pad3 = 0;
    *pad_delta_data_pad4 = 0;
    *pad_delta_data_pad5 = 0;
    *pad_delta_data_pad6 = 0;
    *pad_delta_data_pad7 = 0;
    *pad_delta_data_pad8 = 0;
  }
}

/*
 * Arguments    : uint16_T b_index
 *                const uint16_T pad_window_blockA_pad1[20]
 *                const uint16_T pad_window_blockA_pad2[20]
 *                const uint16_T pad_window_blockA_pad3[20]
 *                const uint16_T pad_window_blockA_pad4[20]
 *                const uint16_T pad_window_blockA_pad5[20]
 *                const uint16_T pad_window_blockA_pad6[20]
 *                const uint16_T pad_window_blockA_pad7[20]
 *                const uint16_T pad_window_blockA_pad8[20]
 *                const padBlock_t *pad_window_blockB
 *                const b_padBlock_t *pad_window_blockOA
 *                const b_padBlock_t *pad_window_blockOB
 *                Window pad_window_read_window
 *                uint8_T *success
 *                int16_T *sample_pad1
 *                int16_T *sample_pad2
 *                int16_T *sample_pad3
 *                int16_T *sample_pad4
 *                int16_T *sample_pad5
 *                int16_T *sample_pad6
 *                int16_T *sample_pad7
 *                int16_T *sample_pad8
 * Return Type  : void
 */
static void read_sample(uint16_T b_index, const uint16_T pad_window_blockA_pad1
  [20], const uint16_T pad_window_blockA_pad2[20], const uint16_T
  pad_window_blockA_pad3[20], const uint16_T pad_window_blockA_pad4[20], const
  uint16_T pad_window_blockA_pad5[20], const uint16_T pad_window_blockA_pad6[20],
  const uint16_T pad_window_blockA_pad7[20], const uint16_T
  pad_window_blockA_pad8[20], const padBlock_t *pad_window_blockB, const
  b_padBlock_t *pad_window_blockOA, const b_padBlock_t *pad_window_blockOB,
  Window pad_window_read_window, uint8_T *success, int16_T *sample_pad1, int16_T
  *sample_pad2, int16_T *sample_pad3, int16_T *sample_pad4, int16_T *sample_pad5,
  int16_T *sample_pad6, int16_T *sample_pad7, int16_T *sample_pad8)
{
  int16_T sample_pad1_tmp;
  int16_T b_sample_pad1_tmp;
  int16_T c_sample_pad1_tmp;
  int16_T d_sample_pad1_tmp;
  int16_T e_sample_pad1_tmp;
  int16_T f_sample_pad1_tmp;

  /*  Static Functions  */
  *success = 1U;
  if ((b_index < 1U) || (b_index > 120U) || (pad_window_read_window == no_window))
  {
    *success = 0U;
  } else {
    /*  Find which block the index falls into */
    if (b_index <= 50U) {
      /*  Read from the first overlap block */
      if (pad_window_read_window == windowA) {
        b_sample_pad1_tmp = (int16_T)b_index - 1;
        *sample_pad1 = (int16_T)pad_window_blockOA->pad1[b_sample_pad1_tmp];
        *sample_pad2 = (int16_T)pad_window_blockOA->pad2[b_sample_pad1_tmp];
        *sample_pad3 = (int16_T)pad_window_blockOA->pad3[b_sample_pad1_tmp];
        *sample_pad4 = (int16_T)pad_window_blockOA->pad4[b_sample_pad1_tmp];
        *sample_pad5 = (int16_T)pad_window_blockOA->pad5[b_sample_pad1_tmp];
        *sample_pad6 = (int16_T)pad_window_blockOA->pad6[b_sample_pad1_tmp];
        *sample_pad7 = (int16_T)pad_window_blockOA->pad7[b_sample_pad1_tmp];
        *sample_pad8 = (int16_T)pad_window_blockOA->pad8[b_sample_pad1_tmp];
      } else {
        sample_pad1_tmp = (int16_T)b_index - 1;
        *sample_pad1 = (int16_T)pad_window_blockOB->pad1[sample_pad1_tmp];
        *sample_pad2 = (int16_T)pad_window_blockOB->pad2[sample_pad1_tmp];
        *sample_pad3 = (int16_T)pad_window_blockOB->pad3[sample_pad1_tmp];
        *sample_pad4 = (int16_T)pad_window_blockOB->pad4[sample_pad1_tmp];
        *sample_pad5 = (int16_T)pad_window_blockOB->pad5[sample_pad1_tmp];
        *sample_pad6 = (int16_T)pad_window_blockOB->pad6[sample_pad1_tmp];
        *sample_pad7 = (int16_T)pad_window_blockOB->pad7[sample_pad1_tmp];
        *sample_pad8 = (int16_T)pad_window_blockOB->pad8[sample_pad1_tmp];
      }
    } else if (b_index <= 70U) {
      /*  Read from the non-overlap block */
      if (pad_window_read_window == windowA) {
        f_sample_pad1_tmp = (int16_T)b_index - 51;
        *sample_pad1 = (int16_T)pad_window_blockA_pad1[f_sample_pad1_tmp];
        *sample_pad2 = (int16_T)pad_window_blockA_pad2[f_sample_pad1_tmp];
        *sample_pad3 = (int16_T)pad_window_blockA_pad3[f_sample_pad1_tmp];
        *sample_pad4 = (int16_T)pad_window_blockA_pad4[f_sample_pad1_tmp];
        *sample_pad5 = (int16_T)pad_window_blockA_pad5[f_sample_pad1_tmp];
        *sample_pad6 = (int16_T)pad_window_blockA_pad6[f_sample_pad1_tmp];
        *sample_pad7 = (int16_T)pad_window_blockA_pad7[f_sample_pad1_tmp];
        *sample_pad8 = (int16_T)pad_window_blockA_pad8[f_sample_pad1_tmp];
      } else {
        e_sample_pad1_tmp = (int16_T)b_index - 51;
        *sample_pad1 = (int16_T)pad_window_blockB->pad1[e_sample_pad1_tmp];
        *sample_pad2 = (int16_T)pad_window_blockB->pad2[e_sample_pad1_tmp];
        *sample_pad3 = (int16_T)pad_window_blockB->pad3[e_sample_pad1_tmp];
        *sample_pad4 = (int16_T)pad_window_blockB->pad4[e_sample_pad1_tmp];
        *sample_pad5 = (int16_T)pad_window_blockB->pad5[e_sample_pad1_tmp];
        *sample_pad6 = (int16_T)pad_window_blockB->pad6[e_sample_pad1_tmp];
        *sample_pad7 = (int16_T)pad_window_blockB->pad7[e_sample_pad1_tmp];
        *sample_pad8 = (int16_T)pad_window_blockB->pad8[e_sample_pad1_tmp];
      }
    } else {
      /*  Read from last overlap block */
      if (pad_window_read_window == windowA) {
        d_sample_pad1_tmp = (int16_T)b_index - 71;
        *sample_pad1 = (int16_T)pad_window_blockOB->pad1[d_sample_pad1_tmp];
        *sample_pad2 = (int16_T)pad_window_blockOB->pad2[d_sample_pad1_tmp];
        *sample_pad3 = (int16_T)pad_window_blockOB->pad3[d_sample_pad1_tmp];
        *sample_pad4 = (int16_T)pad_window_blockOB->pad4[d_sample_pad1_tmp];
        *sample_pad5 = (int16_T)pad_window_blockOB->pad5[d_sample_pad1_tmp];
        *sample_pad6 = (int16_T)pad_window_blockOB->pad6[d_sample_pad1_tmp];
        *sample_pad7 = (int16_T)pad_window_blockOB->pad7[d_sample_pad1_tmp];
        *sample_pad8 = (int16_T)pad_window_blockOB->pad8[d_sample_pad1_tmp];
      } else {
        c_sample_pad1_tmp = (int16_T)b_index - 71;
        *sample_pad1 = (int16_T)pad_window_blockOA->pad1[c_sample_pad1_tmp];
        *sample_pad2 = (int16_T)pad_window_blockOA->pad2[c_sample_pad1_tmp];
        *sample_pad3 = (int16_T)pad_window_blockOA->pad3[c_sample_pad1_tmp];
        *sample_pad4 = (int16_T)pad_window_blockOA->pad4[c_sample_pad1_tmp];
        *sample_pad5 = (int16_T)pad_window_blockOA->pad5[c_sample_pad1_tmp];
        *sample_pad6 = (int16_T)pad_window_blockOA->pad6[c_sample_pad1_tmp];
        *sample_pad7 = (int16_T)pad_window_blockOA->pad7[c_sample_pad1_tmp];
        *sample_pad8 = (int16_T)pad_window_blockOA->pad8[c_sample_pad1_tmp];
      }
    }
  }
}

/*
 * Arguments    : waterAlgoData_t *algo_data
 *                waterCalibration_t *water_calib
 *                const padWindows_t *pad_window
 *                ReasonCodes reason_codes[8]
 * Return Type  : void
 */
void calculateWaterVolume(waterAlgoData_t *algo_data, waterCalibration_t
  *water_calib, const padWindows_t *pad_window, ReasonCodes reason_codes[8])
{
  uint8_T check_cal_counter;
  int16_T i;
  int16_T b_present_start_idx;
  int16_T b_present_stop_idx;
  int16_T idx;
  uint8_T curr_sample_success;
  int16_T current_sample_pad1;
  int16_T current_sample_pad2;
  int16_T current_sample_pad3;
  int16_T current_sample_pad4;
  int16_T current_sample_pad5;
  int16_T current_sample_pad6;
  int16_T current_sample_pad7;
  int16_T current_sample_pad8;
  uint8_T past_sample_present_success;
  int16_T expl_temp;
  int16_T b_expl_temp;
  int16_T c_expl_temp;
  int16_T d_expl_temp;
  int16_T past_present_sample_pad5;
  int16_T past_present_sample_pad6;
  int16_T past_present_sample_pad7;
  int16_T past_present_sample_pad8;
  int16_T delta_sample_pad1;
  int16_T delta_sample_pad2;
  int16_T delta_sample_pad3;
  int16_T delta_sample_pad4;
  int16_T delta_sample_pad5;
  int16_T delta_sample_pad6;
  int16_T delta_sample_pad7;
  int16_T delta_sample_pad8;
  int16_T present_pad6_diff;
  int16_T present_pad7_diff;
  int16_T present_pad8_diff;
  int16_T present_diff_sum;
  uint8_T OA_sample_success;
  int16_T OA_sample_pad1;
  int16_T OA_sample_pad2;
  int16_T OA_sample_pad3;
  int16_T OA_sample_pad4;
  int16_T OA_sample_pad5;
  int16_T OA_sample_pad6;
  int16_T OA_sample_pad7;
  int16_T OA_sample_pad8;
  uint8_T calibration_reset;
  uint8_T water_calibrated;
  uint8_T neg_delta;
  uint8_T small_delta_reset;
  int16_T water_height;
  uint8_T bContinue;
  Pad pad;
  int16_T calib_water_height;
  int16_T diff_water_height;
  uint8_T b_water_not_present;
  int16_T session_volume;
  int32_T a;

  /*  Verify that the correct data is present */
  /*  Water Volume Algorithm */
  /*  Calibration flag */
  check_cal_counter = 0U;

  /*  Initialize reason code */
  for (i = 0; i < 8; i++) {
    reason_codes[i] = reason_code_none;
  }

  b_present_start_idx = 0;
  b_present_stop_idx = -51;
  for (idx = 0; idx < 70; idx++) {
    /*  Read the current sample */
    read_sample((uint16_T)(idx + 51), pad_window->blockA.pad1,
                pad_window->blockA.pad2, pad_window->blockA.pad3,
                pad_window->blockA.pad4, pad_window->blockA.pad5,
                pad_window->blockA.pad6, pad_window->blockA.pad7,
                pad_window->blockA.pad8, &pad_window->blockB,
                &pad_window->blockOA, &pad_window->blockOB,
                pad_window->read_window, &curr_sample_success,
                &current_sample_pad1, &current_sample_pad2, &current_sample_pad3,
                &current_sample_pad4, &current_sample_pad5, &current_sample_pad6,
                &current_sample_pad7, &current_sample_pad8);

    /*  Get past sample using the water present offset */
    read_sample(idx + 11U, pad_window->blockA.pad1, pad_window->blockA.pad2,
                pad_window->blockA.pad3, pad_window->blockA.pad4,
                pad_window->blockA.pad5, pad_window->blockA.pad6,
                pad_window->blockA.pad7, pad_window->blockA.pad8,
                &pad_window->blockB, &pad_window->blockOA, &pad_window->blockOB,
                pad_window->read_window, &past_sample_present_success,
                &expl_temp, &b_expl_temp, &c_expl_temp, &d_expl_temp,
                &past_present_sample_pad5, &past_present_sample_pad6,
                &past_present_sample_pad7, &past_present_sample_pad8);

    /*      % Get past sample using the volume offset */
    /*      if idx > Constants.WTR_VOLUME_DIFF_OFFSET */
    /*          past_idx = idx - Constants.WTR_VOLUME_DIFF_OFFSET; */
    /*      else */
    /*          past_idx = uint16(1); */
    /*      end */
    /*      [past_sample_vol_success, past_vol_sample] = read_sample(past_idx, pad_window); */
    if ((curr_sample_success != 0) && (past_sample_present_success != 0)) {
      if (algo_data->accum_processed_sample_cnt < MAX_int32_T) {
        algo_data->accum_processed_sample_cnt++;
      }

      add_to_buffer(current_sample_pad1, current_sample_pad2,
                    current_sample_pad3, current_sample_pad4,
                    current_sample_pad5, current_sample_pad6,
                    current_sample_pad7, current_sample_pad8,
                    &algo_data->delta_buffer, &delta_sample_pad1,
                    &delta_sample_pad2, &delta_sample_pad3, &delta_sample_pad4,
                    &delta_sample_pad5, &delta_sample_pad6, &delta_sample_pad7,
                    &delta_sample_pad8);
      present_pad6_diff = current_sample_pad6 - past_present_sample_pad6;
      present_pad7_diff = current_sample_pad7 - past_present_sample_pad7;
      present_pad8_diff = current_sample_pad8 - past_present_sample_pad8;
      present_diff_sum = (present_pad6_diff + present_pad7_diff) +
        present_pad8_diff;
      if (algo_data->algo_state == b_water_present) {
        /*  Increment the open air counter until we find a new OA value */
        if (algo_data->OA_counter < MAX_uint16_T) {
          algo_data->OA_counter++;
        } else {
          algo_data->OA_counter = MAX_uint16_T;
        }

        /*  Detect the front of the water ON point */
        /*  These tend to be high frequency (high difference from point to point) */
        if ((present_diff_sum <= -13) || (present_pad8_diff <= -7) ||
            (present_pad7_diff <= -7) || (present_pad6_diff <= -7) ||
            (current_sample_pad5 - past_present_sample_pad5 <= -7)) {
          algo_data->present = 1U;
          algo_data->algo_state = water_volume;

          /*  We will only have 1 starting index per window even if */
          /*  there are multiples in a window */
          if (b_present_start_idx == 0) {
            b_present_start_idx = idx + 51;
            b_present_stop_idx = 69;
          }

          /*  Debug */
          /*  Look back to get Open Air values */
          if (algo_data->OA_counter >= 300U) {
            read_sample(idx + 11U, pad_window->blockA.pad1,
                        pad_window->blockA.pad2, pad_window->blockA.pad3,
                        pad_window->blockA.pad4, pad_window->blockA.pad5,
                        pad_window->blockA.pad6, pad_window->blockA.pad7,
                        pad_window->blockA.pad8, &pad_window->blockB,
                        &pad_window->blockOA, &pad_window->blockOB,
                        pad_window->read_window, &OA_sample_success,
                        &OA_sample_pad1, &OA_sample_pad2, &OA_sample_pad3,
                        &OA_sample_pad4, &OA_sample_pad5, &OA_sample_pad6,
                        &OA_sample_pad7, &OA_sample_pad8);
            if (OA_sample_success != 0) {
              algo_data->pad1_OA = OA_sample_pad1;
              algo_data->pad2_OA = OA_sample_pad2;
              algo_data->pad3_OA = OA_sample_pad3;
              algo_data->pad4_OA = OA_sample_pad4;
              algo_data->pad5_OA = OA_sample_pad5;
              algo_data->pad6_OA = OA_sample_pad6;
              algo_data->pad7_OA = OA_sample_pad7;
              algo_data->pad8_OA = OA_sample_pad8;
            } else {
              algo_data->pad1_OA = 800;
              algo_data->pad2_OA = 800;
              algo_data->pad3_OA = 800;
              algo_data->pad4_OA = 800;
              algo_data->pad5_OA = 800;
              algo_data->pad6_OA = 800;
              algo_data->pad7_OA = 800;
              algo_data->pad8_OA = 800;
              water_calib->pad_1_calib[0] = 0;
              water_calib->pad_2_calib[0] = 0;
              water_calib->pad_3_calib[0] = 0;
              water_calib->pad_4_calib[0] = 0;
              water_calib->pad_5_calib[0] = 0;
              water_calib->pad_6_calib[0] = 0;
              water_calib->pad_7_calib[0] = 0;
              water_calib->pad_8_calib[0] = 0;
              water_calib->pad_1_calib[1] = 0;
              water_calib->pad_2_calib[1] = 0;
              water_calib->pad_3_calib[1] = 0;
              water_calib->pad_4_calib[1] = 0;
              water_calib->pad_5_calib[1] = 0;
              water_calib->pad_6_calib[1] = 0;
              water_calib->pad_7_calib[1] = 0;
              water_calib->pad_8_calib[1] = 0;
              water_calib->pad_1_calib_done = 0U;
              water_calib->pad_2_calib_done = 0U;
              water_calib->pad_3_calib_done = 0U;
              water_calib->pad_4_calib_done = 0U;
              water_calib->pad_5_calib_done = 0U;
              water_calib->pad_6_calib_done = 0U;
              water_calib->pad_7_calib_done = 0U;
              water_calib->pad_8_calib_done = 0U;
              algo_data->water_cal_error_count = 0U;
              add_reason_code(water_calib_reset, reason_codes);
            }
          }
        }
      } else {
        /*  If we are in this state and the present_start_idx is */
        /*  not set, it means water is present  */
        if (b_present_start_idx == 0) {
          b_present_start_idx = 1;
          b_present_stop_idx = 69;
        }

        /*  Pad present states using differential approach */
        detectWaterChange(delta_sample_pad1, &algo_data->pad1_present, 20U);
        detectWaterChange(delta_sample_pad2, &algo_data->pad2_present, 30U);
        detectWaterChange(delta_sample_pad3, &algo_data->pad3_present, 40U);
        detectWaterChange(delta_sample_pad4, &algo_data->pad4_present, 50U);
        detectWaterChange(delta_sample_pad5, &algo_data->pad5_present, 60U);
        detectWaterChange(delta_sample_pad6, &algo_data->pad6_present, 70U);
        detectWaterChange(delta_sample_pad7, &algo_data->pad7_present, 80U);
        detectWaterChange(delta_sample_pad8, &algo_data->pad8_present, 90U);

        /*  Update the pad states based on previous resulsts */
        promotePadStates(algo_data);

        /*  Only check the calibration every so often */
        if (check_cal_counter == 20) {
          calibration_reset = checkWaterCalibration(algo_data, water_calib,
            current_sample_pad2, current_sample_pad3, current_sample_pad4,
            current_sample_pad5, current_sample_pad6, current_sample_pad7,
            current_sample_pad8);
          if (calibration_reset != 0) {
            add_reason_code(water_calib_reset, reason_codes);
          }

          check_cal_counter = 0U;
        } else {
          if (check_cal_counter < 255) {
            check_cal_counter++;
          }
        }

        /*  Update pad calibration as needed */
        waterCalibration(algo_data, water_calib, current_sample_pad1,
                         current_sample_pad2, current_sample_pad3,
                         current_sample_pad4, current_sample_pad5,
                         current_sample_pad6, current_sample_pad7,
                         current_sample_pad8, &water_calibrated, &neg_delta,
                         &small_delta_reset);
        if (water_calibrated != 0) {
          add_reason_code(water_calib_calibrated, reason_codes);
        }

        if (neg_delta != 0) {
          add_reason_code(water_calib_neg_delta, reason_codes);
        }

        if (small_delta_reset != 0) {
          add_reason_code(water_calib_reset, reason_codes);
        }

        /*  Determine water height */
        water_height = 0;
        bContinue = 1U;
        pad = b_pad1;
        while (bContinue != 0) {
          switch (pad) {
           case b_pad1:
            calib_water_height = 0;
            if ((water_calib->pad_1_calib_done == 1) && (algo_data->pad1_OA -
                 current_sample_pad1 > water_calib->pad_1_calib[0] - 5)) {
              calib_water_height = 262;
            }

            diff_water_height = 0;
            if ((algo_data->pad1_present.present_type != water_not_present) &&
                (algo_data->pad2_present.present_type != water_not_present)) {
              diff_water_height = 262;
            }

            /*  Prefer calibration over differential */
            if (calib_water_height != 0) {
              water_height = calib_water_height;
              bContinue = 0U;
            } else {
              if (diff_water_height != 0) {
                water_height = diff_water_height;
                bContinue = 0U;
              }
            }

            if (diff_water_height != 0) {
              /*  Upgrade the water state to master */
              /*  Promote the state to the master state */
              if (algo_data->pad2_present.present_type <
                  algo_data->pad1_present.present_type) {
                algo_data->pad2_present = algo_data->pad1_present;
              }

              /*  Promote the state to the master state */
              if (algo_data->pad3_present.present_type <
                  algo_data->pad1_present.present_type) {
                algo_data->pad3_present = algo_data->pad1_present;
              }

              /*  Promote the state to the master state */
              if (algo_data->pad4_present.present_type <
                  algo_data->pad1_present.present_type) {
                algo_data->pad4_present = algo_data->pad1_present;
              }

              /*  Promote the state to the master state */
              if (algo_data->pad5_present.present_type <
                  algo_data->pad1_present.present_type) {
                algo_data->pad5_present = algo_data->pad1_present;
              }

              /*  Promote the state to the master state */
              if (algo_data->pad6_present.present_type <
                  algo_data->pad1_present.present_type) {
                algo_data->pad6_present = algo_data->pad1_present;
              }

              /*  Promote the state to the master state */
              if (algo_data->pad7_present.present_type <
                  algo_data->pad1_present.present_type) {
                algo_data->pad7_present = algo_data->pad1_present;
              }

              /*  Promote the state to the master state */
              if (algo_data->pad8_present.present_type <
                  algo_data->pad1_present.present_type) {
                algo_data->pad8_present = algo_data->pad1_present;
              }
            }

            pad = b_pad2;
            break;

           case b_pad2:
            calib_water_height = 0;
            if ((water_calib->pad_2_calib_done == 1) && (algo_data->pad2_OA -
                 current_sample_pad2 > water_calib->pad_2_calib[0] - 5)) {
              calib_water_height = 229;
            }

            diff_water_height = 0;
            if ((algo_data->pad2_present.present_type != water_not_present) &&
                (algo_data->pad3_present.present_type != water_not_present)) {
              diff_water_height = 229;
            }

            /*  Prefer calibration over differential */
            if (calib_water_height != 0) {
              water_height = calib_water_height;
              bContinue = 0U;
            } else {
              if (diff_water_height != 0) {
                water_height = diff_water_height;
                bContinue = 0U;
              }
            }

            if (diff_water_height != 0) {
              /*  Upgrade the water state to master */
              /*  Promote the state to the master state */
              if (algo_data->pad3_present.present_type <
                  algo_data->pad2_present.present_type) {
                algo_data->pad3_present = algo_data->pad2_present;
              }

              /*  Promote the state to the master state */
              if (algo_data->pad4_present.present_type <
                  algo_data->pad2_present.present_type) {
                algo_data->pad4_present = algo_data->pad2_present;
              }

              /*  Promote the state to the master state */
              if (algo_data->pad5_present.present_type <
                  algo_data->pad2_present.present_type) {
                algo_data->pad5_present = algo_data->pad2_present;
              }

              /*  Promote the state to the master state */
              if (algo_data->pad6_present.present_type <
                  algo_data->pad2_present.present_type) {
                algo_data->pad6_present = algo_data->pad2_present;
              }

              /*  Promote the state to the master state */
              if (algo_data->pad7_present.present_type <
                  algo_data->pad2_present.present_type) {
                algo_data->pad7_present = algo_data->pad2_present;
              }

              /*  Promote the state to the master state */
              if (algo_data->pad8_present.present_type <
                  algo_data->pad2_present.present_type) {
                algo_data->pad8_present = algo_data->pad2_present;
              }
            }

            pad = b_pad3;
            break;

           case b_pad3:
            calib_water_height = 0;
            if ((water_calib->pad_3_calib_done == 1) && (algo_data->pad3_OA -
                 current_sample_pad3 > water_calib->pad_3_calib[0] - 5)) {
              calib_water_height = 197;
            }

            diff_water_height = 0;
            if ((algo_data->pad3_present.present_type != water_not_present) &&
                (algo_data->pad4_present.present_type != water_not_present)) {
              diff_water_height = 197;
            }

            /*  Prefer calibration over differential */
            if (calib_water_height != 0) {
              water_height = calib_water_height;
              bContinue = 0U;
            } else {
              if (diff_water_height != 0) {
                water_height = diff_water_height;
                bContinue = 0U;
              }
            }

            if (diff_water_height != 0) {
              /*  Upgrade the water state to master */
              /*  Promote the state to the master state */
              if (algo_data->pad4_present.present_type <
                  algo_data->pad3_present.present_type) {
                algo_data->pad4_present = algo_data->pad3_present;
              }

              /*  Promote the state to the master state */
              if (algo_data->pad5_present.present_type <
                  algo_data->pad3_present.present_type) {
                algo_data->pad5_present = algo_data->pad3_present;
              }

              /*  Promote the state to the master state */
              if (algo_data->pad6_present.present_type <
                  algo_data->pad3_present.present_type) {
                algo_data->pad6_present = algo_data->pad3_present;
              }

              /*  Promote the state to the master state */
              if (algo_data->pad7_present.present_type <
                  algo_data->pad3_present.present_type) {
                algo_data->pad7_present = algo_data->pad3_present;
              }

              /*  Promote the state to the master state */
              if (algo_data->pad8_present.present_type <
                  algo_data->pad3_present.present_type) {
                algo_data->pad8_present = algo_data->pad3_present;
              }
            }

            pad = b_pad4;
            break;

           case b_pad4:
            calib_water_height = 0;
            if ((water_calib->pad_4_calib_done == 1) && (algo_data->pad4_OA -
                 current_sample_pad4 > water_calib->pad_4_calib[0] - 5)) {
              calib_water_height = 164;
            }

            diff_water_height = 0;
            if ((algo_data->pad4_present.present_type != water_not_present) &&
                (algo_data->pad5_present.present_type != water_not_present)) {
              diff_water_height = 164;
            }

            /*  Prefer calibration over differential */
            if (calib_water_height != 0) {
              water_height = calib_water_height;
              bContinue = 0U;
            } else {
              if (diff_water_height != 0) {
                water_height = diff_water_height;
                bContinue = 0U;
              }
            }

            if (diff_water_height != 0) {
              /*  Upgrade the water state to master */
              /*  Promote the state to the master state */
              if (algo_data->pad5_present.present_type <
                  algo_data->pad4_present.present_type) {
                algo_data->pad5_present = algo_data->pad4_present;
              }

              /*  Promote the state to the master state */
              if (algo_data->pad6_present.present_type <
                  algo_data->pad4_present.present_type) {
                algo_data->pad6_present = algo_data->pad4_present;
              }

              /*  Promote the state to the master state */
              if (algo_data->pad7_present.present_type <
                  algo_data->pad4_present.present_type) {
                algo_data->pad7_present = algo_data->pad4_present;
              }

              /*  Promote the state to the master state */
              if (algo_data->pad8_present.present_type <
                  algo_data->pad4_present.present_type) {
                algo_data->pad8_present = algo_data->pad4_present;
              }
            }

            pad = b_pad5;
            break;

           case b_pad5:
            calib_water_height = 0;
            if ((water_calib->pad_5_calib_done == 1) && (algo_data->pad5_OA -
                 current_sample_pad5 > water_calib->pad_5_calib[0] - 5)) {
              calib_water_height = 131;
            }

            diff_water_height = 0;
            if ((algo_data->pad5_present.present_type != water_not_present) &&
                (algo_data->pad6_present.present_type != water_not_present)) {
              diff_water_height = 131;
            }

            /*  Prefer calibration over differential */
            if (calib_water_height != 0) {
              water_height = calib_water_height;
              bContinue = 0U;
            } else {
              if (diff_water_height != 0) {
                water_height = diff_water_height;
                bContinue = 0U;
              }
            }

            if (diff_water_height != 0) {
              /*  Upgrade the water state to master */
              /*  Promote the state to the master state */
              if (algo_data->pad6_present.present_type <
                  algo_data->pad5_present.present_type) {
                algo_data->pad6_present = algo_data->pad5_present;
              }

              /*  Promote the state to the master state */
              if (algo_data->pad7_present.present_type <
                  algo_data->pad5_present.present_type) {
                algo_data->pad7_present = algo_data->pad5_present;
              }

              /*  Promote the state to the master state */
              if (algo_data->pad8_present.present_type <
                  algo_data->pad5_present.present_type) {
                algo_data->pad8_present = algo_data->pad5_present;
              }
            }

            pad = b_pad6;
            break;

           case b_pad6:
            calib_water_height = 0;
            if ((water_calib->pad_6_calib_done == 1) && (algo_data->pad6_OA -
                 current_sample_pad6 > water_calib->pad_6_calib[0] - 5)) {
              calib_water_height = 98;
            }

            diff_water_height = 0;
            if ((algo_data->pad6_present.present_type != water_not_present) &&
                (algo_data->pad7_present.present_type != water_not_present)) {
              diff_water_height = 98;
            }

            /*  Prefer calibration over differential */
            if (calib_water_height != 0) {
              water_height = calib_water_height;
              bContinue = 0U;
            } else {
              if (diff_water_height != 0) {
                water_height = diff_water_height;
                bContinue = 0U;
              }
            }

            if (diff_water_height != 0) {
              /*  Upgrade the water state to master */
              /*  Promote the state to the master state */
              if (algo_data->pad7_present.present_type <
                  algo_data->pad6_present.present_type) {
                algo_data->pad7_present = algo_data->pad6_present;
              }

              /*  Promote the state to the master state */
              if (algo_data->pad8_present.present_type <
                  algo_data->pad6_present.present_type) {
                algo_data->pad8_present = algo_data->pad6_present;
              }
            }

            pad = b_pad7;
            break;

           case b_pad7:
            calib_water_height = 0;
            if ((water_calib->pad_7_calib_done == 1) && (algo_data->pad7_OA -
                 current_sample_pad7 > water_calib->pad_7_calib[0] - 5)) {
              calib_water_height = 66;
            }

            diff_water_height = 0;
            if ((algo_data->pad7_present.present_type != water_not_present) &&
                (algo_data->pad8_present.present_type != water_not_present)) {
              diff_water_height = 66;
            }

            /*  Prefer calibration over differential */
            if (calib_water_height != 0) {
              water_height = calib_water_height;
              bContinue = 0U;
            } else {
              if (diff_water_height != 0) {
                water_height = diff_water_height;
                bContinue = 0U;
              }
            }

            if ((diff_water_height != 0) &&
                (algo_data->pad8_present.present_type <
                 algo_data->pad7_present.present_type)) {
              /*  Upgrade the water state to master */
              /*  Promote the state to the master state */
              algo_data->pad8_present = algo_data->pad7_present;
            }

            pad = b_pad8;
            break;

           default:
            calib_water_height = 0;
            if ((water_calib->pad_8_calib_done == 1) && (algo_data->pad8_OA -
                 current_sample_pad8 > water_calib->pad_8_calib[0] - 5)) {
              calib_water_height = 33;
            }

            diff_water_height = 0;
            if (algo_data->pad8_present.present_type != water_not_present) {
              diff_water_height = 33;
            }

            /*  Prefer calibration over differential */
            if (calib_water_height != 0) {
              water_height = calib_water_height;
            } else {
              if (diff_water_height != 0) {
                water_height = diff_water_height;
              }
            }

            bContinue = 0U;
            break;
          }
        }

        /*  Debug */
        /*  Look for water not present */
        /*  Check for a period of stable differential on the bottom two pads - this is when the pad 7, 8 differential */
        /*  values are small for a while - keep the water height at pad 8 for the duration this time */
        if ((water_height == 0) && (delta_sample_pad7 < 3) && (delta_sample_pad8
             < 3)) {
          /*  Increment the not present counter */
          if (algo_data->not_present_counter < MAX_uint16_T) {
            algo_data->not_present_counter++;
          }

          /*                      % Keep pad8 in the draining state until we hit the */
          /*                      % counter */
          /*                      water_height = Constants.WTR_VOLUME_PAD8_HEIGHT; */
          /*                      algo_data.pad8_present.present_type = PresentType.water_draining; */
          /*                      algo_data.pad8_present.draining_count = Constants.WTR_VOLUME_PAD8_DRAINING_COUNT_THRESH; % Go sample by sample at this point - no timeout */
        } else {
          /*  Reset the counter */
          algo_data->not_present_counter = 0U;
        }

        /*  Compute features for scaler selection */
        if (water_height > 0) {
          if (algo_data->session_sample_counter < MAX_int32_T) {
            algo_data->session_sample_counter++;
          }

          if (algo_data->accum_water_sample_cnt < MAX_int32_T) {
            algo_data->accum_water_sample_cnt++;
          }

          if ((algo_data->prev_water_height == water_height) &&
              (algo_data->no_change_counter < MAX_int32_T)) {
            algo_data->no_change_counter++;
          }
        }

        /*  Detect the "back" of the water OFF point - this is the closest point when the water */
        /*  is only dribbling */
        b_water_not_present = 0U;
        if (present_diff_sum >= 15) {
          algo_data->water_stopped = 1U;
        }

        /*  Reset water stopped flag if we get down to average again */
        if ((present_diff_sum < 0) && (algo_data->water_stopped != 0)) {
          algo_data->pad_8_stop_flag = 0U;
          algo_data->water_stopped = 0U;
        }

        /*  Look for pad 8 to change significantly */
        if ((algo_data->water_stopped != 0) && (present_pad8_diff >= 6)) {
          algo_data->pad_8_stop_flag = 1U;
        }

        /*  Set water stopped if all conditions are met */
        if ((present_diff_sum < 4) && (algo_data->water_stopped != 0) &&
            (algo_data->pad_8_stop_flag != 0)) {
          b_water_not_present = 1U;
        }

        /*  Global timer to timeout when water height is constant for a long */
        /*  period of time */
        if ((algo_data->prev_water_height == water_height) && (water_height <=
             229) && (water_height != 0)) {
          if (algo_data->constant_height_counter < MAX_uint16_T) {
            algo_data->constant_height_counter++;
          }

          /*  Check for standing water or clogged pump */
          if ((algo_data->constant_height_counter >= 600U) && (water_height <=
               66)) {
            add_reason_code(water_flow_standing_water, reason_codes);
          }
        } else {
          algo_data->constant_height_counter = 0U;
        }

        /*  Add height to integral value */
        if (algo_data->water_int_value <= MAX_int32_T - water_height) {
          algo_data->water_int_value += water_height;
        } else {
          algo_data->water_int_value = MAX_int32_T;
        }

        algo_data->prev_water_height = water_height;

        /*  Debug */
        /*  Check for end of session */
        if ((algo_data->not_present_counter > 60U) ||
            ((algo_data->constant_height_counter >= 600U) && (water_height <=
              229)) || ((algo_data->constant_height_counter >= 2400U) &&
                        (water_height > 229)) || (b_water_not_present != 0)) {
          /*  Mark the ending index */
          b_present_stop_idx = idx;

          /*  End the session */
          session_volume = 0;

          /*  Calculate the % no change */
          if (algo_data->session_sample_counter != 0L) {
            if (algo_data->no_change_counter < 21474836L) {
              a = (int32_T)(algo_data->no_change_counter * 100LL) /
                algo_data->session_sample_counter;
            } else {
              a = 100L;
            }

            /*  Calculate the scaler (Y = b - mx , x = no_change_percentage) */
            /*  Calculate the water volume */
            session_volume = (int16_T)((algo_data->water_int_value *
              ((11811160064LL - 3506LL * (a << 15L)) >> 15L) + 536870912LL) >>
              30L);
          }

          /*  Debug */
          /*  Add session volume to total volume */
          if (algo_data->water_volume_sum <= MAX_int32_T - session_volume) {
            algo_data->water_volume_sum += session_volume;
          } else {
            algo_data->water_volume_sum = MAX_int32_T;
          }

          /*  Session ended so move to water present state */
          algo_data->algo_state = b_water_present;

          /*  Reset session variables */
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

          /*  Reset session volume variables */
          algo_data->water_int_value = 0L;
          algo_data->session_sample_counter = 0L;
          algo_data->no_change_counter = 0L;

          /*  NOTE: do not reset the accumulated water volume - */
          /*  this is reset when hourly water volume is computed */
        }
      }

      /*  Debug */
    } else {
      add_reason_code(water_bad_sample, reason_codes);
    }

    /*  Debug */
  }

  algo_data->present_start_idx = (uint16_T)b_present_start_idx;
  algo_data->present_stop_idx = (uint16_T)(b_present_stop_idx + 51);
}

/*
 * File trailer for calculateWaterVolume.c
 *
 * [EOF]
 */
