/*
 * File: promotePadStates.c
 *
 * MATLAB Coder version            : 5.0
 * C/C++ source code generated on  : 27-Oct-2022 08:10:46
 */

/* Include Files */
#include "promotePadStates.h"
#include "calculateWaterVolume.h"

/* Function Definitions */

/*
 * Arguments    : waterAlgoData_t *prev_water_data
 * Return Type  : void
 */
void promotePadStates(waterAlgoData_t *prev_water_data)
{
  /*  Section to check teh pad states and updated the necessary pads */
  if ((prev_water_data->pad1_present.present_type == water_present) &&
      (prev_water_data->pad2_present.present_type != water_not_present)) {
    /*  Function to update a lower pad water state to the master state */
    /*  Promote the state to the master state */
    if (prev_water_data->pad2_present.present_type <
        prev_water_data->pad1_present.present_type) {
      prev_water_data->pad2_present.present_type =
        prev_water_data->pad1_present.present_type;
      prev_water_data->pad2_present.draining_count = 0U;
    } else {
      if (prev_water_data->pad2_present.present_type ==
          prev_water_data->pad1_present.present_type) {
        prev_water_data->pad2_present.draining_count = 0U;
      }
    }

    /*  Function to update a lower pad water state to the master state */
    /*  Promote the state to the master state */
    if (prev_water_data->pad3_present.present_type <
        prev_water_data->pad1_present.present_type) {
      prev_water_data->pad3_present.present_type =
        prev_water_data->pad1_present.present_type;
      prev_water_data->pad3_present.draining_count = 0U;
    } else {
      if (prev_water_data->pad3_present.present_type ==
          prev_water_data->pad1_present.present_type) {
        prev_water_data->pad3_present.draining_count = 0U;
      }
    }

    /*  Function to update a lower pad water state to the master state */
    /*  Promote the state to the master state */
    if (prev_water_data->pad4_present.present_type <
        prev_water_data->pad1_present.present_type) {
      prev_water_data->pad4_present.present_type =
        prev_water_data->pad1_present.present_type;
      prev_water_data->pad4_present.draining_count = 0U;
    } else {
      if (prev_water_data->pad4_present.present_type ==
          prev_water_data->pad1_present.present_type) {
        prev_water_data->pad4_present.draining_count = 0U;
      }
    }

    /*  Function to update a lower pad water state to the master state */
    /*  Promote the state to the master state */
    if (prev_water_data->pad5_present.present_type <
        prev_water_data->pad1_present.present_type) {
      prev_water_data->pad5_present.present_type =
        prev_water_data->pad1_present.present_type;
      prev_water_data->pad5_present.draining_count = 0U;
    } else {
      if (prev_water_data->pad5_present.present_type ==
          prev_water_data->pad1_present.present_type) {
        prev_water_data->pad5_present.draining_count = 0U;
      }
    }

    /*  Function to update a lower pad water state to the master state */
    /*  Promote the state to the master state */
    if (prev_water_data->pad6_present.present_type <
        prev_water_data->pad1_present.present_type) {
      prev_water_data->pad6_present.present_type =
        prev_water_data->pad1_present.present_type;
      prev_water_data->pad6_present.draining_count = 0U;
    } else {
      if (prev_water_data->pad6_present.present_type ==
          prev_water_data->pad1_present.present_type) {
        prev_water_data->pad6_present.draining_count = 0U;
      }
    }

    /*  Function to update a lower pad water state to the master state */
    /*  Promote the state to the master state */
    if (prev_water_data->pad7_present.present_type <
        prev_water_data->pad1_present.present_type) {
      prev_water_data->pad7_present.present_type =
        prev_water_data->pad1_present.present_type;
      prev_water_data->pad7_present.draining_count = 0U;
    } else {
      if (prev_water_data->pad7_present.present_type ==
          prev_water_data->pad1_present.present_type) {
        prev_water_data->pad7_present.draining_count = 0U;
      }
    }

    /*  Function to update a lower pad water state to the master state */
    /*  Promote the state to the master state */
    if (prev_water_data->pad8_present.present_type <
        prev_water_data->pad1_present.present_type) {
      prev_water_data->pad8_present.present_type =
        prev_water_data->pad1_present.present_type;
      prev_water_data->pad8_present.draining_count = 0U;
    } else {
      if (prev_water_data->pad8_present.present_type ==
          prev_water_data->pad1_present.present_type) {
        prev_water_data->pad8_present.draining_count = 0U;
      }
    }
  } else if ((prev_water_data->pad2_present.present_type == water_present) &&
             (prev_water_data->pad3_present.present_type != water_not_present))
  {
    /*  Function to update a lower pad water state to the master state */
    /*  Promote the state to the master state */
    if (prev_water_data->pad3_present.present_type <
        prev_water_data->pad2_present.present_type) {
      prev_water_data->pad3_present.present_type =
        prev_water_data->pad2_present.present_type;
      prev_water_data->pad3_present.draining_count = 0U;
    } else {
      if (prev_water_data->pad3_present.present_type ==
          prev_water_data->pad2_present.present_type) {
        prev_water_data->pad3_present.draining_count = 0U;
      }
    }

    /*  Function to update a lower pad water state to the master state */
    /*  Promote the state to the master state */
    if (prev_water_data->pad4_present.present_type <
        prev_water_data->pad2_present.present_type) {
      prev_water_data->pad4_present.present_type =
        prev_water_data->pad2_present.present_type;
      prev_water_data->pad4_present.draining_count = 0U;
    } else {
      if (prev_water_data->pad4_present.present_type ==
          prev_water_data->pad2_present.present_type) {
        prev_water_data->pad4_present.draining_count = 0U;
      }
    }

    /*  Function to update a lower pad water state to the master state */
    /*  Promote the state to the master state */
    if (prev_water_data->pad5_present.present_type <
        prev_water_data->pad2_present.present_type) {
      prev_water_data->pad5_present.present_type =
        prev_water_data->pad2_present.present_type;
      prev_water_data->pad5_present.draining_count = 0U;
    } else {
      if (prev_water_data->pad5_present.present_type ==
          prev_water_data->pad2_present.present_type) {
        prev_water_data->pad5_present.draining_count = 0U;
      }
    }

    /*  Function to update a lower pad water state to the master state */
    /*  Promote the state to the master state */
    if (prev_water_data->pad6_present.present_type <
        prev_water_data->pad2_present.present_type) {
      prev_water_data->pad6_present.present_type =
        prev_water_data->pad2_present.present_type;
      prev_water_data->pad6_present.draining_count = 0U;
    } else {
      if (prev_water_data->pad6_present.present_type ==
          prev_water_data->pad2_present.present_type) {
        prev_water_data->pad6_present.draining_count = 0U;
      }
    }

    /*  Function to update a lower pad water state to the master state */
    /*  Promote the state to the master state */
    if (prev_water_data->pad7_present.present_type <
        prev_water_data->pad2_present.present_type) {
      prev_water_data->pad7_present.present_type =
        prev_water_data->pad2_present.present_type;
      prev_water_data->pad7_present.draining_count = 0U;
    } else {
      if (prev_water_data->pad7_present.present_type ==
          prev_water_data->pad2_present.present_type) {
        prev_water_data->pad7_present.draining_count = 0U;
      }
    }

    /*  Function to update a lower pad water state to the master state */
    /*  Promote the state to the master state */
    if (prev_water_data->pad8_present.present_type <
        prev_water_data->pad2_present.present_type) {
      prev_water_data->pad8_present.present_type =
        prev_water_data->pad2_present.present_type;
      prev_water_data->pad8_present.draining_count = 0U;
    } else {
      if (prev_water_data->pad8_present.present_type ==
          prev_water_data->pad2_present.present_type) {
        prev_water_data->pad8_present.draining_count = 0U;
      }
    }
  } else if ((prev_water_data->pad3_present.present_type == water_present) &&
             (prev_water_data->pad4_present.present_type != water_not_present))
  {
    /*  Function to update a lower pad water state to the master state */
    /*  Promote the state to the master state */
    if (prev_water_data->pad4_present.present_type <
        prev_water_data->pad3_present.present_type) {
      prev_water_data->pad4_present.present_type =
        prev_water_data->pad3_present.present_type;
      prev_water_data->pad4_present.draining_count = 0U;
    } else {
      if (prev_water_data->pad4_present.present_type ==
          prev_water_data->pad3_present.present_type) {
        prev_water_data->pad4_present.draining_count = 0U;
      }
    }

    /*  Function to update a lower pad water state to the master state */
    /*  Promote the state to the master state */
    if (prev_water_data->pad5_present.present_type <
        prev_water_data->pad3_present.present_type) {
      prev_water_data->pad5_present.present_type =
        prev_water_data->pad3_present.present_type;
      prev_water_data->pad5_present.draining_count = 0U;
    } else {
      if (prev_water_data->pad5_present.present_type ==
          prev_water_data->pad3_present.present_type) {
        prev_water_data->pad5_present.draining_count = 0U;
      }
    }

    /*  Function to update a lower pad water state to the master state */
    /*  Promote the state to the master state */
    if (prev_water_data->pad6_present.present_type <
        prev_water_data->pad3_present.present_type) {
      prev_water_data->pad6_present.present_type =
        prev_water_data->pad3_present.present_type;
      prev_water_data->pad6_present.draining_count = 0U;
    } else {
      if (prev_water_data->pad6_present.present_type ==
          prev_water_data->pad3_present.present_type) {
        prev_water_data->pad6_present.draining_count = 0U;
      }
    }

    /*  Function to update a lower pad water state to the master state */
    /*  Promote the state to the master state */
    if (prev_water_data->pad7_present.present_type <
        prev_water_data->pad3_present.present_type) {
      prev_water_data->pad7_present.present_type =
        prev_water_data->pad3_present.present_type;
      prev_water_data->pad7_present.draining_count = 0U;
    } else {
      if (prev_water_data->pad7_present.present_type ==
          prev_water_data->pad3_present.present_type) {
        prev_water_data->pad7_present.draining_count = 0U;
      }
    }

    /*  Function to update a lower pad water state to the master state */
    /*  Promote the state to the master state */
    if (prev_water_data->pad8_present.present_type <
        prev_water_data->pad3_present.present_type) {
      prev_water_data->pad8_present.present_type =
        prev_water_data->pad3_present.present_type;
      prev_water_data->pad8_present.draining_count = 0U;
    } else {
      if (prev_water_data->pad8_present.present_type ==
          prev_water_data->pad3_present.present_type) {
        prev_water_data->pad8_present.draining_count = 0U;
      }
    }
  } else if ((prev_water_data->pad4_present.present_type == water_present) &&
             (prev_water_data->pad5_present.present_type != water_not_present))
  {
    /*  Function to update a lower pad water state to the master state */
    /*  Promote the state to the master state */
    if (prev_water_data->pad5_present.present_type <
        prev_water_data->pad4_present.present_type) {
      prev_water_data->pad5_present.present_type =
        prev_water_data->pad4_present.present_type;
      prev_water_data->pad5_present.draining_count = 0U;
    } else {
      if (prev_water_data->pad5_present.present_type ==
          prev_water_data->pad4_present.present_type) {
        prev_water_data->pad5_present.draining_count = 0U;
      }
    }

    /*  Function to update a lower pad water state to the master state */
    /*  Promote the state to the master state */
    if (prev_water_data->pad6_present.present_type <
        prev_water_data->pad4_present.present_type) {
      prev_water_data->pad6_present.present_type =
        prev_water_data->pad4_present.present_type;
      prev_water_data->pad6_present.draining_count = 0U;
    } else {
      if (prev_water_data->pad6_present.present_type ==
          prev_water_data->pad4_present.present_type) {
        prev_water_data->pad6_present.draining_count = 0U;
      }
    }

    /*  Function to update a lower pad water state to the master state */
    /*  Promote the state to the master state */
    if (prev_water_data->pad7_present.present_type <
        prev_water_data->pad4_present.present_type) {
      prev_water_data->pad7_present.present_type =
        prev_water_data->pad4_present.present_type;
      prev_water_data->pad7_present.draining_count = 0U;
    } else {
      if (prev_water_data->pad7_present.present_type ==
          prev_water_data->pad4_present.present_type) {
        prev_water_data->pad7_present.draining_count = 0U;
      }
    }

    /*  Function to update a lower pad water state to the master state */
    /*  Promote the state to the master state */
    if (prev_water_data->pad8_present.present_type <
        prev_water_data->pad4_present.present_type) {
      prev_water_data->pad8_present.present_type =
        prev_water_data->pad4_present.present_type;
      prev_water_data->pad8_present.draining_count = 0U;
    } else {
      if (prev_water_data->pad8_present.present_type ==
          prev_water_data->pad4_present.present_type) {
        prev_water_data->pad8_present.draining_count = 0U;
      }
    }
  } else if ((prev_water_data->pad5_present.present_type == water_present) &&
             (prev_water_data->pad6_present.present_type != water_not_present))
  {
    /*  Function to update a lower pad water state to the master state */
    /*  Promote the state to the master state */
    if (prev_water_data->pad6_present.present_type <
        prev_water_data->pad5_present.present_type) {
      prev_water_data->pad6_present.present_type =
        prev_water_data->pad5_present.present_type;
      prev_water_data->pad6_present.draining_count = 0U;
    } else {
      if (prev_water_data->pad6_present.present_type ==
          prev_water_data->pad5_present.present_type) {
        prev_water_data->pad6_present.draining_count = 0U;
      }
    }

    /*  Function to update a lower pad water state to the master state */
    /*  Promote the state to the master state */
    if (prev_water_data->pad7_present.present_type <
        prev_water_data->pad5_present.present_type) {
      prev_water_data->pad7_present.present_type =
        prev_water_data->pad5_present.present_type;
      prev_water_data->pad7_present.draining_count = 0U;
    } else {
      if (prev_water_data->pad7_present.present_type ==
          prev_water_data->pad5_present.present_type) {
        prev_water_data->pad7_present.draining_count = 0U;
      }
    }

    /*  Function to update a lower pad water state to the master state */
    /*  Promote the state to the master state */
    if (prev_water_data->pad8_present.present_type <
        prev_water_data->pad5_present.present_type) {
      prev_water_data->pad8_present.present_type =
        prev_water_data->pad5_present.present_type;
      prev_water_data->pad8_present.draining_count = 0U;
    } else {
      if (prev_water_data->pad8_present.present_type ==
          prev_water_data->pad5_present.present_type) {
        prev_water_data->pad8_present.draining_count = 0U;
      }
    }
  } else if ((prev_water_data->pad6_present.present_type == water_present) &&
             (prev_water_data->pad7_present.present_type != water_not_present))
  {
    /*  Function to update a lower pad water state to the master state */
    /*  Promote the state to the master state */
    if (prev_water_data->pad7_present.present_type <
        prev_water_data->pad6_present.present_type) {
      prev_water_data->pad7_present.present_type =
        prev_water_data->pad6_present.present_type;
      prev_water_data->pad7_present.draining_count = 0U;
    } else {
      if (prev_water_data->pad7_present.present_type ==
          prev_water_data->pad6_present.present_type) {
        prev_water_data->pad7_present.draining_count = 0U;
      }
    }

    /*  Function to update a lower pad water state to the master state */
    /*  Promote the state to the master state */
    if (prev_water_data->pad8_present.present_type <
        prev_water_data->pad6_present.present_type) {
      prev_water_data->pad8_present.present_type =
        prev_water_data->pad6_present.present_type;
      prev_water_data->pad8_present.draining_count = 0U;
    } else {
      if (prev_water_data->pad8_present.present_type ==
          prev_water_data->pad6_present.present_type) {
        prev_water_data->pad8_present.draining_count = 0U;
      }
    }
  } else {
    if ((prev_water_data->pad7_present.present_type == water_present) &&
        (prev_water_data->pad8_present.present_type != water_not_present)) {
      /*  Function to update a lower pad water state to the master state */
      /*  Promote the state to the master state */
      if (prev_water_data->pad8_present.present_type <
          prev_water_data->pad7_present.present_type) {
        prev_water_data->pad8_present.present_type =
          prev_water_data->pad7_present.present_type;
        prev_water_data->pad8_present.draining_count = 0U;
      } else {
        if (prev_water_data->pad8_present.present_type ==
            prev_water_data->pad7_present.present_type) {
          prev_water_data->pad8_present.draining_count = 0U;
        }
      }
    }
  }
}

/*
 * File trailer for promotePadStates.c
 *
 * [EOF]
 */
