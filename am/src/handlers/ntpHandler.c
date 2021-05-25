/**************************************************************************************************
* \file     ntpHandler.c
* \brief    NTP handler - Initialize, send request to and handle response from NTP server
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

/* Standard includes. */
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "logTypes.h"

/* TCP/IP abstraction includes. */
#include "iot_network.h"
#include "lwipopts.h"
#include "iot_network.h"
#include "lwip/ip_addr.h"

/* Platform layer includes. */
#include "iot_network_types.h"
#include <iot_secure_sockets.h>
#include "eventManager.h"
#include "queue.h"

#include "am-ssm-spi-protocol.h"

/* NTP include. */
#include "ntpHandler.h"

#define NTP_CLIENT_PORT     123
#define NTP_MODE_CLIENT     3
#define NTP_MODE_SERVER     4
#define NTP_MSG_BUFFER      48
#define NTP_LI_NO_WARNING   ( 0 << 6 )
#define NTP_VERSION         ( NTP_MODE_SERVER << NTP_MODE_CLIENT )
#define NTP_SERVER_ENDPOINT "pool.ntp.org"

#define RECEIVE_TIMEOUT     6000000
#define TRANSMIT_TIMEOUT    20000

#define NTP_TIMESTAMP_DELTA ( 2208988800UL )

typedef enum{
    ERROR_NONE,
    ERROR_UNKNOWN,
    DNS_LOOKUP_ERROR,
    OPEN_SOCKET_ERROR,
    CONNECT_ERROR,
    SEND_ERROR,
    RECEIVE_ERROR
}ntpTimeSyncError_t;

typedef enum{
    DNS_LOOKUP,
    OPENING_SOCKET,
    CONNECTING,
    SENDING,
    RECEIVING,
    DONE
}ntpTimeSyncState_t;

typedef struct{
  uint32_t seconds;
  uint32_t epoch;
}timeInfo_t;

PACK_STRUCT_BEGIN
struct ntp_msg {
  PACK_STRUCT_FLD_8(u8_t  li_vn_mode);
  PACK_STRUCT_FLD_8(u8_t  stratum);
  PACK_STRUCT_FLD_8(u8_t  poll);
  PACK_STRUCT_FLD_8(u8_t  precision);
  PACK_STRUCT_FIELD(u32_t root_delay);
  PACK_STRUCT_FIELD(u32_t root_dispersion);
  PACK_STRUCT_FIELD(u32_t reference_identifier);
  PACK_STRUCT_FIELD(u32_t reference_timestamp[2]);
  PACK_STRUCT_FIELD(u32_t originate_timestamp[2]);
  PACK_STRUCT_FIELD(u32_t receive_timestamp[2]);
  PACK_STRUCT_FIELD(u32_t transmit_timestamp[2]);
} PACK_STRUCT_STRUCT;
PACK_STRUCT_END

/* Rx and Tx time outs are used to ensure the sockets do not wait too long for
 * missing data. */
static const TickType_t xReceiveTimeOut = RECEIVE_TIMEOUT;
static const TickType_t xSendTimeOut = TRANSMIT_TIMEOUT;

/*
 * @brief Connection parameters placeholder for a TCP/IP network.
 */
static IotNetworkServerInfo_t NTPConnectionParams = { 0 };

static timeInfo_t unixTimeStamp = { 0 };
static ntpTimeSyncError_t status;
static ntpTimeSyncState_t state;

TaskHandle_t xNTPHandle;

static void xHandleNtpResult(void);

int NTP_init(void)
{
    int status = EXIT_FAILURE;

    NTPConnectionParams.pHostName = NTP_SERVER_ENDPOINT;
    NTPConnectionParams.port = NTP_CLIENT_PORT;

    //create the task to handle MQTT
    if( xTaskCreate( NTP_task, "ntp_thread", DEFAULT_THREAD_STACKSIZE, NULL, 7, &xNTPHandle ) != pdPASS )
    {
        elogError("Failed to create NTP task");
    }
    else
    {
        elogInfo("Created NTP task");
        status = EXIT_SUCCESS;
    }

    return status;
}

//this task runs when the SSM requests a time sync
void NTP_task(void)
{
    Socket_t xSocket;
    SocketsSockaddr_t xNtpServerAddress;
    BaseType_t xReturned;

    status = ERROR_NONE;
    state = DNS_LOOKUP;

    /* Establish connection. */
    xNtpServerAddress.ucSocketDomain = SOCKETS_AF_INET;
    xNtpServerAddress.usPort = SOCKETS_htons( NTP_CLIENT_PORT );
    xNtpServerAddress.ulAddress = SOCKETS_GetHostByName( NTPConnectionParams.pHostName );

    // Build the NTP message
     struct ntp_msg ntpMsg;
     memset((void *)&ntpMsg, 0, NTP_MSG_BUFFER);
     ntpMsg.li_vn_mode = NTP_LI_NO_WARNING | NTP_VERSION | NTP_MODE_CLIENT;

     while(1)
     {
         while (status == ERROR_NONE && state != DONE)
         {
             switch (state)
             {
                 case DNS_LOOKUP:

                     if( xNtpServerAddress.ulAddress == 0 )
                     {
                         elogOffNominal("Failed to resolve %s.", NTPConnectionParams.pHostName);
                         status = DNS_LOOKUP_ERROR;
                     }
                     else
                     {
                         state = OPENING_SOCKET;
                     }
                     break;

                 case OPENING_SOCKET:
                     xSocket = SOCKETS_Socket( SOCKETS_AF_INET, SOCKETS_SOCK_DGRAM, SOCKETS_IPPROTO_UDP );
                     if ( xSocket == SOCKETS_INVALID_SOCKET )
                     {
                         elogOffNominal("Failed to open socket");
                         status = OPEN_SOCKET_ERROR;
                     }
                     else
                     {
                         state = CONNECTING;
                     }
                     break;

                 case CONNECTING:
                     // Set a time out so a missing reply does not cause the task to block indefinitely.
                     SOCKETS_SetSockOpt( xSocket, 0, SOCKETS_SO_RCVTIMEO, &xReceiveTimeOut, sizeof( xReceiveTimeOut ) );
                     SOCKETS_SetSockOpt( xSocket, 0, SOCKETS_SO_SNDTIMEO, &xSendTimeOut, sizeof( xSendTimeOut ) );

                     if ( SOCKETS_Connect( xSocket, &xNtpServerAddress, sizeof( xNtpServerAddress ) ) != SOCKETS_ERROR_NONE )
                     {
                         elogOffNominal("Failed to connect to NTP server");
                         status = CONNECT_ERROR;
                     }
                     else
                     {
                         elogInfo("Connected to NTP server!");
                         state = SENDING;
                     }
                     break;

                 case SENDING:
                     if ( SOCKETS_Send( xSocket, ( void * )&ntpMsg, sizeof(ntpMsg), 0 ) < SOCKETS_ERROR_NONE)
                     {
                         elogOffNominal("Failed to send request to NTP server");
                         status = SEND_ERROR;
                     }
                     else
                     {
                         elogInfo("Sent request to NTP server");
                         state = RECEIVING;
                     }
                     break;

                 case RECEIVING:
                     xReturned = SOCKETS_Recv( xSocket, ( void * )&ntpMsg, sizeof( ntpMsg ), 0 );
                     if( xReturned == sizeof( ntpMsg ) )
                     {
                         unixTimeStamp.seconds = lwip_ntohl( ntpMsg.transmit_timestamp[0] );

                         //check the value before we use it
                         if ( unixTimeStamp.seconds >= NTP_TIMESTAMP_DELTA )
                         {
                             unixTimeStamp.epoch = ( unixTimeStamp.seconds - NTP_TIMESTAMP_DELTA );
                             elogInfo("Received response from NTP server: %lu", unixTimeStamp.epoch);
                             state = DONE;
                         }
                         else
                         {
                             elogNotice("Did not receive a valid time from the server: %lu", unixTimeStamp.seconds);
                             status = RECEIVE_ERROR;
                         }
                     }
                     else
                     {
                         elogNotice("Timed out receiving from NTP server");
                         status = RECEIVE_ERROR;
                     }
                     break;

                 default:
                     status = ERROR_UNKNOWN;
                     break;
             }
         }

         if (state == DONE || status >= CONNECT_ERROR)
         {
             if ( SOCKETS_Close( xSocket ) == SOCKETS_ERROR_NONE )
             {
                 elogInfo("Successfully closed socket");
             }
             else
             {
                 elogOffNominal("Failed to close socket");
             }
         }

         xHandleNtpResult();

         vTaskDelete(xNTPHandle);
     }
}


uint32_t NTP_getTime(void)
{
    //will either be 0x00000000 or contain a valid epoch time
    return unixTimeStamp.epoch;
}

static void xHandleNtpResult(void)
{
    if ( status == ERROR_NONE && state == DONE)
    {
        EVT_indicateNtpTimeSyncSuccess();
    }
    else
    {
        EVT_indicateNtpTimeSyncFailure();
    }
}
