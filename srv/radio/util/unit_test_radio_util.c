
/**************************************************************************************************
***************************************************************************************************/

/**************************************************************************************************
* Includes
***************************************************************************************************/

#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>

#include "stubs.h"
#include "dn_types.h"
#include "conf_manager.h"
#include "conf_table.h"

/**************************************************************************************************
* Defines
***************************************************************************************************/
#define RECORDING_LOG_DIR  "/tmp/ifclogs"
#define FINISHED_LOG_DIR   "/tmp/ifclogs_done"
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

/* capture_pcap.c */
extern int32_t capture_pcap_main(int32_t argc, char_t *argv[]);

/**************************************************************************************************
* Function Bodies
***************************************************************************************************/

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void test_main(void)
{
  char_t  *token[MAX_TOKENS * 2];
  int32_t  my_argc=2;
  int32_t  i;

  /* force clean start */
  system("rm -rf /tmp/ifclogs");
  sleep(1);
  system("rm -rf /tmp/ifclogs_done");
  sleep(1);
  system("rm -rf /tmp/pktlogs");
  sleep(1);
  /* Check for pending tests that did not complete.
   * Set tickle mask and ack mask for test.  
   */
  CU_ASSERT(0x0 == check_stub_pending_signals());
  clear_all_stub_signal();

  for(i=0;i<MAX_TOKENS * 2;i++) {
      token[i] = (char_t *)malloc(MAX_TOKEN_SIZE * 2); /* Allow room for null + extra junk to break conf_manager */
      strcpy(token[i],"");
  }

  /* too few arguements */
  my_argc=2;
  strcpy(token[0],"./capture_pcap");
  strcpy(token[1],"-m");
  capture_pcap_main(my_argc,token);
#if 0
  /* test mask and create a bunch of logs 1 minute long*/
  my_argc=11;
  strcpy(token[0],"./capture_pcap");
  strcpy(token[1],"-m");
  strcpy(token[2],"1");
  strcpy(token[3],"-i");
  strcpy(token[4],"-DEV_CV2X0");
  strcpy(token[5],"-s");
  strcpy(token[6],"512");
  strcpy(token[7],"-t");
  strcpy(token[8],"1");
  strcpy(token[9],"-T");
  strcpy(token[10],"1");
  capture_pcap_main(my_argc,token);

  strcpy(token[2],"2");
  capture_pcap_main(my_argc,token);

  strcpy(token[2],"3");
  capture_pcap_main(my_argc,token);

  strcpy(token[2],"7");
  capture_pcap_main(my_argc,token);

  strcpy(token[2],"15");
  capture_pcap_main(my_argc,token);

  strcpy(token[2],"31");
  capture_pcap_main(my_argc,token);

  /* now test with limit set on size of dir with output.
   * create logs 512KB till 5MB reached or 10 minutes expires,
   * will cause logs to be pruged.
   */
  my_argc=13;
  strcpy(token[1],"-m");
  strcpy(token[2],"3");
  strcpy(token[3],"-i");
  strcpy(token[4],"-DEV_CV2X0");
  strcpy(token[5],"-s");
  strcpy(token[6],"512");
  strcpy(token[7],"-t");
  strcpy(token[8],"10");
  strcpy(token[9],"-T");
  strcpy(token[10],"60");
  strcpy(token[11],"-x");
  strcpy(token[12],"5");
  capture_pcap_main(my_argc,token);

  /* send data to ip & port */
  my_argc=17;
  strcpy(token[0],"./capture_pcap");
  strcpy(token[1],"-m");
  strcpy(token[2],"3");
  strcpy(token[3],"-i");
  strcpy(token[4],"-DEV_CV2X0");
  strcpy(token[5],"-s");
  strcpy(token[6],"512");
  strcpy(token[7],"-t");
  strcpy(token[8],"1");
  strcpy(token[9],"-T");
  strcpy(token[10],"1");
  strcpy(token[11],"-x");
  strcpy(token[12],"5");
  strcpy(token[13],"-a");
  strcpy(token[14],"192.168.2.47");
  strcpy(token[15],"-b");
  strcpy(token[16],"10001");
  capture_pcap_main(my_argc,token);
#endif

  my_argc=15;
  strcpy(token[0],"./capture_pcap");
  strcpy(token[1],"-m");
  strcpy(token[2],"3");
  strcpy(token[3],"-i");
  strcpy(token[4],"-DEV_CV2X0");
  strcpy(token[5],"-s");
  strcpy(token[6],"1024");
  strcpy(token[7],"-t");
  strcpy(token[8],"1");
  strcpy(token[9],"-T");
  strcpy(token[10],"1");
  strcpy(token[11],"-x");
  strcpy(token[12],"1");
  strcpy(token[13],"--filename");
  strcpy(token[14],"cv2x0-tracker");
  capture_pcap_main(my_argc,token);

  CU_ASSERT(0x0 == check_stub_pending_signals());
  clear_all_stub_signal();

  for(i=0;i<MAX_TOKENS * 2;i++) {
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
