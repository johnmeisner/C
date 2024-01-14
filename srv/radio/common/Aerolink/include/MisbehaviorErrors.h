/*===========================================================================
Copyright (c) 2019 OnBoard Security, Inc.
Copyright (c) 2020 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.
=========================================================================== */

#ifndef AEROLINK_MISBEHAVIOR_ERRORS_H
#define AEROLINK_MISBEHAVIOR_ERRORS_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    SI_SUCCESS = 0,
    SI_ERR_BAD_ARGS,
    SI_ERR_SECURITY_LIBRARY_MISBEHAVIOR_CONFIGURATION,
    SI_ERR_SECURITY_LIBRARY_FAILURE,
    SI_ERR_NOT_SIGNED_SPDU,
    SI_ERR_NOT_INITIALIZED,
    SI_ERR_SECURITY_LIBRARY_NOT_INITIALIZED,
    SI_ERR_NO_REPORT_AVAILABLE,
    SI_ERR_BUFFER_TOO_SMALL,
    SI_ERR_NO_CERTIFICATE,
} SECURITY_INTERFACE_RESULT;


#ifdef __cplusplus
}
#endif

#endif //AEROLINK_MISBEHAVIOR_ERRORS_H
