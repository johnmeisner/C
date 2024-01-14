#ifndef RSK_SMH_H
#define RSK_SMH_H
/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Filename: wme.h                                                            */
/*  Purpose: Shared memory definitions for Radio Services.                    */
/*                                                                            */
/* Copyright (C) 2019 DENSO International America, Inc.  All Rights Reserved. */
/*                                                         S                   */
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
#include "../common/ns_cv2x.h"

/*----------------------------------------------------------------------------*/
/* Defines                                                                    */
/*----------------------------------------------------------------------------*/
#define RSK_SHM_PATH    "/shm_rsk"

/*----------------------------------------------------------------------------*/
/* Typedefs                                                                   */
/*----------------------------------------------------------------------------*/
typedef struct {
    bool_t   UserService;       // TRUE: userService, FALSE: Provider service
    bool_t   ServiceRunning;    // TRUE: 1 FALSE: 0
    uint16_t Pid;
    uint8_t  CurServiceIndex;   // service index entry
    uint8_t  CurChannel;        // SCH channel currently being scheduled
    uint8_t  CurPriority;       // priority of this service
    uint8_t  CurAccessType;     // AUTO_ACCESS_MATCH, AUTO_ACCESS_UNCONDITIONAL, NO_SCH_ACCESS
    uint8_t  CurProvAccessType; // 0: continuous mode, 1: alternating mode (CCH)
                                // 2: alternating mode (SCH)
    uint16_t CurExtendedAccess; // extendedAccess of this service
} CurRadioServiceType;

typedef struct {
    uint32_t GetCfgReq;
    uint32_t SetCfgReq;
    uint32_t SendWSMReq[MAX_RADIO_TYPES];
    uint32_t UnknownReq;
    uint32_t InitRadioComm;
    uint32_t TerminateRadioComm;
    uint32_t WsaSignSecurityCfm; /* VJR WILLBEREVISITED Increment in wsaSignSecurityCfm() in wme.c */
    uint32_t WsmServiceReq;
    uint32_t EUServiceReq; /* VJR WILLBEREVISITED Increment when EU is implemented */
    uint32_t UserServiceReq;
    uint32_t UserChangeServiceReq;
    uint32_t ProvServiceReq; /* VJR WILLBEREVISITED Increment when provider services are implemented */
    uint32_t VerifyWsaReq;
    uint32_t SendEUPktReq[MAX_RADIO_TYPES]; /* VJR WILLBEREVISITED Increment when EU is implemented */
    uint32_t GetIndPort;
    uint32_t TestCmd;
    uint32_t SendRawPktReq[MAX_RADIO_TYPES];
} RISReqCntType;

typedef struct {
    uint32_t AvailableService;
    uint32_t ReceiveWSMData[MAX_RADIO_TYPES];       /*Rcvd WSM Queued*/
    uint32_t ReceiveWSMDataOK[MAX_RADIO_TYPES];
    uint32_t ReceiveWSMError[MAX_RADIO_TYPES];
    uint32_t WsaSignReq; /* VJR WILLBEREVISITED Increment when sign WSA is implemented */
    uint32_t ReceiveEUData[MAX_RADIO_TYPES];        /*Rcvd EU Queued*/ /* VJR WILLBEREVISITED Increment when EU is implemented */
} RISIndCntType;

typedef struct {
    uint32_t WSMLengthInvalid;
    uint32_t WSMLengthMismatch;
    uint32_t WSMWrongVersion;
    uint32_t PSIDNotRegistered;
    uint32_t PSIDEncodingFailure;
    uint32_t WSMUnknownElementId;
    uint32_t WSMStripError;
    uint32_t WSMVerifyOnDemandError;
    uint32_t WSMVerifyThenProcessError;
    uint32_t WSMUnknownTPID;
    uint32_t WSMPrematureEndOfPacket;
    uint32_t WSMUnsecuredDropped;
    uint32_t WSMVerificationFailedDropped;
    uint32_t WSMNo1609p2HeaderDropped;
} WSMErrorType;

typedef struct {
    uint32_t PIDOK;
    uint32_t PIDNoMatchAST;
    uint32_t PIDSendFailed;
    uint32_t RTPSIDOK;
    uint32_t RTPSIDNoMatchWST;
    uint32_t RTPSIDNoMatchGST;
    uint32_t RTPSIDNoCBRegistered;
    uint32_t RTPSIDSendFailed;
} SendIndErrorType;

typedef struct {
    uint32_t SentWSMPData[MAX_RADIO_TYPES];
    uint32_t StartWSAReq; /* VJR WILLBEREVISITED Increment when Tx WSA is implemented */
    uint32_t EndWAVEReq; /* VJR WILLBEREVISITED Increment when Tx WSA is implemented */
    uint32_t SentEUData; /* VJR WILLBEREVISITED Increment when EU is implemented */
} CCLCntType;

typedef struct {
    uint32_t SentWSMPData[MAX_RADIO_TYPES];

    uint32_t StartWSAReq; /* VJR WILLBEREVISITED Increment when Tx WSA is implemented */
    uint32_t EndWAVEReq; /* VJR WILLBEREVISITED Increment when Tx WSA is implemented */
    uint32_t SentEUData; /* VJR WILLBEREVISITED Increment when EU is implemented */
    uint32_t ReceiveData;
} CCLErrorType;

typedef struct {
    uint32_t WSAReceived;
    uint32_t WSAAnalyzed;
    uint32_t WSAParseError;
    uint32_t WSAProcessError;
    uint32_t WSALengthInvalid;
    uint32_t WSAChannelAccessInvalid;
    uint32_t WSAPsidInvalid;
    uint32_t WSAWrongVersion;
    uint32_t WSAServiceCountExceeded;
    uint32_t WSAChannelCountInvalid;
    uint32_t WSAChannelLengthMismatch;
    uint32_t WSASecurityInfoStripAttempt;
    uint32_t WSASecurityInfoStripFailed;
    uint32_t WSASemaphoreError;
    uint32_t WSAVerifySemaphoreError;
    uint32_t WSANoAvailableWSAStatusTableEntry;
    uint32_t WSAVerifySuccess;
    uint32_t WSAVerifyFailed;
    uint32_t WSASecurityVerifyInvalidHandle;
    uint32_t WSASecurityVerifySignatureFailed;
    uint32_t WSASecurityUnsupportedSecurityType;
} WSACntType;

typedef struct {
    uint32_t         P2PReceived;
    uint32_t         P2PProcessed;
    uint32_t         P2PProcessError;
    int              P2PProcessLastErrorCode;
    uint32_t         P2PCallbackCalled;
    uint32_t         P2PCallbackSendSuccess;
    uint32_t         P2PCallbackNoRadioConfigured;
    uint32_t         P2PCallbackLengthError;
    uint32_t         P2PCallbackSendError;
    rsResultCodeType P2PCallbackSendLastErrorCode;
} P2PCntType;

typedef struct {
    uint32_t EUPacketsTransmitted[MAX_RADIO_TYPES]; /* VJR WILLBEREVISITED Increment when EU is implemented */
    uint32_t EUPacketsReceivedFromRadio[MAX_RADIO_TYPES]; /* VJR WILLBEREVISITED Increment when EU is implemented */
    uint32_t EUPacketsReceivedFromEthernet; /* VJR WILLBEREVISITED Increment when EU is implemented */
    uint32_t EUPacketsDiscardedFromEthernet; /* VJR WILLBEREVISITED Increment when EU is implemented */
} EUCntType;

typedef struct {
    uint32_t Register;
    uint32_t Unregister;
    uint32_t Hdr1609p2Requests;
    uint32_t Hdr1609p2Successes;
    uint32_t Hdr1609p2Failures;
    uint8_t  LastHdr1609p2ErrorCode;
    uint32_t SignRequests;
    uint32_t SignSuccesses;
    uint32_t SignFailures;
    uint32_t SignCBFailures;
    uint32_t SFUnknownPacketType;
    uint32_t SFInvalidPSID;
    uint32_t SFPayloadTooLarge;
    uint32_t SFCertificateChangeInProgress;
    uint32_t SFSignBufferOverflow;
    uint32_t SFsmg_newFailed;
    uint8_t  LastSignErrorCode;
    uint32_t StripRequests;
    uint32_t StripSuccesses;
    uint32_t StripFailures;
    uint8_t  LastStripErrorCode;
    uint32_t VerifyRequests;
    uint32_t VerifySuccesses;
    uint32_t VerifyFailures;
    uint32_t VerifyCBFailures;
    uint8_t  LastVerifyErrorCode;
    uint32_t CertChangeRequests;
    uint32_t CertChangeRequestsAccepted;
    uint32_t CertChangeRequestsRejected;
    uint8_t  CertChangeRequestLastErrorCode;
    uint32_t CertChangeSuccesses;
    uint32_t CertChangeFailures;
    uint8_t  CertChangeLastErrorCode;
    uint32_t IdChangeLockRequests;
    uint32_t IdChangeLockSuccesses;
    uint32_t IdChangeLockFailures;
    uint32_t IdChangeLockLastErrorCode;
    uint32_t IdChangeUnlockRequests;
    uint32_t IdChangeUnlockSuccesses;
    uint32_t IdChangeUnlockFailures;
    uint32_t IdChangeUnlockLastErrorCode;
    uint32_t StartCertChangeTimer;
    uint32_t CancelCertChangeTimer;
    uint32_t CertChangeRetries;
    uint32_t CertChangeTimeouts;
    uint32_t IdChangeInitCallbacks;
    uint32_t IdChangeInitZeroCerts;
    uint32_t GetAerolinkVersion;
    uint32_t GetLeapSecondsSince2004;
    uint32_t UpdatePositionAndTime;
} ALSMICntType;

typedef struct {
    uint32_t cv2xInited;
    /* Uncomment the line below if the recovery logic is added back into ns_cv2x.cpp */
//    uint32_t recoveryCnt;
    uint32_t serviceID[MAX_SPS_FLOWS];
    uint32_t onReadyGetCalledCnt;
    uint32_t onReadyGetReturnedCnt;
    uint32_t createRxSubCalledCnt;
    uint32_t rxSubCallbackCalledCnt;
} CV2XCntType;

/* Note that for now, no locking mechanism exists for shm_rsk_t. It is assumed
 * that the only other program accessing the shared memory structure is
 * v2xmonitor, and it is just doing reads. We want to avoid the overhead
 * associated with massive numbers of locks and unlocks. */
typedef struct {
    CurRadioServiceType wsuNsCurService[MAX_RADIO_TYPES][MAX_RADIOS];
    RISReqCntType       RISReqCnt;
    RISIndCntType       RISIndCnt;
    WSMErrorType        WSMError;
    SendIndErrorType    SendIndError;
    CCLCntType          CCLCnt;
    CCLErrorType        CCLError;
    WSACntType          WSACnt;
    P2PCntType          P2PCnt;
    EUCntType           EUCnt;
    ALSMICntType        ALSMICnt;
    CV2XCntType         CV2XCnt;
    uint32_t            bootupState;    // Bits 16-31:  0x1234 when radioServices is running
                                        // Bit 15:      0
                                        // Bit 14:      1 if C-V2X Rx thread is running
                                        // Bit 13:      1 if WSA timeout thread is running
                                        // Bit 12:      1 if Dummy ID change thread is running
                                        // Bit 11:      1 if ALSMI timeout thread is running
                                        // Bit 10:      1 if radioServices receive command thread is running
                                        // Bit 9:       1 if Ignore WSAs thread is running
                                        // Bit 8:       1 if DSRC Rx thread is running
                                        // Bits 0-7:    0x00 at start
                                        //              0x01 after WSA table initialized
                                        //              0x02 after ALSMI initialized
                                        //              0x03 after PCAP initialized
                                        //              0x04 after receive commands thread created
                                        //              0x05 after DSRC module initialized
                                        //              0x06 after DSRC started
                                        //              0x07 after C-V2X module initialized
                                        //              0x08 after C-V2X started
    uint32_t            connectCount;
    uint32_t            unsecureConnectCount;
    uint32_t            closeCount;
    uint32_t            txCount;
    uint32_t            rxCount;
    bool_t              txSuppressed[MAX_RADIO_TYPES]; // Set to TRUE if Tx suppressed due to LTE activity
} shm_rsk_t;

#endif // RSK_SMH_H

