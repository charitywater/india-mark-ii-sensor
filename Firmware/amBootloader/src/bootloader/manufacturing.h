/*
================================================================================================#=
Module:   Manufacturing

Description:
    Move internal flash to external and update image registry
    This should only be called during the manufacturing process when
    an image package is loaded into internal flash

Copyright 2021 Twisthink LLC
This code is licensed under Twisthink license.
See Twisthink-SoftwareLicenseAgreement-Auris.docx for details.
================================================================================================#=
*/

#ifndef BOOTLOADER_MANUFACTURING_H_
#define BOOTLOADER_MANUFACTURING_H_

#include "stdint.h"
#include "stdbool.h"

typedef struct
{
    uint32_t major;
    uint32_t minor;
    uint32_t build;
}manufacturingImageVersion_t;

extern bool MFG_MODE_checkForImagePackageInternalFlash(void);
extern manufacturingImageVersion_t MFG_MODE_getImageVersionFoundInFlash(void);
extern bool MFG_MODE_copyImagePackageToExternalFlash(void);

#endif /* BOOTLOADER_MANUFACTURING_H_ */
