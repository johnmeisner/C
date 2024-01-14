#ifndef SECURED_MESSAGE_PARSER_C_H
#define SECURED_MESSAGE_PARSER_C_H

/**
 * @file SecuredMessageParserC.h
 * @brief C interface to the SecuredMessageParser.
 */

#include <stdint.h>
#include "aerolink_api.h"
#include "SecurityContextC.h"
#include "Encryption.h"
#include "ExternalDataHashAlg.h"
#include "PayloadType.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Handle to an allocated Secured Message Parser.
 */
typedef void* SecuredMessageParserC;


/**
 * @brief Callback function prototype for use with smp_verifySignaturesAsync().
 */
typedef void (*ValidateCallback)(AEROLINK_RESULT returnCode, void *userData);


/**
 * @brief Create a new SecuredMessageParser using the specified SecurityContextC.
 *
 * @param scHandle (IN) Handle to a previously opened security context.
 * @param smpHandle (OUT) Will point to the handle or the new secured message parser.
 *
 * @return AEROLINK_RESULT
 *
 * @retval WS_SUCCESS
 *     - operation was successful
 *
 * @retval WS_ERR_NOT_INITIALIZED
 *     - library is not initialized
 *
 * @retval WS_ERR_BAD_ARGS
 *     - scHandle is NULL
 *     - smpHandle is NULL
 *
 * @retval WS_ERR_INVALID_HANDLE
 *     - scHandle does not refer to a valid open security context
 *
 */
AEROLINK_EXPORT AEROLINK_RESULT
smp_new(
        SecurityContextC              scHandle,
        SecuredMessageParserC * const smpHandle);


/**
 * @brief Destroys a SecuredMessageParserC that was created by smp_new().
 *  After this call the handle will no longer be valid.
 *
 * @param smpHandle (IN) Handle to a previously allocated secured message parser.
 *
 * @return AEROLINK_RESULT
 *
 * @retval WS_SUCCESS
 *     - operation was successful
 *
 * @retval WS_ERR_NOT_INITIALIZED
 *     - library is not initialized
 *
 * @retval WS_ERR_BAD_ARGS
 *     - smpHandle is NULL
 *
 * @retval WS_ERR_INVALID_HANDLE
 *     - smpHandle does not refer to a valid secured message parser
 */
AEROLINK_EXPORT AEROLINK_RESULT
smp_delete(
        SecuredMessageParserC smpHandle);


/**
 * @brief Parse and extract data from the provided secured message.
 *     This function must be called before any of the other functions in this file.
 *
 *     This function returns the type of secured message and the application payload.
 *
 * @param smpHandle (IN) Handle to a previously allocated secured message parser.
 * @param spdu (IN) The secured message to be parsed.
 * @param spduLen (IN) The length of the secured message to be parsed.
 * @param spduType (OUT) Will point to the type of secured message being parsed.
 * @param payload (OUT) Will point to the address of the payload in the secured message.
 * @param payloadLen (OUT) Will point to the size of the payload in the secured message.
 *
 * @return AEROLINK_RESULT
 *
 * @retval WS_SUCCESS
 *     - operation was successful
 *
 * @retval WS_ERR_NOT_INITIALIZED
 *     - library is not initialized
 *
 * @retval WS_ERR_BAD_ARGS
 *     - smgHandle is NULL
 *     - message is NULL
 *     - messageLen is zero
 *     - type is NULL
 *     - payload is NULL
 *     - payloadLen is NULL
 *
 * @retval WS_ERR_INVALID_HANDLE
 *     - smpHandle does not refer to a valid secured message parser
 *
 * @retval WS_ERR_INVALID_CONTEXT
 *     - secured message parser is not associated with an open security context
 *
 * @retval WS_ERR_PARSE_FAIL
 *     - the secured message is malformed
 *
 * @retval WS_ERR_BUFFER_TOO_SMALL
 *     - the payload buffer is not large enough to hold the message payload
 *
 * @retval WS_ERR_NOT_SUPPORTED
 *     - the message security type of the secured message is currently not supported
 *
 * @retval WS_ERR_NO_KEY_AVAILABLE
 *     - No private decryption key can be found for the message.
 */
AEROLINK_EXPORT AEROLINK_RESULT
smp_extract(
    SecuredMessageParserC  smpHandle,
    uint8_t const *        spdu,
    uint32_t               spduLen,
    PayloadType *          spduType,
    uint8_t const **       payload,
    uint32_t *             payloadLen,
    PayloadType *          payloadType,
    uint8_t const **       externalDataHash,
    ExternalDataHashAlg *  externalDataHashAlg);


/**
 * @brief Get the PSID from the security headers of the secured message.
 *
 * @param smpHandle (IN) Handle to a previously allocated secured message parser.
 * @param psid (OUT) Will point to the PSID found in the security headers.
 *
 * @return AEROLINK_RESULT
 *
 * @retval WS_SUCCESS
 *     - operation was successful
 *
 * @retval WS_ERR_NOT_INITIALIZED
 *     - library is not initialized
 *
 * @retval WS_ERR_BAD_ARGS
 *     - smpHandle is NULL
 *     - psid is NULL
 *
 * @retval WS_ERR_INVALID_HANDLE
 *     - smpHandle does not refer to a valid secured message parser
 *
 * @retval WS_ERR_INVALID_CONTEXT
 *     - secured message parser is not associated with an open security context
 *
 * @retval WS_ERR_DATA_NOT_EXTRACTED
 *     - smp_extract() has not passed for this secured message
 *
 * @retval WS_ERR_NOT_SUPPORTED_FOR_THIS_MESSAGE_SECURITY_TYPE
 *     - this function does not support this message's security type
 */
AEROLINK_EXPORT AEROLINK_RESULT
smp_getPsid(
    SecuredMessageParserC  smpHandle,
    uint32_t       * const psid);

/**
 * @brief Get the Hashed Id8 from the security headers of the secured message.
 *
 * @param smpHandle (IN) Handle to a previously allocated secured message parser.
 *
 * @param signingCertifiateId (OUT) Will point to the address of the first byte of the hashed id of the messages signing certificate.
 *      - This pointer is NOT to be freed by the caller.
 *      - The address of the signingCertifiateId will only be valid until smp_delete() or smp_extract() is called with the same smpHandle.
 *
 * @return AEROLINK_RESULT
 *
 * @retval WS_SUCCESS
 *     - operation was successful
 *
 * @retval WS_ERR_NOT_INITIALIZED
 *     - library is not initialized
 *
 * @retval WS_ERR_BAD_ARGS
 *     - smpHandle is NULL
 *     - signingCertifiateId is NULL
 *
 * @retval WS_ERR_INVALID_HANDLE
 *     - smpHandle does not refer to a valid secured message parser
 *
 * @retval WS_ERR_INVALID_CONTEXT
 *     - secured message parser is not associated with an open security context
 *
 * @retval WS_ERR_DATA_NOT_EXTRACTED
 *     - smp_extract() has not passed for this secured message
 *
 * @retval WS_ERR_NO_CERTIFICATE
 *     - the certificate referenced in the secured message is not in the certificate store
 *
 * @retval WS_ERR_NOT_SUPPORTED_FOR_THIS_MESSAGE_SECURITY_TYPE
 *     - this function does not support this message's security type
 */
AEROLINK_EXPORT AEROLINK_RESULT
smp_getSigningCertHashedId(
    SecuredMessageParserC  smpHandle,
    uint8_t const  **      signingCertificateId);

/**
 * @brief Get the Service Specific Permissions (SSP) from the certificate associated with this signed message.
 *
 * @param smpHandle (IN) Handle to a previously allocated SecuredMessageParser.
 * @param ssp (OUT) Will point to the address of the first byte of the SSP.
 *     This pointer is NOT to be freed by the caller.
 *     The address of the SSP will only be valid until smp_delete() or smp_extract() is called with the same smpHandle.
 * @param sspLen (OUT) Will point to the length of the SSP.
 *
 * @return AEROLINK_RESULT
 *
 * @retval WS_SUCCESS
 *     - operation was successful
 *
 * @retval WS_ERR_NOT_INITIALIZED
 *     - library is not initialized
 *
 * @retval WS_ERR_BAD_ARGS
 *     - smpHandle is NULL
 *     - ssp is NULL
 *     - sspLen is NULL
 *
 * @retval WS_ERR_INVALID_HANDLE
 *     - smpHandle does not refer to a valid secured message parser
 *
 * @retval WS_ERR_INVALID_CONTEXT
 *     - secured message parser is not associated with an open security context
 *
 * @retval WS_ERR_DATA_NOT_EXTRACTED
 *     - smp_extract() has not passed for this secured message
 *
 * @retval WS_ERR_NO_CERTIFICATE
 *     - the certificate referenced in the secured message is not in the certificate store
 *
 * @retval WS_ERR_NOT_SUPPORTED_FOR_THIS_MESSAGE_SECURITY_TYPE
 *     - this function does not support this message's security type
 */
AEROLINK_EXPORT AEROLINK_RESULT
smp_getServiceSpecificPermissions(
    SecuredMessageParserC   smpHandle,
    uint8_t const  **       ssp,
    uint32_t        * const sspLen);


/**
 * @brief Returns the generation time if present.
 *
 * @param smpHandle (IN) Handle to a previously allocated secured message parser.
 * @param time (OUT) Will point to the generation time.
 * @param logStdDev (OUT) Will point to the log of the standard deviation for the generation time.
 *
 * @return AEROLINK_RESULT
 *
 * @retval WS_SUCCESS
 *     - operation was successful
 *
 * @retval WS_ERR_NOT_INITIALIZED
 *     - library is not initialized
 *
 * @retval WS_ERR_BAD_ARGS
 *     - smpHandle is NULL
 *     - time is NULL
 *     - logStdDev is NULL
 *
 * @retval WS_ERR_INVALID_HANDLE
 *     - smpHandle does not refer to a valid secured message parser
 *
 * @retval WS_ERR_INVALID_CONTEXT
 *     - secured message parser is not associated with an open security context
 *
 * @retval WS_ERR_NO_GENERATION_TIME
 *     - the security headers in the secured message do not contain the generation time and it has not been set with smp_setGenerationTime()
 *
 * @retval WS_ERR_DATA_NOT_EXTRACTED
 *     - smp_extract() has not passed for this secured message
 *
 * @retval WS_ERR_NOT_SUPPORTED_FOR_THIS_MESSAGE_SECURITY_TYPE
 *     - this function does not support this message's security type
 */
AEROLINK_EXPORT AEROLINK_RESULT
smp_getGenerationTime(
    SecuredMessageParserC smpHandle,
    uint64_t *const generationTime);

/**
 * @brief Returns the expiry time if present.
 *
 * @param smpHandle  (IN) Handle to a previously allocated secured message parser.
 * @param expiryTime (OUT) Will point to the generation time.
 *
 * @return AEROLINK_RESULT
 *
 * @retval WS_SUCCESS
 *     - operation was successful
 *
 * @retval WS_ERR_NOT_INITIALIZED
 *     - library is not initialized
 *
 * @retval WS_ERR_BAD_ARGS
 *     - smpHandle is NULL
 *     - time is NULL
 *
 * @retval WS_ERR_INVALID_HANDLE
 *     - smpHandle does not refer to a valid secured message parser
 *
 * @retval WS_ERR_INVALID_CONTEXT
 *     - secured message parser is not associated with an open security context
 *
 * @retval WS_ERR_NO_EXPIRY_TIME
 *     - the security headers in the secured message do not contain the expiry time and it has not been set with smp_setExpiryTime()
 *
 * @retval WS_ERR_DATA_NOT_EXTRACTED
 *     - smp_extract() has not passed for this secured message
 *
 * @retval WS_ERR_NOT_SUPPORTED_FOR_THIS_MESSAGE_SECURITY_TYPE
 *     - this function does not support this message's security type
 */
AEROLINK_EXPORT AEROLINK_RESULT
smp_getExpiryTime(
    SecuredMessageParserC  smpHandle,
    uint64_t       * const expiryTime);


/**
 * @brief Returns the generation location if present.
 *
 * @param smpHandle (IN) Handle to a previously allocated secured message parser.
 * @param latitude (OUT) Will point to the latitude.
 * @param longitude (OUT) Will point to the longitude.
 * @param elevation (OUT) Will point to the elevation.
 *
 * @return AEROLINK_RESULT
 *
 * @retval WS_SUCCESS
 *     - operation was successful
 *
 * @retval WS_ERR_NOT_INITIALIZED
 *     - library is not initialized
 *
 * @retval WS_ERR_BAD_ARGS
 *     - smpHandle is NULL
 *     - latitude is NULL
 *     - longitude is NULL
 *     - elevation is NULL
 *
 * @retval WS_ERR_INVALID_HANDLE
 *     - smpHandle does not refer to a valid secured message parser
 *
 * @retval WS_ERR_INVALID_CONTEXT
 *     - secured message parser is not associated with an open security context
 *
 * @retval WS_ERR_NO_GENERATION_LOCATION
 *     - the security headers in the secured message do not contain the generation location and it has not been set with smp_setGenerationLocation()
 *
 * @retval WS_ERR_DATA_NOT_EXTRACTED
 *     - smp_extract() has not passed for this secured message
 *
 * @retval WS_ERR_NOT_SUPPORTED_FOR_THIS_MESSAGE_SECURITY_TYPE
 *     - this function does not support this message's security type
 */
AEROLINK_EXPORT AEROLINK_RESULT
smp_getGenerationLocation(
    SecuredMessageParserC smpHandle,
    int32_t *       const latitude,
    int32_t *       const longitude,
    uint16_t *      const elevation);


/**
 * @brief Set the generation time of the secured message as long as it is not already set.
 *
 * If the security headers do not contain a generation time and the security profile requires the library to perform
 * the generation time validation checks, then the generation time must be provided using this method.
 *
 * This function needs to be called after a successful call to smp_extract() and before smp_validate().
 * When smp_extract() is called it resets any previously set value.
 *
 * @param smpHandle (IN) Handle to a previously allocated secured message parser.
 * @param time (IN) Generation time to be set.
 *
 * @return AEROLINK_RESULT
 *
 * @retval WS_SUCCESS
 *     - operation was successful
 *
 * @retval WS_ERR_NOT_INITIALIZED
 *     - library is not initialized
 *
 * @retval WS_ERR_BAD_ARGS
 *     - smpHandle is NULL
 *
 * @retval WS_ERR_INVALID_HANDLE
 *     - smpHandle does not refer to a valid secured message parser
 *
 * @retval WS_ERR_INVALID_CONTEXT
 *     - secured message parser is not associated with an open security context
 *
 * @retval WS_ERR_DATA_NOT_EXTRACTED
 *     - smp_extract() has not passed for this secured message
 *
 * @retval WS_ERR_VALUE_ALREADY_SET
 *     - security header in the secured message contains a generation time which cannot be overridden
 *     - smp_setGenerationTime() was called previously with a different value
 *
 * @retval WS_ERR_NOT_SUPPORTED_FOR_THIS_MESSAGE_SECURITY_TYPE
 *     - this function does not support this message's security type
*/
AEROLINK_EXPORT AEROLINK_RESULT
smp_setGenerationTime(
        SecuredMessageParserC smpHandle,
        uint64_t              generationTime);


/**
 * @brief Set the expiry time of the secured message as long as it is not already set.
 *
 * If the security headers do not contain an expiry time and the security profile requires the library to perform
 * the expiry time validation checks, then the expiry time must be provided using this method.
 *
 * This function needs to be called after a successful call to smp_extract() and before smp_validate().
 * When smp_extract() is called it resets any previously set value.
 *
 * @param smpHandle (IN) Handle to a previously allocated secured message parser.
 * @param time (IN) Expiry time to be set.
 *
 * @return AEROLINK_RESULT
 *
 * @retval WS_SUCCESS
 *     - operation was successful
 *
 * @retval WS_ERR_NOT_INITIALIZED
 *     - library is not initialized
 *
 * @retval WS_ERR_BAD_ARGS
 *     - smpHandle is NULL
 *
 * @retval WS_ERR_INVALID_HANDLE
 *     - smpHandle does not refer to a valid secured message parser
 *
 * @retval WS_ERR_INVALID_CONTEXT
 *     - secured message parser is not associated with an open security context
 *
 * @retval WS_ERR_DATA_NOT_EXTRACTED
 *     - smp_extract() has not passed for this secured message
 *
 * @retval WS_ERR_VALUE_ALREADY_SET
 *     - security header in the secured message contains an expiry time which cannot be overridden
 *     - smp_setExpiryTime() was called previously with a different value
 *
 * @retval WS_ERR_NOT_SUPPORTED_FOR_THIS_MESSAGE_SECURITY_TYPE
 *     - this function does not support this message's security type
 */
AEROLINK_EXPORT AEROLINK_RESULT
smp_setExpiryTime(
        SecuredMessageParserC smpHandle,
        uint64_t              expiryTime);


/**
 * @brief Set the generation location of the secured message as long as it is not already set.
 *
 * If the security headers do not contain a generation location and the security profile requires the library to perform
 * the generation location validation checks, then the generation location must be provided using this method.
 *
 * This function needs to be called after a successful call to smp_extract() and before smp_validate().
 * When smp_extract() is called it resets any previously set value.
 *
 * @param smpHandle (IN) Handle to a previously allocated secured message parser.
 * @param latitude (IN) Latitude to be set.
 * @param longitude (IN) Longitude to be set.
 * @param elevation (IN) Elevation to be set.
 *
 * @return AEROLINK_RESULT
 *
 * @retval WS_SUCCESS
 *     - operation was successful
 *
 * @retval WS_ERR_NOT_INITIALIZED
 *     - library is not initialized
 *
 * @retval WS_ERR_BAD_ARGS
 *     - smpHandle is NULL
 *     - latitude is an invalid value
 *     - longitude is an invalid value
 *
 * @retval WS_ERR_INVALID_HANDLE
 *     - smpHandle does not refer to a valid secured message parser
 *
 * @retval WS_ERR_INVALID_CONTEXT
 *     - secured message parser is not associated with an open security context
 *
 * @retval WS_ERR_DATA_NOT_EXTRACTED
 *     - smp_extract() has not passed for this secured message
 *
 * @retval WS_ERR_VALUE_ALREADY_SET
 *     - security header in the secured message contains a generation location which cannot be overridden
 *     - smp_setGenerationLocation() was called previously with different values
 *
 * @retval WS_ERR_NOT_SUPPORTED_FOR_THIS_MESSAGE_SECURITY_TYPE
 *     - this function does not support this message's security type
 */
AEROLINK_EXPORT AEROLINK_RESULT
smp_setGenerationLocation(
        SecuredMessageParserC smpHandle,
        int32_t   latitude,
        int32_t   longitude,
        uint16_t  elevation);


/**
 * @brief Perform all relevance checks for the most recently extracted message.
 *
 * @param smpHandle (IN) Handle to a previously allocated secured message parser.
 *
 * @return AEROLINK_RESULT
 *
 * @retval WS_SUCCESS
 *     - operation was successful
 *
 * @retval WS_ERR_NOT_INITIALIZED
 *     - library is not initialized
 *
 * @retval WS_ERR_BAD_ARGS
 *     - smpHandle is NULL
 *     - externalDataLen is non-zero and externalData is NULL
 *
 * @retval WS_ERR_INVALID_HANDLE
 *     - smgHandle does not refer to a valid secured message parser
 *
 * @retval WS_ERR_INVALID_CONTEXT
 *     - secured message parser is not associated with an open security context
 *
 * @retval WS_ERR_DATA_NOT_EXTRACTED
 *     - smp_extract() has not passed for this secured message
 *
 * @retval WS_ERR_NOT_SUPPORTED_FOR_THIS_MESSAGE_SECURITY_TYPE
 *     - this function does not support this message's security type
 *
 * @retval WS_ERR_MESSAGE_REPLAY
 *     - this message is a duplicate of a previously received message
 *
 * @retval WS_ERR_NO_GENERATION_TIME
 *     - no message generation time is set and it is required for validation
 *
 * @retval WS_ERR_MESSAGE_TOO_OLD
 *     - message generation time is too far in the past
 *
 * @retval WS_ERR_MESSAGE_IN_FUTURE
 *     - message generation time is in the future
 *
 * @retval WS_ERR_NO_EXPIRY_TIME
 *     - no message expiry time is set and it is required for validation
 *
 * @retval WS_ERR_MESSAGE_EXPIRED
 *     - the current time is after the message's expiry time
 *
 * @retval WS_ERR_MESSAGE_EXPIRY_BEFORE_GENERATION
 *     - the message's expiry time is before its generation time
 *
 * @retval WS_ERR_NO_GENERATION_LOCATION
 *     - no message generation location is set and it is required for validation
 *
 * @retval WS_ERR_MESSAGE_FROM_TOO_FAR_AWAY
 *     - message generation location is beyond the distance tolerance
 */
AEROLINK_EXPORT AEROLINK_RESULT
smp_checkRelevance(
    SecuredMessageParserC  smpHandle);



/**
 * @brief Perform all consistency and certificate-chain checks for the most
 * recently extracted message.
 *
 * @param smpHandle (IN) Handle to a previously allocated secured message parser.
 *
 * @return AEROLINK_RESULT
 *
 * @retval WS_SUCCESS
 *     - operation was successful
 *
 * @retval WS_ERR_NOT_INITIALIZED
 *     - library is not initialized
 *
 * @retval WS_ERR_BAD_ARGS
 *     - smpHandle is NULL
 *     - externalDataLen is non-zero and externalData is NULL
 *
 * @retval WS_ERR_INVALID_HANDLE
 *     - smgHandle does not refer to a valid secured message parser
 *
 * @retval WS_ERR_INVALID_CONTEXT
 *     - secured message parser is not associated with an open security context
 *
 * @retval WS_ERR_DATA_NOT_EXTRACTED
 *     - smp_extract() has not passed for this secured message
 *
 * @retval WS_ERR_NOT_SUPPORTED_FOR_THIS_MESSAGE_SECURITY_TYPE
 *     - this function does not support this message's security type
 *
 * @retval WS_ERR_NO_CERTIFICATE
 *     - at least one certificate in the certificate chain is not in the certificate store
 *
 * @retval WS_ERR_CERTIFICATES_INCONSISTENT
 *     - data in the certificate chain is not consistent among certificates
 *
 * @retval WS_ERR_CERTIFICATE_REVOKED
 *     - a certificate in the certificate chain was signed after its signing certificate was revoked
 *     - the message was signed after its signing certificate was revoked
 *
 * @retval WS_ERR_NO_GENERATION_TIME
 *     - no message generation time is set and it is required for validation
 *
 * @retval WS_ERR_MESSAGE_GEN_BEFORE_CERT_VALIDITY_PERIOD
 *     - message generation time is before the signing certificate was valid
 *
 * @retval WS_ERR_MESSAGE_GEN_AFTER_CERT_VALIDITY_PERIOD
 *     - message generation time is after the signing certificate expired
 *
 * @retval WS_ERR_NO_EXPIRY_TIME
 *     - no message expiry time is set and it is required for validation
 *
 * @retval WS_ERR_MESSAGE_EXP_BEFORE_CERT_VALIDITY_PERIOD
 *     - message expiration time is before the signing certificate was valid
 *
 * @retval WS_ERR_MESSAGE_EXP_AFTER_CERT_VALIDITY_PERIOD
 *     - message expiration time is after the signing certificate expired
 *
 * @retval WS_ERR_NO_GENERATION_LOCATION
 *     - no message generation location is set and it is required for validation
 *
 * @retval WS_ERR_MESSAGE_SIGNED_OUTSIDE_REGION
 *     - message generation location is outside the signing certificate's geographic region
 *
 * @retval WS_ERR_UNAUTHORIZED_CERTIFICATE
 *     - PSID in the message is not contained in the signing certificate
 *
 * @retval WS_ERR_UNAUTHORIED_CERTIFICATE_TYPE
 *     - the signing certificate does not have a holder type compatible with the message type
 *
 * @retval WS_ERR_INVALID_SIGNER_ID_TYPE
 *     - signer id type not allowed for this type of message
 */
AEROLINK_EXPORT AEROLINK_RESULT
smp_checkConsistency(
    SecuredMessageParserC  smpHandle);



/**
 * @brief Verify signatures for the most recently extracted message.
 *
 * @param smpHandle (IN) Handle to a previously allocated secured message parser.
 *
 * @return AEROLINK_RESULT
 *
 * @retval WS_SUCCESS
 *     - operation was successful
 *
 * @retval WS_ERR_NOT_INITIALIZED
 *     - library is not initialized
 *
 * @retval WS_ERR_BAD_ARGS
 *     - smpHandle is NULL
 *
 * @retval WS_ERR_INVALID_HANDLE
 *     - smgHandle does not refer to a valid secured message parser
 *
 * @retval WS_ERR_INVALID_CONTEXT
 *     - secured message parser is not associated with an open security context
 *
 * @retval WS_ERR_DATA_NOT_EXTRACTED
 *     - smp_extract() has not passed for this secured message
 *
 * @retval WS_ERR_CONSISTENCY_NOT_CHECKED
 *     - smp_checkConsistency() has not been called for this secured message
 *
 * @retval WS_ERR_NOT_SUPPORTED_FOR_THIS_MESSAGE_SECURITY_TYPE
 *     - this function does not support this message's security type
 *
 * @retval WS_ERR_SIGNATURE_VERIFICATION_FAILED
 *     - cryptographic verification of the signature failed
 */
AEROLINK_EXPORT AEROLINK_RESULT
smp_verifySignatures(
    SecuredMessageParserC smpHandle);



/**
 * @brief Verify signatures asynchronously for the most recently extracted message.
 *
 * @param smpHandle (IN) Handle to a previously allocated secured message parser.
 * @param userData  (IN) User supplied data to is passed to the users callback function.
 * @param callbackFunction (IN) User supplied function to be called when the operation completes.
 *
 * @return AEROLINK_RESULT
 *
 * @retval WS_SUCCESS
 *     - operation was successful
 *
 * @retval WS_ERR_NOT_INITIALIZED
 *     - library is not initialized
 *
 * @retval WS_ERR_BAD_ARGS
 *     - smpHandle is NULL
 *
 * @retval WS_ERR_INVALID_HANDLE
 *     - smgHandle does not refer to a valid secured message parser
 *
 * @retval WS_ERR_INVALID_CONTEXT
 *     - secured message parser is not associated with an open security context
 *
 * @retval WS_ERR_CRYPTO_QUEUE_FULL
 *     - verification operation queue is full
 *
 * @retval WS_ERR_DATA_NOT_EXTRACTED
 *     - smp_extract() has not passed for this secured message
 *
 * @retval WS_ERR_CONSISTENCY_NOT_CHECKED
 *     - smp_checkConsistency() has not been called for this secured message
 *
 * @retval WS_ERR_NOT_SUPPORTED_FOR_THIS_MESSAGE_SECURITY_TYPE
 *     - this function does not support this message's security type
 */
AEROLINK_EXPORT AEROLINK_RESULT
smp_verifySignaturesAsync(
    SecuredMessageParserC smpHandle,
    void *userData,
    ValidateCallback callbackFunction);



/**
 * @brief Verify signatures asynchronously for the most recently extracted message with the priority indicated.
 *
 * @param smpHandle (IN) Handle to a previously allocated secured message parser.
 * @param priority (IN) The priority assigned to this verification.
 * @param userData  (IN) User supplied data to is passed to the users callback function.
 * @param callbackFunction (IN) User supplied function to be called when the operation completes.
 *
 * @return AEROLINK_RESULT
 *
 * @retval WS_SUCCESS
 *     - operation was successful
 *
 * @retval WS_ERR_NOT_INITIALIZED
 *     - library is not initialized
 *
 * @retval WS_ERR_BAD_ARGS
 *     - smpHandle is NULL
 *
 * @retval WS_ERR_INVALID_HANDLE
 *     - smgHandle does not refer to a valid secured message parser
 *
 * @retval WS_ERR_INVALID_CONTEXT
 *     - secured message parser is not associated with an open security context
 *
 * @retval WS_ERR_CRYPTO_QUEUE_FULL
 *     - verification operation queue is full
 *
 * @retval WS_ERR_DATA_NOT_EXTRACTED
 *     - smp_extract() has not passed for this secured message
 *
 * @retval WS_ERR_CONSISTENCY_NOT_CHECKED
 *     - smp_checkConsistency() has not been called for this secured message
 *
 * @retval WS_ERR_NOT_SUPPORTED_FOR_THIS_MESSAGE_SECURITY_TYPE
 *     - this function does not support this message's security type
 */
AEROLINK_EXPORT AEROLINK_RESULT
smp_verifySignaturesAsyncPriority
    (SecuredMessageParserC smpHandle,
     uint8_t priority,
     void *userData,
     ValidateCallback callbackFunction);

AEROLINK_EXPORT AEROLINK_RESULT
smp_getAssuranceLevel(
        SecuredMessageParserC smpHandle,
        uint8_t * const       assuranceLevel);

/**
 * @brief Allows the caller to provide the PSID derived from some context outside the signed SPDU.
 *
 * @param smpHandle (IN) Handle to a previously allocated secured message parser.
 * @param psid      (IN) Value to be set
 *
 * @return AEROLINK_RESULT
 *
 * @retval WS_SUCCESS
 *     - operation was successful
 *
 * @retval WS_ERR_NOT_INITIALIZED
 *     - library is not initialized
 *
 * @retval WS_ERR_INVALID_HANDLE
 *     - smpHandle does not refer to a valid secured message parser
 *
 * @retval WS_ERR_INVALID_CONTEXT
 *     - secured message parser is not associated with an open security context
 */
AEROLINK_EXPORT AEROLINK_RESULT
smp_setPsidFromContext(
    SecuredMessageParserC   smpHandle,
    uint32_t                psid);

/**
 * @brief Returns the public encryption key contained in an end-entity certificate associated with the SignedData PDU.
 *
 * @param smpHandle (IN) Handle to a previously allocated secured message parser.
 * @param encryptionKey (OUT) Pointer to an AerolinkEncryptionKey structure containing the key.
 *                            Aerolink dynamically allocates the memory for blob and the caller is responsible for
 *                            freeing the memory.
 * @return AEROLINK_RESULT
 *
 * @retval WS_SUCCESS
 *     - operation was successful
 *
 * @retval WS_ERR_NOT_INITIALIZED
 *     - library is not initialized
 *
 * @retval WS_ERR_INVALID_HANDLE
 *     - smpHandle does not refer to a valid secured message parser
 *
 * @retval WS_ERR_INVALID_CONTEXT
 *     - secured message parser is not associated with an open security context
 *
 * @retval WS_ERR_NO_CERTIFICATE
 *      - the certificate does not have an encryption key
 *
 * @retval WS_ERR_ENCRYPTION_KEY_NOT_FOUND
 *      - the certificate does not have an encryption key
 *
 * @retval WS_ERR_NOT_SUPPORTED_FOR_THIS_MESSAGE_SECURITY_TYPE
 *     - this function does not support this message's security type
 */
AEROLINK_EXPORT AEROLINK_RESULT
smp_getCertPublicKey(
    SecuredMessageParserC    smpHandle,
    AerolinkEncryptionKey *  encryptionKey);

#ifdef __cplusplus
}
#endif


#endif /* SECURED_MESSAGE_PARSER_C_H */
