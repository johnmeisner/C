/**************************************************************************
 *                                                                        *
 *     File Name:  amh.c  (active message handler)                        *
 *     Author:                                                            *
 *         DENSO International America, Inc.                              *
 *         North America Research Laboratory, California Office           *
 *         Rutherford road                                                *
 *         Carlsbad, CA 92008                                             *
 *                                                                        *
 **************************************************************************/
/**************************************************************************
    This module will check if map, spat, and tim apps are active -
    if active it will create simplified broadcast files from any
    corresponding active message files in the active message dir.
    For any of the apps that are not active, this app will handle
    broadcasting corresponding active messages directly.

    Implementation notes: 
       - there is handling in this code that will probably never get used
       - originally the design was to use map and tim broadcast directories
         and let them send at their expected frequencies - that changed
         and the immediate forward shared memory is overloaded
       - amhCreateFile puts map and tims in their directories - it will
         in most environments never be used
       - amhPrepareBcastMsgs has logic for amh to directly broadcast -
         amh will probably never broadcast directly although it can
       - the 'dead' code is retained because RSU functional specs are a 
         work in progress - as example v3 defined functionality for a
         heartbeat system that v4 completely removed; since RSU specs are
         in flux the author of this code believes it is better to retain
         the original implementation and place hooks to enable or disable
         the functionality based on the 'latest' requirements
         (NOTE: the 'dead' code may come back alive if I2V moves to a 
         single broadcast thread instead of having all the apps
         individually broadcast - with a single thread and some refactoring
         amh could be simplified significantly to ignore peer apps)
         20140722: the single broadcast is now available (iwmh) - amh has
         been interop tested and works so extricating code may not be the
         best use of time; but if improving this module when iwmh is 
         enabled amh can send on its own if desired (note that amh sending
         will still require validation and fixes such as the usersvc 
         change which is not the appropriate implementation)
       - 20140821 - added minimal RTCM support - no support for stored
         files since RTCM content is 'Real Time' so can't be stored -
         unlikely RTCM will ever be an active message though
       - 20220119 - We have reached Europa, Dave. NTCIP-1218 update complete.
 **************************************************************************/
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h> 
#include <string.h>
#include <mqueue.h>
#include <errno.h>
#include <sys/socket.h>
#include <poll.h>
#include <fcntl.h>
#include "wsu_util.h"
#include "wsu_shm.h"
#include "i2v_util.h"
#include "amh.h"
#include "tps_api.h"
#if !defined S_SPLINT_S
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#endif
#if defined(J2735_2016)
#include "DSRC.h"
#endif
#if defined(J2735_2023)
#include "MessageFrame.h"
#endif
/* MIB definition for RSU */
#include "ntcip-1218.h"
#include "conf_table.h"
#include "rsuhealth.h"
#if defined(MY_UNIT_TEST)
#include "stubs.h"
#define MAIN amh_main
#else
#define MAIN main
#endif

#if defined(EXTRA_DEBUG)
#define OUTPUT_MODULUS  30  /* Seconds. */
#else
#define OUTPUT_MODULUS  1215
#endif

#define AMH_LIMIT_RADIO_REG  /* limit the max concurrent services amh can ask of the radio. */

/* mandatory logging defines */
#define MY_ERR_LEVEL   LEVEL_PRIV    /* from i2v_util.h */
#define MY_NAME        "amh"

/* see P1609 */
#define MAX_WSM_PRIORITY  7  /* RSU 4.1. */

#define PEER_LIST  "/tmp/amhListingFile.txt"

#define AMC_DEF_WMH_TO   90  /* msec */

/* 
 * usleep time per iteration of AMH mainloop.
 * At 1 msec sleep time the max messages it can service is 1k per second.
 * Can handle ~700Hz SAR and slighly more if IMF only.
 * Under extreme load SAR will win out over IMF it appears if conflicting.
 */
#if defined(MY_UNIT_TEST)
#define DEFAULT_AMH_SLEEP_FACTOR (1000 * 20) /* 50hz. Sharing and caring thread. CPU for all. */
#else
#define DEFAULT_AMH_SLEEP_FACTOR 1000 /* 1Khz Ramming speed, no prisoners! Not bits left behind. */
#endif

#define UTC_DELTA_1_sec 1000

/* state control for file STAT */
#define AMH_RELOAD  1
#define AMH_IGNORE  2

#define AMH_FWD_IGNORE_MSG_TYPE_MASK 0x80000000 /* For amh.conf::AMHForwardEnable. */

#define SOCKET_POLL_TIMEOUT  0  /* Zero wait. Don't hang around if not ready. */

#define AMH_MANAGE_IMF_SOCKET_RATE 5 /* Seconds */
#define AMH_MANAGE_FWD_SOCKET_RATE 5 /* Seconds */

#define IMF_READ_ERROR_THRESHOLD 10 /* Consecutive read fails on socket that triggers reset. */
#define FWD_WRITE_ERROR_THRESHOLD 10 /* Consecutive write fails to socket that triggers reset. */

/* Management of imf socket: These counts persist. */
uint32_t imf_socket_fail = 0;
uint32_t imf_socket_bind_fail = 0;
uint32_t imf_socket_reset_count = 0;

/* Reading from imf socket: These counts reach IMF_READ_ERROR_THRESHOLD and socket reset occurs.*/
uint32_t imf_read_count = 0;
uint32_t total_imf_read_error_count = 0;
uint32_t imf_read_error_count = 0;
uint32_t imf_read_poll_to_count = 0;
uint32_t imf_read_poll_notready_count = 0;
uint32_t imf_read_poll_fail_count = 0;

/* Record in RAM list: Needs to support 4.1 and ntcip-1218 */
typedef struct amhStoredRec_t {
    amhBitmaskType  encoding;  /* User tells us DSRC Msg ID. We don't verify payload. */
    uint32_t        psid;
    uint8_t         priority;
    bool_t          continuous;  /* Not suppoted. only if not in bcastLockStep */
    uint8_t         channel;
    uint32_t        interval; /* msecs. */
    uint32_t        startMin; /* UTC seconds.*/
    uint32_t        endMin;   /* UTC seconds.*/
    bool_t          signature;
    bool_t          encryption; /* Not supported. */
    payloadType     data;
    char_t          myName[RSU_MSG_FILENAME_LENGTH_MAX]; /* record file name.*/
} __attribute__((packed)) amhStoredRecT;

typedef struct {
    time_t   last_st_atime;  /* access */
    time_t   last_st_mtime;  /* mod    */
    time_t   last_st_ctime;  /* status */
    uint64_t lastSend;
    uint64_t delta;
    int32_t  MIBupdatePending;    
} __attribute__((packed)) MsgRepeat_Stat_t;

amhManagerStatsT amhManagerStats;

i2vShmMasterT * shmPtr = NULL; /* Shared with imf.c */
bool_t          mainloop = WTRUE; /* Shared with IMF thread. */
cfgItemsT       cfg;      
amhCfgItemsT    amhCfg;   
STATIC char_t   confFileName[I2V_CFG_MAX_STR_LEN];
#ifdef HSM
uint8_t sspLenTim;
uint8_t sspLenMap;
uint8_t sspLenSpat;
uint8_t sspTim[MAX_SSP_DATA];
uint8_t sspMap[MAX_SSP_DATA];
uint8_t sspSpat[MAX_SSP_DATA];
uint8_t sspMaskTim[MAX_SSP_DATA];
uint8_t sspMaskMap[MAX_SSP_DATA];
uint8_t sspMaskSpat[MAX_SSP_DATA];
bool_t sspBitmappedTim;
bool_t sspBitmappedMap;
bool_t sspBitmappedSpat;
#endif

/* RSU 4.1 RAM list and file stat. */
STATIC amhStoredRecT    amhstoredb[RSU_SAR_MESSAGE_MAX];      /* 4.1 RAM list: Need to support 4.1 spec and ntcip-1218 */
STATIC MsgRepeat_Stat_t amhstoredb_statTable[RSU_SAR_MESSAGE_MAX];

/* NTCIP-1218 RAM list and file stat. */
STATIC RsuMsgRepeatStatusEntry_t msgRepeatTable[RSU_SAR_MESSAGE_MAX];  /* NTCIP-1218 RAM list. Legacy upconverted list. */
STATIC MsgRepeat_Stat_t          msgRepeat_statTable[RSU_SAR_MESSAGE_MAX]; /* This alwasy 1 to 1 with above */

/* For all messages no matter where they are from. This is a lot of seperate PSIDs. */
STATIC amcTrackPSIDRec  myAMCpsidlist[RSU_SAR_MESSAGE_MAX]; /* Only de-reg service if only one using it which AMH may not be. */

uint16_t amhstoredbcnt   = 0; /* RSU 4.1 */
int32_t  msgRepeat_count = 0; /* NTCIP-1218 */

/* Respective error states. */
STATIC uint32_t amh_error_states   = 0x0;
STATIC uint32_t sar_error_states   = 0x0;
STATIC uint32_t imf_error_states   = 0x0;

STATIC uint32_t amh_load_files_err_count = 0;

/* For statistics. */
uint32_t prior_sar_count = 0;
uint32_t prior_ifm_count = 0;
float32_t sar_datarate   = 0.0f;
float32_t ifm_datarate   = 0.0f;
uint32_t debug_counter   = 0; /* rolling counter. */

/* Number of times Amh Manager has run */
uint32_t amhEpochCount  = 0x0; 

STATIC struct sockaddr_in dest;

STATIC mqd_t fwdmsgfd;
fwdmsgData amhFwdmsgData;
char sendbuf[sizeof(amhFwdmsgData)];
#if defined(AMH_LIMIT_RADIO_REG)
/* TRACK number of Radio reg */
#define AMH_MAX_CONCURRENT_RADIO_REG 50
STATIC uint32_t my_reg_count     = 0x0;
STATIC uint32_t my_dereg_count   = 0x0;
#endif

#if defined(ENABLE_AMH_PSID_TIMEOUT)
#define MAX_PSID_TIMEOUT  300  /* Seconds: If PSID is not active(wsm_send fail) for this long then de-reg service. */
#endif

/* service parameters.I think IWMH overides us anyways. */
STATIC bool_t    AMCenablechsw   = WFALSE;   /* channel switching flag */

pthread_mutex_t amhRadio_lock = PTHREAD_MUTEX_INITIALIZER;
/* fwdmsg related variables */
STATIC pthread_mutex_t amh_fwdmsg_send_lock = PTHREAD_MUTEX_INITIALIZER;
STATIC bool_t   amhForward;
STATIC fwdmsgCfgItemsT fwdmsgCfg;

/* 1218 MIB: 5.12 rsuMessageStats. */
STATIC pthread_t MessageStatsThreadID;
messageStats_t messageStats[RSU_PSID_TRACKED_STATS_MAX];
STATIC messageStats_t * shm_messageStats_ptr = NULL; /* SHM. */

/* Notifications: rsuAsync */
STATIC rsuhealth_t * shm_rsuhealth_ptr = NULL;
STATIC int32_t lock_send = 0;

/* imf protos used by amh.c */
extern void   * imfThread(void __attribute__((unused)) *arg);
extern void   * mib_rsuIFM_thread(void __attribute__((unused)) *arg);
extern void     close_IFM_queue (void);
extern int32_t  open_IFM_queue(void);

extern void dump_amc_stats(void);

/*
 * Function Bodies.
 */
void set_amh_error_state(int32_t amh_error)
{
  int32_t dummy = 0;

    dummy = abs(amh_error);
    /* -1 to -32 */
    if((amh_error < AMH_ERROR_BASE) && (amh_error > AMH_SAR_ERROR_BASE)) {
        amh_error_states |= (uint32_t)(0x1) << (dummy - 1);
    }
    /* -33 to -65 */
    if((amh_error <= AMH_SAR_ERROR_BASE) && (amh_error > AMH_IMF_ERROR_BASE)) {
       sar_error_states |= (uint32_t)(0x1) << (dummy + AMH_SAR_ERROR_BASE);
    }
    /* -66 to -98 */
    if((amh_error <= AMH_IMF_ERROR_BASE) && (amh_error > AMH_ERROR_TOP)) {
        imf_error_states |= (uint32_t)(0x1) << (dummy + AMH_IMF_ERROR_BASE);
    }
    return;
}
void clear_amh_error_state(int32_t amh_error)
{
  int32_t dummy = 0;

    dummy = abs(amh_error);
    /* -1 to -32 */
    if((amh_error < AMH_ERROR_BASE) && (amh_error > AMH_SAR_ERROR_BASE)) {
        amh_error_states &= ~((uint32_t)(0x1) << (dummy - 1));
    }
    /* -33 to -65 */
    if((amh_error <= AMH_SAR_ERROR_BASE) && (amh_error > AMH_IMF_ERROR_BASE)) {
       sar_error_states &= ~((uint32_t)(0x1) << (dummy + AMH_SAR_ERROR_BASE));
    }
    /* -66 to -98 */
    if((amh_error <= AMH_IMF_ERROR_BASE) && (amh_error > AMH_ERROR_TOP)) {
        imf_error_states &= ~((uint32_t)(0x1) << (dummy + AMH_IMF_ERROR_BASE));
    }
    return;
}
int32_t is_amh_error_set(int32_t amh_error)
{
  int32_t dummy = 0;
  int32_t ret = 0; /* FALSE till proven TRUE */

    dummy = abs(amh_error);
    /* -1 to -32 */
    if((amh_error < AMH_ERROR_BASE) && (amh_error > AMH_SAR_ERROR_BASE)) {
        if (amh_error_states & ((uint32_t)(0x1) << (dummy - 1))) {
            ret = 1;
        }
    }
    /* -33 to -65 */
    if((amh_error <= AMH_SAR_ERROR_BASE) && (amh_error > AMH_IMF_ERROR_BASE)) {
        if (sar_error_states & ((uint32_t)(0x1) << (dummy + AMH_SAR_ERROR_BASE))) {
            ret = 1;
        }
    }
    /* -66 to -98 */
    if((amh_error <= AMH_IMF_ERROR_BASE) && (amh_error > AMH_ERROR_TOP)) {
        if(imf_error_states & ((uint32_t)(0x1) << (dummy + AMH_IMF_ERROR_BASE))) {
            ret = 1;
        }
    }
    return ret;
}
#if defined(PRINT_RAMLIST)
void print_ramlist(void)
{
  int32_t i;

  for(i=0;i<RSU_SAR_MESSAGE_MAX;i++){
      if(0 != amhstoredb[i].priority){
          I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"legacy ramlist(%d):psid=%u,pri=%u,ch=%u,int=%u,start=%u,stop=%u\n", i
              , amhstoredb[i].psid
              , amhstoredb[i].priority
              , amhstoredb[i].channel
              , amhstoredb[i].interval
              , amhstoredb[i].startMin
              , amhstoredb[i].endMin);
          I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"legacy stat(%d): AMC(%lu,%lu,%lu) last=%lu, delta=%lu, pend=%d.\n", i
            , amhstoredb_statTable[i].last_st_atime   
            , amhstoredb_statTable[i].last_st_mtime   
            , amhstoredb_statTable[i].last_st_ctime
            , amhstoredb_statTable[i].lastSend
            , amhstoredb_statTable[i].delta
            , amhstoredb_statTable[i].MIBupdatePending); 
      }
      if(SNMP_ROW_NONEXISTENT != msgRepeatTable[i].rsuMsgRepeatStatus){
          I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"1218 ramlist(%d):pri=%d,ch=%d,int=%d.\n", i 
            , msgRepeatTable[i].rsuMsgRepeatPriority
            , msgRepeatTable[i].rsuMsgRepeatTxChannel
            , msgRepeatTable[i].rsuMsgRepeatTxInterval);
          I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"1218 stat(%d): AMC(%lu,%lu,%lu) last=%lu, delta=%lu, pend=%d.\n", i
            , msgRepeat_statTable[i].last_st_atime   
            , msgRepeat_statTable[i].last_st_mtime   
            , msgRepeat_statTable[i].last_st_ctime
            , msgRepeat_statTable[i].lastSend
            , msgRepeat_statTable[i].delta 
            , msgRepeat_statTable[i].MIBupdatePending);
      } 
  }
}
#endif
STATIC void amhUtilGetUTCTime(i2vTimeT *clock)
{
    time_t    val;
    struct tm t, *tmptr;

    if (NULL == clock) return;
   
    val = time(NULL);
    tmptr = gmtime_r(&val, &t);

    clock->year  = (uint16_t)(1900 + tmptr->tm_year);
    clock->month = (uint8_t) (1 + tmptr->tm_mon);
    clock->day   = (uint8_t)tmptr->tm_mday;
    clock->hour  = (uint8_t)tmptr->tm_hour;
    clock->min   = (uint8_t)tmptr->tm_min;
    clock->sec   = (uint8_t)tmptr->tm_sec;
    clock->yday  = (uint16_t)tmptr->tm_yday;
    clock->wday  = (uint8_t)tmptr->tm_wday;
}
/* Shared with IMF so need to lock. */
#define MAX_TRY_LOCK 10
void sendnotification(char_t * msg, int32_t level)
{
  int32_t  i = 0;
  char_t   rsuMsgFileIntegrityMsg[RSUHEALTH_ALERT_MSG_LENGTH_MAX];

  if(1 == lock_send) {
      #if defined(EXTRA_DEBUG)
      I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"sendnotification: Busy, notifcation dropped.\n");
      #endif
      return;  
  }
  lock_send = 1;

  if(NULL == shm_rsuhealth_ptr) {
      if(NULL == (shm_rsuhealth_ptr = wsu_share_init(sizeof(rsuhealth_t), RSUHEALTH_SHM_PATH))) {
          I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"RSUDIAG SHM init failed.\n");
          shm_rsuhealth_ptr = NULL; /* Keep going, not fatal. */
          set_amh_error_state(AMH_RSUHEALTH_NOT_READY);
      }
  } 

  /* Send notification to ntcip-1218 who will send trap for us. */
  if(NULL != shm_rsuhealth_ptr) {

      /* Clear variables and statics. */
      memset(rsuMsgFileIntegrityMsg,0x0,sizeof(rsuMsgFileIntegrityMsg));

      /* send to syslog first. */
      if(NULL == msg) { /* zero length msg is legal in ntcip-1218 MIB.  */
          #if defined(EXTRA_DEBUG)
          I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"sendnotification: NULL input.\n");
          #endif
      } else {
          strncpy(rsuMsgFileIntegrityMsg,msg,sizeof(rsuMsgFileIntegrityMsg));
          #if defined(EXTRA_DEBUG)
          I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,rsuMsgFileIntegrityMsg);
          #endif
      }
      if((level < rsuAlertLevel_info) || (rsuAlertLevel_critical < level)){
          I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"sendnotification: bad level(%d)\n", level);
          level = rsuAlertLevel_info;
      }

      for(i = 0; i < MAX_TRY_LOCK; i++) {
          if(WTRUE == wsu_shmlock_trylockw(&shm_rsuhealth_ptr->h.ch_lock)) {
              if(WTRUE == shm_rsuhealth_ptr->h.ch_data_valid) {
                  clear_amh_error_state(AMH_RSUHEALTH_NOT_READY);
                  shm_rsuhealth_ptr->messageFileIntegrityError = level;
                  strncpy(shm_rsuhealth_ptr->rsuMsgFileIntegrityMsg,rsuMsgFileIntegrityMsg,sizeof(rsuMsgFileIntegrityMsg));
                  if(WFALSE == wsu_shmlock_unlockw(&shm_rsuhealth_ptr->h.ch_lock)) {
                      I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"wsu_shmlock_unlockw() failed.\n");
                  }
                  #if defined(EXTRA_DEBUG)
                  I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"sendnotification success: (%d)(%s)\n",shm_rsuhealth_ptr->messageFileIntegrityError,shm_rsuhealth_ptr->rsuMsgFileIntegrityMsg);
                  #endif
                  break;
              } else {
                  #if defined(EXTRA_DEBUG)
                  I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"shm not valid yet.\n");
                  #endif
                  set_amh_error_state(AMH_RSUHEALTH_NOT_READY);
                  if(WFALSE == wsu_shmlock_unlockw(&shm_rsuhealth_ptr->h.ch_lock)) {
                      I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"wsu_shmlock_unlockw() failed.\n");
                  }
              }
          }
          usleep(1000);
      }
      if (MAX_TRY_LOCK <= i) { /* Could happen. */
          #if defined(EXTRA_DEBUG)
          I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"wsu_shmlock_trylockw() failed.\n");
          #endif
          set_amh_error_state(AMH_RSUHEALTH_NOT_READY);
      }
  }
  lock_send = 0;  
}
STATIC bool_t amcNewRegister(bool_t adding, uint32_t psidval) 
{
  UserServiceType uService = {
        .radioNum = cfg.uradioNum,
        .action = (adding) ? ADD : DELETE,
        .userAccess = AUTO_ACCESS_UNCONDITIONAL,
        .psid = psidval,
        .servicePriority = cfg.uwsaPriority,  // What?
        .wsaSecurity = (cfg.security) ? SECURED_WSA : UNSECURED_WSA,
        .lengthPsc = 10,
        .channelNumber = cfg.uchannel,
        .lengthAdvertiseId = 15,
        .linkQuality = 20,
        .immediateAccess = (!AMCenablechsw) ? 1 : 0,
        .extendedAccess = (!AMCenablechsw) ? 65535: 0,
        .radioType = RT_DSRC,
  };
  int32_t ret     = I2V_RETURN_OK;
  uint32_t optret = I2V_RETURN_OK;

  if(1 == cfg.RadioType) {
      uService.radioType = RT_CV2X;
  } else {
      uService.radioType = RT_DSRC;
  }

#if defined(AMH_LIMIT_RADIO_REG)
  if((AMH_MAX_CONCURRENT_RADIO_REG <= my_reg_count)  && (adding)){
      set_amh_error_state(AMH_RADIO_REG_FULL);
      ret = AMH_RADIO_REG_FULL;
  } else {
#endif

  /* SPAT will manage this service. Just return success. */
  if((psidval == cfg.secSpatPsid) && (cfg.scsAppEnable)) {
      ret = I2V_RETURN_OK;
  } else {
      I2V_UTIL_USER_SVC_REQ(&uService, &cfg, WMH_SVC_TIMEOUT, &ret, &optret);
      usleep(1000); /* Give radio time to get setup before letting rip. */
  }

#if defined(AMH_LIMIT_RADIO_REG)
  }
  if(I2V_RETURN_OK == ret) {
      if(adding) {
          my_reg_count++; //check on entry already for size.
      } else {
          if(my_reg_count == 0) {
              set_amh_error_state(AMH_RADIO_REG_NEGATIVE);
              ret = AMH_RADIO_REG_NEGATIVE;
          } else {
              my_reg_count--;
              my_dereg_count++;
          }
      }
  }
#endif /* AMH_LIMIT_RADIO_REG */

  /* ret set in service request call. */
  if(ret == I2V_RETURN_OK) {
      #if defined(EXTRA_DEBUG)
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"amcNewRegister: PSID=0x%x, ret=%d, optret=%u.\n",psidval, ret, optret);
      #endif
      return WTRUE;
  } else {
      set_amh_error_state(AMH_NEW_PSID_REG_FAIL);
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"AMH_NEW_PSID_REG_FAILL: failed to register service with iwmh: ret(%d) optret(0x%x)\n", ret, optret);
      return WFALSE;
  }
}

/*    
 * Call every MAX_PSID_TIMEOUT.
 */
#if defined(ENABLE_AMH_PSID_TIMEOUT)
STATIC void amcPrunePSIDList(void)
{
  uint64_t timenow;
  uint32_t i;

  timenow = i2vUtilGetTimeInMs();
  for(i=0;i<MAX_AMH_EXTRA_PSID;i++){
      if((0x0 != myAMCpsidlist[i].track_psid) && ((timenow - myAMCpsidlist[i].track_lastSend) > MAX_PSID_TIMEOUT)){
          if (WFALSE == amcNewRegister(WFALSE, myAMCpsidlist[i].track_psid)){
              #if defined(EXTRA_DEBUG)
              I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"AMH_DELETE_SERVICE_FAILED:(0x%x)\n",myAMCpsidlist[i].track_psid);
              #endif
              set_amh_error_state(AMH_DELETE_SERVICE_FAILED);
          } else {
              #if defined(EXTRA_DEBUG)
              I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"amcPrunePSIDList: delete service success:(0x%x)\n",myAMCpsidlist[i].track_psid);
              #endif
          }
          myAMCpsidlist[i].track_psid = 0x0;
      }
  }
}
#endif
/* if addentry WTRUE, returns WTRUE if list entry updated (entry existed)
   or WTRUE if new list entry created OTHERWISE returns WFALSE

   if addentry WFALSE, returns WTRUE if list contains entry (psid match)
   OTHERWISE returns WFALSE
   DO NOT set addentry WTRUE unless a registration was completed
*/

STATIC bool_t amcUpdateList(bool_t addentry, uint32_t psid)
{
  uint64_t timenow;
  uint32_t i;

  timenow = i2vUtilGetTimeInMs();
  for(i=0;i<MAX_AMH_EXTRA_PSID;i++){
      if(psid == myAMCpsidlist[i].track_psid){
          break;
      }
  }
  if(WFALSE == addentry){
      if(i < MAX_AMH_EXTRA_PSID){
          return WTRUE; /* found */
      } 
      return WFALSE; /* not found */      
  }
  if(WTRUE == addentry){ /* Find free space */
      if(i < MAX_AMH_EXTRA_PSID){
          myAMCpsidlist[i].track_lastSend = timenow;
          return WTRUE; /* found. Update send time */
      } 
      /* New entry. Find space */
      for(i=0;i<MAX_AMH_EXTRA_PSID;i++){
          if(0x0 == myAMCpsidlist[i].track_psid){
              break;
          }
      }
      if(i < MAX_AMH_EXTRA_PSID){ /* found empty space */
          myAMCpsidlist[i].track_psid = psid;
          myAMCpsidlist[i].track_lastSend = timenow;
      }
  }    
  return WTRUE;
}

void amhForwardManager(amhBitmaskType amh_msgType, uint8_t * buf, int32_t count)
{
  /* Courtesy Check: Are we not forwarding a message? */
  if(   (0x0 == (AMH_FWD_IGNORE_MSG_TYPE_MASK & amhCfg.amhFwdEnable))
     && (0x0 == (amh_msgType & amhCfg.amhFwdEnable)) ){ /* Message not enabled for forwarding. */
      set_amh_error_state(AMH_FWD_WRITE_IGNORE);        /* Not error but status indiator. */
      return;
  }
  if(0x0 == amhCfg.amhFwdEnable){
      return; /* Nothing to do. */
  } 
  if((NULL == buf) || (0 == count) || (MAX_WSM_DATA < count)){
      set_amh_error_state(AMH_FWD_BAD_INPUT);
      return;
  }
  if(amhForward) {
      pthread_mutex_lock(&amh_fwdmsg_send_lock);
      memset(&amhFwdmsgData,0,sizeof(fwdmsgData));
      amhFwdmsgData.fwdmsgType = AMH;
      memcpy(&amhFwdmsgData.fwdmsgData,buf,count);
      amhFwdmsgData.fwdmsgDataLen = count;
      memcpy(sendbuf,&amhFwdmsgData,sizeof(fwdmsgData));
      if(-1 == mq_send(fwdmsgfd, sendbuf,sizeof(fwdmsgData),DEFAULT_MQ_PRIORITY))   {
          #if defined(EXTRA_EXTRA_DEBUG)
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"AMH_FWD_WRITE_ERROR.\n");
          #endif
          set_amh_error_state(AMH_FWD_WRITE_ERROR);
      }
      pthread_mutex_unlock(&amh_fwdmsg_send_lock);
  }
}
STATIC void amhSigHandler(int __attribute__((unused)) sig)
{
  /* 
   * One job only, short & sweet: Signal main and all threads to exit.
   * Can be called from multiple signals so watch out. 
   */
  if(WTRUE == mainloop) {
      set_amh_error_state(AMH_SIG_FAULT);
      mainloop = WFALSE;
  }
}
/* 
 * AMH & IFM call this to send message to radio: Future, we can just pack params once and hand it in.   
 */ 
uint32_t amhTxMessage(uint8_t * buf, int32_t count, amhSendParams * shmsendp)
{    
    outWSMType sendpkt;
    uint32_t ret = 0;
    uint8_t optret = 0;
    amhSendParams params;
    int32_t i;
    char_t buffer[RSU_ALERT_MSG_LENGTH_MAX];

    if (   (NULL == buf) 
        || (NULL == shmsendp)
        || (0    == count)
        || (MAX_WSM_DATA < count)) {
        #if defined(EXTRA_DEBUG)
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"amhTxMessage:: NULL inputs: count = %d.\n",count);
        #endif
        set_amh_error_state(AMH_MAIN_SAR_NULL_INPUT_ERROR);
    } else {
        #if defined(EXTRA_EXTRA_DEBUG)
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"amhTxMessage:: new message.\n");
        #endif
        memset(&sendpkt, 0, sizeof(sendpkt));
        memcpy(&params, shmsendp, sizeof(params));
        memcpy(sendpkt.data, buf, count);
        if(1 == cfg.RadioType) {
            sendpkt.radioType = RT_CV2X;
        } else {
            sendpkt.radioType = RT_DSRC;
        }
        sendpkt.dataLength = count;       
        sendpkt.dataRate = DR_6_MBPS;
        sendpkt.txPwrLevel = cfg.txPwrLevel;
        sendpkt.txPriority = params.priority; 
        sendpkt.radioNum = cfg.uradioNum;
        sendpkt.channelInterval = 0;
        sendpkt.security = params.security;
        if (params.security == SECURITY_SIGNED) {
            sendpkt.securityFlag = WTRUE;
        }
        memset(sendpkt.peerMacAddress, 0xff, LENGTH_MAC); 
        sendpkt.psid          = params.psid;
        sendpkt.channelNumber = params.useCCH;
        if (amhCfg.hdrExtra) {
          sendpkt.wsmpHeaderExt = WAVE_ID_TRANSMIT_POWER_BITMASK | WAVE_ID_CHANNEL_NUMBER_BITMASK | WAVE_ID_DATARATE_BITMASK;
        }    
        /* check list of psids to see if new registration required */
        if (WFALSE == amcUpdateList(WFALSE, params.psid)) {
            if (WFALSE == amcNewRegister(WTRUE, params.psid)) {
                #if defined(EXTRA_EXTRA_DEBUG)
                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"amcNewRegister: FAIL psid(0x%x)\n", params.psid);
                #endif
                if(0x0 == (is_amh_error_set(AMH_NEW_SAR_PSID_REG_FAIL))) {
                    set_amh_error_state(AMH_NEW_SAR_PSID_REG_FAIL);
                    memset(buffer,0x0,sizeof(buffer));
                    snprintf(buffer,sizeof(buffer), "Failed to register psid(0x%x)", params.psid);
                    sendnotification(buffer, rsuAlertLevel_error);
                }
                return AMH_NEW_SAR_PSID_REG_FAIL;
            }
            #if defined(EXTRA_DEBUG)
            else {
                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"amcNewRegister: SUCCESS psid(0x%x)\n", params.psid);
            }
            #endif
        }
        #if defined(EXTRA_EXTRA_DEBUG) 
        else I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"amcNewRegister: NOT NEEDED psid(0x%x)\n", params.psid);
        #endif

/* Do we want to verify versus params.dsrcMsgId? */       
#ifdef HSM 
        /* Set the Ssp values just before sending the message out */
        /* Check to see if ssp and security are enabled */
        if (cfg.timSspEnable && cfg.security && (params.psid == cfg.secTimPsid ))  {
            /* Copy the TIM SSP Length, Value, and Mask to the Tx pkt */
            sendpkt.sspLen = sspLenTim;
            memcpy(sendpkt.ssp,sspTim,sspLenTim);
            if (sspBitmappedTim == 1)   {
                sendpkt.isBitmappedSsp = sspBitmappedTim;
                memcpy(sendpkt.sspMask,sspMaskTim,sspLenTim);
            }
        } // TIM

        if (cfg.mapSspEnable && cfg.security && (params.psid == cfg.secMapPsid)) {
            /* Copy the MAP SSP Length, Value, and Mask to the Tx pkt */
            sendpkt.sspLen = sspLenMap;
            memcpy(sendpkt.ssp,sspMap,sspLenMap);
            if (sspBitmappedMap == 1)   {
                sendpkt.isBitmappedSsp = sspBitmappedMap;
                memcpy(sendpkt.sspMask,sspMaskMap,sspLenMap);
            }
        } // MAP

        if (cfg.spatSspEnable && cfg.security && (params.psid == cfg.secSpatPsid)) {
            /* Copy the SPAT SSP Length, Value, and Mask to the Tx pkt */
            sendpkt.sspLen = sspLenSpat;
            memcpy(sendpkt.ssp,sspSpat,sspLenSpat);
            if (sspBitmappedSpat == 1)   {
                sendpkt.isBitmappedSsp = sspBitmappedSpat;
                memcpy(sendpkt.sspMask,sspMaskSpat,sspLenSpat);
            }
        } // SPAT
#endif    
        /* Send message.
         * If tx fails, because no certs or no gnss, MIB will not know.
         * Client needs to look through syslogs to figure out why.
         */
        optret = 0;
        ret = 0;
        I2V_UTIL_WSM_REQ(&sendpkt, &cfg, AMC_DEF_WMH_TO, &ret, &optret);
        if ((0 != optret) && (ret == I2V_RETURN_OK)) {
            ret = optret;
            #if defined(EXTRA_DEBUG)
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"AMH_WSM_REQ_RESP_ERROR: ret=%u, optret=%u psid=0x%x.\n",ret,optret,params.psid);
            #endif
        }
        if (ret == I2V_RETURN_OK) {
            for(i=0;i<RSU_PSID_TRACKED_STATS_MAX;i++){
                if((sendpkt.psid == messageStats[i].psid) && (SNMP_ROW_ACTIVE == messageStats[i].rowStatus)) {
                    messageStats[i].count++;
                }
            }
        } else {
#if 1
            set_amh_error_state(AMH_SAR_WSM_REQ_ERROR);
#else //Maybe useful to know, but not message integrity?
            if(0x0 == (is_amh_error_set(AMH_SAR_WSM_REQ_ERROR))) {
                set_amh_error_state(AMH_SAR_WSM_REQ_ERROR);
                memset(buffer,0x0,sizeof(buffer));
                snprintf(buffer,sizeof(buffer), "AMH_SAR_WSM_REQ_ERROR: Radio failed to send psid(%d): ret(%u) optret(%u)",params.psid,ret,optret);
                sendnotification(buffer, rsuAlertLevel_error);
            }
#endif
        }
        /* IMPORTANT: update otherwise de-reg service after X minutes! */
        amcUpdateList(WTRUE, params.psid);  
    }
    return ret;
}

/*
 * Legacy Implementation: RSU 4.1 etc.
 */

/* AMH specifc parsing functions. */
STATIC char_t buf[MEM_PARSE_CONF_MAX_BUF];
STATIC char_t dummy[MEM_PARSE_CONF_MAX_BUF];
STATIC char_t val[MEM_PARSE_CONF_MAX_LEN];

/* Dont Share with IMF */
STATIC uint8_t my_i2v_ascii_2_hex_nibble(char_t  value)
{       
  uint8_t dummy = 0xFF;

  if(( '0' <= value) && (value <= '9'))
      dummy =  (uint8_t)(value - 48);
  if(( 'A' <= value) && (value <= 'F'))
      dummy =  (uint8_t)(value - 55);
  if(( 'a' <= value) && (value <= 'f'))
      dummy =  (uint8_t)(value - 87);

  return dummy;
}

/* See TRAC 2698 for discussion. */
STATIC uint32_t memGets(char_t *outbuf, uint32_t maxBytes, char_t *inbuf)
{
    uint32_t count = 0; 

    if ((outbuf == NULL) || (inbuf == NULL)) {
        return 0;
    }
    memset(outbuf, 0, maxBytes);

    while (count < maxBytes) {
        count++;
        switch (inbuf[(count-1)]) {
            /* new line characters need to be here */
            case '\n':   /* linux new line only for now */
                outbuf[(count-1)] = 0;   /* null term string */
                return count;
            default:
                outbuf[(count-1)] = inbuf[(count-1)];
        }
    }
    return count;
}

/*
 * Took these functions out of amhlib.c & i2v_util.c.
 * The threads for IMF and AMH were clobbering each other.
 * Duplicating code here to make thread safe and re-entrant.
 * Pros: easy, faster and works.
 * Cons: duplicate code.
 */
STATIC void amhUpdateMinute(char_t GCC_UNUSED_VAR *tag, char_t GCC_UNUSED_VAR *parsedItem, char_t *value, void *configItem, uint32_t *status)
{
    /* Min strlen is 17 */
    /* format: mm/dd/yyyy, hh:mm */
    uint32_t *result = (uint32_t *)configItem;
    bool_t valid = WFALSE;
    i2vTimeT clock;
    struct tm tval;
    char_t *tok = NULL;
    char_t *saveptr = NULL;

    if ((NULL == value) || (NULL == configItem)) {
        return;
    }
#if defined (EXTRA_DEBUG)
    if(strlen(value) < 5) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"amhUpdateMinute: value too short=%lu=%s.\n",strlen(value) , value);
    }

    if(25 < strlen(value)) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"amhUpdateMinute: value too long=%lu=%s.\n",strlen(value) , value);
    }
#endif

    memset(&tval, 0, sizeof(tval));

    tok = strtok_r(value, "/",&saveptr);

    if ((NULL != tok) && (strcmp(tok, " "))) {    /* should be impossible to get a space but just in case */
        tval.tm_mon = (int)(strtoul(tok, NULL, 10) - 1);
        tok = strtok_r(NULL, "/",&saveptr);
    } else {
        /* blank means current time */
        amhUtilGetUTCTime(&clock);
        *result = CONVERT_TO_UTC(clock.year, clock.yday, clock.hour, clock.min, clock.sec);
        valid = WTRUE;
        goto setresult;
    }
    if (NULL != tok) {
        tval.tm_mday = (int)strtoul(tok, NULL, 10);
        tok = strtok_r(NULL, ",",&saveptr);

    } else {
        goto setresult;
    }
    if (NULL != tok) {
        tval.tm_year = (int)(strtoul(tok, NULL, 10) - 1900);
        tok = strtok_r(NULL, ":",&saveptr);
    } else {
        goto setresult;
    }
    if (NULL != tok) {
        tval.tm_hour = (int)strtoul(tok, NULL, 10);
        tok = strtok_r(NULL, ":",&saveptr);
    } else {
        goto setresult;
    }
    if (NULL != tok) {
        tval.tm_min = (int)strtoul(tok, NULL, 10);
        valid = WTRUE;
    } else {
        goto setresult;
    }

    mktime(&tval);
    *result = CONVERT_TO_UTC((tval.tm_year + 1900), tval.tm_yday, tval.tm_hour, tval.tm_min, tval.tm_sec);

setresult:
    if (NULL != status) *status = (valid) ? I2V_RETURN_OK : I2V_RETURN_FAIL;
}
STATIC void amhUpdateType(char_t GCC_UNUSED_VAR *tag, char_t GCC_UNUSED_VAR *parsedItem, char_t *value, void *configItem, uint32_t *status)
{
    amhBitmaskType *result = (amhBitmaskType *)configItem;
 
    if ((NULL == value) || (NULL == configItem) || (NULL == status)) {
        return;
    }

    *status = I2V_RETURN_FAIL;
    *result = AMH_MSG_UNKNOWN_MASK;

    if (!strcmp(value, "SPAT")) {
        *status = I2V_RETURN_OK;
        *result = AMH_MSG_SPAT_MASK;
        return;
    }
    if (!strcmp(value, "MAP")) {
        *status = I2V_RETURN_OK;
        *result = AMH_MSG_MAP_MASK;
        return;
    }
    if (!strcmp(value, "TIM")) {
        *status = I2V_RETURN_OK;
        *result = AMH_MSG_TIM_MASK;
        return;
    }
    if (!strcmp(value, "RTCM")) {
        *status = I2V_RETURN_OK;
        *result = AMH_MSG_RTCM_MASK;
        return;
    }
    if (!strcmp(value, "SPAT16")) {
        *status = I2V_RETURN_OK;
        *result = AMH_MSG_SPAT_MASK;
        return;
    }
    if (!strcmp(value, "MAP16")) {
        *status = I2V_RETURN_OK;
        *result = AMH_MSG_MAP_MASK;
        return;
    }
    if (!strcmp(value, "TIM16")) {
        *status = I2V_RETURN_OK;
        *result = AMH_MSG_TIM_MASK;
        return;
    }
    if (!strcmp(value, "RTCM16")) {
        *status = I2V_RETURN_OK;
        *result = AMH_MSG_RTCM_MASK;
        return;
    }
    if (!strcmp(value, "ICA")) {
        *status = I2V_RETURN_OK;
        *result = AMH_MSG_ICA_MASK;
        return;
    }
    if (!strcmp(value, "PDM")) {
        *status = I2V_RETURN_OK;
        *result = AMH_MSG_PDM_MASK;
        return;
    }
    if (!strcmp(value, "RSA")) {
        *status = I2V_RETURN_OK;
        *result = AMH_MSG_RSA_MASK;
        return;
    }
    if (!strcmp(value, "SSM")) {
        *status = I2V_RETURN_OK;
        *result = AMH_MSG_SSM_MASK;
        return;
    }
    if (!strcmp(value, "CSR")) {
        *status = I2V_RETURN_OK;
        *result = AMH_MSG_CSR_MASK;
        return;
    }
    if (!strcmp(value, "EVA")) {
        *status = I2V_RETURN_OK;
        *result = AMH_MSG_EVA_MASK;
        return;
    }
    if (!strcmp(value, "NMEA")) {
        *status = I2V_RETURN_OK;
        *result = AMH_MSG_NMEA_MASK;
        return;
    }
    if (!strcmp(value, "PSM")) {
        *status = I2V_RETURN_OK;
        *result = AMH_MSG_PSM_MASK;
        return;
    }
    if (!strcmp(value, "PVD")) {
        *status = I2V_RETURN_OK;
        *result = AMH_MSG_PVD_MASK;
        return;
    }
    if (!strcmp(value, "SRM")) {
        *status = I2V_RETURN_OK;
        *result = AMH_MSG_SRM_MASK;
        return;
    }
    if (!strcmp(value, "BSM")) {
        *status = I2V_RETURN_OK;
        *result = AMH_MSG_BSM_MASK;
        return;
    }
    if (!strcmp(value, "TEST00")) {
        *status = I2V_RETURN_OK;
        *result = AMH_MSG_TEST00_MASK;
        return;
    }
    if (!strcmp(value, "TEST01")) {
        *status = I2V_RETURN_OK;
        *result = AMH_MSG_TEST01_MASK;
        return;
    }
    if (!strcmp(value, "TEST02")) {
        *status = I2V_RETURN_OK;
        *result = AMH_MSG_TEST02_MASK;
        return;
    }
    if (!strcmp(value, "TEST03")) {
        *status = I2V_RETURN_OK;
        *result = AMH_MSG_TEST03_MASK;
        return;
    }
    if (!strcmp(value, "TEST04")) {
        *status = I2V_RETURN_OK;
        *result = AMH_MSG_TEST04_MASK;
        return;
    }
    if (!strcmp(value, "TEST05")) {
        *status = I2V_RETURN_OK;
        *result = AMH_MSG_TEST05_MASK;
        return;
    }
    if (!strcmp(value, "TEST06")) {
        *status = I2V_RETURN_OK;
        *result = AMH_MSG_TEST06_MASK;
        return;
    }
    if (!strcmp(value, "TEST07")) {
        *status = I2V_RETURN_OK;
        *result = AMH_MSG_TEST07_MASK;
        return;
    }
    if (!strcmp(value, "TEST08")) {
        *status = I2V_RETURN_OK;
        *result = AMH_MSG_TEST08_MASK;
        return;
    }
    if (!strcmp(value, "TEST09")) {
        *status = I2V_RETURN_OK;
        *result = AMH_MSG_TEST09_MASK;
        return;
    }
    if (!strcmp(value, "TEST10")) {
        *status = I2V_RETURN_OK;
        *result = AMH_MSG_TEST10_MASK;
        return;
    }
    if (!strcmp(value, "TEST11")) {
        *status = I2V_RETURN_OK;
        *result = AMH_MSG_TEST11_MASK;
        return;
    }
    if (!strcmp(value, "TEST12")) {
        *status = I2V_RETURN_OK;
        *result = AMH_MSG_TEST12_MASK;
        return;
    }
    if (!strcmp(value, "TEST13")) {
        *status = I2V_RETURN_OK;
        *result = AMH_MSG_TEST13_MASK;
        return;
    }
    if (!strcmp(value, "TEST14")) {
        *status = I2V_RETURN_OK;
        *result = AMH_MSG_TEST14_MASK;
        return;
    }
    if (!strcmp(value, "TEST15")) {
        *status = I2V_RETURN_OK;
        *result = AMH_MSG_TEST15_MASK;
        return;
    }
    set_amh_error_state(AMH_PROCESS_BAD_MSG_TYPE);
    return; /* *status == I2V_RETURN_FAIL */
}

STATIC void amhUpdateTxMode(char_t GCC_UNUSED_VAR *tag, char_t GCC_UNUSED_VAR *parsedItem, char_t *value, void *configItem, uint32_t *status)
{
    bool_t *result = (bool_t *)configItem;
    bool_t valid = WFALSE;
 
    if ((NULL == value) || (NULL == configItem)) {
        return;
    }

    if (!strcmp(value, "CONT")) {
        *result = WTRUE;
        valid = WTRUE;
    } else if (!strcmp(value, "ALT")) {
        *result = WFALSE;
        valid = WTRUE;
    }

    if (NULL != status) *status = (valid) ? I2V_RETURN_OK : I2V_RETURN_FAIL;
}

/* 20140903 - adding in support for any numeric channel number within range -
   this was a request from the june plugfest */
STATIC void amhUpdateTxChannel(char_t GCC_UNUSED_VAR *tag, char_t GCC_UNUSED_VAR *parsedItem, char_t *value, void *configItem, uint32_t *status)
{
    uint8_t *result = (uint8_t *)configItem;
    bool_t valid = WFALSE;
 
    if ((NULL == value) || (NULL == configItem)) {
        return;
    }

    if (!strcmp(value, "CCH")) {
        *result = I2V_CONTROL_CHANNEL;   /* control channel */
        valid = WTRUE;
    } else if (!strcmp(value, "SCH")) {
        *result = AMH_NEED_SCH;   /* indication to get service channel */
        valid = WTRUE;
    } else if ((*result = (uint8_t)strtoul(value, NULL, 10))) {
        /* above will set value to 0 if not a number and this block not entered */
        if ((*result >= I2V_MIN_SVC_CHANNEL) && (*result <= I2V_MAX_SVC_CHANNEL)) {
            valid = WTRUE;
        } /* us band specific - may need to consider other country frequencies (i.e. macro for vals) */
    }

    if (NULL != status) *status = (valid) ? I2V_RETURN_OK : I2V_RETURN_FAIL;
}

STATIC void amhUpdateBool(char_t GCC_UNUSED_VAR *tag, char_t GCC_UNUSED_VAR *parsedItem, char_t *value, void *configItem, uint32_t *status)
{
    bool_t *result = (bool_t *)configItem;
    bool_t valid = WFALSE;
 
    if ((NULL == value) || (NULL == configItem)) {
        return;
    }

    if (!strcmp(value, "True")) {
        *result = WTRUE;
        valid = WTRUE;
    } else if (!strcmp(value, "False")) {
        *result = WFALSE;
        valid = WTRUE;
    }

    if (NULL != status) *status = (valid) ? I2V_RETURN_OK : I2V_RETURN_FAIL;
}
void amhUpdateFloatValue(void *configItem, char_t *val, char_t *min, char_t *max, char_t  **trange, uint32_t *status, i2vLogWrapper GCC_UNUSED_VAR *logger)
{
    float32_t  *cfg = (float32_t  *)configItem;
    float32_t   value = strtof(val, NULL);
    float32_t   minval = 0.0f, maxval = 0.0f;
    int32_t   i = 0;
    bool_t   valueValid = WFALSE;

    if (strcmp(min, I2V_CFG_UNDEFINED) != 0) minval = strtof(min, NULL);
    if (strcmp(max, I2V_CFG_UNDEFINED) != 0) maxval = strtof(max, NULL);
    if ((value >= minval) && (value <= maxval)) {
        *cfg = value;
#if defined(EXTRA_DEBUG)
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"%f {%f, %f} (float32_t)\n", value, minval, maxval);
#endif
    } else if( (strcmp(min, I2V_CFG_UNDEFINED) == 0) && (strcmp(max, I2V_CFG_UNDEFINED) == 0) ) {
          /* No Min/Max limits for this value. */
          *cfg = value;
    } else {
        if (value < minval) {
            *cfg = minval;
#if defined(EXTRA_DEBUG)
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"ERROR, %s(): MIN LIMIT, value (%f) < minval (%f). value set to minval\n", __FUNCTION__, value, minval);
#endif
            *status = I2V_RETURN_MIN_LIMIT_FAIL;
        } else if (value > maxval) {
            *cfg = maxval;
#if defined(EXTRA_DEBUG)
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"ERROR, %s(): MAX LIMIT, value (%f) > maxval (%f). value set to maxval\n", __FUNCTION__, value, maxval);
#endif
            *status = I2V_RETURN_MAX_LIMIT_FAIL;
        }
    }
    if( trange == NULL ) return;

    /* Ensure that the specified value is equal to a table discrete 
     * and that min/max agree with table values. 
     */
    while( *(trange + i) != NULL ) {
        if( !valueValid && (value  == strtof(*(trange+i), NULL)) ) { 
            valueValid = WTRUE;
            break;
        }
        i++;
    }

    /* Current value is not contained in discrete value range */
    if( !valueValid ) {
#if defined(EXTRA_DEBUG)
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"ERROR, %s(): Current value= %f is NOT in discrete range\n", __FUNCTION__, value);
#endif
        *status = I2V_RETURN_RANGE_LIMIT_FAIL;  // Current value NOT in discrete range
    }
}

STATIC void amhUpdatePayload( char_t GCC_UNUSED_VAR * tag
                            , char_t GCC_UNUSED_VAR * parsedItem
                            , char_t                * value  /*input*/
                            , void                  * configItem /*output*/
                            , uint32_t              * status)
{
    payloadType * result  = (payloadType *)configItem;
    bool_t        valid   = WFALSE;
    uint16_t      i       = 0;
    uint16_t      valsize = 0;
    uint8_t       dummy   = 0;
    char_t buffer[RSU_ALERT_MSG_LENGTH_MAX];
    if ((NULL == value) || (NULL == result) || (NULL == configItem)) {
        #if defined (EXTRA_DEBUG)
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"amhUpdatePayload:NULL Inputs::Rejected.\n");
        #endif
        valid = WFALSE;
    }  else {

        valid = WTRUE;
        valsize = strlen(value); 
      
        #if defined (EXTRA_EXTRA_EXTRA_DEBUG)
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"amhUpdatePayload:[%lu][%s][%d]\n",valsize,value,result->count);
        #endif

        /* Too small ? */
        if(valsize < 6) {
            #if defined (EXTRA_DEBUG)
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"amhUpdatePayload:number of nibbles(%lu) < 6. Rejected.\n", valsize);
            #endif
            if(0x0 == (is_amh_error_set(AMH_RAMLIST_LOAD_TOO_SMALL))) {
                set_amh_error_state(AMH_RAMLIST_LOAD_TOO_SMALL);
                memset(buffer,0x0,sizeof(buffer));
                snprintf(buffer,sizeof(buffer), "Payload is too small, payload(%d) < min bytes(%d): Rejected.", valsize, 6);
                sendnotification(buffer, rsuAlertLevel_error);
            }
            valid = WFALSE;
        }

        /* Too big? */
        if(RSU_MIB_MAX_PAYLOAD_OCTET_STRING < valsize) {
            #if defined (EXTRA_DEBUG)
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"amhUpdatePayload: RSU_MIB_MAX_PAYLOAD_OCTET_STRING < number of nibbles(%d). Rejected.\n", valsize);
            #endif
            if(0x0 == (is_amh_error_set(AMH_RAMLIST_LOAD_TOO_BIG))) {
                set_amh_error_state(AMH_RAMLIST_LOAD_TOO_BIG);
                memset(buffer,0x0,sizeof(buffer));
                snprintf(buffer,sizeof(buffer), "Payload is too big, max bytes(%d) < payload(%d): Rejected.", RSU_MIB_MAX_PAYLOAD_OCTET_STRING , valsize);
                sendnotification(buffer, rsuAlertLevel_error);
            }
            valid = WFALSE;
        }

        /* Too odd? */
        if( 1 == (valsize % 2)) {

            #if defined (EXTRA_DEBUG)
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"amhUpdatePayload:ODD number of nibbles::size=%d Rejected.\n",valsize);
            #endif
            if(0x0 == (is_amh_error_set(AMH_RAMLIST_LOAD_ODD_SIZE))) {
                set_amh_error_state(AMH_RAMLIST_LOAD_ODD_SIZE);
                memset(buffer,0x0,sizeof(buffer));
                snprintf(buffer,sizeof(buffer), "Payload is odd number, should be even: bytes(%d): Rejected.", valsize);
                sendnotification(buffer, rsuAlertLevel_error);
            }
            valid = WFALSE;
        }

        /* Just right ! */
        if(WTRUE == valid) {

            #if defined (EXTRA_EXTRA_DEBUG)
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"amhUpdatePayload:[%d][%s]\n",valsize,value);
            #endif
 
            for(i=0, result->count=0; (WTRUE == valid) && (result->count < MAX_WSM_DATA) && (i < valsize ); i++) {

                dummy = my_i2v_ascii_2_hex_nibble(value[i]);

                if(0xff == dummy) {

                    #if defined (EXTRA_DEBUG)
                    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"amhUpdatePayload:Not HEX values i(%d)=[%c] Rejected.\n",i,value[i]);
                    #endif

                    if (NULL != status) 
                        *status = I2V_RETURN_FAIL;

                    valid = WFALSE;
                    if(0x0 == (is_amh_error_set(AMH_RAMLIST_LOAD_NOT_ASCII_DATA))) {
                        set_amh_error_state(AMH_RAMLIST_LOAD_NOT_ASCII_DATA);
                        memset(buffer,0x0,sizeof(buffer));
                        snprintf(buffer,sizeof(buffer), "Payload contains non-HEX values i(%d)=[%c]: Rejected.\n",i,value[i]);
                        sendnotification(buffer, rsuAlertLevel_error);
                    }
                    break; /* Error out */
                  
                } else {

                    if(0 == (i % 2)) {
                        result->payload[result->count] = (0xf) & dummy;
                    } else {
                        result->payload[result->count]  =  result->payload[result->count] << 4; 
                        result->payload[result->count] |=  (0xf) & dummy;
                        result->count++;
                    }
                }
            }

            #if defined (EXTRA_EXTRA_DEBUG)
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"amhUpdatePayload2:[\n");
            for(i=0; (i < MAX_WSM_DATA) && (i < result->count) ; i++) {
                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"%x,\n",result->payload[i]);
            }
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"][%d]\n",result->count);
            #endif
        }
    }

    if (NULL != status) *status = (valid) ? I2V_RETURN_OK : I2V_RETURN_FAIL;
}

STATIC void amhUpdateUint32Value(void *configItem, char_t *val, char_t *min, char_t *max, char_t **trange, uint32_t *status, i2vLogWrapper GCC_UNUSED_VAR *logger)
{
    uint32_t * cfg        = (uint32_t *)configItem;
    uint32_t   value      = (uint32_t)strtoul(val, NULL, 0);
    uint32_t   minval     = 0, maxval = 0;
    int32_t    i          = 0;
    bool_t     valueValid = WFALSE;

    *status = I2V_RETURN_OK;

    if (strcmp(min, I2V_CFG_UNDEFINED) != 0 ) minval = strtoul(min, NULL, 0);
    if (strcmp(max, I2V_CFG_UNDEFINED) != 0 ) maxval = strtoul(max, NULL, 0);

    if ((value >= minval) && (value <= maxval)) {
        *cfg = value;
#if defined(EXTRA_DEBUG)
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"%u {%u, %u} (uint32)\n", value, minval, maxval);
#endif
    } 
    else if( (strcmp(min, I2V_CFG_UNDEFINED) == 0) && (strcmp(max, I2V_CFG_UNDEFINED) == 0) ) {
      /* No Min/Max limits for this value. */
      *cfg = value;
    } else {
        if (value < minval) {
            *cfg = minval;
#if defined(EXTRA_DEBUG)
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"ERROR, %s(): MIN LIMIT, value= %u < minval= %u.\n", __FUNCTION__, value, minval);
#endif
            *status = I2V_RETURN_MIN_LIMIT_FAIL;
        } 
        if (value > maxval) {
            *cfg = maxval;
#if defined(EXTRA_DEBUG)
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"ERROR, %s: MAX LIMIT, value= %u > maxval= %u.\n", __FUNCTION__, value, maxval);
#endif
            *status = I2V_RETURN_MAX_LIMIT_FAIL;
        }
    }

    if( trange == NULL ) return;

    /* Ensure that the specified value is equal to a table discrete. */
    while( *(trange + i) != NULL ) {
        if( !valueValid && (value  == strtoul(*(trange+i), NULL, 0)) ) {
            valueValid = WTRUE;
            break;
        }
        i++;
    }

    /* Value is not contained in discrete value range */
    if( !valueValid ) {
#if defined(EXTRA_DEBUG)
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"ERROR, %s(): Current value= %d is NOT in discrete range\n", __FUNCTION__, value);
#endif
        *status = I2V_RETURN_RANGE_LIMIT_FAIL;  // Current value NOT in discrete range
    }
}

STATIC void amhUpdateUint8Value(void *configItem, char_t *val, char_t *min, char_t *max, char_t **trange, uint32_t *status, i2vLogWrapper GCC_UNUSED_VAR *logger)
{
    uint8_t * cfg        = (uint8_t *)configItem;
    uint8_t   value      = (uint8_t)strtoul(val, NULL, 0);
    uint8_t   minval     = 0, maxval = 0;
    int32_t   i          = 0;
    bool_t    valueValid = WFALSE;
    
    *status = I2V_RETURN_OK;

    if (strcmp(min, I2V_CFG_UNDEFINED) != 0 ) minval = strtoul(min, NULL, 0);
    if (strcmp(max, I2V_CFG_UNDEFINED) != 0 ) maxval = strtoul(max, NULL, 0);
    if ((value >= minval) && (value <= maxval)) {
        *cfg = value;
#if defined(EXTRA_DEBUG)
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"%d {%d, %d} (uint8)\n", value, minval, maxval);
#endif
    } 
    else if( (strcmp(min, I2V_CFG_UNDEFINED) == 0) && (strcmp(max, I2V_CFG_UNDEFINED) == 0) ){
        /* No Min/Max limits for this value. */
        *cfg = value;
    }
    else {
        if (value < minval) {
            *cfg = minval;
#if defined(EXTRA_DEBUG)
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"ERROR, %s(): MIN LIMIT, value= %d < minval= %d.\n", __FUNCTION__, value, minval);
#endif
            *status = I2V_RETURN_MIN_LIMIT_FAIL;
        } 
        if (value > maxval) {
            *cfg = maxval;
#if defined(EXTRA_DEBUG)
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"ERROR, %s(): MAX LIMIT, value= %d > maxval= %d.\n", __FUNCTION__, value, maxval);
#endif
            *status = I2V_RETURN_MAX_LIMIT_FAIL;
        }
    }

    if( trange == NULL ) return;

    /* Ensure that the specified value is equal to a table discrete. */
    while( *(trange + i) != NULL ) {
        if( !valueValid && (value  == strtoul(*(trange+i), NULL, 0)) ) {
            valueValid = WTRUE;
            break;
        }
        i++;
    }
    /* Value is not contained in discrete range */
    if( !valueValid ) {
#if defined(EXTRA_DEBUG)
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"ERROR, %s(): Current value= %d is NOT in discrete range\n", __FUNCTION__, value);
#endif
        *status = I2V_RETURN_RANGE_LIMIT_FAIL;  // Current value NOT in discrete range
    }
}

/* This called by amh.c only. */
STATIC uint32_t AMHParseConfMem(char_t *inbuf, uint32_t bufsize, cfgItemsTypeT *cfgItems, uint32_t numCfgItems)
{
  bool_t     cfgFound[numCfgItems];
  uint32_t   i = 0, parserStatus = I2V_RETURN_OK;
  uint32_t   retVal = I2V_RETURN_OK;
  char_t  *  bptr = inbuf;
  uint32_t   bytesRead = 0, remCount = bufsize;
  bool_t     looping = WTRUE;
  char_t   * ptr = NULL;
  uint32_t   j = 0;

    memset(cfgFound, 0, sizeof(cfgFound));  /* required for jjg comment later in this fxn */

    if (NULL == inbuf) {
#if defined(EXTRA_DEBUG)
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"AMH_SAR_PARSE_CONF_NULL_INPUT.\n");
#endif
        set_amh_error_state(AMH_SAR_PARSE_CONF_NULL_INPUT);
        return AMH_SAR_PARSE_CONF_NULL_INPUT;
    }

    while (looping) {

        /* Never trust strings. Junk left behind can really come back to haunt */
        memset(buf, '\0', sizeof(buf));
        memset(dummy, '\0', sizeof(dummy));
        memset(val, '\0', sizeof(val));

        /* Reads to newline then stops. Avoids conflict with unix vs dos file types. */
        bytesRead = memGets(buf, MEM_PARSE_CONF_GET_MAX_BUF, bptr);
        remCount = (bytesRead > remCount) ? 0: (remCount - bytesRead);

        /* End of reading data but still need to parse it. */
        if (!bytesRead || !remCount) {
            looping = WFALSE;
        }

        if (remCount) bptr += (bytesRead);
        /* Identify and remove comment sections */
        if ((ptr = strrchr(buf, '#')) != NULL) *ptr = '\0';

        /* Remove white space */
        memset(dummy, '\0', sizeof(dummy));
        for(i=0,j=0;(i < MEM_PARSE_CONF_MAX_BUF) && ('\0' != buf[i]); i++) {

            switch(buf[i]) {
                case ' ' :
                case '\t':
                case '\n':
                case '\r':
                    break;
                default:
                    dummy[j] = buf[i];
                    j++;
                    break;
            }
        }

        for(i=0;(i<MEM_PARSE_CONF_MAX_BUF) && ('\0' != dummy[i]);i++) {
            buf[i] = dummy[i];
        }

        if(i < MEM_PARSE_CONF_MAX_BUF)
            buf[i] = '\0';
        else
            buf[MEM_PARSE_CONF_MAX_BUF-1] = '\0';

        /* Blank/Comment lines */
        if ((strlen(buf) == 0) || (buf[0] == '#')) {
#if defined(EXTRA_DEBUG)
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Blank/Comment lines.\n");
#endif
            set_amh_error_state(AMH_SAR_PARSE_CONF_BLANK_INPUT); /* Harmless, keep going. */
            continue;
        }

        /* Identify value lines */
        if ((ptr = strrchr(buf, '=')) == NULL) {
#if defined(EXTRA_DEBUG)
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%d)  Rejected - No assignment: %s\n",getpid(),buf);
#endif
            set_amh_error_state(AMH_SAR_PARSE_CONF_BLANK_INPUT); /* Harmless, keep going. */
            continue;
        }

        /* Replace '=' with NULL; point ptr to value in string and copy into 'val' */
        *ptr = '\0';
        strcpy(val, ptr+1);
        /* Do not print payload like below.
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"  Config:  %s:\t val: %s\n", buf, val);
        */
        for (i = 0; i < numCfgItems; i++) {
            /* custom functions may have tags that don't match length of parsed buf */

            /* 
             * If buf has more to it than cfgItems[i].tag then we still get a match so long as a custom function exists.
             * i.e. "Payload" from cfgItem.tag will match "PayloadNotForThisTaskPleaseDontUseMe" from buf[]
             */
            if (((strncmp(buf, cfgItems[i].tag, strlen(cfgItems[i].tag)) == 0) && (strlen(buf) == strlen(cfgItems[i].tag))) 
             || ((strncmp(buf, cfgItems[i].tag, strlen(cfgItems[i].tag)) == 0) && (cfgItems[i].customUpdateFxn != NULL))) {

                if (cfgItems[i].customUpdateFxn == NULL) {
                    (*cfgItems[i].updateFunction)(cfgItems[i].itemToUpdate, val, UNDEFINED, UNDEFINED, NULL, &parserStatus, NULL);
                } else {
                    /* custom functions may not print out text to log terminate open ended log msg printed above */
                    (*cfgItems[i].customUpdateFxn)(cfgItems[i].tag, buf, val, cfgItems[i].itemToUpdate, &parserStatus);
                }

                if( parserStatus == I2V_RETURN_OK ) {
                    cfgFound[i] = WTRUE;
                } else if(   (parserStatus == I2V_RETURN_STR_MAX_LIMIT_FAIL)
                          || (parserStatus == I2V_RETURN_MIN_LIMIT_FAIL)
                          || (parserStatus == I2V_RETURN_MAX_LIMIT_FAIL) ) {
#if defined(EXTRA_DEBUG)
                    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"AMHParseConfMem: Limit error on '%s'. Examine Debug Log for details\n", cfgItems[i].tag);
#endif
                    /*jjg cleanup - do we really need to return here if later assuming override not set we fail anyway?
                     *pro: if we fail here it speeds up initialization time
                     *con: if we fail here and calling process doesn't check return, other config items will be missing */
                    looping = WFALSE;
                    cfgFound[i] = WFALSE;
                    continue;
                }
                else {
#if defined(EXTRA_DEBUG)
                    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"AMHParseConfMem: ~Discrete value range error on '%s'. Examine Debug Log for details\n", cfgItems[i].tag);
#endif
                    looping = WFALSE;
                    cfgFound[i] = WFALSE;
                    continue;
                }

                break; /* found our item so break out either way. */
            }
        }
    }
//TODO: Legacy, maybe should be on be default?
    /* Legacy item. */
    /* if (WTRUE == amhCfg.validateData) */  
    {
        for (i = 0; i < NUMITEMS(cfgFound); i++) {
            if (!cfgFound[i]) {
#if defined(EXTRA_DEBUG)
                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"I2V_RETURN_INVALID_CONFIG: Config values for '%s' not found.\n", cfgItems[i].tag);
#endif
                set_amh_error_state(AMH_SAR_PARSE_CONF_VALIDATE_FAIL);
                retVal = I2V_RETURN_INVALID_CONFIG;
            }
        }
    }
    return retVal;
}
STATIC bool_t amhLoadRecData(char_t *fname, char_t *basename, amhStoredRecT *record)
{
  float32_t  dummyVersion = 0.0f;
  FILE     * f    = NULL;
  uint32_t   size = 0;
  char_t     buf[MAX_AMH_FILE_SIZE] = {0};
//TODO should be one error for each type
    /* DO NOT mix custom function with "regular". */
    cfgItemsTypeT recorddata[] = {
        {"Version",       (void *)amhUpdateFloatValue,  &dummyVersion,       NULL,(ITEM_VALID| UNINIT)},
        {"Type",          NULL,                         &record->encoding,   (void *)amhUpdateType,(ITEM_VALID| UNINIT)},
        {"PSID",          (void *)amhUpdateUint32Value, &record->psid,       NULL,(ITEM_VALID| UNINIT)},
        {"Priority",      (void *)amhUpdateUint8Value,  &record->priority,   NULL,(ITEM_VALID| UNINIT)},
        {"TxMode",        NULL,                         &record->continuous, (void *)amhUpdateTxMode,(ITEM_VALID| UNINIT)},
        {"TxChannel",     NULL,                         &record->channel,    (void *)amhUpdateTxChannel,(ITEM_VALID| UNINIT)},
        {"TxInterval",    (void *)amhUpdateUint8Value,  &record->interval,   NULL,(ITEM_VALID| UNINIT)},
        {"DeliveryStart", NULL,                         &record->startMin,   (void *)amhUpdateMinute,(ITEM_VALID| UNINIT)},
        {"DeliveryStop",  NULL,                         &record->endMin,     (void *)amhUpdateMinute,(ITEM_VALID| UNINIT)},
        {"Signature",     NULL,                         &record->signature,  (void *)amhUpdateBool,(ITEM_VALID| UNINIT)},
        {"Encryption",    NULL,                         &record->encryption, (void *)amhUpdateBool,(ITEM_VALID| UNINIT)},
        {"Payload",       NULL,                         &record->data,       (void *)amhUpdatePayload,(ITEM_VALID| UNINIT)},
    };

    if ((NULL == fname) || (NULL == basename)) {
        set_amh_error_state(AMH_RAMLIST_LOAD_NULL_INPUT_ERROR);
        return WFALSE;
    }
    if (NULL == record) {
        set_amh_error_state(AMH_RAMLIST_LOAD_NULL_INPUT_ERROR);
        return WFALSE;
    }
    if (NULL == (f=fopen(fname, "r"))) {
        set_amh_error_state(AMH_RAMLIST_LOAD_FOPEN_ERROR);
        return WFALSE;
    }

    /* because payload can be longer than i2vUtilParseConfFile string limit of 100 bytes,
       read in the file to the buffer and then parse the memory */
    while (!feof(f) && (size <= MAX_AMH_FILE_SIZE)) {
        size += fread(&buf[size], 1, 256, f);   /* 256 bytes at a time */
        /* NOTE: 256 divides evenly into MAX_AMH_FILE_SIZE but it may be wise to add a check
           on size to make sure the the fread never tries to read more bytes in than 
           MAX_AMH_FILE_SIZE - size */
    }
    fclose(f);
    /* Set limit for how small message could be within reason. NOT EXACT just rough check for bogus stuff. */
    if(size < MIN_SAR_FILE_SIZE) {
        set_amh_error_state(AMH_RAMLIST_FILE_TOO_SMALL);
        return WFALSE;
    }
    if(MAX_AMH_FILE_SIZE <= size) {
        set_amh_error_state(AMH_RAMLIST_FILE_TOO_BIG);
        return WFALSE;
    }
    /* Parse file. */
    if (AMH_AOK != AMHParseConfMem(buf, size, recorddata, NUMITEMS(recorddata))) {
        set_amh_error_state(AMH_RAMLIST_PARSE_ERROR);
        return WFALSE;
    }
    /* new requirement to check channel after parsing */
    if (record->channel == AMH_NEED_SCH) {
        record->channel = amhCfg.channelNum;   /* service channel */
    }
    /* priority validation */
    if (record->priority > MAX_WSM_PRIORITY) {
        set_amh_error_state(AMH_RAMLIST_PRIORITY_ERROR);
        record->priority = MAX_WSM_PRIORITY;
        //return WFALSE;
    }
    /* Enforce I2V unified settings */
    if ((record->channel != cfg.uchannel) && (record->channel != I2V_CONTROL_CHANNEL)) {
        record->channel = cfg.uchannel; 
    }

    /* save name */ 
    size = strlen(basename) + 1;

    if(RSU_MSG_FILENAME_LENGTH_MAX <= size)
        size = RSU_MSG_FILENAME_LENGTH_MAX - 1; /*leave room for NULL*/
    
    memset(record->myName, '\0', RSU_MSG_FILENAME_LENGTH_MAX);  
    memcpy(record->myName, basename, size);
    /* If this is MIB item then prefix of name is index in rsuMsgRepeatTable i.e. 1.txt is index 1. 
     *  WEB GUI doesn't understand this but the 4.1 MIB is not supported now. 
     */
    return WTRUE;
}
/*
 * End of Legacy Implementation: RSU 4.1 etc.
 */

/* 
 * Start of NTCIP-1218 implementation. 
 */
STATIC void print_msgRepeat(RsuMsgRepeatStatusEntry_t * msgRepeat)
{
  if(NULL == msgRepeat) {
#if defined(EXTRA_DEBUG)
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"print_msgRepeat: NUll input.");
#endif
      return;
  }
#if defined(EXTRA_DEBUG)
  /* One line, compact i.e."20220523_194747 |    amh    | INFO |1,1:1,1,00008002,180,1000,7,C0,07E60517132D1500:07E6051713321500,241:001280ED." */
  I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"%d,%d:%d,%d,%02X%02X%02X%02X,%d,%d,%d,%02X,%02X%02X%02X%02X%02X%02X%02X%02X:%02X%02X%02X%02X%02X%02X%02X%02X,%d:%02X%02X%02X%02X.\n",
                       msgRepeat->rsuMsgRepeatIndex
                      ,msgRepeat->rsuMsgRepeatStatus
                      ,msgRepeat->status
                      ,msgRepeat->rsuMsgRepeatEnable
                      ,msgRepeat->rsuMsgRepeatPsid[0]
                      ,msgRepeat->rsuMsgRepeatPsid[1]
                      ,msgRepeat->rsuMsgRepeatPsid[2]
                      ,msgRepeat->rsuMsgRepeatPsid[3]
                      ,msgRepeat->rsuMsgRepeatTxChannel
                      ,msgRepeat->rsuMsgRepeatTxInterval
                      ,msgRepeat->rsuMsgRepeatPriority
                      ,msgRepeat->rsuMsgRepeatOptions
                      ,msgRepeat->rsuMsgRepeatDeliveryStart[0]
                      ,msgRepeat->rsuMsgRepeatDeliveryStart[1]
                      ,msgRepeat->rsuMsgRepeatDeliveryStart[2]
                      ,msgRepeat->rsuMsgRepeatDeliveryStart[3]
                      ,msgRepeat->rsuMsgRepeatDeliveryStart[4]
                      ,msgRepeat->rsuMsgRepeatDeliveryStart[5]
                      ,msgRepeat->rsuMsgRepeatDeliveryStart[6]
                      ,msgRepeat->rsuMsgRepeatDeliveryStart[7]
                      ,msgRepeat->rsuMsgRepeatDeliveryStop[0]
                      ,msgRepeat->rsuMsgRepeatDeliveryStop[1]
                      ,msgRepeat->rsuMsgRepeatDeliveryStop[2]
                      ,msgRepeat->rsuMsgRepeatDeliveryStop[3]
                      ,msgRepeat->rsuMsgRepeatDeliveryStop[4]
                      ,msgRepeat->rsuMsgRepeatDeliveryStop[5]
                      ,msgRepeat->rsuMsgRepeatDeliveryStop[6]
                      ,msgRepeat->rsuMsgRepeatDeliveryStop[7]
                      ,msgRepeat->rsuMsgRepeatPayload_length 
                      ,msgRepeat->rsuMsgRepeatPayload[0]
                      ,msgRepeat->rsuMsgRepeatPayload[1]
                      ,msgRepeat->rsuMsgRepeatPayload[2]
                      ,msgRepeat->rsuMsgRepeatPayload[3]);
#endif
}
/* 
 * Return negative if bogus: Should be in MIB too.
 *
 * from ntcip-1218.h: MIB_DATEANDTIME_LENGTH: "2d-1d-1d,1d:1d:1d.1d": 8 OCTETS: DateAndTime: SNMPv2-TC
 *
 * from SNMPv2-TC.txt: A date-time specification.
 *
 *        field  octets  contents                  range
 *        -----  ------  --------                  -----
 *          1      1-2   year*                     0..65536
 *          2       3    month                     1..12
 *          3       4    day                       1..31
 *          4       5    hour                      0..23
 *          5       6    minutes                   0..59
 *          6       7    seconds                   0..60
 *                       (use 60 for leap-second)
 *          7       8    deci-seconds              0..9
 *
 *  Not supported, ignore.
 *  
 *          8       9    direction from UTC        '+' / '-'
 *          9      10    hours from UTC*           0..13
 *         10      11    minutes from UTC          0..59
 *
 *        Notes:
 *        - the value of year is in network-byte order
 *        - daylight saving time in New Zealand is +13
 *
 *   Tuesday May 26, 1992 at 1:30:15 PM EDT == 1992-5-26,13:30:15.0,-4:0
 */
STATIC int32_t DateAndTime_To_UTC_DSEC(uint8_t * DateAndTime, uint64_t * utcDsec, bool_t DebugOn )
{
  int32_t year        = 0;
  int32_t month       = 0;
  int32_t day         = 0;
  int32_t hour        = 0;
  int32_t minute      = 0;
  int32_t second      = 0;
  int32_t dsec        = 0;
  uint64_t testDsec   = 0;
  struct tm mib_tm;

  if(NULL == utcDsec) {
      return AMH_STAT_BAD_DATA;
  }

  year   = (int32_t)DateAndTime[0]; /* Year */
  year   = year << 8;
  year  += (int32_t)DateAndTime[1]; 
  month  = (int32_t)DateAndTime[2]; /* Month */
  day    = (int32_t)DateAndTime[3]; /* Day */
  hour   = (int32_t)DateAndTime[4]; /* Hour */
  minute = (int32_t)DateAndTime[5]; /* Minute */
  second = (int32_t)DateAndTime[6]; /* Second */
  dsec   = (int32_t)DateAndTime[7]; /* Dsecond */

  if((year < 0) || (65536 < year)){ /* Basic range check: */
      if(DebugOn) I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"DateAndTime_To_UTC_DSEC: Bad year.\n");
      return AMH_STAT_BAD_DATA;       /* From unsigned above so can't be less than zero really. */
  }
  year = year - 1900; /* adjust for mktime(). */
  if((month < 1) || (12 < month)){ /* Not exhaustive for month & day. See below for more. */
      if(DebugOn) I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"DateAndTime_To_UTC_DSEC: Bad month.\n");
      return AMH_STAT_BAD_DATA;
  }
  month = month - 1; /* adjust for mktime(). */
  if((day < 1) || (31 < day)){
      if(DebugOn) I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"DateAndTime_To_UTC_DSEC: Bad day.\n");
      return AMH_STAT_BAD_DATA;
  }
  if((hour < 0) || (23 < hour)){
      if(DebugOn) I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"DateAndTime_To_UTC_DSEC: Bad hour.\n");
      return AMH_STAT_BAD_DATA;
  }
  if((minute < 0) || (59 < minute)){
      if(DebugOn) I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"DateAndTime_To_UTC_DSEC: Bad minute.\n");
      return AMH_STAT_BAD_DATA;
  }
  if((second < 0) || (60 < second)){
      if(DebugOn) I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"DateAndTime_To_UTC_DSEC: Bad second.\n");
      return AMH_STAT_BAD_DATA;
  }
  if((dsec < 0) || (9 < dsec)){
      if(DebugOn) I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"DateAndTime_To_UTC_DSEC: Bad dsec.\n");
      return AMH_STAT_BAD_DATA;
  }

  mib_tm.tm_sec   = second;
  mib_tm.tm_min   = minute;
  mib_tm.tm_hour  = hour;
  mib_tm.tm_mday  = day;
  mib_tm.tm_mon   = month;
  mib_tm.tm_year  = year;
  mib_tm.tm_wday  = 0;
  mib_tm.tm_yday  = 0;
  mib_tm.tm_isdst = 0;

  /* Assumming our error checking above is correct this can't fail. */
  testDsec = (uint64_t)mktime(&mib_tm);
  if ((time_t)-1 == (time_t)testDsec){
      if(DebugOn) I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"DateAndTime_To_UTC_DSEC: mktime() failed.\n");
      return AMH_STAT_BAD_DATA;
  }
  testDsec *= (uint64_t)10;   /* Convert to deci-seconds and add final value. */
  testDsec += (uint64_t)dsec;
  *utcDsec = testDsec;
  return AMH_AOK;
}

STATIC int32_t check_file_existence(const char_t *filename, uint32_t filesize, int32_t index)
{
  struct stat buffer;

  if( ((index <  RSU_SAR_MESSAGE_MIN) || (RSU_SAR_MESSAGE_MAX < index)) || (NULL == filename)) {
       return AMH_STAT_FAIL;
  }
  if(0 != stat(filename, &buffer)){ /* Has MIB created a new row in rsuMsgRepeatTable? If so there will be a file here. */

      #if defined (EXTRA_EXTRA_DEBUG)
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"check_file_existence(%d): stat failed[%s].\n",index, filename);
      #endif
      memset(&msgRepeatTable[index-1],     0x0, sizeof(RsuMsgRepeatStatusEntry_t));
      memset(&msgRepeat_statTable[index-1],0x0, sizeof(MsgRepeat_Stat_t));
      msgRepeatTable[index-1].rsuMsgRepeatStatus = SNMP_ROW_NONEXISTENT;
      msgRepeatTable[index-1].status             = SNMP_ROW_NONEXISTENT;
      return AMH_STAT_FAIL;
  }
  if ((0 != filesize) && (filesize != buffer.st_size)) { /* Size mismatch. Internal Error if this occurs. */
      #if defined (EXTRA_DEBUG)
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"check_file_existence(%d): stat size failed[%s]: size %d != to %d.\n",index, filename, filesize, buffer.st_size);
      #endif
      return AMH_STAT_SIZE_FAIL;
  }

  if((0 == buffer.st_ctime) && (buffer.st_mtime) && (buffer.st_atime)) {
      msgRepeat_statTable[index-1].lastSend         = 0;
      msgRepeat_statTable[index-1].MIBupdatePending = 0;
      msgRepeatTable[index-1].rsuMsgRepeatStatus    = SNMP_ROW_NONEXISTENT;
      msgRepeatTable[index-1].status                = SNMP_ROW_NONEXISTENT;
      return AMH_IGNORE;
  }

  /* Check stat. If different then reload or load for first time. */
  if (    (msgRepeat_statTable[index-1].last_st_atime == buffer.st_atime)
       && (msgRepeat_statTable[index-1].last_st_mtime == buffer.st_mtime)
       && (msgRepeat_statTable[index-1].last_st_ctime == buffer.st_ctime)){
      #if defined (EXTRA_EXTRA_DEBUG)
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"check_file_existence(i=%d): file exits IGNORE no update.[%s].\n",index, filename);
      #endif
      msgRepeat_statTable[index-1].MIBupdatePending = 0;
      return AMH_IGNORE;
  } else {
      msgRepeat_statTable[index-1].last_st_atime    = buffer.st_atime;
      msgRepeat_statTable[index-1].last_st_mtime    = buffer.st_mtime;
      msgRepeat_statTable[index-1].last_st_ctime    = buffer.st_ctime;
      msgRepeat_statTable[index-1].lastSend         = 0;
      msgRepeat_statTable[index-1].MIBupdatePending = 1; /* Update to MIB pending. */
      #if defined (EXTRA_DEBUG)
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"check_file_existence(i=%d): file exits RELOAD update pending.[%s].\n",index, filename);
      #endif
      return AMH_RELOAD;   
  }
}

STATIC int32_t check_file_existence_legacy(const char_t *filename, int32_t index)
{
  struct stat buffer;

  if( ((index < 0) || ((RSU_SAR_MESSAGE_MAX - 1) < index)) || (NULL == filename)) {
      return AMH_STAT_FAIL;
  }
  if(2 > strlen(filename)) {
      #if defined (EXTRA_EXTRA_DEBUG_LEGACY)
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"check_file_existence_legacy(%d): stat failed 0 [%s].\n",index, filename);
      #endif
      memset(&amhstoredb[index],           0x0, sizeof(amhStoredRecT));
      memset(&amhstoredb_statTable[index], 0x0, sizeof(MsgRepeat_Stat_t));
      return AMH_IGNORE;
  }
  /* If the file no longer exists then user has deleted it. */
  if(0 != stat(filename, &buffer)){ /* Has MIB created a new row in rsuMsgRepeatTable? If so there will be a file here. */
      #if defined (EXTRA_EXTRA_DEBUG_LEGACY)
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"check_file_existence_legacy(%d): stat failed 1 [%s].\n",index, filename);
      #endif
      memset(&amhstoredb[index],           0x0, sizeof(amhStoredRecT));
      memset(&amhstoredb_statTable[index], 0x0, sizeof(MsgRepeat_Stat_t));
      return AMH_IGNORE;
  }
  /* Is this a different way to say it doesn't exist? */
  if(((0 == buffer.st_ctime) && (buffer.st_mtime) && (buffer.st_atime)) || (0 == buffer.st_size)){
      #if defined (EXTRA_EXTRA_DEBUG_LEGACY)
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"check_file_existence_legacy(%d): stat failed 2 [%s].\n",index, filename);
      #endif
      memset(&amhstoredb[index],           0x0, sizeof(amhStoredRecT));
      memset(&amhstoredb_statTable[index], 0x0, sizeof(MsgRepeat_Stat_t));
      return AMH_IGNORE;
  }
  /* Check stat. If different then reload or load for first time. */
  if (    (amhstoredb_statTable[index].last_st_atime == buffer.st_atime)
       && (amhstoredb_statTable[index].last_st_mtime == buffer.st_mtime)
       && (amhstoredb_statTable[index].last_st_ctime == buffer.st_ctime)){
      #if defined (EXTRA_EXTRA_DEBUG_LEGACY)
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"check_file_existence_legacy(i=%d): file exits IGNORE no update.[%s].\n",index, filename);
      #endif
      amhstoredb_statTable[index].MIBupdatePending = 0;
      return AMH_IGNORE;
  } else {
      #if defined (EXTRA_DEBUG_LEGACY)
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"check_file_existence_legacy(i=%d): file exits RELOAD update pending.[%s].\n",index, filename);
      #endif
      amhstoredb_statTable[index].last_st_atime    = buffer.st_atime;
      amhstoredb_statTable[index].last_st_mtime    = buffer.st_mtime;
      amhstoredb_statTable[index].last_st_ctime    = buffer.st_ctime;
      amhstoredb_statTable[index].lastSend         = 0;
      amhstoredb_statTable[index].MIBupdatePending = 1; /* Update to ramlist pending. */
      return AMH_RELOAD;   
  }
}
STATIC uint16_t crc16(const uint8_t * data_p, size_t length)
{
  uint8_t x;
  uint16_t crc = 0xFFFF;

  while (length--){
      x = crc >> 8 ^ *data_p++;
      x ^= x>>4;
      crc = (crc << 8) ^ ((uint16_t)(x << 12)) ^ ((uint16_t)(x <<5)) ^ ((uint16_t)x);
  }
  return crc;
}

/* Do we need semaphore around fopen or does OS protect us from MIB smashing our file mid read? */
/*
 * Supported versions:
 *       i) #define MSG_REPEAT_VERSION 1
 *
 * Next message revision need: 
 *       i) Match size to known version then check version to verify. 
 *       ii) If known message version we should be able to upgrade on the fly.
 */
STATIC int32_t restore_from_disk(char_t * fname, RsuMsgRepeatStatusEntry_t * msgRepeat, int32_t index)
{
  FILE    * file_in        = NULL;
  uint32_t  mib_byte_count = sizeof(RsuMsgRepeatStatusEntry_t);
  int32_t   ret            = AMH_AOK; /* SUCCESS till proven false. */
  char_t buffer[RSU_ALERT_MSG_LENGTH_MAX];
   
  if((NULL == fname) || (NULL == msgRepeat)){
      ret = AMH_STAT_NULL_INPUT;
  } else {
      if(AMH_RELOAD == (ret = check_file_existence(fname, mib_byte_count,index))){ /* Exist, right size, and stat shows mod then load-reload */
          if (NULL != (file_in = fopen(fname, "rb"))){                             /* Open file for input.*/
              if(1 != fread((uint8_t *)msgRepeat, mib_byte_count, 1, file_in)){    /* Load to tmp space before overwriting. */  
#if defined(EXTRA_DEBUG)
                  I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"restore_from_disk: FAIL: read_count is 0: mib_byte_count=(0x%X) errno=(%d:%s).\n",mib_byte_count,errno,strerror(errno));
#endif
                  ret = AMH_STAT_FREAD_FAIL;
              }
              fclose(file_in);
          } else {
#if defined(EXTRA_DEBUG)
              I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"restore_from_disk: fopen fail.\n");
#endif
              ret = AMH_STAT_FOPEN_FAIL;
          }
      }
  }
  if((AMH_AOK == ret) && (MSG_REPEAT_VERSION != msgRepeat->version)){ /* Check version. Only one version supported today. */
      ret = AMH_PROCESS_MSG_WRONG_VERSION;
      if(0x0 == (is_amh_error_set(AMH_PROCESS_MSG_WRONG_VERSION))) {
          set_amh_error_state(AMH_PROCESS_MSG_WRONG_VERSION);
          memset(buffer,0x0,sizeof(buffer));
          snprintf(buffer,sizeof(buffer), "AMH_PROCESS_MSG_WRONG_VERSION: Supported version(%d) != given(%d)",MSG_REPEAT_VERSION, msgRepeat->version);
          sendnotification(buffer, rsuAlertLevel_error);
      }
  }
  if((AMH_AOK == ret) && ((uint16_t)msgRepeat->crc != crc16((const uint8_t *)msgRepeat, mib_byte_count-4))){ /* CRC check. */
      ret = AMH_STAT_CRC_FAIL;
      if(0x0 == (is_amh_error_set(AMH_STAT_CRC_FAIL))) {
          set_amh_error_state(AMH_STAT_CRC_FAIL);
          memset(buffer,0x0,sizeof(buffer));
          snprintf(buffer,sizeof(buffer), "AMH_STAT_CRC_FAIL: CRC error on recover msgRepeat from disk.");
          sendnotification(buffer, rsuAlertLevel_error);
      }
  }
  if(ret < AMH_AOK){
      set_amh_error_state(ret);
  }
  return ret;
}

STATIC int32_t restore_from_disk_legacy(char_t * fname, int32_t index)
{
  int32_t ret = AMH_AOK;
  if(NULL == fname){
      ret = AMH_STAT_NULL_INPUT;
      set_amh_error_state(ret);
  } else {
      if(AMH_RELOAD == (ret = check_file_existence_legacy(fname,index))){ 
          /* Exist and stat shows mod then load-reload */
      }
  }
  return ret;
}
STATIC int32_t check_stopStart_UTC_dsec(uint8_t * StartTime, uint8_t * StopTime, uint64_t UTC_msec, bool_t debugOn, int32_t index)
{
  uint64_t  StartTimeDsec = 0;
  uint64_t  StopTimeDsec  = 0;
  uint64_t  UTC_dsec      = (uint64_t)(UTC_msec/(uint64_t)100);
  /* Wellness checks. */
  if((NULL == StartTime) || (NULL == StopTime)){
      if(debugOn) I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"check_stopStart_UTC_dsec: index=%d: bad input.\n", index);
      return AMH_STAT_BAD_DATA; /* Should never happen. */
  }
  if(AMH_AOK != DateAndTime_To_UTC_DSEC(StartTime, &StartTimeDsec, debugOn)){
      if(debugOn) I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"check_stopStart_UTC_dsecs: index=%d: bad start time.\n", index);
      return AMH_STAT_BAD_DATA;
  }
  if(AMH_AOK != DateAndTime_To_UTC_DSEC(StopTime, &StopTimeDsec, debugOn)){
      if(debugOn) I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"check_stopStart_UTC_dsec: index=%d: bad stop time.\n", index);
      return AMH_STAT_BAD_DATA;
  }
  if(StopTimeDsec <= StartTimeDsec){
      if(debugOn) I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"check_stopStart_dates: index=%d: stop < start time.\n", index);
      return SNMP_ROW_NOTREADY;
  }
  if(StopTimeDsec < UTC_dsec){
      if(debugOn) I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"check_stopStart_UTC_dsec: index=%d: stop(0x%lx) < now(0x%lx): Expired & ready for destroy.\n", index, StopTimeDsec, UTC_dsec);
      /* We never destroy rows. Snmp client manages rows any way they want. */
      return SNMP_ROW_NOTREADY; /* Expired. Set to NOTREADY. */
  }
  if(StartTimeDsec <= UTC_dsec) { /* This message is active assuming it is well formed. */
      return SNMP_ROW_ACTIVE;
  }
  if(UTC_dsec < StartTimeDsec) {
      /* These files stick around so lots of debug if enabled. */
      if(debugOn) I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"check_stopStart_UTC_dsec: index=%d: now < start: Not in service yet.\n",index);
      return SNMP_ROW_NOTINSERVICE; /* Time to start not here but valid nonetheless. */
  }
  if(debugOn) I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"check_stopStart_UTC_dsec: index=%d: internal error.\n",index);
  return AMH_STAT_BAD_DATA; /* Should not be possible to get here. */
}
/* RSU 4.1 Legacy: start and stop are UTC seconds. */
STATIC int32_t check_stopStart_UTC_sec(uint32_t startMin, uint32_t endMin, uint64_t UTC_msec)
{
  uint32_t  UTC_sec = (uint32_t)(UTC_msec/(uint64_t)1000);

  if(endMin <= startMin){
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"check_stopStart_UTC_sec: stop(%u) < start(%u): Illegal: DESTROY.\n",endMin, startMin);
      return SNMP_ROW_DESTROY;
  }
  if(endMin < UTC_sec){
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"check_stopStart_UTC_sec: stop(%u) < now(%u): Expired: DESTROY.\n", endMin , UTC_sec);
      return SNMP_ROW_DESTROY; /* This message has expired based on provided times. DESTROY. */     
  }
  if(startMin <= UTC_sec) { /* This message is active assuming it is well formed. */
      return SNMP_ROW_ACTIVE;
  }
  if(UTC_sec < startMin) {
      #if defined(EXTRA_DEBUG) /* These files stick around so lots of debug if enabled. */
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"check_stopStart_UTC_sec: now(%u) < start(%u): Not in service: KEEP.\n", UTC_sec, startMin);
      #endif
      return SNMP_ROW_NOTINSERVICE; /* Time to start not here but valid nonetheless. */
  }
  I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"check_stopStart_UTC_sec: internal error.\n");
  return AMH_STAT_BAD_DATA; /* Should not be possible to get here. */
}

/* 
 * Well formed means message is valid completely and ready to start sending.
 * If we find anything we dont like then we make row status SNMP_ROW_NOTREADY.
 */
STATIC int32_t msgRepeat_wellformed(RsuMsgRepeatStatusEntry_t * tmp_msgRepeat, char_t * fname, int32_t index) 
{
  int32_t  ret  = AMH_AOK;
  uint32_t psid = 0x0;
  if((NULL == tmp_msgRepeat) || (NULL == fname)){
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"msgRepeat_wellformed: index=%d: bad input.\n", index);
      ret = AMH_STAT_BAD_FORM;
  } else {
      tmp_msgRepeat->status = SNMP_ROW_NOTREADY; /* Assume notReady till proven otherwise. */
      if((tmp_msgRepeat->rsuMsgRepeatIndex < RSU_SAR_MESSAGE_MIN) || (RSU_SAR_MESSAGE_MAX < tmp_msgRepeat->rsuMsgRepeatIndex)){
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"msgRepeat_wellformed: index=%d: bad rsuMsgRepeatIndex(%d) beyond min and max.\n",index, tmp_msgRepeat->rsuMsgRepeatIndex);
          ret = AMH_STAT_BAD_FORM;
      }
      if(tmp_msgRepeat->rsuMsgRepeatIndex != index){ /* This is not accessible by the client so internal error.*/
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"msgRepeat_wellformed: index=%d: bad rsuMsgRepeatIndex(%d) != %d\n",index, tmp_msgRepeat->rsuMsgRepeatIndex,index);
          tmp_msgRepeat->rsuMsgRepeatIndex = index; /* Correct in RAM list and send to MIB. */
          ret = AMH_STAT_BAD_MODE;
      }
      if((tmp_msgRepeat->rsuMsgRepeatPsid_length < RSU_RADIO_PSID_MIN_SIZE) || (RSU_RADIO_PSID_SIZE < tmp_msgRepeat->rsuMsgRepeatPsid_length)){
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"msgRepeat_wellformed: index=%d: bad rsuMsgRepeatPsid_length = %d.\n",index, tmp_msgRepeat->rsuMsgRepeatPsid_length);
          tmp_msgRepeat->rsuMsgRepeatPsid_length = RSU_RADIO_PSID_MIN_SIZE; /* Not acessible by client so internal error. */
          ret = AMH_STAT_BAD_MODE;
      } else {
          psid  = ((uint32_t)(tmp_msgRepeat->rsuMsgRepeatPsid[0])) << 8;
          psid |= (uint32_t)tmp_msgRepeat->rsuMsgRepeatPsid[1];
          psid  = psid << 8;
          psid |= (uint32_t)tmp_msgRepeat->rsuMsgRepeatPsid[2];
          psid  = psid << 8;
          psid |= (uint32_t)tmp_msgRepeat->rsuMsgRepeatPsid[3];
          /*From ris.c: Validate PSID. */
          if(psid <= (uint32_t) PSID_1BYTE_MAX_VALUE){ 
          } else if(psid >= (uint32_t)PSID_2BYTE_MIN_VALUE && psid <= (uint32_t)PSID_2BYTE_MAX_VALUE){
          } else if(psid >= (uint32_t)PSID_3BYTE_MIN_VALUE && psid <= (uint32_t)PSID_3BYTE_MAX_VALUE){
          } else if(psid >= (uint32_t)PSID_4BYTE_MIN_VALUE && psid <= (uint32_t)PSID_4BYTE_MAX_VALUE){
          } else {
              I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"msgRepeat_wellformed: index=%d: bad PSID=0x%X.\n", index, psid);
              ret = AMH_STAT_BAD_FORM;
          }
      }
      if((tmp_msgRepeat->rsuMsgRepeatTxChannel < RSU_RADIO_CHANNEL_MIN) || (RSU_RADIO_CHANNEL_MAX < tmp_msgRepeat->rsuMsgRepeatTxChannel)){
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"msgRepeat_wellformed: index=%d: bad rsuMsgRepeatTxChannel) = %d.\n",index, tmp_msgRepeat->rsuMsgRepeatTxChannel);
          tmp_msgRepeat->rsuMsgRepeatTxChannel = RSU_RADIO_CHANNEL_MIN;
          ret = AMH_STAT_BAD_FORM;
      }
      if(0 == cfg.RadioType){
          if(tmp_msgRepeat->rsuMsgRepeatTxChannel != cfg.uchannel){
              I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"msgRepeat_wellformed: index=%d: bad DSRC rsuMsgRepeatTxChannel = %d != i2v.conf = %d.\n", index, tmp_msgRepeat->rsuMsgRepeatTxChannel, cfg.uchannel);
              ret = AMH_STAT_BAD_FORM;
          }
      } else {
          if(tmp_msgRepeat->rsuMsgRepeatTxChannel != RSU_CV2X_CHANNEL_DEFAULT){
              I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"msgRepeat_wellformed: index=%d: bad CV2X rsuMsgRepeatTxChannell = %d != RSU_CV2X_CHANNEL_DEFAULT = %d.\n", index, tmp_msgRepeat->rsuMsgRepeatTxChannel, RSU_CV2X_CHANNEL_DEFAULT);
              ret = AMH_STAT_BAD_FORM;
          }
      }
      if((tmp_msgRepeat->rsuMsgRepeatTxInterval < RSU_MSG_REPEAT_TX_INTERVAL_MIN) || (RSU_MSG_REPEAT_TX_INTERVAL_MAX < tmp_msgRepeat->rsuMsgRepeatTxInterval)){
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"msgRepeat_wellformed: index=%d: bad rsuMsgRepeatTxInterval = %d.\n", index, tmp_msgRepeat->rsuMsgRepeatTxInterval);
          ret = AMH_STAT_BAD_FORM;
      }
      /* rsuMsgRepeatPayload: How to check 2.5k binary data? Assuming dsrcMsgId and a few bytes at least. */
      if((tmp_msgRepeat->rsuMsgRepeatPayload_length < RSU_MSG_PAYLOAD_MIN) || (RSU_MSG_PAYLOAD_MAX < tmp_msgRepeat->rsuMsgRepeatPayload_length)){
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"msgRepeat_wellformed: index=%d: bad rsuMsgRepeatPayload_length = %d.\n", index, tmp_msgRepeat->rsuMsgRepeatPayload_length);
          ret = AMH_STAT_BAD_FORM;
      } 
#if defined(ENABLE_DSRC_MSG_ID_CHECK) /* NTCIP-1218 does not mention using payload like this. Maybe feature for the future. */
      /* DENSO imposed constraint. No SPAT allowed. Also must be a valid DSRC MSG ID. */
      if(  (ASN1V_signalPhaseAndTimingMessage == tmp_msgRepeat->rsuMsgRepeatPayload[1]) 
        || (0 == dsrcMsgId_to_amhMsgType(tmp_msgRepeat->rsuMsgRepeatPayload[1]))){
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"msgRepeat_wellformed: index=%d: bad Uper DSRC MSG ID[0x%X]:\n", index, tmp_msgRepeat->rsuMsgRepeatPayload[1]);
          ret = AMH_STAT_BAD_FORM;
      }
#endif
      if((tmp_msgRepeat->rsuMsgRepeatEnable < 0) || (1 < tmp_msgRepeat->rsuMsgRepeatEnable)){
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"msgRepeat_wellformed: index=%d: bad rsuMsgRepeatEnable = %d.\n", index, tmp_msgRepeat->rsuMsgRepeatEnable);
          ret = AMH_STAT_BAD_FORM;
      }
      if((tmp_msgRepeat->rsuMsgRepeatStatus < SNMP_ROW_NONEXISTENT) || (SNMP_ROW_DESTROY < tmp_msgRepeat->rsuMsgRepeatStatus)){
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"msgRepeat_wellformed: index=%d: bad rsuMsgRepeatStatus = %d.\n", index, tmp_msgRepeat->rsuMsgRepeatStatus);
          ret = AMH_STAT_BAD_FORM;
      } 
      if((tmp_msgRepeat->rsuMsgRepeatPriority < RSU_MSG_REPEAT_PRIORITY_MIN) || (RSU_MSG_REPEAT_PRIORITY_MAX < tmp_msgRepeat->rsuMsgRepeatPriority)){
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"msgRepeat_wellformed: index=%d: bad rsuMsgRepeatPriority = %d.\n", index, tmp_msgRepeat->rsuMsgRepeatPriority);
          ret = AMH_STAT_BAD_FORM;
      }
      if(~RSU_MSG_REPEAT_OPTIONS_MASK & tmp_msgRepeat->rsuMsgRepeatOptions){ /* Erroneous bit check. */
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"msgRepeat_wellformed: index=%d: bad rsuMsgRepeatOptions = 0x%X.\n", index, tmp_msgRepeat->rsuMsgRepeatOptions);
          ret = AMH_STAT_BAD_FORM;
      }
      /* Generated by the MIB upon creation of file on disk. Legacy support RSU 4.1 spec. */
      if((strlen(tmp_msgRepeat->filePathName) < RSU_MSG_FILENAME_LENGTH_MIN) || ((RSU_MSG_FILENAME_LENGTH_MAX-1) < strlen(tmp_msgRepeat->filePathName))){
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"msgRepeat_wellformed: index=%d: bad filePathName: wrong size.\n", index);
          ret = AMH_STAT_BAD_FORM;
      }
      if((strlen(fname) < RSU_MSG_FILENAME_LENGTH_MIN) || ((RSU_MSG_FILENAME_LENGTH_MAX-1) < strlen(fname))){
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"msgRepeat_wellformed: index=%d: bad fname input len=%d.\n",index,strlen(fname));
          ret = AMH_STAT_BAD_FORM;
      }
      /* fname and value inside file must match. */
      if(0 != strncmp(fname, tmp_msgRepeat->filePathName,strlen(fname))){
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"msgRepeat_wellformed: index=%d: bad fname[%s] != filePathName[tmp_msgRepeat->filePathName]\n",index,fname);
          ret = AMH_STAT_BAD_FORM;
      }

      /* The only columns AMH writes to are status and enable. If there's bogus data above not our problem. Just dont send message out. */
      if(AMH_AOK == ret) { /* Don't bother checking times if the above fails. */
          /* If we make it this far then we have a well formed message. Now check start stop times. */
          ret = check_stopStart_UTC_dsec(tmp_msgRepeat->rsuMsgRepeatDeliveryStart, tmp_msgRepeat->rsuMsgRepeatDeliveryStop, i2vUtilGetTimeInMs(), WTRUE, index);

//TODO: These can become annoying outputs. Think about throttling them. Important for user to see but not at light speed.

          switch(ret){
              case SNMP_ROW_ACTIVE: /* This message is active assuming it is well formed. */
                  tmp_msgRepeat->status = SNMP_ROW_ACTIVE;
                  ret = AMH_AOK;
                  break;
              case SNMP_ROW_NOTREADY: /* Stop is <= Start */
                  tmp_msgRepeat->status = SNMP_ROW_NOTREADY;
                  I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"msgRepeat_wellformed: index=%d: stop < start.\n", index);    
                  ret = AMH_STAT_BAD_FORM;
                  break;
              case SNMP_ROW_DESTROY: /* This message has expired based on provided times. If it was active destroy else not ready */
                  tmp_msgRepeat->status = SNMP_ROW_NOTREADY;
                  I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"msgRepeat_wellformed: index=%d: stop < now.\n", index);
                  ret = AMH_STAT_BAD_FORM;
                  break;    
              case SNMP_ROW_NOTINSERVICE: /* Time to start not here but valid nonetheless. Valid transaction */
                  tmp_msgRepeat->status = SNMP_ROW_NOTINSERVICE;
                  I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"msgRepeat_wellformed: index=%d: start < now.\n", index);              
                  ret = AMH_AOK;
                  break;
              case AMH_STAT_BAD_DATA:
              default: 
                  tmp_msgRepeat->status = SNMP_ROW_NOTREADY;
                  ret = AMH_STAT_BAD_FORM;
                  break;
          }
          /*  Future think: More courtesy checks. These cause Tx to fail intermittently.
           *    i) No Nav in GNSS.
           *    ii) Invalid security certs.
           *  For now we have is AMH_SAR_WSM_REQ_ERROR. Too generic and not 100% helpful. User still needs to dig to understand failure.
           */
          if(AMH_AOK == ret) {
              #if defined(EXTRA_DEBUG)
              I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"msgRepeat_wellformed: index=%d: AOK.\n", index);
              #endif
          } else { 
              I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"msgRepeat_wellformed: index=%d: bad start stop times.\n", index);
          }
      } else {
          tmp_msgRepeat->status = SNMP_ROW_NOTREADY;
#if defined(EXTRA_DEBUG)
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"msgRepeat_wellformed: index=%d: Failed.\n", index);
#endif
      }
      if(tmp_msgRepeat->status != SNMP_ROW_ACTIVE) {
          print_msgRepeat(tmp_msgRepeat); /* Only report what is not active. */
      }
  }
  return ret;
}

/* 
 * Will enumerate S&R files on micro adding or deleting from RAM list.
 */
#define AMH_JUNK_FILE_GRACE  ((RSU_SAR_MESSAGE_MAX * 2) + 10)  /* Grace for bogus files and 1218 .dats . Otherwise denial by junk attack :-S */

STATIC bool_t amh_load_files(void)
{  
  int32_t    ret = AMH_AOK;
  uint32_t   i=0,j=0,k=0;
  uint32_t   idx = 0; 
  FILE     * f = NULL;
  char_t   * file_ptr = NULL;
  char_t cmd[RSU_MSG_FILENAME_LENGTH_MAX + 16]; /* Add 16 so we can put command + fname ie 'rm -f fname' */
  char_t fname[RSU_MSG_FILENAME_LENGTH_MAX];
  RsuMsgRepeatStatusEntry_t tmp_msgRepeat;
  amhStoredRecT  tmp_amhstore;
  char_t buffer[RSU_ALERT_MSG_LENGTH_MAX];

    /* 
     * NTCIP-1218 disk enumeration to determine current number of messages ready or not.
     * By definition file names are AMH_ACTIVE_DIR/1.dat...AMH_ACTIVE_DIR/maxRsuMsgRepeat.dat. Do file stat and update accordingly. 
     */
    for(i=0,msgRepeat_count=0; i < RSU_SAR_MESSAGE_MAX;i++){
        memset(&tmp_msgRepeat,0x0,sizeof(tmp_msgRepeat));
        memset(cmd,0x0,sizeof(cmd));
        snprintf(cmd,sizeof(cmd),"%s/%d.dat",AMH_ACTIVE_DIR,i+1); /* Name and location of table data known. */
        ret = restore_from_disk(cmd, &tmp_msgRepeat,i+1);
        switch(ret){
            case AMH_RELOAD: /* If the file exists, correct size, and stat shows modified then load-reload. */
                if(AMH_AOK != msgRepeat_wellformed(&tmp_msgRepeat,cmd,i+1)){ /* If not wellformed it will set row status to not ready. */
                    if(0x0 == (is_amh_error_set(AMH_STAT_BAD_FORM))) {
                        set_amh_error_state(AMH_STAT_BAD_FORM); /* Signal Client an issue with file by changing row status reported back. */
                        memset(buffer,0x0,sizeof(buffer));
                        snprintf(buffer,sizeof(buffer), "AMH_STAT_BAD_FORM: msgRepeat index(%d) is not well formed: Not Sent.",i+1);
                        sendnotification(buffer, rsuAlertLevel_error);
                    }
                }
                memcpy(&msgRepeatTable[tmp_msgRepeat.rsuMsgRepeatIndex-1], &tmp_msgRepeat, sizeof(RsuMsgRepeatStatusEntry_t)); 
                msgRepeat_count++;
                ret = AMH_AOK;
                break;
            case AMH_IGNORE: /* Message already loaded and no pending update. */
                msgRepeat_count++; 
                ret = AMH_AOK;
                break;
            default:  /* Let error in ret go back to calling function. */
                break;
        } /* If ret != AMH_AOK we may want to stop trying. Today we keep going. */  
    }

    /* 
     * RSU 4.1 Legacy support below. AMH dir is an open folder and anything can show up at any time. File stat is harder.
     */

    /* First look at 4.1 RAM list and stat files we already know. Purge deleted files. */
    for(i=0; i < RSU_SAR_MESSAGE_MAX; i++){
        memset(cmd,0x0,sizeof(cmd));
        snprintf(cmd,sizeof(cmd),"%s",amhstoredb[i].myName); /* Name and location of file known. Legacy file format */
        ret = restore_from_disk_legacy(cmd,i);
    }    

    /* Now look to see if any new files added to dir. */
    if (I2V_RETURN_OK != i2vDirectoryListing(AMH_ACTIVE_DIR,PEER_LIST, NULL)) {
        if(0 == (amh_load_files_err_count % 20)){
            I2V_DBG_LOG(LEVEL_CRIT,MY_NAME,"AMH_ACTIVE_MSG_DIR_READ_ERROR(%u)\n",amh_load_files_err_count);
        }
        amh_load_files_err_count++;
        set_amh_error_state(AMH_ACTIVE_MSG_DIR_READ_ERROR);
        return WFALSE;
    }
    if (NULL == (f=fopen(PEER_LIST, "r"))) {
        set_amh_error_state(AMH_PEER_LIST_FOPEN_ERROR);
        return WFALSE;
    }

    /* Add '/' just in case. Should now be NULL terminated after snprintf() too. */ 
    snprintf(fname,sizeof(fname), "%s/", AMH_ACTIVE_DIR); 
    idx = strlen(fname);
   
    /* 
     * Parse each file and store content in db: 
     * i2vDirectoryListing file has '\n' character at end.
     * Therefore, last line is always throw away junk.
     * Since we create it, if it isn't perfect then there's a problem somewhere.
     * Too many files is one thing: imf.c should limit new SAR when full from socket?
     * Need 'i <' term incase we find nothing or all junk. 
     * I wont try till EOF cause who knows how large? 
     * If user puts junk in folder then thats a problem hard to deal with.
     */

    /* First load files into temp struct.*/
    for (i=0,j=0;   (idx < RSU_MSG_FILENAME_LENGTH_MAX) && (NULL != (file_ptr=fgets(&fname[idx], RSU_MSG_FILENAME_LENGTH_MAX,f)))
                 && (j < RSU_SAR_MESSAGE_MAX) && (i < AMH_JUNK_FILE_GRACE); 
                    i++, memset(&fname[idx], 0, RSU_MSG_FILENAME_LENGTH_MAX - idx)){

        fname[strlen(fname)-1]  = 0; /* remove '\n' character or fname fails stat check.*/
        if(NULL != strstr(fname,".dat")){ /* NTCIP-1218: Ignore */
        } else {
            /* RSU 4.1 legacy error checking. Future: Could use 1218 method but need to upconvert messages. */
            if (amhLoadRecData(fname, &fname[idx], &tmp_amhstore)) {          
                memcpy(tmp_amhstore.myName,fname, RSU_MSG_FILENAME_LENGTH_MAX); 
                tmp_amhstore.myName[RSU_MSG_FILENAME_LENGTH_MAX - 1] = '\0';
                j++; /* Go to next message. */
                for(k=0;k<RSU_SAR_MESSAGE_MAX;k++){/* Found existing message. Checking pending.*/
                    if(0 == strncmp(amhstoredb[k].myName,tmp_amhstore.myName,RSU_MSG_FILENAME_LENGTH_MAX)){
                        if(amhstoredb_statTable[k].MIBupdatePending == 1){  
                            memcpy(&amhstoredb[k], &tmp_amhstore, sizeof(amhStoredRecT));
                            /* Convert and store ONCE. */
                            if(0 == amhCfg.amhIntervalSelect) {
                                amhstoredb[k].interval = amhstoredb[k].interval * 100;
                            } else {
                                amhstoredb[k].interval = amhstoredb[k].interval * 1000;
                            }
                            amhstoredb_statTable[k].MIBupdatePending = 0; /* Update complete. */
                        }
                        break;
                    } 
                }
                if(RSU_SAR_MESSAGE_MAX == k) { /* Did not find existing message. Find free spot and add. */
                    for(k=0; k<RSU_SAR_MESSAGE_MAX; k++){ 
                        if((0 == amhstoredb[k].priority) && (0 == amhstoredb[k].channel)){
                            memcpy(&amhstoredb[k], &tmp_amhstore, sizeof(amhStoredRecT));
                            memset(&amhstoredb_statTable[k], 0x0, sizeof(MsgRepeat_Stat_t));
                            /* Convert and store ONCE. */
                            if(0 == amhCfg.amhIntervalSelect) {
                                amhstoredb[k].interval = amhstoredb[k].interval * 100;
                            } else {
                                amhstoredb[k].interval = amhstoredb[k].interval * 1000;
                            }
                            break;
                        }
                    }
                    if(RSU_SAR_MESSAGE_MAX == k){
                        if(0x0 == (is_amh_error_set(AMH_SAR_TOO_MANY_WARNING))) {
                            set_amh_error_state(AMH_SAR_TOO_MANY_WARNING);
                            memset(buffer,0x0,sizeof(buffer));
                            snprintf(buffer,sizeof(buffer), "AMH_SAR_TOO_MANY_WARNING: Too many 4.1 SAR files: max is (%d).",RSU_SAR_MESSAGE_MAX);
                            sendnotification(buffer, rsuAlertLevel_warning);
                        }
                    }
                }
            } else {
//TODO: The criteria is too weak and we may delete a file as written to.
#if 0
                snprintf(cmd,sizeof(cmd),"rm -f %s",fname);/* Ruthlessly remove junk files. */
                if(system(cmd)){}; /* Silence unused warning compile warning. If this call fails its a dont care. */
#endif
                if(0x0 == (is_amh_error_set(AMH_SAR_JUNK_FILES_WARNING))) {
                    set_amh_error_state(AMH_SAR_JUNK_FILES_WARNING);
                    memset(buffer,0x0,sizeof(buffer));
                    snprintf(buffer,sizeof(buffer), "AMH_SAR_JUNK_FILES_WARNING: Please review AMH dir for junk or incorrect SAR files. Ignoring some files.");
                    sendnotification(buffer, rsuAlertLevel_warning);
                }
            }
        }
    } /* for */
    fclose(f);
    /* Calculate final total of RSU 4.1 messages. */
    for(i = 0, amhstoredbcnt = 0; i < RSU_SAR_MESSAGE_MAX; i++){
        if((0 < amhstoredb[i].priority) && (0 != amhstoredb[i].channel)) {
            amhstoredbcnt++; /* Must have non zero priority and channel else assume empty. */
        }
    }
    /* 
     * We gave up parsing list because its too big. We allow a certain amount but at some point give up.
     * Question: Either we delete contents or we let human do it? For now we leave junk where it is.
     */
    if(AMH_JUNK_FILE_GRACE == i){
        if(0x0 == (is_amh_error_set(AMH_SAR_DIR_LIST_TOO_LARGE_WARNING))) {
            set_amh_error_state(AMH_SAR_DIR_LIST_TOO_LARGE_WARNING);
            memset(buffer,0x0,sizeof(buffer));
            snprintf(buffer,sizeof(buffer), "AMH_SAR_DIR_LIST_TOO_LARGE_WARNING: There's too many files in AMH dir. Please limit to max(%d)",RSU_SAR_MESSAGE_MAX);
            sendnotification(buffer, rsuAlertLevel_warning);
        }
    } 
    /* NTCIP-1218 is a conceptual table and we know the name of each row. Keeping stat therefore, has meaning: 1.dat will always be first row etc. */
    if(0 == msgRepeat_count){
        memset(msgRepeatTable,0x0,sizeof(msgRepeatTable));
    }
    /* Legacy is open ended. Who knows what will show up second to second so no point preserving anything.*/
    if(0 == amhstoredbcnt){
        memset(amhstoredb,0x0,sizeof(amhstoredb));
        memset(amhstoredb_statTable, 0x0, sizeof(msgRepeat_statTable));
    }
    #if defined(EXTRA_DEBUG)
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"amh_load_files::amhstoredbcnt = %d,%d\n",amhstoredbcnt,msgRepeat_count);
    #endif
    if(amhstoredbcnt || msgRepeat_count)
        return WTRUE;
    else
        return WFALSE;
}
/*----------------------------------------------------------------------------*/
/* Dump to /tmp for user to cat. Only actionable items for user.              */
/*----------------------------------------------------------------------------*/
STATIC void dump_amh_report(void)
{
  FILE * file_out = NULL;
  if ((file_out = fopen("/tmp/amh.txt", "w")) == NULL){
      if(0x0 == is_amh_error_set(AMH_CUSTOMER_DIGEST_FAIL)) { /* Report fail one time. */
          set_amh_error_state(AMH_CUSTOMER_DIGEST_FAIL);
          I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"Unable to create user AMH report.\n"); /* Not fatal */
      }
  } else {
      clear_amh_error_state(AMH_CUSTOMER_DIGEST_FAIL);
      if(is_amh_error_set(AMH_FWD_WRITE_ERROR)){
          fprintf(file_out,"AMH_FWD_WRITE_ERROR: Unable to FWDMSG. Is FWDMSG using valid IP+Port or have we lost connection to it?\n");
      }
      if(is_amh_error_set(AMH_FWD_WRITE_IGNORE)){
          fprintf(file_out,"AMH_FWD_WRITE_IGNORE: Friendly remainder, FWDMSG is not enabled. Ignore if that was your intent.\n");
      }
      if(is_amh_error_set(AMH_RAMLIST_PRIORITY_ERROR)){
          fprintf(file_out,"AMH_RAMLIST_PRIORITY_ERROR: RSU 4.1 SAR: file rejected because priority is beyond valid range.\n");
      }
      if(is_amh_error_set(AMH_RADIO_REG_FULL)){
          fprintf(file_out,"AMH_RADIO_REG_FULL: AMH has reached the maximum allowed psids registered with radio.\n");
      }
      if(is_amh_error_set(AMH_RAMLIST_LOAD_NOT_ASCII_DATA)){
          fprintf(file_out,"AMH_RAMLIST_LOAD_NOT_ASCII_DATA: RSU 4.1: SAR files with payload that is not ascii hex data. Please correct or remove.\n");
      }
      if(is_amh_error_set(AMH_RAMLIST_LOAD_ODD_SIZE)){
          fprintf(file_out,"AMH_RAMLIST_LOAD_ODD_SIZE: RSU 4.1 SAR: files with odd length of payload. Please correct or remove.\n");
      }
      if(is_amh_error_set(AMH_RAMLIST_LOAD_TOO_BIG)){
          fprintf(file_out,"AMH_RAMLIST_LOAD_TOO_BIG: RSU 4.1 SAR: files that payload is too large to be legit. Please correct or remove.\n");
      }
      if(is_amh_error_set(AMH_RAMLIST_LOAD_TOO_SMALL)){
          fprintf(file_out,"AMH_RAMLIST_LOAD_TOO_SMALL: RSU 4.1 SAR: files that payload is too small to be legit. Please correct or remove.\n");
      }
      if(is_amh_error_set(AMH_RAMLIST_FILE_TOO_BIG)){
          fprintf(file_out,"AMH_RAMLIST_LOAD_TOO_BIG: RSU 4.1 SAR: files that are too large to be legit. Please correct or remove.\n");
      }
      if(is_amh_error_set(AMH_RAMLIST_FILE_TOO_SMALL)){
          fprintf(file_out,"AMH_RAMLIST_LOAD_TOO_SMALL: RSU 4.1 SAR: files that are too small to be legit. Please correct or remove.\n");
      }
      if(is_amh_error_set(AMH_STAT_BAD_FORM)){
          fprintf(file_out,"AMH_STAT_BAD_FORM: NTCIP-1218 msgRepeat: Some rows of table are being rejected as not well formed. Please review SEL and correct.\n");
      }
//TODO: This is goofy, 4.1 vs 1218
      if(is_amh_error_set(AMH_PROCESS_MSG_WRONG_VERSION)){
          fprintf(file_out,"AMH_PROCESS_MSG_WRONG_VERSION: RSU 4.1 SAR or NTCIP-1218 msgRepeat: Supported version(%d). Message rejected see SEL.\n",MSG_REPEAT_VERSION);
      }
      if(is_amh_error_set(AMH_PROCESS_BAD_MSG_TYPE)){
          fprintf(file_out,"AMH_PROCESS_BAD_MSG_TYPE: RSU 4.1 SAR: Bad Msg Type. Message rejected see SEL.\n");
      }
      if(is_amh_error_set(AMH_SAR_FULL_WARNING)){
          fprintf(file_out,"AMH_SAR_FULL_WARNING : RSU 4.1 SAR AMH: dir is full: max(%d)\n",RSU_SAR_MESSAGE_MAX);
      }
      if(is_amh_error_set(AMH_SAR_REJECTED)){
          fprintf(file_out,"AMH_SAR_REJECTED: RSU 4.1 SAR: Files are being rejected. Please review SEL.\n");
      }
      if(is_amh_error_set(AMH_SAR_BAD_START_TIME)){
          fprintf(file_out,"AMH_SAR_BAD_START_TIME: RSU 4.1 SAR: Files are being rejected because of bad start time. Please review SEL.\n");
      }
      if(is_amh_error_set(AMH_SAR_BAD_STOP_TIME)){
          fprintf(file_out,"AMH_SAR_BAD_STOP_TIME: RSU 4.1 SAR: Files are being rejected because of bad stop time. Please review SEL.\n");
      }
      if(is_amh_error_set(AMH_SAR_STOP_TIME_BEFORE_START)){
          fprintf(file_out,"AMH_SAR_STOP_TIME_BEFORE_START: RSU 4.1 SAR: Files are being rejected because stop time is before start time. Please review SEL.\n");
      }
      if(is_amh_error_set(AMH_SAR_PARSE_CONF_VALIDATE_FAIL)){
          fprintf(file_out,"AMH_SAR_PARSE_CONF_VALIDATE_FAIL: RSU 4.1 SAR: Files are being rejected because missing message item in file. Please review SEL.\n");
      }
      if(is_amh_error_set(AMH_IMF_BIND_FAIL)){
          fprintf(file_out,"AMH_IMF_BIND_FAIL: RSU 4.1 IMF: Unable to bind to given Port. Is there a conflict for this port?\n");
      }
      if(is_amh_error_set(AMH_IMF_PAYLOAD_NOT_HEX_DATA)){
          fprintf(file_out,"AMH_IMF_PAYLOAD_NOT_HEX_DATA: RSU 4.1 IMF: files with payload that is not ascii hex data. Please correct or remove.\n");
      }
      if(is_amh_error_set(AMH_IMF_BAD_TXINTERVAL)){
          fprintf(file_out,"AMH_IMF_BAD_TXINTERVAL: RSU 4.1 IMF: files with non-zero TXINTERVAL detected. Please correct or remove.\n");
      }
      if(is_amh_error_set(AMH_IMF_ENCRYPT_REJECT)){
          fprintf(file_out,"AMH_IMF_ENCRYPT_REJECT: RSU 4.1 IMF: files with unsupported ENCRYPTION enabled. Please correct or remove.\n");
      }
      if(is_amh_error_set(AMH_IMF_SECURITY_REJECT)){
          fprintf(file_out,"AMH_IMF_SECURITY_REJECT: RSU 4.1 IMF: files with unsupported SECURITY enabled. Please correct or remove.\n");
      }
      if(is_amh_error_set(AMH_IMF_MSGTYPE_REJECT)){
          fprintf(file_out,"AMH_IMF_MSGTYPE_REJECT: RSU 4.1 IMF: files with unsupported MSGTYPE detected. Please correct or remove.\n");
      }
//TODO: Interesting, do we check SAR sent via IMF for valid psid?
      if(is_amh_error_set(AMH_IMF_INVALID_PSID)){
          fprintf(file_out,"AMH_IMF_INVALID_PSID: RSU 4.1 IMF: files with unsupported MSGTYPE detected. Please correct or remove.\n");
      }
      /* In theory, this can't happen because MIB has on well formed test prior to sending here. */
      if(is_amh_error_set(AMH_IFM_STAT_BAD_FORM)){
          fprintf(file_out,"AMH_IFM_STAT_BAD_FORM: NTCIP-1218 rsuIFM: files are being rejected for not being well formed. Please correct or remove.\n");
      }
      if(is_amh_error_set(AMH_IMF_CONF_ITEM_MISSING)){
          fprintf(file_out,"AMH_IMF_CONF_ITEM_MISSING: RSU 4.1 IMF: Files are being rejected because missing message item in file. Please review SEL.\n");
      }
      if(is_amh_error_set(AMH_IMF_PAYLOAD_TOO_SMALL)){
          fprintf(file_out,"AMH_IMF_PAYLOAD_TOO_SMALL: RSU 4.1 IMF: Files are being rejected because payload too small. Please review SEL.\n");
      }
      if(is_amh_error_set(AMH_IMF_PAYLOAD_TOO_BIG)){
          fprintf(file_out,"AMH_IMF_PAYLOAD_TOO_BIG: RSU 4.1 IMF: Files are being rejected because payload too big. Please review SEL.\n");
      }
      if(is_amh_error_set(AMH_IMF_PAYLOAD_ODD_LENGTH)){
          fprintf(file_out,"AMH_IMF_PAYLOAD_ODD_LENGTH: RSU 4.1 IMF: Files are being rejected because payload odd length. Must be even. Please review SEL.\n");
      }
      if(is_amh_error_set(AMH_IMF_BAD_OPTIONS)){
          fprintf(file_out,"AMH_IMF_BAD_OPTIONS: NTCIP-1218 rsuIFM: Files are being rejected because invalid options bits set. Please review SEL.\n");
      }
//TODO: Hmmmm, we dont seem to do this for 4.1 IMF/SAR?
      if(is_amh_error_set(AMH_IMF_RESTRICT_IP)){
          fprintf(file_out,"AMH_IMF_RESTRICT_IP: NTCIP-1218 rsuIFM: Files are being rejected because they came from invalid IP.\n");
      }
      fflush(file_out);
      fclose(file_out);
      file_out = NULL;
  }
}
STATIC void dump_amh_status(void)
{
  int32_t i;

  if(0 == (debug_counter % OUTPUT_MODULUS)){ /* Future: add prior and if change push out early.*/
      if(amhManagerStats.sar_tx_cnt < prior_sar_count){ /* reset when counter rolls. */
          prior_sar_count = 0;
      }
      sar_datarate = (float32_t)(amhManagerStats.sar_tx_cnt - prior_sar_count)/(float32_t)OUTPUT_MODULUS;

      if(amhManagerStats.imf_tx_cnt < prior_ifm_count){ /* reset when counter rolls. */
          prior_ifm_count = 0;
      }
      ifm_datarate = (float32_t)(amhManagerStats.imf_tx_cnt - prior_ifm_count)/(float32_t)OUTPUT_MODULUS;

      I2V_DBG_LOG(LEVEL_INFO,MY_NAME,"(0x%x) sar(0x%x: tx=%u s:m=%u:%lu err=%u rate=%.2f) imf(0x%x: tx=%u err=%u rate=%.2f)\n",
                amh_error_states,sar_error_states,amhManagerStats.sar_tx_cnt,amhManagerStats.sar_tx_slip, amhManagerStats.sar_tx_max_slip
              , amhManagerStats.sar_tx_err, sar_datarate, imf_error_states, amhManagerStats.imf_tx_cnt
              , amhManagerStats.imf_tx_err, ifm_datarate);
      if(amhCfg.enableImf) {
          I2V_DBG_LOG(LEVEL_PRIV,MY_NAME,"imf socket: RX:RXe[%u,%u] Re:!RDY:TO:TOe[%u,%u,%u,%u] Sf:Bf:R#[%u,%u,%u]\n",
              imf_read_count,total_imf_read_error_count,imf_read_error_count,imf_read_poll_notready_count,imf_read_poll_to_count
              ,imf_read_poll_fail_count,imf_socket_fail,imf_socket_bind_fail,imf_socket_reset_count);
      }
      prior_sar_count = amhManagerStats.sar_tx_cnt;
      prior_ifm_count = amhManagerStats.imf_tx_cnt;
      amhManagerStats.sar_tx_slip = 0;
      amhManagerStats.sar_tx_max_slip = 0;
      #if defined(PRINT_RAMLIST) /* This is a lot of data so really be careful enabling this. */
      print_ramlist();
      #endif
      dump_amc_stats();
  }
  if(0x0 == (debug_counter % OUTPUT_MODULUS)) { /* Output rsuRepeatTable. */
      for (i=0; i < RSU_SAR_MESSAGE_MAX; i++){
          if(SNMP_ROW_NONEXISTENT != msgRepeatTable[i].rsuMsgRepeatStatus)
              print_msgRepeat(&msgRepeatTable[i]); /* Dump to syslog for client if they care. */
      }
  }
  if(0x0 == (debug_counter % 60)) {
      dump_amh_report();
  }
  debug_counter++;
}

STATIC void amh_broadcast_messages(void)
{
  uint16_t i=0;
  uint32_t ret = 0;
  amhSendParams sendparams;
  uint64_t UTC_msec = 0; 
  uint64_t Last_UTC_msec_epoch = 0;
  uint64_t delta = 0;
  amhBitmaskType amh_msgType = 0x0;   /* Bitmask of msg types. Not dsrc_id. */
  char_t cmd[RSU_MSG_FILENAME_LENGTH_MAX + 16]; /* Add 16 so we can put command + fname ie 'rm -f fname' */
  uint32_t one_sec_counter = 0; /* Rolling one second counter for internal scheduling of tasks. */

    /* Stay here till !mainloop. This is heart of AMH.*/
    for(; mainloop; amhEpochCount++) {

        UTC_msec = i2vUtilGetTimeInMs();
   
        /* 1 HZ processing: Do some internal house keeping. Update & prune RAM list.*/
        if(UTC_DELTA_1_sec <= (UTC_msec - Last_UTC_msec_epoch)){
            amh_load_files();          /* Enumerate disk for new or updated messages from MIB. These are client snmpset() requests. Dump results to syslog. */
            #if defined(ENABLE_AMH_PSID_TIMEOUT)
            amcPrunePSIDList();  /* 
                                  * Find inactive PSID's and remove from service.
                                  * Problem: The fault may be radio is busted yet AMH file takes the blame. User will get confused.
                                  * Question: What if that psid is being used by someone else? Like SPAT16.C?
                                  */
            #endif
            Last_UTC_msec_epoch = UTC_msec;
            dump_amh_status(); /* Periodically dump AMH status. */
            one_sec_counter++;
        } /* if UTC_DELTA_1_sec  */

        /*
         * Will send S&R messages straight from RAM to radio. No shm interaction.
         * All handling of RAM list is in this mainloop.
         */

        /* NTCIP-1218 Message output. */
        for (i=0; mainloop && (0 < msgRepeat_count) && (i < RSU_SAR_MESSAGE_MAX); i++){

            if(   (SNMP_ROW_ACTIVE != msgRepeatTable[i].rsuMsgRepeatStatus) /* Client controlled. */
               || (0 == msgRepeatTable[i].rsuMsgRepeatEnable)){             /* Client controlled. */
                continue; /* not active - skip it */
            }
            UTC_msec = i2vUtilGetTimeInMs(); /* Time will drift out, keep checking through loop. */

            /* Check if active message expired or notInService has become ready. These are well formed messages. */
            if(   (SNMP_ROW_NOTINSERVICE == msgRepeatTable[i].status) 
               || (SNMP_ROW_ACTIVE == msgRepeatTable[i].status)) {
                msgRepeatTable[i].status = check_stopStart_UTC_dsec(msgRepeatTable[i].rsuMsgRepeatDeliveryStart, msgRepeatTable[i].rsuMsgRepeatDeliveryStop, UTC_msec, WFALSE, i);
            }

            if(SNMP_ROW_ACTIVE != msgRepeatTable[i].status){ /* RSU controlled. */
                continue; /* not active - skip it */
            }

            if(UTC_msec < msgRepeat_statTable[i].lastSend){ /* Future check. */
                set_amh_error_state(AMH_TIME_FAULT);
                delta = 0;
            } else {
                if(0 != msgRepeat_statTable[i].lastSend)
                    delta = UTC_msec - msgRepeat_statTable[i].lastSend;
                else
                    delta = (uint64_t)msgRepeatTable[i].rsuMsgRepeatTxInterval; /* assuming first time sent so no lastSend info. Send right away */  
            }

            if((uint64_t)msgRepeatTable[i].rsuMsgRepeatTxInterval <= delta){

                memset(&sendparams, 0, sizeof(amhSendParams));

                sendparams.psid  = ((uint32_t)(msgRepeatTable[i].rsuMsgRepeatPsid[0])) << 8;
                sendparams.psid |= (uint32_t)msgRepeatTable[i].rsuMsgRepeatPsid[1];
                sendparams.psid  = (sendparams.psid << 8);
                sendparams.psid |= (uint32_t)msgRepeatTable[i].rsuMsgRepeatPsid[2];
                sendparams.psid  = (sendparams.psid << 8);
                sendparams.psid |= (uint32_t)msgRepeatTable[i].rsuMsgRepeatPsid[3];
                sendparams.priority = msgRepeatTable[i].rsuMsgRepeatPriority;
                sendparams.useCCH = msgRepeatTable[i].rsuMsgRepeatTxChannel;
                if(MSG_REPEAT_OPT_1609_BYPASS_OFF_MASK & msgRepeatTable[i].rsuMsgRepeatOptions){
                    sendparams.dsrcMsgId = msgRepeatTable[i].rsuMsgRepeatPayload[1]; /* First byte is always 0x00. Second has our value. */
                    if(MSG_REPEAT_OPT_SECURITY_OFF_MASK & msgRepeatTable[i].rsuMsgRepeatOptions){
                        sendparams.security = UNSECURED_WSA;
                    } else {
                        sendparams.security = SECURED_WSA;
                    }
                } else {
                    /* TODO: Careful, have to account for TLV bytes. Mark unknown for now. */
                    sendparams.dsrcMsgId = AMH_MSG_UNKNOWN_MASK;
                    sendparams.security = UNSECURED_WSA;
                }
                amh_msgType = dsrcMsgId_to_amhMsgType(sendparams.dsrcMsgId);  /* Only need this for forwarding mask. */
                
                while (0 != pthread_mutex_lock(&amhRadio_lock)){ /* Lock it from imf thread. */
                    usleep(500); 
                }
                /* Send message to IWMH for tx. We are done with message now. */
                if(I2V_RETURN_OK != (ret = amhTxMessage(msgRepeatTable[i].rsuMsgRepeatPayload, msgRepeatTable[i].rsuMsgRepeatPayload_length, &sendparams))){
                    /* Error is handled within call. No need to error handle. */
                }
                pthread_mutex_unlock(&amhRadio_lock);
                msgRepeat_statTable[i].lastSend = UTC_msec; /* Keep the time msg leaves the dock.*/
                /* Forward message to user via socket. */
                amhForwardManager(amh_msgType, msgRepeatTable[i].rsuMsgRepeatPayload, msgRepeatTable[i].rsuMsgRepeatPayload_length);
                msgRepeat_statTable[i].delta = delta;
                update_sar_count(ret, amh_msgType); /* Track stats. */
                /* Depending on where message is in RAM list will determine how much it slips past ready time. */
                if((0 != msgRepeat_statTable[i].lastSend) && (10 < amhEpochCount)){
                    if((uint64_t)msgRepeatTable[i].rsuMsgRepeatTxInterval < delta){
                        amhManagerStats.sar_tx_slip++;
                        delta = delta - (uint64_t)msgRepeatTable[i].rsuMsgRepeatTxInterval;
                        if(amhManagerStats.sar_tx_max_slip < delta) {
                            amhManagerStats.sar_tx_max_slip = delta;
                        }
                    }
                }
                amh_handle_diagnostics(amh_msgType, delta);
            }/* Time to send */
        }/*for*/

        /* May need to yeild here if lots of messages. Be kind, and share the program counter 
           usleep(DEFAULT_AMH_SLEEP_FACTOR); 
         */

        /* RSU 4.1 Message output. */
        for (i=0; mainloop && (0 < amhstoredbcnt) && (i < RSU_SAR_MESSAGE_MAX); i++){

            if((0 == amhstoredb[i].priority) && (0 == amhstoredb[i].channel)) {
                continue; /* not active - skip it */
            }
            UTC_msec = i2vUtilGetTimeInMs();

            /* Check to see if message has expired or not ready yet. */
            switch(check_stopStart_UTC_sec(amhstoredb[i].startMin, amhstoredb[i].endMin, UTC_msec)){
                case SNMP_ROW_DESTROY:
                    snprintf(cmd,sizeof(cmd),"rm -f %s",amhstoredb[i].myName);
                    if(system(cmd)){}; /* Silence unused warning compile warning. If this call fails its a dont care. */
                    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Msg has expired, removing[%s].\n",cmd);
                    memset(&amhstoredb[i],           0x0, sizeof(amhStoredRecT));
                    memset(&amhstoredb_statTable[i], 0x0, sizeof(MsgRepeat_Stat_t));
                    continue;
                    break;
                case SNMP_ROW_NOTINSERVICE:
                    continue;
                    break;
                default:
                    /* active. */
                    break;
            }

            sendparams.dsrcMsgId = amhstoredb[i].data.payload[1]; /* First byte is always 0x00. Second has our value. */
            amh_msgType          = dsrcMsgId_to_amhMsgType(sendparams.dsrcMsgId);
            if(UTC_msec < amhstoredb_statTable[i].lastSend){ /* Future check. */
                set_amh_error_state(AMH_TIME_FAULT);
                delta = 0;
            } else {
                if(0 != amhstoredb_statTable[i].lastSend)
                    delta = UTC_msec - amhstoredb_statTable[i].lastSend;
                else
                    delta = (uint64_t)amhstoredb[i].interval; /* assuming first time sent so no lastSend info. Send right away */  
            }

            if((uint64_t)amhstoredb[i].interval <= delta){
                memset(&sendparams, 0, sizeof(amhSendParams));
                sendparams.psid  = amhstoredb[i].psid;
                if(WTRUE == amhstoredb[i].signature)
                    sendparams.security = SECURED_WSA; 
                else
                    sendparams.security = UNSECURED_WSA;
                sendparams.useCCH = (int32_t)amhstoredb[i].channel;
                while (0 != pthread_mutex_lock(&amhRadio_lock)){ /* Lock it from imf thread. */
                    usleep(500); 
                }
                /* Send message to IWMH for tx. We are done with message now. */
                if(I2V_RETURN_OK != (ret = amhTxMessage(amhstoredb[i].data.payload, amhstoredb[i].data.count, &sendparams))){
                    /* Error is handled within call. No need to error handle. */
                }
                pthread_mutex_unlock(&amhRadio_lock);
                amhstoredb_statTable[i].lastSend = UTC_msec; /* Keep the time msg leaves the dock.*/
                /* Forward message via socket to user. */
                amhForwardManager(amh_msgType, amhstoredb[i].data.payload, amhstoredb[i].data.count);
                amhstoredb_statTable[i].delta = delta;
                update_sar_count(ret, amh_msgType); /* Track stats. */
                /* Depending on where message is in RAM list will determine how much it slips past ready time. */
                if((0 != amhstoredb_statTable[i].lastSend) && (10 < amhEpochCount)){
                    if((uint64_t)amhstoredb[i].interval < delta){
                        delta = delta - (uint64_t)amhstoredb[i].interval; /* How much did we slip. */
                        amhManagerStats.sar_tx_slip++;
                        if(amhManagerStats.sar_tx_max_slip < delta) {
                            amhManagerStats.sar_tx_max_slip = delta;
                        }
                    }
                }
                amh_handle_diagnostics(amh_msgType, delta);
            }/* Time to send */
        }/*for*/
        usleep(DEFAULT_AMH_SLEEP_FACTOR); /* Be kind, and share the program counter */
#if defined(MY_UNIT_TEST)
        break;
#endif
   }/*for*/
}
/*
 * End NTCIP-1218 implementation.
 */

/* Defaults from conf_table.h. */
STATIC int32_t amhSetConfDefault(char_t * tag, void * itemToUpdate)
{
  int32_t ret = AMH_AOK; /* Return success till proven fail. */

  if((NULL == tag) || (NULL == itemToUpdate)) {
      /* Nothing to do. */
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"amhSetConfDefault: NULL input.\n");
      ret = AMH_HEAL_FAIL;
  } else {
      if(0 == strcmp("RadioNum",tag)) {
          *(uint8_t *)itemToUpdate = I2V_RADIO_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%u)\n",tag,*(uint8_t *)itemToUpdate);
      } else if(0 == strcmp("ChannelNumber",tag)) {
          *(uint8_t *)itemToUpdate = I2V_RADIO_CHANNEL_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%u)\n",tag,*(uint8_t *)itemToUpdate);
      } else if(0 == strcmp("WSMExtension",tag)) {
          *(uint8_t *)itemToUpdate = AMH_WSM_EXTENSION_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%u)\n",tag,*(uint8_t *)itemToUpdate);
      } else if(0 == strcmp("EnableValidation",tag)) {
          *(bool_t *)itemToUpdate = AMH_ENABLE_VERIFICATION_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%u)\n",tag,*(bool_t *)itemToUpdate);
      } else if(0 == strcmp("EnableStoreAndRepeat",tag)) {
          *(bool_t *)itemToUpdate = AMH_ENABLE_SAR_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%u)\n",tag,*(bool_t *)itemToUpdate);
      } else if(0 == strcmp("SettingsOverride",tag)) {
          *(bool_t *)itemToUpdate = AMH_SETTINGS_OVERRIDE_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%u)\n",tag,*(bool_t *)itemToUpdate);
      } else if(0 == strcmp("ForcePSIDMatch",tag)) {
          *(bool_t *)itemToUpdate = AMH_FORCE_PSID_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%u)\n",tag,*(bool_t *)itemToUpdate);
      } else if(0 == strcmp("SendOverride",tag)) {
          *(bool_t *)itemToUpdate = AMH_SEND_OVERRIDE_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%u)\n",tag,*(bool_t *)itemToUpdate);
      } else if(0 == strcmp("ImmediateEnable",tag)) {
          *(bool_t *)itemToUpdate = AMH_ENABLE_IMF_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%u)\n",tag,*(bool_t *)itemToUpdate);
      } else if(0 == strcmp("ImmediateIPFilter",tag)) {
          *(bool_t *)itemToUpdate = AMH_IMF_IP_FILTER_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%u)\n",tag,*(bool_t *)itemToUpdate);
      } else if(0 == strcmp("ImmediateIP",tag)) {
          strcpy((char_t *)itemToUpdate,AMH_IMF_IP_DEFAULT_S);
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%s)\n",tag,(char_t *)itemToUpdate);
      } else if(0 == strcmp("ImmediatePort",tag)) {
          *(uint16_t *)itemToUpdate = I2V_IMF_PORT_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%u)\n",tag,*(uint16_t *)itemToUpdate);
      } else if(0 == strcmp("AMHForwardEnable",tag)) {
          *(uint32_t *)itemToUpdate = I2V_AMH_FORWARD_ENABLE_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%u)\n",tag,*(uint32_t *)itemToUpdate);
      } else if(0 == strcmp("AMHIntervalSelect",tag)) {
          *(uint8_t *)itemToUpdate = AMH_INTERVAL_SELECT_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%u)\n",tag,*(uint8_t *)itemToUpdate);
      } else {
          /* Nothing to do. */
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"amhSetConfDefault:(%s) item is not known, ignoring.\n",tag);
          ret = AMH_HEAL_FAIL;
      }
  }
  if(ret < AMH_AOK){
      set_amh_error_state(ret);
  }
  return ret;
}
STATIC int32_t amhUpdateCfg(void)
{
    cfgItemsTypeT cfgItems[] = {
        {"RadioNum",         (void *)i2vUtilUpdateUint8Value, &amhCfg.radioNum,      NULL,(ITEM_VALID| UNINIT)},
        {"ChannelNumber",      (void *)i2vUtilUpdateUint8Value,  &amhCfg.channelNum,    NULL,(ITEM_VALID| UNINIT)},
        {"WSMExtension",       (void *)i2vUtilUpdateUint8Value,  &amhCfg.hdrExtra,      NULL,(ITEM_VALID| UNINIT)},
        {"EnableValidation",   (void *)i2vUtilUpdateWBOOLValue,  &amhCfg.validateData,  NULL,(ITEM_VALID| UNINIT)},
        {"EnableStoreAndRepeat",(void *)i2vUtilUpdateWBOOLValue, &amhCfg.EnableSAR,     NULL,(ITEM_VALID| UNINIT)},
        {"SettingsOverride",   (void *)i2vUtilUpdateWBOOLValue,  &amhCfg.bcastGeneric,  NULL,(ITEM_VALID| UNINIT)},
        {"ForcePSIDMatch",     (void *)i2vUtilUpdateWBOOLValue,  &amhCfg.psidMatch,     NULL,(ITEM_VALID| UNINIT)},
        {"SendOverride",       (void *)i2vUtilUpdateWBOOLValue,  &amhCfg.lastChanceSend,NULL,(ITEM_VALID| UNINIT)},
        {"ImmediateEnable",    (void *)i2vUtilUpdateWBOOLValue,  &amhCfg.enableImf,     NULL,(ITEM_VALID| UNINIT)},
        {"ImmediateIPFilter",  (void *)i2vUtilUpdateWBOOLValue,  &amhCfg.restrictIP,    NULL,(ITEM_VALID| UNINIT)},
        {"ImmediateIP",        (void *)i2vUtilUpdateStrValue,    &amhCfg.imfIP,         NULL,(ITEM_VALID| UNINIT)},
        {"ImmediatePort",      (void *)i2vUtilUpdateUint16Value, &amhCfg.imfPort,       NULL,(ITEM_VALID| UNINIT)}, 
        {"AMHForwardEnable",   (void *)i2vUtilUpdateUint32Value, &amhCfg.amhFwdEnable,  NULL,(ITEM_VALID| UNINIT)},
        {"AMHIntervalSelect",  (void *)i2vUtilUpdateUint8Value,  &amhCfg.amhIntervalSelect, NULL,(ITEM_VALID| UNINIT)},
    };
    FILE   * f = NULL;
    char_t   fileloc[I2V_CFG_MAX_STR_LEN + I2V_CFG_MAX_STR_LEN + 100];
    int32_t  retVal = AMH_AOK;
    uint32_t i = 0;
    uint32_t i2vRet = I2V_RETURN_OK;

    memset(fileloc,'\0',sizeof(fileloc));
    /* Wait on I2V SHM to get I2V CFG. If fail then error out. No stand alone support.*/
    for(i=0; mainloop && (i < MAX_I2V_SHM_WAIT_ITERATIONS); i++){
        WSU_SHMLOCK_LOCKR(&shmPtr->cfgData.h.ch_lock);
        if(WTRUE == shmPtr->cfgData.h.ch_data_valid) {
            memcpy(&cfg, &shmPtr->cfgData, sizeof(cfg));  /* Copy config for later use. */
            WSU_SHMLOCK_UNLOCKR(&shmPtr->cfgData.h.ch_lock);
            break; /* Got it so break out.*/
        }
        WSU_SHMLOCK_UNLOCKR(&shmPtr->cfgData.h.ch_lock);
        usleep(MAX_i2V_SHM_WAIT_USEC);
    }

    memset(&fwdmsgCfg,0x0,sizeof(fwdmsgCfgItemsT));
    /* At this point shared memory should be ready. Read fwdmsgConfig for getting fwdmsgMask */
    /* Wait on I2V SHM to get FWDMSG CFG.*/
    for(i=0; mainloop && (i < MAX_I2V_SHM_WAIT_ITERATIONS); i++){
      WSU_SEM_LOCKR(&shmPtr->fwdmsgCfgData.h.ch_lock);

      if(WTRUE == shmPtr->fwdmsgCfgData.h.ch_data_valid) {
          /* Copy fwdmsgCfg to local structure */
          memcpy(&fwdmsgCfg.fwdmsgAddr, &shmPtr->fwdmsgCfgData.fwdmsgAddr, sizeof(fwdmsgCfg.fwdmsgAddr));  
          WSU_SEM_UNLOCKR(&shmPtr->fwdmsgCfgData.h.ch_lock);
          break;
      }

      WSU_SEM_UNLOCKR(&shmPtr->fwdmsgCfgData.h.ch_lock);
      usleep(MAX_i2V_SHM_WAIT_USEC);
    }

    amhForward = WFALSE;
    /* Check from fwdmsgCfg if SPaT/BSM/PSMBSM need to forwarded */
    for (i = 0; i < MAX_FWD_MSG_IP; i++)  {
    if (fwdmsgCfg.fwdmsgAddr[i].fwdmsgEnable)   {

        /* AMH */
        if ((fwdmsgCfg.fwdmsgAddr[i].fwdmsgMask) & 0x10)
            amhForward = WTRUE;
        }
    }

    /* If we fail to get I2V SHM update or sig'ed to exit beforehand then fail. */
    if((WFALSE == mainloop) || (MAX_I2V_SHM_WAIT_ITERATIONS <= i)) {
        retVal = AMH_TIMEOUT_WAITING_FOR_I2V_SHM;
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"mainloop == WFALSE: retVal=%d.\n",retVal);
    } else {

        strncpy(fileloc, cfg.config_directory, sizeof(fileloc));
        strncat(fileloc, confFileName, sizeof(fileloc)-1);

        if ((f = fopen(fileloc, "r")) == NULL) {
            retVal = AMH_CONF_FOPEN_ERROR;
            I2V_DBG_LOG(LEVEL_WARN,MY_NAME,"AMH_CONF_FOPEN_ERROR: Could not open config file(%s)\n",fileloc)
        }
    }

    if(AMH_AOK == retVal) {
        if(I2V_RETURN_OK != (i2vRet = i2vUtilParseConfFile(f, cfgItems, NUMITEMS(cfgItems), WFALSE, NULL))) {
            I2V_DBG_LOG(LEVEL_WARN,MY_NAME,"i2vUtilParseConfFile: not happy i2vRet=%u.\n",i2vRet);
            retVal = AMH_LOAD_CONFIG_FAIL;
            set_amh_error_state(AMH_LOAD_CONFIG_FAIL); /* Note for user. */
        }
        if(NULL != f) fclose(f);
        
        /* Check IP's or anything else for extra wellness above what I2V_UTIL does. */
        if(I2V_RETURN_OK != i2vCheckIPAddr(amhCfg.imfIP, I2V_IP_ADDR_V4)) {
            cfgItems[10].state = (ITEM_VALID|BAD_VAL);
            retVal = AMH_LOAD_CONFIG_FAIL;
            set_amh_error_state(AMH_LOAD_CONFIG_FAIL); /* Note for user. */
        }

        /* Regardless of i2vRet, check. */
        for (i = 0; i < NUMITEMS(cfgItems); i++) {
            if (   ((ITEM_VALID|BAD_VAL) == cfgItems[i].state) 
                || ((ITEM_VALID|UNINIT) == cfgItems[i].state)) {
                I2V_DBG_LOG(LEVEL_CRIT,MY_NAME,"config override on (%s): Using default.\n", cfgItems[i].tag);
                if(AMH_AOK == (retVal = amhSetConfDefault(cfgItems[i].tag, cfgItems[i].itemToUpdate))) {
                    cfgItems[i].state = (ITEM_VALID|INIT);
                    set_amh_error_state(AMH_HEAL_CFGITEM); /* Note for user. */
                    retVal = AMH_AOK;
                } else {
                    break; /* Heal has failed. FATAL. */
                }
            }
        }
    }

    if(AMH_AOK == retVal) {
        /* Do unified updates. */
        if (cfg.bcastLockStep) {  /* all rse apps broadcast on same radio and channel */
            amhCfg.channelNum = cfg.uchannel;
            amhCfg.radioNum = cfg.uradioNum;
        }
        if (cfg.iwmhAppEnable) {
            /* overrides bcastLockStep and local cfg channel setting */
            amhCfg.channelNum = IWMH_CHANNEL_NUMBER(amhCfg.radioNum, &cfg);
        }
    }
    /* Setting twice won't hurt and if you missed it we got it. */
    if(AMH_AOK != retVal) {
        set_amh_error_state(retVal);
    }
    return retVal;
}

/* Only call once on shutdown */
STATIC void amhCleanUp(void)
{  UserServiceType uService = {
        .radioNum = cfg.uradioNum,
        .action = DELETE,
        .userAccess = AUTO_ACCESS_UNCONDITIONAL,
        .psid = 0,
        .servicePriority = cfg.uwsaPriority, 
        .wsaSecurity = (cfg.security) ? SECURED_WSA : UNSECURED_WSA,
        .lengthPsc = 10,
        .channelNumber = cfg.uchannel,
        .lengthAdvertiseId = 15,
        .linkQuality = 20,
        .immediateAccess = (!AMCenablechsw) ? 1 : 0,
        .extendedAccess = (!AMCenablechsw) ? 65535: 0,
        .radioType = RT_DSRC,
  };
  uint16_t i;
  int32_t  ret    = I2V_RETURN_OK; /* Don't really care about the return value at this point. Time to exit. */
  uint32_t optret = I2V_RETURN_OK;

  if(1 == cfg.RadioType) {
      uService.radioType = RT_CV2X;
  } else {
      uService.radioType = RT_DSRC;
  }

  #if defined(EXTRA_DEBUG)
  I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"System shutting down - ALL active message files ceasing broadcast\n");
  #endif

  if(amhCfg.enableImf){
     close_IFM_queue(); 
  }

  for(i=0;i<MAX_AMH_EXTRA_PSID;i++) { /* De-Reg radio services from PSID List:: S&R & IMF msgs */
      if(0x0 != myAMCpsidlist[i].track_psid) {
          uService.psid = myAMCpsidlist[i].track_psid;
          I2V_UTIL_USER_SVC_REQ(&uService, &cfg, WMH_SVC_TIMEOUT, &ret, &optret);
          myAMCpsidlist[i].track_psid = 0x0;
      }
  }

  pthread_mutex_destroy(&amhRadio_lock);
  return;
}

STATIC void  amhInitStatics(void) /* Recover from soft reset */
{
    shmPtr = NULL;
    mainloop = WTRUE;
    amhstoredbcnt   = 0; /* RSU 4.1 */
    msgRepeat_count = 0; /* NTCIP-1218 */
    amhEpochCount  = 0x0; 
    pthread_mutex_init(&amhRadio_lock, NULL);
    memset(&cfg,0x0,sizeof(cfg));
    memset(&amhCfg,0x0,sizeof(amhCfg));
    memset(&amhManagerStats,0x0,sizeof(amhManagerStatsT));
    memset(&dest,0x0,sizeof(dest));
    memset(amhstoredb,0x0,sizeof(amhstoredb));
    memset(amhstoredb_statTable, 0x0, sizeof(msgRepeat_statTable));
    memset(myAMCpsidlist,0x0,sizeof(myAMCpsidlist));
    memset(msgRepeatTable,0x0,sizeof(msgRepeatTable));
    memset(msgRepeat_statTable, 0x0, sizeof(msgRepeat_statTable));
    amh_error_states = 0x0;
    sar_error_states = 0x0;
    imf_error_states = 0x0;
    lock_send = 0;
    amh_load_files_err_count = 0;
    prior_sar_count = 0;
    prior_ifm_count = 0;
    sar_datarate    = 0.0f;
    ifm_datarate    = 0.0f;
    debug_counter   = 0; /* rolling counter. */
    AMCenablechsw = WFALSE; /* On 5912 channel switching has issues? IWMH will overule. */
#if defined(AMH_LIMIT_RADIO_REG) /* TRACK number of Radio reg */
    my_reg_count     = 0x0;
    my_dereg_count   = 0x0;
#endif
#if !defined(MY_UNIT_TEST) /* Select flavor in unit_test. */
    strncpy(confFileName,AMH_CFG_FILE,sizeof(confFileName));
#endif
    imf_socket_fail = 0;
    imf_socket_bind_fail = 0;
    imf_socket_reset_count = 0;
    imf_read_count = 0;
    total_imf_read_error_count = 0;
    imf_read_error_count = 0;
    imf_read_poll_to_count = 0;
    imf_read_poll_notready_count = 0;
    imf_read_poll_fail_count = 0;
    memset(&amh_fwdmsg_send_lock,0x0,sizeof(amh_fwdmsg_send_lock));
    amhForward = WFALSE;
    memset(&fwdmsgCfg,0x0,sizeof(fwdmsgCfg));
    memset(&fwdmsgfd,0x0,sizeof(fwdmsgfd));
    memset(&amhFwdmsgData,0x0,sizeof(amhFwdmsgData));
    memset(sendbuf,0x0,sizeof(sendbuf));
}
/* Tailor for the PSIDs and directions the particular module needs. */
void *MessageStatsThread(void __attribute__((unused)) *arg)
{
  size_t shm_messageStats_size = sizeof(messageStats_t) * RSU_PSID_TRACKED_STATS_MAX;
  int32_t i,j,psid_match;
  uint32_t rolling_counter = 0;
#if defined(EXTRA_DEBUG)
  I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"messageStatsThr: Entry.\n");
#endif
  /* Open SHM. */
  if (NULL == (shm_messageStats_ptr = wsu_share_init(shm_messageStats_size, MSG_STATS_SHM_PATH))) {
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"messageStatsThr: SHM init failed.\n");
      set_amh_error_state(AMH_INIT_SHM_FAIL);
  } else {
      memset(&messageStats[0],0x0,shm_messageStats_size);
      while(mainloop) {
          sleep(1);
          if(0 == (rolling_counter % MSG_STATS_APP_SHM_UPDATE_RATE)) {
              for(i=0;i<RSU_PSID_TRACKED_STATS_MAX;i++){
                  /* If dirty then update from client. */
                  if(0x1 == shm_messageStats_ptr[i].dirty) {
                      psid_match = 0;
                      for(j=0;j<MAX_AMH_EXTRA_PSID;j++) {
                          if(   (0x0 != myAMCpsidlist[j].track_psid)
                              && (shm_messageStats_ptr[i].psid == myAMCpsidlist[j].track_psid)
                              && (RSU_CV2X_CHANNEL_DEFAULT == shm_messageStats_ptr[i].channel)
                              && (RSU_MESSAGE_COUNT_DIRECTION_IN != shm_messageStats_ptr[i].direction) ){
                              psid_match = 1;
                              break;
                          }
                      }
                      if(1 == psid_match) { /* Setup local copy of service. */
                          if (SNMP_ROW_ACTIVE == shm_messageStats_ptr[i].rowStatus) {
                              messageStats[i].psid = shm_messageStats_ptr[i].psid;
                              messageStats[i].channel = shm_messageStats_ptr[i].channel;
                              messageStats[i].direction = shm_messageStats_ptr[i].direction;
                              messageStats[i].start_utc_dsec = shm_messageStats_ptr[i].start_utc_dsec;
                          } else {
                              messageStats[i].psid = 0x0;
                              messageStats[i].channel = 0x0;
                              messageStats[i].direction = 0x0;
                              messageStats[i].start_utc_dsec = 0x0;
                          }
                          messageStats[i].count = 0; /* Do not clear SHM count. MIB does that. */
                          messageStats[i].rowStatus = shm_messageStats_ptr[i].rowStatus;
                          shm_messageStats_ptr[i].dirty = 0x0;
                      }
                  } else {
                      /* 
                       * If row is active and matches our local then update.
                       * Don't overwrite total, only add to it.
                       * Some PSID's shared like SPAT and MAP.
                       */
                      if(SNMP_ROW_ACTIVE == shm_messageStats_ptr[i].rowStatus) {
                          if (   (SNMP_ROW_ACTIVE == messageStats[i].rowStatus) 
                              && (messageStats[i].psid == shm_messageStats_ptr[i].psid)
                              && (0 < messageStats[i].count)){ 
                              shm_messageStats_ptr[i].count += messageStats[i].count;
                              messageStats[i].count = 0; /* Clear for next iteration. */
                          }
                      }
                  }
              } /* for */
          }
          rolling_counter++;
          #if defined(EXTRA_DEBUG)
          if(0 == (rolling_counter % 30)) {
              for(i=0;i<5;i++){
                  I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"MsgStatShm[%d]: dirty=%u, status=%d, count=%u, psid=0x%x, chan=%d, dir=%d, start=0x%lx.\n"
                          , i
                          , messageStats[i].dirty
                          , messageStats[i].rowStatus
                          , messageStats[i].count
                          , messageStats[i].psid
                          , messageStats[i].channel
                          , messageStats[i].direction
                          , messageStats[i].start_utc_dsec);
              }
          }
          #endif
      } /* While */
  }
  pthread_exit(NULL);
}

int32_t MAIN(void)
{
    struct sigaction sa;
    int32_t ret = AMH_AOK; /* TRUE until proven FALSE */
    pthread_t imfThrId;
    pthread_t mib_rsuIFM_ThrId;

    amhInitStatics();

    /* Enable serial debug with I2V_DBG_LOG until i2v.conf::globalDebugFlag says otherwise. */
    i2vUtilEnableDebug(MY_NAME);
    /* LOGMGR assumed up by now. Could retry if fails. */
    if(0 != i2vUtilEnableSyslog(MY_ERR_LEVEL, MY_NAME)){
        I2V_DBG_LOG(LEVEL_WARN,MY_NAME,"Failed to open syslog. Only serial output available,\n");
    }

    if (NULL == (shmPtr = wsu_share_init(sizeof(i2vShmMasterT), I2V_SHM_PATH))) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Main shared memory init failed\n");
        return AMH_INIT_SHM_FAIL; /* FATAL. */
    }

    if ((AMH_AOK == ret) && (AMH_AOK != (ret = amhUpdateCfg()))) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Failed to load config\n");     
    }

    if (AMH_AOK == ret) {

        /* catch SIGINT/SIGTERM */
        memset(&sa, 0, sizeof(sa));
        sa.sa_handler = amhSigHandler;
        sigaction(SIGINT, &sa, NULL);
        sigaction(SIGTERM, &sa, NULL);

#ifdef HSM 
        int i;
        /* If SSP is enabled, copy the values in global variables */
        /* They are copied to the messages sent out for TIM and MAP */
        if (cfg.timSspEnable && cfg.security)  {

            /* Set the Ssp length */
            sspLenTim = strlen((char_t *)cfg.timSsp)/2;   

            /* Convert the string into byte array */
            for (i = 0; i < sspLenTim; i++)    {
                sspTim[i] = (fromHexDigit(cfg.timSsp[(i * 2) + 0]) << 4) +
                                    (fromHexDigit(cfg.timSsp[(i * 2) + 1]) << 0);
            }

            if (cfg.timBitmappedSspEnable)  {

                sspBitmappedTim = cfg.timBitmappedSspEnable;

                /* Convert the string into byte array */
                for (i = 0; i < sspLenTim; i++)    {
                    sspMaskTim[i] = (fromHexDigit(cfg.timSspMask[(i * 2) + 0]) << 4) +
                                        (fromHexDigit(cfg.timSspMask[(i * 2) + 1]) << 0);
                }
            }
        }

        if (cfg.mapSspEnable && cfg.security)  {

            /* Set the Ssp length */
            sspLenMap = strlen((char_t *)cfg.mapSsp)/2;   

            /* Convert the string into byte array */
            for (i = 0; i < sspLenMap; i++) {
                sspMap[i] = (fromHexDigit(cfg.mapSsp[(i * 2) + 0]) << 4) +
                                    (fromHexDigit(cfg.mapSsp[(i * 2) + 1]) << 0);
            }

            if (cfg.mapBitmappedSspEnable)  {

                sspBitmappedMap = cfg.mapBitmappedSspEnable;

                /* Convert the string into byte array */
                for (i = 0; i < sspLenMap; i++)    {
                    sspMaskMap[i] = (fromHexDigit(cfg.mapSspMask[(i * 2) + 0]) << 4) +
                                        (fromHexDigit(cfg.mapSspMask[(i * 2) + 1]) << 0);
                }
            }
        }

        if (cfg.spatSspEnable && cfg.security)  {

            /* Set the Ssp length */
            sspLenSpat = strlen((char_t *)cfg.spatSsp)/2;   

            /* Convert the string into byte array */
            for (i = 0; i < sspLenSpat; i++) {
                sspSpat[i] = (fromHexDigit(cfg.spatSsp[(i * 2) + 0]) << 4) +
                                    (fromHexDigit(cfg.spatSsp[(i * 2) + 1]) << 0);
            }

            if (cfg.spatBitmappedSspEnable)  {

                sspBitmappedSpat = cfg.spatBitmappedSspEnable;

                /* Convert the string into byte array */
                for (i = 0; i < sspLenSpat; i++)    {
                    sspMaskSpat[i] = (fromHexDigit(cfg.spatSspMask[(i * 2) + 0]) << 4) +
                                        (fromHexDigit(cfg.spatSspMask[(i * 2) + 1]) << 0);
                }
            }
        }
#endif

        if (!i2vCheckDirOrFile(AMH_ACTIVE_DIR)) {
            ret = AMH_ACTIVE_MSG_DIR_ERROR;
            set_amh_error_state(AMH_ACTIVE_MSG_DIR_ERROR);
        }

        if (amhForward)     {
            if ((fwdmsgfd = mq_open(I2V_FWDMSG_QUEUE, O_RDWR|O_NONBLOCK)) == -1) {
                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME," mq() failed to open: errno = %d(%s)\n",errno,strerror(errno)); 
                ret = AMH_OPEN_MQ_FAIL;
            }
        }
    }
#if !defined(MY_UNIT_TEST)
    if((AMH_AOK == ret) && (amhCfg.enableImf)){        
        if(AMH_AOK != (ret = open_IFM_queue())){
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"ifm mq() failed to open: ret=%d,errno = %d(%s)\n",ret, errno,strerror(errno)); 
        }
    }
#endif
    /* Legacy: Via simple socket and RSU 4.1 file format. */
    if ((AMH_AOK == ret) && (amhCfg.enableImf)) {
        if (pthread_create(&imfThrId, NULL, imfThread, NULL) == -1) {
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"immediate forward thread failed.\n");
            ret = AMH_CREATE_IMF_THREAD_FAIL;
        } else {
            /* 20190208: QNX pthread_join fails; detach instead (will properly exit) */
            pthread_detach(imfThrId);
        }
    }

    /* NTCIP-1218: Via mq from rsuIFM.c. */
    if ((AMH_AOK == ret) && (amhCfg.enableImf)) {
        if (pthread_create(&mib_rsuIFM_ThrId, NULL, mib_rsuIFM_thread, NULL) == -1) {
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"mib rsuIFM thread failed.\n");
            ret = AMH_IMF_MIB_THREAD_FAIL;
        } else {
            /* 20190208: QNX pthread_join fails; detach instead (will properly exit) */
            pthread_detach(mib_rsuIFM_ThrId);
        }
    }
    if (AMH_AOK == ret){
        if (0 == (ret = pthread_create(&MessageStatsThreadID, NULL, MessageStatsThread, NULL))) {
            pthread_detach(MessageStatsThreadID);
        } else {
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"MessageStatsThreadID: Failed. Not Fatal: ret=%d.\n",ret);
            set_amh_error_state(AMH_MSG_STATS_MIB_THREAD_FAIL);/* Preserve info for user. */
            //ret = AMH_MSG_STATS_MIB_THREAD_FAIL;
        }
    }
    /* There is only one return and even with failure all must flow through here.*/
    if(AMH_AOK != ret) {
        mainloop = WFALSE;
        set_amh_error_state(ret); /* Skip update of shm since next step is exit. Update at bottom. */
    }
    
    /* 
     * MAIN LOOP: This will not return till !mainloop 
     */
    if (AMH_AOK == ret) {
#if defined(EXTRA_DEBUG)
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Entering mainloop.\n");
#endif
        amh_broadcast_messages(); /* NTCIP-1218 MIB and 4.1 MIB. */
    }
    #if defined(MY_UNIT_TEST)
    if ((AMH_AOK == ret) && (amhCfg.enableImf)) {
        /* To be really careful cancel. */
        if( 0 != pthread_cancel(imfThrId)) {
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"pthread_cancel(imfThrId) failed.\n");
        }
    }
    #endif
#if defined(EXTRA_DEBUG)
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"EXIT: error states: ret(%d) (0x%x) sar(0x%x) imf(0x%x).\n",ret,amh_error_states,sar_error_states,imf_error_states);
    if(amhCfg.enableImf) {
        I2V_DBG_LOG(LEVEL_PRIV,MY_NAME,"imf socket: RX:RXe[%u,%u] Re:!RDY:TO:TOe[%u,%u,%u,%u] Sf:Bf:R#[%u,%u,%u]\n",
            imf_read_count,total_imf_read_error_count,imf_read_error_count,imf_read_poll_notready_count,imf_read_poll_to_count
            ,imf_read_poll_fail_count,imf_socket_fail,imf_socket_bind_fail,imf_socket_reset_count);
    }
#endif
    /* Stop I2V_DBG_LOG output. Last chance to output to syslog. */
    i2vUtilDisableSyslog();
    i2vUtilDisableDebug();

    /* Clean up called once and only once, right here, right now. */
    if(ret != AMH_INIT_SHM_FAIL) {
        amhCleanUp();
        WSU_SHARE_KILL(shmPtr, sizeof(i2vShmMasterT)); /* To kill but i2v to delete. */
        shmPtr = NULL;
    }
    if(NULL != shm_rsuhealth_ptr) {
        wsu_share_kill(shm_rsuhealth_ptr, sizeof(rsuhealth_t));
        shm_rsuhealth_ptr = NULL;
    }
    return abs(ret);
}
