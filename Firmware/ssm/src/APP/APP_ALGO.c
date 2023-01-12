/**************************************************************************************************
* \file     APP_ALGO.c
* \brief    Application level algorithm management
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

#include <stdio.h>
#include "HW_MAG.h"
#include "APP_WTR.h"
#include "APP_ALGO.h"
#include "APP.h"
#include "HW_TERM.h"
#include "APP_NVM_Cfg.h"
#include "am-ssm-spi-protocol.h"
#include "APP_NVM_Custom.h"

/* Algorithm Includes */
#include "algo-c-code/calculateWaterVolume/calculateWaterVolume.h"
#include "algo-c-code/clearMagWindowProcess/clearMagWindowProcess.h"
#include "algo-c-code/clearPadWindowProcess/clearPadWindowProcess.h"
#include "algo-c-code/cliResetStrokeCount/cliResetStrokeCount.h"
#include "algo-c-code/detectStrokes/detectStrokes.h"
#include "algo-c-code/detectTransitions/detectTransitions.h"
#include "algo-c-code/hourlyStrokeCount/hourlyStrokeCount.h"
#include "algo-c-code/hourlyWaterVolume/hourlyWaterVolume.h"
#include "algo-c-code/initializeMagCalibration/initializeMagCalibration.h"
#include "algo-c-code/initializeStrokeAlgorithm/initializeStrokeAlgorithm.h"
#include "algo-c-code/initializeWaterAlgorithm/initializeWaterAlgorithm.h"
#include "algo-c-code/initializeWindows/initializeWindows.h"
#include "algo-c-code/magnetometerCalibration/magnetometerCalibration.h"
#include "algo-c-code/wakeupDataReset/wakeupDataReset.h"
#include "algo-c-code/waterPadFiltering/waterPadFiltering.h"
#include "algo-c-code/writeMagSample/writeMagSample.h"
#include "algo-c-code/writePadSample/writePadSample.h"
#include "algo-c-code/getMaxUsageTime/getMaxUsageTime.h"
#include "algo-c-code/computePumpHealth/computePumpHealth.h"

#define DEBUG_STRLEN                50
#define STATIC_SAMPLE_COUNT_MAX     200
#define WEEKS_FOR_RED_FLAG_CALC     4
#define DAYS_PER_WEEK               7
#define DAYS_FOR_RED_FLAG_CALC      WEEKS_FOR_RED_FLAG_CALC * DAYS_PER_WEEK
#define PERCENTAGE_DIVISOR          100
#define MAX_RETURNED_REASON_CODES   8

typedef enum {
    WATERPAD_PROCESSING,
    MAGNETOMETER_PROCESSING,
}algoNestProcessingStates_t;

static padWindows_t padWindow;
static magWindows_t magWindow;
static padSample_t currentPadSample;
static padSample_t lastPadSample;
static magSample_t magSample;
static waterAlgoData_t waterAlgoData;
static waterCalibration_t waterCalibration;
static padFilteringData_t padFilterData;
static strokeTransitionInfo_t transitionInfo;
static strokeTransitionBuffer_t transitionBuffer;
static strokeBuffer_t strokeBuffer;
static strokeDetectInfo_t strokeInfo;
static magCalibration_t magCalibration;
static bool runStrokeDetection = false; // set to true when activated either on power up or when we hit 50 liters
static uint16_t redFlagOnThreshold = 0;
static uint16_t redFlagOffThreshold = 0;
static algoNestProcessingStates_t state;
static hourlyStrokeInfo_t hourlyStrokeInfo;
static accumStrokeCount_t strokeCount;
static uint32_t algoErrorBits = 0u;
static uint32_t tempErrorBits = 0u;
static pumpUsage_t pumpUsage;
static hourlyPumpHealthInfo_t pumpHealth;
static hourlyWaterInfo_t hourlyWaterInfo;

//we use this array for average liter & red flag calculations. Once it is filled it will look something like this:
/* ( use mod 7 to split the array up into weeks)
 * Sun  Mon  Tues Wed  Thur Fri  Sat
   3276 2654 3356 3275 3176 2985 2871
   3158 2713 3245 3182 3008 3060 2994
   3314 2694 3158 3074 3157 2956 3012
   3214 2718 3215 3013 3089 3021 2852
 *
 */
static uint16_t dailyLitersOverPastMonth[DAYS_FOR_RED_FLAG_CALC] = {};
static bool isRedFlagConditionSet = false;
static uint8_t dailyLiterIdx = 0u;
static bool oneMonthOfDataPresent = false;
static uint8_t pumpHealthHr = 0;

static void xGetLatestSamples(bool activeSampling);
static void xWaterpadProcess(void);
static void xMagnetometerProcess(void);
static void xHandleError(ReasonCodes reason);
static void xReportAlgoErrors(APP_NVM_SENSOR_DATA_T *sensorData);
static void xCalcDailyLiters(APP_NVM_SENSOR_DATA_T *sensorData);
static void xCalcAvgLitersAndCheckForBreakdown(APP_NVM_SENSOR_DATA_T *sensorData);
static void xUpdateTotalLiters(APP_NVM_SENSOR_DATA_T *sensorData);

void APP_ALGO_Init(void)
{
    initializeWindows( &padWindow, &magWindow );
    initializeWaterAlgorithm( &waterAlgoData, &waterCalibration, &padFilterData, &pumpUsage );
    initializeStrokeAlgorithm( &transitionInfo, &strokeInfo, &strokeCount );
    initializeMagCalibration( &magCalibration );
    state = WATERPAD_PROCESSING;
}

void APP_ALGO_wakeUpInit(void)
{
    wakeupDataReset( &padWindow, &magWindow, &waterAlgoData, &padFilterData );
}

void APP_ALGO_Nest(bool activeSampling)
{
    xGetLatestSamples(activeSampling);

    waterPadFiltering( &currentPadSample, &padFilterData, &currentPadSample );
    writePadSample( &padWindow,  &currentPadSample );

    if (runStrokeDetection)
    {
        writeMagSample( &magWindow, &magSample );
    }

    switch (state)
    {
        case WATERPAD_PROCESSING:
            if (padWindow.process)
            {
                xWaterpadProcess();

                if (runStrokeDetection)
                {
                    state = MAGNETOMETER_PROCESSING;
                }
            }
            break;

        case MAGNETOMETER_PROCESSING:
            if (magWindow.process)
            {
                xMagnetometerProcess();
                state = WATERPAD_PROCESSING;
            }
            break;

        default:
            break;
    }
}

void APP_ALGO_setStrokeDetectionIsOn(bool algIsOn)
{
    runStrokeDetection = algIsOn;

    if ( algIsOn == true )
    {
        HW_TERM_Print("enabling stroke detection");
    }
    else
    {
        HW_TERM_Print("disabling stroke detection");

        //reset nest state
        state = WATERPAD_PROCESSING;
    }
}


void APP_ALGO_initRedFlagThresholds(uint16_t onThreshold, uint16_t offThreshold)
{
    redFlagOnThreshold = onThreshold;
    redFlagOffThreshold = offThreshold;
}

void APP_ALGO_resetRedFlagData(void)
{
    //set the data array to 0
    memset(&dailyLitersOverPastMonth, 0, sizeof(dailyLitersOverPastMonth));

    //set red flag state
    isRedFlagConditionSet = false;

    //we dont have 1 month of data
    oneMonthOfDataPresent = false;

    //next day is day 0
    dailyLiterIdx = 0;
}

bool APP_ALGO_isWaterPresent(void)
{
    return waterAlgoData.present;
}

bool APP_ALGO_isMagnetPresent(void)
{
    return magCalibration.magnet_present;
}

void APP_ALGO_computePumpHealth(uint8_t hour)
{
    computePumpHealth(&hourlyWaterInfo, &hourlyStrokeInfo, &pumpHealth);
    pumpHealthHr = hour;
}

uint32_t APP_ALGO_monitorTotalLiters(void)
{
    return (uint32_t)waterAlgoData.water_volume_sum;
}

void APP_ALGO_updateHourlyFields(APP_NVM_SENSOR_DATA_T *sensorData, uint8_t hoursIdx)
{
    ReasonCodes reasonCode;

    //compute hourly water volume
    hourlyWaterVolume( &waterAlgoData, &pumpUsage, hoursIdx, (dailyLiterIdx % DAYS_PER_WEEK), &reasonCode, &hourlyWaterInfo);

    //check reason code
    if ( reasonCode != reason_code_none )
    {
        xHandleError(reasonCode);
    }

    //put liters into sensor data payload
    sensorData->litersPerHour[hoursIdx] = (uint16_t)hourlyWaterInfo.volume;

    APP_ALGO_calculateHourlyStrokes();
    sensorData->strokesPerHour[hoursIdx] = hourlyStrokeInfo.combined_stroke_count;
    sensorData->strokeHeightPerHour[hoursIdx] = hourlyStrokeInfo.c_combined_stroke_avg_displacem;
}

void APP_ALGO_updateDailyFields(APP_NVM_SENSOR_DATA_T *sensorData)
{
    xCalcDailyLiters( sensorData );
    xCalcAvgLitersAndCheckForBreakdown( sensorData );
    xUpdateTotalLiters( sensorData );
    xReportAlgoErrors( sensorData );

    //set pump capacity and magnet present flags
    sensorData->pumpCapacity = (uint16_t)pumpHealth.pump_capacity;
    sensorData->pumpUsage = (uint16_t)pumpHealth.quality_factor;

    //TODO take this out, just for testing.
    sensorData->dryStrokes = pumpHealthHr;
    sensorData->magnetDetected = APP_ALGO_isMagnetPresent();
}

//CLI function
uint16_t APP_ALGO_getHourlyWaterVolume(void)
{
    ReasonCodes reasonCode;

    hourlyWaterVolume( &waterAlgoData, &pumpUsage, 0, 0, &reasonCode, &hourlyWaterInfo);

    return (uint16_t)hourlyWaterInfo.volume;
}

void APP_ALGO_calculateHourlyStrokes(void)
{
    hourlyStrokeCount( &strokeCount, &hourlyStrokeInfo );
}

uint8_t APP_ALGO_getMaxHourUsage(void)
{
    //based on the percent of time used
    return getMaxUsageTime(&pumpUsage, (dailyLiterIdx % DAYS_PER_WEEK));
}

uint16_t APP_ALGO_getHourlyStrokeCount(void)
{
    return hourlyStrokeInfo.combined_stroke_count;
}

uint16_t APP_ALGO_getHourlyDisplacement(void)
{
    return hourlyStrokeInfo.c_combined_stroke_avg_displacem;
}

uint16_t APP_ALGO_getMagWindowsProcessed(void)
{
    return hourlyStrokeInfo.windows_processed;
}

void APP_ALGO_resetHourlyStrokeCount(void)
{
    cliResetStrokeCount( &strokeCount );
}

static void xGetLatestSamples(bool activeSampling)
{
    static uint8_t staticDataCount = 0;

    // Pad Samples - Notice that pad X does NOT correspond with channel X
    // This is intentional and related to the way that the cap sense peripheral is initialized/sampling order.
    currentPadSample.pad1 = APP_WTR_GetPadValue( CAPTIVATE_RX_CHANNEL2 );
    currentPadSample.pad2 = APP_WTR_GetPadValue( CAPTIVATE_RX_CHANNEL1 );
    currentPadSample.pad3 = APP_WTR_GetPadValue( CAPTIVATE_RX_CHANNEL6 );
    currentPadSample.pad4 = APP_WTR_GetPadValue( CAPTIVATE_RX_CHANNEL5 );
    currentPadSample.pad5 = APP_WTR_GetPadValue( CAPTIVATE_RX_CHANNEL4 );
    currentPadSample.pad6 = APP_WTR_GetPadValue( CAPTIVATE_RX_CHANNEL8 );
    currentPadSample.pad7 = APP_WTR_GetPadValue( CAPTIVATE_RX_CHANNEL3 );
    currentPadSample.pad8 = APP_WTR_GetPadValue( CAPTIVATE_RX_CHANNEL7 );

    if ( activeSampling == true )
    {
        //compare this sample to the last one to check for static data
        if ( memcmp(&currentPadSample, &lastPadSample, sizeof(currentPadSample)) == 0 )
        {
            staticDataCount++;

            //if we have too many missed samples, flag the issue
            if ( staticDataCount > STATIC_SAMPLE_COUNT_MAX )
            {
                HW_TERM_Print("\n\rSTATIC CAP SENSE DATA\n\r");

                APP_indicateError(CAP_SENSE_NO_DATA);
                staticDataCount = 0;
            }
        }
        else
        {
            staticDataCount = 0;

            //if there had previously been an error, clear it because it has resolved
            if ( (APP_getErrorBits() & CAP_SENSE_NO_DATA) == CAP_SENSE_NO_DATA )
            {
                APP_indicateErrorResolved(CAP_SENSE_NO_DATA);
            }
        }

        lastPadSample = currentPadSample;
    }
    else
    {
        staticDataCount = 0;
    }

    // Mag Samples
    HW_MAG_GetLatestMagAndTempData( &magSample.x_lsb, &magSample.y_lsb, &magSample.z_lsb, &magSample.temp_lsb, &magSample.status );
}

static void xWaterpadProcess(void)
{
    int8_t i = 0;
    ReasonCodes reasonCodes[MAX_RETURNED_REASON_CODES];

    calculateWaterVolume( &waterAlgoData, &waterCalibration, &padWindow, reasonCodes );

    for (i = 0; i< MAX_RETURNED_REASON_CODES; i++)
    {
        if ( reasonCodes[i] != reason_code_none)
        {
            xHandleError(reasonCodes[i]);
        }
    }

    clearPadWindowProcess( &padWindow );
}

static void xMagnetometerProcess(void)
{
    int8_t i = 0;

    //buffer of reason codes for calibration
    ReasonCodes reasonCodes[MAX_RETURNED_REASON_CODES];

    //singular reason code
    ReasonCodes reason;

    magnetometerCalibration( &magWindow, &magCalibration, &waterAlgoData, reasonCodes );

    for (i = 0; i< MAX_RETURNED_REASON_CODES; i++)
    {
        if ( reasonCodes[i] != reason_code_none)
        {
            xHandleError(reasonCodes[i]);
        }
    }

    reason = detectTransitions( &magWindow, &magCalibration, &transitionBuffer, &transitionInfo );

    if ( reason != reason_code_none)
    {
        xHandleError(reason);
    }

    reason = detectStrokes( &transitionBuffer, &strokeBuffer, &strokeInfo, &strokeCount, &magCalibration, & waterAlgoData );

    if ( reason != reason_code_none)
    {
        xHandleError(reason);
    }

    clearMagWindowProcess( &magWindow );
}

//Map reason code to an error bit to include in the sensor data log for this day
static void xHandleError(ReasonCodes reason)
{
    tempErrorBits = 0;
    switch (reason)
    {
        case mag_calib_present_reset:
            tempErrorBits |= CALIB_PRESENT_RESET;
            break;

        case mag_calib_orientation_reset:
            tempErrorBits |= CALIB_ORIENT_RESET;
            break;

        case mag_calib_major_change_reset:
            tempErrorBits |= CALIB_MAJOR_CHANGE_RESET;
            break;

        case c_mag_calib_orientation_calibra:
            tempErrorBits |= ORIENTATION_CALIB;
            break;

        case mag_calib_offset_calibrated:
            tempErrorBits |= OFFSET_CALIB;
            break;

        case mag_calib_magnet_present:
            tempErrorBits |= MAGNET_PRESENT;
            break;

        case stroke_trans_buffer_overflow:
            tempErrorBits |= TRANS_BUFFER_OVERFLOW;
            break;

        case stroke_buffer_overflow:
            tempErrorBits |= STROKE_BUFFER_OVERFLOW;
            break;

        case water_calib_calibrated:
            tempErrorBits |= WATER_CALIB;
            break;

        case water_calib_reset:
            tempErrorBits |= WATER_CALIB_RESET;
            break;

        case water_calib_neg_delta:
            tempErrorBits |= WATER_CALIB_NEG;
            break;

        case water_bad_sample:
            tempErrorBits |= WATER_BAD_SAMPLE;
            break;

        case water_flow_standing_water:
            tempErrorBits |= WATER_STANDING;
            break;

        case mag_calib_xy_cnt_low:
            tempErrorBits |= CALIB_XY_CNT_LOW;
            break;

        case mag_calib_xz_cnt_low:
            tempErrorBits |= CALIB_XZ_CNT_LOW;
            break;

        case mag_calib_bad_placement_wobble:
            tempErrorBits |= CALIB_BAD_PLACEMENT;
            break;

        case mag_calib_new_offset_value_l1:
            tempErrorBits |= CALIB_NEW_OFFSET_VAL_1;
            break;
    }

    algoErrorBits |= tempErrorBits;
}


static void xReportAlgoErrors(APP_NVM_SENSOR_DATA_T *sensorData)
{
    sensorData->errorBits |= ( (uint32_t)algoErrorBits << ALGO_ERROR_OFFSET);
    algoErrorBits = 0;
}

static void xCalcDailyLiters(APP_NVM_SENSOR_DATA_T *sensorData)
{
    uint8_t i;
    sensorData->dailyLiters = 0;

    for (i = 0; i < APP_NVM_SAMPLES_PER_DAY; i++)
    {
        sensorData->dailyLiters += sensorData->litersPerHour[i];
    }
}

static void xCalcAvgLitersAndCheckForBreakdown(APP_NVM_SENSOR_DATA_T *sensorData)
{
    uint8_t idx = 0;
    uint32_t dailyLitersSum = 0;
    uint32_t avgForThisdayOverPastMonth = 0u;

    //if we have 4 weeks worth of data, compute average for this day, look for red flag
    if ( oneMonthOfDataPresent == true )
    {
        //calculate the average for this day of the week over the past 4 weeks, start on value of 0 - 6
        for ( idx = (dailyLiterIdx % DAYS_PER_WEEK); idx < DAYS_FOR_RED_FLAG_CALC; idx += DAYS_PER_WEEK )
        {
            dailyLitersSum += dailyLitersOverPastMonth[idx];
        }

        //get average
        avgForThisdayOverPastMonth = (dailyLitersSum/WEEKS_FOR_RED_FLAG_CALC);

        //update the sensor data field with the average
        sensorData->avgLiters = avgForThisdayOverPastMonth;

        //check for red flag, which is when todays liters are less than X% of the
        //average for this day:
        if ( sensorData->dailyLiters < ((avgForThisdayOverPastMonth * redFlagOnThreshold)/PERCENTAGE_DIVISOR) )
        {
            isRedFlagConditionSet = true;
            HW_TERM_Print("red flag true");
        }
        else
        {
            //check if red flag was previously set:
            if ( isRedFlagConditionSet == true )
            {
                //check if recovered, which is when todays liters are back to X% of the average for this day
                if ( sensorData->dailyLiters >= ((avgForThisdayOverPastMonth * redFlagOffThreshold)/PERCENTAGE_DIVISOR) )
                {
                    //reset red flag
                    isRedFlagConditionSet = false;

                    HW_TERM_Print("red flag cleared");

                    //add the value to the map
                    dailyLitersOverPastMonth[dailyLiterIdx] = sensorData->dailyLiters;
                }
            }
            else
            {
                //add the value to the map
                dailyLitersOverPastMonth[dailyLiterIdx] = sensorData->dailyLiters;
            }
        }
    }
    else
    {
        //just add the value to the map since we dont have a full 4 weeks worth of data yet
        dailyLitersOverPastMonth[dailyLiterIdx] = sensorData->dailyLiters;

        //set average to 0
        sensorData->avgLiters = 0;
    }

    //set sensor data breakdown flag to match the red flag state
    sensorData->breakdown = isRedFlagConditionSet;

    //increment the daily liter counter and wrap around if the end of the month
    dailyLiterIdx++;
    if ( dailyLiterIdx >= DAYS_FOR_RED_FLAG_CALC)
    {
        dailyLiterIdx = 0u;
        oneMonthOfDataPresent = true;
    }
}

// To use this function:
// set RTC time to 3 minutes before midnight (make sure device is activated)
// call this function from CLI
// At midnight, a red flag will be generated since liters will be 0 for the given day
void APP_ALGO_populateRedFlagArrayWithFakeData(void)
{
    int i = 0;

    //manipulate the data map and associated variables
    for(i = 0; i< DAYS_FOR_RED_FLAG_CALC; i++)
    {
       dailyLitersOverPastMonth[i] = 3000 + (i*2);
    }

    dailyLiterIdx = 0;
    oneMonthOfDataPresent = true;
}

static void xUpdateTotalLiters(APP_NVM_SENSOR_DATA_T *sensorData)
{
    sensorData->totalLiters = APP_NVM_Custom_GetTotalLiters() + sensorData->dailyLiters;
    APP_NVM_Custom_WriteTotalLiters(sensorData->totalLiters);
}
