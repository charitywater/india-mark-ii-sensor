/**************************************************************************************************
* \file     HW_MAG.c
* \brief    Magnetometer (lis2mdl) functions
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
#include "lis2mdl_reg.h"
#include "HW_TERM.h"
#include "am-ssm-spi-protocol.h"
#include "APP.h"
#include "string.h"
#include <stdio.h>
#include "uC_I2C.h"
#include "uC_TIME.h"
#include "HW_MAG.h"

#define READ_SIZE_BYTES         1
#define WRITE_SIZE_BYTES        2

#define MAG_HANDLE_VALUE        1

#define LSB_TO_TEMP_DIVISOR     8
#define LSB_TO_TEMP_OFFSET      25

#define LSB_TO_TEMP_DIVISOR     8
#define LSB_TO_TEMP_OFFSET      25

#define WRITE_READ_MULTIPLE_CMD 0x80

#define THRESH_TO_TRIGGER_LSB   35
#define THRESH_TO_TRIGGER_MSB   0

#define X_POSITION              0
#define Y_POSITION              1
#define Z_POSITION              2

/* return codes for the magnetometer */
typedef enum
{
    MAG_SUCCESS,
    MAG_READ_ERROR,
    MAG_WRITE_ERROR,
    MAG_NOT_INITIALIZED,
    MAG_UNKNOWN,
} magnetometer_status_t;

typedef union{
  int16_t i16bit[3];
  uint8_t u8bit[6];
} axis3bit16_t;

typedef union{
  int16_t i16bit;
  uint8_t u8bit[2];
} axis1bit16_t;

/* Read/write funtions for the ST driver. Need to fit their prototype */
static int32_t xWriteToChip(void *handle, uint8_t reg, uint8_t *bufp,
                                                uint16_t len);
static int32_t xReadFromChip(void *handle, uint8_t reg, uint8_t *bufp,
                                                uint16_t len);
static void xReadLatestSampleFromMag(void);

//static vars
static stmdev_ctx_t magControl;
static uint8_t magDeviceHandle = MAG_HANDLE_VALUE;
static uint8_t magWriteDataBuffer[WRITE_SIZE_BYTES];
static axis3bit16_t data_raw_magnetic;
static axis1bit16_t data_raw_temperature;
static bool dataRdy = false;
static lis2mdl_status_reg_t statusReg = {};
static uint32_t lastInterruptTime = 0u;
static bool xMagnetometerSamplingInitialized = false;
static bool xThresholdModeEnabled = false;

/* init the magnetometer bus and read out the ID of the chip */
void HW_MAG_InitBusAndDevice(void)
{
    uint8_t whoAmIReg = 0u;

    /* set up the device control */
    magControl.write_reg = xWriteToChip;
    magControl.read_reg = xReadFromChip;
    magControl.handle = &magDeviceHandle;

    /* read out the device ID */
    lis2mdl_device_id_get(&magControl, &whoAmIReg);

    if (whoAmIReg != LIS2MDL_ID)
    {
        HW_TERM_PrintColor("HW_MAG: ERROR. Unable to communicate with magnetometer.\n", KRED);
    }
    else
    {
        HW_TERM_Print("HW_MAG: Magnetometer communication successful.\n");

        //save power by putting the device into p down mode
        HW_MAG_TurnOffSampling();
    }
}

void HW_MAG_TurnOffSampling(void)
{
    uint8_t regValueToSet;

    //disable interrupts and clear state flags
    GPIO_disableInterrupt(GPIO_PORT_P2, GPIO_PIN7);
    xMagnetometerSamplingInitialized = false;
    xThresholdModeEnabled = false;
    dataRdy = false;

    //Set the magnetometer to do a reboot, which will start it up in IDLE mode
    //When putting the magnetometer into IDLE mode after its been sampling it
    //does not exit continuous mode

    //set the RST bit
    regValueToSet = 0x20;
    lis2mdl_write_reg(&magControl, LIS2MDL_CFG_REG_A, &regValueToSet, 1);

    //set the REBOOT bit - will be in IDLE mode now
    regValueToSet = 0x40;
    lis2mdl_write_reg(&magControl, LIS2MDL_CFG_REG_A, &regValueToSet, 1);
}

//perform any periodic functions & check for data ready
void HW_MAG_Monitor(void)
{
    lis2mdl_int_source_reg_t intReg;

    if ( dataRdy == true )
    {
        if ( xThresholdModeEnabled == true )
        {
            lis2mdl_int_gen_source_get(&magControl, &intReg);

            //alert the application level handler that a threshold interrupt occurred
            APP_indicateMagnetometerThresholdInterrupt();
        }
        else
        {
            //data is ready, pull the data from the magnetometer
            xReadLatestSampleFromMag();
        }

        //reset flag
        dataRdy = false;

        if ( (APP_getErrorBits() & MAG_ERROR) == MAG_ERROR )
        {
            APP_indicateErrorResolved(MAG_ERROR);
        }

        lastInterruptTime = uC_TIME_GetRuntimeSeconds();
    }
    else
    {
        //check for timeout condition - not getting new interrupts:
        if ( xMagnetometerSamplingInitialized == true )
        {
            if ( (uC_TIME_GetRuntimeSeconds() - lastInterruptTime) > FIVE_MINUTES )
            {
                //set error condition
                APP_indicateError(MAG_ERROR);

                //reset mag sample rate
                HW_MAG_InitSampleRateAndPowerModeOn();
            }
        }
    }
}

void HW_Mag_DataReadyIntOccured(void)
{
    //set the flag to true so that when the main loop
    //calls the periodic function, we can pull out the data!

    //this function is called from within an interrupt, so need to be speedy and bail
    dataRdy = true;
}

static void xReadLatestSampleFromMag(void)
{
    //read the status register to verify data is ready
    lis2mdl_status_get(&magControl, &statusReg);

    //check if new data is available
    if ( statusReg.zyxda > 0 )
    {
        //clear the last samples
        memset(data_raw_magnetic.u8bit, 0x00, sizeof(axis3bit16_t));
        memset(data_raw_temperature.u8bit, 0x00, sizeof(axis1bit16_t));

        /* read out the NEW magnetometer data */
        lis2mdl_magnetic_raw_get(&magControl, data_raw_magnetic.u8bit);
        lis2mdl_temperature_raw_get(&magControl, data_raw_temperature.u8bit);
    }
}

uint8_t HW_MAG_getID(void)
{
    uint8_t magId = 0u;
    lis2mdl_device_id_get(&magControl, &magId);

    return magId;
}


void HW_MAG_StartConversion(void)
{
    uint8_t regValueToSet = 0u;

    //single shot mode....temp compensation
     regValueToSet = 0x85;
     lis2mdl_write_reg(&magControl, LIS2MDL_CFG_REG_A, &regValueToSet, 1);
}

/* Get the latest sample from the magnetometer
 * The x y and z axis samples are in units of LSB: -1.5mG/LSB
 *
 * The temperature is also in units of LSB - convert using this: lsb/8 + 25 = C
 *
 * */

void HW_MAG_GetLatestMagAndTempData(int16_t *xLsb, int16_t *yLsb, int16_t *zLsb, int16_t *tempLsb, uint8_t *bitFlags)
{
    //set to the last sample we just pulled out of the magnetometer
    *xLsb = data_raw_magnetic.i16bit[0];
    *yLsb = data_raw_magnetic.i16bit[1];
    *zLsb = data_raw_magnetic.i16bit[2];
    *tempLsb = data_raw_temperature.i16bit;

    /*  Status Bitflags
        0 : xda - new data x
        1 : yda
        2 : zda
        3 : zyxda - new data x y AND z
        4 : _xor  - data overrun x
        5 : yor
        6 : zor
        7:  zyxor - data overrun on x y AND z
    */

    memcpy(bitFlags, &statusReg, sizeof(lis2mdl_status_reg_t));
}


void HW_MAG_SampleAndReport(void)
{
    int16_t x;
    int16_t y;
    int16_t z;
    int16_t temp;
    uint8_t statusBits;
    uint8_t str[50];

    HW_MAG_GetLatestMagAndTempData(&x, &y, &z, &temp, &statusBits);

    //convert temp to C
    temp = (temp/LSB_TO_TEMP_DIVISOR) + LSB_TO_TEMP_OFFSET;

    sprintf((char *)str, "MAG X: %d, MAG Y: %d, MAG Z: %d MAG TEMP: %d C \n", x, y, z, temp);
    HW_TERM_Print(str);
}

//Enable a threshold detection interrupt
//Samples at 10 Hz in a low resolution mode
void HW_MAG_EnableThresholdInterrupt(int16_t xAxisOffset, int16_t yAxisOffset, int16_t zAxisOffset)
{
    uint8_t regValueToSet = 0u;
    int16_t hardIronOffsets[3];
    lis2mdl_int_source_reg_t intReg;

    //clear any previous interrupts & set up for new ones
    GPIO_clearInterrupt(GPIO_PORT_P2, GPIO_PIN7);
    GPIO_disableInterrupt(GPIO_PORT_P2, GPIO_PIN7);

    // High-> Low edge
    GPIO_selectInterruptEdge(GPIO_PORT_P2, GPIO_PIN7, GPIO_HIGH_TO_LOW_TRANSITION);

    //clear interrupt register:
    lis2mdl_int_gen_source_get(&magControl, &intReg);

    //set continuous mode, temp compensation, 10Hz sample rate, LP
    //0x80 is high resolution if we end up needing this
    regValueToSet = 0x90;
    lis2mdl_write_reg(&magControl, LIS2MDL_CFG_REG_A, &regValueToSet, 1);

    //enable low pass filter and offset cancellation & hard iron compensation to trigger interrupt
    regValueToSet = 0x0B;
    lis2mdl_write_reg(&magControl, LIS2MDL_CFG_REG_B, &regValueToSet, 1);

    //threshold interrupt enable (INT_on_PIN)
    regValueToSet = 0x40;
    lis2mdl_write_reg(&magControl, LIS2MDL_CFG_REG_C, &regValueToSet, 1);

    //threshold of 35 LSB - TODO may want this as a config
    regValueToSet = THRESH_TO_TRIGGER_LSB;
    lis2mdl_write_reg(&magControl, LIS2MDL_INT_THS_L_REG, &regValueToSet, 1);

    regValueToSet = THRESH_TO_TRIGGER_MSB;
    lis2mdl_write_reg(&magControl, LIS2MDL_INT_THS_H_REG, &regValueToSet, 1);

    //set offsets
    hardIronOffsets[X_POSITION] = xAxisOffset;
    hardIronOffsets[Y_POSITION] = yAxisOffset;
    hardIronOffsets[Z_POSITION] = zAxisOffset;
    lis2mdl_mag_user_offset_set(&magControl, (uint8_t*)&hardIronOffsets);

    //latched active LOW interrupt on all 3 axis
    regValueToSet = 0xE3;
    lis2mdl_write_reg(&magControl, LIS2MDL_INT_CRTL_REG , &regValueToSet, 1);

    //set the mode
    xThresholdModeEnabled = true;
    xMagnetometerSamplingInitialized = false;
    dataRdy = false;

    //enable MSP interrupt on the pin
    GPIO_enableInterrupt(GPIO_PORT_P2, GPIO_PIN7);
}

//Put magnetometer into 20 Hz sample rate and data ready interrupt
void HW_MAG_InitSampleRateAndPowerModeOn(void)
{
    int16_t hardIronOffsets[3];
    uint8_t regValueToSet = 0u;

    //clear interrupts and enable new ones
    GPIO_clearInterrupt(GPIO_PORT_P2, GPIO_PIN7);
    GPIO_disableInterrupt(GPIO_PORT_P2, GPIO_PIN7);

    // Low-> High edge
    GPIO_selectInterruptEdge(GPIO_PORT_P2, GPIO_PIN7, GPIO_LOW_TO_HIGH_TRANSITION);

    //set up the offset registers for hard iron comp - currently 0
    memset(&hardIronOffsets, 0, 6);
    lis2mdl_mag_user_offset_set(&magControl, (uint8_t*)&hardIronOffsets);

    //set continuous mode, temp compensation, 50Hz sample rate
    regValueToSet = 0x84;
    lis2mdl_write_reg(&magControl, LIS2MDL_CFG_REG_A, &regValueToSet, 1);

    //enable low pass filter and offset cancellation
    regValueToSet = 0x03;
    lis2mdl_write_reg(&magControl, LIS2MDL_CFG_REG_B, &regValueToSet, 1);

    //data ready interrupt enable
    regValueToSet = 0x01;
    lis2mdl_write_reg(&magControl, LIS2MDL_CFG_REG_C, &regValueToSet, 1);

    //disable threshold interrupt detection
    regValueToSet = 0x00;
    lis2mdl_write_reg(&magControl, LIS2MDL_INT_CRTL_REG , &regValueToSet, 1);

    regValueToSet = 0x00;
    lis2mdl_write_reg(&magControl, LIS2MDL_INT_THS_L_REG, &regValueToSet, 1);

    regValueToSet = 0x00;
    lis2mdl_write_reg(&magControl, LIS2MDL_INT_THS_H_REG, &regValueToSet, 1);


    //now enable MSP interrupts on this pin
    GPIO_enableInterrupt(GPIO_PORT_P2, GPIO_PIN7);

    //Now read a sample from the magnetometer to get data ready interrupts going
    xReadLatestSampleFromMag();

    xMagnetometerSamplingInitialized = true;
    xThresholdModeEnabled = false;
}

void HW_MAG_ChangeSampleRate(uint8_t rateInHz)
{
    if (rateInHz == 10)
    {
        lis2mdl_data_rate_set(&magControl, LIS2MDL_ODR_10Hz);
        HW_TERM_Print("Set Sample Rate to 10 Hz");
    }
    else if (rateInHz == 20)
    {
        lis2mdl_data_rate_set(&magControl, LIS2MDL_ODR_20Hz);
        HW_TERM_Print("Set Sample Rate to 20 Hz");
    }
    else if (rateInHz == 50)
    {
        lis2mdl_data_rate_set(&magControl, LIS2MDL_ODR_50Hz);
        HW_TERM_Print("Set Sample Rate to 50 Hz");
    }
    else if (rateInHz == 100)
    {
        lis2mdl_data_rate_set(&magControl, LIS2MDL_ODR_100Hz);
        HW_TERM_Print("Set Sample Rate to 100 Hz");
    }
    else
    {
        HW_TERM_Print("Unable to set sample rate, provide a valid rate");
    }
}

void HW_MAG_ChangeOperatingMode(uint8_t mode)
{
    if (mode == 0)
    {
        lis2mdl_operating_mode_set(&magControl, LIS2MDL_CONTINUOUS_MODE);
        HW_TERM_Print("Set continuous mode");
    }
    else if (mode == 1)
    {
        lis2mdl_operating_mode_set(&magControl, LIS2MDL_SINGLE_TRIGGER);
        HW_TERM_Print("Set single trigger");
    }
    else if (mode == 2)
    {
        lis2mdl_operating_mode_set(&magControl, LIS2MDL_POWER_DOWN);
        HW_TERM_Print("Set power down");
    }
    else
    {
        HW_TERM_Print("Provide a valid operating mode");
    }
}

/* return the current operating mode of the magnetometer */
uint8_t HW_MAG_GetOperatingMode(void)
{
    lis2mdl_md_t val = LIS2MDL_CONTINUOUS_MODE;
    lis2mdl_operating_mode_get(&magControl, &val);

    return val;
}

/* functions to read and write to the magnetometer */

static int32_t xWriteToChip(void *handle, uint8_t reg, uint8_t *bufp,
                                                uint16_t len)
{
    int32_t stat = MAG_UNKNOWN;

    if (handle == &magDeviceHandle)
    {
        /* Write multiple command */
        reg |= WRITE_READ_MULTIPLE_CMD;

        magWriteDataBuffer[0] = reg;
        memcpy(&magWriteDataBuffer[1], bufp, len);
        uC_I2C_WriteMulti(LIS2MDL_I2C_ADD,  (uint8_t*)&magWriteDataBuffer, len+1, false);

        stat = MAG_SUCCESS;
    }

    return stat;
}

static int32_t xReadFromChip(void *handle, uint8_t reg, uint8_t *bufp,
                                                uint16_t len)
{
    int32_t stat = MAG_UNKNOWN;
    uint8_t readData = 0;

    if (handle == &magDeviceHandle)
    {
        /* Read multiple command */
        reg |= WRITE_READ_MULTIPLE_CMD;

        //need to read multiple bytes...
        if (len > 1)
        {
            while (len > 0)
            {
                readData = uC_I2C_ReadRegSingle(LIS2MDL_I2C_ADD, reg, false);

                reg++;
                *bufp = readData;
                bufp++;
                len--;
            }
        }
        else
        {
            readData = uC_I2C_ReadRegSingle(LIS2MDL_I2C_ADD, reg, false);
            *bufp = readData;
        }
    }

    stat = MAG_SUCCESS;
    return stat;
}
