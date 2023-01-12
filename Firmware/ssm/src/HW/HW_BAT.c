/**************************************************************************************************
* \file     HW_BAT.c
* \brief    Battery Fuel gauge & voltage measurement functionality.
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

#include <msp430.h>
#include "driverlib.h"                   // MSPWare Driver Library
#include <stdint.h>
#include "stdbool.h"
#include "string.h"
#include "stdio.h"
#include "HW_TERM.h"
#include "uC_ADC.h"
#include "HW_BAT.h"

//------------------------------------------------------------------------------
// Single-wire commands Registers
//------------------------------------------------------------------------------
#define READ_ROM                0x33                // Read ROM command
#define MATCH_ROM               0x55                // Match ROM command
#define SKIP_ROM                0xCC                // Skip ROM command

#define WRITE_MEM               0x0F                // Write memory command
#define VERITY_MEM              0xAA                // Verify memory command
#define COPY_MEM                0x55                // Copy memory command
#define READ_MEM                0xF0                // Read memory command

#define BITS_PER_BYTE           8

#define OWI_RESET_BAUD          10000
#define OWI_READ_WRITE_BAUD     100000

#define RESET_PULSE             0xF0
#define DEVICE_PRESENT          0xE0
#define READ_TX_VALUE           0xFF
#define READ_ZERO_MAX_VALUE     0xFE
#define READ_ONE_VALUE          0xFF

// Timer clock frequency (MHz)
#define CLKFREQ                 16u

//------------------------------------------------------------------------------
// Define Single-wire Protocol Related Timing Constants
//------------------------------------------------------------------------------
#define RESET_LOW               (500 * CLKFREQ)            // Set line low (500us)
#define RESET_DETECT            (80 * CLKFREQ)             // Reset detect (80us)
#define RESET_DELAY             (360 * CLKFREQ)            // Reset delay (280us)

#define WRITE_ONE               (6 * CLKFREQ)              // Write byte 1 (6us)
#define DELAY_ONE               (64 * CLKFREQ)             // Write delay 1 (64us)
#define WRITE_ZERO              (60 * CLKFREQ)             // Write byte 0 (60us)
#define DELAY_ZERO              (10 * CLKFREQ)             // Write delay 0 (10us)

#define READ_LOW                (6 * CLKFREQ)              // Set line low (6us)
#define READ_DETECT             (9 * CLKFREQ)              // Slave response (9us)
#define READ_DELAY              (55 * CLKFREQ)             // Slave release (55us)

//FUEL GAUGE SPECIFIC:
#define DS2740U_PICO_V_PER_CURR_LSB     1562500 //1.5625uV
#define DS2740U_PICO_V_HR_PER_ACR_LSB   6250000 //6.250uV

#define COULOMB_PICO_V_PER_CURR_LSB     DS2740U_PICO_V_PER_CURR_LSB
#define COULOMB_PICO_V_HR_PER_ACR_LSB   DS2740U_PICO_V_HR_PER_ACR_LSB

#define NANO_AMPS_PER_LSB               (COULOMB_PICO_V_PER_CURR_LSB/R_SNS_MILLI_OHMS)
#define NANO_AMP_HOURS_PER_LSB          (COULOMB_PICO_V_HR_PER_ACR_LSB/R_SNS_MILLI_OHMS)
#define NANO_AMPS_PER_MICRO_AMP         1000

//per schematic
#define R_SNS_MILLI_OHMS                33

#define SKIP_NET_ADDR_CMD               0xCC
#define READ_DATA_CMD                   0x69
#define WRITE_DATA_CMD                  0x6C
#define MAX_RD_WR_LEN                   2
#define STATUS_REG_ADDR                 0x01
#define STATUS_REG_LEN                  1
#define CURRENT_REG_ADDR                0x0E
#define CURRENT_REG_LEN                 2
#define ACCUM_REG_ADDR                  0x10
#define ACCUM_REG_LEN                   2
#define STATUS_REG_SMOD_MASK            0x40
#define SERIAL_NUM_LEN                  8


//voltage divider multiplier/divisor at the
//input to the battery ADC pin (see schematic)
#define BAT_VOLTAGE_DIVIDER_MULT        9
#define BAT_VOLTAGE_DIVIDER_DIVISOR     5

#define CYCLES_PROP_DELAY               8000
#define SAMPLES_PER_MEASUREMENT         21

//Based on Flexco's depleted battery testing
//Range could be 2500 - 2800 & is temperature dependent
//can do some testing to find sweet spot in the future
#define CRITICAL_VOLTAGE_THRESHOLD_MV   2700

//one wire interface
typedef enum
{
    OWI_NOT_INITIALIZED,
    OWI_SUCCESS,
    OWI_INVALID_PARAMS,
    OWI_NO_DEVICE_DETECTED,
}owiStatus_t;

typedef struct
{
    uint8_t netAddrCmd;
    uint8_t functCmd;
    uint8_t addr;
    uint8_t data0;
    uint8_t data1;
}__attribute__((packed)) fuelGaugeOwiTxData_t;

static uint8_t rxDataByte;
static uint8_t printBuffer[25];
static uint8_t fuelGaugeId[SERIAL_NUM_LEN];
static bool isBatteryCriticallyLow = false;
static uint16_t latestBatteryVoltageMv = 0u;

static void xSingleReset(void);
static void xSingleWrite(uint8_t);
static uint8_t xSingleRead(void);
static void sortVoltages(uint16_t *arrayOfVoltages , uint8_t length);
static uint16_t findVoltageMedian(uint16_t arrayOfVoltages[] , uint8_t length);
static owiStatus_t xOwiWriteRead(uint8_t* txData, uint8_t txLen, uint8_t* rxData, uint8_t rxLen);
static fuelGaugeStatus_t xReadRegister(uint8_t addr, uint8_t* data, uint8_t len);
static fuelGaugeStatus_t xWriteRegister(uint8_t addr, uint8_t* data, uint8_t len);

fuelGaugeStatus_t HW_FUEL_GAUGE_Initialize(void)
{
    GPIO_setAsOutputPin(GPIO_PORT_P2, GPIO_PIN2);
    GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN2);

    uint8_t statusRegVal = 0;
    uint8_t idByte = 0;

    xSingleReset();
    xSingleWrite(READ_ROM);

    //read the ID on power up and save it
    for(idByte = 0; idByte<SERIAL_NUM_LEN; idByte++)
    {
        fuelGaugeId[idByte] = xSingleRead();
    }

    //PUT FUEL GAUGE INTO SLEEP MODE:

    //enable sleep in the status register
    statusRegVal |= (BIT6);
    HW_FUEL_GAUGE_WriteStatusReg(statusRegVal);

    statusRegVal=0xFF;
    __delay_cycles(RESET_LOW);

    //read it back
    HW_FUEL_GAUGE_ReadStatusReg(&statusRegVal);

    if ( statusRegVal & BIT6 )
    {
        HW_TERM_Print("\r\nFuel Gauge Sleep Mode Enabled\r\n");
    }
    else
    {
        HW_TERM_Print("\r\nFuel Gauge Sleep NOT Enabled");
        sprintf((char *)printBuffer, "\r\n reg: 0x%X", statusRegVal);
        HW_TERM_Print(printBuffer);
    }

    //CLEAR data line to put the fuel gauge to sleep
    GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN2);

    return GAUGE_SUCCESS;
}

void HW_FUEL_GAUGE_PrintSerialNumber(void)
{
    //print out the ID that we have saved off
    HW_TERM_Print("\r\nFuel Gauge Serial Number: ");
    sprintf((char *)printBuffer, "0x%02X%02X%02X%02X%02X%02X%02X%02X",fuelGaugeId[0],
            fuelGaugeId[1],fuelGaugeId[2],fuelGaugeId[3],fuelGaugeId[4],fuelGaugeId[5],fuelGaugeId[6],
            fuelGaugeId[7]);
    HW_TERM_Print(printBuffer);
}


void HW_BAT_Init(void)
{
    uint16_t currentVoltage = 0u;

    //ADC en pin - low to prevent excess current draw
    GPIO_setAsOutputPin(GPIO_PORT_P3, GPIO_PIN2);
    GPIO_setOutputLowOnPin(GPIO_PORT_P3, GPIO_PIN2);

    //grab a voltage measurement:
    HW_BAT_TakeNewVoltageMeasurement();

    currentVoltage = HW_BAT_GetVoltage();

    sprintf((char *)printBuffer, "\r\nBattery Voltage %d mV", currentVoltage);
    HW_TERM_Print(printBuffer);
}

uint16_t HW_BAT_GetVoltage(void)
{
    return latestBatteryVoltageMv;
}

void HW_BAT_TakeNewVoltageMeasurement(void)
{
    uint16_t battVoltageMv[SAMPLES_PER_MEASUREMENT] = {};
    uint16_t voltageReading = 0;
    uint8_t idx = 0;
    uint16_t medianVoltageMv = 0;

    //enable FET to connect adc input pin to the battery
    GPIO_setOutputHighOnPin(GPIO_PORT_P3, GPIO_PIN2);

    //Propagation delay from turning on the FET to reaching full voltage, allow a few hundred microseconds
    __delay_cycles(CYCLES_PROP_DELAY);

   for ( idx = 0; idx < SAMPLES_PER_MEASUREMENT; idx++ )
   {
       voltageReading = uC_ADC_newConversionCh0();

       //Apply voltage divider math to the measurement
       battVoltageMv[idx] = (voltageReading * BAT_VOLTAGE_DIVIDER_MULT) / BAT_VOLTAGE_DIVIDER_DIVISOR;
   }

    //disable FET to prevent excess current draw
    GPIO_setOutputLowOnPin(GPIO_PORT_P3, GPIO_PIN2);

    //now get the median - we do this instead of an average to prevent outliers
    //impacting the result since we know this could be noisy
    sortVoltages((uint16_t*)&battVoltageMv, SAMPLES_PER_MEASUREMENT);

    medianVoltageMv = findVoltageMedian(battVoltageMv, SAMPLES_PER_MEASUREMENT);

    //update voltage with the median
    latestBatteryVoltageMv = medianVoltageMv;

    //check critical voltage here since we just took a new reading
    if ( latestBatteryVoltageMv <= CRITICAL_VOLTAGE_THRESHOLD_MV )
    {
        isBatteryCriticallyLow = true;
        HW_TERM_Print("CRITICALLY LOW BATTERY LEVEL DETECTED");
    }
}

// function to sort the array in ascending order
static void sortVoltages(uint16_t *arrayOfVoltages , uint8_t length)
{
    int i = 0 , j = 0 , temp = 0;

    for( i = 0; i < length; i++ )
    {
        for( j = 0; j < length-1; j++ )
        {
            if( arrayOfVoltages[j] > arrayOfVoltages[j+1] )
            {
                temp = arrayOfVoltages[j];
                arrayOfVoltages[j] = arrayOfVoltages[j+1];
                arrayOfVoltages[j+1] = temp;
            }
        }
    }
}

static uint16_t findVoltageMedian(uint16_t arrayOfVoltages[] , uint8_t length)
{
    uint16_t medianVoltage = 0;

    // if number of elements are even
    if(length%2 == 0)
    {
        medianVoltage = (arrayOfVoltages[(length-1)/2] + arrayOfVoltages[length/2])/2;
    }
    // if number of elements are odd
    else
    {
        medianVoltage = arrayOfVoltages[length/2];
    }

    return medianVoltage;
}


bool HW_BAT_IsBatteryLow(void)
{
    return isBatteryCriticallyLow;
}

//void HW_FUEL_GAUGE_readAccumulatedCurrent(int32_t* accumulator_uAh)
//{
//    uint16_t acrRegVal;
//
//    HW_FUEL_GAUGE_ReadAccumReg(&acrRegVal);
//
//    // Convert to milli-amp hours
//    *accumulator_uAh = ((int32_t)((int16_t)acrRegVal)*NANO_AMP_HOURS_PER_LSB)/NANO_AMPS_PER_MICRO_AMP;
//}
//
//void HW_FUEL_GAUGE_readInstantCurrent(int32_t* instCurrent_uA)
//{
//    uint16_t currentRegVal;
//    HW_FUEL_GAUGE_ReadCurrentReg(&currentRegVal);
//
//    sprintf((char *)printBuffer, "\r\n raw val 0x%X", currentRegVal);
//    HW_TERM_Print(printBuffer);
//
//    // Convert to micro-amps
//    *instCurrent_uA = ((int32_t)((int16_t)currentRegVal)*NANO_AMPS_PER_LSB)/NANO_AMPS_PER_MICRO_AMP;
//}


static fuelGaugeStatus_t xReadRegister(uint8_t addr, uint8_t* data, uint8_t len)
{
    fuelGaugeOwiTxData_t owiTxData;
    owiStatus_t owiStatus;
    uint8_t rxData[MAX_RD_WR_LEN];
    uint8_t i;

    if(data==NULL || len==0 || len>MAX_RD_WR_LEN)
    {
        return GAUGE_INVALID_PARAM;
    }

    owiTxData.netAddrCmd = SKIP_NET_ADDR_CMD;
    owiTxData.functCmd = READ_DATA_CMD;
    owiTxData.addr = addr;
    owiStatus = xOwiWriteRead((uint8_t*)&owiTxData, 3, rxData, len);

    // Swap byte order; coulomb regs are read out MSB first
    for(i=0; i<len; i++)
    {
        data[i]=rxData[(len-1)-i];
    }

    if(owiStatus != OWI_SUCCESS)
    {
        return GAUGE_OWI_ERROR;
    }

    return GAUGE_SUCCESS;
}

static fuelGaugeStatus_t xWriteRegister(uint8_t addr, uint8_t* data, uint8_t len)
{
    fuelGaugeOwiTxData_t owiTxData;
    owiStatus_t owiStatus;

    if(data==NULL || len==0 || len>MAX_RD_WR_LEN)
    {
        return GAUGE_INVALID_PARAM;
    }

    owiTxData.netAddrCmd = SKIP_NET_ADDR_CMD;
    owiTxData.functCmd = WRITE_DATA_CMD;
    owiTxData.addr = addr;
    owiTxData.data0 = data[0];
    owiTxData.data1 = data[1];
    owiStatus = xOwiWriteRead((uint8_t*)&owiTxData, 3+len, NULL, 0);

    if(owiStatus != OWI_SUCCESS)
    {
        return GAUGE_OWI_ERROR;
    }

    return GAUGE_SUCCESS;
}

fuelGaugeStatus_t HW_FUEL_GAUGE_ReadStatusReg(uint8_t* data)
{
    return xReadRegister(STATUS_REG_ADDR, data, STATUS_REG_LEN);
}

fuelGaugeStatus_t HW_FUEL_GAUGE_WriteStatusReg(uint8_t data)
{
    return xWriteRegister(STATUS_REG_ADDR, &data, STATUS_REG_LEN);
}

//fuelGaugeStatus_t HW_FUEL_GAUGE_ReadCurrentReg(uint16_t* data)
//{
//    return xReadRegister(CURRENT_REG_ADDR, (uint8_t*)data, CURRENT_REG_LEN);
//}
//
//fuelGaugeStatus_t HW_FUEL_GAUGE_ReadAccumReg(uint16_t* data)
//{
//    return xReadRegister(ACCUM_REG_ADDR, (uint8_t*)data, ACCUM_REG_LEN);
//}
//
//fuelGaugeStatus_t HW_FUEL_GAUGE_ClearAccumReg(void)
//{
//    uint16_t accumRegWriteVal = 0x0000;
//
//    return xWriteRegister(ACCUM_REG_ADDR, (uint8_t*)&accumRegWriteVal, ACCUM_REG_LEN);
//}

static owiStatus_t xOwiWriteRead(uint8_t* txData, uint8_t txLen, uint8_t* rxData, uint8_t rxLen)
{
    uint32_t byteIdx;

    if(txData==0 || txLen==0)
    {
        return OWI_INVALID_PARAMS;
    }

    xSingleReset();

    // Send each bit of data (LSb first) as a full byte of UART data
    for(byteIdx=0; byteIdx<txLen; byteIdx++)
    {

       xSingleWrite(txData[byteIdx]);
    }

    if(rxData != 0 && rxLen != 0)
    {
        memset(rxData, 0, rxLen); // Start with cleared data for bit-wise logic below to work
        for(byteIdx=0; byteIdx<rxLen; byteIdx++)
        {
            rxDataByte = xSingleRead();
            rxData[byteIdx] = rxDataByte;
        }
    }

    return OWI_SUCCESS;
}


static void xSingleReset(void)
{
    P2OUT &= (~BIT2);                           // P2.2 driven low
    __delay_cycles(RESET_LOW);                  // Set delay
    P2DIR = 0x00;                               // P2.2 input
    P2OUT |=  BIT2;                             // select pull-up mode, 1
    P2REN |= BIT2;                              // enable internal pull up, 1
    __delay_cycles(RESET_DELAY);                // Set delay
    P2OUT = BIT2;                               // Reset P2.2 to output high
    P2DIR = BIT2;
}

static void xSingleWrite(uint8_t Data)
{
    uint8_t bit;                                // Initialize bit counter
    for (bit = 0; bit < 8; bit++)               // Loop 8 bits to write a byte
    {
        P2OUT &= (~BIT2);                       // Set P2.2 low
        if((Data >> bit) & 0x01)                // If bit is a 1
        {
            __delay_cycles(WRITE_ONE);          // Set delay
            P2OUT |= BIT2;                      // Set P2.2 back to high
            __delay_cycles(DELAY_ONE);          // Set delay
        }
        else                                    // Else if a 0
        {
            __delay_cycles(WRITE_ZERO);         // Set delay
            P2OUT |= BIT2;                      // Set P2.2 back to high
            __delay_cycles(DELAY_ZERO);         // Set delay
        }
    }
}


static uint8_t xSingleRead(void)
{
    uint8_t bit;                                // Initialize bit counter
    uint8_t Data = 0;                           // Initialize return data byte
    for (bit = 0; bit < 8; bit++)               // Loop 8 bits to read a byte
    {
        P2OUT &= (~BIT2);                       // P2.2 driven low
        __delay_cycles(READ_LOW);               // Set delay
        P2DIR = 0x00;                           // P2.2 input
        P2OUT |=  BIT2;                         // select pull-up mode, 1
        P2REN |= BIT2;                          // enable internal pull up, 1
        __delay_cycles(READ_DETECT);            // Set delay
        if(P2IN & BIT2) Data |= (0x01 << bit);  // If high then read 1, else 0
        __delay_cycles(READ_DELAY);             // Set delay
        P2OUT |= BIT2;                           // Reset P2.2 to output high
        P2DIR = BIT2;
    }

    return Data;
}
