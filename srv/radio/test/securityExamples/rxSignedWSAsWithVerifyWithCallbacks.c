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
#include <stdlib.h>
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

static bool_t   EnableVOD            = FALSE;
static uint16_t SecurityVtPMsgRateMs = 50;
static bool_t   radioCommInited      = FALSE;
static bool_t   userServiceStarted   = FALSE;
static bool_t   securityInitialized  = FALSE;
static uint32_t signedWsaHandle      = 0;
static bool_t   mainLoop             = TRUE;

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

    /* Start up user service */
    uService.action = ADD;
    risRet = wsuRisUserServiceRequest(&uService);

    if (risRet != RS_SUCCESS) {
        printf("wsuRisUserServiceRequest (ADD) failed (%s)\n", rsResultCodeType2Str(risRet));
        return -1;
    }

    userServiceStarted = TRUE;
    printf("User service started\n");

    /* Initialize the security services */
    init.debug                             = FALSE;
    init.vodEnabled                        = EnableVOD;
    init.smi_cert_change_callback          = NULL;
    init.vtp_milliseconds_per_verification = SecurityVtPMsgRateMs;
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

void parseCommonConf(void)
{
    FILE *fd;
    char  line[128];
    int   i;
    
    fd = fopen("/rwflash/configs/v2x/common.conf", "r");
    
    if (fd != NULL) {
        while (fgets(line, 128, fd) != NULL) {
            if (strncmp(line, "EnableVOD", 9) == 0) {
                sscanf(line, "EnableVOD = %d", &i);
                EnableVOD = (i != 0);
            }
            else if (strncmp(line, "SecurityVtPMsgRateMs", 20) == 0) {
                sscanf(line, "SecurityVtPMsgRateMs = %u", &SecurityVtPMsgRateMs);
                break; // Should be nothing of interest after this
            }
        }

        fclose(fd);
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

    parseCommonConf();
    printf("VJR EnableVOD            = %s\n", EnableVOD ? "TRUE" : "FALSE");
    printf("VJR SecurityVtPMsgRateMs = %u\n", SecurityVtPMsgRateMs);

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

