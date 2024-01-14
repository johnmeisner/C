
/**************************************************************************************************
* Includes
***************************************************************************************************/

#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>
#include "wsu_util.h"
#include "wsu_shm.h"
#include "i2v_general.h"
#include "ris_struct.h"
#include "ris.h"
#include "i2v_util.h"
#include "i2v_shm_master.h"
#if defined(J2735_2016)
#include "DSRC.h"
#endif
#if defined(J2735_2023)
#include "MessageFrame.h"
#include "Common.h"
#endif
#include "i2v_riscapture.h"
#include "stubs.h"
#include "rs.h"
#include "conf_table.h"
#include "logmgr.h"
/**************************************************************************************************
* Defines
***************************************************************************************************/
/* mandatory logging defines */
#define MY_ERR_LEVEL   LEVEL_INFO    /* from i2v_util.h */
#define MY_NAME        "logmgr_unit"

#define IPV6_PREFIX_WORDS 4

/* debug only. */
#define PRINTIT   printf("\nret=%d.\n",ret);
/**************************************************************************************************
* Globals
***************************************************************************************************/
static i2vShmMasterT * UnitshmPtr;

/* logmgr.c */
extern lconfig lmgrCfg;
extern WBOOL mainloop;
extern char_t  confFileName[I2V_CFG_MAX_STR_LEN];

/* ifcmgr.c */

/**************************************************************************************************
* Protos & Externs
***************************************************************************************************/
/* logmgr.c */
extern int logmgr_main(int argc, char *argv[]);

/* ifclogger.c */
extern int ifclog_main(int argc, char *argv[]);

/* test.c */
void test_main(void);
int init_suite(void);
int clean_suite(void);

/* stubs.c */
extern i2vShmMasterT  shm_ptr_unit_test;
/**************************************************************************************************
* Function Bodies
***************************************************************************************************/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

void test_main(void)
{
  i2vReturnTypesT ret = I2V_RETURN_OK;

  /* Can test the good or bad but not together. 
   * Multiple calls to main() not working yet. 
   * Dangling threads or defunct system() calls or?
   */
  char   * argv_buffer[] = { (char []){"logmgr_app"},
                        (char []){"-d"},
                        (char []){"./"} ,
                        (char []){"-D"} ,
                        (char []){"0"} };
  /* Validate conf_table.h matches sysdotlog.conf */
  if ((UnitshmPtr = wsu_share_init(sizeof(i2vShmMasterT), I2V_SHM_PATH)) == NULL) {
      printf("\nMain shared memory init failed\n");
      return;
  }
  CU_ASSERT(I2V_RETURN_OK == i2v_setupSHM_Default(UnitshmPtr)); /* Should reflect i2v.conf. */
  sleep(1); /* Give SHM a chance to digest. */
  strcpy(confFileName, "syslogdot.conf");
  shm_ptr_unit_test.cfgData.globalDebugFlag = 1; /* If you want to see debug */
  ret = logmgr_main(5,argv_buffer);
  CU_ASSERT(I2V_RETURN_OK == ret);
  CU_ASSERT(lmgrCfg.logDuration == LOGMGR_LOG_LIFE_DEFAULT);
  CU_ASSERT(lmgrCfg.logSize == LOGMGR_LOG_SIZE_DEFAULT);
  CU_ASSERT(lmgrCfg.logPriority == LOGMGR_LOG_MIN_PRIORITY_DEFAULT);
  CU_ASSERT(strcmp(lmgrCfg.logPath,LOGMGR_LOG_DIR_DEFAULT_S) == 0);
  CU_ASSERT(lmgrCfg.dirsize == LOGMGR_LOG_DIR_SIZE_DEFAULT);
  CU_ASSERT(lmgrCfg.threshold == LOGMGR_LOG_DIR_THRESHOLD_DEFAULT);
  CU_ASSERT(lmgrCfg.logRmvAgeThresh == LOGMGR_LOG_DURATION_DEFAULT);
  CU_ASSERT(lmgrCfg.xmitShutdownEnable ==  LOGMGR_LOG_XMIT_ONHALT_ENABLE_DEFAULT);
  CU_ASSERT(lmgrCfg.xmitInterval == LOGMGR_LOG_XMIT_INTERVAL_DEFAULT);
  CU_ASSERT(lmgrCfg.xmitIpv4 == LOGMGR_LOG_XMIT_USE_IPV4_DEFAULT);
  CU_ASSERT(lmgrCfg.ifcLogSizeMax == LOGMGR_IFACE_LOG_SIZE_DEFAULT);
  CU_ASSERT(lmgrCfg.ifcLogDuration == LOGMGR_IFACE_LIFE_DEFAULT);
  CU_ASSERT(strcmp(lmgrCfg.scpuser,LOGMGR_SVR_MGR_USER_DEFAULT_S) == 0);
  CU_ASSERT(strcmp(lmgrCfg.scpdir,LOGMGR_SVR_MGR_DIR_DEFAULT_S) == 0);
#if 0 //Not ready for primetime. Feature disabled.
  CU_ASSERT(strcmp(lmgrCfg.ipv4addr,LOGMGR_SVR_MGR_IPV4_DEFAULT_S) == 0);
  printf("\nipv4addr(%s) != (%s).\n",lmgrCfg.ipv4addr, LOGMGR_SVR_MGR_IPV4_DEFAULT_S);
  CU_ASSERT(strcmp(lmgrCfg.ipv6addr,LOGMGR_SVR_MGR_IPV6_PREFIX_DEFAULT_S ) == 0);
  printf("\nipv6addr(%s) != (%s).\n",lmgrCfg.ipv6addr, LOGMGR_SVR_MGR_IPV6_PREFIX_DEFAULT_S);
  CU_ASSERT(strcmp(&lmgrCfg.ipv6addr[IPV6_PREFIX_WORDS * 2],LOGMGR_SVR_MGR_IPV6_ID_DEFAULT_S) == 0);
  printf("\nipv6addr_prefix(%s) != (%s).\n",&lmgrCfg.ipv6addr[IPV6_PREFIX_WORDS * 2], LOGMGR_SVR_MGR_IPV4_DEFAULT_S);
#endif
  i2v_setupSHM_Clean(UnitshmPtr);

#if 0
  char   * argv_bad_buffer[] = { (char []){"logmgr_app"},
                        (char []){"-d"},
                        (char []){"../stubs/configs/"} ,
                        (char []){"-D"} ,
                        (char []){"0"} };
  /* Test against bad conf. */
  if ((UnitshmPtr = wsu_share_init(sizeof(i2vShmMasterT), I2V_SHM_PATH)) == NULL) {
      printf("\nMain shared memory init failed\n");
      return;
  }
  CU_ASSERT(I2V_RETURN_OK == i2v_setupSHM_Default(UnitshmPtr)); /* Should reflect i2v.conf. */
  sleep(1); /* Give SHM a chance to digest. */
  strcpy(confFileName, "syslogdot_bad.conf");
  shm_ptr_unit_test.cfgData.globalDebugFlag = 1; /* If you want to see debug */
  ret = logmgr_main(5,argv_bad_buffer);
  CU_ASSERT(I2V_RETURN_OK == ret);
  CU_ASSERT(lmgrCfg.logDuration == LOGMGR_LOG_LIFE_DEFAULT);
  CU_ASSERT(lmgrCfg.logSize == LOGMGR_LOG_SIZE_DEFAULT);
  CU_ASSERT(lmgrCfg.logPriority == LOGMGR_LOG_MIN_PRIORITY_DEFAULT);
  CU_ASSERT(strcmp(lmgrCfg.logPath,LOGMGR_LOG_DIR_DEFAULT_S) == 0);
  CU_ASSERT(lmgrCfg.dirsize == LOGMGR_LOG_DIR_SIZE_DEFAULT);
  CU_ASSERT(lmgrCfg.threshold == LOGMGR_LOG_DIR_THRESHOLD_DEFAULT);
  CU_ASSERT(lmgrCfg.logRmvAgeThresh == LOGMGR_LOG_DURATION_DEFAULT);
  CU_ASSERT(lmgrCfg.xmitShutdownEnable == LOGMGR_LOG_XMIT_ONHALT_ENABLE_DEFAULT);
  CU_ASSERT(lmgrCfg.xmitInterval == LOGMGR_LOG_XMIT_INTERVAL_DEFAULT);
  CU_ASSERT(lmgrCfg.xmitIpv4 == LOGMGR_LOG_XMIT_USE_IPV4_DEFAULT);
  CU_ASSERT(lmgrCfg.ifcLogSizeMax == LOGMGR_IFACE_LOG_SIZE_DEFAULT);
  CU_ASSERT(lmgrCfg.ifcLogDuration == LOGMGR_IFACE_LIFE_DEFAULT);
  CU_ASSERT(strcmp(lmgrCfg.scpuser,LOGMGR_SVR_MGR_USER_DEFAULT_S) == 0);
  CU_ASSERT(strcmp(lmgrCfg.scpdir,LOGMGR_SVR_MGR_DIR_DEFAULT_S) == 0);
#if 0 //Not ready for primetime. Feature disabled.
  CU_ASSERT(strcmp(lmgrCfg.ipv4addr,LOGMGR_SVR_MGR_IPV4_DEFAULT_S) == 0);
  printf("\nipv4addr(%s) != (%s).\n",lmgrCfg.ipv4addr, LOGMGR_SVR_MGR_IPV4_DEFAULT_S);
  CU_ASSERT(strcmp(lmgrCfg.ipv6addr,LOGMGR_SVR_MGR_IPV6_PREFIX_DEFAULT_S ) == 0);
  printf("\nipv6addr(%s) != (%s).\n",lmgrCfg.ipv6addr, LOGMGR_SVR_MGR_IPV6_PREFIX_DEFAULT_S);
  CU_ASSERT(strcmp(&lmgrCfg.ipv6addr[IPV6_PREFIX_WORDS * 2],LOGMGR_SVR_MGR_IPV6_ID_DEFAULT_S) == 0);
  printf("\nipv6addr_prefix(%s) != (%s).\n",&lmgrCfg.ipv6addr[IPV6_PREFIX_WORDS * 2], LOGMGR_SVR_MGR_IPV4_DEFAULT_S);
#endif
  i2v_setupSHM_Clean(UnitshmPtr);
#endif
//30% coverage

  /* Validate conf_table.h matches sysdotlog.conf */
  if ((UnitshmPtr = wsu_share_init(sizeof(i2vShmMasterT), I2V_SHM_PATH)) == NULL) {
      printf("\nMain shared memory init failed\n");
      return;
  }
  CU_ASSERT(I2V_RETURN_OK == i2v_setupSHM_Default(UnitshmPtr)); /* Should reflect i2v.conf. */
  sleep(1); /* Give SHM a chance to digest. */
  strcpy(confFileName, "syslogdot.conf");
  shm_ptr_unit_test.cfgData.globalDebugFlag = 1; /* If you want to see debug */
  char   * ifclog_argv_buffer[] = { (char []){"ifclogger_app"},
                        (char []){"-s"},
                        (char []){"-i"} ,
                        (char []){"cv2x0"} ,
                        (char []){"-n"} ,
                        (char []){"0"} ,
                        (char []){"-t"},
                        (char []){"/tmp/cv2x0-tracker"},
                        (char []){"-N"},
                        (char []){"0123456789ABCDEF0123456789ABABB"},
                        (char []){"-S"},
                        (char []){"1"},
                        (char []){"-T"},
                        (char []){"24"},
                        (char []){"-D"},
                        (char []){"4"},
                        (char []){"0"}} ;
  ret = ifclog_main(16,ifclog_argv_buffer); //50% coverage
  i2v_setupSHM_Clean(UnitshmPtr);

  /* Clear tickle mask for next test. 
   * Check ack mask is complete
   */
  CU_ASSERT(0x0 == check_stub_pending_signals());
  clear_all_stub_signal();

}


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
int init_suite(void) 
{ 
  return 0; 
}
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
int clean_suite(void) 
{ 
  return 0; 
}
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/**************************************************************************************************
* MAIN()
***************************************************************************************************/
int main (void)
{

  CU_pSuite    pSuite1 = NULL;
  CU_ErrorCode ret     = CUE_SUCCESS; /* SUCCESS till proven FAIL */

  init_stub_control();

  ret = CU_initialize_registry();

  if(CUE_SUCCESS == ret) {
      if(NULL == (pSuite1 = CU_add_suite("Basic_Test_Suite1", init_suite, clean_suite))) {
          ret = ~CUE_SUCCESS;
      }
  }

  if(CUE_SUCCESS == ret) {
      if ((NULL == CU_add_test(pSuite1, "\ntest_main...",test_main))) {
          ret = ~CUE_SUCCESS;
      }
  }

  if(CUE_SUCCESS == ret) {
     printf("\nCUnit CU_basic_run_tests...\n");
     ret = CU_basic_run_tests();
  } 

  fflush(stdout);

  if(CUE_SUCCESS != ret) {
      printf("\nCUnit Exception: %d [%s]\n",CU_get_error(),CU_get_error_msg());
  } else {
      printf("\nCUnit Complete.\n");
  }

  CU_cleanup_registry();

  return CU_get_error();

}  /* END of MAIN() */

/**************************************************************************************************
* End of Module
***************************************************************************************************/
