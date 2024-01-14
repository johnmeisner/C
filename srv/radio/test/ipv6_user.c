/******************************************************************************
 *                                                                            *
 *     File Name:  ipv6_user.c                                                *
 *     Author:                                                                *
 *         DENSO International America, Inc.                                  *
 *         North America Research Laboratory, California Office               *
 *         3252 Business Park Drive                                           *
 *         Vista, CA 92081                                                    *
 *                                                                            *
 ******************************************************************************
 * (C) Copyright 2012 DENSO International America, Inc.  All rights reserved. *
 ******************************************************************************/

/*
 * IPv6 WSA User Application
 * The command line is as follows:
 *
 * ipv6_user
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <signal.h>
#include <ctype.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include "ris.h"
#include "tps_api.h"
#include "alsmi_api.h"
#if 0
#include "ndp_lib.h"
#endif
#include "libipv6.h"
#include "type2str.h"
#include "v2v_general.h"

/* Defines */
#define CONNECT_TO_SERVICE

#define MAX_CMD_LENGTH  256

#define ABS(x) (x) < 0 ? -(x) : (x)

#define appCredentialsV2Type appCredentialsType
#define UserServiceV2Type UserServiceType

/* Prototypes */
static void serviceAvailable(serviceInfoType *serviceInfo);

/* Variables */
static uint32_t              maxRSERange = 300;
static int16_t               minRssi = -85;
static int                   wsaVerificationFailedTimeoutSecs = 5;
static uint8_t               wsaRadio = 1;
static uint8_t               wsachannel = 178;
static bool_t                mainLoop = TRUE;
static bool_t                wBSSJoined = FALSE;
static uint8_t               wBSSMacAddress[LENGTH_MAC];
static uint16_t              wsaRxAnt = 0;
static uint8_t               printWsa = 0;
static bool_t                ipv6Inited = FALSE;
static bool_t                radioCommInited = FALSE;
static bool_t                userServiceInited = FALSE;
static bool_t                tpsInited = FALSE;
static bool_t                tpsRegistered = FALSE;
static bool_t                smiInitialized = FALSE;
static bool_t                matchMacAddr = FALSE;
static bool_t                smiDebug = FALSE;
static bool_t                verifyWsa = FALSE;
static uint8_t               verifyResults = 0;
static uint8_t               macAddr[LENGTH_MAC];
static tpsDataType           tpsUserData = {0};
static uint8_t               localTpsError = 1;
static bool_t                wsaSubmittedForVerification = FALSE;
#ifdef CONNECT_TO_SERVICE
static uint8_t               wsaSubmittedForVerificationMacAddr[LENGTH_MAC];
/* The following arrays are needed in case we have multiple RSE's in range. If
 * so, one or more may fail verification. If so, we need to keep track of which
 * ones failed, and how far away they are. The following arrays keep track of
 * the MAC addresses and the time stamps of the failed verifications. Also, if
 * the verification of a WSA from one RSE fails, we don't want that to prevent
 * us from attempting the verification of a WSA from another RSE. */
static bool_t                wsaVerificationFailed[MAX_RSES] =
    { FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE };
static bool_t                wsaVerificationFailedMacAddr[MAX_RSES][LENGTH_MAC];
static struct timeval       wsaVerificationFailedTimeStamp[MAX_RSES];
#endif // CONNECT_TO_SERVICE

static appCredentialsV2Type appCredentialV2 = {
    .cbServiceAvailable = serviceAvailable,
    .cbReceiveWSM = NULL,
};

static UserServiceV2Type    uService = {
    .radioNum          = 1,
    .action            = ADD,
    .userAccess        = NO_SCH_ACCESS,
    .psid              = 0x23,
    .servicePriority   = 10,
    .wsaSecurity       = UNSECURED_WSA,
    .lengthPsc         = 10,
    .radioType         = RT_DSRC,
    .channelNumber     = 178,
    .lengthAdvertiseId = 15,
    .linkQuality       = 20,
    .immediateAccess   = 0,
    .extendedAccess    = 0,
    .srcMacAddress     = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff},
};

#ifdef CONNECT_TO_SERVICE
uint16_t currentExtendedAccess = 0; /* What the user service extended access
                                    * is currently set to */

static int diffSecs(struct timeval *ts2, struct timeval *ts1)
{
    int diff = (int)(ts2->tv_sec - ts1->tv_sec);

    if (ts2->tv_usec < ts1->tv_usec) {
        diff--;
    }

    return diff;
}

static int getFreeWsaVerificationFailedIndex(void)
{
    int i;
    struct timeval ts;

    /* Look for an unused entry. */
    for (i = 0; i < MAX_RSES; i++) {
        if (wsaVerificationFailed[i] == FALSE) {
            return i;
        }
    }

    /* No unused entry found; look for an expired one. */
    gettimeofday(&ts, NULL);

    for (i = 0; i < MAX_RSES; i++) {
        if (diffSecs(&ts, &wsaVerificationFailedTimeStamp[i]) >=
            wsaVerificationFailedTimeoutSecs) {
            wsaVerificationFailed[i] = FALSE;
            return i;
        }
    }

    return -1;
}

static int getWsaVerificationFailedIndex(uint8_t *macAddr)
{
    int i;

    for (i = 0; i < MAX_RSES; i++) {
        if (wsaVerificationFailed[i] &&
            (memcmp(wsaVerificationFailedMacAddr[i], macAddr,
             LENGTH_MAC) == 0)) {
            return i;
        }
    }

    return -1;
}
#endif // CONNECT_TO_SERVICE

static void ipv6_deinit(void)
{
    rsResultCodeType risRet;

    if (ipv6Inited == TRUE) {
        /* Undo IPv6 setup in RIS */
        risRet = wsuRisTeardownIPv6Info(wsaRadio);

        if (risRet != RS_SUCCESS) {
            printf("wsuRisTeardownIPv6Info() failed (%s)\n",
                   rsResultCodeType2Str(risRet));
        }

        ipv6Inited = FALSE;
    }
}

static void ipv6_init(serviceInfoType *serviceInfo)
{
    rsResultCodeType risRet;

    if (ipv6Inited == TRUE) {
        /* Probably not necessary, but just in case */
        ipv6_deinit();
    }

    if (serviceInfo->routingBitmask & WSA_EXT_WRA_BITMASK) {
        risRet = wsuRisSetIPv6Info(serviceInfo, wsaRadio);

        if (risRet != RS_SUCCESS) {
            printf("wsuRisSetIPv6Info() failed (%s)\n",
                   rsResultCodeType2Str(risRet));
        }

        ipv6Inited = TRUE;
    }
}

static void wsaExitHandler()
{
    if (mainLoop) {
        printf("ExitHandler is called.\n");
    }

    mainLoop = FALSE;
    return;
}

/* Changes the user service request in the radio stack and the radio driver */
int32_t changeUserServiceRequest(UserServiceChangeType *cfg)
{
    rsResultCodeType risRet;
    cfgType risCfg;

    /* Change the user service in the radio stack */
    risRet = wsuRisChangeUserServiceRequest(cfg);

    if (risRet != RS_SUCCESS) {
        return risRet;
    }

#ifdef CONNECT_TO_SERVICE
    currentExtendedAccess = cfg->user.access.extendedAccess;
#endif // CONNECT_TO_SERVICE

    /* Set the IPv6 Tx power in the radio driver */
    risCfg.u.TxPwrLevel = cfg->user.access.txPwrLevel;

    risRet = wsuRisSetCfgReq(CFG_ID_IPV6_TXPWRLEVEL, &risCfg);

    if (risRet != RS_SUCCESS) {
        return risRet;
    }

    /* Set the IPv6 Tx data rate in the radio driver */
    risCfg.u.DataRate = cfg->user.access.dataRate;
    return wsuRisSetCfgReq(CFG_ID_IPV6_DATARATE, &risCfg);
}

static void wsaCleanHandler()
{
    static int        term_times = 0;
    rsResultCodeType  risRet;
    tpsResultCodeType tpsRet;

    printf("CleanHandler is called.\n");

    if (term_times > 0) {
        exit(-1);
    }

    term_times++;

    /* If we have joined a service, disconnect from it. */
    /* VJR WILLBEREVISITED consider doing this from the wsuRisUserServiceRequest
     * routine where uService.action = DELETE */
    if (wBSSJoined == TRUE) {
        UserServiceChangeType cfg;

        printf("Disconnecting from service "
               "mac=%02x:%02x:%02x:%02x:%02x:%02x\n",
               wBSSMacAddress[0], wBSSMacAddress[1],
               wBSSMacAddress[2], wBSSMacAddress[3],
               wBSSMacAddress[4], wBSSMacAddress[5]);

        cfg.psid                       = uService.psid;
        cfg.command                    = CHANGE_ACCESS;
        cfg.user.access.channelNumber  = wsachannel;
        cfg.user.access.adaptable      = 0;
        cfg.user.access.dataRate       = 6;
        cfg.user.access.txPwrLevel     = 20;
        cfg.user.access.userAccess     = NO_SCH_ACCESS;
        cfg.user.access.extendedAccess = 0;
        risRet = changeUserServiceRequest(&cfg);

        if (risRet != RS_SUCCESS) {
            printf("changeUserServiceRequest failed (%d)\n", risRet);
        }
    }

    /* Quit Listening to WSAs */
    uService.action = DELETE;

    if (userServiceInited) {
        risRet = wsuRisUserServiceRequest(&uService);

        if (risRet != RS_SUCCESS) {
            printf("wsuRisUserServiceRequest failed (%s)\n",
                   rsResultCodeType2Str(risRet));
        }
        else {
            printf("All User Services Unregistered\n");
        }
    }

    /* Done - Terminate Radio Communication */
    if (radioCommInited) {
        risRet = wsuRisTerminateRadioComm();

        if (risRet != RS_SUCCESS) {
            printf("wsuRisTerminateRadioComm failed (%s)\n",
                   rsResultCodeType2Str(risRet));
        }
        else {
            printf("RadioComm Terminated\n");
        }
    }

    /* Terminate security */
    if (smiInitialized) {
        smiTerm();
    }

    /* Terminate TPS services */
    if (tpsRegistered) {
        tpsRet = wsuTpsDeregister();

        if (tpsRet != TPS_SUCCESS) {
            printf("wsuTpsDeregister failed (%s).\n",
                   tpsResultCodeType2Str(tpsRet));
        }
        else {
            printf("TPS Deregistered.\n");
        }
    }

    if (tpsInited) {
        wsuTpsTerm();
        printf("TPS Terminated.\n");
    }

    /* Tear down IPv6 setup */
    ipv6_deinit();
}

static void printMinimalServiceInfo(serviceInfoType *serviceInfo)
{
    if (serviceInfo->serviceBitmask & WSA_EXT_SERVICE_PORT_BITMASK) {
        printf("port=%d\n", serviceInfo->port);
    }

    if (serviceInfo->serviceBitmask & WSA_EXT_IPV6ADDRESS_BITMASK) {
        printf("ipv6Address=%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x\n",
               serviceInfo->ipv6Address[0],
               serviceInfo->ipv6Address[1],
               serviceInfo->ipv6Address[2],
               serviceInfo->ipv6Address[3],
               serviceInfo->ipv6Address[4],
               serviceInfo->ipv6Address[5],
               serviceInfo->ipv6Address[6],
               serviceInfo->ipv6Address[7],
               serviceInfo->ipv6Address[8],
               serviceInfo->ipv6Address[9],
               serviceInfo->ipv6Address[10],
               serviceInfo->ipv6Address[11],
               serviceInfo->ipv6Address[12],
               serviceInfo->ipv6Address[13],
               serviceInfo->ipv6Address[14],
               serviceInfo->ipv6Address[15]);
    }

    if (serviceInfo->serviceBitmask & WSA_EXT_PROVIDER_MACADDRESS_BITMASK) {
        printf("providerMacAddress=%02x:%02x:%02x:%02x:%02x:%02x\n",
               serviceInfo->providerMacAddress[0],
               serviceInfo->providerMacAddress[1],
               serviceInfo->providerMacAddress[2],
               serviceInfo->providerMacAddress[3],
               serviceInfo->providerMacAddress[4],
               serviceInfo->providerMacAddress[5]);
    }

    printf("channelNumber=%d\n", serviceInfo->channelNumber);

    if (serviceInfo->channelBitmask & WSA_EXT_CHANNEL_ACCESS_BITMASK) {
        printf("channelAccess=%d (%s)\n", serviceInfo->channelAccess,
               (serviceInfo->channelAccess == 2) ? "alternating slot 1" :
               (serviceInfo->channelAccess == 1) ? "alternating slot 0" :
               (serviceInfo->channelAccess == 0) ? "continuous" : "?");
    }

    if (serviceInfo->routingBitmask & WSA_EXT_WRA_BITMASK) {
        printf("routerLifeTime=%d prefixLen=%d\n",
            serviceInfo->routerLifeTime, serviceInfo->prefixLen);
        printf("ipPrefix=%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x\n",
               serviceInfo->ipPrefix[0],
               serviceInfo->ipPrefix[1],
               serviceInfo->ipPrefix[2],
               serviceInfo->ipPrefix[3],
               serviceInfo->ipPrefix[4],
               serviceInfo->ipPrefix[5],
               serviceInfo->ipPrefix[6],
               serviceInfo->ipPrefix[7],
               serviceInfo->ipPrefix[8],
               serviceInfo->ipPrefix[9],
               serviceInfo->ipPrefix[10],
               serviceInfo->ipPrefix[11],
               serviceInfo->ipPrefix[12],
               serviceInfo->ipPrefix[13],
               serviceInfo->ipPrefix[14],
               serviceInfo->ipPrefix[15]);
        printf("defaultGateway=%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x\n",
               serviceInfo->defaultGateway[0],
               serviceInfo->defaultGateway[1],
               serviceInfo->defaultGateway[2],
               serviceInfo->defaultGateway[3],
               serviceInfo->defaultGateway[4],
               serviceInfo->defaultGateway[5],
               serviceInfo->defaultGateway[6],
               serviceInfo->defaultGateway[7],
               serviceInfo->defaultGateway[8],
               serviceInfo->defaultGateway[9],
               serviceInfo->defaultGateway[10],
               serviceInfo->defaultGateway[11],
               serviceInfo->defaultGateway[12],
               serviceInfo->defaultGateway[13],
               serviceInfo->defaultGateway[14],
               serviceInfo->defaultGateway[15]);
        if (serviceInfo->routingBitmask & WSA_EXT_GATEWAY_MACADDRESS_BITMASK)
            printf("gatewayMacAddress=%02x:%02x:%02x:%02x:%02x:%02x\n",
                   serviceInfo->gatewayMacAddress[0],
                   serviceInfo->gatewayMacAddress[1],
                   serviceInfo->gatewayMacAddress[2],
                   serviceInfo->gatewayMacAddress[3],
                   serviceInfo->gatewayMacAddress[4],
                   serviceInfo->gatewayMacAddress[5]);
    }

    printf("\n");
}

static void printServiceInfo(serviceInfoType *serviceInfo)
{
    printf("rss=%d sourceMac=%02x:%02x:%02x:%02x:%02x:%02x\n",
           serviceInfo->rss,
           serviceInfo->sourceMac[0], serviceInfo->sourceMac[1],
           serviceInfo->sourceMac[2], serviceInfo->sourceMac[3],
           serviceInfo->sourceMac[4], serviceInfo->sourceMac[5]);
    printf("changeCount=%d psid=0x%08x channelIndex=%d\n",
           serviceInfo->changeCount, serviceInfo->psid,
           serviceInfo->channelIndex);

    if (serviceInfo->WSABitmask & (WSA_EXT_REPEAT_RATE_BITMASK |
                                   WSA_EXT_ADVERTISER_ID_BITMASK)) {
        if (serviceInfo->WSABitmask & WSA_EXT_REPEAT_RATE_BITMASK) {
            printf("repeatRate=%d ", serviceInfo->repeatRate);
        }

        if (serviceInfo->WSABitmask & WSA_EXT_ADVERTISER_ID_BITMASK) {
            printf("advertiseId=\"%.*s\" ", serviceInfo->advertiseIdLength, serviceInfo->advertiseId);
        }

        printf("\n");
    }

    if (serviceInfo->WSABitmask & WSA_EXT_2DLOCATION_BITMASK) {
        printf("latitude2D=%d.%d longitude2D=%d.%d\n",
               serviceInfo->latitude2D / 10000000,
               ABS(serviceInfo->latitude2D % 10000000),
               serviceInfo->longitude2D / 10000000,
               ABS(serviceInfo->longitude2D % 10000000));
    }

    if (serviceInfo->WSABitmask & WSA_EXT_3DLOCATION_BITMASK) {
        printf("latitude3D=%d.%d longitude3D=%d.%d elevation3D=%d.%d "
               "confidence3D=0x%02x\n",
               serviceInfo->latitude3D / 10000000,
               ABS(serviceInfo->latitude3D % 10000000),
               serviceInfo->longitude3D / 10000000,
               ABS(serviceInfo->longitude3D % 10000000),
               serviceInfo->elevation3D / 10,
               ABS(serviceInfo->elevation3D % 10),
               serviceInfo->confidence3D);
    }

    printf("semiMajorAccuracy3D=%d semiMinorAccuracy3D=%d "
           "semiMajorOrientation3D=%d\n",
           serviceInfo->semiMajorAccuracy3D, serviceInfo->semiMinorAccuracy3D,
           serviceInfo->semiMajorOrientation3D);

    if (serviceInfo->serviceBitmask & (WSA_EXT_PROVIDER_SERVICE_CONTEXT_BITMASK |
                                       WSA_EXT_IPV6ADDRESS_BITMASK |
                                       WSA_EXT_SERVICE_PORT_BITMASK)) {
        if (serviceInfo->serviceBitmask & WSA_EXT_PROVIDER_SERVICE_CONTEXT_BITMASK) {
            printf("psc=\"%.*s\" ", serviceInfo->servicePscLength,
                   serviceInfo->psc);
        }

        if (serviceInfo->serviceBitmask & WSA_EXT_IPV6ADDRESS_BITMASK) {
            printf("ipv6Address=%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x\n",
                   serviceInfo->ipv6Address[0],
                   serviceInfo->ipv6Address[1],
                   serviceInfo->ipv6Address[2],
                   serviceInfo->ipv6Address[3],
                   serviceInfo->ipv6Address[4],
                   serviceInfo->ipv6Address[5],
                   serviceInfo->ipv6Address[6],
                   serviceInfo->ipv6Address[7],
                   serviceInfo->ipv6Address[8],
                   serviceInfo->ipv6Address[9],
                   serviceInfo->ipv6Address[10],
                   serviceInfo->ipv6Address[11],
                   serviceInfo->ipv6Address[12],
                   serviceInfo->ipv6Address[13],
                   serviceInfo->ipv6Address[14],
                   serviceInfo->ipv6Address[15]);
        }
    }

    if (serviceInfo->serviceBitmask & (WSA_EXT_SERVICE_PORT_BITMASK |
                                       WSA_EXT_RCPI_THRESHOLD_BITMASK |
                                       WSA_EXT_WSA_COUNT_THRESHOLD_BITMASK |
                                       WSA_EXT_WSA_COUNT_THRESHOLD_INTERVAL_BITMASK)) {
        if (serviceInfo->serviceBitmask & WSA_EXT_SERVICE_PORT_BITMASK) {
            printf("port=%d ", serviceInfo->port);
        }

        if (serviceInfo->serviceBitmask & WSA_EXT_RCPI_THRESHOLD_BITMASK) {
            printf("rcpiThreshold=%d ", serviceInfo->rcpiThreshold);
        }

        if (serviceInfo->serviceBitmask & WSA_EXT_WSA_COUNT_THRESHOLD_BITMASK) {
            printf("wsaCountThreshold=%d ", serviceInfo->wsaCountThreshold);
        }

        if (serviceInfo->serviceBitmask & WSA_EXT_WSA_COUNT_THRESHOLD_INTERVAL_BITMASK) {
            printf("wsaCountThresholdInterval=%d",
                   serviceInfo->wsaCountThresholdInterval);
        }

        printf("\n");
    }

    if (serviceInfo->serviceBitmask & WSA_EXT_PROVIDER_MACADDRESS_BITMASK) {
        printf("providerMacAddress=%02x:%02x:%02x:%02x:%02x:%02x\n",
               serviceInfo->providerMacAddress[0],
               serviceInfo->providerMacAddress[1],
               serviceInfo->providerMacAddress[2],
               serviceInfo->providerMacAddress[3],
               serviceInfo->providerMacAddress[4],
               serviceInfo->providerMacAddress[5]);
    }

    printf("operatingClass=%d channelNumber=%d adaptable=%d "
           "dataRate=%d txPwrLevel=%d\n",
           serviceInfo->operatingClass, serviceInfo->channelNumber,
           serviceInfo->adaptable, serviceInfo->dataRate,
           serviceInfo->txPwrLevel);

    if (serviceInfo->channelBitmask & WSA_EXT_CHANNEL_ACCESS_BITMASK) {
        printf("channelAccess=%d (%s)\n", serviceInfo->channelAccess,
               (serviceInfo->channelAccess == 2) ? "alternating slot 1" :
               (serviceInfo->channelAccess == 1) ? "alternating slot 0" :
               (serviceInfo->channelAccess == 0) ? "continuous" : "?");
    }

    if (serviceInfo->channelBitmask & WSA_EXT_EDCA_PARAM_SET_BITMASK) {
        int i;

        printf("qosInfo=%d\n", serviceInfo->qosInfo);

        for (i = 0; i < 4; i++) {
            printf("aifs[%d]=0x%02x ecw[%d]=0x%02x txop[%d]=%d\n",
                   i, serviceInfo->aifs[i], i, serviceInfo->ecw[i],
                   i, serviceInfo->txop[i]);
        }
    }

    if (serviceInfo->routingBitmask & WSA_EXT_WRA_BITMASK) {
        printf("routerLifeTime=%d prefixLen=%d\n",
            serviceInfo->routerLifeTime, serviceInfo->prefixLen);
        printf("ipPrefix=%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x\n",
               serviceInfo->ipPrefix[0],
               serviceInfo->ipPrefix[1],
               serviceInfo->ipPrefix[2],
               serviceInfo->ipPrefix[3],
               serviceInfo->ipPrefix[4],
               serviceInfo->ipPrefix[5],
               serviceInfo->ipPrefix[6],
               serviceInfo->ipPrefix[7],
               serviceInfo->ipPrefix[8],
               serviceInfo->ipPrefix[9],
               serviceInfo->ipPrefix[10],
               serviceInfo->ipPrefix[11],
               serviceInfo->ipPrefix[12],
               serviceInfo->ipPrefix[13],
               serviceInfo->ipPrefix[14],
               serviceInfo->ipPrefix[15]);
        printf("defaultGateway=%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x\n",
               serviceInfo->defaultGateway[0],
               serviceInfo->defaultGateway[1],
               serviceInfo->defaultGateway[2],
               serviceInfo->defaultGateway[3],
               serviceInfo->defaultGateway[4],
               serviceInfo->defaultGateway[5],
               serviceInfo->defaultGateway[6],
               serviceInfo->defaultGateway[7],
               serviceInfo->defaultGateway[8],
               serviceInfo->defaultGateway[9],
               serviceInfo->defaultGateway[10],
               serviceInfo->defaultGateway[11],
               serviceInfo->defaultGateway[12],
               serviceInfo->defaultGateway[13],
               serviceInfo->defaultGateway[14],
               serviceInfo->defaultGateway[15]);
        printf("primaryDns=%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x\n",
               serviceInfo->primaryDns[0],
               serviceInfo->primaryDns[1],
               serviceInfo->primaryDns[2],
               serviceInfo->primaryDns[3],
               serviceInfo->primaryDns[4],
               serviceInfo->primaryDns[5],
               serviceInfo->primaryDns[6],
               serviceInfo->primaryDns[7],
               serviceInfo->primaryDns[8],
               serviceInfo->primaryDns[9],
               serviceInfo->primaryDns[10],
               serviceInfo->primaryDns[11],
               serviceInfo->primaryDns[12],
               serviceInfo->primaryDns[13],
               serviceInfo->primaryDns[14],
               serviceInfo->primaryDns[15]);
        if (serviceInfo->routingBitmask & WSA_EXT_SECONDARY_DNS_BITMASK)
            printf("secondaryDns=%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x\n",
                   serviceInfo->secondaryDns[0],
                   serviceInfo->secondaryDns[1],
                   serviceInfo->secondaryDns[2],
                   serviceInfo->secondaryDns[3],
                   serviceInfo->secondaryDns[4],
                   serviceInfo->secondaryDns[5],
                   serviceInfo->secondaryDns[6],
                   serviceInfo->secondaryDns[7],
                   serviceInfo->secondaryDns[8],
                   serviceInfo->secondaryDns[9],
                   serviceInfo->secondaryDns[10],
                   serviceInfo->secondaryDns[11],
                   serviceInfo->secondaryDns[12],
                   serviceInfo->secondaryDns[13],
                   serviceInfo->secondaryDns[14],
                   serviceInfo->secondaryDns[15]);
        if (serviceInfo->routingBitmask & WSA_EXT_GATEWAY_MACADDRESS_BITMASK)
            printf("gatewayMacAddress=%02x:%02x:%02x:%02x:%02x:%02x\n",
                   serviceInfo->gatewayMacAddress[0],
                   serviceInfo->gatewayMacAddress[1],
                   serviceInfo->gatewayMacAddress[2],
                   serviceInfo->gatewayMacAddress[3],
                   serviceInfo->gatewayMacAddress[4],
                   serviceInfo->gatewayMacAddress[5]);
    }

    printf("\n");
}

double getRelativeDistance(double lat1, double lon1,
                           double lat2, double lon2)
{
    double alt1 = 0.0;
    double alt2 = 0.0;
    double x1, y1, z1;
    double x2, y2, z2;
    double c, s;
    double a = 6378137.0;
    double f = 0.003353;

    lat1 += 180.0;
    lat2 += 180.0;
    lon1 += 180.0;
    lon2 += 180.0;

    c = 1 / sqrt((pow(cos(lat1 / 180.0 * M_PI), 2)) +
                 pow(1 - f, 2) * (pow(sin(lat1 / 180.0 * M_PI), 2)));
    s = pow(1 - f, 2) * c;
    x1 = (a * c + alt1) * cos(lat1 / 180.0 * M_PI) * cos(lon1 / 180.0 * M_PI);
    y1 = (a * c + alt1) * cos(lat1 / 180.0 * M_PI) * sin(lon1 / 180.0 * M_PI);
    z1 = (a * s + alt1) * sin(lat1 / 180.0 * M_PI);
    c = 1 / sqrt((pow(cos(lat2 / 180.0 * M_PI), 2)) +
                 pow(1 - f, 2) * (pow(sin(lat2 / 180.0 * M_PI), 2)));
    /* s = pow(1 - f, 2) * c; */
    x2 = (a * c + alt2) * cos(lat2 / 180.0 * M_PI) * cos(lon2 / 180.0 * M_PI);
    y2 = (a * c + alt2) * cos(lat2 / 180.0 * M_PI) * sin(lon2 / 180.0 * M_PI);
    z2 = (a * s + alt2) * sin(lat2 / 180.0 * M_PI);

    return sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2) + pow(z2 - z1, 2));
}

#ifdef CONNECT_TO_SERVICE
static void printGpsCoordinatesAndDistance(double wsuLat, double wsuLong,
    double wsuElev, double rseLat, double rseLong, double rseElev)
{
    printf("WSU coordinates: lat = %.7lf, long = %.7lf, elev = %.1lf\n",
        wsuLat, wsuLong, wsuElev);

    printf("RSE coordinates: lat = %.7lf, long = %.7lf, elev = %.1lf\n",
        rseLat, rseLong, rseElev);

    printf("Distance = %lfm\n", getRelativeDistance(
        wsuLat, wsuLong, rseLat, rseLong));
}
#endif // CONNECT_TO_SERVICE

static void serviceAvailable(serviceInfoType *serviceInfo)
{
    rsResultCodeType risRet;
#ifndef CONNECT_TO_SERVICE

    /* Print out what SERVICE_AVAILABLE and SERVICE_NOT_AVAILABLE indications come
     * up, and optionally print the service info data */
    if (serviceInfo->serviceAvail == RS_SERVICE_NOT_AVAILABLE) {
        printf("SERVICE_NOT_AVAILABLE\n");
    }
    else if (serviceInfo->serviceAvail == RS_SERVICE_AVAILABLE) {
        printf("SERVICE_AVAILABLE\n");

        if (printWsa == 1) {
            printMinimalServiceInfo(serviceInfo);
        }
        else if (printWsa == 2) {
            printServiceInfo(serviceInfo);
        }
    }
    else {
        printf("Unknown serviceInfo->serviceAvail value (%d)\n", serviceInfo->serviceAvail);
    }

    if (verifyWsa) {
        if ((serviceInfo->serviceBitmask & (WSA_SIGNED | WSA_VERIFIED)) == WSA_SIGNED) {
            /* If we already have a previous WSA submitted for verification;
             * ignore this one. */
            if (wsaSubmittedForVerification == TRUE) {
                return;
            }

            /* Submit the WSA for verification. */
            if (localTpsError == 0) {
                wsaSubmittedForVerification = TRUE;
                risRet = wsuRisVerifyWsaRequest(serviceInfo->signedWsaHandle);

                if (risRet != RS_SUCCESS) {
                    printf("wsuRisVerifyWsaRequest() failed (%s)\n",
                           rsResultCodeType2Str(risRet));
                }
            }
        }
        else if ((serviceInfo->serviceBitmask & (WSA_SIGNED | WSA_VERIFIED)) == (WSA_SIGNED | WSA_VERIFIED)) {
            printf("WSA verified successfully!\n");
            wsaSubmittedForVerification = FALSE;
        }
    }
#else // CONNECT_TO_SERVICE
    UserServiceChangeType    cfg;
    int32_t                  rseLatitude;
    int32_t                  rseLongitude;
    int32_t                  rseElevation;

    if ((serviceInfo->serviceAvail == RS_SERVICE_NOT_AVAILABLE) &&
        (wBSSJoined == TRUE) && (currentExtendedAccess == 0) &&
        (memcmp(wBSSMacAddress, serviceInfo->sourceMac, LENGTH_MAC) == 0)) {
        printf("\nsvcAvailable=SERVICE_NOT_AVAILABLE "
               "mac=%02x:%02x:%02x:%02x:%02x:%02x\n",
               serviceInfo->sourceMac[0], serviceInfo->sourceMac[1],
               serviceInfo->sourceMac[2], serviceInfo->sourceMac[3],
               serviceInfo->sourceMac[4], serviceInfo->sourceMac[5]);

        /* Tear down IPv6 setup */
        ipv6_deinit();

        cfg.psid                       = uService.psid;
        cfg.command                    = CHANGE_ACCESS;
        cfg.user.access.channelNumber  = wsachannel;
        cfg.user.access.adaptable      = 0;
        cfg.user.access.dataRate       = 6;
        cfg.user.access.txPwrLevel     = 20;
        cfg.user.access.userAccess     = NO_SCH_ACCESS;
        cfg.user.access.extendedAccess = 0;
        risRet = changeUserServiceRequest(&cfg);

        if (risRet != RS_SUCCESS) {
            printf("changeUserServiceRequest failed (%s)\n",
                   rsResultCodeType2Str(risRet));
        }

        wBSSJoined = FALSE;
    }
    else if ((serviceInfo->serviceAvail == RS_SERVICE_AVAILABLE) &&
             (wBSSJoined == FALSE)) {
        if ((matchMacAddr == TRUE) &&
            (memcmp(serviceInfo->sourceMac, macAddr, LENGTH_MAC) != 0)) {
            return;
        }

        /* See that we have either 3D or 2D location field; error if we do not */
        if ((serviceInfo->WSABitmask & WSA_EXT_3DLOCATION_BITMASK) ==
            WSA_EXT_3DLOCATION_BITMASK) {
            rseLatitude = serviceInfo->latitude3D;
            rseLongitude = serviceInfo->longitude3D;
            rseElevation = serviceInfo->elevation3D;
        }
        else if ((serviceInfo->WSABitmask & WSA_EXT_2DLOCATION_BITMASK) ==
            WSA_EXT_2DLOCATION_BITMASK) {
            rseLatitude = serviceInfo->latitude2D;
            rseLongitude = serviceInfo->longitude2D;
            rseElevation = 0;
        }
        else {
            printf("Verify WSA failed due to no 2D or 3D location in WSA: Error code = 0x11a\n");
            return;
        }

        if (getRelativeDistance(tpsUserData.latitude, tpsUserData.longitude,
            (double)rseLatitude / 1e7, (double)rseLongitude / 1e7) > (double)maxRSERange) {
            printGpsCoordinatesAndDistance(tpsUserData.latitude,
                tpsUserData.longitude, tpsUserData.altitude,
                (double)rseLatitude / 1e7, (double)rseLongitude / 1e7,
                (double)rseElevation / 10);
            printf("Verify WSA failed due to distance: Error code = 0x3a\n");
            return;
        }

        if (serviceInfo->rss < minRssi) {
            printf("RSSI = %d\n", serviceInfo->rss);
            printf("Verify WSA failed due to weak RSS: Error code = 0x11b\n");
            return;
        }

        if (((serviceInfo->serviceBitmask & (WSA_SIGNED | WSA_VERIFIED)) ==
             WSA_SIGNED) && verifyWsa) {
            /* We have a signed but not verified WSA. */
            int wsaVerificationFailedIndex;

            /* If we already have a previous WSA submitted for verification;
             * ignore this one. */
            if (wsaSubmittedForVerification == TRUE) {
                return;
            }

            /* If a previous WSA failed verification, and not enough time has
             * elapsed, ignore this one. */
            if (((wsaVerificationFailedIndex =
                getWsaVerificationFailedIndex(serviceInfo->sourceMac)) != -1)) {
                struct timeval ts;

                gettimeofday(&ts, NULL);

                if (diffSecs(&ts, &wsaVerificationFailedTimeStamp[wsaVerificationFailedIndex]) <
                    wsaVerificationFailedTimeoutSecs) {
                    /* Not enough time has elapsed. */
                    return;
                }

                wsaVerificationFailed[wsaVerificationFailedIndex] = FALSE;
            }

            /* Submit the WSA for verification. */
            if (localTpsError == 0) {
                wsaSubmittedForVerification = TRUE;
                memcpy(wsaSubmittedForVerificationMacAddr, serviceInfo->sourceMac,
                       LENGTH_MAC);
                risRet = wsuRisVerifyWsaRequest(serviceInfo->signedWsaHandle);

                if (risRet != RS_SUCCESS) {
                    printf("wsuRisVerifyWsaRequest() failed (%s)\n",
                           rsResultCodeType2Str(risRet));
                }
            }
        }
        else {
            /* If we submitted a WSA for verification, and the verification was
             * successful, but it was overridden by an unsigned WSA, ignore this
             * one. */
            if (((serviceInfo->serviceBitmask & WSA_SIGNED) == WSA_SIGNED) &&
                verifyWsa && (wsaSubmittedForVerification == FALSE)) {
                return;
            }

            /* Ignore if invalid channel number in WSA. */
            if ((serviceInfo->channelNumber != 84) &&
                (serviceInfo->channelNumber != 88) &&
                (serviceInfo->channelNumber != 164) &&
                (serviceInfo->channelNumber != 168)) {
                if ((serviceInfo->channelNumber < 172)  ||
                    (serviceInfo->channelNumber > 184)) {
                    return;
                }
            }
            wsaSubmittedForVerification = FALSE;
            printf("\nsvcAvailable=SERVICE_AVAILABLE "
                   "mac=%02x:%02x:%02x:%02x:%02x:%02x\n",
                   serviceInfo->sourceMac[0], serviceInfo->sourceMac[1],
                   serviceInfo->sourceMac[2], serviceInfo->sourceMac[3],
                   serviceInfo->sourceMac[4], serviceInfo->sourceMac[5]);

            /* Accept Service */
            cfg.psid                       = uService.psid;
            cfg.command                    = CHANGE_ACCESS;
            cfg.user.access.channelNumber  = serviceInfo->channelNumber;
            cfg.user.access.adaptable      = serviceInfo->adaptable;
            cfg.user.access.dataRate       = serviceInfo->dataRate;
            cfg.user.access.txPwrLevel     = serviceInfo->txPwrLevel;
            cfg.user.access.userAccess     = AUTO_ACCESS_UNCONDITIONAL;
            cfg.user.access.extendedAccess = (serviceInfo->channelAccess == 0) ?
                                             65535 : 0;
            risRet = changeUserServiceRequest(&cfg);

            if (risRet != RS_SUCCESS) {
                printf("changeUserServiceRequest failed (%d)\n", risRet);
                return;
            }

            printf("WSA Detected (PSID = 0x%x)\n", uService.psid);

            if ((serviceInfo->serviceBitmask & (WSA_SIGNED | WSA_VERIFIED)) ==
                (WSA_SIGNED | WSA_VERIFIED)) {
                printf("Verify WSA succeeded.\n");
            }

            /* Setup the IPv6 communication */
            ipv6_init(serviceInfo);

            if (printWsa == 1) {
                printMinimalServiceInfo(serviceInfo);
            }
            else if (printWsa == 2) {
                printServiceInfo(serviceInfo);
            }

            printGpsCoordinatesAndDistance(tpsUserData.latitude,
                tpsUserData.longitude, tpsUserData.altitude,
                (double)rseLatitude / 1e7, (double)rseLongitude / 1e7,
                (double)rseElevation / 10);
            printf("RSSI = %d\n", serviceInfo->rss);
            wBSSJoined = TRUE;
            memcpy(wBSSMacAddress, serviceInfo->sourceMac, LENGTH_MAC);
        }
    }
#endif // CONNECT_TO_SERVICE
}

void verifyWSAFail(uint16_t error_code)
{
#ifdef CONNECT_TO_SERVICE
    int wsaVerificationFailedIndex;
#endif // CONNECT_TO_SERVICE

    /* If we submitted a WSA for verification, but it was overridden by an
     * unsigned WSA, ignore the error. */
    if (wsaSubmittedForVerification == FALSE) {
        return;
    }

    wsaSubmittedForVerification = FALSE;
    printf("Verify WSA failed: Error code = %d\n", error_code);

#ifdef CONNECT_TO_SERVICE
    if ((wsaVerificationFailedIndex = getFreeWsaVerificationFailedIndex()) != -1) {
        wsaVerificationFailed[wsaVerificationFailedIndex] = TRUE;
        memcpy(wsaVerificationFailedMacAddr[wsaVerificationFailedIndex],
               wsaSubmittedForVerificationMacAddr, LENGTH_MAC);
        gettimeofday(&wsaVerificationFailedTimeStamp[wsaVerificationFailedIndex], NULL);
    }
#endif // CONNECT_TO_SERVICE
}

/* Call back function for Registering to get TPS data */
static void tpsCallback(tpsDataType *data)
{
    /* Copy the data and print it */
    if (data->valid) {
        tpsUserData.latitude  = data->latitude;
        tpsUserData.longitude = data->longitude;
        tpsUserData.altitude  = data->altitude;
        localTpsError         = 0;
    }
    else {
        tpsUserData.latitude  = 0;
        tpsUserData.longitude = 0;
        tpsUserData.altitude  = 0;
        localTpsError         = 1;
    }
}

uint8_t convert1HexDigit(char digit)
{
    if (digit >= '0' && digit <= '9') {
        return digit - '0';
    }
    else if (digit >= 'A' && digit <= 'F') {
        return digit - 'A' + 0xa;
    }
    else { // hopefully digit >= 'a' && digit <= 'f'
        return digit - 'a' + 0xa;
    }
}

uint8_t convert2HexDigits(char *ptr)
{
    return convert1HexDigit(ptr[0]) * 16 + convert1HexDigit(ptr[1]);
}

static int WsaParseArg(int argc, char *argv[])
{
#define MAX_CMD_BUF 9
#define MAX_CMDS    13
    int           i = 1;
    unsigned long temp;
    long  temp_signed=0;
    char          tempBuf[MAX_CMD_BUF];
    char         *pCommandArray[MAX_CMDS] = {
        "PSID",
        "RAD",
        "RXANT",
        "PRINTWSA",
        "VERWSA",
        "MAC",
        "DIST",
        "VFTO",
        "MINRSS",
        "SMIDEBUG",
        "WSASEC",
        "VERNRES",
        "CHANNEL",
    };

    if (argc <= 1) {
        return 1;
    }

    while (i < argc)
    {
        uint16_t index;
        uint8_t k;

        if (i >= argc) {
            return 0;
        }

        if (argv[i][0] != '-' || argv[i][1] != '-') {
            return 0;
        }

        bzero(tempBuf, MAX_CMD_BUF);

        for (k = 0; k < MAX_CMD_BUF && k < strlen(&argv[i][2]); k ++) {
            tempBuf[k] = (char)toupper((int)(argv[i][k+2]));
        }

        tempBuf[k] = '\0';

        for (index = 0; index < MAX_CMDS; index++) {
            if (strncmp((const char *)pCommandArray[index],
                (const char *)tempBuf, strlen(pCommandArray[index])) == 0) {
                break;
            }
        }

        if (index == MAX_CMDS) {
            return 0;
        }

        i++;

        if (i >= argc) {
            return 0;
        }


        if (index != 5) { // Don't convert to integer for MAC
            if ((argv[i][0] == '0') &&
                ((argv[i][1] == 'x') || (argv[i][1] == 'X'))) {
                /* Convert hexidecimal value */
                sscanf(&argv[i][2], "%lx", &temp);
            }
            else if (index == 8) { // MINRSS
                /* Convert to signed integer */
                temp_signed = strtol(argv[i], NULL, 0);
            }
            else {
                /* Convert to unsigned integer */
                temp = strtoul(argv[i], NULL, 0);
            }
        }

        switch (index) {
        case 0:     // PSID
            uService.psid = temp;
            break;

        case 1:     // RAD
            if (temp > 1) {
                return 0;
            }

            wsaRadio = temp;
            uService.radioNum = temp;
            break;

        case 2:     // RXANT
            if (temp > 2) { // 0: diversity, 1, 2
                return 0;
            }
            wsaRxAnt = (uint16_t)temp;
            break;

        case 3:     // PRINTWSA
            if (temp > 2) {
                return 0;
            }

            printWsa = temp;
            break;

        case 4:     // VERWSA
            if (temp > 1) {
                return 0;
            }

            verifyWsa = (temp == 0) ? FALSE : TRUE;
            uService.wsaSecurity = SECURED_WSA;
            break;

        case 5:     // MAC
            {
                char *ptr;
                int j;

                matchMacAddr = TRUE;
                ptr = argv[i];

                for (j = 0; j < 6; j++) {
                    macAddr[j] = convert2HexDigits(ptr);
                    ptr += 2;
                    if ((*ptr < '0' || *ptr > '9') &&
                        (*ptr < 'A' || *ptr > 'F') &&
                        (*ptr < 'a' || *ptr > 'f')) {
                        ptr++;
                    }
                }
            }
            break;

        case 6:     // DIST
            maxRSERange = temp;
            break;

        case 7:     // VFTO
            if (temp > 65535) {
                return 0;
            }

            wsaVerificationFailedTimeoutSecs = (int)temp;
            break;

        case 8:     // MINRSS
            if ((temp_signed < -127) || (temp_signed > 127)) {
                return 0;
            }

            minRssi = (int16_t)temp_signed;
            break;

        case 9:     // SMIDEBUG
            if (temp > 1) {
                return 0;
            }

            smiDebug = (temp == 0) ? FALSE : TRUE;
            break;

        case 10:    // WSASEC
            if (temp > 1) {
                return 0;
            }

            uService.wsaSecurity = (temp == 0) ? UNSECURED_WSA : SECURED_WSA;
            break;

        case 11: // VERNRES
            if (temp > 2) {
                return 0;
            }

            verifyResults = temp;
            break;

        case 12: // CHANNEL
            if ((temp > 184) || (temp < 172)) {
                return 0;
            }

            wsachannel = temp;
            uService.channelNumber = temp;
            break;

        default:
            return 0;
        }

        i++;
    }

    return 1;
}

/* Verify WSA results callback */
void verifyResultsCallback(uint16_t result)
{
    printf("Verify results = %u\n", result);
}

#if 0
/* Initializes security with debug turned on. */
static void securityInitialize(void)
{
    rsSecurityInitializeType init;

    init.debug                           = smiDebug ? TRUE : FALSE;
    init.sec_verify_approach             = smi_verify_none;
    init.smi_cert_change_callback        = NULL;
    init.vtp_seconds_per_verification    = 1;
    init.smi_sign_results_options        = smi_results_none;
    init.smi_sign_wsa_results_options    = smi_results_none;
    init.smi_sign_results_callback       = NULL;
    init.smi_sign_wsa_results_callback   = NULL;
    strcpy(init.lcmName, "");
    init.numSignResultsPSIDs             = 0;

    if (!verifyWsa) {
        if (verifyResults != 0) {
            printf("WARNING: --verwsa 1 must be specified to get verification results.\n");
        }

        init.smi_verify_wsa_results_options  = smi_results_none;
        init.smi_verify_wsa_results_callback = NULL;
    }
    else {
        init.smi_verify_wsa_results_options  =
            (verifyResults == 0) ? smi_results_none :
            (verifyResults == 1) ? smi_results_all  :
                                  smi_results_errors_only;
        init.smi_verify_wsa_results_callback = (verifyResults == 0) ? NULL :
                                               verifyResultsCallback;
    }

    if (smiInitialize(&init) == -1) {
        printf("Security initialization failed.\n");
        return;
    }

    smiInitialized = TRUE;
}
#endif

static void WsaShowSyntax(int argc, char *argv[])
{
    printf("Usage: %s [--psid (0-0xEFFFFFFF)] [--rad (0-1)] [--rxant (0-2)] "
           "[--printwsa (0-2)] [--verwsa (0-1)] [--mac aabbccddeeff] "
           "[--dist (0 - 4294967295)] [--vfto (0 - 65535)] "
           "[--minrss (-127 - 127)] [--smidebug (0-1)] "
           "[--vernres (0-2)]\n", argv[0]);
}

int initRadio(void)
{
    rsResultCodeType  risRet;
    cfgType risCfg;
    int i;
    unsigned char idx = 0;
    int32_t val;
    uint8_t *pval = (uint8_t *)&val;
    char buf[30];   /* pro tip: don't make it 'perfectly' sized; it can screw a future developer needing to refactor */
    FILE *pf;
    unsigned int seed[2] = {0};
    long long uuid;
    struct timeval tv;

    /* below command obtains lower 4 bytes of unique ID */
    if (NULL != (pf = popen("hexdump -n 72 -e ' 8/4 \"%02x\" \"\\n\" ' /sys/devices/platform/ocotp/imx-ocotp0/nvmem | tail -1", "r"))) {
        fgets(buf, sizeof(char)*sizeof(buf), pf);
        uuid = strtoull(buf, NULL, 16);
        pclose(pf);
        seed[0] = (unsigned int)uuid;
        seed[1] = (unsigned int)(uuid >> 32);
    } else {
        printf("failed to read unique ID; using time value instead\n");
        gettimeofday(&tv, NULL);
        seed[0] = (unsigned int)tv.tv_usec;    /* ignore second seed, just use one */
    }

    /* seed random() */
    srandom(seed[0]);
    for (i=0; i<LENGTH_MAC; i++) {
        val = random();
        idx = random()%4;
        risCfg.u.RadioMacAddress[i] = pval[idx];
        if ((i==2) && seed[1]) {
            /* reseed with different value */
            srandom(seed[1]);
        }
    }
    risCfg.u.RadioMacAddress[0] &= 0xFE;
    risCfg.u.RadioMacAddress[0] |= 0x02;

    risCfg.radioType = RT_DSRC;
    if (RS_SUCCESS != (risRet = wsuRisSetCfgReq(CFG_ID_MACADDRESS_RADIO_1, &risCfg))) {
        printf("failed to set IPv6 radio\n");
    }

    /* Start - Init Radio Communication */
    risRet = wsuRisInitRadioComm("ipv6_user", &appCredentialV2);

    if (risRet != RS_SUCCESS) {
        printf("wsuRisInitRadioComm failed (%s)\n",
               rsResultCodeType2Str(risRet));
        return 0;
    }

    return 1;
}

int main(int argc, char *argv[])
{
    rsResultCodeType  risRet;
    tpsResultCodeType tpsRet;

    /* Do any initialization first */
    if (WsaParseArg(argc, argv) == 0) {
        WsaShowSyntax(argc, argv);
        goto err;
    }

    /* Register with TPS */
    tpsRet = wsuTpsInit();

    if (tpsRet != TPS_SUCCESS) {
        printf("wsuTpsInit failed (%s)\n", tpsResultCodeType2Str(tpsRet));
        goto err;
    }

    tpsInited = TRUE;
    tpsRet = wsuTpsRegister(tpsCallback);

    if (tpsRet != TPS_SUCCESS) {
        printf("wsuTpsRegister failed (%s)\n", tpsResultCodeType2Str(tpsRet));
        goto err;
    }

    tpsRegistered = TRUE;

#if 0
    /* If SMI debugging selected, initialize security */
    if (smiDebug || verifyResults) {
        securityInitialize();
    }
#endif

    if (!initRadio()) {
        goto err;
    }
    radioCommInited = TRUE;

    /* Register Services */
    uService.action = ADD;
    risRet = wsuRisUserServiceRequest(&uService);

    if (risRet != RS_SUCCESS) {
        printf("wsuRisUserServiceRequest failed (%s)\n",
               rsResultCodeType2Str(risRet));
        goto err;
    }

#ifdef CONNECT_TO_SERVICE
    currentExtendedAccess = uService.extendedAccess;
#endif // CONNECT_TO_SERVICE
    printf("User Registered Service (PSID=0x%x)\n", uService.psid);

    userServiceInited = TRUE;
    signal(SIGINT,  wsaExitHandler);
    signal(SIGTERM, wsaExitHandler);
    signal(SIGKILL, wsaExitHandler);

    // ready to catch signal
    while (mainLoop) {
        sleep(1);
    }

    if (!mainLoop) {
        wsaCleanHandler();
    }

    return EXIT_SUCCESS;

err:
    mainLoop = FALSE;
    wsaCleanHandler();
    return EXIT_FAILURE;
}

