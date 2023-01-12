/**************************************************************************************************
* \file     HW_RTC.c
* \brief    Real-time clock functionality for the M41T62Q6F part
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
* \date     01/29/2021
* \author   Twisthink
*
***************************************************************************************************/

#include <msp430.h>                      // Generic MSP430 Device Include
#include "driverlib.h"                   // MSPWare Driver Library
#include "HW_RTC.h"
#include <stdint.h>
#include "HW_TERM.h"
#include "uC_I2C.h"
#include "HW.h"
#include <stdio.h>
#include "uC_TIME.h"
#include <time.h>
#include "commandLineDriver.h"
#include "APP.h"
#include "am-ssm-spi-protocol.h"
#include "APP_NVM.h"

// Register addresses and indices in the buffer if you attempt to write all of them in sequence.
// You can start with a register address followed by all of the contiguous data you wish to write.
// E.g. to write them a group, place address 0x00 (sub second register address) followed by the subsecond
// values, followed by the second values, folloed by minutes, etc., as laid out in the device register map.
#define SET_TIME_BUF_SIZE           9 // 1 byte for start register addr and 8 bytes of data.
#define SET_ALM_BUF_SIZE            6 // 1 byte for state byte register and 5 bytes of data. (Subseconds, day of week, year not used for alarms.)

#define START_REG_IDX               0

#define SUB_SEC_REG_ADDR            0x00
#define SUB_SEC_IDX                 (START_REG_IDX + 1) // 1
#define SUB_SEC_100MS_REG_MASK      0xF0
#define SUB_SEC_100MS_CMP_MASK      0x0F
#define SUB_SEC_10MS_REG_MASK       0x0F

#define SEC_REG_ADDR                0x01
#define SEC_IDX                     (SUB_SEC_IDX + 1) // 2
#define SEC_10S_REG_MASK            0x70
#define SEC_10S_CMP_MASK            0x07
#define SEC_1S_REG_MASK             0x0F

#define MIN_REG_ADDR                0x02
#define MIN_IDX                     (SEC_IDX + 1) // 3
#define MIN_10M_REG_MASK            0x70
#define MIN_10M_CMP_MASK            0x07
#define MIN_1M_REG_MASK             0x0F

#define HOUR_REG_ADDR               0x03
#define HOUR_IDX                    (MIN_IDX + 1) // 4
#define HOUR_10H_REG_MASK           0x30
#define HOUR_10H_CMP_MASK           0x03
#define HOUR_1H_REG_MASK            0x0F

#define DAY_REG_ADDR                0x04
#define DAY_IDX                     (HOUR_IDX + 1) // 5
#define DAY_1D_REG_MASK             0x07

#define DATE_REG_ADDR               0x05
#define DATE_IDX                    (DAY_IDX + 1) // 6
#define DATE_10D_REG_MASK           0x30
#define DATE_10D_CMP_MASK           0x03
#define DATE_1D_REG_MASK            0x0F

#define MONTH_REG_ADDR              0x06
#define MONTH_IDX                   (DATE_IDX + 1) // 7
#define MONTH_10M_REG_MASK          0x10
#define MONTH_10M_CMP_MASK          0x01
#define MONTH_1M_REG_MASK           0x0F

#define CENT_REG_ADDR               0x06           // Century bits share register with month.
#define CENT_CMP_MASK               0x03
#define CENT_IDX                    (MONTH_IDX)    // 7

#define YEAR_REG_ADDR               0x07
#define YEAR_IDX                    (MONTH_IDX + 1) // 8
#define YEAR_10Y_REG_MASK           0xF0
#define YEAR_10Y_CMP_MASK           0x0F
#define YEAR_1Y_REG_MASK            0x0F

// Alarm registers.
#define ALM_MONTH_REG_ADDR              0x0A
#define ALM_MONTH_IDX                   (START_REG_IDX + 1) // 1
#define ALM_MONTH_10M_REG_MASK          0x10
#define ALM_MONTH_10M_CMP_MASK          0x01
#define ALM_MONTH_1M_REG_MASK           0x0F
#define ALM_MONTH_AFE_MASK              0x80                // Alarm flag enable is in the month reg.  This will cause irq assertion when alarm accurs.

#define ALM_DATE_REG_ADDR               0x0B
#define ALM_DATE_IDX                    (ALM_MONTH_IDX + 1) // 2
#define ALM_DATE_10D_REG_MASK           0x30
#define ALM_DATE_10D_CMP_MASK           0x03
#define ALM_DATE_1D_REG_MASK            0x0F
#define ALM_DATE_RPT4                   (BIT_7)            // Alarm repeat bit 4
#define ALM_DATE_RPT5                   (BIT_6)            // Alarm repeat bit 5

#define ALM_HOUR_REG_ADDR               0x0C
#define ALM_HOUR_IDX                    (ALM_DATE_IDX + 1) // 3
#define ALM_HOUR_10H_REG_MASK           0x30
#define ALM_HOUR_10H_CMP_MASK           0x03
#define ALM_HOUR_1H_REG_MASK            0x0F
#define ALM_HOUR_RPT3                   (BIT_7)            // Alarm repeat bit 3

#define ALM_MIN_REG_ADDR                0x0D
#define ALM_MIN_IDX                     (ALM_HOUR_IDX + 1) // 4
#define ALM_MIN_10M_REG_MASK            0x70
#define ALM_MIN_10M_CMP_MASK            0x07
#define ALM_MIN_1M_REG_MASK             0x0F
#define ALM_MIN_RPT2                    (BIT_7)            // Alarm repeat bit 2

#define ALM_SEC_REG_ADDR                0x0E
#define ALM_SEC_IDX                     (ALM_MIN_IDX + 1)  // 2
#define ALM_SEC_10S_REG_MASK            0x70
#define ALM_SEC_10S_CMP_MASK            0x07
#define ALM_SEC_1S_REG_MASK             0x0F
#define ALM_SEC_RPT1                    (BIT_7)            // Alarm repeat bit 1

#define FLAGS_REG_ADDR                  0x0F

// Century bits indicating which century.
#define CB_2300_MASK                    (BIT_7 | BIT_6)
#define CB_2200_MASK                    (BIT_7)
#define CB_2100_MASK                    (BIT_6)
#define CB_2000_MASK                    (0)

#define BYTES_PER_DATE                  60

#define MAX_VALUE_MINUTES_REG           59
#define MAX_VALUE_SECONDS_REG           59
#define COMM_RETRIES                    2

#pragma PERSISTENT(Day_Strings)
static const char * Day_Strings[] = {
    "Sunday",
    "Monday",
    "Tuesday",
    "Wednesday",
    "Thursday",
    "Friday",
    "Saturday"
};

#pragma PERSISTENT(Month_Strings)
static const char * Month_Strings[] = {
    "January",
    "February",
    "March",
    "April",
    "May",
    "June",
    "July",
    "August",
    "September",
    "October",
    "November",
    "December"
};

void HW_RTC_Init(void);
void HW_RTC_Monitor(void);
bool HW_RTC_SetTime(uint8_t _10_ms,
                    uint8_t seconds,
                    uint8_t minutes,
                    uint8_t hours,
                    HW_RTC_DAY_OF_WEEK_T day_of_week,
                    uint8_t date,
                    HW_RTC_MONTH_T month,
                    uint16_t year);
void HW_RTC_ReportTime(void);
bool HW_RTC_GetTime(uint8_t * _10_ms,
                    uint8_t * seconds,
                    uint8_t * minutes,
                    uint8_t * hours,
                    HW_RTC_DAY_OF_WEEK_T * day_of_week,
                    uint8_t * date,
                    HW_RTC_MONTH_T * month,
                    uint16_t * year);
void HW_RTC_GetHour(uint8_t * hour);
void HW_RTC_GetSecToNextHour(uint16_t * secondsToNextHour);
uint32_t HW_RTC_GetEpochTime(void);
bool HW_RTC_SetTimeEpoch(uint32_t epoch_time);
bool HW_RTC_CheckValidTime(void);


static bool xVerifyTime(uint8_t _10_ms,
                       uint8_t seconds,
                       uint8_t minutes,
                       uint8_t hours,
                       HW_RTC_DAY_OF_WEEK_T day_of_week,
                       uint8_t date,
                       HW_RTC_MONTH_T month,
                       uint16_t year);
static uint8_t xGetCenturyMask(uint16_t year);
static const char * xLookupDayOfWeek(HW_RTC_DAY_OF_WEEK_T day_of_week);
static const char * xLookupMonth(HW_RTC_MONTH_T month);
static void xClearOscillatorError(void);

void HW_RTC_Init(void)
{
    //S1 IFG cleared
    GPIO_clearInterrupt(GPIO_PORT_P1, GPIO_PIN7);

    //S1 interrupt enabled
    GPIO_enableInterrupt(GPIO_PORT_P1, GPIO_PIN7);

    //S1 Hi/Lo edge
    GPIO_selectInterruptEdge(GPIO_PORT_P1, GPIO_PIN7, GPIO_HIGH_TO_LOW_TRANSITION);

    //init library time zone to UTC
    _tz.timezone = 0;
}

bool HW_RTC_CheckValidTime(void)
{
    uint8_t _10_ms;
    uint8_t seconds;
    uint8_t minutes;
    uint8_t hours;
    HW_RTC_DAY_OF_WEEK_T day_of_week;
    uint8_t date;
    HW_RTC_MONTH_T month;
    uint16_t year;

    return HW_RTC_GetTime(&_10_ms, &seconds, &minutes, &hours, &day_of_week,  &date,  &month,  &year);
}

// Periodic function for the RTC
void HW_RTC_Monitor(void)
{
    static bool osc_err_cleared = false;

    // Data sheet says we should be able to clear the OE flag four seconds after power up and, if not, there is an issue.
    // Waiting 5 to give a little extra time.
    if ((osc_err_cleared == false ) && (uC_TIME_GetRuntimeSeconds() >= 5))
    {
        xClearOscillatorError();
        osc_err_cleared = true;
    }
}

// WARNING: "EPOCH Time" is not well-defined but it generally is a number representing the
// the number of seconds passed between the current time and some specific time in the past.
// Generally this is either January 1, 1970 or January 1, 1900. The former is considered
// the more-accepted time.  However, for TI, the latter is the default.  In order to
// use January 1, 1970, you mused define __TI_TIME_USES_64.  This is all explained here:
// http://processors.wiki.ti.com/index.php/Time_and_clock_RTS_Functions.
//
// Also, be advised that some websites out there seem to come up with incorrect results.
// https://www.unixtimestamp.com/index.php appears to be correct.
//
// Example:
// November 13, 2019 @ 12:48:30
// t.tm_year = 119;  // This is year-1900
// t.tm_mon = 10; // 0-based month
// t.tm_mday = 13;
// t.tm_hour = 12;
// t.tm_min = 48;
// t.tm_sec = 30;

time_t HW_RTC_ConvertTo32BitEpoch( uint8_t seconds,
                                     uint8_t minutes,
                                     uint8_t hours,
                                     uint8_t date,
                                     uint8_t month,
                                     uint16_t year)
{
    struct tm t = {0};  // Initalize to all 0's
    time_t timeSinceEpoch = 0;

    _tz.timezone = 0;   // Set timezone to UTC

    t.tm_year = (year - 1900);  // This is year-1900, so 112 = 2012
    t.tm_mon = (month - 1); // zero-based
    t.tm_mday = date;
    t.tm_hour = hours;
    t.tm_min = minutes;
    t.tm_sec = seconds;

    /* NOTE THAT THIS TAKES 35 ms TO RUN */
    timeSinceEpoch = mktime(&t);

    return timeSinceEpoch;
}

bool HW_RTC_SetTimeEpoch(uint32_t epoch_time)
{
    time_t t = (time_t) epoch_time;
    struct tm *time = localtime(&t);

    HW_TERM_Print("HW_RTC: Setting time ");

    return HW_RTC_SetTime(0,
                          time->tm_sec,
                          time->tm_min,
                          time->tm_hour,
                          (HW_RTC_DAY_OF_WEEK_T)(time->tm_wday + 1), // tm struct uses 0-11 for months.  RTC expects 1-12
                          time->tm_mday,
                          (HW_RTC_MONTH_T)(time->tm_mon + 1), // tm struct uses 0-11 for months.  RTC expects 1-12
                          (time->tm_year + 1900)); // tm struct uses years since 1900.  Adding it back in four our interface.
}


bool HW_RTC_GetTime(uint8_t *  p_10_ms,
                    uint8_t * p_seconds,
                    uint8_t * p_minutes,
                    uint8_t * p_hours,
                    HW_RTC_DAY_OF_WEEK_T * p_day_of_week,
                    uint8_t * p_date,
                    HW_RTC_MONTH_T * p_month,
                    uint16_t * p_year)
{
    uint8_t century;
    bool validTime = false;

    // todo: change this to multi-byte read to get snapshot.  otherwise the data can be inconsistent.
    * p_10_ms = uC_I2C_ReadRegSingle(HW_RTC_SLAVE_ADDR, SUB_SEC_REG_ADDR, false);
    *p_seconds = uC_I2C_ReadRegSingle(HW_RTC_SLAVE_ADDR, SEC_REG_ADDR, false);
    *p_minutes = uC_I2C_ReadRegSingle(HW_RTC_SLAVE_ADDR, MIN_REG_ADDR, false);
    *p_hours = uC_I2C_ReadRegSingle(HW_RTC_SLAVE_ADDR, HOUR_REG_ADDR, false);
    *p_day_of_week = (HW_RTC_DAY_OF_WEEK_T)uC_I2C_ReadRegSingle(HW_RTC_SLAVE_ADDR, DAY_REG_ADDR, false);
    *p_month = (HW_RTC_MONTH_T)uC_I2C_ReadRegSingle(HW_RTC_SLAVE_ADDR, MONTH_REG_ADDR, false);
    *p_date = uC_I2C_ReadRegSingle(HW_RTC_SLAVE_ADDR, DATE_REG_ADDR, false);
    *p_year = uC_I2C_ReadRegSingle(HW_RTC_SLAVE_ADDR, YEAR_REG_ADDR, false);
    century = uC_I2C_ReadRegSingle(HW_RTC_SLAVE_ADDR, CENT_REG_ADDR, false);

    // Now convert the values from register storage format to something we can display.
    * p_10_ms =  ( (((* p_10_ms   >> 4) & SUB_SEC_100MS_CMP_MASK) * 10)  + (* p_10_ms  & SUB_SEC_10MS_REG_MASK) );
    *p_seconds = ( (((*p_seconds  >> 4) & SEC_10S_CMP_MASK)       * 10)  + (*p_seconds & SEC_1S_REG_MASK) );
    *p_minutes = ( (((*p_minutes  >> 4) & MIN_10M_CMP_MASK)       * 10)  + (*p_minutes & MIN_1M_REG_MASK) );
    *p_hours =   ( (((*p_hours    >> 4) & HOUR_10H_CMP_MASK)      * 10)  + (*p_hours   & HOUR_1H_REG_MASK) );
    *p_day_of_week &= DAY_1D_REG_MASK;
    *p_month =   (HW_RTC_MONTH_T)( (((*p_month    >> 4) & MONTH_10M_CMP_MASK)     * 10)  + (*p_month   & MONTH_1M_REG_MASK) );
    *p_date =    ( (((*p_date     >> 4) & DATE_10D_CMP_MASK)      * 10)  + (*p_date    & DATE_1D_REG_MASK) );
    *p_year =    ( (((*p_year     >> 4) & YEAR_10Y_CMP_MASK)      * 10)  + (*p_year    & YEAR_1Y_REG_MASK) );
    *p_year +=   ( (((century  >> 6) & CENT_CMP_MASK)     * 100) + 2000);

    if (xVerifyTime(*p_10_ms, *p_seconds, *p_minutes, *p_hours, *p_day_of_week, *p_date, *p_month, *p_year) != true)
    {
        HW_TERM_Print("HW_RTC: Bad time read from device.\n");

        *p_10_ms = 0;
        *p_seconds = 0;
        *p_minutes = 0;
        *p_hours = 0;
        *p_day_of_week = (HW_RTC_DAY_OF_WEEK_T)0;
        *p_month = (HW_RTC_MONTH_T)0;
        *p_date = 0;
        *p_year = 0;
        *p_year += 0;
    }
    else
    {
        validTime = true;
    }

    return validTime;
}

void HW_RTC_GetHour(uint8_t * hour)
{
    *hour = uC_I2C_ReadRegSingle(HW_RTC_SLAVE_ADDR, HOUR_REG_ADDR, false);

    // Now convert the values from register storage format to something we can display.
    *hour = ( (((*hour    >> 4) & HOUR_10H_CMP_MASK)      * 10)  + (*hour   & HOUR_1H_REG_MASK) );
}

uint32_t HW_RTC_GetSecondsSinceMidnight(void)
{
    uint8_t hour = 0;
    uint8_t mins = 0;
    uint8_t secs = 0;
    uint32_t secondsSinceMidnight = 0;

    hour = uC_I2C_ReadRegSingle(HW_RTC_SLAVE_ADDR, HOUR_REG_ADDR, false);
    secs = uC_I2C_ReadRegSingle(HW_RTC_SLAVE_ADDR, SEC_REG_ADDR, false);
    mins = uC_I2C_ReadRegSingle(HW_RTC_SLAVE_ADDR, MIN_REG_ADDR, false);

    // Now convert the values from register storage to actual time
    hour = ( (((hour >> 4) & HOUR_10H_CMP_MASK) * 10) + (hour & HOUR_1H_REG_MASK) );
    secs = ( (((secs >> 4) & SEC_10S_CMP_MASK) * 10) + (secs & SEC_1S_REG_MASK) );
    mins = ( (((mins >> 4) & MIN_10M_CMP_MASK) * 10) + (mins & MIN_1M_REG_MASK) );

    secondsSinceMidnight = (hour * SEC_PER_HOUR) + (mins * SEC_PER_MIN) + secs;

    return secondsSinceMidnight;
}

void HW_RTC_GetSecToNextHour(uint16_t * secondsToNextHour)
{
    uint8_t seconds;
    uint8_t secondsToNextMin;
    uint8_t minutesToNextHour;
    uint8_t minutes;
    uint16_t totalSeconds;

    seconds = uC_I2C_ReadRegSingle(HW_RTC_SLAVE_ADDR, SEC_REG_ADDR, false);
    seconds = ( (((seconds  >> 4) & SEC_10S_CMP_MASK) * 10)  + (seconds & SEC_1S_REG_MASK) );

    minutes = uC_I2C_ReadRegSingle(HW_RTC_SLAVE_ADDR, MIN_REG_ADDR, false);
    minutes = ( (((minutes  >> 4) & MIN_10M_CMP_MASK) * 10)  + (minutes & MIN_1M_REG_MASK) );

    secondsToNextMin = MAX_VALUE_SECONDS_REG - seconds;
    minutesToNextHour = MAX_VALUE_MINUTES_REG - minutes;

    totalSeconds = (uint16_t)secondsToNextMin + ( (uint16_t)minutesToNextHour * SEC_PER_MIN );

    //error checking:
    if ( totalSeconds > SEC_PER_HOUR )
    {
        totalSeconds = SEC_PER_HOUR;
    }

    *secondsToNextHour = totalSeconds;
}

uint32_t HW_RTC_GetEpochTime(void)
{
    uint8_t _10_ms;
    uint8_t seconds;
    uint8_t minutes;
    uint8_t hours;
    HW_RTC_DAY_OF_WEEK_T day_of_week;
    uint8_t date;
    HW_RTC_MONTH_T month;
    uint16_t year;
    uint32_t epoch = 0;

    if ( true == HW_RTC_GetTime(&_10_ms, &seconds, &minutes, &hours, &day_of_week,  &date,  &month,  &year) )
    {
        epoch = (uint32_t) HW_RTC_ConvertTo32BitEpoch(seconds, minutes, hours, date, month, year);
    }
    else
    {
        epoch = 0x00000000;
    }

    return epoch;
}
// Convert the time from the register format to a printable format and report.
void HW_RTC_ReportTime(void)
{
    uint8_t _10_ms;
    uint8_t seconds;
    uint8_t minutes;
    uint8_t hours;
    HW_RTC_DAY_OF_WEEK_T day_of_week;
    uint8_t date;
    HW_RTC_MONTH_T month;
    uint16_t year;
    uint8_t str[80];
    uint32_t epoch = 0;

    HW_RTC_GetTime(&_10_ms, &seconds, &minutes, &hours, &day_of_week,  &date,  &month,  &year);

    sprintf((char *)str, "\nHW_RTC Time: %s, %s %u, %u - %02u:%02u:%02u.%02u\n",
                xLookupDayOfWeek((HW_RTC_DAY_OF_WEEK_T)day_of_week),
                xLookupMonth((HW_RTC_MONTH_T)month),
                date,
                year,
                hours,
                minutes,
                seconds,
                _10_ms);

    HW_TERM_Print(str);

    epoch = HW_RTC_ConvertTo32BitEpoch( seconds, minutes, hours, date, month, year);
    sprintf((char *)str, "HW_RTC Epoch: 0x%lX\n", epoch);

    HW_TERM_Print(str);
}

// Convert to the provided values to their register format and write them.
bool HW_RTC_SetTime(uint8_t _10_ms,
                    uint8_t seconds,
                    uint8_t minutes,
                    uint8_t hours,
                    HW_RTC_DAY_OF_WEEK_T day_of_week,
                    uint8_t date,
                    HW_RTC_MONTH_T month,
                    uint16_t year)
{
    uint8_t wbuf[SET_TIME_BUF_SIZE] = {};
    bool time_good = false;
    uint8_t retry = COMM_RETRIES;

    if (xVerifyTime(_10_ms, seconds, minutes, hours, day_of_week, date, month, year) == true)
    {
        wbuf[START_REG_IDX] = SUB_SEC_REG_ADDR;                        // Begin writing at register 0, the subsecond register.

        wbuf[SUB_SEC_IDX] = 0x00;                                      // Set subsecs to 0

        wbuf[SEC_IDX] = (((seconds/10u) << 4) & SEC_10S_REG_MASK);         // Put the 10 s in the upper 4 bits.
        wbuf[SEC_IDX] += ((seconds%10u) & SEC_1S_REG_MASK);                // Put the 1 s in the lower 4 bits.

        wbuf[MIN_IDX] = (((minutes/10u) << 4) & MIN_10M_REG_MASK);         // Put the 10 min in the upper 4 bits.
        wbuf[MIN_IDX] += ((minutes%10u) & MIN_1M_REG_MASK);                // Put the 1 min in the lower 4 bits.

        wbuf[HOUR_IDX] = (((hours/10u) << 4) & HOUR_10H_REG_MASK);         // Put the 10 h in bits 4 and 5.
        wbuf[HOUR_IDX] += ((hours%10u) & HOUR_1H_REG_MASK);                // Put the 1 h in the lower 4 bits.

        wbuf[DAY_IDX] += (day_of_week & DAY_1D_REG_MASK);                  // Put the day of week in the lower 3 bits.

        wbuf[DATE_IDX] = (((date/10u) << 4) & DATE_10D_REG_MASK);          // Put the 10 day in bits 4 and 5.
        wbuf[DATE_IDX] += ((date%10u) & DATE_1D_REG_MASK);                 // Put the 1 day in the lower 4 bits.

        wbuf[MONTH_IDX] = (((month/10u) << 4) & MONTH_10M_REG_MASK);       // Put the 10 month in bit 4.
        wbuf[MONTH_IDX] += ((month%10u) & MONTH_1M_REG_MASK);              // Put the 1 month in the lower 4 bits.

        wbuf[CENT_IDX] |= xGetCenturyMask(year);                       // Century bits share reg with month so OR them in.

        year %= 100; // Century is stored in seperate bits so just save the 10s and 1s.
        wbuf[YEAR_IDX] = (((year/10u) << 4) & YEAR_10Y_REG_MASK);         // Put the 10 month in bit 4.
        wbuf[YEAR_IDX] += ((year%10u) & YEAR_1Y_REG_MASK);                // Put the 1 month in the lower 4 bits.

        while ( retry > 0 && time_good == false )
        {
            if (uC_I2C_WriteMulti(HW_RTC_SLAVE_ADDR, wbuf, SET_TIME_BUF_SIZE, false) == true)    // Send it as a multi-byte transmission.
            {
                time_good = true;
                HW_TERM_Print("HW_RTC: Time set\n");
            }
            else
            {
                HW_TERM_Print("HW_RTC: Error setting time \n");
            }
        }

        if ( time_good == false )
        {
            HW_TERM_Print("HW_RTC: ERROR.  Could not set time.\n");
            APP_indicateError(RTC_COMM_ERROR);
        }
    }

    return time_good;
}

static const char * xLookupDayOfWeek(HW_RTC_DAY_OF_WEEK_T day_of_week)
{
    if((day_of_week >= HW_RTC_SUN) && (day_of_week <= HW_RTC_SAT))
    {
        return Day_Strings[(day_of_week-1)];
    }
    else
    {
        return "Unknown day";
    }
}

static const char * xLookupMonth(HW_RTC_MONTH_T month)
{
    if((month >= HW_RTC_JAN) && (month <= HW_RTC_DEC))
    {
        return Month_Strings[(month-1)];
    }
    else
    {
        return "Unknown month";
    }
}

static uint8_t xGetCenturyMask(uint16_t year)
{
    // See table 8 in the M41T62 data sheet for examples.
    // The 2 century bits can be used however we want but we are using them according to the suggestion in the data sheet.
    // 2000 = 00
    // 2100 = 01
    // 2200 = 10
    // 2300 = 11
    if (year >= 2300)
    {
        return CB_2300_MASK;
    }
    else if (year >= 2200)
    {
        return CB_2200_MASK;
    }
    else if (year >= 2100)
    {
        return CB_2100_MASK;
    }
    else return CB_2000_MASK;
}

// Write a value of zero to the oscillator error flag since it will be set to 1 on initial power up.
static void xClearOscillatorError(void)
{
    uint8_t wbuf[2] = {FLAGS_REG_ADDR, 0};

    uC_I2C_WriteMulti(HW_RTC_SLAVE_ADDR, wbuf, 2, false);         // Send it as a multi-byte transmission.
}

// Confirm range of all values we want to use for setting time.
static bool xVerifyTime(uint8_t _10_ms,
                       uint8_t seconds,
                       uint8_t minutes,
                       uint8_t hours,
                       HW_RTC_DAY_OF_WEEK_T day_of_week,
                       uint8_t date,
                       HW_RTC_MONTH_T month,
                       uint16_t year)
{
    if (_10_ms > 99) return false;
    if (seconds > 59) return false;
    if (minutes > 59) return false;
    if (hours > 23) return false;
    if ((day_of_week < HW_RTC_SUN) || (day_of_week > HW_RTC_SAT)) return false;
    if ((date < 1) || (date > 31)) return false;
    if ((month < 1) || (month > 12)) return false;
    if ((year < 2019 ) || (year > 2399)) return false;

    return true;
}
