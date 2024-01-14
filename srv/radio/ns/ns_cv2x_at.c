/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Filename: ns_cv2x_at.cpp                                                   */
/*  Purpose: C-V2X interface module                                           */
/*           Low-level interface to the C-V2X radio via AutoTalks             */
/*                                                                            */
/* Copyright (C) 2022 DENSO International America, Inc.                       */
/*----------------------------------------------------------------------------*/

/* NOTE: This was a CPP file!!!   Converted to C for AutoTalks */
/*    - Removed promise code */
/*    - Removed templates */
//    - Removed stuff marked "Interface defined by the Qualcomm Telematics SDK"
//    - The DENSO functions created here and the AutoTalks SDK functions 
//      were both named starting cv2x_* and in similar naming styles.
//      Renamed some radiostack functions to rs_cv2x_* to reduce confusion.
//    - The new rs_cv2x_tx removes the 2nd param "priority" from old cv2x_tx,
//      Autotalks SDK cv2x_tx does not accept priority param.
//    - However, note that in cv2x_send_params_t, there is a
//      power_dbm8[ATLK_INTERFACES_MAX], description is
//      "Transmission power level in units of 1/8 dBm Per Antenna"
/****************************************************************/

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <stdlib.h>     // for system()

#include <stdint.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <dn_types.h>
#include "shm_rsk.h"
// Autotalks Includes:
#include <atlk/sdk.h>
#include <atlk/dsm.h>
#include <atlk/ddm.h>
#include <atlk/ddm_service.h>
#include <atlk/wdm.h>
#include <atlk/wdm_service.h>
#include <atlk/v2x_service.h>
#include <atlk/cv2x.h>
#include <atlk/cv2x_service.h>
#include <atlk/verstring.h>
#include <extern/time_sync.h>
#include <poti_api.h>
#include <extern/ref_cv2x_sys.h>

#include <extern/ref_sys.h>
#include <extern/target_type.h>
#if defined(ENABLE_HSM_DAEMON)
#include "v2xSe.h"
#endif
#include "i2v_util.h"
#include "nscfg.h"
#if defined(MY_UNIT_TEST)
#include "stubs.h"
#endif
extern shm_rsk_t *shm_rsk_ptr;
extern bool_t     mainLoop;

#define CV2X_DEVICE "/dev/spidev1.0"
#define MAX_CV2X_IS_READY 10 /* Number of tries for CV2X ready before we bail. */

/* mandatory logging defines */
#define MY_ERR_LEVEL   LEVEL_PRIV    /* from i2v_util.h */
#define MY_NAME        "cv2x"

/* GLOBALS for AutoTalks cv2x */

/* 
 * Question: Are these the max sizes AT can tx & rx?
 *   From acme tool: "cv2x-example /dev/spidev1.0 5" && "acme -RV"
 *         non IP MTU: 8188 (rmnet_data1)
 *             IP MTU: 1500 (rmnet_data0)
 */
#if 1
#define MAX_CV2X_MSG_SIZE 2048  // TODO: Get a number based on real data
#else
#define MAX_CV2X_MSG_SIZE 2321 // MAX_WSA_DATA Too big for socket policy when tried.
#endif
cv2x_service_t        *cv2x_service_ptr = NULL;
cv2x_socket_t         *cv2x_rx_socket_ptr = NULL;      // The one cv2x rx socket
cv2x_socket_t         *cv2x_tx_socket_ptr[MAX_SPS_FLOWS];   // The many cv2x tx sockets
wdm_service_t *wdm_service_ptr = NULL;
ddm_service_t *ddm_service_ptr = NULL;
cv2x_send_params_t    cv2x_send_params = CV2X_SEND_PARAMS_INIT;

rskStatusType cv2xStatus;

/* Uncomment EXTRA_CONSOLE_MESSAGES to have extra console messages printed in
 * case anything goes wrong. */
//#define EXTRA_CONSOLE_MESSAGES

/*----------------------------------------------------------------------------*/
/* Constants                                                                  */
/*----------------------------------------------------------------------------*/
#define SPS_SRC_PORT_NUM     2500u
#define SPS_EVT_PORT_NUM     (SPS_SRC_PORT_NUM + MAX_SPS_FLOWS)
#define RX_PORT_NUM          9000u
#define PRIORITY             3
#define DDM_TEMP_READING_CONSECUTIVE_FAIL_MAX 10 /* Allow so many API consecutive fails then error */

/*----------------------------------------------------------------------------*/
/* Local Variables                                                            */
/*----------------------------------------------------------------------------*/
static bool_t                            cv2xInited = FALSE;

/* The semaphone below insures only one instance of either cv2x_init() or
 * cv2x_term() is running at a time. This way, we can insure that gTxHandle will
 * match whatever instance of the function is running */
static sem_t                           init_term_tx_sem;
/* Some globals that we do actually need [20220513] */
static int                             gTxHandle;
static uint32_t                        gServiceId[MAX_SPS_FLOWS];
static uint32_t                        cv2x_api_error_count = 0;
static int32_t                         prior_cv2x_temperature_celsius = 0;
static DeviceTallyType currenttallies;
static cv2x_socket_t * cv2x_socket_ptr = NULL; // Moved outside of originally posn in pasted block, so can be passed to following code to store it in tx_socket array
static cv2x_socket_policy_t  reg_policy;
/*----------------------------------------------------------------------------*/
/* Functions                                                                  */
/*----------------------------------------------------------------------------*/
void initCv2x(uint32_t debugEnable)
{
  cv2xInited = FALSE;
  memset(&init_term_tx_sem,0x0,sizeof(init_term_tx_sem));
  gTxHandle = 0;
  memset(gServiceId,0x0,sizeof(gServiceId));
  cv2x_api_error_count = 0;
  prior_cv2x_temperature_celsius = 0;
  memset(&currenttallies,0x0,sizeof(currenttallies));
  cv2x_socket_ptr = NULL;
  memset(&reg_policy,0x0,sizeof(reg_policy));
}

/*----------------------------------------------------------------------------*/
/** Function:  getCv2xTemp                            
** @brief  Do once-at-radioServices-up C-V2X communication initialization.
** @return -45 to 85: less than -900 == error
**
** Details:
**/ 
#if 0 /* Value looks wrong 10C or 12C. */
      /* Grab temperature. */
      memset(&rf_status,0x0,sizeof(rf_status));
      rc = wdm_rf_status_get(wdm_service_ptr, &rf_status);
      if (atlk_error(rc)) {
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"RF STATUS err = %d(%s)\n", rc, atlk_rc_to_str(rc));
          cv2xStatus.temperature = -1;
      } else {
          cv2xStatus.temperature = rf_status.temperature_celsius;
      }
#endif
/*----------------------------------------------------------------------------*/
static int32_t getCv2xTemp(void)
{
#if defined (MY_UNIT_TEST)
  return 66;
#else
  atlk_rc_t rc;
  ddm_status_t status;

    if(NULL != ddm_service_ptr) { /* Set in rs_cv2x_init() */
        if(0 == sem_trywait(&init_term_tx_sem)) {
            rc = ddm_status_get(ddm_service_ptr, &status);
            if (atlk_error(rc)) {
                status.temperature_celsius = -903; /* Service call failure. */
            } else {
                if (status.temperature_celsius == DDM_TEMPERATURE_READING_INVALID) {
                    status.temperature_celsius = -904; /* ddm temp reading failure. */
                } else {
                    prior_cv2x_temperature_celsius = status.temperature_celsius;
                    cv2x_api_error_count = 0;
                }
            }
            sem_post(&init_term_tx_sem);
        } else {
            status.temperature_celsius = -902; /* Locked. */
        }
    } else {
        status.temperature_celsius = -901; /* Not ready */
    }
    if(-903 >= status.temperature_celsius) {
        if(0 == (cv2x_api_error_count % 10)) {
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"getCv2xTemp: API fail(%d) rc(%d). Using prior(%d)\n",status.temperature_celsius,prior_cv2x_temperature_celsius);
        }
        /* TODO: why?
         * See API fail once in awhile but not consecutive fails. 
         * Allow only so many consecutive fals then let failure fall through.
         */
        if(cv2x_api_error_count < DDM_TEMP_READING_CONSECUTIVE_FAIL_MAX) {
            status.temperature_celsius = prior_cv2x_temperature_celsius;
        }
        cv2x_api_error_count++;
    }
    return status.temperature_celsius;
#endif
}

/*------------------------------------------------------------------------------
** Function:  rs_cv2x_module_init
** @brief  Do once-at-radioServices-up C-V2X communication initialization.
** @return 0 for success; -1 for error
**
** Details: Do the initialization of the C-V2X module that needs to be done
**          once when radioServices is first brought up. This routine is called
**          from rsk_init_module() in rsk.c. It initializes the init_term
**          semaphore and sets all of the gServiceId's to 0xffffffff (unused).
*
**          Before radioServices is terminated, rs_cv2x_module_term() should be
**          called.
**----------------------------------------------------------------------------*/
int rs_cv2x_module_init(void)
{
    int txHandle;

    memset(&cv2xStatus,0x0,sizeof(cv2xStatus));
    if (0x1 == getI2vSyslogEnable()) {
        i2vUtilEnableSyslog(MY_ERR_LEVEL, MY_NAME); //if LOGMGR not up then only serial output.
        /* Enable serial debug with I2V_DBG_LOG until i2v.conf says otherwise. */
        i2vUtilEnableDebug(MY_NAME);
    }

    sem_init(&init_term_tx_sem, FALSE, 1);

    for (txHandle = 0; txHandle < MAX_SPS_FLOWS; txHandle++) {
        gServiceId[txHandle] = 0xffffffff; // Marks it as unused
        shm_rsk_ptr->CV2XCnt.serviceID[txHandle] = 0xffffffff;
    }

    return 0;
}

/*------------------------------------------------------------------------------
** Function:  rs_cv2x_module_term
** @brief  Do once-at-radioServices-down C-V2X communication termination.
** @return 0 for success; -1 for error
**
** Details: Do the termination of the C-V2X module that needs to be done
**          when radioServices is terminated. This routine is called from
**          rsk_exit_module() in rsk.c. It terminates any Tx or Rx service that
**          may have been started. Then it destroys the init_term semaphore.
**----------------------------------------------------------------------------*/
int rs_cv2x_module_term(void)
{
    int txHandle;

    for (txHandle = 0; txHandle < MAX_SPS_FLOWS; txHandle++) {
        if (gServiceId[txHandle] != 0xffffffff) {
            rs_cv2x_term_tx(gServiceId[txHandle]);
        }
    }

    if (cv2xInited) {
        rs_cv2x_term();
    }

    sem_destroy(&init_term_tx_sem);
 #if defined(EXTRA_DEBUG)
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"rs_cv2x_module_term: Returning 0\n");
#endif
    i2vUtilDisableSyslog();
    i2vUtilDisableDebug(); 
    return 0;
}

/* This is a local function, not to be confused with ref_sys_init. This function calls ref_sys_init in order
   to initialize the system. ref_sys_init is defined in another c file: ref_sys.c */
// Function pulled from cv2x-example.c.  Added "autotalks__" in front of name.
// Remove eth/interface_name.  Using #define'd CV2X_DEVICE now.
static atlk_rc_t
autotalks__init_ref_sys(
             unsigned *src_l2id_ptr,
             CV2X_OUT cv2x_configuration_t *config_ptr,
             CV2X_OUT cv2x_rrc_pre_config_t *rrc_config_ptr)
{
  atlk_rc_t rc;

  /* We must set SDK in pass-through mode so it will not assume v2x headers. */
  rc = v2x_sockets_passthrough_set(1);
  if (atlk_error(rc)) {
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"init_ref_sys v2x_sockets_passthrough_set failed %u (%s)\n", rc, atlk_rc_to_str(rc));
    cv2xStatus.error_states |= CV2X_SOCKET_INIT_FAIL;
    return ATLK_E_UNSPECIFIED;
  }


  /* Reference system initialization - for Secton init the USB ethernet adapter for host-to-device connections,
     for both Craton/Secton perform different link layer inits etc */
  rc = ref_cv2x_sys_init(CV2X_DEVICE, NULL, config_ptr, rrc_config_ptr);
  if (atlk_error(rc)) {
    cv2xStatus.error_states |= CV2X_SYS_INIT_FAIL;
    return ATLK_E_UNSPECIFIED;
  }

  *src_l2id_ptr = ref_cv2x_ue_id_get();

  return ATLK_OK;
}
// Another function copied from cv2x-example.c 
/* Show CV2XLIB + SDK + BSP + DSP version */
static int
autotalks__cmd_sdk_version(wdm_service_t *wdm_service, ddm_service_t *ddm_service)
{
  char sdk_version[256];
  char h_version[256];
  size_t sdk_version_size;
  char *sdk_ver = sdk_version;
  atlk_rc_t rc;
  wdm_dsp_version_t dsp_version;
  ddm_baseband_v2x_hw_revision_t v2x_hw_rev;
#ifndef SECTON
  ddm_bsp_version_t bsp_version;
#endif
  uint32_t mj, mn, rv;

  cv2xStatus.firmware = 1; /* True till proven FALSE */

  snprintf(h_version,256,"%d.%d.%d %s", ATLK_SDK_VER_MAJOR, ATLK_SDK_VER_MINOR, ATLK_SDK_VER_PATCH, HOST_LL_TYPE);
#if 1 //defined(EXTRA_DEBUG)
  I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Host:\n");
  I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"  CV2X version:      %s\n", h_version);
  I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"  Software version:  %s\n", HOST_SDK_REV_ID);
#endif
  sdk_version_size = sizeof(sdk_version);

  rc = ddm_version_get(ddm_service, sdk_version, &sdk_version_size);
  if (atlk_error(rc)) {
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Failed to get SDK version rc=%d\n", rc);
  }
  else {
    if(0 != (strcmp(h_version, sdk_ver))){
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Device SDK version Mismatch\n");
        cv2xStatus.firmware = -1;
    }
#if 1 //defined(EXTRA_DEBUG)
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Device:\n");
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"  SDK version:       %s\n", sdk_ver);
#endif
    sdk_ver = sdk_ver + strlen(sdk_version) + 1;
    if(0 != (strcmp(HOST_SDK_REV_ID, sdk_ver))){
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Device Software version Mismatch\n");
        cv2xStatus.firmware = -2;
    }
#if 1 //defined(EXTRA_DEBUG)
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"  Software version:  %s\n", sdk_ver);
#endif
  }

  rc = wdm_dsp_version_get(wdm_service, &dsp_version);
  if (atlk_error(rc)) {
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Failed to get WDM DSP version rc=%d\n", rc);
  }
  else {
#if 1 //defined(EXTRA_DEBUG)
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"DSP:\n");
#endif
    snprintf(h_version,256,"%u.%u.%u %s",dsp_version.major, dsp_version.minor, dsp_version.sw_revision, dsp_version.dsp_rev_id);
    if(   (0 != (strcmp(h_version, "3.9.0 rc1-bld1")))
       && (0 != (strcmp(h_version, "3.10.0 rc1-bld7")))){
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"DSP Software version Mismatch\n");
        cv2xStatus.firmware = -3;
    }
#if 1 //defined(EXTRA_DEBUG)
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"  Software version:  %"PRIu8".%"PRIu8".%"PRIu8" %s\n",
           dsp_version.major, dsp_version.minor, dsp_version.sw_revision, dsp_version.dsp_rev_id);
#endif
  }

#ifndef SECTON
  rc = ddm_bsp_version_get(ddm_service, &bsp_version);
  if (atlk_error(rc)) {
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Failed to get DDM BSP version rc=%d\n", rc);
  }
#if 1 //defined(EXTRA_DEBUG)
  else {
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"BSP:\n"
           "  M3 loader version: %s\n"
           "  OS loader version: %s\n"
           "  OS version:        %s\n",
           bsp_version.m3_loader_version, bsp_version.ap_loader_version, bsp_version.ap_os_version);
  }
#endif
#endif

  rc = ddm_baseband_v2x_hw_revision_get(ddm_service, &v2x_hw_rev);
  if (atlk_error(rc)) {
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Failed to get DDM baseband V2X hardware version rc=%d\n", rc);
  }
  else {
    snprintf(h_version,256,"%u.%u", v2x_hw_rev.bb_v2x_hw_rev_major, v2x_hw_rev.bb_v2x_hw_rev_minor);
    if(0 != (strcmp(h_version, "3.0"))){
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Baseband Hardware version Mismatch\n");
        cv2xStatus.firmware = -4;
    }
#if 1 // defined(EXTRA_DEBUG)
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Baseband V2X:\n");
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"  Hardware version:  %"PRIu32".%"PRIu32"\n", v2x_hw_rev.bb_v2x_hw_rev_major, v2x_hw_rev.bb_v2x_hw_rev_minor);
#endif
  }
//TODO: move magic numbers to .h
  if (cv2x_version_get(&mj, &mn, &rv) == 0) {
    uint32_t build_number = 0;
    cv2x_build_version_get(&build_number);
    if(   ((3==mj) && (8==mn) && (2==rv) && (0==build_number))
       || ((3==mj) && (9==mn) && (0==rv) && (0==build_number))){

    } else {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"MAC CV2X Version Mismatch.\n");
        cv2xStatus.firmware = -5;
    }
#if 1 // defined(EXTRA_DEBUG)
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"  MAC CV2X Version:  %d.%d.%d-%d (%s)\n", mj, mn, rv, build_number, sdk_version);
//    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"  Software version:  %s\n", CV2X_LIB_REV_ID); // First available in Secton 5.16
#endif
  }
  if(1 != cv2xStatus.firmware){
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Firmware revisions wrong: rc = %d.\n",cv2xStatus.firmware);
      cv2xStatus.error_states |= CV2X_FW_REVISION_FAIL;
  }
  return 0;
}



/*------------------------------------------------------------------------------
** Function:  rs_cv2x_init  (orig named cv2x_init() in ns_cv2x.cpp before conversion)
** @brief  Initialize C-V2X communication.
** @return 0 for success; -1 for error
**
** Details: Does the initialization of C-V2X that is common to both Tx and Rx,
**          then does the initialization of C-V2X that is specific to Rx.
**
**          It is intended that this function be called when radioServices
**          starts up (e.g. from rsk_init_module()). It is intended that
**          cv2x_term() be called when radioServices shuts down (e.g. from
**          rsk_exit_module()).
**
**          Some time soon after cv2x_init is called, a thread should be
**          launched that repeatedly calls cv2x_rx() and sends whatever packets
**          are received to their ultimate destination. This thread should be
**          terminated before calling cv2x_term().
**----------------------------------------------------------------------------*/
int rs_cv2x_init(void)
{
    atlk_rc_t             rc;
    cv2x_rrc_pre_config_t rrc_config;

#ifdef EXTRA_CONSOLE_MESSAGES
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"%s()\n", __func__);
#endif


    /* Exit if radioServices going down */
    if (!mainLoop) {
#ifdef EXTRA_CONSOLE_MESSAGES
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"RS_CV2X: Returning -1\n");
#endif
        return -1;
    }

    // Don't initialize more than once
    if (cv2xInited) {
#ifdef EXTRA_CONSOLE_MESSAGES
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"RS_CV2X already initialized!\n");
#endif
        cv2xStatus.error_states |= CV2X_INIT_TWICE_WARNING;
        goto cv2x_init_ok;
    }

    /* Set up Secton CV2X */
    // THIS CODE INITIALLY PULLED FROM cv2x-example.c from Autotalk's SECTON SDK [20220513]
    {
      unsigned              ue_id = 0;
      cv2x_configuration_t  initial_cv2x_config = CV2X_CONFIGURATION_INIT;
#if defined(EXTRA_DEBUG)
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"init_ref_sys...\n");
#endif

      //
      if (autotalks__init_ref_sys(&ue_id, &initial_cv2x_config, &rrc_config) == ATLK_OK) {
#if defined(EXTRA_DEBUG)
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"           --> init_ref_sys succeded!\n");
#endif
      }
#if defined(EXTRA_DEBUG)
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"getting WDM service ptr...\n");
#endif
      rc = wdm_service_get(NULL, &wdm_service_ptr);
      if (atlk_error(rc)) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"wdm_service_get failed: %d\n", rc);
        cv2xStatus.error_states |= CV2X_WDM_SERVICE_GET_FAIL;
        return EXIT_FAILURE;
      }
#if defined(EXTRA_DEBUG)
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"getting DDM service ptr...\n");
#endif
      rc = ddm_service_get(NULL, &ddm_service_ptr);
      if (atlk_error(rc)) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"ddm_service_get failed: %d\n", rc);
        cv2xStatus.error_states |= CV2X_DDM_SERVICE_GET_FAIL;
        return EXIT_FAILURE;
      }
#if defined(EXTRA_DEBUG)
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Waiting for Device to be ready...\n");
#endif
      ddm_state_t state;

      rc = ddm_state_get(ddm_service_ptr, &state);
      if ((atlk_error(rc)) || (state != DDM_STATE_READY)) {
        do {
          sleep(2);
          rc = ddm_state_get(ddm_service_ptr, &state);
          if (atlk_error(rc)) {
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"ddm_state_get failed: %d\n", rc);
            cv2xStatus.error_states |= CV2X_DDM_STATE_GET_FAIL;
            return EXIT_FAILURE;
          }

          if (state != DDM_STATE_READY) {
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Device still not ready ... waiting...\n");
          }
        } while (state != DDM_STATE_READY);
      }
#if defined(EXTRA_DEBUG)
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Device ready\n");
#endif
      autotalks__cmd_sdk_version(wdm_service_ptr, ddm_service_ptr);

      rc = time_sync_init(ddm_service_ptr);
      if (atlk_error(rc)) {
        cv2xStatus.error_states |= CV2X_DDM_SYNC_INIT_FAIL;
        return EXIT_FAILURE;
      }
#if defined(EXTRA_DEBUG)
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"getting CV2X service ptr...\n");
#endif
      rc = cv2x_service_get(NULL, &cv2x_service_ptr);
      if (atlk_error(rc)) {
        cv2xStatus.error_states |= CV2X_GET_SERVICE_FAIL;
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"getting service ptr...ERROR: cv2x_service_get fail. %s\n", atlk_rc_to_str(rc));
        return EXIT_FAILURE;
      }

      rc = cv2x_configuration_set(&initial_cv2x_config, &rrc_config);
      if (atlk_error(rc)) {
        cv2xStatus.error_states |= CV2X_CONFIG_SET_FAIL;
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"cv2x_configuration_set failed: %d\n", rc);
        return EXIT_FAILURE;
      }

      rc = cv2x_service_enable(cv2x_service_ptr);
      if (atlk_error(rc)) {
        cv2xStatus.error_states |= CV2X_SERVICE_ENABLE_FAIL;
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"cv2x_service_enable failed: %d\n", rc);
        return EXIT_FAILURE;
      }
#if defined(EXTRA_DEBUG)
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Setting CV2X src_l2id to 0x%x.\n", ue_id);
#endif
      rc = cv2x_src_l2id_set(cv2x_service_ptr, ue_id);
      if (atlk_error(rc)) {
        cv2xStatus.error_states |= CV2X_L2ID_SET_FAIL;
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Error: cv2x_src_l2id_set failed. rc = %d \n",rc);
      }
#if defined(EXTRA_DEBUG)
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"CV2X ready !\n");
#endif
    }
    // END - THIS CODE INITIALLY PULLED FROM cv2x-example.c from Autotalk's SECTON SDK [20220513]
{
  int32_t i = 0;
  wdm_interface_status_t interface_status;
  char_t interface_name[IF_NAMESIZE+1];

      /* Grab interface status. */
      for(i=0;i<1;i++){
          memset(&interface_status,0x0,sizeof(interface_status));
          rc = wdm_interface_status_get(wdm_service_ptr,i,&interface_status);
          if (atlk_error(rc)) {
              I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"IFC%d STATUS err = %d(%s)\n",i, rc, atlk_rc_to_str(rc));
          } else {
              memset(&interface_name,0,sizeof(interface_name));
              if (if_indextoname(i, interface_name) == NULL) {
                  interface_name[0] = 'X';
                  interface_name[1] = 'X';
                  interface_name[2] = 'X';
                  interface_name[3] = '\0';
              }
             // I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"IFC%d(%s) mode(%d) state(%d)\n",i, interface_name,interface_status.mode,interface_status.state);
          }
      }
      if(   (WDM_INTERFACE_MODE_CV2X != interface_status.mode)
         || (WDM_INTERFACE_STATE_ATTACHED != interface_status.state)) {
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Interface mode(%d) & state(%d) wrong.\n",interface_status.mode,interface_status.state);
          cv2xStatus.interface = -1;
      } else {
          cv2xStatus.interface = 1;
      }
}
    // NOTE: Per the comments of the original function, we need to activate for RX as well, so
    // NOTE: This code initializes the RX service
    // COPIED from cv2x_rx_mode() thread function in cv2x-example.c - xxxxxxxxxxxxxxxxxxxxxxxx
    {
      cv2x_socket_config_t socket_config = CV2X_SOCKET_CONFIG_INIT;

      //passed arg -- cv2x_service_ptr = (cv2x_service_t *) arg;
#if defined(EXTRA_DEBUG)
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"RX started.\n");

      //I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"creating RX socket...\n");
#endif
      // Create the one Rx socket
      rc = cv2x_socket_create(cv2x_service_ptr, CV2X_SOCKET_TYPE_RX, &socket_config, &cv2x_rx_socket_ptr);
      if (atlk_error(rc)) {
        cv2xStatus.error_states |= CV2X_SOCKET_CREATE_FAIL;
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"creating RX socket... ERROR: cv2x_socket_create fail. %s\n", atlk_rc_to_str(rc));
        goto cv2x_init_err;
      }
#if defined(EXTRA_DEBUG)
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"creating RX socket...OK\n");
#endif
    }
    // END - COPIED from cv2x_tx_mode() thread function in cv2x-example.c - xxxxxxxxxxxxxxxxxxxxxxxx

    /* Exit if radioServices going down */
    if (!mainLoop) {
#ifdef EXTRA_CONSOLE_MESSAGES
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"radioServices doing down!\n");
#endif
        goto cv2x_init_err;
    }

    /* Dump config items to /tmp for cv2xcfg who is called by web gui eventually. */
    {
      char_t cmd[128];
        #if defined(EXTRA_DEBUG)
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"rcc freq(%u) cv2x freq(%u)\n",
                rrc_config.v2x_pre_config_freq_list_r14_ptr->v2x_comm_pre_config_general_r14.carrier_freq_r12
                ,RRC_FREQ_TO_CV2X_FREQ(rrc_config.v2x_pre_config_freq_list_r14_ptr->v2x_comm_pre_config_general_r14.carrier_freq_r12));
        #endif
        memset(cmd,0x0,sizeof(cmd));
        /* Convert from rrc to cv2x freq. */
        sprintf(cmd,"echo %u > /tmp/cv2x_freq.txt",RRC_FREQ_TO_CV2X_FREQ(rrc_config.v2x_pre_config_freq_list_r14_ptr->v2x_comm_pre_config_general_r14.carrier_freq_r12));
        if(system(cmd) < 0) {
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"rs_cv2x_init: system() failed.\n");
            //cv2xStatus.error_states |= CV2X_INIT_FAIL;
        }
    }

    cv2xStatus.ready = 1;
    // Normal exit
    cv2xInited = TRUE;
    shm_rsk_ptr->CV2XCnt.cv2xInited = 1;
#if defined(EXTRA_DEBUG)
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"cv2xInited = TRUE\n");
#endif
cv2x_init_ok:
#if defined(EXTRA_DEBUG)
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"%s() returning 0\n", __func__);
#endif
    return 0;

    // Error exit
cv2x_init_err:
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"rs_cv2x_init failed: ret == -1\n");
    return -1;
}

/*------------------------------------------------------------------------------
** Function:  rs_cv2x_init_tx
** @brief  Initialize a C-V2X Tx communication.
** @param  serviceId      -- The PSID or ITS_AID associated with the Tx flow
** @param  reservedBufLen -- The initial reserved buffer lendth for the Tx flow
** @param  peakTxPower    -- The peak Tx power value. If 0, use the default.
** @return The Tx handle for success; -1 for error
**
** Details: Only one instance of rs_cv2x_init_tx or rs_cv2x_term_tx can run at a time
**          so that no errors result in the manipulation of the gServiceID
**          array and we can be sure which Tx flow gHandle is associated with.
**
**          First we make sure the serviceId is not already registered. Then we
**          search for a free Tx handle; error if there is none. Then the Tx
**          flow is created.
**          rsk-exit-module()).
**
** NOTE: This is setting up a channel to transmit
**
**          The service Id is stored in the gServiceId array.
**----------------------------------------------------------------------------*/
#define RSVP_MILLIS     100         // from SECTON SDK's cv2x-example.c
int rs_cv2x_init_tx(uint32_t serviceId, uint32_t reservedBufLen,
                 int32_t peakTxPower)
{
    // Allow only one instance of either cvx2_init_tx() or cvx2_term_tx() to
    // run at a time
    sem_wait(&init_term_tx_sem);

    int         txHandle;
// COPY OF CODE FROM .cpp file's function
//   Error if passed serviceId is a duplicate of one passed before
//   Assign an unused gServiceId[] to passed serviceId

    // Look for duplicate serviceId - Did user already create a tx socket for passed serviceId?
    for (txHandle = 0; txHandle < MAX_SPS_FLOWS; txHandle++) {
        if (gServiceId[txHandle] == serviceId) {
            cv2xStatus.error_states |= CV2X_TX_DUPE_SERVICE_ID_FAIL;
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Error: Duplicate serviceId 0x%x\n", serviceId);
            goto cv2x_init_tx_err2;
        }
    }

    /** Unit scale conversion factors */

    /*
      Tx thread - transmit mode thread (logical thread)
      This thread creates a transmit socket and periodically (every 100ms) transmits a messages on the socket.
      The socket may be of SPS type or ad-hoc type, depending on the execution mode of the application.
      For SPS socket type, the function also defines the transmit policy.
      Timestamp and message-ID is printed for each received message.
      Parameters:
      - [IN] arg             : cv2x_service_t pointer
      - [global] ad_hoc_mode : indicates whether the application is executed in ad-hoc mode (otherwise SPS mode is used)
    */
    /* FOLLOWING is a paste of cv2x_tx_mode() from cv2x-example.c.  Its code needs adjustment to live here. */
    //cv2x_tx_mode(void *arg) // note: param is a pointer to a cv2x_service_t
//    cv2x_socket_t         *cv2x_socket_ptr; // Moved outside of originally posn in pasted block, so can be passed to following code to store it in tx_socket array
    {
      cv2x_socket_config_t  cv2x_socket_config = CV2X_SOCKET_CONFIG_INIT;
      ddm_service_t         *ddm_service_ptr   = NULL;
 //     cv2x_socket_policy_t  reg_policy;
      atlk_rc_t             rc;
#if defined(EXTRA_DEBUG)
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"TX started.\n");
#endif

      rc = ddm_service_get(NULL, &ddm_service_ptr);
      if (atlk_error(rc)) {
        cv2xStatus.error_states |= CV2X_TX_DDM_SERVICE_GET_FAIL;
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"ddm_service_get failed: %d\n", rc);
        // return NULL;
        goto cv2x_init_tx_err2;
      }

      /* Socket creation */
      {
#if defined(EXTRA_DEBUG)
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"creating SP Socket...\n");
#endif
        // Create Tx SPS socket using AutoTalks API call
        rc = cv2x_socket_create(cv2x_service_ptr,
                                CV2X_SOCKET_TYPE_SEMI_PERSISTENT_TX,
                                &cv2x_socket_config,
                                &cv2x_socket_ptr);
        if (atlk_error(rc)) {
          cv2xStatus.error_states |= CV2X_TX_SOCKET_CREATE_FAIL;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"creating SP Socket...ERROR: cv2x_socket_create fail. %s\n", atlk_rc_to_str(rc));
          // return NULL;
          goto cv2x_init_tx_err2;
        }
#if defined(EXTRA_DEBUG)
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"creating SP Socket...OK\n");
#endif
        reg_policy.control_interval_ms = RSVP_MILLIS;

        reg_policy.size     = MAX_CV2X_MSG_SIZE;
        reg_policy.priority = CV2X_PPPP_3;
#if defined(EXTRA_DEBUG)
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Setting SP Socket Policy...\n");
#endif
        // Policy set
        rc = cv2x_socket_policy_set(cv2x_socket_ptr, &reg_policy);
        if (atlk_error(rc)) {
          cv2xStatus.error_states |= CV2X_TX_SOCKET_POLICY_FAIL;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Setting SP Socket Policy...ERROR: cv2x_socket_policy_set fail. %s\n", atlk_rc_to_str(rc));
          // return NULL;
          goto cv2x_init_tx_err2;
        }
#if defined(EXTRA_DEBUG)
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Setting SP Socket Policy...OK\n");
#endif
        /* update power. It's been doubled to remove fractional float part so we only need to 4X here not 8X */
        for (txHandle = 0;  txHandle < ATLK_INTERFACES_MAX; txHandle++) {
          /* ATLKS expects 1/8 dBm units so power has to be multiplied by 8; macros below perform that conversion */
          cv2x_send_params.power_dbm8[txHandle] = peakTxPower * (POWER_DBM8_PER_DBM/2);    /* units already in half steps so no need to do additional adjustment */
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Txpwr=%d, peakTxPower=%d.\n",cv2x_send_params.power_dbm8[txHandle],peakTxPower);
        }

        /* Dump config items to /tmp for cv2xcfg who is called by web gui eventually. */
        {
          char_t cmd[128];
            currenttallies.tx_power = cv2x_send_params.power_dbm8[0];
            memset(cmd,0x0,sizeof(cmd));
            /* Scale power to be the same as web gui display. */
            sprintf(cmd,"echo %2.1f > /tmp/cv2x_tx_pwr.txt",(float32_t)((float32_t)currenttallies.tx_power/8.0f));
            if(system(cmd) < 0) {
                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"rs_cv2x_init_tx: system() failed.\n");
                //cv2xStatus.error_states |= CV2X_INIT_FAIL;
            }
        }
      }

    }

    // Save the tx socket in our cv2x_tx_socket_ptr[] array
    // Store serviceId in an available gServiceId[] entry
    //   both indexed by txHandle as their index
    for (txHandle = 0; txHandle < MAX_SPS_FLOWS; txHandle++) {
        if (gServiceId[txHandle] == 0xffffffff) {
            gServiceId[txHandle] = serviceId;
            cv2x_tx_socket_ptr[txHandle] = cv2x_socket_ptr;
            shm_rsk_ptr->CV2XCnt.serviceID[txHandle] = serviceId;
            break;
        }
    }
    if (txHandle == MAX_SPS_FLOWS) {
        cv2xStatus.error_states |= CV2X_SPS_FLOWS_USED_FAIL;
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Error: All Tx SPS flows in use\n");
        goto cv2x_init_tx_err2;
    }
#if defined(EXTRA_DEBUG)
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"txHandle = %d\n", txHandle);
#endif
    // Allow callback functions to know which Tx handle is in use
    gTxHandle = txHandle;


#ifdef xxxxxxxxxxxxxxxxxx_this_not_necessary_on_secton_i_hope_xxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    Status      status;
    ErrorCode   errorCode;
    SpsFlowInfo spsInfo;

#ifdef EXTRA_CONSOLE_MESSAGES
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"%s(0x%x, %u, %d)\n", __func__,
                   serviceId, reservedBufLen, peakTxPower);
#endif

    // Abort if C-V2X not inited
    if (!cv2xInited) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"C-V2X not initialized!\n");
        goto cv2x_init_tx_err2;
    }

    // Look for duplicate serviceId
    for (txHandle = 0; txHandle < MAX_SPS_FLOWS; txHandle++) {
        if (gServiceId[txHandle] == serviceId) {
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Error: Duplicate serviceId 0x%x\n", serviceId);
            goto cv2x_init_tx_err2;
        }
    }

    // Find an unused txHandle
    for (txHandle = 0; txHandle < MAX_SPS_FLOWS; txHandle++) {
        if (gServiceId[txHandle] == 0xffffffff) {
            gServiceId[txHandle] = serviceId;
            shm_rsk_ptr->CV2XCnt.serviceID[txHandle] = serviceId;
            break;
        }
    }

    if (txHandle == MAX_SPS_FLOWS) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Error: All Tx SPS flows in use\n");
        goto cv2x_init_tx_err2;
    }
#if defined(EXTRA_DEBUG)
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"txHandle = %d\n", txHandle);
#endif
    // Allow callback functions to know which Tx handle is in use
    gTxHandle = txHandle;

    // Create new Tx SPS flow
    spsInfo.priority                = Priority::PRIORITY_2;
    spsInfo.periodicity             = Periodicity::PERIODICITY_100MS;
    //spsInfo.periodicity             = Periodicity::PERIODICITY_10MS;
    spsInfo.nbytesReserved          = reservedBufLen;
    spsInfo.autoRetransEnabledValid = TRUE;
    spsInfo.autoRetransEnabled      = TRUE;

    if (peakTxPower == 0) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Setting spsInfo.peakTxPowerValid = FALSE\n");
        spsInfo.peakTxPowerValid    = FALSE;
    }
    else {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Setting spsInfo.peakTxPowerValid = TRUE\n");
        spsInfo.peakTxPowerValid    = TRUE;
        spsInfo.peakTxPower         = peakTxPower;
    }
#if defined(EXTRA_DEBUG)
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Calling gCv2xRadio->createTxSpsFlow(TrafficIpType::TRAFFIC_NON_IP, 0x%x, spsInfo, SPS_SRC_PORT_NUM, FALSE, 0, createSpsFlowCallback)\n",
                         serviceId);
#endif
    status = gCv2xRadio->createTxSpsFlow(TrafficIpType::TRAFFIC_NON_IP,
                                         serviceId,
                                         spsInfo,
                                         SPS_SRC_PORT_NUM + txHandle,
                                         TRUE,
                                         SPS_EVT_PORT_NUM + txHandle,
                                         createSpsFlowCallback);

    if (Status::SUCCESS != status) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"gCv2xRadio->createTxSpsFlow() failed: %s\n", statusToString(status));
        goto cv2x_init_tx_err1;
    }

//    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Calling gTxCallbackPromise.get_future().get()\n");
//    errorCode = gTxCallbackPromise.get_future().get();
//    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"errorCode = %s\n", errorCodeToString(errorCode));
//    if (ErrorCode::SUCCESS != errorCode) {
//        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Create Tx SPS flow (%d) failed: %s\n",
//                       txHandle, errorCodeToString(errorCode));
//        goto cv2x_init_tx_err1;
//    }
#endif // xxxxxxxxxxxxxxxxxx_this_not_necessary_on_secton_i_hope_xxxxxxxxxxxxxxxxxxxxxxxxxxxxxx

    // Normal exit
    // Allow other instances of cvx2_init_tx() or cvx2_term_tx() to run
    sem_post(&init_term_tx_sem);
#if defined(EXTRA_DEBUG)
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Returning %d\n", txHandle);
#endif
    return txHandle;

#ifdef CURRENTLY_DONT_NEED_ERR1_20220513
    // Error exit
cv2x_init_tx_err1:
    // If we have a txHandle, indicate this entry is not being used
    gServiceId[txHandle] = 0xffffffff;
    shm_rsk_ptr->CV2XCnt.serviceID[txHandle] = 0xffffffff;
#endif

cv2x_init_tx_err2:
    // Allow other instances of cvx2_init_tx() or cvx2_term_tx() to run
    sem_post(&init_term_tx_sem);
#ifdef EXTRA_CONSOLE_MESSAGES
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Returning -1\n");
#endif

    return -1;
}

/*------------------------------------------------------------------------------
** Function:  serviceIdToTxHandle
** @brief  Convert a service ID into an internal handle, which can be used as
** @brief  an index into various arrays.
** @param  serviceId -- The service ID (PSID or ITS-AID)
** @return The handle corresponding to serviceID. -1 is returned if there is no
**         handle associated with the specified serviceId.
**----------------------------------------------------------------------------*/
static int serviceIdToTxHandle(uint32_t serviceId)
{
    int txHandle;
#if defined(EXTRA_DEBUG)
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"%s(%u)\n", __func__, serviceId);
#endif
    // Search for the handle in the gServiceId table
    for (txHandle = 0; txHandle < MAX_SPS_FLOWS; txHandle++) {
        if (gServiceId[txHandle] == serviceId) {
            break;
        }
    }

    if (txHandle == MAX_SPS_FLOWS) {
        // Indicate the serviceId was not found with a -1 handle
        txHandle = -1;
    }
#if defined(EXTRA_DEBUG)
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Returning %d\n", txHandle);
#endif
    return txHandle;
}


/*------------------------------------------------------------------------------
** Function:  rs_cv2x_tx
** @brief  Submit a packet for transmission via C-V2X.
** @param  serviceId -- [input]The PSID or ITS_AID
** @param  buf       -- [input]Pointer to the bytes to be transmitted
** @param  len       -- [input]The number of bytes to transmit
** @return The number of bytes submitted for transmission for success;
**         -1 for error.
**
** Details: Submits a packet for transmission via C-V2X. The success of failure
**          shown by the return value indicates the success or failure of
**          submitting the packet for transmission. The hardware will attempt
**          to transmit the packet at some time in the future.
**
**          rs_cv2x_init() must have been successfully called with the same
**          serviceId and with tx = 1 before making this call.
**----------------------------------------------------------------------------*/

int rs_cv2x_tx(uint32_t serviceId, uint8_t *data_buf, int32_t data_len, uint32_t psid)
{
    int     txHandle;
    int     rc;
    int32_t i=0;
    int32_t ret = 0;
#if defined(EXTRA_DEBUG)
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"%s(0x%x, %p, %d)\n", __func__, serviceId, data_buf, data_len);
#endif
    // Abort if C-V2X not inited
    if (!cv2xInited) {
        ret = RS_AT_CV2X_NOT_INIT;
        cv2xStatus.error_states |= CV2X_TX_RADIO_NOT_READY_FAIL;
        goto cv2x_tx_err;
    } else {
        cv2xStatus.error_states &= ~CV2X_TX_RADIO_NOT_READY_FAIL;
    }

    // Is SECTON ready?
    //   TODO: The cv2x-example transmitted forever and had this check inside the loop,
    //      so if SECTON wasn't ready, it just tried again the next loop.
    for(i=0;i<MAX_CV2X_IS_READY;i++){
        if (cv2x_tx_is_ready())
            break;
        else
            usleep(1000);
    }

    if(MAX_CV2X_IS_READY <= i) {
        ret = RS_AT_CV2X_TX_NOT_READY;
        cv2xStatus.error_states |= CV2X_TX_NOT_READY;
        goto cv2x_tx_err;
    } else {
        cv2xStatus.error_states &= ~CV2X_TX_NOT_READY;
    }

    // Get our internal handle, which lets us index to the correct cv2x tx socket ptr
    txHandle = serviceIdToTxHandle(serviceId);
    if (txHandle < 0) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Invalid serviceId (0x%x)\n", serviceId);
        ret = RS_AT_CV2X_BAD_SERVICE_ID;
        cv2xStatus.error_states |= CV2X_SERVICE_ID_BAD;
        goto cv2x_tx_err;
    } else {
        cv2xStatus.error_states &= ~CV2X_SERVICE_ID_BAD;
    }
#if defined(EXTRA_DEBUG)
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"txHandle = %d\n", txHandle);
#endif
    // Send via AutoTalks.  This block replaces the DSRC sending code block ==

    uint64_t time64;
    uint64_t ddm_accuracy = 0;
    ddm_tsf_lock_status_t ddm_tsf_lock_status;

    // Note: AT max is CV2X_PPPP_7
    // Policy set: bsm/eva == 2, map == 3, spat == 5, SSM = 6 , TIM == 7 , TCP/UDP == 8
    switch(psid) {
        case 0x20:   //BSM
        case 0x21:   //EVA? wild guess.
        case 0x26:   //Misbehavior report maybe 4?
        case 0x27:   //PSM
            reg_policy.priority = CV2X_PPPP_2;
            break;
        case 0x8000: //RTCM
        case 0x8002: //SPAT
            reg_policy.priority = CV2X_PPPP_5;
            break;
        case 0x8003: //TIM or RSM
        case 0x8007: //WSA
            reg_policy.priority = CV2X_PPPP_7;
            break;
        case 0xE0000015: //SSM
        case 0xE0000016: //SRM
            reg_policy.priority = CV2X_PPPP_6;
            break;
        case 0xE0000017: //MAP
            reg_policy.priority = CV2X_PPPP_3;
            break;
        case 0xE0000019: //RWA
            reg_policy.priority = CV2X_PPPP_7;
            break;
        default:   //UDP, TCP, anything else we don't know of.
            reg_policy.priority = CV2X_PPPP_7;
            break;
    }
    rc = cv2x_socket_policy_set(cv2x_socket_ptr, &reg_policy);
    if (atlk_error(rc)) {
        cv2xStatus.error_states |= CV2X_TX_SOCKET_POLICY_FAIL;
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Setting SP Socket Policy...ERROR: cv2x_socket_policy_set fail. %s\n", atlk_rc_to_str(rc));
        ret = CV2X_TX_SOCKET_POLICY_FAIL;
        goto cv2x_tx_err;
    }

    // TODO: Remove this block if we don't want to use debug info in metrics
    /* Retrieving time is not part of the flow, just for debug sake */
    rc = ddm_tsf_get(ddm_service_ptr, (uint64_t *)&time64, &ddm_accuracy, &ddm_tsf_lock_status);
    if (atlk_error(rc)) {
        cv2xStatus.error_states |= CV2X_DDM_TSF_GET_FAIL;
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"ddm_tsf_get fail. %s\n", atlk_rc_to_str(rc));
    } else {
        cv2xStatus.error_states &= ~CV2X_DDM_TSF_GET_FAIL;
    }

    // Increment message_id to new value for sending next message
    cv2x_send_params.message_id++;
#if defined(EXTRA_DEBUG)
    /* Send the packet */
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Sending SP message (id = %u)... length %d bytes ... \n",
             cv2x_send_params.message_id, data_len);
#endif
    // Autotalks API call
    rc = cv2x_send(cv2x_tx_socket_ptr[txHandle], data_buf, data_len, &cv2x_send_params);
    if (atlk_error(rc)) {
        ret = RS_AT_CV2X_SEND_FAIL;
        cv2xStatus.error_states |= CV2X_TX_FAIL;
        goto cv2x_tx_err; // retry?
    } else {
        cv2xStatus.tx_count++;
        cv2xStatus.error_states &= ~CV2X_TX_FAIL;
        currenttallies.tx_good_bytes +=data_len;
    }

    // END - Send via AutoTalks. This block replaces the DSRC sending code block ==
    // Return success  // TODO: Is is possible to retrieve actual bytes send?
    return data_len;

cv2x_tx_err:
    cv2xStatus.tx_err_count++;
    return ret;
}

/*------------------------------------------------------------------------------
** Function:  rs_cv2x_rx
** @brief  Wait for a packet to be received via C-V2X.
** @param  buf       -- Pointer to a buffer to hold the received packet
** @param  len       -- The maximum number of bytes that can be received
** @return The number of bytes received; -1 for error.
**
** Details: This call is blocking. Waits for a packet to be received via C-V2X.
**          The packet will be stored in the buffer pointed to by buf. The
**          number of bytes received is returned.
**
**          cv2x_init() must have been successfully called before making this
**          call.
**          If needed, time of rec available as (rx_params.receive_time)
**----------------------------------------------------------------------------*/;

int rs_cv2x_rx(uint8_t *cv2x_rx_buffer, uint32_t maxLen, uint32_t rx_timeout_usec, uint32_t * ppp)
{
    int32_t bytes_received = -1;
    int32_t i;

#if defined(EXTRA_DEBUG)
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"%s(%p, %d)\n", __func__, cv2x_rx_buffer, maxLen);
#endif

// xxxxxxxxx COPIED FROM cv2x-example - cv2x_rx_mode() RX THREAD CODE xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx

    {
      atlk_rc_t            rc;

        cv2x_receive_params_t rx_params = CV2X_RECEIVE_PARAMS_INIT;
        size_t                rx_buffer_size;
        atlk_wait_t           receive_timeout = {
                                                    .wait_type = ATLK_WAIT_TYPE_INTERVAL,
                                                    .wait_usec = rx_timeout_usec
                                                };
        int32_t rssi_rx_max = -999; /* Seed accordingly.*/

        rx_buffer_size = maxLen;// Converts from int to size_t; fixable if we change cv2x_rx api to size_t  TODO: Do this [20220518]

        /* Wait for message. NOTE: doesn't wait infinitely, see above recieve_timeout var */
        rc = cv2x_receive(cv2x_rx_socket_ptr, cv2x_rx_buffer, &rx_buffer_size, &rx_params, &receive_timeout);
        if (atlk_error(rc)) {
            if (rc != ATLK_E_TIMEOUT) {
                cv2xStatus.rx_err_count++;
                cv2xStatus.error_states |= CV2X_RX_FAIL;
            } else {
                bytes_received = 0; /* Timeout, we have no data. */
            }
        } else {
#if defined(EXTRA_DEBUG)
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"received from RX socket... got %lu bytes, rc = %d\n", rx_buffer_size, rc);
#endif
            if(NULL != ppp) {
                *ppp = rx_params.pppp;
            }
            currenttallies.rx_good_bytes +=rx_buffer_size;
            bytes_received = rx_buffer_size;
            cv2xStatus.rx_count++;
            cv2xStatus.error_states &= ~CV2X_RX_FAIL; /* Clear any fail. */
            /* Grabs last rx highest rssi. */
            rssi_rx_max = -999; /* Seed accordingly. */
            for(i=0;i<ATLK_INTERFACES_MAX;i++) {
                if(   (CV2X_LMAC_MEASUREMENT_RSSI_MIN_THRESHOLD  <= (rx_params.rssi[i]/8))
                   && ((rx_params.rssi[i]/8) <= CV2X_LMAC_MEASUREMENT_RSSI_MAX_THRESHOLD)) {
                    if(rssi_rx_max < (rx_params.rssi[i]/8)){
                        rssi_rx_max = rx_params.rssi[i]/8;
                    }
                }
            }
            currenttallies.rssi = rssi_rx_max; /* Good or bad update. */
        }
    }
// xxENDxxxx COPIED FROM cv2x-example - cv2x_rx_mode() RX THREAD CODE xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
#if defined(EXTRA_DEBUG)
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"returning %d\n", bytes_received);
#endif
    return bytes_received;
}



/*------------------------------------------------------------------------------
** Function:  rs_cv2x_term_tx
** @brief  Terminate a C-V2X Tx communication.
** @param  serviceId -- The PSID or ITS_AID for the Tx we want to terminate
** @return 0 for success; -1 for error
**
** Details: Makes sure that the serviceId was registered for Tx. Then close
**          the associated Tx flow.
**
**          The entry in the gServiceId array that contained the service Id is
**          set to 0xffffffff to indicate it is no longer in use.
**----------------------------------------------------------------------------*/
int rs_cv2x_term_tx(uint32_t serviceId)
{
    // NOTE: In cv2x-example.c, there is zero AutoTalks TX shutdown
    //       So, just reversing what we did in cv2x_init_tx()
    int txHandle;

    // Get our internal handle, which lets us index to the correct cv2x tx socket ptr
    txHandle = serviceIdToTxHandle(serviceId);

    if (txHandle < 0) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"cv2x_term_tx() failed: passed serviceId=%u is unknown!\n", serviceId);
        return -1;
    }

    // Delete the socket
    if (cv2x_socket_delete(cv2x_tx_socket_ptr[txHandle]) != ATLK_OK) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Delete cv2x socket for txHandle=%d serviceId=%u failed!\n", txHandle, serviceId);
        return -1;
    }

    // Mark the freed entries as available
    gServiceId[txHandle] = 0xffffffff;
    cv2x_tx_socket_ptr[txHandle] = NULL;
    shm_rsk_ptr->CV2XCnt.serviceID[txHandle] = 0xffffffff;

    return 0;
}

/*------------------------------------------------------------------------------
** Function:  rs_cv2x_term
** @brief  Terminate C-V2X communication.
** @return 0 for success; -1 for error
**
** Details: Terminates communication for the C-V2X radio initialized by a
**          previous rs_cv2x_init() call.
**
**          The thread that repeatedly calls rs_cv2x_rx() should be terminated
**          before making the call to this function.
**
**          The Tx flow for any entry in the gServiceId table is terminated.
**          Then the Rx subscription is terminated.
**
**          This function must be called before a program terminates, or else
**          subsequent calls to rs_cv2x_init() will crash with a core dump.
**----------------------------------------------------------------------------*/
// NOTE: This shuts down cv2x RX then shuts down common cv2x, ie opposite of rs_cv2x_init()'s work [20220512]
int rs_cv2x_term(void)
{
    atlk_rc_t rc;
#if defined(EXTRA_DEBUG)
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"%s()\n", __func__);
#endif
    if (!cv2xInited) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Returning 0\n");
        return 0;
    }

    // NOTE: The de-register of all Tx flows is done in rs_cv2x_module_term()

    // Close Rx socket
    rc = cv2x_socket_delete(cv2x_rx_socket_ptr);
    if (rc != ATLK_OK) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Error deleting RX socket - %s\n", atlk_rc_to_str(rc));
        return -1;
    }

    cv2xInited = FALSE;
    shm_rsk_ptr->CV2XCnt.cv2xInited = 0;
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"cv2xInited = FALSE, ret=0\n");
    return 0;
}

/*------------------------------------------------------------------------------
** Function:  rs_cv2x_get_radio_tallies
** @brief  Get C-V2X radio tallies
** @return 0 for success; -1 for error
**
** Details: Fits the cv2x stats into our tallies structure
**----------------------------------------------------------------------------*/
int rs_cv2x_get_radio_tallies(uint8_t radioNum, DeviceTallyType *tallies)
{
    int i;
    atlk_rc_t rc; 
    cv2x_rx_stats_t cv2x_rx_stats;
    cv2x_socket_stats_t cv2x_socket_stats;
#if defined(EXTRA_DEBUG)
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"cv2x_get_radio_tallies(%u, %p)\n", radioNum, tallies);
#endif
    memset(tallies, 0, sizeof(*tallies));
    // Get receive stats
    rc = cv2x_rx_stats_get(cv2x_service_ptr, &cv2x_rx_stats);   // Note needs service ptr not rx_socket_ptr
    if (rc == ATLK_OK) {
        // Store receive tallies - fit the cv2x_rx_stats_t's data into tallies fields
        // Fields listed in cv2x-craton2-sdk-5.16.0-rel/docs/html/cv2x__service_8h.html
        tallies->rx_good = cv2x_rx_stats.l2_messages_received_successfully;
        tallies->rx_err =    /* rx errors */
              cv2x_rx_stats.l2_messages_dropped_crc_errors
            + cv2x_rx_stats.l2_messages_dropped_queue_full
            + cv2x_rx_stats.l2_messages_dropped_exceeding_max_rx_size
            + cv2x_rx_stats.rlc_messages_reassembly_timed_out
            + cv2x_rx_stats.rlc_messages_duplicated
            + cv2x_rx_stats.pdcp_messages_dropped_parsing_error
            + cv2x_rx_stats.svc_messages_dropped
            + cv2x_rx_stats.mac_messages_dropped_invalid_destination_l2id
            + cv2x_rx_stats.mac_messages_dropped_queue_full
            + cv2x_rx_stats.mac_messages_dropped_invalid_struct
            + cv2x_rx_stats.mac_messages_dropped_invalid_version;
        /* Populate the Autotalks-specific tallies */
        tallies->cv2x_rx_l2_messages_received_successfully =
                cv2x_rx_stats.l2_messages_received_successfully;
        tallies->cv2x_rx_rlc_messages_reassembled_successfully =
                cv2x_rx_stats.rlc_messages_reassembled_successfully;
        tallies->cv2x_rx_pdcp_messages_received_successfully =
                cv2x_rx_stats.pdcp_messages_received_successfully;
        tallies->cv2x_rx_svc_messages_received_successfully =
                cv2x_rx_stats.svc_messages_received_successfully;
        tallies->cv2x_rx_mac_messages_received_successfully =
                cv2x_rx_stats.mac_messages_received_successfully;
        tallies->cv2x_rx_l2_messages_dropped_crc_errors =
              cv2x_rx_stats.l2_messages_dropped_crc_errors;
        tallies->cv2x_rx_l2_messages_dropped_queue_full =
              cv2x_rx_stats.l2_messages_dropped_queue_full;
        tallies->cv2x_rx_l2_messages_dropped_exceeding_max_rx_size =
              cv2x_rx_stats.l2_messages_dropped_exceeding_max_rx_size;
        tallies->cv2x_rx_rlc_messages_reassembly_timed_out =
              cv2x_rx_stats.rlc_messages_reassembly_timed_out;
        tallies->cv2x_rx_rlc_messages_duplicated =
              cv2x_rx_stats.rlc_messages_duplicated;
        tallies->cv2x_rx_pdcp_messages_dropped_parsing_error =
              cv2x_rx_stats.pdcp_messages_dropped_parsing_error;
        tallies->cv2x_rx_svc_messages_dropped =
              cv2x_rx_stats.svc_messages_dropped;
        tallies->cv2x_rx_mac_messages_dropped_invalid_destination_l2id =
              cv2x_rx_stats.mac_messages_dropped_invalid_destination_l2id;
        tallies->cv2x_rx_mac_messages_dropped_queue_full =
              cv2x_rx_stats.mac_messages_dropped_queue_full;
        tallies->cv2x_rx_mac_messages_dropped_invalid_struct =
              cv2x_rx_stats.mac_messages_dropped_invalid_struct;
        tallies->cv2x_rx_mac_messages_dropped_invalid_version =
              cv2x_rx_stats.mac_messages_dropped_invalid_version;
        tallies->rssi = currenttallies.rssi;
        tallies->rx_good_bytes = currenttallies.rx_good_bytes;
    } else {
        cv2xStatus.error_states |= CV2X_GET_TALLIES_FAIL;
    }
    // Get tx stats
    for (i=0; i<MAX_SPS_FLOWS; i++) {
        if (cv2x_tx_socket_ptr[i] != NULL) {
            rc = cv2x_socket_stats_get(cv2x_service_ptr, cv2x_tx_socket_ptr[i], 0 /*0==alltime*/, &cv2x_socket_stats);
            if (rc == ATLK_OK) {
                // Store xmit tallies - fit the cv2x_socket_stats's data into tallies fields
                // Fields listed in cv2x-craton2-sdk-5.16.0-rel/docs/html/structcv2x__socket__stats__st.html
                tallies->tx_good += cv2x_socket_stats.messages_sent_success; /* good Xmits */
                tallies->tx_err += cv2x_socket_stats.messages_sent_failures; /* tx errors */
                tallies->tx_drop += cv2x_socket_stats.messages_dropped; /* tx packet dropped */
            }
        }
    }
    tallies->tx_power = currenttallies.tx_power/4; /* GUI divies by 2 already */
    tallies->tx_good_bytes += currenttallies.tx_good_bytes;
    return 0;
}
/*------------------------------------------------------------------------------
** Function:  ns_get_cv2x_status
** @brief  Get C-V2X status: ready, interface state, firmware revs ok, temperature.
** @return 0 for success; -1 for error
**
** Details: Allows apps to query CV2X status using API at runtime.
**----------------------------------------------------------------------------*/
extern bool_t cv2x_module_inited;

int32_t ns_get_cv2x_status(rskStatusType * cv2xStatusCopy)
{
  int32_t ret = -1;

  if(NULL != cv2xStatusCopy){
      if((1 == cv2xStatus.ready) && (cv2x_module_inited)) {
          cv2xStatus.temperature = getCv2xTemp();
          /* See it tx is ready. */
          if (cv2x_tx_is_ready()) {
              cv2xStatus.error_states &= ~CV2X_TX_NOT_READY;
          } else {
              cv2xStatus.error_states |= CV2X_TX_NOT_READY;
          }
          if(NULL != shm_rsk_ptr){

              /* Dump alsmi stats */
              cv2xStatus.SignRequests = shm_rsk_ptr->ALSMICnt.SignRequests;
              cv2xStatus.SignSuccesses = shm_rsk_ptr->ALSMICnt.SignSuccesses;
              cv2xStatus.SignFailures = shm_rsk_ptr->ALSMICnt.SignFailures;
              cv2xStatus.SignCBFailures = shm_rsk_ptr->ALSMICnt.SignCBFailures;
              cv2xStatus.LastSignErrorCode = shm_rsk_ptr->ALSMICnt.LastSignErrorCode;
              cv2xStatus.StripSuccesses = shm_rsk_ptr->ALSMICnt.StripSuccesses;
              cv2xStatus.StripFailures = shm_rsk_ptr->ALSMICnt.StripFailures;
              cv2xStatus.VerifyRequests = shm_rsk_ptr->ALSMICnt.VerifyRequests;
              cv2xStatus.VerifySuccesses = shm_rsk_ptr->ALSMICnt.VerifySuccesses;
              cv2xStatus.VerifyFailures = shm_rsk_ptr->ALSMICnt.VerifyFailures;
              cv2xStatus.VerifyCBFailures = shm_rsk_ptr->ALSMICnt.VerifyCBFailures;
              cv2xStatus.LastVerifyErrorCode = shm_rsk_ptr->ALSMICnt.LastVerifyErrorCode;
              cv2xStatus.P2PCallbackCalled = shm_rsk_ptr->P2PCnt.P2PCallbackCalled;
              cv2xStatus.P2PCallbackSendSuccess = shm_rsk_ptr->P2PCnt.P2PCallbackSendSuccess;
              cv2xStatus.P2PCallbackNoRadioConfigured = shm_rsk_ptr->P2PCnt.P2PCallbackNoRadioConfigured;
              cv2xStatus.P2PCallbackLengthError = shm_rsk_ptr->P2PCnt.P2PCallbackLengthError;
              cv2xStatus.P2PCallbackSendError = shm_rsk_ptr->P2PCnt.P2PCallbackSendError;
              cv2xStatus.P2PReceived = shm_rsk_ptr->P2PCnt.P2PReceived;
              cv2xStatus.P2PProcessed = shm_rsk_ptr->P2PCnt.P2PProcessed;
              cv2xStatus.P2PProcessError = shm_rsk_ptr->P2PCnt.P2PProcessError;

              /* Dump WSM/WSA handling events. Don't care about counters just if it happended. */
              if(shm_rsk_ptr->WSMError.WSMLengthInvalid) {
                  cv2xStatus.wsm_error_states |= WSM_LENGTH_FAIL;
              }
              if(shm_rsk_ptr->WSMError.WSMLengthMismatch){
                  cv2xStatus.wsm_error_states |= WSM_LENGTH_MISMATCH_FAIL;
              }
              if(shm_rsk_ptr->WSMError.WSMWrongVersion){
                  cv2xStatus.wsm_error_states |= WSM_VERSION_FAIL;
              }
              if(shm_rsk_ptr->WSMError.PSIDNotRegistered){
                  cv2xStatus.wsm_error_states |= WSM_PSID_NOT_REG_FAIL;
              }
              if(shm_rsk_ptr->WSMError.PSIDEncodingFailure){
                  cv2xStatus.wsm_error_states |= WSM_PSID_ENCODING_FAIL;
              }
              if(shm_rsk_ptr->WSMError.WSMUnknownElementId){
                  cv2xStatus.wsm_error_states |= WSM_UNK_ELEMENT_ID_FAIL;
              }
              if(shm_rsk_ptr->WSMError.WSMStripError){
                  cv2xStatus.wsm_error_states |= WSM_STRIP_FAIL;
              }
              if(shm_rsk_ptr->WSMError.WSMVerifyOnDemandError){
                  cv2xStatus.wsm_error_states |= WSM_VOD_FAIL;
              }
              if(shm_rsk_ptr->WSMError.WSMVerifyThenProcessError){
                  cv2xStatus.wsm_error_states |= WSM_VTP_FAIL;
              }
              if(shm_rsk_ptr->WSMError.WSMUnknownTPID){
                  cv2xStatus.wsm_error_states |= WSM_UNK_TPID_FAIL;
              }
              if(shm_rsk_ptr->WSMError.WSMPrematureEndOfPacket){
                  cv2xStatus.wsm_error_states |= WSM_PEOP_FAIL;
              }
              if(shm_rsk_ptr->WSMError.WSMUnsecuredDropped){
                  cv2xStatus.wsm_error_states |= WSM_UNSEC_DROP_FAIL;
              }
              if(shm_rsk_ptr->WSMError.WSMVerificationFailedDropped){
                  cv2xStatus.wsm_error_states |= WSM_VFD_FAIL;
              }
              if(shm_rsk_ptr->WSMError.WSMNo1609p2HeaderDropped){
                  cv2xStatus.wsm_error_states |= WSM_1609_P2H_DROP_FAIL;
              }
              if(shm_rsk_ptr->WSACnt.WSAParseError){
                  cv2xStatus.wsm_error_states |= WSM_PARSE_FAIL;
              }
              if(shm_rsk_ptr->WSACnt.WSAProcessError){
                  cv2xStatus.wsm_error_states |= WSA_PROCESS_FAIL;
              }
              if(shm_rsk_ptr->WSACnt.WSALengthInvalid){
                  cv2xStatus.wsm_error_states |= WSA_LENGTH_FAIL; 
              }
              if(shm_rsk_ptr->WSACnt.WSAChannelAccessInvalid){
                  cv2xStatus.wsm_error_states |= WSA_CH_ACCESS_FAIL;
              }
              if(shm_rsk_ptr->WSACnt.WSAPsidInvalid){
                  cv2xStatus.wsm_error_states |= WSA_PSID_FAIL;
              }
              if(shm_rsk_ptr->WSACnt.WSAWrongVersion){
                  cv2xStatus.wsm_error_states |= WSA_VERSION_FAIL;
              }
              if(shm_rsk_ptr->WSACnt.WSAServiceCountExceeded){
                  cv2xStatus.wsm_error_states |= WSA_SRV_CNT_EXCEED_FAIL;
              }
              if(shm_rsk_ptr->WSACnt.WSAChannelCountInvalid){
                  cv2xStatus.wsm_error_states |= WSA_CH_CNT_IVALID_FAIL;
              }
              if(shm_rsk_ptr->WSACnt.WSAChannelLengthMismatch){
                  cv2xStatus.wsm_error_states |= WSA_CH_LEN_MISMATCH_FAIL;
              }
              if(shm_rsk_ptr->WSACnt.WSASecurityInfoStripFailed){
                  cv2xStatus.wsm_error_states |= WSA_SEC_INFO_STRIP_FAIL;
              }
              if(shm_rsk_ptr->WSACnt.WSASemaphoreError){
                  cv2xStatus.wsm_error_states |= WSA_SEM_FAIL;
              }
              if(shm_rsk_ptr->WSACnt.WSAVerifySemaphoreError){
                  cv2xStatus.wsm_error_states |= WSA_VER_SEM_FAIL;
              }
              if(shm_rsk_ptr->WSACnt.WSANoAvailableWSAStatusTableEntry){
                  cv2xStatus.wsm_error_states |= WSA_ST_ENTRY_FAIL;
              }
              if(shm_rsk_ptr->WSACnt.WSAVerifyFailed){
                  cv2xStatus.wsm_error_states |= WSA_VERIFY_FAIL;
              }
              if(shm_rsk_ptr->WSACnt.WSASecurityVerifyInvalidHandle){
                  cv2xStatus.wsm_error_states |= WSA_SEC_VERIFY_HANDLE_FAIL;
              }
              if(shm_rsk_ptr->WSACnt.WSASecurityVerifySignatureFailed){
                  cv2xStatus.wsm_error_states |= WSA_SEC_VER_SIG_FAIL;
              }
              if(shm_rsk_ptr->WSACnt.WSASecurityUnsupportedSecurityType){
                  cv2xStatus.wsm_error_states |= WSA_SEC_TYPE_FAIL;
              }              
          }
      }
      memcpy(cv2xStatusCopy,&cv2xStatus,sizeof(rskStatusType));
      ret = 0;
  }
  return ret;
}
