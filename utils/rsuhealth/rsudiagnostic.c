/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Filename: rsudiagnostics.c                                                 */
/* Purpose: RSU Diagnostics & Recovery                                        */
/*                                                                            */
/* Copyright (C) 2023 DENSO International America, Inc.  All Rights Reserved. */
/*                                                                            */
/*                                                                            */
/*----------------------------------------------------------------------------*/
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <pthread.h>
#include <time.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <ifaddrs.h>

#include <sys/socket.h>
#include <linux/if.h>
#include <linux/if_link.h>
#include <linux/if_packet.h>
#include <net/ethernet.h> /* the L2 protocols */

#include <linux/sockios.h>
#include <sys/ioctl.h>
#include <linux/ethtool.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>

#include <dirent.h>

#include "dn_types.h"
#include "wsu_sharedmem.h"
#include "i2v_util.h"
#include "ipcsock.h"
#include "rs.h"
#include "shm_tps.h"
#include "shm_sous.h"
#include "rsuhealth.h"
#include "rsudiagnostic.h"
#include "conf_table.h"
#include "ntcip-1218.h"

#if defined(MY_UNIT_TEST)
#include "stubs.h"
extern void seedtpsshm(int32_t control);
extern void seedsousshm(int32_t control);
#else
#define POPEN          popen
#define PCLOSE         pclose
#define WSUSENDDATA    wsuSendData
#define WSURECEIVEDATA wsuReceiveData
#endif

/* mandatory logging defines */
#if defined(MY_UNIT_TEST)
#define MY_ERR_LEVEL   LEVEL_DBG
#else
#define MY_ERR_LEVEL   LEVEL_PRIV /* syslog only, no serial. */
#endif
#define MY_NAME        "diag"

#define OUTPUT_MODULUS  1200      /* seconds, to serial & syslog */
#define SYSLOG_OUTPUT_MODULUS  300 /* seconds, to syslog only */

#if defined(MY_UNIT_TEST)
#undef OUTPUT_MODULUS
#define OUTPUT_MODULUS 1
/* Child thread exits before parent thread. Should be less than parent for oredrerly exit */
#define CHILD_TEST_ITERATION_MAX 75  /* # times through mainloop before normal exit mainloop==WFALSE */
#endif

#if defined(MY_UNIT_TEST)
#define RSUHEALTH_NIC_OF_INTEREST  "ens33"
#define IP_UNDER_TEST              "192.168.109.166"
#else
#define RSUHEALTH_NIC_OF_INTEREST  "eth0"
#endif


static int32_t fault_count = 0;

#define I2V_FAIL_THRESHOLD 10
extern char i2vfailcnt;

#define RSUHEALTH_SET_READY_THRESHOLD 2
int32_t ready_set_fault_count = 0;

uint32_t onesecondcounter = 0; /* number of times through mainloop */

extern rsuhealth_t * shm_rsuhealth_ptr; /* RSUHEALTH SHM */
extern rsuhealth_t   localrsuhealth;
extern rsuhealth_t   priorlocalrsuhealth; /* keep track on change */

rskStatusType rskstatus;  /* from RSK via service call */
static rskStatusType priorrskstatus;

static shm_tps_t * shm_tps_ptr = NULL; /* TPS SHM  */
shm_tps_t   localshmtps; /* Local copy of TPS SHM */
static tps_geodetic_publish_t tps_pub_geo; /* For readability of code. Could just use localshmtps */
static uint16_t pvt_index = 0;
static uint32_t time_of_week_msec = 0;
static uint8_t  init_time_once = 0;
static uint32_t priortpsshmseqno = 0;
static uint32_t priorpvtcount = 0;
static uint32_t priorpvtunusedcount = 0;
static uint32_t priorggacount = 0;
static uint32_t stuckgeodatacount = 0;
static uint32_t priornotimeacccount = 0;

static uint32_t rh850_update_count = 0;
static uint8_t init_antenna_once = 0;
static uint32_t rh850shmfailcount = 0;
static shm_sous_t * shm_sous_ptr = NULL; /* SOUS and related */
static shm_sous_t localsous;

extern led_diagnostics_t * shm_leddiag_ptr; /* LED SHM */
extern led_diagnostics_t   local_leddiag; 

static int32_t radioServicesSock = -1; /* radioServices socket*/

static int32_t checkhsmcount = 0;

static int32_t enableDebug = 0;

STATIC int32_t priorvalidcertcount = -1; /* On purpose, seed to be different so first count goes to file. */

extern char loop;
extern int rsusystem;

/* Manage customer folder */
#define CUSTOMER_FILE_NAME_SIZE        1024
#define CUSTOMER_DIR                   "/rwflash/customer" 
 /* bytes: This seems like plenty. */
#define CUSTOMER_STORAGE_MAX           (uint64_t)(1024 * 1024 * 1024)
/* 10000 files max or assume some kind of error */
#define CUSTOMER_CLEAN_ITERATIONS_MAX  10000
/* seconds */                         
#define CUSTOMER_FOLDER_CHECK_MODULUS  300
/* once every 5 mins is good enough. */                            
#define CHECK_CUSTOMER_FOLDER_RATE     300                           

static uint32_t customerfoldererrorstates = 0x0;

/* It's possible, if reading at 1 Hz, to miss another 1Hz update.
 * Allow at least more than 1 missed update before setting bit.
 */
#define UBLOX_STUCK_MSG_MAX  2
STATIC uint32_t stuck_gga_count = 0;
STATIC uint32_t stale_pvt_count = 0;
STATIC uint32_t stuck_pvt_timestamp_count = 0;

/* Seems like in weak signal conditions we can get intermittent jam indicators.
 * Allow the jam status to persist for some time before indicating.
 */
#define UBLOX_JAM_INDICATOR_THRESHOLD 30 /* seconds */
STATIC uint32_t jam_withfix = 0;
STATIC uint32_t jam_nofix = 0;

void initrsudiagnostics(int32_t debugEnable)
{
  onesecondcounter = 0;
#if defined(MY_UNIT_TEST)
  onesecondcounter = 59; /* Nothing exciting happens for first 60 seconds, so jump ahead. */
#endif

  shm_tps_ptr = NULL;
  shm_sous_ptr = NULL;
  memset(&localshmtps,0,sizeof(localshmtps));
  memset(&tps_pub_geo,0x0,sizeof(tps_pub_geo));
  pvt_index = 0;
  time_of_week_msec = 0;
  init_time_once = 0;
  priortpsshmseqno = 0;
  priorpvtcount = 0;
  priorpvtunusedcount = 0;
  priorggacount = 0;
  stuckgeodatacount = 0;
  memset(&localsous,0x0,sizeof(localsous));
  rh850_update_count = 0;
  init_antenna_once = 0;
  rh850shmfailcount = 0;
  fault_count = 0;
  priornotimeacccount = 0;
  radioServicesSock = -1;
  checkhsmcount = 0;
  memset(&rskstatus,0x0,sizeof(rskstatus));
  memset(&priorrskstatus,0x0,sizeof(priorrskstatus));
  enableDebug = debugEnable;

  /* Set to DENSO empty cause zero'ed value is rsuAlertLevel_info */
  localrsuhealth.h.ch_data_valid = WFALSE;
  localrsuhealth.trapsenabled = 0; /* Off by default. */
  localrsuhealth.messageFileIntegrityError = rsuAlertLevel_denso_empty;
  localrsuhealth.rsuSecStorageIntegrityError = rsuAlertLevel_denso_empty;
  localrsuhealth.rsuAuthError = rsuAlertLevel_denso_empty;
  localrsuhealth.rsuSignatureVerifyError = rsuAlertLevel_denso_empty;
  localrsuhealth.rsuAccessError = rsuAlertLevel_denso_empty;
  localrsuhealth.rsuTimeSourceLost = rsuAlertLevel_denso_empty;
  localrsuhealth.rsuTimeSourceMismatch = rsuAlertLevel_denso_empty;
  localrsuhealth.rsuGnssAnomaly = rsuAlertLevel_denso_empty;
  localrsuhealth.rsuGnssDeviationError = rsuAlertLevel_denso_empty;
  localrsuhealth.rsuCertificateError = rsuAlertLevel_denso_empty;
  localrsuhealth.rsuServiceDenialError = rsuAlertLevel_denso_empty;
  localrsuhealth.rsuWatchdogError = rsuAlertLevel_denso_empty;
  localrsuhealth.rsuEnvironError = rsuAlertLevel_denso_empty;
  localrsuhealth.cv2xruntime = RSUHEALTH_CV2X_ANTENNA_CHECK;

  stuck_gga_count = 0;
  stale_pvt_count = 0;
  stuck_pvt_timestamp_count = 0;
  jam_withfix = 0;
  jam_nofix = 0;

  priorvalidcertcount = -1; /* seed negative on purpose */
}
/*----------------------------------------------------------------------------*/
/* skipcriticalfiles:                                                         */
/* Returns 1 if this file is critcal system file or zero if it is not.        */
/*----------------------------------------------------------------------------*/
static int32_t skipcriticalfiles(char_t * filename)
{
  int32_t ret = 1; /* Skip file by default */
  if(NULL == filename) {
      ret = 1; /* Definately skip */
      I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"skipcriticalfiles: NULL input.\n");
      customerfoldererrorstates |= CUSTOMER_CLEAN_SYSFAIL;
  } else {
      /* Skip critical folders */
      if(   (0 == strcmp(filename, "/rwflash/customer/rwflash"))
         || (0 == strcmp(filename, "/rwflash/customer/."))
         || (0 == strcmp(filename, "/rwflash/customer/.."))
         || (0 == strcmp(filename, "/rwflash/customer/densonor.bz2")) 
         || (0 == strcmp(filename, "/rwflash/customer/profile"))
         || (0 == strcmp(filename, "/rwflash/customer/mysys"))){
          #if defined(EXTRA_DEBUG)
          I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"Ignoring(%s)\n",filename);
          #endif
          ret = 1; /* Skip */
      } else {
          ret = 0; /* Don't skip */
      }
  }
  return ret;
}
/*----------------------------------------------------------------------------*/
/* get_directory_size:                                                        */
/* Returns total size of all of the regular files in the specified directory. */
/*----------------------------------------------------------------------------*/
static uint64_t get_directory_size(char_t *dirname)
{
  DIR *dirp  = NULL;
  struct dirent *direntp = NULL;
  struct stat fstat;
  char_t filename[CUSTOMER_FILE_NAME_SIZE];
  uint64_t total = 0x0;
  int32_t iterations = 0;

  dirp  = NULL;
  direntp = NULL;
  memset(&fstat,0x0,sizeof(fstat));
  memset(filename,0x0,sizeof(filename));
  total = 0x0;

  if (NULL == (dirp = opendir(dirname))) { /* Open the output directory: Possbily directory does not exist or not mounted yet */
      I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"checkcustomerfolder: get_directory_size: can't open (%s) errno(%s)\n", dirname, strerror(errno));
      customerfoldererrorstates |= CUSTOMER_CLEAN_SYSFAIL;
  } else {
      iterations = 0;
      while ((iterations < CUSTOMER_CLEAN_ITERATIONS_MAX) && (NULL != (direntp = readdir(dirp)))) { /* Get info on the next directory entry */
          usleep(1000 * 10); /* no rush, dont hog */
          iterations++;
          sprintf(filename, "%s/%s", dirname, direntp->d_name);
          /* Skip critical folders */
          if(0 == skipcriticalfiles(filename)) {
              if(0 == stat(filename, &fstat)){
                  /* TODO: leave logic in place but disarmed for now. Maybe one day we care.
                   * if (fstat.st_mode & S_IFREG)
                   */ 
                  {
                  total += (uint64_t)fstat.st_size;  /* If this is a regular file, add in its size */
                  #if defined(EXTRA_DEBUG)
                  I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"Counting(%s) size(0x%lx) total(0x%lx)\n",filename,(uint64_t)fstat.st_size,total);
                  #endif
                  }
              } else {
                  I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"Stat(%s) failed, ignoring: total(0x%lx) \n",filename,total);
                  customerfoldererrorstates |= CUSTOMER_CLEAN_SYSFAIL;     
              }
          }
      }
      if(CUSTOMER_CLEAN_ITERATIONS_MAX <= iterations) {
          I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"checkcustomerfolder:get_directory_size: warning: too many files(%s) and still folder too full: total_bytes(0x%x) iterations(%d)\n",iterations,total);
          customerfoldererrorstates |= CUSTOMER_CLEAN_SYSFAIL;
      }
      #if defined(EXTRA_DEBUG)
      I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"get_directory_size: current folder size bytes(0x%lx) iterations(%d) state(0x%x)\n",total,iterations,customerfoldererrorstates);
      #endif
      closedir(dirp);
  }
  return total;
}
/*----------------------------------------------------------------------------*/
/* checkcustomerfolder:                                                       */                                
/* Returns: None.                                                             */
/* This function checks whether "du" in the output directory exceeds          */
/* CUSTOMER_STORAGE_MAX. If it does  old files are deleted until              */
/* CUSTOMER_DIR "du" is less than CUSTOMER_STORAGE_MAX.                       */
/*----------------------------------------------------------------------------*/
static void checkcustomerfolder(void)
{
  DIR           *dirP = NULL;
  struct dirent *file_name = NULL;
  struct stat    file_status;
  time_t         file_mod_time;
  char_t         curr_file_name[CUSTOMER_FILE_NAME_SIZE];
  char_t         oldest_file_name[CUSTOMER_FILE_NAME_SIZE];
  int32_t        iterations = 0;
  char_t         cmd[512];
  uint64_t       customer_dir_size = 0;

  dirP = NULL;
  file_name = NULL;
  memset(&file_status,0x0,sizeof(file_status));
  memset(&file_mod_time,0x0,sizeof(file_mod_time));
  memset(curr_file_name,0x0,sizeof(curr_file_name)); 
  memset(oldest_file_name,0x0,sizeof(oldest_file_name));

  /* If directory size is less than CUSTOMER_STORAGE_MAX, exit the
   * loop logging in /tmp there will not be old files after reset. 
   */
  for(iterations=0; iterations < CUSTOMER_CLEAN_ITERATIONS_MAX; iterations++) {
      customer_dir_size = get_directory_size(CUSTOMER_DIR);
      if(customer_dir_size < CUSTOMER_STORAGE_MAX) {
          I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"checkcustomerfolder: Folder is below size limit: AOK: (0x%lx) < (0x%lx)\n",customer_dir_size,CUSTOMER_STORAGE_MAX);
          /* Do not clear CUSTOMER_CLEAN_STATE here. Clear when RSUDIAG sends trap.*/
          break; /* Exit loop, done */
      } else {
          I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"checkcustomerfolder: Folder is above limit: CLEAN: (0x%lx) >= (0x%lx)\n",customer_dir_size,CUSTOMER_STORAGE_MAX);
          customerfoldererrorstates |= CUSTOMER_CLEAN_STATE;
      }
      /* Find out the oldest file to delete in the output directory */
      file_mod_time = time(NULL);
      if ((dirP = opendir(CUSTOMER_DIR)) != NULL) {
          while((file_name = readdir(dirP))) {
              memset(curr_file_name,0x0,sizeof(curr_file_name));
              snprintf(curr_file_name, sizeof(curr_file_name),"%s/%s", CUSTOMER_DIR, file_name->d_name);
              /* Skip critical folders */
              if(0 == skipcriticalfiles(curr_file_name)) {
                  /* Get the information about the file */
                  if (stat(curr_file_name, &file_status) == 0) {
                      #if defined(EXTRA_DEBUG)
                      I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"checkcustomerfolder: stat(%s): st_mtime(0x%lx) file_mod_time(0x%lx)\n",curr_file_name,file_status.st_mtime, file_mod_time);
                      #endif
                      /* If the current file modification time is less
                       * than previous one, copy the current one as last
                       * file 
                       */
                      if (file_status.st_mtime < file_mod_time) {
                          file_mod_time = file_status.st_mtime;
                          strncpy(oldest_file_name, curr_file_name,sizeof(oldest_file_name));
                      }
                  }  else {
                      I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"checkcustomerfolder: stat(%s) failed: errno(%s)\n",curr_file_name,strerror(errno));
                      customerfoldererrorstates |= CUSTOMER_CLEAN_SYSFAIL;
                  }
              }
          } /* while */
          /* Close the directory before deleting any files from that directory */
          closedir(dirP);
          if (oldest_file_name[0] != '\0') {
              memset(cmd,0x0,sizeof(cmd));
              snprintf(cmd,sizeof(cmd),"rm -rf %s",oldest_file_name);
              if(0 != system(cmd)){
                  I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"checkcustomerfolder: Error removing(%s) errno(%s)\n",oldest_file_name, strerror(errno));
                  customerfoldererrorstates |= CUSTOMER_CLEAN_SYSFAIL;
              } else {
                  /* Remove the oldest file */
                  I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"checkcustomerfolder: Deleting oldest(%s) Disk usage greater than max(0x%lx) bytes.\n",oldest_file_name,CUSTOMER_STORAGE_MAX);
              }
          } else {
              I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"checkcustomerfolder: Didn't find file to delete.\n");
              customerfoldererrorstates |= CUSTOMER_CLEAN_SYSFAIL;
          }
      } else {
          I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"checkcustomerfolder: opendir(%s) failed.\n",CUSTOMER_DIR);
          customerfoldererrorstates |= CUSTOMER_CLEAN_SYSFAIL;
      }
  }
  #if defined(EXTRA_DEBUG)
  I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"checkcustomerfolder: done checking: state(0x%x)\n",customerfoldererrorstates);
  #endif
}
/*------------------------------------------------------------------------------
** Function:  communicateWithRadioServices
** @brief  Send a message to Radio Services and get its reply.
** @param  msg       -- [input]Message to send to Radio Services
** @param  size      -- [input]Size of the message
** @param  reply     -- [output]Buffer to hold the received reply
** @param  replySize -- [input]Size of the reply buffer
** @return Size of the message received for success; -1 for error. Will never
**         return 0.
**
** Details: If it hasn't been done previously, creates the socket to
**          communicate with Radio Services. Sends a message to Radio Services
**          via wsuSendData() using port RS_RECV_PORT. Blocks waiting  for areply via
**          reply via wsuReceiveData().
**
**          Error messages are printed (maximum of 10 times) on errors of
**          socket creating, sending the message, or receiving the message.
**----------------------------------------------------------------------------*/
static int32_t communicateWithRadioServices(void *msg, size_t size, void *reply,
                                            size_t replySize)
{
    RcvDataType rcvData;
    size_t      rcvSize=0;
    int32_t     i=0;

    memset(&rcvData,0x0,sizeof(rcvData));
    /* Open socket to rsk */
    if (radioServicesSock < 0) {
        if(0 > (radioServicesSock = wsuConnectSocket(-1))) {
            if(0x0 == (localrsuhealth.cv2xstatus & RSUHEALTH_RSK_CONNECT_FAIL)) {
                localrsuhealth.cv2xstatus |= RSUHEALTH_RSK_CONNECT_FAIL;
                I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"nsstats: wsuConnectSocket() failed\n");
            }
        } else {
            localrsuhealth.cv2xstatus &= ~RSUHEALTH_RSK_CONNECT_FAIL;
        }
    }
    /* Send the message to Radio Services */
    if(-1 < radioServicesSock) {
        if (!WSUSENDDATA(radioServicesSock, RS_RECV_PORT, msg, size)) {
            if(0x0 == (localrsuhealth.cv2xstatus & RSUHEALTH_RSK_SEND_FAIL)) {
                localrsuhealth.cv2xstatus |= RSUHEALTH_RSK_SEND_FAIL;
                I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"nsstats: wsuSendData() failed\n");
            }
        } else {
            localrsuhealth.cv2xstatus &= ~RSUHEALTH_RSK_SEND_FAIL;
        }
    }
    /* If no failures above then proceed to receieve */
    for(i=0; (-1 < radioServicesSock) && (0 == rcvSize) && loop && (i < 10); i++){
        usleep(10 * 1000);
        rcvData.available = FALSE;
        rcvData.fd        = radioServicesSock;
        rcvData.data      = reply;
        rcvData.size      = replySize;
        if (0 < (rcvSize = WSURECEIVEDATA(0, &rcvData))) {
            /* We received data. In this case, there is no way
             * rcvData.available could have been set to FALSE. */
            localrsuhealth.cv2xstatus &= ~RSUHEALTH_RSK_RECEIVE_FAIL;
        } else {
            if(0x0 == (localrsuhealth.cv2xstatus & RSUHEALTH_RSK_RECEIVE_FAIL)) {
                localrsuhealth.cv2xstatus |= RSUHEALTH_RSK_RECEIVE_FAIL;
                I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"nsstats: wsuReceiveData() failed\n");
            }
        }
        /* If rcvSize is 0, then continue in the loop and try again. A return
         * value of 0 is something that can occur normally with datagram
         * sockets; wsuConnectSocket() specifies SOCK_DGRAM on its call to
         * wsuSendData(). Also, a return value of 0 does NOT indicate the
         * socket was closed. wsuReceiveData() will also return 0 if errno is
         * set to EINTR upron return upon return from recvfrom(); in this case,
         * you should try again. */
    }
    return (int32_t)rcvSize;
}
static int32_t nsstats_c(void)
{
  rsIoctlType cmd = WSU_NS_GET_RSK_STATUS;

  memset(&rskstatus,0x0,sizeof(rskstatus));
  return communicateWithRadioServices(&cmd, sizeof(cmd), &rskstatus, sizeof(rskstatus));
}
/*----------------------------------------------------------------------------*/
/* Will copy new msg in or concat till space exhausted. Once exhausted your   */
/* trap is dropped if equal or lesser level. Otherwise, higher priorty        */
/* overwrites. Higher level bumps higher. A zero length alertmsg is legal.    */
/*----------------------------------------------------------------------------*/
#define MAX_TRAP_MSG_LENGTH  128 /* alertmsg is 256. So src is less then sizeof(dest) below always.  */
static void setuptrap(rsuTrapMsgID_e msgid, rsuAlertLevel_e level, char_t * msg)
{
  char_t nextmsg[MAX_TRAP_MSG_LENGTH];
  size_t destlen=0,srclen=0,freespace=0,alertmsgsize=0;
  char_t *dest=NULL;
  rsuAlertLevel_e *destlevel=NULL;
  int32_t ret=0; /* Success till proven fail. */

  if((NULL == msg) || (level < rsuAlertLevel_info) || (rsuAlertLevel_critical < level)) {
      localrsuhealth.errorstates |= RSUHEALTH_BAD_INPUT;
      /* Error noted, ignore now. */
  } else if (localrsuhealth.trapsenabled) {
       /* Init */
      destlen=srclen=freespace=alertmsgsize=0;
      dest=NULL;
      destlevel=NULL;
      memset(nextmsg,0x0,sizeof(nextmsg));
      /* Set the destination pointers to correct alertmsg in MIB. */
      switch(msgid) {
          case messageFileIntegrityID:
              #if 0   /* Supported from amh.c & imf.c. RSUDIAG does not track this, */
              destlevel = &localrsuhealth.messageFileIntegrityError;
              dest      = localrsuhealth.rsuMsgFileIntegrityMsg;
              #endif
              break;
          case SecStorageIntegrityID:
              destlevel = &localrsuhealth.rsuSecStorageIntegrityError;
              dest      = localrsuhealth.rsuSecStorageIntegrityMsg;
              break;
          case AuthID: /* TODO: not supported */
              destlevel = &localrsuhealth.rsuAuthError;
              dest      = localrsuhealth.rsuAuthMsg;
              break;
          case SignatureVerifyID:
              destlevel = &localrsuhealth.rsuSignatureVerifyError;
              dest      = localrsuhealth.rsuSignatureVerifyMsg;
              break;
          case AccessID: /* TODO: not supported */
              #if 0 /* Supported from rsuaccessmgr */
              destlevel = &localrsuhealth.rsuAccessError;
              dest      = localrsuhealth.rsuAccessMsg;
              #endif
              break;
          case TimeSourceLostID:
              destlevel = &localrsuhealth.rsuTimeSourceLost;
              dest      = localrsuhealth.rsuTimeSourceLostMsg;
              break;
          case TimeSourceMismatchID:
              destlevel = &localrsuhealth.rsuTimeSourceMismatch;
              dest      = localrsuhealth.rsuTimeSourceMismatchMsg;
              break;
          case GnssAnomalyID:
              destlevel = &localrsuhealth.rsuGnssAnomaly;
              dest      = localrsuhealth.rsuGnssAnomalyMsg;
              break;
          case GnssDeviationID: /* Supported from rsuGnssOutput.c */
              destlevel = &localrsuhealth.rsuGnssDeviationError;
              dest      = localrsuhealth.rsuGnssDeviationMsg;
              break;
          case GnssNmeaNotifyID:
              #if 0 /* Supported from rsuGnssOutput.c, doesn't need to be in SHM. Irregular type of notify. */
              destlevel = &localrsuhealth.rsuGnssNmeaNotify;
              dest      = localrsuhealth.rsuGnssOutputString;
              #endif
              break;
          case CertificateID:
              destlevel = &localrsuhealth.rsuCertificateError;
              dest      = localrsuhealth.rsuCertificateMsg;
              break;
          case ServiceDenialID:
              destlevel = &localrsuhealth.rsuServiceDenialError;
              dest      = localrsuhealth.rsuServiceDenialMsg;
              break;
          case WatchdogID:
              destlevel = &localrsuhealth.rsuWatchdogError;
              dest      = localrsuhealth.rsuWatchdogMsg;
              break;
          case EnvironID:
              destlevel = &localrsuhealth.rsuEnvironError;
              dest      = localrsuhealth.rsuEnvironMsg;
              break;
          default:
              ret = -1;
              localrsuhealth.errorstates |= RSUHEALTH_BAD_INPUT;
              break;
      }
      if(0 == ret) {
          /* Get src values: Copy msg over safely and truncate if need be. */
          snprintf(nextmsg,sizeof(nextmsg),"%s",msg); /* Copy only to the sizeof and NULL terminate string. */
          srclen = strnlen(msg,sizeof(nextmsg));      /* Get length to see if we did have to truncate */
          if(sizeof(nextmsg) <= srclen) {             /* Error check length to see if we trucated. */
              nextmsg[sizeof(nextmsg)-3] = '~';       /* If so, put special char to denote truncation. */
              nextmsg[sizeof(nextmsg)-2] = ']';
              nextmsg[sizeof(nextmsg)-1] = '\0';      /* Shouldn't need it, but just in case. */
              localrsuhealth.errorstates |= RSUHEALTH_TRAP_TRUCATED;
          }
          /* Get dest values: Whats in the alertmsg already. */
          alertmsgsize = RSU_ALERT_MSG_LENGTH_MAX;
          destlen      = strnlen(dest,alertmsgsize);
          freespace    = alertmsgsize - destlen - 1; /* Account for NULL. */


#if defined(EXTRA_DEBUG)
          I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"before:msgid(%d)dest(%u:%s)\n",msgid,freespace,dest);
#endif
          /* If alert is empty then copy. Higher level always bumps alert level.
             If not enough room for concat then nextmsg dropped if equal or less level than current.
             Otherwise, purge current & overwrite with higher level alert msg.
          */
          if(rsuAlertLevel_denso_empty == *destlevel) {
#if defined(EXTRA_DEBUG)
              I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"src(%d:%d) dst empty: (%u:%s)\n",msgid,level,srclen,nextmsg);
#endif
              *destlevel = level;
              memset(dest,0x0,alertmsgsize);
              strncpy(dest,nextmsg,srclen);

          } else {
              if(srclen <= freespace) {                  /* src is small enough to fit dest */
#if defined(EXTRA_DEBUG)
                  I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"src(%d:%d) aok: (%u:%s)\n",msgid,level, srclen,nextmsg);
#endif
                  strncat(dest,nextmsg,srclen);          /* concat */
                  if(*destlevel > level){                /* bump to next level */
                      *destlevel = level;
                  }
              } else {                                   /* src is too big. */
#if defined(EXTRA_DEBUG)
                  I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"src(%d:%d) too big: (%u:%s)\n",msgid,level,srclen,nextmsg);
#endif
                  if(*destlevel > level){                /* if src is greater level then overwrite alertmsg */
                      *destlevel = level;
                      memset(dest,0x0,alertmsgsize);
                      strncpy(dest,nextmsg,srclen);
                  }                                      /* else drop it, too bad. */
                  localrsuhealth.errorstates |= RSUHEALTH_TRAP_DROPPED;
              }
          }
#if defined(EXTRA_DEBUG)
          freespace = freespace - srclen;
          I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"done:msgid(%d:%d)dest(%u:%s)\n",msgid,level,freespace,dest);
#endif
      }
  }
}
/*----------------------------------------------------------------------------*/
/* JIRA XXXX: Ensure we have a good fix and min sv required.                  */
/*----------------------------------------------------------------------------*/
static void checkgnssfix(void)
{
  if(tps_pub_geo.last_PVT[pvt_index].flag_gnss_fix_ok && (RSUHEALTH_MIN_GPS_SV_COUNT <= tps_pub_geo.last_PVT[pvt_index].num_sats)){
      if(0x0 == (localrsuhealth.gnssstatus & RSUHEALTH_GNSS_FIX_VALID)) {
          localrsuhealth.gnssstatus |= RSUHEALTH_GNSS_FIX_VALID;
          I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"checkgnssfix valid: ok(%d) sv cnt(%u)\n",tps_pub_geo.last_PVT[pvt_index].flag_gnss_fix_ok,tps_pub_geo.last_PVT[pvt_index].num_sats);
          setuptrap(GnssAnomalyID, rsuAlertLevel_info,"[GNSS fix valid now]");
      }
  } else {
      if(localrsuhealth.gnssstatus & RSUHEALTH_GNSS_FIX_VALID){
          localrsuhealth.gnssstatus &= ~RSUHEALTH_GNSS_FIX_VALID;
          I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"checkgnssfix: failed cnt(%u) ok(%d) sv cnt(%u)\n",localrsuhealth.gnsserrorcounts.badfix,tps_pub_geo.last_PVT[pvt_index].flag_gnss_fix_ok,tps_pub_geo.last_PVT[pvt_index].num_sats);
          setuptrap(GnssAnomalyID, rsuAlertLevel_critical,"[GNSS fix was lost, too few sats in view]");
      }
      localrsuhealth.gnsserrorcounts.badfix++;
  }
}
/*----------------------------------------------------------------------------*/
/* Grab latest published PVT. Ensure tps shm geo data is not stale.           */
/*----------------------------------------------------------------------------*/
static int32_t checkgeodata(void)
{
  int32_t ret = 0;

  /* Copy once. */
  memcpy(&localshmtps,shm_tps_ptr,sizeof(localshmtps));
  memcpy(&tps_pub_geo, &localshmtps.pub_geodetic, sizeof(tps_geodetic_publish_t));
  pvt_index = tps_pub_geo.last_updated_pvt_index;

  if((0 == pvt_index) || (1 == pvt_index)){ /* Check index is valid range. */
      if(0 == init_time_once) { /* Check that TOW has changed. Can roll or jump back but never be the same as prior. */
          time_of_week_msec = tps_pub_geo.last_PVT[pvt_index].time_of_week_msec;
          init_time_once = 1;
      } else {
          localrsuhealth.errorstates &= ~(uint64_t)RSUHEALTH_BOGUS_PVT_INDEX;
          if(time_of_week_msec == tps_pub_geo.last_PVT[pvt_index].time_of_week_msec) {
              if(UBLOX_STUCK_MSG_MAX <= stale_pvt_count) {
                  ret = -2;
                  if(0x0 == (localrsuhealth.errorstates & RSUHEALTH_STALE_PVT)){
                      I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"RSUHEALTH_STALE_PVT: published tow(%u) same as prior(%u)\n",tps_pub_geo.last_PVT[pvt_index].time_of_week_msec,time_of_week_msec);
                      localrsuhealth.errorstates |= RSUHEALTH_STALE_PVT;
                  }
              } else {
                  stale_pvt_count++;
              }
          } else {
              time_of_week_msec = tps_pub_geo.last_PVT[pvt_index].time_of_week_msec;
              localrsuhealth.errorstates &= ~(uint64_t)RSUHEALTH_STALE_PVT;
              stale_pvt_count = 0;
          }
      }
  } else {    
      ret = -1;
      if(0x0 == (localrsuhealth.errorstates & RSUHEALTH_BOGUS_PVT_INDEX)){
          localrsuhealth.errorstates |= RSUHEALTH_BOGUS_PVT_INDEX;
      }
  }
  return ret;
}
/*----------------------------------------------------------------------------*/
/* gnss antenna: JIRA 347                                                     */
/*   Check sous shm for go or no go on antenna connection                     */                
/*----------------------------------------------------------------------------*/
static void checkgnsscv2xantenna(void)
{
  char_t nextmsg[64];

  /* Until we see something from SOUS SHM don't check antenna. */
  if(0 == localsous.rh850_update_count) {  /* Nothing to do so exit. */
      rh850shmfailcount++;
      if(RH850_SHM_UPDATE_RATE <= rh850shmfailcount) {
          if(0x0 == (localrsuhealth.errorstates & RSUHEALTH_STALE_SOUS)) {
              I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"checkgnsscv2xantenna: sous shm failed to start(%u)\n", rh850_update_count);
              localrsuhealth.errorstates |= RSUHEALTH_STALE_SOUS;
          }
      }
      return;
  } else {
      if(localrsuhealth.errorstates & RSUHEALTH_STALE_SOUS) { /* Clear error and continue. */
          I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"checkgnsscv2xantenna: sous shm has started.\n");
          localrsuhealth.errorstates &= ~RSUHEALTH_STALE_SOUS;
          rh850shmfailcount = 0;
      }
  }
  /* Now we have at least one update. Check antennas and look for any stall in SOUS SHM updates. */
  if(0 == init_antenna_once) {
      if(ANTCON_CONNECTED == localsous.rh850_data.gps_ant2_status) {  /* go or no go. */
          localrsuhealth.gnssstatus &= ~RSUHEALTH_GNSS_BAD_ANTENNA;
      } else {
          localrsuhealth.gnsserrorcounts.badantenna++; 
          if(0x0 == (localrsuhealth.gnssstatus & RSUHEALTH_GNSS_BAD_ANTENNA)) {
              localrsuhealth.gnssstatus |= RSUHEALTH_GNSS_BAD_ANTENNA;
              memset(nextmsg,0x0,sizeof(nextmsg));
              snprintf(nextmsg,sizeof(nextmsg),"[GNSS BAD ANTENNA]");
              setuptrap(GnssAnomalyID, rsuAlertLevel_critical,nextmsg);
              I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"GNSS_BAD_ANTENNA: init(%u) status(%u) cnt(%u)\n",
                  init_antenna_once
                  ,localsous.rh850_data.gps_ant2_status
                  ,localsous.rh850_update_count);
          }
      }
      if(RSUHEALTH_CV2X_ANTENNA_CHECK & localrsuhealth.cv2xruntime) {
          if(ANTCON_CONNECTED == localsous.rh850_data.cv2x_ant1_status) {  /* go or no go. */
              localrsuhealth.cv2xstatus &= ~RSUHEALTH_CV2X1_BAD_ANTENNA;
          } else {
              localrsuhealth.cv2xstatus |= RSUHEALTH_CV2X1_BAD_ANTENNA;
              localrsuhealth.cv2xerrorcounts.badantenna1++;
          }
          if(ANTCON_CONNECTED == localsous.rh850_data.cv2x_ant2_status) {  /* go or no go. */
              localrsuhealth.cv2xstatus &= ~RSUHEALTH_CV2X2_BAD_ANTENNA;
          } else {
              localrsuhealth.cv2xstatus |= RSUHEALTH_CV2X2_BAD_ANTENNA;
              localrsuhealth.cv2xerrorcounts.badantenna2++;
          }
      } else {
          localrsuhealth.cv2xstatus &= ~RSUHEALTH_CV2X1_BAD_ANTENNA;
          localrsuhealth.cv2xstatus &= ~RSUHEALTH_CV2X2_BAD_ANTENNA;
      }
      rh850_update_count = localsous.rh850_update_count;
      init_antenna_once = 1;
  } else {
      if(rh850_update_count == localsous.rh850_update_count) {
          rh850shmfailcount++;
          if(RH850_SHM_UPDATE_RATE <= rh850shmfailcount) {
              if(0x0 == (localrsuhealth.errorstates & RSUHEALTH_STALE_SOUS)) {
                  I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"checkgnsscv2xantenna: sous shm not updating = %u.\n", rh850_update_count);
                  localrsuhealth.errorstates |= RSUHEALTH_STALE_SOUS;
              }
          } 
      } else {
          rh850shmfailcount = 0;
          localrsuhealth.errorstates &= ~(uint64_t)RSUHEALTH_STALE_SOUS;

          if(ANTCON_CONNECTED == localsous.rh850_data.gps_ant2_status) { /* go or no go. */
              localrsuhealth.gnssstatus &= ~RSUHEALTH_GNSS_BAD_ANTENNA;
          } else {
              localrsuhealth.gnsserrorcounts.badantenna++;
              if(0x0 == (localrsuhealth.gnssstatus & RSUHEALTH_GNSS_BAD_ANTENNA)) {
                  localrsuhealth.gnssstatus |= RSUHEALTH_GNSS_BAD_ANTENNA;
                  memset(nextmsg,0x0,sizeof(nextmsg));
                  snprintf(nextmsg,sizeof(nextmsg),"[GNSS BAD ANTENNA]");
                  setuptrap(GnssAnomalyID, rsuAlertLevel_critical,nextmsg);
                  I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"GNSS_BAD_ANTENNA: init(%u) status(%u) cnt(%u)\n",
                      init_antenna_once
                      ,localsous.rh850_data.gps_ant2_status
                      ,localsous.rh850_update_count);
              }
          }
          if(RSUHEALTH_CV2X_ANTENNA_CHECK & localrsuhealth.cv2xruntime) {
              if(ANTCON_CONNECTED == localsous.rh850_data.cv2x_ant1_status) {  /* go or no go. */
                  localrsuhealth.cv2xstatus &= ~RSUHEALTH_CV2X1_BAD_ANTENNA;
              } else {
                  localrsuhealth.cv2xstatus |= RSUHEALTH_CV2X1_BAD_ANTENNA;
                  localrsuhealth.cv2xerrorcounts.badantenna1++;
              }
              if(ANTCON_CONNECTED == localsous.rh850_data.cv2x_ant2_status) {  /* go or no go. */
                  localrsuhealth.cv2xstatus &= ~RSUHEALTH_CV2X2_BAD_ANTENNA;
              } else {
                  localrsuhealth.cv2xstatus |= RSUHEALTH_CV2X2_BAD_ANTENNA;
                  localrsuhealth.cv2xerrorcounts.badantenna2++;
              }
          } else {
              localrsuhealth.cv2xstatus &= ~RSUHEALTH_CV2X1_BAD_ANTENNA;
              localrsuhealth.cv2xstatus &= ~RSUHEALTH_CV2X2_BAD_ANTENNA;
          }
          rh850_update_count = localsous.rh850_update_count;
      }
  }
}
//TODO: the spec has a hard number of 60 seconds so configurable? Nothing in MIB
/*----------------------------------------------------------------------------*/
/* gnss signal loss: JIRA 608                                                 */
/*   Ensure we get 3D position fix within a configurable(?) time threshold    */                
/*----------------------------------------------------------------------------*/
static void checkgnsssignal(void)
{
  /* Test once and either we met it or we didn't. */
  if(onesecondcounter == RSU_TIME_TO_FIRST_FIX_MAX) {
      if(0x0 == (localrsuhealth.gnssstatus & RSUHEALTH_GNSS_FIX_VALID)) {
          localrsuhealth.gnssstatus |= RSUHEALTH_GNSS_BAD_TTFF;
          setuptrap(GnssAnomalyID, rsuAlertLevel_critical,"[Time to first fix took too long. No GNSS fix]");
      } else {
          localrsuhealth.gnssstatus &= ~RSUHEALTH_GNSS_BAD_TTFF;
      }
  }
}
/*----------------------------------------------------------------------------*/
/* gnss init: JIRA 1671                                                       */
/*   Check for gps init issues, check ublox cfg and version.                  */                
/*----------------------------------------------------------------------------*/
static void checkgnssinit(void)
{
  /* UBlox Configuration Version */
  if (   (0 == strcmp((char_t *)&localshmtps.ublox_config_version_str[0],UBX_CFG_VERSION_STR)) 
      || (0 == strcmp((char_t *)&localshmtps.ublox_config_version_str[0],UBX_CFG_VERSION_STR_2))){
      localrsuhealth.gnssstatus &= ~RSUHEALTH_GNSS_BAD_CFG;
      #if defined(EXTRA_EXTRA_DEBUG)
      I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"UBX_CFG_VERSION_STR match: wanted [%s] vs [%s]\n",
          UBX_CFG_VERSION_STR,localshmtps.ublox_config_version_str);
      #endif
  } else {
      if(0x0 == (localrsuhealth.gnssstatus & RSUHEALTH_GNSS_BAD_CFG)){
          localrsuhealth.gnssstatus |= RSUHEALTH_GNSS_BAD_CFG;
          I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"UBX_CFG_VERSION_STR wrong(%u): wanted [%s] vs [%s]\n",
              localrsuhealth.gnsserrorcounts.badcfg,UBX_CFG_VERSION_STR,localshmtps.ublox_config_version_str);
      }
      localrsuhealth.gnsserrorcounts.badcfg++;
  }
  /* UBlox Firmware Version */
  if (0 == strcmp((char_t *)localshmtps.ublox_firmware_version_str,UBX_FW_VERSION_STR)) {
      localrsuhealth.gnssstatus &= ~RSUHEALTH_GNSS_BAD_FW;
      #if defined(EXTRA_EXTRA_DEBUG)
      I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"UBX_FW_VERSION_STR match: wanted [%s] vs [%s]\n",
          UBX_FW_VERSION_STR,localshmtps.ublox_firmware_version_str);
      #endif
  } else {
      if(0x0 == (localrsuhealth.gnssstatus & RSUHEALTH_GNSS_BAD_FW)){
          localrsuhealth.gnssstatus |= RSUHEALTH_GNSS_BAD_FW;
          I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"UBX_FW_VERSION_STR wrong(%u): wanted [%s] vs [%s]\n",
              localrsuhealth.gnsserrorcounts.badfw,UBX_FW_VERSION_STR,localshmtps.ublox_firmware_version_str);
      }
      localrsuhealth.gnsserrorcounts.badfw++;
  }
}
/*----------------------------------------------------------------------------*/
/* gnss interface: JIRA 609                                                   */
/*   Check if data is not received on the interface within a configurable     */
/*   time threshold.                                                          */                
/*----------------------------------------------------------------------------*/
static int32_t checkgnssinterface(void)
{
  int32_t ret = 0;
  /* Could be SHM is broken or not ready at boot. Deem it some kind of interface issue for now. */
  if(priortpsshmseqno == localshmtps.seqno) {
      if(0x0 == (localrsuhealth.gnssstatus & RSUHEALTH_GNSS_TPS_SEQNO_STUCK)){
          I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"TPS SHM seqno stuck = %u.\n",priortpsshmseqno);
          localrsuhealth.gnssstatus |= RSUHEALTH_GNSS_TPS_SEQNO_STUCK;
      }
      ret = -1;
  } else {
      priortpsshmseqno = localshmtps.seqno;
      localrsuhealth.gnssstatus &= ~RSUHEALTH_GNSS_TPS_SEQNO_STUCK;
  }
  /* Check that PVT's are coming in regular. */ 
  if(priorpvtcount == localshmtps.debug_gps_cmd_count_ubx_nav_pvt_total) {
      if(UBLOX_STUCK_MSG_MAX <= stuck_pvt_timestamp_count) {
          if(0x0 == (localrsuhealth.gnssstatus & RSUHEALTH_GNSS_PVT_STUCK)){
              I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"TPS SHM pvt count stuck = %u.\n",priorpvtcount);
              localrsuhealth.gnssstatus |= RSUHEALTH_GNSS_PVT_STUCK;
          }
      } else {
          stuck_pvt_timestamp_count++;
      }
      ret = -2;
  } else {
      stuck_pvt_timestamp_count = 0;
      priorpvtcount = localshmtps.debug_gps_cmd_count_ubx_nav_pvt_total;
      localrsuhealth.gnssstatus &= ~RSUHEALTH_GNSS_PVT_STUCK; 
  }
  /* Check to see if unused(late) PVT's are growing at consecutive rate. */
  if(priorpvtunusedcount != localshmtps.debug_gps_cmd_count_ubx_nav_pvt_unused){
      priorpvtunusedcount = localshmtps.debug_gps_cmd_count_ubx_nav_pvt_unused;
      if(0x0 == (localrsuhealth.gnssstatus & RSUHEALTH_GNSS_PVT_UNUSED)){
          I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"TPS SHM pvt unused count grew = %u.\n",priorpvtunusedcount);
          localrsuhealth.gnssstatus |= RSUHEALTH_GNSS_PVT_UNUSED;
      }
  } else {
      localrsuhealth.gnssstatus &= ~RSUHEALTH_GNSS_PVT_UNUSED;
  }
  /* Check that GGA coming in regular. */
  if(priorggacount == localshmtps.debug_gps_cmd_count_nmea_gpgga_valid_data){
      if(UBLOX_STUCK_MSG_MAX <= stuck_gga_count) {
          if(0x0 == (localrsuhealth.gnssstatus & RSUHEALTH_GNSS_GGA_STUCK)){
              I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"TPS SHM gga count stuck = %u.\n",priorggacount);
              localrsuhealth.gnssstatus |= RSUHEALTH_GNSS_GGA_STUCK;
          }
      } else {
          stuck_gga_count++;
      }
  } else {
      priorggacount = localshmtps.debug_gps_cmd_count_nmea_gpgga_valid_data;
      localrsuhealth.gnssstatus &= ~RSUHEALTH_GNSS_GGA_STUCK;
      stuck_gga_count = 0;
  }
  /* Check if we have valid leap second. */
  if(localshmtps.leap_secs_valid) {
      if(localrsuhealth.gnssstatus & RSUHEALTH_GNSS_BAD_LEAPSEC) {
          localrsuhealth.gnssstatus &= ~RSUHEALTH_GNSS_BAD_LEAPSEC;
          I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"TPS SHM valid leap second.\n");
      }
  } else {
      if(0x0 == (localrsuhealth.gnssstatus & RSUHEALTH_GNSS_BAD_LEAPSEC)) {
          localrsuhealth.gnssstatus |= RSUHEALTH_GNSS_BAD_LEAPSEC;
          I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"TPS SHM leap second not valid.\n");
      }
  }

  return ret;
}
/*----------------------------------------------------------------------------*/
/* gnss anomaly: JIRA 353                                                     */
/*   Check for spoofing, interference, jamming, or signal integrity issues.   */                
/*----------------------------------------------------------------------------*/
static void checkgnssanomaly(void)
{
  char_t nextmsg[64];

  /* Check spoofing. 
     0: Unknown or deactivated
     1: No spoofing indicated
     2: Spoofing indicated
     3: Multiple spoofing indications
     Note that the spoofing state value only reflects the detector state for the current navigation epoch.
     As spoofing can be detected most easily at the transition from real signal to spoofing signal, this is
     also where the detector is triggered the most. I.e. a value of 1 - No spoofing indicated does not mean
     that the receiver is not spoofed, it simply states that the detector was not triggered in this epoch.
  */
  memset(nextmsg,0x0,sizeof(nextmsg));
  if (1 < localshmtps.nav_status.spoofing_detection_state){
      if(0x0 == (localrsuhealth.gnssstatus & RSUHEALTH_GNSS_SPOOF_DETECT)) {
          localrsuhealth.gnssstatus |= RSUHEALTH_GNSS_SPOOF_DETECT;
          memset(nextmsg,0x0,sizeof(nextmsg));
          snprintf(nextmsg,sizeof(nextmsg),"[SPOOF DETECT: state(%u)]",localshmtps.nav_status.spoofing_detection_state);
          setuptrap(GnssAnomalyID, rsuAlertLevel_warning,nextmsg);
          I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"RSUHEALTH_GNSS_SPOOF_DETECT: state(%u)\n",localshmtps.nav_status.spoofing_detection_state);
      }
  } else {
      localrsuhealth.gnssstatus &= ~RSUHEALTH_GNSS_SPOOF_DETECT;
  }
  /* Check jamming.
     0 == no cw jam , 255 == strong cw jam. 
    The field jamInd of the UBX-MON-HW message can be used as an indicator for continuous wave
    (narrowband) jammers/interference only. The interpretation of the value depends on the
    application. It is necessary to run the receiver in an unjammed environment to determine an
    appropriate value for the unjammed case. If the value rises significantly above this threshold, this
    indicates that a continuous wave jammer is present
  */
  if(3 == localshmtps.mon_hw_jamming_status){
      if(UBLOX_JAM_INDICATOR_THRESHOLD <= jam_withfix) {
          if(0x0 == (localrsuhealth.gnssstatus & RSUHEALTH_GNSS_JAM_PRESENT)){
              localrsuhealth.gnssstatus |= RSUHEALTH_GNSS_JAM_PRESENT;
              memset(nextmsg,0x0,sizeof(nextmsg));
              snprintf(nextmsg,sizeof(nextmsg),"[JAM PRESENT: FIX OK: ind(%u)]",localshmtps.nav_status.spoofing_detection_state);
              setuptrap(GnssAnomalyID,rsuAlertLevel_warning,nextmsg);
              I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"RSUHEALTH_GNSS_JAM_PRESENT: FIX OK: ind(%u)\n",localshmtps.mon_hw_jamming_ind);
          }
      } else {
          jam_withfix++;
      }
  } else {
      jam_withfix=0;
      localrsuhealth.gnssstatus &= ~RSUHEALTH_GNSS_JAM_PRESENT;
  }
  if(4 == localshmtps.mon_hw_jamming_status) {
      if(UBLOX_JAM_INDICATOR_THRESHOLD <= jam_nofix) {
          if(0x0 == (localrsuhealth.gnssstatus & RSUHEALTH_GNSS_JAM_NOFIX)){
              localrsuhealth.gnssstatus |= RSUHEALTH_GNSS_JAM_NOFIX; 
              memset(nextmsg,0x0,sizeof(nextmsg));
              snprintf(nextmsg,sizeof(nextmsg),"[JAM PRESENT: NO FIX: ind(%u)]",localshmtps.nav_status.spoofing_detection_state);
              setuptrap(GnssAnomalyID,rsuAlertLevel_warning,nextmsg);
              I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"RSUHEALTH_GNSS_JAM_NOFIX: NO FIX: ind(%u)\n",localshmtps.mon_hw_jamming_ind);
          }
      } else {
          jam_nofix++;
      }
  } else {
      jam_nofix=0;
      localrsuhealth.gnssstatus &= ~RSUHEALTH_GNSS_JAM_NOFIX;
  }
  if(0x0 == localshmtps.mon_hw_rtc_is_calibrated){
      if(0x0 == (localrsuhealth.gnssstatus & RSUHEALTH_GNSS_RTC_BAD_CAL)){
#if 0 //Means weak signal or poor fix quality. Not updating rtc anymore.
          I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"GNSS: rtc not calibrated: tps(0x%x).\n",localshmtps.error_states);
          memset(nextmsg,0x0,sizeof(nextmsg));
          snprintf(nextmsg,sizeof(nextmsg),"[rtc not calibrated]");
          setuptrap(GnssAnomalyID,rsuAlertLevel_warning,nextmsg);
#endif
          localrsuhealth.gnssstatus |= RSUHEALTH_GNSS_RTC_BAD_CAL;
      }
  } else {
      localrsuhealth.gnssstatus &= ~RSUHEALTH_GNSS_RTC_BAD_CAL;
  }
}
//TODO: reference time is the incoming message and 1PPS isr. Is this right?
/*----------------------------------------------------------------------------*/
/* gnss 1pps: JIRA 864                                                        */
/*   Check that 1pps is regular and no init or runtime errors.                */
/* gnss time sync: JIRA 311 or 608?                                           */
/*   System time reference source lost for a configurable time threshold      */
/* gnss time accurracy: JIRA 309                                              */
/*  Check ubx time error stats for threshold.                                 */
/*----------------------------------------------------------------------------*/
static void checkgnss1pps(void)
{
  /* 5.18.1.6 Time Source Lost Message: Lost time source. */

  /* Init error checks seem fatal and should be recovered. */
  if(localshmtps.error_states & (GPS_FIND_1PPS_ERROR|GPS_1PPS_THREAD_EXIT|GPS_OPEN_1PPS_ERROR|GPS_CREATE_1PPS_ERROR|GPS_GET_PARAMS_1PPS_ERROR|GPS_SET_PARAMS_1PPS_ERROR)){
      if(0x0 == (localrsuhealth.gnssstatus & RSUHEALTH_1PPS_INIT_FAIL)){
          I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"1PPS: init error detected = 0x%x.\n",localshmtps.error_states);
          localrsuhealth.gnssstatus |= RSUHEALTH_1PPS_INIT_FAIL;
          setuptrap(TimeSourceLostID,rsuAlertLevel_critical,"[1PPS:INIT FAIL]");
      }
  }
  /* Runtime errors may pop up from time to time but do we need recovery? Not sure today. */
  if(localshmtps.debug_cnt_1pps_fetch_event) {
      if(localrsuhealth.gnsserrorcounts.badfetch != localshmtps.debug_cnt_1pps_fetch_event) {
          localrsuhealth.gnsserrorcounts.badfetch = localshmtps.debug_cnt_1pps_fetch_event;
          if(0x0 == (localrsuhealth.gnssstatus & RSUHEALTH_1PPS_BAD_FETCH)){
              I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"1PPS: fetch fetch error detected(%u): 0x%x.\n",localshmtps.debug_cnt_1pps_fetch_event,localshmtps.error_states);
              localrsuhealth.gnssstatus |= RSUHEALTH_1PPS_BAD_FETCH;
          }
      }
  }
  /* TPS has a couple 1PPS latency checks. */
  if(localshmtps.debug_cnt_1pps_latency_event) {
      if(localrsuhealth.gnsserrorcounts.bad1ppslatency != localshmtps.debug_cnt_1pps_latency_event) {
          if(0x0 == (localrsuhealth.gnssstatus & RSUHEALTH_1PPS_BAD_LATENCY)){
              I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"1PPS: latency error detected(%u): 0x%x.\n",localshmtps.debug_cnt_1pps_latency_event,localshmtps.error_states);
              localrsuhealth.gnssstatus |= RSUHEALTH_1PPS_BAD_LATENCY;
          }
          localrsuhealth.gnsserrorcounts.bad1ppslatency = localshmtps.debug_cnt_1pps_latency_event;
      }
  }
 /* TODO: This seems to happen from boot to boot and persist. Not sure its a real issue? */
#if 0
  if(localshmtps.debug_cnt_1pps_tps_latency_event){
      if(localrsuhealth.gnsserrorcounts.bad1ppstpslatency != localshmtps.debug_cnt_1pps_tps_latency_event) {
          if(0x0 == (localrsuhealth.gnssstatus & RSUHEALTH_1PPS_TPS_BAD_LATENCY)){
              I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"1PPS: isr vs tps latency error detected(%u): 0x%x.\n",localshmtps.debug_cnt_1pps_tps_latency_event,localshmtps.error_states);
              localrsuhealth.gnssstatus |= RSUHEALTH_1PPS_TPS_BAD_LATENCY;
          }
          localrsuhealth.gnsserrorcounts.bad1ppstpslatency = localshmtps.debug_cnt_1pps_tps_latency_event;
      }
  }
#endif
  if(localshmtps.debug_cnt_timestamp_missing) {
      if(localrsuhealth.gnsserrorcounts.badtimestamp != localshmtps.debug_cnt_timestamp_missing) {
          if(0x0 == (localrsuhealth.gnssstatus |= RSUHEALTH_1PPS_BAD_TIMESTAMP)) {
              I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"1PPS: timestamp missing (%u): 0x%x.\n",localshmtps.debug_cnt_timestamp_missing,localshmtps.error_states);
              localrsuhealth.gnssstatus |= RSUHEALTH_1PPS_BAD_TIMESTAMP;
          }
          localrsuhealth.gnsserrorcounts.badtimestamp = localshmtps.debug_cnt_timestamp_missing;
      }
  }
  /* mktime() for gnss failed. Should never fail i think. */
  if(localshmtps.debug_cnt_time_adjustment_fail_mktime) {
      if(localrsuhealth.gnsserrorcounts.badmktime != localshmtps.debug_cnt_time_adjustment_fail_mktime){
          if(0x0 == (localrsuhealth.gnssstatus & RSUHEALTH_1PPS_BAD_MKTIME)){
              I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"1PPS: mktime failed(%u): 0x%x.\n",localshmtps.debug_cnt_time_adjustment_fail_mktime,localshmtps.error_states);
              localrsuhealth.gnssstatus |= RSUHEALTH_1PPS_BAD_MKTIME;
          }
          localrsuhealth.gnsserrorcounts.badmktime = localshmtps.debug_cnt_time_adjustment_fail_mktime;
      }
  }
  if(localshmtps.debug_cnt_1pps_halt_event){
      if(localrsuhealth.gnsserrorcounts.bad1ppshalt != localshmtps.debug_cnt_1pps_halt_event){
          if(0x0 == (localrsuhealth.gnssstatus & RSUHEALTH_1PPS_HALTED)){
              I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"1PPS: halted (%u): 0x%x.\n",localshmtps.debug_cnt_1pps_halt_event,localshmtps.error_states);
              localrsuhealth.gnssstatus |= RSUHEALTH_1PPS_HALTED;
              setuptrap(TimeSourceLostID,rsuAlertLevel_critical,"[1PPS:HALTED]");
          }
          localrsuhealth.gnsserrorcounts.bad1ppshalt = localshmtps.debug_cnt_1pps_halt_event;
      }
  }
  if(priornotimeacccount < localshmtps.debug_gps_cmd_count_ubx_nav_pvt_no_timeacc) {
      priornotimeacccount = localshmtps.debug_gps_cmd_count_ubx_nav_pvt_no_timeacc;
      if(0x0 == (localrsuhealth.gnssstatus & RSUHEALTH_GNSS_NO_TIME_ACCURACY)){
          I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"GNSS: No time accuracy(%u): tps(0x%x).\n",priornotimeacccount,localshmtps.error_states);
          localrsuhealth.gnssstatus |= RSUHEALTH_GNSS_NO_TIME_ACCURACY;
      }
  } else {
      localrsuhealth.gnssstatus &= ~RSUHEALTH_GNSS_NO_TIME_ACCURACY;
  }
#if defined(EXTRA_DEBUG)
  if(0 == (onesecondcounter % 15)){
      I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"GNSS: time accuracy error(%u) min/max/thresh(%u,%u,%u)\n",
          localshmtps.current_time_accuracy
          ,localshmtps.time_accuracy_min
          ,localshmtps.time_accuracy_max
          ,localshmtps.time_accuracy_threshold);
  }
#endif
  if(localshmtps.time_accuracy_threshold < localshmtps.current_time_accuracy) {
      if(RSU_TIME_TO_FIRST_FIX_MAX <= onesecondcounter) { /* Give GNSS a chance to get a fix. */
          if(0x0 == (localrsuhealth.gnssstatus & RSUHEALTH_GNSS_BAD_TIME_ACCURACY)){
              I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"GNSS: time accuracy usec(%u) exceeds limit(%u): tps(0x%x).\n",localshmtps.current_time_accuracy,localshmtps.time_accuracy_threshold,localshmtps.error_states);
              localrsuhealth.gnssstatus |= RSUHEALTH_GNSS_BAD_TIME_ACCURACY;
              setuptrap(TimeSourceLostID,rsuAlertLevel_critical,"[1PPS:BAD TIME ACCURACY]");
          }
      }
  } else {
      localrsuhealth.gnssstatus &= ~RSUHEALTH_GNSS_BAD_TIME_ACCURACY;
  }
}
/*----------------------------------------------------------------------------*/
/* checkgnss: Checked at ~1Hz seconds.                                        */
/*----------------------------------------------------------------------------*/
static void checkgnss(void)
{
  if(NULL != shm_rsuhealth_ptr) {
  /* 5.18.1.6 Time Source Lost Message: Lost time source: ie lost GNSS fix. */
  /* 5.18.1.7 Time Source Mismatch Message: deviation between two time sources exceeds vendor-defined threshold.*/
  /* 5.18.1.8 GNSS Anomaly Message */

      if(NULL != shm_tps_ptr) {
          if((0 == checkgeodata()) && (0 == checkgnssinterface())) { /* Update tps_pub_geo and ensure tow is incrementing in sane fashion. */
              stuckgeodatacount = 0;

              checkgnssfix();     /* JIRA XXXX: Check we have a good fix and min sv required. */
   
              checkgnsssignal();  /* JIRA 608: Check timely 3D position fix. After X seconds we failed and note. */

              checkgnssinit();    /* JIRA 1671: Check for gps init issues, check ublox cfg and version. */

              checkgnssanomaly(); /* JIRA 353: Check for gnss anomoly. */
 
              checkgnss1pps();    /* JIRA 864: check 1pps init and runtime operation. */
              
          } else {
              stuckgeodatacount++;
              /* If we sample too quick then TPS SHM won't change. */
              if(10 < stuckgeodatacount) {
                  if(localrsuhealth.gnssstatus & RSUHEALTH_GNSS_FIX_VALID) {
                      localrsuhealth.gnssstatus &= ~RSUHEALTH_GNSS_FIX_VALID;
                      I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"GNSS: Lost fix, geo data stuck.\n");
                  }
              }
//TODO: Purge TPS SHM & DATA
              if(0x0 == (stuckgeodatacount % 10)) {
                  I2V_DBG_LOG(LEVEL_PRIV, MY_NAME,"checkgnss event(%u): Lost geo data.\n",stuckgeodatacount); /* Failed serial or gps init, TPS SHM issue or we no new messages from ubx. */
              }
          }
      }

      if(NULL != shm_sous_ptr) { /* Do GNSS and CV2X antennas in one go. */
          memcpy(&localsous,shm_sous_ptr,sizeof(localsous));
          if(RH850_BOOT_WAIT < onesecondcounter){
              checkgnsscv2xantenna(); /* JIRA 347: check sous shm for antenna status */
          }
      }
  }
}
/*----------------------------------------------------------------------------*/
/* Did we reach threshold to request MIB to reset if recovery enabled?        */
/* There's probably persistent intermittent fail types.                       */
/* i.e. Fails for 1 minute then comes back then fails for 1 minute.           */
/* Criteria:                                                                  */
/*     i) no valid gnss fix for prolonged period                              */
/*     ii) attempts to start i2v exhausted                                    */
/*     iii) couldn't touch ready file to signal others we are ready           */
/*----------------------------------------------------------------------------*/
//TODO: add modes rsuMode standby: no tx if cv2x temp or hsm over temp,
static void managerecovery(void)
{
  int32_t fault=0;

  /* lost gnss for prolonged period */
  if(0x0 == (localrsuhealth.gnssstatus & RSUHEALTH_GNSS_FIX_VALID)){
#if defined(EXTRA_DEBUG)
      I2V_DBG_LOG(LEVEL_PRIV, MY_NAME,"RSUHEALTH_FAULT_MODE: No gnss fix(%d)\n",fault_count);
#endif
      fault=1;
  }
//todo, test and consider
  /* attempts to start i2v failed */
  if((RSUHEALTH_I2V_RESET & localrsuhealth.errorstates) && (I2V_FAIL_THRESHOLD < i2vfailcnt)){
#if defined(EXTRA_DEBUG)
      I2V_DBG_LOG(LEVEL_PRIV, MY_NAME,"RSUHEALTH_FAULT_MODE: i2v failed to start(%d,%d)\n",i2vfailcnt,fault_count);
#endif
      fault=1;
  }
  /* rsuhealth couldn't touch ready file */
  if((localrsuhealth.errorstates & RSUHEALTH_READY_SET_FAIL) && (RSUHEALTH_SET_READY_THRESHOLD < ready_set_fault_count)) {
#if defined(EXTRA_DEBUG)
      I2V_DBG_LOG(LEVEL_PRIV, MY_NAME,"RSUHEALTH_FAULT_MODE: set ready failed (%d,%d)\n",ready_set_fault_count,fault_count);
#endif
      fault=1;
  }

  if(fault) {
      fault_count += fault;
  } else {
      fault_count = 0;
  }

  if(RSUHEALTH_FAULT_TIME_THRESHOLD < fault_count) { /* Must be consecutive fails. */
      if(0x0 == (localrsuhealth.errorstates & RSUHEALTH_FAULT_MODE)){
          localrsuhealth.errorstates |= RSUHEALTH_FAULT_MODE; /* Will cause 1218 MIB to issue reset if recovery enabled. */
          rsusystem = RSUHSTATE_FAULTSET;
          /* 5.18.1.14 Enclosure Environment: although maybe not techincally environ? */
          setuptrap(EnvironID,rsuAlertLevel_critical,"[Fault recovery threshold reached]");
          I2V_DBG_LOG(LEVEL_PRIV, MY_NAME,"RSUHEALTH_FAULT_MODE: Fault recovery threshold reached: Issued reset request if enabled.\n");
      }
  } else {
      localrsuhealth.errorstates &= ~(uint64_t)RSUHEALTH_FAULT_MODE;
      rsusystem = RSUHSTATE_NORMAL;
  }
}
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
static void printrskerrorstates(void)
{
   I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"rsk event occurred: rsk(0x%x)\n",rskstatus.error_states);
   if(rskstatus.error_states & CV2X_INIT_FAIL) {
      I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"CV2X_INIT_FAIL\n");
   }
   if(rskstatus.error_states & CV2X_TX_NOT_READY) {
      I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"CV2X_TX_NOT_READY\n");
   } else {
      I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"CV2X_TX_READY\n");
   }
   if(rskstatus.error_states & CV2X_SERVICE_ID_BAD) {
      I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"CV2X_SERVICE_ID_BAD\n");
   }
   if(rskstatus.error_states & CV2X_TX_FAIL) {
      I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"CV2X_TX_FAIL\n");
   }
   if(rskstatus.error_states & CV2X_RX_FAIL) {
      I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"CV2X_RX_FAIL\n");
   }
   if(rskstatus.error_states & CV2X_SOCKET_INIT_FAIL) {
      I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"CV2X_SOCKET_INIT_FAIL\n");
   }
   if(rskstatus.error_states & CV2X_SYS_INIT_FAIL) {
      I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"CV2X_SYS_INIT_FAIL\n");
   }
   if(rskstatus.error_states & CV2X_FW_REVISION_FAIL) {
      I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"CV2X_FW_REVISION_FAIL\n");
   }
   if(rskstatus.error_states & CV2X_INIT_TWICE_WARNING) {
      I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"CV2X_INIT_TWICE_WARNING\n");
   }
   if(rskstatus.error_states & CV2X_WDM_SERVICE_GET_FAIL) {
      I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"CV2X_WDM_SERVICE_GET_FAIL\n");
   }
   if(rskstatus.error_states & CV2X_DDM_SERVICE_GET_FAIL) {
      I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"CV2X_DDM_SERVICE_GET_FAIL\n");
   }
   if(rskstatus.error_states & CV2X_DDM_STATE_GET_FAIL) {
      I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"CV2X_DDM_STATE_GET_FAIL\n");
   }
   if(rskstatus.error_states & CV2X_DDM_SYNC_INIT_FAIL) {
      I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"CV2X_DDM_SYNC_INIT_FAIL\n");
   }
   if(rskstatus.error_states & CV2X_GET_SERVICE_FAIL) {
      I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"CV2X_GET_SERVICE_FAIL\n");
   }
   if(rskstatus.error_states & CV2X_CONFIG_SET_FAIL) {
      I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"CV2X_CONFIG_SET_FAIL\n");
   }
   if(rskstatus.error_states & CV2X_SERVICE_ENABLE_FAIL) {
      I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"CV2X_SERVICE_ENABLE_FAIL\n");
   }
   if(rskstatus.error_states & CV2X_L2ID_SET_FAIL) {
      I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"CV2X_L2ID_SET_FAIL\n");
   }
   if(rskstatus.error_states & CV2X_SOCKET_CREATE_FAIL) {
      I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"CV2X_SOCKET_CREATE_FAIL\n");
   }
   if(rskstatus.error_states & CV2X_TX_DUPE_SERVICE_ID_FAIL) {
      I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"CV2X_TX_DUPE_SERVICE_ID_FAIL\n");
   }
   if(rskstatus.error_states & CV2X_TX_DDM_SERVICE_GET_FAIL) {
      I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"CV2X_TX_DDM_SERVICE_GET_FAIL\n");
   }
   if(rskstatus.error_states & CV2X_TX_SOCKET_CREATE_FAIL) {
      I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"CV2X_TX_SOCKET_CREATE_FAIL\n");
   }
   if(rskstatus.error_states & CV2X_TX_SOCKET_POLICY_FAIL) {
      I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"CV2X_TX_SOCKET_POLICY_FAIL\n");
   }
   if(rskstatus.error_states & CV2X_SPS_FLOWS_USED_FAIL) {
      I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"CV2X_SPS_FLOWS_USED_FAIL\n");
   }
   if(rskstatus.error_states & CV2X_TX_RADIO_NOT_READY_FAIL) {
      I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"CV2X_TX_RADIO_NOT_READY_FAIL\n");
   }
   if(rskstatus.error_states & CV2X_DDM_TSF_GET_FAIL) {
      I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"CV2X_DDM_TSF_GET_FAIL\n");
   }
   if(rskstatus.error_states & CV2X_GET_TALLIES_FAIL) {
      I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"CV2X_GET_TALLIES_FAIL\n");
   }
   if(rskstatus.error_states & CV2X_PCAP_WRITE_SEM_WAIT_FAIL) {
      I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"CV2X_PCAP_WRITE_SEM_WAIT_FAIL\n");
   }
   if(rskstatus.error_states & CV2X_PCAP_READ_SEM_WAIT_FAIL) {
      I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"CV2X_PCAP_READ_SEM_WAIT_FAIL\n");
   }
}
/*----------------------------------------------------------------------------*/
/* Check RSK & CV2X status                                                    */
/*----------------------------------------------------------------------------*/
static void checkrsk(void)
{
  char_t nextmsg[64];

  if(0 < nsstats_c()) {
      localrsuhealth.temperature.cv2x = rskstatus.temperature;
      if(0 == (onesecondcounter % SYSLOG_OUTPUT_MODULUS)) {
          I2V_DBG_LOG(LEVEL_PRIV, MY_NAME,"rsk(0x%x): int|fw|tm|rdy(%d,%d,%d,%d) tx|txE|rx|rxE(%u,%u,%u,%u): wme rx|rxE|txNotR(%u,%u,%u)\n",
              rskstatus.error_states
             ,rskstatus.interface
             ,rskstatus.firmware
             ,rskstatus.temperature
             ,rskstatus.ready
             ,rskstatus.tx_count
             ,rskstatus.tx_err_count
             ,rskstatus.rx_count
             ,rskstatus.rx_err_count
             ,rskstatus.wme_rx_good
             ,rskstatus.wme_rx_bad
             ,rskstatus.wme_tx_not_ready);
      }
      localrsuhealth.errorstates &= ~(uint64_t)RSUHEALTH_NSSTATS_FAILED;
  } else {
      if(0x0 == (localrsuhealth.errorstates & RSUHEALTH_NSSTATS_FAILED)){
          I2V_DBG_LOG(LEVEL_PRIV, MY_NAME,"nsstats_c failed.\n");
          localrsuhealth.errorstates |= RSUHEALTH_NSSTATS_FAILED;
      }
  }
  /* 
   * JIRA 610: V2X radio data communication interface is lost.
   * Worth noting we only check at init time.
   */ 
  if(0 == rskstatus.interface) {
      if(0x0 == (localrsuhealth.cv2xstatus & RSUHEALTH_CV2X_BAD_INTERFACE)) {
          I2V_DBG_LOG(LEVEL_CRIT, MY_NAME,"RSUHEALTH_CV2X_BAD_INTERFACE: CV2X interface has been lost.\n");
          localrsuhealth.cv2xstatus |= RSUHEALTH_CV2X_BAD_INTERFACE;
      }
  } else {
      if(localrsuhealth.cv2xstatus & RSUHEALTH_CV2X_BAD_INTERFACE) {
          localrsuhealth.cv2xstatus &= ~RSUHEALTH_CV2X_BAD_INTERFACE;
          I2V_DBG_LOG(LEVEL_INFO, MY_NAME,"~RSUHEALTH_CV2X_BAD_INTERFACE: CV2X interface is AOK.\n");
      }
  }
//TODO: make sure denso update reflected in revision of file in bsp
  /* JIRA 611: V2X Radio Firmware fail */
  if(0 == rskstatus.firmware) {
      localrsuhealth.cv2xstatus |= RSUHEALTH_CV2X_BAD_FW_REVISION;
  } else {
      localrsuhealth.cv2xstatus &= ~RSUHEALTH_CV2X_BAD_FW_REVISION;
  }
  /* JIRA 612: V2X Radio Init fail */
  if(0 == rskstatus.ready) {
      localrsuhealth.cv2xstatus |= RSUHEALTH_CV2X_INIT_FAIL;
  } else {
      localrsuhealth.cv2xstatus &= ~RSUHEALTH_CV2X_INIT_FAIL;
  }
  /* JIRA 614: V2X Radio Over temp */
  /* Check critical range. */
  if(   (rskstatus.temperature <  SECTON_TEMP_LOW_CRITICAL)
     || (SECTON_TEMP_HIGH_CRITICAL < rskstatus.temperature)){
      if(0x0 == (localrsuhealth.cv2xstatus & RSUHEALTH_CV2X_TEMP_FAIL)){
          memset(nextmsg,0x0,sizeof(nextmsg));
          snprintf(nextmsg,sizeof(nextmsg),"[CV2X temp in CRITICAL range(%d)]",rskstatus.temperature);
          setuptrap(EnvironID, rsuAlertLevel_critical,nextmsg); 
          localrsuhealth.cv2xstatus |= RSUHEALTH_CV2X_TEMP_FAIL;
          I2V_DBG_LOG(LEVEL_CRIT, MY_NAME,"RSUHEALTH_CV2X_TEMP_FAIL: cv2xtemperature(%d)\n",rskstatus.temperature);
      }
      localrsuhealth.cv2xerrorcounts.badcv2xtemperature++;
  } else {
      localrsuhealth.cv2xstatus &= ~RSUHEALTH_CV2X_TEMP_FAIL;
  }
  /* Check warn range. */
  if(   ((SECTON_TEMP_LOW_CRITICAL < rskstatus.temperature) && (rskstatus.temperature < SECTON_TEMP_LOW_WARN))
     || ((SECTON_TEMP_HIGH_WARN    < rskstatus.temperature) && (rskstatus.temperature < SECTON_TEMP_HIGH_CRITICAL))){
      if(0x0 == (localrsuhealth.cv2xstatus & RSUHEALTH_CV2X_TEMP_WARN)){
          memset(nextmsg,0x0,sizeof(nextmsg));
          snprintf(nextmsg,sizeof(nextmsg),"[CV2X temp in WARN range(%d)]",rskstatus.temperature);
          setuptrap(EnvironID, rsuAlertLevel_warning,nextmsg);                      
          I2V_DBG_LOG(LEVEL_CRIT, MY_NAME,"RSUHEALTH_CV2X_TEMP_WARN: cv2xtemperature(%d)\n",rskstatus.temperature);
          localrsuhealth.cv2xstatus |= RSUHEALTH_CV2X_TEMP_WARN;
      }
  } else {
      localrsuhealth.cv2xstatus &= ~RSUHEALTH_CV2X_TEMP_WARN;
  }
}
/*----------------------------------------------------------------------------*/
/* Check antennas are good and we have valid gnss fix.                        */
/*----------------------------------------------------------------------------*/
static void checkcv2x(void)
{
  /* Depends on TPS and SOUS. IWMH depends on RSUHEALTH SHM */
  localrsuhealth.cv2xstatus &= ~RSUHEALTH_CV2X_TX_READY; /* Not ready till proven ready. */
  localrsuhealth.cv2xstatus &= ~RSUHEALTH_CV2X_RX_READY;
  if(RSUHEALTH_AT_WAIT_TIME < onesecondcounter){ /* Magic wait time needed else temp check kills AT */
      checkrsk(); /* This test doesn't depend on SHM. */
  }
  if((NULL != shm_rsuhealth_ptr) && (NULL != shm_tps_ptr) && (NULL != shm_sous_ptr)) {
      if(0x0 == (localrsuhealth.cv2xstatus & (RSUHEALTH_CV2X1_BAD_ANTENNA|RSUHEALTH_CV2X2_BAD_ANTENNA))) {
          if(   (localrsuhealth.gnssstatus & RSUHEALTH_GNSS_FIX_VALID) 
             && (0 == (localrsuhealth.gnssstatus & RSUHEALTH_GNSS_BAD_LOC_DEV))
             && (0 == (localrsuhealth.gnssstatus & RSUHEALTH_GNSS_NO_TIME_ACCURACY))
             && (0 == (localrsuhealth.gnssstatus & RSUHEALTH_GNSS_BAD_TIME_ACCURACY))
             && (0 == (rskstatus.error_states & CV2X_TX_NOT_READY))) {
              localrsuhealth.cv2xstatus |= RSUHEALTH_CV2X_TX_READY;
          }
          localrsuhealth.cv2xstatus |= RSUHEALTH_CV2X_RX_READY; 
      }
  }
  if(RSUHEALTH_AT_WAIT_TIME < onesecondcounter){ /* Magic wait time needed else temp check kills AT */
      if(   (localrsuhealth.cv2xstatus & RSUHEALTH_CV2X_TEMP_FAIL) 
         || (localrsuhealth.cv2xstatus & RSUHEALTH_CV2X_BAD_FW_REVISION)
         || (localrsuhealth.cv2xstatus & RSUHEALTH_CV2X_BAD_INTERFACE)
         || (localrsuhealth.cv2xstatus & RSUHEALTH_CV2X_INIT_FAIL)){
          localrsuhealth.cv2xstatus &= ~RSUHEALTH_CV2X_TX_READY;
          localrsuhealth.cv2xstatus &= ~RSUHEALTH_CV2X_RX_READY;
      }
  }
  if(RSUHEALTH_STANDBY_MODE & localrsuhealth.errorstates) {
      localrsuhealth.cv2xstatus &= ~RSUHEALTH_CV2X_TX_READY;
      localrsuhealth.cv2xstatus &= ~RSUHEALTH_CV2X_RX_READY;
  }
}
/*----------------------------------------------------------------------------*/
//TODO: Doesn't help web gui with its get one time stuff.
//TODO: lets us know HSM Temp is not going to work right for us. Wrong applet
//TODO: If web gui uses v2xse-info-example then yes cause this will be the only way to know query failed.
/*----------------------------------------------------------------------------*/
#if defined(ENABLE_HSM_DAEMON)
static void gethsmdenso(void)
{
  int32_t   i=0;
  FILE    * fp = NULL;
  char_t    replyBuf[128]; /* Size according to your needs. */
  char_t    config[32];

  memset(replyBuf,'\0',sizeof(replyBuf));
  memset(config,'\0',sizeof(config));
  if(NULL != (fp = POPEN(GET_HSM_DENSO_REV, "r"))) {
      if(NULL != fgets(replyBuf, sizeof(replyBuf), fp)){
          i = strnlen(replyBuf,sizeof(replyBuf));
          if(0<i) {replyBuf[i-1] = '\0';} /* Delete extra char added. */
          if(1 == sscanf(&replyBuf[1],GET_HSM_DENSO_REV_REPLY,config)) {
              if(0 != strcmp(config,HSM_DENSO_REV)){
                  localrsuhealth.cv2xstatus |= RSUHEALTH_HSM_BAD_DENSO;
              } else {
                  localrsuhealth.cv2xstatus &= ~RSUHEALTH_HSM_BAD_DENSO;
              }
          } else {
              localrsuhealth.cv2xstatus |= RSUHEALTH_HSM_SSCANF_FAIL;
              localrsuhealth.cv2xstatus |= RSUHEALTH_HSM_BAD_DENSO;
          }
      } else {
          localrsuhealth.cv2xstatus  |= RSUHEALTH_HSM_FGETS_FAIL;
          localrsuhealth.cv2xstatus  |= RSUHEALTH_HSM_BAD_DENSO;
      }
      PCLOSE(fp);
  } else {
      localrsuhealth.cv2xstatus |= RSUHEALTH_HSM_POPEN_FAIL;
      localrsuhealth.cv2xstatus |= RSUHEALTH_HSM_BAD_DENSO;
  }
}
#endif
#if 0 /* TODO: API is disruptive */
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
static void gethsmgsa(void)
{
  int32_t   i=0;
  FILE    * fp = NULL;
  char_t    replyBuf[128]; /* Size according to your needs. */
  char_t    config[32];

  memset(replyBuf,'\0',sizeof(replyBuf));
  memset(config,'\0',sizeof(config));
  if(NULL != (fp = POPEN(GET_HSM_GSA, "r"))) {
      if(NULL != fgets(replyBuf, sizeof(replyBuf), fp)){
          i = strnlen(replyBuf,sizeof(replyBuf));
          if(0<i) {replyBuf[i-1] = '\0';} /* Delete extra char added. */
          if(1 == sscanf(&replyBuf[1],GET_HSM_GSA_REPLY,config)) {
              if(0 != strcmp(config,HSM_GSA_VERSION)){
                  localrsuhealth.cv2xstatus |= RSUHEALTH_HSM_BAD_GSA;
              } else {
                  localrsuhealth.cv2xstatus &= ~RSUHEALTH_HSM_BAD_GSA;
              }
          } else {
              localrsuhealth.cv2xstatus |= RSUHEALTH_HSM_SSCANF_FAIL;
              localrsuhealth.cv2xstatus |= RSUHEALTH_HSM_BAD_GSA;
          }
      } else {
          localrsuhealth.cv2xstatus  |= RSUHEALTH_HSM_FGETS_FAIL;
          localrsuhealth.cv2xstatus  |= RSUHEALTH_HSM_BAD_GSA;
      }
      PCLOSE(fp);
  } else {
      localrsuhealth.cv2xstatus |= RSUHEALTH_HSM_POPEN_FAIL;
      localrsuhealth.cv2xstatus |= RSUHEALTH_HSM_BAD_GSA;
  }
}
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
static void gethsmusapp(void)
{
  int32_t   i=0;
  FILE    * fp = NULL;
  char_t    replyBuf[128]; /* Size according to your needs. */
  char_t    config[32];

  memset(replyBuf,'\0',sizeof(replyBuf));
  memset(config,'\0',sizeof(config));
  if(NULL != (fp = POPEN(GET_HSM_USAPP, "r"))) {
      if(NULL != fgets(replyBuf, sizeof(replyBuf), fp)){
          i = strnlen(replyBuf,sizeof(replyBuf));
          if(0<i) {replyBuf[i-1] = '\0';} /* Delete extra char added. */
          if(1 == sscanf(&replyBuf[1],GET_HSM_USAPP_REPLY,config)) {
              if(0 != strcmp(config,HSM_USAPP_VERSION)){
                  localrsuhealth.cv2xstatus |= RSUHEALTH_HSM_BAD_USAPP;
              } else {
                  localrsuhealth.cv2xstatus &= ~RSUHEALTH_HSM_BAD_USAPP;
              }
          } else {
              localrsuhealth.cv2xstatus |= RSUHEALTH_HSM_SSCANF_FAIL;
              localrsuhealth.cv2xstatus |= RSUHEALTH_HSM_BAD_USAPP;
          }
      } else {
          localrsuhealth.cv2xstatus |= RSUHEALTH_HSM_FGETS_FAIL;
          localrsuhealth.cv2xstatus |= RSUHEALTH_HSM_BAD_USAPP;
      }
      PCLOSE(fp);
  } else {
      localrsuhealth.cv2xstatus |= RSUHEALTH_HSM_POPEN_FAIL;
      localrsuhealth.cv2xstatus |= RSUHEALTH_HSM_BAD_USAPP;
  }
}
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
static void gethsmjcop(void)
{
  int32_t   i=0;
  FILE    * fp = NULL;
  char_t    replyBuf[128]; /* Size according to your needs. */
  char_t    config[32];

  memset(replyBuf,'\0',sizeof(replyBuf));
  memset(config,'\0',sizeof(config));
  if(NULL != (fp = POPEN(GET_HSM_JCOP, "r"))) {
      if(NULL != fgets(replyBuf, sizeof(replyBuf), fp)){
          i = strnlen(replyBuf,sizeof(replyBuf));
          if(0<i) {replyBuf[i-1] = '\0';} /* Delete extra char added. */
              #if defined(EXTRA_EXTRA_DEBUG)
              I2V_DBG_LOG(LEVEL_DBG, MY_NAME,"HSM_JCOP_VERSION: have (%s) vs want (%s)\n",replyBuf,HSM_JCOP_VERSION);
              #endif
          if(1 == sscanf(&replyBuf[1],GET_HSM_JCOP_REPLY,config)) {
              if(0 != strcmp(config,HSM_JCOP_VERSION)){
                  localrsuhealth.cv2xstatus |= RSUHEALTH_HSM_BAD_JCOP;
              } else {
                  localrsuhealth.cv2xstatus &= ~RSUHEALTH_HSM_BAD_JCOP;
              }
          } else {
              localrsuhealth.cv2xstatus |= RSUHEALTH_HSM_SSCANF_FAIL;
              localrsuhealth.cv2xstatus |= RSUHEALTH_HSM_BAD_JCOP;
          }
      } else {
          localrsuhealth.cv2xstatus |= RSUHEALTH_HSM_FGETS_FAIL;
          localrsuhealth.cv2xstatus |= RSUHEALTH_HSM_BAD_JCOP;
      }
      PCLOSE(fp);
  } else {
      localrsuhealth.cv2xstatus |= RSUHEALTH_HSM_POPEN_FAIL;
      localrsuhealth.cv2xstatus |= RSUHEALTH_HSM_BAD_JCOP;
  }
}
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
static void gethsmconfig(void)
{
  int32_t   i=0;
  FILE    * fp = NULL;
  char_t    replyBuf[128]; /* Size according to your needs. */
  char_t    config[16];

  memset(replyBuf,'\0',sizeof(replyBuf));
  memset(config,'\0',sizeof(config));
  if(NULL != (fp = POPEN(GET_HSM_CFG, "r"))) {
      if(NULL != fgets(replyBuf, sizeof(replyBuf), fp)){
          i = strnlen(replyBuf,sizeof(replyBuf));
          if(0<i) {replyBuf[i-1] = '\0';} /* Delete extra char added. */
          #if defined(EXTRA_EXTRA_DEBUG)
          I2V_DBG_LOG(LEVEL_DBG, MY_NAME,"CFG_VERSION: have (%s) vs want (%s)\n",replyBuf,HSM_PLATFORM_CONFIG);
          #endif
          if(1 == sscanf(&replyBuf[1],GET_HSM_CFG_REPLY,config)) {
              if(0 != strcmp(config,HSM_PLATFORM_CONFIG)){
                  localrsuhealth.cv2xstatus |= RSUHEALTH_HSM_BAD_CFG;
              } else {
                  localrsuhealth.cv2xstatus &= ~RSUHEALTH_HSM_BAD_CFG;
              }
          } else {
              localrsuhealth.cv2xstatus |= RSUHEALTH_HSM_SSCANF_FAIL;
              localrsuhealth.cv2xstatus |= RSUHEALTH_HSM_BAD_CFG;
          }
      } else {
          localrsuhealth.cv2xstatus |= RSUHEALTH_HSM_FGETS_FAIL;
          localrsuhealth.cv2xstatus |= RSUHEALTH_HSM_BAD_CFG;
      }
      PCLOSE(fp);
  } else {
      localrsuhealth.cv2xstatus |= RSUHEALTH_HSM_POPEN_FAIL;
      localrsuhealth.cv2xstatus |= RSUHEALTH_HSM_BAD_CFG;
  }
}
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
static void gethsmtemperature(void)
{
  uint32_t  i = 0;
  FILE    * fp = NULL;
  char_t    replyBuf[64]; /* Size according to your needs. */
  float64_t hsmtemperature = -999.99;

  memset(replyBuf,'\0',sizeof(replyBuf));
  if(NULL != (fp = POPEN(GET_HSM_TEMP, "r"))) {
      if(NULL != fgets(replyBuf, sizeof(replyBuf), fp)){
          i = strnlen(replyBuf,sizeof(replyBuf));
          if((HSM_TEMP_MIN_LENGTH <= i) && (i <= HSM_TEMP_MAX_LENGTH)){
              replyBuf[i-1] = '\0'; /* delete extra char added. */
          } else { 
              replyBuf[0] = '\0'; /* force scanf to fail */
          }
          if(1 == sscanf(replyBuf,GET_HSM_TEMP_REPLY,&hsmtemperature)) {
              #if defined(EXTRA_EXTRA_DEBUG)
              I2V_DBG_LOG(LEVEL_DBG, MY_NAME,"hsmtemperature(%lf)\n",hsmtemperature);
              #endif
              if(   (hsmtemperature < (float64_t) I2V_TEMP_LOW_CELSIUS_THRESHOLD) 
                 || ((float64_t) I2V_TEMP_HIGH_CELSIUS_THRESHOLD < hsmtemperature)){
                  if(0x0 == (localrsuhealth.cv2xstatus & RSUHEALTH_HSM_TEMP_FAIL)){
                      I2V_DBG_LOG(LEVEL_CRIT, MY_NAME,"RSUHEALTH_HSM_TEMP_FAIL: hsmtemperature(%lf)\n",hsmtemperature);
                      localrsuhealth.cv2xstatus |= RSUHEALTH_HSM_TEMP_FAIL;
                  }
                  localrsuhealth.cv2xerrorcounts.badhsmtemperature++;
              } else {
                  localrsuhealth.cv2xstatus &= ~RSUHEALTH_HSM_TEMP_FAIL;
                  localrsuhealth.temperature.hsm = hsmtemperature;
              }
          } else {
              localrsuhealth.cv2xstatus |= RSUHEALTH_HSM_SSCANF_FAIL;
              localrsuhealth.cv2xstatus |= RSUHEALTH_HSM_TEMP_FAIL;
              hsmtemperature = -999.98;
          }
      } else {
          localrsuhealth.cv2xstatus |= RSUHEALTH_HSM_FGETS_FAIL;
          localrsuhealth.cv2xstatus |= RSUHEALTH_HSM_TEMP_FAIL;
          hsmtemperature = -999.97;
      }
      PCLOSE(fp);
  } else {
      localrsuhealth.cv2xstatus |= RSUHEALTH_HSM_POPEN_FAIL;
      localrsuhealth.cv2xstatus |= RSUHEALTH_HSM_TEMP_FAIL;
      hsmtemperature = -999.96;
  }
}
/*----------------------------------------------------------------------------*/
/* If we can pass the tests below then assume we have fullfilled these items  */
/* JIRA 1668: HSM Interface Fail.                                             */
/* JIRA 1669: HSM Initialization Fail.                                        */
/* JIRA 1670: HSM Over Temperature.                                           */
/*----------------------------------------------------------------------------*/
static void checkhsm(void)
{
  gethsmtemperature(); /* always check temperature */

//TODO: Do exactly once dumping v2xse-se-info to tmp for us and webgui.
//    : there is a reset at the end of this call so do just one time.

  if(checkhsmcount < CHECK_HSM_CONFIG_MAX) { /* once verfied, stop. allow so many fails then give up */
      gethsmconfig();
      gethsmjcop();
      gethsmusapp();
      gethsmgsa();
#if defined(ENABLE_HSM_DAEMON)
      gethsmdenso(); /* if we don't find the denso rev then wrong bsp update */
#endif
      if (localrsuhealth.cv2xstatus & RSUHEALTH_HSM_BAD_CFG_MASK) {
          checkhsmcount++; /* keep trying */
          I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"HSM config check failed(%d)\n",checkhsmcount);
      } else {
         I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"HSM config check success(%d)\n",checkhsmcount);
         checkhsmcount = CHECK_HSM_CONFIG_MAX; /* status is all clear so jump to end of count. */ 
      }
  }
  /* Can only make not ready. Can't make txrx valid by itself. */
#if 0 
//TODO too strict? If radio is working then keep going. Sign & verify fails will pile up so easy to see real problem
  if(localrsuhealth.cv2xstatus & (RSUHEALTH_HSM_TEMP_FAIL|RSUHEALTH_HSM_BAD_CFG_MASK)){
#else
  if(localrsuhealth.cv2xstatus & (RSUHEALTH_HSM_TEMP_FAIL)){
#endif
      localrsuhealth.cv2xstatus &= ~RSUHEALTH_CV2X_TX_READY;
      localrsuhealth.cv2xstatus &= ~RSUHEALTH_CV2X_RX_READY;
  }
}
#endif
/*----------------------------------------------------------------------------*/
/*
  rsu-202303162108.acf

  certificate expires on March 23 at 21:08 UTC. 
  Cert downloaded time + 7 days. 
  In this case downloaded time was 2023 March 16 at 21:08.
*/
/*----------------------------------------------------------------------------*/
static int32_t checkacffile(char_t * acf_file)
{
/* Does not include any leap second events but... 
 * we add 2 minutes to allow expire and clean by aerolink. 
 */
#define SECONDS_IN_WEEK_PLUS_EXPIRE_DWELL ((60 * 60 * 24 * 7) + 120) 
  int32_t  ret = 0;
  int32_t  i=0;
  uint32_t utcsec = 0;
  uint32_t acfexpiresec = 0;
  /* do not use "clock" since linux has clock() function. */
  i2vTimeT myclock;
  int32_t  isleap = 0;

  if(NULL == acf_file) {
      ret = -1;
  } else {
      /* check if welformed file name */
      if((0 == strlen(acf_file)) || (RSU_ACF_FILENAME_MAX_LENGTH < strlen(acf_file))) {
          ret = -2;
      } else {
//TODO: unit test kudo
          if(('r' != acf_file[0]) || ('s' != acf_file[1]) || ('u' != acf_file[2]) || ('-' != acf_file[3])) {
              ret = -2;
          } else {
              for(i=4,ret=0;(0==ret) && (i<16);i++) {
                  if( (acf_file[i] < '0') || ('9' < acf_file[i])) {
                      ret = -2;
                      break;
                  }
              }
          }
      }
      if(0 == ret) {   
         /* get current utc second */
         memset(&myclock,0x0,sizeof(myclock));
         i2vUtilGetUTCTime(&myclock); /* blank means current time */
         utcsec = CONVERT_TO_UTC(myclock.year, myclock.yday, myclock.hour, myclock.min, myclock.sec);

         /* strip out date and convert to UTC seconds */
         memset(&myclock,0x0,sizeof(myclock));
         myclock.year    = (uint16_t)((acf_file[4] - 48 ) * 1000);   
         myclock.year   += (uint16_t)((acf_file[5] - 48 ) * 100);
         myclock.year   += (uint16_t)((acf_file[6] - 48 ) * 10);
         myclock.year   += (uint16_t)(acf_file[7]  - 48);

         myclock.month  = (uint8_t)(((acf_file[8]  - 48) * 10) + (acf_file[9] - 48));
         myclock.day    = (uint8_t)(((acf_file[10] - 48) * 10) + (acf_file[11] - 48));
         myclock.hour   = (uint8_t)(((acf_file[12] - 48) * 10) + (acf_file[13] - 48));
         myclock.min    = (uint8_t)(((acf_file[14] - 48) * 10) + (acf_file[15] - 48));
         /* basic error checking on timestamp given */
//TODO: 200 years good?
         if((myclock.year < 2023) || (2223 < myclock.year)) {
             ret = -2;
         }
         if((0 == myclock.month) || (12 < myclock.month)) {
             ret = -2;
         }
         if((0 == myclock.day) || (31 < myclock.day)) { /* Not checking calendar day */
             ret = -2;
         }
         if(23 < myclock.hour) {
             ret = -2;
         }
         if(59 < myclock.sec) {
             ret = -2;
         }
     }
//TODO: Good till 2100 which is not a leap year.
     if(0 == ret) {
         if(0 == (myclock.year % 4)) {
             isleap = 1; /* Get leap day */
         }
         /* Now calculate year day. Month error checked above. */
         switch(myclock.month) {
             case 1:
                 myclock.yday = (uint16_t)myclock.day;
                 break;
             case 2:
                 myclock.yday = (uint16_t)(myclock.day + 31);
                 break;
             case 3:
                 myclock.yday = (uint16_t)(myclock.day + 31 + 28 + isleap);
                 break;
             case 4:
                 myclock.yday = (uint16_t)(myclock.day + 31 + 28 + isleap + 31);
                 break;
             case 5:
                 myclock.yday = (uint16_t)(myclock.day + 31 + 28 + isleap + 31 + 30);
                 break;
             case 6:
                 myclock.yday = (uint16_t)(myclock.day + 31 + 28 + isleap + 31 + 30 + 31);
                 break;
             case 7:
                 myclock.yday = (uint16_t)(myclock.day + 31 + 28 + isleap + 31 + 30 + 31 + 30);
                 break;
             case 8:
                 myclock.yday = (uint16_t)(myclock.day + 31 + 28 + isleap + 31 + 30 + 31 + 30 + 31);
                 break;
             case 9:
                 myclock.yday = (uint16_t)(myclock.day + 31 + 28 + isleap + 31 + 30 + 31 + 30 + 31 + 31);
                 break;
             case 10:
                 myclock.yday = (uint16_t)(myclock.day + 31 + 28 + isleap + 31 + 30 + 31 + 30 + 31 + 31 + 30);
                 break;
             case 11:
                 myclock.yday = (uint16_t)(myclock.day + 31 + 28 + isleap + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31);
                 break;
             case 12:
                 myclock.yday = (uint16_t)(myclock.day + 31 + 28 + isleap + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31 + 30);
                 break;
         }
         myclock.yday = myclock.yday - 1; /* Subtract 1 day cause calender day 1 is really day 0 plus  hrs+min+sec etc. */
         acfexpiresec = CONVERT_TO_UTC(myclock.year, myclock.yday, myclock.hour, myclock.min, myclock.sec);
         acfexpiresec = acfexpiresec + SECONDS_IN_WEEK_PLUS_EXPIRE_DWELL;
         if(acfexpiresec <= utcsec) {
             ret = -3; /* expired */
             #if defined(EXTRA_DEBUG)
             I2V_DBG_LOG(LEVEL_PRIV,MY_NAME,"Expired: acfexpiresec(%u) <= utcsec(%u)\n",
                 acfexpiresec,utcsec);
             #endif
         } else {
             if((acfexpiresec - utcsec) < RSU_ACF_TIMEOUT_COUNT) {
                 ret = -4; /* expiring */
                 #if defined(EXTRA_DEBUG)
                 I2V_DBG_LOG(LEVEL_PRIV,MY_NAME,"Ready to expire: acfexpiresec(%u) - utcsec(%u)=(%u) < (%u)\n",
                     acfexpiresec,utcsec,(acfexpiresec - utcsec),RSU_ACF_TIMEOUT_COUNT);
                 #endif
             } else {
                 ret = 0; /* ok */
                 #if defined(EXTRA_DEBUG)
                 I2V_DBG_LOG(LEVEL_PRIV,MY_NAME,"Cert AOK: acfexpiresec(%u) > utcsec(%u) life left(%u)\n",
                     acfexpiresec,utcsec,(acfexpiresec - utcsec));
                 #endif
             }
         }
         #if defined(EXTRA_DEBUG)
         I2V_DBG_LOG(LEVEL_PRIV,MY_NAME,"Date:%u:%u:%u: %u:%u:%u\n",myclock.year,myclock.day, myclock.yday, myclock.hour, myclock.min, myclock.sec);
         I2V_DBG_LOG(LEVEL_PRIV,MY_NAME,"acfexpiresec(%u) - utcsec(%u)=(%u)\n",
             acfexpiresec,utcsec,(acfexpiresec - utcsec));
         #endif
      }
  }
  return ret;
#undef SECONDS_IN_WEEK
}
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
static void dumpalsmistats(void)
{
  char_t nextmsg[64]; /* size accordingly */

  I2V_DBG_LOG(LEVEL_PRIV, MY_NAME,"ALSMI: Sign Requests: %u\n", rskstatus.SignRequests);
  I2V_DBG_LOG(LEVEL_PRIV, MY_NAME,"ALSMI: Sign Successes: %u\n", rskstatus.SignSuccesses);
  /* 5.18.1.4 Signature Verification Error Message: Any failed signature on WSM's. */
  I2V_DBG_LOG(LEVEL_PRIV, MY_NAME,"ALSMI: Sign Failures: %u\n", rskstatus.SignFailures);
  if(rskstatus.SignFailures) {
      if(0x0 == (localrsuhealth.cv2xstatus & RSUHEALTH_ALSMI_SIGN_FAIL)) {
          localrsuhealth.cv2xstatus |= RSUHEALTH_ALSMI_SIGN_FAIL;
          memset(nextmsg,0x0,sizeof(nextmsg));
          snprintf(nextmsg,sizeof(nextmsg),"[Sign Fail: %u][Last Sign Error Code: %u]", rskstatus.SignFailures,rskstatus.LastSignErrorCode);
          setuptrap(SignatureVerifyID,rsuAlertLevel_warning,nextmsg);
      }
  }
  I2V_DBG_LOG(LEVEL_PRIV, MY_NAME,"ALSMI: Last Sign Error Code: %u\n", rskstatus.LastSignErrorCode);
  I2V_DBG_LOG(LEVEL_PRIV, MY_NAME,"ALSMI: Successful Stripped Messages: %u\n", rskstatus.StripSuccesses);
  I2V_DBG_LOG(LEVEL_PRIV, MY_NAME,"ALSMI: Unsuccessful Stripped Messages: %u\n", rskstatus.StripFailures);
  I2V_DBG_LOG(LEVEL_PRIV, MY_NAME,"ALSMI: Verification Requests: %u\n", rskstatus.VerifyRequests);
  I2V_DBG_LOG(LEVEL_PRIV, MY_NAME,"ALSMI: Verification Successes: %u\n", rskstatus.VerifySuccesses);
  /* 5.18.1.4 Signature Verification Error Message: Any failed signature on WSM's. */
  I2V_DBG_LOG(LEVEL_PRIV, MY_NAME,"ALSMI: Verification Failures: %u\n", rskstatus.VerifyFailures);
  if(rskstatus.VerifyFailures) {
      if(0x0 == (localrsuhealth.cv2xstatus & RSUHEALTH_ALSMI_VERIFY_FAIL)) {
          localrsuhealth.cv2xstatus |= RSUHEALTH_ALSMI_VERIFY_FAIL;
          memset(nextmsg,0x0,sizeof(nextmsg));
          snprintf(nextmsg,sizeof(nextmsg),"[Verify Fails: %u]", rskstatus.VerifyFailures);
          setuptrap(SignatureVerifyID,rsuAlertLevel_warning,nextmsg);
      }
  }
  I2V_DBG_LOG(LEVEL_PRIV, MY_NAME,"ALSMI: Last Verification Error Code: %u\n",  rskstatus.LastVerifyErrorCode);
#if 0
  I2V_DBG_LOG(LEVEL_PRIV, MY_NAME,"ALSMI: P2P Transmitted: %u\n", rskstatus.P2PCallbackCalled);
  I2V_DBG_LOG(LEVEL_PRIV, MY_NAME,"ALSMI: P2P Tx Success: %u\n", rskstatus.P2PCallbackSendSuccess);
  I2V_DBG_LOG(LEVEL_PRIV, MY_NAME,"ALSMI: P2P Tx Fail: %u\n", rskstatus.P2PCallbackNoRadioConfigured + rskstatus.P2PCallbackLengthError + rskstatus.P2PCallbackSendError);
  I2V_DBG_LOG(LEVEL_PRIV, MY_NAME,"ALSMI: P2P Received: %u\n", rskstatus.P2PReceived);
  I2V_DBG_LOG(LEVEL_PRIV, MY_NAME,"ALSMI: P2P Process Success: %u\n", rskstatus.P2PProcessed);
  I2V_DBG_LOG(LEVEL_PRIV, MY_NAME,"ALSMI: P2P Process Fail: %u\n", rskstatus.P2PProcessError);
#endif
}
/*----------------------------------------------------------------------------*/
/* JIRA 847: any security credential has expired or is missing. aka failed.   */
/*----------------------------------------------------------------------------*/
static void checksecurity(void)
{
  int32_t   validcertcount = 0;
  int32_t   abouttoexpirecount = 0;
  int32_t   expiredcount = 0;
  int32_t   boguscount = 0;
  int32_t   i = 0;
  FILE    * fp = NULL;
  char_t    replyBuf[RSU_ACF_FILENAME_MAX_LENGTH * RSU_ACF_MAX_COUNT]; /* Size according to your needs. */
  char_t    acf_file[64];
  char_t    nextmsg[64]; /* size accordingly */

  /* check aerolink folder exists */
  if(WFALSE == i2vCheckDirOrFile(RSU_AEROLINK_CERTS_LOCATION)){
      localrsuhealth.cv2xstatus |= RSUHEALTH_AEROLINK_DIR_FAIL;
  } else {
      localrsuhealth.cv2xstatus &= ~RSUHEALTH_AEROLINK_DIR_FAIL;
      memset(replyBuf,'\0',sizeof(replyBuf));
      memset(acf_file,'\0',sizeof(acf_file));
      /* check .acf files exist and if ready to expire */
      if (NULL != (fp = popen(RSU_AEROLINK_GET_CERTS, "r"))) {
          validcertcount = 0;
          abouttoexpirecount = 0;
          expiredcount = 0;
          boguscount = 0;
          for(i=0; (i < RSU_ACF_MAX_COUNT) && (validcertcount < RSU_ACF_MAX_COUNT); i++){
              if(NULL != fgets(replyBuf, sizeof(replyBuf), fp)){
                  if(1 == sscanf(replyBuf,RSU_AEROLINK_CERTS_ACF,acf_file)) {
                      switch(checkacffile(acf_file)){
                          case -4:                              
                              abouttoexpirecount++; /* about to expire */
                              validcertcount++;
                              /* Do SEL output here so we have acf name handy. */
                              if(0x0 == (localrsuhealth.errorstates & RSUHEALTH_SEC_CERT_NEAR_EXP)) {
                                  I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"Cert will expire soon (%s)\n",acf_file);
                              }
                              break;
                          case -3:
                              expiredcount++; /* expired */
                              /* Do SEL output here so we have acf name handy. */
                              if(0x0 == (localrsuhealth.errorstates & RSUHEALTH_SEC_CERT_EXPIRED)) {
                                  I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"Cert has expired (%s)\n",acf_file);
                              }
                              break;
                          case -2:
                              /* Do SEL output here so we have acf name handy. */
                              if(0x0 == (localrsuhealth.errorstates & RSUHEALTH_SEC_CERT_BOGUS)) {
                                  I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"Cert is bogus format (%s)\n",acf_file);
                              }
                              boguscount++; /* bogus */
                              break;
                          case -1:
                              if(0x0 == (localrsuhealth.errorstates & RSUHEALTH_BAD_INPUT)){
                                  I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"checkacffile: RSUHEALTH_BAD_INPUT\n");
                                  localrsuhealth.errorstates |= RSUHEALTH_BAD_INPUT; /* NULL INPUT should not happen. */
                              }
                              break;
                          case 0:
                              validcertcount++;
                              break;
                          default:
                              if(0x0 == (localrsuhealth.errorstates & RSUHEALTH_BAD_MODE)){
                                  I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"checkacffile: RSUHEALTH_BAD_MODE\n");
                                  localrsuhealth.errorstates |= RSUHEALTH_BAD_MODE; /* Unknown return should not happen. */
                              }
                              break;
                      }
                  } else {
                      localrsuhealth.errorstates |= RSUHEALTH_SSCANF_FAIL;
                      I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"checksecurity: RSUHEALTH_SSCANF_FAIL.\n");
                      break; /* abnormal exit, should happen based on how query is formed */
                  }
              } else {
                   break; /* normal exit, expect this once output consumed */
              }
          } /* for */
          pclose(fp);
      } else {
          localrsuhealth.errorstates |= RSUHEALTH_POPEN_FAIL;
          I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"checksecurity: RSUHEALTH_POPEN_FAIL.\n");
      }
  }
#if defined(EXTRA_DEBUG)
  I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"Certs: valid(%u) expiring(%u) expired(%u) bogus(%u)\n",
      validcertcount, abouttoexpirecount, expiredcount, boguscount);
#endif
  /* Eval results setting states accordingly */
  if(0 == validcertcount) {
      /* If no valid certs existed before then send info message telling user we found some. */
      if(0x0 == (localrsuhealth.errorstates & RSUHEALTH_SEC_CERT_FAIL)){
          I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"RSUHEALTH_SEC_CERT_FAIL.\n");
          /* 5.18.1.11 Certificate Error Message */
          setuptrap(CertificateID, rsuAlertLevel_critical,"[No Valid Certs found]");
      }
      localrsuhealth.errorstates |= RSUHEALTH_SEC_CERT_FAIL;
  } else {
      if((localrsuhealth.errorstates & RSUHEALTH_SEC_CERT_FAIL)){
          localrsuhealth.errorstates &= ~(uint64_t)RSUHEALTH_SEC_CERT_FAIL;
          I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"Valid Certs found(%d)\n",validcertcount);
          memset(nextmsg,0x0,sizeof(nextmsg));
          snprintf(nextmsg,sizeof(nextmsg),"[Valid Certs found(%d)]",validcertcount);
          /* 5.18.1.11 Certificate Error Message */
          setuptrap(CertificateID, rsuAlertLevel_info,nextmsg);
      }
  }
  if(RSU_ACF_MAX_COUNT <= validcertcount) {
      if(0x0 == (localrsuhealth.errorstates & RSUHEALTH_SEC_CERT_FULL)) {
          memset(nextmsg,0x0,sizeof(nextmsg));
          snprintf(nextmsg,sizeof(nextmsg),"[Certs full. Valid Certs found(%d)]",validcertcount);
          /* 5.18.1.11 Certificate Error Message */
          setuptrap(CertificateID, rsuAlertLevel_info,nextmsg);
      }
      localrsuhealth.errorstates |= RSUHEALTH_SEC_CERT_FULL;
  } else {
      localrsuhealth.errorstates &= ~(uint64_t)RSUHEALTH_SEC_CERT_FULL;
  }
  if(abouttoexpirecount) {
      if(0x0 == (localrsuhealth.errorstates & RSUHEALTH_SEC_CERT_NEAR_EXP)) {
          memset(nextmsg,0x0,sizeof(nextmsg));
          snprintf(nextmsg,sizeof(nextmsg),"[There are expiring certs(%d). Valid Certs(%d)]",abouttoexpirecount,validcertcount);
          /* 5.18.1.11 Certificate Error Message */
          setuptrap(CertificateID, rsuAlertLevel_info,nextmsg);
      }
      localrsuhealth.errorstates |= RSUHEALTH_SEC_CERT_NEAR_EXP;
  } else {
      localrsuhealth.errorstates &= ~(uint64_t)RSUHEALTH_SEC_CERT_NEAR_EXP;
  }
  if(expiredcount){
      if(0x0 == (localrsuhealth.errorstates & RSUHEALTH_SEC_CERT_EXPIRED)) {
         memset(nextmsg,0x0,sizeof(nextmsg));
         snprintf(nextmsg,sizeof(nextmsg),"[There are expired certs(%d). Valid Certs(%d)]",expiredcount,validcertcount);
         /* 5.18.1.11 Certificate Error Message */
         setuptrap(CertificateID, rsuAlertLevel_info,nextmsg);
      }
      localrsuhealth.errorstates |= RSUHEALTH_SEC_CERT_EXPIRED;
  } else {
      localrsuhealth.errorstates &= ~(uint64_t)RSUHEALTH_SEC_CERT_EXPIRED;
  }
  if(boguscount){
      if(0x0 == (localrsuhealth.errorstates & RSUHEALTH_SEC_CERT_BOGUS)) {
         memset(nextmsg,0x0,sizeof(nextmsg));
         snprintf(nextmsg,sizeof(nextmsg),"[There are bogus certs(%d). Valid Certs(%d)]",boguscount,validcertcount);
         /* 5.18.1.11 Certificate Error Message */
         setuptrap(CertificateID, rsuAlertLevel_info,nextmsg);
      }
      localrsuhealth.errorstates |= RSUHEALTH_SEC_CERT_BOGUS;
  } else {
      localrsuhealth.errorstates &= ~(uint64_t)RSUHEALTH_SEC_CERT_BOGUS;
  }
//
//TODO: What should we do? too many failures could be problematic and get RSU on bad actor list
//    : Maybe have max # of fails?
//TODO: Multiple ways to see fails. Consolidate.

  /* look for verify or sign failures */
  if(1 == rskstatus.ready) {
      if(priorrskstatus.SignCBFailures != rskstatus.SignCBFailures){
          localrsuhealth.cv2xstatus |= RSUHEALTH_SECURITY_SIGN_FAIL;
          memset(nextmsg,0x0,sizeof(nextmsg));
          snprintf(nextmsg,sizeof(nextmsg),"[Signing Failure]");
          /* 5.18.1.11 Certificate Error Message */
          setuptrap(CertificateID, rsuAlertLevel_warning,nextmsg);
      } else {
          localrsuhealth.cv2xstatus &= ~RSUHEALTH_SECURITY_SIGN_FAIL;
      }
      if(priorrskstatus.VerifyCBFailures != rskstatus.VerifyCBFailures){
          localrsuhealth.cv2xstatus |= RSUHEALTH_SECURITY_VERIFY_FAIL;
          memset(nextmsg,0x0,sizeof(nextmsg));
          snprintf(nextmsg,sizeof(nextmsg),"[Verify Failure]");
          /* 5.18.1.11 Certificate Error Message */
          setuptrap(CertificateID, rsuAlertLevel_warning,nextmsg);
      } else {
          localrsuhealth.cv2xstatus &= ~RSUHEALTH_SECURITY_VERIFY_FAIL;
      }
  }
  /* Only update if change. If update fails, dont change prior and try next time. */
  if(priorvalidcertcount != validcertcount) { 
      if(NULL != (fp = fopen("/tmp/rsudiag.validcerts","w"))){
          /* Dump number of valid certs to tmp for webgui to cat */
          fprintf(fp,"%d\n",validcertcount);
          fflush(fp);
          fclose(fp);
          localrsuhealth.cv2xstatus &= ~RSUHEALTH_SECURITY_FOPEN_FAIL;
          priorvalidcertcount = validcertcount;
      } else {
          if(0x0 == (localrsuhealth.cv2xstatus & RSUHEALTH_SECURITY_FOPEN_FAIL)){
              localrsuhealth.cv2xstatus |= RSUHEALTH_SECURITY_FOPEN_FAIL;
          }
      }
  }
}
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
static void checkcputemp(void)
{
  uint32_t  i = 0;
  FILE    * fp = NULL;
  char_t    replyBuf[64]; /* Using this twice below. Do not make larger than 64. */
  int32_t   cputemperature = 0;
  float64_t cputemperature_double = -999.99;

  memset(replyBuf,'\0',sizeof(replyBuf));
  if(NULL != (fp = POPEN(GET_CPU_TEMP, "r"))) {
      if(NULL != fgets(replyBuf, sizeof(replyBuf), fp)){
          i = strnlen(replyBuf,sizeof(replyBuf));
          if((CPU_TEMP_MIN_LENGTH <= i) && (i <= CPU_TEMP_MAX_LENGTH)){
              replyBuf[i-1] = '\0'; /* delete extra char added. */
          } else { 
              replyBuf[0] = '\0'; /* force scanf to fail */
          }
          if(1 == sscanf(replyBuf,GET_CPU_TEMP_REPLY,&cputemperature)) {
              cputemperature_double = (float64_t)(((float64_t)(cputemperature))/1000.0);
              localrsuhealth.temperature.cpu = cputemperature_double;
              /* Check critical range. */
              if(   (cputemperature_double < (float64_t)IMX8_TEMP_LOW_CRITICAL) 
                 || ((float64_t)IMX8_TEMP_HIGH_CRITICAL < cputemperature_double)){
                  if(0x0 == (localrsuhealth.errorstates & RSUHEALTH_CPU_TEMP_FAIL)){
                      memset(replyBuf,0x0,sizeof(replyBuf));
                      snprintf(replyBuf,sizeof(replyBuf),"[CPU temp in CRITICAL range(%lf)]",cputemperature_double);
                      setuptrap(EnvironID, rsuAlertLevel_critical,replyBuf);
                      I2V_DBG_LOG(LEVEL_CRIT, MY_NAME,"RSUHEALTH_CPU_TEMP_FAIL: cputemperature(%lf)\n",cputemperature_double);
                      localrsuhealth.errorstates |= RSUHEALTH_CPU_TEMP_FAIL;
                  }
              } else {
                  localrsuhealth.errorstates &= ~(uint64_t)RSUHEALTH_CPU_TEMP_FAIL;
              }
              /* Check warn range. */
              if(  ((IMX8_TEMP_LOW_CRITICAL < cputemperature_double) && (cputemperature_double < IMX8_TEMP_LOW_WARN))
                 ||((IMX8_TEMP_HIGH_WARN    < cputemperature_double) && (cputemperature_double < IMX8_TEMP_HIGH_CRITICAL))){
                  if(0x0 == (localrsuhealth.errorstates & RSUHEALTH_CPU_TEMP_WARN)){
                      memset(replyBuf,0x0,sizeof(replyBuf));
                      snprintf(replyBuf,sizeof(replyBuf),"[CPU temp in WARN range(%lf)]",cputemperature_double);
                      setuptrap(EnvironID, rsuAlertLevel_warning,replyBuf);                      
                      I2V_DBG_LOG(LEVEL_CRIT, MY_NAME,"RSUHEALTH_CPU_TEMP_WARN: cputemperature(%lf)\n",cputemperature_double);
                      localrsuhealth.errorstates |= RSUHEALTH_CPU_TEMP_WARN;
                  }
              } else {
                  localrsuhealth.errorstates &= ~(uint64_t)RSUHEALTH_CPU_TEMP_WARN;
              }
          } else {
              cputemperature_double = -999.99;
              localrsuhealth.errorstates |= RSUHEALTH_SSCANF_FAIL;
              localrsuhealth.errorstates |= RSUHEALTH_CPU_TEMP_FAIL;
          }
      } else {
          localrsuhealth.errorstates |= RSUHEALTH_FGETS_FAIL;
          localrsuhealth.errorstates |= RSUHEALTH_CPU_TEMP_FAIL;
      }
      PCLOSE(fp);
  } else {
      localrsuhealth.errorstates |= RSUHEALTH_POPEN_FAIL;
      localrsuhealth.errorstates |= RSUHEALTH_CPU_TEMP_FAIL;
  }
}
/*----------------------------------------------------------------------------*/
#if 0 //unit test kudos: too long data on %lf turns into -1.0 which is valid
   diag    | DBG  |cputemperature(-1,-0.001000)
   diag    | DBG  |ambienttemperature=(-64.001000,-1)
   diag    | DBG  |RSUHEALTH_AMBIENT_TEMP_FAIL: ambienttemperature(-64.001000)
   diag    | DBG  |hsmtemperature(12345678901234567214037922236636108099897988801446595923017728.000000)
#endif
/*----------------------------------------------------------------------------*/
static void checkambienttemp(void)
{
  uint32_t  i = 0;
  FILE    * fp = NULL;
  char_t    replyBuf[64]; /* Used twice below for two different operations. Do not make larger than 64 */
  int32_t   ambienttemperature = 0;
  float64_t ambienttemperature_double = -999.99;

  memset(replyBuf,'\0',sizeof(replyBuf));
  if(NULL != (fp = POPEN(GET_AMBIENT_TEMP, "r"))) {
      if(NULL != fgets(replyBuf, sizeof(replyBuf), fp)){
          i = strnlen(replyBuf,sizeof(replyBuf));
          if((AMBIENT_TEMP_MIN_LENGTH <= i) && (i <= AMBIENT_TEMP_MAX_LENGTH)){
              replyBuf[i-1] = '\0'; /* delete extra char added. */
          } else { 
              replyBuf[0] = '\0'; /* force scanf to fail */
          }
          if(1 == sscanf(replyBuf,GET_AMBIENT_TEMP_REPLY,&ambienttemperature)) {
              ambienttemperature_double = (float64_t)(((float64_t)(ambienttemperature))/1000.0) ;
              ambienttemperature_double -= 64.0;
              localrsuhealth.temperature.ambient = ambienttemperature_double;
              /* Check critical range. */
              if(   (ambienttemperature_double < (float64_t)AMBIENT_TEMP_LOW_CRITICAL) 
                 || ((float64_t)AMBIENT_TEMP_HIGH_CRITICAL < ambienttemperature_double)){
                  if(0x0 == (localrsuhealth.errorstates & RSUHEALTH_AMBIENT_TEMP_FAIL)){
                      memset(replyBuf,0x0,sizeof(replyBuf));
                      snprintf(replyBuf,sizeof(replyBuf),"[Ambient temp in CRITICAL range(%lf)]",ambienttemperature_double);
                      setuptrap(EnvironID, rsuAlertLevel_critical,replyBuf);
                      I2V_DBG_LOG(LEVEL_CRIT, MY_NAME,"RSUHEALTH_AMBIENT_TEMP_FAIL: ambienttemperature(%lf)\n",ambienttemperature_double);
                      localrsuhealth.errorstates |= RSUHEALTH_AMBIENT_TEMP_FAIL;
                  }
              } else {
                  localrsuhealth.errorstates &= ~(uint64_t)RSUHEALTH_AMBIENT_TEMP_FAIL;
              }
              /* Check warn range. */
              if(  ((AMBIENT_TEMP_LOW_CRITICAL < ambienttemperature_double) && (ambienttemperature_double < AMBIENT_TEMP_LOW_WARN))
                 ||((AMBIENT_TEMP_HIGH_WARN    < ambienttemperature_double) && (ambienttemperature_double < AMBIENT_TEMP_HIGH_CRITICAL))){
                  if(0x0 == (localrsuhealth.errorstates & RSUHEALTH_AMBIENT_TEMP_WARN)){
                      memset(replyBuf,0x0,sizeof(replyBuf));
                      snprintf(replyBuf,sizeof(replyBuf),"[Ambient temp in WARN range(%lf)]",ambienttemperature_double);
                      setuptrap(EnvironID, rsuAlertLevel_warning,replyBuf);                      
                      I2V_DBG_LOG(LEVEL_CRIT, MY_NAME,"RSUHEALTH_AMBIENT_TEMP_WARN: ambienttemperature(%lf)\n",ambienttemperature_double);
                      localrsuhealth.errorstates |= RSUHEALTH_AMBIENT_TEMP_WARN;
                  }
              } else {
                  localrsuhealth.errorstates &= ~(uint64_t)RSUHEALTH_AMBIENT_TEMP_WARN;
              }
          } else {
              ambienttemperature_double = -999.99;
              localrsuhealth.errorstates |= RSUHEALTH_SSCANF_FAIL;
              localrsuhealth.errorstates |= RSUHEALTH_AMBIENT_TEMP_FAIL;
          }
      } else {
          localrsuhealth.errorstates |= RSUHEALTH_FGETS_FAIL;
          localrsuhealth.errorstates |= RSUHEALTH_AMBIENT_TEMP_FAIL;
      }
      PCLOSE(fp);
  } else {
      localrsuhealth.errorstates |= RSUHEALTH_POPEN_FAIL;
      localrsuhealth.errorstates |= RSUHEALTH_AMBIENT_TEMP_FAIL;
  }
}
/*----------------------------------------------------------------------------*/
/*TODO: add tamperproofing                                                    */
/*----------------------------------------------------------------------------*/
static void checkenviornment(void)
{
  char_t nextmsg[64];

  if((NULL != shm_sous_ptr) && (RH850_SHM_UPDATE_RATE < onesecondcounter)){ /* Give SOUS 10 seconds to get going. */
      if(0x0 == (localrsuhealth.errorstates & RSUHEALTH_STALE_SOUS)){       /* If SOUS is not ready then dont bother. */
          localrsuhealth.volts.dcin = localsous.rh850_data.dcin_voltage;    /* Update. */
          /* Check critical range. */
          if(   (localsous.rh850_data.dcin_voltage < DCIN_LOW_CRITICAL) 
             || (DCIN_HIGH_CRITICAL                < localsous.rh850_data.dcin_voltage)) {
              if(0x0 == (localrsuhealth.errorstates & RSUHEALTH_DCIN_FAIL)){
                  memset(nextmsg,0x0,sizeof(nextmsg));
                  snprintf(nextmsg,sizeof(nextmsg),"[voltage(DCIN) in CRITICAL range:dcin(%f)]",localsous.rh850_data.dcin_voltage);
                  setuptrap(EnvironID, rsuAlertLevel_critical,nextmsg);
                  I2V_DBG_LOG(LEVEL_CRIT, MY_NAME,"RSUHEALTH_DCIN_FAIL: dcin (%f)\n",localsous.rh850_data.dcin_voltage);
                  localrsuhealth.errorstates |= RSUHEALTH_DCIN_FAIL;
              }
          } else {
              localrsuhealth.errorstates &= ~(uint64_t)RSUHEALTH_DCIN_FAIL;
          }
          /* Check warn range. */
          if(  ((DCIN_LOW_CRITICAL < localsous.rh850_data.dcin_voltage) && (localsous.rh850_data.dcin_voltage < DCIN_LOW_WARN))
             ||((DCIN_HIGH_WARN    < localsous.rh850_data.dcin_voltage) && (localsous.rh850_data.dcin_voltage < DCIN_HIGH_CRITICAL))){
              if(0x0 == (localrsuhealth.errorstates & RSUHEALTH_DCIN_WARN)){
                  memset(nextmsg,0x0,sizeof(nextmsg));
                  snprintf(nextmsg,sizeof(nextmsg),"[voltage(DCIN) in WARN range:dcin(%f)]",localsous.rh850_data.dcin_voltage);
                  setuptrap(EnvironID, rsuAlertLevel_warning,nextmsg);
                  localrsuhealth.errorstates |= RSUHEALTH_DCIN_WARN;
                  I2V_DBG_LOG(LEVEL_INFO, MY_NAME,"RSUHEALTH_DCIN_WARN: dcin (%f)\n",localsous.rh850_data.dcin_voltage);
              }
         } else {
              localrsuhealth.errorstates &= ~(uint64_t)RSUHEALTH_DCIN_WARN;
         }
      }
  }
  checkcputemp();
  checkambienttemp();

//TODO: Ambient by itself does not indicating a failure yet. Ignore ambient for now
#if 0
  if(localrsuhealth.errorstates & (RSUHEALTH_DCIN_FAIL|RSUHEALTH_CPU_TEMP_FAIL|RSUHEALTH_CV2X_TEMP_FAIL|RSUHEALTH_AMBIENT_TEMP_FAIL)) {
#else
  if(localrsuhealth.errorstates & (RSUHEALTH_DCIN_FAIL|RSUHEALTH_CPU_TEMP_FAIL|RSUHEALTH_CV2X_TEMP_FAIL)) {
#endif
      /* 5.18.1.14 Enclosure Environment */
      if(0x0 == (localrsuhealth.errorstates & RSUHEALTH_STANDBY_MODE)){
          setuptrap(EnvironID, rsuAlertLevel_critical,"[Entering standby mode. Please review syslog]");
      }
      /* go to standby, halt txrx, halt ifclog, halt forwarding */
      localrsuhealth.errorstates |= RSUHEALTH_STANDBY_MODE;
  } else {
      /* go to operate */
      localrsuhealth.errorstates &= ~(uint64_t)RSUHEALTH_STANDBY_MODE;
  }
}
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
static int32_t getipinfo(char_t *ifname, char_t *ipaddrstr, char_t *maskaddrstr, char_t *bcastaddrstr,uint16_t *active)
{
  struct ifaddrs *ifap=NULL, *ifaptr=NULL;
  int32_t ret=0;

  if(  (NULL==ifname)||(NULL==ipaddrstr)||(NULL==maskaddrstr)
     ||(NULL==bcastaddrstr)||(NULL==active)){
      return -1;
  } 
  if (0 != getifaddrs(&ifap)) {
      return -2;
  }
  ret = -3; /* not found */
  for(ifaptr=ifap; (0!=ret) && (ifaptr!=NULL); ifaptr= (ifaptr)->ifa_next) {
      if (!strcmp((ifaptr)->ifa_name, ifname) && (((ifaptr)->ifa_addr)->sa_family == AF_INET)) {

          struct sockaddr_in *pAddr = (struct sockaddr_in *)ifaptr->ifa_addr;
          strcpy(ipaddrstr,inet_ntoa(pAddr->sin_addr)); /* get ip */

          struct sockaddr_in *pMask = (struct sockaddr_in *)ifaptr->ifa_netmask;
          strcpy(maskaddrstr,inet_ntoa(pMask->sin_addr)); /* get mask */

          *active=0x0;
          if(IFF_BROADCAST & ifaptr->ifa_flags) {
              struct sockaddr_in *pBcast = (struct sockaddr_in *)ifaptr->ifa_ifu.ifu_broadaddr;
              strcpy(bcastaddrstr,inet_ntoa(pBcast->sin_addr)); /* get bcast */
              *active |= IFF_BROADCAST;
          }
          if(IFF_UP & ifaptr->ifa_flags){
              *active |= IFF_UP;
          }
          if(IFF_RUNNING & ifaptr->ifa_flags){
              *active |= IFF_RUNNING;
          }
          if(IFF_MULTICAST & ifaptr->ifa_flags){
              *active |= IFF_MULTICAST;
          }
          ret = 0; /* found, done */
      }
  }
  freeifaddrs(ifap);
  return ret;
}
/*----------------------------------------------------------------------------*/
/* JIRA 898 Network Init Fail                                                 */
/*ifconfig eth0
eth0      Link encap:Ethernet  HWaddr de:40:2f:ae:b9:8d
          inet addr:192.168.2.80  Bcast:192.168.2.255  Mask:255.255.255.0
          inet6 addr: fe80::dc40:2fff:feae:b98d/64 Scope:Link
          UP BROADCAST RUNNING MULTICAST  MTU:1500  Metric:1
          RX packets:75722 errors:0 dropped:0 overruns:0 frame:0
          TX packets:117533 errors:0 dropped:0 overruns:0 carrier:0
          collisions:0 txqueuelen:1000
          RX bytes:25395822 (24.2 MiB)  TX bytes:35420604 (33.7 MiB)
*/
/*----------------------------------------------------------------------------*/
static void checknetworkinit(void)
{
  int32_t   ret = 0;
  uint32_t  i = 0;
  FILE    * fp = NULL;
  char_t    replyBuf[IP_PACKET_REPLY_MAX_SIZE * 2]; /* Size according to your needs. */
  char_t    rsu_ip[I2V_DEST_IP_MAX];
  char_t    net_ip[I2V_DEST_IP_MAX];
  char_t    net_mask[I2V_DEST_IP_MAX];
  char_t    net_bcast[I2V_DEST_IP_MAX];
  uint16_t  active=0;

  fp = NULL;
  active=0;
  ret=0;
  i=0;
  /* get ip info from OS */
  memset(rsu_ip,'\0',sizeof(rsu_ip));
  memset(net_ip,'\0',sizeof(net_ip));
  memset(net_mask,'\0',sizeof(net_mask));
  memset(net_bcast,'\0',sizeof(net_bcast));
  if(0 == (ret = getipinfo(RSUHEALTH_NIC_OF_INTEREST,net_ip,net_mask,net_bcast,&active))){
      if(localrsuhealth.errorstates & RSUHEALTH_IP_INFO_FAIL) {
          I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"checknetworkinit: ip(%s) mask(%s) bcast(%s) active(0x%x)\n",
              net_ip,net_mask,net_bcast,active);
          localrsuhealth.errorstates &= ~(uint64_t)RSUHEALTH_IP_INFO_FAIL;
      }
  } else {
      if(0x0 == (localrsuhealth.errorstates & RSUHEALTH_IP_INFO_FAIL)){
          I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"checknetworkinit: getipinfo failed ret(%d).\n",ret);
          localrsuhealth.errorstates |= RSUHEALTH_IP_INFO_FAIL;
      }
  }
  /* get rsu ip as defined in CFGMGR */
  memset(replyBuf,'\0',sizeof(replyBuf));
  memset(rsu_ip,'\0',sizeof(rsu_ip));
  if(NULL != (fp = POPEN(GET_RSU_IP, "r"))) {
      if(NULL != fgets(replyBuf, sizeof(replyBuf), fp)){
          i = strnlen(replyBuf,sizeof(replyBuf));
          if((0 != i) && (i <= (uint32_t)I2V_DEST_IP_MAX)){
              replyBuf[i-1] = '\0'; /* delete extra char added. */
          } else { 
              replyBuf[0] = '\0'; /* force scanf to fail */
          }
          if(1 != sscanf(replyBuf,GET_RSU_IP_REPLY,rsu_ip)) {
              if(0x0 == (localrsuhealth.errorstates & RSUHEALTH_IP_GET_FAIL)) {
                  I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"RSUHEALTH_IP_GET_FAIL.\n");
              }
              localrsuhealth.errorstates |= RSUHEALTH_IP_GET_FAIL;
              localrsuhealth.errorstates |= RSUHEALTH_NETWORK_INIT_FAIL;
          } else {
              localrsuhealth.errorstates &= ~(uint64_t)RSUHEALTH_IP_GET_FAIL;
          }
      } else {
          localrsuhealth.errorstates |= RSUHEALTH_FGETS_FAIL;
          localrsuhealth.errorstates |= RSUHEALTH_IP_GET_FAIL;
          localrsuhealth.errorstates |= RSUHEALTH_NETWORK_INIT_FAIL;
      }
      PCLOSE(fp);
  } else {
      localrsuhealth.errorstates |= RSUHEALTH_POPEN_FAIL;
      localrsuhealth.errorstates |= RSUHEALTH_IP_GET_FAIL;
      localrsuhealth.errorstates |= RSUHEALTH_NETWORK_INIT_FAIL;
  }
  /* compare OS vs CFGMGR ips and should match */
  if(0x0 == (localrsuhealth.errorstates & RSUHEALTH_IP_GET_FAIL)) {
      //TODO: Under Unit Test strcmp returned 0 even though rsu_ip was all nulls?
      //printf("netip(%s) rsuip(%s)\n",net_ip,rsu_ip);
      if((0 == strnlen(net_ip,sizeof(net_ip))) || (0 == strnlen(net_ip,sizeof(rsu_ip)))) {
          net_ip[sizeof(net_ip)-1] = '\0'; /* Make sure null terminated before printfing. */
          net_ip[sizeof(rsu_ip)-1] = '\0';
          localrsuhealth.errorstates |= RSUHEALTH_IP_GET_FAIL;
          localrsuhealth.errorstates |= RSUHEALTH_NETWORK_INIT_FAIL;
          I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"checknetworkinit() failed: net_ip(%s) != ip(%s)\n",net_ip,rsu_ip);
      } else {
          if(0 == strcmp(net_ip,rsu_ip)) {
              if(localrsuhealth.errorstates & RSUHEALTH_NETWORK_INIT_FAIL) {
                  I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"checknetworkinit() success: net_ip(%s) == ip(%s)\n",net_ip,rsu_ip);
              }
              localrsuhealth.errorstates &= ~(uint64_t)RSUHEALTH_NETWORK_INIT_FAIL;
          } else {
              if(0x0 == (localrsuhealth.errorstates & RSUHEALTH_NETWORK_INIT_FAIL)) {
                  I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"checknetworkinit() failed: net_ip(%s) != ip(%s)\n",net_ip,rsu_ip);
              }
              localrsuhealth.errorstates |= RSUHEALTH_NETWORK_INIT_FAIL;
          }
      }
  }
  /* grab rx and tx stats on nic */
  if(0 != system(DUMP_NETWORK_INIT)){
      localrsuhealth.errorstates |= RSUHEALTH_SYS_CALL_FAIL;
  } else {
      localrsuhealth.errorstates &= ~(uint64_t)RSUHEALTH_SYS_CALL_FAIL;
  }
  memset(replyBuf,'\0',sizeof(replyBuf));
  if(NULL != (fp = POPEN(GET_IP_RX_PACKET, "r"))) {
      if(NULL != fgets(replyBuf, sizeof(replyBuf), fp)){
          i = strnlen(replyBuf,sizeof(replyBuf));
          if((0 != i) && (i <= IP_PACKET_REPLY_MAX_SIZE)){
              replyBuf[i-1] = '\0'; /* delete extra char added. */
          } else { 
              replyBuf[0] = '\0'; /* force scanf to fail */
          }
          if(5 == sscanf(replyBuf,GET_IP_RX_PACKET_REPLY,
               &localrsuhealth.ipcounts.rxpackets
              ,&localrsuhealth.ipcounts.rxerrors
              ,&localrsuhealth.ipcounts.rxdropped
              ,&localrsuhealth.ipcounts.rxoverruns
              ,&localrsuhealth.ipcounts.rxframe)) {
              localrsuhealth.errorstates &= ~(uint64_t)RSUHEALTH_IP_RX_GET_FAIL;
          } else {
              localrsuhealth.errorstates |= RSUHEALTH_IP_RX_GET_FAIL;
          }
      } else {
          localrsuhealth.errorstates |= RSUHEALTH_FGETS_FAIL;
          localrsuhealth.errorstates |= RSUHEALTH_IP_RX_GET_FAIL;
      }
      PCLOSE(fp);
  } else {
      localrsuhealth.errorstates |= RSUHEALTH_POPEN_FAIL;
      localrsuhealth.errorstates |= RSUHEALTH_IP_RX_GET_FAIL;
  }
  memset(replyBuf,'\0',sizeof(replyBuf));
  if(NULL != (fp = POPEN(GET_IP_TX_PACKET, "r"))) {
      if(NULL != fgets(replyBuf, sizeof(replyBuf), fp)){
          i = strnlen(replyBuf,sizeof(replyBuf));
          if((0 != i) && (i <= IP_PACKET_REPLY_MAX_SIZE)){
              replyBuf[i-1] = '\0'; /* delete extra char added. */
          } else { 
              replyBuf[0] = '\0'; /* force scanf to fail */
          }
          if(5 == sscanf(replyBuf,GET_IP_TX_PACKET_REPLY,
               &localrsuhealth.ipcounts.txpackets
              ,&localrsuhealth.ipcounts.txerrors
              ,&localrsuhealth.ipcounts.txdropped
              ,&localrsuhealth.ipcounts.txoverruns
              ,&localrsuhealth.ipcounts.txcarrier)) {
              localrsuhealth.errorstates &= ~(uint64_t)RSUHEALTH_IP_TX_GET_FAIL;
          } else {
              localrsuhealth.errorstates |= RSUHEALTH_IP_TX_GET_FAIL;
          }
      } else {
          localrsuhealth.errorstates |= RSUHEALTH_FGETS_FAIL;
          localrsuhealth.errorstates |= RSUHEALTH_IP_TX_GET_FAIL;
      }
      PCLOSE(fp);
  } else {
      localrsuhealth.errorstates |= RSUHEALTH_POPEN_FAIL;
      localrsuhealth.errorstates |= RSUHEALTH_IP_TX_GET_FAIL;
  }
}
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
#if defined(EXTRA_EXTRA_DEBUG)
/* Not expecting anything beyond 16 bits, maybe one day */
#define _10baseT_Half_BIT    0x1
#define _10baseT_Full_BIT    0x2
#define _100baseT_Half_BIT   0x4
#define _100baseT_Full_BIT   0x8
#define _1000baseT_Half_BIT  0x10
#define _1000baseT_Full_BIT  0x20
#define Autoneg_BIT          0x40
#define TP_BIT               0x80
#define AUI_BIT              0x100
#define MII_BIT              0x200
#define FIBRE_BIT            0x400
#define BNC_BIT              0x800
#define _10000baseT_Full_BIT 0x1000
#define Pause_BIT            0x2000
#define Asym_Pause_BIT       0x4000
#define _2500baseX_Full_BIT  0x8000
#define Backplane_BIT        0x10000

static void printlinkmodes(uint32_t bitfield)
{
  char_t buffer[64];
  
  memset(buffer,'\0',sizeof(buffer));
  if(bitfield & _10baseT_Half_BIT) {
      strncat(buffer,"  10baseT/Half",sizeof(buffer));
  }
  if(bitfield & _10baseT_Full_BIT) {
      strncat(buffer," 10baseT/Full",sizeof(buffer));
  }
  I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"%s\n",buffer);

  memset(buffer,'\0',sizeof(buffer));
  if(bitfield & _100baseT_Half_BIT) {
      strncat(buffer,"  100baseT/Half",sizeof(buffer));
  }
  if(bitfield & _100baseT_Full_BIT) {
      strncat(buffer," 100baseT/Full",sizeof(buffer));
  }
  I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"%s\n",buffer);

  memset(buffer,'\0',sizeof(buffer));
  if(bitfield & _1000baseT_Half_BIT) {
      strncat(buffer,"  1000baseT/Half",sizeof(buffer));
  }
  if(bitfield & _1000baseT_Full_BIT) {
      strncat(buffer,"  1000baseT/Full",sizeof(buffer));
  }
  I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"%s\n",buffer);

  if(bitfield & _10000baseT_Full_BIT) {
      I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"  1000baseT/Full\n");
  }
  if(bitfield & Pause_BIT) {
      I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"  pause frame enabled:\n");
      if(bitfield & Asym_Pause_BIT) {
          I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"  Supported pause frame use: Asymmetric\n");
      } else {
          I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"  Supported pause frame use: Symmetric\n");
      }
  } else {
    I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"  Supported pause frame use: None\n");
  }
  if(bitfield & _2500baseX_Full_BIT) {
      I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"  2500baseX/Full\n");
  }
  if(bitfield & Backplane_BIT) {
      I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"  Backplane\n");
  }
}
#endif
/*----------------------------------------------------------------------------*/
/*
20230324_004834 |   diag    | DBG  |verifylinkinfo:supp(0x22ef)adv(0x22ef)spd(1000)duplx(1)prt(2)phy(1)tran(0)
20230324_004834 |   diag    | DBG  |auto(1)mdio(0)maxtxrxpky(0,0)spdhi(0)emidx(0)emidxc(0)lpadv(0x206f)
20230324_184514 |   diag    | DBG  |wake(0x20 0x0)
*/
/*----------------------------------------------------------------------------*/
static void printsupportnetworklinks(struct ethtool_cmd * etoolcmd
                                     ,struct ethtool_wolinfo * etoolwolinfo
                                     ,int32_t index) 
{
#if defined(EXTRA_EXTRA_DEBUG)
  char_t  wake_on_options[16];
  char_t  ports_supported[80];
  char_t  port_used[16];
#endif

  index = index;
  if((NULL == etoolcmd) || (NULL == etoolwolinfo)) {
    return; /* no soup for you */
  }
#if defined(EXTRA_DEBUG)
  I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"verifylinkinfo:supp(0x%x)adv(0x%x)spd(%u)duplx(%u)prt(%u)phy(%u)tran(%u)auto(%u)\n",
              etoolcmd->supported
              ,etoolcmd->advertising
              ,etoolcmd->speed
              ,etoolcmd->duplex
              ,etoolcmd->port
              ,etoolcmd->phy_address
              ,etoolcmd->transceiver
              ,etoolcmd->autoneg)
  I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"verifylinkinfo:mdio(%u)maxtxrxpk(%u,%u)spdhi(%u)emidx(%u)emidxc(%u)lpadv(0x%x)wake(0x%x 0x%x)\n",
              etoolcmd->mdio_support
              ,etoolcmd->maxtxpkt
              ,etoolcmd->maxrxpkt
              ,etoolcmd->speed_hi
              ,etoolcmd->eth_tp_mdix
              ,etoolcmd->eth_tp_mdix_ctrl
              ,etoolcmd->lp_advertising,etoolwolinfo->supported,etoolwolinfo->wolopts);

//TODO: Once successful output once. then only on change
#endif

#if defined(EXTRA_EXTRA_DEBUG)
      memset(ports_supported,'\0',sizeof(ports_supported));
      if(etoolcmd->supported & TP_BIT) {
          strncat(ports_supported,"TP ",sizeof(ports_supported));
      }
      if(etoolcmd->supported & AUI_BIT) {
          strncat(ports_supported,"AUI ",sizeof(ports_supported));
      }
      if(etoolcmd->supported & MII_BIT) {
          strncat(ports_supported,"MII ",sizeof(ports_supported));
      }
      if(etoolcmd->supported & FIBRE_BIT) {
          strncat(ports_supported,"FIBRE ",sizeof(ports_supported));
      }
      if(etoolcmd->supported & BNC_BIT) {
          strncat(ports_supported,"BNC ",sizeof(ports_supported));
      }
      I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"Supported ports: %s\n",ports_supported);

      I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"Supported link modes:\n");
      printlinkmodes(etoolcmd->supported);
      I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"Advertised link modes:\n");
      printlinkmodes(etoolcmd->advertising);
      I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"Link partner advertised modes:\n");
      printlinkmodes(etoolcmd->lp_advertising);

      I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"Speed %uMb/sec\n",etoolcmd->speed);

      switch (etoolcmd->duplex) {
      case DUPLEX_HALF:
          I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"Duplex: HALF\n");
      break;
      case DUPLEX_FULL:
          I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"Duplex: FULL\n");
      break;
      case DUPLEX_UNKNOWN:
          I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"Duplex: UNKNOWN\n");
      break;
      default:
         I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"Duplex: ERROR\n");
      break;
      }

      memset(port_used,'\0',sizeof(port_used));
      switch(etoolcmd->port) {
      case PORT_TP:
          strncat(port_used,"TP ",sizeof(port_used));
      break;
      case PORT_AUI:
          strncat(port_used,"AUI ",sizeof(port_used));
      break;
      case PORT_MII:
          strncat(port_used,"MII ",sizeof(port_used));
      break;
      case PORT_FIBRE:
          strncat(port_used,"FIBRE ",sizeof(port_used));
      break;
      case PORT_BNC:
          strncat(port_used,"BNC ",sizeof(port_used));
      break;
      case PORT_DA:
          strncat(port_used,"DA ",sizeof(port_used));
      break;
      case PORT_NONE:
          strncat(port_used,"NONE ",sizeof(port_used));
      break;
      case PORT_OTHER:
          strncat(port_used,"OTHER ",sizeof(port_used));
      default:
          strncat(port_used,"ERROR ",sizeof(port_used));
      break;
      }
      I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"Port: %s\n",port_used);

      I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"PHYAD: %u\n",etoolcmd->phy_address);

      if(1 == etoolcmd->transceiver) {
          I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"Transceiver: extrernal\n");
      } else {
          I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"Transceiver: internal\n");
      }
      if(1 == etoolcmd->autoneg) {
          I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"Auto-negotiation: on\n");
      } else {
          I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"Auto-negotiation: off\n");
      }

      memset(wake_on_options,'\0',sizeof(wake_on_options));
      if(etoolwolinfo->supported & WAKE_PHY) {
          strncat(wake_on_options,"p",sizeof(wake_on_options));
      }
      if(etoolwolinfo->supported & WAKE_UCAST) {
          strncat(wake_on_options,"u",sizeof(wake_on_options));
      }
      if(etoolwolinfo->supported & WAKE_MCAST) {
          strncat(wake_on_options,"m",sizeof(wake_on_options));
      }
      if(etoolwolinfo->supported & WAKE_BCAST) {
          strncat(wake_on_options,"b",sizeof(wake_on_options));
      }
      if(etoolwolinfo->supported & WAKE_ARP) {
          strncat(wake_on_options,"a",sizeof(wake_on_options));
      }
      if(etoolwolinfo->supported & WAKE_MAGIC) {
          strncat(wake_on_options,"g",sizeof(wake_on_options));
      }
      if(etoolwolinfo->supported & WAKE_MAGICSECURE) {
          strncat(wake_on_options,"s",sizeof(wake_on_options));
      }
      I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"Supports Wake-on: %s\n",wake_on_options);

      memset(wake_on_options,'\0',sizeof(wake_on_options));
      if(etoolwolinfo->wolopts) {
          if(etoolwolinfo->supported & WAKE_PHY) {
              strncat(wake_on_options,"p",sizeof(wake_on_options));
          }
          if(etoolwolinfo->supported & WAKE_UCAST) {
              strncat(wake_on_options,"u",sizeof(wake_on_options));
          }
          if(etoolwolinfo->supported & WAKE_MCAST) {
              strncat(wake_on_options,"m",sizeof(wake_on_options));
          }
          if(etoolwolinfo->supported & WAKE_BCAST) {
              strncat(wake_on_options,"b",sizeof(wake_on_options));
          }
          if(etoolwolinfo->supported & WAKE_ARP) {
              strncat(wake_on_options,"a",sizeof(wake_on_options));
          }
          if(etoolwolinfo->supported & WAKE_MAGIC) {
              strncat(wake_on_options,"g",sizeof(wake_on_options));
          }
          if(etoolwolinfo->supported & WAKE_MAGICSECURE) {
              strncat(wake_on_options,"s",sizeof(wake_on_options));
          }
      } else {          
          strncat(wake_on_options,"d",sizeof(wake_on_options));
      }
      I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"Wake-on: %s\n",wake_on_options);

      /* No link partner then link not detected */
      if(etoolcmd->lp_advertising) {
          I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"Link detected: yes\n");
      } else {
          I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"Link detected: no\n");
      }

      /* Nothing supported then failure of some sort */
      if(etoolcmd->supported) {
          I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"Support link detected: yes\n");
      } else {
          I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"Support link detected: no\n");
      }
#endif
}
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
static int32_t waitforclose(int32_t sock)
{
  int32_t ret = 0;
  int32_t i = 0;

  if(sock < 0) {
      /* nothing to do, ignore */
      ret = 0;
  } else {
      for(i=0,ret = 0; i<10 ;i++) {
          ret = close(sock);
          if(ret == -1 && errno == EINTR) {
              /* keep trying */
          } else {
              break;
          }
      }
  }
  sock = -1;
  return ret;
}
/*----------------------------------------------------------------------------*/
/*
  ethtool eth0
  Settings for eth0:\
        Supported ports: [ TP MII ]\
        Supported link modes:   10baseT/Half 10baseT/Full\
                                100baseT/Half 100baseT/Full\
                                1000baseT/Full\
        Supported pause frame use: Symmetric\
        Supports auto-negotiation: Yes\
        Advertised link modes:  10baseT/Half 10baseT/Full\
                                100baseT/Half 100baseT/Full\
                                1000baseT/Full\
        Advertised pause frame use: Symmetric\
        Advertised auto-negotiation: Yes\
        Link partner advertised link modes:  10baseT/Half 10baseT/Full\
                                             100baseT/Half 100baseT/Full\
                                             1000baseT/Full\
        Link partner advertised pause frame use: Symmetric\
        Link partner advertised auto-negotiation: Yes\
        Speed: 1000Mb/s\
        Duplex: Full\
        Port: MII\
        PHYAD: 1\
        Transceiver: internal\
        Auto-negotiation: on\
        Supports Wake-on: g\
        Wake-on: d\
        Link detected: yes
*/
/*----------------------------------------------------------------------------*/
static int32_t verifylinkinfoetool(char_t * ifname)
{
  int32_t ret = 0;
  int32_t sock = -1;
  struct  ifreq  ifr;
  struct  ethtool_cmd etoolcmd;
  struct  ethtool_wolinfo etoolwolinfo;

  if(NULL == ifname) {
      ret = -1;
  }
  /* first need to find interface index by name */
  if(0 == ret){
     if(-1 == (sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP))){
#if defined(EXTRA_DEBUG)
          I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"verifylinkinfo: sock failed: ret(-2) errno(%s)\n",strerror(errno));
#endif
          ret = -2;
      } else {
          strncpy(ifr.ifr_name, ifname, sizeof(ifr.ifr_name));
          if (0 > ioctl(sock, SIOCGIFINDEX, &ifr)) {
#if defined(EXTRA_DEBUG)
              I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"verifylinkinfo: ioctl failed: ret(-3) errno(%s)\n",strerror(errno));
#endif
              ret = -3;
          }
          if(-1 == waitforclose(sock)) { /* close this socket type, done with it */
#if defined(EXTRA_DEBUG)
              I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"verifylinkinfo: close failed: ret(-4) errno(%s)\n",strerror(errno));
#endif
              ret = -4;
          }
      }
  }
  /* now we can open socket and use ethtool ioctl */
  if(0 == ret) {
      if (-1 == (sock = socket(AF_INET, SOCK_STREAM, 0))) {
#if defined(EXTRA_DEBUG)
          I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"verifylinkinfo: sock failed: ret(-5) errno(%s)\n",strerror(errno));
#endif
          ret = -5;
      }
  }
  if(0 == ret){
      ifr.ifr_data = (void *)&etoolcmd;
      etoolcmd.cmd = ETHTOOL_GSET;
      if (0 > ioctl(sock, SIOCETHTOOL, &ifr)) {
#if defined(EXTRA_DEBUG)
          I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"verifylinkinfo: ioctl failed: ret(-6) errno(%s)\n",strerror(errno));
#endif
          ret = -6;
      } else {
          ethtool_cmd_speed(&etoolcmd);
      }
  }
  if(0 == ret) {
      ifr.ifr_data = (void *)&etoolwolinfo;
      etoolwolinfo.cmd = ETHTOOL_GWOL;
      if (0 > ioctl(sock, SIOCETHTOOL, &ifr)) {
#if defined(EXTRA_DEBUG)
          I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"verifylinkinfo: ioctl failed: ret(-7) errno(%s)\n",strerror(errno));
#endif
          ret = -7;
      }
  }
  if(sock) {
      if(-1 == waitforclose(sock)) { /* close this socket type, done with it */
#if defined(EXTRA_DEBUG)
          I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"verifylinkinfo: close failed: ret(-4) errno(%s)\n",strerror(errno));
#endif
          ret = -8;
      }
  }
  /* eval bits to declare link good */
  if(0 == ret){
      /* No link partner then link not detected */
      if(etoolcmd.lp_advertising) {
          ret = 0;
      } else {
          ret = -9;
      }
      /* Nothing supported then failure of some sort */
      if(etoolcmd.supported) {
          ret = 0;
      } else {
          ret = -10;
      }
  }
  printsupportnetworklinks(&etoolcmd, &etoolwolinfo,ifr.ifr_ifindex);
  return ret;
}
/*----------------------------------------------------------------------------*/
/* JIRA 897 Network Link Fail                                                 */
/*
imx8qxpc0mek:/mnt# ip route
192.168.2.0/24 dev eth0 proto kernel scope link src 192.168.2.80

imx8qxpc0mek:/mnt# ip -4 addr show dev eth0
2: eth0: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc mq state UP group default qlen 1000
    inet 192.168.2.80/24 brd 192.168.2.255 scope global eth0
       valid_lft forever preferred_lft forever
*/
/*----------------------------------------------------------------------------*/
static void checknetworklink(void)
{
  int32_t ret = 0;
  if(0 != (ret = verifylinkinfoetool(RSUHEALTH_NIC_OF_INTEREST))) {
      if(0x0 == (localrsuhealth.errorstates & RSUHEALTH_NETWORK_LINK_FAIL)) {
          I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"checknetworklink: failed ret(%d)\n",ret);
      }
      localrsuhealth.errorstates |= RSUHEALTH_NETWORK_LINK_FAIL;
  } else {
      if(localrsuhealth.errorstates & RSUHEALTH_NETWORK_LINK_FAIL) {
          I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"checknetworklink: success\n");
      }
      localrsuhealth.errorstates &= ~(uint64_t)RSUHEALTH_NETWORK_LINK_FAIL;
  }
}
/*============================================================================
 * Functions to monitor and log networks events
 * Mainly a monitoring thread to listen for network events 
 * and supporting functions which interface to kernel network subsystem 
 * thru Netlink interface
 *===========================================================================*/
#define MAX_BUFFER_SIZE 4096

/*----------------------------------------------------------------------------
 * Function to process link events
 *---------------------------------------------------------------------------*/
static void processLinkEvent(struct ifinfomsg *ifi) 
{
    char ifname[16] = "";
    char msgStr[256] = "";

    sprintf(msgStr, "Link event: "); 
    
    /* Interface name */
    if_indextoname(ifi->ifi_index, ifname);
        sprintf(msgStr + strlen(msgStr), "%s ", ifname);

    /* Interface flags */
    unsigned int flags = ifi->ifi_flags;

    /* Check if the link is up or down */
    if (flags & IFF_RUNNING) {
        sprintf(msgStr + strlen(msgStr), "UP, ");
    } else {
        sprintf(msgStr + strlen(msgStr), "DOWN, ");
    }

    /* Check if the interface is a loopback interface */
    if (flags & IFF_LOOPBACK) {
        sprintf(msgStr + strlen(msgStr), "LOOPBACK | ");
    }

    /* Check if the interface is a point-to-point link */
    if (flags & IFF_POINTOPOINT) {
        sprintf(msgStr + strlen(msgStr), "POINTOPOINT | ");
    }

    /* Check if the interface is a multicast interface */
    if (flags & IFF_MULTICAST) {
        sprintf(msgStr + strlen(msgStr), "MULTICAST | ");
    }

    /* Check if the interface supports ARP */
    if (flags & IFF_NOARP) {
        sprintf(msgStr + strlen(msgStr), "NOARP | ");
    }

    /* Check if the interface is a broadcast interface */
    if (flags & IFF_BROADCAST) {
        sprintf(msgStr + strlen(msgStr), "BROADCAST | ");
    }

    /* Check if the interface is in promiscuous mode */
    if (flags & IFF_PROMISC) {
        sprintf(msgStr + strlen(msgStr), "PROMISC | ");
    }

    /* Check if the interface is a lower layer device */
    if (flags & IFF_LOWER_UP) {
        sprintf(msgStr + strlen(msgStr), "LOWER_UP | ");
    }

    /* Check if the interface is running in dynamic mode */
    if (flags & IFF_DYNAMIC) {
        sprintf(msgStr + strlen(msgStr), "DYNAMIC | ");
    }

    /* Check if the interface has internal debugging enabled */
    if (flags & IFF_DEBUG) {
        sprintf(msgStr + strlen(msgStr), "DEBUG | ");
    }           
    
    /* Save to log */
    I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME, "%s \n", msgStr);
}

/*----------------------------------------------------------------------------
 * Function to process address events
 *---------------------------------------------------------------------------*/
static void processAddressEvent(struct nlmsghdr *nlh) {
    struct ifaddrmsg *ifa = (struct ifaddrmsg *)NLMSG_DATA(nlh);
    char ifname[16] = "";
    char msgStr[256] = "";
    struct rtattr *rta;
    int len;

    sprintf(msgStr, "Address event: ");

    /* Interface name */
    if_indextoname(ifa->ifa_index, ifname);
    sprintf(msgStr + strlen(msgStr), "%s, ", ifname);

    /* Process the attributes of the address message */
    len = NLMSG_PAYLOAD(nlh, sizeof(struct ifaddrmsg));
    rta = IFA_RTA(ifa);
    while (RTA_OK(rta, len)) {
        switch (rta->rta_type) {
            case IFA_LOCAL:
            case IFA_ADDRESS:
                if (ifa->ifa_family == AF_INET6) {
                    struct in6_addr *addr = (struct in6_addr *)RTA_DATA(rta);
                    char str[INET6_ADDRSTRLEN];
                    inet_ntop(AF_INET6, addr, str, INET6_ADDRSTRLEN);
                    sprintf(msgStr + strlen(msgStr), "IPv6 Address: %s, ", str);
                } else if (ifa->ifa_family == AF_INET) {
                    struct in_addr *addr = (struct in_addr *)RTA_DATA(rta);
                    char str[INET_ADDRSTRLEN];
                    inet_ntop(AF_INET, addr, str, INET_ADDRSTRLEN);
                    sprintf(msgStr + strlen(msgStr), "IPv4 Address: %s, ", str);
                }
                break;

            // Add cases for other address attributes as needed

            default:
                // Handle other attributes if necessary
                break;
        }

        // Move to the next attribute
        rta = RTA_NEXT(rta, len);
    }

    /* Save to log */
    I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME, "%s \n", msgStr);
}
/*----------------------------------------------------------------------------
 * Function to process route events
 *---------------------------------------------------------------------------*/
static void processRouteEvent(struct nlmsghdr *nlh) {
    struct rtmsg *rtm = (struct rtmsg *)NLMSG_DATA(nlh);
    struct rtattr *rta;
    int len;
    char msgStr[256] = "";

    // Check the route message type
    if (rtm->rtm_family == AF_INET6) {
        sprintf(msgStr, "Route event: IPv6 - ");
    } else if (rtm->rtm_family == AF_INET) {
        sprintf(msgStr, "Route event: IPv4 - ");
    } else {
        sprintf(msgStr, "Route event: Unknown\n");
    }

    // Process the route attributes
    len = RTM_PAYLOAD(nlh);
    rta = RTM_RTA(rtm);
    while (RTA_OK(rta, len)) {
        switch (rta->rta_type) {
            case RTA_DST:
                if (rtm->rtm_family == AF_INET6) {
                    struct in6_addr *addr = (struct in6_addr *)RTA_DATA(rta);
                    char str[INET6_ADDRSTRLEN];
                    inet_ntop(AF_INET6, addr, str, INET6_ADDRSTRLEN);
                    sprintf(msgStr + strlen(msgStr), "Destination: %s, ", str);
                } else if (rtm->rtm_family == AF_INET) {
                    struct in_addr *addr = (struct in_addr *)RTA_DATA(rta);
                    char str[INET_ADDRSTRLEN];
                    inet_ntop(AF_INET, addr, str, INET_ADDRSTRLEN);
                    sprintf(msgStr + strlen(msgStr), "Destination: %s, ", str);
                }
                break;

            case RTA_GATEWAY:
                if (rtm->rtm_family == AF_INET6) {
                    struct in6_addr *addr = (struct in6_addr *)RTA_DATA(rta);
                    char str[INET6_ADDRSTRLEN];
                    inet_ntop(AF_INET6, addr, str, INET6_ADDRSTRLEN);
                    sprintf(msgStr + strlen(msgStr), "Gateway: %s, ", str);
                } else if (rtm->rtm_family == AF_INET) {
                    struct in_addr *addr = (struct in_addr *)RTA_DATA(rta);
                    char str[INET_ADDRSTRLEN];
                    inet_ntop(AF_INET, addr, str, INET_ADDRSTRLEN);
                    sprintf(msgStr + strlen(msgStr), "Gateway: %s, ", str);
                }
                break;

            // Add cases for other route attributes as needed

            default:
                // Handle other attributes if necessary
                break;
        }

        // Move to the next attribute
        rta = RTA_NEXT(rta, len);
    }

    /* Save to log */
    I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME, "%s \n", msgStr);
}

/*----------------------------------------------------------------------------
 * Function to process neighbor events
 *---------------------------------------------------------------------------*/
static void processNeighborEvent(struct ndmsg *ndm) 
{
    /*
     * TBD: add more details
     */
    char ifname[16];
    if_indextoname(ndm->ndm_ifindex, ifname);
#if defined(EXTRA_DEBUG)
    I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME, "Neighbor event: Interface index: %d, Name: %s \n", ndm->ndm_ifindex, ifname);
#endif
}

/*----------------------------------------------------------------------------
 * Function to process traffic control events
 *---------------------------------------------------------------------------*/
static void processTrafficControlEvent(struct tcmsg *tcm) 
{
    /*
     * TBD: add more details
     */
    I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME, "Traffic control event: Qdisc index: %u\n", tcm->tcm_handle);
}

/*----------------------------------------------------------------------------
 * Network event monitoring thread
 *---------------------------------------------------------------------------*/
void *networkEventThread(void *arg) {
    int32_t sockfd;
    struct sockaddr_nl sa;
    struct nlmsghdr *nlh;
    char_t buffer[MAX_BUFFER_SIZE];

    arg = arg;

    /* Create a Netlink socket */
    sockfd = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
    if (sockfd < 0) {
        localrsuhealth.errorstates |= RSUHEALTH_SYS_CALL_FAIL;
        pthread_exit(NULL);
    }

    /* 
     * Set up the sockaddr_nl structure 
     */
    memset(&sa, 0, sizeof(sa));
    sa.nl_family = AF_NETLINK;
    /* Monitor all network events - including IPV6*/
    sa.nl_groups = RTMGRP_LINK | RTMGRP_IPV4_IFADDR | RTMGRP_IPV6_IFADDR |
                   RTMGRP_IPV4_ROUTE | RTMGRP_IPV6_ROUTE | RTMGRP_NEIGH |
                   RTMGRP_TC | RTMGRP_IPV4_MROUTE | RTMGRP_IPV6_MROUTE;

    /* Bind socket */
    if (bind(sockfd, (struct sockaddr *)&sa, sizeof(sa)) < 0) {
        localrsuhealth.errorstates |= RSUHEALTH_SYS_CALL_FAIL;
        close(sockfd);
        pthread_exit(NULL);
    }

    /* 
     * Receive and process network events
     */
    while (loop) {
        memset(buffer, 0, sizeof(buffer));

        /* Receive data from the socket */
        if (recv(sockfd, buffer, sizeof(buffer), 0) < 0) {
            localrsuhealth.errorstates |= RSUHEALTH_SYS_CALL_FAIL;
            break;
        }

        /* Process the received message */
        nlh = (struct nlmsghdr *)buffer;
        while (NLMSG_OK(nlh, sizeof(buffer))) {
            if (nlh->nlmsg_type == NLMSG_DONE)
                break;

            // Switch based on the event type
            switch (nlh->nlmsg_type) {
                case RTM_NEWLINK:
                case RTM_DELLINK:
                    processLinkEvent((struct ifinfomsg *)NLMSG_DATA(nlh));
                    break;

                case RTM_NEWADDR:
                case RTM_DELADDR:
                    processAddressEvent(nlh);
                    break;

                case RTM_NEWROUTE:
                case RTM_DELROUTE:
                    processRouteEvent(nlh);
                    break;

                case RTM_NEWNEIGH:
                case RTM_DELNEIGH:
                    processNeighborEvent((struct ndmsg *)NLMSG_DATA(nlh));
                    break;

                case RTM_NEWQDISC:
                case RTM_DELQDISC:
                    processTrafficControlEvent((struct tcmsg *)NLMSG_DATA(nlh));
                    break;

                default:
                    // Unknown event type
                    localrsuhealth.errorstates |= RSUHEALTH_BAD_MODE;
                    break;
            }
            
            /* Next message */
            nlh = (struct nlmsghdr *)((char_t *)nlh + NLMSG_ALIGN(nlh->nlmsg_len));
        }
    }

    close(sockfd);
    pthread_exit(NULL);
}

/*----------------------------------------------------------------------------*/
/* network init and link: Can be disconnected at any moment so keep checking. */
/*----------------------------------------------------------------------------*/
static void checknetwork(void)
{
  checknetworklink();
  checknetworkinit();
}
/*----------------------------------------------------------------------------*/
/* For A & B: 0x1 to 0xA is 0 to 100%. 0xB is exceeded max.                   */
/* For EOL: 0x1 == Normal, 0x2 == warning 80%, 0x3 == urgent 90% done.        */
/* All cases 0x0 is undefined. The rest is reserved.                          */
/*----------------------------------------------------------------------------*/
static void checkemmc(void)
{
  uint32_t  i = 0;
  FILE    * fp = NULL;
  char_t    replyBuf[128]; /* Size according to your needs. */
  uint32_t  emmchealthA = 0;
  uint32_t  emmchealthB = 0;
  uint32_t  emmchealthEOL = 0;
  //float64_t size = 0;
  char_t    size_s[32];
  //float64_t used = 0;
  char_t    used_s[32];
  //float64_t remaining = 0;
  char_t    remaining_s[32];
  float64_t percent = 0;
  char_t    percent_s[32];
  char_t    * endptr = NULL;
  char_t errormsg[92];

  if(1 == (onesecondcounter % EMMC_CHECK_MODULUS)) {
      /* Check lifetime and EOL estimates. */
      memset(replyBuf,'\0',sizeof(replyBuf));
      if(NULL != (fp = POPEN(GET_EMMC_HEALTH_A, "r"))) {
          if(NULL != fgets(replyBuf, sizeof(replyBuf), fp)){
              i = strnlen(replyBuf,sizeof(replyBuf));
              if((0 != i) && (i <= sizeof(replyBuf))){
                  replyBuf[i-1] = '\0'; /* delete extra char added. */
              } else { 
                  replyBuf[0] = '\0'; /* force scanf to fail */
              }
              if(1 == sscanf(replyBuf,GET_EMMC_HEALTH_A_REPLY, &emmchealthA)) {
                  /* Today there is no error checking or fault handling on values. */
              } else {
                  localrsuhealth.systemstates |= RSUHEALTH_EMMCA_SSCANF_FAIL;
                  emmchealthA = 999;
              }
          } else {
              I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"FGETS failed[%s]\n",GET_EMMC_HEALTH_A);
              localrsuhealth.systemstates |= RSUHEALTH_EMMCA_FGETS_FAIL;
              emmchealthA = 998;
          }
          PCLOSE(fp);
      } else {
          I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"POPEN failed[%s]\n",GET_EMMC_HEALTH_A);
          localrsuhealth.systemstates |= RSUHEALTH_EMMCA_POPEN_FAIL;
          emmchealthA = 997;
      }
      memset(replyBuf,'\0',sizeof(replyBuf));
      if(NULL != (fp = POPEN(GET_EMMC_HEALTH_B, "r"))) {
          if(NULL != fgets(replyBuf, sizeof(replyBuf), fp)){
              i = strnlen(replyBuf,sizeof(replyBuf));
              if((0 != i) && (i <= sizeof(replyBuf))){
                  replyBuf[i-1] = '\0'; /* delete extra char added. */
              } else {
                  replyBuf[0] = '\0'; /* force scanf to fail */
              }
              if(1 == sscanf(replyBuf,GET_EMMC_HEALTH_B_REPLY, &emmchealthB)) {
                  /* Today there is no error checking or fault handling on values. */
              } else {
                  localrsuhealth.systemstates |= RSUHEALTH_EMMCB_SSCANF_FAIL;
                  emmchealthB = 999;
              }
          } else {
              localrsuhealth.systemstates |= RSUHEALTH_EMMCB_FGETS_FAIL;
              emmchealthB = 998;
          }
          PCLOSE(fp);
      } else {
          localrsuhealth.systemstates |= RSUHEALTH_EMMCB_POPEN_FAIL;
          emmchealthB = 997;
      }
      memset(replyBuf,'\0',sizeof(replyBuf));
      if(NULL != (fp = POPEN(GET_EMMC_HEALTH_EOL, "r"))) {
          if(NULL != fgets(replyBuf, sizeof(replyBuf), fp)){
              i = strnlen(replyBuf,sizeof(replyBuf));
              if((0 != i) && (i <= sizeof(replyBuf))){
                  replyBuf[i-1] = '\0'; /* delete extra char added. */
              } else { 
                  replyBuf[0] = '\0'; /* force scanf to fail */
              }
              if(1 == sscanf(replyBuf,GET_EMMC_HEALTH_EOL_REPLY, &emmchealthEOL)) {
                  /* Today there is no error checking or fault handling on values. */
              } else {
                  localrsuhealth.systemstates |= RSUHEALTH_EMMCEOL_SSCANF_FAIL;
                  emmchealthEOL = 999;
              }
          } else {
              localrsuhealth.systemstates |= RSUHEALTH_EMMCEOL_FGETS_FAIL;
              emmchealthEOL = 998;
          }
          PCLOSE(fp);
      } else {
          localrsuhealth.systemstates |= RSUHEALTH_EMMCEOL_POPEN_FAIL;
          emmchealthEOL = 997;
      }

      /* Range check all three values together. */
      if(   (0x0 == emmchealthA)   || (0xb < (0xf & emmchealthA))
         || (0x0 == emmchealthB)   || (0xb < (0xf & emmchealthB))
         || (0x0 == emmchealthEOL) || (0x3 < (0xf & emmchealthEOL))) {
          localrsuhealth.systemstates |= RSUHEALTH_EMMCEOL_RANGE_FAIL;
          I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"EMMC: health range error: emmchealthA(0x%X) emmchealthB(0x%X) emmchealthEOL(0x%X)\n",emmchealthA,emmchealthB,emmchealthEOL);
       } else {
          //localrsuhealth.systemstates &= ~RSUHEALTH_EMMCEOL_RANGE_FAIL;

          /* If the device is reaching EOL send message to serial only with critical level. */
          if(   (0x0 == emmchealthA)   || (0xa == emmchealthA) || (0xb == emmchealthA)
             || (0x0 == emmchealthB)   || (0xa == emmchealthB) || (0xb == emmchealthB)
             || (0x0 == emmchealthEOL) || (0x3 == emmchealthEOL)) {
              /* 5.18.1.2 Storage Integrity Error Message: Errors in file system. */
              if(0x0 == (localrsuhealth.systemstates & RSUHEALTH_EMMCEOL_FAULT)){
                  memset(errormsg,0x0,sizeof(errormsg));
                  snprintf(errormsg,sizeof(errormsg), "[EMMC: EOL approaching: emmchealthA(0x%X) emmchealthB(0x%X) emmchealthEOL(0x%X)]",emmchealthA,emmchealthB,emmchealthEOL);
                  setuptrap(SecStorageIntegrityID, rsuAlertLevel_critical, errormsg);
              }
              I2V_DBG_LOG(LEVEL_CRIT, MY_NAME,"EMMC: EOL approaching: emmchealthA(0x%X) emmchealthB(0x%X) emmchealthEOL(0x%X)\n",emmchealthA,emmchealthB,emmchealthEOL);
              localrsuhealth.systemstates |= RSUHEALTH_EMMCEOL_FAULT;
          } else { 
              I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"EMMC: health nominal: emmchealthA(0x%X) emmchealthB(0x%X) emmchealthEOL(0x%X)\n",emmchealthA,emmchealthB,emmchealthEOL);
          }
      }
      /* Get size, used, remaining, percent used emmc values. */
      memset(replyBuf,'\0',sizeof(replyBuf));
      if(NULL != (fp = POPEN(GET_EMMC_USAGE, "r"))) {
          if(NULL != fgets(replyBuf, sizeof(replyBuf), fp)){
              i = strnlen(replyBuf,sizeof(replyBuf));
              if((0 != i) && (i <= sizeof(replyBuf))){
                  replyBuf[i-1] = '\0'; /* delete extra char added. */
              } else { 
                  replyBuf[0] = '\0'; /* force scanf to fail */
              }
              if(4 == sscanf(replyBuf,GET_EMMC_USAGE_REPLY, size_s, used_s, remaining_s, percent_s)) {
                  for(i=0;(i<32) && ('\0' != percent_s[i]) ;i++) {
                      if('%' == percent_s[i]) {
                          percent_s[i] = '\0';
                          break;    
                      }
                  }
                  /* conversion will stop at % sign */
                  percent = strtod(percent_s, &endptr); /* Range checked below */
              } else {
                  localrsuhealth.systemstates |= RSUHEALTH_EMMCUSE_SSCANF_FAIL;
                  percent = -999.0;
              }
          } else {
              localrsuhealth.systemstates |= RSUHEALTH_EMMCUSE_FGETS_FAIL;
              percent = -998.0;
          }
          PCLOSE(fp);
      } else {
          localrsuhealth.systemstates |= RSUHEALTH_EMMCUSE_POPEN_FAIL;
          percent = -997.0;
      }
      /* If no parsing fails and range is good use. */
      if(percent > -900.0){
          if((percent < 0.0) || (100.0 < percent)) {
              localrsuhealth.systemstates |= RSUHEALTH_EMMCUSE_RANGE_FAIL;
              I2V_DBG_LOG(LEVEL_CRIT, MY_NAME,"EMMC: use range error: percent[%lf]: [%s]\n",percent, replyBuf);
          } else {
              //localrsuhealth.systemstates &= ~RSUHEALTH_EMMCUSE_RANGE_FAIL;
              if(percent < localrsuhealth.systems.emmcusemin)
                  localrsuhealth.systems.emmcusemin = percent;
              if(localrsuhealth.systems.emmcusemax < percent)
                  localrsuhealth.systems.emmcusemax = percent;

              if(EMMC_USE_MAX_THRESHOLD < percent) {
                  I2V_DBG_LOG(LEVEL_CRIT, MY_NAME,"EMMC: use is near or at full: [%s] current,min,max(%.2lf,%.2lf,%.2lf)\n",replyBuf,percent,localrsuhealth.systems.emmcusemin,localrsuhealth.systems.emmcusemax);
                  /* 5.18.1.2 Storage Integrity Error Message: Errors in file system. */
                  if(0x0 == (localrsuhealth.systemstates & RSUHEALTH_EMMCUSE_FAULT)){
                      memset(errormsg,0x0,sizeof(errormsg));
                      snprintf(errormsg,sizeof(errormsg), "[EMMC: use is near or at full: [%s] current,min,max(%.2lf,%.2lf,%.2lf)]",replyBuf,percent,localrsuhealth.systems.emmcusemin,localrsuhealth.systems.emmcusemax);
                      setuptrap(SecStorageIntegrityID, rsuAlertLevel_critical, errormsg);
                  }
                  localrsuhealth.systemstates |= RSUHEALTH_EMMCUSE_FAULT;
              } else { 
                  I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"EMMC: use nominal: [%s] current,min,max(%.2lf,%.2lf,%.2lf)\n",replyBuf,percent,localrsuhealth.systems.emmcusemin,localrsuhealth.systems.emmcusemax);
                  localrsuhealth.systemstates &= ~RSUHEALTH_EMMCUSE_FAULT;
              }
          }
      } else {
          I2V_DBG_LOG(LEVEL_CRIT, MY_NAME,"EMMC: parse error: [%s]\n",replyBuf);
      }
  }
}
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
static void checkcustomernotifications(void)
{
  if(70 == (onesecondcounter % CUSTOMER_FOLDER_CHECK_MODULUS)){
      if(customerfoldererrorstates & CUSTOMER_CLEAN_STATE){
          /* 5.18.1.14 Enclosure Environment: although maybe not techincally environment? */
          setuptrap(EnvironID, rsuAlertLevel_warning, "[/rwflash/customer is too full and being pruned]");
          /* Now clear event after trap sent */
          customerfoldererrorstates &= ~CUSTOMER_CLEAN_STATE;

      }
      if(customerfoldererrorstates & CUSTOMER_CLEAN_SYSFAIL){
          /* 5.18.1.14 Enclosure Environment: although maybe not techincally environment? */
          setuptrap(EnvironID, rsuAlertLevel_critical, "[syscall failed on customer folder: check syslogs]");
          /* Now clear event after trap sent */
          customerfoldererrorstates &= ~CUSTOMER_CLEAN_SYSFAIL;
      }
  }
}
/*----------------------------------------------------------------------------*/
/* JIRA TBD: cpu load, ram usage, wd, nvm usage, storage integrity            */
/*
top -b -n 1 snapshot
*/
/*----------------------------------------------------------------------------*/
static void checksystem(void)
{
  if((60 < onesecondcounter) && (0 == (onesecondcounter % 60))) {
      checkemmc();
  }
}
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
static void printgnssstatus(void)
{
  I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"event occurred: gnssstatus(0x%x)\n", localrsuhealth.gnssstatus);
  if(localrsuhealth.gnssstatus & RSUHEALTH_GNSS_FIX_VALID) {
      I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"RSUHEALTH_GNSS_FIX_VALID\n");
  }
  if(0x0 == (localrsuhealth.gnssstatus & RSUHEALTH_GNSS_FIX_VALID)) {
      if(priorlocalrsuhealth.gnssstatus & RSUHEALTH_GNSS_FIX_VALID) {
          I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"Just lost GNSS fix.\n");
      }
  }
  if(localrsuhealth.gnssstatus & RSUHEALTH_GNSS_BAD_ANTENNA) {
      I2V_DBG_LOG(LEVEL_CRIT, MY_NAME,"RSUHEALTH_GNSS_BAD_ANTENNA\n");
  }
  if(localrsuhealth.gnssstatus & RSUHEALTH_GNSS_BAD_LOC_DEV) {
      I2V_DBG_LOG(LEVEL_CRIT, MY_NAME,"RSUHEALTH_GNSS_BAD_LOC_DEV\n");
  }
  if(localrsuhealth.gnssstatus & RSUHEALTH_GNSS_BAD_TTFF) {
      I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"RSUHEALTH_GNSS_BAD_TTFF\n");
  }
  if(localrsuhealth.gnssstatus & RSUHEALTH_GNSS_BAD_FW) {
      I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"RSUHEALTH_GNSS_BAD_FW\n");
  }
  if(localrsuhealth.gnssstatus & RSUHEALTH_GNSS_BAD_CFG) {
      I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"RSUHEALTH_GNSS_BAD_CFG\n");
  }
  if(localrsuhealth.gnssstatus & RSUHEALTH_1PPS_BAD_FETCH) {
      I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"RSUHEALTH_1PPS_BAD_FETCH\n");
  }
  if(localrsuhealth.gnssstatus & RSUHEALTH_1PPS_BAD_LATENCY) {
      I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"RSUHEALTH_1PPS_BAD_LATENCY\n");
  }
  if(localrsuhealth.gnssstatus & RSUHEALTH_1PPS_TPS_BAD_LATENCY) {
      I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"RSUHEALTH_1PPS_TPS_BAD_LATENCY\n");
  }
  if(localrsuhealth.gnssstatus & RSUHEALTH_1PPS_BAD_TIMESTAMP) {
      I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"RSUHEALTH_1PPS_BAD_TIMESTAMP\n");
  }
  if(localrsuhealth.gnssstatus & RSUHEALTH_1PPS_BAD_MKTIME) {
      I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"RSUHEALTH_1PPS_BAD_MKTIME\n");
  }
  if(localrsuhealth.gnssstatus & RSUHEALTH_1PPS_HALTED) {
      I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"RSUHEALTH_1PPS_HALTED\n");
  }
  if(localrsuhealth.gnssstatus & RSUHEALTH_1PPS_INIT_FAIL) {
      I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"RSUHEALTH_1PPS_INIT_FAIL\n");
  }
  if(localrsuhealth.gnssstatus & RSUHEALTH_GNSS_TPS_SEQNO_STUCK) {
      I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"RSUHEALTH_GNSS_TPS_SEQNO_STUCK\n");
  }
  if(localrsuhealth.gnssstatus & RSUHEALTH_GNSS_PVT_STUCK) {
      I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"RSUHEALTH_GNSS_PVT_STUCK\n");
  }
  if(localrsuhealth.gnssstatus & RSUHEALTH_GNSS_PVT_UNUSED) {
      I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"RSUHEALTH_GNSS_PVT_UNUSED\n");
  }
  if(localrsuhealth.gnssstatus & RSUHEALTH_GNSS_GGA_STUCK) {
      I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"RSUHEALTH_GNSS_GGA_STUCK\n");
  }
  if(localrsuhealth.gnssstatus & RSUHEALTH_GNSS_NO_TIME_ACCURACY) {
      I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"RSUHEALTH_GNSS_NO_TIME_ACCURACY\n");
  }
  if(localrsuhealth.gnssstatus & RSUHEALTH_GNSS_BAD_TIME_ACCURACY) {
      I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"RSUHEALTH_GNSS_BAD_TIME_ACCURACY\n");
  }
  if(localrsuhealth.gnssstatus & RSUHEALTH_GNSS_RTC_BAD_CAL) {
      I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"RSUHEALTH_GNSS_RTC_BAD_CAL\n");
  }
}
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
static void printcv2xstatus(void)
{
  I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"event occurred: cv2xstatus(0x%x)\n", localrsuhealth.cv2xstatus);
  if(localrsuhealth.cv2xstatus & RSUHEALTH_CV2X_TX_READY) {
      I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"RSUHEALTH_CV2X_TX_READY\n");
  }
  if(localrsuhealth.cv2xstatus & RSUHEALTH_CV2X_RX_READY) {
      I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"RSUHEALTH_CV2X_RX_READY\n");
  }
  if(localrsuhealth.cv2xstatus & RSUHEALTH_CV2X1_BAD_ANTENNA) {
      I2V_DBG_LOG(LEVEL_CRIT, MY_NAME,"RSUHEALTH_CV2X1_BAD_ANTENNA\n");
  }
  if(localrsuhealth.cv2xstatus & RSUHEALTH_CV2X2_BAD_ANTENNA) {
      I2V_DBG_LOG(LEVEL_CRIT, MY_NAME,"RSUHEALTH_CV2X2_BAD_ANTENNA\n");
  }
  if(localrsuhealth.cv2xstatus & RSUHEALTH_CV2X_TEMP_FAIL) {
      I2V_DBG_LOG(LEVEL_CRIT, MY_NAME,"RSUHEALTH_CV2X_TEMP_FAIL\n");
  }
  if(localrsuhealth.cv2xstatus & RSUHEALTH_CV2X_BAD_FW_REVISION) {
      I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"RSUHEALTH_CV2X_BAD_FW_REVISION\n");
  }
  if(localrsuhealth.cv2xstatus & RSUHEALTH_CV2X_BAD_INTERFACE) {
      I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"RSUHEALTH_CV2X_BAD_INTERFACE\n");
  }
  if(localrsuhealth.cv2xstatus & RSUHEALTH_CV2X_INIT_FAIL) {
      I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"RSUHEALTH_CV2X_INIT_FAIL\n");
  }
  if(localrsuhealth.cv2xstatus & RSUHEALTH_HSM_TEMP_FAIL) {
      I2V_DBG_LOG(LEVEL_CRIT, MY_NAME,"HSM_BAD_TEMPERATURE\n");
  }
  if(localrsuhealth.cv2xstatus & RSUHEALTH_HSM_BAD_CFG) {
      I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"RSUHEALTH_HSM_BAD_CFG\n");
  }
  if(localrsuhealth.cv2xstatus & RSUHEALTH_HSM_BAD_JCOP) {
      I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"RSUHEALTH_HSM_BAD_JCOP\n");
  }
  if(localrsuhealth.cv2xstatus & RSUHEALTH_HSM_BAD_USAPP) {
      I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"RSUHEALTH_HSM_BAD_USAPP\n");
  }
  if(localrsuhealth.cv2xstatus & RSUHEALTH_HSM_BAD_GSA) {
      I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"RSUHEALTH_HSM_BAD_GSA\n");
  }
  if(localrsuhealth.cv2xstatus & RSUHEALTH_AEROLINK_DIR_FAIL) {
      I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"RSUHEALTH_AEROLINK_DIR_FAIL\n");
  }
  if(localrsuhealth.cv2xstatus & RSUHEALTH_SECURITY_FOPEN_FAIL) {
      I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"RSUHEALTH_SECURITY_FOPEN_FAIL\n");
  }
}
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
static void printerrorstates(void)
{
  I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"event occurred: (0x%lx)\n",localrsuhealth.errorstates);
  if(localrsuhealth.errorstates & RSUHEALTH_SSCANF_FAIL) {
      I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"RSUHEALTH_SSCANF_FAIL\n");
  }
  if(localrsuhealth.errorstates & RSUHEALTH_FGETS_FAIL) {
      I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"RSUHEALTH_FGETS_FAIL\n");
  }
  if(localrsuhealth.errorstates & RSUHEALTH_POPEN_FAIL) {
      I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"RSUHEALTH_POPEN_FAIL\n");
  }
  if(localrsuhealth.errorstates & RSUHEALTH_STALE_PVT) {
      I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"RSUHEALTH_STALE_PVT\n");
  }
  if(localrsuhealth.errorstates & RSUHEALTH_BOGUS_PVT_INDEX) {
      I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"RSUHEALTH_BOGUS_PVT_INDEX\n");
  }
  if(localrsuhealth.errorstates & RSUHEALTH_STALE_SOUS) {
      I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"RSUHEALTH_STALE_SOUS\n");
  }
  if(localrsuhealth.errorstates & RSUHEALTH_TPS_SHM_FAIL) {
      I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"RSUHEALTH_TPS_SHM_FAIL\n");
  }
  if(localrsuhealth.errorstates & RSUHEALTH_SOUS_SHM_FAIL) {
      I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"RSUHEALTH_SOUS_SHM_FAIL\n");
  }
  if(localrsuhealth.errorstates & RSUHEALTH_SHM_FAIL) {
      I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"RSUHEALTH_SHM_FAIL\n");
  }
  if(localrsuhealth.errorstates & RSUHEALTH_GUI_SCRIPT_FAIL) {
      I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"RSUHEALTH_GUI_SCRIPT_FAIL\n");
  }
  if(localrsuhealth.errorstates & RSUHEALTH_READY_SET_FAIL) {
      I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"RSUHEALTH_READY_SET_FAIL\n");
  }
  if(localrsuhealth.errorstates & RSUHEALTH_I2V_RESET) {
      I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"RSUHEALTH_I2V_RESET\n");
  }
  if(localrsuhealth.errorstates & RSUHEALTH_BOGUS_LED_STATE) {
      I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"RSUHEALTH_BOGUS_LED_STATE\n");
  }
  if(localrsuhealth.errorstates & RSUHEALTH_READY_FILE_FAIL) {
      I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"RSUHEALTH_READY_FILE_FAIL\n");
  }
  if(localrsuhealth.errorstates & RSUHEALTH_SYSSETTS_SHM_FAIL) {
      I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"RSUHEALTH_SYSSETTS_SHM_FAIL\n");
  }
  if(localrsuhealth.errorstates & RSUHEALTH_SHM_SEMA_FAIL) {
      I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"RSUHEALTH_SHM_SEMA_FAIL\n");
  }
  if(localrsuhealth.errorstates & RSUHEALTH_DCIN_FAIL) {
      I2V_DBG_LOG(LEVEL_CRIT, MY_NAME,"RSUHEALTH_DCIN_FAIL\n");
  }
  if(localrsuhealth.errorstates & RSUHEALTH_CPU_TEMP_FAIL) {
      I2V_DBG_LOG(LEVEL_CRIT, MY_NAME,"RSUHEALTH_CPU_TEMP_FAIL\n");
  }
  if(localrsuhealth.errorstates & RSUHEALTH_AMBIENT_TEMP_FAIL) {
      I2V_DBG_LOG(LEVEL_CRIT, MY_NAME,"RSUHEALTH_AMBIENT_TEMP_FAIL\n");
  }
  if(localrsuhealth.errorstates & RSUHEALTH_NETWORK_LINK_FAIL) {
      I2V_DBG_LOG(LEVEL_CRIT, MY_NAME,"RSUHEALTH_NETWORK_LINK_FAIL\n");
  }
  if(localrsuhealth.errorstates & RSUHEALTH_NETWORK_INIT_FAIL) {
      I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"RSUHEALTH_NETWORK_INIT_FAIL\n");
  }
  if(localrsuhealth.errorstates & RSUHEALTH_IP_GET_FAIL) {
      I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"RSUHEALTH_IP_GET_FAIL\n");
  }
  if(localrsuhealth.errorstates & RSUHEALTH_IP_RX_GET_FAIL) {
      I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"RSUHEALTH_IP_RX_GET_FAIL\n");
  }
  if(localrsuhealth.errorstates & RSUHEALTH_IP_TX_GET_FAIL) {
      I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"RSUHEALTH_IP_TX_GET_FAIL\n");
  }
  if(localrsuhealth.errorstates & RSUHEALTH_SYS_CALL_FAIL) {
      I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"RSUHEALTH_SYS_CALL_FAIL\n");
  }
  if(localrsuhealth.errorstates & RSUHEALTH_IP_INFO_FAIL) {
      I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"RSUHEALTH_IP_INFO_FAIL\n");
  }
  if(localrsuhealth.errorstates & RSUHEALTH_NETMASK_GET_FAIL) {
      I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"RSUHEALTH_NETMASK_GET_FAIL\n");
  }
  if(localrsuhealth.errorstates & RSUHEALTH_SEC_CERT_FAIL) {
      I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"RSUHEALTH_SEC_CERT_FAIL\n");
  }
  if(localrsuhealth.errorstates & RSUHEALTH_SEC_CERT_EXPIRED) {
      I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"RSUHEALTH_SEC_CERT_EXPIRED\n");
  }
  if(localrsuhealth.errorstates & RSUHEALTH_SEC_CERT_NEAR_EXP) {
      I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"RSUHEALTH_SEC_CERT_NEAR_EXP\n");
  }
  if(localrsuhealth.errorstates & RSUHEALTH_SEC_CERT_BOGUS) {
      I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"RSUHEALTH_SEC_CERT_BOGUS\n");
  }
  if(localrsuhealth.errorstates & RSUHEALTH_SEC_CERT_FULL) {
      I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"RSUHEALTH_SEC_CERT_FULL\n");
  }
  if(localrsuhealth.errorstates & RSUHEALTH_STANDBY_MODE) {
      I2V_DBG_LOG(LEVEL_CRIT, MY_NAME,"RSUHEALTH_STANDBY_MODE\n");
  }
  if(localrsuhealth.errorstates & RSUHEALTH_FAULT_MODE) {
      I2V_DBG_LOG(LEVEL_CRIT, MY_NAME,"RSUHEALTH_FAULT_MODE\n");
  }
  if(localrsuhealth.errorstates & RSUHEALTH_NSSTATS_FAILED) {
      I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"RSUHEALTH_NSSTATS_FAILED\n");
  }
  if(localrsuhealth.errorstates & RSUHEALTH_MANAGE_FOLDER_OFF) {
      I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"RSUHEALTH_MANAGE_FOLDER_OFF\n");
  }
  if(localrsuhealth.errorstates & RSUHEALTH_SYSCALL_FAIL) {
      I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"RSUHEALTH_SYSCALL_FAIL\n");
  }
}
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
static void seedstatisticalmemory(void)
{
  localrsuhealth.temperature.cv2xmin =  999;
  localrsuhealth.temperature.cv2xmax = -999;
  localrsuhealth.temperature.cv2x = localrsuhealth.temperature.cv2xmin; /* init state, dont use */

  localrsuhealth.temperature.hsmmin  =  999.0;
  localrsuhealth.temperature.hsmmax  = -999.0;
  localrsuhealth.temperature.hsm = localrsuhealth.temperature.hsmmin; /* init state, dont use */

  localrsuhealth.temperature.cpumin =  999.0;
  localrsuhealth.temperature.cpumax = -999.0;
  localrsuhealth.temperature.cpu = localrsuhealth.temperature.cpumin; /* init state, dont use */

  localrsuhealth.temperature.ambientmin =  999.0;
  localrsuhealth.temperature.ambientmax = -999.0;
  localrsuhealth.temperature.ambient = localrsuhealth.temperature.ambientmin; /* init state, dont use */

  localrsuhealth.volts.dcinmin =  999.0f;
  localrsuhealth.volts.dcinmax = -999.0f;
  localrsuhealth.volts.dcin = localrsuhealth.volts.dcinmin; /* init state, dont use */

  localrsuhealth.systems.emmcusemin =  999.0f;
  localrsuhealth.systems.emmcusemax = -999.0f;
} 
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
static void calculatestatistics(void)
{
  /* Don't check until a value has been read. */

  if(999 != localrsuhealth.temperature.cv2x) {

      if(localrsuhealth.temperature.cv2x < localrsuhealth.temperature.cv2xmin) 
          localrsuhealth.temperature.cv2xmin = localrsuhealth.temperature.cv2x;

      if(localrsuhealth.temperature.cv2x > localrsuhealth.temperature.cv2xmax) 
          localrsuhealth.temperature.cv2xmax = localrsuhealth.temperature.cv2x;

  }
  if(999.0 != localrsuhealth.temperature.hsm) {

      if(localrsuhealth.temperature.hsm < localrsuhealth.temperature.hsmmin) 
          localrsuhealth.temperature.hsmmin = localrsuhealth.temperature.hsm;
  
      if(localrsuhealth.temperature.hsm > localrsuhealth.temperature.hsmmax) 
          localrsuhealth.temperature.hsmmax = localrsuhealth.temperature.hsm;

  }
  if(999.0 != localrsuhealth.temperature.cpu) {

      if(localrsuhealth.temperature.cpu < localrsuhealth.temperature.cpumin) 
          localrsuhealth.temperature.cpumin = localrsuhealth.temperature.cpu;

      if(localrsuhealth.temperature.cpu > localrsuhealth.temperature.cpumax) 
          localrsuhealth.temperature.cpumax = localrsuhealth.temperature.cpu;

  }
  if(999.0 != localrsuhealth.temperature.ambient) {

      if(localrsuhealth.temperature.ambient < localrsuhealth.temperature.ambientmin) 
          localrsuhealth.temperature.ambientmin = localrsuhealth.temperature.ambient;

      if(localrsuhealth.temperature.ambient > localrsuhealth.temperature.ambientmax) 
          localrsuhealth.temperature.ambientmax = localrsuhealth.temperature.ambient;

  }
  if(999.0f != localrsuhealth.volts.dcin) {

      if(localrsuhealth.volts.dcin < localrsuhealth.volts.dcinmin) 
          localrsuhealth.volts.dcinmin = localrsuhealth.volts.dcin;

      if(localrsuhealth.volts.dcin > localrsuhealth.volts.dcinmax) 
          localrsuhealth.volts.dcinmax = localrsuhealth.volts.dcin;

  }
}
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
static void dumphealthoutput(void) 
{
  uint8_t  n = 0; 

  /* To serial and syslog. Redundent to syslog for now */
#if defined(MY_UNIT_TEST)
  if(0 == (onesecondcounter % OUTPUT_MODULUS)) {
#else
  if(1 == (onesecondcounter % OUTPUT_MODULUS)) {
#endif
      I2V_DBG_LOG(LEVEL_INFO,MY_NAME,"(0x%lx) gnss(0x%x,%u,%u) tps(0x%x) cv2x(%u:0x%x) rsk(0x%x) sys(0x%x) wsm(0x%x)\n",
          localrsuhealth.errorstates
          ,localrsuhealth.gnssstatus
          ,localshmtps.debug_cnt_gps_fix_lost
          ,localshmtps.debug_cnt_gps_time_and_system_time_out_of_sync
          ,localshmtps.error_states
          ,localrsuhealth.cv2xruntime
          ,localrsuhealth.cv2xstatus
          ,rskstatus.error_states
          ,localrsuhealth.systemstates
          ,rskstatus.wsm_error_states);
  }
  /* Everything else to syslog only */
  if(0 == (onesecondcounter % SYSLOG_OUTPUT_MODULUS)) {
      I2V_DBG_LOG(LEVEL_PRIV, MY_NAME,"(0x%lx) gnss(0x%x,%u,%u) tps(0x%x) cv2x(%u:0x%x) rsk(0x%x) sys(0x%x) wsm(0x%x)\n",
          localrsuhealth.errorstates
          ,localrsuhealth.gnssstatus
          ,localshmtps.debug_cnt_gps_fix_lost
          ,localshmtps.debug_cnt_gps_time_and_system_time_out_of_sync
          ,localshmtps.error_states
          ,localrsuhealth.cv2xruntime
          ,localrsuhealth.cv2xstatus
          ,rskstatus.error_states
          ,localrsuhealth.systemstates
          ,rskstatus.wsm_error_states);
      I2V_DBG_LOG(LEVEL_PRIV, MY_NAME,"temp: cv2x(%d,%d,%d) hsm(%03.2lf,%03.2lf,%03.2lf) cpu(%03.2lf,%03.2lf,%03.2lf)\n",
      localrsuhealth.temperature.cv2x
      ,localrsuhealth.temperature.cv2xmin
      ,localrsuhealth.temperature.cv2xmax
      ,localrsuhealth.temperature.hsm
      ,localrsuhealth.temperature.hsmmin
      ,localrsuhealth.temperature.hsmmax
      ,localrsuhealth.temperature.cpu
      ,localrsuhealth.temperature.cpumin
      ,localrsuhealth.temperature.cpumax);
      I2V_DBG_LOG(LEVEL_PRIV, MY_NAME,"temp: amb(%03.2lf,%03.2lf,%03.2lf) volts: dcin(%03.2f,%03.2f,%03.2f)\n",
       localrsuhealth.temperature.ambient
      ,localrsuhealth.temperature.ambientmin
      ,localrsuhealth.temperature.ambientmax
      ,localrsuhealth.volts.dcin
      ,localrsuhealth.volts.dcinmin
      ,localrsuhealth.volts.dcinmax);
      I2V_DBG_LOG(LEVEL_PRIV, MY_NAME,"IP(cnt,err,drp,ovr,f/c) rx(0x%lx,0x%lx,0x%lx,0x%lx,0x%lx) tx(0x%lx,0x%lx,0x%lx,0x%lx,0x%lx)\n",
      localrsuhealth.ipcounts.txpackets
      ,localrsuhealth.ipcounts.txerrors
      ,localrsuhealth.ipcounts.txdropped
      ,localrsuhealth.ipcounts.txoverruns
      ,localrsuhealth.ipcounts.txcarrier
      ,localrsuhealth.ipcounts.rxpackets
      ,localrsuhealth.ipcounts.rxerrors
      ,localrsuhealth.ipcounts.rxdropped
      ,localrsuhealth.ipcounts.rxoverruns
      ,localrsuhealth.ipcounts.rxframe);
  }
  /* TODO: Looks like azim, prRes and elev are decoded or encoded wrong. Oh well. */
  if(0 == (onesecondcounter % 1203)) {
      I2V_DBG_LOG(LEVEL_PRIV, MY_NAME,"GNSS: current time accuracy error(%u) min/max/thresh(%u,%u,%u)\n",
          localshmtps.current_time_accuracy,localshmtps.time_accuracy_min
          ,localshmtps.time_accuracy_max,localshmtps.time_accuracy_threshold);
      I2V_DBG_LOG(LEVEL_PRIV, MY_NAME,"gps sv cnt(%u) glo sv cnt(%u)\n",localshmtps.sat_detail.cnt_gps_sats,localshmtps.sat_detail.cnt_glo_sats);
      for (n=0; (n < MAX_DETAIL_SATS) && (n < localshmtps.sat_detail.cnt_gps_sats); n++) {
          if(localshmtps.sat_detail.gps[n].svUsed){
              I2V_DBG_LOG(LEVEL_PRIV, MY_NAME,"gps:svid(%hhu) CNR(%hhu) qual(%hhu) health(%hhu) elv(%d) azim(%d) prR(%d)\n",
                        localshmtps.sat_detail.gps[n].svId
                        ,localshmtps.sat_detail.gps[n].CNR
                        ,localshmtps.sat_detail.gps[n].quality
                        ,localshmtps.sat_detail.gps[n].health
                        ,localshmtps.sat_detail.gps[n].svUsed
                        ,localshmtps.sat_detail.gps[n].elev
                        ,localshmtps.sat_detail.gps[n].azim
                        ,localshmtps.sat_detail.gps[n].prRes);
              usleep(1000);
          }
      }
      for (n=0; (n < MAX_DETAIL_SATS) && (n < localshmtps.sat_detail.cnt_glo_sats); n++) {
          if(localshmtps.sat_detail.glo[n].svUsed){
              I2V_DBG_LOG(LEVEL_PRIV, MY_NAME,"glo:svid(%hhu) CNR(%hhu) qual(%hhu) health(%hhu) elv(%d) azim(%d) prR(%d)\n",
                        localshmtps.sat_detail.glo[n].svId
                        ,localshmtps.sat_detail.glo[n].CNR
                        ,localshmtps.sat_detail.glo[n].quality
                        ,localshmtps.sat_detail.glo[n].health
                        ,localshmtps.sat_detail.glo[n].svUsed
                        ,localshmtps.sat_detail.glo[n].elev
                        ,localshmtps.sat_detail.glo[n].azim
                        ,localshmtps.sat_detail.glo[n].prRes);
              usleep(1000);
          }
      }
  }
}
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
static void dumpchangedevents(void)
{
  /* On change dump events and update any SHM items */
  if(priorlocalrsuhealth.gnssstatus != localrsuhealth.gnssstatus) {
      printgnssstatus();
      priorlocalrsuhealth.gnssstatus = localrsuhealth.gnssstatus;
  }
  if(priorlocalrsuhealth.cv2xstatus != localrsuhealth.cv2xstatus) {
      printcv2xstatus();
      priorlocalrsuhealth.cv2xstatus = localrsuhealth.cv2xstatus;
  }
  if(priorlocalrsuhealth.errorstates != localrsuhealth.errorstates) {
      printerrorstates();
      priorlocalrsuhealth.errorstates = localrsuhealth.errorstates;  
  }
  if(priorrskstatus.error_states != rskstatus.error_states){
      printrskerrorstates();
  }
  memcpy(&priorrskstatus, &rskstatus, sizeof(rskstatus));
}
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
static void opendiagnosticshm(void)
{
  /* Open RSUHEALTH SHM, if fails not fatal. MIB and IWMH will suffer. */
  if ((shm_rsuhealth_ptr = wsu_share_init(sizeof(rsuhealth_t), RSUHEALTH_SHM_PATH)) == NULL) {
      I2V_DBG_LOG(LEVEL_CRIT, MY_NAME,"SHM init failed.\n");
      shm_rsuhealth_ptr = NULL; /* Keep going, not fatal. */
      localrsuhealth.errorstates |= RSUHEALTH_SHM_FAIL;
  } else {
      if (!wsu_shmlock_init(&shm_rsuhealth_ptr->h.ch_lock)) {
          I2V_DBG_LOG(LEVEL_CRIT,MY_NAME,"sem init failed for SHM.\n");
          wsu_share_kill(shm_rsuhealth_ptr, sizeof(rsuhealth_t));
          shm_rsuhealth_ptr = NULL;
          localrsuhealth.errorstates |= RSUHEALTH_SHM_SEMA_FAIL;
      }
  }
  /* open TPS SHM */
  if ((shm_tps_ptr = wsu_share_init(sizeof(shm_tps_t), SHM_TPS_PATH)) == NULL) {
      I2V_DBG_LOG(LEVEL_CRIT,MY_NAME,"TPS SHM init failed.\n");
      shm_tps_ptr = NULL; /* Keep going, not fatal. */
      localrsuhealth.errorstates |= RSUHEALTH_TPS_SHM_FAIL;
  }
  /* open SOUS SHM */
  if ((shm_sous_ptr = wsu_share_init(sizeof(shm_sous_t), SHM_SOUS_PATH)) == NULL) {
      I2V_DBG_LOG(LEVEL_CRIT,MY_NAME,"SOUS SHM init failed.\n");
      shm_sous_ptr = NULL; /* Keep going, not fatal. */
      localrsuhealth.errorstates |= RSUHEALTH_SOUS_SHM_FAIL;
  }
  if(NULL != shm_rsuhealth_ptr) {
      shm_rsuhealth_ptr->h.ch_data_valid = WFALSE;
      shm_rsuhealth_ptr->cv2xruntime = localrsuhealth.cv2xruntime = RSUHEALTH_CV2X_ANTENNA_CHECK; /* Set any defaults now */

      /* Set to DENSO empty cause zero'ed value is rsuAlertLevel_info */
      shm_rsuhealth_ptr->messageFileIntegrityError = localrsuhealth.messageFileIntegrityError;
      shm_rsuhealth_ptr->rsuSecStorageIntegrityError = localrsuhealth.rsuSecStorageIntegrityError;
      shm_rsuhealth_ptr->rsuAuthError = localrsuhealth.rsuAuthError;
      shm_rsuhealth_ptr->rsuSignatureVerifyError = localrsuhealth.rsuSignatureVerifyError;
      shm_rsuhealth_ptr->rsuAccessError = localrsuhealth.rsuAccessError;
      shm_rsuhealth_ptr->rsuTimeSourceLost = localrsuhealth.rsuTimeSourceLost;
      shm_rsuhealth_ptr->rsuTimeSourceMismatch = localrsuhealth.rsuTimeSourceMismatch;
      shm_rsuhealth_ptr->rsuGnssAnomaly = localrsuhealth.rsuGnssAnomaly;
      shm_rsuhealth_ptr->rsuGnssDeviationError = localrsuhealth.rsuGnssDeviationError;
      shm_rsuhealth_ptr->rsuCertificateError = localrsuhealth.rsuCertificateError;
      shm_rsuhealth_ptr->rsuServiceDenialError = localrsuhealth.rsuServiceDenialError;
      shm_rsuhealth_ptr->rsuWatchdogError = localrsuhealth.rsuWatchdogError;
      shm_rsuhealth_ptr->rsuEnvironError = localrsuhealth.rsuEnvironError;
  }
}
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
static void closediagnosticshm(void)
{
  /* Close SHM. Don't destroy. Recover on start up.*/
  if(NULL != shm_rsuhealth_ptr) {
      wsu_shmlock_kill(&shm_rsuhealth_ptr->h.ch_lock);
      wsu_share_kill(shm_rsuhealth_ptr, sizeof(rsuhealth_t));
      shm_rsuhealth_ptr = NULL;
  }
  if(NULL != shm_tps_ptr) {
      wsu_share_kill(shm_tps_ptr, sizeof(shm_tps_t));
      shm_tps_ptr = NULL;
  }
  if(NULL != shm_sous_ptr) {
      wsu_share_kill(shm_sous_ptr, sizeof(shm_sous_t));
      shm_sous_ptr = NULL;
  }
}
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
//TODO: If MIB is not ready then traps fall on the floor. Need ack from MIB beforehand.
//#define ENABLE_TEST_TRAPS
#if defined(ENABLE_TEST_TRAPS)
static uint32_t testiteration = 0;
#endif
#define MAX_TRY_LOCK 10
void updatediagnosticshm(void)
{
  int32_t i = 0;
  if(NULL != shm_rsuhealth_ptr) {
      for(i = 0; i < MAX_TRY_LOCK; i++) {
          if(WTRUE == wsu_shmlock_trylockw(&shm_rsuhealth_ptr->h.ch_lock)) {
              // memcpy(&localrsuhealth.h, &shm_rsuhealth_ptr->h, sizeof(wsu_shm_hdr_t)); /* Preserve header. */
              localrsuhealth.cv2xruntime = shm_rsuhealth_ptr->cv2xruntime; /* Take any runtime updates. */
              shm_rsuhealth_ptr->errorstates = localrsuhealth.errorstates;

              /* Special case: rsuGnssOutput.c will set & clear for us. */
              if(RSUHEALTH_GNSS_BAD_LOC_DEV & shm_rsuhealth_ptr->gnssstatus) {
                  localrsuhealth.gnssstatus |= RSUHEALTH_GNSS_BAD_LOC_DEV;
              } else {
                  localrsuhealth.gnssstatus &= ~RSUHEALTH_GNSS_BAD_LOC_DEV;
              }
              shm_rsuhealth_ptr->gnssstatus =  localrsuhealth.gnssstatus;
              memcpy(&shm_rsuhealth_ptr->gnsserrorcounts, &localrsuhealth.gnsserrorcounts,sizeof(localrsuhealth.gnsserrorcounts));
              shm_rsuhealth_ptr->cv2xstatus = localrsuhealth.cv2xstatus;
              memcpy(&shm_rsuhealth_ptr->cv2xerrorcounts, &localrsuhealth.cv2xerrorcounts,sizeof(localrsuhealth.cv2xerrorcounts));
              shm_rsuhealth_ptr->cv2xruntime = localrsuhealth.cv2xruntime;
              memcpy(&shm_rsuhealth_ptr->temperature, &localrsuhealth.temperature, sizeof(localrsuhealth.temperature));
              memcpy(&shm_rsuhealth_ptr->ipcounts, &localrsuhealth.ipcounts,sizeof(localrsuhealth.ipcounts));
              memcpy(&shm_rsuhealth_ptr->volts, &localrsuhealth.volts,sizeof(localrsuhealth.volts));
              shm_rsuhealth_ptr->systemstates = localrsuhealth.systemstates;
              memcpy(&shm_rsuhealth_ptr->systems, &localrsuhealth.systems, sizeof(localrsuhealth.systems));
              localrsuhealth.trapsenabled = shm_rsuhealth_ptr->trapsenabled; /* Got handshake from async in snmpd */

              if (1 == localrsuhealth.trapsenabled) {
                  /* 
                   * Only update and clear what we are responsible for.
                   * rsuAsync.c will sample at twice the rate.
                   */
                  shm_rsuhealth_ptr->rsuSecStorageIntegrityError = localrsuhealth.rsuSecStorageIntegrityError;
                  localrsuhealth.rsuSecStorageIntegrityError = rsuAlertLevel_denso_empty;
                  memcpy(shm_rsuhealth_ptr->rsuSecStorageIntegrityMsg
                      , localrsuhealth.rsuSecStorageIntegrityMsg
                      , RSUHEALTH_ALERT_MSG_LENGTH_MAX);
                  memset(localrsuhealth.rsuSecStorageIntegrityMsg,0x0,sizeof(localrsuhealth.rsuSecStorageIntegrityMsg));

                  shm_rsuhealth_ptr->rsuAuthError = localrsuhealth.rsuAuthError;
                  localrsuhealth.rsuAuthError = rsuAlertLevel_denso_empty;
                  memcpy(shm_rsuhealth_ptr->rsuAuthMsg
                      , localrsuhealth.rsuAuthMsg
                      , RSUHEALTH_ALERT_MSG_LENGTH_MAX);
                  memset(localrsuhealth.rsuAuthMsg,0x0,sizeof(localrsuhealth.rsuAuthMsg));

                  shm_rsuhealth_ptr->rsuSignatureVerifyError = localrsuhealth.rsuSignatureVerifyError;
                  localrsuhealth.rsuSignatureVerifyError = rsuAlertLevel_denso_empty;
                  memcpy(shm_rsuhealth_ptr->rsuSignatureVerifyMsg
                      , localrsuhealth.rsuSignatureVerifyMsg
                      , RSUHEALTH_ALERT_MSG_LENGTH_MAX);
                  memset(localrsuhealth.rsuSignatureVerifyMsg,0x0,sizeof(localrsuhealth.rsuSignatureVerifyMsg));
#if 0
                  shm_rsuhealth_ptr->rsuAccessError = localrsuhealth.rsuAccessError;
                  localrsuhealth.rsuAccessError = rsuAlertLevel_denso_empty;
                  memcpy(shm_rsuhealth_ptr->rsuAccessMsg
                      , localrsuhealth.rsuAccessMsg
                      , RSUHEALTH_ALERT_MSG_LENGTH_MAX);
                  memset(localrsuhealth.rsuAccessMsg,0x0,sizeof(localrsuhealth.rsuAccessMsg));
#endif
                  shm_rsuhealth_ptr->rsuTimeSourceLost = localrsuhealth.rsuTimeSourceLost;
                      localrsuhealth.rsuTimeSourceLost = rsuAlertLevel_denso_empty;
                  memcpy(shm_rsuhealth_ptr->rsuTimeSourceLostMsg
                      , localrsuhealth.rsuTimeSourceLostMsg
                      , RSUHEALTH_ALERT_MSG_LENGTH_MAX);
                  memset(localrsuhealth.rsuTimeSourceLostMsg,0x0,sizeof(localrsuhealth.rsuTimeSourceLostMsg));

                  shm_rsuhealth_ptr->rsuTimeSourceMismatch = localrsuhealth.rsuTimeSourceMismatch;
                  localrsuhealth.rsuTimeSourceMismatch = rsuAlertLevel_denso_empty;
                  memcpy(shm_rsuhealth_ptr->rsuTimeSourceMismatchMsg
                      , localrsuhealth.rsuTimeSourceMismatchMsg
                      , RSUHEALTH_ALERT_MSG_LENGTH_MAX);
                  memset(localrsuhealth.rsuTimeSourceMismatchMsg,0x0,sizeof(localrsuhealth.rsuTimeSourceMismatchMsg));

                  shm_rsuhealth_ptr->rsuGnssAnomaly = localrsuhealth.rsuGnssAnomaly;
                  localrsuhealth.rsuGnssAnomaly = rsuAlertLevel_denso_empty;
                  memcpy(shm_rsuhealth_ptr->rsuGnssAnomalyMsg
                      , localrsuhealth.rsuGnssAnomalyMsg
                      , RSUHEALTH_ALERT_MSG_LENGTH_MAX);
                  memset(localrsuhealth.rsuGnssAnomalyMsg,0x0,sizeof(localrsuhealth.rsuGnssAnomalyMsg));

                  shm_rsuhealth_ptr->rsuCertificateError = localrsuhealth.rsuCertificateError;
                  localrsuhealth.rsuCertificateError = rsuAlertLevel_denso_empty;
                  memcpy(shm_rsuhealth_ptr->rsuCertificateMsg
                      , localrsuhealth.rsuCertificateMsg
                      , RSUHEALTH_ALERT_MSG_LENGTH_MAX);
                  memset(localrsuhealth.rsuCertificateMsg,0x0,sizeof(localrsuhealth.rsuCertificateMsg));

                  shm_rsuhealth_ptr->rsuEnvironError = localrsuhealth.rsuEnvironError;
                  localrsuhealth.rsuEnvironError = rsuAlertLevel_denso_empty;
                  memcpy(shm_rsuhealth_ptr->rsuEnvironMsg
                      , localrsuhealth.rsuEnvironMsg
                      , RSUHEALTH_ALERT_MSG_LENGTH_MAX);
                  memset(localrsuhealth.rsuEnvironMsg,0x0,sizeof(localrsuhealth.rsuEnvironMsg));
                  /* As far as we are concerned the notifcations have been sent and handled. */
              }

              localrsuhealth.h.ch_sequence_number = shm_rsuhealth_ptr->h.ch_sequence_number++;
              localrsuhealth.h.ch_data_valid = shm_rsuhealth_ptr->h.ch_data_valid = WTRUE;

              if(WFALSE == wsu_shmlock_unlockw(&shm_rsuhealth_ptr->h.ch_lock)) {
                  if(0x0 == (localrsuhealth.errorstates & RSUHEALTH_SHM_SEMA_FAIL)) {
                      I2V_DBG_LOG(LEVEL_CRIT, MY_NAME,"wsu_shmlock_unlockw() failed.\n");
                      localrsuhealth.errorstates |= RSUHEALTH_SHM_SEMA_FAIL;
                  }
              }
              break; /* Done. */
          }
      }/* for */
      if (MAX_TRY_LOCK <= i) { /* Could happen. */
          if(localrsuhealth.errorstates & RSUHEALTH_SHM_SEMA_FAIL) {
              I2V_DBG_LOG(LEVEL_CRIT, MY_NAME,"wsu_shmlock_trylockw() failed.\n");
              localrsuhealth.errorstates |= RSUHEALTH_SHM_SEMA_FAIL;
          }
      }
  }
}
/*----------------------------------------------------------------------------*/
/* These events are general sys calls and don't clear on their own.           */
/* Only have relevence in the iteration they occured.                         */
/*----------------------------------------------------------------------------*/
static void clearevents(void)
{
  localrsuhealth.errorstates &= ~(uint64_t)RSUHEALTH_SSCANF_FAIL;
  localrsuhealth.errorstates &= ~(uint64_t)RSUHEALTH_FGETS_FAIL;
  localrsuhealth.errorstates &= ~(uint64_t)RSUHEALTH_POPEN_FAIL;

  localrsuhealth.cv2xstatus &= ~(uint64_t)RSUHEALTH_HSM_SSCANF_FAIL;
  localrsuhealth.cv2xstatus &= ~(uint64_t)RSUHEALTH_HSM_FGETS_FAIL;
  localrsuhealth.cv2xstatus &= ~(uint64_t)RSUHEALTH_HSM_POPEN_FAIL;

  localrsuhealth.gnssstatus &= ~(uint64_t)RSUHEALTH_GNSS_SSCANF_FAIL;
  localrsuhealth.gnssstatus &= ~(uint64_t)RSUHEALTH_GNSS_FGETS_FAIL;
  localrsuhealth.gnssstatus &= ~(uint64_t)RSUHEALTH_GNSS_POPEN_FAIL;
}
//TODO: deprecate or move?
/*----------------------------------------------------------------------------*/
/* JIRA 892: The RSU shall set a software controlled status indication output */
/*           controlling a red LED to ON (solid red) when the system is in a  */
/*           diagnostic state to indicate a fault has occurred.               */
/*----------------------------------------------------------------------------*/

static void openledshm(void) 
{
    /* Open LED SHM. */
    if ((shm_leddiag_ptr = wsu_share_init(sizeof(led_diagnostics_t), FAULTLED_SHM_PATH)) == NULL) {
        printf("LED SHM init failed. \n");
        shm_leddiag_ptr = NULL;
    } else {
        if (!wsu_shmlock_init(&shm_leddiag_ptr->h.ch_lock)) {
            printf("sem init fialed for LED shm \n");
            wsu_share_kill(shm_leddiag_ptr, sizeof(led_diagnostic_t));
            shm_leddiag_ptr = NULL;
        }
    }

    if (NULL != shm_leddiag_ptr) {
        /* set defaults */
       shm_leddiag_ptr->diag_thread = local_leddiag.diag_thread;
       shm_leddiag_ptr->gps_fix = local_leddiag.gps_fix;
       shm_leddiag_ptr->time_accuracy = local_leddiag.time_accuracy;
       shm_leddiag_ptr->gps_position = local_leddiag.gps_position;
       shm_leddiag_ptr->gnss_antenna = local_leddiag.gnss_antenna;
       shm_leddiag_ptr->cv2x_antenna = local_leddiag.cv2x_antenna;
       shm_leddiag_ptr->battery_voltage = local_leddiag.battery_voltage;
       shm_leddiag_ptr->network = local_leddiag.network;
       shm_leddiag_ptr->temperature = local_leddiag.temperature;
       shm_leddiag_ptr->hsm = local_leddiag.hsm;
       shm_leddiag_ptr->cv2x_interface = local_leddiag.cv2x_interface;
       shm_leddiag_ptr->max_error = local_leddiag.max_error;
    }
}

static void closeledshm(void) 
{
    /* Close LED SHM. Don't destroy. Recover on start up.  */
    if (NULL != shm_leddiag_ptr) {
        wsu_shmlock_kill(&shm_leddiag_ptr->h.ch_lock);
        wsu_share_kill(shm_leddiag_ptr, sizeof(led_diagnostics_t));
        shm_leddiag_ptr = NULL;
    }
}

void updateledshm(void) 
{
    int32_t i = 0;
    if (NULL != shm_leddiag_ptr) {
        for (i=0; i<MAX_TRY_LOCK; i++) {
            if(WTRUE == wsu_shmlock_trylockw(&shm_leddiag_ptr->h.ch_lock)) {
                memcpy(&shm_leddiag_ptr->diag_thread, &local_leddiag.diag_thread, sizeof(local_leddiag.diag_thread));
                memcpy(&shm_leddiag_ptr->gps_fix, &local_leddiag.gps_fix, sizeof(local_leddiag.gps_fix));
                memcpy(&shm_leddiag_ptr->time_accuracy, &local_leddiag.time_accuracy, sizeof(local_leddiag.time_accuracy));
                memcpy(&shm_leddiag_ptr->gps_position, &local_leddiag.gps_position, sizeof(local_leddiag.gps_position));
                memcpy(&shm_leddiag_ptr->gnss_antenna, &local_leddiag.gnss_antenna, sizeof(local_leddiag.gnss_antenna));
                memcpy(&shm_leddiag_ptr->cv2x_antenna, &local_leddiag.cv2x_antenna, sizeof(local_leddiag.cv2x_antenna));
                memcpy(&shm_leddiag_ptr->battery_voltage, &local_leddiag.battery_voltage, sizeof(local_leddiag.battery_voltage));
                memcpy(&shm_leddiag_ptr->network, &local_leddiag.network, sizeof(local_leddiag.network));
                memcpy(&shm_leddiag_ptr->temperature, &local_leddiag.temperature, sizeof(local_leddiag.temperature));
                memcpy(&shm_leddiag_ptr->hsm, &local_leddiag.hsm, sizeof(local_leddiag.hsm));
                memcpy(&shm_leddiag_ptr->cv2x_interface, &local_leddiag.cv2x_interface, sizeof(local_leddiag.cv2x_interface));
                memcpy(&shm_leddiag_ptr->max_error, &local_leddiag.max_error, sizeof(local_leddiag.max_error));

                shm_leddiag_ptr->faultledstate = local_leddiag.faultledstate;
                
                shm_leddiag_ptr->h.ch_sequence_number++;
                shm_leddiag_ptr->h.ch_data_valid = WTRUE;
            
                if (WFALSE == wsu_shmlock_unlockw(&shm_leddiag_ptr->h.ch_lock)) {
                    printf("wsu_shmlock_unlockw failed for led shm. \n");
                }
                break;
            }
        } //for
        if (MAX_TRY_LOCK <= i) {
            printf("wsu_shmlock_trylockw failed for led shm. \n");
        }
    }
}


void init_local_leddiag(void)
{
    local_leddiag.diag_thread.error_level = -1;
    local_leddiag.diag_thread.start_time = -1;
    sprintf(local_leddiag.diag_thread.name, "RSUHealth Diagnostic Thread");
    
    local_leddiag.gps_fix.error_level = -1;
    local_leddiag.gps_fix.start_time = -1;
    sprintf(local_leddiag.gps_fix.name, "GPS Fix");

    local_leddiag.time_accuracy.error_level = -1;
    local_leddiag.time_accuracy.start_time = -1;
    sprintf(local_leddiag.time_accuracy.name, "Time Accuracy");

    local_leddiag.gps_position.error_level = -1;
    local_leddiag.gps_position.start_time = -1;
    sprintf(local_leddiag.gps_position.name, "GPS Position");
    
    local_leddiag.gnss_antenna.error_level = -1;
    local_leddiag.gnss_antenna.start_time = -1;
    sprintf(local_leddiag.gnss_antenna.name, "GNSS Antenna");

    local_leddiag.cv2x_antenna.error_level = -1;
    local_leddiag.cv2x_antenna.start_time = -1;
    sprintf(local_leddiag.cv2x_antenna.name, "CV2X Antenna");
    
    local_leddiag.battery_voltage.error_level = -1;
    local_leddiag.battery_voltage.start_time = -1;
    sprintf(local_leddiag.battery_voltage.name, "Battery Voltage");
    
    local_leddiag.network.error_level = -1;
    local_leddiag.network.start_time = -1;
    sprintf(local_leddiag.network.name, "Network");

    local_leddiag.temperature.error_level = -1;
    local_leddiag.temperature.start_time = -1;
    sprintf(local_leddiag.temperature.name, "Temperature");
    
    local_leddiag.hsm.error_level = -1;
    local_leddiag.hsm.start_time = -1;
    sprintf(local_leddiag.hsm.name, "HSM");

    local_leddiag.cv2x_interface.error_level = -1;
    local_leddiag.cv2x_interface.start_time = -1;
    sprintf(local_leddiag.cv2x_interface.name, "CV2X Interface");

    local_leddiag.max_error.error_level = -1;
    local_leddiag.max_error.start_time = -1;
    memset(local_leddiag.max_error.name,0,sizeof(local_leddiag.max_error.name));

    local_leddiag.faultledstate = -1;
}

/* Set the error level for the given LED diagnostic depending on how long the error has been active */
void set_led_diagnostic_error(led_diagnostic_t *diagnostic, int uptime, int start_level, int escalate) 
{
    int prior_level = diagnostic->error_level;

    // initial error detection
    if (diagnostic->error_level <= LED_NO_ERR) {
        diagnostic->start_time = uptime;
        diagnostic->error_level = start_level;
    }
    // escalate based on timing
    if (escalate) {
        // first escalation after 10 minutes
        if (diagnostic->error_level == start_level && uptime - diagnostic->start_time > 10*60) { 
            diagnostic->error_level += 1;
        }
        // If there's another level left, escalate a second time after 30 minutes
        if (diagnostic->error_level < LED_ERR_3 && uptime - diagnostic->start_time > 30*60) { 
            diagnostic->error_level = LED_ERR_3;
        }
    }

    // Log Error
    if (diagnostic->error_level != prior_level) {
        //I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"Fault LED: detected %s error, level %d.\n", diagnostic->name,  diagnostic->error_level);
    }   
}

/* Set the error level to No Error for the given diagnostic. */
void clear_led_diagnostic_error(led_diagnostic_t *diagnostic, int uptime)
{
    if (diagnostic->error_level != LED_NO_ERR) {
        diagnostic->start_time = uptime;
        diagnostic->error_level = LED_NO_ERR;
        //I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"Fault LED: cleared %s error. \n", diagnostic->name);
    }
}

static void setleds(void)
{
    led_diagnostic_t max_error;
    struct timespec ts;
    int uptime;
    
    /* Get the current uptime */
    clock_gettime(CLOCK_MONOTONIC, &ts);
    uptime = ts.tv_sec;

    /* Initialize current Max Error with No Error*/
    max_error.error_level=0;
    max_error.start_time=uptime;
    sprintf(max_error.name, "None");
    
    /* RSU Health thread stopped */  
    if (RSUHEALTH_DIAG_THR_TO & localrsuhealth.errorstates){
        set_led_diagnostic_error(&local_leddiag.diag_thread, uptime, LED_ERR_2, 0); // level 2, don't escalate 
    } else {
        clear_led_diagnostic_error(&local_leddiag.diag_thread, uptime);
    } 
    max_error = local_leddiag.diag_thread.error_level > max_error.error_level? local_leddiag.diag_thread : max_error;
     
    /* Battery voltage out of range */ // TODO: Add warning vs critical ranges 
    if (RSUHEALTH_DCIN_FAIL & localrsuhealth.errorstates){
        set_led_diagnostic_error(&local_leddiag.battery_voltage, uptime, LED_ERR_2, 0); // level 2, don't escalate 
    } else {
        clear_led_diagnostic_error(&local_leddiag.battery_voltage, uptime);
    } 
    max_error = local_leddiag.battery_voltage.error_level > max_error.error_level? local_leddiag.battery_voltage : max_error;
    
    /* GNSS Antenna detection fail*/
    if (RSUHEALTH_GNSS_BAD_ANTENNA & localrsuhealth.gnssstatus){ 
        set_led_diagnostic_error(&local_leddiag.gnss_antenna, uptime, LED_ERR_2, 0); // level 2, don't escalate
    } else { 
        clear_led_diagnostic_error(&local_leddiag.gnss_antenna, uptime);
    }
    max_error = local_leddiag.gnss_antenna.error_level > max_error.error_level? local_leddiag.gnss_antenna : max_error;
    
    /* CV2X Antenna detection fail*/
   if (RSUHEALTH_CV2X_ANTENNA_CHECK & localrsuhealth.cv2xruntime 
       && (RSUHEALTH_CV2X1_BAD_ANTENNA & localrsuhealth.cv2xstatus  
          ||RSUHEALTH_CV2X2_BAD_ANTENNA & localrsuhealth.cv2xstatus)) {
        set_led_diagnostic_error(&local_leddiag.cv2x_antenna, uptime, LED_ERR_2, 0); // level 2, don't escalate        
    } else {
        clear_led_diagnostic_error(&local_leddiag.cv2x_antenna, uptime);
    }
    max_error = local_leddiag.cv2x_antenna.error_level > max_error.error_level? local_leddiag.cv2x_antenna : max_error;

    /* GPS fix not available */
    if (uptime > 2*60 && !local_leddiag.gnss_antenna.error_level) { // preconditions: system up 2 minutes, GNSS ant connected
        if (0x0 == (RSUHEALTH_GNSS_FIX_VALID & localrsuhealth.gnssstatus)) {
            set_led_diagnostic_error(&local_leddiag.gps_fix, uptime, LED_ERR_2, 0); // level 2, don't escalate
        } else {
            clear_led_diagnostic_error(&local_leddiag.gps_fix, uptime);
        }
        max_error = local_leddiag.gps_fix.error_level > max_error.error_level? local_leddiag.gps_fix : max_error;
    } else if (-1 !=local_leddiag.gps_fix.error_level) { //preconditions not met but error level has been set
        clear_led_diagnostic_error(&local_leddiag.gps_fix, uptime); 
    }

    /* Time accuracy exceeds minimum */
    if (!local_leddiag.gps_fix.error_level) { // precondition: gps fix is available 
        if (RSUHEALTH_GNSS_NO_TIME_ACCURACY & localrsuhealth.gnssstatus
           || RSUHEALTH_GNSS_BAD_TIME_ACCURACY & localrsuhealth.gnssstatus) {
            set_led_diagnostic_error(&local_leddiag.time_accuracy, uptime, LED_ERR_2, 0); // level 2, don't escalate
        } else {
            clear_led_diagnostic_error(&local_leddiag.time_accuracy, uptime);
        }
        max_error= local_leddiag.time_accuracy.error_level > max_error.error_level? local_leddiag.time_accuracy : max_error;
    } else if (-1 !=local_leddiag.time_accuracy.error_level) { //preconditions not met but error level has been set
        clear_led_diagnostic_error(&local_leddiag.time_accuracy, uptime); 
    }
    
    /* GPS position is not the stored location*/
    if (!local_leddiag.gps_fix.error_level) { // precondition: gps fix is available
        if (RSUHEALTH_GNSS_BAD_LOC_DEV & localrsuhealth.gnssstatus){
            set_led_diagnostic_error(&local_leddiag.gps_position, uptime, LED_ERR_2, 0); // level 2, don't escalate
        } else {
            clear_led_diagnostic_error(&local_leddiag.gps_position, uptime);
        }
        max_error = local_leddiag.gps_position.error_level > max_error.error_level? local_leddiag.gps_position : max_error;
    } else if (-1 !=local_leddiag.gps_position.error_level) { //preconditions not met but error level has been set
        clear_led_diagnostic_error(&local_leddiag.gps_position, uptime); 
    }
    

    /* network failure */
    if (RSUHEALTH_NETWORK_INIT_FAIL & localrsuhealth.errorstates
        || RSUHEALTH_NETWORK_LINK_FAIL  & localrsuhealth.errorstates) {
        set_led_diagnostic_error(&local_leddiag.network, uptime, LED_ERR_2, 0); // level 2, don't escalate 
    } else {
        clear_led_diagnostic_error(&local_leddiag.network, uptime);
    }
    max_error = local_leddiag.network.error_level > max_error.error_level? local_leddiag.network : max_error;

    /* temperature error  */
    if (RSUHEALTH_CPU_TEMP_FAIL & localrsuhealth.errorstates
        || RSUHEALTH_AMBIENT_TEMP_FAIL  & localrsuhealth.errorstates
        || RSUHEALTH_HSM_TEMP_FAIL & localrsuhealth.cv2xstatus
        || RSUHEALTH_CV2X_TEMP_FAIL & localrsuhealth.cv2xstatus) {
        set_led_diagnostic_error(&local_leddiag.temperature, uptime, LED_ERR_2, 0); // level 2, don't escalate 
    } else {
        clear_led_diagnostic_error(&local_leddiag.temperature, uptime);
    }
    max_error = local_leddiag.temperature.error_level > max_error.error_level? local_leddiag.temperature : max_error;
    
    /* hsm error */
    if (RSUHEALTH_HSM_BAD_CFG_MASK & localrsuhealth.cv2xstatus) {
        set_led_diagnostic_error(&local_leddiag.hsm, uptime, LED_ERR_2, 0); // level 2, don't escalate
    } else {
        clear_led_diagnostic_error(&local_leddiag.hsm, uptime);
    }
    max_error = local_leddiag.hsm.error_level > max_error.error_level? local_leddiag.hsm : max_error;
    
    /* cv2x interface error */
    if (RSUHEALTH_CV2X_BAD_INTERFACE & localrsuhealth.cv2xstatus) {
        set_led_diagnostic_error(&local_leddiag.cv2x_interface, uptime, LED_ERR_2, 0); // level 2, don't escalate
    } else {
        clear_led_diagnostic_error(&local_leddiag.cv2x_interface, uptime);
    }
    max_error = local_leddiag.cv2x_interface.error_level > max_error.error_level? local_leddiag.cv2x_interface : max_error;
 
    /* If the current maximum error has changed (different test case or different error level) update the local_leddiag.max_error  */
    if (strcmp(max_error.name, local_leddiag.max_error.name)               // new test case
        || max_error.error_level != local_leddiag.max_error.error_level){  // new error level 
        local_leddiag.max_error = max_error;
    }
   
   /* Update the LED once the max_error has been valid for 10s and log it*/
    if (local_leddiag.faultledstate != local_leddiag.max_error.error_level
        && local_leddiag.max_error.start_time + 10 <= uptime) {
            
        switch(local_leddiag.max_error.error_level) {
            case LED_NO_ERR: // no error: solid green
                system("rsuhealth -lps");
                break;
            case LED_ERR_1: // recoverable error, mild severity: Solid Red LED
                system("rsuhealth -lfs"); 
                break;
            case LED_ERR_2: // recoverable error: blink red low
                system("rsuhealth -lfb");
                break;
            case LED_ERR_3: // irrecoverable error: blink red fast
                system("rsuhealth -lff");
                break;
        }

        local_leddiag.faultledstate = local_leddiag.max_error.error_level; //TODO add a check that setting the LED worked? how
    }    
}

void *SetLedsThrd(void *ptr)
{
    int32_t i;
    
    ptr = ptr;

    init_local_leddiag(); /* Initialize the local led diagnostics */
    openledshm(); //open LED SHM

    for(i=0; i < (2 * RSUHEALTH_BOOTUP_WAIT); i++) { /* More than enough time. */
        sleep(1);
        if ((WTRUE == localrsuhealth.h.ch_data_valid) && (0 < localrsuhealth.h.ch_sequence_number)) {
            break;
        }
    }
    if((2 * RSUHEALTH_BOOTUP_WAIT) <= i) {
        I2V_DBG_LOG(LEVEL_WARN, "led","Failed to get handshake from RSUDIAG.\n");
    }
    while (loop) {
        setleds();
        if(!loop) break;
        updateledshm();
        if(!loop) break;
        sleep(1);
    }
#if defined(EXTRA_DEBUG)
    I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"LED Thread Exiting ...\n");
#endif
    closeledshm();
    pthread_exit(0);
}

void logFaultLED(void)
{
    static int last_faultledstate = 0;

    if (local_leddiag.faultledstate != last_faultledstate) {
        if (local_leddiag.faultledstate == 0) {
            I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"Fault LED cleared.\n");
        } else {
            I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"Fault LED set.\n");
        }
    }
    last_faultledstate = local_leddiag.faultledstate;
}
/*----------------------------------------------------------------------------*/
/* Dump to /tmp for user to cat. Only understandle+actionable items for user. */
/*----------------------------------------------------------------------------*/
static void dump_rsudiag_report(void)
{
  FILE * file_out = NULL;
  if ((file_out = fopen("/tmp/rsudiag.txt", "w")) == NULL){
      if(0x0 == (localrsuhealth.errorstates & RSUHEALTH_DUMP_REPORT_FAIL)) {
          localrsuhealth.errorstates |= RSUHEALTH_DUMP_REPORT_FAIL;
          I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"RSUHEALTH_DUMP_REPORT_FAIL\n");
      }
  } else {
      #if 0
      if(localrsuhealth.gnssstatus & RSUHEALTH_GNSS_FIX_VALID) {
          fprintf(file_out,"RSUHEALTH_GNSS_FIX_VALID\n");
      }
      #endif
      if(localrsuhealth.gnssstatus & RSUHEALTH_GNSS_BAD_ANTENNA) {
          fprintf(file_out,"RSUHEALTH_GNSS_BAD_ANTENNA: RSU can not detect GNSS antenna.\n");
      }
      if(localrsuhealth.gnssstatus & RSUHEALTH_GNSS_BAD_LOC_DEV) {
          fprintf(file_out,"RSUHEALTH_GNSS_BAD_LOC_DEV: RSU location has gone beyond rsuGnssMaxDeviation.\n");
      }
      if(localrsuhealth.gnssstatus & RSUHEALTH_GNSS_BAD_TTFF) {
          fprintf(file_out,"RSUHEALTH_GNSS_BAD_TTFF: Time to first fix was longer than what we would hope for. Is antenna in open sky and is antenna attached?\n");
      }
      if(localrsuhealth.gnssstatus & RSUHEALTH_GNSS_BAD_FW) {
          fprintf(file_out,"RSUHEALTH_GNSS_BAD_FW: GNSS has wrong FW revision. If GNSS have valid fix then ignore event.\n");
      }
      if(localrsuhealth.gnssstatus & RSUHEALTH_GNSS_BAD_CFG) {
          fprintf(file_out,"RSUHEALTH_GNSS_BAD_CFG: GNSS has wrong config revision. If GNSS have valid fix then ignore event.\n");
      }
      #if 0
      if(localrsuhealth.gnssstatus & RSUHEALTH_1PPS_BAD_FETCH) {
          fprintf(file_out,"RSUHEALTH_1PPS_BAD_FETCH\n");
      }
      if(localrsuhealth.gnssstatus & RSUHEALTH_1PPS_BAD_LATENCY) {
          fprintf(file_out,"RSUHEALTH_1PPS_BAD_LATENCY\n");
      }
      if(localrsuhealth.gnssstatus & RSUHEALTH_1PPS_TPS_BAD_LATENCY) {
          fprintf(file_out,"RSUHEALTH_1PPS_TPS_BAD_LATENCY\n");
      }
      if(localrsuhealth.gnssstatus & RSUHEALTH_1PPS_BAD_TIMESTAMP) {
          fprintf(file_out,"RSUHEALTH_1PPS_BAD_TIMESTAMP\n");
      }
      if(localrsuhealth.gnssstatus & RSUHEALTH_1PPS_BAD_MKTIME) {
          fprintf(file_out,"RSUHEALTH_1PPS_BAD_MKTIME\n");
      }
      #endif
      if(localrsuhealth.gnssstatus & RSUHEALTH_1PPS_HALTED) {
          fprintf(file_out,"RSUHEALTH_1PPS_HALTED: GNSS 1 PPS has halted. RSU will not transmit.\n");
      }
      #if 0
      if(localrsuhealth.gnssstatus & RSUHEALTH_1PPS_INIT_FAIL) {
          fprintf(file_out,"RSUHEALTH_1PPS_INIT_FAIL\n");
      }
      if(localrsuhealth.gnssstatus & RSUHEALTH_GNSS_TPS_SEQNO_STUCK) {
          fprintf(file_out,"RSUHEALTH_GNSS_TPS_SEQNO_STUCK\n");
      }
      #endif
      if(localrsuhealth.gnssstatus & RSUHEALTH_GNSS_PVT_STUCK) {
          fprintf(file_out,"RSUHEALTH_GNSS_PVT_STUCK: GNSS chip has stopped sending messages to RSU. RSU will not transmit.\n");
      }
      #if 0
      if(localrsuhealth.gnssstatus & RSUHEALTH_GNSS_PVT_UNUSED) {
          fprintf(file_out,"RSUHEALTH_GNSS_PVT_UNUSED\n");
      }
      #endif
      if(localrsuhealth.gnssstatus & RSUHEALTH_GNSS_GGA_STUCK) {
          fprintf(file_out,"RSUHEALTH_GNSS_GGA_STUCK: GNSS has stopped sending valid NMEA GGA messages. Not fatal but rsuGnssOutputString will be bogus.\n");
      }
      #if 0
      if(localrsuhealth.gnssstatus & RSUHEALTH_GNSS_NO_TIME_ACCURACY) {
          fprintf(file_out,"RSUHEALTH_GNSS_NO_TIME_ACCURACY\n");
      }
      #endif
      if(localrsuhealth.gnssstatus & RSUHEALTH_GNSS_BAD_TIME_ACCURACY) {
          fprintf(file_out,"RSUHEALTH_GNSS_BAD_TIME_ACCURACY: GNSS time accuracy outside operational range. RSU will not transmit.\n");
      }
      #if 0
      if(localrsuhealth.gnssstatus & RSUHEALTH_GNSS_RTC_BAD_CAL) {
          fprintf(file_out,"RSUHEALTH_GNSS_RTC_BAD_CAL\n");
      }
      #endif
      /*----------------------------------------------------------------------------*/
      /*----------------------------------------------------------------------------*/
      #if 0
      if(localrsuhealth.cv2xstatus & RSUHEALTH_CV2X_TX_READY) {
          fprintf(file_out,"RSUHEALTH_CV2X_TX_READY\n");
      }
      if(localrsuhealth.cv2xstatus & RSUHEALTH_CV2X_RX_READY) {
          fprintf(file_out,"RSUHEALTH_CV2X_RX_READY\n");
      }
      #endif
      if(localrsuhealth.cv2xstatus & RSUHEALTH_CV2X1_BAD_ANTENNA) {
          fprintf(file_out,"RSUHEALTH_CV2X1_BAD_ANTENNA\n");
      }
      if(localrsuhealth.cv2xstatus & RSUHEALTH_CV2X2_BAD_ANTENNA) {
          fprintf(file_out,"RSUHEALTH_CV2X2_BAD_ANTENNA\n");
      }
      if(localrsuhealth.cv2xstatus & RSUHEALTH_CV2X_TEMP_FAIL) {
          fprintf(file_out,"RSUHEALTH_CV2X_TEMP_FAIL\n");
      }
      if(localrsuhealth.cv2xstatus & RSUHEALTH_CV2X_BAD_FW_REVISION) {
          fprintf(file_out,"RSUHEALTH_CV2X_BAD_FW_REVISION\n");
      }
      if(localrsuhealth.cv2xstatus & RSUHEALTH_CV2X_BAD_INTERFACE) {
          fprintf(file_out,"RSUHEALTH_CV2X_BAD_INTERFACE\n");
      }
      if(localrsuhealth.cv2xstatus & RSUHEALTH_CV2X_INIT_FAIL) {
          fprintf(file_out,"RSUHEALTH_CV2X_INIT_FAIL\n");
      }
      if(localrsuhealth.cv2xstatus & RSUHEALTH_HSM_TEMP_FAIL) {
          fprintf(file_out,"HSM_BAD_TEMPERATURE\n");
      }
      if(localrsuhealth.cv2xstatus & RSUHEALTH_HSM_BAD_CFG) {
          fprintf(file_out,"RSUHEALTH_HSM_BAD_CFG\n");
      }
      if(localrsuhealth.cv2xstatus & RSUHEALTH_HSM_BAD_JCOP) {
          fprintf(file_out,"RSUHEALTH_HSM_BAD_JCOP\n");
      }
      if(localrsuhealth.cv2xstatus & RSUHEALTH_HSM_BAD_USAPP) {
          fprintf(file_out,"RSUHEALTH_HSM_BAD_USAPP\n");
      }
      if(localrsuhealth.cv2xstatus & RSUHEALTH_HSM_BAD_GSA) {
          fprintf(file_out,"RSUHEALTH_HSM_BAD_GSA\n");
      }
      if(localrsuhealth.cv2xstatus & RSUHEALTH_AEROLINK_DIR_FAIL) {
          fprintf(file_out,"RSUHEALTH_AEROLINK_DIR_FAIL: %s folder does not exist.\n",RSU_AEROLINK_CERTS_LOCATION);
      }
      /*----------------------------------------------------------------------------*/
      /*----------------------------------------------------------------------------*/
      if(rskstatus.error_states & CV2X_INIT_FAIL) {
          fprintf(file_out,"CV2X_INIT_FAIL: CV2X failed to init. RSU will not transmot or receive.\n");
      }
      if(rskstatus.error_states & CV2X_TX_NOT_READY) {
          fprintf(file_out,"CV2X_TX_NOT_READY: CV2X transmit is not ready.\n");
      }
      if(rskstatus.error_states & CV2X_TX_FAIL) {
          fprintf(file_out,"CV2X_TX_FAIL: CV2X failed to transmit some messages. This may happen from time to time. See if problem persists.\n");
      }
      if(rskstatus.error_states & CV2X_RX_FAIL) {
          fprintf(file_out,"CV2X_RX_FAIL: CV2X failed to receive some messages. This may happen from time to time. See if problem persists.\n");
      }
      #if 0
      if(rskstatus.error_states & CV2X_SERVICE_ID_BAD) {
          fprintf(file_out,"CV2X_SERVICE_ID_BAD\n");
      }
      if(rskstatus.error_states & CV2X_SOCKET_INIT_FAIL) {
          fprintf(file_out,"CV2X_SOCKET_INIT_FAIL\n");
      }
      if(rskstatus.error_states & CV2X_SYS_INIT_FAIL) {
          fprintf(file_out,"CV2X_SYS_INIT_FAIL\n");
      }
      if(rskstatus.error_states & CV2X_FW_REVISION_FAIL) {
          fprintf(file_out,"CV2X_FW_REVISION_FAIL\n");
      }
      if(rskstatus.error_states & CV2X_INIT_TWICE_WARNING) {
          fprintf(file_out,"CV2X_INIT_TWICE_WARNING\n");
      }
      if(rskstatus.error_states & CV2X_WDM_SERVICE_GET_FAIL) {
          fprintf(file_out,"CV2X_WDM_SERVICE_GET_FAIL\n");
      }
      if(rskstatus.error_states & CV2X_DDM_SERVICE_GET_FAIL) {
          fprintf(file_out,"CV2X_DDM_SERVICE_GET_FAIL\n");
      }
      if(rskstatus.error_states & CV2X_DDM_STATE_GET_FAIL) {
          fprintf(file_out,"CV2X_DDM_STATE_GET_FAIL\n");
      }
      if(rskstatus.error_states & CV2X_DDM_SYNC_INIT_FAIL) {
          fprintf(file_out,"CV2X_DDM_SYNC_INIT_FAIL\n");
      }
      if(rskstatus.error_states & CV2X_GET_SERVICE_FAIL) {
          fprintf(file_out,"CV2X_GET_SERVICE_FAIL\n");
      }
      if(rskstatus.error_states & CV2X_CONFIG_SET_FAIL) {
          fprintf(file_out,"CV2X_CONFIG_SET_FAIL\n");
      }
      if(rskstatus.error_states & CV2X_SERVICE_ENABLE_FAIL) {
          fprintf(file_out,"CV2X_SERVICE_ENABLE_FAIL\n");
      }
      if(rskstatus.error_states & CV2X_L2ID_SET_FAIL) {
          fprintf(file_out,"CV2X_L2ID_SET_FAIL\n");
      }
      if(rskstatus.error_states & CV2X_SOCKET_CREATE_FAIL) {
          fprintf(file_out,"CV2X_SOCKET_CREATE_FAIL\n");
      }
      if(rskstatus.error_states & CV2X_TX_DUPE_SERVICE_ID_FAIL) {
          fprintf(file_out,"CV2X_TX_DUPE_SERVICE_ID_FAIL\n");
      }
      if(rskstatus.error_states & CV2X_TX_DDM_SERVICE_GET_FAIL) {
          fprintf(file_out,"CV2X_TX_DDM_SERVICE_GET_FAIL\n");
      }
      if(rskstatus.error_states & CV2X_TX_SOCKET_CREATE_FAIL) {
          fprintf(file_out,"CV2X_TX_SOCKET_CREATE_FAIL\n");
      }
      if(rskstatus.error_states & CV2X_TX_SOCKET_POLICY_FAIL) {
          fprintf(file_out,"CV2X_TX_SOCKET_POLICY_FAIL\n");
      }
      if(rskstatus.error_states & CV2X_SPS_FLOWS_USED_FAIL) {
          fprintf(file_out,"CV2X_SPS_FLOWS_USED_FAIL\n");
      }
      if(rskstatus.error_states & CV2X_TX_RADIO_NOT_READY_FAIL) {
          fprintf(file_out,"CV2X_TX_RADIO_NOT_READY_FAIL\n");
      }
      if(rskstatus.error_states & CV2X_DDM_TSF_GET_FAIL) {
          fprintf(file_out,"CV2X_DDM_TSF_GET_FAIL\n");
      }
      if(rskstatus.error_states & CV2X_GET_TALLIES_FAIL) {
          fprintf(file_out,"CV2X_GET_TALLIES_FAIL\n");
      }
      if(rskstatus.error_states & CV2X_PCAP_WRITE_SEM_WAIT_FAIL) {
          fprintf(file_out,"CV2X_PCAP_WRITE_SEM_WAIT_FAIL\n");
      }
      if(rskstatus.error_states & CV2X_PCAP_READ_SEM_WAIT_FAIL) {
          fprintf(file_out,"CV2X_PCAP_READ_SEM_WAIT_FAIL\n");
      }
      #endif
      /*----------------------------------------------------------------------------*/
      /*----------------------------------------------------------------------------*/
      #if 0
      if(localrsuhealth.errorstates & RSUHEALTH_SSCANF_FAIL) {
          fprintf(file_out,"RSUHEALTH_SSCANF_FAIL\n");
      }
      if(localrsuhealth.errorstates & RSUHEALTH_FGETS_FAIL) {
          fprintf(file_out,"RSUHEALTH_FGETS_FAIL\n");
      }
      if(localrsuhealth.errorstates & RSUHEALTH_POPEN_FAIL) {
          fprintf(file_out,"RSUHEALTH_POPEN_FAIL\n");
      }
      #endif
      if(localrsuhealth.errorstates & RSUHEALTH_STALE_PVT) {
          fprintf(file_out,"RSUHEALTH_STALE_PVT: GNSS Chip has stopped sending messages to RSU.\n");
      }
      #if 0
      if(localrsuhealth.errorstates & RSUHEALTH_BOGUS_PVT_INDEX) {
          fprintf(file_out,"RSUHEALTH_BOGUS_PVT_INDEX\n");
      }
      #endif
      if(localrsuhealth.errorstates & RSUHEALTH_STALE_SOUS) {
          fprintf(file_out,"RSUHEALTH_STALE_SOUS: RH850 has stopped sending messages to RSU.\n");
      }
      #if 0
      if(localrsuhealth.errorstates & RSUHEALTH_TPS_SHM_FAIL) {
          fprintf(file_out,"RSUHEALTH_TPS_SHM_FAIL\n");
      }
      if(localrsuhealth.errorstates & RSUHEALTH_SOUS_SHM_FAIL) {
          fprintf(file_out,"RSUHEALTH_SOUS_SHM_FAIL\n");
      }
      if(localrsuhealth.errorstates & RSUHEALTH_SHM_FAIL) {
          fprintf(file_out,"RSUHEALTH_SHM_FAIL\n");
      }
      if(localrsuhealth.errorstates & RSUHEALTH_GUI_SCRIPT_FAIL) {
          fprintf(file_out,"RSUHEALTH_GUI_SCRIPT_FAIL\n");
      }
      if(localrsuhealth.errorstates & RSUHEALTH_READY_SET_FAIL) {
          fprintf(file_out,"RSUHEALTH_READY_SET_FAIL\n");
      }
      #endif
      if(localrsuhealth.errorstates & RSUHEALTH_I2V_RESET) {
          fprintf(file_out,"RSUHEALTH_I2V_RESET: RSUHEALTH had to restart I2V.\n");
      }
      #if 0
      if(localrsuhealth.errorstates & RSUHEALTH_BOGUS_LED_STATE) {
          fprintf(file_out,"RSUHEALTH_BOGUS_LED_STATE\n");
      }
      if(localrsuhealth.errorstates & RSUHEALTH_READY_FILE_FAIL) {
          fprintf(file_out,"RSUHEALTH_READY_FILE_FAIL\n");
      }
      if(localrsuhealth.errorstates & RSUHEALTH_SYSSETTS_SHM_FAIL) {
          fprintf(file_out,"RSUHEALTH_SYSSETTS_SHM_FAIL\n");
      }
      if(localrsuhealth.errorstates & RSUHEALTH_SHM_SEMA_FAIL) {
          fprintf(file_out,"RSUHEALTH_SHM_SEMA_FAIL\n");
      }
      #endif
      if(localrsuhealth.errorstates & RSUHEALTH_DCIN_FAIL) {
          fprintf(file_out,"RSUHEALTH_DCIN_FAIL: Voltage to RSU is outside operational levels.\n");
      }
      if(localrsuhealth.errorstates & RSUHEALTH_CPU_TEMP_FAIL) {
          fprintf(file_out,"RSUHEALTH_CPU_TEMP_FAIL: Temperature for iMX8 is outside operational levels.\n");
      }
      if(localrsuhealth.errorstates & RSUHEALTH_AMBIENT_TEMP_FAIL) {
          fprintf(file_out,"RSUHEALTH_AMBIENT_TEMP_FAIL: Temperature inside RSu enclosure is outside operational levels.\n");
      }
      if(localrsuhealth.errorstates & RSUHEALTH_NETWORK_LINK_FAIL) {
          fprintf(file_out,"RSUHEALTH_NETWORK_LINK_FAIL: Network connection to RSU has failed.\n");
      }
      if(localrsuhealth.errorstates & RSUHEALTH_NETWORK_INIT_FAIL) {
          fprintf(file_out,"RSUHEALTH_NETWORK_INIT_FAIL: Network connection to RSU failed to initalize.\n");
      }
      if(localrsuhealth.errorstates & RSUHEALTH_IP_GET_FAIL) {
          fprintf(file_out,"RSUHEALTH_IP_GET_FAIL: Network could not obtain IP specified.\n");
      }
      if(localrsuhealth.errorstates & RSUHEALTH_IP_RX_GET_FAIL) {
          fprintf(file_out,"RSUHEALTH_IP_RX_GET_FAIL: Receive errors on Network connection.\n");
      }
      if(localrsuhealth.errorstates & RSUHEALTH_IP_TX_GET_FAIL) {
          fprintf(file_out,"RSUHEALTH_IP_TX_GET_FAIL: Transmit errors on Network connection.\n");
      }
      #if 0
      if(localrsuhealth.errorstates & RSUHEALTH_SYS_CALL_FAIL) {
          fprintf(file_out,"RSUHEALTH_SYS_CALL_FAIL\n");
      }
      if(localrsuhealth.errorstates & RSUHEALTH_IP_INFO_FAIL) {
          fprintf(file_out,"RSUHEALTH_IP_INFO_FAIL\n");
      }
      if(localrsuhealth.errorstates & RSUHEALTH_NETMASK_GET_FAIL) {
          fprintf(file_out,"RSUHEALTH_NETMASK_GET_FAIL\n");
      }
      #endif
      if(localrsuhealth.errorstates & RSUHEALTH_SEC_CERT_FAIL) {
          fprintf(file_out,"RSUHEALTH_SEC_CERT_FAIL: No valid certifcates for RSU found.\n");
      }
      if(localrsuhealth.errorstates & RSUHEALTH_SEC_CERT_EXPIRED) {
          fprintf(file_out,"RSUHEALTH_SEC_CERT_EXPIRED: Certifcates for RSU have expired.\n");
      }
      if(localrsuhealth.errorstates & RSUHEALTH_SEC_CERT_NEAR_EXP) {
          fprintf(file_out,"RSUHEALTH_SEC_CERT_NEAR_EXP: Certifcates for RSU will soon expire.\n");
      }
      if(localrsuhealth.errorstates & RSUHEALTH_SEC_CERT_BOGUS) {
          fprintf(file_out,"RSUHEALTH_SEC_CERT_BOGUS: There are expired or invalid certificates on RSU.\n");
      }
      if(localrsuhealth.errorstates & RSUHEALTH_SEC_CERT_FULL) {
          fprintf(file_out,"RSUHEALTH_SEC_CERT_FULL: Certificates are now full on RSU.\n");
      }
      if(localrsuhealth.errorstates & RSUHEALTH_STANDBY_MODE) {
          fprintf(file_out,"RSUHEALTH_STANDBY_MODE: RSU is in standby mode and will not tx. See other errors.\n");
      }
      if(localrsuhealth.errorstates & RSUHEALTH_FAULT_MODE) {
          fprintf(file_out,"RSUHEALTH_FAULT_MODE: RSU is in standby mode and will not transmit. See other errors. Recommend RSU be power cycled to recover.\n");
      }
      #if 0
      if(localrsuhealth.errorstates & RSUHEALTH_NSSTATS_FAILED) {
          fprintf(file_out,"RSUHEALTH_NSSTATS_FAILED\n");
      }
      if(localrsuhealth.errorstates & RSUHEALTH_MANAGE_FOLDER_OFF) {
          fprintf(file_out,"RSUHEALTH_MANAGE_FOLDER_OFF\n");
      }
      if(localrsuhealth.errorstates & RSUHEALTH_SYSCALL_FAIL) {
          fprintf(file_out,"RSUHEALTH_SYSCALL_FAIL\n");
      }
      #endif
      fflush(file_out);
      fclose(file_out);
      file_out = NULL;
      if((localrsuhealth.errorstates & RSUHEALTH_DUMP_REPORT_FAIL)) {
          localrsuhealth.errorstates &= ~(uint64_t)(RSUHEALTH_DUMP_REPORT_FAIL);
      }
  }
}
//
//TODO: In theory I2V should not launch until RSUHEALTH declares all systems go.
//    : rsuMode standy support needed. This would address by holding I2V off.
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
void *OneSecThrd(void *ptr)
{
  struct timespec t;
  int32_t syslog_open = 0;
  ptr=ptr;
  t.tv_sec = 1; /* set once, always 1 second */

  opendiagnosticshm(); /* open SHM items needed */

  seedstatisticalmemory(); /* setup statistical aspects of memory */

  sleep(RSUHEALTH_BOOTUP_WAIT); /* start too soon and just a bunch of weird fails of no interest while smashing devices trying to get ready, */

  while (loop) {
      /* Assume logmgr up and ready after boot up wait. */
      if((0 == syslog_open) && (1 == enableDebug)){ /* assume LOGMGR is ready */
          if(0 == i2vUtilEnableSyslog(MY_ERR_LEVEL, MY_NAME)){
              syslog_open = 1;
          } else {
              if(onesecondcounter < 30) {
                  printf("%s: syslog enable fail(%d)\n",MY_NAME,onesecondcounter);
              }
              i2vUtilDisableSyslog();
          }
      }

      if(!loop) break;

      dumphealthoutput(); /* periodically output disagnostics to event log */

      if(0 != nanosleep(&t, NULL)) { /* run diagnostics at 1 HZ */
          I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"OneSecThrd: nanosleep failed: errno(%d)(%s)\n", errno,strerror(errno));
      }
      onesecondcounter++;

      if(!loop) break;

      /*
       * order in list of urgency. Temperatures and voltage highest
       */
      checkenviornment(); /* cpu & ambient temperature, dcin voltage, tamper detection */

      if(!loop) break;

      checkcv2x();        /* radio */
#if 0 /* API is disruptive. Disable for now */
      checkhsm();         /* hsm */
#endif
      if(!loop) break;

      checkgnss();        /* gnss */

      if(!loop) break;

      checksystem();      /* cpu load, ram use, watchdog, nvm use, storage integrity */

      if(!loop) break;

      if(0 == (onesecondcounter % 10)){
          if(localrsuhealth.gnssstatus & RSUHEALTH_GNSS_FIX_VALID){
              checksecurity(); /* security credentials: only check when we have valid time. */
          }
      }

      if(!loop) break;

      if((60 < onesecondcounter) && (1 == (onesecondcounter % 10))) { /* Once every 10 seconds is good enough. */
          checknetwork();     /* network init and link */
      }

      if(!loop) break;

      if(60 == (onesecondcounter % 1200)) { /* Once every 20 mins is good enough. */
          dumpalsmistats();
      }
      if(!loop) break;
      /* 
       * if new event occured dump to logs
       */
      if(60 < onesecondcounter) { /* First minute is stream of false warnings. */
          dumpchangedevents();
      }

      if(!loop) break;

      if((60 < onesecondcounter) && (0 == (onesecondcounter % 60))) {
          dump_rsudiag_report();
      }

      if(!loop) break;

      logFaultLED();

      if(!loop) break;

      /* 
       * if fault detected and recovery enabled in MIB then signal recovery
       */
      managerecovery();

      if(!loop) break;

      /* Customer notifcation for folder full or issues processing folder */
      checkcustomernotifications();

      if(!loop) break;

      /* 
       * calculate statistics here at the bottom of the loop before posting to SHM
       */
      calculatestatistics();

      if(!loop) break;

      /*
       * update RSUHEALTH SHM now this cycle diagnostics is done
       */
      updatediagnosticshm();

      if(!loop) break;

      clearevents();
      #if defined(MY_UNIT_TEST)
      if(CHILD_TEST_ITERATION_MAX < onesecondcounter) {
          loop = 0; /* signal main time to end test */
      }
      setnexttestiteration(onesecondcounter);
      #endif
  } /* while mainloop */
#if defined(EXTRA_DEBUG)
  I2V_DBG_LOG(LEVEL_PRIV, MY_NAME,"OneSecThrd Exiting: (0x%lx) gnss(0x%x) tps(0x%x) cv2x(0x%x,%u) rsk(0x%x)\n",
      localrsuhealth.errorstates,localrsuhealth.gnssstatus,localshmtps.error_states,localrsuhealth.cv2xstatus,localrsuhealth.cv2xruntime,rskstatus.error_states);
  dumphealthoutput();   /* One last dump. Likely never make it but try. */
#endif
  closediagnosticshm(); /* close SHM used */

  pthread_exit(0);
}
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
void *cleancustomerfolderThrd(void *ptr)
{
  uint32_t iteration_counter = 0;

  ptr = ptr;

  while (loop) {
      sleep(1);
      if(60 == (iteration_counter % CHECK_CUSTOMER_FOLDER_RATE)) {
          checkcustomerfolder();
      }
      iteration_counter++;
  }
  pthread_exit(0);
}
