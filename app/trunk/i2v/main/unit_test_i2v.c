
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
#define MY_NAME        "i2v_unit"
/* 
 * Config items that are stupid big. Parsing functions MUST fail these
 * because I2V_UTIL and I2V_SHM can NOT handle length greater than I2V_CFG_MAX_STR_LEN.
 */
#define STUPID_BIG_0       "0                                                                                                            "  /* End of STUPID. */
#define STUPID_BIG_1       "1                                                                                                            "  /* End of STUPID. */
#define STUPID_BIG_SPACE   "                                                                                                             "  /* End of STUPID. */
#define STUPID_BIG_PSID    "0x8002                                                                                                       "  /* End of STUPID. */
#define STUPID_BIG_CHANNEL "183                                                                                                          "  /* End of STUPID. */
#define STUPID_BIG_QWERTY  "qwerty                                                                                                       "  /* End of STUPID. */

/* PSID Tests. */
#define VALID_PSID_MAX  20
#define BAD_PSID_MAX  32

/* SSP & Mask tests. */

/* The good: */

#define VALID_SSP_STRING      "0080013040"
#define VALID_SSP_MASK_STRING "0080013040"

/* The bad: */

/* Empty. */
#define EMPTY_SSP_STRING      ""
#define EMPTY_SSP_MASK_STRING ""

#define BADCHAR_SSP_STRING      "0080@13040"
#define BADCHAR_SSP_MASK_STRING "0080@13040"

/* The ugly: */

/* 1 over MAX_SSP_DATA but under I2V_IP_MAX_STR_LEN. The sweet spot for string parsing failure. */
#define TOOBIG_SSP_STRING      "00800130400080013040008001300BADBEEF"
#define TOOBIG_SSP_MASK_STRING "00800130400080013040008001300BADBEEF"

/* debug only. */
#define PRINTIT   printf("\nret=%d.\n",ret);
/**************************************************************************************************
* Globals
***************************************************************************************************/

/* i2v.c */
extern WBOOL mainloop;
extern cfgItemsT cfg;
extern i2vUtilTimeParams dbgTime;
/**************************************************************************************************
* Protos & Externs
***************************************************************************************************/
/* i2v.c */
extern int my_main(int argc, char *argv[]);

/* i2v_util.c */
extern uint32_t i2vUtilUpdateFloatValue(void *configItem, char_t *val, char_t *min, char_t *max, char_t  **trange, uint32_t *status, i2vLogWrapper GCC_UNUSED_VAR *logger);
extern void i2vUtilEnableDebug(char_t *name);
extern void i2vUtilDisableDebug(void);
extern int32_t i2vUtilEnableSyslog(i2vLogSevT level, char_t *name);
extern void i2vUtilDisableSyslog(void);

/* test.c */
void test_main(void);
int init_suite(void);
int clean_suite(void);

/**************************************************************************************************
* Function Bodies
***************************************************************************************************/


#define VAL_FLOAT_STUPID_BIG "22.0                                                                                                                                  "
#define MIN_FLOAT_STUPID_BIG "0.0                                                                                                                                  "
#define MAX_FLOAT_STUPID_BIG "23.0                                                                                                                                  "

#define VAL_FLOAT_STUPID_BIG2 "                                                                                                                                  22.0"
#define MIN_FLOAT_STUPID_BIG2 "                                                                                                                                  0.0"
#define MAX_FLOAT_STUPID_BIG2 "                                                                                                                                  23.0"

/* 
 * void i2vUtilUpdateFloatValue(void *configItem, char_t *val, char_t *min, char_t *max,
 * char_t  **trange, uint32_t *status, i2vLogWrapper GCC_UNUSED_VAR *logger);
 */
void test_float32_t_update(void)
{
  
  uint32_t status = I2V_RETURN_OK;
  uint32_t ret = I2V_RETURN_OK;
  float32_t configItem = 0.0f;

  i2vUtilEnableDebug(MY_NAME);
  status = I2V_RETURN_OK;
  ret = i2vUtilUpdateFloatValue(&configItem,"22.0","0.0","23.0",NULL, &status, NULL);
  CU_ASSERT(I2V_RETURN_OK == status)
  CU_ASSERT(I2V_RETURN_OK == ret)
  i2vUtilDisableDebug();

  status = I2V_RETURN_OK;
  /* Dont include C notation for float. Parsing stops before endptr. */
  ret = i2vUtilUpdateFloatValue(&configItem,"22.0f","0.0f","23.0f",NULL, &status, NULL);
  CU_ASSERT(I2V_RETURN_RANGE_LIMIT_FAIL == status)
  CU_ASSERT(I2V_RETURN_RANGE_LIMIT_FAIL == ret)

  status = I2V_RETURN_OK;
  ret = i2vUtilUpdateFloatValue(&configItem,"22","0","23",NULL, &status, NULL);
  CU_ASSERT(I2V_RETURN_OK == status)
  CU_ASSERT(I2V_RETURN_OK == ret)

  /* Works because space not parsed and endptrs are eqaul. */
  status = I2V_RETURN_OK;
  ret = i2vUtilUpdateFloatValue(&configItem," 22.0","0.0","23.0",NULL, &status, NULL);
  CU_ASSERT(I2V_RETURN_OK == status)
  CU_ASSERT(I2V_RETURN_OK == ret)

  status = I2V_RETURN_OK;
  ret = i2vUtilUpdateFloatValue(&configItem,"22.0"," 0.0","23.0",NULL, &status, NULL);
  CU_ASSERT(I2V_RETURN_OK == status)
  CU_ASSERT(I2V_RETURN_OK == ret)

  status = I2V_RETURN_OK;
  ret = i2vUtilUpdateFloatValue(&configItem,"22.0","0.0"," 23.0",NULL, &status, NULL);
  CU_ASSERT(I2V_RETURN_OK == status)
  CU_ASSERT(I2V_RETURN_OK == ret)

  status = I2V_RETURN_OK;
  /* fails because trailing space is not parsed so endptr not the same. */
  ret = i2vUtilUpdateFloatValue(&configItem,"22.0 ","0.0","23.0",NULL, &status, NULL);
  CU_ASSERT(I2V_RETURN_OK == status)
  CU_ASSERT(I2V_RETURN_OK == ret)

  status = I2V_RETURN_OK;
  ret = i2vUtilUpdateFloatValue(&configItem,"22.0","0.0 ","23.0",NULL, &status, NULL);
  CU_ASSERT(I2V_RETURN_OK == status)
  CU_ASSERT(I2V_RETURN_OK == ret)

  status = I2V_RETURN_OK;
  ret = i2vUtilUpdateFloatValue(&configItem,"22.0","0.0","23.0 ",NULL, &status, NULL);
  CU_ASSERT(I2V_RETURN_OK == status)
  CU_ASSERT(I2V_RETURN_OK == ret)

  status = I2V_RETURN_OK;
  ret = i2vUtilUpdateFloatValue(&configItem," 22.0 "," 0.0 "," 23.0 ",NULL, &status, NULL);
  CU_ASSERT(I2V_RETURN_OK == status)
  CU_ASSERT(I2V_RETURN_OK == ret)

  status = I2V_RETURN_OK;
  ret = i2vUtilUpdateFloatValue(NULL,"22.0","0.0","23.0",NULL, &status, NULL);
  CU_ASSERT(I2V_RETURN_NULL_PTR == status)
  CU_ASSERT(I2V_RETURN_NULL_PTR == ret)

  status = I2V_RETURN_OK;
  ret = i2vUtilUpdateFloatValue(&configItem,NULL,"0.0","23.0",NULL, &status, NULL);
  CU_ASSERT(I2V_RETURN_NULL_PTR == status)
  CU_ASSERT(I2V_RETURN_NULL_PTR == ret)

  status = I2V_RETURN_OK;
  ret = i2vUtilUpdateFloatValue(&configItem,"22.0",NULL,"23.0",NULL, &status, NULL);
  CU_ASSERT(I2V_RETURN_OK == status)
  CU_ASSERT(I2V_RETURN_OK == ret)

  status = I2V_RETURN_OK;
  ret = i2vUtilUpdateFloatValue(&configItem,"22.0","0.0",NULL,NULL, &status, NULL);
  CU_ASSERT(I2V_RETURN_OK == status)
  CU_ASSERT(I2V_RETURN_OK == ret)

  status = I2V_RETURN_OK;
  ret = i2vUtilUpdateFloatValue(&configItem,"22.0","0.0","23.0",NULL, NULL, NULL);
  CU_ASSERT(I2V_RETURN_OK == status)
  CU_ASSERT(I2V_RETURN_NULL_PTR == ret)

  status = I2V_RETURN_OK;
  ret = i2vUtilUpdateFloatValue(&configItem,"","0.0","23.0",NULL, &status, NULL);
  CU_ASSERT(I2V_RETURN_RANGE_LIMIT_FAIL == status)
  CU_ASSERT(I2V_RETURN_RANGE_LIMIT_FAIL == ret)

  status = I2V_RETURN_OK;
  ret = i2vUtilUpdateFloatValue(&configItem,"22.0","","23.0",NULL, &status, NULL);
  CU_ASSERT(I2V_RETURN_RANGE_LIMIT_FAIL == status)
  CU_ASSERT(I2V_RETURN_RANGE_LIMIT_FAIL == ret)

  status = I2V_RETURN_OK;
  ret = i2vUtilUpdateFloatValue(&configItem,"22.0","0.0","",NULL, &status, NULL);
  CU_ASSERT(I2V_RETURN_RANGE_LIMIT_FAIL == status)
  CU_ASSERT(I2V_RETURN_RANGE_LIMIT_FAIL == ret)

  status = I2V_RETURN_OK;
  ret = i2vUtilUpdateFloatValue(&configItem,"22.0foo","0.0","23.0",NULL, &status, NULL);
  CU_ASSERT(I2V_RETURN_RANGE_LIMIT_FAIL == status)
  CU_ASSERT(I2V_RETURN_RANGE_LIMIT_FAIL == ret)

  status = I2V_RETURN_OK;
  ret = i2vUtilUpdateFloatValue(&configItem,"22.0","0.0foo","23.0",NULL, &status, NULL);
  CU_ASSERT(I2V_RETURN_RANGE_LIMIT_FAIL == status)
  CU_ASSERT(I2V_RETURN_RANGE_LIMIT_FAIL == ret)

  status = I2V_RETURN_OK;
  ret = i2vUtilUpdateFloatValue(&configItem,"22.0","0.0","23.0foo",NULL, &status, NULL);
  CU_ASSERT(I2V_RETURN_RANGE_LIMIT_FAIL == status)
  CU_ASSERT(I2V_RETURN_RANGE_LIMIT_FAIL == ret)

  status = I2V_RETURN_OK;
  ret = i2vUtilUpdateFloatValue(&configItem,"23.5","0.0","23.0",NULL, &status, NULL);
  CU_ASSERT(I2V_RETURN_MAX_LIMIT_FAIL == status)
  CU_ASSERT(I2V_RETURN_MAX_LIMIT_FAIL == ret)

  status = I2V_RETURN_OK;
  ret = i2vUtilUpdateFloatValue(&configItem,"-1.0","0.0","23.0",NULL, &status, NULL);
  CU_ASSERT(I2V_RETURN_MIN_LIMIT_FAIL == status)
  CU_ASSERT(I2V_RETURN_MIN_LIMIT_FAIL == ret)

  status = I2V_RETURN_OK;
  ret = i2vUtilUpdateFloatValue(&configItem,VAL_FLOAT_STUPID_BIG,"0.0","23.0",NULL, &status, NULL);
  CU_ASSERT(I2V_RETURN_RANGE_LIMIT_FAIL == status)
  CU_ASSERT(I2V_RETURN_RANGE_LIMIT_FAIL == ret)

  status = I2V_RETURN_OK;
  ret = i2vUtilUpdateFloatValue(&configItem,"22.0",MIN_FLOAT_STUPID_BIG,"23.0",NULL, &status, NULL);
  CU_ASSERT(I2V_RETURN_RANGE_LIMIT_FAIL == status)
  CU_ASSERT(I2V_RETURN_RANGE_LIMIT_FAIL == ret)

  status = I2V_RETURN_OK;
  ret = i2vUtilUpdateFloatValue(&configItem,"22.0","0.0",MAX_FLOAT_STUPID_BIG,NULL, &status, NULL);
  CU_ASSERT(I2V_RETURN_RANGE_LIMIT_FAIL == status)
  CU_ASSERT(I2V_RETURN_RANGE_LIMIT_FAIL == ret)

  status = I2V_RETURN_OK;
  ret = i2vUtilUpdateFloatValue(&configItem,VAL_FLOAT_STUPID_BIG2,"0.0","23.0",NULL, &status, NULL);
  CU_ASSERT(I2V_RETURN_RANGE_LIMIT_FAIL == status)
  CU_ASSERT(I2V_RETURN_RANGE_LIMIT_FAIL == ret)

  status = I2V_RETURN_OK;
  ret = i2vUtilUpdateFloatValue(&configItem,"22.0",MIN_FLOAT_STUPID_BIG2,"23.0",NULL, &status, NULL);
  CU_ASSERT(I2V_RETURN_RANGE_LIMIT_FAIL == status)
  CU_ASSERT(I2V_RETURN_RANGE_LIMIT_FAIL == ret)

  status = I2V_RETURN_OK;
  ret = i2vUtilUpdateFloatValue(&configItem,"22.0","0.0",MAX_FLOAT_STUPID_BIG2,NULL, &status, NULL);
  CU_ASSERT(I2V_RETURN_RANGE_LIMIT_FAIL == status)
  CU_ASSERT(I2V_RETURN_RANGE_LIMIT_FAIL == ret)

  status = I2V_RETURN_OK;
  ret = i2vUtilUpdateFloatValue(&configItem,"f22.0","0.0","23.0",NULL, &status, NULL);
  CU_ASSERT(I2V_RETURN_RANGE_LIMIT_FAIL == status)
  CU_ASSERT(I2V_RETURN_RANGE_LIMIT_FAIL == ret)

  status = I2V_RETURN_OK;
  ret = i2vUtilUpdateFloatValue(&configItem,"22.0","f0.0","23.0",NULL, &status, NULL);
  CU_ASSERT(I2V_RETURN_RANGE_LIMIT_FAIL == status)
  CU_ASSERT(I2V_RETURN_RANGE_LIMIT_FAIL == ret)

  status = I2V_RETURN_OK;
  ret = i2vUtilUpdateFloatValue(&configItem,"22.0","0.0","f23.0",NULL, &status, NULL);
  CU_ASSERT(I2V_RETURN_RANGE_LIMIT_FAIL == status)
  CU_ASSERT(I2V_RETURN_RANGE_LIMIT_FAIL == ret)

  status = I2V_RETURN_OK;
  ret = i2vUtilUpdateFloatValue(&configItem,"2f2.0","0.0","23.0",NULL, &status, NULL);
  CU_ASSERT(I2V_RETURN_RANGE_LIMIT_FAIL == status)
  CU_ASSERT(I2V_RETURN_RANGE_LIMIT_FAIL == ret)

  status = I2V_RETURN_OK;
  ret = i2vUtilUpdateFloatValue(&configItem,"22.0","0f.0","23.0",NULL, &status, NULL);
  CU_ASSERT(I2V_RETURN_RANGE_LIMIT_FAIL == status)
  CU_ASSERT(I2V_RETURN_RANGE_LIMIT_FAIL == ret)

  status = I2V_RETURN_OK;
  ret = i2vUtilUpdateFloatValue(&configItem,"22.0","0.0","2f3.0",NULL, &status, NULL);
  CU_ASSERT(I2V_RETURN_RANGE_LIMIT_FAIL == status)
  CU_ASSERT(I2V_RETURN_RANGE_LIMIT_FAIL == ret)

  status = I2V_RETURN_OK;
  ret = i2vUtilUpdateFloatValue(&configItem,"2 2.0","0.0","23.0",NULL, &status, NULL);
  CU_ASSERT(I2V_RETURN_RANGE_LIMIT_FAIL == status)
  CU_ASSERT(I2V_RETURN_RANGE_LIMIT_FAIL == ret)

  status = I2V_RETURN_OK;
  ret = i2vUtilUpdateFloatValue(&configItem,"22.0","0 .0","23.0",NULL, &status, NULL);
  CU_ASSERT(I2V_RETURN_RANGE_LIMIT_FAIL == status)
  CU_ASSERT(I2V_RETURN_RANGE_LIMIT_FAIL == ret)

  status = I2V_RETURN_OK;
  ret = i2vUtilUpdateFloatValue(&configItem,"22.0","0.0","2 3.0",NULL, &status, NULL);
  CU_ASSERT(I2V_RETURN_RANGE_LIMIT_FAIL == status)
  CU_ASSERT(I2V_RETURN_RANGE_LIMIT_FAIL == ret)

  status = I2V_RETURN_OK;
  ret = i2vUtilUpdateFloatValue(&configItem,"2.0 3.0","0.0","23.0",NULL, &status, NULL);
  CU_ASSERT(I2V_RETURN_RANGE_LIMIT_FAIL == status)
  CU_ASSERT(I2V_RETURN_RANGE_LIMIT_FAIL == ret)

  status = I2V_RETURN_OK;
  ret = i2vUtilUpdateFloatValue(&configItem,"22.0","0 0.0","23.0",NULL, &status, NULL);
  CU_ASSERT(I2V_RETURN_RANGE_LIMIT_FAIL == status)
  CU_ASSERT(I2V_RETURN_RANGE_LIMIT_FAIL == ret)

  ret = i2vUtilUpdateFloatValue(&configItem," ","0.0","23.0",NULL, &status, NULL);
  CU_ASSERT(I2V_RETURN_RANGE_LIMIT_FAIL == status)
  CU_ASSERT(I2V_RETURN_RANGE_LIMIT_FAIL == ret)

  i2vUtilEnableDebug(MY_NAME);
  status = I2V_RETURN_OK;
  ret = i2vUtilUpdateFloatValue(&configItem,"22.0"," ","23.0",NULL, &status, NULL);
  CU_ASSERT(I2V_RETURN_RANGE_LIMIT_FAIL == status)
  CU_ASSERT(I2V_RETURN_RANGE_LIMIT_FAIL == ret)
  i2vUtilDisableDebug();

  ret = i2vUtilUpdateFloatValue(&configItem,"22.0","0.0"," ",NULL, &status, NULL);
  CU_ASSERT(I2V_RETURN_RANGE_LIMIT_FAIL == status)
  CU_ASSERT(I2V_RETURN_RANGE_LIMIT_FAIL == ret)

  /* 
   * endptrs match but errno == ERANGE: Value is not a number: inf or nan. 
   * float32_t: val(22333464646464646464646464646464646464646464)(inf) strtof failed: endptr(0x412b64) != val(0x412b64) errno=34(Numerical result out of range).
   */
  status = I2V_RETURN_OK;
  ret = i2vUtilUpdateFloatValue(&configItem,"22333464646464646464646464646464646464646464","0.0","23.0",NULL, &status, NULL);
  CU_ASSERT(I2V_RETURN_RANGE_LIMIT_FAIL == status)
  CU_ASSERT(I2V_RETURN_RANGE_LIMIT_FAIL == ret)
}
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

void test_main(void)
{
  i2vReturnTypesT ret = I2V_RETURN_OK; 

  /* Check for pending tests that did not complete.
   * Set tickle mask and ack mask for test.  
   */
  CU_ASSERT(0x0 == check_stub_pending_signals());
  clear_all_stub_signal();

  /* Bad conf test. Should pass. */
  memset(&cfg, 0, sizeof(cfg));
  strcpy(cfg.config_directory, "../stubs/configs/");
  strcpy(cfg.config_filename, "i2v_bad.conf");
  ret = my_main(0,NULL);
  CU_ASSERT(cfg.i2vrsuID == I2VRSUID_DEFAULT);
  CU_ASSERT(cfg.scsAppEnable == I2V_SCS_ENABLE_DEFAULT); 
  CU_ASSERT(cfg.srmAppEnable == I2V_SRM_ENABLE_DEFAULT);
  CU_ASSERT(cfg.ipbAppEnable == I2V_IPB_ENABLE_DEFAULT);
  CU_ASSERT(cfg.amhAppEnable == I2V_AMH_ENABLE_DEFAULT);
  CU_ASSERT(cfg.iwmhAppEnable == I2V_IWMH_ENABLE_DEFAULT);
  CU_ASSERT(cfg.globalDebugFlag == I2V_GLOBAL_DEBUG_DEFAULT);
  CU_ASSERT(cfg.i2vDebugEnableFlag == I2V_DEBUG_DEFAULT);
  CU_ASSERT(cfg.enableGPSOverride == I2V_GPS_OVERRIDE_ENABLE_DEFAULT);
  CU_ASSERT(cfg.gpsOvrLat == I2V_GPS_LAT_DEFAULT);
  CU_ASSERT(cfg.gpsOvrLong == I2V_GPS_LON_DEFAULT);;
  CU_ASSERT(cfg.gpsOvrElev == I2V_GPS_ELEV_DEFAULT);
  CU_ASSERT(cfg.ethIPv6Enable == I2V_ENABLE_IPV6_DEFAULT);
  CU_ASSERT(cfg.ethIPv6Scope == I2V_ETH_IPV6_SCOPE_DEFAULT);
  CU_ASSERT(strcmp(cfg.i2vipv6addr,I2V_ETH_IPV6_ADDR_DEFAULT_S) == 0);
  CU_ASSERT(strcmp(cfg.i2vipv6gwaddr,I2V_IPV6_GATEWAY_DEFAULT_S) == 0);
//printf("ipv6 gateway: %s vs %s \n",cfg.i2vipv6gwaddr, I2V_IPV6_GATEWAY_DEFAULT_S);
  CU_ASSERT(cfg.txPwrLevel == I2V_TX_POWER_DEFAULT);
  CU_ASSERT(cfg.uchannel == I2V_RADIO_CHANNEL_DEFAULT);
  CU_ASSERT(cfg.uradioNum == I2V_RADIO_DEFAULT);
//inverted in i2v.c
  CU_ASSERT(!cfg.termOnChildExit == I2V_FORCE_ALIVE_DEFAULT);
  CU_ASSERT(cfg.wsaEnable == I2V_WSA_ENABLE_DEFAULT);
  CU_ASSERT(cfg.cchradio == I2V_CONTROL_RADIO_DEFAULT);
  CU_ASSERT(cfg.iwmhVerifyOD == I2V_RADIO_SEC_VERIFY_DEFAULT );
  CU_ASSERT(cfg.radio0wmh.radioEnable == I2V_RADIO0_ENABLE_DEFAULT);
  CU_ASSERT(cfg.radio0wmh.radioMode == I2V_RADIO0_MODE_DEFAULT);
  CU_ASSERT(cfg.radio0wmh.radioChannel == I2V_RADIO_CHANNEL_DEFAULT);
  CU_ASSERT(cfg.radio1wmh.radioEnable == I2V_RADIO1_ENABLE_DEFAULT);
  CU_ASSERT(cfg.radio1wmh.radioMode == I2V_RADIO1_MODE_DEFAULT);
  CU_ASSERT(cfg.radio1wmh.radioChannel == I2V_RADIO_CHANNEL_DEFAULT);
  CU_ASSERT(cfg.radio2wmh.radioEnable == I2V_RADIO2_ENABLE_DEFAULT);
  CU_ASSERT(cfg.radio2wmh.radioMode == I2V_RADIO2_MODE_DEFAULT);
  CU_ASSERT(cfg.radio2wmh.radioChannel == I2V_RADIO_CHANNEL_DEFAULT);
  CU_ASSERT(cfg.radio3wmh.radioEnable == I2V_RADIO3_ENABLE_DEFAULT);
  CU_ASSERT(cfg.radio3wmh.radioMode == I2V_RADIO3_MODE_DEFAULT);
  CU_ASSERT(cfg.radio3wmh.radioChannel == I2V_RADIO_CHANNEL_DEFAULT);
  CU_ASSERT(cfg.bcastLockStep == I2V_BROADCAST_UNIFIED_DEFAULT);
  CU_ASSERT(cfg.uwsaPriority == I2V_WSA_UNIFIED_PRIORITY_DEFAULT);
  CU_ASSERT(cfg.security == I2V_SECURITY_ENABLE_DEFAULT);
  CU_ASSERT(cfg.vodEnable == I2V_ENABLE_VOD_DEFAULT);
  CU_ASSERT(cfg.securityVtPMsgRateMs == I2V_SECURITY_VTP_MSG_RATE_DEFAULT);
  CU_ASSERT(cfg.secStartTO == I2V_SEC_TIME_BUFFER_DEFAULT);
  CU_ASSERT(cfg.certAttach == I2V_CERT_ATTACH_RATE_DEFAULT);
  CU_ASSERT(cfg.dfltCert == I2V_GEN_DEFAULT_CERTS_DEFAULT);
  CU_ASSERT(cfg.regenCert == I2V_AUTO_GEN_CERT_RESTART_DEFAULT);
  CU_ASSERT(cfg.secSpatPsid == I2V_SPAT_PSID_DEFAULT);
  CU_ASSERT(cfg.secMapPsid == I2V_MAP_PSID_DEFAULT);
  CU_ASSERT(cfg.secTimPsid == I2V_TIM_PSID_DEFAULT);
  CU_ASSERT(cfg.secIpbPsid == I2V_IPB_PSID_DEFAULT);
  CU_ASSERT(cfg.bsmPsidDer == I2V_BSM_PSID_DEFAULT);
  CU_ASSERT(cfg.bsmPsidPerA == I2V_BSM_PSID_DEFAULT);
  CU_ASSERT(cfg.bsmPsidPerU == I2V_BSM_PSID_DEFAULT);
  CU_ASSERT(cfg.nocflash ==  I2V_STORAGE_BYPASS_DEFAULT);
  CU_ASSERT(cfg.nosyslog == I2V_DISABLE_SYSLOG_DEFAULT);
  CU_ASSERT(cfg.limitconsoleprints == I2V_CONSOLE_CLEANUP_DEFAULT);
  CU_ASSERT(cfg.spatSspEnable == I2V_SPAT_SSP_ENABLE_DEFAULT);
  CU_ASSERT(cfg.spatBitmappedSspEnable == I2V_SPAT_BITMAPPED_SSP_ENABLE_DEFAULT);
  CU_ASSERT(strcmp(cfg.spatSsp,I2V_SPAT_SSP_STRING_DEFAULT_S) == 0);
  CU_ASSERT(strcmp(cfg.spatSspMask,I2V_SPAT_SSP_MASK_STRING_DEFAULT_S) == 0);
  CU_ASSERT(cfg.mapSspEnable == I2V_MAP_SSP_ENABLE_DEFAULT);
  CU_ASSERT(cfg.mapBitmappedSspEnable == I2V_MAP_BITMAPPED_SSP_ENABLE_DEFAULT);
  CU_ASSERT(strcmp(cfg.mapSsp,I2V_MAP_SSP_STRING_DEFAULT_S) == 0);
  CU_ASSERT(strcmp(cfg.mapSspMask,I2V_MAP_SSP_MASK_STRING_DEFAULT_S) == 0);
  CU_ASSERT(cfg.timSspEnable == I2V_TIM_SSP_ENABLE_DEFAULT);
  CU_ASSERT(cfg.timBitmappedSspEnable == I2V_TIM_BITMAPPED_SSP_ENABLE_DEFAULT);
  CU_ASSERT(strcmp(cfg.timSsp,I2V_TIM_SSP_STRING_DEFAULT_S) == 0);
  CU_ASSERT(strcmp(cfg.timSspMask,I2V_TIM_SSP_MASK_STRING_DEFAULT_S) == 0);
  CU_ASSERT(cfg.enableASCSNMP == I2V_SNMP_ENABLE_DEFAULT);
  CU_ASSERT(cfg.RadioType == I2V_RADIO_TYPE_DEFAULT);
  CU_ASSERT(cfg.i2vUseDeviceID == I2V_USE_DEVICE_ID_DEFAULT);
  CU_ASSERT(cfg.gpsCtlTx ==  I2V_USE_GPS_TX_CONTROL_DEFAULT);
  CU_ASSERT(cfg.bcastTimeShift == I2V_BROADCAST_TIMESHIFT_DEFAULT);
  CU_ASSERT(cfg.InterfaceLogMask == I2V_IFACE_LOG_MASK_DEFAULT);
  CU_ASSERT(strcmp(cfg.securityDir,I2V_SECURITY_DIR_DEFAULT_S) == 0);
  CU_ASSERT(cfg.cfgTimeMode == I2V_CONFIG_TIME_MODE_DEFAULT);
  CU_ASSERT(dbgTime.epochsecs == I2V_DEFAULT_UTC_SECONDS);
  CU_ASSERT(dbgTime.epochusecs == I2V_DEFAULT_UTC_USECONDS);

//73% coverage;

  /* Restore config. */
  memset(&cfg, 0, sizeof(cfg));
  strcpy(cfg.config_directory, I2V_CONF_DIR);
  strcpy(cfg.config_filename, I2V_CONF_FILE);

  /* Nominal test. Should pass if i2v.conf matches conf_table.h. */
  ret = my_main(0,NULL);
  CU_ASSERT(I2V_RETURN_OK == ret);
  CU_ASSERT(cfg.i2vrsuID == I2VRSUID_DEFAULT);
  CU_ASSERT(cfg.scsAppEnable == I2V_SCS_ENABLE_DEFAULT); 
  CU_ASSERT(cfg.srmAppEnable == I2V_SRM_ENABLE_DEFAULT);
  CU_ASSERT(cfg.ipbAppEnable == I2V_IPB_ENABLE_DEFAULT);
  CU_ASSERT(cfg.amhAppEnable == I2V_AMH_ENABLE_DEFAULT);
  CU_ASSERT(cfg.iwmhAppEnable == I2V_IWMH_ENABLE_DEFAULT);
  CU_ASSERT(cfg.globalDebugFlag == I2V_GLOBAL_DEBUG_DEFAULT);
  CU_ASSERT(cfg.i2vDebugEnableFlag == I2V_DEBUG_DEFAULT);
  CU_ASSERT(cfg.enableGPSOverride == I2V_GPS_OVERRIDE_ENABLE_DEFAULT);
  CU_ASSERT(cfg.gpsOvrLat == I2V_GPS_LAT_DEFAULT);
  CU_ASSERT(cfg.gpsOvrLong == I2V_GPS_LON_DEFAULT);;
  CU_ASSERT(cfg.gpsOvrElev == I2V_GPS_ELEV_DEFAULT);
  CU_ASSERT(cfg.ethIPv6Enable == I2V_ENABLE_IPV6_DEFAULT);
  CU_ASSERT(cfg.ethIPv6Scope == I2V_ETH_IPV6_SCOPE_DEFAULT);
  CU_ASSERT(strcmp(cfg.i2vipv6addr,I2V_ETH_IPV6_ADDR_DEFAULT_S) == 0);
  CU_ASSERT(strcmp(cfg.i2vipv6gwaddr,I2V_IPV6_GATEWAY_DEFAULT_S) == 0);
//printf("ipv6 gateway2: %s vs %s \n",cfg.i2vipv6gwaddr, I2V_IPV6_GATEWAY_DEFAULT_S);
  CU_ASSERT(cfg.txPwrLevel == I2V_TX_POWER_DEFAULT);
  CU_ASSERT(cfg.uchannel == I2V_RADIO_CHANNEL_DEFAULT);
  CU_ASSERT(cfg.uradioNum == I2V_RADIO_DEFAULT);
//inverted in i2v.c
  CU_ASSERT(!cfg.termOnChildExit == I2V_FORCE_ALIVE_DEFAULT);
  CU_ASSERT(cfg.wsaEnable == I2V_WSA_ENABLE_DEFAULT);
  CU_ASSERT(cfg.cchradio == I2V_CONTROL_RADIO_DEFAULT);
  CU_ASSERT(cfg.iwmhVerifyOD == I2V_RADIO_SEC_VERIFY_DEFAULT );
  CU_ASSERT(cfg.radio0wmh.radioEnable == I2V_RADIO0_ENABLE_DEFAULT);
  CU_ASSERT(cfg.radio0wmh.radioMode == I2V_RADIO0_MODE_DEFAULT);
  CU_ASSERT(cfg.radio0wmh.radioChannel == I2V_RADIO_CHANNEL_DEFAULT);
  CU_ASSERT(cfg.radio1wmh.radioEnable == I2V_RADIO1_ENABLE_DEFAULT);
  CU_ASSERT(cfg.radio1wmh.radioMode == I2V_RADIO1_MODE_DEFAULT);
  CU_ASSERT(cfg.radio1wmh.radioChannel == I2V_RADIO_CHANNEL_DEFAULT);
  CU_ASSERT(cfg.radio2wmh.radioEnable == I2V_RADIO2_ENABLE_DEFAULT);
  CU_ASSERT(cfg.radio2wmh.radioMode == I2V_RADIO2_MODE_DEFAULT);
  CU_ASSERT(cfg.radio2wmh.radioChannel == I2V_RADIO_CHANNEL_DEFAULT);
  CU_ASSERT(cfg.radio3wmh.radioEnable == I2V_RADIO3_ENABLE_DEFAULT);
  CU_ASSERT(cfg.radio3wmh.radioMode == I2V_RADIO3_MODE_DEFAULT);
  CU_ASSERT(cfg.radio3wmh.radioChannel == I2V_RADIO_CHANNEL_DEFAULT);
  CU_ASSERT(cfg.bcastLockStep == I2V_BROADCAST_UNIFIED_DEFAULT);
  CU_ASSERT(cfg.uwsaPriority == I2V_WSA_UNIFIED_PRIORITY_DEFAULT);
  CU_ASSERT(cfg.security == I2V_SECURITY_ENABLE_DEFAULT);
  CU_ASSERT(cfg.vodEnable == I2V_ENABLE_VOD_DEFAULT);
  CU_ASSERT(cfg.securityVtPMsgRateMs == I2V_SECURITY_VTP_MSG_RATE_DEFAULT);
  CU_ASSERT(cfg.secStartTO == I2V_SEC_TIME_BUFFER_DEFAULT);
  CU_ASSERT(cfg.certAttach == I2V_CERT_ATTACH_RATE_DEFAULT);
  CU_ASSERT(cfg.dfltCert == I2V_GEN_DEFAULT_CERTS_DEFAULT);
  CU_ASSERT(cfg.regenCert == I2V_AUTO_GEN_CERT_RESTART_DEFAULT);
  CU_ASSERT(cfg.secSpatPsid == I2V_SPAT_PSID_DEFAULT);
  CU_ASSERT(cfg.secMapPsid == I2V_MAP_PSID_DEFAULT);
  CU_ASSERT(cfg.secTimPsid == I2V_TIM_PSID_DEFAULT);
  CU_ASSERT(cfg.secIpbPsid == I2V_IPB_PSID_DEFAULT);
  CU_ASSERT(cfg.bsmPsidDer == I2V_BSM_PSID_DEFAULT);
  CU_ASSERT(cfg.bsmPsidPerA == I2V_BSM_PSID_DEFAULT);
  CU_ASSERT(cfg.bsmPsidPerU == I2V_BSM_PSID_DEFAULT);
  CU_ASSERT(cfg.nocflash ==  I2V_STORAGE_BYPASS_DEFAULT);
  CU_ASSERT(cfg.nosyslog == I2V_DISABLE_SYSLOG_DEFAULT);
  CU_ASSERT(cfg.limitconsoleprints == I2V_CONSOLE_CLEANUP_DEFAULT);
  CU_ASSERT(cfg.spatSspEnable == I2V_SPAT_SSP_ENABLE_DEFAULT);
  CU_ASSERT(cfg.spatBitmappedSspEnable == I2V_SPAT_BITMAPPED_SSP_ENABLE_DEFAULT);
  CU_ASSERT(strcmp(cfg.spatSsp,I2V_SPAT_SSP_STRING_DEFAULT_S) == 0);
  CU_ASSERT(strcmp(cfg.spatSspMask,I2V_SPAT_SSP_MASK_STRING_DEFAULT_S) == 0);
  CU_ASSERT(cfg.mapSspEnable == I2V_MAP_SSP_ENABLE_DEFAULT);
  CU_ASSERT(cfg.mapBitmappedSspEnable == I2V_MAP_BITMAPPED_SSP_ENABLE_DEFAULT);
  CU_ASSERT(strcmp(cfg.mapSsp,I2V_MAP_SSP_STRING_DEFAULT_S) == 0);
  CU_ASSERT(strcmp(cfg.mapSspMask,I2V_MAP_SSP_MASK_STRING_DEFAULT_S) == 0);
  CU_ASSERT(cfg.timSspEnable == I2V_TIM_SSP_ENABLE_DEFAULT);
  CU_ASSERT(cfg.timBitmappedSspEnable == I2V_TIM_BITMAPPED_SSP_ENABLE_DEFAULT);
  CU_ASSERT(strcmp(cfg.timSsp,I2V_TIM_SSP_STRING_DEFAULT_S) == 0);
  CU_ASSERT(strcmp(cfg.timSspMask,I2V_TIM_SSP_MASK_STRING_DEFAULT_S) == 0);
  CU_ASSERT(cfg.enableASCSNMP == I2V_SNMP_ENABLE_DEFAULT);
  CU_ASSERT(cfg.RadioType == I2V_RADIO_TYPE_DEFAULT);
  CU_ASSERT(cfg.i2vUseDeviceID == I2V_USE_DEVICE_ID_DEFAULT);
  CU_ASSERT(cfg.gpsCtlTx ==  I2V_USE_GPS_TX_CONTROL_DEFAULT);
  CU_ASSERT(cfg.bcastTimeShift == I2V_BROADCAST_TIMESHIFT_DEFAULT);
  CU_ASSERT(cfg.InterfaceLogMask == I2V_IFACE_LOG_MASK_DEFAULT);
  CU_ASSERT(strcmp(cfg.securityDir,I2V_SECURITY_DIR_DEFAULT_S) == 0);
  CU_ASSERT(cfg.cfgTimeMode == I2V_CONFIG_TIME_MODE_DEFAULT);
  // These will fail test because heal function uses I2V_DEFAULT_UTC_SECONDS rather than string. Not as issue.
#if 0
  CU_ASSERT(dbgTime.epochsecs == I2V_DEFAULT_UTC_SECONDS);
  CU_ASSERT(dbgTime.epochusecs == I2V_DEFAULT_UTC_USECONDS);
#endif

#if 0 //use large cap functions
  set_stub_signal(Signal_wsu_share_init);
  ret = my_main(0,NULL);
  CU_ASSERT(I2V_RETURN_SHM_FAIL == ret);
  CU_ASSERT(0x0 == check_stub_pending_signals());
#endif

  /* Really weird case where  0 < agrc but argv is NULL. */
  ret = my_main(1,NULL);
  CU_ASSERT(I2V_RETURN_NULL_PTR == ret);
  CU_ASSERT(0x0 == check_stub_pending_signals());

  /*
   * Test I2V_UTIL functions.
   */
  test_float32_t_update();

//73 % coverage

  /* Clear tickle mask for next test. 
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
