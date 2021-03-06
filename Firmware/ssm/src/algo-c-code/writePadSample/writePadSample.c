/*
 * File: writePadSample.c
 *
 * MATLAB Coder version            : 5.0
 * C/C++ source code generated on  : 18-May-2021 11:56:52
 */

/* Include Files */
#include "writePadSample.h"

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
 *                const padSample_t *pad_sample
 * Return Type  : void
 */
void writePadSample(padWindows_t *pad_windows, const padSample_t *pad_sample)
{
  /*  Check inputs */
  /*  Writing to block OA */
  if (pad_windows->write_block == b_blockOA) {
    pad_windows->write_idx++;

    /* # codegen */
    pad_windows->blockOA.pad1[(int16_T)pad_windows->write_idx - 1] = (uint16_T)
      pad_sample->pad1;
    pad_windows->blockOA.pad2[(int16_T)pad_windows->write_idx - 1] = (uint16_T)
      pad_sample->pad2;
    pad_windows->blockOA.pad3[(int16_T)pad_windows->write_idx - 1] = (uint16_T)
      pad_sample->pad3;
    pad_windows->blockOA.pad4[(int16_T)pad_windows->write_idx - 1] = (uint16_T)
      pad_sample->pad4;
    pad_windows->blockOA.pad5[(int16_T)pad_windows->write_idx - 1] = (uint16_T)
      pad_sample->pad5;
    pad_windows->blockOA.pad6[(int16_T)pad_windows->write_idx - 1] = (uint16_T)
      pad_sample->pad6;
    pad_windows->blockOA.pad7[(int16_T)pad_windows->write_idx - 1] = (uint16_T)
      pad_sample->pad7;
    pad_windows->blockOA.pad8[(int16_T)pad_windows->write_idx - 1] = (uint16_T)
      pad_sample->pad8;
    if (pad_windows->write_idx >= 50U) {
      if (pad_windows->first_pass == 0) {
        pad_windows->process = 1U;
        pad_windows->read_window = windowB;
      }

      pad_windows->write_block = b_blockA;
      pad_windows->write_idx = 0U;
    }

    /*  Writing to block A */
  } else if (pad_windows->write_block == b_blockA) {
    pad_windows->write_idx++;

    /* # codegen */
    pad_windows->blockA.pad1[(int16_T)pad_windows->write_idx - 1] = (uint16_T)
      pad_sample->pad1;
    pad_windows->blockA.pad2[(int16_T)pad_windows->write_idx - 1] = (uint16_T)
      pad_sample->pad2;
    pad_windows->blockA.pad3[(int16_T)pad_windows->write_idx - 1] = (uint16_T)
      pad_sample->pad3;
    pad_windows->blockA.pad4[(int16_T)pad_windows->write_idx - 1] = (uint16_T)
      pad_sample->pad4;
    pad_windows->blockA.pad5[(int16_T)pad_windows->write_idx - 1] = (uint16_T)
      pad_sample->pad5;
    pad_windows->blockA.pad6[(int16_T)pad_windows->write_idx - 1] = (uint16_T)
      pad_sample->pad6;
    pad_windows->blockA.pad7[(int16_T)pad_windows->write_idx - 1] = (uint16_T)
      pad_sample->pad7;
    pad_windows->blockA.pad8[(int16_T)pad_windows->write_idx - 1] = (uint16_T)
      pad_sample->pad8;
    if (pad_windows->write_idx >= 20U) {
      pad_windows->write_block = b_blockOB;
      pad_windows->write_idx = 0U;
    }

    /*  Writing to block OB */
  } else if (pad_windows->write_block == b_blockOB) {
    pad_windows->write_idx++;

    /* # codegen */
    pad_windows->blockOB.pad1[(int16_T)pad_windows->write_idx - 1] = (uint16_T)
      pad_sample->pad1;
    pad_windows->blockOB.pad2[(int16_T)pad_windows->write_idx - 1] = (uint16_T)
      pad_sample->pad2;
    pad_windows->blockOB.pad3[(int16_T)pad_windows->write_idx - 1] = (uint16_T)
      pad_sample->pad3;
    pad_windows->blockOB.pad4[(int16_T)pad_windows->write_idx - 1] = (uint16_T)
      pad_sample->pad4;
    pad_windows->blockOB.pad5[(int16_T)pad_windows->write_idx - 1] = (uint16_T)
      pad_sample->pad5;
    pad_windows->blockOB.pad6[(int16_T)pad_windows->write_idx - 1] = (uint16_T)
      pad_sample->pad6;
    pad_windows->blockOB.pad7[(int16_T)pad_windows->write_idx - 1] = (uint16_T)
      pad_sample->pad7;
    pad_windows->blockOB.pad8[(int16_T)pad_windows->write_idx - 1] = (uint16_T)
      pad_sample->pad8;
    if (pad_windows->write_idx >= 50U) {
      pad_windows->first_pass = 0U;
      pad_windows->process = 1U;
      pad_windows->read_window = windowA;
      pad_windows->write_block = b_blockB;
      pad_windows->write_idx = 0U;
    }

    /*  Writing to block B */
  } else {
    if (pad_windows->write_block == b_blockB) {
      pad_windows->write_idx++;

      /* # codegen */
      pad_windows->blockB.pad1[(int16_T)pad_windows->write_idx - 1] = (uint16_T)
        pad_sample->pad1;
      pad_windows->blockB.pad2[(int16_T)pad_windows->write_idx - 1] = (uint16_T)
        pad_sample->pad2;
      pad_windows->blockB.pad3[(int16_T)pad_windows->write_idx - 1] = (uint16_T)
        pad_sample->pad3;
      pad_windows->blockB.pad4[(int16_T)pad_windows->write_idx - 1] = (uint16_T)
        pad_sample->pad4;
      pad_windows->blockB.pad5[(int16_T)pad_windows->write_idx - 1] = (uint16_T)
        pad_sample->pad5;
      pad_windows->blockB.pad6[(int16_T)pad_windows->write_idx - 1] = (uint16_T)
        pad_sample->pad6;
      pad_windows->blockB.pad7[(int16_T)pad_windows->write_idx - 1] = (uint16_T)
        pad_sample->pad7;
      pad_windows->blockB.pad8[(int16_T)pad_windows->write_idx - 1] = (uint16_T)
        pad_sample->pad8;
      if (pad_windows->write_idx >= 20U) {
        pad_windows->write_block = b_blockOA;
        pad_windows->write_idx = 0U;
      }
    }
  }
}

/*
 * File trailer for writePadSample.c
 *
 * [EOF]
 */
