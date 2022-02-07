/*
================================================================================================#=
Module:   OTA Update

Description:
    Handle OTA update downloads and store to FLASH, update image registry accordingly for the bootloader

Copyright 2021 Twisthink LLC
This code is licensed under Twisthink license.
See Twisthink-SoftwareLicenseAgreement-Auris.docx for details.
================================================================================================#=
*/

#ifndef HANDLERS_OTAUPDATE_C_
#define HANDLERS_OTAUPDATE_C_

#include "stdint.h"
#include "string.h"
#include "stdbool.h"
#include "lwip/apps/http_client.h"
#include "logTypes.h"
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"
#include "memoryMap.h"
#include "memMapHandler.h"
#include "flashHandler.h"
#include "MT29F1.h"
#include "eventManager.h"
#include "otaUpdate.h"

#define MAX_CHARS_FROM_OTA_TO_STORE_IN_RAM      PAGE_DATA_SIZE*40

typedef enum
{
    FIRST_PACKET,
    DOWNLOADING_AM_RECORD,
    WAITING_ON_SSM_HEADER,
    DOWNLOADING_SSM_RECORD,
}otaDownloadState_t;

static otaDownloadState_t downloadState = FIRST_PACKET;

//track ota download progress
static uint32_t packetCnt = 0;
static uint32_t totalIncomingBytes = 0;
static uint32_t fileSize = 0;

//these are found within the file that is downloaded
static uint32_t amRecordLength = 0xFFFFFF;
static uint32_t ssmRecordLength = 0u;


static uint32_t downloadedImageFwMaj = 0;
static uint32_t downloadedImageFwMin = 0;
static uint32_t downloadedImageFwBuild = 0;

//track where to store into flash
//TODO change this to the correct SLOT (either A or B)
static uint32_t nextAddrToStoreImage = 0;
static uint32_t amImageStartAddr = 0;
static uint32_t ssmImageStartAddr = 0;


//store 40 pages to minimize flash writes..
static uint8_t nextFlashWriteBuffer[MAX_CHARS_FROM_OTA_TO_STORE_IN_RAM] = {0};
static uint32_t writeBufferLen = 0u;

static uint8_t tempBufferForAmSsmCrossover[2000];
static uint16_t tempBufferIdx = 0;
static uint32_t amBytesInPacket = 0;
static uint32_t ssmBytesInPacket = 0;
static uint16_t crc = 0xFFFF;

//s3 bucket that contains the ota package
static char fileLocationUrl[MAX_OTA_FILEPATH_LEN_BYTES] = {};
static char fileLocationDomainName[MAX_OTA_FILEPATH_LEN_BYTES] = {};

TaskHandle_t otaDownloadHandle;

//callback functions for LWIP
void HttpClientResultCallback (void *arg, httpc_result_t httpc_result, u32_t
                                    rx_content_len, u32_t srv_res, err_t err);

err_t RecvImageBytesCallback(void *arg, struct tcp_pcb *tpcb, struct pbuf *p,
                                        err_t err) ;

err_t RecvHttpHeaderCallback (httpc_state_t *connection, void *arg, struct
                                    pbuf *hdr, u16_t hdr_len, u32_t content_len);


static uint32_t checkWhichAddrToStoreSsmImage(void);
static uint32_t checkWhichAddrToStoreAmImage(void);
static void downloadFinishedUpdateRegistry(void);
static uint16_t xRunningCrc(const uint8_t* data_p, uint32_t length);

//pass in the S3 file link contained in the AWS job to init the download
bool OTA_initDownload(char * filePath)
{
    bool res = false;

    //copy the full filepath into the URL
    strncpy(fileLocationUrl, filePath, strlen(filePath));

    //we dont want the file name in the domain name, so need to
    //remove everything AFTER .com
    char *fileStartAddr = strstr(filePath, ".com");

    // get pointer to end of string to be removed
    char *fileEndAddr = fileStartAddr + strlen(".com");

    //length of just the domain
    uint8_t len = strlen (filePath) - strlen(fileEndAddr);

    //now copy just the domain into the buffer using the computed
    //length
    strncpy(fileLocationDomainName, fileLocationUrl, len);

    elogInfo("url:\n%s\n", fileLocationUrl);
    elogInfo("domain:\n%s\n", fileLocationDomainName);

    //figure out where the image will be stored
    ssmImageStartAddr = checkWhichAddrToStoreSsmImage();
    amImageStartAddr = checkWhichAddrToStoreAmImage();

    //we start saving the image at the am slot
    nextAddrToStoreImage = amImageStartAddr;

    if( xTaskCreate( OTA_downloadTask, "downloadThread", ( configSTACK_DEPTH_TYPE ) 768*12, NULL, 7, &otaDownloadHandle ) != pdPASS )
    {
       elogError("Failed to create task");
    }
    else
    {
        elogDebug("Created ota task");
        res = true;
    }

    return res;
}


//set up the URL of the s3 bucket and begin the file download
void OTA_downloadTask()
{
    httpc_connection_t conn_settings;
    httpc_state_t *connection;
    err_t error;

    //set up the callback functions
    conn_settings.use_proxy = 0;
    conn_settings.headers_done_fn = RecvHttpHeaderCallback;
    conn_settings.result_fn = HttpClientResultCallback;

    //init the file download & assign a callback for bytes received
    error = httpc_get_file_dns(fileLocationDomainName, 80, fileLocationUrl, &conn_settings,
    RecvImageBytesCallback, NULL, &connection);

    elogInfo("DNS lookup result %d\n", error);

    //this will get deleted once we finish downloading the image
    while(1)
    {

    }
}


static uint32_t checkWhichAddrToStoreAmImage(void)
{
    uint32_t addr;

    //store the new image into the slot that does NOT contain the current loaded image
    //so that the BL can fall back if there is an issue with this new image
    if ( MEM_getLoadedImage() == A )
    {
        addr = APP_MEM_ADR_FW_APPLICATION_AM_B_START;
    }
    else
    {
        addr = APP_MEM_ADR_FW_APPLICATION_AM_A_START;
    }

    elogDebug("store am image to 0x%X", addr);

    return addr;
}

static uint32_t checkWhichAddrToStoreSsmImage(void)
{
    uint32_t addr;

    //store the new image into the slot that does NOT contain the current loaded image
    //so that the BL can fall back if there is an issue with this new image
    if ( MEM_getLoadedImage() == A )
    {
        addr = APP_MEM_ADR_FW_APPLICATION_SSM_B_START;
    }
    else
    {
        addr = APP_MEM_ADR_FW_APPLICATION_SSM_A_START;
    }

    elogDebug("store ssm image to 0x%X", addr);

    return addr;
}


static void downloadFinishedUpdateRegistry(void)
{
    uint32_t tempLen;
    uint32_t addr;
    imageSlotTypes_t currentLoadedSlot = MEM_getLoadedImage();
    uint16_t calculatedAmCrc = 0u;
    uint16_t storedAmCrc = 0u;
    uint16_t calculatedSsmCrc = 0u;
    uint16_t storedSsmCrc = 0u;
    bool valid = false;

    //get address where we loaded the image (will be in the slot other than the loaded slot)
    if ( currentLoadedSlot == A)
    {
        tempLen = amRecordLength - CRC_LEN;
        addr = APP_MEM_ADR_FW_APPLICATION_AM_B_START + CRC_LEN;
    }
    else
    {
        tempLen = amRecordLength - CRC_LEN;
        addr = APP_MEM_ADR_FW_APPLICATION_AM_A_START + CRC_LEN;
    }

    //compute AM image checksum
    while (tempLen)
    {
        if(tempLen < MAX_CHARS_FROM_OTA_TO_STORE_IN_RAM)
        {
            FLASH_read(addr, (uint8_t*) &nextFlashWriteBuffer, tempLen);

            calculatedAmCrc = xRunningCrc((uint8_t*)&nextFlashWriteBuffer, tempLen);

            tempLen = 0;
        }
        else
        {
            FLASH_read(addr, (uint8_t*)&nextFlashWriteBuffer, MAX_CHARS_FROM_OTA_TO_STORE_IN_RAM);

            calculatedAmCrc = xRunningCrc((uint8_t*)&nextFlashWriteBuffer, MAX_CHARS_FROM_OTA_TO_STORE_IN_RAM);
            tempLen-=MAX_CHARS_FROM_OTA_TO_STORE_IN_RAM;
            addr+= MAX_CHARS_FROM_OTA_TO_STORE_IN_RAM;
        }
    }

    //now do the ssm crc
    //get address where we loaded the image (will be in the slot other than the loaded slot)
    if ( currentLoadedSlot == A)
    {
        tempLen = ssmRecordLength - CRC_LEN;
        addr = APP_MEM_ADR_FW_APPLICATION_SSM_B_START + CRC_LEN;
    }
    else
    {
        tempLen = ssmRecordLength - CRC_LEN;
        addr = APP_MEM_ADR_FW_APPLICATION_SSM_A_START + CRC_LEN;
    }

    //reset crc value used in calculation
    crc = 0xFFFF;

    //compute SSM image checksum
    while (tempLen)
    {
        if(tempLen < MAX_CHARS_FROM_OTA_TO_STORE_IN_RAM)
        {
            FLASH_read(addr, (uint8_t*) &nextFlashWriteBuffer, tempLen);

            calculatedSsmCrc = xRunningCrc((uint8_t*)&nextFlashWriteBuffer, tempLen);

            tempLen = 0;
        }
        else
        {
            FLASH_read(addr, (uint8_t*)&nextFlashWriteBuffer, MAX_CHARS_FROM_OTA_TO_STORE_IN_RAM);

            calculatedSsmCrc = xRunningCrc((uint8_t*)&nextFlashWriteBuffer, MAX_CHARS_FROM_OTA_TO_STORE_IN_RAM);
            tempLen-=MAX_CHARS_FROM_OTA_TO_STORE_IN_RAM;
            addr+= MAX_CHARS_FROM_OTA_TO_STORE_IN_RAM;
        }
    }

    if ( currentLoadedSlot == A )
    {
        FLASH_read(APP_MEM_ADR_FW_APPLICATION_AM_B_START, (uint8_t*)&storedAmCrc, CRC_LEN);
        FLASH_read(APP_MEM_ADR_FW_APPLICATION_SSM_B_START, (uint8_t*)&storedSsmCrc, CRC_LEN);

        //shift bytes since they are read out of flash backwards:
        storedAmCrc = (uint8_t)storedAmCrc<<8 | storedAmCrc>>8;
        storedSsmCrc = (uint8_t)storedSsmCrc<<8 | storedSsmCrc>>8;

        if ( (calculatedAmCrc == storedAmCrc) && (calculatedSsmCrc == storedSsmCrc) )
        {
            valid = true;

            MEM_setImageBoperationalState(OP_UNKNOWN);

            //set the fw version
            MEM_setImageBversion(downloadedImageFwMaj, downloadedImageFwMin, downloadedImageFwBuild);
            MEM_setPrimaryImage(B);

            elogDebug("Set image B to the primary image");
        }
        else
        {
            elogError("Bad checksum AM: 0x%X, 0x%X SSM: 0x%X, 0x%X", calculatedAmCrc, storedAmCrc, calculatedSsmCrc, storedSsmCrc);

            //overwrite the metadata section in FLASH with invalid values for this image
            memset(&nextFlashWriteBuffer, 0xFF, MAX_CHARS_FROM_OTA_TO_STORE_IN_RAM);

            FLASH_write(APP_MEM_ADR_FW_APPLICATION_AM_B_START, (uint8_t*)nextFlashWriteBuffer, MAX_CHARS_FROM_OTA_TO_STORE_IN_RAM);
        }
    }
    else
    {
        FLASH_read(APP_MEM_ADR_FW_APPLICATION_AM_A_START, (uint8_t*)&storedAmCrc, CRC_LEN);
        FLASH_read(APP_MEM_ADR_FW_APPLICATION_SSM_A_START, (uint8_t*)&storedSsmCrc, CRC_LEN);

        //shift bytes since they are read out of flash backwards:
        storedAmCrc = (uint8_t)storedAmCrc<<8 | storedAmCrc>>8;
        storedSsmCrc = (uint8_t)storedSsmCrc<<8 | storedSsmCrc>>8;

        if ( (calculatedAmCrc == storedAmCrc) && (calculatedSsmCrc == storedSsmCrc) )
        {
            valid = true;
            MEM_setImageAoperationalState(OP_UNKNOWN);

            //set new fw version
            MEM_setImageAversion(downloadedImageFwMaj, downloadedImageFwMin, downloadedImageFwBuild);
            MEM_setPrimaryImage(A);

            elogDebug("Set image A to the primary image");
        }
        else
        {
            elogError("Bad checksum AM: 0x%X, 0x%X SSM: 0x%X, 0x%X", calculatedAmCrc, storedAmCrc, calculatedSsmCrc, storedSsmCrc);

            //overwrite the metadata section in FLASH with invalid values for this image
            memset(&nextFlashWriteBuffer, 0xFF, MAX_CHARS_FROM_OTA_TO_STORE_IN_RAM);

            FLASH_write(APP_MEM_ADR_FW_APPLICATION_AM_A_START, (uint8_t*)nextFlashWriteBuffer, MAX_CHARS_FROM_OTA_TO_STORE_IN_RAM);
        }
    }

    if ( valid == true )
    {
        //pass up to the event manager that we are done
        EVT_indicateFwDownloadComplete();
    }
    else
    {
        EVT_indicateFwDownloadFail();
    }
}

//header received - this will contain the file size of the download
err_t RecvHttpHeaderCallback (httpc_state_t *connection, void *arg, struct
                                pbuf *hdr, u16_t hdr_len, u32_t content_len)
{
    elogInfo("HEADER RECEIVED OTA Filesize =  %lu bytes", content_len);

    //set the file size - this value is contained in the header
    fileSize = content_len;

    return ERR_OK;
}


//this is called DURING the download process when there is an error, or when the
//download has completed without errors
void HttpClientResultCallback (void *arg, httpc_result_t httpc_result, u32_t
                                    rx_content_len, u32_t srv_res, err_t err)
{
   elogInfo("CLIENT RESULT - httpc_result: %u\n", httpc_result);
   elogInfo("received number of bytes: %lu\n", rx_content_len);
   elogInfo("Deleting OTA Download Task");

   //now destroy the task!
   vTaskDelete(otaDownloadHandle);
}

//The tcp packets come in chains, so typically there are 3 of them chained together to operate on
//in this function
//the p->tot_len variable is the total # bytes between all 3 chains
err_t RecvImageBytesCallback(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
    uint16_t thisPacketLen = 0;

    if (p == NULL || p->tot_len > PAGE_DATA_SIZE)
    {
        elogError("invalid TCP packet has arrived");
    }
    else
    {

        //if we are about to hit the max stored chars, write them all to FLASH and reset the write buffer idx
        if( p->tot_len + writeBufferLen >= MAX_CHARS_FROM_OTA_TO_STORE_IN_RAM)
        {
            FLASH_write(nextAddrToStoreImage, (uint8_t*)&nextFlashWriteBuffer, writeBufferLen);
            nextAddrToStoreImage += writeBufferLen;
            writeBufferLen = 0;
        }

        //if we are simply downloading the AM image before the ssm-am overlap or downloading the ssm record,
        //throw the payloads intp the buffer that we will write to flash:
        if ( (totalIncomingBytes + p->tot_len <= (amRecordLength + RECORD_HEADER_LEN) && downloadState != FIRST_PACKET && downloadState != WAITING_ON_SSM_HEADER )
                || downloadState == DOWNLOADING_SSM_RECORD )
        {
            //copy the payload into a buffer so its easier to operate on:
            memcpy(&nextFlashWriteBuffer[writeBufferLen], p->payload, p->len);
            writeBufferLen += p->len;

            //second chain
            if(p->next != NULL )
            {

                memcpy(&nextFlashWriteBuffer[writeBufferLen], p->next->payload, p->next->len);

                writeBufferLen += p->next->len;

                //final chain
                if ( p->next->next != NULL )
                {
                    //another copy...
                    memcpy(&nextFlashWriteBuffer[writeBufferLen], p->next->next->payload, p->next->next->len);
                    writeBufferLen += p->next->next->len;
                }
            }
        }

        thisPacketLen = p->tot_len;

        //process the bytes according to which state we are in:
        switch(downloadState)
        {

            case FIRST_PACKET:

                //copy the payload into a TEMP buffer so its easier to operate on:
                memcpy(&tempBufferForAmSsmCrossover, p->payload, p->len);
                tempBufferIdx += p->len;

                //second chain
                if(p->next != NULL )
                {

                  memcpy(&tempBufferForAmSsmCrossover[tempBufferIdx], p->next->payload, p->next->len);

                  tempBufferIdx += p->next->len;

                  //final chain
                  if ( p->next->next != NULL )
                  {
                      //another copy...
                      memcpy(&tempBufferForAmSsmCrossover[tempBufferIdx], p->next->next->payload, p->next->next->len);
                      tempBufferIdx += p->next->next->len;
                  }
                }

                //check first record type - either AM or SSM
                if ( tempBufferForAmSsmCrossover[RECORD_TYPE_IDX] == AM_IMAGE)
                {
                    //this is an AM record and follows the OTA package format so we can continue to parse it

                    //record length - cs, metadata, and binary
                    amRecordLength = ((uint32_t)tempBufferForAmSsmCrossover[RECORD_LEN_IDX] <<24) | ((uint32_t)tempBufferForAmSsmCrossover[RECORD_LEN_IDX+1] <<16)
                            | ((uint32_t)tempBufferForAmSsmCrossover[RECORD_LEN_IDX+2] << 8) | ((uint32_t)tempBufferForAmSsmCrossover[RECORD_LEN_IDX+3]);

                    //also set the fw verson:
                    downloadedImageFwMaj = ((uint32_t)tempBufferForAmSsmCrossover[AM_FW_VERSION_START_IDX] <<24) | ((uint32_t)tempBufferForAmSsmCrossover[AM_FW_VERSION_START_IDX+1] <<16)
                                    | ((uint32_t)tempBufferForAmSsmCrossover[AM_FW_VERSION_START_IDX+2] << 8) | ((uint32_t)tempBufferForAmSsmCrossover[AM_FW_VERSION_START_IDX+3]);

                    downloadedImageFwMin = ((uint32_t)tempBufferForAmSsmCrossover[AM_FW_VERSION_START_IDX+4] <<24) | ((uint32_t)tempBufferForAmSsmCrossover[AM_FW_VERSION_START_IDX+5] <<16)
                                    | ((uint32_t)tempBufferForAmSsmCrossover[AM_FW_VERSION_START_IDX+6] << 8) | ((uint32_t)tempBufferForAmSsmCrossover[AM_FW_VERSION_START_IDX+7]);


                    downloadedImageFwBuild = ((uint32_t)tempBufferForAmSsmCrossover[AM_FW_VERSION_START_IDX+8] <<24) | ((uint32_t)tempBufferForAmSsmCrossover[AM_FW_VERSION_START_IDX+9] <<16)
                                    | ((uint32_t)tempBufferForAmSsmCrossover[AM_FW_VERSION_START_IDX+10] << 8) | ((uint32_t)tempBufferForAmSsmCrossover[AM_FW_VERSION_START_IDX+11]);


                    memcpy(&nextFlashWriteBuffer, &tempBufferForAmSsmCrossover[RECORD_HEADER_LEN], tempBufferIdx-RECORD_HEADER_LEN);

                    writeBufferLen+= tempBufferIdx - RECORD_HEADER_LEN;
                    tempBufferIdx = 0;

                    //bump up the state
                    downloadState = DOWNLOADING_AM_RECORD;

                }
                else
                {
                    //this packet does NOT follow the OTA package format - stop receiving the file
                    tcp_recved(tpcb, p->tot_len);
                    pbuf_free(p);

                    //BAIL
                    EVT_indicateFwDownloadFail();

                    return tcp_close(tpcb);

                }
                break;
            case DOWNLOADING_AM_RECORD:

                //check if we are about to cross over into the SSM record section, we will handle differently if so
                if ( totalIncomingBytes + p->tot_len > amRecordLength + RECORD_HEADER_LEN)
                {
                    //now calculate the makeup of the packet - ssm vs am line - subtract 5 from incoming bytes to account for the record type + len = 5
                    amBytesInPacket = p->tot_len - (totalIncomingBytes - RECORD_HEADER_LEN + p->tot_len - amRecordLength);
                    ssmBytesInPacket = p->tot_len - amBytesInPacket;

                    //copy the payload into a TEMPORARY buffer so its easier to operate on:
                    memcpy(&tempBufferForAmSsmCrossover, p->payload, p->len);
                    tempBufferIdx += p->len;

                    //second chain
                    if(p->next != NULL )
                    {
                       memcpy(&tempBufferForAmSsmCrossover[tempBufferIdx], p->next->payload, p->next->len);

                       tempBufferIdx += p->next->len;

                       //final chain
                       if ( p->next->next != NULL )
                       {
                           //another copy...
                           memcpy(&tempBufferForAmSsmCrossover[tempBufferIdx], p->next->next->payload, p->next->next->len);
                           tempBufferIdx += p->next->next->len;
                       }
                    }

                    //put the AM byes into the actual buffer
                    memcpy(&nextFlashWriteBuffer[writeBufferLen], &tempBufferForAmSsmCrossover, amBytesInPacket);

                    writeBufferLen += amBytesInPacket;

                    //write the current buffer to flash
                    FLASH_write(nextAddrToStoreImage, (uint8_t*)&nextFlashWriteBuffer, writeBufferLen);

                    //reset variables
                    writeBufferLen = 0;

                    //now parse out the ssm bytes, which are located at nextFlashWriteBuffer[amBytesInPacket] index
                    //and have a length of ssmBytesInPacket

                    if ( ssmBytesInPacket >= RECORD_HEADER_LEN )
                    {
                        if ( tempBufferForAmSsmCrossover[amBytesInPacket] == SSM_IMAGE)
                        {
                            //this is an SSM record and follows the OTA package format so we can continue to parse it

                            //record length - cs, metadata, and binary
                            ssmRecordLength = ((uint32_t)tempBufferForAmSsmCrossover[RECORD_LEN_IDX + amBytesInPacket] <<24) | ((uint32_t)tempBufferForAmSsmCrossover[RECORD_LEN_IDX+1+amBytesInPacket] <<16)
                                                       | ((uint32_t)tempBufferForAmSsmCrossover[RECORD_LEN_IDX+2+amBytesInPacket] << 8) | ((uint32_t)tempBufferForAmSsmCrossover[RECORD_LEN_IDX+3+amBytesInPacket]);

                            //now start saving at the SSM slot
                            nextAddrToStoreImage = ssmImageStartAddr;

                            memcpy(&nextFlashWriteBuffer, &tempBufferForAmSsmCrossover[amBytesInPacket + RECORD_HEADER_LEN], ssmBytesInPacket - RECORD_HEADER_LEN);
                            writeBufferLen += (ssmBytesInPacket - RECORD_HEADER_LEN);

                            downloadState = DOWNLOADING_SSM_RECORD;
                        }
                        else
                        {

                            //this packet does NOT follow the OTA package format - stop receiving the file
                            tcp_recved(tpcb, p->tot_len);
                            pbuf_free(p);

                            //bail
                            EVT_indicateFwDownloadFail();
                            return tcp_close(tpcb);
                        }
                    }
                    else
                    {
                        //wait for the next chunk and then process this edge case
                        downloadState = WAITING_ON_SSM_HEADER;
                    }
                }

                break;

            case WAITING_ON_SSM_HEADER:

                //copy the next X ssm packet bytes into the temporary buffer...
                memcpy(&tempBufferForAmSsmCrossover[tempBufferIdx], p->next->payload, (RECORD_HEADER_LEN - ssmBytesInPacket));

                ssmBytesInPacket += (RECORD_HEADER_LEN - ssmBytesInPacket);

               if ( tempBufferForAmSsmCrossover[amBytesInPacket] == SSM_IMAGE)
               {
                   //this is an SSM record and follows the OTA package format so we can continue to parse it

                   //record length - cs, metadata, and binary
                   ssmRecordLength = ((uint32_t)tempBufferForAmSsmCrossover[RECORD_LEN_IDX + amBytesInPacket] <<24) | ((uint32_t)tempBufferForAmSsmCrossover[RECORD_LEN_IDX+1+amBytesInPacket] <<16)
                                              | ((uint32_t)tempBufferForAmSsmCrossover[RECORD_LEN_IDX+2+amBytesInPacket] << 8) | ((uint32_t)tempBufferForAmSsmCrossover[RECORD_LEN_IDX+3+amBytesInPacket]);

                   //now start saving at the SSM slot
                   nextAddrToStoreImage = ssmImageStartAddr;
               }
               else
               {

                   //this packet does NOT follow the OTA package format - stop receiving the file
                   tcp_recved(tpcb, p->tot_len);
                   pbuf_free(p);

                   //bail
                   EVT_indicateFwDownloadFail();

                   return tcp_close(tpcb);
               }

                p->payload += (RECORD_HEADER_LEN - ssmBytesInPacket);

                //copy the payload into a buffer so its easier to operate on:
                memcpy(&nextFlashWriteBuffer[writeBufferLen], p->payload, p->len - (RECORD_HEADER_LEN - ssmBytesInPacket));
                writeBufferLen += p->len - (RECORD_HEADER_LEN - ssmBytesInPacket);

                //second chain

                if(p->next != NULL )
                {

                    memcpy(&nextFlashWriteBuffer[writeBufferLen], p->next->payload, p->next->len);

                    writeBufferLen += p->next->len;

                    //final chain
                    if ( p->next->next != NULL )
                    {
                        //another copy...
                        memcpy(&nextFlashWriteBuffer[writeBufferLen], p->next->next->payload, p->next->next->len);
                        writeBufferLen += p->next->next->len;
                    }
                }

                downloadState = DOWNLOADING_SSM_RECORD;

                break;
            default:
                break;
        }

        tcp_recved(tpcb, p->tot_len);
        pbuf_free(p);

        totalIncomingBytes += thisPacketLen;
        packetCnt++;

        elogDebug("incoming bytes (%lu)\n",totalIncomingBytes);

        if (totalIncomingBytes >= fileSize)
        {
           elogInfo("incoming bytes (%lu) >= filesize (%lu)\n", totalIncomingBytes, fileSize);
           elogInfo("Last packet received -> closing of the tcp connection has been initiated");
           totalIncomingBytes = 0;
           packetCnt = 0;

           //do a final write with the remaining characters
           FLASH_write(nextAddrToStoreImage, (uint8_t*)&nextFlashWriteBuffer, writeBufferLen);

           writeBufferLen = 0;

           //finally, update the image registry so that on the next PC, the BL will
           //see that we have a new primary image (the one we just downloaded)
           downloadFinishedUpdateRegistry();

           //close off the connection
           return tcp_close(tpcb);
        }
    }

    return ERR_OK;
}


/*
     CRC-16 Attributes:
    Name                 |   Polynomial | Reversed? |  Init-value | XOR-out Check
    crc-ccitt-false [1]  |   0x11021    | False     | 0xFFFF      | 0x0000  0x29B1
 */
static uint16_t xRunningCrc(const uint8_t* data_p, uint32_t length)
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


#endif /* HANDLERS_OTAUPDATE_C_ */
