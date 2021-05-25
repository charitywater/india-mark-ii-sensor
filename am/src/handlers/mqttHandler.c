/**************************************************************************************************
* \file     mqttHandler.c
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

#include "iot_config.h"
#include "iot_mqtt.h"
#include "aws_iot_network_config.h"
#include "iot_serializer.h"
#include "iot_json_utils.h"

/* Standard includes. */
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "logTypes.h"
#include "messages.pb.h"
#include "pb_encode.h"

/* Platform layer includes. */
#include "iot_clock.h"
#include "iot_threads.h"
#include "iot_network_types.h"
#include "eventManager.h"
#include "queue.h"

/* MQTT include. */
#include "mqttHandler.h"

#define UNKNOWN_DEVICE_IDENTIFIER                 "UnknownDevice"

//The longest client identifier that an MQTT server must accept (as defined
//by the MQTT 3.1.1 spec) is 23 characters. Add 1 to include the length of the NULL
//terminator.
#define CLIENT_IDENTIFIER_MAX_LENGTH             ( 24 )

//An MQTT ping request will be sent periodically at this interval.
//we can delete this later but good for testing
#define KEEP_ALIVE_SECONDS                       ( 60 * 8 )

//keep the connection alive up to 15 mins
#define MQTT_TIMEOUT_MS                          ( 60000 * 15)

//there are 2 fixed job related topics and 3 others
#define TOPIC_FILTER_COUNT                       ( 5 )
#define PUBLISH_RETRY_LIMIT                      ( 10 )
#define PUBLISH_RETRY_MS                         ( 1000 )

#define MAX_MSG_SIZE                             400
#define MAX_DUID_BYTE_LEN                        30
#define MAX_TOPIC_LEN                            80
#define MAX_JOB_TYPE_LEN                         100
#define MAX_UPDATE_LINK_LEN                      150

#define ARBITRARY_TIMEOUT_MINS                   100

#define PUBLISH_TOPICS_MAX_IDX                   2
#define START_NEXT_JOB_IDX                       3
#define START_NEXT_ACC_REJ_IDX                   4

//Received message types
typedef enum
{
    NEW_JOB,
    GET_NEW_JOBS,
    NO_NEW_JOBS,
    SEND_SENSOR_DATA_MSG,
    SEND_GPS_DATA_MSG,
    SEND_STATUS_MSG,
    SEND_JOB_PASS,
    SEND_JOB_FAIL,
    SEND_JOB_FAIL_BAD_CONFIGS,
}mqttMsgId_t;

typedef union
{
    IotMqttPublishInfo_t mqttPulishInfo;
    //add more here if needed
}mqttQueuePayload_t;

typedef struct
{
    mqttMsgId_t eventID;
    mqttQueuePayload_t payload;
}mqttMsg_t;

//request types in the start next job topic
typedef enum
{
    RESET_ALARMS,
    ACTIVATE,
    DEACTIVATE,
    CONFIGURE,
    RESET,
    GPS,
    UPDATE,
    HW_RESET,
    UNKNOWN
}jobRequestType_t;

//Contains the different aws job states
typedef enum {
    IDLE,
    IN_PROGRESS,
    QUEUED,
    FAILED,
    SUCCEEDED,
    CANCELED,
    TIMED_OUT,
    REJECTED,
    REMOVED
}awsJobStatus_t;

//used for jobs Encoding
typedef struct
{
    IotSerializerEncoderObject_t object; /* Encoder object handle. */
    uint8_t * pDataBuffer;               /* Raw data buffer to be published with MQTT. */
    size_t size;                         /* Raw data size. */
} outgoingJobMessageType_t;


const IotSerializerEncodeInterface_t * awsJobsJsonEncoder = NULL;

/* Flags for tracking which cleanup functions must be called. */
static bool librariesInitialized = false;
static bool connectionEstablished = false;
static bool xAwsIotMqttMode = true;
const char * xThingName;
void *xpNetworkServerInfo;
void * xpNetworkCredentialInfo;
const IotNetworkInterface_t * xpNetworkInterface;

static uint8_t xDuid[MAX_DUID_BYTE_LEN];
static uint8_t xDuidLength = 0u;
static uint8_t payloadBufferForOutgoingJob[MAX_MSG_SIZE];
static uint8_t payloadBufferOutgoingTopic[MAX_MSG_SIZE];

//job msg encoding struct
static outgoingJobMessageType_t jsonEncodedRsp =
{
    .object      = IOT_SERIALIZER_ENCODER_CONTAINER_INITIALIZER_STREAM,
    .pDataBuffer = payloadBufferForOutgoingJob,
    .size        = MAX_MSG_SIZE,
};

//queue to unblock this task
QueueHandle_t mqttQueue;

//task handle
TaskHandle_t xMqttHandle;

/* Handle of the MQTT connection */
IotMqttConnection_t mqttConnection = IOT_MQTT_CONNECTION_INITIALIZER;

static char statusTopic[MAX_TOPIC_LEN];
static char gpsTopic[MAX_TOPIC_LEN];
static char sensorDataTopic[MAX_TOPIC_LEN];

//jobs related topics
static char startNextJobTopic[MAX_TOPIC_LEN];
static char startNextAccRejJobTopic[MAX_TOPIC_LEN];
static char xCurrentJobId[MAX_JOB_TYPE_LEN];
static awsJobStatus_t xCurrentJobStatus = IDLE;
static uint32_t xVersionNum = 0u;
static jobRequestType_t xCurrentJobRequest = UNKNOWN;
static mqttMsg_t xMqttQueueEvt;
static configureMsg_t configsReceived = {};
static bool deactivateWithHwReset = false;
static bool takeNewGpsMeasurement = false;
static char fwUpdateLink[MAX_UPDATE_LINK_LEN] = {};
static bool xCloudTxInProgress = false;

/* Topics, will be initialized with the DUID provided */
const char * pTopicStrings[ TOPIC_FILTER_COUNT ] =
{
    statusTopic,
    gpsTopic,
    sensorDataTopic,
    startNextJobTopic,
    startNextAccRejJobTopic,
};

static void xOperationCompleteCb( void * param1,
                                        IotMqttCallbackParam_t * const pOperation);

static void xSubscriptionCb( void * param1,
                                       IotMqttCallbackParam_t * const pPublish);

static void xStartNextJobCb( void * param1,
                                       IotMqttCallbackParam_t * const pPublish);

static void xStartNextJobAcceptRejectCb( void * param1,
                                       IotMqttCallbackParam_t * const pPublish);

static int xNewMqttConnection( bool awsIotMqttMode,
                                     const char * pIdentifier,
                                     void * pNetworkServerInfo,
                                     void * pNetworkCredentialInfo,
                                     const IotNetworkInterface_t * pNetworkInterface,
                                     IotMqttConnection_t * pMqttConnection);

static int xChangeSubscriptions( IotMqttConnection_t mqttConnection,
                                 IotMqttOperationType_t operation,
                                 const char ** pTopicFilters,
                                 void * pCallbackParameter);

static bool xPublishMessage( IotMqttConnection_t mqttConnection,
                            IotMqttPublishInfo_t publishInfo);

static int xDisconnectAndCleanUp(void);

static uint32_t xEncodeStatusMessagePayload(StatusMessage message, uint8_t *buf, uint16_t bufLen);
static uint32_t xEncodeGpsMessagePayload(GpsMessage message, uint8_t *buf, uint16_t bufLen);
static uint32_t xEncodeSensorDataMessagePayload(SensorDataMessage message, uint8_t *buf, uint16_t bufLen);
static bool jsonEncodeJobUpdateMessage(awsJobStatus_t jobStat, jobRequestType_t jobRequest, uint32_t expectedVersion, uint32_t stepTimeoutMins, char *clientToken, bool valid);
static bool xSendJobUpdate(char * jobId, awsJobStatus_t jobStat, jobRequestType_t jobRequest, uint32_t expectedVersion, uint32_t stepTimeoutMins, char *clientToken, bool valid);
static bool xSendGetNextJobReq(void);

static const char *xJobStatusToString(awsJobStatus_t status);
static const char *xRequestTypeToString(jobRequestType_t jobRequest);
static jobRequestType_t xStringToRequestType(const char * requestTypeStr);
static void xSetTxOperationIp(bool inProgress);


int MQTT_init( bool awsIotMqttMode,
        void * pNetworkServerInfo,
        void * pNetworkCredentialInfo,
        const IotNetworkInterface_t * pNetworkInterface, uint8_t *duid, uint8_t duidLength)
{
    int status = EXIT_FAILURE;

    //check DUID param
    if (duidLength <= MAX_DUID_BYTE_LEN)
    {
        // set up init network info and DUID
        xAwsIotMqttMode = awsIotMqttMode;
        xThingName = (char*) xDuid;
        xpNetworkServerInfo = pNetworkServerInfo;
        xpNetworkCredentialInfo = pNetworkCredentialInfo;
        xpNetworkInterface = pNetworkInterface;

        memcpy(&xDuid, duid, duidLength);
        xDuidLength = duidLength;

        //create the task to handle MQTT
        if( xTaskCreate( MQTT_task, "mqtt_thread", ( configSTACK_DEPTH_TYPE ) 64*24, NULL, 7, &xMqttHandle ) != pdPASS )
        {
            elogError("Failed to create mqtt task");
        }
        else
        {
            elogDebug("Created mqtt task");
            status = EXIT_SUCCESS;
        }
    }
    else
    {
        elogError("INVALID DUID");
    }

    return status;
}

void MQTT_task()
{
    int status = EXIT_SUCCESS;

    /* Initialize the MQTT related libraries */
    status = IotMqtt_Init();

    //init encoder
    awsJobsJsonEncoder = &_IotSerializerJsonEncoder;

    if( status == EXIT_SUCCESS )
    {
        /* Mark the libraries as initialized. */
        librariesInitialized = true;

        /* Establish a new MQTT connection */
        status = xNewMqttConnection( xAwsIotMqttMode,
                                     xThingName,
                                     xpNetworkServerInfo,
                                     xpNetworkCredentialInfo,
                                     xpNetworkInterface,
                                     &mqttConnection );
    }

    if( status == EXIT_SUCCESS )
    {
        /* Mark the MQTT connection as established. */
        connectionEstablished = true;

        //using the DUID, set up the topic subscribes
        sprintf((char*)statusTopic, "async/%s/status", (char*)xDuid);
        sprintf((char*)gpsTopic, "async/%s/gps", (char*)xDuid);
        sprintf((char*)sensorDataTopic, "async/%s/sensor_data", (char*)xDuid);

        //job related
        sprintf((char*)startNextJobTopic, "$aws/things/%s/jobs/start-next", (char*)xDuid);
        sprintf((char*)startNextAccRejJobTopic, "$aws/things/%s/jobs/start-next/#", (char*)xDuid);

        /* Add the topic filter subscriptions */
        status = xChangeSubscriptions( mqttConnection, IOT_MQTT_SUBSCRIBE, pTopicStrings, NULL);
    }

    if ( status == EXIT_SUCCESS )
    {
        //Message queue for incoming mqtt messages
        mqttQueue = xQueueCreate( 3, sizeof( mqttMsg_t ) );

        if ( mqttQueue == NULL )
        {
            elogError("FAILED to create mqtt msg queue");
        }
        else
        {
            //indicate to the event manager that we are connected to the cloud and available to send messages
            EVT_indicateMqttReady();
        }
    }

    if ( status != EXIT_SUCCESS)
    {
        elogError("Failed to connect to mqtt or init the mqtt task - deleting task");

        //pass up the error
        EVT_indicateCloudConnectFailure();

        vTaskDelete(xMqttHandle);
    }

    while (1)
    {
        //block on waiting for jobs.messages to send
        if( xQueueReceive( mqttQueue, &( xMqttQueueEvt ), ( TickType_t ) 50 ) == pdPASS )
        {
            switch(xMqttQueueEvt.eventID)
            {
                case NEW_JOB:

                    if ( xCurrentJobRequest == UNKNOWN)
                    {
                        xCurrentJobStatus = FAILED;

                        //send the update
                        xSendJobUpdate(xCurrentJobId, xCurrentJobStatus, xCurrentJobRequest, xVersionNum, ARBITRARY_TIMEOUT_MINS, (char*)&xDuid, true);

                        //put state back to idle
                        xCurrentJobStatus = IDLE;
                        memset(&xCurrentJobId, 0, sizeof(xCurrentJobId));
                        xVersionNum = 0u;
                    }
                    else
                    {
                        // update state and pass to event handler
                        xCurrentJobStatus = IN_PROGRESS;

                        switch(xCurrentJobRequest)
                        {
                           case ACTIVATE:
                               EVT_indicateActivateFromCloud();
                               break;
                           case DEACTIVATE:
                               EVT_indicateDeActivate();
                               break;
                           case CONFIGURE:
                               EVT_indicateNewConfigMessage(configsReceived);
                               break;
                           case UPDATE:
                               EVT_indicateOta((char*)&fwUpdateLink);
                               break;
                           case HW_RESET:
                               EVT_indicateHwResetCmd(deactivateWithHwReset);
                               break;
                           case GPS:
                               EVT_indicateGpsLocationRequested(takeNewGpsMeasurement);
                               break;
                           case RESET_ALARMS:
                               EVT_indicateResetAlarmsCmd();
                               break;
                           default:
                               break;
                        }
                    }
                    break;
                case GET_NEW_JOBS:
                    //check for any jobs
                    xSendGetNextJobReq();
                    break;
                case NO_NEW_JOBS:
                    //no jobs, the event handler will start power down and turn off cell
                    EVT_indicateNoNewJobsFromCloud();
                    break;
                case SEND_JOB_PASS:

                    xCurrentJobStatus = SUCCEEDED;

                    //send the update to the cloud
                    xSendJobUpdate(xCurrentJobId, xCurrentJobStatus, xCurrentJobRequest, xVersionNum, ARBITRARY_TIMEOUT_MINS, (char*)&xDuid, true);

                    //put state back to idle
                    xCurrentJobStatus = IDLE;
                    memset(&xCurrentJobId, 0, sizeof(xCurrentJobId));
                    xVersionNum = 0u;

                    break;

                //generic job failed message
                case SEND_JOB_FAIL:
                    xCurrentJobStatus = FAILED;

                    //send the update to the cloud
                    xSendJobUpdate(xCurrentJobId, xCurrentJobStatus, xCurrentJobRequest, xVersionNum, ARBITRARY_TIMEOUT_MINS, (char*)&xDuid, true);

                    //put state back to idle
                    xCurrentJobStatus = IDLE;
                    memset(&xCurrentJobId, 0, sizeof(xCurrentJobId));
                    xVersionNum = 0u;
                    break;

                //config job failed due to bad parameters
                case SEND_JOB_FAIL_BAD_CONFIGS:
                    xCurrentJobStatus = FAILED;

                    //send the update to the cloud
                    xSendJobUpdate(xCurrentJobId, xCurrentJobStatus, xCurrentJobRequest, xVersionNum, ARBITRARY_TIMEOUT_MINS, (char*)&xDuid, false);

                    //put state back to idle
                    xCurrentJobStatus = IDLE;
                    memset(&xCurrentJobId, 0, sizeof(xCurrentJobId));
                    xVersionNum = 0u;
                    break;

                case SEND_STATUS_MSG:
                    //Send out the message
                    xPublishMessage(mqttConnection, xMqttQueueEvt.payload.mqttPulishInfo);
                    break;

                case SEND_GPS_DATA_MSG:
                    xPublishMessage(mqttConnection, xMqttQueueEvt.payload.mqttPulishInfo);
                    break;

                case SEND_SENSOR_DATA_MSG:
                    // Send out sensor data message
                    xPublishMessage(mqttConnection, xMqttQueueEvt.payload.mqttPulishInfo);
                    break;

                default:
                    break;

            }
        }
    }
}

//return if we are sending a cloud message or waiting on a response
bool MQTT_getOperationInProgressFlag(void)
{
    return xCloudTxInProgress;
}

static void xSetTxOperationIp(bool inProgress)
{
    xCloudTxInProgress = inProgress;
}

bool MQTT_sendStatusMsg(StatusMessage statusToSend)
{
    bool status = false;
    uint32_t lenEncoded = 0;
    IotMqttPublishInfo_t publishInfo = IOT_MQTT_PUBLISH_INFO_INITIALIZER;
    mqttMsg_t msg;

    memset(&payloadBufferOutgoingTopic, 0, MAX_MSG_SIZE);

    //encode the status payload per our mqtt protocol
    lenEncoded = xEncodeStatusMessagePayload(statusToSend, payloadBufferOutgoingTopic, MAX_MSG_SIZE);

    if (lenEncoded > 0 )
    {
        status = true;

        //set up the publish
        publishInfo.qos = IOT_MQTT_QOS_0;
        publishInfo.pTopicName = pTopicStrings[0];
        publishInfo.topicNameLength = strlen(pTopicStrings[0]);

        //fill in the payload with the protobuf
        publishInfo.pPayload = &payloadBufferOutgoingTopic;
        publishInfo.payloadLength = lenEncoded;
        publishInfo.retryMs = PUBLISH_RETRY_MS;
        publishInfo.retryLimit = PUBLISH_RETRY_LIMIT;

        //queue up the msg to be sent out
        msg.eventID = SEND_STATUS_MSG;
        msg.payload.mqttPulishInfo = publishInfo;

        elogInfo("queued status msg");

        xQueueSend(mqttQueue, &msg, ( TickType_t ) QUEUE_WAIT_TIME_MS );
    }

    return status;
}

bool MQTT_sendGpsMsg(GpsMessage gpsMsgToSend)
{
    bool status = false;
    uint32_t lenEncoded = 0;
    IotMqttPublishInfo_t publishInfo = IOT_MQTT_PUBLISH_INFO_INITIALIZER;
    mqttMsg_t msg;

    memset(&payloadBufferOutgoingTopic, 0, MAX_MSG_SIZE);

    //encode the status payload per our mqtt protocol
    lenEncoded = xEncodeGpsMessagePayload(gpsMsgToSend, payloadBufferOutgoingTopic, MAX_MSG_SIZE);

    if (lenEncoded > 0 )
    {
        status = true;

        //set up the publish
        publishInfo.qos = IOT_MQTT_QOS_0;
        publishInfo.pTopicName = pTopicStrings[1];
        publishInfo.topicNameLength = strlen(pTopicStrings[1]);

        //fill in the payload with the protobuf
        publishInfo.pPayload = &payloadBufferOutgoingTopic;
        publishInfo.payloadLength = lenEncoded;
        publishInfo.retryMs = PUBLISH_RETRY_MS;
        publishInfo.retryLimit = PUBLISH_RETRY_LIMIT;

        //queue up the msg to be sent out
        msg.eventID = SEND_GPS_DATA_MSG;
        msg.payload.mqttPulishInfo = publishInfo;

        elogInfo("queued GPS msg");

        xQueueSend(mqttQueue, &msg, ( TickType_t ) QUEUE_WAIT_TIME_MS );
    }

    return status;
}


bool MQTT_sendSensorDataMsg(SensorDataMessage sensorDataMessageToSend)
{
    bool status = false;
    uint32_t lenEncoded = 0;
    IotMqttPublishInfo_t publishInfo = IOT_MQTT_PUBLISH_INFO_INITIALIZER;
    mqttMsg_t msg;

    memset(&payloadBufferOutgoingTopic, 0, MAX_MSG_SIZE);

    //encode the sensor data payload per our mqtt protocol
    lenEncoded = xEncodeSensorDataMessagePayload(sensorDataMessageToSend, payloadBufferOutgoingTopic, MAX_MSG_SIZE);

    if (lenEncoded > 0 )
    {
        status = true;

        //set up the publish
        publishInfo.qos = IOT_MQTT_QOS_1;
        publishInfo.pTopicName = pTopicStrings[2];
        publishInfo.topicNameLength = strlen(pTopicStrings[2]);

        //fill in the payload with the protobuf
        publishInfo.pPayload = &payloadBufferOutgoingTopic;
        publishInfo.payloadLength = lenEncoded;
        publishInfo.retryMs = PUBLISH_RETRY_MS;
        publishInfo.retryLimit = PUBLISH_RETRY_LIMIT;

        //queue up the msg to be sent out
        msg.eventID = SEND_SENSOR_DATA_MSG;
        msg.payload.mqttPulishInfo = publishInfo;

        elogInfo("queued sensor data msg");

        xQueueSend(mqttQueue, &msg, ( TickType_t ) QUEUE_WAIT_TIME_MS );
    }

    return status;
}

void MQTT_indicateOperationPass(void)
{
    mqttMsg_t msg;

    //job passed
    msg.eventID = SEND_JOB_PASS;
    xQueueSend(mqttQueue, &msg, ( TickType_t ) QUEUE_WAIT_TIME_MS );
}

void MQTT_indicateOperationFail(void)
{
    mqttMsg_t msg;

    //job failed
    msg.eventID = SEND_JOB_FAIL;
    xQueueSend(mqttQueue, &msg, ( TickType_t ) QUEUE_WAIT_TIME_MS );
}

void MQTT_indicateBadConfigs(void)
{
    mqttMsg_t msg;

    //job failed
    msg.eventID = SEND_JOB_FAIL_BAD_CONFIGS;
    xQueueSend(mqttQueue, &msg, ( TickType_t ) QUEUE_WAIT_TIME_MS );
}

void MQTT_IndicateReadyForNewJobs(void)
{
    mqttMsg_t msg;

    //job failed
    msg.eventID = GET_NEW_JOBS;
    xQueueSend(mqttQueue, &msg, ( TickType_t ) QUEUE_WAIT_TIME_MS );
}

bool MQTT_disconnect(void)
{
    return xDisconnectAndCleanUp();
}

static uint32_t xEncodeStatusMessagePayload(StatusMessage message, uint8_t *buf, uint16_t bufLen)
{
    uint32_t msgLen;
    bool status;

    /* Create a stream that will write to our buffer. */
    pb_ostream_t stream = pb_ostream_from_buffer(buf, bufLen);

    /* Now we are ready to encode the message */
    status = pb_encode(&stream, StatusMessage_fields, &message);
    msgLen = stream.bytes_written;

    /* Then check for any errors.. */
    if ( status == false )
    {
       elogError("Encoding failed: %s\n", PB_GET_ERROR(&stream));
       msgLen = 0;
    }

    //return the message length
    return msgLen;
}

static uint32_t xEncodeGpsMessagePayload(GpsMessage message, uint8_t *buf, uint16_t bufLen)
{
    uint32_t msgLen;
    bool status;

    /* Create a stream that will write to our buffer. */
    pb_ostream_t stream = pb_ostream_from_buffer(buf, bufLen);

    /* Now we are ready to encode the message */
    status = pb_encode(&stream, GpsMessage_fields, &message);
    msgLen = stream.bytes_written;

    /* Then check for any errors.. */
    if ( status == false )
    {
       elogError("Encoding failed: %s\n", PB_GET_ERROR(&stream));
       msgLen = 0;
    }

    //return the message length
    return msgLen;
}

static uint32_t xEncodeSensorDataMessagePayload(SensorDataMessage message, uint8_t *buf, uint16_t bufLen)
{
    uint32_t msgLen;
    bool status;

    /* Create a stream that will write to our buffer. */
    pb_ostream_t stream = pb_ostream_from_buffer(buf, bufLen);

    /* Now we are ready to encode the message */
    status = pb_encode(&stream, SensorDataMessage_fields, &message);
    msgLen = stream.bytes_written;

    /* Then check for any errors.. */
    if ( status == false )
    {
       elogError("Encoding failed: %s\n", PB_GET_ERROR(&stream));
       msgLen = 0;
    }

    //return the message length
    return msgLen;
}

static int xDisconnectAndCleanUp()
{
    int status = EXIT_SUCCESS;

    if( status == EXIT_SUCCESS )
    {
        /* Remove the topic  subscriptions */
        status = xChangeSubscriptions( mqttConnection, IOT_MQTT_UNSUBSCRIBE, pTopicStrings, NULL );
    }

    /* Disconnect the MQTT connection if it was established. */
    if( connectionEstablished == true )
    {
        IotMqtt_Disconnect( mqttConnection, 0 );
    }

    /* Clean up libraries if they were initialized. */
    if( librariesInitialized == true )
    {
        IotMqtt_Cleanup();
    }

    //delete the task!
    vTaskDelete(xMqttHandle);

    return status;
}

static void xOperationCompleteCb( void * param1, IotMqttCallbackParam_t * const pOperation )
{
    //Reset operation in progress flag
    xSetTxOperationIp(false);

    /* Print the status of the completed operation. A PUBLISH operation is
     * successful when transmitted over the network. */
    if( pOperation->u.operation.result == IOT_MQTT_SUCCESS )
    {
        elogInfo( "MQTT %s successfully sent.", IotMqtt_OperationType( pOperation->u.operation.type));
        EVT_indicateMqttPublishSuccess();
    }
    else
    {
        elogInfo( "MQTT %s could not be sent. Error %s.", IotMqtt_OperationType( pOperation->u.operation.type ),IotMqtt_strerror( pOperation->u.operation.result ) );
    }
}

//Called by the MQTT library when an incoming PUBLISH message is received.
//we will use jobs for incoming messages so this is just for debugging
static void xSubscriptionCb( void * param1, IotMqttCallbackParam_t * const pPublish )
{
    /* Print information about the incoming PUBLISH message. */
    elogDebug( "Incoming PUBLISH received:\r\n"
                "Subscription topic filter: %.*s\r\n"
                "Publish topic name: %.*s\r\n"
                "Publish retain flag: %d\r\n"
                "Publish QoS: %d\r\n"
                "Publish payload len : %d",
                pPublish->u.message.topicFilterLength,
                pPublish->u.message.pTopicFilter,
                pPublish->u.message.info.topicNameLength,
                pPublish->u.message.info.pTopicName,
                pPublish->u.message.info.retain,
                pPublish->u.message.info.qos,
                pPublish->u.message.info.payloadLength
              );

    elogDebug("rcd %s", pPublish->u.message.info.pPayload);
}

//Doesnt appear that this is ever called. Instead, the accept/reject cb is used
static void xStartNextJobCb( void * param1, IotMqttCallbackParam_t * const pPublish)
{
    elogInfo("start next callback!");
}

static void xStartNextJobAcceptRejectCb( void * param1, IotMqttCallbackParam_t * const pPublish)
{
    //decode response
    bool found = false;
    const char * executionSection = NULL;
    const char * jobId = NULL;
    const char * jobDocument = NULL;
    const char * requestType = NULL;
    const char * versionNum = NULL;
    const char * transRate = NULL;
    const char * numSatellites = NULL;
    const char * maxHop = NULL;
    const char * minMeasTime = NULL;
    const char * gpsTimeout = NULL;
    const char * strokeAlgOn = NULL;
    const char * fwLink = NULL;
    const char * deactFlag = NULL;
    const char * gpsFlag = NULL;
    const char * redFlagOn = NULL;
    const char * redFlagOff = NULL;
    size_t execSize = 0;
    size_t sizeId = 0;
    size_t sizeDocument = 0;
    size_t sizeReqType = 0;
    size_t sizeVersionNum = 0;
    size_t sizeTransRate = 0;
    size_t sizeNumSat = 0;
    size_t sizeMaxHop = 0;
    size_t sizeminMeasTime = 0;
    size_t sizeGpsTimeout = 0;
    size_t sizeStrokeAlgOn = 0;
    size_t sizeFwLink = 0;
    size_t sizeDeactFlag = 0;
    size_t SizeGpsFlag = 0;
    size_t sizeRedFlagOn = 0;
    size_t sizeRedFlagOff = 0;
    mqttMsg_t msg;
    char tempBuffer[50] = {0};
    char *tempPtr;

    elogInfo("start next callback- Accepted/rejected!");

    //if there is a new job, get the request type
    if(strstr(pPublish->u.message.info.pTopicName, "accepted") != NULL)
    {
        elogInfo("Get Next Accepted, parse job: ");

        //Check first if we are already doing something with a job - dont start if not finished
        if ( xCurrentJobStatus == IDLE )
        {
              // Find the "execution" key in the payload
               found = IotJsonUtils_FindJsonValue( pPublish->u.message.info.pPayload,
                                                   pPublish->u.message.info.payloadLength,
                                                   "execution",
                                                   9,
                                                   &executionSection,
                                                   &execSize );

               //need 4 things - the job ID , version, job document and finally the request type (within job doc)
               if( found == true )
               {
                    //first look for job ID
                    found = IotJsonUtils_FindJsonValue(executionSection, execSize, "jobId", 5, &jobId, &sizeId);

                    if (found == true )
                    {
                        //save job ID
                        //elogInfo("Found job ID %s", jobId);

                        //now look for version number
                        found = IotJsonUtils_FindJsonValue(executionSection, execSize, "versionNumber", 13, &versionNum, &sizeVersionNum);

                        if (found == true)
                        {
                            //elogInfo("Found version num %s", versionNum);

                            //now look for job document
                            found = IotJsonUtils_FindJsonValue(executionSection, execSize, "jobDocument", 11, &jobDocument, &sizeDocument);

                           if (found == true)
                           {
                               //now get request type from within the job doc!
                               found = IotJsonUtils_FindJsonValue(jobDocument, sizeDocument, "requestType", 11, &requestType, &sizeReqType);

                               if (found == true )
                               {
                                   //set the static variables to the new job data:

                                   //increase by 1 to get rid of "
                                   jobId++;
                                   requestType++;

                                   //subtract 2 from the length since we dont want ""
                                   memcpy(&xCurrentJobId,jobId , (sizeId-2));
                                   memcpy(&tempBuffer, requestType, (sizeReqType-2));

                                   //set current job request type
                                   xCurrentJobRequest = xStringToRequestType(tempBuffer);

                                   memset(&tempBuffer, 0, sizeof(tempBuffer));
                                   memcpy(&tempBuffer, versionNum, sizeVersionNum);
                                   xVersionNum = strtol(tempBuffer, &tempPtr, 10);

                                   //get the rest of the contents if this is a message w/ payload
                                   if ( xCurrentJobRequest == CONFIGURE )
                                   {
                                       //now get request type from within the job doc!
                                       found = IotJsonUtils_FindJsonValue(jobDocument, sizeDocument, "transmissionRate", strlen("transmissionRate"), &transRate, &sizeTransRate);
                                       found = IotJsonUtils_FindJsonValue(jobDocument, sizeDocument, "numOfSatellites", strlen("numOfSatellites"), &numSatellites, &sizeNumSat);
                                       found = IotJsonUtils_FindJsonValue(jobDocument, sizeDocument, "maxHdop", strlen("maxHdop"), &maxHop, &sizeMaxHop);
                                       found = IotJsonUtils_FindJsonValue(jobDocument, sizeDocument, "minMeasTime", strlen("minMeasTime"), &minMeasTime, &sizeminMeasTime);
                                       found = IotJsonUtils_FindJsonValue(jobDocument, sizeDocument, "gpsTimeout", strlen("gpsTimeout"), &gpsTimeout, &sizeGpsTimeout);
                                       found = IotJsonUtils_FindJsonValue(jobDocument, sizeDocument, "strokeDetection", strlen("strokeDetection"), &strokeAlgOn, &sizeStrokeAlgOn);
                                       found = IotJsonUtils_FindJsonValue(jobDocument, sizeDocument, "redFlagOnThreshold", strlen("redFlagOnThreshold"), &redFlagOn, &sizeRedFlagOn);
                                       found = IotJsonUtils_FindJsonValue(jobDocument, sizeDocument, "redFlagOffThreshold", strlen("redFlagOffThreshold"), &redFlagOff, &sizeRedFlagOff);

                                       //set up the static configs payload by converting each string value to an integer:
                                       memset(&tempBuffer, 0, sizeof(tempBuffer));
                                       memcpy(&tempBuffer, transRate, sizeTransRate);
                                       configsReceived.transmissionRateDays = strtol(tempBuffer, &tempPtr, 10);

                                       memset(&tempBuffer, 0, sizeof(tempBuffer));
                                       memcpy(&tempBuffer, numSatellites, sizeNumSat);
                                       configsReceived.numOfSatellites = strtol(tempBuffer, &tempPtr, 10);


                                       memset(&tempBuffer, 0, sizeof(tempBuffer));
                                       memcpy(&tempBuffer, maxHop, sizeMaxHop);
                                       configsReceived.maxHop = strtol(tempBuffer, &tempPtr, 10);


                                       memset(&tempBuffer, 0, sizeof(tempBuffer));
                                       memcpy(&tempBuffer, minMeasTime, sizeminMeasTime);
                                       configsReceived.minMeasureTime = strtol(tempBuffer, &tempPtr, 10);


                                       memset(&tempBuffer, 0, sizeof(tempBuffer));
                                       memcpy(&tempBuffer, gpsTimeout, sizeGpsTimeout);
                                       configsReceived.gpsTimeoutSeconds = strtol(tempBuffer, &tempPtr, 10);

                                       memset(&tempBuffer, 0, sizeof(tempBuffer));
                                       memcpy(&tempBuffer, strokeAlgOn, sizeStrokeAlgOn);

                                       //returns 0 if the 2 strings are the same
                                       if ( strcmp("true", (const char*)&tempBuffer) == 0)
                                       {
                                           configsReceived.strokeAlgIsOn = true;
                                       }
                                       else
                                       {
                                           configsReceived.strokeAlgIsOn = false;
                                       }

                                       memset(&tempBuffer, 0, sizeof(tempBuffer));
                                       memcpy(&tempBuffer, redFlagOn, sizeRedFlagOn);
                                       configsReceived.redFlagOnThreshold = strtol(tempBuffer, &tempPtr, 10);

                                       memset(&tempBuffer, 0, sizeof(tempBuffer));
                                       memcpy(&tempBuffer, redFlagOff, sizeRedFlagOff);
                                       configsReceived.redFlagOffThreshold = strtol(tempBuffer, &tempPtr, 10);

                                       elogInfo("config:  %d, %lu, %lu, %lu, %lu, %d, %lu, %lu",configsReceived.transmissionRateDays, configsReceived.numOfSatellites,
                                               configsReceived.maxHop,configsReceived.minMeasureTime,configsReceived.gpsTimeoutSeconds,
                                               configsReceived.strokeAlgIsOn, configsReceived.redFlagOnThreshold, configsReceived.redFlagOffThreshold );
                                   }
                                   else if ( xCurrentJobRequest == UPDATE )
                                   {
                                       found = IotJsonUtils_FindJsonValue(jobDocument, sizeDocument, "firmwareUpdate", strlen("firmwareUpdate"), &fwLink, &sizeFwLink);

                                       fwLink++;
                                       memcpy(&fwUpdateLink, fwLink , (sizeFwLink-2));

                                       elogInfo("Fw update URL: %s", fwUpdateLink);
                                   }
                                   else if ( xCurrentJobRequest == HW_RESET )
                                   {
                                       found = IotJsonUtils_FindJsonValue(jobDocument, sizeDocument, "deactivate", strlen("deactivate"), &deactFlag, &sizeDeactFlag);

                                       //set up the static configs payload by converting each string value to an integer:
                                       memset(&tempBuffer, 0, sizeof(tempBuffer));
                                       memcpy(&tempBuffer, deactFlag, sizeDeactFlag);

                                       //returns 0 if the 2 strings are the same
                                       if ( strcmp("true", (const char*)&tempBuffer) == 0)
                                       {
                                           deactivateWithHwReset = true;
                                       }
                                       else
                                       {
                                           deactivateWithHwReset = false;
                                       }

                                       elogInfo("Hw reset command, deactivate: %d", deactivateWithHwReset);
                                   }
                                   else if ( xCurrentJobRequest == GPS )
                                   {
                                        found = IotJsonUtils_FindJsonValue(jobDocument, sizeDocument, "newMeasurement", strlen("newMeasurement"), &gpsFlag, &SizeGpsFlag);

                                        //copy the string into a temporary buffer
                                        memset(&tempBuffer, 0, sizeof(tempBuffer));
                                        memcpy(&tempBuffer, gpsFlag, SizeGpsFlag);

                                        //returns 0 if the 2 strings are the same. Convert the string to boolean type
                                        if ( strcmp("true", (const char*)&tempBuffer) == 0)
                                        {
                                            takeNewGpsMeasurement = true;
                                        }
                                        else
                                        {
                                            takeNewGpsMeasurement = false;
                                        }

                                        elogInfo("GPS job received, new measurement: %d", takeNewGpsMeasurement);
                                   }

                                   elogInfo("New job parsed successfully");

                                   //send the job update msg, in progress, then pass to mqtt handler
                                   msg.eventID = NEW_JOB;
                                   xQueueSend(mqttQueue, &msg, ( TickType_t ) QUEUE_WAIT_TIME_MS );
                               }
                               else
                               {
                                   elogError("Did NOT find request type");
                               }
                           }
                           else
                           {
                               elogError("Did NOT find job document");
                           }
                    }
                    else
                    {
                        elogError("Did NOT find version number");
                    }
                }
                else
                {
                    elogError("Did NOT find job id");
                }
            }
            else
            {
                //this also means no new job, pass to mqtt handler
                elogInfo( "No\"execution\" in job message" );
                msg.eventID = NO_NEW_JOBS;
                xQueueSend(mqttQueue, &msg, ( TickType_t ) QUEUE_WAIT_TIME_MS );
            }
        }
        else
        {
            //if a job is already in progress we will just wait until its finished to get the next one
            elogInfo("Already a job in progress!");
        }
    }
    else
    {
         elogInfo("Rejected - no new job");

         //pass to mqtt handler
         msg.eventID = NO_NEW_JOBS;
         xQueueSend(mqttQueue, &msg, ( TickType_t ) QUEUE_WAIT_TIME_MS );
    }
}

/* Encode job update with this form
 *
 * This one is for jobs that do not have any extra payload besides request type
 * {
        "status": "job-execution-state",
        "statusDetails": {
            "string": "string"
            ...
        },
        "expectedVersion": "number",
        "stepTimeoutInMinutes": long,
        "clientToken": "string"
}
 */
static bool jsonEncodeJobUpdateMessage( awsJobStatus_t jobStat, jobRequestType_t jobRequest, uint32_t expectedVersion, uint32_t stepTimeoutMins, char *clientToken, bool valid )
{
    IotSerializerError_t serializerError = IOT_SERIALIZER_SUCCESS;
    bool stat = EXIT_FAILURE;
    IotSerializerEncoderObject_t * pEncoderObject = &( jsonEncodedRsp.object );

    IotSerializerEncoderObject_t updateJobExecutionMap = IOT_SERIALIZER_ENCODER_CONTAINER_INITIALIZER_MAP;
    IotSerializerEncoderObject_t statusDetails = IOT_SERIALIZER_ENCODER_CONTAINER_INITIALIZER_MAP;


    serializerError = awsJobsJsonEncoder->init( pEncoderObject, jsonEncodedRsp.pDataBuffer, jsonEncodedRsp.size );


    // Create the outermost map with 5 keys: status, statusDetails, expectedVersion, steptimeoutInMinutes, clientToken
    serializerError = awsJobsJsonEncoder->openContainer( pEncoderObject, &updateJobExecutionMap, 5 );

    //set up each entry
     serializerError = awsJobsJsonEncoder->appendKeyValue( &updateJobExecutionMap,
                                                                 "status",
                                                                 IotSerializer_ScalarTextString(xJobStatusToString(jobStat))
                                                                 );

    //copy configs into the status details section
    if ( jobRequest == CONFIGURE)
    {
        //only need one item for status details:
        serializerError = awsJobsJsonEncoder->openContainerWithKey( &updateJobExecutionMap,
                                                                            "statusDetails",
                                                                            &statusDetails,
                                                                            7 );
        serializerError = awsJobsJsonEncoder->appendKeyValue( &statusDetails,
                                                                       "requestType",
                                                                         IotSerializer_ScalarTextString(xRequestTypeToString(jobRequest)) );

        serializerError = awsJobsJsonEncoder->appendKeyValue( &statusDetails,
                                                                       "transmissioRate",
                                                                       IotSerializer_ScalarSignedInt( configsReceived.transmissionRateDays));

        serializerError = awsJobsJsonEncoder->appendKeyValue( &statusDetails,
                                                                       "numOfSatellites",
                                                                       IotSerializer_ScalarSignedInt( configsReceived.numOfSatellites));


        serializerError = awsJobsJsonEncoder->appendKeyValue( &statusDetails,
                                                                       "maxHdop",
                                                                       IotSerializer_ScalarSignedInt( configsReceived.maxHop));

        serializerError = awsJobsJsonEncoder->appendKeyValue( &statusDetails,
                                                                       "minMeasTime",
                                                                       IotSerializer_ScalarSignedInt( configsReceived.minMeasureTime));

        serializerError = awsJobsJsonEncoder->appendKeyValue( &statusDetails,
                                                                       "gpsTimeout",
                                                                       IotSerializer_ScalarSignedInt( configsReceived.gpsTimeoutSeconds));

        if ( configsReceived.strokeAlgIsOn == true )
        {
            serializerError = awsJobsJsonEncoder->appendKeyValue( &statusDetails,
                                                                        "strokeDetection",
                                                                        IotSerializer_ScalarTextString("true"));
        }
        else
        {
            serializerError = awsJobsJsonEncoder->appendKeyValue( &statusDetails,
                                                                        "strokeDetection",
                                                                        IotSerializer_ScalarTextString("false"));
        }

        serializerError = awsJobsJsonEncoder->appendKeyValue( &statusDetails,
                                                                       "redFlagOnThreshold",
                                                                       IotSerializer_ScalarSignedInt( configsReceived.redFlagOnThreshold));

        serializerError = awsJobsJsonEncoder->appendKeyValue( &statusDetails,
                                                                       "redFlagOffThreshold",
                                                                       IotSerializer_ScalarSignedInt( configsReceived.redFlagOffThreshold));

        if ( valid == true )
        {
            serializerError = awsJobsJsonEncoder->appendKeyValue( &statusDetails,
                                                                        "valid",
                                                                        IotSerializer_ScalarTextString("true"));
        }
        else
        {
            serializerError = awsJobsJsonEncoder->appendKeyValue( &statusDetails,
                                                                        "valid",
                                                                         IotSerializer_ScalarTextString("false"));
        }
    }
    else
    {
        //only need one item for status details:
        serializerError = awsJobsJsonEncoder->openContainerWithKey( &updateJobExecutionMap,
                                                                            "statusDetails",
                                                                            &statusDetails,
                                                                            1 );
        serializerError = awsJobsJsonEncoder->appendKeyValue( &statusDetails,
                                                                       "requestType",
                                                                         IotSerializer_ScalarTextString(xRequestTypeToString(jobRequest)) );
    }

    //close status details map within the main map
    serializerError = awsJobsJsonEncoder->closeContainer( &updateJobExecutionMap, &statusDetails );


    //continue to add the rest of them
    serializerError = awsJobsJsonEncoder->appendKeyValue( &updateJobExecutionMap,
                                                                    "expectedVersion",
                                                                    IotSerializer_ScalarSignedInt( expectedVersion ) );

    serializerError = awsJobsJsonEncoder->appendKeyValue( &updateJobExecutionMap,
                                                                    "stepTimeoutInMinutes",
                                                                    IotSerializer_ScalarSignedInt( stepTimeoutMins ) );


    serializerError = awsJobsJsonEncoder->appendKeyValue( &updateJobExecutionMap,
                                                                    "clientToken",
                                                                    IotSerializer_ScalarTextString( clientToken ));

    //close the map
    serializerError = awsJobsJsonEncoder->closeContainer( pEncoderObject, &updateJobExecutionMap );

    if( serializerError != IOT_SERIALIZER_SUCCESS)
    {
        elogError( "Error encoding json payload");
    }
    else
    {
        stat = EXIT_SUCCESS;
    }

    return stat;
}

//Establish a new connection to the MQTT server.
static int xNewMqttConnection( bool awsIotMqttMode,
                                     const char * pIdentifier,
                                     void * pNetworkServerInfo,
                                     void * pNetworkCredentialInfo,
                                     const IotNetworkInterface_t * pNetworkInterface,
                                     IotMqttConnection_t * pMqttConnection )
{
    int status = EXIT_SUCCESS;
    IotMqttError_t connectStatus = IOT_MQTT_STATUS_PENDING;
    IotMqttNetworkInfo_t networkInfo = IOT_MQTT_NETWORK_INFO_INITIALIZER;
    IotMqttConnectInfo_t connectInfo = IOT_MQTT_CONNECT_INFO_INITIALIZER;
    char pClientIdentifierBuffer[ CLIENT_IDENTIFIER_MAX_LENGTH ] = { 0 };

    /* Set the members of the network info not set by the initializer. This
     * struct provided information on the transport layer to the MQTT connection. */
    networkInfo.createNetworkConnection = true;
    networkInfo.u.setup.pNetworkServerInfo = pNetworkServerInfo;
    networkInfo.u.setup.pNetworkCredentialInfo = pNetworkCredentialInfo;
    networkInfo.pNetworkInterface = pNetworkInterface;

    /* Set the members of the connection info not set by the initializer. */
    connectInfo.awsIotMqttMode = awsIotMqttMode;
    connectInfo.cleanSession = true;
    connectInfo.keepAliveSeconds = KEEP_ALIVE_SECONDS;

    /* Use the parameter client identifier if provided. Otherwise, generate a
     * unique client identifier. */
    if( ( pIdentifier != NULL ) && ( pIdentifier[ 0 ] != '\0' ) )
    {
        connectInfo.pClientIdentifier = pIdentifier;
        connectInfo.clientIdentifierLength = ( uint16_t ) strlen( pIdentifier );
    }
    else
    {
        /* Every active MQTT connection must have a unique client identifier.
         * generate this unique client identifier by appending a timestamp to a common
         * prefix. */
        status = snprintf( pClientIdentifierBuffer,
                           CLIENT_IDENTIFIER_MAX_LENGTH,
                           UNKNOWN_DEVICE_IDENTIFIER "%lu",
                           ( long unsigned int ) IotClock_GetTimeMs() );

        /* Check for errors from snprintf. */
        if( status < 0 )
        {
            elogDebug( "Failed to generate unique client identifier" );
            status = EXIT_FAILURE;
        }
        else
        {
            /* Set the client identifier buffer and length. */
            connectInfo.pClientIdentifier = pClientIdentifierBuffer;
            connectInfo.clientIdentifierLength = ( uint16_t ) status;

            status = EXIT_SUCCESS;
        }
    }

    /* Establish the MQTT connection. */
    if( status == EXIT_SUCCESS )
    {
        elogInfo( "MQTT client identifier is %.*s (length %hu).",
                    connectInfo.clientIdentifierLength,
                    connectInfo.pClientIdentifier,
                    connectInfo.clientIdentifierLength );

        connectStatus = IotMqtt_Connect(&networkInfo, &connectInfo, MQTT_TIMEOUT_MS, pMqttConnection);

        if( connectStatus != IOT_MQTT_SUCCESS )
        {
            elogDebug( "MQTT CONNECT returned error %s.", IotMqtt_strerror( connectStatus ) );

            status = EXIT_FAILURE;
        }
    }

    return status;
}

//Add or remove topic subscriptions
static int xChangeSubscriptions( IotMqttConnection_t mqttConnection,
                                 IotMqttOperationType_t operation,
                                 const char ** pTopicFilters,
                                 void * pCallbackParameter )
{
    int status = EXIT_SUCCESS;
    int32_t i = 0;
    IotMqttError_t subscriptionStatus = IOT_MQTT_STATUS_PENDING;
    IotMqttSubscription_t pSubscriptions[ TOPIC_FILTER_COUNT ] = { IOT_MQTT_SUBSCRIPTION_INITIALIZER };

    /* Set the members of the subscription list. - for now they are all set to the same callback function */
    for( i = 0; i < TOPIC_FILTER_COUNT; i++ )
    {
        pSubscriptions[ i ].qos = IOT_MQTT_QOS_1;
        pSubscriptions[ i ].pTopicFilter = pTopicFilters[ i ];
        pSubscriptions[ i ].topicFilterLength = strlen(pTopicFilters[i]);
        pSubscriptions[ i ].callback.pCallbackContext = pCallbackParameter;

        //set up appropriate cb functions
        if ( i <= PUBLISH_TOPICS_MAX_IDX )
        {
            pSubscriptions[ i ].callback.function = xSubscriptionCb;
        }
        else
        {
            if ( i == START_NEXT_JOB_IDX  )
            {
                pSubscriptions[ i ].callback.function = xStartNextJobCb;
            }
            else if ( i == START_NEXT_ACC_REJ_IDX )
            {
                pSubscriptions[ i ].callback.function = xStartNextJobAcceptRejectCb;
            }
        }
    }

    /* Modify subscriptions by either subscribing or unsubscribing. */
    if( operation == IOT_MQTT_SUBSCRIBE )
    {
        subscriptionStatus = IotMqtt_TimedSubscribe( mqttConnection,
                                                     pSubscriptions,
                                                     TOPIC_FILTER_COUNT,
                                                     0,
                                                     MQTT_TIMEOUT_MS );

        /* Check the status of SUBSCRIBE. */
        switch( subscriptionStatus )
        {
            case IOT_MQTT_SUCCESS:
                elogDebug( "All Topic filter subscriptions accepted." );
                break;

            case IOT_MQTT_SERVER_REFUSED:

                /* Check which subscriptions were rejected */
                for( i = 0; i < TOPIC_FILTER_COUNT; i++ )
                {
                    if( IotMqtt_IsSubscribed( mqttConnection,
                                              pSubscriptions[ i ].pTopicFilter,
                                              pSubscriptions[ i ].topicFilterLength,
                                              NULL ) == true )
                    {
                        elogDebug( "Topic filter %.*s was accepted.",
                                    pSubscriptions[ i ].topicFilterLength,
                                    pSubscriptions[ i ].pTopicFilter );
                    }
                    else
                    {
                        elogDebug( "Topic filter %.*s was rejected.",
                                     pSubscriptions[ i ].topicFilterLength,
                                     pSubscriptions[ i ].pTopicFilter );
                    }
                }

                status = EXIT_FAILURE;
                break;

            default:

                status = EXIT_FAILURE;
                break;
        }
    }
    else if( operation == IOT_MQTT_UNSUBSCRIBE )
    {
        subscriptionStatus = IotMqtt_TimedUnsubscribe( mqttConnection,
                                                       pSubscriptions,
                                                       TOPIC_FILTER_COUNT,
                                                       0,
                                                       MQTT_TIMEOUT_MS );

        /* Check the status of UNSUBSCRIBE. */
        if( subscriptionStatus != IOT_MQTT_SUCCESS )
        {
            status = EXIT_FAILURE;
        }
    }
    else
    {
        /* Only SUBSCRIBE and UNSUBSCRIBE are valid for modifying subscriptions. */
        elogDebug( "MQTT operation %s is not valid for modifying subscriptions.",
                     IotMqtt_OperationType( operation ) );

        status = EXIT_FAILURE;
    }

    return status;
}

static bool xPublishMessage( IotMqttConnection_t mqttConnection, IotMqttPublishInfo_t publishInfo)
{
    int status = EXIT_SUCCESS;
    IotMqttError_t publishStatus = IOT_MQTT_STATUS_PENDING;
    IotMqttCallbackInfo_t publishComplete = IOT_MQTT_CALLBACK_INFO_INITIALIZER;

    /* The MQTT library should invoke this callback when a PUBLISH message
     * is successfully transmitted. */
    publishComplete.function = xOperationCompleteCb;

    if(publishInfo.qos == IOT_MQTT_QOS_0)
    {
        publishStatus = IotMqtt_Publish( mqttConnection, &publishInfo, 0, NULL, NULL );
    }
    else
    {
        /* PUBLISH a message. This is an asynchronous function that notifies of
         * completion through a callback. */
        publishStatus = IotMqtt_Publish( mqttConnection, &publishInfo, 0, &publishComplete, NULL );

        //set flag, this will be cleared in the cb after completion of the send
        xSetTxOperationIp(true);

        if( publishStatus != IOT_MQTT_STATUS_PENDING )
        {
            elogError( "MQTT PUBLISH returned error %s.", IotMqtt_strerror( publishStatus ) );
            status = EXIT_FAILURE;
        }
    }

    return status;
}

static bool xSendJobUpdate(  char * jobId, awsJobStatus_t jobStat, jobRequestType_t jobRequest, uint32_t expectedVersion, uint32_t stepTimeoutMins, char *clientToken, bool valid )
{
    IotMqttPublishInfo_t publishInfo = IOT_MQTT_PUBLISH_INFO_INITIALIZER;
    bool status = EXIT_FAILURE;

    // create a the topic string
    char topicStr[200] = "";
    sprintf( (char*)topicStr,"$aws/things/%s/jobs/%s/update", (char*)xDuid, jobId);

    //json endoder
    status = jsonEncodeJobUpdateMessage(jobStat, jobRequest, expectedVersion, stepTimeoutMins, clientToken, valid);

    if ( status == EXIT_SUCCESS )
    {
        //set up the publish
       publishInfo.qos = IOT_MQTT_QOS_0;
       publishInfo.pTopicName = (char*)&topicStr;
       publishInfo.topicNameLength = strlen((char*)&topicStr);

       //fill in the payload with the json doc
       publishInfo.pPayload = jsonEncodedRsp.pDataBuffer;
       publishInfo.payloadLength = strlen((char*)jsonEncodedRsp.pDataBuffer);

       publishInfo.retryMs = PUBLISH_RETRY_MS;
       publishInfo.retryLimit = PUBLISH_RETRY_LIMIT;

       //Send out the message, will return false if couldnt queue up the msg
       status = xPublishMessage(mqttConnection, publishInfo);
    }

    return status;
}


/*
 * Request the next job if there is one
 *
 * {
        "statusDetails": {  },
        "stepTimeoutInMinutes": long,
        "clientToken": "string"
    }

 */
static bool xSendGetNextJobReq(void)
{

    IotMqttPublishInfo_t publishInfo = IOT_MQTT_PUBLISH_INFO_INITIALIZER;
    IotSerializerError_t serializerError = IOT_SERIALIZER_SUCCESS;
    bool stat = EXIT_FAILURE;
    IotSerializerEncoderObject_t * pEncoderObject = &( jsonEncodedRsp.object );

    IotSerializerEncoderObject_t jobRequestMap = IOT_SERIALIZER_ENCODER_CONTAINER_INITIALIZER_MAP;
    IotSerializerEncoderObject_t statusDetails = IOT_SERIALIZER_ENCODER_CONTAINER_INITIALIZER_MAP;

    memset(jsonEncodedRsp.pDataBuffer, 0, jsonEncodedRsp.size);

    serializerError = awsJobsJsonEncoder->init( pEncoderObject, jsonEncodedRsp.pDataBuffer, jsonEncodedRsp.size );


    // Create the outermost map with 3 keys: statusDetails, steptimeoutInMinutes, clientToken
    serializerError = awsJobsJsonEncoder->openContainer( pEncoderObject, &jobRequestMap, 4 );

    //for status details, need these values within another map
    serializerError = awsJobsJsonEncoder->openContainerWithKey( &jobRequestMap,
                                                                    "statusDetails",
                                                                    &statusDetails,
                                                                    1 );

    //close status details map within the main map
    serializerError = awsJobsJsonEncoder->closeContainer( &jobRequestMap, &statusDetails );

    serializerError = awsJobsJsonEncoder->appendKeyValue( &jobRequestMap,
                                                                   "stepTimeoutInMinutes",
                                                                   IotSerializer_ScalarSignedInt( ARBITRARY_TIMEOUT_MINS ) );

    serializerError = awsJobsJsonEncoder->appendKeyValue( &jobRequestMap, "includeJobDocument", IotSerializer_ScalarTextString( "true" ));


    serializerError = awsJobsJsonEncoder->appendKeyValue( &jobRequestMap, "clientToken", IotSerializer_ScalarTextString((char *) xDuid ));

    //close the map
    serializerError = awsJobsJsonEncoder->closeContainer( pEncoderObject, &jobRequestMap );


    if( serializerError == IOT_SERIALIZER_SUCCESS)
    {
       //set up the publish
        publishInfo.qos = IOT_MQTT_QOS_0;
        publishInfo.pTopicName = pTopicStrings[START_NEXT_JOB_IDX];
        publishInfo.topicNameLength = strlen(pTopicStrings[START_NEXT_JOB_IDX]);

        //fill in the payload with the json doc
        publishInfo.pPayload = jsonEncodedRsp.pDataBuffer;
        publishInfo.payloadLength = strlen((char*)jsonEncodedRsp.pDataBuffer);

        publishInfo.retryMs = PUBLISH_RETRY_MS;
        publishInfo.retryLimit = PUBLISH_RETRY_LIMIT;

        //Send out the message, will return false if couldnt queue up the msg
        stat = xPublishMessage(mqttConnection, publishInfo);
    }

    return stat;
}


// return pointer to string of associated status... or "" if not found
static const char *xJobStatusToString(awsJobStatus_t status)
{
    switch (status) {
        case SUCCEEDED:
            return "SUCCEEDED";
        case IN_PROGRESS:
            return "IN_PROGRESS";
        case FAILED:
            return "FAILED";
        default:
            return "";
    }
}

// return pointer to string of associated job request.. or "" if not found
static const char *xRequestTypeToString(jobRequestType_t jobRequest)
{
    switch (jobRequest) {
        case RESET_ALARMS:
            return "RESET_ALARMS";
        case ACTIVATE:
            return "ACTIVATE";
        case DEACTIVATE:
            return "DEACTIVATE";
        case CONFIGURE:
            return "CONFIGURE";
        case RESET:
            return "RESET";
        case GPS:
            return "GPS";
        case UPDATE:
            return "UPDATE";
        case HW_RESET:
            return "HW_RESET";
        default:
            return "";
    }
}

//convert sting to enum
static jobRequestType_t xStringToRequestType(const char * requestTypeStr)
{
    if (strcmp(requestTypeStr, "RESET_ALARMS") == 0)
    {
        return RESET_ALARMS;
    }
    if (strcmp(requestTypeStr, "ACTIVATE") == 0)
    {
        return ACTIVATE;
    }
    if (strcmp(requestTypeStr, "DEACTIVATE") == 0)
    {
        return DEACTIVATE;
    }
    if (strcmp(requestTypeStr, "CONFIGURE") == 0)
    {
        return CONFIGURE;
    }
    if (strcmp(requestTypeStr, "RESET") == 0)
    {
        return RESET;
    }
    if (strcmp(requestTypeStr, "GPS") == 0)
    {
        return GPS;
    }
    if (strcmp(requestTypeStr, "UPDATE") == 0)
    {
        return UPDATE;
    }
    if (strcmp(requestTypeStr, "HW_RESET") == 0)
    {
        return HW_RESET;
    }
    else
    {
        return UNKNOWN;
    }
}

