/**************************************************************************************************
* \file     atca_cert_chain.h
* \brief    Template for security chip. Microchip drivers will use this.
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

#ifndef TESTS_ATCA_CERT_CHAIN_H_
#define TESTS_ATCA_CERT_CHAIN_H_

#include "atcacert/atcacert_def.h"

//these are specific to our crypto chip part - ATECC608A-TNGTLSS-B
extern const atcacert_def_t g_tngtls_cert_def_1_signer;
extern const atcacert_def_t g_tngtls_cert_def_3_device;

#endif /* TESTS_ATCA_CERT_CHAIN_H_ */
