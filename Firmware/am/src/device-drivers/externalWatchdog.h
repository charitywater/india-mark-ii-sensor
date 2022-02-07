/*
================================================================================================#=
Module:   External Watchdog Driver

Description:
    Kick logic for an external supervisor chip

Copyright 2021 Twisthink LLC
This code is licensed under Twisthink license.
See Twisthink-SoftwareLicenseAgreement-Auris.docx for details.
================================================================================================#=
*/


#ifndef DEVICE_DRIVERS_EXTERNALWATCHDOG_H_
#define DEVICE_DRIVERS_EXTERNALWATCHDOG_H_

extern void WD_initKick(void);
extern void WD_deinitKick(void);

#endif /* DEVICE_DRIVERS_EXTERNALWATCHDOG_H_ */
