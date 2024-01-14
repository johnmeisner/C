/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Filename: pshbtnmonitor.c                                                  */
/* Purpose: Push Button Monitor                                               */
/*                                                                            */
/* Copyright (C) 2023 DENSO International America, Inc.  All Rights Reserved. */
/*                                                                            */
/*----------------------------------------------------------------------------*/
/* Derived from and credit to:
 *
 *  Filename: btn_srv.c
 *  Purpose: Reset button service
 *
 *  Copyright (C) 2019 DENSO International America, Inc.
 *
 *  Revision History:
 *
 *  Date        Author        Comments
 * --------------------------------------------------
 *  05/16/19    BDOBRYANSKI   Initial release.
 *  08/29/19    BDOBRYANSKI   CfgMgr release.
 *
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <mtd/mtd-user.h>
#include <sys/stat.h>
#include <time.h>
#include <pthread.h>
#include <signal.h>
#include "dn_types.h"
#include "i2v_util.h"
#include "conf_table.h"

#if defined(MY_UNIT_TEST)
#include "stubs.h"
#define MAIN pshbtn_main
#else
#define MAIN main
#endif

/* Mandatory logging defines. */
#if defined(MY_UNIT_TEST)
#define MY_ERR_LEVEL   LEVEL_DBG
#else
#define MY_ERR_LEVEL   LEVEL_PRIV /* syslog only, no serial. */
#endif
#define MY_NAME        "pshbtn"

/* mainloop and gpio sampling runs at ~4hz. */
#define WAIT_MSECS       250000

/* All dwell times are in msec */
#define REBOOT_DWELL_MIN 1500
#define REBOOT_DWELL_MAX 2500

#define RESET_IP_DWELL_MIN 4500
#define RESET_IP_DWELL_MAX 5500

#define RESET_RWFLASH_DWELL_MIN 9000
#define RESET_RWFLASH_DWELL_MAX 11500

#define RESET_FACTORY_DWELL_MIN 14000
#define RESET_FACTORY_DWELL_MAX 16500

/* half seconds */
#define OUTPUT_MODULUS 600

/* error states 32 bit */
#define PSHBTN_GPIO_VALUE_BAD   0x00000001
#define PSHBTN_GPIO_SSCANF_FAIL 0x00000002
#define PSHBTN_GPIO_FGETS_FAIL  0x00000004
#define PSHBTN_GPIO_POPEN_FAIL  0x00000008
#define PSHBTN_GPIO_TEST_FAIL   0x00000010

#define LOCATION_PSHBTM_GPIO    "/sys/class/gpio/gpio448/value"

#define READ_PSHBTN_GPIO       "cat /sys/class/gpio/gpio448/value"
#define READ_PSHBTN_GPIO_REPLY "%d"

/*
 * Globals.
 */
static int32_t  mainloop = 1;
static uint32_t error_states = 0x0;
static uint32_t prior_error_states = 0x0;
/*
 * ============ Helper functions ==============
 */
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
STATIC void init_statics(void)
{
  mainloop = 1;
  error_states = 0x0;
  prior_error_states = 0x0;
}
/*
 * Reboot()
 * Executes shutdown call
 */
STATIC void Reboot(void)
{
  if(0 != system("reboot")) {
      I2V_DBG_LOG(LEVEL_CRIT, MY_NAME,"Reboot failed: errno(%s)\n",strerror(errno));
  }
}
/*
 * ResetIP()
 * Resets IP to default RSU IP
 */
STATIC void ResetIP(void)
{
  char_t cmd[300];

  memset(cmd,0x0,sizeof(cmd));
  snprintf(cmd,sizeof(cmd),"/usr/bin/conf_agent WRITE RSU_INFO SC0IPADDRESS %s",I2V_RSU_ETH0_IP_DEFAULT_S);
  if(0 != system(cmd)) {
      I2V_DBG_LOG(LEVEL_CRIT, MY_NAME,"Reset IP failed: errno(%s)\n",strerror(errno));
  } else {
      if(0 != system("reboot")) {
        I2V_DBG_LOG(LEVEL_CRIT, MY_NAME,"RSU failed to reboot: IP is reset: errno(%s)\n",strerror(errno));
      }
  }
}
/*
 * Reset()
 * Resets /rwflash/configs to default, NOT IP, NOT rsuuser pwd
 */
STATIC void Reset(void)
{
  if(0 != system("/usr/bin/conf_agent FACTORY_DEFAULTS_RESET_BUT_PRESERVE_NETWORKING Yes ")) {
      I2V_DBG_LOG(LEVEL_CRIT, MY_NAME,"Reset Configs failed: errno(%s)\n",strerror(errno));
  } else {
      if(0 != system("reboot")) {
          I2V_DBG_LOG(LEVEL_CRIT, MY_NAME,"RSU failed to reboot: Configs have been reset: errno(%s)\n",strerror(errno));
      }
  }
}
/*
 * FactoryReset()
 * Resets /rwflash/configs to default, including IP & rsuuser pwd
 */
STATIC void FactoryReset(void)
{
  if(0 != system("/usr/bin/conf_agent FACTORY_DEFAULTS_RESET Yes ")) {
      I2V_DBG_LOG(LEVEL_CRIT, MY_NAME,"Factory Reset failed: Configs, IP, rsuuser PWD not reset: errno(%s)\n",strerror(errno));
  } else {
      /* In theory, no return for reset pwd. */
      if(0 != system("/usr/local/bin/rsuhealth -p")) {
          I2V_DBG_LOG(LEVEL_CRIT, MY_NAME,"Factory Reset failed: rsuuser PWD not reset: errno(%s)\n",strerror(errno));
      }
  }
}
/*
 * get_pshbtn_value()
 * Grab value of PSGBTN GPIO
 */
STATIC int32_t get_pshbtn_value(void)
{
  FILE    * fp = NULL;
  char_t    replyBuf[128]; /* Size according to your needs. */
  int32_t   gpio = 0x0;
  uint32_t  i = 0;
  int32_t   ret = -1; /* Fail till proven success */

  memset(replyBuf,'\0',sizeof(replyBuf));
  if(NULL != (fp = popen(READ_PSHBTN_GPIO, "r"))) {
      if(NULL != fgets(replyBuf, sizeof(replyBuf), fp)){
          i = strnlen(replyBuf,sizeof(replyBuf));
          if(0<i) {replyBuf[i-1] = '\0';} /* Delete extra char added. */
          if(1 == (ret = sscanf(replyBuf,READ_PSHBTN_GPIO_REPLY,&gpio))) {
              if((0 == gpio) || (1 == gpio)) {
#if defined(EXTRA_EXTRA_DEBUG) /* It's a lot so use cautiously */
                  I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"gpio value(%u)\n");
#endif
                  ret = gpio;
              } else {
                  if(0x0 == (error_states & PSHBTN_GPIO_VALUE_BAD)) {
                      error_states |= PSHBTN_GPIO_VALUE_BAD;
                      I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"gpio is bogus value(%u) failed errno(%s)\n",gpio);
                      error_states |= PSHBTN_GPIO_VALUE_BAD;
                  }
                  ret = -1;
              }
          } else {
              if(0x0 == (error_states & PSHBTN_GPIO_SSCANF_FAIL)) {
                  I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"sscanf failed ret(%d) errno(%s)\n",ret,strerror(errno));
                  error_states |= PSHBTN_GPIO_SSCANF_FAIL;
              }
              ret = -2;
          }
      } else {
          if(0x0 == (error_states & PSHBTN_GPIO_FGETS_FAIL)) {
              I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"fgets failed errno(%s)\n",strerror(errno));
              error_states |= PSHBTN_GPIO_FGETS_FAIL;
          }
          ret = -3;
      }
      pclose(fp);
  } else {
      if(0x0 == (error_states & PSHBTN_GPIO_POPEN_FAIL)) {
          I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"popen failed errno(%s)\n",strerror(errno));
          error_states |= PSHBTN_GPIO_POPEN_FAIL;
      }
      ret = -4;
  }
  return ret;
}
int32_t MAIN(int32_t argc, char_t *argv[])
{
  uint32_t  iteration   = 0;
  int32_t   counter     = 0; /* Polling time counter */
  int32_t   gpio_output = 0;
  uint64_t  start_UTC_msec = 0;
  uint64_t  stop_UTC_msec  = 0;
  uint64_t  total_UTC_msec = 0;
  int32_t   syslog_enabled = 0;

  argc = argc; /* Silence warnings */
  argv = argv;

  /* Init local & static vars */
  iteration   = 0;
  counter     = 0;
  gpio_output = 0;
  init_statics();

  /* Enable syslog and serial output */
  i2vUtilEnableDebug(MY_NAME);
  if (0 == i2vUtilEnableSyslog(MY_ERR_LEVEL, MY_NAME)){
      syslog_enabled = 1;
  }

  /* Install sig handlers */
  sighandler(termhandler, SIGTERM);
  sighandler(termhandler, SIGINT);

  /* Test existence of pin first. Dont bang on non-existent pin. */
  if(WFALSE == i2vCheckDirOrFile(LOCATION_PSHBTM_GPIO)) {
      sleep(10); /* No rush, let system boot a bit or output hard to see. */
      error_states |= PSHBTN_GPIO_TEST_FAIL; /* Fatal. */
      /* Not fatal, but very dull beyond this point. 
       * Periodic output below indicating we are toast. 
       */
      //mainloop = WFALSE;
      I2V_DBG_LOG(LEVEL_CRIT, MY_NAME,"GPIO does not exists or is not ready(%s)\n",LOCATION_PSHBTM_GPIO);
  }

  /* Forever, keep checking even if errors. */
  while(mainloop) {
      if(0 == syslog_enabled) { /* LOGMGR may not be ready so keep trying till it's ready. */
          if (0 == i2vUtilEnableSyslog(MY_ERR_LEVEL, MY_NAME)){
              syslog_enabled = 1;
          }
      } 
      if(0 == (iteration % OUTPUT_MODULUS)) {
          if(prior_error_states != error_states) { /* Only output on event change. */
              I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"(0x%x) GPIO value(%d)\n",error_states,gpio_output);
              prior_error_states = error_states;
          }
      }
      /* Longer counter time frame given to account for potential inaccuracy */
      if(0 <= (gpio_output = get_pshbtn_value())){
          if (gpio_output == 0) { /* Increase counter for each half second button is pressed */
              if(0 == counter) {
                  start_UTC_msec = i2vUtilGetTimeInMs(); /* Get start time. */
              }
              counter++;
          } else { /* Either they have let go of button or never pushed it */
              if(counter) {
                  stop_UTC_msec = i2vUtilGetTimeInMs(); /* Get stop time. */
                  total_UTC_msec = stop_UTC_msec - start_UTC_msec;
                  if (total_UTC_msec >= REBOOT_DWELL_MIN && total_UTC_msec <= REBOOT_DWELL_MAX) { /* REBOOT: Between 1.5 and 2.5 seconds */
                      I2V_DBG_LOG(LEVEL_WARN, MY_NAME,"Reboot RSU: counter(%d) msec(%lu)\n",counter,total_UTC_msec);
                      Reboot(); 
                  } else if (total_UTC_msec >= RESET_IP_DWELL_MIN && total_UTC_msec <= RESET_IP_DWELL_MAX) { /* RESET IP: Between 4.5 and 5.5 seconds */
                      I2V_DBG_LOG(LEVEL_WARN, MY_NAME,"Reset RSU IP: counter(%d) msec(%lu)\n",counter,total_UTC_msec);
                      ResetIP();
                  } else if (total_UTC_msec >= RESET_RWFLASH_DWELL_MIN && total_UTC_msec <= RESET_RWFLASH_DWELL_MAX) { /* RESET RWFLASH: Between 9 and 11.5 seconds */
                      I2V_DBG_LOG(LEVEL_WARN, MY_NAME,"Reset configs: counter(%d) msec(%lu)\n",counter,total_UTC_msec);
                      Reset();
                  } else if (total_UTC_msec >= RESET_FACTORY_DWELL_MIN && total_UTC_msec <= RESET_FACTORY_DWELL_MAX) { /* FACTORY RESET: Between 14 and 16.5 seconds */
                      I2V_DBG_LOG(LEVEL_WARN, MY_NAME,"Factory Reset RSU: counter(%d) msec(%lu)\n",counter,total_UTC_msec);
                      FactoryReset();
                  } else {
                      /* Either under or over pressed Ignore. */
                      I2V_DBG_LOG(LEVEL_WARN, MY_NAME,"Over/Under shoot, ignoring: counter(%d) msec(%lu)\n",counter,total_UTC_msec);
                  }
                  counter = 0; /* Done, reset counter. */
              }
          }
      } else {
          /* get_pshbtn_value() has its own error checking and handling. */
          counter = 0;
      }
      iteration++;
      usleep(WAIT_MSECS);    /* Check GPIO output every 0.25 seconds */
      #if defined(MY_UNIT_TEST)
      if(10<iteration)
          mainloop=WFALSE;
      #endif
  }
  #if defined(EXTRA_DEBUG)
  I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"EXIT: mainloop(%d) error_states(0x%x)\n", mainloop, error_states);
  #endif
  i2vUtilDisableSyslog();
  i2vUtilDisableDebug();
  return 0;
}
