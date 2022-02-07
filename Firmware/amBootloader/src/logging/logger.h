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

#ifndef HANDLERS_LOGGER_H_
#define HANDLERS_LOGGER_H_

#include <stdint.h>
#include <stdbool.h>
#include <ctype.h>

//Use these to print in color
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

// -------------------------------------------------------------+-
// Define the logging "level" associated with each log message.
// The numerical values are ordered from lowest to highest severity.
// -------------------------------------------------------------+-
typedef enum tLogLvl
{
    eLogLvlDebug      = 0,
    eLogLvlInfo       = 1,
    eLogLvlNotice     = 2,
    eLogLvlOffNominal = 3,
    eLogLvlError      = 4,
    eLogLvlFatal      = 5,
    eLogLvlInvalid    = 6,

} tLogLvl;

/*
--------------------------------------------------------------------------------+-
This is the core function that implements the logging feature.
It is called from application code using the macros defined in eLog.h
--------------------------------------------------------------------------------+-
*/
extern void logCore(
    const char  *fileName,
    const char  *functionName,
    int          lineNumber,
    tLogLvl    loggingLevel,
    const char  *formatStr,
    ...
);


/*
--------------------------------------------------------------------------------+-
Initialize the logging module. Registers a command handler for the CLI command 'log'
And configures the UART and callback functions.
--------------------------------------------------------------------------------+-
*/
extern bool LOG_initializeLogger();

/*
--------------------------------------------------------------------------------+-
Enable or disable printing. Default is disabled.
--------------------------------------------------------------------------------+-
*/
extern bool LOG_enableLogging(bool enable);


#endif /* HANDLERS_LOGGER_H_ */
