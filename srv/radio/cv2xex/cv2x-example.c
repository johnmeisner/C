/**
   AUTOTALKS PROPRIETARY AND CONFIDENTIAL

   C-V2X Sample Application

   - This application should be executed on two EVK units, one in Tx mode and the other in Rx mode.
   - The application links with the C-V2X library in order to access the C-V2X MAC, RLC PDCP and PHY layers.
   - For more information, please refer to CRATON2-SECTON-CV2X-Application-Notes.
*/

#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <limits.h>
#include <errno.h>
#include <net/if.h>
#include <sys/socket.h>

#include <common.h>
#include <atlk/sdk.h>
#include <atlk/ddm.h>
#include <atlk/ddm_service.h>
#include <atlk/wdm.h>
#include <atlk/wdm_service.h>
#include <atlk/v2x_service.h>
#include <atlk/cv2x.h>
#include <atlk/cv2x_service.h>
#include <extern/time_sync.h>
#include <poti_api.h>
#include <extern/ref_cv2x_sys.h>

#include "verstring.h"
#include <arpa/inet.h>

#define MAX_BUFF_SIZE     300
/*We need 6 byte to store ACME header*/
#define MAX_ACME_MSG_SIZE     MAX_BUFF_SIZE-6
#define SEND_MSG_FILE_PATH "/home/root/sendMsg"
static char g_payload2Send[MAX_ACME_MSG_SIZE] = {0};
static int isAcme = 0;

static bool ad_hoc_mode = false;
static int32_t iterations_count_g = -1;

/** Unit scale conversion factors */
#define NANO_PER_MICRO  1000
#define NANO_PER_MILLI  1000000
#define NANO_PER_UNIT   1000000000

#define RSVP_MILLIS     100

void deserialize_acme_message_and_print(uint8_t *buf, uint16_t buf_len, uint32_t delta)
{
    uint8_t *buf_ptr = buf;
    uint16_t buf_total_len = buf_len;
    uint16_t len_info = 0;

    // Skip 1 Bytes Family ID
    if (buf_len) {
        buf_ptr++;
        buf_len--;
    }
    else{
      printf("RX recevie nothing, buf_len is: %u\n", buf_len);
      return;
    }

    // Get 2 Byte for Non-dummy Payload length
    if (buf_len >= sizeof(uint16_t)) {
        len_info = (ntohs(*(uint16_t*)buf_ptr));
        // At least 3 byte (1 bytes family ID + 2 bytes length information)
        if (len_info <= sizeof(unsigned char) + sizeof(uint16_t) ||
            len_info > buf_total_len) 
        {
            printf("RX filled length is invalid: %u\n", len_info);
            return;
        }
    }
  int msg_len = (int)buf_total_len - (int)len_info;
  char msg_buf[msg_len + 1];
  char *msg_buf_ptr = msg_buf;

  //Shift to Message Address
  buf_ptr = buf;
  buf_ptr += (int)len_info;

  memcpy(msg_buf_ptr, buf_ptr, msg_len);
  msg_buf[msg_len] = '\0';
  
  printf("______cv2x-example_______________\n"
        "**********Received ACME message: ********\n"
        "%s\n"
        "delta=%u\n"
        , msg_buf, delta);

  return;
}

int serialize_acme_message(uint8_t *buf, char* sendPayloadBuf, uint16_t seq_num, uint16_t buf_len)
{
    uint8_t *buf_ptr;
    uint8_t *size_ptr = NULL;
    buf_ptr = buf;
    uint16_t total_buf__len = buf_len;
    int isErr = 0;

    /* Store 1 Byte V2X Family ID */
    if (buf_len) {
        *buf_ptr++ = 'W'; 
        buf_len--;
    } else {
      isErr = 1;
    }

    /* 2 bytes for store non-dummy information data length */
    if (buf_len >= sizeof(uint16_t)) {
        size_ptr = buf_ptr;
        buf_ptr += sizeof(uint16_t);
        buf_len -= sizeof(uint16_t);
    } else {
      isErr = 1;
    }
    
    /* 1 Byte UE Equipment ID.*/
    if (buf_len) {
        *buf_ptr++ = (uint8_t) 1;
        buf_len--;
    } else {
      isErr = 1;
    }

    /* 2 byte seq number */
    
    if (buf_len >= sizeof(uint16_t)) {
        uint16_t short_seq_num = htons(seq_num);
        memcpy(buf_ptr, (unsigned char *)&short_seq_num, sizeof(uint16_t));
        buf_ptr += (sizeof(uint16_t));
        buf_len -= (sizeof(uint16_t));
    } else {
      isErr = 1;
    }

    /* FIll non-dummy information data length here */
    if (size_ptr) {
        uint16_t len_info = total_buf__len - buf_len;
        uint16_t len = htons(len_info);
        memcpy(size_ptr, (unsigned char *)&len, sizeof(uint16_t));
    } else {
      isErr = 1;
    }

    if(isErr){
      printf("Buffer length is too small. Total buffer size: %u\n", total_buf__len);
    } else {
    /* Append message in send buffer  */
      strncpy((char*)buf_ptr,sendPayloadBuf, total_buf__len);
    }

  return isErr;
}
/*
  Tx thread - transmit mode thread (logical thread)
  This thread creates a transmit socket and periodically (every 100ms) transmits a messages on the socket.
  The socket may be of SPS type or ad-hoc type, depending on the execution mode of the application.
  For SPS socket type, the function also defines the transmit policy.
  Timestamp and message-ID is pronted for each received message.
  Parameters:
  - [IN] arg             : cv2x_service_t pointer
  - [global] ad_hoc_mode : indicates whether the application is executed in ad-hoc mode (otherwise SPS mode is used)
*/
void *
cv2x_tx_mode(void *arg)
{
  cv2x_socket_t         *cv2x_socket_ptr;
  cv2x_socket_config_t  cv2x_socket_config = CV2X_SOCKET_CONFIG_INIT;
  ddm_service_t         *ddm_service_ptr   = NULL;
  cv2x_socket_policy_t  reg_policy;
  cv2x_service_t        *cv2x_service_ptr;
  atlk_rc_t             rc;

  char *msg_format = "cur time = %llu, cv2x example message %d!";
  uint16_t tx_seq_num = 0;
  cv2x_service_ptr = (cv2x_service_t *) arg;

  int                      counter          = 0;
  cv2x_send_params_t       cv2x_send_params = CV2X_SEND_PARAMS_INIT;
  cv2x_adhoc_send_params_t cv2x_adhoc_param = CV2X_ADHOC_SEND_PARAMS_INIT;

  cv2x_service_ptr = (cv2x_service_t *) arg;

  printf("\n______cv2x-example - TX started_______________\n");
  printf("[cv2x-example TX] getting DDM service ptr...\n");

  rc = ddm_service_get(NULL, &ddm_service_ptr);
  if (atlk_error(rc)) {
    (void)fprintf(stderr, "ddm_service_get failed: %d\n", rc);
    return NULL;
  }

  /* Socket creation */
  if (ad_hoc_mode != false) {
    printf("[cv2x-example] creating AD HOC Socket...\n");

    // Create Tx Ad-Hoc socket
    rc = cv2x_socket_create(cv2x_service_ptr, CV2X_SOCKET_TYPE_AD_HOC_TX, &cv2x_socket_config, &cv2x_socket_ptr);
    if (atlk_error(rc)) {
      printf("[cv2x-example] creating AD HOC Socket...ERROR: cv2x_socket_create fail. %s\n", atlk_rc_to_str(rc));
      return NULL;
    }

    printf("[cv2x-example] creating AD HOC Socket...OK\n");

  }
  else {
    printf("[cv2x-example] creating SP Socket...\n");

    // Create Tx SPS socket
    rc = cv2x_socket_create(cv2x_service_ptr,
                            CV2X_SOCKET_TYPE_SEMI_PERSISTENT_TX,
                            &cv2x_socket_config,
                            &cv2x_socket_ptr);
    if (atlk_error(rc)) {
      printf("[cv2x-example] creating SP Socket...ERROR: cv2x_socket_create fail. %s\n", atlk_rc_to_str(rc));
      return NULL;
    }

    printf("[cv2x-example] creating SP Socket...OK\n");

    reg_policy.control_interval_ms = RSVP_MILLIS;

    // Ensure that we have enough space after formatting (snprintf which is done later)
    if(isAcme)
    {
      reg_policy.size = 6+strlen(g_payload2Send)+1;
    } else {
      reg_policy.size     = strlen(msg_format) + 40;
    }

    reg_policy.priority = CV2X_PPPP_1;

    printf("[cv2x-example] Setting SP Socket Policy...\n");

    // Policy set
    rc = cv2x_socket_policy_set(cv2x_socket_ptr, &reg_policy);
    if (atlk_error(rc)) {
      printf("[cv2x-example] Setting SP Socket Policy...ERROR: cv2x_socket_policy_set fail. %s\n", atlk_rc_to_str(rc));
      return NULL;
    }

    printf("[cv2x-example] Setting SP Socket Policy...OK\n");
  }

  /* We need to preserve monotonic time (which can not be set by other applications)
     for the purpose of "time_sync_non_drifting_sleep" */
  struct timespec curr_monotonic_time;

  if (clock_gettime(CLOCK_MONOTONIC, &curr_monotonic_time) != 0) {
    fprintf(stderr, "clock_gettime failed\n");
    return NULL;
  }

  while (1) {

    if (cv2x_tx_is_ready()) {

      uint8_t msg[MAX_BUFF_SIZE]= {0};
      char print_msg[MAX_BUFF_SIZE]= {0};
      uint64_t time64;
      uint64_t ddm_accuracy = 0;
      ddm_tsf_lock_status_t ddm_tsf_lock_status;
	  int msg_len = 0;
      if ( (iterations_count_g >= 0) && (iterations_count_g <= counter) ) {
        break;
      }
      counter++;

      /* Retrieving time is not part of the flow, just for debug sake */
      rc = ddm_tsf_get(ddm_service_ptr, (uint64_t *)&time64, &ddm_accuracy, &ddm_tsf_lock_status);
      if (atlk_error(rc)) {
        printf("[cv2x-example] ddm_tsf_get fail. %s\n", atlk_rc_to_str(rc));
      }

      if(isAcme){
        int serialize_fail = 0;
        serialize_fail = serialize_acme_message(msg, g_payload2Send, tx_seq_num++, (uint16_t)strlen(g_payload2Send) );
        if(serialize_fail == 0){
        /* ACME packet has '\0' in non-dummy date, we can't use "strlen(msg)" directly.*/
          msg_len = 6 + strlen(g_payload2Send);
        /*Skip ACME 6 byte ACME header, only shows payload*/
          snprintf(print_msg, sizeof(print_msg)-1, "[cv2x-example] - send payload message - %s\n", msg+6);
        } else {
           return NULL;
        }
      } else {
        snprintf((char *)msg, sizeof(msg), msg_format, time64, counter);
        snprintf(print_msg, sizeof(print_msg)-1, "[cv2x-example] - send - %s\n", msg);
        msg_len = strlen((char *)msg);
      }
        printf("%s",print_msg);

      cv2x_send_params.message_id++;
      cv2x_adhoc_param.message_id++;

      /* Send the packet */
      if (ad_hoc_mode) {
        printf("[cv2x-example] Sending AD HOC message (id = %u)... length %d bytes ... \n",
               cv2x_adhoc_param.message_id, (int)strlen((char *)msg) + 1);

        rc = cv2x_adhoc_send(cv2x_socket_ptr, msg, strlen((char *)msg), CV2X_PPPP_1, &cv2x_adhoc_param);
      }
      else {
        printf("[cv2x-example] Sending SP message (id = %u)... length %d bytes ... \n",
               cv2x_send_params.message_id, msg_len + 1);

        rc = cv2x_send(cv2x_socket_ptr, msg, msg_len, &cv2x_send_params);
      }

      if (atlk_error(rc)) {
        printf("[cv2x-example] Sending message...ERROR: cv2x_send fail. %s\n", atlk_rc_to_str(rc));
      }
      else {
        printf("[cv2x-example] Sending message...OK\n");
      }
    }
    else {
      printf("[cv2x-example] Not ready for Tx!\n");
    }

    rc = time_sync_non_drifting_sleep(&curr_monotonic_time, RSVP_MILLIS * NANO_PER_MILLI);
    if (atlk_error(rc)) {
      printf("[cv2x-example] Sending message...ERROR: time_sync_non_drifting_sleep fail. %s\n", atlk_rc_to_str(rc));
    }
  } // while

  return NULL;
}

/*
  Rx thread - receive mode thread (logical thread)
  This thread creates a receive socket and waits for messages to be available.
  Each received message is printed, as well as the time difference between the transmission
  (by transmitting application on another unit) and reception.

  Parameters:
  - [IN] arg: cv2x_service_t pointer
**/
void *
cv2x_rx_mode(void *arg)
{
  atlk_rc_t            rc;
  cv2x_service_t       *cv2x_service_ptr;
  cv2x_socket_t        *cv2x_socket_ptr;
  cv2x_socket_config_t socket_config = CV2X_SOCKET_CONFIG_INIT;
  int                  counter = 0;

  cv2x_service_ptr = (cv2x_service_t *) arg;

  printf("\n______cv2x-example - RX started_______________\n");

  printf("[cv2x-example] creating RX socket...\n");

  // Create Rx socket
  rc = cv2x_socket_create(cv2x_service_ptr, CV2X_SOCKET_TYPE_RX, &socket_config, &cv2x_socket_ptr);
  if (atlk_error(rc)) {
    printf("[cv2x-example] creating RX socket... ERROR: cv2x_socket_create fail. %s\n", atlk_rc_to_str(rc));
    return NULL;
  }

  printf("[cv2x-example] creating RX socket...OK\n");

  while (1) {
    char                  str[MAX_BUFF_SIZE] = {0};
    uint8_t               cv2x_rx_buffer[MAX_BUFF_SIZE] = {0};
    cv2x_receive_params_t rx_params;
    size_t                rx_buffer_size;
    atlk_wait_t           receive_timeout = {
                                                .wait_type = ATLK_WAIT_TYPE_INTERVAL,
                                                .wait_usec = (5000 * MICRO_PER_MILLI)  /* 5 seconds */
                                            };

    if ( (iterations_count_g >= 0) && (iterations_count_g <= counter) ) {
      break;
    }
    counter++;

    printf("[cv2x-example] cv2x_rx waiting for msg\n");

    rx_buffer_size = sizeof(cv2x_rx_buffer);

    printf("[cv2x-example] receiving from RX socket...\n");

    /* Wait for message infinitely */
    rc = cv2x_receive(cv2x_socket_ptr, cv2x_rx_buffer, &rx_buffer_size, &rx_params, &receive_timeout);
    if (atlk_error(rc)) {
      if (rc != ATLK_E_TIMEOUT) {
        (void)printf("[cv2x-example] receiving from RX socket... ERROR: cv2x_receive failed. %s\n", atlk_rc_to_str(rc));
      }
      continue;
    }

    printf("[cv2x-example] receiving from RX socket...OK (got %d bytes)\n", (int)rx_buffer_size);

    if (rx_buffer_size >= sizeof(cv2x_rx_buffer)) {
      (void)printf("[cv2x-example] ERROR: Invalid receive size %u\n", (int)rx_buffer_size);
      continue;
    }

    uint64_t t     = strtoll((char *)cv2x_rx_buffer + 11, NULL, 10);
    uint32_t delta = (uint32_t)((rx_params.receive_time - t) / 1000);

    if(isAcme){
      //Deserialize ACME message and print ACME message without header
      deserialize_acme_message_and_print(cv2x_rx_buffer, (uint16_t) rx_buffer_size, delta);
    } else{
      snprintf(str, sizeof(str), "%s", cv2x_rx_buffer);
      // Set NULL termination 
      str[rx_buffer_size] = 0;
      printf("______cv2x-example_______________\n"
            "**********Received packet: ********\n"
            "%s\n"
            "delta=%u\n" , str, delta);
    }
  }
  return NULL;
}


/* Show CV2XLIB + SDK + BSP + DSP version */
static int
cmd_sdk_version(wdm_service_t *wdm_service, ddm_service_t *ddm_service)
{
  char sdk_version[256];
  size_t sdk_version_size;
  char *sdk_ver = sdk_version;
  atlk_rc_t rc;
  wdm_dsp_version_t dsp_version;
  ddm_baseband_v2x_hw_revision_t v2x_hw_rev;
#ifndef SECTON
  ddm_bsp_version_t bsp_version;
#endif
  uint32_t mj, mn, rv;

  printf("\nHost:\n");
  printf(
            "  CV2X version: %d.%d.%d\n",
            ATLK_SDK_VER_MAJOR,
            ATLK_SDK_VER_MINOR,
            ATLK_SDK_VER_PATCH);

  sdk_version_size = sizeof(sdk_version);

  rc = ddm_version_get(ddm_service, sdk_version, &sdk_version_size);
  if (atlk_error(rc)) {
    printf( "failed to get SDK version rc=%d\n", rc);
  }
  else {
    printf( "Device:\n");
    printf( "  SDK version: %s\n", sdk_ver);

    sdk_ver = sdk_ver + strlen(sdk_version) + 1;
    printf( "  Software version: %s\n", sdk_ver);
  }

  rc = wdm_dsp_version_get(wdm_service, &dsp_version);
  if (atlk_error(rc)) {
    printf( "failed to get WDM DSP version rc=%d\n", rc);
  }
  else {
    printf( "DSP:\n");
    printf(
              "  Software version: %"PRIu8".%"PRIu8".%"PRIu8" %s\n",
              dsp_version.major,
              dsp_version.minor,
              dsp_version.sw_revision,
              dsp_version.dsp_rev_id);
  }

#ifndef SECTON
  rc = ddm_bsp_version_get(ddm_service, &bsp_version);
  if (atlk_error(rc)) {
    printf( "failed to get DDM BSP version rc=%d\n", rc);
  } else {
    printf(
              "BSP:\n"
              "  M3 loader version: %s\n"
              "  OS loader version: %s\n"
              "  OS version: %s\n",
              bsp_version.m3_loader_version,
              bsp_version.ap_loader_version,
              bsp_version.ap_os_version);
  }
#endif

  rc = ddm_baseband_v2x_hw_revision_get(ddm_service, &v2x_hw_rev);
  if (atlk_error(rc)) {
    printf( "failed to get DDM baseband V2X hardware version rc=%d\n", rc);
  }
  else {
    printf( "Baseband V2X:\n");
    printf( "  Hardware version: %"PRIu32".%"PRIu32"\n",
        v2x_hw_rev.bb_v2x_hw_rev_major, v2x_hw_rev.bb_v2x_hw_rev_minor);
  }

  if (cv2x_version_get(&mj, &mn, &rv) == 0) {
    uint32_t build_number = 0;
    cv2x_build_version_get(&build_number);
    printf( "  MAC CV2X Version %d.%d.%d-%d (%s)\n", mj, mn, rv, build_number, sdk_version);
  }

  printf( "\n");

  return 0;
}

/* This is a local function, not to be confused with ref_sys_init. This function calls ref_sys_init in order
   to initialize the system. ref_sys_init is defined in another c file: ref_sys.c */
static atlk_rc_t
init_ref_sys(char *eth,
             unsigned *src_l2id_ptr,
             CV2X_OUT cv2x_configuration_t *config_ptr,
             CV2X_OUT cv2x_rrc_pre_config_t *rrc_config_ptr)
{
  atlk_rc_t rc;

  char *eth0 = "eth0";
  char interface_name[IF_NAMESIZE+1];
  /* Initialize default values for args */
  int i;

  /* We must set SDK in pass-through mode so it will not assume v2x headers. */
  rc = v2x_sockets_passthrough_set(1);
  if (atlk_error(rc)) {
    (void)fprintf(stderr, "\n init_ref_sys v2x_sockets_passthrough_set failed %u (%s)\n", rc, atlk_rc_to_str(rc));
    return ATLK_E_UNSPECIFIED;
  }

  if ((eth != NULL) && (strlen(eth) > 0)) {
    strcpy(interface_name, eth);
  }
  else {
    for (i = 0; i < 256; i++) {
      if (if_indextoname(i, interface_name) != NULL) {
        //printf("%s: interface_name(%d)=%s\n", __FUNCTION__, i, interface_name);
        if ((interface_name[0] == 'e') && (interface_name[1] == 'n') && (interface_name[2] == 'x')) {
          break;
        }
      }
      else {
        strcpy(interface_name, eth0);
      }
    }
  }

  printf("%s: interface_name=%s\n", __FUNCTION__, interface_name);

  /* Reference system initialization - for Secton init the USB ethernet adapter for host-to-device connections,
     for both Craton/Secton perform different link layer inits etc */
  rc = ref_cv2x_sys_init(interface_name, NULL, config_ptr, rrc_config_ptr);
  if (atlk_error(rc)) {
    return ATLK_E_UNSPECIFIED;
  }

  *src_l2id_ptr = ref_cv2x_ue_id_get();

  return ATLK_OK;
}

/* This internal function supports [-i val] option to limit example iterations */
static int32_t
iterations_count_get(int argc, char *argv[])
{
  uint8_t argv_index;
  int32_t interations_count = -1;

  if (argc > 2) {
    for (argv_index = 1; argv_index < (argc - 1); argv_index++) {
      if (strcasecmp("-i", argv[argv_index]) == 0) {
        interations_count = atoi(argv[argv_index+1]);
        break;
      }
    }
  }

  return interations_count;
}

typedef enum cv2x_test_app_mode {
  CV2X_TX,
  CV2X_RX,
  CV2X_INVALID_MODE,
} cv2x_test_app_mode;

/*
   Sample application entry point:
   - Initializes the system and prints the versions of relevant SW packages
   - Creates either a Tx or Rx thread, according to execution argument (argv[1])
*/
int main(int argc, char *argv[])
{
  atlk_rc_t             rc;
  cv2x_test_app_mode    mode = CV2X_INVALID_MODE; /* use this value to mark invalid input */
  cv2x_service_t        *cv2x_service_ptr;
  bool                  print_usage_and_exit = false;
  char                  *eth = NULL;
  unsigned              ue_id = 0;
  cv2x_configuration_t  initial_cv2x_config = CV2X_CONFIGURATION_INIT;
  cv2x_rrc_pre_config_t rrc_config;
  FILE *pFile_r;
 

  /* Print our environment */
#ifndef SECTON
  printf("[cv2x-example] cv2x hello !!! craton mode\n");
#else
  printf("[cv2x-example] cv2x hello !!! secton mode\n");
#endif

  if (argc <= 1) {
    print_usage_and_exit = true;
  }

  /** Num of iterations */
  iterations_count_g = iterations_count_get(argc, argv);

  /* Parse command line arguments */
  for (int iArg = 1; iArg < argc; iArg++) {
    char *endptr;

    /* To distinguish success / failure after call */
    errno = 0;

    if (strcasecmp("-i", argv[iArg]) == 0) {
      iArg++;
      continue;
    }

    long test_option = strtol(argv[iArg], &endptr, 10);

    /* Check for various possible errors */
    if ( ( (errno == ERANGE) && ( (test_option == LONG_MAX) || (test_option == LONG_MIN) ) ) ||
         ( (errno != 0) && (test_option == 0) ) ) {
      // perror("strtol");
      printf("got error %d, %ld\n",errno, test_option);
      print_usage_and_exit = true;
      break;
    }

    if (endptr == argv[iArg]) {
      eth = argv[iArg];
    }
    else {
      switch (test_option) {
        case 1:
          printf("[cv2x-example] CV2X_RX mode \n");
          mode = CV2X_RX;
          break;

        case 2:
          printf("[cv2x-example] CV2X_TX mode \n");
          mode = CV2X_TX;
          break;

        case 3:
          printf("[cv2x-example] CV2X_TX ad-hoc mode \n");
          mode = CV2X_TX;
          ad_hoc_mode = true;
          break;

        case 4:
          printf("[cv2x-example] CV2X_RX ACME mode \n");
          mode = CV2X_RX;
          isAcme = 1;
          break;

        case 5:
          printf("[cv2x-example] CV2X_TX ACME mode \n");
          mode = CV2X_TX;
          isAcme = 1;
          break;

        default:
          print_usage_and_exit = true;
          break;
      }
    }
  }

  /* Invalid params (usage) */
  if ( (print_usage_and_exit) || (mode == CV2X_INVALID_MODE) ) {
    printf("[cv2x-example] usage:\n\t cv2x-example [eth] %%d (1 - RX / 2 - TX / 3 - TX Ad-Hoc / 4 - ACME mode RX / 5 - ACME mode TX) [-i] %%d (limit the number of iterations) \n");
	printf("[cv2x-example] In ACME mode TX, message source file is %s \n", SEND_MSG_FILE_PATH);
    return EXIT_FAILURE;
  }

/*Read ACME Message from File*/
  if(isAcme &&  mode == CV2X_TX){
    pFile_r = fopen(SEND_MSG_FILE_PATH,"r" );
    if (pFile_r != NULL) {
      fread(g_payload2Send, sizeof(char), MAX_ACME_MSG_SIZE-1, pFile_r);
      fclose(pFile_r);
      g_payload2Send[MAX_ACME_MSG_SIZE-1] = 0;
      printf("[cv2x-example] Append ACME message: %s\n", g_payload2Send);
    }
    else
    {
      printf("[cv2x-example] Can not find ACME message, please check file %s\n", SEND_MSG_FILE_PATH);
      return EXIT_FAILURE;
    }
  }

  init_ref_sys(eth, &ue_id, &initial_cv2x_config, &rrc_config);

  wdm_service_t *wdm_service_ptr = NULL;
  ddm_service_t *ddm_service_ptr = NULL;

  printf("[cv2x-example] getting WDM service ptr...\n");

  rc = wdm_service_get(NULL, &wdm_service_ptr);
  if (atlk_error(rc)) {
    (void)fprintf(stderr, "wdm_service_get failed: %d\n", rc);
    return EXIT_FAILURE;
  }

  printf("[cv2x-example] getting DDM service ptr...\n");

  rc = ddm_service_get(NULL, &ddm_service_ptr);
  if (atlk_error(rc)) {
    (void)fprintf(stderr, "ddm_service_get failed: %d\n", rc);
    return EXIT_FAILURE;
  }

  printf("[cv2x-example] Waiting for Device to be ready...\n");

  ddm_state_t state;

  rc = ddm_state_get(ddm_service_ptr, &state);
  if ((atlk_error(rc)) || (state != DDM_STATE_READY)) {
    do {
      sleep(2);
      rc = ddm_state_get(ddm_service_ptr, &state);
      if (atlk_error(rc)) {
        (void)fprintf(stderr, "ddm_service_get failed: %d\n", rc);
        return EXIT_FAILURE;
      }

      if (state != DDM_STATE_READY) {
        printf("Device still not ready ... waiting...\n");
      }
    } while (state != DDM_STATE_READY);
  }

  printf("[cv2x-example] Device ready\n");

  cmd_sdk_version(wdm_service_ptr, ddm_service_ptr);

  rc = time_sync_init(ddm_service_ptr);
  if (atlk_error(rc)) {
    return EXIT_FAILURE;
  }

  printf("[cv2x-example] getting CV2X service ptr...\n");

  rc = cv2x_service_get(NULL, &cv2x_service_ptr);
  if (atlk_error(rc)) {
    printf("[cv2x-example] getting service ptr...ERROR: cv2x_service_get fail. %s\n", atlk_rc_to_str(rc));
    return EXIT_FAILURE;
  }

  rc = cv2x_configuration_set(&initial_cv2x_config, &rrc_config);
  if (atlk_error(rc)) {
    fprintf(stderr, "cv2x_configuration_set failed: %d\n", rc);
    return EXIT_FAILURE;
  }

  rc = cv2x_service_enable(cv2x_service_ptr);
  if (atlk_error(rc)) {
    fprintf(stderr, "cv2x_service_enable failed: %d\n", rc);
    return EXIT_FAILURE;
  }

  printf("[cv2x-example] Setting CV2X src_l2id to 0x%x.\n", ue_id);

  rc = cv2x_src_l2id_set(cv2x_service_ptr, ue_id);
  if (atlk_error(rc)) {
    printf("Error: cv2x_src_l2id_set failed. rc = %d \n",rc);
  }

  printf("[cv2x-example] Starting CV2X task...OK\n");

  if (mode == CV2X_RX) {
    cv2x_rx_mode(cv2x_service_ptr);
  }
  else {
    cv2x_tx_mode(cv2x_service_ptr);
  }

  fprintf(stderr, "done\n");
  return EXIT_SUCCESS;
}
