/**************************************************************************************************
* \file     spi.h
* \brief    SPI API w/ CS control
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
* \date     2/1/2021
* \author   Twisthink
*
***************************************************************************************************/

#ifndef PERIPHERAL_DRIVERS_SPI_H_
#define PERIPHERAL_DRIVERS_SPI_H_

/* chip select definitions */
#define MT28F1_CS_PORT          GPIOA
#define MT28F1_CS_PIN           GPIO_PIN_4

#define SSM_RDY_PORT            GPIOB
#define SSM_RDY_PIN             GPIO_PIN_12

#define GPIO_LOW                0
#define GPIO_HIGH               1

typedef enum
{
    spiError,
    spiSuccess,
    spiTimeout,
}spiStatus_t;

/* Acceptable values for SPI master side configuration */
typedef enum _SpiConfigOptions
{
    OpsNull,            // do nothing
    OpsInitTransfer,
    OpsEndTransfer,
}spiConfigOptions_t;


/* char stream definition for */
typedef struct _structCharStream
{
    uint8_t* pChar;  /* buffer address that holds the streams */
    uint32_t length; /* length of the stream in bytes */
}spiData_t;

extern spiStatus_t SPI_nandTransfer(const spiData_t* pDataToSend,
               spiData_t* pDataReceived,
               spiConfigOptions_t optAfter);

extern spiStatus_t SPI_ssmTransfer(const spiData_t* pDataToSend, spiData_t* pDataReceived,
        spiConfigOptions_t optAfter);

extern void SPI_Init(void);

#endif /* PERIPHERAL_DRIVERS_SPI_H_ */
