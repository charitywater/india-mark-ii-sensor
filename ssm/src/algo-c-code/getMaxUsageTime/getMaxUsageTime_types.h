/*
 * File: getMaxUsageTime_types.h
 *
 * MATLAB Coder version            : 5.0
 * C/C++ source code generated on  : 18-May-2021 11:59:14
 */

#ifndef GETMAXUSAGETIME_TYPES_H
#define GETMAXUSAGETIME_TYPES_H

/* Include Files */
#include "rtwtypes.h"

/* Type Definitions */
#ifndef typedef_pumpUsage_t
#define typedef_pumpUsage_t

typedef struct {
  int32_T hourly_usage[24];
  uint8_T daily_usage[7];
  uint8_T is_first;
  uint8_T first_hour;
  uint8_T is_filling_hourly_usage;
  uint8_T first_day;
  uint8_T is_filling_daily_usage;
  uint8_T prev_day_max_usage_hour;
} pumpUsage_t;

#endif                                 /*typedef_pumpUsage_t*/
#endif

/*
 * File trailer for getMaxUsageTime_types.h
 *
 * [EOF]
 */
