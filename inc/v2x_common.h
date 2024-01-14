/*----------------------------------------------------------------------------*/
/*                                                                            */
/*    Filename: v2v_common.h                                                  */
/*                                                                            */
/* Copyright (C) 2019 DENSO International America, Inc.                       */
/*                                                                            */
/* Description: Common Constants and Macros                                   */
/*                                                                            */
/*----------------------------------------------------------------------------*/
#ifndef __V2V_COMMON_H__
#define __V2V_COMMON_H__

/*----------------------------------------------------------------------------*/
/* Include Files                                                              */
/*----------------------------------------------------------------------------*/
#include <syslog.h>
#include <stdio.h>          // For printf


/*----------------------------------------------------------------------------*/
/* Common Constant Definitions                                                */
/*----------------------------------------------------------------------------*/

/* Establish the base logic type */

#ifdef LOGIC_1
    #undef LOGIC_1
#endif
#define LOGIC_1 (1)

#ifdef LOGIC_0
    #undef LOGIC_0
#endif
#define LOGIC_0 (0)

/* Define public logic constants */

#ifdef TRUE
    #undef TRUE
#endif
#define TRUE LOGIC_1

#ifdef FALSE
    #undef FALSE
#endif
#define FALSE LOGIC_0

#ifdef ON
    #undef ON
#endif
#define ON LOGIC_1

#ifdef OFF
    #undef OFF
#endif
#define OFF LOGIC_0

#ifdef HIGH
    #undef HIGH
#endif
#define HIGH LOGIC_1

#ifdef LOW
    #undef LOW
#endif
#define LOW LOGIC_0

/* Define public bits */

#define BIT0  0x0001U
#define BIT1  0x0002U
#define BIT2  0x0004U
#define BIT3  0x0008U
#define BIT4  0x0010U
#define BIT5  0x0020U
#define BIT6  0x0040U
#define BIT7  0x0080U
#define BIT8  0x0100U
#define BIT9  0x0200U
#define BIT10 0x0400U
#define BIT11 0x0800U
#define BIT12 0x1000U
#define BIT13 0x2000U
#define BIT14 0x4000U
#define BIT15 0x8000U

#undef B0           // Warning control. Redefinition of B0 in <termios.h> Declare termios.h BEFORE v2x_common.h
#define B0  0U
#define B1  1U
#define B2  2U
#define B3  3U
#define B4  4U
#define B5  5U
#define B6  6U
#define B7  7U
#define B8  8U
#define B9  9U
#define B10 10U
#define B11 11U
#define B12 12U
#define B13 13U
#define B14 14U
#define B15 15U
#define B16 16U
#define B17 17U
#define B18 18U
#define B19 19U
#define B20 20U
#define B21 21U
#define B22 22U
#define B23 23U
#define B24 24U
#define B25 25U
#define B26 26U
#define B27 27U
#define B28 28U
#define B29 29U
#define B30 30U
#define B31 31U
/* If you want DEBUG for stand alone apps then define. */
#if defined(ENABLE_STANDALONE_DEBUG)
/*
 * V2X Logging
 */
#define LOG_EMERG   0
#define LOG_ALERT   1
#define LOG_CRIT    2
#define LOG_ERR     3
#define LOG_WARN    4
#define LOG_NOTICE  5
#define LOG_INFO    6
#define LOG_DEBUG   7

#define DEBUG_LEVEL LOG_INFO
#define WHERESTR "[%s : %s() : %d]: "
#define WHEREARG __FILE__,__func__,__LINE__

/* HD note:
 * Would be nice if we could use RSU syslogs but TPS and RADIO outside of RSU.
 * For now just a printf to serial.
 */
// Log everything to console, based on logging level defined above
#define V2X_LOG(X, fmt, ...)  if (X <= DEBUG_LEVEL) \
                                  printf(WHERESTR fmt "\n", WHEREARG, ##__VA_ARGS__)
#else
#define V2X_LOG(X, fmt, ...) 
#endif
/*
 * V2X Error Codes
 */
#define V2X_SUCCESS 0
#define V2X_FAILURE -1

/* ADDITION FOR HERC */
#ifndef EOK
#define EOK 0
#endif

///// Imported from WSU_GLOBAL.H -- App Messaging definitions ////

#include "dn_types.h"

/* To create a message queue for each service, need to obtain
   a System V IPC key with a valid pathname and a project ID. */

/* Project IDs for each service (process) */
#define EAS_PROJ_ID         0x10

/* Base of Message IDs for each service */
#define DEBUG_BASE_MSG_ID   001
#define EAS_BASE_MSG_ID     100
#define TPS_BASE_MSG_ID     200
#define VIS_BASE_MSG_ID     300
#define CMM_BASE_MSG_ID     400
#define PRM_BASE_MSG_ID     500
#define TAF_BASE_MSG_ID     600
#define SAM_BASE_MSG_ID     700
#define DLH_BASE_MSG_ID     800
#define OMM_BASE_MSG_ID     900
#define TCF_BASE_MSG_ID     1000


#define APP_ABBREV_SIZE     8           /* 8 bytes to maintain msg struct alignment */

#define SHM_PATH_SIZE       40          /* Max length for a SHM path */

/* Type for the message header for the message queue buffer */
typedef struct {
    uint32_t msgId;                      /* Message ID */
    int32_t  appPid;                     /* Source process's PID */
    int32_t  errorCode;                  /* Error code from the services */
} msgHeader_t;


/* Definitions for Debug messages.  They are not app-specific so they're here in global  */
typedef enum
{
    DEBUG_COMMAND_REQ = DEBUG_BASE_MSG_ID,
    DEBUG_COMMAND_RESP,
    DEBUG_PING_REQ,
    DEBUG_PING_RESP,
    DEBUG_REPORT_REQ,
    DEBUG_REPORT_RESP,
} debug_msgid_t;
typedef struct   // Request Msg type
{
    int32_t         cmd_id;
    int32_t         int_arg1;
    int32_t         int_arg2;
    char_t          char_arg1[100];
    char_t          char_arg2[100];
} dbg_req_t;
typedef struct   // Response msg type
{
    int32_t         cmd_id;
    int32_t         result_id;
    uint8_t         page_number;
    uint8_t         total_pages;
    char_t          text[1000];
} dbg_rpt_t;        // TODO: Rename rpt to resp
typedef struct
{
    msgHeader_t         hdr;
    char_t              appAbbrev[APP_ABBREV_SIZE]; /* Application name abbreviation */
    union
    {
        dbg_req_t       req;
        dbg_rpt_t       resp;        // Renamed rpt to resp
    } body;
} dbg_msg_t;

/* VALUES used by TPS and other projects */
#define MAX_TPS_TIME_BYTES  12
#define MAX_TPS_DATE_BYTES  8

#endif // __V2V_COMMON_H__
