/**************************************************************************************************
* \file     HW_EEP.c
* \brief    EEPROM driver. Interfaces with the CAT24C512WI
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
#include <uC/inc/uC_I2C.h>
#include "driverlib.h"                   // MSPWare Driver Library
#include "HW_EEP.h"
#include "HW_TERM.h"
#include "HW_GPIO.h"
#include "APP.h"
#include "am-ssm-spi-protocol.h"
#include "uC_TIME.h"

#define HW_EEP_WRITE_TEST_ADDR      HW_EEP_END_ADDR
#define HW_EEP_WRITE_TEST_VAL       0xA5

#define HW_EEP_ADDR_SIZE_BYTES      2
#define HW_EEP_ADDR_MSB_POSITION    0
#define HW_EEP_ADDR_LSB_POSITION    1
#define HW_EEP_DATA_BYTE_POSITION   2

#define HW_EEP_CMD_SIZE_BYTES       1
#define WRITE_TEST                  {HW_EEP_WRITE_TEST_ADDR_MSB, HW_EEP_WRITE_TEST_ADDR_LSB,HW_EEP_WRITE_TEST_VAL}
#define HW_EEP_NUM_PAGES            512u
#define COMM_RETRIES                2

// EEP Can write up to 128 bytes at a time, plus two bytes for address.
#define HW_EEP_PAGE_LEN_BYTES       128u
uint8_t HW_EEP_Write_Buf[HW_EEP_PAGE_LEN_BYTES + HW_EEP_ADDR_SIZE_BYTES];


void HW_EEP_Init(void);
void HW_EEP_DoTest(void);
uint8_t HW_EEP_ReadByte(uint16_t addr);
void HW_EEP_WriteByte(uint16_t addr, uint8_t value);
void HW_EEP_EraseAll(void);
void HW_EEP_WriteBlock(uint16_t addr, uint8_t * p_values, uint8_t num_bytes);

static void xEnableWrite(void);
static void xDisableWrite(void);
static bool xWaitForWriteToFinish(void);

void HW_EEP_Init(void)
{
    xDisableWrite();
}

// De-assert WP so that we can write.
static void xEnableWrite(void)
{
    HW_GPIO_Clear_WP_EEPRM();
}

// Assert WP to inhibit writes.
static void xDisableWrite(void)
{
    HW_GPIO_Set_WP_EEPRM();
}

// Read a single byte from EEP.
uint8_t HW_EEP_ReadByte(uint16_t addr)
{
    uint8_t cmd[HW_EEP_ADDR_SIZE_BYTES];
    uint8_t rx_data = 0x00;
    uint8_t retry = COMM_RETRIES;
    bool stat = false;

    while ( retry > 0 && stat == false )
    {
        // "Dummy" write to set up address pointer.
        cmd[HW_EEP_ADDR_MSB_POSITION] = ((uint8_t)(addr >> 8));
        cmd[HW_EEP_ADDR_LSB_POSITION] = ((uint8_t)addr);

        if (uC_I2C_WriteMulti(HW_EEP_SLAVE_ADDR, cmd, HW_EEP_ADDR_SIZE_BYTES, false) == true)    // Send it as a multi-byte transmission.
        {
            //Set Master in receive mode and receive the value.
            EUSCI_B_I2C_setMode(EUSCI_B0_BASE,EUSCI_B_I2C_RECEIVE_MODE);
            EUSCI_B_I2C_clearInterrupt(EUSCI_B0_BASE, EUSCI_B_I2C_RECEIVE_INTERRUPT0 + EUSCI_B_I2C_BYTE_COUNTER_INTERRUPT);


            // WARNING: Interrupts must be disabled while EUSCI_B_I2C_masterReceiveSingleByte() is executing.  Otherwise, if an interrupt arrives at a specific time,
            //          this function's busy wait poll can miss the reception and not set the stop condition.  This results in receiving two bytes and creates a
            //          lockup as a result.
            __disable_interrupt();
            rx_data = EUSCI_B_I2C_masterReceiveSingleByte(EUSCI_B0_BASE);
            __enable_interrupt();

            stat = true;
        }
        else
        {
            HW_TERM_PrintColor("HW_EEP: ERROR.  Could not read EEP.\n", KRED);
        }

        retry--;
    }

    if ( stat == false )
    {
        HW_TERM_PrintColor("HW_EEP: ERROR.  Could not read EEP - FAILED RETRIES .\n", KRED);
        APP_indicateError(EEPROM_READ_ERROR);
    }

    return rx_data;
}


uint8_t BytesToEndOfPage(uint16_t addr)
{
    uint16_t current_page = 0;
    uint16_t next_page = 0;
    uint16_t next_page_addr = 0;

    current_page = (addr / HW_EEP_PAGE_LEN_BYTES);
    next_page = (current_page + 1);
    next_page_addr = (next_page * HW_EEP_PAGE_LEN_BYTES);

    return (next_page_addr - addr);
}

void HW_EEP_WriteBlock(uint16_t addr, uint8_t * p_data, uint8_t num_bytes)
{
    uint8_t bytes_this_write = 0;
    uint8_t bytes_written = 0;
    uint8_t bytes_to_end_of_page = 0;
    uint8_t retry = COMM_RETRIES;
    bool pass = false;

    if (p_data == NULL) return;

    while ( num_bytes != 0u )
    {
        HW_EEP_Write_Buf[0] = ((addr >> 8) & 0x00FF);           // Put address at beginning of write buffer.
        HW_EEP_Write_Buf[1] = (addr  & 0x00FF);

        bytes_to_end_of_page = BytesToEndOfPage(addr);
        if (num_bytes > bytes_to_end_of_page)
        {
            bytes_this_write = bytes_to_end_of_page;
            num_bytes -= bytes_to_end_of_page;
        }
        else
        {
            bytes_this_write = num_bytes;
            num_bytes = 0;
        }

        memcpy(&(HW_EEP_Write_Buf[2]), &(p_data[bytes_written]), bytes_this_write);      // Then append the data to be written.

        xEnableWrite();
        pass = false;
        retry = COMM_RETRIES;

        while ( retry > 0 && pass == false )
        {
            if (uC_I2C_WriteMulti(HW_EEP_SLAVE_ADDR, HW_EEP_Write_Buf, (bytes_this_write + sizeof(addr)), false) == true)                  // Send it as a multi-byte transmission.
            {
                if ( xWaitForWriteToFinish() == true )
                {
                    bytes_written += bytes_this_write;
                    addr += bytes_this_write;

                    pass = true;
                }
            }
            else
            {
                HW_TERM_PrintColor("HW_EEP: ERROR. Could not write block to EEP.\n", KRED);
            }

            retry--;
        }

        if ( pass == false)
        {
            HW_TERM_PrintColor("HW_EEP: ERROR. Could not write block to EEP - FAILED RETRIES.\n", KRED);
            num_bytes = 0;
            APP_indicateError(EEPROM_WRITE_ERROR);
        }

        xDisableWrite();
    }
}

// Write a single byte to EEP and wait for write to complete.
void HW_EEP_WriteByte(uint16_t addr, uint8_t value)
{
    uint8_t cmd[HW_EEP_ADDR_SIZE_BYTES + HW_EEP_CMD_SIZE_BYTES];
    uint8_t lenToWrite = HW_EEP_ADDR_SIZE_BYTES + HW_EEP_CMD_SIZE_BYTES;
    uint8_t retry = COMM_RETRIES;
    bool pass = false;

    cmd[HW_EEP_ADDR_MSB_POSITION] = ((uint8_t)(addr >> 8));
    cmd[HW_EEP_ADDR_LSB_POSITION] = ((uint8_t)addr);
    cmd[HW_EEP_DATA_BYTE_POSITION] = (value);

    xEnableWrite();

    while (retry > 0 && pass == false )
    {
        if (uC_I2C_WriteMulti(HW_EEP_SLAVE_ADDR, cmd, lenToWrite, false) == true)                  // Send it as a multi-byte transmission.
        {
            if ( xWaitForWriteToFinish() == true )
            {
                pass = true;
            }

        }
        else
        {
            HW_TERM_PrintColor("HW_EEP: ERROR. Could not write byte to EEP.\n", KRED);
        }

        retry--;
    }

    if ( pass == false)
    {
        HW_TERM_PrintColor("HW_EEP: ERROR. Could not write byte to EEP. - FAILED RETRIES \n", KRED);
        APP_indicateError(EEPROM_WRITE_ERROR);
    }

    xDisableWrite();
}

// Write value to test address and read it back to verify write/read.  Then do the same with the bitwise not of the test value.
void HW_EEP_DoTest(void)
{
    uint8_t rx_data = 0x00;
    bool test_pass = true;

    HW_EEP_WriteByte(HW_EEP_WRITE_TEST_ADDR, HW_EEP_WRITE_TEST_VAL);
    rx_data = HW_EEP_ReadByte(HW_EEP_WRITE_TEST_ADDR);

    if (rx_data != HW_EEP_WRITE_TEST_VAL)
        test_pass = false;

    HW_EEP_WriteByte(HW_EEP_WRITE_TEST_ADDR, ((uint8_t)(~HW_EEP_WRITE_TEST_VAL)));
    rx_data = HW_EEP_ReadByte(HW_EEP_WRITE_TEST_ADDR);

    if (rx_data !=  ((uint8_t)(~HW_EEP_WRITE_TEST_VAL)))
        test_pass = false;


    if (test_pass == true)
    {
        HW_TERM_PrintColor("\nHW_EEP: CAT24C512WI write/read test pass.\n", KGRN);
    }
    else
    {
        HW_TERM_PrintColor("\nHW_EEP: CAT24C512WI write/read test fail!\n", KRED);
    }
}

void HW_EEP_EraseAll(void)
{
    uint32_t i = 0;
    uint8_t ffBuffer[HW_EEP_PAGE_LEN_BYTES];

    memset(&ffBuffer, 0xFF, HW_EEP_PAGE_LEN_BYTES);

    //erase each page
    for(i = 0; i <= HW_EEP_NUM_PAGES; i++)
    {
        HW_EEP_WriteBlock(HW_EEP_PAGE_LEN_BYTES*i, (uint8_t*)&ffBuffer, HW_EEP_PAGE_LEN_BYTES);
    }
}

// Per the data sheet, perform a dummy selective read and wait for the chip to ACK its
// address, which indicates the write is complete.  Technically, we're not actually
// reading here- just setting up the pointer, which seems to be all that is required.
// We will receive several nacks after a single byte write.
static bool xWaitForWriteToFinish(void)
{
    uint8_t cmd[3];
    uint32_t target_S = 0;
    bool pass = true;

    cmd[0] = 0;
    cmd[1] = 0;

    // We're going to make sure that we wait at least 1.01 seconds (depending on the tick counter)
    // and up to 2s.  Using lower resolution here not a problem since if we get to this function,
    // we know that the EEP responded earier in the sequence. So, the problem here would be
    // some issue internal to the EEP.
    target_S = (uC_TIME_GetRuntimeSeconds() + 2);

    uC_I2C_WriteMulti(HW_EEP_SLAVE_ADDR, cmd, 2, true);                  // Send it as a multi-byte transmission.
    while (EUSCI_B_I2C_isBusBusy(EUSCI_B0_BASE))
    {
        if (uC_TIME_GetRuntimeSeconds() >= target_S)
        {
            HW_TERM_PrintColor("HW_EEP: ERROR. EEP could not complete write after good coms.\n", KRED);
            pass = false;
            break; // Don't want to keep waiting.
        }
    };

   return pass;
}
