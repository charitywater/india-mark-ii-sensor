/*
 * File: computePumpHealth_types.h
 *
 * MATLAB Coder version            : 5.0
 * C/C++ source code generated on  : 27-Oct-2022 08:13:43
 */

#ifndef COMPUTEPUMPHEALTH_TYPES_H
#define COMPUTEPUMPHEALTH_TYPES_H

/* Include Files */
#include "rtwtypes.h"

/* Type Definitions */
#ifndef typedef_hourlyPumpHealthInfo_t
#define typedef_hourlyPumpHealthInfo_t

typedef struct {
  uint32_T pump_capacity;
  int32_T quality_factor;
} hourlyPumpHealthInfo_t;

#endif                                 /*typedef_hourlyPumpHealthInfo_t*/

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

#ifndef typedef_hourlyWaterInfo_t
#define typedef_hourlyWaterInfo_t

typedef struct {
  int32_T volume;
  int32_T percent_pump_usage;
} hourlyWaterInfo_t;

#endif                                 /*typedef_hourlyWaterInfo_t*/
#endif

/*
 * File trailer for computePumpHealth_types.h
 *
 * [EOF]
 */
