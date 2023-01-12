/**************************************************************************************************
* \file     HW_TERM.c
* \brief    Hardware - Send and receive characters over UART for CLI and logging.
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

#include "HW_TERM.h"

#include <msp430.h>                      // Generic MSP430 Device Include
#include "driverlib.h"                   // MSPWare Driver Library
#include "APP_WTR.h"
#include "HW_ENV.h"
#include <stdio.h>
#include <string.h>
#include "uC_UART.h"
#include <limits.h>
#include "APP_NVM.h"
#include "uC_TIME.h"
#include "HW_RTC.h"
#include "HW_MAG.h"
#include "APP_CLI.h"
#include "version.h"
#include "version-git-info.h"
#include "HW_TERM.h"

#define MAX_STRING_SIZE         80

void HW_TERM_Init(void);
void HW_TERM_Print(uint8_t * p_str);
void HW_TERM_GetCommand(uint8_t * p_buff);
bool HW_TERM_CommandRdy(void);
void HW_TERM_ReportPadValues(void);
void HW_TERM_StoreByte(uint8_t byte);
void HW_TERM_SetCommandReady(void);
void HW_TERM_RxByte(uint8_t byte);
void HW_TERM_DisableOrEnableLogging(bool isEnabled);
bool HW_TERM_IsLoggingEnabled(void);

static uint8_t HW_TERM_Rx_Buf[HW_TERM_RX_BUF_LEN];
static uint8_t HW_TERM_Rx_Buf_Idx = 0;
static bool    HW_TERM_Command_Rdy = false;
static bool    HW_TERM_Logging_Is_Enabled = false;  // If true, logging is enabled

bool HW_TERM_CommandRdy(void)
{
    return HW_TERM_Command_Rdy;
}

void HW_TERM_RxByte(uint8_t byte)
{
    HW_TERM_Rx_Buf[HW_TERM_Rx_Buf_Idx] = byte;

    if (( HW_TERM_Rx_Buf[HW_TERM_Rx_Buf_Idx] == '\r' )  || // If we received the carriage return or linefeed then prep command.
        ( HW_TERM_Rx_Buf[HW_TERM_Rx_Buf_Idx] == '\n' ))
    {
        HW_TERM_Rx_Buf[HW_TERM_Rx_Buf_Idx] = 0;  // Apply a null terminator instead of the CR/LF
        HW_TERM_Rx_Buf_Idx = 0;                  // Reset our index.
        HW_TERM_Command_Rdy = true;              // Mark the command as ready for processing.
    }
    else
    {
        if (HW_TERM_Rx_Buf_Idx < (HW_TERM_RX_BUF_LEN - 1)) // Don't write input beyond the end of the command buffer.
            HW_TERM_Rx_Buf_Idx++;
    }
}

void HW_TERM_DisableOrEnableLogging(bool isEnabled)
{
    HW_TERM_Logging_Is_Enabled = isEnabled;
}

bool HW_TERM_IsLoggingEnabled(void)
{
    return HW_TERM_Logging_Is_Enabled;
}

void HW_TERM_PrintVersionInfo(void)
{
    uint8_t result[MAX_STRING_SIZE];

    // Print version information
    HW_TERM_Print("\n\n*****************************************************\n");
    HW_TERM_Print(" SSM APPLICATION\n");
    sprintf((char *)result, " Release Version: %d.%d.%d\n", VERSION_MAJOR, VERSION_MINOR, VERSION_BUILD);
    HW_TERM_Print(result);
    snprintf((char *)result, MAX_STRING_SIZE, " Build Version: %s %s %s\n", GIT_BRANCH_NAME, GIT_COMMIT_HASH, BUILD_TIMESTAMP_UTC);
    HW_TERM_Print(result);
    HW_TERM_Print("*****************************************************\n\n");
}

void HW_TERM_Init(void)
{
    // Initialize debug verbosity, reporting, etc.
}

void HW_TERM_GetCommand(uint8_t * p_buff)
{
    strcpy((char *)p_buff, (char *)HW_TERM_Rx_Buf);
    HW_TERM_Command_Rdy = false;
}

// Print a null-terminated string to the terminal.
void HW_TERM_Print(uint8_t * p_str)
{
    if ( HW_TERM_IsLoggingEnabled() == true )
    {
        uint16_t len = strlen((const char *)p_str);
        uC_UART_Tx(p_str, len);
    }
}

//// Print a null-terminated string to the terminal using color.
//// Available colors:
////      - KNRM
////      - KRED
////      - KGRN
////      - KCYN
//void HW_TERM_PrintColor(uint8_t * p_str, uint8_t * p_color)
//{
//    if ( HW_TERM_IsLoggingEnabled() == true )
//    {
//        uint8_t len = strlen((const char *)p_color);
//        uC_UART_Tx(p_color, len);
//
//        len = strlen((const char *)p_str);
//        uC_UART_Tx(p_str, len);
//
//        len = strlen(KNRM);
//        uC_UART_Tx(KNRM, len);
//    }
//}


void HW_TERM_ReportPadValues(void)
{
    APP_WTR_PAD_CHANNELS_T i = APP_WTR_LOWEST_PAD;
    uint8_t str[20];

    for(i=APP_WTR_LOWEST_PAD; i < APP_WTR_GetNumPads(); i++)
    {
       //sprintf((char *)str, "\n\rPad %s:   %d", APP_WTR_GetStrForPad(i), APP_WTR_GetPadValue(i));
       sprintf((char *)str, "\n\rPad %d:   %d", i+1, APP_WTR_GetPadValue(i));
       HW_TERM_Print(str);
    }
}
