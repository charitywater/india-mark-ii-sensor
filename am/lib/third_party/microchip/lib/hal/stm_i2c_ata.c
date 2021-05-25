/**************************************************************************************************
* \file     stm_i2c_ata.c
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

#include <string.h>
#include <stdio.h>
#include "atca_hal.h"
#include "atca_device.h"
#include "atca_execution.h"
#include <stm32l4xx_hal.h>
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "i2c.h"
#include "stm_i2c_ata.h"

#define MAX_I2C_BUSES           2
#define RX_TIMEOUT_TICKS        400
#define I2C_M_SEVEN             7
#define I2C_M_STOP              0
#define I2C_M_RD                0

#define MAX_TIMEOUT_MS          400
#define MUTEX_WAIT_MS           10000

/**
 * \defgroup hal_ Hardware abstraction layer (hal_)
 *
 * \brief These methods define the hardware abstraction layer for
 *        communicating with a CryptoAuth device using I2C bit banging.
   @{ */

/**
 * \brief Logical to physical bus mapping structure.
 */
static ATCAI2CMaster_t i2c_hal_data[MAX_I2C_BUSES];   //!< map logical, 0-based bus number to index



ATCA_STATUS hal_create_mutex(void ** ppMutex, char* pName)
{
    (void)pName;

    if (!ppMutex)
    {
        return ATCA_BAD_PARAM;
    }

    (*ppMutex) = xSemaphoreCreateMutex();

    if (!*ppMutex)
    {
        return ATCA_FUNC_FAIL;
    }

    return ATCA_SUCCESS;
}

ATCA_STATUS hal_destroy_mutex(void * pMutex)
{
    if (!pMutex)
    {
        return ATCA_BAD_PARAM;
    }

    vSemaphoreDelete(pMutex);

    return ATCA_SUCCESS;
}

ATCA_STATUS hal_lock_mutex(void * pMutex)
{
    if (!pMutex)
    {
        return ATCA_BAD_PARAM;
    }

    if (!xSemaphoreTake((SemaphoreHandle_t)pMutex, ( TickType_t )MUTEX_WAIT_MS))
    {
        return ATCA_GEN_FAIL;
    }
    else
    {
        return ATCA_SUCCESS;
    }
}

ATCA_STATUS hal_unlock_mutex(void * pMutex)
{
    if (!pMutex)
    {
        return ATCA_BAD_PARAM;
    }

    if (!xSemaphoreGive((SemaphoreHandle_t)pMutex))
    {
        return ATCA_GEN_FAIL;
    }
    else
    {
        return ATCA_SUCCESS;
    }
}

void atca_delay_ms(uint32_t delay)
{
    vTaskDelay(delay);
}

/** \brief discover i2c buses available for this hardware
 * this maintains a list of logical to physical bus mappings freeing the application
 * of the prior knowledge
 * \param[in] i2c_buses - an array of logical bus numbers
 * \param[in] max_buses - maximum number of buses the app wants to attempt to discover
 * \return ATCA_SUCCESS on success, otherwise an error code.
 */

ATCA_STATUS hal_i2c_discover_buses(int i2c_buses[], int max_buses)
{
    i2c_buses[0] = 1;
    return ATCA_SUCCESS;

}


/** \brief discover any CryptoAuth devices on a given logical bus number
 * \param[in] bus_num - logical bus number on which to look for CryptoAuth devices
 * \param[out] cfg[] - pointer to head of an array of interface config structures which get filled in by this method
 * \param[out] *found - number of devices found on this bus
 * \return ATCA_SUCCESS on success, otherwise an error code.
 */

ATCA_STATUS hal_i2c_discover_devices(int bus_num, ATCAIfaceCfg cfg[], int *found)
{
    ATCAIfaceCfg *head = cfg;
    uint8_t slaveAddress = 0x01;
    ATCADevice device;

#ifdef ATCA_NO_HEAP
    struct atca_device disc_device;
    struct atca_command disc_command;
    struct atca_iface disc_iface;
#endif
    ATCAPacket packet;
    ATCA_STATUS status;
    uint8_t revs608[][4] = { { 0x00, 0x00, 0x60, 0x01 }, { 0x00, 0x00, 0x60, 0x02 } };
    uint8_t revs508[][4] = { { 0x00, 0x00, 0x50, 0x00 } };
    uint8_t revs108[][4] = { { 0x80, 0x00, 0x10, 0x01 } };
    uint8_t revs204[][4] = { { 0x00, 0x02, 0x00, 0x08 }, { 0x00, 0x02, 0x00, 0x09 }, { 0x00, 0x04, 0x05, 0x00 } };
    int i;

    // default configuration, to be reused during discovery process
    ATCAIfaceCfg discoverCfg = {
        .iface_type             = ATCA_I2C_IFACE,
        .devtype                = ATECC608A,
        .atcai2c.slave_address  = 0xC0,
        .atcai2c.bus            = bus_num,
        .atcai2c.baud           = 400000,
        //.atcai2c.baud = 100000,
        .wake_delay             = 800,
        .rx_retries             = 3
    };

    if (bus_num < 0)
    {
        return ATCA_COMM_FAIL;
    }

#ifdef ATCA_NO_HEAP
    disc_device.mCommands = &disc_command;
    disc_device.mIface    = &disc_iface;
    status = initATCADevice(&discoverCfg, &disc_device);
    if (status != ATCA_SUCCESS)
    {
        return status;
    }
    device = &disc_device;
#else
    device = newATCADevice(&discoverCfg);
    if (device == NULL)
    {
        return ATCA_COMM_FAIL;
    }
#endif

    // iterate through all addresses on given i2c bus
    // all valid 7-bit addresses go from 0x07 to 0x78
    for (slaveAddress = 0x07; slaveAddress <= 0x78; slaveAddress++)
    {
        discoverCfg.atcai2c.slave_address = slaveAddress;  // turn it into an 8-bit address which is what the rest of the i2c HAL is expecting when a packet is sent

        memset(packet.data, 0x00, sizeof(packet.data));
        // build an info command
        packet.param1 = INFO_MODE_REVISION;
        packet.param2 = 0;
        // get devrev info and set device type accordingly
        atInfo(device->mCommands, &packet);
        if ((status = atca_execute_command(&packet, device)) != ATCA_SUCCESS)
        {
            continue;
        }

        // determine device type from common info and dev rev response byte strings... start with unknown
        discoverCfg.devtype = ATCA_DEV_UNKNOWN;
        for (i = 0; i < (int)sizeof(revs608) / 4; i++)
        {
            if (memcmp(&packet.data[1], &revs608[i], 4) == 0)
            {
                discoverCfg.devtype = ATECC608A;
                break;
            }
        }

        for (i = 0; i < (int)sizeof(revs508) / 4; i++)
        {
            if (memcmp(&packet.data[1], &revs508[i], 4) == 0)
            {
                discoverCfg.devtype = ATECC508A;
                break;
            }
        }

        for (i = 0; i < (int)sizeof(revs204) / 4; i++)
        {
            if (memcmp(&packet.data[1], &revs204[i], 4) == 0)
            {
                discoverCfg.devtype = ATSHA204A;
                break;
            }
        }

        for (i = 0; i < (int)sizeof(revs108) / 4; i++)
        {
            if (memcmp(&packet.data[1], &revs108[i], 4) == 0)
            {
                discoverCfg.devtype = ATECC108A;
                break;
            }
        }

        if (discoverCfg.devtype != ATCA_DEV_UNKNOWN)
        {
            // now the device type is known, so update the caller's cfg array element with it
            (*found)++;
            memcpy( (uint8_t*)head, (uint8_t*)&discoverCfg, sizeof(ATCAIfaceCfg));
            head->devtype = discoverCfg.devtype;
            head++;
        }

        atca_delay_ms(15);
    }

#ifdef ATCA_NO_HEAP
    releaseATCADevice(device);
#else
    deleteATCADevice(&device);

#endif

    return ATCA_SUCCESS;
}

/** \brief hal_i2c_init manages requests to initialize a physical interface.  it manages use counts so when an interface
 * has released the physical layer, it will disable the interface for some other use.
 * You can have multiple ATCAIFace instances using the same bus, and you can have multiple ATCAIFace instances on
 * multiple i2c buses, so hal_i2c_init manages these things and ATCAIFace is abstracted from the physical details.
 */

/** \brief initialize an I2C interface using given config
 * \param[in] hal - opaque ptr to HAL data
 * \param[in] cfg - interface configuration
 * \return ATCA_SUCCESS on success, otherwise an error code.
 */

ATCA_STATUS hal_i2c_init(void *hal, ATCAIfaceCfg *cfg)
{
    if (cfg->atcai2c.bus > MAX_I2C_BUSES)
    {
        return ATCA_COMM_FAIL;
    }
    ATCAI2CMaster_t* data = &i2c_hal_data[cfg->atcai2c.bus];
    ((ATCAHAL_t*)hal)->hal_data = data;

    return ATCA_SUCCESS;
}

/** \brief HAL implementation of I2C post init
 * \param[in] iface  instance
 * \return ATCA_STATUS
 */
ATCA_STATUS hal_i2c_post_init(ATCAIface iface)
{
    return ATCA_SUCCESS;
}

/** \brief HAL implementation of I2C send over START
 * \param[in] iface     instance
 * \param[in] txdata    pointer to space to bytes to send
 * \param[in] txlength  number of bytes to send
 * \return ATCA_SUCCESS on success, otherwise an error code.
 */

ATCA_STATUS hal_i2c_send(ATCAIface iface, uint8_t *txdata, int txlength)
{
    ATCAIfaceCfg *cfg = atgetifacecfg(iface);

    struct _i2c_m_msg packet = {
        .addr   = cfg->atcai2c.slave_address,
        .buffer = txdata,
        .flags  = I2C_M_SEVEN | I2C_M_STOP,
    };

    // for this implementation of I2C with CryptoAuth chips, txdata is assumed to have ATCAPacket format

    // other device types that don't require i/o tokens on the front end of a command need a different hal_i2c_send and wire it up instead of this one
    // this covers devices such as ATSHA204A and ATECCx08A that require a word address value pre-pended to the packet
    // txdata[0] is using _reserved byte of the ATCAPacket
    txdata[0] = 0x03;   // insert the Word Address Value, Command token
    txlength++;         // account for word address value byte.
    packet.len = txlength;

    if (I2C_SUCCESS != I2C_transmit(ATE, packet.buffer, packet.len, RX_TIMEOUT_TICKS))
    {
        return ATCA_COMM_FAIL;
    }

    return ATCA_SUCCESS;
}

/** \brief HAL implementation of I2C receive function for START I2C
 * \param[in]    iface     Device to interact with.
 * \param[out]   rxdata    Data received will be returned here.
 * \param[inout] rxlength  As input, the size of the rxdata buffer.
 *                         As output, the number of bytes received.
 * \return ATCA_SUCCESS on success, otherwise an error code.
 */
ATCA_STATUS hal_i2c_receive(ATCAIface iface, uint8_t *rxdata, uint16_t *rxlength)
{
    ATCAIfaceCfg *cfg = atgetifacecfg(iface);
    int retries = cfg->rx_retries;
    int status = !ATCA_SUCCESS;
    uint16_t rxdata_max_size = *rxlength;

    struct _i2c_m_msg packet = {
        .addr   = cfg->atcai2c.slave_address,
        .len    = 1,
        .buffer = rxdata,
        .flags  = I2C_M_SEVEN | I2C_M_RD | I2C_M_STOP,
    };

    *rxlength = 0;
    if (rxdata_max_size < 1)
    {
        return ATCA_SMALL_BUFFER;
    }

    while (retries-- > 0 && status != ATCA_SUCCESS)
    {
        vTaskDelay(2);
        if (I2C_SUCCESS != I2C_receive(ATE, packet.buffer, packet.len, RX_TIMEOUT_TICKS))
        {
            status = ATCA_COMM_FAIL;
        }
        else
        {
            status = ATCA_SUCCESS;
        }
    }
    if (status != ATCA_SUCCESS)
    {
        return status;
    }
    if (rxdata[0] < ATCA_RSP_SIZE_MIN)
    {
        return ATCA_INVALID_SIZE;
    }
    if (rxdata[0] > rxdata_max_size)
    {
        return ATCA_SMALL_BUFFER;
    }

    //Update receive length with first byte received and set to read rest of the data
    packet.len = rxdata[0] - 1;
    packet.buffer = &rxdata[1];

    if (I2C_SUCCESS != I2C_receive(ATE, packet.buffer, packet.len, RX_TIMEOUT_TICKS))
    {
        status = ATCA_COMM_FAIL;
    }
    else
    {
        status = ATCA_SUCCESS;
    }
    if (status != ATCA_SUCCESS)
    {
        return status;
    }

    *rxlength = rxdata[0];

    return ATCA_SUCCESS;
}


/** \brief wake up CryptoAuth device using I2C bus
 * \param[in] iface  interface to logical device to wakeup
 * \return ATCA_SUCCESS on success, otherwise an error code.
 */

ATCA_STATUS hal_i2c_wake(ATCAIface iface)
{
    ATCAIfaceCfg *cfg = atgetifacecfg(iface);
    int retries = cfg->rx_retries;
    int status = !HAL_OK;
    uint8_t data[4];

    memset((uint8_t*)&data, 0, 4);

    // send the wake by writing to an address of 0x00
    struct _i2c_m_msg packet = {
        .addr   = 0x00,
        .len    = 4,
        .buffer = data,
        .flags  = I2C_M_SEVEN | I2C_M_STOP,
    };

    I2C_transmit(ATE, data, 4, 100);

    vTaskDelay(2);

    // receive the wake up response
    packet.len = 4;
    packet.buffer = data;
    packet.flags  = I2C_M_SEVEN | I2C_M_RD | I2C_M_STOP;

    while (retries-- > 0 && status != I2C_SUCCESS)
    {

        status = I2C_receive(ATE, packet.buffer, packet.len, MAX_TIMEOUT_MS);
        vTaskDelay(2);
    }

    return hal_check_wake(data, 4);
}

/** \brief idle CryptoAuth device using I2C bus
 * \param[in] iface  interface to logical device to idle
 * \return ATCA_SUCCESS on success, otherwise an error code.
 */
ATCA_STATUS hal_i2c_idle(ATCAIface iface)
{
    ATCAIfaceCfg *cfg = atgetifacecfg(iface);
    uint8_t data[4];

    struct _i2c_m_msg packet = {
        .addr   = cfg->atcai2c.slave_address,
        .len    = 1,
        .buffer = &data[0],
        .flags  = I2C_M_SEVEN | I2C_M_STOP,
    };

    data[0] = 0x02;  // idle word address value
    if (I2C_SUCCESS != I2C_transmit(ATE, packet.buffer,packet.len, MAX_TIMEOUT_MS))
    {
        return ATCA_COMM_FAIL;
    }

    return ATCA_SUCCESS;
}

/** \brief sleep CryptoAuth device using I2C bus
 * \param[in] iface  interface to logical device to sleep
 * \return ATCA_SUCCESS on success, otherwise an error code.
 */

ATCA_STATUS hal_i2c_sleep(ATCAIface iface)
{
    ATCAIfaceCfg *cfg = atgetifacecfg(iface);
    uint8_t data[4];

    struct _i2c_m_msg packet = {
        .addr   = cfg->atcai2c.slave_address,
        .len    = 1,
        .buffer = &data[0],
        .flags  = I2C_M_SEVEN | I2C_M_STOP,
    };

    data[0] = 0x01;  // sleep word address value
    if (I2C_SUCCESS != I2C_receive(ATE, packet.buffer,packet.len, MAX_TIMEOUT_MS))
    {
        return ATCA_COMM_FAIL;
    }

    return ATCA_SUCCESS;
}

/** \brief manages reference count on given bus and releases resource if no more refences exist
 * \param[in] hal_data - opaque pointer to hal data structure - known only to the HAL implementation
 * \return ATCA_SUCCESS on success, otherwise an error code.
 */

ATCA_STATUS hal_i2c_release(void *hal_data)
{
    return ATCA_SUCCESS;
}

/** @} */
