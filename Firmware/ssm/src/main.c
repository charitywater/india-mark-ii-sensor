/**************************************************************************************************
* \file     main.c
* \brief    Initialize hardware and peripherals and run each module. Does not exit the main loop.
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

#include <msp430.h>                      // Generic MSP430 Device Include
#include "driverlib.h"                   // MSPWare Driver Library
#include "captivate.h"                   // CapTIvate Touch Software Library
#include "CAPT_App.h"                    // CapTIvate Application Code
#include "CAPT_BSP.h"                    // CapTIvate EVM Board Support Package
#include "I2CSlave_Definitions.h"
#include "HW.h"
#include "HW_TERM.h"
#include "HW_EEP.h"
#include "HW_MAG.h"
#include "APP_CLI.h"
#include "APP.h"
#include "HW_BAT.h"
#include "uC.h"
#include "APP_NVM.h"
#include "uC_TIME.h"
#include "HW_RTC.h"
#include "version.h"
#include "version-git-info.h"
#include "HW_AM.h"
#include "APP_WTR.h"
#include "am-ssm-spi-protocol.h"
#include "HW_GPIO.h"

void main(void)
{
    //
    // Initialize the MCU
    // BSP_configureMCU() sets up the device IO and clocking
    // The global interrupt enable is set to allow peripherals
    // to wake the MCU.
    //
    WDTCTL = (WDTPW | WDTHOLD);
    BSP_configureMCU();
    uC_Init();

    //init the battery module and take a new batt level reading
    HW_BAT_Init();

    //dont start the main loop if the battery is too low to prevent corrupting EEPROM
    if ( HW_BAT_IsBatteryLow() != true )
    {
        HW_Init();

        __bis_SR_register(GIE);

        //log the version info
        HW_TERM_PrintVersionInfo();

        //
        // Start the CapTIvate application
        //
        CAPT_appStart();

        if (BSP_XT1_Good() == true)
        {
            HW_TERM_Print("External oscillator good.\n");
        }
        else
        {
            HW_TERM_Print("External oscillator FAILED!\n");
        }

        APP_init();

        //
        // Background Loop
        //
        while(1)
        {
            // Run the captivate application handler.
            CAPT_appHandler();

            //check periodically if we have reached a critical level - if so, do nothing
            //to prevent the cell modem from being turned on (waking the AM)
            if ( HW_BAT_IsBatteryLow() != true )
            {
                //check for oscillator errors
                HW_RTC_Monitor();

                //Check for UART rx chars
                APP_CLI_Periodic();

                //check for SPI comm
                ASP_SSM_Periodic();

                //monitor the magnetometer
                HW_MAG_Monitor();

                //let the app run
                APP_periodic();
            }

            // End of background loop iteration
            // Go to sleep if there is nothing left to do
            CAPT_appSleep();
        } // End background loop
    }
    else
    {
        HW_TERM_Print("CRITICALLY LOW BATTERY LEVEL - DO NOTHING");
    }


} // End main()

#ifdef ENGINEERING_DATA

static void CollectData(uint32_t ticks)
{
    static bool sample_discarded = false;

    if(APP_CLI_CollectingData() == true)
    {
        // Ignore first sample since the CLI command can come in in the middle of a 100ms period, which is our tick resolution.
        // This ensures timing between first and second sample.
        if (sample_discarded == true)
        {
            APP_WTR_CollectData(ticks);
        }
        else
        {
            sample_discarded = true;
        }
    }
    else
    {
        sample_discarded = false;
    }
}

#endif
