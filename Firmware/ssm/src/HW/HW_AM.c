/**************************************************************************************************
* \file     HW_AM.c
* \brief    Application Micro interface functionality.
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

#include "HW_AM.h"
#include "HW_GPIO.h"


void HW_AM_Init(void);

void HW_AM_Init(void)
{
    // Clear the wake line
    HW_GPIO_Clear_WAKE_AP();
}
