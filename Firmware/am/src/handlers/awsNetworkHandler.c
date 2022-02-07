/**************************************************************************************************
* \file     awsNetworkHandler.c
* \brief    Get 'network' up and running, init callack for nw connected and disconnected
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "logTypes.h"

#include "iot_demo_logging.h"

/* Platform layer includes. */
#include "iot_clock.h"
#include "iot_threads.h"
#include "iot_network_types.h"

#include "mqttHandler.h"
#include "iot_init.h"
#include "aws_iot_network_config.h"
#include "iot_network_manager_private.h"

#include "mqttHandler.h"
#include "awsNetworkHandler.h"

//using crpyto ID as DUID, so should be 18 bytes ( 9 hex bytes * 2)
#define EXPECTED_DUID_LEN            18

/* Semaphore used to wait for a network to be available. */
static IotSemaphore_t iotNetworkSemaphore;

/* Variable used to indicate the connected network. */
static uint32_t xConnectedNw = AWSIOT_NETWORK_TYPE_NONE;

static IotNetworkManagerSubscription_t subscription = IOT_NETWORK_MANAGER_SUBSCRIPTION_INITIALIZER;

static uint8_t duidBuffer[EXPECTED_DUID_LEN] = {};

static int xInitAwsLibs( void );
static uint32_t xGetConnectedNetworks( uint32_t nwType );
static uint32_t xWaitForNetworkConnection( uint32_t nw );
static void xNetworkStateChangeCb( uint32_t network, AwsIotNetworkState_t state, void * pContext );
static void xDisconnect(void);

void AWS_initNetworkHandler(uint8_t *duid, uint8_t duidLen)
{
    //create the task to handle aws network
    if ( duidLen == EXPECTED_DUID_LEN )
    {
        if( xTaskCreate( AWS_NwTask, "aws_thread", ( configSTACK_DEPTH_TYPE ) 768*4, NULL, 7, NULL ) != pdPASS )
        {
           elogError("Failed to create task");
        }
        else
        {
           elogDebug("Created AWS nw task");
        }

        //init the imei which will be used for MQTT once the network is up
        memcpy(&duidBuffer, duid, duidLen);
    }
    else
    {
        elogError("invalid DUID length");
    }
}

void AWS_NwTask()
{
    /* credentials and server info are defined in a header for now. We will switch to crypto later */
    const IotNetworkInterface_t * pNetworkInterface = NULL;
    void * pConnectionParams = NULL, * pCredentials = NULL;
    int status;

    IotLogInfo( "init network and libraries \n" );

    status = xInitAwsLibs();

    if( status == EXIT_SUCCESS )
    {
        IotLogInfo( "Successfully initialized the AWS libs and network" );

        //pull out the nw interface that was initialized, connection params, and credentials
        pNetworkInterface = AwsIotNetworkManager_GetNetworkInterface( xConnectedNw );
        pConnectionParams = AwsIotNetworkManager_GetConnectionParams( xConnectedNw );
        pCredentials = AwsIotNetworkManager_GetCredentials( xConnectedNw );

        //now, kick off mqtt
        status = MQTT_init(true, pConnectionParams, pCredentials, pNetworkInterface, (uint8_t*)&duidBuffer, EXPECTED_DUID_LEN);

        if ( status == EXIT_SUCCESS )
        {
           IotLogInfo("MQTT Task Initialzed");
        }
        else
        {
           IotLogError( "MQTT Handler Failed to Init" );
           xDisconnect();
        }
    }
    else
    {
        IotLogError( "Failed to initialize AWS" );
    }

    while(1)
    {

    }
}

static void xDisconnect(void)
{
    /* Remove network manager subscription */
    AwsIotNetworkManager_RemoveSubscription( subscription );

    /* Disable all the networks used */
    AwsIotNetworkManager_DisableNetwork( configENABLED_NETWORKS );

    IotSemaphore_Destroy( &iotNetworkSemaphore );
    IotSdk_Cleanup();
}

static void xNetworkStateChangeCb( uint32_t network, AwsIotNetworkState_t state, void * pContext )
{
    uint32_t disconnectedNetworks = AWSIOT_NETWORK_TYPE_NONE;

    if( ( state == eNetworkStateEnabled ) && ( xConnectedNw == AWSIOT_NETWORK_TYPE_NONE ) )
    {
        //save the nw that was just initialized
        xConnectedNw = network;

        //nw is up! post to semephore
        IotSemaphore_Post( &iotNetworkSemaphore );

        /* Disable the disconnected (unused) networks to save power and reclaim any unused memory. */
        disconnectedNetworks = configENABLED_NETWORKS & ( ~xConnectedNw );

        if( disconnectedNetworks != AWSIOT_NETWORK_TYPE_NONE )
        {
            AwsIotNetworkManager_DisableNetwork( disconnectedNetworks );
        }
    }
    else if( ( state == eNetworkStateDisabled ) && ( xConnectedNw == network ) )
    {
        /* Re-enable all the networks for reconnection. */
        disconnectedNetworks = configENABLED_NETWORKS & ( ~xConnectedNw );

        if( disconnectedNetworks != AWSIOT_NETWORK_TYPE_NONE )
        {
            AwsIotNetworkManager_EnableNetwork( disconnectedNetworks );
        }

        xConnectedNw = xGetConnectedNetworks( configENABLED_NETWORKS );
    }
}

//returns which networks are active
static uint32_t xGetConnectedNetworks( uint32_t nwType )
{
    uint32_t ret = ( AwsIotNetworkManager_GetConnectedNetworks() & nwType );

    if( ( ret & AWSIOT_NETWORK_TYPE_WIFI ) == AWSIOT_NETWORK_TYPE_WIFI )
    {
        ret = AWSIOT_NETWORK_TYPE_WIFI;
    }
    else if( ( ret & AWSIOT_NETWORK_TYPE_BLE ) == AWSIOT_NETWORK_TYPE_BLE )
    {
        ret = AWSIOT_NETWORK_TYPE_BLE;
    }
    else if( ( ret & AWSIOT_NETWORK_TYPE_ETH ) == AWSIOT_NETWORK_TYPE_ETH )
    {
        ret = AWSIOT_NETWORK_TYPE_ETH;
    }
    else
    {
        ret = AWSIOT_NETWORK_TYPE_NONE;
    }

    return ret;
}


//when a network is active, will post to a semephore
static uint32_t xWaitForNetworkConnection(uint32_t nw)
{
    //wait for the network to become ready
    IotSemaphore_Wait(&iotNetworkSemaphore);

    return xGetConnectedNetworks(nw);
}


//Initialize the aws common libraries, Mqtt library and network manager
//make sure nw is up and running
static int xInitAwsLibs( void )
{
    int status = EXIT_SUCCESS;
    bool commonLibrariesInitialized = false;
    bool semaphoreCreated = false;

    /* Initialize common libraries required by network manager and mqtt */
    if( IotSdk_Init() == true )
    {
        commonLibrariesInitialized = true;
    }
    else
    {
        IotLogInfo( "Failed to initialize the common library." );
        status = EXIT_FAILURE;
    }

    if( status == EXIT_SUCCESS )
    {
        if( AwsIotNetworkManager_Init() != pdTRUE )
        {
            IotLogError( "Failed to initialize network manager library." );
            status = EXIT_FAILURE;
        }
    }

    if( status == EXIT_SUCCESS )
    {
        /* Create semaphore to signal that a network is ready  - in our case TCP/IP Network */
        if( IotSemaphore_Create( &iotNetworkSemaphore, 0, 1 ) != true )
        {
            IotLogError( "Failed to create semaphore to wait for a network connection." );
            status = EXIT_FAILURE;
        }
        else
        {
            semaphoreCreated = true;
        }
    }

    if( status == EXIT_SUCCESS )
    {
        /* Subscribe for network state change from Network Manager.. pass in cb function */
        if( AwsIotNetworkManager_SubscribeForStateChange( configENABLED_NETWORKS,
                                                          xNetworkStateChangeCb,
                                                          NULL,
                                                          &subscription ) != pdTRUE )
        {
            IotLogError( "Failed to subscribe network state change callback." );
            status = EXIT_FAILURE;
        }
    }

    /* Initialize all the networks configured for the device. */
    if( status == EXIT_SUCCESS )
    {
        if( AwsIotNetworkManager_EnableNetwork( configENABLED_NETWORKS ) != configENABLED_NETWORKS )
        {
            IotLogError( "Failed to intialize all the networks configured for the device." );
            status = EXIT_FAILURE;
        }
    }

    if( status == EXIT_SUCCESS )
    {
        /* Wait for network to be initialized */
        xConnectedNw = xGetConnectedNetworks( configENABLED_NETWORKS );

        if( xConnectedNw == AWSIOT_NETWORK_TYPE_NONE )
        {
            /* Network not yet initialized. Block for a network to be initialized. */
            IotLogInfo( "No networks connected, Waiting for a network connection. " );
            xConnectedNw = xWaitForNetworkConnection( configENABLED_NETWORKS );
        }
    }

    if( status == EXIT_FAILURE )
    {
        if( semaphoreCreated == true )
        {
            IotSemaphore_Destroy( &iotNetworkSemaphore );
        }

        if( commonLibrariesInitialized == true )
        {
            IotSdk_Cleanup();
        }
    }

    return status;
}


