/*
 * File: initializeWindows_types.h
 *
 * MATLAB Coder version            : 5.0
 * C/C++ source code generated on  : 27-Oct-2022 08:08:22
 */

#ifndef INITIALIZEWINDOWS_TYPES_H
#define INITIALIZEWINDOWS_TYPES_H

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

#ifndef typedef_b_padBlock_t
#define typedef_b_padBlock_t

typedef struct {
  uint16_T pad1[50];
  uint16_T pad2[50];
  uint16_T pad3[50];
  uint16_T pad4[50];
  uint16_T pad5[50];
  uint16_T pad6[50];
  uint16_T pad7[50];
  uint16_T pad8[50];
} b_padBlock_t;

#endif                                 /*typedef_b_padBlock_t*/

#ifndef typedef_magBlock_t
#define typedef_magBlock_t

typedef struct {
  int16_T x_lsb[20];
  int16_T y_lsb[20];
  int16_T z_lsb[20];
} magBlock_t;

#endif                                 /*typedef_magBlock_t*/

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

#ifndef typedef_padBlock_t
#define typedef_padBlock_t

typedef struct {
  uint16_T pad1[20];
  uint16_T pad2[20];
  uint16_T pad3[20];
  uint16_T pad4[20];
  uint16_T pad5[20];
  uint16_T pad6[20];
  uint16_T pad7[20];
  uint16_T pad8[20];
} padBlock_t;

#endif                                 /*typedef_padBlock_t*/

#ifndef typedef_padWindows_t
#define typedef_padWindows_t

typedef struct {
  padBlock_t blockA;
  padBlock_t blockB;
  b_padBlock_t blockOA;
  b_padBlock_t blockOB;
  uint16_T write_idx;
  WindowBlock write_block;
  Window read_window;
  uint8_T process;
  uint8_T first_pass;
} padWindows_t;

#endif                                 /*typedef_padWindows_t*/
#endif

/*
 * File trailer for initializeWindows_types.h
 *
 * [EOF]
 */
