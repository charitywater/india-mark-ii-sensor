/*
================================================================================================#=
Module:   Update FW

Description:
    Pull data from SPI flash and load into STM32 internal flash. Zero pads
    remaining STM32 internal flash

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
#include "memoryMap.h"
#include "stmFlash.h"
#include "updateFw.h"

#define NUM_U32_PER_PAGE            STM_FLASH_PAGE_SIZE/sizeof(uint32_t)

static uint32_t xFirstPage = 0;
static uint32_t xNumPages = 0;
static uint32_t xBankNumber = 0;
static uint32_t xIntAddress = 0;
static uint32_t xPageError = 0;

static uint32_t xExternalFlashAddr = 0;
static uint32_t xImageSize = 0;

//used for Erase procedure
static FLASH_EraseInitTypeDef xPageEraseInitStruct;

//data read from EXTERNAL flash to be put into INTERNAL flash
static uint32_t dataToWriteBuffer[NUM_U32_PER_PAGE];

static uint32_t xGetPage(uint32_t addressInternalFlash);
static uint32_t xGetBank(uint32_t addressInternalFlash);


bool UPDATE_readExternalFlashAndProgramInternal(uint32_t startAddrExternal, uint32_t length )
{
    bool successfulProgram = true;
    uint32_t lowerWord = 0;
    uint32_t upperWord = 1;
    uint64_t nextWords = 0;

    //check that we wont exceed internal OR external flash:
    if ( length > (INTERNAL_FLASH_END_ADDR - INTERNAL_FLASH_START_ADDR) || ( startAddrExternal + length )> MT29F1_MAX_ADDR )
    {
        elogError("INVALID LEN");
        return false;
    }
    //set external flash address
    xExternalFlashAddr = startAddrExternal;

    //set size of the image stored in external flash
    xImageSize = length;

    //clear data buffer
    memset(&dataToWriteBuffer, 0, STM_FLASH_PAGE_SIZE);

    // Unlock the Flash to enable the flash control register access
    HAL_FLASH_Unlock();

    // Erase bank 1 of flash AFTER the bootloader:
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_ERRORS);
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_OPTVERR);

    //set up bank 1 erase:
    xFirstPage = xGetPage(INTERNAL_FLASH_START_ADDR);
    xNumPages = xGetPage(INTERNAL_FLASH_END_ADDR) - xFirstPage + 2;
    xBankNumber = xGetBank(INTERNAL_FLASH_START_ADDR);

    xPageEraseInitStruct.TypeErase   = FLASH_TYPEERASE_PAGES;
    xPageEraseInitStruct.Banks       = xBankNumber;
    xPageEraseInitStruct.Page        = xFirstPage;
    xPageEraseInitStruct.NbPages     = xNumPages;

    //erase & check result
    if (HAL_FLASHEx_Erase(&xPageEraseInitStruct, &xPageError) != HAL_OK)
    {
        elogError("COULDNT ERASE FLASH BANK 1");
        successfulProgram = false;
    }
    else
    {
        //second bank erase:
        __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_ERRORS);
        __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_OPTVERR);

        xFirstPage = xGetPage(BANK_2_ADDR);
        xNumPages = xGetPage(INTERNAL_FLASH_END_ADDR) - xFirstPage + 1;
        xBankNumber = xGetBank(BANK_2_ADDR);

        xPageEraseInitStruct.TypeErase   = FLASH_TYPEERASE_PAGES;
        xPageEraseInitStruct.Banks       = xBankNumber;
        xPageEraseInitStruct.Page        = xFirstPage;
        xPageEraseInitStruct.NbPages     = xNumPages;

        //erase bank 2 and check result
        if (HAL_FLASHEx_Erase(&xPageEraseInitStruct, &xPageError) != HAL_OK)
        {
            /* Error occurred while page erase. */
             elogError("COULDNT ERASE FLASH BANK 2");
             successfulProgram = false;
        }
        else
        {
            //set internal flash address;
            xIntAddress = INTERNAL_FLASH_START_ADDR;

            //loop through, reading a page worth of data from the external flash and writing it to internal
            while ( xIntAddress < INTERNAL_FLASH_END_ADDR && (xExternalFlashAddr < (xImageSize + startAddrExternal)) && successfulProgram == true )
            {
                 elogDebug("flash addr x%X, Internal x%X", xExternalFlashAddr, xIntAddress);

                 //read next batch from external
                 FLASH_read(xExternalFlashAddr, (uint8_t*)&dataToWriteBuffer, STM_FLASH_PAGE_SIZE);
                 xExternalFlashAddr += STM_FLASH_PAGE_SIZE;

                 lowerWord = 0;
                 upperWord = 1;
                 nextWords = 0;

                //write 2 words at a time into internal flash
                for ( uint32_t i = 0; i< NUM_U32_PER_PAGE; i+=2 )
                {
                    //write the data to INTERNAL FLASH
                    //shift the upper and lower words around
                    //program 8 bytes at a time:
                    nextWords = (uint64_t) ((uint64_t)dataToWriteBuffer[upperWord] << 32 | dataToWriteBuffer[lowerWord]);
                    if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, xIntAddress, nextWords) == HAL_OK)
                    {
                        xIntAddress = xIntAddress + 8;

                        lowerWord+=2;
                        upperWord+=2;
                    }
                    else
                    {
                        /* Error occurred while writing data in Flash memory.*/
                        elogError("COULDNT PROGRAM FLASH");
                        successfulProgram = false;
                        break;
                    }
                }
            }

            //now zero pad the rest of the internal flash:
            elogInfo("Done programming image, now writing zeros to Internal Addr x%X", xIntAddress);
            nextWords = 0;

            while ( xIntAddress < INTERNAL_FLASH_END_ADDR && successfulProgram == true )
            {
                if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, xIntAddress, nextWords) == HAL_OK)
                {
                    //increment address
                    xIntAddress = xIntAddress + 8;
                }
                else
                {
                    /* Error occurred while writing data in Flash memory.*/
                    elogError("COULDNT PROGRAM FLASH");
                    successfulProgram = false;
                    break;
                }
            }

            //check at the end
            if ( successfulProgram == true )
            {
                elogInfo("Finished writing zeros to Internal Addr x%X", xIntAddress);
                elogInfo("Finished updating internal flash with image + zero padding");
            }
        }
    }

    /* Lock the Flash to disable the flash control register access (recommended
    to protect the FLASH memory against possible unwanted operation) */
    HAL_FLASH_Lock();

    return successfulProgram;
}

/**
  * @brief  Gets the page of a given address
  * @param  Addr: Address of the FLASH Memory
  * @retval The page of a given address
  */
static uint32_t xGetPage(uint32_t addressInternalFlash)
{
    uint32_t page = 0;

    if (addressInternalFlash < (FLASH_BASE + FLASH_BANK_SIZE))
    {
        /* Bank 1 */
        page = (addressInternalFlash - FLASH_BASE) / FLASH_PAGE_SIZE;
    }
    else
    {
        /* Bank 2 */
        page = (addressInternalFlash - (FLASH_BASE + FLASH_BANK_SIZE)) / FLASH_PAGE_SIZE;
    }

    return page;
}

/**
  * @brief  Gets the bank of a given address
  * @param  Addr: Address of the FLASH Memory
  * @retval The bank of a given address
  */
static uint32_t xGetBank(uint32_t addressInternalFlash)
{
    uint32_t bank = 0;

    if (READ_BIT(SYSCFG->MEMRMP, SYSCFG_MEMRMP_FB_MODE) == 0)
    {
        /* No Bank swap */
        if (addressInternalFlash < (FLASH_BASE + FLASH_BANK_SIZE))
        {
            bank = FLASH_BANK_1;
        }
        else
        {
            bank = FLASH_BANK_2;
        }
    }
    else
    {
        /* Bank swap */
        if (addressInternalFlash < (FLASH_BASE + FLASH_BANK_SIZE))
        {
          bank = FLASH_BANK_2;
        }
        else
        {
          bank = FLASH_BANK_1;
        }
  }

  return bank;
}
