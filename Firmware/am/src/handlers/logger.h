/**************************************************************************************************
* \file     logger.h
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

#ifndef HANDLERS_LOGGER_H_
#define HANDLERS_LOGGER_H_

#include <stdint.h>
#include <stdbool.h>
#include <ctype.h>

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
