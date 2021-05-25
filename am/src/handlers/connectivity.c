/**************************************************************************************************
* \file     connectivity.c
* \brief    Manage the cellular/gps connection, including antenna setup and power to the modules. Includes
*           a task that can be used to poll RSSI for debugging
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

#include "stm32l4xx_hal.h"
#include "string.h"
#include "stdlib.h"
#include "sara_u201.h"
#include "pwrMgr.h"
#include "PE42424A_RF.h"
#include "logTypes.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "taskMonitor.h"
#include "nwStackFunctionality.h"
#include "CLI.h"
#include "memMapHandler.h"
#include "uart.h"
#include "gpsManager.h"
#include "connectivity.h"

#define DELAY_TASK_MS                5*1000
#define SECONDS_PER_MONTH            60*60*24*28 //use 28 days per month...
#define MONTHS_TO_WAIT_FOR_SWITCH    3
#define BAD_RSSI_VAL                 99

#define CONN_TASK_PRIORITY           ( configMAX_PRIORITIES - 1 )
#define CONN_TASK_STACK_SIZE         ( configMINIMAL_STACK_SIZE * 18 )

typedef enum
{
    WAITING_FOR_RDY,
    WAITING_FOR_DEVICE_ID,
    WAITING_FOR_LOC,
    RDY,
    IDLE,
    ERR,
}connectivityStates_t;

TaskHandle_t xConnHandle;
static bool pollStat = false;
static antenna_t antennaConfig = UNKNOWN_ANTENNA;
static uint32_t epochTime = 0;

static void modemPowerAndCellRfConfig(antenna_t config);
static antenna_t determineWhichAntennaToUse(uint32_t currentTimestamp);
static void connectionStatusPolling(void);
static void commandHandlerForConn(int argc, char **argv);

void CONN_init(uint32_t timestamp)
{
    //store latest time
    epochTime = timestamp;

    //start task
    xTaskCreate(CONN_task, "CONN", CONN_TASK_STACK_SIZE, NULL, CONN_TASK_PRIORITY, &xConnHandle );
}

void CONN_initCliCommands(void)
{
    //register a command line handler
    CLI_Command_Handler_s cmdHandler;
    cmdHandler.ptrFunction = &commandHandlerForConn;
    cmdHandler.cmdString   = "conn";
    cmdHandler.usageString = "\n\r\ton <antenna> \n\r\tpoll \n\r\toff \n\r\ticcid \n\r\timei \n\r\tversion \n\r\ttx[channel][output power dBm]";
    CLI_registerThisCommandHandler(&cmdHandler);
}

void CONN_task(void)
{
    //check with antenna we should use this time
    antennaConfig = determineWhichAntennaToUse(epochTime);

    /* Power up the cell module. Note, this power up routine takes ~4 seconds */
    modemPowerAndCellRfConfig(antennaConfig);
    NW_initLwip();

    while (1)
    {
        vTaskDelay(DELAY_TASK_MS);

        if (pollStat == true)
        {
            connectionStatusPolling();
        }

        //check in
        TM_connTaskCheckIn();
    }
}

void CONN_updateRssiForAntennaUsed(uint8_t rssi)
{
    //do not update rssi if GPS is running - this means that
    //antenna switching logic was overridden
    if ( GPS_isGpsEnabled() == false )
    {
        if ( antennaConfig == PRIMARY )
        {
            MEM_setPrimaryAntennaRssi(rssi);
        }
        else
        {
            MEM_setSecondaryAntennaRssi(rssi);
        }
    }
}

static void connectionStatusPolling(void)
{
    SARA_getRssi();
}

static antenna_t determineWhichAntennaToUse(uint32_t currentTimestamp)
{
    uint8_t primaryRssi = 0;
    uint8_t secondaryRssi = 0;
    antenna_t antennaToUseThisConnection = PRIMARY;
    antenna_t savedAntenna = UNKNOWN_ANTENNA;
    uint32_t timestampLastAntennaSwitch = 0;

    //read out the antenna config to set up:
    savedAntenna = MEM_getAntennaToUse();

    //get the rssi values
    primaryRssi = MEM_getPrimaryAntennaRssi();
    secondaryRssi = MEM_getSecondaryAntennaRssi();
    timestampLastAntennaSwitch = MEM_getEpochTimeLastAntennaSwitch();

    if ( primaryRssi == 0xFF )
    {
        //use the primary antenna this time
        antennaToUseThisConnection = PRIMARY;

    }
    else if ( secondaryRssi == 0xFF )
    {
        //use the secondary antenna this time
        antennaToUseThisConnection = SECONDARY;
    }
    else
    {
        //if we have never selected the best antenna, or we re-took rssi readings the
        //last connection:
        if ( savedAntenna == UNKNOWN_ANTENNA )
        {
            //pick based on the RSSI's
            if ( primaryRssi == secondaryRssi )
            {
                //they are equal, use the primary one
                antennaToUseThisConnection = PRIMARY;
            }
            else if ( primaryRssi > secondaryRssi )
            {
                if ( primaryRssi == BAD_RSSI_VAL )
                {
                    antennaToUseThisConnection = SECONDARY;
                }
                else
                {
                    //use primary antenna
                    antennaToUseThisConnection = PRIMARY;
                }
            }
            else
            {
                if ( secondaryRssi == BAD_RSSI_VAL )
                {
                    antennaToUseThisConnection = PRIMARY;
                }
                else
                {
                    antennaToUseThisConnection = SECONDARY;
                }
            }

            //now save the new epoch time
            MEM_setEpochTimeLastAntennaSwitch(currentTimestamp);

            //update antenna
            MEM_setAntennaToUse(antennaToUseThisConnection);
        }
        else
        {
            //check how long its been since we tried the other antenna:
            if ( (currentTimestamp == 0) ||
                    ( (currentTimestamp - timestampLastAntennaSwitch) >= (uint32_t)SECONDS_PER_MONTH * MONTHS_TO_WAIT_FOR_SWITCH) )
            {
                if ( savedAntenna == PRIMARY )
                {
                    antennaToUseThisConnection = SECONDARY;
                }
                else
                {
                    antennaToUseThisConnection = PRIMARY;
                }

                //clear the antenna config in flash, so the next connection we
                //pick based on RSSI
                MEM_setAntennaToUse(UNKNOWN_ANTENNA);
            }
            else
            {
                //if its been less than 1 month, use the one we deemed "best"
                antennaToUseThisConnection = savedAntenna;
            }
        }
    }

    return antennaToUseThisConnection;
}

static void modemPowerAndCellRfConfig(antenna_t config)
{
    PWR_turnOnCellModemPowerSupply();
    SARA_turnOnSequence();

    elogInfo("Cell modem turned on");

    //do not set up antennas if the GPS module is already enabled
    if ( GPS_isGpsEnabled() == false )
    {
        //now set the antenna to primary or secondary:
        if ( config == SECONDARY )
        {
            RF_noPrimaryAntennaCellSecondary();
            elogInfo("secondary antenna");
        }
        else
        {
            RF_cellPrimaryAntennaNoGps();
            elogInfo("primary antenna");
        }
    }
}

static void commandHandlerForConn(int argc, char **argv)
{
    if ((argc >= ONE_ARGUMENT) && (0 == strcmp(argv[FIRST_ARG_IDX], "on")))
    {
        antenna_t antenna = PRIMARY;

        if ( argc == TWO_ARGUMENTS )
        {
            //parse to see which antenna to use
            if (0 == strcmp(argv[SECOND_ARG_IDX], "secondary"))
            {
                antenna = SECONDARY;
            }
            else
            {
                //default to primary
                antenna = PRIMARY;
            }
        }
        else
        {
            //default to primary
            antenna = PRIMARY;
        }

        modemPowerAndCellRfConfig(antenna);

        //init cell uart...need to init AFTER we have already
        //turned on the modem
        UART_initCellUart();

        //send a command to get auto-baud working on the modem
        SARA_getImei();

        //now enable interrupts
        HAL_NVIC_EnableIRQ(UART5_IRQn);

        //send a 2nd time after interrupts are turned on
        SARA_getImei();
    }
    else if  ((argc == ONE_ARGUMENT) && (0 == strcmp(argv[FIRST_ARG_IDX], "off")))
    {
        pollStat = false;
        PWR_turnOffCellModemPowerSupply();
        elogInfo("Turned off modem");
    }
    else if  ((argc == ONE_ARGUMENT) && (0 == strcmp(argv[FIRST_ARG_IDX], "poll")))
    {
        modemPowerAndCellRfConfig(PRIMARY);
        pollStat = true;
    }
    else if ((argc == ONE_ARGUMENT) && (0 == strcmp(argv[FIRST_ARG_IDX], "iccid")))
    {
        SARA_get_Iccid();
    }
    else if ((argc == ONE_ARGUMENT) && (0 == strcmp(argv[FIRST_ARG_IDX], "imei")))
    {
        SARA_getImei();
    }
    else if ((argc == ONE_ARGUMENT) && (0 == strcmp(argv[FIRST_ARG_IDX], "version")))
    {
        SARA_getModemVersion();
    }
    else if ((argc == THREE_ARGUMENTS) && (0 == strcmp(argv[FIRST_ARG_IDX], "tx")))
    {
        uint32_t ch = 0;
        int32_t pow = 0;
        char *p;

        //support only 3G channels:
        /*
            o [1312-1513]: band 4 (1700 MHz)
            o [2712-2863]: band 8 (900 MHz)
            o [4132-4233]: band 5 (850 MHz)
            o [4162-4188]: band 6 (800 MHz)
            o [20312-20363]: band 19 (800 MHz)
            o [9262-9538]: band 2 (1900 MHz)
            o [9612-9888]: band 1 (2100 MHz)
            o [10050-10125]: TD-SCDMA band 34 (2000 MHz)
            o [9400-9600]: TD-SCDMA band 39 (1900 MHz)
         */
        ch = strtol(argv[SECOND_ARG_IDX], &p, 10);
        elogInfo("Channel to TX: %lu", ch);

        //see above for channel selection.
        if ( ch < 1312 )
        {
            elogError("Invalid channel");
        }
        else
        {
            pow = strtol(argv[THIRD_ARG_IDX], &p, 10);
            elogInfo("TX Power: %d dBm", pow);

            if ( pow < -56 || pow > 24 )
            {
                elogError("Invalid power. Must be between -56 and 24 dBm");
            }
            else
            {
                //pass params to the at command
                SARA_startTxTest(ch, pow);
            }
        }
    }
    else
    {
        elogDebug("Unknown Command");
    }
}
