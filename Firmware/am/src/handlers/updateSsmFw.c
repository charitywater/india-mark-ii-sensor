/**************************************************************************************************
* \file     updateSsmFw.c
* \brief    Update the MSP430 with new firmware stored in external SPI flash
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

#include "stdint.h"
#include "stdbool.h"
#include "ssm.h"
#include "mspBslProtocol.h"
#include "logTypes.h"
#include "memMapHandler.h"
#include "flashHandler.h"
#include "FreeRTOS.h"
#include "task.h"
#include "updateSsmFw.h"

#define MAX_FRAM_SECTIONS               9
#define SSM_BOOT_UP_TIME_DELAY_MS       5000

typedef struct  __attribute__ ((__packed__))
{
    uint16_t checksum;
    imageTypes_t type;
    uint32_t length;
    uint32_t fwVersionMajor;
    uint32_t fwVersionMinor;
    uint32_t fwVersionBuild;
    uint16_t framAddress_1;
    uint16_t framAddress_2;
    uint16_t framAddress_3;
    uint16_t framAddress_4;
    uint16_t framAddress_5;
    uint16_t framAddress_6;
    uint16_t framAddress_7;
    uint16_t framAddress_8;
    uint16_t framAddress_9;
    uint16_t framLength_1;
    uint16_t framLength_2;
    uint16_t framLength_3;
    uint16_t framLength_4;
    uint16_t framLength_5;
    uint16_t framLength_6;
    uint16_t framLength_7;
    uint16_t framLength_8;
    uint16_t framLength_9;
}ssmMetaData_t;

#define IMAGE_META_DATA_LEN             sizeof(ssmMetaData_t)

static ssmMetaData_t ssmImageMetaData = {};
static uint32_t externalFlashAddr = 0u;
static uint8_t pageReadBuffer[MT29F1_PAGE_SIZE];

static bool xGetAndValidateMetaDataStruct(uint32_t addr);
static void xFinishProgrammingAndReset(void);
static bool xProgramEachFramSection(uint32_t startAddr);

bool SSM_FW_programBslWithExternalFlashImage(uint32_t startAddrExternal)
{
    bool successful = false;

    //init the static var
    externalFlashAddr = startAddrExternal;

    //read metadata to verify there is a valid image stored in the external flash
    if ( xGetAndValidateMetaDataStruct(externalFlashAddr) == true )
    {
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

            //continue on
            successful = xProgramEachFramSection(externalFlashAddr);

            elogInfo("Finished programming, result %d", successful);

            //disable the mux
            SSM_disableUart();

            vTaskDelay(100);

            if ( successful == true )
            {
                //reset the SSM at this point
                xFinishProgrammingAndReset();

                //give the ssm some time to boot up
                vTaskDelay(SSM_BOOT_UP_TIME_DELAY_MS);

                //make sure we can still comunicate w/ the SSM
                successful = SSM_communicationCheck();
            }
        }
    }
    else
    {
        elogError("Invalid image in the provided address");
    }

    return successful;
}

static bool xProgramEachFramSection(uint32_t startAddr)
{
    uint32_t len = 0;
    uint32_t framAddr = 0;
    uint32_t externalSpiAddr = startAddr + IMAGE_META_DATA_LEN;
    bool res = true;

    for (uint8_t i = 0; i< MAX_FRAM_SECTIONS; i++)
    {
        //first init the length and fram address for each section
        switch (i)
        {
            case 0:
                len = ssmImageMetaData.framLength_1;
                framAddr = ssmImageMetaData.framAddress_1;
                break;
            case 1:
                len = ssmImageMetaData.framLength_2;
                framAddr = ssmImageMetaData.framAddress_2;
                break;
            case 2:
                len = ssmImageMetaData.framLength_3;
                framAddr = ssmImageMetaData.framAddress_3;
                break;
            case 3:
                len = ssmImageMetaData.framLength_4;
                framAddr = ssmImageMetaData.framAddress_4;
                break;
            case 4:
                len = ssmImageMetaData.framLength_5;
                framAddr = ssmImageMetaData.framAddress_5;
                break;
            case 5:
                len = ssmImageMetaData.framLength_6;
                framAddr = ssmImageMetaData.framAddress_6;
                break;
            case 6:
                len = ssmImageMetaData.framLength_7;
                framAddr = ssmImageMetaData.framAddress_7;
                break;
            case 7:
                len = ssmImageMetaData.framLength_8;
                framAddr = ssmImageMetaData.framAddress_8;
                break;
            case 8:
                len = ssmImageMetaData.framLength_9;
                framAddr = ssmImageMetaData.framAddress_9;
                break;
            default:
                break;
        }

        framAddr =  (framAddr & 0x00FFU) << 8 | (framAddr & 0xFF00U) >> 8;
        len =  (len & 0x00FFU) << 8 | (len & 0xFF00U) >> 8;
        elogInfo("Programming SSM section %d, fram addr = x%X, len = %lu", i, framAddr, len);

        while ( len > 0 && res == true )
        {
            //read sections out of external spi flash and send it over uart to the msp's BSL
            if ( len >=  MT29F1_PAGE_SIZE )
            {
                FLASH_read(externalSpiAddr, (uint8_t*)&pageReadBuffer, MT29F1_PAGE_SIZE);

                res = BSL_writeLargeChunkOfDataToMemory(framAddr, MT29F1_PAGE_SIZE, (uint8_t*)&pageReadBuffer);

                if ( res == false )
                {
                    break;
                }
                externalSpiAddr += MT29F1_PAGE_SIZE;
                framAddr += MT29F1_PAGE_SIZE;
                len -= MT29F1_PAGE_SIZE;
            }
            else
            {
                FLASH_read(externalSpiAddr, (uint8_t*)&pageReadBuffer, len);
                res = BSL_writeLargeChunkOfDataToMemory(framAddr, len, (uint8_t*)&pageReadBuffer);

                if ( res == false )
                {
                   break;
                }

                externalSpiAddr += len;
                framAddr += len;
                len = 0;
            }
        }

        //bail any time that the result code is not true
        if ( res != true )
        {
            elogError("Programming SSM BAILING at section %d x%X %lu", i, framAddr, len);
            break;
        }
    }

    return res;
}

static void xFinishProgrammingAndReset(void)
{
   elogInfo("Resetting SSM");

   //physically reset the SSM
   SSM_hardwareReset();
}


static bool xGetAndValidateMetaDataStruct(uint32_t addr)
{
    bool valid = false;

    //get The metadata structure for the given address
    FLASH_read(addr, (uint8_t*) &ssmImageMetaData, IMAGE_META_DATA_LEN);

    if ( ssmImageMetaData.type == SSM_IMAGE )
    {
        valid = true;
    }

    return valid;

}
