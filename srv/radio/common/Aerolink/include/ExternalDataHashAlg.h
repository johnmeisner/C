/*===========================================================================
Copyright (c) 2017-2019 OnBoard Security, Inc.
Copyright (c) 2020 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.
=========================================================================== */

#ifndef EXTERNAL_DATA_HASH_ALG_H
#define EXTERNAL_DATA_HASH_ALG_H

/**
 * @file ExternalDataHashAlg.h
 * @brief Enumeration of hash algorithms for externalDataHash.
 */

typedef enum
{
    EDHA_NONE,          // implies no externalDataHash
    EDHA_SHA256,        // implies an externalDataHash length of 32 octets
} ExternalDataHashAlg;


#endif /* EXTERNAL_DATA_HASH_ALG_H */

