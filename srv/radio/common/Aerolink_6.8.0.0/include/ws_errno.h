/* `ws_errno.h'
 *
 * AUTOMATICALLY GENERATED -- DO NOT EDIT
 *
 * Defines error codes for the Aerolink project.
 *
 * Copyright (c) NTRU Cryptosystems, Inc. 2007.
 * Copyright (c) Security Innovation, Inc. 2010-2015.
 */

#ifndef _WS_ERRNO_H /* Prevent multiple inclusion */
#define _WS_ERRNO_H

#include <aerolink_api.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Get ID string (e.g. "WS_SUCCESS") corresponding to error number. */
AEROLINK_EXPORT const char *
ws_errid(int err);

/* Get description string corresponding to error number. */
AEROLINK_EXPORT const char *
ws_strerror(int err);

typedef enum {
     WS_SUCCESS                                          = (  0), /* No error */
     WS_ERR_SYS                                          = (  1), /* System call failed */
     WS_ERR_CONFIG_SYNTAX                                = (  2), /* Syntax error in configuration file */
     WS_ERR_DB_BDB                                       = (  3), /* Internal Berkeley DB failure */
     WS_ERR_DB_DEADLOCK                                  = (  4), /* Deadlock on transactional operation; try again? */
     WS_ERR_DB_NOT_FOUND                                 = (  5), /* Database not found */
     WS_ERR_NULLPARAM                                    = (  6), /* NULL parameter */
     WS_ERR_DATA                                         = (  7), /* Badly formed input WSA */
     WS_ERR_BUFLEN                                       = (  8), /* Buffer supplied for signing is too short */
     WS_ERR_BAD_SIG                                      = (  9), /* WSA signature didn't verify */
     WS_ERR_NOMEM                                        = ( 10), /* Out of memory */
     WS_ERR_APP_AUTHENTICATION                           = ( 11), /* Authenticating application */
     WS_ERR_BAD_ARGS                                     = ( 12), /* Bad arguments to method */
     WS_ERR_CA_APP_ID_SCOPE                              = ( 13), /* Certificate authority application identity scope */
     WS_ERR_UNABLE_TO_DECRYPT_MESSAGE                    = ( 14), /* Ws err unable to decrypt message */
     WS_ERR_MESSAGE_REPLAY                               = ( 15), /* Duplicate message -- possible replay attack */
     WS_ERR_CERTIFICATE_EXPIRED                          = ( 16), /* Expired certificate */
     WS_ERR_CERTIFICATE_OUTSIDE_BOUNDS                   = ( 17), /* Ws err certificate outside bounds */
     WS_ERR_INVALID_CERTIFICATE_SUBJECT_TYPE             = ( 18), /* Ws err invalid certificate subject type */
     WS_ERR_NO_CERTIFICATE                               = ( 19), /* Missing end-entity certificate or no valid signing certificate */
     WS_ERR_NO_CA_CERTIFICATE                            = ( 20), /* Missing CA certificate in chain */
     WS_ERR_INVALID_CERTIFICATE                          = ( 21), /* Certificate contains a disallowed set of field values */
     WS_ERR_MESSAGE_GEN_BEFORE_CERT_VALIDITY_PERIOD      = ( 22), /* Message generated before certificate is valid */
     WS_ERR_MESSAGE_GEN_AFTER_CERT_VALIDITY_PERIOD       = ( 23), /* Message generated after certificate expiration */
     WS_ERR_MESSAGE_EXP_BEFORE_CERT_VALIDITY_PERIOD      = ( 24), /* Message expiration is before certificate is valid */
     WS_ERR_MESSAGE_EXP_AFTER_CERT_VALIDITY_PERIOD       = ( 25), /* Message expiration is after certificate expiration */
     WS_ERR_MESSAGE_OUTSIDE_BOUNDS                       = ( 26), /* Ws err message outside bounds */
     WS_ERR_NO_KEY_AVAILABLE                             = ( 27), /* Could not find a key to perform the requested operation */
     WS_ERR_NO_MATCHING_KEY                              = ( 28), /* No key in WSC to match key in imported cert */
     WS_ERR_RECIPIENT_NO_KEY                             = ( 29), /* No certificate or key corresponding to recipient identifier */
     WS_ERR_CERTIFICATE_REVOKED                          = ( 30), /* Ws err certificate revoked */
     WS_ERR_UNKNOWN_CA_CERTIFICATE                       = ( 31), /* Unknown certificate authority certificate */
     WS_ERR_PARSE_FAIL                                   = ( 32), /* Ws err parse fail */
     WS_ERR_WAVE_SECURITY_CONTEXT                        = ( 33), /* Ws err wave security context */
     WS_ERR_UNSUPPORTED_CRYPTO_ALGORITHM                 = ( 34), /* Ws err unsupported crypto algorithm */
     WS_ERR_WRONG_KEY_TYPE                               = ( 35), /* Key does not support an operation of the specified type */
     WS_ERR_CRYPTO_INTERNAL                              = ( 36), /* Undocumented internal error in crypto libraries */
     WS_ERR_BAD_DATA_FROM_DATABASE                       = ( 37), /* Bad data read from database */
     WS_ERR_HPSAM                                        = ( 38), /* HPSAM error. To use software crypto, set crypto_provider = software_security_builder in /etc/viicsec.conf. To use the HPSAM emulator (for NTRU internal development purposes only), set the environment variable VIICSEC_HARDWARE_TYPE to FLAG_SIMULATED_HW. */
     WS_ERR_MESSAGE_DOES_NOT_USE_GENERATION_TIME         = ( 39), /* Message does not use generation time */
     WS_ERR_MESSAGE_DOES_NOT_USE_GENERATION_LOCATION     = ( 40), /* Message does not use generation location */
     WS_ERR_MESSAGE_DOES_NOT_USE_EXPIRY                  = ( 41), /* Message does not use expiry */
     WS_ERR_PARSE_IGNORE                                 = ( 42), /* Ws err parse ignore */
     WS_ERR_NO_POSITION_INFORMATION_AVAILABLE            = ( 43), /* Positioning information unavailable */
     WS_ERR_INCOMPLETE_STATE                             = ( 44), /* Object has not been given all the information needed to carry out this action */
     WS_ERR_READ_CONFIG                                  = ( 45), /* Error reading global configuration file */
     WS_ERR_CONFIG_VAL                                   = ( 46), /* Unexpected value in global configuration file */
     WS_ERR_ENCODE                                       = ( 47), /* Encoding error */
     WS_ERR_SYMMETRIC_AUTHENTICATION_CHECK_FAILED        = ( 48), /* Symmetric authentication check failed */
     WS_ERR_RECIPIENT_REVOKED                            = ( 49), /* The recipient's certificate has been revoked */
     WS_ERR_RECIPIENT_EXPIRED                            = ( 50), /* The recipient's certificate has expired */
     WS_ERR_RECIPIENT_DUPLICATE                          = ( 51), /* Recipient identifier already exists */
     WS_ERR_NO_GENERATION_TIME                           = ( 52), /* Expected but did not find generation time field in signed message */
     WS_ERR_NO_GENERATION_LOCATION                       = ( 53), /* Expected but did not find generation location field in signed message */
     WS_ERR_NO_EXPIRY_TIME                               = ( 54), /* Expected but did not find expiry time field in signed message */
     WS_ERR_INVALID_PERMISSIONS_REQUEST                  = ( 55), /* Certificate does not have permissions of the requested type */
     WS_ERR_NO_EXTERNAL_CA_KEY                           = ( 56), /* No external CA key available for requested PSID */
     WS_ERR_LCM_FEATURE_UNSUPPORTED                      = ( 57), /* The Local Certificate Manager does not support this feature */
     WS_ERR_LCMD_UNAVAILABLE                             = ( 58), /* Unable to connect to the CAMP LCM daemon */
     WS_ERR_INTERNAL                                     = ( 59), /* Internal error */
     WS_ERR_ALREADY_OPEN                                 = ( 60), /* The specified Security Context is already open */
     WS_ERR_INVALID_PSID                                 = ( 61), /* The PSID in the message does not match the PSID in the context */
     WS_ERR_INVALID_PERMISSIONS                          = ( 62), /* The provided permissions are not a valid format */
     WS_ERR_NOT_INITIALIZED                              = ( 63), /* The security services has not been initialized */
     WS_ERR_CONTEXT_NOT_INITIALIZED                      = ( 64), /* The security context has not been initialized */
     WS_ERR_BUFFER_TOO_SMALL                             = ( 65), /* The supplied buffer is too small */
     WS_ERR_INVALID_CONTEXT                              = ( 66), /* The object is not associated with a valid Security Context */
     WS_ERR_INVALID_HANDLE                               = ( 67), /* The provided handle is not valid */
     WS_ERR_PSID_NOT_IN_CONTEXT                          = ( 68), /* The provided Security Context does not contain the specified PSID */
     WS_ERR_NOT_IMPLEMENTED                              = ( 69), /* The mesage type is not supported */
     WS_ERR_INVALID_GLOBAL_CONFIG                        = ( 70), /* An error with the global configuration file occured */
     WS_ERR_INVALID_PROFILE_CONFIG                       = ( 71), /* An error with a profile configuration file occured */
     WS_ERR_INVALID_CONTEXT_CONFIG                       = ( 72), /* An error with a context configuration file occured */
     WS_ERR_MESSAGE_EXPIRED                              = ( 73), /* Message has expired */
     WS_ERR_MESSAGE_IN_FUTURE                            = ( 74), /* Message is in the future */
     WS_ERR_DATA_NOT_EXTRACTED                           = ( 75), /* Parser has not been given all the information needed to carry out this action */
     WS_ERR_NOT_SUPPORTED_FOR_THIS_CONTENT_TYPE          = ( 76), /* Messages of this content type do not support this operation */
     WS_ERR_EXTERNAL_DATA_MISSING                        = ( 77), /* Message requires external data be provided for this operation */
     WS_ERR_EXTERNAL_DATA_NOT_NEEDED                     = ( 78), /* Message does not allow external data to be provided for this operation */
     WS_ERR_MESSAGE_SIGNED_OUTSIDE_REGION                = ( 79), /* Cert not authorized to sign where it did */
     WS_ERR_UNAUTHORIZED_PERMISSIONS                     = ( 80), /* Psid/ITS-AID not found in signing certificate */
     WS_ERR_UNAUTHORIZED_CERTIFICATE_TYPE                = ( 81), /* Correct holder type not found in signing certificate */
     WS_ERR_VALUE_ALREADY_SET                            = ( 82), /* Ws err value already set */
     WS_ERR_PSID_NOT_IN_CERTIFICATE                      = ( 83), /* Specified psid is not in the certificate permissions list */
     WS_ERR_NOT_SUPPORTED_FOR_THIS_MESSAGE_SECURITY_TYPE = ( 84), /* Function does not support this message's security type */
     WS_ERR_KEY_ALREADY_TAGGED                           = ( 85), /* Encryption Key has already been tagged */
     WS_ERR_KEY_NOT_TAGGED                               = ( 86), /* Encryption Key has not been tagged */
     WS_ERR_DUPLICATE_ENCRYPTION_ID                      = ( 87), /* The encryption id is already used */
     WS_ERR_INVALID_RECIPIENT                            = ( 88), /* Recipient is not known */
     WS_ERR_INVALID_SIGNER_ID_TYPE                       = ( 89), /* The SignerIdentifier is not valid */
     WS_ERR_DISALLOWED_MESSAGE_SECURITY_TYPE             = ( 90), /* The message security parameters are not allowed */
     WS_ERR_INVALID_PRIVATE_KEY_ENCODING                 = ( 91), /* The private key encoding is not supported */
     WS_ERR_CRYPTO_QUEUE_FULL                            = ( 92), /* All threads in the pool are in use */
     WS_ERR_CERTIFICATES_INCONSISTENT                    = ( 93), /* Data in the certificate chain is inconsistent */
     WS_ERR_CONSISTENCY_NOT_CHECKED                      = ( 94), /* Consistency checks for the message have not been run */
     WS_ERR_MEMORY_FAILURE                               = ( 95), /* Memory cannot be allocated */
     WS_ERR_MESSAGE_EXPIRY_BEFORE_GENERATION             = ( 96), /* Message expiry time is before the message generation time */
     WS_ERR_MESSAGE_FROM_TOO_FAR_AWAY                    = ( 97), /* Message was generated too far from the current location */
     WS_ERR_MESSAGE_TOO_OLD                              = ( 98), /* Message was signed too far in the past */
     WS_ERR_MESSAGE_SIGNATURE_VERIFICATION_FAILED        = ( 99), /* Message signature fails to verify */
     WS_ERR_CERTIFICATE_SIGNATURE_VERIFICATION_FAILED    = (100), /* Certificate signature fails to verify */
     WS_ERR_DISALLOWED_IN_PROFILE                        = (101), /* Input parameters do not meet the requirements of the security profile */
     WS_ERR_EXTRA_HEADER                                 = (102), /* Message violates the security profile associated with the PSID/(ITS-AID) by including a header that is not allowed */
     WS_ERR_INVALID_FIELD                                = (103), /* Field has an invalid value */
     WS_ERR_MISSING_HEADER                               = (104), /* Message violates the security profile associated with the PSID/(ITS-AID) because it lacks a header field that is required */
     WS_ERR_UNABLE_TO_DECRYPT_DATA                       = (105), /* Failed to decrypt the data with the symmetric key */
     WS_ERR_UNABLE_TO_DECRYPT_SYMMETRIC_KEY              = (106), /* Failed to decrypt symmetric key */
     WS_ERR_WSA_CERTS_NOT_IN_CONTEXT                     = (107), /* WSA certificates are not configured in the security context associated with the secured message */
     WS_ERR_INVALID_WSA                                  = (108), /* Wsa is invalid */
     WS_ERR_UNAUTHORIZED_REGION                          = (109), /* Certificate is not allowed to sign a message at the current location */
     WS_ERR_ALREADY_REGISTERED                           = (110), /* The  context/lcm combination is already registered */
     WS_ERR_ID_CHANGE_ABORT                              = (111), /* Id change aborted */
     WS_ERR_ID_CHANGE_BLOCKED                            = (112), /* Id changes have been blocked */
     WS_ERR_ID_CHANGE_NOT_LOCKED                         = (113), /* No locks to release */
     WS_ERR_NOT_REGISTERED                               = (114), /* The context/lcm combination is not registered */
     WS_ERR_UNKNOWN_LCM                                  = (115), /* The specified LCM name is not known */
     WS_ERR_INCONSISTENT_SIGNING_KEY_TYPE                = (116), /* The ACF's signing key type is not valid. */
     WS_ERR_HSM_CLA_NOT_SUPPORTED                        = (117), /* HSM: The CLA value is not supported by the applet */
     WS_ERR_HSM_INS_NOT_SUPPORTED                        = (118), /* HSM: The INS value is not supported by the applet */
     WS_ERR_HSM_INCORRECT_P1P2                           = (119), /* HSM: The value of parameter P1 or P2 is invalid */
     WS_ERR_HSM_WRONG_LENGTH                             = (120), /* HSM: The value of parameter Lc or Le is invalid */
     WS_ERR_HSM_WRONG_DATA                               = (121), /* HSM: The data field of the command contains wrong data */
     WS_ERR_HSM_FILE_FULL                                = (122), /* HSM: No more memory available */
     WS_ERR_HSM_EXECUTION_ERROR_NVRAM_UNCHANGED          = (123), /* HSM: Internal execution error and the result is that the NVRAM is unchanged */
     WS_ERR_HSM_EXECUTION_ERROR_NVRAM_CHANGED            = (124), /* HSM: Internal execution error and the result is that the NVRAM is changed */
     WS_ERR_HSM_NO_PRECISE_DIAGNOSIS                     = (125), /* HSM: An exception occured of which no precise diagnosis is available. */
     WS_ERR_HSM_CONDITIONS_NOT_SATISFIED                 = (126), /* HSM: Conditions not satisfied */
     WS_ERR_HSM_SECURITY_STATUS_NOT_SATISFIED            = (127), /* HSM: Security status not satisfied */
     WS_ERR_HSM_AUTHENTICATION_METHOD_BLOCKED            = (128), /* HSM: Authentication method blocked */
     WS_ERR_HSM_UNDEFINED_ERROR                          = (129), /* HSM: Error code not relevant. */
     WS_ERR_HSM_API_ERROR                                = (130), /* HSM: Internal API Wrapper error */
     WS_ERR_HSM_MEMORY_FAILURE                           = (131), /* HSM: EEPROM Memory Failure */
     WS_ERR_INVALID_CRYPTO_API_ERROR                     = (132), /* Invalid aerolink crypto api return code */
     WS_ERR_PARSE_FAIL_EXTRA_DATA                        = (133), /* The parse failed due to extra data present */
     WS_ERR_DATA_NOT_SIGNED                              = (134), /* The processed data was not a Signed Message as expected */
     WS_ERR_NO_PROFILES                                  = (135), /* No configured profiles */
     WS_ERR_DUPLICATE_PROFILES                           = (136), /* Multiple profiles found with the same psid. */
     WS_ERR_NO_ASSURANCE_LEVEL                           = (137), /* Expected but did not find assurance level. */
     WS_ERR_PKI_GENERATE_REQUEST                         = (138), /* PKI: Failed to generate service request */
     WS_ERR_PKI_SERVICE_REQUEST                          = (139), /* PKI: Failed to request service */
     WS_ERR_PKI_SERVICE_RESPONSE                         = (140), /* PKI: Failed to process response */
     WS_ERR_PKI_INVALID_LCM_TYPE                         = (141), /* PKI: Invalid LCM type */
     WS_ERR_PKI_INVALID_URL                              = (142), /* PKI: Invalid URL */
     WS_ERR_PKI_INVALID_CONFIG                           = (143), /* PKI: Invalid PKI config detected */
     WS_ERR_PKI_ENCODE_BASE64                            = (144), /* PKI: Failed to encode base64 */
     WS_ERR_PKI_BAD_DATA                                 = (145), /* PKI: Corrupted server response */
     WS_ERR_SAVE_FILE                                    = (146), /* Failed to save file */
     WS_ERR_UNSUPPORTED_CUSTOM_SERVICE                   = (147), /* Unsupported Aerolink service. */
     WS_ERR_UNSUPPORTED_CUSTOM_SETTING                   = (148), /* Unsupported Aerolink custom setting. */
     WS_ERR_INVALID_CUSTOM_SETTING_FORMAT                = (149), /* Invalid Aerolink custom setting format. */
     WS_ERR_SERVICE_DISABLED                             = (150), /* The specified service is disabled. */
     WS_ERR_INVALID_ENCRYPTION_KEY                       = (151), /* Encryption key is invalid */
     WS_ERR_ENCRYPTION_KEY_NOT_FOUND                     = (152), /* Encryption key not found */
     WS_ERR_CMAC_VERIFICATION_FAILED                     = (153), /* CMAC verification failed */
     WS_ERR_INVALID_CA_CERTIFICATE_STORE                 = (154), /* Invalid CA certificate store format */
     WS_ERR_CA_CERTIFICATE_STORE_VERIFICATION_FAILED     = (155), /* Failed to verify CA certificate store */
     WS_ERR_PAYLOAD_NOT_SPDU                             = (156), /* The provided SPDU payload is not valid. */
     WS_ERR_NO_PROFILE_FOR_PERMISSON                     = (157), /* There is not persmission for requested permission. */
     WS_ERR_INVALID_CRL_TYPE                             = (158), /* The crl type is not supported by this crl processor. */
     WS_ERR_READ_FILE                                    = (159), /* Failed to read file */
     WS_ERR_INVALID_SPDU_HASH                            = (160), /* The hash algorithm in the signed SPDU does not match the hash algorithm used by the signer */
     WS_ERR_PKI_SERVICE_RESPONSE_PARTIAL_DATA            = (161), /* PKI: server response contains partial data */
     WS_ERR_INVALID_ACF_HEADER                           = (162), /* The provided data cannot be encoded into an acf header. */
     WS_ERR_CRL_TOO_OVERDUE                              = (163), /* A valid CRL for the certificate is not available */
     WS_ERR_CRL_SIGNER_INCONSISTENT                      = (164), /* The signer of a CRL is inconsistent with the CraCa ID in the CRL contents */
     WS_ERR_CRL_TIMES_INCONSISTENT                       = (165), /* The issue date of a CRL is not strictly before the nextCrl date. */
     WS_ERR_NO_REPORT_AVAILABLE                          = (166), /* No misbehavior reports are available. */
     WS_ERR_SECURITY_LIBRARY_NOT_INITIALIZED             = (167), /* The security library is not initialized. */
     WS_ERR_SECURITY_LIBRARY_MISBEHAVIOR_CONFIGURATION   = (168), /* The security library is not configured for misbehavior. */
     WS_ERR_CERTIFICATE_LINKAGE_I_NOT_CURRENT            = (169), /* Linkage values for the time period indicated by the i value in a certificate's linkage revocation data have not been calculated, so a revocation check cannot be made. */
     WS_ERR_NO_ETSI_REGISTRATION                         = (170), /* Cannot find valid ETSI registration information. */
     WS_ERR_LCM_DIR_DOES_NOT_EXIST                       = (171), /* The LCM directory provided to the AT Record does not exist. */
     WS_ERR_ATRECORD_NOT_INITIALIZED                     = (172), /* The AT Record has not been initialized. */
     WS_ERR_CREATE_DIRECTORY                             = (173), /* Failed to create the directory. */
     WS_ERR_DELETE_FILE                                  = (174), /* Failed to delete the file. */
     WS_ERR_REQUEST_HASH_NOT_BEING_SERVICED              = (175), /* The request hash associated with the certificate is not being serviced. */
     WS_ERR_CANONICAL_KEY_ALREADY_EXISTS                 = (176), /* The canonical key has already been generated and can only be generated once. */
     WS_ERR_PKI_LCM_NOT_INITIALIZED                      = (177), /* The LCM has not be initialized for SCMS service. */
     WS_ERR_INVALID_ENROLLMENT_CONFIG                    = (178), /* The enrollment.conf file has one or more invalid entries. */
     WS_ERR_INVALID_ENROLLMENT_DATA                      = (179), /* The enrollment certificate and/or its signing key is invalid. */

} AEROLINK_RESULT;

#ifdef __cplusplus
}
#endif

#endif /* ndef _WS_ERRNO_H */
