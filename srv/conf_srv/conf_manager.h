/*
 *  Filename: conf_manager.h
 *  Purpose: Configuration manager for Web interface
 *
 *  Copyright (C) 2019 DENSO International America, Inc.
 *
 *  Revision History:
 *
 *  Date        Author        Comments
 * --------------------------------------------------
 *  07-01-19    LNGUYEN    Initial release.
 */

#ifndef CONF_MANAGER_H
#define CONF_MANAGER_H


/* Error states. */

#define CFG_AOK 0

#define CFG_PARAM_FAIL -1   /* Bogus input to function ie null pointer. */
#define CFG_SET_VALUE_RANGE_ERROR -2
#define CFG_GETOPT_FAIL -3
#define CFG_MQ_OPEN_FAIL -4

#define CFG_HANDLING_THREAD_FAIL -5
#define CFG_MONITOR_THREAD_FAIL -6
#define CFG_GROUP_NOT_FOUND -7
#define CFG_FOPEN_FAIL -8

#define CFG_ELEMENT_NOT_FLAGGED -9
#define CFG_READ_ITEM_NOT_VALID -10
#define CFG_READ_ITEM_NOT_FOUND -11
#define CFG_UPDATE_ITEM_NOT_VALID -12

#define CFG_UPDATE_ITEM_NOT_FOUND -13
#define CFG_UPDATE_ITEM_FAILED -14
#define CFG_WRITE_ITEM_NOT_FOUND -15
#define CFG_WRITE_ITEM_NOT_VALID -16

#define CFG_WRITE_ITEM_REJECTED -17
#define CFG_FACTORY_REQUEST_FAIL -18
#define CFG_FACTORY_PRESERVE_REQUEST_FAIL -19
#define CFG_SESSION_CHECK_FAIL -20

#define CFG_UNKNOWN_REQUEST -21
#define CFG_NONE_REQUEST -22
#define CFG_MQ_READ_FAIL -23
#define CFG_MQ_MSG_UNKNOWN -24

#define CFG_MQ_RESP_OPEN_FAIL -25
#define CFG_MQ_RESP_SEND_FAIL -26
#define CFG_MQ_RESP_CLOSE_FAIL -27
#define CFG_SESSION_TIMEOUT -28

#define CFG_SYSTEM_CALL_FAILED -29
#define CFG_SET_ITEM_REJECTED -30
#define CFG_GET_ITEM_REJECTED -31
#define CFG_ITEM_NOT_FOUND   -32

#define CFG_PSID_BAD -33
#define CFG_SSP_LENGTH_BAD -34
#define CFG_FGETS_FAIL -35 
#define CFG_ITEM_LENGTH_BAD -36

#define CFG_VALUE_ILLEGAL_CHAR -37
#define CFG_VALUE_UNKOWN_PARAM -38
#define CFG_IP_ADDR_EMPTY -39
#define CFG_IP_ADDR_BAD -40

#define CFG_TOKEN_PARSE_KEY_FAIL -41
#define CFG_TOKEN_PARSE_VALUE_FAIL -42
#define CFG_DHCP_ENABLE_FAIL -43
#define CFG_STATIC_ENABLE_FAIL -44

#define CFG_TSCBM_UPDATE_FAIL -44
#define CFG_UNLOCK_FAIL      -45
#define CFG_COMMIT_FAIL      -46
#define CFG_SET_NETMASK_FAIL -47

#define CFG_SET_HTTP_PORT_FAIL -48
#define CFG_SET_HTTPS_PORT_FAIL -49
#define CFG_REVERT_FAIL  -50
#define CFG_DCU_RESPONSE_ERROR -51

#define CFG_CONF_TABLE_CHECK_FAIL -52
#define CFG_GET_NETMASK_FAIL -53
#define CFG_GET_IP_FAIL -54
#define CFG_GET_HTTP_PORT_FAIL -55

#define CFG_GET_HTTPS_PORT_FAIL   -56
#define CFG_GET_I2V_ENABLE_FAIL   -57
#define CFG_SEARCH_STARTUP_FAIL   -58
#define CFG_GET_NETWORK_MODE_FAIL -59

#define CFG_OPT_ITEM_NOT_FOUND     -60
#define CFG_GET_HEALTH_ENABLE_FAIL -61
#define CFG_SET_COMMIT_FAIL        -62
#define CFG_SET_REVERT_FAIL        -63
/* Dont define more than -63 for 64 bit error states */

#define CFG_GET_MTU_FAIL -64
#define CFG_GET_VLAN_ENABLE_FAIL -65
#define CFG_GET_VLAN_ID_FAIL -66
#define CFG_GET_VLAN_IP_FAIL -67

#define CFG_GET_VLAN_MASK_FAIL -68
#define CFG_SET_MTU_FAIL -69
#define CFG_SET_VLAN_ENABLE_FAIL -70
#define CFG_SET_VLAN_ID_FAIL -71

//#define CFG_SET_VLAN_IP_FAIL -72
#define CFG_SET_VLAN_MASK_FAIL -73

#define CFG_CUSTOMER_BACKUP_FAIL -74
#define CFG_CUSTOMER_RESET_FAIL -75

/* 
 *  Build message
 *  Msg format: 
 *      Read : HDR | "READ"  | CONF_GROUP | CONF_ITEM
 *      Write: HDR | "WRITE" | CONF_GROUP | CONF_ITEM | CONF_VAL
 */

//TODO: can token size handle long path/file name? 64 chars may not be enough?
/*
 * ===== MQ Stuff =====
 * Note: client MQ name will be process ID followed by thread id
 *       client MQ name is used for responses back from config manager
 */
#define QUEUE_NAME      "/config_server"
#define QUEUE_PERMISSIONS 0660
#define MAX_MESSAGES 10                         /* Max queue messages */
#define MAX_MSG_SIZE 128                        /* Size of message */
#define MAX_NAME_SIZE 32
/* max size is IPv6 addr + null. Strings can be longer like path + file name? */
#define MAX_TOKEN_SIZE 65                       /* Max token size -  */
/* set limit for arguments(tokens) that can be sent. */
#define MAX_TOKENS     64                       /* Max Tokens we'll look for: MAX_MSG_SIZE divide by  2 = 64 */
//TODO: use macro for 24, but from where?
#define MSG_BUFFER_SIZE MAX_MSG_SIZE + 24

/* Assume after this many chars we have failed. */
#define I2V_STARTUP_LINE_SEARCH_MAX  256

/*
 * conf_agent items
 */
/* conf_agent UPDATEALL */
/* conf_agent -d WRITE AMH AMHForwardEnable 1 */
#define CONF_AGENT_ARGC_MIN 2
#define CONF_AGENT_ARGC_MAX 6

/* Device type */
#define RSU_DEVICE      0
#define OBU_DEVICE      1


/*
 * ===== Config Stuff =====
 */
typedef enum {
    CONF_NONE,
    CONF_READ,
    CONF_WRITE,
    CONF_UPDATE,
    CONF_UPDATE_ALL,
    CONF_LOGIN,
    CONF_LOGOUT,
    CONF_SESSION_CHK,
    FACTORY_RESET,
    FACTORY_RESET_BUT_PRESRV_NET,
    CUSTOMER_RESET,
    CUSTOMER_BACKUP,
} confAction_e;

typedef enum {
    MSG_REQUEST,
    MSG_RESPONSE,
/* Compile error? keyword maybe?*/
    MSG_INFORM,
    MSG_ACK,
} msgResp_e;

typedef struct{
    uint8_t msg_type;    /* MSG_REQUEST, MSG_RESPONSE, MSG_INFO */
    uint8_t msg_op;      /* READ, WRITE, NONE, .... */
    uint8_t msg_len;
} __attribute__ ((packed)) msg_hdr_t;

typedef struct {
  uint32_t proc_id;
  uint32_t thrd_id;
  msg_hdr_t msg_attr;
} __attribute__ ((packed)) mq_hdr_t;


/*=== User Sessions 
 *  Config manager restrict one user session at a time
 * 
 * 
 * ===*/

#define LOG_OFF     0
#define LOG_ON      1
typedef struct {
  uint8_t    log_state;
  int16_t    duration;       /* unit in seconds */
  char_t     user_id[MAX_NAME_SIZE]; /* convert from remote addr */
} __attribute__ ((packed)) session_t;




#endif  /* CONF_MANAGER_H */ 
