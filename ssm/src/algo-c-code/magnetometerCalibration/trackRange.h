/*
 * File: trackRange.h
 *
 * MATLAB Coder version            : 5.0
 * C/C++ source code generated on  : 21-Dec-2020 10:28:01
 */

#ifndef TRACKRANGE_H
#define TRACKRANGE_H

/* Include Files */
#include <stddef.h>
#include <stdlib.h>
#include "rtwtypes.h"
#include "magnetometerCalibration_types.h"

/* Function Declarations */
extern void trackRange(int16_T val, int16_T *b_max_val, int16_T *b_min_val,
  uint8_T *new_max_val, uint8_T *in_max_range, uint8_T *large_max_change,
  uint8_T *new_min_val, uint8_T *in_min_range, uint8_T *large_min_change);

#endif

/*
 * File trailer for trackRange.h
 *
 * [EOF]
 */
