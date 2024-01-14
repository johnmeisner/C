/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Filename: nsstats.c                                                        */
/*  Purpose: Utility to display information about radioServices               */
/*                                                                            */
/* Copyright (C) 2019 DENSO International America, Inc.  All Rights Reserved. */
/*                                                                            */
/* Revision History:                                                          */
/*                                                                            */
/* Date        Author        Comments                                         */
/* -------------------------------------------------------------------------- */
/* 2019-03-22  VROLLINGER    Initial revision.                                */
/*                                                                            */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/* Include Files                                                              */
/*----------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ipcsock.h"
#include "wsu_sharedmem.h"
#include "shm_rsk.h"
#include "rs.h"

/*----------------------------------------------------------------------------*/
/* Macros                                                                     */
/*----------------------------------------------------------------------------*/
#define USAGE "\
\nUsage: %s [OPTION] [COUNT]\n \
\n\tList NS information (help page (-h) by default) \
\n\tOptions: \
\n\t[-c]\tList NS configuration information \
\n\t[-h]\tPrint this help page \
\n\t[-m]\tList NS MIB status information \
\n\t[-r]\tPrint NS event counts and errors report \
\n\t[-w]\tList NS WBSS status information \
\n\t[-x]\tList NS WSA information \
\n\tCount (0..) \
\n\t0 means forever; 1 is default, if not specified \
\n\n", argv[0]

/* Note: nsstats is a radioServices client and does not use the RIS API. */

/*----------------------------------------------------------------------------*/
/* Local Variables                                                            */
/*----------------------------------------------------------------------------*/
static int radioServicesSock = -1; /* radioServices socket*/

/*------------------------------------------------------------------------------
** Function:  communicateWithRadioServices
** @brief  Send a message to Radio Services and get its reply.
** @param  msg       -- [input]Message to send to Radio Services
** @param  size      -- [input]Size of the message
** @param  reply     -- [output]Buffer to hold the received reply
** @param  replySize -- [input]Size of the reply buffer
** @return Size of the message received for success; -1 for error. Will never
**         return 0.
**
** Details: If it hasn't been done previously, creates the socket to
**          communicate with Radio Services. Sends a message to Radio Services
**          via wsuSendData() using port RS_RECV_PORT. Blocks waiting  for areply via
**          reply via wsuReceiveData().
**
**          Error messages are printed (maximum of 10 times) on errors of
**          socket creating, sending the message, or receiving the message.
**----------------------------------------------------------------------------*/
static int32_t communicateWithRadioServices(void *msg, size_t size, void *reply,
                                            size_t replySize)
{
    RcvDataType rcvData;
    size_t      rcvSize;

    if (radioServicesSock < 0) {
        radioServicesSock = wsuConnectSocket(-1 /* port ignored */);

        if (radioServicesSock < 0) {
            /* Connect failed */
            printf("nsstats %s: wsuConnectSocket() failed\n", __func__);
            return -1;
        }
    }

    /* Send the message to Radio Services */
    if (!wsuSendData(radioServicesSock, RS_RECV_PORT, msg, size)) {
        /* Send failed */
        printf("nsstats %s: wsuSendData() failed\n", __func__);
        return -1;
    }

    /* Wait for a response */
    while (1) {
        /* Get the reply to the message */
        rcvData.available = FALSE;
        rcvData.fd        = radioServicesSock;
        rcvData.data      = reply;
        rcvData.size      = replySize;

        rcvSize = wsuReceiveData(/* Timeout ignored */0, &rcvData);

        if (rcvSize > 0) {
            /* We received data. In this case, there is no way
             * rcvData.available could have been set to FALSE. */
            break;
        }

        if (rcvSize < 0) {
            /* Receive failed */
            printf("wsuReceiveData() failed\n");
            return -1;
        }

        /* If rcvSize is 0, then continue in the loop and try again. A return
         * value of 0 is something that can occur normally with datagram
         * sockets; wsuConnectSocket() specifies SOCK_DGRAM on its call to
         * wsuSendData(). Also, a return value of 0 does NOT indicate the
         * socket was closed. wsuReceiveData() will also return 0 if errno is
         * set to EINTR upron return upon return from recvfrom(); in this case,
         * you should try again. */
    }

    return rcvSize;
}

void nsstats_c(void)
{
    rsIoctlType                 cmd = WSU_NS_GET_CFG_INFO;
    static rsGetStringReplyType reply;
    int32_t                     size =
        communicateWithRadioServices(&cmd, sizeof(cmd), &reply, sizeof(reply));

    if (size < 0) {
        printf("nsstats %s: Error getting configuration info\n", __func__);
    }
    else {
        printf("%s", reply.str);
    }
}

void nsstats_w(void)
{
    rsIoctlType                 cmd = WSU_NS_GET_WBSS_STS;
    static rsGetStringReplyType reply;
    int32_t                     size =
        communicateWithRadioServices(&cmd, sizeof(cmd), &reply, sizeof(reply));

    if (size < 0) {
        printf("nsstats %s: Error getting WBSS info\n", __func__);
    }
    else {
        printf("%s", reply.str);
    }
}

void nsstats_r(void)
{
    shm_rsk_t *rskShmPtr;
    int i, j;

    /* Mount Radio_ns shared memory */
    rskShmPtr = wsu_share_init(sizeof(shm_rsk_t), RSK_SHM_PATH);

    if (rskShmPtr == NULL) {
        fprintf(stderr, "Could not Allocate Radio_ns Shared Memory\n");
        return;
    }

    /* Print the statistics */
    printf("== RIS Messages == \t\tCounts\t\t\t\t\t\tCounts\n");
    printf("Init Radio\t\t\t%-10d\tTerm Radio\t\t\t%-10d\n", rskShmPtr->RISReqCnt.InitRadioComm, rskShmPtr->RISReqCnt.TerminateRadioComm);
    printf("Get Cfg\t\t\t\t%-10d\tSet Cfg\t\t\t\t%-10d\n", rskShmPtr->RISReqCnt.GetCfgReq, rskShmPtr->RISReqCnt.SetCfgReq);
    printf("Send WSM (C-V2X)\t\t%-10d\tSend EU (C-V2X)\t\t\t%-10d\n", rskShmPtr->RISReqCnt.SendWSMReq[RT_CV2X], rskShmPtr->RISReqCnt.SendEUPktReq[RT_CV2X]);
    printf("Send WSM (DSRC)\t\t\t%-10d\tSend EU (DSRC)\t\t\t%-10d\n", rskShmPtr->RISReqCnt.SendWSMReq[RT_DSRC], rskShmPtr->RISReqCnt.SendEUPktReq[RT_DSRC]);
    printf("WSA Sign Confirm\t\t%-10d\n", rskShmPtr->RISReqCnt.WsaSignSecurityCfm);
    printf("Wsm Service Request\t\t%-10d\tEU Service Request\t\t%-10d\n", rskShmPtr->RISReqCnt.WsmServiceReq, rskShmPtr->RISReqCnt.EUServiceReq);
    printf("User Service Request\t\t%-10d\tProvider Service Request\t%-10d\n", rskShmPtr->RISReqCnt.UserServiceReq, rskShmPtr->RISReqCnt.ProvServiceReq);
    printf("Get Indication Port\t\t%-10d\tTest Command\t\t\t%-10d\n", rskShmPtr->RISReqCnt.GetIndPort, rskShmPtr->RISReqCnt.TestCmd);
    printf("Send Raw Packet (C-V2X)\t\t%-10d\tSend Raw Packet (DSRC)\t\t%-10d\n", rskShmPtr->RISReqCnt.SendRawPktReq[RT_CV2X], rskShmPtr->RISReqCnt.SendRawPktReq[RT_DSRC]);

    printf("Receive WSM Data Ind (C-V2X)\t%-10d\tReceive EU Data Ind (C-V2X)\t%-10d\n", rskShmPtr->RISIndCnt.ReceiveWSMData[RT_CV2X], rskShmPtr->RISIndCnt.ReceiveEUData[RT_CV2X]);
    printf("Receive WSM OK (C-V2X)\t\t%-10d\tReceive WSM Error (C-V2X)\t%-10d\n", rskShmPtr->RISIndCnt.ReceiveWSMDataOK[RT_CV2X], rskShmPtr->RISIndCnt.ReceiveWSMError[RT_CV2X]);
    printf("Receive WSM Data Ind (DSRC)\t%-10d\tReceive EU Data Ind (DSRC)\t%-10d\n", rskShmPtr->RISIndCnt.ReceiveWSMData[RT_DSRC], rskShmPtr->RISIndCnt.ReceiveEUData[RT_DSRC]);
    printf("Receive WSM OK (DSRC)\t\t%-10d\tReceive WSM Error (DSRC)\t%-10d\n", rskShmPtr->RISIndCnt.ReceiveWSMDataOK[RT_DSRC], rskShmPtr->RISIndCnt.ReceiveWSMError[RT_DSRC]);
    printf("Initiate WSA Sign Request\t%-10d\tInitiate WSA Verify Request\t%-10d\n", rskShmPtr->RISIndCnt.WsaSignReq, rskShmPtr->RISReqCnt.VerifyWsaReq);

    printf("\n== CCL Calls ==\t\t\tCounts\t\tErrors\n");
    printf("Send WSMP Data (C-V2X)\t\t%-10d\t%-10d\n",  rskShmPtr->CCLCnt.SentWSMPData[RT_CV2X], rskShmPtr->CCLError.SentWSMPData[RT_CV2X]);
    printf("Send WSMP Data (DSRC)\t\t%-10d\t%-10d\n",  rskShmPtr->CCLCnt.SentWSMPData[RT_DSRC], rskShmPtr->CCLError.SentWSMPData[RT_DSRC]);
    printf("Start WSA Request\t\t%-10d\t%-10d\n", rskShmPtr->CCLCnt.StartWSAReq,  rskShmPtr->CCLError.StartWSAReq);
    printf("End WAVE Request\t\t%-10d\t%-10d\n\n",rskShmPtr->CCLCnt.EndWAVEReq,   rskShmPtr->CCLError.EndWAVEReq);

    printf("WSM Length Invalid:\t\t\t\t%-10d\n", rskShmPtr->WSMError.WSMLengthInvalid);
    printf("WSM Length Mismatch:\t\t\t\t%-10d\n", rskShmPtr->WSMError.WSMLengthMismatch);
    printf("WSM Wrong Version:\t\t\t\t%-10d\n", rskShmPtr->WSMError.WSMWrongVersion);
    printf("WSM PSID Not Registered:\t\t\t%-10d\n", rskShmPtr->WSMError.PSIDNotRegistered);
    printf("WSM PSID Encoding failure:\t\t\t%-10d\n", rskShmPtr->WSMError.PSIDEncodingFailure);
    printf("WSM Unknown WSM Element Id:\t\t\t%-10d\n", rskShmPtr->WSMError.WSMUnknownElementId);
    printf("WSM Strip Error:\t\t\t\t%-10d\n", rskShmPtr->WSMError.WSMStripError);
    printf("WSM VOD Error:\t\t\t\t\t%-10d\n", rskShmPtr->WSMError.WSMVerifyOnDemandError);
    printf("WSM VTP Error:\t\t\t\t\t%-10d\n", rskShmPtr->WSMError.WSMVerifyThenProcessError);
    printf("WSM Unknown TPID:\t\t\t\t%-10d\n", rskShmPtr->WSMError.WSMUnknownTPID);
    printf("WSM Unsecured Dropped:\t\t\t\t%-10d\n", rskShmPtr->WSMError.WSMUnsecuredDropped);
    printf("WSM Verification Failed Dropped:\t\t%-10d\n", rskShmPtr->WSMError.WSMVerificationFailedDropped);
    printf("WSM No 1609.2 Header Dropped:\t\t\t%-10d\n", rskShmPtr->WSMError.WSMNo1609p2HeaderDropped);
    printf("WSA Received:\t\t\t\t\t%-10d\n", rskShmPtr->WSACnt.WSAReceived);
    printf("WSA Analyzed:\t\t\t\t\t%-10d\n", rskShmPtr->WSACnt.WSAAnalyzed);
    if (rskShmPtr->WSACnt.WSAReceived != 0) {
        printf("WSA Parse Errors:\t\t\t\t%-10d\n", rskShmPtr->WSACnt.WSAParseError);
        printf("WSA Process Errors:\t\t\t\t%-10d\n", rskShmPtr->WSACnt.WSAProcessError);
    }

    if (rskShmPtr->WSACnt.WSALengthInvalid != 0)
        printf("WSA Length Invalid:\t\t\t\t%-10d\n", rskShmPtr->WSACnt.WSALengthInvalid);
    if (rskShmPtr->WSACnt.WSAChannelAccessInvalid != 0)
        printf("WSA Channel Access Invalid:\t\t\t%-10d\n", rskShmPtr->WSACnt.WSAChannelAccessInvalid);
    if (rskShmPtr->WSACnt.WSAPsidInvalid != 0)
        printf("WSA PSID Invalid:\t\t\t\t%-10d\n", rskShmPtr->WSACnt.WSAPsidInvalid);
    if (rskShmPtr->WSACnt.WSAWrongVersion != 0)
        printf("WSA Wrong Version:\t\t\t\t%-10d\n", rskShmPtr->WSACnt.WSAWrongVersion);
    if (rskShmPtr->WSACnt.WSAServiceCountExceeded != 0)
        printf("WSA Service Count Exceeded:\t\t\t%-10d\n", rskShmPtr->WSACnt.WSAServiceCountExceeded);
    if (rskShmPtr->WSACnt.WSAChannelCountInvalid != 0)
        printf("WSA Channel Count Invalid:\t\t\t%-10d\n", rskShmPtr->WSACnt.WSAChannelCountInvalid);
    if (rskShmPtr->WSACnt.WSAChannelLengthMismatch != 0)
        printf("WSA Channel Length Mismatch:\t\t\t%-10d\n", rskShmPtr->WSACnt.WSAChannelLengthMismatch);
    if (rskShmPtr->WSACnt.WSASecurityInfoStripAttempt != 0) {
        printf("WSA Attempts to strip security information:\t%-10d\n", rskShmPtr->WSACnt.WSASecurityInfoStripAttempt);
        printf("WSA Strip security information failed:\t\t%-10d\n", rskShmPtr->WSACnt.WSASecurityInfoStripFailed);
    }
    if (rskShmPtr->WSACnt.WSASemaphoreError != 0)
        printf("WSA Take semaphore error:\t\t\t%-10d\n", rskShmPtr->WSACnt.WSASemaphoreError);
    if (rskShmPtr->WSACnt.WSAVerifySemaphoreError != 0)
        printf("WSA Verify semaphore error:\t\t\t%-10d\n", rskShmPtr->WSACnt.WSAVerifySemaphoreError);
    if (rskShmPtr->WSACnt.WSANoAvailableWSAStatusTableEntry != 0)
        printf("WSA No available WSA Table Entry:\t\t%-10d\n", rskShmPtr->WSACnt.WSANoAvailableWSAStatusTableEntry);
    if (rskShmPtr->RISReqCnt.VerifyWsaReq != 0) {
        printf("WSA Verify successes:\t\t\t%-10d\n", rskShmPtr->WSACnt.WSAVerifySuccess);
        printf("WSA Verify failures:\t\t\t%-10d\n", rskShmPtr->WSACnt.WSAVerifyFailed);
    }
    if (rskShmPtr->WSACnt.WSASecurityVerifyInvalidHandle != 0)
        printf("WSA Security Verify Invalid Handle:\t\t%-10d\n", rskShmPtr->WSACnt.WSASecurityVerifyInvalidHandle);
    if (rskShmPtr->WSACnt.WSASecurityVerifySignatureFailed != 0)
        printf("WSA Security Verify Signature Failed:\t\t%-10d\n", rskShmPtr->WSACnt.WSASecurityVerifySignatureFailed);
    if (rskShmPtr->WSACnt.WSASecurityUnsupportedSecurityType != 0)
        printf("WSA Security Unsupported Security Type:\t\t%-10d\n", rskShmPtr->WSACnt.WSASecurityUnsupportedSecurityType);
    printf("\nSend Indication:\n");
    printf("By\t\tOK\t\tNo Mat (A/WST)\tNo Match (GST)\tNo CB Reg\tSend Data Fail\n");
    printf("PID\t\t%-10u\t%-10u\t\t\t\t\t%-10u\n",           rskShmPtr->SendIndError.PIDOK,    rskShmPtr->SendIndError.PIDNoMatchAST,                                                                                            rskShmPtr->SendIndError.PIDSendFailed);
    printf("RT/PSID\t\t%-10u\t%-10u\t%-10u\t%-10u\t%-10u\n", rskShmPtr->SendIndError.RTPSIDOK, rskShmPtr->SendIndError.RTPSIDNoMatchWST, rskShmPtr->SendIndError.RTPSIDNoMatchGST, rskShmPtr->SendIndError.RTPSIDNoCBRegistered, rskShmPtr->SendIndError.RTPSIDSendFailed);
    for (i=0; i < MAX_RADIO_TYPES; i++) {
        for (j=0; j < MAX_RADIOS; j++) {
            if (!rskShmPtr->wsuNsCurService[i][j].ServiceRunning) continue;
                printf("\nCurrent Radio Services Radio %s%d:\n", (i == 0) ? "C_V2X" : "DSRC", j);
                printf("    User    Service\t    = %s\n", (rskShmPtr->wsuNsCurService[i][j].UserService) ? ("true"): ("false"));
                printf("    Service Running\t    = %s\n", (rskShmPtr->wsuNsCurService[i][j].ServiceRunning) ? ("true"): ("false"));
                printf("    Service Channel\t    = %-10d\n",  rskShmPtr->wsuNsCurService[i][j].CurChannel);
                printf("    Service Priority\t    = %-10d\n",  rskShmPtr->wsuNsCurService[i][j].CurPriority);
            if (rskShmPtr->wsuNsCurService[i][j].UserService) {
                printf("    ExtendedAccess\t    = %-10d\n", rskShmPtr->wsuNsCurService[i][j].CurExtendedAccess);
            if (rskShmPtr->wsuNsCurService[i][j].CurAccessType == 0)
                printf("    Service Access\t    = AUTO\n");
            else if (rskShmPtr->wsuNsCurService[i][j].CurAccessType == 1)
                printf("    Service Access\t    = UNCONDITONAL\n");
            if (rskShmPtr->wsuNsCurService[i][j].CurAccessType == 2)
                printf("    Service Access\t    = NO_SCH_ACCESS\n");
            } else {
                printf("    Provider Access Type = %s\n", (rskShmPtr->wsuNsCurService[i][j].CurProvAccessType == 0) ? "Continuous" : "Alternating");
            }
        }
    }

    printf("\n== EU Counters ==\n");
    printf("EU Packets Transmitted (C-V2X):\t\t%-10d\tEU Packets Received (C-V2X):\t\t%-10d\n", rskShmPtr->EUCnt.EUPacketsTransmitted[RT_CV2X], rskShmPtr->EUCnt.EUPacketsReceivedFromRadio[RT_CV2X]);
    printf("EU Packets Transmitted (DSRC):\t\t%-10d\tEU Packets Received (DSRC):\t\t%-10d\n", rskShmPtr->EUCnt.EUPacketsTransmitted[RT_DSRC], rskShmPtr->EUCnt.EUPacketsReceivedFromRadio[RT_DSRC]);

    printf("\n== ALSMI Counters ==\n");
    printf("Register:\t\t\t%-10u\n", rskShmPtr->ALSMICnt.Register);
    printf("Unregister:\t\t\t%-10u\n", rskShmPtr->ALSMICnt.Unregister);
    printf("Get Aerolink Version:\t\t%-10u\n", rskShmPtr->ALSMICnt.GetAerolinkVersion);
    printf("Get Leap Seconds Since 2004:\t%-10u\n", rskShmPtr->ALSMICnt.GetLeapSecondsSince2004);
    printf("Update Position And Time:\t%-10u\n", rskShmPtr->ALSMICnt.UpdatePositionAndTime);
    printf("\n\t\tRequests\tSuccesses\tFailures\tLast Aerolink Error Code\n");
    printf("1609.2\t\t%-10u\t%-10u\t%-10u\t%-10u\n", rskShmPtr->ALSMICnt.Hdr1609p2Requests,      rskShmPtr->ALSMICnt.Hdr1609p2Successes,         rskShmPtr->ALSMICnt.Hdr1609p2Failures,          rskShmPtr->ALSMICnt.LastHdr1609p2ErrorCode);
    printf("Sign\t\t%-10u\t%-10u\t%-10u\t%-10u\n",   rskShmPtr->ALSMICnt.SignRequests,           rskShmPtr->ALSMICnt.SignSuccesses,              rskShmPtr->ALSMICnt.SignFailures,               rskShmPtr->ALSMICnt.LastSignErrorCode);
    printf("Strip\t\t%-10u\t%-10u\t%-10u\t%-10u\n",  rskShmPtr->ALSMICnt.StripRequests,          rskShmPtr->ALSMICnt.StripSuccesses,             rskShmPtr->ALSMICnt.StripFailures,              rskShmPtr->ALSMICnt.LastStripErrorCode);
    printf("Verify\t\t%-10u\t%-10u\t%-10u\t%-10u\n", rskShmPtr->ALSMICnt.VerifyRequests,         rskShmPtr->ALSMICnt.VerifySuccesses,            rskShmPtr->ALSMICnt.VerifyFailures,             rskShmPtr->ALSMICnt.LastVerifyErrorCode);
    printf("CertChReq\t%-10u\t%-10u\t%-10u\t%-10u\n",rskShmPtr->ALSMICnt.CertChangeRequests,     rskShmPtr->ALSMICnt.CertChangeRequestsAccepted, rskShmPtr->ALSMICnt.CertChangeRequestsRejected, rskShmPtr->ALSMICnt.CertChangeRequestLastErrorCode);
    printf("CertCh\t\t\t\t%-10u\t%-10u\t%-10u\n",                                               rskShmPtr->ALSMICnt.CertChangeSuccesses,        rskShmPtr->ALSMICnt.CertChangeFailures,         rskShmPtr->ALSMICnt.CertChangeLastErrorCode);
    printf("IDChLock\t%-10u\t%-10u\t%-10u\t%-10u\n", rskShmPtr->ALSMICnt.IdChangeLockRequests,   rskShmPtr->ALSMICnt.IdChangeLockSuccesses,      rskShmPtr->ALSMICnt.IdChangeLockFailures,       rskShmPtr->ALSMICnt.IdChangeLockLastErrorCode);
    printf("IDChUnl\t\t%-10u\t%-10u\t%-10u\t%-10u\n",rskShmPtr->ALSMICnt.IdChangeUnlockRequests, rskShmPtr->ALSMICnt.IdChangeUnlockSuccesses,    rskShmPtr->ALSMICnt.IdChangeUnlockFailures,     rskShmPtr->ALSMICnt.IdChangeUnlockLastErrorCode);
    printf("\nSign Failures:\n");
    printf("Unknown Packet Type\t\t%-10u\tInvalid PSID\t\t\t\t%-10u\n",             rskShmPtr->ALSMICnt.SFUnknownPacketType,  rskShmPtr->ALSMICnt.SFInvalidPSID);
    printf("Payload Too Large\t\t%-10u\tCertificate Change In Progress\t\t%-10u\n", rskShmPtr->ALSMICnt.SFPayloadTooLarge,    rskShmPtr->ALSMICnt.SFCertificateChangeInProgress);
    printf("Sign Buffer Overflow\t\t%-10u\tsmg_new() Failed\t\t\t%-10u\n",          rskShmPtr->ALSMICnt.SFSignBufferOverflow, rskShmPtr->ALSMICnt.SFsmg_newFailed);
    printf("\n");
    printf("Start Cert Change Timer\t\t%-10u\tCancel Cert Change Timer\t\t%-10u\n",  rskShmPtr->ALSMICnt.StartCertChangeTimer,       rskShmPtr->ALSMICnt.CancelCertChangeTimer);
    printf("Cert Change Retries\t\t%-10u\tCert Change Timeouts\t\t\t%-10u\n",        rskShmPtr->ALSMICnt.CertChangeRetries,          rskShmPtr->ALSMICnt.CertChangeTimeouts);
    printf("ID Change Init CBs\t\t%-10u\tID Change Init Zero Certs\t\t%-10u\n",      rskShmPtr->ALSMICnt.IdChangeInitCallbacks,      rskShmPtr->ALSMICnt.IdChangeInitZeroCerts);
    printf("P2P Received\t\t\t%-10u\tP2P Processed\t\t\t\t%-10u\n",                  rskShmPtr->P2PCnt.P2PReceived,                  rskShmPtr->P2PCnt.P2PProcessed);
    printf("P2P Process Error\t\t%-10u\tP2P Process Last Error Code\t\t%-10d\n",     rskShmPtr->P2PCnt.P2PProcessError,              rskShmPtr->P2PCnt.P2PProcessLastErrorCode);
    printf("P2P Callback Called\t\t%-10u\tP2P Callback Send Success\t\t%-10u\n",     rskShmPtr->P2PCnt.P2PCallbackCalled,            rskShmPtr->P2PCnt.P2PCallbackSendSuccess);
    printf("P2P CB No Radio Configured\t%-10u\tP2P Callback Length Error\t\t%-10u\n", rskShmPtr->P2PCnt.P2PCallbackNoRadioConfigured, rskShmPtr->P2PCnt.P2PCallbackLengthError);
    printf("P2P Callback Send Error\t\t%-10u\tP2P Callback Send Last Error Code\t%-10d\n", rskShmPtr->P2PCnt.P2PCallbackSendError,         rskShmPtr->P2PCnt.P2PCallbackSendLastErrorCode);

    printf("\n== C-V2X Counters ==\n");
    /* Uncomment the "Recovery" parts of the line below if the recovery logic is added back into ns_cv2x.cpp */
    printf("C-V2X Inited:\t\t\t\t\t%-10u"/*"\tRecovery:\t\t\t\t\t%-10u"*/"\n", rskShmPtr->CV2XCnt.cv2xInited/*, rskShmPtr->CV2XCnt.recoveryCnt*/);
    printf("service IDs:\t\t\t     0x%08x 0x%08x\n", rskShmPtr->CV2XCnt.serviceID[0], rskShmPtr->CV2XCnt.serviceID[1]);
    printf("cv2xRadioManager->onReady().get() called:\t%-10u\tcv2xRadioManager->onReady().get() returned:\t%-10u\n", rskShmPtr->CV2XCnt.onReadyGetCalledCnt, rskShmPtr->CV2XCnt.onReadyGetReturnedCnt);
    printf("gCv2xRadio->createRxSubscription() called:\t%-10u\trxSubCallback called:\t\t\t\t%-10u\n", rskShmPtr->CV2XCnt.createRxSubCalledCnt, rskShmPtr->CV2XCnt.rxSubCallbackCalledCnt);
    printf("\nBootup state: 0x%x\n", rskShmPtr->bootupState);
    /* Unmount Radio_ns shared memory */
    wsu_share_kill(rskShmPtr, sizeof(shm_rsk_t));
}

void nsstats_x(void)
{
    rsGetWsaInfoMsgType         getWsaInfo;
    rsGetWsaInfoElementMsgType  getWsaInfoElement;
    static rsGetStringReplyType reply;
    int32_t                     size;
    int                         wsa_num, service_num, channel_idx;

    for (wsa_num = 0; wsa_num < NUM_WSA; wsa_num++) {
        getWsaInfo.cmd     = WSU_NS_GET_WSA_INFO;
        getWsaInfo.wsa_num = wsa_num;
        size = communicateWithRadioServices(&getWsaInfo, sizeof(getWsaInfo),
                                            &reply,      sizeof(reply));

        if ((size < 0) || (reply.risRet != RS_SUCCESS)) {
            printf("Error getting WSA info\n");
            return;
        }

        printf("%s", reply.str);

        for (service_num = 0; service_num < MAX_SERVICE_INFO; service_num++) {
            getWsaInfoElement.cmd         = WSU_NS_GET_WSA_SERVICE_INFO;
            getWsaInfoElement.wsa_num     = wsa_num;
            getWsaInfoElement.element_num = service_num;
            size = communicateWithRadioServices(&getWsaInfoElement,
                                                sizeof(getWsaInfoElement),
                                                &reply, sizeof(reply));

            if (size < 0) {
                printf("Error getting WSA service info\n");
                return;
            }

            if (reply.risRet == RS_SUCCESS) {
                printf("%s", reply.str);
            }
        }

        for (channel_idx = 0; channel_idx < MAX_CHANNEL_INFO; channel_idx++) {
            getWsaInfoElement.cmd         = WSU_NS_GET_WSA_CHANNEL_INFO;
            getWsaInfoElement.wsa_num     = wsa_num;
            getWsaInfoElement.element_num = channel_idx;
            size = communicateWithRadioServices(&getWsaInfoElement,
                                                sizeof(getWsaInfoElement),
                                                &reply, sizeof(reply));

            if (size < 0) {
                printf("Error getting WSA service info\n");
                return;
            }

            if (reply.risRet == RS_SUCCESS) {
                printf("%s", reply.str);
            }
        }

        getWsaInfo.cmd  = WSU_NS_GET_WSA_WRA_INFO;
        getWsaInfo.wsa_num = wsa_num;
        size = communicateWithRadioServices(&getWsaInfo, sizeof(getWsaInfo),
                                            &reply,      sizeof(reply));

        if (size < 0) {
            printf("Error getting WSA WRA info\n");
            return;
        }

        if (reply.risRet == RS_SUCCESS) {
            printf("%s", reply.str);
        }
    } // for (wsa_num = 0; wsa_num < NUM_WSA; wsa_num++)
}

int main (int argc, char *argv[])
{
    int i;
    int count = 1;

    if (argc > 2) {
        count = atoi(argv[2]);
    }

    if (count == 0) {
        count = -1;
    }

    if ((argc == 1) || ((argc > 1) && (strchr(argv[1], 'h') != NULL))) {
        printf(USAGE);
        return 0;
    }

    if ((argc > 1) && (strchr(argv[1], 'c') != NULL)) {
        nsstats_c();
    }

    for (i = 0; i < count; i++) {
        if (argc > 1) {
            if (strchr(argv[1], 'w') != NULL) {
                nsstats_w();
            }
            else if (strchr(argv[1], 'r') != NULL) {
                nsstats_r();
            }
            else if (strchr(argv[1], 'x') != NULL) {
                nsstats_x();
            }
        }

        if (i != (count - 1)) {
            usleep(200000);
        }
    }

    if (radioServicesSock != -1) {
        close(radioServicesSock);
    }

    return 0;
}
