/**************************************************************************************************
* \file     uC_TIME.c
* \brief    MSP timer peripheral driver
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
#include "uC_TIME.h"
#include <msp430.h>                             // Generic MSP430 Device Include
#include "driverlib.h"                          // MSPWare Driver Library
#include <stdio.h>
#include <string.h>
#include "HW_TERM.h"

#define WATCHDOG_KICK_HIGH_RATE_MS             750

static volatile uint64_t TimerA_Ch0_Ticks = 0;
static volatile uint32_t SecondsCounter = 0;
static volatile int32_t SecondsSinceLastHr = 0;

static uint16_t wdKickTimer = 0;
static bool wdOn = true;
static bool firstKick = true;
static uint8_t firstHigh = 0;

void uC_TIME_Init(void);
uint64_t uC_TIME_GetRuntimeTicks(void);
uint32_t uC_TIME_GetRuntimeSeconds(void);
void uC_TIME_SetRuntime(uint32_t seconds);

//flag to disable kicking the external watchdog
void HW_WatchdogStopKick(bool stop)
{
    wdOn = stop;
}

void uC_TIME_Init(void)
{
    //Stop Watchdog Timer
    WDT_A_hold(WDT_A_BASE);

    //Start timer in continuous mode sourced by ACLK (external oscillator)
    Timer_A_initContinuousModeParam initContParam = {0};
    initContParam.clockSource = TIMER_A_CLOCKSOURCE_ACLK; // Running at 32.768kHz
    initContParam.clockSourceDivider = TIMER_A_CLOCKSOURCE_DIVIDER_1;
    initContParam.timerInterruptEnable_TAIE = TIMER_A_TAIE_INTERRUPT_DISABLE;
    initContParam.timerClear = TIMER_A_DO_CLEAR;
    initContParam.startTimer = false;
    Timer_A_initContinuousMode(TIMER_A1_BASE, &initContParam);

    //Initialize compare mode for channel 0
    Timer_A_clearCaptureCompareInterrupt(TIMER_A1_BASE,
        TIMER_A_CAPTURECOMPARE_REGISTER_0);

    //Initialize compare mode for channel 1
    Timer_A_clearCaptureCompareInterrupt(TIMER_A1_BASE,
        TIMER_A_CAPTURECOMPARE_REGISTER_1);

    //set up 10ms timer with interrupts
    Timer_A_initCompareModeParam initCompParam0 = {0};
    initCompParam0.compareRegister = TIMER_A_CAPTURECOMPARE_REGISTER_0;
    initCompParam0.compareInterruptEnable = TIMER_A_CAPTURECOMPARE_INTERRUPT_ENABLE;
    initCompParam0.compareOutputMode = TIMER_A_OUTPUTMODE_OUTBITVALUE;
    initCompParam0.compareValue = UC_TIME_10_MS_IN_CYCLES;
    Timer_A_initCompareMode(TIMER_A1_BASE, &initCompParam0);

    //set up 1 second timer with interrupts
    Timer_A_initCompareModeParam initCompParam1 = {0};
    initCompParam1.compareRegister = TIMER_A_CAPTURECOMPARE_REGISTER_1;
    initCompParam1.compareInterruptEnable = TIMER_A_CAPTURECOMPARE_INTERRUPT_ENABLE;
    initCompParam1.compareOutputMode = TIMER_A_OUTPUTMODE_OUTBITVALUE;
    initCompParam1.compareValue = UC_TIME_1000_MS_IN_CYCLES;
    Timer_A_initCompareMode(TIMER_A1_BASE, &initCompParam1);

    //start all timer channels
    Timer_A_startCounter( TIMER_A1_BASE,
            TIMER_A_CONTINUOUS_MODE );

}

// Return the current number of ticks.  Each tick is UC_TIMER_TICK_TIME_MS milliseconds.
uint64_t uC_TIME_GetRuntimeTicks(void)
{
    uint64_t ticks;

    __disable_interrupt();
    ticks =  TimerA_Ch0_Ticks;
    __enable_interrupt();

    return ticks;
}

// Return runtime in seconds.
uint32_t uC_TIME_GetRuntimeSeconds(void)
{
    uint32_t secs;

    __disable_interrupt();
    secs = SecondsCounter;
    __enable_interrupt();

    return secs;
}

// Return current hourly timing adjustment counter value
int32_t uC_TIME_GetHourlyTimeAdjustSeconds(void)
{
    int32_t secs;

    __disable_interrupt();
    secs = SecondsSinceLastHr;
    __enable_interrupt();

    return secs;
}


// Set the next hourly time adjustment starting value, can be a negative value
void uC_TIME_SetHourlyTimeAdjustSeconds(int32_t secs)
{
    __disable_interrupt();
    SecondsSinceLastHr = secs;
    __enable_interrupt();
}

// Set the run timer to the provided value.
void uC_TIME_SetRuntime(uint32_t seconds)
{
    __disable_interrupt();
    TimerA_Ch0_Ticks = (seconds * UC_TIME_TICKS_PER_S) ;
    SecondsCounter = seconds;
    __enable_interrupt();
}

//******************************************************************************
//
//This is the TIMER1_A0 interrupt vector service routine.
//
//******************************************************************************
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=TIMER1_A0_VECTOR
__interrupt
#elif defined(__GNUC__)
__attribute__((interrupt(TIMER1_A0_VECTOR)))
#endif
void TIMER1_A0_ISR (void)
{
    uint16_t compVal = Timer_A_getCaptureCompareCount(TIMER_A1_BASE,
            TIMER_A_CAPTURECOMPARE_REGISTER_0)
            + UC_TIME_10_MS_IN_CYCLES;

    //Add Offset to CCR0
    Timer_A_setCompareValue(TIMER_A1_BASE,
        TIMER_A_CAPTURECOMPARE_REGISTER_0,
        compVal);

    //update sys tick counter
    TimerA_Ch0_Ticks++;

    //increment the watchdog kick timer
    wdKickTimer++;

    //kick every 750 ms (75 interrupts)
    if ( wdKickTimer >= (WATCHDOG_KICK_HIGH_RATE_MS/UC_TIMER_TICK_TIME_MS) && (true == wdOn))
    {
        //if exactly 750 ms, set high
        if(wdKickTimer == (WATCHDOG_KICK_HIGH_RATE_MS/UC_TIMER_TICK_TIME_MS))
        {
            P1OUT |= BIT7;
        }
        //at 760ms set low (so its high for 10ms)
        else
        {
            P1OUT &= (~BIT7);
            wdKickTimer = 0u;
        }
    }

    //do a quick toggle on power up
    if ( firstKick == true && firstHigh == 0 )
    {
        P1OUT |= BIT7;
        firstHigh++;
    }
    else if ( firstKick == true && firstHigh == 1 )
    {
       P1OUT &= (~BIT7);
       firstKick = false;
    }

    __bic_SR_register_on_exit(LPM3_bits);
}

//This is the TIMER1_A1 interrupt vector service routine.

#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=TIMER1_A1_VECTOR
__interrupt
#elif defined(__GNUC__)
__attribute__((interrupt(TIMER1_A0_VECTOR)))
#endif
void TIMER1_A1_ISR (void)
{
    uint16_t compVal = 0;

    //make sure that this is the correct timer channel:
    if ( TA1IV == 0x02 )
    {
        compVal = Timer_A_getCaptureCompareCount(TIMER_A1_BASE,
                            TIMER_A_CAPTURECOMPARE_REGISTER_1)
                            + UC_TIME_1000_MS_IN_CYCLES;

        //Add Offset to CCR0
        Timer_A_setCompareValue(TIMER_A1_BASE,
            TIMER_A_CAPTURECOMPARE_REGISTER_1,
            compVal);

        //increment seconds counter
        SecondsCounter++;
        SecondsSinceLastHr++;

        __bic_SR_register_on_exit(LPM3_bits);
    }
}
