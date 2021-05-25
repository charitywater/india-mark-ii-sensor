/*
 * File: hourlyStrokeCount_types.h
 *
 * MATLAB Coder version            : 5.0
 * C/C++ source code generated on  : 18-May-2021 12:00:46
 */

#ifndef HOURLYSTROKECOUNT_TYPES_H
#define HOURLYSTROKECOUNT_TYPES_H

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

#ifndef typedef_hourlyStrokeInfo_t
#define typedef_hourlyStrokeInfo_t

typedef struct {
  uint16_T wet_stroke_count;
  uint16_T wet_stroke_avg_displacement;
  uint16_T dry_stroke_count;
  uint16_T dry_stroke_avg_displacement;
  uint16_T combined_stroke_count;
  uint16_T c_combined_stroke_avg_displacem;
  uint8_T mag_calibration_changed;
  uint16_T windows_processed;
} hourlyStrokeInfo_t;

#endif                                 /*typedef_hourlyStrokeInfo_t*/
#endif

/*
 * File trailer for hourlyStrokeCount_types.h
 *
 * [EOF]
 */
