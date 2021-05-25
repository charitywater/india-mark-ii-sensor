/**************************************************************************************************
* \file     gpsManager.h
* \brief    Functions to handle and maintain the gps module.
*           The GPS module should only be turned on one time upon activation to obtain a GPS fix.
*           It should then remain powered off throughout the rest of the sensors lifetime.
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

#ifndef HANDLERS_GPSMANAGER_H_
#define HANDLERS_GPSMANAGER_H_

#include "stdbool.h"

extern void GPS_initModule(void);
extern void GPS_Enable(void);
extern void GPS_Disable(void);
extern void GPS_monitorTask();
extern void GPS_processReceivedChars(void);
extern void GPS_HandleUartErr(void);
extern bool GPS_isGpsEnabled(void);

#endif /* HANDLERS_GPSMANAGER_H_ */
