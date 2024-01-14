#ifndef MK5TYPES_H_
#define MK5TYPES_H_
/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Filename: mk5types.h                                                       */
/*  Purpose: Radio Stack types/enums for QNX use with the MK5 radio.          */
/*           (Some of these emum definitions came from Cohda includes files   */
/*           that are not being used by the radio stack.)                     */
/*                                                                            */
/* Copyright (C) 2015 DENSO International America, Inc.                       */
/*                                                                            */
/* Revision History:                                                          */
/*                                                                            */
/* Date        Author        Comments                                         */
/* -------------------------------------------------------------------------- */
/* 2015-06-04  DSCPERKINS    Initial release.                                 */
/*                                                                            */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/* Include Files                                                              */
/*----------------------------------------------------------------------------*/
#include <stdint.h>
#include "llc.h"
#include <unistd.h>
#include <arpa/inet.h>
#include <if_ether.h>

/**
 * MK2 Modulation and Coding scheme
 */
typedef enum MK2MCS
{
  MK2MCS_R12BPSK  = 0xB, /// Rate 1/2 BPSK
  MK2MCS_R34BPSK  = 0xF, /// Rate 3/4 BPSK
  MK2MCS_R12QPSK  = 0xA, /// Rate 1/2 QPSK
  MK2MCS_R34QPSK  = 0xE, /// Rate 3/4 QPSK
  MK2MCS_R12QAM16 = 0x9, /// Rate 1/2 16QAM
  MK2MCS_R34QAM16 = 0xD, /// Rate 3/4 16QAM
  MK2MCS_R23QAM64 = 0x8, /// Rate 2/3 64QAM
  MK2MCS_R34QAM64 = 0xC, /// Rate 3/4 64QAM
  MK2MCS_DEFAULT  = 0x0, /// Use default data rate
  MK2MCS_TRC      = 0x1, /// Use transmit rate control
} eMK2MCS;
/// @copydoc eMK2MCS
typedef uint8_t tMK2MCS;

/**
 * MK2 Transmit Antenna
 * Indicates if manual, or automatic antenna control is to
 * be used, and a manual antenna setting if needed.
 */
typedef enum MK2TxAntenna
{
    /// Transmit packet using automatic/default transmit antenna selection.
    MK2_TXANT_DEFAULT = 0,
    /// Transmit packet on antenna 1
    MK2_TXANT_ANTENNA1 = 1,
    /// Transmit packet on antenna 2 (when available).
    MK2_TXANT_ANTENNA2 = 2,
    /// Transmit packet on both antenna
    MK2_TXANT_ANTENNA1AND2 = 3
} eMK2TxAntenna;
/// @copydoc eMK2TxAntenna
typedef uint8_t tMK2TxAntenna;

/**
 * MK2 Priority
 */
typedef enum MK2Priority
{
    /// Priority level 0
    MK2_PRIO_0 = 0,
    /// Priority level 1
    MK2_PRIO_1 = 1,
    /// Priority level 2
    MK2_PRIO_2 = 2,
    /// Priority level 3
    MK2_PRIO_3 = 3,
    /// Priority level 4
    MK2_PRIO_4 = 4,
    /// Priority level 5
    MK2_PRIO_5 = 5,
    /// Priority level 6
    MK2_PRIO_6 = 6,
    /// Priority level 7
    MK2_PRIO_7 = 7,
    /// Priority (non-QOS)
    MK2_PRIO_NON_QOS = 15,
} eMK2Priority;
/// @copydoc eMK2Priority
typedef uint8_t tMK2Priority;

/**
 * MK2 802.11 service class specification.
 */
typedef enum MK2Service
{
    /// Packet should be (was) transmitted using normal ACK policy
    MK2_QOS_ACK = 0x00,
    /// Packet should be (was) transmitted without Acknowledgement.
    MK2_QOS_NOACK = 0x01
} eMK2Service;
/// @copydoc eMK2Service
typedef uint8_t tMK2Service;

#define SNAP_HEADER_DSAP     0xAA
#define SNAP_HEADER_SSAP     0xAA
#define SNAP_HEADER_CONTROL  0x03

/// 802.2 SNAP header
typedef struct SNAPHeader
{
    uint8_t DSAP;
    uint8_t SSAP;
    uint8_t Control;
    uint8_t OUI[3];
                       /// @note network order
    uint16_t Type;
} __attribute__ ((packed)) tSNAPHeader;

/* Global Types */

/*   80211 Header Structs   */
/// MAC Address
typedef uint8_t tMACAddr[6];

/// MAC frame type
typedef enum MacFrameType
{
    /// Managment (00)
    MAC_FRAME_TYPE_MGNT = 0x0,
    /// Control (01)
    MAC_FRAME_TYPE_CONTROL = 0x1,
    /// Data (10)
    MAC_FRAME_TYPE_DATA = 0x2
} eMacFrameType;
/// @copydoc eMacFrameType
typedef uint8_t tMacFrameType;

/// MAC frame subtype
typedef enum MacFrameSubType
{
    /// Management - Action
    MAC_FRAME_SUB_TYPE_ACTION = 0xD,
    /// Data - Data (non QoS)
    MAC_FRAME_SUB_TYPE_DATA = 0x0,
    /// Data - QoS Data
    MAC_FRAME_SUB_TYPE_QOS_DATA = 0x8
} eMacFrameSubType;
/// @copydoc eMacFrameSubType
typedef uint8_t tMacFrameSubType;

/// 802.11 header frame control
typedef union Dot4FrameCtrl
{
    struct
    {
#if (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
        /// Protocol version. Currently 0
        uint16_t ProtocolVer:2;
        /// Type -00 management frame 01-control frame,10-Data frame
        uint16_t Type:2;
        /// Subtype
        uint16_t SubType:4;
        /// To the distribution system
        uint16_t ToDS: 1;
        /// Exit from the distribution system
        uint16_t FromDS: 1;
        /// more fragment frames to follow (last or unfragmented frame=0)
        uint16_t MoreFrag: 1;
        /// This re-transmission
        uint16_t Retry: 1;
        /// Station in power save mode
        uint16_t PwrMgt: 1;
        /// Additional frames buffered for the destination address
        uint16_t MoreData: 1;
        /// 1= data processed with WEP algorithm 0= no WEP
        uint16_t WEP: 1;
        /// Frames must be strictly ordered
        uint16_t Order: 1;
#else // __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
        /// Frames must be strictly ordered
        uint16_t Order :1;
        /// 1= data processed with WEP algorithm 0= no WEP
        uint16_t WEP :1;
        /// Additional frames buffered for the destination address
        uint16_t MoreData :1;
        /// Station in power save mode
        uint16_t PwrMgt :1;
        /// This re-transmission
        uint16_t Retry :1;
        /// more fragment frames to follow (last or unfragmented frame=0)
        uint16_t MoreFrag :1;
        /// Exit from the distribution system
        uint16_t FromDS :1;
        /// To the distribution system
        uint16_t ToDS :1;
        /// Subtype
        uint16_t SubType :4;
        /// Type -00 management frame 01-control frame,10-Data frame
        uint16_t Type :2;
        /// Protocol version. Currently 0
        uint16_t ProtocolVer :2;
#endif
    } Fields;

    uint16_t FrameCtrl;
} __attribute__ ((packed)) tDot4FrameCtrl;

/// 802.11 sequence control bits
typedef union Dot4SeqCtrl
{
    struct
    {
#if (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
        uint16_t FragmentNo:4; //Frame fragment number
        uint16_t SeqNo:12;     //Frame sequence number
#else // __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
        uint16_t SeqNo :12;     //Frame sequence number
        uint16_t FragmentNo :4; //Frame fragment number
#endif
    } Fields;

    uint16_t SeqCtrl;
} __attribute__ ((packed)) tDot4SeqCtrl;

/// 802.11 header QoS control
typedef union Dot4QoSCtrl
{
    struct
    {
#if (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
        /// TID
        uint16_t TID:4;
        /// EOSP
        uint16_t EOSP:1;
        /// Ack Policy
        uint16_t AckPolicy:2;
        /// Reserved
        uint16_t Reserved:1;
        /// 'TXOP Duration Requested' or 'Queue size'
        uint16_t TXOPorQueue:8;
#else // __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
        /// 'TXOP Duration Requested' or 'Queue size'
        uint16_t TXOPorQueue :8;
        /// Reserved
        uint16_t Reserved :1;
        /// Ack Policy
        uint16_t AckPolicy :2;
        /// EOSP
        uint16_t EOSP :1;
        /// TID
        uint16_t TID :4;
#endif
    } Fields;

    uint16_t QoSCtrl;
} __attribute__ ((packed)) tDot4QoSCtrl;

/// 802.11 MAC header (for QoS data frames)
typedef struct IEEE80211QoSHeader
{
    /// Frame control info
    tDot4FrameCtrl FrameControl;
    /// Duration ID, for data frames= duration of frames
    uint16_t       DurationId;
    /// SA Source address
    tMACAddr       Address1;
    /// DA Destination address
    tMACAddr       Address2;
    /// BSSID Receiving station address (destination wireless station)
    tMACAddr       Address3;
    /// Sequence control info
    tDot4SeqCtrl   SeqControl;
    /// QoS control info
    tDot4QoSCtrl   QoSControl;
} __attribute__ ((packed)) tIEEE80211QoSHeader;

/// 802.11 MAC header
typedef struct IEEE80211Header
{
  tDot4FrameCtrl FrameControl;    /// Frame control info
  uint16_t       DurationId;      /// Duration ID, for data frames= duration of frames
  uint8_t        Address1[6];     /// SA Source address
  uint8_t        Address2[6];     /// DA Destination address
  uint8_t        Address3[6];     /// BSSID Receiving station address (destination wireless station)
  tDot4SeqCtrl   SeqControl;      /// Sequence control info
} __attribute__ ((packed)) tIEEE80211Header;


/*
 * Radio Stack specific structures referencing MK5 device (DENSO)
 */

/*
 * MK5 NS Tx Stats struct
 */
typedef struct sNsTxStats
{
    uint32_t LastDataRate; // last Xmitted data rate (converted to RIS enum value)
    uint32_t LastPower;    // last Xmitted power (converted to RIS 1dB  scale)
} tNsTxStats;

/*
 * MK5 NS Rx Stats struct
 */
typedef struct sNsRxStats
{
    uint32_t      LastDataRate; // last Received data rate (converted to RIS enum value)
    uint32_t      LastPower;    // last Xmitted power (converted to RIS 1dB  scale)
    tMKxPower     LastRxPowerA;
    tMKxPower     LastRxPowerB;
    tMKxPower     LastRxNoiseA;
    tMKxPower     LastRxNoiseB;
} tNsRxStats;

/*
 * MK5 NS LLC LLC-global stats and counters
 */
typedef struct sNsllc
{
    uint32_t RxMKXBadPackets;
    uint32_t RxMKXShortPackets;
} tNsllcStatCnt;

/*
 * MK5 NS LLC per-radio stats counters
 */
typedef struct sNsllcRadioStatCnt
{
  uint32_t      RxAntNone;
  uint32_t      RxAnt1;
  uint32_t      RxAnt2;
  uint32_t      RxBroadcast;
  uint32_t      RxUnicast;

  uint32_t      TxAntNone;
  uint32_t      TxAnt1;
  uint32_t      TxAnt2;
  uint32_t      TxBroadcast;
  uint32_t      TxUnicast;

  uint32_t      TxBadChannel;    /*wsmSend requests sending WSM on Channel(freq) not configured on radio.*/
} tNsllcRadioStatCnt;

/*
 * MK5 radio state structure (netdevice)
 */
typedef struct NetDev
{
    uint32_t            SeqNum;
    tMKx               *pMKx;                           // MKx handle to MK5 radio device
    int                 Fd;                             // MKx interface file number (for use with select/poll)
    int                 Cnt;                            // Message count
    tNsTxStats          NsTxStats[2];                   // Values from NS's most recent transmission (per radio).
    tNsRxStats          NsRxStats[2];                   // Values from NS's most recent reception (per radio).
    tNsllcRadioStatCnt  NsllcRadioCnt[2];               // NS LLC tally counters (per radio).
    tNsllcStatCnt       NsllcCnt;                       // NS LLC tally counters (per LLC).
    bool_t              radioEnabled[MAX_RADIOS];       // Radio enabled/disabled state
    uint8_t             configured_channel[MAX_RADIOS]; // Saved value of the last channel each radio was configured to
    bool_t              dsrcRxThreadRunning;            // DSRC Rx thread running state
} tNetDev;

#endif /* MK5TYPES_H_ */
