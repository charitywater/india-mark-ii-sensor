/*
================================================================================================#=
Module:   Flash Handler

Description:
    Flash Driver, interfaces with the NAND flash chip

Copyright 2021 Twisthink LLC
This code is licensed under Twisthink license.
See Twisthink-SoftwareLicenseAgreement-Auris.docx for details.
================================================================================================#=
*/

#ifndef DEVICE_DRIVERS_FLASHHANDLER_H_
#define DEVICE_DRIVERS_FLASHHANDLER_H_

#include "stdint.h"

/*
 * Organization
– Page size x1: 2112 bytes (2048 + 64 bytes)
– Block size: 64 pages (128K + 4K bytes)
– Plane size: 2 planes x 512 blocks per plane
– Device size: 1Gb: 1024 blocks
 */
#define MT29F1_MAX_ADDR             (0x8000000 - 1)
#define MT29F1_PAGE_SIZE            2048
#define MT29F1_OVERHEAD             64
#define MT29F1_PAGES_PER_BLOCK      64
#define MT29F1_BLOCK_SIZE           (MT29F1_PAGE_SIZE + MT29F1_OVERHEAD) * MT29F1_PAGES_PER_BLOCK
#define MT29F1_BLOCKS_PER_PLANE     512
#define MT29F1_PLANE_SIZE           MT29F1_BLOCK_SIZE * MT29F1_BLOCKS_PER_PLANE
#define MT29F1_PLANES_PER_DEVICE    2


#define MAX_FLASH_RX_LEN            MT29F1_PAGE_SIZE + MT29F1_OVERHEAD
#define MAX_FLASH_TX_LEN            MT29F1_PAGE_SIZE + MT29F1_OVERHEAD

typedef enum
{
   FLASH_SUCCESS = 0,
   FLASH_INIT_ERR,
   FLASH_SPI_ERR,
   FLASH_ADDR_ERR,
   FLASH_LEN_ERR,
   FLASH_MUTEX_ERR,
   FLASH_GEN_ERROR,
} flashErr_t;

extern void FLASH_init(void);
extern flashErr_t FLASH_write(uint32_t address, uint8_t* data, uint32_t len);
extern flashErr_t FLASH_read(uint32_t address, uint8_t *data, uint32_t len);
extern flashErr_t FLASH_erase(uint32_t address, uint32_t len);

#endif /* DEVICE_DRIVERS_FLASHHANDLER_H_ */
