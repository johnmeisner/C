/**************************************************************************
 *                                                                        *
 *     File Name:  spat16.h                                               *
 *     Author:                                                            *
 *         DENSO International America, Inc.                              *
 *         North America Research Laboratory, California Office           *
 *         3252 Business Park Drive                                       *
 *         Vista, CA 92081                                                *
 *                                                                        *
 **************************************************************************/

#ifndef SPAT_16_H
#define SPAT_16_H

#include "ris_struct.h"
#include "i2v_general.h"

#define SPAT_CFG_FILE "spat16.conf"
#define TSCBM_PATH_AND_NAME "/rwflash/configs/tscbm.conf"

/* Basic return values. */
#define SPAT16_AOK                     0

#define SPAT16_ERROR_BASE              0
#define SPAT16_SHM_FAIL                -1  /* FATAL. */
#define SPAT16_LOAD_CONF_FAIL          -2  /* FATAL. */
#define SPAT16_INIT_RADIO_FAIL         -3  /* FATAL. */
#define SPAT16_INIT_ASN1_CONTEXT_FAIL  -4  /* FATAL. */
#define SPAT16_FWDMSG_MQ_SEND_ERROR    -5
#define SPAT16_BAD_INPUT               -6
#define SPAT16_RTX_MEM_ALLOC_FAIL      -7
#define SPAT16_ASN1_ENCODE_ERROR       -8
#define SPAT16_UPER_FRAME_ERROR        -9
#define SPAT16_UPER_ENCODE_ERROR       -10
#define SPAT16_WSM_REQ_FAILED          -11
#define SPAT16_CUSTOMER_DIGEST_FAIL    -12
//#define SPAT16_FWD_IP_ERROR            -13
//#define SPAT16_FWD_CONNECT_ERROR       -14
//#define SPAT16_FWD_WRITE_ERROR         -15
#define SPAT16_COMM_RANGE_THREAD_EXIT  -16
#define SPAT16_FWD_INPUT_ERROR         -17
#define SPAT16_SIG_FAULT               -18
#define SPAT16_RTINIT_CONTEXT_FAIL     -19
#define SPAT16_WSM_REQ_RESP_FAILED     -20
//#define SPAT16_EMPTY_MSG                      -21
#define SPAT16_TSCBM_FOPEN_FAIL               -22
#define SPAT16_OPEN_CONF_FAIL                 -23
#define SPAT16_CONF_PHASE_ENTRIES_ZERO        -24
#define SPAT16_UPERFRAME_SPAT_BAD_PARAMS      -25
#define SPAT16_ASN1_INIT_MESSAGE_FRAME_FAIL   -26
#define SPAT16_HEAL_CFGITEM                   -27
#define SPAT16_HEAL_FAIL                      -28
#define SPAT16_NULL_INPUT                     -29
#define SPAT16_CREATE_PSM_FWD_FAIL            -30
//#define SPAT16_BSMRX_FWD_DROPPED              -31
#define SPAT16_BSM_DECODE_FAILED              -32
#define SPAT16_BSM_BAD_LOCATION               -33
#define SPAT16_BSM_RSU_SAME_LOCATION          -34
#define SPAT16_BSM_ZERO_LENGTH                -35
#define SPAT16_BSM_DSRC_MSG_ID_BAD            -36
#define SPAT16_BSM_DISTANCE_WARNING           -37
#define SPAT16_BSM_BEARING_WARNING            -38
#define SPAT16_BSMRX_NULL_WSM                 -39
#define SPAT16_BSMRX_PSID_MISMATCH            -40
//#define SPAT16_FWD_SOCKET_NOT_READY           -41
//#define SPAT16_CREATE_FWD_MGR_THREAD_FAIL     -42
//#define SPAT16_FWD_SOCKET_BUSY                -43
//#define SPAT16_FWD_WRITE_NOT_READY            -44
//#define SPAT16_FWD_WRITE_POLL_TIMEOUT         -45
//#define SPAT16_FWD_WRITE_POLL_FAIL            -46
#define SPAT16_RX_MSG_THREAD_EXIT             -47
#define SPAT16_MSG_STATS_THREAD_EXIT          -48
#define SPAT16_ASN1_SPAT_CREATE_FAIL          -49
#define SPAT16_BSM_FWD_DROPPED -50
#define SPAT16_BSM_DECODE_MSG_FRAME_FAIL -51
#define SPAT16_BSM_DECODE_MSG_ID_FAIL -52
#define SPAT16_BSM_DECODE_BODY_FAIL -53
//#define SPAT16_BSM_FWD_DECODE_FAIL -54
#define SPAT16_PSM_DECODE_MSG_FRAME_FAIL -55
#define SPAT16_PSM_DECODE_MSG_ID_FAIL -56
#define SPAT16_PSM_DECODE_BODY_FAIL -57
//#define SPAT16_PSM_FWD_DECODE_FAIL -58
#define SPAT16_RTINIT_FAIL -59


#define SPAT16_CREATE_FWD_THREAD_FAIL  -64
#define SPAT16_ERROR_TOP  65 /* Only define down to -64. Non-neg int on purpose.*/

/* JJG interoperability compatibility options (bitmask) */
#define INTEROP_SIMPLE_SPAT                   0x01
#define INTEROP_MIN_AND_MAX_SPAT              0x02
#define INTEROP_UTC_SPAT                      0x04
#define INTEROP_PED_PRESENCE                  0x08

#define BSMFWD_ETH 0x1
#define BSMFWD_LTE 0x2
/*
 * Note:
 *  use timestamp as timer when to update
 * test sending 10-100 messages per minutes - array of buffers, no malloc
 */
#define MAX_BSM_LEN         (128)
#define MAX_BSM_NODE        (100)
/* update interval set at 60 seconds */
//#define BSM_FWD_INTERVAL  (60)
#define BSM_FWD_INTERVAL    (5)
#define LTE_NET_INTERFACE   "qwf0"
#define BSM_FW_MAXSIZE      (96)

#define MAX_SIGNAL_GROUP_ID  (254)
#define MIN_SIGNAL_GROUP_ID  (0)

#define MAX_PHASE_NUMBER  (16)
#define MIN_PHASE_NUMBER  (1)

void    bsmRxHandler(inWSMType *wsm);
void    Init_BSMRX_Statics(void);

#endif  /* SPAT_16_H */
