/**************************************************************************************************
* \file     HW_ENV.h
* \brief    Driver to interface with the temperature and humidity sensor (HDC2010)
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

#ifndef HW_ENV_H
#define HW_ENV_H

#include "HW.h"

// When the address pin for the HDC2010 is grounded, address is 1000000b
#define HW_ENV_SLAVE_ADDR                       0x40

typedef struct
{
    uint8_t temp_c;
    uint8_t humidity;
    uint16_t temp_c_raw;
    uint16_t humidity_raw;
}HW_ENV_SAMPLE_T;

extern void HW_ENV_DoTest(void);
extern void HW_ENV_Init(void);
extern bool HW_ENV_CheckMfgID(void);
extern HW_ENV_SAMPLE_T * HW_ENV_GetLatestSample(void);
extern void HW_ENV_GetLatestSampleAndReport(void);
extern void HW_ENV_DataRdyInt(void);
extern void HW_ENV_Monitor(void);
extern bool HW_ENV_ChangeSampleRate(uint8_t newSamleRateInHz);
extern void HW_ENV_TriggerNewEnvSample(void);
extern void HW_ENV_GetNewEnvSample(void);

#endif /* HW_ENV_H */
