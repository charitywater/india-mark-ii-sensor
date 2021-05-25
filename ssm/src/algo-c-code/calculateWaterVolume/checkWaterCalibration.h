/*
 * File: checkWaterCalibration.h
 *
 * MATLAB Coder version            : 5.0
 * C/C++ source code generated on  : 18-May-2021 11:58:35
 */

#ifndef CHECKWATERCALIBRATION_H
#define CHECKWATERCALIBRATION_H

/* Include Files */
#include <stddef.h>
#include <stdlib.h>
#include "rtwtypes.h"
#include "calculateWaterVolume_types.h"

/* Custom Header Code */

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

/* Function Declarations */
extern uint8_T checkWaterCalibration(waterAlgoData_t *prev_water_data,
  waterCalibration_t *water_calib, int16_T current_sample_pad2, int16_T
  current_sample_pad3, int16_T current_sample_pad4, int16_T current_sample_pad5,
  int16_T current_sample_pad6, int16_T current_sample_pad7, int16_T
  current_sample_pad8);

#endif

/*
 * File trailer for checkWaterCalibration.h
 *
 * [EOF]
 */
