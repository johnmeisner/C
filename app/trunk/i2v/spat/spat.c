/**************************************************************************
 *                                                                        *
 *     File Name:  spat.c                                                 *
 *     Author:                                                            *
 *         DENSO International America, Inc.                              *
 *         North America Research Laboratory, California Office           *
 *         Rutherford road                                                *
 *         Carlsbad, CA 92008                                             *
 *                                                                        *
 **************************************************************************/

#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <poll.h>
#include <fcntl.h>
#include "wsu_util.h"
#include "wsu_shm.h"
#include "i2v_general.h"
#include "ris_struct.h"
#include "ris.h"
#include "i2v_util.h"
#include "i2v_shm_master.h"
#if defined(J2735_2016)
#include "DSRC.h"
#elif defined(J2735_2023)
#include "MessageFrame.h"
#include "Common.h"
#include "SPAT.h"
#else
#ERROR: You must define  J2735_2016 or J2735_2023 in v2xcommon.mk
#endif
#include "rtxsrc/rtxMemLeakCheck.h"
#include "rtxsrc/rtxDiag.h"
#include "i2v_riscapture.h"
#include "spat.h"
#include "libtsmapreader.h"
#include "ntcip-1218.h"
#include "conf_table.h"

#if defined(MY_UNIT_TEST)
#include "stubs.h"
#define MAIN my_main
extern void unit_test_spat_config_twiddle(void);
#else
#define MAIN main
#define RTXMEMALLOC rtxMemAlloc
#endif

#if defined(EXTRA_DEBUG)
#define OUTPUT_MODULUS  3000  /* blobs rx. */
#else
#define OUTPUT_MODULUS  12000
#endif

/* mandatory logging defines */
#if defined(MY_UNIT_TEST)
#define MY_ERR_LEVEL   LEVEL_DBG
#else
#define MY_ERR_LEVEL   LEVEL_PRIV    /* from i2v_util.h */
#endif
#define MY_NAME        "spat"

/* allows up to 90 ms for iwmh response - need to be faster than 100 ms */
#define SPAT_DEF_WMH_TO  50 /* Should be very quick reply or give up. */

mqd_t fwdmsgfd;     
fwdmsgData spatFwdmsgData;
bool_t  spatForward;
char sendbuf[sizeof(spatFwdmsgData)];
pthread_mutex_t fwdmsg_send_lock = PTHREAD_MUTEX_INITIALIZER;

/* Shared with bsmrx16.c */
bool_t  bsmForward;
bool_t  bsmpsmForwardF;
i2vShmMasterT * shm_ptr; 
bool_t  mainloop = WTRUE;
spatCfgItemsT spat_cfg;
STATIC char_t confFileName[I2V_CFG_MAX_STR_LEN];
cfgItemsT cfg;
fwdmsgCfgItemsT fwdmsgCfg;
uint64_t my_error_states = 0x0;
uint32_t spat_tx_count = 0x0;
uint32_t spat_tx_err_count = 0x0;
static appCredentialsType appCredential = {
    .cbServiceAvailable = NULL,
    .cbReceiveWSM     = NULL,
};

static UserServiceType spatUService = {
    .radioNum = 0,
    .action = ADD,
    .userAccess = AUTO_ACCESS_UNCONDITIONAL,
    .psid = 0x8002,
    .servicePriority = 10,
    .wsaSecurity = UNSECURED_WSA,
    .lengthPsc = 10,
    .channelNumber = 183,
    .lengthAdvertiseId = 15,
    .linkQuality = 20,
    .immediateAccess = 1,
    .extendedAccess = 65535, 
    .radioType      = RT_DSRC,   
};

ProviderServiceType spatPService = {
    .action = ADD,
    .destMacAddress = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
    .wsaSecurity = UNSECURED_WSA,
    .psid = 0x8002,
    .lengthPsc = 0,
    .servicePriority = 0,
    .channelNumber = 183,
    .channelAccess = 2,  
    .repeatRate = I2V_DEFAULT_WSA_REPEAT_RATE,
    .ipService = 0,
    .IPv6ServicePort = 23000,
    .providerMACAddress = {0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE},
    .rcpiThreshold = 0,
    .wsaCountThreshold = 0,
    .wsaCountThresholdInterval = 0,
    .wsaHeaderExtension = 0,
    .radioType      = RT_CV2X,
};

static WsmServiceType service = {
     .action    = ADD,
     .psid      = 0x8002,
    .radioType  = RT_CV2X,
};  

static outWSMType outWSMMsg = {
    .dataRate = DR_6_MBPS,
    .txPwrLevel = 20,
    .txPriority = 2,
    .wsmpExpireTime = 0,
    .radioNum = 0,
    .channelInterval = 0,
    .security = SECURITY_UNSECURED,
    .securityFlag = 0,  
    .peerMacAddress = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff},
    .wsmpHeaderExt = 0,
    .radioType = RT_CV2X,
};
static OSCTXT spat_ctxt;

/* see comments in i2v_riscapture.h */
static i2vRisStruct svctrack;
static i2vRisStruct bsmtrack;

/* In the future perhaps we'll support more than one. */
#define MY_MAX_SPAT16_MOVE_EVENT  (1)

static int32_t my_max_move_states = 1;
static int32_t my_max_move_event  = 1;
static uint8_t MY_REVISION_COUNTER = 0; /*sequence number for messages*/
static uint32_t spat16_FwdProcessCount = 0;

STATIC i2vMovementPhaseState prior_eventState[SPAT_MAX_APPROACHES];

#if defined(BSM_PSM_FORWARDING) /* Grabs from AMH SHM and not part of radio callbacl. */
static pthread_t bsmpsmForwardThreadID;
extern void *bsmpsmForwardThread(void __attribute__((unused)) *arg);
#endif

static pthread_t CommRangeThreadID;
extern void *CommRangeThread(void __attribute__((unused)) *arg);

static pthread_t ReceivedMsgThreadID;
extern void *ReceivedMsgThread(void __attribute__((unused)) *arg);

static pthread_t MessageStatsThreadID;
extern void *MessageStatsThread(void __attribute__((unused)) *arg);
extern messageStats_t messageStats[RSU_PSID_TRACKED_STATS_MAX];

/* Manage forwarding socket. Init and recover from failures. */
#define SOCKET_POLL_TIMEOUT  0       /* Zero wait. Don't hang around if not ready. */
#define FWD_SOCKET_MANAGEMENT_RATE 5 /* Seconds. */
#define FWD_WRITE_ERROR_THRESHOLD 10 /* Consecutive write fails to socket that triggers reset. */

static void Init_SPAT_Statics(void)
{
  shm_ptr  = NULL;
  mainloop = WTRUE;
  memset(&spat_cfg,0x0,sizeof(spat_cfg));
  memset(&cfg,0x0,sizeof(cfg));

  //appCredential.cbServiceAvailable = NULL;
  appCredential.cbReceiveWSM     = bsmRxHandler;

  spatUService.radioNum = 0;
  spatUService.action = ADD;
  spatUService.userAccess = AUTO_ACCESS_UNCONDITIONAL;
  spatUService.psid = 0x8002;
  spatUService.servicePriority = 10;
  spatUService.wsaSecurity = UNSECURED_WSA;
  spatUService.lengthPsc = 10;
  spatUService.channelNumber = 172;
  spatUService.lengthAdvertiseId = 15;
  spatUService.linkQuality = 20;
  spatUService.immediateAccess = 1;
  spatUService.extendedAccess = 65535; 

  spatPService.action = ADD;
  memset(spatPService.destMacAddress,0xFF,LENGTH_MAC);
  spatPService.radioNum = 0;
  spatPService.wsaSecurity = UNSECURED_WSA;
  spatPService.psid = 0x8002;
  spatPService.lengthPsc = 0;
  spatPService.servicePriority = 0;
  spatPService.channelNumber = 0;
  spatPService.channelAccess = 2;      
  spatPService.repeatRate = I2V_DEFAULT_WSA_REPEAT_RATE;
  spatPService.ipService = 0;
  spatPService.IPv6ServicePort = 23000;
  memset(spatPService.providerMACAddress,0xEE,LENGTH_MAC);
  spatPService.rcpiThreshold = 0;
  spatPService.wsaCountThreshold = 0;
  spatPService.wsaCountThresholdInterval = 0;
  spatPService.wsaHeaderExtension = 0;

  service.action    = ADD;
  service.psid      = 0x8002;

  if(1 == cfg.RadioType) {
      service.radioType      = RT_CV2X;
      spatPService.radioType = RT_CV2X;
      spatUService.radioType = RT_CV2X;
      outWSMMsg.radioType    = RT_CV2X;
  }  else {
      service.radioType      = RT_DSRC;
      spatPService.radioType = RT_DSRC;
      spatUService.radioType = RT_DSRC;
      outWSMMsg.radioType    = RT_DSRC;
  }

  outWSMMsg.dataRate = DR_6_MBPS,
  outWSMMsg.txPwrLevel = 20;
  outWSMMsg.txPriority = 2;
  outWSMMsg.wsmpExpireTime = 0;
  outWSMMsg.radioNum = 0;
  outWSMMsg.channelInterval = 0;
  outWSMMsg.security = SECURITY_UNSECURED;
  outWSMMsg.securityFlag = 0;

  memset(outWSMMsg.peerMacAddress,0xFF,LENGTH_MAC);

  outWSMMsg.wsmpHeaderExt = 0;

  memset(&spat_ctxt,0x0,sizeof(spat_ctxt));

  memset(&svctrack,0x0,sizeof(svctrack));
  memset(&bsmtrack,0x0,sizeof(bsmtrack));

  my_max_move_states = 1;
  my_max_move_event  = 1;

  MY_REVISION_COUNTER = 0; 
  spat16_FwdProcessCount = 0;
  Init_BSMRX_Statics();
  memset(prior_eventState,MPS_UNAVAILABLE,sizeof(prior_eventState));
  my_error_states = 0x0;
  spat_tx_count = 0x0;
  spat_tx_err_count = 0x0;
#if !defined(MY_UNIT_TEST) /* Select flavor in unit_test. */
    strncpy(confFileName,SPAT_CFG_FILE,sizeof(confFileName));
#endif
  memset(&fwdmsgfd,0x0,sizeof(fwdmsgfd));     
  memset(&spatFwdmsgData,0x0,sizeof(spatFwdmsgData));
  spatForward = WFALSE;
  memset(sendbuf,0x0,sizeof(sendbuf));
  memset(&fwdmsg_send_lock,0x0,sizeof(fwdmsg_send_lock));
  bsmForward=WFALSE;
  bsmpsmForwardF=WFALSE;
  memset(&fwdmsgCfg,0x0,sizeof(fwdmsgCfg));
}
void set_my_error_state(int32_t my_error)
{
  int32_t dummy = 0;

  dummy = abs(my_error);

  if((dummy < SPAT16_ERROR_TOP) && (SPAT16_ERROR_BASE < dummy)) {
      my_error_states |= (uint64_t)(0x1) << (dummy - 1);
  }
}
void clear_my_error_state(int32_t my_error)
{
  int32_t dummy = 0;

  dummy = abs(my_error);
  if((dummy < SPAT16_ERROR_TOP) && (SPAT16_ERROR_BASE < dummy)) {
      my_error_states &= ~((uint64_t)(0x1) << (dummy - 1));
  }
}
int32_t is_my_error_set(int32_t my_error)
{
  int32_t dummy = 0;
  int32_t ret = 0; /* FALSE till proven TRUE */

  dummy = abs(my_error);
  if((dummy < SPAT16_ERROR_TOP) && (SPAT16_ERROR_BASE < dummy)) {
      if (my_error_states & ((uint64_t)(0x1) << (dummy - 1))) {
          ret = 1;
      }
  }
  return ret;
}
/*----------------------------------------------------------------------------*/
/* Dump to /tmp for user to cat. Only actionable items for user.              */
/*----------------------------------------------------------------------------*/
STATIC void dump_spat_report(void)
{
  FILE * file_out = NULL;
  if ((file_out = fopen("/tmp/spat.txt", "w")) == NULL){
      if(0x0 == is_my_error_set(SPAT16_CUSTOMER_DIGEST_FAIL)) { /* Report fail one time. */
          set_my_error_state(SPAT16_CUSTOMER_DIGEST_FAIL);
      }
  } else {
      clear_my_error_state(SPAT16_CUSTOMER_DIGEST_FAIL);
      if(is_my_error_set(SPAT16_CONF_PHASE_ENTRIES_ZERO)){
          fprintf(file_out,"SPAT16_CONF_PHASE_ENTRIES_ZERO: Not getting any data from SCS(controller). Check IP+Port is valid and no conflict exists.\n");
      }
      if(is_my_error_set(SPAT16_BSMRX_PSID_MISMATCH)){
          fprintf(file_out,"SPAT16_BSMRX_PSID_MISMATCH: Expected PSID for BSM is different. In Web Gui go to AdStatus and check BSMRxPSIDPERU is desired value.\n");
      }
      if(is_my_error_set(SPAT16_BSM_FWD_DROPPED)){
          fprintf(file_out,"SPAT16_BSM_FWD_DROPPED: Friendly reminder, BSMs received are not being forwarded. In Web Gui go to Config->Forward Message if you want to forward.\n");
      }
      if(is_my_error_set(SPAT16_TSCBM_FOPEN_FAIL)){
          fprintf(file_out,"SPAT16_TSCBM_FOPEN_FAIL: TSCBM is undefined. In Web Gui go to Config->Interesection Settings->Signal Group Mapping and create mapping for this intersection.\n");
      }
      fflush(file_out);
      fclose(file_out);
      file_out = NULL;
  }
}
STATIC void spat_sighandler(int __attribute__((unused)) sig)
{
#if !defined(MY_UNIT_TEST)
  mainloop = WFALSE;
#endif
  set_my_error_state(SPAT16_SIG_FAULT);
  /* otherwise spat may get stuck waiting for scs which may already be killed */
  if(NULL != shm_ptr)
      wsu_open_gate(&shm_ptr->scsSpatData.spatTriggerGate);

}

static void Free_SPAT(SPAT *spat)
{
#if defined(MY_UNIT_TEST)
  spat = spat;
#else
  IntersectionState *int_state;
  OSRTDListNode *node;

  node = rtxDListFindByIndex(&spat->intersections, 0); /* only one intersection */
  int_state = node->data;

  rtxDListFreeAll(&spat_ctxt, &int_state->states);

  if (int_state->m.namePresent == 1) {
      free((void *)int_state->name);
  }

  rtxDListFreeAll(&spat_ctxt, &spat->intersections);
#if defined(J2735_2016)
  asn1Free_SPAT(&spat_ctxt,spat);
#endif
  rtxMemReset(&spat_ctxt);
  rtxMemFree(&spat_ctxt);
  rtxErrReset(&spat_ctxt);
#endif
  return;
}

/* can be used to obtain seconds in hour or tenths of seconds in hour */
static uint32_t getCurrSecInHour(bool_t convertToTenths)
{
  i2vTimeT clock;
  uint32_t output;
  struct timeval tv;

  i2vUtilGetUTCTime(&clock);
  output = clock.min * 60 + clock.sec;

  if (convertToTenths) {
      gettimeofday(&tv, NULL);
      output *= 10;   /* converting seconds to tenths of seconds */
      output += tv.tv_usec/100000;   /* usec converted to tenths */
  }

  return output;
}

/* SPAT16 specific.
 * This version does not use file I/O and saves upto 30 msecs.
 * Also returns milliseconds of minute need for SPAT16 creation.
 */
STATIC uint32_t getCurrMinVal(uint16_t * millisecond)
{
  uint64_t my_minute      = 0;       
  uint64_t my_millisecond = 0;
  uint64_t temp           = 0;
  wtime    spat_minute    = 0;
  i2vTimeT clock;

    my_minute      = i2vUtilGetTimeInMs();
    temp           = my_minute;
    my_minute      = my_minute/(uint64_t)(60000); /*This also drops the milliseconds out of the value*/
    my_millisecond = (temp - (my_minute * (uint64_t)(60000))); /*So when we do this we can get the msec of the moy*/

    i2vUtilGetUTCTime(&clock);

    spat_minute = clock.yday * 24 * 60;

    spat_minute += (clock.hour * 60) + clock.min;

    //my_millisecond += (my_minute * 60 * 1000) + (clock.sec * 1000);
    
    *millisecond   = (uint16_t)my_millisecond;

    if(527040 < spat_minute){
      spat_minute = 0;
    }
    
    if(( 59999 < *millisecond) && (*millisecond < 61000)){ 
        /*leap second...Assuming system takes care of this and not sure how we detect here.*/
    }
    if(60999 < *millisecond){
       *millisecond = 65535; /* unavailable. Not sure how we detect this? */
    }   

    return (uint32_t)spat_minute;
}

/* asn1Fill_SPAT(SPAT *spat, scsSpatInfo *spat_info)  
 *     Processes scsSpatInfo * spat_info into SPAT * spat
 *     Puts data into global spat_ctxt
 *     Creates IntersectionState with int_state and spat_ctxt
 */
STATIC int32_t asn1Fill_SPAT(SPAT *spat, scsSpatInfo *spat_info)  
{
  IntersectionState * int_state = NULL; /* 1..32 */
#if defined(J2735_2023)
    IntersectionReferenceID * my_id = NULL;
#endif
    MovementState           * move_state = NULL; /* 1..255 */
      MovementEvent            * move_event = NULL; /* 1..16 */
      ConnectionManeuverAssist * cnxnAssist = NULL; /* for pedestrian detection */
  int32_t   i = 0;
  uint16_t  moy_milliseconds = 0;
  int32_t   approach_idx = 0;

    if(NULL == shm_ptr) {
        if(0x0 == (my_error_states & SPAT16_SHM_FAIL)) {
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"asn1Fill_SPAT:SPAT16_SHM_FAIL\n");
            my_error_states |= SPAT16_SHM_FAIL;
        }
        return -1;   
    }
    spat->m.timeStampPresent = 0;
    spat->m.namePresent      = 0;
    spat->m.regionalPresent  = 0;
    spat->m.timeStampPresent = 1;
    spat->timeStamp          = getCurrMinVal(&moy_milliseconds); /*minutes of the year & milliseconds of minute*/
    int_state = rtxMemAlloc(&spat_ctxt, sizeof(IntersectionState));
    if(NULL == int_state) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"SPAT16_ASN1_SPAT_CREATE_FAIL: NULL == int_state.\n");
        set_my_error_state(SPAT16_ASN1_SPAT_CREATE_FAIL);
        return -2;
    }
    memset(int_state, 0, sizeof(IntersectionState));
    rtxDListInit(&spat->intersections);
    int_state->m.namePresent               = 0;
    int_state->m.moyPresent                = 0;
    int_state->m.timeStampPresent          = 0;
    int_state->m.enabledLanesPresent       = 0;
    int_state->m.maneuverAssistListPresent = 0;
    int_state->m.regionalPresent           = 0;
    int_state->m.timeStampPresent  = 1;
    int_state->timeStamp           = moy_milliseconds;

    // Count this loop and mark the time
    ++shm_ptr->scsSpatData.metrics.cnt_asn1fill_spat_calls;
    shm_ptr->scsSpatData.spat16_process_timestamp = time(0L);

    // save the TSCBM's spatIntersectionStatus as well as the J2735 IntersectionStatusObject to output_copy
        // for (unit) testing and WebUI display
    shm_ptr->scsSpatData.output_copy.TSCBMIntersectionStatus = spat_info->flexSpat.intersectionStatus;
    shm_ptr->scsSpatData.output_copy.IntersectionStatus = spat_info->flexSpat.intersectionStatus << 8;  // the 8 bits from the TSCBM instersection status are the higher bits of J2735
    shm_ptr->scsSpatData.output_copy.IntersectionStatus &= (MANUAL_CONTROL_IS_ENABLED | STOP_TIME_IS_ACTIVATED | FAILURE_FLASH | PREEMPT_IS_ACTIVE | SIGNAL_PRIORITY_IS_ACTIVE | STANDBY_OPERATION);
                                                        // only 6 of the bits in the TSCBM's spatIntersectionStatus make it into J2735's IntersectionStatusObject
    
    int_state->status.numbits = 16;     // J2735 output has 16 bits (IntersectionStatusObject)
    int_state->status.data[0] = shm_ptr->scsSpatData.output_copy.IntersectionStatus >> 8;
    int_state->status.data[1] = shm_ptr->scsSpatData.output_copy.IntersectionStatus & 0xFF;
#if defined(J2735_2016)
    int_state->id.id  = spat_cfg.IntersectionID; /* MAP sent via AMH. We'll never see or decode it */
#else //(J2735_2023)
    my_id = rtxMemAlloc(&spat_ctxt, sizeof(IntersectionReferenceID));
    if(NULL == my_id) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"SPAT16_ASN1_SPAT_CREATE_FAIL: NULL == int_state->id.\n");
        set_my_error_state(SPAT16_ASN1_SPAT_CREATE_FAIL);
        return -3;
    }
    my_id->m.regionPresent = 0;
    my_id->id              = spat_cfg.IntersectionID;
    int_state->id          = my_id;
#endif

    int_state->revision = MY_REVISION_COUNTER++; 
    if(128 == MY_REVISION_COUNTER){
        MY_REVISION_COUNTER = 0;
    }
    rtxDListInit(&int_state->states);
#if defined(J2735_2016)
    shm_ptr->scsSpatData.output_copy.intersection_id = int_state->id.id;
#else //(J2735_2023)
    shm_ptr->scsSpatData.output_copy.intersection_id = spat_cfg.IntersectionID;
#endif
    shm_ptr->scsSpatData.output_copy.revision_counter = int_state->revision;

    my_max_move_states = SPAT_MAX_APPROACHES; /* just physical limits on size of message    */
    my_max_move_event  = MY_MAX_SPAT16_MOVE_EVENT;
    if (spat_cfg.interopMode & INTEROP_SIMPLE_SPAT) {
        my_max_move_event  = 1;
    }

    /* Main Processing Loop */
    for (i=0; i < spat_cfg.numEntries; i++) {

        // get the index into .spatApproach to use - based on phase type & number
        if (spat_cfg.phaseType[i] == TSMAP_VEHICLE) {
            approach_idx = spat_cfg.phaseNumber[i]-1;
        } else {
            approach_idx = spat_cfg.phaseNumber[i]+16-1;
        }
        
        // at this point, i is the index to the spat_cfg config info and the J2735 output,
        // and approach_idx is the index into the TSCBM input

            // Create a J2735 entry for this spat_cfg row
            move_state = rtxMemAlloc(&spat_ctxt, sizeof(MovementState));
            memset(move_state , 0, sizeof(MovementState));
            move_state->m.movementNamePresent       = 0;
            move_state->m.maneuverAssistListPresent = 0;
            move_state->m.regionalPresent           = 0;
            rtxDListInit(&move_state->state_time_speed);

            // Set siggrpid in move state and current output row
            move_state->signalGroup = spat_cfg.signalGroupID[i];
            shm_ptr->scsSpatData.output_copy.signal_group_id[i] = move_state->signalGroup;

            /* Create move event for this siggrpid */
            move_event = rtxMemAlloc(&spat_ctxt, sizeof(MovementEvent));
            memset(move_event , 0, sizeof(MovementEvent));
            move_event->m.timingPresent   = 0;
            move_event->m.speedsPresent   = 0;
            move_event->m.regionalPresent = 0;

            /* Determine event state (movement phase state) */
            move_event->eventState = i2v_signal_phase_to_movement_phase_state(
                                        spat_info->flexSpat.spatApproach[approach_idx].curSigPhase,
                                        spat_cfg.channelGreenType[i],
                                        prior_eventState[i]);

            /* Record inputs & result for output copy */
            /* NOTE: Assumes only one move event per signal_group_id */
            shm_ptr->scsSpatData.output_copy.tsc_phase_number[i] = spat_cfg.phaseNumber[i];
            shm_ptr->scsSpatData.output_copy.tsc_phase_type[i] = spat_cfg.phaseType[i];
            shm_ptr->scsSpatData.output_copy.signal_phase[i] = spat_info->flexSpat.spatApproach[approach_idx].curSigPhase;
            shm_ptr->scsSpatData.output_copy.channel_green_type[i] = spat_cfg.channelGreenType[i];
            shm_ptr->scsSpatData.output_copy.prior_event_state[i] = prior_eventState[i];
            shm_ptr->scsSpatData.output_copy.final_event_state[i] = move_event->eventState;

            // Save current eventState for the next decode loop.
            //   Note that index of prior values array is the spat_cfg index, "i".
            prior_eventState[i] = move_event->eventState;

            // Set a few more move_event fields
            move_event->m.timingPresent   = 1;
            move_event->timing.minEndTime = spat_info->flexSpat.spatApproach[approach_idx].timeNextPhase;

#if defined(J2735_2016)
            /*
            7.194 Data Element: DE_TimeMark
            Use: The TimeMark data element is used to relate a moment in UTC 
            (Coordinated Universal Time)-based time when a signal phase is predicted to change, 
            with a precision of 1/10 of a second. A range of 60 full minutes is supported and it 
            can be presumed that the receiver shares a common sense of time with the sender which 
            is kept aligned to within a fraction of a second or better.
            If there is a need to send a value greater than the range allowed by the data element 
            (over one hour in the future), the value 36000 shall be sent and shall be interpreted to 
            indicate an indefinite future time value. When the value to be used is undefined or 
            unknown a value of 36001 shall be sent. 
            Note that leap seconds are also supported.
            ASN.1 Representation:
            TimeMark ::= INTEGER (0..36001) 
              -- Tenths of a second in the current or next hour 
              -- In units of 1/10th second from UTC time 
              -- A range of 0~36000 covers one hour 
              -- The values 35991..35999 are used when a leap second occurs 
              -- The value 36000 is used to indicate time >3600 seconds 
              -- 36001 is to be used when value undefined or unknown 
              -- Note that this is NOT expressed in GPS time 
              -- or in local time
            */
            /* JJG for compatibility */
            if (spat_cfg.interopMode & INTEROP_UTC_SPAT) {
                if (move_event->timing.minEndTime == 1202) {   /* ICD magic value for undefined/unknown */
                    move_event->timing.minEndTime = 36001;   /* J2735 magic value for undefined/unknown */
                } else if (move_event->timing.minEndTime == 1201) {   /* ICD magic value for indefinite */
                    move_event->timing.minEndTime = 36000;   /* J2735 magic value for max time */
                } else {
                    move_event->timing.minEndTime += getCurrSecInHour(WTRUE);
                    if (move_event->timing.minEndTime >= 36000) {
                        move_event->timing.minEndTime -= 36000;   /* EndTime is in the next hour */
                    }
                }
            }
            // Adjust Min & Max EndTimes.  Note >=36000 rolls over into the next hour
            if (spat_cfg.interopMode & INTEROP_MIN_AND_MAX_SPAT) {
                move_event->timing.m.maxEndTimePresent = 1;
                move_event->timing.maxEndTime = spat_info->flexSpat.spatApproach[approach_idx].secondaryTimeNextPhase;
                if (spat_cfg.interopMode & INTEROP_UTC_SPAT) {
                    if (move_event->timing.maxEndTime == 1202) {   /* ICD magic value for undefined/unknown */
                        move_event->timing.maxEndTime = 36001;   /* J2735 magic value for undefined/unknown */
                    } else if (move_event->timing.maxEndTime == 1201) {   /* ICD magic value for indefinite */
                        move_event->timing.maxEndTime = 36000;   /* J2735 magic value for max time */
                    } else {
                        move_event->timing.maxEndTime += getCurrSecInHour(WTRUE);
                        if (move_event->timing.maxEndTime >= 36000) {
                            move_event->timing.maxEndTime -= 36000;   /* EndTime is in the next hour */
                        }
                    }
                }
            }
#else //(J2735_2023)
            /*
            7.206 Data Element: DE_TimeMark
            Use: The TimeMark data element is used to relate a moment in UTC 
            (Coordinated Universal Time)-based time when a signal phase is predicted to change, 
            with a precision of 1/10 of a second. A range of 60 full minutes is supported and it 
            can be presumed that the receiver shares a common sense of time with the sender which 
            is kept aligned to within a fraction of a second or better.
            If the value of TimeMark is greater than the current time, it applies in the current hour, 
            and if it is less than the current time, it applies in the next hour. 
            When the value to be used is undefined or unknown, a value of 36111 shall be sent. 
            Note that leap seconds are also supported.
            TimeMark ::= INTEGER (0..36111)
            -- In units of 1/10th second from UTC time in the current or next hour
            -- A range of 0~35999 covers one hour
            -- The values 36000..36009 are used when a leap second occurs
            -- The values 36010..36110 are reserved for future use
            -- 36111 is to be used when the value is undefined or unknown
            -- Note that this is NOT expressed in GPS time or in local time
            */
            /* JJG for compatibility */
            if (spat_cfg.interopMode & INTEROP_UTC_SPAT) {
                if (move_event->timing.minEndTime == 1202) {   /* ICD magic value for undefined/unknown */
                    move_event->timing.minEndTime = 36111;   /* J2735 magic value for undefined/unknown */
                } else if (move_event->timing.minEndTime == 1201) {   /* ICD magic value for indefinite */
                    move_event->timing.minEndTime = 35999;   /* J2735 magic value for max time */
                } else {
                    move_event->timing.minEndTime += getCurrSecInHour(WTRUE);
                    if (move_event->timing.minEndTime >= 36000) {
                        move_event->timing.minEndTime -= 36000;   /* EndTime is in the next hour */
                    }
                }
            }
            // Adjust Min & Max EndTimes.  Note >=36000 rolls over into the next hour
            if (spat_cfg.interopMode & INTEROP_MIN_AND_MAX_SPAT) {
                move_event->timing.m.maxEndTimePresent = 1;
                move_event->timing.maxEndTime = spat_info->flexSpat.spatApproach[approach_idx].secondaryTimeNextPhase;
                if (spat_cfg.interopMode & INTEROP_UTC_SPAT) {
                    if (move_event->timing.maxEndTime == 1202) {   /* ICD magic value for undefined/unknown */
                        move_event->timing.maxEndTime = 36111;   /* J2735 magic value for undefined/unknown */
                    } else if (move_event->timing.maxEndTime == 1201) {   /* ICD magic value for indefinite */
                        move_event->timing.maxEndTime = 35999;   /* J2735 magic value for max time */
                    } else {
                        move_event->timing.maxEndTime += getCurrSecInHour(WTRUE);
                        if (move_event->timing.maxEndTime >= 36000) {
                            move_event->timing.maxEndTime -= 36000;   /* EndTime is in the next hour */
                        }
                    }
                }
            }
#endif

            rtxDListAppend(&spat_ctxt, &move_state->state_time_speed, move_event);       
            /* Record Timing for output copy */
            shm_ptr->scsSpatData.output_copy.min_end_time[i] = move_event->timing.minEndTime;
            shm_ptr->scsSpatData.output_copy.max_end_time[i] = move_event->timing.maxEndTime;

            if (spat_cfg.interopMode & INTEROP_PED_PRESENCE) {
              move_state->m.maneuverAssistListPresent = 1;
              rtxDListInit(&move_state->maneuverAssistList);
              cnxnAssist = rtxMemAlloc(&spat_ctxt, sizeof(ConnectionManeuverAssist));
              memset(cnxnAssist , 0, sizeof(ConnectionManeuverAssist));
              cnxnAssist->m.pedBicycleDetectPresent = 1;
              /* hardcode the connectionID to 0; already set by memset but just so devs know */
              cnxnAssist->connectionID = 0;
              cnxnAssist->pedBicycleDetect = spat_info->flexSpat.spatApproach[approach_idx].pedPresent;
              rtxDListAppend(&spat_ctxt, &move_state->maneuverAssistList, cnxnAssist);
            }
            rtxDListAppend(&spat_ctxt, &int_state->states, move_state);
    } /* for i ...*/

    /* Send if not empty.  Trying to send nothing will cause nasty error. */
    if (spat_cfg.numEntries > 0 && spat_cfg.numEntries <= SPAT_MAX_APPROACHES) {
        rtxDListAppend(&spat_ctxt, &spat->intersections, int_state);
    }

    // Now, the number of output rows now is the number of spat_cfg rows
    shm_ptr->scsSpatData.output_copy.num_groups = spat_cfg.numEntries;

    // Collect max & min and empty/nonemtpy statistics
    if (shm_ptr->scsSpatData.output_copy.num_groups > shm_ptr->scsSpatData.metrics.max_num_groups) {
        shm_ptr->scsSpatData.metrics.max_num_groups = shm_ptr->scsSpatData.output_copy.num_groups;
    }
    if (shm_ptr->scsSpatData.output_copy.num_groups < shm_ptr->scsSpatData.metrics.min_num_groups) {
        shm_ptr->scsSpatData.metrics.min_num_groups = shm_ptr->scsSpatData.output_copy.num_groups;
    }
    if (shm_ptr->scsSpatData.output_copy.num_groups == 0) {
        ++shm_ptr->scsSpatData.metrics.cnt_empty_spat_fills;
    } else {
        ++shm_ptr->scsSpatData.metrics.cnt_nonempty_spat_fills;
    }

    return 0;
}

STATIC OSOCTET *spatASN1EncodePER(outWSMType *wsm, SPAT *spat, uint8_t aligned)
{
    int32_t len = 0;
    OSOCTET *msg = NULL;

    if( 0 != pu_setBuffer(&spat_ctxt, wsm->data, sizeof(wsm->data), aligned)) {
        return NULL;
    }
    if (asn1PE_SPAT(&spat_ctxt, spat) != 0) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME," ASN1 PER %d Encode error\n", aligned);
        rtxErrPrint(&spat_ctxt);
        wsm->dataLength = 0;
        return NULL;
    } else {
        msg = pe_GetMsgPtr(&spat_ctxt, &len);
        wsm->dataLength = len;
        return msg;
    }
}

/****************************************************************
* J2735 2016 we need messageFrame. Contains message id etc       
* 
* return 1 == SUCCESS, return -1 == FAILURE
*
*****************************************************************/
STATIC int32_t uperFrame_SPAT(uint8_t     * blob
                            ,uint32_t       blobLength
                            ,MessageFrame * frame
                            ,OSCTXT       * ctxt)
{

    if(  (NULL == blob) 
      || (NULL == frame) 
      || (NULL == ctxt) 
      || (0    == blobLength)
      ) {
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"uperFrame_SPAT(error)::bad param:::blb=%p 0x%x,frm=%p,txt=%p\n"
                  ,blob,blobLength,frame,ctxt);
      set_my_error_state(SPAT16_UPERFRAME_SPAT_BAD_PARAMS);
      return SPAT16_UPERFRAME_SPAT_BAD_PARAMS; 
    }
    if( asn1Init_MessageFrame(frame) != 0){
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"uperFrame_SPAT::asn1Init_MessageFrame(error).\n"); 
      set_my_error_state(SPAT16_ASN1_INIT_MESSAGE_FRAME_FAIL);
      return SPAT16_ASN1_INIT_MESSAGE_FRAME_FAIL;
    }

    frame->value.data = RTXMEMALLOC(ctxt, sizeof(blobLength));

    if(NULL == frame->value.data){
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"rtxMemAlloc(error).\n");
      set_my_error_state(SPAT16_RTX_MEM_ALLOC_FAIL); 
      return SPAT16_RTX_MEM_ALLOC_FAIL;
    } 
    frame->messageId     = ASN1V_signalPhaseAndTimingMessage;
    frame->value.numocts = blobLength;
    memcpy((void *)frame->value.data, blob, blobLength);
    return 1;
}
STATIC OSOCTET *asn1EncodeUPER_MessageFrame( outWSMType * wsm
                                           , MessageFrame * frame
                                           , OSCTXT       * ctxt)
{
  int32_t   len = 0;
  OSOCTET * msg = NULL;

    if(0 != pu_setBuffer(ctxt, wsm->data, sizeof(wsm->data), 0)) {
        return NULL;
    }

    if (asn1PE_MessageFrame(ctxt, frame) != 0) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"asn1EncodeUPER_MessageFrame:: %d Encode error\n", 0);
        rtxErrPrint(ctxt);
        wsm->dataLength = 0;
        return NULL;
    } else {

        msg             = pe_GetMsgPtr(ctxt, &len);
        wsm->dataLength = len;
        return msg;
    }
}

STATIC void spat_process_main(SPAT *spat)
{
    int32_t         retval                  = 0;
    scsSpatInfo     spat_info_block;                /* SPAT Selector */
    scsSpatInfo   * spat_info               = NULL;
    OSOCTET       * spat_enc_msg            = NULL;
    unsigned char   spat_buf[SPAT_BUF_SIZE];
    i2vTimeT        clock;
    uint32_t        optretval               = 0;
    OSCTXT          frameCtxt;
    MessageFrame    frame;
    uint8_t         tmpbuf[MAX_WSM_DATA];
    int32_t         i=0;
    memset(&spat_info_block,0 ,sizeof(scsSpatInfo)); 
    memset(&clock          ,0 ,sizeof(i2vTimeT));
    memset(&spat_buf[0]    ,0 ,sizeof(SPAT_BUF_SIZE));
    memset(&tmpbuf[0]      ,0 ,sizeof(MAX_WSM_DATA));
    memset(&frame          ,0 ,sizeof(MessageFrame));

    if(NULL == shm_ptr) {
        if(0x0 == (my_error_states & SPAT16_SHM_FAIL)) {
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"spat_process_main: SPAT16_SHM_FAIL\n");
            my_error_states |= SPAT16_SHM_FAIL;
        }
        return; 
    }

    spat_info               = NULL;
    spat_enc_msg            = NULL;

    outWSMMsg.radioNum      = spat_cfg.radioNum;
    outWSMMsg.psid          = cfg.secSpatPsid;
    outWSMMsg.channelNumber = spat_cfg.channel_number;
    outWSMMsg.txPwrLevel    = cfg.txPwrLevel;
    spat_info               = &spat_info_block;

    if (spat_cfg.hdrExtra) {
        outWSMMsg.wsmpHeaderExt |= (WAVE_ID_TRANSMIT_POWER_BITMASK | WAVE_ID_CHANNEL_NUMBER_BITMASK | WAVE_ID_DATARATE_BITMASK);
    }

    memset(outWSMMsg.data, 0, sizeof(outWSMMsg.data));

    WSU_SEM_LOCKR(&shm_ptr->scsSpatData.h.ch_lock);
    memcpy(&spat_info_block, &shm_ptr->scsSpatData.spat_info, sizeof(spat_info_block));
    WSU_SEM_UNLOCKR(&shm_ptr->scsSpatData.h.ch_lock);

    /* Keep reading out of SHM incase NOT breaks something else. */
    if (0 == spat_cfg.numEntries) {
        set_my_error_state(SPAT16_CONF_PHASE_ENTRIES_ZERO);
        return;
    }

    spat_enc_msg = NULL;
    if(0 == asn1Fill_SPAT(spat,spat_info)) {
        spat_enc_msg = spatASN1EncodePER(&outWSMMsg, spat, 0);
    }
    if (NULL == spat_enc_msg) {
        Free_SPAT(spat);  /* cleanup; otherwise, memory leak */
        set_my_error_state(SPAT16_ASN1_ENCODE_ERROR);
        return;
    }

    if(outWSMMsg.dataLength < (MAX_WSM_DATA - sizeof(MessageFrame) - 1)) {
        if (rtInitContext(&frameCtxt) != 0) {
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"rtInitContext failed.\n");
            rtxErrPrint(&frameCtxt);
            set_my_error_state(SPAT16_RTINIT_CONTEXT_FAIL);
        } else {
            memcpy(tmpbuf, spat_enc_msg, outWSMMsg.dataLength);
            memset(outWSMMsg.data, 0, outWSMMsg.dataLength);

            if(uperFrame_SPAT(tmpbuf, outWSMMsg.dataLength, &frame,&frameCtxt) != 1) {
                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME," uperFrame_SPAT() failed.\n");
                set_my_error_state(SPAT16_UPER_FRAME_ERROR);
                return;
            }
                         
            if(NULL == (spat_enc_msg = asn1EncodeUPER_MessageFrame(&outWSMMsg, &frame, &frameCtxt))) {
                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME," asn1EncodeUPER_MessageFrame == NULL...exiting\n");
                set_my_error_state(SPAT16_UPER_ENCODE_ERROR);
                return;
            }
        }
    } else {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"spat_process_main:: Sorry, msg encoded greater than WSM max %d. Msg not sent!!\n",outWSMMsg.dataLength);
        outWSMMsg.dataLength = 0;
    }
#if !defined(MY_UNIT_TEST)
#if defined(J2735_2016)
    asn1Free_MessageFrame(&frameCtxt,&frame);
#endif
    rtxMemReset(&frameCtxt);
    rtxErrReset(&frameCtxt);
    rtFreeContext(&frameCtxt);
#endif
    if (outWSMMsg.dataLength > 0) { 
        /* DataLength set inside encode routines */
        memcpy(outWSMMsg.data, spat_enc_msg, outWSMMsg.dataLength); 
    }
    Free_SPAT(spat);

    /* Send WSM */
    if(0 < outWSMMsg.dataLength) {
        /* call this fxn even if security not enabled */
        outWSMMsg.securityFlag = i2vUtilCertOrDigest(shm_ptr, &cfg, AMH_MSG_SPAT_MASK);
        optretval = 0;
        I2V_UTIL_WSM_REQ(&outWSMMsg, &cfg, SPAT_DEF_WMH_TO, &retval, &optretval)
        if ((SPAT_DEF_WMH_TO != 0) && (retval == I2V_RETURN_OK) && (0 != optretval) ){
            #if defined(EXTRA_DEBUG)
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Send WSM optret failed: ret=%d, optret=%u.\n",retval,optretval);
            #endif
            set_my_error_state(SPAT16_WSM_REQ_RESP_FAILED);
            spat_tx_err_count++;
        }
        if ((retval != I2V_RETURN_OK) && (retval != I2V_RETURN_NO_ACTION)){
            #if defined(EXTRA_DEBUG)
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME," send WSM failed (%d %x)\n", retval, optretval);
            #endif
            set_my_error_state(SPAT16_WSM_REQ_FAILED);
            spat_tx_err_count++;
        } else {
            for(i=0;i<RSU_PSID_TRACKED_STATS_MAX;i++){
                if((cfg.secSpatPsid == messageStats[i].psid) && (SNMP_ROW_ACTIVE == messageStats[i].rowStatus)) {
                    messageStats[i].count++;
                }
            }
            if (spatForward) {
                // Send the SPaT message to fwdmsg process
                pthread_mutex_lock(&fwdmsg_send_lock);
                memset(&spatFwdmsgData,0,sizeof(fwdmsgData));
                spatFwdmsgData.fwdmsgType = SPAT16;
                memcpy(spatFwdmsgData.fwdmsgData,outWSMMsg.data,outWSMMsg.dataLength);
                spatFwdmsgData.fwdmsgDataLen = outWSMMsg.dataLength;

                memcpy(sendbuf,&spatFwdmsgData,sizeof(spatFwdmsgData));

                if ( -1 == mq_send(fwdmsgfd, sendbuf,sizeof(spatFwdmsgData),DEFAULT_MQ_PRIORITY))
                {
                    set_my_error_state(SPAT16_FWDMSG_MQ_SEND_ERROR);
                }
                pthread_mutex_unlock(&fwdmsg_send_lock);
            }
            spat_tx_count++;          
        }
    }
}

/* Reads tscbm.conf into our spat_cfg.
 * If parsing tscbm.conf fails or returns zero entries, spat16 will still run doing nothing.
 * If spat16 were to exit on config failure, i2v_app will think we’ve crashed and will shut down the rest of the system.
 */
void parse_tscbm()
{
  char_t value_vo, value_er;
  int32_t value_src_int, value_dest_int;

    /* Read and compare */
    if (!tsmr_open(TSCBM_PATH_AND_NAME)) {
        spat_cfg.numEntries = 0;
        /* Report error state but don't return error.*/
        set_my_error_state(SPAT16_TSCBM_FOPEN_FAIL);
    } else {
        for(spat_cfg.numEntries = 0;(spat_cfg.numEntries < SPAT_MAX_APPROACHES) && (tsmr_readnext(&value_vo, &value_src_int, &value_dest_int, &value_er) > 0);spat_cfg.numEntries++){ 
            spat_cfg.phaseType[spat_cfg.numEntries] = value_vo;
            spat_cfg.phaseNumber[spat_cfg.numEntries] = value_src_int;
            spat_cfg.signalGroupID[spat_cfg.numEntries] = value_dest_int;
            if (value_er == TSMAP_PERMITTED) {
                spat_cfg.channelGreenType[spat_cfg.numEntries] = MPS_PERMISSIVE_MOVEMENT_ALLOWED;
            } else {
                spat_cfg.channelGreenType[spat_cfg.numEntries] = MPS_PROTECTED_MOVEMENT_ALLOWED;
            }
        }
        tsmr_close();
    }
}
STATIC uint32_t spatSetConfDefault(char_t * tag, void * itemToUpdate)
{
  uint32_t ret = I2V_RETURN_OK; /* Success till proven fail. */

  if((NULL == tag) || (NULL == itemToUpdate)) {
      /* Nothing to do. */
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"spatSetConfDefault: NULL input.\n");
      set_my_error_state(SPAT16_NULL_INPUT);
      ret = I2V_RETURN_NULL_PTR;
  } else {
      if(0 == strcmp("RadioCfg",tag)) {
          *(uint8_t *)itemToUpdate = I2V_RADIO_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%u)\n",tag,*(uint8_t *)itemToUpdate);
      } else if(0 == strcmp("WSAPriority",tag)) {
          *(uint8_t *)itemToUpdate = SPAT_WSA_PRIORITY_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%u)\n",tag,*(uint8_t *)itemToUpdate);
      } else if(0 == strcmp("SPATTOMPSID",tag)) {
          *(uint32_t *)itemToUpdate = I2V_SPAT_PSID_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%u)\n",tag,*(uint32_t *)itemToUpdate);
      } else if(0 == strcmp("SPATChannelNumber",tag)) {
          *(uint8_t *)itemToUpdate = I2V_RADIO_CHANNEL_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%u)\n",tag,*(uint8_t *)itemToUpdate);
      } else if(0 == strcmp("WSMExtension",tag)) {
          *(bool_t *)itemToUpdate = SPAT_WSM_EXTENSION_ENABLE_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%u)\n",tag,*(bool_t *)itemToUpdate);
      } else if(0 == strcmp("SPATInteroperabilityMode",tag)) {
          *(uint8_t *)itemToUpdate = SPAT_INTEROP_MODE_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%u)\n",tag,*(uint8_t *)itemToUpdate);
      } else if(0 == strcmp("IntersectionID",tag)) {
          *(uint32_t *)itemToUpdate = SPAT_INT_ID_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%u)\n",tag,*(uint32_t *)itemToUpdate);
      } else if(0 == strcmp("SPATChannelNumber",tag)) {
          *(uint8_t *)itemToUpdate = I2V_RADIO_CHANNEL_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%u)\n",tag,*(uint8_t *)itemToUpdate);
      } else if(0 == strcmp("BSMRxEnable",tag)) {
          *(bool_t *)itemToUpdate = BSM_RX_ENABLE_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%u)\n",tag,*(bool_t *)itemToUpdate);
      } else if(0 == strcmp("BSMUnsecurePSIDPERU",tag)) {
          *(uint32_t *)itemToUpdate = I2V_BSM_PSID_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%u)\n",tag,*(uint32_t *)itemToUpdate);
      } else {
          /* Nothing to do. */
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"spatSetConfDefault:(%s) item is not known, ignoring.\n",tag);
          set_my_error_state(SPAT16_HEAL_FAIL);
          ret = I2V_RETURN_UNKNOWN;
      }
  }
  return ret;
}
/* spat_update_cfg(): Reads spat.conf into spat_cfg
 * Returns TRUE on success, FALSE on failures
 */
STATIC bool_t spat_update_cfg(void)
{
  cfgItemsTypeT cfgItems[] = {
      {"RadioCfg",                 (void *)i2vUtilUpdateUint8Value,  &spat_cfg.radioNum,       NULL,(ITEM_VALID|UNINIT)},
      {"WSAPriority",              (void *)i2vUtilUpdateUint8Value,  &spat_cfg.wsaPriority,    NULL,(ITEM_VALID|UNINIT)},
      {"SPATTOMPSID",              (void *)i2vUtilUpdatePsidValue,   &spat_cfg.psid,           NULL,(ITEM_VALID|UNINIT)},
      {"SPATChannelNumber",        (void *)i2vUtilUpdateUint8Value,  &spat_cfg.channel_number, NULL,(ITEM_VALID|UNINIT)},
      {"WSMExtension",             (void *)i2vUtilUpdateWBOOLValue,  &spat_cfg.hdrExtra,       NULL,(ITEM_VALID|UNINIT)},
      {"SPATInteroperabilityMode", (void *)i2vUtilUpdateUint8Value,  &spat_cfg.interopMode,    NULL,(ITEM_VALID|UNINIT)},
      /* SPAT and BSM share the same IP+PORT for FWD messages. */
      /* RSU does not decode MAP uper. SPAT msg requires MAP IntersectionID to work */
      {"IntersectionID",           (void *)i2vUtilUpdateUint32Value, &spat_cfg.IntersectionID, NULL,(ITEM_VALID|UNINIT)},
      {"BSMRxEnable",              (void *)i2vUtilUpdateWBOOLValue,  &spat_cfg.bsmEnable,     NULL,(ITEM_VALID| UNINIT)},
      {"BSMUnsecurePSIDPERU",      (void *)i2vUtilUpdatePsidValue,   &spat_cfg.bsmPsidPeruUnsecure,NULL,(ITEM_VALID| UNINIT)},
  };
  FILE *f;
  char_t fileloc[I2V_CFG_MAX_STR_LEN + I2V_CFG_MAX_STR_LEN];  /* these already have space for null term string */
  uint32_t i;
  uint32_t retVal = I2V_RETURN_OK;

    if(NULL == shm_ptr) {
        if(0x0 == (my_error_states & SPAT16_SHM_FAIL)) {
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"spat_update_cfg:SPAT16_SHM_FAIL\n");
            my_error_states |= SPAT16_SHM_FAIL;
        }
        return WFALSE; 
    }
  memset(&spat_cfg,0x0,sizeof(spat_cfg));
  memset(&fwdmsgCfg,0x0,sizeof(fwdmsgCfgItemsT));
  /* Wait on I2V SHM to get I2V CFG.*/
  for(i=0; mainloop && (i < MAX_I2V_SHM_WAIT_ITERATIONS); i++){
      WSU_SEM_LOCKR(&shm_ptr->cfgData.h.ch_lock);
      if(WTRUE == shm_ptr->cfgData.h.ch_data_valid) {
          memcpy(&cfg, &shm_ptr->cfgData, sizeof(cfgItemsT));  /* save config for later use */
          WSU_SEM_UNLOCKR(&shm_ptr->cfgData.h.ch_lock);
          break;
      }
      WSU_SEM_UNLOCKR(&shm_ptr->cfgData.h.ch_lock);
      usleep(MAX_i2V_SHM_WAIT_USEC);
  }

  /* At this point shared memory should be ready. Read fwdmsgConfig for getting fwdmsgMask */
  /* Wait on I2V SHM to get FWDMSG CFG.*/
  for(i=0; mainloop && (i < MAX_I2V_SHM_WAIT_ITERATIONS); i++){
      WSU_SEM_LOCKR(&shm_ptr->fwdmsgCfgData.h.ch_lock);

      if(WTRUE == shm_ptr->fwdmsgCfgData.h.ch_data_valid) {
          /* Copy fwdmsgCfg to local structure */
          memcpy(&fwdmsgCfg.fwdmsgAddr, &shm_ptr->fwdmsgCfgData.fwdmsgAddr, sizeof(fwdmsgCfg.fwdmsgAddr));  
          WSU_SEM_UNLOCKR(&shm_ptr->fwdmsgCfgData.h.ch_lock);
          break;
      }

      WSU_SEM_UNLOCKR(&shm_ptr->fwdmsgCfgData.h.ch_lock);
      usleep(MAX_i2V_SHM_WAIT_USEC);
  }

  if((WFALSE == mainloop) || (MAX_I2V_SHM_WAIT_ITERATIONS <= i)) {
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"spat_update_cfg: Failed to get SHM from I2V.i=%d,mainloop=%d\n",i,mainloop);
      return WFALSE;
  }

  spatForward = bsmForward = bsmpsmForwardF = 0x00;
  /* Check from fwdmsgCfg if SPaT/BSM/PSMBSM need to forwarded */
  for (i = 0; i < MAX_FWD_MSG_IP; i++)  {
    if (fwdmsgCfg.fwdmsgAddr[i].fwdmsgEnable)   {

        /* BSM */
        if ((fwdmsgCfg.fwdmsgAddr[i].fwdmsgMask) & 0x01)
            bsmForward = WTRUE;

        /* SPaT */
        if ((fwdmsgCfg.fwdmsgAddr[i].fwdmsgMask) & 0x08)
            spatForward = WTRUE;

        /* PSM */
        if ((fwdmsgCfg.fwdmsgAddr[i].fwdmsgMask) & 0x04)
            bsmpsmForwardF = WTRUE;
    }
  }

  if((WFALSE == mainloop) || (MAX_I2V_SHM_WAIT_ITERATIONS <= i)) {
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"spat_update_cfg: Failed to get SHM from I2V.i=%d,mainloop=%d\n",i,mainloop);
      return WFALSE;
  }


  /* Parse spat.conf */
  strncpy(fileloc, cfg.config_directory, sizeof(fileloc));
  strncat(fileloc, confFileName, sizeof(fileloc)-1);
  f = fopen(fileloc, "r");

  /* If not in a unit test, then failing to open SPAT_CFG_FILE is fatal */
  if (f == NULL) {
      set_my_error_state(SPAT16_OPEN_CONF_FAIL);
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"spat_update_cfg: SPAT16_OPEN_CONF_FAIL.\n");
      return WFALSE; 
  }

  if(I2V_RETURN_OK != (retVal = i2vUtilParseConfFile(f, cfgItems, NUMITEMS(cfgItems), WFALSE, NULL))) {
    I2V_DBG_LOG(LEVEL_CRIT,MY_NAME,"i2vUtilParseConfFile: not happy i2vRet=%u.\n",retVal);      
    set_my_error_state(SPAT16_LOAD_CONF_FAIL);
  }
  if(NULL != f) fclose(f);

  for (i = 0; i < NUMITEMS(cfgItems); i++) {
      if (   ((ITEM_VALID|BAD_VAL) == cfgItems[i].state) 
          || ((ITEM_VALID|UNINIT) == cfgItems[i].state)) {
          I2V_DBG_LOG(LEVEL_CRIT,MY_NAME,"config override on (%s): Using default.\n", cfgItems[i].tag);
          if(I2V_RETURN_OK == (retVal = spatSetConfDefault(cfgItems[i].tag, cfgItems[i].itemToUpdate))) {
              cfgItems[i].state = (ITEM_VALID|INIT);
              set_my_error_state(SPAT16_HEAL_CFGITEM); /* Note for user. */
          } else {
              break; /* Heal has failed. FATAL. */
          }
      }
  }

  if(I2V_RETURN_OK == retVal){
      parse_tscbm();

      if (cfg.bcastLockStep) {  /* all rse apps broadcast on same radio and channel */
          spat_cfg.channel_number = cfg.uchannel;
          spat_cfg.radioNum = cfg.uradioNum;
      }

#if defined(MY_UNIT_TEST)
      /* Here is our chance to alter conf shm.*/
      unit_test_spat_config_twiddle();
#endif

      WSU_SEM_LOCKW(&shm_ptr->spatCfgData.h.ch_lock);
      memcpy(&spat_cfg.h, &shm_ptr->spatCfgData.h, sizeof(wsu_shm_hdr_t)); /* preserve header */
      memcpy(&shm_ptr->spatCfgData, &spat_cfg, sizeof(spatCfgItemsT));
      shm_ptr->spatCfgData.h.ch_data_valid = WTRUE;
      WSU_SEM_UNLOCKW(&shm_ptr->spatCfgData.h.ch_lock);
  }

  return (I2V_RETURN_OK == retVal) ? WTRUE : WFALSE;
}

STATIC int32_t spat_init_radio(void)
{
  int32_t   retVal    = RS_SUCCESS;
#if !defined(MY_UNIT_TEST)
  uint32_t  optRet    = (uint32_t)RS_SUCCESS;
  PSIDType bsmpsid   = 0x20;
#endif
    spatPService.action = ADD;         
    spatPService.wsaSecurity = UNSECURED_WSA,
    spatPService.psid = cfg.secSpatPsid;
    spatPService.lengthPsc = 0,
    spatPService.servicePriority = (cfg.bcastLockStep) ? cfg.uwsaPriority : spat_cfg.wsaPriority;
    spatPService.channelNumber = spat_cfg.channel_number;
    spatPService.channelAccess = 2,       
    spatPService.repeatRate = I2V_DEFAULT_WSA_REPEAT_RATE;
    spatPService.ipService = 0;
    spatPService.IPv6ServicePort = 23000;

    spatPService.rcpiThreshold = 0;
    spatPService.wsaCountThreshold = 0;
    spatPService.wsaCountThresholdInterval = 0;
    spatPService.wsaHeaderExtension = 0;

    spatUService.radioNum        = spat_cfg.radioNum;
    spatUService.channelNumber   = spat_cfg.channel_number;
    spatUService.psid            = cfg.secSpatPsid;
    spatUService.servicePriority = (cfg.bcastLockStep) ? cfg.uwsaPriority : spat_cfg.wsaPriority;

    if (spat_cfg.bsmEnable) {
        appCredential.cbReceiveWSM = bsmRxHandler;
    }
    if (cfg.security) {
        spatPService.wsaSecurity = SECURED_WSA;
        spatUService.wsaSecurity = SECURED_WSA;
        outWSMMsg.security     = SECURITY_SIGNED;
        outWSMMsg.psid         = cfg.secSpatPsid;
        outWSMMsg.securityFlag = WTRUE;
    } else {
        spatPService.wsaSecurity = UNSECURED_WSA;
        spatUService.wsaSecurity = UNSECURED_WSA;
        outWSMMsg.security       = SECURITY_UNSECURED;
        outWSMMsg.psid           = cfg.secSpatPsid;
        outWSMMsg.securityFlag   = WFALSE;   
    }

    if(1 == cfg.RadioType) {
        spatPService.radioType = RT_CV2X;
        spatUService.radioType = RT_CV2X;
        outWSMMsg.radioType    = RT_CV2X;
        service.radioType      = RT_CV2X;
    }  else {
        spatPService.radioType = RT_DSRC;
        spatUService.radioType = RT_DSRC;
        outWSMMsg.radioType    = RT_DSRC;
        service.radioType      = RT_DSRC;
    }
    /* Continous seems to be what works.*/
    spatUService.immediateAccess = 1;
    spatUService.extendedAccess  = 65535; 

#ifdef HSM 
	int i;

    /* Check if Ssp and security are enabled */
    if (cfg.spatSspEnable && cfg.security)  {

        /* Set the Ssp length */
        outWSMMsg.sspLen = strlen((const char_t *)cfg.spatSsp)/2;   

        /* Convert the string into byte array */
        for (i = 0; i < outWSMMsg.sspLen; i++) {
            outWSMMsg.ssp[i] = (fromHexDigit(cfg.spatSsp[(i * 2) + 0]) << 4) +
                                (fromHexDigit(cfg.spatSsp[(i * 2) + 1]) << 0);
        }

        if (cfg.spatBitmappedSspEnable) {
            outWSMMsg.isBitmappedSsp = cfg.spatBitmappedSspEnable;

            /* Convert the string into byte array */
            for (i = 0; i < outWSMMsg.sspLen; i++) {
                outWSMMsg.sspMask[i] = (fromHexDigit(cfg.spatSspMask[(i * 2) + 0]) << 4) +
                                    (fromHexDigit(cfg.spatSspMask[(i * 2) + 1]) << 0);
            }
        }
    }
#endif

#if !defined(MY_UNIT_TEST)
        I2V_UTIL_USER_SVC_REQ(&spatUService, &cfg, WMH_SVC_TIMEOUT, &retVal, &optRet);
        if (retVal != I2V_RETURN_OK) {
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"failed to register spat service with iwmh: (0x%x)\n", retVal);
            return SPAT16_INIT_RADIO_FAIL;
        }
#endif

    /* see comments in i2v_riscapture.h */
    memcpy(&svctrack.u.userData, &spatUService, sizeof(UserServiceType));
    svctrack.isUserService = WTRUE;
  
    if (spat_cfg.bsmEnable) {
        spatUService.psid = cfg.bsmPsidPerU;
        if (!cfg.iwmhAppEnable) {
            if (RS_SUCCESS != (retVal = wsuRisUserServiceRequest(&spatUService))) {
                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME," wsuRisUserServiceRequest failed (%d)\n", retVal);
                return SPAT16_INIT_RADIO_FAIL;
            }
        } else {
#if !defined(MY_UNIT_TEST)
            I2V_UTIL_USER_SVC_REQ(&spatUService, &cfg, WMH_SVC_TIMEOUT, &retVal, &optRet);
            if (retVal != I2V_RETURN_OK) {
                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME," wsuRisUserServiceRequest failed: %x\n", retVal);
                return SPAT16_INIT_RADIO_FAIL;
            }
#endif
        }
        /* see comments in i2v_riscapture.h */
        memcpy(&bsmtrack.u.userData, &spatUService, sizeof(UserServiceType));
        bsmtrack.isUserService = WTRUE;
    }

    if (spat_cfg.bsmEnable) {

        service.action = ADD;
        service.psid   = cfg.bsmPsidPerU;

        if (!cfg.iwmhAppEnable) {
            if (RS_SUCCESS != (retVal = wsuRisWsmServiceRequest(&service))) {
                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME," terminateRadioComm failed (%d)\n", retVal);
                return SPAT16_INIT_RADIO_FAIL;
            }
            memcpy(&bsmtrack.wsmData, &service, sizeof(WsmServiceType));
        } else {
#if !defined(MY_UNIT_TEST)
            I2V_UTIL_REGISTER_WSM_CB(&cfg, WMH_SVC_TIMEOUT, &retVal, spatUService.radioNum, bsmpsid, &mainloop, bsmRxHandler, NULL)
            if (retVal != I2V_RETURN_OK) {
                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"failed to register rx callback with iwmh: %x\n", retVal);
                return SPAT16_INIT_RADIO_FAIL;
            }
#endif
        }
    }

    return SPAT16_AOK;
}
//TODO: use argc argv to pass in config, mainloop
int MAIN(void) 
{ 
  int32_t ret = SPAT16_AOK;
  struct sigaction sa;
  static SPAT spat_tx_msg;
  uint32_t rolling_counter = 0;

  Init_SPAT_Statics(); /* Handle soft resets */

  /* Enable serial debug with I2V_DBG_LOG until i2v.conf::globalDebugFlag says otherwise. */
  i2vUtilEnableDebug(MY_NAME);
  /* LOGMGR assumed up by now. Could retry if fails. */
  if(0 != i2vUtilEnableSyslog(MY_ERR_LEVEL, MY_NAME)){
      I2V_DBG_LOG(LEVEL_WARN,MY_NAME,"Failed to open syslog. Only serial output available,\n");
  }

  shm_ptr = WSU_SHARE_INIT(sizeof(i2vShmMasterT), I2V_SHM_PATH);
  if (shm_ptr == NULL) {
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME," critical error: Main Shared memory init failed\n");
      ret = SPAT16_SHM_FAIL;
      goto out;
  }
  /* Setup the metrics part of the shm */
  memset(&shm_ptr->scsSpatData.metrics, 0, sizeof(scsMetricsT));
  shm_ptr->scsSpatData.metrics.min_num_groups = SPAT_MAX_APPROACHES + 1;    // A max number to be replaced as smaller numbers come in for min

  /* catch SIGINT/SIGTERM */
  memset(&sa, 0, sizeof(sa));
  sa.sa_handler = spat_sighandler;
  sigaction(SIGINT, &sa, NULL);
  sigaction(SIGTERM, &sa, NULL);

  // Read config files into our shm for later
  if (spat_update_cfg() != WTRUE) {
      ret = SPAT16_LOAD_CONF_FAIL;
      goto out_shm;
  }

  if (SPAT16_AOK != (ret = spat_init_radio())) {
      goto out_radio;
  }

  /* We only support j2735 2016 UPER encoding */
  asn1Init_SPAT(&spat_tx_msg);
  if (rtInitContext(&spat_ctxt) != 0) {
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Could not initialize ASN1 SPAT16 context\n");
      rtxErrPrint(&spat_ctxt);
      ret = SPAT16_INIT_ASN1_CONTEXT_FAIL;
      set_my_error_state(ret);  
      goto out_radio; /* Primary function is SPAT. If this fails then fatal. */
  }

  /* Open and manage FWD socket: Could be spat, bsm, psm or map(TC legacy). */
  if((spatForward) || (bsmForward) || (bsmpsmForwardF)) {
    
    /* When Forward message is enabled, open the message queue to fwdmsg process */
    if ((fwdmsgfd = mq_open(I2V_FWDMSG_QUEUE, O_RDWR|O_NONBLOCK)) == -1) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME," mq() failed to open: errno = %d(%s)\n",errno,strerror(errno)); 
        //ret = SPAT16_OPEN_MQ_FAIL;  NO WAY to define because maximum values are only 64???????
        ret = SPAT16_FWDMSG_MQ_SEND_ERROR;  
        set_my_error_state(SPAT16_FWDMSG_MQ_SEND_ERROR);
        goto out_radio; /* Primary function is SPAT. If this fails then fatal. */
        
    }

    ret = SPAT16_AOK;
  }

#if defined(BSM_PSM_FORWARDING)
  /* Create BSM/PSM Thread after calling bsmRxInit as asn.1 context is initialized in bsmRxInit */
  if ((bsmpsmForwardF) && (bsmForward)) {
      if (0 == (ret = pthread_create(&bsmpsmForwardThreadID, NULL, bsmpsmForwardThread, NULL))){
          pthread_detach(bsmpsmForwardThreadID);
       } else {
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"bsmpsmForwardThreadID. Not Fatal: ret=%d, errno=%d.\n",ret, errno);
          ret = SPAT16_CREATE_PSM_FWD_FAIL;
          set_my_error_state(ret); 
      }
  }
#endif

  /* Start NTCIP-1218 Services. */
  if (0 == (ret = pthread_create(&CommRangeThreadID, NULL, CommRangeThread, NULL))) {
      pthread_detach(CommRangeThreadID);
  } else {
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"CommRangeThreadID: Failed. Not Fatal: errno=%d.\n",ret, errno);
      set_my_error_state(SPAT16_CREATE_FWD_THREAD_FAIL);
  }
  if (0 == (ret = pthread_create(&ReceivedMsgThreadID, NULL, ReceivedMsgThread, NULL))) {
      pthread_detach(ReceivedMsgThreadID);
  } else {
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"ReceivedMsgThreadID: Failed. Not Fatal: errno=%d.\n",ret, errno);
      set_my_error_state(SPAT16_CREATE_FWD_THREAD_FAIL);
  }
  if (0 == (ret = pthread_create(&MessageStatsThreadID, NULL, MessageStatsThread, NULL))) {
      pthread_detach(MessageStatsThreadID);
  } else {
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"MessageStatsThreadID: Failed. Not Fatal: errno=%d.\n",ret, errno);
      set_my_error_state(SPAT16_CREATE_FWD_THREAD_FAIL);
  }
  sleep(1); /* take a break and let threads get started. */
  /* ret will not change once inside mainloop so check just once. */
  if(SPAT16_AOK == ret) {
      while (mainloop) {
          if(0 == (rolling_counter % OUTPUT_MODULUS)){
              I2V_DBG_LOG(LEVEL_INFO,MY_NAME,"(0x%lx) SPAT tx,E(%u,%u)\n",my_error_states, spat_tx_count, spat_tx_err_count);
          }
          if(0 == (rolling_counter % 600)){
              dump_spat_report();
          }
          rolling_counter++;
          /* Blocking at gate in mainloop means we have no way to communicate with I2V. */
          if(WTRUE == wsu_wait_at_gate(&shm_ptr->scsSpatData.spatTriggerGate)) {
              ++shm_ptr->scsSpatData.metrics.cnt_spat_process_main_calls;
              spat_process_main(&spat_tx_msg);
          } else {
              mainloop = WFALSE;
              break;
          }
          #if defined(MY_UNIT_TEST)
          mainloop=WFALSE;
          sleep(1); 
          break;
          #endif
      } /* while */
  } /* SPAT16_AOK == ret */

/*
 * Report fails on cleanup but preserve the original ret value that got us here.
 */
out_radio:
  rtFreeContext(&spat_ctxt);
  /* see comments in i2v_riscapture.h */
  if (svctrack.isUserService) {
      svctrack.u.userData.action = DELETE;
      I2V_UTIL_USER_SVC_REQ(&svctrack.u.userData, &cfg, WMH_SVC_TIMEOUT, &svctrack.sdummy, &svctrack.udummy)
  } else {
      svctrack.u.providerData.action = DELETE;
      I2V_UTIL_PROVIDER_SVC_REQ(&svctrack.u.providerData, &cfg, WMH_SVC_TIMEOUT, &svctrack.sdummy, &svctrack.udummy)
  }
  if (svctrack.sdummy != RS_SUCCESS) {
#if defined(EXTRA_DEBUG)
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"deleting radio service failed (%d); radio may not restart correctly without reboot\n", svctrack.sdummy);
#endif
  }
  if (!cfg.iwmhAppEnable) {
      svctrack.wsmData.action = DELETE;
      if (wsuRisWsmServiceRequest(&svctrack.wsmData) != RS_SUCCESS) {
#if defined(EXTRA_DEBUG)
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"wsmServiceRequest cleanup failed, radio may not restart correctly without reboot\n");
#endif
      }
      if (spat_cfg.bsmEnable) {
          bsmtrack.wsmData.action = DELETE;
          if (wsuRisWsmServiceRequest(&bsmtrack.wsmData) != RS_SUCCESS) {
#if defined(EXTRA_DEBUG)
              I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"wsmServiceRequest cleanup failed, radio may not restart correctly without reboot\n");
#endif
          }
          i2vUtilRxCBCleanup();
       }
       if (wsuRisTerminateRadioComm() != RS_SUCCESS) {
#if defined(EXTRA_DEBUG)
           I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"terminateRadioComm failed.\n");
#endif
       }
   }
out_shm:
#if defined(EXTRA_DEBUG)
  /* Dump final state to syslog. */
  I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"EXIT: error states: ret(%d) spat(0x%llx)\n",ret,my_error_states);
#endif
  /* Stop I2V_DBG_LOG output. Last chance to output to syslog. */
  i2vUtilDisableSyslog();
  i2vUtilDisableDebug();

  if(NULL != shm_ptr) {
      wsu_share_kill(shm_ptr, sizeof(shm_ptr));
      shm_ptr = NULL;
  }

out:
  return abs(ret);
} /* END OF MAIN */
