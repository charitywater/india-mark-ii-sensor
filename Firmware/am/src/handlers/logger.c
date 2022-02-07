/**************************************************************************************************
* \file     logger.c
* \brief    Log data at various levels for debugging and documenting errors
*
* \par      Copyright Notice
*           Copyright 2021 charity: water
*
*           Licensed under the Apache License, Version 2.0 (the "License");
*           you may not use this file except in compliance with the License.
*           You may obtain a copy of the License at
*
*               http://www.apache.org/licenses/LICENSE-2.0
*
*           Unless required by applicable law or agreed to in writing, software
*           distributed under the License is distributed on an "AS IS" BASIS,
*           WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*           See the License for the specific language governing permissions and
*           limitations under the License.
*           
* \date     2/1/2021
* \author   Twisthink
*
***************************************************************************************************/

#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "stdbool.h"
#include <ctype.h>
#include <CLI.h>
#include <logger.h>

#include <stm32l4xx_hal.h>

#include <logTypes.h>
#include <uart.h>
#include "FreeRTOS.h"
#include"semphr.h"
#include "task.h"

#define MAX_PRINT_CHARS     255

static bool loggerInitialized = false;
static char prepBuf[MAX_PRINT_CHARS];
static char lineBuf[MAX_PRINT_CHARS];

//print enabled is OFF by default
//can use the CLI to enable it, which will allow
//all levels of logging to be printed
static bool enablePrinting = false;

static tLogLvl minLevelToPrint = eLogLvlInfo;

SemaphoreHandle_t xLogMutex;

// Local functions
static void xCommandHandlerForLogger(int argc, char **argv);
static void xFormatStringAndPutInLineBuffer(tLogLvl given_level, char* msg, ... );

bool LOG_initializeLogger()
{
    /*register a command handler function */
    CLI_Command_Handler_s cmdHandler;
    cmdHandler.ptrFunction = &xCommandHandlerForLogger;
    cmdHandler.cmdString   = "log";
    cmdHandler.usageString = "\n\r\tenable \n\r\tdisable \n\r\tinfo \n\r\tdebug \n\r\toffnom \n\r\terror \n\r\tfatal \n\r\tlevel [min level]";
    CLI_registerThisCommandHandler(&cmdHandler);

    loggerInitialized = true;

    /* create a mutex */
    xLogMutex = xSemaphoreCreateMutex();

    return true;
}

const char* logging_level_to_string(tLogLvl given_level)
{
    static const char *fatal      = ANSI_COLOR_RED "fatl";
    static const char *error      = ANSI_COLOR_RED "erro";
    static const char *offnominal = "ofno";
    static const char *notice     = "notc";
    static const char *info       = "info";
    static const char *debug      = "debg";
    static const char *notlvl     = "nalv";

    if(given_level == eLogLvlDebug)
    {
        return debug;
    }
    else if(given_level == eLogLvlInfo)
    {
        return info;
    }
    else if(given_level == eLogLvlNotice)
    {
        return notice;
    }
    else if(given_level == eLogLvlOffNominal)
    {
        return offnominal;
    }
    else if(given_level == eLogLvlError)
    {
        return error;
    }
    else if(given_level == eLogLvlFatal)
    {
        return fatal;
    }
    else
    {
        return notlvl;
    }
}


/*
--------------------------------------------------------------------------------+-
FUNCTION: logCore()

This is the core function that implements the logging feature.
It is called from application code using the macros that are
defined in the eLog.h file.  Note: this uses var args.

--------------------------------------------------------------------------------+-
*/
void logCore(
    const char  *file_name,
    const char  *function_name,
    int          line_number,
    tLogLvl      given_level,
    const char  *format_string,
    ...)
{
    if(loggerInitialized)
    {
        /* Take mutex */
        if( xSemaphoreTake(xLogMutex, ( TickType_t ) 2000) == pdTRUE )
        {
            //get current time
            uint32_t milliseconds = xTaskGetTickCount();
            va_list argptr;
            va_start(argptr, format_string);
            vsprintf(prepBuf, format_string, argptr);
            va_end(argptr);

            xFormatStringAndPutInLineBuffer(given_level,"%d - %s:%s:%d:%s",
                                milliseconds,
                                logging_level_to_string(given_level),
                                function_name,
                                line_number,
                                prepBuf);

            if( enablePrinting == true || given_level >= minLevelToPrint )
            {
                // add end of line
                strcat( lineBuf, "\r\n" ANSI_COLOR_RESET );

                // send out the characters
                UART_sendDataBlocking(LOG, (uint8_t*)lineBuf,  strlen(lineBuf));

                memset(&lineBuf, 0 , MAX_PRINT_CHARS);
                memset(&prepBuf, 0, MAX_PRINT_CHARS);
            }

            /* Return mutex */
            xSemaphoreGive(xLogMutex);
       }
    }
}

//Add newline character to a string and send to the print queue
void xFormatStringAndPutInLineBuffer(tLogLvl given_level, char* msg, ... )
{
        va_list argp;

        va_start( argp, msg );
        vsnprintf( &lineBuf[0], sizeof(lineBuf), msg, argp );
        va_end( argp );
}

bool LOG_enableLogging(bool enable)
{
    enablePrinting = enable;
    return true;
}

static void xCommandHandlerForLogger(int argc, char **argv)
{
    bool enable = false;

    if ((argc == ONE_ARGUMENT) && (0 == strcmp(argv[FIRST_ARG_IDX], "enable")))
    {
        enable = true;
        LOG_enableLogging(enable);
        elogDebug("Logging enabled");
    }
    else if ((argc == ONE_ARGUMENT) && (0 == strcmp(argv[FIRST_ARG_IDX], "disable")))
    {
        enable = false;
        elogDebug("Logging disabled");
        LOG_enableLogging(enable);
    }
    else if ((argc == ONE_ARGUMENT) && (0 == strcmp(argv[FIRST_ARG_IDX], "info")))
    {
        elogInfo("CLI FORCED example INFO log");
    }
    else if ((argc == ONE_ARGUMENT) && (0 == strcmp(argv[FIRST_ARG_IDX], "debug")))
    {
        elogDebug("CLI FORCED example DEBUG log");
    }
    else if ((argc == ONE_ARGUMENT) && (0 == strcmp(argv[FIRST_ARG_IDX], "offnom")))
    {
        elogOffNominal("CLI FORCED example OFF-NOMINAL log");
    }
    else if ((argc == ONE_ARGUMENT) && (0 == strcmp(argv[FIRST_ARG_IDX], "error")))
    {
        elogError("CLI FORCED example ERROR log");
    }
    else if ((argc == ONE_ARGUMENT) && (0 == strcmp(argv[FIRST_ARG_IDX], "fatal")))
    {
        elogFatal("CLI FORCED example FATAL log");
    }
    else if ((argc == TWO_ARGUMENTS) && (0 == strcmp(argv[FIRST_ARG_IDX], "level")))
    {
        elogInfo("Setting new level");

        if ( 0 == strcmp(argv[SECOND_ARG_IDX], "debug"))
        {
            minLevelToPrint = eLogLvlDebug;
        }
        else if ( 0 == strcmp(argv[SECOND_ARG_IDX], "info"))
        {
            minLevelToPrint = eLogLvlInfo;
        }
        else if ( 0 == strcmp(argv[SECOND_ARG_IDX], "offnom"))
        {
            minLevelToPrint = eLogLvlOffNominal;
        }
        else if ( 0 == strcmp(argv[SECOND_ARG_IDX], "notice"))
        {
            minLevelToPrint = eLogLvlNotice;
        }
        else if ( 0 == strcmp(argv[SECOND_ARG_IDX], "error"))
        {
            minLevelToPrint = eLogLvlError;
        }
        else if ( 0 == strcmp(argv[SECOND_ARG_IDX], "fatal"))
        {
            minLevelToPrint = eLogLvlFatal;
        }
        else
        {
            elogError("Invalid log level arg");
        }
    }
    else
    {
        elogDebug("Unknown Command");
    }
}

