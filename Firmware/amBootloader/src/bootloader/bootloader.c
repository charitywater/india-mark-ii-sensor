/*
================================================================================================#=
Module:   Bootloader

Description:
    Bootloader for the AM. Decides if we need to load an image from SPI flash into
    the SMT32 internal flash and does so. Also implements the BL cache

Copyright 2021 Twisthink LLC
This code is licensed under Twisthink license.
See Twisthink-SoftwareLicenseAgreement-Auris.docx for details.
================================================================================================#=
*/

#include "stddef.h"
#include "stdbool.h"
#include "stm32l4xx_hal.h"
#include "logTypes.h"
#include "string.h"
#include "memMapHandler.h"
#include "flashHandler.h"
#include "memoryMap.h"
#include "updateFw.h"
#include "manufacturing.h"
#include "bootloader.h"

//tries to load one of the images before entering LP mode
#define START_COUNT_LIMIT               10

#define IMAGE_META_DATA_LEN             19
#define SW_RESET_KEY                    0xAABBAABB

#define CRC_LEN                         2
#define MAX_CHARS_TO_STORE_IN_RAM       2048

typedef enum
{
    NOMINAL,
    UPGRADE,
    FALLBACK,
    OFF_NOMINAL,
    PANIC,
    UNKNOWN,
    MANUFACTURING,
}reasonLoaded_t;

typedef struct
{
    uint32_t startCount;
    reasonLoaded_t cacheReasonLastLoaded;
    imageSlotTypes_t cacheLastLoaded;
    uint32_t keyToCheckSwResetOrPowerCycle;
}bootloaderCache_t;

typedef struct  __attribute__ ((__packed__))
{
    uint16_t checksum;
    imageTypes_t type;
    uint32_t length;
    uint32_t fwVersionMajor;
    uint32_t fwVersionMinor;
    uint32_t fwVersionBuild;
}metaDataImage_t;

static bool xPanicMode(void);
static bool xNominalMode(void);
static bool xManufacturingMode(void);
static bool xCasesTenThroughSixteen(void);
static bool xCasesSeventeenThroughNineteen(void);
static bool xSetupExternalFlashWithImage(void);

static imageSlotTypes_t xGetAlternateSlot(imageSlotTypes_t slotToGetAlternateOf);
static bool xLoadImageFromSlotIntoFlash(imageSlotTypes_t slotToLoad);
static bool xCheckForImageInSpiFlash(imageSlotTypes_t slot);
static uint16_t xRunningCrc(uint8_t* data_p, uint32_t length);

volatile bootloaderCache_t blCache  __attribute__((section (".noinit")));

static metaDataImage_t imageToLoadMetaData;
static bool imageRegistryValid = false;
static uint16_t crc = 0xFFFF;
static uint8_t nextFlashReadBuffer[MAX_CHARS_TO_STORE_IN_RAM] = {0};

bool BOOT_RunBootloader(void)
{
    bool shouldWeJumpToApp = true;

    elogInfo("%lu , %d, %d", blCache.startCount, blCache.cacheReasonLastLoaded, blCache.cacheLastLoaded);

    //first check if flash has ever been initialized
    if ( blCache.keyToCheckSwResetOrPowerCycle != SW_RESET_KEY && MFG_MODE_checkForImagePackageInternalFlash() == true  )
    {
        bool imagePackageStored = xSetupExternalFlashWithImage();

        if ( imagePackageStored == true )
        {
            blCache.cacheReasonLastLoaded = MANUFACTURING;
            blCache.keyToCheckSwResetOrPowerCycle = SW_RESET_KEY;
            blCache.startCount = 0;

            //reset the processor - we will come back into the BL and hit the else statement on line 109
            //since we now have set up the reset key
            NVIC_SystemReset();
        }
    }
    else
    {
        //init the image registry from external flash first
        imageRegistryValid = MEM_init();

        //check if we just came out of a power reset:
        if ( blCache.keyToCheckSwResetOrPowerCycle != SW_RESET_KEY )
        {
            elogInfo("Power reset, so the BL cache will be uninitialized");
            blCache.startCount = 0;
            blCache.cacheLastLoaded = UNKNOWN_SLOT;
            blCache.cacheReasonLastLoaded = UNKNOWN;

            if( imageRegistryValid == true )
            {
                //if image reg is valid, set the last loaded to the loaded image
                blCache.cacheLastLoaded = MEM_getLoadedImage();
            }

            //now set the key
            blCache.keyToCheckSwResetOrPowerCycle = SW_RESET_KEY;
        }

        //first we need to see if we are dealing with a VALID image registry or not:
        if ( imageRegistryValid == true && (blCache.cacheLastLoaded == A || blCache.cacheLastLoaded == B ) )
        {
            //valid & we know the last loaded image:
            shouldWeJumpToApp = xNominalMode();
        }
        else if ( blCache.cacheReasonLastLoaded == MANUFACTURING )
        {
            shouldWeJumpToApp = xManufacturingMode();
        }
        else
        {
           //invalid image registry/unknown last loaded image
            shouldWeJumpToApp = xPanicMode();
        }

        //increment start count before leaving this function
        blCache.startCount++;
    }


    return shouldWeJumpToApp;
}


static bool xSetupExternalFlashWithImage(void)
{
    //lets see if there is an 'image package' stored in internal flash:
    manufacturingImageVersion_t versionInFlash = {0};
    bool imagePackageStored = false;

   if (  MFG_MODE_checkForImagePackageInternalFlash() == true )
   {
       //there is an image package, copy it to external flash
       if ( MFG_MODE_copyImagePackageToExternalFlash() == true )
       {
            //update the image registry so that when we reset the BL will upgrade
            //to this new image
            versionInFlash = MFG_MODE_getImageVersionFoundInFlash();

            //set the image registry to the default values
            MEM_defaultSection(SECTION_IMAGE_REGISTRY);

            //re-init so we are using the defaulted image registry
            MEM_init();

            //now set the preferred image, firmware version, and write the magic value:
            MEM_setPrimaryImage(A);
            MEM_setImageAoperationalState(OP_UNKNOWN);
            MEM_setImageAversion(versionInFlash.major, versionInFlash.minor, versionInFlash.build);
            MEM_writeMagicValue();

            imagePackageStored = true;
       }
   }

   return imagePackageStored;
}

static bool xManufacturingMode(void)
{
    bool jumpToApp = true;

    if ( xCheckForImageInSpiFlash(A) == true )
    {
        //Load the image into internal flash
        elogInfo("Manufacturing case - load image A");

        //LOAD A SLOT INTO FLASH
        jumpToApp = xLoadImageFromSlotIntoFlash(A);
        blCache.cacheLastLoaded = A;
        blCache.startCount = 0;
        blCache.cacheReasonLastLoaded = MANUFACTURING;
    }
    else
    {
        jumpToApp = false;
    }

    return jumpToApp;
}

static bool xPanicMode(void)
{
    bool jumpToApp = true;

    //no valid image registry so we will just try the images from slot A/B

    //case 5
    if (blCache.cacheLastLoaded == UNKNOWN_SLOT )
    {
        if ( xCheckForImageInSpiFlash(A) == true )
        {
            //start whatever image is already in flash
            elogInfo("BL Case #5 - start and load image A");

            //LOAD A SLOT INTO FLASH
            jumpToApp = xLoadImageFromSlotIntoFlash(A);
            blCache.cacheLastLoaded = A;
            blCache.startCount = 0;
            blCache.cacheReasonLastLoaded = PANIC;
        }
        else if ( xCheckForImageInSpiFlash(B) == true )
        {
            //start whatever image is already in flash
            elogInfo("BL Case #5 - start and load image B");

            //LOAD B SLOT INTO FLASH
            jumpToApp = xLoadImageFromSlotIntoFlash(B);
            blCache.cacheLastLoaded = B;
            blCache.startCount = 0;
            blCache.cacheReasonLastLoaded = PANIC;
        }
        else
        {
            //start whatever image is already in flash
            elogInfo("BL Case #5 - Restart whatever image is already in flash");

            blCache.cacheLastLoaded = UNKNOWN_SLOT;
            blCache.startCount = 0;
            blCache.cacheReasonLastLoaded = PANIC;
        }
    }

    //cases 6-7
    else if ( blCache.cacheLastLoaded == A )
    {
        if ( blCache.startCount <= START_COUNT_LIMIT )
        {
            blCache.cacheLastLoaded = A;
            blCache.cacheReasonLastLoaded = PANIC;
            elogInfo("BL Case #6 - Restart Image A.");
        }
        else
        {
            //if B has an image try to load it
            if ( xCheckForImageInSpiFlash(B) == true )
            {
                //LOAD B SLOT INTO FLASH
                jumpToApp = xLoadImageFromSlotIntoFlash(B);

                blCache.cacheLastLoaded = B;
                blCache.startCount = 0;
                blCache.cacheReasonLastLoaded = PANIC;

                //start whatever image is already in flash
                elogInfo("BL Case #7 - Start Count Limit Exceeded, Load and Start Image B.");
            }
            else
            {
                //TODO SOMETHING HERE
                elogError("BL Case #7 - should this be like Case 9? Just goto low power mode?");
            }
        }
    }

    //cases 8 - 9
    else if ( blCache.cacheLastLoaded == B )
    {
        if ( blCache.startCount <= START_COUNT_LIMIT )
        {
            blCache.cacheLastLoaded = B;
            blCache.cacheReasonLastLoaded = PANIC;
            elogInfo("BL Case #8 - Restart Image B.");
        }
        else
        {
            //Start Count Limit Exceeded.
            // We've tried A and B multiple times
            //No recourse available... shutdown.
            elogError("BL Case 9 - Goto Low Power Mode.");
            jumpToApp = false;
        }
    }
    else
    {
        elogError("Case not supported");
    }


    return jumpToApp;
}

static bool xNominalMode(void)
{
    bool jump = true;

    imageSlotTypes_t primarySlot = MEM_getPrimaryImage();
    imageSlotTypes_t alternateSlot = xGetAlternateSlot(primarySlot);
    if ( blCache.cacheLastLoaded == primarySlot )
    {
        //cases 10 -16
        jump = xCasesTenThroughSixteen();
    }
    else if (blCache.cacheLastLoaded == alternateSlot )
    {
        //cases 17 - 19
        jump = xCasesSeventeenThroughNineteen();
    }
    else
    {
        //invalid case...
        //TODO something
        elogError("INVALID CASE. HANDLE THIS");
    }

    return jump;
}

static bool xCasesTenThroughSixteen(void)
{
    bool jumpToApp = true;

    imageSlotTypes_t primarySlot = MEM_getPrimaryImage();
    imageSlotTypes_t alternateSlot = xGetAlternateSlot(primarySlot);
    imageOperationalState_t primaryOpState;
    imageOperationalState_t alternateOpState;

    if ( primarySlot == A )
    {
        primaryOpState = MEM_getImageAoperationalState();
        alternateOpState = MEM_getImageBoperationalState();
    }
    else
    {
        primaryOpState = MEM_getImageBoperationalState();
        alternateOpState = MEM_getImageAoperationalState();
    }

    //cases 10 - 12 Unknown operational state
    if ( primaryOpState == OP_UNKNOWN )
    {
        if ( blCache.startCount <= START_COUNT_LIMIT )
        {
            elogInfo("BL case 10 - restart primary image for upgrade");
            blCache.cacheLastLoaded = primarySlot;
            blCache.cacheReasonLastLoaded = UPGRADE;
        }
        else
        {
            if ( alternateOpState != FAILED  && xCheckForImageInSpiFlash(alternateSlot) == true)
            {
                elogInfo("BL case 11 - Load and Start Alternate Image");

                //LOAD image into flash
                jumpToApp = xLoadImageFromSlotIntoFlash(alternateSlot);

                blCache.cacheLastLoaded = alternateSlot;
                blCache.startCount = 0;
                blCache.cacheReasonLastLoaded = FALLBACK;
            }
            else
            {
                elogError("BL Case #12 - Panic! Primary is Failing, Alternate is already Failed.");
                elogError(" Goto Low-Power Mode" );

                blCache.cacheLastLoaded = primarySlot;
                blCache.cacheReasonLastLoaded = PANIC;

                jumpToApp = false;
            }
        }
    }

    //cases 13 -14
    else if ( primaryOpState == FAILED )
    {
        if ( alternateOpState != FAILED && xCheckForImageInSpiFlash(alternateSlot) == true )
        {
            elogInfo("BL case 13 - Load and Start Alternate Image.");

            //flash function
            //LOAD image into flash
            jumpToApp = xLoadImageFromSlotIntoFlash(alternateSlot);

            blCache.cacheLastLoaded = alternateSlot;
            blCache.startCount = 0;
            blCache.cacheReasonLastLoaded = FALLBACK;
        }
        else
        {
            elogError("BL Case #14 - Panic! Images in both slots are Failed.");
            elogError(" Goto Low-Power Mode" );

            blCache.cacheLastLoaded = primarySlot;
            blCache.cacheReasonLastLoaded = PANIC;
            jumpToApp = false;
        }
    }
    else if ( primaryOpState == PARTIAL )
    {
        elogInfo("BL case 15 - Restart primary image for upgrade.");

        blCache.cacheLastLoaded = primarySlot;
        blCache.cacheReasonLastLoaded = UPGRADE;
    }
    else if ( primaryOpState == FULL )
    {
        elogInfo("BL case 16 - Restart primary image nominal.");

        blCache.cacheLastLoaded = primarySlot;
        blCache.cacheReasonLastLoaded = NOMINAL;
    }
    else
    {
        elogError("CASE NOT SUPPORTED");
    }

    return jumpToApp;
}

static bool xCasesSeventeenThroughNineteen(void)
{
    bool jumpToApp = true;

    imageSlotTypes_t primarySlot = MEM_getPrimaryImage();
    imageSlotTypes_t alternateSlot = xGetAlternateSlot(primarySlot);
    imageOperationalState_t primaryOpState;
    imageOperationalState_t alternateOpState;

    if ( primarySlot == A )
    {
       primaryOpState = MEM_getImageAoperationalState();
       alternateOpState = MEM_getImageBoperationalState();
    }
    else
    {
       primaryOpState = MEM_getImageBoperationalState();
       alternateOpState = MEM_getImageAoperationalState();
    }

    if ( primaryOpState != FAILED &&  xCheckForImageInSpiFlash(primarySlot) == true )
    {
        elogInfo("BL case 17 - Load and Start Primary Image for Upgrade.");
        //LOAD image into flash
        jumpToApp = xLoadImageFromSlotIntoFlash(primarySlot);

        blCache.cacheLastLoaded = primarySlot;
        blCache.startCount = 0;
        blCache.cacheReasonLastLoaded = UPGRADE;
    }
    else
    {
        if ( alternateOpState != FAILED )
        {
            elogInfo("BL case 18 - Restart Alternate Off-Nominal.");

            blCache.cacheLastLoaded = alternateSlot;
            blCache.cacheReasonLastLoaded = OFF_NOMINAL;
        }
        else
        {
            elogError("BL Case #19 - Panic! Images in both slots are Failed.");
            elogError(" Goto Low-Power Mode" );

            blCache.cacheLastLoaded = alternateSlot;
            blCache.cacheReasonLastLoaded = PANIC;
            jumpToApp = false;
        }
    }

    return jumpToApp;
}

static bool xLoadImageFromSlotIntoFlash(imageSlotTypes_t slotToLoad)
{
    bool res = false;
    uint8_t retries = 3;

    //retry a couple times if we were unable to program the flash for some reason
    for (retries = 3; retries > 0; retries --)
    {
        if ( slotToLoad == A )
        {
           res = UPDATE_readExternalFlashAndProgramInternal(APP_MEM_ADR_FW_APPLICATION_AM_A_START + IMAGE_META_DATA_LEN, imageToLoadMetaData.length);

           if ( res == true )
           {
               break;
           }
        }
        else if ( slotToLoad == B )
        {
           res = UPDATE_readExternalFlashAndProgramInternal(APP_MEM_ADR_FW_APPLICATION_AM_B_START + IMAGE_META_DATA_LEN, imageToLoadMetaData.length);

           if ( res == true )
           {
               break;
           }
        }
        else
        {
           elogError("INVALID SLOT");
           break;
        }
    }

    return res;
}


static bool xCheckForImageInSpiFlash(imageSlotTypes_t slot)
{
    bool valid = true;
    uint32_t tempLen = 0;
    uint32_t addr = 0;
    uint16_t calculatedCrc = 0;
    uint16_t storedCrc = 0;

    //reinit crc value
    crc = 0xFFFF;

    //set to invalid values first
    memset(&imageToLoadMetaData, 0xFF, sizeof(metaDataImage_t));

    //since the image registry was invalid, lets check the actual slots to see if there appears to be a
    //valid image there

    if ( slot == A )
    {
        FLASH_read(APP_MEM_ADR_FW_APPLICATION_AM_A_START, (uint8_t*) &imageToLoadMetaData, IMAGE_META_DATA_LEN );
        FLASH_read(APP_MEM_ADR_FW_APPLICATION_AM_A_START, (uint8_t*)&storedCrc, CRC_LEN);

        //flip bit order of the length
        imageToLoadMetaData.length =  (imageToLoadMetaData.length & 0x000000FFU) << 24 | (imageToLoadMetaData.length & 0x0000FF00U) << 8 |
                (imageToLoadMetaData.length & 0x00FF0000U) >> 8 | (imageToLoadMetaData.length & 0xFF000000U) >> 24;

        if( imageToLoadMetaData.type == AM_IMAGE )
        {
            tempLen = imageToLoadMetaData.length;
            addr = APP_MEM_ADR_FW_APPLICATION_AM_A_START + IMAGE_META_DATA_LEN;
        }
        else
        {
            valid = false;
        }
    }
    else if ( slot == B )
    {
        FLASH_read(APP_MEM_ADR_FW_APPLICATION_AM_B_START, (uint8_t*)&imageToLoadMetaData, IMAGE_META_DATA_LEN );
        FLASH_read(APP_MEM_ADR_FW_APPLICATION_AM_B_START, (uint8_t*)&storedCrc, CRC_LEN);

        //flip bit order of the length
        imageToLoadMetaData.length =  (imageToLoadMetaData.length & 0x000000FFU) << 24 | (imageToLoadMetaData.length & 0x0000FF00U) << 8 |
                        (imageToLoadMetaData.length & 0x00FF0000U) >> 8 | (imageToLoadMetaData.length & 0xFF000000U) >> 24;

        if( imageToLoadMetaData.type == AM_IMAGE )
        {
            tempLen = imageToLoadMetaData.length;
            addr = APP_MEM_ADR_FW_APPLICATION_AM_B_START + IMAGE_META_DATA_LEN;
        }
        else
        {
            valid = false;
        }
    }
    else
    {
        valid = false;
        elogError("BAD Input");
    }

    //if so far so good, compute the checksum to make sure the full image is there
    if ( valid == true)
    {
        //flip bit order of the length back to original content read out of flash
        imageToLoadMetaData.length =  (imageToLoadMetaData.length & 0x000000FFU) << 24 | (imageToLoadMetaData.length & 0x0000FF00U) << 8 |
                (imageToLoadMetaData.length & 0x00FF0000U) >> 8 | (imageToLoadMetaData.length & 0xFF000000U) >> 24;


        memcpy(&nextFlashReadBuffer, &imageToLoadMetaData.type, IMAGE_META_DATA_LEN - CRC_LEN);
        calculatedCrc = xRunningCrc((uint8_t*)&nextFlashReadBuffer, IMAGE_META_DATA_LEN - CRC_LEN);

        //flip bit order of the length back
        imageToLoadMetaData.length =  (imageToLoadMetaData.length & 0x000000FFU) << 24 | (imageToLoadMetaData.length & 0x0000FF00U) << 8 |
                (imageToLoadMetaData.length & 0x00FF0000U) >> 8 | (imageToLoadMetaData.length & 0xFF000000U) >> 24;


        //compute AM image checksum
        while (tempLen)
        {
            if(tempLen < MAX_CHARS_TO_STORE_IN_RAM)
            {
                FLASH_read(addr, (uint8_t*) &nextFlashReadBuffer, tempLen);

                calculatedCrc = xRunningCrc((uint8_t*)&nextFlashReadBuffer, tempLen);
                tempLen = 0;
            }
            else
            {
                FLASH_read(addr, (uint8_t*)&nextFlashReadBuffer, MAX_CHARS_TO_STORE_IN_RAM);

                calculatedCrc = xRunningCrc((uint8_t*)&nextFlashReadBuffer, MAX_CHARS_TO_STORE_IN_RAM);
                tempLen-=MAX_CHARS_TO_STORE_IN_RAM;
                addr+= MAX_CHARS_TO_STORE_IN_RAM;
            }
        }

        //shift bytes since they are read out of flash backwards:
        storedCrc = (uint8_t)storedCrc<<8 | storedCrc>>8;

        if ( calculatedCrc == storedCrc )
        {
            valid = true;
        }
        else
        {
            valid = false;
        }
    }

    return valid;
}


static imageSlotTypes_t xGetAlternateSlot(imageSlotTypes_t slotToGetAlternateOf)
{
    if ( slotToGetAlternateOf == A )
    {
        return B;
    }
    else if ( slotToGetAlternateOf == B )
    {
        return A;
    }
    else
    {
        return UNKNOWN_SLOT;
    }
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
