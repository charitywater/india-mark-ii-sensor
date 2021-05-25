/**************************************************************************************************
* \file     uC_I2C.h
* \brief    I2C driver
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

#ifndef uC_I2C_H
#define uC_I2C_H

#include <stdint.h>
#include <stdbool.h>

extern void uC_I2C_Init(void);
extern void uC_I2C_WriteRegSingle(uint8_t slave_addr, uint8_t reg_addr, uint8_t value, bool retry_on_nak);
extern bool uC_I2C_WriteMulti(uint8_t slave_addr, uint8_t * p_payload, uint8_t num_bytes,  bool retry_on_nak);
extern uint8_t uC_I2C_ReadRegSingle(uint8_t slave_addr, uint8_t reg_addr,  bool retry_on_nak);

#endif /* uC_I2C_H */
