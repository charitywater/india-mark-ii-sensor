/*
 * File: initializeStrokeAlgorithm_types.h
 *
 * MATLAB Coder version            : 5.0
 * C/C++ source code generated on  : 18-May-2021 11:59:32
 */

#ifndef INITIALIZESTROKEALGORITHM_TYPES_H
#define INITIALIZESTROKEALGORITHM_TYPES_H

/* Include Files */
#include "rtwtypes.h"

/* Type Definitions */
#ifndef typedef_StrokeTransitionState
#define typedef_StrokeTransitionState

typedef uint8_T StrokeTransitionState;

#endif                                 /*typedef_StrokeTransitionState*/

#ifndef StrokeTransitionState_constants
#define StrokeTransitionState_constants

/* enum StrokeTransitionState */
#define no_activity                    ((StrokeTransitionState)0U)
#define finding_peak                   ((StrokeTransitionState)1U)
#define finding_valley                 ((StrokeTransitionState)2U)
#endif                                 /*StrokeTransitionState_constants*/

#ifndef typedef_TransitionType
#define typedef_TransitionType

typedef uint8_T TransitionType;

#endif                                 /*typedef_TransitionType*/

#ifndef TransitionType_constants
#define TransitionType_constants

/* enum TransitionType */
#define no_transition                  ((TransitionType)0U)
#define no_transition_activity         ((TransitionType)1U)
#define transition_peak                ((TransitionType)2U)
#define transition_valley              ((TransitionType)3U)
#endif                                 /*TransitionType_constants*/

#ifndef typedef_accumStrokeCount_t
#define typedef_accumStrokeCount_t

typedef struct {
  uint16_T num_windows_processed;
  uint16_T wet_stroke_count_sum;
  uint32_T wet_percent_displacement_sum;
  uint16_T dry_stroke_count_sum;
  uint32_T dry_percent_displacement_sum;
  uint8_T mag_calibration_changed;
} accumStrokeCount_t;

#endif                                 /*typedef_accumStrokeCount_t*/

#ifndef typedef_strokeTransition_t
#define typedef_strokeTransition_t

typedef struct {
  TransitionType type;
  int16_T val;
  int16_T idx;
} strokeTransition_t;

#endif                                 /*typedef_strokeTransition_t*/

#ifndef typedef_strokeDetectInfo_t
#define typedef_strokeDetectInfo_t

typedef struct {
  uint8_T is_first;
  strokeTransition_t last_transition;
} strokeDetectInfo_t;

#endif                                 /*typedef_strokeDetectInfo_t*/

#ifndef typedef_strokeTransitionInfo_t
#define typedef_strokeTransitionInfo_t

typedef struct {
  StrokeTransitionState state;
  uint8_T is_first;
  uint8_T downslope_cnt;
  int16_T downslope_sum;
  uint8_T upslope_cnt;
  int16_T upslope_sum;
  int16_T peak_val;
  int16_T peak_idx;
  uint8_T state_switch_cnt;
  int16_T prev_val;
} strokeTransitionInfo_t;

#endif                                 /*typedef_strokeTransitionInfo_t*/
#endif

/*
 * File trailer for initializeStrokeAlgorithm_types.h
 *
 * [EOF]
 */
