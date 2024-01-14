/******************************************************************************
 *                                                                            *
 *     File Name:  alsmi.c                                                    *
 *     Author:                                                                *
 *         DENSO International America, Inc.                                  *
 *         North America Research and Development, California Office          *
 *         3252 Business Park Drive                                           *
 *         Vista, CA 92081                                                    *
 *                                                                            *
 ******************************************************************************
 * (C) Copyright 2020 DENSO International America, Inc.  All rights reserved. *
 ******************************************************************************/
 /*
  * Denso to Aerolink Security Management Interface
  */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <pthread.h>
#include <semaphore.h>
#include <dirent.h>
#include <ctype.h>
#include "viicsec.h"
#include "shm_rsk.h"
#include "rs.h"
#include "rpsi.h"
#include "alsmi.h"
#include "wme.h"
#include "ipcsock.h"
#include "nscfg.h"

extern shm_rsk_t *shm_rsk_ptr;
extern bool_t     p2pEnabled;
extern uint8_t    secmode;
extern bool_t     dummy_verify_succeeds;

//#define CERTIFICATE_CHANGE_TIMING_TEST

#define ERROR_MESSAGE_MAX_PRINT_TIMES 10

#define PRINT_MAX_N_TIMES(n, fmt, args...) \
{ \
    static int counter = 0; \
    if (counter < (n)) { \
        printf(fmt, ##args); \
        fflush(stdout); \
        counter++; \
    } \
}

#define ALSMI_DEBUG_PRINTF(fmt, args...) \
    if (debug) { \
        printf(fmt, ##args); \
        fflush(stdout); \
    } \
    if (debug_log) { \
        FILE *fd = fopen("/mnt/rwflash/dbglogs/alsmi_debug.log", "a"); \
        fprintf(fd, fmt, ##args); \
        fclose(fd); \
    }

#define ALSMI_DEBUG_LOG(fmt, args...) \
    if (debug_log) { \
        FILE *fd = fopen("/mnt/rwflash/dbglogs/alsmi_debug.log", "a"); \
        fprintf(fd, fmt, ##args); \
        fclose(fd); \
    }

//#define AEROLINK_CALL_DEBUG
#ifdef AEROLINK_CALL_DEBUG
#define AEROLINK_CALL_PRINTF(fmt, args...) \
    printf(fmt, ##args); \
    fflush(stdout); \
    if (debug_log) { \
        FILE *fd = fopen("/mnt/rwflash/dbglogs/alsmi_debug.log", "a"); \
        fprintf(fd, fmt, ##args); \
        fclose(fd); \
    }
#else
#define AEROLINK_CALL_PRINTF(fmt, args...) \
    if (debug) { \
        printf(fmt, ##args); \
        fflush(stdout); \
    } \
    if (debug_log) { \
        FILE *fd = fopen("/mnt/rwflash/dbglogs/alsmi_debug.log", "a"); \
        fprintf(fd, fmt, ##args); \
        fclose(fd); \
    }
#endif

#define MAX_WSMP_HDR_LEN             18 /* The maximum length of a WSMP
                                         * header */
#define MAX_UNSECURED_1609P2_HDR_LEN 5  /* The maximum length of an unsecured
                                         * 1609.2 header */

/* Registration Table */
typedef struct {
    bool_t             inUse;
    uint8_t            numSignResultsPSIDs;
    uint32_t           signResultsPSIDs[ALSMI_NUM_PSIDS_PER_REGISTRATION];
    enum smi_results_e sign_results_options;
    char               lcmName[32];
    bool_t             lcmNameRegistered;
} registration_t;

static pthread_mutex_t regTableMutex = PTHREAD_MUTEX_INITIALIZER;
static registration_t  regTable[ALSMI_NUM_REGISTRATIONS];

/* Sign buffer. All data buffered for signing are stored here */
/* Note that NUM_SIGN_BUFFER_ELEMENTS must divide evenly into
 * NUM_SIGN_BUFFER_HANDLES and that NUM_SIGN_BUFFER_HANDLES must be <= 65536 */
#define NUM_SIGN_BUFFER_ELEMENTS 32
#define NUM_SIGN_BUFFER_HANDLES  ((65536 / NUM_SIGN_BUFFER_ELEMENTS) * NUM_SIGN_BUFFER_ELEMENTS)

typedef struct {
    uint32_t                 signHandle;                            // The sign handle; used to locate the sign buffer element
                                                                    // and determine if the buffer has been overwritten
    SecuredMessageGeneratorC smg;                                   // The SMG that is being used to sign the packet
    bool_t                   usingGeneratedSmg;                     // TRUE = Using SMG that needs to be freed when it is done
                                                                    // FALSE = Using SMG that was allocated at startup and is reused
    uint8_t                  unsignedData[MAX_WSM_DATA];            // Packet before it is signed
    uint16_t                 unsignedDataLength;                    // Packet before it is signed length
    uint8_t                  packetData[HDRS_LEN + MAX_WSM_DATA];   // Where the packet goes after it has had the signature or the
                                                                    // unsecured 1609.2 header added. Note that an extra HDRS_LEN
                                                                    // bytes of space are added to allow for the addition of things
                                                                    // such as WSMP, 802.11, and MK5 headers.
    uint8_t                  additionalDataLen;                     // Length of the additional data outside of the data to be signed
                                                                    // Used for the basic header for EU
    union {
        sendWSMStruct_S      sendWSMStruct;                         // Data needed for transmitting the packet for WSMP
        risTxMetaEUPktType   EUTxMetaData;                          // Data needed for transmitting the packet for EU
        wsaTxSecurityType    wsaTxSecurity;                         // Data needed for Tx security for the WSA
    } u;
    uint8_t                  pktType;                               // ALSMI_PKT_TYPE_WSMP, ALSMI_PKT_TYPE_EU, or ALSMI_PKT_TYPE_WSA
    bool_t                   signingSubmitted;                      // TRUE if packet has been submitted to Aerolink for signing
} signBufferElement_t;

static pthread_mutex_t     signBufferMutex = PTHREAD_MUTEX_INITIALIZER;
static signBufferElement_t signBuffer[NUM_SIGN_BUFFER_ELEMENTS];
static uint32_t            signBufferNextHandle;
/* End of sign buffer variables */

/* Verify buffer. All data buffered for verification are stored here.
 * We have a larger number of verify buffer handles than we have verify buffer
 * elements. When the buffers "warp around" and we go from using the last
 * verify buffer element to using the first one, we will overwrite what was in
 * there previously. To convert the handle (vodMsgSeqNum) to the buffer element
 * number, we do element# = (vodMsgSeqNum % NUM_VERIFY_BUFFER_ELEMENTS). One
 * of the things saved in the element is the vodMsgSeqNum we used. Therefore,
 * if we later access the element, and the verify buffer element vodMsgSeqNum
 * does not match the vodMsgSeqNum we are currently using, then we know that
 * what was previously written to the verify buffer has been overwritten, and
 * we have a verify buffer overflow condition. Note that for this to work,
 * NUM_VERIFY_BUFFER_ELEMENTS must divide evenly into NUM_VERIFY_BUFFER_HANDLES
 * and that NUM_VERIFY_BUFFER_HANDLES must be <= 65536 */
#define NUM_VERIFY_BUFFER_ELEMENTS    (V2V_MAX_RV * 3)
#define NUM_VERIFY_BUFFER_HANDLES     ((65536 / NUM_VERIFY_BUFFER_ELEMENTS) * NUM_VERIFY_BUFFER_ELEMENTS)
#define MAX_UNSTRIPPED_MESSAGE_LENGTH MAX_WSM_DATA

typedef struct {
    uint32_t               vodMsgSeqNum;                               // The VOD message sequence number; used to locate the verify buffer element
                                                                       // and determine if the buffer has been overwritten
    SecuredMessageParserC  smp;                                        // The SMP used to strip and verify the packet
    uint8_t                signedData[MAX_UNSTRIPPED_MESSAGE_LENGTH];  // The packet as it was originally received
    uint16_t               signedDataLength;                           // The packet as it was originally received's length
    uint8_t                pktType;                                    // ALSMI_PKT_TYPE_WSMP or ALSMI_PKT_TYPE_EU.
    uint32_t              *pSecurityFlags;                             // Pointer to the securityFlags in v2v-i to be set
    uint8_t               *unsignedData;                               // Pointer to the unsigned data
    uint16_t               unsignedDataLength;                         // Pointer to the unsigned data length
    uint32_t               PSID;                                       // The PSID of the packet being verified
    uint16_t               offsetToBasicHeader;                        // Offset from pPkb->data to the start of the basic header
    uint8_t               *pOffsetToCommonHeader;                      // Pointer to the offset from pPkb->data to the start of the common header
    int                    recvCmdSock;                                // Socket to send the verify results to
    uint16_t               sender_port;                                // Port to send the verify results to
    uint8_t                sspLen;                                     // SSP data length
    uint8_t                ssp[MAX_SSP_DATA];                          // SSP data
    bool_t                 verificationSubmitted;                      // TRUE if packet has been submitted to Aerolink for verification
    uint32_t               signedWsaHandle;                            // Signed WSA handle. Used by verify WSA only.
    rsRadioType            radioType;                                  // Radio type the packet was originally received on.
    AEROLINK_RESULT        verifyResult_vodResult;                     // Verify result vodResult value. WS_ERR_VERIFY_NOT_ATTEMPTED if
                                                                       // verification not attempted yet.
} verifyBufferElement_t;

static pthread_mutex_t       verifyBufferMutex = PTHREAD_MUTEX_INITIALIZER;
static verifyBufferElement_t verifyBuffer[NUM_VERIFY_BUFFER_ELEMENTS];
static uint32_t              verifyBufferNextHandle;
/* End of verify buffer variables */

/* Lock for the GPS, and saved GPS values for when it is locked */
static bool_t   gps_locked       = FALSE;
static bool_t   gps_saved_values = FALSE;
static double   saved_latitude;
static double   saved_longitude;
static double   saved_elevation;
static int16_t  saved_leapSecondsSince2004;
static uint16_t saved_countryCode;


/* Static variables */
static bool_t                   debug;
static bool_t                   debug_log;
static bool_t                   alsmiInitializeCalled = FALSE;
static bool_t                   alsmiVodEnabled = FALSE;
static uint32_t                 milliseconds_per_verification;
static float64_t                scaled_milliseconds_per_verification;
static enum smi_results_e       sign_wsa_results_options;
static pid_t                    sign_wsa_results_pid;
static enum smi_results_e       verify_wsa_results_options;
static pid_t                    verify_wsa_results_pid;
static pid_t                    cert_change_pid;
static SecurityContextC         context;
static SecuredMessageGeneratorC sign_smg;
static bool_t                   sign_smgInUse;
static SecuredMessageParserC    strip_smp;
static SecuredMessageParserC    vtp_smp;
static bool_t                   verify_smpInUse;
static bool_t                   certificateChangeInProgress;
static uint8_t                  useCount;
static int16_t                  lastLeapSecondsSince2004;
static bool_t                   certChangeLocked = FALSE;
static pthread_mutex_t          certChangeLockMutex = PTHREAD_MUTEX_INITIALIZER;
static uint16_t                 certChangeTimeoutTestCount;
static bool_t                   randomCertificate = TRUE;
static uint8_t                  signErrCount[256];
static pthread_mutex_t          aerolinkCallMutex = PTHREAD_MUTEX_INITIALIZER;
/* The following variable controls the ASN.1 encoding method. 0 means DER. 1
 * means UPER. If is currently set to 1, because I assume that v3 security is
 * only used with BSM's that use UPER encoding. If this ever changes. it will
 * be necessary to write a function in alsmi to set ASN1EncodeMethod (and
 * possibly get it). Then a function in the Radio_ns resource manager will need
 * to handle messages to call these functions. Then a new function will need to
 * be added to alsmi_api to allow an application to send these message to the
 * Radio_ns resource manager. Finally, applications such as wmh will need to
 * call the new alsmi_api function to set ASN1EncodeMethod. Probably this will
 * need to be called whenever the configuration is either first read or
 * changed. */
static int                      ASN1EncodeMethod = 1;

/* Prototypes */
static void alsmiIDChangeInitCallback(void    *userData,
                                      uint8_t  numCerts,
                                      uint8_t *certIndex);
static void alsmiIDChangeDoneCallback(AEROLINK_RESULT  returnCode,
                                      void            *userData,
                                      uint8_t const   *certId);
static void alsmiCertChangeTimeout(void);
int32_t alsmiSendCertChangeReq(void);
AEROLINK_RESULT dummy_securityServices_idChangeRegister(
        SecurityContextC             scHandle,
        char const                  *lcmName,
        void                        *userData,
        IdChangeInitCallback         idChangeInitCallbackFunction,
        IdChangeDoneCallback         idChangedoneCallbackFunction);
AEROLINK_RESULT dummy_securityServices_idChangeUnregister(
        SecurityContextC             scHandle,
        char const                  *lcmName);
AEROLINK_RESULT dummy_securityServices_idChangeInit(void);
AEROLINK_RESULT dummy_smg_new(
    SecurityContextC          scHandle,
    SecuredMessageGeneratorC *smgHandle);
AEROLINK_RESULT dummy_smg_createUnsecuredData(
    SecuredMessageGeneratorC smgHandle,
    uint8_t const * const appPayload,
    uint32_t              appPayloadLen,
    uint8_t       * const unsecuredDataSPDU,
    uint32_t      * const unsecuredDataSPDULen);
AEROLINK_RESULT dummy_smg_signAsync(
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
AEROLINK_RESULT dummy_smp_new(
        SecurityContextC              scHandle,
        SecuredMessageParserC * const smpHandle);
AEROLINK_RESULT dummy_smp_extract(
    SecuredMessageParserC  smpHandle,
    uint8_t const *        spdu,
    uint32_t               spduLen,
    PayloadType *          spduType,
    uint8_t const **       payload,
    uint32_t *             payloadLenOut,
    PayloadType *          payloadType,
    uint8_t const **       externalDataHash,
    ExternalDataHashAlg *  externalDataHashAlg);
AEROLINK_RESULT dummy_smp_getPsid(
    SecuredMessageParserC  smpHandle,
    uint32_t       * const psid);
AEROLINK_RESULT dummy_smp_getServiceSpecificPermissions(
    SecuredMessageParserC   smpHandle,
    uint8_t const  **       ssp,
    uint32_t        * const sspLen);
AEROLINK_RESULT dummy_smp_verifySignaturesAsync(
    SecuredMessageParserC smpHandle,
    void *userData,
    ValidateCallback callbackFunction);
AEROLINK_RESULT dummy_sc_open(
        char const       *filename,
        SecurityContextC *scHandle);

/*
 * Timeout thread
 */
static pthread_t alsmiTimeoutThreadId;
static bool_t    alsmiTimeoutThreadRun = FALSE;

/* Certificate change timeout */
#define INITIAL_CERT_CHANGE_TIMEOUT_MS  5000
#define CERT_CHANGE_TIMEOUT_MS          50
#define START_CERT_CHANGE_TIMER \
    shm_rsk_ptr->ALSMICnt.StartCertChangeTimer++; \
    alsmiSetCertChangeTimeout(initialCertChangeDone ? CERT_CHANGE_TIMEOUT_MS : INITIAL_CERT_CHANGE_TIMEOUT_MS)
#define CANCEL_CERT_CHANGE_TIMER \
    shm_rsk_ptr->ALSMICnt.CancelCertChangeTimer++; \
    alsmiSetCertChangeTimeout(0)
static bool_t          initialCertChangeDone = FALSE;
static uint32_t        certChangeTimeout = 0;
static pthread_mutex_t certChangeTimeoutMutex = PTHREAD_MUTEX_INITIALIZER;
#define MAX_CERT_CHANGE_TRIES 1
static uint8_t         certChangeTries = 0;

#ifdef CERTIFICATE_CHANGE_TIMING_TEST
static struct timeval cctt_tv1;
#endif // CERTIFICATE_CHANGE_TIMING_TEST

/* Definitions having to do with whether or not to get the generation
 * location from the body of the BSM */
#define PROFILE_DIR                        "/etc/aerolink/security-profile"
#define MAX_GET_GEN_LOC_FROM_BSM_TABLE_LEN 16
#define MAX_PROF_NAME_LEN                  128
#define MAX_PROF_LINE_LEN                  128

typedef struct {
    uint32_t psid;
    bool_t   getGenLocFromBSM;
} get_gen_loc_from_body_t;

static int getGenLocFromBSMTableLen = 0;
static get_gen_loc_from_body_t getGenLocFromBSMTable[MAX_GET_GEN_LOC_FROM_BSM_TABLE_LEN];

static uint64_t dummy_handle_counter = 1;

/*
 * This function sets the value of certChangeTimeout atomically.
 * Input parameters:  value - The value to set certChangeTimeout to
 * Output parameters: None
 * Return Value:      None
 */
static inline void alsmiSetCertChangeTimeout(uint32_t value)
{
    ALSMI_DEBUG_PRINTF("Set certificate change timeout to %u\n", value);
    pthread_mutex_lock(&certChangeTimeoutMutex);
    certChangeTimeout = value;
    pthread_mutex_unlock(&certChangeTimeoutMutex);
}

/* This function does a securityServices_idChangeLock if certificate change is
 * not already locked */
AEROLINK_RESULT alsmi_idChangeLock(void)
{
    AEROLINK_RESULT result = WS_SUCCESS;

    pthread_mutex_lock(&certChangeLockMutex);

    if (!certChangeLocked) {
        shm_rsk_ptr->ALSMICnt.IdChangeLockRequests++;
        AEROLINK_CALL_PRINTF("AEROLINK: %s: Calling securityServices_idChangeLock()\n",
                             __func__);
        result = (secmode == 0) ? WS_SUCCESS : securityServices_idChangeLock();
        AEROLINK_CALL_PRINTF("AEROLINK: %s: securityServices_idChangeLock() returned %s\n",
                             __func__, ws_strerror(result));

        if (result == WS_SUCCESS) {
            shm_rsk_ptr->ALSMICnt.IdChangeLockSuccesses++;
            certChangeLocked = TRUE;
        }
        else {
            shm_rsk_ptr->ALSMICnt.IdChangeLockFailures++;
            shm_rsk_ptr->ALSMICnt.IdChangeLockLastErrorCode = result;
        }
    }

    pthread_mutex_unlock(&certChangeLockMutex);
    return result;
}

/* This function does a securityServices_idChangeUnlock only if the certificate
 * change is in fact locked */
AEROLINK_RESULT alsmi_idChangeUnlock(void)
{
    AEROLINK_RESULT result = WS_SUCCESS;

    pthread_mutex_lock(&certChangeLockMutex);

    if (certChangeLocked) {
        shm_rsk_ptr->ALSMICnt.IdChangeUnlockRequests++;
        AEROLINK_CALL_PRINTF("AEROLINK: %s: Calling securityServices_idChangeUnlock()\n",
                             __func__);
        result = (secmode == 0) ? WS_SUCCESS : securityServices_idChangeUnlock();
        AEROLINK_CALL_PRINTF("AEROLINK: %s: securityServices_idChangeUnlock() returned %s\n",
                             __func__, ws_strerror(result));

        if (result == WS_SUCCESS) {
            shm_rsk_ptr->ALSMICnt.IdChangeUnlockSuccesses++;
            certChangeLocked = FALSE;
        }
        else {
            shm_rsk_ptr->ALSMICnt.IdChangeUnlockFailures++;
            shm_rsk_ptr->ALSMICnt.IdChangeUnlockLastErrorCode = result;
        }
    }

    pthread_mutex_unlock(&certChangeLockMutex);
    return result;
}

/*
 * This thread handles the various timeouts. The timeout is given in ms.
 * The timeouts have a precision of 10ms.
 * Input parameters:  arg - Required by convention, but not used
 * Output parameters: None
 * Return Value:      Unused void *
 */
static void *alsmiTimeoutThread(void *arg)
{
    bool_t callAlsmiCertChangeTimeout = FALSE;

    ALSMI_DEBUG_PRINTF("alsmiTimeoutThread started\n");
    alsmiTimeoutThreadRun = TRUE;

    shm_rsk_ptr->bootupState |= 0x80; // alsmiTimeoutThread running
    while (alsmiTimeoutThreadRun) {
        /* Sleep for 10ms */
        usleep(10000);

        if (!alsmiTimeoutThreadRun) {
            break;
        }

        /*********************/
        /* certChangeTimeout */
        /*********************/
        pthread_mutex_lock(&certChangeTimeoutMutex);

        if (certChangeTimeout != 0) {
            if (certChangeTimeout < 10) {
                certChangeTimeout = 0;
            }
            else {
                certChangeTimeout -= 10;
            }

            if (certChangeTimeout == 0) {
                callAlsmiCertChangeTimeout = TRUE;
            }
        }

        pthread_mutex_unlock(&certChangeTimeoutMutex);

        if (callAlsmiCertChangeTimeout) {
            ALSMI_DEBUG_PRINTF("Certificate change timed out\n");
            alsmiCertChangeTimeout();
        }
    }

    shm_rsk_ptr->bootupState &= ~0x80; // alsmiTimeoutThread running
    ALSMI_DEBUG_PRINTF("alsmiTimeoutThread exiting\n");
    pthread_exit(NULL);
    return NULL;
}

/*
 * This function converts a PSID pointed to by "psid" from decimal to
 * P-encoded.
 *
 * Input parameters:  psid - Pointer to the PSID to be converted.
 * Output parameters: psid - PSID pointed to by this pointer converted from
 *                           decimal to P-encoded.
 * Return Value:      Success(0) or failure(-1).
 */
static int decimal2PCoded(uint32_t *psid)
{
    /* Convert PSID from decimal to P-encoded */
    if (*psid < 0x80) {
        /* Do nothing */
    }
    else if (*psid < 0x4080) {
        *psid += 0x7f80;
    }
    else if (*psid < 0x204080) {
        *psid += 0xbfbf80;
    }
    else if (*psid < 0x10204080) {
        *psid += 0xdfdfbf80;
    }
    else {
        /* Invalid PSID */
        return -1;
    }

    return 0;
}

/*
 * This function converts a PSID pointed to by "psid" from P-encoded to
 * decimal.
 *
 * Input parameters:  psid - Pointer to the PSID to be converted.
 * Output parameters: psid - PSID pointed to by this pointer converted from
 *                           P-encoded to decimal.
 * Return Value:      Success(0) or failure(-1).
 */
static int pCoded2Decimal(uint32_t *psid)
{
    /* Convert PSID from P-encoded to decimal */
    if (*psid <= 0x7f) {
        /* Leave it alone */
    }
    else if ((*psid >= 0x8000) && (*psid <= 0xbfff)) {
        *psid -= 0x7f80; /* 0x80..0x407f */
    }
    else if ((*psid >= 0xc00000) && (*psid <= 0xdfffff)) {
        *psid -= 0xbfbf80; /* 0x4080..0x20407f */
    }
    else if ((*psid >= 0xe0000000) && (*psid <= 0xefffffff)) {
        *psid -= 0xdfdfbf80; /* 0x204080..0x1020407f */
    }
    else {
        /* Invalid PSID */
        return -1;
    }

    return 0;
}

/*
 * This function parses a single profile file. It returns the values of the
 * parameters "psid", "include_generation_location",
 * "check_relevance_generation_location", and
 * "check_consistency_generation_location".
 *
 * Input parameters:  name - Name of the profile file to parse, without the
 *                           path.
 * Output parameters: ppsid - Pointer to where to write the PSID value.
 *                    pinclude_generation_location - Pointer to where to write
 *                        the "include_generation_location" value.
 *                    pcheck_relevance_generation_location - Pointer to where
 *                        to write the "check_relevance_generation_location"
 *                        value.
 *                    pcheck_consistency_generation_location - Pointer to where
 *                        to write the "check_consistency_generation_location"
 *                        value.
 * Return Value:      Success(0) or failure(-1).
 */
static int alsmiParseProf(const char *name, uint32_t *ppsid,
                          bool_t *pinclude_generation_location,
                          bool_t *pcheck_relevance_generation_location,
                          bool_t *pcheck_consistency_generation_location)
{
    FILE     *fd;
    char      fname[MAX_PROF_NAME_LEN];
    char      line[MAX_PROF_LINE_LEN];
    uint32_t  lineLen;
    char     *res, *ptr;
    uint32_t  psid;
    bool_t    include_generation_location                = FALSE;
    bool_t    check_relevance_generation_location        = FALSE;
    bool_t    check_consistency_generation_location      = FALSE;
    bool_t    psid_seen                                  = FALSE;
    bool_t    include_generation_location_seen           = FALSE;
    bool_t    check_relevance_generation_location_seen   = FALSE;
    bool_t    check_consistency_generation_location_seen = FALSE;

    /* Open the profile file */
    sprintf(fname, "%s/%s", PROFILE_DIR, name);
    fd = fopen(fname, "r");

    /* Error is fopen() failed */
    if (fd == NULL) {
        return -1;
    }

    do {
        /* Get the next line in the file */
        res = fgets(line, MAX_PROF_LINE_LEN, fd);

        if (res != NULL) {
            /* Erase any comment from the end of the line */
            ptr = line;

            while (*ptr != '\0') {
                if (*ptr == '#') {
                    *ptr = '\0';
                    break;
                }

                ptr++;
            }

            /* Convert line to lower case */
            ptr = line;

            while (*ptr != '\0') {
                *ptr = tolower(*ptr);
                ptr++;
            }

            /* Check to see if the line starts with the string "psid",
             * "include_generation_location", "check_relevance_generation_location",
             * or "check_consistency_generation_location" */
            lineLen = strlen(line);

            if ((lineLen > sizeof("psid")) && (strncmp(line, "psid", sizeof("psid") - 1) == 0)) {
                /* "psid" parameter; convert it to an integer */
                ptr = strstr(line, "0x");

                if (ptr != NULL) {
                    sscanf(&ptr[2], "%x", &psid);

                    if (decimal2PCoded(&psid) == -1) {
                        /* Invalid PSID */
                        fprintf(stdout,"%s: Invalid PSID (0x%x) in %s\n", __func__,
                                psid, name);
                        fflush(stdout);
                        fclose(fd);
                        return -1;
                    }

                    psid_seen = TRUE;
                }
            }
            else if ((lineLen > sizeof("include_generation_location")) &&
                     (strncmp(line, "include_generation_location",
                              sizeof("include_generation_location") - 1) == 0)) {
                /* "include_generation_location" parameter; see if the string
                 * "true" lies in the line */
                ptr = strstr(line, "true");

                if (ptr != NULL) {
                    /* If "true" is in the line, the value must be "true" */
                    include_generation_location      = TRUE;
                    include_generation_location_seen = TRUE;
                }
                else {
                    /* If "true" is not in the line, assume the value must be
                     * "false" */
                    include_generation_location      = FALSE;
                    include_generation_location_seen = TRUE;
                }
            }
            else if ((lineLen > sizeof("check_relevance_generation_location")) &&
                     (strncmp(line, "check_relevance_generation_location",
                              sizeof("check_relevance_generation_location") - 1) == 0)) {
                /* "check_relevance_generation_location" parameter; see if the
                 * string "true" lies in the line */
                ptr = strstr(line, "true");

                if (ptr != NULL) {
                    /* If "true" is in the line, the value must be "true" */
                    check_relevance_generation_location      = TRUE;
                    check_relevance_generation_location_seen = TRUE;
                }
                else {
                    /* If "true" is not in the line, assume the value must be
                     * "false" */
                    check_relevance_generation_location      = FALSE;
                    check_relevance_generation_location_seen = TRUE;
                }
            }
            else if ((lineLen > sizeof("check_consistency_generation_location")) &&
                     (strncmp(line, "check_consistency_generation_location",
                              sizeof("check_consistency_generation_location") - 1) == 0)) {
                /* "check_consistency_generation_location" parameter; see if
                 * the string "true" lies in the line */
                ptr = strstr(line, "true");

                if (ptr != NULL) {
                    /* If "true" is in the line, the value must be "true" */
                    check_consistency_generation_location      = TRUE;
                    check_consistency_generation_location_seen = TRUE;
                }
                else {
                    /* If "true" is not in the line, assume the value must be
                     * "false" */
                    check_consistency_generation_location      = FALSE;
                    check_consistency_generation_location_seen = TRUE;
                }
            }
        }
    } while (res != NULL);

    fclose(fd);

    /* Result is valid only if "psid", "include_generation_location",
     * "check_relevance_generation_location", and
     * "check_consistency_generation_location" all seen. If so set the output
     * values and return success. */
    if (psid_seen && include_generation_location_seen &&
        check_relevance_generation_location_seen &&
        check_consistency_generation_location_seen) {
        *ppsid                                  = psid;
        *pinclude_generation_location           = include_generation_location;
        *pcheck_relevance_generation_location   = check_relevance_generation_location;
        *pcheck_consistency_generation_location = check_consistency_generation_location;
        return 0;
    }

    return -1;
}

/*
 * This function parses all of the profile files and populates the table to
 * determine whether or not to get the generation location from the BSM based
 * on PSID.
 *
 * Input parameters:  None
 * Output parameters: Get generation location from BSM" table is populated.
 * Return Value:      None
 */
static void alsmiParseProfs(void)
{
    DIR           *dir;
    struct dirent *dent;
    int            res;
    uint32_t       psid;
    bool_t         include_generation_location;
    bool_t         check_relevance_generation_location;
    bool_t         check_consistency_generation_location;

    dir = opendir(PROFILE_DIR);

    if (dir != NULL) {
        dent = readdir(dir);

        while (dent != NULL) {
            if ((strlen(dent->d_name) >= sizeof(".prof") - 1) &&
                (strncmp(&dent->d_name[strlen(dent->d_name) - (sizeof(".prof") - 1)],
                         ".prof", sizeof(".prof") - 1) == 0)) {
                /* Parse this profile file */
                res = alsmiParseProf(dent->d_name, &psid,
                                     &include_generation_location,
                                     &check_relevance_generation_location,
                                     &check_consistency_generation_location);

                /* If parse was successful, put a new entry into the table */
                if (res == 0) {
                    /* Make 0x20 the highest priority PSID by putting the PSID
                     * currently in location 0 in the table into the current
                     * location in the table, and by putting PSID 0x20 into
                     * location 0 in the table (unless location 0 in the table
                     * IS the current location) */
                    if ((psid == 0x20) && (getGenLocFromBSMTableLen != 0)) {
                        getGenLocFromBSMTable[getGenLocFromBSMTableLen].psid =
                            getGenLocFromBSMTable[0].psid;
                        getGenLocFromBSMTable[getGenLocFromBSMTableLen].getGenLocFromBSM =
                            getGenLocFromBSMTable[0].getGenLocFromBSM;
                        getGenLocFromBSMTable[0].psid = psid;
                        getGenLocFromBSMTable[0].getGenLocFromBSM =
                            !include_generation_location &&
                            (check_relevance_generation_location ||
                             check_consistency_generation_location);
                    }
                    else {
                        getGenLocFromBSMTable[getGenLocFromBSMTableLen].psid = psid;
                        getGenLocFromBSMTable[getGenLocFromBSMTableLen].getGenLocFromBSM =
                            !include_generation_location &&
                            (check_relevance_generation_location ||
                             check_consistency_generation_location);
                    }

                    getGenLocFromBSMTableLen++;

                    /* Stop processing if table full */
                    if (getGenLocFromBSMTableLen == MAX_GET_GEN_LOC_FROM_BSM_TABLE_LEN) {
                        break;
                    }
                }
            }

            dent = readdir(dir);
        }

        closedir(dir);
    }
}

/*
 * This function sets debug_log equal to the value of the nsconfig
 * ALSMI_DEBUG_LOG_ENABLE value.
 * Input parameters:  None
 * Output parameters: None
 * Return Value:      None
 */
void alsmiSetDebugLogValue(void)
{
    if(0x0 == getALSMIDebugLogEnable())
        debug_log = FALSE;
    else
        debug_log = TRUE;
}

/*
 * This function should be called by Radio_ns during startup.
 * Input parameters:  None
 * Output parameters: None
 * Return Value:      Success(0) or failure(-1).
 */
int32_t alsmiInitialize(void)
{
    AEROLINK_RESULT  result;
    char            *contextName = "denso.wsc";
    int              i;
    bool_t           securityInitialized = FALSE;
    bool_t           contextOpened       = FALSE;
    bool_t           signSmgCreated      = FALSE;
    bool_t           stripSmpCreated     = FALSE;
    bool_t           verifySmpCreated    = FALSE;
    bool_t           mutexesInitialized  = FALSE;
    const char      *version;

    /* Set the global variables to their initial value */
    debug                       = FALSE;
    alsmiSetDebugLogValue();
    alsmiVodEnabled             = FALSE;
    sign_wsa_results_options    = smi_results_none;
    sign_wsa_results_pid        = 0;
    verify_wsa_results_options  = smi_results_none;
    verify_wsa_results_pid      = 0;
    cert_change_pid             = 0;
    sign_smgInUse               = FALSE;
    verify_smpInUse             = FALSE;
    certificateChangeInProgress = FALSE;
    useCount                    = 0;
    lastLeapSecondsSince2004    = 0;
    memset(signErrCount,   0, 256);

    /* Remove any previous ALSMI debug log */
    if (debug_log) {
        system("rm /mnt/rwflash/dbglogs/alsmi_debug.log");
    }

    for (i = 0; i < ALSMI_NUM_REGISTRATIONS; i++) {
        regTable[i].inUse = FALSE;
    }

    /* Populate the "get generation location from BSM" table */
    alsmiParseProfs();

    /* Print security software version */
    AEROLINK_CALL_PRINTF("AEROLINK: %s: Calling securityServices_getVersion()\n",
                         __func__);
    version = (secmode == 0) ? "0.0.0.0" : securityServices_getVersion();
    AEROLINK_CALL_PRINTF("AEROLINK: %s: securityServices_getVersion() returned \"%s\"\n",
                         __func__, version);
    fprintf(stdout,"Aerolink version is %s\n", version);
    fflush(stdout);

    /* Initialize security services */
    AEROLINK_CALL_PRINTF("AEROLINK: %s: Calling securityServices_initialize()\n",
                         __func__);
    result = (secmode == 0) ? WS_SUCCESS : securityServices_initialize();
    AEROLINK_CALL_PRINTF("AEROLINK: %s: securityServices_initialize() returned %s\n",
                         __func__, ws_strerror(result));

    if (result != WS_SUCCESS) {
        fprintf(stdout,"%s: securityServices_initialize() failed (%s)\n",
                __func__, ws_strerror(result));
        fflush(stdout);
        goto alsmiInitialize_err;
    }

    securityInitialized = TRUE;

    /* Open applications security context */
    AEROLINK_CALL_PRINTF("AEROLINK: %s: Calling sc_open(\"%s\", %p)\n",
                         __func__, contextName, &context);
    result = (secmode == 0) ? dummy_sc_open(contextName, &context) :
             sc_open(contextName, &context);
    AEROLINK_CALL_PRINTF("AEROLINK: %s: sc_open() returned %d; context = %p\n",
                         __func__, result, context);

    if (result != WS_SUCCESS) {
        fprintf(stdout,"%s: Failed to open security context (%s)\n",
                __func__, ws_strerror(result));
        fflush(stdout);
        goto alsmiInitialize_err;
    }

    contextOpened = TRUE;

    /* Create Secured Message Generator */
    AEROLINK_CALL_PRINTF("AEROLINK: %s: Calling smg_new(%p, %p)\n",
                         __func__, context, &sign_smg);
    result = (secmode == 0) ? dummy_smg_new(context, &sign_smg) :
                              smg_new(context, &sign_smg);
    AEROLINK_CALL_PRINTF("AEROLINK: %s: smg_new() returned %d; sign_smg = %p\n",
                         __func__, result, sign_smg);

    if (result != WS_SUCCESS) {
        fprintf(stdout,"%s: Unable to create a signed message generator (%s)\n",
                __func__, ws_strerror(result));
        fflush(stdout);
        goto alsmiInitialize_err;
    }

    signSmgCreated = TRUE;

    /* Create Strip Secured Message Parser */
    AEROLINK_CALL_PRINTF("AEROLINK: %s: Calling smp_new(%p, %p)\n",
                         __func__, context, &strip_smp);
    result = (secmode == 0) ? dummy_smp_new(context, &strip_smp) :
             smp_new(context, &strip_smp);
    AEROLINK_CALL_PRINTF("AEROLINK: %s: smp_new() returned %d; strip_smp = %p\n",
                         __func__, result, strip_smp);

    if (result != WS_SUCCESS) {
        fprintf(stdout,"%s: Unable to create a secured message parser (%s)\n",
                __func__, ws_strerror(result));
        fflush(stdout);
        goto alsmiInitialize_err;
    }

    stripSmpCreated = TRUE;

    /* Create VTP Secured Message Parser */
    AEROLINK_CALL_PRINTF("AEROLINK: %s: Calling smp_new(%p, %p)\n",
                         __func__, context, &vtp_smp);
    result = (secmode == 0) ? dummy_smp_new(context, &vtp_smp) :
             smp_new(context, &vtp_smp);
    AEROLINK_CALL_PRINTF("AEROLINK: %s: smp_new() returned %d; vtp_smp = %p\n",
                         __func__, result, vtp_smp);

    if (result != WS_SUCCESS) {
        fprintf(stdout,"%s: Unable to create a secured message parser (%s)\n",
                __func__, ws_strerror(result));
        fflush(stdout);
        goto alsmiInitialize_err;
    }

    verifySmpCreated = TRUE;

    /* Disable ID-change protocol (it will be enabled when we request a
     * certificate change. ID-change protocol will be initiated anyway when
     * a currently in-use certificate is about to expire). */
    result = alsmi_idChangeLock();

    if (result != WS_SUCCESS) {
        fprintf(stdout,"%s: Unable to block ID-change protocol (%s)\n",
                __func__, ws_strerror(result));
        fflush(stdout);
    }

    /* Initialize mutexes */
    pthread_mutex_init(&regTableMutex,          NULL);
    pthread_mutex_init(&signBufferMutex,        NULL);
    pthread_mutex_init(&verifyBufferMutex,      NULL);
    pthread_mutex_init(&certChangeTimeoutMutex, NULL);
    pthread_mutex_init(&certChangeLockMutex,    NULL);
    pthread_mutex_init(&aerolinkCallMutex,      NULL);
    mutexesInitialized = TRUE;

    /* Invalidate all sign buffer entries */
    for (i = 0; i < NUM_SIGN_BUFFER_ELEMENTS; i++) {
        signBuffer[i].signHandle = -1;
        signBuffer[i].signingSubmitted = FALSE;
    }

    signBufferNextHandle = 0;

    /* Invalidate all verify buffer entries */
    for (i = 0; i < NUM_VERIFY_BUFFER_ELEMENTS; i++) {
        verifyBuffer[i].vodMsgSeqNum = 0;
        verifyBuffer[i].verificationSubmitted = FALSE;
    }

    /* Don't use 0 or 1 as a VOD message sequence number. A VOD message
     * sequence number value of 0 indicates that the packet has been
     * successfully verified by VTP. A VOD message sequence number value of 1
     * indicates that the packet failed verification by VTP. */
    verifyBufferNextHandle = 2;

    /* Launch the timeout thread */
    if (pthread_create(&alsmiTimeoutThreadId, NULL, alsmiTimeoutThread, NULL) != 0) {
        fprintf(stdout,"%s: Error creating alsmiTimeoutThread: %s\n",
                __func__, strerror(errno));
        fflush(stdout);
        goto alsmiInitialize_err;
    }

    ALSMI_DEBUG_PRINTF("%s: Created alsmiTimeoutThread\n", __func__);

    /* Register peer-to-peer callback function */
    if (p2pEnabled && (secmode != 0)) {
        if ((result = p2p_setCallback(NULL, wsuWMEP2PCallback)) != WS_SUCCESS) {
            fprintf(stdout,"%s: Error registering peer-to-peer callback function (%s)\n",
                    __func__, ws_strerror(result));
            fflush(stdout);
            goto alsmiInitialize_err;
        }
    }

    alsmiInitializeCalled = TRUE;
    return 0;

alsmiInitialize_err:
    if (p2pEnabled && (secmode != 0)) {
        p2p_setCallback(NULL, NULL);
    }

    if (mutexesInitialized) {
        pthread_mutex_destroy(&regTableMutex);
        pthread_mutex_destroy(&signBufferMutex);
        pthread_mutex_destroy(&verifyBufferMutex);
        pthread_mutex_destroy(&certChangeTimeoutMutex);
        pthread_mutex_destroy(&certChangeLockMutex);
        pthread_mutex_destroy(&aerolinkCallMutex);
    }

    if (verifySmpCreated) {
        AEROLINK_CALL_PRINTF("AEROLINK: %s: Calling smp_delete(%p)\n", __func__, vtp_smp);

        if (secmode != 0) {
            smp_delete(vtp_smp);
        }

        AEROLINK_CALL_PRINTF("AEROLINK: %s: smp_delete() returned\n", __func__);
    }

    if (stripSmpCreated) {
        AEROLINK_CALL_PRINTF("AEROLINK: %s: Calling smp_delete(%p)\n", __func__, strip_smp);

        if (secmode != 0) {
            smp_delete(strip_smp);
        }

        AEROLINK_CALL_PRINTF("AEROLINK: %s: smp_delete() returned\n", __func__);
    }

    if (signSmgCreated) {
        AEROLINK_CALL_PRINTF("AEROLINK: %s: Calling smg_delete(%p)\n", __func__, sign_smg);

        if (secmode != 0) {
            smg_delete(sign_smg);
        }

        AEROLINK_CALL_PRINTF("AEROLINK: %s: smg_delete() returned\n", __func__);
    }

    if (contextOpened) {
        AEROLINK_CALL_PRINTF("AEROLINK: %s: Calling sc_close(%p)\n", __func__, context);
        
        if (secmode != 0) {
            sc_close(context);
        }

        AEROLINK_CALL_PRINTF("AEROLINK: %s: sc_close() returned\n", __func__);
    }

    if (securityInitialized) {
        if (secmode != 0) {
            AEROLINK_CALL_PRINTF("AEROLINK: %s: Calling securityServices_shutdown()\n",
                                 __func__);
            fprintf(stdout,"AEROLINK: %s: Calling securityServices_shutdown()\n", __func__);
            result = securityServices_shutdown();
            AEROLINK_CALL_PRINTF("AEROLINK: %s: securityServices_shutdown() returned %s\n",
                                 __func__, ws_strerror(result));
            fprintf(stdout,"AEROLINK: %s: securityServices_shutdown() returned %s\n",
                    __func__, ws_strerror(result));
            fflush(stdout);
        }
    }

    return -1;
}

/*
 * Function to close and cleanup the resources. Radio_ns should call this
 * during shutdown.
 * Input parameters:  None
 * Output parameters: None
 * Return Value:      None
 */
void alsmiTerm(void)
{
    alsmiInitializeCalled = FALSE;

    /* Unregister peer-to-peer callback function */
    if (p2pEnabled && (secmode != 0)) {
        p2p_setCallback(NULL, NULL);
    }

    /* Terminate the timeout thread */
    alsmiTimeoutThreadRun = FALSE;
    pthread_join(alsmiTimeoutThreadId, NULL);

    AEROLINK_CALL_PRINTF("AEROLINK: %s: Calling smp_delete(%p)\n",
                         __func__, vtp_smp);

    if (secmode != 0) {
        smp_delete(vtp_smp);
    }

    AEROLINK_CALL_PRINTF("AEROLINK: %s: smp_delete() returned\n", __func__);
    AEROLINK_CALL_PRINTF("AEROLINK: %s: Calling smp_delete(%p)\n",
                         __func__, strip_smp);

    if (secmode != 0) {
        smp_delete(strip_smp);
    }

    AEROLINK_CALL_PRINTF("AEROLINK: %s: smp_delete() returned\n", __func__);
    AEROLINK_CALL_PRINTF("AEROLINK: %s: Calling smg_delete(%p)\n",
                         __func__, sign_smg);

    if (secmode != 0) {
        smg_delete(sign_smg);
    }

    AEROLINK_CALL_PRINTF("AEROLINK: %s: smg_delete() returned\n", __func__);
    AEROLINK_CALL_PRINTF("AEROLINK: %s: Calling sc_close(%p)\n", __func__, context);

    if (secmode != 0) {
        sc_close(context);
    }

    AEROLINK_CALL_PRINTF("AEROLINK: %s: sc_close() returned\n", __func__);
    AEROLINK_CALL_PRINTF("AEROLINK: %s: Calling securityServices_shutdown()\n", __func__);
    fprintf(stdout,"AEROLINK: %s: Calling securityServices_shutdown()\n", __func__);

    if (secmode == 0) {
        securityServices_shutdown();
    }

    AEROLINK_CALL_PRINTF("AEROLINK: %s: securityServices_shutdown() returned.\n", __func__);
    fprintf(stdout,"AEROLINK: %s: securityServices_shutdown() returned.\n", __func__);
    fflush(stdout);

    /* Destroy mutexes */
    pthread_mutex_destroy(&regTableMutex);
    pthread_mutex_destroy(&signBufferMutex);
    pthread_mutex_destroy(&verifyBufferMutex);
    pthread_mutex_destroy(&certChangeTimeoutMutex);
    pthread_mutex_destroy(&certChangeLockMutex);
    pthread_mutex_destroy(&aerolinkCallMutex);
}

/*
 * This function should be called by an application to register for signing
 * packets with security.
 * Input parameters:
 *        debug_flag_forsmi   : Debug flag. When enabled creates debug file in the path as specified in psmiConfigParams->debugLogFilename
 *        smi_vodEnabled : TRUE if VOD is enabled, FALSE otherwise
 *        vtp_milliseconds_per_verification : Millieconds per verification in VTP mode.
 *        smi_sign_results_options : Which sign results to return.
 *        smi_sign_wsa_results_options : Which sign WSA results to return.
 *        smi_verify_wsa_results_options : Which verify WSA results to return.
 *        lcmNameParam        : LCM name, obtained from the denso.wsc file.
 *        numSignResultsPSIDs : Number of PSID's to be signed.
 *        signResultsPSIDs    : PSID's to be signed. numSignResultsPSIDs is the length of the array.
 *        smi_sign_wsa_results_pid : Process ID of sign WSA results application.
 *        smi_verify_wsa_results_pid : Process ID of verify WSA results application.
 *        smi_cert_change_pid : Process ID of certificate change results application.
 *        pRegIdx             : Value passed back to the caller. This value is passed to the alsmiUnregister() function.
 *
 * Output parameters: None
 * Return Value:      Success(0) or failure(-1).
 */
int alsmiRegister(bool_t                        debug_flag_forsmi,
                  bool_t                        smi_vodEnabled,
                  uint32_t                      vtp_milliseconds_per_verification,
                  enum smi_results_e            smi_sign_results_options,
                  enum smi_results_e            smi_sign_wsa_results_options,
                  enum smi_results_e            smi_verify_wsa_results_options,
                  char                         *lcmNameParam,
                  uint8_t                       numSignResultsPSIDs,
                  uint32_t                     *signResultsPSIDs,
                  pid_t                         smi_sign_wsa_results_pid,
                  pid_t                         smi_verify_wsa_results_pid,
                  pid_t                         smi_cert_change_pid,
                  uint8_t                      *pRegIdx)
{
    AEROLINK_RESULT result;
    uint8_t          regIdx;

    ALSMI_DEBUG_PRINTF("%s int32_t alsmiRegister(%d, %d, %u, %d, %d, %d, \"%s\", %u, "
                       "{0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x}, "
                       "%u, %u, %u, %p)\n",
                       (useCount != 0) ? "subsequent" : "initial",
                       debug_flag_forsmi,
                       smi_vodEnabled,
                       vtp_milliseconds_per_verification,
                       smi_sign_results_options,
                       smi_sign_wsa_results_options,
                       smi_verify_wsa_results_options,
                       lcmNameParam,
                       numSignResultsPSIDs,
                       signResultsPSIDs[0], signResultsPSIDs[1], signResultsPSIDs[2],
                       signResultsPSIDs[3], signResultsPSIDs[4], signResultsPSIDs[5],
                       signResultsPSIDs[6], signResultsPSIDs[7],
                       signResultsPSIDs[8], signResultsPSIDs[9],
                       smi_sign_wsa_results_pid,
                       smi_verify_wsa_results_pid,
                       smi_cert_change_pid,
                       pRegIdx);

    shm_rsk_ptr->ALSMICnt.Register++;

    /* Error if two programs are using diffent values for alsmiVodEnabled or
     * vtp_milliseconds_per_verification. Normally, this will not be a problem,
     * since values are specified in common.conf, which every application
     * should make use of. */
    if (useCount != 0) {
        if (smi_vodEnabled != alsmiVodEnabled) {
            fprintf(stdout, "%s ERROR: Two programs using different vodEnabled values\n", __func__);
            fflush(stdout);
            return -1;
        }

        if (milliseconds_per_verification != vtp_milliseconds_per_verification) {
            fprintf(stdout, "%s ERROR: Two programs using different milliseconds per verification values (%u, %u)\n",
                    __func__, milliseconds_per_verification, vtp_milliseconds_per_verification);
            fflush(stdout);
            return -1;
        }
    }

    if (numSignResultsPSIDs > ALSMI_NUM_PSIDS_PER_REGISTRATION) {
        fprintf(stdout,"%s ERROR: numSignResultsPSIDs must be in the range of 0..%d\n", __func__, ALSMI_NUM_PSIDS_PER_REGISTRATION);
        fflush(stdout);
        return -1;
    }

    /* Turn on debugging if desired */
    if ((useCount == 0) || (!debug && debug_flag_forsmi)) {
        debug = debug_flag_forsmi;
    }

    /* Find an empty registration table entry */
    pthread_mutex_lock(&regTableMutex);

    for (regIdx = 0; regIdx < ALSMI_NUM_REGISTRATIONS; regIdx++) {
        if (!regTable[regIdx].inUse) {
            break;
        }
    }

    if (regIdx == ALSMI_NUM_REGISTRATIONS) {
        pthread_mutex_unlock(&regTableMutex);
        fprintf(stdout, "%s ERROR: Number of security registrations exceeded\n", __func__);
        fflush(stdout);
        return -1;
    }

    /* Get the PSID array */
    regTable[regIdx].numSignResultsPSIDs = numSignResultsPSIDs;
    memcpy(regTable[regIdx].signResultsPSIDs, signResultsPSIDs, regTable[regIdx].numSignResultsPSIDs * sizeof(signResultsPSIDs[0]));

    /* Save the sign results options */
    regTable[regIdx].sign_results_options = smi_sign_results_options;

    /* Save the sign WSA results options and pid */
    if (smi_sign_wsa_results_pid != 0) {
        if (sign_wsa_results_pid != 0) {
            fprintf(stdout, "%s ERROR: WSA sign results already registered for\n", __func__);
            fflush(stdout);
        }
        else {
            sign_wsa_results_options = smi_sign_wsa_results_options;
            sign_wsa_results_pid     = smi_sign_wsa_results_pid;
        }
    }

    /* Save the verify WSA results options and pid */
    if (smi_verify_wsa_results_pid != 0) {
        if (verify_wsa_results_pid != 0) {
            fprintf(stdout, "%s ERROR: WSA verify results already registered for\n", __func__);
            fflush(stdout);
        }
        else {
            verify_wsa_results_options = smi_verify_wsa_results_options;
            verify_wsa_results_pid     = smi_verify_wsa_results_pid;
        }
    }
    
    /* Save the certificate change pid */
    if (smi_cert_change_pid != 0) {
        if (cert_change_pid != 0) {
            fprintf(stdout, "%s ERROR: Certificate change results already registered for\n", __func__);
            fflush(stdout);
        }
        else {
            cert_change_pid = smi_cert_change_pid;
        }
    }

    /* If an LCM name is specified, save it and register it for IDchange */
    regTable[regIdx].lcmNameRegistered = FALSE;

    pthread_mutex_unlock(&regTableMutex);

    if (lcmNameParam[0] != '\0') {
        /* Save the LCM name in the registration table */
        memcpy(regTable[regIdx].lcmName, lcmNameParam, sizeof(regTable[regIdx].lcmName));
        regTable[regIdx].lcmName[sizeof(regTable[regIdx].lcmName) - 1] = '\0'; /* Just in case */

        /* Register LCM for ID change */
        /* Lock Aerolink calls */
        pthread_mutex_lock(&aerolinkCallMutex);
        AEROLINK_CALL_PRINTF("AEROLINK: %s: Calling securityServices_idChangeRegister(%p, \"%s\", \"%s\", %p, %p); "
                             "alsmiIDChangeInitCallback = %p; alsmiIDChangeDoneCallback = %p\n", __func__,
                             context,
                             regTable[regIdx].lcmName,
                             regTable[regIdx].lcmName,
                             alsmiIDChangeInitCallback,
                             alsmiIDChangeDoneCallback,
                             alsmiIDChangeInitCallback,
                             alsmiIDChangeDoneCallback);
        result = (secmode == 0) ?
                 dummy_securityServices_idChangeRegister(context,
                                                         regTable[regIdx].lcmName,
                                                         regTable[regIdx].lcmName, /* Send LCM name to the callbacks as user data */
                                                         alsmiIDChangeInitCallback,
                                                         alsmiIDChangeDoneCallback) :
                 securityServices_idChangeRegister(context,
                                                   regTable[regIdx].lcmName,
                                                   regTable[regIdx].lcmName, /* Send LCM name to the callbacks as user data */
                                                   alsmiIDChangeInitCallback,
                                                   alsmiIDChangeDoneCallback);
        AEROLINK_CALL_PRINTF("AEROLINK: %s: securityServices_idChangeRegister() returned %s\n", __func__, ws_strerror(result));
        /* Unlock Aerolink calls */
        pthread_mutex_unlock(&aerolinkCallMutex);

        if ((result != WS_SUCCESS) && (result != WS_ERR_ALREADY_REGISTERED)) {
            fprintf(stdout,"%s: Unable to register ID change callbacks (%s)\n",
                    __func__, ws_strerror(result));
            fflush(stdout);
            goto alsmiRegister_err;
        }

        if (result != WS_ERR_ALREADY_REGISTERED) {
            regTable[regIdx].lcmNameRegistered = TRUE;
        }

        /* Change to a random certificate */
        randomCertificate = TRUE;
        /* Be sure aerolinkCallMutex is unlocked here, or the following
         * function call will hang!! */
        alsmiSendCertChangeReq();
    }

    /* Initialize these values */
    alsmiVodEnabled                      = smi_vodEnabled;
    milliseconds_per_verification        = vtp_milliseconds_per_verification;
    scaled_milliseconds_per_verification = (double)milliseconds_per_verification;
    *pRegIdx                             = regIdx;
    ALSMI_DEBUG_PRINTF("%s: regIdx = %u\n", __func__, regIdx);
    useCount++;
    regTable[regIdx].inUse = TRUE;
    return 0;

alsmiRegister_err:
    regTable[regIdx].inUse = FALSE;

    if (useCount == 0) {
        debug = FALSE;
    }

    return -1;
}

/*
 * This function should be called by an application to unregister from signing
 * packets with security.
 * Input parameters:
 *        regIdx : Value returned from alsmiRegister()
 *        sign_wsa_results   : Set to TRUE if the process that called alsmiInitialize wanted sign WSA results
 *        verify_wsa_results : Set to TRUE if the process that called alsmiInitialize wanted verify WSA results
 *        cert_change        : Set to TRUE if the process that called alsmiInitialize wanted certificate change results
 * Output parameters:  None
 * Return Value:       Use count after terminating this instance.
 */
int alsmiUnregister(uint8_t regIdx, bool_t sign_wsa_results,
                    bool_t verify_wsa_results, bool_t cert_change_results)
{
    ALSMI_DEBUG_PRINTF("alsmiUnregister(%u, %d, %d, %d)\n", regIdx,
                       sign_wsa_results, verify_wsa_results, cert_change_results);

    shm_rsk_ptr->ALSMICnt.Unregister++;

    if (useCount == 0) {
        return 0;
    }

    if (regIdx > ALSMI_NUM_REGISTRATIONS) {
        fprintf(stdout,"%s ERROR: regIdx must be in the range of 0..%d\n", __func__, ALSMI_NUM_REGISTRATIONS);
        fflush(stdout);
        return -1;
    }

    if (regTable[regIdx].lcmNameRegistered) {
        /* Lock Aerolink calls */
        pthread_mutex_lock(&aerolinkCallMutex);
        AEROLINK_CALL_PRINTF("AEROLINK: %s: Calling securityServices_idChangeUnregister(%p, \"%s\")\n",
                             __func__, context, regTable[regIdx].lcmName);

        if (secmode == 0) {
            dummy_securityServices_idChangeUnregister(context, regTable[regIdx].lcmName);
        }
        else {
            securityServices_idChangeUnregister(context, regTable[regIdx].lcmName);
        }

        AEROLINK_CALL_PRINTF("AEROLINK: %s: securityServices_idChangeUnregister() returned\n", __func__);
        /* Unlock Aerolink calls */
        pthread_mutex_unlock(&aerolinkCallMutex);
    }

    pthread_mutex_lock(&regTableMutex);

    regTable[regIdx].lcmNameRegistered = FALSE;

    if (!regTable[regIdx].inUse) {
        pthread_mutex_unlock(&regTableMutex);
        return 0;
    }

    regTable[regIdx].inUse = FALSE;
    pthread_mutex_unlock(&regTableMutex);

    if (sign_wsa_results) {
        sign_wsa_results_pid = 0;
    }

    if (verify_wsa_results) {
        verify_wsa_results_pid = 0;
    }

    if (cert_change_results) {
        cert_change_pid = 0;
    }

    if (--useCount == 0) {
        sign_wsa_results_pid   = 0;
        verify_wsa_results_pid = 0;
        cert_change_pid        = 0;
        debug                  = FALSE;
    }

    return useCount;
}

/*
 * alsmiASN1ExtractJ2735_LatLonElev: Extrace the latitude, longitude, and
 * elevation from the body of a BSM.
 *
 * Input parameters:
 *             latDeg: Pointer to location to store the latitude in units of
 *                     degrees * 1e7.
 *             lonDeg: Pointer to location to store the longitude in units of
 *                     degrees * 1e7.
 *             elevM: Pointer to location to store the elevation in units of
 *                    meters * 10.
 *                    0x0000 to 0xefff represents 0 to 6143.9.
 *                    0xf001 to 0xffff represents -409.5 to -0.1.
 *                    0xf000 represents unknown.
 *             bsmData: Pointer to the BSM data.
 *             bsmDataLen: BSM data length.
 * Output parameters:
 *             Latitude, longitude, and elevation written to locations pointed
 *             to by latDeg, lonDeg, and elevM respectively.
 * Return Value:  WS_SUCCESS on Success WS_ERR_ASN1_DECODE_FAILED on failure.
 */
static AEROLINK_RESULT alsmiASN1ExtractJ2735_LatLonElev(
                  int32_t *latDeg, int32_t *lonDeg,
                  uint16_t *elevM,
                  const uint8_t *bsmData, uint16_t bsmDataLen)
{
    int32_t  lat;
    int32_t  lon;
    uint16_t elev;

    if (ASN1EncodeMethod == 0) { /* DER */
        uint32_t lenBytes = 0;

        if ((bsmData[0] == 0x30) && (bsmData[1] & 0x80)) {
            lenBytes = bsmData[1] & 0x7F;
        }

        if (bsmDataLen <= (lenBytes + 24)) {
            return WS_ERR_ASN1_DECODE_FAILED;
        }

        memcpy(&lat,  &bsmData[14 + lenBytes], 4);
        lat  = ntohl(lat);
        memcpy(&lon,  &bsmData[18 + lenBytes], 4);
        lon  = ntohl(lon);
        memcpy(&elev, &bsmData[22 + lenBytes], 2);
        elev = ntohs(elev);
    }
    else if (ASN1EncodeMethod == 1) { /* PER_U Unblobbed */
        uint32_t ulat;
        uint32_t ulon;
        uint8_t  uperBSMBOI = (bsmData[2] & 0x80) ? 1 : 0;

        if (bsmDataLen < (uperBSMBOI + 21)) {
            return WS_ERR_ASN1_DECODE_FAILED;
        }

        /* Signed values are encoded subtracting lower bound value */
        /* Encoded as 31 bit unsigned value (fit the range) for latitude */
        /* So use uint32_t to copy the bits and then >> by 1 */
        ulat = (              (((bsmData[10 + uperBSMBOI] & 0x3F) << 2) |
                               ((bsmData[11 + uperBSMBOI] & 0xC0) >> 6)));
        ulat = ((ulat << 8) | (((bsmData[11 + uperBSMBOI] & 0x3F) << 2) |
                               ((bsmData[12 + uperBSMBOI] & 0xC0) >> 6)));
        ulat = ((ulat << 8) | (((bsmData[12 + uperBSMBOI] & 0x3F) << 2) |
                               ((bsmData[13 + uperBSMBOI] & 0xC0) >> 6)));
        ulat = ((ulat << 8) | (((bsmData[13 + uperBSMBOI] & 0x3F) << 2) |
                               ((bsmData[14 + uperBSMBOI] & 0x80) >> 6)));
        ulat >>= 1;
        lat = ulat - 900000000;

        ulon = (              (((bsmData[14 + uperBSMBOI] & 0x7F) << 1) |
                               ((bsmData[15 + uperBSMBOI] & 0x80) >> 7)));
        ulon = ((ulon << 8) | (((bsmData[15 + uperBSMBOI] & 0x7F) << 1) |
                               ((bsmData[16 + uperBSMBOI] & 0x80) >> 7)));
        ulon = ((ulon << 8) | (((bsmData[16 + uperBSMBOI] & 0x7F) << 1) |
                               ((bsmData[17 + uperBSMBOI] & 0x80) >> 7)));
        ulon = ((ulon << 8) | (((bsmData[17 + uperBSMBOI] & 0x7F) << 1) |
                               ((bsmData[18 + uperBSMBOI] & 0x80) >> 7)));
        lon = ulon - 1799999999;

        elev = (              (((bsmData[18 + uperBSMBOI] & 0x7F) << 1) |
                               ((bsmData[19 + uperBSMBOI] & 0x80) >> 7)));
        elev = ((elev << 8) | (((bsmData[19 + uperBSMBOI] & 0x7F) << 1) |
                               ((bsmData[20 + uperBSMBOI] & 0x80) >> 7)));
        elev -= 4096;
    }
    else {
        return WS_ERR_ASN1_DECODE_FAILED;
    }

    if (lat == 900000001) {
        lat = -900000001;
    }
    else if (lon == 1800000001) {
        lon = -1800000001;
    }

    *latDeg = lat;
    *lonDeg = lon;
    *elevM  = elev;
    return WS_SUCCESS;
}

/*
 * alsmiGetGenLocFromBsm: Get whether or not to get the generation location
 *                        from the body of the BSM based on PSID.
 *
 * Input parameters:
 *             psid: The PSID to check.
 * Output parameters:
 *             None.
 * Return Value: 1 if we want to get the generation location from the boday of
 *               the BSM, 0 if we don't, -1 if the PSID is not in the table.
 */
static int8_t alsmiGetGenLocFromBsm(uint32_t psid)
{
    int i;

    for (i = 0; i < getGenLocFromBSMTableLen; i++) {
        if (psid == getGenLocFromBSMTable[i].psid) {
            return getGenLocFromBSMTable[i].getGenLocFromBSM ? 1 : 0;
        }
    }

    return -1;
}

/*
 * alsmiDoUpdatePositionAndTime
 *
 * Does the actual time and position update by making the appropriate
 * Aerolink calls.
 *
 * Input parameters:
 *             latitude, longitude, elevation - Current GPS position
 *             leapSecondsSince2004 - Leap seconds since 1/1/2004, 00:00:00 UTC
 *             countryCode - field representing the country the device is
 *                           currently in
 * Output parameters:
 *             None
 * Return Value:  None
 */
void alsmiDoUpdatePositionAndTime(double latitude,  double  longitude,
                                  double elevation, int16_t leapSecondsSince2004,
                                  uint16_t countryCode)
{
    int32_t         scaledLatitude, scaledLongitude;
    int16_t         scaledElevation;
    AEROLINK_RESULT result;
    static bool_t   firstTime = TRUE;

    shm_rsk_ptr->ALSMICnt.UpdatePositionAndTime++;

    scaledLatitude = (latitude < -90.0) ? -900000000 :
                     (latitude >  90.0) ?  900000000 :
                     (int32_t)(latitude * 10000000.0);
    scaledLongitude = (longitude < -180.0) ? -1800000000 :
                      (longitude >  180.0) ?  1800000000 :
                      (int32_t)(longitude * 10000000.0);
    scaledElevation = (elevation < -409.5)  ? -4095  :
                      (elevation >  6143.9) ?  61440 :
                      (int16_t)(elevation * 10.0);

    /* Lock Aerolink calls */
    pthread_mutex_lock(&aerolinkCallMutex);

    /* Update the GPS position in Aerolink */
    AEROLINK_CALL_PRINTF("AEROLINK: %s: Calling securityServices_setCurrentLocation(%d, %d, %d, %u)\n",
                         __func__, scaledLatitude, scaledLongitude,
                         scaledElevation, countryCode);
    result = (secmode == 0) ? WS_SUCCESS : 
             securityServices_setCurrentLocation(scaledLatitude,
                                                 scaledLongitude,
                                                 scaledElevation, countryCode);
    AEROLINK_CALL_PRINTF("AEROLINK: %s: securityServices_setCurrentLocation() returned %s\n",
                         __func__, ws_strerror(result));

    if (result != WS_SUCCESS) {
        PRINT_MAX_N_TIMES(ERROR_MESSAGE_MAX_PRINT_TIMES,
                          "%s: securityServices_setCurrentLocation() failed (%s)\n",
                          __func__, ws_strerror(result));
    }

    /* Update the number of leap seconds since 2004 in Aerolink. */
    if ((leapSecondsSince2004 != lastLeapSecondsSince2004) || firstTime) {
        firstTime = FALSE;
        AEROLINK_CALL_PRINTF("AEROLINK: %s: Calling securityServices_setTimeAdjustment(%d)\n",
                             __func__, leapSecondsSince2004);
        result = (secmode == 0) ? WS_SUCCESS :
                 securityServices_setTimeAdjustment(leapSecondsSince2004);
        AEROLINK_CALL_PRINTF("AEROLINK: %s: securityServices_setTimeAdjustment() returned %s\n",
                             __func__, ws_strerror(result));

        if (result == WS_SUCCESS) {
            lastLeapSecondsSince2004 = leapSecondsSince2004;
        }
        else {
            PRINT_MAX_N_TIMES(ERROR_MESSAGE_MAX_PRINT_TIMES,
                              "%s: securityServices_setTimeAdjustment failed (%s)\n",
                              __func__, ws_strerror(result));
        }
    }

    /* Unlock Aerolink calls */
    pthread_mutex_unlock(&aerolinkCallMutex);
}

/*
 * alsmiUpdatePositionAndTime
 *
 * Updates the current position and leap seconds since 2004. Called from TPS
 * when new GPS data is received. If GPS is locked, the values are saved and
 * the position and time are updated when the GPS is later unlocked.
 *
 * Input parameters:
 *             latitude, longitude, elevation - Current GPS position
 *             leapSecondsSince2004 - Leap seconds since 1/1/2004, 00:00:00 UTC
 *             countryCode - field representing the country the device is
 *                           currently in
 * Output parameters:
 *             None
 * Return Value:  None
 */
void alsmiUpdatePositionAndTime(double latitude,  double  longitude,
                                double elevation, int16_t leapSecondsSince2004,
                                uint16_t countryCode)
{
    ALSMI_DEBUG_PRINTF("alsmiUpdatePositionAndTime(%.7lf, %.7lf, %.7lf, %d)\n",
                       latitude, longitude, elevation, leapSecondsSince2004);

    /* If alsmiInitialize() has not been called yet, just update the number of
     * leap seconds since 2004 so that it can be retrieved later. */
    if (!alsmiInitializeCalled) {
        lastLeapSecondsSince2004 = leapSecondsSince2004;
        return;
    }

    if (gps_locked) {
        /* If the GPS is locked save the update values for later */
        gps_saved_values           = TRUE;
        saved_latitude             = latitude;
        saved_longitude            = longitude;
        saved_elevation            = elevation;
        saved_leapSecondsSince2004 = leapSecondsSince2004;
        saved_countryCode          = countryCode;;
    }
    else {
        /* If the GPS is unlocked, do the update now */
        alsmiDoUpdatePositionAndTime(latitude, longitude, elevation,
                                     leapSecondsSince2004, countryCode);
    }
}

/*
 * alsmiUnlockGps
 *
 * Unlocks the GPS. If there was a saved GPS update, it is done.
 *
 * Input parameters:
 *             None
 * Output parameters:
 *             None
 * Return Value:  None
 */
static void alsmiUnlockGps(void)
{
    gps_locked = FALSE;

    if (gps_saved_values) {
        alsmiDoUpdatePositionAndTime(saved_latitude, saved_longitude,
                                     saved_elevation, saved_leapSecondsSince2004,
                                     saved_countryCode);
        gps_saved_values = FALSE;
    }
}

/*
 * alsmiValidateVODCallback
 *
 * The callback function that is called when verification is complete for
 * verify-on-demand
 *
 * Note: It is assumed that the GPS is locked when this routine is called. This
 * routine unlocks the GPS.
 *
 * Input parameters:
 *             returnCode: Result of the verification
 *             pVodMsgSeqNum: Pointer to the VOD message sequence number
 * Output parameters:
 *             None
 * Return Value:  None
 */
static void alsmiValidateVODCallback(AEROLINK_RESULT  returnCode,
                                     void            *pVodMsgSeqNum)
{
    uint32_t               vodMsgSeqNum = *((uint32_t *)pVodMsgSeqNum);
    verifyBufferElement_t *element;
    rsVodReplyType         vodReply;

    AEROLINK_CALL_PRINTF("AEROLINK: alsmiValidateVODCallback(%d, %u)\n",
                         returnCode, vodMsgSeqNum);

    if (returnCode == WS_SUCCESS) {
        shm_rsk_ptr->ALSMICnt.VerifySuccesses++;
    }
    else {
        shm_rsk_ptr->ALSMICnt.VerifyFailures++;
        shm_rsk_ptr->ALSMICnt.LastVerifyErrorCode = returnCode;
    }

    /* Unlock the GPS */
    alsmiUnlockGps();

    element = &verifyBuffer[vodMsgSeqNum % NUM_VERIFY_BUFFER_ELEMENTS];

    /* Send back a reply to unblock VOD */
    vodReply.risRet    = RS_SUCCESS;
    vodReply.vodResult = returnCode;

    ALSMI_DEBUG_PRINTF("Calling wsuSendData(%d, %u, %p, %lu))\n", element->recvCmdSock, element->sender_port, &vodReply, sizeof(vodReply));
    if (!wsuSendData(element->recvCmdSock, element->sender_port,
                     &vodReply, sizeof(vodReply))) {
        /* Send failed */
        PRINT_MAX_N_TIMES(ERROR_MESSAGE_MAX_PRINT_TIMES, "wsuSendData() failed\n");
    }

    /* Deallocate the SMP */
    if (secmode != 0) {
        pthread_mutex_lock(&aerolinkCallMutex);
        AEROLINK_CALL_PRINTF("AEROLINK: %s: Calling smp_delete(%p)\n", __func__, element->smp);
        smp_delete(element->smp);
        AEROLINK_CALL_PRINTF("AEROLINK: %s: smp_delete() returned\n", __func__);
        pthread_mutex_unlock(&aerolinkCallMutex);
    }

    pthread_mutex_lock(&verifyBufferMutex);
    /* Save the result in case the packet is verified again */
    element->verifyResult_vodResult = returnCode;
    /* Make the entry usable again */
    element->verificationSubmitted  = FALSE;
    pthread_mutex_unlock(&verifyBufferMutex);
}

/*
 * alsmiVerifyThisOne: API get a WSM of interest verified in the case of VOD.
 *
 * A call to this function may eventually result in a call to the verification
 * result callback (alsmiValidateVODCallback()), either immediately or at a
 * later time.
 *
 * Input parameters:
 *             vodMsgSeqNum: The VOD message sequence number of the message to be verified.
 *             recvCmdSock: Socket to send the results to.
 *             sender_port: Port to send the results to.
 * Output parameters:
 *             radioType: The radio type the packet being verified was
 *                        originally received on.
 * Return Value: RS_SUCCESS on Success, error code on failure. If RS_SUCCESS is
 *               returned, then either alsmiVerifyThisOne() or
 *               alsmiValidateVODCallback() will respond to the
 *               WSU_NS_SECURITY_VERIFY_WSM_REQ command, so
 *               radioServicesReceiveCommandsThread() should not. If something
 *               other than RS_SUCCESS is returned, then the
 *               WSU_NS_SECURITY_VERIFY_WSM_REQ command will not be responded
 *               to, so radioServicesReceiveCommandsThread() should respond to
 *               it.
 */
rsResultCodeType alsmiVerifyThisOne(uint32_t vodMsgSeqNum, int recvCmdSock,
                                    uint16_t sender_port, rsRadioType *radioType)

{
    verifyBufferElement_t *element;
    AEROLINK_RESULT        result;
    const uint8_t         *payload;
    uint32_t               payloadLen;
    PayloadType            spduType;
    PayloadType            payloadType;
    uint8_t const         *receivedExternalDataHash = NULL;
    ExternalDataHashAlg    externalDataHashAlg;
    rsVodReplyType         vodReply;
    int32_t                lat, lon;
    uint16_t               elev;
    int8_t                 res;
    SecuredMessageParserC  smp;

    ALSMI_DEBUG_PRINTF("alsmiVerifyThisOne(%u, %d, %u)\n",
                       vodMsgSeqNum, recvCmdSock, sender_port);

    /* Error if vodMsgSeqNum >= NUM_VERIFY_BUFFER_HANDLES */
    if (vodMsgSeqNum >= NUM_VERIFY_BUFFER_HANDLES) {
        PRINT_MAX_N_TIMES(ERROR_MESSAGE_MAX_PRINT_TIMES,
                          "%s: Error: vodMsgSeqNum > %u (%u)\n",
                          __func__, NUM_VERIFY_BUFFER_HANDLES, vodMsgSeqNum);
        return RS_ESECINVSEQN;
    }

    /* Calculate where the data should be in the buffer */
    element = &verifyBuffer[vodMsgSeqNum % NUM_VERIFY_BUFFER_ELEMENTS];

    /* Error if data not there */
    if (element->vodMsgSeqNum != vodMsgSeqNum) {
        ALSMI_DEBUG_PRINTF("%s: element->vodMsgSeqNum = %u; vodMsgSeqNum = %u; WS_ERR_NOT_FOUND\n",
                           __func__, element->vodMsgSeqNum, vodMsgSeqNum);
        /* Send back a reply with vodResult of WS_ERR_NOT_FOUND to unblock
         * VOD */
        vodReply.risRet    = RS_SUCCESS;
        vodReply.vodResult = WS_ERR_NOT_FOUND;

        ALSMI_DEBUG_PRINTF("Calling wsuSendData(%d, %u, %p, %lu))\n", recvCmdSock, sender_port, &vodReply, sizeof(vodReply));
        if (!wsuSendData(recvCmdSock, sender_port, &vodReply, sizeof(vodReply))) {
            /* Send failed */
            PRINT_MAX_N_TIMES(ERROR_MESSAGE_MAX_PRINT_TIMES, "wsuSendData() failed\n");
        }

        return RS_SUCCESS;
    }

    /* If the verification has been done previously, send back the previous
     * reply */
    if (element->verifyResult_vodResult != WS_ERR_VERIFY_NOT_ATTEMPTED) {
        ALSMI_DEBUG_PRINTF("%s: vodMsgSeqNum = %u; Already been verified. Sending back vodResult = %u\n",
                           __func__, vodMsgSeqNum, element->verifyResult_vodResult);
        /* Send back the previous reply */
        vodReply.risRet    = RS_SUCCESS;
        vodReply.vodResult = element->verifyResult_vodResult;

        ALSMI_DEBUG_PRINTF("Calling wsuSendData(%d, %u, %p, %lu))\n", recvCmdSock, sender_port, &vodReply, sizeof(vodReply));
        if (!wsuSendData(recvCmdSock, sender_port, &vodReply, sizeof(vodReply))) {
            /* Send failed */
            PRINT_MAX_N_TIMES(ERROR_MESSAGE_MAX_PRINT_TIMES, "wsuSendData() failed\n");
        }

        return RS_SUCCESS;
    }

    *radioType = element->radioType;

    /* Lock the GPS. alsmiValidateVODCallback() will unlock it. */
    gps_locked = TRUE;

    /* Allocate an smp */
    pthread_mutex_lock(&aerolinkCallMutex);
    AEROLINK_CALL_PRINTF("AEROLINK: %s: Calling smp_new(%p, %p)\n",
                         __func__, context, &smp);
    result = (secmode == 0) ? dummy_smp_new(context, &smp) :
             smp_new(context, &smp);
    AEROLINK_CALL_PRINTF("AEROLINK: %s: smp_new() returned %d; smp = %p\n",
                         __func__, result, smp);
    pthread_mutex_unlock(&aerolinkCallMutex);

    if (result != WS_SUCCESS) {
        PRINT_MAX_N_TIMES(ERROR_MESSAGE_MAX_PRINT_TIMES,
                          "%s: Unable to create a secured message parser (%s)\n",
                          __func__, ws_strerror(result));
        /* Unlock the GPS */
        alsmiUnlockGps();
        return RS_ESECCSMP;
    }

    /* Save some values in the verify buffer */
    pthread_mutex_lock(&verifyBufferMutex);
    element->smp                   = smp;
    element->verificationSubmitted = TRUE;
    element->recvCmdSock           = recvCmdSock;
    element->sender_port           = sender_port;
    pthread_mutex_unlock(&verifyBufferMutex);

    /* Lock Aerolink calls */
    pthread_mutex_lock(&aerolinkCallMutex);

    /* Process signed data */
    shm_rsk_ptr->ALSMICnt.VerifyRequests++;
    AEROLINK_CALL_PRINTF("AEROLINK: %s: Calling smp_extract(%p, %p, %d, %p, %p, %p, %p, %p, %p)\n",
                         __func__, smp, element->signedData,
                         element->signedDataLength, &spduType, &payload,
                         &payloadLen, &payloadType, &receivedExternalDataHash,
                         &externalDataHashAlg);
    result = (secmode == 0) ?
             dummy_smp_extract(smp, element->signedData,
                               element->signedDataLength, &spduType,
                               &payload, &payloadLen, &payloadType,
                               &receivedExternalDataHash, &externalDataHashAlg) :
             smp_extract(smp, element->signedData,
                         element->signedDataLength, &spduType,
                         &payload, &payloadLen, &payloadType,
                         &receivedExternalDataHash, &externalDataHashAlg);
    AEROLINK_CALL_PRINTF("AEROLINK: %s: smp_extract() returned %s\n",
                         __func__, ws_strerror(result));

    /* If extract failed, call callback with result */
    if (result != WS_SUCCESS) {
        goto callValidateVODCallback;
    }

    /* Determine whether or not to get the generation location from the body of
     * the BSM based on the PSID */
    res = alsmiGetGenLocFromBsm(element->PSID);

    if (res == -1) {
        result = WS_ERR_INVALID_PSID;
        goto callValidateVODCallback;
    }

    if (res == 1) {
        /* Get the generation location from the body of the BSM */
        result = alsmiASN1ExtractJ2735_LatLonElev(&lat, &lon, &elev, payload, payloadLen);

        if (result != WS_SUCCESS) {
            goto callValidateVODCallback;
        }

        /* Set the generation location in Aerolink */
        AEROLINK_CALL_PRINTF("AEROLINK: %s: Calling smp_setGenerationLocation(%p, %d, %d, %d)\n",
                             __func__, smp, lat, lon, elev);
        result = (secmode == 0) ? WS_SUCCESS :
                 smp_setGenerationLocation(smp, lat, lon, elev);
        AEROLINK_CALL_PRINTF("AEROLINK: %s: smp_setGenerationLocation() returned %s\n", __func__, ws_strerror(result));

        if (result != WS_SUCCESS) {
            goto callValidateVODCallback;
        }
    }

    /* Check relevance */
    AEROLINK_CALL_PRINTF("AEROLINK: %s: Calling smp_checkRelevance(%p)\n", __func__, smp);
    result = (secmode == 0) ? WS_SUCCESS : smp_checkRelevance(smp);
    AEROLINK_CALL_PRINTF("AEROLINK: %s: smp_checkRelevance() returned %s\n", __func__, ws_strerror(result));

    /* If relevance check failed, call callback with result */
    if (result != WS_SUCCESS) {
        goto callValidateVODCallback;
    }

    /* Check consistency */
    AEROLINK_CALL_PRINTF("AEROLINK: %s: Calling smp_checkConsistency(%p)\n", __func__, smp);
    result = (secmode == 0) ? WS_SUCCESS : smp_checkConsistency(smp);
    AEROLINK_CALL_PRINTF("AEROLINK: %s: smp_checkConsistency() returned %s\n", __func__, ws_strerror(result));

    /* If consistency check failed, call callback with result */
    if (result != WS_SUCCESS) {
        goto callValidateVODCallback;
    }

    /* Check signatures */
    /* Put the VOD message sequence number in a location where it will remain valid even after
     * this function returns */
    AEROLINK_CALL_PRINTF("AEROLINK: %s: Calling smp_verifySignaturesAsync(%p, %p:%u, %p); "
                         "alsmiValidateVODCallback = %p\n",
                         __func__, smp, &element->vodMsgSeqNum, element->vodMsgSeqNum,
                         alsmiValidateVODCallback, alsmiValidateVODCallback);
    result = (secmode == 0) ?
             dummy_smp_verifySignaturesAsync(smp, &element->vodMsgSeqNum,
                                             alsmiValidateVODCallback) :
             smp_verifySignaturesAsync(smp, &element->vodMsgSeqNum,
                                       alsmiValidateVODCallback);
    AEROLINK_CALL_PRINTF("AEROLINK: %s: smp_verifySignaturesAsync() returned %s\n",
                         __func__, ws_strerror(result));

callValidateVODCallback:
    /* Unlock Aerolink calls */
    pthread_mutex_unlock(&aerolinkCallMutex);

    if (result != WS_SUCCESS) {
        /* If error, call the callback function with the result */
        alsmiValidateVODCallback(result, &vodMsgSeqNum);
    }

    /* If we reach this point, and result is WS_SUCCESS, then
     * smp_verifySignaturesAsync is running in the background. It will call
     * alsmiValidateVODCallback when the verify is done.
     * alsmiValidateVODCallback() will reply to the
     * WSU_NS_SECURITY_VERIFY_WSM_REQ command. */
    return RS_SUCCESS;
}

/*
 * alsmiSendCertChangeReq: API to send certificate change request.
 *
 * Input parameters:
 *             None.
 * Output parameters:
 *             None.
 * Return Value:  0 on Success -1 on failure.
 */
int alsmiSendCertChangeReq(void)
{
    AEROLINK_RESULT result;

    ALSMI_DEBUG_PRINTF("alsmiSendCertChangeReq()\n");
    /* Initiate ID change */
    shm_rsk_ptr->ALSMICnt.CertChangeRequests++;

    /* Start the certificate change timer */
    START_CERT_CHANGE_TIMER;

    /* If the certChangeTimeoutTestCount variable is set to non-zero, decrement
     * it and suppress the call to securityServices_idChangeInit(). This
     * variable will only be set to non-zero if someone sent us the cert change
     * timeout test command. Normally, certChangeTimeoutTestCount is always
     * zero. */
    if (certChangeTimeoutTestCount != 0) {
        ALSMI_DEBUG_PRINTF("%s: Call to securityServices_idChangeInit() suppressed\n", __func__);
        certChangeTimeoutTestCount--;
        result = WS_ERR_ID_CHANGE_BLOCKED;
    }
    else {
        AEROLINK_CALL_PRINTF("AEROLINK: %s: Calling securityServices_idChangeInit()\n", __func__);
        result = (secmode == 0) ? dummy_securityServices_idChangeInit() :
                                  securityServices_idChangeInit();
        AEROLINK_CALL_PRINTF("AEROLINK: %s: securityServices_idChangeInit() returned %s\n", __func__, ws_strerror(result));
    }

    /*
     * Note that the usual result we expect is not WS_SUCCESS, but actually
     * WS_ERR_ID_CHANGE_BLOCKED. We normally have alsmi_idChangeLock() in
     * effect during certificate changes. From the Aerolink User API Manual:
     * "If ID-change-protocol initiations are blocked, WS_ERR_ID_CHANGE_BLOCKED
     * is returned but the request will be marked pending and will execute then
     * the blocking condition is removed." (Probably they meant "when" and not
     * "then"). Note that we do an alsmi_idChangeUnlock() almost immediately
     * after initiating the certificate change.
     */
    if ((result != WS_SUCCESS) && (result != WS_ERR_ID_CHANGE_BLOCKED)) {
        CANCEL_CERT_CHANGE_TIMER;
        shm_rsk_ptr->ALSMICnt.CertChangeRequestsRejected++;
        shm_rsk_ptr->ALSMICnt.CertChangeRequestLastErrorCode = result;
        PRINT_MAX_N_TIMES(ERROR_MESSAGE_MAX_PRINT_TIMES,
                          "%s: Failed to initiate ID change (%s)\n",
                          __func__, ws_strerror(result));
        return -1;
    }

    shm_rsk_ptr->ALSMICnt.CertChangeRequestsAccepted++;
#ifdef CERTIFICATE_CHANGE_TIMING_TEST
    gettimeofday(&cctt_tv1, NULL);
#endif // CERTIFICATE_CHANGE_TIMING_TEST

    /* Enable ID-change protocol (it will be disabled when the certificate
     * change is completed). */
    result = alsmi_idChangeUnlock();

    if (result != WS_SUCCESS) {
        PRINT_MAX_N_TIMES(ERROR_MESSAGE_MAX_PRINT_TIMES,
                          "%s: Unable to un-block ID-change protocol (%s)\n",
                          __func__, ws_strerror(result));
    }

    return 0;
}

/* Possible additional information for the application "ver" command */
void alsmiPrintVersionInfo(void)
{
    fprintf(stdout,"Only asynchronous signing is supported in %s\n", __FILE__);
    fprintf(stdout,"Only asynchronous verification is supported in %s\n", __FILE__);
    fflush(stdout);
}

/*
 * alsmiGetVersion
 *
 * Returns a pointer to a string containing the Aerolink software version.
 *
 * Input parameters:
 *             None
 * Output parameters:
 *             None
 * Return Value:  pointer to a string containing the Aerolink software version
 */
const char *alsmiGetVersion(void)
{
    const char  *result;
    static char  dummy_version[] = "0.0.0.0";

    shm_rsk_ptr->ALSMICnt.GetAerolinkVersion++;
    AEROLINK_CALL_PRINTF("AEROLINK: %s: Calling securityServices_getVersion()\n",
                         __func__);
    result = (secmode == 0) ? dummy_version : securityServices_getVersion();
    AEROLINK_CALL_PRINTF("AEROLINK: %s: securityServices_getVersion() returned \"%s\"\n",
                         __func__, result);
    return result;
}

/*
 * alsmiGetLeapSecondsSince2004
 *
 * Gets the number of leap seconds since 2004. alsmiUpdatePositionAndTime()
 * must have been called some time previous to this function being called.
 *
 * Input parameters:
 *             None.
 * Output parameters:
 *             None
 * Return Value:  Number of leap seconds since 2004.
 */
int16_t alsmiGetLeapSecondsSince2004(void)
{
    ALSMI_DEBUG_PRINTF("%s:(): lastLeapSecondsSince2004 = %d\n",
                       __func__, lastLeapSecondsSince2004);
    shm_rsk_ptr->ALSMICnt.GetLeapSecondsSince2004++;
    return lastLeapSecondsSince2004;
}

/*
 * alsmiGetRegIdx
 *
 * Gets the index into the registration table from the PSID.
 *
 * Input parameters:
 *             psid: The PSID to search for.
 * Output parameters:
 *             None.
 * Return Value:  The index into the registration table. -1 if not found.
 */
static uint8_t alsmiGetRegIdx(uint32_t psid)
{
    int i, j;

    pthread_mutex_lock(&regTableMutex);

    for (i = 0; i < ALSMI_NUM_REGISTRATIONS; i++) {
        if ((regTable[i].inUse) && (regTable[i].numSignResultsPSIDs > 0)) {
            for (j = 0; j < regTable[i].numSignResultsPSIDs; j++) {
                if (regTable[i].signResultsPSIDs[j] == psid) {
                    pthread_mutex_unlock(&regTableMutex);
                    return i;
                }
            }
        }
    }

    pthread_mutex_unlock(&regTableMutex);
    return (uint8_t)-1;
}

/*
 * alsmiCallSignResultsCallback
 *
 * Calls the (WSA) sign results callback function with the signStatus and
 * signResults fields.
 *
 * Input parameters:
 *             WSA: Set to TRUE if for sign WSA, FALSE otherwise
 *             radioType: The radio type for the packet that was being signed.
 *                        Not used if WSA is TRUE.
 *             psid: The PSID of the packet that was being signed
 *             returnCode: Result of the signing operation
 *             signedDataLength: The length of the signed data, if successful
 * Output parameters:
 *             None
 * Return Value:  None
 */
static void alsmiCallSignResultsCallback(bool_t WSA,
                                         rsRadioType radioType, uint32_t psid,
                                         AEROLINK_RESULT returnCode,
                                         uint16_t signedDataLength)
{
    uint8_t            regIdx;
    enum smi_results_e options;
    rsReceiveDataType  recv;

    AEROLINK_CALL_PRINTF("AEROLINK: alsmiCallSignResultsCallback(%d, %d, 0x%x, %d, %u)\n",
                         WSA, radioType, psid, returnCode, signedDataLength);


    if (WSA) {
        options = sign_wsa_results_options;
    }
    else {
        regIdx = alsmiGetRegIdx(psid);

        if (regIdx == (uint8_t)-1) {
            ALSMI_DEBUG_PRINTF("%s: alsmiGetRegIdx(0x%x) returned -1; no results sent back\n",
                               __func__, psid);
            return;
        }

        options = regTable[regIdx].sign_results_options;
    }

    if ((options == smi_results_all) ||
        (returnCode != WS_SUCCESS)) {
        recv.indType                  = WSA ? IND_SIGN_WSA_RESULTS :
                                              IND_SIGN_WSM_RESULTS;
        recv.u.signResults.psid       = psid;
        recv.u.signResults.returnCode = (returnCode & 0xff);

        if (returnCode == WS_SUCCESS) {
            recv.u.signResults.securitySignStatus = securedSignSuccess;
            recv.u.signResults.signedDataLength   = signedDataLength;
        }
        else {
            recv.u.signResults.securitySignStatus = securedSignFail;
            recv.u.signResults.signedDataLength   = 0;
        }

        /* Send the indication to RIS */
        if (WSA) {
            sendIndicationToRisByPid(sign_wsa_results_pid, &recv);
        }
        else {
            sendIndicationToRisByRadioTypeAndPsid(radioType, psid, &recv,
                                                  FALSE);
        }
    }
}

/*
 * alsmiSignCallback
 *
 * The callback function that is called when signing is complete.
 *
 * Note: It is assumed that the GPS is locked when this routine is called. This
 * routine unlocks the GPS.
 *
 * Input parameters:
 *             returnCode: Result of the signing operation
 *             userCallbackData: The sign handle, cast as a pointer
 *             signedData: Pointer to the signed data
 *             signedDataLength: The signed data length
 * Output parameters:
 *             None
 * Return Value:  None
 */
static void alsmiSignCallback(AEROLINK_RESULT  returnCode,
                              void            *userCallbackData,
                              uint8_t         *signedData,
                              uint32_t         signedDataLength)
{
    uint32_t             signHandle = *((uint32_t *)userCallbackData);
    signBufferElement_t *element;

    AEROLINK_CALL_PRINTF("AEROLINK: alsmiSignCallback(%d, %u, %p, %u)\n",
                         returnCode, signHandle, signedData, signedDataLength);

    if (returnCode == WS_SUCCESS) {
        shm_rsk_ptr->ALSMICnt.SignSuccesses++;
    }
    else {
        shm_rsk_ptr->ALSMICnt.SignFailures++;
        shm_rsk_ptr->ALSMICnt.SignCBFailures++;
        shm_rsk_ptr->ALSMICnt.LastSignErrorCode = returnCode;

        if (returnCode < 256) {
            if (signErrCount[returnCode] < 10) {
                signErrCount[returnCode]++;
                fprintf(stdout,"ALSMI sign failed: error code = %s\n",
                        ws_strerror(returnCode));
                fflush(stdout);
            }
        }
    }

    /* unlock the GPS */
    alsmiUnlockGps();

    element = &signBuffer[signHandle % NUM_SIGN_BUFFER_ELEMENTS];

    /* Be sure buffer hasn't wrapped around */
    if (signHandle == element->signHandle) {
        /* Report back sign results */
        if ((element->pktType == ALSMI_PKT_TYPE_WSA)  ||
            (element->pktType == ALSMI_PKT_TYPE_WSMP) ||
            (element->pktType == ALSMI_PKT_TYPE_EU)) {
            bool_t      WSA;
            rsRadioType radioType;
            uint32_t    psid;

            if (element->pktType == ALSMI_PKT_TYPE_WSA) {
                WSA  = TRUE;
                /* radioType ends up not being used for WSA */
                radioType = RT_DSRC;
                psid      = WSM_WSA_PSID;
            }
            else {
                WSA = FALSE;

                if (element->pktType == ALSMI_PKT_TYPE_WSMP) {
                    radioType = element->u.sendWSMStruct.radioType;
                    psid      = element->u.sendWSMStruct.psid;
                }
                else {
                    radioType = element->u.EUTxMetaData.radioType;
                    psid      = element->u.EUTxMetaData.ITS_AID;
                }
            }

            alsmiCallSignResultsCallback(WSA, radioType, psid, returnCode,
                                         signedDataLength);
        }

        if (returnCode == WS_SUCCESS) {
            switch (element->pktType) {
            case ALSMI_PKT_TYPE_WSMP:
                /* Transmit the WSMP packet */
                ALSMI_DEBUG_PRINTF("%s: Calling sendWSMActual(%p, %p, %u)\n", __func__,
                                   &element->u.sendWSMStruct,
                                   signedData, signedDataLength);
                sendWSMActual(&element->u.sendWSMStruct,
                              signedData, signedDataLength);
                break;

            case ALSMI_PKT_TYPE_EU:
                /* Transmit the EU packet */
                ALSMI_DEBUG_PRINTF("%s: Calling sendEUPacketX(%p, %u, %p)\n", __func__,
                                   element->packetData,
                                   signedDataLength + element->additionalDataLen,
                                   &element->u.EUTxMetaData);
#ifdef SUPPORT_EU
                sendEUPacketX(element->packetData,
                              signedDataLength + element->additionalDataLen,
                              &element->u.EUTxMetaData);
#endif
                break;

            case ALSMI_PKT_TYPE_WSA:
                /* Call the WSM sign confirm routine */
                ALSMI_DEBUG_PRINTF("%s: Calling wsaSignSecurityCfm(%u, %p, %u, %p)\n", __func__,
                                   returnCode, signedData, signedDataLength,
                                   &element->u.wsaTxSecurity);
/* TODO: assuming this is needed for RSU. */
#if defined(SUPPORT_WSA_SIGN) || defined(HEAVY_DUTY)
                wsaSignSecurityCfm(returnCode, signedData, signedDataLength,
                                   &element->u.wsaTxSecurity);
#endif
                break;
            }
        }

        /* Deallocate the SMG */
        if (!element->usingGeneratedSmg) {
            ALSMI_DEBUG_PRINTF("%s: Deallocating pre-defined smg\n", __func__);
            sign_smgInUse = FALSE;
        }
        else {
            if (secmode != 0) {
                pthread_mutex_lock(&aerolinkCallMutex);
                AEROLINK_CALL_PRINTF("AEROLINK: %s: Calling smg_delete(%p)\n", __func__, element->smg);
                smg_delete(element->smg);
                AEROLINK_CALL_PRINTF("AEROLINK: %s: smg_delete() returned\n", __func__);
                pthread_mutex_unlock(&aerolinkCallMutex);
            }
        }

        /* Make the element usable again */
        pthread_mutex_lock(&signBufferMutex);
        element->signHandle = -1;
        element->signingSubmitted = FALSE;
        pthread_mutex_unlock(&signBufferMutex);
    } // if (signHandle == element->signHandle)
}

/*
 * alsmiSign
 *
 * Signs a packet. When signing is complete, a callback routine (generally in
 * radioServices) is called to transmit the packet. Also, the sign result
 * callback is called if desired.
 *
 * Input parameters:
 *             unsignedData:       Pointer to the data to be signed
 *             unsignedDataLength: Length of the data to be signed
 *             additionalData:     Pointer to additional data outside of the
 *                                 signed data area.
 *                                 This is the basic header for an EU packet.
 *             additionalDataLen:  Additional data length
 *             pktType:            Set to ALSMI_PKT_TYPE_EU if EU data packet,
 *                                 ALSMI_PKT_TYPE_WSMP if BSM WSMP packet,
 *                                 ALSMI_PKT_TYPE_WSA if WSA WSMP packet
 *             additionalSecurityParameters:
 *                                 Additional security parameters. This is a
 *                                 pointer to a sendWSMStruct_s for BSM WSMP
 *                                 packet. This is a pointer to a
 *                                 risTxMetaEUPktType for an EU packet. This is
 *                                 a pointer to a wsaTxSecurityType for a WSA
 *                                 WSMP packet.
 *
 * Output parameters:
 *             None
 * Return Value:  WS_SUCCESS on Success; Error code on failure.
 */
AEROLINK_RESULT alsmiSign(uint8_t  *unsignedData,
                          uint16_t  unsignedDataLength,
                          uint8_t  *additionalData,
                          uint8_t   additionalDataLen,
                          uint8_t   pktType,
                          void     *additionalSecurityParameters)
{
    signBufferElement_t *element;
    AEROLINK_RESULT      result;
    SigningPermissions   signingPermissions;
    sendWSMStruct_S     *sendWSMStruct = NULL;
    risTxMetaEUPktType  *EUTxMetaData;
    bool_t               WSA = (pktType == ALSMI_PKT_TYPE_WSA);
    SignerTypeOverride   signerTypeOverride;
    uint8_t              externalDataHash[32];
    rsRadioType          radioType;
    uint8_t             *outData;
    uint16_t             outDataLength = MAX_WSM_DATA - additionalDataLen;

    ALSMI_DEBUG_PRINTF("%s(%p, %u, %p, %u, %u, %p)\n",
                       __func__, unsignedData, unsignedDataLength,
                       additionalData, additionalDataLen, pktType,
                       additionalSecurityParameters);

    shm_rsk_ptr->ALSMICnt.SignRequests++;

    /* Initialize either the sendWSMStruct or the EUTxMetaData structure,
     * and get the psid value */
    memset(&signingPermissions, 0, sizeof(signingPermissions));

    switch (pktType) {
    case ALSMI_PKT_TYPE_WSMP:
        sendWSMStruct = (sendWSMStruct_S *)additionalSecurityParameters;
        radioType               = sendWSMStruct->radioType;
        signingPermissions.psid = sendWSMStruct->psid;
        signerTypeOverride =
            ((sendWSMStruct->securityFlag & 0x03) == 0) ? STO_DIGEST :
            ((sendWSMStruct->securityFlag & 0x03) == 1) ? STO_CERTIFICATE :
                                                          STO_AUTO;
        break;

    case ALSMI_PKT_TYPE_EU:
        EUTxMetaData = (risTxMetaEUPktType *)additionalSecurityParameters;
        radioType               = EUTxMetaData->radioType;
        signingPermissions.psid = EUTxMetaData->ITS_AID;
        signerTypeOverride = STO_AUTO;
        break;

    case ALSMI_PKT_TYPE_WSA:
        radioType = RT_DSRC; /* This ends up not being used */
        signingPermissions.psid = WSM_WSA_PSID;
        signerTypeOverride = STO_AUTO;
        break;

    default:
        PRINT_MAX_N_TIMES(ERROR_MESSAGE_MAX_PRINT_TIMES,
                          "%s: Error: Unknown pktType %u\n", __func__, pktType);
        alsmiCallSignResultsCallback(WSA, RT_DSRC, 0, WS_ERR_UNKNOWN_PACKET_TYPE, 0);
        shm_rsk_ptr->ALSMICnt.SignFailures++;
        shm_rsk_ptr->ALSMICnt.SFUnknownPacketType++;
        return WS_ERR_UNKNOWN_PACKET_TYPE;
    }

    /* Convert PSID from P-encoded to decimal */
    if (pCoded2Decimal(&signingPermissions.psid) == -1) {
        PRINT_MAX_N_TIMES(ERROR_MESSAGE_MAX_PRINT_TIMES,
                          "%s: Error: Invalid PSID (0x%x)\n",
                          __func__, signingPermissions.psid);
        alsmiCallSignResultsCallback(WSA, radioType, signingPermissions.psid,
                                     WS_ERR_INVALID_PSID, 0);
        shm_rsk_ptr->ALSMICnt.SignFailures++;
        shm_rsk_ptr->ALSMICnt.SFInvalidPSID++;
        return WS_ERR_INVALID_PSID;
    }

    /* Be sure data is not too long */
    if (unsignedDataLength > outDataLength) {
        PRINT_MAX_N_TIMES(ERROR_MESSAGE_MAX_PRINT_TIMES,
                          "%s: Error: unsignedDataLength(%u) > MAX_WSM_DATA(%u)"
                          " - additionalDataLen(%u)\n",
                          __func__, unsignedDataLength, MAX_WSM_DATA,
                          additionalDataLen);
        alsmiCallSignResultsCallback(WSA, radioType, signingPermissions.psid,
                                     WS_ERR_PAYLOAD_TOO_LARGE, 0);
        shm_rsk_ptr->ALSMICnt.SignFailures++;
        shm_rsk_ptr->ALSMICnt.SFPayloadTooLarge++;
        return WS_ERR_PAYLOAD_TOO_LARGE;
    }

    /* Don't sign if certificate change in progress */
    if (certificateChangeInProgress) {
        ALSMI_DEBUG_PRINTF("%s: Error: Certificate change in progress\n", __func__);
        alsmiCallSignResultsCallback(WSA, radioType, signingPermissions.psid,
                                     WS_ERR_CERTIFICATE_CHANGE_IN_PROGRESS, 0);
        shm_rsk_ptr->ALSMICnt.SignFailures++;
        shm_rsk_ptr->ALSMICnt.SFCertificateChangeInProgress++;
        return WS_ERR_CERTIFICATE_CHANGE_IN_PROGRESS;
    }

    /* Allocate and fill out the sign buffer entry */
    pthread_mutex_lock(&signBufferMutex);
    element = &signBuffer[signBufferNextHandle % NUM_SIGN_BUFFER_ELEMENTS];

    /* If the next sign buffer element contains a packet submitted for
     * signing, send a sign buffer overflow error to the sign results
     * callback */
    if ((element->signHandle != -1) && element->signingSubmitted) {
        pthread_mutex_unlock(&signBufferMutex);
        ALSMI_DEBUG_PRINTF("%s: Error: Sign buffer overflow\n", __func__);
        alsmiCallSignResultsCallback(WSA, radioType, signingPermissions.psid,
                                     WS_ERR_SIGN_BUFFER_OVERFLOW, 0);
        shm_rsk_ptr->ALSMICnt.SignFailures++;
        shm_rsk_ptr->ALSMICnt.SFSignBufferOverflow++;
        return WS_ERR_SIGN_BUFFER_OVERFLOW;
    }

    /* Fill out the sign buffer element */
    switch (pktType) {
    case ALSMI_PKT_TYPE_WSMP:
        element->u.sendWSMStruct          = *sendWSMStruct;
        signingPermissions.isBitmappedSsp = sendWSMStruct->isBitmappedSsp ? 1 : 0;
        signingPermissions.sspMask        = sendWSMStruct->sspMask;
        signingPermissions.ssp            = sendWSMStruct->ssp;
        signingPermissions.sspLen         = sendWSMStruct->sspLen;
        break;

    case ALSMI_PKT_TYPE_EU:
        element->u.EUTxMetaData           = *EUTxMetaData;
        signingPermissions.isBitmappedSsp = (EUTxMetaData->isBitmappedSsp == 0) ? 0 : 1;
        signingPermissions.sspMask        = EUTxMetaData->sspMask;
        signingPermissions.ssp            = EUTxMetaData->ssp;
        signingPermissions.sspLen         = EUTxMetaData->sspLen;
        break;

    case ALSMI_PKT_TYPE_WSA:
        /* VJR WILLBEREVISITED Delete this line when we add support for SSP to
         * WSA security */
        element->u.wsaTxSecurity = *((wsaTxSecurityType *)additionalSecurityParameters);
        /* VJR WILLBEREVISITED We may want to support SSP in the future for
         * WSA. in this case, we would most likely add SSP and SSP length fields
         * to the wsaTxSecurityType structure. The actual values would probably
         * be gotten from something like a configuration file.
        wsaTxSecurity                     = (wsaTxSecurityType *)additionalSecurityParameters;
        element->u.wsaTxSecurity          = *wsaTxSecurity;
        signingPermissions.isBitmappedSsp = wsaTxSecurity->isBitmappedSsp;
        signingPermissions.sspMask        = wsaTxSecurity->sspMask;
        signingPermissions.ssp            = wsaTxSecurity->ssp;
        signingPermissions.sspLen         = wsaTxSecurity->sspLen;
         */
        signingPermissions.isBitmappedSsp = 0;
        signingPermissions.sspLen         = 0;
        /* VJR WILLBEREVISITED Be sure to design wsaSignSecurityCfm() consistent
         * with the next statement! */
        break;
    }

    /* Copy over the unsigned data */
    memcpy(element->unsignedData, unsignedData, unsignedDataLength);
    element->unsignedDataLength = unsignedDataLength;

    /* Copy over the additional data, leaving free space at the beginning */
    if (additionalDataLen != 0) {
        memcpy(&element->packetData[HDRS_LEN], additionalData,
               additionalDataLen);
    }

    element->additionalDataLen = additionalDataLen;
    element->pktType           = pktType;
    element->signHandle        = signBufferNextHandle;
    element->signingSubmitted  = TRUE;
    signBufferNextHandle = (signBufferNextHandle + 1) % NUM_SIGN_BUFFER_HANDLES;

    if (signBufferNextHandle == -1) {
        signBufferNextHandle = 0;
    }

    pthread_mutex_unlock(&signBufferMutex);

    /* Lock the GPS lock. alsmiSignCallback() will unlock it. */
    gps_locked = TRUE;

    /* Submit the packet for signing */
    outData = &element->packetData[HDRS_LEN + additionalDataLen];
    /* Lock Aerolink calls */
    pthread_mutex_lock(&aerolinkCallMutex);

    /* Use the sigm_smg if it is not already in use. Otherwise, generate an smg
     * to use. */
    if (!sign_smgInUse) {
        ALSMI_DEBUG_PRINTF("%s: Allocating pre-defined smg\n", __func__);
        element->smg = sign_smg;
        element->usingGeneratedSmg = FALSE;
        sign_smgInUse = TRUE;
    }
    else {
        AEROLINK_CALL_PRINTF("AEROLINK: %s: Calling smg_new(%p, %p)\n",
                             __func__, context, &element->smg);
        result = (secmode == 0) ? dummy_smg_new(context, &element->smg) :
                                  smg_new(context, &element->smg);
        AEROLINK_CALL_PRINTF("AEROLINK: %s: smg_new() returned %d; element->smg = %p\n",
                             __func__, result, element->smg);

        if (result != WS_SUCCESS) {
            PRINT_MAX_N_TIMES(ERROR_MESSAGE_MAX_PRINT_TIMES,
                              "%s: Unable to create a secured message generator (%s)\n",
                              __func__, ws_strerror(result));
            pthread_mutex_unlock(&aerolinkCallMutex);
            alsmiCallSignResultsCallback(WSA, radioType, signingPermissions.psid,
                                         result, 0);
            shm_rsk_ptr->ALSMICnt.SignFailures++;
            shm_rsk_ptr->ALSMICnt.SFsmg_newFailed++;
            return result;
        }

        element->usingGeneratedSmg = TRUE;
    }

    AEROLINK_CALL_PRINTF("AEROLINK: %s: Calling smg_signAsync(%p, signingPermissions, %u, 0, %p, %u, 0, %p, EDHA_NONE, NULL, %p, %u, %p, %p)\n",
                         __func__, element->smg, signerTypeOverride,
                         element->unsignedData, element->unsignedDataLength,
                         externalDataHash,
                         outData, outDataLength,
                         &element->signHandle, alsmiSignCallback);
    AEROLINK_CALL_PRINTF("AEROLINK: %s: alsmiSignCallback = %p\n",
                         __func__, alsmiSignCallback);
    AEROLINK_CALL_PRINTF("AEROLINK: %s: signingPermissions.isBitmappedSsp = %d; sspLen = %u\n",
                         __func__, signingPermissions.isBitmappedSsp,
                         signingPermissions.sspLen);

    result = (secmode == 0) ?
             dummy_smg_signAsync(element->smg,
                                 signingPermissions,
                                 signerTypeOverride,
                                 0, // don't use lifetime override
                                 element->unsignedData, element->unsignedDataLength,
                                 0, // SPDU as payload not supported yet
                                 externalDataHash, // dummy
                                 EDHA_NONE, // no external data hash algorithm
                                 NULL, // no encryption key
                                 outData, outDataLength,
                                 &element->signHandle,
                                 alsmiSignCallback) :
             smg_signAsync(element->smg,
                           signingPermissions,
                           signerTypeOverride,
                           0, // don't use lifetime override
                           element->unsignedData, element->unsignedDataLength,
                           0, // SPDU as payload not supported yet
                           externalDataHash, // dummy
                           EDHA_NONE, // no external data hash algorithm
                           NULL, // no encryption key
                           outData, outDataLength,
                           &element->signHandle,
                           alsmiSignCallback);
    AEROLINK_CALL_PRINTF("AEROLINK: %s: smg_signAsync() returned %s\n",
                         __func__, ws_strerror(result));
    /* Unlock Aerolink calls */
    pthread_mutex_unlock(&aerolinkCallMutex);

    if (result != WS_SUCCESS) {
        /* If error, call the sign callback function with the result */
        alsmiSignCallback(result, (void *)&element->signHandle, outData, 0);
    }

    /* If we reach this point, smg_signAsync is running in the background.
     * It will call alsmiSignCallback when the sign is done. */
    return WS_SUCCESS;
}

/*
 * alsmiCallSignResCbFor1609p2
 *
 * Calls the sign results callback function for adding the 1609.2 header with
 * the signStatus and signResults fields. The packet isn't really signed, but
 * we want to call the sign results callback to minimize the amount of special
 * case code that has to be written.
 *
 * Input parameters:
 *             WSA: TRUE if this is a WSA, FALSE otherwise
 *             radioType: The radio type for the packet that was being signed.
 *                        Not used if WSA is TRUE.
 *             psid: The PSID of the packet that was being signed
 *             returnCode: Result of the signing operation
 *             outDataLength: The length of the data after the 1609.2 header
 *                            was added, if successful
 * Output parameters:
 *             None
 * Return Value:  None
 */
static void alsmiCallSignResCbFor1609p2(bool_t WSA,
                                        rsRadioType radioType, uint32_t psid,
                                        AEROLINK_RESULT returnCode,
                                        uint16_t outDataLength)
{
    uint8_t            regIdx;
    enum smi_results_e options;
    rsReceiveDataType  recv;

    AEROLINK_CALL_PRINTF("AEROLINK: %s(%d, %d, 0x%x, %d, %u)\n",
                         __func__, WSA, radioType, psid, returnCode, outDataLength);

    if (WSA) {
        options = sign_wsa_results_options;
    }
    else {
        regIdx = alsmiGetRegIdx(psid);

        if (regIdx == (uint8_t)-1) {
            return;
        }

        options = regTable[regIdx].sign_results_options;
    }

    if ((options == smi_results_all) || (returnCode != WS_SUCCESS)) {
        recv.indType                  = IND_ADD_UNSEC_1609P2_RESULTS;
        recv.u.signResults.psid       = psid;
        recv.u.signResults.returnCode = (returnCode & 0xff);

        if (returnCode == WS_SUCCESS) {
            recv.u.signResults.securitySignStatus = securedSignSuccess;
            recv.u.signResults.signedDataLength   = outDataLength;
        }
        else {
            recv.u.signResults.securitySignStatus = securedSignFail;
            recv.u.signResults.signedDataLength   = 0;
        }

        /* Send the indication to RIS */
        if (WSA) {
            sendIndicationToRisByPid(sign_wsa_results_pid, &recv);
        }
        else {
            sendIndicationToRisByRadioTypeAndPsid(radioType, psid, &recv,
                                                  FALSE);
        }
    }
}

/*
 * alsmiAddUnsecured1609p2Hdr
 *
 * Adds a 1609.2 header to a packet. When complete, a callback routine
 * (generally in Radio_ns) is called to transmit the packet. Also, the sign
 * result callback is called if desired. The packet isn't really signed, but
 * we want to call the sign results callback to minimize the amount of special
 * case code that has to be written.
 *
 * Note that if the 1609 version is 3, then the sendWSMStruct.wsmpHdrLen
 * assumes that the data length will require 2 bytes. If it turns out that,
 * after adding the 1609.2 header the data length only requires 1 byte, then
 * the sendWSMStruct->wsmpHdrLen will need to be decremented by 1, and the
 * data will be shifted to the left one byte before sendWSMActual() is called.
 *
 * Input parameters:
 *             WSA:                    TRUE if this is a WSA, FALSE otherwise
 *             inData:                 Pointer to the data to have the header
 *                                     added
 *             inDataLength:           Length of that data
 *             additionalSecurityData: Pointer to additional security data.
 *                                     This is a pointer to a wsaTxSecurityType
 *                                     if WSA is TRUE. This is a pointer to a
 *                                     sendWSMStruct_S if WSA is FALSE.
 * Output parameters:
 *             None
 * Return Value:  0 on Success -1 on failure.
 */
int alsmiAddUnsecured1609p2Hdr(bool_t    WSA,
                               uint8_t  *inData,
                               uint16_t  inDataLength,
                               void     *additionalSecurityData)
{
    rsRadioType               radioType;
    uint32_t                  psid;
    uint8_t                  *outData;
    uint32_t                  outDataLength;
    SecuredMessageGeneratorC  smg;
    bool_t                    usingGeneratedSmg;
    AEROLINK_RESULT           returnCode;
    sendWSMStruct_S          *sendWSMStruct;
    uint8_t                   packetData[HDRS_LEN + MAX_WSM_DATA];

    ALSMI_DEBUG_PRINTF("alsmiAddUnsecured1609p2Hdr(%d, %p, %u, %p)\n",
                       WSA, inData, inDataLength, additionalSecurityData);

    if (!WSA) {
        sendWSMStruct = (sendWSMStruct_S *)additionalSecurityData;
        radioType     = sendWSMStruct->radioType;
        psid          = sendWSMStruct->psid;
    }
    else {
        sendWSMStruct = NULL;    /* Ends up being not used */
        radioType     = RT_DSRC; /* Ends up being not used */
        psid          = WSM_WSA_PSID;
    }

    /* Be sure data is not too long */
    if (inDataLength > MAX_WSM_DATA) {
        PRINT_MAX_N_TIMES(ERROR_MESSAGE_MAX_PRINT_TIMES,
                          "%s: Error: inDataLength(%u) > "
                          "MAX_WSM_DATA(%u)\n",
                          __func__, inDataLength, MAX_WSM_DATA);
        alsmiCallSignResCbFor1609p2(WSA, radioType, psid,
                                    WS_ERR_PAYLOAD_TOO_LARGE, 0);
        return -1;
    }

    /* Lock Aerolink calls */
    pthread_mutex_lock(&aerolinkCallMutex);

    /* Use the sigm_smg if it is not already in use. Otherwise, generate an smg
     * to use. */
    if (!sign_smgInUse) {
        ALSMI_DEBUG_PRINTF("%s: Allocating pre-defined smg\n", __func__);
        smg               = sign_smg;
        usingGeneratedSmg = FALSE;
        sign_smgInUse     = TRUE;
    }
    else {
        AEROLINK_CALL_PRINTF("AEROLINK: %s: Calling smg_new(%p, %p)\n",
                             __func__, context, &smg);
        returnCode = (secmode == 0) ? dummy_smg_new(context, &smg) :
                                      smg_new(context, &smg);
        AEROLINK_CALL_PRINTF("AEROLINK: %s: smg_new() returned %d; smg = %p\n",
                             __func__, returnCode, smg);

        if (returnCode != WS_SUCCESS) {
            /* Unlock Aerolink calls */
            pthread_mutex_unlock(&aerolinkCallMutex);
            PRINT_MAX_N_TIMES(ERROR_MESSAGE_MAX_PRINT_TIMES,
                              "%s: Unable to create a secured message generator (%s)\n",
                              __func__, strerror(returnCode));
            alsmiCallSignResCbFor1609p2(WSA, radioType, psid, returnCode, 0);
            return -1;
        }

        usingGeneratedSmg = TRUE;
    }

    /* Add 1609.2 header to the data */
    /* Allow room at front of packet for adding of various headers later */
    outData       = &packetData[HDRS_LEN];
    outDataLength = MAX_WSM_DATA;
    shm_rsk_ptr->ALSMICnt.Hdr1609p2Requests++;
    AEROLINK_CALL_PRINTF("AEROLINK: %s: Calling smg_createUnsecuredData(%p, %p, %u, %p, %p)\n",
                         __func__, smg, inData, inDataLength,
                         outData, &outDataLength);
    returnCode = (secmode == 0) ?
                 dummy_smg_createUnsecuredData(smg, inData, inDataLength,
                                               outData, &outDataLength) :
                 smg_createUnsecuredData(smg, inData, inDataLength,
                                         outData, &outDataLength);
    AEROLINK_CALL_PRINTF("AEROLINK: %s: smg_createUnsecuredData() returned %s\n",
                         __func__, ws_strerror(returnCode));

    if (!usingGeneratedSmg) {
        ALSMI_DEBUG_PRINTF("%s: Deallocating pre-defined smg\n", __func__);
        sign_smgInUse = FALSE;
    }
    else {
        AEROLINK_CALL_PRINTF("AEROLINK: %s: Calling smg_delete(%p)\n", __func__, smg);
        smg_delete(smg);
        AEROLINK_CALL_PRINTF("AEROLINK: %s: smg_delete() returned\n", __func__);
    }

    /* Unlock Aerolink calls */
    pthread_mutex_unlock(&aerolinkCallMutex);

    /* Report back sign results */
    alsmiCallSignResCbFor1609p2(WSA, radioType, psid, returnCode, outDataLength);

    if (returnCode == WS_SUCCESS) {
        shm_rsk_ptr->ALSMICnt.Hdr1609p2Successes++;

        if (WSA) {
            /* Call the WSM sign confirm routine */
            ALSMI_DEBUG_PRINTF("%s: Calling wsaSignSecurityCfm(%d, %p, %u, %p)\n",
                               __func__, returnCode, outData, outDataLength,
                               additionalSecurityData);
/* TODO: assuming this is needed for RSU. */
#if defined(SUPPORT_WSA_SIGN) || defined(HEAVY_DUTY)
            wsaSignSecurityCfm(returnCode, outData, outDataLength,
                               (wsaTxSecurityType *)additionalSecurityData);
#endif
        }
        else {
            /* Transmit the packet */
            ALSMI_DEBUG_PRINTF("%s: Calling sendWSMActual(%p, %p, %u)\n",
                               __func__, sendWSMStruct, outData, outDataLength);

//TODO: need this return
            return sendWSMActual(sendWSMStruct, outData, outDataLength);
        }
    }
    else {
        shm_rsk_ptr->ALSMICnt.Hdr1609p2Failures++;
        shm_rsk_ptr->ALSMICnt.LastHdr1609p2ErrorCode = returnCode;
    }

    return 0;
}

/*
 * PayloadType2securityType
 *
 * Convert an Aerolink PayloadType to a DENSO securityType.
 */
static securityType
PayloadType2securityType(PayloadType security)
{
    switch (security) {
    case PLT_SIGNED_SPDU:
        return SECURITY_SIGNED;
    case PLT_UNSECURED_APP_PAYLOAD:
    case PLT_UNSECURED_SPDU:
        return SECURITY_UNSECURED;
    default:
        return SECURITY_INVALID;
    }
}

/*
 * alsmiDoStripSecurity: Checks to see if the header is an unsecured 1609.2
 * header. If so, it is stripped off. Otherwise, control is passed to
 * smp_extract(). I am hoping that this function is temporary, and that
 * Onboard Security will accept our request and write a function similar to
 * smp_extract() that just finds the payload position and length, doesn't do
 * anything else, and operates much more efficiently. smp_extract was taking a
 * long time to complete, even when all it did was remove the unsecured 1609.2
 * header.
 *
 * Input parameters:
 *             smpHandle:
 *             message:
 *             messageLen: All same as for smp_extract()
 * Output parameters:
 *             type:
 *             payload:
 *             payloadLen: All same as for smp_extract()
 * Return Value:  0 on Success -1 on failure.
 */
static AEROLINK_RESULT alsmiDoStripSecurity(
    SecuredMessageParserC  smpHandle,
    uint8_t const *        message,
    uint32_t               messageLen,
    PayloadType           *spduType,
    uint8_t const        **payload,
    uint32_t              *payloadLen,
    PayloadType           *payloadType,
    uint8_t  const       **externalDataHash,
    ExternalDataHashAlg   *externalDataHashAlg)
{
    AEROLINK_RESULT result = WS_SUCCESS;

    if ((message[0] == 0x03) && (message[1] == 0x80)) {
        /* We have an unsecured 1609.2 header; remove the 03, 80, length-of-
         * length byte (if present), and the length itself. */
        if (message[2] == 0x82) {
            /* Two-byte payload length */
            *payload = message + 5;
            *payloadLen = messageLen - 5;
        }
        else if (message[2] == 0x81) {
            /* One-byte payload length */
            *payload = message + 4;
            *payloadLen = messageLen - 4;
        }
        else {
            if ((message[2] % 0x80) == 0x80) {
                /* Invalid length-of-length value */
                return WS_ERR_PARSE_FAIL;
            }

            /* One-byte payload length, but no length-of-length byte */
            *payload = message + 3;
            *payloadLen = messageLen - 3;
        }

        /* Packet type is unsecured */
        *spduType            = PLT_UNSECURED_SPDU;
        *payloadType         = PLT_UNSECURED_APP_PAYLOAD;
        *externalDataHash    = NULL;
        *externalDataHashAlg = EDHA_NONE;
    }
    else {
        if (!alsmiInitializeCalled) {
            /* If we have not initialized security, set result to WS_ERR_PARSE_FAIL
             * so that we will treat all packets as being unsecured. */
            return WS_ERR_PARSE_FAIL;
        }

        /* Pass control to smp_extract() */
        /* Lock Aerolink calls */
        pthread_mutex_lock(&aerolinkCallMutex);
        AEROLINK_CALL_PRINTF("AEROLINK: %s: Calling smp_extract(%p, %p, %d, %p, %p, %p, %p, %p, %p)\n",
                             __func__, smpHandle, message, messageLen, spduType,
                             payload, payloadLen, payloadType,
                             externalDataHash, externalDataHashAlg);
        result = (secmode == 0) ?
                 dummy_smp_extract(smpHandle, message, messageLen, spduType,
                                   payload, payloadLen, payloadType,
                                   externalDataHash, externalDataHashAlg) :
                 smp_extract(smpHandle, message, messageLen, spduType,
                              payload, payloadLen, payloadType,
                              externalDataHash, externalDataHashAlg);
        AEROLINK_CALL_PRINTF("AEROLINK: %s: smp_extract() returned %s\n", __func__, ws_strerror(result));
        /* Unlock Aerolink calls */
        pthread_mutex_unlock(&aerolinkCallMutex);

        if (debug_log && (result == WS_ERR_PARSE_FAIL)) {
            ALSMI_DEBUG_LOG("smp_extract() returned WS_ERR_PARSE_FAIL\n");
        }
    }

    return result;
}

/*
 * alsmiValidateVTPReportResult1
 *
 * The function that is called when verification is complete for
 * verify-then-process
 *
 * Note: This routine assumes the GPS locked. This routine unlocks the GPS.
 *
 * Input parameters:
 *             returnCode: Result of the verification
 *             userCallbackData: The VOD message sequence number, cast as a void pointer
 * Output parameters:
 *             None
 * Return Value:  None
 */
static void alsmiValidateVTPReportResult1(AEROLINK_RESULT  returnCode,
                                          uint32_t        *pSecurityFlags)
{
    AEROLINK_CALL_PRINTF("AEROLINK: alsmiValidateVTPReportResult1(%d, %p)\n",
                         returnCode, pSecurityFlags);

    /* Unlock the GPS lock */
    alsmiUnlockGps();
    /* Set the security result */
    *pSecurityFlags =
        (returnCode == WS_SUCCESS)      ? securedVerifiedSuccess :
        (returnCode == WS_ERR_UNSIGNED) ? unsecured              :
        (returnCode == WS_ERR_SIGNED_BUT_NOT_VERIFIED)
                                        ? securedButNotVerified  :
                                          securedVerifiedFail;
    /* Set the security return code */
    *pSecurityFlags |= ((uint32_t)returnCode << 8);
}

/* This VTP callback is called from everywhere except immediately after
 * smp_extract(). It is used so that you know whether to increment the verify
 * error count or not. */
static void alsmiValidateVTPReportResult(AEROLINK_RESULT  returnCode,
                                          uint32_t       *pSecurityFlags)
{
    if (returnCode == WS_SUCCESS) {
        shm_rsk_ptr->ALSMICnt.VerifySuccesses++;
    }
    else {
        shm_rsk_ptr->ALSMICnt.VerifyFailures++;
        shm_rsk_ptr->ALSMICnt.LastVerifyErrorCode = returnCode;
    }

    alsmiValidateVTPReportResult1(returnCode, pSecurityFlags);
}

/*
 * alsmiVerifyThenProcess: Strips the security information off of a packet and,
 * if the packet was signed, and also verifies it periodically.
 *
 * Input parameters:
 *             data: Pointer to the data to be verified. Note: points into the
 *                   structure pointed to be pPkb below.
 *             dataLength: Length of the data to be verified
 *             pPayload: Pointer to a pointer to the payload. This value is
 *                       written when the security is stripped.
 *             pPayloadLength: Pointer to to the payload length. This value is
 *                             written when the security is stripped.
 *             pktType: ALSMI_PKT_TYPE_WSMP, or ALSMI_PKT_TYPE_EU.
 *             pSecurity: Points to the Security field of the rsReceiveDataType
 *                        structure (for ALSMI_PKT_TYPE_WSMP) or to the
 *                        security field of the euPacketInfo structure (for
 *                        ALSMI_PKT_TYPE_EU).
 *             pSecurityFlags: Points to the SecurityFlags field of the
 *                             rsReceiveDataType structure (for
 *                             ALSMI_PKT_TYPE_WSMP) or to the securityFlags
 *                             field of the euPacketInfo structure (for
 *                             ALSMI_PKT_TYPE_EU).
 *                             securedVerifiedSuccess, unsecured,
 *                             securedButNotVerified, or securedVerifiedFail is
 *                             eventually stored in the location where this
 *                             points to.
 *             offsetToBasicHeader: Offset from pPkb->data to the start of the
 *                                  basic header. Used by ALSMI_PKT_TYPE_EU to
 *                                  calculate the offset to the start of the
 *                                  common header. Not used by
 *                                  ALSMI_PKT_TYPE_WSMP.
 *             pOffsetToCommonHeader: Points to the offsetToCommonHeader field
 *                                    of the euPacketInfo structure for
 *                                    ALSMI_PKT_TYPE_EU. Not used by
 *                                    ALSMI_PKT_TYPE_WSMP and can be set to
 *                                    NULL.
 *             pSspLen: Pointer to where to store the the SSP data length. Not
 *                      used by ALSMI_PKT_TYPE_EU.
 *             ssp: Pointer to where to store the SSP data. Not used by
 *                  ALSMI_PKT_TYPE_EU.
 *             pVodMsgSeqNum: Pointer to where to write the VOD message sequence number.
 *             psid: Needed in case the packet is unsecured. The WSMP header
 *                   will already have been stripped off, and the PSID will not
 *                   be in the 1609.2 header (if present). But the PSID will be
 *                   needed in order to determine which queue to eventually put
 *                   the packet on.
 *             radioType: The radio type the packet was received on.
 * Output parameters:
 *             The data pointed to by pSecurity and pSecurityFlags variables
 *             will be updated.
 * Return Value:  0 on Success -1 on failure.
 */
int alsmiVerifyThenProcess(uint8_t        *data,
                           uint16_t        dataLength,
                           uint8_t const **pPayload,
                           uint16_t       *pPayloadLength, 
                           uint8_t         pktType,
                           securityType   *pSecurity,
                           uint32_t       *pSecurityFlags,
                           uint32_t        offsetToBasicHeader,
                           uint8_t        *pOffsetToCommonHeader,
                           uint8_t        *pSspLen,
                           uint8_t        *ssp,
                           uint32_t       *pVodMsgSeqNum,
                           uint32_t        psid,
                           rsRadioType     radioType)
{
    AEROLINK_RESULT        result;
    verifyBufferElement_t *element;
    PayloadType            spduType;
    PayloadType            payloadType;
    uint8_t  const        *externalDataHash = NULL;
    ExternalDataHashAlg    externalDataHashAlg;
    uint32_t               certPsid;
    uint8_t const         *certSsp;
    uint32_t               certSspLen;
    int32_t                lat, lon;
    uint16_t               elev;
    int8_t                 res;
    uint32_t               tmpLength;
    bool_t                 verifyPacket;
    uint8_t               *unsignedData;
    uint16_t               unsignedDataLength;
    uint32_t               PSID = psid;

    ALSMI_DEBUG_PRINTF("alsmiVerifyThenProcess()\n");

    /* Be sure data is not too long */
    if (dataLength > MAX_UNSTRIPPED_MESSAGE_LENGTH) {
        PRINT_MAX_N_TIMES(ERROR_MESSAGE_MAX_PRINT_TIMES,
                          "%s: Error: data length > %u (%u)\n",
                          __func__, MAX_UNSTRIPPED_MESSAGE_LENGTH, dataLength);
        return -1;
    }

    /* Error if not ALSMI_PKT_TYPE_WSMP, ALSMI_PKT_TYPE_EU, or
       ALSMI_PKT_TYPE_WSA */
    if ((pktType != ALSMI_PKT_TYPE_WSMP) && (pktType != ALSMI_PKT_TYPE_EU) &&
        (pktType != ALSMI_PKT_TYPE_WSA)) {
        PRINT_MAX_N_TIMES(ERROR_MESSAGE_MAX_PRINT_TIMES,
                          "%s: Error: pktType != ALSMI_PKT_TYPE_WSMP or ALSMI_PKT_TYPE_EU (%d)\n",
                          __func__, pktType);
        return -1;
    }

    /* Lock the GPS. alsmiValidateVTPReportResult1() will unlock it. */
    gps_locked = TRUE;

    /*
     * Call our function, which checks to see if the header is an unsecured
     * 1609.2 header. If so, it is stripped off efficiently. Otherwise,
     * control is passed to smp_extract(). I am hoping that this function
     * is temporary, and that Onboard Security will accept our request and
     * write a function similar to smp_extract() that just finds the
     * payload position and length, doesn't do anything else, and operates
     * much more efficiently. If they do so, I will replace this call with
     * a call to the new Aerolink function, and delete the definition of
     * alsmiDoStripSecurity(). */
    /* Be sure aerolinkCallMutex is unlocked here, or the following
     * function call will hang!! */
    result = alsmiDoStripSecurity(vtp_smp, data, dataLength, &spduType,
                                  pPayload, &tmpLength, &payloadType,
                                  &externalDataHash, &externalDataHashAlg);
    *pPayloadLength    = tmpLength;
    unsignedData       = (uint8_t *)*pPayload;
    unsignedDataLength = tmpLength;

    /* If we got a parse error, treat the packet as if it were unsigned */
    if (result == WS_ERR_PARSE_FAIL) {
        result             = WS_SUCCESS;
        spduType           = PLT_UNSECURED_SPDU;
        unsignedData       = data;
        unsignedDataLength = dataLength;
        /* payloadType, externalDataHash, and externalDataHashAlg currently
         * not used */
    }
    else {
        shm_rsk_ptr->ALSMICnt.StripRequests++;

        /* Count up the strip success/fail statistics. If extract failed, return an
         * error */
        if (result == WS_SUCCESS) {
            shm_rsk_ptr->ALSMICnt.StripSuccesses++;
        }
        else {
            shm_rsk_ptr->ALSMICnt.StripFailures++;
            shm_rsk_ptr->ALSMICnt.LastStripErrorCode = result;
            alsmiValidateVTPReportResult1(result, pSecurityFlags);
            return 0;
        }
    }
    
    /* Set the security type */
    *pSecurity = PayloadType2securityType(spduType);

    /* If not signed, or it's a WSA, send packet to app and return. */
    if ((spduType != PLT_SIGNED_SPDU) || (pktType == ALSMI_PKT_TYPE_WSA)) {
        alsmiValidateVTPReportResult1(WS_ERR_UNSIGNED, pSecurityFlags);
        return 0;
    }

    /* Lock Aerolink calls */
    pthread_mutex_lock(&aerolinkCallMutex);

    /* Get the SSP data */
    AEROLINK_CALL_PRINTF("AEROLINK: %s: Calling smp_getServiceSpecificPermissions(%p, %p, %p)\n",
                         __func__, vtp_smp, &certSsp, &certSspLen);
    result = (secmode == 0) ?
             dummy_smp_getServiceSpecificPermissions(vtp_smp, &certSsp,
                                                     &certSspLen) :
             smp_getServiceSpecificPermissions(vtp_smp, &certSsp,
                                               &certSspLen);
    AEROLINK_CALL_PRINTF("AEROLINK: %s: smp_getServiceSpecificPermissions() returned %s\n",
                         __func__, ws_strerror(result));

    if (result == WS_SUCCESS) {
        if (pktType == ALSMI_PKT_TYPE_WSMP) {
            *pSspLen = certSspLen;

            if (certSspLen != 0) {
                memcpy(ssp, certSsp, certSspLen);
            }
        }
    }
    else {
        /* WS_ERR_NO_CERTIFICATE can happen a lot when you receive
         * packets from an RV you haven't seen before. The error will
         * occur until you receive a packet with a full certificate
         * from the RV. So as not to confuse the user, don't print an
         * error message in this case. */
        if (result != WS_ERR_NO_CERTIFICATE) {
            PRINT_MAX_N_TIMES(ERROR_MESSAGE_MAX_PRINT_TIMES,
                              "%s: smp_getServiceSpecificPermissions() failed (%s)\n",
                              __func__, ws_strerror(result));
        }

        if (pktType == ALSMI_PKT_TYPE_WSMP) {
            *pSspLen = 0;
            certSspLen = 0;
        }
    }

    /* Get the PSID data */
    AEROLINK_CALL_PRINTF("AEROLINK: %s: Calling smp_getPsid(%p, %p)\n",
                         __func__, vtp_smp, &certPsid);
    result = (secmode == 0) ? dummy_smp_getPsid(vtp_smp, &certPsid) :
             smp_getPsid(vtp_smp, &certPsid);
    AEROLINK_CALL_PRINTF("AEROLINK: %s: smp_getPsid() returned %s\n",
                         __func__, ws_strerror(result));

    if (result == WS_SUCCESS) {
        /* Store P-encoded equivalent for later verification */
        PSID = certPsid;

        if (decimal2PCoded(&PSID) == -1) {
            /* Unlock Aerolink calls */
            pthread_mutex_unlock(&aerolinkCallMutex);
            alsmiValidateVTPReportResult1(WS_ERR_INVALID_PSID, pSecurityFlags);
            return 0;
        }
    }
    else {
        PRINT_MAX_N_TIMES(ERROR_MESSAGE_MAX_PRINT_TIMES,
                          "%s: smp_getPsid() failed (%s)\n", __func__, ws_strerror(result));
        PSID = 0;
    }

    verifyPacket = TRUE;

    /* If milliseconds_per_verification equals 65535, then we don't want to
     * verify the packet. */
    if (milliseconds_per_verification == 65535) {
        verifyPacket = FALSE;
    }

    /* If milliseconds_per_verification equals 0, then we want to verify every
     * packet. */
    else if (milliseconds_per_verification != 0) {
        struct timeval tv;
        static double  lastVerificationTime = 0.0;
        double         currentTime;
        double         scale;

        /* If not enough time has passed since the last verify, don't verify this
         * packet, but send it to the app immediately. */
        gettimeofday(&tv, NULL);
        currentTime = ((double)tv.tv_sec * 1000.0) + ((double)tv.tv_usec / 1000.0);
        ALSMI_DEBUG_PRINTF("%s: currentTime = %.6lf; lastVerificationTime = %.6lf; diff = %.6lf\n", __func__,
                           currentTime, lastVerificationTime,
                           currentTime - lastVerificationTime);

        /* Check to see if enough time has elapsed since our last verification
         * attempt */
        if ((currentTime - lastVerificationTime) < scaled_milliseconds_per_verification) {
            verifyPacket = FALSE;
        }
        else {
            lastVerificationTime = currentTime;

            /* Generate a new scaled_milliseconds_per_verification, which is
             * milliseconds_per_verification +/- 10% */
            /* According to the documentation on random(), the function random()
             * returns a number between 0 and RAND_MAX. However, this is not true.
             * random() can return numbers higher than RAND_MAX. So either the
             * documentation is in error, or Linux's implementation is in error. In
             * any case, observations have shown that random() can return the
             * number 122498802. Therefore, doing modulo 1,000,000 arithmetic is
             * safe, at least for this OS. */
            scale = ((double)(random() % 1000000) / 1000000.0) * 0.2 + 0.9; /* 0.9 - 1.0999999 */
            scaled_milliseconds_per_verification = (double)milliseconds_per_verification * scale;
        }
    }

    /* Unlock Aerolink calls */
    pthread_mutex_unlock(&aerolinkCallMutex);

    if (!verifyPacket) {
        ALSMI_DEBUG_PRINTF("%s: Don't verify\n", __func__);

        /* We are not verifying this packet now, but if VOD is enabled, set up
         * the necessary data so it can be verified via VOD later if desired */
        if (alsmiVodEnabled) {
            /* Allocate a verify buffer element */
            pthread_mutex_lock(&verifyBufferMutex);
            element = &verifyBuffer[verifyBufferNextHandle % NUM_VERIFY_BUFFER_ELEMENTS];

            /* If the next verify buffer element contains a packet submitted
             * for verification, print error message and return */
            if ((element->vodMsgSeqNum != 0) && element->verificationSubmitted) {
                pthread_mutex_unlock(&verifyBufferMutex);
                alsmiValidateVTPReportResult1(WS_ERR_VERIFY_BUFFER_OVERFLOW,
                                              pSecurityFlags);
                PRINT_MAX_N_TIMES(ERROR_MESSAGE_MAX_PRINT_TIMES,
                                  "%s: Verify buffer overflow\n", __func__);
                return -1;
            }

            element->vodMsgSeqNum  = verifyBufferNextHandle;
            verifyBufferNextHandle = (verifyBufferNextHandle + 1) %
                                     NUM_VERIFY_BUFFER_HANDLES;

            /* Don't use 0 or 1 as a VOD message sequence number. A VOD message
             * sequence number value of 0 indicates that the packet has been
             * successfully verified by VTP. A VOD message sequence number
             * value of 1 indicates that the packet failed verification by VTP. */
            if ((verifyBufferNextHandle == 0) || (verifyBufferNextHandle == 1)) {
                verifyBufferNextHandle = 2;
            }

            /* Fill out the fields in the verify element */
            memcpy(element->signedData, data, dataLength);
            element->signedDataLength       = dataLength;
            element->pktType                = pktType;
            element->pSecurityFlags         = pSecurityFlags;
            element->unsignedData           = unsignedData;
            element->unsignedDataLength     = unsignedDataLength;
            element->PSID                   = PSID;
            element->offsetToBasicHeader    = offsetToBasicHeader;
            element->pOffsetToCommonHeader  = pOffsetToCommonHeader;
            element->sspLen                 = certSspLen;
            element->verificationSubmitted  = FALSE;
            element->verifyResult_vodResult = WS_ERR_VERIFY_NOT_ATTEMPTED;
            element->radioType              = radioType;

            if (certSspLen != 0) {
                memcpy(element->ssp, certSsp, certSspLen);
            }

            /* Return the VOD message sequence number */
            *pVodMsgSeqNum                  = element->vodMsgSeqNum;
            pthread_mutex_unlock(&verifyBufferMutex);
        } // if (alsmiVodEnabled)

        alsmiValidateVTPReportResult1(WS_ERR_SIGNED_BUT_NOT_VERIFIED,
                                      pSecurityFlags);
        return 0;
    } // if (!verifyPacket)

    /* We will now perform verification of this packet now (VtP) */

    /* Lock Aerolink calls */
    pthread_mutex_lock(&aerolinkCallMutex);

    ALSMI_DEBUG_PRINTF("%s: Do verify\n", __func__);
    shm_rsk_ptr->ALSMICnt.VerifyRequests++;

    /* Determine whether or not to get the generation location from the body of
     * the BSM based on the PSID */
    res = alsmiGetGenLocFromBsm(PSID);

    if (res == -1) {
        /* PSID not in table, meaning not in any profile file */
        result = WS_ERR_INVALID_PSID;
        goto callalsmiValidateVTPReportResult;
    }

    if (res == 1) {
        /* Get the generation location from the body of the BSM */
        result = alsmiASN1ExtractJ2735_LatLonElev(&lat, &lon, &elev, unsignedData, unsignedDataLength);

        if (result != WS_SUCCESS) {
            goto callalsmiValidateVTPReportResult;
        }

        /* Set the generation location in Aerolink */
        AEROLINK_CALL_PRINTF("AEROLINK: %s: Calling smp_setGenerationLocation(%p, %d, %d, %d)\n",
                             __func__, vtp_smp, lat, lon, elev);
        result = (secmode == 0) ? WS_SUCCESS :
                 smp_setGenerationLocation(vtp_smp, lat, lon, elev);
        AEROLINK_CALL_PRINTF("AEROLINK: %s: smp_setGenerationLocation() returned %s\n", __func__, ws_strerror(result));

        if (result != WS_SUCCESS) {
            goto callalsmiValidateVTPReportResult;
        }
    }

    /* Check relevance */
    AEROLINK_CALL_PRINTF("AEROLINK: %s: Calling smp_checkRelevance(%p)\n", __func__, vtp_smp);
    result = (secmode == 0) ? WS_SUCCESS : smp_checkRelevance(vtp_smp);
    AEROLINK_CALL_PRINTF("AEROLINK: %s: smp_checkRelevance() returned %s\n", __func__, ws_strerror(result));

    /* If verification failed, send packet to app and return */
    if (result != WS_SUCCESS) {
        goto callalsmiValidateVTPReportResult;
    }

    /* Check consistency */
    AEROLINK_CALL_PRINTF("AEROLINK: %s: Calling smp_checkConsistency(%p)\n", __func__, vtp_smp);
    result = (secmode == 0) ? WS_SUCCESS : smp_checkConsistency(vtp_smp);
    AEROLINK_CALL_PRINTF("AEROLINK: %s: smp_checkConsistency() returned %s\n", __func__, ws_strerror(result));

    /* If verification failed, send packet to app and return */
    if (result != WS_SUCCESS) {
        goto callalsmiValidateVTPReportResult;
    }

    /* Check signatures synchronously. This avoids the problem where we may
     * be waiting for an asynchronous verify to complete, and another packet
     * comes in that we send up to the application immediately. This could
     * possibly result in packets being received out of order. */
    AEROLINK_CALL_PRINTF("AEROLINK: %s: Calling smp_verifySignatures(%p)\n",
                         __func__, vtp_smp);
    result = (secmode == 0) ? 
             (dummy_verify_succeeds ? WS_SUCCESS : WS_ERR_MESSAGE_SIGNATURE_VERIFICATION_FAILED) :
             smp_verifySignatures(vtp_smp);
    AEROLINK_CALL_PRINTF("AEROLINK: %s: smp_verifySignatures() returned %s\n",
                         __func__, ws_strerror(result));

callalsmiValidateVTPReportResult:
    /* Since the verify is synchronous, always call the callback function with
     * the result */
    /* Unlock Aerolink calls */
    pthread_mutex_unlock(&aerolinkCallMutex);
    alsmiValidateVTPReportResult(result, pSecurityFlags);
    *pVodMsgSeqNum = (result == WS_SUCCESS) ? 0 : 1;
    return 0;
}

/*
 * alsmiVerifyThenProcessIeee: Strips the security information off of an IEEE
 * packet and, if the packet was signed, and also verifies it periodically.
 *
 * Input parameters:
 *             recv:             Pointer to the rsReceiveDataType structure
 *                               associated with this packet.
 *             packet: Pointer to the data to be verified. Note: points into the
 *                   structure pointed to be pPkb below.
 *             packetLength: Length of the data to be verified
 *             pPayload: Pointer to a pointer to the payload. This value is
 *                       written when the security is stripped.
 *             pPayloadLenget: Pointer to to the payload length. This value is
 *                             written when the security is stripped.
 *             radioType: The radio type the packet was received on.
 * Output parameters:
 *             The data pointed to by pSecurity and pSecurityFlags variables
 *             will be updated.
 * Return Value:  0 on Success -1 on failure.
 */
int alsmiVerifyThenProcessIeee(rsReceiveDataType *recv, uint8_t *packet,
                               uint16_t packetLength, uint8_t const **pPayload,
                               uint16_t *pPayloadLength, rsRadioType radioType)
{
    return alsmiVerifyThenProcess(packet,
                                  packetLength,
                                  pPayload,
                                  pPayloadLength,
                                  ALSMI_PKT_TYPE_WSMP,
                                  &recv->u.wsm.security,
                                  &recv->u.wsm.securityFlags,
                                  0, NULL,
                                  &recv->u.wsm.sspLen,
                                  recv->u.wsm.ssp,
                                  &recv->u.wsm.vodMsgSeqNum,
                                  recv->u.wsm.psid,
                                  radioType);
}

/*
 * alsmiVerifyWSACallback1
 *
 * The callback function that is called when WSA verification is complete.
 *
 * Note: This routine assumes GPS is locked. This routine unlocks the GPS.
 *
 * Input parameters:
 *             returnCode: Result of the verification
 *             userCallbackData: The VOD message sequence number, cast as a void pointer
 * Output parameters:
 *             None
 * Return Value:  None
 */
static void alsmiVerifyWSACallback1(AEROLINK_RESULT  returnCode,
                                    void            *userCallbackData)
{
    uint32_t               vodMsgSeqNum = *((uint32_t *)userCallbackData);
    verifyBufferElement_t *element;
    rsReceiveDataType      recv;

    AEROLINK_CALL_PRINTF("AEROLINK: alsmiVerifyWSACallback1(%d, %u)\n",
                         returnCode, vodMsgSeqNum);

    /* Unlock the GPS lock */
    alsmiUnlockGps();

    element = &verifyBuffer[vodMsgSeqNum % NUM_VERIFY_BUFFER_ELEMENTS];

    if (vodMsgSeqNum == element->vodMsgSeqNum) {
        if ((verify_wsa_results_options == smi_results_all) ||
            (returnCode != WS_SUCCESS)) {
            recv.indType      = IND_VERIFY_WSA_RESULTS;
            recv.u.returnCode = returnCode;
            /* Send the indication to RIS */
            sendIndicationToRisByPid(verify_wsa_results_pid, &recv);
        }

        /* Send a service available indication to the application */
        ALSMI_DEBUG_PRINTF("%s: Calling verifyWSACfm(%d, %u)\n",
                           __func__, returnCode, element->signedWsaHandle);
        verifyWSACfm(returnCode, element->signedWsaHandle);

        /* Make the entry usable again */
        pthread_mutex_lock(&verifyBufferMutex);
        element->verificationSubmitted  = FALSE;
        pthread_mutex_unlock(&verifyBufferMutex);

        /* Deallocate the SMP */
        if (secmode != 0) {
            pthread_mutex_lock(&aerolinkCallMutex);
            AEROLINK_CALL_PRINTF("AEROLINK: %s: Calling smp_delete(%p)\n", __func__, element->smp);
            smp_delete(element->smp);
            AEROLINK_CALL_PRINTF("AEROLINK: %s: smp_delete() returned\n", __func__);
            pthread_mutex_unlock(&aerolinkCallMutex);
        }

    }
    else {
        shm_rsk_ptr->WSACnt.WSAVerifyFailed++;
        shm_rsk_ptr->WSACnt.WSASecurityVerifyInvalidHandle++;
    }
}

/* This VTP callback is called from everywhere except immediately after
 * smp_extract(). It is used so that you know whether to increment the verify
 * error count or not. */
static void alsmiVerifyWSACallback(AEROLINK_RESULT  returnCode,
                                   void            *userCallbackData)
{
    if (returnCode == WS_SUCCESS) {
        shm_rsk_ptr->ALSMICnt.VerifySuccesses++;
    }
    else {
        shm_rsk_ptr->ALSMICnt.VerifyFailures++;
        shm_rsk_ptr->ALSMICnt.VerifyCBFailures++;
        shm_rsk_ptr->ALSMICnt.LastVerifyErrorCode = returnCode;
    }

    alsmiVerifyWSACallback1(returnCode, userCallbackData);
}

/*
 * alsmiVerifyWSA: Strips the security information off of a WSA and verifies
 * it. The message does not go through the message buffer.
 *
 * A call to this function may eventually result in a call to
 * alsmiVerifyWSACallback1() (perhaps before alsmiVerifyWSA() returns.)
 *
 * Input parameters:
 *             wsaData: Pointer to the WSA data to be verified
 *             wsaLen: Length of the WSA data to be verified
 *             signedWsaHandle: Handle to the signed WSA
 * Output parameters:
 *             vodResult: The Aerolink result code is written to here
 * Return Value:  RS_SUCCESS on success, RS_EVERWSAFAIL on failure
 */
rsResultCodeType alsmiVerifyWSA(uint8_t *wsaData, uint16_t wsaLen,
                                uint32_t signedWsaHandle,
                                AEROLINK_RESULT *vodResult)
{
    AEROLINK_RESULT        result;
    verifyBufferElement_t *element;
    uint8_t               *unsignedData;
    uint32_t               unsignedDataLength;
    PayloadType            spduType;
    PayloadType            payloadType;
    uint8_t const         *receivedExternalDataHash = NULL;
    ExternalDataHashAlg    externalDataHashAlg;
    SecuredMessageParserC  smp;

    ALSMI_DEBUG_PRINTF("alsmiVerifyWSA(0p%p, %u, %u, 0p%p)\n",
                       wsaData, wsaLen, signedWsaHandle, vodResult);

    /* Be sure data is not too long */
    if (wsaLen > MAX_UNSTRIPPED_MESSAGE_LENGTH) {
        shm_rsk_ptr->WSACnt.WSAVerifyFailed++;
        PRINT_MAX_N_TIMES(ERROR_MESSAGE_MAX_PRINT_TIMES,
                          "%s: Error: data length > %u (%u)\n",
                          __func__, MAX_UNSTRIPPED_MESSAGE_LENGTH, wsaLen);
        *vodResult = WS_ERR_PAYLOAD_TOO_LARGE;
        return RS_EVERWSAFAIL;
    }

    /* Lock the GPS. alsmiVerifyWSACallback1() will unlock it. */
    gps_locked = TRUE;

    /* Allocate an SMP */
    pthread_mutex_lock(&aerolinkCallMutex);
    AEROLINK_CALL_PRINTF("AEROLINK: %s: Calling smp_new(%p, %p)\n",
                         __func__, context, &smp);
    result = (secmode == 0) ? dummy_smp_new(context, &smp) :
             smp_new(context, &smp);
    AEROLINK_CALL_PRINTF("AEROLINK: %s: smp_new() returned %d; element->smg = %p\n",
                         __func__, result, smp);
    pthread_mutex_unlock(&aerolinkCallMutex);

    if (result != WS_SUCCESS) {
        shm_rsk_ptr->WSACnt.WSAVerifyFailed++;
        alsmiUnlockGps();
        PRINT_MAX_N_TIMES(ERROR_MESSAGE_MAX_PRINT_TIMES,
                          "%s: Unable to create a secured message parser (%s)\n",
                          __func__, ws_strerror(result));
        *vodResult = result;
        return RS_EVERWSAFAIL;
    }

    /* Allocate a verify buffer element */
    pthread_mutex_lock(&verifyBufferMutex);
    element = &verifyBuffer[verifyBufferNextHandle % NUM_VERIFY_BUFFER_ELEMENTS];

    /* If the next verify buffer element contains a packet submitted for
     * verification, print error message and return */
    if ((element->vodMsgSeqNum != 0) && element->verificationSubmitted) {
        pthread_mutex_unlock(&verifyBufferMutex);
        
        if (secmode != 0) {
            pthread_mutex_lock(&aerolinkCallMutex);
            smp_delete(smp);
            pthread_mutex_unlock(&aerolinkCallMutex);
        }

        shm_rsk_ptr->WSACnt.WSAVerifyFailed++;
        alsmiUnlockGps();
        PRINT_MAX_N_TIMES(ERROR_MESSAGE_MAX_PRINT_TIMES,
                          "%s: Verify buffer overflow\n", __func__);
        *vodResult = WS_ERR_VERIFY_BUFFER_OVERFLOW;
        return RS_EVERWSAFAIL;
    }

    /* Store some things in the verify buffer */
    element->smp                    = smp;
    element->verificationSubmitted  = TRUE;
    element->signedWsaHandle        = signedWsaHandle;
    element->vodMsgSeqNum           = verifyBufferNextHandle;
    verifyBufferNextHandle          = (verifyBufferNextHandle + 1) %
                                      NUM_VERIFY_BUFFER_HANDLES;

    /* Don't use 0 or 1 as a VOD message sequence number. A VOD message sequence number value of 1 indicates that
     * the packet has been successfully verified by VTP. A VOD message sequence number value of
     * 1 indicates that the packet failed verification by VTP. */
    if ((verifyBufferNextHandle == 0) || (verifyBufferNextHandle == 1)) {
        verifyBufferNextHandle = 2;
    }

    pthread_mutex_unlock(&verifyBufferMutex);

    /* Lock Aerolink calls */
    pthread_mutex_lock(&aerolinkCallMutex);

    /* Strip off the security, if any */
    shm_rsk_ptr->ALSMICnt.StripRequests++;
    AEROLINK_CALL_PRINTF("AEROLINK: %s: Calling smp_extract(%p, %p, %d, %p, %p, %p, %p, %p, %p)\n",
                         __func__, smp, wsaData, wsaLen,
                         &spduType, &unsignedData, &unsignedDataLength,
                         &payloadType, &receivedExternalDataHash,
                         &externalDataHashAlg);
    result = (secmode == 0) ?
             dummy_smp_extract(smp, wsaData, wsaLen, &spduType,
                               (const uint8_t **)&unsignedData, &unsignedDataLength,
                               &payloadType, &receivedExternalDataHash,
                               &externalDataHashAlg) :
             smp_extract(smp, wsaData, wsaLen, &spduType,
                         (const uint8_t **)&unsignedData, &unsignedDataLength,
                         &payloadType, &receivedExternalDataHash,
                         &externalDataHashAlg);
    AEROLINK_CALL_PRINTF("AEROLINK: %s: smp_extract() returned %s\n",
                         __func__, ws_strerror(result));

    if (result == WS_SUCCESS) {
        shm_rsk_ptr->ALSMICnt.StripSuccesses++;
    }
    else {
        shm_rsk_ptr->ALSMICnt.StripFailures++;
        shm_rsk_ptr->ALSMICnt.LastStripErrorCode = result;
    }

    /* If extract failed, call the callback function with the result */
    if (result != WS_SUCCESS) {
        goto alsmiVerifyWSAExit;
    }

    /* If not signed, call the callback function with the result */
    if (spduType != PLT_SIGNED_SPDU) {
        result = WS_ERR_UNSIGNED;
        goto alsmiVerifyWSAExit;
    }

    shm_rsk_ptr->ALSMICnt.VerifyRequests++;

    /* Check relevance */
    AEROLINK_CALL_PRINTF("AEROLINK: %s: Calling smp_checkRelevance(%p)\n", __func__, smp);
    result = (secmode == 0) ? WS_SUCCESS : smp_checkRelevance(smp);
    AEROLINK_CALL_PRINTF("AEROLINK: %s: smp_checkRelevance() returned %s\n", __func__, ws_strerror(result));

    /* If check relevance failed, call the callback function with the result */
    if (result != WS_SUCCESS) {
        goto alsmiVerifyWSAExit;
    }

    /* Check consistency */
    AEROLINK_CALL_PRINTF("AEROLINK: %s: Calling smp_checkConsistency(%p)\n", __func__, smp);
    result = (secmode == 0) ? WS_SUCCESS : smp_checkConsistency(smp);
    AEROLINK_CALL_PRINTF("AEROLINK: %s: smp_checkConsistency() returned %s\n", __func__, ws_strerror(result));

    /* If check consistency failed, call the callback function with the result */
    if (result != WS_SUCCESS) {
        goto alsmiVerifyWSAExit;
    }

    /* Check signatures */
    /* Submit the packet for verification */
    AEROLINK_CALL_PRINTF("AEROLINK: %s: Calling smp_verifySignaturesAsync(%p, %u, %p)\n",
                         __func__, smp, element->vodMsgSeqNum, alsmiVerifyWSACallback);
    result = (secmode == 0) ?
             dummy_smp_verifySignaturesAsync(smp,
                                       &element->vodMsgSeqNum,
                                       alsmiVerifyWSACallback) :
             smp_verifySignaturesAsync(smp,
                                       &element->vodMsgSeqNum,
                                       alsmiVerifyWSACallback);
    AEROLINK_CALL_PRINTF("AEROLINK: %s: smp_verifySignaturesAsync() returned %s\n",
                         __func__, ws_strerror(result));

alsmiVerifyWSAExit:
    /* Unlock Aerolink calls */
    pthread_mutex_unlock(&aerolinkCallMutex);

    if (result != WS_SUCCESS) {
        /* If error, call the callback function with the result */
        shm_rsk_ptr->WSACnt.WSASecurityVerifySignatureFailed++;
        alsmiVerifyWSACallback(result, &element->vodMsgSeqNum);
    }

    /* If result is WS_SUCCESS, smp_verifySignaturesAsync is running in the
     * background. It will call alsmiVerifyWSACallback when the verify is
     * done. Setting *vodResult to WS_SUCCESS means that the submission of the
     * WSA for verification was a success, not that the verification itself was
     * a success. */
    *vodResult = result;
    return RS_SUCCESS;
}

/*
 * alsmiIDChangeInitCallback
 *
 * The callback function that is called before an ID (certificate) change
 *
 * Input parameters:
 *             userData: Supplied during securityServices_idChangeRegister
 *                       Points to the LCM name
 *             numCerts: Number of certificates that are available. NOTE: This
 *                       value CAN be zero under some circumstances!
 * Output parameters:
 *             certIndex: Write 1-20 here to select which certificate to use.
 *                        Write 0 here to let Aerolink select which certificate
 *                        to use.
 * Return Value:  None
 */
static void alsmiIDChangeInitCallback(void    *userData,
                                      uint8_t  numCerts,
                                      uint8_t *certIndex)
{
    AEROLINK_CALL_PRINTF("AEROLINK: alsmiIDChangeInitCallback(%p, %u, %p)\n",
                         userData, numCerts, certIndex);
    shm_rsk_ptr->ALSMICnt.IdChangeInitCallbacks++;

    /* If numCerts is 0, count it up */
    if (numCerts == 0) {
        shm_rsk_ptr->ALSMICnt.IdChangeInitZeroCerts++;
        /* Let Aerolink choose which certificate to use */
        *certIndex = 0;
    }
    else if (randomCertificate) {
        /* Pick a random certificate to use */
        *certIndex = random() % numCerts + 1;
        randomCertificate = FALSE;
    }
    else {
        /* Let Aerolink choose which certificate to use */
        *certIndex = 0;
    }

    /* Set cert change in progress flag */
    certificateChangeInProgress = TRUE;
}

/*
 * alsmiIDChangeDoneCallback
 *
 * The callback function that is called when an ID (certificate) change is done
 *
 * Input parameters:
 *             returnCode: WS_SUCCESS if certificate changed successfully.
 *                         Error code if not.
 *             userData: Supplied during securityServices_idChangeRegister
 *                       Points to the LCM name
 *             certId: Pointer to the last eight bytes of the SHA-256 or the
 *                     new certificate
 * Output parameters:
 *             None
 * Return Value:  None
 */
static void alsmiIDChangeDoneCallback(AEROLINK_RESULT  returnCode,
                                      void            *userData,
                                      uint8_t const   *certId)
{
    AEROLINK_RESULT   result;
    rsReceiveDataType recv;
#ifdef CERTIFICATE_CHANGE_TIMING_TEST
    struct timeval    cctt_tv2;
    static bool_t     cctt_firstTime = TRUE;
    static uint32_t   maxCertChangeTime = 0;
#endif // CERTIFICATE_CHANGE_TIMING_TEST

    certChangeTries = 0;

    /* Cancel the certificate change timer */
    if (returnCode != WS_ERR_CERTIFICATE_CHANGE_TIMEOUT) {
        CANCEL_CERT_CHANGE_TIMER;
        initialCertChangeDone = TRUE;
    }

#ifdef CERTIFICATE_CHANGE_TIMING_TEST
    gettimeofday(&cctt_tv2, NULL);

    if (cctt_tv1.tv_usec > cctt_tv2.tv_usec) {
        cctt_tv2.tv_usec += 1000000;
    }

    if (cctt_firstTime || ((cctt_tv2.tv_usec - cctt_tv1.tv_usec) > maxCertChangeTime)) {
        cctt_firstTime = FALSE;
        maxCertChangeTime = cctt_tv2.tv_usec - cctt_tv1.tv_usec;
        fprintf(stdout,"%s: Max Certificate change time %u usec\n", __func__, maxCertChangeTime);
        fflush(stdout);
    }
#endif // CERTIFICATE_CHANGE_TIMING_TEST

    AEROLINK_CALL_PRINTF("AEROLINK: alsmiIDChangeDoneCallback(%d, %p, {%02x, %02x, %02x, %02x, %02x, %02x, %02x, %02x})\n",
                         returnCode, userData,
                         certId[0], certId[1], certId[2], certId[3],
                         certId[4], certId[5], certId[6], certId[7]);

    /* Disable ID-change protocol (it will be enabled when we request a
     * certificate change. ID-change protocol will be initiated anyway when
     * a currently in-use certificate is about to expire). */
    result = alsmi_idChangeLock();

    if (result != WS_SUCCESS) {
        PRINT_MAX_N_TIMES(ERROR_MESSAGE_MAX_PRINT_TIMES,
                          "%s: Unable to block ID-change protocol (%s)\n",
                          __func__, ws_strerror(result));
    }

    /* Clear cert change in progress flag */
    certificateChangeInProgress = FALSE;

    if (returnCode == WS_SUCCESS) {
        shm_rsk_ptr->ALSMICnt.CertChangeSuccesses++;
    }
    else {
        shm_rsk_ptr->ALSMICnt.CertChangeFailures++;
        shm_rsk_ptr->ALSMICnt.CertChangeLastErrorCode = returnCode;
    }

    /* Send back the certificate change results */
    recv.indType  = IND_CERT_CHANGE_RESULTS;
    recv.u.certChangeResults.returnCode = (returnCode & 0xff);
    memcpy(recv.u.certChangeResults.certId, certId,
           sizeof(recv.u.certChangeResults.certId));
    memcpy(recv.u.certChangeResults.lcmName, userData,
           sizeof(recv.u.certChangeResults.lcmName) - 1);
    recv.u.certChangeResults.lcmName[sizeof(recv.u.certChangeResults.lcmName) - 1] = '\0';
    /* Send the indication to RIS */
    sendIndicationToRisByPid(cert_change_pid, &recv);
}

/*
 * This function is called upon a certificate change timeout
 * Input parameters:  None
 * Output parameters: None
 * Return Value:      None
 */
static void alsmiCertChangeTimeout(void)
{
    uint8_t userData[23];
    uint8_t certId[8];

    PRINT_MAX_N_TIMES(ERROR_MESSAGE_MAX_PRINT_TIMES, "%s()\n", __func__);

    /* If certificate change tries is less than maximum, retry the certificate
     * change */
    if (certChangeTries < MAX_CERT_CHANGE_TRIES) {
        certChangeTries++;
        shm_rsk_ptr->ALSMICnt.CertChangeRetries++;

        /* Be sure aerolinkCallMutex is unlocked here, or the following
         * function call will hang!! */
        if (alsmiSendCertChangeReq() == 0) {
            return;
        }
    }

    /* Abort the certificate change and call the certificiate change done
     * callback with a error */
    shm_rsk_ptr->ALSMICnt.CertChangeTimeouts++;
    memset(userData, 0, sizeof(userData));
    memset(certId,   0, sizeof(certId));
    alsmiIDChangeDoneCallback(WS_ERR_CERTIFICATE_CHANGE_TIMEOUT, userData,
                              certId);
}

/*
 * This function sets the certChangeTimeoutTestCount variable to the specified
 * amount. In int32_t alsmiSendCertChangeReq(), the call to
 * securityServices_idChangeInit() will be suppressed this many times, causing
 * the certificate change to time out.
 * Input parameters:  count - The value to set certChangeTimeoutTestCount to.
 * Output parameters: None
 * Return Value:      None
 */
void alsmiSetCertChangeTimeoutTestCount(uint16_t count)
{
    ALSMI_DEBUG_PRINTF("alsmiSetCertChangeTimeoutTestCount(%d)\n", count);
    certChangeTimeoutTestCount = count;
}

/*
 * This function simulates an unsolicited certificate change from Aerolink.
 * Input parameters:  None
 * Output parameters: None
 * Return Value:      None
 */
void alsmiUnsolicitedCertChangeTest(void)
{
    AEROLINK_RESULT result;

    ALSMI_DEBUG_PRINTF("alsmiUnsolicitedCertChangeTest()\n");

    if (secmode != 0) {
        alsmi_idChangeUnlock();
        AEROLINK_CALL_PRINTF("AEROLINK: %s: Calling securityServices_idChangeInit()\n",
                             __func__);
        result = securityServices_idChangeInit();
        AEROLINK_CALL_PRINTF("AEROLINK: %s: securityServices_idChangeInit() returned %s\n",
                             __func__, ws_strerror(result));
    }
}

/*
 * This function processes P2P data.
 * Input parameters:
 *             data: P2P data length.
 *             len: P2P data length.
 * Output parameters:
 *             None
 * Return Value:  0 if success. Error code if error.
 */
int alsmiProcessP2PData(uint8_t *data, uint16_t len)
{
    AEROLINK_RESULT result;

    ALSMI_DEBUG_PRINTF("alsmiProcessP2PData()\n");

    if (secmode == 0) {
        return 0;
    }

    if ((result = p2p_process(len, data)) != WS_SUCCESS) {
        return result;
    }

    return 0;
}

/* Dummy variables for ID change (certificate change) */
static void                 *idChangeRegisterUserData     = NULL;
static IdChangeInitCallback  idChangeInitCallbackFunction = NULL;
static IdChangeDoneCallback  idChangeDoneCallbackFunction = NULL;
static uint8_t               idChangeCertIndex            = 1;

/**
 * @brief Register an LCM to participate in the ID-change protocol.
 *
 * @param scHandle (IN)  of the context containing the LCM to be registered
 * @param lcmName (IN) to be registered
 * @param userData (IN) user supplied data that will be returned in the callback.
 * @param initCallbackFunction (IN) callback function to be called at the start of an id change.
 * @param doneCallbackFunction (IN) callback function to be called at the completion of an id change.
 *
 * @return AEROLINK_RESULT
 *
 * @retval WS_SUCCESS
 *     - operation was successful
 *
 * @retval WS_ERR_NOT_INITIALIZED
 *     - security services have not been initialized.
 *
 * @retval WS_ERR_BAD_ARGS
 *     - scHandle  is NULL
 *     - lcmName   is NULL
 *
 * @retval WS_ERR_INVALID_HANDLE
 *     - scHandle  does not refer to a valid security context
 *
 * @retval WS_ERR_ALREADY_REGISTERED
 *     -  the specified LCM has already been registered
 */
AEROLINK_RESULT dummy_securityServices_idChangeRegister(
        SecurityContextC             scHandle,
        char const                  *lcmName,
        void                        *userData,
        IdChangeInitCallback         initCallbackFunction,
        IdChangeDoneCallback         doneCallbackFunction)
{
    idChangeRegisterUserData     = userData;
    idChangeInitCallbackFunction = initCallbackFunction;
    idChangeDoneCallbackFunction = doneCallbackFunction;
    return WS_SUCCESS;
}

/**
 * @brief Unregister an LCM to participate in the ID Change protocol.
 *
 * @param scHandle (IN)  of the context containing the LCM to be unregistered
 * @param lcmName (IN) to be unregistered
 *
 * @return AEROLINK_RESULT
 *
 * @retval WS_SUCCESS
 *     - operation was successful
 *
 * @retval WS_ERR_NOT_INITIALIZED
 *     - security services have not been initialized.
 *
 * @retval WS_ERR_BAD_ARGS
 *     - scHandle  is NULL
 *     - lcmName   is NULL
 *
 * @retval WS_ERR_INVALID_HANDLE
 *     - scHandle does not refer to a valid security context
 *
 * @retval WS_ERR_NOT_REGISTERED
 *     -  the specified LCM is not registered
 *
 */
AEROLINK_RESULT dummy_securityServices_idChangeUnregister(
        SecurityContextC             scHandle,
        char const                  *lcmName)
{
    idChangeRegisterUserData     = NULL;
    idChangeInitCallbackFunction = NULL;
    idChangeDoneCallbackFunction = NULL;
    return WS_SUCCESS;
}

static void *dummy_securityServices_idChangeThread(void *arg)
{
    uint8_t newIdChangeCertIndex = 0x0;
    uint8_t certId[8];

    shm_rsk_ptr->bootupState |= 0x100; // alsmiTimeoutThread running
    ALSMI_DEBUG_PRINTF("dummy_securityServices_idChangeThread started\n");
    usleep(10000);

    if (idChangeInitCallbackFunction != NULL) {
        idChangeInitCallbackFunction(idChangeRegisterUserData, 20,
                                     &newIdChangeCertIndex);
    }

    if (newIdChangeCertIndex == 0) {
        idChangeCertIndex = (idChangeCertIndex + 1) % 21;

        if (idChangeCertIndex == 0) {
            idChangeCertIndex = 1;
        }
    }
    else {
        idChangeCertIndex = newIdChangeCertIndex;
    }

    memset(certId, idChangeCertIndex, 8);

    if (idChangeDoneCallbackFunction != NULL) {
        idChangeDoneCallbackFunction(WS_SUCCESS, idChangeRegisterUserData,
                                     certId);
    }

    shm_rsk_ptr->bootupState &= ~0x100; // alsmiTimeoutThread running
    ALSMI_DEBUG_PRINTF("dummy_securityServices_idChangeThread exiting\n");
    pthread_exit(NULL);
    return NULL;
}

/**
 * @brief Request an ID Change initiation.
 *
 * @return AEROLINK_RESULT
 *
 * @retval WS_SUCCESS
 *     - operation was successful
 *
 * @retval WS_ERR_NOT_INITIALIZED
 *     - security services have not been initialized.
 *
 * @retval WS_ERR_ID_CHANGE_BLOCKED
 *     - Id changes are blocked
 */
AEROLINK_RESULT dummy_securityServices_idChangeInit(void)
{
    pthread_t dummy_securityServices_idChangeThreadId;

    if (pthread_create(&dummy_securityServices_idChangeThreadId, NULL,
                       dummy_securityServices_idChangeThread, NULL) != 0) {
        fprintf(stdout,"%s: Error creating dummy_securityServices_idChangeThread: %s\n", __func__, strerror(errno));
        fflush(stdout);
    }
    else {
        ALSMI_DEBUG_PRINTF("%s: Created dummy_securityServices_idChangeThread\n", __func__);
    }

    return WS_SUCCESS;
}

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
AEROLINK_RESULT dummy_smg_new(
    SecurityContextC          scHandle,
    SecuredMessageGeneratorC *smgHandle)
{
    *smgHandle = (void *)dummy_handle_counter++;
    return WS_SUCCESS;
}

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
AEROLINK_RESULT dummy_smg_createUnsecuredData(
    SecuredMessageGeneratorC smgHandle,
    uint8_t const * const appPayload,
    uint32_t              appPayloadLen,
    uint8_t       * const unsecuredDataSPDU,
    uint32_t      * const unsecuredDataSPDULen)
{
    int hdrLen;

    if ((appPayload == NULL) || (unsecuredDataSPDU == NULL) ||
        (unsecuredDataSPDULen == NULL)) {
        return WS_ERR_BAD_ARGS;
    }

    if (appPayloadLen <= 127) {
        hdrLen = 3;
    }
    else if (appPayloadLen <= 255) {
        hdrLen = 4;
    }
    else {
        hdrLen = 5;
    }

    if (appPayloadLen + hdrLen > *unsecuredDataSPDULen) {
        return WS_ERR_BUFFER_TOO_SMALL;
    }

    unsecuredDataSPDU[0] = 0x03;
    unsecuredDataSPDU[1] = 0x80;

    if (appPayloadLen <= 127) {
        unsecuredDataSPDU[2] = appPayloadLen;
    }
    else if (appPayloadLen <= 255) {
        unsecuredDataSPDU[2] = 0x81;
        unsecuredDataSPDU[3] = appPayloadLen;
    }
    else {
        unsecuredDataSPDU[2] = 0x82;
        unsecuredDataSPDU[3] = (appPayloadLen & 0xff00) >> 8;
        unsecuredDataSPDU[4] = (appPayloadLen & 0x00ff);
    }

    memcpy(unsecuredDataSPDU + hdrLen, appPayload, appPayloadLen);
    *unsecuredDataSPDULen = appPayloadLen + hdrLen;
    return WS_SUCCESS;
}

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
AEROLINK_RESULT dummy_smg_signAsync(
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
    SignCallback                        callbackFunction)
{
    int hdrLen;

    if ((payload == NULL) || (signedDataSPDU == NULL)) {
        return WS_ERR_BAD_ARGS;
    }

    if (payloadLen <= 127) {
        hdrLen = 3;
    }
    else if (payloadLen <= 255) {
        hdrLen = 4;
    }
    else {
        hdrLen = 5;
    }

    if (payloadLen + hdrLen > signedDataSPDULen) {
        return WS_ERR_BUFFER_TOO_SMALL;
    }

    signedDataSPDU[0] = 0x03;
    signedDataSPDU[1] = 0x80;

    if (payloadLen <= 127) {
        signedDataSPDU[2] = payloadLen;
    }
    else if (payloadLen <= 255) {
        signedDataSPDU[2] = 0x81;
        signedDataSPDU[3] = payloadLen;
    }
    else {
        signedDataSPDU[2] = 0x82;
        signedDataSPDU[3] = (payloadLen % 0xff00) >> 8;
        signedDataSPDU[4] = (payloadLen % 0x00ff);
    }

    memcpy(signedDataSPDU + hdrLen, payload, payloadLen);
    callbackFunction(WS_SUCCESS, (uint8_t *)userCallbackData, signedDataSPDU,
                     payloadLen + hdrLen);
    return WS_SUCCESS;
}

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
AEROLINK_RESULT dummy_smp_new(
        SecurityContextC              scHandle,
        SecuredMessageParserC * const smpHandle)
{
    *smpHandle = (void *)dummy_handle_counter++;
    return WS_SUCCESS;
}

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
AEROLINK_RESULT dummy_smp_extract(
    SecuredMessageParserC  smpHandle,
    uint8_t const *        spdu,
    uint32_t               spduLen,
    PayloadType *          spduType,
    uint8_t const **       payload,
    uint32_t *             payloadLenOut,
    PayloadType *          payloadType,
    uint8_t const **       externalDataHash,
    ExternalDataHashAlg *  externalDataHashAlg)
{
    uint8_t const *ptr;
    int            hdrLen = 0;
    uint32_t       payloadLen;

    if ((spdu == NULL) || (spduType == NULL) || (payload == NULL) ||
        (payloadLenOut == NULL) || (payloadType == NULL)) {
        return WS_ERR_BAD_ARGS;
    }

    ptr = spdu;

    if (spduLen < 3) {
        return WS_ERR_BUFFER_TOO_SMALL;
    }

    if ((ptr[0] == 0x03) && (ptr[1] == 0x81)) {
        /* The packet is secured, skip past the first 4 bytes, which should be
         * 0x03, 0x81, 0x00, and 0x40, and account for this in the header
         * length */
        if (spduLen < 7) {
            return WS_ERR_BUFFER_TOO_SMALL;
        }

        ptr    += 4;
        hdrLen += 4;
        *spduType = PLT_SIGNED_SPDU;
    }
    else {
        /* The packet is unsecured */
        if (spduLen < 3) {
            return WS_ERR_BUFFER_TOO_SMALL;
        }

        *spduType = PLT_UNSECURED_SPDU;
    }

    /* What follows should be an unsecured 1609.2 header */
    if ((ptr[0] != 0x03) || (ptr[1] != 0x80)) {
        /* Not a valid unsecured 1609.2 header */
        return WS_ERR_PARSE_FAIL;
    }

    /* We have an unsecured 1609.2 header; remove the 03, 80, length-of-
     * length byte (if present), and the length itself. */
    if (ptr[2] == 0x82) {
        /* Two-byte payload length */
        hdrLen += 5;

        if (spduLen < hdrLen) {
            return WS_ERR_BUFFER_TOO_SMALL;
        }

        payloadLen = (ptr[3] << 8) + ptr[4];
    }
    else if (ptr[2] == 0x81) {
        /* One-byte payload length */
        hdrLen += 4;

        if (spduLen < hdrLen) {
            return WS_ERR_BUFFER_TOO_SMALL;
        }

        payloadLen = ptr[3];
    }
    else if (ptr[2] >= 0x80) {
        /* Invalid length-of-length value */
        return WS_ERR_PARSE_FAIL;
    }
    else {
        /* One-byte payload length, but no length-of-length byte */
        hdrLen += 3;
        payloadLen = ptr[2];
    }

    if (payloadLen > spduLen) {
        return WS_ERR_BUFFER_TOO_SMALL;
    }

    *payload       = spdu + hdrLen;
    *payloadLenOut = payloadLen;
    *payloadType   = PLT_UNSECURED_APP_PAYLOAD;
    return WS_SUCCESS;
}

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
AEROLINK_RESULT dummy_smp_getPsid(
    SecuredMessageParserC  smpHandle,
    uint32_t       * const psid)
{
    if (psid == NULL) {
        return WS_ERR_BAD_ARGS;
    }

    *psid = 0x20;
    return WS_SUCCESS;
}

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
AEROLINK_RESULT dummy_smp_getServiceSpecificPermissions(
    SecuredMessageParserC   smpHandle,
    uint8_t const  **       ssp,
    uint32_t        * const sspLen)
{
    if (sspLen == NULL) {
        return WS_ERR_BAD_ARGS;
    }

    *sspLen = 0;
    return WS_SUCCESS;
}

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
AEROLINK_RESULT dummy_smp_verifySignaturesAsync(
    SecuredMessageParserC smpHandle,
    void *userData,
    ValidateCallback callbackFunction)
{
    callbackFunction((dummy_verify_succeeds ? WS_SUCCESS :
                      WS_ERR_MESSAGE_SIGNATURE_VERIFICATION_FAILED), userData);
    return WS_SUCCESS;
}

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
AEROLINK_RESULT dummy_sc_open(
        char const       *filename,
        SecurityContextC *scHandle)
{
    *scHandle = (void *)dummy_handle_counter++;
    return WS_SUCCESS;
}

