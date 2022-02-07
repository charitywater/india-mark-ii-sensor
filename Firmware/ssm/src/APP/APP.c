/**************************************************************************************************
* \file     APP.c
* \brief    Top level application
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

#include "APP_CLI.h"
#include "APP_NVM.h"
#include "APP_ALGO.h"
#include "am-ssm-spi-protocol.h"
#include "HW_GPIO.h"
#include "HW_RTC.h"
#include "HW_BAT.h"
#include "uC_TIME.h"
#include "APP.h"
#include "HW_ENV.h"
#include "HW_MAG.h"

//this is non configurable
#define WAKE_RATE_DEACTIVATED_DAYS          28
#define TIME_SYNC_RATE                      7 // once every 7 days sync up with NTP. Align with next wakeup, so could be 7+ days
#define FIRST_HOUR_OF_DAY                   0
#define WAITING_ON_SYS_RECOVERY_MINS        20*60 //20 minutes
#define LITERS_TO_ACTIVATE                  50
#define SECONDS_TO_WAIT_FOR_HW_RST_CMD      10
#define LATE_ALGO_RUNS_THRESHOLD            20
#define TIMEOUT_WAIT_ON_PUMP_ACTIVE_MINS    10
#define POWER_UP_TIME_SYNC_WAIT_MINS        30
#define POWER_UP_TIME_SYNC_WAIT_SECS        POWER_UP_TIME_SYNC_WAIT_MINS * SEC_PER_MIN

//6 runs is approximately 1 stroke
#define MISSED_ALGO_RUNS_THRESHOLD          6
#define MISSED_ALGO_RUN_WINDOW_SECS         3

//take new battery voltage readings 15 seconds after we turn on the AM
#define AM_ON_BATT_VOLTAGE_WAIT_TIME        15

//dont do a system reset for these error bits, a sys reset will either not fix
//the error, or the errors arent critical
#define NON_CRITICAL_ERR_BIT_MASK          (TEMP_HUMID_ERROR|NO_RTC_TIME)


static APP_NVM_SENSOR_DATA_T sensorData =
{
    .timestamp = 0,
    .litersPerHour = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    .tempPerHour = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    .humidityPerHour = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    .strokesPerHour = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    .strokeHeightPerHour = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    .avgLiters = 0,
    .dailyLiters = 0,
    .totalLiters = 0,
    .breakdown = false,
    .pumpCapacity = 0,
    .batteryVoltage = 3600,
    .powerRemaining = 100,
    .state = FAULT,
    .magnetDetected = true,
    .errorBits = 0,
    .unexpectedResets = 0,
    .timestampOfLastReset = 0,
    .activatedDate = 0,
    .checksum = 0
};

static asp_attn_source_payload_t xCurrentAttnList;
static app_state_t xCurrentState = FAULT;
static reset_state_t xResetState = STATE_POR;
static uint16_t xTransmissionRateInDays = WAKE_RATE_DEACTIVATED_DAYS;
static uint8_t xTimeSyncStatus = RTC_FIRST_TIME_SYNC;
static uint32_t xLastTimeSync = 0u;
static uint32_t xCurrentTimeStamp = 0u;
static uint32_t xLastWakeupTime = 0u;
static uint64_t xlastAlgorithmRun = 0u;
static uint8_t currentHrIdx = 0u;
static bool dailySensorDataRdy = false; // Flag to indicate that the daily sensor message is ready to be sent to eeprom
static bool activeSampling = false;     // Flag to indicate that the pump is in use and we are actively sampling
static bool xFirstHourOfSensorData = true;
static uint32_t xLastDailyTimeAdjust = 0u;
static uint32_t xLastRtcEpoch = 0u;
static uint32_t xCurrentErrors = 0u;
static bool waitingOnAm = false;
static uint32_t xWaitOnAmStartTime = 0u;
static uint32_t xErrorStartTimeSeconds = 0;
static uint32_t xWaitTimeForFaultStatusMsg = 0u;

//we will prevent sensor data storage until we have a valid time
static bool xValidTimestamp = false;
static uint32_t xLastTimestampRetryTime = 0u;
static uint32_t xTimeWaitingOnPumpActive = 0u;
static bool xStartTimerForPumpActive = false;
static bool xPumpActiveTimerExpired = false;
static bool xWaitingOnTimeSync = false;
static bool xTimerForVoltageReadingOn = false;
static uint32_t xTimerForVoltReadingStartTime = 0u;
static bool xNewRedFlagDetected = false;
static bool xisPumpHealthRunning = false;

void APP_init(void);
void APP_handleAttnSourceRequest(void);
void APP_handleAttnSourceAck(asp_attn_source_payload_t *pMsg);
void APP_indicateActivation(void);
void APP_indicateError(uint32_t errorBit);
void APP_indicateErrorResolved(uint32_t errorBit);
void APP_indicateNeedTime(void);
void APP_indicateCheckIn(void);
void APP_handleActivateCmd(void);
void APP_handleDeactivateCmd(void);
void APP_indicateInvalidSpiMsg(void);
app_state_t APP_getState(void);
reset_state_t APP_getResetState(void);
void APP_periodic(void);
void APP_handleConfigs(uint32_t transmissRate, bool strokeAlgIsOn, uint16_t redFlagOnThresh, uint16_t redFlagOffThresh);

static void xResetSensorData(void);
static void xUpdateSensorData(void);
static void xAddTimestampToDataLog(void);
static void xPollBootPinAndEnterBslIfRequested(void);
static void xWakeAm(void);
static void xPowerCycleSystem(void);
static void xInitCurrentRtcHrIdxAndResetAlgoData(bool resetAlgoHourlyData);
static void xCheckRtcAlignmentAndSensorDataLogging(void);
static void xSetCurrentHourIdx(uint8_t hr);
static uint8_t xGetCurrentHourIdx(void);
static void xRunAlgoDiagnostics(uint64_t timeLastRan, uint64_t currentTime);

static uint8_t appString[150];

void APP_init(void)
{
    APP_CLI_Init();
    APP_NVM_Init();
    APP_ALGO_Init();

    //Get the high level state and reset state(tells us if there have been any non planned resets)
    xCurrentState = (app_state_t)APP_NVM_Custom_GetHighLevelState();
    xResetState = (reset_state_t)APP_NVM_Custom_GetResetStateAndInit();
    xTimeSyncStatus = APP_NVM_Custom_GetRtcTimeStatus();
    xTransmissionRateInDays = APP_NVM_Custom_GetAmTransmissionRate();

    //algo configs
    APP_ALGO_initRedFlagThresholds(APP_NVM_Custom_GetRedFlagOnThreshold(), APP_NVM_Custom_GetRedFlagOffThreshold());

    //if deactivated, ignore the value we read out of NVM and use the default of 28 days
    if ( xCurrentState == DEACTIVATED )
    {
        xTransmissionRateInDays = WAKE_RATE_DEACTIVATED_DAYS;
    }
    else if ( xCurrentState == ACTIVATED && APP_NVM_Custom_GetStrokeDetectionIsOn() == true )
    {
        //this is off by default, turn on the magnetometer & setup the algo
        HW_MAG_InitSampleRateAndPowerModeOn();
        APP_ALGO_setStrokeDetectionIsOn(true);
    }

    sprintf((char *)appString, "\n\rHigh level state: %d \n\r Tx rate %d\n\r", xCurrentState, xTransmissionRateInDays);
    HW_TERM_Print(appString);

    //init attention source list
    memset(&xCurrentAttnList, 0, sizeof(asp_attn_source_payload_t));

    //check the rtc
    xValidTimestamp = HW_RTC_CheckValidTime();

    if ( xValidTimestamp == true )
    {
        xCurrentTimeStamp = HW_RTC_GetEpochTime();
        xLastRtcEpoch = HW_RTC_GetEpochTime();

        //if activated, see when the next hour will be
        //otherwise adjust the '1 day' time period for accounting
        //for daily drift
        xInitCurrentRtcHrIdxAndResetAlgoData(false);
    }
    else
    {
        HW_TERM_Print("\r\n We do not have a valid time! \r\n");
        xTimeSyncStatus = RTC_FIRST_TIME_SYNC;
        APP_NVM_Custom_WriteRtcTimeStatus(xTimeSyncStatus);

        //we need to set the RTC before doing anything non 'relative time' based
        APP_indicateError(NO_RTC_TIME);

        //set the flag for waiting 30 minutes and then requesting time from AM
        xWaitingOnTimeSync = true;
    }

    //if we have an unexpected reset, then update the counter
    if ( xResetState == STATE_ERR )
    {
        if ( xValidTimestamp == true )
        {
            APP_NVM_Custom_IncrementUnexpectedResetCount(xCurrentTimeStamp);
        }
        else
        {
            //still increment the counter, but the timestamp will just be 0
            APP_NVM_Custom_IncrementUnexpectedResetCount(0x00000000);
        }
    }
}

void APP_periodic(void)
{
    uint64_t xCurrentRuntimeTickVal = uC_TIME_GetRuntimeTicks();
    uint32_t xCurrentRuntimeSecVal = uC_TIME_GetRuntimeSeconds();

    //retry once/day to get a time from the AM if we are without a valid time
    if ( xValidTimestamp == false &&
            (uC_TIME_GetRuntimeSeconds() - xLastTimestampRetryTime) >= SEC_PER_DAY )
    {
        xLastTimestampRetryTime = uC_TIME_GetRuntimeSeconds();

        HW_TERM_Print("\r\n We still do not have a valid time! \r\n");

        //we need to set the RTC before doing anything non 'relative time' based
        APP_indicateError(NO_RTC_TIME);
        APP_indicateNeedTime();
    }

    //On power up we wait 30 minutes if we need an RTC time to allow the system to stabilize
    if ( xValidTimestamp == false && xWaitingOnTimeSync == true )
    {
        //check if its been 30 mins
        if ( xCurrentRuntimeSecVal >= POWER_UP_TIME_SYNC_WAIT_SECS)
        {
            xWaitingOnTimeSync = false;

            //now wake up the AM
            APP_indicateNeedTime();
        }
    }

    //deactivated case:
    //decide if we need to wake up the AM for a check in message, typically 28 days
    if (xCurrentState == DEACTIVATED &&
            (xCurrentRuntimeSecVal - xLastWakeupTime) >= (xTransmissionRateInDays * SEC_PER_DAY) )
    {
        //reset the wakeup time
        xLastWakeupTime = xCurrentRuntimeSecVal;

        //always get an updated RTC time during deactivated check ins
        xTimeSyncStatus = RTC_TIME_SYNC_PERIODIC;
        APP_NVM_Custom_WriteRtcTimeStatus(xTimeSyncStatus);

        //time to wake up the AM
        APP_indicateCheckIn();
    }

    // If it has been 50 ms, run the algorithm nest
    if ( (xCurrentRuntimeTickVal - xlastAlgorithmRun) >= UC_TIME_TICKS_PER_50MS )
    {
        xRunAlgoDiagnostics(xlastAlgorithmRun, xCurrentRuntimeTickVal);
        xlastAlgorithmRun = xCurrentRuntimeTickVal;

        APP_ALGO_Nest(activeSampling);

        if (xCurrentState != ACTIVATED)
        {
            sensorData.totalLiters = APP_ALGO_monitorTotalLiters();

            // Activate when the total liters count reaches 50
            if (sensorData.totalLiters >= LITERS_TO_ACTIVATE)
            {
                APP_indicateActivation();
            }
        }
    }

    //check if we need to update the hourly data, store the log to EEPROM,
    //or send data to the cloud IF we have a valid timestamp for the data!
    if ( xCurrentState == ACTIVATED && xValidTimestamp == true )
    {
        //determine if we should re-adjust the wake time using the RTC or log sensor data
        xCheckRtcAlignmentAndSensorDataLogging();

        // When there is a full days worth of sensor data, store to EEPROM/send to the cloud
        if ( dailySensorDataRdy == true )
        {
            if ( APP_ALGO_isWaterPresent() == false || xPumpActiveTimerExpired == true )
            {
                //log to EEPROM immediately
                APP_NVM_Custom_LogSensorData(&sensorData);

                dailySensorDataRdy = false;
                xResetSensorData();

                //check if we have enough data logs to send to the AM/cloud:
                if ( APP_NVM_Custom_GetSensorDataNumEntries() >= xTransmissionRateInDays || xNewRedFlagDetected == true )
                {
                    HW_TERM_Print("waking up the AM\r\n");

                    //check if we should get a new time stamp
                    if ( (xCurrentRuntimeSecVal - xLastTimeSync) >= (TIME_SYNC_RATE * SEC_PER_DAY) )
                    {
                        HW_TERM_Print("Request a time sync \r\n");
                        xLastTimeSync = xCurrentRuntimeSecVal;

                        xTimeSyncStatus = RTC_TIME_SYNC_PERIODIC;
                        APP_NVM_Custom_WriteRtcTimeStatus(xTimeSyncStatus);
                    }

                    //time to wake up the AM
                    APP_indicateCheckIn();

                    //reset flag
                    xNewRedFlagDetected = false;
                }

                xPumpActiveTimerExpired = false;
                xStartTimerForPumpActive = false;
            }
            else
            {
                //set timer
                if ( xStartTimerForPumpActive == false )
                {
                    xStartTimerForPumpActive = true;
                    xTimeWaitingOnPumpActive = uC_TIME_GetRuntimeSeconds();

                    HW_TERM_Print("pump active, started timer");
                }
                else
                {
                    //we have already started the timer, check if we have exceeded the timeout
                    if ( ( uC_TIME_GetRuntimeSeconds() - xTimeWaitingOnPumpActive )  >= ( TIMEOUT_WAIT_ON_PUMP_ACTIVE_MINS * SEC_PER_MIN ) )
                    {
                        HW_TERM_Print("pump active, timer expired");
                        xPumpActiveTimerExpired = true;
                    }
                }
            }
        }
    }

    if ( waitingOnAm == true )
    {
        //Keep retrying until we hit the timeout mark from logging an error and get reset
        if ( (uC_TIME_GetRuntimeSeconds() - xWaitOnAmStartTime) >= FIVE_MINUTES )
        {
            HW_TERM_Print("AM Comm Timeout, lets retry");
            APP_indicateError(AM_NOT_RESPONSIVE);

            //clear pin
            HW_GPIO_Clear_WAKE_AP();

            //retry
            xWakeAm();
        }
    }

    if ( xTimerForVoltageReadingOn == true )
    {
        if ( (xCurrentRuntimeSecVal - xTimerForVoltReadingStartTime ) >= AM_ON_BATT_VOLTAGE_WAIT_TIME )
        {
            xTimerForVoltageReadingOn = false;
            xTimerForVoltReadingStartTime = 0;

            //sample the battery voltage now and use this value until the AM is turned on again
            HW_BAT_TakeNewVoltageMeasurement();
        }
    }

    //if we have errors set, check how long the errors have been present and determine if we need to reset the system
    if ( xCurrentErrors > 0 )
    {
        //dont reset for 'no rtc time' or eeprom being full or temp sensor issue:
        if ( (xCurrentErrors & ~NON_CRITICAL_ERR_BIT_MASK) > 0 )
        {
            if ( (uC_TIME_GetRuntimeSeconds() - xErrorStartTimeSeconds) >= WAITING_ON_SYS_RECOVERY_MINS )
            {
                if ( xCurrentState != FAULT )
                {
                    HW_TERM_Print("Resetting System");

                    //Send a status message to the cloud to indicate that something is broken
                    xCurrentState = FAULT;
                    xWaitTimeForFaultStatusMsg = uC_TIME_GetRuntimeSeconds();
                    APP_indicateCheckIn();
                }
                else
                {
                    //wait some amount of time to allow the AM to finish up connecting to the cloud
                    //5 minutes for now
                    if ( uC_TIME_GetRuntimeSeconds() - xWaitTimeForFaultStatusMsg >= FIVE_MINUTES )
                    {
                        //Restart the system, hopefully this will fix the issue :)
                        xPowerCycleSystem();
                    }
                }
            }
        }
    }

    //if we have a timestamp, adjust the next wake time according to the drift this day
    //regardless of activation state

    if ( (uC_TIME_GetRuntimeSeconds() - xLastDailyTimeAdjust >= SEC_PER_DAY) && (xValidTimestamp == true) )
    {
        //set new time to adjust:
        xLastDailyTimeAdjust = uC_TIME_GetRuntimeSeconds();

        HW_TERM_Print("1 day time adjust");

        //make an adjustment to the next transmission rate based on drift for this day
        xLastWakeupTime -= (int)(SEC_PER_DAY - (HW_RTC_GetEpochTime() - xLastRtcEpoch));

        //set last updated epoch time:
        xLastRtcEpoch = HW_RTC_GetEpochTime();

        //reset any algo diagnostic errors for the day
        sensorData.errorBits &= (~AVG_SAMPLE_PERIOD_DRIFT);
        sensorData.errorBits &= (~MISSED_SAMPLE_THRESH);
    }

    //check BL pin periodically to see if we need to exit the application
    //(ensures we arent in the middle of an eeprom write or sensor data communication
    //when the AM begins to load a new image
    xPollBootPinAndEnterBslIfRequested();
}

app_state_t APP_getState(void)
{
    return xCurrentState;
}

reset_state_t APP_getResetState(void)
{
    return xResetState;
}

void APP_setNewRedFlagDetected(bool isNewRedFlagPresent)
{
    xNewRedFlagDetected = isNewRedFlagPresent;
}

void APP_handleConfigs(uint32_t transmissRate, bool strokeAlgIsOn, uint16_t redFlagOnThresh, uint16_t redFlagOffThresh)
{
    uint8_t wakeString[50];

    //save the new config to EEPROM - these have been verified already
    APP_NVM_Custom_WriteAmTransmissionRate(transmissRate);
    APP_NVM_Custom_WriteStrokeDetectionIsOn(strokeAlgIsOn);
    APP_NVM_Custom_WriteRedFlagOnThreshold(redFlagOnThresh);
    APP_NVM_Custom_WriteRedFlagOffThreshold(redFlagOffThresh);

    //reinit wake time with new config
    xTransmissionRateInDays = transmissRate;

    //also reinit stroke algo & magnetometer settings
    if ( strokeAlgIsOn == true )
    {
        if ( xCurrentState == ACTIVATED )
        {
            //start up the magnetometer
            HW_MAG_InitSampleRateAndPowerModeOn();

            //start running the stroke algo
            APP_ALGO_setStrokeDetectionIsOn(true);
        }
    }
    else
    {
        //put the magnetometer into low power mode
        HW_MAG_TurnOffSampling();

        //stop running the stroke algo
        APP_ALGO_setStrokeDetectionIsOn(false);
    }

    //pass new configs to the algo module
    APP_ALGO_initRedFlagThresholds(redFlagOnThresh, redFlagOffThresh);

    sprintf((char *)wakeString, "\n\rCONFIG MSG! Wake Rate: %d days, stroke detect: %d \n\r", xTransmissionRateInDays, strokeAlgIsOn);
    HW_TERM_Print(wakeString);

    sprintf((char *)wakeString, "\n\rRed flag on: %d %%, Red flag off: %d %% \n\r", redFlagOnThresh, redFlagOffThresh);
    HW_TERM_Print(wakeString);
}

uint32_t APP_getErrorBits(void)
{
    return xCurrentErrors;
}

void APP_indicateErrorResolved(uint32_t errorBit)
{
    xCurrentErrors &= ~errorBit;
}

void APP_indicateInvalidSpiMsg(void)
{
    //We reset the AM after bad communication
    HW_TERM_Print("Invalid SPI message. \n");
}

void APP_handleAttnSourceRequest(void)
{
    //send list to the AM
    ASP_TransmitAttnSourceList(&xCurrentAttnList);
}

void APP_handleAttnSourceAck(asp_attn_source_payload_t *pMsg)
{
    //clear any acked sources
    xCurrentAttnList.attnSourceList &= pMsg->attnSourceList;

    //clear attn line if all sources are acked
    if(xCurrentAttnList.attnSourceList == 0)
    {
        HW_GPIO_Clear_WAKE_AP();
        HW_TERM_Print("Received attn src ACK\n");

        //not waiting on the AM anymore, stop timer
        waitingOnAm = false;

        //if the error bit was set, we can clear it now
        if ( xCurrentErrors & AM_NOT_RESPONSIVE )
        {
            APP_indicateErrorResolved(AM_NOT_RESPONSIVE);
        }
    }
    else
    {
        HW_TERM_Print("Attn src ACK not cleared \n");
    }
}

void APP_indicateActivation(void)
{
    xCurrentAttnList.attnSourceList |= ASP_ACTIVATE;

    xWakeAm();
}

void APP_indicateMagnetometerThresholdInterrupt(void)
{
    //turn on sampling and update algorithm to run stroke detection
    APP_ALGO_setStrokeDetectionIsOn(true);
    HW_MAG_InitSampleRateAndPowerModeOn();

    //TODO Will actually probably call a 'compute dry strokes' function or something
    //like that instead based on convo this morning. Stubbing out here for now.
}

void APP_indicateError(uint32_t errorBit)
{

    //start timer if this is a non crit error bit & the errors are 0
    if ( (errorBit & (~NON_CRITICAL_ERR_BIT_MASK)) > 0 )
    {
        if ( xCurrentErrors == 0 || (xCurrentErrors & (~NON_CRITICAL_ERR_BIT_MASK)) == 0)
        {
            xErrorStartTimeSeconds = uC_TIME_GetRuntimeSeconds();
        }
    }

    xCurrentErrors |= errorBit;
}

void APP_indicateNeedTime(void)
{
    xCurrentAttnList.attnSourceList |= ASP_REQUEST_TIME;

    if ( APP_getErrorBits() & NO_RTC_TIME )
    {
        HW_TERM_Print("\r\n waking AM - need a timestamp \r\n");

        //if we are toally without a working time, send a status message to the cloud
        //via the deactivated check in event (will send the header only)
        xCurrentAttnList.attnSourceList |= ASP_CHECK_IN_DEACTIVATED;

        xWakeAm();
    }
}

void APP_indicateCheckIn(void)
{
    if ( xCurrentState == ACTIVATED )
    {
        xCurrentAttnList.attnSourceList |= ASP_CHECK_IN_ACTIVATED;
    }
    else
    {
        xCurrentAttnList.attnSourceList |= ASP_CHECK_IN_DEACTIVATED;
    }

    if (xTimeSyncStatus != RTC_TIME_UPDATED)
    {
        APP_indicateNeedTime();
    }

    xWakeAm();
}

void APP_handleActivateCmd(void)
{
    HW_TERM_Print("Received Activate Command\n");

    if ( xCurrentState != ACTIVATED )
    {
        //Write state to eeprom
        xCurrentState = ACTIVATED;
        APP_NVM_Custom_WriteHighLevelState(xCurrentState);

        //get time and update the activated date
        xCurrentTimeStamp = HW_RTC_GetEpochTime();
        APP_NVM_Custom_WriteActivatedDate(xCurrentTimeStamp);

        //init hourly data updating if we have a time
        if ( xValidTimestamp == true )
        {
            xInitCurrentRtcHrIdxAndResetAlgoData(false);
        }

        //new transmission/wake rate
        xTransmissionRateInDays = APP_NVM_Custom_GetAmTransmissionRate();

        // Reset the wake-time once activated
        xLastWakeupTime = uC_TIME_GetRuntimeSeconds();

        sprintf((char *)appString, "\n\rlast wake time set to: %lu sec \n\r", xLastWakeupTime);
        HW_TERM_Print(appString);

        //if stroke detection is enabled, turn on the magnetometer + pass to algo
        if ( APP_NVM_Custom_GetStrokeDetectionIsOn() == true )
        {
            HW_MAG_InitSampleRateAndPowerModeOn();
            APP_ALGO_setStrokeDetectionIsOn(true);
        }
    }
}

void APP_handleDeactivateCmd(void)
{
    uint8_t wakeString[50];
    HW_TERM_Print("Received Deactivate Command\n");

    if ( xCurrentState != DEACTIVATED )
    {
        xCurrentState = DEACTIVATED;

        //Write state to eeprom
        APP_NVM_Custom_WriteHighLevelState(xCurrentState);

        xCurrentTimeStamp = HW_RTC_GetEpochTime();
        APP_NVM_Custom_WriteDeactivatedDate(xCurrentTimeStamp);

        //wipe activated date
        APP_NVM_Custom_WriteActivatedDate(0x00000000);

        //clear sensor logs
        APP_NVM_DefaultSensorDataLogs();

        //Update the transmission/wake rate to the default
        xTransmissionRateInDays = WAKE_RATE_DEACTIVATED_DAYS;

        //put magnemometer in power down
        HW_MAG_TurnOffSampling();

        APP_ALGO_setStrokeDetectionIsOn(false);

        //Add anything else here, disable algorithm

        // Reset the sensor data
        xResetSensorData();

        //reset ALGO
        APP_ALGO_Init();
    }

    sprintf((char *)wakeString, "\n\rWake Rate: %d days \n\r", xTransmissionRateInDays);
    HW_TERM_Print(wakeString);
}

void APP_handleIncrementSensorDataCmd(void)
{
    APP_NVM_SensorDataMsgAcked();
}

bool APP_getPumpActive(void)
{
    return activeSampling;
}

void APP_setPumpActive(bool active)
{
    activeSampling = active;

    if ( activeSampling == true )
    {
        APP_ALGO_wakeUpInit();
    }
}

void APP_setTimeUpdated(void)
{
    xTimeSyncStatus = RTC_TIME_UPDATED;
    APP_NVM_Custom_WriteRtcTimeStatus(xTimeSyncStatus);

    xValidTimestamp = true;
    xInitCurrentRtcHrIdxAndResetAlgoData(true);

    //clear the error if present
    if ( APP_getErrorBits() & NO_RTC_TIME )
    {
        APP_indicateErrorResolved(NO_RTC_TIME);
    }
}

void APP_setTimeFailed(void)
{
    //if we still have a time, we will just try again the next time sync
    //but if we are without a working time, we need to try again later

    HW_TERM_Print("\r\n AM time sync failed \r\n");

    if ( APP_getErrorBits() & NO_RTC_TIME )
    {
        xValidTimestamp = false;

        HW_TERM_Print("\r\n Will try again in 24 hours \r\n");

        //set the current time so that we will try again in 1 day
        xLastTimestampRetryTime = uC_TIME_GetRuntimeSeconds();
    }
}

void APP_setTimeSyncStatus(uint8_t status)
{
    xTimeSyncStatus = status;
}

void APP_handleHwResetCommand(void)
{
    uint32_t startTime = uC_TIME_GetRuntimeSeconds();

    HW_TERM_Print("\r\n Received hardware reset command, waiting 10 seconds \r\n");

    //Reset in 10 seconds to allow the AM to finish
    //communicating with the cloud - hang out in this function
    //so we dont start doing something else

    while (uC_TIME_GetRuntimeSeconds() - startTime < SECONDS_TO_WAIT_FOR_HW_RST_CMD);

    //perform a system reset
    xPowerCycleSystem();
}

void APP_handleResetAlarmsCommand(void)
{
    HW_TERM_Print("\r\n Resetting Alarm data \r\n");

    //reset data
    APP_ALGO_resetRedFlagData();

    //reset flag
    xNewRedFlagDetected = false;
}

static void xUpdateSensorData(void)
{
    uint8_t printStr[100];
    HW_ENV_SAMPLE_T *xEnvSample;

    //hour index
    uint8_t hourIdx = xGetCurrentHourIdx();
    uint8_t logBufferIdx = 0;

    //static flag to track state
    static bool redFlagDetected = false;

    //start a temp/humidity conversion first thing
    HW_ENV_TriggerNewEnvSample();

    hourIdx++;

    if ( hourIdx  >= (MAX_HOUR_VALUE + 1) )
    {
        hourIdx = 0;
    }

    //set the new hour
    xSetCurrentHourIdx(hourIdx);

    //set position in the log to the hour index,
    //the log bufferIndex gets manipulated
    logBufferIdx = hourIdx;

    sprintf((char *)printStr, "\n\rHour %d \n\r", logBufferIdx);
    HW_TERM_Print(printStr);

    //first hour of the day we will update the timestamp:
    if ( xFirstHourOfSensorData == true )
    {
        xAddTimestampToDataLog();
    }

    // It is 12AM, time to update sensor data log
    if (logBufferIdx == FIRST_HOUR_OF_DAY)
    {
        //fill in the data that we only do once/day
        sensorData.batteryVoltage = HW_BAT_GetVoltage(); // get the last voltage reading we took
        sensorData.powerRemaining = 100;   // TODO: Update when we have new fuel gauge
        sensorData.state = xCurrentState;
        sensorData.activatedDate = APP_NVM_Custom_GetActivatedDate();
        sensorData.unexpectedResets = APP_NVM_Custom_GetUnexpectedResetCount();
        sensorData.timestampOfLastReset = APP_NVM_Custom_GetTimestampLastUnexpectedReset();
        sensorData.errorBits |= xCurrentErrors;

        logBufferIdx = APP_NVM_SAMPLES_PER_DAY-1;
        dailySensorDataRdy = true;
        xFirstHourOfSensorData = true;
    }
    else
    {
        logBufferIdx--;
    }

    //update the fields for this hour
    APP_ALGO_updateHourlyFields(&sensorData, (logBufferIdx));

    if ( logBufferIdx == APP_NVM_SAMPLES_PER_DAY-1 )
    {
        //after we have updated hourly fields, calculate the daily liters
        //if it is the final sample of the day
        APP_ALGO_updateDailyFields(&sensorData);

        //check if new red flag event
        if ( sensorData.breakdown == true )
        {
            if ( redFlagDetected == false )
            {
                APP_setNewRedFlagDetected(true);
                HW_TERM_Print("new red flag");

                //set static variable to prevent constantly triggering sensor data
                //messages to go out
                redFlagDetected = true;
            }
        }
        else
        {
            redFlagDetected = false;
        }
    }

    // Get the new env sample
    HW_ENV_GetNewEnvSample();
    xEnvSample = HW_ENV_GetLatestSample();
    sensorData.tempPerHour[logBufferIdx] = xEnvSample->temp_c;
}

static void xWakeAm(void)
{
    uint32_t currentTime = uC_TIME_GetRuntimeSeconds();

    //init the timer to wait on the AM - this is to detect comm failures
    xWaitOnAmStartTime = currentTime;
    waitingOnAm = true;

    //wait 15 seconds and then take a voltage reading
    xTimerForVoltageReadingOn = true;
    xTimerForVoltReadingStartTime = currentTime;

    //Assert the wake/attention source line
    HW_GPIO_Set_WAKE_AP();
}

static void xPowerCycleSystem(void)
{

    //reset the entire system
    HW_GPIO_Set_SYS_OFF();

    //if we havent reset in a few seconds...something is wrong
}

static void xPollBootPinAndEnterBslIfRequested(void)
{
    //periodically check the boot pin
    //The AM will give us a heads up when it is time to do a FW upgrade
    //This is a hardware specific and messaging independent way to enter BL mode & ensure that
    //we are not in the middle of an eeprom write or sensor transaction when the AM pulls the reset line
    if ( HW_GPIO_Read_SSM_BOOT() == true )
    {
        //Save off any critical data - for now there is not any data
        //that we need to save. Could lose up to 1 hour's worth of alg data
        //but this will recover gracefully.

        //update reset state so we know this wasnt random or a HW reset
        APP_NVM_Custom_WriteResetState(STATE_SWR);

        HW_TERM_Print("Ready for OTA - Jumping to BSL \n");

        //Update program counter to put us in BL mode
        __disable_interrupt();

        // jump to BSL - we dont exit this until the AM is finished updating with the new image
       ((void (*)())0x1000)();
    }
}

static void xInitCurrentRtcHrIdxAndResetAlgoData(bool resetAlgoHourlyData)
{
   //init hourly data updating if we have a time & are activated
    if ( xValidTimestamp == true  )
    {
        uint32_t currentRuntimeS = uC_TIME_GetRuntimeSeconds();

        if ( xCurrentState == ACTIVATED )
        {
            //adjust next hourly data update
            uint16_t secToNextHr = 0;
            uint8_t rtcHr = 0;
            int32_t nextHourTimeAdjustmentInt = 0;

            //print out the rtc time
            HW_RTC_ReportTime();

            //get seconds till the next hour and the current hour
            HW_RTC_GetSecToNextHour(&secToNextHr);
            HW_RTC_GetHour(&rtcHr);

            //if there are less than 5 minutes until the next hour,
            //wait one hour extra hr for the next hourly time adjustment to
            //prevent us logging data twice in once day
            if ( secToNextHr <= (FIVE_MINUTES) )
            {
                rtcHr++;
                nextHourTimeAdjustmentInt = (SEC_PER_HOUR - secToNextHr) - SEC_PER_HOUR;
            }
            else
            {
                nextHourTimeAdjustmentInt = SEC_PER_HOUR - secToNextHr;
            }

            //check for wrap around
            if ( rtcHr  >= (MAX_HOUR_VALUE + 1) )
            {
                rtcHr = 0;
            }

            //initialize the current RTC hour
            xSetCurrentHourIdx(rtcHr);
            uC_TIME_SetHourlyTimeAdjustSeconds(nextHourTimeAdjustmentInt);

            xFirstHourOfSensorData = true;

            //Reset algo hourly data. This is to clear out any data that was
            //collected while we were without a timestamp.
            if ( resetAlgoHourlyData == true )
            {
                APP_ALGO_resetHourlyStrokeCount();
                APP_ALGO_getHourlyWaterVolume();
            }
        }

        //Adjust the "day" period - mostly for the deactivated case but lets
        //do it while activated in case we need this in the future
        xLastDailyTimeAdjust = currentRuntimeS;
        xLastRtcEpoch = HW_RTC_GetEpochTime();
    }
}

static void xCheckRtcAlignmentAndSensorDataLogging(void)
{
    int32_t offset = 0;
    uint8_t hr = 0;
    uint8_t previousHour = 0;
    int32_t secsSinceLastAdjustment = uC_TIME_GetHourlyTimeAdjustSeconds();

    //if its been ~ 1 hour since the last time we checked
    if (  secsSinceLastAdjustment >= (int32_t)SEC_PER_HOUR )
    {
        uint32_t secsSinceMidnight = HW_RTC_GetSecondsSinceMidnight();

        //get the last hour
        //TODO some refactoring in this function and xUpdateSensorData()
        //later to make things more readable
        previousHour = xGetCurrentHourIdx();

        //update sensor data + get the new hour Index
        xUpdateSensorData();

        if ( xisPumpHealthRunning == true )
        {
            //turn off now
            if ( APP_NVM_Custom_GetStrokeDetectionIsOn() == false )
            {
                HW_MAG_TurnOffSampling();
                APP_ALGO_setStrokeDetectionIsOn(false);
            }

            //run the pump health algo & reset flag
            APP_ALGO_computePumpHealth(previousHour);
            xisPumpHealthRunning = false;
            HW_TERM_Print("Pump health off");
        }

        hr = xGetCurrentHourIdx();

        //if we have rolled over and seconds since midnight is more than half an hour
        if ( hr == 0 && secsSinceMidnight > (SEC_PER_HOUR/2) )
        {
            offset = (int32_t)secsSinceMidnight - (int32_t)(SEC_PER_DAY);
        }
        else
        {
            offset = (int32_t)secsSinceMidnight - ((int32_t)hr * (int32_t)SEC_PER_HOUR);
        }

        uC_TIME_SetHourlyTimeAdjustSeconds(offset);

        //if the hour is the max used hour for this day, do pump health
        if ( APP_ALGO_getMaxHourUsage() == hr )
        {
            if ( APP_NVM_Custom_GetStrokeDetectionIsOn() == false )
            {
                APP_ALGO_setStrokeDetectionIsOn(true);
                HW_MAG_InitSampleRateAndPowerModeOn();
            }

            HW_TERM_Print("Pump health running");
            xisPumpHealthRunning = true;
        }
    }
}

static void xSetCurrentHourIdx(uint8_t hr)
{
    currentHrIdx = hr;
}

static uint8_t xGetCurrentHourIdx(void)
{
    return currentHrIdx;
}


/*  Check to make sure we are hitting the algorithm timing requirements. If not, we
    will take action.

    Requirements:
        -Average sample period shall be 50ms +/- 5ms
        -Dropped samples shall not be larger than 6 in a 3 second time span. Dropped = its been 100ms
         since last running the algo

    Actions (Always log to the terminal):
        -Level 0: Log to cloud through error bits
        -Level 1: Re-initialize algorithm
        -Level 2: Reset device
*/
static void xRunAlgoDiagnostics(uint64_t timeLastRan, uint64_t currentTime)
{
    static uint8_t algLastRanGreaterThan50MsCounts = 0;
    static uint8_t missedSamplesCount = 0;
    static uint64_t lastWindowValue = 0;
    static uint8_t numAvgSamplePeriodDriftsThisDay = 0;
    static uint8_t numMissedSamplesThisDay = 0;
    static bool samplePeriodDriftLevel2 = false;
    static bool missedSampleThreshLevel2 = false;
    static bool firstDiagnosticCalc = true;

    //do not run on the first call after power up to allow the algorithm nest to run once
    if ( firstDiagnosticCalc == true )
    {
        firstDiagnosticCalc = false;
        return;
    }

    //1. Average sample period:
    if ( currentTime - timeLastRan > UC_TIME_TICKS_PER_50MS )
    {
        algLastRanGreaterThan50MsCounts++;

        //2. Dropped a sample:
        if ( currentTime - timeLastRan > UC_TIME_TICKS_PER_100MS )
        {
            missedSamplesCount += ( (currentTime - timeLastRan)/UC_TIME_TICKS_PER_50MS ) - 1;
        }
    }
    else
    {
        if ( algLastRanGreaterThan50MsCounts > 0 )
        {
           algLastRanGreaterThan50MsCounts--;
        }
    }

    //if we have exceeded the threshold, that means our avg sampling period has drifted too much
    if ( algLastRanGreaterThan50MsCounts >= LATE_ALGO_RUNS_THRESHOLD )
    {
        //take an action based on how many times this has happened today
        HW_TERM_PrintColor("\r\n Exceeded 20 late algorithm runs \r\n", KRED);

        if ( (sensorData.errorBits & AVG_SAMPLE_PERIOD_DRIFT) == 0 )
        {
            numAvgSamplePeriodDriftsThisDay = 0;
            samplePeriodDriftLevel2 = false;

            //this will be cleared at midnight of the current day
            sensorData.errorBits |= AVG_SAMPLE_PERIOD_DRIFT;

            numAvgSamplePeriodDriftsThisDay++;
        }
        else
        {
            //the bit was already set, take the next level action
            if ( samplePeriodDriftLevel2 == false )
            {
                APP_ALGO_Init();
                samplePeriodDriftLevel2 = true;
            }
            else
            {
                //third level - set a system error to generate a reset
                APP_indicateError( ((uint32_t)AVG_SAMPLE_PERIOD_DRIFT << ALGO_ERROR_OFFSET) );
            }
        }

        algLastRanGreaterThan50MsCounts = 0;
    }

    //every 3 seconds check the missed sample counter
    if ( currentTime - lastWindowValue >= ( MISSED_ALGO_RUN_WINDOW_SECS * UC_TIME_TICKS_PER_S ) )
    {
        if ( missedSamplesCount >= MISSED_ALGO_RUNS_THRESHOLD )
        {
            HW_TERM_PrintColor("\r\n Exceeded 6 missed samples in 3 seconds \r\n", KRED);

            if ( (sensorData.errorBits & MISSED_SAMPLE_THRESH) == 0 )
            {
                numMissedSamplesThisDay = 0;
                missedSampleThreshLevel2 = false;

                //this will be cleared at midnight of the current day
                sensorData.errorBits |= MISSED_SAMPLE_THRESH;
                numMissedSamplesThisDay++;
            }
            else
            {
                if ( missedSampleThreshLevel2 == false )
                {
                    //bit was already set, so take the next level action
                    APP_ALGO_Init();
                    missedSampleThreshLevel2 = true;
                }
                else
                {
                    //third level - set a system error to generate a reset
                    APP_indicateError( ( (uint32_t)MISSED_SAMPLE_THRESH << ALGO_ERROR_OFFSET) );
                }
            }
        }

        //reset counters
        lastWindowValue = currentTime;
        missedSamplesCount = 0;
    }
}

static void xAddTimestampToDataLog(void)
{
    xFirstHourOfSensorData = false;
    sensorData.timestamp = HW_RTC_GetEpochTime();
}

static void xResetSensorData(void)
{
    memset(&sensorData.litersPerHour, 0, sizeof(sensorData.litersPerHour));
    memset(&sensorData.tempPerHour, 0, sizeof(sensorData.tempPerHour));
    memset(&sensorData.humidityPerHour, 0, sizeof(sensorData.humidityPerHour));
    memset(&sensorData.strokesPerHour, 0, sizeof(sensorData.strokesPerHour));
    memset(&sensorData.strokeHeightPerHour, 0, sizeof(sensorData.strokeHeightPerHour));
    sensorData.dailyLiters = 0;
    sensorData.totalLiters = 0;
    sensorData.avgLiters = 0;
    sensorData.errorBits = 0;
}
