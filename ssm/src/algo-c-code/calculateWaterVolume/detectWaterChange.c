/*
 * File: detectWaterChange.c
 *
 * MATLAB Coder version            : 5.0
 * C/C++ source code generated on  : 18-May-2021 11:58:35
 */

/* Include Files */
#include "detectWaterChange.h"
#include "calculateWaterVolume.h"

/* Function Definitions */

/*
 * Arguments    : int32_T diff
 *                padWaterState_t *water_state
 *                uint8_T counter_thresh
 * Return Type  : void
 */
void detectWaterChange(int32_T diff, padWaterState_t *water_state, uint8_T
  counter_thresh)
{
  /*  Detect the front of the water ON point  */
  /*  These tend to be high frequency (high difference from point to point) */
  if (diff <= -5L) {
    water_state->present_type = water_present;
    water_state->draining_count = 0U;
  } else {
    /*  Sit in water draining until timeout or another large negative */
    /*  differential */
    if ((water_state->present_type == water_present) ||
        (water_state->present_type == water_draining)) {
      water_state->present_type = water_draining;

      /*  Increment the drainings state counter */
      if (water_state->draining_count < 255) {
        water_state->draining_count++;
      }

      /*  Reset to present if we see a large enough negative diff */
      /*  If we've been in the draining state for so long, transition to */
      /*  not present */
      /*  NOTE: we can also get reset in the water volume computation if */
      /*  there is pad activity above this pad... */
      if ((water_state->draining_count > counter_thresh) || (diff >= 4L)) {
        water_state->present_type = water_not_present;
        water_state->draining_count = 0U;
      }
    }
  }
}

/*
 * File trailer for detectWaterChange.c
 *
 * [EOF]
 */
