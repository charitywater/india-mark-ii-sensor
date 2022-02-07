/**************************************************************************************************
* \file     HW_ENV.c
* \brief    Driver to interface with the temperature and humidity sensor (HDC2010)
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
#include <stdio.h>
#include <uC/inc/uC_I2C.h>
#include "HW.h"
#include "HW_TERM.h"
#include "am-ssm-spi-protocol.h"
#include "APP.h"
#include "HW_ENV.h"

// Temperature LSB register
#define HW_ENV_TEMP_LSB_REG_ADDR                0x00

// Temperature MSB register
#define HW_ENV_TEMP_MSB_REG_ADDR                0x01

// Humidity LSB register
#define HW_ENV_HUMID_LSB_REG_ADDR               0x02

// Humidity MSB register
#define HW_ENV_HUMID_MSB_REG_ADDR               0x03

// Interrupt/DRDY (Data Ready) register
#define HW_ENV_INT_DRDY_REG_ADDR                0x04
#define HW_ENV_INT_DRDY_STATUS_BIT              BIT_7

#define HW_ENV_INTERRUPT_CONFIG_REG_ADDR        0x0E

//data ready interrupt on, high polarity, heater disabled
#define HW_ENV_BASE_CONFIG_VALUE                0x06

//5Hz
#define HW_ENV_DEFAULT_SAMPLE_RATE              7
#define SAMPLE_RATE_BIT_SHIFT                   4

#define HW_ENV_INT_ENABLE_REG_ADDR              0x07

// Measurement configuration register
#define HW_ENV_MEAS_CFG_REG_ADDR                0x0F

//write 0x01 to the above address to kick off a sample
#define HW_ENV_MEAS_CFG_MEAS_TRIG_BIT           BIT_0

// Manufacturing ID lower byte.
#define HW_ENV_MFG_ID_LOW_REG_ADDR              0xFC

// This is the value we should receive when reading the manufacturing ID low byte.
#define HW_ENV_MFG_ID_LOW_VAL                   0x49

// Manufacturing ID high byte.
#define HW_ENV_MFG_ID_HIGH_REG_ADDR             0xFD

// This is the value we should receive when reading the manufacturing ID high byte.
#define HW_ENV_MFG_ID_HIGH_VAL                  0x54

#define TEMP_HUMID_CONVERSION_DIVIDER           65535
#define TEMP_CONVERSION_MULTIPLIER              165
#define TEMP_CONVERSION_SUBTRACT                40

#define CMD_REG_IDX                             0
#define CMD_VALUE_IDX                           1
#define CMD_LEN_BYTES                           2

#define COMM_RETRIES                            2


void HW_ENV_DoTest(void);
void HW_ENV_Init(void);
bool HW_ENV_CheckMfgID(void);
HW_ENV_SAMPLE_T * HW_ENV_GetLatestSample(void);
void HW_ENV_GetLatestSampleAndReport(void);
static bool xWriteToRegister(uint8_t reg, uint8_t value);
static void xSampleTempAndHumidity(void);

static uint8_t xCmdBuffer[CMD_LEN_BYTES] = {};
static HW_ENV_SAMPLE_T xLatestSample = {.temp_c = 0, .humidity = 0, .temp_c_raw = 0, .humidity_raw = 0};
static bool xDataRdy = false;

//Initialize the temp/humidity sensor
void HW_ENV_Init(void)
{
    //configure the HDC2010 for manually triggered samples, no interrupts

    // Leaving this if an interrupt is needed in the future
    //route data ready signal to the interrupt pin, low -> high
    GPIO_clearInterrupt(GPIO_PORT_P2, GPIO_PIN3);
    GPIO_selectInterruptEdge(GPIO_PORT_P2, GPIO_PIN3, GPIO_LOW_TO_HIGH_TRANSITION);
    GPIO_disableInterrupt(GPIO_PORT_P2, GPIO_PIN3);

    //Disable 'data ready' interrupt line of the sensor (Change this to 0x80 to enable the interrupt)
    xWriteToRegister(HW_ENV_INT_ENABLE_REG_ADDR, 0x00);

    //data ready interrupt off, heater disabled, auto sampling disabled "000 = Disabled"
    // For data ready interrupt with high polarity change to 0x06
    xWriteToRegister(HW_ENV_INTERRUPT_CONFIG_REG_ADDR, 0x00);

    //turn off sampling
    xWriteToRegister(HW_ENV_MEAS_CFG_REG_ADDR, 0x00);

    HW_ENV_Monitor();

    //read out ID
    HW_ENV_DoTest();
}

//call this function periodically to flush out
//the temp/humidity sensor if it has data
void HW_ENV_Monitor(void)
{
    if (xDataRdy == true)
    {
        xSampleTempAndHumidity();
        xDataRdy = false;
    }
}

void HW_ENV_TriggerNewEnvSample(void)
{
    xWriteToRegister(HW_ENV_MEAS_CFG_REG_ADDR, 0x01);
}

void HW_ENV_GetNewEnvSample(void)
{
    xSampleTempAndHumidity();
}

void HW_ENV_DataRdyInt(void)
{
    //Note: this function is called from within an interrupt

    //set flag and bail
    xDataRdy = true;
}

//return a pointer to the latest sample read from the sensor
HW_ENV_SAMPLE_T* HW_ENV_GetLatestSample(void)
{
    return &xLatestSample;
}

//Get sample from the sensor
static void xSampleTempAndHumidity(void)
{
    uint8_t lsb = 0x00;
    uint8_t msb = 0x00;
    uint16_t raw = 0x0000;
    float float_val = 0.0;

    // *******  Read and convert temperature
    lsb = uC_I2C_ReadRegSingle(HW_ENV_SLAVE_ADDR, HW_ENV_TEMP_LSB_REG_ADDR, false);
    msb = uC_I2C_ReadRegSingle(HW_ENV_SLAVE_ADDR, HW_ENV_TEMP_MSB_REG_ADDR, false);

    raw = ((msb << 8) + lsb);

    xLatestSample.temp_c_raw = raw;

    // Convert from raw to degrees C
    float_val = (float) raw/TEMP_HUMID_CONVERSION_DIVIDER;
    float_val *= TEMP_CONVERSION_MULTIPLIER;
    float_val -= TEMP_CONVERSION_SUBTRACT;
    xLatestSample.temp_c = (uint8_t) float_val;  // Currently returning an integer C

    // *******  Read and convert humidity
    lsb = uC_I2C_ReadRegSingle(HW_ENV_SLAVE_ADDR, HW_ENV_HUMID_LSB_REG_ADDR, false);
    msb = uC_I2C_ReadRegSingle(HW_ENV_SLAVE_ADDR, HW_ENV_HUMID_MSB_REG_ADDR, false);

    raw = ((msb << 8) + lsb);

    xLatestSample.humidity_raw = raw;

    // Convert from raw to percentage
    float_val = (float) raw/TEMP_HUMID_CONVERSION_DIVIDER;
    float_val *= 100.0;
    xLatestSample.humidity = (uint8_t) float_val;  // Humidity percentage.
}

bool HW_ENV_ChangeSampleRate(uint8_t newSampleRateInHz)
{
    bool stat = false;
    uint8_t regValue = 0;
    uint8_t str[50];
    /*
     * Auto Measurement Mode (AMM)
        000 = Disabled. Initiate measurement via I2C
        001 = 1/120Hz (1 samples every 2 minutes)
        010 = 1/60Hz (1 samples every minute)
        011 = 0.1Hz (1 samples every 10 seconds)
        100 = 0.2 Hz (1 samples every 5 second)
        101 = 1Hz (1 samples every second)
        110 = 2Hz (2 samples every second)
        111 = 5Hz (5 samples every second)
     */

    if ( newSampleRateInHz <= HW_ENV_DEFAULT_SAMPLE_RATE )
    {

        //turn off sampling
        xWriteToRegister(HW_ENV_MEAS_CFG_REG_ADDR, 0x00);

        //data ready interrupt on, high polarity, heater disabled, new sample rate
        regValue = HW_ENV_BASE_CONFIG_VALUE;
        regValue|= (newSampleRateInHz << SAMPLE_RATE_BIT_SHIFT);
        xWriteToRegister(HW_ENV_INTERRUPT_CONFIG_REG_ADDR, regValue);

        sprintf((char *)str, "HW_ENV: New sample rate %d \n", newSampleRateInHz);
        HW_TERM_Print(str);

        stat = true;
    }
    else
    {
        HW_TERM_Print("INVALID sample rate");
    }

    return stat;
}

// Read the high and low bytes of the manufacturing ID and compare them to the expected values.
bool HW_ENV_CheckMfgID(void)
{
    bool rv = false;
    uint8_t id_low = 0x00;
    uint8_t id_high = 0x00;

    id_low = uC_I2C_ReadRegSingle(HW_ENV_SLAVE_ADDR, HW_ENV_MFG_ID_LOW_REG_ADDR, false);
    id_high = uC_I2C_ReadRegSingle(HW_ENV_SLAVE_ADDR, HW_ENV_MFG_ID_HIGH_REG_ADDR, false);

    //check if the id's match
    if ((id_low == HW_ENV_MFG_ID_LOW_VAL) &&
        (id_high == HW_ENV_MFG_ID_HIGH_VAL))
    {
        rv = true;
    }

    return rv;
}

// Grab the latest sample and print it out
void HW_ENV_GetLatestSampleAndReport(void)
{
    uint8_t str[50];

    //pull sample from chip:
    xSampleTempAndHumidity();

    //grab the value in RAM & print
    volatile HW_ENV_SAMPLE_T * p_sample = HW_ENV_GetLatestSample();
#if INT_MAX >= UCHAR_MAX
    sprintf((char *)str, "HW_ENV: Temperature: %u\370C\n", p_sample->temp_c);
#else
    #error Use %u or %hhu.
#endif
    HW_TERM_Print(str);
#if INT_MAX >= UCHAR_MAX
    sprintf((char *)str, "HW_ENV: Humidity: %u%%\n", p_sample->humidity);
#else
    #error Use %u or %hhu.
#endif
    HW_TERM_Print(str);
}

// Read the manufacturing ID and, if it is correct, report temperature and humidity.
void HW_ENV_DoTest(void)
{
    if (HW_ENV_CheckMfgID() == true)
    {
        HW_TERM_PrintColor("\nHW_ENV: HDC2010 Mfg ID test pass.\n", KGRN);
    }
    else
    {
        HW_TERM_PrintColor("\nHW_ENV: HDC2010 Mfg ID test fail!\n", KRED);
    }
}


//helper function to write to the sensor registers
static bool xWriteToRegister(uint8_t reg, uint8_t value)
{
    bool stat = false;
    uint8_t retry = COMM_RETRIES;

    xCmdBuffer[CMD_REG_IDX] = reg;
    xCmdBuffer[CMD_VALUE_IDX] = value;

    while ( retry > 0 && stat == false )
    {
        stat = uC_I2C_WriteMulti(HW_ENV_SLAVE_ADDR, (uint8_t*)&xCmdBuffer, CMD_LEN_BYTES, false);
        retry--;
    }

    if ( stat == false )
    {
        HW_TERM_PrintColor("\nHW_ENV: FAILED TO READ REGISTER \n", KRED);
        APP_indicateError(TEMP_HUMID_ERROR);
    }

    return stat;
}
