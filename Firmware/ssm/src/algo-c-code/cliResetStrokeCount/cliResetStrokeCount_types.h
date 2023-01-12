/*
 * File: cliResetStrokeCount_types.h
 *
 * MATLAB Coder version            : 5.0
 * C/C++ source code generated on  : 27-Oct-2022 08:13:35
 */

#ifndef CLIRESETSTROKECOUNT_TYPES_H
#define CLIRESETSTROKECOUNT_TYPES_H

/* Include Files */
#include "rtwtypes.h"

/* Type Definitions */
#ifndef typedef_accumStrokeCount_t
#define typedef_accumStrokeCount_t

typedef struct {
  uint16_T num_windows_processed;
  uint16_T wet_stroke_count_sum;
  uint32_T wet_percent_displacement_sum;
  uint16_T dry_stroke_count_sum;
  uint32_T dry_percent_displacement_sum;
  uint8_T mag_calibration_changed;
} accumStrokeCount_t;

#endif                                 /*typedef_accumStrokeCount_t*/
#endif

/*
 * File trailer for cliResetStrokeCount_types.h
 *
 * [EOF]
 */
