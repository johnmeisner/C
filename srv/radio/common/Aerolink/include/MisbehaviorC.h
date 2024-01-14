/*===========================================================================
Copyright (c) 2019 OnBoard Security, Inc.
Copyright (c) 2020 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.
=========================================================================== */

#ifndef AEROLINK_MISBEHAVIORC_H
#define AEROLINK_MISBEHAVIORC_H

#include <stdint.h>
#include "ws_errno.h"

#include "MisbehaviorErrors.h"

#ifdef __cplusplus
extern "C" {
#endif

/***
 * @brief Initialize the misbehavior library
 *        This routine must to be called once prior to any other library function.

 * @return SECURITY_INTERFACE_RESULT
 *
 * @retval WS_SUCCESS
 *     - operation was successful
 *
 * @retval SI_SECURITY_LIBRARY_NOT_INITIALIZED
 *     - the security library has not been previously initialized.
 *
 */
AEROLINK_EXPORT SECURITY_INTERFACE_RESULT
misbehavior_initialize(void);

/***
 * @brief Shut down the misbehavior library
 *
 * @return SECURITY_INTERFACE_RESULT
 *
 * @retval WS_SUCCESS
 *     - operation was successful
 *
 * @retval SI_NOT_INITIALIZED
 *     - the misbehavior library has not been previously initialized.
 *
 * @retval SI_SECURITY_LIBRARY_NOT_INITIALIZED
 *     - the security library has not been previously initialized.
 */
AEROLINK_EXPORT SECURITY_INTERFACE_RESULT
misbehavior_shutdown(void);


AEROLINK_EXPORT SECURITY_INTERFACE_RESULT
misbehavior_prepareSpduForReport(
    uint8_t const *sourceSpdu,
    uint32_t sourceSpduLength,
    uint8_t *updatedSpdu,
    uint32_t *updatedSpduLength,
    uint8_t *certificateBuffer,
    uint32_t *certificateBufferLength);



AEROLINK_EXPORT SECURITY_INTERFACE_RESULT
misbehavior_signEncryptReport (
    uint8_t const *	reportData,
    uint32_t 		reportDataLength,
    uint8_t *		preparedReport,
    uint32_t *		preparedReportLength);

AEROLINK_EXPORT SECURITY_INTERFACE_RESULT
misbehavior_getCurrentLocation(
    int32_t *   latitude,
    int32_t *   longitude,
    uint16_t *  elevation);


/**
 * @brief Convert the SECURITY_INTERFACE_RESULT to a string
 * @param errorCode
 * @return
 */
AEROLINK_EXPORT char const *
si_errid(
    SECURITY_INTERFACE_RESULT err);

#ifdef __cplusplus
}
#endif

#endif //AEROLINK_MISBEHAVIORC_H
