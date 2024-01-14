
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
#include "i2v_riscapture.h"
#include "stubs.h"
#include "rs.h"
#include "cfgmgrapi.h"
#include "conf_manager.h"
#include "conf_table.h"
#include "cfgmgr.h"

/**************************************************************************************************
* Defines
***************************************************************************************************/
/* mandatory logging defines */
#define MY_ERR_LEVEL   LEVEL_INFO    /* from i2v_util.h */
#define MY_NAME        "cfgmgr_unit"
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

/* conf_agent */
#define IAMTOOBIGTOKEN "1234567890123456789012345678901234567890123456789012345678901234567890     "
#define IAMMAXTOKEN    "1234567890123456789012345678901234567890123456789012345678901234" 

/* debug only. */
#define PRINTIT   printf("\nret=%d.\n",ret);
/**************************************************************************************************
* Globals
***************************************************************************************************/
/* conf_manager.c */
extern bool_t mainloop;
extern bool_t loop;


/* PSID Tests. */
char_t * valid_psid[VALID_PSID_MAX] = {"0x01", "0x0001", "0x000001", "0x00000001",
                                       "0x7f", "0x007f", "0x00007f", "0x0000007f",
                                               "0x8000", "0x008000", "0x00008000",
                                               "0xbfff", "0x00bfff", "0x0000bfff",
                                                         "0xc00000", "0x00c00000",
                                                         "0xdfffff", "0x00dfffff",
                                                                     "0xe0000000",
                                                                     "0xefffffff"};


char_t * bad_psid[BAD_PSID_MAX] = {"0x80", "0x0080", "0x000080", "0x00000080", 
                                   "0xff", "0x00ff", "0x0000ff", "0x000000ff", 
                                           "0x0100", "0x000100", "0x00000100", 
                                           "0x7fff", "0x007fff", "0x00007fff", 
                                           "0xc000", "0x00c000", "0x0000c000", 
                                           "0xffff", "0x00ffff", "0x0000ffff", 
                                                     "0x010000", "0x00010000", 
                                                     "0xbfffff", "0x00bfffff", 
                                                     "0xe00000", "0x00e00000", 
                                                     "0xffffff", "0x00ffffff", 
                                                                 "0x01000000", 
                                                                 "0xdfffffff", 
                                                                 "0xf0000000", 
                                                                 "0xffffffff"};
/**************************************************************************************************
* Protos & Externs
***************************************************************************************************/
/* conf_manager.c */
extern int32_t conf_manager_main(int32_t argc, char_t *argv[]);

/* conf_agent.c */
extern int32_t conf_agent_main(int32_t argc, char_t *argv[]);

/*cfgmgrapi.c*/
extern int32_t cfgCheckParam(char_t * group, char_t * param, char_t * value);
extern int32_t cfgCheckI2V(char_t * param, char_t * value);
extern int32_t cfgCheckAMH(char_t * param, char_t * value);
extern int32_t cfgCheckIPB(char_t * param, char_t * value);
extern int32_t cfgCheckRSUINFO(char_t * param, char_t * value);
extern int32_t cfgCheckSPAT(char_t * param, char_t * value);
extern int32_t cfgCheckSCS(char_t * param, char_t * value);
extern int32_t cfgCheckSRMRX(char_t * param, char_t * value);
extern int32_t cfgCheckNSCONFIG(char_t * param, char_t * value);
extern int32_t cfgCheckSNMPD(char_t * param, char_t * value);
extern int32_t cfgNetworkGetMode(int32_t isOBU);
extern int32_t cfgNetworkGetMask(int32_t isFECIP);
extern int32_t cfgNetworkSetMask(int32_t mask, int32_t isFECIP);

/*conf_manager.c */
extern void process_write_request(char_t* in_msg, char_t* ret_msg);
extern void process_read_request(char_t* in_msg, char_t* ret_msg);
extern void update_cache(char_t  *conf_group,  conf_element_t *conf_table, int32_t ecount);
extern void process_request_msg(char_t* in_msg, char_t* ret_msg);
extern void exit_handler(void);
extern void configSigHandler(int32_t __attribute__((unused)) sig);

extern void process_login_request(char_t* in_msg, char_t* ret_msg);
extern void process_logout_request(char_t* in_msg, char_t* ret_msg);
extern void process_factory_reset_request(char_t* in_msg, char_t* ret_msg);
extern void process_factory_reset_but_presrv_net_request(char_t* in_msg, char_t* ret_msg);
extern void process_session_chk_request(char_t* in_msg, char_t* ret_msg);
extern void process_info_msg(char_t* in_msg, char_t* ret_msg);
extern void process_response_msg(char_t* in_msg, char_t* ret_msg);

/* i2v_util.c */
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

/* Common test to make sure element doesn't accept something it shouldn't. */
int32_t basicElementTest(char_t * group, char_t * param)
{
  if(CFG_AOK == cfgCheckParam(group,param, "qwerty")) { /* Non numeric and not hex. */
      printf("\nbasicElementTest: grp(%s), param(%s): fail test 5.\n",group,param);
      return CFG_SET_VALUE_RANGE_ERROR;
  }
  if(CFG_AOK == cfgCheckParam(group,param, "")) { /* empty */
      printf("\nbasicElementTest: grp(%s), param(%s): fail test 6.\n",group,param);
      return CFG_SET_VALUE_RANGE_ERROR;
  }
  /* Still look kinda right but way too stupid long. */
  if(CFG_AOK == cfgCheckParam(group,param,STUPID_BIG_0)){
      printf("\nbasicElementTest: grp(%s), param(%s): fail test 7.\n",group,param);
      return CFG_SET_VALUE_RANGE_ERROR;
  }
  if(CFG_AOK == cfgCheckParam(group,param,STUPID_BIG_1)){
      printf("\nbasicElementTest: grp(%s), param(%s): fail test 8.\n",group,param);
      return CFG_SET_VALUE_RANGE_ERROR;
  }
  if(CFG_AOK == cfgCheckParam(group,param,STUPID_BIG_CHANNEL)){
      printf("\nbasicElementTest: grp(%s), param(%s): fail test 9.\n",group,param);
      return CFG_SET_VALUE_RANGE_ERROR;
  }
  if(CFG_AOK == cfgCheckParam(group,param,STUPID_BIG_QWERTY)){
      printf("\nbasicElementTest: grp(%s), param(%s): fail test 10.\n",group,param); 
      return CFG_SET_VALUE_RANGE_ERROR;
  }
  if(CFG_AOK == cfgCheckParam(group,param,STUPID_BIG_SPACE)){
      printf("\nbasicElementTest: grp(%s), param(%s): fail test 11.\n",group,param);
      return CFG_SET_VALUE_RANGE_ERROR;
  }
  if(CFG_AOK == cfgCheckParam(group,param,STUPID_BIG_PSID)){
      printf("\nbasicElementTest: grp(%s), param(%s): fail test 12.\n",group,param);
      return CFG_SET_VALUE_RANGE_ERROR;
  }
  if(CFG_AOK == cfgCheckParam(group,param, ".")) { /* Almost a float. */
      printf("\nbasicElementTest: grp(%s), param(%s): fail test 16.\n",group,param);
      return CFG_SET_VALUE_RANGE_ERROR;
  }
  if(CFG_AOK == cfgCheckParam(group,param, "Q")) { /* Not a hex char */
      printf("\nbasicElementTest: grp(%s), param(%s): fail test 17.\n",group,param);
      return CFG_SET_VALUE_RANGE_ERROR;
  }
  return CFG_AOK;
}

//TODO: may need test for special chars or non alphanumeric?
/* Every string element must FAIL these basic tests above normal range check. */
int32_t basicStringElementTest(char_t * group, char_t * param, bool_t empty_allowed)
{
  /* 
   * Never ok to be empty at this level. App may decide "" is ok but not here.
   * ie ipv6 addr may decide its ok, but IP for RSU heck no.
   */
  if((WFALSE==empty_allowed) && (CFG_AOK == cfgCheckParam(group,param, ""))) { /* empty */
      printf("\nbasicStringElementTest: grp(%s), param(%s): fail test 1.\n",group,param);
      return CFG_SET_VALUE_RANGE_ERROR;
  }
  if(CFG_AOK == cfgCheckParam(group,param," ")){
      printf("\nbasicStringElementTest: grp(%s), param(%s): fail test 2.\n",group,param);
      return CFG_SET_VALUE_RANGE_ERROR;
  }
  if(CFG_AOK == cfgCheckParam(group,param,"  ")){
      printf("\nbasicStringElementTest: grp(%s), param(%s): fail test 3.\n",group,param);
      return CFG_SET_VALUE_RANGE_ERROR;
  }
  /* Still look kinda right but way too stupid long. */
  if(CFG_AOK == cfgCheckParam(group,param,STUPID_BIG_0)){
      printf("\nbasicStringElementTest: grp(%s), param(%s): fail test 4.\n",group,param);
      return CFG_SET_VALUE_RANGE_ERROR;
  }
  if(CFG_AOK == cfgCheckParam(group,param,STUPID_BIG_1)){
      printf("\nbasicStringElementTest: grp(%s), param(%s): fail test 5.\n",group,param);
      return CFG_SET_VALUE_RANGE_ERROR;
  }
  if(CFG_AOK == cfgCheckParam(group,param,STUPID_BIG_CHANNEL)){
      printf("\nbasicStringElementTest: grp(%s), param(%s): fail test 6.\n",group,param);
      return CFG_SET_VALUE_RANGE_ERROR;
  }
  if(CFG_AOK == cfgCheckParam(group,param,STUPID_BIG_QWERTY)){
      printf("\nbasicStringElementTest: grp(%s), param(%s): fail test 7.\n",group,param);
      return CFG_SET_VALUE_RANGE_ERROR;
  }
  if(CFG_AOK == cfgCheckParam(group,param,STUPID_BIG_SPACE)){
      printf("\nbasicStringElementTest: grp(%s), param(%s): fail test 8.\n",group,param);
      return CFG_SET_VALUE_RANGE_ERROR;
  }
  if(CFG_AOK == cfgCheckParam(group,param,STUPID_BIG_PSID)){
      printf("\nbasicStringElementTest: grp(%s), param(%s): fail test 10.\n",group,param);
      return CFG_SET_VALUE_RANGE_ERROR;
  }
  return CFG_AOK;
}
int32_t enableElementTest(char_t * group, char_t * param)
{
  int32_t ret = CFG_AOK;

  /* Good values. Should not fail. */
  if(CFG_AOK != (ret = cfgCheckParam(group,param, "0"))){
      printf("\nenableElementTest: grp(%s), param(%s): fail test 1 ret=%d.\n",group,param,ret);
      return CFG_SET_VALUE_RANGE_ERROR;
  }
  if(CFG_AOK != (ret = cfgCheckParam(group,param, "1"))){
      printf("\nenableElementTest: grp(%s), param(%s): fail test 2 ret=%d.\n",group,param,ret);
      return CFG_SET_VALUE_RANGE_ERROR;
  }
  /* Out of Range. Should not be OK. */
  if(CFG_AOK == (ret = cfgCheckParam(group,param,"2"))){
      printf("\nenableElementTest: grp(%s), param(%s): fail test 3 ret=%d.\n",group,param,ret);
      return CFG_SET_VALUE_RANGE_ERROR;
  }
  /* Basic element test. Should not fail. */
  if(CFG_AOK != (ret = basicElementTest(group,param))){
      printf("\nenableElementTest: grp(%s), param(%s): fail test 4 ret=%d.\n",group,param,ret);
      return CFG_SET_VALUE_RANGE_ERROR;
  }
  return CFG_AOK;
}
int32_t txPowerElementTest(char_t * group, char_t * param)
{
  int32_t ret = CFG_AOK;

  /* Valid values. */
  if(CFG_AOK != (ret = cfgCheckParam(group,param, "0"))){
      printf("\ntxPowerElementTest: grp(%s), param(%s): fail test 1 ret=%d.\n",group,param,ret);
      return CFG_SET_VALUE_RANGE_ERROR;
  }
  if(CFG_AOK != (ret = cfgCheckParam(group,param, "23"))){
      printf("\ntxPowerElementTest: grp(%s), param(%s): fail test 2 ret=%d.\n",group,param,ret);
      return CFG_SET_VALUE_RANGE_ERROR;
  }
  if(CFG_AOK != (ret = cfgCheckParam(group,param, "0.5"))){
      printf("\ntxPowerElementTest: grp(%s), param(%s): fail test 3 ret=%d.\n",group,param,ret);
      return CFG_SET_VALUE_RANGE_ERROR;
  }
  if(CFG_AOK != (ret = cfgCheckParam(group,param, "0.50"))){
      printf("\ntxPowerElementTest: grp(%s), param(%s): fail test 4 ret=%d.\n",group,param,ret);
      return CFG_SET_VALUE_RANGE_ERROR;
  }
  if(CFG_AOK != (ret = cfgCheckParam(group,param, ".500"))){
      printf("\ntxPowerElementTest: grp(%s), param(%s): fail test 5 ret=%d.\n",group,param,ret);
      return CFG_SET_VALUE_RANGE_ERROR;
  }
  /* Out of Range.  */
  if(CFG_AOK == (ret = cfgCheckParam(group,param, "23.5"))){
      printf("\ntxPowerElementTest: grp(%s), param(%s): fail test 6 ret=%d.\n",group,param,ret);
      return CFG_SET_VALUE_RANGE_ERROR;
  }
  if(CFG_AOK == (ret = cfgCheckParam(group,param, "22.1"))){
      printf("\ntxPowerElementTest: grp(%s), param(%s): fail test 7 ret=%d.\n",group,param,ret);
      return CFG_SET_VALUE_RANGE_ERROR;
  }
  if(CFG_AOK == (ret = cfgCheckParam(group,param, "24"))){
      printf("\ntxPowerElementTest: grp(%s), param(%s): fail test 8 ret=%d.\n",group,param,ret);
      return CFG_SET_VALUE_RANGE_ERROR;
  }
  /* Illegal chars. '-', 'e', 'f' not allowed. */
  if(CFG_AOK == (ret = cfgCheckParam(group,param, "-.5f"))){
      printf("\ntxPowerElementTest: grp(%s), param(%s): fail test 9 ret=%d.\n",group,param,ret);
      return CFG_SET_VALUE_RANGE_ERROR;
  }
  if(CFG_AOK == (ret = cfgCheckParam(group,param, "-1"))){
      printf("\ntxPowerElementTest: grp(%s), param(%s): fail test 10 ret=%d.\n",group,param,ret);
      return CFG_SET_VALUE_RANGE_ERROR;
  }
  if(CFG_AOK == (ret = cfgCheckParam(group,param, "-"))){
      printf("\ntxPowerElementTest: grp(%s), param(%s): fail test 11 ret=%d.\n",group,param,ret);
      return CFG_SET_VALUE_RANGE_ERROR;
  }
  if(CFG_AOK != (ret = cfgCheckParam(group,param, "1.e1"))){
      printf("\ntxPowerElementTest: grp(%s), param(%s): fail test 12 ret=%d.\n",group,param,ret);
      return CFG_SET_VALUE_RANGE_ERROR;
  }
  if(CFG_AOK == (ret = cfgCheckParam(group,param, "1.e6"))){
      printf("\ntxPowerElementTest: grp(%s), param(%s): fail test 13 ret=%d.\n",group,param,ret);
      return CFG_SET_VALUE_RANGE_ERROR;
  }
  if(CFG_AOK != (ret = cfgCheckParam(group,param, "-0"))){
      printf("\ntxPowerElementTest: grp(%s), param(%s): fail test 14 ret=%d.\n",group,param,ret);
      return CFG_SET_VALUE_RANGE_ERROR;
  }
  if(CFG_AOK != (ret = cfgCheckParam(group,param, "-.0"))){
      printf("\ntxPowerElementTest: grp(%s), param(%s): fail test 15 ret=%d.\n",group,param,ret);
      return CFG_SET_VALUE_RANGE_ERROR;
  }
  if(CFG_AOK != (ret = cfgCheckParam(group,param, "-.00"))){
      printf("\ntxPowerElementTest: grp(%s), param(%s): fail test 16 ret=%d.\n",group,param,ret);
      return CFG_SET_VALUE_RANGE_ERROR;
  }
  if(CFG_AOK == (ret = cfgCheckParam(group,param, "-0f"))){
      printf("\ntxPowerElementTest: grp(%s), param(%s): fail test 17 ret=%d.\n",group,param,ret);
      return CFG_SET_VALUE_RANGE_ERROR;
  }
  if(CFG_AOK == (ret = cfgCheckParam(group,param, "-.0f"))){
      printf("\ntxPowerElementTest: grp(%s), param(%s): fail test 18 ret=%d.\n",group,param,ret);
      return CFG_SET_VALUE_RANGE_ERROR;
  }
  if(CFG_AOK == (ret = cfgCheckParam(group,param, "0f"))){
      printf("\ntxPowerElementTest: grp(%s), param(%s): fail test 19 ret=%d.\n",group,param,ret);
      return CFG_SET_VALUE_RANGE_ERROR;
  }
  if(CFG_AOK == (ret = cfgCheckParam(group,param, "23f"))){
      printf("\ntxPowerElementTest: grp(%s), param(%s): fail test 20 ret=%d.\n",group,param,ret);
      return CFG_SET_VALUE_RANGE_ERROR;
  }
  if(CFG_AOK == (ret = cfgCheckParam(group,param, "0.5f"))){
      printf("\ntxPowerElementTest: grp(%s), param(%s): fail test 21 ret=%d.\n",group,param,ret);
      return CFG_SET_VALUE_RANGE_ERROR;
  }
  if(CFG_AOK == (ret = cfgCheckParam(group,param, ".5f"))){
      printf("\ntxPowerElementTest: grp(%s), param(%s): fail test 22 ret=%d.\n",group,param,ret);
      return CFG_SET_VALUE_RANGE_ERROR;
  }
  /* Length errors. */
  if(CFG_AOK != (ret = cfgCheckParam(group,param, ".5000"))){
      printf("\ntxPowerElementTest: grp(%s), param(%s): fail test 23 ret=%d.\n",group,param,ret);
      return CFG_SET_VALUE_RANGE_ERROR;
  }
  if(CFG_AOK  != (ret = cfgCheckParam(group,param, "-.000"))){
      printf("\ntxPowerElementTest: grp(%s), param(%s): fail test 24 ret=%d.\n",group,param,ret);
      return CFG_SET_VALUE_RANGE_ERROR;
  }
  if(CFG_AOK  != (ret = cfgCheckParam(group,param, ".0000"))){
      printf("\ntxPowerElementTest: grp(%s), param(%s): fail test 25 ret=%d.\n",group,param,ret);
      return CFG_SET_VALUE_RANGE_ERROR;
  }
  if(CFG_AOK  != (ret = cfgCheckParam(group,param, "22.50"))){
      printf("\ntxPowerElementTest: grp(%s), param(%s): fail test 26 ret=%d.\n",group,param,ret);
      return CFG_SET_VALUE_RANGE_ERROR;
  }
  if(CFG_AOK != (ret = basicElementTest(group,param))){
      printf("\ntxPowerElementTest: grp(%s), param(%s): fail test 27 ret=%d.\n",group,param,ret);
      return CFG_SET_VALUE_RANGE_ERROR;
  }
  return ret;
}
int32_t latElementTest(char_t * group, char_t * param)
{
  int32_t ret = CFG_AOK;

  /* Valid values. */
  if(CFG_AOK != (ret = cfgCheckParam(group,param, "-90.0"))){
      printf("\nlatElementTest: grp(%s), param(%s): fail test 1 ret=%d.\n",group,param,ret);
      return CFG_SET_VALUE_RANGE_ERROR;
  }
  if(CFG_AOK != (ret = cfgCheckParam(group,param, "-90."))){
      printf("\nlatElementTest: grp(%s), param(%s): fail test 2 ret=%d.\n",group,param,ret);
      return CFG_SET_VALUE_RANGE_ERROR;
  }
  if(CFG_AOK != (ret = cfgCheckParam(group,param, "-90"))){
      printf("\nlatElementTest: grp(%s), param(%s): fail test 3 ret=%d.\n",group,param,ret);
      return CFG_SET_VALUE_RANGE_ERROR;
  }
  if(CFG_AOK != (ret = cfgCheckParam(group,param, "90.0"))){
      printf("\nlatElementTest: grp(%s), param(%s): fail test 4 ret=%d.\n",group,param,ret);
      return CFG_SET_VALUE_RANGE_ERROR;
  }
  if(CFG_AOK != (ret = cfgCheckParam(group,param, "90."))){
      printf("\nlatElementTest: grp(%s), param(%s): fail test 5 ret=%d.\n",group,param,ret);
      return CFG_SET_VALUE_RANGE_ERROR;
  }
  if(CFG_AOK != (ret = cfgCheckParam(group,param, "90"))){
      printf("\nlatElementTest: grp(%s), param(%s): fail test 6 ret=%d.\n",group,param,ret);
      return CFG_SET_VALUE_RANGE_ERROR;
  }
  if(CFG_AOK != (ret = cfgCheckParam(group,param, "0.0"))){
      printf("\nlatElementTest: grp(%s), param(%s): fail test 7 ret=%d.\n",group,param,ret);
      return CFG_SET_VALUE_RANGE_ERROR;
  }
  if(CFG_AOK != (ret = cfgCheckParam(group,param, "0."))){
      printf("\nlatElementTest: grp(%s), param(%s): fail test 8 ret=%d.\n",group,param,ret);
      return CFG_SET_VALUE_RANGE_ERROR;
  }
  if(CFG_AOK != (ret = cfgCheckParam(group,param, "0"))){
      printf("\nlatElementTest: grp(%s), param(%s): fail test 9 ret=%d.\n",group,param,ret);
      return CFG_SET_VALUE_RANGE_ERROR;
  }
  if(CFG_AOK != (ret = cfgCheckParam(group,param, "-0"))){
      printf("\nlatElementTest: grp(%s), param(%s): fail test 10 ret=%d.\n",group,param,ret);
      return CFG_SET_VALUE_RANGE_ERROR;
  }
  if(CFG_AOK != (ret = cfgCheckParam(group,param, "-.0"))){
      printf("\nlatElementTest: grp(%s), param(%s): fail test 11 ret=%d.\n",group,param,ret);
      return CFG_SET_VALUE_RANGE_ERROR;
  }
  if(CFG_AOK != (ret = cfgCheckParam(group,param, "-.00"))){
      printf("\nlatElementTest: grp(%s), param(%s): fail test 12 ret=%d.\n",group,param,ret);
      return CFG_SET_VALUE_RANGE_ERROR;
  }
  if(CFG_AOK != (ret = cfgCheckParam(group,param, "-90.0000000"))){
      printf("\nlatElementTest: grp(%s), param(%s): fail test 13 ret=%d.\n",group,param,ret);
      return CFG_SET_VALUE_RANGE_ERROR;
  }
  if(CFG_AOK != (ret = cfgCheckParam(group,param, "90.0000000"))){
      printf("\nlatElementTest: grp(%s), param(%s): fail test 14 ret=%d.\n",group,param,ret);
      return CFG_SET_VALUE_RANGE_ERROR;
  }
  /* Out of Range. */
  if(CFG_SET_VALUE_RANGE_ERROR != (ret = cfgCheckParam(group,param, "-90.0000001"))){
      printf("\nlatElementTest: grp(%s), param(%s): fail test 15 ret=%d.\n",group,param,ret);
      return CFG_SET_VALUE_RANGE_ERROR;
  }
  if(CFG_SET_VALUE_RANGE_ERROR != (ret = cfgCheckParam(group,param, "90.0000001"))){
      printf("\nlatElementTest: grp(%s), param(%s): fail test 16 ret=%d.\n",group,param,ret);
      return CFG_SET_VALUE_RANGE_ERROR;
  }

  if(CFG_AOK == (ret = cfgCheckParam(group,param, "-.5f"))){
      printf("\nlatElementTest: grp(%s), param(%s): fail test 17 ret=%d.\n",group,param,ret);
      return CFG_SET_VALUE_RANGE_ERROR;
  }
  if(CFG_SET_VALUE_RANGE_ERROR != (ret = cfgCheckParam(group,param, "-"))){
      printf("\nlatElementTest: grp(%s), param(%s): fail test 18 ret=%d.\n",group,param,ret);
      return CFG_SET_VALUE_RANGE_ERROR;
  }
  if(CFG_AOK != (ret = cfgCheckParam(group,param, "1.e1"))){
      printf("\nlatElementTest: grp(%s), param(%s): fail test 19 ret=%d.\n",group,param,ret);
      return CFG_SET_VALUE_RANGE_ERROR;
  }
  if(CFG_SET_VALUE_RANGE_ERROR != (ret = cfgCheckParam(group,param, "1.e6"))){
      printf("\nlatElementTest: grp(%s), param(%s): fail test 20 ret=%d.\n",group,param,ret);
      return CFG_SET_VALUE_RANGE_ERROR;
  }
  if(CFG_AOK == (ret = cfgCheckParam(group,param, "-0f"))){
      printf("\nlatElementTest: grp(%s), param(%s): fail test 21 ret=%d.\n",group,param,ret);
      return CFG_SET_VALUE_RANGE_ERROR;
  }
  if(CFG_AOK == (ret = cfgCheckParam(group,param, "-.0f"))){
      printf("\nlatElementTest: grp(%s), param(%s): fail test 22 ret=%d.\n",group,param,ret);
      return CFG_SET_VALUE_RANGE_ERROR;
  }
  if(CFG_AOK == (ret = cfgCheckParam(group,param, "0f"))){
      printf("\nlatElementTest: grp(%s), param(%s): fail test 23 ret=%d.\n",group,param,ret);
      return CFG_SET_VALUE_RANGE_ERROR;
  }
  if(CFG_AOK == (ret = cfgCheckParam(group,param, "23f"))){
      printf("\nlatElementTest: grp(%s), param(%s): fail test 24 ret=%d.\n",group,param,ret);
      return CFG_SET_VALUE_RANGE_ERROR;
  }
  if(CFG_AOK == (ret = cfgCheckParam(group,param, "0.5f"))){
      printf("\nlatElementTest: grp(%s), param(%s): fail test 25 ret=%d.\n",group,param,ret);
      return CFG_SET_VALUE_RANGE_ERROR;
  }
  if(CFG_AOK == (ret = cfgCheckParam(group,param, ".5f"))){
      printf("\nlatElementTest: grp(%s), param(%s): fail test 26 ret=%d.\n",group,param,ret);
      return CFG_SET_VALUE_RANGE_ERROR;
  }
  /* Length errors. */
  if(CFG_AOK != (ret = cfgCheckParam(group,param, "-90.00000000"))){
      printf("\nlatElementTest: grp(%s), param(%s): fail test 27 ret=%d.\n",group,param,ret);
      return CFG_SET_VALUE_RANGE_ERROR;
  }
  if(CFG_AOK != (ret = cfgCheckParam(group,param, "-.0000000000"))){
      printf("\nlatElementTest: grp(%s), param(%s): fail test 28 ret=%d.\n",group,param,ret);
      return CFG_SET_VALUE_RANGE_ERROR;
  }
  if(CFG_AOK != (ret = cfgCheckParam(group,param, ".00000000000"))){
      printf("\nlatElementTest: grp(%s), param(%s): fail test 29 ret=%d.\n",group,param,ret);
      return CFG_SET_VALUE_RANGE_ERROR;
  }
  if(CFG_AOK != (ret = cfgCheckParam(group,param, "22.500000000"))){
      printf("\nlatElementTest: grp(%s), param(%s): fail test 30 ret=%d.\n",group,param,ret);
      return CFG_SET_VALUE_RANGE_ERROR;
  }

  if(CFG_AOK != (ret = basicElementTest(group,param))){
      return CFG_SET_VALUE_RANGE_ERROR;
  }
  return CFG_AOK;
}
int32_t lonElementTest(char_t * group, char_t * param)
{
  int32_t ret = CFG_AOK;

  /* Valid values. */
  if(CFG_AOK != (ret = cfgCheckParam(group,param, "-180.0"))){
      return CFG_SET_VALUE_RANGE_ERROR;
  }
  if(CFG_AOK != (ret = cfgCheckParam(group,param, "-180."))){
      return CFG_SET_VALUE_RANGE_ERROR;
  }
  if(CFG_AOK != (ret = cfgCheckParam(group,param, "-180"))){
      return CFG_SET_VALUE_RANGE_ERROR;
  }
  if(CFG_AOK != (ret = cfgCheckParam(group,param, "180.0"))){
      return CFG_SET_VALUE_RANGE_ERROR;
  }
  if(CFG_AOK != (ret = cfgCheckParam(group,param, "180."))){
      return CFG_SET_VALUE_RANGE_ERROR;
  }
  if(CFG_AOK != (ret = cfgCheckParam(group,param, "180"))){
      return CFG_SET_VALUE_RANGE_ERROR;
  }
  if(CFG_AOK != (ret = cfgCheckParam(group,param, "0.0"))){
      return CFG_SET_VALUE_RANGE_ERROR;
  }
  if(CFG_AOK != (ret = cfgCheckParam(group,param, "0."))){
      return CFG_SET_VALUE_RANGE_ERROR;
  }
  if(CFG_AOK != (ret = cfgCheckParam(group,param, "0"))){
      return CFG_SET_VALUE_RANGE_ERROR;
  }
  if(CFG_AOK != (ret = cfgCheckParam(group,param, "-0"))){
      return CFG_SET_VALUE_RANGE_ERROR;
  }
  if(CFG_AOK != (ret = cfgCheckParam(group,param, "-.0"))){
      return CFG_SET_VALUE_RANGE_ERROR;
  }
  if(CFG_AOK != (ret = cfgCheckParam(group,param, "-.00"))){
      return CFG_SET_VALUE_RANGE_ERROR;
  }
  if(CFG_AOK != (ret = cfgCheckParam(group,param, "-180.0000000"))){
      return CFG_SET_VALUE_RANGE_ERROR;
  }
  if(CFG_AOK != (ret = cfgCheckParam(group,param, "180.0000000"))){
      return CFG_SET_VALUE_RANGE_ERROR;
  }
  /* Out of Range. */
  if(CFG_SET_VALUE_RANGE_ERROR != (ret = cfgCheckParam(group,param, "-180.0000001"))){
      return CFG_SET_VALUE_RANGE_ERROR;
  }
  if(CFG_SET_VALUE_RANGE_ERROR != (ret = cfgCheckParam(group,param, "180.0000001"))){
      return CFG_SET_VALUE_RANGE_ERROR;
  }
  /* Illegal chars. 'e', 'f' not allowed. */
  if(CFG_AOK == (ret = cfgCheckParam(group,param, "-.5f"))){
      return CFG_SET_VALUE_RANGE_ERROR;
  }
  if(CFG_AOK == (ret = cfgCheckParam(group,param, "-"))){
      return CFG_SET_VALUE_RANGE_ERROR;
  }
  if(CFG_AOK != (ret = cfgCheckParam(group,param, "1.e1"))){
      return CFG_SET_VALUE_RANGE_ERROR;
  }
  if(CFG_AOK == (ret = cfgCheckParam(group,param, "1.e6"))){
      return CFG_SET_VALUE_RANGE_ERROR;
  }
  if(CFG_AOK == (ret = cfgCheckParam(group,param, "-0f"))){
      return CFG_SET_VALUE_RANGE_ERROR;
  }
  if(CFG_AOK == (ret = cfgCheckParam(group,param, "-.0f"))){
      return CFG_SET_VALUE_RANGE_ERROR;
  }
  if(CFG_AOK == (ret = cfgCheckParam(group,param, "0f"))){
      return CFG_SET_VALUE_RANGE_ERROR;
  }
  if(CFG_AOK == (ret = cfgCheckParam(group,param, "23f"))){
      return CFG_SET_VALUE_RANGE_ERROR;
  }
  if(CFG_AOK == (ret = cfgCheckParam(group,param, "0.5f"))){
      return CFG_SET_VALUE_RANGE_ERROR;
  }
  if(CFG_AOK == (ret = cfgCheckParam(group,param, ".5f"))){
      return CFG_SET_VALUE_RANGE_ERROR;
  }
  /* Length errors. */
  if(CFG_AOK != (ret = cfgCheckParam(group,param, "-180.00000000"))){
      return CFG_SET_VALUE_RANGE_ERROR;
  }
  if(CFG_AOK != (ret = cfgCheckParam(group,param, "-.00000000000"))){
      return CFG_SET_VALUE_RANGE_ERROR;
  }
  if(CFG_AOK!= (ret = cfgCheckParam(group,param, ".000000000000"))){
      return CFG_SET_VALUE_RANGE_ERROR;
  }
  if(CFG_AOK != (ret = cfgCheckParam(group,param, "22.5000000000"))){
      return CFG_SET_VALUE_RANGE_ERROR;
  }

  if(CFG_AOK != (ret = basicElementTest(group,param))){
      return CFG_SET_VALUE_RANGE_ERROR;
  }
  return CFG_AOK;
}
int32_t elvElementTest(char_t * group, char_t * param)
{
  int32_t ret = CFG_AOK;

  /* Good values. */
  if(CFG_AOK != (ret = cfgCheckParam(group,param,"-100"))) {
      printf("\nenableElementTest: grp(%s), param(%s): fail test 1 ret=%d.\n",group,param,ret);
      return CFG_SET_VALUE_RANGE_ERROR;
  }    
  if(CFG_AOK != (ret = cfgCheckParam(group,param,"1500"))) {
      printf("\nenableElementTest: grp(%s), param(%s): fail test 2 ret=%d.\n",group,param,ret);
      return CFG_SET_VALUE_RANGE_ERROR;
  }
  /* Out of Range. */
  if(CFG_SET_VALUE_RANGE_ERROR != (ret = cfgCheckParam(group,param,"-101"))) {
      printf("\nenableElementTest: grp(%s), param(%s): fail test 3 ret=%d.\n",group,param,ret);
      return CFG_SET_VALUE_RANGE_ERROR;
  }
  if(CFG_SET_VALUE_RANGE_ERROR != (ret = cfgCheckParam(group,param,"1501"))) {
      printf("\nenableElementTest: grp(%s), param(%s): fail test 4 ret=%d.\n",group,param,ret);
      return CFG_SET_VALUE_RANGE_ERROR;
  }
  /* Illegal char. */
  if(CFG_SET_VALUE_RANGE_ERROR != (ret = cfgCheckParam(group,param,"A"))) {
      printf("\nenableElementTest: grp(%s), param(%s): fail test 5 ret=%d.\n",group,param,ret);
      return CFG_SET_VALUE_RANGE_ERROR;
  }
  if(CFG_SET_VALUE_RANGE_ERROR != (ret = cfgCheckParam(group,param,"-"))) {
      printf("\nenableElementTest: grp(%s), param(%s): fail test 6 ret=%d.\n",group,param,ret);
      return CFG_SET_VALUE_RANGE_ERROR;
  }
  if(CFG_SET_VALUE_RANGE_ERROR != (ret = cfgCheckParam(group,param,"1.0"))) {
      printf("\nenableElementTest: grp(%s), param(%s): fail test 7 ret=%d.\n",group,param,ret);
      return CFG_SET_VALUE_RANGE_ERROR;
  }
  if(CFG_SET_VALUE_RANGE_ERROR != (ret = cfgCheckParam(group,param,"1.e1"))) {
      printf("\nenableElementTest: grp(%s), param(%s): fail test 8 ret=%d.\n",group,param,ret);
      return CFG_SET_VALUE_RANGE_ERROR;
  }
  if(CFG_SET_VALUE_RANGE_ERROR != (ret = cfgCheckParam(group,param,"1f"))) {
      printf("\nenableElementTest: grp(%s), param(%s): fail test 9 ret=%d.\n",group,param,ret);
      return CFG_SET_VALUE_RANGE_ERROR;
  }
  /* Length test with valid value. */
  if(CFG_AOK != (ret = cfgCheckParam(group,param,"   64"))) {
      printf("\nenableElementTest: grp(%s), param(%s): fail test 10 ret=%d.\n",group,param,ret);
      return CFG_SET_VALUE_RANGE_ERROR;
  }
  if(CFG_AOK != (ret = cfgCheckParam(group,param,"64   "))) {
      printf("\nenableElementTest: grp(%s), param(%s): fail test 11 ret=%d.\n",group,param,ret);
      return CFG_SET_VALUE_RANGE_ERROR;
  }
  if(CFG_AOK != (ret = basicElementTest(group,param))){
      printf("\nenableElementTest: grp(%s), param(%s): fail test 12 ret=%d.\n",group,param,ret);
      return CFG_SET_VALUE_RANGE_ERROR;
  }
  return CFG_AOK;
}

int32_t channelElementTest(char_t * group, char_t * param)
{
  int32_t ret = CFG_AOK;

  /* Good values. Should not fail. */
  if(CFG_AOK != (ret = cfgCheckParam(group,param, "183"))){
      printf("\nchannelElementTest: grp(%s), param(%s): fail test 1 ret=%d.\n",group,param,ret);
      return CFG_SET_VALUE_RANGE_ERROR;
  }
  /* Out of Range. Should not be OK. */
  if(CFG_AOK == (ret = cfgCheckParam(group,param,"171"))){
      printf("\nchannelElementTest: grp(%s), param(%s): fail test 2 ret=%d.\n",group,param,ret);
      return CFG_SET_VALUE_RANGE_ERROR;
  }
  if(CFG_AOK == (ret = cfgCheckParam(group,param,"185"))){
      printf("\nchannelElementTest: grp(%s), param(%s): fail test 3 ret=%d.\n",group,param,ret);
      return CFG_SET_VALUE_RANGE_ERROR;
  }
  if(CFG_AOK == (ret = cfgCheckParam(group,param,"0"))){
      printf("\nchannelElementTest: grp(%s), param(%s): fail test 4 ret=%d.\n",group,param,ret);
      return CFG_SET_VALUE_RANGE_ERROR;
  }

  /* Illegal. Should not be OK. */
  if(CFG_AOK == (ret = cfgCheckParam(group,param," 83"))){
      printf("\nchannelElementTest: grp(%s), param(%s): fail test 5 ret=%d.\n",group,param,ret);
      return CFG_SET_VALUE_RANGE_ERROR;
  }
  if(CFG_AOK == (ret = cfgCheckParam(group,param,"83 "))){
      printf("\nchannelElementTest: grp(%s), param(%s): fail test 6 ret=%d.\n",group,param,ret);
      return CFG_SET_VALUE_RANGE_ERROR;
  }
  if(CFG_AOK == (ret = cfgCheckParam(group,param,"-83"))){
      printf("\nchannelElementTest: grp(%s), param(%s): fail test 7 ret=%d.\n",group,param,ret);
      return CFG_SET_VALUE_RANGE_ERROR;
  }

  //i2vUtilEnableDebug(MY_NAME);
  /* Length test with valid value. Should NOT be ok. */
  if(CFG_AOK != (ret = cfgCheckParam(group,param,"183 "))){
      printf("\nchannelElementTest: grp(%s), param(%s): fail test 8 ret=%d.\n",group,param,ret);
      return CFG_SET_VALUE_RANGE_ERROR;
  }
  //i2vUtilDisableDebug();
  if(CFG_AOK != (ret = cfgCheckParam(group,param," 183"))){
      printf("\nchannelElementTest: grp(%s), param(%s): fail test 9 ret=%d.\n",group,param,ret);
      return CFG_SET_VALUE_RANGE_ERROR;
  }
  /* Basic element test. Should not fail. */
  if(CFG_AOK != (ret = basicElementTest(group,param))){
      printf("\nchannelElementTest: grp(%s), param(%s): fail test 10 ret=%d.\n",group,param,ret);
      return CFG_SET_VALUE_RANGE_ERROR;
  }
  return CFG_AOK;
}

int32_t psidElementTest(char_t * group, char_t * param)
{
  int32_t ret = CFG_AOK;
  int32_t i = 0;

  /* Good values should not fail. */
  /* decimal */  
  if(CFG_AOK != (ret = cfgCheckParam(group,param,"32"))){
      printf("\npsidElementTest: grp(%s), param(%s): fail test 1 ret=%d.\n",group,param,ret);
      return CFG_SET_VALUE_RANGE_ERROR;
  }
  /* hex */
  for(i=0;i<VALID_PSID_MAX;i++){
      if(CFG_AOK != (ret = cfgCheckParam(group,param,valid_psid[i]))){
          printf("\npsidElementTest: grp(%s), param(%s): fail valid test %d ret=%d.\n",group,param,i,ret);
          return CFG_SET_VALUE_RANGE_ERROR;
      }
  }
  /* Bad values should fail. */
  for(i=0;i<BAD_PSID_MAX;i++){
      if(CFG_AOK == (ret = cfgCheckParam(group,param,bad_psid[i]))){
          printf("\npsidElementTest: grp(%s), param(%s): fail bad test %d ret=%d.\n",group,param,i,ret);
          return CFG_SET_VALUE_RANGE_ERROR;
      }
  }
  /* Basic element test. Should not fail. */
  if(CFG_AOK != (ret = basicElementTest(group,param))){
      printf("\npsidElementTest: grp(%s), param(%s): fail basic test ret=%d.\n",group,param,ret);
      return CFG_SET_VALUE_RANGE_ERROR;
  }
  return CFG_AOK;
}

/*
 * ===============================  
 * ============ AMH ==============
 * ===============================
 */
void test_amh_conf(void)
{
  int32_t ret = 0;

  /* Test NULL inputs */

/* RadioNum */

  /* Valid values. */
  ret = cfgCheckAMH("RadioNum", "0");
  CU_ASSERT(CFG_AOK == ret);

  /* Out of Range. */
  ret = cfgCheckAMH("RadioNum", "1");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);
  ret = cfgCheckAMH("RadioNum", "2");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);

  /* Illegal char. */
  ret = cfgCheckAMH("RadioNum", "A");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);
  ret = cfgCheckAMH("RadioNum", "-");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);

  /* Length test with valid value. */
  ret = cfgCheckAMH("RadioNum", " 0");
  CU_ASSERT(CFG_AOK == ret);
  ret = cfgCheckAMH("RadioNum", "0 ");
  CU_ASSERT(CFG_AOK == ret);

  ret = basicElementTest(AMH_GRP,"RadioNum");
  CU_ASSERT(CFG_AOK == ret);

/* ChannelNumber */

  ret = channelElementTest(AMH_GRP, "ChannelNumber");
  CU_ASSERT(CFG_AOK == ret);

/* AMHBroadcastDir */

  /* Good values. */
  ret = cfgCheckAMH("AMHBroadcastDir", AMH_ACTIVE_DIR);
  CU_ASSERT(CFG_AOK == ret);
  ret = cfgCheckAMH("AMHBroadcastDir", "/rwflash");
  CU_ASSERT(CFG_AOK == ret);

  /* Bad values. */
  ret = cfgCheckAMH("AMHBroadcastDir", "\\");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);
  ret = cfgCheckAMH("AMHBroadcastDir", " ");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);

  /* Can't be empty. */
  ret = basicStringElementTest(AMH_GRP, "AMHBroadcastDir",WFALSE);
  CU_ASSERT(CFG_AOK == ret);

/* EnableValidation */
  ret = enableElementTest(AMH_GRP, "EnableValidation");
  CU_ASSERT(CFG_AOK == ret);

/* EnableStoreAndRepeat */
  ret = enableElementTest(AMH_GRP, "EnableStoreAndRepeat");
  CU_ASSERT(CFG_AOK == ret);

/* MaxStoreMessages */
  /* Good values. */
  ret = cfgCheckAMH("MaxStoreMessages", "1");
  CU_ASSERT(CFG_AOK == ret);
  ret = cfgCheckAMH("MaxStoreMessages", "100");
  CU_ASSERT(CFG_AOK == ret);

  /* Out of Range. */
  ret = cfgCheckAMH("MaxStoreMessages", "101");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);
  ret = cfgCheckAMH("MaxStoreMessages", "-1");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);

  /* Basic element test. */
  ret = basicElementTest(AMH_GRP,"MaxStoreMessages");
  CU_ASSERT(CFG_AOK == ret);

/* SettingsOverride */
  ret = enableElementTest(AMH_GRP, "SettingsOverride");
  CU_ASSERT(CFG_AOK == ret);

/* ForcePSIDMatch */
  ret = enableElementTest(AMH_GRP, "ForcePSIDMatch");
  CU_ASSERT(CFG_AOK == ret);

/* SendOverride */
  ret = enableElementTest(AMH_GRP, "SendOverride");
  CU_ASSERT(CFG_AOK == ret);

/* WSMExtension */
  ret = enableElementTest(AMH_GRP, "WSMExtension");
  CU_ASSERT(CFG_AOK == ret);

/* ImmediateEnable */
  ret = enableElementTest(AMH_GRP, "ImmediateEnable");
  CU_ASSERT(CFG_AOK == ret);

/* ImmediateIPFilter */
  ret = enableElementTest(AMH_GRP, "ImmediateIPFilter");
  CU_ASSERT(CFG_AOK == ret);

/* ImmediateIP */

  /* Good values. */
  ret = cfgCheckAMH("ImmediateIP", "192.168.2.80");
  CU_ASSERT(CFG_AOK == ret);

  /* Bad values. */
  ret = cfgCheckAMH("ImmediateIP", "0::ABBA");
  CU_ASSERT(I2V_RETURN_RANGE_LIMIT_FAIL == ret);
  ret = cfgCheckAMH("ImmediateIP", "DEAD::0");
  CU_ASSERT(I2V_RETURN_RANGE_LIMIT_FAIL == ret);
  ret = cfgCheckAMH("ImmediateIP", "qwerty");
  CU_ASSERT(I2V_RETURN_RANGE_LIMIT_FAIL == ret);
  ret = cfgCheckAMH("ImmediateIP", "ABBA:BEEF:2.80");
  CU_ASSERT(I2V_RETURN_RANGE_LIMIT_FAIL == ret);

   /* Can be empty. */
   ret = basicStringElementTest(AMH_GRP, "ImmediateIP",WFALSE);
   CU_ASSERT(CFG_AOK == ret);

/* ImmediatePort */

  /* Good values. */
  ret = cfgCheckAMH("ImmediatePort", "0");
  CU_ASSERT(CFG_AOK == ret);
  ret = cfgCheckAMH("ImmediatePort", "65535");
  CU_ASSERT(CFG_AOK == ret);

  /* Out of Range. */
  ret = cfgCheckAMH("ImmediatePort", "-1");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);
  ret = cfgCheckAMH("ImmediatePort", "65536");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);

  /* Basic element test. */
  ret = basicElementTest(AMH_GRP,"ImmediatePort");
  CU_ASSERT(CFG_AOK == ret);
}
/*
 * ===============================  
 * ============ I2V ==============
 * ===============================
 */
void test_i2v_conf(void)
{
  int32_t ret = 0;

  /* Test NULL inputs */
  ret = cfgCheckI2V("I2VUnifiedChannelNumber",NULL);
  CU_ASSERT(CFG_PARAM_FAIL == ret);
  ret = cfgCheckI2V(NULL,"183");
  CU_ASSERT(CFG_PARAM_FAIL == ret);
  ret = cfgCheckI2V(NULL,NULL);
  CU_ASSERT(CFG_PARAM_FAIL == ret);

/* I2VRSUID. */

  /* Good values. */
  ret = cfgCheckI2V("I2VRSUID", "0");
  CU_ASSERT(CFG_AOK == ret);
  ret = cfgCheckI2V("I2VRSUID", "999");
  CU_ASSERT(CFG_AOK == ret);

  /* Out of Range. */
  ret = cfgCheckI2V("I2VRSUID", "1000");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);

  /* Basic element test. */
  ret = basicElementTest(I2V_GRP,"I2VRSUID");
  CU_ASSERT(CFG_AOK == ret);

/* I2VSCSAppEnable */

  ret = enableElementTest(I2V_GRP, "I2VSCSAppEnable");
  CU_ASSERT(CFG_AOK == ret);

/* I2VSRMAppEnable */

  ret = enableElementTest(I2V_GRP, "I2VSRMAppEnable");
  CU_ASSERT(CFG_AOK == ret);

/* I2VIPBAppEnable */

  ret = enableElementTest(I2V_GRP, "I2VIPBAppEnable");
  CU_ASSERT(CFG_AOK == ret);

/* I2VAMHAppEnable */

  ret = enableElementTest(I2V_GRP, "I2VAMHAppEnable");
  CU_ASSERT(CFG_AOK == ret);

/* I2VFWDMSGAppEnable */

  ret = enableElementTest(I2V_GRP, "I2VFWDMSGAppEnable");
  CU_ASSERT(CFG_AOK == ret);

/* I2VIWMHAppEnable */

  ret = enableElementTest(I2V_GRP, "I2VIWMHAppEnable");
  CU_ASSERT(CFG_AOK == ret);

/* I2VGlobalDebugFlag */

  ret = enableElementTest(I2V_GRP, "I2VGlobalDebugFlag");
  CU_ASSERT(CFG_AOK == ret);

/* I2VDebugFlag */

  ret = enableElementTest(I2V_GRP, "I2VDebugFlag");
  CU_ASSERT(CFG_AOK == ret);

/* I2VGPSOverrideEnable */

  ret = enableElementTest(I2V_GRP, "I2VGPSOverrideEnable");
  CU_ASSERT(CFG_AOK == ret);

/* I2VGPSOverrideLat */

  ret = latElementTest(I2V_GRP,"I2VGPSOverrideLat");
  CU_ASSERT(CFG_AOK == ret);

/* I2VGPSOverrideLong */

  ret = lonElementTest(I2V_GRP,"I2VGPSOverrideLong");
  CU_ASSERT(CFG_AOK == ret);

/* "I2VGPSOverrideElev" */

  ret = elvElementTest(I2V_GRP,"I2VGPSOverrideElev");
  CU_ASSERT(CFG_AOK == ret);

/* I2VEnableEthIPv6 */

  ret = enableElementTest(I2V_GRP, "I2VEnableEthIPv6");
  CU_ASSERT(CFG_AOK == ret);

/* I2VEthIPv6Addr */

  /* Good values. */
  ret = cfgCheckI2V("I2VEthIPv6Addr", "DEAD::0");
  CU_ASSERT(CFG_AOK == ret);
  ret = cfgCheckI2V("I2VEthIPv6Addr", "0::ABBA");
  CU_ASSERT(CFG_AOK == ret);

  /* Bad values. */
  ret = cfgCheckI2V("I2VEthIPv6Addr", "192.168.2.80");
  CU_ASSERT(I2V_RETURN_RANGE_LIMIT_FAIL == ret);
  ret = cfgCheckI2V("I2VEthIPv6Addr", "qwerty");
  CU_ASSERT(I2V_RETURN_RANGE_LIMIT_FAIL == ret);
  ret = cfgCheckI2V("I2VEthIPv6Addr", "ABBA:BEEF:2.80");
  CU_ASSERT(I2V_RETURN_RANGE_LIMIT_FAIL == ret);

   /* Can be empty. */
   ret = basicStringElementTest(I2V_GRP, "I2VEthIPv6Addr",WFALSE);
   CU_ASSERT(CFG_AOK == ret);

/* I2VEthIPv6Scope */

  /* Good values. */
  ret = cfgCheckI2V("I2VEthIPv6Scope", "1");
  CU_ASSERT(CFG_AOK == ret);
  ret = cfgCheckI2V("I2VEthIPv6Scope", "128");
  CU_ASSERT(CFG_AOK == ret);

  /* Out of Range. */
  ret = cfgCheckI2V("I2VEthIPv6Scope", "129");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);

  /* Illegal char. */
  ret = cfgCheckI2V("I2VEthIPv6Scope", "A");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);
  ret = cfgCheckI2V("I2VEthIPv6Scope", "-");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);

  /* Length test with valid value. */
  ret = cfgCheckI2V("I2VEthIPv6Scope", "  64");
  CU_ASSERT(CFG_AOK == ret);
  ret = cfgCheckI2V("I2VEthIPv6Scope", "64  ");
  CU_ASSERT(CFG_AOK == ret);

  /* Basic element test. */
  ret = basicElementTest(I2V_GRP,"I2VEthIPv6Scope");
  CU_ASSERT(CFG_AOK == ret);

/* I2VIPv6DefaultGateway */
  /* Good values. */
  ret = cfgCheckI2V("I2VIPv6DefaultGateway", "DEAD::0");
  CU_ASSERT(CFG_AOK == ret);
  ret = cfgCheckI2V("I2VIPv6DefaultGateway", "0::ABBA");
  CU_ASSERT(CFG_AOK == ret);

  /* Bad values. */
  ret = cfgCheckI2V("I2VIPv6DefaultGateway", "192.168.2.80");
  CU_ASSERT(I2V_RETURN_RANGE_LIMIT_FAIL == ret);
  ret = cfgCheckI2V("I2VIPv6DefaultGateway", "qwerty");
  CU_ASSERT(I2V_RETURN_RANGE_LIMIT_FAIL == ret);
  ret = cfgCheckI2V("I2VIPv6DefaultGateway", "ABBA:BEEF:2.80");
  CU_ASSERT(I2V_RETURN_RANGE_LIMIT_FAIL == ret);

   /* Can be empty. */
   ret = basicStringElementTest(I2V_GRP, "I2VIPv6DefaultGateway",WFALSE);
   CU_ASSERT(CFG_AOK == ret);

  /* I2VForceAlive. */

  ret = enableElementTest(I2V_GRP, "I2VForceAlive");
  CU_ASSERT(CFG_AOK == ret);

/* I2VControlRadio */

  /* Valid values. */
  ret = cfgCheckI2V("I2VControlRadio", "0");
  CU_ASSERT(CFG_AOK == ret);
  ret = cfgCheckI2V("I2VControlRadio", "1");
  CU_ASSERT(CFG_AOK == ret);
  ret = cfgCheckI2V("I2VControlRadio", "2");
  CU_ASSERT(CFG_AOK == ret);
  ret = cfgCheckI2V("I2VControlRadio", "3");
  CU_ASSERT(CFG_AOK == ret);

  /* Out of Range. */
  ret = cfgCheckI2V("I2VControlRadio", "4");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);

  /* Illegal char. */
  ret = cfgCheckI2V("I2VControlRadio", "A");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);
  ret = cfgCheckI2V("I2VControlRadio", "-");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);

  /* Length test with valid value. */
  ret = cfgCheckI2V("I2VControlRadio", " 3");
  CU_ASSERT(CFG_AOK == ret);
  ret = cfgCheckI2V("I2VControlRadio", "3 ");
  CU_ASSERT(CFG_AOK == ret);

  ret = basicElementTest(I2V_GRP,"I2VControlRadio");
  CU_ASSERT(CFG_AOK == ret);

/* I2VRadioSecVerify */

  ret = enableElementTest(I2V_GRP, "I2VRadioSecVerify");
  CU_ASSERT(CFG_AOK == ret);

  ret = enableElementTest(I2V_GRP, "I2VRadio0Enable");
  CU_ASSERT(CFG_AOK == ret);
  ret = enableElementTest(I2V_GRP, "I2VRadio0Mode");
  CU_ASSERT(CFG_AOK == ret);
  ret = channelElementTest(I2V_GRP, "I2VRadio0Channel");
  CU_ASSERT(CFG_AOK == ret);

  ret = enableElementTest(I2V_GRP, "I2VRadio1Enable");
  CU_ASSERT(CFG_AOK == ret);
  ret = enableElementTest(I2V_GRP, "I2VRadio1Mode");
  CU_ASSERT(CFG_AOK == ret);
  ret = channelElementTest(I2V_GRP, "I2VRadio1Channel");
  CU_ASSERT(CFG_AOK == ret);

  ret = enableElementTest(I2V_GRP, "I2VRadio2Enable");
  CU_ASSERT(CFG_AOK == ret);
  ret = enableElementTest(I2V_GRP, "I2VRadio2Mode");
  CU_ASSERT(CFG_AOK == ret);
  ret = channelElementTest(I2V_GRP, "I2VRadio2Channel");
  CU_ASSERT(CFG_AOK == ret);

  ret = enableElementTest(I2V_GRP, "I2VRadio3Enable");
  CU_ASSERT(CFG_AOK == ret);
  ret = enableElementTest(I2V_GRP, "I2VRadio3Mode");
  CU_ASSERT(CFG_AOK == ret);
  ret = channelElementTest(I2V_GRP, "I2VRadio3Channel");
  CU_ASSERT(CFG_AOK == ret);

  ret = enableElementTest(I2V_GRP, "I2VBroadcastUnified");
  CU_ASSERT(CFG_AOK == ret);

/* I2VUnifiedChannelNumber. */

  ret = channelElementTest(I2V_GRP, "I2VUnifiedChannelNumber");
  CU_ASSERT(CFG_AOK == ret);

/* I2VUnifiedRadioNumber */

  /* Valid values. */
  ret = cfgCheckI2V("I2VUnifiedRadioNumber", "0");
  CU_ASSERT(CFG_AOK == ret);

  /* Out of Range. */
  ret = cfgCheckI2V("I2VUnifiedRadioNumber", "1");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);
  ret = cfgCheckI2V("I2VUnifiedRadioNumber", "2");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);

  /* Illegal char. */
  ret = cfgCheckI2V("I2VUnifiedRadioNumber", "A");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);
  ret = cfgCheckI2V("I2VUnifiedRadioNumber", "-");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);

  /* Length test with valid value. */
  ret = cfgCheckI2V("I2VUnifiedRadioNumber", " 0");
  CU_ASSERT(CFG_AOK == ret);
  ret = cfgCheckI2V("I2VUnifiedRadioNumber", "0 ");
  CU_ASSERT(CFG_AOK == ret);

  ret = basicElementTest(I2V_GRP,"I2VUnifiedRadioNumber");
  CU_ASSERT(CFG_AOK == ret);

/* I2VUnifiedWSAPriority */

  /* Valid values. */
  ret = cfgCheckI2V("I2VUnifiedWSAPriority", "0");
  CU_ASSERT(CFG_AOK == ret);
  ret = cfgCheckI2V("I2VUnifiedWSAPriority", "62");
  CU_ASSERT(CFG_AOK == ret);

  /* Out of Range. */
  ret = cfgCheckI2V("I2VUnifiedWSAPriority", "63");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);

  /* Illegal char. */
  ret = cfgCheckI2V("I2VUnifiedWSAPriority", "A");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);
  ret = cfgCheckI2V("I2VUnifiedWSAPriority", "-");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);

  /* Length test with valid value. */
  ret = cfgCheckI2V("I2VUnifiedWSAPriority", "  3");
  CU_ASSERT(CFG_AOK == ret);
  ret = cfgCheckI2V("I2VUnifiedWSAPriority", "3  ");
  CU_ASSERT(CFG_AOK == ret);

  ret = basicElementTest(I2V_GRP,"I2VUnifiedWSAPriority");
  CU_ASSERT(CFG_AOK == ret);

/* "I2VSecurityEnable */
  ret = enableElementTest(I2V_GRP, "I2VSecurityEnable");
  CU_ASSERT(CFG_AOK == ret);

/* I2VTransmitPower */

  ret = txPowerElementTest(I2V_GRP,"I2VTransmitPower");
  CU_ASSERT(CFG_AOK == ret);
PRINTIT

/* I2VUnifiedWSAPriority */

  /* Valid values. */
  ret = cfgCheckI2V("I2VSecurityTimeBuffer", "0");
  CU_ASSERT(CFG_AOK == ret);
  ret = cfgCheckI2V("I2VSecurityTimeBuffer", "30");
  CU_ASSERT(CFG_AOK == ret);

  /* Out of Range. */
  ret = cfgCheckI2V("I2VSecurityTimeBuffer", "31");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);

  /* Illegal char. */
  ret = cfgCheckI2V("I2VSecurityTimeBuffer", "A");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);
  ret = cfgCheckI2V("I2VSecurityTimeBuffer", "-");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);

  /* Length test with valid value. */
  ret = cfgCheckI2V("I2VSecurityTimeBuffer", "  3");
  CU_ASSERT(CFG_AOK == ret);
  ret = cfgCheckI2V("I2VSecurityTimeBuffer", "3  ");
  CU_ASSERT(CFG_AOK == ret);

  ret = basicElementTest(I2V_GRP,"I2VSecurityTimeBuffer");
  CU_ASSERT(CFG_AOK == ret);

/* I2VCertAttachRate */

  /* Valid values. */
  ret = cfgCheckI2V("I2VCertAttachRate", "1");
  CU_ASSERT(CFG_AOK == ret);
  ret = cfgCheckI2V("I2VCertAttachRate", "100");
  CU_ASSERT(CFG_AOK == ret);

  /* Out of Range. */
  ret = cfgCheckI2V("I2VCertAttachRate", "101");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);

  /* Illegal char. */
  ret = cfgCheckI2V("I2VCertAttachRate", "A");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);
  ret = cfgCheckI2V("I2VCertAttachRate", "-1");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);

  /* Length test with valid value. */
  ret = cfgCheckI2V("I2VCertAttachRate", " 100");
  CU_ASSERT(CFG_AOK == ret);
  ret = cfgCheckI2V("I2VCertAttachRate", "100 ");
  CU_ASSERT(CFG_AOK == ret);

  ret = basicElementTest(I2V_GRP,"I2VCertAttachRate");
  CU_ASSERT(CFG_AOK == ret);

  ret = enableElementTest(I2V_GRP, "I2VGenDefaultCert");
  CU_ASSERT(CFG_AOK == ret);

  ret = enableElementTest(I2V_GRP, "I2VAutoGenCertRestart");
  CU_ASSERT(CFG_AOK == ret);

  ret = enableElementTest(I2V_GRP, "I2VStorageBypass");
  CU_ASSERT(CFG_AOK == ret);

  ret = enableElementTest(I2V_GRP, "I2VDisableSysLog");
  CU_ASSERT(CFG_AOK == ret);

  ret = enableElementTest(I2V_GRP, "I2VConsoleCleanup");
  CU_ASSERT(CFG_AOK == ret);

  ret = psidElementTest(I2V_GRP, "I2VMAPSecPSID");
  CU_ASSERT(CFG_AOK == ret);

  ret = psidElementTest(I2V_GRP, "I2VSPATSecPSID");
  CU_ASSERT(CFG_AOK == ret);

  ret = psidElementTest(I2V_GRP, "I2VTIMSecPSID");
  CU_ASSERT(CFG_AOK == ret);

  ret = psidElementTest(I2V_GRP, "I2VIPBSecPSID");
  CU_ASSERT(CFG_AOK == ret);

  ret = psidElementTest(I2V_GRP, "BSMRxPSIDDER");
  CU_ASSERT(CFG_AOK == ret);

  ret = psidElementTest(I2V_GRP, "BSMRxPSIDPERA");
  CU_ASSERT(CFG_AOK == ret);

  ret = psidElementTest(I2V_GRP, "BSMRxPSIDPERU");
  CU_ASSERT(CFG_AOK == ret);

  ret = enableElementTest(I2V_GRP, "I2VDirectASCSNMP");
  CU_ASSERT(CFG_AOK == ret);

  ret = enableElementTest(I2V_GRP, "I2VWSAEnable");
  CU_ASSERT(CFG_AOK == ret);

/* I2VRadioType */

  /* Valid values. */
  ret = cfgCheckI2V("I2VRadioType", "1");
  CU_ASSERT(CFG_AOK == ret);

  /* Out of Range. */
  ret = cfgCheckI2V("I2VRadioType", "0");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);
  ret = cfgCheckI2V("I2VRadioType", "2");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);

  /* Illegal char. */
  ret = cfgCheckI2V("I2VRadioType", "A");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);

  /* Length test with valid value. */
  ret = cfgCheckI2V("I2VRadioType", " 1");
  CU_ASSERT(CFG_AOK == ret);
  ret = cfgCheckI2V("I2VRadioType", "1 ");
  CU_ASSERT(CFG_AOK == ret);
  ret = cfgCheckI2V("I2VRadioType", "-1");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);

  ret = basicElementTest(I2V_GRP,"I2VRadioType");
  CU_ASSERT(CFG_AOK == ret);

/* SSP Related items */

/* SPAT SSP */
  ret = enableElementTest(I2V_GRP, "I2VSPATSSPEnable");
  CU_ASSERT(CFG_AOK == ret);

  ret = enableElementTest(I2V_GRP, "I2VSPATBITMAPPEDSSPEnable");
  CU_ASSERT(CFG_AOK == ret);

  /* Valid values. */
  ret = cfgCheckI2V("I2VSPATSspString", VALID_SSP_STRING );
  CU_ASSERT(CFG_AOK == ret);
  /* Out of Range. */
  ret = cfgCheckI2V("I2VSPATSspString", EMPTY_SSP_STRING);
  CU_ASSERT(CFG_ITEM_LENGTH_BAD == ret);
  /* Illegal char. */
  ret = cfgCheckI2V("I2VSPATSspString", BADCHAR_SSP_STRING);
  CU_ASSERT(CFG_AOK != ret);
  /* Length test with valid value. */
  ret = cfgCheckI2V("I2VSPATSspString", TOOBIG_SSP_STRING);
  CU_ASSERT(CFG_AOK != ret);
  ret = basicElementTest(I2V_GRP,"I2VSPATSspString");
  CU_ASSERT(CFG_AOK == ret);

  /* Valid values. */
  ret = cfgCheckI2V("I2VSPATSspMaskString", VALID_SSP_STRING );
  CU_ASSERT(CFG_AOK == ret);
  /* Out of Range. */
  ret = cfgCheckI2V("I2VSPATSspMaskString", EMPTY_SSP_STRING);
  CU_ASSERT(CFG_ITEM_LENGTH_BAD == ret);
  /* Illegal char. */
  ret = cfgCheckI2V("I2VSPATSspMaskString", BADCHAR_SSP_STRING);
  CU_ASSERT(CFG_AOK != ret);
  /* Length test with valid value. */
  ret = cfgCheckI2V("I2VSPATSspMaskString", TOOBIG_SSP_STRING);
  CU_ASSERT(CFG_AOK != ret);
  ret = basicElementTest(I2V_GRP,"I2VSPATSspMaskString");
  CU_ASSERT(CFG_AOK == ret);

/* MAP SSP */
  ret = enableElementTest(I2V_GRP, "I2VMAPSSPEnable");
  CU_ASSERT(CFG_AOK == ret);
  ret = enableElementTest(I2V_GRP, "I2VMAPBITMAPPEDSSPEnable");
  CU_ASSERT(CFG_AOK == ret);

  /* Valid values. */
  ret = cfgCheckI2V("I2VMAPSspString", VALID_SSP_STRING );
  CU_ASSERT(CFG_AOK == ret);
  /* Out of Range. */
  ret = cfgCheckI2V("I2VMAPSspString", EMPTY_SSP_STRING);
  CU_ASSERT(CFG_ITEM_LENGTH_BAD == ret);
  /* Illegal char. */
  ret = cfgCheckI2V("I2VMAPSspString", BADCHAR_SSP_STRING);
  CU_ASSERT(CFG_AOK != ret);
  /* Length test with valid value. */
  ret = cfgCheckI2V("I2VMAPSspString", TOOBIG_SSP_STRING);
  CU_ASSERT(CFG_AOK != ret);
  ret = basicElementTest(I2V_GRP,"I2VMAPSspString");
  CU_ASSERT(CFG_AOK == ret);

  /* Valid values. */
  ret = cfgCheckI2V("I2VMAPSspMaskString", VALID_SSP_STRING );
  CU_ASSERT(CFG_AOK == ret);
  /* Out of Range. */
  ret = cfgCheckI2V("I2VMAPSspMaskString", EMPTY_SSP_STRING);
  CU_ASSERT(CFG_ITEM_LENGTH_BAD == ret);
  /* Illegal char. */
  ret = cfgCheckI2V("I2VMAPSspMaskString", BADCHAR_SSP_STRING);
  CU_ASSERT(CFG_AOK != ret);
  /* Length test with valid value. */
  ret = cfgCheckI2V("I2VMAPSspMaskString", TOOBIG_SSP_STRING);
  CU_ASSERT(CFG_AOK != ret);
  ret = basicElementTest(I2V_GRP,"I2VMAPSspMaskString");
  CU_ASSERT(CFG_AOK == ret);

/* TIM SSP */

  ret = enableElementTest(I2V_GRP, "I2VTIMSSPEnable");
  CU_ASSERT(CFG_AOK == ret);
  ret = enableElementTest(I2V_GRP, "I2VTIMBITMAPPEDSSPEnable");
  CU_ASSERT(CFG_AOK == ret);

  /* Valid values. */
  ret = cfgCheckI2V("I2VTIMSspString", VALID_SSP_STRING );
  CU_ASSERT(CFG_AOK == ret);
  /* Out of Range. */
  ret = cfgCheckI2V("I2VTIMSspString", EMPTY_SSP_STRING);
  CU_ASSERT(CFG_ITEM_LENGTH_BAD == ret);
  /* Illegal char. */
  ret = cfgCheckI2V("I2VTIMSspString", BADCHAR_SSP_STRING);
  CU_ASSERT(CFG_AOK != ret);
  /* Length test with valid value. */
  ret = cfgCheckI2V("I2VTIMSspString", TOOBIG_SSP_STRING);
  CU_ASSERT(CFG_AOK != ret);
  ret = basicElementTest(I2V_GRP,"I2VTIMSspString");
  CU_ASSERT(CFG_AOK == ret);

  /* Valid values. */
  ret = cfgCheckI2V("I2VTIMSspMaskString", VALID_SSP_STRING);
  CU_ASSERT(CFG_AOK == ret);
  /* Out of Range. */
  ret = cfgCheckI2V("I2VTIMSspMaskString", EMPTY_SSP_STRING);
  CU_ASSERT(CFG_ITEM_LENGTH_BAD == ret);
  /* Illegal char. */
  ret = cfgCheckI2V("I2VTIMSspMaskString", BADCHAR_SSP_STRING);
  CU_ASSERT(CFG_AOK != ret);
  /* Length test with valid value. */
  ret = cfgCheckI2V("I2VTIMSspMaskString", TOOBIG_SSP_STRING);
  CU_ASSERT(CFG_AOK != ret);
  ret = basicElementTest(I2V_GRP,"I2VTIMSspMaskString");
  CU_ASSERT(CFG_AOK == ret);

  ret = enableElementTest(I2V_GRP, "I2VUseDeviceID");
  CU_ASSERT(CFG_AOK == ret);

  ret = enableElementTest(I2V_GRP, "I2VUseGPSTXControl");
  CU_ASSERT(CFG_AOK == ret);

  /* VOD Enable */
  ret = enableElementTest(I2V_GRP, "I2VEnableVOD");
  CU_ASSERT(CFG_AOK == ret);

  /* VtP Msg Rate */
  /* Good Values */
  ret = cfgCheckI2V("I2VSecurityVTPMsgRateMs","0" );
  CU_ASSERT(CFG_AOK == ret);
  ret = cfgCheckI2V("I2VSecurityVTPMsgRateMs","65535" );
  CU_ASSERT(CFG_AOK == ret);

  /* Bad Values */
  /* Out of Range & Length Bad same thing. */
  ret = cfgCheckI2V("I2VSecurityVTPMsgRateMs","6665535" );
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);


}

/*
 * ===============================  
 * ============ IPB ==============
 * ===============================
 */
void test_ipb_conf(void)
{
  int32_t ret = 0;

  /* Test NULL inputs */
  ret = cfgCheckIPB("RadioNum",NULL);
  CU_ASSERT(CFG_PARAM_FAIL == ret);
  ret = cfgCheckIPB(NULL,"0");
  CU_ASSERT(CFG_PARAM_FAIL == ret);
  ret = cfgCheckIPB(NULL,NULL);
  CU_ASSERT(CFG_PARAM_FAIL == ret);

/* RadioNum */

  /* Valid values. */
  ret = cfgCheckIPB("RadioNum", "0");
  CU_ASSERT(CFG_AOK == ret);

  /* Out of Range. */
  ret = cfgCheckIPB("RadioNum", "1");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);
  ret = cfgCheckIPB("RadioNum", "2");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);

  /* Illegal char. */
  ret = cfgCheckIPB("RadioNum", "A");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);
  ret = cfgCheckIPB("RadioNum", "-");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);

  /* Length test with valid value. */
  ret = cfgCheckIPB("RadioNum", " 0");
  CU_ASSERT(CFG_AOK == ret);
  ret = cfgCheckIPB("RadioNum", "0 ");
  CU_ASSERT(CFG_AOK == ret);

  ret = basicElementTest(IPB_GRP,"RadioNum");
  CU_ASSERT(CFG_AOK == ret);

/* WSAPriority */

  /* Valid values. */
  ret = cfgCheckIPB("WSAPriority", "0");
  CU_ASSERT(CFG_AOK == ret);
  ret = cfgCheckIPB("WSAPriority", "62");
  CU_ASSERT(CFG_AOK == ret);

  /* Out of Range. */
  ret = cfgCheckIPB("WSAPriority", "63");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);

  /* Illegal char. */
  ret = cfgCheckIPB("WSAPriority", "A");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);
  ret =cfgCheckIPB("WSAPriority", "-");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);

  /* Length test with valid value. */
  ret = cfgCheckIPB("WSAPriority", "  3");
  CU_ASSERT(CFG_AOK == ret);
  ret = cfgCheckIPB("WSAPriority", "3  ");
  CU_ASSERT(CFG_AOK == ret);

  ret = basicElementTest(IPB_GRP,"WSAPriority");
  CU_ASSERT(CFG_AOK == ret);

/* WSATxRate */

  /* Good values. */
  ret = cfgCheckIPB("WSATxRate", "0");
  CU_ASSERT(CFG_AOK == ret);
  ret = cfgCheckIPB("WSATxRate", "50");
  CU_ASSERT(CFG_AOK == ret);

  /* Out of Range. */
  ret = cfgCheckIPB("WSATxRate", "51");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);

  /* Basic element test. */
  ret = basicElementTest(IPB_GRP,"WSATxRate");
  CU_ASSERT(CFG_AOK == ret);

/* DataRate */

  /* Good values: 3, 6, 9, 12, 18, 24, 36, 48, 54 */
  ret = cfgCheckIPB("DataRate", "3");
  CU_ASSERT(CFG_AOK == ret);
  ret = cfgCheckIPB("DataRate", "6");
  CU_ASSERT(CFG_AOK == ret);
  ret = cfgCheckIPB("DataRate", "9");
  CU_ASSERT(CFG_AOK == ret);
  ret = cfgCheckIPB("DataRate", "12");
  CU_ASSERT(CFG_AOK == ret);
  ret = cfgCheckIPB("DataRate", "18");
  CU_ASSERT(CFG_AOK == ret);
  ret = cfgCheckIPB("DataRate", "24");
  CU_ASSERT(CFG_AOK == ret);
  ret = cfgCheckIPB("DataRate", "36");
  CU_ASSERT(CFG_AOK == ret);
  ret = cfgCheckIPB("DataRate", "48");
  CU_ASSERT(CFG_AOK == ret);
  ret = cfgCheckIPB("DataRate", "54");
  CU_ASSERT(CFG_AOK == ret);

  /* Out of Range. */
  ret = cfgCheckIPB("DataRate", "0");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);
  ret = cfgCheckIPB("DataRate", "5");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);
  ret = cfgCheckIPB("DataRate", "8");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);
  ret = cfgCheckIPB("DataRate", "11");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);
  ret = cfgCheckIPB("DataRate", "17");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);
  ret = cfgCheckIPB("DataRate", "23");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);
  ret = cfgCheckIPB("DataRate", "35");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);
  ret = cfgCheckIPB("DataRate", "49");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);
  ret = cfgCheckIPB("DataRate", "55");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);
  ret = cfgCheckIPB("DataRate", "100");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);

  /* Basic element test. */
  ret = basicElementTest(IPB_GRP,"DataRate");
  CU_ASSERT(CFG_AOK == ret);

/* TransmitPower */

  ret = txPowerElementTest(IPB_GRP,"TransmitPower");
  CU_ASSERT(CFG_AOK == ret);

/* WSAPower */

  ret = txPowerElementTest(IPB_GRP,"WSAPower");
  CU_ASSERT(CFG_AOK == ret);

/* OverridePower */

  ret = enableElementTest(IPB_GRP, "OverridePower");
  CU_ASSERT(CFG_AOK == ret);

/* EnableContinuous */                    

  ret = enableElementTest(IPB_GRP, "EnableContinuous");
  CU_ASSERT(CFG_AOK == ret);

/* OverrideCCH */

  ret = enableElementTest(IPB_GRP, "OverrideCCH");
  CU_ASSERT(CFG_AOK == ret);

/* CCHChannel */

  ret = channelElementTest(IPB_GRP, "CCHChannel");
  CU_ASSERT(CFG_AOK == ret);

/* SecurityDebugEnable */

  ret = enableElementTest(IPB_GRP, "SecurityDebugEnable");
  CU_ASSERT(CFG_AOK == ret);

/* IPBPSID */

  ret = psidElementTest(IPB_GRP, "IPBPSID");
  CU_ASSERT(CFG_AOK == ret);

/* ChannelNumber */

  ret = channelElementTest(IPB_GRP, "ChannelNumber");
  CU_ASSERT(CFG_AOK == ret);

/* IPBAdvertiseID */

//TODO: have api check for dir versus name

  /* Good values. */
  ret = cfgCheckIPB("IPBAdvertiseID", "DensoRsu");
  CU_ASSERT(CFG_AOK == ret);

//TODO: this should probably fail
  ret = cfgCheckIPB("IPBAdvertiseID","/rwflash");
  CU_ASSERT(CFG_AOK == ret);

  /* Bad values. */
  ret = cfgCheckIPB("IPBAdvertiseID", "\\");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);
  ret = cfgCheckIPB("IPBAdvertiseID", " ");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);

  /* Can be empty. */
  ret = basicStringElementTest(IPB_GRP, "IPBAdvertiseID", WTRUE);
  CU_ASSERT(CFG_AOK == ret);

/* IPBMinSatellites */

  /* Good values. */
  ret = cfgCheckIPB("IPBMinSatellites", "1");
  CU_ASSERT(CFG_AOK == ret);
  ret = cfgCheckIPB("IPBMinSatellites", "10");
  CU_ASSERT(CFG_AOK == ret);

  /* Out of Range. */
  ret = cfgCheckIPB("IPBMinSatellites", "0");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);
  ret = cfgCheckIPB("IPBMinSatellites", "11");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);

  /* Basic element test. */
  ret = basicElementTest(IPB_GRP,"IPBMinSatellites");
  CU_ASSERT(CFG_AOK == ret);

/* IPBSatelliteTO */ 

  /* Good values. */
  ret = cfgCheckIPB("IPBSatelliteTO", "1");
  CU_ASSERT(CFG_AOK == ret);
  ret = cfgCheckIPB("IPBSatelliteTO", "60");
  CU_ASSERT(CFG_AOK == ret);

  /* Out of Range. */
  ret = cfgCheckIPB("IPBSatelliteTO", "0");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);
  ret = cfgCheckIPB("IPBSatelliteTO", "61");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);

  /* Basic element test. */
  ret = basicElementTest(IPB_GRP,"IPBSatelliteTO");
  CU_ASSERT(CFG_AOK == ret);

/* IPBGPSOverride */

  ret = enableElementTest(IPB_GRP, "IPBGPSOverride");
  CU_ASSERT(CFG_AOK == ret);

/* IPBGPSSettleTime */

  /* Good values. */
  ret = cfgCheckIPB("IPBGPSSettleTime", "20");
  CU_ASSERT(CFG_AOK == ret);
  ret = cfgCheckIPB("IPBGPSSettleTime", "300");
  CU_ASSERT(CFG_AOK == ret);

  /* Out of Range. */
  ret = cfgCheckIPB("IPBGPSSettleTime", "19");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);
  ret = cfgCheckIPB("IPBGPSSettleTime", "301");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);

  /* Basic element test. */
  ret = basicElementTest(IPB_GRP,"IPBGPSSettleTime");
  CU_ASSERT(CFG_AOK == ret);

/* IPBEnableEDCA */

  ret = enableElementTest(IPB_GRP, "IPBEnableEDCA");
  CU_ASSERT(CFG_AOK == ret);

/* Elevation */

  ret = elvElementTest(IPB_GRP,"Elevation");
  CU_ASSERT(CFG_AOK == ret);

/* Latitude */

  ret = latElementTest(IPB_GRP,"Latitude");
  CU_ASSERT(CFG_AOK == ret);

/* Longitude */

  ret = lonElementTest(IPB_GRP,"Longitude");
  CU_ASSERT(CFG_AOK == ret);

/* PosConfidence */

  /* Good values. */
  ret = cfgCheckIPB("PosConfidence", "0");
  CU_ASSERT(CFG_AOK == ret);
  ret = cfgCheckIPB("PosConfidence", "0x0");
  CU_ASSERT(CFG_AOK == ret);
  ret = cfgCheckIPB("PosConfidence", "255");
  CU_ASSERT(CFG_AOK == ret);
  ret = cfgCheckIPB("PosConfidence", "0xFF");
  CU_ASSERT(CFG_AOK == ret);

  /* Out of Range. */
  ret = cfgCheckIPB("PosConfidence", "256");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);
  ret = cfgCheckIPB("PosConfidence", "0x100");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);

  /* Basic element test. */
  ret = basicElementTest(IPB_GRP,"PosConfidence");
  CU_ASSERT(CFG_AOK == ret);

/* IPBIPv6Prefix */

  /* Good values. */
  ret = cfgCheckIPB("IPBIPv6Prefix","2000:0:0:0");
  CU_ASSERT(CFG_AOK == ret);
  ret = cfgCheckIPB("IPBIPv6Prefix","0::ABBA");
  CU_ASSERT(CFG_AOK == ret);

  /* Bad values. */
  ret = cfgCheckIPB("IPBIPv6Prefix","192.168.2.80");
  CU_ASSERT(I2V_RETURN_RANGE_LIMIT_FAIL == ret);
  ret = cfgCheckIPB("IPBIPv6Prefix","qwerty");
  CU_ASSERT(I2V_RETURN_RANGE_LIMIT_FAIL == ret);
  ret = cfgCheckIPB("IPBIPv6Prefix","ABBA:BEEF:2.80");
  CU_ASSERT(I2V_RETURN_RANGE_LIMIT_FAIL == ret);

  /* Can't be empty. */
  ret = basicStringElementTest(IPB_GRP,"IPBIPv6Prefix",WFALSE);
  CU_ASSERT(CFG_AOK == ret);

//TODO: Should define and use defaults from conf_table.h

/* IPBIPv6Identifier    = 0:0:0:1*/

  /* Good values. */
  ret = cfgCheckIPB("IPBIPv6Identifier","0:0:0:1");
  CU_ASSERT(CFG_AOK == ret);
  ret = cfgCheckIPB("IPBIPv6Identifier","0::ABBA");
  CU_ASSERT(CFG_AOK == ret);

  /* Bad values. */
  ret = cfgCheckIPB("IPBIPv6Identifier","192.168.2.80");
  CU_ASSERT(I2V_RETURN_RANGE_LIMIT_FAIL == ret);
  ret = cfgCheckIPB("IPBIPv6Identifier","qwerty");
  CU_ASSERT(I2V_RETURN_RANGE_LIMIT_FAIL == ret);
  ret = cfgCheckIPB("IPBIPv6Identifier","ABBA:BEEF:2.80");
  CU_ASSERT(I2V_RETURN_RANGE_LIMIT_FAIL == ret);

  /* Can't be empty. */
  ret = basicStringElementTest(IPB_GRP,"IPBIPv6Identifier",WFALSE);
  CU_ASSERT(CFG_AOK == ret);

/* IPBIPv6Port */

  /* Good values. */
  ret = cfgCheckIPB("IPBIPv6Port", "0");
  CU_ASSERT(CFG_AOK == ret);
  ret = cfgCheckIPB("IPBIPv6Port", "65535");
  CU_ASSERT(CFG_AOK == ret);

  /* Out of Range. */
  ret = cfgCheckIPB("IPBIPv6Port", "65536");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);

  /* Basic element test. */
  ret = basicElementTest(IPB_GRP,"IPBIPv6Port");
  CU_ASSERT(CFG_AOK == ret);

/* IPv6ServerPrefix */

  /* Good values. */
  ret = cfgCheckIPB("IPv6ServerPrefix","2000:0:0:0");
  CU_ASSERT(CFG_AOK == ret);
  ret = cfgCheckIPB("IPv6ServerPrefix","0::ABBA");
  CU_ASSERT(CFG_AOK == ret);

  /* Bad values. */
  ret = cfgCheckIPB("IPv6ServerPrefix","192.168.2.80");
  CU_ASSERT(I2V_RETURN_RANGE_LIMIT_FAIL == ret);
  ret = cfgCheckIPB("IPv6ServerPrefix","qwerty");
  CU_ASSERT(I2V_RETURN_RANGE_LIMIT_FAIL == ret);
  ret = cfgCheckIPB("IPv6ServerPrefix","ABBA:BEEF:2.80");
  CU_ASSERT(I2V_RETURN_RANGE_LIMIT_FAIL == ret);

  /* Can't be empty. */
  ret = basicStringElementTest(IPB_GRP,"IPv6ServerPrefix",WFALSE);
  CU_ASSERT(CFG_AOK == ret);

/* IPv6ServerID */

  /* Good values. */
  ret = cfgCheckIPB("IPv6ServerID","0:0:0:10");
  CU_ASSERT(CFG_AOK == ret);
  ret = cfgCheckIPB("IPv6ServerID","0::ABBA");
  CU_ASSERT(CFG_AOK == ret);

  /* Bad values. */
  ret = cfgCheckIPB("IPv6ServerID","192.168.2.80");
  CU_ASSERT(I2V_RETURN_RANGE_LIMIT_FAIL == ret);
  ret = cfgCheckIPB("IPv6ServerID","qwerty");
  CU_ASSERT(I2V_RETURN_RANGE_LIMIT_FAIL == ret);
  ret = cfgCheckIPB("IPv6ServerID","ABBA:BEEF:2.80");
  CU_ASSERT(I2V_RETURN_RANGE_LIMIT_FAIL == ret);

  /* Can't be empty. */
  ret = basicStringElementTest(IPB_GRP,"IPv6ServerID",WFALSE);
  CU_ASSERT(CFG_AOK == ret);

/* IPBServiceMac */

  /* Good values. */
  ret = cfgCheckIPB("IPBServiceMac",IPB_SRV_MAC_DEFAULT_S);
  CU_ASSERT(CFG_AOK == ret);

  /* Bad values. */
  ret = cfgCheckIPB("IPBServiceMac","192.168.2.80");
  CU_ASSERT(CFG_AOK != ret);
  ret = cfgCheckIPB("IPBServiceMac","qwerty");
  CU_ASSERT(CFG_AOK != ret);
  ret = cfgCheckIPB("IPBServiceMac","ABBA:BEEF:2.80");
  CU_ASSERT(CFG_AOK != ret);

  /* Can't be empty. */
  ret = basicStringElementTest(IPB_GRP,"IPBServiceMac",WFALSE);
  CU_ASSERT(CFG_AOK == ret);

/* IPBProviderCtxt */

//TODO: have api check for dir versus name
  /* Good values. */
  ret = cfgCheckIPB("IPBProviderCtxt",IPB_PROVIDER_CTXT_DEFAULT_S);
  CU_ASSERT(CFG_AOK == ret);

  /* Bad values. */
  ret = cfgCheckIPB("IPBProviderCtxt", "\\");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);
  ret = cfgCheckIPB("IPBProviderCtxt", " ");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);
  ret = cfgCheckIPB("IPBProviderCtxt","/rwflash");
  CU_ASSERT(CFG_AOK == ret);

  /* Can't be empty. */
  ret = basicStringElementTest(IPB_GRP, "IPBProviderCtxt", WFALSE);
  CU_ASSERT(CFG_AOK == ret);

/* IPBIPv6UseLinkLocal */

  ret = enableElementTest(IPB_GRP, "IPBIPv6UseLinkLocal");
  CU_ASSERT(CFG_AOK == ret);

/* IPBEnableBridge */

  ret = enableElementTest(IPB_GRP, "IPBEnableBridge");
  CU_ASSERT(CFG_AOK == ret);

/* IPBBridgeOnce */

  ret = enableElementTest(IPB_GRP, "IPBBridgeOnce");
  CU_ASSERT(CFG_AOK == ret);

/* IPBIPv4Bridge */

  ret = enableElementTest(IPB_GRP, "IPBIPv4Bridge");
  CU_ASSERT(CFG_AOK == ret);

/* BridgeMaxClients */

  /* Good values. */
  ret = cfgCheckIPB("BridgeMaxClients", "1");
  CU_ASSERT(CFG_AOK == ret);
  ret = cfgCheckIPB("BridgeMaxClients", "5");
  CU_ASSERT(CFG_AOK == ret);

  /* Out of Range. */
  ret = cfgCheckIPB("BridgeMaxClients", "0");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);
  ret = cfgCheckIPB("BridgeMaxClients", "6");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);

  /* Bad length. */
  ret = cfgCheckIPB("BridgeMaxClients", "10");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);

  /* Basic element test. */
  ret = basicElementTest(IPB_GRP,"BridgeMaxClients");
  CU_ASSERT(CFG_AOK == ret);

/* ConnectionTimeout */

  /* Good values. */
  ret = cfgCheckIPB("ConnectionTimeout", "1");
  CU_ASSERT(CFG_AOK == ret);
  ret = cfgCheckIPB("ConnectionTimeout", "60");
  CU_ASSERT(CFG_AOK == ret);

  /* Out of Range. */
  ret = cfgCheckIPB("ConnectionTimeout", "0");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);
  ret = cfgCheckIPB("ConnectionTimeout", "61");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);

  /* Bad length. */
  ret = cfgCheckIPB("ConnectionTimeout", "100");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);

  /* Basic element test. */
  ret = basicElementTest(IPB_GRP,"ConnectionTimeout");
  CU_ASSERT(CFG_AOK == ret);

/* BridgeDelayClients */
  ret = enableElementTest(IPB_GRP, "BridgeDelayClients");
  CU_ASSERT(CFG_AOK == ret);

/* IPv4ServerIP */

  /* Good values. */
  ret = cfgCheckIPB("IPv4ServerIP", "192.168.2.230");
  CU_ASSERT(CFG_AOK == ret);

  /* Bad values. */
  ret = cfgCheckIPB("IPv4ServerIP", "0::ABBA");
  CU_ASSERT(I2V_RETURN_RANGE_LIMIT_FAIL == ret);
  ret = cfgCheckIPB("IPv4ServerIP", "DEAD::0");
  CU_ASSERT(I2V_RETURN_RANGE_LIMIT_FAIL == ret);
  ret = cfgCheckIPB("IPv4ServerIP", "qwerty");
  CU_ASSERT(I2V_RETURN_RANGE_LIMIT_FAIL == ret);
  ret = cfgCheckIPB("IPv4ServerIP", "ABBA:BEEF:2.80");
  CU_ASSERT(I2V_RETURN_RANGE_LIMIT_FAIL == ret);

   /* Can't be empty. */
   ret = basicStringElementTest(IPB_GRP, "IPv4ServerIP",WFALSE);
   CU_ASSERT(CFG_AOK == ret);

/* IPv4ServerPort */

  /* Good values. */
  ret = cfgCheckIPB("IPv4ServerPort", "0");
  CU_ASSERT(CFG_AOK == ret);
  ret = cfgCheckIPB("IPv4ServerPort", "65535");
  CU_ASSERT(CFG_AOK == ret);

  /* Out of Range. */
  ret = cfgCheckIPB("IPv4ServerPort", "65536");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);

  /* Basic element test. */
  ret = basicElementTest(IPB_GRP,"IPv4ServerPort");
  CU_ASSERT(CFG_AOK == ret);

/* IPBServerURL */

  ret = cfgCheckIPB("IPBServerURL","");
  CU_ASSERT(CFG_AOK == ret);
  ret = cfgCheckIPB("IPBServerURL","www.denso.com");
  CU_ASSERT(CFG_AOK == ret);

  /* Bad values. */
  ret = cfgCheckIPB("IPBServerURL", "\\");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);
  ret = cfgCheckIPB("IPBServerURL", " ");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);

  /* Can be empty. */
  ret = basicStringElementTest(IPB_GRP, "IPBServerURL", WTRUE);
  CU_ASSERT(CFG_AOK == ret);

}
/*
 * ===============================  
 * ============ RSUINFO ==========
 * ===============================
 */
//TODO: Irregular, most runtime. Test only potential conf values. The rest is not strictly needed here.
void test_rsuinfo_conf(void)
{
  int32_t ret = 0;

  /* Test NULL inputs */
  ret = cfgCheckRSUINFO("SC0IPADDRESS",NULL);
  CU_ASSERT(CFG_PARAM_FAIL == ret);
  ret = cfgCheckRSUINFO(NULL,"192.168.2.80");
  CU_ASSERT(CFG_PARAM_FAIL == ret);
  ret = cfgCheckRSUINFO(NULL,NULL);
  CU_ASSERT(CFG_PARAM_FAIL == ret);

/* SC0IPADDRESS */

  /* Good values. */
  ret = cfgCheckRSUINFO("SC0IPADDRESS", "192.168.2.80");
  CU_ASSERT(CFG_AOK == ret);

  /* Bad values. */
  ret = cfgCheckRSUINFO("SC0IPADDRESS", "0::ABBA");
  CU_ASSERT(I2V_RETURN_RANGE_LIMIT_FAIL == ret);
  ret = cfgCheckRSUINFO("SC0IPADDRESS", "DEAD::0");
  CU_ASSERT(I2V_RETURN_RANGE_LIMIT_FAIL == ret);
  ret = cfgCheckRSUINFO("SC0IPADDRESS", "qwerty");
  CU_ASSERT(I2V_RETURN_RANGE_LIMIT_FAIL == ret);
  ret = cfgCheckRSUINFO("SC0IPADDRESS", "ABBA:BEEF:2.80");
  CU_ASSERT(I2V_RETURN_RANGE_LIMIT_FAIL == ret);

  /* Can't be empty. */
  ret = basicStringElementTest(RSUINFO_GRP, "SC0IPADDRESS",WFALSE);
  CU_ASSERT(CFG_AOK == ret);

/* NETWORK_MODE */

  ret = enableElementTest(RSUINFO_GRP, "NETWORK_MODE"); /* static="0",dhcp="1" */
  CU_ASSERT(CFG_AOK == ret);

/* NETMASK */

  /* Good values. */
  ret = cfgCheckRSUINFO("NETMASK", "1");
  CU_ASSERT(CFG_AOK == ret);
  ret = cfgCheckRSUINFO("NETMASK", "31");
  CU_ASSERT(CFG_AOK == ret);

  /* Out of Range. */
  ret = cfgCheckRSUINFO("NETMASK", "0");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);
  ret = cfgCheckRSUINFO("NETMASK", "32");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);

  /* Bad length. */
  ret = cfgCheckRSUINFO("NETMASK", "1000");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);

  /* Basic element test. */
  ret = basicElementTest(RSUINFO_GRP,"NETMASK");
  CU_ASSERT(CFG_AOK == ret);

  ret = enableElementTest(RSUINFO_GRP, "I2V_APP_ENABLE");
  CU_ASSERT(CFG_AOK == ret);
  ret = enableElementTest(RSUINFO_GRP, "RSU_HEALTH_ENABLE");
  CU_ASSERT(CFG_AOK == ret);
#if 0
  ret = enableElementTest(RSUINFO_GRP, "RSU_SYSTEM_MONITOR_ENABLE");
  CU_ASSERT(CFG_AOK == ret);
#endif

/* WEBGUI_HTTP_SERVICE_PORT */

  /* Good values. */
  ret = cfgCheckRSUINFO("WEBGUI_HTTP_SERVICE_PORT", "0");
  CU_ASSERT(CFG_AOK == ret);
  ret = cfgCheckRSUINFO("WEBGUI_HTTP_SERVICE_PORT", "65535");
  CU_ASSERT(CFG_AOK == ret);

  /* Out of Range. */
  ret = cfgCheckRSUINFO("WEBGUI_HTTP_SERVICE_PORT", "65536");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);

  /* Basic element test. */
  ret = basicElementTest(RSUINFO_GRP,"WEBGUI_HTTP_SERVICE_PORT");
  CU_ASSERT(CFG_AOK == ret);

/* WEBGUI_HTTPS_SERVICE_PORT */

  /* Good values. */
  ret = cfgCheckRSUINFO("WEBGUI_HTTPS_SERVICE_PORT", "0");
  CU_ASSERT(CFG_AOK == ret);
  ret = cfgCheckRSUINFO("WEBGUI_HTTPS_SERVICE_PORT", "65535");
  CU_ASSERT(CFG_AOK == ret);

  /* Out of Range. */
  ret = cfgCheckRSUINFO("WEBGUI_HTTPS_SERVICE_PORT", "65536");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);

  /* Basic element test. */
  ret = basicElementTest(RSUINFO_GRP,"WEBGUI_HTTPS_SERVICE_PORT");
  CU_ASSERT(CFG_AOK == ret);
}
/*
 * ===============================  
 * ============ SPAT =============
 * ===============================
 */

void test_spat_conf(void)
{
  int32_t ret = 0;

  /* Test NULL inputs */
  ret = cfgCheckSPAT("RadioCfg",NULL);
  CU_ASSERT(CFG_PARAM_FAIL == ret);
  ret = cfgCheckSPAT(NULL,"0");
  CU_ASSERT(CFG_PARAM_FAIL == ret);
  ret = cfgCheckSPAT(NULL,NULL);
  CU_ASSERT(CFG_PARAM_FAIL == ret);

/* RadioCfg */

  /* Valid values. */
  ret = cfgCheckSPAT("RadioCfg", "0");
  CU_ASSERT(CFG_AOK == ret);

  /* Out of Range. */
  ret = cfgCheckSPAT("RadioCfg", "1");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);
  ret = cfgCheckSPAT("RadioCfg", "2");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);

  /* Illegal char. */
  ret = cfgCheckSPAT("RadioCfg", "A");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);
  ret = cfgCheckSPAT("RadioCfg", "-");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);

  /* Length test with valid value. */
  ret = cfgCheckSPAT("RadioCfg", " 0");
  CU_ASSERT(CFG_AOK == ret);
  ret = cfgCheckSPAT("RadioCfg", "0 ");
  CU_ASSERT(CFG_AOK == ret);

  ret = basicElementTest(SPAT_GRP,"RadioCfg");
  CU_ASSERT(CFG_AOK == ret);

/* WSAEnable */

  ret = enableElementTest(SPAT_GRP, "WSAEnable");
  CU_ASSERT(CFG_AOK == ret);

/* WSAPriority */

  /* Valid values. */
  ret = cfgCheckSPAT("WSAPriority", "0");
  CU_ASSERT(CFG_AOK == ret);
  ret = cfgCheckSPAT("WSAPriority", "62");
  CU_ASSERT(CFG_AOK == ret);

  /* Out of Range. */
  ret = cfgCheckSPAT("WSAPriority", "63");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);

  /* Illegal char. */
  ret = cfgCheckSPAT("WSAPriority", "A");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);
  ret =cfgCheckSPAT("WSAPriority", "-");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);

  /* Length test with valid value. */
  ret = cfgCheckSPAT("WSAPriority", "  3");
  CU_ASSERT(CFG_AOK == ret);
  ret = cfgCheckSPAT("WSAPriority", "3  ");
  CU_ASSERT(CFG_AOK == ret);

  ret = basicElementTest(SPAT_GRP,"WSAPriority");
  CU_ASSERT(CFG_AOK == ret);

/* WSMExtension */

  ret = enableElementTest(SPAT_GRP, "WSMExtension");
  CU_ASSERT(CFG_AOK == ret);

/* SPATTOMPSID */

  ret = psidElementTest(SPAT_GRP, "SPATTOMPSID");
  CU_ASSERT(CFG_AOK == ret);

/* SPATChannelNumber */

  ret = channelElementTest(SPAT_GRP, "SPATChannelNumber");
  CU_ASSERT(CFG_AOK == ret);

/* BSMRxEnable */

  ret = enableElementTest(SPAT_GRP,"BSMRxEnable");
  CU_ASSERT(CFG_AOK == ret);

/* BSMRxForward */

  /* Valid values. */
  ret = cfgCheckSPAT("BSMRxForward", "0");
  CU_ASSERT(CFG_AOK == ret);
  ret = cfgCheckSPAT("BSMRxForward", "3");
  CU_ASSERT(CFG_AOK == ret);

  /* Out of Range. */
  ret = cfgCheckSPAT("BSMRxForward", "4");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);

  /* Illegal char. */
  ret = cfgCheckSPAT("BSMRxForward", "A");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);
  ret =cfgCheckSPAT("BSMRxForward", "-");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);

  /* Length test with valid value. */
  ret = cfgCheckSPAT("BSMRxForward", "  1");
  CU_ASSERT(CFG_AOK == ret);
  ret = cfgCheckSPAT("BSMRxForward", "1  ");
  CU_ASSERT(CFG_AOK == ret);

  ret = basicElementTest(SPAT_GRP,"BSMRxForward");
  CU_ASSERT(CFG_AOK == ret);

/* BSMUnsecurePSIDPERU */

  ret = psidElementTest(SPAT_GRP, "BSMUnsecurePSIDPERU");
  CU_ASSERT(CFG_AOK == ret);

/* SPATInteroperabilityMode */

  /* Good values. */
  ret = cfgCheckSPAT("SPATInteroperabilityMode", "0");
  CU_ASSERT(CFG_AOK == ret);
  ret = cfgCheckSPAT("SPATInteroperabilityMode", "0x0");
  CU_ASSERT(CFG_AOK == ret);
  ret = cfgCheckSPAT("SPATInteroperabilityMode", "255");
  CU_ASSERT(CFG_AOK == ret);
  ret = cfgCheckSPAT("SPATInteroperabilityMode", "0xFF");
  CU_ASSERT(CFG_AOK == ret);

  /* Out of Range. */
  ret = cfgCheckSPAT("SPATInteroperabilityMode", "256");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);
  ret = cfgCheckSPAT("SPATInteroperabilityMode", "0x100");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);

  /* Basic element test. */
  ret = basicElementTest(SPAT_GRP,"SPATInteroperabilityMode");
  CU_ASSERT(CFG_AOK == ret);

/* IntersectionID */

  /* Good values. */
  ret = cfgCheckSPAT("IntersectionID", "1");
  CU_ASSERT(CFG_AOK == ret);
  ret = cfgCheckSPAT("IntersectionID", "2147483648");
  CU_ASSERT(CFG_AOK == ret);

  /* Out of Range. */
  ret = cfgCheckSPAT("IntersectionID", "2147483649"); /* This is larger than int32_t can hold. */
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);
  ret = cfgCheckSPAT("IntersectionID", "0");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);

  /* Length test with valid value. */
  ret = cfgCheckSPAT("IntersectionID", "  2147483648");
  CU_ASSERT(CFG_AOK == ret);
  ret = cfgCheckSPAT("IntersectionID", "2147483648  ");
  CU_ASSERT(CFG_AOK == ret);

  /* Basic element test. */
  ret = basicElementTest(SPAT_GRP,"IntersectionID");
  CU_ASSERT(CFG_AOK == ret);
}
/*
 * ===============================  
 * ============ SCS ==============
 * ===============================
 */

void test_scs_conf(void)
{
  int32_t ret = 0;

  /* Test NULL inputs */
  ret = cfgCheckSCS("SRMFwdPort",NULL);
  CU_ASSERT(CFG_PARAM_FAIL == ret);
  ret = cfgCheckSCS(NULL,"0");
  CU_ASSERT(CFG_PARAM_FAIL == ret);
  ret = cfgCheckSCS(NULL,NULL);
  CU_ASSERT(CFG_PARAM_FAIL == ret);

/* InterfaceTOM */

  ret = enableElementTest(SCS_GRP, "InterfaceTOM");
  CU_ASSERT(CFG_AOK == ret);

/* UseMaxTimeToChange */

  ret = enableElementTest(SCS_GRP, "UseMaxTimeToChange");
  CU_ASSERT(CFG_AOK == ret);

/* RSUHeartBeatEnable */

  ret = enableElementTest(SCS_GRP, "RSUHeartBeatEnable");
  CU_ASSERT(CFG_AOK == ret);

/* RSUHeartBeatInt */

  /* Good values. */
  ret = cfgCheckSCS("RSUHeartBeatInt", "1");
  CU_ASSERT(CFG_AOK == ret);
  ret = cfgCheckSCS("RSUHeartBeatInt", "60");
  CU_ASSERT(CFG_AOK == ret);

  /* Out of Range. */
  ret = cfgCheckSCS("RSUHeartBeatInt", "61");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);
  ret = cfgCheckSCS("RSUHeartBeatInt", "0");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);

  /* Length test with valid value. */
  ret = cfgCheckSCS("RSUHeartBeatInt", "  60");
  CU_ASSERT(CFG_AOK == ret);
  ret = cfgCheckSCS("RSUHeartBeatInt", "60  ");
  CU_ASSERT(CFG_AOK == ret);

  /* Basic element test. */
  ret = basicElementTest(SCS_GRP,"RSUHeartBeatInt");
  CU_ASSERT(CFG_AOK == ret);

#if 0 //legacy, there is no file play back at this level.
/* SRMFwdTest */

  ret = enableElementTest(SCS_GRP, "SRMFwdTest");
  CU_ASSERT(CFG_AOK == ret);

  /* SRMFile */

  /* Good values. */
  ret = cfgCheckSCS("SRMFile","/rwflash/srm/srm_out.csv");
  CU_ASSERT(CFG_AOK == ret);
  ret = cfgCheckSCS("SRMFile", "/rwflash"); 
  CU_ASSERT(CFG_AOK == ret);

  /* Bad values. */
  ret = cfgCheckSCS("SRMFile", "\\");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);
  ret = cfgCheckSCS("SRMFile", " ");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);

  /* Can't be empty. */
  ret = basicStringElementTest(SCS_GRP,"SRMFile",WFALSE);
  CU_ASSERT(CFG_AOK == ret);
#endif

/* SRMFwdInterval */

  /* Good values. */
  ret = cfgCheckSCS("SRMFwdInterval", "1000");
  CU_ASSERT(CFG_AOK == ret);

  /* Out of Range. */
  ret = cfgCheckSCS("SRMFwdInterval", "999");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);

  /* Length test with valid value. */
  ret = cfgCheckSCS("SRMFwdInterval", " 1000");
  CU_ASSERT(CFG_AOK == ret);
  ret = cfgCheckSCS("SRMFwdInterval", "1000 ");
  CU_ASSERT(CFG_AOK == ret);

  /* Basic element test. */
  ret = basicElementTest(SCS_GRP,"SRMFwdInterval");
  CU_ASSERT(CFG_AOK == ret);

/* SRMFwdIP */

  /* Good values. */
  ret = cfgCheckSCS("SRMFwdIP", "192.168.2.47");
  CU_ASSERT(CFG_AOK == ret);

  /* Bad values. */
  ret = cfgCheckSCS("SRMFwdIP", "0::ABBA");
  CU_ASSERT(I2V_RETURN_RANGE_LIMIT_FAIL == ret);
  ret = cfgCheckSCS("SRMFwdIP", "DEAD::0");
  CU_ASSERT(I2V_RETURN_RANGE_LIMIT_FAIL == ret);
  ret = cfgCheckSCS("SRMFwdIP", "qwerty");
  CU_ASSERT(I2V_RETURN_RANGE_LIMIT_FAIL == ret);
  ret = cfgCheckSCS("SRMFwdIP", "ABBA:BEEF:2.80");
  CU_ASSERT(I2V_RETURN_RANGE_LIMIT_FAIL == ret);

  /* Can't be empty. */
  ret = basicStringElementTest(SCS_GRP, "SRMFwdIP",WFALSE);
  CU_ASSERT(CFG_AOK == ret);

/* SRMFwdPort */

  /* Good values. */
  ret = cfgCheckSCS("SRMFwdPort", "9091");
  CU_ASSERT(CFG_AOK == ret);
  ret = cfgCheckSCS("SRMFwdPort", "0");
  CU_ASSERT(CFG_AOK == ret);
  ret = cfgCheckSCS("SRMFwdPort", "65535");
  CU_ASSERT(CFG_AOK == ret);

  /* Out of Range. */
  ret = cfgCheckSCS("SRMFwdPort", "65536");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);

  /* Length test with valid value. */
  ret = cfgCheckSCS("SRMFwdPort", "  9091");
  CU_ASSERT(CFG_AOK == ret);
  ret = cfgCheckSCS("SRMFwdPort", "9091  ");
  CU_ASSERT(CFG_AOK == ret);

  /* Basic element test. */
  ret = basicElementTest(SCS_GRP,"SRMFwdPort");
  CU_ASSERT(CFG_AOK == ret);

/* SRMFwdEnable */

  ret = enableElementTest(SCS_GRP, "SRMFwdEnable");
  CU_ASSERT(CFG_AOK == ret);

/* SRMFwdInterface */

  /* Good values. */
  ret = cfgCheckSCS("SRMFwdInterface", "1");
  CU_ASSERT(CFG_AOK == ret);
  ret = cfgCheckSCS("SRMFwdInterface", "3");
  CU_ASSERT(CFG_AOK == ret);

  /* Out of Range. */
  ret = cfgCheckSCS("SRMFwdInterface", "0");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);
  ret = cfgCheckSCS("SRMFwdInterface", "4");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);

  /* Length test with valid value. */
  ret = cfgCheckSCS("SRMFwdInterface", " 3");
  CU_ASSERT(CFG_AOK == ret);
  ret = cfgCheckSCS("SRMFwdInterface", "3 ");
  CU_ASSERT(CFG_AOK == ret);

  /* Basic element test. */
  ret = basicElementTest(SCS_GRP,"SRMFwdInterface");
  CU_ASSERT(CFG_AOK == ret);

/* SRMFwdRequireTTI */

  ret = enableElementTest(SCS_GRP, "SRMFwdRequireTTI");
  CU_ASSERT(CFG_AOK == ret);

/* LocalSignalControllerIP */

  /* Good values. */
  ret = cfgCheckSCS("LocalSignalControllerIP", "192.168.2.32");
  CU_ASSERT(CFG_AOK == ret);

  /* Bad values. */
  ret = cfgCheckSCS("LocalSignalControllerIP", "0::ABBA");
  CU_ASSERT(I2V_RETURN_RANGE_LIMIT_FAIL == ret);
  ret = cfgCheckSCS("LocalSignalControllerIP", "DEAD::0");
  CU_ASSERT(I2V_RETURN_RANGE_LIMIT_FAIL == ret);
  ret = cfgCheckSCS("LocalSignalControllerIP", "qwerty");
  CU_ASSERT(I2V_RETURN_RANGE_LIMIT_FAIL == ret);
  ret = cfgCheckSCS("LocalSignalControllerIP", "ABBA:BEEF:2.80");
  CU_ASSERT(I2V_RETURN_RANGE_LIMIT_FAIL == ret);

  /* Can't be empty. */
  ret = basicStringElementTest(SCS_GRP, "LocalSignalControllerIP",WFALSE);
  CU_ASSERT(CFG_AOK == ret);

/* LocalSignalSNMPPort */

  /* Good values. */
  ret = cfgCheckSCS("LocalSignalSNMPPort", "501");
  CU_ASSERT(CFG_AOK == ret);
  ret = cfgCheckSCS("LocalSignalSNMPPort", "0");
  CU_ASSERT(CFG_AOK == ret);
  ret = cfgCheckSCS("LocalSignalSNMPPort", "65535");
  CU_ASSERT(CFG_AOK == ret);

  /* Out of Range. */
  ret = cfgCheckSCS("LocalSignalSNMPPort", "65536");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);

  /* Length test with valid value. */
  ret = cfgCheckSCS("LocalSignalSNMPPort", "   501");
  CU_ASSERT(CFG_AOK == ret);
  ret = cfgCheckSCS("LocalSignalSNMPPort", "501   ");
  CU_ASSERT(CFG_AOK == ret);

  /* Basic element test. */
  ret = basicElementTest(SCS_GRP,"LocalSignalSNMPPort");
  CU_ASSERT(CFG_AOK == ret);

/* LocalSignalSNMPPort */

  /* Good values. */
  ret = cfgCheckSCS("LocalSignalControllerPort", "6053");
  CU_ASSERT(CFG_AOK == ret);
  ret = cfgCheckSCS("LocalSignalControllerPort", "0");
  CU_ASSERT(CFG_AOK == ret);
  ret = cfgCheckSCS("LocalSignalControllerPort", "65535");
  CU_ASSERT(CFG_AOK == ret);

  /* Out of Range. */
  ret = cfgCheckSCS("LocalSignalControllerPort", "65536");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);

  /* Length test with valid value. */
  ret = cfgCheckSCS("LocalSignalControllerPort", "  6053");
  CU_ASSERT(CFG_AOK == ret);
  ret = cfgCheckSCS("LocalSignalControllerPort", "6053  ");
  CU_ASSERT(CFG_AOK == ret);

  /* Basic element test. */
  ret = basicElementTest(SCS_GRP,"LocalSignalSNMPPort");
  CU_ASSERT(CFG_AOK == ret);

/* LocalSignalIntersectionID */

  /* Good values. */
  ret = cfgCheckSCS("LocalSignalIntersectionID", "147483700");
  CU_ASSERT(CFG_AOK == ret);
  ret = cfgCheckSCS("LocalSignalIntersectionID", "147483705");
  CU_ASSERT(CFG_AOK == ret);

  /* Out of Range. */
  ret = cfgCheckSCS("LocalSignalIntersectionID", "147483706"); /* This is larger than int32_t can hold. */
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);
  ret = cfgCheckSCS("LocalSignalIntersectionID", "147483699");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);

  /* Length test with valid value. */
  ret = cfgCheckSCS("LocalSignalIntersectionID", " 147483705");
  CU_ASSERT(CFG_AOK == ret);
  ret = cfgCheckSCS("LocalSignalIntersectionID", "147483705 ");
  CU_ASSERT(CFG_AOK == ret);

  /* Basic element test. */
  ret = basicElementTest(SCS_GRP,"LocalSignalIntersectionID");
  CU_ASSERT(CFG_AOK == ret);

/* LocalSignalControllerEnable */

  ret = enableElementTest(SCS_GRP, "LocalSignalControllerEnable");
  CU_ASSERT(CFG_AOK == ret);

/* LocalSignalControllerIsSnmp */

  ret = enableElementTest(SCS_GRP, "LocalSignalControllerIsSnmp");
  CU_ASSERT(CFG_AOK == ret);

/* LocalSignalControllerHW */

  /* Good values. */
  ret = cfgCheckSCS("LocalSignalControllerHW", "0");
  CU_ASSERT(CFG_AOK == ret);
  ret = cfgCheckSCS("LocalSignalControllerHW", "0xFF");
  CU_ASSERT(CFG_AOK == ret);
  ret = cfgCheckSCS("LocalSignalControllerHW", "0x0");
  CU_ASSERT(CFG_AOK == ret);
  ret = cfgCheckSCS("LocalSignalControllerHW", "255");
  CU_ASSERT(CFG_AOK == ret);

  /* Out of Range but length gets it first. */
  ret = cfgCheckSCS("LocalSignalControllerHW", "0x100"); /* This is larger than int32_t can hold. */
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);

  /* Length test with valid value. */
  ret = cfgCheckSCS("LocalSignalControllerHW", "  0xF");
  CU_ASSERT(CFG_AOK == ret);
  ret = cfgCheckSCS("LocalSignalControllerHW", "0xF  ");
  CU_ASSERT(CFG_AOK == ret);

  /* Basic element test. */
  ret = basicElementTest(SCS_GRP,"LocalSignalControllerHW");
  CU_ASSERT(CFG_AOK == ret);

/* SNMPEnableSpatCommand */

//TODO: weak test for OID. we can do better.

  /* Good values. */
  ret = cfgCheckSCS("SNMPEnableSpatCommand","1.3.6.1.4.1.1206.3.5.2.9.44.1.1");
  CU_ASSERT(CFG_AOK == ret);
  ret = cfgCheckSCS("SNMPEnableSpatCommand", "/rwflash"); 
  CU_ASSERT(CFG_AOK == ret);

  /* Bad values. */
  ret = cfgCheckSCS("SNMPEnableSpatCommand", "\\");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);
  ret = cfgCheckSCS("SNMPEnableSpatCommand", " ");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);

  /* Can't be empty. */
  ret = basicStringElementTest(SCS_GRP,"SNMPEnableSpatCommand",WFALSE);
  CU_ASSERT(CFG_AOK == ret);

/* SPATEnableValue: OID integer value or string */

  /* Good values. */
  ret = cfgCheckSCS("SPATEnableValue","2");
  CU_ASSERT(CFG_AOK == ret);
  ret = cfgCheckSCS("SPATEnableValue", "6"); 
  CU_ASSERT(CFG_AOK == ret);

  /* Bad values. */
  ret = cfgCheckSCS("SPATEnableValue", "\\");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);
  ret = cfgCheckSCS("SPATEnableValue", " ");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);

  /* Can't be empty. */
  ret = basicStringElementTest(SCS_GRP,"SPATEnableValue",WFALSE);
  CU_ASSERT(CFG_AOK == ret);

/* NTCIP_YELLOW_DURATION */

//TODO: weak test for OID. we can do better.

  /* Good values. */
  ret = cfgCheckSCS("NTCIP_YELLOW_DURATION","1.3.6.1.4.1.1206.4.2.1.1.2.1.8.1");
  CU_ASSERT(CFG_AOK == ret);
  ret = cfgCheckSCS("NTCIP_YELLOW_DURATION", "/rwflash"); 
  CU_ASSERT(CFG_AOK == ret);

  /* Bad values. */
  ret = cfgCheckSCS("NTCIP_YELLOW_DURATION", "\\");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);
  ret = cfgCheckSCS("NTCIP_YELLOW_DURATION", " ");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);

  /* Can't be empty. */
  ret = basicStringElementTest(SCS_GRP,"NTCIP_YELLOW_DURATION",WFALSE);
  CU_ASSERT(CFG_AOK == ret);

/* NTCIP_PHASE_OPTIONS */

//TODO: weak test for OID. we can do better.
#if 0 //legacy or 1202 or?
  /* Good values. */
  ret = cfgCheckSCS("NTCIP_PHASE_OPTIONS","1.3.6.1.4.1.1206.4.2.1.1.2.1.8.1");
  CU_ASSERT(CFG_AOK == ret);
  ret = cfgCheckSCS("NTCIP_PHASE_OPTIONS", "/rwflash"); 
  CU_ASSERT(CFG_AOK == ret);

  /* Bad values. */
  ret = cfgCheckSCS("NTCIP_YELLOW_DURATION", "\\");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);
  ret = cfgCheckSCS("NTCIP_YELLOW_DURATION", " ");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);

  /* Can't be empty. */
  ret = basicStringElementTest(SCS_GRP,"NTCIP_YELLOW_DURATION",WFALSE);
  CU_ASSERT(CFG_AOK == ret);


/* NTCIP_PHASE_GREENS */

//TODO: weak test for OID. we can do better.

  /* Good values. */
  ret = cfgCheckSCS("NTCIP_PHASE_GREENS","1.3.6.1.4.1.1206.4.2.1.1.2.1.8.1");
  CU_ASSERT(CFG_AOK == ret);
  ret = cfgCheckSCS("NTCIP_PHASE_GREENS", "/rwflash"); 
  CU_ASSERT(CFG_AOK == ret);

  /* Bad values. */
  ret = cfgCheckSCS("NTCIP_PHASE_GREENS", "\\");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);
  ret = cfgCheckSCS("NTCIP_PHASE_GREENS", " ");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);

  /* Can't be empty. */
  ret = basicStringElementTest(SCS_GRP,"NTCIP_PHASE_GREENS",WFALSE);
  CU_ASSERT(CFG_AOK == ret)

/* NTCIP_PHASE_YELLOWS */

//TODO: weak test for OID. we can do better.

  /* Good values. */
  ret = cfgCheckSCS("NTCIP_PHASE_YELLOWS","1.3.6.1.4.1.1206.4.2.1.1.2.1.8.1");
  CU_ASSERT(CFG_AOK == ret);
  ret = cfgCheckSCS("NTCIP_PHASE_YELLOWS", "/rwflash"); 
  CU_ASSERT(CFG_AOK == ret);

  /* Bad values. */
  ret = cfgCheckSCS("NTCIP_PHASE_YELLOWS", "\\");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);
  ret = cfgCheckSCS("NTCIP_PHASE_YELLOWS", " ");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);

  /* Can't be empty. */
  ret = basicStringElementTest(SCS_GRP,"NTCIP_PHASE_YELLOWS",WFALSE);
  CU_ASSERT(CFG_AOK == ret)

/* NTCIP_PHASE_NUMBER */

//TODO: weak test for OID. we can do better.

  /* Good values. */
  ret = cfgCheckSCS("NTCIP_PHASE_NUMBER","1.3.6.1.4.1.1206.4.2.1.1.2.1.8.1");
  CU_ASSERT(CFG_AOK == ret);
  ret = cfgCheckSCS("NTCIP_PHASE_NUMBER", "/rwflash"); 
  CU_ASSERT(CFG_AOK == ret);

  /* Bad values. */
  ret = cfgCheckSCS("NTCIP_PHASE_NUMBER", "\\");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);
  ret = cfgCheckSCS("NTCIP_PHASE_NUMBER", " ");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);

  /* Can't be empty. */
  ret = basicStringElementTest(SCS_GRP,"NTCIP_PHASE_NUMBER",WFALSE);
  CU_ASSERT(CFG_AOK == ret)

/* NTCIP_PHASE_MAX_COUNTDOWN */

//TODO: weak test for OID. we can do better.

  /* Good values. */
  ret = cfgCheckSCS("NTCIP_PHASE_MAX_COUNTDOWN","1.3.6.1.4.1.1206.4.2.1.1.2.1.8.1");
  CU_ASSERT(CFG_AOK == ret);
  ret = cfgCheckSCS("NTCIP_PHASE_MAX_COUNTDOWN", "/rwflash"); 
  CU_ASSERT(CFG_AOK == ret);

  /* Bad values. */
  ret = cfgCheckSCS("NTCIP_PHASE_MAX_COUNTDOWN", "\\");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);
  ret = cfgCheckSCS("NTCIP_PHASE_MAX_COUNTDOWN", " ");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);

  /* Can't be empty. */
  ret = basicStringElementTest(SCS_GRP,"NTCIP_PHASE_MAX_COUNTDOWN",WFALSE);
  CU_ASSERT(CFG_AOK == ret)
#endif

/* BypassSignalControllerNTCIP */

  ret = enableElementTest(SCS_GRP, "BypassSignalControllerNTCIP");
  CU_ASSERT(CFG_AOK == ret);

/* BypassYellowDuration */

  /* Good values. */
  ret = cfgCheckSCS("BypassYellowDuration", "1");
  CU_ASSERT(CFG_AOK == ret);
  ret = cfgCheckSCS("BypassYellowDuration", "255");
  CU_ASSERT(CFG_AOK == ret);

  /* Out of Range. */
  ret = cfgCheckSCS("BypassYellowDuration", "0");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);
  ret = cfgCheckSCS("BypassYellowDuration", "256");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);
  
  /* Bad length. */
  ret = cfgCheckSCS("BypassYellowDuration", "255 ");
  CU_ASSERT(CFG_AOK == ret);
  ret = cfgCheckSCS("BypassYellowDuration", "  255");
  CU_ASSERT(CFG_AOK == ret);

  /* Basic element test. */
  ret = basicElementTest(SCS_GRP,"BypassYellowDuration");
  CU_ASSERT(CFG_AOK == ret);

}
/*
 * ===============================  
 * ============ SRMRX ============
 * ===============================
 */
void test_srmrx_conf(void)
{
  int32_t ret = 0;

  /* Test NULL inputs */
  ret = cfgCheckSRMRX("RadioCfg",NULL);
  CU_ASSERT(CFG_PARAM_FAIL == ret);
  ret = cfgCheckSRMRX(NULL,"0");
  CU_ASSERT(CFG_PARAM_FAIL == ret);
  ret = cfgCheckSRMRX(NULL,NULL);
  CU_ASSERT(CFG_PARAM_FAIL == ret);

/* RadioCfg */

  /* Valid values. */
  ret = cfgCheckSRMRX("RadioCfg", "0");
  CU_ASSERT(CFG_AOK == ret);

  /* Out of Range. */
  ret = cfgCheckSRMRX("RadioCfg", "1");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);
  ret = cfgCheckSRMRX("RadioCfg", "2");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);

  /* Illegal char. */
  ret = cfgCheckSRMRX("RadioCfg", "A");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);
  ret = cfgCheckSRMRX("RadioCfg", "-");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);

  /* Length test with valid value. */
  ret = cfgCheckSRMRX("RadioCfg", " 0");
  CU_ASSERT(CFG_AOK == ret);
  ret = cfgCheckSRMRX("RadioCfg", "0 ");
  CU_ASSERT(CFG_AOK == ret);

  ret = basicElementTest(SRMRX_GRP,"RadioCfg");
  CU_ASSERT(CFG_AOK == ret);

/* SecurityDebugEnable */

  ret = enableElementTest(SRMRX_GRP, "SecurityDebugEnable");
  CU_ASSERT(CFG_AOK == ret);

/* SRMDecodeMethod */

  /* Valid values. */
  ret = cfgCheckSRMRX("SRMDecodeMethod", "0");
  CU_ASSERT(CFG_AOK == ret);
  ret = cfgCheckSRMRX("SRMDecodeMethod", "1");
  CU_ASSERT(CFG_AOK == ret);

  /* Out of Range. */
  ret = cfgCheckSRMRX("SRMDecodeMethod", "2");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);

  /* Illegal char. */
  ret = cfgCheckSRMRX("SRMDecodeMethod", "A");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);
  ret = cfgCheckSRMRX("SRMDecodeMethod", "-");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);

  /* Length test with valid value. */
  ret = cfgCheckSRMRX("SRMDecodeMethod", " 1");
  CU_ASSERT(CFG_AOK == ret);
  ret = cfgCheckSRMRX("SRMDecodeMethod", "1 ");
  CU_ASSERT(CFG_AOK == ret);

  ret = basicElementTest(SRMRX_GRP,"SRMDecodeMethod");
  CU_ASSERT(CFG_AOK == ret);

/* SRMDecodeMethod */

  /* Valid values. */
  ret = cfgCheckSRMRX("SRMASN1DecodeMethod", "0");
  CU_ASSERT(CFG_AOK == ret);
  ret = cfgCheckSRMRX("SRMASN1DecodeMethod", "2");
  CU_ASSERT(CFG_AOK == ret);

  /* Out of Range. */
  ret = cfgCheckSRMRX("SRMASN1DecodeMethod", "3");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);

  /* Illegal char. */
  ret = cfgCheckSRMRX("SRMASN1DecodeMethod", "A");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);
  ret = cfgCheckSRMRX("SRMASN1DecodeMethod", "-");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);

  /* Length test with valid value. */
  ret = cfgCheckSRMRX("SRMASN1DecodeMethod", " 2");
  CU_ASSERT(CFG_AOK == ret);
  ret = cfgCheckSRMRX("SRMASN1DecodeMethod", "2 ");
  CU_ASSERT(CFG_AOK == ret);

  ret = basicElementTest(SRMRX_GRP,"SRMASN1DecodeMethod");
  CU_ASSERT(CFG_AOK == ret);

/* SRMChannelNumber */

  ret = channelElementTest(SRMRX_GRP, "SRMChannelNumber");
  CU_ASSERT(CFG_AOK == ret);

/* SRMRxForward */

  ret = enableElementTest(SRMRX_GRP, "SRMRxForward");
  CU_ASSERT(CFG_AOK == ret);

/* SRMUnsecurePSIDDER */

  ret = psidElementTest(SRMRX_GRP, "SRMUnsecurePSIDDER");
  CU_ASSERT(CFG_AOK == ret);

/* SRMUnsecurePSIDPERA */

  ret = psidElementTest(SRMRX_GRP, "SRMUnsecurePSIDPERA");
  CU_ASSERT(CFG_AOK == ret);

/* SRMUnsecurePSIDPERU */

  ret = psidElementTest(SRMRX_GRP, "SRMUnsecurePSIDPERU");
  CU_ASSERT(CFG_AOK == ret);

/* SRMPermissive */

  ret = enableElementTest(SRMRX_GRP, "SRMPermissive");
  CU_ASSERT(CFG_AOK == ret);

/* SRMTXVehBasicRole */

//TODO: bitmask
  /* Good values. */
  ret = cfgCheckSRMRX("SRMTXVehBasicRole", "0");
  CU_ASSERT(CFG_AOK == ret);
  ret = cfgCheckSRMRX("SRMTXVehBasicRole", "0xFFFFFFFF");
  CU_ASSERT(CFG_AOK == ret);

  /* Out of Range & Length Bad same thing. */
  ret = cfgCheckSRMRX("SRMTXVehBasicRole", "0xABCD12345");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);

  /* Basic element test. */
  ret = basicElementTest(SRMRX_GRP,"SRMTXVehBasicRole");
  CU_ASSERT(CFG_AOK == ret);

  ret = cfgCheckSRMRX("SRMVODMsgVerifyCount", "1");
  CU_ASSERT(CFG_AOK == ret);
  ret = cfgCheckSRMRX("SRMVODMsgVerifyCount", "65535");
  CU_ASSERT(CFG_AOK == ret);

  /* Out of Range & Length Bad same thing. */
  ret = cfgCheckSRMRX("SRMVODMsgVerifyCount", "665534");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);

  /* Basic element test. */
  ret = basicElementTest(SRMRX_GRP,"SRMVODMsgVerifyCount");
  CU_ASSERT(CFG_AOK == ret);

}
/*
 * ===============================  
 * ============ SNMPD ============
 * ===============================
 */
void test_snmpd_conf(void)
{
  int32_t ret = 0;

  /* Test each input for NULL */
  ret = cfgCheckSNMPD(NULL, "161");
  CU_ASSERT(CFG_PARAM_FAIL == ret);
  ret = cfgCheckSNMPD("agentaddress", NULL);
  CU_ASSERT(CFG_PARAM_FAIL == ret);
  ret = cfgCheckSNMPD(NULL, NULL);
  CU_ASSERT(CFG_PARAM_FAIL == ret);

/* agentaddress */

  /* Good values. */
  ret = cfgCheckSNMPD("agentaddress", "161");
  CU_ASSERT(CFG_AOK == ret);
  ret = cfgCheckSNMPD("agentaddress", "0");
  CU_ASSERT(CFG_AOK == ret);
  ret = cfgCheckSNMPD("agentaddress", "65535");
  CU_ASSERT(CFG_AOK == ret);

  /* Out of Range. */
  ret = cfgCheckSNMPD("agentaddress", "65536");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);

  /* Length test with valid value. */
  ret = cfgCheckSNMPD("agentaddress", "  6053");
  CU_ASSERT(CFG_AOK == ret);
  ret = cfgCheckSNMPD("agentaddress", "6053  ");
  CU_ASSERT(CFG_AOK == ret);

  /* Basic element test. */
  ret = basicElementTest(SNMPD_GRP,"agentaddress");
  CU_ASSERT(CFG_AOK == ret);
}
/*
 * ===============================  
 * ============ NSCONFIG =========
 * ===============================
 */
void test_nsconfig_conf(void)
{
  int32_t ret = 0;

  /* Test each input for NULL */
  ret = cfgCheckNSCONFIG("AC_BE_CCH_RADIO_0", NULL);
  CU_ASSERT(CFG_PARAM_FAIL == ret);
  ret = cfgCheckNSCONFIG(NULL, "163");
  CU_ASSERT(CFG_PARAM_FAIL == ret);
  ret = cfgCheckNSCONFIG(NULL, NULL);
  CU_ASSERT(CFG_PARAM_FAIL == ret);

/* AC_BE_CCH_RADIO_0 */

//TODO: weak test for OID. we can do better.

  /* Good values. */
  ret = cfgCheckNSCONFIG("AC_BE_CCH_RADIO_0",AC_BE_CCH_RADIO_0_DEFAULT_S);
  CU_ASSERT(CFG_AOK == ret);
  ret = cfgCheckNSCONFIG("AC_BE_CCH_RADIO_0", "/rwflash"); 
  CU_ASSERT(CFG_AOK == ret);

  /* Bad values. */
  ret = cfgCheckNSCONFIG("AC_BE_CCH_RADIO_0", "\\");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);
  ret = cfgCheckNSCONFIG("AC_BE_CCH_RADIO_0", " ");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);

  /* Can't be empty. */
  ret = basicStringElementTest(NSCONFIG_GRP,"AC_BE_CCH_RADIO_0",WFALSE);
  CU_ASSERT(CFG_AOK == ret)

/* AC_BK_CCH_RADIO_0 = 15,1023,9,0 */

//TODO: weak test for OID. we can do better.

  /* Good values. */
  ret = cfgCheckNSCONFIG("AC_BK_CCH_RADIO_0","15,1023,9,0");
  CU_ASSERT(CFG_AOK == ret);
  ret = cfgCheckNSCONFIG("AC_BK_CCH_RADIO_0", "/rwflash"); 
  CU_ASSERT(CFG_AOK == ret);

  /* Bad values. */
  ret = cfgCheckNSCONFIG("AC_BK_CCH_RADIO_0", "\\");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);
  ret = cfgCheckNSCONFIG("AC_BK_CCH_RADIO_0", " ");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);

  /* Can't be empty. */
  ret = basicStringElementTest(NSCONFIG_GRP,"AC_BK_CCH_RADIO_0",WFALSE);
  CU_ASSERT(CFG_AOK == ret)

/* AC_VI_CCH_RADIO_0 = 15,1023,4,0 */
//TODO: weak test for OID. we can do better.

  /* Good values. */
  ret = cfgCheckNSCONFIG("AC_VI_CCH_RADIO_0","15,1023,4,0");
  CU_ASSERT(CFG_AOK == ret);
  ret = cfgCheckNSCONFIG("AC_VI_CCH_RADIO_0", "/rwflash"); 
  CU_ASSERT(CFG_AOK == ret);

  /* Bad values. */
  ret = cfgCheckNSCONFIG("AC_VI_CCH_RADIO_0", "\\");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);
  ret = cfgCheckNSCONFIG("AC_VI_CCH_RADIO_0", " ");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);

  /* Can't be empty. */
  ret = basicStringElementTest(NSCONFIG_GRP,"AC_VI_CCH_RADIO_0",WFALSE);
  CU_ASSERT(CFG_AOK == ret)

/* AC_VO_CCH_RADIO_0 = 3,7,2,0 */

//TODO: weak test for OID. we can do better.

  /* Good values. */
  ret = cfgCheckNSCONFIG("AC_VO_CCH_RADIO_0","3,7,2,0");
  CU_ASSERT(CFG_AOK == ret);
  ret = cfgCheckNSCONFIG("AC_VO_CCH_RADIO_0", "/rwflash"); 
  CU_ASSERT(CFG_AOK == ret);

  /* Bad values. */
  ret = cfgCheckNSCONFIG("AC_VO_CCH_RADIO_0", "\\");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);
  ret = cfgCheckNSCONFIG("AC_VO_CCH_RADIO_0", " ");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);

  /* Can't be empty. */
  ret = basicStringElementTest(NSCONFIG_GRP,"AC_VO_CCH_RADIO_0",WFALSE);
  CU_ASSERT(CFG_AOK == ret)

/* AC_BE_CCH_RADIO_1 */

//TODO: weak test for OID. we can do better.

  /* Good values. */
  ret = cfgCheckNSCONFIG("AC_BE_CCH_RADIO_1","15,1023,6,0");
  CU_ASSERT(CFG_AOK == ret);
  ret = cfgCheckNSCONFIG("AC_BE_CCH_RADIO_1", "/rwflash"); 
  CU_ASSERT(CFG_AOK == ret);

  /* Bad values. */
  ret = cfgCheckNSCONFIG("AC_BE_CCH_RADIO_1", "\\");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);
  ret = cfgCheckNSCONFIG("AC_BE_CCH_RADIO_1", " ");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);

  /* Can't be empty. */
  ret = basicStringElementTest(NSCONFIG_GRP,"AC_BE_CCH_RADIO_1",WFALSE);
  CU_ASSERT(CFG_AOK == ret)

/* AC_BK_CCH_RADIO_1 = 15,1023,9,0 */

//TODO: weak test for OID. we can do better.

  /* Good values. */
  ret = cfgCheckNSCONFIG("AC_BK_CCH_RADIO_1","15,1023,9,0");
  CU_ASSERT(CFG_AOK == ret);
  ret = cfgCheckNSCONFIG("AC_BK_CCH_RADIO_1", "/rwflash"); 
  CU_ASSERT(CFG_AOK == ret);

  /* Bad values. */
  ret = cfgCheckNSCONFIG("AC_BK_CCH_RADIO_1", "\\");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);
  ret = cfgCheckNSCONFIG("AC_BK_CCH_RADIO_1", " ");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);

  /* Can't be empty. */
  ret = basicStringElementTest(NSCONFIG_GRP,"AC_BK_CCH_RADIO_1",WFALSE);
  CU_ASSERT(CFG_AOK == ret)

/* AC_VI_CCH_RADIO_1 = 15,1023,4,0 */
//TODO: weak test for OID. we can do better.

  /* Good values. */
  ret = cfgCheckNSCONFIG("AC_VI_CCH_RADIO_1","15,1023,4,0");
  CU_ASSERT(CFG_AOK == ret);
  ret = cfgCheckNSCONFIG("AC_VI_CCH_RADIO_1", "/rwflash"); 
  CU_ASSERT(CFG_AOK == ret);

  /* Bad values. */
  ret = cfgCheckNSCONFIG("AC_VI_CCH_RADIO_1", "\\");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);
  ret = cfgCheckNSCONFIG("AC_VI_CCH_RADIO_1", " ");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);

  /* Can't be empty. */
  ret = basicStringElementTest(NSCONFIG_GRP,"AC_VI_CCH_RADIO_1",WFALSE);
  CU_ASSERT(CFG_AOK == ret)

/* AC_VO_CCH_RADIO_1 = 3,7,2,0 */

//TODO: weak test for OID. we can do better.

  /* Good values. */
  ret = cfgCheckNSCONFIG("AC_VO_CCH_RADIO_1","3,7,2,0");
  CU_ASSERT(CFG_AOK == ret);
  ret = cfgCheckNSCONFIG("AC_VO_CCH_RADIO_1", "/rwflash"); 
  CU_ASSERT(CFG_AOK == ret);

  /* Bad values. */
  ret = cfgCheckNSCONFIG("AC_VO_CCH_RADIO_1", "\\");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);
  ret = cfgCheckNSCONFIG("AC_VO_CCH_RADIO_1", " ");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);

  /* Can't be empty. */
  ret = basicStringElementTest(NSCONFIG_GRP,"AC_VO_CCH_RADIO_1",WFALSE);
  CU_ASSERT(CFG_AOK == ret)

/* AC_BE_SCH_RADIO_0 */

//TODO: weak test for OID. we can do better.

  /* Good values. */
  ret = cfgCheckNSCONFIG("AC_BE_SCH_RADIO_0","15,1023,6,0");
  CU_ASSERT(CFG_AOK == ret);
  ret = cfgCheckNSCONFIG("AC_BE_SCH_RADIO_0", "/rwflash"); 
  CU_ASSERT(CFG_AOK == ret);

  /* Bad values. */
  ret = cfgCheckNSCONFIG("AC_BE_SCH_RADIO_0", "\\");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);
  ret = cfgCheckNSCONFIG("AC_BE_SCH_RADIO_0", " ");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);

  /* Can't be empty. */
  ret = basicStringElementTest(NSCONFIG_GRP,"AC_BE_SCH_RADIO_0",WFALSE);
  CU_ASSERT(CFG_AOK == ret)

/* AC_BK_SCH_RADIO_0 = 15,1023,9,0 */

//TODO: weak test for OID. we can do better.

  /* Good values. */
  ret = cfgCheckNSCONFIG("AC_BK_SCH_RADIO_0","15,1023,9,0");
  CU_ASSERT(CFG_AOK == ret);
  ret = cfgCheckNSCONFIG("AC_BK_SCH_RADIO_0", "/rwflash"); 
  CU_ASSERT(CFG_AOK == ret);

  /* Bad values. */
  ret = cfgCheckNSCONFIG("AC_BK_SCH_RADIO_0", "\\");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);
  ret = cfgCheckNSCONFIG("AC_BK_SCH_RADIO_0", " ");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);

  /* Can't be empty. */
  ret = basicStringElementTest(NSCONFIG_GRP,"AC_BK_SCH_RADIO_0",WFALSE);
  CU_ASSERT(CFG_AOK == ret)

/* AC_VI_SCH_RADIO_0 = 15,1023,4,0 */
//TODO: weak test for OID. we can do better.

  /* Good values. */
  ret = cfgCheckNSCONFIG("AC_VI_SCH_RADIO_0","15,1023,4,0");
  CU_ASSERT(CFG_AOK == ret);
  ret = cfgCheckNSCONFIG("AC_VI_SCH_RADIO_0", "/rwflash"); 
  CU_ASSERT(CFG_AOK == ret);

  /* Bad values. */
  ret = cfgCheckNSCONFIG("AC_VI_SCH_RADIO_0", "\\");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);
  ret = cfgCheckNSCONFIG("AC_VI_SCH_RADIO_0", " ");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);

  /* Can't be empty. */
  ret = basicStringElementTest(NSCONFIG_GRP,"AC_VI_SCH_RADIO_0",WFALSE);
  CU_ASSERT(CFG_AOK == ret)

/* AC_VO_SCH_RADIO_0 = 3,7,2,0 */

//TODO: weak test for OID. we can do better.

  /* Good values. */
  ret = cfgCheckNSCONFIG("AC_VO_SCH_RADIO_0","3,7,2,0");
  CU_ASSERT(CFG_AOK == ret);
  ret = cfgCheckNSCONFIG("AC_VO_SCH_RADIO_0", "/rwflash"); 
  CU_ASSERT(CFG_AOK == ret);

  /* Bad values. */
  ret = cfgCheckNSCONFIG("AC_VO_SCH_RADIO_0", "\\");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);
  ret = cfgCheckNSCONFIG("AC_VO_SCH_RADIO_0", " ");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);

  /* Can't be empty. */
  ret = basicStringElementTest(NSCONFIG_GRP,"AC_VO_SCH_RADIO_0",WFALSE);
  CU_ASSERT(CFG_AOK == ret)

/* AC_BE_SCH_RADIO_1 */

//TODO: weak test for OID. we can do better.

  /* Good values. */
  ret = cfgCheckNSCONFIG("AC_BE_SCH_RADIO_1","15,1023,6,0");
  CU_ASSERT(CFG_AOK == ret);
  ret = cfgCheckNSCONFIG("AC_BE_SCH_RADIO_1", "/rwflash"); 
  CU_ASSERT(CFG_AOK == ret);

  /* Bad values. */
  ret = cfgCheckNSCONFIG("AC_BE_SCH_RADIO_1", "\\");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);
  ret = cfgCheckNSCONFIG("AC_BE_SCH_RADIO_1", " ");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);

  /* Can't be empty. */
  ret = basicStringElementTest(NSCONFIG_GRP,"AC_BE_SCH_RADIO_1",WFALSE);
  CU_ASSERT(CFG_AOK == ret)

/* AC_BK_SCH_RADIO_1 = 15,1023,9,0 */

//TODO: weak test for OID. we can do better.

  /* Good values. */
  ret = cfgCheckNSCONFIG("AC_BK_SCH_RADIO_1","15,1023,9,0");
  CU_ASSERT(CFG_AOK == ret);
  ret = cfgCheckNSCONFIG("AC_BK_SCH_RADIO_1", "/rwflash"); 
  CU_ASSERT(CFG_AOK == ret);

  /* Bad values. */
  ret = cfgCheckNSCONFIG("AC_BK_SCH_RADIO_1", "\\");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);
  ret = cfgCheckNSCONFIG("AC_BK_SCH_RADIO_1", " ");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);

  /* Can't be empty. */
  ret = basicStringElementTest(NSCONFIG_GRP,"AC_BK_SCH_RADIO_1",WFALSE);
  CU_ASSERT(CFG_AOK == ret)

/* AC_VI_SCH_RADIO_1 = 15,1023,4,0 */
//TODO: weak test for OID. we can do better.

  /* Good values. */
  ret = cfgCheckNSCONFIG("AC_VI_SCH_RADIO_1","15,1023,4,0");
  CU_ASSERT(CFG_AOK == ret);
  ret = cfgCheckNSCONFIG("AC_VI_SCH_RADIO_1", "/rwflash"); 
  CU_ASSERT(CFG_AOK == ret);

  /* Bad values. */
  ret = cfgCheckNSCONFIG("AC_VI_SCH_RADIO_1", "\\");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);
  ret = cfgCheckNSCONFIG("AC_VI_SCH_RADIO_1", " ");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);

  /* Can't be empty. */
  ret = basicStringElementTest(NSCONFIG_GRP,"AC_VI_SCH_RADIO_1",WFALSE);
  CU_ASSERT(CFG_AOK == ret)

/* AC_VO_SCH_RADIO_1 = 3,7,2,0 */

//TODO: weak test for OID. we can do better.

  /* Good values. */
  ret = cfgCheckNSCONFIG("AC_VO_SCH_RADIO_1","3,7,2,0");
  CU_ASSERT(CFG_AOK == ret);
  ret = cfgCheckNSCONFIG("AC_VO_SCH_RADIO_1", "/rwflash"); 
  CU_ASSERT(CFG_AOK == ret);

  /* Bad values. */
  ret = cfgCheckNSCONFIG("AC_VO_SCH_RADIO_1", "\\");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);
  ret = cfgCheckNSCONFIG("AC_VO_SCH_RADIO_1", " ");
  CU_ASSERT(CFG_SET_VALUE_RANGE_ERROR == ret);

  /* Can't be empty. */
  ret = basicStringElementTest(NSCONFIG_GRP,"AC_VO_SCH_RADIO_1",WFALSE);
  CU_ASSERT(CFG_AOK == ret)
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void test_cfgCheckParam(void)
{
  int32_t ret = 0;
  /* Not for us we dont recognize group so dont fail. */
  ret = cfgCheckParam("foobar","I2VUnifiedChannelNumber", "183");
  CU_ASSERT(CFG_AOK == ret);

  /* Test each input for NULL */
  ret = cfgCheckParam(NULL,"I2VUnifiedChannelNumber", "183");
  CU_ASSERT(CFG_PARAM_FAIL == ret);
  ret = cfgCheckParam(I2V_GRP,NULL, "183");
  CU_ASSERT(CFG_PARAM_FAIL == ret);  
  ret = cfgCheckParam(I2V_GRP,"I2VUnifiedChannelNumber", NULL);
  CU_ASSERT(CFG_PARAM_FAIL == ret);

  /* If we recognize group but not param then fail. */
  ret = cfgCheckParam(I2V_GRP,"foobar", "183");
  CU_ASSERT(CFG_VALUE_UNKOWN_PARAM == ret);
  ret = cfgCheckParam(AMH_GRP,"foobar", "183");
  CU_ASSERT(CFG_VALUE_UNKOWN_PARAM == ret);
  ret = cfgCheckParam(IPB_GRP,"foobar", "183");
  CU_ASSERT(CFG_VALUE_UNKOWN_PARAM == ret);
  ret = cfgCheckParam(RSUINFO_GRP,"foobar", "183");
  CU_ASSERT(CFG_VALUE_UNKOWN_PARAM == ret);
  ret = cfgCheckParam(SPAT_GRP,"foobar", "183");
  CU_ASSERT(CFG_VALUE_UNKOWN_PARAM == ret);
  ret = cfgCheckParam(SCS_GRP,"foobar", "183");
  CU_ASSERT(CFG_VALUE_UNKOWN_PARAM == ret);
  ret = cfgCheckParam(SRMRX_GRP,"foobar", "183");
  CU_ASSERT(CFG_VALUE_UNKOWN_PARAM == ret);
  ret = cfgCheckParam(SNMPD_GRP,"foobar", "183");
  CU_ASSERT(CFG_VALUE_UNKOWN_PARAM == ret);
  ret = cfgCheckParam(NSCONFIG_GRP,"foobar", "183");
  CU_ASSERT(CFG_VALUE_UNKOWN_PARAM == ret);
}
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*
 * process_login_request(char_t* in_msg, char_t* ret_msg)
 */
void test_process_login_request(void)
{
  process_login_request(NULL, NULL);
}
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*
 * process_logout_request(char_t* in_msg, char_t* ret_msg)
 */
void test_process_logout_request(void)
{
  process_logout_request(NULL, NULL);
}
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*
 * process_factory_reset_request(char_t* in_msg, char_t* ret_msg)
 */
void test_process_factory_reset_request(void)
{
  process_factory_reset_request(NULL, NULL);
}
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*
 * process_factory_reset_but_presrv_net_request(char_t* in_msg, char_t* ret_msg)
 */
void test_process_factory_reset_but_presrv_net_request(void)
{
  process_factory_reset_but_presrv_net_request(NULL, NULL);
}
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*
 * process_session_chk_request(char_t* in_msg, char_t* ret_msg)
 */
void test_process_session_chk_request(void)
{
  process_session_chk_request(NULL, NULL);
}
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*
 * process_info_msg(char_t* in_msg, char_t* ret_msg)
 */
void test_process_info_msg(void)
{
  process_info_msg(NULL, NULL);
}
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*
 * process_response_msg(char_t* in_msg, char_t* ret_msg)
 */
void test_process_response_msg(void)
{
  process_response_msg(NULL, NULL);
}
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*
 * process_request_msg(char_t* in_msg, char_t* ret_msg)
 */
void test_process_request_msg(void)
{
  process_request_msg(NULL, NULL);
}
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/* 
 * void update_cache(char_t  *conf_group,  conf_element_t *conf_table, int32_t ecount)
 */
void test_update_cache(void)
{
  update_cache(NULL,NULL,0);
}
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*
 * process_read_request(char_t* in_msg, char_t* ret_msg);
 */
void test_process_read_request(void)
{
  process_read_request(NULL,NULL);
//TODO
}
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*
 * process_write_request(char_t* in_msg, char_t* ret_msg);
 */
void test_process_write_request(void)
{
  char_t ret_msg[10];

  process_write_request(NULL,NULL);

  process_write_request( "|amh|ImmediateIPFilter|1",ret_msg);

}
/*
 * int32_t cfgNetworkGetMode(int32_t isOBU)
 */
void test_cfgNetworkGetMode(void)
{
  int32_t ret = 0;

  /* No DCU so system() will return NULL */
  ret = cfgNetworkGetMode(0);
  CU_ASSERT(-1 == ret);

  ret = cfgNetworkGetMode(1);
  CU_ASSERT(-1 == ret);

}
/*
 * cfgNetworkGetMask(int32_t isFECIP)
 */
void test_cfgNetworkGetMask(void)
{
  int32_t ret = 0;

  /* No DCU so system() will return NULL */
  ret = cfgNetworkGetMask(0);
  CU_ASSERT(CFG_GET_NETMASK_FAIL == ret);

  ret = cfgNetworkGetMask(1);
  CU_ASSERT(CFG_GET_NETMASK_FAIL == ret);
}
/*
 * cfgNetworkSetMask(int32_t mask, int32_t isFECIP)
 */
void test_cfgNetworkSetMask(void)
{
  int32_t ret = 0;

  /* No DCU so system() will return NULL */
  ret = cfgNetworkSetMask(0, 0);
  CU_ASSERT(CFG_PARAM_FAIL == ret);

  ret = cfgNetworkSetMask(32, 0);
  CU_ASSERT(CFG_PARAM_FAIL == ret);

  ret = cfgNetworkSetMask(1, 0);
  CU_ASSERT(CFG_SET_NETMASK_FAIL == ret);

  ret = cfgNetworkSetMask(31, 0);
  CU_ASSERT(CFG_SET_NETMASK_FAIL == ret);

  ret = cfgNetworkSetMask(1, 1);
  CU_ASSERT(CFG_SET_NETMASK_FAIL == ret);

  ret = cfgNetworkSetMask(31, 1);
  CU_ASSERT(CFG_SET_NETMASK_FAIL == ret);
}
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void test_conf_agent_main(void)
{
  int32_t  ret = 0;
  char_t  *token[MAX_TOKENS];
  int32_t  my_argc=2;
  int32_t  i;

  /* Check for pending tests that did not complete.
   * Set tickle mask and ack mask for test.  
   */
  CU_ASSERT(0x0 == check_stub_pending_signals());
  clear_all_stub_signal();

  /* 
   * main() 
   */
  
  if(0 == system("rm -f ./zc_agt_log.txt")){

  }

  for(i=0;i<MAX_TOKENS;i++) {
      token[i] = (char_t *)malloc(MAX_TOKEN_SIZE * 2); /* Allow room for null + extra junk to break conf_manager */
      strcpy(token[i],"");
  }

  my_argc=1;
  ret = conf_agent_main(my_argc,token);
  CU_ASSERT(1 == ret)

  my_argc=7;
  ret = conf_agent_main(my_argc,token);
  CU_ASSERT(1 == ret)

  my_argc=3;
  ret = conf_agent_main(my_argc,token);
  CU_ASSERT(1 == ret)

/*
 * Debug On
 */
  strcpy(token[0],"./conf_agent");
  strcpy(token[1],"-d");
  strcpy(token[2],"READ");
  strcpy(token[3],AMH_GRP);
  strcpy(token[4],"AMHForwardEnable");

  my_argc=4;
  ret = conf_agent_main(my_argc,token);
  CU_ASSERT(1 == ret)

  my_argc=5;
  ret = conf_agent_main(my_argc,token);
  CU_ASSERT(1 == ret)

/* Now make each token too big. */

  strcpy(token[1],IAMTOOBIGTOKEN);

  my_argc=5;
  ret = conf_agent_main(my_argc,token);
  CU_ASSERT(1 == ret)

/* Now try to make the message too big by pass max token size for each param. */
  strcpy(token[1],"-d");
  strcpy(token[2],"READ"); /* This token cant be too large by def. */
  strcpy(token[3],IAMMAXTOKEN);
  strcpy(token[4],IAMMAXTOKEN);
/* the rest */
  my_argc=5;
  ret = conf_agent_main(my_argc,token);
  CU_ASSERT(1 == ret)

  strcpy(token[0],"./conf_agent");
  strcpy(token[1],"-d");
  strcpy(token[2],"WRITE");
  strcpy(token[3],IAMMAXTOKEN);
  strcpy(token[4],IAMMAXTOKEN);
  strcpy(token[5],IAMMAXTOKEN);

  my_argc=5;
  ret = conf_agent_main(my_argc,token);
  CU_ASSERT(1 == ret)

  my_argc=6;
  ret = conf_agent_main(my_argc,token);
  CU_ASSERT(1 == ret)


/* Now try to make the message too big by pass max token size for each param. */

  strcpy(token[3],AMH_GRP);
  strcpy(token[4],"AMHForwardEnable");
  strcpy(token[5],"1");

  my_argc=6;
  ret = conf_agent_main(my_argc,token);
  CU_ASSERT(1 == ret)

  strcpy(token[0],"./conf_agent");
  strcpy(token[1],"-d");
  strcpy(token[2],"UPDATE");
  strcpy(token[3],AMH_GRP);

  my_argc=3;
  ret = conf_agent_main(my_argc,token);
  CU_ASSERT(1 == ret)

  my_argc=4;
  ret = conf_agent_main(my_argc,token);
  CU_ASSERT(1 == ret)

  strcpy(token[0],"./conf_agent");
  strcpy(token[1],"-d");
  strcpy(token[2],"UPDATE_ALL");

  my_argc=2;
  ret = conf_agent_main(my_argc,token);
  CU_ASSERT(1 == ret)

  my_argc=3;
  ret = conf_agent_main(my_argc,token);
  CU_ASSERT(1 == ret)

  strcpy(token[0],"./conf_agent");
  strcpy(token[1],"-d");
  strcpy(token[2],"LOGIN");
  strcpy(token[3],"192.168.2.47");

  my_argc=3;
  ret = conf_agent_main(my_argc,token);
  CU_ASSERT(1 == ret)

  my_argc=4;
  ret = conf_agent_main(my_argc,token);
  CU_ASSERT(1 == ret)

  strcpy(token[0],"./conf_agent");
  strcpy(token[1],"-d");
  strcpy(token[2],"LOGOUT");
  strcpy(token[3],"192.168.2.47");

  my_argc=3;
  ret = conf_agent_main(my_argc,token);
  CU_ASSERT(1 == ret)

  my_argc=4;
  ret = conf_agent_main(my_argc,token);
  CU_ASSERT(1 == ret)

  strcpy(token[0],"./conf_agent");
  strcpy(token[1],"-d");
  strcpy(token[2],"SESSION_CHK");
  strcpy(token[3],"1969");

  my_argc=3;
  ret = conf_agent_main(my_argc,token);
  CU_ASSERT(1 == ret)

  my_argc=4;
  ret = conf_agent_main(my_argc,token);
  CU_ASSERT(1 == ret)


  strcpy(token[0],"./conf_agent");
  strcpy(token[1],"-d");
  strcpy(token[2],"FACTORY_DEFAULTS_RESET");
  strcpy(token[3],"Yes");

  my_argc=3;
  ret = conf_agent_main(my_argc,token);
  CU_ASSERT(1 == ret)

  my_argc=4;
  ret = conf_agent_main(my_argc,token);
  CU_ASSERT(1 == ret)

  strcpy(token[0],"./conf_agent");
  strcpy(token[1],"-d");
  strcpy(token[2],"FACTORY_DEFAULTS_RESET_BUT_PRESERVE_NETWORKING");
  strcpy(token[3],"Yes");

  my_argc=3;
  ret = conf_agent_main(my_argc,token);
  CU_ASSERT(1 == ret)

  my_argc=4;
  ret = conf_agent_main(my_argc,token);
  CU_ASSERT(1 == ret)


/*
 * Debug Off
 */
  for(i=0;i<MAX_TOKENS;i++) {
      strcpy(token[i],"");
  }

  strcpy(token[0],"./conf_agent");
  strcpy(token[1],"READ");
  strcpy(token[2],AMH_GRP);
  strcpy(token[3],"AMHForwardEnable");

  my_argc=3;
  ret = conf_agent_main(my_argc,token);
  CU_ASSERT(1 == ret)

  my_argc=4;
  ret = conf_agent_main(my_argc,token);
  CU_ASSERT(1 == ret)

  strcpy(token[0],"./conf_agent");
  strcpy(token[1],"WRITE");
  strcpy(token[2],AMH_GRP);
  strcpy(token[3],"AMHForwardEnable");
  strcpy(token[4],"1");

  my_argc=4;
  ret = conf_agent_main(my_argc,token);
  CU_ASSERT(1 == ret)

  my_argc=5;
  ret = conf_agent_main(my_argc,token);
  CU_ASSERT(1 == ret)

  strcpy(token[0],"./conf_agent");
  strcpy(token[1],"UPDATE");
  strcpy(token[2],AMH_GRP);

  my_argc=2;
  ret = conf_agent_main(my_argc,token);
  CU_ASSERT(1 == ret)

  my_argc=3;
  ret = conf_agent_main(my_argc,token);
  CU_ASSERT(1 == ret)

  strcpy(token[0],"./conf_agent");
  strcpy(token[1],"UPDATEALL");
  my_argc=1;
  ret = conf_agent_main(my_argc,token);
  CU_ASSERT(1 == ret)

  my_argc=2;
  ret = conf_agent_main(my_argc,token);
  CU_ASSERT(1 == ret)

  strcpy(token[0],"./conf_agent");
  strcpy(token[1],"LOGIN");
  strcpy(token[2],"192.168.2.47");

  my_argc=2;
  ret = conf_agent_main(my_argc,token);
  CU_ASSERT(1 == ret)

  my_argc=3;
  ret = conf_agent_main(my_argc,token);
  CU_ASSERT(1 == ret)

  strcpy(token[0],"./conf_agent");
  strcpy(token[1],"LOGOUT");
  strcpy(token[2],"192.168.2.47");

  my_argc=2;
  ret = conf_agent_main(my_argc,token);
  CU_ASSERT(1 == ret)

  my_argc=3;
  ret = conf_agent_main(my_argc,token);
  CU_ASSERT(1 == ret)

  strcpy(token[0],"./conf_agent");
  strcpy(token[1],"SESSION_CHK");
  strcpy(token[2],"1969");

  my_argc=2;
  ret = conf_agent_main(my_argc,token);
  CU_ASSERT(1 == ret)

  my_argc=3;
  ret = conf_agent_main(my_argc,token);
  CU_ASSERT(1 == ret)

  strcpy(token[0],"./conf_agent");
  strcpy(token[1],"FACTORY_DEFAULTS_RESET");
  strcpy(token[2],"Yes");

  my_argc=2;
  ret = conf_agent_main(my_argc,token);
  CU_ASSERT(1 == ret)

  my_argc=3;
  ret = conf_agent_main(my_argc,token);
  CU_ASSERT(1 == ret)

  strcpy(token[0],"./conf_agent");
  strcpy(token[1],"FACTORY_DEFAULTS_RESET_BUT_PRESERVE_NETWORKING");
  strcpy(token[2],"Yes");

  my_argc=2;
  ret = conf_agent_main(my_argc,token);
  CU_ASSERT(1 == ret)

  my_argc=3;
  ret = conf_agent_main(my_argc,token);
  CU_ASSERT(1 == ret)

/* Too few no debug */

  for(i=0;i<MAX_TOKENS;i++) {
      strcpy(token[i],"");
  }
  ret = conf_agent_main(my_argc,token);
  CU_ASSERT(1 == ret)
 
//80% coverage

  /* Check for pending tests that did not complete.
   * Set tickle mask and ack mask for test.  
   */
  CU_ASSERT(0x0 == check_stub_pending_signals());
  clear_all_stub_signal();

  for(i=0;i<MAX_TOKENS;i++) {
      free(token[i]);
  }
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void test_main(void)
{
  int32_t ret = 0; 

  /* If unsure then init these before any function call or call to conf_manager_main(). */
  mainloop = WTRUE;
  loop = WTRUE;

  /* Check for pending tests that did not complete.
   * Set tickle mask and ack mask for test.  
   */
  CU_ASSERT(0x0 == check_stub_pending_signals());
  clear_all_stub_signal();

  /* 
   * I2V CONF 
   */
  test_i2v_conf();
  /* 
   * AMH CONF 
   */
  test_amh_conf();

  /* 
   * IPB CONF 
   */
  test_ipb_conf();

  /* 
   * RSUINFO CONF 
   */
  test_rsuinfo_conf();

  /* 
   * SPAT CONF 
   */
  test_spat_conf();

  /* 
   * SCS CONF 
   */
  test_scs_conf();

  /* 
   * SRMRX CONF 
   */
  test_srmrx_conf();

  /* 
   * SNMPD CONF 
   */
  test_snmpd_conf();

  /* 
   * NSCONFIG CONF 
   */
  test_nsconfig_conf();

  /* 
   * CfgCheckParam()  
   */
  test_cfgCheckParam();

  /*
   * process_write_request()
   */
  test_process_write_request();

  /*
   * process_read_request()
   */
  test_process_read_request();

  /*
   * process_write_request()
   */
  test_process_write_request();

  /*
   * process_request_msg()
   */
  test_process_request_msg();

  /*
   * update_cache()
   */
  test_update_cache();

  test_process_login_request();

  test_process_logout_request();

  test_process_factory_reset_request();

  test_process_factory_reset_but_presrv_net_request();

  test_process_session_chk_request();

  test_process_info_msg();

  test_process_response_msg();

  test_cfgNetworkGetMode();

  test_cfgNetworkGetMask();

  test_cfgNetworkSetMask();
//return;
  /* 
   * main() 
   */
  ret = conf_manager_main(0,NULL);
  CU_ASSERT(CFG_AOK == ret)

  /* 
   * For coverage only. 
   */
  exit_handler();
  configSigHandler(9);


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
      if ((NULL == CU_add_test(pSuite1, "\ntest_conf_agent_main...",test_conf_agent_main))) {
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
