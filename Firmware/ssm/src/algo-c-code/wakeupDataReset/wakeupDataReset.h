/*
 * File: wakeupDataReset.h
 *
 * MATLAB Coder version            : 5.0
 * C/C++ source code generated on  : 18-May-2021 11:57:23
 */

#ifndef WAKEUPDATARESET_H
#define WAKEUPDATARESET_H

/* Include Files */
#include <stddef.h>
#include <stdlib.h>
#include "rtwtypes.h"
#include "wakeupDataReset_types.h"

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
extern void wakeupDataReset(padWindows_t *pad_windows, magWindows_t *mag_windows,
  waterAlgoData_t *algo_data, padFilteringData_t *Pad_Filter_Data);

#endif

/*
 * File trailer for wakeupDataReset.h
 *
 * [EOF]
 */
