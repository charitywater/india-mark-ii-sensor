/**************************************************************************************************
* \file     watchdog.c
* \brief    Driver to init and refresh the system watchdog (WWDG peripheral)
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

/* Includes */
#include "stdint.h"
#include "stm32l4xx_hal.h"
#include "logTypes.h"
#include "watchdog.h"

#define INTERNAL_DIVIDER    4096
#define US_PER_MS           1000
#define WD_WINDOW           80
#define WD_COUNTER_MAX      0x7F

static WWDG_HandleTypeDef wwdgHandle;
static uint32_t xTimeoutCalculation(uint32_t timevalue);

void WD_init(void)
{
    /* Clear reset flags */
    __HAL_RCC_CLEAR_RESET_FLAGS();

    /*  Configuration:
     1] Set WWDG counter to maximum 0x7F (127 cycles)  and window to 80 cycles
     2] Set Prescaler to 8 (2^3)

     Timing calculation:
     a) WWDG clock counter period (in ms) = (4096 * 8) / (PCLK1 / 1000)
                                          = 2043 us
     b) WWDG timeout (in ms) = (127 + 1) * 2043 us
                             ~= 262 ms
     c) Time to enter inside window
     Window timeout (in ms) = (127 - 80 + 1) * 2043us
                            = 98 ms */
    wwdgHandle.Instance = WWDG;
    wwdgHandle.Init.Prescaler = WWDG_PRESCALER_8;
    wwdgHandle.Init.Window    = WD_WINDOW;
    wwdgHandle.Init.Counter   = WD_COUNTER_MAX;
    wwdgHandle.Init.EWIMode   = WWDG_EWI_DISABLE;
    if (HAL_WWDG_Init(&wwdgHandle) != HAL_OK)
    {
        elogError("failed to init wd");
    }
}

bool WD_recoveredFromReset(void)
{
    bool recovered = false;

    /* Check if the system has resumed from WWDG reset */
    if (__HAL_RCC_GET_FLAG(RCC_FLAG_WWDGRST) != RESET)
    {
        elogError("****Recovered from a wd reset ****");
        recovered = true;
    }

    return recovered;
}

void WD_refresh(void)
{
    if (HAL_WWDG_Refresh(&wwdgHandle) != HAL_OK)
    {
        elogError("failed to refresh wd");
    }
}

uint32_t WD_getRefreshRateMs(void)
{
    return xTimeoutCalculation((wwdgHandle.Init.Counter-wwdgHandle.Init.Window) + 1) + 1;
}

//Returns the length of time the app should wait before kicking the wd
static uint32_t xTimeoutCalculation(uint32_t timevalue)
{
    uint32_t timeoutvalue = 0;
    uint32_t pclk1 = 0;
    uint32_t wdgtb = 0;

    /* Get PCLK1 value */
    pclk1 = HAL_RCC_GetPCLK1Freq();

    /* get prescaler */
    switch(wwdgHandle.Init.Prescaler)
    {
        case WWDG_PRESCALER_1:   wdgtb = 1;   break;
        case WWDG_PRESCALER_2:   wdgtb = 2;   break;
        case WWDG_PRESCALER_4:   wdgtb = 4;   break;
        case WWDG_PRESCALER_8:   wdgtb = 8;   break;
        default: wdgtb = 1; break;
    }

    /* calculate timeout */
    timeoutvalue = ((INTERNAL_DIVIDER * wdgtb * timevalue) / (pclk1 / US_PER_MS));

    return timeoutvalue;
}
