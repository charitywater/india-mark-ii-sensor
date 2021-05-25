/**************************************************************************************************
* \file     HW_GPIO.h
* \brief    GPIO (General purpose input/output).
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

#ifndef HW_GPIO_H
#define HW_GPIO_H

#include "HW.h"

#define HW_GPIO_NUM_DISCRETES 8

typedef enum hw_gpio{
    HW_GPIO_INPUT = 0,      // Signal is an input
    HW_GPIO_OUTPUT = 1      // Signal is an output
} hw_gpio_t;

typedef bool (*hw_gpio_discrete_read_t)(void);
typedef void (*hw_gpio_discrete_set_t)(void);
typedef void (*hw_gpio_discrete_clear_t)(void);


typedef struct hw_gpio_discrete
{
    char * gpio_num_str;                   // string indicating the gpio number
    char * signal_name;                    // string indicating the signal name
    hw_gpio_t io;                          // input or output
    hw_gpio_discrete_read_t read_func;     // pointer to accessor function to read the discrete
    hw_gpio_discrete_set_t set_func;       // pointer to accessor function to set the discrete
    hw_gpio_discrete_clear_t clear_func;   // pointer to accessor function to clear the discrete
}hw_gpio_discrete_t;


extern void HW_GPIO_Init(void);

extern hw_gpio_discrete_t DiscreteDesc[HW_GPIO_NUM_DISCRETES];

extern bool HW_GPIO_Read_WP_EEPRM(void);
extern void HW_GPIO_Set_WP_EEPRM(void);
extern void HW_GPIO_Clear_WP_EEPRM(void);

extern bool HW_GPIO_Read_SSM_BOOT(void);

extern bool HW_GPIO_Read_SYS_OFF(void);
extern void HW_GPIO_Set_SYS_OFF(void);
extern void HW_GPIO_Clear_SYS_OFF(void);

extern bool HW_GPIO_Read_WAKE_AP(void);
extern void HW_GPIO_Set_WAKE_AP(void);
extern void HW_GPIO_Clear_WAKE_AP(void);

extern void HW_GPIO_Clear_SSM_RDY(void);
extern void HW_GPIO_Set_SSM_RDY(void);


extern bool HW_GPIO_Read_FUEL_GAUGE_DATA(void);

extern bool HW_GPIO_Read_INT_TEMP(void);

extern bool HW_GPIO_Read_INT_MAG(void);

//adding system shutdown control pin
extern void HW_GPIO_Set_SYSTEM_SHUTDOWN(void);
extern void HW_GPIO_Clear_SYSTEM_SHUTDOWN(void);
extern bool HW_GPIO_Read_SYSTEM_SHUTDOWN(void);

extern void HW_GPIO_Set_WD(void);
extern void HW_GPIO_Clear_WD(void);

#endif /* HW_GPIO_H */
