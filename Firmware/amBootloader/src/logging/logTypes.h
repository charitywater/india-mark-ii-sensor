/*
================================================================================================#=
Module:   Logger

Description:
    Log levels

Copyright 2021 Twisthink LLC
This code is licensed under Twisthink license.
See Twisthink-SoftwareLicenseAgreement-Auris.docx for details.
================================================================================================#=
*/

#ifndef HANDLERS_LOGTYPES_H_
#define HANDLERS_LOGTYPES_H_

#include <logger.h>
#include "stdbool.h"

/*
================================================================================#=
The Event Log module defines a number of 'logging levels' which are used to
classify each event.  These levels are defined below.

ERROR
    The 'Error' level is used to indicate that the system has detected
    something wrong that needs to be fixed.  Typically, these are
    software design or coding errors, i.e., bugs.
    The occurrence of any error event must be communicated to the developers,
    maintainers, and administrators of the system because the goal is to drive
    the number of 'Error' occurrences to zero; even for systems
    that have deployed to the field.

    The 'ERROR' level is used regardless of whether the origin of the error
    is internal, external, or indeterminate.  So, for example, when a function
    receives a bad parameter value from an external interface, the function
    should log an ERROR even though the ultimate source of the bad value
    is coming from some external source.

    If the developers determine, at some point, that there is no way to correct
    the data originating at the external source, the requirements and behavior
    of the system must be updated so that the system can handle or recover from
    the bad parameter value, at which point, the value is no longer considered "bad",
    and the event can then be logged at a another lower level as appropriate.
================================================================================#=
*/

#undef  CONFIG_LOGGER

#ifdef  CONFIG_LOGGER

#define elogFatal(formatStr, ...) logCore( \
    __FILE__, __FUNCTION__, __LINE__, \
    eLogLvlFatal, formatStr, ##__VA_ARGS__ )

#define elogError(formatStr, ...) logCore( \
    __FILE__, __FUNCTION__, __LINE__, \
    eLogLvlError, formatStr, ##__VA_ARGS__ )

#define elogOffNominal(formatStr, ...) logCore( \
    __FILE__, __FUNCTION__, __LINE__, \
    eLogLvlOffNominal, formatStr, ##__VA_ARGS__ )

#define elogNotice(formatStr, ...) logCore( \
    __FILE__, __FUNCTION__, __LINE__, \
    eLogLvlNotice, formatStr, ##__VA_ARGS__ )

#define elogInfo(formatStr, ...) logCore( \
    __FILE__, __FUNCTION__, __LINE__, \
    eLogLvlInfo, formatStr, ##__VA_ARGS__ )

#define elogDebug(formatStr, ...) logCore( \
    __FILE__, __FUNCTION__, __LINE__, \
    eLogLvlDebug, formatStr, ##__VA_ARGS__ )

#define elog(formatStr, ...) logCore( \
    __FILE__, __FUNCTION__, __LINE__, \
    eLogLvlNone, formatStr, ##__VA_ARGS__ )

#define printf elog

#define elogAssert(x) if(x){ printf("Fatal Error: Exiting program..."); exit(1); }

#else

#define elogAssert(_condition_, formatStr, ...)  do{} while(0)

#define elogFatal(formatStr, ...) logCore( \
    __FILE__, __FUNCTION__, __LINE__, \
    eLogLvlFatal, formatStr, ##__VA_ARGS__ )

#define elogError(formatStr, ...) logCore( \
    __FILE__, __FUNCTION__, __LINE__, \
    eLogLvlError, formatStr, ##__VA_ARGS__ )

#define elogOffNominal(formatStr, ...) logCore( \
    __FILE__, __FUNCTION__, __LINE__, \
    eLogLvlOffNominal, formatStr, ##__VA_ARGS__ )

#define elogNotice(formatStr, ...) logCore( \
    __FILE__, __FUNCTION__, __LINE__, \
    eLogLvlNotice, formatStr, ##__VA_ARGS__ )

#define elogInfo(formatStr, ...) logCore( \
    __FILE__, __FUNCTION__, __LINE__, \
    eLogLvlInfo, formatStr, ##__VA_ARGS__ )

#define elogDebug(formatStr, ...) logCore( \
    __FILE__, __FUNCTION__, __LINE__, \
    eLogLvlDebug, formatStr, ##__VA_ARGS__ )

#endif

#if 0
//  ----------------------------------------------------------------------------#-
/// <ul>
///
/// <li> 'FATAL'
///         is identical to an 'ERROR' (as described above) in all respects
///         with one additional system behavior:  logging a FATAL error will result
///         in a system reset.  Use 'FATAL' when a reset is the only viable method
///         by which the system can recover from the error.  Also use 'FATAL' for
///         design-time errors (a.k.a. assertions).
///
/// <li> 'INFO'
///         is used to log significant state changes and other important events
///         that are part of the normal operation of the system.
///         It may also be used to log the system's identity and other fixed
///         but important characteristics such as the software version.
///         The purpose of an INFO message is to provide visibility
///         into the operation of the system in order to support troubleshooting.
///         The INFO logging statements remain in the production code,
///         although the destination of INFO message may be configured
///         and thus they can be effectively turned off.
///
///         Examples include:  Application state changes, connection/disconnection,
///         requested configuration changes, messages passing through external interfaces.
///
/// <li> 'DEBUG'
///         is identical to 'INFO' (as described above) in all respects
///         with one exception: DEBUG statements are not left in the
///         production version of the code.
///         Use DEBUG to focus in greater detail on the operation of particular
///         areas of the code during development and integration.
///
/// <li> 'OFF-NOMINAL'
///         is identical to 'INFO' (as described above) in all respects
///         except that OFF-NOMINAL is used to log incidents that are
///         (1) unusual, exceptional, or otherwise anomolous but which also
///         (2) must be properly handled and recovered from
///         as part of the required normal operation of the system.
///         The occurance of an OFF-NOMINAL incident is NOT an error.
///         We would expect OFF-NOMINALs to be infrequent,
///         but there is no need to address them as issues unless
///         they occuring more often than we might expect.
///         Examples include loosing a network connection,
///         receiving an invalid input value from a user,
///         and an unexpected power loss.
///
/// <li> 'None'
///         Equates to a regular printf().
///
/// </ul>
///
/// <b>Remote Logging</b><br>
/// The incident log module provides the ability to send log messages over the network
/// to a remote server where they can be collected and analyzed.
/// This capability is available in production as well as development.
/// For production and customer test deployments, a device may be configured at run time
/// to send any message at or above a particular logging level to the remote server.
/// This configuration is restricted to log levels INFO thru FATAL because all DEBUG logging
/// will be compiled out for production deployments.
///
/// The incident log module provides the ability to "compile out" all debug tracing.
/// There is a build option that causes all calls to the logDebug() macro to be
/// eliminated from the build.
//  ----------------------------------------------------------------------------#-
#endif


#endif /* HANDLERS_LOGTYPES_H_ */
