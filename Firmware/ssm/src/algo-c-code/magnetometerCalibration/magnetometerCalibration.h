/*
 * File: magnetometerCalibration.h
 *
 * MATLAB Coder version            : 5.0
 * C/C++ source code generated on  : 18-May-2021 11:59:40
 */

#ifndef MAGNETOMETERCALIBRATION_H
#define MAGNETOMETERCALIBRATION_H

/* Include Files */
#include <stddef.h>
#include <stdlib.h>
#include "rtwtypes.h"
#include "magnetometerCalibration_types.h"

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
extern void magnetometerCalibration(const magWindows_t *mag_windows,
  magCalibration_t *mag_calib, const waterAlgoData_t *water_data, ReasonCodes
  reason_codes[8]);

#endif

/*
 * File trailer for magnetometerCalibration.h
 *
 * [EOF]
 */
