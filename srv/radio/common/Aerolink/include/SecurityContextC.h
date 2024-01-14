/*===========================================================================
Copyright (c) 2017-2019 OnBoard Security, Inc.
Copyright (c) 2020 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.
=========================================================================== */

#ifndef SECURITY_CONTEXT_C_H
#define SECURITY_CONTEXT_C_H

/**
 * @file SecurityContextC.h
 * @brief C interface to the SecurityContext.
 */

#include <stdint.h>
#include "aerolink_api.h"
#include "ws_errno.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Handle to an allocated Security Context.
 */
typedef void* SecurityContextC;

/**
 * @brief Open a named security context.
 *
 * @param filename (IN) Name of the context file to open.
 * @param scHandle (OUT) Will point to the handle of the opened security context.
 *
 * @return AEROLINK_RESULT
 *
 * @retval WS_SUCCESS
 *     - operation was successful
 *
 * @retval WS_ERR_NOT_INITIALIZED
 *     - the security services have not been initialized.
 *
 * @retval WS_ERR_BAD_ARGS
 *     - name is NULL
 *     - scHandle is NULL
 *
 * @retval WS_ERR_ALREADY_OPEN
 *     - security context identified by name is already open
 *
 * @retval WS_ERR_INVALID_CONTEXT_CONFIG
 *     - no context configuration file for the given name
 */
AEROLINK_EXPORT AEROLINK_RESULT
sc_open(
        char const       *filename,
        SecurityContextC *scHandle);

/**
 * @brief Close a handle that was opened with sc_open(). After this call
 * the context will no longer be open and all generator and parsers associated
 * with this context will no longer be valid.
 *
 * @param scHandle (IN) Handle to a previously opened security context.
 *
 * @return AEROLINK_RESULT
 *
 * @retval WS_SUCCESS
 *     - operation was successful
 *
 * @retval WS_ERR_BAD_ARGS
 *     - scHandle is NULL
 *
 * @retval WS_ERR_INVALID_HANDLE
 *     - scHandle does not refer to a valid open security context.
 */
AEROLINK_EXPORT AEROLINK_RESULT
sc_close(
        SecurityContextC scHandle);

/**
 * @brief Return a pointer to the name of the context. This is the name provided to sc_open().
 *
 * @param scHandle (IN) Handle to a previously opened security context.
 * @param name (OUT) Will point to the name of the security context.
 *    This pointer is NOT to be freed by the caller and is valid only as long as the security context is open.
 *
 * @return AEROLINK_RESULT
 *
 * @retval WS_SUCCESS
 *     - operation was successful
 *
 * @retval WS_ERR_BAD_ARGS
 *     - scHandle is NULL
 *     - name is NULL
 *
 * @retval WS_ERR_INVALID_HANDLE
 *     - scHandle does not refer to a valid open security context.
 */
AEROLINK_EXPORT AEROLINK_RESULT
sc_getName(
        SecurityContextC   scHandle,
        char const       **name);


#ifdef __cplusplus
}
#endif

#endif /* SECURITY_CONTEXT_C_H */
