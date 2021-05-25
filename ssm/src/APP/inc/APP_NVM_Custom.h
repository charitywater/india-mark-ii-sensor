/**************************************************************************************************
* \file     APP_NVM_Custom.h
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

#ifndef APP_NVM_CUSTOM_H
#define APP_NVM_CUSTOM_H

extern void APP_NVM_Custom_InitDeviceInfo(void);
extern bool APP_NVM_Custom_CheckSectionData(uint8_t map_index);
extern void APP_NVM_Custom_LogSensorData(APP_NVM_SENSOR_DATA_T * p_sensorData);

extern uint8_t APP_NVM_Custom_GetResetStateAndInit(void);
extern void APP_NVM_Custom_WriteResetState(uint8_t reset_state);

extern uint16_t APP_NVM_Custom_GetAmTransmissionRate(void);
extern bool APP_NVM_Custom_WriteAmTransmissionRate(uint16_t rateInDays);

extern uint8_t APP_NVM_Custom_GetHighLevelState(void);
extern void APP_NVM_Custom_WriteHighLevelState(uint8_t state);

extern uint32_t APP_NVM_Custom_GetActivatedDate(void);
extern bool APP_NVM_Custom_WriteActivatedDate(uint32_t date);

extern uint32_t APP_NVM_Custom_GetDeactivatedDate(void);
extern bool APP_NVM_Custom_WriteDeactivatedDate(uint32_t date);

extern uint32_t APP_NVM_Custom_GetTotalLiters(void);
extern bool APP_NVM_Custom_WriteTotalLiters(uint32_t totalLiters);

extern uint8_t APP_NVM_Custom_GetSensorDataNumEntries(void);
extern bool APP_NVM_GetSensorData(APP_NVM_SENSOR_DATA_T *sensorData);
extern void APP_NVM_Custom_IndicateBufferFull(bool bufferIsFull);
extern bool APP_NVM_Custom_GetBufferFullFlag(void);
extern void APP_NVM_SensorDataMsgAcked(void);

extern uint8_t APP_NVM_Custom_GetRtcTimeStatus(void);
extern bool APP_NVM_Custom_WriteRtcTimeStatus(uint8_t status);

extern uint32_t APP_NVM_Custom_GetUnexpectedResetCount(void);
extern uint32_t APP_NVM_Custom_GetTimestampLastUnexpectedReset(void);
extern void APP_NVM_Custom_IncrementUnexpectedResetCount(uint32_t timestamp);

extern bool APP_NVM_Custom_GetStrokeDetectionIsOn(void);
extern void APP_NVM_Custom_WriteStrokeDetectionIsOn(bool onOff);

extern void APP_NVM_Custom_WriteRedFlagOffThreshold(uint16_t threshold);
extern uint16_t APP_NVM_Custom_GetRedFlagOffThreshold(void);
extern void APP_NVM_Custom_WriteRedFlagOnThreshold(uint16_t threshold);
extern uint16_t APP_NVM_Custom_GetRedFlagOnThreshold(void);

#endif /* APP_NVM_CUSTOM_H */
