/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Filename: rxUnsecuredWSMs.c                                                */
/*  Purpose: Simple sample program to receive unsecured WSMs.                 */
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
 * Receives unsecured WSM's with no verification. No calls to smiInitialize() or
 * smiTerm() are needed.
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include "ris.h"
#include "param.h"
#include "type2str.h"

static void receiveWsm(inWSMType *Wsm);

static appCredentialsType appCredential = {
    .cbServiceAvailable = NULL,
    .cbReceiveWSM       = receiveWsm
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

static WsmServiceType service = {
    .radioType = RT_CV2X,
    .psid      = UNSECURED_WSM_PSID
};

static bool_t   radioCommInited     = FALSE;
static bool_t   userServiceStarted  = FALSE;
static bool_t   wsmServiceStarted   = FALSE;
static uint32_t rxCount             = 0;
static bool_t   printReceivedPacket = FALSE;
static bool_t   mainLoop            = TRUE;

static void dump(void *ptr, int len)
{
    uint8_t *p = (uint8_t *)ptr;
    int      l;
    int      i;

    while (len != 0) {
        l = ((len > 16) ? 16 : len);
        printf("%04lx ", p - (uint8_t *)ptr);

        for (i = 0; i < l; i++) {
            printf("%02x ", p[i]);
        }

        for (i = 0; i < l; i++) {
            printf("%c%c",
                    ((p[i] >= ' ') && (p[i] <= '~')) ? p[i] : '.',
                    (i == l - 1) ? '\n' : ' ');
        }

        p += l;
        len -= l;
    }
}

static void receiveWsm(inWSMType *Wsm)
{
    rxCount++;

    if (printReceivedPacket) {
        printf("%s: Received packet from %s: length = %u\n", __func__,
               (Wsm->radioType == RT_CV2X) ? "C-V2X" : "DSRC", Wsm->dataLength);
        dump(Wsm->data, Wsm->dataLength);
    }
}

int init(void)
{
    rsResultCodeType risRet;

    risRet = wsuRisInitRadioComm("rxUnsecuredWSMs", &appCredential);

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
    return 0;
}

void term(void)
{
    rsResultCodeType risRet;

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
    rsResultCodeType risRet;
    cfgType          getCfg;
    bool_t           cv2x_enable = FALSE;
    bool_t           dsrc_enable = FALSE;

    if ((argc > 1) && (argv[1][0] == '-') && (argv[1][1] == 'p')) {
        printReceivedPacket = TRUE;
    }

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

        if (count == 10) {
            count = 0;
            printf("Packets received = %u\n", rxCount);
        }
    }

exit:
    term();
    return retVal;
}

