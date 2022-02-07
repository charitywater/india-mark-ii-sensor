/*
================================================================================================#=
Module:   UART Driver

Description:
    Driver to send and receive uart data. Manages all of the UART peripherals

Copyright 2021 Twisthink LLC
This code is licensed under Twisthink license.
See Twisthink-SoftwareLicenseAgreement-Auris.docx for details.
================================================================================================#=
*/

#ifndef PERIPHERAL_DRIVERS_UART_H_
#define PERIPHERAL_DRIVERS_UART_H_

#include "stdint.h"

//add more if needed in the future
typedef enum
{
    LOG,
}UART_Periph_t;

extern void UART_initPeripherals(void);
extern void UART_deinitDebugPeripherals(void);
extern void UART_sendDataBlocking(UART_Periph_t device, uint8_t *pData, uint16_t bytesToSend);

#endif /* PERIPHERAL_DRIVERS_UART_H_ */
