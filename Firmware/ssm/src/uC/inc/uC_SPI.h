/**************************************************************************************************
* \file     uC_SPI.h
* \brief    SPI driver
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

#ifndef uC_SPI_H
#define uC_SPI_H

#include <stdbool.h>
#include <stdint.h>

extern void uC_SPI_Init(void);
extern bool uC_SPI_BytesReady(void);
extern uint8_t uC_SPI_GetNextByte(void);
extern bool uC_SPI_Tx(uint8_t * p_bytes, uint8_t num_bytes);

#endif /* uC_SPI_H */
