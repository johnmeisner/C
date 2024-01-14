/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Filename: rs.h                                                            */
/*                                                                            */
/* Copyright (C) 2019 DENSO International America, Inc.                       */
/*                                                                            */
/* Description: Definitions that are internal to RIS and Radio Services.      */
/*                                                                            */
/* Revision History:                                                          */
/*                                                                            */
/* Date        Author        Comments                                         */
/* -------------------------------------------------------------------------- */
/* [2019-03-07][VROLLINGER]  Initial revision                                 */
/*----------------------------------------------------------------------------*/
#ifndef _RS_H
#define _RS_H

#include <sys/types.h>
#include "ris_struct.h"
#include "alsmi_struct.h"

#define MAX_SERVICES 64
/* Note that if MAX_APPS ever changes, you will need to change the number of
 * port numbers allocated for RIS receiving incoming indications in ipcsock.h
 * (search for MAX_APPS in that file). */
#define MAX_APPS     8

#define WSMP_NUM_EXT        3
#define SCH_EXTENDED_ACCESS 65535

#define NUM_WSA          10
#define MAX_SERVICE_INFO 8
#define MAX_CHANNEL_INFO 6

/* Type definition of WAVE Element ID */
#define WAVE_ID_RESERVED                      0
#define WAVE_ID_SERVICE_INFO                  1     // WSA service info
#define WAVE_ID_CHANNEL_INFO                  2     // WSA channel info
#define WAVE_ID_WRA                           3     // WSA WRA
#define WAVE_ID_TRANSMIT_POWER                4     // WSA/WSMP header
#define WAVE_ID_2DLOCATION                    5     // WSA header
#define WAVE_ID_3DLOCATION                    6     // WSA header
#define WAVE_ID_ADVERTISER_ID                 7     // WSA header
#define WAVE_ID_PROVIDER_SERVICE_CONTEXT      8     // WSA service info
#define WAVE_ID_IPV6ADDRESS                   9     // WSA service info
#define WAVE_ID_SERVICE_PORT                  10    // WSA service info
#define WAVE_ID_PROVIDER_MACADDRESS           11    // WSA service info
#define WAVE_ID_EDCA_PARAM_SET                12    // WSA channel info
#define WAVE_ID_SECONDARY_DNS                 13    // WSA WRA
#define WAVE_ID_GATEWAY_MACADDRESS            14    // WSA WRA
#define WAVE_ID_CHANNEL_NUMBER                15    // WSMP header
#define WAVE_ID_DATARATE                      16    // WSMP header
#define WAVE_ID_REPEAT_RATE                   17    // WSA header
#define WAVE_ID_RCPI_THRESHOLD                19    // WSA service info
#define WAVE_ID_WSA_COUNT_THRESHOLD           20    // WSA service info
#define WAVE_ID_CHANNEL_ACCESS                21    // WSA channel info
#define WAVE_ID_WSA_COUNT_THRESHOLD_INTERVAL  22    // WSA service info
#define WAVE_ID_WAVE_SHORT_MSG                128   // WSMP Header usage
#define WAVE_ID_WSMP_I                        130   // WSMP Header usage
#define WAVE_ID_WSMP_RESERVED                 255   // WSMP MAX

/* For PSIDs */
#define PSID_ENCODE_MASK_1BYTE    0x0     //PSID 1 bytes   (bit 7) 0 (bit 6) 0 (bit 5) 0 (bit 4)0
#define PSID_ENCODE_MASK_2BYTE    0x80    //PSID 2 bytes   (bit 7) 1 (bit 6) 0 (bit 5) x (bit 4)x
#define PSID_ENCODE_MASK_3BYTE    0xC0    //PSID 3 bytes   (bit 7) 1 (bit 6) 1 (bit 5) 0 (bit 4)x
#define PSID_ENCODE_MASK_4BYTE    0xE0    //PSID 4 bytes   (bit 7) 1 (bit 6) 1 (bit 5) 1 (bit 4)0
#define PSID_1BYTE_MAX_VALUE      0x7F    // max value for 1 byte encoding  (0  encoding)
#define PSID_2BYTE_MIN_VALUE      0x8000  // min value for 2 byte encoding  (10 encoding)
#define PSID_2BYTE_MAX_VALUE      0xBFFF  // max value for 2 byte encoding  (10 encoding)
#define PSID_3BYTE_MIN_VALUE      0xC00000 // min value for 3 byte encoding (110 encoding)
#define PSID_3BYTE_MAX_VALUE      0xDFFFFF // max value for 3 byte encoding (110 encoding)
#define PSID_4BYTE_MIN_VALUE      0xE0000000 // min value for 4 byte encoding (1110 encoding)
#define PSID_4BYTE_MAX_VALUE      0xEFFFFFFF // max value for 4 byte encoding (1110 encoding)

/* Family IDs */
#define FAMILY_ID_IEEE 1
#define FAMILY_ID_ISO  2
#define FAMILY_ID_ETSI 3

/* Ethertypes */
#define ETH_P_WSMP 0x88DC  /* WSM Protocol EtherType */
#define ETH_P_EU   0x8947  /* EU Protocol EtherType */

/* WSA defines */
#ifndef PREFIX_LENGTH
#define PREFIX_LENGTH    64
#endif
#define LOW_BYTE(x)        ((x) & 0x00FF)
#define HIGH_BYTE(x)       ((x >> 8) & 0x00FF)
#define HIGH_LOW_BYTE(x)   ((x >> 16) & 0x00FF)
#define HIGH_HIGH_BYTE(x)  ((x >> 24) & 0x00FF)

/* Radio services test command types */
typedef enum {
    RS_TEST_CMD_CERT_CHANGE_TIMEOUT,
    RS_TEST_CMD_UNSOLICITED_CERT_CHANGE,
    RS_TEST_CMD_IGNORE_WSAS,
} radioServicesTestCommandType;

/* Device Id definitions. Determines which bank of DSRC radios to use. For the
 * Hercules platform, you should always use DEV_ID0.
 */
enum {
    DEV_ID0 = 0,
    MAX_DEV_IDS
};

typedef struct {
    rsIoctlType cmd; /* WSU_NS_INIT_RPS or WSU_NS_RELEASE_RPS */
    bool_t      serviceAvailableInd; // TRUE if want SERVICE_AVAILABLE indications
    bool_t      receiveWSMInd;       // TRUE if want receive WSM indications
    pid_t       pid;                 // PID of registering application
} rsRPSMsgType;

typedef struct {
    rsIoctlType cmd; /* WSU_NS_GET_IND_PORT */
    pid_t       pid;
} rsGetIndPortMsgType;

typedef struct {
    rsResultCodeType risRet;
    uint16_t port;
} rsGetIndPortReplyType;

/* Indication message from Radio Services */
typedef enum {
    IND_WSM,
    IND_SERVICE_AVAILABLE,
    IND_SIGN_WSM_RESULTS,
    IND_SIGN_WSA_RESULTS,
    IND_ADD_UNSEC_1609P2_RESULTS,
    IND_VERIFY_WSA_RESULTS,
    IND_CERT_CHANGE_RESULTS,
} indicationType;

typedef struct {
    indicationType indType;
    struct ethhdr eth_hdr;

    union {
        inWSMType       wsm;                      /* if indType == IND_WSM */
        serviceInfoType serviceInfo;              /* if indType == IND_SERVICE_AVAILABLE */
        signResultsType signResults;              /* if indType == IND_SIGN_RESULTS
                                                   * or IND_ADD_UNSEC_1609P2_RESULTS
                                                   * or IND_ADD_DUMMY_SECURITY_RESULTS */
        uint32_t        returnCode;               /* if indType == IND_VERIFY_WSA_RESULTS */
        smiCertChangeResults_t certChangeResults; /* if indType == IND_CERT_CHANGE_RESULTS */
    } u;
} rsReceiveDataType;

/* Internal radio stack structure containing various parameters needed for
 * transmitting */
typedef struct {
    uint8_t     wsmpVersion;           /* Must be WSM_VERSION_3 */
    uint8_t     channelNumber;         /* Used by dsrc_tx(); The channel number
                                        * to transmit on. Must match either the
                                        * configured service channel number or
                                        * control channel number. */
    uint8_t     dataRate;              /* Used by dsrc_tx(); Tx data rate. */
    uint8_t     txPwrLevel;            /* Used by dsrc_tx(); Tx power level
                                        * (-128..127). */
    uint8_t     userPriority;          /* Used by dsrc_tx(); User priority
                                        * (0..7). */
    uint32_t    expireTime;            /* Don't think this is currently used. */
    rsRadioType radioType;             /* RT_CV2X or RT_DSRC */
    uint8_t     radioNum;              /* Used by dsrc_tx(); The radio number to
                                        * transmit on. */
    uint8_t     channelInterval;       /* Used by dsrc_tx();
                                        * 0 = Determine which interval to use
                                        *     based on the channelNumber field,
                                        *     and the configured control channel
                                        *     and service channel numbers.
                                        * 1 = Control Channel interval.
                                        * 2 = Service Channel interval. */
    uint8_t     security;              /* SECURITY_UNSECURED, SECURITY_SIGNED,
                                        * SECURITY_ENCRYPTED, SECURITY_CERT,
                                        * SECURITY_TESLA_KEY, SECURITY_TESLA_CERT,
                                        * or SECURITY_TWO_SIG_SIGNED. */
    uint32_t    securityFlag;          /* 0 = Use certificate or digest;
                                        * 1 = Always use full certificate. */
    uint8_t     destMac[LENGTH_MAC];   /* Used by dsrc_tx(); Destination MAC
                                        * address. */
    uint32_t    wsmpHeaderExt;         /* Used by createWSMPHeaderV3(); Bitmask
                                        * to determine what extension fields are
                                        * present. Bit names:
                                        * WAVE_ID_TRANSMIT_POWER_BITMASK,
                                        * WAVE_ID_CHANNEL_NUMBER_BITMASK, and
                                        * WAVE_ID_DATARATE_BITMASK. */
    uint32_t    psid;                  /* PSID */
    uint16_t    eth_proto;             /* Used by dsrc_tx(); ETH_P_WSMP for WSMP;
                                        * ETH_P_EU for EU. */
    uint8_t     srcMac[LENGTH_MAC];    /* Source MAC address. */
    bool_t      isBitmappedSsp;        /* TRUE if bitmapped SSP */
    uint8_t     sspLen;                /* Length of the SSP data */
    uint8_t     sspMask[MAX_SSP_DATA]; /* Used if bitmasked SSP */
    uint8_t     ssp[MAX_SSP_DATA];     /* SSP data */
} sendWSMStruct_S;

/* WSMP extension element type */
typedef struct {
    uint8_t WsmElementId;   // WSM element Id, WSM extension field only support
                            // Transmit power: 4, Channel Number: 15, Data Rate: 16
    uint8_t WsmDataLength;  // WSM Data Length
    uint8_t WsmData;        // WSM element value
} __attribute__((__packed__)) WsmExtType;

typedef struct {
    rsIoctlType cmd; /* WSU_NS_SECURITY_VERIFY_WSM_REQ */
    uint32_t    vodMsgSeqNum;
} rsVodMsgType;

typedef struct {
    rsResultCodeType risRet;
    AEROLINK_RESULT  vodResult;
} rsVodReplyType;

typedef struct {
    rsIoctlType cmd; /* WSU_NS_GET_WSA_INFO or WSU_NS_GET_WSA_WRA_INFO */
    uint8_t     wsa_num;
} rsGetWsaInfoMsgType;

typedef struct {
    rsIoctlType cmd; /* WSU_NS_GET_WSA_SERVICE_INFO or
                      * WSU_NS_GET_WSA_CHANNEL_INFO */
    uint8_t     wsa_num;
    uint8_t     element_num; /* Service info number or channel info number */
} rsGetWsaInfoElementMsgType;

typedef struct {
    rsIoctlType                  cmd; /* WSU_NS_TEST_CMD */
    radioServicesTestCommandType testCmd;

    union {
        int16_t count;
        int16_t seconds;
    } u;
} rsTestCmdType;

typedef struct {
    rsIoctlType cmd; /* WSU_NS_GET_RADIO_TALLIES */
    rsRadioType radioType;
    uint8_t     radioNum;
} rsGetRadioTalliesCmdType;

#endif // _RS_H

