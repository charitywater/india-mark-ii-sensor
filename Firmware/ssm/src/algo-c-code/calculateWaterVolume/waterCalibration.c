/*
 * File: waterCalibration.c
 *
 * MATLAB Coder version            : 5.0
 * C/C++ source code generated on  : 27-Oct-2022 08:10:46
 */

/* Include Files */
#include "waterCalibration.h"
#include "addToAverage.h"
#include "calculateWaterVolume.h"

/* Function Definitions */

/*
 * Arguments    : waterAlgoData_t *prev_water_data
 *                waterCalibration_t *water_calib
 *                int16_T current_sample_pad1
 *                int16_T current_sample_pad2
 *                int16_T current_sample_pad3
 *                int16_T current_sample_pad4
 *                int16_T current_sample_pad5
 *                int16_T current_sample_pad6
 *                int16_T current_sample_pad7
 *                int16_T current_sample_pad8
 *                uint8_T *water_calibrated
 *                uint8_T *neg_delta
 *                uint8_T *small_delta_reset
 * Return Type  : void
 */
void waterCalibration(waterAlgoData_t *prev_water_data, waterCalibration_t
                      *water_calib, int16_T current_sample_pad1, int16_T
                      current_sample_pad2, int16_T current_sample_pad3, int16_T
                      current_sample_pad4, int16_T current_sample_pad5, int16_T
                      current_sample_pad6, int16_T current_sample_pad7, int16_T
                      current_sample_pad8, uint8_T *water_calibrated, uint8_T
                      *neg_delta, uint8_T *small_delta_reset)
{
  /*  Function used to calibrate the water threshold values */
  *neg_delta = 0U;
  *water_calibrated = 0U;
  *small_delta_reset = 0U;

  /*  Case that we have no OA Average Values */
  if ((prev_water_data->pad1_OA != 0) && (prev_water_data->pad2_OA != 0) &&
      (prev_water_data->pad3_OA != 0) && (prev_water_data->pad4_OA != 0) &&
      (prev_water_data->pad5_OA != 0) && (prev_water_data->pad6_OA != 0) &&
      (prev_water_data->pad7_OA != 0) && (prev_water_data->pad8_OA != 0)) {
    /*  Add a check to make sure we don't see any low value pad values */
    /*  indicating water in the housing */
    /*          if (current_sample.pad1 <= 100 || current_sample.pad2 <= 100 || current_sample.pad3 <= 100 || ... */
    /*              current_sample.pad4 <= 100 || current_sample.pad5 <= 100 || current_sample.pad6 <= 100 || ... */
    /*              current_sample.pad7 <= 100 || current_sample.pad8 <= 100) */
    /*           */
    /*              reason_code = ReasonCodes.water_flow_water_in_housing; */
    /*          end */
    /*  Section to store the water height */
    if ((prev_water_data->pad1_present.present_type == water_present) &&
        (prev_water_data->pad2_present.present_type == water_present) &&
        (water_calib->pad_2_calib_done == 1) && (water_calib->pad_1_calib_done ==
         0)) {
      /*  Update the calibration count */
      if (water_calib->pad_1_calib[1] < 255) {
        water_calib->pad_1_calib[1]++;
      }

      /*  Get fully covered valeus for pads 2-8 */
      addToAverage(water_calib->pad_1_calib[1], water_calib->pad_1_calib[0],
                   current_sample_pad1, prev_water_data->pad1_OA, neg_delta,
                   &water_calib->pad_1_calib[0], &water_calib->pad_1_calib[1]);
    } else if ((prev_water_data->pad1_present.present_type == water_present) &&
               (prev_water_data->pad2_present.present_type == water_present) &&
               (water_calib->pad_2_calib_done == 0)) {
      /*  Update the calibration count */
      if (water_calib->pad_1_calib[1] < 255) {
        water_calib->pad_1_calib[1]++;
      }

      if (water_calib->pad_2_calib[1] < 255) {
        water_calib->pad_2_calib[1]++;
      }

      if (water_calib->pad_3_calib[1] < 255) {
        water_calib->pad_3_calib[1]++;
      }

      if (water_calib->pad_4_calib[1] < 255) {
        water_calib->pad_4_calib[1]++;
      }

      if (water_calib->pad_5_calib[1] < 255) {
        water_calib->pad_5_calib[1]++;
      }

      if (water_calib->pad_6_calib[1] < 255) {
        water_calib->pad_6_calib[1]++;
      }

      if (water_calib->pad_7_calib[1] < 255) {
        water_calib->pad_7_calib[1]++;
      }

      if (water_calib->pad_8_calib[1] < 255) {
        water_calib->pad_8_calib[1]++;
      }

      /*  Get fully covered valeus for pads 3-8 */
      addToAverage(water_calib->pad_1_calib[1], water_calib->pad_1_calib[0],
                   current_sample_pad1, prev_water_data->pad1_OA, neg_delta,
                   &water_calib->pad_1_calib[0], &water_calib->pad_1_calib[1]);
      addToAverage(water_calib->pad_2_calib[1], water_calib->pad_2_calib[0],
                   current_sample_pad2, prev_water_data->pad2_OA, neg_delta,
                   &water_calib->pad_2_calib[0], &water_calib->pad_2_calib[1]);
      addToAverage(water_calib->pad_3_calib[1], water_calib->pad_3_calib[0],
                   current_sample_pad3, prev_water_data->pad3_OA, neg_delta,
                   &water_calib->pad_3_calib[0], &water_calib->pad_3_calib[1]);
      addToAverage(water_calib->pad_4_calib[1], water_calib->pad_4_calib[0],
                   current_sample_pad4, prev_water_data->pad4_OA, neg_delta,
                   &water_calib->pad_4_calib[0], &water_calib->pad_4_calib[1]);
      addToAverage(water_calib->pad_5_calib[1], water_calib->pad_5_calib[0],
                   current_sample_pad5, prev_water_data->pad5_OA, neg_delta,
                   &water_calib->pad_5_calib[0], &water_calib->pad_5_calib[1]);
      addToAverage(water_calib->pad_6_calib[1], water_calib->pad_6_calib[0],
                   current_sample_pad6, prev_water_data->pad6_OA, neg_delta,
                   &water_calib->pad_6_calib[0], &water_calib->pad_6_calib[1]);
      addToAverage(water_calib->pad_7_calib[1], water_calib->pad_7_calib[0],
                   current_sample_pad7, prev_water_data->pad7_OA, neg_delta,
                   &water_calib->pad_7_calib[0], &water_calib->pad_7_calib[1]);
      addToAverage(water_calib->pad_8_calib[1], water_calib->pad_8_calib[0],
                   current_sample_pad8, prev_water_data->pad8_OA, neg_delta,
                   &water_calib->pad_8_calib[0], &water_calib->pad_8_calib[1]);
    } else if ((prev_water_data->pad2_present.present_type == water_present) &&
               (prev_water_data->pad3_present.present_type == water_present) &&
               (water_calib->pad_3_calib_done == 0)) {
      /*  Update the calibration count */
      if (water_calib->pad_3_calib[1] < 255) {
        water_calib->pad_3_calib[1]++;
      }

      if (water_calib->pad_4_calib[1] < 255) {
        water_calib->pad_4_calib[1]++;
      }

      if (water_calib->pad_5_calib[1] < 255) {
        water_calib->pad_5_calib[1]++;
      }

      if (water_calib->pad_6_calib[1] < 255) {
        water_calib->pad_6_calib[1]++;
      }

      if (water_calib->pad_7_calib[1] < 255) {
        water_calib->pad_7_calib[1]++;
      }

      if (water_calib->pad_8_calib[1] < 255) {
        water_calib->pad_8_calib[1]++;
      }

      /*  Get fully covered valeus for pads 3-8 */
      addToAverage(water_calib->pad_3_calib[1], water_calib->pad_3_calib[0],
                   current_sample_pad3, prev_water_data->pad3_OA, neg_delta,
                   &water_calib->pad_3_calib[0], &water_calib->pad_3_calib[1]);
      addToAverage(water_calib->pad_4_calib[1], water_calib->pad_4_calib[0],
                   current_sample_pad4, prev_water_data->pad4_OA, neg_delta,
                   &water_calib->pad_4_calib[0], &water_calib->pad_4_calib[1]);
      addToAverage(water_calib->pad_5_calib[1], water_calib->pad_5_calib[0],
                   current_sample_pad5, prev_water_data->pad5_OA, neg_delta,
                   &water_calib->pad_5_calib[0], &water_calib->pad_5_calib[1]);
      addToAverage(water_calib->pad_6_calib[1], water_calib->pad_6_calib[0],
                   current_sample_pad6, prev_water_data->pad6_OA, neg_delta,
                   &water_calib->pad_6_calib[0], &water_calib->pad_6_calib[1]);
      addToAverage(water_calib->pad_7_calib[1], water_calib->pad_7_calib[0],
                   current_sample_pad7, prev_water_data->pad7_OA, neg_delta,
                   &water_calib->pad_7_calib[0], &water_calib->pad_7_calib[1]);
      addToAverage(water_calib->pad_8_calib[1], water_calib->pad_8_calib[0],
                   current_sample_pad8, prev_water_data->pad8_OA, neg_delta,
                   &water_calib->pad_8_calib[0], &water_calib->pad_8_calib[1]);
    } else if ((prev_water_data->pad3_present.present_type == water_present) &&
               (water_calib->pad_4_calib_done == 0) &&
               (prev_water_data->pad4_present.present_type == water_present)) {
      /*  Update the calibration count */
      if (water_calib->pad_4_calib[1] < 255) {
        water_calib->pad_4_calib[1]++;
      }

      if (water_calib->pad_5_calib[1] < 255) {
        water_calib->pad_5_calib[1]++;
      }

      if (water_calib->pad_6_calib[1] < 255) {
        water_calib->pad_6_calib[1]++;
      }

      if (water_calib->pad_7_calib[1] < 255) {
        water_calib->pad_7_calib[1]++;
      }

      if (water_calib->pad_8_calib[1] < 255) {
        water_calib->pad_8_calib[1]++;
      }

      /*  Get fully covered valeus for pads 4-8 */
      addToAverage(water_calib->pad_4_calib[1], water_calib->pad_4_calib[0],
                   current_sample_pad4, prev_water_data->pad4_OA, neg_delta,
                   &water_calib->pad_4_calib[0], &water_calib->pad_4_calib[1]);
      addToAverage(water_calib->pad_5_calib[1], water_calib->pad_5_calib[0],
                   current_sample_pad5, prev_water_data->pad5_OA, neg_delta,
                   &water_calib->pad_5_calib[0], &water_calib->pad_5_calib[1]);
      addToAverage(water_calib->pad_6_calib[1], water_calib->pad_6_calib[0],
                   current_sample_pad6, prev_water_data->pad6_OA, neg_delta,
                   &water_calib->pad_6_calib[0], &water_calib->pad_6_calib[1]);
      addToAverage(water_calib->pad_7_calib[1], water_calib->pad_7_calib[0],
                   current_sample_pad7, prev_water_data->pad7_OA, neg_delta,
                   &water_calib->pad_7_calib[0], &water_calib->pad_7_calib[1]);
      addToAverage(water_calib->pad_8_calib[1], water_calib->pad_8_calib[0],
                   current_sample_pad8, prev_water_data->pad8_OA, neg_delta,
                   &water_calib->pad_8_calib[0], &water_calib->pad_8_calib[1]);
    } else if ((prev_water_data->pad4_present.present_type == water_present) &&
               (water_calib->pad_5_calib_done == 0) &&
               (prev_water_data->pad5_present.present_type == water_present)) {
      /*  Update the calibration count */
      if (water_calib->pad_5_calib[1] < 255) {
        water_calib->pad_5_calib[1]++;
      }

      if (water_calib->pad_6_calib[1] < 255) {
        water_calib->pad_6_calib[1]++;
      }

      if (water_calib->pad_7_calib[1] < 255) {
        water_calib->pad_7_calib[1]++;
      }

      if (water_calib->pad_8_calib[1] < 255) {
        water_calib->pad_8_calib[1]++;
      }

      /*  Get fully covered valeus for pads 5-8 */
      addToAverage(water_calib->pad_5_calib[1], water_calib->pad_5_calib[0],
                   current_sample_pad5, prev_water_data->pad5_OA, neg_delta,
                   &water_calib->pad_5_calib[0], &water_calib->pad_5_calib[1]);
      addToAverage(water_calib->pad_6_calib[1], water_calib->pad_6_calib[0],
                   current_sample_pad6, prev_water_data->pad6_OA, neg_delta,
                   &water_calib->pad_6_calib[0], &water_calib->pad_6_calib[1]);
      addToAverage(water_calib->pad_7_calib[1], water_calib->pad_7_calib[0],
                   current_sample_pad7, prev_water_data->pad7_OA, neg_delta,
                   &water_calib->pad_7_calib[0], &water_calib->pad_7_calib[1]);
      addToAverage(water_calib->pad_8_calib[1], water_calib->pad_8_calib[0],
                   current_sample_pad8, prev_water_data->pad8_OA, neg_delta,
                   &water_calib->pad_8_calib[0], &water_calib->pad_8_calib[1]);
    } else if ((prev_water_data->pad5_present.present_type == water_present) &&
               (water_calib->pad_6_calib_done == 0) &&
               (prev_water_data->pad6_present.present_type == water_present)) {
      /*  Update the calibration count */
      if (water_calib->pad_6_calib[1] < 255) {
        water_calib->pad_6_calib[1]++;
      }

      if (water_calib->pad_7_calib[1] < 255) {
        water_calib->pad_7_calib[1]++;
      }

      if (water_calib->pad_8_calib[1] < 255) {
        water_calib->pad_8_calib[1]++;
      }

      /*  Get fully covered valeus for pads 6-8 */
      addToAverage(water_calib->pad_6_calib[1], water_calib->pad_6_calib[0],
                   current_sample_pad6, prev_water_data->pad6_OA, neg_delta,
                   &water_calib->pad_6_calib[0], &water_calib->pad_6_calib[1]);
      addToAverage(water_calib->pad_7_calib[1], water_calib->pad_7_calib[0],
                   current_sample_pad7, prev_water_data->pad7_OA, neg_delta,
                   &water_calib->pad_7_calib[0], &water_calib->pad_7_calib[1]);
      addToAverage(water_calib->pad_8_calib[1], water_calib->pad_8_calib[0],
                   current_sample_pad8, prev_water_data->pad8_OA, neg_delta,
                   &water_calib->pad_8_calib[0], &water_calib->pad_8_calib[1]);
    } else if ((prev_water_data->pad6_present.present_type == water_present) &&
               (water_calib->pad_7_calib_done == 0) &&
               (prev_water_data->pad7_present.present_type == water_present)) {
      /*  Update the calibration count */
      if (water_calib->pad_7_calib[1] < 255) {
        water_calib->pad_7_calib[1]++;
      }

      if (water_calib->pad_8_calib[1] < 255) {
        water_calib->pad_8_calib[1]++;
      }

      /*  Get fully covered valeus for pads 7-8 */
      addToAverage(water_calib->pad_7_calib[1], water_calib->pad_7_calib[0],
                   current_sample_pad7, prev_water_data->pad7_OA, neg_delta,
                   &water_calib->pad_7_calib[0], &water_calib->pad_7_calib[1]);
      addToAverage(water_calib->pad_8_calib[1], water_calib->pad_8_calib[0],
                   current_sample_pad8, prev_water_data->pad8_OA, neg_delta,
                   &water_calib->pad_8_calib[0], &water_calib->pad_8_calib[1]);
    } else {
      if ((prev_water_data->pad7_present.present_type == water_present) &&
          (water_calib->pad_8_calib_done == 0) &&
          (prev_water_data->pad8_present.present_type == water_present)) {
        /*  Update the calibration count */
        if (water_calib->pad_8_calib[1] < 255) {
          water_calib->pad_8_calib[1]++;
        }

        /*  Get fully covered values for pads 8 */
        addToAverage(water_calib->pad_8_calib[1], water_calib->pad_8_calib[0],
                     current_sample_pad8, prev_water_data->pad8_OA, neg_delta,
                     &water_calib->pad_8_calib[0], &water_calib->pad_8_calib[1]);
      }
    }

    /*  Check for negative delta error code */
    /*     %% Check to see if calibration complete */
    /*     %% Pad 1 */
    if (water_calib->pad_1_calib[1] >= 50) {
      water_calib->pad_1_calib_done = 1U;
      *water_calibrated = 1U;

      /*  Check for error case */
      if (water_calib->pad_1_calib[0] < 7) {
        *small_delta_reset = 1U;
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
      }
    }

    /*     %% Pad 2 */
    if (water_calib->pad_2_calib[1] >= 50) {
      water_calib->pad_2_calib_done = 1U;

      /*  Check for error case */
      if (water_calib->pad_2_calib[0] < 7) {
        *small_delta_reset = 1U;
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
      }
    }

    /*     %% Pad 3 */
    if (water_calib->pad_3_calib[1] >= 50) {
      water_calib->pad_3_calib_done = 1U;

      /*  Check for error case */
      if (water_calib->pad_3_calib[0] < 7) {
        *small_delta_reset = 1U;
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
      }
    }

    /*     %% Pad 4 */
    if (water_calib->pad_4_calib[1] >= 50) {
      water_calib->pad_4_calib_done = 1U;

      /*  Check for error case */
      if (water_calib->pad_4_calib[0] < 7) {
        *small_delta_reset = 1U;
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
      }
    }

    /*     %% Pad 5 */
    if (water_calib->pad_5_calib[1] >= 50) {
      water_calib->pad_5_calib_done = 1U;

      /*  Check for error case */
      if (water_calib->pad_5_calib[0] < 7) {
        *small_delta_reset = 1U;
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
      }
    }

    /*     %% Pad 6 */
    if (water_calib->pad_6_calib[1] >= 50) {
      water_calib->pad_6_calib_done = 1U;

      /*  Check for error case */
      if (water_calib->pad_6_calib[0] < 7) {
        *small_delta_reset = 1U;
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
      }
    }

    /*     %% Pad 7 */
    if (water_calib->pad_7_calib[1] >= 50) {
      water_calib->pad_7_calib_done = 1U;

      /*  Check for error case */
      if (water_calib->pad_7_calib[0] < 7) {
        *small_delta_reset = 1U;
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
      }
    }

    /*     %% Pad 8 */
    if (water_calib->pad_8_calib[1] >= 50) {
      water_calib->pad_8_calib_done = 1U;

      /*  Check for error case */
      if (water_calib->pad_8_calib[0] < 7) {
        *small_delta_reset = 1U;
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
        prev_water_data->water_cal_error_count = 0U;
      }
    }
  } else {
    prev_water_data->pad1_OA = 800;
    prev_water_data->pad2_OA = 800;
    prev_water_data->pad3_OA = 800;
    prev_water_data->pad4_OA = 800;
    prev_water_data->pad5_OA = 800;
    prev_water_data->pad6_OA = 800;
    prev_water_data->pad7_OA = 800;
    prev_water_data->pad8_OA = 800;
  }
}

/*
 * File trailer for waterCalibration.c
 *
 * [EOF]
 */
