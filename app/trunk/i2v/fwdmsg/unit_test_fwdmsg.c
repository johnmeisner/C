
/*
** Unit tests for fwdmsg.c. For coverage using gcov unless test states otherwise.
*/

/**************************************************************************************************
* Includes
***************************************************************************************************/
#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>
#include <byteswap.h>
#include <unistd.h>
#include "wsu_util.h"
#include "wsu_shm.h"
#include "i2v_general.h"
#include "i2v_util.h"
#include "i2v_shm_master.h"
#if defined(J2735_2016)
#include "DSRC.h"
#endif
#if defined(J2735_2023)
#include "MessageFrame.h"
#include "Common.h"
#endif
#include "stubs.h"
#include "fwdmsg.h"
#include "conf_table.h"
/**************************************************************************************************
* Defines
***************************************************************************************************/
/* mandatory logging defines */
#define MY_ERR_LEVEL   LEVEL_INFO    /* from i2v_util.h */
#define MY_NAME        "fwdmsg_unit"

/* Test configs used. */
#define DEFAULT_CONFIG  "./"
#define NO_CONFIG       "../stubs/"
#define HOST_PC_IP      "10.52.11.47"  //Your PC IP here. Intention this is a valid IP.
#define UT_FLAVOR_MYPCIP 1
#define UT_FLAVOR_ENABLEFWD 2

#define PRINTIT printf("\nret=%d.",ret);
/**************************************************************************************************
* Globals
***************************************************************************************************/
static i2vShmMasterT * UnitshmPtr;

/* fwdmsg.c */
extern bool_t           mainloop; /* Calling main() will set to WTRUE. Otherwise you must reset depending on function you call. */
extern fwdmsgCfgItemsT fwdmsgCfg;          
extern int fwdmsgSock[MAX_FWD_MSG_IP];
extern struct sockaddr_in dest[MAX_FWD_MSG_IP];
char_t  confFileName[I2V_CFG_MAX_STR_LEN];

static i2vShmMasterT * UnitshmPtr; /* If you are not calling main() then you need to set SHM to this. */

/**************************************************************************************************
* Protos & Externs: Declare to silence warnings.
***************************************************************************************************/
/* fwdmsg.c */
extern int my_main(void);
extern void fwdmsg_sighandler(int __attribute__((unused)) sig);
extern STATIC int32_t uperFrame_SPAT(uint8_t * blob, uint32_t blobLength, MessageFrame * frame, OSCTXT * ctxt);

/* test.c */
void test_main(void);
int init_suite(void);
int clean_suite(void);

/**************************************************************************************************
* Function Bodies
***************************************************************************************************/
static int unit_test_flavor = 0;
void unit_test_spat_config_twiddle(void)
{

    switch(unit_test_flavor) {
        case UT_FLAVOR_ENABLEFWD:
            unit_test_flavor = 0;
            break;
        case UT_FLAVOR_MYPCIP:
            unit_test_flavor = 0;
            break;
        default:
            break;
    }
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

void test_main(void)
{
  int32_t       ret = FWDMSG_AOK; 

  /* Check for pending tests that did not complete.
   * Set tickle mask and ack mask for test.  
   */
  CU_ASSERT(0x0 == check_stub_pending_signals());
  clear_all_stub_signal();

  if ((UnitshmPtr = wsu_share_init(sizeof(i2vShmMasterT), I2V_SHM_PATH)) == NULL) {
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME," Main shared memory init failed\n");
      return;
  }
  CU_ASSERT(I2V_RETURN_OK == i2v_setupSHM_Default(UnitshmPtr)); /* Should reflect i2v.conf. */
  sleep(1); /* Give SHM a chance to digest. */

  /* Test against bad conf. */
  strcpy(UnitshmPtr->cfgData.config_directory, "../stubs/configs/");
  strcpy(confFileName, "fwdmsg_bad.conf");
  UnitshmPtr->cfgData.globalDebugFlag  = 1; /* If you want to see debug */
  UnitshmPtr->cfgData.h.ch_data_valid  = WTRUE;

  ret = my_main();
  CU_ASSERT(I2V_RETURN_OK == ret);
#if 0 //TODO: Need healing logic after bad conf parsed
  CU_ASSERT(fwdmsgCfg.fwdmsgAddr[0].fwdmsgEnable == FWDMSG_ENABLE1_DEFAULT);

printf("\nfwdmsgCfg.fwdmsgAddr[0]=%d vs %d\n",fwdmsgCfg.fwdmsgAddr[0].fwdmsgEnable,FWDMSG_ENABLE1_DEFAULT);

  CU_ASSERT(fwdmsgCfg.fwdmsgAddr[1].fwdmsgEnable == FWDMSG_ENABLE2_DEFAULT);
  CU_ASSERT(fwdmsgCfg.fwdmsgAddr[2].fwdmsgEnable == FWDMSG_ENABLE3_DEFAULT);
  CU_ASSERT(fwdmsgCfg.fwdmsgAddr[3].fwdmsgEnable == FWDMSG_ENABLE4_DEFAULT);
  CU_ASSERT(fwdmsgCfg.fwdmsgAddr[4].fwdmsgEnable == FWDMSG_ENABLE5_DEFAULT);

  CU_ASSERT(strcmp(fwdmsgCfg.fwdmsgAddr[0].fwdmsgFwdIp,FWDMSG_IP1_DEFAULT_S) == 0);
  CU_ASSERT(strcmp(fwdmsgCfg.fwdmsgAddr[0].fwdmsgFwdIp,FWDMSG_IP2_DEFAULT_S) == 0);
  CU_ASSERT(strcmp(fwdmsgCfg.fwdmsgAddr[0].fwdmsgFwdIp,FWDMSG_IP3_DEFAULT_S) == 0);
  CU_ASSERT(strcmp(fwdmsgCfg.fwdmsgAddr[0].fwdmsgFwdIp,FWDMSG_IP4_DEFAULT_S) == 0);
  CU_ASSERT(strcmp(fwdmsgCfg.fwdmsgAddr[0].fwdmsgFwdIp,FWDMSG_IP5_DEFAULT_S) == 0);

  CU_ASSERT(fwdmsgCfg.fwdmsgAddr[0].fwdmsgFwdPort == FWDMSG_PORT1_DEFAULT);
  CU_ASSERT(fwdmsgCfg.fwdmsgAddr[1].fwdmsgFwdPort == FWDMSG_PORT2_DEFAULT);
  CU_ASSERT(fwdmsgCfg.fwdmsgAddr[2].fwdmsgFwdPort == FWDMSG_PORT3_DEFAULT);
  CU_ASSERT(fwdmsgCfg.fwdmsgAddr[3].fwdmsgFwdPort == FWDMSG_PORT4_DEFAULT);
  CU_ASSERT(fwdmsgCfg.fwdmsgAddr[4].fwdmsgFwdPort == FWDMSG_PORT5_DEFAULT);

  CU_ASSERT(fwdmsgCfg.fwdmsgAddr[0].fwdmsgMask == FWDMSG_MASK1_DEFAULT);
  CU_ASSERT(fwdmsgCfg.fwdmsgAddr[1].fwdmsgMask == FWDMSG_MASK2_DEFAULT);
  CU_ASSERT(fwdmsgCfg.fwdmsgAddr[2].fwdmsgMask == FWDMSG_MASK3_DEFAULT);
  CU_ASSERT(fwdmsgCfg.fwdmsgAddr[3].fwdmsgMask == FWDMSG_MASK4_DEFAULT);
  CU_ASSERT(fwdmsgCfg.fwdmsgAddr[4].fwdmsgMask == FWDMSG_MASK5_DEFAULT);
#endif
  i2v_setupSHM_Clean(UnitshmPtr); /* Close SHM. */
  //39% coverage

  //Validate conf_table.h matches spat16.conf
  if ((UnitshmPtr = wsu_share_init(sizeof(i2vShmMasterT), I2V_SHM_PATH)) == NULL) {
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME," Main shared memory init failed\n");
      return;
  }
  CU_ASSERT(I2V_RETURN_OK == i2v_setupSHM_Default(UnitshmPtr)); /* Should reflect i2v.conf. */
  sleep(1); /* Give SHM a chance to digest. */

  strncpy(UnitshmPtr->cfgData.config_directory,DEFAULT_CONFIG,I2V_CFG_MAX_STR_LEN); /* Default conf used in install. */
  strcpy(confFileName, "fwdmsg.conf");
  ret = my_main();
#if 0
  CU_ASSERT(fwdmsgCfg.fwdmsgAddr[0].fwdmsgEnable == FWDMSG_ENABLE1_DEFAULT);
  CU_ASSERT(fwdmsgCfg.fwdmsgAddr[1].fwdmsgEnable == FWDMSG_ENABLE2_DEFAULT);
#endif
  CU_ASSERT(fwdmsgCfg.fwdmsgAddr[2].fwdmsgEnable == FWDMSG_ENABLE3_DEFAULT);
  CU_ASSERT(fwdmsgCfg.fwdmsgAddr[3].fwdmsgEnable == FWDMSG_ENABLE4_DEFAULT);
#if 0 //TODO: Maybe some defaults not in conf_table.h yet?
  CU_ASSERT(fwdmsgCfg.fwdmsgAddr[4].fwdmsgEnable == FWDMSG_ENABLE5_DEFAULT);
  CU_ASSERT(strcmp(fwdmsgCfg.fwdmsgAddr[0].fwdmsgFwdIp,FWDMSG_IP1_DEFAULT_S) == 0);
  CU_ASSERT(strcmp(fwdmsgCfg.fwdmsgAddr[0].fwdmsgFwdIp,FWDMSG_IP2_DEFAULT_S) == 0);
  CU_ASSERT(strcmp(fwdmsgCfg.fwdmsgAddr[0].fwdmsgFwdIp,FWDMSG_IP3_DEFAULT_S) == 0);
  CU_ASSERT(strcmp(fwdmsgCfg.fwdmsgAddr[0].fwdmsgFwdIp,FWDMSG_IP4_DEFAULT_S) == 0);
  CU_ASSERT(strcmp(fwdmsgCfg.fwdmsgAddr[0].fwdmsgFwdIp,FWDMSG_IP5_DEFAULT_S) == 0);

  CU_ASSERT(fwdmsgCfg.fwdmsgAddr[0].fwdmsgFwdPort == FWDMSG_PORT1_DEFAULT);
  CU_ASSERT(fwdmsgCfg.fwdmsgAddr[1].fwdmsgFwdPort == FWDMSG_PORT2_DEFAULT);
  CU_ASSERT(fwdmsgCfg.fwdmsgAddr[2].fwdmsgFwdPort == FWDMSG_PORT3_DEFAULT);
  CU_ASSERT(fwdmsgCfg.fwdmsgAddr[3].fwdmsgFwdPort == FWDMSG_PORT4_DEFAULT);
  CU_ASSERT(fwdmsgCfg.fwdmsgAddr[4].fwdmsgFwdPort == FWDMSG_PORT5_DEFAULT);

  CU_ASSERT(fwdmsgCfg.fwdmsgAddr[0].fwdmsgMask == FWDMSG_MASK1_DEFAULT);
  CU_ASSERT(fwdmsgCfg.fwdmsgAddr[1].fwdmsgMask == FWDMSG_MASK2_DEFAULT);
#endif
  CU_ASSERT(fwdmsgCfg.fwdmsgAddr[2].fwdmsgMask == FWDMSG_MASK3_DEFAULT);
  CU_ASSERT(fwdmsgCfg.fwdmsgAddr[3].fwdmsgMask == FWDMSG_MASK4_DEFAULT);
  CU_ASSERT(fwdmsgCfg.fwdmsgAddr[4].fwdmsgMask == FWDMSG_MASK5_DEFAULT);
  i2v_setupSHM_Clean(UnitshmPtr); /* Close SHM. */

#if 0
  if ((UnitshmPtr = wsu_share_init(sizeof(i2vShmMasterT), I2V_SHM_PATH)) == NULL) {
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME," Main shared memory init failed\n");
      return;
  }
  CU_ASSERT(I2V_RETURN_OK == i2v_setupSHM_Default(UnitshmPtr)); /* Should reflect i2v.conf. */
  sleep(1); /* Give SHM a chance to digest. */

  UnitshmPtr->cfgData.RadioType               = 1;
  UnitshmPtr->cfgData.globalDebugFlag         = 0;
  UnitshmPtr->cfgData.i2vDebugEnableFlag      = 1;
  UnitshmPtr->cfgData.bcastLockStep           = WTRUE;
  UnitshmPtr->cfgData.security                = WTRUE;
  ret = my_main();
  CU_ASSERT(FWDMSG_AOK == ret);
  i2v_setupSHM_Clean(UnitshmPtr); /* Close SHM. */

  fwdmsg_sighandler(1);

  if ((UnitshmPtr = wsu_share_init(sizeof(i2vShmMasterT), I2V_SHM_PATH)) == NULL) {
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME," Main shared memory init failed\n");
      return;
  }
  CU_ASSERT(I2V_RETURN_OK == i2v_setupSHM_Default(UnitshmPtr)); /* Should reflect i2v.conf. */
  sleep(1); /* Give SHM a chance to digest. */

  set_stub_signal(Signal_wsu_share_init);
  ret = my_main();
  CU_ASSERT(abs(FWDMSG_SHM_FAIL)  == ret);
  CU_ASSERT(0x0 == check_stub_pending_signals());

  //Create connect fail.
  unit_test_flavor = UT_FLAVOR_ENABLEFWD;
  set_stub_signal(Signal_connect_error);
  ret = my_main();
  //CU_ASSERT(FWDMSG_AOK == ret);                   //Did we get our result? If not fail test.
  //CU_ASSERT(unit_test_flavor == 0);               //Did we use our flavor? If not fail test.
  //CU_ASSERT(check_stub_pending_signals() == 0x0); //Did we use our signal? If not fail test.
  unit_test_flavor = 0;                           //If unsure clear flavor for next test.
  clear_all_stub_signal();                        //If unsure clear all signals for next test.

  //Set SPAT & BSM FWD on in spat16.conf.
  //Create inet_aton() fail.
  //Result is either FWDMSG_AOK or SPAT16_FWD_IP_ERROR.
  unit_test_flavor = UT_FLAVOR_ENABLEFWD;
  set_stub_signal(Signal_inet_aton);
  ret = my_main();
  //CU_ASSERT(FWDMSG_AOK == ret);                   //Did we get our result? If not fail test.
  //CU_ASSERT(0 == unit_test_flavor);               //Did we use our flavor? If not fail test.
  //CU_ASSERT(0x0 == check_stub_pending_signals()); //Did we use our signal? If not fail test.
  unit_test_flavor = 0;                           //Recommended:If unsure clear flavor for next test.
  clear_all_stub_signal();                        //Recommended:If unsure clear all signals for next test.

  //Point to non-existent spat16.conf and fopen() will fail.
  //Result is we get FWDMSG_LOAD_CONF_FAIL.
  strncpy(&UnitshmPtr->cfgData.config_directory,NO_CONFIG,I2V_CFG_MAX_STR_LEN);
  CU_ASSERT(0x0 == check_stub_pending_signals()); //Did we use our signal? If not fail test.
  clear_all_stub_signal();                        //Recommended:If unsure clear all signals for next test.

  //Use default config in ./spat16 used by install.
  //Result is FWDMSG_AOK.
  strncpy(UnitshmPtr->cfgData.config_directory,DEFAULT_CONFIG,I2V_CFG_MAX_STR_LEN);
  ret = my_main();
  CU_ASSERT(FWDMSG_AOK == ret);                   //Did we get our result? If not fail test.
  CU_ASSERT(0x0 == check_stub_pending_signals()); //Did we use our signal? If not fail test.
  clear_all_stub_signal();                        //Recommended:If unsure clear all signals for next test.

  //Set config to your pc ip so connect works and we write to socket in bsmrx.c
  //Note: We write even if nothing on otherside of connect.
  //Result is FWDMSG_AOK.
  //unit_test_flavor = UT_FLAVOR_MYPCIP;
  //wsu_open_gate(&UnitshmPtr->scsSpatData.spatTriggerGate);
  //ret = my_main();
  //CU_ASSERT(FWDMSG_AOK == ret);                   //Did we get our result? If not fail test.
  //CU_ASSERT(0 == unit_test_flavor);               //Did we use our flavor? If not fail test.
  //CU_ASSERT(0x0 == check_stub_pending_signals()); //Did we use our signal? If not fail test.
  //unit_test_flavor = 0;                           //Recommended:If unsure clear flavor for next test.
  clear_all_stub_signal();                        //Recommended:If unsure clear all signals for next test.

  i2v_setupSHM_Clean(UnitshmPtr); /* Close SHM. */
  #endif

  mainloop = WTRUE;  

  /* Pass in bad param and trigger bsmFwdLogger to print. */ 

  /* Excercise for coverage. */

  //set_stub_signal(Signal_socket);
  //CU_ASSERT(0x0 == check_stub_pending_signals());

//82% coverage

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
