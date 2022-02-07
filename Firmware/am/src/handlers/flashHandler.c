/*
================================================================================================#=
Module:   Flash Handler

Description:
    Flash Driver, interfaces with the NAND flash chip

Copyright 2021 Twisthink LLC
This code is licensed under Twisthink license.
See Twisthink-SoftwareLicenseAgreement-Auris.docx for details.
================================================================================================#=
*/

/* Includes */
#include <CLI.h>
#include <logTypes.h>
#include <stm32l4xx_hal.h>
#include "stdbool.h"
#include "string.h"
#include "stdlib.h"
#include <MT29F1.h>
#include "am-ssm-spi-protocol.h"
#include "APP_NVM_Cfg_Shared.h"
#include "memoryMap.h"
#include <flashHandler.h>

//bump this if there is a change to mem map in future versions
#define FLASH_VERSION           1

// private functions
static void xNandCommandHandlerFunction(int argc, char **argv);

//this is about 132kB....Holds an entire FLASH block
//which is the minimum erase size for this chip
uint8_t blockBuffer[BLOCK_SIZE];

void FLASH_init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* unlock the registers for writing, enable ECC */
    FlashUnlockAll();

    //set the WP pins high so that writes are not protected physically
    HAL_GPIO_WritePin(GPIOE, GPIO_PIN_9, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOE, GPIO_PIN_10, GPIO_PIN_SET);

    GPIO_InitStruct.Pin = GPIO_PIN_10 |GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

    /* register a command handler cb function */
    CLI_Command_Handler_s nandCmdHandler;
    nandCmdHandler.ptrFunction = &xNandCommandHandlerFunction;
    nandCmdHandler.cmdString   = "nand";
    nandCmdHandler.usageString = "\n\r\tpattern [ read | write ] - write or read pattern \n\r\tid \n\r\teraseb [block 0-1023] \n\r\terase [address] - erase addr's block \n\r\teraseall - erase everything";
    CLI_registerThisCommandHandler(&nandCmdHandler);
}

flashErr_t FLASH_write(uint32_t addr, uint8_t* data, uint32_t len)
{
    int i;
    flashErr_t flashErr = FLASH_GEN_ERROR;
    mt29f_status_t err;
    uint32_t index;
    uint32_t lenToWrite;
    uint32_t blockNum;
    uint32_t nextPageAddr = 0;
    uint32_t readableBlockAddr =0;
    uint32_t blockAddr;

    if (addr + (len - 1) > (MT29F1_MAX_ADDR) || (addr + len) < addr)
    {
        return FLASH_ADDR_ERR;
    }

    FlashUnlockAll();

    //put the new data into the block, only replacing LEN bytes
    do
     {
        // Get start address of block containing addr
        blockNum = ADDRESS_2_BLOCK(addr);

        // Walk through each page to READ the data
        for (i = 0; i < BLOCK_SIZE/PAGE_DATA_SIZE; i++)
        {
           //build the row address (block + page)
           Build_RowAddressNoCmd(blockNum, i, &nextPageAddr);

           //read out the next page into the buffer @ the next page index
           err = FlashPageRead( nextPageAddr, (blockBuffer + (i * PAGE_DATA_SIZE)) );

           // If we had a flash error, bail out of the for loop
           if (err != Flash_Success)
           {
               elogError("FLASH WRITE ERROR");
               break;
           }
        }

        // If we had a flash error, bail out.
        if (err == Flash_Success)
        {
            // Copy len to temp var
            lenToWrite = len;

            //build the row address (block + page)
            Build_RowAddressNoCmd(blockNum, 0, &blockAddr);

            //build the readable address of the block. This
            //will be the same format address that was passed in
            Build_Address(blockNum, 0, 0, &readableBlockAddr);

            // Set index within block where we want to copy the data
            index = addr - readableBlockAddr;

            // If we exceed a block, set length to write to end of block
            if ((index + len) > BLOCK_SIZE)
            {
                lenToWrite = BLOCK_SIZE - index;
            }

            // Copy data to blockBuffer to be written to Flash
            memcpy(blockBuffer + index, data, lenToWrite);

            // Increment data buffer
            data += lenToWrite;

            // Remaining amount of data left to write
            len -= lenToWrite;

            // Set address to where next data write would start
            addr += lenToWrite;

            // Erase the entire block first
            err = FlashBlockErase(blockAddr);

            if (err == Flash_Success)
            {
                // Walk through each page to replace the data
                for (i = 0; i <  BLOCK_SIZE/PAGE_DATA_SIZE; i++)
                {
                    //create the address (block + page)
                    Build_RowAddressNoCmd(blockNum, i, &nextPageAddr);

                    // write the next page
                    err = FlashPageProgram( nextPageAddr, blockBuffer + (i * PAGE_DATA_SIZE), PAGE_DATA_SIZE);

                    // If we had a flash error, bail out of the for loop
                    if (err != Flash_Success)
                    {
                        elogError("FLASH WRITE ERROR");
                        break;
                    }
                }
            }
        }
        else
        {
            elogError("FLASH WRITE ERROR");
            break;
        }
     } while (len > 0); //len will be > 0 if we need to move into the next block


    switch (err)
    {
        case Flash_Success:
           flashErr = FLASH_SUCCESS;
           break;
        case Flash_ProgramFailed:
           flashErr = FLASH_SPI_ERR;
           break;

        case Flash_AddressInvalid:
           flashErr = FLASH_ADDR_ERR;
           break;
        default:
         flashErr = FLASH_GEN_ERROR;
         break;
    }

    return flashErr;
}

flashErr_t FLASH_read(uint32_t addr, uint8_t *data, uint32_t len)
{
    mt29f_status_t err = Flash_NoInformationAvailable;
    flashErr_t flashErr;
    uint32_t pageNum = 0u;
    uint32_t index;
    uint32_t blockNum = 0;
    uint32_t address = 0;
    uint32_t readableAddr = 0;
    uint32_t offset = 0;
    int32_t tempLen = 0;
    uint32_t pagesRead = 0;


    if (addr + (len - 1) > (MT29F1_MAX_ADDR) || (addr + len) < addr)
      return FLASH_ADDR_ERR;

    FlashUnlockAll();

    tempLen = len;

    //get block and page addresses
    blockNum = ADDRESS_2_BLOCK(addr);
    pageNum = ADDRESS_2_PAGE(addr);

    //now convert to a ROW (block + page addr)
    Build_RowAddressNoCmd(blockNum, pageNum, &address);

    //convert to readable address (same format as passed in)
    Build_Address(blockNum, pageNum, 0, &readableAddr);

    // get index within page that we want copied into the
    //provided buffer
    index = addr - readableAddr;

    tempLen += index;

    do
    {
        //get block and page addresses
        blockNum = ADDRESS_2_BLOCK(addr);
        pageNum = ADDRESS_2_PAGE(addr);

        //now convert to a ROW (block + page addr)
        Build_RowAddressNoCmd(blockNum, pageNum, &address);

        //convert to readable address (same format as passed in)
        Build_Address(blockNum, pageNum, 0, &readableAddr);

        //just read one page
        if ( tempLen <= PAGE_DATA_SIZE )
        {
            pagesRead = 1;

            //create the address (block + page)
            Build_RowAddressNoCmd(blockNum, pageNum, &address);

            // write the next page
            err = FlashPageRead( address, blockBuffer+offset);

            // If we had a flash error, bail out of the for loop
            if (err != Flash_Success)
            {
               elogError("FLASH WRITE ERROR");
               break;
            }
        }
        else
        {
            //how many pages to read
            if ( (tempLen ) % PAGE_DATA_SIZE == 0)
            {
                pagesRead = (tempLen/PAGE_DATA_SIZE);
            }
            else
            {
                pagesRead = (tempLen/PAGE_DATA_SIZE + 1);
            }

            if ( pagesRead + pageNum > NUM_PAGE_BLOCK)
            {
                //split up because we will overflow the block....
                pagesRead = NUM_PAGE_BLOCK - pageNum;
            }

            // Walk through each page and read out the contents
             for (int i = 0; i <  pagesRead; i++)
             {
                 //create the address (block + page)
                 Build_RowAddressNoCmd(blockNum, pageNum+i, &address);

                 // write the next page
                 err = FlashPageRead( address, blockBuffer + (i * PAGE_DATA_SIZE)+offset);

                 // If we had a flash error, bail out of the for loop
                 if (err != Flash_Success)
                 {
                    elogError("FLASH WRITE ERROR");
                    break;
                 }
             }
        }

        tempLen -= PAGE_DATA_SIZE*pagesRead;

        // Set address to where next data read would start
        addr += PAGE_DATA_SIZE*pagesRead;

        //set index in the block buffer where we would copy next read data
        offset+= PAGE_DATA_SIZE*pagesRead;

    }while (tempLen > 0);

    if (err == Flash_Success)
    {
        // Copy the data into the buffer at the index we found earlier
        memcpy(data, blockBuffer + index, len);
    }

    switch (err)
    {
        case Flash_Success:
            flashErr = FLASH_SUCCESS;
            break;
        case Flash_ProgramFailed:
            flashErr = FLASH_SPI_ERR;
            break;
        case Flash_AddressInvalid:
            flashErr = FLASH_ADDR_ERR;
            break;
         default:
             flashErr = FLASH_GEN_ERROR;
             break;
    }

    if (flashErr)
    {
      elogError("Failed to read");
    }

    return flashErr;
}

static void xNandCommandHandlerFunction(int argc, char **argv)
{
    uint16_t id;
    uint32_t addr = 0;
    uint32_t block = 0;
    flashErr_t flashStat = FLASH_GEN_ERROR;
    mt29f_status_t err;
    uint8_t testPatternWrite[4] = {0xAA, 0xBB, 0xAB, 0xBA};
    uint8_t testPatternRead[4] = {};
    //pick address outside of range of flash we are using
    uint32_t address = 0x0040001F;

    /* process the user input */
    if ( (argc == TWO_ARGUMENTS) &&  (0 == strcmp(argv[FIRST_ARG_IDX], "pattern")) )
    {
        if (strcmp(argv[SECOND_ARG_IDX], "read") == 0)
        {
            //read pattern stored at the test flash address
            flashStat = FLASH_read(address, (uint8_t*)&testPatternRead, 4);

            if ( memcmp(&testPatternRead, &testPatternWrite, 4) == 0 && flashStat == FLASH_SUCCESS )
            {
                elogInfo("Pass");
            }
            else
            {
                elogError("FAIL");
            }
        }
        else if (strcmp(argv[SECOND_ARG_IDX], "write") == 0)
        {
            //write
            flashStat = FLASH_write(address, (uint8_t*)&testPatternWrite, 4);

            if ( flashStat == FLASH_SUCCESS )
            {
                elogInfo("Done writing pattern");
            }
            else
            {
                elogError("Error writing pattern");
            }
        }
        else
        {
            elogInfo("Invaid args");
        }
    }
    else if ( (argc == ONE_ARGUMENT) &&  (0 == strcmp(argv[FIRST_ARG_IDX], "id")) )
    {
        FlashReadDeviceIdentification(&id);
        elogInfo("NAND device ID x%x", id);
    }
    else if ( (argc == TWO_ARGUMENTS) &&  (0 == strcmp(argv[FIRST_ARG_IDX], "eraseb")) )
    {
        block = strtoul(argv[SECOND_ARG_IDX], NULL, 10);

        if ( block < NUM_BLOCKS )
        {
            Build_RowAddressNoCmd(block, 0 , &addr);
            err = FlashBlockErase(addr);

            if ( err == Flash_Success)
                elogInfo("ERASE Block %lu - DONE", block);
            else
                elogError("Erase block %ul FAILED", block);
        }
        else
        {
            elogError("ERASE Block %lu - INVALID block number", addr);
        }
    }
    else if ( (argc == TWO_ARGUMENTS) &&  (0 == strcmp(argv[FIRST_ARG_IDX], "erase")) )
    {
        addr = strtoul(argv[SECOND_ARG_IDX], NULL, 10);

        block = ADDRESS_2_BLOCK(addr);

        if ( block < NUM_BLOCKS )
        {
            Build_RowAddressNoCmd(block, 0 , &addr);
            err = FlashBlockErase(addr);

            if ( err == Flash_Success)
                elogInfo("ERASE Block %lu - DONE", block);
            else
                elogError("Erase block %ul FAILED", block);
        }
        else
        {
            elogError("ERASE Block %lu - INVALID block number", addr);
        }
    }
    else if ( (argc == ONE_ARGUMENT) &&  (0 == strcmp(argv[FIRST_ARG_IDX], "eraseall")) )
    {
        for ( block = 0; block< NUM_BLOCKS; block++)
        {
            Build_RowAddressNoCmd(block, 0 , &addr);
            err = FlashBlockErase(addr);

            if ( err != Flash_Success)
                elogError("Erase block %ul FAILED", block);
        }

        elogInfo("Finished Erasing FLASH");
    }
    else if ( (argc == ONE_ARGUMENT) &&  (0 == strcmp(argv[FIRST_ARG_IDX], "reset")) )
    {
        FlashReset();
        elogInfo("RESET FLASH");
    }
    else
    {
        elogInfo("Invaid args");
    }
}


