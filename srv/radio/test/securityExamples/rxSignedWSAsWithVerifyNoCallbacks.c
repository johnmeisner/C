/*
 * rxSignedWSAsWithVerifyNoCallbacks
 *
 * Receives signed WSA's with verification once per second. No calls to
 * smiInitialize() and smiTerm() are needed.
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
    .cbReceiveWSM       = NULL
};

static UserServiceType uService = {
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

static bool_t  radioCommInited     = FALSE;
static bool_t  userServiceStarted  = FALSE;
static bool_t  mainLoop            = TRUE;
static uint32_t signedWsaHandle     = 0;

static void serviceAvailable(serviceInfoType *serviceInfo)
{
    if (serviceInfo->serviceAvail == RS_SERVICE_NOT_AVAILABLE) {
        printf("serviceAvail == RS_SERVICE_NOT_AVAILABLE\n");
        signedWsaHandle = 0;
    }
    else if ((serviceInfo->serviceBitmask & (WSA_SIGNED | WSA_VERIFIED)) ==
        (WSA_SIGNED | WSA_VERIFIED)) {
        printf("Verify WSA succeeded.\n");
    }
    else {
        signedWsaHandle = serviceInfo->signedWsaHandle;
    }
}

int init(void)
{
    rsResultCodeType risRet;

    risRet = wsuRisInitRadioComm("rxSignedWSAsWithVerifyNoCallbacks", &appCredential);

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

            if (signedWsaHandle != 0) {
                wsuRisVerifyWsaRequest(signedWsaHandle);
            }
        }

        usleep(100000);
    }

exit:
    term();
    return retVal;
}

