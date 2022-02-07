/**************************************************************************************************
* \file     mspBslProtocol.c
* \brief    API to communicate with an MSP430 BSL FRAM bootloader
*           Assumes we are doing a fw update and focusing primarily on this, uses blocking uart calls
*           to reduce complexity
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

#ifndef DEVICE_DRIVERS_MSPBSLPROTOCOL_C_
#define DEVICE_DRIVERS_MSPBSLPROTOCOL_C_

#include "stdint.h"
#include "logTypes.h"
#include "FreeRTOS.h"
#include "string.h"
#include "uart.h"
#include "task.h"
#include "mspBslProtocol.h"


//BSL protocol  definitions:
#define RX_DATA_BLOCK_RESP_CMD  0x3B
#define RX_DATA_BLOCK_RESP_NL   0x02
#define RX_DATA_BLOCK_RESP_NH   0x00

#define PASSWORD_LENGTH         32
#define RX_PASSWORD_NL          0x21
#define RX_PASSWORD_NH          0x00
#define RX_PASSWORD_RESP_CMD    0x3B
#define RX_PASSWORD_RESP_NL     0x02
#define RX_PASSWORD_RESP_NH     0x00
#define CRC_CHECK_LEN           0x06
#define CRC_CHECK_RSP_LEN       0x00

#define MASS_ERASE_NL           0x01
#define MASS_ERASE_NH           0x00
#define MASS_ERASE_RESP_CMD     0x3B
#define MASS_ERASE_RESP_NL      0x02
#define MASS_ERASE_RESP_NH      0x00

#define LOAD_PC_NL              0x04
#define LOAD_PC_NH              0x00

#define TX_DATA_BLOCK_NL        0x06
#define TX_DATA_BLOCK_NH        0x00
#define TX_DATA_RESP_CMD        0x3A

#define RX_PASSWORD             0x11
#define TX_DATA_BLOCK           0x18
#define RX_DATA_BLOCK           0x10
#define MASS_ERASE              0x15
#define LOAD_PC                 0x17
#define CRC_CHECK               0x16

#define HEADER                  0x80

#define CS_START_IDX            3
#define CS_LEN                  2
#define PW_RESPONSE_LEN         8
#define MEM_RESPONSE_LEN        8
#define CRC_RESPONSE_LEN        9
#define READ_RESPONSE_LEN       7


#define ACK                     0x00
#define HEADER_INCORRECT        0x51
#define CHECKSUM_INCORRECT      0x52
#define PACKET_SIZE_ZERO        0x53
#define PACKET_SIZE_EXCEEDS     0x54
#define UNKNOWN_ERROR           0x55
#define UNKNOWN_BAUDRATE        0x56
#define PACKET_SIZE_ERROR       0x57

#define GetCKL(cs)          (uint8_t)(cs)
#define GetCKH(cs)          (uint8_t)(cs >> 8)

#define MAX_CHUNK_SIZE   254
#define MAX_MSG_SIZE     MAX_CHUNK_SIZE + 45


static uint8_t sendBuffer[MAX_MSG_SIZE] = {0};
static uint8_t receiveBuffer[MAX_MSG_SIZE] = {0};

static void sendAndReceiveOverUart(uint8_t *pSendData, uint16_t bytesToSend, uint8_t *pRxData, uint16_t bytesToRx);

//*****************************************************************************
// Write The Default Password *************************************************
// Uses the write password function to write the default password (0xFF) ******
//*****************************************************************************

bool BSL_writePasswordDefault(void)
{
    uint8_t password[PASSWORD_LENGTH] = {0};
    uint16_t loopIndex = 0;

    for (loopIndex = 0; loopIndex < PASSWORD_LENGTH; loopIndex++)
    {
        password[loopIndex] = 0xFF;
    }

    return BSL_writePassword(password, PASSWORD_LENGTH);
}


//*****************************************************************************
// Write the BSL password *****************************************************
// password: The password for BSL *********************************************
// passwordSize: The size of the password array *******************************
//*****************************************************************************

bool BSL_writePassword(uint8_t* password, uint16_t passwordSize)
{
    uint16_t checksum = 0;
    uint16_t len = 0u;
    bool result = false;

    if (passwordSize != PASSWORD_LENGTH)
    {
        return false;
    }

    //set up the command:
    sendBuffer[len++] = (uint8_t)(HEADER);
    sendBuffer[len++] = RX_PASSWORD_NL;
    sendBuffer[len++] = RX_PASSWORD_NH;
    sendBuffer[len++] = RX_PASSWORD;

    //insert the password
    memcpy(&sendBuffer[len], password, PASSWORD_LENGTH);

    len+= PASSWORD_LENGTH;

    //calc checksum
    checksum = BSL_calculateChecksum(&sendBuffer[CS_START_IDX], (len - CS_START_IDX));
    sendBuffer[len++] = GetCKL(checksum);
    sendBuffer[len++] = GetCKH(checksum);

    sendAndReceiveOverUart((uint8_t*)sendBuffer, len, (uint8_t*)receiveBuffer, PW_RESPONSE_LEN);

    //check response to verify that the password was accepted:
    if ((receiveBuffer[0] == ACK)
        &&(receiveBuffer[1] == HEADER)
        &&(receiveBuffer[2] == RX_PASSWORD_RESP_NL)
        &&(receiveBuffer[3] == RX_PASSWORD_RESP_NH)
        &&(receiveBuffer[4] == RX_PASSWORD_RESP_CMD)
        &&(receiveBuffer[5] == 0x00))
    {
        result = true;
    }

    return result;
}


//*****************************************************************************
// Read the MSP memory ********************************************************
// startAddress: The address to start reading *********************************
// lenght: The length of the memory block to be read **************************
// dataResult: The array to contain the data read *****************************
//*****************************************************************************

bool BSL_readMemory(uint32_t startAddress, uint8_t length, uint8_t * dataResult)
{
    uint16_t checksum = 0;
    uint16_t lenToSend = 0;
    bool result = false;

    if ( length > MAX_CHUNK_SIZE )
    {
        return false;
    }

    sendBuffer[lenToSend++] = (uint8_t)(HEADER);
    sendBuffer[lenToSend++] = (uint8_t)(TX_DATA_BLOCK_NL);
    sendBuffer[lenToSend++] = (uint8_t)(TX_DATA_BLOCK_NH);
    sendBuffer[lenToSend++] = TX_DATA_BLOCK;

    sendBuffer[lenToSend++] = (uint8_t)(startAddress & 0x00ff);
    sendBuffer[lenToSend++] = (uint8_t)((startAddress >> 8) & 0x00ff);
    sendBuffer[lenToSend++] = (uint8_t)((startAddress >> 16) & 0x00ff);
    sendBuffer[lenToSend++] = (uint8_t)(length & 0x00ff);
    sendBuffer[lenToSend++] = (uint8_t)((length >> 8) & 0x00ff);

    checksum = BSL_calculateChecksum(&sendBuffer[CS_START_IDX], (lenToSend - CS_START_IDX));
    sendBuffer[lenToSend++] = (uint8_t)(checksum);
    sendBuffer[lenToSend++] = (uint8_t)(checksum >> 8);

    sendAndReceiveOverUart((uint8_t*)sendBuffer, lenToSend, (uint8_t*)receiveBuffer, length + READ_RESPONSE_LEN);

    //if ack and header are received, copy the data into the provided buffer
    if ((receiveBuffer[0] != ACK)
        &&(receiveBuffer[1] != HEADER))
    {
        result = false;
    }
    else
    {
        //copy into the buffer starting after the header/ack/etc receive block
        memcpy(dataResult, &receiveBuffer[READ_RESPONSE_LEN- CS_LEN], length);
        result = true;
    }

    return result;
}

//*****************************************************************************
// Write data to MSP memory ***************************************************
// startAddress: The address to start the memory write ************************
// length: The length of the data to be writtem *******************************
// data: The array containing the data to write *******************************
//*****************************************************************************

bool BSL_writeMemory(uint32_t startAddress, uint8_t length, uint8_t * data)
{
    uint16_t checksum = 0;
    uint16_t lenToSend = 0;
    bool res = false;

    sendBuffer[lenToSend++] = (uint8_t)(HEADER);
    sendBuffer[lenToSend++] = (uint8_t)((length + 4) & 0x00ff);
    sendBuffer[lenToSend++] = (uint8_t)(((length + 4) >> 8) & 0x00ff);
    sendBuffer[lenToSend++] = RX_DATA_BLOCK;

    sendBuffer[lenToSend++] = (uint8_t)(startAddress & 0x00ff);
    sendBuffer[lenToSend++] = (uint8_t)((startAddress >> 8) & 0x00ff);
    sendBuffer[lenToSend++] = (uint8_t)((startAddress >> 16) & 0x00ff);

    memcpy(&sendBuffer[lenToSend], data, length);

    lenToSend += length;

    checksum = BSL_calculateChecksum(&sendBuffer[CS_START_IDX], (lenToSend - CS_START_IDX));
    sendBuffer[lenToSend++] = (uint8_t)(checksum);
    sendBuffer[lenToSend++] = (uint8_t)(checksum >> 8);

    memset(&receiveBuffer, 0, MEM_RESPONSE_LEN);

    sendAndReceiveOverUart((uint8_t*)sendBuffer, lenToSend, (uint8_t*)receiveBuffer, MEM_RESPONSE_LEN);

    //check if the write was successful
    if ((receiveBuffer[0] == ACK)
        &&(receiveBuffer[1] == HEADER)
        &&(receiveBuffer[2] == RX_DATA_BLOCK_RESP_NL)
        &&(receiveBuffer[3] == RX_DATA_BLOCK_RESP_NH)
        &&(receiveBuffer[4] == RX_DATA_BLOCK_RESP_CMD)
        &&(receiveBuffer[5] == 0x00))
    {
        res = true;
    }
    else
    {
       res = false;
    }

    return res;
}


//*****************************************************************************
// Write large data array to memory. This includes data with length larger ****
// than MAX_CHUNK_SIZE ********************************************************
// startAddress: The address to start the memory write ************************
// length: The length of the data to be writtem *******************************
// data: The array containing the data to write *******************************
//*****************************************************************************

bool BSL_writeLargeChunkOfDataToMemory(uint32_t startAddress, uint32_t length, uint8_t * data)
{
    uint32_t currentAddress = startAddress;
    uint32_t currentLength = length;
    uint8_t * currentData = data;
    bool done = false;
    bool result = true;

    //write data piece by piece
    while(!done)
    {
        if (currentLength <= 0)
        {
            done = true;
        }
        else if (currentLength < MAX_CHUNK_SIZE)
        {
            result = BSL_writeMemory(currentAddress, currentLength, currentData);

            if (!result)
            {
                return result;
            }

            done = true;
        }
        else
        {
            result = BSL_writeMemory(currentAddress, MAX_CHUNK_SIZE, currentData);

            if (!result)
            {
                return result;
            }

            currentAddress += MAX_CHUNK_SIZE;
            currentData += MAX_CHUNK_SIZE;
            currentLength -= MAX_CHUNK_SIZE;
        }
    }

    return true;
}

//*****************************************************************************
// Perform a mass erase *******************************************************
//*****************************************************************************

bool BSL_massErase(void)
{
    uint16_t checksum = 0;
    uint16_t lenToSend = 0;

    sendBuffer[lenToSend++] = (uint8_t)(HEADER);
    sendBuffer[lenToSend++] = MASS_ERASE_NL;
    sendBuffer[lenToSend++] = MASS_ERASE_NH;
    sendBuffer[lenToSend++] = MASS_ERASE;

    checksum = BSL_calculateChecksum(&sendBuffer[CS_START_IDX], (lenToSend - CS_START_IDX));
    sendBuffer[lenToSend++] = (uint8_t)(checksum);
    sendBuffer[lenToSend++] = (uint8_t)(checksum >> 8);

    sendAndReceiveOverUart((uint8_t*)sendBuffer, lenToSend, (uint8_t*)receiveBuffer, MEM_RESPONSE_LEN);

    //verify the erase was successful
    if ((receiveBuffer[0] == ACK)
        &&(receiveBuffer[1] == HEADER)
        &&(receiveBuffer[2] == MASS_ERASE_RESP_NL)
        &&(receiveBuffer[3] == MASS_ERASE_RESP_NH)
        &&(receiveBuffer[4] == MASS_ERASE_RESP_CMD)
        &&(receiveBuffer[5] == 0x00))
    {
        return true;
    }

    return false;
}

//*****************************************************************************
// Load the program counter with the specified address ************************
//*****************************************************************************

bool BSL_loadPC(uint32_t startAddress)
{
    uint16_t checksum = 0;
    uint16_t lenToSend = 0;

    sendBuffer[lenToSend++] = (uint8_t)(HEADER);
    sendBuffer[lenToSend++] = LOAD_PC_NL;
    sendBuffer[lenToSend++] = LOAD_PC_NH;
    sendBuffer[lenToSend++] = LOAD_PC;

    sendBuffer[lenToSend++] = (uint8_t)(startAddress & 0x00ff);
    sendBuffer[lenToSend++] = (uint8_t)((startAddress >> 8) & 0x00ff);
    sendBuffer[lenToSend++] = (uint8_t)((startAddress >> 16) & 0x00ff);

    checksum = BSL_calculateChecksum(&sendBuffer[CS_START_IDX], (lenToSend - CS_START_IDX));
    sendBuffer[lenToSend++] = (uint8_t)(checksum);
    sendBuffer[lenToSend++] = (uint8_t)(checksum >> 8);

    sendAndReceiveOverUart((uint8_t*)sendBuffer, lenToSend, (uint8_t*)receiveBuffer, 0);

    return true;
}


//calculate CRC over a given section of FRAM
uint16_t BSL_performCrcCheck(uint32_t startAddress, uint16_t len)
{
    uint16_t checksum = 0;
    uint16_t lenToSend = 0;
    uint16_t checksumReceived = 0;

    sendBuffer[lenToSend++] = (uint8_t)(HEADER);
    sendBuffer[lenToSend++] = CRC_CHECK_LEN;
    sendBuffer[lenToSend++] = CRC_CHECK_RSP_LEN;
    sendBuffer[lenToSend++] = CRC_CHECK;

    sendBuffer[lenToSend++] = (uint8_t)(startAddress & 0x00ff);
    sendBuffer[lenToSend++] = (uint8_t)((startAddress >> 8) & 0x00ff);
    sendBuffer[lenToSend++] = (uint8_t)((startAddress >> 16) & 0x00ff);

    memcpy(&sendBuffer[lenToSend], &len, sizeof(uint16_t));

    lenToSend += sizeof(uint16_t);

    checksum = BSL_calculateChecksum(&sendBuffer[CS_START_IDX], (lenToSend - CS_START_IDX));
    sendBuffer[lenToSend++] = (uint8_t)(checksum);
    sendBuffer[lenToSend++] = (uint8_t)(checksum >> 8);

    sendAndReceiveOverUart((uint8_t*)sendBuffer, lenToSend, (uint8_t*)receiveBuffer, CRC_RESPONSE_LEN);

    //check response before returning the crc check values:
    if ((receiveBuffer[0] == ACK)
        &&(receiveBuffer[1] == HEADER)
        &&(receiveBuffer[2] == 3)
        &&(receiveBuffer[3] == 0)
        &&(receiveBuffer[4] == 0x3A))
    {
        checksumReceived =  (uint16_t)( (uint16_t)receiveBuffer[6] << 8 | (uint8_t)receiveBuffer[5]);
    }

    return checksumReceived;
}

static void sendAndReceiveOverUart(uint8_t *pSendData, uint16_t bytesToSend, uint8_t *pRxData, uint16_t bytesToRx)
{
    //we dont want to task switch in the middle of the uart transfer:
    vTaskSuspendAll();

    //send and receive data
    UART_sendDataBlockingSsm(SSM, pSendData, bytesToSend);
    UART_recieveDataBlocking(SSM, pRxData, bytesToRx);

    //switch tasks if needed
    xTaskResumeAll();
}

//*****************************************************************************
// Calculate the CRC16 ********************************************************
// data_p: Pointer to the array containing the data for CRC16 *****************
// lenght: The length of the data for CRC16 ***********************************
//*****************************************************************************

uint16_t BSL_calculateChecksum(const uint8_t* data_p, uint16_t length)
{
    uint8_t x;
    uint16_t crc = 0xFFFF;

    while (length--){
        x = crc >> 8 ^ *data_p++;
        x ^= x>>4;
        crc = (crc << 8) ^ ((uint16_t)(x << 12)) ^ ((uint16_t)(x <<5)) ^ ((uint16_t)x);
    }
    return crc;
}

#endif /* DEVICE_DRIVERS_MSPBSLPROTOCOL_C_ */
