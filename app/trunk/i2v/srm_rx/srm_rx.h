/**************************************************************************
 *                                                                        *
 *     File Name:  srmrx.h                                                *
 *     Author:                                                            *
 *         DENSO International America, Inc.                              *
 *         North America Research Laboratory, California Office           *
 *         3252 Business Park Drive                                       *
 *         Vista, CA 92081                                                *
 *                                                                        *
 **************************************************************************/

#ifndef SRM_RX_H
#define SRM_RX_H

#define SRM_AOK   0
#define SRM_ERROR_BASE 0

#define SRM_SHM_FAIL -1
#define SRM_INVALID_CONF -2
#define SRM_ASN1_INIT_FAIL -3
#define SRM_ASN1_CONTEXT_FAIL -4

#define SRM_CUSTOMER_DIGEST_FAIL -5
#define SRM_USER_SERVICE_FAIL -6
#define SRM_WSM_REQ_FAIL -7
#define SRM_BAD_INPUT    -8

#define SRM_CONF_FOPEN_ERROR -9
#define SRM_CONF_PARSE_ERROR -10
#define SRM_ASN1_FRAME_INIT_ERROR -11
#define SRM_ASN1_FRAME_BUFFER_ERROR -12

#define SRM_ASN1_FRAME_DECODE_ERROR -13
#define SRM_ASN1_ILLEGAL_SIZE       -14
#define SRM_ASN1_DECODE_PER_INIT_ERROR -15
#define SRM_ASN1_DECODE_PER_BUFFER_ERROR -16

#define SRM_ASN1_DECODE_PER_MSG_ERROR -17
#define SRM_THREAD_FAIL            -18
#define SRM_GS2_LANE_REQUEST_ERROR -19
#define SRM_GS2_LANE_NODE_ERROR -20

#define SRM_GS2_REQUEST_COUNT_ERROR -21
#define SRM_WSM_LENGTH_ERROR -22
#define SRM_SEQUENCE_ERROR -23
#define SRM_NO_REQUEST_ERROR -24

#define SRM_NO_TIMESTAMP_ERROR -25
#define SRM_REGIONAL_PRSENT -26
#define SRM_VEHICLE_ROLE_REJECT -27
//#define SRM_FWD_SOCKET_ERROR -28

#define SRM_WSM_WRONG_PSID  -29
//#define SRM_FWD_CONNECT_ERROR -30
//#define SRM_FWD_IP_ERROR -31
//#define SRM_FWD_WRITE_ERROR -32

#define SRM_FWD_TO_SCS_DISABLED -33
#define SRM_WRONG_INT_ID -34
//#define SRM_WSM_REQ_RESP_FAIL -35
#define SRM_HEAL_CFGITEM -36

//#define SRM_RX_MSG_THREAD_EXIT -37
#define SRM_MSG_STATS_THREAD_EXIT -38
#define SRM_HEAL_FAIL -37
#define SRM_VERIFICATION_FAIL -38

#define SRM_ERROR_TOP 65 /* Only define down to -64. Non-neg int on purpose.*/
/*
 * Function Prototypes
 */
#endif  /* SRM_RX_H */
