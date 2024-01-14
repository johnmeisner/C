/***************************************************
 *                                                 *
 *     File Name: scs.h                            *
 *     Author:    DENSO Int. America, Inc.         *
 *                Carlsbad, CA USA                 *
 *                                                 *
 ***************************************************
 * (C) Copyright 2021 DENSO Int. America, Inc.     *
 *      All rights reserved.                       *
 ***************************************************/
#ifndef SCS_H
#define SCS_H

#define SCS_CFG_FILE "scs.conf"

/*
 * MAXIMUM 64 bit mask => i2v_scs_data.h::scsCfgItemsT:: uint64_t error_states. 
 * To error or fail: That is the question.  
 */
#define SCS_AOK                    (0)
#define SCS_SHM_INIT_FAIL          (-1) /* FATAL: Control Loop won't work */
#define SCS_LOAD_CONF_FAIL         (-2)
#define SCS_CUSTOMER_DIGEST_FAIL   (-3)
#define SCS_INIT_TC_SOCK_FAIL      (-4)
//#define SCS_INIT_TC_THREAD_FAIL    (-5)
#define SCS_INIT_GS2_SOCK_FAIL     (-6)
#define SCS_CREATE_GS2_THREAD_FAIL (-7)
#define SCS_INIT_SRM_SOCK_FAIL     (-8)
#define SCS_CREATE_SRM_THREAD_FAIL (-9)
#define SCS_CREATE_SRM_HEARTB_FAIL (-10)
//#define SCS_SEM_INIT_FAIL          (-11)
#define SCS_SIG_FAULT              (-12)
#define SCS_TC_SOCKET_ERROR        (-13)
#define SCS_TC_BIND_ERROR          (-14)
#define SCS_SRM_SOCKET_ERROR       (-15)
#define SCS_SRM_BIND_ERROR         (-16)
#define SCS_TW_PARSE_ERROR         (-17)
#define SCS_ICD_PARSE_ERROR        (-18)
#define SCS_SPAT_MISSING_DATA      (-19)
#define SCS_SEND_SRM_NULL_INPUT    (-20)
#define SCS_SEND_SRM_SOCKET_CLOSED (-21)
#define SCS_SEND_SRM_SEND_ERROR     (-22)
#define SCS_SEND_SRM_RAW_NULL_INPUT  (-23)
#define SCS_SEND_SRM_RAW_SOCKET_CLOSED (-24)
#define SCS_SEND_SRM_RAW_SEND_ERROR   (-25)
#define SCS_GET_MAC_ADDR_NOT_FOUND    (-26)
#define SCS_GET_MAC_ADDR_ERROR        (-27)
#define SCS_SRMMC_MIBETA_ERROR        (-28)
#define SCS_SRMMC_MIBACTIVE_ERROR     (-29)
#define SCS_SRM_BAD_INTERFACE         (-30)
#define SCS_SRM_MALFORMED             (-31)
#define SCS_SRM_CSV_FOPEN_ERROR      (-32)
#define SCS_SRM_HEARTBEAT_INPUT_ERROR (-33)
#define SCS_SRM_HEARTBEAT_SOCKET_NOT_READY (-34)
#define SCS_SRM_HEARTBEAT_SEND_FAIL (-35)
//#define SCS_OPEN_LOG_ERROR          (-36)
#define SCS_CONF_FOPEN_ERROR        (-37)
#define SCS_CONF_PARSE_ERROR        (-38)
#define SCS_MAIN_SELECT_ERROR       (-39)
#define SCS_MAIN_RECVFROM_ERROR     (-40)
#define SCS_MAIN_SPAT_PROCESS_ERROR (-41)
#define SCS_MAIN_SPAT_LENGTH_ERROR  (-42)
#define SCS_MAIN_CONTROLLER_IP_ERROR (-43)
#define SCS_MAIN_YELLOW_DURATION_ERROR (-44)
//#define SCS_MAIN_PHASE_ENABLE_ERROR    (-45)
#define SCS_SNMP_MAIN_MISSING_DATA_ERROR (-46)
#define SCS_SNMP_MAIN_PARSE_ERROR        (-47)
#define SCS_SRM_BAD_TTI -48
#define SCS_HEAL_CFGITEM -49
#define SCS_HEAL_FAIL -50
#define SCS_HEAL_NULL_INPUT -51
#define SCS_SRM_OPEN_MQ_FAIL       (-52)
#define SCS_SEND_SRM_SEND_TRUNCATE -53

/* 
 * TM TODO: Is this better to do in SCS before it goes out to world?
 *        : If srmrx.c is the only place generated then better there?
 *        : But if the source is perhaps from elsewhere better to have SCS verify?
 */
/* Bitmask for validate_srm_msg() return type. */
#define SRM_VERIFY_AOK                   (0x0)
#define SRM_VERIFY_NULL_INPUT            (0x1)
#define SRM_VERIFY_BAD_VERSION_TYPE      (0x2)
#define SRM_VERIFY_BAD_MESSAGE_SECOND    (0x4)
#define SRM_VERIFY_BAD_MESSAGE_COUNT     (0x8)
#define SRM_VERIFY_BAD_SEQ_NUM           (0x10)
#define SRM_VERIFY_BAD_TIMESTAMP         (0x20)
#define SRM_VERIFY_BAD_MESSAGE_TIMESTAMP (0x40)
#define SRM_VERIFY_WARN_INTERSECTION_ID  (0x80)
#define SRM_VERIFY_WARN_REQ_ID           (0x100)
#define SRM_VERIFY_WARN_INBOUND_LANE     (0x200)
#define SRM_VERIFY_BAD_ETA_MUY           (0x400)
#define SRM_VERIFY_BAD_ETA_SECOND        (0x800)
#define SRM_VERIFY_WARN_VEHICLE_ID       (0x1000)
#define SRM_VERIFY_BAD_VEHICLE_ROLE      (0x2000)
#define SRM_VERIFY_BAD_LAT               (0x4000)
#define SRM_VERIFY_BAD_LON               (0x8000)
#define SRM_VERIFY_BAD_ELV               (0x10000)
#define SRM_VERIFY_BAD_HDG               (0x20000)
#define SRM_VERIFY_BAD_RSU_ID            (0x40000)
#define SRM_VERIFY_BAD_PRIORITY_REQUEST_TYPE (0x80000)

/* msgType = 1 msgVersion = 1 */

 /* One and only one def today */
#define SRM_MESSAGE_VERSION  MSG_VERSION_SRM 
#define SRM_MESSAGE_TYPE     MSG_TYPE_SRM  

/* Unsigned so not possible to be lower */
#define SRM_MESSAGE_SECOND_MIN           (0)     
#define SRM_MESSAGE_LEAP_SEC_START       (60000)
#define SRM_MESSAGE_LEAP_SEC_END         (60999)

#define SRM_MESSAGE_SECOND_RESERVE_START (61000)
#define SRM_MESSAGE_SECOND_RESERVE_STOP  (65534)

#define SRM_MESSAGE_SECOND_UNAVAILABLE   (65535)
#define SRM_MESSAGE_SECOND_MAX           (65535)

#define SRM_MESSAGE_MUY_MAX        (60 * 24 * 365) /* Minutes of Year: Leap second concerns? */

/* unsigned so not possible to be less than */
#define SRM_MSG_COUNT_MIN (0)    
#define SRM_MSG_COUNT_MAX (127)

/* Feb 1st 2021 sometime. */
#define UTC_AT_AUTHOR_TIME  (1612201832)

/* SRM Priority Request types: could be enum  */
#define SRM_REQ_TYPE_RESERVED             (0)
#define SRM_REQ_TYPE_PRIORITY_REQUEST     (1)
#define SRM_REQ_TYPE_PRIORITY_UPDATE      (2)
#define SRM_REQ_TYPE_PRIORITY_CANCELATION (3) 

/* FROM : fac/wmh/asn1/J2735_2016_03_UPER/DSRC.h */

#if 0 /* I guess we could use this but today I'm going easy route */
#include "dsrc.h" /* SRM Vehicle Role types aka SRM_RX.CONF::SRMTXVehBasicRole */
#else
#define basicVehicle     (0)
#define publicTransport  (1) 
#define specialTransport (2) 
#define dangerousGoods   (3) 
#define roadWork         (4) 
#define roadRescue       (5) 
#define emergency        (6) 
#define safetyCar        (7) 
#define noneunknown      (8) 
#define truck            (9) 
#define motorcycle       (10) 
#define roadSideSource   (11) 
#define police           (12) 
#define fire             (13) 
#define ambulance        (14) 
#define dot              (15) 
#define transit          (16) 
#define slowMoving       (17) 
#define stopNgo          (18) 
#define cyclist          (19) 
#define pedestrian       (20)
#define nonMotorized     (21) 
#define military         (22)
#endif

#define SRM_LAT_MIN      (-900000000)
#define SRM_LAT_MAX      ( 900000001)

#define SRM_LON_MIN      (-1799999999)
#define SRM_LON_MAX      ( 1800000001)

#define SRM_ELV_MIN      (-4096)
#define SRM_ELV_MAX      (61493)  /* Sure hope thats dm's cause my car dont go that high */

#define SRM_HDG_ANGLE_MIN  (0)
#define SRM_HDG_ANGLE_MAX  (28800)

#define RSU_ETHERNET    "eth0" /*Herc RSU*/
#if defined(MY_UNIT_TEST)
#undef  RSU_ETHERNET
#define RSU_ETHERNET    "ens33" /* Herc VM */
#endif

#define SRM_CSV_MAX_LINE (256)

#define MSG_VERSION_HB      (1)           /* Message version */
#define MSG_VERSION_SRM     (2)           /* Message version */
#define MSG_TYPE_SRM        (1)           /* SRM */
#define MSG_TYPE_HB         (200)         /* Heartbeat */
#define SECOND_TO_MICRO     (1000*1000)   /* one second to micro second */
#define SECOND_TO_MILLI     (1000)        /* one second to milli second */
#define SRM_CSV_LOOP_COUNT  (100)         /* SRM playback loop count */

typedef struct {
    uint8_t  rsuID[LENGTH_MAC];     /* RSU ID using Eth address */
    uint32_t seqNum;
    uint8_t  msgVersion;   /* Msg version */
    wtime   timeStamp;
    uint8_t  msgType;
} __attribute__((packed)) hbMsgType;

#if defined(GS2_SUPPORT) && defined(GS2_FAKE_DZ_MESSAGES)
#define FAKE_GS2_DZ_MESSAGE_TRIGGER (100)
static enum GS2Request nextFakeDZRequest = DilemmaZoneSet;
#endif


#endif /* SCS_H */
