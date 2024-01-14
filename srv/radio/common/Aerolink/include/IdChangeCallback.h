/*===========================================================================
Copyright (c) 2017-2019 OnBoard Security, Inc.
Copyright (c) 2020 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.
=========================================================================== */

#ifndef ID_CHANGE_CALLBACK_H
#define ID_CHANGE_CALLBACK_H

#include <stdint.h>
#include "ws_errno.h"

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @brief ID Change initiation callback function
 */
typedef void (*IdChangeInitCallback)(void  *userData, uint8_t  numCerts, uint8_t *certIndex);


/**
 * @brief ID Change completion callback function
 */
typedef void (*IdChangeDoneCallback)(AEROLINK_RESULT  returnCode, void  *userData, uint8_t const * certId);


#ifdef __cplusplus
}
#endif

#endif
