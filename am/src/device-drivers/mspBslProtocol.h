/**************************************************************************************************
* \file     mspBslProtocol.h
* \brief    API to communicate with an MSP430 BSL FRAM bootloader
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

#ifndef DEVICE_DRIVERS_MSPBSLPROTOCOL_H_
#define DEVICE_DRIVERS_MSPBSLPROTOCOL_H_

#include <stdint.h>
#include <stdbool.h>

extern uint16_t BSL_calculateChecksum(const uint8_t* data_p, uint16_t length);
extern bool BSL_writePasswordDefault(void);
extern bool BSL_writePassword(uint8_t* password, uint16_t passwordSize);
extern bool BSL_readMemory(uint32_t startAddress, uint8_t length, uint8_t * dataResult);
extern bool BSL_writeMemory(uint32_t startAddress, uint8_t length, uint8_t * data);
extern bool BSL_massErase(void);
extern bool BSL_loadPC(uint32_t startAddress);
extern bool BSL_writeLargeChunkOfDataToMemory(uint32_t startAddress, uint32_t length, uint8_t * data);
extern bool BSL_programMSP430(void);
extern uint16_t BSL_performCrcCheck(uint32_t startAddress, uint16_t len);


#endif /* DEVICE_DRIVERS_MSPBSLPROTOCOL_H_ */
