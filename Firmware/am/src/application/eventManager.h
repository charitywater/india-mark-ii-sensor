/**************************************************************************************************
* \file     eventManager.h
* \brief    Handle application level events - activation, deactivation, manage check ins, data
*           polling from the SSM, etc
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

#ifndef APPLICATION_EVENTMANAGER_H_
#define APPLICATION_EVENTMANAGER_H_

#include "mqttHandler.h"

#define AM_TIME_ON_MINS             11
#define AM_ALLOWED_TIME_ON_MS      (AM_TIME_ON_MINS * 60 *1000)

#define AM_CELL_TIME_ON_MINS       (AM_TIME_ON_MINS - 1)
#define AM_CELL_TIME_ON_MS         (AM_CELL_TIME_ON_MINS * 60 *1000)

//up to 15 mins when doing OTA
#define AM_ALLOWED_TIME_ON_OTA_MS   15*60*1000

//timeout for adding an event to the message queue
#define QUEUE_WAIT_TIME_MS          100

extern void EVT_eventManagerTask();

//Report an event:
extern void EVT_indicateActivateFromSsm(void);
extern void EVT_indicateActivateFromCloud(void);
extern void EVT_indicateNewConfigMessage(configureMsg_t configs);
extern void EVT_indicateDeActivate(void);
extern void EVT_indicateCheckInActivated(void);
extern void EVT_indicateCheckInDeactivated(void);
extern void EVT_initiateNtpTimeSync(void);
extern void EVT_indicateNtpTimeSyncSuccess(void);
extern void EVT_indicateNtpTimeSyncFailure(void);
extern void EVT_indicateNoNewJobsFromCloud(void);
extern void EVT_indicateSsmUnresponsive(void);
extern void EVT_indicateSsmNackedRequest(void);
extern void EVT_indicateOta(char * link);
extern void EVT_indicateFwDownloadFail(void);
extern void EVT_indicateFwDownloadComplete(void);
extern void EVT_indicateSensorDataMsgReceivedFromSSM(void);
extern void EVT_indicateHwResetCmd(bool deactivateBeforeReset);
extern void EVT_indicateResetAlarmsCmd(void);
extern void EVT_indicateMqttPublishSuccess(void);
extern void EVT_indicateCloudConnectFailure(void);
extern void EVT_indicateManufacturingComplete(void);
extern void EVT_indicateGpsLocationRequested(bool takeNewMeasurement);
extern void EVT_initializeEventQueue(void);
extern void EVT_indicateGpsFixCompleted(bool fixSucceeded);
extern void EVT_indicateMqttReady(void);
extern void EVT_indicateMqttNWError(void);
extern void EVT_indicateSensorDataReady(void);

#endif /* APPLICATION_EVENTMANAGER_H_ */
