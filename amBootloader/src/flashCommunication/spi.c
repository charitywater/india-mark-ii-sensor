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

#include "stdint.h"
#include "stddef.h"
#include "logTypes.h"
#include <stm32l4xx_hal.h>
#include "string.h"
#include "spi.h"

static SPI_HandleTypeDef hspi1;

static void xEnableNandCommunication(void);
static void xDisableNandCommunication(void);
static void xInitNandSpi(void);
static void xDeInitNandSpi(void);

/* init spi peripherals */
 void SPI_Init(void)
{
    xInitNandSpi();
}


//De init spi peripherals
void SPI_DeInit(void)
{
    xDeInitNandSpi();
}

/*******************************************************************************
Send data to the NAND flash chip over spi. Also receivev data is an rx len and
buffer is provided.
*******************************************************************************/
 spiStatus_t SPI_nandTransfer(const spiData_t* pDataToSend, spiData_t* pDataReceived,
         spiConfigOptions_t optAfter)
{
    uint8_t *dataSend;
    uint8_t *dataRecv;
    uint16_t rxLen = 0;
    uint16_t txLen = 0;
    HAL_StatusTypeDef stat = HAL_ERROR;

    txLen = pDataToSend->length;
    dataSend = pDataToSend->pChar;

    if (NULL != pDataReceived)
    {
        rxLen = pDataReceived->length;
        dataRecv = pDataReceived->pChar;
    }

    xEnableNandCommunication();

    /* Send the spi command/data */
    stat = HAL_SPI_Transmit(&hspi1, dataSend, txLen, 0xffffff );

    if (stat == HAL_OK)
    {
        if (rxLen > 0)
        {
            // now read the actual data
            stat = HAL_SPI_Receive(&hspi1, dataRecv, rxLen, 0xffffff);
        }
    }

    /* only clear cs if it is the end of a transfer */
    if ( optAfter == OpsEndTransfer )
    {
        xDisableNandCommunication();
    }

    /* check the spi code */
    if(stat != HAL_OK)
    {
        return spiError;
    }

    return spiSuccess;
}

static void xDeInitNandSpi(void)
{
    if (HAL_SPI_DeInit(&hspi1) != HAL_OK)
    {
        elogError("FAILED to deinit SPI peripheral in BL");
    }
}

 static void xInitNandSpi(void)
 {
     GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* SPI1 parameter configuration*/
    hspi1.Instance = SPI1;
    hspi1.Init.Mode = SPI_MODE_MASTER;
    hspi1.Init.Direction = SPI_DIRECTION_2LINES;
    hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
    hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
    hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
    hspi1.Init.NSS = SPI_NSS_SOFT;
    hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
    hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
    hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
    hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    hspi1.Init.NSSPMode = SPI_NSS_PULSE_DISABLE;
    if (HAL_SPI_Init(&hspi1) != HAL_OK)
    {
        elogError("FAILED to init SPI peripheral in BL. Check Clock config");
    }

    /* init chip select */
    HAL_GPIO_WritePin(MT28F1_CS_PORT, MT28F1_CS_PIN, GPIO_PIN_SET);

    /*Configure GPIO pin : PA4 */
    GPIO_InitStruct.Pin = MT28F1_CS_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(MT28F1_CS_PORT, &GPIO_InitStruct);
 }


static void xEnableNandCommunication(void)
{
    /* clear cs */
    HAL_GPIO_WritePin(MT28F1_CS_PORT, MT28F1_CS_PIN, GPIO_LOW);
}

static void xDisableNandCommunication(void)
{
    /* set cs */
    HAL_GPIO_WritePin(MT28F1_CS_PORT, MT28F1_CS_PIN, GPIO_HIGH);
}
