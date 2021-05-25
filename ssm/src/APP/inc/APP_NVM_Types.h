/**************************************************************************************************
* \file     APP_NVM_Types.h
* \brief    This file holds types used by all NVM implementations.
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

#ifndef APP_NVM_TYPES_H
#define APP_NVM_TYPES_H

#include <stdbool.h>
#include <stdint.h>

//this is not written to NVM
typedef struct APP_NVM_SECTION_MAP
{
    uint8_t         type;
    uint16_t        start_addr;             // Address of of the APP_NVM_SECTION_HDR_T.  Data will begin immediately after the header.
    uint16_t        end_addr;               // Last byte that may be used by this section.
    bool            is_array;               // Set to true if multiple instances.
    uint16_t        entry_len;              // The length of a single entry of the given type.
    uint16_t        default_num_entries;    // If the section is defaulted, this number of default entries will be written.
    void *          p_default_values;       // If the section is defaulted, this is the default data that will be written to the section.
}APP_NVM_SECTION_MAP_T;

// A section is a region of data of a specific type.  Each section begins with the section
// header below, which indicates the type of data and information about the current status
// of the data.  The two's complement checksum is performed on all elements of the header.
// The data of the specified type will begin immediately after the header.  The location
// of sections is specified in Section_Map[].

//this is written to NVM
typedef struct APP_NVM_SECTION_HDR
{
    uint8_t         type;
    uint8_t         head;
    uint8_t         tail;
    uint16_t        entry_len;
    uint16_t        current_addr;
    uint8_t         checksum;
}__attribute__ ((__packed__)) APP_NVM_SECTION_HDR_T;

#endif /* APP_NVM_TYPES_H */
