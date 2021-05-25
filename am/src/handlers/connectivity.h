/**************************************************************************************************
* \file     connectivity.h
* \brief    Manage the cellular/gps connection
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

#ifndef HANDLERS_CONNECTIVITY_H_
#define HANDLERS_CONNECTIVITY_H_

extern void CONN_init(uint32_t timestamp);
extern void CONN_task(void);
extern void CONN_updateRssiForAntennaUsed(uint8_t rssi);
extern void CONN_initCliCommands(void);

#endif /* HANDLERS_CONNECTIVITY_H_ */
