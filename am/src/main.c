/**************************************************************************************************
* \file     main.c
* \brief    Start up RTOS tasks and perform any initialization of the clocks, peripherals, modules.
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

/* Includes ------------------------------------------------------------------*/
#include <appVersion.h>
#include <CLI.h>
#include <flashHandler.h>
#include <i2c.h>
#include <logger.h>
#include <main.h>
#include <MT29F1.h>
#include <uart.h>
#include "connectivity.h"
#include "taskMonitor.h"
#include "spi.h"
#include "ssm.h"
#include "ATECC608A.h"
#include "logTypes.h"
#include "pwrMgr.h"
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"
#include "string.h"
#include "version-git-info.h"
#include "gpsManager.h"
#include "PE42424A_RF.h"
#include "nwStackFunctionality.h"
#include "sara_u201.h"
#include "portable.h"
#include "iot_system_init.h"
#include "eventManager.h"
#include "memMapHandler.h"
#include "memoryMap.h"
#include "updateSsmFw.h"
#include "externalWatchdog.h"
#include "aws_dev_mode_key_provisioning.h"


//todo move all of this rtos init to another task
#define CLI_TASK_PRIORITY                       ( configMAX_PRIORITIES - 1 )
#define CLI_TASK_STACK_SIZE                     ( configMINIMAL_STACK_SIZE * 12 )

#define WATCHDOG_TASK_PRIORITY                  ( configMAX_PRIORITIES - 1 )
#define WATCHDOG_TASK_STACK_SIZE                ( configMINIMAL_STACK_SIZE * 5 )

#define AT_TASK_PRIORITY                        ( configMAX_PRIORITIES - 1 )
#define AT_TASK_STACK_SIZE                      ( configMINIMAL_STACK_SIZE * 12 )

#define STARTUP_TASK_PRIORITY                   ( configMAX_PRIORITIES - 1 )
#define STARTUP_TASK_STACK_SIZE                 ( configMINIMAL_STACK_SIZE * 18 )

#define SSM_SPI_TASK_PRIORITY                   ( configMAX_PRIORITIES - 1 )
#define SSM_SPI_TASK_STACK_SIZE                 ( configMINIMAL_STACK_SIZE * 5 )

#define EVENT_MANAGER_TASK_PRIORITY             ( configMAX_PRIORITIES - 1 )
#define EVENT_MANAGER_TASK_STACK_SIZE           ( configMINIMAL_STACK_SIZE * 28 )

#define MS_PER_MINUTE                           (60*1000)
#define STARTUP_TASK_DELAY                      (10*1000)
#define ONE_HUNDRED_PERCENT                     100
#define SSM_UPGRADE_DELAY_TIME                  4000

/* Private includes ----------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

// Bootloader State Structure
typedef enum
{
    NOMINAL,
    UPGRADE,
    FALLBACK,
    OFF_NOMINAL,
    PANIC,
    UNKNOWN,
    MANUFACTURING,
}reasonLoaded_t;

typedef struct
{
    uint32_t startCount;
    reasonLoaded_t cacheReasonLastLoaded;
    imageSlotTypes_t cacheLastLoaded;
    uint32_t keyToCheckSwResetOrPowerCycle;
}bootloaderCache_t;

//we will use this to determine if we just were upgraded, etc - Read ONLY
//Set the image register appropriately based on these contents during initialization
static bootloaderCache_t *blState = (bootloaderCache_t *) 0x20000000;

static uint8_t ucHeap1[ configTOTAL_HEAP_SIZE*2 ];   //60 *1024 * 2
static uint8_t ucHeap2[ 120 * 1024 ] __attribute__( ( section( ".freertos_heap2" ) ) ); //98
static bool printStats = false;
static uint32_t runTimeCounterMs = 0u;

TaskHandle_t xStartupHandle;
TaskHandle_t xCLIHandle;
TaskHandle_t xAtHandle;
TaskHandle_t xSSM_SPIHandle;
TaskHandle_t xTmHandle;
TaskHandle_t xEventHandle;

RNG_HandleTypeDef xHrng;

HeapRegion_t xHeapRegions[] =
{
    { ( unsigned char * ) ucHeap2, sizeof( ucHeap2 ) },
    { ( unsigned char * ) ucHeap1, sizeof( ucHeap1 ) },
    { NULL,                                        0 }
};

//RNG_HandleTypeDef xHrng;

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void vApplicationDaemonTaskStartupHook( void );

void startupTask();
static void xGpio_Init(void);
static void xInitAndStartRtos(void);
static void runTaskStats(void);
static void xStackCommandHandlerFunction(int argc, char **argv);
static void printWatermarkWithColor(const char * const pcName, uint32_t usedWords, uint32_t stackSize, uint32_t percent);
static uint32_t xGetRuntimeMs(void);
static void xUpdateRuntimeMs(uint32_t ms);
static void xCacheAndAppInit(void);
static void xVerifySsmAppIsRunning(void);
static void xProgramSsmWithFallBackImage(void);


/* Psuedo random number generator.  Just used by demos so does not need to be
 * secure.  Do not use the standard C library rand() function as it can cause
 * unexpected behaviour, such as calls to malloc(). */
int iMainRand32( void )
{
    static UBaseType_t uxlNextRand; /*_RB_ Not seeded. */
    const uint32_t ulMultiplier = 0x015a4e35UL, ulIncrement = 1UL;

    /* Utility function to generate a pseudo random number. */

    uxlNextRand = ( ulMultiplier * uxlNextRand ) + ulIncrement;

    return( ( int ) ( uxlNextRand >> 16UL ) & 0x7fffUL );
}

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
    /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
    HAL_Init();

    /* Configure the system clock */
    SystemClock_Config();

    /* Initialize all configured peripherals */
    xGpio_Init();

    //start kicking the watchdog in the event that the SSM is in the weeds - we will
    //detect this shortly:
    WD_initKick();

    /* RNG init function. */
    xHrng.Instance = RNG;

    if( HAL_RNG_Init( &xHrng ) != HAL_OK )
    {
        Error_Handler();
    }

    /* Initialize tasks and kick off the RTOS */
    xInitAndStartRtos();

    /* Infinite loop */
    while (1)
    {
        /* should never enter this loop if the RTOS was started properly */
    }
}

static void xInitAndStartRtos(void)
{
    vPortDefineHeapRegions(xHeapRegions);

    xTaskCreate(startupTask, "START", STARTUP_TASK_STACK_SIZE, NULL, STARTUP_TASK_PRIORITY, &xStartupHandle);

    /* start up the RTOS */
    vTaskStartScheduler();
}


/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
    RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};


    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE | RCC_OSCILLATORTYPE_MSI;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.MSIState = RCC_MSI_ON;
    RCC_OscInitStruct.MSICalibrationValue = 0;
    RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_11;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_MSI;
    RCC_OscInitStruct.PLL.PLLM = 6;
    RCC_OscInitStruct.PLL.PLLN = 20;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
    RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
    RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_OFF;

    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        Error_Handler();
    }

    /* Initializes the CPU, AHB and APB busses clocks */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                          |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_MSI;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV16;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
    {
        Error_Handler();
    }

    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1|RCC_PERIPHCLK_LPUART1|RCC_PERIPHCLK_I2C2|RCC_PERIPHCLK_UART4 |RCC_PERIPHCLK_I2C2 | RCC_PERIPHCLK_UART5 | RCC_PERIPHCLK_UART4| RCC_PERIPHCLK_RNG;
    PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
    PeriphClkInit.Uart5ClockSelection = RCC_UART5CLKSOURCE_SYSCLK;

    PeriphClkInit.Uart4ClockSelection = RCC_UART4CLKSOURCE_PCLK1;
    PeriphClkInit.Lpuart1ClockSelection = RCC_LPUART1CLKSOURCE_PCLK1;
    PeriphClkInit.I2c2ClockSelection = RCC_I2C2CLKSOURCE_PCLK1;
    PeriphClkInit.Usart3ClockSelection = RCC_USART3CLKSOURCE_SYSCLK;
    PeriphClkInit.RngClockSelection = RCC_RNGCLKSOURCE_MSI;

    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
    {
        Error_Handler();
    }

    __HAL_RCC_PWR_CLK_ENABLE();

    if( HAL_PWREx_ControlVoltageScaling( PWR_REGULATOR_VOLTAGE_SCALE1 ) != HAL_OK )
    {
        Error_Handler();
    }

    //rng requirement
    HAL_RCCEx_EnableMSIPLLMode();
}

void startupTask()
{
    static uint32_t taskCounterMs = 0u;

    UART_initPeripherals();
    SPI_Init();
    LOG_initializeLogger();

    //print a welcome message now that the logger has been initialized, to indicate that we have just started up
    elogNotice(ANSI_COLOR_GREEN "***************************************************");
    elogNotice(ANSI_COLOR_CYAN " AM APPLICATION");
    elogNotice(ANSI_COLOR_CYAN " AM Release Version: %d.%d.%d", VERSION_MAJOR, VERSION_MINOR, VERSION_BUILD);
    elogNotice(ANSI_COLOR_CYAN " AM Build Version: %s %s %s", GIT_BRANCH_NAME, GIT_COMMIT_HASH, BUILD_TIMESTAMP_UTC);
    elogNotice(ANSI_COLOR_GREEN "***************************************************" ANSI_COLOR_RESET);

    PWR_init();
    I2C_Init();

    FLASH_init();
    MEM_init();
    EVT_initializeEventQueue();
    SSM_Init();

    //after mem is initalzied, look at the image registry/bootloader cache
    xCacheAndAppInit();
    xVerifySsmAppIsRunning();

    //init crypto cli function
    ATECC_init();

    //init crypto, pkcs11/security library
    vDevModeKeyProvisioning();

    SYSTEM_Init();

    /* register a command handler cb function */
    CLI_Command_Handler_s stackCmdHandler;
    stackCmdHandler.ptrFunction = &xStackCommandHandlerFunction;
    stackCmdHandler.cmdString   = "stats";
    stackCmdHandler.usageString = "\n\r\tprint \n\r\ton \n\r\toff \n\r\trun \n\r\tmeta";
    CLI_registerThisCommandHandler(&stackCmdHandler);

    GPS_initModule();
    RF_initRfSwitchesAndCli();
    SARA_initHardware();
    CONN_initCliCommands();

    /* Create tasks */
    xTaskCreate(EVT_eventManagerTask, "STATE", EVENT_MANAGER_TASK_STACK_SIZE, NULL, EVENT_MANAGER_TASK_PRIORITY, &xEventHandle);
    xTaskCreate(SSM_SPI_Task, "SSM", SSM_SPI_TASK_STACK_SIZE, NULL, SSM_SPI_TASK_PRIORITY, &xSSM_SPIHandle);
    xTaskCreate(ATcommandModeParsing_Task, "AT", AT_TASK_STACK_SIZE, NULL, AT_TASK_PRIORITY, &xAtHandle );
    xTaskCreate(TM_task, "WD", WATCHDOG_TASK_STACK_SIZE, NULL, WATCHDOG_TASK_PRIORITY, &xTmHandle);
    xTaskCreate(CLI_commandLineHandler_task, "CLI", CLI_TASK_STACK_SIZE, NULL, CLI_TASK_PRIORITY, &xCLIHandle);

    /* use this task to periodically log task stats to the terminal */
    while (1)
    {
        vTaskDelay(STARTUP_TASK_DELAY);

        //check in with watchdog task monitor
        TM_mainTaskCheckIn();

        taskCounterMs += STARTUP_TASK_DELAY;
        xUpdateRuntimeMs(STARTUP_TASK_DELAY);

        if ( printStats == true && (taskCounterMs >= MS_PER_MINUTE) )
        {
            taskCounterMs = 0;
            runTaskStats();
        }
    }
}

static void xVerifySsmAppIsRunning(void)
{
    uint8_t resetsSinceLpMode = MEM_getResetsSinceLastLpMode();
    asp_status_payload_t ssmStat = {};

    //first check the reset counter to see if we are resetting continuously
    //just because the ssm passes the comm test, there could still be a bug in the FW
    //causing the watchdog to reset the system

    if ( resetsSinceLpMode >= MAX_RESETS_SINCE_LP_MODE )
    {
        //reload FW on the SSM
        elogError("CONTINUOUSLY RESETTING - RELOAD FW");
        xProgramSsmWithFallBackImage();
    }
    else
    {
        if ( SSM_communicationCheck() == true )
        {
            elogInfo("SSM APP RUNNING");

            //stop kicking the watchdog - the SSM is up and running
            WD_deinitKick();
        }
        else
        {
            elogError("SSM APP is locked up - resetting ssm");
            //reset the SSM
            SSM_hardwareReset();

            vTaskDelay(2000);

            //try 1 more time
            if ( SSM_communicationCheck() == true )
            {
                elogInfo("SSM APP RUNNING NOW");

                //stop kicking the watchdog - the SSM is up and running
                WD_deinitKick();
            }
            else
            {
                //if fails, load new FW onto the SSM
                elogError("SSM APP still not working - loading new FW");

                //mark image as bad and reload FW
                xProgramSsmWithFallBackImage();
            }
        }

        //we know there will be 1 reset since lp mode, but if there has been more than 1 then
        //we know we just came out of an unexpected reset
        if ( MEM_getResetsSinceLastLpMode() > 1 )
        {
            //get a status message from the SSM
            ssmStat = SSM_getStatus();

            MEM_incrementUnexpectedResetCount(ssmStat.timestamp);
        }
    }
}

static void xProgramSsmWithFallBackImage(void)
{
    imageSlotTypes_t currentSlot;
    bool ssmImageProgrammed = false;

    //make sure we have a back up image - if not, what do we do??
    currentSlot = MEM_getLoadedImage();

    if ( currentSlot == UNKNOWN_SLOT )
    {
        elogError("This slot isnt stored in external flash...");

        return;
    }

    //mark the image as bad and program w/ fallback image
    if ( currentSlot == A && (MEM_getImageOpState(B) == PARTIAL || MEM_getImageOpState(B) == FULL) )
    {
       MEM_setImageAoperationalState(OP_FAILED);
       MEM_setPrimaryImage(B);

       //rollback the ssm image
       elogError("Rolling back ssm to image in slot B");

       ssmImageProgrammed = SSM_FW_programBslWithExternalFlashImage(APP_MEM_ADR_FW_APPLICATION_SSM_B_START);

       //try again if doesnt program
       if ( ssmImageProgrammed != true )
       {
           ssmImageProgrammed = SSM_FW_programBslWithExternalFlashImage(APP_MEM_ADR_FW_APPLICATION_SSM_B_START);
       }
    }
    else if (currentSlot == B && (MEM_getImageOpState(A) == PARTIAL || MEM_getImageOpState(A) == FULL))
    {
        MEM_setImageBoperationalState(OP_FAILED);
        MEM_setPrimaryImage(A);

        //rollback the ssm image
        elogError("Rolling back ssm to image in slot A");

        ssmImageProgrammed = SSM_FW_programBslWithExternalFlashImage(APP_MEM_ADR_FW_APPLICATION_SSM_A_START);

        //try again if doesnt program
        if ( ssmImageProgrammed != true )
        {
            ssmImageProgrammed = SSM_FW_programBslWithExternalFlashImage(APP_MEM_ADR_FW_APPLICATION_SSM_B_START);
        }
    }
    else
    {
        //TODO:
        elogError("We dont have a fallback image - try to alert the cloud");
    }

    if ( ssmImageProgrammed == true )
    {
        elogInfo("reprogrammed SSM");

        //update reset counter
        MEM_setResetsSinceLastLpMode(0);

        //now reset ourselves so the BL downgrades our image
        NVIC_SystemReset();
    }
    else
    {
        //TODO what should we do here
        elogFatal("SSM without working image");
    }
}

static void xCacheAndAppInit(void)
{
    //update the image registry based on the reason loaded and last loaded slot
    imageSlotTypes_t currentSlot = MEM_getSlotWithThisVersion(VERSION_MAJOR, VERSION_MINOR, VERSION_BUILD);
    imageOperationalState_t currentOpState;
    uint32_t primaryImageAddr = 0;
    bool programSsm = false;

    //lets see if this matches what just printed from the BL:
    elogInfo("%lu , %d, %d", blState->startCount, blState->cacheReasonLastLoaded, blState->cacheLastLoaded);

    //bail out of we dont know the current slot
    if ( currentSlot == UNKNOWN_SLOT )
    {
        elogNotice("we should ota a copy of this current image into the SPI flash so we have it in the image reg.");
        return;
    }

    if ( MEM_getLoadedImage() != currentSlot )
    {
        //update the loaded image with this slot
        MEM_setLoadedImage(currentSlot);
    }

    elogInfo("Loaded image %d", currentSlot);


    if ( MEM_getPrimaryImage() == currentSlot )
    {
        //check out the operational state of this current slot
        currentOpState = MEM_getImageOpState(currentSlot);

        if ( currentOpState == PARTIAL )
        {
            //we will update this to full hopefully on the next cloud connect.
            elogInfo("Partial");
        }
        else if ( currentOpState == FULL )
        {
            //cool, do nothing
            elogInfo("Full");
        }
        else if ( currentOpState == OP_UNKNOWN || currentOpState == OP_FAILED )
        {
            elogInfo("OP unknown or failed - update to partial");
            MEM_setImageOpState(PARTIAL, currentSlot);

            if ( blState->cacheReasonLastLoaded == UPGRADE )
            {
                elogInfo("Upgrade - we can go back to sleep now");
                PWR_enterStandbyMode();
            }
            else if (blState->cacheReasonLastLoaded == MANUFACTURING )
            {
                elogInfo("Lets update the SSM for the first time");

                primaryImageAddr = APP_MEM_ADR_FW_APPLICATION_SSM_A_START;

                //start kicking for the ssm
                WD_initKick();

                programSsm = SSM_FW_programBslWithExternalFlashImage(primaryImageAddr);

                if ( programSsm != true )
                {
                    elogError("SSM BSL DID NOT ACCEPT THE IMAGE. Try one more time");

                    //try one more time
                    programSsm = SSM_FW_programBslWithExternalFlashImage(primaryImageAddr);

                    //if it worked this time
                    if ( programSsm == true )
                    {
                       elogInfo("Programmed SSM with image");
                    }
                    else
                    {
                        elogError("SSM WITHOUT A WORKING IMAGE");
                    }
                }
                else
                {
                    elogInfo("Programmed SSM with image. Good to go");
                }

                //we will not go to sleep after this update since we are in manufacturing
            }
        }
        else
        {
            elogError("invalid op state");
        }
    }
    else
    {
        //primary image and the current image are not the same

        if ( blState->cacheReasonLastLoaded == FALLBACK )
        {
            //set the OTHER slot as 'FAILED' so that we dont try to boot from it again
            elogInfo ("Fallback - set the OTHER slot as 'FAILED' so that we dont try to boot from it again");
            MEM_setImageOpState(OP_FAILED, MEM_getAlternateSlot(currentSlot) );

            //update the new primary image to this working slot
            MEM_setPrimaryImage(currentSlot);


            //rollback the SSM to the current slot:

            if ( currentSlot == A )
            {
                primaryImageAddr = APP_MEM_ADR_FW_APPLICATION_SSM_A_START;
            }
            else
            {
                primaryImageAddr = APP_MEM_ADR_FW_APPLICATION_SSM_B_START;
            }

            //start kicking for the ssm
            WD_initKick();

            //give the ssm a heads up
            SSM_enableBootPin();

            vTaskDelay(SSM_UPGRADE_DELAY_TIME);

            //now reset the boot pin & downgrade the ssm fw
            SSM_disableBootPin();

            programSsm = SSM_FW_programBslWithExternalFlashImage(primaryImageAddr);

            if ( programSsm != true )
            {
                elogError("SSM BSL DID NOT ACCEPT THE IMAGE. Try one more time");

                //try one more time
                programSsm = SSM_FW_programBslWithExternalFlashImage(primaryImageAddr);

                //if it worked this time
                if ( programSsm == true )
                {
                   elogInfo("Programmed SSM with fallback image");
                }
                else
                {
                    elogError("SSM WITHOUT A WORKING IMAGE");
                }
            }
            else
            {
                elogInfo("Programmed SSM with fallback image. Good to go");
            }
        }
    }
}

static void xStackCommandHandlerFunction(int argc, char **argv)
{
    /* process the user input */
    if ( (argc == ONE_ARGUMENT) &&  (0 == strcmp(argv[FIRST_ARG_IDX], "print")) )
    {
        runTaskStats();
    }
    else if ( (argc == ONE_ARGUMENT) &&  (0 == strcmp(argv[FIRST_ARG_IDX], "on")) )
    {
        printStats = true;
    }
    else if ( (argc == ONE_ARGUMENT) &&  (0 == strcmp(argv[FIRST_ARG_IDX], "off")) )
    {
        printStats = false;
    }
    else if ( (argc == ONE_ARGUMENT) &&  (0 == strcmp(argv[FIRST_ARG_IDX], "run")) )
    {
        elogInfo("Runtime since pc: %d ms", xGetRuntimeMs());
    }
    else if ( (argc == ONE_ARGUMENT) &&  (0 == strcmp(argv[FIRST_ARG_IDX], "meta")) )
    {
        elogNotice(ANSI_COLOR_CYAN " Release Version: %d.%d.%d", VERSION_MAJOR, VERSION_MINOR, VERSION_BUILD);
        elogNotice(ANSI_COLOR_CYAN " Build Version: %s %s %s", GIT_BRANCH_NAME, GIT_COMMIT_HASH, BUILD_TIMESTAMP_UTC);
        elogNotice(ANSI_COLOR_CYAN " HW Version: %d", HW_VERSION);
    }
    else
    {
        elogInfo("Invaid args");
    }
}

/*
 * Get and print out stats for each task. Call this function at a regular interval to monitor
 * for stack overflows.
 *
 * If additional tasks are created, add the stats here
 */
static void runTaskStats(void)
{
    uint32_t cliWatermark = uxTaskGetStackHighWaterMark(xCLIHandle);
    uint32_t ssmWatermark = uxTaskGetStackHighWaterMark(xSSM_SPIHandle);
    uint32_t startWatermark = uxTaskGetStackHighWaterMark(xStartupHandle);
    uint32_t atWatermark = uxTaskGetStackHighWaterMark(xAtHandle);
    uint32_t tmWatermark = uxTaskGetStackHighWaterMark(xTmHandle);
    uint32_t eventWatermark = uxTaskGetStackHighWaterMark(xEventHandle);

    uint32_t cliPercentUsed = ONE_HUNDRED_PERCENT - ((CLI_TASK_STACK_SIZE - cliWatermark) * 100)/(CLI_TASK_STACK_SIZE);
    uint32_t ssmPercentUsed = ONE_HUNDRED_PERCENT -((SSM_SPI_TASK_STACK_SIZE - ssmWatermark) * 100)/(SSM_SPI_TASK_STACK_SIZE);
    uint32_t startPercentUsed = ONE_HUNDRED_PERCENT -((STARTUP_TASK_STACK_SIZE - startWatermark) * 100)/(STARTUP_TASK_STACK_SIZE);
    uint32_t atPercentUsed =  ONE_HUNDRED_PERCENT -((AT_TASK_STACK_SIZE - atWatermark) * 100)/(AT_TASK_STACK_SIZE);
    uint32_t tmPercentUsed = ONE_HUNDRED_PERCENT - ((WATCHDOG_TASK_STACK_SIZE - tmWatermark) * 100)/(WATCHDOG_TASK_STACK_SIZE);

    uint32_t eventPercentUsed = ONE_HUNDRED_PERCENT-((EVENT_MANAGER_TASK_STACK_SIZE - eventWatermark) * 100)/(EVENT_MANAGER_TASK_STACK_SIZE);

    elogInfo(ANSI_COLOR_CYAN"RUNTIME since power cycle %d ms (%d minutes)", xGetRuntimeMs(),(xGetRuntimeMs()/1000/60));
    printWatermarkWithColor("CLI", cliWatermark, CLI_TASK_STACK_SIZE, cliPercentUsed);
    printWatermarkWithColor("SSM", ssmWatermark, SSM_SPI_TASK_STACK_SIZE, ssmPercentUsed);
    printWatermarkWithColor("MAIN", startWatermark, STARTUP_TASK_STACK_SIZE, startPercentUsed);
    printWatermarkWithColor("AT", atWatermark, AT_TASK_STACK_SIZE, atPercentUsed);
    printWatermarkWithColor("TASK_MANAGER", tmWatermark, WATCHDOG_TASK_STACK_SIZE, tmPercentUsed);
    printWatermarkWithColor("STATE", eventWatermark, EVENT_MANAGER_TASK_STACK_SIZE, eventPercentUsed);
}


/* Add the appropriate color to a stack watermark message and print it */
static void printWatermarkWithColor(const char * const pcName, uint32_t usedWords, uint32_t stackSize, uint32_t percent)
{
    /* check the stack percentage remaining and print out the stats in the appropriate color */
    if ( percent < 80 )
    {
        elogInfo(ANSI_COLOR_GREEN"%s task: \r\n \t\t\t\t\t**** %d/%d words used **** %d%% Stack used ***", pcName, usedWords, stackSize, percent);
    }
    else if ( percent < 90 )
    {
        elogInfo(ANSI_COLOR_YELLOW"%s task: \r\n \t\t\t\t\t**** %d/%d words used **** %d%% Stack used ***", pcName, usedWords, stackSize, percent);
    }
    else
    {
        /* close to the watermark for the task */
        elogInfo(ANSI_COLOR_RED"%s task: \r\n \t\t\t\t\t**** %d/%d words used **** %d%% Stack used ***", pcName, usedWords, stackSize, percent);
    }
}


static uint32_t xGetRuntimeMs(void)
{
    return runTimeCounterMs;
}

static void xUpdateRuntimeMs(uint32_t ms)
{
    runTimeCounterMs += ms;
}

void vApplicationDaemonTaskStartupHook( void )
{

}

/**
  * @brief  Pre Sleep Processing
  * @param  ulExpectedIdleTime: Expected time in idle state
  * @retval None
  */
void vMainPreStopProcessing()
{
  /* Called by the kernel before it places the MCU into a sleep mode because
  configPRE_SLEEP_PROCESSING() is #defined to PreSleepProcessing().

  NOTE:  Additional actions can be taken here to get the power consumption
  even lower.  For example, peripherals can be turned off here, and then back
  on again in the post sleep processing function.  For maximum power saving
  ensure all unused pins are in their lowest power state. */
}


/* configUSE_STATIC_ALLOCATION is set to 1, so the application must provide an
 * implementation of vApplicationGetIdleTaskMemory() to provide the memory that is
 * used by the Idle task. */
void vApplicationGetIdleTaskMemory( StaticTask_t ** ppxIdleTaskTCBBuffer,
                                    StackType_t ** ppxIdleTaskStackBuffer,
                                    uint32_t * pulIdleTaskStackSize )
{
/* If the buffers to be provided to the Idle task are declared inside this
 * function then they must be declared static - otherwise they will be allocated on
 * the stack and so not exists after this function exits. */
    static StaticTask_t xIdleTaskTCB;
    static StackType_t uxIdleTaskStack[ configMINIMAL_STACK_SIZE ];

    /* Pass out a pointer to the StaticTask_t structure in which the Idle
     * task's state will be stored. */
    *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;

    /* Pass out the array that will be used as the Idle task's stack. */
    *ppxIdleTaskStackBuffer = uxIdleTaskStack;

    /* Pass out the size of the array pointed to by *ppxIdleTaskStackBuffer.
     * Note that, as the array is necessarily of type StackType_t,
     * configMINIMAL_STACK_SIZE is specified in words, not bytes. */
    *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}
/*-----------------------------------------------------------*/

/* configUSE_STATIC_ALLOCATION is set to 1, so the application must provide an
 * implementation of vApplicationGetTimerTaskMemory() to provide the memory that is
 * used by the RTOS daemon/time task. */
void vApplicationGetTimerTaskMemory( StaticTask_t ** ppxTimerTaskTCBBuffer,
                                     StackType_t ** ppxTimerTaskStackBuffer,
                                     uint32_t * pulTimerTaskStackSize )
{
/* If the buffers to be provided to the Timer task are declared inside this
 * function then they must be declared static - otherwise they will be allocated on
 * the stack and so not exists after this function exits. */
    static StaticTask_t xTimerTaskTCB;
    static StackType_t uxTimerTaskStack[ configTIMER_TASK_STACK_DEPTH ];

    /* Pass out a pointer to the StaticTask_t structure in which the Idle
     * task's state will be stored. */
    *ppxTimerTaskTCBBuffer = &xTimerTaskTCB;

    /* Pass out the array that will be used as the Timer task's stack. */
    *ppxTimerTaskStackBuffer = uxTimerTaskStack;

    /* Pass out the size of the array pointed to by *ppxTimerTaskStackBuffer.
     * Note that, as the array is necessarily of type StackType_t,
     * configMINIMAL_STACK_SIZE is specified in words, not bytes. */
    *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
}

/*-----------------------------------------------------------*/

/**
 * @brief Warn user if pvPortMalloc fails.
 *
 * Called if a call to pvPortMalloc() fails because there is insufficient
 * free memory available in the FreeRTOS heap.  pvPortMalloc() is called
 * internally by FreeRTOS API functions that create tasks, queues, software
 * timers, and semaphores.  The size of the FreeRTOS heap is set by the
 * configTOTAL_HEAP_SIZE configuration constant in FreeRTOSConfig.h.
 *
 */
void vApplicationMallocFailedHook()
{
    taskDISABLE_INTERRUPTS();
    for( ;; );
}
/*-----------------------------------------------------------*/

/**
 * @brief Loop forever if stack overflow is detected.
 *
 * If configCHECK_FOR_STACK_OVERFLOW is set to 1,
 * this hook provides a location for applications to
 * define a response to a stack overflow.
 *
 * Use this hook to help identify that a stack overflow
 * has occurred.
 *
 */
void vApplicationStackOverflowHook( TaskHandle_t xTask,
                                    char * pcTaskName )
{
    portDISABLE_INTERRUPTS();

    /* Loop forever */
    for( ; ; );
}
/*-----------------------------------------------------------*/

void vApplicationIdleHook( void )
{
    while(1)
    {

    }
}
/*-----------------------------------------------------------*/

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void xGpio_Init(void)
{
    /* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOE_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
    while(1)
    {
        elogError("Error Handler.");
    }
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(char *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
