/*
================================================================================================#=
Module:   Main

Description:
    Run Bootloader for STM32 - update FW if necessary and jump to the APP

Copyright 2021 Twisthink LLC
This code is licensed under Twisthink license.
See Twisthink-SoftwareLicenseAgreement-Auris.docx for details.
================================================================================================#=
*/

#include "string.h"
#include "stdint.h"
#include "spi.h"
#include "logger.h"
#include "logTypes.h"
#include "uart.h"
#include "memMapHandler.h"
#include "updateFw.h"
#include "stmFlash.h"
#include "bootloader.h"
#include "externalWatchdog.h"
#include "version-git-info.h"
#include "version.h"
#include "main.h"

static void xSystemClock_Config(void);
static void xJumpToApplication(void);
static void xEnterLpMode(void);

//function pointer definition
typedef void (*pFunction)(void);


int main(void)
{
    /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
    HAL_Init();

    /* Configure the system clock */
    xSystemClock_Config();

    /* start kicking watchdog in the event that the SSM is in the weeds
     * The application will detect if this is the case and handle appropriately */
    WD_initKick();

    //init logger/lp uart for logging
    LOG_initializeLogger();

    //init spi
    SPI_Init();

    //print a welcome message now that the logger has been initialized, to indicate that we have just started up
    elogNotice(ANSI_COLOR_YELLOW "***************************************************");
    elogNotice(ANSI_COLOR_MAGENTA " BOOTLOADER ");
    elogNotice(ANSI_COLOR_MAGENTA " BL Release Version: %d.%d.%d", BL_VERSION_MAJOR, BL_VERSION_MINOR, BL_VERSION_BUILD);
    elogNotice(ANSI_COLOR_MAGENTA " BL Build Version: %s %s %s", GIT_BRANCH_NAME, GIT_COMMIT_HASH, BUILD_TIMESTAMP_UTC);
    elogNotice(ANSI_COLOR_YELLOW "***************************************************" ANSI_COLOR_RESET);

    elogInfo("Starting BL");

    if ( BOOT_RunBootloader() == true )
    {
        //if we get here we need to just jump to the application since the bootloader
        //has either A.) Updated the AM FW & ready to run it

        //or B.) Verified that we do not need to update the AM FW, so we are also ready to run it

        //we will never return after this is called
        elogInfo("Jumping to App");
        xJumpToApplication();
    }
    else
    {
        //bootloader detected that both images are bad. Instead of entering LP mode, just try to jump to the application
        //in the event that there is a useable image in flash
        elogInfo("Jumping to App instead of LP mode");
        xJumpToApplication();
    }
    while (1)
    {
        //if we do end up in here, something went very wrong
    }
}


static void xJumpToApplication(void)
{
    //this is where the program actually starts  (app entry point)
    uint32_t  JumpAddress = *(__IO uint32_t*)(APP_LOCATION + 4);
    pFunction Jump = (pFunction)JumpAddress;

    //un-initialize the clock, HAL, disable interrupts
    __HAL_RCC_GPIOC_CLK_DISABLE();
    HAL_RCC_DeInit();

    //peripherals and periph clocks
    SPI_DeInit();
    UART_deinitDebugPeripherals();

    HAL_DeInit();

    //should i be doing this?
    __disable_irq();

    //stop sys tick
    SysTick->CTRL = 0;
    SysTick->LOAD = 0;
    SysTick->VAL  = 0;

    //remmap the vector table for the new App
    //without doing this, the app will vector to the bootloaders interrupt handlers
    SCB->VTOR = APP_LOCATION;

    //set stack pointer - this has to be done last otherwise variables in this function wont have the
    //right values
    __set_MSP(*(__IO uint32_t*)APP_LOCATION);

    //vector to the application
    Jump();
}


static void xEnterLpMode(void)
{
    /* STANDBY MODE INIT & WAKE ON PC13 */

    /* FROM example Proj: The Following Wakeup sequence is highly recommended prior to each Standby mode entry
       mainly when using more than one wakeup source this is to not miss any wakeup event.
        - Disable all used wakeup sources,
        - Clear all related wakeup flags,
        - Re-enable all used wakeup sources,
        - Enter the Standby mode.
     */

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


/**
  * @brief System Clock Configuration
  * @retval None
  */
static void xSystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
    RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

    /** Configure the main internal regulator output voltage
    */
    if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
    {
        Error_Handler();
    }

    /** Initializes the CPU, AHB and APB busses clocks
    */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
    RCC_OscInitStruct.MSIState = RCC_MSI_ON;
    RCC_OscInitStruct.MSICalibrationValue = 0;
    RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_11; // 48MHz
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;

    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        Error_Handler();
    }
    /** Initializes the CPU, AHB and APB busses clocks
    */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_MSI;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV16;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    /*
     * Use FLash latency 2 or greater based on the clock frequency of 48 MHz.
     * See page 122 in the STM32l4 reference manual for recommended values based on MSI frequency
     * https://www.st.com/resource/en/reference_manual/rm0432-stm32l4-series-advanced-armbased-32bit-mcus-stmicroelectronics.pdf
     */
    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
    {
        Error_Handler();
    }

    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_LPUART1;
    PeriphClkInit.Lpuart1ClockSelection = RCC_LPUART1CLKSOURCE_PCLK1;

    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
    {
        Error_Handler();
    }

    __HAL_RCC_GPIOA_CLK_ENABLE();
}

void Error_Handler(void)
{
    while (1)
    {
        elogError("Bad clock config.");
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
