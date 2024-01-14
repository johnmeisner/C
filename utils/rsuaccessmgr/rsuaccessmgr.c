/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Filename: rsuaccessmgr.c                                                   */
/* Purpose: RSU Access Manager                                                */
/*                                                                            */
/* Copyright (C) 2023 DENSO International America, Inc.  All Rights Reserved. */
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
#define MAIN rsuaccessmgr_main /* rename main() so we dont conflict with unit test main */
#else
#define MAIN main
#endif

//#define EXTRA_DEBUG /* Disable before check in. Use for any program debug you want.  */

/* Mandatory logging defines. */
#if defined(MY_UNIT_TEST)
#define MY_ERR_LEVEL   LEVEL_DBG  /* SEL and serial */
#else
#define MY_ERR_LEVEL   LEVEL_PRIV /* SEL only, no serial. */
#endif
#define MY_NAME        "accmgr" /* This will be entry name in SEL */

#define OUTPUT_MODULUS  1200    /* Interval is seconds. to limit serial and SEL output. */
#if defined(MY_UNIT_TEST)
#undef OUTPUT_MODULUS
#define OUTPUT_MODULUS 1
#endif


/* 32 bit error state. If more need, create another register for those bits. */
#define RSUACCMGR_SHM_INIT_FAIL        0x00000001
#define RSUACCMGR_BAD_INPUT            0x00000002
#define RSUACCMGR_MSG_TRUNCATE         0x00000004
#define RSUACCMGR_SHM_SEMA_UNLOCK_FAIL 0x00000008
#define RSUACCMGR_SHM_SEMA_LOCK_FAIL   0x00000010

#define CLIENT_LOGIN_FILE      "/rwflash/customer/client5.txt"   
#define DEBUG_FILE "/tmp/xcgi-login-debug.txt"  

#define WEBGUI_AUTH_FAIL "Authentication Failure" 
#define SSH_AUTH_FAIL "authentication failure"

/* Number of times to try to lock RSUHEALTH SHM then give up. */
#define MAX_TRY_LOCK 10

#define MAX_IPV6_STRING_LENGTH  48 

/*
 * Globals.
 */
STATIC int32_t  mainloop = 1;
STATIC uint32_t errorstates = 0x0;
STATIC uint32_t prior_errorstates = 0x0;
STATIC rsuhealth_t * shm_rsuhealth_ptr = NULL; /* RSUHEALTH SHM */

/*
 * Function bodies.
 */
STATIC void init_statics(void)
{
  mainloop = 1;
  errorstates = 0x0;
  prior_errorstates = 0x0;
  shm_rsuhealth_ptr = NULL;
}
STATIC void sighandler(void (*handler)(int, siginfo_t *, void*), int sig)
{
    struct sigaction act;

    memset(&act, 0, sizeof(act));
    act.sa_sigaction = handler;
    sigemptyset(&act.sa_mask);
    act.sa_flags = SA_SIGINFO|SA_RESTART;
    sigaction(sig, &act, NULL);
}
STATIC void termhandler(int a, siginfo_t *b, void __attribute__((unused)) *c)
{
  mainloop = 0;
}
STATIC void opendiagnosticshm(void)
{
  /* Open RSUHEALTH SHM, if fails not fatal. MIB and IWMH will suffer. */
  if ((shm_rsuhealth_ptr = wsu_share_init(sizeof(rsuhealth_t), RSUHEALTH_SHM_PATH)) == NULL) {
      I2V_DBG_LOG(LEVEL_CRIT, MY_NAME,"RSUACCMGR_SHM_INIT_FAIL: RSUHEALTH SHM init failed.\n");
      shm_rsuhealth_ptr = NULL; /* Keep going, not fatal, just no traps. */
      errorstates |= RSUACCMGR_SHM_INIT_FAIL;
  }
}
STATIC void closediagnosticshm(void)
{
  /* Close SHM. Don't destroy. Recover on start up.*/
  if(NULL != shm_rsuhealth_ptr) {
      wsu_share_kill(shm_rsuhealth_ptr, sizeof(rsuhealth_t));
      shm_rsuhealth_ptr = NULL;
  }
}
STATIC void sendtrap(char_t * alertMsg, rsuAlertLevel_e alertLevel)
{
  int32_t i = 0;

  if((NULL == alertMsg) || ((alertLevel < rsuAlertLevel_info) || (rsuAlertLevel_critical < alertLevel))){
      if(0x0 == (errorstates & RSUACCMGR_BAD_INPUT)) {
          errorstates |= RSUACCMGR_BAD_INPUT;
          I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"sendtrap: RSUACCMGR_BAD_INPUT.\n");
      }
  } else {
      if(NULL != shm_rsuhealth_ptr) {
          for(i = 0; i < MAX_TRY_LOCK; i++) {
              if(WTRUE == wsu_shmlock_trylockw(&shm_rsuhealth_ptr->h.ch_lock)) {
                  /* 
                   * Only update and what we are responsible for.
                   * rsuAsync.c will sample at 2Hz and clear for you when done.
                   */
                  shm_rsuhealth_ptr->rsuAccessError = alertLevel;
                  if(RSU_ALERT_MSG_LENGTH_MAX < strnlen(alertMsg,RSU_ALERT_MSG_LENGTH_MAX)) {
                      alertMsg[RSU_ALERT_MSG_LENGTH_MAX-1] = '\0'; /* Truncate and null terminate */
                      if(0x0 == (errorstates & RSUACCMGR_MSG_TRUNCATE)) {
                          errorstates |= RSUACCMGR_MSG_TRUNCATE;
                          I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"sendtrap: RSUACCMGR_MSG_TRUNCATE.\n");
                      }
                  }
                  memcpy(shm_rsuhealth_ptr->rsuAccessMsg, alertMsg, RSUHEALTH_ALERT_MSG_LENGTH_MAX);
              }
              if(WFALSE == wsu_shmlock_unlockw(&shm_rsuhealth_ptr->h.ch_lock)) {
                  if(0x0 == (errorstates & RSUACCMGR_SHM_SEMA_UNLOCK_FAIL)) {
                      I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"sendtrap: RSUACCMGR_SHM_SEMA_UNLOCK_FAIL.\n");
                      errorstates |= RSUACCMGR_SHM_SEMA_UNLOCK_FAIL;
                  }
              }
              break; /* Done. */
          }/* for */
          if (MAX_TRY_LOCK <= i) { /* Could happen. */
              if(0x0 == (errorstates & RSUACCMGR_SHM_SEMA_LOCK_FAIL)) {
                  I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"sendtrap: RSUACCMGR_SHM_SEMA__LOCKFAIL.\n");
                  errorstates |= RSUACCMGR_SHM_SEMA_LOCK_FAIL;
              }
          }
      }
  }
}



int32_t MAIN(int32_t argc, char_t *argv[])
{
  uint32_t  iteration   = 0;
  uint64_t  start_UTC_msec = 0;
  uint64_t  stop_UTC_msec  = 0;
  uint64_t  total_UTC_msec = 0;
  int32_t   syslog_enabled = 0;
  char_t    trap_message[RSU_ALERT_MSG_LENGTH_MAX];
  uint32_t result;     //jm
  FILE    * fp = NULL;  //jm
  char_t    replyBuf[128]; /* Size according to your needs. */
  int32_t   i=0;

  argc = argc; /* Silence warnings */
  argv = argv;

  /* Init local & static vars */
  iteration   = 0;
  init_statics();

  /* Enable syslog and serial output */
  i2vUtilEnableDebug(MY_NAME);
  if (0 == i2vUtilEnableSyslog(MY_ERR_LEVEL, MY_NAME)){
      syslog_enabled = 1;
  }

  /* Install sig handlers */
  sighandler(termhandler, SIGTERM);
  sighandler(termhandler, SIGINT);

  /* Setup access to RSUHEALTH SHM for trap handling. */
  opendiagnosticshm();
  /* Forever, keep checking even if errors. */
  while(mainloop) {

   // syslog test case
  I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"snmp: mainloop check\n");   //jm

      if(0 == syslog_enabled) { /* LOGMGR may not be ready so keep trying till it's ready. */
          if (0 == i2vUtilEnableSyslog(MY_ERR_LEVEL, MY_NAME)){
              syslog_enabled = 1;
          }
      } 
      if(0 == (iteration % OUTPUT_MODULUS)) {
          if(prior_errorstates != errorstates) { /* Only output on event change. */
              I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"(0x%x)\n",errorstates);
              prior_errorstates = errorstates;
          }
      }
      start_UTC_msec = i2vUtilGetTimeInMs(); /* I2V timer example in UTC msecs. */
      //jm sleep(10);
      sleep(20);
      stop_UTC_msec  = i2vUtilGetTimeInMs();
//TODO: pipe to null device so we get no output to screen.

      //jm I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"snmp: check for Authentication Failure\n");   //jm

      //result = system("egrep 'Authentication Failure' /rwflash/client5.txt | wc -l  > /dev/null");  //jm
      //result = system("egrep 'Authentication Failure' /tmp/xcgi-login-debug.txt | wc -l");  //jm

      //result = system("egrep 'authentication failure' /var/log/messages | wc -l");  //jm

      //I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"result:  %d \n",result);


      //result = system("egrep 'Authentication Failure' /rwflash/client5.txt | wc -l  > /dev/null");  //jm
      //result = system("egrep 'Authentication Failure' /tmp/xcgi-login-debug.txt | wc -l");  //jm

      //result = system("egrep 'authentication failure' /var/log/messages | wc -l");  //jm

      //I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"result:  %d \n",result);

          /* Log "ssh Authentication failure" to syslog if excessive ssh login attempts */
 //     if ( (system( "egrep 'authentication failure' /var/log/messages | wc -l"))  >  0) 
 //if ( system( "egrep 'authentication failure' /var/log/messages | wc -l  >  0")  )

/*         result = system("egrep 'authentication failure' /var/log/messages |wc -l");
        I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"ssh Authentication failure, result: %d\n", result);

        if (result)
        {

        // log to syslog
        I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"ssh Authentication failure\n");

        // send snmp trap
        memset(trap_message,0x0,sizeof(trap_message));
        snprintf(trap_message,sizeof(trap_message),
              "[Authentication Failure occured: utc_msec(0x%lx)]"            
              ,total_UTC_msec);
        sendtrap(trap_message, rsuAlertLevel_warning);
        } */

        memset(replyBuf,'\0',sizeof(replyBuf));
        //memset(config,'\0',sizeof(config));

        if (NULL != (fp = popen(DEBUG_FILE, "r"))) {
            if(NULL != fgets(replyBuf, sizeof(replyBuf), fp)){

               i = strnlen(replyBuf,sizeof(replyBuf));
               if(0<i) {replyBuf[i-1] = '\0';} /* Delete extra char added. */

               if(0 != strcmp(replyBuf, SSH_AUTH_FAIL)){
                   result = 1;
               } else {
                   result = 0;
               } 
      
            }

        fclose(fp);

        }


      total_UTC_msec = stop_UTC_msec - start_UTC_msec;

      if(0 == (iteration % 60)) {

        if(result >= 1) {
          memset(trap_message,0x0,sizeof(trap_message));
          snprintf(trap_message,sizeof(trap_message),
              "[Authentication Failure occured: utc_msec(0x%lx)]"            
              ,total_UTC_msec);
          sendtrap(trap_message, rsuAlertLevel_warning);
        }
      }

      iteration++;
      #if defined(MY_UNIT_TEST)
      if(10<iteration)
          mainloop=WFALSE;
      #endif
  }
  closediagnosticshm();
  #if defined(EXTRA_DEBUG)
  I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"EXIT: mainloop(%d) errorstates(0x%x)\n", mainloop, errorstates);
  #endif
  i2vUtilDisableSyslog();
  i2vUtilDisableDebug();
  return 0;
}
