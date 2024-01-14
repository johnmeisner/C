
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

/**************************************************************************************************
* Defines
***************************************************************************************************/

/**************************************************************************************************
* Globals
***************************************************************************************************/
static i2vShmMasterT * UnitshmPtr;

/* iwmh.c */
extern WBOOL          mainloop;
extern cfgItemsT      cfg;
extern pthread_t      txhlthID;

/**************************************************************************************************
* Protos & Externs
***************************************************************************************************/
/* iwmh.c */
extern int my_main(int argc, char *argv[]);

/* test.c */
void test_main(void);
int init_suite(void);
int clean_suite(void);

/**************************************************************************************************
* Function Bodies
***************************************************************************************************/

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

void test_main(void)
{
  int  ret = 0; 
  char * argv_buffer[] = { (char []){"iwhm_app"},
                           (char []){"-r"},
                           (char []){"0"}} ;

  /* Check for pending tests that did not complete.
   * Set tickle mask and ack mask for test.  
   */
  CU_ASSERT(0x0 == check_stub_pending_signals());
  clear_all_stub_signal();

  /* 
   * Setup I2V SHM flavor for this test.
   * Make sure module statics are reset per iteration.
   */ 
  if ((UnitshmPtr = wsu_share_init(sizeof(i2vShmMasterT), I2V_SHM_PATH)) == NULL) {
      printf("\nMain shared memory init failed\n");
      return;
  }
  CU_ASSERT(I2V_RETURN_OK == i2v_setupSHM_Default(UnitshmPtr)); /* Should reflect i2v.conf. */
  sleep(1); /* Give SHM a chance to digest. */


  /* Nominal case: Should just run. */
  ret = my_main(3,argv_buffer);
  CU_ASSERT(0 == ret);
  CU_ASSERT(0x0 == check_stub_pending_signals());

#if 0 //use large cap macros
  set_stub_signal(Signal_wsu_share_init);
  ret = my_main(3,argv_buffer);
  CU_ASSERT(2 == ret);
  CU_ASSERT(0x0 == check_stub_pending_signals());
#endif

  /* new thread testing */
  txhlthID = 0;
  cfg.gpsCtlTx = 1;
  cfg.RadioType = 1;    /* c-v2x branch check; identical branches but check different 
                           environment utilities not avialable in unit test */
  ret = my_main(3,argv_buffer);
  CU_ASSERT(0 == ret);
  CU_ASSERT(0 != txhlthID);    /* thread created in nominal case */
  CU_ASSERT(0x0 == check_stub_pending_signals());
  
  txhlthID = 0;
  cfg.gpsCtlTx = 1;
  cfg.RadioType = 0;    /* dsrc branch check; identical branches but check different 
                           environment utilities not avialable in unit test */
  ret = my_main(3,argv_buffer);
  CU_ASSERT(0 == ret);
  CU_ASSERT(0 != txhlthID);    /* thread created in nominal case */
  CU_ASSERT(0x0 == check_stub_pending_signals());
  
  txhlthID = 0;
  cfg.gpsCtlTx = 0;
  ret = my_main(3,argv_buffer);
  CU_ASSERT(0 == ret);
  CU_ASSERT(0 == txhlthID);    /* thread not created */
  CU_ASSERT(0x0 == check_stub_pending_signals());

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
