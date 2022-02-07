/**************************************************************************************************
* \file     uC_I2C.c
* \brief    I2C driver
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
#include "gpio.h"
#include "HW_TERM.h"
#include "uC_I2C.h"
#include "CAPT_BSP.h"

#define uC_I2C_MAX_MSG_LEN        130
#define uC_I2C_TIMEOUT_MS         50
#define REVOVER_BUS_TOGGLES       32
#define RECOVER_BUS_DELAY_TICKS   800

void uC_I2C_WriteRegSingle(uint8_t slave_addr, uint8_t reg_addr, uint8_t value, bool retry_on_nak);
bool uC_I2C_WriteMulti(uint8_t slave_addr, uint8_t * p_payload, uint8_t num_bytes, bool retry_on_nak);
uint8_t uC_I2C_ReadRegSingle(uint8_t slave_addr, uint8_t reg_addr, bool retry_on_nak);
void uC_I2C_Init(void);

static uint8_t Num_Tx_Bytes = 2;
static uint8_t Tx_Index = 0;
static uint8_t Tx_Data[uC_I2C_MAX_MSG_LEN] = {};
static bool Retry_On_NAK = false;
static bool Unexpected_NACK = false;

// Initialize I2C
// todo: Probably will move this to an i2c manager module when ready to bring up more slaves.
void uC_I2C_Init(void)
{
    uint16_t delayTicks = 0;
    uint16_t toggles = 0;

    //Init the I2C pins as GPIO to determine if the data line is being held low
    //by any of the peripherals
    GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN3);
    GPIO_setAsInputPin(GPIO_PORT_P1, GPIO_PIN2);

    //check if the data line is low
    if (GPIO_getInputPinValue(GPIO_PORT_P1, GPIO_PIN2) == 0)
    {
        HW_TERM_Print("I2C Data line held low...Will attempt to recover\n\n");

        //Manually toggle the clock line to get the i2c slave to release the
        //data line
        for(toggles = 0; toggles < REVOVER_BUS_TOGGLES; toggles++)
        {
            GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN3);
            //small delay
            for(delayTicks = 0; delayTicks< RECOVER_BUS_DELAY_TICKS; delayTicks++) ;
            GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN3);
            //small delay
            for(delayTicks =0; delayTicks < RECOVER_BUS_DELAY_TICKS; delayTicks++) ;
        }
    }

    //Now set up the i2c pins as peripheral pins instead of i/o
    GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P1, GPIO_PIN3, GPIO_PRIMARY_MODULE_FUNCTION);
    GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P1, GPIO_PIN2, GPIO_PRIMARY_MODULE_FUNCTION);

    //Initialize I2c peripheral as master device
    EUSCI_B_I2C_initMasterParam param = {0};
    param.selectClockSource = EUSCI_B_I2C_CLOCKSOURCE_SMCLK;
    param.i2cClk = SMCLK_FREQ;
    param.dataRate = EUSCI_B_I2C_SET_DATA_RATE_400KBPS;
    param.byteCounterThreshold = 1;
    param.autoSTOPGeneration = EUSCI_B_I2C_NO_AUTO_STOP;
    EUSCI_B_I2C_initMaster(EUSCI_B0_BASE, &param);
}


// Read a single byte from a register.  Writes the register value to set up the pointer, then performs a read.
uint8_t uC_I2C_ReadRegSingle(uint8_t slave_addr, uint8_t reg_addr, bool retry_on_nak)
{
    volatile uint8_t rx_data = 0x00;
    bool status;

    Retry_On_NAK = retry_on_nak;

    //Initialize transmit data packet
    Tx_Data[0] = reg_addr;

    //Specify slave address
    EUSCI_B_I2C_setSlaveAddress(EUSCI_B0_BASE,slave_addr);

    //Set in transmit mode
    EUSCI_B_I2C_setMode(EUSCI_B0_BASE, EUSCI_B_I2C_TRANSMIT_MODE);

    //Enable I2C Module to start operations
    EUSCI_B_I2C_enable(EUSCI_B0_BASE);

    //Send single byte data.
    status = EUSCI_B_I2C_masterSendSingleByteWithTimeout(EUSCI_B0_BASE,Tx_Data[0], 100);

    // If the slave ack'd, get the data.  Otherwise just return.
    if (status == STATUS_SUCCESS)
    {
        //Delay until transmission completes
        while (EUSCI_B_I2C_isBusBusy(EUSCI_B0_BASE));

        //Set Master in receive mode
        EUSCI_B_I2C_setMode(EUSCI_B0_BASE,EUSCI_B_I2C_RECEIVE_MODE);

        EUSCI_B_I2C_clearInterrupt(EUSCI_B0_BASE, EUSCI_B_I2C_RECEIVE_INTERRUPT0 + EUSCI_B_I2C_BYTE_COUNTER_INTERRUPT);

        __disable_interrupt();
        rx_data = EUSCI_B_I2C_masterReceiveSingleByte(EUSCI_B0_BASE);
        __enable_interrupt();
    }

    EUSCI_B_I2C_clearInterrupt(EUSCI_B0_BASE, EUSCI_B_I2C_NAK_INTERRUPT);

    return rx_data;
}

// Write a single byte to a register.  Writes the register value to set up the pointer, then writes the value.
void uC_I2C_WriteRegSingle(uint8_t slave_addr, uint8_t reg_addr, uint8_t value, bool retry_on_nak)
{
    bool status;

    Retry_On_NAK = retry_on_nak;

    WDT_A_hold(WDT_A_BASE);

    //Specify slave address
    EUSCI_B_I2C_setSlaveAddress(EUSCI_B0_BASE, slave_addr);

    //Set in transmit mode
    EUSCI_B_I2C_setMode(EUSCI_B0_BASE, EUSCI_B_I2C_TRANSMIT_MODE);

    //Enable I2C Module to start operations
    EUSCI_B_I2C_enable(EUSCI_B0_BASE);

    while (EUSCI_B_I2C_isBusBusy(EUSCI_B0_BASE));

    //Initialize transmit data packet
    Tx_Data[0] = reg_addr;

    //Send single byte data.
    status = EUSCI_B_I2C_masterSendSingleByteWithTimeout(EUSCI_B0_BASE,Tx_Data[0], 100);

    if (status == STATUS_SUCCESS)
    {
        //Delay until transmission completes
        while (EUSCI_B_I2C_isBusBusy(EUSCI_B0_BASE));

        Tx_Data[0] = value;

        //Send single byte data.
        EUSCI_B_I2C_masterSendSingleByte(EUSCI_B0_BASE,Tx_Data[0]);
    }

    EUSCI_B_I2C_clearInterrupt(EUSCI_B0_BASE, EUSCI_B_I2C_NAK_INTERRUPT);
}



// Write multiple data bytes.  Expects data to be in Tx_Data[].
bool uC_I2C_WriteMulti(uint8_t slave_addr, uint8_t * p_payload, uint8_t num_bytes, bool retry_on_nak)
{
    bool result = false;

    Retry_On_NAK = retry_on_nak;

    memcpy(Tx_Data, p_payload, num_bytes);  // Put the command in the transmit buffer

    WDT_A_hold(WDT_A_BASE);

    //Specify slave address
    EUSCI_B_I2C_setSlaveAddress(EUSCI_B0_BASE, slave_addr);

    //Set in transmit mode
    EUSCI_B_I2C_setMode(EUSCI_B0_BASE, EUSCI_B_I2C_TRANSMIT_MODE);

    //Enable I2C Module to start operations
    EUSCI_B_I2C_enable(EUSCI_B0_BASE);

    EUSCI_B_I2C_clearInterrupt(EUSCI_B0_BASE, EUSCI_B_I2C_TRANSMIT_INTERRUPT0 + EUSCI_B_I2C_NAK_INTERRUPT);
    //Enable master Tx interrupt
    EUSCI_B_I2C_enableInterrupt(EUSCI_B0_BASE, EUSCI_B_I2C_TRANSMIT_INTERRUPT0 + EUSCI_B_I2C_NAK_INTERRUPT);

    Num_Tx_Bytes = num_bytes;
    Tx_Index = 0;

    result = EUSCI_B_I2C_masterSendMultiByteStartWithTimeout(EUSCI_B0_BASE, Tx_Data[Tx_Index], uC_I2C_TIMEOUT_MS);
    if (result == STATUS_SUCCESS)
    {
        while (EUSCI_B_I2C_isBusBusy(EUSCI_B0_BASE));

        // The only expected NACK is from EEPROM while waiting for a write to complete.  If we see another one,
        // there is a problem.
        if (Unexpected_NACK == true)
        {
            Unexpected_NACK = false;
            result = false;
        }
        else
        {
            result = true;
        }
    }

    EUSCI_B_I2C_disableInterrupt(EUSCI_B0_BASE, EUSCI_B_I2C_TRANSMIT_INTERRUPT0 + EUSCI_B_I2C_NAK_INTERRUPT);
    EUSCI_B_I2C_clearInterrupt(EUSCI_B0_BASE, EUSCI_B_I2C_TRANSMIT_INTERRUPT0 + EUSCI_B_I2C_NAK_INTERRUPT);

    return result;
}

#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=USCI_B0_VECTOR
__interrupt
#elif defined(__GNUC__)
__attribute__((interrupt(USCI_B0_VECTOR)))
#endif
void USCIB0_ISR(void)
{
    switch(__even_in_range(UCB0IV, USCI_I2C_UCBIT9IFG))
  {
        case USCI_NONE:             // No interrupts break;
            break;
        case USCI_I2C_UCALIFG:      // Arbitration lost
            break;
        case USCI_I2C_UCNACKIFG:    // NAK received (master only)
            //resend start if NACK
            // The only NACKs we should be receiving are from EEP when we are selective read polling it to see if a write is complete.
            if(Retry_On_NAK == true)
            {
                EUSCI_B_I2C_masterSendMultiByteStart(EUSCI_B0_BASE, Tx_Data[Tx_Index]);
            }
            else
            {
                Unexpected_NACK = true;
                EUSCI_B_I2C_masterSendMultiByteStop(EUSCI_B0_BASE);
            }

            // Exit LPM3
            __bic_SR_register_on_exit(LPM3_bits);
            break;
        case USCI_I2C_UCTXIFG0:     // TXIFG0
            // Check TX byte counter
            Tx_Index++;
            if (Tx_Index < Num_Tx_Bytes)
            {
                EUSCI_B_I2C_masterSendMultiByteNext(EUSCI_B0_BASE, Tx_Data[Tx_Index]);
            }
            else
            {
                EUSCI_B_I2C_masterSendMultiByteStop(EUSCI_B0_BASE);
                // Exit LPM3
                __bic_SR_register_on_exit(LPM3_bits);
            }
            break;
        default:
            break;
  }
}
