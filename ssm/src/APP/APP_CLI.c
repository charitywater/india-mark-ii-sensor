/**************************************************************************************************
* \file     APP_CLI.c
* \brief    Command line interface
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

#include "HW_GPIO.h"
#include "commandLineDriver.h"
#include "lis2mdl/lis2mdl_reg.h"
#include "version.h"
#include "HW_MAG.h"
#include "HW_TERM.h"
#include "HW_ENV.h"
#include "HW_RTC.h"
#include "HW_EEP.h"
#include <stdio.h>
#include <version-git-info.h>
#include "uC_I2C.h"
#include "APP_CLI.h"
#include "CAPT_App.h"
#include "APP_WTR.h"
#include "APP_NVM.h"
#include "HW_BAT.h"
#include "uC_UART.h"
#include "uC_TIME.h"
#include "APP_ALGO.h"

#ifdef ENGINEERING_DATA
const APP_NVM_SENSOR_DATA_T Test_Sensor_Data =
{
    .timestamp = 0x6E000000,
    .pads = {500,501,502,503,504,505},
    .checksum = 0
};
#else
#pragma PERSISTENT (Test_Sensor_Data)
APP_NVM_SENSOR_DATA_T Test_Sensor_Data =
{
    .timestamp = 0x5E000000,
    .litersPerHour = {0,10,0,0,1254,0,0,0,0,0,0,0,33,0,0,0,0,0,0,444,0,0,0,0},
    .tempPerHour = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    .humidityPerHour = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    .strokesPerHour = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    .strokeHeightPerHour = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    .avgLiters = 0,
    .dailyLiters = 0,
    .totalLiters = 50,
    .breakdown = false,
    .pumpCapacity = 0,
    .batteryVoltage = 3600,
    .powerRemaining = 100,
    .state = FAULT,
    .magnetDetected = true,
    .errorBits = 0,
    .unexpectedResets = 0,
    .timestampOfLastReset = 0,
    .activatedDate = 1597347495
};
#endif

void APP_CLI_Periodic(void);
void APP_CLI_Init(void);
void APP_CLI_Print(uint8_t * pubStr);
bool APP_CLI_CollectingData(void);

static void HandleRTC(int argc, char **argv);
static void HandleEEP(int argc, char **argv);
static void HandleReadDiscrete(int argc, char **argv);
static void HandleWriteDiscrete(int argc, char **argv);
static void HandleRuntime(int argc, char **argv);
static void HandleTerm(int argc, char **argv);
static void HandleSensorData(int argc, char **argv);
static void HandleReset(int argc, char **argv);
static void HandleAm(int argc, char **argv);
static void HandleAlgorithm(int argc, char **argv);
static void PrintPrompt(void);
static void HandleWdKickSettings(int argc, char **argv);
static void HandleBatt(int argc, char **argv);
static void HandleEnv(int argc, char **argv);
static void HandleMag(int argc, char **argv);

static uint8_t APP_CLI_CommandBuffer[HW_TERM_RX_BUF_LEN];
static bool Collecting_Data = false;

#define APP_CLI_PROMPT  "\nssm_cli> "

void APP_CLI_Init(void)
{
    CLD_Command_Handler_s handler;

    gvCLD_Initialize();

    handler.pfnPtrFunction = &HandleWdKickSettings;
    handler.pszCmdString   = "kick";
    handler.pszUsageString = "{\"on\"|\"off\"} - Turn wd kick on or off.";
    gvCLD_Register_This_Command_Handler(&handler);

    handler.pfnPtrFunction = &HandleBatt;
    handler.pszCmdString   = "batt";
    handler.pszUsageString = "read - reads current voltage| \n \
    \t comm - check communication w/ the fuel gauge| \n \
    \t serial - read 64 bit serial num of fuel gauge";
    gvCLD_Register_This_Command_Handler(&handler);

    handler.pfnPtrFunction = &HandleRTC;
    handler.pszCmdString   = "rtc";
    handler.pszUsageString = "{\t\"rt\"  | \n \
    \t\"st\" <10ms> <sec 0-59> <min 0-59> <hour 0-23> <day 1-7> <date 1-31> <month 1-12> <year> | \n \
    \t  - M41T62: Read Time, Set Time";
    gvCLD_Register_This_Command_Handler(&handler);

    handler.pfnPtrFunction = &HandleEEP;
    handler.pszCmdString   = "eep";
    handler.pszUsageString = "{\"e\" | \"v\" | \"d\" | \"pattern [write | read]\"} - Erase/Validate/Default EEPROM or write/read the test pattern";
    gvCLD_Register_This_Command_Handler(&handler);

    handler.pfnPtrFunction = &HandleReadDiscrete;
    handler.pszCmdString   = "rd";
    handler.pszUsageString = "{<port.pin>|\"all\"} - read the discrete.";
    gvCLD_Register_This_Command_Handler(&handler);

    handler.pfnPtrFunction = &HandleWriteDiscrete;
    handler.pszCmdString   = "wd";
    handler.pszUsageString = "<port.pin> {\"high\"|\"low\"} - Write the discrete to the provided value.";
    gvCLD_Register_This_Command_Handler(&handler);

    handler.pfnPtrFunction = &HandleTerm;
    handler.pszCmdString   = "log";
    handler.pszUsageString = "{\"enable\"|\"disable\"} - Turn printing on or off.";
    gvCLD_Register_This_Command_Handler(&handler);

    handler.pfnPtrFunction = &HandleRuntime;
    handler.pszCmdString   = "runt";
    handler.pszUsageString = "Run time. {\"set\" <seconds> } - Display or set run time.";
    gvCLD_Register_This_Command_Handler(&handler);

    handler.pfnPtrFunction = &HandleSensorData;
    handler.pszCmdString   = "SensorData";
    handler.pszUsageString = "\"add\" <num sensor data logs> - log dummy data | \n \
    \t\"ds\" - default sensor data logs | \n \
    \t\"redflag\" - indicate new red flag | \n \
     \t\"fake\"";
    gvCLD_Register_This_Command_Handler(&handler);

    handler.pfnPtrFunction = &HandleReset;
    handler.pszCmdString   = "reset";
    handler.pszUsageString = "Reset the SSM.";
    gvCLD_Register_This_Command_Handler(&handler);

    handler.pfnPtrFunction = &HandleAm;
    handler.pszCmdString   = "am";
    handler.pszUsageString = " \"act\" - send activation | \n \
    \t\"check\" - check in | \n \
    \t\"time\" -  set RTC time status: 0-first, 1-periodic, 2-updated";
    gvCLD_Register_This_Command_Handler(&handler);

    handler.pfnPtrFunction = &HandleAlgorithm;
    handler.pszCmdString   = "algo";
    handler.pszUsageString = " \"watervolume\" - get water volume | \n \
       \t\"strokecount\" - get stroke count | \n \
       \t\"strokereset\" - reset stroke count | \n \
       \t\"rawdata\" - raw pad values";
    gvCLD_Register_This_Command_Handler(&handler);


    handler.pfnPtrFunction = &HandleEnv;
    handler.pszCmdString   = "env";
    handler.pszUsageString =  " \"sample\" - get temp and humidity";
    gvCLD_Register_This_Command_Handler(&handler);

    handler.pfnPtrFunction = &HandleMag;
    handler.pszCmdString   = "mag";
    handler.pszUsageString =  " \"sample\" - get latest magnetometer sample | \n \
     \t\"on\" - enable sampling";
    gvCLD_Register_This_Command_Handler(&handler);

    PrintPrompt();
}

void APP_CLI_Periodic(void)
{
    if(HW_TERM_CommandRdy() == true)
    {
        HW_TERM_GetCommand(APP_CLI_CommandBuffer);
        gvCLD_Invoke_The_Handler_For_This_Command((char *)APP_CLI_CommandBuffer, 30);

        PrintPrompt();
    }
}

static void HandleAm(int argc, char **argv)
{
    if ((argc == ONE_ARGUMENT) && (0 == strcmp(argv[FIRST_ARG_IDX], "act")))
    {
        APP_indicateActivation();
    }
    else if ((argc == ONE_ARGUMENT) && (0 == strcmp(argv[FIRST_ARG_IDX], "check")))
    {
        APP_indicateCheckIn();
    }
    else if ((argc == ONE_ARGUMENT) && (0 == strcmp(argv[FIRST_ARG_IDX], "deact")))
    {
        APP_handleDeactivateCmd();
    }
    else if ((argc == TWO_ARGUMENTS) && (0 == strcmp(argv[FIRST_ARG_IDX], "time")))
    {
        uint8_t status = strtoul(argv[SECOND_ARG_IDX], NULL, 10);

        if (status == RTC_TIME_UPDATED ||
            status == RTC_TIME_SYNC_PERIODIC ||
            status == RTC_FIRST_TIME_SYNC)
        {
            HW_TERM_Print("Updating RTC Time Status");
            APP_setTimeSyncStatus(status);
        }
        else
        {
            HW_TERM_Print("Invalid parameter.\n");
        }
    }
    else
    {
        HW_TERM_Print("Invalid parameter.\n");
    }
}

static void HandleReset(int argc, char **argv)
{
    HW_TERM_Print("Resetting.\n");
    HW_PerformSwReset();
}

static void HandleReadDiscrete(int argc, char **argv)
{
    uint8_t i = 0;
    bool found = false;

    if(argc == ONE_ARGUMENT)
    {
        for ( i = 0; i < HW_GPIO_NUM_DISCRETES; i++ )
        {
            if (( 0 == strcmp(argv[FIRST_ARG_IDX], DiscreteDesc[i].gpio_num_str)) ||  /** Look up which pin to read (or all) */
                ( 0 == strcmp(argv[FIRST_ARG_IDX], "all")) )
            {
                found = true;
                HW_TERM_Print("Reading P");
                HW_TERM_Print((uint8_t *)DiscreteDesc[i].gpio_num_str);
                HW_TERM_Print("-");
                HW_TERM_Print((uint8_t *)DiscreteDesc[i].signal_name);
                HW_TERM_Print(": ");
                if ( DiscreteDesc[i].read_func != NULL )
                {
                    if (  DiscreteDesc[i].read_func() == true )
                    {
                        HW_TERM_Print("High.\n");
                    }
                    else
                    {
                        HW_TERM_Print("Low.\n");
                    }
                }
                else
                {
                    HW_TERM_Print("Error.\n");
                }
            }
        }
        if( false == found )
        {
            HW_TERM_Print("Invalid parameter.\n");
        }
    }
}

static void HandleWriteDiscrete(int argc, char **argv)
{
    uint8_t i = 0;
    bool found = false;

    /** - Correct number of arguments passed */
    if(argc == TWO_ARGUMENTS)
    {
        /** - Look for this GPIO discrete in the discrete descriptors */
        for ( i = 0; i < HW_GPIO_NUM_DISCRETES; i++ )
        {
            if ( 0 == strcmp(argv[FIRST_ARG_IDX], DiscreteDesc[i].gpio_num_str) )
            {
                /** - Found a descriptor for this GPIO number. */
                found = true;

                /** - Make sure it is an output. */
                if (HW_GPIO_OUTPUT == DiscreteDesc[i].io )
                {
                    HW_TERM_Print("Writing P");
                    HW_TERM_Print((uint8_t *)DiscreteDesc[i].gpio_num_str);
                    HW_TERM_Print("-");
                    HW_TERM_Print((uint8_t *)DiscreteDesc[i].signal_name);
                    HW_TERM_Print(" to: ");

                    if( 0 == strcmp(argv[SECOND_ARG_IDX], "high"))
                    {
                        /** - Commanded high.  Make sure we have a set function then call it. */
                        if ( NULL != DiscreteDesc[i].set_func )
                        {
                            HW_TERM_Print("High.\n");
                            DiscreteDesc[i].set_func();
                        }
                        else
                        {
                            HW_TERM_Print("\nError. No accessor defined for this discrete.\n");
                        }
                    }
                    else if( 0 == strcmp(argv[SECOND_ARG_IDX], "low"))
                    {
                        /** - Commanded low.  Make sure we have a clear function then call it. */
                        if ( NULL != DiscreteDesc[i].clear_func )
                        {
                            HW_TERM_Print("Low.\n");
                            DiscreteDesc[i].clear_func();
                        }
                        else
                        {
                            HW_TERM_Print("\nError. No accessor defined for this discrete.\n");
                        }
                    }
                    else
                    {
                        HW_TERM_Print("Invalid parameter format.\n");
                    }
                }

                /** - You can't write to an input */
                else
                {
                    HW_TERM_Print("Not an output.\n");
                }
            }
        }
        /** - No descriptor for this GPIO */
        if( false == found )
        {
            HW_TERM_Print("GPIO not supported.\n");
        }
    }
    else
    {
        HW_TERM_Print("Invalid parameter format.\n");
    }
}

// Enable or disable test mode.  Test mode disables software activity for HW testing.
static void HandleWdKickSettings(int argc, char **argv)
{
    /** - Enable test mode. */
    if ((argc == ONE_ARGUMENT) && (strcmp(argv[FIRST_ARG_IDX], "on") == 0))
    {

            HW_TERM_Print("Enabling wd.");
            HW_WatchdogStopKick(true);
    }
    /** - Disable test mode. */
    else if ((argc == ONE_ARGUMENT) && (strcmp(argv[FIRST_ARG_IDX], "off") == 0))
    {

            HW_TERM_Print("Disabling wd");
            HW_WatchdogStopKick(false);
    }
    else
    {
        HW_TERM_Print("Invalid parameter format.");
    }
}

static void HandleEnv(int argc, char **argv)
{
    if ((argc == ONE_ARGUMENT) && (strcmp(argv[FIRST_ARG_IDX], "sample") == 0))
    {
        //trigger a new conversion
        HW_ENV_TriggerNewEnvSample();

        //short delay
        __delay_cycles(80000);

        //get latest samples and print them out
        HW_ENV_GetLatestSampleAndReport();
    }
    else
    {
        HW_TERM_Print("Invalid parameter format.");
    }
}

static void HandleMag(int argc, char **argv)
{
    if ((argc == ONE_ARGUMENT) && (strcmp(argv[FIRST_ARG_IDX], "sample") == 0))
    {
        //grab latest sample and print them out
        HW_MAG_SampleAndReport();

        //put MAG back in LP mode
        HW_MAG_TurnOffSampling();
    }
    else if ((argc == ONE_ARGUMENT) && (strcmp(argv[FIRST_ARG_IDX], "on") == 0))
    {
        //put magnetometer into correct mode - 20Hz sample rate
        HW_MAG_InitSampleRateAndPowerModeOn();
        HW_TERM_Print("Magnetometer is on");
    }
    else
    {
        HW_TERM_Print("Invalid parameter format.");
    }
}

static void HandleBatt(int argc, char **argv)
{
    uint8_t buff[40];
    if ((argc == ONE_ARGUMENT) && (strcmp(argv[FIRST_ARG_IDX], "read") == 0))
    {
        HW_BAT_TakeNewVoltageMeasurement();
        uint16_t voltage = HW_BAT_GetVoltage();
        sprintf((char *)buff, "\r\nCurrent Batt Voltage: %d mV", voltage);
        HW_TERM_Print(buff);
    }
    else if ((argc == ONE_ARGUMENT) && (strcmp(argv[FIRST_ARG_IDX], "comm") == 0))
    {
        //communicate w/ the fuel gauge
        HW_FUEL_GAUGE_Initialize();
    }
    else if ((argc == ONE_ARGUMENT) && (strcmp(argv[FIRST_ARG_IDX], "serial") == 0))
    {
        //print fuel fauge serial number
        HW_FUEL_GAUGE_PrintSerialNumber();
    }
    else
    {
        HW_TERM_Print("Invalid parameter format.");
    }
}

bool APP_CLI_CollectingData(void)
{
    return Collecting_Data;
}

static void HandleSensorData(int argc, char **argv)
{
    uint16_t i,num_reports;
    char str[20];

    #ifdef ENGINEERING_DATA
    static uint32_t start_ticks = 0;
    uint32_t end_ticks = 0;

    // log or logb
    if (argc == ZERO_ARGUMENTS)
    {
        if (Collecting_Data == true)
        {
            Collecting_Data = false;
            Binary_Data = false;
            HW_TERM_Print("\nStopped collecting data.\n");
            end_ticks = uC_TIME_GetRuntimeTicks();
            sprintf(str, "T: %lu\n", (end_ticks - start_ticks));
            HW_TERM_Print((uint8_t *)str);
        }
        else
        {
            Collecting_Data = true;
            HW_TERM_Print("Collecting data...\n");
            start_ticks = uC_TIME_GetRuntimeTicks();
        }
    }

    else if ((argc == TWO_ARGUMENTS) && (strcmp(argv[FIRST_ARG_IDX], "add") == 0))
#else
    if ((argc == TWO_ARGUMENTS) && (strcmp(argv[FIRST_ARG_IDX], "add") == 0))
#endif // ENGINEERING_DATA
    {
        num_reports = strtoul(argv[SECOND_ARG_IDX], NULL, 10);
        sprintf(str, "Logging %u reports\n", num_reports);
        HW_TERM_Print((uint8_t *)str);

        for(i = 0; i < num_reports; i++)
        {
            HW_TERM_PrintColor("Logging sensor data (test).\n", KMAG);
            APP_NVM_Custom_LogSensorData(&Test_Sensor_Data);
        }
    }
    else if ((argc == ONE_ARGUMENT) && (strcmp(argv[FIRST_ARG_IDX], "ds") == 0))
    {
        APP_NVM_DefaultSensorDataLogs();
    }
    else if ((argc == ONE_ARGUMENT) && (strcmp(argv[FIRST_ARG_IDX], "redflag") == 0))
    {
        HW_TERM_Print("New red flag \n");
        APP_setNewRedFlagDetected(true);
    }
    else if ((argc == ONE_ARGUMENT) && (strcmp(argv[FIRST_ARG_IDX], "fake") == 0))
    {
        HW_TERM_Print("Setting up fake map \n");
        APP_ALGO_populateRedFlagArrayWithFakeData();
    }
    else
    {
        HW_TERM_Print("Invalid parameter format.");
    }
}

// Read or Write from/to RTC
static void HandleRTC(int argc, char **argv)
{
    if (strcmp(argv[COMMAND_IDX], "rtc") == 0)
    {
        // read time
        if ((argc == ONE_ARGUMENT) && (strcmp(argv[FIRST_ARG_IDX], "rt") == 0))
        {
            HW_RTC_ReportTime();
        }
        // set time
        else if ((argc == NINE_ARGUMENTS) && (strcmp(argv[FIRST_ARG_IDX], "st") == 0))
        {
            if (HW_RTC_SetTime( strtoul(argv[SECOND_ARG_IDX], NULL, 10), // 10_ms
                                strtoul(argv[THIRD_ARG_IDX], NULL, 10), // seconds
                                strtoul(argv[FOURTH_ARG_IDX], NULL, 10), // minutes
                                strtoul(argv[FIFTH_ARG_IDX], NULL, 10), // hours
                                (HW_RTC_DAY_OF_WEEK_T) strtoul(argv[SIXTH_ARG_IDX], NULL, 10), // day_of_week
                                strtoul(argv[SEVENTH_ARG_IDX], NULL, 10), // date
                                (HW_RTC_MONTH_T) strtoul(argv[EIGHTH_ARG_IDX], NULL, 10), //month
                                strtoul(argv[NINTH_ARG_IDX], NULL, 10))  // year
                                == true)
            {
                HW_TERM_Print("Time set: \n");
                HW_RTC_ReportTime();
            }
            else
            {
                HW_TERM_Print("Invalid parameter format.");
            }
        }
        else
        {
            HW_TERM_Print("Invalid parameter format.");
        }
    }
    else
    {
        HW_TERM_Print("Invalid parameter format.");
    }
}


// Read or Write from/to EEP I2C slave.
static void HandleEEP(int argc, char **argv)
{
    uint8_t testPatternWrite[4] = {0xAA, 0xBB, 0xAB, 0xBA};
    uint8_t testPatternRead[4] = {};
    uint8_t patternIdx = 0;
    uint16_t addr = 0x3000; // pick an address outside of the sections being used

    if ((argc == ONE_ARGUMENT) && (strcmp(argv[FIRST_ARG_IDX], "e") == 0))
    {
        HW_TERM_Print("Erasing all EEPROM...");
        HW_EEP_EraseAll();
        HW_TERM_Print("EEPROM Erased");
    }
    else if ((argc == ONE_ARGUMENT) && (strcmp(argv[FIRST_ARG_IDX], "v") == 0))
    {
        HW_TERM_Print("Validating NVM structures...\n");
        APP_NVM_Validate();
    }
    else if ((argc == ONE_ARGUMENT) && (strcmp(argv[FIRST_ARG_IDX], "d") == 0))
    {
        HW_TERM_Print("Defaulting NVM structures...\n");
        APP_NVM_DefaultAll();
    }
    else if ((argc == TWO_ARGUMENTS) && (strcmp(argv[FIRST_ARG_IDX], "pattern") == 0))
    {
        if (strcmp(argv[SECOND_ARG_IDX], "read") == 0)
        {
            //read the contents of EEPROM starting at our test address
            for (patternIdx = 0; patternIdx < 4; patternIdx++)
            {
                testPatternRead[patternIdx] = HW_EEP_ReadByte(addr);
                addr++;
            }

            //now compare what we read to the expected pattern
           if ( memcmp(&testPatternRead, &testPatternWrite, 4) == 0 )
           {
               HW_TERM_Print("\nPass\n");
           }
           else
           {
               HW_TERM_PrintColor("\nFAIL\n", KRED);
           }
        }
        else if (strcmp(argv[SECOND_ARG_IDX], "write") == 0)
        {
            //write the test pattern to eeprom at the test address
            HW_EEP_WriteBlock(addr, (uint8_t*)&testPatternWrite, 4);

            HW_TERM_Print("\nWrite complete\n");
        }
        else
        {
            HW_TERM_Print("Invalid parameter format.");
        }
    }
    else
    {
        HW_TERM_Print("Invalid parameter format.");
    }
}

static void PrintPrompt(void)
{
   HW_TERM_Print(APP_CLI_PROMPT);
}

static void HandleTerm(int argc, char **argv)
{
    bool isEnabled = false;

    /** - Enable terminal prints */
    if ((argc == ONE_ARGUMENT) && (strcmp(argv[FIRST_ARG_IDX], "enable") == 0))
    {
        isEnabled = true;

        //if logging was previously disabled, enable it and print
        //the version
        if ( HW_TERM_IsLoggingEnabled() == false )
        {
            HW_TERM_DisableOrEnableLogging(isEnabled);

            //log version info to the user
            HW_TERM_PrintVersionInfo();
        }

        HW_TERM_Print("\r\nEnabled logging\r\n");
    }
    /** - Disable terminal prints */
    else if ((argc == ONE_ARGUMENT) && (strcmp(argv[FIRST_ARG_IDX], "disable") == 0))
    {
        HW_TERM_Print("\r\nDisabling logging\r\n");
        isEnabled = false;
        HW_TERM_DisableOrEnableLogging(isEnabled);
    }
    else
    {
        HW_TERM_Print("Invalid parameter format.");
    }
}

static void HandleRuntime(int argc, char **argv)
{
    uint8_t str[40];
    uint32_t value = 0;
    uint32_t runtime_s;
    uint64_t ticks;

    if(argc == ZERO_ARGUMENTS)
    {
        runtime_s = uC_TIME_GetRuntimeSeconds();
        ticks = uC_TIME_GetRuntimeTicks();

        sprintf((char *)str, "Runtime: Ticks: %llu\nSeconds: %lu\n", ticks, runtime_s);
        HW_TERM_Print(str);
    }
    else if(argc == TWO_ARGUMENTS)
    {
        if(0 == strcmp(argv[FIRST_ARG_IDX], "set"))
        {
            value = strtoul(argv[SECOND_ARG_IDX], NULL, 10);
            sprintf((char *)str, "Setting run time to %lu seconds.", value);
            HW_TERM_Print(str);
            uC_TIME_SetRuntime(value);
        }
        else
        {
            HW_TERM_Print("Invalid parameter.\n");
        }
    }
    else
    {
        HW_TERM_Print("Invalid parameter format.\n");
    }
}

static void HandleAlgorithm(int argc, char **argv)
{
    uint16_t value = 0u;
    uint16_t disp = 0u;
    uint8_t str[40];

    if ((argc == ONE_ARGUMENT) && (0 == strcmp(argv[FIRST_ARG_IDX], "watervolume")))
    {
        value = APP_ALGO_getHourlyWaterVolume();

        sprintf((char *)str, "Water Volume: %u\n", value);
        HW_TERM_Print(str);
    }
    else if ((argc == ONE_ARGUMENT) && (0 == strcmp(argv[FIRST_ARG_IDX], "strokecount")))
    {
        APP_ALGO_calculateHourlyStrokes();
        value = APP_ALGO_getHourlyStrokeCount();
        disp = APP_ALGO_getHourlyDisplacement();

        sprintf((char *)str, "Stroke Count: %u, Disp: %u\n", value, disp);
        HW_TERM_Print(str);

        value = APP_ALGO_getMagWindowsProcessed();

        sprintf((char *)str, "Num Windows: %u\n", value);
        HW_TERM_Print(str);
    }
    else if ((argc == ONE_ARGUMENT) && (0 == strcmp(argv[FIRST_ARG_IDX], "strokereset")))
    {
        APP_ALGO_resetHourlyStrokeCount();
        HW_TERM_Print("Stroke Count Reset\n");
    }
    else if ((argc == ONE_ARGUMENT) && (0 == strcmp(argv[FIRST_ARG_IDX], "rawdata")))
    {
        HW_TERM_ReportPadValues();
    }
    else
    {
        HW_TERM_Print("Invalid parameter format.\n");
    }
}
