/**************************************************************************************************
* \file     PE42424A_RF.h
* \brief    API to control the rf switches.
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

#ifndef DEVICE_DRIVERS_PE42424A_RF_H_
#define DEVICE_DRIVERS_PE42424A_RF_H_

#include <stm32l4xx_hal.h>

#define CELL_RF_SWITCH_PIN           GPIO_PIN_11
#define CELL_RF_SWITCH_PORT          GPIOC

#define GPS_RF_SWITCH_PIN            GPIO_PIN_1
#define GPS_RF_SWITCH_PORT           GPIOD

extern void RF_initRfSwitchesAndCli(void);
extern void RF_cellPrimaryAntennaNoGps(void);
extern void RF_cellPrimaryAntennaGpsSecondary(void);
extern void RF_noPrimaryAntennaGpsSecondary(void);
extern void RF_noPrimaryAntennaCellSecondary(void);

#endif /* DEVICE_DRIVERS_PE42424A_RF_H_ */
