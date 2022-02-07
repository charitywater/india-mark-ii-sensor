/**************************************************************************************************
* \file     PE42424A_RF.c
* \brief    API to control the rf switches. See the truth table for how to enable antennas to the gps/cell
*           moudles using the switches
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

#include "CLI.h"
#include "logTypes.h"
#include "stddef.h"
#include "string.h"
#include "PE42424A_RF.h"

static void setCellSwitch(void);
static void clearCellSwitch(void);
static void setGpsSwitch(void);
static void clearGpsSwitch(void);
static void commandHandlerForRf(int argc, char **argv);

/* truth table for the two PE42424A switches:
 * "cell switch" and "gps switch" match names given on the schematic for the 2 switches.

    cell switch  | Gps switch  | Primary Ant  | Secondary Ant
    ************************************************************
          L      |     L       |     none     |    cell
          L      |     H       |     none     |    gps
          H      |     L       |     cell     |    none
          H      |     H       |     cell     |    gps
*/

void RF_initRfSwitchesAndCli(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    HAL_GPIO_WritePin(CELL_RF_SWITCH_PORT, CELL_RF_SWITCH_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPS_RF_SWITCH_PORT, GPS_RF_SWITCH_PIN, GPIO_PIN_SET);

    /* Set up the switches into default settings - cell as primary antenna and gps secondary */
    GPIO_InitStruct.Pin = CELL_RF_SWITCH_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(CELL_RF_SWITCH_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPS_RF_SWITCH_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPS_RF_SWITCH_PORT, &GPIO_InitStruct);

    /*register a command handler function */
    CLI_Command_Handler_s cmdHandler;
    cmdHandler.ptrFunction = &commandHandlerForRf;
    cmdHandler.cmdString   = "rf";
    cmdHandler.usageString = "\n\r\t set [cell | gps] \n\r\tclear [cell | gps]";
    CLI_registerThisCommandHandler(&cmdHandler);
}

void RF_cellPrimaryAntennaNoGps(void)
{
    setCellSwitch();
    clearGpsSwitch();
}

void RF_cellPrimaryAntennaGpsSecondary(void)
{
    setCellSwitch();
    setGpsSwitch();
}

void RF_noPrimaryAntennaGpsSecondary(void)
{
    clearCellSwitch();
    setGpsSwitch();
}

void RF_noPrimaryAntennaCellSecondary(void)
{
    clearCellSwitch();
    clearGpsSwitch();
}

static void setCellSwitch(void)
{
    HAL_GPIO_WritePin(CELL_RF_SWITCH_PORT, CELL_RF_SWITCH_PIN, GPIO_PIN_SET);
}

static void clearCellSwitch(void)
{
    HAL_GPIO_WritePin(CELL_RF_SWITCH_PORT, CELL_RF_SWITCH_PIN, GPIO_PIN_RESET);
}

static void setGpsSwitch(void)
{
    HAL_GPIO_WritePin(GPS_RF_SWITCH_PORT, GPS_RF_SWITCH_PIN, GPIO_PIN_SET);
}

static void clearGpsSwitch(void)
{
    HAL_GPIO_WritePin(GPS_RF_SWITCH_PORT, GPS_RF_SWITCH_PIN, GPIO_PIN_RESET);
}

static void commandHandlerForRf(int argc, char **argv)
{
    if ((argc == TWO_ARGUMENTS) && (0 == strcmp(argv[FIRST_ARG_IDX], "set")))
    {
        if ( 0 == strcmp(argv[SECOND_ARG_IDX], "gps") )
        {
           elogDebug("setting gps switch pin");
           setGpsSwitch();

        }
        else if ( 0 == strcmp(argv[SECOND_ARG_IDX], "cell") )
        {
            elogDebug("setting cell switch pin");
            setCellSwitch();

        }
        else
        {
           elogDebug("Invalid arg");
        }
    }
    else  if ((argc == TWO_ARGUMENTS) && (0 == strcmp(argv[FIRST_ARG_IDX], "clear")))
    {
        if ( 0 == strcmp(argv[SECOND_ARG_IDX], "gps") )
        {
           elogDebug("clearing gps switch pin");
           clearGpsSwitch();

        }
        else if ( 0 == strcmp(argv[SECOND_ARG_IDX], "cell") )
        {
            elogDebug("clearing cell switch pin");
            clearCellSwitch();

        }
        else
        {
           elogDebug("Invalid arg");
        }
    }
    else
    {
        elogDebug("Unknown Command");
    }
}
