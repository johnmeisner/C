/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Filename: ris.c                                                            */
/*                                                                            */
/* Copyright (C) 2022 DENSO International America, Inc.                       */
/*                                                                            */
/* Description: Radio Interface Services (RIS API for Radio Services)         */
/* Revision History:                                                          */
/*                                                                            */
/* Date        Author        Comments                                         */
/* -------------------------------------------------------------------------- */
/* [2019-03-07][VROLLINGER]  Initial revision                                 */
/*----------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <arpa/inet.h>
#include "ris.h"
#include "rs.h"
#include "ipcsock.h"
#if defined(ENABLE_SR)
#include "sr.h"
#include "ris_sr.h"
#endif
#include "type2str.h"
#include "ris_dbg.h"
#include "alsmi_api.h"

/*----------------------------------------------------------------------------*/
/* Constants                                                                  */
/*----------------------------------------------------------------------------*/
#ifdef CHINA_FREQS_ONLY
static const uint8_t g_valid_ch_nums[] = {147, 149, 151, 153, 155, 157, 159, 161, 163, 165, 167, 169};
#else
static const uint8_t g_valid_ch_nums[] = {178, 172, 173, 174, 175, 176, 177, 179, 180, 181, 182, 183, 184, 156, 164, 168};
#endif

/*----------------------------------------------------------------------------*/
/* Local variables                                                            */
/*----------------------------------------------------------------------------*/
static pid_t           ris_pid              = 0; // App pid
static uint8_t         g_numberCv2xRadios   = 2; // Number of C-V2X Radios
static uint8_t         g_numberDsrcRadios   = 2; // Number of DSRC Radios
static bool_t          radioCommInitialized = FALSE;
static int             radioServicesSock    = -1;
static pthread_mutex_t radioServicesMutex   = PTHREAD_MUTEX_INITIALIZER;
static bool_t          radioServicesMutexInited = FALSE;
static pthread_t       wsuRisReceiveIndicationsThreadId;
static bool_t          wsuRisReceiveIndicationsThreadRunning = FALSE;
static int             recvIndSock = -1;

/*----------------------------------------------------------------------------*/
/* Global variables                                                           */
/*----------------------------------------------------------------------------*/
#if defined(ENABLE_SR)
char               sr_app_name[MAX_RIS_APP_NAME_LEN];
#endif
appCredentialsType appCredentialsRisInternal;
uint32_t           debug_cnt_process_wsm;

/*----------------------------------------------------------------------------*/
/* Local Function Prototypes                                                  */
/*----------------------------------------------------------------------------*/
static int32_t communicateWithRadioServices(void *msg, size_t size, void *reply,
                                            size_t replySize);
static rsResultCodeType initRPS(bool_t serviceAvailableInd,
                                bool_t receiveWSMInd);
static rsResultCodeType releaseRPS(void);
static rsResultCodeType createReceiveIndicationsThread(void);
static rsResultCodeType terminateReceiveIndicationsThread(void);

/*----------------------------------------------------------------------------*/
/* Functions                                                                  */
/*----------------------------------------------------------------------------*/

/***********************************************************************************
 * This function inquires the number of radios of a particular type are
 * available in the WSU devices.
 *
 * Parameters: radioType: RT_CV2X or RT_DSRC
 *             *radios: returns number of radios (1 or 2)
 *
 * Returns - Result code defined by rsResultCodeType
 **********************************************************************************/
rsResultCodeType wsuRisGetNumRadios(rsRadioType radioType,
                                    uint8_t *num_radios)
{
    RIS_DBG("radioType = %s\n", (radioType == RT_CV2X) ? "RT_CV2X" : "RT_DSRC");

    switch (radioType) {
    case RT_CV2X:
        *num_radios = 1;    // The Hercules unit has 1 C-V2X Radio.
        break;

    case RT_DSRC:
        *num_radios = 2;    // The Hercules unit has 2 DSRC Radios.
        break;

    default:                // Invalid radioType
        RIS_DBG("Invalid radio type (%d)\n", radioType);
        return RS_ERANGE;
    }

    RIS_DBG("*num_radios = %u\n", *num_radios);
    return RS_SUCCESS;
}

/*******************************************************************************
 * This function initializes Radio Communication for the calling application.
 * It must be called when using the Full-Use version of the WAVE protocol
 * stack. The application must call this API function before any other API
 * function call.
 *
 * Parameters:
 *   appCredential - Application specific parameters defined by
 *                   appCredentialsType
 *
 * Returns - Result code defined by rsResultCodeType
 **********************************************************************************/
rsResultCodeType wsuRisInitRadioComm(char *app_abbrev,
                                     appCredentialsType *appCredential)
{
    rsResultCodeType risRet;

    RIS_DBG("app_abbrev = %s\n", app_abbrev);

#if defined(ENABLE_SR)
    // Copy our enclosing app's name for use later in ris_sr
    strncpy(sr_app_name, app_abbrev, MAX_RIS_APP_NAME_LEN - 1);
    sr_app_name[MAX_RIS_APP_NAME_LEN - 1] = 0;
#endif

    /* Get our application's PID */
    ris_pid = getpid();

    /* Set the call back function pointers */
    appCredentialsRisInternal = *appCredential;

    /* Initialize Radio Protocol Stack (RPS) */
    RIS_DBG("Calling initRPS(%d, %d)\n",
            (appCredential->cbServiceAvailable != NULL),
            (appCredential->cbReceiveWSM       != NULL));
    risRet = initRPS((appCredential->cbServiceAvailable != NULL),
                     (appCredential->cbReceiveWSM       != NULL));

    if (risRet != RS_SUCCESS) {
        return risRet;
    }

    /* VJR WILLBEREVISITED See if this delay is still needed */
    /* This delay is necessary. Otherwise, in EU mode, the LLC won't configure
     * properly on channel 172. Probably some two hardware operations are being
     * done in too quick a succession. Someone should probably eventually
     * figure out what these are. */
    usleep(10000);

    /* Get number of C-V2X and DSRC radios */
    risRet = wsuRisGetNumRadios(RT_CV2X, &g_numberCv2xRadios);

    if (risRet != RS_SUCCESS) {
        RIS_PRINTF("Unknown number of C-V2X radios (%d)\n", g_numberCv2xRadios);
        return RS_EPROCFS;
    }

    risRet = wsuRisGetNumRadios(RT_DSRC, &g_numberDsrcRadios);

    if (risRet != RS_SUCCESS) {
        RIS_PRINTF("Unknown number of DSRC radios (%d)\n", g_numberCv2xRadios);
        return RS_EPROCFS;
    }
#if defined(ENABLE_SR)
    /* Initialize SR connection */
    init_ris_with_sr();
#endif

    /* Create the wsuRisReceiveIndicationsThread thread */
    risRet = createReceiveIndicationsThread();

    if (risRet != RS_SUCCESS) {
        return risRet;
    }

    radioCommInitialized = TRUE;
    return RS_SUCCESS;
}

/***********************************************************************************
 * This function terminates Radio Communication for the calling application.
 * The application must call this API function as the final API function call before
 * exiting.
 *
 * Parameters:
 *   None
 *
 * Returns - Result code defined by rsResultCodeType
 **********************************************************************************/
rsResultCodeType wsuRisTerminateRadioComm(void)
{
    rsResultCodeType risRet;

    radioCommInitialized = FALSE;

    /* Terminate the receive indications thread. Don't bother checking the
     * return code, since it prints an error message if an error occurs, and we
     * can't do anything about it anyway. */
    RIS_DBG("Calling terminateReceiveIndicationsThread()\n");
    terminateReceiveIndicationsThread();
#if defined(ENABLE_SR)
    /* Terminate SR connection */
    RIS_DBG("Calling kill_ris_with_sr()\n");
    kill_ris_with_sr();
#endif
    /* Release Radio Protocol Stack (RPS) */
    RIS_DBG("Calling releaseRPS()\n");
    risRet = releaseRPS();

    if (risRet != RS_SUCCESS) {
        RIS_DBG("releaseRPS() returned %s\n", rsResultCodeType2Str(risRet));
    }

    /* Close the radio services socket and destroy the mutex */
    close(radioServicesSock);
    radioServicesSock = -1;
    pthread_mutex_destroy(&radioServicesMutex);
    radioServicesMutexInited = FALSE;
    return risRet;
}

/***********************************************************************************
 * This function registers the user service's request functions.  It is used with the
 * Full-Use version of the WAVE radio stack.  This function is used to allow an
 * application to register for services and access the SCH as a user.
 *
 * See IEEE 2016-1609.3 WME-UserService.request (Section 7.4.2.4)
 *
 * Parameters:
 *   service - definition of User service parameters for messages
 *
 * Returns - Result code defined by rsResultCodeType
 **********************************************************************************/
rsResultCodeType wsuRisUserServiceRequest(UserServiceType *service)
{
    rsResultCodeType risRet;
    int32_t          size;


    /* The fields "radioType" through "extendedAccess" should have been filled out
     * by the caller */

    /* Do some range checking */
    if ((service->radioType != RT_CV2X) && (service->radioType != RT_DSRC)) {
        RIS_PRINTF("Radio type must be RT_CV2X or RT_DSRC\n");
        return RS_ERANGE;
    }

    if (service->radioType == RT_CV2X) {
        if (service->radioNum > g_numberCv2xRadios - 1) {
            RIS_PRINTF("Radio number (%d) greater then MAX (%d)\n",
                   service->radioNum, g_numberCv2xRadios - 1);
            return RS_ERANGE;
        }
    }
    else {
        if (service->radioNum > g_numberDsrcRadios - 1) {
            RIS_PRINTF("Radio number (%d) greater then MAX (%d)\n",
                   service->radioNum, g_numberDsrcRadios - 1);
            return RS_ERANGE;
        }
    }

    if (service->psid > PSID_4BYTE_MAX_VALUE ||
        (service->psid < PSID_4BYTE_MIN_VALUE &&
         service->psid > PSID_3BYTE_MAX_VALUE) ||
        (service->psid < PSID_3BYTE_MIN_VALUE &&
         service->psid > PSID_2BYTE_MAX_VALUE) ||
        (service->psid < PSID_2BYTE_MIN_VALUE &&
         service->psid > PSID_1BYTE_MAX_VALUE)) {
        RIS_PRINTF("PSID (0x%x) is out of range.\n", service->psid);
        return RS_ERANGE;
    }

    if (service->lengthPsc > LENGTH_PSC) {
        RIS_PRINTF("PSC length (%d) greater than MAX (%d)\n",
               service->lengthPsc, LENGTH_PSC);
        return RS_ENOROOM;
    }

    if (service->lengthAdvertiseId > LENGTH_ADVERTISE) {
        RIS_PRINTF("ADVERTISE length (%d) greater than MAX (%d) \n",
               service->lengthAdvertiseId, LENGTH_ADVERTISE);
        return RS_ENOROOM;
    }

    /* Fill out the fields not filled in by the caller */
    service->cmd = WSU_NS_USER_SERVICE_REQ;
    service->pid = ris_pid;

    /* Send the message to Radio Service and wait for a reply */
    size = communicateWithRadioServices(service, sizeof(*service),
                                        &risRet, sizeof(risRet));

    if (size < 0) {
        risRet = RS_EIOCTL;
        RIS_PRINTF("Error risRet = RS_EIOCTL\n");
    }
    else if (risRet != RS_SUCCESS) {
        RIS_PRINTF("Error risRet = %s\n", rsResultCodeType2Str(risRet));
    }

    return risRet;
}

/***********************************************************************************
 * This function process provider service requests from application and sends to NS layer *
 * It is used with the Full-Use version of the WAVE radio stack.  This function is used
 * by an application to register for a provider service and transmit WSAs.
 *
 * See IEEE 2016-1609.3 WME-ProviderService.request (Section 7.4.2.2)
 *
 * Parameters:
 *   service - definition of provider service parameters for messages
 *
 * Returns - Result code defined by rsResultCodeType
 **********************************************************************************/
rsResultCodeType wsuRisProviderServiceRequest(ProviderServiceType *service)
{
    rsResultCodeType risRet;
    int32_t          size;

    /* Do some range checking */
    if ((service->radioType != RT_CV2X) && (service->radioType != RT_DSRC)) {
        RIS_PRINTF("Radio type must be RT_CV2X or RT_DSRC\n");
        return RS_ERANGE;
    }

    if (service->radioType == RT_CV2X) {
        if (service->radioNum > g_numberCv2xRadios - 1) {
            RIS_PRINTF("Radio number (%d) greater then MAX (%d)\n",
                   service->radioNum, g_numberCv2xRadios - 1);
            return RS_ERANGE;
        }
    }
    else {
        if (service->radioNum > g_numberDsrcRadios - 1) {
            RIS_PRINTF("Radio number (%d) greater then MAX (%d)\n",
                   service->radioNum, g_numberDsrcRadios - 1);
            return RS_ERANGE;
        }
    }

    if (service->psid > PSID_4BYTE_MAX_VALUE ||
        (service->psid < PSID_4BYTE_MIN_VALUE &&
         service->psid > PSID_3BYTE_MAX_VALUE) ||
        (service->psid < PSID_3BYTE_MIN_VALUE &&
         service->psid > PSID_2BYTE_MAX_VALUE) ||
        (service->psid < PSID_2BYTE_MIN_VALUE &&
         service->psid > PSID_1BYTE_MAX_VALUE)) {
        RIS_PRINTF("PSID (0x%x) is out of range.\n", service->psid);
        return RS_ERANGE;
    }

    if (service->lengthPsc > LENGTH_PSC) {
        RIS_PRINTF("PSC length (%d) greater than MAX (%d)\n",
               service->lengthPsc, LENGTH_PSC);
        return RS_ENOROOM;
    }

    /* Fill out the fields not filled in by the caller */
    service->cmd = WSU_NS_PROVIDER_SERVICE_REQ;
    service->pid = ris_pid;

    /* Send the message to Radio Service and wait for a reply */
    size = communicateWithRadioServices(service, sizeof(*service),
                                        &risRet, sizeof(risRet));

    if (size < 0) {
        risRet = RS_EIOCTL;
        RIS_PRINTF("Error risRet = RS_EIOCTL\n");
    }
    else if (risRet != RS_SUCCESS) {
        RIS_PRINTF("Error risRet = %s\n", rsResultCodeType2Str(risRet));
    }

    return risRet;
}

/***********************************************************************************
 * This function registers the WSM service request functions.  It is used with the
 * Full-Use version of the WAVE radio stack.  This function is used to allow an
 * application to receive WSMs.
 *
 * See IEEE 2016-1609.3 WME-WSMService.request (Section 7.4.2.6)
 *
 * Parameters:
 *   service - definition of WSM service parameters for messages
 *
 * Returns - Result code defined by rsResultCodeType
 **********************************************************************************/
rsResultCodeType wsuRisWsmServiceRequest(WsmServiceType *service)
{
    rsResultCodeType risRet;
    int32_t          size;

    /* The "action" and "psid" fields should have been filled out by the
     * caller */

    /* Fill out the fields not filled in by the caller */
    service->cmd = WSU_NS_WSM_SERVICE_REQ;
    service->pid = ris_pid;

    /* Send the message to Radio Service and wait for a reply */
    size = communicateWithRadioServices(service, sizeof(*service),
                                        &risRet, sizeof(risRet));

    if (size < 0) {
        risRet = RS_EIOCTL;
        RIS_PRINTF("Error risRet = RS_EIOCTL\n");
    }
    else if (risRet != RS_SUCCESS) {
        RIS_PRINTF("Error risRet = %s\n", rsResultCodeType2Str(risRet));
    }

    return risRet;
}

/***********************************************************************************
 * This function sends a WSM.
 *
 * See IEEE 2016-1609.3 WSM-WaveShortMessage.Request (Section 7.3.2)
 *
 * Parameters:
 *   WSM - WSM parameters defined by outWSMType
 *
 * Returns - Result code defined by rsResultCodeType
 **********************************************************************************/
rsResultCodeType wsuRisSendWSMReq(outWSMType *WSM)
{
    rsResultCodeType risRet;
    int32_t          size;
    int              i;
    uint32_t         wsmpExtensions;
    uint8_t          numberExtensions = 0;

    static const dataRateType dr[] = {
        DR_3_MBPS,  DR_4_5_MBPS, DR_6_MBPS,  DR_9_MBPS,
        DR_12_MBPS, DR_18_MBPS,  DR_24_MBPS, DR_27_MBPS
    };

    if ((WSM->radioType != RT_CV2X) && (WSM->radioType != RT_DSRC)) {
        RIS_PRINTF("radioType must be RT_CV2X or RT_DSRC (%d)\n", WSM->radioType);
        return RS_ERANGE;
    }

    /* Only do these checks for DSRC */
    if (WSM->radioType == RT_DSRC) {
        // check channel is in the range
        for (i = 0; i < sizeof(g_valid_ch_nums); i++) {
            if (g_valid_ch_nums[i] == WSM->channelNumber) {
                break;
            }
        }

        if (i == sizeof(g_valid_ch_nums)) {
            RIS_DBG("channelNumber(%d) out-of-range\n", WSM->channelNumber);
            return RS_ERANGE;
        }

        // check data rate is in the range
        for (i = 0; i < sizeof(dr) / sizeof(dataRateType); i++) {
            if (dr[i] == WSM->dataRate) {
                break;
            }
        }

        if (i == (sizeof(dr) / sizeof(dataRateType))) {
            RIS_PRINTF("DataRate %d is not a valid value\n", WSM->dataRate);
            return RS_ERANGE;
        }

        if (WSM->radioNum > g_numberDsrcRadios - 1) {
            RIS_PRINTF("DSRC radio number %u greater than MAX %d\n", WSM->radioNum, g_numberDsrcRadios - 1);
            return RS_ERANGE;
        }
    }
    else {
        if (WSM->radioNum > g_numberCv2xRadios - 1) {
            RIS_PRINTF("C-V2X radio number %u greater than MAX %d\n", WSM->radioNum, g_numberCv2xRadios - 1);
            return RS_ERANGE;
        }
    }

    if (WSM->txPriority > MAX_USER_PRIORITY) {
        RIS_PRINTF("User priority %d > MAX %d\n", WSM->txPriority, MAX_USER_PRIORITY);
        return RS_ERANGE;
    }

    // Validate the PSID so the lower layer's don't have to
    if (WSM->psid <= (uint32_t) PSID_1BYTE_MAX_VALUE) {
        /* OK */
    }
    else if (WSM->psid >= (uint32_t) PSID_2BYTE_MIN_VALUE &&
             WSM->psid <= (uint32_t) PSID_2BYTE_MAX_VALUE) {
        /* OK */
    }
    else if (WSM->psid >= (uint32_t) PSID_3BYTE_MIN_VALUE &&
             WSM->psid <= (uint32_t) PSID_3BYTE_MAX_VALUE) {
        /* OK */
    }
    else if (WSM->psid >= (uint32_t) PSID_4BYTE_MIN_VALUE &&
             WSM->psid <= (uint32_t) PSID_4BYTE_MAX_VALUE) {
        /* OK */
    }
    else {
        RIS_PRINTF("Encoding variable PSID=0x%x failed\n", WSM->psid);
        return RS_ERANGE;
    }

    // Validate the number of extension elements doesn't exceed the maximum
    wsmpExtensions = WSM->wsmpHeaderExt;

    while (wsmpExtensions) { /* count # of set bits in bitmask */
        if (wsmpExtensions & 0x01) {
            if (++numberExtensions > WSMP_NUM_EXT) {
                RIS_PRINTF("number of WSMP Extensions has exceed %d > MAX %d\n",
                           numberExtensions, WSMP_NUM_EXT);
                return RS_ERANGE;
            }
        }

        wsmpExtensions = wsmpExtensions >> 1;
    }

    // Validate the data length
    if (WSM->dataLength > MAX_WSM_DATA) {
        RIS_PRINTF("WSMV Datalen %d > MAX %d. Returning %d\n",
                   WSM->dataLength, MAX_WSM_DATA, RS_ERANGE);
        return RS_ERANGE;
    }

    /* Fill out the fields not filled in by the caller */
    WSM->cmd      = WSU_NS_SEND_WSM_REQ;
    WSM->pid      = ris_pid;
    WSM->version  = WSM_VERSION_3;

    /* Send the message to Radio Service and wait for a reply */
    size = communicateWithRadioServices(WSM,     sizeof(*WSM),
                                        &risRet, sizeof(risRet));

    if ((size < 0) && (risRet == RS_SUCCESS)) {
        risRet = RS_EIOCTL;
        RIS_PRINTF("Error psid=0x%X: risRet = %d.\n",WSM->psid,risRet);
    }
    if ((risRet != RS_SUCCESS) && (risRet != RS_ESECCERTCHGIP)) {
        RIS_PRINTF("Error psid=0x%X: risRet = %s\n", WSM->psid,rsResultCodeType2Str(risRet));
    }

    return risRet;
}

/**
** Function:  communicateWithRadioServices
** @brief  Send a message to Radio Services and get its reply.
** @param  msg       -- [input]Message to send to Radio Services
** @param  size      -- [input]Size of the message
** @param  reply     -- [output]Buffer to hold the received reply
** @param  replySize -- [input]Size of the reply buffer
** @return Size of the message received for success; -1 for error. Will never
**         return 0;
**
** Details: If it hasn't been done previously, creates the socket to
**          communicate with Radio Services. Sends a message to Radio Services
**          via wsuSendData() using port RS_RECV_PORT. Commands waiting for a
**          reply get the reply via wsuReceiveData().
**
**          Error messages are printed (maximum of 10 times) on errors of
**          socket creating, sending the message, or receiving the message.
**/
static int32_t communicateWithRadioServices(void *msg, size_t size, void *reply,
                                            size_t replySize)
{
    RcvDataType rcvData;
    size_t      rcvSize;

    RIS_DBG("size = %ld; replySize = %ld\n", size, replySize);

    /* Open the socket to communicate with radioServices with if it isn't
     * already opened */
    if (radioServicesSock < 0) {
        radioServicesSock = wsuConnectSocket(-1 /* port ignored */);

        if (radioServicesSock < 0) {
            /* Connect failed */
            RIS_PRINTF_MAX_N_TIMES(10, "wsuConnectSocket() failed\n");
            return -1;
        }
    }

    /* If the radio services mutex is not inited, init it now */
    if (!radioServicesMutexInited) {
        pthread_mutex_init(&radioServicesMutex, NULL);
        radioServicesMutexInited = TRUE;
    }

    pthread_mutex_lock(&radioServicesMutex);

    /* Send the message to Radio Services */
    if (!wsuSendData(radioServicesSock, RS_RECV_PORT, msg, size)) {
        /* Send failed */
        RIS_PRINTF_MAX_N_TIMES(10, "wsuSendData() failed\n");
        pthread_mutex_unlock(&radioServicesMutex);
        return -1;
    }

    /* Wait for a response */
    while (1) {
        /* Get the reply to the message */
        rcvData.available = FALSE;
        rcvData.fd        = radioServicesSock;
        rcvData.data      = reply;
        rcvData.size      = replySize;

        rcvSize = wsuReceiveData(/* Timeout ignored */0, &rcvData);

        if (rcvSize > 0) {
            /* We received data. In this case, there is no way
             * rcvData.available could have been set to FALSE. */
            break;
        }

        if (rcvSize < 0) {
            /* Receive failed */
            printf("wsuReceiveData() failed\n");
            fflush(stdout);
            pthread_mutex_unlock(&radioServicesMutex);
            return -1;
        }

        /* If rcvSize is 0, then continue in the loop and try again. A return
         * value of 0 is something that can occur normally with datagram
         * sockets; wsuConnectSocket() specifies SOCK_DGRAM on its call to
         * wsuSendData(). Also, a return value of 0 does NOT indicate the
         * socket was closed. wsuReceiveData() will also return 0 if errno is
         * set to EINTR upron return upon return from recvfrom(); in this case,
         * you should try again. */
    }

    pthread_mutex_unlock(&radioServicesMutex);
    return rcvSize;
}

/**
** Function:  initRPS
** @brief  Initialize Radio Protocol Stack (RPS).
** @param  serviceAvailableInd -- [input]TRUE if receiving service available
**                                indications (WSAs) desired
** @param  receiveWSMInd       -- [input]TRUE if receiving WSMs desired
** @return RS_SUCCESS for success, error code for failure.
**
** Details: Sends a WSU_NS_INIT_RPS message to radioServices to register an
**          application and indicate whether or not the receiving of service
**          available indications or WSMs is desired.
**/
static rsResultCodeType initRPS(bool_t serviceAvailableInd,
                                bool_t receiveWSMInd)
{
    rsRPSMsgType     initRps;
    rsResultCodeType risRet;
    int32_t          size;

    /* Initialize the message */
    initRps.cmd                 = WSU_NS_INIT_RPS;
    initRps.serviceAvailableInd = serviceAvailableInd;
    initRps.receiveWSMInd       = receiveWSMInd;
    initRps.pid                 = ris_pid;

    /* Send the message to Radio Service and wait for a reply */
    size = communicateWithRadioServices(&initRps, sizeof(initRps),
                                        &risRet, sizeof(risRet));

    if (size < 0) {
        risRet = RS_EIOCTL;
        RIS_PRINTF("Error risRet = RS_EIOCTL\n");
    }
    else if (risRet != RS_SUCCESS) {
        RIS_PRINTF("Error risRet = %s\n", rsResultCodeType2Str(risRet));
    }

    return risRet;
}

/**
** Function:  releaseRPS
** @brief  Release Radio Protocol Stack (RPS).
** @return RS_SUCCESS for success, error code for failure.
**
** Details: Sends a WSU_NS_RELEASE_RPS message to Radio Services and gets its
**          reply. Prints an error message if the reply is not RS_SUCCESS.
**/
static rsResultCodeType releaseRPS(void)
{
    rsRPSMsgType     releaseRps;
    rsResultCodeType risRet;
    int32_t          size;

    /* Initialize the message */
    releaseRps.cmd = WSU_NS_RELEASE_RPS;
    releaseRps.pid = ris_pid;

    /* Send the message to Radio Service and wait for a reply */
    size = communicateWithRadioServices(&releaseRps, sizeof(releaseRps),
                                        &risRet, sizeof(risRet));

    if (size < 0) {
        risRet = RS_EIOCTL;
        RIS_PRINTF("Error risRet = RS_EIOCTL\n");
    }
    else if (risRet != RS_SUCCESS) {
        RIS_PRINTF("Error risRet = %s\n", rsResultCodeType2Str(risRet));
    }

    return risRet;
}

#ifdef RIS_DEBUG
/**
** Function:  printEthHeader
** @brief  Print Ethernet header.
** @param  eth_hdr -- [input]The Ethernet header to print.
** @return None.
**
** Details: This function only exists if RIS_DEBUG is defined. Prints the
**          contents of the Ethernet header for debugging purposes.
**/
static void printEthHeader (struct ethhdr *eth_hdr)
{
    printf("Dest Add:%02X-%02X-%02X-%02X-%02X-%02X, Source Add:%02X-%02X-%02X-%02X-%02X-%02X, Proto:0x%0X\n",
           eth_hdr->h_dest[0], eth_hdr->h_dest[1],
           eth_hdr->h_dest[2], eth_hdr->h_dest[3],
           eth_hdr->h_dest[4], eth_hdr->h_dest[5],
           eth_hdr->h_source[0], eth_hdr->h_source[1],
           eth_hdr->h_source[2], eth_hdr->h_source[3],
           eth_hdr->h_source[4], eth_hdr->h_source[5],
           ntohs(eth_hdr->h_proto));
    fflush(stdout);
}

/**
** Function:  PrintBytes
** @brief  Print a string of bytes.
** @param  array          -- [input]The bytes to print.
** @param  len            -- [input]The number of bytes to print.
** @param  carriageReturn -- [input]Puts a carriage return at the end of the
**                                  line printed if carriageReturn > 0.
**                                  Otherwise, it doesn't.
** @return 0 for success.
**
** Details: This function only exists if RIS_DEBUG is defined. Prints the
**          contents of the array of bytes for debugging purposes.
**/
int PrintBytes(unsigned char array[], int len, int carriageReturn)
{
    char spacer[4];
    int  i;

    sprintf(spacer, "%s", (len < 31) ? " " : "  ");
    printf(" [");

    for (i = 0; i < len; i++) {
        if (((i + 0) % 4) != 0) {
            printf("-");
        }

        printf("%02x", array[i]);

        if ((((i + 1) % 32) == 0) && (i < (len - 1))) {
            printf("\n");
        }

        if ((((i + 1) %  4) == 0) && (i < (len - 1))) {
            printf("%s",spacer);
        }
    }

    printf("]");

    if (carriageReturn > 0) {
        printf("\n");
    }

    fflush(stdout);
    return 0;
}
#endif

/**
** Function:  processWSM
** @brief  Process an incoming WSM.
** @param  wsm -- [input]The WSM to process.
** @return None.
**
** Details: Calls the "receive WSM" callback function. Also sends a record to
**          SR for recording in the SR file if SE is in record mode.
**/
static void processWSM(inWSMType *wsm)
{
    ++debug_cnt_process_wsm;
    RIS_DBG("wsm=\n");
#ifdef RIS_DEBUG
    PrintBytes((void *)wsm, sizeof(inWSMType), 1);
#endif
    RIS_DBG("PSID:%8x V:%d S:%d C:%d R:%d P:%d D:%s(%d)\n", wsm->psid,
        wsm->wsmVersion, wsm->security, wsm->channelNumber,
        wsm->dataRate, wsm->txPwrLevel, wsm->data, wsm->dataLength);

    /* Call the callback function */
    if (appCredentialsRisInternal.cbReceiveWSM != NULL) {
        appCredentialsRisInternal.cbReceiveWSM(wsm);

        /* Send this to SR. Note that this routine does endian swapping and
         * compression in place. So don't call this routine until everyone else
         * using the WSM data is done with it. */
#if defined(ENABLE_SR)
        risSendSRMessage(RIS_WSM_DATA, (uint8_t *)wsm, sizeof(*wsm));
#endif
    }
}

/**
** Function:  processServiceAvailable
** @brief  Process an incoming Service Available indication.
** @param  wsm -- [input]The Service Available Indication to process.
** @return None.
**
** Details: Calls the "service available" callback function. Also sends a record to
**          SR for recording in the SR file if SE is in record mode.
**/
static void processServiceAvailable(serviceInfoType *ind)
{
    if (appCredentialsRisInternal.cbServiceAvailable != NULL) {
        RIS_DBG("Calling appCredentialsRisInternal.cbServiceAvailable(ind)\n");
        appCredentialsRisInternal.cbServiceAvailable(ind);
#if defined(ENABLE_SR)
        /* Send this to SR */
        RIS_DBG("Calling risSendSRMessage(RIS_SERVICE_AVAILABLE, (uint8_t *)ind, sizeof(*ind))\n");
        risSendSRMessage(RIS_SERVICE_AVAILABLE, (uint8_t *)ind, sizeof(*ind));
#endif
    }
}

/**
** Function:  wsuRisSIGUSR1SignalHandler
** @brief  Signal handler for SIGUSR1, used to shut down threads.
** @param  signum -- [input]Signal received. Should be SIGUSR1.
** @return None.
**
** Details: Just prints a message saying what signal was received. The main
**          purpose of having this signal handler is to prevent the default
**          signal handler from running, which may have undesirable side
**          effects such as propogating the signal to its parent.
**/
static void wsuRisSIGUSR1SignalHandler(int signum)
{
    RIS_DBG("signum = %d\n", signum);
}

/**
** Function:  wsuRisReceiveIndicationsThread
** @brief  Thread to receive incoming indications from Radio Services.
** @param  arg -- [input]Required by convention, but not used.
** @return NULL
**
** Details: Sets up a socket server on the port number received in response to
**          the WSU_NS_GET_IND_PORT command to receive incoming indications
**          from radioServices. Then loops as long as
**          wsuRisReceiveIndicationsThreadRunning is TRUE receiving indications
**          via wsuReceiveData(). No reply is sent back.
**
**          For each indication, if either SR is not in playback mode or SR is
**          is RIS live mode, the appropriate routine is called to process the
**          indication. The indication type is checked to see which routine to
**          call.
**
**          The thread exits immediately if radio communication has not been
**          initialized (i.e. the terminate function has been called).
**
**          Console messages are printed on error creating socket, receive data
**          failure, or unknown indication type.
**/
static void *wsuRisReceiveIndicationsThread(void *arg)
{
    rsGetIndPortMsgType    getIndPortMsg;
    rsGetIndPortReplyType  getIndPortReply;
    RcvDataType            rcvData;
    size_t                 rcvSize;
    rsReceiveDataType      recv;

    RIS_DBG("Started\n");

    wsuRisReceiveIndicationsThreadRunning = TRUE;
    /* Unblock the SIGUSR1 signal and disable the default signal handler */
    signal(SIGUSR1, wsuRisSIGUSR1SignalHandler);

    /* If terminate function has been called, then we don't want to
       poll any sockets. */
    if (!radioCommInitialized) {
        usleep(20000);
        goto wsuRisReceiveIndicationsThreadExit;
    }

    /* Initialize the "get receive indications port" message */
    getIndPortMsg.cmd = WSU_NS_GET_IND_PORT;
    getIndPortMsg.pid = ris_pid;

    /* Send the message to Radio Service and wait for a reply */
    rcvSize = communicateWithRadioServices(&getIndPortMsg, sizeof(getIndPortMsg),
                                       &getIndPortReply, sizeof(getIndPortReply));

    if (rcvSize < 0) {
        RIS_PRINTF("Error sending WSU_NS_GET_IND_PORT to radioServices\n");
        goto wsuRisReceiveIndicationsThreadExit;
    }
    else if (getIndPortReply.risRet != RS_SUCCESS) {
        RIS_PRINTF("Error getIndPortReply.risRet = %s\n",
                   rsResultCodeType2Str(getIndPortReply.risRet));
        goto wsuRisReceiveIndicationsThreadExit;
    }

    /* Create and bind the socket */
    RIS_DBG("Creating socket server on port %u\n", getIndPortReply.port);
    recvIndSock = wsuCreateSockServer(getIndPortReply.port);

    if (recvIndSock < 0) {
        RIS_PRINTF("wsuCreateSockServer() failed\n");
        goto wsuRisReceiveIndicationsThreadExit;
    }

    /* Main receive loop */
    while (wsuRisReceiveIndicationsThreadRunning) {
        /* Wait for an indication */
        rcvData.available = FALSE;
        rcvData.fd        = recvIndSock;
        rcvData.data      = &recv;
        rcvData.size      = sizeof(recv);

        RIS_DBG("Calling wsuReceiveData(0, &rcvData)\n");
        rcvSize = wsuReceiveData(/* Timeout ignored */0, &rcvData);
        RIS_DBG("wsuReceiveData() returned\n");

        if (!wsuRisReceiveIndicationsThreadRunning) {
            RIS_DBG("(!wsuRisReceiveIndicationsThreadRunning)\n");
            break;
        }

        if (rcvSize < 0) {
            RIS_PRINTF("wsuReceiveData() error (rcvSize < 0)\n");
            /* Close and recreate the socket */
            close(recvIndSock);
            recvIndSock = wsuCreateSockServer(getIndPortReply.port);

            if (recvIndSock < 0) {
                RIS_PRINTF("wsuCreateSockServer() failed\n");
                goto wsuRisReceiveIndicationsThreadExit;
            }

            continue;
        }

        if (rcvSize == 0) {
            /* If rcvSize is 0, then continue in the loop and try again. A
             * return value of 0 is something that can occur normally with
             * datagram sockets; wsuConnectSocket() specifies SOCK_DGRAM on its
             * call to wsuSendData(). Also, a return value of 0 does NOT
             * indicate thesocket was closed. wsuReceiveData() will also return
             * 0 if errnois set to EINTR upon return upon return from
             * recvfrom(); in this case, you should try again. */
            continue;
        }
        
        /* rcvSize is > 0. In this case, there is no way for rcvData.available
         * to have been set to FALSE. */
#if defined(ENABLE_SR)
        /* NOTE:  This whole function is for receiving live radio data.
         * No playback messages get here. So, if SR is in playback
         * mode, then any wsm does not get sent to the application via
         * processWSM() unless SRRisLive is set to TRUE. */
        if (!SR_in_playback_mode || SRRisLive)
#endif 
        {
            switch (recv.indType) {
            case IND_WSM:
                RIS_DBG("IND_WSM\n");
#ifdef RIS_DEBUG
                printEthHeader(&recv.eth_hdr);
#endif
                processWSM(&recv.u.wsm);
                break;

            case IND_SERVICE_AVAILABLE:
                RIS_DBG("IND_SERVICE_AVAILABLE\n");
#ifdef RIS_DEBUG
                printEthHeader(&recv.eth_hdr);
#endif
                processServiceAvailable(&recv.u.serviceInfo);
                break;

            case IND_SIGN_WSM_RESULTS:
                RIS_DBG("IND_SIGN_WSM_RESULTS\n");
                processSmiSignResults(&recv);
                break;

            case IND_SIGN_WSA_RESULTS:
                RIS_DBG("IND_SIGN_WSA_RESULTS\n");
                processSmiSignResults(&recv);
                break;

            case IND_ADD_UNSEC_1609P2_RESULTS:
                RIS_DBG("IND_ADD_UNSEC_1609P2_RESULTS\n");
                processSmiSignResults(&recv);
                break;

            case IND_VERIFY_WSA_RESULTS:
                RIS_DBG("IND_VERIFY_WSA_RESULTS\n");
                processSmiVerifyWSAResults(&recv);
                break;

            case IND_CERT_CHANGE_RESULTS:
                RIS_DBG("IND_CERT_CHANGE_RESULTS\n");
                processCertChangeResults(&recv);
                break;

            default:
                RIS_PRINTF_MAX_N_TIMES(10, "Unknown rsIndicationType (%d)\n", recv.indType);
                break;
            } // switch (recv.indType)
        } // if ((!SR_in_playback_mode || SRRisLive)  && (rcvSize != 0))
    } // while (wsuRisReceiveIndicationsThreadRunning)

wsuRisReceiveIndicationsThreadExit:
    RIS_DBG("Exiting\n");
    wsuRisReceiveIndicationsThreadRunning = FALSE;

    if (recvIndSock != -1) {
        close(recvIndSock);
    }

    pthread_exit(NULL);
    return NULL;
}

/**
** Function:  createReceiveIndicationsThread
** @brief  Creates the thread to receive incoming indications from
**         radioServices.
** @return RS_SUCCESS for success. Error code for failure.
**
** Details: The thread to receive incoming indications from Radio Services is
**          spawned. A console message is printed on an error spawning the
**          thread.
**/
static rsResultCodeType createReceiveIndicationsThread(void)
{
    /* Make sure that multiple threads are not created - We need only one */
    if (!wsuRisReceiveIndicationsThreadRunning) {
        RIS_DBG("Calling pthread_create(&wsuRisReceiveIndicationsThreadId, NULL, &wsuRisReceiveIndicationsThread, NULL)\n");

        if (pthread_create(&wsuRisReceiveIndicationsThreadId, NULL,
                           &wsuRisReceiveIndicationsThread, NULL) != 0) {
            RIS_DBG("wsuRisReceiveIndicationsThread creation failed\n");
            return RS_ETHREAD;
        }

        RIS_DBG("wsuRisReceiveIndicationsThread creation succeeded\n");
    }

    return RS_SUCCESS;
}

/**
** Function:  terminateReceiveIndicationsThread
** @brief  Terminates the thread to receive incoming indications from
**         radioServices.
** @return RS_SUCCESS for success. Error code for failure.
**
** Details: The "running" variable is set to FALSE, and the SIGTERM signal is
**          sent to the thread. Then a pthread_join() is done to wait for the
**          thread to terminate.
**/
static rsResultCodeType terminateReceiveIndicationsThread(void)
{
    int result;
    int tmpSock;

    /* Be sure the thread is running before you try to terminate it */
    if (wsuRisReceiveIndicationsThreadRunning) {
        wsuRisReceiveIndicationsThreadRunning = FALSE;

        /* Close the socket that wsuRisReceiveIndicationsThread is reading
         * from. This will force the recvfrom() to abort with an error. The
         * thread, seeing that wsuRisReceiveIndicationsThreadRunning is now
         * FALSE, will terminate execution. */
        if (recvIndSock != -1) {
            tmpSock = recvIndSock;
            recvIndSock = -1;
            close(tmpSock);
        }

        RIS_DBG("Calling pthread_kill(wsuRisReceiveIndicationsThreadId, SIGUSR1)\n");
        result = pthread_kill(wsuRisReceiveIndicationsThreadId, SIGUSR1);

        if (result != 0) {
            RIS_PRINTF("Error terminating wsuRisReceiveIndicationsThread: %s\n",
                       strerror(result));
        }
        else {
            RIS_DBG("Calling pthread_join(wsuRisReceiveIndicationsThreadId, NULL)\n");
            result = pthread_join(wsuRisReceiveIndicationsThreadId, NULL);

            if (result != 0) {
                RIS_PRINTF("pthread_join() failed: %s\n", strerror(result));
            }
#ifdef RIS_DEBUG
            else {
                RIS_DBG("pthread_join() succeeded\n");
            }
#endif
        }
    }

    return RS_SUCCESS;
}

/***********************************************************************************
 * This function gets the value of various Radio Services configuration items,
 * indicated by the identifier, cfgId
 *
 * See IEEE 2016-1609.3 WME-Get.request (Section 7.4.4.1)
 *
 * Parameters:
 *   cfgId - Configuration item identifier defined by cfgIdType
 *   cfg   - Configuration item value defined by cfgType. If appropriate,
 *           cfg->radioType must be set to indicate the radio type (RT_CV2X or
 *           RT_DSRC).
 *
 * Returns - Result code defined by rsResultCodeType
 **********************************************************************************/
rsResultCodeType wsuRisGetCfgReq(cfgIdType cfgId, cfgType *cfg)
{
    int32_t size;

    /* Fill out the fields not filled in by the caller */
    cfg->cmd   = WSU_NS_GET_NS_CFG;
    cfg->cfgId = cfgId;

    /* Send the message to Radio Service and wait for a reply */
    size = communicateWithRadioServices(cfg, sizeof(*cfg), cfg, sizeof(*cfg));

    if (size < 0) {
        cfg->risRet = RS_EIOCTL;
        RIS_PRINTF("Error risRet = RS_EIOCTL\n");
    }
    else if (cfg->risRet != RS_SUCCESS) {
        RIS_PRINTF("Error risRet = %s\n", rsResultCodeType2Str(cfg->risRet));
    }

    return cfg->risRet;
}

/***********************************************************************************
 * This function sets the value of various Radio Services configuration items,
 * indicated by the identifier, cfgId
 *
 * See IEEE 2016-1609.3 WME-Set.request (Section 7.4.4.3)
 *
 * Parameters:
 *   cfgId - Configuration item identifier defined by cfgIdType
 *   cfg   - Configuration item value defined by cfgType
 *
 * Returns - Result code defined by rsResultCodeType
 **********************************************************************************/
rsResultCodeType wsuRisSetCfgReq(cfgIdType cfgId, cfgType *cfg)
{
    int32_t          size;
    rsResultCodeType risRet;
    char             cmd[64];
    int              pos;
    int              i;

    /* Fill out the fields not filled in by the caller */
    cfg->cmd   = WSU_NS_SET_NS_CFG;
    cfg->cfgId = cfgId;

    /* Send the message to Radio Service and wait for a reply */
    size = communicateWithRadioServices(cfg, sizeof(*cfg), &risRet,
                                        sizeof(risRet));

    if (size < 0) {
        risRet = RS_EIOCTL;
        RIS_PRINTF("Error risRet = RS_EIOCTL\n");
    }
    else if (risRet != RS_SUCCESS) {
        RIS_PRINTF("Error risRet = %s\n", rsResultCodeType2Str(risRet));
    }
    else if ((cfgId == CFG_ID_MACADDRESS_RADIO_0) || 
             (cfgId == CFG_ID_MACADDRESS_RADIO_1)) {
        /* For CFG_ID_MACADDRESS_RADIO_0 and CFG_ID_MACADDRESS_RADIO_1, set the
         * corresponding IPv6 device to have the same MAC address */
        sprintf(cmd, "ifconfig llc-%cch-ipv6 down",
                (cfgId == CFG_ID_MACADDRESS_RADIO_0) ? 'c' : 's');
        system(cmd);

        pos = 0;
        pos = sprintf(cmd, "ifconfig llc-%cch-ipv6 hw ether ",
                      (cfgId == CFG_ID_MACADDRESS_RADIO_0) ? 'c' : 's');

        for (i = 0; i < LENGTH_MAC; i++) {
            pos += sprintf(&cmd[pos], "%02x", cfg->u.RadioMacAddress[i]);

            if (i < (LENGTH_MAC - 1)) {
                cmd[pos++] = ':';
            }
        }

        system(cmd);

        sprintf(cmd, "ifconfig llc-%cch-ipv6 up",
                (cfgId == CFG_ID_MACADDRESS_RADIO_0) ? 'c' : 's');
        system(cmd);
    }

    return risRet;
}

/***********************************************************************************
 * This function changes the parameters of an existing user service request that has
 * already been registered.  It is used with the Full-Use version of the WAVE radio
 * stack.  This function is intended to be used when accepting or ending a service
 * that was advertised in a WSA, allowing access to the advertised service channel
 * and other designated parameters.
 *
 * Parameters:
 *   psid - psid Service
 *   UserServiceChangeType - currently only support service access.
 * Returns - Result code defined by rsResultCodeType
 **********************************************************************************/
rsResultCodeType wsuRisChangeUserServiceRequest(UserServiceChangeType *change)
{
    int32_t          size;
    rsResultCodeType risRet;

    if (change->command != CHANGE_ACCESS) {
        RIS_PRINTF("Invalid command (%d) \n", change->command);
        return RS_ERANGE;
    }

    change->cmd = WSU_NS_USER_CHANGE_SERVICE_REQ;
    change->pid = ris_pid;

    /* Send the message to Radio Service and wait for a reply */
    size = communicateWithRadioServices(change, sizeof(*change), &risRet,
                                        sizeof(risRet));

    if (size < 0) {
        risRet = RS_EIOCTL;
        RIS_PRINTF("Error risRet = RS_EIOCTL\n");
    }
    else if (risRet != RS_SUCCESS) {
        RIS_PRINTF("Error risRet = %s\n", rsResultCodeType2Str(risRet));
    }

    return risRet;
}

/***********************************************************************************
 * This function inquires the tallies of specified radio
 *
 * Parameters: radioType: RT_CV2X or RT_DSRC
 *             radioNum: Radio index number (0-1)
 *             *tallies: returns radio statistics
 *
 * Returns - Result code defined by rsResultCodeType
 **********************************************************************************/
rsResultCodeType wsuRisGetRadioTallies(rsRadioType radioType, uint8_t radioNum,
                                       DeviceTallyType *tallies)
{
    rsGetRadioTalliesCmdType getRadioTallies;
    int32_t                  size;

    /* Initialize the message */
    getRadioTallies.cmd       = WSU_NS_GET_RADIO_TALLIES;
    getRadioTallies.radioType = radioType;
    getRadioTallies.radioNum  = radioNum;

    /* Send the message to Radio Service and wait for a reply */
    size = communicateWithRadioServices(&getRadioTallies,
                                        sizeof(getRadioTallies),
                                        tallies, sizeof(*tallies));

    if (size < 0) {
        tallies->risRet = RS_EIOCTL;
        RIS_PRINTF("Error tallies->risRet = RS_EIOCTL\n");
    }
    else if (tallies->risRet != RS_SUCCESS) {
        RIS_PRINTF("Error tallies->risRet = %s\n",
                   rsResultCodeType2Str(tallies->risRet));
    }

    return tallies->risRet;
}

/***********************************************************************************
 * This function sets the value of one or more of various Radio Services configuration
 * items in as single function call.
 *
 * Parameters:
 *   rsRadioCfg - structure containing multipel radio configuration items.
 *                (Any item with the valid flag set, will be modified by the
 *                Radio Stack.)
 *
 * Returns - Result code defined by rsResultCodeType
 **********************************************************************************/
rsResultCodeType wsuRisRadioCfgReq(rsRadioCfgType *radioCfg)
{
    rsResultCodeType risRet;
    int32_t          size;

    /* Initialize the message */
    radioCfg->cmd = WSU_NS_SET_RADIO_CFG;
    /* Send the message to Radio Service and wait for a reply */
    size = communicateWithRadioServices(radioCfg, sizeof(*radioCfg),
                                        &risRet, sizeof(risRet));

    if (size < 0) {
        risRet = RS_EIOCTL;
        RIS_PRINTF("Error risRet = RS_EIOCTL\n");
    }
    else if (risRet != RS_SUCCESS) {
        RIS_PRINTF("Error risRet = %s\n", rsResultCodeType2Str(risRet));
    }

    return risRet;
}

/***********************************************************************************
 * Stubbed-out functions
 **********************************************************************************/

/***********************************************************************************
 * This function returns the status of the Tx queue of the specified radio.
 * True is returned if any of the 4 Tx Queues is above the IsFull threshhold.
 *
 * Parameters: radioType: RT_CV2X or RT_DSRC
 *             radioNum: Radio index number (0-1)
 *
 * Returns - WTRUE, if Tx queue is full. WFALSE, if Tx queue is empty.
 **********************************************************************************/
bool_t wsuRisIsRadioTxQFull(rsRadioType radioType, uint8_t radioNum)
{
    return FALSE;
}

/***********************************************************************************
 * This function returns the status of the specified Tx queue of the specified radio.
 * CCH/SCH channel type and priority queue.
 *
 * Parameters: radioType: RT_CV2X or RT_DSRC
 *             radioNum: Radio index number (0-1)
 *             *CHANType channelType: 0: CHANTYPE_CCH, 1: CHANTYPE_SCH, 2:CHANTYPE_ALL
 *             *txPriority 0-7 (TxPriority is the same as WsmOut struct and 0xff=ALL)
 *
 * Returns - Result code defined by rsResultCodeType
 **********************************************************************************/
bool_t wsuRisIsRadioTxQueueFull(rsRadioType radioType, uint8_t radioNum, CHANType channelType, uint8_t txPriority)
{
    return FALSE;
}

/***********************************************************************************
 * This function flushes the transmission queue by specified radio interface and
 * CCH/SCH channel type and priority queue.
 *
 * Parameters: radioType: RT_CV2X or RT_DSRC
 *             radioNum: Radio index number (0-1)
 *             *CHANType channelType: 0: CHANTYPE_CCH, 1: CHANTYPE_SCH, 2:CHANTYPE_ALL
 *             *txPriority 0-7 (TxPriority is the same as WsmOut struct and 0xff=ALL)
 *
 * Returns - Result code defined by rsResultCodeType
 **********************************************************************************/
rsResultCodeType wsuRisFlushTxQueue(rsRadioType radioType, uint8_t radioNum, CHANType channelType, uint8_t txPriority)
{
    return RS_SUCCESS;
}

/***********************************************************************************
 * This function registers the EU service request functions.  It is used with the
 * Full-Use version of the WAVE radio stack for Registering for receiving non-WSM
 * (Currently EU) packets.  This function is used to allow an application to
 * supported non- ETH_P_WSMP packets based on EtherType.    This function cannot
 * be used to receive ETH_P_WSMP packets.
 *
 * Parameters:
 *   service - definition of EU service parameters for V2 messages
 *
 * Returns - Result code defined by rsResultCodeType
 **********************************************************************************/
rsResultCodeType wsuRisEUServiceRequest(EUServiceType *service)
{
    return RS_SUCCESS;
}

/***********************************************************************************
 * This function requests a previously received signed WSA be verified.
 *
 * If the verification of the signed WSA is successful, the application's
 * service available callback will be called, and the serviceBitmask field in
 * serviceInfoType structure sent to the callback will have both the
 * WSA_SIGNED and WSA_VERIFIED bits set.
 *
 * Parameters:
 *   signedWsaHandle - The handle to the signed WSA that was received in the
 *                     serviceInfoType structure sent to the service available
 *                     callback in the signedWsaHandle field.
 * Returns - Result code defined by rsResultCodeType
 **********************************************************************************/
rsResultCodeType wsuRisVerifyWsaRequest(uint32_t signedWsaHandle)
{
    rsVodMsgType   vodMsg;
    rsVodReplyType vodReply;
    int32_t        size;

    vodMsg.cmd          = WSU_NS_SECURITY_VERIFY_WSA_REQ;
    vodMsg.vodMsgSeqNum = signedWsaHandle;
    /* Send the message to Radio Service and wait for a reply */
    size = communicateWithRadioServices(&vodMsg, sizeof(vodMsg),
                                        &vodReply, sizeof(vodReply));

    if (size < 0) {
        vodReply.risRet = RS_EIOCTL;
        RIS_PRINTF("Error risRet = RS_EIOCTL\n");
    }
    else if (vodReply.risRet != RS_SUCCESS) {
        RIS_PRINTF("Error risRet = %s\n", rsResultCodeType2Str(vodReply.risRet));
        RIS_PRINTF("Error vodResult = %u\n", vodReply.vodResult);
    }

    return vodReply.risRet;
}
/***********************************************************************************
 * This function processes delete all services requests from the application
 * and sends them to NS layer. This function is used to delete all services for
 * a given radio number, either for all PID's or only for the calling
 * application's PID.
 *
 * Parameters:
 *   service - definition of delete all services parameters for messages
 *
 * Returns - Result code defined by rsResultCodeType
 **********************************************************************************/
rsResultCodeType wsuRisDeleteAllServicesRequest(DeleteAllServicesType *service)
{
    return RS_SUCCESS;
}

/***********************************************************************************
 * This function gets the MAC address for the specified radio number.
 *
 * This function may be deprecated in the future.  It is recommended to use
 * the wsuRisGetCfg function instead.
 *
 * Parameters: radioType: RT_CV2X or RT_DSRC
 *             radioNum: Radio index number (0-1)
 *             mac: Where to store the MAC address
 *
 **********************************************************************************/
rsResultCodeType wsuRisGetRadioMacaddress(rsRadioType radioType, uint8_t radioNum, uint8_t *mac)
{
    return RS_SUCCESS;
}

/***********************************************************************************
 * This function sends an EU Packet, to be used with the WAVE Radio Stack, when
 * it is operating in EU mode.
 *
 * Parameters:
 *   EUPkt     - EU Packet defined by risEUPktType
 *   EUPktMeta - EU Packet metadata defined by risTxMetaEUPktType
 *
 * Returns - Result code defined by rsResultCodeType
 **********************************************************************************/
rsResultCodeType wsuRisSendEUPacketReq(risEUPktType *EUPkt, risTxMetaEUPktType *EUPktMeta)
{
    return RS_SUCCESS;
}

/***********************************************************************************
 * This function receives an EU Packet, to be used with the WAVE Radio Stack when
 * it is operating in EU mode.
 *
 * IF the WAVE Radio Stack queue of recevied EU packets is empty, this function
 * will block until a packet is received, which will then be returned.  Otherwise,
 * this function will return the next EU packet in the WAVE Radio Stack queue of
 * received EU packets.
 *
 * Parameters:
 *   EUPkt     - EU Packet defined by risEUPktType
 *   EUPktMeta - EU Packet metadata defined by risRxMetaEUPktType
 *
 * Returns - Result code defined by rsResultCodeType
 **********************************************************************************/
rsResultCodeType wsuRisReceiveEUPacketReq(risEUPktType *EUPkt, risRxMetaEUPktType *EUPktMeta)
{
    return RS_SUCCESS;
}

/***********************************************************************************
 * This function sends a raw packet, to be used with the WAVE Radio Stack.  The
 * exact data provided to this function is transmitted * over-the-air, nothing
 * more and nothing less.
 *
 * Parameters:
 *   raw - Raw packet parameters defined by outRawType
 *
 * Returns - Result code defined by rsResultCodeType
 **********************************************************************************/
rsResultCodeType wsuRisSendRawPacketReq(outRawType *raw)
{
    rsResultCodeType risRet;
    int32_t          size;

    raw->cmd = WSU_NS_SEND_RAW_PACKET_REQ;
    /* Send the message to Radio Service and wait for a reply */
    size = communicateWithRadioServices(raw,     sizeof(*raw),
                                        &risRet, sizeof(risRet));

    if (size < 0) {
        risRet = RS_EIOCTL;
        RIS_PRINTF("Error risRet = RS_EIOCTL\n");
    }
    else if (risRet != RS_SUCCESS) {
        RIS_PRINTF("Error risRet = %s\n", rsResultCodeType2Str(risRet));
    }

    return risRet;
}

/***********************************************************************************
 * This function enables PCAP capture for a particualr radio.
 *
 * Parameters: pcapEnable: Parameters for the PCAP enable command
 *
 * Returns - Result code defined by rsResultCodeType
 **********************************************************************************/
rsResultCodeType wsuRisPcapEnable(rsPcapEnableType *pcapEnable)
{
    int32_t          size;
    rsResultCodeType risRet;

    RIS_DBG("radioType = %s; radioNum = %u; mask = 0x%x\n",
            (pcapEnable->radioType == RT_CV2X) ? "RT_CV2X" : "RT_DSRC",
            pcapEnable->radioNum, pcapEnable->mask);

    pcapEnable->cmd = WSU_NS_PCAP_ENABLE;
    /* Send the message to Radio Service and wait for a reply */
    size = communicateWithRadioServices(pcapEnable, sizeof(*pcapEnable),
                                        &risRet, sizeof(risRet));

    if (size < 0) {
        risRet = RS_EIOCTL;
        RIS_PRINTF("Error risRet = RS_EIOCTL\n");
    }
    else if (risRet != RS_SUCCESS) {
        RIS_PRINTF("Error risRet = %s\n", rsResultCodeType2Str(risRet));
    }

    return risRet;
}

/***********************************************************************************
 * This function disables PCAP capture for a particualr radio.
 *
 * Parameters: pcapDisable: Parameters for the PCAP disable command
 *
 * Returns - Result code defined by rsResultCodeType
 **********************************************************************************/
rsResultCodeType wsuRisPcapDisable(rsPcapDisableType *pcapDisable)
{
    int32_t          size;
    rsResultCodeType risRet;

    RIS_DBG("radioType = %s; radioNum = %u\n",
            (pcapDisable->radioType == RT_CV2X) ? "RT_CV2X" : "RT_DSRC",
            pcapDisable->radioNum);

    pcapDisable->cmd = WSU_NS_PCAP_DISABLE;
    /* Send the message to Radio Service and wait for a reply */
    size = communicateWithRadioServices(pcapDisable, sizeof(*pcapDisable),
                                        &risRet, sizeof(risRet));

    if (size < 0) {
        risRet = RS_EIOCTL;
        RIS_PRINTF("Error risRet = RS_EIOCTL\n");
    }
    else if (risRet != RS_SUCCESS) {
        RIS_PRINTF("Error risRet = %s\n", rsResultCodeType2Str(risRet));
    }

    return risRet;
}

/***********************************************************************************
 * This function reads bytes from the PCAP buffer.
 *
 * Parameters: pcapRead: Parameters for the PCAP read command
 *             pcapBuffer: The buffer to hold the PCAP data bytes. Should be
 *                         PCAP_TX_LOG_BUF_SIZE (for tx only) or PCAP_LOG_BUF_SIZE
 *                         (in all other cases) in length.
 *             pcapBufferLen: The length of pcapBuffer. See above description for
 *                            what the size should be. Returns RS_ERANGE if the
 *                            size is less than minimum.
 *             totalLength: The acutal number of bytes read is written to the
 *                          int pointed to by totalLength.
 *
 * Returns - Result code defined by rsResultCodeType
 **********************************************************************************/
rsResultCodeType wsuRisPcapRead(rsPcapReadType *pcapRead, uint8_t *pcapBuffer,
                                int pcapBufferLen, int *totalLength)
{
    rsPcapReadReplyType pcapReadReply;
    int                 offset;
    int                 remainingLength;
    int                 size;

    if (pcapRead->tx) {
        if (pcapBufferLen < PCAP_TX_LOG_BUF_SIZE) {
            return RS_ERANGE;
        }
    }
    else {
        if (pcapBufferLen < PCAP_LOG_BUF_SIZE) {
            return RS_ERANGE;
        }
    }

    pcapRead->cmd   = WSU_NS_PCAP_READ;
    pcapBufferLen   = (pcapRead->tx) ? PCAP_TX_LOG_BUF_SIZE : PCAP_LOG_BUF_SIZE;
    remainingLength = pcapBufferLen;
    offset          = 0;
    *totalLength    = 0;

    do {
        /* Send the message to Radio Service and wait for a reply */
        size = communicateWithRadioServices(pcapRead, sizeof(*pcapRead),
                                            &pcapReadReply,
                                            sizeof(pcapReadReply));

        if (size < 0) {
            pcapReadReply.risRet = RS_EIOCTL;
            RIS_PRINTF("Error pcapReadReply.risRet = RS_EIOCTL\n");
            return pcapReadReply.risRet;
        }

        if (pcapReadReply.risRet != RS_SUCCESS) {
            RIS_PRINTF("Error pcapReadReply.risRet = %s\n",
                       rsResultCodeType2Str(pcapReadReply.risRet));
            return pcapReadReply.risRet;
        }

        if (remainingLength < pcapReadReply.size) {
            pcapReadReply.size = remainingLength;
        }

        memcpy(&pcapBuffer[offset], pcapReadReply.data, pcapReadReply.size);
        offset          += pcapReadReply.size;
        *totalLength    += pcapReadReply.size;
        remainingLength -= pcapReadReply.size;
    } while ((!pcapReadReply.done) && (remainingLength > 0));
    
    if (!pcapReadReply.done) {
        RIS_PRINTF("Error Attempted to read more than %d bytes!\n",
                   pcapBufferLen);
    }

    return pcapReadReply.risRet;
}

