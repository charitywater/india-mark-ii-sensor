/*
================================================================================================#=
Module:   Bootloader

Description:
    Bootloader for the AM. Decides if we need to load an image from SPI flash into
    the SMT32 internal flash and does so. Also implements the BL cache

Copyright 2021 Twisthink LLC
This code is licensed under Twisthink license.
See Twisthink-SoftwareLicenseAgreement-Auris.docx for details.
================================================================================================#=
*/

#ifndef BOOTLOADER_BOOTLOADER_H_
#define BOOTLOADER_BOOTLOADER_H_

#include "stdbool.h"

extern bool BOOT_RunBootloader(void);

#endif /* BOOTLOADER_BOOTLOADER_H_ */
