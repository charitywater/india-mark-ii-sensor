/**************************************************************************************************
* \file     eventManager.c
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

#include "FreeRTOS.h"
#include "queue.h"
#include "timers.h"
#include "stm32l4xx_hal.h"
#include "task.h"
#include "logTypes.h"
#include "connectivity.h"
#include "ssm.h"
#include "messages.pb.h"
#include "appVersion.h"
#include "mqttHandler.h"
#include "CLI.h"
#include "memMapHandler.h"
#include "memoryMap.h"
#include "pwrMgr.h"
#include "nwStackFunctionality.h"
#include "otaUpdate.h"
#include "updateSsmFw.h"
#include "externalWatchdog.h"
#include "gpsManager.h"
#include <eventManager.h>

//product ID will always be 4
#define PRODUCT_ID                  4
#define MAX_EVT_PAYLOAD             50

#define WAIT_ONE_SECOND             1000

//if we are in the middle of sending a message to the cloud, wait an additional
//half a second to finish before going to sleep!
#define AM_ALLOWED_TOLERANCE_MS     500

//run the task every 50 ms when not in the middle of something
#define EVT_TASK_POLL_RATE_MS       50

//Event types that other application modules can report:
typedef enum
{
    SSM_ACTIVATE_EVT,
    CLOUD_ACTIVATE_EVT,
    DEACTIVATE_EVT,
    CHECK_IN_EVT_ACTIVATED,
    CHECK_IN_EVT_DEACTIVATED,
    INITIATE_NTP_TIME_SYNC_EVT,
    NTP_TIME_SYNC_SUCCESS,
    NTP_TIME_SYNC_FAIL,
    GPS_FIX_REQUESTED,
    GPS_FIX_SUCCESS,
    GPS_FIX_TIMEOUT,
    CONFIGURE_EVT,
    HW_RESET_CMD,
    RESET_ALARMS_CMD,
    OTA,
    FW_DOWNLOAD_COMPLETE,
    FW_DOWNLOAD_FAIL,
    UNRESPONSIVE_SSM,
    SSM_REQ_NACK,
    MQTT_READY,
    MQTT_NW_ERROR,
    MQTT_NO_JOBS,
    SENSOR_DATA_MSG_RECEIVED,
    SENSOR_DATA_READY,
    SENSOR_DATA_PUBLISH_SUCCESS,
    CLOUD_CONNECT_FAILURE,
    MANF_COMPLETE,
}eventID_t;

//payloads for the events
//(not all events will have a payload)
typedef union
{
    uint8_t bytes[MAX_EVT_PAYLOAD];
    configureMsg_t configs;
    char * fwLinkForOtaAddr;
    bool deactivateBeforeReset;
    bool newGpsMeasurement;
}evtQueuePayload_t;

//Event contents:
typedef struct
{
    eventID_t eventID;
    evtQueuePayload_t payload;
}eventMsg_t;

QueueHandle_t eventQueue;
static eventMsg_t xIncomingEvent;
static bool xWaitingOnCell = false;
static bool xAwsConnected = false;

//System Status
static uint32_t xTimeAmHasBeenOn = 0u;
static uint32_t xAllowedTimeOn = AM_ALLOWED_TIME_ON_MS;

//Enable test mode to prevent the AM from sleeping
static bool xTestMode = false;

//use these variables to determine if we need to
//update the loaded image op state at any time
static bool xUpdateImageOpState = false;
static imageOperationalState_t xCurrentState;
static imageSlotTypes_t xCurrentSlot;

//last ssm status
static asp_status_payload_t xSsmStatus = {};

// Sensor data message
static asp_sensor_data_entry_t xSensorData = {};
static bool xSensorDataReady = false;
static uint32_t awsConnectTimeMs = 0u;
static uint8_t gpsRetryCount = 0u;
static bool xIsStrokeDetectionEnabled = false;

static TimerHandle_t xManfCompleteTimerHandle;

static void xInitStateManager(void);
static void xHandleCheckInActivated(void);
static void xHandleCheckInDeactivated(void);
static void xHandleSsmActivate(void);
static void xHandleCloudActivate(void);
static void xHandleDeactivate(void);
static void xInitiateNtpTimeSync(void);
static void xHandleConfigs(configureMsg_t configsRcd);
static void xTurnOffCellAndPowerDown(void);
static void xPackageAndSendStatusToCloud(void);
static void xPackageAndSendGpsMsgToCloud(void);
static void xPackageAndStoreSensorDataToFlash(void);
static void xHandleSensorDataReady(void);
static bool xPackageAndSendSensorDataToCloud(void);
static void xHandleMqttReady(void);
static void xOtaFwDownloadSuccessful(void);
static void commandHandlerForApp(int argc, char **argv);
static void xHandleHwResetCloudCmd(bool deactivateBeforeReset);
static void xHandleResetAlarmsCloudCmd(void);
static void xHandleGpsJob(bool newMeasurement);
static void xInitMfgCompleteTimer(uint32_t timerSeconds);
static void xHandleMfgCompleteTimerElapsed(void);

void EVT_initializeEventQueue(void)
{
    //Init the message queue for events
    eventQueue = xQueueCreate( 10, sizeof( eventMsg_t ) );

    if ( eventQueue == NULL )
    {
        elogError("FAILED to create event queue");
    }
    else
    {
        elogInfo("initialized event queue");
    }
}

void EVT_eventManagerTask()
{
    xInitStateManager();

    while (1)
    {
        //handle any received events - block on waiting
        if( xQueueReceive( eventQueue, &( xIncomingEvent ), ( TickType_t ) EVT_TASK_POLL_RATE_MS ) == pdPASS )
        {
            switch(xIncomingEvent.eventID)
            {
                case SSM_ACTIVATE_EVT:

                    //start up the cell task to send the cloud message
                    xHandleSsmActivate();

                    break;
                case CLOUD_ACTIVATE_EVT:

                    xHandleCloudActivate();

                    break;
                case DEACTIVATE_EVT:

                    xHandleDeactivate();

                    break;
                case CHECK_IN_EVT_DEACTIVATED:

                    xHandleCheckInDeactivated();

                    break;
                case CHECK_IN_EVT_ACTIVATED:

                    xHandleCheckInActivated();

                    break;
                case INITIATE_NTP_TIME_SYNC_EVT:

                    xInitiateNtpTimeSync();

                    break;
                case NTP_TIME_SYNC_SUCCESS:

                    elogInfo("Time received from NTP server");
                    SSM_performTimeSync();

                    break;
                case NTP_TIME_SYNC_FAIL:

                    elogNotice("Failed to get time from NTP server");

                    //we send 0x00000000 as the response to the SSM HW command so it knows to try again
                    SSM_performTimeSync();

                    break;
                case MQTT_READY:

                    xHandleMqttReady();

                    break;
                case CONFIGURE_EVT:
                    elogInfo("New configs received from the cloud!");

                    //save off configs, alert the SSM, and send pass/fail to cloud
                    xHandleConfigs(xIncomingEvent.payload.configs);

                    break;
                case MQTT_NW_ERROR:
                    break;
                case UNRESPONSIVE_SSM:
                    //AM will reload the SSM on next boot up if this continues. For now just log
                    elogError("SSM unresponsive");

                    break;
                case SSM_REQ_NACK:
                    //We sent a bad request to the ssm, or it is not properly receiving our messages.
                    //AM will reload the SSM on next boot up if this continues
                    elogInfo("SSM nacked request");

                    break;
                case MQTT_NO_JOBS:

                    //turn off modem only if we are not doing anything else connectivity related
                    if ( GPS_isGpsEnabled() == false )
                    {
                        //nominal case - turn off since there is nothing left to do
                        xTurnOffCellAndPowerDown();
                    }

                    xWaitingOnCell = false;

                    break;
                case OTA:

                    //update the allowed time on since we can stay "on" for up to 15 minutes during OTA
                    xAllowedTimeOn = AM_ALLOWED_TIME_ON_OTA_MS;
                    OTA_initDownload(xIncomingEvent.payload.fwLinkForOtaAddr);

                    break;
                case FW_DOWNLOAD_COMPLETE:

                    xOtaFwDownloadSuccessful();

                    break;
                case FW_DOWNLOAD_FAIL:

                    MQTT_indicateOperationFail();
                    MQTT_IndicateReadyForNewJobs();

                    break;
                case SENSOR_DATA_MSG_RECEIVED:

                    xPackageAndStoreSensorDataToFlash();

                    break;
                case SENSOR_DATA_READY:

                    xHandleSensorDataReady();

                    break;
                case SENSOR_DATA_PUBLISH_SUCCESS:

                    MEM_updateSensorDataHeadAndLifoCount();
                    xPackageAndSendSensorDataToCloud();

                    break;
                case CLOUD_CONNECT_FAILURE:

                    if ( GPS_isGpsEnabled() == false )
                    {
                        //Go back to sleep
                        xTurnOffCellAndPowerDown();
                    }

                    break;
                case HW_RESET_CMD:
                    xHandleHwResetCloudCmd(xIncomingEvent.payload.deactivateBeforeReset);

                    break;
                case RESET_ALARMS_CMD:

                    xHandleResetAlarmsCloudCmd();

                    break;
                case GPS_FIX_SUCCESS:

                    if ( xAwsConnected == true )
                    {
                        xPackageAndSendGpsMsgToCloud();
                        MEM_SetGpsSentToCloud(true);

                        MQTT_IndicateReadyForNewJobs();
                    }

                    break;
                case GPS_FIX_TIMEOUT:

                    //we will want to try again the next time the AM is on
                    gpsRetryCount++;
                    MEM_SetGpsNumRetries(gpsRetryCount);

                    if ( xWaitingOnCell == false )
                    {
                        xTurnOffCellAndPowerDown();
                    }

                    break;
                case GPS_FIX_REQUESTED:

                    xHandleGpsJob(xIncomingEvent.payload.newGpsMeasurement);

                    break;
                case MANF_COMPLETE:

                    xHandleMfgCompleteTimerElapsed();

                    break;

                default:
                    elogInfo("Unknown ID");
                    break;
            }
        }

        //increment on time
        xTimeAmHasBeenOn+= EVT_TASK_POLL_RATE_MS;

        //check if we need to turn off due to a timeout - we will automatically turn off if we have received a 'no jobs' event above
        if (xTimeAmHasBeenOn >= xAllowedTimeOn  && xTestMode == false)
        {
            if ( MQTT_getOperationInProgressFlag() == false || ( xTimeAmHasBeenOn >= (xAllowedTimeOn + AM_ALLOWED_TOLERANCE_MS)) )
            {
                xTurnOffCellAndPowerDown();
            }
        }
    }
}

//callback function for the manufacturing complete timer
//which indicates that its time to send a message to the cloud
void vMfgTimerCallback( TimerHandle_t xTimer )
{
    //pass up the event
    EVT_indicateManufacturingComplete();
}

//Activate event can be from a cloud message, or
// from the SSM
void EVT_indicateActivateFromSsm(void)
{
    eventMsg_t msg;
    msg.eventID = SSM_ACTIVATE_EVT;
    xQueueSend(eventQueue, &msg, ( TickType_t ) QUEUE_WAIT_TIME_MS );
}

void EVT_indicateActivateFromCloud(void)
{
    eventMsg_t msg;
    msg.eventID = CLOUD_ACTIVATE_EVT;
    xQueueSend(eventQueue, &msg, ( TickType_t ) QUEUE_WAIT_TIME_MS );
}

void EVT_indicateNewConfigMessage(configureMsg_t configs)
{
    eventMsg_t msg;
    msg.eventID = CONFIGURE_EVT;
    msg.payload.configs = configs;
    xQueueSend(eventQueue, &msg, ( TickType_t ) QUEUE_WAIT_TIME_MS );
}

void EVT_indicateHwResetCmd(bool deactivateBeforeReset)
{
    eventMsg_t msg;
    msg.eventID = HW_RESET_CMD;
    msg.payload.deactivateBeforeReset = deactivateBeforeReset;
    xQueueSend(eventQueue, &msg, ( TickType_t ) QUEUE_WAIT_TIME_MS );
}

void EVT_indicateResetAlarmsCmd(void)
{
    eventMsg_t msg;
    msg.eventID = RESET_ALARMS_CMD;
    xQueueSend(eventQueue, &msg, ( TickType_t ) QUEUE_WAIT_TIME_MS );
}

void EVT_indicateOta(char * link)
{
    eventMsg_t msg;
    msg.eventID = OTA;
    msg.payload.fwLinkForOtaAddr = link;
    xQueueSend(eventQueue, &msg, ( TickType_t ) QUEUE_WAIT_TIME_MS );
}

void EVT_indicateFwDownloadComplete(void)
{
    eventMsg_t msg;
    msg.eventID = FW_DOWNLOAD_COMPLETE;
    xQueueSend(eventQueue, &msg, ( TickType_t ) QUEUE_WAIT_TIME_MS );
}

void EVT_indicateFwDownloadFail(void)
{
    eventMsg_t msg;
    msg.eventID = FW_DOWNLOAD_FAIL;
    xQueueSend(eventQueue, &msg, ( TickType_t ) QUEUE_WAIT_TIME_MS );
}

void EVT_indicateDeActivate(void)
{
    eventMsg_t msg;
    msg.eventID = DEACTIVATE_EVT;
    xQueueSend(eventQueue, &msg, ( TickType_t ) QUEUE_WAIT_TIME_MS );
}

void EVT_indicateCheckInActivated(void)
{
    eventMsg_t msg;
    msg.eventID = CHECK_IN_EVT_ACTIVATED;
    xQueueSend(eventQueue, &msg, ( TickType_t ) QUEUE_WAIT_TIME_MS );
}

void EVT_indicateCheckInDeactivated(void)
{
    eventMsg_t msg;
    msg.eventID = CHECK_IN_EVT_DEACTIVATED;
    xQueueSend(eventQueue, &msg, ( TickType_t ) QUEUE_WAIT_TIME_MS );
}

void EVT_initiateNtpTimeSync(void)
{
    eventMsg_t msg;
    msg.eventID = INITIATE_NTP_TIME_SYNC_EVT;
    xQueueSend(eventQueue, &msg, ( TickType_t ) QUEUE_WAIT_TIME_MS );
}

void EVT_indicateNtpTimeSyncSuccess(void)
{
    eventMsg_t msg;
    msg.eventID = NTP_TIME_SYNC_SUCCESS;
    xQueueSend(eventQueue, &msg, ( TickType_t ) QUEUE_WAIT_TIME_MS );
}

void EVT_indicateNtpTimeSyncFailure(void)
{
    eventMsg_t msg;
    msg.eventID = NTP_TIME_SYNC_FAIL;
    xQueueSend(eventQueue, &msg, ( TickType_t ) QUEUE_WAIT_TIME_MS );
}

void EVT_indicateNoNewJobsFromCloud(void)
{
    eventMsg_t msg;
    msg.eventID = MQTT_NO_JOBS;
    xQueueSend(eventQueue, &msg, ( TickType_t ) QUEUE_WAIT_TIME_MS );
}

void EVT_indicateSsmUnresponsive(void)
{
    eventMsg_t msg;
    msg.eventID = UNRESPONSIVE_SSM;
    xQueueSend(eventQueue, &msg, ( TickType_t ) QUEUE_WAIT_TIME_MS );
}

void EVT_indicateSsmNackedRequest(void)
{
    eventMsg_t msg;
    msg.eventID = SSM_REQ_NACK;
    xQueueSend(eventQueue, &msg, ( TickType_t ) QUEUE_WAIT_TIME_MS );
}

void EVT_indicateMqttReady(void)
{
    eventMsg_t msg;
    msg.eventID = MQTT_READY;
    xQueueSend(eventQueue, &msg, ( TickType_t ) QUEUE_WAIT_TIME_MS );
}

void EVT_indicateMqttNWError(void)
{
    eventMsg_t msg;
    msg.eventID = MQTT_NW_ERROR;
    xQueueSend(eventQueue, &msg, ( TickType_t ) QUEUE_WAIT_TIME_MS );
}

void EVT_indicateSensorDataMsgReceivedFromSSM(void)
{
    eventMsg_t msg;
    msg.eventID = SENSOR_DATA_MSG_RECEIVED;
    xQueueSend(eventQueue, &msg, ( TickType_t ) QUEUE_WAIT_TIME_MS );
}

void EVT_indicateSensorDataReady(void)
{
    eventMsg_t msg;
    msg.eventID = SENSOR_DATA_READY;
    xQueueSend(eventQueue, &msg, ( TickType_t ) QUEUE_WAIT_TIME_MS );
}

void EVT_indicateMqttPublishSuccess(void)
{
    eventMsg_t msg;
    msg.eventID = SENSOR_DATA_PUBLISH_SUCCESS;
    xQueueSend(eventQueue, &msg, ( TickType_t ) QUEUE_WAIT_TIME_MS );
}

void EVT_indicateManufacturingComplete(void)
{
    eventMsg_t msg;
    msg.eventID = MANF_COMPLETE;
    xQueueSend(eventQueue, &msg, ( TickType_t ) QUEUE_WAIT_TIME_MS );
}

void EVT_indicateGpsFixCompleted(bool fixSucceeded)
{
    eventMsg_t msg;

    if (fixSucceeded)
    {
        msg.eventID = GPS_FIX_SUCCESS;
    }
    else
    {
        msg.eventID = GPS_FIX_TIMEOUT;
    }

    xQueueSend(eventQueue, &msg, ( TickType_t ) QUEUE_WAIT_TIME_MS );
}

void EVT_indicateGpsLocationRequested(bool takeNewMeasurement)
{
    eventMsg_t msg;

    msg.eventID = GPS_FIX_REQUESTED;
    msg.payload.newGpsMeasurement = takeNewMeasurement;

    xQueueSend(eventQueue, &msg, ( TickType_t ) QUEUE_WAIT_TIME_MS );
}

void EVT_indicateCloudConnectFailure(void)
{
    eventMsg_t msg;
    msg.eventID = CLOUD_CONNECT_FAILURE;
    xQueueSend(eventQueue, &msg, ( TickType_t ) QUEUE_WAIT_TIME_MS );
}

static void xInitStateManager(void)
{
    //Check if we should update the image slot state when we connect to the cloud
    xCurrentSlot = MEM_getLoadedImage();

    if ( xCurrentSlot != UNKNOWN_SLOT )
    {
        xCurrentState = MEM_getImageOpState(xCurrentSlot);

       if ( xCurrentState == PARTIAL )
       {
           //set flag to true, when we connect to cloud
           //we will update the op state to FULL
           xUpdateImageOpState = true;
       }
    }
    else
    {
        elogNotice("No copy of this image in external flash. OTA a copy asap");
    }

    //init gps retry count
    gpsRetryCount = MEM_GetGpsNumRetries();

    //if we dont have a GPS fix, but we have tried previously + not gone over the
    //retry limit, turn on gps to try to get a fix (aka if we are deactivated dont try to get a fix.)
    if ( MEM_GetGpsFixedFlag() == false && gpsRetryCount > 0 && gpsRetryCount < MAX_GPS_RETRIES )
    {
        GPS_Enable();
    }

    //check if we have completed manufacturing and need to send a message soon:
    if ( MEM_getMfgCompleteFlag() == true )
    {
        elogInfo("Manufacturing complete flag is set - starting timer");

        //start timer
        xInitMfgCompleteTimer(MEM_getSecondsToWaitForMfgComplete());
    }

    //get a status message from the SSM
    xSsmStatus = SSM_getStatus();

    //read out the stroke detection config
    xIsStrokeDetectionEnabled = MEM_getIsStrokeDetectionEnabledFlag();

    //include a CLI handler for testing
    CLI_Command_Handler_s cmdHandler;
    cmdHandler.ptrFunction = &commandHandlerForApp;
    cmdHandler.cmdString   = "app";
    cmdHandler.usageString = "\n\r\tdeact \n\r\tact \n\r\tmode [prod test] \n\r\trresetcount [number] \n\r\tmfgComplete \n\r\twaitTime [30 - 600] seconds";
    CLI_registerThisCommandHandler(&cmdHandler);
}

static void xHandleSsmActivate(void)
{
    elogInfo("new activate event - from SSM");

    if ( xWaitingOnCell == false )
    {
        //start cell task
        CONN_init(xSsmStatus.timestamp);

        //we need to send a status message once the network is up and running
        xWaitingOnCell = true;
    }

    //now the SSM should also update its state
    SSM_changeStateToActivated();

    //request a new ssm status message to include in the payload
    SSM_requestStatusFromSsm();

    if ( xTestMode == false )
    {
        //Get GPS location
        GPS_Enable();
    }
}

static void xHandleCloudActivate(void)
{
    elogInfo("new activate event - from Cloud");

    //alert the SSM
    SSM_changeStateToActivated();

    //get the status from ssm
    SSM_requestStatusFromSsm();

    //wait a seconds to allow the SSM to finish activating before reading the state
    vTaskDelay(2000);

    if ( xTestMode == false )
    {
        //Get GPS location
        GPS_Enable();
    }

    //cell should be on & connected to get to this point in the field
    //If issued through the CLI then do not do this part
    if (xAwsConnected == true )
    {
        xSsmStatus = SSM_getStatus();

        //check the ssm status to determine if we have passed or failed
        if ( xSsmStatus.activatedState == eState_ACTIVATED )
        {
            //indicate that we have changed state
            MQTT_indicateOperationPass();
            elogInfo("Activate job passed");
        }
        else
        {
            MQTT_indicateOperationFail();
            elogInfo("Activate job FAILED");
        }

        xPackageAndSendStatusToCloud();

        //get new jobs
        MQTT_IndicateReadyForNewJobs();
    }
}

static void xHandleMfgCompleteTimerElapsed(void)
{
    elogInfo("Time to send a manufacturing complete message - turn on cell");

    if ( xWaitingOnCell == false )
    {
        //start cell task
        CONN_init(xSsmStatus.timestamp);

        //we need to send a status message once the network is up and running
        xWaitingOnCell = true;
    }

    //request a new ssm status message to include in the payload
    SSM_requestStatusFromSsm();
}

static void xHandleDeactivate(void)
{
    bool pass = false;
    GpsMessage clearedGpsMsg = {};

    elogInfo("new deactivate event from Cloud");

    //delete any saved sensor data
    pass = MEM_defaultSection(SECTION_DATA);

    //also remove GPS location
    if ( pass == true )
    {
        MEM_UpdateGpsCoordinates(clearedGpsMsg);
        MEM_SetGpsFixedFlag(false);
        MEM_SetGpsNumRetries(0);
        MEM_SetGpsSentToCloud(false);
    }

    if (pass == true)
    {
        SSM_deactivateionReceivedFromCloud();

        //tell the SSM manager to request the ssm status
        SSM_requestStatusFromSsm();

        //delay a second to allow the SSM to finish deactivating/sending stat
        vTaskDelay(1000);

        //read the latest status
        xSsmStatus = SSM_getStatus();
    }


    //cell should be on & connected to get to this point in the field
    //If issued through the CLI then do not do this part
    if ( xAwsConnected == true )
    {
        //check the ssm status to determine if we have passed or failed
        if ( xSsmStatus.activatedState == eState_DEACTIVATED && pass == true)
        {
            //indicate that we have changed state
            MQTT_indicateOperationPass();
        }
        else
        {
            MQTT_indicateOperationFail();
        }

        xPackageAndSendStatusToCloud();

        MQTT_IndicateReadyForNewJobs();
    }
}

static void xPackageAndSendStatusToCloud(void)
{
    xSsmStatus = SSM_getStatus();

    //send any messages now
    StatusMessage statusToSend = StatusMessage_init_zero;

    statusToSend.header.productId = PRODUCT_ID;
    statusToSend.header.timestamp = xSsmStatus.timestamp;
    statusToSend.header.msgNumber = MEM_getMsgNumber();
    statusToSend.header.fwMajor = VERSION_MAJOR;
    statusToSend.header.fwMinor = VERSION_MINOR;
    statusToSend.header.fwBuild = VERSION_BUILD;
    statusToSend.header.voltage = xSsmStatus.voltageMv;
    statusToSend.header.state = xSsmStatus.activatedState;
    statusToSend.header.powerRemaining = xSsmStatus.powerRemainingPercent;
    statusToSend.header.activatedDate = xSsmStatus.activatedDate;
    statusToSend.header.magnetDetected = xSsmStatus.magnetDetected;
    statusToSend.header.errorBits = xSsmStatus.errorBits;
    statusToSend.header.numSSMResets = xSsmStatus.unexpectedResetCount;
    statusToSend.header.lastSSMResetDate = xSsmStatus.timeLastReset;
    statusToSend.header.numAMResets = MEM_getUnexpectedResetCount();
    statusToSend.header.lastAMResetDate = MEM_getTimestampLastUnexpectedReset();
    statusToSend.header.rssi = NW_getRssiValue();
    statusToSend.header.connectTime = awsConnectTimeMs;
    statusToSend.header.imei = NW_getImeiOfModem();
    statusToSend.header.mfgComplete = MEM_getMfgCompleteFlag();


    //set the flags to true for the optional fields
    statusToSend.header.has_activatedDate = true;
    statusToSend.header.has_connectTime = true;
    statusToSend.header.has_errorBits = true;
    statusToSend.header.has_lastAMResetDate = true;
    statusToSend.header.has_lastSSMResetDate = true;
    statusToSend.header.has_magnetDetected = true;
    statusToSend.header.has_mfgComplete = true;
    statusToSend.header.has_numAMResets = true;
    statusToSend.header.has_numSSMResets = true;
    statusToSend.header.has_powerRemaining = true;
    statusToSend.header.has_rssi = true;
    statusToSend.header.has_state = true;
    statusToSend.header.has_voltage = true;

    //send the msg over mqtt
    MQTT_sendStatusMsg(statusToSend);

    MEM_updateMsgNumber();
}

static void xPackageAndSendGpsMsgToCloud(void)
{
    GpsMessage gpsMsg;

    gpsMsg = MEM_GetGpsCoordinates();

    //populate the header
    gpsMsg.header.productId = PRODUCT_ID;
    gpsMsg.header.timestamp = xSsmStatus.timestamp;
    gpsMsg.header.msgNumber = MEM_getMsgNumber();
    gpsMsg.header.fwMajor = VERSION_MAJOR;
    gpsMsg.header.fwMinor = VERSION_MINOR;
    gpsMsg.header.fwBuild = VERSION_BUILD;
    gpsMsg.header.voltage = xSsmStatus.voltageMv;
    gpsMsg.header.state = xSsmStatus.activatedState;
    gpsMsg.header.powerRemaining = xSsmStatus.powerRemainingPercent;
    gpsMsg.header.activatedDate = xSsmStatus.activatedDate;
    gpsMsg.header.magnetDetected = xSsmStatus.magnetDetected;
    gpsMsg.header.errorBits = xSsmStatus.errorBits;
    gpsMsg.header.numSSMResets = xSsmStatus.unexpectedResetCount;
    gpsMsg.header.lastSSMResetDate = xSsmStatus.timeLastReset;
    gpsMsg.header.numAMResets = MEM_getUnexpectedResetCount();
    gpsMsg.header.lastAMResetDate = MEM_getTimestampLastUnexpectedReset();
    gpsMsg.header.rssi = NW_getRssiValue();
    gpsMsg.header.connectTime = awsConnectTimeMs;
    gpsMsg.header.imei = NW_getImeiOfModem();
    gpsMsg.header.mfgComplete = MEM_getMfgCompleteFlag();

    //set the flags to true for the optional fields in header
    gpsMsg.header.has_activatedDate = true;
    gpsMsg.header.has_connectTime = true;
    gpsMsg.header.has_errorBits = true;
    gpsMsg.header.has_lastAMResetDate = true;
    gpsMsg.header.has_lastSSMResetDate = true;
    gpsMsg.header.has_magnetDetected = true;
    gpsMsg.header.has_mfgComplete = true;
    gpsMsg.header.has_numAMResets = true;
    gpsMsg.header.has_numSSMResets = true;
    gpsMsg.header.has_powerRemaining = true;
    gpsMsg.header.has_rssi = true;
    gpsMsg.header.has_state = true;
    gpsMsg.header.has_voltage = true;

    //set the flags to true for the optional fields in payload
    gpsMsg.has_altitude = true;
    gpsMsg.has_fixQuality = true;
    gpsMsg.has_hdopValue = true;
    gpsMsg.has_hours = true;
    gpsMsg.has_latitude = true;
    gpsMsg.has_longitude = true;
    gpsMsg.has_measurementTime = true;
    gpsMsg.has_minutes = true;
    gpsMsg.has_satellitesTracked = true;

    MQTT_sendGpsMsg(gpsMsg);

    MEM_updateMsgNumber();
}

static bool xPackageAndSendSensorDataToCloud(void)
{
    SensorDataMessage sensorDataMessageToSend = SensorDataMessage_init_default;
    APP_NVM_SENSOR_DATA_WITH_HEADER_T sensorDataEntry = {};
    uint32_t formattedLiters[APP_NVM_SAMPLES_PER_DAY];
    uint32_t formattedTemp[APP_NVM_SAMPLES_PER_DAY];
    uint32_t formattedHumidity[APP_NVM_SAMPLES_PER_DAY];
    uint32_t formattedStrokes[APP_NVM_SAMPLES_PER_DAY];
    uint32_t formattedStrokeHeight[APP_NVM_SAMPLES_PER_DAY];
    uint8_t i;
    bool status = false;

    int16_t msgsToSend = MEM_getNumSensorDataEntries();

    elogInfo("num logs %d", msgsToSend);

    if (msgsToSend > 0)
    {
        if(MEM_getSensorDataLog(&sensorDataEntry) == true)
        {
            /********** Message Header **********/
            sensorDataMessageToSend.header.productId = sensorDataEntry.productId;
            sensorDataMessageToSend.header.timestamp = sensorDataEntry.timestamp;
            sensorDataMessageToSend.header.msgNumber = sensorDataEntry.msgNumber;
            sensorDataMessageToSend.header.fwMajor = sensorDataEntry.fwVersionMaj;
            sensorDataMessageToSend.header.fwMinor = sensorDataEntry.fwVersionMinor;
            sensorDataMessageToSend.header.fwBuild = sensorDataEntry.fwVersionBuild;
            sensorDataMessageToSend.header.voltage = (uint32_t)sensorDataEntry.batteryVoltage;
            sensorDataMessageToSend.header.powerRemaining = (uint32_t)sensorDataEntry.powerRemaining;
            sensorDataMessageToSend.header.state = (eState)sensorDataEntry.state;
            sensorDataMessageToSend.header.activatedDate = sensorDataEntry.activatedDate;
            sensorDataMessageToSend.header.magnetDetected = sensorDataEntry.magnetDetected;
            sensorDataMessageToSend.header.errorBits = sensorDataEntry.errorBits;
            sensorDataMessageToSend.header.numSSMResets = sensorDataEntry.numSSMResets;
            sensorDataMessageToSend.header.lastSSMResetDate = sensorDataEntry.lastSSMResetDate;
            sensorDataMessageToSend.header.numAMResets = sensorDataEntry.numAMResets;
            sensorDataMessageToSend.header.lastAMResetDate = sensorDataEntry.lastAMResetDate;

            //get rssi value on the fly:
            sensorDataMessageToSend.header.rssi = NW_getRssiValue();
            sensorDataMessageToSend.header.connectTime = awsConnectTimeMs;
            sensorDataMessageToSend.header.imei = NW_getImeiOfModem();
            sensorDataMessageToSend.header.mfgComplete = MEM_getMfgCompleteFlag();

            //set the flags to true for the optional fields in the header
            sensorDataMessageToSend.header.has_activatedDate = true;
            sensorDataMessageToSend.header.has_connectTime = true;
            sensorDataMessageToSend.header.has_errorBits = true;
            sensorDataMessageToSend.header.has_lastAMResetDate = true;
            sensorDataMessageToSend.header.has_lastSSMResetDate = true;
            sensorDataMessageToSend.header.has_magnetDetected = true;
            sensorDataMessageToSend.header.has_mfgComplete = true;
            sensorDataMessageToSend.header.has_numAMResets = true;
            sensorDataMessageToSend.header.has_numSSMResets = true;
            sensorDataMessageToSend.header.has_powerRemaining = true;
            sensorDataMessageToSend.header.has_rssi = true;
            sensorDataMessageToSend.header.has_state = true;
            sensorDataMessageToSend.header.has_voltage = true;

            // TODO: Add debug log

            /********** Message Body **********/
            for(i = 0; i < APP_NVM_SAMPLES_PER_DAY; i++)
            {
                formattedLiters[i] = (uint32_t)sensorDataEntry.litersPerHour[i];
                formattedTemp[i] = (uint32_t)sensorDataEntry.tempPerHour[i];
                formattedHumidity[i] = (uint32_t)sensorDataEntry.humidityPerHour[i];
                formattedStrokes[i] = (uint32_t)sensorDataEntry.strokesPerHour[i];
                formattedStrokeHeight[i] = (uint32_t)sensorDataEntry.strokeHeightPerHour[i];
            }
            memcpy(&sensorDataMessageToSend.litersPerHour, &formattedLiters, sizeof(sensorDataMessageToSend.litersPerHour));
            memcpy(&sensorDataMessageToSend.tempPerHour, &formattedTemp, sizeof(sensorDataMessageToSend.tempPerHour));
            memcpy(&sensorDataMessageToSend.humidityPerHour, &formattedHumidity, sizeof(sensorDataMessageToSend.humidityPerHour));
            memcpy(&sensorDataMessageToSend.strokesPerHour, &formattedStrokes, sizeof(sensorDataMessageToSend.strokesPerHour));
            memcpy(&sensorDataMessageToSend.strokeHeightPerHour, &formattedStrokeHeight, sizeof(sensorDataMessageToSend.strokeHeightPerHour));
            sensorDataMessageToSend.dailyLiters = sensorDataEntry.dailyLiters;
            sensorDataMessageToSend.avgLiters = sensorDataEntry.avgLiters;
            sensorDataMessageToSend.totalLiters = sensorDataEntry.totalLiters;
            sensorDataMessageToSend.breakdown = sensorDataEntry.breakdown;
            sensorDataMessageToSend.pumpCapacity = sensorDataEntry.pumpCapacity;
            sensorDataMessageToSend.pumpUnusedTime = sensorDataEntry.pumpUnusedTime;
            sensorDataMessageToSend.pumpUsage = sensorDataEntry.pumpUsage;
            sensorDataMessageToSend.dryStrokes = sensorDataEntry.dryStrokes;
            sensorDataMessageToSend.dryStrokeHeight = sensorDataEntry.dryStrokeHeight;


            //set the flags to true for the optional fields in the payload
            sensorDataMessageToSend.has_avgLiters = true;
            sensorDataMessageToSend.has_breakdown = true;
            sensorDataMessageToSend.has_dailyLiters = true;
            sensorDataMessageToSend.humidityPerHour_count = 0; // do not send humidity data, will be 0
            sensorDataMessageToSend.litersPerHour_count = APP_NVM_SAMPLES_PER_DAY;
            sensorDataMessageToSend.has_pumpCapacity = true;
            sensorDataMessageToSend.tempPerHour_count = APP_NVM_SAMPLES_PER_DAY;
            sensorDataMessageToSend.has_totalLiters = true;
            sensorDataMessageToSend.has_pumpUnusedTime = true;
            sensorDataMessageToSend.has_pumpUsage = true;
            sensorDataMessageToSend.has_dryStrokes = true;
            sensorDataMessageToSend.has_dryStrokeHeight = true;

            //if stroke detection is enabled, send the stroke info to the cloud
            if ( xIsStrokeDetectionEnabled == true )
            {
                sensorDataMessageToSend.strokeHeightPerHour_count = APP_NVM_SAMPLES_PER_DAY;
                sensorDataMessageToSend.strokesPerHour_count = APP_NVM_SAMPLES_PER_DAY;
            }
            else
            {
                sensorDataMessageToSend.strokeHeightPerHour_count = 0;
                sensorDataMessageToSend.strokesPerHour_count = 0;
            }


            //if stroke detection is enabled, send the stroke info to the cloud
            if ( xIsStrokeDetectionEnabled == true )
            {
                sensorDataMessageToSend.strokeHeightPerHour_count = APP_NVM_SAMPLES_PER_DAY;
                sensorDataMessageToSend.strokesPerHour_count = APP_NVM_SAMPLES_PER_DAY;
            }
            else
            {
                sensorDataMessageToSend.strokeHeightPerHour_count = 0;
                sensorDataMessageToSend.strokesPerHour_count = 0;
            }


            // Queue up the sensor data message
            if (MQTT_sendSensorDataMsg(sensorDataMessageToSend))
            {
                status = true;
            }
        }
        else
        {
            elogError("couldnt get data log");
        }
    }
    else
    {
        elogInfo("No data logs");
    }

    if (status == false)
    {
        MQTT_IndicateReadyForNewJobs();
    }

    return status;
}

static void xPackageAndStoreSensorDataToFlash(void)
{
    APP_NVM_SENSOR_DATA_WITH_HEADER_T sensorData;
    xSensorData = SSM_getSensorData();

    sensorData.productId = PRODUCT_ID;
    sensorData.timestamp = xSensorData.timestamp;
    sensorData.msgNumber = MEM_getMsgNumber();
    sensorData.fwVersionMaj = VERSION_MAJOR;
    sensorData.fwVersionMinor = VERSION_MINOR;
    sensorData.fwVersionBuild = VERSION_BUILD;
    sensorData.batteryVoltage = xSensorData.batteryVoltage;
    sensorData.powerRemaining = xSensorData.powerRemaining;
    sensorData.state = xSensorData.state;
    sensorData.activatedDate = xSensorData.activatedDate;
    sensorData.magnetDetected = xSensorData.magnetDetected;
    sensorData.errorBits = xSensorData.errorBits;


    sensorData.numSSMResets = xSensorData.unexpectedResets;
    sensorData.lastSSMResetDate = xSensorData.timestampOfLastReset;
    sensorData.numAMResets = MEM_getUnexpectedResetCount();
    sensorData.lastAMResetDate = MEM_getTimestampLastUnexpectedReset();
    // TODO: Add debug log

    memcpy(&sensorData.litersPerHour, &xSensorData.litersPerHour, sizeof(xSensorData.litersPerHour));
    memcpy(&sensorData.tempPerHour, &xSensorData.tempPerHour, sizeof(xSensorData.tempPerHour));
    memcpy(&sensorData.humidityPerHour, &xSensorData.humidityPerHour, sizeof(xSensorData.humidityPerHour));
    memcpy(&sensorData.strokesPerHour, &xSensorData.strokesPerHour, sizeof(xSensorData.strokesPerHour));
    memcpy(&sensorData.strokeHeightPerHour, &xSensorData.strokeHeightPerHour, sizeof(xSensorData.strokeHeightPerHour));
    sensorData.dailyLiters = xSensorData.dailyLiters;
    sensorData.avgLiters = xSensorData.avgLiters;
    sensorData.totalLiters = xSensorData.totalLiters;
    sensorData.breakdown = xSensorData.breakdown;
    sensorData.pumpCapacity = xSensorData.pumpCapacity;
    sensorData.pumpUnusedTime = xSensorData.pumpUnusedTime;
    sensorData.pumpUsage = xSensorData.pumpUsage;
    sensorData.dryStrokes = xSensorData.dryStrokes;
    sensorData.dryStrokeHeight = xSensorData.dryStrokeHeight;

    // Store the message to flash
    if (MEM_writeSensorDataLog( &sensorData ))
    {
        MEM_updateMsgNumber();

        elogDebug("get next");

        // Let ssm know that the sensor data message has been stored to flash
        SSM_sensorDataStoredToFlash();
    }
}

static void xHandleSensorDataReady(void)
{
    elogInfo("sensor data ready");

    // All new sensor data entries received from the SSM and stored to flash
    xSensorDataReady = true;
}

static void xHandleMqttReady(void)
{
    //set flag
    xAwsConnected = true;

    //set connection time
    awsConnectTimeMs = NW_getCellOnTimeMs();

   //send any messages that we have buffered
    if ( xWaitingOnCell == true )
    {
        //first check if we have some gps data that needs to go out:
        if ( MEM_GetGpsFixedFlag() == true && MEM_GetGpsSentToCloud() == false)
        {
            xPackageAndSendGpsMsgToCloud();
            MEM_SetGpsSentToCloud(true);
        }

       if (xSensorDataReady == true)
       {
           xSensorDataReady = false;
           xPackageAndSendSensorDataToCloud();
       }
       else
       {
           xPackageAndSendStatusToCloud();
           MQTT_IndicateReadyForNewJobs();
       }
    }
    else
    {
       elogInfo("Request new jobs - cell is ready");
       //send a request for new jobs since we dont have any data buffered to send
       MQTT_IndicateReadyForNewJobs();
    }


    if (xUpdateImageOpState == true)
    {
       //set the loaded image to fully functional at this point
       MEM_setImageOpState(FULL, xCurrentSlot);

       elogInfo("Updated current slot to FULLY functional");
       xUpdateImageOpState = false;
    }

    //reset manufacturing flag if it was previously set
    if ( MEM_getMfgCompleteFlag() == true )
    {
        MEM_setMfgCompleteFlag(false);
    }
}

static void xHandleCheckInDeactivated(void)
{
    if ( xWaitingOnCell == false )
    {
        //start cell task
        CONN_init(xSsmStatus.timestamp);

        //we need to send a status message once the network is up and running
        xWaitingOnCell = true;
    }

    //request a new ssm status message to include in the payload
    SSM_requestStatusFromSsm();
}

static void xHandleCheckInActivated(void)
{
    if ( xWaitingOnCell == false )
    {
        //start cell task
        CONN_init(xSsmStatus.timestamp);

        //we need to send a status message once the network is up and running
        xWaitingOnCell = true;
    }

    //request a new ssm status message to include in the payload
     SSM_requestStatusFromSsm();


    //request sensor data messages to include in the payload
    SSM_requestNumDataLogEntriesFromSsm();
}

static void xInitiateNtpTimeSync(void)
{
    // set a flag to indicate that we want to do a time sync
    NW_timeSyncRequested(true);
}

static void xHandleResetAlarmsCloudCmd(void)
{
    //send command to the SSM
    SSM_sendResetAlarms();

    //get new jobs
    MQTT_indicateOperationPass();
    MQTT_IndicateReadyForNewJobs();
}

static void xHandleHwResetCloudCmd(bool deactivateBeforeReset)
{
    bool operationpassed = true;
    bool ssmAckedResetCmd = false;

    //send a spi message to the SSM to do a reset
    if ( deactivateBeforeReset == true )
    {
        elogInfo("Deactivating before hw reset");

        //delete any saved sensor data
        operationpassed = MEM_defaultSection(SECTION_DATA);

        if ( operationpassed == true )
        {
            SSM_deactivateionReceivedFromCloud();

            vTaskDelay(WAIT_ONE_SECOND);

            //tell the SSM manager to request the ssm status
            SSM_requestStatusFromSsm();

            //delay a second to allow the SSM to finish deactivating/sending stat
            vTaskDelay(WAIT_ONE_SECOND);

            //read the latest status
            xSsmStatus = SSM_getStatus();
        }

        //check the ssm status to determine if we have passed or failed
        if ( xSsmStatus.activatedState == eState_DEACTIVATED )
        {
            operationpassed = true;
        }
        else
        {
            operationpassed = false;
            elogError("SSM has not deactivated");
        }
    }

    if ( operationpassed == true )
    {
        //send the reset command, make sure ssm acks the msg
        ssmAckedResetCmd = SSM_sendHwResetCmd();

        //now do a hw reset - ssm will reset us in 10 seconds if ACKED
        if (ssmAckedResetCmd == true )
        {
            MQTT_indicateOperationPass();
        }
        else
        {
            //send back failed to the cloud and get next
            MQTT_indicateOperationFail();
            MQTT_IndicateReadyForNewJobs();
        }
    }
    else
    {
        //send back failed to the cloud and get next
        MQTT_indicateOperationFail();
        MQTT_IndicateReadyForNewJobs();
    }
}

static void xInitMfgCompleteTimer(uint32_t timerSeconds)
{

    //set up timer - has 'timerSeconds' period, single shot, and triggers the cb function when elapsed
    xManfCompleteTimerHandle = xTimerCreate("ManfCompleteTimer", pdMS_TO_TICKS(timerSeconds * 1000),
                                             pdFALSE, ( void * ) 0, vMfgTimerCallback);

    if( xManfCompleteTimerHandle == NULL )
    {
        elogError("Couldn't create timer");
    }
    else
    {
         //start timer
         if( xTimerStart( xManfCompleteTimerHandle, 300 ) != pdPASS )
         {
             elogError("Couldnt set timer");
         }
         else
         {
             elogInfo("Started %lu second timer for cloud message to be sent", timerSeconds);

             //increase the allowed time on so that we dont go to sleep early
             xAllowedTimeOn = AM_ALLOWED_TIME_ON_MS + (timerSeconds*1000);
         }
    }
}

static void xHandleGpsJob(bool newMeasurement)
{
    GpsMessage clearedGpsMsg = {};

    if( newMeasurement == true )
    {
        //clear old data
        MEM_UpdateGpsCoordinates(clearedGpsMsg);
        MEM_SetGpsNumRetries(0);
        MEM_SetGpsSentToCloud(false);
        MEM_SetGpsFixedFlag(false);

        //turn on GPS
        GPS_Enable();

        //send the MQTT jobs update msg
        MQTT_indicateOperationPass();
    }
    else
    {
        //cell should be on & connected to get to this point in the field
        if ( xAwsConnected == true )
        {
            //if we have data send it and send back that the job succeeded
            if ( MEM_GetGpsFixedFlag() == true )
            {
                xPackageAndSendGpsMsgToCloud();

                //send the MQTT jobs update msg
                MQTT_indicateOperationPass();
            }
            else
            {
                //send the MQTT jobs update msg
                MQTT_indicateOperationFail();
            }
        }
    }

    if ( xAwsConnected == true )
    {
        //request the next job
        MQTT_IndicateReadyForNewJobs();
    }
}

static void xHandleConfigs(configureMsg_t configsRcd)
{
    bool result = false;

    //Validate and store new transmission rate
    result = MEM_writeAmWakeRate(xIncomingEvent.payload.configs.transmissionRateDays);

    if ( result == true )
    {
        result = MEM_writeStrokeDetectionEnabledFlag(xIncomingEvent.payload.configs.strokeAlgIsOn);
    }

    if ( result == true )
    {
        //stroke on/off config
        SSM_setAlgoConfig(xIncomingEvent.payload.configs.strokeAlgIsOn);

        //set up & check red flag on/off thresholds
        result = SSM_setRedFlagThresholdConfigs(xIncomingEvent.payload.configs.redFlagOnThreshold, xIncomingEvent.payload.configs.redFlagOffThreshold);
    }

    //if we validated the first config and stored it, move along
    if ( result == true )
    {
        //store gps configs into flash (these are for AM only)
        result = MEM_writeGpsConfigs(xIncomingEvent.payload.configs.gpsTimeoutSeconds, xIncomingEvent.payload.configs.maxHop,
                xIncomingEvent.payload.configs.minMeasureTime, xIncomingEvent.payload.configs.numOfSatellites);

        //send to SSM as the last step
        SSM_sendConfigs();
    }

    //cell should be on & connected to get to this point in the field
    //If issued through the CLI then do not do this part
    if ( xAwsConnected == true )
    {
        if ( result == true)
        {
            //send the MQTT jobs update msg
            MQTT_indicateOperationPass();
        }
        else
        {
            MQTT_indicateBadConfigs();
        }

        //request the next job
        MQTT_IndicateReadyForNewJobs();
    }
}

static void xOtaFwDownloadSuccessful(void)
{
    uint32_t ssmPrimaryImageAddr = 0;
    uint32_t ssmImageProgrammed = false;

    //Alert the SSM we are about to do a fw update
    //It will have 4 seconds to get ready -

    //take over the watchdog kick
    WD_initKick();

    //give the ssm a heads up
    SSM_enableBootPin();

    //send the MQTT jobs update msg
    MQTT_indicateOperationPass();
    vTaskDelay(WAIT_ONE_SECOND);

    //now disconnect
    MQTT_disconnect();
    vTaskDelay(WAIT_ONE_SECOND*3);

    //turn off cell to save power
    PWR_turnOffCellModemPowerSupply();

    //now reset the boot pin
    SSM_disableBootPin();

    //it should save off any necessary things
    imageSlotTypes_t slot = MEM_getPrimaryImage();

    //now update the ssm:
    if ( slot == A )
    {
        ssmPrimaryImageAddr = APP_MEM_ADR_FW_APPLICATION_SSM_A_START;
    }
    else if ( slot == B )
    {
        ssmPrimaryImageAddr = APP_MEM_ADR_FW_APPLICATION_SSM_B_START;
    }
    else
    {
        elogError("Bad primary slot");
        return;
    }

    ssmImageProgrammed = SSM_FW_programBslWithExternalFlashImage(ssmPrimaryImageAddr);

    if ( ssmImageProgrammed != true )
    {
        elogError("SSM BSL DID NOT ACCEPT THE IMAGE. Try one more time");
        //try one more time
        ssmImageProgrammed = SSM_FW_programBslWithExternalFlashImage(ssmPrimaryImageAddr);

        //if it worked this time
        if ( ssmImageProgrammed == true )
        {
            //reset ourself here!
            MEM_setResetsSinceLastLpMode(0);

            //we will now go into the bootloader and update the FW to the new slot!!!
            NVIC_SystemReset();
        }
        else
        {
            elogError("SSM BSL DID NOT ACCEPT THE IMAGE. Set this slot to FAILED");

            //DONT update the STM to this new image since the ssm didnt accept it
            if ( slot == A )
            {
               MEM_setImageAoperationalState(OP_FAILED);
               MEM_setPrimaryImage(B);

               //rollback the ssm image
               elogError("Rolling back ssm to image in slot B");

               ssmImageProgrammed = SSM_FW_programBslWithExternalFlashImage(APP_MEM_ADR_FW_APPLICATION_SSM_B_START);

               //try again if doesnt program
               if ( ssmImageProgrammed != true )
               {
                   ssmImageProgrammed = SSM_FW_programBslWithExternalFlashImage(APP_MEM_ADR_FW_APPLICATION_SSM_B_START);
               }
            }
            else
            {
                MEM_setImageBoperationalState(OP_FAILED);
                MEM_setPrimaryImage(A);

                //rollback the ssm image
                elogError("Rolling back ssm to image in slot A");

                ssmImageProgrammed = SSM_FW_programBslWithExternalFlashImage(APP_MEM_ADR_FW_APPLICATION_SSM_A_START);


                //try again if doesnt program
                if ( ssmImageProgrammed != true )
                {
                    ssmImageProgrammed = SSM_FW_programBslWithExternalFlashImage(APP_MEM_ADR_FW_APPLICATION_SSM_B_START);
                }
            }

            //check flag again
            if ( ssmImageProgrammed == true )
            {
                elogInfo("Rolled back SSM to previous image. Entering LP mode now");
                xTurnOffCellAndPowerDown();
            }
            else
            {
                elogError("SSM IS WITHOUT A WORKING IMAGE");
                //TODO ok so what do we do in this case??
            }
        }
    }
    else
    {
        //reset ourself here!
        MEM_setResetsSinceLastLpMode(0);

        //we will now go into the bootloader and update the FW to the new slot!!!
        NVIC_SystemReset();
    }

}

static void xTurnOffCellAndPowerDown(void)
{
	//gracefully shut off the modem
	PWR_turnOffCellModemPowerSupply();
	elogInfo("Turned off Modem");

    //reset flag
    xAwsConnected = false;

    //any other things we need to do? Add here:

    //clear reset counter
    MEM_setResetsSinceLastLpMode(0);

    if ( xTestMode == false )
    {
        //Now enter standby mode - Wake up from the SSM GPIO line
        elogInfo("Entering Standby Mode");
        PWR_enterStandbyMode();
    }
}

static void commandHandlerForApp(int argc, char **argv)
{
    if ( argc == ONE_ARGUMENT && 0 == strcmp(argv[FIRST_ARG_IDX], "deact") )
    {
        elogInfo("Adding deactivate event");
        EVT_indicateDeActivate();
    }
    else if ( argc == ONE_ARGUMENT && 0 == strcmp(argv[FIRST_ARG_IDX], "act") )
    {
        elogInfo("Adding activate event");
        EVT_indicateActivateFromCloud();
    }
    else if ( argc == ONE_ARGUMENT && 0 == strcmp(argv[FIRST_ARG_IDX], "check") )
    {
        elogInfo("Adding check in event");
        EVT_indicateCheckInDeactivated();
    }
    else if ( argc == TWO_ARGUMENTS && 0 == strcmp(argv[FIRST_ARG_IDX], "mode") )
    {
        if ( 0 == strcmp(argv[SECOND_ARG_IDX], "test") )
        {
            xTestMode = true;
            elogInfo("Test mode on");

        }
        else if ( 0 == strcmp(argv[SECOND_ARG_IDX], "prod") )
        {
            xTestMode = false;
            elogInfo("Test mode off");
        }
        else
        {
            elogInfo("invalid mode");
        }
    }
    else if ( argc == ONE_ARGUMENT && 0 == strcmp(argv[FIRST_ARG_IDX], "mfgComplete") )
    {
        MEM_setMfgCompleteFlag(true);
        elogInfo("Set manufacturing complete & starting timer");

        xInitMfgCompleteTimer(MEM_getSecondsToWaitForMfgComplete());
    }
    else if ( argc == TWO_ARGUMENTS && 0 == strcmp(argv[FIRST_ARG_IDX], "waitTime") )
    {
        uint32_t secs = 0;
        char *p;
        secs = strtol(argv[SECOND_ARG_IDX], &p, 10);
        elogInfo("Setting wait time in seconds to %lu", secs);

        MEM_setSecondsToWaitForMfgComplete(secs);
    }
    else if ( argc == TWO_ARGUMENTS && 0 == strcmp(argv[FIRST_ARG_IDX], "resetcount") )
    {
        uint32_t counts = 0;
        char *p;
        counts = strtol(argv[SECOND_ARG_IDX], &p, 10);

        elogInfo("Setting reset counts to %lu", counts);
        MEM_setResetsSinceLastLpMode(counts);
    }
    else
    {
        elogInfo("Unknown command");
    }
}

