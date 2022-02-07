/**************************************************************************************************
* \file     mqttHandler.h
* \brief    MQTT handler - initialize, subscribe, post, handle all mqtt communication
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

#ifndef HANDLERS_MQTTHANDLER_H_
#define HANDLERS_MQTTHANDLER_H_

#include "iot_network.h"
#include "messages.pb.h"

//any app-cloud protocol should go here:
typedef struct
{
    uint16_t transmissionRateDays;
    uint32_t numOfSatellites;
    uint32_t maxHop;
    uint32_t minMeasureTime;
    uint32_t gpsTimeoutSeconds;
    bool strokeAlgIsOn;
    uint32_t redFlagOnThreshold;
    uint32_t redFlagOffThreshold;
}configureMsg_t;

extern void MQTT_task();

extern int MQTT_init( bool awsIotMqttMode,
        void * pNetworkServerInfo,
        void * pNetworkCredentialInfo,
        const IotNetworkInterface_t * pNetworkInterface, uint8_t *duid, uint8_t duidLength);


extern bool MQTT_sendStatusMsg(StatusMessage statusToSend);
extern bool MQTT_sendGpsMsg(GpsMessage gpsMsgToSend);
extern bool MQTT_sendSensorDataMsg(SensorDataMessage sensorDataMessageToSend);
extern bool MQTT_disconnect(void);
extern void MQTT_indicateOperationPass(void);
extern void MQTT_indicateOperationFail(void);
extern void MQTT_indicateBadConfigs(void);
extern void MQTT_IndicateReadyForNewJobs(void);
extern bool MQTT_getOperationInProgressFlag(void);

#endif /* HANDLERS_MQTTHANDLER_H_ */
