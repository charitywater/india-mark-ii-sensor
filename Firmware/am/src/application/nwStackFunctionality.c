/**************************************************************************************************
* \file     nwStackFunctionality.c
* \brief    Init and handle uart routing of ppp/lwip, parse AT commands for responses
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

#include "uart.h"
#include "stdbool.h"
#include "ppp.h"
#include "logTypes.h"
#include "lwip/init.h"
#include "lwip/tcpip.h"
#include "lwip/sockets.h"
#include "pppos.h"
#include "sara_u201.h"
#include "ntpHandler.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"
#include "stm32l4r5xx.h"
#include <stm32l4xx_hal.h>
#include "awsNetworkHandler.h"
#include "ntpHandler.h"
#include "eventManager.h"
#include "connectivity.h"
#include "ATECC608A.h"
#include "nwStackFunctionality.h"

#define MSG_FREE_INDICATOR          'Z'
#define MSG_START_INDICATOR         '\r'
#define MSG_FIRST_END_INDICATOR     '\r'
#define MSG_END_INDICATOR           '\n'
#define MAX_MSGS                     4
#define IMEI_BYTE_LEN                15
#define CRYPTO_DEVICE_ID_LEN         9
#define MAX_BUFFERED_CELL_CHARS      1800
#define TX_TIMEOUT_TICKS             0xFFFFFFF
#define TASK_POLLING_RATE_MS         2
#define NW_REG_ROAMING               5
#define NW_REG_HOME                  1

//from AT command
#define IMEI_ID                     "CGSN"
#define LEN_IMEI_ID                  7
#define NW_STAT_RESPONSE            "CGREG"
#define NW_STAT_IDX                  19
#define RSSI_RESPONSE               "CSQ"
#define RSSI_RSP_IDX                 12

typedef struct
{
    char startFrame;
    char msg[50];
    uint8_t len;
}cellMessage_t;


typedef enum
{
    WAITING_FOR_START,
    WAITING_FOR_FIRST_END_BYTE,
    WAITING_FOR_END,
}cellParsingState_t;

//buffer to hold incoming messages that need to be processed
static cellMessage_t incomingMsgs[MAX_MSGS];


static ppp_pcb *pppHandle;
static struct netif pppNetifHandle;
volatile uint8_t rxByte = 0u;
static bool inPppRxMode = false;
static bool currentlyTransmitting = false;

//unique ID of the cell modem
static uint8_t imei[IMEI_BYTE_LEN] = {};

//unique ID of the crypto device to use for a cell connection
static uint8_t cryptoUniqueId[CRYPTO_DEVICE_ID_LEN];

//Each hex byte will be 2 characters:
static char xCrypIdString[CRYPTO_DEVICE_ID_LEN*2];

uint8_t receivedCharacters[MAX_BUFFERED_CELL_CHARS];
uint16_t rxHead = 0u;
uint16_t rxTail = 0u;

static uint16_t uartErrors = 0;
static uint16_t xTimeSinceLastPrint = 0;

static bool timeSyncRequested = false;
static bool nwRegistered = false;
static uint8_t nwStat = 0u;
static uint32_t currentRssi = 99u; //init to invalid number
static uint32_t waitingOnNwTimeout = 0u;
static uint32_t cellStartTime = 0u;


static u32_t ppposTxOutputCb(ppp_pcb *pcb, u8_t *data, u32_t len, void *ctx);
static void ppposStatusCb(ppp_pcb *pcb, int err_code, void *ctx);
static void ctxcbFunction(void);
static void parseAtModeResponseInput(char byte);
static void xLookForImei(uint8_t *incomingAtRsp);
static void xLookForRssi(uint8_t *incomingAtRsp);
static void xLookForRegStatus(uint8_t *incomingAtRsp);
static void initMessageBuffers(void);
static void freeUpMessageBuffer(cellMessage_t * pMsg);
static cellMessage_t * getFreeMessageBufferPointer(void);


void ATcommandModeParsing_Task(void)
{
    initMessageBuffers();

    while (1)
    {
        //route received UART chars depending on which mode we are in:
        if ( inPppRxMode == false )
        {
            //check if there's any complete AT command buffers to process
            for ( uint8_t index = 0; index < MAX_MSGS; index++ )
            {
                if ( 0 != incomingMsgs[index].len &&  MSG_START_INDICATOR == incomingMsgs[index].startFrame )
                {
                    //process and free up the message
                    elogInfo("rcd: %s", incomingMsgs[index].msg);

                    //parse response for unique ID
                    xLookForImei((uint8_t*)&incomingMsgs[index].msg);

                    //look for registration status
                    xLookForRegStatus((uint8_t*)&incomingMsgs[index].msg);

                    //parse rssi
                    xLookForRssi((uint8_t*)&incomingMsgs[index].msg);

                    freeUpMessageBuffer(&incomingMsgs[index]);
                }
            }
        }
        else
        {
            //if we have new data in the buffer, process it
            if ( rxHead != rxTail)
            {
                while (rxHead != rxTail )
                {
                    //route characters to the ppp handler
                     pppos_input(pppHandle, &receivedCharacters[rxTail], sizeof(uint8_t));

                     rxTail++;

                     if (rxTail >= MAX_BUFFERED_CELL_CHARS)
                     {
                         rxTail = 0;
                     }
                }
            }
        }

        //every 30 secs print uart errors
        if ( xTimeSinceLastPrint >= 30000)
        {
           // elogInfo(" uart errors %d", uartErrors);
            xTimeSinceLastPrint = 0;
        }

        xTimeSinceLastPrint += TASK_POLLING_RATE_MS;

        vTaskDelay(TASK_POLLING_RATE_MS);
    }
}


//callback function when we receive UART chars.
//Either throw into a buffer of raw bytes to process in the task (for ppp mode)
//or put into an at command buffer to parse
void NW_processCellRxByte(void)
{
    if (inPppRxMode == true )
    {
        //route characters to the ppp handler
        receivedCharacters[rxHead] = rxByte;

        rxHead++;
        if(rxHead >= MAX_BUFFERED_CELL_CHARS )
        {
            rxHead = 0;
        }
    }
    else
    {
        //save off the response into a buffer, similar to GPS code structure
        parseAtModeResponseInput(rxByte);
    }

    if ( currentlyTransmitting != true )
    {
        //kick off another rx
        UART_recieveDataNonBlocking(CELLULAR, &rxByte, sizeof(uint8_t));
    }
}

uint32_t NW_getRssiValue(void)
{
    /* Values can be interpreted based on this table:
     *  <Rssi Reading>| RSSI of the NW in dBm
       _______________|________________________
     * 31-46 dBm      | <= RSSI of the network <= -25 dBm
     * 30             | -50.5 dBm
     * 27             | -55.5 dBm
     * 24             | -60.5 dBm
     * 22             | -65.5 dBm
     * 19             | -70.5 dBm
     * 17             | -75.5 dBm
     * 14             | -80.5 dBm
     * 12             | -85.5 dBm
     * 10             | -90.5 dBm
     * 7              | -95.5 dBm
     * 4              | -100.5 dBm
     * 2              | -105.5 dBm
     * 0              | RSSI of the network <= -110.5 dBm
     * 99             | Not connected to a NW
     */

    return currentRssi;
}

uint64_t NW_getImeiOfModem(void)
{
    uint64_t imeiVariable = 0;

    //convert to 64 bit
    imeiVariable = atoll((char*)&imei);

    return imeiVariable;
}

//how returns how long this task has been active for
uint32_t NW_getCellOnTimeMs(void)
{
    return ( xTaskGetTickCount() - cellStartTime );
}

static void parseAtModeResponseInput(char byte)
{
    static cellParsingState_t cellState = WAITING_FOR_START;
    static cellMessage_t * pMsg = NULL;
    static uint8_t payloadBytesReceived = 0u;

    //switch how we handle the incoming byte based on state
    switch ( cellState )
    {
        case WAITING_FOR_START:
        {
            pMsg = NULL;

            //if this is the start of a new message, start tracking the msg contents
            pMsg = getFreeMessageBufferPointer();

            if ( NULL != pMsg)
            {
                pMsg->msg[payloadBytesReceived] = byte;
                payloadBytesReceived++;

                //change state
                cellState = WAITING_FOR_FIRST_END_BYTE;
            }
            break;
        }
        case WAITING_FOR_FIRST_END_BYTE:
        {
            // wait for the second to last byte of the msg
            if ( MSG_FIRST_END_INDICATOR == byte )
            {
                // put the byte received into the message
                pMsg->msg[payloadBytesReceived] = byte;
                payloadBytesReceived++;

                cellState = WAITING_FOR_END;
            }
            else
            {
                pMsg->msg[payloadBytesReceived] = byte;
                payloadBytesReceived++;
                //handle this error case
            }
            break;
        }
        case WAITING_FOR_END:
        {
            // wait for the end of the msg
            if ( '\n' == byte )
            {
                //dont complete the message if /n is part of the message. /r/r indicates this
                if(pMsg->msg[payloadBytesReceived-2] == '\r')
                {
                    // put the byte received into the message
                    pMsg->msg[payloadBytesReceived] = byte;
                    payloadBytesReceived++;
                }
                else
                {
                    //put the length of the message into the buffer
                    pMsg->len = payloadBytesReceived;

                    //reset state machine
                    payloadBytesReceived = 0;
                    cellState = WAITING_FOR_START;
                }
            }
            else
            {
                // put the byte received into the message
                pMsg->msg[payloadBytesReceived] = byte;
                payloadBytesReceived++;
            }
            break;
        }
    }
}


static void initMessageBuffers(void)
{
    //init all bubbers as free
    for ( uint8_t idx = 0; idx < MAX_MSGS; idx++ )
    {
        incomingMsgs[idx].startFrame = MSG_FREE_INDICATOR;
    }
}


static cellMessage_t * getFreeMessageBufferPointer(void)
{
    // Fand return a free message Tx buffer
    for ( uint8_t idx = 0; idx < MAX_MSGS; idx++ )
    {
        if ( MSG_FREE_INDICATOR == incomingMsgs[idx].startFrame )
        {
            //return pointer to this buffer
            incomingMsgs[idx].startFrame = MSG_START_INDICATOR;
            return &incomingMsgs[idx];
        }
    }

    //no buffer was found
    return NULL;
}

static void xLookForImei(uint8_t *incomingAtRsp)
{
    char *imeiLoc = strstr((char*)incomingAtRsp, IMEI_ID);

    if ( imeiLoc != NULL )
    {
        imeiLoc += LEN_IMEI_ID;
        memcpy(&imei, imeiLoc, IMEI_BYTE_LEN);
        elogInfo("Found IMEI: %s", imei);
    }
}

static void xLookForRssi(uint8_t *incomingAtRsp)
{
    char *rssiStr = strstr((char*)incomingAtRsp, RSSI_RESPONSE);
    char* p;

    if ( rssiStr != NULL )
    {
        //found the keyword, increment the string to the index of the actual status
        rssiStr += RSSI_RSP_IDX;

        //convert to integer
        currentRssi = strtol(rssiStr, &p, 10);
    }
}

static void xLookForRegStatus(uint8_t *incomingAtRsp)
{
    char *regStat = strstr((char*)incomingAtRsp, NW_STAT_RESPONSE);
    char* p;

    if ( regStat != NULL )
    {
        //found the keyword, increment the string to the index of the actual status
        regStat += NW_STAT_IDX;

        //convert to integer
        nwStat = strtol(regStat, &p, 10);

        //From the datasheet:
        /*
         *    0: not registered, the MT is not currently searching an operator to register to
            • 1: registered, home network
            • 2: not registered, but MT is currently searching a new operator to register to
            • 3: registration denied
            • 4: unknown (e.g. out of GERAN/UTRAN coverage)
            • 5: registered, roaming
            • 8: attached for emergency bearer services only (see 3GPP TS 24.008 [12] and 3GPP
            TS 24.301 [69] that specify the condition when the MS is considered as attached
            for emergency bearer services) (applicable only when <AcT> indicates 2,4,5,6)
         */
        if ( nwStat == NW_REG_HOME || nwStat == NW_REG_ROAMING )
        {
            nwRegistered = true;
        }
    }
}

static void freeUpMessageBuffer(cellMessage_t * pMsg)
{
    //indicate this buffer is free by setting length to 0 and start frame
    pMsg->startFrame = MSG_FREE_INDICATOR;
    pMsg->len = 0;

    for (uint8_t i = 0; i< 50; i++ )
    {
        //fill in will null chars
        pMsg->msg[i] = '\0';
    }
}

//called when a transmission has completed
void NW_txComplete(void)
{
    currentlyTransmitting = false;

    //kick off another rx
    UART_recieveDataNonBlocking(CELLULAR, &rxByte, sizeof(uint8_t));
}

//uart error handler..
void NW_handleUartError(void)
{
    uartErrors++;
  // UART_recieveDataNonBlocking(CELLULAR, &rxByte, sizeof(uint8_t));
}

void NW_initUart(void)
{
    //init cell uart...need to init AFTER we have already
    //turned on the modem
    UART_initCellUart();

    //dont start receiving yet... just send some chars to get the baud rate set
    //(the modem uses 'auto baud' to set the baud rate
    SARA_getSimId();

    //now enable interrupts
    HAL_NVIC_EnableIRQ(UART5_IRQn);

    //start rx-ing
    UART_recieveDataNonBlocking(CELLULAR, &rxByte, sizeof(uint8_t));
}

//this needs to be called from within a task!!!!
void NW_initLwip(void)
{
    err_t err = ERR_RST;
    bool initStatusCode = false;

    //init timeout variable for cell being on
    waitingOnNwTimeout = xTaskGetTickCount();

    //start time
    cellStartTime =  xTaskGetTickCount();

    //set up thing name for connection
    //get thing name
    initStatusCode = ATECC_getUniqueId((uint8_t*)&cryptoUniqueId);

    //do not continue if we dont have a unique ID
    if ( initStatusCode == true )
    {
        //now convert the byte array to hex character array:
        for (uint8_t i = 0; i < CRYPTO_DEVICE_ID_LEN ; i++)
        {
            //each byte takes up 2 characters:
            sprintf(&xCrypIdString[i*2],"%02x",cryptoUniqueId[i]);
        }

        //log the ID:
        elogInfo("Crypto ID: %s", xCrypIdString);

        //this function initializes all of LWIP
        //also creates a new TASK for tcp input
        tcpip_init(NULL, NULL);

        NW_initUart();

        //add a call to grab IMEI
        SARA_getImei();

        //the following code takes the modem OUT of AT mode.
        //AT commands will no longer work on the modem!
        SARA_initAndSetApn();

        while (nwRegistered == false && (waitingOnNwTimeout - cellStartTime) <= AM_CELL_TIME_ON_MS)
        {
            SARA_sendNwRegistrationCmd();
            vTaskDelay(10000);

            waitingOnNwTimeout += (xTaskGetTickCount() - waitingOnNwTimeout);
        }

        if ( nwRegistered == true )
        {
            elogDebug("Registered To Network");

            //send rssi cmd
            SARA_getRssi();

            //save RSSI
            CONN_updateRssiForAntennaUsed(currentRssi);
        }
        else
        {
            //save RSSI - will be 99
            CONN_updateRssiForAntennaUsed(currentRssi);

            //Pass up the error so we can go back to sleep
            EVT_indicateCloudConnectFailure();
            return;
        }

        SARA_initDataMode();

        //create a ppp struct
        pppHandle = pppos_create(&pppNetifHandle, ppposTxOutputCb, ppposStatusCb, ctxcbFunction);
        ppp_set_default(pppHandle);

        /* Ask the peer for up to 2 DNS server addresses. */
        ppp_set_usepeerdns(pppHandle, 1);

        //for now no authentication
        ppp_set_auth(pppHandle, PPPAUTHTYPE_NONE, "", "");

        //init ip address as 0. The lwip stack will fill this in
        pppNetifHandle.ip_addr.addr = 0;

        //we are now in ppp mode - route uart chars to the stack
        inPppRxMode = true;

        //attempt to open the ppp connection over serial
        err = ppp_connect(pppHandle, 0);
    }
    else
    {
        //Pass up the error so we can go back to sleep and try again later
        EVT_indicateCloudConnectFailure();
    }


    //to do something else with error handling...
    while ( err!=ERR_OK )
    {
        vTaskDelay(100);
    }
}

void NW_timeSyncRequested(bool flag)
{
    timeSyncRequested = flag;
}

/* sys_now needed by pppos.c module */
u32_t sys_now(void)
{
    //get current systick value
    return xTaskGetTickCount();
}

/* sys_jiffies needed by magic.c module..idk what this is even for */
u32_t sys_jiffies(void)
{
  static u32_t jiffies_loc = 0U;
  jiffies_loc += 10U;
  return jiffies_loc;
}

static void ctxcbFunction(void)
{
    //do nothing, may want to add to this later
}


/*
 * PPPoS serial output callback
 *
 * ppp_pcb, PPP control block
 * data, buffer to write to serial port
 * len, length of the data buffer
 * ctx, optional user-provided callback context pointer
 *
 * Return value: len if write succeed
 */
static u32_t ppposTxOutputCb(ppp_pcb *pcb, u8_t *data, u32_t len, void *ctx)
{
    uint32_t stat;
    uint32_t msTimeout = 0;

    //this flag is reset in the DMA interrupt handler once finished
    currentlyTransmitting = true;

    //start transmitting, interrupt based using DMA
    stat = UART_sendDataNonBlockingWithDma(CELLULAR, data, len);

    if (stat == HAL_OK)
    {
       //hang out while transmitting, time out if stuck
       while ( currentlyTransmitting == true && msTimeout <= TX_TIMEOUT_TICKS )
       {
           msTimeout++;
       }

       //if still transmitting, return 0
       if ( currentlyTransmitting == true )
       {
           len = 0;
       }
    }
    else
    {
        len = 0;
    }

    return len;
}

//Add more to this cb as we get further along
static void ppposStatusCb(ppp_pcb *pcb, int err_code, void *ctx)
{
  //UNUSED(ctx);
  switch (err_code)
  {
    case PPPERR_NONE:
    {
        elogInfo("CONNECTED!!!");
        elogInfo("IP Address %d.%d.%d.%d", (uint8_t)(pcb->netif->ip_addr.addr>>0), (uint8_t)(pcb->netif->ip_addr.addr>>8), (uint8_t)(pcb->netif->ip_addr.addr>>16), (uint8_t)(pcb->netif->ip_addr.addr>>24));

        //now kick off AWS libraries for connection to AWS and then MQTT - pass in the crypto unique ID
        AWS_initNetworkHandler((uint8_t*)&xCrypIdString, sizeof(xCrypIdString));

        // If we need to get an NTP time update
        if (timeSyncRequested)
        {
            NW_timeSyncRequested(false);
            NTP_init();
        }

      break;
    }
    case PPPERR_PARAM:
    {
      elogError("status_cb: Invalid parameter\n\r");
      break;
    }
    case PPPERR_OPEN:
    {
      elogError("status_cb: Unable to open PPP session\n\r");
      break;
    }
    case PPPERR_DEVICE:
    {
      elogError("status_cb: Invalid I/O device for PPP\n\r");
      break;
    }
    case PPPERR_ALLOC:
    {
      elogError("status_cb: Unable to allocate resources\n\r");
      break;
    }
    case PPPERR_USER:
    {
      elogError("status_cb: User interrupt\n\r");
      break;
    }
    case PPPERR_CONNECT:
    {
      elogError("status_cb: Connection lost\n\r");
      break;
    }
    case PPPERR_AUTHFAIL:
    {
      elogError("status_cb: Failed authentication challenge\n\r");
      break;
    }
    case PPPERR_PROTOCOL:
    {
      elogError("status_cb: Failed to meet protocol\n\r");
      break;
    }
    case PPPERR_PEERDEAD:
    {
      elogError("status_cb: Connection timeout\n\r");
      break;
    }
    case PPPERR_IDLETIMEOUT:
    {
      elogError("status_cb: Idle Timeout\n\r");
      break;
    }
    case PPPERR_CONNECTTIME:
    {
      elogError("status_cb: Max connect time reached\n\r");
      break;
    }
    case PPPERR_LOOPBACK:
    {
      elogError("status_cb: Loopback detected\n\r");
      break;
    }
    default:
    {
      elogError("status_cb: Unknown error code %ld\n\r", err_code);
      break;
    }
  }

  //if there were any errors, don't retry, just pass the error up
  if (err_code != PPPERR_NONE)
  {
      elogError("FAILED to get or LOST cell connection");

      //Pass up the error
      EVT_indicateCloudConnectFailure();
  }
}
