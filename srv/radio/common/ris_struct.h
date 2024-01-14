/*----------------------------------------------------------------------------*/
/* Filename: srv/radio/common/ris_struct.h                                    */
/*                                                                           */
/* Copyright (C) 2019 DENSO International America, Inc.                      */
/*                                                                           */
/* Description: Include file to define the RIS/Radio Stack interface         */
/*           Contains the type definitions and structure definitions         */
/*                                                                           */
/*---------------------------------------------------------------------------*/
#ifndef _RIS_STRUCT_H
#define _RIS_STRUCT_H


#ifdef PLATFORM_QNX
/*
 * QNX Networking based on NetBSD network stack (not linux)
 *
 * TODO: Both Cohda's LLC API, and DENSO's Radio Stack code were written
 *       for Linux and ported to QNX.  The QNX Networking Stack is based
 *       on the BSD Network stack, and uses BSD include files, which are
 *       significantly different from those used in Linux.
 *
 *       Both the Cohda LLC and the DENSO DSRC RadioStack conssistantly
 *       use the Linux defintion of the IEEE 802.3 Ethernet Frame Header
 *       found under Linux in linux/if_ether.h.
 */
#include "lnx_ether.h"         /*Linux definition of the 802.3 Ethernet Header*/
#include <sys/time.h>          /*For defn of timeval (Unix98)*/

#else
#include <linux/if_ether.h>
#endif
#include <sys/time.h>       // struct timeval, struct timespec
#include <unistd.h>         // pid_t

#include "dn_types.h"

/*----------------------------------------------------------------------------*/
/* Constant Definitions                                                       */
/*----------------------------------------------------------------------------*/
#ifndef ETH_P_EU
#define ETH_P_EU        0x8947  /* EU Protocol EtherType */
/* NOTE: ETH_P_* are defined in rs.h (ETH_P_EU only repeated here since needed by RIS client.)*/
#endif

#define LENGTH_PSC             31         // Maximum PSC Length
#define LENGTH_MAC             ETH_ALEN   // MAC Address Length (linux/if_ether.h)
#define LENGTH_BSSID           LENGTH_MAC // BSSID length
#define LENGTH_IPV6            16         // IPv6 Address Length
#define LENGTH_ADVERTISE       32         // Maximum Advertise Length
#define MAX_WSM_DATA           2302       // Maximum WSM Data Length
#define MAX_WSM_DATA_LEGACY    1400       // Maximum WSM Data Length (legacy)
#define MAX_WSA_DATA           2302       // Maximum WSA Data Length (only used by WME VSCStatusTableType)
#define MAX_EU_PACKET_DATA     2302       // Maximum EU packet Data Length
#define MAX_SSP_DATA           32         // Maximum SSP Data Length

#ifdef CHINA_FREQS_ONLY
#define CCH_NUM_DEFAULT        163        // Default Control Channel Number
#define SCH_NUM_DEFAULT        157        // Default Service Channel Number
#else
#define CCH_NUM_DEFAULT        178        // Default Control Channel Number
#define SCH_NUM_DEFAULT        184        // Default Service Channel Number
#endif

#define WSM_VERSION_MASK       0x07

/* WSM V3 defines */
#define WSM_VERSION_3                               3 // WSM Version
#define WSM_WSMP_N_EXTENSIONS                       0x08
#define WSM_SUBTYPE_MASK                            0xF0
#define WSM_SUBTYPE_NULL_NETWORKING_PROTOCOL        0x00
#define WSM_SUBTYPE_ITS_STATION_INTERNAL_FORWARDING 0x10
#define WSM_SUBTYPE_N_HOP_FORWARDING                0x20
#define WSM_SUBTYPE_GEONETWORKING                   0x30

#define WSA_UNSECURED_TYPE                          0x80 // 1609.2 unsigned WSA
#define WSA_SECURED_TYPE                            0x81 // 1609.2 signed WSA
#define WSA_UNKNOWN_SECURED_TYPE                    3    // some other value
#define VERSION_1609p2                              0x03 // 1609.2 version
#define UNSECURED_TYPE_1609p2                       0x80 // 1609.2 unsigned WSM
#define SECURED_TYPE_1609p2                         0x81 // 1609.2 signed WSM

#define WSM_WSA_PSID                                0x8007
#define WSM_WSA_PSID_HI_BYTE                        0x80
#define WSM_WSA_PSID_LO_BYTE                        0x07

#define WSM_P2P_PSID                                0x8008
#define WSM_P2P_PSID_HI_BYTE                        0x80
#define WSM_P2P_PSID_LO_BYTE                        0x08

#define TPID_PSID                                   0x00
#define TPID_EXTENSION_ELEMENTS_PRESENT             0x01
#define TPID_MASK                                   0xFE
#define TPID_ITS                                    0x02
#define TPID_ITS_LPP                                0x04

#define WSA_16093_PROTOCOL_VER_MASK                 0xF0
#define WSA_16092_PROTOCOL_VER_3                    3 // 1609.2 version 3
#define WSA_16093_PROTOCOL_VER_3                    0x30 // 3 in bits 4 to 7; WSA version 3

#define WSA_CHANNEL_ACCESS_CONTINUOUS               0
#define WSA_CHANNEL_ACCESS_SLOT_0_ONLY              1
#define WSA_CHANNEL_ACCESS_SLOT_1_ONLY              2

#define WSA_CHANNEL_INFO_OPERATING_CLASS            17 // 80211p define 10MHZ channel class

#define MIN_SRVC_PRIORITY      0     // Minimum Service Priority
#define MAX_SRVC_PRIORITY      63    // Maximum Service Priority
#define MIN_REPEATS            0     // Minimum Repeat Count
#define MAX_REPEATS            7     // Maximum Repeat Count
#define MIN_TX_PWR_LEVEL       0     // Minimum Tx Power Level

/* AMR changed from 20 TODO find where this is used and change it dynamically */
#define MAX_TX_PWR_LEVEL       23    // Maximum Tx Power Level (21 for WSU1.x, 23 for WSU5900A) [MK5-SPECIFIC!]

#define SRVC_INACTIVITY_INTRVL 60000 // Default Service Inactivity Interval (ms)
#define WSA_TIMEOUT_INTRVL     2000  // Default WSA Timeout Interval (ms)
#define WSA_TIMEOUT_INTRVL2    2000  // Default WSA Timeout Interval (ms)
                                     // Used when >1 RSE in range
#define RECEIVER_DISABLE       0
#define RECEIVER_ENABLE        1
#define WSA_ADVERTISE_ID_LEN   32    // Maximum WSA header advertiser ID length
#define WSA_RESERVED3_LEN      3     // (Was WSA header country string length)
#define DEFAULT_GATEWAY_LEN    16    // WSA routing info default gateway length
#define DNS_LEN                16    // WSA routing info DNS length
#define MAX_EDCA_TXQ            8    // 8 TX queues with EDCA values (4 SCH + 4 CCH)

/*
 * Note:  Below are more more constant definitions that are intrinsically
 *        connected to the structures they are defined with.  To the extent
 *        possible, the radio stack code is being refactored to remove
 *        dependence on these defines which specify structure offsets of
 *        specific fields.
 */

/*----------------------------------------------------------------------------*/
/* Type Declarations                                                          */
/*----------------------------------------------------------------------------*/

/* Type definition of radio type */
/* Note: The various RT_* values are used as indexes into arrays, so they must
 * start at 0 and be numbered consecutively. MAX_RADIO_TYPES must be the number
 * of possible radio types, since it is used as the size of the arrays. */
typedef enum {
    RT_CV2X         = 0, // C-V2X
    RT_DSRC         = 1, // DSRC
    MAX_RADIO_TYPES = 2, // Number of possible rsRadioType values
} rsRadioType;

/* The number of radios of each type */
#define MAX_RADIOS 2

/* Type definition of Result Codes, returned by the APIs */
typedef enum {
    RS_SUCCESS      = 0,        // Success
    RS_ETHREAD      = (-1),     // Thread error
    RS_EPROCFS      = (-2),     // proc file system error
    RS_EIOCTL       = (-3),     // ioctl call error
    RS_ESOCKOPER    = (-4),     // Socket operation error
    RS_ESOCKCRE     = (-5),     // Socket creation error
    RS_ESOCKSEND    = (-6),     // Socket send error
    RS_ESOCKRCV     = (-7),     // Socket receive error
    RS_ENOSUPPORT   = (-8),     // No support
    RS_ENOCFM       = (-9),     // Received message is not a Confirmation
    RS_ERANGE       = (-10),    // Out of range error
    RS_EINVID       = (-11),    // Invalid ID
    RS_ENOROOM      = (-12),    // No room
    RS_EWRNGMODE    = (-13),    // Wrong Mode
    RS_EWRNGROLE    = (-14),    // Wrong Station Role
    RS_EWRNGSTATE   = (-15),    // Wrong Application State
    RS_ENOPID       = (-16),    // Application (PID) not registered
    RS_EDUPPID      = (-17),    // Duplicate Application (PID)
    RS_ENOSRVC      = (-18),    // Service (PSID) not registered
    RS_EDUPSRVC     = (-19),    // Duplicate Service (PSID)
    RS_ENOWBSS      = (-20),    // No active WBSS
    RS_EWBSS        = (-21),    // WBSS error
    RS_EWRNGVER     = (-22),    // Wrong Version
    RS_EMATCH       = (-23),    // Match error
    RS_EMIB         = (-24),    // MIB error
    RS_ELICENSE     = (-25),    // Error with WSU license file
    RS_ESECPTR      = (-26),    // Security function ptr not initialized
    RS_ESECLIC      = (-27),    // WSU license has security disabled
    RS_ESECNENABLED = (-28),    // Security not enabled
    RS_SECURITY_LENGTH_TOO_LONG = (-29),
                                // Security length is too long
    RS_ESECOBENC    = (-30),    // Security OBE not connected
    RS_ESECSMIRTSSF = (-31),    // Security SMI Ready To Send still false
    RS_ESECSBF      = (-32),    // Security sign buffer full
    RS_ESECIWTSBF   = (-33),    // Security insert WSM to sign buffer failed
    RS_ESECFTRWFSF  = (-35),    // Security Failed to remove WSM from sign FIFO
    RS_ESECSMDSDF   = (-36),    // Security SMI send data failed
    RS_EDEPRECATED  = (-37),    // RIS API command is obsolete and has been deprecated
    RS_EINTERN      = (-38),    // RadioStack Internal error
    RS_EINVACT      = (-39),    // Invalid action
    RS_ECV2XRAD     = (-40),    // C-V2X radio error
    RS_EWSMPHDRERR  = (-41),    // WSMP header error
    RS_UNKNTPID     = (-42),    // Unknown TPID
    RS_PSIDENCERR   = (-43),    // PSID encoding error
    RS_NOTENABLED   = (-44),    // Radio not enabled
    RS_ESECADDF     = (-45),    // Security add dummy digest failed
    RS_ESECADCF     = (-46),    // Security add dummy certificate failed
    RS_ESECSIGNF    = (-47),    // Security add signature failed
    RS_ESECA16092F  = (-48),    // Security add unsecured 1609.2 header failed
    RS_ESECINVSEQN  = (-49),    // Security invalid VOD message sequence number
    RS_ESECCSMP     = (-50),    // Security create SMP error
    RS_ESECINITF    = (-51),    // Security initialize failure
    RS_ESECTERMF    = (-52),    // Security terminate failure
    RS_ESECCERTCHF  = (-53),    // Security certificate change failure
    RS_EGETWSAINFO  = (-54),    // Get WSA Info Error
    RS_ECHNLCONFL   = (-55),    // Channel conflicts
    RS_EDSRCRAD     = (-56),    // DSRC radio error
    RS_ENOINIT      = (-57),    // Radio not initialized
    RS_ESECUPKTT    = (-58),    // Security unknown packet type
    RS_ESECINVPSID  = (-59),    // Security invalid PSID
    RS_ESECCERTCHGIP = (-60),   // Security certificate change in progress
    RS_EVERWSAFAIL   = (-61),   // Verify WSA failed
    RS_ELOCKFAIL     = (-62),   // Threads collision for mutex
    RS_BADPARAM      = (-63),   // Bad input
    RS_PCAPOVRFLOW   = (-64),   // Attempt to read beyond size of pcap buffer
    RS_ECCL         = (-100),   // CCL reported errors start here
    /* The following are rsResultCodeType codes corresponding to tMKxStatus values.
     * RS_SUCCESS is used as the equivalent for MKXSTATUS_SUCCESS.
     * RS_MKXSTAT_UNKNOWN indicates an unknown tMKxStatus value. */
    RS_MKXSTAT_FAIL_INTERN_ERR        = (-200),
    RS_MKXSTAT_FAIL_INV_HANDLE        = (-201),
    RS_MKXSTAT_FAIL_INV_CFG           = (-202),
    RS_MKXSTAT_FAIL_INV_PARAM         = (-203),
    RS_MKXSTAT_FAIL_AUTOCAL_REJ_SIMUL = (-204),
    RS_MKXSTAT_FAIL_AUTOCAL_REJ_UNCFG = (-205),
    RS_MKXSTAT_FAIL_RADIOCFG_MAX      = (-206),
    RS_MKXSTAT_FAIL_RADIOCFG_MIN      = (-207),
    RS_MKXSTAT_TX_FAIL_TTL            = (-208),
    RS_MKXSTAT_TX_FAIL_RETR           = (-209),
    RS_MKXSTAT_TX_FAIL_QFULL          = (-210),
    RS_MKXSTAT_TX_FAIL_RADIO_NOT_PRES = (-211),
    RS_MKXSTAT_TX_FAIL_MALFORMED      = (-212),
    RS_MKXSTAT_TX_FAIL_RADIO_UNCFG    = (-213),
    RS_MKXSTAT_TX_FAIL_PKT_TOO_LONG   = (-214),
    RS_MKXSTAT_SEC_ACC_NOT_PRES       = (-215),
    RS_MKXSTAT_SEC_FIFO_FULL          = (-216),
    RS_MKXSTAT_RESERVED               = (-217),
    RS_MKXSTAT_UNKNOWN                = (-218),
    /* AT return values. */
    RS_AT_CV2X_NOT_INIT               = (-230),
    RS_AT_CV2X_TX_NOT_READY           = (-231),
    RS_AT_CV2X_BAD_SERVICE_ID         = (-232),
    RS_AT_CV2X_SEND_FAIL              = (-233),
} rsResultCodeType;

/* Messages to Radio Services and their replies */
typedef enum {
    WSU_NS_INIT_RPS,
    WSU_NS_RELEASE_RPS,
    WSU_NS_USER_SERVICE_REQ,
    WSU_NS_WSM_SERVICE_REQ,
    WSU_NS_SEND_WSM_REQ,
    WSU_NS_GET_WBSS_STS,
    WSU_NS_GET_IND_PORT,
    WSU_NS_GET_NS_CFG,
    WSU_NS_SET_NS_CFG,
    WSU_NS_GET_CFG_INFO,
    WSU_NS_SECURITY_VERIFY_WSM_REQ,
    WSU_NS_SECURITY_VERIFY_WSA_REQ,
    WSU_NS_SECURITY_INITIALIZE,
    WSU_NS_SECURITY_TERMINATE,
    WSU_NS_SECURITY_UPDATE_POSITION_AND_TIME,
    WSU_NS_SECURITY_GET_LEAP_SECONDS_SINCE_2004,
    WSU_NS_SECURITY_SEND_CERT_CHANGE_REQ,
    WSU_NS_GET_AEROLINK_VERSION,
    WSU_NS_GET_WSA_INFO,
    WSU_NS_GET_WSA_SERVICE_INFO,
    WSU_NS_GET_WSA_CHANNEL_INFO,
    WSU_NS_GET_WSA_WRA_INFO,
    WSU_NS_USER_CHANGE_SERVICE_REQ,
    WSU_NS_TEST_CMD,
    WSU_NS_SEND_RAW_PACKET_REQ,
    WSU_NS_GET_RADIO_TALLIES,
    WSU_NS_SET_RADIO_CFG,
    WSU_NS_PCAP_ENABLE,
    WSU_NS_PCAP_DISABLE,
    WSU_NS_PCAP_READ,
    /* JJG added here because only a higher power knows what will break if injecting
       above; not testing the entire radio stack in the remainder of this week */
    WSU_NS_PROVIDER_SERVICE_REQ,
    WSU_NS_GET_RSK_STATUS, /* Interface, firware revision, temperature, ready, but not gnss status. */
} rsIoctlType;

/* Type definition of User Access Services Type */
typedef enum {
    AUTO_ACCESS_MATCH,          // Provide SCH access upon PSID match in received WSA
    AUTO_ACCESS_UNCONDITIONAL,  // Provide SCH access immediately (WSA not required)
    NO_SCH_ACCESS,              // Match PSID in received WSA, but don't access SCH
} rsUserRequestType;

/* Type definition of WSA Access Services Type */
typedef enum {
    UNSECURED_WSA               = 0,
    SECURED_WSA                 = 1,
    ANY_WSA                     = 2
} rsWsaSecurityType;

/* WSA Status type info */
typedef enum {
    RS_SERVICE_AVAILABLE        = 0,    // service available
    RS_SERVICE_NOT_AVAILABLE    = 1,    // service not available
} rsServiceAvailableInfoType;

/* WSA SCH Channel assigned type info */
typedef enum {
    RS_SCH_ASSIGNED             = 0,    // service channel assigned
    RS_SCH_NOT_ASSIGNED         = 1,    // service channel not assigned
} rsSCHAssignInfoType;

/* Type definition of Station Roles */
typedef enum {
    ROLE_PROVIDER,  // Provider
    ROLE_USER   // User
} roleType;

/* Type definition of Configuration Item Identifiers.
 * Note that CFG_ID_ANTENNA, CFG_ID_ANTENNA_RADIO_0, CFG_ID_ANTENNA_RADIO_1,
 * CFG_ID_ANTENNA_TXOVERRIDE_RADIO_0, CFG_ID_ANTENNA_TXOVERRIDE_RADIO_1,
 * CFG_ID_MACADDRESS_RADIO_0, CFG_ID_MACADDRESS_RADIO_1, CFG_ID_EDCA_PARAMS,
 * CFG_ID_TX_MAX_POWER_LEVEL, CFG_ID_TX_MAX_POWER_LEVEL_RADIO_0,
 * CFG_ID_TX_MAX_POWER_LEVEL_RADIO_1, CFG_ID_MACADDRESS_ORIG_RADIO_0, and
 * CFG_ID_MACADDRESS_ORIG_RADIO_1 are applicable only for the DSRC radio. If
 * support for the C-V2X radio is to be added in the future, additional enums
 * will be defined. */
typedef enum {
    CFG_ID_STATION_ROLE,            // Station Role
    CFG_ID_REPEATS,                 // WSA Repeat count every CCH interval
    CFG_ID_CCH_NUM_RADIO_0,         // Control Channel (CCH) Number
    CFG_ID_CCH_NUM_RADIO_1,         // Control Channel (CCH) Number
    CFG_ID_SCH_NUM_RADIO_0,         // Service Channel (SCH) Number
    CFG_ID_SCH_NUM_RADIO_1,         // Service Channel (SCH) Number
    CFG_ID_SCH_ADAPTABLE,           // Adaptable SCH Data Rate / Tx Power Level (only 0 is supported)
    CFG_ID_SCH_DATARATE,            // SCH Data Rate
    CFG_ID_SCH_TXPWRLEVEL,          // SCH Tx Power Level
    CFG_ID_SERVICE_INACTIVITY_INTERVAL, // Service Inactivity Interval
    CFG_ID_WSA_TIMEOUT_INTERVAL,        // WSA Timeout Interval
    CFG_ID_WSA_TIMEOUT_INTERVAL2,       // WSA Timeout Interval 2
                                        // Used when >1 RSE in range
    CFG_ID_ANTENNA,                     // Alias for CFG_ID_ANTENNA_RADIO_0
    CFG_ID_ANTENNA_RADIO_0,             // Tx+RX+Diversity Antenna attributes for Radio 0 [MK5: new functionality]
    CFG_ID_ANTENNA_RADIO_1,             // Tx+RX+Diversity Antenna attributes for Radio 1 [MK5: new functionality]

    CFG_ID_ANTENNA_TXOVERRIDE_RADIO_0,      // Tx-only` Antenna temporary override (for limitied use) Radio 0 [MK5: new]
    CFG_ID_ANTENNA_TXOVERRIDE_RADIO_1,      // Tx-only` Antenna temporary override (for limitied use) Radio 1 [MK5: new]

    CFG_ID_MACADDRESS_RADIO_0,          // Mac address Id for Radio 0
    CFG_ID_MACADDRESS_RADIO_1,          // Mac address Id for Radio 1
    CFG_ID_MACADDRESS_ORIG_RADIO_0,     // Original Mac address Id for Radio 0
    CFG_ID_MACADDRESS_ORIG_RADIO_1,     // Original Mac address Id for Radio 1

    CFG_ID_RANDOM_MAC_COUNT,            // CCH random MAC address counter
    CFG_ID_CCH_BSSID_RADIO_0,           // CCH BSSID address Id for Radio 0
    CFG_ID_CCH_BSSID_RADIO_1,           // CCH BSSID address Id for Radio 1
    CFG_ID_NODEQOS_RADIO_0,             // NODEQOS for Radio 0
    CFG_ID_NODEQOS_RADIO_1,             // NODEQOS for Radio 1
    CFG_ID_DUAL_RADIO_ECHO_ENABLE,      // Enable/Disable Tx pkt echo from one radio to another
    CFG_ID_ALLOW_DUAL_WAVE_CONFIG,      // Allow both radios to be configured to WAVE mode
    CFG_ID_RECEIVER_MODE_RADIO_0,       // Enable/Disable the radio 0 receiver
    CFG_ID_RECEIVER_MODE_RADIO_1,       // Enable/Disable the radio 1 receiver
    CFG_ID_EDCA_PARAMS,                 // Setting EDCA parameters in EDCA categories
    CFG_ID_TX_MAX_POWER_LEVEL,
    CFG_ID_TX_MAX_POWER_LEVEL_RADIO_0,
    CFG_ID_TX_MAX_POWER_LEVEL_RADIO_1,
    CFG_ID_RESET_RADIO_0,
    CFG_ID_RESET_RADIO_1,
    CFG_ID_RX_QUEUE_THRESHOLD,          // Max total packets to queue before dropping
    CFG_ID_WSA_CHANNEL_INTERVAL,        // WSA Channel Interval (0=Auto 1=CCH 2=SCH)
    CFG_ID_WSA_PRIORITY,                // WSA Priority (0-7)
    CFG_ID_WSA_DATARATE,                // WSA Tx Data Rate
    CFG_ID_WSA_TXPWRLEVEL,              // WSA Tx Power Level (0-23)
    CFG_ID_P2P_CHANNEL_INTERVAL,        // P2P Channel Interval (0=Auto 1=CCH 2=SCH)
    CFG_ID_P2P_PRIORITY,                // P2P Priority (0-7)
    CFG_ID_P2P_DATARATE,                // P2P Tx Data Rate
    CFG_ID_P2P_TXPWRLEVEL,              // P2P Tx Power Level (0-23)
    CFG_ID_IPV6_PRIORITY,               // IPv6 Priority (0-7)
    CFG_ID_IPV6_DATARATE,               // IPv6 Tx Data Rate
    CFG_ID_IPV6_TXPWRLEVEL,             // IPv6 Tx Power Level (0-23)
    CFG_ID_IPV6_ADAPTABLE,              // Adaptable SCH Data Rate / Tx Power Level (only 0 is supported)
    CFG_ID_IPV6_PROVIDER_MODE,          // IPv6 Provider Mode (BOOLEAN)
    CFG_ID_IPV6_RADIO_TYPE,             // IPv6 Radio Type (RT_CV2X or RT_DSRC)
    CFG_ID_IPV6_RADIO_NUM,              // IPv6 Radio Number (0-1)
    CFG_ID_CONDUCTED_2_RADIATED_POWER_OFFSET_RADIO_0,
                                        // Conducted to radiated power offset (-20.0..20.0) radio 0
    CFG_ID_CONDUCTED_2_RADIATED_POWER_OFFSET_RADIO_1,
                                        // Conducted to radiated power offset (-20.0..20.0) radio 1
    CFG_ID_CHANGE_MAC_ADDRESS_ENABLE,   // Enable changing of the MAC address (0-1)
    CFG_ID_RCV_EU_FROM_ETH_ENABLE,      // Enable receiving EU packets from the Ethernet port (0-1)
    CFG_ID_RCV_EU_FROM_ETH_PORT_NUM,    // Port number to receive EU packets from the Ethernet on (0-65535)
    CFG_ID_CV2X_ENABLE,                 // Enable the C-V2X radio
    CFG_ID_DSRC_ENABLE,                 // Enable the DSRC radio
    CFG_ID_SSP,                         // SSP to include in signature
    CFG_ID_CV2X_PWR_CTRL_ENABLE,        // C-V2X power control enable
    CFG_ID_DISCARD_UNSECURE_MSG,        // Discard WSMs that are unsecured
    CFG_ID_DISCARD_VERFAIL_MSG,         // Discard WSMs that fail security verification
    CFG_ID_PRIMARY_SERVICE_ID,          // Primary Service ID used for C-V2X Tx
    CFG_ID_SECONDARY_SERVICE_ID,        // Secondary Service ID used for C-V2X Tx
    CFG_ID_ALSMI_DEBUG_LOG_ENABLE,      // ALSMI debug log enable
    CFG_ID_ALSMI_SIGN_VERIFY_ENABLE,    // ALSMI sign & verify enable
    CFG_ID_I2V_SYSLOG_ENABLE,           // I2V Syslog enable
    MAX_CFG_ID
} cfgIdType;

/* Type definition of Provider Service Identifiers (PSID) (the actual values in this enum should not be used
   because they are out of date). */
typedef enum {
    PSID_SYSTEM                         = 0,
    PSID_AUTOMATIC_FEE_COLLECTION       = 1,
    PSID_FREIGHT_FLEET_MANAGEMENT       = 2,
    PSID_PUBLIC_TRANSPORT               = 3,
    PSID_TRAFFIC_TRAVELLER_INFORMATION  = 4,
    PSID_TRAFFIC_CONTROL                = 5,
    PSID_PARKING_MANAGEMENT             = 6,
    PSID_GEOGRAPHIC_ROAD_DATABASE       = 7,
    PSID_MEDIUM_RANGE_PREINFORMATION    = 8,
    PSID_MAN_MACHINE_INTERFACE          = 9,
    PSID_INTERSYSTEM_INTERFACE          = 10,
    PSID_AUTOMATIC_VEHICLE_IDENTIFICATION   = 11,
    PSID_EMERGENCY_WARNING              = 12,
    PSID_PRIVATE                        = 13,
    PSID_MULTI_PURPOSE_PAYMENT          = 14,
    PSID_DSRC_RESOURCE_MANAGER          = 15,
    PSID_AFTER_THEFT_SYSTEMS            = 16,
    PSID_CRUISE_ASSIST_HIGHWAY_SYSTEM   = 17,
    PSID_MULTI_PURPOSE_INFORMATION_SYSTEM   = 18,
    PSID_PUBLIC_SAFETY                  = 19,
    PSID_VEHICLE_SAFETY                 = 20,
    PSID_GENERAL_PURPOSE_INTERNET_ACCESS    = 21,
    PSID_ON_BOARD_DIAGNOSTICS           = 22,
    PSID_SECURITY_MANAGER               = 23,
    PSID_SIGNED_WSA                     = 24,

    PSID_MAX_VALUE                      = 0xFFFFFFFF
} PSIDType;

/* Type definition of channel interval type */
typedef enum {
    CHANTYPE_CCH                = 0,
    CHANTYPE_SCH                = 1,
    CHANTYPE_ALL                = 2
} CHANType;

/* Type definition of payload data for incoming and outgoing EU Packet messages */
typedef struct {
    uint16_t dataLength;               // EUPacket Data Length; 0..MAX_EU_PACKET_DATA
    uint8_t  data[MAX_EU_PACKET_DATA]; // EUPacket Data
} risEUPktType;


/* Type definition of Metadata for outgoing EU Packet messages */
typedef struct {
    struct ethhdr eth_hdr;               // Src/Dst MAC, EthType
    uint8_t       channelNumber;         // Tx Channel Number (TBD: Include to verify, or don't include since set by USR)
    uint8_t       dataRate;              // Tx Data Rate
    uint8_t       txPwrLevel;            // Tx Power Level (0-46, in 0.5 dB increments (0-23dB))
    uint8_t       priority;              // User priority (0 - 7)
    rsRadioType   radioType;             // RT_CV2X or RT_DSRC
    uint8_t       radioNum;              // Radio to transmit with (0-1)
    bool_t        needsSigning;          // TRUE=Sign packet before transmitting
    uint32_t      ITS_AID;               // ITS_AID (PSID) to sign the packet with
    bool_t        isBitmappedSsp;        // TRUE=Is bitmapped SSP
    uint8_t       sspLen;                // Length of SSP to sign the packet with
    uint8_t       sspMask[MAX_SSP_DATA]; // SSP Mask (if isBitmappedSsp is TRUE)
    uint8_t       ssp[MAX_SSP_DATA];     // SSP to sign the packet with
} risTxMetaEUPktType;

/* Type definition Metadata for incoming EU Packet messages */
typedef struct {
    struct ethhdr eth_hdr;              // Src/Dst MAC, EthType
    uint8_t       channelNumber;        // Channel Number (0xff = not available) (?)
    uint8_t       dataRate;             // Rx Data Rate
    uint8_t       priority;             // Received user priority (0=[0,3],1=[1,2],5=[4,5],6=[6,7], 0xff=not available) (?)
    int8_t        rss;                  // Receive Signal Strength (dBm) from Antenna actually used (calculated for Diversity Mode)
    int8_t        rss1;                 // Receive Signal Strength (dBm) from 1st Antenna
    int8_t        rss2;                 // Receive Signal Strength (dBm) from 2nd Antenna
    int8_t        noise1;               // Receive noise (dBm) at 1st antenna
    int8_t        noise2;               // Receive noise (dBm) at 2nd Antenna
    rsRadioType   radioType;            // RT_CV2X or RT_DSRC
    uint8_t       radioNum;             // Receive radio number (0-1)
    uint32_t      vodMsgSeqNum;         // Verify-on-Demand msg seq num, assigned by security and used by App to reference pkt.
                                        // A value of 0 indicates the Verify-on-Demand handle is invalid.
    uint32_t      offsetToCommonHeader; // Offset from the start of the packet to the start of the common header
    uint16_t      unsignedDataLength;   // Length of the unsigned portion of the data
                                        // starting with the common header and up to but not including the security footer
    uint32_t      securityFlags;        // unsecured, securedButNotVerified, securedVerifiedSuccess, or securedVerifiedFail
    uint8_t       sspLen;               // Length of received SSP data
    uint8_t       ssp[MAX_SSP_DATA];    // SSP data
    uint32_t      psid;                 // The PSID
} risRxMetaEUPktType;

/* Type definition of Provider Service Context (PSC) parameters */
typedef struct {
    uint8_t     PSCLength;      // PSC Length (0..LENGTH_PSC)
    uint8_t     PSC[LENGTH_PSC];    // PSC
} PSCType;

typedef struct {
    uint8_t  serviceAvail;              // Service available indicator (see rsServiceAvailableInfoType)
    uint8_t  channelAvail;              // SCH assigned indicator (see rsSCHAssignInfoType)
    int8_t   rss;                       // Received signal strength (dBm)
    uint8_t  servicePscLength;          // Provider service content length
    uint8_t  sourceMac[LENGTH_MAC];     // Source Mac address
    uint32_t serviceBitmask;            // Service info bitmask for optional fields
                                        // WSA_EXT_PROVIDER_SERVICE_CONTEXT_BITMASK,
                                        // WSA_EXT_IPV6ADDRESS_BITMASK,
                                        // WSA_EXT_SERVICE_PORT_BITMASK,
                                        // WSA_EXT_PROVIDER_MACADDRESS_BITMASK,
                                        // WSA_EXT_RCPI_THRESHOLD_BITMASK,
                                        // WSA_EXT_WSA_COUNT_THRESHOLD_BITMASK
    uint32_t psid;                      // Provider Service Identifier
    uint8_t  reserved1;                 // (Was Service priority)
    int8_t   channelIndex;              // WSA_ServiceInfo channel id
    uint8_t  psc[LENGTH_PSC];           // Provider service Content
    uint8_t  ipv6Address[LENGTH_IPV6];  // IPv6 Address (optional)
    uint16_t port;                      // IPv6 port (optional)
    uint8_t  providerMacAddress[LENGTH_MAC];
                                        // Provider Mac Address (optional)
    int8_t   rcpiThreshold;             // RCPI threshold (optional)
    uint8_t  wsaCountThreshold;         // WSA Count Threshold (optional)
    uint8_t  wsaCountThresholdInterval; // WSA Count Threshold Interval (optional)
    uint32_t channelBitmask;            // Channel info bitmask for optional fields
                                        // WSA_EXT_EDCA_PARAM_SET_BITMASK,
                                        // WSA_EXT_CHANNEL_ACCESS_BITMASK
    uint8_t  operatingClass;            // Channel info Operating Class
    uint8_t  channelNumber;             // Channel info channel number
    uint8_t  adaptable;                 // Channel info adaptable field
    uint8_t  dataRate;                  // Channel info data rate
    uint8_t  txPwrLevel;                // Channel info transmit power level
    uint8_t  qosInfo;                   // Channel info EDCA, QoS Info
    uint16_t txop[4];                   // Channel info EDCA, TxOp limit (b0-b15) (optional)
    uint8_t  ecw[4];                    // Channel info EDCA, ECWmin b0-b3, ECWmax b4-b7 (optional)
    uint8_t  aifs[4];                   // Channel info EDCA, Arbitration Interframe Spacing (optional)
    uint8_t  channelAccess;             // Channel info, channel access field (optional)
    uint32_t WSABitmask;                // WSA header bitmask for optional fields
    uint8_t  advertiseIdLength;         // Advertiser ID length
    uint8_t  changeCount;               // WSA header change count
    uint8_t  repeatRate;                // WSA header repeat rate
    uint8_t  reserved2;                 // (Was WSA header transmit power)
    int32_t  latitude2D;                // WSA header 2D Location, latitude
    int32_t  longitude2D;               // WSA header 2D Location, longitude
    int32_t  latitude3D;                // WSA header 3D Location and confidence, latitude
    int32_t  longitude3D;               // WSA header 3D Location and confidence, longitude
    int16_t  elevation3D;               // WSA header 3D Location and confidence, elevation
    // The following 4 fields are no longer used. However, they must be present
    // for compatibility with past SR files.
    uint8_t  confidence3D;              // WSA header 3D Location and confidence, confidence
    uint8_t  semiMajorAccuracy3D;       // WSA header 3D Positional accuracy, semi-major
    uint8_t  semiMinorAccuracy3D;       // WSA header 3D Positional accuracy, semi-minor
    uint16_t semiMajorOrientation3D;    // WSA header 3D Positional accuracy, semi-major orientation
    uint8_t  advertiseId[WSA_ADVERTISE_ID_LEN];
                                        // WSA header advertiser ID
    uint8_t  reserved3[WSA_RESERVED3_LEN];
                                        // (Was WSA header country string)
    uint32_t routingBitmask;            // Routing info bitmask for all fields
    uint16_t routerLifeTime;            // Routing info router life time
    uint8_t  ipPrefix[LENGTH_IPV6];     // Routing info IP Prefix
    uint8_t  prefixLen;                 // Routing info prefix length
    uint8_t  defaultGateway[DEFAULT_GATEWAY_LEN];
                                        // Routing info default gateway
    uint8_t  primaryDns[LENGTH_IPV6];   // Routing info primary DNS
    uint8_t  secondaryDns[LENGTH_IPV6]; // Routing info secondary DNS
    uint8_t  gatewayMacAddress[LENGTH_MAC];
                                        // Routing info gateway MAC address
    uint32_t signedWsaHandle;           // Handle to original signed WSA
} __attribute__((__packed__)) serviceInfoType;

/* Type definition of Security Options */
typedef enum {
    SECURITY_UNSECURED,         // Unsecured
    SECURITY_SIGNED,            // Signed
    SECURITY_ENCRYPTED,         // Encrypted.  Not Supported.
    SECURITY_CERT,              // Certificate
    SECURITY_TESLA_KEY,         // TESLA Key
    SECURITY_TESLA_CERT,        // TESLA Certificate
    SECURITY_TWO_SIG_SIGNED,    // Two Signatures Signed
    // Not Supported; SECURITY_SIGNED_ENCRYPTED, // Signed & Encrypted
    SECURITY_INVALID = 0xAAAAAAAA,
} securityType;

/* Type definition of Data Rate Options */
typedef enum {
    DR_3_MBPS   = 6,    // 3 Mbps
    DR_4_5_MBPS = 9,    // 4.5 Mbps
    DR_6_MBPS   = 12,   // 6 Mbps
    DR_9_MBPS   = 18,   // 9 Mbps
    DR_12_MBPS  = 24,   // 12 Mbps
    DR_18_MBPS  = 36,   // 18 Mbps
    DR_24_MBPS  = 48,   // 24 Mbps
    DR_27_MBPS  = 54,   // 27 Mbps
    DR_6_MBPS_20MHz     = 6,    // 6 Mbps (20 MHz channel)
    DR_9_MBPS_20MHz     = 9,    // 9 Mbps (20 MHz channel)
    DR_12_MBPS_20MHz    = 12,   // 12 Mbps (20 MHz channel)
    DR_18_MBPS_20MHz    = 18,   // 18 Mbps (20 MHz channel)
    DR_24_MBPS_20MHz    = 24,   // 24 Mbps (20 MHz channel)
    DR_36_MBPS_20MHz    = 36,   // 36 Mbps (20 MHz channel)
    DR_48_MBPS_20MHz    = 48,   // 48 Mbps (20 MHz channel)
    DR_54_MBPS_20MHz    = 54,   // 54 Mbps (20 MHz channel)
} dataRateType;

/* mapping between the ACI to access category (AC) */
typedef enum {
    AC_BE   = 0,        // Best effort category
    AC_BK   = 1,        // Background category
    AC_VI   = 2,        // Video in category
    AC_VO   = 3,        // Video out category
} rsEdcaACType;

/* EDCA parameter configuration structure */
typedef struct EDAC_VAL
{
    rsRadioType  radioType;           // RT_CV2X or RT_DSRC
    uint8_t      radioNum;            // radio number (0-1)
    rsEdcaACType accessCategory;      // Access Category (0-3)
    CHANType     chnlType;            // CHANTYPE_CCH or CHANTYPE_SCH
    uint32_t     ecw_min;             // CWmin Exponent (2-10)
    uint32_t     ecw_max;             // CWmax Exponent (2-10)
    uint32_t     aifs;                // Arbitration Interframe Spacing (AIFS) (1-15)
    uint32_t     txop;                // Transmit Opportunity (Txop) (normally 0)
} rsEdcaType;

/* Structure used to specify a CCL (LLC) transmit queue. */
typedef struct
{
    rsRadioType radioType;            /* RT_CV2X or RT_DSRC */
    uint8_t     radioNum;             /* Radio number (0-1) */
    uint8_t     chanType;             /* 0:CHANTYPE_CCH, 1:CHANTYPE_SCH, 2:CHANTYPE_ALL */
    uint8_t     txPriority;           /* Transmit user priority (0-7, 0xff=all) */
} cclTxQueueType;

typedef struct {
    uint8_t  status;        /* 0=Success (Flushed) */
} TxQFlushStatusType;


typedef struct {
    uint8_t  status;        /* 1=True (Q Full) */
} TxQFullStatusType;


/* Structure used to configure CCL transmit queue size and queue drop policy when full. */
typedef struct
{
    uint8_t     cmd;                 // Internal command field for ioctl (app can ignore)
    rsRadioType radioType;           // RT_CV2X or RT_DSRC
    uint8_t     radioNum;            // Radio number (0-1)
    uint8_t     chnlType;            // 0=CCH, 1=SCH, 2=ALL
    uint8_t     txPriority;          // Transmit user priority (0-7, 0xff=all)
    uint8_t     size;                // Transmit queue size (1-128)
    uint8_t     dropPolicy;          // Controls which item to drop when FIFO queue is full
                                     // (0=drop newest entry, 1=drop oldest entry)
} cclTxQueueSizeType;

/* Type definition of Configuration Item Values and their types & ranges.
 * Note that certain cfgId's are only applicable for the DSRC radio; a set
 * request will return RS_SUCCESS and a get request will return RS_NOTENABLED
 * for C-V2X. See the definition of cfgIdType for a list of cfgId's that are
 * DSRC-only. */
typedef struct {
    /* The following fields are automatically filled by RIS, and are not filled
     * out by the application:
     * vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv */
    rsIoctlType      cmd;    /* WSU_NS_GET_NS_CFG or WSU_NS_SET_NS_CFG */
    cfgIdType        cfgId;  /* For SET */
    rsResultCodeType risRet; /* For GET */
    /* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ */

    rsRadioType      radioType;          // RT_CV2X or RT_DSRC. Not relevant
                                         // for all cfgId values

    union {
        bool_t       CV2XEnable;         // C-V2X radio enable flag
        bool_t       DSRCEnable;         // DSRC radio enable flag
        roleType     StationRole;        // Station Role; Default - ROLE_USER
        uint8_t      Repeats;            // WSA Repeat count; 0..7; Default - MIN_REPEATS
        uint8_t      CCHNum;             // Control Channel Number
                                         // 172, 174, 176, 178, 180, 182, 184; Default - CCH_NUM_DEFAULT
        uint8_t      SCHNum;             // Service Channel Number
                                         // 172, 174, 176, 178, 180, 182, 184; Default - SCH_NUM_DEFAULT
        bool_t       Adaptable;          // Adaptable SCH Data Rate / Tx Power Level
                                         // Not Supported; Default - FALSE
        dataRateType DataRate;           // SCH Data Rate; Default - DR_3_MBPS
        uint8_t      TxPwrLevel;         // SCH Tx Power Level; 0..20 dBm; Default - MAX_TX_PWR_LEVEL
        uint8_t      Priority;           // User priority 0..7
        uint8_t      ChannelInterval;    // 0=Auto 1=CCH 2=SCH
        uint32_t     ServiceInactivityInterval;  // Service Inactivity Interval
                                         // 0..4,000,000 ms; Default - SRVC_INACTIVITY_INTRVL ms
        uint32_t     WSATimeoutInterval; // WSA Timeout Interval
                                         // 0..4,000,000 ms; Default - WSA_TIMEOUT_INTRVL ms
        uint32_t     WSATimeoutInterval2;// WSA Timeout Interval
                                         // 0..4,000,000 ms; Default - WSA_TIMEOUT_INTRVL2 ms
                                         // Used when >1 RSE in range
        uint8_t      RadioMacAddress[LENGTH_MAC];    // Mac address to be configured
        uint8_t      Antenna;            // 0 = Auto, 1 = Tx Antenna 1, 2 = Tx Antenna 2
        uint8_t      RxAntenna;          // 1 = Rx Antenna 1, 2 = Rx Antenna 2
        uint8_t      RxDiversity;        // 0 = Disable Rx diversity, 1 = Enable Rx diversity
        uint32_t     RxQueueThreshold;   // Rx queue threshold
        uint32_t     RandomMacCount;     // 0 - max value of 32 bit unsigned int
        uint8_t      CCHBssid[LENGTH_MAC];
                                         // CCH BSSID address to be configured
        uint8_t      NodeQos;            // 0 = Disable, 1 = Enable
        uint8_t      DualRadioEchoEnable;// 0 = Disable, 1 = Enable
        uint8_t      AllowDualWaveConfig;// 0 = Disable, 1 = Enable
        uint8_t      AdjustCCLTxTiming;  // Delay before transmitting 1st packet per channel interval
                                         // 0=none, 1=0-255 us (random), 2=0-4ms (random),
                                         // 3=timeshifter (all pkts)
        uint8_t      RecvMode;           // Disable Receiver: RECEIVER_DISABLE, Enable: RECEIVER_ENABLE
        rsEdcaType   edca;               // EDCA parameters
        uint8_t      MaxTxPowerLevel;    // Max Tx Power Level
        cclTxQueueSizeType queueSize;    // Configure CCL (1609.4) transmit queue size
        bool_t       ProviderMode;       // Provider Mode
        rsRadioType  RadioType;          // RT_CV2X or RT_DSRC
        uint8_t      RadioNum;           // Radio Number
        float32_t    conducted2RadiatedPowerOffset;
                                         // Conducted to Radiated Power Offset
        bool_t       ChangeMacAddressEnable;
                                         // Enable the ability to change MAC address
        bool_t       RcvEUFromEthEnable; // Enable receiveing EU packets on the Ethernet interface
        uint16_t     RcvEUFromEthPort;   // Port to receive EU packets on the Ethernet interface
        struct {
            uint8_t  SSPLen;             // SSP length
            uint8_t  SSPValue[MAX_SSP_DATA];
                                         // SSP value
        } SSP;
        bool_t       CV2XPwrCtrlEnable;  // C-V2X power control enable flag
        bool_t       DiscardUnsecureMsg; // Discard WSMs that are unsecured
        bool_t       DiscardVerFailMsg;  // Discard WSMs that fail security verification
        uint32_t     PrimaryServiceId;   // Primary Service ID used for C-V2X Tx
        uint32_t     SecondaryServiceId; // Secondary Service ID used for C-V2X Tx
        uint32_t     ALSMIDebugLogEnable;// 0x1 == ALSMI Debug Log Enable
        uint32_t     ALSMISignVerifyEnable;// 0x1 == ALSMI Sign/Verify
        uint32_t     I2vSyslogEnable;      // 0x1 == Enable I2V Syslog
    } u;
} cfgType;


/* Type definition of valid flags for  Configuration Item Values and their types & ranges */
typedef struct {
        bool_t    MacAddressRadio0;            // Radio 0 Mac address to be configured
        bool_t    MacAddressRadio1;            // Radio 1 Mac address to be configured
        bool_t    AntennaRadio0;               // 0 = Auto, 1 = Tx Antenna 1, 2 = Tx Antenna 2
        bool_t    AntennaRadio1;               // 0 = Auto, 1 = Tx Antenna 1, 2 = Tx Antenna 2
        ///bool_t    RecvMode;                 // Disable Receiver: RECEIVER_DISABLE, Enable: RECEIVER_ENABLE
        bool_t    edca[MAX_EDCA_TXQ];          // EDCA parameters
} RadioCfgValidType;


/* Type definition of Configuration Item Values and their types & ranges.
 * Note that certain cfgId's are only applicable for the DSRC radio; This will
 * return RS_SUCCESS for C-V2X.  See the definition of cfgIdType for a list of
 * cfgId's that are DSRC-only. */
typedef struct {
    /* The following field is automatically filled by RIS, and is not filled
     * out by the application:
     * vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv */
    rsIoctlType       cmd; /* WSU_NS_SET_RADIO_CFG */
    /* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ */
    rsRadioType       radioType;                    // RT_CV2X or RT_DSRC
    uint8_t           MacAddressRadio0[LENGTH_MAC]; // Radio 0 Mac address to be configured
    uint8_t           MacAddressRadio1[LENGTH_MAC]; // Radio 1 Mac address to be configured
    uint8_t           AntennaRadio0;                // 0 = Auto, 1 = Tx Antenna 1, 2 = Tx Antenna 2
    uint8_t           AntennaRadio1;                // 0 = Auto, 1 = Tx Antenna 1, 2 = Tx Antenna 2
    rsEdcaType        edca[MAX_EDCA_TXQ];           // EDCA parameters
    RadioCfgValidType valid;             // Set to true if corresponding structure field has a value set.
} rsRadioCfgType;

//-----------------------------------------------------------------------------


/* Type definition of valid flags for  Configuration Item Values and their types & ranges */
typedef struct {
        bool_t    MacAddress;            // Mac address to be configured
        bool_t    Antenna;               // 0 = Auto, 1 = Tx Antenna 1, 2 = Tx Antenna 2
        ///bool_t    RecvMode;                 // Disable Receiver: RECEIVER_DISABLE, Enable: RECEIVER_ENABLE
        bool_t    edca[MAX_EDCA_TXQ];          // EDCA parameters
} RadioCfgValidType2;


/* Type definition of Configuration Item Values and their types & ranges */
typedef struct {
        uint8_t           MacAddress[LENGTH_MAC]; // Mac address to be configured
        uint8_t           Antenna;                // 0 = Auto, 1 = Tx Antenna 1, 2 = Tx Antenna 2
        ///uint8_t        RecvMode;                     // Disable Receiver: RECEIVER_DISABLE, Enable: RECEIVER_ENABLE
        rsEdcaType        edca[MAX_EDCA_TXQ];          // EDCA parameters
        RadioCfgValidType2 valid;             // Set to true if corresponding structure field has a value set.
} rsRadioCfgType2;



//-----------------------------------------------------------------------------

///TODO:REMOVE
/* Type definition of Configuration Item Values and their types & ranges */
///typedef struct {
///        RadioCfgType2  radioCfg[2];           //One batch RadioCfg struct for each radio.
///} rsRadioCfgType;



//// radioCfgType radioCfg;
//// radioCfg.radio[0].Antenna = 1; ...




/* Type definition of outgoing WSA parameters */
typedef struct {
    uint8_t     handle;             // Security Handle
    uint16_t    dataLength;         // WSA Data Length; 0..MAX_WSA_DATA
    uint8_t     data[MAX_WSA_DATA]; // WSA Data
} outWSAType;

/* Type definition of incoming WSA parameters */
typedef struct {
    uint8_t     handle;             // Security Handle
    uint16_t    dataLength;         // WSA Data Length; 0..MAX_WSA_DATA
    uint8_t     data[MAX_WSA_DATA]; // WSA Data
} inWSAType;

/* Type definition of indication of Channel interval */
typedef struct {
    bool_t         cch_interval;       // TRUE=CCH, FALSE=SCH
    uint8_t         channel_number;     // Channel Number
    uint16_t        timeOnChannel;      // Time on this channel before switching
    uint32_t    timestamp_tv_sec;   // Time stamp - seconds - forced 32-bit to be backwards compatible w 32bit WSU's
    uint32_t    timestamp_tv_usec;  // Time stamp -- usecs - forced 32-bit to be backwards compatible w 32bit WSU's
    uint32_t         wavecch_busyratio;  // WAVE control channel busy ratio
                                        // (0-100%), 999=unknown
    uint32_t         wavesch_busyratio;  // WAVE service channel busy ratio
                                        // (0-100%), 999=unknown
} ChannelIntervalType;

/* WBSS Status type events */
typedef enum {
    RS_ENDED    = 0,        // Not active
    RS_JOINED   = 1,        // Join wbss
} WBSS_EVENT;

/* WBSS Status type reasons */
typedef enum {
    RS_USER_CMD                 = 0,    // User initiate
    RS_WSA_INACTIVITY           = 1,    // No WSA activity
    RS_DATA_INACTIVITY          = 2,    // No data activity on SCH
    RS_OTHER_SERVICE_AVAILABLE  = 3,    // Higher priority service available
} WBSS_REASON;

/* Timing Advertisement (TA) structure */
typedef struct timeAdvertise {
    uint8_t repeatRate;         // The number of TA transmitted every 5 seconds. (1-255)
    uint8_t channelNumber;      // Channel number (172, 174, ...)
    CHANType channelInterval;   // Channel interval to transmit TA message
    uint8_t servicePriority;    // Service priority (0 - 63)
} timingAdvertisementType;

/* Action Type */
typedef enum {
     ADD = 0,
     DELETE,
     CHANGE
} rsActionType;

/* Type definition for WBSS status notification indication */
typedef struct {
    WBSS_EVENT  wbssEvent;              // Status change event
    WBSS_REASON wbssReason;             // Reason for status change
    /* The following fields are only valid if wbssEvent == RS_JOINED */
    uint8_t     bssid[LENGTH_BSSID];    // Provider BSSID
    uint8_t     channelNumber;          // Service channel (SCH) number
} WbssStatusType;





typedef struct dnTallies {
    /* Result of the WSU_NS_GET_RADIO_TALLIES command to radioServices */
    rsResultCodeType risRet;

    /* Rx Tallies */
    uint32_t rx_good;    /* good receives */
    uint32_t rx_ucast;   /* rx unicast frames */
    uint32_t rx_bcast;   /* rx broadcast frames */
    uint32_t rx_mcast;   /* rx broadcast frames */
    uint32_t rx_good_bytes;  /* good received bytes */
    uint32_t rx_data;    /* rx data frames */
    uint32_t rx_ctrl;    /* rx control frames */
    uint32_t rx_mgt;     /* rx management frames */
    uint32_t rx_err;     /* rx errors */
    uint32_t rx_crc_err; /* rx CRC errors */
    /*uint32_t   rx_fcs_fail;*/  /* FCS fail count */ /* Not supported */
    uint32_t rx_phy_err; /* rx phy errors */
    uint32_t rx_dup_seq_err; /* rx duplicate seq errors */
    uint32_t rx_overrun;     /* Number of rx overrun, new */

    /* Tx Tallies */
    uint32_t tx_good;    /* good Xmits */
    uint32_t tx_ucast;   /* tx unicast frames */
    uint32_t tx_bcast;   /* tx broadcast frames */
    uint32_t tx_mcast;   /* tx multicast frames */
    uint32_t tx_good_bytes;  /* good Xmitted bytes */
    uint32_t tx_data;    /* tx data frames */
    uint32_t tx_mgt;     /* tx data frames */
    uint32_t tx_ctrl;    /* tx data frames */
    uint32_t tx_err;     /* tx errors */
    uint32_t tx_drop;    /* tx packet dropped */
    uint32_t tx_retries;      /* Number of retries */

    /* Misc Tallies */
    int32_t rssi;       /* last recvd rssi (value_dBm = rssi - 100) */
    uint32_t tx_power;   /* last Xmitted Power Level (scale:1=0.5db) */
    uint32_t rx_rate;    /* last recvd data rate   */
    uint32_t tx_rate;    /* last Xmitted data rate */
                        /* Both rx_rate and tx_rate have the following */
                        /* values, depending on the channel bandwidth */
                        /*               (Data Rate)                 */
                        /* Value   10MHz Channel   20MHz Channel     */
                        /* -----   -------------   -------------     */
                        /* 8       24 Mbps         48 Mbps           */
                        /* 9       12 Mbps         24 Mbps           */
                        /* 10      6 Mbps          12 Mbps           */
                        /* 11      3 Mbps          6 Mbps            */
                        /* 12      27 Mbps         54 Mbps           */
                        /* 13      18 Mbps         36 Mbps           */
                        /* 14      9 Mbps          18 Mbps           */
                        /* 15      4.5 Mbps        9 Mbps            */
    uint32_t rx_ant;     /* last recvd antenna */
    uint32_t ant_tx[2];  /* Xmitted frame count per antenna */
    uint32_t ant_rx[2];  /* Rcvd frame count per antenna */
    uint32_t rxant_sw_cnt;   /* antenna switch count */
    uint32_t rawchan_busyratio;  /* raw channel busy ratio (0-100%), 999=unknown */
    uint32_t wavecch_busyratio;  /* WAVE control channel busy ratio (0-100%), 999=unknown */
    uint32_t wavesch_busyratio;  /* WAVE service channel busy ratio (0-100%), 999=unknown */
    uint32_t rx_queue_depth_radio_driver; /* Rx queue depth for the radio driver */
    uint32_t tx_queue_depth[4]; /* current elements in Tx queue for AC=0-3 */
    uint32_t tx_queue_drop[4];  /* Tx frames dropped by CCL for AC=0-3 */
    uint32_t rx_queue_depth;    /* current elements in Rx queue */
    uint32_t rx_queue_drop;     /* Rx frames dropped due to max Rx queue depth exceeded */

    uint32_t rx_mkx_bad_pkts;   /* LLC rx passed packet not an MKXIF_RXPACKET */
    uint32_t rx_mkx_short_pkts; /* LLC rx passed packet < minimum header size */
    uint32_t tx_bad_channel;    /* WSM requested to be sent on channel not configured on the radio. */

    /* Autotalks-specific RX Tallies */
    uint32_t cv2x_rx_l2_messages_received_successfully;
    uint32_t cv2x_rx_rlc_messages_reassembled_successfully;
    uint32_t cv2x_rx_pdcp_messages_received_successfully;
    uint32_t cv2x_rx_svc_messages_received_successfully;
    uint32_t cv2x_rx_mac_messages_received_successfully;
    /* Autotalks-specific RX Error Tallies */
    uint32_t cv2x_rx_l2_messages_dropped_crc_errors;
    uint32_t cv2x_rx_l2_messages_dropped_queue_full;
    uint32_t cv2x_rx_l2_messages_dropped_exceeding_max_rx_size;
    uint32_t cv2x_rx_rlc_messages_reassembly_timed_out;
    uint32_t cv2x_rx_rlc_messages_duplicated;
    uint32_t cv2x_rx_pdcp_messages_dropped_parsing_error;
    uint32_t cv2x_rx_svc_messages_dropped;
    uint32_t cv2x_rx_mac_messages_dropped_invalid_destination_l2id;
    uint32_t cv2x_rx_mac_messages_dropped_queue_full;
    uint32_t cv2x_rx_mac_messages_dropped_invalid_struct;
    uint32_t cv2x_rx_mac_messages_dropped_invalid_version;
} DeviceTallyType;

/* The following is defined in order to let other code know that the fields
 * tx_queue_depth_radio_0 through rx_queue_drop exist in the DeviceTallyType
 * structure. Any reference to these fields should have an #ifdef
 * RADIO_SUPPORT_TXRX_QUEUE_COUNTERS and an #endif around it.
 */
#define RADIO_SUPPORT_TXRX_QUEUE_COUNTERS

/* The following structures support the full use radio stack. */

/* These WAVE_ID_XXX_BITMASK are used in the outWSMType WsmpHeaderExt field
 to enable optional extensions in the WSM over-the-air header */
#define WAVE_ID_TRANSMIT_POWER_BITMASK        0x01  // Transmit power
#define WAVE_ID_CHANNEL_NUMBER_BITMASK        0x02  // Channel
#define WAVE_ID_DATARATE_BITMASK              0x04  // Transmit Data rate
#define MAX_SAFETY_SUBLAYER_DATA              16    // Max bytes supported in safety sublayer

/* These WSA optional extension bitmasks are used in the provider service request */
/* The WSA_SIGNED and WSA_VERIFIED bit definitions are special ones used to
 * indicate whether or not the WSA has been signed and/or verified. They do
 * not indicate the presence or absence of extenrion fields. */
#define WSA_SIGNED                                      (1 << 0)
#define WSA_VERIFIED                                    (1 << 1)
#define WSA_EXT_WRA_BITMASK                             (1 << 3)    // WSA WRA
#define WSA_EXT_2DLOCATION_BITMASK                      (1 << 5)    // WSA header
#define WSA_EXT_3DLOCATION_BITMASK                      (1 << 6)    // WSA header
#define WSA_EXT_ADVERTISER_ID_BITMASK                   (1 << 7)    // WSA header
#define WSA_EXT_PROVIDER_SERVICE_CONTEXT_BITMASK        (1 << 8)    // WSA service info
#define WSA_EXT_IPV6ADDRESS_BITMASK                     (1 << 9)    // WSA service info
#define WSA_EXT_SERVICE_PORT_BITMASK                    (1 << 10)   // WSA service info
#define WSA_EXT_PROVIDER_MACADDRESS_BITMASK             (1 << 11)   // WSA service info
#define WSA_EXT_EDCA_PARAM_SET_BITMASK                  (1 << 12)   // WSA channel info
#define WSA_EXT_SECONDARY_DNS_BITMASK                   (1 << 13)   // WSA WRA
#define WSA_EXT_GATEWAY_MACADDRESS_BITMASK              (1 << 14)   // WSA WRA
#define WSA_EXT_REPEAT_RATE_BITMASK                     (1 << 17)   // WSA header
#define WSA_EXT_RCPI_THRESHOLD_BITMASK                  (1 << 19)   // WSA service info
#define WSA_EXT_WSA_COUNT_THRESHOLD_BITMASK             (1 << 20)   // WSA service info
#define WSA_EXT_CHANNEL_ACCESS_BITMASK                  (1 << 21)   // WSA channel info
#define WSA_EXT_WSA_COUNT_THRESHOLD_INTERVAL_BITMASK    (1 << 22)   // WSA service info

/* The following fields within the userServiceType and outWSMType structures
   impact channel switching and control when and where WSMs are transmitted.
   There are different combinations of values that control this functionality and
   some combinations of values will result in unsupported functionality.
   The following table describes the supported and unsupported combinations
   (note that channel 172 can be changed to be any of the service channels).

       (userServiceType)                          (outWSMType)
extended channel                                  channel  channel
Access   Number      userAccess                   Number   Interval
-------  ------      -------------                -------  --------
  0      172         AUTO_ACCESS_UNCONDITIONAL    172      0 or 2
     - Switch to chan 178 during CCH interval and chan 172 during SCH interval
     - Tx WSM on chan 172 during SCH interval

  0      172         AUTO_ACCESS_UNCONDITIONAL    178      0 or 1
     - Switch to chan 178 during CCH interval and chan 172 during SCH interval
     - Tx WSM on chan 178 during CCH interval

  0      dontcare    NO_SCH_ACCESS                178      0 or 1
     - Remain on chan 178 during CCH and SCH intervals (but keep track of intervals)
     - Tx WSM on chan 178 during CCH interval

  0      dontcare    NO_SCH_ACCESS                178      2
     - Remain on chan 178 during CCH and SCH intervals (but keep track of intervals)
     - Tx WSM on chan 178 during SCH interval

  65535  172         AUTO_ACCESS_UNCONDITIONAL    172      0
     - Remain on chan 172 all the time
     - Tx WSM on chan 172 anytime

  1-65534 172        AUTO_ACCESS_UNCONDITIONAL    172      0 or 2
     - Stay on chan 172 until "extendedAccess" intervals and then switch to
       chan 178 during CCH interval and chan 172 during SCH interval
     - Tx WSM on chan 172 anytime or during SCH interval after  switching resumes

  1-65534 172        AUTO_ACCESS_UNCONDITIONAL    178      0 or 1
     - Stay on chan 172 until "extendedAccess" intervals and then switch to
       chan 178 during CCH interval and chan 172 during SCH interval
     - Tx WSM on chan 178 during CCH interval after access to channel 178 has resumed

[Known Unsupported Combinations]
  0      172         AUTO_ACCESS_UNCONDITIONAL    172      1
     - Switch to chan 178 during CCH interval and chan 172 during SCH interval
     - Tx WSM on chan 172 during CCH interval (not possible)

  0      172         AUTO_ACCESS_UNCONDITIONAL    178      2
     - Switch to chan 178 during CCH interval and chan 172 during SCH interval
     - Tx WSM on chan 178 during SCH interval (not possible)
*/

#define MAX_WSMP_HDR_LEN    18  // Maximum length of WSMP header = 1
                                // (WSMP version) + 1 (extension count) + 9
                                // (max of 3 extensions, 3 bytes each) + 1
                                // (TPID) + 4 (max PSID length) + 2 (max
                                // data length) = 18 bytes
#define FAMILY_TYPE_LEN     1   // Size of family type for C-V2X = 1 byte
#define MK5_HDR_LEN         28  // Size of MK5 header = 28 bytes
#define IEEE_QOS_HDR_LEN    26  // Size of IEEE QoS header = 26 bytes
#define FULL_SNAP_HDR_LEN   8   // Size of full SNAP header = 8 bytes
#define HDRS_LEN \
(MK5_HDR_LEN + IEEE_QOS_HDR_LEN + FULL_SNAP_HDR_LEN + MAX_WSMP_HDR_LEN)
                                // Maximum number of bytes to add to the
                                // beginning of the C-V2X packets =
                                // MAX_WSMP_HDR_LEN + FAMILY_TYPE_LEN
                                // = 19 bytes
                                //
                                // Maximum number of bytes to add to the
                                // beginning of the DSRC packets =
                                // MAX_WSMP_HDR_LEN + MK5_HDR_LEN +
                                // IEEE_QOS_HDR_LEN + FULL_SNAP_HDR_LEN
                                // = 80 bytes
                                //
                                // So for now, make hdrs 85 bytes to allow
                                // room in front of "data" to add headers

/* Type definition of outgoing WSM parameters for messages */
typedef struct {
    /* The following fields are automatically filled by RIS, and are not filled
     * out by the application:
     * vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv */
    rsIoctlType  cmd;               // WSU_NS_SEND_WSM_REQ
    pid_t        pid;               // Caller PID
    uint8_t      version;           // WSMP version
    /* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ */
    uint8_t      channelNumber;     // WSM Channel Number
    dataRateType dataRate;          // WSM Tx Data Rate
    float32_t    txPwrLevel;        // WSM Conducted Tx Power Level (0-23, in 0.5 dB increments)
    uint8_t      txPriority;        // User priority (0 - 7)
    uint32_t     wsmpExpireTime;    // Expiration time in milliseconds (0=none)
    rsRadioType  radioType;         // RT_CV2X or RT_DSRC
    uint8_t      radioNum;          // Radio to transmit with (0-1)
    uint8_t      channelInterval;   // 0=default (based on channelNumber), 1=CCH interval (time slot 0), 2=SCH interval (time slot 1)
    securityType security;          // SECURITY_UNSECURED = Packet is unsecured
                                    // SECIRITY_SIGNED = Packet is to be signed
                                    // All other values unsupported
    uint32_t     securityFlag;      // Application specific security manager interface
                                    // 0=Tx digest, 1=Tx certificate
    uint8_t      peerMacAddress[LENGTH_MAC];
                                    // Destination Mac address of packet
    uint32_t     wsmpHeaderExt;     // Set to include optional WSM header extensions (0=no extensions)
                                    // (set to one or more WAVE_ID_ ... bit masks)
    uint32_t     psid;              // Provider Service Identifier (PSID)
    bool_t       isBitmappedSsp;    // TRUE if bitmapped SSP
    uint8_t      sspLen;            // Length of the SSP data
    uint8_t      sspMask[MAX_SSP_DATA];
                                    // Used if bitmasked SSP
    uint8_t      ssp[MAX_SSP_DATA]; // SSP data
    uint16_t     dataLength;        // WSM Data Length; 0..MAX_WSM_DATA
    /* VJR WILLBEREVISITED May need to allow room for security headers or
     * 1609.2 headers in the future */
    /* The following field is automatically filled by RIS, and is not filled 
     * out by the application. Must be immediately before "data".
     * vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv */
    uint8_t      hdrs[HDRS_LEN];    // Allow room for various headers. See
                                    // definition for HDRS_LEN for details.
    /* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ */
    uint8_t      data[MAX_WSM_DATA];// WSM Data
} outWSMType;

typedef struct {
    /* The following field is automatically filled by RIS, and is not filled
     * out by the application:
     * vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv */
    rsIoctlType cmd;                // WSU_NS_SEND_RAW_PACKET_REQ
    /* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ */
    rsRadioType radioType;          // RT_CV2X or RT_DSRC
    uint8_t     radioNum;           // Used by dsrc_tx(); The radio number to
                                    // transmit on.
    uint8_t     channelInterval;    // 0 = Determine which interval to use
                                    //     based on the channelNumber field,
                                    //     and the configured control channel
                                    //     and service channel numbers.
                                    // 1 = Control Channel interval.
                                    // 2 = Service Channel interval.
    uint8_t     channelNumber;      // The channel number to transmit on. Must
                                    // match either the configured service
                                    // channel number or control channel
                                    // number.
    uint8_t     dataRate;           // Tx data rate.
    uint8_t     txPwrLevel;         // Tx power level (-128..127).
    uint8_t     data[MAX_WSM_DATA]; // Packet data
    uint16_t    dataLen;            // Packet length
} outRawType;

/* Type definition of incoming WSM parameters for messages */
typedef struct {
    uint8_t      wsmVersion;        // Version 3
    uint8_t      channelNumber;     // WSM Channel Number from WSM header (0xff = not available)
    dataRateType dataRate;          // WSM Rx Data Rate
    uint8_t      txPwrLevel;        // WSM Tx Power Level from WSM header (0xff = not available)
    uint32_t     psid;              // Provider Service Identifier (PSID)
    uint8_t      userPriority;      // Received user priority (0=[0,3],1=[1,2],5=[4,5],6=[6,7], 0xff=not available)
    uint8_t      srcMacAddress[LENGTH_MAC];
                                    // Source Mac address of packet
    int8_t       rss;               // Receive signal strength (dBm)
    rsRadioType  radioType;         // Radio Type: RT_CV2X or RT_DSRC
    uint8_t      radioNum;          // Receive radio number (0-1)
    securityType security;          // Security Option (TBD if needed)
    uint32_t     securityFlags;     // Application specific security manager interface
                                    // internal flags. Will be set to following enumerated values:
                                    // LSB indicates verfication status:
                                    // unsecured = 1, securedButNotVerified=2,
                                    // securedVerifiedSuccess=3,
                                    // securedVerifiedFail=4, securedVerNotNeeded=5

                                    // LSB + 1 byte indicates verfication error code,
                                    // in case of securedVerifiedFail.
                                    // For interpretation of error code, refer to Aerolink spec.
    uint32_t     securityID;        // Application specific security manager interface
                                    // message identifier
    uint16_t     recvPlcpLength;    // Receive total length from driver
    uint16_t     recvWsmOTALength;  // Actual over-the-air value of received WSM length field
    uint64_t     timestamp_tv_sec;  // Time stamp - seconds - forced 64-bit to be compatible w 64bit WSU's
    uint64_t     timestamp_tv_usec; // Time stamp -- usecs - forced 64-bit to be backwards compatible w 64bit WSU's
    uint32_t     vodMsgSeqNum;      // Verify-on-Demand msg seq num, assigned by security and used by App to reference pkt
                                    // A value of 0 indicates the Verify-on-Demand handle is invalid.
    float32_t    rssAntA;           // received power on Antenna A (0.5 dB resolution, only valid if antenna used)
    float32_t    rssAntB;           // received power on Antenna B (0.5 dB resolution, only valid if antenna used)
    float32_t    noiseAntA;         // receiver noise on Antenna A (0.5 dB resolution, only valid if antenna used)
    float32_t    noiseAntB;         // receiver noise on Antenna B (0.5 dB resolution, only valid if antenna used)
    uint8_t      sspLen;            // Length of received SSP data
    uint8_t      ssp[MAX_SSP_DATA]; // SSP data
    uint8_t      radioChannel;      // Radio channel message received on (as configured at radio)
    uint8_t      timeSlot;          // time slot message received on per 1609.4 (0 or 1)
    uint8_t      unused[64];        // Some spare bytes so we can later add fields to this structure and still keep
                                    // dataLength and data at the end. All of the fields after but NOT including
                                    // timeSlot and before but NOT including dataLength should add up to 64 bytes
                                    // in length.
    uint16_t     dataLength;        // WSM Data Length; 0..MAX_WSM_DATA
    uint8_t      data[MAX_WSM_DATA];// WSM Data
    uint32_t     ppp;               // Point to Point Priority
} inWSMType;

/* This is an old type that is no longer used, except for playing back older
 * SR files. In this case, items of type inWSMTypeLegacy are converted to
 * items of type inWSMType before being delivered to the application. */
typedef struct {
    uint8_t     wsmVersion;         // Version 2
    uint8_t     channelNumber;      // WSM Channel Number from WSM header (0xff = not available)
    dataRateType dataRate;          // WSM Rx Data Rate
    uint8_t     txPwrLevel;         // WSM Tx Power Level from WSM header (0xff = not available)
    uint32_t    psid;               // Provider Service Identifier (PSID)
    uint8_t     userPriority;       // Received user priority (0=[0,3],1=[1,2],5=[4,5],6=[6,7], 0xff=not available)
    uint16_t    dataLength;         // WSM Data Length; 0..MAX_WSM_DATA_LEGACY
    uint8_t     data[MAX_WSM_DATA_LEGACY];
                                    // WSM Data
    uint8_t     srcMacAddress[LENGTH_MAC];  // Source Mac address of packet
    int8_t      rss;                // Receive signal strength (dBm)
    uint8_t     radioNum;           // Receive radio number (0-1)
    securityType security;          // Security Option (TBD if needed)
    uint32_t    securityFlags;      // Application specific security manager interface
                                    // internal flags. Will be set to following enumerated values:
                                    // LSB indicates verfication status:
                                    // unsecured = 1, securedButNotVerified=2,
                                    // securedVerifiedSuccess=3,
                                    // securedVerifiedFail=4, securedVerNotNeeded=5

                                    // LSB + 1 byte indicates verfication error code,
                                    // in case of securedVerifiedFail.
                                    // For interpretation of error code, refer to esBOX spec.

    uint32_t    securityID;         // Application specific security manager interface
                                    // message identifier
    uint8_t     wsmp_S_Length;      // Number of bytes in safety sublayer (0=not used)
    uint8_t     wsmp_S_Control[MAX_SAFETY_SUBLAYER_DATA];  // safety sublayer control field (if WSMP_S_Length!=0)
                                    // App should ignore MSbit of each byte in wsmp_S_Control
    uint16_t    recvPlcpLength;     // Receive total length from driver
    uint16_t    recvWsmOTALength;   // Actual over-the-air value of received WSM length field
    uint32_t    timestamp_tv_sec;   // Time stamp - seconds - forced 32-bit to be backwards compatible w 32bit WSU's
    uint32_t    timestamp_tv_usec;  // Time stamp -- usecs - forced 32-bit to be backwards compatible w 32bit WSU's
    uint32_t    vodMsgSeqNum;       // Verify-on-Demand msg seq num, assigned by security and used by App to reference pkt
                                    // A value of 0 indicates the Verify-on-Demand handle is invalid.
    float32_t   rssAntA;            // received power on Antenna A (0.5 dB resolution, only valid if antenna used)
    float32_t   rssAntB;            // received power on Antenna B (0.5 dB resolution, only valid if antenna used)
    float32_t   noiseAntA;          // receiver noise on Antenna A (0.5 dB resolution, only valid if antenna used)
    float32_t   noiseAntB;          // receiver noise on Antenna B (0.5 dB resolution, only valid if antenna used)
    uint8_t     sspLen;             // Length of received SSP data
    uint8_t     ssp[MAX_SSP_DATA];  // SSP data
    uint8_t     radioChannel;       // Radio channel message received on (as configured at radio)
    uint8_t     timeSlot;           // time slot message received on per 1609.4 (0 or 1)
} inWSMTypeLegacy;

/* Type definition of User service parameters for messages */
typedef struct {
    /* The following two fields are automatically filled by RIS, and are not
     * filled out by the application:
     * vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv */
    rsIoctlType       cmd;              // WSU_NS_USER_SERVICE_REQ
    pid_t             pid;              // Caller PID
    /* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ */

    rsRadioType       radioType;        // RT_CV2X or RT_DSRC
    uint8_t           radioNum;         // Radio number to use (0-1)
    rsActionType      action;           // Action of this request (add, delete)
    rsUserRequestType userAccess;       // Defines when to provide SCH access
    uint32_t          psid;             // Provider Service Identifier of desired service
    uint8_t           servicePriority;  // Service Priority (0-63, 63=highest)
    rsWsaSecurityType wsaSecurity;      // WSA security type
    uint8_t           lengthPsc;        // PSC length (0-31)
    uint8_t           psc[LENGTH_PSC];  // Optional PSC in incoming WSA of desired service
                                        // (if length != 0)
    uint8_t           channelNumber;    // Channel Number to use for service (0=use value in incoming WSA)
    uint8_t           srcMacAddress[LENGTH_MAC];
                                        // if FF:FF:FF:FF:FF:FF, accept incoming WSA with
                                        // any source MAC address. Otherwise, incoming
                                        // WSA must match srcMACAddress field to be accepted.
    uint8_t           lengthAdvertiseId;// Advertise ID length (0-32)
    uint8_t           advertiseId[LENGTH_ADVERTISE];
                                        // Optional advertiseID in incoming WSA of desired
                                        //service  (if length != 0)
    int8_t            linkQuality;      // Minimum acceptable signal strength (in dbm) of incoming
                                        // WSA to be accepted (range is -100 thru -20).
    bool_t            immediateAccess;  // Indicates if device should immediately visit SCH instead
                                        // of waiting for next SCH interval
    uint16_t          extendedAccess;   // Indicates how many CCH intervals the radio should dwell on the SCH.
                                        // 0 = switch between CCH and SCH every interval
                                        // 1 - 65534 = dwell on SCH for x number of intervals
                                        //             (without switching to CCH)
                                        // 65535 = remain on SCH indefinitely (until service is deleted).
} UserServiceType;

typedef enum {
   CHANGE_ACCESS = 0,
   CHANGE_PSC    = 1,
   CHANGE_MAX    = 2,
} rsUserCommandType;

typedef struct {
    /* The following two fields are automatically filled by RIS, and are not
     * filled out by the application:
     * vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv */
    rsIoctlType               cmd;          // WSU_NS_USER_CHANGE_SERVICE_REQ
    pid_t                     pid;          // Caller PID
    /* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ */
    uint32_t                  psid;
    rsUserCommandType         command;

    union {
        struct {
            uint8_t           channelNumber;// The channel which carried from indication
                                            // from provider service
            uint8_t           adaptable;    // 1 ==> data rate is min, txpower = max
                                            // 0 ==> data rate and txpower fixed
            uint8_t           dataRate;     // minimum or fixed data rate
            float32_t         txPwrLevel;   // WSM maxiumum or fixed Tx Power Level (0-23, in 0.5 dB increments)
            rsUserRequestType userAccess;   // Defines when to provide SCH access
            uint16_t          extendedAccess;
                                            // Indicates how many CCH intervals the radio should dwell on the SCH.
                                            // 0 = switch between CCH and SCH every interval
                                            // 1 - 65534 = dwell on SCH for x number of intervals
                                            //             (without switching to CCH)
                                            // 65535 = remain on SCH indefinitely (until service is deleted).
        } access;
    } user;
} UserServiceChangeType;

/* Type definition of WSM service parameters for messages */
typedef struct {
    /* The following two fields are automatically filled by RIS, and are not
     * filled out by the application:
     * vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv */
    rsIoctlType  cmd;       // WSU_NS_WSM_SERVICE_REQ
    pid_t        pid;       // Caller PID
    /* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ */
    rsActionType action;    // Action of this request (add, delete)
    rsRadioType  radioType; // RT_CV2X or RT_DSRC
    uint32_t     psid;      // Provider Service Identifier of received WSMs
} WsmServiceType;

#define PCAP_LOG_BUF_SIZE    65536
#define PCAP_TX_LOG_BUF_SIZE 16384

/* Bit definitions for PCAP bitmask */
#define PCAP_TX                 0x01
#define PCAP_RX                 0x02
#define PCAP_TX_RX_SEPARATE     0x04
#define PCAP_ALL_DEVICES_SHARED 0x08
#define PCAP_PRISM_HDR          0x10

/* Type definition of PCAP enable */
typedef struct {
    /* The following field is automatically filled by RIS, and is not filled
     * out by the application:
     * vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv */
    rsIoctlType  cmd;       // WSU_NS_PCAP_ENABLE
    /* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ */
    rsRadioType  radioType; // RT_CV2X or RT_DSRC
    uint8_t      radioNum;  // Radio number 0..1
    uint32_t     mask;      // PCAP bitmask
} rsPcapEnableType;

/* Type definition of PCAP disable */
typedef struct {
    /* The following field is automatically filled by RIS, and is not filled
     * out by the application:
     * vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv */
    rsIoctlType  cmd;       // WSU_NS_PCAP_DISABLE
    /* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ */
    rsRadioType radioType; // RT_CV2X or RT_DSRC
    uint8_t     radioNum;  // Radio number 0..1
} rsPcapDisableType;

/* Type definition of PCAP read */
typedef struct {
    /* The following field is automatically filled by RIS, and is not filled
     * out by the application:
     * vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv */
    rsIoctlType  cmd;       // WSU_NS_PCAP_READ
    /* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ */
    rsRadioType radioType; // RT_CV2X or RT_DSRC
    uint8_t     radioNum;  // Radio number 0..1
    bool_t      tx;        // TRUE if desire to read from Tx buffer
} rsPcapReadType;

#define MAX_PCAP_READ_BYTES 2048
typedef struct {
    /* Result of the WSU_NS_PCAP_READ command to radioServices */
    rsResultCodeType risRet;                    // RS_SUCCESS or error code
    bool_t           done;                      // TRUE if the last byte in the
                                                // PCAP buffer has been read
    uint16_t         size;                      // Number of bytes read in this
                                                // iteration
    uint8_t          data[MAX_PCAP_READ_BYTES]; // Up to MAX_PCAP_READ_BYTES
                                                // bytes of data
} rsPcapReadReplyType;

/* Type definition of EU service parameters for messages */
/*
 * NOTE:
 * EU packets are registered by etherType and port number.  PSID field must be
 * a PSID that is a valid PSID (per PSID definition) but is guaranteed to never
 * be used by a WSM msg. (e.g. EU packets will use the CAM ITS-AID (0x24) which
 * meets this criteria.)
 *  * For all supported EU EtherTypes (currently EU), incomming packets are
 *    filtered based on EtherType and destPort(and not the PSID field).
 *  * Specifying a destPort value of EU_CATCH_ALL_PORT means that the APP
 *    registering for that port number will get all packets whose destination
 *    port is not otherwise registered for. It will also get all packets which
 *    don't specify a destination port (e.g. beacon, LS request, LS reply).
 *    Only one APP can resiter for the EU_CATCH_ALL_PORT (usually euwmh).
 *  * The registered PSID will show up in RadioStack stats for EU packets.
 *  * All packets will the registered EtherType will be sent to the registering
 *    APP regardless of the registered PSID.
 *  * Received EU packet are discarded unless the EtherType is registered via
 *     wsuRisEUServiceRequest().
 *  * wsuRisEUServiceRequest() cannot be use to register for WSMP packets.
 *
 */
typedef struct {
    rsActionType action;            // Action of this request (add, delete)
    uint32_t     psid;              // PSID registered for EU packets(
    uint16_t     etherType;         // EtherType of EU packets registering to receive.
    uint16_t     destPort;          // Destination port of EU packets registering to receive.
} EUServiceType;


/* Type definition of User Access Services Type */
#define MAX_SECURITY_PERMISSIONS 255
#define MAX_SECURITY_SERVICE_ID  16

/* Type definition of Provider service parameters for messages */
typedef struct {
    /* The following two fields are automatically filled by RIS, and are not
     * filled out by the application:
     * vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv */
    rsIoctlType       cmd;              // WSU_NS_USER_SERVICE_REQ
    pid_t             pid;              // Caller PID
    /* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ */

    rsRadioType radioType;          // RT_CV2X or RT_DSRC
    uint8_t     radioNum;           // Radio number to use (0-1)
    rsActionType action;            // Action of this request (add, delete)
    uint8_t     destMacAddress[LENGTH_MAC];
                                    // Destination MAC address of WSA.
                                    // Normally should be if FF:FF:FF:FF:FF:FF
                                    // for broadcast.
    rsWsaSecurityType wsaSecurity;  // WSA security type
    uint32_t     psid;              // Provider Service Identifier of desired service
    uint8_t     lengthPsc;          // Provider Service Contents (PSC) length (0-31)
    uint8_t     psc[LENGTH_PSC];    // Optional PSC data (if length !=0)
    uint8_t     servicePriority;    // Service Priority (0-63, 63=highest)
    uint8_t     lengthSSP;          // SSP length (0-MAX_SECURITY_PERMISSIONS)
    uint8_t     serviceSpecificPermissions[MAX_SECURITY_PERMISSIONS];
                                    // Security processing P1609.2
    uint8_t     lengthSecurityServiceId;
                                    // Security Service ID length (0-MAX_SECURITY_SERVICE_ID)
    uint8_t     securityServiceId[MAX_SECURITY_SERVICE_ID]; // Security service
                                    // (if length != 0)
    uint8_t     channelNumber;      // Channel Number to use for service
    uint8_t     edca_count;         // Number of elements in the EDCA arrays below [1..4] (optional)
    uint16_t    txop[4];            // Channel info EDCA, TxOp limit (b0-b15) (optional)
    uint8_t     ecw[4];             // Channel info EDCA, ECWmin b0-b3, ECWmax b4-b7 (optional)
    uint8_t     aifs[4];            // Channel info EDCA, Arbitration Interframe Spacing (optional)
    uint8_t     channelAccess;      // 0=continuous (CCH+SCH intervals),
                                    // 1=alternating (CCH interval only, not currently supported)
                                    // 2=alternating (SCH interval only)
    uint8_t     repeatRate;         // The number of WSA transmitted every 5 seconds.
                                    // If destination MAC address is an individual
                                    // address, Repeat rate is ignored
    bool_t     ipService;           // Indicates whether the advertised service is
                                    // IP-based, and a WAVE Routing Advertisement is
                                    // needed to support this service.
    uint8_t     IPv6Address[LENGTH_IPV6];
                                    // IPv6 Address
    uint16_t    IPv6ServicePort;    // IPv6 Service Port
    uint8_t     providerMACAddress[LENGTH_MAC]; // Service MAC Address
    uint8_t     rcpiThreshold;      // For insertion in WSA if present
    uint8_t     wsaCountThreshold;  // For insertion in WSA if present
    uint8_t     wsaCountThresholdInterval; // For insertion in WSA if present
    uint32_t     wsaHeaderExtension;// Indicates which of the WSA header extension
                                    // should be included in the transmitted WSA
                                    // (see WSA_EXT ... bitmasks)
    uint16_t    signatureLifeTime;  // The number of milliseconds over which the WSA
                                    // signature should be valid (10 - 30,000 ms)
    uint8_t     advertiseIdLength;  // Advertiser ID length
    int32_t     latitude2D;         // WSA header 2D Location, latitude
    int32_t     longitude2D;        // WSA header 2D Location, longitude
    int32_t     latitude3D;         // WSA header 3D Location and confidence, latitude
    int32_t     longitude3D;        // WSA header 3D Location and confidence, longitude
    int16_t     elevation3D;        // WSA header 3D Location and confidence, elevation
    uint8_t     confidence3D;       // WSA header 3D Location and confidence, confidence
    uint8_t     semiMajorAccuracy3D;// WSA header 3D Positional accuracy, semi-major
    uint8_t     semiMinorAccuracy3D;// WSA header 3D Positional accuracy, semi-minor
    uint16_t    semiMajorOrientation3D;
                                    // WSA header 3D Positional accuracy, semi-major orientation
    uint8_t     advertiseId[WSA_ADVERTISE_ID_LEN];
                                    // WSA header advertiser ID
    uint16_t    routerLifeTime;     // Routing info router life time
    uint8_t     ipPrefix[LENGTH_IPV6];
                                    // Routing info IP Prefix
    uint8_t     prefixLen;          // Routing info prefix length
    uint8_t     defaultGateway[DEFAULT_GATEWAY_LEN];
                                    // Routing info default gateway
    uint8_t     primaryDns[LENGTH_IPV6];
                                    // Routing info primary DNS
    uint8_t     secondaryDns[LENGTH_IPV6];
                                    // Routing info secondary DNS
    uint8_t     gatewayMacAddress[LENGTH_MAC];
                                    // Routing info gateway MAC address
} ProviderServiceType;

/* Type definition of Delete all services parameters for messages */
typedef struct {
    rsRadioType radioType; // RT_CV2X or RT_DSRC
    uint8_t     radioNum;  // Radio number to delete services for (0-1)
    bool_t      allPids;   // TRUE ==> Delete all services for the given radio number
                           //           for all PID's
                           // FALSE ==> Delete all services for the given radio number
                           //            for the caller's PID only
} DeleteAllServicesType;

#define RIS_LAYER_MASK          0x00000001
#define NS_LAYER_MASK           0x00000002
#define CCL_LAYER_MASK          0x00000004
#define IEEE80211_LAYER_MASK    0x00000008
#define ATH_LAYER_MASK          0x00000010

#define TX_LEVEL_MASK           0x00000001
#define RX_LEVEL_MASK           0x00000002
#define MGMT_LEVEL_MASK         0x00000004
#define PSID_LEVEL_MASK         0x00000008

/* Warning Status type reasons */
typedef enum {
    WS_ERROR_NONE               = 0x0000,
    WS_ERROR_VERSION            = 0x1000,    // Error WSM version
    WS_ERROR_PARAMETER          = 0x1001,    // User function error parameter
    WS_RADIO_ACCESS_ERROR       = 0x1002,    // Radio stack support MAX 2 radios and may Physically connect 1 radio
    WS_WSA_VERIFY_LEN_TOO_BIG   = 0x1003,    // WSA verification length too big
    WS_WSA_SIGN_LEN_TOO_BIG     = 0x1004,    // WSA sign length too big
    WS_USER_PRIORITY_ERROR      = 0x1005,    // User priority should be in 0 - 7 range
    WS_UNKNOWN_PROTOCOL         = 0x1006,    // Protocol is other than WSMP and WSP_S
    WS_EXT_UNKNOWN_PROTOCOL     = 0x1007,    // WSMP extension unknown
    WS_TOO_MANY_SERVICES        = 0x1008,    // Exceed maximum services
    WS_APP_NOT_REGISTERED       = 0x1009,    // Application not registered
    WS_TX_WSMP_DROPPED_PKT      = 0x100A,    // Tx packet drop in NS
    WS_WSA_LENGTH_PSC_TOO_LONG  = 0x100B,    // Tx packet PSC too long
    WS_PSID_FORMAT_UNKNOWN      = 0x2000,    // PSID first byte doesn't follow the encoding format
    WS_PSID_HAS_ALREADY_REGISTERED = 0x2001,    // PSID already exist.
    WS_PSID_NOT_REGISTERED      = 0x2002,    // PSID not registered
    WS_RX_RECV_LEN_NOT_MATCHED  = 0x2003,    // RX Length not matched.
    WS_RX_RECV_SAFETY_LEN_NOT_MATCHED  = 0x2004,    // RX Safety Length not matched.
    WS_RX_RECV_WSA_SERVICE_INFO = 0x2005,    // RX too many service info in WSA
    WS_RX_RECV_WSA_CHANNEL_INFO = 0x2006,    // RX too many channel info in WSA
    WS_TX_DROP_POOR_TIME_QUALITY =0x3000,
    WS_TX_DROP_SCH_FRAMES =       0x3001,
    WS_TX_CHANNEL_NOT_MATCH     = 0x3002,    // Tx channel need to match the switching channel
    WS_TX_CHANNEL_TYPE_NOT_MATCH= 0x3003,    // Tx channel type need to match with the channel interval type
    WS_TX_NO_CCL_SERVICE        = 0x3004,    // No Tx CCL device
    WS_TX_NO_CCL_TX_QUEUE       = 0x3005,    // No Tx CCL Queue
    WS_TX_NOT_CURRENT_CHNL      = 0x3006,    // Not current channel
    WS_TX_UNSUPPORTED_PKT_TYPE  = 0x3007,    // Unsupported Pkt type
    WS_TXPROFILE_ERROR          = 0x3008,    // No Tx Profile service
    WS_TX_QUEUE_FULL            = 0x3009,    // Queue full
    WS_NO_PROVIDER_SERVICE      = 0x300A,    // No provider service
    WS_TA_REQUEST_ERROR         = 0x300B,    // TA request error
    WS_WSA_REQUEST_ERROR        = 0x300C,    // WSA request error

    /*---New (not fully implemented yet)---*/
    WS_TX_RAWP_DROPPED_PKT      = 0x400A,    // Tx Raw packet drop in NS
} Warning_ID;

/* Type definition for warning notification */
typedef struct {
    uint32_t     level;              // TX_LEVEL_MASK, RX_LEVEL_MASK, MGMT_LEVEL_MASK
    Warning_ID  warningId;          // Warning message ID
    uint32_t     optionalInfo;       // Optional information to provide with warning
    uint32_t     counter;            // Number of occurances since last report
} WarningType;

#define RS_MAX_NUM_WARNINGS 16

#define WSU_NS_IOC_BUF_SIZE 2404

typedef struct {
    rsResultCodeType risRet;
    char             str[WSU_NS_IOC_BUF_SIZE];  /* VJR WILLBEREVISISTED Must be large enough to hold the largest string reply */
} rsGetStringReplyType;

typedef struct {
    uint32_t numWarnings;
    WarningType warning[RS_MAX_NUM_WARNINGS];
} WarningIndicationType;

/* rskStatusType.error_states */
#define CV2X_INIT_FAIL                       0x1
#define CV2X_TX_NOT_READY                    0x2
#define CV2X_SERVICE_ID_BAD                  0x4
#define CV2X_TX_FAIL                         0x8
#define CV2X_RX_FAIL                        0x10
#define CV2X_SOCKET_INIT_FAIL               0x20
#define CV2X_SYS_INIT_FAIL                  0x40
#define CV2X_FW_REVISION_FAIL               0x80
#define CV2X_INIT_TWICE_WARNING            0x100
#define CV2X_WDM_SERVICE_GET_FAIL          0x200
#define CV2X_DDM_SERVICE_GET_FAIL          0x400
#define CV2X_DDM_STATE_GET_FAIL            0x800
#define CV2X_DDM_SYNC_INIT_FAIL           0x1000
#define CV2X_GET_SERVICE_FAIL             0x2000
#define CV2X_CONFIG_SET_FAIL              0x4000
#define CV2X_SERVICE_ENABLE_FAIL          0x8000
#define CV2X_L2ID_SET_FAIL               0x10000
#define CV2X_SOCKET_CREATE_FAIL          0x20000
#define CV2X_TX_DUPE_SERVICE_ID_FAIL     0x40000
#define CV2X_TX_DDM_SERVICE_GET_FAIL     0x80000
#define CV2X_TX_SOCKET_CREATE_FAIL      0x100000
#define CV2X_TX_SOCKET_POLICY_FAIL      0x200000
#define CV2X_SPS_FLOWS_USED_FAIL        0x400000
#define CV2X_TX_RADIO_NOT_READY_FAIL    0x800000
#define CV2X_DDM_TSF_GET_FAIL         0x01000000
#define CV2X_GET_TALLIES_FAIL         0x02000000
#define CV2X_PCAP_WRITE_SEM_WAIT_FAIL 0x04000000
#define CV2X_PCAP_READ_SEM_WAIT_FAIL  0x08000000
#define CV2X_PCAP_INIT_FAIL           0x10000000
#define CV2X_ALSMI_INIT_FAIL          0x20000000
#define CV2X_COMM_THREAD_FAIL         0x40000000
#define CV2X_RSK_INIT_FAIL            0x80000000

/* rskStatusType.wsm_error_states */
#define WSM_LENGTH_FAIL            0x00000001
#define WSM_LENGTH_MISMATCH_FAIL   0x00000002
#define WSM_VERSION_FAIL           0x00000004
#define WSM_PSID_NOT_REG_FAIL      0x00000008
#define WSM_PSID_ENCODING_FAIL     0x00000010
#define WSM_UNK_ELEMENT_ID_FAIL    0x00000020
#define WSM_STRIP_FAIL             0x00000040
#define WSM_VOD_FAIL               0x00000080
#define WSM_VTP_FAIL               0x00000100
#define WSM_UNK_TPID_FAIL          0x00000200
#define WSM_PEOP_FAIL              0x00000400
#define WSM_UNSEC_DROP_FAIL        0x00000800
#define WSM_VFD_FAIL               0x00001000
#define WSM_1609_P2H_DROP_FAIL     0x00002000
#define WSM_PARSE_FAIL             0x00004000
#define WSA_PROCESS_FAIL           0x00008000 
#define WSA_LENGTH_FAIL            0x00010000
#define WSA_CH_ACCESS_FAIL         0x00020000
#define WSA_PSID_FAIL              0x00040000
#define WSA_VERSION_FAIL           0x00080000
#define WSA_SRV_CNT_EXCEED_FAIL    0x00100000
#define WSA_CH_CNT_IVALID_FAIL     0x00200000
#define WSA_CH_LEN_MISMATCH_FAIL   0x00400000
#define WSA_SEC_INFO_STRIP_FAIL    0x00800000
#define WSA_SEM_FAIL               0x01000000
#define WSA_VER_SEM_FAIL           0x02000000
#define WSA_ST_ENTRY_FAIL          0x04000000
#define WSA_VERIFY_FAIL            0x08000000
#define WSA_SEC_VERIFY_HANDLE_FAIL 0x10000000
#define WSA_SEC_VER_SIG_FAIL       0x20000000
#define WSA_SEC_TYPE_FAIL          0x40000000

/* Try to create a consolidated state of everything. Available by radio call not SHM. */
typedef struct {
  int32_t  interface;
  int32_t  firmware;
  int32_t  temperature;
  int32_t  ready;
  uint32_t tx_count;
  uint32_t tx_err_count;
  uint32_t rx_count;
  uint32_t rx_err_count;
  uint32_t wme_rx_good;
  uint32_t wme_rx_bad;
  uint32_t wme_tx_not_ready;
  uint32_t SignRequests;
  uint32_t SignSuccesses;
  uint32_t SignFailures;
  uint32_t SignCBFailures;
  uint32_t LastSignErrorCode;
  uint32_t StripSuccesses;
  uint32_t StripFailures;
  uint32_t VerifyRequests;
  uint32_t VerifySuccesses;
  uint32_t VerifyFailures;
  uint32_t VerifyCBFailures;
  uint32_t LastVerifyErrorCode;
  uint32_t P2PCallbackCalled;
  uint32_t P2PCallbackSendSuccess;
  uint32_t P2PCallbackNoRadioConfigured;
  uint32_t P2PCallbackLengthError;
  uint32_t P2PCallbackSendError;
  uint32_t P2PReceived;
  uint32_t P2PProcessed;
  uint32_t P2PProcessError;
  uint32_t error_states;  /* bitfield */
  uint32_t wsm_error_states; /* bitfield */
} rskStatusType;

#endif
