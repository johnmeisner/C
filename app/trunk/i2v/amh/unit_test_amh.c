
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
#include "amh.h"
#include "ntcip-1218.h"
#include "conf_table.h"

/**************************************************************************************************
* Defines
***************************************************************************************************/
/* Record in RAM list: Needs to support 4.1 and ntcip-1218 */
typedef struct amhStoredRec_t {
    amhBitmaskType  encoding;  /* data.payload[1] == dsrcMsgId. */
    uint32_t        psid;
    uint8_t         priority;
    bool_t          continuous;  /* never was supported. only if not in bcastLockStep */
    uint8_t         channel;
    uint8_t         interval;
    uint32_t        startMin;   /* RSU 4.1 */
    uint32_t        endMin;
    uint8_t         deliveryStart[MIB_DATEANDTIME_LENGTH]; /* NTCIP-1218 */
    uint8_t         deliveryStop[MIB_DATEANDTIME_LENGTH];
    uint32_t        lastSend;    /* Internal use, not needed.*/
    bool_t          signature;   /* security determined by i2v but messages that don't agree will be ignored */
    bool_t          encryption;  /* encryption not supported by i2v but messages that don't agree will be ignored */
    bool_t          active; /* RSU 4.1 MIB RSU-MIB.txt */
    payloadType     data;
    char_t          myName[RSU_MSG_FILENAME_LENGTH_MAX]; /* record file name.*/
} __attribute__((packed)) amhStoredRecT;

/* mandatory logging defines */
#define MY_ERR_LEVEL   LEVEL_INFO    /* from i2v_util.h */
#define MY_NAME        "unit"
/**************************************************************************************************
* Globals
***************************************************************************************************/
/* unit_test_ahm.c */
static i2vShmMasterT * UnitshmPtr;
inWSMType   Wsm;
inWSMType * Wsm_null;

/* amh.c */
extern bool_t           mainloop;
extern cfgItemsT       cfg;      
extern amhCfgItemsT    amhCfg;   
extern uint32_t         amhSendImmediate_Sig;
extern bool_t           mainloop;
extern char_t           confFileName[I2V_CFG_MAX_STR_LEN];
extern amhStoredRecT    amhstoredb[RSU_SAR_MESSAGE_MAX];
extern amcTrackPSIDRec  myAMCpsidlist[RSU_SAR_MESSAGE_MAX]; 

extern uint16_t        amhstoredbcnt;
extern uint32_t        amh_shutdown_complete;

/**************************************************************************************************
* Protos & Externs
***************************************************************************************************/

/* amh.c: normally these are static types and not in .h for public consumption. */

extern void  amhFilterOut(void);
extern void  amhPrepareBcastMsgs(void);
extern bool_t amhBroadcastingStored(void);
extern void  amcMainProcess_one_shot(void);
extern void  amhSigHandler(int32_t __attribute__((unused)) sig);
extern void  amhInitStatics(void);

extern uint32_t getSvcName(uint32_t svcname);
extern void amhForwardManager(amhBitmaskType amh_msgType, uint8_t * buf, int32_t count);

extern uint32_t imfStoreAndRepeat_Sig;
extern uint32_t amhSendImmediate_Sig;
extern int32_t  amh_main(void);

extern int64_t DateAndTime_To_UTC_DSEC(uint8_t * DateAndTime, uint64_t * utcDsec );
extern int32_t check_stopStart_UTC_dsec(uint8_t * StartTime, uint8_t * StopTime, uint64_t UTC_msec, bool_t debugOn, int32_t index);

extern void amhUpdatePayload( char_t GCC_UNUSED_VAR * tag , char_t GCC_UNUSED_VAR * parsedItem , char_t * value , void * configItem , uint32_t * status);
extern void amhUpdateTxMode(char_t GCC_UNUSED_VAR *tag, char_t GCC_UNUSED_VAR *parsedItem, char_t *value, void *configItem, uint32_t *status);
extern void amhUpdateTxChannel(char_t GCC_UNUSED_VAR *tag, char_t GCC_UNUSED_VAR *parsedItem, char_t *value, void *configItem, uint32_t *status);
extern void amhUpdateType(char_t GCC_UNUSED_VAR *tag, char_t GCC_UNUSED_VAR *parsedItem, char_t *value, void *configItem, uint32_t *status);
extern void amhUpdateMinute(char_t GCC_UNUSED_VAR *tag, char_t GCC_UNUSED_VAR *parsedItem, char_t *value, void *configItem, uint32_t *status);
extern void amhUpdateBool(char_t GCC_UNUSED_VAR *tag, char_t GCC_UNUSED_VAR *parsedItem, char_t *value, void *configItem, uint32_t *status);

/* imf.c */
extern int32_t imfSockInit(void);
extern int32_t Process_Store_and_Repeat(rawImmediateType * msg, float32_t version, char_t * buf, uint32_t size, char_t * start_date_buffer, char_t * stop_date_buffer);
extern void imfParseBytes(char_t *buf, uint32_t size);
extern void *imfThread(void __attribute__((unused)) *arg);
extern void imf_init_statics(void);

/* test.c */
void test_srm_update_cfg(void);
void test_asn1DecodeUPER_MessageFrame(void);
void test_asn1DecodePER_SRM(void);
void test_srmToGS2(void);
void test_receive_srm(void);
void test_main(void);
int32_t init_suite(void);
int32_t clean_suite(void);

//TODO ; Add flavors
/**************************************************************************************************
* Function Bodies
***************************************************************************************************/

void test_amhSigHandler(void)
{
  amhCfg.enableImf = WTRUE;

  amhSigHandler(0);
}
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void test_main(void)
{
  uint32_t i,j;
  int32_t ret;

  /* Check for pending tests that did not complete.
   * Set tickle mask and ack mask for test.  
   */
  CU_ASSERT(0x0 == check_stub_pending_signals());
  clear_all_stub_signal();

  amhInitStatics();
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
  strcpy(confFileName, "amh_bad.conf");
  ret = amh_main();
  CU_ASSERT(AMH_AOK == ret);

  CU_ASSERT(amhCfg.radioNum == I2V_RADIO_DEFAULT);
  CU_ASSERT(amhCfg.channelNum == I2V_RADIO_CHANNEL_DEFAULT);
  CU_ASSERT(amhCfg.hdrExtra == AMH_WSM_EXTENSION_DEFAULT);
  CU_ASSERT(amhCfg.validateData == AMH_ENABLE_VERIFICATION_DEFAULT);
  CU_ASSERT(amhCfg.EnableSAR == AMH_ENABLE_SAR_DEFAULT);
  CU_ASSERT(amhCfg.bcastGeneric == AMH_SETTINGS_OVERRIDE_DEFAULT);
  CU_ASSERT(amhCfg.psidMatch == AMH_FORCE_PSID_DEFAULT);
  CU_ASSERT(amhCfg.lastChanceSend == AMH_SEND_OVERRIDE_DEFAULT);
  CU_ASSERT(amhCfg.enableImf == AMH_ENABLE_IMF_DEFAULT);
  CU_ASSERT(amhCfg.restrictIP == AMH_IMF_IP_FILTER_DEFAULT);
  CU_ASSERT(strcmp(amhCfg.imfIP,AMH_IMF_IP_DEFAULT_S) == 0);
  CU_ASSERT(amhCfg.imfPort == I2V_IMF_PORT_DEFAULT); 
  CU_ASSERT(amhCfg.amhFwdEnable == I2V_AMH_FORWARD_ENABLE_DEFAULT);
  //CU_ASSERT(strcmp(amhCfg.amhFwdIp,AMH_FWD_IP_DEFAULT_S) == 0);
  //CU_ASSERT(amhCfg.amhFwdPort == I2V_AMH_FWD_PORT_DEFAULT);
  CU_ASSERT(amhCfg.amhIntervalSelect == AMH_INTERVAL_SELECT_DEFAULT);

/* Get us 62% of coverage. */

  /* Reset cfgs. */
  strncpy(UnitshmPtr->cfgData.config_directory,DEFAULT_CONFIG,I2V_CFG_MAX_STR_LEN); /* Default conf used in install. */
  strcpy(confFileName, AMH_CFG_FILE);

  /* Proves conf_table.h matches amh.conf. */
  ret = amh_main();
  CU_ASSERT(AMH_AOK == ret);
  CU_ASSERT(amhCfg.radioNum == I2V_RADIO_DEFAULT);
  CU_ASSERT(amhCfg.channelNum == I2V_RADIO_CHANNEL_DEFAULT);
  CU_ASSERT(amhCfg.hdrExtra == AMH_WSM_EXTENSION_DEFAULT);
  CU_ASSERT(amhCfg.validateData == AMH_ENABLE_VERIFICATION_DEFAULT);
  CU_ASSERT(amhCfg.EnableSAR == AMH_ENABLE_SAR_DEFAULT);
  CU_ASSERT(amhCfg.bcastGeneric == AMH_SETTINGS_OVERRIDE_DEFAULT);
  CU_ASSERT(amhCfg.psidMatch == AMH_FORCE_PSID_DEFAULT);
  CU_ASSERT(amhCfg.lastChanceSend == AMH_SEND_OVERRIDE_DEFAULT);
  CU_ASSERT(amhCfg.enableImf == AMH_ENABLE_IMF_DEFAULT);
  CU_ASSERT(amhCfg.restrictIP == AMH_IMF_IP_FILTER_DEFAULT);
  CU_ASSERT(strcmp(amhCfg.imfIP,AMH_IMF_IP_DEFAULT_S) == 0);
  CU_ASSERT(amhCfg.imfPort == I2V_IMF_PORT_DEFAULT); 
  CU_ASSERT(amhCfg.amhFwdEnable == I2V_AMH_FORWARD_ENABLE_DEFAULT);
  //CU_ASSERT(strcmp(amhCfg.amhFwdIp,AMH_FWD_IP_DEFAULT_S) == 0);
  //CU_ASSERT(amhCfg.amhFwdPort == I2V_AMH_FWD_PORT_DEFAULT);
  CU_ASSERT(amhCfg.amhIntervalSelect == AMH_INTERVAL_SELECT_DEFAULT);

  amhCfg.radioNum       = 0;   
  amhCfg.channelNum    = 172;
  amhCfg.hdrExtra       = 0;
  amhCfg.logRate        = 0;
  amhCfg.validateData   = 1;
  amhCfg.EnableSAR      = 1;
  amhCfg.bcastGeneric   = 1;
  amhCfg.psidMatch      = 0;
  amhCfg.lastChanceSend = 1;
  amhCfg.enableImf      = 1;
  amhCfg.restrictIP     = 0;

  amhCfg.imfPort        = 1516;
  amhCfg.amhFwdEnable   = 0x7fffffff;

  //amhCfg.amhFwdPort     = 11001;
  amhCfg.amhIntervalSelect = 0;

  strncpy(amhCfg.imfIP,"192.168.2.47", I2V_CFG_MAX_STR_LEN);
  //strncpy(amhCfg.amhFwdIp,"192.168.2.47", I2V_CFG_MAX_STR_LEN);

  cfg.scsAppEnable = 1;
  //cfg.srmAppEnable = 1;
  cfg.spatAppEnable = 1;
  cfg.ipbAppEnable = 1;
  cfg.amhAppEnable = 1;
  cfg.iwmhAppEnable = 0;

  cfg.wsaEnable = 1;
  cfg.radio0wmh.radioChannel=172;
  cfg.radio1wmh.radioChannel=172;
  cfg.radio2wmh.radioChannel=172;
  cfg.radio3wmh.radioChannel=172;
  cfg.bcastLockStep =1;
  cfg.uchannel= 172;
  cfg.uradioNum = 0;
  cfg.uwsaPriority = 20;
  cfg.security = 0;
  cfg.iwmhAppEnable = 0;

  cfg.secSpatPsid = 0x8002;
  cfg.secMapPsid = 0x8002;
  cfg.secTimPsid = 0x8003;

  amhstoredb[0].encoding    = AMH_MSG_TEST15_MASK;
  amhstoredb[0].psid        = 0x8002;
  amhstoredb[0].priority    = 0x7;
  amhstoredb[0].continuous  = 0x0;
  amhstoredb[0].channel     = 172;
  amhstoredb[0].interval    = 1;
  amhstoredb[0].startMin    = 0xFFFF;
  amhstoredb[0].endMin      = 0x1234FFFF;
  amhstoredb[0].lastSend    = 0x20000;
  amhstoredb[0].signature   = WFALSE;
  amhstoredb[0].encryption  = WFALSE;
  amhstoredb[0].active      = WTRUE;
  amhstoredb[0].myName[0]      = 'a';
  amhstoredb[0].myName[1]      = 'm';
  amhstoredb[0].myName[2]      = 'h';
  amhstoredb[0].myName[3]      = '\0';
  amhstoredb[0].data.count  = MAX_WSM_DATA;
  amhstoredbcnt = RSU_SAR_MESSAGE_MAX;

  amhCfg.enableImf = WTRUE;
  UnitshmPtr->amhImmediateBsm.newmsg = WTRUE;
  amh_main();

  for(i=0,j=0;i<RSU_SAR_MESSAGE_MAX;i++){
      amhstoredb[i].encoding = i;
      amhstoredb[i].psid     = 0x8000 + i;
      amhstoredb[i].active     = 1;

      {
        amhstoredb[i].continuous = 1; 
        amhstoredb[i].signature  = WFALSE;  
        amhstoredb[i].encryption = WFALSE;  
        
      }


      amhstoredb[i].priority   = 0x7;
      amhstoredb[i].channel    = 172;
      amhstoredb[i].interval   = j;

      if(6 == j)
        j=1;
      else
        j++;

      amhstoredb[i].startMin   = 0xFF;
      amhstoredb[i].endMin     = 0XFFFFFFFF;
      amhstoredb[i].lastSend   = 0xFF;
      amhstoredb[i].data.count = 6;
      amhstoredb[i].data.payload[0] = '0';
      amhstoredb[i].data.payload[1] = '0';
      amhstoredb[i].data.payload[2] = '1';
      amhstoredb[i].data.payload[3] = '4';
      amhstoredb[i].data.payload[4] = 'C';
      amhstoredb[i].data.payload[5] = 'A';

      amhstoredb[i].myName[0] = 'a';
      amhstoredb[i].myName[0] = 'm';
      amhstoredb[i].myName[0] = 'h';
      amhstoredb[i].myName[0] = '\0';
  }

  amhstoredbcnt = RSU_SAR_MESSAGE_MAX;
  UnitshmPtr->amhImmediateBsm.newmsg = WTRUE;


  amh_main();
  amhstoredbcnt = RSU_SAR_MESSAGE_MAX;

  UnitshmPtr->amhImmediatePsm.newmsg = WTRUE;
  UnitshmPtr->amhImmediateBsm.newmsg = WTRUE; 

  for(i=0,j=0;i<RSU_SAR_MESSAGE_MAX;i++){
      amhstoredb[i].active     = 1;
  }
  
  
  amh_main();
  amhstoredbcnt = RSU_SAR_MESSAGE_MAX;

  amhCfg.enableImf         = 0;
  amhCfg.amhFwdEnable      = 0;
  amhCfg.amhIntervalSelect = 1;
  cfg.iwmhAppEnable        = 1;

  UnitshmPtr->scsCfgData.localSignalControllerPort = amhCfg.imfPort;
  for(i=0,j=0;i<RSU_SAR_MESSAGE_MAX;i++){
      amhstoredb[i].active     = 1;
  }
  
  
  amh_main();
  amhstoredbcnt = RSU_SAR_MESSAGE_MAX;

  amhCfg.enableImf         = 1;
  amhCfg.amhFwdEnable      = 1;
  amhCfg.amhIntervalSelect = 0;
  cfg.iwmhAppEnable        = 0;
  cfg.spatAppEnable        = 0;
  for(i=0,j=0;i<RSU_SAR_MESSAGE_MAX;i++){
      amhstoredb[i].active     = 1;
  }
  
  
  amh_main();
  amhstoredbcnt = RSU_SAR_MESSAGE_MAX;
  cfg.spatAppEnable        = 1;
  for(i=0,j=0;i<RSU_SAR_MESSAGE_MAX;i++){
      amhstoredb[i].active     = 1;
  }
  
  
  amh_main();
  amhstoredbcnt = RSU_SAR_MESSAGE_MAX;
  cfg.spatAppEnable        = 1;


  for(i=0,j=0;i<RSU_SAR_MESSAGE_MAX;i++){
      amhstoredb[i].encoding = AMH_MSG_TIM_MASK;
      amhstoredb[i].active     = 1;
  }

  //shmPtr->amhImmediateTim.newmsg = WFALSE;
  amhstoredbcnt = RSU_SAR_MESSAGE_MAX;
  
  
  amh_main();

  cfg.spatAppEnable        = 1;

  for(i=0,j=0;i<RSU_SAR_MESSAGE_MAX;i++){
      amhstoredb[i].encoding = AMH_MSG_MAP_MASK;
      amhstoredb[i].active     = 1;
  }
  UnitshmPtr->amhImmediateBsm.newmsg = WTRUE;

  for(i=0,j=0;i<RSU_SAR_MESSAGE_MAX;i++){

      amhstoredb[i].psid     = 0x8002;


      {
        amhstoredb[i].continuous = 1; 
        amhstoredb[i].signature  = WFALSE;  
        amhstoredb[i].encryption = WFALSE;  
        
      }


      amhstoredb[i].priority   = 0x7;
      amhstoredb[i].channel    = 172;
      amhstoredb[i].interval   = j;

      if(6 == j) {
        j=1;
        amhstoredb[i].channel    = 175;
      }
      else
        j++;

      amhstoredb[i].startMin   = 0xFF;
      amhstoredb[i].endMin     = 0XFFFFFFFF;
      amhstoredb[i].lastSend   = 0xFF;
      amhstoredb[i].data.count = 6;
      amhstoredb[i].data.payload[0] = '0';
      amhstoredb[i].data.payload[1] = '0';
      amhstoredb[i].data.payload[2] = '1';
      amhstoredb[i].data.payload[3] = '4';
      amhstoredb[i].data.payload[4] = 'C';
      amhstoredb[i].data.payload[5] = 'A';

      amhstoredb[i].myName[0] = 'a';
      amhstoredb[i].myName[0] = 'm';
      amhstoredb[i].myName[0] = 'h';
      amhstoredb[i].myName[0] = '\0';
  }

  amhstoredbcnt = RSU_SAR_MESSAGE_MAX;
  mainloop = WTRUE;
  
  
  amh_main();

  for(i=0,j=0;i<RSU_SAR_MESSAGE_MAX;i++){
      amhstoredb[i].active     = 1;
  }

  amhCfg.psidMatch         = 1;
  cfg.iwmhAppEnable        = 1;
  amhCfg.amhIntervalSelect = 1;
  amhCfg.amhFwdEnable      = 1;

  /*
     default start:
       07e6-01-01:00:00:00.1
       2022-01-01:00:00:00.1
 
     default stop:
       0833-0c-1f-17:3b:3b.9
       2099-12-31:23:59:59.9
  */
  {
    uint8_t start[8];
    uint8_t stop[8];
    //uint64_t utcDsec = 0x0;
    uint64_t UTC_msec = (uint64_t)1653002175 * (uint64_t)1000;

    start[0] = 0x07;
    start[1] = 0xe6;
    start[2] = 0x01;
    start[3] = 0x01;
    start[4] = 0x00;
    start[5] = 0x00;
    start[6] = 0x00;
    start[7] = 0x01;

    stop[0] = 0x08;
    stop[1] = 0x33;
    stop[2] = 0x0C;
    stop[3] = 0x1F;
    stop[4] = 0x17;
    stop[5] = 0x3B;
    stop[6] = 0x3B;
    stop[7] = 0x09;
    ret = check_stopStart_UTC_dsec(start, stop, UTC_msec,WFALSE,0);
    CU_ASSERT(1 == ret);
  }
  /* Clear tickle mask for next test. 
   * Check ack mask is complete
   */
  CU_ASSERT(0x0 == check_stub_pending_signals());
  clear_all_stub_signal();

}


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void test_amhUpdateType(void)
{

  uint32_t         status = 0xffffffff;
  amhBitmaskType  type;
  char_t          value[100];

  amhUpdateType(NULL,NULL,NULL,NULL,NULL); 
  CU_ASSERT(0xffffffff == status);            
  amhUpdateType(NULL,NULL,value,NULL,NULL);
  CU_ASSERT(0xffffffff == status);            
  amhUpdateType(NULL,NULL,value,&type,NULL);
  CU_ASSERT(0xffffffff == status);            

  memcpy(value,"SPLAT",sizeof("SPLAT"));
  amhUpdateType(NULL,NULL,value,&type,&status);
  CU_ASSERT(I2V_RETURN_FAIL      == status);
  CU_ASSERT(AMH_MSG_UNKNOWN_MASK == type);

  memcpy(value,"SPAT",sizeof("SPAT"));
  amhUpdateType(NULL,NULL,value,&type,&status);
  CU_ASSERT(I2V_RETURN_OK     == status);
  CU_ASSERT(AMH_MSG_SPAT_MASK == type);


  memcpy(value,"MAP",sizeof("MAP"));
  amhUpdateType(NULL,NULL,value,&type,&status);
  CU_ASSERT(I2V_RETURN_OK     == status);
  CU_ASSERT(AMH_MSG_MAP_MASK == type);

  memcpy(value,"TIM",sizeof("TIM"));
  amhUpdateType(NULL,NULL,value,&type,&status);
  CU_ASSERT(I2V_RETURN_OK     == status);
  CU_ASSERT(AMH_MSG_TIM_MASK == type);

  memcpy(value,"RTCM",sizeof("RTCM"));
  amhUpdateType(NULL,NULL,value,&type,&status);
  CU_ASSERT(I2V_RETURN_OK     == status);
  CU_ASSERT(AMH_MSG_RTCM_MASK == type);

  memcpy(value,"SPAT16",sizeof("SPAT16"));
  amhUpdateType(NULL,NULL,value,&type,&status);
  CU_ASSERT(I2V_RETURN_OK     == status);
  CU_ASSERT(AMH_MSG_SPAT_MASK == type);

  memcpy(value,"MAP16",sizeof("MAP16"));
  amhUpdateType(NULL,NULL,value,&type,&status);
  CU_ASSERT(I2V_RETURN_OK     == status);
  CU_ASSERT(AMH_MSG_MAP_MASK == type);

  memcpy(value,"TIM16",sizeof("TIM16"));
  amhUpdateType(NULL,NULL,value,&type,&status);
  CU_ASSERT(I2V_RETURN_OK     == status);
  CU_ASSERT(AMH_MSG_TIM_MASK == type);

  memcpy(value,"RTCM16",sizeof("RTCM16"));
  amhUpdateType(NULL,NULL,value,&type,&status);
  CU_ASSERT(I2V_RETURN_OK     == status);
  CU_ASSERT(AMH_MSG_RTCM_MASK == type);

  memcpy(value,"ICA",sizeof("ICA"));
  amhUpdateType(NULL,NULL,value,&type,&status);
  CU_ASSERT(I2V_RETURN_OK     == status);
  CU_ASSERT(AMH_MSG_ICA_MASK == type);

  memcpy(value,"PDM",sizeof("PDM"));
  amhUpdateType(NULL,NULL,value,&type,&status);
  CU_ASSERT(I2V_RETURN_OK     == status);
  CU_ASSERT(AMH_MSG_PDM_MASK == type);

  memcpy(value,"RSA",sizeof("RSA"));
  amhUpdateType(NULL,NULL,value,&type,&status);
  CU_ASSERT(I2V_RETURN_OK     == status);
  CU_ASSERT(AMH_MSG_RSA_MASK == type);

  memcpy(value,"SSM",sizeof("SSM"));
  amhUpdateType(NULL,NULL,value,&type,&status);
  CU_ASSERT(I2V_RETURN_OK     == status);
  CU_ASSERT(AMH_MSG_SSM_MASK == type);

  memcpy(value,"CSR",sizeof("CSR"));
  amhUpdateType(NULL,NULL,value,&type,&status);
  CU_ASSERT(I2V_RETURN_OK     == status);
  CU_ASSERT(AMH_MSG_CSR_MASK == type);

  memcpy(value,"EVA",sizeof("EVA"));
  amhUpdateType(NULL,NULL,value,&type,&status);
  CU_ASSERT(I2V_RETURN_OK     == status);
  CU_ASSERT(AMH_MSG_EVA_MASK == type);

  memcpy(value,"NMEA",sizeof("NMEA"));
  amhUpdateType(NULL,NULL,value,&type,&status);
  CU_ASSERT(I2V_RETURN_OK     == status);
  CU_ASSERT(AMH_MSG_NMEA_MASK == type);

  memcpy(value,"PSM",sizeof("PSM"));
  amhUpdateType(NULL,NULL,value,&type,&status);
  CU_ASSERT(I2V_RETURN_OK     == status);
  CU_ASSERT(AMH_MSG_PSM_MASK == type);

  memcpy(value,"PVD",sizeof("PVD"));
  amhUpdateType(NULL,NULL,value,&type,&status);
  CU_ASSERT(I2V_RETURN_OK     == status);
  CU_ASSERT(AMH_MSG_PVD_MASK == type);

  memcpy(value,"SRM",sizeof("SRM"));
  amhUpdateType(NULL,NULL,value,&type,&status);
  CU_ASSERT(I2V_RETURN_OK     == status);
  CU_ASSERT(AMH_MSG_SRM_MASK == type);

  memcpy(value,"BSM",sizeof("BSM"));
  amhUpdateType(NULL,NULL,value,&type,&status);
  CU_ASSERT(I2V_RETURN_OK     == status);
  CU_ASSERT(AMH_MSG_BSM_MASK == type);


  memcpy(value,"TEST00",sizeof("TEST00"));
  amhUpdateType(NULL,NULL,value,&type,&status);
  CU_ASSERT(I2V_RETURN_OK     == status);
  CU_ASSERT(AMH_MSG_TEST00_MASK == type);

  memcpy(value,"TEST01",sizeof("TEST0"));
  amhUpdateType(NULL,NULL,value,&type,&status);
  CU_ASSERT(I2V_RETURN_OK     == status);
  CU_ASSERT(AMH_MSG_TEST01_MASK == type);

  memcpy(value,"TEST02",sizeof("TEST02"));
  amhUpdateType(NULL,NULL,value,&type,&status);
  CU_ASSERT(I2V_RETURN_OK     == status);
  CU_ASSERT(AMH_MSG_TEST02_MASK == type);

  memcpy(value,"TEST03",sizeof("TEST03"));
  amhUpdateType(NULL,NULL,value,&type,&status);
  CU_ASSERT(I2V_RETURN_OK     == status);
  CU_ASSERT(AMH_MSG_TEST03_MASK == type);

  memcpy(value,"TEST04",sizeof("TEST04"));
  amhUpdateType(NULL,NULL,value,&type,&status);
  CU_ASSERT(I2V_RETURN_OK     == status);
  CU_ASSERT(AMH_MSG_TEST04_MASK == type);

  memcpy(value,"TEST05",sizeof("TEST05"));
  amhUpdateType(NULL,NULL,value,&type,&status);
  CU_ASSERT(I2V_RETURN_OK     == status);
  CU_ASSERT(AMH_MSG_TEST05_MASK == type);

  memcpy(value,"TEST06",sizeof("TEST06"));
  amhUpdateType(NULL,NULL,value,&type,&status);
  CU_ASSERT(I2V_RETURN_OK     == status);
  CU_ASSERT(AMH_MSG_TEST06_MASK == type);

  memcpy(value,"TEST07",sizeof("TEST07"));
  amhUpdateType(NULL,NULL,value,&type,&status);
  CU_ASSERT(I2V_RETURN_OK     == status);
  CU_ASSERT(AMH_MSG_TEST07_MASK == type);

  memcpy(value,"TEST08",sizeof("TEST08"));
  amhUpdateType(NULL,NULL,value,&type,&status);
  CU_ASSERT(I2V_RETURN_OK     == status);
  CU_ASSERT(AMH_MSG_TEST08_MASK == type);

  memcpy(value,"TEST09",sizeof("TEST09"));
  amhUpdateType(NULL,NULL,value,&type,&status);
  CU_ASSERT(I2V_RETURN_OK     == status);
  CU_ASSERT(AMH_MSG_TEST09_MASK == type);

  memcpy(value,"TEST10",sizeof("TEST10"));
  amhUpdateType(NULL,NULL,value,&type,&status);
  CU_ASSERT(I2V_RETURN_OK     == status);
  CU_ASSERT(AMH_MSG_TEST10_MASK == type);

  memcpy(value,"TEST11",sizeof("TEST11"));
  amhUpdateType(NULL,NULL,value,&type,&status);
  CU_ASSERT(I2V_RETURN_OK     == status);
  CU_ASSERT(AMH_MSG_TEST11_MASK == type);

  memcpy(value,"TEST12",sizeof("TEST12"));
  amhUpdateType(NULL,NULL,value,&type,&status);
  CU_ASSERT(I2V_RETURN_OK     == status);
  CU_ASSERT(AMH_MSG_TEST12_MASK == type);

  memcpy(value,"TEST13",sizeof("TEST113"));
  amhUpdateType(NULL,NULL,value,&type,&status);
  CU_ASSERT(I2V_RETURN_OK     == status);
  CU_ASSERT(AMH_MSG_TEST13_MASK == type);

  memcpy(value,"TEST14",sizeof("TEST14"));
  amhUpdateType(NULL,NULL,value,&type,&status);
  CU_ASSERT(I2V_RETURN_OK     == status);
  CU_ASSERT(AMH_MSG_TEST14_MASK == type);

  memcpy(value,"TEST00",sizeof("TEST00"));
  amhUpdateType(NULL,NULL,value,&type,&status);
  CU_ASSERT(I2V_RETURN_OK     == status);
  CU_ASSERT(AMH_MSG_TEST00_MASK == type);

  memcpy(value,"TEST15",sizeof("TEST15"));
  amhUpdateType(NULL,NULL,value,&type,&status);
  CU_ASSERT(I2V_RETURN_OK     == status);
  CU_ASSERT(AMH_MSG_TEST15_MASK == type);

}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


void test_amhUpdateTxMode(void)
{
  uint32_t    status = 0xffffffff;
  bool_t      type;
  char_t     value[100];

  amhUpdateTxMode(NULL,NULL,NULL,NULL,NULL); 
  CU_ASSERT(0xffffffff == status);            
  amhUpdateTxMode(NULL,NULL,value,NULL,NULL);
  CU_ASSERT(0xffffffff == status);            
  amhUpdateTxMode(NULL,NULL,value,&type,NULL);
  CU_ASSERT(0xffffffff == status);  

  memcpy(value,"CONT",sizeof("CONT"));
  amhUpdateTxMode(NULL,NULL,value,&type,&status);
  CU_ASSERT(I2V_RETURN_OK == status);
  CU_ASSERT(WTRUE == type);

  memcpy(value,"ALT",sizeof("ALT"));
  amhUpdateTxMode(NULL,NULL,value,&type,&status);
  CU_ASSERT(I2V_RETURN_OK == status);
  CU_ASSERT(WFALSE == type);

  memcpy(value,"",sizeof(""));
  amhUpdateTxMode(NULL,NULL,value,&type,&status);
  CU_ASSERT(I2V_RETURN_OK != status);
  //CU_ASSERT(WFALSE == type);



}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


void test_amhUpdateTxChannel(void)
{
  uint32_t   status = 0xffffffff;
  uint8_t    type;
  char_t     value[100];


  amhUpdateTxChannel(NULL,NULL,NULL,NULL,NULL); 
  CU_ASSERT(0xffffffff == status);            
  amhUpdateTxChannel(NULL,NULL,value,NULL,NULL);
  CU_ASSERT(0xffffffff == status);            
  amhUpdateTxChannel(NULL,NULL,value,&type,NULL);
  CU_ASSERT(0xffffffff == status);

  memcpy(value,"CCH",sizeof("CCH"));
  amhUpdateTxChannel(NULL,NULL,value,&type,&status);
  CU_ASSERT(I2V_RETURN_OK == status);
  //printf("\nCUNIT:[%u,%u]=CCH type\n",type,I2V_CONTROL_CHANNEL);
  CU_ASSERT(I2V_CONTROL_CHANNEL == type);

  amhCfg.channelNum = 172;
  memcpy(value,"SCH",sizeof("SCH"));
  amhUpdateTxChannel(NULL,NULL,value,&type,&status);
  CU_ASSERT(I2V_RETURN_OK == status);
 // printf("\nCUNIT:[%u,%u]=SCH type\n",type,172);
  CU_ASSERT(AMH_NEED_SCH == type);

  memcpy(value,"175",sizeof("175"));
  amhUpdateTxChannel(NULL,NULL,value,&type,&status);
  CU_ASSERT(I2V_RETURN_OK == status);
  CU_ASSERT(175 == type);


  memcpy(value,"111",sizeof("111"));
  amhUpdateTxChannel(NULL,NULL,value,&type,&status);
  CU_ASSERT(I2V_RETURN_FAIL == status);
  //CU_ASSERT(175 == type);


}


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

void test_amhUpdateMinute(void)
{

  uint32_t    status = 0xffffffff;
  uint32_t    type;
  char_t     value[100];


  amhUpdateMinute(NULL,NULL,NULL,NULL,NULL); 
  CU_ASSERT(0xffffffff == status);            
  amhUpdateMinute(NULL,NULL,value,NULL,NULL);
  CU_ASSERT(0xffffffff == status);            
  amhUpdateMinute(NULL,NULL,value,&type,NULL);
  CU_ASSERT(0xffffffff == status);


  memcpy(value,"04/16/2017, 01:01",sizeof("04/16/2017, 01:01"));
  amhUpdateMinute(NULL,NULL,value,&type,&status);
  CU_ASSERT(I2V_RETURN_OK == status);
  CU_ASSERT(0 != type);

  memcpy(value,"",sizeof(""));
  amhUpdateMinute(NULL,NULL,value,&type,&status);
  CU_ASSERT(I2V_RETURN_OK == status);
  CU_ASSERT(0 != type);

  memcpy(value,"04/16/2017, ",sizeof("04/16/2017, "));
  type = 0x0;
  amhUpdateMinute(NULL,NULL,value,&type,&status);
  CU_ASSERT(I2V_RETURN_OK != status);
  CU_ASSERT(0 == type);


  memcpy(value,"04/16/, 01:01",sizeof("04/16/, 01:01"));
  type = 0x0;
  amhUpdateMinute(NULL,NULL,value,&type,&status);
  CU_ASSERT(I2V_RETURN_OK != status);
  CU_ASSERT(0 == type);

  memcpy(value,"04/, 01:01",sizeof("04/, 01:01"));
  type = 0x0;
  amhUpdateMinute(NULL,NULL,value,&type,&status);
  CU_ASSERT(I2V_RETURN_OK != status);
  CU_ASSERT(0 == type);

  memcpy(value,"/, 01:01",sizeof("/, 01:01"));
  type = 0x0;
  amhUpdateMinute(NULL,NULL,value,&type,&status);
  CU_ASSERT(I2V_RETURN_OK != status);
  CU_ASSERT(0 == type);

}


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


void test_amhUpdateBool(void)
{

  uint32_t   status = 0xffffffff;
  bool_t     type;
  char_t     value[100];


  amhUpdateBool(NULL,NULL,NULL,NULL,NULL); 
  CU_ASSERT(0xffffffff == status);            
  amhUpdateBool(NULL,NULL,value,NULL,NULL);
  CU_ASSERT(0xffffffff == status);            
  amhUpdateBool(NULL,NULL,value,&type,NULL);
  CU_ASSERT(0xffffffff == status);

  memcpy(value,"False",sizeof("False"));
  type = WTRUE;
  amhUpdateBool(NULL,NULL,value,&type,&status);
  CU_ASSERT(I2V_RETURN_OK == status);
  CU_ASSERT(WFALSE == type);

  memcpy(value,"True",sizeof("True"));
  type = WTRUE;
  amhUpdateBool(NULL,NULL,value,&type,&status);
  CU_ASSERT(I2V_RETURN_OK == status);
  CU_ASSERT(WTRUE == type);

  memcpy(value,"Fargy",sizeof("Fargy"));
  type = WFALSE;
  amhUpdateBool(NULL,NULL,value,&type,&status);
  CU_ASSERT(I2V_RETURN_FAIL == status);
  CU_ASSERT(WFALSE == type);

}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


void test_amhUpdatePayload(void)
{

  uint32_t      status = 0xffffffff;
  payloadType  type;
  char_t       value[RSU_MIB_MAX_PAYLOAD_OCTET_STRING+1]; //must have room for largest plus NULL
  uint32_t      i;


  type.count = 0; 
  memset(&(type.payload[0]), '\0',sizeof(MAX_WSM_DATA));
  memset(value,              '\0',sizeof(RSU_MIB_MAX_PAYLOAD_OCTET_STRING+1));

  for(i=0;i<(RSU_MIB_MAX_PAYLOAD_OCTET_STRING+1);i++) { //ascii values

      if(0 == (i % 2)) 
        value[i] = 'a';
      else
        value[i] = '4';
  }
  value[RSU_MIB_MAX_PAYLOAD_OCTET_STRING]    = '\0';
  value[RSU_MIB_MAX_PAYLOAD_OCTET_STRING -1] = 'C';
  value[RSU_MIB_MAX_PAYLOAD_OCTET_STRING -2] = 'D';

  amhUpdatePayload(NULL,NULL,value,&type,&status);
  CU_ASSERT(I2V_RETURN_OK == status);


  value[RSU_MIB_MAX_PAYLOAD_OCTET_STRING]    = 'F'; //one too big & odd number


  amhUpdatePayload(NULL,NULL,value,&type,&status);
  CU_ASSERT(I2V_RETURN_FAIL == status);

  value[4]    = '\0'; //too small

  amhUpdatePayload(NULL,NULL,value,&type,&status);
  CU_ASSERT(I2V_RETURN_FAIL == status);

  value[RSU_MIB_MAX_PAYLOAD_OCTET_STRING]    = '\0';
  value[4]    = 'G'; //not hex

  amhUpdatePayload(NULL,NULL,value,&type,&status);
  CU_ASSERT(I2V_RETURN_FAIL == status);

  amhUpdatePayload(NULL,NULL,NULL,&type,&status);
  CU_ASSERT(I2V_RETURN_FAIL == status);

  amhUpdatePayload(NULL,NULL,value,NULL,&status);
  CU_ASSERT(I2V_RETURN_FAIL == status);

  amhUpdatePayload(NULL,NULL,value,&type,NULL);
  CU_ASSERT(I2V_RETURN_FAIL == status);


}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void test_amhForwardManager(void)
{
  uint8_t buf[512];
  amhBitmaskType amh_msgType = AMH_MSG_MAP_MASK;

  amhForwardManager(amh_msgType, buf, 512);

}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void test_cleanup(void)
{
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

  ret = CU_initialize_registry();

  if(CUE_SUCCESS == ret) {
      if(NULL == (pSuite1 = CU_add_suite("Basic_Test_Suite1", init_suite, clean_suite))) {
          ret = ~CUE_SUCCESS;
      }
  }

  init_stub_control();


  if(CUE_SUCCESS == ret) {
      if ((NULL == CU_add_test(pSuite1, "\ntest_main...",test_main))) {
          ret = ~CUE_SUCCESS;
      }
  }

  if(CUE_SUCCESS == ret) {
      if ((NULL  == CU_add_test( pSuite1,"\ntest_amhForwardManager...",test_amhForwardManager))) {
          ret = ~CUE_SUCCESS;
      }
  }

  if(CUE_SUCCESS == ret) {
      if ((NULL  == CU_add_test( pSuite1,"\ntest_amhSigHandler...",test_amhSigHandler))) {
          ret = ~CUE_SUCCESS;
      }
  }

  if(CUE_SUCCESS == ret) {
      if ((NULL == CU_add_test(pSuite1, "\ntest_amhUpdateType...",test_amhUpdateType))) {
          ret = ~CUE_SUCCESS;
      }
  }

  if(CUE_SUCCESS == ret) {
      if ((NULL == CU_add_test(pSuite1, "\ntest_amhUpdateTxMode...",test_amhUpdateTxMode))) {
          ret = ~CUE_SUCCESS;
      }
  }

  if(CUE_SUCCESS == ret) {
      if ((NULL == CU_add_test(pSuite1, "\ntest_amhUpdateTxChannel...",test_amhUpdateTxChannel))) {
          ret = ~CUE_SUCCESS;
      }
  }

  if(CUE_SUCCESS == ret) {
      if ((NULL == CU_add_test(pSuite1, "\ntest_amhUpdateMinute...",test_amhUpdateMinute))) {
          ret = ~CUE_SUCCESS;
      }
  }

  if(CUE_SUCCESS == ret) {
      if ((NULL == CU_add_test(pSuite1, "\ntest_amhUpdateBool...",test_amhUpdateBool))) {
          ret = ~CUE_SUCCESS;
      }
  }

  if(CUE_SUCCESS == ret) {
      if ((NULL == CU_add_test(pSuite1, "\ntest_amhUpdatePayload...",test_amhUpdatePayload))) {
          ret = ~CUE_SUCCESS;
      }
  }

  if(CUE_SUCCESS == ret) {
     printf("\nCUnit CU_basic_run_tests...\n");
     ret = CU_basic_run_tests();
  } 

  i2v_setupSHM_Clean(UnitshmPtr);

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
