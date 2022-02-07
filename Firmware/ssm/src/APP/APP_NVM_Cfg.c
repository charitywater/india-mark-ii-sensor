/**************************************************************************************************
* \file     APP_NVM_Cfg.c
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
#include "APP_NVM_Cfg.h"
#include "am-ssm-spi-protocol.h"


const APP_NVM_DEVICE_INFO_T Device_Info_Default =
{
    .nvm_version = APP_NVM_VERSION,                             // Set to APP_NVM_VERSION, which should updated on changes.
    .current_high_lev_state = DEACTIVATED,                      // activated, deactivated, fault
    .first_act_date = 0,                                        // Date the device was first activated.
    .recent_act_date = 0,                                       // Most-recent date the device was activated.
    .recent_deact_date = 0,                                     // Most-recent date the device was deactivated.
    .num_act = 0,                                               // Total number of times the devices as been activated.
    .num_deact = 0,                                             // Total number of times the devices as been deactivated.
    .am_wake_rate_days = DEFAULT_WAKE_AM_RATE_DAYS_ACTIVATED,   // Rate (in days) to wake the AM for cloud communication when activated
    .reset_state = STATE_POR,
    .total_liters = 0,
    .rtc_time_status = RTC_FIRST_TIME_SYNC,
    .strokeDetectionAlgIsOn = false,
    .unexpected_reset_counter = 0,
    .time_of_last_unexpected_reset = 0,
    .redFlagOnThreshold = DEFAULT_RED_FLAG_ON_THRESH,
    .redFlagOffThreshold = DEFAULT_RED_FLAG_OFF_THRESH,
    .sensorDataBufferFull = false,
};

const APP_NVM_SECTION_MAP_T Section_Map[APP_NVM_NUM_SECTIONS] =
{
    // Device info section.  128 bytes allocated
    {
        .type = APP_NVM_SECT_TYPE_DEVICE_INFO,
        .start_addr = 0x0000,
        .end_addr = 0x007F,
        .is_array = false,
        .entry_len = sizeof(APP_NVM_DEVICE_INFO_T),
        .default_num_entries = 1,
        .p_default_values = (void *)&Device_Info_Default,
    },

    // Daily reports section
    // Current record size is 214 bytes. This section will store 56 (8 weeks) of days of daily message data
    {
        .type = APP_NVM_SECT_TYPE_SENSOR_DATA,
        .start_addr = 0x0100,
        .end_addr = (0x0100 + (MAX_SENSOR_DATA_LOGS * sizeof(APP_NVM_SENSOR_DATA_T))) + sizeof(APP_NVM_SECTION_HDR_T),
        .is_array = true,
        .entry_len = sizeof(APP_NVM_SENSOR_DATA_T),
        .default_num_entries = 0,
        .p_default_values = NULL,
    },
};
