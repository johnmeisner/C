/**************************************************************************
 *                                                                        *
 *     File Name: i2v_general.h                                           *
 *     Author:                                                            *
 *         DENSO International America, Inc.                              *
 *         North America Research Laboratory, California Office           *
 *         3252 Business Park Drive                                       *
 *         Vista, CA 92081                                                *
 *                                                                        *
 **************************************************************************/

/* NEVER add functions protos to this header file - it is only for type definitions
   and defines - it is included by the standalone library for i2v */

#ifndef I2V_GENERAL_H
#define I2V_GENERAL_H

#include <stdio.h>
#include "wsu_shm_inc.h"
#include "i2v_types.h"

#define GCC_UNUSED_VAR __attribute__((unused))

/* 
 * Time & Iteration each task should sleep waiting for I2V CFG SHM to become valid: shmPtr->cfgData.h.ch_data_valid = WTRUE. 
 * If the task timesout waiting it is free to continue as stand alone app or error out depending.
 */
#define MAX_i2V_SHM_WAIT_USEC  (500000)   /* Half second. */
#define MAX_I2V_SHM_WAIT_ITERATIONS (60)  /* 30 secs seems plenty. */

/* Handles printf and syslog output.
 * Controlled by i2v.conf:i2vGlobalDebugFlag and i2v.conf:i2vSysLogGlobalControl.
 */
#define I2V_DBG_LOG(lvl,name,fmt, args...) {i2vUtilAddToSysLog(lvl, name, fmt, ##args);}

/* 
 * Ideally this stuff does not go to syslogs but user requests for syslogs dumps are parsed.
 * We can take what's need and ignore the rest.
 * Benefit is LOGMGR will manage size and date reqs of this output so it doesn't grow out of control.
 */
#define I2V_CSV_LOG(lvl,name,fmt, args...) I2V_DBG_LOG(lvl, name, fmt, ##args)
#define I2V_APP_LOG(lvl,name,fmt, args...) I2V_DBG_LOG(lvl, name, fmt, ##args)

#define I2V_CFG_UNDEFINED       "Undefined"
#define I2V_CONF_FILE           "i2v.conf"
#if defined(MY_UNIT_TEST)
#define I2V_CONF_DIR            "./"
/* no backup directory (when no rwflash) for logging */
#define I2V_DEFAULT_APPLOGS     "/tmp/I2V/applogs"
#define I2V_DEFAULT_DBGLOGS     "/tmp/I2V/logs"
#define I2V_DEFAULT_DATADIR     "/tmp/I2V/data"

#define I2V_LOGMSG_QUEUE        "/tmp/loggingMsgQ"
#define I2V_WMH_QUEUE           "/tmp/rwflash/I2V/wmhQ"
#define I2V_WMH_BACKUP_QUEUE    "tmp/I2V/wmhQ"
#define I2V_FWDMSG_QUEUE        "/tmp/I2V/i2vFwdmsgQ"

#define IFC_COMPLETED_LOGS_DIR   "/tmp/rwflash/I2V/ifclogs"
#define IFC_INPROGRESS_LOGS_DIR  "/tmp/ifclogs"  

#else
/* Already defined ../shm_inc/i2v_shm_master.h:34. Remove? */
#define I2V_SHM_PATH            "/i2v_shm"
#define I2V_CONF_DIR            "/rwflash/configs/"
/* no backup directory (when no rwflash) for logging */
#define I2V_DEFAULT_APPLOGS     "/mnt/rwflash/I2V/applogs"
#define I2V_DEFAULT_DBGLOGS     "/mnt/rwflash/I2V/logs"
#define I2V_DEFAULT_DATADIR     "/mnt/rwflash/I2V/data"
//TODO: LOGMGR can open mq at root but I2V can't. Neither can open in /tmp :-S
//I2V uses mkfifo not mqopen. Maybe thats why?
#define I2V_LOGMSG_QUEUE        "/loggingMsgQ"  //ends up in /dev/mqueue
#define I2V_WMH_QUEUE           "/rwflash/I2V/wmhQ"
#define I2V_FWDMSG_QUEUE        "/i2vFwdmsgQ"
#define I2V_WMH_BACKUP_QUEUE    "/rwflash/I2V/wmhQ_backup"

/* 
 * These are two different folders on purpose. Makes maintaining easier.
 * Using /tmp on purpose to avoid /rwflash for recordings.
 */
#define IFC_COMPLETED_LOGS_DIR   "/rwflash/I2V/ifclogs"  /* do not log in same location as destination */
#define IFC_INPROGRESS_LOGS_DIR  "/tmp/ifclogs"    /* capture_pcap uses /tmp so stay away */

#endif /* MY_UNIT_TEST */

/* max log msg is 1024 of which 17 is reserved for timestamp and 17 for label */
#define MAX_LOG_MSG_SIZE        990
#define LOG_LABEL_SIZE          17
typedef struct _LogMsg
{
  char_t entry[MAX_LOG_MSG_SIZE + LOG_LABEL_SIZE + 2];
} LogMsg;

#ifdef RSE_SIMULATOR
#define I2V_SIM_SPAT_QUEUE      "/spsQ"
#endif /* RSE_SIMULATOR */
#define DEFAULT_Q_MODE          664
#define I2V_Q_READER            O_RDONLY | O_NONBLOCK
#define I2V_Q_SENDER            O_WRONLY | O_NONBLOCK

/* Mqueue attributes */
#define DEFAULT_MQ_ATTRIBUTES    (0)
#define DEFAULT_MQ_MAX_MSG       (8)
#define DEFAULT_MQ_DESCR_FLAGS   O_WRONLY | O_CREAT | O_EXCL | O_NONBLOCK
#define DEFAULT_MQ_PERMS         0666
#define DEFAULT_MQ_PRIORITY      0

/* modeling v2v-i for STR_LEN */
// If you use i2vUtilUpdateStrValue() to load strings from confs 
// then the destination must be sizeof I2V_CFG_MAX_STR_LEN.
#define I2V_CFG_MAX_STR_LEN   (100 + 1)
#define I2V_CFG_MAX_FILE_LEN  (32 + 1)
#define I2V_IP_MAX_STR_LEN    I2V_CFG_MAX_STR_LEN  //(42)
/* to save space format: MM/DD/YYYY,hh:mm:ss:[uuuuuu] */
#define I2V_TIME_MAX_STR_LEN  I2V_CFG_MAX_STR_LEN  //(30)

/* Max SSP String Length */
#ifdef HSM
#define MAX_SSP_STRING_LEN I2V_CFG_MAX_STR_LEN  //64
#endif

/* 0 to 63 are supported values - really a don't care as long as all apps use it */
#define I2V_ALL_APP_WSA_PRIORITY   40
/* dummy value - not used by the stack */
#define I2V_DEFAULT_WSA_REPEAT_RATE  10
/* remember to modify these values for appropriate geo support */
#define I2V_CONTROL_CHANNEL     183
#define I2V_MIN_SVC_CHANNEL     172
#define I2V_MAX_SVC_CHANNEL     184

/* device ID; 32 bytes (customer bytes) reserved (not null terminated) */
#define I2VRSUIDSIZE    32

#ifndef NUMITEMS
#define NUMITEMS(x) (sizeof(x)/sizeof((x)[0]))
#endif

#ifndef UNDEFINED
#define UNDEFINED   "Undefined"
#endif

#define PROC_IPV6_FORWARD_FNAME "/proc/sys/net/ipv6/conf/all/forwarding"

/* Interface Logging: Only works if (_nosyslog == WFALSE).
 *     none  = 0x00 
 *     all   = 0xff
 *     eth0  = 0x01
 *     eth1  = 0x02 :Not present: room for growth.
 *     dsrc0 = 0x04 :If Cont mode then no dsrc1.
 *     dsrc1 = 0x08
 *     cv2x0 = 0x10
 *     cv2x1 = 0x20 :Not present: room for growth. 
 */
#define IFCLOG_ENABLE_ALL_MASK  (0xff)
#define IFCLOG_DISABLE_ALL_MASK (0x00)
#define IFCLOG_ETH0_ENABLE_BIT  (0x01)
#define IFCLOG_ETH1_ENABLE_BIT  (0x02)
#define IFCLOG_DSRC0_ENABLE_BIT (0x04)
#define IFCLOG_DSRC1_ENABLE_BIT (0x08)
#define IFCLOG_CV2X0_ENABLE_BIT (0x10)
#define IFCLOG_CV2X1_ENABLE_BIT (0x20)

/* global i2v return type */
typedef enum {
    I2V_RETURN_OK=0,
    I2V_RETURN_FAIL,
    I2V_RETURN_UNKNOWN,
    I2V_RETURN_SHM_FAIL,
    I2V_RETURN_CHILD_FAIL,
    I2V_RETURN_INVALID_PARAM,
    I2V_RETURN_INVALID_CONFIG,
    I2V_RETURN_INV_FILE_OR_DIR,
    I2V_RETURN_NULL_PTR,
    I2V_RETURN_STR_MAX_LIMIT_FAIL,
    I2V_RETURN_MIN_LIMIT_FAIL,
    I2V_RETURN_MAX_LIMIT_FAIL,
    I2V_RETURN_RANGE_LIMIT_FAIL,
    I2V_RETURN_OP_INCOMPLETE,
    I2V_RETURN_MISSING_DATA,
    I2V_RETURN_NO_ACTION,
    I2V_RADIO_INIT_FAIL,
    I2V_RADIO_FAIL,
    I2V_RADIO_USR_SERVICE_FAIL,
    I2V_RADIO_WSM_SERVICE_FAIL,
    I2V_WSM_ILLEGAL_SIZE,
    I2V_ASN1_CONTEXT_INIT_FAIL,
    I2V_ASN1_FRAME_INIT_FAIL,
    I2V_ASN1_SRM_INIT_FAIL,
    I2V_ASN1_FRAME_DECODE_ERROR,
    I2V_ASN1_SRM_DECODE_ERROR,
    I2V_ASN1_SRM_SEQUENCE_NUMBER_DUPLICATE,
    I2V_ASN1_SRM_INVALID_PARAM,
    I2V_ASN1_FRAME_INVALID_PARAM,
    I2V_ASN1_SET_BUFFER_FAIL,
    I2V_ASN1_SRM_ILLEGAL_SIZE,
    I2V_ASN1_FRAME_ILLEGAL_SIZE,
    I2V_SRM_VEHICLE_ROLE_FAIL,
    I2V_IWMH_RETURN_INVALID_CONFIG,
    I2V_IWMH2_RETURN_INVALID_CONFIG,
    I2V_IWMH3_RETURN_INVALID_CONFIG,
} i2vReturnTypesT;

/* struct for passing logging to supporting library calls */
typedef struct {
    FILE  *flog;
    FILE  *fcsv;
    WBOOL *dbgFlag;
    WBOOL *appFlag;
} i2vLogWrapper;

/* Config types for files versus memory are different. In memory no need for min,max,trange since already parsed from file(SAR) or socket(IMF).*/
typedef struct {
        char_t  *tag;
        void (*updateFunction)(void* configItem, char_t *val, char_t *min, char_t *max, char_t **trange, uint32_t *status, i2vLogWrapper *logger);
        void *itemToUpdate;
        void (*customUpdateFxn)(char_t *tag, char_t *parsedItem, char_t *val, void* configItem, uint32_t *status);	
        /* this function will take priority if non null instead of the generic updateFunction in the struct the purpose of 
           this pointer is to allow special app specific update functions for config values - parsedItem is the string 
           parsed from the file, configItem may not be used but is passed in case fxn requires it 
           TBD improvement: the function should get the full line from the config file not just the parsedItem and val */
        uint8_t state; /* valid AND (uninitialized or initialized or bad value or none) */
                       /* from conf_table.h*/
} cfgItemsTypeT;

typedef struct {
    wsu_shm_hdr_t h;
    
    pid_t   i2v;
    pid_t   scs;
    pid_t   fwdmsg;
    pid_t   srm;
    pid_t   escsmpc;
    pid_t   spat;
    pid_t   map;
    pid_t   tim;
    pid_t   ipb;
    pid_t   amh;
#ifdef RSE_SIMULATOR
    pid_t   sps;
#endif /* RSE_SIMULATOR */
} processPidsT;

struct pcmd {
    char_t   *pname;
    char_t   **cmd;
    pid_t    *pid;
    uint32_t alive;     /* copied from v2vi - may not support in i2v */
    bool_t   *appEnableFlag;
};
#define LENGTH_IPV4  4
#define LENGTH_IPV6  16         // IPv6 Address Length

#if !defined(MY_UNIT_TEST)
#define CONNECT             connect
#define INET_ATON           inet_aton
#define SOCKET              socket
#define WSU_SHARE_INIT      wsu_share_init
#define WSU_SHARE_KILL      wsu_share_kill
#define WSU_SEM_LOCKW     wsu_shmlock_lockw
#define WSU_SEM_UNLOCKW   wsu_shmlock_unlockw
#define WSU_SEM_LOCKR      wsu_shmlock_lockr
#define WSU_SEM_UNLOCKR    wsu_shmlock_unlockr
#define WSU_SHMLOCK_LOCKW     wsu_shmlock_lockw
#define WSU_SHMLOCK_UNLOCKW   wsu_shmlock_unlockw
#define WSU_SHMLOCK_LOCKR      wsu_shmlock_lockr
#define WSU_SHMLOCK_UNLOCKR    wsu_shmlock_unlockr
#endif

/* from snmp.h. Just easier than dragging whole net-snmp in. */
#if !defined(SNMP_ROW_NONEXISTENT)
#define SNMP_ROW_NONEXISTENT		0
#endif
#if !defined(SNMP_ROW_ACTIVE)
#define SNMP_ROW_ACTIVE		1
#endif
#if !defined(SNMP_ROW_NOTINSERVICE)
#define SNMP_ROW_NOTINSERVICE		2
#endif
#if !defined(SNMP_ROW_NOTREADY)
#define SNMP_ROW_NOTREADY		3
#endif
#if !defined(ROW_CREATEANDGO)
#define SNMP_ROW_CREATEANDGO		4
#endif
#if !defined(SNMP_ROW_CREATEANDWAIT)
#define SNMP_ROW_CREATEANDWAIT		5
#endif
#if !defined(SNMP_ROW_DESTROY)
#define SNMP_ROW_DESTROY		6
#endif

#endif /* I2V_GENERAL_H */
