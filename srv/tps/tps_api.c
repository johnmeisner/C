/*---------------------------------------------------------------------------*/
/*                                                                           */
/*    Filename: tps_api.c                                                    */
/*     Purpose: Library of functions to communicate with the TPS process     */
/*  Department: DENSO North America Research & Development                   */
/*                                                                           */
/* Copyright (C) 2015 DENSO International America, Inc.                      */
/*                                                                           */
/*     Project: V2X Special Projects                                         */
/* Description: Contains functions to communicate with the TPS process       */
/*                                                                           */
/*---------------------------------------------------------------------------*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>   // for usleep
#include <time.h>
#include <syslog.h>
#include <errno.h>
#include "dn_types.h"
#include "wsu_util.h"
#include "tps_msg.h"
#include "tps_types.h"
#include "ipcsock.h"
#include "v2x_common.h"

/* Global variables */
int32_t          tpsSockFd;
static pid_t     appPid;
static pthread_t tpsMsgReadThread;
static int32_t   tpsThreadAlive;
static bool_t    threadReadHung;

void (*tpsRegisterCB)(tpsDataType *);
void (*tpsRequestCB)(tpsDataType *);
void (*errorReportCB)(tpsResultCodeType);
void (*tpsRegisterRTCMCB)(char_t *, int32_t);
void (*tpsRegisterRawMsgCB)(char_t *, int32_t);
void (*tpsGpsOutputMaskIndCB)(uint32_t);


/*
 * @brief Converts a tpsResultCodeType to a string
 */
char *tpsResultCodeType2Str(tpsResultCodeType result)
{
    static char buffer[40];

    switch(result)
    {
       case TPS_SUCCESS:
           return "TPS_SUCCESS";
       case TPS_FAILURE:
           return "TPS_FAILURE";
       case TPS_CONN_LOST:
           return "TPS_CONN_LOST";
       case TPS_REG_LIST_FULL:
           return "TPS_REG_LIST_FULL";
       case GPS_WRITE_ERROR:
           return "GPS_WRITE_ERROR";
       case GPS_READ_ERROR_EXIT:
           return "GPS_READ_ERROR_EXIT";
       case GPS_NMEA_CKSUM_ERROR:
           return "GPS_NMEA_CKSUM_ERROR";
       case GPS_NMEA_MSG_PARSE_ERROR:
           return "GPS_NMEA_MSG_PARSE_ERROR";
       case GPS_NO_NMEA_OUTPUT:
           return "GPS_NO_NMEA_OUTPUT";
       case GPS_NO_PPS_OUTPUT:
           return "GPS_NO_PPS_OUTPUT";
       case GPS_SUCCESS_PPS_OUTPUT:
           return "GPS_SUCCESS_PPS_OUTPUT";
       case GPS_UBX_CKSUM_ERROR:
           return "GPS_UBX_CKSUM_ERROR";
       default:
           snprintf(buffer, 40, "Unknown tpsResultCodeType (%d)", result);
           return buffer;
    }
}

/*
 * @brief Sends a request message to the TPS process
 */
bool_t tpsApiSendMsg(int32_t msgId)
{
    tpsMsgBufType  msg;
    int32_t        len;

    /* Send a TPS Data message to TPS. */
    msg.msgHdr.msgId = msgId;
    msg.msgHdr.appPid = appPid;
    msg.msgLen = 0;

    len = sizeof(msg.msgHdr) + sizeof(msg.msgLen);
    return wsuSendData(tpsSockFd, TPS_RECV_DATAREQ_PORT, &msg, len);
}

/**
 * @brief Wait to receive a specific message-type message
 * @note There is no timeout time. You will have to program this behavior yourself with an alarm timer.
 */
bool_t tpsApiRecvMsg(int32_t want_msgId, int32_t give_up_msg_point, void *rcvbuf, size_t rcvbuf_size)
{
    tpsMsgBufType msg;
    RcvDataType   rcv_data;
    int32_t       cnt_msgs_dropped_while_waiting_for_msgId = 0;
    int32_t       rc;

    rcv_data.fd = tpsSockFd;

    do {
        rc = wsuReceiveData(0, &rcv_data);
        if (rc < 0)
        {
            V2X_LOG(LOG_ERR,"Socket Error: wsuReceiveData() rtn=%d (<0), errno=%d[%s] fd=%d",rc,errno,strerror(errno),rcv_data.fd);
            return FALSE;
        }
#ifdef DEBUG_SOCKET_EINTR
        if (rc == 0)
            V2X_LOG(LOG_ERR,"Socket Error: wsuReceiveData() rtn=%d (==0), errno=%d[%s] fd=%d",rc,errno,strerror(errno),rcv_data.fd);
#endif /*DEBUG_SOCKET_EINTR*/

        if (rcv_data.available == TRUE) {
            if (rcv_data.size >= sizeof(msg.msgHdr)) {
                memcpy(&msg.msgHdr, rcv_data.data, sizeof(msg.msgHdr));
                // Did we get the one we wanted?
                if (msg.msgHdr.msgId == want_msgId) {
                    if (rcv_data.size <= sizeof(msg)) {
                        memcpy(&msg, rcv_data.data, sizeof(msg));
                        // Now copy data in msg into caller's rcvbuf
                        if (msg.msgLen <= rcvbuf_size) {
                            memcpy(rcvbuf, msg.mtext, msg.msgLen);
                            return TRUE;
                        }
                        V2X_LOG(LOG_ERR, "WARNING: tpsApiRecvMsg wanted and got msgid %d, but its data was too big for caller's buffer! (msgLen=%ul, rcvbuf max=%ld)", want_msgId, msg.msgLen, rcvbuf_size);
                        return FALSE;
                    }
                    V2X_LOG(LOG_ERR, "WARNING: tpsApiRecvMsg wanted and got msgid %d, but its data was too big for the msg structure (rcv_data.size=%ld, sizeof(msg)=%ld)", want_msgId, rcv_data.size, sizeof(msg));
                    return FALSE;
                // We got somebody else .. drop it and keep trying
                } else {
                    ++cnt_msgs_dropped_while_waiting_for_msgId;
                    V2X_LOG(LOG_ERR, "WARNING: tpsApiRecvMsg: Dropping message type %d while waiting for msgid %d", msg.msgHdr.msgId, want_msgId);
                }
            // We got something small - drop it but count it
            } else {
                ++cnt_msgs_dropped_while_waiting_for_msgId;
                V2X_LOG(LOG_ERR, "WARNING: tpsApiRecvMsg: Dropping a small message - not a tpsMsgBufType msg");
            }
        }
    } while (cnt_msgs_dropped_while_waiting_for_msgId < give_up_msg_point);

    return FALSE;
}

static void *tpsApiMsgReadThread(void *arg)
{
    //tpsMsgBufType  qbuf
    //int32_t        ret;
    int32_t        rcvd_len;
    tpsDataType    tpsUserData;    /* This is the TPS User Data structure */
    tpsMsgBufType  buf;
    tpsRawMsgRes   *raw_data;
    RcvDataType    rcv_data = {
        FALSE, tpsSockFd, &buf, sizeof(buf)
    };

    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);

    tpsThreadAlive = 1;
    while (tpsThreadAlive)
    {
        threadReadHung = TRUE;
        rcvd_len       = wsuReceiveData(0, &rcv_data);
        threadReadHung = FALSE;
        //if ((rcvd_len = wsuReceiveData(SOCKRECV_TIMEOUT, &rcv_data)) == -1)
        if (tpsThreadAlive == 0)
        {
            break;
        }
        if (rcvd_len == -1)
        {
            V2X_LOG(LOG_ERR,"Socket Error: wsuReceiveData() rtn=%d (<0), errno=%d[%s] fd=%d",rcvd_len,errno,strerror(errno),rcv_data.fd);

            ////TODO: Probably should not close socket since it is probably OK (see KNOTE in tps_api.c)

            /* Socket error occurred or connection is closed.
               Exit from this thread.
               Note: In the future release, can notify the application that
               the service is shut down at this point. */

            close(tpsSockFd);
            tpsSockFd = 0;

            if (errorReportCB)
            {
                errorReportCB(TPS_CONN_LOST);
            }

            /* End this thread */
            tpsThreadAlive = 0;
            break;
        }
#ifdef DEBUG_SOCKET_EINTR
        if (rcvd_len == 0)
            V2X_LOG(LOG_WARN,"Socket Error: wsuReceiveData() rtn=%d (==0), errno=%d[%s] fd=%d",rcvd_len,errno,strerror(errno),rcv_data.fd);
#endif /*DEBUG_SOCKET_EINTR*/


        if (rcv_data.available)
        {
            switch (buf.msgHdr.msgId)
            {
            case TPS_DATA_IND:
                /* TPS Data Indication for Register */
                //V2X_LOG(LOG_ERR, "Indication received\n");
                memcpy(&tpsUserData, buf.mtext, sizeof(tpsUserData));
                /* Call the call back function here */
                tpsRegisterCB(&tpsUserData);

                break;

            case TPS_DATA_CFM:
                /* TPS Data Confirmation for Request */
                //V2X_LOG(LOG_ERR, "Confirmation received\n");
                memcpy(&tpsUserData, buf.mtext, sizeof(tpsUserData));
                /* Call the call back function here */
                tpsRequestCB(&tpsUserData);

                break;

            case TPS_GPS_OUTPUT_CFM:    // Change this to error report later
                /* TPS GPS Corrections confirmation Request */
                //memcpy((int32_t *)&gpsCrtnRet,(int32_t *)buf.mtext, sizeof(int32_t));
                //sem_post(&sem_GpsCrtn);
                break;

            case TPS_RTCM_IND:
                if (tpsRegisterRTCMCB)
                {
                    tpsRegisterRTCMCB(buf.mtext, buf.msgLen);
                }
                break;

            case TPS_RAW_MSG_IND:
                if (tpsRegisterRawMsgCB)
                {
                    raw_data = (tpsRawMsgRes *)buf.mtext;
                    tpsRegisterRawMsgCB(raw_data->msgbuf, raw_data->msg_length);
                }
                break;

            case TPS_ERROR_CODE:

                if (errorReportCB)
                {
                    errorReportCB(buf.msgHdr.errorCode);
                }
                break;

            case TPS_GPS_OUTPUT_MASK_IND:
                if (tpsGpsOutputMaskIndCB)
                {
                    uint32_t mask;
                    memcpy(&mask, buf.mtext, sizeof(mask));
                    tpsGpsOutputMaskIndCB(mask);
                }
                break;

            default:
                V2X_LOG(LOG_DEBUG, "KDEBUG: tps_api.c::tpsApiMsgReadThread(): Received and ignoring msg of type %d (0x%x)", buf.msgHdr.msgId, buf.msgHdr.msgId);
                break;
            }
        }
    }
    return 0;
}

/* Returns 0 for success, -1 for failure */
//tpsResultCodeType wsuTpsInit(void (*errReportCB)(tpsResultCodeType))
tpsResultCodeType wsuTpsInit(void)
{
    threadReadHung      = FALSE;
    errorReportCB       = NULL;
    tpsRegisterRTCMCB   = NULL;
    tpsRegisterRawMsgCB = NULL;
    appPid              = getpid();
    tpsSockFd           = wsuConnectSocket(0);

    if (pthread_create(&tpsMsgReadThread, NULL, tpsApiMsgReadThread, NULL) != 0)
    {
        /* Error: thread failed */
        return TPS_FAILURE;
    }

    return TPS_SUCCESS;
}

/* Returns 0 on success, -1 on failure */
tpsResultCodeType wsuTpsRegister(void (*registerCB)(tpsDataType *))
{
    //V2X_LOG(LOG_ERR, "Register for TPS Data Indications\n");
    tpsRegisterCB = registerCB;

    /* Send a TPS Data Register message to TPS. */
    if ((tpsApiSendMsg(TPS_DATA_REG)) == FALSE)
    {
        V2X_LOG(LOG_ERR, "send Register Message failed");
        return TPS_FAILURE;
    }

    return TPS_SUCCESS;
}

/* Returns 0 on success, -1 on failure */
tpsResultCodeType wsuTpsDeregister(void)
{
    //V2X_LOG(LOG_ERR, "Deregister for TPS Data Indications\n");

    /* Send a TPS Data Deregister message to TPS. */
    if ((tpsApiSendMsg(TPS_DATA_DEREG)) == FALSE)
    {
        V2X_LOG(LOG_ERR, "send Deregister Message failed");
        return TPS_FAILURE;
    }

    //V2X_LOG(LOG_ERR, "Deregister successful\n");
    return TPS_SUCCESS;
}

/* Returns 0 on success, -1 on failure */
tpsResultCodeType wsuTpsRequest(void (*requestCB)(tpsDataType *))
{
    //V2X_LOG(LOG_ERR, "Request TPS Data\n");

    tpsRequestCB = requestCB;

    /* Send a TPS Data Request message to TPS. */
    if ((tpsApiSendMsg(TPS_DATA_REQ)) == FALSE)
    {
        V2X_LOG(LOG_ERR, "send Request Message failed");
        return TPS_FAILURE;
    }

    return TPS_SUCCESS;
}

/* Clean up all the resources */
void wsuTpsTerm(void)
{
    //int8_t   command[136];

#if 0
    /* Delete the file that was created to create a message queue for
       this application. */
    strcpy(command, "rm -f ");
    strcat(command, appPath);
    //V2X_LOG(LOG_ERR, "cmd: %s\n", command);
    system(command);

    /* Remove this app's message queue */
    if (msgctl(tpsUserAppQid, IPC_RMID, NULL) != 0)
    {
        V2X_LOG(LOG_ERR, "msgctl: remove mqueue");
    }
#endif

    tpsThreadAlive = 0;

    /* Wait for the read thread to finish reading */
    if (threadReadHung == TRUE)
    {
        usleep(1000);
    }

    /* If still hangs from reading, cancel the thread. */
    if (threadReadHung == TRUE)
    {
        pthread_cancel(tpsMsgReadThread);
        //V2X_LOG(LOG_ERR, "* Dereg: CAN thread 1 cancelled.\n");
    }
    else
    {
        pthread_join(tpsMsgReadThread, NULL);
        //V2X_LOG(LOG_ERR, "CAN thread 1 joined.\n");
    }

    //pthread_join(tpsMsgReadThread, NULL);
    close(tpsSockFd);
    //sem_destroy(&sem_GpsCrtn);

    //strcpy(command, "ipcrm -q ");
    //itoa(tpsUserAppQid, command + strlen(command), 10);
    //V2X_LOG(LOG_ERR, "cmd: %s\n", command);
    //system(command);
}

int32_t wsuTpsSndGpsOutput(char_t *ubuf, int32_t dlen)
{
    tpsMsgBufType  msg;
    int32_t        len;

    if (dlen > 500 ||  dlen < 1)
    {
        return TPS_FAILURE;
    }

    msg.msgHdr.msgId = TPS_GPS_OUTPUT_REQ;
    msg.msgHdr.appPid = appPid;
    msg.msgLen = dlen;

    memcpy(msg.mtext, ubuf, dlen);
    len = dlen + sizeof(msg.msgHdr) + sizeof(msg.msgLen);

    if ((wsuSendData(tpsSockFd, TPS_RECV_DATAREQ_PORT, &msg, len)) == FALSE)
    {
        V2X_LOG(LOG_ERR, "wsuSendData failed while sending GPS correction to TPS");
        return TPS_FAILURE;
    }

    //sem_wait(&sem_GpsCrtn);
    //return gpsCrtnRet;
    return TPS_SUCCESS;
}
#if defined(ENABLE_WHEELTIX)
int32_t wsuTpsSendWheelTicks(WheelTickData_t WheelTickData)
{
    tpsWheelTickMsg_t   msg;
    int32_t             len;
    struct timespec     ts;

    msg.msgHdr.msgId = TPS_WHEEL_TICK;
    msg.msgHdr.appPid = appPid;
    msg.msgLen = sizeof(WheelTickData_t);
    strncpy(msg.appAbbrev, TPS, APP_ABBREV_SIZE);

    memcpy((uint8_t *)&msg.tick_data, (uint8_t *)&WheelTickData, sizeof(WheelTickData_t));

    // NOTE: This calculation is done to convert time tag to ms. If u-blox config changes
    //       this value may need to be changed.
    // Get current systime of the API call
    clock_gettime(CLOCK_MONOTONIC, &ts);
    msg.tick_data.timeTag = ts.tv_sec * 1000 + ts.tv_nsec / 1e6;

    len =  sizeof(tpsWheelTickMsg_t) + sizeof(msgHeader_t);

    if ((wsuSendData(tpsSockFd, TPS_RECV_DATAREQ_PORT, &msg, len)) == FALSE)
    {
        V2X_LOG(LOG_ERR, "wsuSendData failed while sending WHEEL TICK to TPS");
        return TPS_FAILURE;
    }

    return TPS_SUCCESS;
}

#endif /* ENABLE_WHEELTIX */

void  wsuTpsShowDebugMsg(uint32_t OutputType)
{
    tpsMsgBufType  msg;
    int32_t        len;

    msg.msgHdr.msgId = TPS_DEBUG_OUTPUT;
    //msg.msgHdr.srcProcQHndl = tpsUserAppQid;

    //V2X_LOG(LOG_ERR, "DBG Output Type: %x\n", OutputType);

    memcpy((uint32_t *)msg.mtext, (uint32_t *)&OutputType, len=sizeof(uint32_t));
    msg.msgLen = len;
    len = len + sizeof(msg.msgHdr) + sizeof(msg.msgLen);

    if ((wsuSendData(tpsSockFd, TPS_RECV_DATAREQ_PORT, &msg, len)) == FALSE)
    {
        V2X_LOG(LOG_ERR, "wsuSendData for \"Debug Output\" failed");
    }
}

void wsuTpsRegisterErrorHandler(void (*errHandlerCB)(tpsResultCodeType))
{
    errorReportCB = errHandlerCB;
}

void wsuTpsRegisterRTCMHandler(void (*handleRTCMCB)(char_t *, int32_t))
{
    tpsRegisterRTCMCB = handleRTCMCB;
}

int32_t wsuTpsRegisterRawMsgHandler(void (*handleRawMsgCB)(char_t *, int32_t), char_t msgHeader[], int32_t raw_hdrlen, int32_t msgUpdIntvl)
{
    tpsMsgBufType  msg;
    tpsRawMsgReq   raw_req = {0};
    int32_t        len;

    if ((msgHeader == NULL) || (raw_hdrlen >= MAX_RAW_MSG_HDR_LEN) || (raw_hdrlen < 1))
    {
        return TPS_FAILURE;
    }

    raw_req.hdrLen = raw_hdrlen;
    raw_req.msgUpdateIntvl = msgUpdIntvl;
    memcpy(raw_req.rawMsgHeader, msgHeader, raw_hdrlen);

    msg.msgHdr.msgId = TPS_RAW_MSG_REQ;
    msg.msgHdr.appPid = appPid;
    msg.msgLen = sizeof(tpsRawMsgReq);

    memcpy((int8_t *)msg.mtext, (int8_t *)&raw_req, msg.msgLen);
    len = msg.msgLen + sizeof(msg.msgHdr) + sizeof(msg.msgLen);

    if (tpsRegisterRawMsgCB == NULL)
        tpsRegisterRawMsgCB = handleRawMsgCB;

    if ((wsuSendData(tpsSockFd, TPS_RECV_DATAREQ_PORT, &msg, len)) == FALSE)
    {
        V2X_LOG(LOG_ERR, "wsuSendData failed while registering request for RAW data from TPS");
        return TPS_FAILURE;
    }
    return TPS_SUCCESS;

}

int32_t wsuTpsDeregisterRawMsgHandler(char_t msgHeader[],int32_t raw_hdrlen)
{
    tpsMsgBufType  msg;
    tpsRawMsgReq   raw_req = {0};
    int32_t        len;

    if ((msgHeader == NULL) || (raw_hdrlen >= MAX_RAW_MSG_HDR_LEN) || (raw_hdrlen < 1))
    {
        return TPS_FAILURE;
    }

    raw_req.hdrLen = raw_hdrlen;
    memcpy(raw_req.rawMsgHeader, msgHeader, raw_hdrlen);

    msg.msgHdr.msgId = TPS_RAW_MSG_DEREG;
    msg.msgHdr.appPid = appPid;
    msg.msgLen = sizeof(tpsRawMsgReq);

    memcpy((int8_t *)msg.mtext, (int8_t *)&raw_req, msg.msgLen);
    len = msg.msgLen + sizeof(msg.msgHdr) + sizeof(msg.msgLen);

    if ((wsuSendData(tpsSockFd, TPS_RECV_DATAREQ_PORT, &msg, len)) == FALSE)
    {
        V2X_LOG(LOG_ERR, "wsuSendData failed while deregistering RAW data from TPS");
        return TPS_FAILURE;
    }
    return TPS_SUCCESS;
}


void wsuTpsRegisterDebugOutputMaskHandler(void (*handlerCB)(uint32_t))
{
    tpsGpsOutputMaskIndCB = handlerCB;
}


static int16_t  flagDebugOutputMaskUpdated = 0;
static uint32_t curDebugOutputMaskValue = 0;

void getDebugOutputMaskHelper(uint32_t newMask)
{
    flagDebugOutputMaskUpdated = 1;
    curDebugOutputMaskValue = newMask;
}

#define MAX_MASK_WAIT_IN_TENTH_SECONDS 50
uint32_t wsuTpsGetDebugOutputMask()
{
    int32_t i;

    wsuTpsRegisterDebugOutputMaskHandler(getDebugOutputMaskHelper);
    flagDebugOutputMaskUpdated = 0;

    if (tpsApiSendMsg(TPS_GPS_OUTPUT_MASK_REQ) == FALSE)
    {
        V2X_LOG(LOG_ERR, "Failed to request tps Debug Ouput Mask\n");
        wsuTpsRegisterDebugOutputMaskHandler(NULL);
        return 0;
    }

    // Now wait for response
    for (i=0; i<MAX_MASK_WAIT_IN_TENTH_SECONDS; i++) {
        if (flagDebugOutputMaskUpdated) {
            wsuTpsRegisterDebugOutputMaskHandler(NULL);
            return curDebugOutputMaskValue;
        }
        usleep(100000);  // 1/10 sec
    }
    wsuTpsRegisterDebugOutputMaskHandler(NULL);
    return 0;
}

void wsuTpsDumpGNSSHWstatus(void)
{
    tpsMsgBufType  msg;
    uint32_t         len;
    len = 0;
    memset(&msg,0x0,sizeof(msg));
    msg.msgHdr.msgId = TPS_GNSS_HW_STAT;
    len = len + sizeof(msg.msgHdr) + sizeof(msg.msgLen);

    if ((wsuSendData(tpsSockFd, TPS_RECV_DATAREQ_PORT, &msg, len)) == FALSE)
    {
        V2X_LOG(LOG_ERR, "wsuSendData for \"GNSS HW STATUS\" failed");
    }
}


void  wsuTpsColdStartuBlox(void)
{
    tpsMsgBufType  msg;
    uint32_t         len;
    len = 0;
    memset(&msg,0x0,sizeof(msg));
    msg.msgHdr.msgId = TPS_COLD_START;
    len = len + sizeof(msg.msgHdr) + sizeof(msg.msgLen);

    if ((wsuSendData(tpsSockFd, TPS_RECV_DATAREQ_PORT, &msg, len)) == FALSE)
    {
        V2X_LOG(LOG_ERR, "wsuSendData for \"GNSS Cold(factory) Start\" failed");
    }
}

void  wsuTpsStopuBlox(void)
{
    tpsMsgBufType  msg;
    uint32_t         len;
    len = 0;
    memset(&msg,0x0,sizeof(msg));
    msg.msgHdr.msgId = TPS_STOP_GNSS;
    len = len + sizeof(msg.msgHdr) + sizeof(msg.msgLen);

    if ((wsuSendData(tpsSockFd, TPS_RECV_DATAREQ_PORT, &msg, len)) == FALSE)
    {
      V2X_LOG(LOG_ERR, "wsuSendData for \"STOP GNSS\" failed");
    }
}

void wsuTpsDumpSPIstats(void)
{
    tpsMsgBufType  msg;
    uint32_t         len;
    len = 0;
    memset(&msg,0x0,sizeof(msg));
    msg.msgHdr.msgId = TPS_DUMP_SPI_STATS;
    len = len + sizeof(msg.msgHdr) + sizeof(msg.msgLen);

    if ((wsuSendData(tpsSockFd, TPS_RECV_DATAREQ_PORT, &msg, len)) == FALSE)
    {
      V2X_LOG(LOG_ERR, "wsuSendData for \"SPI STATS DUMP\" failed");
    }

}

/**
 * Dump Ublox CFG (UBX-MON-VER).
 */
void wsuTpsDumpUbloxConfigs(void)
{
    tpsMsgBufType  msg;
    uint32_t         len;
    len = 0;
    memset(&msg,0x0,sizeof(msg));
    msg.msgHdr.msgId = TPS_DUMP_UBLOX_CFG;
    len = len + sizeof(msg.msgHdr) + sizeof(msg.msgLen);

    if ((wsuSendData(tpsSockFd, TPS_RECV_DATAREQ_PORT, &msg, len)) == FALSE)
    {
      V2X_LOG(LOG_ERR, "wsuSendData for \"TPS_DUMP_UBLOX_CFG\" failed");
    }

}

/**
 * Set Ublox CFG (UBX-XXX-YYY).
 */
void wsuTpsSendDefaultUbloxCfgs(void)
{
    tpsMsgBufType  msg;
    uint32_t         len;
    len = 0;
    memset(&msg,0x0,sizeof(msg));
    msg.msgHdr.msgId = TPS_SET_UBLOX_CFG ;
    len = len + sizeof(msg.msgHdr) + sizeof(msg.msgLen);

    if ((wsuSendData(tpsSockFd, TPS_RECV_DATAREQ_PORT, &msg, len)) == FALSE)
    {
      V2X_LOG(LOG_ERR, "wsuSendData for \"TPS_SET_UBLOX_CFG\" failed");
    }

}

