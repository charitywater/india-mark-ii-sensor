/*
================================================================================================#=
Module:   Memory Map Handler

Description:
    App level handler for the NAND Flash chip and contains memory mapping and image registry info.

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
#include "am-ssm-spi-protocol.h"
#include "messages.pb.h"
#include "APP_NVM_Cfg_Shared.h"
#include "memoryMap.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include <flashHandler.h>
#include "memMapHandler.h"

//bump this if there is a change to mem map in future versions
#define FLASH_VERSION           1
#define MIN_MESSAGE_NUMBER      1
#define MAX_MESSAGE_NUMBER      4294967295lu
#define FLASH_MAGIC_VALUE       0xABCDEABC

static deviceInfo_t amConfigsAndInfo = {};
static flashSectionHeader_t sensorDataHdr = {};
static bool xSensorDataIsFull = false;

static imageRegistry_t appImageRegistry = {};

static SemaphoreHandle_t xMemMapMutex;

// private functions
static bool xIsMagicValuePresent(void);
static void xWriteMagicValue(void);
static void xDefaultAllSections(void);
static bool xReadConfigs(void);
static bool xReadSensorDataHeader(void);
static bool xReadImageRegistry(void);
static bool xVerifyAndCorrectConfigs(deviceInfo_t *info);
static bool xReadCurrentEntry(uint8_t sectionTypeIndex, uint8_t * pBuffer, uint16_t bufferLen);
static uint8_t xComputeChecksum(uint8_t * p_bytes, uint16_t num_bytes);
static bool xVerifyChecksum(uint8_t * p_buf, uint16_t buf_len, uint8_t expected_checksum);
static bool xUpdateCurrentEntry(uint8_t map_index, uint8_t * p_data_to_write, bool bump_addr);
static void xMemCommandHandlerFunction(int argc, char **argv);

const deviceInfo_t amDeviceInfoDefault =
{
    .nvm_version = FLASH_VERSION,                        // Set to APP_NVM_VERSION, which should updated on changes.
    .first_act_date = 0,                                 // Date the device was first activated.
    .recent_act_date = 0,                                // Most-recent date the device was activated.
    .recent_deact_date = 0,                              // Most-recent date the device was deactivated.
    .num_act = 0,                                        // Total number of times the devices as been activated.
    .num_deact = 0,                                      // Total number of times the devices as been deactivated.
    .am_wake_rate_days = 1,                              // Rate (in days) to wake the AM for cloud communication.
    .isStrokeDetectionEnabled = false,                   // Used to include or exclude stroke data in protobuf
    .gpsTimeoutSeconds = 600,                            // Timeout to stop searching for GPS fix
    .maxHop = 300,                                       // 3.0 ->Maximum horizontal dilution of position (HDOP) value to consider a valid fix (scaled by factor of 100)
    .minMeasureTime = 300,                               // Minimum measurement time in seconds to consider the fix valid
    .numOfSatellites = 6,                                // Minimum number of satellites in view to consider the fix valid
    .msgNumber = 1,                                      // Between 1 - 4,294,967,295, start at 1 and increment - unique Id for mqtt messages
    .antennaToUse = (uint8_t)UNKNOWN_ANTENNA,            // The antenna to use during the next cell connection
    .rssiPrimaryAntenna = 0xFF,                          // Last recorded RSSI for the primary antenna when connected to the cell modem
    .rssiSecondaryAntenna = 0xFF,                        // Last recorded RSSI for the secondary antenna when connected to the cell modem
    .epochTimeLastSwitch = 0,                            // Epoch time of the last antenna switch. Try the next antenna after 4 weeks
    .reset_state = (uint8_t)STATE_POR,                   // Unintentional vs intentional reset state
    .resetsSinceLpMode = 0,                              // Tracks how many times we have reset since entering LP mode
    .unexpectedResetCounter = 0,                         // Lifetime counter of unexpected resets - reported in cloud msg
    .timeLastUnexpectedReset = 0,                        // Epoch time of the last unexpected reset
    .gpsLocationFixed = 0,                               // Do we have GPS coordinates?
    .gpsLocationSentToCloud = 0,                         // Have we sent the coordinates to the cloud?
    .gpsRetries = 0,                                     // Number of times we have retried to obtain GPS fix
    .gpsCoordinates = {},                                // GPS coordinates
    .manufacturingComplete = false,                      // Set to true via the CLI during manufacturing
    .manfCompleteSecsToWait = DEFAULT_MFG_COMPLETE_WAIT_SECS,  // Seconds to wait after the manufacturingComplete flag has been set to connect to cell
    .sensorDataBufferFull = false,                       // Track state of the LIFO of sensor data entries
};


//Default image registry settings - No images in FLASH
const imageRegistry_t imageRegistryDefault =
{
    .imageAoperationalState = (uint8_t)OP_UNKNOWN,
    .imageBoperationalState = (uint8_t)OP_UNKNOWN,
    .loadedImage = (uint8_t)UNKNOWN_SLOT,
    .primaryImage = (uint8_t)UNKNOWN_SLOT,
    .imageAfwVersionMajor = 0,
    .imageAfwVersionMinor = 0,
    .imageAfwVersionBuild = 0,
    .imageBfwVersionMajor = 0,
    .imageBfwVersionMinor = 0,
    .imageBfwVersionBuild = 0,
};

//Layout of the flash memory, default values, etc
const flashSectionMap_t Section_Map[APP_MEM_NUM_SECTIONS] =
{
    // Device info/configs section
    {
        .type = SECTION_DEVICE_INFO,
        .start_addr = APP_MEM_ADR_CONFIG_START,
        .end_addr = APP_MEM_ADR_CONFIG_END,
        .is_array = false,
        .entry_len = sizeof(deviceInfo_t),
        .default_num_entries = 1,
        .p_default_values = (void *)&amDeviceInfoDefault,
    },

    // Sensor data section.
    {
        .type = SECTION_DATA,
        .start_addr = APP_MEM_ADR_SENSOR_DATA_LOGS_START,
        .end_addr = APP_MEM_ADR_SENSOR_DATA_LOGS_END,
        .is_array = true,
        .entry_len = sizeof(APP_NVM_SENSOR_DATA_WITH_HEADER_T),
        .default_num_entries = 0,
        .p_default_values = NULL,
    },

    // image registry section.
    {
        .type = SECTION_IMAGE_REGISTRY,
        .start_addr = APP_MEM_FW_REGISTRY_START,
        .end_addr = APP_MEM_FW_REGISTRY_END,
        .is_array = false,
        .entry_len = sizeof(imageRegistry_t),
        .default_num_entries = 1,
        .p_default_values = (void *)&imageRegistryDefault,
    },

    // AM IMAGE A
    {
        .type = SECTION_AM_APP_A,
        .start_addr = APP_MEM_ADR_FW_APPLICATION_AM_A_START,
        .end_addr = APP_MEM_ADR_FW_APPLICATION_AM_A_END,
        .is_array = false,
        .entry_len = 0,
        .default_num_entries = 0,
        .p_default_values = NULL,
    },

    // SSM IMAGE A
    {
        .type = SECTION_SSM_APP_A,
        .start_addr = APP_MEM_ADR_FW_APPLICATION_SSM_A_START,
        .end_addr = APP_MEM_ADR_FW_APPLICATION_SSM_A_END,
        .is_array = false,
        .entry_len = 0,
        .default_num_entries = 0,
        .p_default_values = NULL,
    },

    // AM IMAGE B
    {
        .type = SECTION_AM_APP_B,
        .start_addr = APP_MEM_ADR_FW_APPLICATION_AM_B_START,
        .end_addr = APP_MEM_ADR_FW_APPLICATION_AM_B_END,
        .is_array = false,
        .entry_len = 0,
        .default_num_entries = 0,
        .p_default_values = NULL,
    },

    // SSM IMAGE B
    {
        .type = SECTION_SSM_APP_B,
        .start_addr = APP_MEM_ADR_FW_APPLICATION_SSM_B_START,
        .end_addr = APP_MEM_ADR_FW_APPLICATION_SSM_B_END,
        .is_array = true,
        .entry_len = 0,
        .default_num_entries = 0,
        .p_default_values = NULL,
    },
};

bool MEM_init(void)
{
    bool initialized = false;

    /* register a command handler cb function */
    CLI_Command_Handler_s memCmdHandler;

    memCmdHandler.ptrFunction = &xMemCommandHandlerFunction;
    memCmdHandler.cmdString   = "mem";
    memCmdHandler.usageString = "\n\r\tdefault";
    CLI_registerThisCommandHandler(&memCmdHandler);

    /* create a mutex */
    xMemMapMutex = xSemaphoreCreateMutex();

    //first read out the magic value in flash
    if ( xIsMagicValuePresent() == false )
    {
        //default everything and then write the magic value
        xDefaultAllSections();

        initialized = true;

        elogInfo("Defaulted all sections");
    }
    else
    {
        initialized = xReadConfigs();

        if ( initialized == true )
        {
            elogInfo("initialized configs");
        }
        else
        {
            elogNotice(ANSI_COLOR_YELLOW "Failed to init configs - using default values");
        }

        initialized = xReadSensorDataHeader();

        if ( initialized == true )
        {
            elogInfo("initialized sensor data");
        }
        else
        {
            elogNotice(ANSI_COLOR_YELLOW "Failed to init sensor data - using default values");
        }

        initialized = xReadImageRegistry();

        if ( initialized == true )
        {
            elogInfo("initialized image registry");
        }
        else
        {
            elogNotice(ANSI_COLOR_YELLOW "Failed to init image registry - using default values");
        }
    }

    return initialized;
}

uint8_t MEM_getResetsSinceLastLpMode(void)
{
    return amConfigsAndInfo.resetsSinceLpMode;
}

void MEM_setResetsSinceLastLpMode(uint8_t resets)
{
    amConfigsAndInfo.resetsSinceLpMode = resets;
    amConfigsAndInfo.checksum = xComputeChecksum((uint8_t*)&amConfigsAndInfo, sizeof(amConfigsAndInfo)-1);
    xUpdateCurrentEntry((uint8_t)SECTION_DEVICE_INFO,  (uint8_t *) &amConfigsAndInfo, false);
}

uint32_t MEM_getUnexpectedResetCount(void)
{
    return amConfigsAndInfo.unexpectedResetCounter;
}

uint32_t MEM_getTimestampLastUnexpectedReset(void)
{
    return amConfigsAndInfo.timeLastUnexpectedReset;
}

void MEM_incrementUnexpectedResetCount(uint32_t timestampOfReset)
{
    amConfigsAndInfo.unexpectedResetCounter++;
    amConfigsAndInfo.timeLastUnexpectedReset = timestampOfReset;

    amConfigsAndInfo.checksum = xComputeChecksum((uint8_t*)&amConfigsAndInfo, sizeof(amConfigsAndInfo)-1);
    xUpdateCurrentEntry((uint8_t)SECTION_DEVICE_INFO,  (uint8_t *) &amConfigsAndInfo, false);
}

uint32_t MEM_getActivationDate(void)
{
    return amConfigsAndInfo.recent_act_date;
}

uint32_t MEM_getSecondsToWaitForMfgComplete(void)
{
    return amConfigsAndInfo.manfCompleteSecsToWait;
}

bool MEM_getMfgCompleteFlag(void)
{
    return amConfigsAndInfo.manufacturingComplete;
}

void MEM_setSecondsToWaitForMfgComplete(uint32_t seconds)
{
    if ( seconds >= MIN_MFG_COMPLETE_WAIT_SECS && seconds <= MAX_MFG_COMPLETE_WAIT_SECS )
    {
        amConfigsAndInfo.manfCompleteSecsToWait = seconds;

        amConfigsAndInfo.checksum = xComputeChecksum((uint8_t*)&amConfigsAndInfo, sizeof(amConfigsAndInfo)-1);
        xUpdateCurrentEntry((uint8_t)SECTION_DEVICE_INFO,  (uint8_t *) &amConfigsAndInfo, false);
    }
    else
    {
        elogError("Input outside of range");
    }
}

void MEM_setMfgCompleteFlag(bool isMfgComplete)
{
    amConfigsAndInfo.manufacturingComplete = isMfgComplete;

    amConfigsAndInfo.checksum = xComputeChecksum((uint8_t*)&amConfigsAndInfo, sizeof(amConfigsAndInfo)-1);
    xUpdateCurrentEntry((uint8_t)SECTION_DEVICE_INFO,  (uint8_t *) &amConfigsAndInfo, false);
}

void MEM_writeActivationDate(uint32_t date)
{
    amConfigsAndInfo.recent_act_date = date;
    amConfigsAndInfo.checksum = xComputeChecksum((uint8_t*)&amConfigsAndInfo, sizeof(amConfigsAndInfo)-1);
    xUpdateCurrentEntry((uint8_t)SECTION_DEVICE_INFO,  (uint8_t *) &amConfigsAndInfo, false);
}

uint32_t MEM_getDeactivationDate(void)
{
    return amConfigsAndInfo.recent_deact_date;
}

void MEM_writeDeactivationDate(uint32_t date)
{
    amConfigsAndInfo.recent_deact_date = date;
    amConfigsAndInfo.checksum = xComputeChecksum((uint8_t*)&amConfigsAndInfo, sizeof(amConfigsAndInfo)-1);
    xUpdateCurrentEntry((uint8_t)SECTION_DEVICE_INFO,  (uint8_t *) &amConfigsAndInfo, false);
}

uint16_t MEM_getAmWakeRate(void)
{
    return amConfigsAndInfo.am_wake_rate_days;
}

bool MEM_writeAmWakeRate(uint16_t days)
{
    bool stat = false;

    if ( days <= MAX_WAKE_AM_RATE_DAYS && days >= MIN_WAKE_AM_RATE_DAYS)
    {
        amConfigsAndInfo.am_wake_rate_days = days;
        amConfigsAndInfo.checksum = xComputeChecksum((uint8_t*)&amConfigsAndInfo, sizeof(amConfigsAndInfo)-1);
        stat = xUpdateCurrentEntry((uint8_t)SECTION_DEVICE_INFO,  (uint8_t *) &amConfigsAndInfo, false);
    }

    return stat;
}

bool MEM_getIsStrokeDetectionEnabledFlag(void)
{
    return amConfigsAndInfo.isStrokeDetectionEnabled;
}

bool MEM_writeStrokeDetectionEnabledFlag(bool isStrokeDetEnabled)
{
    bool stat = false;

    //update structure and save to FLASH
    amConfigsAndInfo.isStrokeDetectionEnabled = isStrokeDetEnabled;
    amConfigsAndInfo.checksum = xComputeChecksum((uint8_t*)&amConfigsAndInfo, sizeof(amConfigsAndInfo)-1);

    stat = xUpdateCurrentEntry((uint8_t)SECTION_DEVICE_INFO,  (uint8_t *) &amConfigsAndInfo, false);

    return stat;
}

void MEM_setAntennaToUse(antenna_t antenna)
{
    if (antenna == PRIMARY || antenna == SECONDARY || antenna == UNKNOWN_ANTENNA )
    {
        amConfigsAndInfo.antennaToUse = antenna;

        amConfigsAndInfo.checksum = xComputeChecksum((uint8_t*)&amConfigsAndInfo, sizeof(amConfigsAndInfo)-1);
        xUpdateCurrentEntry((uint8_t)SECTION_DEVICE_INFO,  (uint8_t *) &amConfigsAndInfo, false);
    }
    else
    {
        elogError("invalid antenna");
    }
}

antenna_t MEM_getAntennaToUse(void)
{
    return amConfigsAndInfo.antennaToUse;
}

void MEM_setPrimaryAntennaRssi(uint8_t rssi)
{
    amConfigsAndInfo.rssiPrimaryAntenna = rssi;

    amConfigsAndInfo.checksum = xComputeChecksum((uint8_t*)&amConfigsAndInfo, sizeof(amConfigsAndInfo)-1);
    xUpdateCurrentEntry((uint8_t)SECTION_DEVICE_INFO,  (uint8_t *) &amConfigsAndInfo, false);
}

uint8_t MEM_getPrimaryAntennaRssi(void)
{
    return amConfigsAndInfo.rssiPrimaryAntenna;
}

void MEM_setSecondaryAntennaRssi(uint8_t rssi)
{
    amConfigsAndInfo.rssiSecondaryAntenna = rssi;

    amConfigsAndInfo.checksum = xComputeChecksum((uint8_t*)&amConfigsAndInfo, sizeof(amConfigsAndInfo)-1);
    xUpdateCurrentEntry((uint8_t)SECTION_DEVICE_INFO,  (uint8_t *) &amConfigsAndInfo, false);
}

void MEM_setEpochTimeLastAntennaSwitch(uint32_t epochTime)
{
    amConfigsAndInfo.epochTimeLastSwitch = epochTime;

    amConfigsAndInfo.checksum = xComputeChecksum((uint8_t*)&amConfigsAndInfo, sizeof(amConfigsAndInfo)-1);
    xUpdateCurrentEntry((uint8_t)SECTION_DEVICE_INFO,  (uint8_t *) &amConfigsAndInfo, false);
}

uint32_t MEM_getEpochTimeLastAntennaSwitch(void)
{
    return amConfigsAndInfo.epochTimeLastSwitch;
}

uint8_t MEM_getSecondaryAntennaRssi(void)
{
    return amConfigsAndInfo.rssiSecondaryAntenna;
}

uint32_t MEM_getGpsTimeoutSeconds(void)
{
    return amConfigsAndInfo.gpsTimeoutSeconds;
}

uint32_t MEM_getGpsMaxdHop(void)
{
    return amConfigsAndInfo.maxHop;
}

uint32_t MEM_getGpsMinMeasTimeSec(void)
{
    return amConfigsAndInfo.minMeasureTime;
}

uint32_t MEM_getGpsNumSatellites(void)
{
    return amConfigsAndInfo.numOfSatellites;
}

uint32_t MEM_getMsgNumber(void)
{
    return amConfigsAndInfo.msgNumber;
}

bool MEM_updateMsgNumber(void)
{
    bool stat = false;

    if (amConfigsAndInfo.msgNumber == MAX_MESSAGE_NUMBER)
    {
        amConfigsAndInfo.msgNumber = MIN_MESSAGE_NUMBER;
    }
    else
    {
        amConfigsAndInfo.msgNumber++;
    }

    amConfigsAndInfo.checksum = xComputeChecksum((uint8_t*)&amConfigsAndInfo, sizeof(amConfigsAndInfo)-1);
    stat = xUpdateCurrentEntry((uint8_t)SECTION_DEVICE_INFO,  (uint8_t *) &amConfigsAndInfo, false);

    return stat;
}

bool  MEM_writeGpsConfigs(uint32_t gpsTimeoutSecs, uint32_t maxdHop, uint32_t minMeasTimeSecs, uint32_t numSat)
{
    bool stat = false;

    //update the configs stored in flash with the new params:
    amConfigsAndInfo.numOfSatellites = numSat;
    amConfigsAndInfo.gpsTimeoutSeconds = gpsTimeoutSecs;
    amConfigsAndInfo.maxHop = maxdHop;
    amConfigsAndInfo.minMeasureTime = minMeasTimeSecs;

    amConfigsAndInfo.checksum = xComputeChecksum((uint8_t*)&amConfigsAndInfo, sizeof(amConfigsAndInfo)-1);
    stat = xUpdateCurrentEntry((uint8_t)SECTION_DEVICE_INFO,  (uint8_t *) &amConfigsAndInfo, false);

    return stat;
}

bool MEM_UpdateGpsCoordinates(GpsMessage latestGpsLocation)
{
    bool stat = false;

    amConfigsAndInfo.gpsCoordinates = latestGpsLocation;

    amConfigsAndInfo.checksum = xComputeChecksum((uint8_t*)&amConfigsAndInfo, sizeof(amConfigsAndInfo)-1);
    stat = xUpdateCurrentEntry((uint8_t)SECTION_DEVICE_INFO,  (uint8_t *) &amConfigsAndInfo, false);

    return stat;
}

GpsMessage MEM_GetGpsCoordinates(void)
{
    return amConfigsAndInfo.gpsCoordinates;
}

bool MEM_GetGpsFixedFlag(void)
{
    return amConfigsAndInfo.gpsLocationFixed;
}

bool MEM_SetGpsFixedFlag(bool fixPassed)
{
    bool stat = false;

    amConfigsAndInfo.gpsLocationFixed = fixPassed;

    amConfigsAndInfo.checksum = xComputeChecksum((uint8_t*)&amConfigsAndInfo, sizeof(amConfigsAndInfo)-1);
    stat = xUpdateCurrentEntry((uint8_t)SECTION_DEVICE_INFO,  (uint8_t *) &amConfigsAndInfo, false);

    return stat;
}

bool MEM_GetGpsSentToCloud(void)
{
    return amConfigsAndInfo.gpsLocationSentToCloud;
}

bool MEM_SetGpsSentToCloud(bool sent)
{
    bool stat = false;

    amConfigsAndInfo.gpsLocationSentToCloud = sent;

    amConfigsAndInfo.checksum = xComputeChecksum((uint8_t*)&amConfigsAndInfo, sizeof(amConfigsAndInfo)-1);
    stat = xUpdateCurrentEntry((uint8_t)SECTION_DEVICE_INFO,  (uint8_t *) &amConfigsAndInfo, false);

    return stat;
}

uint8_t MEM_GetGpsNumRetries(void)
{
    return amConfigsAndInfo.gpsRetries;
}

bool MEM_SetGpsNumRetries(uint8_t tries)
{
    bool stat = false;

    amConfigsAndInfo.gpsRetries = tries;

    amConfigsAndInfo.checksum = xComputeChecksum((uint8_t*)&amConfigsAndInfo, sizeof(amConfigsAndInfo)-1);
    stat = xUpdateCurrentEntry((uint8_t)SECTION_DEVICE_INFO,  (uint8_t *) &amConfigsAndInfo, false);

    return stat;
}

int16_t MEM_getNumSensorDataEntries(void)
{
    // First read the header info to get entry len.
    if (FLASH_read(Section_Map[SECTION_DATA].start_addr, (uint8_t *)&sensorDataHdr, sizeof(flashSectionHeader_t)) == FLASH_SUCCESS)
    {
        return sensorDataHdr.lifoCount;
    }
    else
    {
        return -1;
    }
}

extern bool MEM_writeSensorDataLog(APP_NVM_SENSOR_DATA_WITH_HEADER_T *pSensorData)
{
    bool stat = false;

    pSensorData->checksum = xComputeChecksum((uint8_t*)pSensorData, sizeof(APP_NVM_SENSOR_DATA_WITH_HEADER_T)-1);
    stat = xUpdateCurrentEntry((uint8_t)SECTION_DATA, (uint8_t *) pSensorData, true);

    if ( xSensorDataIsFull == true && stat == true )
    {
        //write this to flash as well
        amConfigsAndInfo.sensorDataBufferFull = xSensorDataIsFull;

        amConfigsAndInfo.checksum = xComputeChecksum((uint8_t*)&amConfigsAndInfo, sizeof(amConfigsAndInfo)-1);
        stat = xUpdateCurrentEntry((uint8_t)SECTION_DEVICE_INFO,  (uint8_t *) &amConfigsAndInfo, false);
    }

    return stat;
}

extern bool MEM_getSensorDataLog(APP_NVM_SENSOR_DATA_WITH_HEADER_T *pSensorData)
{
    bool status = false;
    flashErr_t err = FLASH_GEN_ERROR;
    uint8_t pos = 0;
    uint32_t latestSensorDataEntry;

    // Update position to read from
    if ( MEM_getNumSensorDataEntries() <= 0 )
    {
        return false;
    }

    //read from the most recent position in the buffer
    pos = (sensorDataHdr.head + MAX_SENSOR_DATA_LOGS - 1) % MAX_SENSOR_DATA_LOGS;

    // Read from correct addr in section data
    latestSensorDataEntry = Section_Map[SECTION_DATA].start_addr + sizeof(flashSectionHeader_t) + (pos * sensorDataHdr.entry_len);

    err = FLASH_read(latestSensorDataEntry, (uint8_t *) pSensorData, (Section_Map[SECTION_DATA].entry_len));

    if (err == FLASH_SUCCESS)
    {
        status = xVerifyChecksum((uint8_t *) pSensorData, sizeof(APP_NVM_SENSOR_DATA_WITH_HEADER_T) - 1, pSensorData->checksum);

        if (status == false)
        {
            elogError("Invalid checksum for sensor data entry in flash block");

           //default the section
           MEM_defaultSection(SECTION_DATA);
        }
    }

    return status;
}

extern bool MEM_updateSensorDataHeadAndLifoCount(void)
{
    bool status = false;
    flashErr_t err = FLASH_GEN_ERROR;

    if ( xSensorDataIsFull == true )
    {
        //reset full flag since we just read one out
        xSensorDataIsFull = false;

        //write this to flash as well
        amConfigsAndInfo.sensorDataBufferFull = xSensorDataIsFull;

        amConfigsAndInfo.checksum = xComputeChecksum((uint8_t*)&amConfigsAndInfo, sizeof(amConfigsAndInfo)-1);
        status = xUpdateCurrentEntry((uint8_t)SECTION_DEVICE_INFO,  (uint8_t *) &amConfigsAndInfo, false);
    }

    //decrement head for the next read/write since we just popped one off the LIFO
    sensorDataHdr.head = (sensorDataHdr.head + MAX_SENSOR_DATA_LOGS - 1) % MAX_SENSOR_DATA_LOGS;
    sensorDataHdr.lifoCount--;
    sensorDataHdr.current_addr = (Section_Map[SECTION_DATA].start_addr + sizeof(flashSectionHeader_t)) + ( sensorDataHdr.head * sizeof(APP_NVM_SENSOR_DATA_WITH_HEADER_T));

    sensorDataHdr.checksum = xComputeChecksum((uint8_t *)&sensorDataHdr, (sizeof(flashSectionHeader_t) - 1)); // Compute a checksum, not including the checksum byte itself.
    err = FLASH_write(Section_Map[SECTION_DATA].start_addr, (uint8_t *) &sensorDataHdr, sizeof(flashSectionHeader_t));

    if (err == FLASH_SUCCESS)
        status = true;

    elogInfo("Updated Sensor Data Head Pointer");

    return status;
}

bool MEM_defaultSection(uint8_t section)
{
    uint32_t addr;
    flashSectionHeader_t hdr;
    uint8_t checksum = 0;
    bool status = false;
    flashErr_t err = FLASH_SUCCESS;

    elogInfo("Defaulting section %d", section);

    if ( section <= APP_MEM_NUM_SECTIONS)
    {
        // For each entry specified by the number of entries in the section map, fill in the default values and checksum.
        for (uint8_t entry_index = 0; entry_index < Section_Map[section].default_num_entries; entry_index++)
        {

            addr = (Section_Map[section].start_addr +                     // start at the start address
                    sizeof(flashSectionHeader_t) +                         // plus room for the header.
                    (Section_Map[section].entry_len * entry_index));      // plus any entries already defaulted.

            err = FLASH_write(addr, (uint8_t *) Section_Map[section].p_default_values, (Section_Map[section].entry_len - 1));

            // And append the checksum.
            checksum = xComputeChecksum(Section_Map[section].p_default_values, (Section_Map[section].entry_len - 1));

            if ( err == FLASH_SUCCESS )
            {
                err = FLASH_write((addr + (Section_Map[section].entry_len - 1)), &checksum, sizeof(uint8_t));
            }
        }

        if ( err == FLASH_SUCCESS )
        {
            // Then default the header.
            hdr.type = Section_Map[section].type;
            hdr.head = Section_Map[section].default_num_entries;
            hdr.lifoCount = Section_Map[section].default_num_entries;
            hdr.entry_len = Section_Map[section].entry_len;
            hdr.current_addr = (Section_Map[section].start_addr + sizeof(flashSectionHeader_t));  // The current address (to be written next) begins after the header.

            // If the section is an array then set the current address back to the initial starting point
            if (Section_Map[section].is_array == true)
            {
                hdr.current_addr += (Section_Map[section].default_num_entries * Section_Map[section].entry_len);
            }

            hdr.checksum = xComputeChecksum((uint8_t *)&hdr, sizeof(hdr) -1 );

            err = FLASH_write( Section_Map[section].start_addr, (uint8_t *) &hdr,  sizeof(flashSectionHeader_t));
        }
    }

    if (err == FLASH_SUCCESS)
        status = true;

    return status;
}


static bool xReadConfigs(void)
{
    bool stat;

    stat = xReadCurrentEntry((uint8_t)SECTION_DEVICE_INFO, (uint8_t *) &amConfigsAndInfo, sizeof(amConfigsAndInfo));

    stat = xVerifyAndCorrectConfigs(&amConfigsAndInfo);

    if ( stat == false)
    {
        elogNotice(ANSI_COLOR_YELLOW "Configs read were invalid. Reset to use defaults.");
    }

    //add one to the # resets since we only call this function on power up
    //this counter gets cleared when we go into LP mode
    amConfigsAndInfo.resetsSinceLpMode++;
    MEM_setResetsSinceLastLpMode(amConfigsAndInfo.resetsSinceLpMode);

    return stat;
}

static bool xIsMagicValuePresent(void)
{
    bool magicValuePresent = false;
    uint32_t valueReadFromFlash = 0u;

    FLASH_read(APP_MEM_ADR_MAGIC_VALUE, (uint8_t*)&valueReadFromFlash, sizeof (uint32_t));

    if ( valueReadFromFlash == FLASH_MAGIC_VALUE )
    {
       elogInfo("Magic value present");
       magicValuePresent = true;
    }
    else
    {
       elogNotice("No magic value!");
    }

    return magicValuePresent;
}


static void xWriteMagicValue(void)
{
    uint32_t magicVal = FLASH_MAGIC_VALUE;

    FLASH_write(APP_MEM_ADR_MAGIC_VALUE, (uint8_t*)&magicVal, sizeof (uint32_t));
}

static void xDefaultAllSections(void)
{
    //default sections
    MEM_defaultSection(SECTION_DEVICE_INFO);
    MEM_defaultSection(SECTION_DATA);
    MEM_defaultSection(SECTION_IMAGE_REGISTRY);

    //finally, write the magic value to indicate that flash has been initialized
    xWriteMagicValue();
}

static bool xReadSensorDataHeader(void)
{
    bool stat = false;
    flashErr_t flashReadResult = FLASH_GEN_ERROR;

    //read header
    flashReadResult = FLASH_read(Section_Map[SECTION_DATA].start_addr, (uint8_t *)&sensorDataHdr, sizeof(flashSectionHeader_t));

    if ( flashReadResult == FLASH_SUCCESS )
    {
        stat = xVerifyChecksum((uint8_t*)&sensorDataHdr, sizeof(flashSectionHeader_t)-1, sensorDataHdr.checksum);
    }
    else
    {
        elogInfo("Sensor data header invalid. Reset to use defaults.");

        //default the section
        MEM_defaultSection(SECTION_DATA);
    }


    if (stat == false)
    {
       elogDebug("Invalid checksum in sensor data flash section header");

       //default the section
       MEM_defaultSection(SECTION_DATA);
    }

    //update flag
    xSensorDataIsFull  = amConfigsAndInfo.sensorDataBufferFull;

    return stat;
}


static bool xReadImageRegistry(void)
{
    bool stat;

    stat = xReadCurrentEntry((uint8_t)SECTION_IMAGE_REGISTRY, (uint8_t *) &appImageRegistry, sizeof(appImageRegistry));

    if (stat == true)
    {
        //we could read the image registry - now validate the cs
        if (xVerifyChecksum((uint8_t*)&appImageRegistry, sizeof(imageRegistry_t)-1, appImageRegistry.checksum) == false)
        {
            elogError("Invalid checksum in image registry flash block");
            stat = false;
        }

        //also validate the values in the image slots
       if ( appImageRegistry.loadedImage != A && appImageRegistry.loadedImage != B && appImageRegistry.loadedImage != UNKNOWN_SLOT  )
       {
           //invalid values
           elogError("Invalid value in the loaded image section");
           stat = false;
       }

       if (appImageRegistry.primaryImage != A && appImageRegistry.primaryImage != B && appImageRegistry.primaryImage != UNKNOWN_SLOT)
       {
           //invalid values
           elogError("Invalid value in the primary image section");
           stat = false;
       }
    }

    if ( stat == false)
    {
        elogNotice(ANSI_COLOR_YELLOW "Image registry was invalid. Attempting to reset to use defaults.");
        MEM_defaultSection(SECTION_IMAGE_REGISTRY);

        //set RAM copy to the default copy
        appImageRegistry = imageRegistryDefault;
    }

    return stat;
}


imageSlotTypes_t MEM_getLoadedImage(void)
{
    return appImageRegistry.loadedImage;
}

bool MEM_setLoadedImage(imageSlotTypes_t imageSlot)
{
    bool stat;

    appImageRegistry.loadedImage = imageSlot;

    appImageRegistry.checksum = xComputeChecksum((uint8_t*)&appImageRegistry, sizeof(appImageRegistry)-1);
    stat = xUpdateCurrentEntry((uint8_t)SECTION_IMAGE_REGISTRY,  (uint8_t *) &appImageRegistry, false);

    return stat;
}


imageOperationalState_t MEM_getImageOpState(imageSlotTypes_t slot)
{
    imageOperationalState_t opState = OP_UNKNOWN;

    if ( slot == A )
    {
        opState = appImageRegistry.imageAoperationalState;
    }
    else if ( slot == B )
    {
        opState = appImageRegistry.imageBoperationalState;
    }
    else
    {
        elogNotice("Slot is not stored in flash");
    }

    return opState;
}

bool MEM_setImageOpState(imageOperationalState_t opState, imageSlotTypes_t slot)
{
    bool res = false;
    if ( slot == A )
    {
        res = MEM_setImageAoperationalState(opState);
    }
    else if ( slot == B )
    {
        res = MEM_setImageBoperationalState(opState);
    }
    else
    {
        elogError("unknown slot");
    }

    return res;
}

imageSlotTypes_t MEM_getSlotWithThisVersion(uint32_t major, uint32_t minor, uint32_t build)
{
    imageSlotTypes_t slot = UNKNOWN_SLOT;

    if ( appImageRegistry.imageAfwVersionMajor == major &&
         appImageRegistry.imageAfwVersionMinor == minor &&
         appImageRegistry.imageAfwVersionBuild == build )
    {
        slot = A;
    }

    else if ( appImageRegistry.imageBfwVersionMajor == major &&
              appImageRegistry.imageBfwVersionMinor == minor &&
              appImageRegistry.imageBfwVersionBuild == build )
    {
        slot = B;
    }
    else
    {
        elogInfo("this version is not stored in external flash. Probably should store a copy");
    }

    return slot;
}


imageSlotTypes_t MEM_getAlternateSlot(imageSlotTypes_t slotToGetAlternateOf)
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

imageSlotTypes_t MEM_getPrimaryImage(void)
{
    return appImageRegistry.primaryImage;
}

bool MEM_setPrimaryImage(imageSlotTypes_t slot)
{
    bool stat;

    appImageRegistry.primaryImage = slot;

    appImageRegistry.checksum = xComputeChecksum((uint8_t*)&appImageRegistry, sizeof(appImageRegistry)-1);
    stat = xUpdateCurrentEntry((uint8_t)SECTION_IMAGE_REGISTRY,  (uint8_t *) &appImageRegistry, false);

    return stat;
}

bool MEM_setImageAoperationalState(imageOperationalState_t state)
{
    bool stat;

    appImageRegistry.imageAoperationalState = state;

    appImageRegistry.checksum = xComputeChecksum((uint8_t*)&appImageRegistry, sizeof(appImageRegistry)-1);
    stat = xUpdateCurrentEntry((uint8_t)SECTION_IMAGE_REGISTRY,  (uint8_t *) &appImageRegistry, false);

    return stat;
}

bool MEM_setImageBoperationalState(imageOperationalState_t state)
{
    bool stat;

    appImageRegistry.imageBoperationalState = state;

    appImageRegistry.checksum = xComputeChecksum((uint8_t*)&appImageRegistry, sizeof(appImageRegistry)-1);
    stat = xUpdateCurrentEntry((uint8_t)SECTION_IMAGE_REGISTRY,  (uint8_t *) &appImageRegistry, false);

    return stat;
}

bool MEM_setImageAversion(uint32_t major, uint32_t minor, uint32_t build)
{
    bool stat;

    appImageRegistry.imageAfwVersionMajor = major;
    appImageRegistry.imageAfwVersionMinor = minor;
    appImageRegistry.imageAfwVersionBuild = build;

    appImageRegistry.checksum = xComputeChecksum((uint8_t*)&appImageRegistry, sizeof(appImageRegistry)-1);
    stat = xUpdateCurrentEntry((uint8_t)SECTION_IMAGE_REGISTRY,  (uint8_t *) &appImageRegistry, false);

    return stat;
}

bool MEM_setImageBversion(uint32_t major, uint32_t minor, uint32_t build)
{
    bool stat;

    appImageRegistry.imageBfwVersionMajor = major;
    appImageRegistry.imageBfwVersionMinor = minor;
    appImageRegistry.imageBfwVersionBuild = build;

    appImageRegistry.checksum = xComputeChecksum((uint8_t*)&appImageRegistry, sizeof(appImageRegistry)-1);
    stat = xUpdateCurrentEntry((uint8_t)SECTION_IMAGE_REGISTRY,  (uint8_t *) &appImageRegistry, false);

    return stat;
}

static bool xVerifyAndCorrectConfigs(deviceInfo_t *info)
{
    bool valid = true;

    //verify the configs/saved states from flash. if invalid, set to the default so that
    //the app never uses garbage info...
    //but return that they failed so an error can be logged.

    //wake rate - the SSM may need to known transmission rate:
    if (info->am_wake_rate_days > MAX_WAKE_AM_RATE_DAYS|| info->am_wake_rate_days < MIN_WAKE_AM_RATE_DAYS)
    {

        info->am_wake_rate_days = DEFAULT_WAKE_AM_RATE_DAYS;
        valid = false;
    }

    if ( info->manfCompleteSecsToWait > MAX_MFG_COMPLETE_WAIT_SECS || info->manfCompleteSecsToWait < MIN_MFG_COMPLETE_WAIT_SECS )
    {
        info->manfCompleteSecsToWait = DEFAULT_MFG_COMPLETE_WAIT_SECS;
        valid = false;
    }

    //calc expected checksum
    if (xVerifyChecksum((uint8_t*)info, sizeof(deviceInfo_t)-1, info->checksum) == false)
    {
        elogNotice(ANSI_COLOR_YELLOW "Invalid checksum in flash");

        //default the section for the next power on
        MEM_defaultSection(SECTION_DEVICE_INFO);

        //now set the RAM copy to the default copy in case flash is corrupt
        amConfigsAndInfo = amDeviceInfoDefault;

        valid = false;
    }

    return valid;
}

// Read the values for the current section entry.
static bool xReadCurrentEntry(uint8_t sectionTypeIndex, uint8_t * pBuffer, uint16_t bufferLen)
{
    flashSectionHeader_t hdr;
    flashErr_t err = FLASH_GEN_ERROR;
    bool status = false;

    if (pBuffer == NULL) return status;
    if (sectionTypeIndex > (APP_MEM_NUM_SECTIONS - 1)) return status;
    if (bufferLen < Section_Map[sectionTypeIndex].entry_len) return status;


    if( xSemaphoreTake(xMemMapMutex, ( TickType_t ) 6000) == pdTRUE )
    {
        // Read the header info
        err = FLASH_read(Section_Map[sectionTypeIndex].start_addr, (uint8_t *)&hdr, sizeof(flashSectionHeader_t));

        if (err == FLASH_SUCCESS)
        {
            //then get the entry value
            err = FLASH_read(hdr.current_addr, (uint8_t *) pBuffer, (Section_Map[sectionTypeIndex].entry_len));
        }

        /* Return mutex */
        xSemaphoreGive(xMemMapMutex);
    }
    else
    {
        elogError("Failed to take mutex");
    }

    if (err == FLASH_SUCCESS)
        status = true;

    return status;
}


// Update the entry pointed to by the current address in the header.  If bump_addr is true
// then update current address to point to the next entry.
static bool xUpdateCurrentEntry(uint8_t map_index, uint8_t * p_data_to_write, bool bump_addr)
{
    flashSectionHeader_t hdr;

    bool status = false;
    flashErr_t err = FLASH_GEN_ERROR;
    uint32_t addressToStoreData = 0u;

    if (p_data_to_write == NULL) return status;
    if (map_index > (APP_MEM_NUM_SECTIONS - 1)) return status;

    if( xSemaphoreTake(xMemMapMutex, ( TickType_t ) 6000) == pdTRUE )
    {
        // First read the header info to get entry len.
        err = FLASH_read(Section_Map[map_index].start_addr, (uint8_t *)&hdr, sizeof(flashSectionHeader_t));

        if ( err == FLASH_SUCCESS )
        {
            addressToStoreData = hdr.current_addr;

            // Increment the header before write
            if (bump_addr == true && err == FLASH_SUCCESS)
            {
                // Circular buffer. Wrap around when we reach the end. If we ill the buffer, overwrite
                //the contents
                if ( map_index == SECTION_DATA )
                {
                    if ( xSensorDataIsFull == true )
                    {
                        //We are overwriting data, the count is still the max value
                        hdr.lifoCount = MAX_SENSOR_DATA_LOGS;
                    }
                    else
                    {
                        //adding 1 more log
                        hdr.lifoCount++;
                    }

                   //wrap around to 0 if we are at the max, otherwise increment the head for next write
                   hdr.head = ( hdr.head + 1 ) % MAX_SENSOR_DATA_LOGS;

                   //bump address for next write based on the header - write the header to flash
                   hdr.current_addr = (Section_Map[map_index].start_addr + sizeof(flashSectionHeader_t)) + ( hdr.head * sizeof(APP_NVM_SENSOR_DATA_WITH_HEADER_T));
                   hdr.checksum = xComputeChecksum((uint8_t *)&hdr, (sizeof(flashSectionHeader_t) - 1)); // Compute a checksum, not including the checksum byte itself.
                   err = FLASH_write(Section_Map[map_index].start_addr, (uint8_t *) &hdr, sizeof(flashSectionHeader_t));

                   //check if the buffer is full with this write
                   if ( hdr.lifoCount == MAX_SENSOR_DATA_LOGS )
                   {
                       //buffer is full now
                       elogInfo("Buffer Full");
                       xSensorDataIsFull = true;
                   }
                }
            }

            if ( err == FLASH_SUCCESS )
            {
                // Then update data.
                err = FLASH_write(addressToStoreData, (uint8_t *) p_data_to_write, (Section_Map[map_index].entry_len));
            }
        }

        /* Return mutex */
        xSemaphoreGive(xMemMapMutex);
    }
    else
    {
        elogError("Failed to take mutex");
    }

    if (err == FLASH_SUCCESS)
        status = true;


    elogInfo("Done updating entry");

    return status;
}

// Compute 2 complement checksum.
static uint8_t xComputeChecksum(uint8_t * p_bytes, uint16_t num_bytes)
{
    uint8_t checksum = 0;
    uint16_t i = 0;

    if (p_bytes == NULL) return 0x00;

    for ( i = 0; i < num_bytes; i++ )
    {
        checksum += p_bytes[i];
    }

    checksum = (~checksum);
    checksum++;

    return checksum;
}

// Compare the computed checksum against the stored value.  Return true if they match.
static bool xVerifyChecksum(uint8_t * p_buf, uint16_t buf_len, uint8_t expected_checksum)
{
    bool checksum_good = false;
    uint8_t computed_checksum = xComputeChecksum(p_buf, buf_len);

    if (p_buf == NULL) return false;

    if(computed_checksum == expected_checksum)
    {
        checksum_good = true;
    }
    else
    {
        checksum_good = false;
    }

    return checksum_good;
}

static void xMemCommandHandlerFunction(int argc, char **argv)
{
    /* process the user input */
    if ( (argc == ONE_ARGUMENT) &&  (0 == strcmp(argv[FIRST_ARG_IDX], "default")) )
    {
        MEM_defaultSection(SECTION_DEVICE_INFO);
    }
    else if ( (argc == ONE_ARGUMENT) &&  (0 == strcmp(argv[FIRST_ARG_IDX], "defaultdata")) )
    {
        MEM_defaultSection(SECTION_DATA);
    }
    else if ( (argc == ONE_ARGUMENT) &&  (0 == strcmp(argv[FIRST_ARG_IDX], "configs")) )
    {
        xReadConfigs();
        elogInfo("wake rate %d", amConfigsAndInfo.am_wake_rate_days);
    }
    else
    {
        elogInfo("Invaid args");
    }
}

