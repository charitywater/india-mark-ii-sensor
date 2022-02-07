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

/* Includes */
#include <logTypes.h>
#include <stm32l4xx_hal.h>
#include "stdbool.h"
#include "string.h"
#include "memoryMap.h"
#include <flashHandler.h>
#include "memMapHandler.h"

//bump this if there is a change to mem map in future versions
#define FLASH_VERSION           1
#define FLASH_MAGIC_VALUE       0xABCDEABC

static imageRegistry_t appImageRegistry = {};

static bool memInitialized = false;

// private functions
static bool xReadImageRegistry(void);
static bool xIsMagicValuePresent(void);
static bool xReadCurrentEntry(uint8_t sectionTypeIndex, uint8_t * pBuffer, uint16_t bufferLen);
static uint8_t xComputeChecksum(uint8_t * p_bytes, uint16_t num_bytes);
static bool xVerifyChecksum(uint8_t * p_buf, uint8_t buf_len, uint8_t expected_checksum);
static bool xUpdateCurrentEntry(uint8_t map_index, uint8_t * p_data_to_write, bool bump_addr);


//use this if the IR ever is corrupted
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

const flashSectionMap_t Section_Map[APP_MEM_NUM_SECTIONS] =
{
    // Device info/configs section
    {
        .type = SECTION_CONFIGS,
        .start_addr = APP_MEM_ADR_CONFIG_START,
        .end_addr = APP_MEM_ADR_CONFIG_END,
        .is_array = false,
        .entry_len = sizeof(deviceInfo_t),
        .default_num_entries = 1,
    },

    // Daily reports section.
    {
        .type = SECTION_DATA,
        .start_addr = APP_MEM_ADR_SENSOR_DATA_LOGS_START,
        .end_addr = APP_MEM_ADR_SENSOR_DATA_LOGS_END,
        .is_array = true,
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
    bool status = false;

    status = xReadImageRegistry();

    if ( status == true )
    {
        elogInfo("initialized image registry");
        memInitialized = true;
    }
    else
    {
        elogNotice(ANSI_COLOR_YELLOW "Image registry INVALID");
    }

    return status;
}

bool MEM_isFlashValid(void)
{
    return xIsMagicValuePresent();
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

void MEM_writeMagicValue(void)
{
    uint32_t magicVal = FLASH_MAGIC_VALUE;

    FLASH_write(APP_MEM_ADR_MAGIC_VALUE, (uint8_t*)&magicVal, sizeof (uint32_t));
}

bool MEM_defaultSection(uint8_t section)
{
    uint32_t addr;
    flashSectionHeader_t hdr;
    uint8_t checksum = 0;
    bool status = false;
    flashErr_t err = FLASH_SUCCESS;

    elogInfo("Defaulting section %d", section);

    if ( section == SECTION_IMAGE_REGISTRY )
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
            hdr.tail = Section_Map[section].default_num_entries;
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
        elogNotice(ANSI_COLOR_YELLOW "Image registry was invalid");
    }

    return stat;
}

imageSlotTypes_t MEM_getLoadedImage(void)
{
    if (!memInitialized)
        return 0xFF;

    return appImageRegistry.loadedImage;
}

imageSlotTypes_t MEM_getPrimaryImage(void)
{
    if (!memInitialized)
        return 0xFF;

    return appImageRegistry.primaryImage;
}


imageOperationalState_t MEM_getImageAoperationalState(void)
{
    if (!memInitialized)
        return 0xFF;

    return appImageRegistry.imageAoperationalState;
}

imageOperationalState_t MEM_getImageBoperationalState(void)
{
    if (!memInitialized)
        return 0xFF;

    return appImageRegistry.imageBoperationalState;
}

bool MEM_getImageAversion(uint32_t *versionMajor, uint32_t *versionMinor, uint32_t *versionBuild )
{
    if (!memInitialized)
        return false;

   *versionMajor = appImageRegistry.imageAfwVersionMajor;
   *versionMinor = appImageRegistry.imageAfwVersionMinor;
   *versionBuild = appImageRegistry.imageAfwVersionBuild;

   return true;
}

bool MEM_getImageBversion(uint32_t *versionMajor, uint32_t *versionMinor, uint32_t *versionBuild )
{
    if (!memInitialized)
        return false;

   *versionMajor = appImageRegistry.imageBfwVersionMajor;
   *versionMinor = appImageRegistry.imageBfwVersionMinor;
   *versionBuild = appImageRegistry.imageBfwVersionBuild;

   return true;
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

    // Read the header info
    err = FLASH_read(Section_Map[sectionTypeIndex].start_addr, (uint8_t *)&hdr, sizeof(flashSectionHeader_t));

    if (err == FLASH_SUCCESS)
    {
        //then get the entry value
        err = FLASH_read(hdr.current_addr, (uint8_t *) pBuffer, (Section_Map[sectionTypeIndex].entry_len));
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

    if (p_data_to_write == NULL) return status;
    if (map_index != (uint8_t)SECTION_IMAGE_REGISTRY) return status;

    // First read the header info to get entry len.
    err = FLASH_read(Section_Map[map_index].start_addr, (uint8_t *)&hdr, sizeof(flashSectionHeader_t));

    if ( err == FLASH_SUCCESS )
    {
        // Increment the header before write
        if (bump_addr == true && err == FLASH_SUCCESS)
        {
            // Only bump the address if there is room in the section.
            if ((Section_Map[map_index].end_addr - hdr.current_addr) <= Section_Map[map_index].entry_len)
            {
                hdr.head = Section_Map[map_index].default_num_entries;
                hdr.current_addr += (Section_Map[map_index].start_addr + sizeof(flashSectionHeader_t));
                hdr.checksum = xComputeChecksum((uint8_t *)&hdr, (sizeof(flashSectionHeader_t) - 1)); // Compute a checksum, not including the checksum byte itself.
                err = FLASH_write(Section_Map[map_index].start_addr, (uint8_t *) &hdr, sizeof(flashSectionHeader_t));
            }
            else
            {
                hdr.head++;
                hdr.current_addr += (Section_Map[map_index].entry_len);
                hdr.checksum = xComputeChecksum((uint8_t *)&hdr, (sizeof(flashSectionHeader_t) - 1)); // Compute a checksum, not including the checksum byte itself.
                err = FLASH_write(Section_Map[map_index].start_addr, (uint8_t *) &hdr, sizeof(flashSectionHeader_t));
            }
        }

        if ( err == FLASH_SUCCESS )
        {
            // Then update data.
            err = FLASH_write(hdr.current_addr, (uint8_t *) p_data_to_write, (Section_Map[map_index].entry_len));
        }
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
    uint8_t i = 0;

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
static bool xVerifyChecksum(uint8_t * p_buf, uint8_t buf_len, uint8_t expected_checksum)
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
