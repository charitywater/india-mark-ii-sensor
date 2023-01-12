/*
 * File: waterPadFiltering.h
 *
 * MATLAB Coder version            : 5.0
 * C/C++ source code generated on  : 27-Oct-2022 08:10:08
 */

#ifndef WATERPADFILTERING_H
#define WATERPADFILTERING_H

/* Include Files */
#include <stddef.h>
#include <stdlib.h>
#include "rtwtypes.h"
#include "waterPadFiltering_types.h"

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
extern void waterPadFiltering(const padSample_t *pad_sample, padFilteringData_t *
  pad_filtering_data, padSample_t *filtered_pad_sample);

#endif

/*
 * File trailer for waterPadFiltering.h
 *
 * [EOF]
 */
