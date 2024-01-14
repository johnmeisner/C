/**************************************************************************
 *                                                                        *
 *     File Name:  alsmi_api.c                                            *
 *     Author:                                                            *
 *         DENSO International America, Inc.                              *
 *         North America Research and Development, California Office      *
 *         3252 Business Park Drive                                       *
 *         Vista, CA 92081                                                *
 *                                                                        *
 **************************************************************************/
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "rs.h"
#include "alsmi_api.h"
#include "ipcsock.h"
#include "type2str.h"

/*----------------------------------------------------------------------------*/
/* Local Variables                                                            */
/*----------------------------------------------------------------------------*/
static bool_t initialized    = FALSE;           /* TRUE if smi is initialized */
static int radioServicesSock = -1;              /* Socket for communicating with radioServices */
static smi_sign_results_p smiSignResultsCallback           = NULL;
                                                /* Pointer to sign results callback function */
static smi_sign_results_p smiSignWSAResultsCallback        = NULL;
                                                /* Pointer to sign WSA results callback function */
static smi_verify_wsa_results_p smiVerifyWSAResultsCallback = NULL;
                                                /* Pointer to verify WSA results callback function */
static smi_cert_change_p smiCertChangeResultsCallback       = NULL;
                                                /* Pointer to certificate change callback function */
static uint8_t regIdx;

/*------------------------------------------------------------------------------
** Function:  communicateWithRadioServices
** @brief  Send a message to Radio Services and get its reply.
** @param  msg       -- [input]Message to send to Radio Services
** @param  size      -- [input]Size of the message
** @param  reply     -- [output]Buffer to hold the received reply
** @param  replySize -- [input]Size of the reply buffer
** @return Size of the message received for success; -1 for error. Will never
**         return 0.
**
** Details: If it hasn't been done previously, creates the socket to
**          communicate with Radio Services. Sends a message to Radio Services
**          via wsuSendData() using port RS_RECV_PORT. Blocks waiting  for areply via
**          reply via wsuReceiveData().
**
**          Error messages are printed (maximum of 10 times) on errors of
**          socket creating, sending the message, or receiving the message.
**----------------------------------------------------------------------------*/
static int32_t communicateWithRadioServices(void *msg, size_t size, void *reply,
                                            size_t replySize)
{
    RcvDataType rcvData;
    int32_t     rcvSize=0;

    /* Open the socket to communicate with radioServices with if it isn't
     * already opened */
    if (radioServicesSock < 0) {
        radioServicesSock = wsuConnectSocket(-1 /* port ignored */);

        if (radioServicesSock < 0) {
            /* Connect failed */
            printf("wsuConnectSocket() failed\n");
            return -1;
        }
    }

    /* Send the message to Radio Services */
    if (!wsuSendData(radioServicesSock, RS_RECV_PORT, msg, size)) {
        /* Send failed */
        printf("wsuSendData() failed\n");
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

    return rcvSize;
}

/*
 * smiInitialize: This function should be called by application during the
 *                application startup when security is enabled.
 *
 * Input parameters:
 *        securityInit : Pointer to structure containing smi initialization
 *                       parameters.
 * Output parameters:
 *        None
 * Return Value:
 *        Success(0) or failure(-1).
 */
int smiInitialize(rsSecurityInitializeType *securityInit)
{
    rsSecurityInitializeReplyType securityInitReply;
    int32_t                       size;

    if (!initialized) {
        /* Connect to radioServices if necessary */
        if (radioServicesSock < 0) {
            /* Connect to radioServices */
            radioServicesSock = wsuConnectSocket(-1 /* port ignored */);

            if (radioServicesSock < 0) {
                /* Connect failed */
                printf("alsmi_api %s: wsuConnectSocket() failed\n", __func__);
                goto smiInitializeErr;
            }
        }

        /* Send a security initialize message to NS and wait for a reply */
        securityInit->cmd                        = WSU_NS_SECURITY_INITIALIZE;
        securityInit->smi_sign_results_pid       =
        securityInit->smi_sign_wsa_results_pid   =
        securityInit->smi_verify_wsa_results_pid =
        securityInit->smi_cert_change_pid        = getpid();
        size = communicateWithRadioServices(securityInit, sizeof(*securityInit),
                                            &securityInitReply,
                                            sizeof(securityInitReply));

        if (size < 0) {
            printf("alsmi_api %s: communicateWithRadioServices() failed\n",
                   __func__);
            goto smiInitializeErr;
        }
        else if (securityInitReply.risRet != RS_SUCCESS) {
            static int counter = 0;

            if (counter < 10) {
                printf("alsmi_api: WSU_NS_SECURITY_INITIALIZE replied with %s\n",
                       rsResultCodeType2Str(securityInitReply.risRet));
                counter++;
            }

            goto smiInitializeErr;
        }

        /* Save the regIdx returned by WSU_NS_SECURITY_INITIALIZE */
        regIdx = securityInitReply.regIdx;

        /* Save the various callback function pointers */
        smiSignResultsCallback       = securityInit->smi_sign_results_callback;
        smiSignWSAResultsCallback    = securityInit->smi_sign_wsa_results_callback;
        smiVerifyWSAResultsCallback  = securityInit->smi_verify_wsa_results_callback;
        smiCertChangeResultsCallback = securityInit->smi_cert_change_callback;
        initialized                  = TRUE;
    }

    return 0;

smiInitializeErr:
    if (radioServicesSock >= 0) {
        close(radioServicesSock);
        radioServicesSock = -1;
    }

    return -1;
}

/*
 * smiTerm: Function to close and cleanup the resources. Application should
 *          call this during application shutdown.
 *
 * Input parameters:
 *        None.
 * Output parameters:
 *        None.
 * Return Value:
 *        void
 */
void smiTerm(void)
{
    rsSecurityTerminateType      securityTerm;
    rsSecurityTerminateReplyType securityTermReply;
    int32_t                      size;

    if (initialized) {
        /* Send a security terminate message to NS */
        securityTerm.cmd                 = WSU_NS_SECURITY_TERMINATE;
        securityTerm.regIdx              = regIdx;
        securityTerm.sign_wsa_results    = (smiSignWSAResultsCallback != NULL)
                                           ? TRUE : FALSE;
        securityTerm.verify_wsa_results  = (smiVerifyWSAResultsCallback != NULL)
                                           ? TRUE : FALSE;
        securityTerm.cert_change_results = (smiCertChangeResultsCallback != NULL)
                                           ? TRUE : FALSE;

        /* Send a security terminate message to NS and wait for a reply */
        size = communicateWithRadioServices(&securityTerm, sizeof(securityTerm),
                                            &securityTermReply,
                                            sizeof(securityTermReply));

        if (size < 0) {
            printf("alsmi_api %s: communicateWithRadioServices() failed\n",
                   __func__);
        }
        else if (securityTermReply.risRet != RS_SUCCESS) {
            static int counter = 0;

            if (counter < 10) {
                printf("alsmi_api: WSU_NS_SECURITY_TERMINATE replied with %s\n",
                       rsResultCodeType2Str(securityTermReply.risRet));
                counter++;
            }
        }

        /* Close FD connection Radio Stack QNX Resource Manager */
        close(radioServicesSock);
        radioServicesSock = -1;

        /* Indicate no callbacks */
        smiSignWSAResultsCallback    = NULL;
        smiVerifyWSAResultsCallback  = NULL;
        smiCertChangeResultsCallback = NULL;
        smiSignResultsCallback       = NULL;
        initialized = FALSE;
    }
}

/*
 * smiUpdatePositionAndTime: API to update the GPS position and the number of
 *          leap seconds since 2004 for security.
 *
 * Input parameters:
 *        latitude
 *        longitude
 *        altitude
 *        leapSecondsSince2004
 *        countryCode
 * Output parameters:
 *        None.
 * Return Value:
 *        None.
 */
void smiUpdatePositionAndTime(double latitude, double longitude,
                              double altitude, int16_t leapSecondsSince2004,
                              uint16_t countryCode)
{
    rsSecurityUpdatePositionAndTimeType updateMsg;
    rsResultCodeType                    risRet;
    int32_t                             size;

    /* Connect to radioServices if necessary */
    if (radioServicesSock < 0) {
        /* Connect to radioServices */
        radioServicesSock = wsuConnectSocket(-1 /* port ignored */);

        if (radioServicesSock < 0) {
            /* Connect failed */
            printf("alsmi_api %s: wsuConnectSocket() failed\n", __func__);
            return;
        }
    }

    updateMsg.cmd                  = WSU_NS_SECURITY_UPDATE_POSITION_AND_TIME;
    updateMsg.latitude             = latitude;
    updateMsg.longitude            = longitude;
    updateMsg.elevation            = altitude;
    updateMsg.leapSecondsSince2004 = leapSecondsSince2004;
    updateMsg.countryCode          = countryCode;

    /* Send an update position and time message to NS and wait for a reply */
    size = communicateWithRadioServices(&updateMsg, sizeof(updateMsg),
                                        &risRet, sizeof(risRet));

    if (size < 0) {
        printf("alsmi_api %s: communicateWithRadioServices() failed\n",
               __func__);
    }
    else if (risRet != RS_SUCCESS) {
        static int counter = 0;

        if (counter < 10) {
            printf("alsmi_api: WSU_NS_SECURITY_UPDATE_POSITION_AND_TIME replied with %s\n",
                   rsResultCodeType2Str(risRet));
            counter++;
        }
    }
}

/*
 * smiGetLeapSecondsSince2004_1: Get number of leap seconds since 2004 from the
 *                               radio stack.
 *
 * Input parameters:
 *        None.
 * Output parameters:
 *        None.
 * Return Value:
 *        The number of leap seconds since 2004.
 */
int16_t smiGetLeapSecondsSince2004_1(void)
{
    rsIoctlType               cmd = WSU_NS_SECURITY_GET_LEAP_SECONDS_SINCE_2004;
    rsGetLeapSecondsReplyType getLeapSecondsReply;
    int32_t                   size;

    /* Connect to radioServices if necessary */
    if (radioServicesSock < 0) {
        /* Connect to radioServices */
        radioServicesSock = wsuConnectSocket(-1 /* port ignored */);

        if (radioServicesSock < 0) {
            /* Connect failed */
            printf("alsmi_api %s: wsuConnectSocket() failed\n", __func__);
            return 0;
        }
    }

    /* Send a get leap seconds since 2004 message to NS and wait for a reply */
    size = communicateWithRadioServices(&cmd, sizeof(cmd),
                                        &getLeapSecondsReply,
                                        sizeof(getLeapSecondsReply));

    if (size < 0) {
        printf("alsmi_api %s: communicateWithRadioServices() failed\n",
               __func__);
        return 0;
    }
    else if (getLeapSecondsReply.risRet != RS_SUCCESS) {
        static int counter = 0;

        if (counter < 10) {
            printf("alsmi_api: WSU_NS_SECURITY_GET_LEAP_SECONDS_SINCE_2004 replied with %s\n",
                   rsResultCodeType2Str(getLeapSecondsReply.risRet));
            counter++;
        }

        return 0;
    }

    return getLeapSecondsReply.leapSeconds;
}

/*
 * smiGetLeapSecondsSince2004: API to get number of leap seconds since 2004.
 *                             Only does a call to the radio stack once every
 *                             5 minutes. Otherwise, uses the last value it got.
 *
 * Input parameters:
 *        None.
 * Output parameters:
 *        None.
 * Return Value:
 *        The number of leap seconds since 2004.
 */
int16_t smiGetLeapSecondsSince2004(void)
{
    #define GET_LEAP_SECONDS_MIN_WAIT_S 300
    static int16_t secondsSince2004Value = 0;
    static struct timeval tv1;
    struct timeval tv2;
    int16_t ret;

    if (secondsSince2004Value == 0) {
        gettimeofday(&tv1, NULL);
        secondsSince2004Value = smiGetLeapSecondsSince2004_1();
    }
    else {
        gettimeofday(&tv2, NULL);

        if (tv2.tv_sec >= tv1.tv_sec + GET_LEAP_SECONDS_MIN_WAIT_S) {
            ret = smiGetLeapSecondsSince2004_1();

            if (ret != 0) {
                tv1 = tv2;
                secondsSince2004Value = ret;
            }
        }
    }

    return secondsSince2004Value;
}

/*
 * smiSendCertChangeReq: API to send certificate change request.
 *
 * Input parameters:
 *        None.
 * Output parameters:
 *        None.
 * Return Value:
 *        0 on Success -1 on failure.
 */
int smiSendCertChangeReq(void)
{
    rsIoctlType      cmd = WSU_NS_SECURITY_SEND_CERT_CHANGE_REQ;
    rsResultCodeType risRet;
    int32_t          size;

    if (!initialized) {
        static int counter = 0;

        if (counter < 10) {
            fprintf(stderr, "%s: Security not initialized!\n", __func__);
            counter++;
        }

        return -1;
    }

    /* Send a certificate change request message to NS and wait for a reply */
    size = communicateWithRadioServices(&cmd, sizeof(cmd),
                                        &risRet, sizeof(risRet));

    if (size < 0) {
        printf("alsmi_api %s: communicateWithRadioServices() failed\n",
               __func__);
        return -1;
    }
    else if (risRet != RS_SUCCESS) {
        static int counter = 0;

        if (counter < 10) {
            printf("alsmi_api: WSU_NS_SECURITY_SEND_CERT_CHANGE_REQ replied with %s\n",
                   rsResultCodeType2Str(risRet));
            counter++;
        }

        return -1;
    }

    return 0;
}

/*
 * smiGetAerolinkVersion: Get the current Aerolink version
 *
 * Input parameters:
 *        len - The length of buf
 * Output parameters:
 *        buf - Where to put the Aerolink version string
 * Return Value:
 *        0 on Success -1 on failure.
 */
int smiGetAerolinkVersion(char *buf, int len)
{
    rsIoctlType                   cmd = WSU_NS_GET_AEROLINK_VERSION;
    rsGetAerolinkVersionReplyType getAerolinkVersionReply;
    int32_t                       size;

    /* Connect to radioServices if necessary */
    if (radioServicesSock < 0) {
        /* Connect to radioServices */
        radioServicesSock = wsuConnectSocket(-1 /* port ignored */);

        if (radioServicesSock < 0) {
            /* Connect failed */
            printf("alsmi_api %s: wsuConnectSocket() failed\n", __func__);
            return 0;
        }
    }

    /* Send a certificate change request message to NS and wait for a reply */
    size = communicateWithRadioServices(&cmd, sizeof(cmd),
                                        &getAerolinkVersionReply,
                                        sizeof(getAerolinkVersionReply));

    if (size < 0) {
        printf("alsmi_api %s: communicateWithRadioServices() failed\n",
               __func__);
        return -1;
    }
    else if (getAerolinkVersionReply.risRet != RS_SUCCESS) {
        static int counter = 0;

        if (counter < 10) {
            printf("alsmi_api: WSU_NS_GET_AEROLINK_VERSION replied with %s\n",
                   rsResultCodeType2Str(getAerolinkVersionReply.risRet));
            counter++;
        }

        return -1;
    }

    if (len > MAX_AEROLINK_VERSION_LEN) {
        len = MAX_AEROLINK_VERSION_LEN;
    }

    strncpy(buf, getAerolinkVersionReply.aerolinkVersion, len);
    buf[len - 1] = '\0';
    return 0;
}

/*
 * processSmiSignResults: Process sign results. This function should only be
 *                        called if recv->indType equals IND_SIGN_WSM_RESULTS
 *                        or IND_SIGN_WSA_RESULTS. An smiSighResults_t
 *                        structure is filled out and sent back to either
 *                        the sign results callback function or the sign WSA
 *                        results callback.
 *
 * Input parameters:
 *        recv: The IND_SIGN_WSM_RESULTS or IND_SIGN_WSM_RESULTS received from
 *              radioServices.
 * Output parameters:
 *        none
 * Return Value:
 *        void
 */
void processSmiSignResults(rsReceiveDataType *recv)
{
    signResultsType  *recvSignResults = &recv->u.signResults;
    smiSignResults_t  signResults;

    /* Fill out the signResults fields */
    memset(&signResults, 0, sizeof(signResults));
    /* VJR WILLBEREVISITED These 4 fields in signResults are not currently
     * filled out */
#if 0
    smi_signer_type_e signerType;
    uint8_t           digest[8];
    uint64_t          msgGenTime;
    uint8_t           msgGenTimeConf;
#endif
    signResults.psid               = recvSignResults->psid;
    signResults.signStatus         = recvSignResults->securitySignStatus;
    signResults.signErrorCode      = recvSignResults->returnCode;
    signResults.wsmLenWithSecurity = recvSignResults->signedDataLength;

    /* Call the callback function with the results */
    if (recv->indType == IND_SIGN_WSA_RESULTS) {
        if (smiSignWSAResultsCallback != NULL) {
            smiSignWSAResultsCallback(&signResults);
        }
    }
    else {
        if (smiSignResultsCallback != NULL) {
            smiSignResultsCallback(&signResults);
        }
    }
}

/*
 * processSmiVerifyWSAResults: Process verify WSA results. This function should
 *                             only be called if recv->indType equals
 *                             IND_VERIFY_WSA_RESULTS. calls the verify WSa
 *                             results callback function.
 *
 * Input parameters:
 *        recv: The IND_VERIFY_WSA_RESULTS received from radioServices.
 * Output parameters:
 *        none
 * Return Value:
 *        void
 */
void processSmiVerifyWSAResults(rsReceiveDataType *recv)
{
    /* Call the callback function with the results */
    if (smiVerifyWSAResultsCallback != NULL) {
        smiVerifyWSAResultsCallback(recv->u.returnCode);
    }
}

/*
 * processCertChangeResults: Process certificate change results. This function
 *                           should only be called if recv->indType equals
 *                           IND_CERT_CHANGE_RESULTS. Calls the certificate
 *                           change results callback function.
 *
 * Input parameters:
 *        recv: The IND_CERT_CHANGE_RESULTS received from radioServices.
 * Output parameters:
 *        none
 * Return Value:
 *        void
 */
void processCertChangeResults(rsReceiveDataType *recv)
{
    /* Call the callback function with the results */
    if (smiCertChangeResultsCallback != NULL) {
        smiCertChangeResultsCallback(&recv->u.certChangeResults);
    }
}

