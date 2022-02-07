/*
 * File: checkWaterCalibration.c
 *
 * MATLAB Coder version            : 5.0
 * C/C++ source code generated on  : 18-May-2021 11:58:35
 */

/* Include Files */
#include "checkWaterCalibration.h"
#include "calculateWaterVolume.h"

/* Function Definitions */

/*
 * Arguments    : waterAlgoData_t *prev_water_data
 *                waterCalibration_t *water_calib
 *                int16_T current_sample_pad2
 *                int16_T current_sample_pad3
 *                int16_T current_sample_pad4
 *                int16_T current_sample_pad5
 *                int16_T current_sample_pad6
 *                int16_T current_sample_pad7
 *                int16_T current_sample_pad8
 * Return Type  : uint8_T
 */
uint8_T checkWaterCalibration(waterAlgoData_t *prev_water_data,
  waterCalibration_t *water_calib, int16_T current_sample_pad2, int16_T
  current_sample_pad3, int16_T current_sample_pad4, int16_T current_sample_pad5,
  int16_T current_sample_pad6, int16_T current_sample_pad7, int16_T
  current_sample_pad8)
{
  uint8_T b_water_calib_reset;
  int16_T pad_2_thresh;
  int16_T pad_3_thresh;
  int16_T pad_4_thresh;
  int16_T pad_5_thresh;
  int16_T pad_6_thresh;
  int16_T pad_7_thresh;
  int16_T pad_8_thresh;
  boolean_T guard1 = false;

  /*  Function used to calibrate the water threshold values */
  b_water_calib_reset = 0U;

  /*  Calculate the current threshoilds to compare agains cal */
  pad_2_thresh = prev_water_data->pad2_OA - current_sample_pad2;
  pad_3_thresh = prev_water_data->pad3_OA - current_sample_pad3;
  pad_4_thresh = prev_water_data->pad4_OA - current_sample_pad4;
  pad_5_thresh = prev_water_data->pad5_OA - current_sample_pad5;
  pad_6_thresh = prev_water_data->pad6_OA - current_sample_pad6;
  pad_7_thresh = prev_water_data->pad7_OA - current_sample_pad7;
  pad_8_thresh = prev_water_data->pad8_OA - current_sample_pad8;

  /*  Section to store the water height */
  guard1 = false;
  if ((prev_water_data->pad1_present.present_type == water_present) &&
      (prev_water_data->pad2_present.present_type == water_present) &&
      (water_calib->pad_2_calib_done == 1)) {
    /*  Check against current calibration */
    if ((water_calib->pad_8_calib[0] - 6 <= pad_8_thresh) && (pad_8_thresh <=
         water_calib->pad_8_calib[0] + 6) && (water_calib->pad_7_calib[0] - 6 <=
         pad_7_thresh) && (pad_7_thresh <= water_calib->pad_7_calib[0] + 6) &&
        (water_calib->pad_6_calib[0] - 6 <= pad_6_thresh) && (pad_6_thresh <=
         water_calib->pad_6_calib[0] + 6) && (water_calib->pad_5_calib[0] - 6 <=
         pad_5_thresh) && (pad_5_thresh <= water_calib->pad_5_calib[0] + 6) &&
        (water_calib->pad_4_calib[0] - 6 <= pad_4_thresh) && (pad_4_thresh <=
         water_calib->pad_4_calib[0] + 6) && (water_calib->pad_3_calib[0] - 6 <=
         pad_3_thresh) && (pad_3_thresh <= water_calib->pad_3_calib[0] + 6) &&
        (water_calib->pad_2_calib[0] - 6 <= pad_2_thresh) && (pad_2_thresh <=
         water_calib->pad_2_calib[0] + 6)) {
      prev_water_data->water_cal_error_count = 0U;
    } else {
      prev_water_data->water_cal_error_count++;
      guard1 = true;
    }
  } else if ((prev_water_data->pad2_present.present_type == water_present) &&
             (prev_water_data->pad3_present.present_type == water_present) &&
             (water_calib->pad_3_calib_done == 1)) {
    /*  Check against current calibration */
    if ((water_calib->pad_8_calib[0] - 6 <= pad_8_thresh) && (pad_8_thresh <=
         water_calib->pad_8_calib[0] + 6) && (water_calib->pad_7_calib[0] - 6 <=
         pad_7_thresh) && (pad_7_thresh <= water_calib->pad_7_calib[0] + 6) &&
        (water_calib->pad_6_calib[0] - 6 <= pad_6_thresh) && (pad_6_thresh <=
         water_calib->pad_6_calib[0] + 6) && (water_calib->pad_5_calib[0] - 6 <=
         pad_5_thresh) && (pad_5_thresh <= water_calib->pad_5_calib[0] + 6) &&
        (water_calib->pad_4_calib[0] - 6 <= pad_4_thresh) && (pad_4_thresh <=
         water_calib->pad_4_calib[0] + 6) && (water_calib->pad_3_calib[0] - 6 <=
         pad_3_thresh) && (pad_3_thresh <= water_calib->pad_3_calib[0] + 6)) {
      prev_water_data->water_cal_error_count = 0U;
    } else {
      prev_water_data->water_cal_error_count++;
      guard1 = true;
    }
  } else if ((prev_water_data->pad3_present.present_type == water_present) &&
             (prev_water_data->pad4_present.present_type == water_present) &&
             (water_calib->pad_4_calib_done == 1)) {
    /*  Check against current calibration */
    if ((water_calib->pad_8_calib[0] - 6 <= pad_8_thresh) && (pad_8_thresh <=
         water_calib->pad_8_calib[0] + 6) && (water_calib->pad_7_calib[0] - 6 <=
         pad_7_thresh) && (pad_7_thresh <= water_calib->pad_7_calib[0] + 6) &&
        (water_calib->pad_6_calib[0] - 6 <= pad_6_thresh) && (pad_6_thresh <=
         water_calib->pad_6_calib[0] + 6) && (water_calib->pad_5_calib[0] - 6 <=
         pad_5_thresh) && (pad_5_thresh <= water_calib->pad_5_calib[0] + 6) &&
        (water_calib->pad_4_calib[0] - 6 <= pad_4_thresh) && (pad_4_thresh <=
         water_calib->pad_4_calib[0] + 6)) {
      prev_water_data->water_cal_error_count = 0U;
    } else {
      prev_water_data->water_cal_error_count++;
      guard1 = true;
    }
  } else if ((prev_water_data->pad4_present.present_type == water_present) &&
             (prev_water_data->pad5_present.present_type == water_present) &&
             (water_calib->pad_5_calib_done == 1)) {
    /*  Check against current calibration */
    if ((water_calib->pad_8_calib[0] - 6 <= pad_8_thresh) && (pad_8_thresh <=
         water_calib->pad_8_calib[0] + 6) && (water_calib->pad_7_calib[0] - 6 <=
         pad_7_thresh) && (pad_7_thresh <= water_calib->pad_7_calib[0] + 6) &&
        (water_calib->pad_6_calib[0] - 6 <= pad_6_thresh) && (pad_6_thresh <=
         water_calib->pad_6_calib[0] + 6) && (water_calib->pad_5_calib[0] - 6 <=
         pad_5_thresh) && (pad_5_thresh <= water_calib->pad_5_calib[0] + 6)) {
      prev_water_data->water_cal_error_count = 0U;
    } else {
      prev_water_data->water_cal_error_count++;
      guard1 = true;
    }
  } else if ((prev_water_data->pad5_present.present_type == water_present) &&
             (prev_water_data->pad6_present.present_type == water_present) &&
             (water_calib->pad_6_calib_done == 1)) {
    /*  Check against current calibration */
    if ((water_calib->pad_8_calib[0] - 6 <= pad_8_thresh) && (pad_8_thresh <=
         water_calib->pad_8_calib[0] + 6) && (water_calib->pad_7_calib[0] - 6 <=
         pad_7_thresh) && (pad_7_thresh <= water_calib->pad_7_calib[0] + 6) &&
        (water_calib->pad_6_calib[0] - 6 <= pad_6_thresh) && (pad_6_thresh <=
         water_calib->pad_6_calib[0] + 6)) {
      prev_water_data->water_cal_error_count = 0U;
    } else {
      prev_water_data->water_cal_error_count++;
      guard1 = true;
    }
  } else if ((prev_water_data->pad6_present.present_type == water_present) &&
             (prev_water_data->pad7_present.present_type == water_present) &&
             (water_calib->pad_7_calib_done == 1)) {
    /*  Check against current calibration */
    if ((water_calib->pad_8_calib[0] - 6 <= pad_8_thresh) && (pad_8_thresh <=
         water_calib->pad_8_calib[0] + 6) && (water_calib->pad_7_calib[0] - 6 <=
         pad_7_thresh) && (pad_7_thresh <= water_calib->pad_7_calib[0] + 6)) {
      prev_water_data->water_cal_error_count = 0U;
    } else {
      prev_water_data->water_cal_error_count++;
      guard1 = true;
    }
  } else if ((prev_water_data->pad7_present.present_type == water_present) &&
             (prev_water_data->pad8_present.present_type == water_present) &&
             (water_calib->pad_8_calib_done == 1)) {
    /*  Check against current calibration */
    if ((water_calib->pad_8_calib[0] - 6 <= pad_8_thresh) && (pad_8_thresh <=
         water_calib->pad_8_calib[0] + 6)) {
      prev_water_data->water_cal_error_count = 0U;
    } else {
      prev_water_data->water_cal_error_count++;
      guard1 = true;
    }
  } else {
    guard1 = true;
  }

  if (guard1 && (prev_water_data->water_cal_error_count > 2)) {
    /*  If we have seen enough bad values reset the cal */
    water_calib->pad_1_calib[0] = 0;
    water_calib->pad_2_calib[0] = 0;
    water_calib->pad_3_calib[0] = 0;
    water_calib->pad_4_calib[0] = 0;
    water_calib->pad_5_calib[0] = 0;
    water_calib->pad_6_calib[0] = 0;
    water_calib->pad_7_calib[0] = 0;
    water_calib->pad_8_calib[0] = 0;
    water_calib->pad_1_calib[1] = 0;
    water_calib->pad_2_calib[1] = 0;
    water_calib->pad_3_calib[1] = 0;
    water_calib->pad_4_calib[1] = 0;
    water_calib->pad_5_calib[1] = 0;
    water_calib->pad_6_calib[1] = 0;
    water_calib->pad_7_calib[1] = 0;
    water_calib->pad_8_calib[1] = 0;
    water_calib->pad_1_calib_done = 0U;
    water_calib->pad_2_calib_done = 0U;
    water_calib->pad_3_calib_done = 0U;
    water_calib->pad_4_calib_done = 0U;
    water_calib->pad_5_calib_done = 0U;
    water_calib->pad_6_calib_done = 0U;
    water_calib->pad_7_calib_done = 0U;
    water_calib->pad_8_calib_done = 0U;
    b_water_calib_reset = 1U;
    prev_water_data->water_cal_error_count = 0U;
  }

  return b_water_calib_reset;
}

/*
 * File trailer for checkWaterCalibration.c
 *
 * [EOF]
 */
