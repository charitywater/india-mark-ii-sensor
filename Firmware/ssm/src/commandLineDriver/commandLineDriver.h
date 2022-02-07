/**************************************************************************************************
* \file     commandLineDriver.h
* \brief    CLI parsing and routing
*
* \par      Copyright Notice
*           Copyright(C) 2021 by charity: water
*           
*           All rights reserved. No part of this software may be disclosed or
*           distributed in any form or by any means without the prior written
*           consent of charity: water.
* \date     01/29/2021
* \author   Twisthink
***************************************************************************************************/

#ifndef QPR2_TRIGGER_DEVICE_DRIVERS_COMMANDLINEDRIVER_H_
#define QPR2_TRIGGER_DEVICE_DRIVERS_COMMANDLINEDRIVER_H_

/* Includes */
#include <stdint.h>

/*========================================================================*
 *  SECTION - Global definitions                                          *
 *========================================================================*
 */

/** @brief index of the cli command */
#define COMMAND_IDX                 0

/** @brief First argument of the cli command */
#define FIRST_ARG_IDX               1

/** @brief Second argument of the cli command */
#define SECOND_ARG_IDX              2

/** @brief Third argument of the cli command */
#define THIRD_ARG_IDX               3

/** @brief Third argument of the cli command */
#define FOURTH_ARG_IDX              4

/** @brief Third argument of the cli command */
#define FIFTH_ARG_IDX               5

/** @brief Third argument of the cli command */
#define SIXTH_ARG_IDX               6

/** @brief Third argument of the cli command */
#define SEVENTH_ARG_IDX             7

/** @brief Third argument of the cli command */
#define EIGHTH_ARG_IDX              8

/** @brief Third argument of the cli command */
#define NINTH_ARG_IDX               9

/** @brief If the cli command contains 0 arguments, there will be
 * 1 item (the command) */
#define ZERO_ARGUMENTS              1

/** @brief If the cli command contains 1 argument, there will be
 * 2 items (the command plus the argument = 2) */
#define ONE_ARGUMENT                2

/** @brief If the cli command contains 2 arguments, there will be
 * 3 items (the command plus 2 arguments = 3) */
#define TWO_ARGUMENTS               3

/** @brief If the cli command contains 3 arguments, there will be
 * 4 items (the command plus 3 arguments = 4) */
#define THREE_ARGUMENTS             4

/** @brief If the cli command contains 3 arguments, there will be
 * 7 items (the command plus 6 arguments = 7) */
#define SIX_ARGUMENTS              7

/** @brief If the cli command contains 3 arguments, there will be
 * 10 items (the command plus 9 arguments = 10) */
#define NINE_ARGUMENTS              10

/** @brief Supported number of command handlers */
#define MAX_REGISTERED_HANDLERS     24

/** @brief Index of the default handler */
#define DEFAULT_HANDLER_IDX         (MAX_REGISTERED_HANDLERS - 1)

/** @brief Supported number of command arguments */
#define MAXARGS                     12

/** @brief Defines the signature of a command handler function. */
typedef void (CLD_Command_Handler_Function_t)(int swArgc, char **pszArgv);

/**
 * @struct  commandHandler
 * @brief   Describes a command line handler.
 *
 * @typedef CLD_Command_Handler_s
 * @brief   @ref commandHandler type definition
 */
typedef struct commandHandler
{
    CLD_Command_Handler_Function_t *pfnPtrFunction; /**< pointer to cb fcn */
    const char *pszCmdString;                       /**< unique string which the dispatcher will match */
    const char *pszUsageString;                     /**< sub commands */
} CLD_Command_Handler_s;


/*========================================================================*
 *  SECTION - extern global variables (minimize global variable use)      *
 *========================================================================*
 */

/*========================================================================*
 *  SECTION - extern global functions                                     *
 *========================================================================*
 */

extern void gvCLD_Invoke_The_Handler_For_This_Command(char *pszGivenBuff, uint32_t ulMaxLen);
extern void gvCLD_Register_This_Command_Handler(CLD_Command_Handler_s *pPtrGivenHandler);
extern void gvCLD_Register_This_Default_Command_Handler(CLD_Command_Handler_s *pPtrGivenHandler);
extern void gvCLD_Initialize(void);

#endif /* QPR2_TRIGGER_DEVICE_DRIVERS_COMMANDLINEDRIVER_H_ */
