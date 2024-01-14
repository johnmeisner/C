/**************************************************************************
 *                                                                        *
 *     File Name:  logmgr.c                                               *
 *     Author:                                                            *
 *         DENSO International America, Inc.                              *
 *         North America Research Laboratory, California Office           *
 *         3252 Business Park Drive                                       *
 *         Vista, CA 92081                                                *
 *                                                                        *
 **************************************************************************/
/* For info on syslog format see https://datatracker.ietf.org/doc/html/rfc5424 */


#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <netinet/in.h>
#include <net/if.h>
#include <time.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <signal.h>
#include <mqueue.h>

#include "wsu_sharedmem.h"
#include "wsu_shm.h"
#include "i2v_general.h"
#include "i2v_shm_master.h"
#include "i2v_util.h"
#include "ntcip-1218.h"
#include "conf_table.h"
#include "logmgr.h"

#if defined(MY_UNIT_TEST)
#include "stubs.h"
#define MAIN logmgr_main
#else
#define MAIN main
#endif
#define POPEN popen
#define PCLOSE pclose

/* mandatory logging defines */
#if defined(MY_UNIT_TEST)
#define MY_ERR_LEVEL   LEVEL_DBG
#else
#define MY_ERR_LEVEL   LEVEL_PRIV
#endif
#define MY_NAME        "logmgr"

//#define EXTRA_DEBUG
#define CONFFILE  "syslogdot.conf"
#define PREFIX    "i2vstat_"
#define PREFIX_LENGTH  8
#define TMPFILE   "/tmp/asdfasdfasd"

//TODO: NTCIP-1218 dir and file names are huge. Need macro rework. 
#define RCVD_MAX_LOG_MSG_SIZE (MAX_LOG_MSG_SIZE + LOG_LABEL_SIZE)
#define UTC_TIME_SIZE    17   /* "YYYYMMDD_hhmmss |" */
#define DEFAULT_TIMEOUT  1000000   /* 1 sec for usleep */
#define MIN_TIMEOUT      506
#define MAX_TIMEOUT      1000000   /* 1 sec - longer will slow down i2v termination on exit */
#define READ_ERROR_TIMEOUT    MIN_TIMEOUT * 5
#define SIZE_WARNING_TIMEOUT   MIN_TIMEOUT * 10

#define IPV6_PREFIX_WORDS 4
#define MAX_IPV6_STRING_LENGTH 48  /* adds some extra space - up to 32 chars + : + / + scope */
#define LOG_NAME_LENGTH 45   /* PREFIX + YYYYMMDD_hhmmss--##########--MAJOR_VER_NUM.MINOR_VER_NUM.log */

/* this needs to update for any changes to log format */
#define MAJOR_LOG_VER  1
#define MINOR_LOG_VER  0

#if defined(EXTRA_DEBUG)
#define OUTPUT_MODULUS  30  /* Seconds. */
#else
#define OUTPUT_MODULUS  1245
#endif

/*
 * Externs
 */
extern int32_t getifclogoldest(char_t * filename);
extern int32_t getifclogdirsize(uint32_t * dirsize);
extern int32_t getifclogcount(void);
extern int32_t readifcrequest(ifcControl * thisreq);
extern int32_t ifclogCheckDirSize(uint32_t requestsize);
extern void *InterfaceLogThread(void __attribute__((unused)) *arg); /* ntcip-1218 */
extern void *lmgrIfcLogHandlerThr(void __attribute__((unused)) *arg);

/*
 * Globals
 */
lconfig lmgrCfg;
STATIC char_t confFileName[I2V_CFG_MAX_STR_LEN];
static char_t cfgdir[I2V_CFG_MAX_STR_LEN];
static char_t openfname[LOG_NAME_LENGTH];
bool_t mainloop = WTRUE;
static lmgrV4MaintMode v4maintenancemode = STATE_DEFAULT;
static pthread_mutex_t filelock = PTHREAD_MUTEX_INITIALIZER;
static FILE *logptr = NULL;
static uint32_t  timemaster = 0;
ifcControl masterIfcReq[I2V_NUM_SUPPORTED_IFCS];
pthread_mutex_t ifcreqlock = PTHREAD_MUTEX_INITIALIZER;
pid_t zprocs[MAX_CHILD_PROCS];  /* for child cleanup - prevent zombies */
static bool_t debugModeOn = WTRUE; /* Can't be turned on easily since started in systemd. */
static uint32_t logmgr_err_count = 0;/* Use to limit debug spew. */
STATIC uint64_t logmgr_error_states = 0x0;
static bool_t i2vSysLogGlobalControl = WTRUE;
static bool_t i2vGlobalDebugFlag = WTRUE;
uint32_t curtime_seconds = 0;

/* Add '8' to size: Stops warnings on sprintf conversion. There is no issue here.*/
static char_t timestamp[UTC_TIME_SIZE + 8 + 1];
static char_t timestamp_msg[UTC_TIME_SIZE + 8 + 1];

static uint32_t current_logfile_size = 0x0;
uint32_t current_logdir_size = 0x0;
static uint32_t max_syslogdir_size = 0x0;

static uint32_t locked_out=0;

/* 1218 MIB: 5.8 rsuInterfaceLogs. */
interfaceLog_t   interfaceLogTable[RSU_IFCLOG_MAX];
STATIC pthread_t InterfaceLogThreadID;
extern uint8_t  ifclogoptions;

/* 1218 MIB: 5.15 rsuSysSettings. */
STATIC sysSetts_t   SystemSettingsTable;
STATIC sysSetts_t * shm_systemSettings_ptr = NULL; /* SHM. */
STATIC pthread_t    SystemSettingsThreadID;
STATIC uint8_t      sysSetting_cmd_close = 0x0;

/* Recover last line of lpwrmgr log to SEL. Dump BuildInfo.txt and device ids to SEL.*/
#if defined(MY_UNIT_TEST)
#define GET_LPWR_LOG   "cat /tmp/rwflash/logs/Denso/pwrloss.log | tail -1"
#define GET_BUILD_INFO "cat /tmp/usr/share/BuildInfo.txt"
#define GET_ID_INFO    "/tmp/usr/local/dnutils/denso_nor_read | cut -c 9-"
#else
#define GET_LPWR_LOG   "cat /rwflash/logs/Denso/pwrloss.log | tail -1"
#define GET_BUILD_INFO "cat /usr/share/BuildInfo.txt"
#define GET_ID_INFO    "/usr/local/dnutils/denso_nor_read | cut -c 9-"
#endif

/* From lpwrmonitor.c LOGMSG_MAXLEN */
#define LPWR_LOG_MAX_LENGTH 256
STATIC int32_t dolpwrlog   = 0;

#define MAX_DEVICE_ID_ROWS 4
STATIC int32_t dodevids    = 0;

#define MAX_BUILDINFO_ROWS 9
STATIC int32_t dobuildinfo = 0;

int32_t ifcLogDirection = 0;
char_t  rsuname[RSU_ID_LENGTH_MAX];

STATIC int32_t syslog_enabled = 0;

/* Can't count on conf_agent being ready yet or snmpd. */
#define GET_SYSLOG_ENABLE "/bin/cat /rwflash/configs/i2v.conf | grep I2VDisableSysLog"
#define GET_SYSLOG_ENABLE_REPLY  "%s"

/*
 * Functions
 */
void set_my_error_state(int32_t my_error)
{
  int32_t dummy = 0;

    dummy = abs(my_error);

    if((dummy < 64) && (0 < dummy)) {
        logmgr_error_states |= (uint64_t)(0x1) << (dummy - 1);
    }
    return;
}

static void lmgrInitStatics(void) 
{
  memset(cfgdir,0x0,sizeof(cfgdir));
  memset(openfname,0x0,sizeof(openfname));
  mainloop = WTRUE;
  v4maintenancemode = STATE_DEFAULT;
  logptr = NULL;
  timemaster = 0;
  memset(&masterIfcReq, 0x0, sizeof(masterIfcReq));
  memset(zprocs,0x0, sizeof(zprocs));
  debugModeOn = WTRUE;
  logmgr_err_count = 0;
  logmgr_error_states = 0x0;
  i2vSysLogGlobalControl = WTRUE; /* Catch 22 - Can't wait for I2V. Override in conf if needed. */
  i2vGlobalDebugFlag = WTRUE;
  curtime_seconds = 0;
  memset(timestamp,'\0',sizeof(timestamp));
  memset(timestamp_msg,'\0',sizeof(timestamp_msg));
  current_logfile_size = 0x0;
  current_logdir_size = 0x0;
  max_syslogdir_size = 0x0;
  locked_out=0;
#if !defined(MY_UNIT_TEST) /* Select flavor in unit_test. */
    strncpy(confFileName,CONFFILE,sizeof(confFileName));
#endif
  dolpwrlog = 0;
  dodevids    = 0;
  dobuildinfo = 0;
  ifcLogDirection = 0;
  ifclogoptions = 0x0;
  memset(rsuname,0x0,sizeof(rsuname));
  syslog_enabled = 0;
}

/* Function to set up signal handlers */
static void lmgrSetupSigHandler(void (*handler)(int32_t, siginfo_t *, void *), int32_t sig)
{
    struct sigaction act;

    memset(&act, 0, sizeof(act));
    act.sa_sigaction = handler;
    sigemptyset(&act.sa_mask);
    // MFM QNX port SA_RESTART not supported on QNX act.sa_flags = SA_SIGINFO|SA_RESTART;
    act.sa_flags = SA_SIGINFO;
    sigaction(sig, &act, NULL);
}
/* IFCLOG slays child threads, so be careful enabling. */
#if defined(ENABLE_LOGMGR_CHILD_SIG)
/* SIGCHLD handler */
static void lmgrChildTermHandler(int32_t GCC_UNUSED_VAR a, siginfo_t *b, void GCC_UNUSED_VAR *c)
{
  #ifdef EXTRA_DEBUG
  I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"child gone (%d): (a) pid=%d, ex=%d\n", i2vUtilGetUTCTimeInSec(), b->si_pid, b->si_status);
  #endif
  set_my_error_state(LOGMGR_SIG_CHILD_HANDLER_EXIT);
}
#endif
/* Ideally LOGMGR last to close but no gurantee here. */
static void lmgrSigHandler(int __attribute__((unused)) sig)
{
  #ifdef EXTRA_DEBUG
  I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"lmgr dbg: caught term sig\n");
  #endif 
  set_my_error_state(LOGMGR_SIG_HANDLER_EXIT);
  mainloop = WFALSE;
//Question: Why wait?
  usleep(DEFAULT_TIMEOUT);
}
// Restores timemaster(when file opened) based off file name time.
/* this function is a safety net when used for time master - it should hopefully never run 
   if called with input params, then it is expected to run as it won't touch timemaster */
static void lmgrRestoreTimeMaster(char_t *namedfile, uint32_t *secval)
{
    char_t buf[5];
    uint8_t i; 
    char_t *ptr;
    struct tm tval;
    bool_t useTimeMaster = WTRUE;

    if (NULL == namedfile) {
        ptr = &openfname[PREFIX_LENGTH];
        set_my_error_state(LOGMGR_RESTORE_TIME_NULL_NAME);
    } else {
        ptr = namedfile;  /* assumption is input namedfile is YYYYMMDD_hhmmss string */
        useTimeMaster = WFALSE;
        set_my_error_state(LOGMGR_NO_TIMEMASTER);
    }
    memset(&tval, 0, sizeof(tval));
    memset(buf, 0, sizeof(buf));
    for (i=0; i<4; i++) buf[i] = ptr[i];   /* YYYY */
    tval.tm_year = (int32_t  )(strtoul(buf, NULL, 10) - 1900);
    ptr += 4;
    memset(buf, 0, sizeof(buf));
    for (i=0; i<2; i++) buf[i] = ptr[i];  /* MM */
    tval.tm_mon = (int32_t  )(strtoul(buf, NULL, 10) - 1);
    ptr += 2;
    memset(buf, 0, sizeof(buf));
    for (i=0; i<2; i++) buf[i] = ptr[i];   /* DD */
    tval.tm_mday = (int32_t  )(strtoul(buf, NULL, 10));
    ptr += 3;       /* _ */
    memset(buf, 0, sizeof(buf));
    for (i=0; i<2; i++) buf[i] = ptr[i];  /* hh */
    tval.tm_hour = (int32_t  )(strtoul(buf, NULL, 10));
    ptr += 2;
    memset(buf, 0, sizeof(buf));
    for (i=0; i<2; i++) buf[i] = ptr[i];  /* mm */
    tval.tm_min = (int32_t  )(strtoul(buf, NULL, 10));
    ptr += 2;
    memset(buf, 0, sizeof(buf));
    for (i=0; i<2; i++) buf[i] = ptr[i];  /* ss */
    tval.tm_sec = (int32_t  )(strtoul(buf, NULL, 10));

    mktime(&tval);
    if (useTimeMaster) {
        timemaster = CONVERT_TO_UTC((tval.tm_year + 1900), tval.tm_yday, tval.tm_hour, tval.tm_min, tval.tm_sec);
    } else {
        if (NULL != secval) *secval = CONVERT_TO_UTC((tval.tm_year + 1900), tval.tm_yday, tval.tm_hour, tval.tm_min, tval.tm_sec);
    }
}
static void lmgrAddHeader(FILE *f)
{
    /* log is a text file - format as follows 
  
        EVENT TIME    |   NAME    |LEVEL | DETAIL
     ********************************************* 
      YYYYMMDD_hhmmss | ABCDEFGHI | SEVR | Msg...
      YYYYMMDD_hhmmss | ABCDEFGHI | SEVR | Msg...
    */
    char_t str[RCVD_MAX_LOG_MSG_SIZE + UTC_TIME_SIZE + 8 + 1]; /* Larger than needed to avoid warning on sprintf. */

    if(1 == syslog_enabled) {
        snprintf(str,sizeof(str),"   EVENT TIME   |   NAME    |LEVEL | DETAIL\n");
        fwrite(str, sizeof(char), strnlen(str,sizeof(str)), f);
        snprintf(str,sizeof(str),"***************************************************\n");
        fwrite(str, sizeof(char), strnlen(str,sizeof(str)), f);
        fflush(f);
    }
}

//TODO: Make sure uptodate for 1218 rsuSysSettings and syslog queries from here.
//TODO: Untested.
/* send handler thread. If conf value bogus we set a default for user.*/
static void *lmgrSendHandlerThr(void __attribute__((unused)) *arg)
{
    char_t cmd[1024];  /* theoretical value, but really mostly unused */
    char_t ipaddr[MAX_IPV6_STRING_LENGTH];
    uint32_t ifc=0, curtime=0, oldtime=0;

    #if defined(EXTRA_DEBUG)
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"lmgrSendHandlerThr: Entry.\n");
    #endif
    memset(cmd,0x0,sizeof(cmd));
    memset(ipaddr,0x0,sizeof(ipaddr));

    if (!strnlen(lmgrCfg.scpuser,sizeof(lmgrCfg.scpuser))) strncpy(lmgrCfg.scpuser, "root",sizeof(lmgrCfg.scpuser));
    if (!strnlen(lmgrCfg.scpdir,sizeof(lmgrCfg.scpdir))) strncpy(lmgrCfg.scpdir, "/",sizeof(lmgrCfg.scpdir));
    if (lmgrCfg.xmitIpv4) {
        inet_ntop(AF_INET, lmgrCfg.ipv4addr, ipaddr, MAX_IPV6_STRING_LENGTH);
        snprintf(cmd, sizeof(cmd), "/usr/bin/scp %s/*.log %s@%s:%s >/dev/null 2>&1", lmgrCfg.logPath, lmgrCfg.scpuser, ipaddr, lmgrCfg.scpdir);
    } else {
        inet_ntop(AF_INET6, lmgrCfg.ipv6addr, ipaddr, MAX_IPV6_STRING_LENGTH);
        ifc = if_nametoindex("eth0");
        snprintf(cmd, sizeof(cmd), "/usr/bin/scp %s/*.log %s@[%s%%%d]:%s >/dev/null 2>&1", lmgrCfg.logPath, lmgrCfg.scpuser, ipaddr, ifc, lmgrCfg.scpdir);
    }

    #ifdef EXTRA_DEBUG
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"lmgr send cmd: %s\n", cmd);
    #endif

    oldtime = curtime = curtime_seconds;
    while (mainloop) {
        sleep(1); /* Once a second is fine for 5 minute intervals. */
        curtime = curtime_seconds;
//Question: why?
        lmgrCfg.xmitInterval = (lmgrCfg.xmitInterval) ? lmgrCfg.xmitInterval : 1;  /* memory corruption protection */
        if (((curtime - oldtime)/300) >= lmgrCfg.xmitInterval) {   /* 5 minute interval [60 sec * 5] */
            #ifdef EXTRA_DEBUG
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"lmgr executing transfer cmd at %d\n", curtime);
            #endif
            if(0 != system(cmd)){
#if defined(EXTRA_DEBUG)
                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"lmgr executing transfer cmd at %d FAILED. error=%d.\n", curtime, errno);
#endif
                set_my_error_state(LOGMGR_SYSTEM_FAIL);
            }
            oldtime = curtime;
        }
#if defined(MY_UNIT_TEST)
        mainloop = WFALSE;
#endif
    } /* while(mainloop). */

/* 
 * Shutting down, wrap it up. There is no controlled shutdown so disable this logic for now.
 * Question: This seems to belong to older standard but I don't see it in 1218.
 */
#if 0
    if (lmgrCfg.xmitShutdownEnable) {
//Question: why? 
        #ifdef EXTRA_DEBUG
        /* leave this print in here - timing issues observed when disabled
         *   I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"lmgr executing final command to transfer log data\n");
         */
        #endif
        if(0 != system(cmd)){
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"lmgr executing final command FAILED\n");
            set_my_error_state(LOGMGR_SYSTEM_FAIL);
        }
    }
#endif

    #ifdef EXTRA_DEBUG
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"lmgr dbg: out of mainloop\n");
    #endif
    set_my_error_state(LOGMGR_SEND_HANDLER_EXIT);
    pthread_exit(NULL);
}

/* msg handler thread */
/* FIXME: msg handler should wait for file handler
   if msg comes in before file handler ready then what?
   currently will display message and error - is that ok?
*/
static void *lmgrMsgHandlerThr(void __attribute__((unused)) *arg)
{
    mqd_t syslogQueue;
    struct mq_attr attr;
    int32_t  ret, ctr, pRet;
    bool_t  closeQueue = WTRUE;
    LogMsg msg;
#if defined(ENABLE_PEER_IFC_REQUESTS)
    uint8_t i = 0;
#endif
    
    locked_out = 0; /* If we are locked out, that means lost messages to syslogs. */

    // MFM - I attempted to port this to QNX name_attach and associated APIs, but if the
    // attach point already exists you get an error, and I found no way of deleting an
    // attach point. Using POSIX queue API.
    attr.mq_flags = 0;
    attr.mq_maxmsg = 256;
    attr.mq_msgsize = sizeof(LogMsg);
    attr.mq_curmsgs = 0;

    //
    //Purge old queue if exists.
    //
    syslogQueue = mq_open(I2V_LOGMSG_QUEUE, O_WRONLY);
    mq_unlink(I2V_LOGMSG_QUEUE);
    mq_close(syslogQueue);

    if ((syslogQueue = mq_open(I2V_LOGMSG_QUEUE, O_RDONLY | O_CREAT, 0644, &attr)) < 0) {
        mainloop = WFALSE;
        printf("%s: failed to open message queue: %s\n", MY_NAME, strerror(errno));
        set_my_error_state(LOGMGR_MSG_HANDLER_MQ_OPEN_FAIL);
        pthread_exit(NULL);
    }
    while (mainloop) {
#if defined(MY_UNIT_TEST)
        mainloop = WFALSE;
        continue;
#endif
        ret = mq_receive(syslogQueue, (char_t *)&msg, sizeof(msg), NULL);
        if (ret < 0) {/* Error condition, exit */
            set_my_error_state(LOGMGR_MSG_HANDLER_MQ_RX_FAIL);
            break;
        }
        if(0 == syslog_enabled) {
            continue;
        }
        /* Allow ifclog requests to continue if syslog disabled. */
        if(   (0 == i2vSysLogGlobalControl)
           && ((uint8_t)msg.entry[0] != LMGR_IFC_REQ_CMD)) { /* Keep emptying mq but do nothing. */
            usleep(1000); /* No rush. MQ can still have stuff in it so bleed out. */
            continue;
        }
        /* lock file lock and write to log file */
        ctr = 5;
        if ((uint8_t)msg.entry[0] == LMGR_IFC_REQ_CMD) {
#if defined(ENABLE_PEER_IFC_REQUESTS)
            /* overloaded byte - this is a request to activate or deactive ifc logging - nothing added to sys log */
            #ifdef EXTRA_DEBUG 
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"lmgr received interface log request - passing to interface thread\n");
            #endif
            while (pthread_mutex_trylock(&ifcreqlock) != 0) {
                usleep(550);
            }

//TODO: Seems to service like a stack not fifo. No implied order to pending. If we dont wait for completetion we can get surprise.
            for (i=0; i<I2V_NUM_SUPPORTED_IFCS; i++) {
                if (!masterIfcReq[i].reqPending) {
                    #if defined(EXTRA_DEBUG) 
                    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"lmgr: found free Req=%d: 0x%x, 0x%x\n",i,msg.entry[1],msg.entry[2]);
                    #endif
                    masterIfcReq[i].reqPending = WTRUE;
                    masterIfcReq[i].ifc = msg.entry[1];
                    masterIfcReq[i].request = msg.entry[2];
                    break;
                }
            }
            pthread_mutex_unlock(&ifcreqlock);
#endif
            continue;
        }
        /* Levels are in inverted priority. 0 is highest. */
        /* Legacy IFCLOG request first byte was 255(or something larger). */
        if ((msg.entry[0] - 1) > lmgrCfg.logPriority) {   /* value is incremented 1 to avoid null term leading byte */
            /* do not log - ignore message */
            continue;
        }
        while (((pRet=pthread_mutex_trylock(&filelock)) != 0) && ctr) {
            usleep(500); 
            ctr--;
        }
        if (pRet != 0) {
            if(locked_out < 0xFFFFFFFF) /* Dont roll. */
                locked_out++;

            #if defined(EXTRA_DEBUG)
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"error(%u) - log file locked and not accessible\n",locked_out);
            #endif

            #if defined(EXTRA_EXTRA_DEBUG)
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"log msg (%s)\n", &msg.entry[1]);
            #endif
            
            set_my_error_state(LOGMGR_MSG_HANDLER_FILE_LOCKED);
            continue;
        }
        if (NULL != logptr) {
            if(UTC_TIME_SIZE != fwrite(timestamp_msg, sizeof(char_t), UTC_TIME_SIZE, logptr))
                set_my_error_state(LOGMGR_FWRITE_FAIL);
            if(strnlen(&msg.entry[1],sizeof(msg.entry)-1) != fwrite(&msg.entry[1], sizeof(char), strnlen(&msg.entry[1],sizeof(msg.entry)-1), logptr))
                set_my_error_state(LOGMGR_FWRITE_FAIL);
            if(0 != fflush(logptr))
                set_my_error_state(LOGMGR_FWRITE_FAIL);
        } else {
            /* This is really dropped syslog entries, not lock fails. Ran out or error_states */
            if(locked_out < 0xFFFFFFFF) /* Dont roll. */
                locked_out++;

            #if defined(EXTRA_EXTRA_DEBUG) /* This can spew at hi rate, careful. */
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"error - log file not available\n");
            #endif

            #if defined(EXTRA_EXTRA_EXTRA_DEBUG) //careful this is a lot of data.
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"log msg (%s)\n", &msg.entry[1]);
            #endif
            
            set_my_error_state(LOGMGR_MSG_HANDLER_FILE_LOCKED);
        }
        pthread_mutex_unlock(&filelock);
#if defined(MY_UNIT_TEST)
        break; /* One and done. */
#endif
    }/* while(mainloop). */
    #ifdef EXTRA_DEBUG
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"msg handler closing log\n");
    #endif
     
    if (closeQueue) 
        mq_close(syslogQueue);
    set_my_error_state(LOGMGR_MSG_HANDLER_EXIT);
//Question: Is it the right call to take LOGMGR completely down?
    mainloop = WFALSE;
    pthread_exit(NULL);
}
/* Returns WTRUE only to force new file for size.
   When log dir is full, keep deleteing ANY oldest file till not full.
//TODO broken pipe
   Will get broken pipe till we have enough space to open log. 
*/
static bool_t lmgrCheckDirSize (int32_t *timeout)
{
    char_t cmd[1024];  /* NTCIP-1218 dir and file name lengths are huge. */
    FILE *f=NULL;
    char_t buf[11];  /* impractical to have MAX_INT for this function's usage */
    uint32_t size=0, s1=0, s2=0, s3=0;

    /* du reports kbytes. */
    *timeout = DEFAULT_TIMEOUT;
    memset(cmd,'\0',sizeof(cmd));
    memset(buf,'\0',sizeof(buf));
    snprintf(cmd, sizeof(cmd), "du %s 2>/dev/null | awk '{print $1}' > " TMPFILE, lmgrCfg.logPath);
    if(0 != system(cmd)){
#if defined(EXTRA_DEBUG)
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"lmgrCheckDirSize: du failed. errno=%d.\n",errno);
#endif
        set_my_error_state(LOGMGR_SYSTEM_FAIL);
        return WFALSE;
    }

    if ((f=fopen(TMPFILE, "r")) == NULL) {
        *timeout = READ_ERROR_TIMEOUT;
        #ifdef EXTRA_DEBUG
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"lmgrCheckDirSize: error reading log directory size. errno=%d.\n",errno);
        #endif
        set_my_error_state(LOGMGR_CHECKDIRSIZE_FOPEN_FAILED);
        return WFALSE;
    }

    if(fread(buf, sizeof(char_t), 10, f) < (sizeof(char_t) * 1)){ /* Must read at least one byte. */
        #ifdef EXTRA_DEBUG
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME," error on fread. errno=%d.\n",errno);
        #endif
        set_my_error_state(LOGMGR_CHECKDIRSIZE_FREAD_FAILED);
        fclose(f);
        return WFALSE;
    }
//TODO: With syslog we have to worry about deleting the file we are logging to.
//TODO: One file can't grow to the entire size by viture of conf setting?
//TODO: Only deletes 1 file without checking request size. 
//    : Looks weird if oldest files are all 1K since next log will be only 1k etc
    fclose(f);
    remove(TMPFILE);
    if (strnlen(buf,sizeof(buf))) {
        size = strtoul(buf, NULL, 0);  /* in kb */
        s1 = size * 1024; /* Convert kbytes to bytes. */
        s2 = lmgrCfg.dirsize * 1024 * 1024;  /* Convert from megs to bytes. */
        s3 = lmgrCfg.threshold * 1024 * 1024;
        current_logdir_size = s1;
        max_syslogdir_size = s2 - s3;
        if (s1 >= max_syslogdir_size) {
            snprintf(cmd,sizeof(cmd), "ls -rt %s 2>/dev/null | head -1 | grep -v %s", lmgrCfg.logPath,openfname);
            #if defined(EXTRA_DEBUG)
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"log directory past threshold: cmd=[%s]\n",cmd);
            #endif
            if (system(cmd)) {
                /* oldest file is the current one in use - implying log file with no size/age limit
                   - need to force close the log when within 1024 bytes of max dir size */
                if (s1 <= (s2 - 1024)) {
                    /* now within threshold but still have more max dir space - do nothing */
                    *timeout = SIZE_WARNING_TIMEOUT;
                    set_my_error_state(LOGMGR_CHECK_DIR_SIZE_WARNING_TO);
                    return WFALSE;
                } else {
                    #if defined(EXTRA_DEBUG)
                    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"unbound log being forced closed - max dir size reached\n");
                    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"WARNING - log will be deleted shortly\n");
                    #endif
                    *timeout = MIN_TIMEOUT;
                    set_my_error_state(LOGMGR_CHECK_DIR_MAX_SIZE_WARN);
                    return WTRUE;
                }
            } else {
                /* safety net with grep -v below */
                snprintf(cmd,sizeof(cmd), "rm -f %s/`ls -rt %s 2>/dev/null | head -1 | grep -v %s`", lmgrCfg.logPath, lmgrCfg.logPath, openfname);
                #if defined(EXTRA_DEBUG)
                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"log directory past threshold:Safety: cmd=[%s]\n",cmd);
                #endif
                if(0 != system(cmd)){
                    #if defined(EXTRA_DEBUG)
                    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME," rm failed:cmd[%s]. errno=%d.\n",cmd, errno);
                    #endif
                    set_my_error_state(LOGMGR_SYSTEM_FAIL);
                }
            }
        }
    } 
    return WFALSE;
}
/* return WTRUE to NOT change log file */
static bool_t lmgrCheckSizeandAge (int32_t *timeout, bool_t force)
{
    char_t cmd[1024];
    uint32_t s1=0, s2=0, curtime=0;
    struct stat buffer; 

    buffer.st_size = 0;
    memset(cmd,'\0',sizeof(cmd));
    *timeout = DEFAULT_TIMEOUT;
    snprintf(cmd,sizeof(cmd), "%s/%s",lmgrCfg.logPath, openfname);
    if(0 != stat(cmd, &buffer)){
        #if defined(EXTRA_DEBUG)
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"stat failed [%s]. errno=%d.\n",cmd, errno);
        #endif
        set_my_error_state(LOGMGR_CHECK_FILE_SIZE_STAT_FAIL);
    } else {
        current_logfile_size = (uint32_t)buffer.st_size;
        if ((lmgrCfg.logSize) && !force) {   /* 0 means size ignored - force overrides */
            s1 = (uint32_t)buffer.st_size;
            s2 = lmgrCfg.logSize * 1024 * 1024;
            if (s1 >= s2) {
                #if defined(EXTRA_DEBUG)
                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"log file max size reached.\n");
                #endif
                *timeout = MAX_TIMEOUT;
                set_my_error_state(LOGMGR_CHECK_SIZEAGE_MAX_HIT);
                return WFALSE;
            } else if (s1 >= s2 - 1024) {
                *timeout = SIZE_WARNING_TIMEOUT;
            } 
        } else {
            *timeout = READ_ERROR_TIMEOUT;
#ifdef EXTRA_DEBUG
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"error parsing log file size\n");
#endif /* EXTRA_DEBUG */
            set_my_error_state(LOGMGR_CHECK_SIZEAGE_PARSE_ERROR);
            return WTRUE;
        }
    }

//TODO: Doesn't detect jump in gnss time from no fix to fix.

    /* Check that timemaster isn't zero. If so restore from current filename. */
    if (!timemaster) {  
        /* this code should never be reached but just in case to avoid file overrun */
        #if defined(EXTRA_DEBUG)
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"WARNING - time tracker reset\n");
        #endif
        set_my_error_state(LOGMGR_TIMEMASTER_RESET);
        lmgrRestoreTimeMaster(NULL, NULL);
    }
    /* Check age of current file versus conf. */
    if (lmgrCfg.logDuration) {  /* 0 means age ignored */
        curtime = curtime_seconds;
        s1 = (curtime - timemaster) * 86400;
        if (s1 >= (uint32_t)lmgrCfg.logDuration) {
            #if defined(EXTRA_DEBUG)
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"log file age at maximum.\n");
            #endif
            *timeout = MAX_TIMEOUT;
            set_my_error_state(LOGMGR_CHECK_SIZEAGE_MAX);
            return WFALSE;
        }
    }

    return WTRUE;
}
#if 0
static void lmgrRenameLastLog(char_t *logname, int32_t  seqNum, bool_t incr)
{
    /* take input name and if incr is true add 1 to seconds [even if this creates
       a second value of 60 which would normally increment min as well] then
       rename last log file with newtimestamp */
    char_t cmd[1024] = {0};
    char_t newname[LOG_NAME_LENGTH] = {0};

    if(WFALSE == mainloop)
        return; /* Time to go, wrap it up. */

    snprintf(newname,sizeof(newname), PREFIX "%s--%.10d--%d.%d.log", timestamp, seqNum, MAJOR_LOG_VER, MINOR_LOG_VER);
    #ifdef EXTRA_DEBUG
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"new log name %s (%s)\n",newname, logname);
    #endif

    /* cmd: mv <logdir>/<logname> <logdir>/<newname> */ 
    snprintf(cmd,sizeof(cmd), "mv %s/%s %s/%s", lmgrCfg.logPath, logname, lmgrCfg.logPath, newname);
    
    if(0 != system(cmd)) {
        #if defined(EXTRA_DEBUG)
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"mv log failed. %s (%s)\n",newname, logname);
        #endif
        set_my_error_state(LOGMGR_SYSTEM_FAIL);
    }
}
#endif
static FILE *lmgrCreateNewLog(int seqNum)
{
    char_t fname[I2V_CFG_MAX_STR_LEN + 50] = {0};  /* extra space for file name */

    timemaster = curtime_seconds;
    snprintf(openfname,sizeof(openfname), PREFIX "%s--%.10d--%d.%d.log", 
        timestamp, seqNum, MAJOR_LOG_VER, MINOR_LOG_VER);
    snprintf(fname,sizeof(fname),"%s/%s", lmgrCfg.logPath, openfname);
    /* Update 1218 rsuSysSettings with current name and location of syslog. */
    if(strnlen(openfname,sizeof(openfname)) <= RSU_SYSLOG_NAME_AND_PATH_LENGTH_MAX){
        memcpy(SystemSettingsTable.rsuSysLogName, openfname, strnlen(openfname,sizeof(openfname)));
        SystemSettingsTable.rsuSysLogName_length = strnlen(openfname,sizeof(openfname));
    }
    if(strnlen(lmgrCfg.logPath,sizeof(lmgrCfg.logPath)) <= RSU_SYSDIR_NAME_LENGTH_MAX) {
        memcpy(SystemSettingsTable.rsuSysDir, lmgrCfg.logPath,strnlen(lmgrCfg.logPath,sizeof(lmgrCfg.logPath)) );
        SystemSettingsTable.rsuSysDir_length = strnlen(lmgrCfg.logPath,sizeof(lmgrCfg.logPath)) ;
    }
    return (fopen(fname, "w+"));
}
#if 0
static void lmgrUpdateIpv6Half(char_t GCC_UNUSED_VAR *tag, char_t GCC_UNUSED_VAR *parsedItem, char_t *value, void *configItem, uint32_t *status)
{
    char_t *tok;
    int32_t  i;
    uint16_t val;
    uint8_t *buf = (uint8_t *)configItem;

    tok = strtok(value, ":");
    for (i=0; i<IPV6_PREFIX_WORDS; i++) {
        if (NULL != tok) {
            val = (uint16_t)strtoul(tok, NULL, 16);
        } else {
            val = 0;
        }
        buf[i*2] = (uint8_t)(val >> 8);
        buf[i*2+1] = (uint8_t)val;
        tok = strtok(NULL, ":");
    }

    *status = I2V_RETURN_OK;
}
static void lmgrUpdateIpv4(char_t GCC_UNUSED_VAR *tag, char_t GCC_UNUSED_VAR *parsedItem, char_t *value, void *configItem, uint32_t *status)
{
    char_t *tok;
    uint8_t *buf = (uint8_t *)configItem;
    uint8_t i;

    tok = strtok(value, ".");
    for (i=0; i<LENGTH_IPV4; i++) {
        if (NULL != tok) {
            buf[i] = (uint8_t)strtoul(tok, NULL, 0);
        } else {
            buf[i] = (uint8_t)0;
        }
        tok = strtok(NULL, ".");
    }

    *status = I2V_RETURN_OK;
}
#endif
/* this function provides the RSU functional spec V4 log maintenance 
   requirements of closing and opening a log at a required time each week
   and deleting logs 4 weeks old */
/* NOTE: the RSU Functional spec V4 requirements define a period of time
   that is minimum 1 min where system logs will be disabled - this is 
   by design */
static bool_t lmgrCheckV4Maintenaince (int32_t *timeout, int32_t  *seqNum)
{
    i2vTimeT clock;
    char_t cmd[1024] = {0};
    int32_t  clockdescriptor = 0;

    *timeout = DEFAULT_TIMEOUT;
    i2vUtilGetUTCTime(&clock);
    clockdescriptor = clock.wday * 24 * 60 + clock.hour * 60 + clock.min;
    if ((clockdescriptor >= 1439) && (clockdescriptor <= 1450)) {
        /* this works out to only (D.HH.MM):  0.23.59 < descriptor < 1.0.10 which is 
           Sunday 11:59 pm to Monday 12:10 am defined in spec v4 */
        v4maintenancemode = (v4maintenancemode == STATE_DEFAULT) ? STATE_STARTING : v4maintenancemode;
#ifdef EXTRA_DEBUG
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"lmgr in maintenaince mode (%d)\n", v4maintenancemode);
#endif /* EXTRA_DEBUG */
        set_my_error_state(LOGMGR_MAINT_MODE_ENABLE);
    } else {
#ifdef EXTRA_EXTRA_DEBUG
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"lmgr not in maintenaince mode\n");
#endif /* EXTRA_DEBUG */
        v4maintenancemode = STATE_DEFAULT;
        if (NULL != logptr) {   /* thread that assigns logptr is this one - so no mutex needed for this test */
            fflush(logptr);
#if defined(EXTRA_EXTRA_DEBUG)
            set_my_error_state(LOGMGR_MAINT_MODE_STATE_DEFAULT_NO_FILE);
#endif
            return WFALSE;   /* no other processing needed */
        }
#if defined(EXTRA_EXTRA_DEBUG)
        set_my_error_state(LOGMGR_MAINT_MODE_STATE_DEFAULT);
#endif
    }
    if ((clock.wday == 0) && (clock.hour == 23) && (clock.min == 59)) {
        /* at 23:59 close log file - then return WTRUE to wait until Monday 0:00 to create new log */
        while (pthread_mutex_trylock(&filelock) != 0) {
#ifdef EXTRA_DEBUG
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"%s: file manager waiting for lock to create new log file\n", __FUNCTION__);
#endif
            usleep(560); /* offset of extra 60 us for sequencing */
        }
        if (NULL != logptr) {
            fflush(logptr);
            fclose(logptr);
//why?
#if 0
            lmgrRenameLastLog(openfname, *seqNum, WFALSE);
#endif
            logptr = NULL;   /* de-assign pointer! */
#ifdef EXTRA_DEBUG
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"%s: maintenance mode for Sunday 11:59 pm log close\n", __FUNCTION__);
#endif
            set_my_error_state(LOGMGR_MAINT_MODE_FILE_CLOSE_NO_FILE);
        }
        pthread_mutex_unlock(&filelock);
        set_my_error_state(LOGMGR_MAINT_MODE_FILE_CLOSE); 
        return WTRUE;
    }
    /* no longer 11:59 when code reaches this point so if logptr is null generate new file */
    while (pthread_mutex_trylock(&filelock) != 0) {
        usleep(560);
    }
    if (NULL == logptr) {
        logptr = lmgrCreateNewLog(++*seqNum);
#ifdef EXTRA_DEBUG
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"%s: maintenaince mode created new log file\n", __FUNCTION__);
#endif
        set_my_error_state(LOGMGR_MAINT_MODE_NEW_LOG);
    }
    pthread_mutex_unlock(&filelock);

    /* at Mon 0:10 remove files 4 weeks old */
    if ((v4maintenancemode != STATE_CLEANED) && (clock.wday == 1) && (clock.hour == 0) && (clock.min == 10)) {
        /* 4 wks = 28 days: cmd - find <dir>'/'* -mtime +28 -exec rm {} \; */
        /* 4 wks defined in spec - switching to use cfg value */
        snprintf(cmd,sizeof(cmd), "rm -f `find %s/" PREFIX "* -mtime +%d`", lmgrCfg.logPath, lmgrCfg.logRmvAgeThresh);
#ifdef EXTRA_DEBUG
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"cleanup cmd - %s\n",cmd);
#endif
        if(0 != system(cmd)) {
            #if defined(EXTRA_DEBUG)
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"cleanup cmd FAILED - %s\n",cmd);
            #endif
            set_my_error_state(LOGMGR_SYSTEM_FAIL);
        }
        v4maintenancemode = STATE_CLEANED;
        set_my_error_state(LOGMGR_MAINT_MODE_STATE_CLEANED);
    }

    return WFALSE;
}

/* log file handler thread */
static void *lmgrFileHandlerThr(void __attribute__((unused)) *arg)
{
    /* required actions: 
       # check file size and age against config - if file is 
         at either threshold create new file
       # check log file directory size - if approaching max size
         avail, delete old file(s)

       rule for new file name - timestamp in name, seq num (up to max_int hex)
       and must create log file header */
    char_t cmd[1024] = {0};  /* accounting for I2V_CFG_MAX_STR_LEN */
    FILE *f;
    char_t buf[11] = {0};   /* MAX_INT + '0' */
    int32_t  seq = 0;
    int32_t  timeout = DEFAULT_TIMEOUT;
    int32_t  secondaryto = DEFAULT_TIMEOUT;
    bool_t ret;
    char_t interimNm[LOG_NAME_LENGTH];

    if (!i2vCheckDirOrFile(lmgrCfg.logPath)) {
        /* attempt once to create directory */
        #ifdef EXTRA_DEBUG
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"log directory not found - attempting to create\n");
        #endif
        if (I2V_RETURN_OK != i2vUtilMakeDir(lmgrCfg.logPath)) {
            mainloop = WFALSE;
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"system log directory inaccessible\n");
            set_my_error_state(LOGMGR_FILE_HANDLER_DIR_CREATE_FAIL);
            pthread_exit(NULL);
        }
    }
    snprintf(cmd,sizeof(cmd), "ls -t %s 2>/dev/null | grep " PREFIX " | head -1 | sed 's/.*--\\(.*\\)--.*/\\1/' > %s", lmgrCfg.logPath, TMPFILE);
    #ifdef EXTRA_DEBUG
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"lmgr dbg: finding contents (%s)\n", cmd);
    #endif
    if(0 != system(cmd)){
        #if defined(EXTRA_DEBUG)
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"lmgr dbg: finding contents FAILED (%s). errno=%d.\n", cmd, errno);
        #endif
        set_my_error_state(LOGMGR_SYSTEM_FAIL);
    }
    usleep(1000);
    if ((f=fopen(TMPFILE, "r")) == NULL) {
        mainloop = WFALSE;
        #if defined(EXTRA_DEBUG)
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"error initializing log directory\n");
        #endif
        set_my_error_state(LOGMGR_FOPEN_FAIL);
        pthread_exit(NULL);
    }

    if(fread(buf, sizeof(char_t), 10, f) < 1) {
        #ifdef EXTRA_DEBUG
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"lmgrFileHandlerThr: error fread failed on(%s). errno=%d.\n", cmd, errno);
        #endif
    }
    fclose(f);
    remove(TMPFILE);
    if (strnlen(buf,sizeof(buf))) {
        seq = (int32_t)strtoul(buf, NULL, 10);
    }
    seq++;

    lmgrCheckDirSize(&timeout);  /* will blow away oldest log if near threshold */

    /* create new log file */
    f = lmgrCreateNewLog(seq);
    if (NULL == f) {
        mainloop = WFALSE;
        #if defined(EXTRA_DEBUG)
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"failed to create log file\n");
        #endif
        set_my_error_state(LOGMGR_FILE_HANDLER_CREATE_FAIL);
        pthread_exit(NULL);
    }
    lmgrAddHeader(f);

    while (pthread_mutex_trylock(&filelock) != 0) {
        /* hang out here until lock freed */
        #ifdef EXTRA_DEBUG
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"file manager waiting for log lock\n");
        #endif
        usleep(1060);  /* 1 ms - offset of extra 60 us for sequencing */
    }
    logptr = f;
    pthread_mutex_unlock(&filelock);
    f = NULL;   /* avoid dangling pointer to log file */
    while (mainloop) {
        strncpy(interimNm, openfname,sizeof(interimNm));
        if (lmgrCheckV4Maintenaince(&timeout, &seq)) {
            #ifdef EXTRA_DEBUG
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"maint mode sleep\n");
            #endif
            usleep(timeout);
            continue;
        }
        ret = lmgrCheckDirSize(&timeout);
        if (   (!lmgrCheckSizeandAge(&secondaryto, ret))
            || (0x1 == sysSetting_cmd_close)) {
            sysSetting_cmd_close = 0x0;
            /* create new file first then update ptr - exposure with this approach
              is there is a chance of having a blank log file as the latest if
              i2v terminates - but this can happen anyway - this approach limits
              actions done in mutex */
            f = lmgrCreateNewLog(++seq);
            if (NULL == f) {
                mainloop = WFALSE; /* This is fatal for LOGMGR but not I2V. Only LOGMGR will terminate. */
                #if defined(EXTRA_DEBUG)
                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"system logging failed\n");
                #endif
                if (NULL != logptr) {
                    fflush(logptr);
                    fclose(logptr);
//Question: Why? If it's broken then proceeding seems risky?
#if 0
                    lmgrRenameLastLog(interimNm, --seq, WFALSE);
#endif
                }
                set_my_error_state(LOGMGR_FILE_HANDLER_CREATE_FAIL);
                pthread_exit(NULL);
            }
            lmgrAddHeader(f);       
            while (pthread_mutex_trylock(&filelock) != 0) {
                #ifdef EXTRA_DEBUG
                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"file manager waiting for lock to create new log file\n");
                #endif
                usleep(560); /* offset of extra 60 us for sequencing */
            }
            fflush(logptr);
            fclose(logptr);
            logptr = f;
            pthread_mutex_unlock(&filelock);
#if 0
            lmgrRenameLastLog(interimNm, (seq - 1), WTRUE);
#endif
            f = NULL;
        }
// Question: What is this trying to ensure?
        secondaryto = (secondaryto >= MIN_TIMEOUT) ? secondaryto : MIN_TIMEOUT;
        secondaryto = (secondaryto < MAX_TIMEOUT) ? secondaryto : MAX_TIMEOUT;
        timeout = (timeout >= MIN_TIMEOUT) ? timeout : MIN_TIMEOUT;
        timeout = (timeout < MAX_TIMEOUT) ? timeout : MAX_TIMEOUT;
        timeout = (timeout < secondaryto) ? timeout : secondaryto;
        usleep(timeout);
#if defined(MY_UNIT_TEST)
        mainloop = WFALSE;
#endif
    } /* while(mainloop). */

    #ifdef EXTRA_DEBUG
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"file handler closing log\n");
    #endif

    if (NULL != logptr) {
        fflush(logptr);
        fclose(logptr);
        logptr = NULL;
    }
//Question: we are shutting down so why?
#if 0
    lmgrRenameLastLog(openfname, seq, WFALSE);
#endif
    set_my_error_state(LOGMGR_FILE_HANDLER_EXIT);
    pthread_exit(NULL);
}

STATIC uint32_t logmgrSetConfDefault(char_t * tag, void * itemToUpdate)
{
  uint32_t ret = I2V_RETURN_OK; /* Success till proven fail. */
  if((NULL == tag) || (NULL == itemToUpdate)) {
      /* Nothing to do. */
      #if defined(EXTRA_DEBUG)
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"logmgrSetConfDefault: NULL input.\n");
      #endif
      set_my_error_state(LOGMGR_INPUT_NULL);
      ret = I2V_RETURN_NULL_PTR;
  } else {
      if(0 == strcmp("MaxLogLifetime",tag)) {
          *(uint16_t *)itemToUpdate = LOGMGR_LOG_LIFE_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%u)\n",tag,*(uint16_t *)itemToUpdate);
      } else if(0 == strcmp("MaxLogSize",tag)) {
          *(uint8_t *)itemToUpdate = LOGMGR_LOG_SIZE_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%u)\n",tag,*(uint8_t *)itemToUpdate);
      } else if(0 == strcmp("LogMinPriority",tag)) {
          *(uint8_t *)itemToUpdate = LOGMGR_LOG_MIN_PRIORITY_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%u)\n",tag,*(uint8_t *)itemToUpdate);
      } else if(0 == strcmp("LogDir",tag)) {
//this did not heal? I guess since its a string what can we do?  ../stubs is valid to set
          strcpy((char_t *)itemToUpdate, LOGMGR_LOG_DIR_DEFAULT_S);
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%s)\n",tag,(char_t *)itemToUpdate);
      } else if(0 == strcmp("LogDirSize",tag)) {
          *(uint16_t *)itemToUpdate = LOGMGR_LOG_DIR_SIZE_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%u)\n",tag,*(uint16_t *)itemToUpdate);
      } else if(0 == strcmp("LogDirThreshold",tag)) {
          *(uint8_t *)itemToUpdate = LOGMGR_LOG_DIR_THRESHOLD_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%u)\n",tag,*(uint8_t *)itemToUpdate);
      } else if(0 == strcmp("LogFileDuration",tag)) {
          *(uint16_t *)itemToUpdate = LOGMGR_LOG_DURATION_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%u)\n",tag,*(uint16_t *)itemToUpdate);
      } else if(0 == strcmp("LogXmitHaltEnable",tag)) {
          *(bool_t *)itemToUpdate = LOGMGR_LOG_XMIT_ONHALT_ENABLE_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%u)\n",tag,*(bool_t *)itemToUpdate);
      } else if(0 == strcmp("LogXmitInterval",tag)) {
          *(uint16_t *)itemToUpdate = LOGMGR_LOG_XMIT_INTERVAL_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%u)\n",tag,*(uint16_t *)itemToUpdate);
      } else if(0 == strcmp("LogXmitUseIPv4",tag)) {
          *(bool_t *)itemToUpdate = LOGMGR_LOG_XMIT_USE_IPV4_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%u)\n",tag,*(bool_t *)itemToUpdate);
      } else if(0 == strcmp("InterfaceLogMaxSize",tag)) {
          *(uint8_t *)itemToUpdate = LOGMGR_IFACE_LOG_SIZE_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%u)\n",tag,*(uint8_t *)itemToUpdate);
      } else if(0 == strcmp("InterfaceLogMaxLife",tag)) {
          *(uint32_t *)itemToUpdate = LOGMGR_IFACE_LIFE_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%u)\n",tag,*(uint32_t *)itemToUpdate);
      } else if(0 == strcmp("ServerMgrUser",tag)) {
          strcpy((char_t *)itemToUpdate, LOGMGR_SVR_MGR_USER_DEFAULT_S);
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%s)\n",tag,(char_t *)itemToUpdate);
      } else if(0 == strcmp("ServerMgrDirectory",tag)) {
          strcpy((char_t *)itemToUpdate, LOGMGR_SVR_MGR_DIR_DEFAULT_S);
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%s)\n",tag,(char_t *)itemToUpdate);
#if 0
      } else if(0 == strcmp("ServerMgrIPv4",tag)) {
          strcpy((char_t *)itemToUpdate, LOGMGR_SVR_MGR_IPV4_DEFAULT_S);
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%s)\n",tag,(char_t *)itemToUpdate);
      } else if(0 == strcmp("ServerMgrIPv6Prefix",tag)) {
          strcpy((char_t *)itemToUpdate, LOGMGR_SVR_MGR_IPV6_PREFIX_DEFAULT_S);
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%s)\n",tag,(char_t *)itemToUpdate);
      } else if(0 == strcmp("ServerMgrIPv6ID",tag)) {
          strcpy((char_t *)itemToUpdate, LOGMGR_SVR_MGR_IPV6_ID_DEFAULT_S);
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%s)\n",tag,(char_t *)itemToUpdate);
#endif  
      } else {
          /* Nothing to do. */
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"logmgrSetConfDefault:(%s) item is not known, ignoring.\n",tag);
          set_my_error_state(LOGMGR_HEAL_FAIL);
          ret = I2V_RETURN_UNKNOWN;
      }
  }
  return ret;
}
static bool_t lmgrUpdateCfg(void)
{
    cfgItemsTypeT cfgItems[] = {
        {"MaxLogLifetime",            (void *)i2vUtilUpdateUint16Value,   &lmgrCfg.logDuration,     NULL,(ITEM_VALID| UNINIT)},
        {"MaxLogSize",                (void *)i2vUtilUpdateUint8Value,    &lmgrCfg.logSize,         NULL,(ITEM_VALID| UNINIT)},
        {"LogMinPriority",            (void *)i2vUtilUpdateUint8Value,    &lmgrCfg.logPriority,     NULL,(ITEM_VALID| UNINIT)},
        {"LogDir",                    (void *)i2vUtilUpdateStrValue,      &lmgrCfg.logPath,         NULL,(ITEM_VALID| UNINIT)},
        {"LogDirSize",                (void *)i2vUtilUpdateUint16Value,   &lmgrCfg.dirsize,         NULL,(ITEM_VALID| UNINIT)},
        {"LogDirThreshold",           (void *)i2vUtilUpdateUint8Value,    &lmgrCfg.threshold,       NULL,(ITEM_VALID| UNINIT)},
        {"LogFileDuration",           (void *)i2vUtilUpdateUint16Value,   &lmgrCfg.logRmvAgeThresh, NULL,(ITEM_VALID| UNINIT)},
        {"LogXmitHaltEnable",         (void *)i2vUtilUpdateWBOOLValue,    &lmgrCfg.xmitShutdownEnable, NULL,(ITEM_VALID| UNINIT)},
        {"LogXmitInterval",           (void *)i2vUtilUpdateUint16Value,   &lmgrCfg.xmitInterval,    NULL,(ITEM_VALID| UNINIT)},
        {"LogXmitUseIPv4",            (void *)i2vUtilUpdateWBOOLValue,    &lmgrCfg.xmitIpv4,        NULL,(ITEM_VALID| UNINIT)},
        {"InterfaceLogMaxSize",       (void *)i2vUtilUpdateUint8Value,    &lmgrCfg.ifcLogSizeMax,   NULL,(ITEM_VALID| UNINIT)},
        {"InterfaceLogMaxLife",       (void *)i2vUtilUpdateUint32Value,   &lmgrCfg.ifcLogDuration,  NULL,(ITEM_VALID| UNINIT)},
        {"ServerMgrUser",             (void *)i2vUtilUpdateStrValue,      &lmgrCfg.scpuser,         NULL,(ITEM_VALID| UNINIT)},
        {"ServerMgrDirectory",        (void *)i2vUtilUpdateStrValue,      &lmgrCfg.scpdir,          NULL,(ITEM_VALID| UNINIT)},
#if 0 /* Untested. */ 
        {"ServerMgrIPv4",             NULL,                       &lmgrCfg.ipv4addr,        (void *)lmgrUpdateIpv4,(ITEM_VALID| UNINIT)},
        {"ServerMgrIPv6Prefix",       NULL,                       &lmgrCfg.ipv6addr,        (void *)lmgrUpdateIpv6Half,(ITEM_VALID| UNINIT)},
        {"ServerMgrIPv6ID",           NULL,                       &lmgrCfg.ipv6addr[IPV6_PREFIX_WORDS * 2],(void *)lmgrUpdateIpv6Half,(ITEM_VALID| UNINIT)},
#endif
    };
    FILE *f = NULL;
    char_t fname[I2V_CFG_MAX_STR_LEN + 20];  /* space for CONFFILE */
    bool_t ret = WTRUE; /* true till proven false. */
    uint32_t i = 0;

    /* 
     * If you want output from I2V_DBG_LOG you must tell i2v_util.c that syslog & serial debug are open.
     * True for all tasks.
     */

    /* If we fail to get I2V SHM update then fail. */
    if(WFALSE == mainloop) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"mainloop == WFALSE: ret=%d\n",ret);
        set_my_error_state(LOGMGR_GET_I2V_CFG_SHM_FAIL);
        ret = WFALSE;
    } else {
        snprintf(fname,sizeof(fname), "%s/%s", cfgdir, confFileName);
        if ((f=fopen(fname, "r")) == NULL) {
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"lmgrUpdateCfg: failed to open (%s): err=%d %s\n", fname, errno, strerror(errno));
            set_my_error_state(LOGMGR_OPEN_CFG_FAIL);
            ret = WFALSE;
        }
    }

    if(WTRUE == ret) {
        if (I2V_RETURN_OK != i2vUtilParseConfFile(f, cfgItems, NUMITEMS(cfgItems), WFALSE, NULL)) {
            I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME, "failed to read required config values.\n");
            ret = WFALSE;
            set_my_error_state(LOGMGR_READ_CFG_FAIL);
        }
        for (i = 0; i < NUMITEMS(cfgItems); i++) {
            if (   ((ITEM_VALID|BAD_VAL) == cfgItems[i].state) 
                || ((ITEM_VALID|UNINIT) == cfgItems[i].state)) {
                I2V_DBG_LOG(LEVEL_CRIT,MY_NAME,"config override on (%s): Using default.\n", cfgItems[i].tag);
                if(I2V_RETURN_OK == logmgrSetConfDefault(cfgItems[i].tag, cfgItems[i].itemToUpdate)) {
                    cfgItems[i].state = (ITEM_VALID|INIT);
                    ret =  WTRUE;
                    set_my_error_state(LOGMGR_HEAL_CFGITEM);
                } else {
                    ret = WFALSE;
                    set_my_error_state(LOGMGR_HEAL_FAIL);
                    I2V_DBG_LOG(LEVEL_CRIT,MY_NAME,"config override on (%s): Failed.\n", cfgItems[i].tag);
                    break; /* Heal has failed. FATAL. */
                }
            }
        }
    }
    if(NULL != f) fclose(f);
    return ret;
}
void *SystemSettingsThread(void __attribute__((unused)) *arg)
{
  uint32_t rolling_counter = 0x0;

  memset(&SystemSettingsTable, 0x0, sizeof(SystemSettingsTable));
  sysSetting_cmd_close = 0x0;

  /* Open SHM. */
  if ((shm_systemSettings_ptr = wsu_share_init(sizeof(sysSetts_t), SYS_SETTS_SHM_PATH)) == NULL) {
      #if defined(EXTRA_DEBUG)
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"SystemSettingsThr: SHM init failed.\n");
      #endif
      set_my_error_state(LOGMGR_INIT_SHM_FAIL);
  } else {
      while(mainloop) {
          /*
           * Check for updates in SHM table from MIB.
           */
          if(1 == shm_systemSettings_ptr->dirty_syslog) {
//TODO: LOGMGR does not use start/stop. MIB doesn't use size & time of file or dir size.
              SystemSettingsTable.start_utc_dsec = shm_systemSettings_ptr->start_utc_dsec;
              SystemSettingsTable.stop_utc_dsec = shm_systemSettings_ptr->stop_utc_dsec;
              SystemSettingsTable.rsuSysLogQueryPriority = shm_systemSettings_ptr->rsuSysLogQueryPriority; 
              SystemSettingsTable.rsuSysLogQueryGenerate = shm_systemSettings_ptr->rsuSysLogQueryGenerate; 
              //SystemSettingsTable.rsuSysLogQueryStatus = shm_systemSettings_ptr->rsuSysLogQueryStatus;
              SystemSettingsTable.rsuSysLogCloseCommand = shm_systemSettings_ptr->rsuSysLogCloseCommand; 
              SystemSettingsTable.rsuSysLogSeverity = shm_systemSettings_ptr->rsuSysLogSeverity;

              if(1 == shm_systemSettings_ptr->rsuSysLogQueryGenerate){
                  /* start-restart logging. */
#if 0
                  i2vSysLogGlobalControl = WTRUE;
#endif
                  SystemSettingsTable.rsuSysLogQueryStatus = syslog_status_progressing;
              } else {
#if 0
                  i2vSysLogGlobalControl = WFALSE;
#endif
                  SystemSettingsTable.rsuSysLogQueryStatus = syslog_status_successful;
              }
              if(1 == shm_systemSettings_ptr->rsuSysLogCloseCommand){
                  /* Close current logfile and stop logging. */
#if 0
                  sysSetting_cmd_close = 0x1;
#endif
                  shm_systemSettings_ptr->rsuSysLogQueryGenerate = SystemSettingsTable.rsuSysLogQueryGenerate; 
                  SystemSettingsTable.rsuSysLogCloseCommand = shm_systemSettings_ptr->rsuSysLogCloseCommand = 0;
                  SystemSettingsTable.rsuSysLogQueryGenerate = shm_systemSettings_ptr->rsuSysLogQueryGenerate = 0;
                  SystemSettingsTable.rsuSysLogQueryStatus = syslog_status_successful;
#if 0
                  i2vSysLogGlobalControl = WFALSE;
#endif
                   
              }
              shm_systemSettings_ptr->dirty_syslog = SystemSettingsTable.dirty_syslog = 0;
          }
          /* 
           * Maintenance portion. Update SHM for MIB. 
           */
//TODO: Check start/stop times.
          if(i2vSysLogGlobalControl == WTRUE)
              SystemSettingsTable.rsuSysLogQueryGenerate = 1;
          else
              SystemSettingsTable.rsuSysLogQueryGenerate = 0;
          shm_systemSettings_ptr->rsuSysLogQueryGenerate = SystemSettingsTable.rsuSysLogQueryGenerate;
          if(1 == SystemSettingsTable.rsuSysLogQueryGenerate) {
          memcpy(&shm_systemSettings_ptr->rsuSysLogName, SystemSettingsTable.rsuSysLogName,RSU_SYSLOG_NAME_AND_PATH_LENGTH_MAX);
          shm_systemSettings_ptr->rsuSysLogName_length = SystemSettingsTable.rsuSysLogName_length;
          memcpy(&shm_systemSettings_ptr->rsuSysDir, SystemSettingsTable.rsuSysDir,RSU_SYSDIR_NAME_LENGTH_MAX);
          shm_systemSettings_ptr->rsuSysDir_length = SystemSettingsTable.rsuSysDir_length;
          }
          shm_systemSettings_ptr->rsuSysLogQueryStatus = SystemSettingsTable.rsuSysLogQueryStatus;
          #if defined(EXTRA_DEBUG)
          if(0 == (rolling_counter % 120)) {
              I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"syslog: dirty=%u,gen=%d,close=%d, start/stop(0x%lx,0x%lx),pri=%d,stat=%d,severity=%d.\n",
                  SystemSettingsTable.dirty_syslog
                  ,SystemSettingsTable.rsuSysLogQueryGenerate
                  ,SystemSettingsTable.rsuSysLogCloseCommand      
                  ,SystemSettingsTable.start_utc_dsec
                  ,SystemSettingsTable.stop_utc_dsec
                  ,SystemSettingsTable.rsuSysLogQueryPriority 
                  ,SystemSettingsTable.rsuSysLogQueryStatus
                  ,SystemSettingsTable.rsuSysLogSeverity);
              I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"syslog: dir[%s], name[%s]\n",
                  SystemSettingsTable.rsuSysDir,SystemSettingsTable.rsuSysLogName);
          }
          #endif
          rolling_counter++;
          sleep(1);
#if defined(MY_UNIT_TEST)
        mainloop = WFALSE;
#endif
      } /* while */
  }
  pthread_exit(NULL);
}
/*
 * Get BuildInfo.txt for SEL.
 */
void grabbuildinfo(void)
{
  int32_t   i = 0;
  int32_t   j = 0;
  FILE    * fp = NULL;
  char_t    replyBuf[MAX_LOG_MSG_SIZE]; /* Size according to your needs. */

  if(0 == dobuildinfo) {
      dobuildinfo=1;
      memset(replyBuf,'\0',sizeof(replyBuf));
      if(NULL != (fp = POPEN(GET_BUILD_INFO, "r"))) {
          for(i=0; (i<MAX_BUILDINFO_ROWS) && (NULL != fgets(replyBuf, sizeof(replyBuf), fp));i++){
              if(0 < (j = strnlen(replyBuf,sizeof(replyBuf)))) {
                  replyBuf[j-1] = '\0'; /* Delete extra char added. */
              }
              I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"BuildInfo[%s]\n",replyBuf);
              usleep(1000); /* Dont hammer on syslog */
          } 
          if(i < MAX_BUILDINFO_ROWS) {
              I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"BuildInfo: FGETS FAIL: rows(%d) < (%d) errno(%s)\n",i,MAX_BUILDINFO_ROWS,strerror(errno));
              set_my_error_state(LOGMGR_FGETS_FAIL);  
          }
          PCLOSE(fp);
      } else {
          I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"BuildInfo: POPEN FAIL: errno(%s)\n",strerror(errno));
          set_my_error_state(LOGMGR_POPEN_FAIL); 
      }
  }
}
/*
 * Get Device & Customer rsuIDs for SEL.
 */
void grabdeviceids(void)
{
  int32_t   i = 0;
  int32_t   j = 0;
  FILE    * fp = NULL;
  char_t    replyBuf[MAX_LOG_MSG_SIZE]; /* Size according to your needs. */

  if(0 == dodevids) {
      dodevids=1;
      memset(replyBuf,'\0',sizeof(replyBuf));
      if(NULL != (fp = POPEN(GET_ID_INFO, "r"))) {
          for(i=0;i<MAX_DEVICE_ID_ROWS && (NULL != fgets(replyBuf, sizeof(replyBuf), fp));i++){
              if(0 < (j = strnlen(replyBuf,sizeof(replyBuf)))) {
                  replyBuf[j-1] = '\0'; /* Delete extra char added. */
              }
              I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"grabdeviceids[%s]\n",replyBuf);
              usleep(1000); /* Dont hammer on syslog */
          } 
          if(i < MAX_DEVICE_ID_ROWS) {
              I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"grabdeviceids: FGETS FAIL: rows(%d) < (%d) errno(%s)\n",i,MAX_DEVICE_ID_ROWS,strerror(errno));
              set_my_error_state(LOGMGR_FGETS_FAIL);  
          }
          PCLOSE(fp);
      } else {
          I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"grabdeviceids: POPEN FAIL: errno(%s)\n",strerror(errno));
          set_my_error_state(LOGMGR_POPEN_FAIL); 
      }
  }
}
/*
 * Recovery lpwrmgr log last entry for SEL.
 * Assume LPWR_LOG_MAX_LENGTH < MAX_LOG_MSG_SIZE.
 */
void grablpwrlog(void)
{
  int32_t   i = 0;
  FILE    * fp = NULL;
  char_t    replyBuf[MAX_LOG_MSG_SIZE]; /* Size according to your needs. */

  if(0 == dolpwrlog) {
      dolpwrlog=1;
      memset(replyBuf,'\0',sizeof(replyBuf));
      if(NULL != (fp = POPEN(GET_LPWR_LOG, "r"))) {
          if(NULL != fgets(replyBuf, sizeof(replyBuf), fp)){
              if(0 < (i = strnlen(replyBuf,sizeof(replyBuf)))) {
                  replyBuf[i-1] = '\0'; /* Delete extra char added. */
              }
              I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"lpwrlog[%s]\n",replyBuf);
              usleep(1000); /* Dont hammer on syslog */
          } else {
              I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"grablpwrlog: FGETS FAIL: errno(%s)\n",strerror(errno));
              set_my_error_state(LOGMGR_FGETS_FAIL);  
          }
          PCLOSE(fp);
      } else {
          I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"grablpwrlog: POPEN FAIL: errno(%s)\n",strerror(errno));
          set_my_error_state(LOGMGR_POPEN_FAIL); 
      }
  }
}
static int32_t get_syslog_enable(void)
{
  int32_t  ret = 0;
  char_t   reply_buf[128];
  char_t  *mystring = NULL;
  FILE    *fp = NULL;
  size_t   len = 0;
  uint32_t i;

  if(NULL != (fp = popen(GET_SYSLOG_ENABLE, "r"))){
      if(NULL != fgets(reply_buf, sizeof(reply_buf), fp)){
          len = strnlen(reply_buf,sizeof(reply_buf));
          if((0 != len) && (len < sizeof(reply_buf))){
              if(strstr(reply_buf,"I2VDisableSysLog") && (NULL != (mystring = strstr(reply_buf,"=")))) {
                  for(i=0; (i < len) && ('\0' != mystring[i]);i++){
                      if('0' == mystring[i]) {
                          ret = 1;
                          printf("\n%s: get_syslog_enable: enabled.\n\n",MY_NAME);
                          break;
                      }
                      if('1' == mystring[i]) {
                          ret = 0;
                          printf("\n%s: get_syslog_enable: disabled.\n\n",MY_NAME);
                          break;
                      }
                  }
                  if(len <= i) {
                      set_my_error_state(LOGMGR_SCANF_FAIL);
                      ret = -1;
                      printf("\n%s: get_syslog_enable: SSCANF failed: i(%d) mystring(%s) errno(%s)\n\n",MY_NAME,i,mystring,strerror(errno));
                  }
              } else {
                  set_my_error_state(LOGMGR_SCANF_FAIL);
                  ret = -2;
                  printf("\n%s: get_syslog_enable: SSCANF failed: reply_buf(%s) errno(%s)\n\n",MY_NAME,reply_buf,strerror(errno));
              }
          } else {
              printf("\n%s: get_syslog_enable: strnlen from i2v.conf is bogus len(%lu)\n\n",MY_NAME,len);
              set_my_error_state(LOGMGR_BAD_DATA);
              ret = -3;
          }
      } else {
          printf("\n%s: get_syslog_enable: FGETS failed(%s)\n\n",MY_NAME,strerror(errno));
          set_my_error_state(LOGMGR_FGETS_FAIL);
          ret = -4;
      }
      pclose(fp);
  } else {
      printf("\n%s: get_syslog_enable: POPEN failed(%s)\n\n",MY_NAME,strerror(errno));
      set_my_error_state(LOGMGR_POPEN_FAIL);
      ret = -5;
  }
  fflush(stdout);
  return ret;
}
int MAIN(int argc, char_t *argv[])
{
    int32_t c;
    pthread_t fileThrID, msgThrID, sendThrID, ifcLogThrID;
    int ret = LOGMGR_AOK;
    uint32_t iteration_counter=0;
    i2vTimeT clock;
    int32_t pRet = 0;
    int32_t i=0;
    int32_t syslog_open = 0;

    lmgrInitStatics();

    memset(&clock, 0, sizeof(clock));

    /* Enable serial debug with I2V_DBG_LOG until i2v.conf says otherwise. */
    i2vUtilEnableDebug(MY_NAME);

    while ((LOGMGR_AOK == ret) && ((c=getopt(argc,argv, "d:I:DQW")) != -1)) {
        switch (c) {
            case 'D': /* debug mode - no file logging */
                debugModeOn = WTRUE;
                break;
            case 'd':  /* config directory */
                if (strnlen(optarg,sizeof(optarg)) > I2V_CFG_MAX_STR_LEN) {
                    printf("%s| directory path too long\n",MY_NAME);
                    ret = LOGMGR_CFG_DIR_NAME_TOO_LONG;
                    set_my_error_state(ret);
                    ret = I2V_RETURN_FAIL;
                    break; 
                }
                strncpy(cfgdir, optarg, sizeof(cfgdir));
                break;
            case 'Q': /* quiet mode - no log message prints to console if log file inaccessible */
                break;
            case 'W':
                /* Legacy: Not used. */
                break;
            default:
                break;
        }
    }

    if(WTRUE == debugModeOn) {
       i2vUtilEnableDebug(MY_NAME);
    } else {
       i2vUtilDisableDebug(); 
    }

    if(0 != pthread_mutex_init(&filelock ,NULL)){
        I2V_DBG_LOG(LEVEL_CRIT,MY_NAME,"filelock Mutex init failed\n");
        sleep(1);
        ret = LOGMGR_INIT_SHM_FAIL;
    } else {
        if(0 != pthread_mutex_init(&ifcreqlock ,NULL)){
            I2V_DBG_LOG(LEVEL_CRIT,MY_NAME,"ifcreqlock Mutex init failed\n");
            pthread_mutex_destroy(&filelock);
            sleep(1);
            ret = LOGMGR_INIT_SHM_FAIL;
        }
    }

    if ((LOGMGR_AOK == ret) && (!strnlen(cfgdir,sizeof(cfgdir)))) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"invalid directory path\n");
        set_my_error_state(LOGMGR_BAD_DATA);
        ret = I2V_RETURN_FAIL;
    }

    memset(&lmgrCfg, 0, sizeof(lmgrCfg));
    if ((LOGMGR_AOK == ret) &&(!lmgrUpdateCfg())) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"failed to load config\n");
        set_my_error_state(LOGMGR_CFG_LOAD_FAIL);
        ret = I2V_RETURN_FAIL;
    }
    if (LOGMGR_AOK == ret) {
        if(1 == get_syslog_enable()) {
//TODO: Revisit Time Master: If there is no gnss fix then using last file time stamp is still wrong.
            /* Get inital timestamp for everone. NO GNSS then this is baloney. */
            i2vUtilGetUTCTime(&clock);
            curtime_seconds = CONVERT_TO_UTC(clock.year, clock.yday, clock.hour, clock.min, clock.sec);

            snprintf(timestamp_msg,sizeof(timestamp_msg), "%d%.2d%.2d_%.2d%.2d%.2d | ", clock.year, clock.month, clock.day, clock.hour, clock.min, clock.sec);
            snprintf(timestamp,sizeof(timestamp), "%d%.2d%.2d_%.2d%.2d%.2d", clock.year, clock.month, clock.day, clock.hour, clock.min, clock.sec);

            syslog_enabled = 1; 
        } else {
            printf("\n%s: SYSLOG disabled.\n\n",MY_NAME);
            fflush(stdout);
        }
        /* 
         * Setup SIGTERM, SIGINT, SIGKILL handlers.
         * SIGCHILD: IFCLOG will cause this when halts logging. Don't care for now.
         */
        lmgrSetupSigHandler((void *)lmgrSigHandler, SIGTERM);
        lmgrSetupSigHandler((void *)lmgrSigHandler, SIGINT);
        lmgrSetupSigHandler((void *)lmgrSigHandler, SIGKILL);
#if defined(ENABLE_LOGMGR_CHILD_SIG)
        lmgrSetupSigHandler(lmgrChildTermHandler, SIGCHLD);
#endif
    }
//TODO: make all the other threads wait till handshake from first thread.
    /* 
     * Start LOGMGR mq requests for syslog entries to file. Start first.
     * Keep this thread going even when syslog output disabled on purpose.
     * Creating & servicing the mq will prevent weird mq errors.
     * if I2V_DBG_LOG not disabled properly in task.
     */
    if(LOGMGR_AOK == ret){
        if (pthread_create(&msgThrID, NULL, lmgrMsgHandlerThr, NULL) == -1) {
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"failed to create msg handler. errno=%d.\n", errno);
            ret = LOGMGR_MSG_HANDLER_FAIL;
            set_my_error_state(ret);
            ret = I2V_RETURN_FAIL;
        } else {
            /* 20190208: QNX pthread_join fails; detach instead (will properly exit) */
            pthread_detach(msgThrID);
        }
        usleep(500);
    }
    /* 
     * Start LOGMGR syslog file handling: Too big, too old or dir too full. Start second. 
     * If syslog not enabled dont bother creating a logfile.
     */
    if ((LOGMGR_AOK == ret) && (1 == syslog_enabled)){
        if (pthread_create(&fileThrID, NULL, lmgrFileHandlerThr, NULL) == -1) {
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"failed to create logfile handler. errno=%d.\n", errno);
            ret = LOGMGR_FILE_HANDLER_FAIL;
            set_my_error_state(ret);
            ret = I2V_RETURN_FAIL;
        } else {
            /* 20190208: QNX pthread_join fails; detach instead (will properly exit) */
            pthread_detach(fileThrID);
        }
    }

//TODO: Not tested.
    /* Start SYSLOG scp service if enabled. Start third. */
    if ((LOGMGR_AOK == ret) && (lmgrCfg.xmitInterval)) {
        if (pthread_create(&sendThrID, NULL, lmgrSendHandlerThr, NULL) == -1) {
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"failed to create send handler: Interval=%d, errno=%d.\n",lmgrCfg.xmitInterval,errno);
            set_my_error_state(LOGMGR_SEND_HANDLER_FAIL); 
            ret =  I2V_RETURN_FAIL;
        } else {
            /* 20190208: QNX pthread_join fails; detach instead (will properly exit) */
            pthread_detach(sendThrID);
        }
        usleep(500);
    }

    /* Start NTCIP-1218 Services. */
    if (LOGMGR_AOK == ret) {
        if (0 == (ret = pthread_create(&InterfaceLogThreadID, NULL, InterfaceLogThread, NULL))) {
            pthread_detach(InterfaceLogThreadID);
        } else {
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"InterfaceLogThreadID: Failed. Not Fatal: ret=%d, errno=%d.\n",ret,errno);
            set_my_error_state(LOGMGR_THREAD_FAIL); /* Not Fatal, keep going. */
        }
        if (0 == (ret = pthread_create(&SystemSettingsThreadID, NULL, SystemSettingsThread, NULL))) {
            pthread_detach(SystemSettingsThreadID);
        } else {
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"SystemSettingsThreadID: Failed. Not Fatal: ret=%d, errno=%d.\n",ret, errno);
            set_my_error_state(LOGMGR_THREAD_FAIL);/* Not Fatal, keep going. */
        }
    }

    /* Start IFCLOG main thread. Start last. */
    if (LOGMGR_AOK == ret) {
        if (pthread_create(&ifcLogThrID, NULL, lmgrIfcLogHandlerThr, NULL) == -1) {
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"failed to create ifc log handler. errno=%d.\n",errno);
            ret = LOGMGR_IFC_HANDLER_FAIL;
            set_my_error_state(ret);
            ret =  I2V_RETURN_FAIL;
        } else {
            /* 20190208: QNX pthread_join fails; detach instead (will properly exit) */
            pthread_detach(ifcLogThrID);
        }
        usleep(500);
    }

    /* mainloop will calculate curtime for all and check I2V SHM for updates. */
    usleep(100000);/* No need to rush in. */
    if (LOGMGR_AOK == ret) {
        while (mainloop) {
//TODO: do we need a better handshake betweek logmgr, syslog and everybody else?
            if((300 < iteration_counter) && (0 == syslog_open) && (WTRUE == debugModeOn) && (1 == syslog_enabled)){ /* assume LOGMGR is ready */
                if(0 == i2vUtilEnableSyslog(MY_ERR_LEVEL, MY_NAME)){
                    /* If you dont enable debug then no SEL output*/
                    syslog_open = 1;
                    grabbuildinfo();
                    grabdeviceids();
                    grablpwrlog();
                } else {
                    if(iteration_counter < 310) {
                        printf("%s: syslog enable fail(%d)\n",MY_NAME,iteration_counter);
                    }
                    i2vUtilDisableSyslog();
                }
            }
            if((1 == syslog_enabled) && ((333 == iteration_counter) || (0 == (iteration_counter % (10 * OUTPUT_MODULUS))))) {
                I2V_DBG_LOG(LEVEL_INFO,MY_NAME,"(0x%lx) drop(%u) size(f:d:md)(%u, %u, %u)\n",
                    logmgr_error_states,locked_out,current_logfile_size,current_logdir_size,max_syslogdir_size);    
            }
            i2vUtilGetUTCTime(&clock);
            curtime_seconds = CONVERT_TO_UTC(clock.year, clock.yday, clock.hour, clock.min, clock.sec);

            i=10;
            pRet=-1;
            while ((0<i) && (pRet = pthread_mutex_trylock(&filelock) != 0)) {
                usleep(360);
                i--;
            }
            if(0 == pRet) {
                snprintf(timestamp_msg,sizeof(timestamp_msg), "%d%.2d%.2d_%.2d%.2d%.2d | ", clock.year, clock.month, clock.day, clock.hour, clock.min, clock.sec);
                snprintf(timestamp,sizeof(timestamp), "%d%.2d%.2d_%.2d%.2d%.2d", clock.year, clock.month, clock.day, clock.hour, clock.min, clock.sec);
                pthread_mutex_unlock(&filelock);
            } else {
                set_my_error_state(LOGMGR_MUTEX_LOCK_FAIL);
            }

            usleep(100 * 1000); /* 100msec - nothing to do */
            if(NULL != logptr) {
                fflush(logptr);
            }
            iteration_counter++;
#if defined(MY_UNIT_TEST)
            if(100 < iteration_counter)
                mainloop = WFALSE; /* One iteration and done. */
#endif
        } /* while mainloop */
    }
#if defined(EXTRA_DEBUG)
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"EXIT: ret(%d) (0x%lx)\n",ret,logmgr_error_states);
#endif
    /* Stop I2V_DBG_LOG output. Last chance to output to syslog. */
    if(WTRUE == debugModeOn) {
        i2vUtilDisableSyslog();
        i2vUtilDisableDebug();
    }

    if(LOGMGR_INIT_SHM_FAIL != ret) {
        pthread_mutex_destroy(&filelock);
        pthread_mutex_destroy(&ifcreqlock);
    }
    return abs(ret);
}

