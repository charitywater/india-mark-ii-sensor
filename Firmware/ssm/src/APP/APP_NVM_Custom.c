/**************************************************************************************************
* \file     APP_NVM_Custom.c
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

#include "APP_NVM.h"
#include "APP_NVM_Types.h"
#include "APP_NVM_Cfg.h"
#include "HW_TERM.h"
#include "HW_EEP.h"
#include "am-ssm-spi-protocol.h"
#include "uC_TIME.h"
#include "APP_NVM_Custom.h"

bool APP_NVM_Custom_CheckSectionData(uint8_t map_index);
void APP_NVM_Custom_LogSensorData(APP_NVM_SENSOR_DATA_T * p_sensorData);
void APP_NVM_Custom_InitDeviceInfo(void);
void APP_NVM_Custom_WriteResetState(uint8_t reset_state);
bool APP_NVM_GetSensorData(APP_NVM_SENSOR_DATA_T *sensorData);
void APP_NVM_SensorDataMsgAcked(void);
uint8_t APP_NVM_Custom_GetSensorDataNumEntries(void);
void APP_NVM_Custom_IndicateBufferFull(bool bufferIsFull);
bool APP_NVM_GenericCheckData(uint8_t map_index);
bool APP_NVM_VerifyChecksum(uint8_t * p_buf, uint8_t buf_len, uint8_t expected_checksum);

static bool CheckDeviceInfo(uint8_t map_index);

static APP_NVM_DEVICE_INFO_T Dev_Info = {};
static APP_NVM_SECTION_HDR_T SensorDataHdr = {};

static bool xSensorDataIsFull = false;

// Read the specified number of sensor data records
bool APP_NVM_GetSensorData(APP_NVM_SENSOR_DATA_T *sensorData)
{
    uint16_t addr = 0;
    uint8_t * p_report = (uint8_t *) sensorData;
    uint8_t pos = 0;

    // Get number of entries, return number of entries
    if ( APP_NVM_Custom_GetSensorDataNumEntries() == 0 )
    {
        // No new entries
        return false;
    }

    // Update and increment the position to read from
    pos = SensorDataHdr.tail;

    // New entries available
    addr = (Section_Map[APP_NVM_SECT_TYPE_SENSOR_DATA].start_addr + sizeof(APP_NVM_SECTION_HDR_T) + (pos * sizeof(APP_NVM_SENSOR_DATA_T)));
    APP_NVM_ReadBytes(addr, sizeof(APP_NVM_SENSOR_DATA_T), (uint8_t *) p_report);

    return true;
}

void APP_NVM_SensorDataMsgAcked(void)
{
    if (  APP_NVM_Custom_GetBufferFullFlag() == true )
    {
        //reset full flag
        APP_NVM_Custom_IndicateBufferFull(false);

        //store to EEPROM
        Dev_Info.sensorDataBufferFull = false;
        APP_NVM_UpdateCurrentEntry(APP_NVM_SECT_TYPE_DEVICE_INFO,  (uint8_t *) &Dev_Info, false);
    }

    SensorDataHdr.tail = (SensorDataHdr.tail + 1) % MAX_SENSOR_DATA_LOGS;
    SensorDataHdr.checksum = APP_NVM_ComputeChecksum((uint8_t *)&SensorDataHdr, (sizeof(APP_NVM_SECTION_HDR_T)-1));
    HW_EEP_WriteBlock(Section_Map[APP_NVM_SECT_TYPE_SENSOR_DATA].start_addr, (uint8_t *) &SensorDataHdr, sizeof(APP_NVM_SECTION_HDR_T));
}

void APP_NVM_Custom_IndicateBufferFull(bool bufferIsFull)
{
    xSensorDataIsFull = bufferIsFull;
}

bool APP_NVM_Custom_GetBufferFullFlag(void)
{
    return xSensorDataIsFull;
}

uint8_t APP_NVM_Custom_GetSensorDataNumEntries(void)
{
    APP_NVM_ReadSectionHeader(APP_NVM_SECT_TYPE_SENSOR_DATA, &SensorDataHdr);

    if ( APP_NVM_Custom_GetBufferFullFlag() != true )
    {
        if(SensorDataHdr.head >= SensorDataHdr.tail)
        {
            return (SensorDataHdr.head - SensorDataHdr.tail);
        }
        else
        {
            return ((MAX_SENSOR_DATA_LOGS - SensorDataHdr.tail) + SensorDataHdr.head);
        }
    }
    else
    {
        return MAX_SENSOR_DATA_LOGS;
    }
}

//read the device info section of eeprom
void APP_NVM_Custom_InitDeviceInfo(void)
{
    APP_NVM_ReadCurrentEntry(APP_NVM_SECT_TYPE_DEVICE_INFO, (uint8_t *) &Dev_Info);

    //validate configs
    if( Dev_Info.current_high_lev_state > MAX_HIGH_LEVEL_STATE_VAL )
    {
        Dev_Info.current_high_lev_state = FAULT;
    }

    if ( Dev_Info.am_wake_rate_days > MAX_WAKE_AM_RATE_DAYS || Dev_Info.am_wake_rate_days < MIN_WAKE_AM_RATE_DAYS )
    {
        if (Dev_Info.current_high_lev_state == DEACTIVATED )
        {
            Dev_Info.am_wake_rate_days = DEFAULT_WAKE_AM_RATE_DAYS_DEACTIVATED;
        }
        else
        {
            Dev_Info.am_wake_rate_days = DEFAULT_WAKE_AM_RATE_DAYS_ACTIVATED;
        }
    }


    if ( Dev_Info.redFlagOnThreshold > MAX_RED_FLAG_ON_THRESH || Dev_Info.redFlagOnThreshold < MIN_RED_FLAG_ON_THRESH )
    {
        Dev_Info.redFlagOnThreshold = DEFAULT_RED_FLAG_ON_THRESH;
    }

    if ( Dev_Info.redFlagOffThreshold > MAX_RED_FLAG_OFF_THRESH || Dev_Info.redFlagOffThreshold < MIN_RED_FLAG_OFF_THRESH )
    {
        Dev_Info.redFlagOffThreshold = DEFAULT_RED_FLAG_OFF_THRESH;
    }

    if ( APP_NVM_VerifyChecksum( (uint8_t*)&Dev_Info, sizeof(APP_NVM_DEVICE_INFO_T) - 1, Dev_Info.checksum ) == false )
    {
        HW_TERM_Print("INVALID CS Read from device info \n");

        //write defaults for the next boot up
        APP_NVM_DefaultSection(APP_NVM_SECT_TYPE_DEVICE_INFO);

        //set the ram copy equal to the default
        Dev_Info = Device_Info_Default;
    }

    APP_NVM_Custom_WriteRtcTimeStatus(RTC_FIRST_TIME_SYNC);

    //init sensor data buffer status
    APP_NVM_Custom_IndicateBufferFull(Dev_Info.sensorDataBufferFull);
}

void APP_NVM_Custom_WriteResetState(uint8_t reset_state)
{
    Dev_Info.reset_state = reset_state;
    APP_NVM_UpdateCurrentEntry(APP_NVM_SECT_TYPE_DEVICE_INFO,  (uint8_t *) &Dev_Info, false);
}

uint8_t APP_NVM_Custom_GetResetStateAndInit(void)
{
    static bool initial_state_reported = false;
    uint8_t reset_state = STATE_OK;

    // Only report a non-ok reset state on the first poll then report ok.
    if (initial_state_reported == false)
    {
        // Report the stored reset state.
        reset_state = Dev_Info.reset_state;
        // Set the state to an error state so that if we have a spurious reset we will know.
        APP_NVM_Custom_WriteResetState(STATE_ERR);
        initial_state_reported = true;
    }

    return reset_state;
}

uint32_t APP_NVM_Custom_GetUnexpectedResetCount(void)
{
    return Dev_Info.unexpected_reset_counter;
}

uint32_t APP_NVM_Custom_GetTimestampLastUnexpectedReset(void)
{
    return Dev_Info.time_of_last_unexpected_reset;
}

void APP_NVM_Custom_IncrementUnexpectedResetCount(uint32_t timestamp)
{
    Dev_Info.unexpected_reset_counter++;
    Dev_Info.time_of_last_unexpected_reset = timestamp;

    APP_NVM_UpdateCurrentEntry(APP_NVM_SECT_TYPE_DEVICE_INFO,  (uint8_t *) &Dev_Info, false);
}

uint16_t APP_NVM_Custom_GetAmTransmissionRate(void)
{
    return Dev_Info.am_wake_rate_days;
}

bool APP_NVM_Custom_WriteAmTransmissionRate(uint16_t rateInDays)
{
    bool result = false;

    //check before writing
    if ( rateInDays <= MAX_WAKE_AM_RATE_DAYS && rateInDays >= MIN_WAKE_AM_RATE_DAYS )
    {
        Dev_Info.am_wake_rate_days = rateInDays;
        APP_NVM_UpdateCurrentEntry(APP_NVM_SECT_TYPE_DEVICE_INFO,  (uint8_t *) &Dev_Info, false);
        result = true;
    }

    return result;
}

uint8_t APP_NVM_Custom_GetHighLevelState(void)
{
    return Dev_Info.current_high_lev_state;
}

void APP_NVM_Custom_WriteHighLevelState(uint8_t state)
{
    Dev_Info.current_high_lev_state = state;
    APP_NVM_UpdateCurrentEntry(APP_NVM_SECT_TYPE_DEVICE_INFO,  (uint8_t *) &Dev_Info, false);
}

void APP_NVM_Custom_LogSensorData(APP_NVM_SENSOR_DATA_T * p_sensorData)
{
    APP_NVM_UpdateCurrentEntry(APP_NVM_SECT_TYPE_SENSOR_DATA, (uint8_t *)p_sensorData, true);

    if ( APP_NVM_Custom_GetBufferFullFlag() == true )
    {
        //store to EEPROM
        Dev_Info.sensorDataBufferFull = true;
        APP_NVM_UpdateCurrentEntry(APP_NVM_SECT_TYPE_DEVICE_INFO,  (uint8_t *) &Dev_Info, false);
    }
}

uint32_t APP_NVM_Custom_GetActivatedDate(void)
{
    return Dev_Info.recent_act_date;
}

bool APP_NVM_Custom_WriteActivatedDate(uint32_t date)
{
    Dev_Info.recent_act_date = date;
    APP_NVM_UpdateCurrentEntry(APP_NVM_SECT_TYPE_DEVICE_INFO,  (uint8_t *) &Dev_Info, false);

    return true;
}

uint32_t APP_NVM_Custom_GetDeactivatedDate(void)
{
    return Dev_Info.recent_act_date;
}

bool APP_NVM_Custom_WriteDeactivatedDate(uint32_t date)
{
    Dev_Info.recent_deact_date = date;
    APP_NVM_UpdateCurrentEntry(APP_NVM_SECT_TYPE_DEVICE_INFO,  (uint8_t *) &Dev_Info, false);

    return true;
}

uint8_t APP_NVM_Custom_GetRtcTimeStatus(void)
{
    return Dev_Info.rtc_time_status;
}

bool APP_NVM_Custom_WriteRtcTimeStatus(uint8_t status)
{
    Dev_Info.rtc_time_status = status;
    APP_NVM_UpdateCurrentEntry(APP_NVM_SECT_TYPE_DEVICE_INFO,  (uint8_t *) &Dev_Info, false);

    return true;
}

bool APP_NVM_Custom_GetStrokeDetectionIsOn(void)
{
    return Dev_Info.strokeDetectionAlgIsOn;
}

void APP_NVM_Custom_WriteStrokeDetectionIsOn(bool onOff)
{
    Dev_Info.strokeDetectionAlgIsOn = onOff;
    APP_NVM_UpdateCurrentEntry(APP_NVM_SECT_TYPE_DEVICE_INFO,  (uint8_t *) &Dev_Info, false);
}

uint16_t APP_NVM_Custom_GetRedFlagOnThreshold(void)
{
    return Dev_Info.redFlagOnThreshold;
}

void APP_NVM_Custom_WriteRedFlagOnThreshold(uint16_t threshold)
{
    Dev_Info.redFlagOnThreshold = threshold;
    APP_NVM_UpdateCurrentEntry(APP_NVM_SECT_TYPE_DEVICE_INFO,  (uint8_t *) &Dev_Info, false);
}

void APP_NVM_Custom_WriteRedFlagOffThreshold(uint16_t threshold)
{
    Dev_Info.redFlagOffThreshold = threshold;
    APP_NVM_UpdateCurrentEntry(APP_NVM_SECT_TYPE_DEVICE_INFO,  (uint8_t *) &Dev_Info, false);
}

uint16_t APP_NVM_Custom_GetRedFlagOffThreshold(void)
{
    return Dev_Info.redFlagOffThreshold;
}

uint32_t APP_NVM_Custom_GetTotalLiters(void)
{
    return Dev_Info.total_liters;
}

bool APP_NVM_Custom_WriteTotalLiters(uint32_t totalLiters)
{
    Dev_Info.total_liters = totalLiters;
    APP_NVM_UpdateCurrentEntry(APP_NVM_SECT_TYPE_DEVICE_INFO,  (uint8_t *) &Dev_Info, false);

    return true;
}

// This function must be implemented and must return true/false indicating whether the data is good for the given section
// section type.  If you do not have anything specific to check, then you may use APP_NVM_GenericCheckData() which will
// only check the checksums for all entries.
bool APP_NVM_Custom_CheckSectionData(uint8_t map_index)
{
    bool data_good = false;

    if (map_index > (APP_NVM_NUM_SECTIONS - 1)) return false;

    switch(Section_Map[map_index].type)
    {
        case APP_NVM_SECT_TYPE_DEVICE_INFO:
        {
            data_good = CheckDeviceInfo(map_index);
            break;
        }
        case APP_NVM_SECT_TYPE_SENSOR_DATA:
        default:
        {
            data_good = APP_NVM_GenericCheckData(map_index);
            break;
        }
    }

    return data_good;
}

// Confirm that the data in the device info section is is good.  E.g. NVM version is
// up-to-date and the checksum is correct. This is very similar to CheckCustomData()
// but will have knowledge of the APP_NVM_DEVICE_INFO_T so can go beyond to check things
// like nvm version.
static bool CheckDeviceInfo(uint8_t map_index)
{
    bool section_good = true;
    uint8_t i = 0;
    uint8_t buf[sizeof(APP_NVM_DEVICE_INFO_T)];
    void * p_buf = buf;
    uint16_t addr = 0;

    HW_TERM_Print("Checking device info.\n");

    if (map_index > (APP_NVM_NUM_SECTIONS - 1)) return false;

    addr = (Section_Map[map_index].start_addr + sizeof(APP_NVM_SECTION_HDR_T));  // Start after the header.

    for (i = 0; i < sizeof(APP_NVM_DEVICE_INFO_T); i++)
    {
        *(uint8_t *)p_buf = HW_EEP_ReadByte(addr + i);
        p_buf = (uint8_t *)p_buf + 1;
    }

    p_buf = buf;

    memcpy(&Dev_Info, buf, sizeof(APP_NVM_DEVICE_INFO_T));

    // Confirm the expected NVM version.
    if (((APP_NVM_DEVICE_INFO_T *)p_buf)->nvm_version == APP_NVM_VERSION)
    {
        HW_TERM_PrintColor("NVM version match.\n", KGRN);
    }
    else
    {
        section_good = false;
        HW_TERM_PrintColor("NVM version mismatch!\n", KRED);
    }

    if (APP_NVM_VerifyChecksum(buf, (sizeof(APP_NVM_DEVICE_INFO_T) - 1), (((APP_NVM_DEVICE_INFO_T *)p_buf)->checksum)) != true)
        section_good = false;

    return section_good;
}
