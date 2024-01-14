/*===========================================================================
Copyright (c) 2017-2019 OnBoard Security, Inc.
Copyright (c) 2020 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.
=========================================================================== */

#ifndef PAYLOAD_TYPE_H
#define PAYLOAD_TYPE_H

/**
 * @file PayloadType.h
 * @brief Enumeration of payload types.
 */

typedef enum
{
    PLT_NOT_SET,
    PLT_UNSECURED_APP_PAYLOAD,
    PLT_UNSECURED_SPDU,
    PLT_SIGNED_SPDU,
    PLT_ENCRYPTED_SPDU,
    PLT_UNKNOWN,
} PayloadType;

#endif /* PAYLOAD_TYPE_H */
