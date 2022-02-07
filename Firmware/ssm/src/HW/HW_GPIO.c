/**************************************************************************************************
* \file     HW_GPIO.c
* \brief    GPIO (General purpose input/output)
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

#include "HW.h"
#include "HW_GPIO.h"
#include "HW_RTC.h"
#include "HW_MAG.h"
#include "HW_ENV.h"
#include <msp430.h>                      // Generic MSP430 Device Include
#include "driverlib.h"                   // MSPWare Driver Library

bool HW_GPIO_Read_WP_EEPRM(void);
void HW_GPIO_Set_WP_EEPRM(void);
void HW_GPIO_Clear_WP_EEPRM(void);

bool HW_GPIO_Read_SSM_BOOT(void);

bool HW_GPIO_Read_SYS_OFF(void);
void HW_GPIO_Set_SYS_OFF(void);
void HW_GPIO_Clear_SYS_OFF(void);

bool HW_GPIO_Read_WAKE_AP(void);
void HW_GPIO_Set_WAKE_AP(void);
void HW_GPIO_Clear_WAKE_AP(void);

void HW_GPIO_Clear_SSM_RDY(void);
void HW_GPIO_Set_SSM_RDY(void);

bool HW_GPIO_Read_FUEL_GAUGE_DATA(void);
void HW_GPIO_Set_FUEL_GAUGE_DATA(void);
void HW_GPIO_Clear_FUEL_GAUGE_DATA(void);

bool HW_GPIO_Read_INT_TEMP(void);
bool HW_GPIO_Read_INT_MAG(void);

#define HW_GPIO_WP_EEPRM_BIT            BIT_1
#define HW_GPIO_WP_EEPRM_PORT           P4OUT

#define HW_GPIO_SSM_BOOT_BIT            BIT_1
#define HW_GPIO_SSM_BOOT_PORT           P1IN

#define HW_GPIO_WAKE_AP_BIT             BIT_6
#define HW_GPIO_WAKE_AP_PORT            P1OUT

#define HW_GPIO_WD_KICK_BIT             BIT_7
#define HW_GPIO_WD_KICK_PORT            P1OUT

#define HW_GPIO_SYSTEM_OFF_BIT          BIT_0
#define HW_GPIO_SYSTEM_OFF_PORT         P2OUT

#define HW_GPIO_FUEL_GAUGE_DATA_BIT     BIT_2
#define HW_GPIO_FUEL_GAUGE_DATA_PORT    P2OUT

#define HW_GPIO_INT_TEMP_BIT            BIT_3
#define HW_GPIO_INT_TEMP_PORT           P2IN

#define HW_GPIO_INT_MAG_BIT             BIT_7
#define HW_GPIO_INT_MAG_PORT            P2IN

#define HW_CS_SSM_RDY_BIT               BIT_1
#define HW_CS_SSM_RDY_PORT              P3OUT

#pragma PERSISTENT(DiscreteDesc)
hw_gpio_discrete_t DiscreteDesc[HW_GPIO_NUM_DISCRETES] = {
   {
      "1.1",                                // gpio_num_str
       "SSM_BOOT",                          // signal_name
       HW_GPIO_INPUT,                       // io
       HW_GPIO_Read_SSM_BOOT,               // read_func
       NULL,                                // set_func
       NULL                                 // clear_func
   },
   {
       "1.6",                               // gpio_num_str
       "WAKE_AP",                           // signal_name
       HW_GPIO_OUTPUT,                      // io
       HW_GPIO_Read_WAKE_AP,                // read_func
       HW_GPIO_Set_WAKE_AP,                 // set_func
       HW_GPIO_Clear_WAKE_AP                // clear_func
   },
   {
       "2.0",                                // gpio_num_str
       "SYS_OFF",                            // signal_name
       HW_GPIO_OUTPUT,                       // io
       HW_GPIO_Read_SYS_OFF,                 // read_func
       HW_GPIO_Set_SYS_OFF,                  // set_func
       HW_GPIO_Clear_SYS_OFF                 // clear_func
   },
   {
      "2.2",                                 // gpio_num_str
       "FUEL_DATA",                          // signal_name
       HW_GPIO_OUTPUT,                       // io
       HW_GPIO_Read_FUEL_GAUGE_DATA,         // read_func
       HW_GPIO_Set_FUEL_GAUGE_DATA,          // set_func
       HW_GPIO_Clear_FUEL_GAUGE_DATA         // clear_func
   },
   {
      "2.3",                                // gpio_num_str
       "INT_TEMP",                          // signal_name
       HW_GPIO_INPUT,                       // io
       HW_GPIO_Read_INT_TEMP,               // read_func
       NULL,                                // set_func
       NULL                                 // clear_func
   },
   {
      "2.7",                                // gpio_num_str
       "INT_MAG",                           // signal_name
       HW_GPIO_INPUT,                       // io
       HW_GPIO_Read_INT_MAG,                // read_func
       NULL,                                // set_func
       NULL                                 // clear_func
   },
   {
      "3.1",                                // gpio_num_str
       "CS",                                // signal_name
       HW_GPIO_OUTPUT,                      // io
       NULL,                                // read_func
       HW_GPIO_Set_SSM_RDY,                 // set_func
       HW_GPIO_Clear_SSM_RDY                // clear_func
   },
   {
      "4.1",                                // gpio_num_str
       "WP_EEPRM",                          // signal_name
       HW_GPIO_OUTPUT,                      // io
       HW_GPIO_Read_WP_EEPRM,               // read_func
       HW_GPIO_Set_WP_EEPRM,                // set_func
       HW_GPIO_Clear_WP_EEPRM               // clear_func
   },
};

void HW_GPIO_Init(void);

void HW_GPIO_Init(void)
{

}

void HW_GPIO_Set_SYSTEM_SHUTDOWN(void)
{
    P2OUT |= BIT_0;
}
void HW_GPIO_Clear_SYSTEM_SHUTDOWN(void)
{
    P2OUT &= (~BIT_0);
}

void HW_GPIO_Set_WD(void)
{
    HW_GPIO_WD_KICK_PORT |= HW_GPIO_WD_KICK_BIT;
}
void HW_GPIO_Clear_WD(void)
{
    HW_GPIO_WD_KICK_PORT &= (~HW_GPIO_WD_KICK_BIT);
}

bool HW_GPIO_Read_SYSTEM_SHUTDOWN(void)
{
    return ((P2OUT & BIT_0) == BIT_0);
}

bool HW_GPIO_Read_WP_EEPRM(void)
{
    return ((HW_GPIO_WP_EEPRM_PORT & HW_GPIO_WP_EEPRM_BIT) == HW_GPIO_WP_EEPRM_BIT);
}
void HW_GPIO_Set_WP_EEPRM(void)
{
   (HW_GPIO_WP_EEPRM_PORT |= HW_GPIO_WP_EEPRM_BIT);
}
void HW_GPIO_Clear_WP_EEPRM(void)
{
   (HW_GPIO_WP_EEPRM_PORT &= (~HW_GPIO_WP_EEPRM_BIT));
}

bool HW_GPIO_Read_SYS_OFF(void)
{
    return ((HW_GPIO_SYSTEM_OFF_PORT & HW_GPIO_SYSTEM_OFF_BIT) == HW_GPIO_SYSTEM_OFF_BIT);
}

void HW_GPIO_Set_SYS_OFF(void)
{
    //only set P2.0 as an OUTPUT when we are going to use it
    //to prevent unwanted system resets
    P2DIR = HW_GPIO_SYSTEM_OFF_BIT;
    (HW_GPIO_SYSTEM_OFF_PORT |= HW_GPIO_SYSTEM_OFF_BIT);
}

void HW_GPIO_Clear_SYS_OFF(void)
{
    //only set P2.0 as an OUTPUT when we are going to use it
    //to prevent unwanted system resets
    P2DIR = HW_GPIO_SYSTEM_OFF_BIT;
    (HW_GPIO_SYSTEM_OFF_PORT &= (~HW_GPIO_SYSTEM_OFF_BIT));
}

bool HW_GPIO_Read_SSM_BOOT(void)
{
    return ((HW_GPIO_SSM_BOOT_PORT & HW_GPIO_SSM_BOOT_BIT) == HW_GPIO_SSM_BOOT_BIT);
}

bool HW_GPIO_Read_WAKE_AP(void)
{
    return ((HW_GPIO_WAKE_AP_PORT & HW_GPIO_WAKE_AP_BIT) == HW_GPIO_WAKE_AP_BIT);
}

void HW_GPIO_Set_WAKE_AP(void)
{
    (HW_GPIO_WAKE_AP_PORT |= HW_GPIO_WAKE_AP_BIT);
}
void HW_GPIO_Clear_WAKE_AP(void)
{
    (HW_GPIO_WAKE_AP_PORT &= (~HW_GPIO_WAKE_AP_BIT));
}

void HW_GPIO_Set_SSM_RDY(void)
{
    (HW_CS_SSM_RDY_PORT |= HW_CS_SSM_RDY_BIT);
}

void HW_GPIO_Clear_SSM_RDY(void)
{
    (HW_CS_SSM_RDY_PORT &= (~HW_CS_SSM_RDY_BIT));
}

void HW_GPIO_Set_FUEL_GAUGE_DATA(void)
{
    (HW_GPIO_FUEL_GAUGE_DATA_PORT |= HW_GPIO_FUEL_GAUGE_DATA_BIT);
}

void HW_GPIO_Clear_FUEL_GAUGE_DATA(void)
{
    (HW_GPIO_FUEL_GAUGE_DATA_PORT &= (~HW_GPIO_FUEL_GAUGE_DATA_BIT));
}

bool HW_GPIO_Read_FUEL_GAUGE_DATA(void)
{
    return ((HW_GPIO_FUEL_GAUGE_DATA_PORT & HW_GPIO_FUEL_GAUGE_DATA_BIT) == HW_GPIO_FUEL_GAUGE_DATA_BIT);
}

bool HW_GPIO_Read_INT_TEMP(void)
{
    return ((HW_GPIO_INT_TEMP_PORT & HW_GPIO_INT_TEMP_BIT) == HW_GPIO_INT_TEMP_BIT);
}

bool HW_GPIO_Read_INT_MAG(void)
{
    return ((HW_GPIO_INT_MAG_PORT & HW_GPIO_INT_MAG_BIT) == HW_GPIO_INT_MAG_BIT);
}

//Interrupts:

#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=PORT1_VECTOR
__interrupt
#elif defined(__GNUC__)
__attribute__((interrupt(PORT1_VECTOR)))
#endif
void P1_ISR(void)
{
    // Pin7 !IRQ_RTC
    if (P1IFG & GPIO_PIN7)
    {
        GPIO_clearInterrupt(GPIO_PORT_P1, GPIO_PIN7);
    }
	__bic_SR_register_on_exit(LPM3_bits);
}

#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=PORT2_VECTOR
__interrupt
#elif defined(__GNUC__)
__attribute__((interrupt(PORT2_VECTOR)))
#endif
void P2_ISR(void)
{
    //check which pin caused the interrupt
    if (P2IFG & GPIO_PIN3)
    {
        GPIO_clearInterrupt(GPIO_PORT_P2, GPIO_PIN3);

        HW_ENV_DataRdyInt();

    }
    if (P2IFG & GPIO_PIN7)
    {
        GPIO_clearInterrupt(GPIO_PORT_P2, GPIO_PIN7);

        HW_Mag_DataReadyIntOccured();
    }
    __bic_SR_register_on_exit(LPM3_bits);
}
