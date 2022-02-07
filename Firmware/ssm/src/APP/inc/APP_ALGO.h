/**************************************************************************************************
* \file     APP_ALGO.h
* \brief    Application level algorithm management
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


#ifndef APP_INC_APP_ALGO_H_
#define APP_INC_APP_ALGO_H_

#include "APP_NVM_Cfg_Shared.h"

#define ALGO_ERROR_OFFSET                   13
#define ORIENTATION_CALIB                   BIT_0
#define OFFSET_CALIB                        BIT_1
#define STROKE_BUFFER_OVERFLOW              BIT_2
#define MAGNET_PRESENT                      BIT_3
#define TRANS_BUFFER_OVERFLOW               BIT_4
#define CALIB_PRESENT_RESET                 BIT_5
#define WATER_CALIB                         BIT_6
#define WATER_CALIB_RESET                   BIT_7
#define WATER_CALIB_NEG                     BIT_8
#define WATER_BAD_SAMPLE                    BIT_9
#define WATER_STANDING                      BIT_10
#define WATER_CLOGGED_PUMP                  BIT_11
#define WATER_VOLUME_CAPPED                 BIT_12
#define MISSED_SAMPLE_THRESH                BIT_13
#define AVG_SAMPLE_PERIOD_DRIFT             BIT_14
#define CALIB_MAJOR_CHANGE_RESET            BIT_15
#define CALIB_ORIENT_RESET                  BIT_16
#define CALIB_NEW_OFFSET_VAL_1              BIT_17
#define CALIB_NEW_OFFSET_VAL_2              BIT_18

extern void APP_ALGO_Init(void);
extern void APP_ALGO_wakeUpInit(void);
extern void APP_ALGO_Nest(bool activeSampling);
extern void APP_ALGO_updateHourlyFields(APP_NVM_SENSOR_DATA_T *sensorData, uint8_t hoursIdx);
extern void APP_ALGO_updateDailyFields(APP_NVM_SENSOR_DATA_T *sensorData);
extern uint16_t APP_ALGO_getHourlyWaterVolume(void);
extern void APP_ALGO_resetHourlyStrokeCount(void);
extern void APP_ALGO_calculateHourlyStrokes(void);
extern uint16_t APP_ALGO_getHourlyStrokeCount(void);
extern uint16_t APP_ALGO_getHourlyDisplacement(void);
extern uint16_t APP_ALGO_getMagWindowsProcessed(void);
extern uint32_t APP_ALGO_monitorTotalLiters(void);
extern void APP_ALGO_setStrokeDetectionIsOn(bool algIsOn);
extern void APP_ALGO_initRedFlagThresholds(uint16_t onThreshold, uint16_t offThreshold);
extern bool APP_ALGO_isWaterPresent(void);
extern bool APP_ALGO_isMagnetPresent(void);
extern void APP_ALGO_resetRedFlagData(void);
extern void APP_ALGO_populateRedFlagArrayWithFakeData(void);
extern uint8_t APP_ALGO_getMaxHourUsage(void);
extern void APP_ALGO_computePumpHealth(uint8_t hour);

#endif /* APP_INC_APP_ALGO_H_ */
