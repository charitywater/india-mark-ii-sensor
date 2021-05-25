/**************************************************************************************************
* \file     ssm.h
* \brief    Functions applicable to system support micro (ssm) control
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

#ifndef DEVICE_DRIVERS_SSM_H_
#define DEVICE_DRIVERS_SSM_H_

#include "am-ssm-spi-protocol.h"

#define AM_SSM_UART_MUX_PIN     GPIO_PIN_8
#define AM_SSM_UART_MUX_PORT    GPIOD

#define SSM_BOOT_PIN            GPIO_PIN_7
#define SSM_BOOT_PORT           GPIOE

#define AM_AWAKE_PIN            GPIO_PIN_13
#define AM_AWAKE_PORT           GPIOC

#define SSM_RST_PIN             GPIO_PIN_2
#define SSM_RST_PORT            GPIOE

#define SSM_TEST_PIN            GPIO_PIN_3
#define SSM_TEST_PORT           GPIOE

extern void SSM_enableUart(void);
extern void SSM_disableUart(void);
extern void SSM_enableBootPin(void);
extern void SSM_disableBootPin(void);
extern void SSM_SPI_Task(void * p_parameters);
extern void SSM_changeStateToActivated(void);
extern void SSM_deactivateionReceivedFromCloud(void);
extern void SSM_Init(void);
extern asp_status_payload_t SSM_getStatus(void);
extern asp_sensor_data_entry_t SSM_getSensorData(void);
extern void SSM_requestStatusFromSsm(void);
extern void SSM_requestNumDataLogEntriesFromSsm(void);
extern void SSM_requestNewDataLogEntryFromSsm(void);
extern void SSM_sensorDataStoredToFlash(void);
extern void SSM_sendConfigs(void);
extern void SSM_performTimeSync(void);
extern bool SSM_getTimeSyncRequested(void);
extern void SSM_putIntoBootloadModeThroughResetPin(void);
extern void SSM_hardwareReset(void);
extern bool SSM_communicationCheck(void);
extern void SSM_setAlgoConfig(bool onOff);
extern bool SSM_setRedFlagThresholdConfigs(uint16_t flagOn, uint16_t flagOff);
extern bool SSM_sendHwResetCmd(void);
extern void SSM_sendResetAlarms(void);

#endif /* DEVICE_DRIVERS_SSM_H_ */
