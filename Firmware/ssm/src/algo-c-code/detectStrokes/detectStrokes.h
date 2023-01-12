/*
 * File: detectStrokes.h
 *
 * MATLAB Coder version            : 5.0
 * C/C++ source code generated on  : 27-Oct-2022 08:13:00
 */

#ifndef DETECTSTROKES_H
#define DETECTSTROKES_H

/* Include Files */
#include <stddef.h>
#include <stdlib.h>
#include "rtwtypes.h"
#include "detectStrokes_types.h"

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
extern ReasonCodes detectStrokes(const strokeTransitionBuffer_t *transitions,
  strokeBuffer_t *strokes, strokeDetectInfo_t *state_info, accumStrokeCount_t
  *accum_stroke_count, const magCalibration_t *mag_calib, const waterAlgoData_t *
  water_data);

#endif

/*
 * File trailer for detectStrokes.h
 *
 * [EOF]
 */
