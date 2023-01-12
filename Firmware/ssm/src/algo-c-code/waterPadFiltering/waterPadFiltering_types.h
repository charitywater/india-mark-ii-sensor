/*
 * File: waterPadFiltering_types.h
 *
 * MATLAB Coder version            : 5.0
 * C/C++ source code generated on  : 27-Oct-2022 08:10:08
 */

#ifndef WATERPADFILTERING_TYPES_H
#define WATERPADFILTERING_TYPES_H

/* Include Files */
#include "rtwtypes.h"

/* Type Definitions */
#ifndef typedef_padFilteringData_t
#define typedef_padFilteringData_t

typedef struct {
  uint8_T buffer_idx;
  int16_T pad_1_buffer[6];
  int16_T pad_2_buffer[6];
  int16_T pad_3_buffer[6];
  int16_T pad_4_buffer[6];
  int16_T pad_5_buffer[6];
  int16_T pad_6_buffer[6];
  int16_T pad_7_buffer[6];
  int16_T pad_8_buffer[6];
} padFilteringData_t;

#endif                                 /*typedef_padFilteringData_t*/

#ifndef typedef_padSample_t
#define typedef_padSample_t

typedef struct {
  int16_T pad1;
  int16_T pad2;
  int16_T pad3;
  int16_T pad4;
  int16_T pad5;
  int16_T pad6;
  int16_T pad7;
  int16_T pad8;
} padSample_t;

#endif                                 /*typedef_padSample_t*/
#endif

/*
 * File trailer for waterPadFiltering_types.h
 *
 * [EOF]
 */
