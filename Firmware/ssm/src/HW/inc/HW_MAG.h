/**************************************************************************************************
* \file     HW_MAG.h
* \brief    Magnetometer (lis2mdl) functions
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

#ifndef HW_INC_HW_MAG_H_
#define HW_INC_HW_MAG_H_

#include <stdint.h>

extern void HW_MAG_InitBusAndDevice(void);
extern uint8_t HW_MAG_getID(void);
extern void HW_MAG_GetLatestMagAndTempData(int16_t *xLsb, int16_t *yLsb, int16_t *zLsb, int16_t *tempLsb, uint8_t *bitFlags);
extern void HW_MAG_SampleAndReport(void);
extern void HW_MAG_ChangeSampleRate(uint8_t rateInHz);
extern void HW_MAG_ChangeOperatingMode(uint8_t mode);
extern uint8_t HW_MAG_GetOperatingMode(void);
extern void HW_MAG_InitSampleRateAndPowerModeOn(void);
extern void HW_MAG_EnableThresholdInterrupt(int16_t xAxisOffset, int16_t yAxisOffset, int16_t zAxisOffset);
extern void HW_MAG_TurnOffSampling(void);
extern void HW_Mag_DataReadyIntOccured(void);
extern void HW_MAG_Monitor(void);
extern void HW_MAG_StartConversion(void);

#endif /* HW_INC_HW_MAG_H_ */
