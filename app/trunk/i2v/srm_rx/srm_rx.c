/******************************************************************************
 *                                                                            *
 *     File Name:  srm_rx.c                                                   *
 *     Author:                                                                *
 *         DENSO International America, Inc.                                  *
 *         North America Research Laboratory, California Office               *
 *         Carlsbad, CA 92008                                                 *
 *                                                                            *
 ******************************************************************************
 * (C) Copyright 2021 DENSO International America, Inc.  All rights reserved. * 
 ******************************************************************************/
#include <signal.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "wsu_util.h"
#include "wsu_shm.h"
#include "i2v_general.h"
#include "i2v_util.h"
#include "i2v_shm_master.h"
#include "vod_api.h"
#include "alsmi_struct.h"
#if defined(J2735_2016)
#include "DSRC.h"
#elif defined(J2735_2023)
#include "Common.h"
#include "MessageFrame.h"
#include "SignalRequestMessage.h"
#else
#ERROR: You must define  J2735_2016 or J2735_2023 in v2xcommon.mk
#endif
#include "srm_rx.h"
#include "ntcip-1218.h"
#include "conf_table.h"
#if defined(MY_UNIT_TEST)
#include "stubs.h"
#define MAIN srm_main
extern void * SRM_WSU_SHARE_INIT(size_t size, char_t *spath);
#else
#define MAIN main
#define SRM_WSU_SHARE_INIT wsu_share_init
#endif

#if defined(EXTRA_DEBUG)
#define OUTPUT_MODULUS  60  /* 30 Seconds. */
#else
#define OUTPUT_MODULUS  2499 /* 1200 Seconds. */
#endif

#if defined(MY_UNIT_TEST)
#undef OUTPUT_MODULUS 
#define OUTPUT_MODULUS      1  /* Seconds. */
#define TEST_ITERATION_MAX  5  /* # times through mainloop before normal exit mainloop==WFALSE */
#endif

#define SRM_RX_CFG_FILE "srm_rx.conf"
#define WSM_PSID         0xe0000016 /* from srm_rx.conf */
#define CHANNEL_NUMBER   183  /* default setting for cv2x */
#define RADIO_NUM         0   /* default setting - should be oposite of BSM */

/* mandatory logging defines */
#define MY_ERR_LEVEL   LEVEL_PRIV    /* from i2v_util.h */
#define MY_NAME        "srmrx"
#define APP_NAME        "srmrx_app"
#define MAX_SRM_SIZE   MAX_WSM_DATA /* What is max size for dynamic message over DSRC */

#define SRM_MAIN_DEFAULT_SLEEP_TIME (500 * 1000) /* 500 msecs */
#define SRM_SECURITY_STATS_LOG_INTERVAL 600     /* Done inside main loop. Once every 5 minutes. */

/* Global */
STATIC i2vShmMasterT * shm_ptr = NULL;
STATIC srmCfgItemsT srm_cfg;                      /* Callback uses this and we can't pass as parameter */
cfgItemsT  cfg;
STATIC char_t confFileName[I2V_CFG_MAX_STR_LEN];
STATIC SignalRequestMessage srm_rx_msg;
STATIC OSCTXT srm_ctxt;
STATIC MessageFrame msgFrameRx;

STATIC SRMMsgType SRM_GS2Msg; 

UserServiceType uService = {
    .radioNum        = RADIO_NUM,
    .action          = ADD,
    .userAccess      = AUTO_ACCESS_UNCONDITIONAL,
    .psid            = WSM_PSID,
    .servicePriority = 10,
    .channelNumber   = CHANNEL_NUMBER,
    .immediateAccess = 1,
    .extendedAccess  = 65535,     /* stay on channel forever */
    .radioType = RT_DSRC
};

WsmServiceType wService = {
     .action    = ADD,
     .psid      = WSM_PSID,
};  

typedef struct OutWsmVer {
    uint32_t WsmSequenceNumber;
    float64_t  latitude;      
    float64_t  longitude;    
} OutWsmMsgData;

STATIC bool_t loop  = WTRUE;
STATIC uint64_t srm_error_states = 0x0;
STATIC uint64_t prior_srm_error_states = 0x0;
#ifdef DEBUG_SRM
STATIC uint32_t my_count = 0;
#endif

/* Stats for srm rx over radio. */
STATIC uint32_t srm_rx_counts = 0;
STATIC uint32_t srm_rx_err_counts = 0;
/* Stats for srm forwarded to scs. scs will FWDMSG it. */
STATIC uint32_t rx_msg_write_count = 0;
STATIC uint32_t rx_msg_write_error_count = 0;

uint64_t time_utc_msecs = 0; /* mainloop will update ~once a second. */

/* 1218 MIB: 5.12 rsuMessageStats. */
STATIC messageStats_t messageStats[RSU_PSID_TRACKED_STATS_MAX];
STATIC messageStats_t * shm_messageStats_ptr = NULL; /* SHM. */
STATIC pthread_t MessageStatsThreadID;

STATIC int32_t IntID = 0; /* From spat16.conf via SHM */

STATIC uint16_t verifyVODCount = 0;
STATIC bool_t   initVOD = FALSE;
STATIC SRMSecStats  securityStats;
STATIC SRMSecStats  secLogStats;
STATIC uint32_t srmSecStatsIntCnt = 0;

STATIC void set_my_error_state(int32_t srm_error)
{
  int32_t dummy;

  dummy = abs(srm_error);
  /* -1 to -64 */
  if((dummy > SRM_ERROR_BASE) && (dummy < SRM_ERROR_TOP)) {
      srm_error_states |= (uint64_t)(0x1) << (dummy - 1);
  } 
}
void clear_my_error_state(int32_t srm_error)
{
  int32_t dummy = 0;

  dummy = abs(srm_error);
  if((dummy < SRM_ERROR_BASE) && (SRM_ERROR_TOP < dummy)) {
      srm_error_states &= ~((uint64_t)(0x1) << (dummy - 1));
  }
}
int32_t is_my_error_set(int32_t srm_error)
{
  int32_t dummy = 0;
  int32_t ret = 0; /* FALSE till proven TRUE */

  dummy = abs(srm_error);
  if((dummy < SRM_ERROR_BASE) && (SRM_ERROR_TOP < dummy)) {
      if (srm_error_states & ((uint64_t)(0x1) << (dummy - 1))) {
          ret = 1;
      }
  }
  return ret;
}
/*----------------------------------------------------------------------------*/
/* Dump to /tmp for user to cat. Only actionable items for user.              */
/*----------------------------------------------------------------------------*/
STATIC void dump_srm_report(void)
{
  FILE * file_out = NULL;
  if ((file_out = fopen("/tmp/srm.txt", "w")) == NULL){
      if(0x0 == is_my_error_set(SRM_CUSTOMER_DIGEST_FAIL)) { /* Report fail one time. */
          set_my_error_state(SRM_CUSTOMER_DIGEST_FAIL);
      }
  } else {
      clear_my_error_state(SRM_CUSTOMER_DIGEST_FAIL);
      if(srm_rx_err_counts){
          fprintf(file_out,"RSU failed to decode %u SRMs. Check SEL for specific failure.\n",srm_rx_err_counts);
      }
      if(is_my_error_set(SRM_VEHICLE_ROLE_REJECT)){
          fprintf(file_out,"SRM_VEHICLE_ROLE_REJECT: Some SRMs rejected because invalid Vehilce Basic Role. If concerned, check Web Gui->Adbanced Config Settings->SRMTXVehBasicRole.\n");
      }
      if(is_my_error_set(SRM_WSM_WRONG_PSID)){
          fprintf(file_out,"SRM_WSM_WRONG_PSID: Some SRMs rejected because invalid PSID. SRM PSID should be 0x%x.\n",cfg.srmPsid);
      }
      if(is_my_error_set(SRM_FWD_TO_SCS_DISABLED)){
          fprintf(file_out,"SRM_FWD_TO_SCS_DISABLED: Friendly warning, RSU is not forwarding SRMs. If concerned, check Web Gui->Configuration Settings->Foward Message to enable.\n");
      }
      if(is_my_error_set(SRM_WRONG_INT_ID)){
          fprintf(file_out,"SRM_WRONG_INT_ID: Friendly warning, RSU is rejecting SRMs that don't match Intersection ID. Set to 1 in Web Gui->Advanced Config Settings->SRM RX->SRMPermissive to allow all SRMs.\n");
          fprintf(file_out,"SRM_WRONG_INT_ID: Friendly warning, RSU is rejecting SRMs that don't match Intersection ID. Check and verify Web Gui->Intersection Settings->Intersection ID is correct.\n");
      }
      if(is_my_error_set(SRM_VERIFICATION_FAIL)){
          fprintf(file_out,"SRM_VERIFICATION_FAIL: Friendly warning, RSU rejected some SRMs due to failed security verification. Set to 0 in Web Gui->Advanced Config Settings->SRM RX->SRMVODMsgVerifyCount to allow all SRMs.\n");
      }

      fflush(file_out);
      fclose(file_out);
      file_out = NULL;
  }
}
/* 
 * exitHandler()
 * This function gets called when a signal is caught by the application, such as Ctrl-C 
 */
void exitHandler(void)
{
    loop = WFALSE;
}
/* 
 * cleanup
 * cleans up all registered WSU services 
 */
void cleanup(void)
{
    /* 
     * Error or not be sure to free/reset ASN1 resources to avoid memory leak. 
     */
#if defined(J2735_2016)
    asn1Free_MessageFrame(&srm_ctxt,&msgFrameRx);
    asn1Free_SignalRequestMessage(&srm_ctxt,&srm_rx_msg);
#endif
    //rtxErrPrint(&srm_ctxt);       
    rtxMemReset(&srm_ctxt);
    rtxMemFree(&srm_ctxt); 
    rtxErrReset(&srm_ctxt);
    rtFreeContext(&srm_ctxt);

    return;
}
STATIC uint32_t srmrxSetConfDefault(char_t * tag, void * itemToUpdate)
{
  uint32_t ret = I2V_RETURN_OK; /* Success till proven fail. */

  if((NULL == tag) || (NULL == itemToUpdate)) {
      /* Nothing to do. */
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"srmrxSetConfDefault: NULL input.\n");
      set_my_error_state(SRM_BAD_INPUT);
      ret = I2V_RETURN_NULL_PTR;
  } else {
      if(0 == strcmp("RadioCfg",tag)) {
          *(uint8_t *)itemToUpdate = I2V_RADIO_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%u)\n",tag,*(uint8_t *)itemToUpdate);
      } else if(0 == strcmp("SecurityDebugEnable",tag)) {
          *(bool_t *)itemToUpdate = SRM_SECURITY_DEBUG_ENABLE_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%u)\n",tag,*(bool_t *)itemToUpdate);
      } else if(0 == strcmp("SRMDecodeMethod",tag)) {
          *(uint32_t *)itemToUpdate = SRMRX_DECODE_METHOD_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%u)\n",tag,*(uint32_t *)itemToUpdate);
      } else if(0 == strcmp("SRMASN1DecodeMethod",tag)) {
          *(uint32_t *)itemToUpdate = SRMRX_ASN1_DECODE_METHOD_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%u)\n",tag,*(uint32_t *)itemToUpdate);
      } else if(0 == strcmp("SRMChannelNumber",tag)) {
          *(uint8_t *)itemToUpdate = I2V_RADIO_CHANNEL_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%u)\n",tag,*(uint8_t *)itemToUpdate);
      } else if(0 == strcmp("SRMRxForward",tag)) {
          *(bool_t *)itemToUpdate = SRM_RX_FWD_ENABLE_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%u)\n",tag,*(bool_t *)itemToUpdate);
      } else if(0 == strcmp("SRMTXVehBasicRole",tag)) {
          *(uint32_t *)itemToUpdate = SRMTX_VEHICLE_ROLE_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%u)\n",tag,*(uint32_t *)itemToUpdate);
      } else if(0 == strcmp("SRMPermissive",tag)) {
          *(uint8_t *)itemToUpdate = SRM_PERMISSIVE_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%u)\n",tag,*(uint8_t *)itemToUpdate);
      } else {
          /* Nothing to do. */
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"srmrxSetConfDefault:(%s) item is not known, ignoring.\n",tag);
          set_my_error_state(SRM_HEAL_FAIL);
          ret = I2V_RETURN_UNKNOWN;
      }
  }
  return ret;
}
/*
 * ==================================================================
 */
STATIC i2vReturnTypesT srm_update_cfg(char_t * filename, i2vShmMasterT * local_shm_ptr, 
                                      srmCfgItemsT * local_srm_cfg, cfgItemsT * local_cfg)
{
    cfgItemsTypeT cfgItems[] = {
        {"RadioCfg",            (void *)i2vUtilUpdateUint8Value, &srm_cfg.radioNum,            NULL,(ITEM_VALID|UNINIT)},
        {"SecurityDebugEnable", (void *)i2vUtilUpdateWBOOLValue,  &srm_cfg.secDebug,            NULL,(ITEM_VALID|UNINIT)},
        {"SRMDecodeMethod",     (void *)i2vUtilUpdateUint32Value, &srm_cfg.decode_method,       NULL,(ITEM_VALID|UNINIT)},
        {"SRMASN1DecodeMethod", (void *)i2vUtilUpdateUint32Value, &srm_cfg.asn1_decode_method,  NULL,(ITEM_VALID|UNINIT)},
        {"SRMChannelNumber",    (void *)i2vUtilUpdateUint8Value,  &srm_cfg.channel_number,      NULL,(ITEM_VALID|UNINIT)},
        {"SRMRxForward",        (void *)i2vUtilUpdateWBOOLValue,  &srm_cfg.srmForward,          NULL,(ITEM_VALID|UNINIT)},
        {"SRMTXVehBasicRole",   (void *)i2vUtilUpdateUint32Value, &srm_cfg.srmTxVehBasicRole,   NULL,(ITEM_VALID|UNINIT)},
        {"SRMPermissive",       (void *)i2vUtilUpdateUint8Value,  &srm_cfg.srmPermissive,       NULL,(ITEM_VALID|UNINIT)},
        {"SRMVODMsgVerifyCount",   (void *)i2vUtilUpdateUint16Value, &srm_cfg.srmVodMsgVerifyCount,   NULL,(ITEM_VALID|UNINIT)},
    };
    FILE     * f = NULL;
    char_t     fileloc[I2V_CFG_MAX_STR_LEN + I2V_CFG_MAX_FILE_LEN];
    uint32_t   retVal = I2V_RETURN_OK;
    uint32_t   i = 0;
    /*
     * Check Params
     */
    if(   (NULL == filename) 
       || (NULL == local_shm_ptr) 
       || (NULL == local_srm_cfg) 
       || (NULL == local_cfg ) ) {
        retVal = I2V_RETURN_INVALID_PARAM;
        set_my_error_state(SRM_BAD_INPUT);
    } else {

        memset(fileloc,'\0',I2V_CFG_MAX_STR_LEN + I2V_CFG_MAX_FILE_LEN);
        /* 
         * Get master i2v config settings 
         */

        /* Wait on I2V SHM to get I2V CFG.*/
        for(i=0;loop && (i < MAX_I2V_SHM_WAIT_ITERATIONS); i++){
            WSU_SEM_LOCKR(&local_shm_ptr->cfgData.h.ch_lock);
            if(WTRUE == local_shm_ptr->cfgData.h.ch_data_valid) {
                memcpy(local_cfg, &local_shm_ptr->cfgData, sizeof(cfgItemsT));  /* save config for later use */
                IntID = local_shm_ptr->spatCfgData.IntersectionID;
                WSU_SEM_UNLOCKR(&local_shm_ptr->cfgData.h.ch_lock);
                break;
            }
            WSU_SEM_UNLOCKR(&local_shm_ptr->cfgData.h.ch_lock);
            usleep(MAX_i2V_SHM_WAIT_USEC);
        }

        if((WFALSE == loop) || (MAX_I2V_SHM_WAIT_ITERATIONS <= i)) {
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"srm_update_cfg: I2V_SHM_FAIL.\n");
            set_my_error_state(SRM_SHM_FAIL);
            return I2V_RETURN_FAIL;
        }

        /* Open config file */
        strncpy(fileloc,(const char_t *)local_cfg->config_directory, I2V_CFG_MAX_STR_LEN);
        strcat(fileloc, filename);
        if ((f = fopen(fileloc, "r")) == NULL) {
            retVal = I2V_RETURN_INV_FILE_OR_DIR; 
            set_my_error_state(SRM_CONF_FOPEN_ERROR);
        }

        if(I2V_RETURN_OK == retVal) {
            if(I2V_RETURN_OK != (retVal = i2vUtilParseConfFile(f, cfgItems, NUMITEMS(cfgItems), WFALSE, NULL))) {
                I2V_DBG_LOG(LEVEL_CRIT,MY_NAME,"i2vUtilParseConfFile: Failed retVal=%u.\n",retVal);    
                set_my_error_state(SRM_CONF_PARSE_ERROR);
            }
            if(NULL != f) fclose(f);

            for (i = 0; i < NUMITEMS(cfgItems); i++) {
                if (   ((ITEM_VALID|BAD_VAL) == cfgItems[i].state) 
                    || ((ITEM_VALID|UNINIT) == cfgItems[i].state)) {
                    I2V_DBG_LOG(LEVEL_CRIT,MY_NAME,"config override on (%s): Using default.\n", cfgItems[i].tag);
                    if(I2V_RETURN_OK == (retVal = srmrxSetConfDefault(cfgItems[i].tag, cfgItems[i].itemToUpdate))) {
                        cfgItems[i].state = (ITEM_VALID|INIT);
                        set_my_error_state(SRM_HEAL_CFGITEM); /* Note for user. */
                    } else {
                        break; /* Heal has failed. FATAL. */
                    }
                }
            }
        }

        if(I2V_RETURN_OK == retVal) {

            if (local_cfg->bcastLockStep) {  /* all rse apps broadcast on same radio and channel */
                local_srm_cfg->channel_number = local_cfg->uchannel;
                local_srm_cfg->radioNum = local_cfg->uradioNum;
            }

            /* copy config into shared memory */
            WSU_SEM_LOCKW(&local_shm_ptr->srmCfgData.h.ch_lock);
            memcpy(&local_srm_cfg->h, &local_shm_ptr->srmCfgData.h, sizeof(wsu_shm_hdr_t)); /* preserve header */
            memcpy(&local_shm_ptr->srmCfgData, local_srm_cfg, sizeof(srmCfgItemsT));
            local_shm_ptr->srmCfgData.h.ch_data_valid = WTRUE;
            WSU_SEM_UNLOCKW(&local_shm_ptr->srmCfgData.h.ch_lock);

            /* 
             * Radio and service settings.
             */
            uService.radioNum       = local_srm_cfg->radioNum;
            uService.channelNumber  = local_srm_cfg->channel_number;
            //wService.radioNum       = local_srm_cfg->radioNum;
            //wService.channelNumber  = local_srm_cfg->channel_number;
            uService.psid           = local_cfg->srmPsid;
            wService.psid           = local_cfg->srmPsid;
        }
    }
    return retVal;
}

/* Decode ASN.1 UPER */
STATIC int32_t asn1DecodeUPER_MessageFrame(uint8_t * data, int32_t datalen, MessageFrame * msgFrameRx,
                                           int8_t aligned, int32_t * decodeLen, OSCTXT * ctxt)
{
  int32_t ret = I2V_RETURN_OK; /* OK till proven otherwise */

  if(   (NULL == msgFrameRx) 
     || (NULL == data)
     || (NULL == decodeLen)
     || (NULL == ctxt)
     || !((0 <= aligned) && (aligned <= 1))
     || !((0 < datalen) && (datalen <= MAX_WSM_DATA))
    ) {

      #ifdef DEBUG_SRM
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"asn1DecodeUPER_MessageFrame::ERROR:Invalid Inputs.\n");
      #endif
      ret = I2V_ASN1_FRAME_INVALID_PARAM; 
      set_my_error_state(SRM_BAD_INPUT);
  } else {

      memset(msgFrameRx, 0x0, sizeof(MessageFrame));

      if(0 != asn1Init_MessageFrame(msgFrameRx)) {
          ret = I2V_ASN1_FRAME_INIT_FAIL;
          set_my_error_state(SRM_ASN1_FRAME_INIT_ERROR);
      }
      if((0 == ret) && (0 != pu_setBuffer(ctxt, data, datalen, aligned))) {
          ret = I2V_ASN1_SET_BUFFER_FAIL;
          set_my_error_state(SRM_ASN1_FRAME_BUFFER_ERROR);
      }
      if((0 == ret) && ( 0 != asn1PD_MessageFrame(ctxt, msgFrameRx))) {
          ret = I2V_ASN1_FRAME_DECODE_ERROR;
          set_my_error_state(SRM_ASN1_FRAME_DECODE_ERROR);
      }
      if(0 == ret) {
          *decodeLen = pu_getMsgLen(ctxt);

          if( (0 < *decodeLen) && (*decodeLen <= MAX_SRM_SIZE)) {  
              ret = I2V_RETURN_OK;
          } else {
              ret = I2V_ASN1_FRAME_ILLEGAL_SIZE;
              set_my_error_state(SRM_ASN1_ILLEGAL_SIZE);
          }
      }
      /* Important: On ERROR release & reset to avoid memory leaks. */
      if (I2V_RETURN_OK != ret) { 
          *decodeLen = 0;
      }
  }

  return ret;
}

STATIC int32_t asn1DecodePER_SRM(uint8_t * data, uint32_t length, SignalRequestMessage * srm, 
                                 int32_t * size, uint8_t aligned, OSCTXT * ctxt)
{

  int32_t ret = I2V_RETURN_OK; /* OK till proven otherwise */

  if(   (NULL == srm) 
     || (NULL == data)
     || (NULL == size)
     || (NULL == ctxt)
     || !((0 == aligned) || (aligned == 1))
     || !((0 < length) && (length <= MAX_WSM_DATA))
    ) {

      #ifdef DEBUG_SRM
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"asn1DecodePER_SRM::ERROR:Invalid Inputs.\n");
      #endif
      ret = I2V_ASN1_SRM_INVALID_PARAM;
      set_my_error_state(SRM_BAD_INPUT);  

  } else {

      if( 0 != asn1Init_SignalRequestMessage(srm)) {
          ret = I2V_ASN1_SRM_INIT_FAIL;
          set_my_error_state(SRM_ASN1_DECODE_PER_INIT_ERROR);
      }

      if(( 0 == ret) && (0 != pu_setBuffer(ctxt, data, length, aligned))) {
          ret = I2V_ASN1_SET_BUFFER_FAIL;
          set_my_error_state(SRM_ASN1_DECODE_PER_BUFFER_ERROR);
      }

      if((0 == ret) && (0 != asn1PD_SignalRequestMessage(ctxt, srm))) {
          ret = I2V_ASN1_SRM_DECODE_ERROR;
          set_my_error_state(SRM_ASN1_DECODE_PER_MSG_ERROR);
      }

      if(0 == ret) {
          *size = pu_getMsgLen(ctxt);    
          if((0 < *size) && (*size <= MAX_SRM_SIZE)) {  
              ret = I2V_RETURN_OK; /* SUCCESS */
          } else {
              ret = I2V_ASN1_SRM_ILLEGAL_SIZE;
              set_my_error_state(SRM_ASN1_ILLEGAL_SIZE);
          }
      }
      /* Important: On ERROR release & reset to avoid memory leaks. */
      if (I2V_RETURN_OK != ret) { 
          *size = 0;
      }
  }

  return ret;

}

/* 
 * Decode ASN1 SRM message. 
 * Ensure well formed before sending to scs.c::scsProcessSRM().
 * scs.c::scsProcessSRM() will handle the truthiness of the data.
 */
STATIC int32_t DecodeSRM(SignalRequestMessage *srm_rx_msg, SRMMsgType *SRMMsg)
{
    SignalRequestPackage *sig_req_pkg = NULL;
    OSRTDListNode        *node        = NULL;
    TemporaryID          *tmpID       = NULL;
    int32_t                ret         = I2V_RETURN_OK; /* OK till proven not OK */
#ifdef WSU_LITTLE_ENDIAN
    uint32_t tmpVal;
#endif
    float64_t tti = 0.0; /* seconds with 3(0.001) decimal of precision */
    float64_t current_time = 0.0;

    if ( (srm_rx_msg == NULL) || (SRMMsg == NULL) ){
        ret = I2V_RETURN_INVALID_PARAM;

        #if defined DEBUG_SRM && !defined(MY_UNIT_TEST)
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"DecodeSRM::I2V_RETURN_INVALID_PARAM.\n");
        #endif
        set_my_error_state(SRM_BAD_INPUT);
    } else {
        /* 
         * Extract request - support one request
         */

        #if defined DEBUG_SRM_EXTRA 
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"DecodeSRM::reqCnt=%d.\n", srm_rx_msg->requests.count);
        #endif

        if (srm_rx_msg->requests.count >= 1){
            if ((node = rtxDListFindByIndex(&srm_rx_msg->requests, 0)) != NULL){
                sig_req_pkg = node->data;

                #if defined DEBUG_SRM
                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"DecodeSRM::SRM/SCS/SPAT ID=(%d,%d,%d).\n",sig_req_pkg->request.id->id
                                                        ,shm_ptr->scsCfgData.spat_intersection_id
                                                        ,IntID);
                #endif

                /* msgVersion, mac_id, seqNum, timeStamp and msgType will be filled in
                 * at time of sending
                 */
                SRMMsg->seqNum       = srm_rx_msg->sequenceNumber;
                SRMMsg->msgTimeStamp = srm_rx_msg->timeStamp;       /* SRM message timestamp in elapsed minutes of the year */
                SRMMsg->msgSecond    = srm_rx_msg->second;          /* This gives SRM message timestamp resolution in milliSecond */
                SRMMsg->msgCount     = srm_rx_msg->sequenceNumber;
                SRMMsg->etaMinute    = sig_req_pkg->minute;        /* ETA minute */
                SRMMsg->etaSeconds   = sig_req_pkg->second;        /* ETA mSecond */
                SRMMsg->etaDuration  = 0;
#if defined(J2735_2016)
                SRMMsg->intID        = sig_req_pkg->request.id.id;
                /* For now single request only? */
                SRMMsg->reqID        = sig_req_pkg->request.requestID;
                SRMMsg->priReqType   = sig_req_pkg->request.requestType;
                SRMMsg->vehRole      = srm_rx_msg->requestor.type.role;
                SRMMsg->vehLatitude  = srm_rx_msg->requestor.position.position.lat;
                SRMMsg->vehLongitude = srm_rx_msg->requestor.position.position.long_;
                SRMMsg->vehElevation = srm_rx_msg->requestor.position.position.elevation;
#else //(J2735_2023)
                SRMMsg->intID = 0;
                if(NULL != sig_req_pkg->request.id) {
                    SRMMsg->intID = sig_req_pkg->request.id->id;
                }

                SRMMsg->reqID = sig_req_pkg->request.requestID;
                SRMMsg->priReqType = sig_req_pkg->request.requestType;

                SRMMsg->vehRole = 0;
                if(NULL != srm_rx_msg->requestor.type) {
                    SRMMsg->vehRole = srm_rx_msg->requestor.type->role;
                }

                SRMMsg->vehLatitude  = 0;
                SRMMsg->vehLongitude = 0;
                SRMMsg->vehElevation = 0;
                if(NULL != srm_rx_msg->requestor.position.position) {
                    SRMMsg->vehLatitude  = srm_rx_msg->requestor.position.position->lat;
                    SRMMsg->vehLongitude = srm_rx_msg->requestor.position.position->long_;
                    SRMMsg->vehElevation = srm_rx_msg->requestor.position.position->elevation;
                }
#endif

                /* Heading angle or slice: 0..28799(359.9875): 28800 = unavailable */
                if(srm_rx_msg->requestor.position.m.headingPresent) {
                    //degrees...if you want
                    //SRMMsg->vehHeading   = (float64_t)(srm_rx_msg->requestor.position.heading) * 0.0125;
                    SRMMsg->vehHeading   = (float64_t)(srm_rx_msg->requestor.position.heading);
                }

                SRMMsg->vehSpeed         = 0;
                SRMMsg->vehTransmisson   = 0;
#if defined(J2735_2016)
                if(srm_rx_msg->requestor.position.m.speedPresent) {
                    SRMMsg->vehSpeed = srm_rx_msg->requestor.position.speed.speed/100.0;
                    SRMMsg->vehTransmisson = srm_rx_msg->requestor.position.speed.transmisson;
                }
#else //(J2735_2023)       
                if(srm_rx_msg->requestor.position.m.speedPresent) {
                    if(NULL != srm_rx_msg->requestor.position.speed) {
                        SRMMsg->vehSpeed = srm_rx_msg->requestor.position.speed->speed/100.0;
                        SRMMsg->vehTransmisson  = srm_rx_msg->requestor.position.speed->transmisson;
                    }
                }
#endif
                if(sig_req_pkg->m.durationPresent) {
                    /* Duration == tti (no conf value on OBU).
                     * No scaling applied(0.3333). 
                     */ 
                    SRMMsg->etaDuration  = sig_req_pkg->duration;
                }

                current_time = (((float64_t)(SRMMsg->msgSecond))/1000.0);
                current_time += (60.0 * (float64_t)(SRMMsg->msgTimeStamp));  

                tti = (((float64_t)(SRMMsg->etaSeconds))/1000.0);
                tti += (60.0 * (float64_t)(SRMMsg->etaMinute));

                #if defined DEBUG_SRM
                if(tti < current_time) { /* ETA NEGATIVE */
                    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"DecodeSRM::ERROR: ETA NEGATIVE: tti(%lf) < current_time(%lf).\n",tti ,current_time);               
                } else {
                    if( tti == current_time ) { /* ETA = ZERO */
                        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"DecodeSRM:: ERROR:ETA = ZERO: tti(%lf) == current_time(%lf).\n",current_time,tti);
                    } else {
                        tti = tti - current_time;
                        if(255.0 < tti) { /* ETA TOO LARGE */
                            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"DecodeSRM::ERROR: ETA TOO LARGE: 255.0 < tti(%lf).\n",tti);
                        }                         
                    }
                }
                #endif
                /* Depending on application you may want lane 
                 * or approach or ? 
                 */
                SRMMsg->inBoundLaneID = 0;
#if defined(J2735_2016)
                switch(sig_req_pkg->request.inBoundLane.t) {
                    case T_IntersectionAccessPoint_lane:
                        #if defined DEBUG_SRM
                        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"DecodeSRM::Lane request=%d.\n",sig_req_pkg->request.inBoundLane.u.lane);
                        #endif
                        SRMMsg->inBoundLaneID = sig_req_pkg->request.inBoundLane.u.lane;
                        break;
                    case T_IntersectionAccessPoint_approach:
                        #if defined DEBUG_SRM
                        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"DecodeSRM::Approach request=%d.\n",sig_req_pkg->request.inBoundLane.u.lane);
                        #endif
                        SRMMsg->inBoundLaneID = sig_req_pkg->request.inBoundLane.u.lane;
                        break;
                    case T_IntersectionAccessPoint_connection:
                        #if defined DEBUG_SRM
                        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"DecodeSRM::Connection request=%d.\n",sig_req_pkg->request.inBoundLane.u.lane);
                        #endif
                        ret = I2V_RETURN_INVALID_PARAM;
                        break;  
                    case T_IntersectionAccessPoint_extElem1:
                        #if defined DEBUG_SRM
                        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"DecodeSRM::extElem1 request=%d.\n",sig_req_pkg->request.inBoundLane.u.lane);
                        #endif
                        ret = I2V_RETURN_INVALID_PARAM;
                        break;
                    default:
                        #if defined DEBUG_SRM
                        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"DecodeSRM::unknown request=%d.\n",sig_req_pkg->request.inBoundLane.u.lane);
                        #endif
                        ret = I2V_RETURN_INVALID_PARAM;
                        break;
                }
#else //(J2735_2023)
                if(NULL != sig_req_pkg->request.inBoundLane) {
                switch(sig_req_pkg->request.inBoundLane->t) {
                    case T_IntersectionAccessPoint_lane:
                        #if defined DEBUG_SRM
                        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"DecodeSRM::Lane request=%d.\n",sig_req_pkg->request.inBoundLane->u.lane);
                        #endif
                        SRMMsg->inBoundLaneID = sig_req_pkg->request.inBoundLane->u.lane;
                        break;
                    case T_IntersectionAccessPoint_approach:
                        #if defined DEBUG_SRM
                        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"DecodeSRM::Approach request=%d.\n",sig_req_pkg->request.inBoundLane->u.lane);
                        #endif
                        SRMMsg->inBoundLaneID = sig_req_pkg->request.inBoundLane->u.lane;
                        break;
                    case T_IntersectionAccessPoint_connection:
                        #if defined DEBUG_SRM
                        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"DecodeSRM::Connection request=%d.\n",sig_req_pkg->request.inBoundLane->u.lane);
                        #endif
                        ret = I2V_RETURN_INVALID_PARAM;
                        break;  
                    case T_IntersectionAccessPoint_extElem1:
                        #if defined DEBUG_SRM
                        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"DecodeSRM::extElem1 request=%d.\n",sig_req_pkg->request.inBoundLane->u.lane);
                        #endif
                        ret = I2V_RETURN_INVALID_PARAM;
                        break;
                    default:
                        #if defined DEBUG_SRM
                        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"DecodeSRM::unknown request=%d.\n",sig_req_pkg->request.inBoundLane->u.lane);
                        #endif
                        ret = I2V_RETURN_INVALID_PARAM;
                        break;
                }
                }
#endif
                if(I2V_RETURN_OK == ret) {
                    SRMMsg->vehID = 0;
#if defined(J2735_2016)   
                    switch(srm_rx_msg->requestor.id.t) {
                        case T_VehicleID_entityID:
                            tmpID = srm_rx_msg->requestor.id.u.entityID;
                            #ifdef WSU_LITTLE_ENDIAN
                            memcpy(&tmpVal,&tmpID->data, 4);    
                            SRMMsg->vehID = htonl (tmpVal);
                            #else
                            memcpy(&SRMMsg->vehID,&tmpID->data, 4);    
                            #endif
                            break;
                        case T_VehicleID_stationID:
                            SRMMsg->vehID = srm_rx_msg->requestor.id.u.stationID;
                            break;
                        default:
                            ret = I2V_RETURN_INVALID_PARAM;
                            break;
                    }
#else //(J2735_2023)
                    if(NULL != srm_rx_msg->requestor.id){
                    switch(srm_rx_msg->requestor.id->t) {
                        case T_VehicleID_entityID:
                            tmpID = srm_rx_msg->requestor.id->u.entityID;
                            #ifdef WSU_LITTLE_ENDIAN
                            memcpy(&tmpVal,&tmpID->data, 4);    
                            SRMMsg->vehID = htonl (tmpVal);
                            #else
                            memcpy(&SRMMsg->vehID,&tmpID->data, 4);    
                            #endif
                            break;
                        case T_VehicleID_stationID:
                            SRMMsg->vehID = srm_rx_msg->requestor.id->u.stationID;
                            break;
                        default:
                            ret = I2V_RETURN_INVALID_PARAM;
                            break;
                    }
                    }
#endif
                } else {
                    set_my_error_state(SRM_GS2_LANE_REQUEST_ERROR);
                }
                #if defined DEBUG_SRM
                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"IntID(%d)Seq(0x%x)Inbnd(%d)VecID(0x%x)Role(%d)RqT(%d)\n",
                     SRMMsg->intID
                    ,SRMMsg->seqNum 
                    ,SRMMsg->inBoundLaneID 
                    ,SRMMsg->vehID
                    ,SRMMsg->vehRole
                    ,SRMMsg->priReqType);

                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Min:Sec:Dur(%d:%d:%d)tti(%.3lf)hdg(%.3lf)spd(%.3lf)tran(%d)\n",
                     SRMMsg->etaMinute
                    ,SRMMsg->etaSeconds
                    ,SRMMsg->etaDuration
                    ,tti
                    ,SRMMsg->vehHeading
                    ,SRMMsg->vehSpeed
                    ,SRMMsg->vehTransmisson);                 
                #endif

                /* 
                 * Permissive mode = 0 : Match SRM to this Intersection ID.
                 * Permissive mode = 1 : Don't care.
                 */
                if((I2V_RETURN_OK == ret) && (0 == srm_cfg.srmPermissive)) {
#if defined(J2735_2016) 
                    if (sig_req_pkg->request.id.id != IntID) {
                        #if defined DEBUG_SRM
                        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"DecodeSRM::SRM != SPAT(%d,%d): IGNORE.\n",sig_req_pkg->request.id.id,IntID);
                        #endif
                        set_my_error_state(SRM_WRONG_INT_ID);
                        ret = I2V_RETURN_NO_ACTION;
                    }
#else //(J2735_2023)
                    if(NULL != sig_req_pkg->request.id) {
                    if (sig_req_pkg->request.id->id != IntID) {
                        #if defined DEBUG_SRM
                        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"DecodeSRM::SRM != SPAT(%d,%d): IGNORE.\n",sig_req_pkg->request.id->id,IntID);
                        #endif
                        set_my_error_state(SRM_WRONG_INT_ID);
                        ret = I2V_RETURN_NO_ACTION;
                    }
                    }
#endif        
                } /* else AOK */
            } else {
                #if defined DEBUG_SRM 
                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"DecodeSRM::Node is NULL\n");
                #endif
                set_my_error_state(SRM_GS2_LANE_NODE_ERROR);
            }
        } else {
            #if defined DEBUG_SRM
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"DecodeSRM::Bad requests count=%d.\n",srm_rx_msg->requests.count);
            #endif
            set_my_error_state(SRM_GS2_REQUEST_COUNT_ERROR);
        }
    }

    return ret;
}

/* 
 * Verify-on-Demand for SRM Messages.
 * returns 0 for success and returns -1 for failure.
 */
static int verifySRMVOD(uint32_t vodSeqNum, SRMSecStats *securityStats)
{
    int8_t  vod_request_handle;
    int8_t  ret;
    int     result;

    /* Request has bee received. Increase the requests count */
    securityStats->srmVODRequests++;


    /* Increase the Successes and Failures in this function. */
    /* They are less frequently logged in the log file outside of this function. */
    if (vodSeqNum == 0)
    {
        /* Positive value. Update shared memory */
        securityStats->srmVODSuccesses++;
        return 0;
    }

    if (vodSeqNum == 1)
    {
        /* Positive value. Update shared memory */
        securityStats->srmVODFailures++;
        return -1;
    }

#if !defined(MY_UNIT_TEST)
    /* Submit for VOD */
    if ((vod_request_handle = request_msg_verification(vodSeqNum, PRIORITY_NORMAL)) < 0) {
        securityStats->srmVODFailures++;
        return -1;
    }

    /* Wait for VOD result */
    if ((ret = wait_for_vod_result(vod_request_handle)) == FALSE)   {
        securityStats->srmVODFailures++;
        return -1;
    }

    /* Retrieve VOD result */
    if (( result = retrieve_vod_result(vod_request_handle)) == FALSE)  {
        securityStats->srmVODFailures++;
        return -1;
    }
#endif
    /* Success */
    securityStats->srmVODSuccesses++;

    return 0;
}


//todo: unit test with shm init but how? its a call back
void receive_srm(inWSMType *Wsm)
{
    int32_t  ret       = I2V_RETURN_OK;
    int32_t  decodeLen = 0;
    int32_t  size      = 0;
    int32_t  i         = 0;

#if !defined(MY_UNIT_TEST)
    if(WFALSE == loop)
        return;
#endif

    srm_rx_counts++; /* Good or bad we got one. */

    if(NULL == Wsm) {
        ret = I2V_RETURN_INVALID_PARAM;
        set_my_error_state(SRM_BAD_INPUT);
    } else {
        if((0 == Wsm->dataLength) || (MAX_WSM_DATA < Wsm->dataLength)) {
            ret = I2V_WSM_ILLEGAL_SIZE;
            set_my_error_state(SRM_WSM_LENGTH_ERROR);
        } else {
            #if defined(DEBUG_SRM) && defined(EXTRA_EXTRA_DEBUG)
            my_count++;
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"UPER[0x%x]=[\n",my_count);
            for(i=0;(i<Wsm->dataLength) && (i < MAX_WSM_DATA);i++) {
                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"%02x \n",Wsm->data[i]);
            }
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"]\n");
            #endif
            if(cfg.srmPsid == Wsm->psid) {
                for(i=0;i<RSU_PSID_TRACKED_STATS_MAX;i++){
                    if((Wsm->psid == messageStats[i].psid) && (SNMP_ROW_ACTIVE == messageStats[i].rowStatus)) {
                        messageStats[i].count++;
                    }
                }
            } else {
                ret = SRM_WSM_WRONG_PSID;
            }
        }

        /* Handle Security Verification here */
        if ((cfg.security) && (ret == I2V_RETURN_OK))   {

            /* Copy Security Stats from Shared memory */
            WSU_SEM_LOCKR(&shm_ptr->scsSRMData.h.ch_lock);
            memcpy(&securityStats,&shm_ptr->scsSRMData.srmSecurityStats,sizeof(SRMSecStats));
            WSU_SEM_UNLOCKR(&shm_ptr->scsSRMData.h.ch_lock);

            /* Copy Security Status and Verify Code */
            securityStats.srmVTPVerifyStatus = (Wsm->securityFlags & 0xFF);
            securityStats.srmVTPVerifyCode = (Wsm->securityFlags & 0xFF00) >> 8;

            if (cfg.vodEnable)  {

                /* If VOD is enabled and it's time to verify then submit */
                if (++verifyVODCount >= srm_cfg.srmVodMsgVerifyCount) {

                    verifyVODCount = 0;

                    if (verifySRMVOD(Wsm->vodMsgSeqNum,&securityStats) < 0)    {
                        /* VOD failed */
                        WSU_SEM_LOCKW(&shm_ptr->scsSRMData.h.ch_lock);
                        memcpy(&shm_ptr->scsSRMData.srmSecurityStats,&securityStats,sizeof(SRMSecStats));
                        WSU_SEM_UNLOCKW(&shm_ptr->scsSRMData.h.ch_lock);

                        ret = SRM_VERIFICATION_FAIL;
                        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"receive_srm (VOD): ret=%d.\n",ret);

                        return;     // Return here if VOD verification fails
                    }
                }
            }   // cfg.vodEnable

            /* Copy the Security Stats back to Shared Memory */
            WSU_SEM_LOCKW(&shm_ptr->scsSRMData.h.ch_lock);
            memcpy(&shm_ptr->scsSRMData.srmSecurityStats,&securityStats,sizeof(SRMSecStats));
            WSU_SEM_UNLOCKW(&shm_ptr->scsSRMData.h.ch_lock);
        }   // cfg.security

        if(I2V_RETURN_OK == ret) {

            memset(&srm_rx_msg, 0, sizeof(srm_rx_msg));
            /* Decode back to srm msg format */
            ret = asn1DecodeUPER_MessageFrame(Wsm->data,Wsm->dataLength,&msgFrameRx,0,&decodeLen,&srm_ctxt);
        }
 
        if(I2V_RETURN_OK == ret) {
     
            #ifdef DEBUG_SRM_EXTRA
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"asn1DecodeUPER_MessageFrame: decoded %d bytes.\n", decodeLen);
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Received SRM:  Length=%d , psid=%x, RSS=%d dBm.\n", Wsm->dataLength, Wsm->psid, Wsm->rss);
            #endif

            ret = asn1DecodePER_SRM((uint8_t *)msgFrameRx.value.data
                                     , msgFrameRx.value.numocts
                                     , &srm_rx_msg
                                     , &size
                                     , 0 
                                     , &srm_ctxt);

            if (I2V_RETURN_OK != ret) {
                #if defined(EXTRA_DEBUG)
                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"receive_srm: Error decoding SRM(%d)\n",ret);
                #endif
                /* 
                 * Error or not be sure to free/reset ASN1 resources to avoid memory leak. 
                 */
                rtxErrPrint(&srm_ctxt);
                rtxMemReset(&srm_ctxt);
                rtxMemFree(&srm_ctxt); 
                rtxErrReset(&srm_ctxt);        
            }
            #ifdef DEBUG_SRM_EXTRA
            else I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"receive_srm: message decoded OK.\n");
            #endif
        } else {
            #ifdef DEBUG_SRM
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"receive_srm: Error decoding MSGFRAME\n");
            #endif
            /* 
             * Error or not be sure to free/reset ASN1 resources to avoid memory leak. 
             */
            rtxErrPrint(&srm_ctxt);       
            rtxMemReset(&srm_ctxt);
            rtxMemFree(&srm_ctxt); 
            rtxErrReset(&srm_ctxt);    
        }

        /* Now check for presence of mandatory optional items.
        */
        if(I2V_RETURN_OK == ret) {

            if(srm_rx_msg.m.sequenceNumberPresent) {

                #ifdef DEBUG_SRM_EXTRA
                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"receive_srm: sequenceNumberPresent.\n");
                #endif

            } else {
                #ifdef DEBUG_SRM
                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"receive_srm: ERROR No sequenceNumberPresent.\n");
                #endif
                set_my_error_state(SRM_SEQUENCE_ERROR);
                ret = I2V_RETURN_INVALID_PARAM;
            }

            if(srm_rx_msg.m.requestsPresent) {
                #ifdef DEBUG_SRM_EXTRA
                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"receive_srm: requestsPresent.\n");
                #endif
            } else {
                #ifdef DEBUG_SRM
                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"receive_srm: ERROR No requestsPresent.\n");
                #endif
                set_my_error_state(SRM_NO_REQUEST_ERROR);
                ret = I2V_RETURN_INVALID_PARAM;
            }

            if(srm_rx_msg.m.timeStampPresent) {
                #ifdef DEBUG_SRM_EXTRA
                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"receive_srm: timeStampPresent.\n");
                #endif
            } else {
                #ifdef DEBUG_SRM
                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"receive_srm: ERROR No timeStampPresent.\n");
                #endif
                set_my_error_state(SRM_NO_TIMESTAMP_ERROR);
                ret = I2V_RETURN_INVALID_PARAM;
            }
            /* Not expecting regional. */
            if(srm_rx_msg.m.regionalPresent) {
                #ifdef DEBUG_SRM
                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"receive_srm: regionalPresent.\n");
                #endif
                set_my_error_state(SRM_REGIONAL_PRSENT);
            } //else dont care 

        }
 
        if(I2V_RETURN_OK == ret) {
            /* Will only return OK if well formed and
               it matches spat_cfg.IntersectionId.
            */
            ret = DecodeSRM(&srm_rx_msg,&SRM_GS2Msg);  
            #ifdef DEBUG_SRM
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"receive_srm: ret=%d.\n",ret);
            #endif      
        }

        if(I2V_RETURN_OK != ret){
            srm_rx_err_counts++;
        }
        if(ret < 0) {
            set_my_error_state(ret);
        }
        /* Error or not free asn1 resources. */
#if defined(J2735_2016)
        asn1Free_MessageFrame(&srm_ctxt,&msgFrameRx);
        asn1Free_SignalRequestMessage(&srm_ctxt,&srm_rx_msg);
#endif
        rtxMemReset(&srm_ctxt);
        rtxMemFree(&srm_ctxt); 
        rtxErrReset(&srm_ctxt);

        if(I2V_RETURN_NO_ACTION == ret) {
            /* 
             * Do not forward SRM. Benign reject do not return error. 
             */
            ret = I2V_RETURN_OK;
        } else {

            /* Verify vehicle role is enabled for SRM. */
            if(I2V_RETURN_OK == ret) {

                #if defined DEBUG_SRM
                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"srmTxVehBasicRole(0x%x)SRM(0x%x).\n",srm_cfg.srmTxVehBasicRole,SRM_GS2Msg.vehRole);
                #endif
                /* Not sure it's useful but if conf set to 0x0 then always fails.*/
                if(0xFFFFFFFF != srm_cfg.srmTxVehBasicRole){
                    /* basicVehicle is first enum and == zero, special case. */
                    if ((basicVehicle == SRM_GS2Msg.vehRole) && (0x0 == (srm_cfg.srmTxVehBasicRole & 0x1))) {
                        ret = I2V_SRM_VEHICLE_ROLE_FAIL;
                    } else if(military < SRM_GS2Msg.vehRole) { /* Illegal range. */
                        ret = I2V_SRM_VEHICLE_ROLE_FAIL;
                    } else if (0x0 == ((0x1 << SRM_GS2Msg.vehRole) & srm_cfg.srmTxVehBasicRole)) {
                        ret = I2V_SRM_VEHICLE_ROLE_FAIL;
                    }
                    if(I2V_RETURN_OK != ret) {
                        #if defined DEBUG_SRM
                        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"srmTxVehBasicRole(%d) REJECTED.\n",SRM_GS2Msg.vehRole);
                        #endif
                        set_my_error_state(SRM_VEHICLE_ROLE_REJECT);
                        srm_rx_err_counts++;
                    }
                }
            } /* I2V_RETURN_OK == ret*/

            /* Forward to scs if enabled */
            if(I2V_RETURN_OK == ret) {
                if(srm_cfg.srmForward) { 
                    WSU_SEM_LOCKW(&shm_ptr->scsSRMData.h.ch_lock);
                    memcpy(&shm_ptr->scsSRMData.srm_msg, &SRM_GS2Msg, sizeof(SRMMsgType));
                    shm_ptr->scsSRMData.srm_len = sizeof(SRMMsgType);
                    memcpy(&shm_ptr->scsSRMData.uper_payload , Wsm->data, Wsm->dataLength);
                    shm_ptr->scsSRMData.uper_payload_len =  Wsm->dataLength;
                    WSU_SEM_UNLOCKW(&shm_ptr->scsSRMData.h.ch_lock);
                    /* open gate for scs can receive */
                    wsu_open_gate(&shm_ptr->scsSRMData.srmTriggerGate);
                    #ifdef DEBUG_SRM
                    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"receive_srm: Forward to SCS.\n");
                    #endif
                    rx_msg_write_count++;
                } else {
                    /* Getting SRM's but we have not enabled them to be sent to SCS. Not error, keep going. */
                    #ifdef DEBUG_SRM
                    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"receive_srm: srm not forwarded to SCS. srm_rx.conf.srmForward NOT enabled.\n");
                    #endif
                    set_my_error_state(SRM_FWD_TO_SCS_DISABLED);
                    rx_msg_write_error_count++;
                }
            }
        }
    }
}
/* Tailor for the PSID's the particular module needs. SRM direction is IN or BOTH. */
void *MessageStatsThread(void __attribute__((unused)) *arg)
{
  size_t shm_messageStats_size = sizeof(messageStats_t) * RSU_PSID_TRACKED_STATS_MAX;
  int32_t i;
  uint32_t rolling_counter = 0;

  /* Open SHM. */
  if (NULL == (shm_messageStats_ptr = wsu_share_init(shm_messageStats_size, MSG_STATS_SHM_PATH))) {
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"messageStatsThr: SHM init failed.\n");
      set_my_error_state(SRM_SHM_FAIL);
  } else {
      memset(&messageStats[0],0x0,shm_messageStats_size);
      while(loop) {
          sleep(1);
          if(0 == (rolling_counter % MSG_STATS_APP_SHM_UPDATE_RATE)) {
              for(i=0;i<RSU_PSID_TRACKED_STATS_MAX;i++){
                  /* If dirty then update from client. */
                  if(   (0x1 == shm_messageStats_ptr[i].dirty)
                     && (RSU_MESSAGE_COUNT_DIRECTION_OUT != shm_messageStats_ptr[i].direction)
                     && (RSU_CV2X_CHANNEL_DEFAULT == shm_messageStats_ptr[i].channel)
                     && (cfg.srmPsid == shm_messageStats_ptr[i].psid)) {
                      if (SNMP_ROW_ACTIVE == shm_messageStats_ptr[i].rowStatus) {
                          messageStats[i].psid = shm_messageStats_ptr[i].psid;
                          messageStats[i].channel = shm_messageStats_ptr[i].channel;
                          messageStats[i].direction = shm_messageStats_ptr[i].direction;
                          messageStats[i].start_utc_dsec = shm_messageStats_ptr[i].start_utc_dsec;
                      } else {
                          messageStats[i].psid = 0x0;
                          messageStats[i].channel = 0x0;
                          messageStats[i].direction = 0x0;
                          messageStats[i].start_utc_dsec = 0x0;
                      }
                      messageStats[i].count = 0; /* Do not clear SHM count. MIB does that. */
                      messageStats[i].rowStatus = shm_messageStats_ptr[i].rowStatus;
                      shm_messageStats_ptr[i].dirty = 0x0;
                  } else {
                      /* 
                       * If row is active and matches then update.
                       * Don't overwrite total, only add to it.
                       * Some PSID's shared like SPAT and MAP.
                       */
                      if(SNMP_ROW_ACTIVE == shm_messageStats_ptr[i].rowStatus) {
                          if (   (SNMP_ROW_ACTIVE == messageStats[i].rowStatus) 
                              && (messageStats[i].psid == shm_messageStats_ptr[i].psid)
                              && (0 < messageStats[i].count)){ 
                              shm_messageStats_ptr[i].count += messageStats[i].count;
                              messageStats[i].count = 0; /* Clear for next iteration. */
                          }
                      }
                  }
              } /* for */
          }
          #if defined(EXTRA_DEBUG)
          if(0 == (rolling_counter % OUTPUT_MODULUS)){
              for(i=0;i < 5;i++){
                  I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"MsgStatShm[%d]: dirty=%u, status=%d, count=%u, psid=0x%x, chan=%d, dir=%d, start=0x%lx.\n"
                      ,i,messageStats[i].dirty,messageStats[i].rowStatus,messageStats[i].count
                      ,messageStats[i].psid,messageStats[i].channel,messageStats[i].direction,messageStats[i].start_utc_dsec);
              }
          }
          #endif
          rolling_counter++;
          #if defined(MY_UNIT_TEST)
          if (TEST_ITERATION_MAX < rolling_counter) {
              loop = WFALSE;
              break;
          }
          #endif   
      } /* While */
  }
  set_my_error_state(SRM_MSG_STATS_THREAD_EXIT); /* Not Fatal. */
  /* Do not destroy as other threads and MIB can be still using. Recover on start up. */
  if(NULL != shm_messageStats_ptr) {
      wsu_share_kill(shm_messageStats_ptr, shm_messageStats_size);
  }
  pthread_exit(NULL);
}
//TODO: Can use argc & argv to pass in conf values to tweak during test.
int32_t MAIN(int32_t argc, char_t *argv[])
{
  int32_t     err_id = 0;
  int32_t     ret    = I2V_RETURN_OK; /* OK till proven not */
  uint32_t    optret = (uint32_t)RS_SUCCESS;
  uint32_t debug_counter=0; /* rolling counter. */
#if defined(EXTRA_DEBUG)
  int32_t i;
#endif

    loop = WTRUE;
    argc = argc;
    argv = argv;
    srm_error_states = 0x0;
    prior_srm_error_states = 0x0;
    memset(&SRM_GS2Msg,0x0,sizeof(SRM_GS2Msg));
    srm_rx_counts = 0;
    srm_rx_err_counts = 0;
    rx_msg_write_count = 0;
    rx_msg_write_error_count = 0;
    time_utc_msecs = 0;
    IntID = 0;

    /* Enable serial debug with I2V_DBG_LOG until i2v.conf::globalDebugFlag says otherwise. */
    i2vUtilEnableDebug(MY_NAME);
    /* LOGMGR assumed up by now. Could retry if fails. */
    if(0 != i2vUtilEnableSyslog(MY_ERR_LEVEL, MY_NAME)){
        I2V_DBG_LOG(LEVEL_WARN,MY_NAME,"Failed to open syslog. Only serial output available,\n");
    }

#if !defined(MY_UNIT_TEST) /* Select flavor in unit_test. */
    strncpy(confFileName,SRM_RX_CFG_FILE,sizeof(confFileName));
#endif

    /* Register signal handlers to catch Ctrl-C */
    signal( SIGINT,  (__sighandler_t) exitHandler);
    signal( SIGTERM, (__sighandler_t) exitHandler);
    signal( SIGKILL, (__sighandler_t) exitHandler);

    /* 
     * Init Shared Memory
     */
    if(NULL == (shm_ptr = SRM_WSU_SHARE_INIT(sizeof(i2vShmMasterT), I2V_SHM_PATH))) {
        ret = SRM_SHM_FAIL; /* FATAL.*/
    }
    /* Init SRM config */
    if ((I2V_RETURN_OK == ret) && (!(I2V_RETURN_OK == srm_update_cfg(confFileName,shm_ptr,&srm_cfg,&cfg))) ){
        /* should not error out here - setup default setting */
        ret = I2V_RETURN_INVALID_CONFIG;
        set_my_error_state(SRM_INVALID_CONF);
    }
    /* Init ASN1 libraries */
    if ((I2V_RETURN_OK == ret) && (0 != (err_id = asn1Init_SignalRequestMessage(&srm_rx_msg)))) {
        ret = I2V_ASN1_SRM_INIT_FAIL;
        set_my_error_state(SRM_ASN1_INIT_FAIL);
    }
    if ((I2V_RETURN_OK == ret) && (0 != rtInitContext(&srm_ctxt))) {
        ret = I2V_ASN1_CONTEXT_INIT_FAIL;
        set_my_error_state(SRM_ASN1_CONTEXT_FAIL);
    }
    /* Start NTCIP-1218 Services. */
    if (I2V_RETURN_OK == ret) {
        if (0 == (ret = pthread_create(&MessageStatsThreadID, NULL, MessageStatsThread, NULL))) {
            pthread_detach(MessageStatsThreadID);
        } else {
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"MessageStatsThreadID: Failed. Not Fatal: ret=%d, errno=%d.\n",ret, errno);
            set_my_error_state(SRM_THREAD_FAIL);
            ret = I2V_RETURN_OK;
        }
    }
#if !defined(MY_UNIT_TEST)
    /* Init with VOD when security is enabled */
    if ((I2V_RETURN_OK == ret) && (cfg.security)) {
        if ((init_with_vod(APP_NAME)) == TRUE)   {
            initVOD = TRUE;
        }
        else {
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"VOD Initialization Failed \n");
        }
    }
#endif
    if (I2V_RETURN_OK == ret) {
        /* Register radio service and change channel */
        uService.action = ADD;
        /* Register to receive WSMs */
        wService.action = ADD;

        if(1 == cfg.RadioType) {
            uService.radioType = RT_CV2X;
            wService.radioType = RT_CV2X;
        } else {
            uService.radioType = RT_DSRC;
            wService.radioType = RT_DSRC;
        }
        
        wService.psid = cfg.srmPsid;
        uService.psid = cfg.srmPsid;
//TODO: Do i really need to have user srv for cb?
        //I2V_UTIL_USER_SVC_REQ(&uService, &cfg, WMH_SVC_TIMEOUT, &ret, &optret);
        optret = optret;
#if !defined(MY_UNIT_TEST)
        if (I2V_RETURN_OK == ret) {
            I2V_UTIL_REGISTER_WSM_CB(&cfg, WMH_SVC_TIMEOUT, &ret, uService.radioNum, uService.psid, &loop, receive_srm, NULL)
            if (I2V_RETURN_OK != ret) {
                ret = I2V_RADIO_WSM_SERVICE_FAIL;
                set_my_error_state(SRM_WSM_REQ_FAIL);
            }
        } else {
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"I2V_UTIL_USER_SVC_REQ: Failed. ret=%d optret=%u.\n",ret, optret);
            ret = I2V_RADIO_USR_SERVICE_FAIL;
            set_my_error_state(SRM_USER_SERVICE_FAIL);
        }
#endif
    }
    if (I2V_RETURN_OK == ret) {
        while(loop) { 
            if(0 == (debug_counter % OUTPUT_MODULUS)) {
                I2V_DBG_LOG(LEVEL_INFO,MY_NAME,"(0x%lx) rx,E(%u,%u) SCSFWD sent,E(%u,%u)\n",
                    srm_error_states,srm_rx_counts, srm_rx_err_counts,rx_msg_write_count,rx_msg_write_error_count);
            }
            if(0 == (debug_counter % 120)) {
                dump_srm_report();
            }
            debug_counter++;
            usleep(SRM_MAIN_DEFAULT_SLEEP_TIME);   /* Main loop, stay here forever - Until Ctrl-C or loop != WTRUE */

            /* Log Security related stats once every 5 minutes */
            if (cfg.security)   {

                if (++srmSecStatsIntCnt >= SRM_SECURITY_STATS_LOG_INTERVAL) {
                    srmSecStatsIntCnt = 0;
                    WSU_SEM_LOCKR(&shm_ptr->scsSRMData.h.ch_lock);
                    memcpy(&secLogStats,&shm_ptr->scsSRMData.srmSecurityStats,sizeof(SRMSecStats));
                    WSU_SEM_UNLOCKR(&shm_ptr->scsSRMData.h.ch_lock);
                    
                    I2V_DBG_LOG(LEVEL_PRIV,MY_NAME,"Sec Ver Status: %d Ver Code: %d Vod Req: %u Vod Suc: %u Vod Fail: %u \n",
                    secLogStats.srmVTPVerifyStatus,
                    secLogStats.srmVTPVerifyCode,
                    secLogStats.srmVODRequests,
                    secLogStats.srmVODSuccesses,
                    secLogStats.srmVODFailures);
                }
            }

            #if defined(MY_UNIT_TEST)
            if ((TEST_ITERATION_MAX * 3) < debug_counter){
                loop = WFALSE;
                break;
            }
            #endif
        }
        cleanup();
    } /* if ret == I2V_RETURN_OK */
#if defined(EXTRA_DEBUG)
    /* Dump final state to log. */
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"EXIT: ret(%d) (0x%lx) SCSFWD sent,E(%u,%u)\n",ret,srm_error_states,rx_msg_write_count,rx_msg_write_error_count);
#endif
    /* Stop I2V_DBG_LOG output. Last chance to output to syslog. */
    i2vUtilDisableSyslog();
    i2vUtilDisableDebug();
    if(SRM_SHM_FAIL != ret) {
        wsu_share_kill(shm_ptr, sizeof(i2vShmMasterT));
        shm_ptr = NULL;
    }
#if !defined(MY_UNIT_TEST)
    if (initVOD)
        denit_with_vod();
#endif
    return abs(ret);

} /* End of Main() */

