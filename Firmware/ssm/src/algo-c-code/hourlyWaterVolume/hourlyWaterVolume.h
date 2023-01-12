/*
 * File: hourlyWaterVolume.h
 *
 * MATLAB Coder version            : 5.0
 * C/C++ source code generated on  : 27-Oct-2022 08:10:20
 */

#ifndef HOURLYWATERVOLUME_H
#define HOURLYWATERVOLUME_H

/* Include Files */
#include <stddef.h>
#include <stdlib.h>
#include "rtwtypes.h"
#include "hourlyWaterVolume_types.h"

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
extern void hourlyWaterVolume(waterAlgoData_t *algo_data, pumpUsage_t
  *pump_usage, uint8_T hour, uint8_T day, ReasonCodes *reason_code,
  hourlyWaterInfo_t *hourly_water_info);

#endif

/*
 * File trailer for hourlyWaterVolume.h
 *
 * [EOF]
 */
