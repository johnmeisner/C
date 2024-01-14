/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Filename: rpsi.c                                                           */
/*                                                                            */
/* Copyright (C) 2022 DENSO International America, Inc.                       */
/*                                                                            */
/* Description: Interface between Radio Services and the applications         */
/*              requesting service from it.                                   */
/* Revision History:                                                          */
/*                                                                            */
/* Date        Author        Comments                                         */
/* -------------------------------------------------------------------------- */
/* [2019-03-07][VROLLINGER]  Initial revision                                 */
/*----------------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include "ipcsock.h"
#include "nscfg.h"
#include "rpsi.h"
#include "shm_rsk.h"
#include "wme.h"
#include "alsmi.h"
#include "rsu_defs.h"
#include "ns_cv2x.h"
#include "ns_pcap.h"
#include "i2v_util.h"
#include "nscfg.h"

/* mandatory logging defines */
#define MY_ERR_LEVEL   LEVEL_PRIV    /* from i2v_util.h */
#define MY_NAME        "rps"

extern shm_rsk_t *shm_rsk_ptr;
extern bool_t     ignoreWSAs;
/*----------------------------------------------------------------------------*/
/* Local Variables                                                            */
/*----------------------------------------------------------------------------*/
static pthread_t radioServicesReceiveCommandsThreadId;
static bool_t    radioServicesReceiveCommandsThreadRunning = FALSE;
static int       recvCmdSock = -1;
static bool_t    pcap_readStarted = FALSE;
static uint8_t  *pcap_buf = NULL;
static uint32_t  pcap_len = 0;
static uint8_t   pcap_bufferNum = 0;
static int       pcap_offset = 0;
static int16_t   test_seconds = 0;
static rsGetStringReplyType  strReply;
/*----------------------------------------------------------------------------*/
/* Global Variables                                                           */
/*----------------------------------------------------------------------------*/
bool_t smiInitialized = FALSE;

/*----------------------------------------------------------------------------*/
/* Functions                                                                  */
/*----------------------------------------------------------------------------*/
void initRpsi(uint32_t debugEnable)
{
  smiInitialized = FALSE;
  memset(&radioServicesReceiveCommandsThreadId,0,sizeof(radioServicesReceiveCommandsThreadId));
  radioServicesReceiveCommandsThreadRunning = FALSE;
  recvCmdSock = -1;
  pcap_readStarted = FALSE;
  pcap_buf = NULL;
  pcap_len = 0;
  pcap_bufferNum = 0;
  pcap_offset = 0;
  test_seconds = 0;
  memset(&strReply,0,sizeof(strReply));
}

/**
** Function:  processUserServiceReq
** @brief  Process WSU_NS_USER_SERVICE_REQ message
** @param  service -- [input]Message to process
** @return RS_SUCCESS if success, error code if failure
**
** Details: Process the WSU_NS_USER_SERVICE_REQ message. Either start a user
**          service (if service->action == ADD) or terminate a user service (if
**          service->action == DELETE) by calling userServiceReq() in wme.c.
**/
static rsResultCodeType processUserServiceReq(UserServiceType *service)
{
    shm_rsk_ptr->RISReqCnt.UserServiceReq++;

    if ((service->radioType == RT_CV2X) && (!getCV2XEnable()))  {
        return RS_NOTENABLED;
    }

    return userServiceReq(service);
}

/**
** Function:  processProviderServiceReq
** @brief  Process WSU_NS_PROVIDER_SERVICE_REQ message
** @param  service -- [input]Message to process
** @return RS_SUCCESS if success, error code if failure
**
** Details: Process the WSU_NS_PROVIDER_SERVICE_REQ message. Either start a provider
**          service (if service->action == ADD) or terminate a provider service (if
**          service->action == DELETE) by calling providerServiceReq() in wme.c.
**/
static rsResultCodeType processProviderServiceReq(ProviderServiceType *service)
{
    shm_rsk_ptr->RISReqCnt.ProvServiceReq++;

    if ((service->radioType == RT_CV2X) && (!getCV2XEnable()))  {
        return RS_NOTENABLED;
    }

    return providerServiceReq(service);
}

/**
** Function:  processSendWsmReq
** @brief  Process WSU_NS_SEND_WSM_REQ message
** @param  WSM -- [input]Message to process
** @return RS_SUCCESS if success, error code if failure
**
** Details: Process the WSU_NS_SEND_WSM_REQ message. Sends the WSM by calling
**          sendWSM() in wme.c.
**/
static rsResultCodeType processSendWsmReq(outWSMType *WSM)
{
    shm_rsk_ptr->RISReqCnt.SendWSMReq[WSM->radioType]++;

    if ((WSM->radioType == RT_CV2X) && (!getCV2XEnable())) {
        return RS_NOTENABLED;
    }

    return sendWSM(WSM);
}

/**
** Function:  processGetNsCfg
** @brief  Process WSU_NS_GET_NS_CFG message
** @param  cfg -- [input/output]Config ID and radio type are read from here.
**                Also, cfg is the pointer to where to write the configuration
**                value once we get it.
** @return RS_SUCCESS if success, error code if failure
**
** Details: Process the WSU_NS_GET_NS_CFG message. Gets the requested
**          configuration value by calling either nsGetCfg() or dsrc_get_cfg().
**/
static rsResultCodeType processGetNsCfg(cfgType *cfg)
{
    shm_rsk_ptr->RISReqCnt.GetCfgReq++;

    /* Get Configuration value */
    switch (cfg->cfgId) {
    case CFG_ID_CV2X_ENABLE:
    case CFG_ID_STATION_ROLE:
    case CFG_ID_REPEATS:
    case CFG_ID_CCH_NUM_RADIO_0:
    case CFG_ID_CCH_NUM_RADIO_1:
    case CFG_ID_SCH_NUM_RADIO_0:
    case CFG_ID_SCH_NUM_RADIO_1:
    case CFG_ID_SCH_ADAPTABLE:
    case CFG_ID_SCH_DATARATE:
    case CFG_ID_SCH_TXPWRLEVEL:
    case CFG_ID_SERVICE_INACTIVITY_INTERVAL:
    case CFG_ID_WSA_TIMEOUT_INTERVAL:
    case CFG_ID_WSA_TIMEOUT_INTERVAL2:
    case CFG_ID_RX_QUEUE_THRESHOLD:
    case CFG_ID_WSA_CHANNEL_INTERVAL:
    case CFG_ID_WSA_PRIORITY:
    case CFG_ID_WSA_DATARATE:
    case CFG_ID_WSA_TXPWRLEVEL:
    case CFG_ID_P2P_CHANNEL_INTERVAL:
    case CFG_ID_P2P_PRIORITY:
    case CFG_ID_P2P_DATARATE:
    case CFG_ID_P2P_TXPWRLEVEL:
    case CFG_ID_IPV6_PRIORITY:
    case CFG_ID_IPV6_DATARATE:
    case CFG_ID_IPV6_TXPWRLEVEL:
    case CFG_ID_IPV6_ADAPTABLE:
    case CFG_ID_IPV6_PROVIDER_MODE:
    case CFG_ID_IPV6_RADIO_TYPE:
    case CFG_ID_IPV6_RADIO_NUM:
    case CFG_ID_CONDUCTED_2_RADIATED_POWER_OFFSET_RADIO_0:
    case CFG_ID_CONDUCTED_2_RADIATED_POWER_OFFSET_RADIO_1:
    case CFG_ID_CHANGE_MAC_ADDRESS_ENABLE:
    case CFG_ID_RCV_EU_FROM_ETH_ENABLE:
    case CFG_ID_RCV_EU_FROM_ETH_PORT_NUM:
    case CFG_ID_CV2X_PWR_CTRL_ENABLE:
    case CFG_ID_DISCARD_UNSECURE_MSG:
    case CFG_ID_DISCARD_VERFAIL_MSG:
    case CFG_ID_PRIMARY_SERVICE_ID:
    case CFG_ID_SECONDARY_SERVICE_ID:
    case CFG_ID_ALSMI_DEBUG_LOG_ENABLE:
        /* If it is any of these CFG_ID's, send it to nsGetCfg() to get
         * the config variable value */
        return nsGetCfg(cfg);

    case CFG_ID_ANTENNA:
    case CFG_ID_ANTENNA_RADIO_0:
    case CFG_ID_ANTENNA_RADIO_1:
    case CFG_ID_ANTENNA_TXOVERRIDE_RADIO_0: /* MK5-Radio only limited use command */
    case CFG_ID_ANTENNA_TXOVERRIDE_RADIO_1:
    case CFG_ID_MACADDRESS_RADIO_0:
    case CFG_ID_MACADDRESS_RADIO_1:
    case CFG_ID_EDCA_PARAMS:
    case CFG_ID_TX_MAX_POWER_LEVEL:
    case CFG_ID_TX_MAX_POWER_LEVEL_RADIO_0:
    case CFG_ID_TX_MAX_POWER_LEVEL_RADIO_1:
    case CFG_ID_MACADDRESS_ORIG_RADIO_0:
    case CFG_ID_MACADDRESS_ORIG_RADIO_1:

        /* These commands are not supported for C-V2X */
        return RS_NOTENABLED;

    default:
        shm_rsk_ptr->RISReqCnt.UnknownReq++;
        return RS_ERANGE;
    }
}

/**
** Function:  processSetNsCfg
** @brief  Process WSU_NS_SET_NS_CFG message
** @param  cfg -- [input]Message structure
** @return RS_SUCCESS if success, error code if failure
**
** Details: Process the WSU_NS_SET_NS_CFG message. Sets the requested
**          configuration value by calling either nsSetCfg() or dsrc_set_cfg().
**/
static rsResultCodeType processSetNsCfg(cfgType *cfg)
{
    shm_rsk_ptr->RISReqCnt.SetCfgReq++;

    /* Set Configuration value */
    switch (cfg->cfgId) {
    case CFG_ID_CV2X_ENABLE:
    case CFG_ID_STATION_ROLE:
    case CFG_ID_REPEATS:
    case CFG_ID_CCH_NUM_RADIO_0:
    case CFG_ID_CCH_NUM_RADIO_1:
    case CFG_ID_SCH_NUM_RADIO_0:
    case CFG_ID_SCH_NUM_RADIO_1:
    case CFG_ID_SCH_ADAPTABLE:
    case CFG_ID_SCH_DATARATE:
    case CFG_ID_SCH_TXPWRLEVEL:
    case CFG_ID_SERVICE_INACTIVITY_INTERVAL:
    case CFG_ID_WSA_TIMEOUT_INTERVAL:
    case CFG_ID_WSA_TIMEOUT_INTERVAL2:
    case CFG_ID_RX_QUEUE_THRESHOLD:
    case CFG_ID_WSA_CHANNEL_INTERVAL:
    case CFG_ID_WSA_PRIORITY:
    case CFG_ID_WSA_DATARATE:
    case CFG_ID_WSA_TXPWRLEVEL:
    case CFG_ID_P2P_CHANNEL_INTERVAL:
    case CFG_ID_P2P_PRIORITY:
    case CFG_ID_P2P_DATARATE:
    case CFG_ID_P2P_TXPWRLEVEL:
    case CFG_ID_IPV6_PRIORITY:
    case CFG_ID_IPV6_DATARATE:
    case CFG_ID_IPV6_TXPWRLEVEL:
    case CFG_ID_IPV6_ADAPTABLE:
    case CFG_ID_IPV6_PROVIDER_MODE:
    case CFG_ID_IPV6_RADIO_TYPE:
    case CFG_ID_IPV6_RADIO_NUM:
    case CFG_ID_CONDUCTED_2_RADIATED_POWER_OFFSET_RADIO_0:
    case CFG_ID_CONDUCTED_2_RADIATED_POWER_OFFSET_RADIO_1:
    case CFG_ID_CHANGE_MAC_ADDRESS_ENABLE:
    case CFG_ID_RCV_EU_FROM_ETH_ENABLE:
    case CFG_ID_RCV_EU_FROM_ETH_PORT_NUM:
    case CFG_ID_CV2X_PWR_CTRL_ENABLE:
    case CFG_ID_DISCARD_UNSECURE_MSG:
    case CFG_ID_DISCARD_VERFAIL_MSG:
    case CFG_ID_PRIMARY_SERVICE_ID:
    case CFG_ID_SECONDARY_SERVICE_ID:
    case CFG_ID_ALSMI_DEBUG_LOG_ENABLE:
        /* If it is any of these CFG_ID's, send it to nsSetCfg() to set the
         * config variable value */
        return nsSetCfg(cfg);

    case CFG_ID_ANTENNA:
    case CFG_ID_ANTENNA_RADIO_0:
    case CFG_ID_ANTENNA_RADIO_1:
    case CFG_ID_ANTENNA_TXOVERRIDE_RADIO_0:
    case CFG_ID_ANTENNA_TXOVERRIDE_RADIO_1:
    case CFG_ID_MACADDRESS_RADIO_0:
    case CFG_ID_MACADDRESS_RADIO_1:
    case CFG_ID_RECEIVER_MODE_RADIO_0:
    case CFG_ID_RECEIVER_MODE_RADIO_1:
    case CFG_ID_EDCA_PARAMS:

        /* These commands are not supported for C-V2X */
        return RS_SUCCESS;

    default:
        shm_rsk_ptr->RISReqCnt.UnknownReq++;
        return RS_ERANGE;
    }
}

/**
** Function:  processGetAerolinkVersion
** @brief  Process WSU_NS_GET_AEROLINK_VERSION message
** @return RS_SUCCESS for success
**
** Details: Process the WSU_NS_GET_AEROLINK_VERSION message. Calls
**          alsmiGetVersion() to do the work.
**/
#if !defined(NO_SECURITY)
static rsResultCodeType processGetAerolinkVersion(char *aerolinkVersion)
{
    const char *version = alsmiGetVersion();
    size_t      len = strlen(version) + 1; /* Allow room for trailing '\0' */
    
    /* ALSMI shared memory statistics are kept in alsmi.c */
    if (len > MAX_AEROLINK_VERSION_LEN - 1) {
        len = MAX_AEROLINK_VERSION_LEN - 1;
    }
    
    memcpy(aerolinkVersion, version, len);
    aerolinkVersion[MAX_AEROLINK_VERSION_LEN - 1] = '\0';
    return RS_SUCCESS;
}
#endif
/**
** Function:  processTestCmd
** @brief  Process WSU_NS_TEST_CMD message
** @return RS_SUCCESS for success, error code for failure
**
** Details: Checks the testCmd field of the message and calls the appropriate
**          routine to execute the command. For RS_TEST_CMD_IGNORE_WSAS, it
**          launches a thread; this is becasue we want to set a variable, wait
**          several seconds, and then unset the variable. Launching a separate
**          thread will prevent the "receive command" thread from being hung
**          up for those several seconds.
**/
static rsResultCodeType processTestCmd(rsTestCmdType *test)
{
    pthread_t      ignoreWSAsThreadID;

    shm_rsk_ptr->RISReqCnt.TestCmd++;

    switch(test->testCmd) {
#ifndef NO_SECURITY
    case RS_TEST_CMD_CERT_CHANGE_TIMEOUT:
        alsmiSetCertChangeTimeoutTestCount(test->u.count);
        break;

    case RS_TEST_CMD_UNSOLICITED_CERT_CHANGE:
        alsmiUnsolicitedCertChangeTest();
        break;

#endif
    case RS_TEST_CMD_IGNORE_WSAS:
        /* Launch a thread so as not to hold up the radioServices receive
         * commands thread */
        if (!ignoreWSAs) {
            test_seconds = test->u.seconds;

            if (pthread_create(&ignoreWSAsThreadID, NULL, ignoreWSAsThread,
                               &test_seconds) != 0) {
                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Error creating ignore WSA's thread: %s\n",
                           strerror(errno));
            }
        }

        break;

    default:
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"WSU_NS_TEST_CMD: Unknown command (%u)\n", test->testCmd);
        break;
    }

    return RS_SUCCESS;
}

/**
** Function:  processUserServiceReq
** @brief  Process WSU_NS_SEND_RAW_PACKET_REQ message
** @param  packet -- [input]Message to process
** @return RS_SUCCESS if success, error code if failure
**
** Details: Determines if the desired radio is enabled; returns RS_NOTENABLED
**          if not. Otherwise, calls sendRawPacket() to do the work.
**/
static rsResultCodeType processSendRawPacketReq(outRawType *packet)
{
    shm_rsk_ptr->RISReqCnt.SendRawPktReq[packet->radioType]++;

    if (((packet->radioType == RT_CV2X) && (!getCV2XEnable()))) {
        return RS_NOTENABLED;
    }

    return sendRawPacket(packet);
}

/**
** Function:  processGetRadioTallies
** @brief  Process WSU_NS_GET_RADIO_TALLIES message
** @param  getRadioTallies -- [input]Pointer to the get radio tallies command
** @param  tallies         -- [output]Pointer to the structure to hold the
**                            radio tallies
** @return RS_SUCCESS if success, error code if failure
**
** Details: Calls cv2x_get_radio_tallies() if the radio type is RT_CV2X.
**          Otherwise, calls dsrc_get_radio_tallies().
**/
static rsResultCodeType processGetRadioTallies(
                            rsGetRadioTalliesCmdType *getRadioTallies,
                            DeviceTallyType          *tallies
                        )
{
    if (getRadioTallies->radioType == RT_CV2X) {
        if (rs_cv2x_get_radio_tallies(getRadioTallies->radioNum, tallies) < 0) {
            return RS_ECV2XRAD;
        }
    }
    return RS_SUCCESS;
}

/**
** Function:  processPcapRead
** @brief  Read up to MAX_PCAP_READ_BYTES bytes from the PCAP buffer. Note that
**         repeated processPcapRead() MUST be made until pcapReadReply->done
**         becomes TRUE.
** @param  pcapRead      -- [input]PCAP read structure with input parameters
** @param  pcapReadReply -- [output]PCAP read reply structure
** @return None.
**
** Details: If a new read needs to be started (readStarted == FALSE), starts
**          one off by calling pcap_read1() to get the address of the buffer
**          (buf), the length of the buffer (len), and the buffer number to
**          pass to an eventual call to pcap_read2() (bufferNum). readStarted
**          is set to TRUE to indicate that a read has been started.
**
**          If more than MAX_PCAP_READ_BYTES need to be read, then
**          MAX_PCAP_READ_BYTES bytes are read via
**          memcpy(pcapReadReply->data, &buf[offset], MAX_PCAP_READ_BYTES),
**          and the offset into the buffer for the next read (offset) is
**          incremented by MAX_PCAP_READ_BYTES bytes and the remaining length
**          (len) is decremented by the same amount.
**
**          If MAX_PCAP_READ_BYTES or fewer need to be read, then the remaining
**          bytes are read via memcpy(pcapReadReply->data, &buf[offset], len).
**          Then the read is finished off by a call to pcap_read2() and the
**          system is set up to start off a new read next time by setting
**          readStarted to FALSE.
**/
static rsResultCodeType processPcapRead(rsPcapReadType      *pcapRead,
                                        rsPcapReadReplyType *pcapReadReply)
{
    int32_t ret = 0;

    if (!pcap_readStarted) {
        /* Start off a new read */
#if defined(EXTRA_DEBUG)
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Calling pcap_read1()\n");
#endif
        pcapReadReply->size = 0;
        pcap_len = 0;
        ret = pcap_read1(pcapRead->radioType, pcapRead->radioNum, pcapRead->tx,&pcap_buf, &pcap_len, &pcap_bufferNum);

        switch(ret) {
            case -2:
                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"pcap_read1: RS_ELOCKFAIL\n");
                pcapReadReply->done = TRUE;
                pcapReadReply->size = 0;
                return RS_ELOCKFAIL;
                break;
            case -1:
                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"pcap_read1: RS_ERANGE\n");
                pcapReadReply->done = TRUE;
                pcapReadReply->size = 0;
                return RS_ERANGE;
                break;
            default:
            break;
        }
        pcap_offset      = 0;
        pcap_readStarted = TRUE;
    }
    
    if (pcap_len > MAX_PCAP_READ_BYTES) {
        /* More than MAX_PCAP_READ_BYTES remain to be read; read the first
         * MAX_PCAP_READ_BYTES bytes and prepare for a subsequent
         * processPcapRead() */
        pcapReadReply->done = FALSE;
        pcapReadReply->size = MAX_PCAP_READ_BYTES;
        memcpy(pcapReadReply->data, &pcap_buf[pcap_offset], MAX_PCAP_READ_BYTES);
        pcap_offset += MAX_PCAP_READ_BYTES;
        pcap_len    -= MAX_PCAP_READ_BYTES;
    }
    else {
        /* MAX_PCAP_READ_BYTES or less remain to be read. Read all of the
         * remaining bytes, and finish off the read. */
        pcapReadReply->done = TRUE;
        pcapReadReply->size = pcap_len;

        if (pcap_len > 0) {
            memcpy(pcapReadReply->data, &pcap_buf[pcap_offset], pcap_len);
        }

        /* Finish off the read */
        if(0 != pcap_read2(pcap_bufferNum)) {
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"pcap_read2: RS_ELOCKFAIL\n");
            pcapReadReply->done = TRUE;
            pcapReadReply->size = 0;
            pcap_readStarted = FALSE;
            return RS_ELOCKFAIL;
        }
        pcap_readStarted = FALSE;
    }

    return RS_SUCCESS;
}

/**
** Function:  rpsiSIGUSR1SignalHandler
** @brief  Signal handler for SIGUSR1, used to shut down threads.
** @param  signum -- [input]Signal received. Should be SIGUSR1.
** @return None.
**
** Details: Just prints a message saying what signal was received. The main
**          purpose of having this signal handler is to prevent the default
**          signal handler from running, which may have undesirable side
**          effects such as propogating the signal to its parent.
**/
static void rpsiSIGUSR1SignalHandler(int signum)
{
#if defined(EXTRA_DEBUG)
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"signum = %d\n", signum);
#endif
}

/**
** Function:  radioServicesReceiveCommandsThread
** @brief  Thread to receive and respond to Radio Services commands.
** @param  arg -- [input]Required by convention; not used.
** @return NULL
**
** Details: Thread creates a socket to receive commands on. Then it waits for
**          commands to be sent to it on RS_RECV_PORT. Each time it gets one,
**          it calls the proper processing function, then sends back a reply
**          to the sender.
**/
/* Thread to receive and respond to Radio Services commands */
static void *radioServicesReceiveCommandsThread(void *arg)
{
    RcvDataType                  rcvData;
    size_t                       rcvSize;
    rsResultCodeType             risRet;
    void                        *replyPtr;
    int32_t                      replyLen;

    int32_t                      ret = 0;
    union {
        rsIoctlType                         cmd;
        rsRPSMsgType                        rps;
        UserServiceType                     userService;
        /* RSU specfic. */
        ProviderServiceType                 providerService;
        WsmServiceType                      wsmService;
        outWSMType                          outWsm;
        rsGetIndPortMsgType                 getIndPort;
        cfgType                             cfg;
        rsVodMsgType                        vod;
        rsSecurityInitializeType            securityInit;
        rsSecurityTerminateType             securityTerm;
        rsSecurityUpdatePositionAndTimeType updatePositionAndTime;
        rsGetWsaInfoMsgType                 getWsaInfo;
        rsGetWsaInfoElementMsgType          getWsaInfoElement;
        UserServiceChangeType               change;
        rsTestCmdType                       testCmd;
        outRawType                          raw;
        rsGetRadioTalliesCmdType            getRadioTallies;
        rsRadioCfgType                      radioCfg;
        rsPcapEnableType                    pcapEnable;
        rsPcapDisableType                   pcapDisable;
        rsPcapReadType                      pcapRead;
    } cmdData;

    union {
        rsGetIndPortReplyType          getIndPort;
        rsVodReplyType                 vodReply;
        rsSecurityInitializeReplyType  securityInit;
        rsSecurityTerminateReplyType   securityTerm;
        rsGetLeapSecondsReplyType      getLeapSecondsSince2004;
        rsGetAerolinkVersionReplyType  getAerolinkVersion;
        DeviceTallyType                tallies;
        rsPcapReadReplyType            pcapReadReply;
        rskStatusType                  rskStatus;
    } reply;
#if defined(EXTRA_DEBUG)
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Started\n");
#endif
    radioServicesReceiveCommandsThreadRunning = TRUE;

    /* Unblock the SIGUSR1 signal and disable the default signal handler */
    signal(SIGUSR1, rpsiSIGUSR1SignalHandler);

    /* Create and bind the socket */
    recvCmdSock = wsuCreateSockServer(RS_RECV_PORT);

    if (recvCmdSock < 0) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"wsuCreateSockServer() failed\n");
        goto radioServicesReceiveCommandsThreadExit;
    }

    shm_rsk_ptr->bootupState |= 0x40; // radioServicesReceiveCommandsThread running
    /* Main receive loop */
    while (radioServicesReceiveCommandsThreadRunning) {
        /* Wait for an command */
        rcvData.available = FALSE;
        rcvData.fd        = recvCmdSock;
        rcvData.data      = &cmdData;
        rcvData.size      = sizeof(cmdData);
#if defined(EXTRA_DEBUG)
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Calling wsuReceiveData(0, &rcvData)\n");
#endif
        ret = wsuReceiveData(/* Timeout ignored */0, &rcvData);
        if (!radioServicesReceiveCommandsThreadRunning) {
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(!radioServicesReceiveCommandsThreadRunning)\n");
            break;
        }

        if (ret < 0) {
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"wsuReceiveData() error: ret=%d\n",ret);
            /* Close and recreate the socket */
            close(recvCmdSock);
            recvCmdSock = wsuCreateSockServer(RS_RECV_PORT);

            if (recvCmdSock < 0) {
                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"wsuCreateSockServer() failed\n");
                break;
            }

            continue;
        }
        rcvSize = (uint32_t)ret;
        if (rcvSize == 0) {
            /* If rcvSize is 0, then continue in the loop and try again. A
             * return value of 0 is something that can occur normally with
             * datagram sockets; wsuConnectSocket() specifies SOCK_DGRAM on its
             * call to wsuSendData(). Also, a return value of 0 does NOT
             * indicate the socket was closed. wsuReceiveData() will also
             *  return 0 if errno is set to EINTR upron return upon return from
             * recvfrom(); in this case, you should try again. */
            continue;
        }

        /* rcvSize is > 0. In this case, there is no way for rcvData.available
         * to have been set to FALSE. */

        /* Process the command. Get the reply and the reply length */
        switch (cmdData.cmd) {
        case WSU_NS_INIT_RPS:
#if defined(EXTRA_DEBUG)
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"WSU_NS_INIT_RPS; rcvData.sender_port = %u\n",
                    rcvData.sender_port);
#endif
            shm_rsk_ptr->RISReqCnt.InitRadioComm++;
            risRet = initRPS(&cmdData.rps);
            replyPtr = &risRet;
            replyLen = sizeof(risRet);
            break;

        case WSU_NS_RELEASE_RPS:
#if defined(EXTRA_DEBUG)
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"WSU_NS_RELEASE_RPS; rcvData.sender_port = %u\n",
                    rcvData.sender_port);
#endif
            shm_rsk_ptr->RISReqCnt.TerminateRadioComm++;
            risRet = releaseRPS(&cmdData.rps);
            replyPtr = &risRet;
            replyLen = sizeof(risRet);
            break;

        case WSU_NS_USER_SERVICE_REQ:
#if defined(EXTRA_DEBUG)
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"WSU_NS_USER_SERVICE_REQ; rcvData.sender_port = %u\n",
                    rcvData.sender_port);
#endif
            risRet = processUserServiceReq(&cmdData.userService);
            replyPtr = &risRet;
            replyLen = sizeof(risRet);
            break;

        case WSU_NS_PROVIDER_SERVICE_REQ:
#if defined(EXTRA_DEBUG)
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"WSU_NS_PROVIDER_SERVICE_REQ; rcvData.sender_port = %u\n",
                    rcvData.sender_port);
#endif
            risRet = processProviderServiceReq(&cmdData.providerService);
            replyPtr = &risRet;
            replyLen = sizeof(risRet);
            break;

        case WSU_NS_WSM_SERVICE_REQ:
#if defined(EXTRA_DEBUG)
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"WSU_NS_WSM_SERVICE_REQ; rcvData.sender_port = %u\n",
                    rcvData.sender_port);
#endif
            shm_rsk_ptr->RISReqCnt.WsmServiceReq++;
            risRet = wsmServiceReq(&cmdData.wsmService);
            replyPtr = &risRet;
            replyLen = sizeof(risRet);
            break;

        case WSU_NS_SEND_WSM_REQ:
#if defined(EXTRA_DEBUG)
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"WSU_NS_SEND_WSM_REQ; rcvData.sender_port = %u\n",
                    rcvData.sender_port);
#endif
            risRet = processSendWsmReq(&cmdData.outWsm);
            replyPtr = &risRet;
            replyLen = sizeof(risRet);
            break;

        case WSU_NS_GET_WBSS_STS:
#if defined(EXTRA_DEBUG)
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"WSU_NS_GET_WBSS_STS; rcvData.sender_port = %u\n",
                    rcvData.sender_port);
#endif
            replyLen = getWBSSInfo(strReply.str, sizeof(strReply.str));
            replyPtr = &strReply;
            replyLen += sizeof(strReply) - sizeof(strReply.str);
            strReply.risRet = RS_SUCCESS;
            break;

        case WSU_NS_GET_IND_PORT:
#if defined(EXTRA_DEBUG)
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"WSU_NS_GET_IND_PORT; rcvData.sender_port = %u\n",
                    rcvData.sender_port);
#endif
            shm_rsk_ptr->RISReqCnt.GetIndPort++;
            reply.getIndPort.risRet =
                getIndPort(cmdData.getIndPort.pid,
                           &reply.getIndPort.port);
            replyPtr = &reply.getIndPort;
            replyLen = sizeof(reply.getIndPort);
            break;

        case WSU_NS_GET_NS_CFG:
#if defined(EXTRA_DEBUG)
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"WSU_NS_GET_NS_CFG\n");
#endif
            cmdData.cfg.risRet = processGetNsCfg(&cmdData.cfg);
            replyPtr = &cmdData;
            replyLen = sizeof(cfgType);
            break;

        case WSU_NS_SET_NS_CFG:
#if defined(EXTRA_DEBUG)
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"WSU_NS_SET_NS_CFG\n");
#endif
            risRet = processSetNsCfg(&cmdData.cfg);
            replyPtr = &risRet;
            replyLen = sizeof(risRet);
            break;

        case WSU_NS_GET_CFG_INFO:
#if defined(EXTRA_DEBUG)
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"WSU_NS_GET_CFG_INFO; rcvData.sender_port = %u\n",
                    rcvData.sender_port);
#endif
            replyLen = getNSCfgInfo(strReply.str, sizeof(strReply.str));
            replyPtr = &strReply;
            replyLen += sizeof(strReply) - sizeof(strReply.str);
            strReply.risRet = RS_SUCCESS;
            break;

        case WSU_NS_SECURITY_VERIFY_WSM_REQ:
#if defined(EXTRA_DEBUG)
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"WSU_NS_SECURITY_VERIFY_WSM_REQ\n");
#endif
#ifndef NO_SECURITY
            reply.vodReply.risRet = alsmiVerifyThisOne(cmdData.vod.vodMsgSeqNum,
                                                       recvCmdSock,
                                                       rcvData.sender_port,
                                                       &lastVerifyRadioType);

            if (reply.vodReply.risRet == RS_SUCCESS) {
                /* If RS_SUCCESS, then alsmiVerifyThisOne() will reply to this
                 * command. Set replyPtr to NULL so that we don't reply to this
                 * command. */
                replyPtr = NULL;
            }
            else {
                /* If not RS_SUCCESS, then alsmiVerifyThisOne() will not reply
                 * to this command. Respond to this command ourselves. */
                shm_rsk_ptr->WSMError.WSMVerifyOnDemandError++;
                replyPtr = &reply.vodReply;
                replyLen = sizeof(reply.vodReply);
            }

#else
            reply.vodReply.risRet    = RS_SUCCESS;
            reply.vodReply.vodResult = WS_SUCCESS;
            replyPtr                 = &reply.vodReply;
            replyLen                 = sizeof(reply.vodReply);
#endif
            break;

        case WSU_NS_SECURITY_VERIFY_WSA_REQ:
#if defined(EXTRA_DEBUG)
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"WSU_NS_SECURITY_VERIFY_WSA_REQ\n");
#endif
#ifndef NO_SECURITY
            reply.vodReply.risRet = verifyWSA(cmdData.vod.vodMsgSeqNum,
                                              &reply.vodReply.vodResult);
            replyPtr = &reply.vodReply;
            replyLen = sizeof(reply.vodReply);
#else
            reply.vodReply.risRet    = RS_SUCCESS;
            reply.vodReply.vodResult = WS_SUCCESS;
            replyPtr                 = &reply.vodReply;
            replyLen                 = sizeof(reply.vodReply);
#endif
            break;

        case WSU_NS_SECURITY_INITIALIZE:
#if defined(EXTRA_DEBUG)
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"WSU_NS_SECURITY_INITIALIZE\n");
#endif
#ifndef NO_SECURITY

            if (alsmiRegister(cmdData.securityInit.debug,
                              cmdData.securityInit.vodEnabled,
                              cmdData.securityInit.vtp_milliseconds_per_verification,
                              cmdData.securityInit.smi_sign_results_options,
                              cmdData.securityInit.smi_sign_wsa_results_options,
                              cmdData.securityInit.smi_verify_wsa_results_options,
                              cmdData.securityInit.lcmName,
                              cmdData.securityInit.numSignResultsPSIDs,
                              cmdData.securityInit.signResultsPSIDs,
                              cmdData.securityInit.smi_sign_wsa_results_pid,
                              cmdData.securityInit.smi_verify_wsa_results_pid,
                              cmdData.securityInit.smi_cert_change_pid,
                              &reply.securityInit.regIdx) == 0) {
                vodEnabled = cmdData.securityInit.vodEnabled;
                reply.securityInit.risRet = RS_SUCCESS;
                smiInitialized = TRUE;
            }
            else {
                reply.securityInit.risRet = RS_ESECINITF;
            }

#else
            reply.securityInit.risRet = RS_SUCCESS;
#endif
            replyPtr = &reply.securityInit;
            replyLen = sizeof(reply.securityInit);
            break;

        case WSU_NS_SECURITY_TERMINATE:
#if defined(EXTRA_DEBUG)
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"WSU_NS_SECURITY_TERMINATE\n");
#endif
#ifndef NO_SECURITY
            reply.securityTerm.useCount =
                alsmiUnregister(cmdData.securityTerm.regIdx,
                                cmdData.securityTerm.sign_wsa_results,
                                cmdData.securityTerm.verify_wsa_results,
                                cmdData.securityTerm.cert_change_results);
            reply.securityTerm.risRet = (reply.securityTerm.useCount >= 0) ?
                                        RS_SUCCESS : RS_ESECTERMF;
            smiInitialized = FALSE;
#else
            reply.securityTerm.useCount = 0;
            reply.securityTerm.risRet   = RS_SUCCESS;
#endif
            replyPtr = &reply.securityTerm;
            replyLen = sizeof(reply.securityTerm);
            break;

        case WSU_NS_SECURITY_UPDATE_POSITION_AND_TIME:
#if defined(EXTRA_DEBUG)
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"WSU_NS_SECURITY_UPDATE_POSITION_AND_TIME\n");
#endif
#ifndef NO_SECURITY
            alsmiUpdatePositionAndTime(cmdData.updatePositionAndTime.latitude,
                                       cmdData.updatePositionAndTime.longitude,
                                       cmdData.updatePositionAndTime.elevation,
                                       cmdData.updatePositionAndTime.leapSecondsSince2004,
                                       cmdData.updatePositionAndTime.countryCode);
#endif
            risRet = RS_SUCCESS;
            replyPtr = &risRet;
            replyLen = sizeof(risRet);
            break;

        case WSU_NS_SECURITY_GET_LEAP_SECONDS_SINCE_2004:
#if defined(EXTRA_DEBUG)
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"WSU_NS_SECURITY_GET_LEAP_SECONDS_SINCE_2004\n");
#endif
#ifndef NO_SECURITY
            reply.getLeapSecondsSince2004.leapSeconds =
                alsmiGetLeapSecondsSince2004();
#else
            {
                struct timeval tv;

                gettimeofday(&tv, NULL);

                if (tv.tv_sec < 1483228800) { /* Jan 1, 2017, 00:00:00 UTC */
                    reply.getLeapSecondsSince2004.leapSeconds = 4;
                }
                else {
                    reply.getLeapSecondsSince2004.leapSeconds = 5;
                }
            }
#endif
            reply.getLeapSecondsSince2004.risRet = RS_SUCCESS;
            replyPtr = &reply.getLeapSecondsSince2004;
            replyLen = sizeof(reply.getLeapSecondsSince2004);
            break;

        case WSU_NS_SECURITY_SEND_CERT_CHANGE_REQ:
#if defined(EXTRA_DEBUG)
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"WSU_NS_SECURITY_SEND_CERT_CHANGE_REQ\n");
#endif
#ifndef NO_SECURITY
            risRet = (alsmiSendCertChangeReq() == 0) ? RS_SUCCESS :
                                                       RS_ESECCERTCHF;
#else
            risRet = RS_SUCCESS;
#endif
            replyPtr = &risRet;
            replyLen = sizeof(risRet);
            break;

        case WSU_NS_GET_AEROLINK_VERSION:
#if defined(EXTRA_DEBUG)
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"WSU_NS_GET_AEROLINK_VERSION\n");
#endif
#if !defined(NO_SECURITY)
            reply.getAerolinkVersion.risRet =
                processGetAerolinkVersion(reply.getAerolinkVersion.aerolinkVersion);
#else
            reply.getAerolinkVersion.risRet = 0;
#endif
            replyPtr = &reply.getAerolinkVersion;
            replyLen = sizeof(reply.getAerolinkVersion);
            break;

        case WSU_NS_GET_WSA_INFO:
#if defined(EXTRA_DEBUG)
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"WSU_NS_GET_WSA_INFO\n");
#endif
            replyLen = getWsaInfo(strReply.str, sizeof(strReply.str),
                                  cmdData.getWsaInfo.wsa_num);
            strReply.risRet = (replyLen == 0) ? RS_EGETWSAINFO : RS_SUCCESS;
            replyPtr = &strReply;
            replyLen += sizeof(strReply) - sizeof(strReply.str);
            break;

        case WSU_NS_GET_WSA_SERVICE_INFO:
#if defined(EXTRA_DEBUG)
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"WSU_NS_GET_WSA_SERVICE_INFO\n");
#endif
            replyLen = getWsaServiceInfo(
                strReply.str, sizeof(strReply.str),
                cmdData.getWsaInfoElement.wsa_num,
                cmdData.getWsaInfoElement.element_num
            );
            strReply.risRet = (replyLen == 0) ? RS_EGETWSAINFO : RS_SUCCESS;
            replyPtr = &strReply;
            replyLen += sizeof(strReply) - sizeof(strReply.str);
            break;

        case WSU_NS_GET_WSA_CHANNEL_INFO:
#if defined(EXTRA_DEBUG)
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"WSU_NS_GET_WSA_CHANNEL_INFO\n");
#endif
            replyLen = getWsaChannelInfo(
                strReply.str, sizeof(strReply.str),
                cmdData.getWsaInfoElement.wsa_num,
                cmdData.getWsaInfoElement.element_num
            );
            strReply.risRet = (replyLen == 0) ? RS_EGETWSAINFO : RS_SUCCESS;
            replyPtr = &strReply;
            replyLen += sizeof(strReply) - sizeof(strReply.str);
            break;

        case WSU_NS_GET_WSA_WRA_INFO:
#if defined(EXTRA_DEBUG)
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"WSU_NS_GET_WSA_WRA_INFO\n");
#endif
            replyLen = getWsaWraInfo(strReply.str, sizeof(strReply.str),
                                     cmdData.getWsaInfo.wsa_num);
            strReply.risRet = (replyLen == 0) ? RS_EGETWSAINFO : RS_SUCCESS;
            replyPtr = &strReply;
            replyLen += sizeof(strReply) - sizeof(strReply.str);
            break;

        case WSU_NS_USER_CHANGE_SERVICE_REQ:
#if defined(EXTRA_DEBUG)
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"WSU_NS_USER_CHANGE_SERVICE_REQ\n");
#endif
            shm_rsk_ptr->RISReqCnt.UserChangeServiceReq++;
            risRet = changeUserServiceReq(&cmdData.change);
            replyPtr = &risRet;
            replyLen = sizeof(risRet);
            break;

        case WSU_NS_TEST_CMD:
#if defined(EXTRA_DEBUG)
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"WSU_NS_TEST_CMD\n");
#endif
            risRet = processTestCmd(&cmdData.testCmd);
            replyPtr = &risRet;
            replyLen = sizeof(risRet);
            break;

        case WSU_NS_SEND_RAW_PACKET_REQ:
#if defined(EXTRA_DEBUG)
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"WSU_NS_SEND_RAW_PACKET_REQ\n");
#endif
            shm_rsk_ptr->RISReqCnt.SendRawPktReq[cmdData.raw.radioType]++;
            risRet = processSendRawPacketReq(&cmdData.raw);
            replyPtr = &risRet;
            replyLen = sizeof(risRet);
            break;

        case WSU_NS_GET_RADIO_TALLIES:
#if defined(EXTRA_DEBUG)
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"WSU_NS_GET_RADIO_TALLIES\n");
#endif
            reply.tallies.risRet =
                processGetRadioTallies(&cmdData.getRadioTallies,
                                       &reply.tallies);
            replyPtr = &reply.tallies;
            replyLen = sizeof(reply.tallies);
            break;

        case WSU_NS_SET_RADIO_CFG:
#if defined(EXTRA_DEBUG)
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"WSU_NS_SET_RADIO_CFG\n");
#endif

            /* This command is not supported for C-V2X */
            risRet = RS_SUCCESS;

            replyPtr = &risRet;
            replyLen = sizeof(risRet);
            break;

        case WSU_NS_PCAP_ENABLE:
#if defined(EXTRA_DEBUG)
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"WSU_NS_PCAP_ENABLE\n");
#endif
            pcap_enable(cmdData.pcapEnable.radioType,
                        cmdData.pcapEnable.radioNum,
                        cmdData.pcapEnable.mask);
            risRet = RS_SUCCESS;
            replyPtr = &risRet;
            replyLen = sizeof(risRet);
            break;

        case WSU_NS_PCAP_DISABLE:
#if defined(EXTRA_DEBUG)
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"WSU_NS_PCAP_DISABLE\n");
#endif
            pcap_disable(cmdData.pcapEnable.radioType,
                         cmdData.pcapEnable.radioNum);
            risRet = RS_SUCCESS;
            replyPtr = &risRet;
            replyLen = sizeof(risRet);
            break;

        case WSU_NS_PCAP_READ:
#if defined(EXTRA_DEBUG)
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"WSU_NS_PCAP_READ\n");
#endif
            reply.pcapReadReply.risRet = processPcapRead(&cmdData.pcapRead,
                                                         &reply.pcapReadReply);
            replyPtr = &reply.pcapReadReply;
            replyLen = sizeof(reply.pcapReadReply);
            break;
        case WSU_NS_GET_RSK_STATUS:
            if(0 != ns_get_cv2x_status(&reply.rskStatus)){
                memset(&reply.rskStatus,0x0,sizeof(reply.rskStatus));
            }
            replyPtr = &reply.rskStatus;
            replyLen = sizeof(reply.rskStatus);
            break;

        default:
            RSU_PRINTF_MAX_N_TIMES(10, "Unknown rsIoctlType (%d)\n",
                                   cmdData.cmd);
            risRet = RS_EINVID;
            replyPtr = &risRet;
            replyLen = sizeof(risRet);
            break;
        } // switch (recvData.indType)

        /* Send back the reply if replyPtr was not set to NULL */
        if (replyPtr != NULL) {
            if (!wsuSendData(recvCmdSock, rcvData.sender_port, replyPtr,
                             replyLen)) {
                /* Send failed */
                RSU_PRINTF_MAX_N_TIMES(10, "wsuSendData() failed\n");
            }
        }
    } // while (radioServicesReceiveCommandsThreadRunning)

radioServicesReceiveCommandsThreadExit:
    radioServicesReceiveCommandsThreadRunning = FALSE;

    if (recvCmdSock != -1) {
        close(recvCmdSock);
        recvCmdSock = -1;
    }

    shm_rsk_ptr->bootupState &= ~0x40; // radioServicesReceiveCommandsThread running
#if defined(EXTRA_DEBUG)
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Exiting\n");
#endif
    pthread_exit(NULL);
    return NULL;
}

/**
** Function:  createReceiveCommandsThread
** @brief  Creates the thread to receive incoming commands to Radio
**         Services.
** @return RS_SUCCESS for success. Error code for failure.
**
** Details: The thread to receive incoming commands to Radio Services is
**          spawned. A console message is printed on an error spawning the
**          thread.
**/
rsResultCodeType createReceiveCommandsThread(void)
{
    int       result;

    /* Make sure that multiple threads are not created - We need only one */
    if (!radioServicesReceiveCommandsThreadRunning) {
#if defined(EXTRA_DEBUG)
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Calling pthread_create(&radioServicesReceiveCommandsThreadId, NULL, &radioServicesReceiveCommandsThread, NULL)\n");
#endif
        result = pthread_create(&radioServicesReceiveCommandsThreadId, NULL,
                                &radioServicesReceiveCommandsThread, NULL);

        if (result != 0) {
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"radioServicesReceiveCommandsThread creation failed: %s\n",
                    strerror(result));
            return RS_ETHREAD;
        }
    }
#ifdef RSU_DEBUG
    else {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"radioServicesReceiveCommandsThread already running\n");
    }
#endif // RSU_DEBUG

    return RS_SUCCESS;
}

/**
** Function:  terminateReceiveCommandsThread
** @brief  Terminates the thread to receive incoming commands to Radio
**         Services.
** @return RS_SUCCESS for success. Error code for failure.
**
** Details: The "running" variable is set to FALSE, and the SIGTERM signal is
**          sent to the thread. Then a pthread_join() is done to wait for the
**          thread to terminate.
**/
rsResultCodeType terminateReceiveCommandsThread(void)
{
    int result;
    int tmpSock;

    if (radioServicesReceiveCommandsThreadRunning) {
        radioServicesReceiveCommandsThreadRunning = FALSE;

        /* Close the socket that radioServicesReceiveCommandsThread is reading
         * from. This will force the recvfrom() to abort with an error. The
         * thread, seeing that radioServicesReceiveCommandsThreadRunning is now
         * FALSE, will terminate execution. */
        if (recvCmdSock != -1) {
            tmpSock = recvCmdSock;
            recvCmdSock = -1;
            close(tmpSock);
        }

        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Calling pthread_kill(radioServicesReceiveCommandsThreadId, SIGUSR1)\n");
        result = pthread_kill(radioServicesReceiveCommandsThreadId, SIGUSR1);

        if (result != 0) {
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Error terminating radioServicesReceiveCommandsThread: %s\n",
                       strerror(result));
        }
        else {
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Calling pthread_join(radioServicesReceiveCommandsThreadId, NULL)\n");
            result = pthread_join(radioServicesReceiveCommandsThreadId, NULL);

            if (result != 0) {
                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"pthread_join() failed: %s\n", strerror(result));
            }
#ifdef RSU_DEBUG
            else {
                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"pthread_join() succeeded\n");
            }
#endif
        }
    }
    return RS_SUCCESS;
}

