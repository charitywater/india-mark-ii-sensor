/**************************************************************************************************
* \file     APP_WTR.h
* \brief    Application Water Level Measurement
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
#ifndef APP_WTR_H
#define APP_WTR_H

#include <stdbool.h>
#include <stdint.h>

// Pads in order from bottom to top
typedef enum
{									// Corresponding Pad Number
	CAPTIVATE_RX_CHANNEL2 = 0,		// PAD 1
	CAPTIVATE_RX_CHANNEL1,			// PAD 2
	CAPTIVATE_RX_CHANNEL6,			// PAD 3
	CAPTIVATE_RX_CHANNEL5,			// PAD 4
	CAPTIVATE_RX_CHANNEL4,			// PAD 5
	CAPTIVATE_RX_CHANNEL8,			// PAD 6
	CAPTIVATE_RX_CHANNEL3,			// PAD 7
	CAPTIVATE_RX_CHANNEL7,			// PAD 8
    APP_WTR_NUM_PADS,
    APP_WTR_LOWEST_PAD = CAPTIVATE_RX_CHANNEL2
}APP_WTR_PAD_CHANNELS_T;

extern uint8_t APP_WTR_CheckLevel(void);
extern uint16_t APP_WTR_GetPadValue(APP_WTR_PAD_CHANNELS_T pad);
extern const char * APP_WTR_GetStrForPad(APP_WTR_PAD_CHANNELS_T pos);
extern void APP_WTR_CollectData(uint32_t ticks);

#define APP_WTR_GetNumPads() (APP_WTR_NUM_PADS)

#endif /* APP_WTR_H */
