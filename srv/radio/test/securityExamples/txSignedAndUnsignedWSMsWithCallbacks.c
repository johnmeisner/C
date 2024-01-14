/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Filename: txSignedAndUnsignedWSMsWithCallbacks.c                           */
/*  Purpose: Simple sample program to transmit unsecured WSMs with no         */
/*           callbacks.                                                       */
/*                                                                            */
/* Copyright (C) 2019 DENSO International America, Inc.  All Rights Reserved. */
/*                                                                            */
/* Revision History:                                                          */
/*                                                                            */
/* Date        Author        Comments                                         */
/* -------------------------------------------------------------------------- */
/* 2019-06-06  VROLLINGER    Initial revision.                                */
/*                                                                            */
/*----------------------------------------------------------------------------*/
/*
 * Transmits a mixture of signed WSM's and unsigned WSM's with callbacks.
 * Therefore, calls to smiInitialize() or smiTerm() are required.
 *
 * This program also does certificate changes. A callback is done for every
 * certificate change.
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include "alsmi_api.h"
#include "ris.h"
#include "param.h"
#include "type2str.h"

/* The PSID for unsecured WSM's MUST be different from the PSID for signed
 * WSM's */
#define DIFFERENT_UNSECURED_WSM_PSID 0x00

static appCredentialsType appCredential = {
    .cbServiceAvailable = NULL,
    .cbReceiveWSM     = NULL,
};

UserServiceType uService = {
    .radioType       = RT_DSRC,
    .radioNum        = 0,
    .action          = ADD,
    .userAccess      = AUTO_ACCESS_UNCONDITIONAL,
    .psid            = DIFFERENT_UNSECURED_WSM_PSID,
    .servicePriority = 10,
    .wsaSecurity     = UNSECURED_WSA,
    .channelNumber   = CHANNEL,
    .linkQuality     = 20,
    .immediateAccess = 0,
    .extendedAccess  = 65535,
};

outWSMType outWsmMsg = {
    .channelNumber  = CHANNEL,
    .dataRate       = DR_6_MBPS,
    .txPwrLevel     = 20,
    .txPriority     = 2,
    .radioType      = RT_DSRC,
    .radioNum       = 0,
    .security       = SECURITY_UNSECURED,
    .securityFlag   = 0,
    .peerMacAddress = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
    .wsmpHeaderExt  = 0,
    .psid           = SIGNED_WSM_PSID,
    .dataLength     = 10,
    .isBitmappedSsp = 0,
    .sspLen         = 0,
};

bool_t mainLoop = TRUE;

static void certChangeCallback(smiCertChangeResults_t *result)
{
    printf("%s: returnCode = %u\n", __func__, result->returnCode);
    printf("%s: certId     = %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x\n", __func__,
           result->certId[0], result->certId[1], result->certId[2], result->certId[3],
           result->certId[4], result->certId[5], result->certId[6], result->certId[7]);
    printf("%s: lcmName    = \"%s\"\n", __func__, result->lcmName);
}

static int signResultsCallback(smiSignResults_t *result)
{
    printf("%s: psid               = 0x%x\n", __func__, result->psid);
    printf("%s: signStatus         = %d\n", __func__, result->signStatus);
    printf("%s: signErrorCode      = %u\n", __func__, result->signErrorCode);
    printf("%s: wsmLenWithSecurity = %u\n", __func__, result->wsmLenWithSecurity);
    return 0;
}

static int securityInitialize(void)
{
    rsSecurityInitializeType init;
    int                      retVal;

    init.debug                             = FALSE;
    init.vodEnabled                        = TRUE;
    init.smi_cert_change_callback          = certChangeCallback;
    init.vtp_milliseconds_per_verification = 1000; // Not used
    init.smi_sign_results_options          = smi_results_all;
    init.smi_sign_results_callback         = signResultsCallback;
    init.smi_sign_wsa_results_options      = smi_results_none;
    init.smi_sign_wsa_results_callback     = NULL;
    init.smi_verify_wsa_results_options    = smi_results_none;
    init.smi_verify_wsa_results_callback   = NULL;
    strcpy(init.lcmName, "bsmLcm"); // from denso.wsc
    init.numSignResultsPSIDs               = 2;
    init.signResultsPSIDs[0]               = DIFFERENT_UNSECURED_WSM_PSID; // Get sign results for unsecured packets
    init.signResultsPSIDs[1]               = SIGNED_WSM_PSID; // Get sign results for signed packets
    retVal = smiInitialize(&init);

    if (retVal != 0) {
        printf("Error initializing security %d\n", retVal);
        return -1;
    }

    printf("Security initialized.\n");
    return 0;
}

void exitHandler()
{
    if (mainLoop) {
        printf("exitHandler() is called.\n");
        mainLoop = FALSE;
    }
}

int main(int argc, char **argv)
{
    int              result = -1;
    rsResultCodeType risRet;
    int              retVal;
    uint32_t         count = 0;

    /* Only the DSRC radio is supported with this program */

    retVal = securityInitialize();

    if (retVal != 0) {
        goto err0;
    }

    risRet = wsuRisInitRadioComm("signedUnsigned", &appCredential);

    if (risRet != RS_SUCCESS) {
        printf("wsuRisInitRadioComm failed (%s)\n", rsResultCodeType2Str(risRet));
        goto err1;
    }

    printf("RadioComm Initialized\n");

    uService.action = ADD;
    uService.psid   = DIFFERENT_UNSECURED_WSM_PSID;
    risRet = wsuRisUserServiceRequest(&uService);

    if (risRet != RS_SUCCESS) {
        printf("wsuRisUserServiceRequest ADD failed (%s)\n", rsResultCodeType2Str(risRet));
        goto err2;
    }

    printf("wsuRisUserServiceRequest ADD (PSID=0x%x)\n", uService.psid);

    uService.action = ADD;
    uService.psid   = SIGNED_WSM_PSID;

    risRet = wsuRisUserServiceRequest(&uService);

    if (risRet != RS_SUCCESS) {
        printf("wsuRisUserServiceRequest ADD failed (%s)\n", rsResultCodeType2Str(risRet));
        goto err3;
    }

    printf("wsuRisUserServiceRequest ADD (PSID=0x%x)\n", uService.psid);

    signal(SIGINT,  exitHandler);
    signal(SIGTERM, exitHandler);

    while (mainLoop) {
        outWsmMsg.security   = SECURITY_UNSECURED;
        outWsmMsg.psid       = DIFFERENT_UNSECURED_WSM_PSID;
        strcpy((char *)outWsmMsg.data, "Unsecured");
        outWsmMsg.dataLength = sizeof("Unsecured");
        risRet = wsuRisSendWSMReq(&outWsmMsg);

        if (risRet != RS_SUCCESS) {
            printf("wsuRisSendWSMReq failed (%s)\n", rsResultCodeType2Str(risRet));
            goto err4;
        }

        printf("Sent unsecured packet.\n");

        if (++count % 30 == 0) {
            retVal = smiSendCertChangeReq();

            if (retVal != 0) {
                printf("smiSendCertChangeReq failed (%d)\n", retVal);
                goto err4;
            }

            printf("Requested certificate change.\n");
        }

        sleep(1);
        outWsmMsg.security     = SECURITY_SIGNED;
        outWsmMsg.securityFlag = 2; /* Let Aerolink decide whether to sign with
                                     * a digest or a full certificate */
        outWsmMsg.psid       = SIGNED_WSM_PSID;
        strcpy((char *)outWsmMsg.data, "Signed");
        outWsmMsg.dataLength = sizeof("Signed");
        risRet = wsuRisSendWSMReq(&outWsmMsg);

        if (risRet != RS_SUCCESS) {
            printf("wsuRisSendWSMReq failed (%s)\n", rsResultCodeType2Str(risRet));
            goto err4;
        }

        printf("Sent signed packet.\n");

        if (++count % 30 == 0) {
            retVal = smiSendCertChangeReq();

            if (retVal != 0) {
                printf("smiSendCertChangeReq failed (%d)\n", retVal);
                goto err4;
            }

            printf("Requested certificate change.\n");
        }

        sleep(1);
    }

    result = 0;

err4:
    uService.action = DELETE;
    uService.psid   = SIGNED_WSM_PSID;
    risRet = wsuRisUserServiceRequest(&uService);

    if (risRet != RS_SUCCESS) {
        result = -1;
        printf("wsuRisUserServiceRequest DELETE failed (%s) (PSID=0x%x)\n",
               rsResultCodeType2Str(risRet), uService.psid);
    }
    else {
        printf("wsuRisUserServiceRequest DELETE (PSID=0x%x)\n", uService.psid);
    }

err3:
    uService.action = DELETE;
    uService.psid   = DIFFERENT_UNSECURED_WSM_PSID;
    risRet = wsuRisUserServiceRequest(&uService);

    if (risRet != RS_SUCCESS) {
        result = -1;
        printf("wsuRisUserServiceRequest DELETE failed (%s) (PSID=0x%x)\n",
               rsResultCodeType2Str(risRet), uService.psid);
    }
    else {
        printf("wsuRisUserServiceRequest DELETE (PSID=0x%x)\n", uService.psid);
    }

err2:
    risRet = wsuRisTerminateRadioComm();

    if (risRet != RS_SUCCESS) {
        result = -1;
        printf("wsuRisTerminateRadioComm failed (%s)\n", rsResultCodeType2Str(risRet));
    }
    else {
        printf("RadioComm Terminated\n");
    }

err1:
    smiTerm();
    printf("Security terminated\n");
err0:
    return result;
}

