/**************************************************************************************************
* \file     otaUpdate.h
* \brief    Handle OTA update downloads and store to FLASH
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

#ifndef OTAUPDATE_H_
#define OTAUPDATE_H_

#include "stdbool.h"

#define MAX_OTA_FILEPATH_LEN_BYTES        150

//some protocol definitions
#define RECORD_TYPE_IDX                   0
#define RECORD_LEN_IDX                    1
#define AM_FW_VERSION_START_IDX           12
#define RECORD_HEADER_LEN                 5

#define CRC_LEN                           2

extern bool OTA_initDownload(char * filePath);
extern void  OTA_downloadTask();

#endif /* OTAUPDATE_H_ */
