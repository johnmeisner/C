/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Filename: txUnsecuredWSMsNoCallbacks.c                                     */
/*  Purpose: Simple sample program to transmit unsecured WSMs with no         */
/*           callbacks.                                                       */
/*                                                                            */
/* Copyright (C) 2019 DENSO International America, Inc.  All Rights Reserved. */
/*                                                                            */
/* Revision History:                                                          */
/*                                                                            */
/* Date        Author        Comments                                         */
/* -------------------------------------------------------------------------- */
/* 2019-04-04  VROLLINGER    Initial revision.                                */
/*                                                                            */
/*----------------------------------------------------------------------------*/
/*
 * Transmits 10 unsecured WSM's per seconds with no callbacks. Therefore,
 * no calls to smiInitialize() or smiTerm() are needed.
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include "ris.h"
#include "param.h"
#include "type2str.h"

static appCredentialsType appCredential = {
    .cbServiceAvailable = NULL,
    .cbReceiveWSM       = NULL
};

static UserServiceType uService = {
    .radioType         = RT_CV2X,
    .radioNum          = 0,
    .userAccess        = AUTO_ACCESS_UNCONDITIONAL,
    .psid              = UNSECURED_WSM_PSID,
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
    .security       = SECURITY_UNSECURED,
    .securityFlag   = 0,
    .peerMacAddress = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}, 
    .wsmpHeaderExt  = 0,
    .psid           = UNSECURED_WSM_PSID,
    .isBitmappedSsp = 0,
    .sspLen         = 0,
};

static bool_t radioCommInited    = FALSE;
static bool_t userServiceStarted = FALSE;
static bool_t mainLoop           = TRUE;

int init(void)
{
    rsResultCodeType risRet;

    risRet = wsuRisInitRadioComm("txUnsecuredWSMsNoCallbacks", &appCredential);

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
    return 0;
}

void term(void)
{
    rsResultCodeType risRet;

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
    int8_t           payload[] = "Tx unsecured WSMs no callbacks";
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

        if (txCount % 10 == 0) {
            printf("Packets transmitted = %u\n", txCount);
        }

        usleep(100000);
    }

exit:
    term();
    return retVal;
}

