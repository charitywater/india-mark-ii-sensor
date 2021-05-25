/**************************************************************************************************
* \file     APP_WTR.c
* \brief    Application Water Level Measurement
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

#include "CAPT_UserConfig.h"
#include "APP_WTR.h"
#include "HW_TERM.h"
#include "APP_NVM.h"
#include "uC_TIME.h"
#include "uC_UART.h"
#include "APP_CLI.h"
#include "HW_MAG.h"
#include "HW_RTC.h"
#include "HW_ENV.h"

extern tElement WaterPads_E00;
extern tElement WaterPads_E01;
extern tElement WaterPads_E02;
extern tElement WaterPads_E03;
extern tElement WaterPads_E04;
extern tElement WaterPads_E05;
extern tElement WaterPads_E06;
extern tElement WaterPads_E07;

static const char * Pads[]={"1","2","3","4","5","6","7","8"};

uint8_t APP_WTR_CheckLevel(void);
uint16_t APP_WTR_GetPadValue(APP_WTR_PAD_CHANNELS_T pad);
const char * APP_WTR_GetStrForPad(APP_WTR_PAD_CHANNELS_T pos);

#ifdef ENGINEERING_DATA

void APP_WTR_CollectData(uint32_t ticks);

uint32_t Stream_Records = 0;
void StreamSensorData(APP_NVM_SENSOR_DATA_T * p_sensorData);

void StreamSensorData(APP_NVM_SENSOR_DATA_T * p_sensorData)
{
    if (APP_CLI_BinaryData() == true)
    {
        uint8_t magic_val = 0xa5;
        uC_UART_Tx((uint8_t *)&magic_val, 1);
        uC_UART_Tx((uint8_t *)p_sensorData, sizeof(APP_NVM_SENSOR_DATA_T));
    }
    else
    {
        char str[100];

        HW_TERM_Print("\n------------------------\n");
        sprintf(str, "[RECORD %lu]\n", Stream_Records);
        HW_TERM_Print((uint8_t *)str);
        HW_TERM_Print("------------------------\n");

        sprintf(str, "0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X,0x%02X,0x%02X,0x%02X, 0x%02X, ",
                ((uint8_t)p_sensorData->timestamp),
                ((uint8_t)(p_sensorData->timestamp>>8)),
                ((uint8_t)(p_sensorData->timestamp>>16)),
                ((uint8_t)(p_sensorData->timestamp>>24)),
                ((uint8_t)p_sensorData->pads[0]),
                ((uint8_t)(p_sensorData->pads[0]>>8)),
                ((uint8_t)p_sensorData->pads[1]),
                ((uint8_t)(p_sensorData->pads[1]>>8)),
                ((uint8_t)p_sensorData->pads[2]),
                ((uint8_t)(p_sensorData->pads[2]>>8)),
                ((uint8_t)p_sensorData->pads[3]),
                ((uint8_t)(p_sensorData->pads[3]>>8)),
                ((uint8_t)p_sensorData->pads[4]),
                ((uint8_t)(p_sensorData->pads[4]>>8)),
                ((uint8_t)p_sensorData->pads[5]),
                ((uint8_t)(p_sensorData->pads[5]>>8)),
                ((uint8_t)(p_sensorData->pads[5]>>8)),
                ((uint8_t)p_sensorData->pads[6]),
                ((uint8_t)(p_sensorData->pads[6]>>8)),
                ((uint8_t)p_sensorData->pads[7]),
                ((uint8_t)(p_sensorData->pads[7]>>8)),
                p_sensorData->magnetometerX,
                p_sensorData->magnetometerY,
                p_sensorData->magnetometerZ,
                p_sensorData->temp_c_raw,
                ((uint8_t)p_sensorData->checksum));

        HW_TERM_Print((uint8_t *)str);
    }

    Stream_Records++;
}


void APP_WTR_CollectData(uint32_t ticks)
{
    APP_WTR_PADS_T i = APP_WTR_LOWEST_PAD;
    APP_NVM_SENSOR_DATA_T sensorData = {};
    HW_ENV_SAMPLE_T * p_env_sample;

    sensorData.timestamp = ticks;

    //get latest temp/humidity value
    p_env_sample = HW_ENV_GetLatestSample();
    sensorData.humidity_raw = p_env_sample->humidity_raw;
    sensorData.temp_c_raw = p_env_sample->temp_c_raw;

    //get cap sense pad values
    for(i=APP_WTR_LOWEST_PAD; i < APP_WTR_GetNumPads(); i++)
    {
        sensorData.pads[i] = APP_WTR_GetPadValue(i);
    }

    //get latest magnetometer XYZ, temperature, and status register
    HW_MAG_GetLatestMagAndTempData(&sensorData.magnetometerX, &sensorData.magnetometerY, &sensorData.magnetometerZ, &sensorData.tempLsb, &sensorData.magStatBitFlags);

    #ifndef STREAM_ENGINEERING_DATA
    APP_NVM_Custom_LogSensorData(&sensorData);
    #else

    // Compute the checksum that would have been computed by the NVM app had we been storing to NVM instead of streaming.
    // That way we can still confirm the data without having to store to eeprom
    sensorData.checksum = APP_NVM_ComputeChecksum((uint8_t *)&sensorData, (sizeof(APP_NVM_SENSOR_DATA_T)-1));

    //send data out over UART
    StreamSensorData(&sensorData);

    #endif// STREAM_ENGINEERING_DATA
}
#endif // ENGINEERING_DATA

// Get the string name for the pad.
const char * APP_WTR_GetStrForPad(APP_WTR_PAD_CHANNELS_T pos)
{
    return Pads[pos];
}

// Start at the lowest pad and see how high the water level is
uint8_t APP_WTR_CheckLevel(void)
{
    uint8_t level = 0;

    if(WaterPads.bSensorTouch == true)
    {
        if (WaterPads_E00.bTouch == true) level++;
        else return level;

        if (WaterPads_E02.bTouch == true) level++;
        else return level;

        if (WaterPads_E03.bTouch == true) level++;
        else return level;

        if (WaterPads_E04.bTouch == true) level++;
        else return level;

        if (WaterPads_E07.bTouch == true) level++;
        else return level;

        if (WaterPads_E01.bTouch == true) level++;

        else return level;

        if (WaterPads_E05.bTouch == true) level++;
        else return level;

        if (WaterPads_E06.bTouch == true) level++;
        else return level;
    }

    return level;
}

// Get the raw counts on the pad.
uint16_t APP_WTR_GetPadValue(APP_WTR_PAD_CHANNELS_T pad)
{
    uint16_t value = 0u;

    switch (pad)
    {
        case CAPTIVATE_RX_CHANNEL2:
        {
            value = (*(WaterPads_E01.pRawCount));
            break;
        }

        case CAPTIVATE_RX_CHANNEL1:
        {
            value = (*(WaterPads_E00.pRawCount));
            break;
        }

        case CAPTIVATE_RX_CHANNEL3:
        {
            value = (*(WaterPads_E02.pRawCount));
            break;
        }

        case CAPTIVATE_RX_CHANNEL5:
        {
            value = (*(WaterPads_E04.pRawCount));
            break;
        }

        case CAPTIVATE_RX_CHANNEL4:
        {
            value = (*(WaterPads_E03.pRawCount));
            break;
        }

        case CAPTIVATE_RX_CHANNEL6:
        {
            value = (*(WaterPads_E05.pRawCount));
            break;
        }

        case CAPTIVATE_RX_CHANNEL7:
        {
            value = (*(WaterPads_E06.pRawCount));
            break;
        }

        case CAPTIVATE_RX_CHANNEL8:
        {
            value = (*(WaterPads_E07.pRawCount));
            break;
        }

        default:
        {
            value = 0u;
            break;
        }
    }

    return value;
}
