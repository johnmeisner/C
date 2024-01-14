/****************************************************************************
 *                                                                          *
 *  File Name: i2v_srmrx16_data.h                                           *
 *  Author:                                                                 *
 *      DENSO International America, Inc.                                   *
 *      North America Research Laboratory, California Office                *
 *      3252 Business Park Drive                                            *
 *      Vista, CA 92081                                                     *
 *                                                                          *
 ****************************************************************************/

#ifndef I2V_SRMRX16_DATA_H
#define I2V_SRMRX16_DATA_H

#include "i2v_general.h"

#define SRM_BUF_SIZE         (2048)

/*
 * GridSmart format
 */

typedef struct {
    uint8_t  rsuID[6];     /* RSU ID using Eth address */
    uint32_t seqNum;
    uint8_t  msgVersion;   /* Msg version */
    wtime   timeStamp;    /* System TimeStamp */
    uint8_t  msgType;
    
    uint32_t msgTimeStamp;   /* Message time stamp - elapsed minute of the year */
    uint32_t msgSecond;      /* Msg micro seconds  */
    uint8_t  msgCount;       /* Msg Count */
    uint16_t intID;          /* Intersection ID */
    uint8_t  reqID;          /* Request ID */
    uint8_t  priReqType;     /* Priority request Type */
    uint8_t  inBoundLaneID;  /* In bound lane ID */
    uint32_t etaMinute;      /* Estimate time of arrival in minute of the year */
    uint16_t etaSeconds;     /* Estimate time of arrival in miliseconds */
    uint16_t etaDuration;    /* Estimate time of arrival duration in miliseconds */
    uint32_t vehID;          /* Requestor vehicle ID */
    uint32_t stationID;      /* Requestor vehicle station ID */
    uint32_t vehRole;        /* Requestor vehicle role type */
    int32_t  vehLatitude;
    int32_t  vehLongitude;
    int32_t  vehElevation;
    float64_t  vehHeading;     /* Heading Angle not Degress */
    float64_t  vehSpeed;       /* Metre per second */
    uint8_t  vehTransmisson; /* DSRC.H: TransmissionState_Root*/
} __attribute__((packed)) SRMMsgType;

/* 
 * Shared srm <--> scs pipe 
 */

typedef struct {
    uint8_t         srmVTPVerifyStatus;
    uint8_t         srmVTPVerifyCode;
    uint32_t        srmVODRequests;
    uint32_t        srmVODSuccesses;
    uint32_t        srmVODFailures;
} __attribute__((packed)) SRMSecStats;

typedef struct {
    wsu_shm_hdr_t   h;
    wsu_gate_t      srmTriggerGate;
    uint8_t         srm_msg[SRM_BUF_SIZE];
    uint8_t         srm_len;
    SRMMsgType      GS2_Msg;        /* GridSmart Msg */
    uint8_t         uper_payload[MAX_WSM_DATA];
    uint32_t        uper_payload_len;
    SRMSecStats     srmSecurityStats;
} scsSRMShmDataT;

#endif /* I2V_SRMRX16_DATA_H */
