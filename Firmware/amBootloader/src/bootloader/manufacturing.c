/*
================================================================================================#=
Module:   Manufacturing

Description:
    Move internal flash to external and update image registry
    This should only be called during the manufacturing process when
    an image package is loaded into internal flash

Copyright 2021 Twisthink LLC
This code is licensed under Twisthink license.
See Twisthink-SoftwareLicenseAgreement-Auris.docx for details.
================================================================================================#=
*/

#include "stdint.h"
#include "string.h"
#include <stm32l4xx_hal.h>
#include "logTypes.h"
#include <flashHandler.h>
#include <manufacturing.h>
#include "memoryMap.h"
#include "memMapHandler.h"
#include "stmFlash.h"

#define IMAGE_TYPE_BYTE      0
#define IMAGE_LEN_BYTE       1
#define IMAGE_CRC_BYTE       5
#define IMAGE_MAJOR_BYTE     12
#define IMAGE_MINOR_BYTE     16
#define IMAGE_BUILD_BYTE     20
#define IMAGE_LEN_SIZE       sizeof(uint32_t)
#define IMAGE_TYPE_SIZE      sizeof(uint8_t)
#define IMAGE_CRC_SIZE       sizeof(uint16_t)
#define CS_START_IDX         7
#define MAX_AM_IMAGE_LEN     0x1DC130 //2Mb - 50kB for the BL
#define MAX_SSM_IMAGE_LEN    0xFF80

//data read from INTERNAL flash
static uint8_t dataBuffer[STM_FLASH_PAGE_SIZE];
static uint16_t crc = 0xFFFF;
static manufacturingImageVersion_t imagePackageVersion = {};

static uint16_t xRunningCrc(uint8_t* data_p, uint32_t length);

bool MFG_MODE_checkForImagePackageInternalFlash(void)
{
    uint32_t imageLength = 0u;
    uint16_t storedCrc = 0u;
    uint8_t imageType = 0u;
    uint32_t ssmImageOffset = 0u;
    volatile uint8_t *internalFlash = (volatile uint8_t *) INTERNAL_FLASH_START_ADDR;
    bool validPackage = true;
    uint8_t nextByte = 0u;

    //read out the first byte, length, and checksum
    imageType = internalFlash[IMAGE_TYPE_BYTE];
    imageLength = (internalFlash[IMAGE_LEN_BYTE] << 24) | (internalFlash[IMAGE_LEN_BYTE + 1] << 16) |
                    (internalFlash[IMAGE_LEN_BYTE + 2] << 8) | internalFlash[IMAGE_LEN_BYTE + 3];
    storedCrc = (internalFlash[IMAGE_CRC_BYTE]<<8) | internalFlash[IMAGE_CRC_BYTE + 1];

    //save off the version
    imagePackageVersion.major = (internalFlash[IMAGE_MAJOR_BYTE] << 24) | (internalFlash[IMAGE_MAJOR_BYTE + 1] << 16) |
            (internalFlash[IMAGE_MAJOR_BYTE + 2] << 8) | internalFlash[IMAGE_MAJOR_BYTE + 3];
    imagePackageVersion.minor = (internalFlash[IMAGE_MINOR_BYTE] << 24) | (internalFlash[IMAGE_MINOR_BYTE + 1] << 16) |
            (internalFlash[IMAGE_MINOR_BYTE + 2] << 8) | internalFlash[IMAGE_MINOR_BYTE + 3];
    imagePackageVersion.build = (internalFlash[IMAGE_BUILD_BYTE] << 24) | (internalFlash[IMAGE_BUILD_BYTE + 1] << 16) |
            (internalFlash[IMAGE_BUILD_BYTE + 2] << 8) | internalFlash[IMAGE_BUILD_BYTE + 3];

    //compute ssm image package offset
    ssmImageOffset = imageLength + IMAGE_LEN_SIZE + IMAGE_TYPE_SIZE;

    elogInfo("AM Image Type %x", imageType);
    elogInfo("AM Image Length %x", imageLength);
    elogInfo("AM Stored CRC %x", storedCrc);

    //bail now if not an AM image type or invalid length
    if ( imageType == AM_IMAGE && imageLength <= MAX_AM_IMAGE_LEN )
    {
        //compute the checksum of the AM image
        for(uint32_t i = CS_START_IDX; i < (imageLength-IMAGE_CRC_SIZE) + CS_START_IDX; i++)
        {
            nextByte = internalFlash[i];
            xRunningCrc((uint8_t*)&nextByte, 1);
        }

        elogInfo("AM Stored CRC %x, Calculated: %x", storedCrc, crc);

        //check computed cs against the stored cs
        if ( storedCrc != crc )
        {
            elogInfo("AM IMAGE CRC MISMATCH - Not an image package");
            validPackage = false;
        }

        //continue if the AM package was good
        if ( validPackage == true )
        {
            //Now do the same for the SSM package
            //read out the first byte, length, and checksum
            imageType = internalFlash[IMAGE_TYPE_BYTE + ssmImageOffset];
            imageLength = (internalFlash[IMAGE_LEN_BYTE + ssmImageOffset] << 24) | (internalFlash[IMAGE_LEN_BYTE + 1 + ssmImageOffset] << 16) |
                       (internalFlash[IMAGE_LEN_BYTE + 2 + ssmImageOffset] << 8) | internalFlash[IMAGE_LEN_BYTE + 3 + ssmImageOffset];
            storedCrc = (internalFlash[IMAGE_CRC_BYTE + ssmImageOffset]<<8) | internalFlash[IMAGE_CRC_BYTE + 1 +ssmImageOffset];

            elogInfo("SSM Image Type %x", imageType);
            elogInfo("SSM Image Length %x", imageLength);
            elogInfo("SSM Stored CRC %x", storedCrc);

            //bail now if not an AM image type
            if ( imageType == SSM_IMAGE && imageLength <= MAX_SSM_IMAGE_LEN )
            {
                //compute the checksum of the SSM image
                crc = 0xFFFF;

                for(uint32_t i = CS_START_IDX + ssmImageOffset; i < (imageLength - IMAGE_CRC_SIZE) + ssmImageOffset + CS_START_IDX; i++)
                {
                    nextByte = internalFlash[i];
                    xRunningCrc((uint8_t*)&nextByte, 1);
                }

                elogInfo("SSM Stored CRC %x, Calculated: %x", storedCrc, crc);

                //check computed cs against the stored cs
                if ( storedCrc != crc )
                {
                    elogInfo("SSM IMAGE CRC MISMATCH - not an image package");
                    validPackage = true;
                }
                else
                {
                    validPackage = true;
                    elogInfo("Valid AM and SSM images");
                }
            }
            else
            {
                validPackage = false;
            }
        }
    }
    else
    {
        validPackage = false;
    }

   //reinit crc
   crc = 0xFFFF;

   return validPackage;

}

manufacturingImageVersion_t MFG_MODE_getImageVersionFoundInFlash(void)
{
    return imagePackageVersion;
}

bool MFG_MODE_copyImagePackageToExternalFlash(void)
{
    uint32_t imageLength = 0u;
    uint32_t ssmImageOffset = 0u;
    uint32_t externalFlashAddr = APP_MEM_ADR_FW_APPLICATION_AM_A_START;
    uint32_t bytesLeftToRead = 0;
    uint32_t startByte = IMAGE_LEN_SIZE + IMAGE_TYPE_SIZE;
    uint32_t bytesReadThisRound = 0;
    uint32_t imagePackageIdx = 0;
    uint16_t idx = 0;
    volatile uint8_t *internalFlash = (volatile uint8_t *) INTERNAL_FLASH_START_ADDR;

    elogInfo("Copying internal flash to external");

    //read out the length
    imageLength = (internalFlash[IMAGE_LEN_BYTE] << 24) | (internalFlash[IMAGE_LEN_BYTE + 1] << 16) |
                    (internalFlash[IMAGE_LEN_BYTE + 2] << 8) | internalFlash[IMAGE_LEN_BYTE + 3];

    //compute ssm image package offset
    ssmImageOffset = imageLength + IMAGE_LEN_SIZE + IMAGE_TYPE_SIZE;

    //page by page update external flash with the internal image package
    bytesLeftToRead = imageLength;

    while (bytesLeftToRead)
    {
        if ( bytesLeftToRead >= STM_FLASH_PAGE_SIZE )
        {
            bytesReadThisRound = STM_FLASH_PAGE_SIZE;
        }
        else
        {
            bytesReadThisRound = bytesLeftToRead;
        }

        memset(&dataBuffer, 0, STM_FLASH_PAGE_SIZE);

        idx = 0;

        //read the next ~4096 bytes of internal flash and store it in a buffer
        for(imagePackageIdx = startByte; imagePackageIdx < (bytesReadThisRound + startByte); imagePackageIdx++)
        {
            dataBuffer[idx] = internalFlash[imagePackageIdx];
            idx++;
        }

        //write to flash
        FLASH_write(externalFlashAddr, (uint8_t*)dataBuffer, bytesReadThisRound);

        //update for next loop
        bytesLeftToRead -= bytesReadThisRound;
        startByte += bytesReadThisRound;
        externalFlashAddr += bytesReadThisRound;
    }


    //now store the SSM image package
    imageLength = (internalFlash[IMAGE_LEN_BYTE + ssmImageOffset] << 24) | (internalFlash[IMAGE_LEN_BYTE + 1 + ssmImageOffset] << 16) |
               (internalFlash[IMAGE_LEN_BYTE + 2 + ssmImageOffset] << 8) | internalFlash[IMAGE_LEN_BYTE + 3 + ssmImageOffset];

    bytesLeftToRead = imageLength;
    externalFlashAddr = APP_MEM_ADR_FW_APPLICATION_SSM_A_START;
    startByte = ssmImageOffset + IMAGE_LEN_SIZE + IMAGE_TYPE_SIZE;

    while (bytesLeftToRead)
    {
        if ( bytesLeftToRead >= STM_FLASH_PAGE_SIZE )
        {
            bytesReadThisRound = STM_FLASH_PAGE_SIZE;
        }
        else
        {
            bytesReadThisRound = bytesLeftToRead;
        }

        memset(&dataBuffer, 0, STM_FLASH_PAGE_SIZE);

        idx = 0;

        //read the next ~4096 bytes of internal flash and store it in a buffer
        for(imagePackageIdx = startByte; imagePackageIdx < (bytesReadThisRound + startByte); imagePackageIdx++)
        {
            dataBuffer[idx] = internalFlash[imagePackageIdx];
            idx++;
        }

        //write to flash
        FLASH_write(externalFlashAddr, (uint8_t*)dataBuffer, bytesReadThisRound);

        //update for next loop
        bytesLeftToRead -= bytesReadThisRound;
        startByte += bytesReadThisRound;
        externalFlashAddr += bytesReadThisRound;
    }

    return true;
}

/*
     CRC-16 Attributes:
    Name                 |   Polynomial | Reversed? |  Init-value | XOR-out Check
    crc-ccitt-false [1]  |   0x11021    | False     | 0xFFFF      | 0x0000  0x29B1
 */
static uint16_t xRunningCrc(uint8_t* data_p, uint32_t length)
{
    uint8_t x;

    while (length--)
    {
        x = crc >> 8 ^ *data_p++;
        x ^= x>>4;
        crc = (crc << 8) ^ ((uint16_t)(x << 12)) ^ ((uint16_t)(x <<5)) ^ ((uint16_t)x);
    }

    return crc;
}

