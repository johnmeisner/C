/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Filename: rxSignedWSMsWithVerifyHybrid.c                                   */
/*  Purpose: Simple sample program to receive signed WSM's with verification  */
/*           once a second via VOD.                                           */
/*                                                                            */
/* Copyright (C) 2021 DENSO International America, Inc.  All Rights Reserved. */
/*                                                                            */
/* Revision History:                                                          */
/*                                                                            */
/* Date        Author        Comments                                         */
/* -------------------------------------------------------------------------- */
/* 2019-06-06  VROLLINGER    Initial revision.                                */
/*                                                                            */
/*----------------------------------------------------------------------------*/
/*
 * Receives signed WSM's with verification once a second via both VOD and VTP.
 * Calls to smiInitialize() and smiTerm() are required.
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include "ris.h"
#include "alsmi_api.h"
#include "vod_api.h"
#include "param.h"
#include "type2str.h"

static void receiveWsm(inWSMType *Wsm);

static appCredentialsType appCredential = {
    .cbServiceAvailable = NULL,
    .cbReceiveWSM     = receiveWsm
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

static WsmServiceType service = {
    .radioType = RT_CV2X,
    .psid      = SIGNED_WSM_PSID
};

static bool_t   radioCommInited     = FALSE;
static bool_t   userServiceStarted  = FALSE;
static bool_t   wsmServiceStarted   = FALSE;
static uint32_t vodMsgSeqNum        = 0;
static bool_t   securityInitialized = FALSE;
static bool_t   mainLoop            = TRUE;

static void receiveWsm(inWSMType *Wsm)
{
    switch (Wsm->securityFlags & 0xff) {
    case unsecured:
        printf("securityFlags == unsecured\n");
        break;

    case securedButNotVerified:
        printf("securityFlags == securedButNotVerified\n");

        if (Wsm->vodMsgSeqNum >= 2) {
            vodMsgSeqNum = Wsm->vodMsgSeqNum;
        }

        break;

    case securedVerifiedSuccess:
        printf("securityFlags == securedVerifiedSuccess\n");
        break;

    case securedVerifiedFail:
        printf("securityFlags == securedVerifiedFail\n");
        break;

    case securedVernNotNeeded:
        printf("securityFlags == securedVernNotNeeded\n");
        break;

    default:
        printf("Unknown securityFlags LSB value (%d)\n",
               (Wsm->securityFlags & 0xff));
        break;
    }
}

int init(void)
{
    int                      rc;
    rsResultCodeType         risRet;
    int                      retVal;
    rsSecurityInitializeType init;

    /* Init VOD */
    rc = init_with_vod("rxWSMsWithVerifyVOD");

    if (rc < 0) {
        printf("init_with_vod(\"rxWSMsWithVerifyVOD\") failed\n");
        return -1;
    }

    printf("VOD initialized\n");

    risRet = wsuRisInitRadioComm("rxSignedWSMsWithVerifyHybrid", &appCredential);

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

    /* Start up WSM service */
    service.action = ADD;
    risRet = wsuRisWsmServiceRequest(&service);

    if (risRet != RS_SUCCESS) {
        printf("wsuRisWsmServiceRequest (ADD) failed (%s)\n", rsResultCodeType2Str(risRet));
        return -1;
    }

    wsmServiceStarted = TRUE;
    printf("WSM service started\n");

    /* Initialize the security services */
    init.debug                             = FALSE;
    init.vodEnabled                        = TRUE;
    init.smi_cert_change_callback          = NULL;
    init.vtp_milliseconds_per_verification = 1000;
    init.smi_sign_results_options          = smi_results_none;
    init.smi_sign_results_callback         = NULL;
    init.smi_sign_wsa_results_options      = smi_results_none;
    init.smi_sign_wsa_results_callback     = NULL;
    init.smi_verify_wsa_results_options    = smi_results_none;
    init.smi_verify_wsa_results_callback   = NULL;
    strcpy(init.lcmName, ""); // lcmName = NULL to signify no signing, only verification
    init.numSignResultsPSIDs               = 0;
    retVal = smiInitialize(&init);

    if (retVal != 0) {
        printf("Error initializing security (%d)\n", retVal);
        return -1;
    }

    securityInitialized = TRUE;
    printf("Security initialized\n");
    return 0;
}

void term(void)
{
    rsResultCodeType risRet;

    /* Terminate security */
    if (securityInitialized) {
        smiTerm();
    }

    printf("Security terminated\n");

    /* Terminate WSM service */
    if (wsmServiceStarted) {
        service.action = DELETE;
        risRet = wsuRisWsmServiceRequest(&service);

        if (risRet != RS_SUCCESS) {
            printf("wsuRisWsmServiceRequest (DELETE) failed (%s)\n", rsResultCodeType2Str(risRet));
        }
        else {
            printf("WSM service stopped\n");
        }
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

    /* Terminating VOD */
    denit_with_vod();
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
    int              count  = 0;
    int              rc;
    uint32_t         localVodMsgSeqNum;
    int8_t           vodRequestHandle;
    int              vodResult;
    rsResultCodeType risRet;
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
        uService.radioType = RT_CV2X;
        service.radioType  = RT_CV2X;
    }
    else if (!cv2x_enable && dsrc_enable) {
        uService.radioType = RT_DSRC;
        service.radioType  = RT_DSRC;
    }

    if (init() != 0) {
        retVal = -1;
        goto exit;
    }

    signal(SIGINT,  exitHandler);
    signal(SIGTERM, exitHandler);

    /* Print packets received once a second, until user terminates */
    while (mainLoop) {
        usleep(100000);
        count++;

        if ((count >= 10) && (vodMsgSeqNum >= 2)) {
            count = 0;
            localVodMsgSeqNum = vodMsgSeqNum;
            vodMsgSeqNum = 0;

            if (localVodMsgSeqNum >= 2) {
                printf("Calling request_msg_verification(%u, 100);\n", localVodMsgSeqNum);
                vodRequestHandle = request_msg_verification(localVodMsgSeqNum, 100);

                if (vodRequestHandle < 0) {
                    printf("request_msg_verification(%u, 100) failed\n", localVodMsgSeqNum);
                    continue;
                }

                printf("Calling wait_for_vod_result(%u);\n", vodRequestHandle);
                rc = wait_for_vod_result(vodRequestHandle);

                if (!rc) {
                    printf("wait_for_vod_result(%d) failed\n", vodRequestHandle);
                    continue;
                }

                printf("Calling retrieve_vod_result(%u);\n", vodRequestHandle);
                vodResult = retrieve_vod_result(vodRequestHandle);
                printf("VOD result = %d\n", vodResult);
            }
        }
    }

exit:
    term();
    return retVal;
}

