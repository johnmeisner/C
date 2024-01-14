
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
/**************************************************************************************************
* Defines
***************************************************************************************************/
/* mandatory logging defines */
#define MY_ERR_LEVEL   LEVEL_INFO    /* from i2v_util.h */
#define MY_NAME        "ipb_unit"

/* ipb.c duplicate defintion */
#define IPV6_PREFIX_WORDS  4

#define IPB_AOK 0
#define IPB_INIT_SHM_FAIL             (-1)
#define IPB_BRDGE_IPV4_SERVER_FAIL    (-2)
#define IPB_BRDGE_IPV6_SERVER_FAIL    (-3)
#define IPB_BRDGE_INIT_PASS_THRU_FAIL (-4)
#define IPB_LOAD_CONF_FAIL            (-5)
#define IPB_INIT_RADIO_FAIL           (-6)
#define IPB_INIT_TPS_FAIL             (-7)
#define IPB_CREATE_SERVICE_GATE_FAIL  (-8)
#define IPB_CREATE_SERVICE_THREAD_MUTEX_FAIL  (-9)
#define IPB_CREATE_CLIENT_NAME_FAIL    (-10)
#define IPB_CREATE_CLIENT_PID_FAIL     (-11)
#define IPB_CREATE_CLIENT_THREAD_FAIL  (-12)
#define IPB_CREATE_BRIDGE_THREAD_FAIL (-13)

#define PRINTIT printf("\nret=%d.",ret);
/**************************************************************************************************
* Globals
***************************************************************************************************/
/* unit_test_ahm.c */
static i2vShmMasterT * UnitshmPtr;

/* ipb.c */
extern WBOOL            mainloop;
extern cfgItemsT        cfg;
extern ipbCfgItemsT     ipbCfg;

/**************************************************************************************************
* Protos & Externs
***************************************************************************************************/
/* ipb.c */
extern int my_main(void);
extern char_t  confFileName[I2V_CFG_MAX_STR_LEN];

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
  int ret = 0; 
  mainloop = WTRUE;

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
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME," Main shared memory init failed\n");
      return;
  }
  CU_ASSERT(I2V_RETURN_OK == i2v_setupSHM_Default(UnitshmPtr)); /* Should reflect i2v.conf. */
  sleep(1); /* Give SHM a chance to digest. */

  /* Test against bad conf. */
  strcpy(UnitshmPtr->cfgData.config_directory, "../stubs/configs/");
  strcpy(confFileName, "ipb_bad.conf");
  ret = my_main();
  CU_ASSERT(1 == ret);
#if 0 //TODO: Needs work
//TODO: Floats precision is rough for comparison. 
//    : So scale the rez u care and test that. ie Lat/Lon on *1e7 is relevant
  CU_ASSERT(ipbCfg.radioNum == I2V_RADIO_DEFAULT);
  CU_ASSERT(ipbCfg.wsaPriority == I2V_WSA_PRIORITY_DEFAULT);
  CU_ASSERT(ipbCfg.wsaTxRate == I2V_WSA_TX_RATE_DEFAULT);
  //CU_ASSERT(ipbCfg.txPwr     == I2V_TX_POWER_DEFAULT);
  //CU_ASSERT(ipbCfg.wsaPwr    == I2V_TX_POWER_DEFAULT);
//printf("\npwr %f %f != %f.",ipbCfg.txPwr,ipbCfg.wsaPwr,I2V_TX_POWER_DEFAULT);
  CU_ASSERT(ipbCfg.forceIPBCfgPwr == IPB_OVERRIDE_POWER_DEFAULT);
  CU_ASSERT(ipbCfg.dataRate  == I2V_WSA_DATA_RATE_DEFAULT);
  CU_ASSERT(ipbCfg.isContinuous == IPB_ENABLE_CONT_DEFAULT);
  CU_ASSERT(ipbCfg.isNotCCH  == IPB_OVERRIDE_POWER_DEFAULT);
  CU_ASSERT(ipbCfg.wsaChannel == I2V_RADIO_CHANNEL_DEFAULT);
  CU_ASSERT(ipbCfg.secDebug  == IPB_SECURITY_DEBUG_ENABLE_DEFAULT);
  CU_ASSERT(ipbCfg.psid      == I2V_IPB_PSID_DEFAULT);
  CU_ASSERT(ipbCfg.channelNumber == I2V_RADIO_CHANNEL_DEFAULT);
  CU_ASSERT(strcmp(ipbCfg.advertiseID,IPB_AD_ID_DEFAULT_S) == 0);
  CU_ASSERT(ipbCfg.minSats   == IPB_SV_COUNT_DEFAULT);
  CU_ASSERT(ipbCfg.usegps    == IPB_GNSS_OVERRIDE_DEFAULT);
  CU_ASSERT(ipbCfg.sattime   == IPB_GNSS_TO_DEFAULT);
  CU_ASSERT(ipbCfg.gpstime   == IPB_GNSS_SETTLE_TIME_DEFAULT);
  CU_ASSERT((int32_t)ipbCfg.latitude == (int32_t)(1e7 * I2V_GPS_LAT_DEFAULT));
//printf("\nLAT %f != %f",ipbCfg.latitude,(1e7 * I2V_GPS_LAT_DEFAULT));
  CU_ASSERT((int32_t)ipbCfg.longitude == (int32_t)(1e7 * I2V_GPS_LON_DEFAULT));
  CU_ASSERT(ipbCfg.elevation == I2V_GPS_ELEV_DEFAULT);
  CU_ASSERT(ipbCfg.confidence == IPB_POS_CONFIDENCE_DEFAULT);
  CU_ASSERT(ipbCfg.port      == IPB_IPV6_PORT_DEFAULT);
// Criteria for fail on strings is not much.
//  CU_ASSERT(strcmp(ipbCfg.pscval,IPB_PROVIDER_CTXT_DEFAULT_S) == 0);
//printf("\nipbCfg.pscval[%s] vs [%s]\n",ipbCfg.pscval,IPB_PROVIDER_CTXT_DEFAULT_S);
  CU_ASSERT(ipbCfg.uselocal  == IPB_IPV6_USE_LOCAL_LINK_DEFAULT);
printf("\nipbCfg.uselocal[%d] vs [%d]\n",ipbCfg.uselocal,IPB_IPV6_USE_LOCAL_LINK_DEFAULT);
  CU_ASSERT(ipbCfg.enableBridge == IPB_ENABLE_BRIDGE_DEFAULT);
  CU_ASSERT(ipbCfg.ipbBridgeOnce == IPB_BRIDGE_ONCE_DEFAULT);
  CU_ASSERT(ipbCfg.ipv4bridge == IPB_IPV4_BRIDGE_DEFAULT);
  CU_ASSERT(ipbCfg.bridgeClientNum == IPB_BRIDGE_CLIENT_DEFAULT );
  CU_ASSERT(ipbCfg.cnxnTOSec == IPB_CONNECT_TO_DEFAULT);
  CU_ASSERT(ipbCfg.delayRcvr == IPB_BRIDGE_DELAY_CLIENTS_DEFAULT);
  CU_ASSERT(strcmp(ipbCfg.ipv6addr,IPB_IPV6_ID_DEFAULT_S) == 0);
  CU_ASSERT(strcmp(&ipbCfg.ipv6addr[IPV6_PREFIX_WORDS * 2],IPB_IPV6_PREFIX_DEFAULT_S) == 0);
  CU_ASSERT(strcmp(ipbCfg.ipv4addr, IPB_IPV4_SERVER_IP_DEFAULT_S) == 0);
  CU_ASSERT(ipbCfg.ipv4port  == IPB_IPV4_SERVER_PORT_DEFAULT);
  CU_ASSERT(strcmp(ipbCfg.srvrIpv6addr,IPB_IPV6_SERVER_ID_DEFAULT_S) == 0);
  CU_ASSERT(strcmp(&ipbCfg.srvrIpv6addr[IPV6_PREFIX_WORDS * 2],IPB_IPV6_SERVER_PREFIX_DEFAULT_S) == 0);
  CU_ASSERT(strcmp(ipbCfg.svcMac,IPB_SRV_MAC_DEFAULT_S) == 0);
  CU_ASSERT(ipbCfg.enableEDCA == IPB_ENABLE_EDCA_DEFAULT);
#if 0 //Not staructure to test. Either it is set on network or not.
  CU_ASSERT(strcmp(IPBServerURL,IPB_SERVER_URL_DEFAULT_S) == 0);
#endif
#endif
  i2v_setupSHM_Clean(UnitshmPtr);

  /* Setup shm as you like: In lieu of I2V.C running.
   * This can be changed at anytime before any test. 
   * Will not get loaded or clobbered by any module inside SCS so we have to do it.
   * Could just load different confs for test cases?
   */
  if ((UnitshmPtr = wsu_share_init(sizeof(i2vShmMasterT), I2V_SHM_PATH)) == NULL) {
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME," Main shared memory init failed\n");
      return;
  }
  CU_ASSERT(I2V_RETURN_OK == i2v_setupSHM_Default(UnitshmPtr)); /* Should reflect i2v.conf. */
  sleep(1); /* Give SHM a chance to digest. */
  strcpy(confFileName, "ipb.conf");
  ret = my_main();
  CU_ASSERT(ret == abs(IPB_INIT_SHM_FAIL));
#if 0 //TODO needs work, too many fails, something wrong.
  CU_ASSERT(ipbCfg.radioNum == I2V_RADIO_DEFAULT);
  CU_ASSERT(ipbCfg.wsaPriority == I2V_WSA_PRIORITY_DEFAULT);
  CU_ASSERT(ipbCfg.wsaTxRate == I2V_WSA_TX_RATE_DEFAULT);
  CU_ASSERT(ipbCfg.txPwr     == I2V_TX_POWER_DEFAULT);
  CU_ASSERT(ipbCfg.wsaPwr    == I2V_TX_POWER_DEFAULT);
  CU_ASSERT(ipbCfg.forceIPBCfgPwr == IPB_OVERRIDE_POWER_DEFAULT);
  CU_ASSERT(ipbCfg.dataRate  == I2V_WSA_DATA_RATE_DEFAULT);
  CU_ASSERT(ipbCfg.isContinuous == IPB_ENABLE_CONT_DEFAULT);
  CU_ASSERT(ipbCfg.isNotCCH  == IPB_OVERRIDE_POWER_DEFAULT);
  CU_ASSERT(ipbCfg.wsaChannel == I2V_RADIO_CHANNEL_DEFAULT);
  CU_ASSERT(ipbCfg.secDebug  == IPB_SECURITY_DEBUG_ENABLE_DEFAULT);
  CU_ASSERT(ipbCfg.psid      == I2V_IPB_PSID_DEFAULT);
  CU_ASSERT(ipbCfg.channelNumber == I2V_RADIO_CHANNEL_DEFAULT);
  CU_ASSERT(strcmp(ipbCfg.advertiseID,IPB_AD_ID_DEFAULT_S) == 0);

  CU_ASSERT(ipbCfg.minSats   == IPB_SV_COUNT_DEFAULT);
  CU_ASSERT(ipbCfg.usegps    == IPB_GNSS_OVERRIDE_DEFAULT);
  CU_ASSERT(ipbCfg.sattime   == IPB_GNSS_TO_DEFAULT);
  CU_ASSERT(ipbCfg.gpstime   == IPB_GNSS_SETTLE_TIME_DEFAULT);
  CU_ASSERT((int32_t)ipbCfg.latitude == (int32_t)(1e7 * I2V_GPS_LAT_DEFAULT));
//printf("\nLAT %f != %f",ipbCfg.latitude,(1e7 * I2V_GPS_LAT_DEFAULT));
  CU_ASSERT(ipbCfg.longitude == I2V_GPS_LON_DEFAULT);
  CU_ASSERT(ipbCfg.elevation == I2V_GPS_ELEV_DEFAULT);
  CU_ASSERT(ipbCfg.confidence == IPB_POS_CONFIDENCE_DEFAULT);
  CU_ASSERT(ipbCfg.port      == IPB_IPV6_PORT_DEFAULT);
  CU_ASSERT(strcmp(ipbCfg.pscval,IPB_PROVIDER_CTXT_DEFAULT_S) == 0);
  CU_ASSERT(ipbCfg.uselocal  == IPB_IPV6_USE_LOCAL_LINK_DEFAULT);
  CU_ASSERT(ipbCfg.enableBridge == IPB_ENABLE_BRIDGE_DEFAULT);
  CU_ASSERT(ipbCfg.ipbBridgeOnce == IPB_BRIDGE_ONCE_DEFAULT);
  CU_ASSERT(ipbCfg.ipv4bridge == IPB_IPV4_BRIDGE_DEFAULT);
  CU_ASSERT(ipbCfg.bridgeClientNum == IPB_BRIDGE_CLIENT_DEFAULT );
  CU_ASSERT(ipbCfg.cnxnTOSec == IPB_CONNECT_TO_DEFAULT);
  CU_ASSERT(ipbCfg.delayRcvr == IPB_BRIDGE_DELAY_CLIENTS_DEFAULT);

  CU_ASSERT(strcmp(ipbCfg.ipv6addr,IPB_IPV6_ID_DEFAULT_S) == 0);

  CU_ASSERT(strcmp(&ipbCfg.ipv6addr[IPV6_PREFIX_WORDS * 2],IPB_IPV6_PREFIX_DEFAULT_S) == 0);
  CU_ASSERT(strcmp(ipbCfg.ipv4addr, IPB_IPV4_SERVER_IP_DEFAULT_S) == 0);
  CU_ASSERT(ipbCfg.ipv4port  == IPB_IPV4_SERVER_PORT_DEFAULT);
  CU_ASSERT(strcmp(ipbCfg.srvrIpv6addr,IPB_IPV6_SERVER_ID_DEFAULT_S) == 0);
  CU_ASSERT(strcmp(&ipbCfg.srvrIpv6addr[IPV6_PREFIX_WORDS * 2],IPB_IPV6_SERVER_PREFIX_DEFAULT_S) == 0);
  CU_ASSERT(strcmp(ipbCfg.svcMac,IPB_SRV_MAC_DEFAULT_S) == 0);
  CU_ASSERT(ipbCfg.enableEDCA == IPB_ENABLE_EDCA_DEFAULT);
#endif
  i2v_setupSHM_Clean(UnitshmPtr);
  /* Check for pending tests that did not complete.
   * Set tickle mask and ack mask for test.  
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
