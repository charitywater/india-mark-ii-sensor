/*
 * File: writeMagSample_types.h
 *
 * MATLAB Coder version            : 5.0
 * C/C++ source code generated on  : 27-Oct-2022 08:08:52
 */

#ifndef WRITEMAGSAMPLE_TYPES_H
#define WRITEMAGSAMPLE_TYPES_H

/* Include Files */
#include "rtwtypes.h"

/* Type Definitions */
#ifndef typedef_Window
#define typedef_Window

typedef uint8_T Window;

#endif                                 /*typedef_Window*/

#ifndef Window_constants
#define Window_constants

/* enum Window */
#define no_window                      ((Window)0U)
#define windowA                        ((Window)1U)
#define windowB                        ((Window)2U)
#endif                                 /*Window_constants*/

#ifndef typedef_WindowBlock
#define typedef_WindowBlock

typedef uint8_T WindowBlock;

#endif                                 /*typedef_WindowBlock*/

#ifndef WindowBlock_constants
#define WindowBlock_constants

/* enum WindowBlock */
#define b_blockA                       ((WindowBlock)1U)
#define b_blockB                       ((WindowBlock)2U)
#define b_blockOA                      ((WindowBlock)3U)
#define b_blockOB                      ((WindowBlock)4U)
#endif                                 /*WindowBlock_constants*/

#ifndef typedef_b_magBlock_t
#define typedef_b_magBlock_t

typedef struct {
  int16_T x_lsb[50];
  int16_T y_lsb[50];
  int16_T z_lsb[50];
} b_magBlock_t;

#endif                                 /*typedef_b_magBlock_t*/

#ifndef typedef_magBlock_t
#define typedef_magBlock_t

typedef struct {
  int16_T x_lsb[20];
  int16_T y_lsb[20];
  int16_T z_lsb[20];
} magBlock_t;

#endif                                 /*typedef_magBlock_t*/

#ifndef typedef_magSample_t
#define typedef_magSample_t

typedef struct {
  int16_T x_lsb;
  int16_T y_lsb;
  int16_T z_lsb;
  int16_T temp_lsb;
  uint8_T status;
} magSample_t;

#endif                                 /*typedef_magSample_t*/

#ifndef typedef_magWindows_t
#define typedef_magWindows_t

typedef struct {
  magBlock_t blockA;
  magBlock_t blockB;
  b_magBlock_t blockOA;
  b_magBlock_t blockOB;
  uint16_T write_idx;
  WindowBlock write_block;
  Window read_window;
  uint8_T process;
  uint8_T first_pass;
} magWindows_t;

#endif                                 /*typedef_magWindows_t*/
#endif

/*
 * File trailer for writeMagSample_types.h
 *
 * [EOF]
 */
