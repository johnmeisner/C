/**************************************************************************
 *                                                                        *
 *     File Name:  alsmi_struct.h                                         *
 *     Author:                                                            *
 *         DENSO International America, Inc.                              *
 *         North America Research Laboratory, California Office           *
 *         3252 Business Park Drive                                       *
 *         Vista, CA 92081                                                *
 *                                                                        *
 **************************************************************************/
#ifndef _ALSMI_STRUCT_H_
#define _ALSMI_STRUCT_H_

#include <sys/time.h>
#include "ris_struct.h" // for securityType
#include "ws_errno.h"   // for AEROLINK_RESULT
#include "v2v_max_rv.h" // for V2V_MAX_RV
#include "ris_struct.h" // for risTxMetaRawPktType and sendWSMStruct_S

#define WSA_VEHID_SIZE             6
#define SECONDS_BETW_1970_AND_2004 1072915234   // from 1/1/1970 00:00:00 to 1/1/2004 00:00:00
#define SMI_VERN_SUCCESS           0
#define SMI_VERN_ERROR             2

/* DENSO error codes start at 255 and go down.
 * (Aerolink error codes are WS_SUCCESS, etc. and start at 0 and go up.) */
#define WS_ERR_VERIFY_NOT_ATTEMPTED           244
#define WS_ERR_ASN1_DECODE_FAILED             245
#define WS_ERR_INVALID_PSID                   246
#define WS_ERR_CERTIFICATE_CHANGE_TIMEOUT     247
#define WS_ERR_UNKNOWN_PACKET_TYPE            248
#define WS_ERR_UNSIGNED                       249
#define WS_ERR_SIGNED_BUT_NOT_VERIFIED        250
#define WS_ERR_CERTIFICATE_CHANGE_IN_PROGRESS 251
#define WS_ERR_SIGN_BUFFER_OVERFLOW           252
#define WS_ERR_VERIFY_BUFFER_OVERFLOW         253
#define WS_ERR_PAYLOAD_TOO_LARGE              254
#define WS_ERR_NOT_FOUND                      255

#define ALSMI_PKT_TYPE_WSMP 0
#define ALSMI_PKT_TYPE_EU   1
#define ALSMI_PKT_TYPE_WSA  2

#define ALSMI_NUM_REGISTRATIONS          10
#define ALSMI_NUM_PSIDS_PER_REGISTRATION 10

typedef enum
{
    securedSignSuccess,
    securedSignFail
} smi_security_sign_status_e;

typedef enum
{
    certificate_explicit,
    certificate_implicit,
    certificateDigest,
    certificateChain
} smi_signer_type_e;

typedef struct
{
    /* Applicable only when signStatus=securedSignSuccess */
    smi_signer_type_e          signerType;
    /* Applicable only when signStatus=securedSignSuccess */
    /* Applicable only when  signerType=certificateDigest */
    /* Applicable only when signStatus=securedSignSuccess and signed message has this fields(mfval) in it */
    uint8_t                    digest[8];
    /* Microseconds since 00:00:00 UTC Jan,1,2004 */
    uint64_t                   msgGenTime;
    /* Applicable only when signStatus=securedSignSuccess and signed message has this fields(mfval) in it */
    /* 0 to 63 */
    uint8_t                    msgGenTimeConf;
    smi_security_sign_status_e signStatus;
    uint8_t                    signErrorCode; // Applicable only when signStatus=securedSignFail
    /* Applicable only when signStatus=securedSignSuccess.
       Indicates the length of WSM including security overhead. */
    uint32_t                   psid;
    uint32_t                   wsmLenWithSecurity;
} smiSignResults_t;

typedef struct
{
    uint8_t returnCode;
    uint8_t certId[8];
    uint8_t lcmName[32];
} smiCertChangeResults_t;

enum smi_results_e
{
    smi_results_none = 0,
    smi_results_errors_only,
    smi_results_all
};

/*
 * Function callback to send sign results to the application.
 *
 * Input  parameters:  smiSignResults_t - The signing results.
 * Output parameters:  None
 * Return Value: Returns 0 on success, -1 on failure. Generally always returns 0.
 */
typedef int (*smi_sign_results_p)(smiSignResults_t *);

/*
 * Function callback to send verify WSA results to the application.
 *
 * Input  parameters:  uint16_t - The verify results.
 * Output parameters:  None
 * Return Value: None.
 */
typedef void (*smi_verify_wsa_results_p)(uint16_t);

enum smi_security_approach_e
{
    smi_verify_none = 0,
    smi_verify_then_process,
    smi_verify_on_demand
};

enum smi_security_status_e {
    unsecured = 1,
    securedButNotVerified,
    securedVerifiedSuccess,
    securedVerifiedFail,
    securedVernNotNeeded,
    securityStatusInvalid = 0xAAAAAAAA // Necessary so smi_security_status_e
                                       // will take up 32 bits
};

enum smi_vern_status_e {
    verify_unnecessary  = 0,
    verify_sent,
    verify_res_success,
    verify_res_fail,
    verify_res_error,
    verify_not_initiated,
    verify_not_requested
};

/*
 * Function callback defined in application and is registered with smi
 * library with smiInit() call. The function will be called by library to
 * update the application with  the verification result in case of VOD.
 * Not used in case of VTP approach. Application can register NULL
 * in case of VTP approach during smiInit.
 *
 * Input  parameters:  result: Verification result.
 *                             SMI_VERN_SUCCESS / VERN_ERROR /
 *                             One byte error code from security module in case of verification error.
 *                     seqnum : The sequence number that maps to this OTA.
 * Output parameters:  None
 * Return Value: None.
 */
typedef void (*smi_vern_res_p)(uint32_t result, uint32_t seqnum);

/*
 * Function callback to send a received and verified packet up to the
 * application. Generally, the function will exist in RIS.
 *
 * Input  parameters:  buffer: The packet data.
 *                     size: The size of the data.
 *                     radioNum: The radio number the packet was received on.
 *                     receive_params: The receive parameters associated with
 *                                     the received packet.
 *                     txPwrLevel: The Tx power level gotten from the WSMP
 *                                 header
 *                     dataRate: The data rate gotten from the WSMP header
 *                     channel: The channel number gotten from the WSMP header
 *                     ext_flags: The extended fields flags from the WSMP
 *                                header
 *                     security: The type of security the received packet had.
 *                     securityFlags: The security flags to send to the
 *                                    application.
 *                     vodMsgSeqNum: The VOD message sequence number associated with the packet.
 *                     regIndex: The index into the RIS registration table.
 *                     psid: Provider Service Identifier (PSID)
 * Output parameters:  None
 * Return Value: None
 */
typedef void (*smi_ris_send_recd_pkt_to_app_p)(uint8_t      *data,
                                               size_t        dataLength,
                                               uint32_t      radioNum,
                                               uint8_t       txPwrLevel,
                                               dataRateType  dataRate,
                                               uint8_t       channel,
                                               uint8_t       ext_flags,
                                               uint32_t      security,
                                               uint32_t      securityFlags,
                                               uint32_t      vodMsgSeqNum,
                                               uint32_t      regIndex,
                                               uint32_t      psid);

/*
 * Function callback defined in application and is registered with alsmi library with smiInit() call.
 * The function will be called by library to update the application with the certificate change result.
 * Application can register NULL during smiInit.
 *
 * Input  parameters:  result: Certificate change result.
 * Output parameters:  None
 * Return Value: None.
 */
typedef void (*smi_cert_change_p)(smiCertChangeResults_t *);

typedef struct
{
    smi_signer_type_e signerType;
    uint8_t           digest[8];      // Applicable only when  signerType=certificateDigest
    uint64_t          msgGenTime;     // Microseconds since 00:00:00 UTC Jan,1,2004
    uint8_t           msgGenTimeConf; // 0 to 63
} smiVerResults_t;

typedef struct
{
    uint32_t security_verify_fail_count;
    uint8_t  last_verify_fail_tempID[WSA_VEHID_SIZE];
    uint8_t  last_verify_fail_error_code;
} wmh_smiOverallSecStatsType;

typedef struct
{
    uint32_t cponly_sent;          /* Certificate Verify Requests */
    uint32_t dropped_cponly_vern;
    uint32_t vern_sent;
    uint32_t vern_overflow;
    uint32_t total_vern_success;   /* Total (ECDSA and TESLA) Verification success   */
    uint32_t vern_success;         /* Verification success incase of  ECDSA  */
    uint32_t tesla_vern_success;   /* Verification success incase of  TESLA  */
    uint32_t dropped_pkt_vern;
    uint32_t vern_fail;
    uint32_t vern_64_error;        /* Tesla Timeout */
    uint32_t vern_6a_error;        /* RcvdMsgTooLate */
    uint32_t vern_sixtyfive_error; /* Digest Unknown */
    uint32_t vern_6d_error;        /* Startup Digest Unknown */
    uint32_t vern_6e_error;        /* Stalled message */
    uint32_t vern_61_error;        /*  Verification failed*/
    uint32_t vern_attack_errors;   /* vern_63_error; vern_62_error; vern_6b_error */
    uint32_t vern_other_errors;    /*  vern_sixtysix_error, vern_sixtyseven_error vern_sixtyeight_error vern_sixtynine_error vern_6c_error  */
                                  /* vern_51_error; vern_EB_error; vern_EC_error; vern_ED_error; vern_EF_error; vern_FF_error; vern_FE_error; */
} wmh_smiPerRvVernStatsType;

typedef struct
{
    uint32_t otaTotalLen;
    uint32_t signatureLen;
    uint32_t signerType;                  /* 0: Cert 1: Digest */
    uint32_t digestOrCertLen;
    uint32_t fixedSecOverheadLen;         /* 4 bytes */
    uint32_t fixed1609AndPhyMacHeaderLen; /* 11 + 46 bytes */

} field_lens_t;

#if 0
typedef struct
{
    wmh_smiPerRvVernStatsType vernStats;
    latency_data_t postSecLatStats;
    latency_data_t interPktGapLatStats;
    field_lens_t   secFieldLens;
} wmh_smiPerRvSecStatsType;
#endif

typedef struct
{
    uint32_t sign_sent;
    uint32_t sign_fail;
    uint32_t sign_success;
    uint32_t sign_overflow;

    uint32_t wsa_sign_sent;
    uint32_t wsa_sign_fail;
    uint32_t wsa_sign_success;
    uint32_t wsa_sign_overflow;

    uint32_t wsa_vern_sent;
    uint32_t wsa_vern_fail;
    uint32_t wsa_vern_success;
    uint32_t wsa_vern_overflow;


    uint32_t tesla_key;

    uint32_t cert_chng_sent;
    uint32_t cert_chng_success;
    uint32_t cert_chng_fail;

    uint32_t smi_rmfromverbuff_fail;

} smiStatsType;

typedef struct
{
    AEROLINK_RESULT returnCode;
    uint32_t        vodMsgSeqNum;
} smiVerifyWsmReplyType;

typedef struct {
    bool_t   enabled;
    uint8_t  wsaRadio;
    uint8_t  destMac[LENGTH_MAC];
    uint8_t  mgmtId;
    uint8_t  repeat;
    uint8_t  channelNumber;
    uint32_t channelType;
    uint8_t  channelInterval;
    uint8_t  dataRate;
    uint8_t  txPwrLevel;
    uint8_t  priority;
} wsaTxSecurityType;

/*
 * Used to initialize security
 */
typedef struct {
    /* The following fields are filled out by smiInitialize() in alsmi_api.c
     * and are not touched by the application.
     * vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv */
    rsIoctlType                  cmd; /* WSU_NS_SECURITY_INITIALIZE */
    pid_t                        smi_sign_results_pid;
    pid_t                        smi_sign_wsa_results_pid;
    pid_t                        smi_verify_wsa_results_pid;
    pid_t                        smi_cert_change_pid;
    /* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ */
    bool_t                       debug;
    bool_t                       vodEnabled;
    smi_cert_change_p            smi_cert_change_callback;
    uint32_t                     vtp_milliseconds_per_verification;
    enum smi_results_e           smi_sign_results_options;
    enum smi_results_e           smi_sign_wsa_results_options;
    enum smi_results_e           smi_verify_wsa_results_options;
    smi_sign_results_p           smi_sign_results_callback;
    smi_sign_results_p           smi_sign_wsa_results_callback;
    smi_verify_wsa_results_p     smi_verify_wsa_results_callback;
    char                         lcmName[32];
    uint8_t                      numSignResultsPSIDs;
    uint32_t                     signResultsPSIDs[ALSMI_NUM_PSIDS_PER_REGISTRATION];
} rsSecurityInitializeType;

typedef struct {
    rsResultCodeType risRet;
    uint8_t          regIdx;
} rsSecurityInitializeReplyType;

typedef struct
{
    /* All of the  fields are filled out by smiTerm() in alsmi_api.c
     * and are not touched by the application. */
    rsIoctlType cmd; /* WSU_NS_SECURITY_TERMINATE */
    uint8_t regIdx;
    bool_t  sign_wsa_results;
    bool_t  verify_wsa_results;
    bool_t  cert_change_results;
} rsSecurityTerminateType;

typedef struct {
    rsResultCodeType risRet;
    int              useCount; /* The number of alsmiInitialize's still in
                                * effect */
} rsSecurityTerminateReplyType;

typedef struct {
    uint32_t                   psid;
    smi_security_sign_status_e securitySignStatus;
    uint8_t                    returnCode;
    uint16_t                   signedDataLength;
} signResultsType;

typedef struct {
    /* The following field is filled out by smiUpdatePositionAndTime()
     * in alsmi_api.c and is not touched by the application.
     * vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv */
    rsIoctlType cmd; /* WSU_NS_SECURITY_UPDATE_POSITION_AND_TIME */
    /* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ */
    double      latitude;
    double      longitude;
    double      elevation;
    int16_t     leapSecondsSince2004;
    uint16_t    countryCode;
} rsSecurityUpdatePositionAndTimeType;

typedef struct {
    rsResultCodeType risRet;
    int16_t          leapSeconds;
} rsGetLeapSecondsReplyType;

typedef struct {
    rsResultCodeType risRet;
    #define MAX_AEROLINK_VERSION_LEN 32
    char             aerolinkVersion[MAX_AEROLINK_VERSION_LEN];
} rsGetAerolinkVersionReplyType;

#endif // _ALSMI_STRUCT_H_
