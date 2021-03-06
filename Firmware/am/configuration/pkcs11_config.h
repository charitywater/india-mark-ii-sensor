/**************************************************************************************************
* \file     atecc_pkc11_config.h
* \brief    PKCS11 config
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

#ifndef PKCS11_CONFIG_H_
#define PKCS11_CONFIG_H_

/* Cryptoauthlib at the time of this module development is not versioned */
#ifndef ATCA_LIB_VER_MAJOR
#define ATCA_LIB_VER_MAJOR  3
#endif

#ifndef ATCA_LIB_VER_MINOR
#define ATCA_LIB_VER_MINOR  2
#endif

/** If an Auth-key or IoProtection Secret is to be used this is the
 * slot number of it */
#ifndef PKCS11_PIN_SLOT
#define PKCS11_PIN_SLOT                 6
#endif

/** Define to lock the PIN slot after writing */
#ifndef PKCS11_LOCK_PIN_SLOT
#define PKCS11_LOCK_PIN_SLOT            0
#endif

/** Enable PKCS#11 Debugging Messages */
#ifndef PKCS11_DEBUG_ENABLE
#define PKCS11_DEBUG_ENABLE             0
#endif

/** Use Static or Dynamic Allocation */
#ifndef PKCS11_USE_STATIC_MEMORY
#define PKCS11_USE_STATIC_MEMORY        1
#endif

/** Use a compiled configuration rather than loading from a filestore */
#ifndef PKCS11_USE_STATIC_CONFIG
#define PKCS11_USE_STATIC_CONFIG        1
#endif

/** Maximum number of slots allowed in the system - if static memory this will
   always be the number of slots */
#ifndef PKCS11_MAX_SLOTS_ALLOWED
#define PKCS11_MAX_SLOTS_ALLOWED        1
#endif

/** Maximum number of total sessions allowed in the system - if using static
   memory then this many session contexts will be allocated */
#ifndef PKCS11_MAX_SESSIONS_ALLOWED
#define PKCS11_MAX_SESSIONS_ALLOWED     10
#endif

/** Maximum number of cryptographic objects allowed to be cached */
#ifndef PKCS11_MAX_OBJECTS_ALLOWED
#define PKCS11_MAX_OBJECTS_ALLOWED      16
#endif

/** Maximum label size in characters */
#ifndef PKCS11_MAX_LABEL_SIZE
#define PKCS11_MAX_LABEL_SIZE           30
#endif

/****************************************************************************/
/* The following configuration options are for fine tuning of the library   */
/****************************************************************************/

/** Defines if the library will produce a static function list or use an
   externally defined one. This is an optimization that allows for a statically
   linked library to include only the PKCS#11 functions that the application
   intends to use. Otherwise compilers will not be able to optimize out the unusued
   functions */
#ifndef PKCS11_EXTERNAL_FUNCTION_LIST
#define PKCS11_EXTERNAL_FUNCTION_LIST    1
#endif

/** Static Search Attribute Cache in bytes (variable number of attributes based
   on size and memory requirements) */
#ifndef PKCS11_SEARCH_CACHE_SIZE
#define PKCS11_SEARCH_CACHE_SIZE        128
#endif

/** Device Support for ATECC508A */
#ifndef PKCS11_508_SUPPORT
#define PKCS11_508_SUPPORT              0
#endif

/** Device Support for ATECC608A */
#ifndef PKCS11_608_SUPPORT
#define PKCS11_608_SUPPORT              1
#endif

/** Support for configuring a "blank" or new device */
#ifndef PKCS11_TOKEN_INIT_SUPPORT
#define PKCS11_TOKEN_INIT_SUPPORT       1
#endif

/** Include the monotonic hardware feature as an object */
#ifndef PKCS11_MONOTONIC_ENABLE
#define PKCS11_MONOTONIC_ENABLE         0
#endif


#include "pkcs11/cryptoki.h"
#include <stddef.h>
typedef struct _pkcs11_slot_ctx *pkcs11_slot_ctx_ptr;
typedef struct _pkcs11_lib_ctx  *pkcs11_lib_ctx_ptr;
typedef struct _pkcs11_object   *pkcs11_object_ptr;

CK_RV pkcs11_config_load_objects(pkcs11_slot_ctx_ptr pSlot);
CK_RV pkcs11_config_load(pkcs11_slot_ctx_ptr slot_ctx);
CK_RV pkcs11_config_cert(pkcs11_lib_ctx_ptr pLibCtx, pkcs11_slot_ctx_ptr pSlot, pkcs11_object_ptr pObject, CK_ATTRIBUTE_PTR pcLabel);
CK_RV pkcs11_config_key(pkcs11_lib_ctx_ptr pLibCtx, pkcs11_slot_ctx_ptr pSlot, pkcs11_object_ptr pObject, CK_ATTRIBUTE_PTR pcLabel);
CK_RV pkcs11_config_remove_object(pkcs11_lib_ctx_ptr pLibCtx, pkcs11_slot_ctx_ptr pSlot, pkcs11_object_ptr pObject);

void pkcs11_config_init_private(pkcs11_object_ptr pObject, char * label, size_t len);
void pkcs11_config_init_public(pkcs11_object_ptr pObject, char * label, size_t len);
void pkcs11_config_init_cert(pkcs11_object_ptr pObject, char * label, size_t len);


#endif /* PKCS11_CONFIG_H_ */
