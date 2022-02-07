/**************************************************************************************************
* \file     atca_config.h
* \brief    Security chip config
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

#ifndef ATCA_CONFIG_H_
#define ATCA_CONFIG_H_

/* Include HALS */
#define ATCA_HAL_I2C

/* Included device support */
#define ATCA_ATECC608_SUPPORT

/* \brief How long to wait after an initial wake failure for the POST to
 *         complete.
 * If Power-on self test (POST) is enabled, the self test will run on waking
 * from sleep or during power-on, which delays the wake reply.
 */
#ifndef ATCA_POST_DELAY_MSEC
#define ATCA_POST_DELAY_MSEC 25
#endif


#endif /* ATCA_CONFIG_H_ */
