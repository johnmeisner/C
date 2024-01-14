/*===========================================================================
Copyright (c) 2017-2019 OnBoard Security, Inc.
Copyright (c) 2020 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.
=========================================================================== */

#ifndef ENCRYPTION_API_H
#define ENCRYPTION_API_H

#include <stdint.h>
#include "aerolink_api.h"
#include "ws_errno.h"


#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Structure that holds an encryption key passed to and from Aerolink for
 * generating, sending, receiving, and using the key.
 *
 * @note Aerolink will allocate the memory for blob. The user application is responsible for freeing the blob
 *       memory when the application is done with this structure.
 */
typedef struct
{
    uint8_t const *blob;
    uint32_t       blobLen;
} AerolinkEncryptionKey;


/***
 * @brief Enumeration of the allowed symmetric encryption algorithms.
 */
typedef enum
{
    SEA_AES128CCM,
    SEA_SM4CCM,
} SymmetricEncryptionAlgorithm;


/***
 * @brief Enumeration of the allowed public encryption algorithms.
 */
typedef enum
{
    PEA_ECIES_NISTP256,
    PEA_ECIES_BRAINPOOL256R1,
    PEA_ECENC_SM2,
} PublicEncryptionAlgorithm;


/***
 * @brief Enumeration of the allowed symmetric key types.
 * @
 */
typedef enum
{
    SKT_DEK,        /* Data Encryption Key */
    SKT_KEK,        /* Key Encryption Key */
} SymmetricKeyType;

/**
 * @brief Generate a new public encryption key pair.
 *
 * @note The encryptionKey contains a blob pointer which is set upon successful completion of this operation. The caller
 *       is responsible for deleting this allocated space if the function securityServices_deleteEncryptionKey() is not called.
 *
 * @param alg (IN) Algorithm to be used.
 * @param encryptionKey (OUT) Pointer where encryption key is stored.
 *
 * @return AEROLINK_RESULT
 *
 * @retval WS_SUCCESS
 *     - operation was successful
 *
 * @retval WS_ERR_BAD_ARGS
 *     - encryptionKey is NULL
 *
 *
 */
AEROLINK_EXPORT AEROLINK_RESULT
securityServices_generatePublicEncryptionKeyPair(
        PublicEncryptionAlgorithm         alg,
        AerolinkEncryptionKey     * const encryptionKey);


/**
 * @brief Generate a new symmetric encryption key.
 *
 * @note The encryptionKey contains a blob pointer which is set upon successful completion of this operation. The caller
 *       is responsible for deleting this allocated space if the function securityServices_deleteEncryptionKey() is not called.
 *
 * @param alg (IN) Algorithm to be used.
 * @param symmetricKeyType (IN) Symmetric key type to be used.
 * @param encryptionKey (OUT) Pointer where encryption key is stored.
 *
 * @return AEROLINK_RESULT
 *
 * @retval WS_SUCCESS
 *     - operation was successful
 *
 * @retval WS_ERR_BAD_ARGS
 *     - encryptionKey is NULL
 *
 *
 */
AEROLINK_EXPORT AEROLINK_RESULT
securityServices_generateSymmetricEncryptionKey(
        SymmetricEncryptionAlgorithm      alg,
        SymmetricKeyType                  symmetricKeyType,
        AerolinkEncryptionKey     * const encryptionKey);

/**
 * @brief Import a public encryption key out of band into an AerolinkEncryptionKey.
 *
 * @note The encryptionKey contains a blob pointer which is set upon successful completion of this operation. The caller
 *       is responsible for deleting this allocated space if the function securityServices_deleteEncryptionKey() is not called.
 *
 * @param alg (IN) Algorithm to be used.
 * @param key (IN) Pointer where public key to be imported is stored.
 *                 The first byte must be the public key type: X-Only, Y-0 or Y-1 or Uncompressed
 * @param encryptionKey (OUT) Pointer where encryption key is stored.
 *
 * @return AEROLINK_RESULT
 *
 * @retval WS_SUCCESS
 *     - operation was successful
 *
 * @retval WS_ERR_BAD_ARGS
 *     - encryptionKey is NULL
 *     - unsupported algorithm
 *
 */
AEROLINK_EXPORT AEROLINK_RESULT
securityServices_importPublicEncryptionKey(
        PublicEncryptionAlgorithm         alg,
        uint8_t const                    *key,
        AerolinkEncryptionKey     * const encryptionKey);

/**
 * @brief Import a symmetric key out of band into an AerolinkEncryptionKey.
 *
 * @note The encryptionKey contains a blob pointer which is set upon successful completion of this operation. The caller
 *       is responsible for deleting this allocated space if the function securityServices_deleteEncryptionKey() is not called.
 *
 * @param alg (IN) Algorithm to be used.
 * @param keyType (IN) Payload type to be encrypted.
 * @param key (IN) Pointer where symmetric key to be imported is stored.
 * @param encryptionKey (OUT) Pointer where encryption key is stored.
 *
 * @return AEROLINK_RESULT
 *
 * @retval WS_SUCCESS
 *     - operation was successful
 *
 * @retval WS_ERR_BAD_ARGS
 *     - encryptionKey is NULL
 *     - unsupported algorithm
 *     - unsupported keyType
 *
 */
AEROLINK_EXPORT AEROLINK_RESULT
securityServices_importSymmetricEncryptionKey(
        SymmetricEncryptionAlgorithm         alg,
        SymmetricKeyType                     keyType,
        uint8_t const                        *key,
        AerolinkEncryptionKey        * const encryptionKey);

/***
 * @brief Delete the decryption key associated with the supplied encryption key.
 *
 * @note The blob pointer will be deleted and replaced with a NULL. The blobLen will be set to 0.
 *
 * @param encryptionKey (IN/OUT)
 *
 * @return AEROLINK_RESULT
 *
 * @retval WS_SUCCESS
 *     - operation was successful
 *
 * @retval WS_ERR_BAD_ARGS
 *     - encryptionKey is NULL
 *     - encryptionKey->blob is NULL
 *
 */
AEROLINK_EXPORT AEROLINK_RESULT
securityServices_deleteEncryptionKey(
        AerolinkEncryptionKey * const encryptionKey);


#ifdef __cplusplus
}
#endif

#endif //ENCRYPTION_API_H
