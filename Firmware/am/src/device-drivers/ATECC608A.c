/**************************************************************************************************
* \file     ATECC608A.c
* \brief    Init the ATECC608A device and provide CLI commands to communicate with the chip
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

#include <stm32l4xx_hal.h>
#include "stddef.h"
#include "i2c.h"
#include "CLI.h"
#include "logTypes.h"
#include "string.h"
#include "stdbool.h"
#include "FreeRTOS.h"
#include "task.h"
#include "cryptoauthlib.h"
#include "ATECC608A.h"
#include "basic/atca_basic.h"

#define ATE_BYTES_PER_RANDOM_NUM    32
#define ATE_SERIAL_NUM_LEN          9

static uint8_t deviceSerialNumber[ATE_SERIAL_NUM_LEN] = {};

static void xAteCmdHandler(int argc, char **argv);

void ATECC_init(void)
{
    ATCA_STATUS initialized;

    /* register CLI handler */
    CLI_Command_Handler_s cmdHandler;
    cmdHandler.ptrFunction = &xAteCmdHandler;
    cmdHandler.cmdString   = "cryp";
    cmdHandler.usageString = "\n\r\trand \n\r\tserial";
    CLI_registerThisCommandHandler(&cmdHandler);
}

bool ATECC_getUniqueId(uint8_t *id)
{
    ATCA_STATUS idReadingStatus;
    idReadingStatus = atcab_read_serial_number(id);

    if ( idReadingStatus != ATCA_SUCCESS )
    {
        elogError("Unable to get ID from Crypto");
        return false;
    }

    return true;
}

//function to test out the crypto chip
void ATECC_printRandomNum(void)
{
    ATCA_STATUS stat;
    uint8_t randomNumber[ATE_BYTES_PER_RANDOM_NUM];

    memset((uint8_t*)&randomNumber, 0, ATE_BYTES_PER_RANDOM_NUM);

    // get a random number from the chip
    stat = atcab_random((uint8_t*)&randomNumber);

    //put back into idle mode
    atcab_idle();

    elogInfo("Status from rand %d", stat);

    for(uint8_t i = 0; i< 32; i++)
    {
        elogInfo("random Num %d: %d", i, randomNumber[i]);
    }
}

static void xAteCmdHandler(int argc, char **argv)
{
    ATCA_STATUS stat;
    char crypIdString[ATE_SERIAL_NUM_LEN*2];

    if ((argc == ONE_ARGUMENT) && (0 == strcmp(argv[FIRST_ARG_IDX], "rand")))
    {
       ATECC_printRandomNum();
    }
    else if ((argc == ONE_ARGUMENT) && (0 == strcmp(argv[FIRST_ARG_IDX], "serial")))
    {
        stat = atcab_read_serial_number((uint8_t*)&deviceSerialNumber);

        if ( stat != ATCA_SUCCESS)
        {
            elogError("Error reading crypto serial number");
        }
        else
        {
            //now convert the byte array to hex character array:
            for (uint8_t i = 0; i < ATE_SERIAL_NUM_LEN ; i++)
            {
               //each byte takes up 2 characters:
               sprintf(&crypIdString[i*2],"%02x",deviceSerialNumber[i]);
            }

            //log the ID:
            elogInfo("Crypto ID: %s", crypIdString);
        }
    }
    else
    {
        elogDebug("unknown crypto command");
    }
}
