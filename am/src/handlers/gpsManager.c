/**************************************************************************************************
* \file     gpsManager.c
* \brief    Functions to handle and maintain the gps module.
*           The GPS module should only be turned on one time upon activation to obtain a GPS fix.
*           It should then remain powered off throughout the rest of the sensors lifetime.
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
#include "stdlib.h"
#include "string.h"
#include "CLI.h"
#include "uart.h"
#include "pwrMgr.h"
#include "PE42424A_RF.h"
#include "FreeRTOS.h"
#include "task.h"
#include "logTypes.h"
#include "messages.pb.h"
#include "memMapHandler.h"
#include "eventManager.h"
#include "gpsManager.h"

#define MSG_FREE_INDICATOR  'Z'
#define MSG_START_INDICATOR '$'

#define MAX_MSGS_RX          8
#define TASK_DELAY_MS        50
#define GPS_RX_SIZE          1
#define MAX_SIZE_NMEA_MSG    95
#define MS_PER_SEC           1000
#define HDOP_SCALAR          100
#define HR_MIN_LEN           2

#define GPS_TASK_PRIORITY    ( configMAX_PRIORITIES - 1 )
#define GPS_TASK_STACK_SIZE  ( configMINIMAL_STACK_SIZE * 14 )

//maybe add more states in the future
typedef enum
{
    NMEA_WAITING_ON_START,
    NMEA_WAITING_ON_END,
}nmeaParsingState_t;

typedef struct
{
    char startFrame;
    char msg[MAX_SIZE_NMEA_MSG];
    uint8_t len;
}nmeaMessage_t;


static bool gpsEnabled = false;
static bool fakeData = false;
static char rxByte = 0u;
static uint32_t printIntervalMs = 15000;

//holds the time the gps module took to obtain a fix
static uint32_t timeToFirstFixMs = 0u;
static bool gpsLocationFixed = false;
static bool printedFirstFix = false;
static bool printWhenMessageReceived = true;


//buffer to hold incoming messages that need to be processed
static nmeaMessage_t incomingMsgs[MAX_MSGS_RX];

//configs:
static uint8_t minNumberSatellites = 0u;
static float maxHdop = 0u;
static uint32_t minTimeGpsOnMs = 0u;
static uint32_t maxTimeGpsOnMs = 0u;

//Contains the GPS fix data
static GpsMessage gpsData;

//task handle
TaskHandle_t xGpsHandle;

static void initMessageBuffers(void);
static void commandHandlerForGps(int argc, char **argv);
static nmeaMessage_t * getFreeMessageBufferPointer(void);
static void processIncomingByte(char rxByte);
static void processFullNmeaMsg(nmeaMessage_t *pMsg);
static void freeUpMessageBuffer(nmeaMessage_t * pMsg);
static char *strtok_f (char *s, char delim);
static char *strtok_fr (char *s, char delim, char **save_ptr);
static float convertNmeaLatAndLongFields(float value);

void GPS_processReceivedChars(void)
{
    // called from the UART RX complete function
    processIncomingByte(rxByte);

    // receive next byte
    UART_recieveDataNonBlocking(GPS_MODULE, (uint8_t*)&rxByte, GPS_RX_SIZE);
}

//task for monitoring GPS
//block on message queue - either time to print out GPS data or turn on GPS to get location
void GPS_monitorTask()
{
    uint8_t index = 0;

    //set up the incoming message buffers as free and ready to be used
    initMessageBuffers();

    while (1)
    {
        //delay 50 ms
        vTaskDelay(TASK_DELAY_MS);

        if ( gpsEnabled == true )
        {
            //Update time tracker
            timeToFirstFixMs += TASK_DELAY_MS;

            if ( timeToFirstFixMs >= maxTimeGpsOnMs )
            {
                elogError("Failed to get GPS coordinates within the timeout");

                //timed out
                EVT_indicateGpsFixCompleted(false);

                //save flash state
                MEM_SetGpsFixedFlag(false);

                //turn off GPS and delete this task
                GPS_Disable();
            }

            //check if there are any buffers to process
            for ( index = 0; index < MAX_MSGS_RX; index++ )
            {
                if ( 0 != incomingMsgs[index].len &&   MSG_START_INDICATOR == incomingMsgs[index].startFrame )
                {
                    //process and free up the message
                    processFullNmeaMsg(&incomingMsgs[index]);
                    freeUpMessageBuffer(&incomingMsgs[index]);
                }
            }
        }
    }
}

void GPS_initModule(void)
{
    /* set up a command line driver */
    CLI_Command_Handler_s cmdHandler;
    cmdHandler.ptrFunction = &commandHandlerForGps;
    cmdHandler.cmdString   = "gps";
    cmdHandler.usageString = "\n\r\ton \n\r\toff \n\r\tfake \n\r\tloc \n\r\tprint";
    CLI_registerThisCommandHandler(&cmdHandler);
}

void GPS_Enable(void)
{
    //grab the latest configs
    uint32_t tempHdhop = MEM_getGpsMaxdHop();

    //max hdop is scaled
    maxHdop = (float)(tempHdhop/HDOP_SCALAR);

    minTimeGpsOnMs = MEM_getGpsMinMeasTimeSec() * MS_PER_SEC;
    maxTimeGpsOnMs = MEM_getGpsTimeoutSeconds() * MS_PER_SEC;
    minNumberSatellites = MEM_getGpsNumSatellites();

    //if we arent already enabled..
    if ( gpsEnabled != true )
    {
        gpsEnabled = true;

        xTaskCreate (GPS_monitorTask, "GPS", GPS_TASK_STACK_SIZE, NULL, GPS_TASK_PRIORITY, &xGpsHandle);

        //enable antenna (gps can only be on the secondary antenna slot)
        RF_cellPrimaryAntennaGpsSecondary();

        //turn on the module
        PWR_turnOnGpsPowerSupply();

        UART_initGpsUart();

        //enable receiving data
        UART_recieveDataNonBlocking(GPS_MODULE, (uint8_t*)&rxByte, GPS_RX_SIZE);

        //reset trackers
        timeToFirstFixMs = 0u;
        gpsLocationFixed = false;
        printedFirstFix = false;
    }

    elogInfo("GPS ENABLED");
}

bool GPS_isGpsEnabled(void)
{
    return gpsEnabled;
}

void GPS_Disable(void)
{
    if ( gpsEnabled == true )
    {
        //disable UART
        UART_deinitGpsUart();

        PWR_turnOffGpsPowerSupply();

        //reset trackers
        timeToFirstFixMs = 0u;
        gpsLocationFixed = false;
        printedFirstFix = false;
        fakeData = false;

        elogInfo("GPS DISABLED");
        gpsEnabled = false;

        vTaskDelete(xGpsHandle);
    }
}


static void processIncomingByte(char rxByte)
{
    static nmeaParsingState_t nmeaState = NMEA_WAITING_ON_START;
    static nmeaMessage_t * pMsg = NULL;
    static uint8_t subPayloadBytesReceived = 0u;

    //switch how we handle the incoming byte based on state
    switch ( nmeaState )
    {
        case NMEA_WAITING_ON_START:
        {
            pMsg = NULL;

            //if this is the start of a new message, start tracking the msg contents
            if ( MSG_START_INDICATOR == rxByte )
            {
                pMsg = getFreeMessageBufferPointer();

                if ( NULL != pMsg)
                {
                    //change state
                    nmeaState = NMEA_WAITING_ON_END;
                }
            }

            break;
        }
        case NMEA_WAITING_ON_END:
        {
            // wait for the end of the msg
            if ( '\n' == rxByte )
            {
                //put the length of the message into the buffer
                pMsg->len = subPayloadBytesReceived;

                //reset state machine
                subPayloadBytesReceived = 0;
                nmeaState = NMEA_WAITING_ON_START;
            }
            else
            {
                // put the byte received into the message
                pMsg->msg[subPayloadBytesReceived] = rxByte;
                subPayloadBytesReceived++;
            }
            break;
        }
    }
}

static void freeUpMessageBuffer(nmeaMessage_t * pMsg)
{
    //indicate this buffer is free by setting length to 0 and start frame
    pMsg->startFrame = MSG_FREE_INDICATOR;
    pMsg->len = 0;
    memset(pMsg->msg, 0, MAX_SIZE_NMEA_MSG);
}

static nmeaMessage_t * getFreeMessageBufferPointer(void)
{
    // Fand return a free message Tx buffer
    for ( uint8_t idx = 0; idx < MAX_MSGS_RX; idx++ )
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

static void initMessageBuffers(void)
{
    //init all bubbers as free
    for ( uint8_t idx = 0; idx < MAX_MSGS_RX; idx++ )
    {
        incomingMsgs[idx].startFrame = MSG_FREE_INDICATOR;
    }
}

//for now kick off another RX. Will need to handle this properly later on
void GPS_HandleUartErr(void)
{
    UART_recieveDataNonBlocking(GPS_MODULE,(uint8_t*)&rxByte, GPS_RX_SIZE);
}

//This function separates out NMEA , separated sentences
static char *strtok_fr (char *s, char delim, char **save_ptr)
{
    char *tail;
    char c;

    if (s == NULL) {
        s = *save_ptr;
    }
    tail = s;
    if ((c = *tail) == '\0') {
        s = NULL;
    }
    else {
        do {
            if (c == delim) {
                *tail++ = '\0';
                break;
           }
        }while ((c = *++tail) != '\0');
    }
    *save_ptr = tail;
    return s;
}

static char *strtok_f (char *s, char delim)
{
    static char *save_ptr;

    return strtok_fr (s, delim, &save_ptr);
}

static float convertNmeaLatAndLongFields(float value)
{
    double degValue = value / 100;
    int degrees = (int) degValue;

    //get the  mm.mmmm part seperated
    double decMinutesSeconds = ((degValue - degrees)) / .60;

    //dd + mm.mmmm/60
    float finalVal = degrees + decMinutesSeconds;

    return finalVal;
}

//Parse and process a COMPELTE message - for now we only care about 4 types of nmea messages
static void processFullNmeaMsg(nmeaMessage_t *pMsg)
{
    //First get the message ID
    char* msgId = strtok_f(pMsg->msg,',');

    if( strcmp(msgId, "GPRMC") == 0 )
    {
        //ignore for now...dont really need this message
    }
    else if ( strcmp(msgId, "GPGSA") == 0  )
    {

    }
    else if ( strcmp(msgId, "GPGGA") == 0  )
    {
        static uint32_t lastPrinted = 0;

        if( fakeData == true )
        {
            //test data...twisthink's location:
            char str[] = "$GPGGA,180641.0,4247.436861,N,08606.318878,W, 1,04,2.2,201.1,M,-35.0,M,,*63";
            msgId = strtok_f(str,',');
        }

        if ( printWhenMessageReceived == true )
        {
            printWhenMessageReceived = false;
            elogInfo("Received GPGGA message from GPS");
        }

        //seperate out the fields in the message:
        char* utcTime = strtok_f(NULL,',');
        char* rawLatitude = strtok_f(NULL,',');
        char* northSouth = strtok_f(NULL,',');
        char* rawLongitude = strtok_f(NULL,',');
        char* eastWest = strtok_f(NULL,',');
        char* qual = strtok_f(NULL,',');
        char* numberSatellites = strtok_f(NULL,',');
        char* hdopValue = strtok_f(NULL,',');
        char* gga_altitude = strtok_f(NULL,',');

        //if we have actual data in the payload:
        if( *rawLongitude != '\0' && *rawLatitude  != '\0')
        {
            //print out
            elogDebug(ANSI_COLOR_GREEN"Message ID is : %s", msgId);
            elogDebug("UTC Time: %s", utcTime);

            //convert the data from strings to appropriate units:
            float longitude;
            float latitude;
            float altitude;
            uint8_t quality;
            uint8_t satellites;
            float hdop;
            char *ptr;
            uint8_t utcHour;
            uint8_t utcMin;
            char utcBytes[HR_MIN_LEN];
            int decPartOfLatLong;

            //UTC time is received in this format
            //hhmmss.ss
            memcpy(&utcBytes, utcTime, HR_MIN_LEN);
            utcHour = strtoul(utcBytes, &ptr,10);

            //increment string by 2 bytes to get minutes
            utcTime+=HR_MIN_LEN;
            memcpy(utcBytes, utcTime, HR_MIN_LEN);
            utcMin = strtoul(utcBytes, &ptr,10);

            latitude = strtof(rawLatitude, &ptr);
            longitude = strtof(rawLongitude, &ptr);

            //convert the latitude and longitude
            //ddmm.mmmmm is the format that it is currently in. We want it in degrees:
            longitude = convertNmeaLatAndLongFields(longitude);
            latitude = convertNmeaLatAndLongFields(latitude);

            altitude = strtod(gga_altitude, &ptr);
            quality = strtoul(qual, &ptr,10);
            satellites = strtoul(numberSatellites, &ptr,10);
            hdop = strtof(hdopValue, &ptr);

            //latitude is negative if SOUTH
            if( *northSouth == 'S' )
            {
                latitude *= -1;
            }

            //longitude is negative if WEST
            if( *eastWest == 'W' )
            {
                longitude *= -1;
            }

            //check if this is a VALID fix (use configs)
            if ( timeToFirstFixMs >= minTimeGpsOnMs && satellites >= minNumberSatellites &&
                    hdop <= maxHdop && gpsLocationFixed == false )
            {
                elogInfo("Found GPS location");

                decPartOfLatLong = (latitude - (int)latitude)* 10000;

                if ( decPartOfLatLong < 0 )
                {
                    decPartOfLatLong*= -1;
                }

                elogInfo("latitude: %d.%d", (int)latitude, decPartOfLatLong);

                decPartOfLatLong = (longitude - (int)longitude)* 10000;
                if ( decPartOfLatLong < 0 )
                {
                    decPartOfLatLong*= -1;
                }

                elogInfo("longitude: %d.%d", (int)longitude, decPartOfLatLong);

                elogInfo("# satellites: %s", numberSatellites);
                elogInfo("Quality: %s", qual);
                elogInfo("hdop: %s", hdop);
                elogInfo("altitude: %s meters", gga_altitude);
                elogInfo(ANSI_COLOR_CYAN"Time to first fix: %d ms", timeToFirstFixMs);

                gpsLocationFixed = true;

                //populate the GPS struct
                gpsData.altitude = altitude;
                gpsData.fixQuality = quality;
                gpsData.hdopValue = hdop;
                gpsData.latitude = latitude;
                gpsData.longitude = longitude;
                gpsData.measurementTime = timeToFirstFixMs/MS_PER_SEC;
                gpsData.satellitesTracked = satellites;
                gpsData.hours = utcHour;
                gpsData.minutes = utcMin;

                //save to FLASH
                MEM_UpdateGpsCoordinates(gpsData);

                //save state to flash
                MEM_SetGpsFixedFlag(true);

                //pass up to the event manager, disable GPS now
                EVT_indicateGpsFixCompleted(true);

                //turn off GPS and delete this task
                GPS_Disable();
            }

            elogDebug("latitude: %s", rawLatitude);
            elogDebug("longitude: %s", rawLongitude);
            elogDebug("# satellites: %s", numberSatellites);
            elogDebug("Quality: %s", qual);
            elogDebug("hdop: %s", hdop);
            elogDebug("altitude: %s meters", gga_altitude);
        }
        else if ( timeToFirstFixMs - lastPrinted >= printIntervalMs ) //just print the raw contents if no fix has been found and its been 5 seconds
        {
            lastPrinted = timeToFirstFixMs;

            //print out to show progress
            elogInfo("Raw packet contents");
            elogInfo(ANSI_COLOR_YELLOW"Message ID is : %s", msgId);
            elogInfo("UTC Time: %s", utcTime);
            elogInfo("lat: %s", rawLatitude);
            elogInfo("long: %s", rawLongitude);
            elogInfo("# satellites: %s", numberSatellites);
        }
    }
    else if ( strcmp(msgId, "GPGSV") == 0  )
    {
       static uint32_t lastPrintedGsv = 0;

       if ( timeToFirstFixMs - lastPrintedGsv >= printIntervalMs )
       {
           lastPrintedGsv = timeToFirstFixMs;

           char* numMessages = strtok_f(NULL,',');
           char* msgNum = strtok_f(NULL,',');
           char* numSV = strtok_f(NULL,',');

           elogInfo(ANSI_COLOR_MAGENTA"Message ID is : %s", msgId);
           elogInfo("# satellites in view: %s", numSV);
       }
    }
    else
    {
        //Ignore any other message types
    }
}

static void commandHandlerForGps(int argc, char **argv)
{

    if ( argc == ONE_ARGUMENT && 0 == strcmp(argv[FIRST_ARG_IDX], "off") )
    {
        GPS_Disable();
        elogInfo("Gps is OFF");
        timeToFirstFixMs = 0u;
        gpsLocationFixed = false;
        printedFirstFix = false;
        fakeData = false;
        printWhenMessageReceived = true;
    }
    else if ( argc == ONE_ARGUMENT && 0 == strcmp(argv[FIRST_ARG_IDX], "on") )
    {
        //Make sure the device is on..
        GPS_Enable();
        elogInfo("Gps is ON - stay tuned for location info");
    }
    else if  ( argc == ONE_ARGUMENT && 0 == strcmp(argv[FIRST_ARG_IDX], "print") )
    {
        //reset the message received flag so that we print as soon as we receive a new
        //gps message
        printWhenMessageReceived = true;
    }
    else if ( argc == ONE_ARGUMENT && 0 == strcmp(argv[FIRST_ARG_IDX], "fake") )
    {
        //change fake data flag to true
       fakeData = true;
    }
    else if ( argc == ONE_ARGUMENT && 0 == strcmp(argv[FIRST_ARG_IDX], "loc") )
    {
        //print out the location data if we have it
        if ( gpsLocationFixed == true )
        {
            elogInfo("***GPS FIX DATA: ***");
            //add here
        }
        else
        {
            elogInfo("***NO GPS FIX AVAILABLE***");
        }
    }
    else
    {
        elogInfo("invalid cmd");
    }
}
