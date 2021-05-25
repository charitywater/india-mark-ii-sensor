/*
================================================================================================#=
Module:   SPI Driver

Description:
    SPI API w/ CS control

Copyright 2021 Twisthink LLC
This code is licensed under Twisthink license.
See Twisthink-SoftwareLicenseAgreement-Auris.docx for details.
================================================================================================#=
*/

#ifndef PERIPHERAL_DRIVERS_SPI_H_
#define PERIPHERAL_DRIVERS_SPI_H_

/* chip select definitions */
#define MT28F1_CS_PORT      GPIOA
#define MT28F1_CS_PIN       GPIO_PIN_4

#define SSM_RDY_PORT        GPIOB
#define SSM_RDY_PIN         GPIO_PIN_12

#define GPIO_LOW            0
#define GPIO_HIGH           1

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


extern void SPI_Init(void);
extern void SPI_DeInit(void);

#endif /* PERIPHERAL_DRIVERS_SPI_H_ */
