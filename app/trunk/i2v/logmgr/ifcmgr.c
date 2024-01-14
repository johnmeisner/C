/**************************************************************************
 *                                                                        *
 *     File Name:  ifcmgr.c                                               *
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
#else
#define POPEN popen
#define PCLOSE pclose
#endif

/* mandatory logging defines */
#if defined(MY_UNIT_TEST)
#define MY_ERR_LEVEL   LEVEL_DBG
#else
#define MY_ERR_LEVEL   LEVEL_PRIV
#endif
#define MY_NAME        "ifcmgr"

#if defined(EXTRA_DEBUG)
#define OUTPUT_MODULUS  30  /* Seconds. */
#else
#define OUTPUT_MODULUS  1200
#endif

#define DEFAULT_TIMEOUT  1000000   /* 1 sec for usleep */
#define MIN_TIMEOUT      506
#define MAX_TIMEOUT      1000000   /* 1 sec - longer will slow down i2v termination on exit */

/*
 * Externs
 */
extern void set_my_error_state(int32_t my_error);

/*
 * Globals
 */
extern bool_t mainloop;
extern lconfig lmgrCfg;

extern ifcControl masterIfcReq[I2V_NUM_SUPPORTED_IFCS];
extern pthread_mutex_t ifcreqlock;
extern pid_t zprocs[MAX_CHILD_PROCS];  /* for child cleanup - prevent zombies */
extern uint32_t curtime_seconds;

/* 1218 MIB: 5.8 rsuInterfaceLogs. */
extern interfaceLog_t interfaceLogTable[RSU_IFCLOG_MAX];
STATIC interfaceLog_t * shm_interfaceLog_ptr = NULL; /* SHM. */
uint8_t  ifclogoptions = 0x0;

extern uint32_t current_logdir_size;

extern int32_t ifcLogDirection;
extern char_t  rsuname[RSU_ID_LENGTH_MAX+1];

/*
 * Functions
 */
#if !defined(ENABLE_PEER_IFC_REQUESTS)
static int32_t sendifcrequest(lmgrIfcReqState request, i2vIfcList ifc)
{
  int32_t ret = -1;
  int32_t i = 0;
 
  for(i=0;i < 10;i++){
      if(0 == (ret = pthread_mutex_trylock(&ifcreqlock))) {
          for (i=0; i<I2V_NUM_SUPPORTED_IFCS; i++) {
              if (!masterIfcReq[i].reqPending) {
                  #if defined(EXTRA_DEBUG) 
                  I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"sendifcrequest:i=%i ifc(%d)req(%d)\n",i,ifc,request);
                  #endif
                  masterIfcReq[i].reqPending = WTRUE;
                  masterIfcReq[i].ifc = ifc;
                  masterIfcReq[i].request = request;
                  break;
              }
          }
          ret = pthread_mutex_unlock(&ifcreqlock);
          break;
      } else {
        usleep(550);
      }
  }
  if(10 <= i) {
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"sendifcrequest: Failed.\n");
      set_my_error_state(LOGMGR_IFC_SEND_REQ_FAIL);
  }
//what if the request fails? just post it?
  return ret;
}
int32_t readifcrequest(ifcControl * thisreq)
{
  int32_t ret = -1;
  int32_t i = 0;
 
  for(i=0;i < 10;i++){
      if(0 == (ret = pthread_mutex_trylock(&ifcreqlock))) {
          for (i=0; i<I2V_NUM_SUPPORTED_IFCS; i++) {
              if (masterIfcReq[i].reqPending) {
                  #if defined(EXTRA_DEBUG) 
                  I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"readifcrequest:i=%d ifc(%d)req(%d)\n",i,masterIfcReq[i].ifc,masterIfcReq[i].request);
                  #endif
                  thisreq->reqPending = masterIfcReq[i].reqPending;
                  thisreq->ifc        = masterIfcReq[i].ifc;
                  thisreq->request    = masterIfcReq[i].request;
                  memset(&masterIfcReq[i], 0, sizeof(ifcControl));
                  break;
              }
          }
          ret = pthread_mutex_unlock(&ifcreqlock);
          break;
      } else {
        usleep(550);
      }
  }
  if(10 <= i) {
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"readifcrequest: Failed.\n");
      set_my_error_state(LOGMGR_IFC_READ_REQ_FAIL);
  }
  return ret;
}
#endif
static bool_t lmgrStartLogger(const i2vIfcList ifc, pid_t *pid)
{
#define RELEASE_START_ARGS \
    for (i=3;i<16;i++) free(cmd[i]);
    /* remove the -s option below if RSU requirement is clarified to require
       csv file that logs all data (and obviously adjust indexes) */
    char_t *cmd[17] = {"ifclogger_app", "-s", "-i"};
    uint8_t i;
    char_t cmdname[] = "/usr/bin/ifclogger_app";
    int32_t  fd = -1;

    /* allocate space for args */
    cmd[3] = (char_t *)malloc(6);   /* eth or dsrc0 or cv2x0 */
    cmd[4] = (char_t *)malloc(3);   /* -n */  /* If the name has the number why do we need this? */
    cmd[5] = (char_t *)malloc(2);   /* 0 or 1 */
    cmd[6] = (char_t *)malloc(3);   /* -t */
    cmd[7] = (char_t *)malloc(TRACKER_LENGTH);
    cmd[8] = (char_t *)malloc(16);
    cmd[9] = (char_t *)malloc(RSU_ID_LENGTH_MAX+1);
    cmd[10] = (char_t *)malloc(16);
    cmd[11] = (char_t *)malloc(16);
    cmd[12] = (char_t *)malloc(16);
    cmd[13] = (char_t *)malloc(16);
    cmd[14] = (char_t *)malloc(16);
    cmd[15] = (char_t *)malloc(16);
    cmd[16] = (char_t *)malloc(16);

    strcpy(cmd[4], "-n");
    strcpy(cmd[6], "-t");
    strcpy(cmd[8],"-N");
    memcpy(cmd[9],rsuname,RSU_ID_LENGTH_MAX+1); /* Nothing says it has to be terminated so be careful. */
    strcpy(cmd[10],"-S");
    sprintf(cmd[11],"%d", lmgrCfg.ifcLogSizeMax); /* Meg */
    strcpy(cmd[12],"-T");
    sprintf(cmd[13],"%d", lmgrCfg.ifcLogDuration/3600); /* Hours */
    strcpy(cmd[14],"-D");
    sprintf(cmd[15],"%d", ifcLogDirection); /* in,out,both,seperate */
    cmd[16] = (char_t *)0;

    switch (ifc) {
        case PHY_ETH0:
            strcpy(cmd[3], "eth0");
            strcpy(cmd[5], "0");
            strncpy(cmd[7], ETH0_IFC_TRACKER,TRACKER_LENGTH);
            break;
        case PHY_ETH1:
            strcpy(cmd[3], "eth1");
            strcpy(cmd[5], "1");
            strncpy(cmd[7], ETH1_IFC_TRACKER,TRACKER_LENGTH);
            break;
        case PHY_DSRC0:
            strcpy(cmd[3], "dsrc0");
            strcpy(cmd[5], "0"); /* don't care. */
            strncpy(cmd[7], DSRC0_IFC_TRACKER,TRACKER_LENGTH);
            break;
        case PHY_DSRC1:
            strcpy(cmd[3], "dsrc1");
            strcpy(cmd[5], "1"); /* don't care. */
            strncpy(cmd[7], DSRC1_IFC_TRACKER,TRACKER_LENGTH);
            break;
        case PHY_CV2X0:
            strcpy(cmd[3], "cv2x0");
            strcpy(cmd[5], "0"); /* don't care. */
            strncpy(cmd[7], CV2X0_IFC_TRACKER,TRACKER_LENGTH);
            break;
        case PHY_CV2X1:
            strcpy(cmd[3], "cv2x1");
            strcpy(cmd[5], "1"); /* don't care. */
            strncpy(cmd[7], CV2X1_IFC_TRACKER,TRACKER_LENGTH);
            break;
        default:
            *pid = -1;
            set_my_error_state(LOGMGR_IFC_START_LOG_PARAM_FAIL);
            RELEASE_START_ARGS
            return WFALSE;
    }
    #ifdef EXTRA_DEBUG
    for(i=0;i<17;i++){
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"lmgrStartLogger: cmd[%d][%s]\n",i,cmd[i]);
    }
    #endif
    if ((*pid = fork()) == -1) {
        RELEASE_START_ARGS
        set_my_error_state(LOGMGR_FORK_FAIL);
        return WFALSE;
    } else if (*pid == 0) {
        if (open(cmdname, O_RDONLY) == -1) {
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Cannot run cmd %s (%s-%d)\n", cmdname, strerror(errno), errno);
            exit(-1);
        }
        if ((fd=open("/dev/null", O_RDWR)) != -1) {
            dup2(fd, 0);
            dup2(fd, 1);  /* suppress all console output from child */
            dup2(fd, 2);  /* suppress all console output from child */
        }
        if (execvp(cmdname, (char_t *const *)cmd) == -1) {
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"error starting ifc logging\n");
            set_my_error_state(LOGMGR_START_IFC_FAIL);
            return WFALSE;
        } 
    }

    #ifdef EXTRA_DEBUG
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"lmgr created child thread %d\n", *pid);
    #endif
    RELEASE_START_ARGS

    return WTRUE;

#undef RELEASE_START_ARGS
}
/* ifc log child process terminator - the child process
   may take some time exiting, so we use waitpid to ensure
   that if there are any delays exiting it doesn't go to 
   zombie state if this process closes first - this also
   allows us to start a new process with the pid tracking
   variable */
static void lmgrTermLogger(pid_t *pid, bool_t force)
{
    union sigval val;
    int32_t stat;
    uint8_t i;

    val.sival_int = 0;
    if (force) {
        sigqueue(*pid, SIGKILL, val);
        waitpid(*pid, &stat, WNOHANG);
    } else {
        sigqueue(*pid, SIGTERM, val);
        for (i=0; i<MAX_CHILD_PROCS; i++) {
            if (zprocs[i] == -1) {
                zprocs[i] = *pid;
                break;
            }
        }
    }

    *pid = -1;  /* reset it so that it can be reused */
}
/* returns I2V_RETURN_NO_ACTION if no need to start new log 
   returns I2V_RETURN_OK if need to start new log

   assumes input filename is a soft link that can be
   dereferenced to obtain required info

   buffer.st_atime seems to stay the same even while task is writing to it so good enough.
   We cant stop people from accessing the file so....
*/
static i2vReturnTypesT lmgrIfcLogSizeandAge(const char_t *filename)
{
    uint32_t maxfilesize = (uint32_t)(lmgrCfg.ifcLogSizeMax * 1024 * 1024);
    uint32_t curtime;
    struct stat buffer;
    int32_t status;

    /* 
     * First, check to see if file exists now. 
     * There can be a few cycles of delay for that to happen.
     * Could put a threashold on number waits before flagging error?
     */
    memset(&buffer,0x0,sizeof(buffer));
    if(0 == (status = stat(filename, &buffer))) {

        /* File is too large so reject it. */
        if (maxfilesize < buffer.st_size) {
            #ifdef EXTRA_DEBUG
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"lmgr found ifc log (%s) size is too large(%ld>=%d)\n", filename,buffer.st_size,maxfilesize);
            #endif
            return I2V_RETURN_OK;
        }
    } else {
        /*  Doesn't exist yet. Seems like a few of these can get out before file is created. */
        set_my_error_state(LOGMGR_IFC_CHECK_SIZEAGE_NOFILE); 
        return I2V_RETURN_NO_ACTION;
    }

    /* 
     * We have passed existence and size check. Now check age.
     */
    curtime = curtime_seconds;
    /* Be really careful enabling this. Will spew at high rate. */
    #if defined(EXTRA_EXTRA_DEBUG)
    if(0 == (logmgr_err_count % 10000)){ /* Loop runs at ~1Khz so watch out. */
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"ifclog(%s): size=%d, max=%d,remain=%d.\n", filename, buffer.st_size, maxfilesize, maxfilesize - buffer.st_size);
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"ifclog    : UTC=%u: age=%u, max=%u.\n",curtime, (curtime - buffer.st_atime), lmgrCfg.ifcLogDuration);
    }
    logmgr_err_count++;
    #endif
    
    /*  
     * Check file age. Make sure clock hasn't jumped backwards.
     * Once again maybe need threshold of attempts before we flag an error. 
     */
    if (curtime < buffer.st_atime) {
        #if defined(EXTRA_DEBUG)
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"lmgr ifc log (%s) clock jump into past (%d < %ld)\n", filename, curtime, buffer.st_atime);
        #endif
        set_my_error_state(LOGMGR_IFC_CHECK_SIZEAGE_TIME_ERROR);
    }  else {

        /* Too old. */
        if ((curtime - buffer.st_atime) > lmgrCfg.ifcLogDuration) {
            #if defined(EXTRA_DEBUG)
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"lmgr ifc log (%s) age is too old (%d != %ld)\n", filename, curtime, buffer.st_atime);
            #endif
            set_my_error_state(LOGMGR_IFC_CHECK_SIZEAGE_TOO_OLD);
            return I2V_RETURN_OK;
        }
    }
    /* It exists, not too big and not too old. Just right. */
    return I2V_RETURN_NO_ACTION;
}
int32_t getifclogoldest(char_t * filename)
{
#define GET_OLDEST_FILE        "ls -rt %s 2>/dev/null | head -1"
#define GET_OLDEST_FILE_REPLY  "%s"

  int32_t   i = 0;
  FILE    * fp = NULL;
  char_t    replyBuf[512]; /* Size according to your needs. */
  int32_t   ret = 0;
  char_t    cmd[512];
  int32_t   result = 0;

  memset(replyBuf,'\0',sizeof(replyBuf));
  memset(cmd,'\0',sizeof(cmd));
  if(NULL == filename) {
      I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"getifclogoldest: NULL input.\n");
      set_my_error_state(LOGMGR_NULL_INPUT);
      ret = -1;
  }
  if((0 == ret) && (0 >= (result = snprintf(cmd,sizeof(cmd),GET_OLDEST_FILE,IFC_COMPLETED_LOGS_DIR)))) {
      I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"getifclogoldest: SPRINTF_FAIL ret(%d) cmd(%s)\n",result,cmd);
      set_my_error_state(LOGMGR_SPRINTF_FAIL); 
      ret = -2;
  }
  if((0 == ret) && (NULL == (fp = popen(cmd, "r")))) {
      I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"getifclogoldest: POPEN_FAIL.\n");
      set_my_error_state(LOGMGR_POPEN_FAIL); 
      ret = -3;

  }
  if((0 == ret) && (NULL == fgets(replyBuf, sizeof(replyBuf), fp))){
      I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"getifclogoldest: FGETS_FAIL.\n");
      set_my_error_state(LOGMGR_FGETS_FAIL);
      ret = -4;
  }
  if(0 == ret) {
      i = strnlen(replyBuf,sizeof(replyBuf));
      if((0<i) && (i <= (int32_t)sizeof(replyBuf))){
          replyBuf[i-1] = '\0'; /* delete extra char added. */
      } else { 
          replyBuf[0] = '\0'; /* force scanf to fail */
      }
  }
  if((0 == ret) && (1 != sscanf(replyBuf,GET_OLDEST_FILE_REPLY,filename))) {
      I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"getifclogoldest: SSCANF_FAIL.\n");
      set_my_error_state(LOGMGR_SCANF_FAIL);
      ret = -5;
  }
  if(NULL != fp) {
      pclose(fp);
  }
  #if defined(EXTRA_DEBUG)
  if(0 == ret) {
      I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"getifclogoldest(%s)\n",filename);
  }
  #endif
  return ret;
#undef GET_OLDEST_FILE
#undef GET_OLDEST_FILE_REPLY
}
/* In bytes */
int32_t getifclogdirsize(uint32_t * dirsize)
{
#if defined(MY_UNIT_TEST)
#define GET_DIR_SIZE        "du --bytes /tmp/rwflash/I2V/ifclogs"
#else
#define GET_DIR_SIZE        "du --bytes /rwflash/I2V/ifclogs"
#endif
#define GET_DIR_SIZE_REPLY  "%u"

  int32_t   i = 0;
  FILE    * fp = NULL;
  char_t    replyBuf[64]; /* Size according to your needs. */
  int32_t   ret = -1;     /* Fail till proven success. */

  if(NULL != dirsize) { 
      memset(replyBuf,'\0',sizeof(replyBuf));
      if(NULL != (fp = popen(GET_DIR_SIZE, "r"))) {
          if(NULL != fgets(replyBuf, sizeof(replyBuf), fp)){
              i = strnlen(replyBuf,sizeof(replyBuf));
              if((0<i) && (i <= (int32_t)sizeof(replyBuf))){
                  replyBuf[i-1] = '\0'; /* delete extra char added. */
              } else { 
                  replyBuf[0] = '\0'; /* force scanf to fail */
              }
              if(1 == sscanf(replyBuf,GET_DIR_SIZE_REPLY,dirsize)) {
                  #if defined(EXTRA_DEBUG)
                  I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"ifclogdirsize=%u\n",*dirsize);
                  #endif
                  ret = 0; /* success */
              } else {
                  I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"getifclogdirsize: SSCANF_FAIL.\n");
                  set_my_error_state(LOGMGR_SCANF_FAIL);              }
          } else {
              I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"getifclogdirsize: FGETS_FAIL.\n");
              set_my_error_state(LOGMGR_FGETS_FAIL); 
          }
          pclose(fp);
      } else {
          I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"getifclogdirsize: POPEN_FAIL.\n");
          set_my_error_state(LOGMGR_POPEN_FAIL); 
      }
  } else {
      I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"getifclogdirsize: NULL input.\n");
      set_my_error_state(LOGMGR_NULL_INPUT);  
  }
  return ret;
#undef GET_DIR_SIZE
#undef GET_DIR_SIZE_REPLY
}
int32_t getifclogcount(void)
{
#if defined(MY_UNIT_TEST)
#define GET_DIR_FILE_COUNT        "/bin/ls /tmp//rwflash/I2V/ifclogs | wc -l"
#else
#define GET_DIR_FILE_COUNT        "/bin/ls /rwflash/I2V/ifclogs | wc -l"
#endif
#define GET_DIR_FILE_COUNT_REPLY  "%d"

  int32_t   i = 0;
  FILE    * fp = NULL;
  char_t    replyBuf[64]; /* Size according to your needs. */
  int32_t   filecount = -1; /* Fail till success. */

  memset(replyBuf,'\0',sizeof(replyBuf));
  if(NULL != (fp = popen(GET_DIR_FILE_COUNT, "r"))) {
      if(NULL != fgets(replyBuf, sizeof(replyBuf), fp)){
          i = strnlen(replyBuf,sizeof(replyBuf));
          if((0<i) && (i <= (int32_t)sizeof(replyBuf))){
              replyBuf[i-1] = '\0'; /* delete extra char added. */
          } else { 
              replyBuf[0] = '\0'; /* force scanf to fail */
          }
          if(1 == sscanf(replyBuf,GET_DIR_FILE_COUNT_REPLY,&filecount)) {
              #if defined(EXTRA_DEBUG)
              I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"filecount=%d\n",filecount);
              #endif
              if(1024 < filecount){
                  I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"getifclogcount: 1024 < filecount(%d). Limit to 1024.\n",filecount);
                  filecount = 1024; 
                  set_my_error_state(LOGMGR_IFCLOG_TOO_MANY_FILE);
              }
          } else {
              I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"getifclogcount: SSCANF_FAIL.\n");
              set_my_error_state(LOGMGR_SCANF_FAIL);
          }
      } else {
          I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"getifclogcount: FGETS_FAIL.\n");
          set_my_error_state(LOGMGR_FGETS_FAIL);
      }
      pclose(fp);
  } else {
      I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"getifclogcount: POPEN_FAIL.\n");
      set_my_error_state(LOGMGR_POPEN_FAIL);
  }
  return filecount;
#undef GET_DIR_FILE_COUNT
#undef GET_DIR_FILE_COUNT_REPLY
}
//TODO: split tcmpdump and capture_pcap into respective interface folders, then by direction.

//    : clean & reserve space you need before you start logging.
//  
//    : I guess always a problem since both will duke it out for file space.
//    : Therefore, if it was just one thread then we could check on for max dir size
//    : But once we have two threads we can do a one time reserver but cant gurantee that space once 2nd smaller og comes in.
//    : That second smaller thread will not clear more space since its under then consume it itself
//    : Onr needs a registered size too so we can compare size of dir versus rreserved space
//    : Then the smaller 2nd task knows to reserve mre space based on the number.
//    : Once all space reserved then no new thread can start? 

//TODO: test stop/start
/* Handles any file in dir. We dont care what the prefix is just clean it up. */
/* requestsize is extra room below max allowed needed. */
/* returns two values : 0 == AOK, 2 == stop  logging */
int32_t ifclogCheckDirSize(uint32_t requestsize)
{
    char_t     cmd[1024]; /* More than enough, adjust for your needs. */
    uint32_t   max_conf_logdir_size = 0;
    //uint32_t   threshold = 0;
    uint32_t   max_logdir_size = 0x0;
    int32_t    filecount = 0;
    int32_t    i = 0;
    char_t     oldestfilename[512];
    int32_t    ret = 0; /* Success till fail */

    /* Set once, convert to bytes. */
    max_conf_logdir_size = lmgrCfg.dirsize * 1024 * 1024;
    //threshold            = lmgrCfg.threshold * 1024 * 1024;
    requestsize          = requestsize * 1024 * 1024;
    max_logdir_size      = max_conf_logdir_size - requestsize;

    /* Get file count, if no files nothing to do. */
    if (0 > (filecount = getifclogcount())) {
        ret = 2; /* Halt logging. */
    }
    for(i=0;(0 == ret) && (i < filecount);i++) {
        memset(cmd,'\0',sizeof(cmd));
        if(0 <= getifclogdirsize(&current_logdir_size)){
            #if defined(EXTRA_DEBUG)
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"ifclog directory: size(%u) >= max(%u:%u,%u) file(%d) attempt(%d) options(0x%x)\n",current_logdir_size ,max_logdir_size,max_conf_logdir_size,requestsize,filecount,i,ifclogoptions);
            #endif
            if (current_logdir_size >= max_logdir_size) {
                memset(oldestfilename,'\0',sizeof(oldestfilename));
                if(0 == getifclogoldest(oldestfilename)) { /* Get oldest. */
                    if(0x0 == (RSU_IFCLOG_OPTIONS_DISK_FULL & ifclogoptions)) {
                        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"ifclogCheckDirSize: output folder full. Halt logging.\n"); 
                        ret = 2; /* If full and not enabled to delete then halt logging. */
                    } else {
                        snprintf(cmd,sizeof(cmd), "rm -f %s/%s",IFC_COMPLETED_LOGS_DIR, oldestfilename);
                        if(0 != system(cmd)){
                            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"ifclog rm failed: cmd[%s]. errno=%d.\n",cmd,errno);
                            set_my_error_state(LOGMGR_SYSTEM_FAIL);
                            ret = 2;
                        }
                        /* Keep going till under the limit */
                    }
                } else {
                    ret = 2; /* Get oldest file failed. Halt logging now. */
                }
            } else {
                ret = 0;
                break; /* Under threshold needed for request size. */
            }
        } else {
            ret = 2; /* Get dir size failed. Halt logging now */
        }
    } /* for */
    if(1024 <= i) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"ifclogCheckDirSize: Exhausted cleanp up attempts=%d. Log will be truncated in size.\n",i);
    }
    return ret;
}
/* interface log handler thread  */
//TODO: Add support for: IFC_IN_ON, IFC_IN_OFF, IFC_OUT_ON, IFC_OUT_OFF. 
//TODO: Add support for concurrent logs on ifc.
/* NOTE: this implementation can only handle one log on an ifc at a time.
 *     : capture_pcap can only handle one at a time logging.
 *     : tcmpdump may be able to handle more than one. Not sure.
 */
void *lmgrIfcLogHandlerThr(void __attribute__((unused)) *arg)
{
    /* each ifc can be logged and MUST be logged in separate files -
       therefore create a separate thread for each ifc requested */
    pid_t eth0 = -1, eth1 = -1;   
    pid_t dsrc0 = -1, dsrc1 = -1, cv2x0 = -1, cv2x1 = -1;
    ifcControl thisreq;
    uint32_t ret = 0, timeout = DEFAULT_TIMEOUT;
    i2vIfcList ifcv = PHY_NONE;
    pid_t *pidp = NULL;
    bool_t *flagp = NULL;
    bool_t eth0flag, eth1flag;
    bool_t dsrc0flag, dsrc1flag, cv2x0flag, cv2x1flag;
    uint8_t i;
    char_t tracker[TRACKER_LENGTH];
    int32_t  pidret;
    char_t  ifaceName[RSU_INTERFACE_NAME_MAX];

    memset(zprocs, -1, sizeof(zprocs));

    for(i=0;i<5;i++){
         sleep(1); /* IFCLOG is not critical at boot. */
    }
    /* check for start/stop indications and then file requirements */
    while (mainloop) {
        usleep(timeout); /* may be merit in fine tuning if requests queue up but today that cant happen. */
        /* child process cleanup */
        for (i=0; i<MAX_CHILD_PROCS; i++) {
            if (zprocs[i] != -1) {
                if (0 < waitpid(zprocs[i], &pidret, WNOHANG)) {
                    if (WIFEXITED(pidret)) {
                        zprocs[i] = -1;
                    }
                }
            }
        }
        memset(&thisreq, 0, sizeof(thisreq));
#if defined(ENABLE_PEER_IFC_REQUESTS)
        while (pthread_mutex_lock(&ifcreqlock) != 0) {
            usleep(560);
        }
        /* NOTE about handling below: this handling opens up the possibility for
           a secondary request on an ifc to be seen before the initial request if 
           the requests come fast enough - in this case it's a user being stupid
           and trying to stress the system (turning logging on and off like this
           is impractical) - so for now the implementation will not protect the
           user from the user's own mischievousness */
        for (i=0; i<I2V_NUM_SUPPORTED_IFCS; i++) {
            if (masterIfcReq[i].reqPending) {
                #ifdef EXTRA_DEBUG
                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"ifc thread found pending request\n");
                #endif
                memcpy(&thisreq, &masterIfcReq, sizeof(thisreq));
                memset(&masterIfcReq[i], 0, sizeof(ifcControl));
                if (i<I2V_NUM_SUPPORTED_IFCS - 1) {
                    timeout = 1000;   /* there may be more pending requests, need a quick timeout */
                }
                break;
            } else {
                memset(&masterIfcReq[i], 0, sizeof(ifcControl));
            }
        }
        pthread_mutex_unlock(&ifcreqlock);
#else
        if(0 != readifcrequest(&thisreq)) {
            set_my_error_state(LOGMGR_IFC_REQ_FAIL);
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"LOGMGR_IFC_REQ_FAIL: cant read req.\n");
            continue;
        }
#endif
        /* Validate we support this ifc. */
        if (thisreq.reqPending) {
            switch (thisreq.ifc) {
                case PHY_ETH0:  pidp = &eth0; flagp = &eth0flag;
                    strcpy(ifaceName,"eth0");
                    break;
                case PHY_ETH1:  pidp = &eth1; flagp = &eth1flag;
                    strcpy(ifaceName,"eth1");
                    break;
                case PHY_DSRC0:  pidp = &dsrc0; flagp = &dsrc0flag;
                    strcpy(ifaceName,"dsrc0");
                    break;
                case PHY_DSRC1:  pidp = &dsrc1; flagp = &dsrc1flag;
                    strcpy(ifaceName,"dsrc1");
                    break;
                case PHY_CV2X0:  pidp = &cv2x0; flagp = &cv2x0flag;
                    strcpy(ifaceName,"cv2x0");
                    break;
                case PHY_CV2X1:  pidp = &cv2x1; flagp = &cv2x1flag;
                    strcpy(ifaceName,"cv2x1");
                    break;
                default:
                    #ifdef EXTRA_DEBUG
                    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"invalid interface for interface logging - ignoring\n");
                    #endif
                    set_my_error_state(LOGMGR_IFC_BAD_INTERFACE);
                    memset(&thisreq, 0, sizeof(thisreq));
                    break;
            }
        }
        /* Start ifclog. */
        if (thisreq.reqPending) {
            if ((thisreq.request == IFC_ON) && (*pidp == -1)) {
                /* 
                 * At start, check output folder remaining space versus request size.
                 * Based on rsuInterfaceLog options bits: 
                 * i)Delete oldest to make room for request.
                 * ii)Do nothing & halt. 
                 */
                if(0 != ifclogCheckDirSize(lmgrCfg.ifcLogSizeMax)) {
                    *flagp = WFALSE;
                    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"ifclogCheckDirSize(): Dont't start logging.\n");
                } else {
                    if (!lmgrStartLogger(thisreq.ifc, pidp)) {
                        /* set flag that tries to start logger */
                        #ifdef EXTRA_DEBUG
                        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"lmgr ifc thread activating log failed\n");
                        #endif
                        set_my_error_state(LOGMGR_IFC_REQ_THREAD_FAIL);
                        *flagp = WTRUE;
                    }
                }
            } else if (thisreq.request == IFC_OFF) {
                *flagp = WFALSE;
                if (*pidp != -1) {
                    lmgrTermLogger(pidp, WFALSE);
                }
            }
        } /* Done with thisreq. */

        /* Maintenace loop: check file size and age according to requirements */
        for (i=0; i<I2V_NUM_SUPPORTED_IFCS; i++) {   /* this switch should be num supported ifcs if ever additional radios added */
            switch (i) {
                case 0: pidp = &eth0; flagp = &eth0flag; ifcv = PHY_ETH0; strcpy(tracker, ETH0_IFC_TRACKER);
                    break;
                case 1: pidp = &eth1; flagp = &eth1flag; ifcv = PHY_ETH1; strcpy(tracker, ETH1_IFC_TRACKER);
                    break;
                case 2: pidp = &dsrc0; flagp = &dsrc0flag; ifcv = PHY_DSRC0; strcpy(tracker, DSRC0_IFC_TRACKER);
                    break;
                case 3: pidp = &dsrc1; flagp = &dsrc1flag; ifcv = PHY_DSRC1; strcpy(tracker, DSRC1_IFC_TRACKER);
                    break;
                case 4: pidp = &cv2x0; flagp = &cv2x0flag; ifcv = PHY_CV2X0; strcpy(tracker, CV2X0_IFC_TRACKER);
                    break;
                case 5: pidp = &cv2x1; flagp = &cv2x1flag; ifcv = PHY_CV2X1; strcpy(tracker, CV2X1_IFC_TRACKER);
                    break;
            }
            if (*pidp != -1) {
                if (I2V_RETURN_OK == (ret = lmgrIfcLogSizeandAge(tracker))) {
                    lmgrTermLogger(pidp, WFALSE);  /* this function resets eth0 so can start a new logging function now */
#if defined(EXTRA_DEBUG)
                    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Time to close[%s] interface[%s]\n",tracker,ifaceName);
#endif
                    for(i=0;i<RSU_IFCLOG_MAX;i++){
                        if(0 == memcmp(interfaceLogTable[i].ifaceName, (uint8_t *)ifaceName, strlen(ifaceName))){
#if defined(EXTRA_DEBUG)
                            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"interfaceLogTable[%d][%s].ifaceGenerate to off.\n",i,ifaceName);
#endif
                        } else {
                            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"interfaceLogTable[%d][%s] != [%s]\n",i,interfaceLogTable[i].ifaceName,ifaceName);
                        }
                    }

//TODO: fail is fail why retry?
#define ENABLE_IFCLOG_FOREVER
#if defined(ENABLE_IFCLOG_FOREVER)
                    if(0 != ifclogCheckDirSize(lmgrCfg.ifcLogSizeMax)) {
                        *flagp = WFALSE;
                        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"ifclogCheckDirSize(): Halt logging.\n");
                    } else {
                        if (!lmgrStartLogger(ifcv, pidp)) {
                            /* failed to start logging - need a short timeout to try again */
                            #ifdef EXTRA_DEBUG
                            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"lmgr ifc thread re-activating log failed\n");
                            #endif
                            set_my_error_state(LOGMGR_IFC_REQ_THREAD_FAIL);
                            *flagp = WTRUE;
                            timeout = 1000;
                        }
                    }
#endif
                } else if (ret == I2V_RETURN_FAIL) {
//TODO: this function does not return this value. Do we really care if not OK?
                    /* operation didn't complete - need a short timeout to check again */
                    timeout = 1000;
                }
            } else if (*flagp) {
                /* a previous attempt to start logging on ifc failed - need to try again */
                if (lmgrStartLogger(ifcv, pidp)) {
                    *flagp = WFALSE;
                } else {
                    timeout = 1000;
                }
                set_my_error_state(LOGMGR_IFC_REQ_THREAD_FAIL);
            }
        } /* End of maintenace loop. */
    }/* while(mainloop). */

    /* cleanup before exiting */
    if (eth0 != -1) lmgrTermLogger(&eth0, WFALSE);
    if (eth1 != -1) lmgrTermLogger(&eth1, WFALSE);
    if (dsrc0 != -1) lmgrTermLogger(&dsrc0, WFALSE);
    if (dsrc1 != -1) lmgrTermLogger(&dsrc1, WFALSE);
    if (cv2x0 != -1) lmgrTermLogger(&cv2x0, WFALSE);
    if (cv2x1 != -1) lmgrTermLogger(&cv2x1, WFALSE);

    #ifdef EXTRA_DEBUG
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"lmgr interface log thread exiting\n");
    #endif
    pthread_exit(NULL);
}


/* 
 * TRAC #2782: RSU:HERC:IFCLOGGER: can only log one interface at a time.
 * cfg.InterfaceLogMask is a bitmask. We should be able to launch any combination of interfaces to log.
 */
void *InterfaceLogThread(void __attribute__((unused)) *arg)
{
  size_t shm_interfaceLog_size = sizeof(interfaceLog_t) * RSU_IFCLOG_MAX;
  int32_t i;
  uint32_t rolling_counter = 0x0;

  shm_interfaceLog_ptr = NULL;
  /* Open SHM. */
  if ((shm_interfaceLog_ptr = wsu_share_init(shm_interfaceLog_size, INTERFACE_LOG_SHM_PATH)) == NULL) {
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"interfaceLogThr: SHM init failed.\n");
      set_my_error_state(LOGMGR_INIT_SHM_FAIL);
  } else {
      memset(&interfaceLogTable[0],0x0,shm_interfaceLog_size);
      while(mainloop) {
          /* Check for updates in SHM table. Any change then first stop logging.*/
          for(i=0;i<RSU_IFCLOG_MAX;i++){
              /* check dirty field. If set & row active then need to update and start/stop/reset services. */
              if(shm_interfaceLog_ptr[i].dirty == 0x1) {
                  if(   (SNMP_ROW_ACTIVE == shm_interfaceLog_ptr[i].ifaceLogStatus)
                     && (mib_on == shm_interfaceLog_ptr[i].ifaceGenerate)){
                      if(0 == memcmp(shm_interfaceLog_ptr[i].ifaceName, "eth0", 4)){
                          /* Stop service. */
                          if(0 != sendifcrequest(IFC_OFF, PHY_ETH0)) {
                              set_my_error_state(LOGMGR_IFC_REQ_FAIL);
                              I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"LOGMGR_IFC_REQ_FAIL: eth0 off.\n");
                          }
                          if(0 != sendifcrequest(IFC_OFF, PHY_CV2X0)) {
                              set_my_error_state(LOGMGR_IFC_REQ_FAIL);
                              I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"LOGMGR_IFC_REQ_FAIL: cv2x0 off.\n");
                          }
                          sleep(IFC_START_WAIT_SECS); /* Ideally we get an ack or something before proceeding. */
                          /* Update from SHM. */
                          memcpy(&interfaceLogTable[i], &shm_interfaceLog_ptr[i],sizeof(interfaceLog_t));
                          lmgrCfg.ifcLogSizeMax = interfaceLogTable[i].ifaceMaxFileSize;
                          lmgrCfg.ifcLogDuration = (3600 * interfaceLogTable[i].ifaceMaxFileTime);
                          ifcLogDirection = interfaceLogTable[i].ifaceLogByDir;
                          ifclogoptions   = interfaceLogTable[i].ifaceLogOptions;
                          if(0 < interfaceLogTable[i].rsuID_length) {
                              if(interfaceLogTable[i].rsuID_length <= RSU_ID_LENGTH_MAX) {
                                  memcpy(rsuname,(char_t *)interfaceLogTable[i].rsuID, interfaceLogTable[i].rsuID_length);
                                  rsuname[interfaceLogTable[i].rsuID_length] = '\0';
                              } else {
                                  memcpy(rsuname,(char_t *)interfaceLogTable[i].rsuID, RSU_ID_LENGTH_MAX);
                                  rsuname[RSU_ID_LENGTH_MAX] = '\0';
                              }
                          } else {
                              strncpy(rsuname,"noname",sizeof(rsuname));
                          }
                          #if defined(EXTRA_DEBUG)
                          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"rsuname(%s)\n",rsuname);
                          #endif
                          interfaceLogTable[i].dirty = shm_interfaceLog_ptr[i].dirty = 0x0;
                          /* Start service and clear dirty bit. */
                          if(0 != sendifcrequest(IFC_ON, PHY_ETH0)) {
                              set_my_error_state(LOGMGR_IFC_REQ_FAIL);
                              I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"LOGMGR_IFC_REQ_FAIL: eth0 on.\n");
                          }
                          sleep(IFC_START_WAIT_SECS);
                          #if defined(EXTRA_DEBUG)
                          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"eth0 IFCLOG enabled.\n");
                          #endif
                      } 
                      if(0 == memcmp(shm_interfaceLog_ptr[i].ifaceName, "cv2x0", 5)){
                          /* Stop service. */
                          if(0 != sendifcrequest(IFC_OFF, PHY_ETH0)) {
                              set_my_error_state(LOGMGR_IFC_REQ_FAIL);
                              #if defined(EXTRA_DEBUG)
                              I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"LOGMGR_IFC_REQ_FAIL: eth0 off.\n");
                              #endif
                          }
                          if(0 != sendifcrequest(IFC_OFF, PHY_CV2X0)) {
                              set_my_error_state(LOGMGR_IFC_REQ_FAIL);
                              #if defined(EXTRA_DEBUG)
                              I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"LOGMGR_IFC_REQ_FAIL: cv2x0 off.\n");
                              #endif
                          }
                          sleep(IFC_START_WAIT_SECS);
                          /* Update from SHM. */
                          memcpy(&interfaceLogTable[i], &shm_interfaceLog_ptr[i],sizeof(interfaceLog_t));
                          lmgrCfg.ifcLogSizeMax = interfaceLogTable[i].ifaceMaxFileSize;
                          lmgrCfg.ifcLogDuration = (3600 * interfaceLogTable[i].ifaceMaxFileTime);
                          ifcLogDirection = interfaceLogTable[i].ifaceLogByDir;
                          ifclogoptions   = interfaceLogTable[i].ifaceLogOptions;
                          if(0 < interfaceLogTable[i].rsuID_length) {
                              if(interfaceLogTable[i].rsuID_length <= RSU_ID_LENGTH_MAX) {
                                  memcpy(rsuname,(char_t *)interfaceLogTable[i].rsuID, interfaceLogTable[i].rsuID_length);
                                  rsuname[interfaceLogTable[i].rsuID_length] = '\0';
                              } else {
                                  memcpy(rsuname,(char_t *)interfaceLogTable[i].rsuID, RSU_ID_LENGTH_MAX);
                                  rsuname[RSU_ID_LENGTH_MAX] = '\0';
                              }
                          } else {
                              strncpy(rsuname,"noname",sizeof(rsuname));
                          }
                          #if defined(EXTRA_DEBUG)
                          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"rsuname(%s)\n",rsuname);
                          #endif
                          interfaceLogTable[i].dirty = shm_interfaceLog_ptr[i].dirty = 0x0;
                          /* Start service and clear dirty bit. */
                          if(0 != sendifcrequest(IFC_ON, PHY_CV2X0)) {
                              set_my_error_state(LOGMGR_IFC_REQ_FAIL);
                              #if defined(EXTRA_DEBUG)
                              I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"LOGMGR_IFC_REQ_FAIL: cv2x0 on.\n");
                              #endif
                          }
                          sleep(IFC_START_WAIT_SECS);
                          #if defined(EXTRA_DEBUG)
                          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"cv2x0 IFCLOG enabled.\n");
                          #endif
                      }
                  } else {
                      /* If enabled then stop service, update local table. */
                      if(   (0 == memcmp(shm_interfaceLog_ptr[i].ifaceName, "eth0", 4))
                         && (SNMP_ROW_ACTIVE == interfaceLogTable[i].ifaceLogStatus)
                         && (mib_on == interfaceLogTable[i].ifaceGenerate)) {
                          /* Stop service. */
                          if(0 != sendifcrequest(IFC_OFF, PHY_ETH0)) {
                              set_my_error_state(LOGMGR_IFC_REQ_FAIL);
                              I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"LOGMGR_IFC_REQ_FAIL: eth0 off.\n");
                          }
                          if(0 != sendifcrequest(IFC_OFF, PHY_CV2X0)) {
                              set_my_error_state(LOGMGR_IFC_REQ_FAIL);
                              I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"LOGMGR_IFC_REQ_FAIL: cv2x0 off.\n");
                          }
                          sleep(IFC_START_WAIT_SECS);
                          /* Update from SHM. */
                          memcpy(&interfaceLogTable[i], &shm_interfaceLog_ptr[i],sizeof(interfaceLog_t));
                          interfaceLogTable[i].dirty = shm_interfaceLog_ptr[i].dirty = 0x0;
                          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"eth0 IFCLOG disabled.\n");
                      }
                      if(   (0 == memcmp(shm_interfaceLog_ptr[i].ifaceName, "cv2x0", 5))
                         && (SNMP_ROW_ACTIVE == interfaceLogTable[i].ifaceLogStatus)
                         && (mib_on == interfaceLogTable[i].ifaceGenerate)){
                          /* Stop service. */
                          if(0 != sendifcrequest(IFC_OFF, PHY_ETH0)) {
                              set_my_error_state(LOGMGR_IFC_REQ_FAIL);
                              I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"LOGMGR_IFC_REQ_FAIL: eth0 off.\n");
                          }
                          if(0 != sendifcrequest(IFC_OFF, PHY_CV2X0)) {
                              set_my_error_state(LOGMGR_IFC_REQ_FAIL);
                              I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"LOGMGR_IFC_REQ_FAIL: cv2x0 off.\n");
                          }
                          sleep(IFC_START_WAIT_SECS);
                          /* Update from SHM. */
                          memcpy(&interfaceLogTable[i], &shm_interfaceLog_ptr[i],sizeof(interfaceLog_t));
                          interfaceLogTable[i].dirty = shm_interfaceLog_ptr[i].dirty = 0x0;
                          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"cv2x0 IFCLOG disabled.\n");
                      }
                  } /* If active */
              } /* If dirty */
          } /* for */
          /* 
           * Manage open logs: 
           * Check size and time constraints on active rows. 
           * Check if dir is full.
           * Practically speaking LOGMGR handles already.
           */
          #if defined(EXTRA_DEBUG)
          if(0 == (rolling_counter % 100)) {
              for(i=0;i<RSU_IFCLOG_MAX;i++){
                  I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"ifclog[%d]:d=%u,g=%u,s=%d,nic=%s,file=%s,dir=%d,o=0x%x\n"
                      , i
                      , interfaceLogTable[i].dirty
                      , interfaceLogTable[i].ifaceGenerate
                      , interfaceLogTable[i].ifaceLogStatus
                      , interfaceLogTable[i].ifaceName
                      , interfaceLogTable[i].ifaceLogName
                      , interfaceLogTable[i].ifaceLogByDir
                      , interfaceLogTable[i].ifaceLogOptions);
              }
          }
          #endif
          rolling_counter++;
          sleep(1);
#if defined(MY_UNIT_TEST)
          mainloop = WFALSE;
#endif
      } /* while */
      wsu_share_kill(shm_interfaceLog_ptr, shm_interfaceLog_size);
      shm_interfaceLog_ptr = NULL;
  }
  pthread_exit(NULL);
}
