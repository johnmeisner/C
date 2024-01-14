/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Filename: certChange.c                                                     */
/*  Purpose: Simple sample program to transmit 10 unsecured packets per       */
/*           second and do a certificate change every 30 seconds.             */
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
 * Transmits 10 signed WSM's per second without callbacks. Also, a certificate
 * change is done once every 30 seconds with callbacks.
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include "ris.h"
#include "alsmi_api.h"
#include "param.h"
#include "type2str.h"

static appCredentialsType appCredential = {
    .cbServiceAvailable = NULL,
    .cbReceiveWSM     = NULL
};

static UserServiceType uService = {
    .radioType         = RT_CV2X,
    .radioNum          = 0,
    .userAccess        = AUTO_ACCESS_UNCONDITIONAL,
    .psid              = SIGNED_WSM_PSID,
    .servicePriority   = 10,
    .wsaSecurity       = UNSECURED_WSA,
    .lengthPsc         = 10,
    .channelNumber     = CHANNEL,
    .lengthAdvertiseId = 15,
    .linkQuality       = 20,
    .immediateAccess   = 0,
    .extendedAccess    = 65535
};

static outWSMType outWsmMsg = {
    .channelNumber  = CHANNEL,
    .dataRate       = DR_6_MBPS,
    .txPwrLevel     = 20,
    .txPriority     = 2,
    .radioType      = RT_CV2X,
    .radioNum       = 0,
    .security       = SECURITY_SIGNED,
    .securityFlag   = 2, /* Let Aerolink decide whether to sign with a digest
                          * or a full certificate */
    .peerMacAddress = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}, 
    .wsmpHeaderExt  = 0,
    .psid           = SIGNED_WSM_PSID,
    .isBitmappedSsp = 0,
    .sspLen         = 0,
};

static bool_t radioCommInited     = FALSE;
static bool_t userServiceStarted  = FALSE;
static bool_t securityInitialized = FALSE;
static bool_t mainLoop            = TRUE;

/* Certificate change done callback */
void certChangeDoneCallback(smiCertChangeResults_t *results)
{
    printf("%s: returnCode = %u\n", __func__, results->returnCode);
    
    if (results->returnCode == WS_SUCCESS) {
        printf("%s: certId     = %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x\n", __func__,
               results->certId[0], results->certId[1], results->certId[2], results->certId[3],
               results->certId[4], results->certId[5], results->certId[6], results->certId[7]);
        printf("%s: lcmName    = \"%s\"\n", __func__, results->lcmName);
    }
}

int init(void)
{
    rsResultCodeType         risRet;
    int32_t                  retVal;
    rsSecurityInitializeType init;

    risRet = wsuRisInitRadioComm("certChange", &appCredential);

    /* Init radio comm */
    if (risRet != RS_SUCCESS) {
        printf("wsuRisInitRadioComm failed (%s)\n", rsResultCodeType2Str(risRet));
        return -1;
    }

    radioCommInited = TRUE;
    printf("RadioComm Initialized\n");

    /* Start up user service */
    uService.action = ADD;
    risRet = wsuRisUserServiceRequest(&uService);

    if (risRet != RS_SUCCESS) {
        printf("wsuRisUserServiceRequest (ADD) failed (%s)\n", rsResultCodeType2Str(risRet));
        return -1;
    }

    userServiceStarted = TRUE;
    printf("User service started\n");

    /* Initialize the security services. Note that a call to smiInitialize() is
     * required if you want to change certificates, even if you don't call any
     * callback functions (we do the certificate change callback here). */
    init.debug                             = FALSE;
    init.vodEnabled                        = TRUE;
    init.smi_cert_change_callback          = certChangeDoneCallback;
    init.vtp_milliseconds_per_verification = 0;
    init.smi_sign_results_options          = smi_results_none;
    init.smi_sign_results_callback         = NULL;
    init.smi_sign_wsa_results_options      = smi_results_none;
    init.smi_sign_wsa_results_callback     = NULL;
    init.smi_verify_wsa_results_options    = smi_results_none;
    init.smi_verify_wsa_results_callback   = NULL;
    strcpy(init.lcmName, "bsmLcm"); // from denso.wsc
    init.numSignResultsPSIDs               = 1;
    init.signResultsPSIDs[0]               = SIGNED_WSM_PSID;
    retVal = smiInitialize(&init);

    if (retVal != 0) {
        printf("Error initializing security (%d)\n", retVal);
        return -1;
    }

    securityInitialized = TRUE;
    return 0;
}

void term(void)
{
    rsResultCodeType risRet;

    /* Terminate security */
    if (securityInitialized) {
        smiTerm();
    }

    /* Terminate user service */
    if (userServiceStarted) {
        uService.action = DELETE;
        risRet = wsuRisUserServiceRequest(&uService);

        if (risRet != RS_SUCCESS) {
            printf("wsuRisUserServiceRequest (DELETE) failed (%s)\n", rsResultCodeType2Str(risRet));
        }
        else {
            printf("User service stopped\n");
        }
    }

    /* Terminate radio comm */
    if (radioCommInited) {
        risRet = wsuRisTerminateRadioComm();

        if (risRet != RS_SUCCESS) {
            printf("wsuRisTerminateRadioComm failed (%s)\n", rsResultCodeType2Str(risRet));
        }
        else {
            printf("RadioComm Terminated\n");
        }
    }
}

void exitHandler(int signum)
{
    if (mainLoop) {
        printf("ExitHandler is called.\n");
        mainLoop = FALSE;
    }
}

int main(int argc, char **argv)
{
    int              retVal = 0;
    rsResultCodeType risRet;
    uint8_t          payload[] = "Cert change test";
    int              txCount = 0;
    cfgType          getCfg;
    bool_t           cv2x_enable = FALSE;
    bool_t           dsrc_enable = FALSE;

    /* Get the value of the nsconfig variable CV2X_ENABLE */
    risRet = wsuRisGetCfgReq(CFG_ID_CV2X_ENABLE, &getCfg);

    if (risRet == RS_SUCCESS) {
        cv2x_enable = getCfg.u.CV2XEnable;
    }

    /* Get the value of the nsconfig variable DSRC_ENABLE */
    risRet = wsuRisGetCfgReq(CFG_ID_DSRC_ENABLE, &getCfg);

    if (risRet == RS_SUCCESS) {
        dsrc_enable = getCfg.u.DSRCEnable;
    }

    /* Error if no radios enabled */
    if (!cv2x_enable && !dsrc_enable) {
        printf("No radios enabled! Enable either the C-V2X or the DSRC radio.\n");
        return -1;
    }

    /* If one radio is enabled, but not the other, override the --rtype value */
    if (cv2x_enable && !dsrc_enable) {
        uService.radioType  =
        outWsmMsg.radioType = RT_CV2X;
    }
    else if (!cv2x_enable && dsrc_enable) {
        uService.radioType  =
        outWsmMsg.radioType = RT_DSRC;
    }

    if (init() != 0) {
        retVal = -1;
        goto exit;
    }

    memcpy(outWsmMsg.data, payload, sizeof(payload));
    outWsmMsg.dataLength = sizeof(payload);
    signal(SIGINT,  exitHandler);
    signal(SIGTERM, exitHandler);

    /* Send a packet 10 times a seconds, until user terminates */
    while (mainLoop) {
        risRet = wsuRisSendWSMReq(&outWsmMsg);

        if (risRet != RS_SUCCESS) {
            printf("wsuRisSendWSMReq failed (%s)\n", rsResultCodeType2Str(risRet));
            retVal = -1;
            goto exit;
        }

        txCount++;

        if (txCount % 300 == 0) {
            if (smiSendCertChangeReq() != 0) {
                printf("smiSendCertChangeReq() failed.\n");
                goto exit;
            }
        }

        usleep(100000);
    }

exit:
    term();
    return retVal;
}

