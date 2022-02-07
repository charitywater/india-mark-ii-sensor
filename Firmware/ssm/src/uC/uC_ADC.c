/**************************************************************************************************
* \file     uC_ADC.c
* \brief    ADC peripheral functionality
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
#include "driverlib.h"
#include <stdint.h>
#include "stdbool.h"
#include "string.h"
#include "stdio.h"
#include "uC_ADC.h"

#define REF_VOLTAGE_MV      2500

// 2^12 - 1
#define ADC_12_BIT_MAX      (4096 - 1)

static uint16_t xCh0AdcReading = 0u;

void uC_ADC_Init(void)
{
    //init ADC channel 0 (p 1.0) with 12 bit resolution
    ADC_init (ADC_BASE, ADC_SAMPLEHOLDSOURCE_SC, ADC_CLOCKSOURCE_ADCOSC, ADC_CLOCKDIVIDER_1);
    ADC_enable(ADC_BASE);
    ADC_setupSamplingTimer(ADC_BASE,ADC_CYCLEHOLD_8_CYCLES,ADC_MULTIPLESAMPLESDISABLE);
    ADC_configureMemory(ADC_BASE, ADC_INPUT_A0, ADC_VREFPOS_INT, ADC_VREFNEG_AVSS);
    ADC_setResolution(ADC_BASE, ADC_RESOLUTION_12BIT);

    //use 2.5 V ref voltage instead of default 1.5V
    PMMCTL2 |=REFVSEL_2;
}

uint16_t uC_ADC_newConversionCh0(void)
{
    uint32_t rawVal = 0;
    ADC_startConversion (ADC_BASE, ADC_SINGLECHANNEL);

    while (!(ADC_getInterruptStatus(ADC_BASE,ADC_COMPLETED_INTERRUPT_FLAG))) ;

    rawVal = ADC_getResults(ADC_BASE);

    ADC_clearInterrupt(ADC_BASE, ADC_COMPLETED_INTERRUPT_FLAG);

    xCh0AdcReading = (rawVal * REF_VOLTAGE_MV)/ADC_12_BIT_MAX;

    //return the voltage in mV - does not apply any voltage divider math
    return xCh0AdcReading;
}
