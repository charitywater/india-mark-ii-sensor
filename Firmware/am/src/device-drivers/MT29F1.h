/************************** Flash Memory Driver ***********************************

   Filename:    SPI-NAND.h
   Description: Library routines for the SPI-NAND Flash Memory.

   Version:     1.0
   Author:      Micron Italia,  Arzano (Napoli)

   Copyright (c) 2011 Micron

   THIS PROGRAM IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND, EITHER
   EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO, THE IMPLIED WARRANTY
   OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE ENTIRE RISK
   AS TO THE QUALITY AND PERFORMANCE OF THE PROGRAM IS WITH YOU. SHOULD THE
   PROGRAM PROVE DEFECTIVE, YOU ASSUME THE COST OF ALL NECESSARY SERVICING,
   REPAIR OR CORRECTION.
********************************************************************************

   Version History.

   Ver.		Date			Comments

   1.0		Dec 2011 		First release 

********************************************************************************

  The following functions are available in this library:

	ReturnType FlashReadStatusRegister(NMX_uint8 *ucpStatusRegister);
	ReturnType FlashWriteEnable(void);
	ReturnType FlashWriteDisable(void);
	ReturnType FlashBlockErase(uAddrType udBlockAddr);
	ReturnType FlashPageRead(uAddrType udAddr, NMX_uint8 *pArray);
	ReturnType FlashPageReadDual(uAddrType udAddr, NMX_uint8 *pArray);
	ReturnType FlashPageReadQuad(uAddrType udAddr, NMX_uint8 *pArray);
	ReturnType FlashReadDeviceIdentification(NMX_uint16 *uwpDeviceIdentification);
	ReturnType FlashPageProgram(uAddrType udAddr, NMX_uint8 *pArray);
	ReturnType FlashRandomProgram(uAddrType rowAddr, chunk* cks, NMX_uint8 num_of_chunk);
	ReturnType FlashInternalDataMove(uAddrType udSourceAddr, uAddrType udDestAddr);
	ReturnType FlashUnlock(ProtectedRows pr);
	ReturnType FlashUnlockAll(void);
	ReturnType FlashReadOTPStatus(NMX_uint8 *otp_reg_value);
	ReturnType FlashGetFeature(NMX_uint8 ucRegAddr, NMX_uint8 *ucpRegValue);
	ReturnType FlashSetFeature(Register ucRegAddr, NMX_uint8 ucpRegValue);
	ReturnType FlashReset(void);

********************************************************************************/

#ifndef _NAND4224_
#define _NAND4224_

#include "stdint.h"

typedef uint32_t uAddrType;

/* List of supported device */
#define MT29F1G01

#ifdef MT29F1G01
	/* device details */
	#define FLASH_WIDTH                 8               /* Flash data width */
	#define PAGE_SIZE                   2176            /* Page size in bytes */
	#define PAGE_DATA_SIZE              2048            /* Page data size in bytes */
	#define PAGE_SPARE_SIZE             128             /* Page spare size in bytes*/
	#define NUM_BLOCKS                  1024            /* Number of blocks*/
	#define NUM_PAGE_BLOCK              64              /* Number of pages for block*/

    #define BLOCK_SIZE                  (NUM_PAGE_BLOCK * PAGE_DATA_SIZE)
    #define MAX_ROW_ADDR                (uint32_t)((NUM_BLOCKS-1)<<6 | (NUM_PAGE_BLOCK -1))

	/* utility macros */
	#define ADDRESS_2_BLOCK(Address)	((uint16_t) (Address >> 17))
	#define ADDRESS_2_PAGE(Address)		((uint8_t)  ((Address >> 11) & 0x3F))
	#define ADDRESS_2_COL(Address)		((uint16_t) (Address & 0x3F))

    #define BLOCK_2_ADDRESS(block)      ((uint32_t) (block << 17))

#endif

#define SE_TIMEOUT 10 	/* MAX timeout in seconds suggested for Sector Erase Operation */
#define TRUE 1
#define FALSE 0


/* Functions Return Codes */
typedef enum {
	Flash_Success,
	Flash_AddressInvalid,
	Flash_RegAddressInvalid,
	Flash_MemoryOverflow,
	Flash_BlockEraseFailed,
	Flash_PageNrInvalid,
	Flash_SubSectorNrInvalid,
	Flash_SectorNrInvalid,
	Flash_FunctionNotSupported,
	Flash_NoInformationAvailable,
	Flash_OperationOngoing,
	Flash_OperationTimeOut,
	Flash_ProgramFailed,
	Flash_SectorProtected,
	Flash_SectorUnprotected,
	Flash_SectorProtectFailed,
	Flash_SectorUnprotectFailed,
	Flash_SectorLocked,
	Flash_SectorUnlocked,
	Flash_SectorLockDownFailed,
	Flash_WrongType
} mt29f_status_t;


#define FlashAddressMask 0x087Ful
#define FlashPageSize 0x0880ul


/*
 * SPI NAND Command Set Definitions (see Datasheet)
 */

enum
{
    SPI_NAND_BLOCK_ERASE_INS 			= 0xD8,
    SPI_NAND_GET_FEATURE_INS 			= 0x0F,
    SPI_NAND_PAGE_READ_INS 				= 0x13,
    SPI_NAND_PROGRAM_EXEC_INS 			= 0x10,
    SPI_NAND_PROGRAM_LOAD_INS 			= 0x02,
    SPI_NAND_PROGRAM_LOAD_RANDOM_INS 	= 0x84,
    SPI_NAND_READ_CACHE_INS 			= 0x03, //Or 0x0B
    SPI_NAND_READ_CACHE_X2_INS 			= 0x3B, // dual wire I/O
    SPI_NAND_READ_CACHE_X4_INS 			= 0x6B, // quad wire I/O
    SPI_NAND_READ_ID 					= 0x9F,
    SPI_NAND_RESET 						= 0xFF,
    SPI_NAND_SET_FEATURE 				= 0x1F,
    SPI_NAND_WRITE_DISABLE 				= 0x04,
    SPI_NAND_WRITE_ENABLE 				= 0x06
};

/*
 * Flash Status Register Definitions (see Datasheet)
 */

/*
 * Status register description:
 *
 * SR7 - reserved
 * SR6 - reserved
 * SR5 - ECC status (*)
 * SR4 - ECC status (*)
 * SR3 - P_Fail Program fail
 * SR2 - E_Fail Erase fail
 * SR1 - WEL Write enable
 * SR0 - OIP Operation in progress
 *
 * ECC status bits (SR5,SR4) should be interpreted as follow:
 *
 * 00 = no errors;
 * 01 = 1 error corrected;
 * 10 = multiple errors _not_ corrected;
 * 11 = reserved.
 *
 */

/*
 * Register type (see Datasheet)
 */

typedef enum
{
	SPI_NAND_BLKLOCK_REG_ADDR 	= 0xA0,
	SPI_NAND_OTP_REG_ADDR 		= 0xB0,
	SPI_NAND_STATUS_REG_ADDR 	= 0xC0,
} Register;


/*
 * Values for status register (see Datasheet)
 */

enum
{
	SPI_NAND_PF   = 0x08, /* program fail */
	SPI_NAND_EF   = 0x04, /* erase fail */
	SPI_NAND_WE   = 0x02, /* write enable */
	SPI_NAND_OIP  = 0x01, /* operation in progress */
};

/*
 * Block Lock bits:
 *
 * BL7 - BRWD1 (*)
 * BL6 - reserved
 * BL5 - BP2
 * BL4 - BP1
 * BL3 - BP0
 * BL2 - reserved
 * BL1 - reserved
 * BL0 - reserved
 *
 * (*) If BRWD (block register write disable) is enabled and WP# is LOW,
 *     then the block lock register cannot be changed
 */

/*
 * Tables of Protected Rows
 *
 * (BP2, BP1, BP0)
 *   0    0    0   ->  None (all unlocked)
 *   0    0    1   ->  Upper 1/64 locked
 *   0    1    0   ->  Upper 1/32 locked
 *   0    1    1   ->  Upper 1/16 locked
 *   1    0    0   ->  Upper 1/8 locked
 *   1    0    1   ->  Upper 1/4 locked
 *   1    1    0   ->  Upper 1/2 locked
 *   1    1    1   ->  All locked (default)
 *
 */

enum
{
	SPI_NAND_BRWD1 	= 0x80, // block register write disable
	SPI_NAND_BP2 	= 0x20,
	SPI_NAND_BP1 	= 0x10,
	SPI_NAND_BP0 	= 0x08,
};

typedef enum
{
	SPI_NAND_PROTECTED_ALL_UNLOCKED		= 0x00, // None�all unlocked
	SPI_NAND_PROTECTED_1_64_UPPER		= 0x01, // Upper 1/64 locked
	SPI_NAND_PROTECTED_1_32_UPPER		= 0x02, // Upper 1/32 locked
	SPI_NAND_PROTECTED_1_16_UPPER		= 0x03, // Upper 1/16 locked
	SPI_NAND_PROTECTED_1_8_UPPER		= 0x04, // Upper 1/8 locked
	SPI_NAND_PROTECTED_1_4_UPPER		= 0x05, // Upper 1/4 locked
	SPI_NAND_PROTECTED_1_2_UPPER		= 0x06, // Upper 1/2 locked
	SPI_NAND_PROTECTED_ALL_LOCKED		= 0x07, // All locked (default)
} ProtectedRows;

/*
 * Page read mode
 */
typedef enum _PageReadMode
{
    ReadFromCache,
    ReadFromCacheX2,
    ReadFromCacheX4,
} PageReadMode;

/* Chunk structure used in random operation */
typedef struct
{
	uint8_t* buffer;		/* pointer to chunk data buffer */
	uint32_t  chunkSize;	/* size in byte of the data in the buffer */
	uAddrType address;		/* address where the chunk will be programmed in the page */
} chunk;

/*
 * Hardware independent function
 */

extern mt29f_status_t FlashReadStatusRegister(uint8_t *ucpStatusRegister);
extern mt29f_status_t FlashWriteEnable(void);
extern mt29f_status_t FlashWriteDisable(void);
extern mt29f_status_t FlashBlockErase(uAddrType udBlockAddr);
extern mt29f_status_t FlashPageRead(uAddrType udAddr, uint8_t *pArray);
extern mt29f_status_t FlashPageReadDual(uAddrType udAddr, uint8_t *pArray);
extern mt29f_status_t FlashPageReadQuad(uAddrType udAddr, uint8_t *pArray, PageReadMode Mode);
extern mt29f_status_t FlashReadDeviceIdentification(uint16_t *uwpDeviceIdentification);
extern mt29f_status_t FlashPageProgram(uAddrType udAddr, uint8_t *pArray, uint32_t udNrOfElementsInArray);
extern mt29f_status_t FlashRandomProgram(uAddrType rowAddr, chunk* cks, uint8_t num_of_chunk);
extern mt29f_status_t FlashInternalDataMove(uAddrType udSourceAddr, uAddrType udDestAddr);
extern mt29f_status_t FlashUnlock(ProtectedRows pr);
extern mt29f_status_t FlashUnlockAll(void);
extern mt29f_status_t FlashReadOTPStatus(uint8_t *otp_reg_value);
extern mt29f_status_t FlashGetFeature(uint8_t ucRegAddr, uint8_t *ucpRegValue);
extern mt29f_status_t FlashSetFeature(Register ucRegAddr, uint8_t ucpRegValue);
extern mt29f_status_t FlashReset(void);

extern mt29f_status_t Build_RowAddressNoCmd(uint16_t block, uint8_t page, uint32_t* addr);
extern mt29f_status_t Build_Address(uint16_t block, uint8_t page, uint16_t col, uint32_t* addr);

#endif /* _NAND4224_ */
