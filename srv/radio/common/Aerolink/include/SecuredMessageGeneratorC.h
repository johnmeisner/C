/*===========================================================================
Copyright (c) 2017-2019 OnBoard Security, Inc.
Copyright (c) 2020 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.
=========================================================================== */

#ifndef SECURED_MESSAGE_GENERATOR_C_H
#define SECURED_MESSAGE_GENERATOR_C_H

/**
 * @file SecuredMessageGeneratorC.h
 * @brief C interface to the SecuredMessageGenerator.
 */


#include <stdint.h>
#include "aerolink_api.h"
#include "Encryption.h"
#include "ExternalDataHashAlg.h"
#include "SecurityContextC.h"
#include "ws_errno.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Handle to an allocated Secured Message Generator.
 */
typedef void*  SecuredMessageGeneratorC;


/**
 * @brief Structure that holds the permissions (PSID and SSP) to be used for
 * generating a signed SPDU.  The SSP may be a simple byte array or bit-mapped.
 *
 * @param psid (IN) The PSID that must be authorized in the signing certificate.
 * @param isBitmappedSsp (IN) Nonzero indicates bit-mapped SSP; zero indicates simple SSP.
 * @param ssp (IN) The SSP that must be authorized in the sigining certificate.
 * @param sspMask (IN) The mask applied to a bit-mapped SSP indicating the required bits
 *                     that must be authorized in the signing certificate.
 * @pararm sspLen (IN) The length of the SSP and the length of the sspMask if sspMask is not nullptr.
 */
typedef struct
{
    uint32_t       psid;
    uint8_t        isBitmappedSsp;
    uint8_t const *ssp;
    uint8_t const *sspMask;
    uint32_t       sspLen;
} SigningPermissions;


/**
 * @brief Signer Type enum used to potentially override the normal choice of
 * including a certificate or a digest in a signed SPDU.
 */

typedef enum
{
    STO_AUTO,
    STO_DIGEST,
    STO_CERTIFICATE,
} SignerTypeOverride;


/**
 * @brief Sign callback function prototype for use with smg_signAsync().
 *     The signedData pointer that is passed to the callback function is the same pointer that was provided to the smg_signAsync() call.
 */
typedef void (*SignCallback)(
    AEROLINK_RESULT  returnCode,
    void            *userData,
    uint8_t         *signedData,
    uint32_t         signedDataLen);


/**
 * @brief Create a new SecuredMessageGenerator using the specified SecurityContextC
 *
 * @param scHandle (IN) Handle to a previously opened security context.
 * @param smgHandle (OUT) Will point to the handle of the new signed message generator.
 *
 * @return AEROLINK_RESULT
 *
 * @retval WS_SUCCESS
 *     - operation was successful
 *
 * @retval WS_ERR_BAD_ARGS
 *     - scHandle is NULL
 *     - smgHandle is NULL
 *
 * @retval WS_ERR_INVALID_HANDLE
 *     - scHandle does not refer to a valid open security context
 *
 */
AEROLINK_EXPORT AEROLINK_RESULT
smg_new(
    SecurityContextC          scHandle,
    SecuredMessageGeneratorC *smgHandle);


/**
 * @brief Generate an unsecured data from the provided payload.
 *
 * @param smgHandle (IN) Handle to a previously allocated secured message generator.
 * @param appPayload (IN) An application payload.
 * @param appPayloadLen (IN) The length of the application payload.
 *
 * @param unsecuredDataSPDU (OUT) Pointer to buffer in which the unsecured data SPDU will be written.
 * @param unsecuredDataSPDULen (IN/OUT) On input, pointer to the size of the buffer;
 *               on output, will point to the length of the unsecured SPDU in the buffer.
 *
 * @return AEROLINK_RESULT
 *
 * @retval WS_SUCCESS
 *     - operation was successful
 *
 * @retval WS_ERR_BAD_ARGS
 *     - smgHandle is NULL
 *     - payload is NULL
 *     - unsecuredData is NULL
 *     - unsecuredDataLen is NULL
 *     - payloadLen is zero
 *
 * @retval WS_ERR_INVALID_HANDLE
 *     - smgHandle does not refer to a valid secured message generator
 *
 * @retval WS_ERR_NOT_INITIALIZED
 *     - library is not initialized
 *
 * @retval WS_ERR_INVALID_CONTEXT
 *     - the generator is not associated with an open security context
 *
 * @retval WS_ERR_BUFFER_TOO_SMALL
 *     - Output buffer provided is not large enough to hold the generated message
 */
AEROLINK_EXPORT AEROLINK_RESULT
smg_createUnsecuredData(
    SecuredMessageGeneratorC smgHandle,
    uint8_t const * const appPayload,
    uint32_t              appPayloadLen,
    uint8_t       * const unsecuredDataSPDU,
    uint32_t      * const unsecuredDataSPDULen);


/**
 * @brief Generate a signed message from the provided payload and/or externalData for
 * the specified PSID (ITS-AID) and SSP.
 * Any external data is included in the signature calculation but not
 * transported within the encoded signed message. At least one of
 * payload or externalData must be included; otherwise this call will fail.
 *
 * @param smgHandle (IN) Handle to a previously allocated secured message generator.
 * @param permissions (IN) The permissions to be used.
 * @param signerType (IN) The signer type to be used.
 * @param lifetimeOverride (IN) Time in seconds to use for message lifetime instead of value in config file
 * @param payload (IN) The payload to be signed.
 * @param payloadLen (IN) The length of the payload, set to 0 to if no payload will be used.
 * @param isPayloadSPDU (IN) Indicate if the payload is already a 1609.2 SPDU, set to 0 for false.
 *
 * @param externalDataHash (IN) Optional pointer to an external data hash to be included in the message header.
 * @param externalDataHashAlg (IN) The algorithm used to calculate the external hash, set to EDHA_NONE if no external hash will be used.
 *
 * @param encryptionKey (IN) Optional pointer to an AerolinkEncryptionKey to be included in the signed data
 *
 * @param signedDataSPDU (OUT) Pointer to buffer into which the signed SPDU will be written.
 * @param signedDataSPDULen (IN/OUT) On input, pointer to the size of the buffer for the signed SPDU;
 *               on output, will point to the size of the signed SPDU in the buffer.
 *
 * @return AEROLINK_RESULT
 *
 * @retval WS_SUCCESS
 *     - operation was successful
 *
 * @retval WS_ERR_BAD_ARGS
 *     - smgHandle is NULL
 *     - permissions.sspLen is non-zero and permissions.ssp is NULL
 *     - permissions.sspLen is greater than 31 and permissions.ssp is not NULL
 *     - permissions.isBitmappedSsp is non-zero and permissions.sspMask is NULL
 *     - signedDataSPDU is NULL
 *     - signedDataSPDULen is NULL
 *     - payloadLen is non-zero and payload is NULL
 *     - externalDataHashAlg is not EDHA_NONE and externalDataHash is NULL
 *     - payloadLen is 0 and externalDataHashAlg is EDHA_NONE
 *     - encryptionKey is not valid, if present
 *
 * @retval WS_ERR_INVALID_HANDLE
 *     - smgHandle does not refer to a valid secured message generator
 *
 * @retval WS_ERR_INVALID_CONTEXT
 *     - the secured message generator is not associated with an open security context
 *
 * @retval WS_ERR_INVALID_PSID
 *     - psid is not valid value
 *
 * @retval WS_ERR_PSID_NOT_IN_CONTEXT
 *     - the security context designated by scHandle does not contain the specified PSID/SSP
 *
 * @retval WS_ERR_NO_CERTIFICATE
 *     - no valid certificate and signing key for the PSID/SSP were found
 *
 * @retval WS_ERR_CRYPTO_INTERNAL
 *     - crypto signing operation failed
 *
 * @retval WS_ERR_BUFFER_TOO_SMALL
 *     - Output buffer provided is not large enough to hold the generated message
 */
AEROLINK_EXPORT AEROLINK_RESULT
smg_sign(
    SecuredMessageGeneratorC            smgHandle,
    SigningPermissions const            permissions,
    SignerTypeOverride                  signerType,
    uint32_t                            lifetimeOverride,
    uint8_t const               * const payload,
    uint32_t                            payloadLen,
    uint8_t                             isPayloadSPDU,
    uint8_t const               * const externalDataHash,
    ExternalDataHashAlg                 externalDataHashAlg,
    AerolinkEncryptionKey const * const encryptionKey,
    uint8_t                     * const signedDataSPDU,
    uint32_t                    * const signedDataSPDULen);


/**
 * @brief Generate a signed message asynchronously from the provided payload and/or
 * externalData for the specified PSID (ITS-AID) and SSP.
 *
 * Any external data is included in the signature calculation but not
 * transported within the encoded signed message. At least one of
 * payload or externalData must be included; otherwise this call will fail.
 *
 * @param smgHandle (IN) Handle to a previously allocated secured message generator.
 * @param permissions (IN) The permissions to be used.
 * @param signerType (IN) The signer type to be used.
 * @param lifetimeOverride (IN) Time in seconds to use for message lifetime instead of value in config file
 * @param payload (IN) The payload to be signed.
 * @param payloadLen (IN) The length of the payload, set to 0 to if no payload will be used.
 * @param isPayloadSPDU (IN) Indicate if the payload is already a 1609.2 SPDU, set to 0 for false.
 *
 * @param externalDataHash (IN) Optional pointer to an external data hash to be included in the message header.
 * @param externalDataHashAlg (IN) The algorithm used to calculate the external hash, set to EDHA_NONE if no external hash will be used.
 *
 * @param encryptionKey (IN) Optional pointer to an AerolinkEncryptionKey to be included in the signed data
 *
 * @param signedDataSPDU (OUT) Pointer to buffer into which the signed SPDU will be written.
 * @param signedDataSPDULen (IN/OUT) On input, pointer to the size of the buffer for the signed SPDU;
 *               on output, will point to the size of the signed SPDU in the buffer.
 *
 * @param userCallbackData  (IN) user supplied data that will be returned in the callback.
 * @param callbackFunction  (IN) callback function to be called when message signing has completed.
 *
 *
 * @return AEROLINK_RESULT
 *
 * @retval WS_SUCCESS
 *     - operation was been queued
 *
 * @retval WS_ERR_BAD_ARGS
 *     - smgHandle is NULL
 *     - permissions.sspLen is non-zero and permissions.ssp is NULL
 *     - permissions.sspLen is greater than 31 and permissions.ssp is not NULL
 *     - permissions.isBitmappedSsp is non-zero and permissions.sspMask is NULL
 *     - signedDataSPDU is NULL
 *     - signedDataSPDULen is NULL
 *     - payloadLen is non-zero and payload is NULL
 *     - externalDataHashAlg is not EDHA_NONE and externalDataHash is NULL
 *     - payloadLen is 0 and externalDataHashAlg is EDHA_NONE
 *     - encryptionKey is not valid, if present
 *     - callbackFunction is NULL
 *
 * @retval WS_ERR_INVALID_HANDLE
 *     - smgHandle does not refer to a valid secured message generator
 *
 * @retval WS_ERR_INVALID_CONTEXT
 *     - the secured message generator is not associated with an open security context
 *
 * @retval WS_ERR_INVALID_PSID
 *     - psid is not valid value
 *
 * @retval WS_ERR_PSID_NOT_IN_CONTEXT
 *     - the security context designated by scHandle does not contain the specified PSID/SSP
 *
 * @retval WS_ERR_NO_CERTIFICATE
 *     - no valid certificate and signing key for the PSID/SSP were found
 *
 * @retval WS_ERR_CRYPTO_INTERNAL
 *     - crypto signing operation failed
 *
 * @retval WS_ERR_BUFFER_TOO_SMALL
 *     - Output buffer provided is not large enough to hold the generated message
 */
AEROLINK_EXPORT AEROLINK_RESULT
smg_signAsync(
    SecuredMessageGeneratorC            smgHandle,
    SigningPermissions const            permissions,
    SignerTypeOverride                  signerType,
    uint32_t                            lifetimeOverride,
    uint8_t const               * const payload,
    uint32_t                            payloadLen,
    uint8_t                             isPayloadSPDU,
    uint8_t const               * const externalDataHash,
    ExternalDataHashAlg                 externalDataHashAlg,
    AerolinkEncryptionKey const * const encryptionKey,
    uint8_t                     * const signedDataSPDU,
    uint32_t                            signedDataSPDULen,
    void const                  * const userCallbackData,
    SignCallback                        callbackFunction);

/**
 * @brief Encrypt data using the provided encryption keys.
 *
 * @param smgHandle (IN) Handle to a previously allocated secured message generator.
 * @param recipients (IN) Array of recipient encryption keys.
 * @param numRecipients  (IN) Number of elements in the recipients array.
 * @param payload (IN) The payload to be encrypted.
 * @param payloadLen (IN) The length of the payload.
 * @param isPayloadSPDU (IN) Indicate if the payload is already a 1609.2 SPDU, set to 0 for false.
 * @param encryptedDataSPDU (IN) Pointer to buffer into which the encrypted SPDU will be written.
 * @param encryptedDataSPDULen (IN/OUT) On input, pointer to the size of the buffer for the encrypted SPDU;
 *               on output, will point to the size of the encrypted SPDU in the buffer.
 *
 * @return AEROLINK_RESULT
 *
 * @retval WS_SUCCESS
 *     - operation was been queued
 *
 * @retval WS_ERR_BAD_ARGS
 *     - smgHandle is NULL
 *     - all recipients are null
 *     - numRecipients is zero
 *     - payload is NULL
 *     - payloadLen is 0
 *     - encryptedData is NULL
 *     - encryptedDataLen is NULL
 *
 * @retval WS_ERR_INVALID_HANDLE
 *     - smgHandle does not refer to a valid secured message generator
 *
 * @retval WS_ERR_INVALID_CONTEXT
 *     - the secured message generator is not associated with an open security context
 *
 * @retval WS_ERR_BUFFER_TOO_SMALL
 *     - Output buffer provided is not large enough to hold the encrypted data
 */
AEROLINK_EXPORT AEROLINK_RESULT
smg_encrypt(
        SecuredMessageGeneratorC            smgHandle,
        AerolinkEncryptionKey const * const recipients[],
        uint32_t                            numRecipients,
        uint8_t const               * const payload,
        uint32_t                            payloadLen,
        uint8_t                             isPayloadSPDU,
        uint8_t                     * const encryptedDataSPDU,
        uint32_t                    * const encryptedDataSPDULen);

 /**
 * @brief Destroys a SecuredMessageGeneratorC that was created by
 * smg_new(). After this call the handle will no longer be valid.
 *
 * @param smgHandle (IN) Handle to a previously allocated secured message generator.

 * @return AEROLINK_RESULT
 *
 * @retval WS_SUCCESS
 *     - operation was successful
 *
 * @retval WS_ERR_BAD_ARGS
 *     - smgHandle is NULL
 *
 * @retval WS_ERR_INVALID_HANDLE
 *     - smgHandle does not refer to a valid secured message generator.
 *
 */
AEROLINK_EXPORT AEROLINK_RESULT
smg_delete(
    SecuredMessageGeneratorC smgHandle);



#ifdef __cplusplus
}
#endif


#endif /* SECURED_MESSAGE_GENERATOR_C_H */
