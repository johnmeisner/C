
/**************************************************************************************************
***************************************************************************************************/

/**************************************************************************************************
* Includes
***************************************************************************************************/
#include <signal.h>
#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>

#include "stubs.h"
#include "dn_types.h"
#include "conf_manager.h"
#include "conf_table.h"

/**************************************************************************************************
* Defines
***************************************************************************************************/

/**************************************************************************************************
* Globals
***************************************************************************************************/

/**************************************************************************************************
* Protos & Externs
***************************************************************************************************/

/* test.c */
void test_main(void);
int init_suite(void);
int clean_suite(void);

/* lpwrmonitor.c */
extern int32_t pshbtn_main(int32_t argc, char_t *argv[]);
extern void termhandler(int a, siginfo_t *b, void __attribute__((unused)) *c);

/**************************************************************************************************
* Function Bodies
***************************************************************************************************/

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void test_main(void)
{
  char_t  *token[MAX_TOKENS];
  int32_t  my_argc=2;
  int32_t  i;
  char_t   cmd[256];

  memset(cmd,0x0,sizeof(cmd));

  /* Check for pending tests that did not complete.
   * Set tickle mask and ack mask for test.  
   */
  CU_ASSERT(0x0 == check_stub_pending_signals());
  clear_all_stub_signal();


  for(i=0;i<MAX_TOKENS;i++) {
      token[i] = (char_t *)malloc(MAX_TOKEN_SIZE * 2); /* Allow room for null + extra junk to break conf_manager */
      strcpy(token[i],"");
  }

  my_argc=2;
  strcpy(token[0],"./pshbtnmonitor");

  /* 
   * Test daemon mode which has rsudiagnostic support.
   */
  strcpy(token[1],"-d");
  pshbtn_main(my_argc,token); /* Nominal case */

 
  strcpy(token[1],"-d");
  pshbtn_main(my_argc,token); /* Fail to open device. */

  /* for coverage */
  termhandler(1, NULL, NULL);

  CU_ASSERT(0x0 == check_stub_pending_signals());
  clear_all_stub_signal();

  for(i=0;i<MAX_TOKENS;i++) {
      free(token[i]);
  }
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
