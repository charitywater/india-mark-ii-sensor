/*
 * File: magnetometerCalibration_internal_types.h
 *
 * MATLAB Coder version            : 5.3
 * C/C++ source code generated on  : 28-Apr-2022 18:30:50
 */

#ifndef MAGNETOMETERCALIBRATION_INTERNAL_TYPES_H
#define MAGNETOMETERCALIBRATION_INTERNAL_TYPES_H

/* Include Files */
#include "magnetometerCalibration_types.h"
#include "rtwtypes.h"

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

/* Type Definitions */
#ifndef typedef_PeakType
#define typedef_PeakType
typedef uint8_T PeakType;
#endif /* typedef_PeakType */

#ifndef PeakType_constants
#define PeakType_constants

/* enum PeakType */
#define no_peak ((PeakType)0U)
#define peak ((PeakType)1U)
#define valley ((PeakType)2U)

#endif /* PeakType_constants */

#endif
/*
 * File trailer for magnetometerCalibration_internal_types.h
 *
 * [EOF]
 */
