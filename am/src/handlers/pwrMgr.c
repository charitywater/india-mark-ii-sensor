/**************************************************************************************************
* \file     pwrMgr.c
* \brief    Enable STANDY mode, wakeup source and any pre or post settings around this low power mode
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

#include "string.h"
#include "stdbool.h"
#include "logTypes.h"
#include "CLI.h"
#include "uart.h"
#include "i2c.h"
#include "spi.h"
#include "stm32l4xx_hal.h"
#include "pwrMgr.h"

#define TPS22_ON_PIN        GPIO_PIN_6
#define TPS22_ON_PORT       GPIOC

// the above 2 pins PLUS this pin is required to go high to turn on GPS
#define GPS_PWR_EN_PIN      GPIO_PIN_1
#define GPS_PWR_EN_PORT     GPIOB

#define GPRS_IND_LED_PIN    GPIO_PIN_10
#define GPRS_IND_LED_PORT   GPIOD

static bool isCellModemPowered = false;
static bool isGpsPowered = false;

static void commandHandlerForPwr(int argc, char **argv);

void PWR_init(void)
{
    /* Check if the system was resumed from Standby mode */
    if ( __HAL_PWR_GET_FLAG(PWR_FLAG_SB) != RESET )
    {
      /* Clear Standby flag */
      __HAL_PWR_CLEAR_FLAG(PWR_FLAG_SB);

      /* log it to the terminal for now */
      elogInfo("Woke up from STANDBY Mode");
    }

    /* Disable wakeup sources: PWR_WAKEUP_PIN2, PC13 */

    /* Actually not sure if we want to disable it or not */
    HAL_PWR_DisableWakeUpPin(PWR_WAKEUP_PIN2);

    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* initialize the remaining power supply pins */
    GPIO_InitStruct.Pin = TPS22_ON_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(TPS22_ON_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPS_PWR_EN_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPS_PWR_EN_PORT, &GPIO_InitStruct);

    //set LED pin as input to prevent it from turning on
    //accidentally/drwing extra current
    GPIO_InitStruct.Pin = GPRS_IND_LED_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPRS_IND_LED_PORT, &GPIO_InitStruct);

    /* init in the OFF position */
    HAL_GPIO_WritePin(TPS22_ON_PORT, TPS22_ON_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPS_PWR_EN_PORT, GPS_PWR_EN_PIN, GPIO_PIN_RESET);

    /*register a command handler function */
    CLI_Command_Handler_s cmdHandler;
    cmdHandler.ptrFunction = &commandHandlerForPwr;
    cmdHandler.cmdString   = "pwr";
    cmdHandler.usageString = "\n\r\tsleep \n\r\tgps [0 1] \n\r\tsw [ 0 1] \n\r\tled [0 1]";
    CLI_registerThisCommandHandler(&cmdHandler);
}

void PWR_enterStandbyMode(void)
{
    /* STANDBY MODE INIT & WAKE ON PC13 */

    /* FROM example Proj: The Following Wakeup sequence is highly recommended prior to each Standby mode entry
       mainly when using more than one wakeup source this is to not miss any wakeup event.
        - Disable all used wakeup sources,
        - Clear all related wakeup flags,
        - Re-enable all used wakeup sources,
        - Enter the Standby mode.
     */

    //Deinitialize I2C, UART, SPI
    UART_deinitDebugPeripherals();
    I2C_DeInit();
    SPI_DeInit();

    //de-init FLASH output pins and set to high z (they have external pulls ups)
    HAL_GPIO_DeInit(GPIOE, GPIO_PIN_9);
    HAL_GPIO_DeInit(GPIOE, GPIO_PIN_10);
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_4);

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_10 |GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_4;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    //Enable pull DOWNS in sleep mode for pins that are inputs
    //on other devices

    HAL_PWREx_EnableGPIOPullDown(PWR_GPIO_A, GPIO_PIN_5);
    HAL_PWREx_EnableGPIOPullDown(PWR_GPIO_A, GPIO_PIN_6);
    HAL_PWREx_EnableGPIOPullDown(PWR_GPIO_A, GPIO_PIN_7);

    HAL_PWREx_EnableGPIOPullDown(PWR_GPIO_B, GPIO_PIN_13);
    HAL_PWREx_EnableGPIOPullDown(PWR_GPIO_B, GPIO_PIN_15);

    HAL_PWREx_EnableGPIOPullDown(PWR_GPIO_E, GPIO_PIN_7);
    HAL_PWREx_EnableGPIOPullDown(PWR_GPIO_A, GPIO_PIN_0);
    HAL_PWREx_EnableGPIOPullDown(PWR_GPIO_C, GPIO_PIN_12);
    HAL_PWREx_EnableGPIOPullDown(PWR_GPIO_D, GPIO_PIN_2);
    HAL_PWREx_EnableGPIOPullDown(PWR_GPIO_B, GPIO_PIN_5);
    HAL_PWREx_EnableGPIOPullDown(PWR_GPIO_B, GPIO_PIN_4);

    HAL_PWREx_EnablePullUpPullDownConfig();

    /* Enable Power Clock */
    __HAL_RCC_PWR_CLK_ENABLE();

    /* Disable all used wakeup sources: PWR_WAKEUP_PIN2 */
    HAL_PWR_DisableWakeUpPin(PWR_WAKEUP_PIN2);

    /* Clear all related wakeup flags*/
    __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);

    /* Enable WakeUp Pin PWR_WAKEUP_PIN2 connected to PC.13, high polarity */
    HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN2);

    /* Enter STANDY MODE, should draw about 535 nA in this mode ---seeing 300 nA*/
    HAL_PWR_EnterSTANDBYMode();

    /***** Now the system should be SLEEPING in STANDY mode - when
           * woken up, the application will begin from main ******/
}

bool PWR_turnOnGpsPowerSupply(void)
{
    if ( isCellModemPowered == false )
    {
        //turn on the cell modem
        PWR_turnOnCellModemPowerSupply();
    }

    //now enable the gps power supply
    HAL_GPIO_WritePin(GPS_PWR_EN_PORT, GPS_PWR_EN_PIN, GPIO_PIN_SET);
    isGpsPowered = true;

    return isGpsPowered;
}

bool PWR_turnOffGpsPowerSupply(void)
{
    HAL_GPIO_WritePin(GPS_PWR_EN_PORT, GPS_PWR_EN_PIN, GPIO_PIN_RESET);
    isGpsPowered = false;

    return isGpsPowered;
}

bool PWR_turnOnCellModemPowerSupply(void)
{
    //turn on if not already on...
    if ( isCellModemPowered == false )
    {
        /* Step 1: Turn on the switch to power the boost, let stabilize */
        HAL_GPIO_WritePin(TPS22_ON_PORT, TPS22_ON_PIN, GPIO_PIN_SET);
        HAL_Delay(300);

        //update state
        isCellModemPowered = true;
    }

    return isCellModemPowered;
}

bool PWR_turnOffCellModemPowerSupply(void)
{
    //make sure the gps is turned off..
    if ( true == isGpsPowered )
    {
        PWR_turnOffGpsPowerSupply();
    }

    /* Step 1: Turn off the switch to power the boost */
    HAL_GPIO_WritePin(TPS22_ON_PORT, TPS22_ON_PIN, GPIO_PIN_RESET);

    //update state
    isCellModemPowered = false;
    return isCellModemPowered;
}

bool PWR_getGpsPwrState(void)
{
    return isGpsPowered;
}

bool PWR_getCellPwrState(void)
{
    return isCellModemPowered;
}


static void commandHandlerForPwr(int argc, char **argv)
{
    if ( argc == ONE_ARGUMENT && 0 == strcmp(argv[FIRST_ARG_IDX], "sleep") )
    {
        elogInfo("Putting AM to sleep...");

        PWR_enterStandbyMode();
    }
    else if ( argc == TWO_ARGUMENTS && 0 == strcmp(argv[FIRST_ARG_IDX], "gps") )
    {
        if ( 0 == strcmp(argv[SECOND_ARG_IDX], "0") )
        {
           elogDebug("clearing gps pwr en pin");
           HAL_GPIO_WritePin(GPS_PWR_EN_PORT, GPS_PWR_EN_PIN, GPIO_PIN_RESET);
        }
        else if ( 0 == strcmp(argv[SECOND_ARG_IDX], "1") )
        {
           elogDebug("setting gps pwr en pin");
           HAL_GPIO_WritePin(GPS_PWR_EN_PORT, GPS_PWR_EN_PIN, GPIO_PIN_SET);
        }
        else
        {
            elogDebug("invalid arg");
        }
    }
    else if ( argc == TWO_ARGUMENTS && 0 == strcmp(argv[FIRST_ARG_IDX], "sw") )
    {
        if ( 0 == strcmp(argv[SECOND_ARG_IDX], "0") )
        {
           elogDebug("clearing pwr switch on pin");
           HAL_GPIO_WritePin(TPS22_ON_PORT, TPS22_ON_PIN, GPIO_PIN_RESET);

        }
        else if ( 0 == strcmp(argv[SECOND_ARG_IDX], "1") )
        {
           elogDebug("setting pwr switch on pin");
           HAL_GPIO_WritePin(TPS22_ON_PORT, TPS22_ON_PIN, GPIO_PIN_SET);
        }
        else
        {
            elogDebug("invalid arg");
        }
    }
    else if ( argc == TWO_ARGUMENTS && 0 == strcmp(argv[FIRST_ARG_IDX], "led") )
    {
        if ( 0 == strcmp(argv[SECOND_ARG_IDX], "0") )
        {
           elogDebug("clearing GPRS LED pin");
           GPIO_InitTypeDef GPIO_InitStruct = {0};
           GPIO_InitStruct.Pin = GPRS_IND_LED_PIN;
           GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
           GPIO_InitStruct.Pull = GPIO_NOPULL;
           GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
           HAL_GPIO_Init(GPRS_IND_LED_PORT, &GPIO_InitStruct);
           HAL_GPIO_WritePin(GPRS_IND_LED_PORT, GPRS_IND_LED_PIN, GPIO_PIN_RESET);

        }
        else if ( 0 == strcmp(argv[SECOND_ARG_IDX], "1") )
        {
           elogDebug("setting GPRS LED pin");

           GPIO_InitTypeDef GPIO_InitStruct = {0};
           GPIO_InitStruct.Pin = GPRS_IND_LED_PIN;
           GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
           GPIO_InitStruct.Pull = GPIO_NOPULL;
           GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
           HAL_GPIO_Init(GPRS_IND_LED_PORT, &GPIO_InitStruct);
           HAL_GPIO_WritePin(GPRS_IND_LED_PORT, GPRS_IND_LED_PIN, GPIO_PIN_SET);

        }
        else
        {
            elogDebug("invalid arg");
        }
    }
    else
    {
        elogDebug("invalid cli command");
    }
}
