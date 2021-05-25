/**************************************************************************************************
* \file     HW.c
* \brief    Hardware - Top level functionality and initialization
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

#include "HW.h"
#include "HW_TERM.h"
#include "HW_ENV.h"
#include "HW_RTC.h"
#include "HW_BAT.h"
#include "HW_MAG.h"
#include "HW_EEP.h"
#include "HW_GPIO.h"
#include <msp430.h>                      // Generic MSP430 Device Include
#include "driverlib.h"                   // MSPWare Driver Library
#include "am-ssm-spi-protocol.h"
#include "APP_NVM.h"
#include "HW_BAT.h"

void HW_Init(void);
void HW_PerformSwReset(void);


void HW_Init(void)
{
    HW_TERM_Init();
    HW_ENV_Init();
    HW_MAG_InitBusAndDevice();
    HW_RTC_Init();
    HW_EEP_Init();
    HW_GPIO_Init();
    HW_FUEL_GAUGE_Initialize();
}

//used for debugging & through the CLI
void HW_PerformSwReset(void)
{
    HW_TERM_Print("HW: Commanded reset.\n\n");
    APP_NVM_Custom_WriteResetState(STATE_SWR);
    WDTCTL = 0xFFFF;
}
