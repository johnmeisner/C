#ifndef TPS_MSG_H
#define TPS_MSG_H
/*---------------------------------------------------------------------------*/
/*                                                                           */
/* Filename: tps_msg.h                                                       */
/*                                                                           */
/* Copyright (C) 2019 DENSO International America, Inc.                      */
/*                                                                           */
/* Description: Define message types used by the TPS API library             */
/*                                                                           */
/*---------------------------------------------------------------------------*/



#include "dn_types.h"
#include "v2x_common.h"

/* TPS Message ID Types */
#define TPS_DATA_REG        TPS_BASE_MSG_ID
#define TPS_DATA_DEREG      TPS_BASE_MSG_ID+1
#define TPS_DATA_IND        TPS_BASE_MSG_ID+2
#define TPS_DATA_REQ        TPS_BASE_MSG_ID+3
#define TPS_DATA_CFM        TPS_BASE_MSG_ID+4
#define TPS_GPS_OUTPUT_REQ  TPS_BASE_MSG_ID+5
#define TPS_GPS_OUTPUT_CFM  TPS_BASE_MSG_ID+6
#define TPS_DEBUG_OUTPUT    TPS_BASE_MSG_ID+7
#define TPS_RTCM_IND        TPS_BASE_MSG_ID+8
#define TPS_ERROR_CODE      TPS_BASE_MSG_ID+9
#define TPS_RAW_MSG_REQ     TPS_BASE_MSG_ID+10
#define TPS_RAW_MSG_IND     TPS_BASE_MSG_ID+11
#define TPS_RAW_MSG_DEREG   TPS_BASE_MSG_ID+12
#define TPS_GPS_OUTPUT_MASK_REQ   TPS_BASE_MSG_ID+13
#define TPS_GPS_OUTPUT_MASK_IND   TPS_BASE_MSG_ID+14
#define TPS_WHEEL_TICK      TPS_BASE_MSG_ID+15
#define TPS_COLD_START      TPS_BASE_MSG_ID+16 
#define TPS_STOP_GNSS       TPS_BASE_MSG_ID+17 
#define TPS_DUMP_SPI_STATS  TPS_BASE_MSG_ID+18
#define TPS_GNSS_HW_STAT    TPS_BASE_MSG_ID+19 
#define TPS_DUMP_UBLOX_CFG  TPS_BASE_MSG_ID+20 
#define TPS_SET_UBLOX_CFG   TPS_BASE_MSG_ID+21 

/* Generic Message Buffer Type */
/////#define MAX_SEND_SIZE 512
#define MAX_SEND_SIZE 1600

/* Message buffer for the message queue */
typedef struct {
    msgHeader_t msgHdr;
    uint32_t    msgLen;                  /* Size of tps message in mtext buffer */
    char_t      mtext[MAX_SEND_SIZE];    /* GPS Data */
} tpsMsgBufType;


#define MAX_RAW_MSG_HDR_LEN 20
#define MAX_RAW_MSG_LEN 400

/* Structure for requesting RAW msg data from TPS by TPS API */
typedef struct {
    uint32_t hdrLen; 
    char_t   rawMsgHeader[MAX_RAW_MSG_HDR_LEN];
    int32_t  msgUpdateIntvl; 
} tpsRawMsgReq;

/* Response of RAW msg data from TPS to TPS API */
typedef struct {
    uint32_t msgFilterIdx;
    uint32_t msg_length;
    char_t   msgbuf[MAX_RAW_MSG_LEN];
} tpsRawMsgRes;

#endif

