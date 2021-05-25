/**************************************************************************************************
* \file     ssm.c
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


#include "stddef.h"
#include "string.h"
#include "stm32l4xx_hal.h"
#include "logTypes.h"
#include "CLI.h"
#include "uart.h"
#include "spi.h"
#include "ssm.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "am-ssm-spi-protocol.h"
#include "task.h"
#include "eventManager.h"
#include "memMapHandler.h"
#include "ntpHandler.h"
#include "appVersion.h"
#include "mspBslProtocol.h"
#include <stdlib.h>

#define SSM_TASK_POLLING_RATE_MS                 100
#define MIN_RED_FLAG_ON_THRESH                   0
#define MAX_RED_FLAG_ON_THRESH                   100
#define MIN_RED_FLAG_OFF_THRESH                  0
#define MAX_RED_FLAG_OFF_THRESH                  100

#define MAX_RETRIES                              3

//SSM event types
typedef enum
{
    SEND_ACTIVATE,
    SEND_DEACTIVATE,
    SEND_RESET,
    REQUEST_STATUS,
    REQUEST_SENSOR_DATA_NUM_ENTRIES,
    REQUEST_SENSOR_DATA_ENTRY,
    INDICATE_SENSOR_DATA_ENTRY_STORED,
    SET_TIME,
    RESET_ALARMS,
    SEND_CONFIGS,
}ssmCmds_t;

static asp_attn_source_payload_t ssmAttentionSrcList;
static aspMessageCode_t ssmOperationSuccess = BAD_REQUEST;
static QueueHandle_t ssmMsgQueue;

static ssmCmds_t xCurrentCmd;
static asp_status_payload_t xCurrentStatus;
static asp_sensor_data_entry_t xCurrentEntry;
static uint16_t entriesLeftToRequest = 0u;

//if this is set to true, we will not try to send/request spi messages
//or handle the attention line
static bool updatingFw = false;
static uint16_t wakeRateInDays = 0;
static bool strokeAlgIsOn = false;
static uint16_t redFlagOnThresh = 0u;
static uint16_t redFlagOffThresh = 0u;

static void commandHandlerForSsm(int argc, char **argv);
static void SSM_initHw(void);
static void SSM_getandHandleAttnList(void);
static bool checkRetryNeeded(aspMessageCode_t code);
static bool checkMsgFailedAndHandle(aspMessageCode_t code);
static void getNextSensorDataEntry(uint16_t entriesToGet);
static void handleSensorDataEntry(void);

void SSM_Init(void)
{
    SSM_initHw();

    //init queue for app to issue commands to SSM
    //This allows the app to talk to the SSM without blocking on responses
    ssmMsgQueue = xQueueCreate( 3, sizeof( ssmCmds_t ) );

    if ( ssmMsgQueue == NULL )
    {
        elogError("FAILED to create ssm queue");
    }
    else
    {
        elogInfo("SSM module & queue initialized");
    }
}

void SSM_SPI_Task( void * p_parameters )
{
    aspMessageCode_t ssmOperationResult = BAD_REQUEST;
    static uint8_t tries = 0u;
    uint32_t timestamp = 0;
    static asp_number_data_entries_payload_t entriesInPayload;

    while(1)
    {
        //handle attn source line and any spi message requests if we are not doing a fw update
        if ( !updatingFw )
        {
            if( xQueueReceive( ssmMsgQueue, &( xCurrentCmd ), ( TickType_t )  SSM_TASK_POLLING_RATE_MS) == pdPASS )
            {
                switch(xCurrentCmd)
                {
                    case SEND_ACTIVATE:

                        //send the activate message to the ssm and get ack
                        tries = 0;
                        do
                        {
                           ssmOperationResult = ASP_SendActivate();
                           tries++;
                        }while ( checkRetryNeeded(ssmOperationResult) == true && tries < MAX_RETRIES);

                        //check final result code for unresponsiveness
                        checkMsgFailedAndHandle(ssmOperationResult);

                        break;
                    case SEND_DEACTIVATE:

                        //send the activate message to the ssm and get ack
                        tries = 0;
                        do
                        {
                           ssmOperationResult = ASP_SendDeActivate();
                           tries++;
                        }while ( checkRetryNeeded(ssmOperationResult) == true && tries < MAX_RETRIES);

                        //check final result code for unresponsiveness
                        checkMsgFailedAndHandle(ssmOperationResult);

                        break;

                    case REQUEST_STATUS:

                        //get status, set ssm status once we receive it
                        tries = 0;
                        do
                        {
                           ssmOperationResult = ASP_GetSSMStatus(&xCurrentStatus);
                           tries++;
                        }while ( checkRetryNeeded(ssmOperationResult) == true && tries < MAX_RETRIES);

                        //check final result code for unresponsiveness
                        checkMsgFailedAndHandle(ssmOperationResult);

                        break;

                    case REQUEST_SENSOR_DATA_NUM_ENTRIES:

                        entriesInPayload.numEntries = 0;
                        tries = 0;
                        do
                        {
                            // Get the number of entries available
                            ssmOperationResult = ASP_GetSensorDataNumEntries(&entriesInPayload);
                            tries++;
                        }while ( checkRetryNeeded(ssmOperationResult) == true && tries < MAX_RETRIES);

                        if (ssmOperationResult == SUCCESSFUL_REQUEST)
                        {
                            entriesLeftToRequest = entriesInPayload.numEntries;

                            SSM_requestNewDataLogEntryFromSsm();
                        }

                        //check final result code for unresponsiveness
                        checkMsgFailedAndHandle(ssmOperationResult);

                        break;

                    case REQUEST_SENSOR_DATA_ENTRY:

                        if ( entriesInPayload.numEntries != 0)
                        {
                            entriesLeftToRequest--;

                            tries = 0;
                            do
                            {
                                //get data log from ssm
                                ssmOperationResult = ASP_GetSensorData( entriesInPayload.numEntries, &xCurrentEntry );
                                tries++;
                            }while ( checkRetryNeeded(ssmOperationResult) == true && tries < MAX_RETRIES);


                            if (ssmOperationResult == SUCCESSFUL_REQUEST)
                            {
                                handleSensorDataEntry();
                            }

                            //check final result code for unresponsiveness
                            checkMsgFailedAndHandle(ssmOperationResult);
                        }

                        break;

                    case INDICATE_SENSOR_DATA_ENTRY_STORED:

                        tries = 0;
                        do
                        {
                            //let the SSM know that the last requested sensor data log has been stored to flash successfully
                            ssmOperationResult = ASP_SensorDataStoredToFlash();
                            tries++;
                        }while ( checkRetryNeeded(ssmOperationResult) == true && tries < MAX_RETRIES);

                        if (ssmOperationResult == SUCCESSFUL_REQUEST)
                        {
                            getNextSensorDataEntry(entriesLeftToRequest);
                        }

                        //check final result code for unresponsiveness
                        checkMsgFailedAndHandle(ssmOperationResult);

                        break;

                    case SET_TIME:
                        //get epoch time, set SSM time
                        timestamp = NTP_getTime();
                        tries = 0;
                        do
                        {
                            ssmOperationResult = ASP_SetTime( timestamp );
                            tries++;
                        }while ( checkRetryNeeded(ssmOperationResult) == true && tries < MAX_RETRIES);

                        //check final result code for unresponsiveness
                        checkMsgFailedAndHandle(ssmOperationResult);

                        break;

                    case SEND_CONFIGS:

                        //Send config message with the latest that is stored in FLASH:
                        wakeRateInDays = MEM_getAmWakeRate();

                        tries = 0;
                        do
                        {
                           ssmOperationResult = ASP_SendConfigs(wakeRateInDays, strokeAlgIsOn, redFlagOnThresh, redFlagOffThresh);
                           tries++;
                        }while ( checkRetryNeeded(ssmOperationResult) == true && tries < MAX_RETRIES);

                        checkMsgFailedAndHandle(ssmOperationResult);

                        break;

                    case RESET_ALARMS:

                        tries = 0;
                        do
                        {
                           ssmOperationResult = ASP_SendResetAlarmsCmd();
                           tries++;
                        }while ( checkRetryNeeded(ssmOperationResult) == true && tries < MAX_RETRIES);

                        //check final result code for unresponsiveness
                        checkMsgFailedAndHandle(ssmOperationResult);

                        break;

                    default:

                        break;
                }
            }

            //Poll the ATTN source line (wakeup line) to see if we need to check in, activate, etc
            if ( HAL_GPIO_ReadPin(AM_AWAKE_PORT, AM_AWAKE_PIN) == GPIO_PIN_SET )
            {
                SSM_getandHandleAttnList();
            }
        }
    }
}

bool SSM_communicationCheck(void)
{
    //get status, set ssm status once we receive it
    uint8_t tries = 0;
    aspMessageCode_t result;
    bool commPassed = false;

    do
    {
        result = ASP_GetSSMStatus(&xCurrentStatus);
        tries++;
    }while ( checkRetryNeeded(result) == true && tries < MAX_RETRIES);

    if ( result != SUCCESSFUL_REQUEST)
    {
        commPassed = false;
    }
    else
    {
        commPassed = true;
    }

    return commPassed;
}

bool SSM_sendHwResetCmd(void)
{
    uint8_t tries = 0;
    aspMessageCode_t resultCode;
    bool ackedResetCmd = false;

    tries = 0;
    do
    {
        resultCode = ASP_SendHwReset();
        tries++;
    }while ( checkRetryNeeded(resultCode) == true && tries < MAX_RETRIES);

    if ( resultCode != SUCCESSFUL_REQUEST)
    {
        ackedResetCmd = false;
    }
    else
    {
        ackedResetCmd = true;
    }

    return ackedResetCmd;
}

asp_status_payload_t SSM_getStatus(void)
{
    return xCurrentStatus;
}

asp_sensor_data_entry_t SSM_getSensorData(void)
{
    return xCurrentEntry;
}

static bool checkRetryNeeded(aspMessageCode_t code)
{
   bool retryNeeded = false;

   if ( code != SUCCESSFUL_REQUEST)
   {
       if (code == NACKED_MSG)
       {
           EVT_indicateSsmNackedRequest();

       }

       retryNeeded = true;
   }

   return retryNeeded;
}

static bool checkMsgFailedAndHandle(aspMessageCode_t code)
{
    bool failed = false;

    if ( code != SUCCESSFUL_REQUEST)
    {
        if (code == NACKED_MSG)
        {
            EVT_indicateSsmNackedRequest();
        }
        else
        {
            failed = true;
            EVT_indicateSsmUnresponsive();
        }
    }

    return failed;
}

static void getNextSensorDataEntry(uint16_t entriesToGet)
{
    if(entriesToGet > 0)
    {
        // Request the next sensor data log from the SSM
        SSM_requestNewDataLogEntryFromSsm();
    }
    else
    {
        // All new sensor data entries received from SSM and stored to flash
        EVT_indicateSensorDataReady();
    }
}

static void handleSensorDataEntry(void)
{
    EVT_indicateSensorDataMsgReceivedFromSSM();
}

static void SSM_getandHandleAttnList(void)
{
    aspMessageCode_t ssmOperationResult = BAD_REQUEST;
    uint8_t tries = 0;

    //request an attn source list
    do
    {
       ssmOperationResult = ASP_GetAttnSrcList(&ssmAttentionSrcList);
       tries++;
    }while ( checkRetryNeeded(ssmOperationResult) == true && tries < MAX_RETRIES);

    //look at the final result code
    if ( ssmOperationResult == SUCCESSFUL_REQUEST )
    {
        //handle and send ACKS as needed
        if(ssmAttentionSrcList.attnSourceList > 0 )
        {
            //Activate:
            if(ssmAttentionSrcList.attnSourceList & ASP_ACTIVATE)
            {
                elogInfo("Activate set");

                //pass the info up to the event task to handle
                EVT_indicateActivateFromSsm();

                //clear the bit
                ssmAttentionSrcList.attnSourceList &= (~ASP_ACTIVATE);
            }

            // Time sync requested
            if(ssmAttentionSrcList.attnSourceList & ASP_REQUEST_TIME)
            {
                elogInfo("Time sync requested");

                EVT_initiateNtpTimeSync();
                ssmAttentionSrcList.attnSourceList &= (~ASP_REQUEST_TIME);
            }

            //Check in state deactivated:
            if(ssmAttentionSrcList.attnSourceList & ASP_CHECK_IN_DEACTIVATED)
            {
                //check in
                elogInfo("Check in deactivated set");

                EVT_indicateCheckInDeactivated();
                ssmAttentionSrcList.attnSourceList &= (~ASP_CHECK_IN_DEACTIVATED);
            }

            //Check in state activated
            if(ssmAttentionSrcList.attnSourceList & ASP_CHECK_IN_ACTIVATED)
            {
                //check in
                elogInfo("Check in activated set");

                EVT_indicateCheckInActivated();
                ssmAttentionSrcList.attnSourceList &= (~ASP_CHECK_IN_ACTIVATED);
            }

            //now send an ATTN src ACK
            tries = 0;

            do
            {
               ssmOperationResult = ASP_SendAttnSrcAckMsg(&ssmAttentionSrcList);
               tries++;
            }while ( checkRetryNeeded(ssmOperationResult) == true && tries < MAX_RETRIES);

            //look at final code and make sure that the ssm responded properly
            if ( ssmOperationResult != SUCCESSFUL_REQUEST )
            {
                elogError("Attn list ACK failed - error code %d", ssmOperationResult );
                EVT_indicateSsmUnresponsive();
            }
        }
    }
    else
    {
        elogError("Get attn list failed - error code %d", ssmOperationResult );
        EVT_indicateSsmUnresponsive();
    }
}

void SSM_changeStateToActivated(void)
{
    //send an activate message to the SSM
    ssmCmds_t msg = SEND_ACTIVATE;
    xQueueSend(ssmMsgQueue, &msg, ( TickType_t ) 10 );
}

void SSM_sendConfigs(void)
{
    //send new configs to SSM
    ssmCmds_t msg = SEND_CONFIGS;
    xQueueSend(ssmMsgQueue, &msg, ( TickType_t ) 10 );
}

void SSM_deactivateionReceivedFromCloud(void)
{
    //send a deactivate  message to the SSM
    ssmCmds_t msg = SEND_DEACTIVATE;
    xQueueSend(ssmMsgQueue, &msg, ( TickType_t ) 10 );
}

void SSM_sendResetAlarms(void)
{
    //send new configs to SSM
    ssmCmds_t msg = RESET_ALARMS;
    xQueueSend(ssmMsgQueue, &msg, ( TickType_t ) 10 );
}

void SSM_requestStatusFromSsm(void)
{
    ssmCmds_t msg = REQUEST_STATUS;
    xQueueSend(ssmMsgQueue, &msg, ( TickType_t ) 10 );
}

void SSM_requestNumDataLogEntriesFromSsm(void)
{
    ssmCmds_t msg = REQUEST_SENSOR_DATA_NUM_ENTRIES;
    xQueueSend(ssmMsgQueue, &msg, ( TickType_t ) 10 );
}

void SSM_requestNewDataLogEntryFromSsm(void)
{
    ssmCmds_t msg = REQUEST_SENSOR_DATA_ENTRY;
    xQueueSend(ssmMsgQueue, &msg, ( TickType_t ) 10 );
}

void SSM_sensorDataStoredToFlash(void)
{
    ssmCmds_t msg = INDICATE_SENSOR_DATA_ENTRY_STORED;
    xQueueSend(ssmMsgQueue, &msg, ( TickType_t ) 10 );
}

void SSM_performTimeSync(void)
{
    ssmCmds_t msg = SET_TIME;
    xQueueSend(ssmMsgQueue, &msg, ( TickType_t ) 10 );
}

void SSM_setAlgoConfig(bool onOff)
{
    strokeAlgIsOn = onOff;
}

bool SSM_setRedFlagThresholdConfigs(uint16_t flagOn, uint16_t flagOff)
{
    bool configCheckPassed = false;

    //check thresholds before saving them or sending them:
    if ( flagOff > flagOn &&
         flagOff <= MAX_RED_FLAG_OFF_THRESH &&
         flagOn <= MAX_RED_FLAG_ON_THRESH )
    {
        redFlagOnThresh = flagOn;
        redFlagOffThresh = flagOff;

        //red flag thresholds are valid
        configCheckPassed = true;
    }

    return configCheckPassed;
}

bool SSM_getTimeSyncRequested(void)
{
    if(ssmAttentionSrcList.attnSourceList & ASP_REQUEST_TIME)
    {
        elogInfo("Time sync requested");

        return true;
    }
    return false;
}

/* Initialize the mux control and boot pin */
static void SSM_initHw(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* init mux control gpio as LOW - this prevents the
     * AM from receiving/ sending unintentional uart data - PD8 */
    GPIO_InitStruct.Pin = AM_SSM_UART_MUX_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(AM_SSM_UART_MUX_PORT, &GPIO_InitStruct);

    /* init ssm boot pin - PE7 */
    GPIO_InitStruct.Pin = SSM_BOOT_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(SSM_BOOT_PORT, &GPIO_InitStruct);

    /* Init the attn line as GPIO in this module */
    GPIO_InitStruct.Pin = AM_AWAKE_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(AM_AWAKE_PORT, &GPIO_InitStruct);

    //bootloader entry pins:
    GPIO_InitStruct.Pin = SSM_RST_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(SSM_RST_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = SSM_TEST_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(SSM_TEST_PORT, &GPIO_InitStruct);

    /* add a command line handler */
    CLI_Command_Handler_s cmdHandler;
    cmdHandler.ptrFunction = &commandHandlerForSsm;
    cmdHandler.cmdString   = "ssm";
    cmdHandler.usageString = "\n\r\tmux [on off] \n\r\tboot [on off] \n\r\tuart \n\r\tstatus \n\r\tr \n\r\treset\n\r\tsensorDataEntries\n\r\tgetSensorData [num entries] \n\r\ttime [epoch (hex)]";
    CLI_registerThisCommandHandler(&cmdHandler);
}

void SSM_enableUart(void)
{
    HAL_GPIO_WritePin(AM_SSM_UART_MUX_PORT, AM_SSM_UART_MUX_PIN, GPIO_PIN_SET);
}

void SSM_disableUart(void)
{
    HAL_GPIO_WritePin(AM_SSM_UART_MUX_PORT, AM_SSM_UART_MUX_PIN, GPIO_PIN_RESET);
}

void SSM_enableBootPin(void)
{
    HAL_GPIO_WritePin(SSM_BOOT_PORT, SSM_BOOT_PIN, GPIO_PIN_SET);
}

void SSM_disableBootPin(void)
{
    HAL_GPIO_WritePin(SSM_BOOT_PORT, SSM_BOOT_PIN, GPIO_PIN_RESET);
}

/*
  To put into bootload mode:
    1. Reset line LOW
    2. TEST line is LOW -> high for > 250ns
    3. Test line back LOW for > 250ns
    4. Test line high for > 250ns
    5. while the test line is high, Set the RST line high
    6. bring the Test line back low

   Should be in BL mode at this point
 */
void SSM_putIntoBootloadModeThroughResetPin(void)
{
    //set this flag to true so that we dont handle the attn src line
    updatingFw = true;
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    //init the pins

    /* init ssm boot pin - PE7 */
    GPIO_InitStruct.Pin = SSM_BOOT_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(SSM_BOOT_PORT, &GPIO_InitStruct);


    //    //bootloader entry pins:
    GPIO_InitStruct.Pin = SSM_RST_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(SSM_RST_PORT, &GPIO_InitStruct);

   //PE2 --> RESET/SBWTDIO
   //PE4 --> TEST/SBWTCK
    HAL_GPIO_WritePin(SSM_RST_PORT, SSM_RST_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(SSM_TEST_PORT, SSM_TEST_PIN, GPIO_PIN_SET);
    vTaskDelay(200);

    HAL_GPIO_WritePin(SSM_TEST_PORT, SSM_TEST_PIN, GPIO_PIN_RESET);
    vTaskDelay(2);

    //first TEST line toggle
    HAL_GPIO_WritePin(SSM_RST_PORT, SSM_RST_PIN, GPIO_PIN_SET);
    vTaskDelay(2);

    HAL_GPIO_WritePin(SSM_TEST_PORT, SSM_TEST_PIN, GPIO_PIN_SET);
    vTaskDelay(2);

    HAL_GPIO_WritePin(SSM_TEST_PORT, SSM_TEST_PIN, GPIO_PIN_RESET);
    vTaskDelay(2);

    //second toggle
    HAL_GPIO_WritePin(SSM_TEST_PORT, SSM_TEST_PIN, GPIO_PIN_SET);
    vTaskDelay(2);

    //release reset line
    HAL_GPIO_WritePin(SSM_RST_PORT, SSM_RST_PIN, GPIO_PIN_RESET);
    vTaskDelay(2);

    //complete the second toggle
    HAL_GPIO_WritePin(SSM_TEST_PORT, SSM_TEST_PIN, GPIO_PIN_RESET);
}

//Do NOT call this function unless the SSM is completely locked up
//or if we just finished programming the SSM, and now want to run the app
void SSM_hardwareReset(void)
{
    //PE2 --> RESET/SBWTDIO
    //PE4 --> TEST/SBWTCK
    HAL_GPIO_WritePin(SSM_TEST_PORT, SSM_TEST_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(SSM_RST_PORT, SSM_RST_PIN, GPIO_PIN_RESET);

    vTaskDelay(200);

    //bring the reset line low for a short amount of time
    HAL_GPIO_WritePin(SSM_RST_PORT, SSM_RST_PIN, GPIO_PIN_SET);
    vTaskDelay(2);

    //Release back high
    HAL_GPIO_WritePin(SSM_RST_PORT, SSM_RST_PIN, GPIO_PIN_RESET);

    //reset flag
    updatingFw = false;
}

static void commandHandlerForSsm(int argc, char **argv)
{
    if ( argc == TWO_ARGUMENTS && 0 == strcmp(argv[FIRST_ARG_IDX], "mux") )
    {
        if ( 0 == strcmp(argv[SECOND_ARG_IDX], "on") )
        {
            SSM_enableUart();
            elogInfo("Turning ON mux");

        }
        else if ( 0 == strcmp(argv[SECOND_ARG_IDX], "off") )
        {
            elogInfo("Turning OFF mux");
            SSM_disableUart();
        }
        else
        {
            elogError("Invalid arg");
        }
    }
    else if ( argc == TWO_ARGUMENTS && 0 == strcmp(argv[FIRST_ARG_IDX], "boot") )
    {
        if ( 0 == strcmp(argv[SECOND_ARG_IDX], "on") )
        {
            elogInfo("Turning ON boot pin");
            SSM_enableBootPin();
        }
        else if ( 0 == strcmp(argv[SECOND_ARG_IDX], "off") )
        {
            elogInfo("Turning OFF boot pin");
            SSM_disableBootPin();
        }
        else
        {
            elogError("Invalid arg");
        }
    }
    else if ( argc == ONE_ARGUMENT && 0 == strcmp(argv[FIRST_ARG_IDX], "uart") )
    {
        //Tests RESET line, TEST line, and UART mux control:
        bool successful = false;

        //put the msp430 into bootloading mode:
        SSM_putIntoBootloadModeThroughResetPin();
        vTaskDelay(100);

        //enable to the mux that allows the STM to communicate with the MSP
        SSM_enableUart();
        vTaskDelay(500);

        //now write the password - we will intentionally fail this so that the MSP flash is erased
        successful = BSL_writePasswordDefault();

        if (!successful)
        {
           elogInfo("Write password failed ( we expect this to happen ONCE) MSP flash erased\r\n");
           vTaskDelay(200);

           successful = BSL_writePasswordDefault();
           successful = BSL_writePasswordDefault();
        }

        //should now be true, if not we need to bail
        if ( !successful )
        {
           elogError("FAILED to accept the default password");

           //disable the mux
           SSM_disableUart();

           vTaskDelay(100);
        }
        else
        {
           elogInfo("Write Password was successful\r\n");
           vTaskDelay(2000);
        }
    }
    else if ( argc == ONE_ARGUMENT && 0 == strcmp(argv[FIRST_ARG_IDX], "status") )
    {
        asp_status_payload_t p_status;

        ssmOperationSuccess = ASP_GetSSMStatus(&p_status);

        if ( ssmOperationSuccess == SUCCESSFUL_REQUEST )
        {
            elogInfo("SSM Status:\n\rReset State: %u\n\rFW: %u.%u.%u\n\rTime: 0x%X\n\rErrors: 0x%X\n\rActivated State: %u\n\r", p_status.resetState,
                                            p_status.ssmFwVersion.fwMaj,
                                            p_status.ssmFwVersion.fwMin,
                                            p_status.ssmFwVersion.fwBuild,
                                            p_status.timestamp,
                                            p_status.errorBits,
                                            p_status.activatedState);


            if ( p_status.ssmFwVersion.fwMaj == VERSION_MAJOR && p_status.ssmFwVersion.fwMin == VERSION_MINOR
                    && p_status.ssmFwVersion.fwBuild == VERSION_BUILD )
            {
                elogInfo("SSM-AM SPI test pass");
            }
            else
            {
                elogError("SSM-AM SPI test fail");
            }
        }
        else
        {
            elogError("SSM get status failed, error code %d ", ssmOperationSuccess);
        }

    }
    else if ( argc == ONE_ARGUMENT && 0 == strcmp(argv[FIRST_ARG_IDX], "reset") )
    {
        elogInfo("Resetting SSM.\n");

    	ssmOperationSuccess = ASP_CommandReset();
        if ( ssmOperationSuccess != SUCCESSFUL_REQUEST )
        {
            elogError("SSM reset failed, error code %d ", ssmOperationSuccess);
        }
    }
    else if ( argc == ONE_ARGUMENT && 0 == strcmp(argv[FIRST_ARG_IDX], "sensorDataEntries") )
    {
        asp_number_data_entries_payload_t logEntries;
        elogInfo("Getting number of sensor data logs.\n");
        ASP_GetSensorDataNumEntries(&logEntries);
        elogInfo("Number of daily sensor data logs %d ", logEntries.numEntries);
    }
    else if ( argc == TWO_ARGUMENTS && 0 == strcmp(argv[FIRST_ARG_IDX], "getSensorData") )
    {
        uint16_t num_records = strtoul(argv[SECOND_ARG_IDX], NULL, 10);
        aspMessageCode_t ssmOperationResult = BAD_REQUEST;
        asp_sensor_data_entry_t entry;
        elogInfo("Get %d of sensor data log entries \n", num_records);

        static asp_sensor_data_entry_t expectedEntry =
        {
            .timestamp = 0x5E000000,
            .litersPerHour = {0,10,0,0,1254,0,0,0,0,0,0,0,33,0,0,0,0,0,0,444,0,0,0,0},
            .tempPerHour = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
            .humidityPerHour = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
            .strokesPerHour = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
            .strokeHeightPerHour = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
            .dailyLiters = 0,
            .avgLiters = 0,
            .totalLiters = 50,
            .breakdown = false,
            .pumpCapacity = 0,
            .batteryVoltage = 3600,
            .powerRemaining = 100,
            .state = FAULT,
            .magnetDetected = true,
            .errorBits = 0,
            .unexpectedResets = 0,
            .timestampOfLastReset = 0,
            .activatedDate = 1597347495
        };

        uint8_t tries = 0;
        do
        {
           ssmOperationResult = ASP_GetSensorData( num_records, &entry );
           tries++;
        }while ( checkRetryNeeded(ssmOperationResult) == true && tries < MAX_RETRIES);

        if ( ssmOperationResult != SUCCESSFUL_REQUEST )
        {
            elogError("SSM get log failed, error code %d ", ssmOperationSuccess);
        }
        else if ( (expectedEntry.timestamp == entry.timestamp)              &&
                  (expectedEntry.dailyLiters == entry.dailyLiters)          &&
                  (expectedEntry.totalLiters == entry.totalLiters)          &&
                  (expectedEntry.dailyLiters == entry.dailyLiters)          &&
                  (expectedEntry.avgLiters == entry.avgLiters)              &&
                  (expectedEntry.pumpCapacity == entry.pumpCapacity)        &&
                  (expectedEntry.batteryVoltage == entry.batteryVoltage)    &&
                  (expectedEntry.state == entry.state)                      &&
                  (expectedEntry.powerRemaining == entry.powerRemaining)    &&
                  (expectedEntry.unexpectedResets == entry.unexpectedResets)&&
                  (expectedEntry.activatedDate == entry.activatedDate))
        {
            elogInfo("Sensor data message verified");
        }
        else
        {
            elogError("Sensor data message does not match");
            elogInfo("contents %lu, %d, %lu, %d, %d, %d, %d, %d, %d, %lu", entry.timestamp, entry.dailyLiters,
                    entry.totalLiters, entry.avgLiters, entry.pumpCapacity, entry.batteryVoltage, entry.state,
                    entry.powerRemaining, entry.unexpectedResets, entry.activatedDate );
        }
        SSM_sensorDataStoredToFlash(); // This lets the ssm know to increment its tail even though we aren't actually storing to flash here
    }

    else if ( argc == ONE_ARGUMENT && 0 == strcmp(argv[FIRST_ARG_IDX], "r"))
    {
        /* read the awake pin state */
        uint8_t ubPinState = HAL_GPIO_ReadPin(AM_AWAKE_PORT, AM_AWAKE_PIN);
        elogInfo("Pin state = %d", ubPinState );
    }

    else if ( argc == TWO_ARGUMENTS && 0 == strcmp(argv[FIRST_ARG_IDX], "time") )
    {
        uint32_t time = strtoul(argv[SECOND_ARG_IDX], NULL, 16);
        elogInfo("Setting time to 0x%lX.\n", time);
    	ssmOperationSuccess = ASP_SetTime(time);

    	if ( ssmOperationSuccess != SUCCESSFUL_REQUEST )
    	{
    	    elogError("SSM set time failed, error code %d ", ssmOperationSuccess);
    	}
    }
    else
    {
        elogError("invalid cli command");
    }
}
