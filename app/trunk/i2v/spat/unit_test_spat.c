
/*
** Unit tests for spat16.c. For coverage using gcov unless test states otherwise.
*/

/**************************************************************************************************
* Includes
***************************************************************************************************/
#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>
#include <byteswap.h>
#include <unistd.h>
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
#include "ProbeVehicleData.h"
#include "BasicSafetyMessage.h"
#include "PersonalSafetyMessage.h" /* 1 or 2? */
#include "SPAT.h"
#endif
#include "i2v_riscapture.h"
#include "stubs.h"
#include "libtsmapreader.h"     // for TSMAP_VEHICLE
#include "rs.h"
#include "ris_struct.h"
#include "spat.h"
#include "ntcip-1218.h"
#include "conf_table.h"
/**************************************************************************************************
* Defines
***************************************************************************************************/
/* mandatory logging defines */
#define MY_ERR_LEVEL   LEVEL_INFO    /* from i2v_util.h */
#define MY_NAME        "spat_unit"

/* Test configs used. */
#define DEFAULT_CONFIG  "./"
#define NO_CONFIG       "../stubs/"
#define HOST_PC_IP      "10.52.11.47"  //Your PC IP here. Intention this is a valid IP.
#define UT_FLAVOR_MYPCIP 1
#define UT_FLAVOR_ENABLEFWD 2

//
// 00 13 52 43 95 00 00 82 90 e6 b0 00 0c 6b a0 80 01 04 34 26 88 a6 88 80 10 21 a1 35 85 37 3c 00 c1 0d 09 eb e9
// f9 a0 08 09 08 4c fd 4c fd 00 50 43 42 68 8a 68 88 03 02 1a 13 58 53 73 c0 1c 10 d0 9e be 9f 9a 01 00 90 84 cf
// d4 cf d0 09 04 74 26 7e a6 7e 80
//
#define PRINTIT printf("\nret=%d.",ret);
/**************************************************************************************************
* Globals
***************************************************************************************************/
/* spat16.c */
extern bool_t           mainloop; /* Calling main() will set to WTRUE. Otherwise you must reset depending on function you call. */
extern spatCfgItemsT spat_cfg; /* Can't be modified if you call main(). We be overwritten. */
extern int FwdSock;
extern char_t  confFileName[I2V_CFG_MAX_STR_LEN];

static i2vShmMasterT * UnitshmPtr; /* If you are not calling main() then you need to set SHM to this. */

/**************************************************************************************************
* Protos & Externs: Declare to silence warnings.
***************************************************************************************************/
/* spat.c */
extern int my_main(void);
extern void spat_sighandler(int __attribute__((unused)) sig);
extern uint32_t getCurrMinVal(uint16_t * millisecond);
extern STATIC int32_t uperFrame_SPAT(uint8_t * blob, uint32_t blobLength, MessageFrame * frame, OSCTXT * ctxt);
extern OSOCTET *spatASN1EncodePER(outWSMType *wsm, SPAT *spat, uint8_t aligned);
extern int32_t asn1Fill_SPAT(SPAT *spat, scsSpatInfo *spat_info);
extern OSOCTET *asn1EncodeUPER_MessageFrame(outWSMType * wsm, MessageFrame * frame, OSCTXT * ctxt);
extern void spat_process_main(SPAT *spat);
extern bool_t spat_update_cfg();
extern void bsmFwdProcess(inWSMType *wsm);
extern int spat_init_radio(void);
extern void spat16FwdProcess(outWSMType *wsm) ;

/* test.c */
void test_main(void);
int init_suite(void);
int clean_suite(void);

/**************************************************************************************************
* Function Bodies
***************************************************************************************************/
static int unit_test_flavor = 0;
void unit_test_spat_config_twiddle(void)
{

    switch(unit_test_flavor) {
        case UT_FLAVOR_ENABLEFWD:
            spat_cfg.bsmEnable = 1;
            unit_test_flavor = 0;
            break;
        case UT_FLAVOR_MYPCIP:
            unit_test_flavor = 0;
            break;
        default:
            break;
    }
}
#if 0
// Dumps metrics and output_copy for the tester to examine
// Used to dump some data on asn1Fill_SPAT's input
// Outputs if no FILE* given to file /tmp/zout.cunit.txt
// CUnit usage: Call from within test_spat16_processing_mikes_worry_example()
//               and pass &shm_ptr_unit_test and NULL.
void dump_spat16_metrics_and_output_copy(i2vShmMasterT * shm_ptr, FILE * fout)
{
    int i;

    // If not overridden, output to default file in /tmp
    if (!fout) {
        fout = fopen("/tmp/zout.cunit.txt","a");
    }
    fprintf(fout,"intersection_id = %d\n", shm_ptr->scsSpatData.output_copy.intersection_id);
    fprintf(fout,"revision_counter = %d\n", shm_ptr->scsSpatData.output_copy.revision_counter);
    fprintf(fout,"num_groups = %d\n", shm_ptr->scsSpatData.output_copy.num_groups);
    fprintf(fout,"\n");

    // Some asn1Fill_SPAT metrics
    fprintf(fout,"cnt_spat_process_main_calls = %d\n", shm_ptr->scsSpatData.metrics.cnt_spat_process_main_calls);
    fprintf(fout,"cnt_asn1fill_spat_calls = %d\n", shm_ptr->scsSpatData.metrics.cnt_asn1fill_spat_calls);
    fprintf(fout,"cnt_empty_spat_fills = %d\n", shm_ptr->scsSpatData.metrics.cnt_empty_spat_fills);
    fprintf(fout,"cnt_nonempty_spat_fills = %d\n", shm_ptr->scsSpatData.metrics.cnt_nonempty_spat_fills);
    fprintf(fout,"max_num_groups = %d\n", shm_ptr->scsSpatData.metrics.max_num_groups);
    fprintf(fout,"min_num_groups = %d\n", shm_ptr->scsSpatData.metrics.min_num_groups);
    fprintf(fout,"\n");


    /* Dump all of output_copy */
    for (i=0; i<SPAT_MAX_APPROACHES; i++) {
        fprintf(fout,"output_copy[%d] = %d,%d,%d,%d,%d,%d,%d,%d,%d\n", i,
            shm_ptr->scsSpatData.output_copy.tsc_phase_number[i],
            shm_ptr->scsSpatData.output_copy.tsc_phase_type[i],
            shm_ptr->scsSpatData.output_copy.signal_group_id[i],
            shm_ptr->scsSpatData.output_copy.signal_phase[i],
            shm_ptr->scsSpatData.output_copy.channel_green_type[i],
            shm_ptr->scsSpatData.output_copy.prior_event_state[i],
            shm_ptr->scsSpatData.output_copy.final_event_state[i],
            shm_ptr->scsSpatData.output_copy.min_end_time[i],
            shm_ptr->scsSpatData.output_copy.max_end_time[i]);
    }
    fprintf(fout,"\n");
    fclose(fout);
}
#endif

// Test Mike's worry example: one SigPhase mapping into two signal groups
//      vehicle,2,1,permitted
//      vehicle,2,2,protected
// Test INPUT: TSC data with single approach phsnum=2, vehicle only, no overlap
// EXPECTED OUTPUT: Two signal groups
void test_spat16_processing_mikes_worry_example()
{
    SPAT my_spat;
    int i;

    UnitshmPtr->scsSpatData.spat_info.flexSpat.numApproach = SPAT_MAX_APPROACHES; 
    UnitshmPtr->scsSpatData.spatSelector = 1;

    // Reset metrics
    memset(&UnitshmPtr->scsSpatData.metrics, 0, sizeof(UnitshmPtr->scsSpatData.metrics));

    // Blank out all approaches
    for(i=0;i<SPAT_MAX_APPROACHES;i++) {
        UnitshmPtr->scsSpatData.spat_info.flexSpat.spatApproach[i].curSigPhase = SIG_PHASE_UNKNOWN;
    }

    // Populate the one approach we want
    UnitshmPtr->scsSpatData.spat_info.flexSpat.spatApproach[1].approachID = 99;    // Must be nonzero
    UnitshmPtr->scsSpatData.spat_info.flexSpat.spatApproach[1].curSigPhase = SIG_PHASE_GREEN;
    UnitshmPtr->scsSpatData.spat_info.flexSpat.spatApproach[1].timeNextPhase = 123;
    UnitshmPtr->scsSpatData.spat_info.flexSpat.spatApproach[1].secondaryTimeNextPhase = 456;

    memset(&spat_cfg,0x0,sizeof(spat_cfg));

    spat_cfg.IntersectionID = 66;       // Magic test number

    // Populate our spat_cfg with our setup: v,2,1,perm  v,2,2,pro
    spat_cfg.numEntries = 2;
        // v,2,1,perm
    spat_cfg.phaseType[0] = TSMAP_VEHICLE;
    spat_cfg.phaseNumber[0] = 2;
    spat_cfg.signalGroupID[0] = 1;
    spat_cfg.channelGreenType[0] = MPS_PERMISSIVE_MOVEMENT_ALLOWED;
        // v,2,2,pro
    spat_cfg.phaseType[1] = TSMAP_VEHICLE;
    spat_cfg.phaseNumber[1] = 2;
    spat_cfg.signalGroupID[1] = 2;
    spat_cfg.channelGreenType[1] = MPS_PROTECTED_MOVEMENT_ALLOWED;

    // Call spat16 to process our input
    asn1Fill_SPAT(&my_spat,
                  &UnitshmPtr->scsSpatData.spat_info);
    // Verify we got two output rows
    CU_ASSERT_EQUAL(UnitshmPtr->scsSpatData.output_copy.num_groups, 2);

    // Verify signal group 1 - we want MPS_PERMISSIVE
    CU_ASSERT_EQUAL(UnitshmPtr->scsSpatData.output_copy.final_event_state[0], MPS_PERMISSIVE_MOVEMENT_ALLOWED);

    // Verify signal group 2 - we want MPS_PROTECTED
    CU_ASSERT_EQUAL(UnitshmPtr->scsSpatData.output_copy.final_event_state[1], MPS_PROTECTED_MOVEMENT_ALLOWED);

    //dump_spat16_metrics_and_output_copy(&shm_ptr_unit_test, NULL);
}


// Test vehicle phase -> signal group
//      vehicle,2,1,protected
// Test INPUT: TSC data with single approach phsnum= veh 2
// EXPECTED OUTPUT: signal group 1 with appropriate state and time-to-change
void test_spat16_processing_vehicle()
{
    SPAT my_spat;
    int i;

    UnitshmPtr->scsSpatData.spat_info.flexSpat.numApproach = SPAT_MAX_APPROACHES; 
    UnitshmPtr->scsSpatData.spatSelector = 1;

    // Reset metrics
    memset(&UnitshmPtr->scsSpatData.metrics, 0, sizeof(UnitshmPtr->scsSpatData.metrics));

    // Blank out all approaches
    for(i=0;i<SPAT_MAX_APPROACHES;i++) {
        UnitshmPtr->scsSpatData.spat_info.flexSpat.spatApproach[i].approachID = 0;
        UnitshmPtr->scsSpatData.spat_info.flexSpat.spatApproach[i].curSigPhase = SIG_PHASE_UNKNOWN;
        UnitshmPtr->scsSpatData.spat_info.flexSpat.spatApproach[i].timeNextPhase = 0;
        UnitshmPtr->scsSpatData.spat_info.flexSpat.spatApproach[i].secondaryTimeNextPhase = 0;
    }

    // Populate vehicle phase 2
    UnitshmPtr->scsSpatData.spat_info.flexSpat.spatApproach[2-1].approachID = 88;    // Must be nonzero
    UnitshmPtr->scsSpatData.spat_info.flexSpat.spatApproach[2-1].curSigPhase = SIG_PHASE_RED;
    UnitshmPtr->scsSpatData.spat_info.flexSpat.spatApproach[2-1].timeNextPhase = 111;
    UnitshmPtr->scsSpatData.spat_info.flexSpat.spatApproach[2-1].secondaryTimeNextPhase = 222;

    memset(&spat_cfg,0x0,sizeof(spat_cfg));

    spat_cfg.IntersectionID = 66;       // Magic test number

    // Populate our spat_cfg with our setup: v,2,1,pro
    spat_cfg.numEntries = 1;
    spat_cfg.phaseType[0] = TSMAP_VEHICLE;
    spat_cfg.phaseNumber[0] = 2;
    spat_cfg.signalGroupID[0] = 1;
    spat_cfg.channelGreenType[0] = MPS_PROTECTED_MOVEMENT_ALLOWED;

    spat_cfg.interopMode =  INTEROP_UTC_SPAT | INTEROP_MIN_AND_MAX_SPAT | INTEROP_SIMPLE_SPAT;

    // Call spat16 to process our input
    asn1Fill_SPAT(&my_spat,
                  &UnitshmPtr->scsSpatData.spat_info);

    // Verify we got 1 output rows
    CU_ASSERT_EQUAL(UnitshmPtr->scsSpatData.output_copy.num_groups, 1);

    // Verify SIG_PHASE_RED => MPS_STOP_AND_REMAIN
    CU_ASSERT_EQUAL(UnitshmPtr->scsSpatData.output_copy.final_event_state[0], MPS_STOP_AND_REMAIN);

    // Verify signal group ID = 1
    CU_ASSERT_EQUAL(UnitshmPtr->scsSpatData.output_copy.signal_group_id[0], 1);

    // Verify time-to-change
      // problem .. TTC is converted from relative to absolute time
        // soln .. compare relative times in CU_ASSERTs
    CU_ASSERT_EQUAL((UnitshmPtr->scsSpatData.output_copy.min_end_time[0]+111)%36000, (UnitshmPtr->scsSpatData.output_copy.max_end_time[0])%36000);
    
    //dump_spat16_metrics_and_output_copy(&shm_ptr_unit_test, NULL);
}


// Test overlap phase -> signal group
//      overlap,2,1,protected
// Test INPUT: TSC data with single approach phsnum= ovlp 2
// EXPECTED OUTPUT: signal group 1 with appropriate state and time-to-change
void test_spat16_processing_overlap()
{
    SPAT my_spat;
    int i;

    UnitshmPtr->scsSpatData.spat_info.flexSpat.numApproach = SPAT_MAX_APPROACHES; 
    UnitshmPtr->scsSpatData.spatSelector = 1;

    // Reset metrics
    memset(&UnitshmPtr->scsSpatData.metrics, 0, sizeof(UnitshmPtr->scsSpatData.metrics));

    // Blank out all approaches
    for(i=0;i<SPAT_MAX_APPROACHES;i++) {
        UnitshmPtr->scsSpatData.spat_info.flexSpat.spatApproach[i].approachID = 0;
        UnitshmPtr->scsSpatData.spat_info.flexSpat.spatApproach[i].curSigPhase = SIG_PHASE_UNKNOWN;
        UnitshmPtr->scsSpatData.spat_info.flexSpat.spatApproach[i].timeNextPhase = 0;
        UnitshmPtr->scsSpatData.spat_info.flexSpat.spatApproach[i].secondaryTimeNextPhase = 0;
    }

    // Populate overlap phase 2 (+16 for the overlaps)
    UnitshmPtr->scsSpatData.spat_info.flexSpat.spatApproach[2-1+16].approachID = 88;    // Must be nonzero
    UnitshmPtr->scsSpatData.spat_info.flexSpat.spatApproach[2-1+16].curSigPhase = SIG_PHASE_RED;
    UnitshmPtr->scsSpatData.spat_info.flexSpat.spatApproach[2-1+16].timeNextPhase = 111;
    UnitshmPtr->scsSpatData.spat_info.flexSpat.spatApproach[2-1+16].secondaryTimeNextPhase = 311;

    memset(&spat_cfg,0x0,sizeof(spat_cfg));

    spat_cfg.IntersectionID = 66;       // Magic test number

    // Populate our spat_cfg with our setup: o,2,1,pro
    spat_cfg.numEntries = 1;
    spat_cfg.phaseType[0] = TSMAP_OVERLAP;
    spat_cfg.phaseNumber[0] = 2;
    spat_cfg.signalGroupID[0] = 1;
    spat_cfg.channelGreenType[0] = MPS_PROTECTED_MOVEMENT_ALLOWED;

    spat_cfg.interopMode =  INTEROP_UTC_SPAT | INTEROP_MIN_AND_MAX_SPAT | INTEROP_SIMPLE_SPAT;

    // Call spat16 to process our input
    asn1Fill_SPAT(&my_spat,
                  &UnitshmPtr->scsSpatData.spat_info);

    // Verify we got 1 output rows
    CU_ASSERT_EQUAL(UnitshmPtr->scsSpatData.output_copy.num_groups, 1);

    // Verify SIG_PHASE_RED => MPS_STOP_AND_REMAIN
    CU_ASSERT_EQUAL(UnitshmPtr->scsSpatData.output_copy.final_event_state[0], MPS_STOP_AND_REMAIN);

    // Verify signal group ID = 1
    CU_ASSERT_EQUAL(UnitshmPtr->scsSpatData.output_copy.signal_group_id[0], 1);

    // Verify time-to-change
      // problem .. TTC is converted from relative to absolute time
        // soln .. compare relative times in CU_ASSERTs
    CU_ASSERT_EQUAL((UnitshmPtr->scsSpatData.output_copy.min_end_time[0]+200)%36000, (UnitshmPtr->scsSpatData.output_copy.max_end_time[0])%36000);
    
    //dump_spat16_metrics_and_output_copy(&shm_ptr_unit_test, NULL);
}

// Test intersection status (booleans for manual control, preempt, etc.)
// Test INPUT: TSCBM data with the intersectionStatus bits cleared/set
// EXPECTED OUTPUT: all but the coordination bits making it from the TSCBM to J2735
void test_spat16_processing_intersection_status()
{
    SPAT my_spat;
    int i;
    
    UnitshmPtr->scsSpatData.spat_info.flexSpat.numApproach = SPAT_MAX_APPROACHES; 
    UnitshmPtr->scsSpatData.spatSelector = 1;

    // Reset metrics
    memset(&UnitshmPtr->scsSpatData.metrics, 0, sizeof(UnitshmPtr->scsSpatData.metrics));

    // Blank out all approaches
    for(i=0;i<SPAT_MAX_APPROACHES;i++) {
        UnitshmPtr->scsSpatData.spat_info.flexSpat.spatApproach[i].approachID = 0;
        UnitshmPtr->scsSpatData.spat_info.flexSpat.spatApproach[i].curSigPhase = SIG_PHASE_UNKNOWN;
        UnitshmPtr->scsSpatData.spat_info.flexSpat.spatApproach[i].timeNextPhase = 0;
        UnitshmPtr->scsSpatData.spat_info.flexSpat.spatApproach[i].secondaryTimeNextPhase = 0;
    }

    // Populate vehicle phase 2
    UnitshmPtr->scsSpatData.spat_info.flexSpat.spatApproach[2-1].approachID = 88;    // Must be nonzero
    UnitshmPtr->scsSpatData.spat_info.flexSpat.spatApproach[2-1].curSigPhase = SIG_PHASE_RED;
    UnitshmPtr->scsSpatData.spat_info.flexSpat.spatApproach[2-1].timeNextPhase = 111;
    UnitshmPtr->scsSpatData.spat_info.flexSpat.spatApproach[2-1].secondaryTimeNextPhase = 222;

    memset(&spat_cfg,0x0,sizeof(spat_cfg));

    spat_cfg.IntersectionID = 66;       // Magic test number

    // Populate our spat_cfg with our setup: v,2,1,pro
    spat_cfg.numEntries = 1;
    spat_cfg.phaseType[0] = TSMAP_VEHICLE;
    spat_cfg.phaseNumber[0] = 2;
    spat_cfg.signalGroupID[0] = 1;
    spat_cfg.channelGreenType[0] = MPS_PROTECTED_MOVEMENT_ALLOWED;

    spat_cfg.interopMode =  INTEROP_UTC_SPAT | INTEROP_MIN_AND_MAX_SPAT | INTEROP_SIMPLE_SPAT;
    
    // no bits set in TSCBM output (0x00)
    UnitshmPtr->scsSpatData.spat_info.flexSpat.intersectionStatus = 0x00; 
    
    // Call spat16 to process our input
    asn1Fill_SPAT(&my_spat,
                  &UnitshmPtr->scsSpatData.spat_info);
    
    CU_ASSERT_EQUAL(UnitshmPtr->scsSpatData.output_copy.TSCBMIntersectionStatus, 0x00);
    CU_ASSERT_EQUAL(UnitshmPtr->scsSpatData.output_copy.IntersectionStatus, 0x0000);
    
    //all bits set in TSCBM output (0xFF)
    UnitshmPtr->scsSpatData.spat_info.flexSpat.intersectionStatus = 0xFF; 
    
    // Call spat16 to process our input
    asn1Fill_SPAT(&my_spat,
                  &UnitshmPtr->scsSpatData.spat_info);
    
    CU_ASSERT_EQUAL(UnitshmPtr->scsSpatData.output_copy.TSCBMIntersectionStatus, 0xFF);
    CU_ASSERT_EQUAL(UnitshmPtr->scsSpatData.output_copy.IntersectionStatus, 0xF900);  // The 2 coordination bits are dropped between the TSCBM and J2735 and TSCBM moves to higher byte in J2735
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#define BLOB_MAX (100)

void test_main(void)
{
  int32_t       ret = SPAT16_AOK; 
  uint16_t      my_millisecond;
  uint8_t       my_blob[BLOB_MAX];
  MessageFrame  my_frame;
  OSCTXT        my_ctxt;
  outWSMType    my_wsm;
  OSOCTET *     result_pointer=NULL;
  SPAT          spat;
  scsSpatInfo   spat_info_block;

  /* Check for pending tests that did not complete.
   * Set tickle mask and ack mask for test.  
   */
  CU_ASSERT(0x0 == check_stub_pending_signals());
  clear_all_stub_signal();

  if ((UnitshmPtr = wsu_share_init(sizeof(i2vShmMasterT), I2V_SHM_PATH)) == NULL) {
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME," Main shared memory init failed\n");
      return;
  }
  CU_ASSERT(I2V_RETURN_OK == i2v_setupSHM_Default(UnitshmPtr)); /* Should reflect i2v.conf. */
  sleep(1); /* Give SHM a chance to digest. */
  /* Test against bad conf. */
  strcpy(UnitshmPtr->cfgData.config_directory, "../stubs/configs/");
  strcpy(confFileName, "spat16_bad.conf");
  UnitshmPtr->cfgData.globalDebugFlag  = 1; /* If you want to see debug */
  UnitshmPtr->cfgData.h.ch_data_valid  = WTRUE;
  wsu_open_gate(&UnitshmPtr->scsSpatData.spatTriggerGate);
  ret = my_main();
  CU_ASSERT(I2V_RETURN_OK == ret);
  CU_ASSERT(spat_cfg.radioNum == I2V_RADIO_DEFAULT);
  CU_ASSERT(spat_cfg.wsaPriority == SPAT_WSA_PRIORITY_DEFAULT);
  CU_ASSERT(spat_cfg.psid == I2V_SPAT_PSID_DEFAULT);
  CU_ASSERT(spat_cfg.channel_number == I2V_RADIO_CHANNEL_DEFAULT);
  CU_ASSERT(spat_cfg.hdrExtra == SPAT_WSM_EXTENSION_ENABLE_DEFAULT);
  CU_ASSERT(spat_cfg.interopMode == SPAT_INTEROP_MODE_DEFAULT);
  CU_ASSERT(spat_cfg.IntersectionID == SPAT_INT_ID_DEFAULT);
  CU_ASSERT(spat_cfg.bsmEnable == BSM_RX_ENABLE_DEFAULT);
  CU_ASSERT(spat_cfg.bsmPsidPeruUnsecure == I2V_BSM_PSID_DEFAULT);
  i2v_setupSHM_Clean(UnitshmPtr); /* Close SHM. */
  //39% coverage

  //Validate conf_table.h matches spat.conf
  if ((UnitshmPtr = wsu_share_init(sizeof(i2vShmMasterT), I2V_SHM_PATH)) == NULL) {
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME," Main shared memory init failed\n");
      return;
  }
  CU_ASSERT(I2V_RETURN_OK == i2v_setupSHM_Default(UnitshmPtr)); /* Should reflect i2v.conf. */
  sleep(1); /* Give SHM a chance to digest. */
  strncpy(UnitshmPtr->cfgData.config_directory,DEFAULT_CONFIG,I2V_CFG_MAX_STR_LEN); /* Default conf used in install. */
  strcpy(confFileName, "spat16.conf");
  wsu_open_gate(&UnitshmPtr->scsSpatData.spatTriggerGate);
  ret = my_main();
  CU_ASSERT(I2V_RETURN_OK == ret);
  CU_ASSERT(spat_cfg.radioNum == I2V_RADIO_DEFAULT);
  CU_ASSERT(spat_cfg.wsaPriority == SPAT_WSA_PRIORITY_DEFAULT);
  CU_ASSERT(spat_cfg.psid == I2V_SPAT_PSID_DEFAULT);
  CU_ASSERT(spat_cfg.channel_number == I2V_RADIO_CHANNEL_DEFAULT);
  CU_ASSERT(spat_cfg.hdrExtra == SPAT_WSM_EXTENSION_ENABLE_DEFAULT);
  CU_ASSERT(spat_cfg.interopMode == SPAT_INTEROP_MODE_DEFAULT);
  CU_ASSERT(spat_cfg.IntersectionID == SPAT_INT_ID_DEFAULT);
  CU_ASSERT(spat_cfg.bsmEnable == BSM_RX_ENABLE_DEFAULT);
  CU_ASSERT(spat_cfg.bsmPsidPeruUnsecure == I2V_BSM_PSID_DEFAULT);
  i2v_setupSHM_Clean(UnitshmPtr); /* Close SHM. */

  if ((UnitshmPtr = wsu_share_init(sizeof(i2vShmMasterT), I2V_SHM_PATH)) == NULL) {
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME," Main shared memory init failed\n");
      return;
  }
  CU_ASSERT(I2V_RETURN_OK == i2v_setupSHM_Default(UnitshmPtr)); /* Should reflect i2v.conf. */
  sleep(1); /* Give SHM a chance to digest. */
  UnitshmPtr->cfgData.RadioType               = 1;
  UnitshmPtr->cfgData.globalDebugFlag         = 0;
  UnitshmPtr->cfgData.i2vDebugEnableFlag      = 1;
  UnitshmPtr->cfgData.bcastLockStep           = WTRUE;
  UnitshmPtr->cfgData.security                = WTRUE;
  wsu_open_gate(&UnitshmPtr->scsSpatData.spatTriggerGate);
  ret = my_main();
  CU_ASSERT(SPAT16_AOK == ret);
  i2v_setupSHM_Clean(UnitshmPtr); /* Close SHM. */

  //Pass in bad params forcing error.
  ret = uperFrame_SPAT( NULL,0,NULL,NULL);
  CU_ASSERT(SPAT16_UPERFRAME_SPAT_BAD_PARAMS == ret);

  //Signal stubs.c to return asn1 messageframe init error.
  set_stub_signal(Signal_asn1Init_MessageFrame);
  ret = uperFrame_SPAT( my_blob,BLOB_MAX,&my_frame,&my_ctxt);
  CU_ASSERT(SPAT16_ASN1_INIT_MESSAGE_FRAME_FAIL == ret);
  CU_ASSERT(0x0 == check_stub_pending_signals());

  //Signal stubs.c to return a rtx malloc fail.
  set_stub_signal(Signal_rtxMemAlloc);
  ret = uperFrame_SPAT( my_blob,BLOB_MAX,&my_frame,&my_ctxt);
  CU_ASSERT(SPAT16_RTX_MEM_ALLOC_FAIL == ret);
  CU_ASSERT(0x0 == check_stub_pending_signals());

  //Send proper params and see it return "ok" or 1 in this case.
  ret = uperFrame_SPAT( my_blob,BLOB_MAX,&my_frame,&my_ctxt);
  CU_ASSERT(1 == ret);
  CU_ASSERT(0x0 == check_stub_pending_signals());

  result_pointer = spatASN1EncodePER(&my_wsm, NULL, 1);
  CU_ASSERT(NULL == result_pointer); 

  set_stub_signal(Signal_pu_setBuffer);
  result_pointer = spatASN1EncodePER(&my_wsm, &spat, 1);
  CU_ASSERT(NULL == result_pointer);
  CU_ASSERT(0x0 == check_stub_pending_signals());

  set_stub_signal(Signal_asn1PE_SPAT);
  result_pointer = spatASN1EncodePER(&my_wsm, &spat, 1);
  CU_ASSERT(NULL == result_pointer);
  CU_ASSERT(0x0 == check_stub_pending_signals());

  set_stub_signal(Signal_pe_GetMsgPtr);
  result_pointer = spatASN1EncodePER(&my_wsm, &spat, 1);
  CU_ASSERT(NULL == result_pointer);
  CU_ASSERT(0x0 == check_stub_pending_signals());

  spat_sighandler(1);

  my_millisecond = 60999 + 1;
  getCurrMinVal(&my_millisecond);

  my_millisecond = 59999  + 1;
  getCurrMinVal(&my_millisecond);

 // test_scs_spat();
  #if 0
  //These wont work, no SHM setup here and SHM being passed in as function paramter?
  //i2v_setupSHM_Default();
  test_spat16_processing_mikes_worry_example();

  test_spat16_processing_overlap();

  test_spat16_processing_vehicle();
  
  test_spat16_processing_intersection_status();
  //i2vCleanShm();
#endif

  if ((UnitshmPtr = wsu_share_init(sizeof(i2vShmMasterT), I2V_SHM_PATH)) == NULL) {
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME," Main shared memory init failed\n");
      return;
  }
  CU_ASSERT(I2V_RETURN_OK == i2v_setupSHM_Default(UnitshmPtr)); /* Should reflect i2v.conf. */
  sleep(1); /* Give SHM a chance to digest. */
  set_stub_signal(Signal_wsu_share_init);
  wsu_open_gate(&UnitshmPtr->scsSpatData.spatTriggerGate);
  ret = my_main();
  CU_ASSERT(abs(SPAT16_SHM_FAIL)  == ret);
  CU_ASSERT(0x0 == check_stub_pending_signals());

  set_stub_signal(Signal_rtInitContextUsingKey);
  wsu_open_gate(&UnitshmPtr->scsSpatData.spatTriggerGate);
  ret = my_main();
  CU_ASSERT(abs(SPAT16_INIT_ASN1_CONTEXT_FAIL) == ret);
  CU_ASSERT(0x0 == check_stub_pending_signals());

#if 0 //use all cap functions
  set_stub_signal(Signal_wsu_wait_at_gate);
  wsu_open_gate(&UnitshmPtr->scsSpatData.spatTriggerGate);
  ret = my_main();
  CU_ASSERT(I2V_RETURN_OK == ret);
  CU_ASSERT(0x0 == check_stub_pending_signals());
#endif

  //Point to non-existent spat.conf and fopen() will fail.
  //Result is we get SPAT16_LOAD_CONF_FAIL.
  strncpy(UnitshmPtr->cfgData.config_directory,NO_CONFIG,I2V_CFG_MAX_STR_LEN);
  wsu_open_gate(&UnitshmPtr->scsSpatData.spatTriggerGate);
  ret = my_main();
  CU_ASSERT(abs(SPAT16_LOAD_CONF_FAIL) == ret);   //Did we get our result? If not fail test.
  CU_ASSERT(0x0 == check_stub_pending_signals()); //Did we use our signal? If not fail test.
  clear_all_stub_signal();                        //Recommended:If unsure clear all signals for next test.

  //Use default config in ./spat16 used by install.
  //Result is SPAT16_AOK.
  strncpy(UnitshmPtr->cfgData.config_directory,DEFAULT_CONFIG,I2V_CFG_MAX_STR_LEN);
  wsu_open_gate(&UnitshmPtr->scsSpatData.spatTriggerGate);
  ret = my_main();
  CU_ASSERT(SPAT16_AOK == ret);                   //Did we get our result? If not fail test.
  CU_ASSERT(0x0 == check_stub_pending_signals()); //Did we use our signal? If not fail test.
  clear_all_stub_signal();                        //Recommended:If unsure clear all signals for next test.

  //Set config to your pc ip so connect works and we write to socket in bsmrx.c
  //Note: We write even if nothing on otherside of connect.
  //Result is SPAT16_AOK.
  unit_test_flavor = UT_FLAVOR_MYPCIP;
  wsu_open_gate(&UnitshmPtr->scsSpatData.spatTriggerGate);
  ret = my_main();
  CU_ASSERT(SPAT16_AOK == ret);                   //Did we get our result? If not fail test.
  CU_ASSERT(0 == unit_test_flavor);               //Did we use our flavor? If not fail test.
  CU_ASSERT(0x0 == check_stub_pending_signals()); //Did we use our signal? If not fail test.
  unit_test_flavor = 0;                           //Recommended:If unsure clear flavor for next test.
  clear_all_stub_signal();                        //Recommended:If unsure clear all signals for next test.

  i2v_setupSHM_Clean(UnitshmPtr); /* Close SHM. */

  set_stub_signal(Signal_pu_setBuffer);
  result_pointer = asn1EncodeUPER_MessageFrame( &my_wsm,&my_frame,&my_ctxt);
  CU_ASSERT(NULL == result_pointer);
  CU_ASSERT(0x0 == check_stub_pending_signals());

  set_stub_signal(Signal_asn1PE_MessageFrame);
  result_pointer = asn1EncodeUPER_MessageFrame( &my_wsm,&my_frame,&my_ctxt);
  CU_ASSERT(NULL == result_pointer);
  CU_ASSERT(0x0 == check_stub_pending_signals());

  set_stub_signal(Signal_pe_GetMsgPtr);
  result_pointer = asn1EncodeUPER_MessageFrame( &my_wsm,&my_frame,&my_ctxt);
  CU_ASSERT(NULL == result_pointer);
  CU_ASSERT(0x0 == check_stub_pending_signals());

  result_pointer = asn1EncodeUPER_MessageFrame( &my_wsm,&my_frame,&my_ctxt);
  CU_ASSERT(NULL != result_pointer);

  mainloop = WTRUE;  

  /* Pass in bad param and trigger bsmFwdLogger to print. */ 

  /* Excercise for coverage. */

  my_wsm.dataLength = MAX_WSM_DATA - 1;

  CU_ASSERT(0x0 == check_stub_pending_signals());

  /* Empty blob. */
  memset(&spat_info_block,0x0,sizeof(spat_info_block));
  spat_cfg.numEntries = 1;
  //memcpy(&UnitshmPtr->scsSpatData.spat_info, &spat_info_block, sizeof(spat_info_block));
  spat_process_main(&spat);

//82% coverage

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
