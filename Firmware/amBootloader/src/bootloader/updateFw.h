/*
================================================================================================#=
Module:   Update FW

Description:
    Pull data from SPI flash and load into STM32 internal flash. Zero pads
    remaining STM32 internal flash

Copyright 2021 Twisthink LLC
This code is licensed under Twisthink license.
See Twisthink-SoftwareLicenseAgreement-Auris.docx for details.
================================================================================================#=
*/

#ifndef BOOTLOADER_UPDATEFW_H_
#define BOOTLOADER_UPDATEFW_H_

extern bool UPDATE_readExternalFlashAndProgramInternal(uint32_t startAddrExternal, uint32_t length);

#endif /* BOOTLOADER_UPDATEFW_H_ */
