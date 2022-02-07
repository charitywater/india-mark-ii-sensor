/**************************************************************************************************
* \file     stm_i2c_ata.h
* \brief    STM32 implementation of hardware abstraction layer
*
* \par      Copyright Notice
*           Copyright(C) 2021 by charity: water
*           
*           All rights reserved. No part of this software may be disclosed or
*           distributed in any form or by any means without the prior written
*           consent of charity: water.
* \date     2/1/2021
* \author   Twisthink
*
***************************************************************************************************/

#ifndef THIRD_PARTY_MICROCHIP_LIB_HAL_STM_I2C_ATA_H_
#define THIRD_PARTY_MICROCHIP_LIB_HAL_STM_I2C_ATA_H_

/**
 * \defgroup hal_ Hardware abstraction layer (hal_)
 *
 * \brief These methods define the hardware abstraction layer for
 *        communicating with a CryptoAuth device using I2C bit banging.
   @{ */

/**
 * \brief This enumeration lists flags for I2C read or write addressing.
 */
enum i2c_read_write_flag
{
    I2C_WRITE = (uint8_t)0x00,  //!< write command flag
    I2C_READ  = (uint8_t)0x01   //!< read command flag
};

typedef struct atcaI2Cmaster
{
    uint8_t  i2c_master_instance;
    // for conveniences during interface release phase
    int bus_index;
} ATCAI2CMaster_t;

typedef struct _i2c_m_msg
{
    uint16_t addr;
    uint16_t len;
    uint8_t* buffer;
    uint8_t flags;
}_i2c_m_msg_t;

#endif /* THIRD_PARTY_MICROCHIP_LIB_HAL_STM_I2C_ATA_H_ */
