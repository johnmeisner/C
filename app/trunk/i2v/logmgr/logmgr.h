/**************************************************************************
 *                                                                        *
 *     File Name: ifcmgr.h                                             *
 *     Author:                                                            *
 *         DENSO International America, Inc.                              *
 *         North America Research Laboratory, California Office           *
 *         3252 Business Park Drive                                       *
 *         Vista, CA 92081                                                *
 *                                                                        *
 **************************************************************************/

#ifndef IFCMGR_H
#define IFCMGR_H


#define LOGMGR_AOK  0

//TODO: split for logmgr and ifclog

/* 64 bit error state, define what you need */
#define LOGMGR_INIT_SHM_FAIL     -1
#define LOGMGR_SIG_HANDLER_EXIT  -2
#define LOGMGR_SEND_HANDLER_EXIT -3
#define LOGMGR_MSG_HANDLER_MQ_OPEN_FAIL -4

#define LOGMGR_CFG_DIR_NAME_TOO_LONG -5
#define LOGMGR_BAD_DATA -6
#define LOGMGR_CFG_LOAD_FAIL -7
#define LOGMGR_MSG_HANDLER_FAIL -8

#define LOGMGR_FILE_HANDLER_FAIL -9
#define LOGMGR_SEND_HANDLER_FAIL -10
#define LOGMGR_IFC_HANDLER_FAIL -11
#define LOGMGR_MSG_HANDLER_MQ_RX_FAIL  -12

#define LOGMGR_MSG_HANDLER_FILE_LOCKED  -13
#define LOGMGR_MSG_HANDLER_EXIT    -14
#define LOGMGR_FILE_HANDLER_DIR_CREATE_FAIL  -15
#define LOGMGR_FILE_HANDLER_CREATE_FAIL  -16

#define LOGMGR_IFC_CONF_CONFLICT  -17
#define LOGMGR_FILE_HANDLER_EXIT   -18
#define LOGMGR_IFC_BAD_INTERFACE  -19
#define LOGMGR_IFC_REQ_THREAD_FAIL  -20

#define LOGMGR_CHECK_DIR_SIZE_WARNING_TO  -21
#define LOGMGR_CHECK_SIZEAGE_MAX_HIT  -22
#define LOGMGR_CHECK_SIZEAGE_MAX  -23
#define LOGMGR_IFC_CHECK_SIZEAGE_NOFILE  -24 /* If no logs to check then this will occur. Should revert on success but is sticky. */

#define LOGMGR_IFC_CHECK_SIZEAGE_TIME_ERROR   -25
#define LOGMGR_IFC_CHECK_SIZEAGE_TOO_OLD  -26
#define LOGMGR_MAINT_MODE_ENABLE  -27
#define LOGMGR_MAINT_MODE_STATE_CLEANED  -28

#define LOGMGR_MAINT_MODE_NEW_LOG  -29
#define LOGMGR_MAINT_MODE_FILE_CLOSE  -30
#define LOGMGR_MAINT_MODE_FILE_CLOSE_NO_FILE  -31
#define LOGMGR_RESTORE_TIME_NULL_NAME  -32

#define LOGMGR_NO_TIMEMASTER  -33
#define LOGMGR_FORK_FAIL  -34
#define LOGMGR_START_IFC_FAIL  -35
#define LOGMGR_GET_I2V_CFG_SHM_FAIL  -36

#define LOGMGR_OPEN_CFG_FAIL  -37
#define LOGMGR_READ_CFG_FAIL  -38
#define LOGMGR_CHECK_SIZEAGE_PARSE_ERROR  -39
#define LOGMGR_CHECK_DIR_MAX_SIZE_WARN -40

#define LOGMGR_IFC_REQ_FAIL -41
#define LOGMGR_FWRITE_FAIL -42
#define LOGMGR_CHECK_FILE_SIZE_STAT_FAIL -43
#define LOGMGR_TIMEMASTER_RESET -44

#define LOGMGR_CHECKDIRSIZE_FOPEN_FAILED -45
#define LOGMGR_CHECKDIRSIZE_FREAD_FAILED -46
#define LOGMGR_HEAL_CFGITEM              -47
#define LOGMGR_HEAL_FAIL                 -48

#define LOGMGR_NULL_INPUT                -49
#define LOGMGR_IFC_START_LOG_PARAM_FAIL  -50
#define LOGMGR_SIG_CHILD_HANDLER_EXIT    -51
#define LOGMGR_IFCLOG_TOO_MANY_FILE      -52

#define LOGMGR_SYSTEM_FAIL -53
#define LOGMGR_FOPEN_FAIL   -54
#define LOGMGR_THREAD_FAIL  -55
#define LOGMGR_SPRINTF_FAIL  -56

#define LOGMGR_MUTEX_LOCK_FAIL -57
#define LOGMGR_SCANF_FAIL -58
#define LOGMGR_INPUT_NULL -59
#define LOGMGR_POPEN_FAIL -60

#define LOGMGR_FGETS_FAIL        -61
#define LOGMGR_IFC_MIB_SHM_ASYNC -62
#define LOGMGR_IFC_SEND_REQ_FAIL -63
#define LOGMGR_IFC_READ_REQ_FAIL -64

#define MAX_CHILD_PROCS   10  /* this is probably more than enough -
                                 ifc logging is through child processes - 
                                 this is for cleanup not to limit active
                                 children */

/* 1218 MIB: 5.8 rsuInterfaceLogs. */
#define IFC_START_WAIT_SECS 2  /* Minimum wait time in seconds between starting IFC(interface logs).*/


/* editors note: bitten twice by strtoul(xx, NULL, 0); - the problem is not defining the base
   at the end; in general it works fine unless a string is 08, 09 - these get parsed as octals
   and do not parse correctly since they are invalid in base 8 - the use of strtoul has been 
   checked but there are a few remaining with the special base value of 0 since by convention
   it is expected the values parsed will never present an issue - but be careful when editing
   the calls to strtoul or adding any (atoi doesn't detect errors btw) */

typedef struct {
    char_t  logPath[I2V_CFG_MAX_STR_LEN];
    uint8_t  logSize;    /* a MB value */
    uint8_t  logPriority;
    uint16_t logDuration;   /* a value in days */
    uint16_t dirsize;
    uint8_t  threshold;
    uint16_t logRmvAgeThresh;
    bool_t   xmitShutdownEnable;
    uint16_t xmitInterval;
    bool_t   xmitIpv4;
    uint8_t  ifcLogSizeMax;  /* a MB value */
    uint32_t ifcLogDuration;  /* a value in seconds */
    uint8_t  ipv4addr[LENGTH_IPV4];
    uint8_t  ipv6addr[LENGTH_IPV6];
    char_t   scpuser[I2V_CFG_MAX_STR_LEN];
    char_t   scpdir[I2V_CFG_MAX_STR_LEN];
    uint32_t silenceOutput;
} __attribute__((packed)) lconfig;

typedef enum {
    STATE_DEFAULT,
    STATE_STARTING,
    STATE_CLEANED,
} lmgrV4MaintMode;

typedef struct {
    lmgrIfcReqState request;
    i2vIfcList ifc;
    bool_t reqPending;
} __attribute__((packed)) ifcControl;


#endif /* I2V_TYPES_H */

