/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Filename: serviceAvailableTest.c                                           */
/*  Purpose: Simple sample program to transmit unsecured WSMs with no         */
/*           callbacks.                                                       */
/*                                                                            */
/* Copyright (C) 2019 DENSO International America, Inc.  All Rights Reserved. */
/*                                                                            */
/* Revision History:                                                          */
/*                                                                            */
/* Date        Author        Comments                                         */
/* -------------------------------------------------------------------------- */
/* 2019-06-21  VROLLINGER    Initial revision.                                */
/*                                                                            */
/*----------------------------------------------------------------------------*/
/* Starts two user services with PSID's specified in PSID1 and PSID2, both on
 * the channel specified in CHANNEL. Then it prints whatever service available
 * or service not available indications it gets.
 */
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include "ris.h"
#include "type2str.h"

#define PSID0         0x20
#define PSID1         0xEFFFFD01
#define PSID2         0xEFFFFD02
#define CHANNEL       172
#define OTHER_CHANNEL 174

void serviceAvailableCallback(serviceInfoType *service);

static bool_t radioCommInited     = FALSE;
static bool_t userService0Started = FALSE;
static bool_t userService1Started = FALSE;
static bool_t userService2Started = FALSE;
static bool_t mainLoop            = TRUE;

static appCredentialsType appCredential = {
    .cbServiceAvailable = serviceAvailableCallback,
    .cbReceiveWSM       = NULL,
};

static UserServiceType uService0 = {
    .radioType         = RT_DSRC,
    .radioNum          = 0,
    .action            = ADD,
    .userAccess        = AUTO_ACCESS_UNCONDITIONAL,
    .psid              = PSID0,
    .servicePriority   = 10,
    .wsaSecurity       = UNSECURED_WSA,
    .lengthPsc         = 10,
    .channelNumber     = CHANNEL,
    .lengthAdvertiseId = 15,
    .linkQuality       = 20,
    .immediateAccess   = 0,
    .extendedAccess    = 65535,
    .srcMacAddress     = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff},
};

static UserServiceType uService1 = {
    .radioType         = RT_DSRC,
    .radioNum          = 1,
    .action            = ADD,
    .userAccess        = NO_SCH_ACCESS,
    .psid              = PSID1,
    .servicePriority   = 10,
    .wsaSecurity       = UNSECURED_WSA,
    .lengthPsc         = 10,
    .channelNumber     = CHANNEL,
    .lengthAdvertiseId = 15,
    .linkQuality       = 20,
    .immediateAccess   = 0,
    .extendedAccess    = 0,
    .srcMacAddress     = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff},
};

static UserServiceType uService2 = {
    .radioType         = RT_DSRC,
    .radioNum          = 1,
    .action            = ADD,
    .userAccess        = NO_SCH_ACCESS,
    .psid              = PSID2,
    .servicePriority   = 10,
    .wsaSecurity       = UNSECURED_WSA,
    .lengthPsc         = 10,
    .channelNumber     = CHANNEL,
    .lengthAdvertiseId = 15,
    .linkQuality       = 20,
    .immediateAccess   = 0,
    .extendedAccess    = 0,
    .srcMacAddress     = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff},
};

/* Service (not) available callback function. Prints the contents of the
 * indication */
void serviceAvailableCallback(serviceInfoType *service)
{
    int i;

    printf("Indication received:\n");
    printf("serviceAvail       = %-5u        channelAvail              = %-5u              rss               = %-5d\n",
           service->serviceAvail, service->channelAvail, service->rss);

    if (service->serviceBitmask & WSA_EXT_PROVIDER_SERVICE_CONTEXT_BITMASK) {
        printf("serviceBitmask     = 0x%08x   sourceMac                 = %02x:%02x:%02x:%02x:%02x:%02x  servicePscLength  = %-5u\n",
               service->servicePscLength, service->sourceMac[0], service->sourceMac[1], service->sourceMac[2],
               service->sourceMac[3], service->sourceMac[4], service->sourceMac[5], service->serviceBitmask);
        printf("psc                = ");

        for (i = 0; i < service->servicePscLength; i++) {
            printf("%02x%c", service->psc[i], (i == service->servicePscLength - 1) ? '\n' : ' ');
        }
    }
    else {
        printf("serviceBitmask     = 0x%08x   sourceMac                 = %02x:%02x:%02x:%02x:%02x:%02x\n",
               service->servicePscLength, service->sourceMac[0], service->sourceMac[1], service->sourceMac[2],
               service->sourceMac[3], service->sourceMac[4], service->sourceMac[5]);
    }

    printf("psid               = 0x%08x   channelIndex              = %-5d\n", service->psid, service->channelIndex);

    if (service->serviceBitmask & WSA_EXT_IPV6ADDRESS_BITMASK) {
        printf("ipv6Address        = %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x\n",
            service->ipv6Address[0],  service->ipv6Address[1],  service->ipv6Address[2],  service->ipv6Address[3],
            service->ipv6Address[4],  service->ipv6Address[5],  service->ipv6Address[6],  service->ipv6Address[7],
            service->ipv6Address[8],  service->ipv6Address[9],  service->ipv6Address[10], service->ipv6Address[11],
            service->ipv6Address[12], service->ipv6Address[13], service->ipv6Address[14], service->ipv6Address[15]);
    }

    if (service->serviceBitmask & WSA_EXT_RCPI_THRESHOLD_BITMASK) {
        if (service->serviceBitmask & (WSA_EXT_SERVICE_PORT_BITMASK | WSA_EXT_PROVIDER_MACADDRESS_BITMASK)) {
            printf("port               = %-5u        providerMacAddress        = %02x:%02x:%02x:%02x:%02x:%02x  rcpiThreshold     = %-5d\n",
            service->port, service->providerMacAddress[0], service->providerMacAddress[1], service->providerMacAddress[2],
            service->providerMacAddress[3], service->providerMacAddress[4], service->providerMacAddress[5], service->rcpiThreshold);
        }
        else if (service->serviceBitmask & WSA_EXT_SERVICE_PORT_BITMASK) {
            printf("port               = %-5u        rcpiThreshold             = %-5d\n", service->port, service->rcpiThreshold);
        }
        else if (service->serviceBitmask & WSA_EXT_PROVIDER_MACADDRESS_BITMASK) {
            printf("providerMacAddress = %02x:%02x:%02x:%02x:%02x:%02x                                          rcpiThreshold     = %-5d\n",
                   service->providerMacAddress[0], service->providerMacAddress[1], service->providerMacAddress[2],
                   service->providerMacAddress[3], service->providerMacAddress[4], service->providerMacAddress[5],
                   service->rcpiThreshold);
        }
        else {
            printf("rcpiThreshold      = %-5d\n", service->rcpiThreshold);
        }
    }
    else {
        if (service->serviceBitmask & (WSA_EXT_SERVICE_PORT_BITMASK | WSA_EXT_PROVIDER_MACADDRESS_BITMASK)) {
            printf("port               = %-5u        providerMacAddress        = %02x:%02x:%02x:%02x:%02x:%02x\n",
                   service->port, service->providerMacAddress[0], service->providerMacAddress[1], service->providerMacAddress[2],
                   service->providerMacAddress[3], service->providerMacAddress[4], service->providerMacAddress[5]);
        }
        else if (service->serviceBitmask & WSA_EXT_SERVICE_PORT_BITMASK) {
            printf("port               = %-5u\n", service->port);
        }
        else if (service->serviceBitmask & WSA_EXT_PROVIDER_MACADDRESS_BITMASK) {
            printf("providerMacAddress = %02x:%02x:%02x:%02x:%02x:%02x\n",
                   service->providerMacAddress[0], service->providerMacAddress[1], service->providerMacAddress[2],
                   service->providerMacAddress[3], service->providerMacAddress[4], service->providerMacAddress[5]);
        }
    }

    if (service->serviceBitmask & (WSA_EXT_WSA_COUNT_THRESHOLD_BITMASK | WSA_EXT_WSA_COUNT_THRESHOLD_INTERVAL_BITMASK)) {
        printf("wsaCountThreshold  = %-5u        wsaCountThresholdInterval = %-5u              channelBitmask    = 0x%08x\n",
               service->wsaCountThreshold, service->wsaCountThresholdInterval, service->channelBitmask);
    }
    else if (service->serviceBitmask & WSA_EXT_WSA_COUNT_THRESHOLD_BITMASK) {
        printf("wsaCountThreshold  = %-5u        channelBitmask            = 0x%08x\n",
               service->wsaCountThreshold, service->channelBitmask);
    }
    else if (service->serviceBitmask & WSA_EXT_WSA_COUNT_THRESHOLD_INTERVAL_BITMASK) {
        printf("wsaCountThresholdInterval = %-5u                                               channelBitmask    = 0x%08x\n",
               service->wsaCountThresholdInterval, service->channelBitmask);
    }
    else {
        printf("channelBitmask     = 0x%08x\n", service->channelBitmask);
    }

    printf("operatingClass     = %-5u        channelNumber             = %-5u              adaptable         = %-5u\n",
           service->operatingClass, service->channelNumber, service->adaptable);
    printf("dataRate           = %-5u        txPwrLevel                = %-5u              qosInfo           = %-5u\n",
           service->dataRate, service->txPwrLevel, service->qosInfo);

    if (service->channelBitmask & WSA_EXT_EDCA_PARAM_SET_BITMASK) {
        printf("ecw                = %02x %02x %02x %02x  aifs                      = %02x %02x %02x %02x        txop              = %04x %04x %04x %04x\n",
               service->ecw[0],  service->ecw[1],  service->ecw[2],  service->ecw[3],
               service->aifs[0], service->aifs[1], service->aifs[2], service->aifs[3],
               service->txop[0], service->txop[1], service->txop[2], service->txop[3]);
    }

    if (service->channelBitmask & WSA_EXT_CHANNEL_ACCESS_BITMASK) {
        if (service->WSABitmask & WSA_EXT_ADVERTISER_ID_BITMASK) {
            printf("channelAccess      = %-5u        WSABitmask                = 0x%08x         advertiseIdLength = %-5u\n",
                   service->channelAccess, service->WSABitmask, service->advertiseIdLength);
            printf("advertiseId        = ");

            for (i = 0; i < service->advertiseIdLength; i++) {
                printf("%02x%c", service->advertiseId[i], (i == service->advertiseIdLength - 1) ? '\n' : ' ');
            }
        }
        else {
            printf("channelAccess      = %-5u        WSABitmask                = 0x%08x\n",
                   service->channelAccess, service->WSABitmask);
        }
    }
    else {
        if (service->WSABitmask & WSA_EXT_ADVERTISER_ID_BITMASK) {
            printf("WSABitmask         = 0x%08x   advertiseIdLength = %-5u\n",
                   service->WSABitmask, service->advertiseIdLength);
            printf("advertiseId        = ");

            for (i = 0; i < service->advertiseIdLength; i++) {
                printf("%02x%c", service->advertiseId[i],
                       (i == service->advertiseIdLength - 1) ? '\n' : ' ');
            }
        }
        else {
            printf("WSABitmask         = 0x%08x\n", service->WSABitmask);
        }
    }

    if (service->WSABitmask & WSA_EXT_REPEAT_RATE_BITMASK) {
        printf("changeCount        = %-5u        repeatRate                = %-5u              signedWsaHandle   = %-11d\n",
               service->changeCount, service->repeatRate, service->signedWsaHandle);
    }
    else {
        printf("changeCount        = %-5u        signedWsaHandle           = %-11d\n",
               service->changeCount, service->signedWsaHandle);
    }
    
    if (service->WSABitmask & WSA_EXT_2DLOCATION_BITMASK) {
        printf("latitude2D         = %-11d  longitude2D               = %-11d\n",
               service->latitude2D, service->longitude2D);
    }

    if (service->WSABitmask & WSA_EXT_3DLOCATION_BITMASK) {
        printf("latitude3D         = %-11d  longitude3D               = %-11d        elevation3D       = %-11d\n",
               service->latitude3D, service->longitude3D, service->elevation3D);
    }

    printf("routingBitmask     = 0x%08x   routerLifeTime            = %-5d              prefixLen         = %-5u\n",
           service->routingBitmask, service->routerLifeTime, service->prefixLen);

    if (service->routingBitmask & WSA_EXT_WRA_BITMASK) {
        printf("ipPrefix           = %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x\n",
            service->ipPrefix[0],  service->ipPrefix[1],  service->ipPrefix[2],  service->ipPrefix[3],
            service->ipPrefix[4],  service->ipPrefix[5],  service->ipPrefix[6],  service->ipPrefix[7],
            service->ipPrefix[8],  service->ipPrefix[9],  service->ipPrefix[10], service->ipPrefix[11],
            service->ipPrefix[12], service->ipPrefix[13], service->ipPrefix[14], service->ipPrefix[15]);
        printf("defaultGateway     = %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x\n",
            service->defaultGateway[0],  service->defaultGateway[1],  service->defaultGateway[2],  service->defaultGateway[3],
            service->defaultGateway[4],  service->defaultGateway[5],  service->defaultGateway[6],  service->defaultGateway[7],
            service->defaultGateway[8],  service->defaultGateway[9],  service->defaultGateway[10], service->defaultGateway[11],
            service->defaultGateway[12], service->defaultGateway[13], service->defaultGateway[14], service->defaultGateway[15]);
        printf("primaryDns         = %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x\n",
            service->primaryDns[0],  service->primaryDns[1],  service->primaryDns[2],  service->primaryDns[3],
            service->primaryDns[4],  service->primaryDns[5],  service->primaryDns[6],  service->primaryDns[7],
            service->primaryDns[8],  service->primaryDns[9],  service->primaryDns[10], service->primaryDns[11],
            service->primaryDns[12], service->primaryDns[13], service->primaryDns[14], service->primaryDns[15]);

        if (service->routingBitmask & WSA_EXT_SECONDARY_DNS_BITMASK) {
            printf("secondaryDns       = %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x\n",
                service->secondaryDns[0],  service->secondaryDns[1],  service->secondaryDns[2],  service->secondaryDns[3],
                service->secondaryDns[4],  service->secondaryDns[5],  service->secondaryDns[6],  service->secondaryDns[7],
                service->secondaryDns[8],  service->secondaryDns[9],  service->secondaryDns[10], service->secondaryDns[11],
                service->secondaryDns[12], service->secondaryDns[13], service->secondaryDns[14], service->secondaryDns[15]);
        }

        if (service->routingBitmask & WSA_EXT_GATEWAY_MACADDRESS_BITMASK) {
            printf("gatewayMacAddress  = %02x:%02x:%02x:%02x:%02x:%02x\n",
                service->gatewayMacAddress[0],  service->gatewayMacAddress[1],  service->gatewayMacAddress[2],
                service->gatewayMacAddress[3],  service->gatewayMacAddress[4],  service->gatewayMacAddress[5]);
        }
    }

    printf("\n");
}

int init(void)
{
    rsResultCodeType risRet;

    risRet = wsuRisInitRadioComm("serviceAvailableTest", &appCredential);

    /* Init radio comm */
    if (risRet != RS_SUCCESS) {
        printf("wsuRisInitRadioComm failed (%s)\n", rsResultCodeType2Str(risRet));
        return -1;
    }

    radioCommInited = TRUE;
    printf("RadioComm Initialized\n");

    /* Start up user service that allows us to receive WSAs on channel 172 */
    uService0.action = ADD;
    risRet = wsuRisUserServiceRequest(&uService0);

    if (risRet != RS_SUCCESS) {
        printf("wsuRisUserServiceRequest (ADD) failed (%s)\n", rsResultCodeType2Str(risRet));
        return -1;
    }

    userService0Started = TRUE;

    /* Start up 2 user services corresponding to the PSIDs in the WSAs */
    uService1.action = ADD;
    risRet = wsuRisUserServiceRequest(&uService1);

    if (risRet != RS_SUCCESS) {
        printf("wsuRisUserServiceRequest (ADD) failed (%s)\n", rsResultCodeType2Str(risRet));
        return -1;
    }

    userService1Started = TRUE;
    printf("User service 1 started\n");

    uService2.action = ADD;
    risRet = wsuRisUserServiceRequest(&uService2);

    if (risRet != RS_SUCCESS) {
        printf("wsuRisUserServiceRequest (ADD) failed (%s)\n", rsResultCodeType2Str(risRet));
        return -1;
    }

    userService2Started = TRUE;
    printf("User service 2 started\n");
    return 0;
}

void term(void)
{
    rsResultCodeType risRet;

    /* Terminate user services */
    if (userService2Started) {
        uService2.action = DELETE;
        risRet = wsuRisUserServiceRequest(&uService2);

        if (risRet != RS_SUCCESS) {
            printf("wsuRisUserServiceRequest (DELETE) failed (%s)\n", rsResultCodeType2Str(risRet));
        }
        else {
            printf("User service 2 stopped\n");
        }
    }

    if (userService1Started) {
        uService1.action = DELETE;
        risRet = wsuRisUserServiceRequest(&uService1);

        if (risRet != RS_SUCCESS) {
            printf("wsuRisUserServiceRequest (DELETE) failed (%s)\n", rsResultCodeType2Str(risRet));
        }
        else {
            printf("User service 1 stopped\n");
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

/* Control-C handler */
void exitHandler(int signum)
{
    if (mainLoop) {
        printf("ExitHandler is called.\n");
        mainLoop = FALSE;
    }
}

int main(int argc, char **argv)
{
    bool_t retVal = 0;
    int count     = 0;
    UserServiceChangeType change =
    {
        .psid                        = PSID0,
        .command                     = CHANGE_ACCESS,
        .user.access.channelNumber   = OTHER_CHANNEL,
        .user.access.adaptable       = 0,
        .user.access.dataRate        = 6,
        .user.access.txPwrLevel      = 20,
        .user.access.userAccess      = AUTO_ACCESS_UNCONDITIONAL,
        .user.access.extendedAccess = 65535,
    };
    rsResultCodeType risRet;

    if (init() != 0) {
        retVal = -1;
        goto exit;
    }

    signal(SIGINT,  exitHandler);
    signal(SIGTERM, exitHandler);

    while (mainLoop) {
        sleep(1);

        if (++count == 10) {
            risRet = wsuRisChangeUserServiceRequest(&change);
            printf("wsuRisChangeUserServiceRequest() returned %s\n", rsResultCodeType2Str(risRet));
        }
    }

exit:
    term();
    return retVal;
}

