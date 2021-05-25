/*
================================================================================================#=
Module:   External Watchdog Driver

Description:
    Kick logic for an external supervisor chip

Copyright 2021 Twisthink LLC
This code is licensed under Twisthink license.
See Twisthink-SoftwareLicenseAgreement-Auris.docx for details.
================================================================================================#=
*/

#include "stm32l4xx_hal.h"
#include "stm32l4xx_hal_rcc.h"
#include "logTypes.h"
#include "externalWatchdog.h"


#define WD_KICK_PIN         GPIO_PIN_2
#define WD_KICK_PORT        GPIOA

#define WD_LOW_TIM_COUNT    75
#define WD_HIGH_TIM_COUNT   76

static TIM_HandleTypeDef htim2;

static void xStartTimer(void);
static void xStopTimer(void);

void WD_initKick(void)
{
    //init gpio
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    //has an external pull down so we dont need to set that:
    GPIO_InitStruct.Pin = WD_KICK_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(WD_KICK_PORT, &GPIO_InitStruct);

    xStartTimer();
}

void WD_deinitKick(void)
{

    //init gpio
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    //reset the pin as an input
    GPIO_InitStruct.Pin = WD_KICK_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(WD_KICK_PORT, &GPIO_InitStruct);

    //turn off the timer
    xStopTimer();
}

static void xStopTimer(void)
{
    HAL_NVIC_DisableIRQ(TIM2_IRQn);

    //deinit timer
    HAL_TIM_Base_DeInit(&htim2);

    //stop Interrupts
    HAL_TIM_Base_Stop_IT(&htim2);
}

static void xStartTimer(void)
{
    RCC_ClkInitTypeDef    clkconfig;
    uint32_t              uwTimclock = 0;
    uint32_t              uwPrescalerValue = 0;
    uint32_t              pFLatency;

    //set up interrupts
    HAL_NVIC_SetPriority(TIM2_IRQn, 5 ,0);
    HAL_NVIC_EnableIRQ(TIM2_IRQn);

    //enable clock
    __HAL_RCC_TIM2_CLK_ENABLE();

    //get clock config
    HAL_RCC_GetClockConfig(&clkconfig, &pFLatency);


    uwTimclock = HAL_RCC_GetPCLK1Freq();

    //compute the prescaler value to have TIM2 counter clock equal to 1MHz
    uwPrescalerValue = (uint32_t) ((uwTimclock / 3000000) - 1);

    //100 Hz signal
    htim2.Instance = TIM2;
    htim2.Init.Period = (60000 - 1);
    htim2.Init.Prescaler = uwPrescalerValue;
    htim2.Init.ClockDivision = 0;
    htim2.Init.CounterMode = TIM_COUNTERMODE_UP;

    //init timer
    HAL_TIM_Base_Init(&htim2);

    //start Interrupts
    HAL_TIM_Base_Start_IT(&htim2);
}

//external Watchdog interrupt handler -
//this fires at a 10ms interval. Every 750 ms set the pin
//high for 1 interrupt (10ms) and then set back low
void TIM2_IRQHandler(void)
{
    static uint8_t count = 0;

    count++;

    HAL_TIM_IRQHandler(&htim2);

    if ( count == WD_LOW_TIM_COUNT )
    {
        HAL_GPIO_WritePin(WD_KICK_PORT, WD_KICK_PIN, GPIO_PIN_SET);
    }
    else if ( count == WD_HIGH_TIM_COUNT )
    {
        HAL_GPIO_WritePin(WD_KICK_PORT, WD_KICK_PIN, GPIO_PIN_RESET);
        count = 0;
    }
}
