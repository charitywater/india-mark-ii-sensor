/*
================================================================================================#=
Module:   Memory Map Handler

Description:
    Handler for the NAND Flash chip and contains memory mapping info. Only
     contains bootloader specific mem areas

Copyright 2021 Twisthink LLC
This code is licensed under Twisthink license.
See Twisthink-SoftwareLicenseAgreement-Auris.docx for details.
================================================================================================#=
*/

#ifndef HANDLERS_MEMMAPHANDLER_H_
#define HANDLERS_MEMMAPHANDLER_H_

#include "stdbool.h"
#include "stdint.h"

typedef enum
{
    SECTION_CONFIGS = (uint8_t)0,
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
    FAILED = (uint8_t)3,
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
    uint8_t         tail;
    uint16_t        entry_len;
    uint32_t        current_addr;
    uint8_t         checksum;
}__attribute__ ((__packed__)) flashSectionHeader_t;


// This section type provides generic info about the device. There will only be one copy of this information
// and by nature it will not be updated frequently (FW updates, activation, etc. )

//Anything we dont want to loose during a reset should be added to this device info struct
typedef struct __attribute__ ((__packed__))
{
    uint16_t        nvm_version;            // Set to APP_NVM_VERSION, which should updated on changes.
    uint32_t        first_act_date;         // Date the device was first activated.
    uint32_t        recent_act_date;        // Most-recent date the device was activated.
    uint32_t        recent_deact_date;      // Most-recent date the device was deactivated.
    uint8_t         num_act;                // Total number of times the devices as been activated.
    uint8_t         num_deact;              // Total number of times the devices as been deactivated.
    uint16_t        am_wake_rate_days;      // Rate in days at which to wake up the AM
    uint8_t         reset_state;            // Information on the reset source
    uint32_t        gpsTimeoutSeconds;      // Timeout to stop searching for GPS fix
    uint32_t        maxHop;                 // Maximum horizontal dilution of position (HDOP) value to consider a valid fix (scaled by factor of 10)
    uint32_t        minMeasureTime;         // Minimum measurement time in seconds to consider a valid fix
    uint32_t        numOfSatellites;        // Minimum number of satellites to consider a valid fix for GPS
    uint8_t         checksum;
}deviceInfo_t;

typedef union
{
    flashSectionHeader_t header;
    deviceInfo_t   device_info;
}flashSectionData_t;


extern bool MEM_init(void);
extern bool MEM_isFlashValid(void);
extern imageSlotTypes_t MEM_getLoadedImage(void);
extern imageSlotTypes_t MEM_getPrimaryImage(void);

extern imageOperationalState_t MEM_getImageAoperationalState(void);
extern imageOperationalState_t MEM_getImageBoperationalState(void);

//BL needs to be able to update the image registry during manufacturing
//lets still limit the functionality of writing to flash by providing just
//these functions:
extern void MEM_writeMagicValue(void);
extern bool MEM_setPrimaryImage(imageSlotTypes_t slot);
extern bool MEM_setImageAoperationalState(imageOperationalState_t state);
extern bool MEM_setImageAversion(uint32_t major, uint32_t minor, uint32_t build);
extern bool MEM_defaultSection(uint8_t section);

#endif /* HANDLERS_MEMMAPHANDLER_H_ */
