/*
 * File: isPeakValley.c
 *
 * MATLAB Coder version            : 5.0
 * C/C++ source code generated on  : 18-May-2021 11:59:40
 */

/* Include Files */
#include "isPeakValley.h"
#include "magnetometerCalibration.h"

/* Function Definitions */

/*
 * Arguments    : const int16_T D[3]
 *                int16_T range_thresh
 *                PeakType *type
 *                int16_T *val
 * Return Type  : void
 */
void isPeakValley(const int16_T D[3], int16_T range_thresh, PeakType *type,
                  int16_T *val)
{
  uint8_T prev_direction;
  uint8_T direction_change;
  uint8_T is_first;
  int16_T i;
  boolean_T exitg1;
  int16_T b_i;
  int16_T i1;
  uint8_T direction;

  /*  Check inputs */
  *type = no_peak;
  *val = D[2];
  prev_direction = 0U;
  direction_change = 0U;
  is_first = 1U;
  i = 0;
  exitg1 = false;
  while ((!exitg1) && (i < 2)) {
    b_i = D[i + 1];
    if (b_i > D[i]) {
      direction = 1U;
    } else if (b_i < D[i]) {
      direction = 2U;
    } else {
      direction = 3U;

      /*  If we find a flat section, it will be a no_peak */
    }

    if ((is_first == 0) && (direction != prev_direction)) {
      direction_change = 1U;
      exitg1 = true;
    } else {
      is_first = 0U;
      prev_direction = direction;
      i++;
    }
  }

  if (direction_change == 0) {
    i1 = D[2] - D[0];
    if (i1 >= range_thresh) {
      *type = peak;
    } else {
      if (i1 <= -range_thresh) {
        *type = valley;
      }
    }
  }
}

/*
 * File trailer for isPeakValley.c
 *
 * [EOF]
 */
