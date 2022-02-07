/**************************************************************************************************
* \file     sara_u201.c
* \brief    API to issue commands to the Ublox Sara-u201 cellular module
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

#include "stddef.h"
#include "stdint.h"
#include "stdarg.h"
#include "string.h"
#include "stdio.h"
#include <stm32l4xx_hal.h>
#include "CLI.h"
#include "uart.h"
#include "FreeRTOS.h"
#include "task.h"
#include "logTypes.h"
#include "sara_u201.h"

#define SARA_PWR_ON_PIN         GPIO_PIN_9
#define SARA_PWR_ON_PORT        GPIOD

#define SARA_RESET_PIN          GPIO_PIN_11
#define SARA_RESET_PORT         GPIOA

#define LVL_SHIFTER_PIN         GPIO_PIN_12
#define LVL_SHIFTER_PORT        GPIOA

#define MAX_CMD_LEN             50

static char cmdBuf[MAX_CMD_LEN];

static void setPwrPin(void);
static void clearPwrPin(void);
static void setResetPin(void);
static void clearResetPin(void);
static void xSendAtCmd(char* msg, ...);
static void commandHandlerForSaraU2(int argc, char **argv);

void SARA_initHardware(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* Init the GPIO pins used to supply power to the module */
    HAL_GPIO_WritePin(SARA_PWR_ON_PORT, SARA_PWR_ON_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(LVL_SHIFTER_PORT, LVL_SHIFTER_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(SARA_RESET_PORT, SARA_RESET_PIN, GPIO_PIN_SET);

    /* set up the cell power pin */
    GPIO_InitStruct.Pin = SARA_PWR_ON_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(SARA_PWR_ON_PORT, &GPIO_InitStruct);

    /* set up the cell reset pin */
    GPIO_InitStruct.Pin = SARA_RESET_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(SARA_RESET_PORT, &GPIO_InitStruct);

    /* set up the level shifter power pin */
    GPIO_InitStruct.Pin = LVL_SHIFTER_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(LVL_SHIFTER_PORT, &GPIO_InitStruct);

    //register a command line handler
    CLI_Command_Handler_s cmdHandler;
    cmdHandler.ptrFunction = &commandHandlerForSaraU2;
    cmdHandler.cmdString   = "cell";
    cmdHandler.usageString = "\n\r\tclear [reset | pwr ] \n\r\tset [reset | pwr ] \n\r\tsim \n\r\trssi";
    CLI_registerThisCommandHandler(&cmdHandler);
}

//This function assumes that the 4V regulator has already been enabled to supply power to
//the modem.
void SARA_turnOnSequence(void)
{
    HAL_GPIO_WritePin(LVL_SHIFTER_PORT, LVL_SHIFTER_PIN, GPIO_PIN_SET);
    vTaskDelay(100);

    HAL_GPIO_WritePin(SARA_RESET_PORT, SARA_RESET_PIN, GPIO_PIN_RESET);
    vTaskDelay(100);
    HAL_GPIO_WritePin(SARA_PWR_ON_PORT, SARA_PWR_ON_PIN, GPIO_PIN_RESET);

    vTaskDelay(4000);
}

void SARA_initDataMode(void)
{
    elogInfo("Put into data mode");

    //starts ppp (data call, make sure APN is set)
    xSendAtCmd("ATD*99#");
    vTaskDelay(1000);

    elogInfo("Finished at command sequence");
}

void SARA_initAndSetApn(void)
{
    xSendAtCmd("AT+CPIN?");
    vTaskDelay(3000);

    xSendAtCmd("AT+CCID?");
    vTaskDelay(1000);

    xSendAtCmd("AT+CMUX?");
    vTaskDelay(1000);

    for (uint8_t i = 0; i< 2; i++ )
    {
       vTaskDelay(10000);
       xSendAtCmd("AT+CREG?");
    }

    xSendAtCmd("AT+CSQ=?");

    vTaskDelay(3000);

    xSendAtCmd("AT+CTZR?");
    vTaskDelay(3000);

    xSendAtCmd("AT+CGDCONT=1,\"IP\",\"iot-eu.aer.net\"");
    vTaskDelay(10000);
}

void SARA_sendNwRegistrationCmd(void)
{
    xSendAtCmd("AT+CGREG?");
}

void SARA_getRssi(void)
{
    //poll rssi
    xSendAtCmd("AT+CSQ");
    vTaskDelay(2000);
}

void SARA_getImei(void)
{
    xSendAtCmd("AT+CGSN");
    vTaskDelay(2000);
}

void SARA_get_Iccid(void)
{
    xSendAtCmd("AT+CCID");
    vTaskDelay(2000);
}

void SARA_getModemVersion(void)
{
    xSendAtCmd("AT+CGMR");
    vTaskDelay(2000);
}

void SARA_startTxTest(uint32_t channel, int8_t power)
{
    //deregister
    xSendAtCmd("AT+cops=2");
    vTaskDelay(5000);

    //enable utest
    xSendAtCmd("AT+UTEST=1");
    vTaskDelay(10000);

    //send actual utest command
    xSendAtCmd("AT+UTEST=3,%lu,%d,,,0", channel, power);
    vTaskDelay(2000);
}

void SARA_getSimId(void)
{
    elogDebug("Check SIM card");

    xSendAtCmd("ATE0");
    vTaskDelay(1000);

    /* Query the SIM card and network status */
    xSendAtCmd("AT+CGSN");
    vTaskDelay(300);

    xSendAtCmd("AT+CREG=1");
    vTaskDelay(300);

    xSendAtCmd("AT+CFUN?");
    vTaskDelay(300);

    xSendAtCmd("AT+COPS?");
    vTaskDelay(300);
}

/* format strings to send to the cell module
 * Responses are received and parsed in the nwStackFunctionality.c file, since modem
 * responses can be in AT command format OR tcp packets
 */
static void xSendAtCmd(char* msg, ...)
{
    va_list argp;

    /** - Format the string */
    va_start(argp, msg );
    vsnprintf(&cmdBuf[0], sizeof(cmdBuf), msg, argp);
    va_end(argp);
    strcat(cmdBuf, "\r\n");

    /* Send message to the cell/gps module */
    UART_sendDataNonBlockingWithDma(CELLULAR, (uint8_t*)cmdBuf,  strlen(cmdBuf));
}

static void setPwrPin(void)
{
    HAL_GPIO_WritePin(SARA_PWR_ON_PORT, SARA_PWR_ON_PIN, GPIO_PIN_SET);
}

static void clearPwrPin(void)
{
    HAL_GPIO_WritePin(SARA_PWR_ON_PORT, SARA_PWR_ON_PIN, GPIO_PIN_RESET);
}

static void setResetPin(void)
{
    HAL_GPIO_WritePin(SARA_RESET_PORT, SARA_RESET_PIN, GPIO_PIN_SET);
}

static void clearResetPin(void)
{
    HAL_GPIO_WritePin(SARA_RESET_PORT, SARA_RESET_PIN, GPIO_PIN_RESET);
}

static void commandHandlerForSaraU2(int argc, char **argv)
{
    if ((argc == TWO_ARGUMENTS) && (0 == strcmp(argv[FIRST_ARG_IDX], "set")))
    {
        if ( 0 == strcmp(argv[SECOND_ARG_IDX], "reset") )
        {
           elogDebug("setting cell reset pin");
           setResetPin();

        }
        else if ( 0 == strcmp(argv[SECOND_ARG_IDX], "pwr") )
        {
            elogDebug("setting cell pwr pin");
            setPwrPin();
        }
        else
        {
           elogDebug("Invalid arg");
        }
    }
    else if ((argc == TWO_ARGUMENTS) && (0 == strcmp(argv[FIRST_ARG_IDX], "clear")))
    {
        if ( 0 == strcmp(argv[SECOND_ARG_IDX], "reset") )
        {
           elogDebug("clearing cell pwr pin");
           clearResetPin();
        }
        else if ( 0 == strcmp(argv[SECOND_ARG_IDX], "pwr") )
        {
            elogDebug("clearing cell pwr pin");
            clearPwrPin();
        }
        else
        {
           elogDebug("Invalid arg");
        }
    }
    else if ((argc == ONE_ARGUMENT) && (0 == strcmp(argv[FIRST_ARG_IDX], "sim")))
    {
        SARA_getSimId();
    }
    else if  ((argc == ONE_ARGUMENT) && (0 == strcmp(argv[FIRST_ARG_IDX], "rssi")))
    {
        xSendAtCmd("AT+CSQ=?");
    }
    else
    {
        elogDebug("Unknown Command");
    }
}
