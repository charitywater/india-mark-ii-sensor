/*
 * File: trackRange.c
 *
 * MATLAB Coder version            : 5.0
 * C/C++ source code generated on  : 21-Dec-2020 10:28:01
 */

/* Include Files */
#include "trackRange.h"
#include "magnetometerCalibration.h"

/* Function Definitions */

/*
 * Arguments    : int16_T val
 *                int16_T *b_max_val
 *                int16_T *b_min_val
 *                uint8_T *new_max_val
 *                uint8_T *in_max_range
 *                uint8_T *large_max_change
 *                uint8_T *new_min_val
 *                uint8_T *in_min_range
 *                uint8_T *large_min_change
 * Return Type  : void
 */
void trackRange(int16_T val, int16_T *b_max_val, int16_T *b_min_val, uint8_T
                *new_max_val, uint8_T *in_max_range, uint8_T *large_max_change,
                uint8_T *new_min_val, uint8_T *in_min_range, uint8_T
                *large_min_change)
{
  /* #codgen */
  /*  Max */
  *new_max_val = 0U;
  *in_max_range = 0U;
  *large_max_change = 0U;
  if (val > *b_max_val + 20) {
    /*  Check for a large change from the current max/min */
    /*  value */
    if ((*b_max_val != MIN_int16_T) && (val > *b_max_val + 200)) {
      *large_max_change = 1U;
    }

    /*  Set a new max val */
    *b_max_val = val;
    *new_max_val = 1U;
  } else {
    if (val > *b_max_val - 80) {
      *in_max_range = 1U;
    }
  }

  /*  Min */
  *new_min_val = 0U;
  *in_min_range = 0U;
  *large_min_change = 0U;
  if (val < *b_min_val - 20) {
    /*  Check for a large change from the current max/min */
    /*  value */
    if ((*b_min_val != MAX_int16_T) && (val < *b_min_val - 200)) {
      *large_min_change = 1U;
    }

    /*  Set a new max/min */
    *b_min_val = val;
    *new_min_val = 1U;
  } else {
    if (val < *b_min_val + 80) {
      *in_min_range = 1U;
    }
  }
}

/*
 * File trailer for trackRange.c
 *
 * [EOF]
 */
