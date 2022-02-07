/**************************************************************************************************
* \file     APP_NVM_Cfg.h
* \brief    Custom functionality related to application level non-volatile memory management.
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
#ifndef APP_NVM_CFG_H
#define APP_NVM_CFG_H

#include "APP_NVM_Types.h"
#include "APP_NVM_Cfg_Shared.h"
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "APP_WTR.h"

#define APP_NVM_VERSION                         ((uint16_t) 1u) // This will be compared to the version stored in EEPROM
#define APP_NVM_NUM_SECTIONS                    ((uint8_t) 2u)  // Number of entries in Section_Map[]

// Add one unique definition for each unique section type.  Good idea to make these sequential so that they can be your
// index into the Section_Map[] for your interface functions in APP_NVM_Custom.
#define APP_NVM_SECT_TYPE_DEVICE_INFO           ((uint8_t) 0u)
#define APP_NVM_SECT_TYPE_SENSOR_DATA           ((uint8_t) 1u)


//define min, max, and default values for configs:
#define MIN_WAKE_AM_RATE_DAYS                   1
#define MAX_WAKE_AM_RATE_DAYS                   56
#define DEFAULT_WAKE_AM_RATE_DAYS_ACTIVATED     7
#define DEFAULT_WAKE_AM_RATE_DAYS_DEACTIVATED   28

//red flag on/off threshold percentages:
#define MIN_RED_FLAG_ON_THRESH                  0
#define MAX_RED_FLAG_ON_THRESH                  100
#define DEFAULT_RED_FLAG_ON_THRESH              25

#define MIN_RED_FLAG_OFF_THRESH                 0
#define MAX_RED_FLAG_OFF_THRESH                 100
#define DEFAULT_RED_FLAG_OFF_THRESH             75

#define MIN_HIGH_LEVEL_STATE_VAL                0
#define MAX_HIGH_LEVEL_STATE_VAL                2
#define DEFAULT_HIGH_LEVEL_STATE_VAL            2

#define RTC_FIRST_TIME_SYNC                     0
#define RTC_TIME_SYNC_PERIODIC                  1
#define RTC_TIME_UPDATED                        2

#define DAILY_LITERS_TO_AVG                     28


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

// This section type provides generic info about the device. There will only be one copy of this information
// and by nature it will not be updated frequently (FW updates, activation, etc. )
// Anything we don't want to loose during a reset should be added to this device info struct
typedef struct __attribute__ ((__packed__)) APP_NVM_DEVICE_INFO
{
    uint16_t        nvm_version;                                            // Set to APP_NVM_VERSION, which should updated on changes.
    uint8_t         current_high_lev_state;                                 //activated, deactivated, etc
    uint32_t        first_act_date;                                         // Date the device was first activated.
    uint32_t        recent_act_date;                                        // Most-recent date the device was activated.
    uint32_t        recent_deact_date;                                      // Most-recent date the device was deactivated.
    uint8_t         num_act;                                                // Total number of times the devices as been activated.
    uint8_t         num_deact;                                              // Total number of times the devices as been deactivated.
    uint16_t        am_wake_rate_days;                                      // Rate in days at which to wake up the AM
    uint8_t         reset_state;                                            // Information on the reset source - this is how we determine unexpected vs intentional reset
    uint32_t        unexpected_reset_counter;                               // Number of resets that are not intentional (ie. a HW reset command would be an intentional reset)
    uint32_t        time_of_last_unexpected_reset;                          // Epoch time of the last reset
    uint32_t        total_liters;                                           // Total liters pumped since device activation
    uint8_t         rtc_time_status;                                        // Information on the RTC time sync status.. first time sync event or periodic.
    bool            strokeDetectionAlgIsOn;                                 //determine if the stroke algorithm should run
    uint16_t        redFlagOnThreshold;                                     // red flag is present when daily liters is less than this % of daily liters avg for the day
    uint16_t        redFlagOffThreshold;                                    // red flag is cleared when daily liters is greather than this % of daily liters avg for the day
    bool            sensorDataBufferFull;                                   // status of the sensor data buffer. Need this because head = tail could be 0 logs or 42
    uint8_t         checksum;
}APP_NVM_DEVICE_INFO_T;

typedef union APP_NVM_SECTION_DATA
{
    APP_NVM_SECTION_HDR_T   header;
    APP_NVM_SENSOR_DATA_T   sensor_data;
    APP_NVM_DEVICE_INFO_T   device_info;
}APP_NVM_SECTION_DATA_T;

extern const APP_NVM_DEVICE_INFO_T Device_Info_Default;

// Section map defines section locations and contents.
extern const APP_NVM_SECTION_MAP_T Section_Map[APP_NVM_NUM_SECTIONS];

#endif /* APP_NVM_CFG_H */
