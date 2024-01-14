/**************************************************************************
 *                                                                        *
 *     File Name:  rsuTable.c                                             *
 *     Author:                                                            *
 *         DENSO International America, Inc.                              *
 *         North America Research Laboratory, California Office           *
 *         Rutherford Road                                                *
 *         Carlsbad, 92008                                                *
 **************************************************************************/
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <pthread.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include "rsutable.h"
#include "ntcip-1218.h"
#include "i2v_util.h"
#if defined(MY_UNIT_TEST)
#include "stubs.h"
#endif

#define MY_NAME       "rsu_table"
#define MY_NAME_EXTRA "Xrsu_table" /* Lots of extra output, be careful. */
#define MY_ERR_LEVEL   LEVEL_PRIV    /* from i2v_util.h */

#if defined(EXTRA_DEBUG)
#define OUTPUT_MODULUS  30  /* Seconds. */
#else
#define OUTPUT_MODULUS  1200
#endif

/* rsutable.c specific: Re-use RSEMIB where possible. Create unique if needed. */
#define RSUTABLE_BAD_INPUT       RSEMIB_BAD_INPUT
#define RSUTABLE_UNLOCK_FAIL     RSEMIB_UNLOCK_FAIL
#define RSUTABLE_LOCK_FAIL       RSEMIB_LOCK_FAIL
#define RSUTABLE_ROW_EMPTY       RSEMIB_ROW_EMPTY
#define RSUTABLE_BAD_DATA        RSEMIB_BAD_DATA
#define RSUTABLE_BAD_MODE        RSEMIB_BAD_MODE
#define RSUTABLE_THREAD_FAIL     RSEMIB_THREAD_FAIL

static uint32_t  rsutable_error_states = 0x0;

int32_t mainloop = 1;
static int32_t rsuDaemonize = 0; /* Will go to 1 once parent has exited. Then we know it's child running now */
static pthread_t rsuTableUpdaterThrId;
static uint32_t  updater_counter = 1; /* rolling counter. Start at 1 so modulus ops don't commense at time zero. */
static pthread_t receivedMsgThrId;      /* 5.6    rsuReceivedMsg. */
static pthread_t gnssOutputThrId;       /* 5.7    rsuGnssOutput. */
static pthread_t interfaceLogThrId;     /* 5.8    rsuInterfaceLog. */
static pthread_t messageStatsThrId;     /* 5.12   rsuMessageStats. */
static pthread_t commRangeThrId;        /* 5.13.6 rsuCommRange. */
static pthread_t installUpdateThrId;    /* 5.14.9 rsuInstallUpdate. */
static pthread_t sysSettingsThrId;      /* 5.15   rsuSysSettings. */
static pthread_t rsuAsyncThrId;         /* 5.18	  Asynchronous Message */

/* function protos */
static void   rsu_table_init_static(void);
static void * rsuTableUpdaterThr(void __attribute__((unused)) *arg);

void set_rsutable_error_states(int32_t rsutable_error)
{
  int32_t dummy = 0;

    dummy = abs(rsutable_error);

    /* -1 to -32 */
    if((rsutable_error < 0) && (rsutable_error > MIB_ERROR_BASE_2)) {
        rsutable_error_states |= (uint32_t)(0x1) << (dummy - 1);
    }
    return;
}

static void rsu_table_init_static(void)
{
  rsutable_error_states = 0x0;
  memset(&rsuTableUpdaterThrId,0x0,sizeof(rsuTableUpdaterThrId));/* Try to close old one? if still around across soft reset? */
  memset(&receivedMsgThrId,0x0,sizeof(receivedMsgThrId));
  memset(&gnssOutputThrId,0x0,sizeof(gnssOutputThrId));
  memset(&messageStatsThrId,0x0,sizeof(messageStatsThrId));
  memset(&interfaceLogThrId,0x0,sizeof(interfaceLogThrId));
  memset(&commRangeThrId,0x0,sizeof(commRangeThrId));
  memset(&installUpdateThrId,0x0,sizeof(installUpdateThrId));
  memset(&sysSettingsThrId,0x0,sizeof(sysSettingsThrId));
  memset(&rsuAsyncThrId,0x0,sizeof(rsuAsyncThrId));
  mainloop = 1;
  /* Do not re-init. Parent & child task of daemon will call.  */
  /* rsuDaemonize = 0; */
}

/* Where applicable refresh from confs of i2v. 
 * Today we have no way of knowing when updated so poll at some rate.
 * Is there a better way? We can query at the time needed but will slow other transactions.
 * Could we have handshake in sous stating update occured?
 */
static void refresh_rsuRadio(void)
{
  int32_t data_out = 0;
  int32_t ret = RSEMIB_OK; /* Fails are don't care, silence of the warnings. */

  /* 5.2.2: MIB write only. */
  /* 5.2.3 */
#if !defined(PLATFORM_HD_RSU_5940)
  if(RSEMIB_OK > (ret = get_rsuRadioEnable(RSU_DSRC_RADIO))){
     DEBUGMSGTL((MY_NAME_EXTRA, "refresh_rsuRadio failed.\n"));
  }
  /* 5.2.4: Hard coded. */
  /* 5.2.5: Hard coded. */
  /* 5.2.6 */
  if(RSEMIB_OK > (ret = get_rsuRadioCh1(RSU_DSRC_RADIO))){
     DEBUGMSGTL((MY_NAME_EXTRA, "refresh_rsuRadio failed.\n"));
  }
  /* 5.2.7 */
  if(RSEMIB_OK > (ret = get_rsuRadioCh2(RSU_DSRC_RADIO))){
     DEBUGMSGTL((MY_NAME_EXTRA, "refresh_rsuRadio failed.\n"));
  }
  /* 5.2.8 */
  if(RSEMIB_OK > (ret = get_rsuRadioTxPower1(RSU_DSRC_RADIO, &data_out))){
     DEBUGMSGTL((MY_NAME_EXTRA, "refresh_rsuRadio failed.\n"));
  }
  /* 5.2.9 */
  if(RSEMIB_OK > (ret = get_rsuRadioTxPower2(RSU_DSRC_RADIO, &data_out))){
     DEBUGMSGTL((MY_NAME_EXTRA, "refresh_rsuRadio failed.\n"));
  }
#endif
  if(RSEMIB_OK > (ret = get_rsuRadioEnable(RSU_CV2X_RADIO))){
     DEBUGMSGTL((MY_NAME_EXTRA, "refresh_rsuRadio failed.\n"));
  }
  /* 5.2.4: Hard coded. */
  /* 5.2.5: Hard coded. */
  /* 5.2.6 */
  if(RSEMIB_OK > (ret = get_rsuRadioCh1(RSU_CV2X_RADIO))){
     DEBUGMSGTL((MY_NAME_EXTRA, "refresh_rsuRadio failed.\n"));
  }
  /* 5.2.7 */
  if(RSEMIB_OK > (ret = get_rsuRadioCh2(RSU_CV2X_RADIO))){
     DEBUGMSGTL((MY_NAME_EXTRA, "refresh_rsuRadio failed.\n"));
  }
  /* 5.2.8 */
  if(RSEMIB_OK > (ret = get_rsuRadioTxPower1(RSU_CV2X_RADIO, &data_out))){
     DEBUGMSGTL((MY_NAME_EXTRA, "refresh_rsuRadio failed.\n"));
  }
  /* 5.2.9 */
  if(RSEMIB_OK > (ret = get_rsuRadioTxPower2(RSU_CV2X_RADIO, &data_out))){
     DEBUGMSGTL((MY_NAME_EXTRA, "refresh_rsuRadio failed.\n"));
  }
}
/* Must be ready across reset for Install Update on disk, but not in MIB. */
static void *rsuInstallUpdateThr(void __attribute__((unused)) *arg)
{
  DEBUGMSGTL((MY_NAME,"rsuInstallUpdateThr: Entry.\n"));
  while (mainloop) {
      sleep(1); /* Once a second is a good rate. */
      installFirmware(); /* Will check if user made request to update firmware.  */
      DEBUGMSGTL((MY_NAME_EXTRA,"rsuInstallUpdateThr: BEEP.\n"));
  }
  DEBUGMSGTL((MY_NAME,"rsuInstallUpdateThr: Exit."));
  pthread_exit(NULL);
}
STATIC void dump_to_syslog(void)
{
  if(get_ntcip_1218_error_states() || get_rsuRadioTable_error_states())
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"NTCIP-1218 = (0x%X) radioTable = (0x%X)\n", get_ntcip_1218_error_states(), get_rsuRadioTable_error_states());

  if(get_rsuGnssStatus_error_states() || get_rsuGnssOutput_error_states())
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"GnssStatus = (0x%X) GnssOutput = (0x%X)\n", get_rsuGnssStatus_error_states(), get_rsuGnssOutput_error_states());

  if(get_rsuSysDescription_error_states() || get_rsuSysSettings_error_states())
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"SysDesc    = (0x%X) SysSetts   = (0x%X)\n", get_rsuSysDescription_error_states(), get_rsuSysSettings_error_states());

  if(get_rsuSysStatus_error_states() || get_rsuAsync_error_states())
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"SysStatus  = (0x%X) Async      = (0x%X)\n", get_rsuSysStatus_error_states(), get_rsuAsync_error_states());

  if(get_rsuSecurity_error_states() || get_rsuAntenna_error_states())
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Security   = (0x%X) Antenna    = (0x%X)\n", get_rsuSecurity_error_states(), get_rsuAntenna_error_states());

  if(get_rsuAppConfig_error_states() || get_rsuService_error_states())
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"App Cfg    = (0x%X) Services   = (0x%X)\n", get_rsuAppConfig_error_states(), get_rsuService_error_states());

  if(get_rsuMessageStats_error_states() || get_rsuMsgRepeat_error_states())
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Msg Stats  = (0x%X) Msg Repeat = (0x%X)\n", get_rsuMessageStats_error_states(), get_rsuMsgRepeat_error_states());

  if(get_rsuIFM_error_states() || get_receivedMsg_error_states())
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"IFM        = (0x%X) Rx Msg     = (0x%X)\n", get_rsuIFM_error_states(), get_receivedMsg_error_states());

  if(get_interfaceLog_error_states() || get_rsuWsaConfig_error_states())
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"IFC log    = (0x%X) WSA Config = (0x%X)\n", get_interfaceLog_error_states(), get_rsuWsaConfig_error_states());

  if(get_rsuSystemStats_error_states() || get_rsuXmitMsgFwding_error_states())
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Sys Stats  = (0x%X) X Msg Fwd  = (0x%X)\n", get_rsuSystemStats_error_states(), get_rsuXmitMsgFwding_error_states());

  if(get_rsuWraConfig_error_states() || rsutable_error_states)
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"WraConfig  = (0x%X) MIB stat   = (0x%X)\n", get_rsuWraConfig_error_states(), rsutable_error_states);
}
/* 
 * TODO: Need health check & recovery on other threads.
 *     : Each thread could have rolling counter so if it gets "stuck".
 *     : Install sig handler.
 */

/* Periodically update MIB with values controlled by I2V. */
static void *rsuTableUpdaterThr(void __attribute__((unused)) *arg)
{

    DEBUGMSGTL((MY_NAME,"rsuTableUpdaterThr: Entry.\n"));

    while (mainloop) {
        sleep(1); /* Once a second is a good rate. */

        /* TODO: Wait till RSU ready and I2V up before polling. */
        if(9 == (updater_counter % 10)){
            refresh_rsuRadio();
        }
        if(0 == (updater_counter % 30)) {
            DEBUGMSGTL((MY_NAME,"**********************************************\n"));
            DEBUGMSGTL((MY_NAME,"NTCIP-1218 = (0x%X) radioTable = (0x%X)\n" , get_ntcip_1218_error_states()
                                                                            , get_rsuRadioTable_error_states()));
            DEBUGMSGTL((MY_NAME,"GnssStatus = (0x%X) GnssOutput = (0x%X)\n" , get_rsuGnssStatus_error_states()
                                                                            , get_rsuGnssOutput_error_states()));
            DEBUGMSGTL((MY_NAME,"SysDesc    = (0x%X) SysSetts   = (0x%X)\n" , get_rsuSysDescription_error_states()
                                                                            , get_rsuSysSettings_error_states()));
            DEBUGMSGTL((MY_NAME,"SysStatus  = (0x%X) Async      = (0x%X)\n" , get_rsuSysStatus_error_states()
                                                                            , get_rsuAsync_error_states()));
            DEBUGMSGTL((MY_NAME,"Security   = (0x%X) Antenna    = (0x%X)\n" , get_rsuSecurity_error_states()
                                                                            , get_rsuAntenna_error_states()));
            DEBUGMSGTL((MY_NAME,"App Cfg    = (0x%X) Services   = (0x%X)\n" , get_rsuAppConfig_error_states()
                                                                            , get_rsuService_error_states()));
            DEBUGMSGTL((MY_NAME,"Msg Stats  = (0x%X) Msg Repeat = (0x%X)\n" , get_rsuMessageStats_error_states()
                                                                            , get_rsuMsgRepeat_error_states()));
            DEBUGMSGTL((MY_NAME,"IFM        = (0x%X) Rx Msg     = (0x%X)\n" , get_rsuIFM_error_states()
                                                                            , get_receivedMsg_error_states()));
            DEBUGMSGTL((MY_NAME,"IFC log    = (0x%X) WSA Config = (0x%X)\n" , get_interfaceLog_error_states()
                                                                            , get_rsuWsaConfig_error_states()));
            DEBUGMSGTL((MY_NAME,"Sys Stats  = (0x%X) X Msg Fwd  = (0x%X)\n" , get_rsuSystemStats_error_states()
                                                                            , get_rsuXmitMsgFwding_error_states()));
            DEBUGMSGTL((MY_NAME,"WraConfig  = (0x%X) MIB stat   = (0x%X)\n" , get_rsuWraConfig_error_states()
                                                                            , rsutable_error_states));
        }
        if(0 == (updater_counter % OUTPUT_MODULUS)) {
            dump_to_syslog();
        }
        #if defined(ENABLE_DUMP_MIB_TO_DISK) /* Test only. Respective handlers do this when needed. */
        commit_to_disk(); /* Dump copy of entire MIB to disk. */
        #endif
        updater_counter++;
    }

    close_ntcip_1218(); /* Close access to MIB. */

    DEBUGMSGTL((MY_NAME,"rsuTableUpdaterThr: Exit: NTCIP-1218 error states=(0x%X).\n", get_ntcip_1218_error_states()));
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"rsuTableUpdaterThr: Exit: NTCIP-1218 error states=(0x%X).\n", get_ntcip_1218_error_states());
    pthread_exit(NULL);
}
/* this function checks if a directory or file exists only
   input pname must be fully qualified */
bool_t mibCheckDirOrFile(const char_t *pname)
{
    FILE *f;

    if ((NULL == pname) || ((f=fopen(pname, "r")) == NULL)) {
        return WFALSE;
    }

    fclose(f);
    return WTRUE;
}
static void init_Table_Threads(void)
{
  int32_t ret = -1;
  char_t cmd[300]; /* size accordingly */

  /* rsu required support for filling table content */
  ret = pthread_create(&rsuTableUpdaterThrId, NULL, rsuTableUpdaterThr, NULL);
  if (0 == ret) {
      DEBUGMSGTL((MY_NAME,"rsuTableUpdaterThrId SUCCESS.\n"));
  } else {
      DEBUGMSGTL((MY_NAME,"rsuTableUpdaterThrId: Failed. Fatal: ret=%d\n",ret));
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"rsuTableUpdaterThrId: Failed. Fatal: ret=%d\n",ret);
      mainloop = 0;
      return; /* This is fatal. In theory SNMPD is still there but who knows.  */
  }
  /* 5.6 rsuReceivedMsg. */
  ret = pthread_create(&receivedMsgThrId, NULL, receivedMsgThr, NULL);
  if (0 == ret) {
      DEBUGMSGTL((MY_NAME,"receivedMsgThrId SUCCESS.\n"));
  } else {
      DEBUGMSGTL((MY_NAME,"receivedMsgThrId: Failed. Not Fatal: ret=%d\n",ret));
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"receivedMsgThrId: Failed. Not Fatal: ret=%d\n",ret);
      set_rsutable_error_states(RSUTABLE_THREAD_FAIL);
  }
  /* 5.7 rsuGnssOutput. */
  ret = pthread_create(&gnssOutputThrId, NULL, gnssOutputThr, NULL);
  if (0 == ret) {
      DEBUGMSGTL((MY_NAME,"gnssOutputThrId SUCCESS.\n"));
  } else {
      DEBUGMSGTL((MY_NAME,"gnssOutputThrId: Failed. Not Fatal: ret=%d\n",ret));
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"gnssOutputThrId: Failed. Not Fatal: ret=%d\n",ret);
      set_rsutable_error_states(RSUTABLE_THREAD_FAIL);
  }
  /* 5.8 rsuInterfaceLog. */
  ret = pthread_create(&interfaceLogThrId, NULL, interfaceLogThr, NULL);
  if (0 == ret) {
      DEBUGMSGTL((MY_NAME,"interfaceLogThrId SUCCESS.\n"));
  } else {
      DEBUGMSGTL((MY_NAME,"interfaceLogThrId: Failed. Not Fatal: ret=%d\n",ret));
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"interfaceLogThrId: Failed. Not Fatal: ret=%d\n",ret);
      set_rsutable_error_states(RSUTABLE_THREAD_FAIL);
  }
  /* 5.12 rsuMessageStats. */
  ret = pthread_create(&messageStatsThrId, NULL, messageStatsThr, NULL);
  if (0 == ret) {
      DEBUGMSGTL((MY_NAME,"messageStatsThrId SUCCESS.\n"));
  } else {
      DEBUGMSGTL((MY_NAME,"messageStatsThrId: Failed. Not Fatal: ret=%d\n",ret));
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"messageStatsThrId: Failed. Not Fatal: ret=%d\n",ret);
      set_rsutable_error_states(RSUTABLE_THREAD_FAIL);
  }
  /* 5.13.2.1 rsuCommRange. */
  ret = pthread_create(&commRangeThrId, NULL, commRangeThr, NULL);
  if (0 == ret) {
      DEBUGMSGTL((MY_NAME,"commRangeThrId SUCCESS.\n"));
  } else {
      DEBUGMSGTL((MY_NAME,"commRangeThrId: Failed. Not Fatal: ret=%d\n",ret));
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"commRangeThrId: Failed. Not Fatal: ret=%d\n",ret);
      set_rsutable_error_states(RSUTABLE_THREAD_FAIL);
  }
  /* 5.14.9 rsuInstallUpdate. */
  ret = pthread_create(&installUpdateThrId, NULL, rsuInstallUpdateThr, NULL);
  if (0 == ret) {
      DEBUGMSGTL((MY_NAME,"installUpdateThrId SUCCESS.\n"));
  } else {
      DEBUGMSGTL((MY_NAME,"installUpdateThrId: Failed. Not Fatal: ret=%d\n",ret));
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"installUpdateThrId: Failed. Not Fatal: ret=%d\n",ret);
      set_rsutable_error_states(RSUTABLE_THREAD_FAIL);
  }
  /* 5.15 rsuSysSettings. */
  ret = pthread_create(&sysSettingsThrId, NULL, sysSettingsThr, NULL);
  if (0 == ret) {
      DEBUGMSGTL((MY_NAME,"sysSettingsThrId SUCCESS.\n"));
  } else {
      DEBUGMSGTL((MY_NAME,"sysSettingsThrId: Failed. Not Fatal: ret=%d\n",ret));
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"sysSettingsThrId: Failed. Not Fatal: ret=%d\n",ret);
      set_rsutable_error_states(RSUTABLE_THREAD_FAIL);
  }
  /* 5.18 rsuAsync.
   * Test if ASYNC_ENABLE_TRAPS_FILE exists. If not don't call async thread.
   */
  memset(cmd,0x0, sizeof(cmd));
  snprintf(cmd,sizeof(cmd),"%s/%s", ASYNC_ACTIVE_DIR, ASYNC_ENABLE_TRAPS_FILE);
  if(mibCheckDirOrFile(cmd)) {
      ret = pthread_create(&rsuAsyncThrId, NULL, rsuAsyncThr, NULL);
      if (0 == ret) {
          DEBUGMSGTL((MY_NAME,"rsuAsyncThrId SUCCESS.\n"));
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"SNMP Traps started.\n");
      } else {
          DEBUGMSGTL((MY_NAME,"rsuAsyncThrId: Failed. Not Fatal: ret=%d\n",ret));
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"rsuAsyncThrId: Failed. Not Fatal: ret=%d\n",ret);
          set_rsutable_error_states(RSUTABLE_THREAD_FAIL);
      }
  } else {
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"rsuAsync start file does not exist. No traps enabled.\n",ret);
  }
}

/** Initializes the rsutable module: Will fork() to detatch from I2V.
 *
 * LINUX
 * How does SNMPD know this is the init? => snmpd.conf::dlmod rsutable /usr/lib/rsumib.so 
 *
 * I called snmpd without -f and I couldn't get this daemonized properly.
 * The parent & child tasks clobbered one another somehow and rsuTable failed.
 *
 * Even with -f snmpd will call this TWICE with the same pid().
 * Therefore, I fork to get rid of one of the snmpd extra calls from above.
 * I don't think snmpd is doing things correctly but this works. 
 * Maybe this was always intended? Dunno.
 *
 * Call without -f. Let SNMPD fork for us. But parent pid different?
 *
 **/
void init_rsutable(void) 
{
  pid_t pid = 0;

    /*
     * Init tables AND scalars supported. 
     */
    pid = getpid();

    /*
     * HUGE assumption first call is parent. Seems so.
     */
    if(0 == rsuDaemonize) {
        DEBUGMSGTL((MY_NAME, "init_rsutable: Parent task Aborting: rsuDaemonize=%d, PID=%d.\n",rsuDaemonize,pid));
        rsuDaemonize = 1;
#if defined(LINUX)
       return; //exit(0); /* normally parent would exit here but don't. Tricky. */
#endif
    } else {
        DEBUGMSGTL((MY_NAME, "init_rsutable: Child task assumes Leader: rsuDaemonize=%d, PID=%d.\n",rsuDaemonize,pid));
#if !defined(HERC)
        return;
#endif
    }

    /*
     * head fake! we fork ourselves. 
     * I let snmpd do it (exclude -f) and both parent & child executed the init. 
     * This seems wrong to me and led to some weird boot up sequences and errors.
     * I was getting the same pid() executing this init twice and that doesn't seem right.
     * This way for sure the init executes only once by the child. This seems the correct way.
     */
    pid = fork();
    if (pid < 0) {
        DEBUGMSGTL((MY_NAME, "fork failed!\n"));
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"fork failed. FATAL.\n");
        exit(1);
    }

    if (pid > 0) {
        DEBUGMSGTL((MY_NAME, "init_rsutable:process_id of child process %d \n", pid));
        exit(0);
    }

    /*
     * Now this process is detached(daemonized) from I2V.
     * The intent is ONLY one daemonized PID makes it here to carry on seperate from I2V.
     */
    DEBUGMSGTL((MY_NAME, "init_rsutable: Entry: rsuDaemonize=%d, PID=%d.\n",rsuDaemonize,pid));

    rsu_table_init_static(); /* Clear local static vars in case of soft reset. */

    install_ntcip_1218();    /* Init & Install MIB handlers and all. */

    init_Table_Threads();    /* Start threads to support MIB. */
  
    DEBUGMSGTL((MY_NAME, "Exit: rsuDaemonize=%d, PID=%d.\n",rsuDaemonize,pid));

    return;
}

