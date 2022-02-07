/**************************************************************************************************
* \file     uC_ADC.h
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

#ifndef UC_INC_UC_ADC_H_
#define UC_INC_UC_ADC_H_

extern void uC_ADC_Init(void);
extern uint16_t uC_ADC_newConversionCh0(void);

#endif /* UC_INC_UC_ADC_H_ */
