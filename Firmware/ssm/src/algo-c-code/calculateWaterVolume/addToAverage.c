/*
 * File: addToAverage.c
 *
 * MATLAB Coder version            : 5.0
 * C/C++ source code generated on  : 18-May-2021 11:58:35
 */

/* Include Files */
#include "addToAverage.h"
#include "calculateWaterVolume.h"

/* Function Declarations */
static int16_T div_s16(int16_T numerator, int16_T denominator);

/* Function Definitions */

/*
 * Arguments    : int16_T numerator
 *                int16_T denominator
 * Return Type  : int16_T
 */
static int16_T div_s16(int16_T numerator, int16_T denominator)
{
  int16_T quotient;
  uint16_T b_numerator;
  uint16_T b_denominator;
  uint16_T tempAbsQuotient;
  if (denominator == 0) {
    if (numerator >= 0) {
      quotient = MAX_int16_T;
    } else {
      quotient = MIN_int16_T;
    }
  } else {
    if (numerator < 0) {
      b_numerator = ~(uint16_T)numerator + 1U;
    } else {
      b_numerator = (uint16_T)numerator;
    }

    if (denominator < 0) {
      b_denominator = ~(uint16_T)denominator + 1U;
    } else {
      b_denominator = (uint16_T)denominator;
    }

    tempAbsQuotient = b_numerator / b_denominator;
    if ((numerator < 0) != (denominator < 0)) {
      quotient = -(int16_T)tempAbsQuotient;
    } else {
      quotient = (int16_T)tempAbsQuotient;
    }
  }

  return quotient;
}

/*
 * Arguments    : int16_T cal_index
 *                int16_T cal_value
 *                int16_T current
 *                int16_T open_air
 *                uint8_T *bad_cal_val
 *                int16_T *result
 *                int16_T *b_index
 * Return Type  : void
 */
void addToAverage(int16_T cal_index, int16_T cal_value, int16_T current, int16_T
                  open_air, uint8_T *bad_cal_val, int16_T *result, int16_T
                  *b_index)
{
  int16_T numerator;
  int16_T mult_result2;

  /*  temp variable */
  /*  Case to check for bad calibration value */
  numerator = open_air - current;
  if (numerator <= 10) {
    *result = cal_index;
    *b_index = cal_index - 1;
    *bad_cal_val = 1U;
  } else if (cal_index == 1) {
    *result = numerator;
    *b_index = 1;
  } else if ((cal_index > 1) && (cal_index <= 30)) {
    /*  Math to add current point to average */
    /*  Function to add additional value to an idiivde to round up a number */
    mult_result2 = numerator / cal_index;
    if (numerator - cal_index * div_s16(numerator, cal_index) > 0) {
      mult_result2++;
    }

    *result = (cal_index - 1) * cal_value / cal_index + mult_result2;
    *b_index = cal_index;
  } else if (cal_index > 30) {
    /* limit max of 20 calibration points */
    *result = cal_value;
    *b_index = 30;
  } else {
    /*  SHouldn't get here but reset to zero if error */
    *result = 0;
    *b_index = 0;
  }
}

/*
 * File trailer for addToAverage.c
 *
 * [EOF]
 */
