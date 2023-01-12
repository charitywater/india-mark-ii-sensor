/*
 * File: writeMagSample.c
 *
 * MATLAB Coder version            : 5.0
 * C/C++ source code generated on  : 27-Oct-2022 08:08:52
 */

/* Include Files */
#include "writeMagSample.h"

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
 * Arguments    : magWindows_t *mag_windows
 *                const magSample_t *mag_sample
 * Return Type  : void
 */
void writeMagSample(magWindows_t *mag_windows, const magSample_t *mag_sample)
{
  int16_T sample_x;
  int16_T sample_y;
  int16_T sample_z;
  int16_T sample_x_tmp;
  int16_T b_sample_x_tmp;
  int16_T c_sample_x_tmp;
  int16_T d_sample_x_tmp;

  /*  Check inputs */
  /*  Only write good samples to memory otherwise copy the previous sample */
  sample_x = mag_sample->x_lsb;
  sample_y = mag_sample->y_lsb;
  sample_z = mag_sample->z_lsb;
  if (mag_sample->status != 15) {
    if (mag_windows->write_idx > 0U) {
      if (mag_windows->write_block == b_blockOA) {
        sample_x_tmp = (int16_T)mag_windows->write_idx - 1;
        sample_x = mag_windows->blockOA.x_lsb[sample_x_tmp];
        sample_y = mag_windows->blockOA.y_lsb[sample_x_tmp];
        sample_z = mag_windows->blockOA.z_lsb[sample_x_tmp];
      } else if (mag_windows->write_block == b_blockA) {
        b_sample_x_tmp = (int16_T)mag_windows->write_idx - 1;
        sample_x = mag_windows->blockA.x_lsb[b_sample_x_tmp];
        sample_y = mag_windows->blockA.y_lsb[b_sample_x_tmp];
        sample_z = mag_windows->blockA.z_lsb[b_sample_x_tmp];
      } else if (mag_windows->write_block == b_blockOB) {
        c_sample_x_tmp = (int16_T)mag_windows->write_idx - 1;
        sample_x = mag_windows->blockOB.x_lsb[c_sample_x_tmp];
        sample_y = mag_windows->blockOB.y_lsb[c_sample_x_tmp];
        sample_z = mag_windows->blockOB.z_lsb[c_sample_x_tmp];
      } else {
        if (mag_windows->write_block == b_blockB) {
          d_sample_x_tmp = (int16_T)mag_windows->write_idx - 1;
          sample_x = mag_windows->blockB.x_lsb[d_sample_x_tmp];
          sample_y = mag_windows->blockB.y_lsb[d_sample_x_tmp];
          sample_z = mag_windows->blockB.z_lsb[d_sample_x_tmp];
        }
      }
    } else {
      sample_x = 0;
      sample_y = 0;
      sample_z = 0;
    }
  }

  /*  Writing to block OA */
  if (mag_windows->write_block == b_blockOA) {
    mag_windows->write_idx++;

    /* # codegen */
    mag_windows->blockOA.x_lsb[(int16_T)mag_windows->write_idx - 1] = sample_x;
    mag_windows->blockOA.y_lsb[(int16_T)mag_windows->write_idx - 1] = sample_y;
    mag_windows->blockOA.z_lsb[(int16_T)mag_windows->write_idx - 1] = sample_z;
    if (mag_windows->write_idx >= 50U) {
      if (mag_windows->first_pass == 0) {
        mag_windows->process = 1U;
        mag_windows->read_window = windowB;
      }

      mag_windows->write_block = b_blockA;
      mag_windows->write_idx = 0U;
    }

    /*  Writing to block A */
  } else if (mag_windows->write_block == b_blockA) {
    mag_windows->write_idx++;

    /* # codegen */
    mag_windows->blockA.x_lsb[(int16_T)mag_windows->write_idx - 1] = sample_x;
    mag_windows->blockA.y_lsb[(int16_T)mag_windows->write_idx - 1] = sample_y;
    mag_windows->blockA.z_lsb[(int16_T)mag_windows->write_idx - 1] = sample_z;
    if (mag_windows->write_idx >= 20U) {
      mag_windows->write_block = b_blockOB;
      mag_windows->write_idx = 0U;
    }

    /*  Writing to block OB */
  } else if (mag_windows->write_block == b_blockOB) {
    mag_windows->write_idx++;

    /* # codegen */
    mag_windows->blockOB.x_lsb[(int16_T)mag_windows->write_idx - 1] = sample_x;
    mag_windows->blockOB.y_lsb[(int16_T)mag_windows->write_idx - 1] = sample_y;
    mag_windows->blockOB.z_lsb[(int16_T)mag_windows->write_idx - 1] = sample_z;
    if (mag_windows->write_idx >= 50U) {
      mag_windows->first_pass = 0U;
      mag_windows->process = 1U;
      mag_windows->read_window = windowA;
      mag_windows->write_block = b_blockB;
      mag_windows->write_idx = 0U;
    }

    /*  Writing to block B */
  } else {
    if (mag_windows->write_block == b_blockB) {
      mag_windows->write_idx++;

      /* # codegen */
      mag_windows->blockB.x_lsb[(int16_T)mag_windows->write_idx - 1] = sample_x;
      mag_windows->blockB.y_lsb[(int16_T)mag_windows->write_idx - 1] = sample_y;
      mag_windows->blockB.z_lsb[(int16_T)mag_windows->write_idx - 1] = sample_z;
      if (mag_windows->write_idx >= 20U) {
        mag_windows->write_block = b_blockOA;
        mag_windows->write_idx = 0U;
      }
    }
  }
}

/*
 * File trailer for writeMagSample.c
 *
 * [EOF]
 */
