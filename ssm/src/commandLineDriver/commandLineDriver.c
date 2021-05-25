/**************************************************************************************************
* \file     commandLineDriver.c
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

#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include "commandLineDriver.h"
#include "HW_TERM.h"

void gvCLD_Invoke_The_Handler_For_This_Command(char *pszGivenBuff, uint32_t ulMaxLen);
void gvCLD_Register_This_Command_Handler(CLD_Command_Handler_s *pPtrGivenHandler);
void gvCLD_Register_This_Default_Command_Handler(CLD_Command_Handler_s *pPtrGivenHandler);
void gvCLD_Initialize(void);
static CLD_Command_Handler_Function_t *xpFindHandler(char *pszGivenCommand);
static void xvPrintHelp(void);
static inline bool xfIsNotValidArgChar(char chChar);
static inline bool xfIsValidArgChar(char chChar);
static inline bool xfIsNull(char chChar);

static CLD_Command_Handler_s  xHandler_List[MAX_REGISTERED_HANDLERS] = {{0}};


/**
 * @fn     void gvCLD_Invoke_The_Handler_For_This_Command(char *pszGivenBuff, uint32_t ulMaxLen)
 *
 * @brief  Given a buffer containing some input from the command line,
 *         use the first argument found in that buffer to identify
 *         the command and the corresponding handler for that command.
 *         Then either invoke the handler and return true,
 *         or return false when there is no matching command.
 *
 * @param  pszGivenBuff = user input buff
 * @param  ulMaxLen = length of input buff
 *
 * @return N/A
 *
 * @author Bob Ensink
 *
 * @note   N/A
 *
 */
void gvCLD_Invoke_The_Handler_For_This_Command(char *pszGivenBuff, uint32_t ulMaxLen)
{
    bool fEndOfCommand = false;
    uint16_t uwPos = 0u;
    char *pszArgv[MAXARGS];
    int swArgc = 0;

    /** ###Functional Overview: */

    /** - Echo back input string without elog header (add line return) */
    //strcat(pszGivenBuff, "\n\r");

    /** - Parse given command line buff into argc and argv */
    while ( true )
    {
        /* Consume any leading whitespace to find start of next arg */
        while ( true == xfIsNotValidArgChar(pszGivenBuff[uwPos]) )
        {
            if  ( true == xfIsNull(pszGivenBuff[uwPos]) )
            {
                fEndOfCommand = true;
                break;
            }

            uwPos++;

            if ( uwPos >= ulMaxLen )
            {
                fEndOfCommand = true;
                break;
            }
        }
        if ( true == fEndOfCommand )
        {
            break;
        }

        /* Save a pointer to first char of next arg and increment arg count. */
        pszArgv[swArgc++] = &pszGivenBuff[uwPos];

        /* Advance to first separator after this new argument. */
        while ( true == xfIsValidArgChar(pszGivenBuff[uwPos]) )
        {
            uwPos++;

            if ( uwPos >= ulMaxLen )
            {
                fEndOfCommand = true;
                break;
            }

            if  ( true == xfIsNull(pszGivenBuff[uwPos]) )
            {
                fEndOfCommand = true;
                break;
            }
        }

        if ( true == fEndOfCommand )
        {
            break;
        }

        /* Replace that separator with NULL term char. */
        pszGivenBuff[uwPos] = '\0';

        /* Advance to the next char. */
        /* And parse the next char if we are not at the end of the buffer. */
        uwPos++;

        if ( uwPos >= ulMaxLen )
        {
            fEndOfCommand = true;
            break;
        }
    }

    /* Nothing to invoke, when command is empty. */
    if ( 0 != swArgc )
    {
        /** - Invoke the corresponding handler if one can be matched. */

        CLD_Command_Handler_Function_t *handler = xpFindHandler(pszArgv[COMMAND_IDX]);

        if ( NULL != handler )
        {
            handler(swArgc, pszArgv);
        }
        /* Otherwise, print help, as needed. */
        else if ( 0 == strcmp("help", pszArgv[COMMAND_IDX]) )
        {
            xvPrintHelp();
        }

        /* Otherwise, invoke the default handler, if there is one. */
        else if ( NULL != xHandler_List[DEFAULT_HANDLER_IDX].pfnPtrFunction )
        {
            handler = xHandler_List[DEFAULT_HANDLER_IDX].pfnPtrFunction;
            handler(swArgc, pszArgv);
        }
        else
        {
            HW_TERM_Print("Command not found");
        }
    }
};

/**
 * @fn     void gvCLD_Register_This_Command_Handler(CLD_Command_Handler_s *pPtrGivenHandler)
 *
 * @brief  Register a new CLI command and cb function
 *
 * @param  pPtrGivenHandler = handler struct to register
 *
 * @return N/A
 *
 * @author Bob Ensink
 *
 * @note   N/A
 *
 */
void gvCLD_Register_This_Command_Handler(CLD_Command_Handler_s *pPtrGivenHandler)
{
     uint16_t uwIdx = 0u;

    /** ###Functional Overview: */

    /* Find the first empty slot; */
    /* but don't use the last one, that's for the default handler. */
    for ( uwIdx = 0; uwIdx < DEFAULT_HANDLER_IDX; uwIdx++ )
    {
        /** - Copy the given struct into the empty slot to register the handler */
        if( xHandler_List[uwIdx].pfnPtrFunction == (CLD_Command_Handler_Function_t *)NULL )
        {
            xHandler_List[uwIdx] = *pPtrGivenHandler;
            break;
        }
    }
    if ( uwIdx >= DEFAULT_HANDLER_IDX )
    {
        /** @todo log */
    }
};


/**
 * @fn     void gvCLD_Register_This_Default_Command_Handler(CLD_Command_Handler_s *pPtrGivenHandler)
 *
 * @brief  Register a default CLI command and cb function
 *
 * @param  pPtrGivenHandler = handler struct to register
 *
 * @return N/A
 *
 * @author Bob Ensink
 *
 * @note   N/A
 *
 */
void gvCLD_Register_This_Default_Command_Handler(CLD_Command_Handler_s *pPtrGivenHandler)
{
    /** ###Functional Overview: */

    /** - Register handler */
    xHandler_List[DEFAULT_HANDLER_IDX] = *pPtrGivenHandler;
};


/**
 * @fn     void gvCLD_Initialize(void)
 *
 * @brief  Perform any module initialization
 *
 * @return N/A
 *
 * @author Bob Ensink
 *
 * @note   N/A
 *
 */
void gvCLD_Initialize(void)
{

    uint8_t ubIndex =0;

    /** ###Functional Overview: */

    for(ubIndex = 0; ubIndex < MAX_REGISTERED_HANDLERS; ubIndex++)
    {
        xHandler_List[ubIndex].pfnPtrFunction = NULL;
    }

    /** - Perform any module initialization */
};

/**
 * @fn     static CLD_Command_Handler_Function_t *xpFindHandler(char *pszGivenCommand)
 *
 * @brief  Find a registered CLI command and cb function
 *
 * @param  pszGivenCommand = command the user entered
 *
 * @return The registered command handler
 *
 * @author Bob Ensink
 *
 * @note   N/A
 *
 */
static CLD_Command_Handler_Function_t *xpFindHandler(char *pszGivenCommand)
{
    CLD_Command_Handler_Function_t *foundHandler = (CLD_Command_Handler_Function_t *)NULL;
    uint16_t uwIdx = 0u;

    /** ###Functional Overview: */

    /** - Find the handler matching the user input */
    for ( uwIdx = 0; uwIdx < DEFAULT_HANDLER_IDX; uwIdx++ )
    {
        if ( (CLD_Command_Handler_Function_t *)NULL == xHandler_List[uwIdx].pfnPtrFunction )
        {
            continue;
        }
        if ( 0 == strcmp(pszGivenCommand, xHandler_List[uwIdx].pszCmdString) )
        {
            foundHandler = xHandler_List[uwIdx].pfnPtrFunction;
            break;
        }
    }

    return foundHandler;
};


/**
 * @fn     static void xvPrintHelp(void)
 *
 * @brief  Print the help menu
 *
 * @return N/A
 *
 * @author Bob Ensink
 *
 * @note   N/A
 *
 */
static void xvPrintHelp(void)
{
    uint16_t uwIdx = 0u;

    /** ###Functional Overview: */

    HW_TERM_Print("\n\n");

    /** - Print help menu */
    for ( uwIdx = 0; uwIdx < DEFAULT_HANDLER_IDX; uwIdx++ )
    {
        if ( (CLD_Command_Handler_Function_t *)NULL == xHandler_List[uwIdx].pfnPtrFunction )
        {
            continue;
        }

        HW_TERM_Print((uint8_t *)xHandler_List[uwIdx].pszCmdString);
        HW_TERM_Print(" - ");
        HW_TERM_Print((uint8_t *)xHandler_List[uwIdx].pszUsageString);
        HW_TERM_Print("\n\n");
    }

    HW_TERM_Print("\n");
};

/**
 * @fn     static inline bool xfIsNotValidArgChar(char chChar)
 *
 * @brief  Given a single character,
 *         return true iff this character is not valid
 *         as part of an argument, i.e., look for something
 *         that would separate one arg from another,
 *         typically some whitespace character.
 *
 * @param  chChar = char to check
 *
 * @return - true if this is a seperation character
 *         - false if this is not a seperation character
 *
 * @author Bob Ensink
 *
 * @note   N/A
 *
 */
static inline bool xfIsNotValidArgChar(char chChar)
{
    bool fIsNotValidFlag = false;

    /** ###Functional Overview: */

    /** - Check for seperation characters. Bail when found */
    /* space */
    if ( '\x20' == chChar )
    {
        fIsNotValidFlag = true;
    }
    /* tab */
    if ( '\t' == chChar )
    {
        fIsNotValidFlag = true;
    }
    /* new line */
    if ( '\n' == chChar )
    {
        fIsNotValidFlag = true;
    }
    /* null */
    if ( '\0' == chChar )
    {
        fIsNotValidFlag = true;
    }

    return fIsNotValidFlag;
};

/**
 * @fn     static inline bool xfIsNull(char chChar)
 *
 * @brief  Check if this is a null character
 *
 * @param  chChar = char to check
 *
 * @return - true if this is a null character
 *         - false if this is not a null character
 *
 * @author Bob Ensink
 *
 * @note   N/A
 *
 */
static inline bool xfIsNull(char chChar)
{
    bool fNullFlag;

    /** ###Functional Overview: */

    /** - Check for null */
    if ( '\0' == chChar )
    {
        fNullFlag = true;
    }
    else
    {
        fNullFlag = false;
    }

    return fNullFlag;
};

/**
 * @fn     static inline bool xfIsValidArgChar(char chChar)
 *
 * @brief  Check if this is a valid character
 *
 * @param  chChar = char to check
 *
 * @return - true if this is a valid character
 *         - false if this is not a valid character
 *
 * @author Bob Ensink
 *
 * @note   N/A
 *
 */
static inline bool xfIsValidArgChar(char chChar)
{
    /** ###Functional Overview: */

    /** - Check if valid char */
    return !xfIsNotValidArgChar(chChar);
};
