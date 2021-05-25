/*
 * File: hourlyWaterVolume.c
 *
 * MATLAB Coder version            : 5.0
 * C/C++ source code generated on  : 18-May-2021 11:58:16
 */

/* Include Files */
#include "hourlyWaterVolume.h"

/* Custom Source Code */

/* Copyright Notice
 * Copyright 2021 charity: water
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/* Function Definitions */

/*
 * Arguments    : waterAlgoData_t *algo_data
 *                pumpUsage_t *pump_usage
 *                uint8_T hour
 *                uint8_T day
 *                ReasonCodes *reason_code
 *                hourlyWaterInfo_t *hourly_water_info
 * Return Type  : void
 */
void hourlyWaterVolume(waterAlgoData_t *algo_data, pumpUsage_t *pump_usage,
  uint8_T hour, uint8_T day, ReasonCodes *reason_code, hourlyWaterInfo_t
  *hourly_water_info)
{
  int16_T session_volume;
  int32_T a;
  int32_T mx;
  uint8_T mx_hour;
  int16_T i;
  int16_T b_day;

  /*  Function used to take teh hourly integration sum and convert it into a */
  /*  volume */
  *reason_code = reason_code_none;
  session_volume = 0;

  /*  Calculate the remaining volume if a session is going */
  if (algo_data->present != 0) {
    /*  Calculate the % no change */
    if (algo_data->session_sample_counter != 0L) {
      if (algo_data->no_change_counter < 21474836L) {
        a = (int32_T)(algo_data->no_change_counter * 100LL) /
          algo_data->session_sample_counter;
      } else {
        a = 100L;
      }

      /*  Calculate the scaler (Y = b - mx , x = no_change_percentage) */
      /*  Calculate the water volume */
      session_volume = (int16_T)((algo_data->water_int_value * ((5368709120LL -
        819LL * (a << 15L)) >> 15L) + 536870912LL) >> 30L);
    }

    /*  Display the plots if the target is the mex file */
    /*  Reset the session volume variables but not the pad variables - this could be */
    /*  called mid-session so we are cutting the session in half */
    algo_data->water_int_value = 0L;
    algo_data->session_sample_counter = 0L;
    algo_data->no_change_counter = 0L;
  }

  /*  Calculate volume */
  if (algo_data->water_volume_sum < MAX_int32_T - session_volume) {
    hourly_water_info->volume = algo_data->water_volume_sum + session_volume;
  } else {
    hourly_water_info->volume = MAX_int32_T;
    *reason_code = water_volume_capped;
  }

  /*  Compute the pump usage - this can be used to indicate the quality of the */
  /*  pump health metric */
  hourly_water_info->percent_pump_usage = 0L;
  if (algo_data->accum_processed_sample_cnt != 0L) {
    if (algo_data->accum_processed_sample_cnt < 21474836L) {
      hourly_water_info->percent_pump_usage = (int32_T)
        (algo_data->accum_water_sample_cnt * 100LL) /
        algo_data->accum_processed_sample_cnt;
    } else {
      hourly_water_info->percent_pump_usage = 100L;
    }
  }

  /*  Track the pump usage over time to choose the best hour to measure pump */
  /*  health */
  /*  The first time we enter into this function after a reset, we will mark */
  /*  the first hour - once full, we will recompute the max usage hour when we */
  /*  come back around to this hour */
  if (pump_usage->is_first != 0) {
    /*  First time through this function after a reset */
    pump_usage->first_hour = hour;
    pump_usage->is_filling_hourly_usage = 1U;
    pump_usage->first_day = day;
    pump_usage->is_filling_daily_usage = 1U;
    pump_usage->is_first = 0U;
  } else {
    if (hour == pump_usage->first_hour) {
      pump_usage->is_filling_hourly_usage = 0U;

      /*  The hourly usage buffer is now full - valid values in each element */
    }

    if ((pump_usage->is_filling_hourly_usage == 0) && (hour == 0)) {
      /*  Compute the max usage hour for this day */
      mx = 0L;
      mx_hour = 8U;

      /*  Default to the first hour of the day */
      for (i = 0; i < 24; i++) {
        if (pump_usage->hourly_usage[i] > mx) {
          /*  Take the earliest max - biased toward morning usually... */
          mx = pump_usage->hourly_usage[i];
          mx_hour = (uint8_T)i;

          /*  Adjust index i for to be zero-based hour (hours are from 0 to 23) */
        }
      }

      pump_usage->prev_day_max_usage_hour = mx_hour;

      /*  Keep track of the previous day's max usage hour to use a fall-back if needed */
      /*  Store this hour into the daily buffer */
      if (day < 7) {
        /*  Store result in the previous day's index  */
        /*  Over writes the last weeks usage with the current weeks usage */
        if (day == 0) {
          b_day = 6;
        } else {
          b_day = (uint8_T)(day - 1);
        }

        pump_usage->daily_usage[b_day] = mx_hour;

        /*  Add 1 for MATLAB - should be removed with code generation */
      }

      if (day == pump_usage->first_day) {
        pump_usage->is_filling_daily_usage = 0U;

        /*  We've filled in a week's worth of max usage data */
      }
    }
  }

  /*  Store this usage for this hour - we are starting over by */
  /*  refilling the hourly buffer with new data */
  if (hour < 24) {
    /*  Overwrites the last day's usage with the current day's usage */
    pump_usage->hourly_usage[hour] = hourly_water_info->percent_pump_usage;

    /*  Add 1 for MATLAB - should be removed with code generation */
  }

  /*  Reset the hourly water data */
  algo_data->water_volume_sum = 0L;
  algo_data->accum_processed_sample_cnt = 0L;
  algo_data->accum_water_sample_cnt = 0L;
}

/*
 * File trailer for hourlyWaterVolume.c
 *
 * [EOF]
 */
