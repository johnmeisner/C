#ifndef _WME_H
#define _WME_H
/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Filename: wme.h                                                            */
/*  Purpose: Include file for the WME module of the Radio Stack NS layer.     */
/*                                                                            */
/* Copyright (C) 2022 DENSO International America, Inc.  All Rights Reserved. */
/*                                                                            */
/* Revision History:                                                          */
/*                                                                            */
/* Date        Author        Comments                                         */
/* -------------------------------------------------------------------------- */
/* [2019-03-18][VROLLINGER]  Initial revision.                                */
/*                                                                            */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/* Include Files                                                              */
/*----------------------------------------------------------------------------*/
#include "ris_struct.h"

// Limits of integral types
#ifndef INT8_MIN
#define INT8_MIN               (-128)
#endif
#ifndef INT16_MIN
#define INT16_MIN              (-32767-1)
#endif
#ifndef INT32_MIN
#define INT32_MIN              (-2147483647-1)
#endif
#ifndef INT8_MAX
#define INT8_MAX               (127)
#endif
#ifndef INT16_MAX
#define INT16_MAX              (32767)
#endif
#ifndef INT32_MAX
#define INT32_MAX              (2147483647)
#endif
#ifndef UINT8_MAX
#define UINT8_MAX              (255U)
#endif
#ifndef UINT16_MAX
#define UINT16_MAX             (65535U)
#endif
#ifndef UINT32_MAX
#define UINT32_MAX             (4294967295U)
#endif
//------------------------------------------------------------------------------
// Type definitions
//------------------------------------------------------------------------------

/**
 * @section llc_remote Remote LLC Module
 *
 * LLCRemote implements a mechanism to allow the MKx Modem to be used as a
 * remote MAC to a Linux Host machine.
 *
 * @verbatim
                       Source provided -> +----------------+
   +-----------------+                    |  llc debug app |
   |   Stack / Apps  |                    +-------*--------+
   +---*-------*-----+                            |                  User Space
 ------|-------|----------------------------------|---------------------------
       | ioctl | socket(s)                        | socket         Kernel Space
     +-*-------*---+                              |
     |  simTD API  | (optional binary)            |
     +------*------+                              |
            | API (MKx_* functions)               |
     +------*------+                              |
     |  LLCRemote  +------------------------------+
     |             |<- Source code provided
     +--*-------*--+
        | USB   | Ethernet (MKxIF_* structures)
    +---*-+ +---*----+
    | USB | | TCP/IP |
    +-----+ +--------+                                         Client side (uP)
 -----------------------------------------------------------------------------
 +---------------------+ +---------------------+              Server Side (SDR)
 |        WMAC         | |   C2X Security      |
 +---------------------+ +---------------------+
 |     802.11p MAC     |
 +---------------------+
 |     802.11p PHY     |
 +---------------------+
 @endverbatim
 *
 * @subsection llc_remote_design LLCRemote MAC Design
 *
 * The LLCRemote module communicates between Server and Client via two USB bulk
 * endpoints or two UDP sockets.
 */

/// Types for the LLCRemote message transfers
typedef enum
{
  /// A null packet (to indicate no message data)
  MKXIF_NONE        = 0,
  /// A transmit packet (message data is @ref tMKxTxPacket)
  MKXIF_TXPACKET    = 1,
  /// A received packet (message data is @ref tMKxRxPacket)
  MKXIF_RXPACKET    = 2,
  /// New UTC Time (message data is @ref tMKxSetTSF)
  MKXIF_SET_TSF     = 3,
  /// Transmitted packet event (message data is @ref tMKxTxEventData)
  MKXIF_TXEVENT     = 4,
  /// Radio config for Radio A (message data is @ref tMKxRadioConfig)
  MKXIF_RADIOACFG   = 5,
  /// Radio config for Radio B (message data is @ref tMKxRadioConfig)
  MKXIF_RADIOBCFG   = 6,
  /// Radio A statistics (message data is @ref tMKxRadioStats)
  MKXIF_RADIOASTATS = 7,
  /// Radio B statistics (message data is @ref tMKxRadioStats)
  MKXIF_RADIOBSTATS = 8,
  /// Flush a single queue or all queues (message data is @ref tMKxFlushQueue)
  MKXIF_FLUSHQ      = 9,
  /// A generic debug container.
  MKXIF_DEBUG       = 10,
  /// C2XSEC message (message data is @ref tMKxC2XSec)
  MKXIF_C2XSEC      = 11,
  /// Temperature config message (message data is @ref tMKxTempConfig)
  MKXIF_TEMPCFG     = 12,
  /// Temperature measurement message (message data is @ref tMKxTemp)
  MKXIF_TEMP        = 13,
  /// Power detector config message (message data is @ref tMKxPowerDetConfig)
  MKXIF_POWERDETCFG = 14,
  /// Read the current UTC Time (message data is @ref tMKxGetTSF)
  MKXIF_GET_TSF     = 15,
  /// RSSI Cal config message (message data is @ref tMKxRSSICalConfig)
  MKXIF_RSSICALCFG  = 16,
  /// Invalid message type, used for array dimensioning
  MKXIF_COUNT       = 17,
  /// Invalid message type, used for bounds checking
  MKXIF_MAX = MKXIF_COUNT - 1
} eMKxIFMsgType;
/// @copydoc eMKxIFMsgType
typedef uint16_t tMKxIFMsgType;

/// LLCRemote message (LLC managed header)
typedef struct MKxIFMsg
{
  /// Message type
  tMKxIFMsgType Type;
  /// Length of the message, including the header itself
  uint16_t Len;
  /// Sequence number
  uint16_t Seq;
  /// Return value
  int16_t Ret;
  /// Message data
  uint8_t Data[];
} __attribute__ ((packed)) tMKxIFMsg;

/**
 * @section llc_api MKx API
 *
 * This section provides an overview of the MKx WAVE MAC usage, in order to clar
ify its
 * functionality.
 *
 * @subsection general_usage General usage in a WSM/Proprietary Protocol System 
(user-space implementation)
 *
 * Typical usage would be:
 * - Load the MKx LLC kernel module
 * - Open the MKx interface using the MKx_Init() function.
 * - Enable notifications by setting the pMKx->API.Callbacks.NotifInd() callback
 * - Enable packet reception by setting the pMKx->API.Callbacks.RxAlloc()
 *    pMKx->API.Callbacks.RxInd() and callbacks
 * - Enable transmit confirmations by setting the pMKx->API.Callbacks.TxCnf() ca
llback
 * - Set the Radio A (CCH & SCH-A) parameters using the MKx_Config() function.
 * - Set the Radio B (CCH & SCH-B) parameters using the MKx_Config() function.
 * - Packets can be transmitted using the TxReq() function and the success/failu
re
 *    of the frame is indicated via the TxCnf() callback
 * - Packets received on either radio will be allocated with the RxAlloc()
 *    callback and delivered via the RxInd() callback
 * - When done, the MKx interface can be gracefully closed with MKx_Exit()
 *
 * @subsection channel_measurements Channel Measurements
 * - Statistics are updates are notified via the NotifInd() callback every 50ms
 * - Counters can be read directly from the MKx handle or using the MKx_GetStats
() helper function
 *   - Channel busy ratio is provided in the per-channel statistics.
 *     This is the ratio of channel busy (virtual carrier sense is asserted)
 *     time to channel idle time.
 *     It is an 8-bit unsigned value, where 100% channel utilisation is indicate
d by a value of 255.
 *   - Average idle period power is provided in the per-channel statistics.
 *     This is the average RSSI recorded whilst the channel isn't busy (virtual 
carrier sense is not asserted).
 *
 * @subsection dual_channel_operation  Dual channel operation
 * When operating in a dual-radio configuration, it is possible to configure the
 MAC channel
 * access function to consider the state of the other radio channel before makin
g transmit
 * decisions. The WMAC allows the following configuration options for the channe
l access
 * function when operating in a dual-radio system.
 *
 * - No consideration of other radio. In this case, the radio will transmit with
out regard to
 * the state of the other radio channel. The system will behave effectively as t
wo
 * independent radio systems.
 * - Tx inhibit. In this mode, the MAC will prevent this radio from transmitting
 while the
 * other radio is transmitting. In this case, when the other radio is transmitti
ng, the local
 * radio behaves as if the local channel is busy.
 * - Rx inhibit. In this mode, the MAC will prevent this radio from transmitting
 while the
 * other radio is actively receiving a frame. In this case, when the other radio
 is receiving,
 * the local radio behaves as if the local channel is busy. This prevents transm
issions from
 * this radio from corrupting the reception of a frame on the other radio, tuned
 to a nearby
 * radio channel (in particular when shared or co-located antennas are in use).
 * - TxRx inhibit. In this mode, the MAC will prevent this radio from transmitti
ng while
 * the other radio is either transmitting or receiving.
 *
 * In all cases, the transmission inhibit occurs at the MAC channel-access level
, so packets will
 * not be dropped when transmission is inhibited, they will simply be deferred.
 *
 */

/// Forward declaration of the MKx Handle
struct MKx;

/// MKx MLME interface return codes
typedef enum
{
  /// Success return code
  MKXSTATUS_SUCCESS = 0,
  // -1 to -255 reserved for @c errno values (see <errno.h>)
  /// Unspecified failure return code (catch-all)
  MKXSTATUS_FAILURE_INTERNAL_ERROR              = -256,
  /// Failure due to invalid MKx Handle
  MKXSTATUS_FAILURE_INVALID_HANDLE              = -257,
  /// Failure due to invalid RadioConfig
  MKXSTATUS_FAILURE_INVALID_CONFIG              = -258,
  /// Failure due to invalid parameter setting
  MKXSTATUS_FAILURE_INVALID_PARAM               = -260,
  /// Auto-cal requested when radio is running auto-cal
  MKXSTATUS_FAILURE_AUTOCAL_REJECT_SIMULTANEOUS = -261,
  /// Auto-cal requested but radio is not configured
  MKXSTATUS_FAILURE_AUTOCAL_REJECT_UNCONFIGURED = -262,
  /// Radio config failed (likely to be a hardware fault) maximum
  MKXSTATUS_FAILURE_RADIOCONFIG_MAX             = -513,
  /// Radio config failed (likely to be a hardware fault) minimum
  MKXSTATUS_FAILURE_RADIOCONFIG_MIN             = -768,
  /// Packet failed by exceeding Time To Live
  MKXSTATUS_TX_FAIL_TTL                         = -769,
  /// Packet failed by exceeding Max Retry count
  MKXSTATUS_TX_FAIL_RETRIES                     = -770,
  /// Packet failed because queue was full
  MKXSTATUS_TX_FAIL_QUEUEFULL                   = -771,
  /// Packet failed because requested radio is not present
  MKXSTATUS_TX_FAIL_RADIO_NOT_PRESENT           = -772,
  /// Packet failed because the frame was malformed
  MKXSTATUS_TX_FAIL_MALFORMED                   = -773,
  /// Packet failed because requested radio is not present
  MKXSTATUS_TX_FAIL_RADIO_UNCONFIGURED          = -774,
  /// Packet failed because it was too long
  MKXSTATUS_TX_FAIL_PACKET_TOO_LONG             = -775,
  /// Security message failed due to security accelerator not being present
  MKXSTATUS_SECURITY_ACCELERATOR_NOT_PRESENT    = -1024,
  /// Security message failed due to security FIFO being full
  MKXSTATUS_SECURITY_FIFO_FULL                  = -1025,
  // Reserved
  MKXSTATUS_RESERVED                            = 0xC0DA
} eMKxStatus;
/// @copydoc eMKxStatus
typedef int tMKxStatus;

/// MKx Radio
typedef enum
{
  /// Selection of Radio A of the MKX
  MKX_RADIO_A = 0,
  /// Selection of Radio B of the MKX
  MKX_RADIO_B = 1,
  // ...
  /// Used for array dimensioning
  MKX_RADIO_COUNT = 2,
  /// Used for bounds checking
  MKX_RADIO_MAX = MKX_RADIO_COUNT - 1
} eMKxRadio;
/// @copydoc eMKxRadio
typedef int8_t tMKxRadio;

/// MKx Channel
typedef enum
{
  /// Indicates Channel Config 0 is selected
  MKX_CHANNEL_0 = 0,
  /// Indicates Channel Config 1 is selected
  MKX_CHANNEL_1 = 1,
  // ...
  /// Used for array dimensioning
  MKX_CHANNEL_COUNT = 2,
  /// Used for bounds checking
  MKX_CHANNEL_MAX = MKX_CHANNEL_COUNT - 1

} eMKxChannel;
/// @copydoc eMKxChannel
typedef int8_t tMKxChannel;

/// MKx Bandwidth
typedef enum
{
  /// Indicates 10 MHz
  MKXBW_10MHz = 10,
  /// Indicates 20 MHz
  MKXBW_20MHz = 20
} eMKxBandwidth;
/// @copydoc eMKxBandwidth
typedef int8_t tMKxBandwidth;

/// The channel's centre frequency [MHz]
typedef uint16_t tMKxChannelFreq;

/**
 * MKx dual radio transmit control
 * Controls transmit behaviour according to activity on the
 * other radio (inactive in single radio configurations)
 */
typedef enum
{
  /// Do not constrain transmissions
  MKX_TXC_NONE,
  /// Prevent transmissions when other radio is transmitting
  MKX_TXC_TX,
  /// Prevent transmissions when other radio is receiving
  MKX_TXC_RX,
  /// Prevent transmissions when other radio is transmitting or receiving
  MKX_TXC_TXRX,
  /// Default behaviour
  MKX_TXC_DEFAULT = MKX_TXC_TX
} eMKxDualTxControl;
/// @copydoc eMKxDualTxControl
typedef uint8_t tMKxDualTxControl;

/**
 * MKx Modulation and Coding scheme
 */
typedef enum
{
  /// Rate 1/2 BPSK
  MKXMCS_R12BPSK = 0xB,
  /// Rate 3/4 BPSK
  MKXMCS_R34BPSK = 0xF,
  /// Rate 1/2 QPSK
  MKXMCS_R12QPSK = 0xA,
  /// Rate 3/4 QPSK
  MKXMCS_R34QPSK = 0xE,
  /// Rate 1/2 16QAM
  MKXMCS_R12QAM16 = 0x9,
  /// Rate 3/4 16QAM
  MKXMCS_R34QAM16 = 0xD,
  /// Rate 2/3 64QAM
  MKXMCS_R23QAM64 = 0x8,
  /// Rate 3/4 64QAM
  MKXMCS_R34QAM64 = 0xC,
  /// Use default data rate
  MKXMCS_DEFAULT = 0x0,
  /// Use transmit rate control (currently unused)
  MKXMCS_TRC = 0x1
} eMKxMCS;
/// @copydoc eMKxMCS
typedef uint8_t tMKxMCS;

/// Tx & Rx power of frame, in 0.5dBm units.
typedef enum
{
  /// Selects the PHY maximum transmit power
  MKX_POWER_TX_MAX      = INT16_MAX,
  /// Selects the PHY minimum transmit power
  MKX_POWER_TX_MIN      = INT16_MIN,
  /// Selects the PHY default transmit power level
  MKX_POWER_TX_DEFAULT  = MKX_POWER_TX_MIN + 1,
  /// Indicates when the Rx power reported is invalid as antenna is disabled
  MKX_POWER_RX_DISABLED = INT16_MIN
} eMKxPower;
/// @copydoc eMKxPower
typedef int16_t tMKxPower;

/**
 * MKx Antenna Selection
 */
typedef enum
{
  /// Transmit packet on neither antenna (dummy transmit)
  MKX_ANT_NONE = 0,
  /// Transmit packet on antenna 1
  MKX_ANT_1 = 1,
  /// Transmit packet on antenna 2 (when available).
  MKX_ANT_2 = 2,
  /// Transmit packet on both antenna
  MKX_ANT_1AND2 = MKX_ANT_1 | MKX_ANT_2,
  /// Selects the default (ChanConfig) transmit antenna setting
  MKX_ANT_DEFAULT = 4
} eMKxAntenna;
/// Number of antennas that are present for the MKX
#define MKX_ANT_COUNT 2
/// @copydoc eMKxAntenna
typedef uint8_t tMKxAntenna;

/**
 * MKx TSF
 * Indicates absolute 802.11 MAC time in microseconds
 */
typedef uint64_t tMKxTSF;

/**
 * MKx Rate sets
 * Each bit indicates if corresponding MCS rate is supported
 */
typedef enum
{
  /// Rate 1/2 BPSK rate mask
  MKX_RATE12BPSK_MASK = 0x01,
  /// Rate 3/4 BPSK rate mask
  MKX_RATE34BPSK_MASK = 0x02,
  /// Rate 1/2 QPSK rate mask
  MKX_RATE12QPSK_MASK = 0x04,
  /// Rate 3/4 QPSK rate mask
  MKX_RATE34QPSK_MASK = 0x08,
  /// Rate 1/2 16QAM rate mask
  MKX_RATE12QAM16_MASK = 0x10,
  /// Rate 2/3 64QAM rate mask
  MKX_RATE23QAM64_MASK = 0x20,
  /// Rate 3/4 16QAM rate mask
  MKX_RATE34QAM16_MASK = 0x40
} eMKxRate;
/// @copydoc eMKxRate
typedef uint8_t tMKxRate;

/**
 * MKx 802.11 service class specification.
 */
typedef enum
{
  /// Packet should be (was) transmitted using normal ACK policy
  MKX_QOS_ACK = 0x00,
  /// Packet should be (was) transmitted without Acknowledgement.
  MKX_QOS_NOACK = 0x01
} eMKxService;
/// @copydoc eMKxService
typedef uint8_t tMKxService;

/**
 * MKx Additional MAC layer tx control
 * These bits signal to the radio that special Tx behavior is required
 */
typedef enum
{
  /// Do not modify the sequence number field
  MKX_DISABLE_MAC_HEADER_UPDATES_SEQCTRL    = 0x01,
  /// Do not modify the duration ID field
  MKX_DISABLE_MAC_HEADER_UPDATES_DURATIONID = 0x02,
  /// Do not modify the Ack Policy field
  MKX_DISABLE_MAC_HEADER_UPDATES_ACKPOLICY  = 0x04,
  /// Do not modify the Retry field and set Max retries to zero
  MKX_DISABLE_MAC_HEADER_UPDATES_RETRY      = 0x08,
  /// Force the use of RTS/CTS with this packet
  MKX_FORCE_RTSCTS                          = 0x10
} eMKxTxCtrlFlags;
/// @copydoc eMKxTxCtrlFlags
typedef uint8_t tMKxTxCtrlFlags;

/**
 * MKx Transmit Descriptor. This header is used to control how the data packet
 * is transmitted by the LLC. This is the header used on all transmitted
 * packets.
 */
typedef struct MKxTxPacketData
{
  /// Indicate the radio that should be used (Radio A or Radio B)
  tMKxRadio RadioID;
  /// Indicate the channel config for the selected radio
  tMKxChannel ChannelID;
  /// Indicate the antennas upon which packet should be transmitted
  /// (may specify default)
  tMKxAntenna TxAntenna;
  /// Indicate the MCS to be used (may specify default)
  tMKxMCS MCS;
  /// Indicate the power to be used (may specify default)
  tMKxPower TxPower;
  /// Additional control over the transmitter behaviour (must be set to zero
  /// for normal operation)
  tMKxTxCtrlFlags TxCtrlFlags;
  // Reserved (for 64 bit alignment and internal processing)
  uint8_t Reserved0;
  /// Indicate the expiry time as an absolute MAC time in microseconds
  /// (0 means never)
  tMKxTSF Expiry;
  /// Length of the frame (802.11 Header + Body, not including FCS)
  uint16_t TxFrameLength;
  // Reserved (for 32 bit alignment and internal processing)
  uint16_t Reserved1;
  /// Frame (802.11 Header + Body, not including FCS)
  uint8_t TxFrame[];
} __attribute__((__packed__)) tMKxTxPacketData;

/**
 * Transmit Event Data. This is the structure of the data field for
 * MKxIFMsg messages of type TxEvent.
 */
typedef struct MKxTxEventData
{
  /// Transmit status (transmitted/retired), @ref eMKxStatus
  int16_t TxStatus;
  /// 802.11 MAC sequence number of the transmitted frame
  uint16_t MACSequenceNumber;
  /// The TSF when the packet was transmitted or retired
  tMKxTSF TxTime;
  /// Delay (VDSP ticks) between end of Tx Data frame and start of Rx Ack frame
  /// Note VDSP Clock runs at 300MHz
  uint32_t AckResponseDelay;
  /// Delay (VDSP ticks) between end of Tx RTS frame and start of Rx CTS frame
  /// Note VDSP Clock runs at 300MHz
  uint32_t CTSResponseDelay;
  /// Time (us) between the arrival of the packet at the MAC and its Tx
  uint32_t MACDwellTime;
  /// Short packet retry counter
  uint8_t NumShortRetries;
  /// Long packet retry counter
  uint8_t NumLongRetries;
  /// Destination address of the transmitted frame
  uint8_t DestAddress[6];
} __attribute__((__packed__)) tMKxTxEventData;

/**
 * MKx Transmit Event format.
 */
typedef struct MKxTxEvent
{
  /// Interface Message Header
  tMKxIFMsg Hdr;
  /// Tx Event Data
  tMKxTxEventData TxEventData;
} __attribute__((__packed__)) tMKxTxEvent;

/**
 * MKx Meta Data type - contains per frame receive meta-data
 *
 * The frequency offset estimate is the measured offset with respect to the
 * local oscillator frequency, which is accurate to +/- 10ppm.
 */
typedef struct MKxRxMeta
{
  /// Estimated frequency offset of rx frame in Hz (with respect to local freq)
  int32_t FreqOffset;
} __attribute__((__packed__)) tMKxRxMeta;

/**
 * MKx Receive descriptor and frame.
 * This header is used to pass receive packet meta-information from
 * the LLC to upper-layers. This header is prepended to all received packets.
 * If only a single receive  power measure is required, then simply take the
 * maximum power of Antenna A and B.
 */
typedef struct MKxRxPacketData
{
  /// Indicate the radio that should be used (Radio A or Radio B)
  tMKxRadio RadioID;
  /// Indicate the channel config for the selected radio
  tMKxChannel ChannelID;
  /// Indicate the data rate that was used
  tMKxMCS MCS;
  // Indicates FCS passed for received frame (1=Pass, 0=Fail)
  uint8_t FCSPass;
  /// Indicate the received power on Antenna A
  tMKxPower RxPowerA;
  /// Indicate the received power on Antenna B
  tMKxPower RxPowerB;
  /// Indicate the receiver noise on Antenna A
  tMKxPower RxNoiseA;
  /// Indicate the receiver noise on Antenna B
  tMKxPower RxNoiseB;
  /// Per Frame Receive Meta Data
  tMKxRxMeta RxMeta;
  /// MAC Rx Timestamp, local MAC TSF time at which packet was received
  tMKxTSF RxTSF;
  /// Length of the Frame (802.11 Header + Body, including FCS)
  uint16_t RxFrameLength;
  // Reserved (for 32 bit alignment)
  uint16_t Reserved1;
  /// Frame (802.11 Header + Body, including FCS)
  uint8_t RxFrame[];
} __attribute__((__packed__)) tMKxRxPacketData;


/*----------------------------------------------------------------------------*/
/* Typedefs                                                                   */
/*----------------------------------------------------------------------------*/
typedef enum {
    APP_UNINITIALIZED,
    APP_INITIALIZED,
    APP_REGISTERED,
    APP_ACTIVE,
} AppStatusType;

typedef struct {
    pid_t         pid;
    AppStatusType AppStatus;
    uint8_t       AppPriority;
    bool_t        serviceAvailableInd;
    bool_t        receiveWSMInd;
    int           index;
} ASTEntryType; /* Registered application status */

typedef struct {
    uint8_t      ASTCount; // 0..MAX_APPS
    ASTEntryType ASTEntry[MAX_APPS];
} ApplicationStatusTableType;

typedef struct {
    uint8_t           mgmtId;
    uint8_t           repeat;      /* timing advertisement repeat rate ( 0 -255)
                                    * The number of timing frames to be transmitted per 5 seconds. */
    uint32_t          channelType; /* 0:WSU_CCL_CHNL_TYPE_CTRL, 1:WSU_CCL_CHNL_TYPE_SERV, 2:WSU_CCL_CHNL_TYPE_ALL */
    rsRadioType       radioType;
    sendWSMStruct_S   sendWSMStruct;
    /* 20210515 (JJG): no clue if the following are necessary; BUT these
       were added to outWSMType (see ris_struct.h) when ported to Hercules;
       adding to accomodate xfer issues with WSAs porting for 5912
       (fields added: isBitmappedSsp; sspLen; sspMask; ssp
    */
    bool_t            isBitmappedSsp;    // TRUE if bitmapped SSP
    uint8_t           sspLen;            // Length of the SSP data
    uint8_t           sspMask[MAX_SSP_DATA]; // Used if bitmasked SSP
    uint8_t           ssp[MAX_SSP_DATA]; // SSP data
    uint16_t          wsaLen;
    /* JJG: this is MANDATORY; screw with this at your own risk */
    uint8_t           hdrs[HDRS_LEN];    // Allow room for various headers. See
                                         // definition for HDRS_LEN for details.
    uint8_t           wsaBuf[MAX_WSA_DATA];
} vru_wsa_req;

/* Is there a logical way to reuse this for CV2X? */
#define DSRC_MAX_RATES 11
#define DSRC_MAX_AC    4

/* A DSRC rate set */
typedef struct {
    uint8_t len;
    uint8_t rates[DSRC_MAX_RATES];
} dsrc_rateset_t;

/* The DSRC EDCA parameters for one access category */
typedef struct {
    uint8_t  aci_aifsn;
    uint8_t  ecw_min_max;
    uint16_t txop_limit;
} dsrc_edca_param_t;

/* The set of DSRC EDCA parameters for all access categories */
typedef struct {
    uint8_t           eid;
    uint8_t           len;
    uint8_t           qos_info;
    uint8_t           reserved;
    dsrc_edca_param_t edca_params[DSRC_MAX_AC];
} dsrc_edca_param_set_t;

/* A DSRC radio configuration */
typedef struct {
    uint8_t               channel;
    dsrc_rateset_t        operationalRateSet;
    dsrc_edca_param_set_t edca_params;
    uint8_t               immediateAccess; /* immediate access to SCH 1: true 0: false */
    uint16_t              extendedAccess;  /* continuous access to SCH 0-65535 */
} dsrc_sch_service_req_t;
/*----------------------------------------------------------------------------*/
/* Externs                                                                    */
/*----------------------------------------------------------------------------*/
extern ApplicationStatusTableType   AST;
extern bool_t                       cv2x_started;
extern bool_t                       dsrc_started;
extern bool_t                       vodEnabled;
extern bool_t                       ignoreWSAs;
extern rsRadioType                  lastVerifyRadioType;

/*----------------------------------------------------------------------------*/
/* Function Prototypes                                                        */
/*----------------------------------------------------------------------------*/
rsResultCodeType providerServiceReq(ProviderServiceType *service);
rsResultCodeType wsaVerifySecurityCfm(AEROLINK_RESULT returnCode, uint32_t signedWsaHandle);
rsResultCodeType initRPS(rsRPSMsgType *req);
rsResultCodeType releaseRPS(rsRPSMsgType *req);
rsResultCodeType userServiceReq(UserServiceType *service);
rsResultCodeType wsmServiceReq(WsmServiceType *service);
rsResultCodeType sendWSM(outWSMType *WSM);
int getWBSSInfo(char *buf, uint32_t buf_size);
rsResultCodeType getIndPort(pid_t pid, uint16_t *port);
int start_cv2x(void);
void stop_cv2x(void);

rsResultCodeType sendWSMActual(sendWSMStruct_S *sendWSMStruct,
                                      uint8_t *wsm, uint16_t wsmLength);
void sendIndicationToRis(uint32_t psid, rsReceiveDataType *recv);
void sendIndicationToRisByPid(pid_t pid, rsReceiveDataType *recv);
void sendIndicationToRisByRadioTypeAndPsid(rsRadioType radioType,
                                                  uint32_t psid,
                                                  rsReceiveDataType *recv,
                                                  bool_t wsm);
uint8_t wsuWMEP2PCallback(void *userData, uint32_t pduLength,
                                 uint8_t *pdu);
int getWsaInfo(char *buf, uint32_t buf_size, int wsa_num);
int getWsaServiceInfo(char *buf, uint32_t buf_size, int wsa_num, int service_num);
int getWsaChannelInfo(char *buf, uint32_t buf_size, int wsa_num, int channel_idx);
int getWsaWraInfo(char *buf, uint32_t buf_size, int wsa_num);
rsResultCodeType verifyWSA(uint32_t signedWsaHandle, AEROLINK_RESULT *vodResult);
void verifyWSACfm(AEROLINK_RESULT vodResult, uint32_t signedWsaHandle);
void initWSAStatusTable(void);
void deinitWSAStatusTable(void);
rsResultCodeType changeUserServiceReq(UserServiceChangeType *change);
void *ignoreWSAsThread(void *ptr);
rsResultCodeType sendRawPacket(outRawType *packet);
void wsaSignSecurityCfm(AEROLINK_RESULT    returnCode,
                        uint8_t           *wsaSignedData,
                        uint32_t           wsaSignedDataLength,
                        wsaTxSecurityType *wsaTxSecurity);

#endif  /*_WME_H*/

