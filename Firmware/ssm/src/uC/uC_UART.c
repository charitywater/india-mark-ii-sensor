/**************************************************************************************************
* \file     uC_UART.c
* \brief    UART peripheral driver
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

#include "uC_UART.h"

#include <msp430.h>                      // Generic MSP430 Device Include
#include "driverlib.h"                   // MSPWare Driver Library
#include "APP_WTR.h"
#include "HW_ENV.h"
#include <stdio.h>
#include <string.h>
#include "HW_TERM.h"
#include "HW_AM.h"

#define GPIO_PORT_UCA0TXD       GPIO_PORT_P1
#define GPIO_PIN_UCA0TXD        GPIO_PIN0
#define GPIO_FUNCTION_UCA0TXD   GPIO_PRIMARY_MODULE_FUNCTION
#define GPIO_PORT_UCA0RXD       GPIO_PORT_P1
#define GPIO_PIN_UCA0RXD        GPIO_PIN1
#define GPIO_FUNCTION_UCA0RXD   GPIO_PRIMARY_MODULE_FUNCTION


void uC_UART_Init(void);
void uC_UART_Tx(uint8_t * p_buf, uint16_t len);

extern uint8_t HW_TERM_Rx_Buf[HW_TERM_RX_BUF_LEN];
extern uint8_t HW_TERM_Rx_Buf_Idx;
extern bool    HW_TERM_Command_Rdy;

//******************************************************************************
// USCI_A0 interrupt vector service routine.
//******************************************************************************
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=USCI_A0_VECTOR
__interrupt
#elif defined(__GNUC__)
__attribute__((interrupt(USCI_A0_VECTOR)))
#endif
void EUSCI_A0_ISR(void)
{
    switch(__even_in_range(UCA0IV,USCI_UART_UCTXCPTIFG))
    {
        case USCI_NONE: break;
        case USCI_UART_UCRXIFG:
            if (HW_TERM_CommandRdy() == false) // Don't start receiving a new command until the previous is received.
            {
                HW_TERM_RxByte(EUSCI_A_UART_receiveData(EUSCI_A0_BASE));
            }
        	__bic_SR_register_on_exit(LPM3_bits);
            break;
       case USCI_UART_UCTXIFG: break;
       case USCI_UART_UCSTTIFG: break;
       case USCI_UART_UCTXCPTIFG: break;
    }
}

void uC_UART_Init(void)
{
    // Configure UART
    // SMCLK = 250kHz, Baudrate = 9600/19200
    // Settings computed at http://software-dl.ti.com/msp430/msp430_public_sw/mcu/msp430/MSP430BaudRateConverter/index.html

    EUSCI_A_UART_initParam param = {0};
    param.selectClockSource = EUSCI_A_UART_CLOCKSOURCE_SMCLK;
    #ifdef STREAM_ENGINEERING_DATA
    param.clockPrescalar = 3;
    param.firstModReg = 4;
    param.secondModReg = 4;
    param.overSampling = EUSCI_A_UART_OVERSAMPLING_BAUDRATE_GENERATION;
    #else
    param.clockPrescalar = 104;
    param.firstModReg = 3;
    param.secondModReg = 0;
    param.overSampling = EUSCI_A_UART_OVERSAMPLING_BAUDRATE_GENERATION;
    #endif
    param.parity = EUSCI_A_UART_NO_PARITY;
    param.msborLsbFirst = EUSCI_A_UART_LSB_FIRST;
    param.numberofStopBits = EUSCI_A_UART_ONE_STOP_BIT;
    param.uartMode = EUSCI_A_UART_MODE;

    if (STATUS_FAIL == EUSCI_A_UART_init(EUSCI_A0_BASE, &param)) {
        return;
    }

    EUSCI_A_UART_enable(EUSCI_A0_BASE);

    EUSCI_A_UART_clearInterrupt(EUSCI_A0_BASE,
        EUSCI_A_UART_RECEIVE_INTERRUPT);

    // Enable USCI_A0 RX interrupt
    EUSCI_A_UART_enableInterrupt(EUSCI_A0_BASE,
        EUSCI_A_UART_RECEIVE_INTERRUPT);

    // Enable global interrupts
    __enable_interrupt();

}

// Transmit buffer on UART bus
void uC_UART_Tx(uint8_t * p_buf, uint16_t len)
{
    uint16_t i = 0;

    //Stop Watchdog Timer
    WDT_A_hold(WDT_A_BASE);

    for (i =0; i < len; i++)
    {
        // Load data onto buffer.  If transmit interrupt is disabled, this checks to make sure it is safe to write.
        EUSCI_A_UART_transmitData(EUSCI_A0_BASE, p_buf[i]);
    }
}
