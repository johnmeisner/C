/*
 * rxSignedWSAsWithVerifyWithCallbacks
 *
 * Receives signed WSA's with verification once per second. The program gets
 * WSA verify results callbacks. Calls to smiInitialize() and smiTerm() are
 * required.
 *
 * Only receiving WSA's on the DSRC radio is currently supported.
 *
 * Note that this program uses the same channel number as for WSM's (generally
 * 172), gives a user access of AUTO_ACCESS_UNCONDITIONAL, and uses
 * extendedAccess of 65535. This will configure the radio for channel 172 for
 * continuous access. This is in line with the scheme generally used where
 * WSA's are received on the same channel as WSM's.
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include "ris.h"
#include "alsmi_api.h"
#include "param.h"
#include "type2str.h"

static void serviceAvailable(serviceInfoType *serviceInfo);

static appCredentialsType appCredential = {
    .cbServiceAvailable = serviceAvailable,
    .cbReceiveWSM     = NULL
};

static UserServiceType uService1 = {
    .radioType         = RT_DSRC, /* Only DSRC is supported for WSA reception */
    .radioNum          = 0,
    .userAccess        = AUTO_ACCESS_UNCONDITIONAL,
    .psid              = SERVICE_PSID,
    .servicePriority   = 10,
    .wsaSecurity       = SECURED_WSA,
    .lengthPsc         = 10,
    .channelNumber     = CHANNEL,
    .lengthAdvertiseId = 15,
    .linkQuality       = 20,
    .immediateAccess   = 0,
    .extendedAccess    = 65535,
};

static UserServiceType uService2 = {
    .radioNum          = 1,
    .userAccess        = NO_SCH_ACCESS,
    .psid              = SERVICE_PSID2,
    .servicePriority   = 10,
    .wsaSecurity       = SECURED_WSA,
    .lengthPsc         = 10,
    .channelNumber     = 178,
    .lengthAdvertiseId = 15,
    .linkQuality       = 20,
    .immediateAccess   = 0,
    .extendedAccess    = 0,
    .srcMacAddress     = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff},
};

static bool_t   radioCommInited     = FALSE;
static bool_t   userService1Started = FALSE;
static bool_t   userService2Started = FALSE;
static bool_t   securityInitialized = FALSE;
static uint32_t signedWsaHandle1    = 0;
static uint32_t signedWsaHandle2    = 0;
static bool_t   mainLoop            = TRUE;

static void serviceAvailable(serviceInfoType *serviceInfo)
{
    char integer[128];
    
    sprintf(integer, "%d", serviceInfo->serviceAvail);
    printf("serviceAvail = %s; ", (serviceInfo->serviceAvail == RS_SERVICE_NOT_AVAILABLE) ? "RS_SERVICE_NOT_AVAILABLE" :
                                   (serviceInfo->serviceAvail == RS_SERVICE_AVAILABLE) ? "RS_SERVICE_AVAILABLE" : integer);
    sprintf(integer, "0x%x", serviceInfo->serviceBitmask);
    printf("serviceBitmask = %s; ", ((serviceInfo->serviceBitmask & (WSA_SIGNED | WSA_VERIFIED)) == (WSA_SIGNED | WSA_VERIFIED)) ? "WSA_SIGNED | WSA_VERIFIED" :
                                    ((serviceInfo->serviceBitmask & (WSA_SIGNED | WSA_VERIFIED)) == WSA_SIGNED) ? "WSA_SIGNED" : integer);
    printf("psid = 0x%x\n", serviceInfo->psid);
    
    if (serviceInfo->psid == SERVICE_PSID) {
        if (serviceInfo->serviceAvail == RS_SERVICE_NOT_AVAILABLE) {
            signedWsaHandle1 = 0;
        }
        else if ((serviceInfo->serviceBitmask & (WSA_SIGNED | WSA_VERIFIED)) ==
            (WSA_SIGNED | WSA_VERIFIED)) {
        }
        else {
            signedWsaHandle1 = serviceInfo->signedWsaHandle;
        }
    }
    else {
        if (serviceInfo->serviceAvail == RS_SERVICE_NOT_AVAILABLE) {
            signedWsaHandle2 = 0;
        }
        else if ((serviceInfo->serviceBitmask & (WSA_SIGNED | WSA_VERIFIED)) ==
            (WSA_SIGNED | WSA_VERIFIED)) {
        }
        else {
            signedWsaHandle2 = serviceInfo->signedWsaHandle;
        }
    }
}

/* Verify WSA results callback */
void verifyWsaResultsCallback(uint16_t results)
{
    printf("Verify results = %u\n", results);
}

int init(void)
{
    rsResultCodeType         risRet;
    int                      retVal;
    rsSecurityInitializeType init;

    risRet = wsuRisInitRadioComm("rxSignedWSAsWithVerifyWithCallbacks", &appCredential);

    /* Init radio comm */
    if (risRet != RS_SUCCESS) {
        printf("wsuRisInitRadioComm failed (%s)\n", rsResultCodeType2Str(risRet));
        return -1;
    }

    radioCommInited = TRUE;
    printf("RadioComm Initialized\n");

    /* Start up user services */
    uService1.action = ADD;
    risRet = wsuRisUserServiceRequest(&uService1);

    if (risRet != RS_SUCCESS) {
        printf("wsuRisUserServiceRequest (ADD) 1 failed (%s)\n", rsResultCodeType2Str(risRet));
        return -1;
    }

    userService1Started = TRUE;
    printf("User service 1 started\n");

    uService2.action = ADD;
    risRet = wsuRisUserServiceRequest(&uService2);

    if (risRet != RS_SUCCESS) {
        printf("wsuRisUserServiceRequest (ADD) 2 failed (%s)\n", rsResultCodeType2Str(risRet));
        return -1;
    }

    userService2Started = TRUE;
    printf("User service 2 started\n");

    /* Initialize the security services */
    init.debug                             = FALSE;
    init.vodEnabled                        = TRUE;
    init.smi_cert_change_callback          = NULL;
    init.vtp_milliseconds_per_verification = 0;
    init.smi_sign_results_options          = smi_results_none;
    init.smi_sign_results_callback         = NULL;
    init.smi_sign_wsa_results_options      = smi_results_none;
    init.smi_sign_wsa_results_callback     = NULL;
    init.smi_verify_wsa_results_options    = smi_results_all;
    init.smi_verify_wsa_results_callback   = verifyWsaResultsCallback;
    strcpy(init.lcmName, ""); // lcmName = NULL to signify no signing, only verification
    init.numSignResultsPSIDs               = 0;
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

    /* Terminate user services */
    if (userService2Started) {
        uService2.action = DELETE;
        risRet = wsuRisUserServiceRequest(&uService1);

        if (risRet != RS_SUCCESS) {
            printf("wsuRisUserServiceRequest (DELETE) 2 failed (%s)\n", rsResultCodeType2Str(risRet));
        }
        else {
            printf("User service 2 stopped\n");
        }
    }

    if (userService1Started) {
        uService1.action = DELETE;
        risRet = wsuRisUserServiceRequest(&uService1);

        if (risRet != RS_SUCCESS) {
            printf("wsuRisUserServiceRequest (DELETE) 1 failed (%s)\n", rsResultCodeType2Str(risRet));
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

void exitHandler(int signum)
{
    if (mainLoop) {
        printf("ExitHandler is called.\n");
        mainLoop = FALSE;
    }
}

int main(int argc, char **argv)
{
    int retVal = 0;
    int count  = 0;

    if (init() != 0) {
        retVal = -1;
        goto exit;
    }

    signal(SIGINT,  exitHandler);
    signal(SIGTERM, exitHandler);

    /* Verify the WSA once a second, until user terminates */
    while (mainLoop) {
        count++;

        if (count == 10) {
            count = 0;

            if (signedWsaHandle1 != 0) {
                wsuRisVerifyWsaRequest(signedWsaHandle1);
            }

            if (signedWsaHandle2 != 0) {
                wsuRisVerifyWsaRequest(signedWsaHandle2);
            }
        }

        usleep(100000);
    }

exit:
    term();
    return retVal;
}

