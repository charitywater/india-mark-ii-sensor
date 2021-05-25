/*
================================================================================================#=
Module:   Logger

Description:
    Log data at various levels for debugging and documenting errors

Copyright 2021 Twisthink LLC
This code is licensed under Twisthink license.
See Twisthink-SoftwareLicenseAgreement-Auris.docx for details.
================================================================================================#=
*/

#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "stdbool.h"
#include <ctype.h>
#include <logger.h>

#include <stm32l4xx_hal.h>

#include <logTypes.h>
#include <uart.h>

static bool loggerInitialized = false;
static char prepBuf[255];
static char lineBuf[255];

//print enabled is OFF by default
//can use the CLI to enable it, which will allow
//all levels of logging to be printed
static bool enablePrinting = false;

// Local functions
static void xFormatStringAndPutInLineBuffer(tLogLvl given_level, char* msg, ... );

bool LOG_initializeLogger()
{
    //init uart
    UART_initPeripherals();

    loggerInitialized = true;

    return true;
}

const char* logging_level_to_string(tLogLvl given_level)
{
    static const char *fatal      = ANSI_COLOR_RED "fatl";
    static const char *error      = ANSI_COLOR_RED "erro";
    static const char *offnominal = ANSI_COLOR_YELLOW "ofno";
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
        //get current time
        uint32_t milliseconds = HAL_GetTick();

        va_list argptr;
        va_start(argptr, format_string);
        vsprintf(prepBuf, format_string, argptr);
        va_end(argptr);

        xFormatStringAndPutInLineBuffer(given_level,"%d - %s:%s:%d:%s",
                            milliseconds,
                            logging_level_to_string(given_level),
                            function_name,
                            line_number,
                            prepBuf
                        );

        if(enablePrinting == true || given_level >= eLogLvlInfo)
        {
            // add end of line
            strcat( lineBuf, "\r\n" ANSI_COLOR_RESET );

            // send out the characters
            UART_sendDataBlocking(LOG, (uint8_t*)lineBuf,  strlen(lineBuf));

            memset(&lineBuf, 0 , 255);
            memset(&prepBuf, 0, 255);
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

