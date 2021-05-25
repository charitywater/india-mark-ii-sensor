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

#ifndef HANDLERS_MEMMAPHANDLER_H_
#define HANDLERS_MEMMAPHANDLER_H_

#include "stdbool.h"
#include "stdint.h"
#include "APP_NVM_Cfg_Shared.h"
#include "messages.pb.h"

typedef enum
{
    SECTION_DEVICE_INFO = (uint8_t)0,
    SECTION_DATA = (uint8_t)1,
    SECTION_IMAGE_REGISTRY = (uint8_t)2,
    SECTION_AM_APP_A = (uint8_t)3,
    SECTION_SSM_APP_A = (uint8_t)4,
    SECTION_AM_APP_B = (uint8_t)5,
    SECTION_SSM_APP_B = (uint8_t)6,
}sectionDataTypes_t;


//used in the image registry to identify the state of each stored image
typedef enum
{
    OP_UNKNOWN = (uint8_t)0,
    PARTIAL = (uint8_t)1,
    FULL = (uint8_t)2,
    OP_FAILED = (uint8_t)3,
}imageOperationalState_t;

//used to identify the loaded image and the primary image
typedef enum
{
    A = (uint8_t)0,
    B = (uint8_t)1,
    UNKNOWN_SLOT = (uint8_t)0xFF
}imageSlotTypes_t;

//used to identify the loaded image and the primary image
typedef enum
{
    AM_IMAGE = (uint8_t)0,
    SSM_IMAGE = (uint8_t)1,
}imageTypes_t;

//used to identify the loaded image and the primary image
typedef enum
{
    PRIMARY = (uint8_t)0,
    SECONDARY =  (uint8_t)1,
    UNKNOWN_ANTENNA = (uint8_t)0xFF
}antenna_t;


//image registry contents
typedef struct __attribute__ ((__packed__))
{
    uint8_t primaryImage;
    uint8_t loadedImage;

    //image A
    uint8_t imageAoperationalState;
    uint32_t imageAfwVersionMajor;
    uint32_t imageAfwVersionMinor;
    uint32_t imageAfwVersionBuild;

    //image B
    uint8_t imageBoperationalState;
    uint32_t imageBfwVersionMajor;
    uint32_t imageBfwVersionMinor;
    uint32_t imageBfwVersionBuild;
    uint8_t checksum;
}imageRegistry_t;

//this is not written to NVM
typedef struct
{
    uint8_t         type;
    uint32_t        start_addr;             // Address of of the APP_NVM_SECTION_HDR_T.  Data will begin immediately after the header.
    uint32_t        end_addr;               // Last byte that may be used by this section.
    bool            is_array;               // Set to true if multiple instances.
    uint16_t        entry_len;              // The length of a single entry of the given type.
    uint16_t        default_num_entries;    // If the section is defaulted, this number of default entries will be written.
    void *          p_default_values;       // If the section is defaulted, this is the default data that will be written to the section.

}flashSectionMap_t;

// A section is a region of data of a specific type.  Each section begins with the section
// header below, which indicates the type of data and information about the current status
// of the data.  The two's complement checksum is performed on all elements of the header.
// The data of the specified type will begin immediately after the header.  The location
// of sections is specified in Section_Map[].

//this is written to NVM
typedef struct
{
    uint8_t         type;
    uint8_t         head;
    uint8_t         lifoCount;
    uint16_t        entry_len;
    uint32_t        current_addr;
    uint8_t         checksum;
}__attribute__ ((__packed__)) flashSectionHeader_t;


// This section type provides generic info about the device. There will only be one copy of this information
// and by nature it will not be updated frequently (FW updates, activation, etc. )

//Anything we don't want to lose during a reset should be added to this device info struct
typedef struct __attribute__ ((__packed__))
{
    uint16_t        nvm_version;             // Set to APP_NVM_VERSION, which should updated on changes.
    uint32_t        first_act_date;          // Date the device was first activated.
    uint32_t        recent_act_date;         // Most-recent date the device was activated.
    uint32_t        recent_deact_date;       // Most-recent date the device was deactivated.
    uint8_t         num_act;                 // Total number of times the devices as been activated.
    uint8_t         num_deact;               // Total number of times the devices as been deactivated.
    uint16_t        am_wake_rate_days;       // Rate in days at which to wake up the AM
    bool            isStrokeDetectionEnabled;// Should match SSM side flag. Used to include or exclude stroke data in protobuf
    uint8_t         reset_state;             // Information on the reset source
    uint32_t        gpsTimeoutSeconds;       // Timeout to stop searching for GPS fix
    uint32_t        maxHop;                  // Maximum horizontal dilution of position (HDOP) value to consider a valid fix (scaled by factor of 10)
    uint32_t        minMeasureTime;          // Minimum measurement time in seconds to consider a valid fix
    uint32_t        numOfSatellites;         // Minimum number of satellites to consider a valid fix for GPS
    uint32_t        msgNumber;               // Unique Id for MQTT messages, start at 1 and increment
    uint8_t         antennaToUse;            // We have two antennas, this config specifies which one to use on the next aws connection
    uint8_t         rssiPrimaryAntenna;      // Last RSSI value of the primary antenna
    uint8_t         rssiSecondaryAntenna;    // Last RSSI value of thesecondary antenna
    uint32_t        epochTimeLastSwitch;     // The last time we did an antenna switch. Epoch time received from the SSM
    uint8_t         resetsSinceLpMode;       // Track how many times we have reset to determine if we need to reload SSM
    uint32_t        unexpectedResetCounter;  // Number of unexpected AM resets over the lifetime of the device
    uint32_t        timeLastUnexpectedReset; // Epoch time of the last unexpected AM reset
    GpsMessage      gpsCoordinates;          // Location of the sensor upon activation
    bool            gpsLocationFixed;        // If we have a valid location saved in the GPS data structure
    bool            gpsLocationSentToCloud;  // Send the location once upon activation
    uint8_t         gpsRetries;              // How many times have we timed out obtaining a fix
    bool            manufacturingComplete;   // Indicates that devices has finished the manufacturing process & a message should be sent to the cloud
    uint32_t        manfCompleteSecsToWait;  // Wait time before sending a message to the cloud. Configurable via CLI
    bool            sensorDataBufferFull;    // save state to NVM since head = tail could be 0 logs OR a bull buffer
    uint8_t         checksum;
}deviceInfo_t;

typedef union
{
    flashSectionHeader_t header;
    deviceInfo_t   device_info;
    APP_NVM_SENSOR_DATA_WITH_HEADER_T  sensor_data;
}flashSectionData_t;


extern bool MEM_init(void);
extern uint8_t MEM_getHighLevelState(void);
extern void MEM_writeHighLevelState(uint8_t state);
extern uint32_t MEM_getActivationDate(void);
extern void MEM_writeActivationDate(uint32_t date);
extern uint32_t MEM_getDeactivationDate(void);
extern void MEM_writeDeactivationDate(uint32_t date);
extern uint16_t MEM_getAmWakeRate(void);
extern bool MEM_writeAmWakeRate(uint16_t days);
extern bool MEM_getIsStrokeDetectionEnabledFlag(void);
extern bool MEM_writeStrokeDetectionEnabledFlag(bool isStrokeDetEnabled);
extern uint8_t MEM_getResetsSinceLastLpMode(void);
extern void MEM_setResetsSinceLastLpMode(uint8_t resets);

extern void MEM_setAntennaToUse(antenna_t antenna);
extern antenna_t MEM_getAntennaToUse(void);

extern void MEM_setPrimaryAntennaRssi(uint8_t rssi);
extern uint8_t MEM_getPrimaryAntennaRssi(void);
extern void MEM_setSecondaryAntennaRssi(uint8_t rssi);
extern uint8_t MEM_getSecondaryAntennaRssi(void);
extern void MEM_setEpochTimeLastAntennaSwitch(uint32_t epochTime);
extern uint32_t MEM_getEpochTimeLastAntennaSwitch(void);

extern uint32_t MEM_getGpsTimeoutSeconds(void);
extern uint32_t MEM_getGpsMaxdHop(void);
extern uint32_t MEM_getGpsMinMeasTimeSec(void);
extern uint32_t MEM_getGpsNumSatellites(void);
extern bool MEM_writeGpsConfigs(uint32_t gpsTimeoutSecs, uint32_t maxdHop, uint32_t minMeasTimeSecs, uint32_t numSat);

extern bool MEM_UpdateGpsCoordinates(GpsMessage latestGpsLocation);
extern GpsMessage MEM_GetGpsCoordinates(void);

extern bool MEM_GetGpsFixedFlag(void);
extern bool MEM_SetGpsFixedFlag(bool fixPassed);

extern bool MEM_GetGpsSentToCloud(void);
extern bool MEM_SetGpsSentToCloud(bool sent);

extern uint8_t MEM_GetGpsNumRetries(void);
extern bool MEM_SetGpsNumRetries(uint8_t tries);

extern uint32_t MEM_getMsgNumber(void);
extern bool MEM_updateMsgNumber(void);

extern int16_t MEM_getNumSensorDataEntries(void);
extern bool MEM_writeSensorDataLog(APP_NVM_SENSOR_DATA_WITH_HEADER_T *pSensorData);
extern bool MEM_getSensorDataLog(APP_NVM_SENSOR_DATA_WITH_HEADER_T *pSensorData);
extern  bool MEM_updateSensorDataHeadAndLifoCount(void);
extern bool MEM_defaultSection(uint8_t section);

extern uint32_t MEM_getUnexpectedResetCount(void);
extern uint32_t MEM_getTimestampLastUnexpectedReset(void);
extern void MEM_incrementUnexpectedResetCount(uint32_t timestampOfReset);

extern uint32_t MEM_getSecondsToWaitForMfgComplete(void);
extern bool MEM_getMfgCompleteFlag(void);

extern void MEM_setSecondsToWaitForMfgComplete(uint32_t seconds);
extern void MEM_setMfgCompleteFlag(bool isMfgComplete);

extern imageSlotTypes_t MEM_getLoadedImage(void);
extern bool MEM_setLoadedImage(imageSlotTypes_t imageSlot);
extern imageSlotTypes_t MEM_getPrimaryImage(void);
extern bool MEM_setPrimaryImage(imageSlotTypes_t slot);
extern bool MEM_setImageAoperationalState(imageOperationalState_t state);
extern bool MEM_setImageBoperationalState(imageOperationalState_t state);
extern bool MEM_setImageBversion(uint32_t major, uint32_t minor, uint32_t build);
extern bool MEM_setImageAversion(uint32_t major, uint32_t minor, uint32_t build);
extern bool MEM_updateLoadedImageOpState(imageOperationalState_t state);
extern bool MEM_updateAlternateImageOpState(imageOperationalState_t state);

extern imageOperationalState_t MEM_getImageOpState(imageSlotTypes_t slot);
extern bool MEM_setImageOpState(imageOperationalState_t opState, imageSlotTypes_t slot);
extern imageSlotTypes_t MEM_getSlotWithThisVersion(uint32_t major, uint32_t minor, uint32_t build);
extern imageSlotTypes_t MEM_getAlternateSlot(imageSlotTypes_t slotToGetAlternateOf);


#endif /* HANDLERS_MEMMAPHANDLER_H_ */
