/* --COPYRIGHT--,BSD
 * Copyright (c) 2017, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * --/COPYRIGHT--*/
//*****************************************************************************
// CAPT_BSP.c
//
// *** CLOCK CONFIGURATION ***
// MCLK is 16 MHz, sourced from the DCO
// SMCLK is 2 MHz, sourced from MCLK
// ACLK is 32kHz, sourced from XT1 or REFO, with XT1 having priority
// FLLREF is 32kHz, sourced from XT1 or REFO, with XT1 having priority
//
// *** SERIAL INTERFACES ***
// This board support package supports the MSP430FR2633 device.
//
// \version 1.80.00.30
// Released on March 26, 2019
//
//*****************************************************************************

#include <msp430.h>
#include <stdint.h>

#include "CAPT_BSP.h"

static bool XT1_Good = false;

bool BSP_XT1_Good(void);


bool BSP_XT1_Good(void)
{
    return XT1_Good;
}

//*****************************************************************************
//
//! This function is configures the MCU Digital IO and CS for operation.
//
//*****************************************************************************
void BSP_configureMCU(void)
{
    uint8_t ui8LFOsc = CS_XT1CLK_SELECT;

    // Direction register.  0=Input.  1=Output.

    // PxSEL are used in combo for function selection.  See the data sheet for selectable functions.
    // PxSEL1   PxSEL0
    //   0        0         GPIO
    //   0        1         Primary function.
    //   1        0         Secondary function.
    //   1        1         Tertiary function.

    // PORT1
    // P1.0: ADC CH 0 - INPUT
    // P1.1: SSM_BOOT. INPUT.
    // P1.2: I2C_SDA. UCB0 I2C SDA
    // P1.3: I2C_SCL. UCB0 I2C SCL
    // P1.4: SSM_TXD. UCA0 UART TXD
    // P1.5: SSM_RXD. UCA0 UART RXD
    // P1.6: WAKE_AP.  OUTPUT LOW.
    // P1.7: Watchdog kick
    P1OUT  = (1);
    P1DIR  = (GPIO_PIN6 | GPIO_PIN7 );
    P1SEL0 = (GPIO_PIN2 | GPIO_PIN3 | GPIO_PIN4 | GPIO_PIN5 );  // Set primary function for pins 2,3,4,5.
    P1SEL1 = (0);

    PM5CTL0 &= ~LOCKLPM5; // Engage GPIOs

    //set wd enable LOW on power up
    P1OUT |= BIT7;

    // PORT2
    // P2.0: SYSTEM_OFF
    // P2.1: XIN
    // P2.2: FUEL GAUGE DATA LINE -I/O
    // P2.3: INT_TEMP. INPUT.
    // P2.4: STM_SCK.
    // P2.5: STM_MISO.
    // P2.6: STM_MOSI.
    // P2.7: INT_MAG. INPUT.
    P2OUT  =  (1);
    P2DIR  =  (GPIO_PIN2);
    P2SEL0 =  (GPIO_PIN1 | GPIO_PIN4 | GPIO_PIN5 | GPIO_PIN6);  // Set primary function for pins 0,1,4,5,6.
    P2SEL1 =  (0);

    //Explicitly set P2.0 as GPIO (not XTAL)
    P2SEL0 &= ~(BIT0);
    P2SEL1 &= ~(BIT0);

    //but set it as an input to prevent unwanted system resets
    P2DIR &= ~BIT0;

    // Specify we are using only XIN for oscillator (so we can use XOUT as GPIO)
    CSCTL6 |= XT1BYPASS;

    // PORT3
    // P3.1: STM_CS. (slave transmit enable)
    // P3.2: ADC Read EN
    P3OUT  =  (1);
    P3DIR  =  (GPIO_PIN1 | GPIO_PIN2);
    P3SEL0 =  (0);
    P3SEL1 =  (0);

    //Clear p3.2 to prevent excess current draw
    //(this pin is used to enable ADC measurements)
    P3OUT &= ~GPIO_PIN2;

    //PORT 4
    //P4.1 - WP EEP
    P4OUT  =  (1);
    P4DIR  =  (GPIO_PIN1);
    P4SEL0 =  (0);
    P4SEL1 =  (0);

    //
    // Clear port lock
    //
    PM5CTL0 &= ~LOCKLPM5;

    // Configure FRAM wait state (set to 1 to support 16MHz MCLK)
    FRAMCtl_configureWaitStateControl(FRAMCTL_ACCESS_TIME_CYCLES_1);

    //another watchdog toggle during boot up
    P1OUT &= (~BIT7);

    //
    // Attempt to start the low frequency crystal oscillator
    //
    CS_setExternalClockSource(XT1_OSC_FREQ);

    //call this function because we are using a TCXO instead of a crystal...
    if ( CS_bypassXT1WithTimeout(XT1_OSC_TIMEOUT) == STATUS_FAIL)
    {
        //
        // If an oscillator  is not present or is failing, switch the LF
        // clock definition to the internal 32kHz reference oscillator.
        //
        ui8LFOsc = CS_REFOCLK_SELECT;
        XT1_Good = false;
    }
    else
    {
        XT1_Good = true;
    }

    //another watchdog toggle during boot up
    P1OUT |= BIT7;

    //
    // Initialize Clock Signals
    //
    // *** CLOCK CONFIGURATION ***
    // DCO is configured to output 1Mhz (lowest available)
    // MCLK is dividing DCO by 1 to run at 1Mhz.
    // SMCLK is dividing by 1 sourced from MCLK to run at 1MHz (See figure 3-1 in the user guide.  the smclk divider comes after mclk's)
    // ACLK is 32kHz, sourced from XT1
    // FLLREF is 32kHz, sourced from XT1

    CS_initClockSignal(CS_FLLREF, ui8LFOsc, CS_CLOCK_DIVIDER_1);
    CS_initClockSignal(CS_ACLK, ui8LFOsc, CS_CLOCK_DIVIDER_1);
    CS_initClockSignal(CS_MCLK, CS_DCOCLKDIV_SELECT, CS_CLOCK_DIVIDER_1);
    CS_initClockSignal(CS_SMCLK, CS_DCOCLKDIV_SELECT, CS_CLOCK_DIVIDER_1);

    //another watchdog toggle during boot up
    P1OUT &= (~BIT7);

    //
    // Tune the DCO parameters
    //
    CS_initFLL((DCO_FREQ/1000), FLL_RATIO);
    CS_clearAllOscFlagsWithTimeout(1000);
}

//*****************************************************************************
//
//! This function disables the watchdog timer during boot, ensuring a WDT
//! reset does not occur during boot before main() is entered.
//
//*****************************************************************************
int _system_pre_init(void)
{
    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer
    return 1;
}
