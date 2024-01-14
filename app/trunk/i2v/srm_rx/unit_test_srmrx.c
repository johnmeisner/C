
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
#include "SignalRequestMessage.h"
#endif
#include "i2v_riscapture.h"
#include "stubs.h"
#include "rs.h"
#include "srm_rx.h"
#include "conf_table.h"
#include "ntcip-1218.h"
/**************************************************************************************************
* Defines
***************************************************************************************************/
/* Test configs used. */
#define DEFAULT_CONFIG  "./"
#define NO_CONFIG       "../stubs/"

#define UT_FLAVOR_MYPCIP 1
#define UT_FLAVOR_ENABLEFWD 2
#define MY_ERR_LEVEL   LEVEL_INFO    /* from i2v_util.h */
#define MY_NAME        "Usrmrx"

#define PRINTIT printf("\nret=%d.",ret);
/**************************************************************************************************
* Globals
***************************************************************************************************/

/* srm_rx.c */
extern srmCfgItemsT  srm_cfg;
extern bool_t loop;
inWSMType   Wsm;
inWSMType * Wsm_null = NULL;
extern char_t  confFileName[I2V_CFG_MAX_STR_LEN];
extern uint64_t srm_error_states;

static i2vShmMasterT * UnitshmPtr; /* If you are not calling main() then you need to set SHM to this. */

/**************************************************************************************************
* Protos & Externs
***************************************************************************************************/

/* srm_rx.c: normally these are static types and not in .h for public consumption. */

extern int32_t asn1DecodeUPER_MessageFrame( uint8_t       * data 
                                         , int32_t         datalen
                                         , MessageFrame * msgFrameRx
                                         , int8_t          aligned
                                         , int32_t       * decodeLen
                                         , OSCTXT       * ctxt);

extern int32_t asn1DecodePER_SRM( uint8_t               * data 
                               , uint32_t                length 
                               , SignalRequestMessage * srm 
                               , int32_t               * size
                               , uint8_t                 aligned
                               , OSCTXT               * ctxt);

extern i2vReturnTypesT srm_update_cfg(  char_t          * filename
                                      , i2vShmMasterT   * local_shm_ptr                            
                                      , srmCfgItemsT    * local_srm_cfg
                                      , cfgItemsT       * local_cfg);

extern int32_t DecodeSRM(SignalRequestMessage *srm_rx_msg, SRMMsgType *SRMMsg);
extern int32_t    srm_main(void);
extern void   exitHandler(void);
extern void   receive_srm(inWSMType *Wsm);
extern void   exitHandler(void);
extern void   cleanup(cfgItemsT *cfg);

/* test.c */
void test_srm_update_cfg(void);
void test_asn1DecodeUPER_MessageFrame(void);
void test_asn1DecodePER_SRM(void);
void test_DecodeSRM(void);
void test_receive_srm(void);
void test_main(void);
int init_suite(void);
int clean_suite(void);

/**************************************************************************************************
* Function Bodies
***************************************************************************************************/
void * SRM_WSU_SHARE_INIT(size_t size, char_t *spath)
{
  if(1 == get_stub_signal(Signal_wsu_share_init)) {
      clear_stub_signal(Signal_wsu_share_init);
      return NULL;
  }
  return wsu_share_init(size, spath);
}
void test_srm_update_cfg(void)
{
  char_t              filename[] = "srm_rx.conf"; //must already exist
  cfgItemsT           cfg;

  CU_ASSERT(0x0 == check_stub_pending_signals());
  clear_all_stub_signal();

  strcpy(UnitshmPtr->cfgData.config_directory,"./");
  UnitshmPtr->cfgData.h.ch_data_valid = WTRUE;
  CU_ASSERT(I2V_RETURN_INVALID_PARAM == srm_update_cfg(NULL,NULL,NULL,NULL) );
  UnitshmPtr->cfgData.h.ch_data_valid = WTRUE;
  CU_ASSERT(I2V_RETURN_INVALID_PARAM == srm_update_cfg(filename,NULL,NULL,NULL) );
  UnitshmPtr->cfgData.h.ch_data_valid = WTRUE;
  CU_ASSERT(I2V_RETURN_INVALID_PARAM == srm_update_cfg(filename,UnitshmPtr,NULL,NULL) );
  UnitshmPtr->cfgData.h.ch_data_valid = WTRUE;
  CU_ASSERT(I2V_RETURN_INVALID_PARAM == srm_update_cfg(filename,UnitshmPtr,&srm_cfg,NULL) );

  strcpy(UnitshmPtr->cfgData.config_directory,"./");
  UnitshmPtr->cfgData.h.ch_data_valid = WTRUE;
  CU_ASSERT(I2V_RETURN_INVALID_PARAM == srm_update_cfg(filename,UnitshmPtr,&srm_cfg,NULL) );
  UnitshmPtr->cfgData.h.ch_data_valid = WTRUE;
  CU_ASSERT(I2V_RETURN_INV_FILE_OR_DIR == srm_update_cfg((char_t *)"foobar",UnitshmPtr,&srm_cfg,&cfg) ); //does not exist
  UnitshmPtr->cfgData.h.ch_data_valid = WTRUE;
  CU_ASSERT(I2V_RETURN_OK == srm_update_cfg(filename,UnitshmPtr,&srm_cfg,&cfg) );

  CU_ASSERT(0x0 == check_stub_pending_signals());
  clear_all_stub_signal();
}
static int unit_test_flavor = 0;
void unit_test_srmrx_config_twiddle(void)
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

void test_asn1DecodeUPER_MessageFrame(void)
{
  uint8_t         data[MAX_WSM_DATA];
  int32_t         datalen = MAX_WSM_DATA;
  MessageFrame   msgFrameRx;
  int8_t          aligned = 0;
  int32_t         decodeLen;
  OSCTXT         ctxt;
  i2vReturnTypesT ret; 

  
  CU_ASSERT(I2V_ASN1_FRAME_INVALID_PARAM == asn1DecodeUPER_MessageFrame( NULL, 0, NULL, 10, NULL, NULL) );

  datalen = sizeof(MessageFrame);
  ret = asn1DecodeUPER_MessageFrame( data ,datalen, &msgFrameRx, aligned, &decodeLen, &ctxt);
  CU_ASSERT(I2V_RETURN_OK == ret);

  datalen = MAX_WSM_DATA + 1;
  CU_ASSERT(I2V_ASN1_FRAME_INVALID_PARAM == asn1DecodeUPER_MessageFrame( data ,datalen, &msgFrameRx, aligned, &decodeLen, &ctxt) );
  

  /* Check for pending tests that did not complete.
   * Set tickle mask and ack mask for test.  
   */
  CU_ASSERT(0x0 == check_stub_pending_signals());
  clear_all_stub_signal();

  /* 
   * Call functions under test 
   */

  datalen = (int32_t)sizeof(MessageFrame); 

  set_stub_signal(Signal_asn1Init_MessageFrame);
  ret = asn1DecodeUPER_MessageFrame( data ,datalen, &msgFrameRx, aligned, &decodeLen, &ctxt);
  CU_ASSERT(I2V_ASN1_FRAME_INIT_FAIL == ret);

  set_stub_signal(Signal_pu_setBuffer);
  ret = asn1DecodeUPER_MessageFrame( data ,datalen, &msgFrameRx, aligned, &decodeLen, &ctxt);
  CU_ASSERT(I2V_ASN1_SET_BUFFER_FAIL == ret);

  set_stub_signal(Signal_asn1PD_MessageFrame);
  ret = asn1DecodeUPER_MessageFrame( data ,datalen, &msgFrameRx, aligned, &decodeLen, &ctxt);
  CU_ASSERT( I2V_ASN1_FRAME_DECODE_ERROR == ret);

  set_stub_signal(Signal_pu_getMsgLen_1);
  ret = asn1DecodeUPER_MessageFrame( data ,datalen, &msgFrameRx, aligned, &decodeLen, &ctxt);
  CU_ASSERT( I2V_ASN1_FRAME_ILLEGAL_SIZE == ret);

  set_stub_signal(Signal_pu_getMsgLen_2);
  ret = asn1DecodeUPER_MessageFrame( data ,datalen, &msgFrameRx, aligned, &decodeLen, &ctxt);
  CU_ASSERT( I2V_ASN1_FRAME_ILLEGAL_SIZE == ret);


  /* Clear tickle mask for next test. 
   * Check ack mask is complete
   */
  CU_ASSERT(0x0 == check_stub_pending_signals());
  clear_all_stub_signal();

}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

void test_asn1DecodePER_SRM(void)
{
  uint8_t                 data[MAX_WSM_DATA]; 
  uint32_t                length = MAX_WSM_DATA;
  SignalRequestMessage   srm; 
  int32_t                 size;
  uint8_t                 aligned = 0;
  OSCTXT                 ctxt;
  i2vReturnTypesT        ret; 

  CU_ASSERT(I2V_ASN1_SRM_INVALID_PARAM == asn1DecodePER_SRM( NULL, 0, NULL, NULL, 10, NULL) );

  length = sizeof(SignalRequestMessage);
  ret = asn1DecodePER_SRM( data, length, &srm, &size, aligned, &ctxt);
  CU_ASSERT(I2V_RETURN_OK == ret);

  length = MAX_WSM_DATA + 1;
  CU_ASSERT(I2V_ASN1_SRM_INVALID_PARAM == asn1DecodePER_SRM( data, length, &srm, &size, aligned, &ctxt) );

  /* Check for pending tests that did not complete.
   * Set tickle mask and ack mask for test.  
   */
  CU_ASSERT(0x0 == check_stub_pending_signals());
  clear_all_stub_signal();

  /* 
   * Call functions under test 
   */

  length = sizeof(SignalRequestMessage); 

  set_stub_signal(Signal_asn1Init_SignalRequestMessage_regional);
  ret = asn1DecodePER_SRM( data, length, &srm, &size, aligned, &ctxt);
  CU_ASSERT(I2V_ASN1_SRM_INIT_FAIL == ret);

  set_stub_signal(Signal_pu_setBuffer);
  ret = asn1DecodePER_SRM( data, length, &srm, &size, aligned, &ctxt);
  CU_ASSERT(I2V_ASN1_SET_BUFFER_FAIL == ret);

  set_stub_signal(Signal_asn1PD_SignalRequestMessage_1);
  ret = asn1DecodePER_SRM( data, length, &srm, &size, aligned, &ctxt);
  CU_ASSERT(I2V_ASN1_SRM_DECODE_ERROR == ret);

  set_stub_signal(Signal_pu_getMsgLen_2);
  ret = asn1DecodePER_SRM( data, length, &srm, &size, aligned, &ctxt);
  CU_ASSERT(I2V_ASN1_SRM_ILLEGAL_SIZE == ret);


  /* Clear tickle mask for next test. 
   * Check ack mask is complete
   */
  CU_ASSERT(0x0 == check_stub_pending_signals());
  clear_all_stub_signal();

}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

void test_DecodeSRM(void)
{
#if 0
  SignalRequestPackage test_sig_req_pkg; 
  TemporaryID          mytmpID;

  SignalRequestPackage *sig_req_pkg = NULL;
  OSRTDListNode        *node        = NULL;
  TemporaryID          *tmpID       = NULL;

#if 0
typedef struct EXTERN SignalRequestMessage {
   struct {
      OSUINT8 timeStampPresent : 1;
      OSUINT8 sequenceNumberPresent : 1;
      OSUINT8 requestsPresent : 1;
      OSUINT8 regionalPresent : 1;
   } m;
   OSUINT32 timeStamp;
   OSUINT16 second;
   OSUINT8 sequenceNumber;

SEQUENCE (SIZE (1..32)) OF SignalRequestPackage

   SignalRequestList requests;
   RequestorDescription requestor;
   SignalRequestMessage_regional regional;
   OSRTDList extElem1;
} SignalRequestMessage;
#endif

  OSRTDListNode my_node;

  VehicleID id;
  SignalRequestMessage srm_rx_msg; //There are dynamic portions of this struct we have not malloc'ed. This will seg fault if decoded as is.
  SRMMsgType           SRMMsg;

  memset(&test_sig_req_pkg,0,sizeof(test_sig_req_pkg));
  memset(&srm_rx_msg,0,sizeof(srm_rx_msg));
  memset(&SRMMsg,0,sizeof(SRMMsg));

  test_sig_req_pkg.m.durationPresent = 0x1;
  test_sig_req_pkg.duration = 1000;

#if 0
typedef struct OSRTDList {
   OSSIZE count;               /**< Count of items in the list. */
   OSRTDListNode* head;        /**< Pointer to first entry in list. */
   OSRTDListNode* tail;        /**< Pointer to last entry in list. */
} OSRTDList;
#endif

  srm_rx_msg.requests.count = 1;
  srm_rx_msg.requests.head = &my_node;
  srm_rx_msg.requests.tail = &my_node;

#if 0
typedef struct OSRTDListNode {
   void* data;            /**< Pointer to list data item. */
   struct OSRTDListNode* next; /**< Pointer to next node in list. */
   struct OSRTDListNode* prev; /**< Pointer to previous node in list. */
} OSRTDListNode;
#endif

  my_node.data = &test_sig_req_pkg;
  my_node.next = NULL;
  my_node.prev = NULL;

  mytmpID.numocts = 4;
  mytmpID.data[0] = 0XA;
  mytmpID.data[0] = 0XB;
  mytmpID.data[0] = 0XB;
  mytmpID.data[0] = 0XA;

  #if defined(J2735_2016)
  srm_rx_msg.requestor.id.u.entityID = &mytmpID;
  #endif
  #if defined(J2735_2023)
  if(NULL != srm_rx_msg.requestor.id) {
      srm_rx_msg.requestor.id->u.entityID = &mytmpID;
  } else {
      printf("\ntest_DecodeSRM: srm_rx_msg.requestor.id == NULL\n");
      memset(&id,0x0,sizeof(id));
      srm_rx_msg.requestor.id = &id;
      srm_rx_msg.requestor.id->u.entityID = &mytmpID;
  }
  #endif

  CU_ASSERT(I2V_RETURN_INVALID_PARAM == DecodeSRM(NULL, NULL));
  //CU_ASSERT(I2V_RETURN_INVALID_PARAM == DecodeSRM(&srm_rx_msg, &SRMMsg));
#endif
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/* SRM with ETA == Zero */
//uint8_t srm_eta_zero[MAX_WSM_DATA] = {0x00,0x1d,0x27,0x70,0xd2,0xca,0x90,0x39,0xb9,0x03,0x02,0xd1,0x41,0xf4,0x81,0xb1,0xa5,0x95,0x20,0x73,0x60,0x1b,0xc4,0x10,0x6b,0x00,0x19,0xa9,0x39,0xa4,0xa8,0xe4,0xa0,0x45,0x5f,0x02,0x10,0x80,0x00,0x00,0x00,0x00};
uint8_t srm_harbor[MAX_WSM_DATA] = {0x00,0x1d,0x28,0x73,0xe8,0x4c,0x92,0x6f,0x0a,0x03,0x81,0x1e,0x51,0xf4,0x96,0x7d
,0x09,0x94,0x0e,0x00,0x00,0x76,0x00,0x00,0x00,0x0f,0x9c,0x02,0x5a,0x92,0x38,0x6e
,0x46,0x4a,0xf7,0x3a,0xae,0x1d,0xff,0x39,0x3f,0x8b,0x3c};
void test_receive_srm(void)
{
  inWSMType     Wsm;

  srm_error_states = 0x0;
  receive_srm(NULL);
  CU_ASSERT(SRM_BAD_INPUT & srm_error_states);  

  srm_cfg.srmForward = 0x1; 
  srm_cfg.srmForward          = 0x1;
  srm_cfg.secDebug            = 0x0;
  srm_cfg.radioNum            = 0x0;
  srm_cfg.channel_number      = 172;
  //srm_cfg.srmPsidPeruUnsecure = 0x31;
  srm_cfg.decode_method       = 0x0;
  //srm_cfg.srmPsidDerUnsecure  = 0x0;
  //srm_cfg.srmPsidPeraUnsecure = 0x1;
  srm_cfg.asn1_decode_method  = 0x0;
  
  UnitshmPtr->cfgData.h.ch_data_valid = WTRUE;

  /* 
   * Call functions under test 
   */
#if 0
  Wsm.psid = srm_cfg.srmPsidPeruUnsecure;
  Wsm.dataLength = MAX_WSM_DATA;
  //memcpy(Wsm.data,srm_eta_zero,MAX_WSM_DATA);
  memcpy(Wsm.data,srm_harbor,MAX_WSM_DATA);

  //since the ASN1 decoders are not present we cant properly decode this here.
  srm_error_states = 0x0;
  receive_srm(&Wsm);
  CU_ASSERT(SRM_BAD_INPUT & srm_error_states);
#endif

  srm_error_states = 0x0;
  Wsm.dataLength = MAX_WSM_DATA +1;
  receive_srm(&Wsm);
  CU_ASSERT(SRM_WSM_LENGTH_ERROR & srm_error_states);

  srm_error_states = 0x0;
  Wsm.dataLength = MAX_WSM_DATA;
  receive_srm(&Wsm);
  CU_ASSERT(SRM_BAD_INPUT & srm_error_states);

  /* Check for pending tests that did not complete.
   * Set tickle mask and ack mask for test.  
   */
  CU_ASSERT(0x0 == check_stub_pending_signals());
  clear_all_stub_signal();

  srm_error_states = 0x0;
  set_stub_signal(Signal_asn1PD_SignalRequestMessage_1);
  receive_srm(&Wsm);
  CU_ASSERT(SRM_ASN1_DECODE_PER_MSG_ERROR & srm_error_states);

  /* Clear tickle mask for next test. 
   * Check ack mask is complete
   */
  CU_ASSERT(0x0 == check_stub_pending_signals());
  clear_all_stub_signal();

}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

void test_main(void)
{
  int32_t ret = SRM_AOK; 


  /* Check for pending tests that did not complete.
   * Set tickle mask and ack mask for test.  
   */
  CU_ASSERT(0x0 == check_stub_pending_signals());
  clear_all_stub_signal();

  /* 
   * Setup I2V SHM flavor for this test. IWMH will open same path etc. 
   * Make sure module statics are reset per iteration.
   */ 
  if ((UnitshmPtr = wsu_share_init(sizeof(i2vShmMasterT), I2V_SHM_PATH)) == NULL) {
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME," Main shared memory init failed\n");
      return;
  }
  CU_ASSERT(I2V_RETURN_OK == i2v_setupSHM_Default(UnitshmPtr)); /* Should reflect i2v.conf. */
  sleep(1); /* Give SHM a chance to digest. */
#if 0 //TODO: Need to update bad srm.conf
  /* Test against bad conf. */
  strcpy(UnitshmPtr->cfgData.config_directory, "../stubs/configs/");
  strcpy(confFileName, "srm_rx_bad.conf");
  UnitshmPtr->cfgData.globalDebugFlag = 1; /* If you want to see debug */

  ret = srm_main();
  CU_ASSERT(I2V_RETURN_OK == ret);
  CU_ASSERT(srm_cfg.radioNum == I2V_RADIO_DEFAULT);
  CU_ASSERT(srm_cfg.secDebug == SRM_SECURITY_DEBUG_ENABLE_DEFAULT);
  CU_ASSERT(srm_cfg.decode_method == SRMRX_DECODE_METHOD_DEFAULT);
  CU_ASSERT(srm_cfg.asn1_decode_method == SRMRX_ASN1_DECODE_METHOD_DEFAULT);
  CU_ASSERT(srm_cfg.channel_number == I2V_RADIO_CHANNEL_DEFAULT);
  CU_ASSERT(srm_cfg.srmForward == SRM_RX_FWD_ENABLE_DEFAULT);
 //CU_ASSERT(srm_cfg.srmPsidDerUnsecure == SRMRX_PSID_DEFAULT);
 // CU_ASSERT(srm_cfg.srmPsidPeraUnsecure == SRMRX_PSID_DEFAULT);
 // CU_ASSERT(srm_cfg.srmPsidPeruUnsecure == SRMRX_PSID_DEFAULT);
  CU_ASSERT(srm_cfg.srmTxVehBasicRole == SRMTX_VEHICLE_ROLE_DEFAULT);
  CU_ASSERT(srm_cfg.srmPermissive == SRM_PERMISSIVE_DEFAULT);
#endif

  //Validate conf_table.h matches srm_rx.conf
  strcpy(UnitshmPtr->cfgData.config_directory, "./");
  strcpy(confFileName, "srm_rx.conf");
  UnitshmPtr->cfgData.globalDebugFlag = 1;
  ret = srm_main();
  CU_ASSERT(ret == I2V_RETURN_OK);
  CU_ASSERT(srm_cfg.radioNum == I2V_RADIO_DEFAULT);
  CU_ASSERT(srm_cfg.secDebug == SRM_SECURITY_DEBUG_ENABLE_DEFAULT);
  CU_ASSERT(srm_cfg.decode_method == SRMRX_DECODE_METHOD_DEFAULT);
  CU_ASSERT(srm_cfg.asn1_decode_method == SRMRX_ASN1_DECODE_METHOD_DEFAULT);
  CU_ASSERT(srm_cfg.channel_number == I2V_RADIO_CHANNEL_DEFAULT);
  CU_ASSERT(srm_cfg.srmForward == SRM_RX_FWD_ENABLE_DEFAULT);
  //CU_ASSERT(srm_cfg.srmPsidDerUnsecure == SRMRX_PSID_DEFAULT);
  //CU_ASSERT(srm_cfg.srmPsidPeraUnsecure == SRMRX_PSID_DEFAULT);
  //CU_ASSERT(srm_cfg.srmPsidPeruUnsecure == SRMRX_PSID_DEFAULT);
  CU_ASSERT(srm_cfg.srmTxVehBasicRole == SRMTX_VEHICLE_ROLE_DEFAULT);
  CU_ASSERT(srm_cfg.srmPermissive == SRM_PERMISSIVE_DEFAULT);

  set_stub_signal(Signal_wsu_share_init);
  ret = srm_main();
  CU_ASSERT(abs(SRM_SHM_FAIL) == ret);
  if(abs(SRM_SHM_FAIL) != ret) {
      printf("\nret=%d\n",ret);
  }
  CU_ASSERT(0x0 == check_stub_pending_signals());
  clear_all_stub_signal();

  set_stub_signal(Signal_asn1Init_SignalRequestMessage);
  ret = srm_main();
  CU_ASSERT(abs(I2V_ASN1_SRM_INIT_FAIL) == ret);
  CU_ASSERT(0x0 == check_stub_pending_signals());
  clear_all_stub_signal();

  set_stub_signal(Signal_rtInitContextUsingKey);
  ret = srm_main();
  CU_ASSERT(abs(I2V_ASN1_CONTEXT_INIT_FAIL) == ret);
  CU_ASSERT(0x0 == check_stub_pending_signals());
  clear_all_stub_signal();

  set_stub_signal(Signal_i2vUtilParseConfFile);
  ret = srm_main();
  CU_ASSERT(abs(I2V_RETURN_INVALID_CONFIG) == ret);
  CU_ASSERT(0x0 == check_stub_pending_signals());
  clear_all_stub_signal();

  /* Clear tickle mask for next test.
   * Check ack mask is complete
   */
  CU_ASSERT(0x0 == check_stub_pending_signals());
  clear_all_stub_signal();

}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void test_cleanup(void)
{
  cfgItemsT cfg;

  /* Check for pending tests that did not complete.
   * Set tickle mask and ack mask for test.  
   */
  CU_ASSERT(0x0 == check_stub_pending_signals());
  clear_all_stub_signal();

  //cfg.iwmhAppEnable = WFALSE;
  //set_stub_signal(Signal_wsuRisUserServiceRequest);  
  exitHandler();
  cleanup(&cfg);

  //set_stub_signal(Signal_wsuRisWsmServiceRequest);
  //cleanup(&cfg);

  //set_stub_signal(Signal_wsuRisTerminateRadioComm);
  //cleanup(&cfg);


  /* Clear tickle mask for next test. Check ack mask is complete*/
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
  /* main() must go first to setup SHM and CONF items for the other functions. */
  if(CUE_SUCCESS == ret) {
      if ((NULL == CU_add_test(pSuite1, "\ntest_main...",test_main))) {
          ret = ~CUE_SUCCESS;
      }
  }
#if 1//Without SHM available we can't call function directly like this.
  if(CUE_SUCCESS == ret) {
      if ((NULL  == CU_add_test( pSuite1,"\ntest_srm_update_cfg...",test_srm_update_cfg))) {
          ret = ~CUE_SUCCESS;
      }
  }

  if(CUE_SUCCESS == ret) {
      if ((NULL == CU_add_test(pSuite1, "\ntest_asn1DecodeUPER_MessageFrame...",test_asn1DecodeUPER_MessageFrame))) {
          ret = ~CUE_SUCCESS;
      }
  }

  if(CUE_SUCCESS == ret) {
      if ((NULL == CU_add_test(pSuite1, "\ntest_asn1DecodePER_SRM...",test_asn1DecodePER_SRM))) {
          ret = ~CUE_SUCCESS;
      }
  }

  if(CUE_SUCCESS == ret) {
      if ((NULL == CU_add_test(pSuite1, "\ntest_DecodeSRM...",test_DecodeSRM))) {
          ret = ~CUE_SUCCESS;
      }
  }

  if(CUE_SUCCESS == ret) {
      if ((NULL == CU_add_test(pSuite1, "\ntest_receive_srm...",test_receive_srm))) {
          ret = ~CUE_SUCCESS;
      }
  }
  if(CUE_SUCCESS == ret) {
      if ((NULL == CU_add_test(pSuite1, "\ntest_cleanup...",test_cleanup))) {
          ret = ~CUE_SUCCESS;
      }
  }

#endif
  if(CUE_SUCCESS == ret) {
     printf("\nCUnit CU_basic_run_tests...\n");
     ret = CU_basic_run_tests();
  }

  i2v_setupSHM_Clean(UnitshmPtr); /* Close SHM. */

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
