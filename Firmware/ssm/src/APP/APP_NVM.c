/**************************************************************************************************
* \file     APP_NVM.c
* \brief    Initialize, validate and update data stored in NVM
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
* \date     01/29/2021
* \author   Twisthink
*
***************************************************************************************************/

#include "APP.h"
#include "HW_TERM.h"
#include <stdio.h>
#include "APP_NVM.h"
#include "APP_NVM_Cfg.h"
#include "APP_NVM_Types.h"
#include "HW_EEP.h"
#include "APP_NVM_Custom.h"

#define APP_NVM_MAGIC_VALUE_LEN     4
#define APP_NVM_MAGIC_ADDR          (HW_EEP_LAST_AVAILABLE_ADDR - (APP_NVM_MAGIC_VALUE_LEN - 1))    // Reserve 4 bytes for magic value.
#define APP_NVM_LAST_ADDR           (APP_NVM_MAGIC_ADDR - 1)                                      // Last address is right before magic value.

const uint8_t Magic_Value[APP_NVM_MAGIC_VALUE_LEN] = {0xA5, 0x5A, 0xFE, 0x00};

void APP_NVM_Init(void);
void APP_NVM_Validate(void);
void APP_NVM_UpdateCurrentEntry(uint8_t map_index, uint8_t * p_data_to_write, bool bump_addr);
void APP_NVM_ReadCurrentEntry(uint8_t map_index, uint8_t * p_data_buffer);
void APP_NVM_DefaultAll(void);
void APP_NVM_DefaultSensorDataLogs(void);
void APP_NVM_ReadSectionHeader(uint8_t map_index, APP_NVM_SECTION_HDR_T * p_hdr);
void APP_NVM_ReadBytes(uint16_t addr, uint16_t num_bytes, uint8_t * p_bytes);
uint8_t APP_NVM_ComputeChecksum(uint8_t * p_data, uint16_t num_bytes);
void APP_NVM_DefaultSection(uint8_t map_index);

static bool CheckSectionHeader(uint8_t map_index);
static bool CheckSectionMap(void);
static bool CheckForMagicValue(void);
static void WriteMagicValue(void);
static void WriteBytes(uint16_t addr, uint16_t num_bytes, uint8_t * p_bytes);


// Functions that will be externed by APP_NVM_Custom
bool APP_NVM_VerifyChecksum(uint8_t * p_buf, uint8_t buf_len, uint8_t expected_checksum);
bool APP_NVM_GenericCheckData(uint8_t map_index);


void APP_NVM_Init(void)
{
    APP_NVM_Validate();
    APP_NVM_Custom_InitDeviceInfo();
}

void APP_NVM_Validate(void)
{
    uint8_t i = 0;
    uint8_t str[60];

    if (CheckForMagicValue() == false)
    {
        APP_NVM_DefaultAll();
    }

    if (CheckSectionMap() == true)
    {
        for(i=0; i < APP_NVM_NUM_SECTIONS; i++)
        {
            sprintf((char *)str, "\nAPP_NVM: Check section 0x%x, Start 0x%x, End 0x%x\n", Section_Map[i].type, Section_Map[i].start_addr, Section_Map[i].end_addr);
            HW_TERM_Print(str);
            if ((CheckSectionHeader(i) == false) ||
                (APP_NVM_Custom_CheckSectionData(i) == false))
            {
                HW_TERM_PrintColor("Defaulting section!\n", KRED);
                APP_NVM_DefaultSection(i);
            }
            else
            {
                HW_TERM_PrintColor("Section check complete.\n", KGRN);
            }
        }
    }
    else
    {
        HW_TERM_PrintColor("Skipping sections checks due to bad map!\n", KRED);
    }
}

// Read the section header for the given map index
void APP_NVM_ReadSectionHeader(uint8_t map_index, APP_NVM_SECTION_HDR_T * p_hdr)
{
    if (p_hdr == NULL) return;
    if (map_index > (APP_NVM_NUM_SECTIONS - 1)) return;

    APP_NVM_ReadBytes(Section_Map[map_index].start_addr, sizeof(APP_NVM_SECTION_HDR_T), (uint8_t *)p_hdr);
}


// Read the values for the current section entry.
void APP_NVM_ReadCurrentEntry(uint8_t map_index, uint8_t * p_data_buffer)
{
    APP_NVM_SECTION_HDR_T hdr;

    if (p_data_buffer == NULL) return;
    if (map_index > (APP_NVM_NUM_SECTIONS - 1)) return;

    // First read the header info to get entry len.
    APP_NVM_ReadBytes(Section_Map[map_index].start_addr, sizeof(APP_NVM_SECTION_HDR_T), (uint8_t *)&hdr);
    // Then update data.
    APP_NVM_ReadBytes(hdr.current_addr, (Section_Map[map_index].entry_len), (uint8_t *) p_data_buffer);
}


// Update the entry pointed to by the current address in the header.  If bump_addr is true
// then update current address to point to the next entry.
void APP_NVM_UpdateCurrentEntry(uint8_t map_index, uint8_t * p_data_to_write, bool bump_addr)
{
    APP_NVM_SECTION_HDR_T hdr;
    uint8_t checksum = 0;
    uint16_t addressToStoreData = 0u;

    if (p_data_to_write == NULL) return;
    if (map_index > (APP_NVM_NUM_SECTIONS - 1)) return;

    // First read the header info to get entry len.
    APP_NVM_ReadBytes(Section_Map[map_index].start_addr, sizeof(APP_NVM_SECTION_HDR_T), (uint8_t *)&hdr);

    addressToStoreData = hdr.current_addr;

    // If we're bumping the address, increment head before write
    if (bump_addr == true)
    {
        // Circular buffer. Wrap around when we reach the end. If we ill the buffer, overwrite
        //the contents
        if ( map_index == APP_NVM_SECT_TYPE_SENSOR_DATA )
        {
            if( APP_NVM_Custom_GetBufferFullFlag() == true )
            {
                //move tail so that the tail moves with the head if the buffer is full
                hdr.tail = (hdr.tail + 1) % MAX_SENSOR_DATA_LOGS;
            }

           //wrap around to 0 if we are at the max, otherwise increment the head for next write
           hdr.head = ( hdr.head + 1 ) % MAX_SENSOR_DATA_LOGS;

           //bump address for next write based on the header
           hdr.current_addr = (Section_Map[map_index].start_addr + sizeof(APP_NVM_SECTION_HDR_T)) + ( hdr.head * sizeof(APP_NVM_SENSOR_DATA_T));
           hdr.checksum = APP_NVM_ComputeChecksum((uint8_t *)&hdr, (sizeof(APP_NVM_SECTION_HDR_T) - 1)); // Compute a checksum, not including the checksum byte itself.
           HW_EEP_WriteBlock(Section_Map[map_index].start_addr, (uint8_t *) &hdr, sizeof(APP_NVM_SECTION_HDR_T));

           //check if the buffer is full with this write
           if ( hdr.head == hdr.tail )
           {
               //buffer is full now
               HW_TERM_Print("Buffer Full");
               APP_NVM_Custom_IndicateBufferFull(true);
           }
        }
    }

    // Then update data.
    HW_EEP_WriteBlock(addressToStoreData, (uint8_t *) p_data_to_write, (Section_Map[map_index].entry_len - 1));
    // And append the checksum.
    checksum = APP_NVM_ComputeChecksum(p_data_to_write, (Section_Map[map_index].entry_len - 1));
    HW_EEP_WriteByte((addressToStoreData + (Section_Map[map_index].entry_len - 1)), checksum);
}

static void WriteBytes(uint16_t addr, uint16_t num_bytes, uint8_t * p_bytes)
{
    uint8_t i = 0;

    if (p_bytes == NULL) return;

    for (i = 0; i < num_bytes; i++)
    {
        HW_EEP_WriteByte((addr + i), *p_bytes);
        p_bytes++;
    }
}

void APP_NVM_ReadBytes(uint16_t addr, uint16_t num_bytes, uint8_t * p_bytes)
{
    uint8_t i = 0;

    if (p_bytes == NULL) return;

    for (i = 0; i < num_bytes; i++)
    {
        *(uint8_t *)p_bytes = HW_EEP_ReadByte((addr + i));
        p_bytes = (uint8_t *)p_bytes + 1;
    }
}

// Based upon the configuration specified in the section map, default the section header and default values.
//
// NOTE: This currently assumes that if you are defaulting more than one entry then you are using the same
//       default values for each entry.  If you want multiple entries with different values, this will have
//       to be tweaked.
void APP_NVM_DefaultSection(uint8_t map_index)
{
    APP_NVM_SECTION_HDR_T hdr;
    uint16_t addr = 0;
    uint16_t entry_index = 0;
    uint8_t checksum = 0;

    if (map_index > (APP_NVM_NUM_SECTIONS - 1)) return;

    // For each entry specified by the number of entries in the section map, fill in the default values and checksum.
    for (entry_index = 0; entry_index < Section_Map[map_index].default_num_entries; entry_index++)
    {

        addr = (Section_Map[map_index].start_addr +                     // start at the start address
                sizeof(APP_NVM_SECTION_HDR_T) +                         // plus room for the header.
                (Section_Map[map_index].entry_len * entry_index));      // plus any entries already defaulted.

        WriteBytes(addr, (Section_Map[map_index].entry_len - 1), (uint8_t *) Section_Map[map_index].p_default_values);

        // And append the checksum.
        checksum = APP_NVM_ComputeChecksum(Section_Map[map_index].p_default_values, (Section_Map[map_index].entry_len - 1));
        HW_EEP_WriteByte((addr + (Section_Map[map_index].entry_len - 1)), checksum);
    }

    // Then default the header.
    hdr.type = Section_Map[map_index].type;
    hdr.tail = Section_Map[map_index].default_num_entries;
    hdr.head = Section_Map[map_index].default_num_entries;
    hdr.entry_len = Section_Map[map_index].entry_len;
    hdr.current_addr = (Section_Map[map_index].start_addr + sizeof(APP_NVM_SECTION_HDR_T));  // The current address (to be written next) begins after the header.

    // If the section is an array then set the current address to point to the next entry to be written (after those that have been initialized)
    if (Section_Map[map_index].is_array == true)
    {
        hdr.current_addr += (Section_Map[map_index].default_num_entries * Section_Map[map_index].entry_len); // Plus any data that was written.
    }

    hdr.checksum = APP_NVM_ComputeChecksum((uint8_t *)&hdr, (sizeof(APP_NVM_SECTION_HDR_T) - 1)); // Compute a checksum, not including the checksum byte itself.

    WriteBytes( Section_Map[map_index].start_addr,  sizeof(APP_NVM_SECTION_HDR_T), (uint8_t *) &hdr);
}

// Confirm that the section header at the index appears to be valid.
// Section headers begin at the start address specified for the section and the
// data begins immediately after.
//
// Check that:
//  - The section type matches the expected section type.
//  - The current address is after the header and before the end of the section.
//  - The current address is at the correct location based upon the number of entries and entry length.
//  - Two complement checksum of the header is good.
static bool CheckSectionHeader(uint8_t map_index)
{
    bool section_good = true;
    APP_NVM_SECTION_HDR_T hdr;
    uint16_t exp_current_addr = 0;

    if (map_index > (APP_NVM_NUM_SECTIONS - 1)) return false;

    APP_NVM_ReadBytes(Section_Map[map_index].start_addr, sizeof(APP_NVM_SECTION_HDR_T), (uint8_t *)&hdr);

    // Confirm the expected section type.
    if (hdr.type == Section_Map[map_index].type)
    {
        HW_TERM_PrintColor("Section match.\n", KGRN);
    }
    else
    {
        section_good = false;
        HW_TERM_PrintColor("Type mismatch!\n", KRED);
    }

    // Only check addressing for sections that store multiple entries of the type.  Otherwise, we expect there to just be one and the
    // current_addr is irrelevant.
    if (Section_Map[map_index].is_array == true)
    {
        // Check that the next address to be written is not out of range.  Write function will inhibit writes beyond end.
        // E.g. current_addr may not allow for enough room for a write but this will be detected when the write is attempted.
        if ((hdr.current_addr <= Section_Map[map_index].end_addr) &&   // Not past end.
            (hdr.current_addr >= (Section_Map[map_index].start_addr + sizeof(APP_NVM_SECTION_HDR_T))))  // Starting after header.
        {
            HW_TERM_PrintColor("Addr in range.\n", KGRN);
        }
        else
        {
            section_good = false;
            HW_TERM_PrintColor("Addr out of range!\n", KRED);
        }

        // Confirm that the entry length stored in the header matches the configured entry length.
        if (hdr.entry_len == Section_Map[map_index].entry_len)
        {
            HW_TERM_PrintColor("Entry len is correct.\n", KGRN);
        }
        else
        {
            section_good = false;
            HW_TERM_PrintColor("Entry len is incorrect!\n", KRED);
        }

        // The expected current address should be the length of an entry times the number of entries plus the size of the header.
        exp_current_addr = ((hdr.entry_len * hdr.head) +
                            sizeof(APP_NVM_SECTION_HDR_T) +
                            Section_Map[map_index].start_addr);

        if (hdr.current_addr == exp_current_addr)
        {
            HW_TERM_PrintColor("Addr is correct.\n", KGRN);
        }
        else
        {
            section_good = false;
            HW_TERM_PrintColor("Addr is incorrect!\n", KRED);
        }
    }
    else  // Otherwise we're dealing with a single entry section type and we just want to make sure it is populated.
    {
        if (hdr.head == 1u)
        {
            HW_TERM_PrintColor("Section is populated.\n", KGRN);
        }
        else
        {
            section_good = false;
            HW_TERM_PrintColor("Section is not populated!\n", KRED);
        }
    }

    if (APP_NVM_VerifyChecksum((uint8_t *)&hdr, (sizeof(APP_NVM_SECTION_HDR_T) - 1), hdr.checksum) != true)
        section_good = false;

    return section_good;
}

// Compare the computed checksum against the stored value.  Return true if they match.
bool APP_NVM_VerifyChecksum(uint8_t * p_buf, uint8_t buf_len, uint8_t expected_checksum)
{
    bool checksum_good = false;
    uint8_t computed_checksum = APP_NVM_ComputeChecksum(p_buf, buf_len);
    uint8_t str[15];

    if (p_buf == NULL) return false;

    if(computed_checksum == expected_checksum)
    {
        checksum_good = true;
        HW_TERM_PrintColor("Checksum correct.\n", KGRN);
    }
    else
    {
        checksum_good = false;
        HW_TERM_PrintColor("Checksum incorrect!\n", KRED);

        sprintf((char*)str, "0x%x != 0x%x", expected_checksum, computed_checksum);
        HW_TERM_Print(str);

    }

    return checksum_good;
}

// Read section header and then go through each entry and confirm checksum.  Can be used by
// APP_NVM_Custom to check data section for which no other checks are required.
bool APP_NVM_GenericCheckData(uint8_t map_index)
{
    bool section_good = true;
    uint16_t entry_index = 0;
    uint8_t buf[sizeof(APP_NVM_SECTION_DATA_T)];
    uint16_t addr = 0;
    APP_NVM_SECTION_HDR_T hdr;
    uint8_t str[20];
    uint8_t checksum = 0;
    uint8_t numEntries;

    HW_TERM_Print("Checking data...");

    if (map_index > (APP_NVM_NUM_SECTIONS - 1)) return false;

    // First read the header so that we will know the number of entries and entry_len.
    APP_NVM_ReadBytes(Section_Map[map_index].start_addr, sizeof(APP_NVM_SECTION_HDR_T), (uint8_t *)&hdr);

    numEntries = APP_NVM_Custom_GetSensorDataNumEntries();
    sprintf((char*)str, "for %u entries.\n", numEntries);
    HW_TERM_Print(str);

    // Go through the entries and check the checksums.
    if ((hdr.tail > hdr.head) || numEntries == MAX_SENSOR_DATA_LOGS)
    {
        for(entry_index = hdr.tail; entry_index < MAX_SENSOR_DATA_LOGS; entry_index++)
        {
           addr = (Section_Map[map_index].start_addr + sizeof(APP_NVM_SECTION_HDR_T) + // Start after the header.
                  (entry_index * hdr.entry_len));  // And add to the address for every entry checked.

           APP_NVM_ReadBytes(addr, (hdr.entry_len - 1), (uint8_t *)&buf);
           checksum = HW_EEP_ReadByte(addr + (hdr.entry_len - 1));

           if (APP_NVM_VerifyChecksum(buf, (hdr.entry_len - 1), checksum) != true)
           {
               section_good = false;
           }
        }
        for(entry_index = hdr.tail; entry_index < hdr.head; entry_index++)
        {
           addr = (Section_Map[map_index].start_addr + sizeof(APP_NVM_SECTION_HDR_T) + // Start after the header.
                  (entry_index * hdr.entry_len));  // And add to the address for every entry checked.

           APP_NVM_ReadBytes(addr, (hdr.entry_len - 1), (uint8_t *)&buf);
           checksum = HW_EEP_ReadByte(addr + (hdr.entry_len - 1));

           if (APP_NVM_VerifyChecksum(buf, (hdr.entry_len - 1), checksum) != true)
           {
               section_good = false;
           }
        }
    }
    else
    {
        for(entry_index = hdr.tail; entry_index < hdr.head; entry_index++)
        {
           addr = (Section_Map[map_index].start_addr + sizeof(APP_NVM_SECTION_HDR_T) + // Start after the header.
                  (entry_index * hdr.entry_len));  // And add to the address for every entry checked.

           APP_NVM_ReadBytes(addr, (hdr.entry_len - 1), (uint8_t *)&buf);
           checksum = HW_EEP_ReadByte(addr + (hdr.entry_len - 1));

           if (APP_NVM_VerifyChecksum(buf, (hdr.entry_len - 1), checksum) != true)
           {
               section_good = false;
           }
        }
    }

    if (section_good == true)
    {
       HW_TERM_PrintColor("Section data good.\n", KGRN);
    }
    else
    {
       section_good = false;
       HW_TERM_PrintColor("Section data bad!\n", KRED);
    }

    return section_good;
}

// Compute 2 complement checksum.
uint8_t APP_NVM_ComputeChecksum(uint8_t * p_bytes, uint16_t num_bytes)
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

// Check for problems in the section map.  Returns true if map is ok.
static bool CheckSectionMap(void)
{
    uint16_t prev_end = 0x0000;
    bool map_good = true;
    uint8_t i = 0;

    HW_TERM_Print("\nAPP_NVM: Checking section map.\n");

    for(i=0; i < APP_NVM_NUM_SECTIONS; i++)
    {
        // Check for overlaps.
        if(Section_Map[i].start_addr != 0x0000 &&
          (Section_Map[i].start_addr <= prev_end))
        {
            map_good = false;
            HW_TERM_PrintColor("Section overlap!\n", KRED);
        }

        // Start address must be < end address
        if(Section_Map[i].start_addr >= Section_Map[i].end_addr)
        {
            HW_TERM_PrintColor("Bad section addresses!\n", KRED);
            map_good = false;
        }

        // Make sure we have not gone over the magic number area
        if(Section_Map[i].end_addr > APP_NVM_LAST_ADDR)
        {
            HW_TERM_PrintColor("End address past end of NVM!\n", KRED);
            map_good = false;
        }

        prev_end = Section_Map[i].end_addr;
    }

    if (map_good == true)
    {
        HW_TERM_PrintColor("Section map good.\n", KGRN);
    }
    else
    {
        HW_TERM_PrintColor("Section map error!\n", KRED);
    }

    return map_good;
}

static bool CheckForMagicValue(void)
{
    bool magic_good = true;
    uint16_t addr = 0;
    uint8_t i = 0;

    HW_TERM_Print("\nAPP_NVM: Checking for magic value.\n");

    addr = APP_NVM_MAGIC_ADDR;
    for (i = 0; i < APP_NVM_MAGIC_VALUE_LEN; i++)
    {
       if (HW_EEP_ReadByte(addr + i) != Magic_Value[i])
           magic_good = false;
    }

    if (magic_good == true)
    {
        HW_TERM_PrintColor("Magic value good.\n", KGRN);
    }
    else
    {
        HW_TERM_PrintColor("No magic value!\n", KRED);
    }

    return magic_good;
}

void APP_NVM_DefaultSensorDataLogs(void)
{
    HW_TERM_PrintColor("Defaulting Sensor Data Logs to 0 \n", KMAG);
    APP_NVM_DefaultSection(APP_NVM_SECT_TYPE_SENSOR_DATA);
}

void APP_NVM_DefaultAll(void)
{
    uint8_t i = 0;

    HW_TERM_PrintColor("Defaulting all sections!\n", KRED);

    for(i=0; i < APP_NVM_NUM_SECTIONS; i++)
    {
        APP_NVM_DefaultSection(i);
    }

    WriteMagicValue();
}

static void WriteMagicValue(void)
{
    HW_TERM_Print("Writing magic value.\n");
    WriteBytes(APP_NVM_MAGIC_ADDR, APP_NVM_MAGIC_VALUE_LEN, (uint8_t *) Magic_Value);
}
