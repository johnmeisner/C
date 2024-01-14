/**************************************************************************
 *                                                                        *
 *     File Name:  i2v_util.c                                             *
 *     Author:                                                            *
 *         DENSO International America, Inc.                              *
 *         North America Research Laboratory, California Office           *
 *         3252 Business Park Drive                                       *
 *         Vista, CA 92081                                                *
 *                                                                        *
 **************************************************************************/
#define _GNU_SOURCE 1 /* for strtof() */
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <math.h>
#include <time.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <regex.h>
#include <stdarg.h>
#include <fcntl.h>
#include <ctype.h>
#include <mqueue.h>
#include "i2v_util.h"
#include "i2v_general.h"
#include "i2v_shm_master.h"
#if defined(J2735_2016)
#include "DSRC.h"
#elif defined(J2735_2023)
#include "MessageFrame.h"
#else
#ERROR: You must define  J2735_2016 or J2735_2023 in v2xcommon.mk
#endif
#include "conf_table.h"
#if defined(MY_UNIT_TEST)
#include "stubs.h"
#endif

/* for iwmh messaging */

/* 20181128: fixes for logging enablement revealed logging bugs introduced in this code and a
   discovery: convention changes were introduced in spite of me telling everyone NOT to do that;
   I'm not allowing mixed conventions in files I originally created */

#define MY_FAUX_PID_NUMBER   (6969)

static uint8_t    mywmhseqnum   = 0;

static bool_t     *mywmhmainloop = NULL;
static bool_t      myWMHRxInited = WFALSE;
static cfgItemsT *mycfgptr      = NULL;
static pthread_t  myWMHRxThrId;
static pid_t      mypid =  MY_FAUX_PID_NUMBER;
static pid_t      mywmhpid;   /* the pid of the iwmh instance */

static void (*myWSMRxFxn)(inWSMType *wsm) = NULL;
static void (*myVODFxn)(uint32_t result, uint32_t seqnum) = NULL;

#ifdef EXTRA_DEBUG
/* there were print statements that used these values, leaving for now */
/* debug timing */
static int32_t firstsec = 0;
static int32_t firstusec = 0;
static int32_t lastsec = 0;
static int32_t lastusec = 0;
static struct timeval tv;
#endif /* EXTRA_DEBUG */

static char_t  qname_i2vFwdmsgQueue[I2V_CFG_MAX_STR_LEN];
static mqd_t   i2vFwdmsgQueue;

static uint32_t i2v_utilFwdMsglocked = 0x1; /* Locked till opened properly */
uint32_t i2v_utilFwdMsglockfailcnt = 0x0;

static uint32_t i2v_utillocked            = 0x1; /* Locked till opened properly */
static uint32_t i2v_utillockfailcnt       = 0x0;
static uint32_t i2v_utilEnableDebug       = 0x0;
static uint32_t i2v_utilSysLogEnableDebug = 0x0;

#define I2V_FWDMSG_QUEUE_DEFAULT_FLAGS (0) //(O_NONBLOCK)
#define I2V_FWDMSG_QUEUE_MSG           (32)
#define I2V_FWDMSG_QUEUE_MSG_SIZE      (sizeof(fwdmsgData) + 1) 
/* 
 * Syslogs: Check SHM at runtime: i2v_cfg.h::shm_ptr->cfgData.nosyslog.
 * Until LOGMGR gets up and running there is no syslogs.
 */
STATIC bool_t i2vSysLogGlobalControl = WFALSE;

/* printf output: Check SHM at runtime: i2v_cfg.h::shm_ptr->cfg.globalDebugFlag. */
STATIC bool_t i2vGlobalDebugFlag = WFALSE;

/* 
 * SERIAL DEBUG only available after i2v.c parses i2v.conf.
 * SYSLOG only available after LOGMGR running.
 */
STATIC char_t     UTIL_NAME[I2V_LOG_NAME_MAX] =  "i2v_util";
/* Lowest priority to limit output. */
STATIC i2vLogSevT UTIL_ERR_LEVEL              = LEVEL_PRIV; /* Do not send i2v_util.c debug to serial by default. */

/* Open only one instance per session. */
STATIC mqd_t syslogQueue;
STATIC uint32_t i2vUtilAddToSysLog_err_count = 0x0;

void i2vInitStatics(void)
{
  mywmhseqnum   = 0;

  mywmhmainloop = NULL;
  myWMHRxInited = WFALSE;
  mycfgptr      = NULL;
  mypid =  MY_FAUX_PID_NUMBER;

#ifdef EXTRA_DEBUG
 firstsec = 0;
 firstusec = 0;
 lastsec = 0;
 lastusec = 0;
 //timeval tv;
#endif 

 i2v_utilFwdMsglocked = 0x1; /* Locked till opened properly */
 i2v_utilFwdMsglockfailcnt = 0x0;

 i2v_utillocked            = 0x1; /* Locked till opened properly */
 i2v_utillockfailcnt       = 0x0;
 i2v_utilEnableDebug       = 0x0;
 i2v_utilSysLogEnableDebug = 0x0;

 i2vSysLogGlobalControl = WFALSE;
 i2vGlobalDebugFlag = WFALSE;
 UTIL_ERR_LEVEL              = LEVEL_PRIV; /* Do not send i2v_util.c debug to serial by default. */
 
  i2vUtilAddToSysLog_err_count = 0x0;
}
/* 
 * Can't count on conf_agent being ready yet or snmpd. 
 * Setting up I2V SHM and waiting is a lot for one value so far.
 * Assumption: popen() is going to manage and file conflict between tasks calling.
 * We are only doing cat on the file so hopefully no issues.
 */
static int32_t get_debug_enable(void)
{
#define GET_DEBUG_ENABLE "/bin/cat /rwflash/configs/i2v.conf | grep I2VGlobalDebugFlag"
#define GET_DEBUG_ENABLE_REPLY  "%s"
  int32_t  ret = 0;
  char_t   reply_buf[128];
  char_t  *mystring = NULL;
  FILE    *fp = NULL;
  size_t   len = 0;
  uint32_t i;

  if(NULL != (fp = popen(GET_DEBUG_ENABLE, "r"))){
      if(NULL != fgets(reply_buf, sizeof(reply_buf), fp)){
          len = strnlen(reply_buf,sizeof(reply_buf));
          if((0 != len) && (len < sizeof(reply_buf))){
              if(strstr(reply_buf,"I2VGlobalDebugFlag") && (NULL != (mystring = strstr(reply_buf,"=")))) {
                  for(i=0; (i < len) && ('\0' != mystring[i]);i++){
                      if('0' == mystring[i]) {
                          ret = 0;
                          #if defined(EXTRA_DEBUG)
                          printf("\n%s: get_debug_enable: enabled.\n\n",UTIL_NAME);
                          #endif
                          break;
                      }
                      if('1' == mystring[i]) {
                          ret = 1;
                          #if defined(EXTRA_DEBUG)
                          printf("\n%s: get_debug_enable: disabled.\n\n",UTIL_NAME);
                          #endif
                          break;
                      }
                  }
                  if(len <= i) {
                      ret = -1;
                      fprintf(stderr,"\n%s: get_debug_enable: SSCANF failed: i(%d) mystring(%s) errno(%s)\n\n",UTIL_NAME,i,mystring,strerror(errno));
                  }
              } else {
                  ret = -2;
                  fprintf(stderr,"\n%s: get_debug_enable: SSCANF failed: reply_buf(%s) errno(%s)\n\n",UTIL_NAME,reply_buf,strerror(errno));
              }
          } else {
              fprintf(stderr,"\n%s: get_debug_enable: STRLEN from i2v.conf is bogus len(%lu)\n\n",UTIL_NAME,len);
              ret = -3;
          }
      } else {
          fprintf(stderr,"\n%s: get_debug_enable: FGETS failed(%s)\n\n",UTIL_NAME,strerror(errno));
          ret = -4;
      }
      pclose(fp);
  } else {
      fprintf(stderr,"\n%s: get_debug_enable: POPEN failed(%s)\n\n",UTIL_NAME,strerror(errno));
      ret = -5;
  }
  if(ret < 0){
      fflush(stderr);
  }
  return ret;
#undef GET_DEBUG_ENABLE
#undef GET_DEBUG_ENABLE_REPLY
}
/* 
 * MUST be called before i2vUtilEnableSyslog()
 * Call ONCE per task.
 * Should allow serial only output if syslog not enabled.
 */
void i2vUtilEnableDebug(char_t *name)
{
  if(0x0 == i2v_utilEnableDebug) {
      i2v_utilEnableDebug = 0x1;
      if((NULL != name) && (strlen(name) < I2V_LOG_NAME_MAX)){ /* Associates errors in i2v_util.c to the calling module, thats all. */
          strncpy(UTIL_NAME,name,sizeof(UTIL_NAME));
      } 
      i2vGlobalDebugFlag = WFALSE;
      if (1 == get_debug_enable()) {
          i2vGlobalDebugFlag = WTRUE;
      }
      i2v_utillocked         = 0x0;
      i2v_utillockfailcnt    = 0x0;
  }
}
void i2vUtilDisableDebug(void)
{
  i2v_utillocked = 0x1;
  i2vGlobalDebugFlag = WFALSE;
  fflush(stdout);
  fflush(stderr);
}
/* Enable/Disable Enhanced Debug:
 * Mutes all I2V_DBG_LOG with LEVEL greater than CRIT (see i2vLogSevT).
 * Enabled by default by i2vUtilEnableDebug.
 */
void i2vUtilEnableEnhanced(void)
{
  i2vGlobalDebugFlag = WTRUE;
}
void i2vUtilDisableEnhanced(void)
{
  i2vGlobalDebugFlag = WFALSE;
}
/*
 * Must call i2vUtilEnableDebug FIRST.
 */
int32_t i2vUtilEnableSyslog(i2vLogSevT level, char_t *name)
{
  int32_t ret = 0;

  name = name; /* Deprecated. Not needed */
  level = level;

  if(0x0 == i2v_utilSysLogEnableDebug) {
      i2v_utilSysLogEnableDebug = 0x1;
      /* This must exist. */
      if (-1 == (syslogQueue = mq_open(I2V_LOGMSG_QUEUE, O_WRONLY))) {
#if defined(EXTRA_DEBUG)
          if(i2vUtilAddToSysLog_err_count < 3) {
              i2vUtilAddToSysLog_err_count++;
              printf("%s|i2vUtilEnableSyslog: mq_open failed: errno(%s)\n\n", UTIL_NAME, strerror(errno));
          }
#endif
          ret = -1;
      } else {
          /* Turn on with default NAME & LEVEL if nothing valid provided as input. */
          i2vSysLogGlobalControl = WTRUE;
          ret = 0;
      }
  }
  return ret;
}
void i2vUtilDisableSyslog(void)
{
  i2vSysLogGlobalControl = WFALSE;
#if 0 /* mq can be recovered on next boot so dont bother on close. */
  i2vUtilCloseSyslogQueue(); /* This is a hard stop, tears down mq. */
#endif
}
/*
 * Open, unlink, close & open again. Only do once!
 * For now order of execution will gurantee everything open before playing with queues.
 */
uint32_t i2v_Util_Open_Queue(void)
{
  uint32_t ret = 0; 
  struct mq_attr attr;

  /* 
   * Create I2V FWDMSG Queue 
   */
  i2v_utilFwdMsglocked = 0x0;
  i2v_utilFwdMsglockfailcnt = 0x0;
  /* Unlink to destroy anything remaining. */
  errno = 0;
  sprintf(qname_i2vFwdmsgQueue , "%s", I2V_FWDMSG_QUEUE);
  i2vFwdmsgQueue = mq_open(I2V_FWDMSG_QUEUE, O_RDWR);
  mq_unlink(I2V_FWDMSG_QUEUE);
  mq_close(i2vFwdmsgQueue);

  if(errno) {
      if(9 == errno) { /* if the queue isn't present then we'll get this error */
          errno = 0;
      } else {      
          I2V_DBG_LOG(UTIL_ERR_LEVEL,UTIL_NAME,"i2v_Util_Open_Queue: unlink warning: errno(%s)\n",strerror(errno));
          ret |= 0x1; 
      }
  }

  /* Open new queue */
  attr.mq_flags     = I2V_FWDMSG_QUEUE_DEFAULT_FLAGS;
  attr.mq_maxmsg    = I2V_FWDMSG_QUEUE_MSG; 
  attr.mq_msgsize   = I2V_FWDMSG_QUEUE_MSG_SIZE;
  attr.mq_curmsgs   = 0;
  if(-1 == (i2vFwdmsgQueue = mq_open(I2V_FWDMSG_QUEUE, O_RDWR | O_CREAT, 0644, &attr))) {
      I2V_DBG_LOG(UTIL_ERR_LEVEL,UTIL_NAME,"i2v_Util_Open_Queue: i2vFwdmsgQueue failed: errno (%s)\n",strerror(errno));
      ret |= 0x2;
  }

  /* Get attributes of new queue and verify */
  attr.mq_flags   = 0;
  attr.mq_maxmsg  = 0;
  attr.mq_msgsize = 0;
  attr.mq_curmsgs = 0;
  if(-1 == mq_getattr(i2vFwdmsgQueue,&attr)){
      I2V_DBG_LOG(UTIL_ERR_LEVEL,UTIL_NAME,"i2v_Util_Open_Queue: mq_getattr fail: errno(%s)\n",strerror(errno));
      ret |= 0x4;  
  }
#if defined(EXTRA_EXTRA_DEBUG)
    I2V_DBG_LOG(UTIL_ERR_LEVEL,UTIL_NAME,"i2vFwdmsgQueue:mq_flags=0x%x mq_maxmsg=%ld mq_msgsize=%ld mq_curmsgs=%ld\n", (uint32_t)attr.mq_flags,attr.mq_maxmsg,attr.mq_msgsize,attr.mq_curmsgs);
#endif
  /* Check return values against our programmed values */
  if(   (attr.mq_maxmsg  != I2V_FWDMSG_QUEUE_MSG)
     || (attr.mq_msgsize != I2V_FWDMSG_QUEUE_MSG_SIZE)){
      I2V_DBG_LOG(UTIL_ERR_LEVEL,UTIL_NAME,"i2v_Util_Open_Queue: Queue attributes wrong: errno(%s)\n",strerror(errno));
      ret |= 0x8; 
  }
  /* I2V FWDMSG QUEUE */

  /* Close Queue handles. We are open for business. */
  mq_close(i2vFwdmsgQueue);

  /* One last check if any error states on queue creation */
#if defined(EXTRA_EXTRA_DEBUG)
  if(errno)
      I2V_DBG_LOG(UTIL_ERR_LEVEL,UTIL_NAME,"i2v_Util_Open_Queue: errno final check(%s)\n",strerror(errno));
#endif
    return ret;
}
/*
 * Singular place to unlink & close Queues.
 */
void  i2v_Util_Close_Queue(void)
{
  /*
   * The queue handles can be in various states of open & close...
   * By unlinking, when the last process dies the queues should too...in theory
   */
  i2v_utilFwdMsglocked = 0x1;
  i2vFwdmsgQueue     = mq_open(qname_i2vFwdmsgQueue, O_WRONLY);
  mq_unlink(qname_i2vFwdmsgQueue);
  mq_close(i2vFwdmsgQueue);
}


/*
 * Config file parse functions: Not thread safe.
 */
int32_t i2vCheckMac(char_t * value)
{
  int32_t  ret = I2V_RETURN_OK;
  uint32_t i = 0;
  uint32_t length = 0;
  uint8_t  colonCount = 0;

  if(NULL == value){
      ret = I2V_RETURN_NULL_PTR;
  } else {
      length = strlen(value);
      if((length < IPB_SRV_MAC_MIN) || (IPB_SRV_MAC_MAX < length)) {
          ret = I2V_RETURN_RANGE_LIMIT_FAIL;
      } else {
          ret = I2V_RETURN_OK;

//TODO: Not exhaustive, can we do better? If only "::" is that legal?

          for(i=0,colonCount=0;(I2V_RETURN_OK==ret)&&(i<strlen(value));i++) {
              if(':' != value[i]) {
                  if(0xff == i2v_ascii_2_hex_nibble(value[i])) {
                      ret = I2V_RETURN_RANGE_LIMIT_FAIL;
                      break;
                  } 
              } else {
                  if(colonCount < 6) {
                      colonCount++;
                  } else {
                      ret = I2V_RETURN_RANGE_LIMIT_FAIL;
                      break;
                  }
              }              
          }      
      }
  }
  return ret;
}
int32_t i2vCheckSspString(char_t * data)
{
  int32_t ret = I2V_RETURN_OK;
  uint32_t i = 0;

  if(NULL == data) {
      ret = I2V_RETURN_NULL_PTR;
  } else {
      if(MAX_SSP_DATA < strlen(data)) {
          ret = I2V_RETURN_RANGE_LIMIT_FAIL;
      } else {
          for(i=0;i<strlen(data);i++) {
              if(0xff == i2v_ascii_2_hex_nibble(data[i])) {
                  ret = I2V_RETURN_RANGE_LIMIT_FAIL;
                  break;
              }
          }
      }
  }
  return ret;
}

int32_t i2vCheckPSID(char_t * data)
{
  int32_t  ret = I2V_RETURN_OK;
  uint32_t status = I2V_RETURN_OK;
  uint32_t psid = 0;
  if(NULL == data) {
      ret = I2V_RETURN_NULL_PTR;
  } else {
      if(I2V_RETURN_OK == i2vUtilUpdateUint32Value(&psid,data,I2V_PSID_MIN_S,I2V_PSID_MAX_S,NULL,&status,NULL)){
          /* Check PSID is valid range. */
          if(psid <= (uint32_t) PSID_1BYTE_MAX_VALUE){ 
          } else if(psid >= (uint32_t)PSID_2BYTE_MIN_VALUE && psid <= (uint32_t)PSID_2BYTE_MAX_VALUE){
          } else if(psid >= (uint32_t)PSID_3BYTE_MIN_VALUE && psid <= (uint32_t)PSID_3BYTE_MAX_VALUE){
          } else if(psid >= (uint32_t)PSID_4BYTE_MIN_VALUE && psid <= (uint32_t)PSID_4BYTE_MAX_VALUE){
          } else {
              ret = I2V_RETURN_RANGE_LIMIT_FAIL; /* Slightly different error. */
          }
      } else {
          ret = I2V_RETURN_RANGE_LIMIT_FAIL;
      }
  }
  return ret;
}
int32_t i2vCheckTxPower(char_t * value)
{
  int32_t ret = I2V_RETURN_OK;
  uint32_t i2vRet = I2V_RETURN_OK;
  uint32_t status = I2V_RETURN_OK;
  int32_t temp = 0;
  float32_t halfstep = 0.0f;
  float32_t floatItem = 0.0f;

  if(NULL == value) {
      ret = I2V_RETURN_NULL_PTR;
  } else {
      i2vRet = i2vUtilUpdateFloatValue(&floatItem,value,I2V_TX_POWER_MIN_S,I2V_TX_POWER_MAX_S,NULL,&status,NULL);
      if(I2V_RETURN_OK == i2vRet) {
          /* Web gui doesn't limit user input so we'll do it for them. */ 
          temp = (int32_t)(floatItem * 10.0f); /* Limit to one decimal place. */
          floatItem = (float32_t)temp;
          floatItem = floatItem / 10.0f;
          halfstep = floatItem - (float32_t)((int32_t)floatItem);

          /* Verify fraction is half step. */
          if ((0.5f != halfstep) && (0.0f != halfstep)) {
             ret = I2V_RETURN_RANGE_LIMIT_FAIL;
          }
      } else {
          ret = I2V_RETURN_RANGE_LIMIT_FAIL;
      }
  }
  return ret;
}
int32_t i2vCheckIPAddr(char_t * value, uint8_t ip_version)
{
  int32_t  ret = I2V_RETURN_OK;
  uint32_t i = 0;
  uint32_t length = 0;
 
  if(NULL == value) {
      ret = I2V_RETURN_NULL_PTR;
  } else {
      length = strlen(value);
      if(I2V_DEST_IP_MAX < length) {
          ret = I2V_RETURN_RANGE_LIMIT_FAIL;
      }
      if(0 == length) {
          /* Per element decide if empty is ok. Force compile default, if you care. */
          ret = I2V_RETURN_RANGE_LIMIT_FAIL;
      }
      if((I2V_RETURN_OK == ret) && (0 < length)) {
          for(i=0;i < length; i++) {
          if(0 == ip_version) { /* IPv4 */
              if(0 == isdigit(value[i])) {
                  if(('.' != value[i]) && ('/' != value[i])) {
                      ret = I2V_RETURN_RANGE_LIMIT_FAIL;
                      break;
                  }
              }
          } else { /* IPv6 */
              if(0 == isxdigit(value[i])) {
                  if((':' != value[i]) && ('/' != value[i]) && ('X' != value[i])) {
                      ret = I2V_RETURN_RANGE_LIMIT_FAIL;
                      break;
                  }
              }
          }
          }
      }
  }
  return ret;
}
//TODO: remove '\n' at end
//    : make a function
/* Find end point for strto(f|d|l)() to compare result. */
uint32_t findEndString(char_t * data, int32_t * end, uint32_t * status)
{
  uint32_t ret = I2V_RETURN_OK;
  int32_t  foundItem = 0;
  int32_t  i = 0;

  if((NULL == data) || (NULL == end) || (NULL == status)) {
      ret = I2V_RETURN_NULL_PTR;
  } else{
      for(i=(int32_t)strlen(data)-1,foundItem=0;(0 <= i);i--) {
          if(' ' == data[i]){
              /* Still empty, keep going. */
          } else {
              foundItem = 1;
              break;
          }
      }
      if(0 == foundItem) {
          I2V_DBG_LOG(UTIL_ERR_LEVEL,UTIL_NAME,"findEndString: failed data(%s) is empty string.\n", data);
          *status = I2V_RETURN_RANGE_LIMIT_FAIL;
          ret = I2V_RETURN_RANGE_LIMIT_FAIL;
      } else {
          if(i < (int32_t)strlen(data)) {
              i++; /* Move endptr beyond this item if we are not at end already. */
          }
          *end = i;
      }
  }
  return ret;
}
uint32_t i2vUtilUpdateWBOOLValue(void *configItem, char_t *val, char_t GCC_UNUSED_VAR *min,
                             char_t GCC_UNUSED_VAR *max, char_t GCC_UNUSED_VAR  **trange, uint32_t *status, i2vLogWrapper GCC_UNUSED_VAR *logger)
{
  bool_t * cfg = configItem;
  int64_t   value = 0; /* needs to be this big to hold LONG_MAX/LONG_MIN. */
  int64_t   minval = 0;
  int64_t   maxval = 0;
  int32_t   i = 0;
  char_t  * endptr = NULL;
  uint8_t   have_minmax = 0x0; /* bitmask */
  uint32_t  i2vRet = I2V_RETURN_OK;

  trange = trange; /* Silence of the warnings. */

  /* Error check input and grab min/max vals if available. */
  if((NULL == configItem) || (NULL == val) || NULL == status) {
      I2V_DBG_LOG(UTIL_ERR_LEVEL,UTIL_NAME,"bool_t: I2V_RETURN_NULL_PTR failed.\n");
      if(NULL != status) { 
          *status = I2V_RETURN_NULL_PTR;
      }
      return I2V_RETURN_NULL_PTR;
  } 
  if((I2V_CFG_MAX_STR_LEN < strlen(val)) || (0 == strlen(val))){
      *status = I2V_RETURN_RANGE_LIMIT_FAIL;
      I2V_DBG_LOG(UTIL_ERR_LEVEL,UTIL_NAME,"bool_t: val length failed=%u.\n",strlen(val));
      return I2V_RETURN_RANGE_LIMIT_FAIL;
  }
  if((NULL != min) && (strcmp((const char_t *)min, I2V_CFG_UNDEFINED) != 0)) {
      if((I2V_CFG_MAX_STR_LEN < strlen(min)) || (0 == strlen(min))){
          *status = I2V_RETURN_RANGE_LIMIT_FAIL;
          I2V_DBG_LOG(UTIL_ERR_LEVEL,UTIL_NAME,"bool_t: min length failed=%u.\n",strlen(min));
          return I2V_RETURN_RANGE_LIMIT_FAIL;
      }

      if(I2V_RETURN_OK != (i2vRet = findEndString(min,&i,status))) {
          return i2vRet;
      }
      errno=0;
      minval = strtol((const char_t *)min, &endptr,0);
      if (   (endptr != &min[i])
          || ((ERANGE == errno) && ((LONG_MIN == minval) || (LONG_MAX == minval)))
          || ((0 != errno) && (0 == minval)) 
          || ((minval < WFALSE) || (WTRUE < minval))) {
          I2V_DBG_LOG(UTIL_ERR_LEVEL,UTIL_NAME,"bool_t: min(%s) strtof failed: endptr(0x%x) != min(0x%x) errno=%d.\n", min,endptr,&min[i],errno);
          *status = I2V_RETURN_RANGE_LIMIT_FAIL;
          return I2V_RETURN_RANGE_LIMIT_FAIL;
      }
      have_minmax |= 0x1;
  }
  if ((NULL != max) && (strcmp((const char_t *)max, I2V_CFG_UNDEFINED) != 0)) { 
      if((I2V_CFG_MAX_STR_LEN < strlen(max)) || (0 == strlen(max))){
          *status = I2V_RETURN_RANGE_LIMIT_FAIL;
          I2V_DBG_LOG(UTIL_ERR_LEVEL,UTIL_NAME,"bool_t: max length failed=%u.\n",strlen(max));
          return I2V_RETURN_RANGE_LIMIT_FAIL;
      }

      if(I2V_RETURN_OK != (i2vRet = findEndString(max,&i,status))) {
          return i2vRet;
      }
      errno=0;
      maxval = strtol((const char_t *)max, &endptr,0);
      if (   (endptr != &max[i])
          || ((ERANGE == errno) && ((LONG_MIN == maxval) || (LONG_MAX == maxval)))
          || ((0 != errno) && (0 == maxval))
          || ((maxval < WFALSE) || (WTRUE < maxval))) {
          I2V_DBG_LOG(UTIL_ERR_LEVEL,UTIL_NAME,"bool_t: max(%s) strtof failed: endptr(0x%x) != max(0x%x) errno=%d.\n", max,endptr,&max[i],errno);
          *status = I2V_RETURN_RANGE_LIMIT_FAIL;
          return I2V_RETURN_RANGE_LIMIT_FAIL;
      }
      have_minmax |= 0x2;
  }

  /*
   * Convert val and compare against min/max.
   * If it fails then calling app may provide default or appropriate action.
   */

  if(I2V_RETURN_OK != (i2vRet = findEndString(val,&i,status))) {
      return i2vRet;
  }
  errno=0;
  value = strtol((const char_t *)val, &endptr,0);
  if(   (endptr != &val[i]) 
     || ((ERANGE == errno) && ((LONG_MIN == value) || (LONG_MAX == value)))
     || ((0 != errno) && (0 == value)) 
     || ((value < WFALSE) || (WTRUE < value))) {
      I2V_DBG_LOG(UTIL_ERR_LEVEL,UTIL_NAME,"bool_t: val(%s)(%ld) strtof failed: endptr(0x%x) != val(0x%x) errno(%s)\n", val,value,endptr,&val[i],strerror(errno));
      *status = I2V_RETURN_RANGE_LIMIT_FAIL;
      return I2V_RETURN_RANGE_LIMIT_FAIL;
  } else {
      #if defined(EXTRA_DEBUG)
      I2V_DBG_LOG(UTIL_ERR_LEVEL,UTIL_NAME,"bool_t: val(%s)=(%ld): min/max{%ld, %ld}: errno=%d.\n", val, value, minval, maxval,errno);
      #endif
      *cfg = (bool_t)value; /* Truncate what we give back just in case. */
      if((0x1 & have_minmax) && (value < minval)) { /* If user provided min/max then do range check else we're good. */
          *cfg = (bool_t)minval;
          I2V_DBG_LOG(UTIL_ERR_LEVEL,UTIL_NAME,"bool_t: MIN LIMIT, value (%ld) < minval (%ld). value set to minval\n", value, minval);
          *status = I2V_RETURN_MIN_LIMIT_FAIL;
          return I2V_RETURN_MIN_LIMIT_FAIL;
      } 
      if((0x2 & have_minmax) &&  (value > maxval)) {
          *cfg = (bool_t)maxval;
          I2V_DBG_LOG(UTIL_ERR_LEVEL,UTIL_NAME,"uint8_t: MAX LIMIT, value (%ld) > maxval (%ld). value set to maxval\n", value, maxval);
          *status = I2V_RETURN_MAX_LIMIT_FAIL;
          return I2V_RETURN_MAX_LIMIT_FAIL;
      }
  }
  *status = I2V_RETURN_OK;
  return I2V_RETURN_OK;
}
//what if buffer is really not this big?
uint32_t i2vUtilUpdateStrValue(void *configItem, char_t *val, char_t GCC_UNUSED_VAR *min,
                           char_t GCC_UNUSED_VAR *max, char_t GCC_UNUSED_VAR  **trange, uint32_t *status, i2vLogWrapper GCC_UNUSED_VAR *logger)
{
  char_t * cfg = configItem;
  uint8_t foundItem = 0;
  uint32_t i = 0;

  /* Error check inputs. */
  if((NULL == configItem) || (NULL == val) || NULL == status) {
      I2V_DBG_LOG(UTIL_ERR_LEVEL,UTIL_NAME,"string: I2V_RETURN_NULL_PTR failed.\n");
      if(NULL != status) { 
          *status = I2V_RETURN_NULL_PTR;
      }
      return I2V_RETURN_NULL_PTR;
  }
  if(I2V_CFG_MAX_STR_LEN < strlen(val)){
      *status = I2V_RETURN_RANGE_LIMIT_FAIL;
      I2V_DBG_LOG(UTIL_ERR_LEVEL,UTIL_NAME,"string: val length failed=%u.\n",strlen(val));
      return I2V_RETURN_STR_MAX_LIMIT_FAIL;
  }

  cfg = configItem;
  *cfg='\0'; /* null terminate till we verify val is a string we want. */

  /* Check for illegal or unwanted ascii chars. */
  for(i=0;i<strlen(val);i++) { /* Illegal ascii values. */
      if((val[i]<0x20)||(0x7E < val[i])||('\\' == val[i])){
          I2V_DBG_LOG(UTIL_ERR_LEVEL,UTIL_NAME,"string: illegal ascii value=0x%x.\n",val[i]);
          *status = I2V_RETURN_RANGE_LIMIT_FAIL;
          return I2V_RETURN_RANGE_LIMIT_FAIL;
      }
  }

  /* All spaces check. Legacy conf this will pass. */
  for(i=0;i<strlen(val);i++) {
      if(' ' != val[i]) {
          foundItem = 1;
          break;
      }
  }
  /* All spaces is not the same as empty. */
  if(0 == foundItem) {
      I2V_DBG_LOG(UTIL_ERR_LEVEL,UTIL_NAME,"string: val(%s) is all spaces.\n",val);
      *status = I2V_RETURN_RANGE_LIMIT_FAIL;
      return I2V_RETURN_RANGE_LIMIT_FAIL;
  }

  /* Passed all our checks so copy over. */
  strcpy(cfg,(char_t *)val);
  *status = I2V_RETURN_OK;
  return I2V_RETURN_OK;
}

/* adding these now - they may never get used but just in case - no need to add later */
uint32_t i2vUtilUpdateInt8Value(void *configItem, char_t *val, char_t *min, 
     char_t *max, char_t **trange, uint32_t *status, i2vLogWrapper GCC_UNUSED_VAR *logger)
{
  int8_t * cfg = configItem;
  int64_t   value = 0; /* needs to be this big to hold LONG_MAX/LONG_MIN. */
  int64_t   minval = 0;
  int64_t   maxval = 0;
  int32_t   i = 0;
  char_t  * endptr = NULL;
  uint8_t   have_minmax = 0x0; /* bitmask */
  uint32_t  i2vRet = I2V_RETURN_OK;

  trange = trange; /* Silence of the warnings. */

  /* Error check input and grab min/max vals if available. */
  if((NULL == configItem) || (NULL == val) || NULL == status) {
      I2V_DBG_LOG(UTIL_ERR_LEVEL,UTIL_NAME,"int8_t: I2V_RETURN_NULL_PTR failed.\n");
      if(NULL != status) { 
          *status = I2V_RETURN_NULL_PTR;
      }
      return I2V_RETURN_NULL_PTR;
  } 
  if((I2V_CFG_MAX_STR_LEN < strlen(val)) || (0 == strlen(val))){
      *status = I2V_RETURN_RANGE_LIMIT_FAIL;
      I2V_DBG_LOG(UTIL_ERR_LEVEL,UTIL_NAME,"int8_t: val length failed=%u.\n",strlen(val));
      return I2V_RETURN_RANGE_LIMIT_FAIL;
  }
  if((NULL != min) && (strcmp((const char_t *)min, I2V_CFG_UNDEFINED) != 0)) {
      if((I2V_CFG_MAX_STR_LEN < strlen(min)) || (0 == strlen(min))){
          *status = I2V_RETURN_RANGE_LIMIT_FAIL;
          I2V_DBG_LOG(UTIL_ERR_LEVEL,UTIL_NAME,"int8_t: min length failed=%u.\n",strlen(min));
          return I2V_RETURN_RANGE_LIMIT_FAIL;
      }

      if(I2V_RETURN_OK != (i2vRet = findEndString(min,&i,status))) {
          return i2vRet;
      }
      errno=0;
      minval = strtol((const char_t *)min, &endptr,0);
      if (   (endptr != &min[i])
          || ((ERANGE == errno) && ((LONG_MIN == minval) || (LONG_MAX == minval)))
          || ((0 != errno) && (0 == minval)) 
          || ((minval < SCHAR_MIN)||(SCHAR_MAX < minval))) {
          I2V_DBG_LOG(UTIL_ERR_LEVEL,UTIL_NAME,"int8_t: min(%s) strtof failed: endptr(0x%x) != min(0x%x) errno=%d.\n", min,endptr,&min[i],errno);
          *status = I2V_RETURN_RANGE_LIMIT_FAIL;
          return I2V_RETURN_RANGE_LIMIT_FAIL;
      }
      have_minmax |= 0x1;
  }
  if ((NULL != max) && (strcmp((const char_t *)max, I2V_CFG_UNDEFINED) != 0)) { 
      if((I2V_CFG_MAX_STR_LEN < strlen(max)) || (0 == strlen(max))){
          *status = I2V_RETURN_RANGE_LIMIT_FAIL;
          I2V_DBG_LOG(UTIL_ERR_LEVEL,UTIL_NAME,"int8_t: max length failed=%u.\n",strlen(max));
          return I2V_RETURN_RANGE_LIMIT_FAIL;
      }

      if(I2V_RETURN_OK != (i2vRet = findEndString(max,&i,status))) {
          return i2vRet;
      }
      errno=0;
      maxval = strtol((const char_t *)max, &endptr,0);
      if (   (endptr != &max[i])
          || ((ERANGE == errno) && ((LONG_MIN == maxval) || (LONG_MAX == maxval)))
          || ((0 != errno) && (0 == maxval)) 
          || ((maxval < SCHAR_MIN)||(SCHAR_MAX < maxval))) {
          I2V_DBG_LOG(UTIL_ERR_LEVEL,UTIL_NAME,"int8_t: max(%s) strtof failed: endptr(0x%x) != max(0x%x) errno=%d.\n", max,endptr,&max[i],errno);
          *status = I2V_RETURN_RANGE_LIMIT_FAIL;
          return I2V_RETURN_RANGE_LIMIT_FAIL;
      }
      have_minmax |= 0x2;
  }

  /*
   * Convert val and compare against min/max.
   * If it fails then calling app may provide default or appropriate action.
   */

  if(I2V_RETURN_OK != (i2vRet = findEndString(val,&i,status))) {
      return i2vRet;
  }
  errno=0;
  value = strtol((const char_t *)val, &endptr,0);
  if(   (endptr != &val[i]) 
     || ((ERANGE == errno) && ((LONG_MIN == value) || (LONG_MAX == value)))
     || ((0 != errno) && (0 == value)) 
     || ((value < SCHAR_MIN)||(SCHAR_MAX < value))) {
      I2V_DBG_LOG(UTIL_ERR_LEVEL,UTIL_NAME,"int8_t: val(%s)(%d) strtof failed: endptr(0x%x) != val(0x%x) errno(%s).\n", val,value,endptr,&val[i],strerror(errno));
      *status = I2V_RETURN_RANGE_LIMIT_FAIL;
      return I2V_RETURN_RANGE_LIMIT_FAIL;
  } else {
      #if defined(EXTRA_DEBUG)
      I2V_DBG_LOG(UTIL_ERR_LEVEL,UTIL_NAME,"int8_t: val(%s)=(%d): min/max{%d, %d}: errno=%d.\n", val, value, minval, maxval,errno);
      #endif
      *cfg = (int8_t)value; /* Truncate what we give back just in case. */
      if((0x1 & have_minmax) && (value < minval)) { /* If user provided min/max then do range check else we're good. */
          *cfg = (int8_t)minval;
          I2V_DBG_LOG(UTIL_ERR_LEVEL,UTIL_NAME,"int8_t: MIN LIMIT, value (%ld) < minval (%ld). value set to minval\n", value, minval);
          *status = I2V_RETURN_MIN_LIMIT_FAIL;
          return I2V_RETURN_MIN_LIMIT_FAIL;
      } 
      if((0x2 & have_minmax) &&  (value > maxval)) {
          *cfg = (int8_t)maxval;
          I2V_DBG_LOG(UTIL_ERR_LEVEL,UTIL_NAME,"int8_t: MAX LIMIT, value (%ld) > maxval (%ld). value set to maxval\n", value, maxval);
          *status = I2V_RETURN_MAX_LIMIT_FAIL;
          return I2V_RETURN_MAX_LIMIT_FAIL;
      }
  }
  *status = I2V_RETURN_OK;
  return I2V_RETURN_OK;
}

uint32_t i2vUtilUpdateUint8Value(void *configItem, char_t *val, char_t *min, 
     char_t *max, char_t **trange, uint32_t *status, i2vLogWrapper GCC_UNUSED_VAR *logger)
{
  uint8_t * cfg = configItem;
  int64_t   value = 0; /* needs to be this big to hold LONG_MAX/LONG_MIN. */
  int64_t   minval = 0;
  int64_t   maxval = 0;
  int32_t   i = 0;
  char_t  * endptr = NULL;
  uint8_t   have_minmax = 0x0; /* bitmask */
  uint32_t  i2vRet = I2V_RETURN_OK;

  trange = trange; /* Silence of the warnings. */

  /* Error check input and grab min/max vals if available. */
  if((NULL == configItem) || (NULL == val) || NULL == status) {
      I2V_DBG_LOG(UTIL_ERR_LEVEL,UTIL_NAME,"uint8_t: I2V_RETURN_NULL_PTR failed.\n");
      if(NULL != status) { 
          *status = I2V_RETURN_NULL_PTR;
      }
      return I2V_RETURN_NULL_PTR;
  } 
  if((I2V_CFG_MAX_STR_LEN < strlen(val)) || (0 == strlen(val))){
      *status = I2V_RETURN_RANGE_LIMIT_FAIL;
      I2V_DBG_LOG(UTIL_ERR_LEVEL,UTIL_NAME,"uint8_t: val length failed=%u.\n",strlen(val));
      return I2V_RETURN_RANGE_LIMIT_FAIL;
  }
  if((NULL != min) && (strcmp((const char_t *)min, I2V_CFG_UNDEFINED) != 0)) {
      if((I2V_CFG_MAX_STR_LEN < strlen(min)) || (0 == strlen(min))){
          *status = I2V_RETURN_RANGE_LIMIT_FAIL;
          I2V_DBG_LOG(UTIL_ERR_LEVEL,UTIL_NAME,"uint8_t: min length failed=%u.\n",strlen(min));
          return I2V_RETURN_RANGE_LIMIT_FAIL;
      }

      if(I2V_RETURN_OK != (i2vRet = findEndString(min,&i,status))) {
          return i2vRet;
      }
      errno=0;
      minval = strtol((const char_t *)min, &endptr,0);
      if (   (endptr != &min[i])
          || ((ERANGE == errno) && ((LONG_MIN == minval) || (LONG_MAX == minval)))
          || ((0 != errno) && (0 == minval)) 
          || ((minval < 0) || (UCHAR_MAX < minval))) {
          I2V_DBG_LOG(UTIL_ERR_LEVEL,UTIL_NAME,"uint8_t: min(%s) strtof failed: endptr(0x%x) != min(0x%x) errno=%d.\n", min,endptr,&min[i],errno);
          *status = I2V_RETURN_RANGE_LIMIT_FAIL;
          return I2V_RETURN_RANGE_LIMIT_FAIL;
      }
      have_minmax |= 0x1;
  }
  if ((NULL != max) && (strcmp((const char_t *)max, I2V_CFG_UNDEFINED) != 0)) { 
      if((I2V_CFG_MAX_STR_LEN < strlen(max)) || (0 == strlen(max))){
          *status = I2V_RETURN_RANGE_LIMIT_FAIL;
          I2V_DBG_LOG(UTIL_ERR_LEVEL,UTIL_NAME,"uint8_t: max length failed=%u.\n",strlen(max));
          return I2V_RETURN_RANGE_LIMIT_FAIL;
      }

      if(I2V_RETURN_OK != (i2vRet = findEndString(max,&i,status))) {
          return i2vRet;
      }
      errno=0;
      maxval = strtol((const char_t *)max, &endptr,0);
      if (   (endptr != &max[i])
          || ((ERANGE == errno) && ((LONG_MIN == maxval) || (LONG_MAX == maxval)))
          || ((0 != errno) && (0 == maxval))
          || ((maxval < 0) || (UCHAR_MAX < maxval))){
          I2V_DBG_LOG(UTIL_ERR_LEVEL,UTIL_NAME,"uint8_t: max(%s) strtof failed: endptr(0x%x) != max(0x%x) errno=%d.\n", max,endptr,&max[i],errno);
          *status = I2V_RETURN_RANGE_LIMIT_FAIL;
          return I2V_RETURN_RANGE_LIMIT_FAIL;
      }
      have_minmax |= 0x2;
  }

  /*
   * Convert val and compare against min/max.
   * If it fails then calling app may provide default or appropriate action.
   */

  if(I2V_RETURN_OK != (i2vRet = findEndString(val,&i,status))) {
      return i2vRet;
  }
  errno=0;
  value = strtol((const char_t *)val, &endptr,0);
  if(   (endptr != &val[i]) 
     || ((ERANGE == errno) && ((LONG_MIN == value) || (LONG_MAX == value)))
     || ((0 != errno) && (0 == value)) 
     || ((value < 0) || (UCHAR_MAX < value))) {
      I2V_DBG_LOG(UTIL_ERR_LEVEL,UTIL_NAME,"uint8_t: val(%s)(%ld) strtof failed: endptr(0x%x) != val(0x%x) errno(%s)\n", val,value,endptr,&val[i],strerror(errno));
      *status = I2V_RETURN_RANGE_LIMIT_FAIL;
      return I2V_RETURN_RANGE_LIMIT_FAIL;
  } else {
      #if defined(EXTRA_DEBUG)
      I2V_DBG_LOG(UTIL_ERR_LEVEL,UTIL_NAME,"uint8_t: val(%s)=(%ld): min/max{%ld, %ld}: errno=%d.\n", val, value, minval, maxval,errno);
      #endif
      *cfg = (uint8_t)value; /* Truncate what we give back just in case. */
      if((0x1 & have_minmax) && (value < minval)) { /* If user provided min/max then do range check else we're good. */
          *cfg = (uint8_t)minval;
          I2V_DBG_LOG(UTIL_ERR_LEVEL,UTIL_NAME,"uint8_t: MIN LIMIT, value (%ld) < minval (%ld). value set to minval\n", value, minval);
          *status = I2V_RETURN_MIN_LIMIT_FAIL;
          return I2V_RETURN_MIN_LIMIT_FAIL;
      } 
      if((0x2 & have_minmax) &&  (value > maxval)) {
          *cfg = (uint8_t)maxval;
          I2V_DBG_LOG(UTIL_ERR_LEVEL,UTIL_NAME,"uint8_t: MAX LIMIT, value (%ld) > maxval (%ld). value set to maxval\n", value, maxval);
          *status = I2V_RETURN_MAX_LIMIT_FAIL;
          return I2V_RETURN_MAX_LIMIT_FAIL;
      }
  }
  *status = I2V_RETURN_OK;
  return I2V_RETURN_OK;
}

uint32_t i2vUtilUpdateInt16Value(void *configItem, char_t *val, char_t *min, char_t *max,
      char_t **trange, uint32_t *status, i2vLogWrapper GCC_UNUSED_VAR *logger)
{
  int16_t * cfg = configItem;
  int64_t   value = 0; /* needs to be this big to hold LONG_MAX/LONG_MIN. */
  int64_t   minval = 0;
  int64_t   maxval = 0;
  int32_t   i = 0;
  char_t  * endptr = NULL;
  uint8_t   have_minmax = 0x0; /* bitmask */
  uint32_t  i2vRet = I2V_RETURN_OK;

  trange = trange; /* Silence of the warnings. */

  /* Error check input and grab min/max vals if available. */
  if((NULL == configItem) || (NULL == val) || NULL == status) {
      I2V_DBG_LOG(UTIL_ERR_LEVEL,UTIL_NAME,"int16_t: I2V_RETURN_NULL_PTR failed.\n");
      if(NULL != status) { 
          *status = I2V_RETURN_NULL_PTR;
      }
      return I2V_RETURN_NULL_PTR;
  } 
  if((I2V_CFG_MAX_STR_LEN < strlen(val)) || (0 == strlen(val))){
      *status = I2V_RETURN_RANGE_LIMIT_FAIL;
      I2V_DBG_LOG(UTIL_ERR_LEVEL,UTIL_NAME,"int16_t: val length failed=%u.\n",strlen(val));
      return I2V_RETURN_RANGE_LIMIT_FAIL;
  }
  if((NULL != min) && (strcmp((const char_t *)min, I2V_CFG_UNDEFINED) != 0)) {
      if((I2V_CFG_MAX_STR_LEN < strlen(min)) || (0 == strlen(min))){
          *status = I2V_RETURN_RANGE_LIMIT_FAIL;
          I2V_DBG_LOG(UTIL_ERR_LEVEL,UTIL_NAME,"int16_t: min length failed=%u.\n",strlen(min));
          return I2V_RETURN_RANGE_LIMIT_FAIL;
      }

      if(I2V_RETURN_OK != (i2vRet = findEndString(min,&i,status))) {
          return i2vRet;
      }
      errno=0;
      minval = strtol((const char_t *)min, &endptr,0);
      if (   (endptr != &min[i])
          || ((ERANGE == errno) && ((LONG_MIN == minval) || (LONG_MAX == minval)))
          || ((0 != errno) && (0 == minval)) 
          || ((minval < SHRT_MIN)||(SHRT_MAX < minval))) {
          I2V_DBG_LOG(UTIL_ERR_LEVEL,UTIL_NAME,"int16_t: min(%s) strtof failed: endptr(0x%x) != min(0x%x) errno=%d.\n", min,endptr,&min[i],errno);
          *status = I2V_RETURN_RANGE_LIMIT_FAIL;
          return I2V_RETURN_RANGE_LIMIT_FAIL;
      }
      have_minmax |= 0x1;
  }
  if ((NULL != max) && (strcmp((const char_t *)max, I2V_CFG_UNDEFINED) != 0)) { 
      if((I2V_CFG_MAX_STR_LEN < strlen(max)) || (0 == strlen(max))){
          *status = I2V_RETURN_RANGE_LIMIT_FAIL;
          I2V_DBG_LOG(UTIL_ERR_LEVEL,UTIL_NAME,"int16_t: max length failed=%u.\n",strlen(max));
          return I2V_RETURN_RANGE_LIMIT_FAIL;
      }

      if(I2V_RETURN_OK != (i2vRet = findEndString(max,&i,status))) {
          return i2vRet;
      }
      errno=0;
      maxval = strtol((const char_t *)max, &endptr,0);
      if (   (endptr != &max[i])
          || ((ERANGE == errno) && ((LONG_MIN == maxval) || (LONG_MAX == maxval)))
          || ((0 != errno) && (0 == maxval)) 
          || ((maxval < SHRT_MIN)||(SHRT_MAX < maxval))) {
          I2V_DBG_LOG(UTIL_ERR_LEVEL,UTIL_NAME,"int16_t: max(%s) strtof failed: endptr(0x%x) != max(0x%x) errno=%d.\n", max,endptr,&max[i],errno);
          *status = I2V_RETURN_RANGE_LIMIT_FAIL;
          return I2V_RETURN_RANGE_LIMIT_FAIL;
      }
      have_minmax |= 0x2;
  }

  /*
   * Convert val and compare against min/max.
   * If it fails then calling app may provide default or appropriate action.
   */

  if(I2V_RETURN_OK != (i2vRet = findEndString(val,&i,status))) {
      return i2vRet;
  }
  errno=0;
  value = strtol((const char_t *)val, &endptr,0);
  if(   (endptr != &val[i]) 
     || ((ERANGE == errno) && ((LONG_MIN == value) || (LONG_MAX == value)))
     || ((0 != errno) && (0 == value)) 
     || ((value < SHRT_MIN)||(SHRT_MAX < value))) {
      I2V_DBG_LOG(UTIL_ERR_LEVEL,UTIL_NAME,"int16_t: val(%s)(%d) strtof failed: endptr(0x%x) != val(0x%x) errno(%s)\n", val,value,endptr,&val[i],strerror(errno));
      *status = I2V_RETURN_RANGE_LIMIT_FAIL;
      return I2V_RETURN_RANGE_LIMIT_FAIL;
  } else {
      #if defined(EXTRA_DEBUG)
      I2V_DBG_LOG(UTIL_ERR_LEVEL,UTIL_NAME,"int16_t: val(%s)=(%d): min/max{%d, %d}: errno=%d.\n", val, value, minval, maxval,errno);
      #endif
      *cfg = (int16_t)value; /* Truncate what we give back just in case. */
      if((0x1 & have_minmax) && (value < minval)) { /* If user provided min/max then do range check else we're good. */
          *cfg = (int16_t)minval;
          I2V_DBG_LOG(UTIL_ERR_LEVEL,UTIL_NAME,"int16_t: MIN LIMIT, value (%ld) < minval (%ld). value set to minval\n", value, minval);
          *status = I2V_RETURN_MIN_LIMIT_FAIL;
          return I2V_RETURN_MIN_LIMIT_FAIL;
      } 
      if((0x2 & have_minmax) &&  (value > maxval)) {
          *cfg = (int16_t)maxval;
          I2V_DBG_LOG(UTIL_ERR_LEVEL,UTIL_NAME,"int16_t: MAX LIMIT, value (%ld) > maxval (%ld). value set to maxval\n", value, maxval);
          *status = I2V_RETURN_MAX_LIMIT_FAIL;
          return I2V_RETURN_MAX_LIMIT_FAIL;
      }
  }
  *status = I2V_RETURN_OK;
  return I2V_RETURN_OK;
}

uint32_t i2vUtilUpdateUint16Value(void *configItem, char_t *val, char_t *min, char_t *max,
      char_t **trange, uint32_t *status, i2vLogWrapper GCC_UNUSED_VAR *logger)
{
  uint16_t * cfg = configItem;
  int64_t   value = 0; /* needs to be this big to hold LONG_MAX/LONG_MIN. */
  int64_t   minval = 0;
  int64_t   maxval = 0;
  int32_t   i = 0;
  char_t  * endptr = NULL;
  uint8_t   have_minmax = 0x0; /* bitmask */
  uint32_t  i2vRet = I2V_RETURN_OK;

  trange = trange; /* Silence of the warnings. */

  /* Error check input and grab min/max vals if available. */
  if((NULL == configItem) || (NULL == val) || NULL == status) {
      I2V_DBG_LOG(UTIL_ERR_LEVEL,UTIL_NAME,"uint16_t: I2V_RETURN_NULL_PTR failed.\n");
      if(NULL != status) { 
          *status = I2V_RETURN_NULL_PTR;
      }
      return I2V_RETURN_NULL_PTR;
  } 
  if((I2V_CFG_MAX_STR_LEN < strlen(val)) || (0 == strlen(val))){
      *status = I2V_RETURN_RANGE_LIMIT_FAIL;
      I2V_DBG_LOG(UTIL_ERR_LEVEL,UTIL_NAME,"uint16_t: val length failed=%u.\n",strlen(val));
      return I2V_RETURN_RANGE_LIMIT_FAIL;
  }
  if((NULL != min) && (strcmp((const char_t *)min, I2V_CFG_UNDEFINED) != 0)) {
      if((I2V_CFG_MAX_STR_LEN < strlen(min)) || (0 == strlen(min))){
          *status = I2V_RETURN_RANGE_LIMIT_FAIL;
          I2V_DBG_LOG(UTIL_ERR_LEVEL,UTIL_NAME,"uint16_t: min length failed=%u.\n",strlen(min));
          return I2V_RETURN_RANGE_LIMIT_FAIL;
      }

      if(I2V_RETURN_OK != (i2vRet = findEndString(min,&i,status))) {
          return i2vRet;
      }
      errno=0;
      minval = strtol((const char_t *)min, &endptr,0);
      if (   (endptr != &min[i])
          || ((ERANGE == errno) && ((LONG_MIN == minval) || (LONG_MAX == minval)))
          || ((0 != errno) && (0 == minval)) 
          || ((minval < 0) || (USHRT_MAX < minval))) {
          I2V_DBG_LOG(UTIL_ERR_LEVEL,UTIL_NAME,"uint16_t: min(%s) strtof failed: endptr(0x%x) != min(0x%x) errno=%d.\n", min,endptr,&min[i],errno);
          *status = I2V_RETURN_RANGE_LIMIT_FAIL;
          return I2V_RETURN_RANGE_LIMIT_FAIL;
      }
      have_minmax |= 0x1;
  }
  if ((NULL != max) && (strcmp((const char_t *)max, I2V_CFG_UNDEFINED) != 0)) { 
      if((I2V_CFG_MAX_STR_LEN < strlen(max)) || (0 == strlen(max))){
          *status = I2V_RETURN_RANGE_LIMIT_FAIL;
          I2V_DBG_LOG(UTIL_ERR_LEVEL,UTIL_NAME,"uint16_t: max length failed=%u.\n",strlen(max));
          return I2V_RETURN_RANGE_LIMIT_FAIL;
      }

      if(I2V_RETURN_OK != (i2vRet = findEndString(max,&i,status))) {
          return i2vRet;
      }
      errno=0;
      maxval = strtol((const char_t *)max, &endptr,0);
      if (   (endptr != &max[i])
          || ((ERANGE == errno) && ((LONG_MIN == maxval) || (LONG_MAX == maxval)))
          || ((0 != errno) && (0 == maxval))
          || ((maxval < 0) || (USHRT_MAX < maxval))){
          I2V_DBG_LOG(UTIL_ERR_LEVEL,UTIL_NAME,"uint16_t: max(%s) strtof failed: endptr(0x%x) != max(0x%x) errno=%d.\n", max,endptr,&max[i],errno);
          *status = I2V_RETURN_RANGE_LIMIT_FAIL;
          return I2V_RETURN_RANGE_LIMIT_FAIL;
      }
      have_minmax |= 0x2;
  }

  /*
   * Convert val and compare against min/max.
   * If it fails then calling app may provide default or appropriate action.
   */

  if(I2V_RETURN_OK != (i2vRet = findEndString(val,&i,status))) {
      return i2vRet;
  }
  errno=0;
  value = strtol((const char_t *)val, &endptr,0);
  if(   (endptr != &val[i]) 
     || ((ERANGE == errno) && ((LONG_MIN == value) || (LONG_MAX == value)))
     || ((0 != errno) && (0 == value)) 
     || ((value < 0) || (USHRT_MAX < value))) {
      I2V_DBG_LOG(UTIL_ERR_LEVEL,UTIL_NAME,"uint16_t: val(%s)(%ld) strtof failed: endptr(0x%x) != val(0x%x) errno(%s)\n", val,value,endptr,&val[i],strerror(errno));
      *status = I2V_RETURN_RANGE_LIMIT_FAIL;
      return I2V_RETURN_RANGE_LIMIT_FAIL;
  } else {
      #if defined(EXTRA_DEBUG)
      I2V_DBG_LOG(UTIL_ERR_LEVEL,UTIL_NAME,"uint16_t: val(%s)=(%ld): min/max{%ld, %ld}: errno=%d.\n", val, value, minval, maxval,errno);
      #endif
      *cfg = (uint16_t)value; /* Truncate what we give back just in case. */
      if((0x1 & have_minmax) && (value < minval)) { /* If user provided min/max then do range check else we're good. */
          *cfg = (uint16_t)minval;
          I2V_DBG_LOG(UTIL_ERR_LEVEL,UTIL_NAME,"uint16_t: MIN LIMIT, value (%ld) < minval (%ld). value set to minval\n", value, minval);
          *status = I2V_RETURN_MIN_LIMIT_FAIL;
          return I2V_RETURN_MIN_LIMIT_FAIL;
      } 
      if((0x2 & have_minmax) &&  (value > maxval)) {
          *cfg = (uint16_t)maxval;
          I2V_DBG_LOG(UTIL_ERR_LEVEL,UTIL_NAME,"uint16_t: MAX LIMIT, value (%ld) > maxval (%ld). value set to maxval\n", value, maxval);
          *status = I2V_RETURN_MAX_LIMIT_FAIL;
          return I2V_RETURN_MAX_LIMIT_FAIL;
      }
  }
  *status = I2V_RETURN_OK;
  return I2V_RETURN_OK;
}

uint32_t i2vUtilUpdateInt32Value(void *configItem, char_t *val, char_t *min, char_t *max, 
  char_t **trange, uint32_t *status, i2vLogWrapper GCC_UNUSED_VAR *logger)
{
  int32_t * cfg = configItem;
  int64_t   value = 0; /* needs to be this big to hold LONG_MAX/LONG_MIN. */
  int64_t   minval = 0;
  int64_t   maxval = 0;
  int32_t   i = 0;
  char_t  * endptr = NULL;
  uint8_t   have_minmax = 0x0; /* bitmask */
  uint32_t  i2vRet = I2V_RETURN_OK;

  trange = trange; /* Silence of the warnings. */

  /* Error check input and grab min/max vals if available. */
  if((NULL == configItem) || (NULL == val) || NULL == status) {
      I2V_DBG_LOG(UTIL_ERR_LEVEL,UTIL_NAME,"int32_t: I2V_RETURN_NULL_PTR failed.\n");
      if(NULL != status) { 
          *status = I2V_RETURN_NULL_PTR;
      }
      return I2V_RETURN_NULL_PTR;
  } 
  if((I2V_CFG_MAX_STR_LEN < strlen(val)) || (0 == strlen(val))){
      *status = I2V_RETURN_RANGE_LIMIT_FAIL;
      I2V_DBG_LOG(UTIL_ERR_LEVEL,UTIL_NAME,"int32_t: val length failed=%u.\n",strlen(val));
      return I2V_RETURN_RANGE_LIMIT_FAIL;
  }
  if((NULL != min) && (strcmp((const char_t *)min, I2V_CFG_UNDEFINED) != 0)) {
      if((I2V_CFG_MAX_STR_LEN < strlen(min)) || (0 == strlen(min))){
          *status = I2V_RETURN_RANGE_LIMIT_FAIL;
          I2V_DBG_LOG(UTIL_ERR_LEVEL,UTIL_NAME,"int32_t: min length failed=%u.\n",strlen(min));
          return I2V_RETURN_RANGE_LIMIT_FAIL;
      }

      if(I2V_RETURN_OK != (i2vRet = findEndString(min,&i,status))) {
          return i2vRet;
      }
      errno=0;
      minval = strtol((const char_t *)min, &endptr,0);
      if (   (endptr != &min[i])
          || ((ERANGE == errno) && ((LONG_MIN == minval) || (LONG_MAX == minval)))
          || ((0 != errno) && (0 == minval)) 
          || ((minval < INT_MIN)||(INT_MAX < minval))) {
          I2V_DBG_LOG(UTIL_ERR_LEVEL,UTIL_NAME,"int32_t: min(%s) strtof failed: endptr(0x%x) != min(0x%x) errno=%d.\n", min,endptr,&min[i],errno);
          *status = I2V_RETURN_RANGE_LIMIT_FAIL;
          return I2V_RETURN_RANGE_LIMIT_FAIL;
      }
      have_minmax |= 0x1;
  }
  if ((NULL != max) && (strcmp((const char_t *)max, I2V_CFG_UNDEFINED) != 0)) { 
      if((I2V_CFG_MAX_STR_LEN < strlen(max)) || (0 == strlen(max))){
          *status = I2V_RETURN_RANGE_LIMIT_FAIL;
          I2V_DBG_LOG(UTIL_ERR_LEVEL,UTIL_NAME,"int32_t: max length failed=%u.\n",strlen(max));
          return I2V_RETURN_RANGE_LIMIT_FAIL;
      }

      if(I2V_RETURN_OK != (i2vRet = findEndString(max,&i,status))) {
          return i2vRet;
      }
      errno=0;
      maxval = strtol((const char_t *)max, &endptr,0);
      if (   (endptr != &max[i])
          || ((ERANGE == errno) && ((LONG_MIN == maxval) || (LONG_MAX == maxval)))
          || ((0 != errno) && (0 == maxval)) 
          || ((maxval < INT_MIN)||(INT_MAX < maxval))) {
          I2V_DBG_LOG(UTIL_ERR_LEVEL,UTIL_NAME,"int32_t: max(%s) strtof failed: endptr(0x%x) != max(0x%x) errno=%d.\n", max,endptr,&max[i],errno);
          *status = I2V_RETURN_RANGE_LIMIT_FAIL;
          return I2V_RETURN_RANGE_LIMIT_FAIL;
      }
      have_minmax |= 0x2;
  }

  /*
   * Convert val and compare against min/max.
   * If it fails then calling app may provide default or appropriate action.
   */

  if(I2V_RETURN_OK != (i2vRet = findEndString(val,&i,status))) {
      return i2vRet;
  }
  errno=0;
  value = strtol((const char_t *)val, &endptr,0);
  if(   (endptr != &val[i]) 
     || ((ERANGE == errno) && ((LONG_MIN == value) || (LONG_MAX == value)))
     || ((0 != errno) && (0 == value)) 
     || ((value < INT_MIN)||(INT_MAX < value))) {
      I2V_DBG_LOG(UTIL_ERR_LEVEL,UTIL_NAME,"int32_t: val(%s)(%d) strtof failed: endptr(0x%x) != val(0x%x) errno(%s)\n", val,value,endptr,&val[i],strerror(errno));
      *status = I2V_RETURN_RANGE_LIMIT_FAIL;
      return I2V_RETURN_RANGE_LIMIT_FAIL;
  } else {
      #if defined(EXTRA_DEBUG)
      I2V_DBG_LOG(UTIL_ERR_LEVEL,UTIL_NAME,"int32_t: val(%s)=(%d): min/max{%d, %d}: errno=%d.\n", val, value, minval, maxval,errno);
      #endif
      *cfg = (int32_t)value; /* Truncate what we give back just in case. */
      if((0x1 & have_minmax) && (value < minval)) { /* If user provided min/max then do range check else we're good. */
          *cfg = (int32_t)minval;
          I2V_DBG_LOG(UTIL_ERR_LEVEL,UTIL_NAME,"int32_t: MIN LIMIT, value (%ld) < minval (%ld). value set to minval\n", value, minval);
          *status = I2V_RETURN_MIN_LIMIT_FAIL;
          return I2V_RETURN_MIN_LIMIT_FAIL;
      } 
      if((0x2 & have_minmax) &&  (value > maxval)) {
          *cfg = (int32_t)maxval;
          I2V_DBG_LOG(UTIL_ERR_LEVEL,UTIL_NAME,"int32_t: MAX LIMIT, value (%ld) > maxval (%ld). value set to maxval\n", value, maxval);
          *status = I2V_RETURN_MAX_LIMIT_FAIL;
          return I2V_RETURN_MAX_LIMIT_FAIL;
      }
  }
  *status = I2V_RETURN_OK;
  return I2V_RETURN_OK;
}


uint32_t i2vUtilUpdateUint32Value(void *configItem, char_t *val, char_t *min, char_t *max,
      char_t **trange, uint32_t *status, i2vLogWrapper GCC_UNUSED_VAR *logger)
{
  uint32_t * cfg = configItem;
  int64_t   value = 0; /* needs to be this big to hold LONG_MAX/LONG_MIN. */
  int64_t   minval = 0;
  int64_t   maxval = 0;
  int32_t   i = 0;
  char_t  * endptr = NULL;
  uint8_t   have_minmax = 0x0; /* bitmask */
  uint32_t  i2vRet = I2V_RETURN_OK;

  trange = trange; /* Silence of the warnings. */

  /* Error check input and grab min/max vals if available. */
  if((NULL == configItem) || (NULL == val) || NULL == status) {
      I2V_DBG_LOG(UTIL_ERR_LEVEL,UTIL_NAME,"uint32_t: I2V_RETURN_NULL_PTR failed.\n");
      if(NULL != status) { 
          *status = I2V_RETURN_NULL_PTR;
      }
      return I2V_RETURN_NULL_PTR;
  } 
  if((I2V_CFG_MAX_STR_LEN < strlen(val)) || (0 == strlen(val))){
      *status = I2V_RETURN_RANGE_LIMIT_FAIL;
      I2V_DBG_LOG(UTIL_ERR_LEVEL,UTIL_NAME,"uint32_t: val length failed=%u.\n",strlen(val));
      return I2V_RETURN_RANGE_LIMIT_FAIL;
  }
  if((NULL != min) && (strcmp((const char_t *)min, I2V_CFG_UNDEFINED) != 0)) {
      if((I2V_CFG_MAX_STR_LEN < strlen(min)) || (0 == strlen(min))){
          *status = I2V_RETURN_RANGE_LIMIT_FAIL;
          I2V_DBG_LOG(UTIL_ERR_LEVEL,UTIL_NAME,"uint32_t: min length failed=%u.\n",strlen(min));
          return I2V_RETURN_RANGE_LIMIT_FAIL;
      }

      if(I2V_RETURN_OK != (i2vRet = findEndString(min,&i,status))) {
          return i2vRet;
      }
      errno=0;
      minval = strtol((const char_t *)min, &endptr,0);
      if (   (endptr != &min[i])
          || ((ERANGE == errno) && ((LONG_MIN == minval) || (LONG_MAX == minval)))
          || ((0 != errno) && (0 == minval)) 
          || ((minval < 0) || (UINT_MAX < minval))) {
          I2V_DBG_LOG(UTIL_ERR_LEVEL,UTIL_NAME,"uint32_t: min(%s) strtol failed: endptr(0x%x) != min(0x%x) errno=%d.\n", min,endptr,&min[i],errno);
          *status = I2V_RETURN_RANGE_LIMIT_FAIL;
          return I2V_RETURN_RANGE_LIMIT_FAIL;
      }
      have_minmax |= 0x1;
  }
  if ((NULL != max) && (strcmp((const char_t *)max, I2V_CFG_UNDEFINED) != 0)) { 
      if((I2V_CFG_MAX_STR_LEN < strlen(max)) || (0 == strlen(max))){
          *status = I2V_RETURN_RANGE_LIMIT_FAIL;
          I2V_DBG_LOG(UTIL_ERR_LEVEL,UTIL_NAME,"uint32_t: max length failed=%u.\n",strlen(max));
          return I2V_RETURN_RANGE_LIMIT_FAIL;
      }

      if(I2V_RETURN_OK != (i2vRet = findEndString(max,&i,status))) {
          return i2vRet;
      }
      errno=0;
      maxval = strtol((const char_t *)max, &endptr,0);
      if (   (endptr != &max[i])
          || ((ERANGE == errno) && ((LONG_MIN == maxval) || (LONG_MAX == maxval)))
          || ((0 != errno) && (0 == maxval))
          || ((maxval < 0) || (UINT_MAX < maxval))){
          I2V_DBG_LOG(UTIL_ERR_LEVEL,UTIL_NAME,"uint32_t: max(%s) strtol failed: endptr(0x%x) != max(0x%x) errno=%d.\n", max,endptr,&max[i],errno);
          *status = I2V_RETURN_RANGE_LIMIT_FAIL;
          return I2V_RETURN_RANGE_LIMIT_FAIL;
      }
      have_minmax |= 0x2;
  }

  /*
   * Convert val and compare against min/max.
   * If it fails then calling app may provide default or appropriate action.
   */

  if(I2V_RETURN_OK != (i2vRet = findEndString(val,&i,status))) {
      return i2vRet;
  }
  errno=0;
  value = strtol((const char_t *)val, &endptr,0);
  if(   (endptr != &val[i]) 
     || ((ERANGE == errno) && ((LONG_MIN == value) || (LONG_MAX == value)))
     || ((0 != errno) && (0 == value)) 
     || ((value < 0) || (UINT_MAX < value))) {
      I2V_DBG_LOG(UTIL_ERR_LEVEL,UTIL_NAME,"uint32_t: val(%s)(%ld) strtol failed: endptr(0x%x) != val(0x%x) errno(%s)\n", val,value,endptr,&val[i],strerror(errno));
      *status = I2V_RETURN_RANGE_LIMIT_FAIL;
      return I2V_RETURN_RANGE_LIMIT_FAIL;
  } else {
      #if defined(EXTRA_DEBUG)
      I2V_DBG_LOG(UTIL_ERR_LEVEL,UTIL_NAME,"uint32_t: val(%s)=(%ld): min/max{%ld, %ld}: errno=%d.\n", val, value, minval, maxval,errno);
      #endif
      *cfg = (uint32_t)value; /* Truncate what we give back just in case. */
      if((0x1 & have_minmax) && (value < minval)) { /* If user provided min/max then do range check else we're good. */
          *cfg = (uint32_t)minval;
          I2V_DBG_LOG(UTIL_ERR_LEVEL,UTIL_NAME,"uint32_t: MIN LIMIT, value (%ld) < minval (%ld). value set to minval\n", value, minval);
          *status = I2V_RETURN_MIN_LIMIT_FAIL;
          return I2V_RETURN_MIN_LIMIT_FAIL;
      } 
      if((0x2 & have_minmax) &&  (value > maxval)) {
          *cfg = (uint32_t)maxval;
          I2V_DBG_LOG(UTIL_ERR_LEVEL,UTIL_NAME,"uint32_t: MAX LIMIT, value (%ld) > maxval (%ld). value set to maxval\n", value, maxval);
          *status = I2V_RETURN_MAX_LIMIT_FAIL;
          return I2V_RETURN_MAX_LIMIT_FAIL;
      }
  }
  *status = I2V_RETURN_OK;
  return I2V_RETURN_OK;
}

/* for float64_t strtod() ensures within data type limits. ** trange not needed. */
uint32_t i2vUtilUpdateDoubleValue(void *configItem, char_t *val, char_t *min, char_t *max, 
      char_t **trange, uint32_t *status, i2vLogWrapper GCC_UNUSED_VAR *logger)
{
  float64_t * cfg = configItem;
  float64_t   value = 0.0f;
  float64_t   minval = 0.0f, maxval = 0.0f;
  int32_t     i = 0;
  char_t    * endptr = NULL;
  uint8_t     have_minmax = 0x0; /* bitmask */
  uint32_t    i2vRet = I2V_RETURN_OK;

  trange = trange; /* Silence of the warnings. */

  /* Error check input and grab min/max vals if available. */
  if((NULL == configItem) || (NULL == val) || NULL == status) {
      I2V_DBG_LOG(UTIL_ERR_LEVEL,UTIL_NAME,"float64_t: I2V_RETURN_NULL_PTR failed.\n");
      if(NULL != status) { 
          *status = I2V_RETURN_NULL_PTR;
      }
      return I2V_RETURN_NULL_PTR;
  } 
  if((I2V_CFG_MAX_STR_LEN < strlen(val)) || (0 == strlen(val))){
      *status = I2V_RETURN_RANGE_LIMIT_FAIL;
      I2V_DBG_LOG(UTIL_ERR_LEVEL,UTIL_NAME,"float64_t: val length failed=%u.\n",strlen(val));
      return I2V_RETURN_RANGE_LIMIT_FAIL;
  }
  if((NULL != min) && (strcmp((const char_t *)min, I2V_CFG_UNDEFINED) != 0)) {
      if((I2V_CFG_MAX_STR_LEN < strlen(min)) || (0 == strlen(min))){
          *status = I2V_RETURN_RANGE_LIMIT_FAIL;
          I2V_DBG_LOG(UTIL_ERR_LEVEL,UTIL_NAME,"float64_t: min length failed=%u.\n",strlen(min));
          return I2V_RETURN_RANGE_LIMIT_FAIL;
      }

      if(I2V_RETURN_OK != (i2vRet = findEndString(min,&i,status))) {
          return i2vRet;
      }
      errno=0;
      minval = strtod((const char_t *)min, &endptr);
      if (   (endptr != &min[i])
          || ((ERANGE == errno) && ((-HUGE_VAL == minval) || (HUGE_VAL == minval)))
          || ((0 != errno) && (0.0 == minval)) ) {
          I2V_DBG_LOG(UTIL_ERR_LEVEL,UTIL_NAME,"float64_t: min(%s) strtof failed: endptr(0x%x) != min(0x%x) errno=%d.\n", min,endptr,&min[i],errno);
          *status = I2V_RETURN_RANGE_LIMIT_FAIL;
          return I2V_RETURN_RANGE_LIMIT_FAIL;
      }
      have_minmax |= 0x1;
  }
  if ((NULL != max) && (strcmp((const char_t *)max, I2V_CFG_UNDEFINED) != 0)) { 
      if((I2V_CFG_MAX_STR_LEN < strlen(max)) || (0 == strlen(max))){
          *status = I2V_RETURN_RANGE_LIMIT_FAIL;
          I2V_DBG_LOG(UTIL_ERR_LEVEL,UTIL_NAME,"float64_t: max length failed=%u.\n",strlen(max));
          return I2V_RETURN_RANGE_LIMIT_FAIL;
      }

      if(I2V_RETURN_OK != (i2vRet = findEndString(max,&i,status))) {
          return i2vRet;
      }
      errno=0;
      maxval = strtod((const char_t *)max, &endptr);
      if (   (endptr != &max[i])
          || ((ERANGE == errno) && ((-HUGE_VAL == maxval) || (HUGE_VAL == maxval)))
          || ((0 != errno) && (0.0 == maxval)) ) {
          I2V_DBG_LOG(UTIL_ERR_LEVEL,UTIL_NAME,"float64_t: max(%s) strtof failed: endptr(0x%x) != max(0x%x) errno=%d.\n", max,endptr,&max[i],errno);
          *status = I2V_RETURN_RANGE_LIMIT_FAIL;
          return I2V_RETURN_RANGE_LIMIT_FAIL;
      }
      have_minmax |= 0x2;
  }

  /*
   * Convert val and compare against min/max.
   * If it fails then calling app may provide default or appropriate action.
   */

  /* Find end point of strtof() to compare result. */
  if(I2V_RETURN_OK != (i2vRet = findEndString(val,&i,status))) {
      return i2vRet;
  }
  errno=0;
  value = strtod((const char_t *)val, &endptr);
  if(   (endptr != &val[i]) 
     || ((ERANGE == errno) && ((-HUGE_VAL == value) || (HUGE_VAL == value)))
     || ((0 != errno) && (0.0 == value)) ) {
      I2V_DBG_LOG(UTIL_ERR_LEVEL,UTIL_NAME,"float64_t: val(%s)(%lf) strtof failed: endptr(0x%x) != val(0x%x) errno(%s)\n", val,value,endptr,&val[i],strerror(errno));
      *status = I2V_RETURN_RANGE_LIMIT_FAIL;
      return I2V_RETURN_RANGE_LIMIT_FAIL;
  } else {
      #if defined(EXTRA_DEBUG)
      I2V_DBG_LOG(UTIL_ERR_LEVEL,UTIL_NAME,"float64_t: val(%s)=(%lf): min/max{%lf, %lf}: errno=%d.\n", val, value, minval, maxval,errno);
      #endif
      *cfg = value;
      if((0x1 & have_minmax) && (value < minval)) { /* If user provided min/max then do range check else we're good. */
          *cfg = minval;
          I2V_DBG_LOG(UTIL_ERR_LEVEL,UTIL_NAME,"float64_t: MIN LIMIT, value (%lf) < minval (%lf). value set to minval\n", value, minval);
          *status = I2V_RETURN_MIN_LIMIT_FAIL;
          return I2V_RETURN_MIN_LIMIT_FAIL;
      } 
      if((0x2 & have_minmax) &&  (value > maxval)) {
          *cfg = maxval;
          I2V_DBG_LOG(UTIL_ERR_LEVEL,UTIL_NAME,"float64_t: MAX LIMIT, value (%lf) > maxval (%lf). value set to maxval\n", value, maxval);
          *status = I2V_RETURN_MAX_LIMIT_FAIL;
          return I2V_RETURN_MAX_LIMIT_FAIL;
      }
  }

  *status = I2V_RETURN_OK;
  return I2V_RETURN_OK;

}
/* for float32_t strtof() ensures within data type limits. ** trange not needed. */
uint32_t i2vUtilUpdateFloatValue(void *configItem, char_t *val, char_t *min, char_t *max,
     char_t  **trange, uint32_t *status, i2vLogWrapper GCC_UNUSED_VAR *logger)
{
  float32_t * cfg = configItem;
  float32_t   value = 0.0f;
  float32_t   minval = 0.0f, maxval = 0.0f;
  int32_t     i = 0;
  char_t    * endptr = NULL;
  uint8_t     have_minmax = 0x0; /* bitmask */
  uint32_t    i2vRet = I2V_RETURN_OK;

  trange = trange; /* Silence of the warnings. */

  /* Error check input and grab min/max vals if available. */
  if((NULL == configItem) || (NULL == val) || NULL == status) {
      I2V_DBG_LOG(UTIL_ERR_LEVEL,UTIL_NAME,"float32_t: I2V_RETURN_NULL_PTR failed.\n");
      if(NULL != status) { 
          *status = I2V_RETURN_NULL_PTR;
      }
      return I2V_RETURN_NULL_PTR;
  } 
  if((I2V_CFG_MAX_STR_LEN < strlen(val)) || (0 == strlen(val))){
      *status = I2V_RETURN_RANGE_LIMIT_FAIL;
      I2V_DBG_LOG(UTIL_ERR_LEVEL,UTIL_NAME,"float32_t: val length failed=%u.\n",strlen(val));
      return I2V_RETURN_RANGE_LIMIT_FAIL;
  }
  if((NULL != min) && (strcmp((const char_t *)min, I2V_CFG_UNDEFINED) != 0)) {
      if((I2V_CFG_MAX_STR_LEN < strlen(min)) || (0 == strlen(min))){
          *status = I2V_RETURN_RANGE_LIMIT_FAIL;
          I2V_DBG_LOG(UTIL_ERR_LEVEL,UTIL_NAME,"float32_t: min length failed=%u.\n",strlen(min));
          return I2V_RETURN_RANGE_LIMIT_FAIL;
      }

      if(I2V_RETURN_OK != (i2vRet = findEndString(min,&i,status))) {
          return i2vRet;
      }
      errno=0;
      minval = strtof((const char_t *)min, &endptr);
      if (   (endptr != &min[i])
          || ((ERANGE == errno) && ((-HUGE_VALF == minval) || (HUGE_VALF == minval)))
          || ((0 != errno) && (0.0f == minval)) ) {
          I2V_DBG_LOG(UTIL_ERR_LEVEL,UTIL_NAME,"float32_t: min(%s) strtof failed: endptr(0x%x) != min(0x%x) errno=%d.\n", min,endptr,&min[i],errno);
          *status = I2V_RETURN_RANGE_LIMIT_FAIL;
          return I2V_RETURN_RANGE_LIMIT_FAIL;
      }
      have_minmax |= 0x1;
  }
  if ((NULL != max) && (strcmp((const char_t *)max, I2V_CFG_UNDEFINED) != 0)) { 
      if((I2V_CFG_MAX_STR_LEN < strlen(max)) || (0 == strlen(max))){
          *status = I2V_RETURN_RANGE_LIMIT_FAIL;
          I2V_DBG_LOG(UTIL_ERR_LEVEL,UTIL_NAME,"float32_t: max length failed=%u.\n",strlen(max));
          return I2V_RETURN_RANGE_LIMIT_FAIL;
      }

      if(I2V_RETURN_OK != (i2vRet = findEndString(max,&i,status))) {
          return i2vRet;
      }
      errno=0;
      maxval = strtof((const char_t *)max, &endptr);
      if (   (endptr != &max[i])
          || ((ERANGE == errno) && ((-HUGE_VALF == maxval) || (HUGE_VALF == maxval)))
          || ((0 != errno) && (0.0f == maxval)) ) {
          I2V_DBG_LOG(UTIL_ERR_LEVEL,UTIL_NAME,"float32_t: max(%s) strtof failed: endptr(0x%x) != max(0x%x) errno=%d.\n", max,endptr,&max[i],errno);
          *status = I2V_RETURN_RANGE_LIMIT_FAIL;
          return I2V_RETURN_RANGE_LIMIT_FAIL;
      }
      have_minmax |= 0x2;
  }

  /*
   * Convert val and compare against min/max.
   * If it fails then calling app may provide default or appropriate action.
   */

  if(I2V_RETURN_OK != (i2vRet = findEndString(val,&i,status))) {
      return i2vRet;
  }
  errno=0;
  value = strtof((const char_t *)val, &endptr);
  if(   (endptr != &val[i]) 
     || ((ERANGE == errno) && ((-HUGE_VALF == value) || (HUGE_VALF == value)))
     || ((0 != errno) && (0.0f == value)) ) {
      I2V_DBG_LOG(UTIL_ERR_LEVEL,UTIL_NAME,"float32_t: val(%s)(%f) strtof failed: endptr(0x%x) != val(0x%x) errno(%s)\n", val,value,endptr,&val[i],strerror(errno));
      *status = I2V_RETURN_RANGE_LIMIT_FAIL;
      return I2V_RETURN_RANGE_LIMIT_FAIL;
  } else {
      #if defined(EXTRA_DEBUG)
      I2V_DBG_LOG(UTIL_ERR_LEVEL,UTIL_NAME,"float32_t: val(%s)=(%f): min/max{%f, %f}: errno=%d.\n", val, value, minval, maxval,errno);
      #endif
      *cfg = value;
      if((0x1 & have_minmax) && (value < minval)) { /* If user provided min/max then do range check else we're good. */
          *cfg = minval;
          I2V_DBG_LOG(UTIL_ERR_LEVEL,UTIL_NAME,"float32_t: MIN LIMIT, value (%f) < minval (%f). value set to minval\n", value, minval);
          *status = I2V_RETURN_MIN_LIMIT_FAIL;
          return I2V_RETURN_MIN_LIMIT_FAIL;
      } 
      if((0x2 & have_minmax) &&  (value > maxval)) {
          *cfg = maxval;
          I2V_DBG_LOG(UTIL_ERR_LEVEL,UTIL_NAME,"float32_t: MAX LIMIT, value (%f) > maxval (%f). value set to maxval\n", value, maxval);
          *status = I2V_RETURN_MAX_LIMIT_FAIL;
          return I2V_RETURN_MAX_LIMIT_FAIL;
      }
  }
  *status = I2V_RETURN_OK; 
  return I2V_RETURN_OK;
}

uint32_t i2vUtilUpdatePsidValue(void *configItem, char_t *val, char_t *min, char_t *max,
      GCC_UNUSED_VAR char_t **trange, uint32_t *status, i2vLogWrapper GCC_UNUSED_VAR *logger)
{
  uint32_t i2vRet = I2V_RETURN_OK;
  if(I2V_RETURN_OK == (i2vRet = i2vUtilUpdateUint32Value(configItem,val,min,max,NULL,status,NULL))){
      if(I2V_RETURN_OK != i2vCheckPSID(val)) {
          i2vRet = I2V_RETURN_RANGE_LIMIT_FAIL;
          *status = I2V_RETURN_RANGE_LIMIT_FAIL;
      }
  }
  return i2vRet;
}
/* config file parsing function - override flag to bypass returning failure
 * if a config item is expected but not found in file 
 */
#define FILE_PARSE_CONF_MAX_BUF  (300)

static     char_t   buf[FILE_PARSE_CONF_MAX_BUF];
static     char_t   dummy[FILE_PARSE_CONF_MAX_BUF];
static     char_t   val[FILE_PARSE_CONF_MAX_BUF];
static     char_t   min[FILE_PARSE_CONF_MAX_BUF];
static     char_t   max[FILE_PARSE_CONF_MAX_BUF]; 

uint32_t i2vUtilParseConfFile(FILE *f, cfgItemsTypeT *cfgItems, uint32_t numCfgItems, bool_t overrideFlag, i2vLogWrapper *logger)
{
    char_t   * ptr          = NULL;
    uint32_t   j            = 0;
    uint32_t   i            = 0; 
    uint32_t   parserStatus = I2V_RETURN_OK;
    int32_t    retVal       = I2V_RETURN_OK;
#if defined(EXTRA_DEBUG)
    bool_t     matchFound   = WFALSE;
#endif
    memset(buf, '\0', sizeof(buf));

    if (NULL == f) {
        I2V_DBG_LOG(UTIL_ERR_LEVEL,UTIL_NAME,"I2V Critical Error: Null file pointer\n");
        return I2V_RETURN_NULL_PTR;
    }

    while (fgets(buf, FILE_PARSE_CONF_MAX_BUF, f)) {

        memset(dummy, '\0', sizeof(dummy));
        memset(val,   '\0', sizeof(val));
        memset(min,   '\0', sizeof(min));
        memset(max,   '\0', sizeof(max));

        /* Identify and remove comment sections */
        if ((ptr = strrchr(buf, '#')) != NULL) *ptr = '\0';
        memset(dummy, '\0', sizeof(dummy));
        for(i=0,j=0;(i < FILE_PARSE_CONF_MAX_BUF) && ('\0' != buf[i]); i++) {

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
        if(0 == j) {
            memset(buf,  '\0', sizeof(buf));
            continue;
        }

        for(i=0;    (i<FILE_PARSE_CONF_MAX_BUF) 
                 && ('\0' != dummy[i])
                 && ('\n' != dummy[i]) ;i++) {
            buf[i] = dummy[i];
        }

        if(i < FILE_PARSE_CONF_MAX_BUF)
            buf[i] = '\0';
        else
            buf[FILE_PARSE_CONF_MAX_BUF-1] = '\0';

        /* Blank/Comment lines */
        if ((strlen(buf) == 0) || (buf[0] == '#')) {
            memset(buf,  '\0', sizeof(buf));
            continue;
        }

        /* Identify value lines */
        if ((ptr = strrchr(buf, '=')) == NULL) {
            I2V_DBG_LOG(UTIL_ERR_LEVEL,UTIL_NAME,"(%d)  Rejected - No assignment: %s\n",getpid(),buf);
            memset(buf,  '\0', sizeof(buf));
            continue;
        }

        /* Replace '=' with NULL; point ptr to value in string and copy into 'val' */
        //*ptr = '\0';

        strncpy(val, ptr+1,sizeof(val));

        if(I2V_CFG_MAX_STR_LEN <= strlen(val)) {
            I2V_DBG_LOG(UTIL_ERR_LEVEL,UTIL_NAME,"I2VU:********val is too long = (%lu):**************\n",strlen(val));
            fflush(stdout);
            memset(buf,  '\0', sizeof(buf));
            continue;
        }
        *ptr = '\0';
        /* Read Range Minimum Value */
        if ((ptr = strrchr(val, ';')) != NULL) {
#if defined(EXTRA_DEBUG)
            I2V_DBG_LOG(UTIL_ERR_LEVEL,UTIL_NAME,"[");
#endif
            *ptr = '\0';
            strncpy(min, ptr+1,sizeof(min));

            /* Read Range Maximum Value */
            if ((ptr = strrchr(min, ',')) != NULL) {
#if defined(EXTRA_DEBUG)
                I2V_DBG_LOG(UTIL_ERR_LEVEL,UTIL_NAME,"]");
#endif
                *ptr = '\0';
                strncpy(max, ptr+1,sizeof(max)); //how do we know max hasn't blown it?

            } else {
#if defined(EXTRA_DEBUG)
                I2V_DBG_LOG(UTIL_ERR_LEVEL,UTIL_NAME,"}");
#endif
                strncpy(min, UNDEFINED,sizeof(min));
                strncpy(max, UNDEFINED,sizeof(max));
            }
        } else {
#if defined(EXTRA_DEBUG)
            I2V_DBG_LOG(UTIL_ERR_LEVEL,UTIL_NAME,"{");
#endif
            strncpy(min, UNDEFINED,sizeof(min));
            strncpy(max, UNDEFINED,sizeof(max));
        }

        /* Identify Configuration Item(s) */
#if defined(EXTRA_DEBUG)
        I2V_DBG_LOG(UTIL_ERR_LEVEL,UTIL_NAME,":Config(errno=%d):  %s: val: %s  min: %s  max: %s \n", errno, buf, val, min, max); 
        matchFound = WFALSE;
#endif
        for (i = 0; i < numCfgItems; i++) {
            /* custom functions may have tags that don't match length of parsed buf */
            if (((strncmp(buf, cfgItems[i].tag, strlen(cfgItems[i].tag)) == 0) && (strlen(buf) == strlen(cfgItems[i].tag))) 
             || ((strncmp(buf, cfgItems[i].tag, strlen(cfgItems[i].tag)) == 0) && (cfgItems[i].customUpdateFxn != NULL))) {
                if (cfgItems[i].customUpdateFxn == NULL) {
                    (*cfgItems[i].updateFunction)(cfgItems[i].itemToUpdate, val, min, max, NULL, &parserStatus, logger);
                } else {
#if defined(EXTRA_DEBUG)
                    I2V_DBG_LOG(UTIL_ERR_LEVEL,UTIL_NAME,"\n");  /* custom functions may not print out text to log terminate open ended log msg printed above */
#endif
                    (*cfgItems[i].customUpdateFxn)(cfgItems[i].tag, buf, val, cfgItems[i].itemToUpdate, &parserStatus);
                }
#if defined(EXTRA_DEBUG)
                matchFound = WTRUE;
#endif
                if( parserStatus == I2V_RETURN_OK ) {
                    cfgItems[i].state &= ~UNINIT;
                    cfgItems[i].state |= INIT;
                } else {
                    I2V_DBG_LOG(UTIL_ERR_LEVEL,UTIL_NAME,"Limit error on '%s'. Examine Debug Log for details\n",  cfgItems[i].tag );
                    cfgItems[i].state &= ~UNINIT;
                    cfgItems[i].state |= BAD_VAL;
                }
                break;
            }
        }
#if defined(EXTRA_DEBUG)
        if (matchFound == WFALSE) { /* Nice to know, but harmless. After a few stop. */
          static int32_t limit_dbg_output = 0;
            if(limit_dbg_output < 1) {
                I2V_DBG_LOG(UTIL_ERR_LEVEL,UTIL_NAME,"Rejected - Configuration item %s unidentified\n",buf);
                limit_dbg_output++;
            }
        }
#endif
        memset(buf,'\0', sizeof(buf));
    } /* while */

    if (!overrideFlag)  {
        for (i = 0; i < numCfgItems; i++) {
            if ((ITEM_VALID|UNINIT) == cfgItems[i].state) {
                I2V_DBG_LOG(UTIL_ERR_LEVEL,UTIL_NAME,"(%d)Configuration values for '%s' not found.\n", getpid(),cfgItems[i].tag);
                retVal = I2V_RETURN_INVALID_CONFIG;
            }
            if ((ITEM_VALID|BAD_VAL) == cfgItems[i].state) {
                I2V_DBG_LOG(UTIL_ERR_LEVEL,UTIL_NAME,"(%d)Configuration values for '%s' bad. rejected.\n", getpid(),cfgItems[i].tag);
                retVal = I2V_RETURN_INVALID_CONFIG;
            }
        }
    }


#if defined(MY_UNIT_TEST)
  if(1 == get_stub_signal(Signal_i2vUtilParseConfFile)) {
      retVal = I2V_RETURN_INVALID_CONFIG;
      clear_stub_signal(Signal_i2vUtilParseConfFile);
  }
#endif
    return retVal;
}

/* add extra debug for printfs if desiring logging */
int i2vUtilParseCfgArrayIndex(uint8_t *configstr, int32_t *index)
{
    const char_t *regex = "[0-9]+";
    const char_t *input = (char_t *) configstr;
    regex_t reg;
    regmatch_t pmatch[1];
#define REG_STR_MAX (10)
#define REG_STR_ARR_MAX (5)
    char_t str[REG_STR_MAX];
    int32_t ret = 0;

    if ( (configstr == NULL) || (index == NULL) ) {
        return -1;
    }

    if ( regcomp(&reg, regex, REG_EXTENDED | REG_ICASE) != 0 ) {
        I2V_DBG_LOG(UTIL_ERR_LEVEL,UTIL_NAME,"(%d)unsuccessful regex compilation. input : %s\n", getpid(),input);
        return -1;
    }
    if ( regexec(&reg, input, 1, pmatch, 0) != 0 ) {
        I2V_DBG_LOG(UTIL_ERR_LEVEL,UTIL_NAME,"(%d)can't find array indexes in %s\n", getpid(),input);
        ret = -1;
        goto out;
    }
    if ( (pmatch[0].rm_eo - pmatch[0].rm_so) > REG_STR_ARR_MAX - 1 ) {
        I2V_DBG_LOG(UTIL_ERR_LEVEL,UTIL_NAME,"(%d)invalid array index(?) in %s\n", getpid(),input);
        ret = -1;
        goto out;
    }

    memset(str, 0, sizeof(str));
    memcpy(str, input + pmatch[0].rm_so, pmatch[0].rm_eo - pmatch[0].rm_so);
    sscanf(str, "%d", index);

    out:
    regfree(&reg);
    return ret;
#undef REG_STR_MAX
#undef REG_STR_ARR_MAX
}
#if 0
/* ONLY SUPPORTS DENSO FORMATTED LOG FILES and then only supports
   val = xxxx format */
uint8_t i2vUtilUpdateCfgValue(uint8_t *cfgfile, uint8_t *param, uint8_t *newval, bool_t hasLimits, bool_t hasComments)
{
#define MAX_FILE_LINE 1024
    bool_t finished = WFALSE, contentafter = WFALSE;
    FILE *f = NULL;
    /* max size line supported */
    uint8_t delim = 0;
    char_t buf[MAX_FILE_LINE] = {0}, newstring[MAX_FILE_LINE] = {0};
    char_t *ptr = NULL;
    uint32_t bytesread = 0, stringoff = 0;

    if ((NULL == cfgfile) || (NULL == param) || (NULL == newval)) {
        return I2V_RETURN_NULL_PTR;
    }
    if (hasLimits) {
        contentafter = WTRUE;
        delim = ';';
    }
    if (!hasLimits && hasComments) {
        contentafter = WTRUE;
        delim = '#';
    }

    if (NULL == (f=fopen((char_t *) cfgfile, "r+"))) {
        return I2V_RETURN_FAIL;
    }

    while (NULL != fgets(buf, MAX_FILE_LINE, f)) {
        if (!strncmp((char_t *)param, buf, strlen((char_t *)param))) {
            /* have a match - make sure next val is space or = */
            ptr = &buf[strlen((char *)param)];
            if ((*ptr == '\t') || (*ptr == ' ') || (*ptr == '=')) {
                /* begin processing */
                ptr = strchr(buf, '=');
                if (NULL == ptr) {
                    bytesread += strlen(buf);
                    continue;
                }
                /* increment ptr to point after = */
                ptr++;
                stringoff = bytesread;
                stringoff += (uint32_t)(ptr - buf);
                bytesread += strlen(buf);
                sprintf(newstring, " %s ", newval);
                if (contentafter) {
                    ptr = strchr(buf, delim);
                    if (NULL != ptr) {
                        strncat(newstring, ptr, (MAX_FILE_LINE - strlen(newstring)));
                    }
                }
                /* have the updated string now need to write without stomping on
                   following lines in file */
                while (WTRUE) {
                    memset(buf, 0, sizeof(buf));
                    fseek(f, bytesread, SEEK_SET);
                    if(NULL == fgets(buf, MAX_FILE_LINE, f)){ /* preserve next line */
                        return I2V_RETURN_FAIL;
                    }   
                    if (strlen(buf)) {
                        bytesread += strlen(buf);
                    }
                    if (feof(f)) {
                        finished = WTRUE;
                    }
                    fseek(f, stringoff, SEEK_SET);
                    stringoff += fwrite(newstring, 1, strlen(newstring), f);
                    if (finished) {
                        if (strlen(buf)) {
                            /* possible last line */
                            stringoff += fwrite(buf, 1, strlen(buf), f);
                        }
                        fflush(f);
                        fclose(f);
                        if(-1 == truncate((char *)cfgfile, stringoff)){
                            return I2V_RETURN_FAIL;
                        } else {
                            return I2V_RETURN_OK;
                        }
                    } else {
                        strncpy(newstring, buf, MAX_FILE_LINE);
                    }
                }
            }
        }
        bytesread += strlen(buf);
        memset(buf, 0, sizeof(buf));
    }

    fclose(f);
    return I2V_RETURN_NO_ACTION;
#undef MAX_FILE_LINE
}

#endif
/*
 *  System log related functions
 */

void i2vGetTime(char_t *data)
{
    i2vTimeT clock;

    if (NULL == data) return;

    memset(&clock, 0, sizeof(clock));
    i2vUtilGetUTCTime(&clock);
    sprintf(data, "%d%.2d%.2d_%.2d%.2d%.2d", clock.year, clock.month, clock.day, clock.hour, clock.min, clock.sec);
}

static void i2vUtilGetLabel(char_t *label, const char_t *name, i2vLogSevT level)
{
    /* label format(not including leading '|'): | ABCDEFGHI | SEVR | */
    /* 9 characters reserved for name - both cols have leading and ending spaces */
   
    size_t i, namespaces, leadingspaces;
    /* this list needs to match enum order - all entries expected 4 chars */
    /* Must match i2v_util.h::i2vLogSevT for array indexing below. */
    char_t *strtxt[] = {"EMER", "ALRT", "CRIT", "ERR ", "WARN", "NOTE", "INFO", "DBG ","PRIV"};
    char_t buf[I2V_LOG_NAME_MAX+1] = {0};

    strncpy(buf, (char *) name, I2V_LOG_NAME_MAX);
    namespaces = 11 - strlen(buf);
    leadingspaces = namespaces / 2;

    for (i=0; i<leadingspaces; i++) strcat(label, " ");
    strcat(label, buf);
    for (i=0; i<(namespaces - leadingspaces); i++) strcat(label, " ");
    strcat(label, "| ");
    strcat(label, strtxt[level]);
    strcat(label, " |");
}

/* Handles printf(i2vGlobalDebugFlag) and syslog(i2vSysLogGlobalControl) output.
 *   Will limit printf output incase user starts logging big strings to syslog.
 *   Must use non-blocking queues:I2V_Q_SENDER.
 *   Syslogs cares about LEVEL but printf does not. 
 *   How do we want logger to be used in I2V?
 *   Possible in future to limit by NAME. Each task could have appDebugEnable in conf to enable per task.
 *   This handles both serial and syslog entries for I2V_DBG_LOG. Name midleading.
 */
void i2vUtilAddToSysLog(i2vLogSevT logLevel, const char_t *name, const char_t *msg, ...)
{
    if(   (NULL == name) || (NULL == msg)
       || ((0x0 == i2v_utilEnableDebug) && (0x0 == i2v_utilSysLogEnableDebug))) {
        return; /* Not ready or bogus. Ignore. */
    }
    if(0x0 != i2v_utillocked) {
        i2v_utillockfailcnt++;
        #if defined(EXTRA_DEBUG)
        if(1 == i2v_utillockfailcnt) { /* One time be kind and send to serial. */
          char_t buf[I2V_PRINTF_MAX];  /* Protect behind lock above */
          uint32_t i,j;
            memset(buf,'\0',sizeof(buf));
            for(i=0,j=0; (j< sizeof(buf)) && (i<strlen(name)); i++,j++) {
                if(('\n' == name[i]) || ('\0' != name[i])){
                    break;
                }
                buf[j] = name[i];
            }
            if(j < sizeof(buf)) {
                buf[j] = '|';
                j++;
            }
            for(i=0,j=j; (j<sizeof(buf)) && (i<strlen(msg)); i++,j++) {
                if(('\n' == msg[i]) || ('\0' == msg[i])){
                    break;
                }
                buf[j] = msg[i];
            }
            /* Will look weird without args parsed out but better than nothing. Enough to debug */
            #if defined(ENABLE_STDOUT)
            printf("%s|I2V_DBG_LOG overflow: (%s)\n\n",UTIL_NAME,buf);
            fflush(stdout);
            #else
            fprintf(stderr,"%s|I2V_DBG_LOG overflow: (%s)\n\n",UTIL_NAME,buf);
            fflush(stderr);
            #endif
        }
        #endif
    } else {
        i2v_utillocked = 0x1;
        {
          char_t str[MAX_LOG_MSG_SIZE + 1]; /* Protect with above lock */
          char_t label[LOG_LABEL_SIZE + 1];
          char_t stdout_buf[I2V_PRINTF_MAX]; /*Limit serial I/O. Not the same as writing to file.*/
          LogMsg syslogMsg;
          va_list args;
            memset(str,'\0',sizeof(str));
            memset(label,'\0',sizeof(label));
            memset(stdout_buf,'\0',sizeof(stdout_buf));
            va_start(args, msg);
            vsnprintf(str, MAX_LOG_MSG_SIZE, (char *) msg, args);
            va_end(args);

            if (i2vGlobalDebugFlag) {
                if (logLevel < LEVEL_PRIV) {
                    i2vUtilGetLabel(label, name, logLevel);
                    /* Dont know why but need extra newline or else output gets hung up */
                    snprintf(stdout_buf,sizeof(stdout_buf),"%s%s\n", label, str);
                    #if defined(ENABLE_STDOUT)
                    printf("%s",stdout_buf);
                    fflush(stdout);
                    #else
                    fprintf(stderr,"%s",stdout_buf);
                    fflush(stderr);
                    #endif
                }
            } else {
                if(logLevel <= LEVEL_CRIT) {
                    i2vUtilGetLabel(label, name, logLevel);
                    /* Dont know why but need extra newline or else output gets hung up */
                    snprintf(stdout_buf,sizeof(stdout_buf),"%s%s\n", label, str);
                    #if defined(ENABLE_STDOUT)
                    printf("%s",stdout_buf);
                    fflush(stdout);
                    #else
                    fprintf(stderr,"%s",stdout_buf);          
                    fflush(stderr);
                    #endif
                }
            }
            /* LOGMGR will drop messages with LEVEL less than syslogdot.conf::LogMinPriority. */
            if(i2vSysLogGlobalControl) {
                if((0 == i2vGlobalDebugFlag) && (LEVEL_CRIT < logLevel)) {
                    /* Do not log these levels if flag off */
                } else {
                    if(LEVEL_PRIV == logLevel) { /* There is no level PRIV, change to DBG */
                        logLevel = LEVEL_DBG;
                        memset(label,'\0',sizeof(label));
                        memset(stdout_buf,'\0',sizeof(stdout_buf));
                        i2vUtilGetLabel(label, name, logLevel);
                        snprintf(stdout_buf,sizeof(stdout_buf),"%s%s", label, str);
                    }
                    /* Must not equal LMGR_IFC_REQ_CMD or NULL char */
                    syslogMsg.entry[0] = logLevel + 1;  /* +1 since we don't want a null term byte to start */
                    sprintf((char *) &syslogMsg.entry[1], "%s%s", label, str);
                    if(-1 == mq_send(syslogQueue, (char_t *)&syslogMsg, sizeof(syslogMsg), 0)) {
                        if(i2vUtilAddToSysLog_err_count < 3) {
                            i2vUtilAddToSysLog_err_count++;
                            #if defined(ENABLE_STDOUT)
                            printf("%s|i2vUtilAddToSysLog:mq_send fail: errno(%s): msg(%s)\n\n", UTIL_NAME, strerror(errno),stdout_buf);
                            fflush(stdout);
                            #else
                            fprintf(stderr,"%s|i2vUtilAddToSysLog:mq_send fail: errno(%s): msg(%s)\n\n", UTIL_NAME, strerror(errno),stdout_buf);
                            fflush(stderr);
                            #endif
                        }
                    }
                }
            }
        }
        i2v_utillocked = 0x0;
    }
}
#if 0 /* Recover queue on reboot, so dont bother. */
static void i2vUtilCloseSyslogQueue(void)
{
  i2vSysLogGlobalControl = WFALSE;
  mq_close(syslogQueue);
  /* Nothing has queue open so only need to un-link to remove. */
  mq_unlink(I2V_LOGMSG_QUEUE);
}
#endif

void i2vUtilIfcLogRequest(i2vIfcList ifc, lmgrIfcReqState state)
{
    char_t buf[sizeof(LogMsg)];
    mqd_t syslogQueue;

    /* No syslogs then no ifc logs. */
    if (!i2vSysLogGlobalControl) {
        return;
    }

    buf[0] = LMGR_IFC_REQ_CMD;
    buf[1] = (char_t)ifc;   /* eth0,eth1,dsrc0,dsrc1,cv2x0,cv2x1. */
    buf[2] = (char_t)state; /* on,off */

    /* This must exist. */
    if (-1 == (syslogQueue = mq_open(I2V_LOGMSG_QUEUE, O_WRONLY))) {
#if defined(EXTRA_DEBUG)
        printf("i2vUtilIfcLogRequest:mq_open fail:errno(%s)\n", strerror(errno));
        sleep(1);
#endif
        return;
    }

    if(-1 == mq_send(syslogQueue, buf, sizeof(buf), 0)) {
#if defined(EXTRA_DEBUG)
        printf("i2vUtilIfcLogRequest:mq_send fail:errno(%s)\n", strerror(errno));
        sleep(1);
#endif
    }
    mq_close(syslogQueue);

}

/*
 *  IWMH messaging functions
 * 
 *  NOTE: if adding new messages NEVER send more bytes than expected;
 *        iwmh can handle it but it CAN and SOMETIMES WILL result in
 *        a follow on message getting missed (which can cause the system
 *        to fail if early enough in the startup process)
 */
/* 20181128: uservice and pvservice were removed; check why no longer needed? */
uint32_t i2vUtilWMHServiceReq(const char_t *name, pid_t pid, bool_t isUserReq, 
    ProviderServiceType __attribute__((unused)) *pvservice, UserServiceType __attribute__((unused)) *uservice, 
    uint16_t msecTO, cfgItemsT *mycfg, uint8_t *respBuf)
{
    uint8_t reqseq, radionum;
    char_t buf[sizeof(i2vWMHServiceReq) + 1];
    char_t qname[I2V_CFG_MAX_STR_LEN];
    i2vWMHServiceReq *req;
    int32_t bufsz, msgqid = -1, fd = -1;
    i2vWMHRespMsg *resp;

    if (NULL == mycfg) {
        return I2V_RETURN_NULL_PTR;
    }

    bufsz = sizeof(i2vWMHServiceReq) + 1;
    memset(buf, 0, bufsz);
    buf[0] = WMH_SERVICE_REQUEST_CMD;
    req = (i2vWMHServiceReq *)&buf[1];
    if (isUserReq) {
        if (NULL == uservice) {
            return I2V_RETURN_NULL_PTR;
        }
        memcpy(&req->servreq.user, uservice, sizeof(UserServiceType));
        radionum = uservice->radioNum;
    } else {
        if (NULL == pvservice) {
            return I2V_RETURN_NULL_PTR;
        }
        memcpy(&req->servreq.provider, pvservice, sizeof(ProviderServiceType));
        radionum = pvservice->radioNum;
    }
    sprintf(qname, "%s%d", I2V_WMH_QUEUE, radionum);
    switch (radionum) {
        case 0: msgqid = mycfg->radio0wmh.msgListener; break;
        case 1: msgqid = mycfg->radio1wmh.msgListener; break;
        case 2: msgqid = mycfg->radio2wmh.msgListener; break;
        case 3: msgqid = mycfg->radio3wmh.msgListener; break;
        default: return I2V_IWMH_RETURN_INVALID_CONFIG;
    }
#if defined(MY_UNIT_TEST)
    if ((fd = open(qname, (O_CREAT | O_WRONLY | O_NONBLOCK ),777)) < 0) {
#else
    if ((fd = open(qname, I2V_Q_SENDER)) < 0) {
#endif

#ifdef EXTRA_DEBUG
        I2V_DBG_LOG(UTIL_ERR_LEVEL,UTIL_NAME,"i2vUtilWMHServiceReq: open WMH queue(%s) failed:errno(%s).\n",qname,strerror(errno));
#endif
        return I2V_RETURN_FAIL;
    }
    req->pid = pid;
    req->seqnum = reqseq = mywmhseqnum++;
    req->isUserReq = isUserReq;
    strncpy((char *) req->name, name, I2V_LOG_NAME_MAX);
    if (write(fd, buf, bufsz) < bufsz) {
#ifdef EXTRA_DEBUG
        I2V_DBG_LOG(UTIL_ERR_LEVEL,UTIL_NAME,"i2vUtilWMHServiceReq: write to WMH queue failed(%s).\n",strerror(errno));
#endif
        close(fd);
        return I2V_RETURN_FAIL;
    }
    close(fd);
#ifdef EXTRA_DEBUG
    gettimeofday(&tv, NULL);
    firstsec = tv.tv_sec;
    firstusec = tv.tv_usec;
#endif /* EXTRA_DEBUG */

    //if(NULL != respBuf)
    //    respBuf[0] = 0x0;
    //if(0 == msecTO) /* Don't want to wait. Done. */
    //    return I2V_RETURN_OK;

getresp:
    /* response from iwmh */
    memset(buf, 0, bufsz);
    resp = (i2vWMHRespMsg *)buf;
    while (WTRUE) {
        /* repurposing fd */
        if (((fd = msgrcv(msgqid, (void *)resp, bufsz, (int64_t)pid, IPC_NOWAIT)) < 0) &&
           ((errno == EAGAIN) || (errno == ENOMSG))) {
            usleep(1000);
            if (msecTO) {
                msecTO--;
            } else {
                /* timeout expiration */
#ifdef EXTRA_DEBUG
                gettimeofday(&tv, NULL);
                lastsec = tv.tv_sec;
                lastusec = tv.tv_usec;
                I2V_DBG_LOG(UTIL_ERR_LEVEL,UTIL_NAME,"i2vUtilWMHServiceReq:(0xD0 | I2V_RETURN_OP_INCOMPLETE)(%s).\n",strerror(errno));
#endif /* EXTRA_DEBUG */
                return (0xD0 | I2V_RETURN_OP_INCOMPLETE);  /* to distinguish incomplete type */
            }
        } else if (fd < 0) {
            /* some type of failure on receive - incomplete finish */
#ifdef EXTRA_DEBUG
            I2V_DBG_LOG(UTIL_ERR_LEVEL,UTIL_NAME,"i2vUtilWMHServiceReq:(0xE0 | I2V_RETURN_OP_INCOMPLETE) fd=%d errno=(%s).\n",fd,strerror(errno));
            I2V_DBG_LOG(UTIL_ERR_LEVEL,UTIL_NAME,"i2vUtilWMHServiceReq:msgqid = %d resp = %p bufsz = %d pid=0x%x.\n", msgqid,resp,bufsz,pid);
#endif
//without iwmh this will fail?
#if !defined(MY_UNIT_TEST)
            return (0xE0 | I2V_RETURN_OP_INCOMPLETE);
#else
            return(I2V_RETURN_OK);
#endif
        } else {
            break;
        }
    }

    /* have a reply - verify its 'our' reply then process */
    if (resp->mtype != (int64_t)pid) {
        /* not for us even though msgrcv said it was - nothing else to do */
#ifdef EXTRA_DEBUG
        I2V_DBG_LOG(UTIL_ERR_LEVEL,UTIL_NAME,"i2vUtilWMHServiceReq:(0xF0 | I2V_RETURN_OP_INCOMPLETE)(%s).\n",strerror(errno));
#endif
        return (0xF0 | I2V_RETURN_OP_INCOMPLETE);
    }
    if (resp->mtext[0] < reqseq) {   /* got an older response */
        goto getresp;  /* this gives one more chance (no timeout) to get response */
        /* this in theory could create an infinite loop if mtext[0] is always less
           than seqnum (data corruption) but in practicality the msgrcv should
           not always come back with a response for this thread unless iwmh is 
           continually sending back to this thread - so in practice at worst
           this won't loop forever and we should get an OP_INCOMPLETE return */
    }
    if ((NULL != respBuf) && (fd > 2)) {
        memcpy(respBuf, &resp->mtext[2], fd - 2);   /* future support */
    }
    /* in this case first byte in response buf is WTRUE or WFALSE */

    return (resp->mtext[1] == WTRUE) ? I2V_RETURN_OK : I2V_RETURN_NO_ACTION;
}

uint32_t i2vUtilWMHWSMReq(pid_t pid, outWSMType *wsm, uint16_t msecTO, cfgItemsT *mycfg, uint8_t *respBuf)
{
    uint8_t reqseq;
    char_t buf[sizeof(i2vWMHWsmReq) + 1];
    char_t qname[I2V_CFG_MAX_STR_LEN];
    i2vWMHWsmReq *req;
    int32_t bufsz, msgqid = -1, fd = -1;
    i2vWMHRespMsg *resp;

    if ((NULL == mycfg) || (NULL == wsm)) return I2V_RETURN_NULL_PTR;
    bufsz = sizeof(i2vWMHWsmReq) + 1;
    memset(buf, 0, bufsz);
    buf[0] = WMH_SEND_WSM_CMD;
    req = (i2vWMHWsmReq *)&buf[1];
    memcpy(&req->wsm, wsm, sizeof(outWSMType));
    sprintf(qname, "%s%d", I2V_WMH_QUEUE, wsm->radioNum);
    switch (wsm->radioNum) {
        case 0: msgqid = mycfg->radio0wmh.msgListener; break;
        case 1: msgqid = mycfg->radio1wmh.msgListener; break;
        case 2: msgqid = mycfg->radio2wmh.msgListener; break;
        case 3: msgqid = mycfg->radio3wmh.msgListener; break;
        default: return I2V_IWMH2_RETURN_INVALID_CONFIG;
    }
    if ((fd = open(qname, I2V_Q_SENDER)) < 0) return I2V_RETURN_FAIL;
    req->pid = pid;
    req->seqnum = reqseq = mywmhseqnum++;
    if(-1 == write(fd, buf, bufsz)) return I2V_RETURN_FAIL;
    close(fd);

getresp:
    /* response from iwmh */
    memset(buf, 0, bufsz);
    resp = (i2vWMHRespMsg *)buf;
#ifdef EXTRA_DEBUG
    gettimeofday(&tv, NULL);
    firstsec = tv.tv_sec;
    firstusec = tv.tv_usec;
#endif /* EXTRA_DEBUG */
    /* never fail for timeout - update status only */
    while (WTRUE) {
        /* repurposing fd */
        if (((fd = msgrcv(msgqid, (void *)resp, bufsz, (int64_t)pid, IPC_NOWAIT)) < 0) &&
           ((errno == EAGAIN) || (errno == ENOMSG))) {
            usleep(1000);
            if (msecTO) {
                msecTO--;
            } else {
                /* timeout expiration */
#ifdef EXTRA_DEBUG
                gettimeofday(&tv, NULL);
                lastsec = tv.tv_sec;
                lastusec = tv.tv_usec;
#endif /* EXTRA_DEBUG */
                if (NULL != respBuf) {
                    *respBuf =  (0xD0 | I2V_RETURN_OP_INCOMPLETE);  /* to distinguish incomplete type */
                }
                return I2V_RETURN_OK;
            }
        } else if (fd < 0) {
            /* some type of failure on receive - incomplete finish */
            if (NULL != respBuf) {
                *respBuf = (0xE0 | I2V_RETURN_OP_INCOMPLETE);
            }
            return I2V_RETURN_OK;
        } else {
            break;
        }
    }
    /* have a reply - verify its 'our' reply then process */
    if (resp->mtype != (int64_t)pid) {
        /* not for us even though msgrcv said it was - nothing else to do */
        if (NULL != respBuf) {
            *respBuf = (0xF0 | I2V_RETURN_OP_INCOMPLETE);
        }
        return I2V_RETURN_OK;
    }
    if (resp->mtext[0] < reqseq) {   /* got an older response */
        goto getresp;  /* this gives one more chance (no timeout) to get response */
        /* this in theory could create an infinite loop if mtext[0] is always less
           than seqnum (data corruption) but in practicality the msgrcv should
           not always come back with a response for this thread unless iwmh is 
           continually sending back to this thread - so in practice at worst
           this won't loop forever and we should get an OP_INCOMPLETE return */
    }
    if ((NULL != respBuf) && (fd > 2)) {
        memcpy(respBuf, &resp->mtext[2], fd - 2);   /* future support */
    }
    /* in this case first byte in response buf is WTRUE or WFALSE */
    return (resp->mtext[1] == WTRUE) ? I2V_RETURN_OK : I2V_RETURN_NO_ACTION;
}

static void *utilRxCBThread(void __attribute__((unused)) *arg)
{
    uint8_t rcvbuf[sizeof(i2vWMHRcvTypes)];
    ssize_t bytes = 0;
    i2vWSMXferMsg *msg = (i2vWSMXferMsg *)rcvbuf;
    inWSMType *wsm = (inWSMType *)&msg->mtext[1];
    i2vWMHVODMsg *vmsg = (i2vWMHVODMsg *)&msg->mtext[1];

    while (*mywmhmainloop) {
        memset(rcvbuf, 0, sizeof(i2vWMHRcvTypes));
        if (((bytes = (msgrcv(mycfgptr->iwmhSecQ, (void *)rcvbuf, sizeof(i2vWMHRcvTypes), (int64_t)mypid, IPC_NOWAIT))) < 0) &&
            ((errno == EAGAIN) || (errno == ENOMSG))) {
            usleep(1100);
        } else if ((uint32_t)bytes < MIN_VALID_WMH_MSG_SZ) {
            usleep(1100);
        } else {
            /* this switch works because first part of all rcv structs is int64_t and first byte of mtext is cmd */
            switch (msg->mtext[0]) {
                case WMH_IN_WSM:
                    /* call my wsm rcv fxn */
                    if (NULL != myWSMRxFxn) {
                        myWSMRxFxn(wsm);
                    }
                    break;
                case WMH_VOD_RESP:
                    /* call my vod rsp fxn */
                    if (NULL != myVODFxn) {
                        myVODFxn(vmsg->result, vmsg->vodSeqNum);
                    }
                    break;
                default:
                    break;
            }
        }
    }
    pthread_exit(NULL);
}
/* 
 * From ris_struct.h : "Type definition of Provider Service Identifiers (PSID) (the actual values in this enum should not be used because they are out of date)."
 */
i2vReturnTypesT i2vUtilWMHRxCallback(pid_t pid, uint16_t msecTO, cfgItemsT *mycfg, uint8_t radioNum, PSIDType psid,
    bool_t *mainloopptr, void *rxFunction, void *vodFunction, uint8_t *respBuf)
{
    uint8_t reqseq;
    char_t buf[MIN_WMH_RESP_BUF];
    char_t qname[I2V_CFG_MAX_STR_LEN];
    int32_t bufsz, msgqid = -1, fd = -1;
    i2vWMHRespMsg *resp;
    i2vWMHCBReq *req;
    i2vWMHIntRespType *pidresp;

    /* need at least the rx function otherwise no use in calling this fxn */
    if ((NULL == mycfg) || (rxFunction == NULL) || (NULL == mainloopptr)) return I2V_RETURN_NULL_PTR;
    if (!myWMHRxInited) {
        /* don't overwrite values if called more than once */
        mycfgptr = mycfg;
        mypid = pid;
        myWSMRxFxn = rxFunction;   /* don't care if null or not - thread handles null */
        myVODFxn = vodFunction;   /* don't care if null or not - thread handles null */
        mywmhmainloop = mainloopptr;
    }
    bufsz = sizeof(i2vWMHCBReq) + 1;
    memset(buf, 0, MIN_WMH_RESP_BUF);
    buf[0] = WMH_RX_CB_REGISTER_CMD;
    req = (i2vWMHCBReq *)&buf[1];
    sprintf(qname, "%s%d", I2V_WMH_QUEUE, radioNum);
    switch (radioNum) {
        case 0: msgqid = mycfg->radio0wmh.msgListener; break;
        case 1: msgqid = mycfg->radio1wmh.msgListener; break;
        case 2: msgqid = mycfg->radio2wmh.msgListener; break;
        case 3: msgqid = mycfg->radio3wmh.msgListener; break;
        default: return I2V_IWMH3_RETURN_INVALID_CONFIG;
    }
    if ((fd = open(qname, I2V_Q_SENDER)) < 0) return I2V_RETURN_FAIL;
    req->pid = pid;
    req->seqnum = reqseq = mywmhseqnum++;
    req->psid = psid;
    if(-1 == write(fd, buf, bufsz)) return I2V_RETURN_FAIL;
    close(fd);

getresp:
    /* response from iwmh */
    memset(buf, 0, bufsz);
    resp = (i2vWMHRespMsg *)buf;
    pidresp = (i2vWMHIntRespType *)&resp->mtext[2];
    while (WTRUE) {
        /* repurposing fd */
        if (((fd = msgrcv(msgqid, (void *)resp, MIN_WMH_RESP_BUF, (int64_t)pid, IPC_NOWAIT)) < 0) &&
           ((errno == EAGAIN) || (errno == ENOMSG))) {
            usleep(1000);
            if (msecTO) {
                msecTO--;
            } else {
                /* timeout expiration */
                return (0xD0 | I2V_RETURN_OP_INCOMPLETE);  /* to distinguish incomplete type */
            }
        } else if (fd < 0) {
            /* some type of failure on receive - incomplete finish */
            return (0xE0 | I2V_RETURN_OP_INCOMPLETE);
        } else {
            break;
        }
    }
    /* have a reply - verify its 'our' reply then process */
    if (resp->mtype != (int64_t)pid) {
        /* not for us even though msgrcv said it was - nothing else to do */
        return (0xF0 | I2V_RETURN_OP_INCOMPLETE);
    }
    if (resp->mtext[0] < reqseq) {   /* got an older response */
        goto getresp;  /* this gives one more chance (no timeout) to get response */
        /* this in theory could create an infinite loop if mtext[0] is always less
           than seqnum (data corruption) but in practicality the msgrcv should
           not always come back with a response for this thread unless iwmh is 
           continually sending back to this thread - so in practice at worst
           this won't loop forever and we should get an OP_INCOMPLETE return */
    }
    if ((NULL != respBuf) && (fd > 2)) {
        memcpy(respBuf, &resp->mtext[2], fd - 2);   /* future support */
    }
    /* in this case first byte in response buf is WTRUE or WFALSE */
    if (resp->mtext[1] == WFALSE) {
        return I2V_RETURN_NO_ACTION;
    }
    if (!myWMHRxInited) {
        mywmhpid = pidresp->intT;
        /* create thread for rx */
        if (pthread_create(&myWMHRxThrId, NULL, utilRxCBThread, NULL)) {
            return I2V_RETURN_FAIL;
        }
        myWMHRxInited = WTRUE;
    }
    return I2V_RETURN_OK;
}

void i2vUtilRxCBCleanup(void)
{
    /* this used to be for the below, do nothing now (see above 20190208 comment) */
    pthread_join(myWMHRxThrId, NULL);
}

bool_t i2vUtilWMHVODVerify(uint32_t seqnum)
{
    uint8_t buf[sizeof(i2vWMHVerifyQMsg)];
    i2vWMHVerifyQMsg *msg = (i2vWMHVerifyQMsg *)buf;
    i2vWMHVODMsg *req = (i2vWMHVODMsg *)msg->mtext;

    if (!myWMHRxInited) {
        return WFALSE;
    }

    req->pid = mypid;
    req->vodSeqNum = seqnum;
    msg->mtype = mywmhpid;

    return (msgsnd(mycfgptr->iwmhSecQ, (void *)buf, sizeof(i2vWMHVerifyQMsg), IPC_NOWAIT) < 0) ? WFALSE : WTRUE;
}


/*
 *  Miscellaneous functions for directory, time, log files, and wsm cert/digest
 */

/* Radio handles security */
bool_t i2vUtilCertOrDigest(i2vShmMasterT *shrptr, cfgItemsT *cfg, uint32_t msgType)
{
    if ((NULL == shrptr) || (NULL == cfg) || (0 == msgType)) {
        /* nothing to do if invalid input */
        return WFALSE;
    }

    if (!cfg->security) {
        /* nothing to do if security disabled */
        return WFALSE;
    }

    return WTRUE;

}


/* Appends time/date to a filename string */
void i2vUtilAppendTimeDateToFileName(char_t *filename)
{
    time_t     result1;
    struct tm *timeptr, tm;
    char_t     timeDateString[40];

    if (NULL == filename) return;

    result1 = time(NULL);
    timeptr = gmtime_r(&result1, &tm);

    sprintf(timeDateString, "_%d%.2d%.2d_%.2d%.2d%.2d",
      1900 + timeptr->tm_year,
      (timeptr->tm_mon + 1),
      timeptr->tm_mday, timeptr->tm_hour,
      timeptr->tm_min, timeptr->tm_sec);

    strcat(filename, timeDateString);
}

void i2vUtilGetUTCTime(i2vTimeT *clock)
{
    time_t    val;
    struct tm t, *tmptr;

    if (NULL == clock) return;
   
    val = time(NULL);
    tmptr = gmtime_r(&val, &t);

    clock->year  = (uint16_t)(1900 + tmptr->tm_year);
    clock->month = (uint8_t)(1 + tmptr->tm_mon);
    clock->day   = (uint8_t)tmptr->tm_mday;
    clock->hour  = (uint8_t)tmptr->tm_hour;
    clock->min   = (uint8_t)tmptr->tm_min;
    clock->sec   = (uint8_t)tmptr->tm_sec;
    clock->yday  = (uint16_t)tmptr->tm_yday;
    clock->wday  = (uint8_t)tmptr->tm_wday;
}

uint32_t i2vUtilGetUTCTimeInSec(void)
{
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_sec;
}

uint64_t i2vUtilGetTimeInMs(void)
{
    struct timeval tv;
    uint64_t timenow;

    gettimeofday(&tv, NULL);
    timenow = tv.tv_sec;
    timenow *= 1000;
    timenow += tv.tv_usec/1000;
    return timenow;
}

i2vReturnTypesT i2vUtilCreateFile(FILE **fd_in, char_t *app_str, char_t *path, char_t *suffix, int8_t options)
{
    char_t filename[100];
    char_t write_option[3];
    FILE *fd = NULL;

    strcpy(filename, path );
    if (options & createFileAddDirSlash) {
        strcat(filename, "/");   /* just in case if input directory doesn't have a slash at end - no harm in opening a file with double slashes */
    }

    /* jjg cleanup need to check on prefix (if needed )
    if (*(shmPtr->cfgData.commonLogFilePrefix) != 0) {
        strcat(filename, shmPtr->cfgData.commonLogFilePrefix);
    } */

    strcat(filename, app_str);
    if (options & createFileAddDateTime) {
        i2vUtilAppendTimeDateToFileName(filename);
    }

    strcat(filename, suffix);

    strcpy(write_option, ((options & createFileBinaryFile) ? "wb" : "w"));
    if ((fd = fopen(filename, write_option )) == NULL) {
        return I2V_RETURN_FAIL;
    }
    *fd_in = fd;

    return I2V_RETURN_OK;
}

void i2vUtilCloseFile(FILE **fd)
{
    if (*fd != NULL) {
        fclose(*fd);
        *fd = NULL;
    }
}

/* this function will check for the existence of a directory
   and create it if non existent */
i2vReturnTypesT i2vUtilMakeDir(char_t *dirname)
{
    FILE *f;
    char_t syscmd[I2V_CFG_MAX_STR_LEN + 10]; /* mkdir -p dirname */

    if (NULL == dirname)
    {
        return I2V_RETURN_INVALID_PARAM;
    }

    if ((f=fopen(dirname, "r")) == NULL) {
        strcpy(syscmd, "mkdir -p ");
        strcat(syscmd, dirname);
        return (system(syscmd) != -1) ? I2V_RETURN_OK : I2V_RETURN_FAIL;
    } else {
        fclose(f);
    }

    return I2V_RETURN_OK;
}

i2vReturnTypesT i2vWriteDataToFile(char_t *fname, char_t *buf, uint32_t size, bool_t useAscii)
{
#define ASCII_ITER_SIZE 128
    FILE *f;
    uint32_t i=0, ctr=0;
    char_t asciibuf[(ASCII_ITER_SIZE * 3) + 1] = {0};  /* i2v 'ascii bytes' */
    char_t *ptr, *bufptr;

    if ((NULL == fname) || (NULL == buf)) {
        return I2V_RETURN_INVALID_PARAM;
    }

    if ((f=fopen(fname, "w")) == NULL) {
        return I2V_RETURN_FAIL;
    }    
    if (useAscii) {
        ctr = size / ASCII_ITER_SIZE;
        size -= (ctr * ASCII_ITER_SIZE);
        bufptr = buf;
        for (; ctr; ctr--) {
            ptr = asciibuf;
            for (i=0; i<ASCII_ITER_SIZE; i++) {
                sprintf(ptr, "%.2X ", bufptr[i]);
                ptr += 3;
            }
            fprintf(f, "%s", asciibuf);
            bufptr += ASCII_ITER_SIZE;
        }
        if (size) {
            /* final iteration or size < ASCII_ITER_SIZE */
            memset(asciibuf, 0, sizeof(asciibuf));  /* may have been used in iter loop */
            ptr = asciibuf;
            for (i=0; i<size; i++) {
                sprintf(ptr, "%.2X ", bufptr[i]);
                ptr +=3;
            }
            fprintf(f, "%s", asciibuf);
            /* for (i=0; i<size; i++) {
                fprintf(f, "%.2X ", buf[i]);
            } */ /* orig implementation - alot less code - alot more inefficient */
        }
    } else {
        i = (uint32_t)fwrite(buf, sizeof(char_t), size, f);
    }
    fclose(f);

    if (i==0) return I2V_RETURN_FAIL;
    if (i < size) return I2V_RETURN_OP_INCOMPLETE;
    return I2V_RETURN_OK;
#undef ASCII_ITER_SIZE
}

/* this function checks if a directory or file exists only
   input pname must be fully qualified */
bool_t i2vCheckDirOrFile(const char_t *pname)
{
    FILE *f;

    if ((NULL == pname) || ((f=fopen(pname, "r")) == NULL)) {
        return WFALSE;
    }

    fclose(f);
    return WTRUE;
}

/* this function scans for the input full filename
   or partial file name */
bool_t i2vScanDirForFile(const char_t *dirname, const char_t *pname)
{
    char_t cmd[2 * I2V_CFG_MAX_STR_LEN + 48 ];

    if ((NULL == dirname) || (NULL == pname)) {
        return WFALSE;
    }
    if ((strlen(dirname) > (I2V_CFG_MAX_STR_LEN + 25)) ||
       (strlen(pname) > I2V_CFG_MAX_STR_LEN)) {
        return WFALSE;
    }

    sprintf(cmd, "ls %s | grep %s >/dev/null", dirname, pname);

    return (!system(cmd) ? WTRUE : WFALSE);  /* returns 1 if name not found */
}

/* this function will create a directory listing of file 
   names in the input filename (fully qualified) using 
   the input directory name; if extension is non null
   the listing will be just files of that extension */
i2vReturnTypesT i2vDirectoryListing(char_t *dirname, char_t *fname, char_t *extension)
{
    char_t syscmd[2 * I2V_CFG_MAX_STR_LEN + 50];  /* assuming dirname and fname max I2V_CFG_MAX_STR_LEN + command args */

    if ((NULL == dirname) || (NULL == fname)) {
        return I2V_RETURN_INVALID_PARAM;
    }

    strcpy(syscmd, "ls ");
    strcat(syscmd, dirname);
    if ((NULL != extension) && (strlen(extension))) {
        strcat(syscmd, " | grep '\\.");
        strcat(syscmd, extension);
        strcat(syscmd, "'");
    }
    strcat(syscmd, " > ");
    strcat(syscmd, fname);
    //return ((system(syscmd)) ? I2V_RETURN_FAIL : I2V_RETURN_OK);   /* system should return 0 on success */
    /* empty directory support */
    if(system(syscmd) < 0){
        /*If this call fails then below will fail so we are covered. Just silence warning. */
    }
    return ((i2vCheckDirOrFile(fname)) ? I2V_RETURN_OK : I2V_RETURN_FAIL);
}

/* simplified from v2v-i - always being created even though
 * there was extra handling - not including that for i2v 
 * calling functions need to properly supply input (no 
 * freebies on assuming directory locations */
void i2vUtilCreateNewLogFiles(
                           FILE **csvlog,
                           FILE **dbglog,
                           bool_t logflg,
                           bool_t debuglog,
                           bool_t global_log,
                           bool_t global_debuglog,
                           char_t *title,
                           char_t *app_str,
                           char_t *applogs_dirname,
                           char_t *logs_dirname,
                           i2vLogErrT *status
                           )
{
    bool_t create_log = (logflg && global_log);
    bool_t create_dbg = (debuglog && global_debuglog);

    *status = I2V_LOG_ERR_NONE;

    if (((create_dbg) && (NULL == logs_dirname)) || ((create_log) && (NULL == applogs_dirname))) {
        /* requesting function didn't supply directory name - failing op */
        *status = I2V_LOG_ERR_INPUT;
        return;
    }

    if ((create_dbg) && (*dbglog == NULL)) {
        if (I2V_RETURN_OK != i2vUtilCreateFile(dbglog, app_str, logs_dirname, ".log", createFileAddDirSlash)) {
            *status = I2V_LOG_ERR_DBG;
        }
    }

    /* continuing even if error on debug log */
    if ((create_log) && (*csvlog == NULL)) {
        if (I2V_RETURN_OK != i2vUtilCreateFile(csvlog, app_str, applogs_dirname, ".csv", ( createFileAddDateTime | createFileAddDirSlash ))) {
            *status |= I2V_LOG_ERR_CSV;
        } else {
            fprintf(*csvlog, "%s", title);
            fflush(*csvlog);
        }
    }
    *status = (*status == I2V_LOG_ERR_NONE) ? I2V_LOG_ERR_SUCCESS : *status;
}

uint32_t i2vConvertToUTC(int32_t tm_year, int32_t tm_yday, int32_t tm_hour, int32_t tm_min, int32_t tm_sec)
{
  int32_t my_year    = tm_year; /*you must have added 1900 at this point.*/
  int32_t leap_days  = 0;
  int32_t i          = 0;
  int32_t dummy_time = 0;
  
    if((tm_year < 1970) || (2050 < tm_year)) {

        #if defined(EXTRA_DEBUG)
        I2V_DBG_LOG(UTIL_ERR_LEVEL,UTIL_NAME,"i2vConvertToUTC: Year is out of range!!= %d\n",tm_year);
        #endif
        return 0;
    }

    if(365 == tm_yday) { /* since we are going to manually add leap days subtract a day here */
        tm_yday = 364;   /* 12/31/2019  = 364, 12/31/2020 = 365 */
    }

    for(i=1972, my_year=tm_year,leap_days=0; (i<=my_year); i++) { /*Skipping to 1st leap year after 1970*/
        if( 0 == (i % 4)) {
            if( (i == my_year) && (tm_yday < (30 + 29 + 1))) { /*jan 1=0 day,jan 31st=30*/

                 /*don't apply if we haven't passed Feb 29th of leap year.*/

            } else {
                leap_days = leap_days + NUM_DAY_SECONDS;
            }  
        }
    } 

    /*
     * Normalized seconds since 1970
     */
    dummy_time = (tm_year - 1970) *  NUM_YEAR_SECONDS + (tm_yday*86400) + (tm_hour*3600) + (tm_min*60) + tm_sec;

    /*
     * Adjusted for leap days since 1970
     */
    dummy_time = dummy_time + leap_days; /*add needed leap days in seconds*/

    return (uint32_t)dummy_time;     
}

/*
   See dsrc.h for values. NOTE:: different values for DER vs UPER encoded.
   We only support J2735 2016 which is UPER only, security or not.

   To preserve legacy 2009 items their DER values are preserved
   and fall eventually into the UPER logic in the case statements
   inside relevent AMH/IMF logic.
*/
uint32_t dsrcMsgId_to_amhMsgType(uint32_t dsrc_msgId)
{
  uint32_t ret_val = 0x0; /* By default return nothing. */
  switch(dsrc_msgId) {
      case ASN1V_mapData:
          ret_val = AMH_MSG_MAP_MASK;
          break;
      case ASN1V_signalPhaseAndTimingMessage:
          ret_val = AMH_MSG_SPAT_MASK;
          break;
      case ASN1V_basicSafetyMessage:
          ret_val = AMH_MSG_BSM_MASK;
          break;
      case ASN1V_commonSafetyRequest:
          ret_val = AMH_MSG_CSR_MASK;
          break;
      case ASN1V_emergencyVehicleAlert:
          ret_val = AMH_MSG_EVA_MASK;
          break;
      case ASN1V_intersectionCollision:
          ret_val = AMH_MSG_ICA_MASK;
          break;
      case ASN1V_nmeaCorrections:
          ret_val = AMH_MSG_NMEA_MASK;
          break;
      case ASN1V_probeDataManagement:
          ret_val = AMH_MSG_PDM_MASK;
          break;
      case ASN1V_probeVehicleData:
          ret_val = AMH_MSG_PVD_MASK;
          break;
      case ASN1V_roadSideAlert:
          ret_val = AMH_MSG_RSA_MASK;
          break;
      case ASN1V_rtcmCorrections:
          ret_val = AMH_MSG_RTCM_MASK;
          break;
      case ASN1V_signalRequestMessage:
          ret_val = AMH_MSG_SRM_MASK;
          break;
      case ASN1V_signalStatusMessage:
          ret_val = AMH_MSG_SSM_MASK;
          break;
      case ASN1V_travelerInformation:
          ret_val = AMH_MSG_TIM_MASK;
          break;
      case ASN1V_personalSafetyMessage:
          ret_val = AMH_MSG_PSM_MASK;
          break;
      case ASN1V_testMessage00:
          ret_val = AMH_MSG_TEST00_MASK;
          break;
      case ASN1V_testMessage01:
          ret_val = AMH_MSG_TEST01_MASK;
          break;
      case ASN1V_testMessage02:
          ret_val = AMH_MSG_TEST02_MASK;
          break;
      case ASN1V_testMessage03:
          ret_val = AMH_MSG_TEST03_MASK;
          break;
      case ASN1V_testMessage04:
          ret_val = AMH_MSG_TEST04_MASK;
          break;
      case ASN1V_testMessage05:
          ret_val = AMH_MSG_TEST05_MASK;
          break;
      case ASN1V_testMessage06:
          ret_val = AMH_MSG_TEST06_MASK;
          break;
      case ASN1V_testMessage07:
          ret_val = AMH_MSG_TEST07_MASK;
          break;
      case ASN1V_testMessage08:
          ret_val = AMH_MSG_TEST08_MASK;
          break;
      case ASN1V_testMessage09:
          ret_val = AMH_MSG_TEST09_MASK;
          break;
      case ASN1V_testMessage10:
          ret_val = AMH_MSG_TEST10_MASK;
          break;
      case ASN1V_testMessage11:
          ret_val = AMH_MSG_TEST11_MASK;
          break;
      case ASN1V_testMessage12:
          ret_val = AMH_MSG_TEST12_MASK;
          break;
      case ASN1V_testMessage13:
          ret_val = AMH_MSG_TEST13_MASK;
          break;
      case ASN1V_testMessage14:
          ret_val = AMH_MSG_TEST14_MASK;
          break;
      case ASN1V_testMessage15:
          ret_val = AMH_MSG_TEST15_MASK;
          break;
      default:
          break;
  }
  return ret_val;
}
uint32_t J2735_DSRC_MSG_TYPE(amhBitmaskType bitmask)
{ 
  uint32_t ret_val = 0;

    switch(bitmask) {
         /* 
          * UPER encoding Only. DER not supported for J2735 2016 which is strictly UPER security or not.
          */
        case AMH_MSG_MAP_MASK:
            ret_val = ASN1V_mapData;
            break; 
        case AMH_MSG_SPAT_MASK:
            ret_val = ASN1V_signalPhaseAndTimingMessage;
            break;
        case AMH_MSG_BSM_MASK:
            ret_val = ASN1V_basicSafetyMessage;
            break;
        case AMH_MSG_CSR_MASK:
            ret_val = ASN1V_commonSafetyRequest;
            break;  
        case AMH_MSG_EVA_MASK:
            ret_val = ASN1V_emergencyVehicleAlert;
            break;
        case AMH_MSG_ICA_MASK:
            ret_val = ASN1V_intersectionCollision;
            break; 
        case AMH_MSG_NMEA_MASK:
            ret_val = ASN1V_nmeaCorrections;
            break;
        case AMH_MSG_PDM_MASK:
            ret_val = ASN1V_probeDataManagement;
            break; 
        case AMH_MSG_PVD_MASK:
            ret_val = ASN1V_probeVehicleData;
            break;
        case AMH_MSG_RSA_MASK:
            ret_val = ASN1V_roadSideAlert;
            break; 
        case AMH_MSG_RTCM_MASK:
            ret_val = ASN1V_rtcmCorrections;
            break; 
        case AMH_MSG_SRM_MASK:
            ret_val = ASN1V_signalRequestMessage;
            break; 
        case AMH_MSG_SSM_MASK:
            ret_val = ASN1V_signalStatusMessage;
            break;
        case AMH_MSG_TIM_MASK:
            ret_val = ASN1V_travelerInformation;
            break;
        case AMH_MSG_PSM_MASK:
            ret_val = ASN1V_personalSafetyMessage;
            break;
        case AMH_MSG_TEST00_MASK:
            ret_val = ASN1V_testMessage00;
            break;
        case AMH_MSG_TEST01_MASK:
            ret_val = ASN1V_testMessage01;
            break;
        case AMH_MSG_TEST02_MASK:
            ret_val = ASN1V_testMessage02;
            break;
        case AMH_MSG_TEST03_MASK:
            ret_val = ASN1V_testMessage03;
            break;
        case AMH_MSG_TEST04_MASK:
            ret_val = ASN1V_testMessage04;
            break;
        case AMH_MSG_TEST05_MASK:
            ret_val = ASN1V_testMessage05;
            break;
        case AMH_MSG_TEST06_MASK:
            ret_val = ASN1V_testMessage06;
            break;
        case AMH_MSG_TEST07_MASK:
            ret_val = ASN1V_testMessage07;
            break;
        case AMH_MSG_TEST08_MASK:
            ret_val = ASN1V_testMessage08;
            break;
        case AMH_MSG_TEST09_MASK:
            ret_val = ASN1V_testMessage09;
            break;
        case AMH_MSG_TEST10_MASK:
            ret_val = ASN1V_testMessage10;
            break;
        case AMH_MSG_TEST11_MASK:
            ret_val = ASN1V_testMessage11;
            break;
        case AMH_MSG_TEST12_MASK:
            ret_val = ASN1V_testMessage12;
            break;
        case AMH_MSG_TEST13_MASK:
            ret_val = ASN1V_testMessage13;
            break;
        case AMH_MSG_TEST14_MASK:
            ret_val = ASN1V_testMessage14;
            break;
        case AMH_MSG_TEST15_MASK:
            ret_val = ASN1V_testMessage15;
            break;
        default:
            ret_val = 0xBADBEEF; /* failure in the air */
            break;
    }
    return ret_val;
}

/* Will convert 8-bit ascii hex value to 4 bit nibble.
   Will return between 0x0 to 0xf.
   Will return 0xff on out of range ascii value.
*/
uint8_t i2v_ascii_2_hex_nibble(uint8_t  value)
{       
  uint8_t dummy = 0xFF;

  if(( '0' <= value) && (value <= '9'))
      dummy =  value - 48;
  if(( 'A' <= value) && (value <= 'F'))
      dummy =  value - 55;
  if(( 'a' <= value) && (value <= 'f'))
      dummy =  value - 87;

  return dummy;
}

/* Stringify spatData.spat_info.flexSpat.spatApproach[i].curSigPhase; */
char_t * i2v_sigphase_to_traffic_light_color_string(i2vSigPhases sigPhase, bool_t mention_flashing)
{
    switch(sigPhase) {
       case SIG_PHASE_DARK:
           return("Dark");
           break;
       case SIG_PHASE_GREEN:
           return("Green");
           break;
       case SIG_PHASE_YELLOW:
           return("Yellow");
           break;
       case SIG_PHASE_RED:
           return("Red");
           break;
       case SIG_PHASE_FLASHING_GREEN:
           return (mention_flashing ? "Flashing Green" : "Green");
           break;
       case SIG_PHASE_FLASHING_YELLOW:
           return (mention_flashing ? "Flashing Yellow" : "Yellow");
           break;
       case SIG_PHASE_FLASHING_RED:
           return (mention_flashing ? "Flashing Red" : "Red");
           break;
       case SIG_PHASE_UNKNOWN:
       default:
           return("UNK");
           break;
    }
    return("ERROR"); /* Never get here. */
}

// Puts a string name to the various movement_phase_state-es
char_t * i2v_movement_phase_state_to_string(i2vMovementPhaseState mps)
{
    switch(mps) {
        case MPS_UNAVAILABLE:
            return("UNAVAIL");
            break;
        case MPS_DARK:
            return("DARK");
            break;
        case MPS_STOP_THEN_PROCEED:
            return("STOP_THEN_PROCEED"); 
            break;        
        case MPS_STOP_AND_REMAIN:
            return("STOP_AND_REMAIN");
            break;        
        case MPS_PRE_MOVEMENT:
            return("PRE_MOVEMENT");
            break;        
        case MPS_PERMISSIVE_MOVEMENT_ALLOWED:
            return("PERMISSIVE_MOVEMENT_ALLOWED");
            break;  
        case MPS_PROTECTED_MOVEMENT_ALLOWED :
            return("PROTECTED_MOVEMENT_ALLOWED");
            break;  
        case MPS_PERMISSIVE_CLEARENCE:
            return("PERMISSIVE_CLEARENCE");
            break;             
        case MPS_PROTECTED_CLEARENCE:
            return("PROTECTED_CLEARENCE");
            break;
        case MPS_CAUTION_CONFLICTING_TRAFFIC:
            return("CAUTION_CONFLICTING_TRAFFIC");
            break;     
        default:
            return("UNAVAIL");
            break;
    }
}


/* Returns a color for the MPS in scsSpatData.output_copy.final_event_state[i] */
char_t * i2v_movement_phase_state_to_traffic_light_color_string(i2vMovementPhaseState mps, bool_t mention_flashing)
{
    switch(mps) {
        case MPS_UNAVAILABLE:
        case MPS_DARK:
            return("Dark");
            break;
        case MPS_STOP_THEN_PROCEED:
            return (mention_flashing ? "Flashing Red" : "Red");
            break;        
        case MPS_STOP_AND_REMAIN:
            return("Red");
            break;        
        case MPS_PRE_MOVEMENT:
            return("Red+Yellow");
            break;        
        case MPS_PERMISSIVE_MOVEMENT_ALLOWED:
            return (mention_flashing ? "Flashing Yellow" : "Yellow");
            break;  
        case MPS_PROTECTED_MOVEMENT_ALLOWED:
            return("Green");
            break;  
        case MPS_PERMISSIVE_CLEARENCE:
        case MPS_PROTECTED_CLEARENCE:
            return("Yellow");
            break;
        case MPS_CAUTION_CONFLICTING_TRAFFIC:
            return (mention_flashing ? "Flashing Yellow" : "Yellow");
            break;     
        default:
            return("Dark");
            break;
    }
}


/*
 * i2v_signal_phase_to_movement_phase_state()
 *
 * Translates the TSCBM signal phase to a J2735 Movement Phase State, 
 * based on the TSCBM signal phase, the lane's GreanMeans value, and the
 * lane's previous movement phase state value.
 *
 * Not moved into the specific one app that uses it so unit testing can more easily test it
 *
 */
i2vMovementPhaseState i2v_signal_phase_to_movement_phase_state(i2vSigPhases sigPhase, uint8_t channelGreenType, i2vMovementPhaseState prior_movement_phase_state)
{
    switch(sigPhase){
        case SIG_PHASE_DARK:
            return MPS_DARK;
            break;
        case SIG_PHASE_RED:
            return MPS_STOP_AND_REMAIN;
            break;       
        case SIG_PHASE_FLASHING_RED:
            return MPS_STOP_THEN_PROCEED;
            break;
        case SIG_PHASE_UNKNOWN:
            return MPS_UNAVAILABLE;
            break;

        case SIG_PHASE_GREEN:
        case SIG_PHASE_FLASHING_GREEN:
            /* Based on GreenMeans column in conf tscbm.conf. */
            return channelGreenType;
            break;

        // Return one of two CLEARANCE phase states based on prior movement phase state's permission
        case SIG_PHASE_YELLOW: 
            /* Based on prior movementPhaseState. */
            // If previous was permissive, this yellow is permissive
            if (prior_movement_phase_state == MPS_PERMISSIVE_MOVEMENT_ALLOWED ||
                prior_movement_phase_state == MPS_PERMISSIVE_CLEARENCE) {
                return MPS_PERMISSIVE_CLEARENCE;
            // If previous was not permissive, normally could assume we return MPS_PROTECTED_CLEARENCE,
            // but to handle the edge case of a red/dark light turning steady yellow in a
            // GreenMeans=permitted lane, we use channelGreenType to choose the clearance phase state.
            } else {
                return (channelGreenType == MPS_PROTECTED_MOVEMENT_ALLOWED)
                    ? MPS_PROTECTED_CLEARENCE
                    : MPS_PERMISSIVE_CLEARENCE;
            }
            break;

        case SIG_PHASE_FLASHING_YELLOW:
            return MPS_PERMISSIVE_MOVEMENT_ALLOWED;
            break;

        default:
            break;
    }
    return MPS_UNAVAILABLE;
}

// Returns if phase is a flashing phase or not
bool_t is_flashing_phase(uint32_t cur_phase)
{
    return (cur_phase == SIG_PHASE_FLASHING_GREEN
        ||  cur_phase == SIG_PHASE_FLASHING_YELLOW
        ||  cur_phase == SIG_PHASE_FLASHING_RED);
}

#ifdef HSM
/*
 * Converts a hex digit to a byte
 */
uint8_t fromHexDigit(char_t digit)
{
    if ((digit >= '0') && (digit <= '9')) {
        return digit - '0';
    }
    else if ((digit >= 'A') && (digit <= 'F')) {
        return digit - 'A' + 0xA;
    }
    else if ((digit >= 'a') && (digit <= 'f')) {
        return digit - 'a' + 0xA;
    }

    return 0;
}
#endif

void i2vUtilLoadDeviceID(cfgItemsT *cfg)
{
    FILE *pf;
    uint8_t idx;

    if (!cfg->i2vDeviceIDLoaded) {
        memset(cfg->i2vDeviceID, 0, I2VRSUIDSIZE);
        /* black magic: the customer device ID is the second 32 bytes at the start of /dev/mtd9 */
        if (NULL != (pf = popen("hexdump -e ' 16/1 \"%02x \" \"\n\" ' /dev/mtd9 | tail -3 | head -2", "r"))) {
            for (idx = 0; idx < I2VRSUIDSIZE; idx++) {
                /* get hex value and ignore whitespace (or newline) */
                if( fscanf(pf, "%02hhx ", &(cfg->i2vDeviceID[idx])) < 1) {
                    #if defined(EXTRA_DEBUG)
                    I2V_DBG_LOG(UTIL_ERR_LEVEL,UTIL_NAME,"i2vUtilLoadDeviceID: fscanf fail. errno=%d.\n",errno);
                    #endif
                }
            }
            pclose(pf);
            cfg->i2vDeviceIDLoaded = WTRUE;
        } else {
            /* reset indication to use device id; not loaded */
            cfg->i2vUseDeviceID = WFALSE;
        }
    }
}

/* Need I2VRSUIDSIZE + input length + 4 bytes for epoch */
int32_t i2vUtilTaggingWrite(int32_t fd, void *buf, size_t length, cfgItemsT *cfg)
{
    uint8_t *data;
    wtime epoch;
    uint32_t realepoch;
    int32_t size;

    if (!cfg->i2vUseDeviceID) {
        return (write(fd, buf, length));
    }

    if (NULL == (data = calloc(1, length + I2VRSUIDSIZE + sizeof(unsigned int)))) {
        return 0;
    }

    memcpy(data, cfg->i2vDeviceID, I2VRSUIDSIZE);
    epoch = (wtime)(i2vUtilGetTimeInMs()/1000); /* wtime tracks msecs as well; don't ship that; just epoch */
    realepoch = (unsigned int)epoch;   /* real epoch fits in 4 bytes */
    memcpy(data + I2VRSUIDSIZE, &realepoch, sizeof(unsigned int));
    memcpy(data + I2VRSUIDSIZE + sizeof(unsigned int), buf, length);
    size = write(fd, data, length + I2VRSUIDSIZE + sizeof(wtime));
    free(data);

    return size;
}

