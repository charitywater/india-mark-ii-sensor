/**************************************************************************************************
* \file     HW_BAT.h
* \brief    Battery Fuel gauge & voltage measurement functionality.
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

#ifndef HW_BAT_H
#define HW_BAT_H

typedef enum
{
    GAUGE_NOT_INITIALIZED,
    GAUGE_SUCCESS,
    GAUGE_INVALID_PARAM,
    GAUGE_OWI_ERROR,
}fuelGaugeStatus_t;

extern fuelGaugeStatus_t HW_FUEL_GAUGE_Initialize(void);
extern void HW_FUEL_GAUGE_PrintSerialNumber(void);
extern fuelGaugeStatus_t HW_FUEL_GAUGE_DeInit(void);
extern fuelGaugeStatus_t HW_FUEL_GAUGE_ReadStatusReg(uint8_t* data);
extern fuelGaugeStatus_t HW_FUEL_GAUGE_WriteStatusReg(uint8_t data);
extern fuelGaugeStatus_t HW_FUEL_GAUGE_ReadCurrentReg(uint16_t* data);
extern fuelGaugeStatus_t HW_FUEL_GAUGE_ReadAccumReg(uint16_t* data);
extern fuelGaugeStatus_t HW_FUEL_GAUGE_ClearAccumReg(void);
extern void HW_FUEL_GAUGE_readAccumulatedCurrent(int32_t* accumulator_uAh);
extern void HW_FUEL_GAUGE_readInstantCurrent(int32_t* instCurrent_uA);
extern void HW_BAT_Init(void);
extern uint16_t HW_BAT_GetVoltage(void);
extern void HW_BAT_TakeNewVoltageMeasurement(void);
extern bool HW_BAT_IsBatteryLow(void);

#endif /* HW_BAT_H */
