/**************************************************************************************************
* \file     uC_SPI.c
* \brief    SPI driver
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

#include "uC_SPI.h"
#include <msp430.h>                             // Generic MSP430 Device Include
#include "driverlib.h"                          // MSPWare Driver Library
#include <stdio.h>
#include <string.h>
#include "HW_TERM.h"
#include "HW_GPIO.h"

#define UC_SPI_RX_CIRC_BUFF_LEN 250
#define UC_SPI_TX_CIRC_BUFF_LEN 250

//buffer to store received characters
uint8_t Rx_Circ_Buff[UC_SPI_RX_CIRC_BUFF_LEN] = {};
uint8_t Rx_Head_Index = 0;
uint8_t Rx_Tail_Index = 0;

//buffer of characters to send
uint8_t Tx_Buff[UC_SPI_TX_CIRC_BUFF_LEN] = {};  // Currently not circular.  One Tx at any given time. (No DMA)
uint8_t Tx_Head_Index = 0;
uint8_t Tx_Tail_Index = 0;
static bool txMode = false;


void uC_SPI_Init(void);
bool uC_SPI_BytesReady(void);
uint8_t uC_SPI_GetNextByte(void);
bool uC_SPI_Tx(uint8_t * p_bytes, uint8_t num_bytes);

void uC_SPI_Init(void)
{
    //Stop watchdog timer
    WDT_A_hold(WDT_A_BASE);

    //Initialize slave to MSB first, inactive high clock polarity and 3 wire SPI
    EUSCI_A_SPI_initSlaveParam param = {0};
    param.msbFirst = EUSCI_A_SPI_MSB_FIRST;
    param.clockPhase = EUSCI_A_SPI_PHASE_DATA_CAPTURED_ONFIRST_CHANGED_ON_NEXT;
    param.clockPolarity = EUSCI_A_SPI_CLOCKPOLARITY_INACTIVITY_LOW;
    param.spiMode = EUSCI_A_SPI_3PIN;

    EUSCI_A_SPI_initSlave(EUSCI_A1_BASE, &param);

    //Enable SPI Module
    EUSCI_A_SPI_enable(EUSCI_A1_BASE);

    //Enable Transmit interrupt
    EUSCI_A_SPI_clearInterrupt(EUSCI_A1_BASE, EUSCI_A_SPI_TRANSMIT_INTERRUPT);
    EUSCI_A_SPI_enableInterrupt(EUSCI_A1_BASE, EUSCI_A_SPI_TRANSMIT_INTERRUPT);

    //Enable Receive interrupt
    EUSCI_A_SPI_clearInterrupt(EUSCI_A1_BASE, EUSCI_A_SPI_RECEIVE_INTERRUPT);
    EUSCI_A_SPI_enableInterrupt(EUSCI_A1_BASE, EUSCI_A_SPI_RECEIVE_INTERRUPT);

    //clear out the tx register by sending a dummy byte
    EUSCI_A_SPI_transmitData(EUSCI_A1_BASE, 0x00);

    //init gpio to a known state
    HW_GPIO_Clear_WAKE_AP();
    HW_GPIO_Set_SSM_RDY();
}

// Return true if bytes are waiting in the receive buffer.
bool uC_SPI_BytesReady(void)
{
  return ((Rx_Head_Index == Rx_Tail_Index) ? false : true);
}

// Get the next byte out of the receive buffer.
uint8_t uC_SPI_GetNextByte(void)
{
    uint8_t byte = 0;

    //pull the oldest byte from the buffer (FIFO)
    byte = Rx_Circ_Buff[Rx_Tail_Index];

    //update the  rx tail
    Rx_Tail_Index++;

    //wrap back around
    if ( Rx_Tail_Index >= UC_SPI_RX_CIRC_BUFF_LEN )
    {
        Rx_Tail_Index = 0;
    }

    return byte;
}

// Transmit the number of bytes provided.  Returns true if data was ok.
bool uC_SPI_Tx(uint8_t * p_bytes, uint8_t num_bytes)
{
    if (num_bytes > UC_SPI_RX_CIRC_BUFF_LEN) return false;
    if (p_bytes == NULL) return false;
    if (Tx_Tail_Index != Tx_Head_Index) return false;

    __disable_interrupt();

    txMode = true;

    Tx_Tail_Index = 0;
    Tx_Head_Index = num_bytes;
    memcpy(Tx_Buff, p_bytes, num_bytes);

    //Clear the data ready pin for the AM to get the clock started
    HW_GPIO_Clear_SSM_RDY();

    //begin the transfer - send first byte of the buffer
    EUSCI_A_SPI_transmitData(EUSCI_A1_BASE,
                             Tx_Buff[Tx_Tail_Index]);  // Currently not circular.  One Tx at any given time.

    //increment the index of the buffer.
    //When the interrupt fires, it will then transmit the second byte
    Tx_Tail_Index++;

    __enable_interrupt();

    return true;
}

#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=USCI_A1_VECTOR
__interrupt
#elif defined(__GNUC__)
__attribute__((interrupt(USCI_A1_VECTOR)))
#endif
void USCI_A1_ISR (void)
{
    switch(__even_in_range(UCA1IV, USCI_SPI_UCTXIFG))
    {
       // transmit interrupt
        case USCI_SPI_UCTXIFG:
        {
            //if we have more bytes, send them
            if (( Tx_Tail_Index < Tx_Head_Index)  && txMode == true )
            {
               //Transmit data to master
               EUSCI_A_SPI_transmitData(EUSCI_A1_BASE,
                                        Tx_Buff[Tx_Tail_Index]);
               Tx_Tail_Index++;

            }
            else
            {
                txMode = false;

                //no more data to send. Go back to receiving
                HW_GPIO_Set_SSM_RDY();
            }

            break;
        }

        //Receive interrupt
        case USCI_SPI_UCRXIFG:

            //verify we are NOT transmitting
            //we actually get an RX interrupt on transmits as well and need to
            //differentiate between what to parse and what to discard
            if ( txMode == false )
            {
                //get the data out of the rx register, put it in the RX buffer
                Rx_Circ_Buff[Rx_Head_Index] = EUSCI_A_SPI_receiveData(EUSCI_A1_BASE);

                //now move the head pointer for the next rx
                Rx_Head_Index++;

                //wrap back around if we exceeded the buffer size
                if ( Rx_Head_Index >= UC_SPI_RX_CIRC_BUFF_LEN )
                {
                    Rx_Head_Index = 0;
                }
            }
            else
            {
                //call the rx function to clear the interrupt
                EUSCI_A_SPI_receiveData(EUSCI_A1_BASE);
            }

            break;
        default:
            break;
    }
	__bic_SR_register_on_exit(LPM3_bits);
}
