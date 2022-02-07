/**************************************************************************************************
* \file     APP_NVM_Cfg_Shared.h
* \brief    Shared data types between the AM and SSM
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

// Configuration items for a specific NVM application.
//
// NOTES ON USAGE:
//
// - Section_Map[] must be ordered from lowest address to highest.
//
// - Section_Map[] must leave APP_NVM_MAGIC_VALUE_LEN bytes at the end of memory for the magic number.
//
// - If you add a section type, add a APP_NVM_SECT_TYPE_<name> #define for it with a unique identifier.
//   That type must also be added to APP_NVM_SECTION_DATA_T to make sure that the r/w buffers are
//   sufficient.
//
// - All section types must be packed and end with a checksum byte.

#ifndef APP_NVM_CFG_SHARED_H
#define APP_NVM_CFG_SHARED_H

#include <stdint.h>

//2 months - 7 days/week * 8 weeks
#define MAX_SENSOR_DATA_LOGS    56

#define APP_NVM_SAMPLES_PER_DAY 24
#define MAX_LOG_SIZE            100

#ifdef ENGINEERING_DATA
typedef struct __attribute__ ((packed)) APP_NVM_SENSOR_DATA_T
{
    uint32_t        timestamp;
    uint16_t        pads[8]; //APP_WTR_NUM_PADS];
    uint16_t        temp_c_raw;
    uint16_t        humidity_raw;
    int16_t         magnetometerX;
    int16_t         magnetometerY;
    int16_t         magnetometerZ;
    int16_t         tempLsb;
    uint8_t         magStatBitFlags;
    uint8_t         checksum;
}APP_NVM_SENSOR_DATA_T;

#else

typedef struct __attribute__ ((packed)) APP_NVM_SENSOR_DATA_T
{
    uint32_t        timestamp;                                      // 0 - 3 Timestamp of the first sample of the day.
    uint16_t        litersPerHour[APP_NVM_SAMPLES_PER_DAY];         // 4 - 51 Sum of pumped liters for each hour of the day
    uint8_t         tempPerHour[APP_NVM_SAMPLES_PER_DAY];           // 52 - 75 Temp C = ((temp_c_raw/65535)*165)-40
    uint8_t         humidityPerHour[APP_NVM_SAMPLES_PER_DAY];       // 76 - 99 Humidity % = ((humidity_raw)/65535)*100
    uint16_t        strokesPerHour[APP_NVM_SAMPLES_PER_DAY];        // 100 - 147 Sum of stroke counts for each hour of the day
    uint8_t         strokeHeightPerHour[APP_NVM_SAMPLES_PER_DAY];   // 148 - 172 Avg. stroke height for each hour of the day
    uint16_t        dailyLiters;                                    // 173 - 174 Daily liter sum
    uint16_t        avgLiters;                                      // 175 - 176 Average liters over the past 4 weeks for this day of the week.
    uint32_t        totalLiters;                                    // 177 - 180 Total liters since activation
    bool            breakdown;                                      // 180 - 180 Boolean indicating if pump is broken
    uint16_t        pumpCapacity;                                   // 181 - 182 Pump capacity recorded for this day
    uint16_t        batteryVoltage;                                 // 183 - 184 current battery voltage
    uint16_t        powerRemaining;                                 // 185 - 186 percentage
    uint8_t         state;                                          // 187 - 187 State: 1 = Activated 2 = Deactivated 3 = Fault
    bool            magnetDetected;                                 // 188 - 188 True = detected False = not detected
    uint32_t        errorBits;                                      // 189 - 192 Each bit indicates a separate error
    uint32_t        unexpectedResets;                               // 193 - 196 Number of unexpected resets over the life of the device
    uint32_t        timestampOfLastReset;                           // 197 - 200 Timestamp of the last reset
    uint32_t        activatedDate;                                  // 201 - 204 GMT date/time of activation (in seconds)
    uint16_t        pumpUsage;                                      // 205 - 206 Quality metric for how long the pump was in use during pump health calc
    uint16_t        dryStrokes;                                     // 207 - 208 Dry stroke count before water flows
    uint16_t        dryStrokeHeight;                                // 209 - 210 Avg stroke height of the dry strokes
    uint16_t        pumpUnusedTime;                                 // 211 - 212 Time the pump was not in use prior to dry stroke detection
    uint8_t         checksum;                                       // 213 - 214 checksum, total size  = 214 bytes
}APP_NVM_SENSOR_DATA_T;

#ifdef AM_BUILD
typedef struct __attribute__ ((packed)) APP_NVM_SENSOR_DATA_WITH_HEADER
{
    uint32_t        productId;              // IM2 Product Identifier, Always set to 4
    uint32_t        timestamp;              // Current GMT date/time (in seconds)
    uint32_t        msgNumber;              // Unique message ID, Start at 1 and increment
    uint32_t        fwVersionMaj;           // x of x.y.z version number
    uint32_t        fwVersionMinor;         // y of x.y.z version number
    uint32_t        fwVersionBuild;         // z of x.y.z version number
    uint16_t        batteryVoltage;         // voltage in mV - taken 15 seconds into modem being turned on
    uint16_t        powerRemaining;         // Battery Percentage Remaining
    uint8_t         state;                  // 1 = Activated 2 = Deactivated 3 = Fault
    uint32_t        activatedDate;          // GMT date/time of activation (in seconds)
    bool            magnetDetected;         // True = detected False = not detected
    uint32_t        errorBits;              // Each bit indicates a separate error
    uint32_t        numSSMResets;           // The number of unexpected SSM resets
    uint32_t        lastSSMResetDate;       // Date and time of last unexpected SSM reset
    uint32_t        numAMResets;            // The number of unexpected AM resets
    uint32_t        lastAMResetDate;        // Date and time of last unexpected AM reset
    char            debugLog[MAX_LOG_SIZE]; // Debug log string Optional

    /* header ends, sensor data begins: */

    uint16_t        litersPerHour[APP_NVM_SAMPLES_PER_DAY];
    uint8_t         tempPerHour[APP_NVM_SAMPLES_PER_DAY];
    uint8_t         humidityPerHour[APP_NVM_SAMPLES_PER_DAY];
    uint16_t        strokesPerHour[APP_NVM_SAMPLES_PER_DAY];
    uint8_t         strokeHeightPerHour[APP_NVM_SAMPLES_PER_DAY];
    uint16_t        dailyLiters;
    uint16_t        avgLiters;
    uint32_t        totalLiters;
    bool            breakdown;
    uint16_t        pumpCapacity;
    uint16_t        pumpUsage;
    uint16_t        dryStrokes;
    uint16_t        dryStrokeHeight;
    uint16_t        pumpUnusedTime;
    uint8_t         checksum;
}APP_NVM_SENSOR_DATA_WITH_HEADER_T;

#endif

#endif // ENGINEERING_DATA

#endif /* APP_NVM_CFG_SHARED_H */
