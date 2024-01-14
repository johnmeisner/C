/**************************************************************************
 *                                                                        *
 *     File Name:  imf.c  (immediate forward handler)                     *
 *     Author:                                                            *
 *         DENSO International America, Inc.                              *
 *         North America Research Laboratory, California Office           *
 *         Rutherford rd, Carlsbad                                        *
 *         CA 92008                                                       *
 *                                                                        *
 **************************************************************************/
/**************************************************************************
    Immediate forward is subordinate to active message handling (amh)
    because it is a subset of the amh functionality - the messages are 
    the same but immediate forward is always pass through - nothing is
    stored.  As such this module only listens for data on the UDP port
    and validates it and passes it to amh as the point of control to
    decide what to do with the data.
 **************************************************************************/  
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <mqueue.h>
#include <poll.h>
#include <fcntl.h>
#include "wsu_util.h"
#include "wsu_shm.h"
#include "i2v_util.h"
#include "amh.h"
#if !defined S_SPLINT_S
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#endif
#include "ntcip-1218.h"
#include "conf_table.h"

#if defined(MY_UNIT_TEST)
#include "stubs.h"
#define BIND      bind
#else
#define SOCKET    socket
#define BIND      bind
#endif
/* mandatory logging defines */
#define MY_ERR_LEVEL   LEVEL_INFO    /* from i2v_util.h */
#define MY_NAME        "imf"

/* 100 ms */
#define DEFAULT_TIMEOUT    100000
/* 1 ms - used only when data has been received */
#define MIN_TIMEOUT        1000
/* 10 ms */
#define ERR_TIMEOUT        10000

/* this is not max UDP data but using macro name as a reminder
   that udp size is real max for imf not MAX_AMH_FILE_SIZE */
#define MAX_UDP_SIZE       MAX_AMH_FILE_SIZE

/* From RSU 4.1 spec for SAR and IMF file type. */
#define AMH_RSU_4_1_SPEC_FILE_VERSION (0.7f)

#define SOCKET_POLL_TIMEOUT  0  /* Don't hang around if not ready. Note error state if not ready.*/

#define IMF_READ_ERROR_THRESHOLD 10 /* Consecutive read fails on socket that triggers reset. */

/* Management of imf socket: These counts persist. */
extern uint32_t imf_socket_fail;
extern uint32_t imf_socket_bind_fail;
extern uint32_t imf_socket_reset_count;

/* Reading from imf socket: These counts reach IMF_READ_ERROR_THRESHOLD and socket reset occurs.*/
extern uint32_t imf_read_count;
extern uint32_t total_imf_read_error_count;
extern uint32_t imf_read_error_count;
extern uint32_t imf_read_poll_to_count;
extern uint32_t imf_read_poll_notready_count;
extern uint32_t imf_read_poll_fail_count;

/* externs */
extern i2vShmMasterT * shmPtr; /* For BSM & PSM forwarding. */
extern bool_t          mainloop;
extern amhCfgItemsT    amhCfg;
extern cfgItemsT       cfg;
extern uint16_t        amhstoredbcnt;
extern pthread_mutex_t amhRadio_lock;

extern uint32_t amhTxMessage(uint8_t * buf, int32_t count, amhSendParams * shmsendp); /* Send message to radio. */
extern void     set_amh_error_state(int32_t my_error); /* Report module error states. */
extern void     amhForwardManager(amhBitmaskType amh_msgType, uint8_t * buf, int32_t count);
extern void     sendnotification(char_t * msg, int32_t level);
extern int32_t  is_amh_error_set(int32_t amh_error);

/* Non-blocking. */
STATIC int32_t imfSockInit(void)
{
    struct sockaddr_in local;
    int32_t sock;

    if ((sock = SOCKET(AF_INET, (SOCK_DGRAM|SOCK_NONBLOCK), IPPROTO_UDP)) == -1) {
#if defined(EXTRA_DEBUG)
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"AMH_IMF_SOCKET_FAIL: errno = %d(%s)\n",errno,strerror(errno));
#endif
        set_amh_error_state(AMH_IMF_SOCKET_FAIL);
        imf_socket_fail++;
        return -1;
    }
    memset(&local, 0, sizeof(local));
    local.sin_family = AF_INET;
    local.sin_port = htons(amhCfg.imfPort);
    local.sin_addr.s_addr = htonl(INADDR_ANY);
    if (BIND(sock, (struct sockaddr *)&local, sizeof(local)) == -1) {
#if defined(EXTRA_DEBUG)
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"AMH_IMF_BIND_FAIL: errno = %d(%s)\n",errno,strerror(errno));
#endif
        close(sock);
        sock = -1;
        set_amh_error_state(AMH_IMF_BIND_FAIL);
        imf_socket_bind_fail++;
        return -1;
    }
    return sock;
}
/**********************************************************************************
The difference between IMF & SAR is msgInterval will always be greater than 0.

Return value is a bitfield. Error values are integers.

Assumption: Storage space is not full with too many SAR files. No check here.
Assumption: Not checking size of file. If we parse correctly then what in it beyond that we dont care.
Assumption: Only validates enough to put into file: start, stop & msg type.
Assumption: Only SAR care about start and stop dates.
**********************************************************************************/
STATIC int32_t Process_Store_and_Repeat(rawImmediateType * msg, float32_t version, char_t * buf, uint32_t size, char_t * start_date_buffer, char_t * stop_date_buffer)
{
  uint32_t  i           = 0;
  uint32_t  start_min   = 0;
  uint32_t  start_hour  = 0;
  uint32_t  start_day   = 0;
  uint32_t  start_month = 0;
  uint32_t  start_year  = 0;
  uint32_t  stop_min    = 0;
  uint32_t  stop_hour   = 0;
  uint32_t  stop_day    = 0;
  uint32_t  stop_month  = 0;
  uint32_t  stop_year   = 0;
  FILE    * my_hold_and_forward_spat = NULL;
  char_t    amh_name_buffer[AMH_PATH_SIZE];
  char_t    file_name_buffer[AMH_FILENAME_SIZE];
  char_t    buffer[RSU_ALERT_MSG_LENGTH_MAX];

    if((NULL == msg) || (NULL == buf) || (0 == size)){
        set_amh_error_state(AMH_IMF_BAD_INPUT);
        return AMH_IMF_BAD_INPUT;
    }
    /* Without validation there is no SAR. */
    if(!amhCfg.validateData){
        return AMH_SAR_VALIDATION_NOT_ENABLED;
    }
    if (AMH_RSU_4_1_SPEC_FILE_VERSION != version){
        if(0x0 == (is_amh_error_set(AMH_PROCESS_MSG_WRONG_VERSION))) {
            set_amh_error_state(AMH_PROCESS_MSG_WRONG_VERSION);
            memset(buffer,0x0,sizeof(buffer));
            snprintf(buffer,sizeof(buffer), "Legacy IMF: Wrong version: expected(%lf) got(%lf)",AMH_RSU_4_1_SPEC_FILE_VERSION, version);
            sendnotification(buffer, rsuAlertLevel_error);
        }
        return AMH_PROCESS_MSG_WRONG_VERSION ;
    }
    memset(&amh_name_buffer,'\0',sizeof(amh_name_buffer));
    snprintf(amh_name_buffer,sizeof(amh_name_buffer),"%s",AMH_ACTIVE_DIR);

    //replace chars that will be problematic

          for(i=0; (i<AMH_FILENAME_SIZE) && ('\0' != start_date_buffer[i]);i++){
            if(   ('/' == start_date_buffer[i])
               || (',' == start_date_buffer[i])
               || (':' == start_date_buffer[i])
              ) {
              start_date_buffer[i] = '_';
            }
          }

          for(i=0; (i<AMH_FILENAME_SIZE) && ('\0' != stop_date_buffer[i]);i++) {
            if(   ('/' == stop_date_buffer[i])
               || (',' == stop_date_buffer[i])
               || (':' == stop_date_buffer[i])
              ) {
              stop_date_buffer[i] = '_';
            }
          }

          //Now compare date & times to make sure they are sane


          /****************************
          * START time & date
          *****************************/

          for(i=0; (i<AMH_FILENAME_SIZE) && ('\0' != start_date_buffer[i]);i++) {

            //get month

             if('_' == start_date_buffer[i]) {
               i++;
               break;
             }
             start_month *= 10; //shift it
             start_month += (uint32_t)start_date_buffer[i] - 48; //add it

          }

          for(i=i; (i<AMH_FILENAME_SIZE) && ('\0' != start_date_buffer[i]);i++) {

             //get day

             if('_' == start_date_buffer[i]) {
               i++;
               break;
             }

             start_day *= 10; //shift it
             start_day += (uint32_t)start_date_buffer[i] - 48; //add it
          }

          for(i=i; (i<AMH_FILENAME_SIZE) && ('\0' != start_date_buffer[i]);i++) {

             //get year
 
             if('_' == start_date_buffer[i]) {
               i++;
               break;
             }
             start_year *= 10; //shift it
             start_year += (uint32_t)start_date_buffer[i] - 48; //add it
          }
 
          for(i=i; (i<AMH_FILENAME_SIZE) && ('\0' != start_date_buffer[i]);i++) {
            //get hour

             if('_' == start_date_buffer[i]) {
               i++;
               break;
             }
             start_hour *= 10; //shift it
             start_hour += (uint32_t)start_date_buffer[i] - 48; //add it

          }
 
          for(i=i; (i<AMH_FILENAME_SIZE) && ('\0' != start_date_buffer[i]);i++) {

             //get min

             if('_' == start_date_buffer[i]) {
               i++;
               break;
             }
             start_min *= 10; //shift it
             start_min += (uint32_t)start_date_buffer[i] - 48; //add it

          }
 
          //range check date & time values

          if(AMH_FILENAME_SIZE <= i) {
              if(0x0 == (is_amh_error_set(AMH_FILENAME_TOO_LONG))) {
                  set_amh_error_state(AMH_FILENAME_TOO_LONG);
                  memset(buffer,0x0,sizeof(buffer));
                  snprintf(buffer,sizeof(buffer), "AMH_FILENAME_TOO_LONG: max(%d) <= size(%d)", AMH_FILENAME_SIZE,i);
                  sendnotification(buffer, rsuAlertLevel_error);
              }
              return AMH_FILENAME_TOO_LONG;
          }
          if(59 < start_min) {
            set_amh_error_state(AMH_SAR_BAD_START_TIME);
            return AMH_SAR_BAD_START_TIME;
          }
          if(23 < start_hour) {
            set_amh_error_state(AMH_SAR_BAD_START_TIME);
            return AMH_SAR_BAD_START_TIME;
          }
          /* TODO should check against month */ 
          if((31 < start_day) || (0 == start_day)) { 
            set_amh_error_state(AMH_SAR_BAD_START_TIME);
            return AMH_SAR_BAD_START_TIME;
          }
          if((12 < start_month) || (0 == start_month)) {
            set_amh_error_state(AMH_SAR_BAD_START_TIME);
            return AMH_SAR_BAD_START_TIME;
          }
          if(2100 < start_year) {
            set_amh_error_state(AMH_SAR_BAD_START_TIME);
            return AMH_SAR_BAD_START_TIME;
          }
//retrospect do we care that much?
          if(start_year < 2017) {
            set_amh_error_state(AMH_SAR_BAD_START_TIME);
            return AMH_SAR_BAD_START_TIME;
          }

          /****************************
          * STOP time & date
          *****************************/

          for(i=0; (i<AMH_FILENAME_SIZE) && ('\0' != stop_date_buffer[i]);i++) {
            
             //get month

             if('_' == stop_date_buffer[i]) {
               i++;
               break;
             }
             stop_month *= 10; //shift it
             stop_month += (uint32_t)stop_date_buffer[i] - 48; //add it
          }

          for(i=i; (i<AMH_FILENAME_SIZE) && ('\0' != stop_date_buffer[i]);i++) {

             //get day

             if('_' == stop_date_buffer[i]) {
               i++;
               break;
             }
             stop_day *= 10; //shift it
             stop_day += (uint32_t)stop_date_buffer[i] - 48; //add it

          }

          for(i=i; (i<AMH_FILENAME_SIZE) && ('\0' != stop_date_buffer[i]);i++) {
            
             //get year

             if('_' == stop_date_buffer[i]) {
               i++;
               break;
             }
             stop_year *= 10; //shift it
             stop_year += (uint32_t)stop_date_buffer[i] - 48; //add it

          }

          for(i=i; (i<AMH_FILENAME_SIZE) && ('\0' != stop_date_buffer[i]);i++) {

             //get hour

             if('_' == stop_date_buffer[i]) {
               i++;
               break;
             }
             stop_hour *= 10; //shift it
             stop_hour += (uint32_t)stop_date_buffer[i] - 48; //add it

          }

          for(i=i; (i<AMH_FILENAME_SIZE) && ('\0' != stop_date_buffer[i]);i++) {
            
             //get min

             if('_' == stop_date_buffer[i]) {
               i++;
               break;
             }
             stop_min *= 10; //shift it
             stop_min += (uint32_t)stop_date_buffer[i] - 48; //add it

          }

          //range check date & time values

          if(AMH_FILENAME_SIZE <= i) {
            set_amh_error_state(AMH_SAR_FILENAME_TOO_LONG);
            return AMH_SAR_FILENAME_TOO_LONG;
          }
          if(59 < stop_min) {
            set_amh_error_state(AMH_SAR_BAD_STOP_TIME);
            return AMH_SAR_BAD_STOP_TIME;
          }
          if(23 < stop_hour) {
            set_amh_error_state(AMH_SAR_BAD_STOP_TIME);
            return AMH_SAR_BAD_STOP_TIME;
          }
          if((31 < stop_day) || (0 == stop_day)) {
            set_amh_error_state(AMH_SAR_BAD_STOP_TIME);
            return AMH_SAR_BAD_STOP_TIME;
          }
          if((12 < stop_month) || (0 == stop_month)) {
            set_amh_error_state(AMH_SAR_BAD_STOP_TIME);
            return AMH_SAR_BAD_STOP_TIME;
          }
          if(2100 < stop_year) {
            set_amh_error_state(AMH_SAR_BAD_STOP_TIME);
            return AMH_SAR_BAD_STOP_TIME;
          }
          if(stop_year < 2017) {
            set_amh_error_state(AMH_SAR_BAD_STOP_TIME);
            return AMH_SAR_BAD_STOP_TIME;
          }

          //Now compare start & stop dates to ensure well formed and relevent
          if(stop_year < start_year) {
            set_amh_error_state(AMH_SAR_STOP_TIME_BEFORE_START);
            return AMH_SAR_STOP_TIME_BEFORE_START;
          }

          if(start_year == stop_year) {
            if(stop_month < start_month) {
                set_amh_error_state(AMH_SAR_STOP_TIME_BEFORE_START);
                return AMH_SAR_STOP_TIME_BEFORE_START;
            }

            if(start_month == stop_month) {
              if(stop_day < start_day) {
                set_amh_error_state(AMH_SAR_STOP_TIME_BEFORE_START);
                return AMH_SAR_STOP_TIME_BEFORE_START;
              }

              if(start_day == stop_day) {
                if(stop_hour < start_hour) {
                  set_amh_error_state(AMH_SAR_STOP_TIME_BEFORE_START);
                  return AMH_SAR_STOP_TIME_BEFORE_START;
                }

                if(start_hour == stop_hour){
                  if(stop_min <= start_min) {
                    set_amh_error_state(AMH_SAR_STOP_TIME_BEFORE_START);
                    return AMH_SAR_STOP_TIME_BEFORE_START;
                  }
                }
              }
            }
          } /* if(start_year == stop_year) */
          memset(&file_name_buffer,'\0',sizeof(file_name_buffer));
          switch(msg->type) {
          case AMH_MSG_MAP_MASK: 
              snprintf(file_name_buffer,sizeof(file_name_buffer),"%s_0x%x_%s_%s.txt",MAP_EXTENSION,msg->psid,start_date_buffer ,stop_date_buffer);
            break;
          case AMH_MSG_SPAT_MASK:
              snprintf(file_name_buffer,sizeof(file_name_buffer),"%s_0x%x_%s_%s.txt",SPAT_EXTENSION,msg->psid,start_date_buffer ,stop_date_buffer);
              break;
          case AMH_MSG_TIM_MASK:
              snprintf(file_name_buffer,sizeof(file_name_buffer),"%s_0x%x_%s_%s.txt",TIM_EXTENSION,msg->psid,start_date_buffer ,stop_date_buffer);
              break;
          case AMH_MSG_RTCM_MASK:
              snprintf(file_name_buffer,sizeof(file_name_buffer),"%s_0x%x_%s_%s.txt",RTCM_EXTENSION,msg->psid,start_date_buffer ,stop_date_buffer);
              break;
          case AMH_MSG_ICA_MASK:
              snprintf(file_name_buffer,sizeof(file_name_buffer),"%s_0x%x_%s_%s.txt",ICA_EXTENSION,msg->psid,start_date_buffer ,stop_date_buffer);
              break;
          case AMH_MSG_PDM_MASK:
              snprintf(file_name_buffer,sizeof(file_name_buffer),"%s_0x%x_%s_%s.txt",PDM_EXTENSION,msg->psid,start_date_buffer ,stop_date_buffer);
              break;
          case AMH_MSG_RSA_MASK:
              snprintf(file_name_buffer,sizeof(file_name_buffer),"%s_0x%x_%s_%s.txt",RSA_EXTENSION,msg->psid,start_date_buffer ,stop_date_buffer);
              break;
          case AMH_MSG_SSM_MASK:
              snprintf(file_name_buffer,sizeof(file_name_buffer),"%s_0x%x_%s_%s.txt",SSM_EXTENSION,msg->psid,start_date_buffer ,stop_date_buffer);
              break;
          case AMH_MSG_CSR_MASK:
              snprintf(file_name_buffer,sizeof(file_name_buffer),"%s_0x%x_%s_%s.txt",CSR_EXTENSION,msg->psid,start_date_buffer ,stop_date_buffer);
              break;
          case AMH_MSG_EVA_MASK:
              snprintf(file_name_buffer,sizeof(file_name_buffer),"%s_0x%x_%s_%s.txt",EVA_EXTENSION,msg->psid,start_date_buffer ,stop_date_buffer);
              break;
          case AMH_MSG_NMEA_MASK:
              snprintf(file_name_buffer,sizeof(file_name_buffer),"%s_0x%x_%s_%s.txt",NMEA_EXTENSION,msg->psid,start_date_buffer ,stop_date_buffer);
              break;
          case AMH_MSG_PSM_MASK:
              snprintf(file_name_buffer,sizeof(file_name_buffer),"%s_0x%x_%s_%s.txt",PSM_EXTENSION,msg->psid,start_date_buffer ,stop_date_buffer);
              break;
          case AMH_MSG_PVD_MASK:
              snprintf(file_name_buffer,sizeof(file_name_buffer),"%s_0x%x_%s_%s.txt",PVD_EXTENSION,msg->psid,start_date_buffer ,stop_date_buffer);
              break;
          case AMH_MSG_SRM_MASK:
              snprintf(file_name_buffer,sizeof(file_name_buffer),"%s_0x%x_%s_%s.txt",SRM_EXTENSION,msg->psid,start_date_buffer ,stop_date_buffer);
              break;
          case AMH_MSG_BSM_MASK:
              snprintf(file_name_buffer,sizeof(file_name_buffer),"%s_0x%x_%s_%s.txt",BSM_EXTENSION,msg->psid,start_date_buffer ,stop_date_buffer);
              break;
          case AMH_MSG_TEST00_MASK:
              snprintf(file_name_buffer,sizeof(file_name_buffer),"%s_0x%x_%s_%s.txt",TEST00_EXTENSION,msg->psid,start_date_buffer ,stop_date_buffer);
              break;
          case AMH_MSG_TEST01_MASK:
              snprintf(file_name_buffer,sizeof(file_name_buffer),"%s_0x%x_%s_%s.txt",TEST01_EXTENSION,msg->psid,start_date_buffer ,stop_date_buffer);
              break;
          case AMH_MSG_TEST02_MASK:
              snprintf(file_name_buffer,sizeof(file_name_buffer),"%s_0x%x_%s_%s.txt",TEST02_EXTENSION,msg->psid,start_date_buffer ,stop_date_buffer);
              break;
          case AMH_MSG_TEST03_MASK:
              snprintf(file_name_buffer,sizeof(file_name_buffer),"%s_0x%x_%s_%s.txt",TEST03_EXTENSION,msg->psid,start_date_buffer ,stop_date_buffer);
              break;
          case AMH_MSG_TEST04_MASK:
              snprintf(file_name_buffer,sizeof(file_name_buffer),"%s_0x%x_%s_%s.txt",TEST04_EXTENSION,msg->psid,start_date_buffer ,stop_date_buffer);
              break;
          case AMH_MSG_TEST05_MASK:
              snprintf(file_name_buffer,sizeof(file_name_buffer),"%s_0x%x_%s_%s.txt",TEST05_EXTENSION,msg->psid,start_date_buffer ,stop_date_buffer);
              break;
          case AMH_MSG_TEST06_MASK:
              snprintf(file_name_buffer,sizeof(file_name_buffer),"%s_0x%x_%s_%s.txt",TEST06_EXTENSION,msg->psid,start_date_buffer ,stop_date_buffer);
              break;
          case AMH_MSG_TEST07_MASK:
              snprintf(file_name_buffer,sizeof(file_name_buffer),"%s_0x%x_%s_%s.txt",TEST07_EXTENSION,msg->psid,start_date_buffer ,stop_date_buffer);
              break;
          case AMH_MSG_TEST08_MASK:
              snprintf(file_name_buffer,sizeof(file_name_buffer),"%s_0x%x_%s_%s.txt",TEST08_EXTENSION,msg->psid,start_date_buffer ,stop_date_buffer);
              break;
          case AMH_MSG_TEST09_MASK:
              snprintf(file_name_buffer,sizeof(file_name_buffer),"%s_0x%x_%s_%s.txt",TEST09_EXTENSION,msg->psid,start_date_buffer ,stop_date_buffer);
              break;
          case AMH_MSG_TEST10_MASK:
              snprintf(file_name_buffer,sizeof(file_name_buffer),"%s_0x%x_%s_%s.txt",TEST10_EXTENSION,msg->psid,start_date_buffer ,stop_date_buffer);
              break;
          case AMH_MSG_TEST11_MASK:
              snprintf(file_name_buffer,sizeof(file_name_buffer),"%s_0x%x_%s_%s.txt",TEST11_EXTENSION,msg->psid,start_date_buffer ,stop_date_buffer);
              break;
          case AMH_MSG_TEST12_MASK:
              snprintf(file_name_buffer,sizeof(file_name_buffer),"%s_0x%x_%s_%s.txt",TEST12_EXTENSION,msg->psid,start_date_buffer ,stop_date_buffer);
              break;
          case AMH_MSG_TEST13_MASK:
              snprintf(file_name_buffer,sizeof(file_name_buffer),"%s_0x%x_%s_%s.txt",TEST13_EXTENSION,msg->psid,start_date_buffer ,stop_date_buffer);
              break;
          case AMH_MSG_TEST14_MASK:
              snprintf(file_name_buffer,sizeof(file_name_buffer),"%s_0x%x_%s_%s.txt",TEST14_EXTENSION,msg->psid,start_date_buffer ,stop_date_buffer);
              break;
          case AMH_MSG_TEST15_MASK:
              snprintf(file_name_buffer,sizeof(file_name_buffer),"%s_0x%x_%s_%s.txt",TEST15_EXTENSION,msg->psid,start_date_buffer ,stop_date_buffer);
              break;
          default:
              /*should never get it based on TYPE checking above. */
              set_amh_error_state(AMH_PROCESS_BAD_MSG_TYPE);
              return AMH_PROCESS_BAD_MSG_TYPE;
          }

          snprintf(amh_name_buffer,sizeof(amh_name_buffer),"%s/%s",AMH_ACTIVE_DIR,file_name_buffer);
          my_hold_and_forward_spat = fopen(amh_name_buffer,"w");

          if (NULL != my_hold_and_forward_spat) {
            for(i=0;i<size;i++) {
              fprintf(my_hold_and_forward_spat ,"%c",buf[i]);
            }
            fflush(my_hold_and_forward_spat);
            fclose(my_hold_and_forward_spat);  
          } else {
            set_amh_error_state(AMH_SAR_FILE_OPEN_ERROR);
            return AMH_SAR_FILE_OPEN_ERROR;
          }

    return AMH_AOK;
}

/*
 * Took these functions out of amhlib.c & i2v_util.c.
 * The threads for IMF and AMH were clobbering each other.
 * Duplicating code here to make thread safe.
 * Pros: easy, faster and works.
 * Cons: duplicate code.
 */
STATIC void imfUpdateType(char_t GCC_UNUSED_VAR *tag, char_t GCC_UNUSED_VAR *parsedItem, char_t *value, void *configItem, uint32_t *status)
{
    amhBitmaskType *result = (amhBitmaskType *)configItem;
 
    if ((NULL == value) || (NULL == configItem) || (NULL == status)) {
        set_amh_error_state(AMH_IMF_BAD_INPUT);
        return;
    }

    *status = I2V_RETURN_FAIL;
    *result = AMH_MSG_UNKNOWN_MASK;

    if (!strcmp(value, "SPAT")) {
        *status = I2V_RETURN_OK;
        *result = AMH_MSG_SPAT_MASK;
        return;
    }
    if (!strcmp(value, "MAP")) {
        *status = I2V_RETURN_OK;
        *result = AMH_MSG_MAP_MASK;
        return;
    }
    if (!strcmp(value, "TIM")) {
        *status = I2V_RETURN_OK;
        *result = AMH_MSG_TIM_MASK;
        return;
    }
    if (!strcmp(value, "RTCM")) {
        *status = I2V_RETURN_OK;
        *result = AMH_MSG_RTCM_MASK;
        return;
    }
    if (!strcmp(value, "SPAT16")) {
        *status = I2V_RETURN_OK;
        *result = AMH_MSG_SPAT_MASK;
        return;
    }
    if (!strcmp(value, "MAP16")) {
        *status = I2V_RETURN_OK;
        *result = AMH_MSG_MAP_MASK;
        return;
    }
    if (!strcmp(value, "TIM16")) {
        *status = I2V_RETURN_OK;
        *result = AMH_MSG_TIM_MASK;
        return;
    }
    if (!strcmp(value, "RTCM16")) {
        *status = I2V_RETURN_OK;
        *result = AMH_MSG_RTCM_MASK;
        return;
    }
    if (!strcmp(value, "ICA")) {
        *status = I2V_RETURN_OK;
        *result = AMH_MSG_ICA_MASK;
        return;
    }
    if (!strcmp(value, "PDM")) {
        *status = I2V_RETURN_OK;
        *result = AMH_MSG_PDM_MASK;
        return;
    }
    if (!strcmp(value, "RSA")) {
        *status = I2V_RETURN_OK;
        *result = AMH_MSG_RSA_MASK;
        return;
    }
    if (!strcmp(value, "SSM")) {
        *status = I2V_RETURN_OK;
        *result = AMH_MSG_SSM_MASK;
        return;
    }
    if (!strcmp(value, "CSR")) {
        *status = I2V_RETURN_OK;
        *result = AMH_MSG_CSR_MASK;
        return;
    }
    if (!strcmp(value, "EVA")) {
        *status = I2V_RETURN_OK;
        *result = AMH_MSG_EVA_MASK;
        return;
    }
    if (!strcmp(value, "NMEA")) {
        *status = I2V_RETURN_OK;
        *result = AMH_MSG_NMEA_MASK;
        return;
    }
    if (!strcmp(value, "PSM")) {
        *status = I2V_RETURN_OK;
        *result = AMH_MSG_PSM_MASK;
        return;
    }
    if (!strcmp(value, "PVD")) {
        *status = I2V_RETURN_OK;
        *result = AMH_MSG_PVD_MASK;
        return;
    }
    if (!strcmp(value, "SRM")) {
        *status = I2V_RETURN_OK;
        *result = AMH_MSG_SRM_MASK;
        return;
    }
    if (!strcmp(value, "BSM")) {
        *status = I2V_RETURN_OK;
        *result = AMH_MSG_BSM_MASK;
        return;
    }
    if (!strcmp(value, "TEST00")) {
        *status = I2V_RETURN_OK;
        *result = AMH_MSG_TEST00_MASK;
        return;
    }
    if (!strcmp(value, "TEST01")) {
        *status = I2V_RETURN_OK;
        *result = AMH_MSG_TEST01_MASK;
        return;
    }
    if (!strcmp(value, "TEST02")) {
        *status = I2V_RETURN_OK;
        *result = AMH_MSG_TEST02_MASK;
        return;
    }
    if (!strcmp(value, "TEST03")) {
        *status = I2V_RETURN_OK;
        *result = AMH_MSG_TEST03_MASK;
        return;
    }
    if (!strcmp(value, "TEST04")) {
        *status = I2V_RETURN_OK;
        *result = AMH_MSG_TEST04_MASK;
        return;
    }
    if (!strcmp(value, "TEST05")) {
        *status = I2V_RETURN_OK;
        *result = AMH_MSG_TEST05_MASK;
        return;
    }
    if (!strcmp(value, "TEST06")) {
        *status = I2V_RETURN_OK;
        *result = AMH_MSG_TEST06_MASK;
        return;
    }
    if (!strcmp(value, "TEST07")) {
        *status = I2V_RETURN_OK;
        *result = AMH_MSG_TEST07_MASK;
        return;
    }
    if (!strcmp(value, "TEST08")) {
        *status = I2V_RETURN_OK;
        *result = AMH_MSG_TEST08_MASK;
        return;
    }
    if (!strcmp(value, "TEST09")) {
        *status = I2V_RETURN_OK;
        *result = AMH_MSG_TEST09_MASK;
        return;
    }
    if (!strcmp(value, "TEST10")) {
        *status = I2V_RETURN_OK;
        *result = AMH_MSG_TEST10_MASK;
        return;
    }
    if (!strcmp(value, "TEST11")) {
        *status = I2V_RETURN_OK;
        *result = AMH_MSG_TEST11_MASK;
        return;
    }
    if (!strcmp(value, "TEST12")) {
        *status = I2V_RETURN_OK;
        *result = AMH_MSG_TEST12_MASK;
        return;
    }
    if (!strcmp(value, "TEST13")) {
        *status = I2V_RETURN_OK;
        *result = AMH_MSG_TEST13_MASK;
        return;
    }
    if (!strcmp(value, "TEST14")) {
        *status = I2V_RETURN_OK;
        *result = AMH_MSG_TEST14_MASK;
        return;
    }
    if (!strcmp(value, "TEST15")) {
        *status = I2V_RETURN_OK;
        *result = AMH_MSG_TEST15_MASK;
        return;
    }
    /* if we are here, we are returning I2V_RETURN_FAIL */
}

STATIC void imfUpdateTxMode(char_t GCC_UNUSED_VAR *tag, char_t GCC_UNUSED_VAR *parsedItem, char_t *value, void *configItem, uint32_t *status)
{
    bool_t *result = (bool_t *)configItem;
    bool_t valid = WFALSE;
 
    if ((NULL == value) || (NULL == configItem)) {
        set_amh_error_state(AMH_IMF_BAD_INPUT);
        return;
    }
    if (!strcmp(value, "CONT")) {
        *result = WTRUE;
        valid = WTRUE;
    } else if (!strcmp(value, "ALT")) {
        *result = WFALSE;
        valid = WTRUE;
    }

    if (NULL != status) *status = (valid) ? I2V_RETURN_OK : I2V_RETURN_FAIL;
}

/* 20140903 - adding in support for any numeric channel number within range -
   this was a request from the june plugfest */
STATIC void imfUpdateTxChannel(char_t GCC_UNUSED_VAR *tag, char_t GCC_UNUSED_VAR *parsedItem, char_t *value, void *configItem, uint32_t *status)
{
    uint8_t *result = (uint8_t *)configItem;
    bool_t valid = WFALSE;
 
    if ((NULL == value) || (NULL == configItem)) {
        set_amh_error_state(AMH_IMF_BAD_INPUT);
        return;
    }
    if (!strcmp(value, "CCH")) {
        *result = I2V_CONTROL_CHANNEL;   /* control channel */
        valid = WTRUE;
    } else if (!strcmp(value, "SCH")) {
        *result = AMH_NEED_SCH;   /* indication to get service channel */
        valid = WTRUE;
    } else if ((*result = (uint8_t)strtoul(value, NULL, 10))) {
        /* above will set value to 0 if not a number and this block not entered */
        if ((*result >= I2V_MIN_SVC_CHANNEL) && (*result <= I2V_MAX_SVC_CHANNEL)) {
            valid = WTRUE;
        } /* us band specific - may need to consider other country frequencies (i.e. macro for vals) */
    }

    if (NULL != status) *status = (valid) ? I2V_RETURN_OK : I2V_RETURN_FAIL;
}

STATIC void imfUpdateBool(char_t GCC_UNUSED_VAR *tag, char_t GCC_UNUSED_VAR *parsedItem, char_t *value, void *configItem, uint32_t *status)
{
    bool_t *result = (bool_t *)configItem;
    bool_t valid = WFALSE;
 
    if ((NULL == value) || (NULL == configItem)) {
        set_amh_error_state(AMH_IMF_BAD_INPUT);
        return;
    }

    if (!strcmp(value, "True")) {
        *result = WTRUE;
        valid = WTRUE;
    } else if (!strcmp(value, "False")) {
        *result = WFALSE;
        valid = WTRUE;
    }

    if (NULL != status) *status = (valid) ? I2V_RETURN_OK : I2V_RETURN_FAIL;
}

static uint8_t my_i2v_ascii_2_hex_nibble(char_t  value)
{       
  uint8_t dummy = 0xFF;

  if(( '0' <= value) && (value <= '9'))
      dummy =  (uint8_t)(value - 48);
  if(( 'A' <= value) && (value <= 'F'))
      dummy =  (uint8_t)(value - 55);
  if(( 'a' <= value) && (value <= 'f'))
      dummy =  (uint8_t)(value - 87);

  return dummy;
}

STATIC void imfUpdatePayload(char_t GCC_UNUSED_VAR *tag, char_t GCC_UNUSED_VAR *parsedItem, char_t *value, void  *configItem, uint32_t *status)
{
    payloadType * result  = (payloadType *)configItem;
    bool_t        valid   = WFALSE;
    uint16_t      i       = 0;
    uint16_t      valsize = 0;
    uint8_t       dummy   = 0;
    char_t        buffer[RSU_ALERT_MSG_LENGTH_MAX];

    if ((NULL == value) || (NULL == result) || (NULL == configItem)) {
        valid = WFALSE;
        set_amh_error_state(AMH_IMF_BAD_INPUT);
        return;
    }  else {
        valid = WTRUE;
        valsize = strlen(value); 
        if(valsize < 6) { /* Too small ? */
            valid = WFALSE;
            set_amh_error_state(AMH_IMF_PAYLOAD_TOO_SMALL);
        }
        if(RSU_MIB_MAX_PAYLOAD_OCTET_STRING < valsize) { /* Too big? */
            valid = WFALSE;
            set_amh_error_state(AMH_IMF_PAYLOAD_TOO_BIG);
        }
        if( 1 == (valsize % 2)) { /* Too odd? */
            valid = WFALSE;
            set_amh_error_state(AMH_IMF_PAYLOAD_ODD_LENGTH);
        }
        if(WTRUE == valid) {  /* Just right ! */
            for(i=0, result->count=0; (WTRUE == valid) && (result->count < MAX_WSM_DATA) && (i < valsize ); i++) {
                dummy = my_i2v_ascii_2_hex_nibble(value[i]);
                if(0xff == dummy) {
                    if (NULL != status)
                        *status = I2V_RETURN_FAIL;
                    valid = WFALSE;
                    if(0x0 == (is_amh_error_set(AMH_IMF_PAYLOAD_NOT_HEX_DATA))) {
                        set_amh_error_state(AMH_IMF_PAYLOAD_NOT_HEX_DATA);
                        memset(buffer,0x0,sizeof(buffer));
                        snprintf(buffer,sizeof(buffer), "AMH_IMF_PAYLOAD_NOT_HEX_DATA: IMF payload is not hex ascii values.");
                        sendnotification(buffer, rsuAlertLevel_error);
                    }
                    break; /* Error out */
                } else {
                    if(0 == (i % 2)) {
                        result->payload[result->count] = (0xf) & dummy;
                    } else {
                        result->payload[result->count]  = result->payload[result->count] << 4; 
                        result->payload[result->count] |= (0xf) & dummy;
                        result->count++;
                    }
                }
            }
        }
    }
    if (NULL != status) { *status = (valid) ? I2V_RETURN_OK : I2V_RETURN_FAIL; }
}
STATIC void imfUpdateUint32Value(void *configItem, char_t *val, char_t *min, char_t *max, char_t **trange, uint32_t *status, i2vLogWrapper GCC_UNUSED_VAR *logger)
{
    uint32_t * cfg        = (uint32_t *)configItem;
    uint32_t   value      = (uint32_t)strtoul(val, NULL, 0);
    uint32_t   minval     = 0, maxval = 0;
    int32_t    i          = 0;
    bool_t     valueValid = WFALSE;

    *status = I2V_RETURN_OK;

    if (strcmp(min, I2V_CFG_UNDEFINED) != 0 ) minval = strtoul(min, NULL, 0);
    if (strcmp(max, I2V_CFG_UNDEFINED) != 0 ) maxval = strtoul(max, NULL, 0);

    if ((value >= minval) && (value <= maxval)) {
        *cfg = value;
    } else if( (strcmp(min, I2V_CFG_UNDEFINED) == 0) && (strcmp(max, I2V_CFG_UNDEFINED) == 0) ) {
          /* No Min/Max limits for this value. */
          *cfg = value;
    } else {
        if (value < minval) {
            *cfg = minval;
            *status = I2V_RETURN_MIN_LIMIT_FAIL;
        } 
        if (value > maxval) {
            *cfg = maxval;
            *status = I2V_RETURN_MAX_LIMIT_FAIL;
        }
    }
    if( trange == NULL ) return;
    /* Ensure that the specified value is equal to a table discrete. */
    while( *(trange + i) != NULL ) {
        if( !valueValid && (value  == strtoul(*(trange+i), NULL, 0)) ) {
            valueValid = WTRUE;
            break;
        }
        i++;
    }
    /* Value is not contained in discrete value range */
    if( !valueValid ) {
        *status = I2V_RETURN_RANGE_LIMIT_FAIL;
    }
}

STATIC void imfUpdateUint8Value(void *configItem, char_t *val, char_t *min, char_t *max, char_t **trange, uint32_t *status, i2vLogWrapper GCC_UNUSED_VAR *logger)
{
    uint8_t * cfg        = (uint8_t *)configItem;
    uint8_t   value      = (uint8_t)strtoul(val, NULL, 0);
    uint8_t   minval     = 0, maxval = 0;
    int32_t   i          = 0;
    bool_t    valueValid = WFALSE;
    
    *status = I2V_RETURN_OK;

    if (strcmp(min, I2V_CFG_UNDEFINED) != 0 ) minval = strtoul(min, NULL, 0);
    if (strcmp(max, I2V_CFG_UNDEFINED) != 0 ) maxval = strtoul(max, NULL, 0);
    if ((value >= minval) && (value <= maxval)) {
        *cfg = value;
    } 
    else if( (strcmp(min, I2V_CFG_UNDEFINED) == 0) && (strcmp(max, I2V_CFG_UNDEFINED) == 0) ) {
        /* No Min/Max limits for this value. */
        *cfg = value;
    } else {
        if (value < minval) {
            *cfg = minval;
            *status = I2V_RETURN_MIN_LIMIT_FAIL;
        } 
        if (value > maxval) {
            *cfg = maxval;
            *status = I2V_RETURN_MAX_LIMIT_FAIL;
        }
    }
    if( trange == NULL ) return;
    /* Ensure that the specified value is equal to a table discrete. */
    while( *(trange + i) != NULL ) {
        if(value == (uint8_t)strtoul(*(trange+i), NULL, 0))  {
            valueValid = WTRUE;
            break;
        }
        i++;
    }
    /* Value is not contained in discrete range */
    if( !valueValid ) {
        *status = I2V_RETURN_RANGE_LIMIT_FAIL;
    }
}
void imfUpdateFloatValue(void *configItem, char_t *val, char_t *min, char_t *max,
                 char_t  **trange, uint32_t *status, i2vLogWrapper GCC_UNUSED_VAR *logger)
{
    float32_t * cfg        = (float32_t *)configItem;
    float32_t   value      = (float32_t)strtof(val, NULL);
    float32_t   minval     = 0.0f, maxval = 0.0f;
    int32_t     i          = 0;
    bool_t      valueValid = WFALSE;

    if (strcmp(min, I2V_CFG_UNDEFINED) != 0) minval = strtof(min, NULL);
    if (strcmp(max, I2V_CFG_UNDEFINED) != 0) maxval = strtof(max, NULL);
    if ((value >= minval) && (value <= maxval)) {
        *cfg = value;
    } else if( (strcmp(min, I2V_CFG_UNDEFINED) == 0) && (strcmp(max, I2V_CFG_UNDEFINED) == 0) ) {
        /* No Min/Max limits for this value. */
        *cfg = value;
    } else {
        if (value < minval) {
            *cfg = minval;
            *status = I2V_RETURN_MIN_LIMIT_FAIL;
        } else if (value > maxval) {
            *cfg = maxval;
            *status = I2V_RETURN_MAX_LIMIT_FAIL;
        }
    }
    if( trange == NULL ) return;
    /* Ensure that the specified value is equal to a table discrete and that min/max agree with table values. */
    while( *(trange + i) != NULL ) {
        if( !valueValid && (value == strtof(*(trange+i), NULL)) ) { 
            valueValid = WTRUE;
            break;
        }
        i++;
    }
    /* Current value is not contained in discrete value range */
    if( !valueValid ) {
        *status = I2V_RETURN_RANGE_LIMIT_FAIL;  // Current value NOT in discrete range
    }
}

STATIC void imfUpdateStrValue(void *configItem, char_t *val, char_t GCC_UNUSED_VAR *min,
                           char_t GCC_UNUSED_VAR *max, char_t GCC_UNUSED_VAR  **trange, uint32_t *status, i2vLogWrapper GCC_UNUSED_VAR *logger)
{
    uint32_t   len = 0;
    char_t   * cfg = configItem;

    if ((len = strlen(val)) >= I2V_CFG_MAX_STR_LEN) {
        *status = I2V_RETURN_STR_MAX_LIMIT_FAIL;
        return;
    }
    strcpy(cfg,val);
    *status = I2V_RETURN_OK;
}

/* IMF specifc function. */
static char_t   buf[MEM_PARSE_CONF_MAX_BUF];
static char_t   dummy[MEM_PARSE_CONF_MAX_BUF];
static char_t   val[MEM_PARSE_CONF_MAX_LEN];

/* See TRAC 2698 for discussion. */

static uint32_t memGets(char_t *outbuf, uint32_t maxBytes, char_t *inbuf)
{
    uint32_t count = 0; 

    if ((outbuf == NULL) || (inbuf == NULL)) {
        return 0;
    }
    memset(outbuf, 0, maxBytes);

    while (count < maxBytes) {
        count++;
        switch (inbuf[(count-1)]) {
            /* new line characters need to be here */
            case '\n':   /* linux new line only for now */
                outbuf[(count-1)] = 0;   /* null term string */
                return count;
            default:
                outbuf[(count-1)] = inbuf[(count-1)];
        }
    }
    return count;
}
/* 
 * Similar calls in imf.c, amh.c. and amhValidator.c
 * Keeping seperate for now to avoid thread clobbering.
 */
STATIC int32_t IMFParseConfMem(char_t *inbuf, uint32_t bufsize, cfgItemsTypeT *cfgItems, uint32_t numCfgItems)
{
  bool_t    cfgFound[numCfgItems];
  uint32_t  i=0,j=0, parserStatus = I2V_RETURN_OK;
  int32_t   retVal = AMH_AOK;

  char_t  * bptr = inbuf;
  uint32_t  bytesRead = 0, remCount = bufsize;
  bool_t    looping = WTRUE;
  char_t  * ptr = NULL;
  char_t buffer[RSU_ALERT_MSG_LENGTH_MAX];

    /* Reset once only. Used to track what was found. */
    memset(cfgFound, 0, sizeof(cfgFound));  /* required for jjg comment later in this fxn */

    if (NULL == inbuf) {
#if defined(EXTRA_DEBUG)
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"IMFParseConfMem: Internal Error, bad input.\n");
#endif
        set_amh_error_state(AMH_IMF_BAD_INPUT);
        return AMH_IMF_BAD_INPUT;
    }

    while (looping) {

        /* 'contine' in loop skips init at bottom of loop. */
        memset(buf, '\0', sizeof(buf));
        memset(dummy, '\0', sizeof(dummy));
        memset(val, '\0', sizeof(val));

        bytesRead = memGets(buf, MEM_PARSE_CONF_GET_MAX_BUF, bptr);
        remCount = (bytesRead > remCount) ? 0: (remCount - bytesRead);
        if (!bytesRead || !remCount) {
            looping = WFALSE;
        }
        if (remCount) bptr += (bytesRead);
        /* Identify and remove comment sections */
        if ((ptr = strrchr(buf, '#')) != NULL) *ptr = '\0';

        /* Remove white space */
        memset(dummy, '\0', sizeof(dummy));
        for(i=0,j=0;(i < MEM_PARSE_CONF_MAX_BUF) && ('\0' != buf[i]); i++) {

            switch(buf[i]) {

                case ' ' :
                case '\t':
                case '\n':
                case '\r':

                    break;
                default:
                    dummy[j] = buf[i];
                    j++;
                    break;
            }

        }

        for(i=0;(i<MEM_PARSE_CONF_MAX_BUF) && ('\0' != dummy[i]);i++) {
            buf[i] = dummy[i];
        }

        if(i < MEM_PARSE_CONF_MAX_BUF)
            buf[i] = '\0';
        else
            buf[MEM_PARSE_CONF_MAX_BUF-1] = '\0';

        /* Blank/Comment lines */
        if (strlen(buf) == 0) {
            //set_amh_error_state(AMH_IFM_PARSE_CONF_BLANK_INPUT);/* Harmless, keep going. */
            continue;
        }
        if (buf[0] == '#') {
            //set_amh_error_state(AMH_IFM_PARSE_CONF_COMMENT_INPUT);/* Harmless, keep going. */
            continue;
        }

        /* Identify value lines */
        if ((ptr = strrchr(buf, '=')) == NULL) {
#if defined(EXTRA_DEBUG)
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%d)Rejected - No assignment. \n",getpid());
#endif
            if(0x0 == (is_amh_error_set(AMH_IFM_PARSE_CONF_BLANK_INPUT))) {
                set_amh_error_state(AMH_IFM_PARSE_CONF_BLANK_INPUT);
                memset(buffer,0x0,sizeof(buffer));
                snprintf(buffer,sizeof(buffer), "Legacy IMF: Assignment '=' with no value.");
                sendnotification(buffer, rsuAlertLevel_error);
            }
            continue;
        }

        /* Replace '=' with NULL; point ptr to value in string and copy into 'val' */
        *ptr = '\0';
        strcpy(val, ptr+1);

        /* do not print payload like below.
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"  Config:  %s:\t val: %s\n", buf, val);
        */
        for (i = 0; i < numCfgItems; i++) {
            /* custom functions may have tags that don't match length of parsed buf */

            /* 
             * If buf has more to it than cfgItems[i].tag then we still get a match so long as a custom function exists.
             * i.e. "Payload" from cfgItem.tag will match "PayloadNotForThisTaskPleaseDontUseMe" from buf[]
             */
            if (((strncmp(buf, cfgItems[i].tag, strlen(cfgItems[i].tag)) == 0) && (strlen(buf) == strlen(cfgItems[i].tag))) 
             || ((strncmp(buf, cfgItems[i].tag, strlen(cfgItems[i].tag)) == 0) && (cfgItems[i].customUpdateFxn != NULL))) {

                if (cfgItems[i].customUpdateFxn == NULL) {
                    (*cfgItems[i].updateFunction)(cfgItems[i].itemToUpdate, val, UNDEFINED, UNDEFINED, NULL, &parserStatus, NULL);
                } else {
                    /* custom functions may not print out text to log terminate open ended log msg printed above */
                    (*cfgItems[i].customUpdateFxn)(cfgItems[i].tag, buf, val, cfgItems[i].itemToUpdate, &parserStatus);
                }
                if( parserStatus == I2V_RETURN_OK ) {
                    cfgFound[i] = WTRUE;
                }
                else if(   (parserStatus == I2V_RETURN_STR_MAX_LIMIT_FAIL)
                        || (parserStatus == I2V_RETURN_MIN_LIMIT_FAIL)
                        || (parserStatus == I2V_RETURN_MAX_LIMIT_FAIL) ) {
                    /*jjg cleanup - do we really need to return here if later assuming override not set we fail anyway?
                     *pro: if we fail here it speeds up initialization time
                     *con: if we fail here and calling process doesn't check return, other config items will be missing */
                    cfgFound[i] = WFALSE;
                }
                else {
                    cfgFound[i] = WFALSE;
                }

                break;
            }
        }
    }
 
    /* Legacy item: */
    /* if(WTRUE == amhCfg.validateData) */ 
    {
        for (i = 0; i < NUMITEMS(cfgFound); i++) {
            if (!cfgFound[i]) {
                retVal = AMH_IMF_CONF_ITEM_MISSING;
                if(0x0 == (is_amh_error_set(AMH_IMF_CONF_ITEM_MISSING))) {
                    set_amh_error_state(AMH_IMF_CONF_ITEM_MISSING);
                    memset(buffer,0x0,sizeof(buffer));
                    snprintf(buffer,sizeof(buffer), "AMH_IMF_CONF_ITEM_MISSING: IMF is missing cfg items: expected(%d) actual (%lu)",i,NUMITEMS(cfgFound));
                    sendnotification(buffer, rsuAlertLevel_error);
                }
            }
        }
    }
    return retVal;
}

/* format of file is defined in RSU functional spec -
   this is v3/v4 specific 

   NOTES:
       - do not care about all the amh fields; amh only needs data and encoding
   Version=0.7
   Type=MAP16
   PSID=0x8002
   Priority=7
   TxMode=ALT
   TxChannel=172
   TxInterval=1
   DeliveryStart=04/16/2017, 01:01
   DeliveryStop=11/30/2030, 23:50
   Signature=False
   Encryption=False
   Payload1=0012....
*/
STATIC void imfParseBytes(char_t * buf, uint32_t size) 
{
    uint32_t         ret = 0;
    float32_t        dummyVersion = 0.0f;     
    uint8_t          msgInterval = 0;
    bool_t           isContinuous = WFALSE, hasSignature = WFALSE, hasEncryption = WFALSE;
    rawImmediateType msg;
    amhSendParams    amhParams;
    char_t           start_date_buffer[AMH_FILENAME_SIZE];
    char_t           stop_date_buffer[AMH_FILENAME_SIZE];
    char_t           buffer[RSU_ALERT_MSG_LENGTH_MAX];
    cfgItemsTypeT    recorddata[] = {
        /* DO NOT mix and match custom style functions with "regular".*/
        {"Version",       (void *)imfUpdateFloatValue,  &dummyVersion,       NULL,(ITEM_VALID| UNINIT)},
        {"Type",          NULL,                         &msg.type,           (void *)imfUpdateType,(ITEM_VALID| UNINIT)},
        {"PSID",          (void *)imfUpdateUint32Value, &msg.psid,           NULL,(ITEM_VALID| UNINIT)},
        {"Priority",      (void *)imfUpdateUint8Value,  &msg.priority,       NULL,(ITEM_VALID| UNINIT)},
        {"TxMode",        NULL,                         &isContinuous,       (void *)imfUpdateTxMode,(ITEM_VALID| UNINIT)},
        {"TxChannel",     NULL,                         &msg.channel,        (void *)imfUpdateTxChannel,(ITEM_VALID| UNINIT)},
        {"TxInterval",    (void *)imfUpdateUint8Value,  &msgInterval,        NULL,(ITEM_VALID| UNINIT)},
        {"DeliveryStart", (void *)imfUpdateStrValue,    start_date_buffer,   NULL,(ITEM_VALID| UNINIT)},
        {"DeliveryStop",  (void *)imfUpdateStrValue,    stop_date_buffer,    NULL,(ITEM_VALID| UNINIT)},
        {"Signature",     NULL,                         &hasSignature,       (void *)imfUpdateBool,(ITEM_VALID| UNINIT)},
        {"Encryption",    NULL,                         &hasEncryption,      (void *)imfUpdateBool,(ITEM_VALID| UNINIT)},
        {"Payload",       NULL,                         &msg.data,           (void *)imfUpdatePayload,(ITEM_VALID| UNINIT)},
    };

    memset(&msg, 0, sizeof(msg));
    memset(&start_date_buffer,'\0',sizeof(start_date_buffer));
    memset(&stop_date_buffer,'\0',sizeof(stop_date_buffer));
    msgInterval = 0;

    if(NULL == buf) {
        set_amh_error_state(AMH_IMF_BAD_INPUT);
        return;
    }
    /* Set limit for how small message could be within reason. NOT EXACT just rough check for bogus stuff. */
    if(size < MIN_SAR_FILE_SIZE) {
        if(0x0 == (is_amh_error_set(AMH_IMF_PAYLOAD_TOO_SMALL))) {
            set_amh_error_state(AMH_IMF_PAYLOAD_TOO_SMALL);
            memset(buffer,0x0,sizeof(buffer));
            snprintf(buffer,sizeof(buffer), "AMH_IMF_PAYLOAD_TOO_SMALL: IMF payload is too big: min(%d) > payload(%u)",MIN_SAR_FILE_SIZE,size);
            sendnotification(buffer, rsuAlertLevel_error);
        }
        return;
    }
    if(MEM_PARSE_CONF_GET_MAX_BUF <= size) {
        if(0x0 == (is_amh_error_set(AMH_IMF_PAYLOAD_TOO_BIG))) {
            set_amh_error_state(AMH_IMF_PAYLOAD_TOO_BIG);
            memset(buffer,0x0,sizeof(buffer));
            snprintf(buffer,sizeof(buffer), "AMH_IMF_PAYLOAD_TOO_BIG: IMF payload is too big: max(%d) <= payload(%u)",MEM_PARSE_CONF_GET_MAX_BUF,size);
            sendnotification(buffer, rsuAlertLevel_error);
        }
        return;
    }
    /* Now parse contents. Error handling inside.*/
    if (AMH_AOK != IMFParseConfMem(buf, size, recorddata, NUMITEMS(recorddata)))  {
        return;
    }

    /* new requirement to check channel after parsing */
    if (msg.channel == AMH_NEED_SCH) {
        msg.channel = amhCfg.channelNum;   /* service channel */
    }
    
    /* SAR (0 < txInterval) specific handling here. 
     * Will check stop, start & msg type then store file.
     * AMG main processing will pick up file to deal with if "valid".
     */
    if(   (amhCfg.EnableSAR)    //If SAR is desired.
       && (amhCfg.validateData) //If so we must validate data otherwise blow up file system.
       && (msgInterval)) {      //SAR is txInterval greater than 0.

        /* If we are full nothing to do.*/
        if(amhstoredbcnt < RSU_SAR_MESSAGE_MAX){
            if (AMH_AOK != Process_Store_and_Repeat(&msg,dummyVersion,buf,size,start_date_buffer,stop_date_buffer)) {
                if(0x0 == (is_amh_error_set(AMH_SAR_REJECTED))) {
                    set_amh_error_state(AMH_SAR_REJECTED);
                    memset(buffer,0x0,sizeof(buffer));
                    snprintf(buffer,sizeof(buffer), "AMH_SAR_REJECTED: Please review SAR files being sent via IMF socket: amhstoredbcnt(%d)",amhstoredbcnt);
                    sendnotification(buffer, LEVEL_WARN);
                }
            }
        } else {
            /* RSU is full. Reject and notify user. */
            if(0x0 == (is_amh_error_set(AMH_SAR_FULL_WARNING))) {
                set_amh_error_state(AMH_SAR_FULL_WARNING);
                memset(buffer,0x0,sizeof(buffer));
                snprintf(buffer,sizeof(buffer), "AMH_SAR_FULL_WARNING: AMH dir is full: max(%d) <= amhstoredbcnt(%d)",RSU_SAR_MESSAGE_MAX,amhstoredbcnt);
                sendnotification(buffer, LEVEL_WARN);
            }
        }

/* 
 * SAR handling stops here. Tricky.
 * Now file in AMH dir waiting for AMC main process to tx.
 * SAR files have stop & start dates that have to be eval'ed before sending.
 * Other book keeping things must be done to track it properly over time.
 * Not doing that here but something to be aware of: extra handling of SAR after this point.
 * This is why we don't just contine to send SAR message out like IMF below.
 */
        return;
    } 

/* 
 * IMF handling continues here. 
 * Because there is no file to save and no need to track item(book keeping),
 * we inject msg directly into AMH data SHM for one time tx.  
 */

    /* validation */
    if (amhCfg.validateData) {

        if(0 != msgInterval) {   //This can only happpen is SAR file which already returned above? */
            if(0x0 == (is_amh_error_set(AMH_IMF_BAD_TXINTERVAL))) {
                set_amh_error_state(AMH_IMF_BAD_TXINTERVAL);
                memset(buffer,0x0,sizeof(buffer));
                snprintf(buffer,sizeof(buffer), "AMH_IMF_BAD_TXINTERVAL: interval for IMF is not zero.");
                sendnotification(buffer, rsuAlertLevel_error);
            }
            return;        
        }

        /* broadcast required validation */
        if (hasEncryption) {
            if(0x0 == (is_amh_error_set(AMH_IMF_ENCRYPT_REJECT))) {
                set_amh_error_state(AMH_IMF_ENCRYPT_REJECT);
                memset(buffer,0x0,sizeof(buffer));
                snprintf(buffer,sizeof(buffer), "AMH_IMF_ENCRYPT_REJECT: IMF does not support encyrption.");
                sendnotification(buffer, rsuAlertLevel_error);
            }
            return;
        }

        /* only check when security disabled and signature enabled - there is a way to turn off signing with security enabled */
        if (hasSignature && !cfg.security) {
            if(0x0 == (is_amh_error_set(AMH_IMF_SECURITY_REJECT))) {
                set_amh_error_state(AMH_IMF_SECURITY_REJECT);
                memset(buffer,0x0,sizeof(buffer));
                snprintf(buffer,sizeof(buffer), "AMH_IMF_SECURITY_REJECT: IMF does not support security. Check security settings for RSU.");
                sendnotification(buffer, rsuAlertLevel_error);
            }
            return;
        }
    }
    /* force compliance with I2V unified settings */
    if ((msg.channel != cfg.uchannel) && (msg.channel != I2V_CONTROL_CHANNEL)) {
        if(0x0 == (is_amh_error_set(AMH_IMF_CHANNEL_ERROR))) {
            set_amh_error_state(AMH_IMF_CHANNEL_ERROR);
            memset(buffer,0x0,sizeof(buffer));
            snprintf(buffer,sizeof(buffer), "AMH_IMF_CHANNEL_ERROR: Enforcing radio channel compliance. Expect channel(%d) but got (%d).",cfg.uchannel,msg.channel);
            sendnotification(buffer, rsuAlertLevel_warning);
        }
        msg.channel = cfg.uchannel;  /*force compliance*/
    }

    /* put in memory and alert amh */
    switch (msg.type) {
        case AMH_MSG_RTCM_MASK:
        case AMH_MSG_SPAT_MASK:
        case AMH_MSG_MAP_MASK:
        case AMH_MSG_TIM_MASK:
        case AMH_MSG_ICA_MASK:
        case AMH_MSG_PDM_MASK:
        case AMH_MSG_RSA_MASK:
        case AMH_MSG_SSM_MASK:
        case AMH_MSG_CSR_MASK:
        case AMH_MSG_EVA_MASK:
        case AMH_MSG_NMEA_MASK:
        case AMH_MSG_PSM_MASK:
        case AMH_MSG_PVD_MASK:
        case AMH_MSG_SRM_MASK:
        case AMH_MSG_BSM_MASK:
        case AMH_MSG_TEST00_MASK:
        case AMH_MSG_TEST01_MASK:
        case AMH_MSG_TEST02_MASK:
        case AMH_MSG_TEST03_MASK:
        case AMH_MSG_TEST04_MASK:
        case AMH_MSG_TEST05_MASK:
        case AMH_MSG_TEST06_MASK:
        case AMH_MSG_TEST07_MASK:
        case AMH_MSG_TEST08_MASK:
        case AMH_MSG_TEST09_MASK:
        case AMH_MSG_TEST10_MASK:
        case AMH_MSG_TEST11_MASK:
        case AMH_MSG_TEST12_MASK:
        case AMH_MSG_TEST13_MASK:
        case AMH_MSG_TEST14_MASK:
        case AMH_MSG_TEST15_MASK:
            break;
        default:
            if(0x0 == (is_amh_error_set(AMH_IMF_MSGTYPE_REJECT))) {
                set_amh_error_state(AMH_IMF_MSGTYPE_REJECT);
                memset(buffer,0x0,sizeof(buffer));
                snprintf(buffer,sizeof(buffer), "AMH_IMF_MSGTYPE_REJECT: Rejecting msg type(%d)", msg.type);
                sendnotification(buffer, rsuAlertLevel_error);
            }
            return;
    }
    memset(&amhParams, 0, sizeof(amhSendParams));
    amhParams.psid     = msg.psid;
    amhParams.priority = msg.priority;
    amhParams.security = UNSECURED_WSA;

    if(cfg.security) {
      amhParams.security = (hasSignature) ? SECURED_WSA : UNSECURED_WSA;
    }

    amhParams.useCCH = (msg.channel == I2V_CONTROL_CHANNEL) ? WTRUE : WFALSE;   /* control channel interval */

    while (0 != pthread_mutex_lock(&amhRadio_lock)){
        usleep(500); 
    }
    if(I2V_RETURN_OK != (ret = amhTxMessage(msg.data.payload, msg.data.count, &amhParams))){
        if(0x0 == (is_amh_error_set(AMH_IFM_LEGACY_WSM_SEND_FAIL))) {
            set_amh_error_state(AMH_IFM_LEGACY_WSM_SEND_FAIL);
            memset(buffer,0x0,sizeof(buffer));
            snprintf(buffer,sizeof(buffer), "AMH_IFM_LEGACY_WSM_SEND_FAIL: Radio failed to send msg psid(0x%x) ret(%d)", msg.psid, ret);
            sendnotification(buffer, rsuAlertLevel_error);
        }
    }
    pthread_mutex_unlock(&amhRadio_lock);
    #if defined BSM_PSM_FORWARDING
    switch (msg.type) {
        case AMH_MSG_PSM_MASK:
            WSU_SHMLOCK_LOCKW(&shmPtr->cfgData.h.ch_lock);
            memcpy(shmPtr->amhImmediatePsm.data, msg.data.payload, msg.data.count);
            shmPtr->amhImmediatePsm.count  = msg.data.count;
            shmPtr->amhImmediatePsm.newmsg = WTRUE;  /* Clear on consumption. */
            WSU_SHMLOCK_UNLOCKW(&shmPtr->cfgData.h.ch_lock);
            break;
        case AMH_MSG_BSM_MASK:
            WSU_SHMLOCK_LOCKW(&shmPtr->cfgData.h.ch_lock);
            memcpy(shmPtr->amhImmediateBsm.data, msg.data.payload, msg.data.count);
            shmPtr->amhImmediateBsm.count  = msg.data.count;
            shmPtr->amhImmediateBsm.newmsg = WTRUE;  /* Clear on consumption. */
            WSU_SHMLOCK_UNLOCKW(&shmPtr->cfgData.h.ch_lock);
            break;
        default:
            break;
    }
    #endif
    /* Forward message via socket to user. */
    amhForwardManager(dsrcMsgId_to_amhMsgType(msg.data.payload[1]), msg.data.payload, msg.data.count);
    update_imf_count(ret, msg.type); /* Track stats. */
}
/* Legacy RSU 4.1 and older. */
void *imfThread(void __attribute__((unused)) *arg)
{
  int32_t size = 0, sock = -1;
  struct sockaddr_in remote;
  uint32_t remSize = sizeof(remote);
  uint32_t timeout = DEFAULT_TIMEOUT;
  char_t   recvbuf[MAX_UDP_SIZE];
  int32_t ret = 0;
  struct pollfd read_poll;
  char_t buffer[RSU_ALERT_MSG_LENGTH_MAX];
#if defined(MY_UNIT_TEST)
  pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL); //useful?
#endif

  while (mainloop) {
      if (-1 != (sock = imfSockInit())) { /* Error handling and reporting in imfSockInit(). Nothing to track at this level. */
          while (mainloop) {
              /*
               * If thresholds met then reset for the next interation to re-open.
               * Fails given equal weight today but that could be fined tuned.
               * Key is it has to be consecutive fails to avoid reset on just intermittent fails.
               * There's lots of event syslog output for intermittent fails if user cares.
               */
              if(   (IMF_READ_ERROR_THRESHOLD < imf_read_error_count)
                 || (IMF_READ_ERROR_THRESHOLD < imf_read_poll_notready_count)
                 || (IMF_READ_ERROR_THRESHOLD < imf_read_poll_fail_count)) {

                   /* Reset all now. */
                  close(sock);
                  sock = -1;
                  imf_socket_reset_count++;
#if defined(EXTRA_DEBUG)
                  I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"imfThread: Resetting socket for %u time, Re:!RDY:TO:TOe[%u,%u,%u,%u]\n",
                      imf_socket_reset_count,imf_read_error_count, imf_read_poll_notready_count, imf_read_poll_to_count, imf_read_poll_fail_count);
#endif
                  imf_read_error_count = 0;
                  imf_read_poll_notready_count = 0;
                  imf_read_poll_to_count   = 0;
                  imf_read_poll_fail_count = 0;
                  break; /* Break from while and head for socket re-init. */
              }

              memset(&remote, 0, sizeof(remote));
              memset(recvbuf, 0, sizeof(recvbuf));
              /*
               * poll() socket to see if Data Terminal Ready.
               */
              read_poll.fd      = sock; /* May change, who knows so just re-add. */
              read_poll.events  = POLLIN;
              read_poll.revents = 0;
              errno = 0;
              if(0 < (ret=poll(&read_poll,1,SOCKET_POLL_TIMEOUT))) {
                  if(read_poll.revents & POLLIN) {
                      /* DTR */
                      if (0 < (size = recvfrom(sock, recvbuf, MAX_UDP_SIZE, 0, (struct sockaddr *) &remote, &remSize))) {
                          imf_read_error_count = 0; /* Reset thresholds. */
                          imf_read_poll_to_count = 0;
                          imf_read_poll_notready_count = 0;
                          imf_read_poll_fail_count = 0;
                          if (amhCfg.restrictIP) {
                              /* validate source */
                              if (strcmp(inet_ntoa(remote.sin_addr), (const char_t *)amhCfg.imfIP)) {
                                  timeout = DEFAULT_TIMEOUT;
                                  if(0x0 == (is_amh_error_set(AMH_IMF_RESTRICT_IP))) {
                                      set_amh_error_state(AMH_IMF_RESTRICT_IP);
                                      memset(buffer,0x0,sizeof(buffer));
                                      snprintf(buffer,sizeof(buffer), "AMH_IMF_RESTRICT_IP: amhCfg.restrictIP rejects incoming ip(%s)", inet_ntoa(remote.sin_addr));
                                      sendnotification(buffer, rsuAlertLevel_error);
                                  }
                                  continue;    /* address didn't match, ignore it */
                              }
                          }  
                          /* got a packet to parse */
                          imfParseBytes(recvbuf, size);
                          timeout = MIN_TIMEOUT;
                          #if defined(EXTRA_EXTRA_DEBUG)
                          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"got data=%d bytes.\n",size);
                          #endif
                          imf_read_count++;
                      } else {
                          if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
                              /* no data to process, use normal timeout */
                              timeout = DEFAULT_TIMEOUT;
                          } else {
                              set_amh_error_state(AMH_IMF_RECVFROM_ERROR); /* Not fatal, keep going. Just means users has stopped sending messages. */
                              timeout = ERR_TIMEOUT;
                              imf_read_error_count++;
                              total_imf_read_error_count++;
                          }
                      } 
                  } else {
                     timeout = DEFAULT_TIMEOUT;
                     imf_read_poll_notready_count++;
                     /* !DTR */
#if defined(EXTRA_DEBUG)
                     I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"AMH_IMF_READ_NOT_READY: strerror(%d)=(%s)\n",errno,strerror(errno));
#endif
                     set_amh_error_state(AMH_IMF_READ_NOT_READY);
                  }
              } else {
                 timeout = DEFAULT_TIMEOUT;
                 if(0 == ret) {
                     /* TO: Going to happen alot assuming data coming in at relatively low rate. */
                     if(0 == (imf_read_poll_to_count % 10000)) {
#if defined(EXTRA_DEBUG)
                         if(errno){
                             I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"AMH_IMF_READ_POLL_TIMEOUT: strerror(%d)=(%s)\n",errno,strerror(errno));
                         }
#endif
                     }
                     imf_read_poll_to_count++;
                 } else {
                     /* poll() malfunction, output errno. */
                     imf_read_poll_fail_count++;
#if defined(EXTRA_DEBUG)
                     I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"imf_read_poll_fail_count(%u): strerror(%d)=(%s)\n",imf_read_poll_fail_count,errno,strerror(errno));
#endif
                 }
              }
              usleep(timeout);
              #if defined(MY_UNIT_TEST)
              break;
              #endif
          } /* while */
      } /* if sock init */
      sleep(5); /* If we got here the socket failed so back off a bit. */
  } /* while */
  if(sock) {
      close(sock);
      sock = -2; /* Normal exit. */
  }
  if(WTRUE == mainloop) {
      sock = -3; /* Abnormal exit indicator. */
  }
//TODO: dupe bit def
  set_amh_error_state(AMH_CREATE_IMF_THREAD_FAIL);
#if defined(EXTRA_DEBUG)
  I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"imfThread: sock=%d exit.\n",sock);
#endif
  pthread_exit(NULL);
}
/* NTCIP-1218 rsuIFM support. */

/* 
 * Well formed means message is valid completely and ready to send
 * If we find anything we dont like then we make row status SNMP_ROW_NOTREADY.
 */
STATIC int32_t msgIFM_wellformed(RsuIFMStatusEntry_t * tmp_msgIFM) 
{
  int32_t  ret  = AMH_AOK;
  uint32_t psid = 0x0;
  char_t buffer[RSU_ALERT_MSG_LENGTH_MAX];

  if(NULL == tmp_msgIFM){
      ret = AMH_IMF_BAD_INPUT;
      set_amh_error_state(ret);
  } else {
      if((tmp_msgIFM->rsuIFMPsid_length < RSU_RADIO_PSID_MIN_SIZE) || (RSU_RADIO_PSID_SIZE < tmp_msgIFM->rsuIFMPsid_length)){
          tmp_msgIFM->rsuIFMPsid_length = RSU_RADIO_PSID_MIN_SIZE; /* Not acessible by client so internal error. */
          ret = AMH_IMF_BAD_INPUT;
          set_amh_error_state(ret);
      } else {
          psid  = ((uint32_t)(tmp_msgIFM->rsuIFMPsid[0])) << 8;
          psid |= (uint32_t)tmp_msgIFM->rsuIFMPsid[1];
          psid  = psid << 8;
          psid |= (uint32_t)tmp_msgIFM->rsuIFMPsid[2];
          psid  = psid << 8;
          psid |= (uint32_t)tmp_msgIFM->rsuIFMPsid[3];
          /* We can probably use just one? From ris.c: Validate PSID. */
          if(psid <= (uint32_t) PSID_1BYTE_MAX_VALUE){ 
          } else if(psid >= (uint32_t)PSID_2BYTE_MIN_VALUE && psid <= (uint32_t)PSID_2BYTE_MAX_VALUE){
          } else if(psid >= (uint32_t)PSID_3BYTE_MIN_VALUE && psid <= (uint32_t)PSID_3BYTE_MAX_VALUE){
          } else if(psid >= (uint32_t)PSID_4BYTE_MIN_VALUE && psid <= (uint32_t)PSID_4BYTE_MAX_VALUE){
          } else {
              ret = AMH_IMF_INVALID_PSID;
              if(0x0 == (is_amh_error_set(AMH_IMF_INVALID_PSID))) {
                  set_amh_error_state(AMH_IMF_INVALID_PSID);
                  memset(buffer,0x0,sizeof(buffer));
                  snprintf(buffer,sizeof(buffer), "AMH_IMF_INVALID_PSID: bad PSID(0x%x)", psid);
                  sendnotification(buffer, rsuAlertLevel_error);
              }
          }
      }
      /* General range error for channel. */
      if((AMH_AOK == ret) && ((tmp_msgIFM->rsuIFMTxChannel < RSU_RADIO_CHANNEL_MIN) || (RSU_RADIO_CHANNEL_MAX < tmp_msgIFM->rsuIFMTxChannel))){
          tmp_msgIFM->rsuIFMTxChannel = RSU_RADIO_CHANNEL_MIN; /* TODO: needed still? */
          ret = AMH_IMF_CHANNEL_ERROR;
          if(0x0 == (is_amh_error_set(AMH_IMF_CHANNEL_ERROR))) {
              set_amh_error_state(AMH_IMF_CHANNEL_ERROR);
              memset(buffer,0x0,sizeof(buffer));
              snprintf(buffer,sizeof(buffer), "AMH_IMF_CHANNEL_ERROR: Bad channel(%d)",tmp_msgIFM->rsuIFMTxChannel);
              sendnotification(buffer, rsuAlertLevel_error);
          }
      }
      /* Specific range for different radio types. */
      if(AMH_AOK == ret) {
          if(0 == cfg.RadioType){
              if(tmp_msgIFM->rsuIFMTxChannel != cfg.uchannel){
                  ret = AMH_IMF_CHANNEL_ERROR;
                  if(0x0 == (is_amh_error_set(AMH_IMF_CHANNEL_ERROR))) {
                      set_amh_error_state(AMH_IMF_CHANNEL_ERROR);
                      memset(buffer,0x0,sizeof(buffer));
                      snprintf(buffer,sizeof(buffer), "AMH_IMF_CHANNEL_ERROR: Bad channel(%d) expected(%d): Check RSU configs.",tmp_msgIFM->rsuIFMTxChannel,cfg.uchannel);
                      sendnotification(buffer, rsuAlertLevel_error);
                  }
              }
          } else {
              if(tmp_msgIFM->rsuIFMTxChannel != RSU_CV2X_CHANNEL_DEFAULT){
                  ret = AMH_IMF_CHANNEL_ERROR;
                  if(0x0 == (is_amh_error_set(AMH_IMF_CHANNEL_ERROR))) {
                      set_amh_error_state(AMH_IMF_CHANNEL_ERROR);
                      memset(buffer,0x0,sizeof(buffer));
                      snprintf(buffer,sizeof(buffer), "AMH_IMF_CHANNEL_ERROR: Bad channel(%d) expected(%d): Check RSU configs.",tmp_msgIFM->rsuIFMTxChannel,RSU_CV2X_CHANNEL_DEFAULT);
                      sendnotification(buffer, rsuAlertLevel_error);
                  }
              }
          }
      }
      /* rsuIFMPayload: How to check 2.5k binary data? Assuming dsrcMsgId and a few bytes at least. */
      if((AMH_AOK == ret) && (tmp_msgIFM->rsuIFMPayload_length < (RSU_MSG_PAYLOAD_MIN+2))){
          ret = AMH_IMF_PAYLOAD_TOO_SMALL;
          if(0x0 == (is_amh_error_set(AMH_IMF_PAYLOAD_TOO_SMALL))) {
              set_amh_error_state(AMH_IMF_PAYLOAD_TOO_SMALL);
              memset(buffer,0x0,sizeof(buffer));
              snprintf(buffer,sizeof(buffer), "AMH_IMF_PAYLOAD_TOO_SMALL: bad payload length, too small(%d) < (%d)\n",tmp_msgIFM->rsuIFMPayload_length,(RSU_MSG_PAYLOAD_MIN+2));
              sendnotification(buffer, rsuAlertLevel_error);
          }
      } 
      if((AMH_AOK == ret) && (RSU_MSG_PAYLOAD_MAX < tmp_msgIFM->rsuIFMPayload_length)){
          ret = AMH_IMF_PAYLOAD_TOO_BIG;
          if(0x0 == (is_amh_error_set(AMH_IMF_PAYLOAD_TOO_BIG))) {
              set_amh_error_state(AMH_IMF_PAYLOAD_TOO_BIG);
              memset(buffer,0x0,sizeof(buffer));
              snprintf(buffer,sizeof(buffer), "AMH_IMF_PAYLOAD_TOO_BIG: bad payload length, too big(%d) > (%d)\n",tmp_msgIFM->rsuIFMPayload_length,RSU_MSG_PAYLOAD_MAX);
              sendnotification(buffer, rsuAlertLevel_error);
          }
      } 
#if defined(ENABLE_DSRC_MSG_ID_CHECK) /* NTCIP-1218 does not mention using payload like this. Maybe feature for the future. */
      /* Must be a valid DSRC MSG ID. */
      if((AMH_AOK == ret) && (0 == dsrcMsgId_to_amhMsgType(tmp_msgIFM->rsuIFMPayload[1]))){
          ret = AMH_IFM_STAT_BAD_FORM;
          if(0x0 == (is_amh_error_set(AMH_IFM_STAT_BAD_FORM))) {
              set_amh_error_state(AMH_IFM_STAT_BAD_FORM);
              memset(buffer,0x0,sizeof(buffer));
              snprintf(buffer,sizeof(buffer), "AMH_IFM_STAT_BAD_FORM: bad Uper DSRC MSG ID[0x%X]:\n",tmp_msgIFM->rsuIFMPayload[1]);
              sendnotification(buffer, rsuAlertLevel_error);
          }
      }
#endif
      if((AMH_AOK == ret) && ((tmp_msgIFM->rsuIFMEnable < 0) || (1 < tmp_msgIFM->rsuIFMEnable))){
          ret = AMH_IFM_STAT_BAD_FORM;
          if(0x0 == (is_amh_error_set(AMH_IFM_STAT_BAD_FORM))) {
              set_amh_error_state(AMH_IFM_STAT_BAD_FORM);
              memset(buffer,0x0,sizeof(buffer));
              snprintf(buffer,sizeof(buffer), "AMH_IFM_STAT_BAD_FORM: rsuIFMEnable == (%d): not a 1 or 0",tmp_msgIFM->rsuIFMEnable);
              sendnotification(buffer, rsuAlertLevel_error);
          }
      }
      if((AMH_AOK == ret) && ((tmp_msgIFM->rsuIFMStatus < SNMP_ROW_NONEXISTENT) || (SNMP_ROW_DESTROY < tmp_msgIFM->rsuIFMStatus))){
          ret = AMH_IFM_STAT_BAD_FORM;
          if(0x0 == (is_amh_error_set(AMH_IFM_STAT_BAD_FORM))) {
              set_amh_error_state(AMH_IFM_STAT_BAD_FORM);
              memset(buffer,0x0,sizeof(buffer));
              snprintf(buffer,sizeof(buffer), "AMH_IFM_STAT_BAD_FORM: bad rsuIFMStatus(%d)",tmp_msgIFM->rsuIFMStatus);
              sendnotification(buffer, rsuAlertLevel_error);
          }
      } 
      if((AMH_AOK == ret) && ((tmp_msgIFM->rsuIFMPriority < RSU_IFM_PRIORITY_MIN) || (RSU_IFM_PRIORITY_MAX < tmp_msgIFM->rsuIFMPriority))){
          ret = AMH_IFM_STAT_BAD_FORM;
          if(0x0 == (is_amh_error_set(AMH_IFM_STAT_BAD_FORM))) {
              set_amh_error_state(AMH_IFM_STAT_BAD_FORM);
              memset(buffer,0x0,sizeof(buffer));
              snprintf(buffer,sizeof(buffer), "AMH_IFM_STAT_BAD_FORM: bad rsuIFMPriority(%d)",tmp_msgIFM->rsuIFMPriority);
              sendnotification(buffer, rsuAlertLevel_error);
          }
      }
      if(~(RSU_IFM_OPTIONS_MASK) & tmp_msgIFM->rsuIFMOptions){ /* Erroneous bit check. */
          ret = AMH_IMF_BAD_OPTIONS;
          if(0x0 == (is_amh_error_set(AMH_IMF_BAD_OPTIONS))) {
              set_amh_error_state(AMH_IMF_BAD_OPTIONS);
              memset(buffer,0x0,sizeof(buffer));
              snprintf(buffer,sizeof(buffer), "AMH_IMF_BAD_OPTIONS: bad rsuIFMOptions(0x%X)",tmp_msgIFM->rsuIFMOptions);
              sendnotification(buffer, rsuAlertLevel_error);
          }
      }
      /* The only columns IFM writes to are status and enable. If there's bogus data above not our problem. Just dont send message out. */
      if(AMH_AOK != ret) {
          /* Furure: Perhaps we can use the traps to inform user of fail? Need feed back to client. */
          tmp_msgIFM->rsuIFMStatus = SNMP_ROW_NOTREADY; /* Client will NOT see transaction fail today. No ACK back to MIB. */
          tmp_msgIFM->rsuIFMEnable = 0;                 /* Extremely terse response for failure but its what SNMP supports. */
          /*print_msgIFM(tmp_msgIFM); Add this is you need it. Today no need. */
      }
  }
  return ret;
}

STATIC mqd_t ifmQueue;
void close_IFM_queue (void)
{
  mq_unlink(IFM_MIB_QUEUE);
  mq_close(ifmQueue);
}
int32_t open_IFM_queue(void)
{
  struct mq_attr attr;
  /* Open or Create AMH Queue to MIB */
  errno = 0;
  attr.mq_flags   = IFM_QUEUE_DEFAULT_FLAGS; /* ignored on open, use with set */
  attr.mq_maxmsg  = IFM_QUEUE_DEPTH; 
  attr.mq_msgsize = IFM_QUEUE_MSG_SIZE;
  attr.mq_curmsgs = 0;
  /* Will likely see errno 17 for file existening. If we don't it means MIB isn't started. */
  if(-1 == (ifmQueue = mq_open(IFM_MIB_QUEUE, (O_RDWR|O_CREAT|O_NONBLOCK), 0644, &attr))) {
      if((0 != errno) && (17 != errno)){
#if defined(EXTRA_DEBUG)
          I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"ifm mq_open: errno=%d(%s)\n",errno,strerror(errno));
#endif
          return AMH_OPEN_MQ_FAIL;
      }
  }
  attr.mq_flags   = 0;
  attr.mq_maxmsg  = 0;
  attr.mq_msgsize = 0;
  attr.mq_curmsgs = 0;
  if( -1 == (mq_getattr(ifmQueue,&attr))) {
#if defined(EXTRA_DEBUG)
      I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,": ifm mq_getattr: fail: errno=%d(%s)\n",errno,strerror(errno));
#endif
      return AMH_OPEN_MQ_FAIL;
  }
  #if defined(EXTRA_EXTRA_DEBUG)
  I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,": ifmQueue: mq_flags=0x%x mq_maxmsg=%ld mq_msgsize=%ld mq_curmsgs=%ld.\n", (uint32_t)attr.mq_flags,attr.mq_maxmsg,attr.mq_msgsize,attr.mq_curmsgs);
  #endif
  /* Check return values against our programmed values */
  if((attr.mq_maxmsg != IFM_QUEUE_DEPTH) || (attr.mq_msgsize != IFM_QUEUE_MSG_SIZE)){
#if defined(EXTRA_DEBUG)
      I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,": ifmQueue: attributes wrong: errno=%d(%s)\n", errno,strerror(errno));
#endif
      return AMH_OPEN_MQ_FAIL;
  }
  return AMH_AOK;
}

/* NTCIP-1218 MIB support. */
void *mib_rsuIFM_thread(void __attribute__((unused)) *arg)
{
  int32_t  ret = 0;
  RsuIFMStatusEntry_t amh_rsuIFM;
  amhSendParams amhParams;
  amhBitmaskType amh_msgType = 0x0;   /* Bitmask of msg types. Not dsrc_id. */
  int32_t limit_debug = 0;
  #if defined(EXTRA_DEBUG)
  uint32_t thread_epoch_counter = 0;
  uint32_t message_processed_count = 0; 
  #endif

#if defined(EXTRA_DEBUG)
  I2V_DBG_LOG(MY_ERR_LEVEL,"rsuIFM","Entering mainloop.\n");
#endif
  while (mainloop) {
      memset(&amh_rsuIFM, 0, sizeof(amh_rsuIFM));
      memset(&amhParams,0,sizeof(amhParams));
      #if defined(EXTRA_DEBUG)
      if(0 == (thread_epoch_counter++ % 5000)) {
          I2V_DBG_LOG(MY_ERR_LEVEL,"rsuIFM","IFM Messages processed from MIB=(0x%X) epoch=(0x%X).\n",message_processed_count,thread_epoch_counter);
      }
      thread_epoch_counter++;
      #endif
      ret = mq_receive(ifmQueue, (char_t *)&amh_rsuIFM, IFM_QUEUE_MSG_SIZE, NULL); /* non-blocking? Can be blocking.*/
      if(ret < 1){ /* Error or no data(EAGAIN) */ 
          if(EAGAIN != errno) {
#if defined(EXTRA_DEBUG)
              if(limit_debug<10) {
                  I2V_DBG_LOG(MY_ERR_LEVEL,"rsuIFM","Got mq error: errno=%d(%s).\n",errno,strerror(errno));
              }
#endif
              limit_debug++;
              set_amh_error_state(AMH_IFM_MQ_FAIL);
              sleep(1); /* back off if really error */
          }
          usleep(1000 * 30); /* non-blocking so EAGAIN is ok */
          continue;
      }
      if (IFM_QUEUE_MSG_SIZE == ret) { /* Should be exactly IFM_QUEUE_MSG_SIZE */
          amhParams.psid  = ((uint32_t)(amh_rsuIFM.rsuIFMPsid[0])) << 8;
          amhParams.psid |= (uint32_t)amh_rsuIFM.rsuIFMPsid[1];
          amhParams.psid  = (amhParams.psid << 8);
          amhParams.psid |= (uint32_t)amh_rsuIFM.rsuIFMPsid[2];
          amhParams.psid  = (amhParams.psid << 8);
          amhParams.psid |= (uint32_t)amh_rsuIFM.rsuIFMPsid[3];
          amhParams.priority = (uint8_t)amh_rsuIFM.rsuIFMPriority;
          amhParams.useCCH   = amh_rsuIFM.rsuIFMTxChannel;
          if(IFM_OPT_1609_BYPASS_OFF_MASK & amh_rsuIFM.rsuIFMOptions){
              amhParams.dsrcMsgId = (uint32_t)amh_rsuIFM.rsuIFMPayload[1]; /* First byte is always 0x00. Second has our value. */
              if(IFM_OPT_SECURITY_OFF_MASK & amh_rsuIFM.rsuIFMOptions){
                  amhParams.security = UNSECURED_WSA;
              } else {
                  amhParams.security = SECURED_WSA;
              }
          } else {
              /* TODO: Careful, have to account for TLV bytes. Mark unknown for now. */
              amhParams.dsrcMsgId = AMH_MSG_UNKNOWN_MASK;
              amhParams.security = UNSECURED_WSA;
          }
          amh_msgType = dsrcMsgId_to_amhMsgType(amhParams.dsrcMsgId); /* Only need this for forwarding mask. */

          /* Validate before sending. In theory done by MIB already. */
          if(AMH_AOK == (ret = msgIFM_wellformed(&amh_rsuIFM))){
              while (0 != pthread_mutex_lock(&amhRadio_lock)){
                  usleep(500); 
              }
              if(I2V_RETURN_OK != (ret = amhTxMessage(amh_rsuIFM.rsuIFMPayload, amh_rsuIFM.rsuIFMPayload_length, &amhParams))){
                  set_amh_error_state(AMH_IFM_MIB_WSM_SEND_FAIL);
              }
              pthread_mutex_unlock(&amhRadio_lock);

              #if defined BSM_PSM_FORWARDING
              switch (amh_msgType) {
              case AMH_MSG_PSM_MASK:
                  WSU_SHMLOCK_LOCKW(&shmPtr->cfgData.h.ch_lock);
                  memcpy(shmPtr->amhImmediatePsm.data, amh_rsuIFM.rsuIFMPayload, amh_rsuIFM.rsuIFMPayload_length);
                  shmPtr->amhImmediatePsm.count  = amh_rsuIFM.rsuIFMPayload_length;
                  shmPtr->amhImmediatePsm.newmsg = WTRUE;  /* Clear on consumption. */
                  WSU_SHMLOCK_UNLOCKW(&shmPtr->cfgData.h.ch_lock);
                  break;
              case AMH_MSG_BSM_MASK:
                  WSU_SHMLOCK_LOCKW(&shmPtr->cfgData.h.ch_lock);
                  memcpy(shmPtr->amhImmediateBsm.data, amh_rsuIFM.rsuIFMPayload, amh_rsuIFM.rsuIFMPayload_length);
                  shmPtr->amhImmediateBsm.count  = amh_rsuIFM.rsuIFMPayload_length;
                  shmPtr->amhImmediateBsm.newmsg = WTRUE;  /* Clear on consumption. */
                  WSU_SHMLOCK_UNLOCKW(&shmPtr->cfgData.h.ch_lock);
                  break;
              default:
                  break;
              }
              #endif
              /* Forward message via socket to user. */
              amhForwardManager(amh_msgType, amh_rsuIFM.rsuIFMPayload, amh_rsuIFM.rsuIFMPayload_length);
              update_imf_count(ret, dsrcMsgId_to_amhMsgType(amh_rsuIFM.rsuIFMPayload[1])); /* Track stats. */
              #if defined(EXTRA_DEBUG)
              message_processed_count++; /* Good or bad count it. */
              #endif
          }
#if defined(EXTRA_DEBUG)
          else {
              /* Notifications and error check within function. */
              I2V_DBG_LOG(MY_ERR_LEVEL,"msgIFM_wellformed failed ret(%d)\n",ret);
          }
#endif    
      } else {
#if defined(EXTRA_DEBUG)
          I2V_DBG_LOG(MY_ERR_LEVEL,"rsuIFM","mq fault: ret=%d is not valid size != IFM_QUEUE_MSG_SIZE(%d).\n", ret, IFM_QUEUE_MSG_SIZE);
#endif
          set_amh_error_state(AMH_IFM_QUEUE_MSG_SIZE_FAIL);
      }
      /* Future: How does the MIB know? Need to queue up ACK. */
      ret = 0; /* we have consumed it */
      usleep(500 * 1); /* Need to be timely but not a hog. Be considerate.*/
#if defined(MY_UNIT_TEST)
      break;
#endif
  }/* While */
  #if defined(EXTRA_EXTRA_DEBUG)
  I2V_DBG_LOG(MY_ERR_LEVEL,"rsuIFM","exit.\n");
  #endif
  pthread_exit(NULL);
}

