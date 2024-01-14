/**************************************************************************
 *                                                                        *
 *     File Name:  rsuAsync.c                                             *
 *     Author:                                                            *
 *         DENSO International America, Inc.                              *
 *         North America Research Laboratory, California Office           *
 *         Rutherford Road                                                *
 *         Carlsbad, 92008                                                *
 **************************************************************************/
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <pthread.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <errno.h>
#include "rsutable.h"
#include "ntcip-1218.h"
#include "i2v_util.h"
#include "rsuhealth.h"
#if defined(MY_UNIT_TEST)
#include "stubs.h"
#endif

/*
 * Defines.
 */

/* Mandatory: For SNMP debugging and output. */
#define MY_NAME       "rsuAsync"
#define MY_NAME_EXTRA "XrsuAsync"

/* Mandatory: For debugging and syslog output. */
#define MY_I2V_NAME   MY_NAME
#define MY_ERR_LEVEL  LEVEL_PRIV    /* from i2v_util.h */

#if defined(EXTRA_DEBUG)
#define OUTPUT_MODULUS  30 /* Seconds. */
#else
#define OUTPUT_MODULUS  1200 /* Seconds. */
#endif
/* Theoritical column ids. Make sure they follow in order with columns and unique. */
#define MSG_FILE_FILTER_INTEG_ERR_OID 1
#define MSG_FILE_FILTER_INTEG_MSG_OID 2

#define RSU_SEC_STORAGE_INTEG_ERR_OID 3
#define RSU_SEC_STORAGE_INTEG_MSG_OID 4

#define RSU_AUTH_ERR_OID 5
#define RSU_AUTH_MSG_OID 6

#define RSU_SIG_VERIFY_ERR_OID 7
#define RSU_SIG_VERIFY_MSG_OID 8

#define RSU_ACCESS_ERR_OID 9
#define RSU_ACCESS_MSG_OID 10

#define RSU_TIME_SOURCE_LOST_OID 11
#define RSU_TIME_SOURCE_LOST_MSG_OID 12

#define RSU_TIME_SOURCE_MISMATCH_OID 13
#define RSU_TIME_SOURCE_MISMATCH_MSG_OID 14

#define RSU_GNSS_ANOMOLY_OID 15
#define RSU_GNSS_ANOMOLY_MSG_OID 16

#define RSU_GNSS_DEVIATION_ERR_OID 17
#define RSU_GNSS_DEVIATION_MSG_OID 18

#define RSU_GNSS_NMEA_NOTIFY_OID 19
#define RSU_GNSS_NMEA_NOTIFY_INTERVAL_OID 20

#define RSU_CERT_ERR_OID 21
#define RSU_CERT_MSG_OID 22

#define RSU_SERVICE_DENIAL_ERR_OID 23
#define RSU_SERVICE_DENIAL_MSG_OID 24

#define RSU_WATCHDOG_ERR_OID 25
#define RSU_WATCHDOG_MSG_OID 26

#define RSU_ENVIORNMENT_ERR_OID 27
#define RSU_ENVIORNMENT_MSG_OID 28

#define RSU_ALERT_LEVEL_OID 29

#define RSU_NOTIFY_REPEAT_INTERVAL_OID 30

#define RSU_NOTIFY_MAX_RESTRIES_OID 31


#define MSG_FILE_FILTER_INTEG_ERR_NAME "messageFileIntegrityError"
#define MSG_FILE_FILTER_INTEG_MSG_NAME "rsuMsgFileIntegrityMsg"

#define RSU_SEC_STORAGE_INTEG_ERR_NAME "rsuSecStorageIntegrityError"
#define RSU_SEC_STORAGE_INTEG_MSG_NAME "rsuSecStorageIntegrityMsg"

#define RSU_AUTH_ERR_NAME "rsuAuthError"
#define RSU_AUTH_MSG_NAME "rsuAuthMsg"

#define RSU_SIG_VERIFY_ERR_NAME "rsuSignatureVerifyError"
#define RSU_SIG_VERIFY_MSG_NAME "rsuSignatureVerifyMsg"

#define RSU_ACCESS_ERR_NAME "rsuAccessError"
#define RSU_ACCESS_MSG_NAME "rsuAccessMsg"

#define RSU_TIME_SOURCE_LOST_NAME "rsuTimeSourceLost"
#define RSU_TIME_SOURCE_LOST_MSG_NAME "rsuTimeSourceLostMsg"

#define RSU_TIME_SOURCE_MISMATCH_NAME "rsuTimeSourceMismatch"
#define RSU_TIME_SOURCE_MISMATCH_MSG_NAME "rsuTimeSourceMismatchMsg"

#define RSU_GNSS_ANOMOLY_NAME "rsuGnssAnomaly"
#define RSU_GNSS_ANOMOLY_MSG_NAME "rsuGnssAnomalyMsg"

#define RSU_GNSS_DEVIATION_ERR_NAME "rsuGnssDeviationError"
#define RSU_GNSS_DEVIATION_MSG_NAME "rsuGnssDeviationMsg"

#define RSU_GNSS_NMEA_NOTIFY_NAME "rsuGnssNmeaNotify"
#define RSU_GNSS_NMEA_NOTIFY_INTERVAL_NAME "rsuGnssNmeaNotifyInterval"

#define RSU_CERT_ERR_NAME "rsuCertificateError"
#define RSU_CERT_MSG_NAME "rsuCertificateMsg"

#define RSU_SERVICE_DENIAL_ERR_NAME "rsuServiceDenialError"
#define RSU_SERVICE_DENIAL_MSG_NAME "rsuServiceDenialMsg"

#define RSU_WATCHDOG_ERR_NAME "rsuWatchdogError"
#define RSU_WATCHDOG_MSG_NAME "rsuWatchdogMsg"

#define RSU_ENVIORNMENT_ERR_NAME "rsuEnvironError"
#define RSU_ENVIORNMENT_MSG_NAME "rsuEnvironMsg"

#define RSU_ALERT_LEVEL_NAME "rsuAlertLevel"

#define RSU_NOTIFY_REPEAT_INTERVAL_NAME "rsuNotificationRepeatInterval"

#define RSU_NOTIFY_MAX_RESTRIES_NAME "rsuNotificationMaxRetries"

/*****************************************************************************
 * 5.18.1 - 5.18.4: Asynchronous Message: {rsu 17}: RO from customer POV.
 *
 *   rsuAsync_t Async;
 *   int32_t    rsuNotificationRepeatInterval;
 *   int32_t    rsuNotificationMaxRetries;
 ****************************************************************************/

/* Grouped logically not in OID numerical order. */
const oid messageFileIntegrityError_oid[] = {1, 3, 6, 1, 4, 1, 1206, 4, 2, 18, 17 , 1, 1};
const oid rsuMsgFileIntegrityMsg_oid[]    = {1, 3, 6, 1, 4, 1, 1206, 4, 2, 18, 17 , 2, 1};
     
const oid rsuSecStorageIntegrityError_oid[] = {1, 3, 6, 1, 4, 1, 1206, 4, 2, 18, 17 , 1, 2};
const oid rsuSecStorageIntegrityMsg_oid[]   = {1, 3, 6, 1, 4, 1, 1206, 4, 2, 18, 17 , 2, 2};

const oid rsuAuthError_oid[] = {1, 3, 6, 1, 4, 1, 1206, 4, 2, 18, 17 , 1, 3};
const oid rsuAuthMsg_oid[]   = {1, 3, 6, 1, 4, 1, 1206, 4, 2, 18, 17 , 2, 3};

const oid rsuSignatureVerifyError_oid[] = {1, 3, 6, 1, 4, 1, 1206, 4, 2, 18, 17 , 1, 4};
const oid rsuSignatureVerifyMsg_oid[]   = {1, 3, 6, 1, 4, 1, 1206, 4, 2, 18, 17 , 2, 4};

const oid rsuAccessError_oid[] = {1, 3, 6, 1, 4, 1, 1206, 4, 2, 18, 17 , 1, 5};
const oid rsuAccessMsg_oid[]   = {1, 3, 6, 1, 4, 1, 1206, 4, 2, 18, 17 , 2, 5};

const oid rsuTimeSourceLost_oid[]    = {1, 3, 6, 1, 4, 1, 1206, 4, 2, 18, 17 , 1, 6};
const oid rsuTimeSourceLostMsg_oid[] = {1, 3, 6, 1, 4, 1, 1206, 4, 2, 18, 17 , 2, 6};

const oid rsuTimeSourceMismatch_oid[]    = {1, 3, 6, 1, 4, 1, 1206, 4, 2, 18, 17 , 1, 7};
const oid rsuTimeSourceMismatchMsg_oid[] = {1, 3, 6, 1, 4, 1, 1206, 4, 2, 18, 17 , 2, 7};

const oid rsuGnssAnomaly_oid[]    = {1, 3, 6, 1, 4, 1, 1206, 4, 2, 18, 17 , 1, 8};
const oid rsuGnssAnomalyMsg_oid[] = {1, 3, 6, 1, 4, 1, 1206, 4, 2, 18, 17 , 2, 8};

const oid rsuGnssDeviationError_oid[] = {1, 3, 6, 1, 4, 1, 1206, 4, 2, 18, 17 , 1, 9};
const oid rsuGnssDeviationMsg_oid[]   = {1, 3, 6, 1, 4, 1, 1206, 4, 2, 18, 17 , 2, 9};

const oid rsuGnssNmeaNotify_oid[]         = {1, 3, 6, 1, 4, 1, 1206, 4, 2, 18, 17 , 1, 10};
const oid rsuGnssNmeaNotifyInterval_oid[] = {1, 3, 6, 1, 4, 1, 1206, 4, 2, 18, 17 , 2, 10};

const oid rsuCertificateError_oid[] = {1, 3, 6, 1, 4, 1, 1206, 4, 2, 18, 17 , 1, 11};
const oid rsuCertificateMsg_oid[]   = {1, 3, 6, 1, 4, 1, 1206, 4, 2, 18, 17 , 2, 12};

const oid rsuServiceDenialError_oid[] = {1, 3, 6, 1, 4, 1, 1206, 4, 2, 18, 17 , 1, 12};
const oid rsuServiceDenialMsg_oid[]   = {1, 3, 6, 1, 4, 1, 1206, 4, 2, 18, 17 , 2, 13};

const oid rsuWatchdogError_oid[] = {1, 3, 6, 1, 4, 1, 1206, 4, 2, 18, 17 , 1, 13};
const oid rsuWatchdogMsg_oid[]   = {1, 3, 6, 1, 4, 1, 1206, 4, 2, 18, 17 , 2, 14};

const oid rsuEnvironError_oid[] = {1, 3, 6, 1, 4, 1, 1206, 4, 2, 18, 17 , 1, 14};
const oid rsuEnvironMsg_oid[]   = {1, 3, 6, 1, 4, 1, 1206, 4, 2, 18, 17 , 2, 15};

/* 5.18.2.11: Odd one out: Alert level with no error message. */
const oid rsuAlertLevel_oid[] = {1, 3, 6, 1, 4, 1, 1206, 4, 2, 18, 17 , 2, 11};
/* 5.18.3 */
const oid rsuNotificationRepeatInterval_oid[] = {1, 3, 6, 1, 4, 1, 1206, 4, 2, 18, 17 , 3 };
/* 5.18.4 */
const oid rsuNotificationMaxRetries_oid[] = {1, 3, 6, 1, 4, 1, 1206, 4, 2, 18, 17 , 4 };

extern int32_t  mainloop; /* For thread handling. */
static uint32_t rsuAsync_error_states = 0x0;

/* 
 * Function Bodies.
 */
#define RSEMIB_SHM_NOT_VALID -30
static void set_error_states(int32_t error)
{
  /* -1 to -32 */
  if((error < 0) && (error > MIB_ERROR_BASE_2)){
      rsuAsync_error_states |= (uint32_t)(0x1) << (abs(error) - 1);
  }
}
static void clear_error_states(int32_t error)
{
  /* -1 to -32 */
  if((error < 0) && (error > MIB_ERROR_BASE_2)){
      rsuAsync_error_states &= ~((uint32_t)(0x1) << (abs(error) - 1));
  }
}
uint32_t get_rsuAsync_error_states(void)
{
  return rsuAsync_error_states;
}
/* Unified way to report back to SNMP requests. For handlers only. */
static int32_t handle_rsemib_errors(int32_t input)
{
  int32_t ret = SNMP_ERR_NOERROR;

  /* The default is BADVALUE. Only add deviations from this. */
  switch(input){
      case RSEMIB_OK:
          ret = SNMP_ERR_NOERROR;
          break;
      case RSEMIB_LOCK_FAIL:
          ret = SNMP_ERR_NOACCESS;   
          break;
      case RSEMIB_ROW_EMPTY:
          ret = SNMP_ERR_NOERROR; /* Row has not been created so nothing to do. */
          break;
      case RSEMIB_BAD_MODE:
          ret = SNMP_ERR_INCONSISTENTVALUE;
          break;
      case RSEMIB_SYSCALL_FAIL:
          ret = SNMP_ERR_GENERR;
          break;
      default:
          ret = SNMP_ERR_BADVALUE;
          break;
  }
  return ret;
}
/*
 * User friendly output. Only print out handler errors for their transactions.
 * The rest go to rsutable.c to be dumped in error states for us developers to digest.
 */
static void print_request_result(uint32_t row_index, uint32_t column, netsnmp_request_info * requests, int32_t mode, int32_t ret)
{
  char_t mode_name[16]; /* Size accordingly. */
  char_t column_name[64];
  char_t passOrFail[16];
  uint8_t dataOctet[65]; /* Limit to max token size(64 chars + null) for most values like ipv4 and ipv6 */
  uint8_t * currentOctet = NULL;
  int32_t dataI32 = 0;
  uint64_t myHigh = 0;
  uint64_t myLow = 0;
  float32_t dataF32 = 0.0f;
  float64_t dataF64 = 0.0;
  uint8_t bitString8 = 0x0;
  uint32_t i = 0;
  char_t alert_msg[RSU_ALERT_MSG_LENGTH_MAX];

  memset(mode_name,0,sizeof(mode_name));
  memset(column_name,0,sizeof(column_name));
  memset(passOrFail,0,sizeof(passOrFail));
  memset(alert_msg,0,sizeof(alert_msg));

  if((MODE_SET_COMMIT != mode) && (RSEMIB_OK == ret)) {
      return; /* Dont bother with AOK GETS. Just noise. */
  }
  switch (mode){
      case MODE_GET:
          strcpy(mode_name,"GET");
          break;
      case MODE_SET_RESERVE1:
          strcpy(mode_name,"SET(R1)");
          break;
      case MODE_SET_RESERVE2:
          strcpy(mode_name,"SET(R2)");
          break;
      case MODE_SET_ACTION:
          strcpy(mode_name,"SET(ACTION)");
          break;
      case MODE_SET_COMMIT:
          strcpy(mode_name,"SET");
          break;
      case MODE_SET_FREE:
          strcpy(mode_name,"SET(FREE)");
          break;
      case MODE_SET_UNDO:
          strcpy(mode_name,"SET(UNDO)");
          break;
      default:
          strcpy(mode_name,"UNKNOWN");
          break;
  }
  switch (column){
      case MSG_FILE_FILTER_INTEG_ERR_OID:
          strcpy(column_name,MSG_FILE_FILTER_INTEG_ERR_NAME);
          break;
      case MSG_FILE_FILTER_INTEG_MSG_OID:
          strcpy(column_name,MSG_FILE_FILTER_INTEG_MSG_NAME);
          break;
      case RSU_SEC_STORAGE_INTEG_ERR_OID:
          strcpy(column_name,RSU_SEC_STORAGE_INTEG_ERR_NAME);
          break;
      case RSU_SEC_STORAGE_INTEG_MSG_OID:
          strcpy(column_name,RSU_SEC_STORAGE_INTEG_MSG_NAME);
          break;
      case RSU_AUTH_ERR_OID:
          strcpy(column_name,RSU_AUTH_ERR_NAME);
          break;
      case RSU_AUTH_MSG_OID:
          strcpy(column_name,RSU_AUTH_MSG_NAME);
          break;
      case RSU_SIG_VERIFY_ERR_OID:
          strcpy(column_name,RSU_SIG_VERIFY_ERR_NAME);
          break;
      case RSU_SIG_VERIFY_MSG_OID:
          strcpy(column_name,RSU_SIG_VERIFY_MSG_NAME);
          break;
      case RSU_ACCESS_ERR_OID:
          strcpy(column_name,RSU_ACCESS_ERR_NAME);
          break;
      case RSU_ACCESS_MSG_OID:
          strcpy(column_name,RSU_ACCESS_MSG_NAME);
          break;
      case RSU_TIME_SOURCE_LOST_OID:
          strcpy(column_name,RSU_TIME_SOURCE_LOST_NAME);
          break;
      case RSU_TIME_SOURCE_LOST_MSG_OID:
          strcpy(column_name,RSU_TIME_SOURCE_LOST_MSG_NAME);
          break;
      case RSU_TIME_SOURCE_MISMATCH_OID:
          strcpy(column_name,RSU_TIME_SOURCE_MISMATCH_NAME);
          break;
      case RSU_TIME_SOURCE_MISMATCH_MSG_OID:
          strcpy(column_name,RSU_TIME_SOURCE_MISMATCH_MSG_NAME);
          break;
      case RSU_GNSS_ANOMOLY_OID:
          strcpy(column_name,RSU_GNSS_ANOMOLY_NAME);
          break;
      case RSU_GNSS_ANOMOLY_MSG_OID:
          strcpy(column_name,RSU_GNSS_ANOMOLY_MSG_NAME);
          break;
      case RSU_GNSS_DEVIATION_ERR_OID:
          strcpy(column_name,RSU_GNSS_DEVIATION_ERR_NAME);
          break;
      case RSU_GNSS_DEVIATION_MSG_OID:
          strcpy(column_name,RSU_GNSS_DEVIATION_MSG_NAME);
          break;
      case RSU_GNSS_NMEA_NOTIFY_OID:
          strcpy(column_name,RSU_GNSS_NMEA_NOTIFY_NAME);
          break;
      case RSU_GNSS_NMEA_NOTIFY_INTERVAL_OID:
          strcpy(column_name,RSU_GNSS_NMEA_NOTIFY_INTERVAL_NAME);
          break;
      case RSU_CERT_ERR_OID:
          strcpy(column_name,RSU_CERT_ERR_NAME);
          break;
      case RSU_CERT_MSG_OID:
          strcpy(column_name,RSU_CERT_MSG_NAME);
          break;
      case RSU_SERVICE_DENIAL_ERR_OID:
          strcpy(column_name,RSU_SERVICE_DENIAL_ERR_NAME);
          break;
      case RSU_SERVICE_DENIAL_MSG_OID:
          strcpy(column_name,RSU_SERVICE_DENIAL_MSG_NAME);
          break;
      case RSU_WATCHDOG_ERR_OID:
          strcpy(column_name,RSU_WATCHDOG_ERR_NAME);
          break;
      case RSU_WATCHDOG_MSG_OID:
          strcpy(column_name,RSU_WATCHDOG_MSG_NAME);
          break;
      case RSU_ENVIORNMENT_ERR_OID:
          strcpy(column_name,RSU_ENVIORNMENT_ERR_NAME);
          break;
      case RSU_ENVIORNMENT_MSG_OID:
          strcpy(column_name,RSU_ENVIORNMENT_MSG_NAME);
          break;
      case RSU_ALERT_LEVEL_OID:
          strcpy(column_name,RSU_ALERT_LEVEL_NAME);
          break;
      case RSU_NOTIFY_REPEAT_INTERVAL_OID:
          strcpy(column_name,RSU_NOTIFY_REPEAT_INTERVAL_NAME);
          break;
      case RSU_NOTIFY_MAX_RESTRIES_OID:
          strcpy(column_name,RSU_NOTIFY_MAX_RESTRIES_NAME);
          break;
      default:
          strcpy(column_name,"UNKOWN");
          break;
  }
  if(RSEMIB_OK == ret) {
      strcpy(passOrFail,"succeeded");
  } else {
      strcpy(passOrFail,"failed");
  }
  /* Extract data type and data from request for user display. */
  switch(requests->requestvb->type) {
      case ASN_INTEGER:
          dataI32 = *requests->requestvb->val.integer;
          snprintf(alert_msg,sizeof(alert_msg),"ASN_INTEGER=%d len=%lu: %s.%u %s to %d %s: ret=%d.\n",
              dataI32,requests->requestvb->val_len,column_name,row_index,mode_name,dataI32,passOrFail,ret);
          break;
      case ASN_BIT_STR: /* Could be more than one octet but for now. */
          bitString8 = *requests->requestvb->val.bitstring;
          snprintf(alert_msg,sizeof(alert_msg),"ASN_BIT_STR=0x%x len=%lu: %s.%u %s to 0x%X %s: ret=%d.\n",
              bitString8, requests->requestvb->val_len,column_name,row_index,mode_name,bitString8,passOrFail,ret);
          break;
      case ASN_OCTET_STR: /* Could be ascii or binary. Need to detect. For these OIDs assume binary for display. */
          memset(dataOctet,0x0, sizeof(dataOctet));
          currentOctet = requests->requestvb->val.string;
          for(i=0; (i < requests->requestvb->val_len) && (i < 65); i++) { 
              dataOctet[i] = *currentOctet++;
          }
          snprintf(alert_msg,sizeof(alert_msg),"ASN_OCTET_STR=[%02X %02X %02X %02X %02X %02X %02X %02X] len=%lu: %s.%u %s to value above %s: ret=%d.\n",
              dataOctet[0],dataOctet[1],dataOctet[2],dataOctet[3],dataOctet[4],dataOctet[5],dataOctet[6],dataOctet[7]
              ,requests->requestvb->val_len,column_name,row_index,mode_name,passOrFail,ret);
          break;
      case ASN_OPAQUE_COUNTER64: /* high and low? */
          myHigh = requests->requestvb->val.counter64->high;
          myLow = requests->requestvb->val.counter64->low;
          snprintf(alert_msg,sizeof(alert_msg),"ASN_OPAQUE_COUNTER64=(%ld,%ld) len=%ld: %s.%u %s to [%ld,%ld] %s: ret=%d.\n",
              myHigh,myLow,requests->requestvb->val_len,column_name,row_index,mode_name,myHigh,myLow,passOrFail,ret);
          break;
      case ASN_OPAQUE_FLOAT:
          dataF32 = *requests->requestvb->val.floatVal;
          snprintf(alert_msg,sizeof(alert_msg),"ASN_OPAQUE_FLOAT=%f len=%lu: %s.%u %s to [%f] %s: ret=%d.\n",
              dataF32,requests->requestvb->val_len,column_name,row_index,mode_name,dataF32,passOrFail,ret);
          break;
      case ASN_OPAQUE_DOUBLE:
          dataF64 = *requests->requestvb->val.doubleVal;
          snprintf(alert_msg,sizeof(alert_msg),"ASN_OPAQUE_DOUBLE=%lf len=%lu: %s.%u %s to [%lf] %s: ret=%d.\n",
              dataF64,requests->requestvb->val_len,column_name,row_index,mode_name,dataF64,passOrFail,ret);
          break;
      default: /* Dont know the data type so don't print it. */
          snprintf(alert_msg,sizeof(alert_msg),"ASN_UNKNOWN=0x%x len=%lu: %s for %s.%u %s: ret=%d.\n",
              0xBADBEEF,requests->requestvb->val_len,mode_name,column_name,row_index,passOrFail,ret);          
          break;
  }
  I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,alert_msg);
}

/*
 * OID Handlers
 */

/* 5.18.1.1 File Integrity Check Error Message: AMH SAR & IMF.*/
int32_t handle_messageFileIntegrityError(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo,
                       netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret = RSEMIB_OK;

  handler = handler; /* Silence of the Warnings. */
  reginfo = reginfo;
  DEBUGMSGTL((MY_NAME, "%s: Entry:\n",__FUNCTION__));
  if((NULL == reqinfo) || (NULL == requests)) {
      ret = RSEMIB_BAD_INPUT;
  } else {
      switch (reqinfo->mode){
          case MODE_GET:
              DEBUGMSGTL((MY_NAME, "GET:\n"));
              if(0 <= (ret = get_messageFileIntegrityError())){
                  snmp_set_var_typed_integer(requests->requestvb, ASN_INTEGER, ret);
                  ret = RSEMIB_OK;
              }
              break;
          default:
              ret = RSEMIB_BAD_MODE;
              break;
      }
      /* scalars and like are always index 0 with unqiue column.*/
      print_request_result(0, MSG_FILE_FILTER_INTEG_ERR_OID,requests,reqinfo->mode,ret);
  }
  if(ret < RSEMIB_OK) {
      set_error_states(ret);
  }
  /* Convert from MIB(negative) error to SNMP(positive) request error. */
  ret = handle_rsemib_errors(ret);
  if(SNMP_ERR_NOERROR < ret){ /* Set once and only if error. Only top handler need do this. */
      netsnmp_set_request_error(reqinfo, requests, ret);
  }
  DEBUGMSGTL((MY_NAME, "ret=%d:Exit.\n",ret));
  return ret;
}
int32_t handle_rsuMsgFileIntegrityMsg(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo,
                       netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret = RSEMIB_OK;
  uint8_t data[RSU_ALERT_MSG_LENGTH_MAX];

  handler = handler; /* Silence of the Warnings. */
  reginfo = reginfo;
  DEBUGMSGTL((MY_NAME, "%s: Entry:\n",__FUNCTION__));
  if((NULL == reqinfo) || (NULL == requests)) {
      ret = RSEMIB_BAD_INPUT;
  } else {
      switch (reqinfo->mode){
          case MODE_GET:
              DEBUGMSGTL((MY_NAME, "GET:\n"));
              memset(data,0x0,sizeof(data));
              if(0 <= (ret = get_rsuMsgFileIntegrityMsg(data))){
                  snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR, data, ret);
                  ret = RSEMIB_OK;
              }
              break;
          default:
              ret = RSEMIB_BAD_MODE;
              break;
      }
      /* scalars and like are always index 0 with unqiue column.*/
      print_request_result(0, MSG_FILE_FILTER_INTEG_MSG_OID,requests,reqinfo->mode,ret);
  }
  if(ret < RSEMIB_OK) {
      set_error_states(ret);
  }
  /* Convert from MIB(negative) error to SNMP(positive) request error. */
  ret = handle_rsemib_errors(ret);
  if(SNMP_ERR_NOERROR < ret){ /* Set once and only if error. Only top handler need do this. */
      netsnmp_set_request_error(reqinfo, requests, ret);
  }
  DEBUGMSGTL((MY_NAME, "ret=%d:Exit.\n",ret));
  return ret;
}
/* 5.18.1.2 Storage Integrity Error Message: Errors in file system.*/
int32_t handle_rsuSecStorageIntegrityError(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo,
                       netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret = RSEMIB_OK;

  handler = handler; /* Silence of the Warnings. */
  reginfo = reginfo;
  DEBUGMSGTL((MY_NAME, "%s: Entry:\n",__FUNCTION__));
  if((NULL == reqinfo) || (NULL == requests)) {
      ret = RSEMIB_BAD_INPUT;
  } else {
      switch (reqinfo->mode){
          case MODE_GET:
              DEBUGMSGTL((MY_NAME, "GET:\n"));
              if(0 <= (ret = get_rsuSecStorageIntegrityError())){
                  snmp_set_var_typed_integer(requests->requestvb, ASN_INTEGER, ret);
                  ret = RSEMIB_OK;
              }
              break;
          default:
              ret = RSEMIB_BAD_MODE;
              break;
      }
      /* scalars and like are always index 0 with unqiue column.*/
      print_request_result(0, RSU_SEC_STORAGE_INTEG_ERR_OID,requests,reqinfo->mode,ret);
  }
  if(ret < RSEMIB_OK) {
      set_error_states(ret);
  }
  /* Convert from MIB(negative) error to SNMP(positive) request error. */
  ret = handle_rsemib_errors(ret);
  if(SNMP_ERR_NOERROR < ret){ /* Set once and only if error. Only top handler need do this. */
      netsnmp_set_request_error(reqinfo, requests, ret);
  }
  DEBUGMSGTL((MY_NAME, "ret=%d:Exit.\n",ret));
  return ret;
}
int32_t handle_rsuSecStorageIntegrityMsg(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo,
                       netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret = RSEMIB_OK;
  uint8_t data[RSU_ALERT_MSG_LENGTH_MAX];

  handler = handler; /* Silence of the Warnings. */
  reginfo = reginfo;
  DEBUGMSGTL((MY_NAME, "%s: Entry:\n",__FUNCTION__));
  if((NULL == reqinfo) || (NULL == requests)) {
      ret = RSEMIB_BAD_INPUT;
  } else {
      switch (reqinfo->mode){
          case MODE_GET:
              DEBUGMSGTL((MY_NAME, "GET:\n"));
              memset(data,0x0,sizeof(data));
              if(0 <= (ret = get_rsuSecStorageIntegrityMsg(data))){
                  snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR, data, ret);
                  ret = RSEMIB_OK;
              }
              break;
          default:
              ret = RSEMIB_BAD_MODE;
              break;
      }
      /* scalars and like are always index 0 with unqiue column.*/
      print_request_result(0, RSU_SEC_STORAGE_INTEG_MSG_OID,requests,reqinfo->mode,ret);
  }
  if(ret < RSEMIB_OK) {
      set_error_states(ret);
  }
  /* Convert from MIB(negative) error to SNMP(positive) request error. */
  ret = handle_rsemib_errors(ret);
  if(SNMP_ERR_NOERROR < ret){ /* Set once and only if error. Only top handler need do this. */
      netsnmp_set_request_error(reqinfo, requests, ret);
  }
  DEBUGMSGTL((MY_NAME, "ret=%d:Exit.\n",ret));
  return ret;
}
/* 5.18.1.3 Authorization Error Message: invalid security creds: Is failed login attempt one? */
int32_t handle_rsuAuthError(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo,
                       netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret = RSEMIB_OK;

  handler = handler; /* Silence of the Warnings. */
  reginfo = reginfo;
  DEBUGMSGTL((MY_NAME, "%s: Entry:\n",__FUNCTION__));
  if((NULL == reqinfo) || (NULL == requests)) {
      ret = RSEMIB_BAD_INPUT;
  } else {
      switch (reqinfo->mode){
          case MODE_GET:
              DEBUGMSGTL((MY_NAME, "GET:\n"));
              if(0 <= (ret = get_rsuAuthError())){
                  snmp_set_var_typed_integer(requests->requestvb, ASN_INTEGER, ret);
                  ret = RSEMIB_OK;
              }
              break;
          default:
              ret = RSEMIB_BAD_MODE;
              break;
      }
      /* scalars and like are always index 0 with unqiue column.*/
      print_request_result(0, RSU_AUTH_ERR_OID,requests,reqinfo->mode,ret);
  }
  if(ret < RSEMIB_OK) {
      set_error_states(ret);
  }
  /* Convert from MIB(negative) error to SNMP(positive) request error. */
  ret = handle_rsemib_errors(ret);
  if(SNMP_ERR_NOERROR < ret){ /* Set once and only if error. Only top handler need do this. */
      netsnmp_set_request_error(reqinfo, requests, ret);
  }
  DEBUGMSGTL((MY_NAME, "ret=%d:Exit.\n",ret));
  return ret;
}
int32_t handle_rsuAuthMsg(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo,
                       netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret = RSEMIB_OK;
  uint8_t data[RSU_ALERT_MSG_LENGTH_MAX];

  handler = handler; /* Silence of the Warnings. */
  reginfo = reginfo;
  DEBUGMSGTL((MY_NAME, "%s: Entry:\n",__FUNCTION__));
  if((NULL == reqinfo) || (NULL == requests)) {
      ret = RSEMIB_BAD_INPUT;
  } else {
      switch (reqinfo->mode){
          case MODE_GET:
              DEBUGMSGTL((MY_NAME, "GET:\n"));
              memset(data,0x0,sizeof(data));
              if(0 <= (ret = get_rsuAuthMsg(data))){
                  snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR, data, ret);
                  ret = RSEMIB_OK;
              }
              break;
          default:
              ret = RSEMIB_BAD_MODE;
              break;
      }
      /* scalars and like are always index 0 with unqiue column.*/
      print_request_result(0, RSU_AUTH_MSG_OID,requests,reqinfo->mode,ret);
  }
  if(ret < RSEMIB_OK) {
      set_error_states(ret);
  }
  /* Convert from MIB(negative) error to SNMP(positive) request error. */
  ret = handle_rsemib_errors(ret);
  if(SNMP_ERR_NOERROR < ret){ /* Set once and only if error. Only top handler need do this. */
      netsnmp_set_request_error(reqinfo, requests, ret);
  }
  DEBUGMSGTL((MY_NAME, "ret=%d:Exit.\n",ret));
  return ret;
}
/* 5.18.1.4 Signature Verification Error Message: Any failed signature on WSM's */
int32_t handle_rsuSignatureVerifyError(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo,
                       netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret = RSEMIB_OK;

  handler = handler; /* Silence of the Warnings. */
  reginfo = reginfo;
  DEBUGMSGTL((MY_NAME, "%s: Entry:\n",__FUNCTION__));
  if((NULL == reqinfo) || (NULL == requests)) {
      ret = RSEMIB_BAD_INPUT;
  } else {
      switch (reqinfo->mode){
          case MODE_GET:
              DEBUGMSGTL((MY_NAME, "GET:\n"));
              if(0 <= (ret = get_rsuSignatureVerifyError())){
                  snmp_set_var_typed_integer(requests->requestvb, ASN_INTEGER, ret);
                  ret = RSEMIB_OK;
              }
              break;
          default:
              ret = RSEMIB_BAD_MODE;
              break;
      }
      /* scalars and like are always index 0 with unqiue column.*/
      print_request_result(0, RSU_SIG_VERIFY_ERR_OID,requests,reqinfo->mode,ret);
  }
  if(ret < RSEMIB_OK) {
      set_error_states(ret);
  }
  /* Convert from MIB(negative) error to SNMP(positive) request error. */
  ret = handle_rsemib_errors(ret);
  if(SNMP_ERR_NOERROR < ret){ /* Set once and only if error. Only top handler need do this. */
      netsnmp_set_request_error(reqinfo, requests, ret);
  }
  DEBUGMSGTL((MY_NAME, "ret=%d:Exit.\n",ret));
  return ret;
}
int32_t handle_rsuSignatureVerifyMsg(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo,
                       netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret = RSEMIB_OK;
  uint8_t data[RSU_ALERT_MSG_LENGTH_MAX];

  handler = handler; /* Silence of the Warnings. */
  reginfo = reginfo;
  DEBUGMSGTL((MY_NAME, "%s: Entry:\n",__FUNCTION__));
  if((NULL == reqinfo) || (NULL == requests)) {
      ret = RSEMIB_BAD_INPUT;
  } else {
      switch (reqinfo->mode){
          case MODE_GET:
              DEBUGMSGTL((MY_NAME, "GET:\n"));
              memset(data,0x0,sizeof(data));
              if(0 <= (ret = get_rsuSignatureVerifyMsg(data))){
                  snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR, data, ret);
                  ret = RSEMIB_OK;
              }
              break;
          default:
              ret = RSEMIB_BAD_MODE;
              break;
      }
      /* scalars and like are always index 0 with unqiue column.*/
      print_request_result(0, RSU_SIG_VERIFY_MSG_OID,requests,reqinfo->mode,ret);
  }
  if(ret < RSEMIB_OK) {
      set_error_states(ret);
  }
  /* Convert from MIB(negative) error to SNMP(positive) request error. */
  ret = handle_rsemib_errors(ret);
  if(SNMP_ERR_NOERROR < ret){ /* Set once and only if error. Only top handler need do this. */
      netsnmp_set_request_error(reqinfo, requests, ret);
  }
  DEBUGMSGTL((MY_NAME, "ret=%d:Exit.\n",ret));
  return ret;
}
/* 5.18.1.5 Access Error Message: error or rejection due to a violation of the Access Control List.*/
int32_t handle_rsuAccessError(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo,
                       netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret = RSEMIB_OK;

  handler = handler; /* Silence of the Warnings. */
  reginfo = reginfo;
  DEBUGMSGTL((MY_NAME, "%s: Entry:\n",__FUNCTION__));
  if((NULL == reqinfo) || (NULL == requests)) {
      ret = RSEMIB_BAD_INPUT;
  } else {
      switch (reqinfo->mode){
          case MODE_GET:
              DEBUGMSGTL((MY_NAME, "GET:\n"));
              if(0 <= (ret = get_rsuAccessError())){
                  snmp_set_var_typed_integer(requests->requestvb, ASN_INTEGER, ret);
                  ret = RSEMIB_OK;
              }
              break;
          default:
              ret = RSEMIB_BAD_MODE;
              break;
      }
      /* scalars and like are always index 0 with unqiue column.*/
      print_request_result(0, RSU_ACCESS_ERR_OID,requests,reqinfo->mode,ret);
  }
  if(ret < RSEMIB_OK) {
      set_error_states(ret);
  }
  /* Convert from MIB(negative) error to SNMP(positive) request error. */
  ret = handle_rsemib_errors(ret);
  if(SNMP_ERR_NOERROR < ret){ /* Set once and only if error. Only top handler need do this. */
      netsnmp_set_request_error(reqinfo, requests, ret);
  }
  DEBUGMSGTL((MY_NAME, "ret=%d:Exit.\n",ret));
  return ret;
}
int32_t handle_rsuAccessMsg(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo,
                       netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret = RSEMIB_OK;
  uint8_t data[RSU_ALERT_MSG_LENGTH_MAX];

  handler = handler; /* Silence of the Warnings. */
  reginfo = reginfo;
  DEBUGMSGTL((MY_NAME, "%s: Entry:\n",__FUNCTION__));
  if((NULL == reqinfo) || (NULL == requests)) {
      ret = RSEMIB_BAD_INPUT;
  } else {
      switch (reqinfo->mode){
          case MODE_GET:
              DEBUGMSGTL((MY_NAME, "GET:\n"));
              memset(data,0x0,sizeof(data));
              if(0 <= (ret = get_rsuAccessMsg(data))){
                  snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR, data, ret);
                  ret = RSEMIB_OK;
              }
              break;
          default:
              ret = RSEMIB_BAD_MODE;
              break;
      }
      /* scalars and like are always index 0 with unqiue column.*/
      print_request_result(0, RSU_ACCESS_MSG_OID,requests,reqinfo->mode,ret);
  }
  if(ret < RSEMIB_OK) {
      set_error_states(ret);
  }
  /* Convert from MIB(negative) error to SNMP(positive) request error. */
  ret = handle_rsemib_errors(ret);
  if(SNMP_ERR_NOERROR < ret){ /* Set once and only if error. Only top handler need do this. */
      netsnmp_set_request_error(reqinfo, requests, ret);
  }
  DEBUGMSGTL((MY_NAME, "ret=%d:Exit.\n",ret));
  return ret;
}
/* 5.18.1.6 Time Source Lost Message: Lost time source: ie lost GNSS fix.*/
int32_t handle_rsuTimeSourceLost(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo,
                       netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret = RSEMIB_OK;

  handler = handler; /* Silence of the Warnings. */
  reginfo = reginfo;
  DEBUGMSGTL((MY_NAME, "%s: Entry:\n",__FUNCTION__));
  if((NULL == reqinfo) || (NULL == requests)) {
      ret = RSEMIB_BAD_INPUT;
  } else {
      switch (reqinfo->mode){
          case MODE_GET:
              DEBUGMSGTL((MY_NAME, "GET:\n"));
              if(0 <= (ret = get_rsuTimeSourceLost())){
                  snmp_set_var_typed_integer(requests->requestvb, ASN_INTEGER, ret);
                  ret = RSEMIB_OK;
              }
              break;
          default:
              ret = RSEMIB_BAD_MODE;
              break;
      }
      /* scalars and like are always index 0 with unqiue column.*/
      print_request_result(0, RSU_TIME_SOURCE_LOST_OID,requests,reqinfo->mode,ret);
  }
  if(ret < RSEMIB_OK) {
      set_error_states(ret);
  }
  /* Convert from MIB(negative) error to SNMP(positive) request error. */
  ret = handle_rsemib_errors(ret);
  if(SNMP_ERR_NOERROR < ret){ /* Set once and only if error. Only top handler need do this. */
      netsnmp_set_request_error(reqinfo, requests, ret);
  }
  DEBUGMSGTL((MY_NAME, "ret=%d:Exit.\n",ret));
  return ret;
}
int32_t handle_rsuTimeSourceLostMsg(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo,
                       netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret = RSEMIB_OK;
  uint8_t data[RSU_ALERT_MSG_LENGTH_MAX];

  handler = handler; /* Silence of the Warnings. */
  reginfo = reginfo;
  DEBUGMSGTL((MY_NAME, "%s: Entry:\n",__FUNCTION__));
  if((NULL == reqinfo) || (NULL == requests)) {
      ret = RSEMIB_BAD_INPUT;
  } else {
      switch (reqinfo->mode){
          case MODE_GET:
              DEBUGMSGTL((MY_NAME, "GET:\n"));
              memset(data,0x0,sizeof(data));
              if(0 <= (ret = get_rsuTimeSourceLostMsg(data))){
                  snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR, data, ret);
                  ret = RSEMIB_OK;
              }
              break;
          default:
              ret = RSEMIB_BAD_MODE;
              break;
      }
      /* scalars and like are always index 0 with unqiue column.*/
      print_request_result(0, RSU_TIME_SOURCE_LOST_MSG_OID,requests,reqinfo->mode,ret);
  }
  if(ret < RSEMIB_OK) {
      set_error_states(ret);
  }
  /* Convert from MIB(negative) error to SNMP(positive) request error. */
  ret = handle_rsemib_errors(ret);
  if(SNMP_ERR_NOERROR < ret){ /* Set once and only if error. Only top handler need do this. */
      netsnmp_set_request_error(reqinfo, requests, ret);
  }
  DEBUGMSGTL((MY_NAME, "ret=%d:Exit.\n",ret));
  return ret;
}
/* 5.18.1.7 Time Source Mismatch Message: deviation between two time sources exceeds vendor-defined threshold. Who is vendor? */
int32_t handle_rsuTimeSourceMismatch(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo,
                       netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret = RSEMIB_OK;

  handler = handler; /* Silence of the Warnings. */
  reginfo = reginfo;
  DEBUGMSGTL((MY_NAME, "%s: Entry:\n",__FUNCTION__));
  if((NULL == reqinfo) || (NULL == requests)) {
      ret = RSEMIB_BAD_INPUT;
  } else {
      switch (reqinfo->mode){
          case MODE_GET:
              DEBUGMSGTL((MY_NAME, "GET:\n"));
              if(0 <= (ret = get_rsuTimeSourceMismatch())){
                  snmp_set_var_typed_integer(requests->requestvb, ASN_INTEGER, ret);
                  ret = RSEMIB_OK;
              }
              break;
          default:
              ret = RSEMIB_BAD_MODE;
              break;
      }
      /* scalars and like are always index 0 with unqiue column.*/
      print_request_result(0, RSU_TIME_SOURCE_MISMATCH_OID,requests,reqinfo->mode,ret);
  }
  if(ret < RSEMIB_OK) {
      set_error_states(ret);
  }
  /* Convert from MIB(negative) error to SNMP(positive) request error. */
  ret = handle_rsemib_errors(ret);
  if(SNMP_ERR_NOERROR < ret){ /* Set once and only if error. Only top handler need do this. */
      netsnmp_set_request_error(reqinfo, requests, ret);
  }
  DEBUGMSGTL((MY_NAME, "ret=%d:Exit.\n",ret));
  return ret;
}
int32_t handle_rsuTimeSourceMismatchMsg(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo,
                       netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret = RSEMIB_OK;
  uint8_t data[RSU_ALERT_MSG_LENGTH_MAX];

  handler = handler; /* Silence of the Warnings. */
  reginfo = reginfo;
  DEBUGMSGTL((MY_NAME, "%s: Entry:\n",__FUNCTION__));
  if((NULL == reqinfo) || (NULL == requests)) {
      ret = RSEMIB_BAD_INPUT;
  } else {
      switch (reqinfo->mode){
          case MODE_GET:
              DEBUGMSGTL((MY_NAME, "GET:\n"));
              memset(data,0x0,sizeof(data));
              if(0 <= (ret = get_rsuTimeSourceMismatchMsg(data))){
                  snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR, data, ret);
                  ret = RSEMIB_OK;
              }
              break;
          default:
              ret = RSEMIB_BAD_MODE;
              break;
      }
      /* scalars and like are always index 0 with unqiue column.*/
      print_request_result(0, RSU_TIME_SOURCE_MISMATCH_MSG_OID,requests,reqinfo->mode,ret);
  }
  if(ret < RSEMIB_OK) {
      set_error_states(ret);
  }
  /* Convert from MIB(negative) error to SNMP(positive) request error. */
  ret = handle_rsemib_errors(ret);
  if(SNMP_ERR_NOERROR < ret){ /* Set once and only if error. Only top handler need do this. */
      netsnmp_set_request_error(reqinfo, requests, ret);
  }
  DEBUGMSGTL((MY_NAME, "ret=%d:Exit.\n",ret));
  return ret;
}
/* 5.18.1.8 GNSS Anomaly Message: report any anomalous GNSS readings: Sky's the limit here. */
int32_t handle_rsuGnssAnomaly(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo,
                       netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret = RSEMIB_OK;

  handler = handler; /* Silence of the Warnings. */
  reginfo = reginfo;
  DEBUGMSGTL((MY_NAME, "%s: Entry:\n",__FUNCTION__));
  if((NULL == reqinfo) || (NULL == requests)) {
      ret = RSEMIB_BAD_INPUT;
  } else {
      switch (reqinfo->mode){
          case MODE_GET:
              DEBUGMSGTL((MY_NAME, "GET:\n"));
              if(0 <= (ret = get_rsuGnssAnomaly())){
                  snmp_set_var_typed_integer(requests->requestvb, ASN_INTEGER, ret);
                  ret = RSEMIB_OK;
              }
              break;
          default:
              ret = RSEMIB_BAD_MODE;
              break;
      }
      /* scalars and like are always index 0 with unqiue column.*/
      print_request_result(0, RSU_GNSS_ANOMOLY_OID,requests,reqinfo->mode,ret);
  }
  if(ret < RSEMIB_OK) {
      set_error_states(ret);
  }
  /* Convert from MIB(negative) error to SNMP(positive) request error. */
  ret = handle_rsemib_errors(ret);
  if(SNMP_ERR_NOERROR < ret){ /* Set once and only if error. Only top handler need do this. */
      netsnmp_set_request_error(reqinfo, requests, ret);
  }
  DEBUGMSGTL((MY_NAME, "ret=%d:Exit.\n",ret));
  return ret;
}
int32_t handle_rsuGnssAnomalyMsg(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo,
                       netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret = RSEMIB_OK;
  uint8_t data[RSU_ALERT_MSG_LENGTH_MAX];

  handler = handler; /* Silence of the Warnings. */
  reginfo = reginfo;
  DEBUGMSGTL((MY_NAME, "%s: Entry:\n",__FUNCTION__));
  if((NULL == reqinfo) || (NULL == requests)) {
      ret = RSEMIB_BAD_INPUT;
  } else {
      switch (reqinfo->mode){
          case MODE_GET:
              DEBUGMSGTL((MY_NAME, "GET:\n"));
              memset(data,0x0,sizeof(data));
              if(0 <= (ret = get_rsuGnssAnomalyMsg(data))){
                  snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR, data, ret);
                  ret = RSEMIB_OK;
              } 
              break;
          default:
              ret = RSEMIB_BAD_MODE;
              break;
      }
      /* scalars and like are always index 0 with unqiue column.*/
      print_request_result(0, RSU_GNSS_ANOMOLY_MSG_OID,requests,reqinfo->mode,ret);
  }
  if(ret < RSEMIB_OK) {
      set_error_states(ret);
  }
  /* Convert from MIB(negative) error to SNMP(positive) request error. */
  ret = handle_rsemib_errors(ret);
  if(SNMP_ERR_NOERROR < ret){ /* Set once and only if error. Only top handler need do this. */
      netsnmp_set_request_error(reqinfo, requests, ret);
  }
  DEBUGMSGTL((MY_NAME, "ret=%d:Exit.\n",ret));
  return ret;
}
/* 5.18.1.9 GNSS Deviation Error Message: GNSS position deviation greater than allowed.*/
int32_t handle_rsuGnssDeviationError(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo,
                       netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret = RSEMIB_OK;

  handler = handler; /* Silence of the Warnings. */
  reginfo = reginfo;
  DEBUGMSGTL((MY_NAME, "%s: Entry:\n",__FUNCTION__));
  if((NULL == reqinfo) || (NULL == requests)) {
      ret = RSEMIB_BAD_INPUT;
  } else {
      switch (reqinfo->mode){
          case MODE_GET:
              DEBUGMSGTL((MY_NAME, "GET:\n"));
          if (0 <= (ret = get_rsuGnssDeviationError())){
              snmp_set_var_typed_integer(requests->requestvb, ASN_INTEGER, ret);
              ret = RSEMIB_OK;
          }
              break;
          default:
              ret = RSEMIB_BAD_MODE;
              break;
      }
      /* scalars and like are always index 0 with unqiue column.*/
      print_request_result(0, RSU_GNSS_DEVIATION_ERR_OID,requests,reqinfo->mode,ret);
  }
  if(ret < RSEMIB_OK) {
      set_error_states(ret);
  }
  /* Convert from MIB(negative) error to SNMP(positive) request error. */
  ret = handle_rsemib_errors(ret);
  if(SNMP_ERR_NOERROR < ret){ /* Set once and only if error. Only top handler need do this. */
      netsnmp_set_request_error(reqinfo, requests, ret);
  }
  DEBUGMSGTL((MY_NAME, "ret=%d:Exit.\n",ret));
  return ret;
}
int32_t handle_rsuGnssDeviationMsg(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo,
                       netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret = RSEMIB_OK;
  uint8_t data[RSU_ALERT_MSG_LENGTH_MAX];

  handler = handler; /* Silence of the Warnings. */
  reginfo = reginfo;
  DEBUGMSGTL((MY_NAME, "%s: Entry:\n",__FUNCTION__));
  if((NULL == reqinfo) || (NULL == requests)) {
      ret = RSEMIB_BAD_INPUT;
  } else {
      switch (reqinfo->mode){
          case MODE_GET:
              DEBUGMSGTL((MY_NAME, "GET:\n"));
              memset(data,0x0,sizeof(data));
              if(0 <= (ret = get_rsuGnssDeviationMsg(data))){
                  snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR, data, ret);
                  ret = RSEMIB_OK;
              }
              break;
          default:
              ret = RSEMIB_BAD_MODE;
              break;
      }
      /* scalars and like are always index 0 with unqiue column.*/
      print_request_result(0, RSU_GNSS_DEVIATION_MSG_OID,requests,reqinfo->mode,ret);
  }
  if(ret < RSEMIB_OK) {
      set_error_states(ret);
  }
  /* Convert from MIB(negative) error to SNMP(positive) request error. */
  ret = handle_rsemib_errors(ret);
  if(SNMP_ERR_NOERROR < ret){ /* Set once and only if error. Only top handler need do this. */
      netsnmp_set_request_error(reqinfo, requests, ret);
  }
  DEBUGMSGTL((MY_NAME, "ret=%d:Exit.\n",ret));
  return ret;
}
/* 5.18.1.10 GNSS NMEA Message:NMEA 0183 string (including the $ starting character and the ending <CR><LF>) */
int32_t handle_rsuGnssNmeaNotify(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo,
                       netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret = RSEMIB_OK;

  handler = handler; /* Silence of the Warnings. */
  reginfo = reginfo;
  DEBUGMSGTL((MY_NAME, "%s: Entry:\n",__FUNCTION__));
  if((NULL == reqinfo) || (NULL == requests)) {
      ret = RSEMIB_BAD_INPUT;
  } else {
      switch (reqinfo->mode){
          case MODE_GET:
              DEBUGMSGTL((MY_NAME, "GET:\n"));
              if(0 <= (ret = get_rsuGnssNmeaNotify())){
                  snmp_set_var_typed_integer(requests->requestvb, ASN_INTEGER, ret);
                  ret = RSEMIB_OK;
              }
              break;
          default:
              ret = RSEMIB_BAD_MODE;
              break;
      }
      /* scalars and like are always index 0 with unqiue column.*/
      print_request_result(0, RSU_GNSS_NMEA_NOTIFY_OID,requests,reqinfo->mode,ret);
  }
  if(ret < RSEMIB_OK) {
      set_error_states(ret);
  }
  /* Convert from MIB(negative) error to SNMP(positive) request error. */
  ret = handle_rsemib_errors(ret);
  if(SNMP_ERR_NOERROR < ret){ /* Set once and only if error. Only top handler need do this. */
      netsnmp_set_request_error(reqinfo, requests, ret);
  }
  DEBUGMSGTL((MY_NAME, "ret=%d:Exit.\n",ret));
  return ret;
}
int32_t handle_rsuGnssNmeaNotifyInterval(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo,
                       netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret = RSEMIB_OK;

  handler = handler; /* Silence of the Warnings. */
  reginfo = reginfo;
  DEBUGMSGTL((MY_NAME, "%s: Entry:\n",__FUNCTION__));
  if((NULL == reqinfo) || (NULL == requests)) {
      ret = RSEMIB_BAD_INPUT;
  } else {
      switch (reqinfo->mode){
          case MODE_GET:
              DEBUGMSGTL((MY_NAME, "GET:\n"));
              if(0 <= (ret = get_rsuGnssNmeaNotifyInterval())){
                  snmp_set_var_typed_integer(requests->requestvb, ASN_INTEGER, ret);
                  ret = RSEMIB_OK;
              }
              break;
          case MODE_SET_RESERVE1:
              break;
          case MODE_SET_RESERVE2:
              break; 
          case MODE_SET_FREE:
              break;
          case MODE_SET_ACTION:
              DEBUGMSGTL((MY_NAME, "ACTION:\n"));
              if(RSEMIB_OK == (ret = preserve_rsuGnssNmeaNotifyInterval())){
                  ret = action_rsuGnssNmeaNotifyInterval( *(requests->requestvb->val.integer) );
              }
              break;
          case MODE_SET_COMMIT:
              DEBUGMSGTL((MY_NAME, "COMMIT:\n"));
              if(RSEMIB_OK == (ret = commit_rsuGnssNmeaNotifyInterval())){
                  /*
                   * Back end magic here.
                   */
              }
              break;
          case MODE_SET_UNDO:
              DEBUGMSGTL((MY_NAME, "UNDO:\n"));
              ret = undo_rsuGnssNmeaNotifyInterval();
              break;
          default:
              ret = RSEMIB_BAD_MODE;
              break;
      }
      /* scalars and like are always index 0 with unqiue column.*/
      print_request_result(0, RSU_GNSS_NMEA_NOTIFY_INTERVAL_OID,requests,reqinfo->mode,ret);
  }
  if(ret < RSEMIB_OK) {
      set_error_states(ret);
  }
  /* Convert from MIB(negative) error to SNMP(positive) request error. */
  ret = handle_rsemib_errors(ret);
  if(SNMP_ERR_NOERROR < ret){ /* Set once and only if error. Only top handler need do this. */
      netsnmp_set_request_error(reqinfo, requests, ret);
  }
  DEBUGMSGTL((MY_NAME, "ret=%d:Exit.\n",ret));
  return ret;
}
/* 5.18.1.11 Certificate Error Message */
int32_t handle_rsuCertificateError(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo,
                       netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret = RSEMIB_OK;

  handler = handler; /* Silence of the Warnings. */
  reginfo = reginfo;
  DEBUGMSGTL((MY_NAME, "%s: Entry:\n",__FUNCTION__));
  if((NULL == reqinfo) || (NULL == requests)) {
      ret = RSEMIB_BAD_INPUT;
  } else {
      switch (reqinfo->mode){
          case MODE_GET:
              DEBUGMSGTL((MY_NAME, "GET:\n"));
              if(0 <= (ret = get_rsuCertificateError())){
                  snmp_set_var_typed_integer(requests->requestvb, ASN_INTEGER, ret);
                  ret = RSEMIB_OK;
              }
              break;
          default:
              ret = RSEMIB_BAD_MODE;
              break;
      }
      /* scalars and like are always index 0 with unqiue column.*/
      print_request_result(0, RSU_CERT_ERR_OID,requests,reqinfo->mode,ret);
  }
  if(ret < RSEMIB_OK) {
      set_error_states(ret);
  }
  /* Convert from MIB(negative) error to SNMP(positive) request error. */
  ret = handle_rsemib_errors(ret);
  if(SNMP_ERR_NOERROR < ret){ /* Set once and only if error. Only top handler need do this. */
      netsnmp_set_request_error(reqinfo, requests, ret);
  }
  DEBUGMSGTL((MY_NAME, "ret=%d:Exit.\n",ret));
  return ret;
}
int32_t handle_rsuCertificateMsg(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo,
                       netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret = RSEMIB_OK;
  uint8_t data[RSU_ALERT_MSG_LENGTH_MAX];

  handler = handler; /* Silence of the Warnings. */
  reginfo = reginfo;
  DEBUGMSGTL((MY_NAME, "%s: Entry:\n",__FUNCTION__));
  if((NULL == reqinfo) || (NULL == requests)) {
      ret = RSEMIB_BAD_INPUT;
  } else {
      switch (reqinfo->mode){
          case MODE_GET:
              DEBUGMSGTL((MY_NAME, "GET:\n"));
              memset(data,0x0,sizeof(data));
              if(0 <= (ret = get_rsuCertificateMsg(data))){
                  snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR, data, ret);
                  ret = RSEMIB_OK;
              }
              break;
          default:
              ret = RSEMIB_BAD_MODE;
              break;
      }
      /* scalars and like are always index 0 with unqiue column.*/
      print_request_result(0, RSU_CERT_MSG_OID,requests,reqinfo->mode,ret);
  }
  if(ret < RSEMIB_OK) {
      set_error_states(ret);
  }
  /* Convert from MIB(negative) error to SNMP(positive) request error. */
  ret = handle_rsemib_errors(ret);
  if(SNMP_ERR_NOERROR < ret){ /* Set once and only if error. Only top handler need do this. */
      netsnmp_set_request_error(reqinfo, requests, ret);
  }
  DEBUGMSGTL((MY_NAME, "ret=%d:Exit.\n",ret));
  return ret;
}
/* 5.18.1.12 Denial of Service Error Message */
int32_t handle_rsuServiceDenialError(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo,
                       netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret = RSEMIB_OK;

  handler = handler; /* Silence of the Warnings. */
  reginfo = reginfo;
  DEBUGMSGTL((MY_NAME, "%s: Entry:\n",__FUNCTION__));
  if((NULL == reqinfo) || (NULL == requests)) {
      ret = RSEMIB_BAD_INPUT;
  } else {
      switch (reqinfo->mode){
          case MODE_GET:
              DEBUGMSGTL((MY_NAME, "GET:\n"));
              if(0 <= (ret = get_rsuServiceDenialError())){
                  snmp_set_var_typed_integer(requests->requestvb, ASN_INTEGER, ret);
                  ret = RSEMIB_OK;
              }
              break;
          default:
              ret = RSEMIB_BAD_MODE;
              break;
      }
      /* scalars and like are always index 0 with unqiue column.*/
      print_request_result(0, RSU_SERVICE_DENIAL_ERR_OID,requests,reqinfo->mode,ret);
  }
  if(ret < RSEMIB_OK) {
      set_error_states(ret);
  }
  /* Convert from MIB(negative) error to SNMP(positive) request error. */
  ret = handle_rsemib_errors(ret);
  if(SNMP_ERR_NOERROR < ret){ /* Set once and only if error. Only top handler need do this. */
      netsnmp_set_request_error(reqinfo, requests, ret);
  }
  DEBUGMSGTL((MY_NAME, "ret=%d:Exit.\n",ret));
  return ret;
}
int32_t handle_rsuServiceDenialMsg(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo,
                       netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret = RSEMIB_OK;
  uint8_t data[RSU_ALERT_MSG_LENGTH_MAX];

  handler = handler; /* Silence of the Warnings. */
  reginfo = reginfo;
  DEBUGMSGTL((MY_NAME, "%s: Entry:\n",__FUNCTION__));
  if((NULL == reqinfo) || (NULL == requests)) {
      ret = RSEMIB_BAD_INPUT;
  } else {
      switch (reqinfo->mode){
          case MODE_GET:
              DEBUGMSGTL((MY_NAME, "GET:\n"));
              memset(data,0x0,sizeof(data));
              if(0 <= (ret = get_rsuServiceDenialMsg(data))){
                  snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR, data, ret);
                  ret = RSEMIB_OK;
              }
              break;
          default:
              ret = RSEMIB_BAD_MODE;
              break;
      }
      /* scalars and like are always index 0 with unqiue column.*/
      print_request_result(0, RSU_SERVICE_DENIAL_MSG_OID,requests,reqinfo->mode,ret);
  }
  if(ret < RSEMIB_OK) {
      set_error_states(ret);
  }
  /* Convert from MIB(negative) error to SNMP(positive) request error. */
  ret = handle_rsemib_errors(ret);
  if(SNMP_ERR_NOERROR < ret){ /* Set once and only if error. Only top handler need do this. */
      netsnmp_set_request_error(reqinfo, requests, ret);
  }
  DEBUGMSGTL((MY_NAME, "ret=%d:Exit.\n",ret));
  return ret;
}
/* 5.18.1.13 Watchdog Error Message */
int32_t handle_rsuWatchdogError(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo,
                       netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret = RSEMIB_OK;

  handler = handler; /* Silence of the Warnings. */
  reginfo = reginfo;
  DEBUGMSGTL((MY_NAME, "%s: Entry:\n",__FUNCTION__));
  if((NULL == reqinfo) || (NULL == requests)) {
      ret = RSEMIB_BAD_INPUT;
  } else {
      switch (reqinfo->mode){
          case MODE_GET:
              DEBUGMSGTL((MY_NAME, "GET:\n"));
              if(0 <= (ret = get_rsuWatchdogError())){
                  snmp_set_var_typed_integer(requests->requestvb, ASN_INTEGER, ret);
                  ret = RSEMIB_OK;
              }
              break;
          default:
              ret = RSEMIB_BAD_MODE;
              break;
      }
      /* scalars and like are always index 0 with unqiue column.*/
      print_request_result(0, RSU_WATCHDOG_ERR_OID,requests,reqinfo->mode,ret);
  }
  if(ret < RSEMIB_OK) {
      set_error_states(ret);
  }
  /* Convert from MIB(negative) error to SNMP(positive) request error. */
  ret = handle_rsemib_errors(ret);
  if(SNMP_ERR_NOERROR < ret){ /* Set once and only if error. Only top handler need do this. */
      netsnmp_set_request_error(reqinfo, requests, ret);
  }
  DEBUGMSGTL((MY_NAME, "ret=%d:Exit.\n",ret));
  return ret;
}
int32_t handle_rsuWatchdogMsg(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo,
                       netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret = RSEMIB_OK;
  uint8_t data[RSU_ALERT_MSG_LENGTH_MAX];

  handler = handler; /* Silence of the Warnings. */
  reginfo = reginfo;
  DEBUGMSGTL((MY_NAME, "%s: Entry:\n",__FUNCTION__));
  if((NULL == reqinfo) || (NULL == requests)) {
      ret = RSEMIB_BAD_INPUT;
  } else {
      switch (reqinfo->mode){
          case MODE_GET:
              DEBUGMSGTL((MY_NAME, "GET:\n"));
              memset(data,0x0,sizeof(data));
              if(0 <= (ret = get_rsuWatchdogMsg(data))){
                  snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR, data, ret);
                  ret = RSEMIB_OK;
              }
              break;
          default:
              ret = RSEMIB_BAD_MODE;
              break;
      }
      /* scalars and like are always index 0 with unqiue column.*/
      print_request_result(0, RSU_WATCHDOG_MSG_OID,requests,reqinfo->mode,ret);
  }
  if(ret < RSEMIB_OK) {
      set_error_states(ret);
  }
  /* Convert from MIB(negative) error to SNMP(positive) request error. */
  ret = handle_rsemib_errors(ret);
  if(SNMP_ERR_NOERROR < ret){ /* Set once and only if error. Only top handler need do this. */
      netsnmp_set_request_error(reqinfo, requests, ret);
  }
  DEBUGMSGTL((MY_NAME, "ret=%d:Exit.\n",ret));
  return ret;
}
/* 5.18.1.14 Enclosure Environment Message: tamper detection */
int32_t handle_rsuEnvironError(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo,
                       netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret = RSEMIB_OK;

  handler = handler; /* Silence of the Warnings. */
  reginfo = reginfo;
  DEBUGMSGTL((MY_NAME, "%s: Entry:\n",__FUNCTION__));
  if((NULL == reqinfo) || (NULL == requests)) {
      ret = RSEMIB_BAD_INPUT;
  } else {
      switch (reqinfo->mode){
          case MODE_GET:
              DEBUGMSGTL((MY_NAME, "GET:\n"));
              if(0 <= (ret = get_rsuEnvironError())){
                  snmp_set_var_typed_integer(requests->requestvb, ASN_INTEGER, ret);
                  ret = RSEMIB_OK;
              }
              break;
          default:
              ret = RSEMIB_BAD_MODE;
              break;
      }
      /* scalars and like are always index 0 with unqiue column.*/
      print_request_result(0, RSU_ENVIORNMENT_ERR_OID,requests,reqinfo->mode,ret);
  }
  if(ret < RSEMIB_OK) {
      set_error_states(ret);
  }
  /* Convert from MIB(negative) error to SNMP(positive) request error. */
  ret = handle_rsemib_errors(ret);
  if(SNMP_ERR_NOERROR < ret){ /* Set once and only if error. Only top handler need do this. */
      netsnmp_set_request_error(reqinfo, requests, ret);
  }
  DEBUGMSGTL((MY_NAME, "ret=%d:Exit.\n",ret));
  return ret;
}
int32_t handle_rsuEnvironMsg(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo,
                       netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret = RSEMIB_OK;
  uint8_t data[RSU_ALERT_MSG_LENGTH_MAX];

  handler = handler; /* Silence of the Warnings. */
  reginfo = reginfo;
  DEBUGMSGTL((MY_NAME, "%s: Entry:\n",__FUNCTION__));
  if((NULL == reqinfo) || (NULL == requests)) {
      ret = RSEMIB_BAD_INPUT;
  } else {
      switch (reqinfo->mode){
          case MODE_GET:
              DEBUGMSGTL((MY_NAME, "GET:\n"));
              memset(data,0x0,sizeof(data));
              if(0 <= (ret = get_rsuEnvironMsg(data))){
                  snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR, data, ret);
                  ret = RSEMIB_OK;
              }
              break;
          default:
              ret = RSEMIB_BAD_MODE;
              break;
      }
      /* scalars and like are always index 0 with unqiue column.*/
      print_request_result(0, RSU_ENVIORNMENT_MSG_OID,requests,reqinfo->mode,ret);
  }
  if(ret < RSEMIB_OK) {
      set_error_states(ret);
  }
  /* Convert from MIB(negative) error to SNMP(positive) request error. */
  ret = handle_rsemib_errors(ret);
  if(SNMP_ERR_NOERROR < ret){ /* Set once and only if error. Only top handler need do this. */
      netsnmp_set_request_error(reqinfo, requests, ret);
  }
  DEBUGMSGTL((MY_NAME, "ret=%d:Exit.\n",ret));
  return ret;
}
/* 5.18.2.11: Odd one out: error level but no error msg like the others: RO: 0..4: rsuAlertLevel_e. */
int32_t handle_rsuAlertLevel(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo,
                       netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret = RSEMIB_OK;

  handler = handler; /* Silence of the Warnings. */
  reginfo = reginfo;
  DEBUGMSGTL((MY_NAME, "%s: Entry:\n",__FUNCTION__));
  if((NULL == reqinfo) || (NULL == requests)) {
      ret = RSEMIB_BAD_INPUT;
  } else {
      switch (reqinfo->mode){
          case MODE_GET:
              DEBUGMSGTL((MY_NAME, "GET:\n"));
              if(0 <= (ret = get_rsuAlertLevel())){
                  snmp_set_var_typed_integer(requests->requestvb, ASN_INTEGER, ret);
                  ret = RSEMIB_OK;
              }
              break;
          default:
              ret = RSEMIB_BAD_MODE;
              break;
      }
      /* scalars and like are always index 0 with unqiue column.*/
      print_request_result(0, RSU_ALERT_LEVEL_OID,requests,reqinfo->mode,ret);
  }
  if(ret < RSEMIB_OK) {
      set_error_states(ret);
  }
  /* Convert from MIB(negative) error to SNMP(positive) request error. */
  ret = handle_rsemib_errors(ret);
  if(SNMP_ERR_NOERROR < ret){ /* Set once and only if error. Only top handler need do this. */
      netsnmp_set_request_error(reqinfo, requests, ret);
  }
  DEBUGMSGTL((MY_NAME, "ret=%d:Exit.\n",ret));
  return ret;
}
/* 5.18.3: RW: 0..255: seconds: int32_t rsuNotificationRepeatInterval = RSU_NOTIFY_INTERVAL_RATE_DEFAULT = 60 */
int32_t handle_rsuNotificationRepeatInterval(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo,
                       netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret = RSEMIB_OK;

  handler = handler; /* Silence of the Warnings. */
  reginfo = reginfo;
  DEBUGMSGTL((MY_NAME, "%s: Entry:\n",__FUNCTION__));
  if((NULL == reqinfo) || (NULL == requests)) {
      ret = RSEMIB_BAD_INPUT;
  } else {
      switch (reqinfo->mode){
          case MODE_GET:
              DEBUGMSGTL((MY_NAME, "GET:\n"));
              if(0 <= (ret = get_rsuNotificationRepeatInterval())){
                  snmp_set_var_typed_integer(requests->requestvb, ASN_INTEGER, ret);
                  ret = RSEMIB_OK;
              }
              break;
          case MODE_SET_RESERVE1:
              break;
          case MODE_SET_RESERVE2:
              break;
          case MODE_SET_FREE:
              break;
          case MODE_SET_ACTION:
              DEBUGMSGTL((MY_NAME, "ACTION:\n"));
              if(RSEMIB_OK == (ret = preserve_rsuNotificationRepeatInterval())){
                  ret = action_rsuNotificationRepeatInterval( *(requests->requestvb->val.integer) );
              }
              break;
          case MODE_SET_COMMIT:
              DEBUGMSGTL((MY_NAME, "COMMIT:\n"));
              if(RSEMIB_OK == (ret = commit_rsuNotificationRepeatInterval())){
                  /*
                   * Back end magic here.
                   */
              }
              break;
          case MODE_SET_UNDO:
              DEBUGMSGTL((MY_NAME, "UNDO:\n"));
              ret = undo_rsuNotificationRepeatInterval();
              break;
          default:
              ret = RSEMIB_BAD_MODE;
              break;
      }
      /* scalars and like are always index 0 with unqiue column.*/
      print_request_result(0, RSU_NOTIFY_REPEAT_INTERVAL_OID,requests,reqinfo->mode,ret);
  }
  if(ret < RSEMIB_OK) {
      set_error_states(ret);
  }
  /* Convert from MIB(negative) error to SNMP(positive) request error. */
  ret = handle_rsemib_errors(ret);
  if(SNMP_ERR_NOERROR < ret){ /* Set once and only if error. Only top handler need do this. */
      netsnmp_set_request_error(reqinfo, requests, ret);
  }
  DEBUGMSGTL((MY_NAME, "ret=%d:Exit.\n",ret));
  return ret;
}
/* 5.18.4: RW: 0..255: 0 = no retry = default: int32_t rsuNotificationMaxRetries = RSU_NOTIFY_MSG_RETRY_MIN */
int32_t handle_rsuNotificationMaxRetries(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo,
                       netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret = RSEMIB_OK;

  handler = handler; /* Silence of the Warnings. */
  reginfo = reginfo;
  DEBUGMSGTL((MY_NAME, "%s: Entry:\n",__FUNCTION__));
  if((NULL == reqinfo) || (NULL == requests)) {
      ret = RSEMIB_BAD_INPUT;
  } else {
      switch (reqinfo->mode){
          case MODE_GET:
              DEBUGMSGTL((MY_NAME, "GET:\n"));
              if(0 <= (ret = get_rsuNotificationMaxRetries())){
                  snmp_set_var_typed_integer(requests->requestvb, ASN_INTEGER, ret);
                  ret = RSEMIB_OK;
              }
              break;
          case MODE_SET_RESERVE1:
              break;
          case MODE_SET_RESERVE2:
              break;
          case MODE_SET_FREE:
              break;
          case MODE_SET_ACTION:
              DEBUGMSGTL((MY_NAME, "ACTION:\n"));
              if(RSEMIB_OK == (ret = preserve_rsuNotificationMaxRetries())){
                  ret = action_rsuNotificationMaxRetries( *(requests->requestvb->val.integer) );
              }
              break;
          case MODE_SET_COMMIT:
              DEBUGMSGTL((MY_NAME, "COMMIT:\n"));
              if(RSEMIB_OK == (ret = commit_rsuNotificationMaxRetries())){
                  /*
                   * Back end magic here.
                   */
              }
              break;
          case MODE_SET_UNDO:
              DEBUGMSGTL((MY_NAME, "UNDO:\n"));
              ret = undo_rsuNotificationMaxRetries();
              break;
          default:
              ret = RSEMIB_BAD_MODE;
              break;
      }
      /* scalars and like are always index 0 with unqiue column.*/
      print_request_result(0, RSU_NOTIFY_MAX_RESTRIES_OID,requests,reqinfo->mode,ret);
  }
  if(ret < RSEMIB_OK) {
      set_error_states(ret);
  }
  /* Convert from MIB(negative) error to SNMP(positive) request error. */
  ret = handle_rsemib_errors(ret);
  if(SNMP_ERR_NOERROR < ret){ /* Set once and only if error. Only top handler need do this. */
      netsnmp_set_request_error(reqinfo, requests, ret);
  }
  DEBUGMSGTL((MY_NAME, "ret=%d:Exit.\n",ret));
  return ret;
}
void install_rsuAsync_handlers(void)
{
  /* Clear local statics. */
  rsuAsync_error_states = 0x0;

  /* 5.18.1.1 File Integrity Check Error Message: AMH SAR & IMF. */
  if(MIB_REGISTERED_OK != netsnmp_register_scalar(netsnmp_create_handler_registration
                          ("messageFileIntegrityError",
                           handle_messageFileIntegrityError,
                           messageFileIntegrityError_oid,
                           OID_LENGTH(messageFileIntegrityError_oid),
                           HANDLER_CAN_RONLY))){
      DEBUGMSGTL((MY_NAME, "messageFileIntegrityError install FAIL.\n"));
      set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
  }
  if(MIB_REGISTERED_OK != netsnmp_register_scalar(netsnmp_create_handler_registration
                          ("rsuMsgFileIntegrityMsg",
                           handle_rsuMsgFileIntegrityMsg,
                           rsuMsgFileIntegrityMsg_oid,
                           OID_LENGTH(rsuMsgFileIntegrityMsg_oid),
                           HANDLER_CAN_RONLY))){
      DEBUGMSGTL((MY_NAME, "rsuMsgFileIntegrityMsg install FAIL.\n"));
      set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
  }
  /* 5.18.1.2 Storage Integrity Error Message: Errors in file system.*/
  if(MIB_REGISTERED_OK != netsnmp_register_scalar(netsnmp_create_handler_registration
                          ("rsuSecStorageIntegrityError",
                           handle_rsuSecStorageIntegrityError,
                           rsuSecStorageIntegrityError_oid,
                           OID_LENGTH(rsuSecStorageIntegrityError_oid),
                           HANDLER_CAN_RONLY))){
      DEBUGMSGTL((MY_NAME, "rsuSecStorageIntegrityError install FAIL.\n"));
      set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
  }
  if(MIB_REGISTERED_OK != netsnmp_register_scalar(netsnmp_create_handler_registration
                          ("rsuSecStorageIntegrityMsg",
                           handle_rsuSecStorageIntegrityMsg,
                           rsuSecStorageIntegrityMsg_oid,
                           OID_LENGTH(rsuSecStorageIntegrityMsg_oid),
                           HANDLER_CAN_RONLY))){
      DEBUGMSGTL((MY_NAME, "rsuSecStorageIntegrityMsg install FAIL.\n"));
      set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
  }
  /* 5.18.1.3 Authorization Error Message: invalid security creds: Is failed login attempt one? */
  if(MIB_REGISTERED_OK != netsnmp_register_scalar(netsnmp_create_handler_registration
                          ("rsuAuthError",
                           handle_rsuAuthError,
                           rsuAuthError_oid,
                           OID_LENGTH(rsuAuthError_oid),
                           HANDLER_CAN_RONLY))){
      DEBUGMSGTL((MY_NAME, "rsuAuthError install FAIL.\n"));
      set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
  }
  if(MIB_REGISTERED_OK != netsnmp_register_scalar(netsnmp_create_handler_registration
                          ("rsuAuthMsg",
                           handle_rsuAuthMsg,
                           rsuAuthMsg_oid,
                           OID_LENGTH(rsuAuthMsg_oid),
                           HANDLER_CAN_RONLY))){
      DEBUGMSGTL((MY_NAME, "rsuAuthMsg install FAIL.\n"));
      set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
  }
  /* 5.18.1.4 Signature Verification Error Message: Any failed signature on WSM's */
  if(MIB_REGISTERED_OK != netsnmp_register_scalar(netsnmp_create_handler_registration
                          ("rsuSignatureVerifyError",
                           handle_rsuSignatureVerifyError,
                           rsuSignatureVerifyError_oid,
                           OID_LENGTH(rsuSignatureVerifyError_oid),
                           HANDLER_CAN_RONLY))){
      DEBUGMSGTL((MY_NAME, "rsuSignatureVerifyError install FAIL.\n"));
      set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
  }
  if(MIB_REGISTERED_OK != netsnmp_register_scalar(netsnmp_create_handler_registration
                          ("rsuSignatureVerifyMsg",
                           handle_rsuSignatureVerifyMsg,
                           rsuSignatureVerifyMsg_oid,
                           OID_LENGTH(rsuSignatureVerifyMsg_oid),
                           HANDLER_CAN_RONLY))){
      DEBUGMSGTL((MY_NAME, "rsuSignatureVerifyMsg install FAIL.\n"));
      set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
  }
  /* 5.18.1.5 Access Error Message: error or rejection due to a violation of the Access Control List.*/
  if(MIB_REGISTERED_OK != netsnmp_register_scalar(netsnmp_create_handler_registration
                          ("rsuAccessError",
                           handle_rsuAccessError,
                           rsuAccessError_oid,
                           OID_LENGTH(rsuAccessError_oid),
                           HANDLER_CAN_RONLY))){
      DEBUGMSGTL((MY_NAME, "rsuAccessError install FAIL.\n"));
      set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
  }
  if(MIB_REGISTERED_OK != netsnmp_register_scalar(netsnmp_create_handler_registration
                          ("rsuAccessMsg",
                           handle_rsuAccessMsg,
                           rsuAccessMsg_oid,
                           OID_LENGTH(rsuAccessMsg_oid),
                           HANDLER_CAN_RONLY))){
      DEBUGMSGTL((MY_NAME, "rsuAccessMsg install FAIL.\n"));
      set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
  }
  /* 5.18.1.6 Time Source Lost Message: Lost time source: ie lost GNSS fix.*/
  if(MIB_REGISTERED_OK != netsnmp_register_scalar(netsnmp_create_handler_registration
                          ("rsuTimeSourceLost",
                           handle_rsuTimeSourceLost,
                           rsuTimeSourceLost_oid,
                           OID_LENGTH(rsuTimeSourceLost_oid),
                           HANDLER_CAN_RONLY))){
      DEBUGMSGTL((MY_NAME, "rsuTimeSourceLost install FAIL.\n"));
      set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
  }
  if(MIB_REGISTERED_OK != netsnmp_register_scalar(netsnmp_create_handler_registration
                          ("rsuTimeSourceLostMsg",
                           handle_rsuTimeSourceLostMsg,
                           rsuTimeSourceLostMsg_oid,
                           OID_LENGTH(rsuTimeSourceLostMsg_oid),
                           HANDLER_CAN_RONLY))){
      DEBUGMSGTL((MY_NAME, "rsuTimeSourceLostMsg install FAIL.\n"));
      set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
  }
  /* 5.18.1.7 Time Source Mismatch Message: deviation between two time sources exceeds vendor-defined threshold. Who is vendor? */
  if(MIB_REGISTERED_OK != netsnmp_register_scalar(netsnmp_create_handler_registration
                          ("rsuTimeSourceMismatch",
                           handle_rsuTimeSourceMismatch,
                           rsuTimeSourceMismatch_oid,
                           OID_LENGTH(rsuTimeSourceMismatch_oid),
                           HANDLER_CAN_RONLY))){
      DEBUGMSGTL((MY_NAME, "rsuTimeSourceMismatch install FAIL.\n"));
      set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
  }
  if(MIB_REGISTERED_OK != netsnmp_register_scalar(netsnmp_create_handler_registration
                          ("rsuTimeSourceMismatchMsg",
                           handle_rsuTimeSourceMismatchMsg,
                           rsuTimeSourceMismatchMsg_oid,
                           OID_LENGTH(rsuTimeSourceMismatchMsg_oid),
                           HANDLER_CAN_RONLY))){
      DEBUGMSGTL((MY_NAME, "rsuTimeSourceMismatchMsg install FAIL.\n"));
      set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
  }
  /* 5.18.1.8 GNSS Anomaly Message: report any anomalous GNSS readings: Sky's the limit here. */
  if(MIB_REGISTERED_OK != netsnmp_register_scalar(netsnmp_create_handler_registration
                          ("rsuGnssAnomaly",
                           handle_rsuGnssAnomaly,
                           rsuGnssAnomaly_oid,
                           OID_LENGTH(rsuGnssAnomaly_oid),
                           HANDLER_CAN_RONLY))){
      DEBUGMSGTL((MY_NAME, "rsuGnssAnomaly install FAIL.\n"));
      set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
  }
  if(MIB_REGISTERED_OK != netsnmp_register_scalar(netsnmp_create_handler_registration
                          ("rsuGnssAnomalyMsg",
                           handle_rsuGnssAnomalyMsg,
                           rsuGnssAnomalyMsg_oid,
                           OID_LENGTH(rsuGnssAnomalyMsg_oid),
                           HANDLER_CAN_RONLY))){
      DEBUGMSGTL((MY_NAME, "rsuGnssAnomalyMsg install FAIL.\n"));
      set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
  }
  /* 5.18.1.9 GNSS Deviation Error Message: GNSS position deviation greater than allowed.*/
  if(MIB_REGISTERED_OK != netsnmp_register_scalar(netsnmp_create_handler_registration
                          ("rsuGnssDeviationError",
                           handle_rsuGnssDeviationError,
                           rsuGnssDeviationError_oid,
                           OID_LENGTH(rsuGnssDeviationError_oid),
                           HANDLER_CAN_RONLY))){
      DEBUGMSGTL((MY_NAME, "rsuGnssDeviationError install FAIL.\n"));
      set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
  }
  if(MIB_REGISTERED_OK != netsnmp_register_scalar(netsnmp_create_handler_registration
                          ("rsuGnssDeviationMsg",
                           handle_rsuGnssDeviationMsg,
                           rsuGnssDeviationMsg_oid,
                           OID_LENGTH(rsuGnssDeviationMsg_oid),
                           HANDLER_CAN_RONLY))){
      DEBUGMSGTL((MY_NAME, "rsuGnssDeviationMsg install FAIL.\n"));
      set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
  }
  /* 5.18.1.10 GNSS NMEA Message:NMEA 0183 string (including the $ starting character and the ending <CR><LF>) */
  if(MIB_REGISTERED_OK != netsnmp_register_scalar(netsnmp_create_handler_registration
                          ("rsuGnssNmeaNotify",
                           handle_rsuGnssNmeaNotify,
                           rsuGnssNmeaNotify_oid,
                           OID_LENGTH(rsuGnssNmeaNotify_oid),
                           HANDLER_CAN_RONLY))){
      DEBUGMSGTL((MY_NAME, "rsuGnssNmeaNotify install FAIL.\n"));
      set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
  }
  if(MIB_REGISTERED_OK != netsnmp_register_scalar(netsnmp_create_handler_registration
                          ("rsuGnssNmeaNotifyInterval",
                           handle_rsuGnssNmeaNotifyInterval,
                           rsuGnssNmeaNotifyInterval_oid,
                           OID_LENGTH(rsuGnssNmeaNotifyInterval_oid),
                           HANDLER_CAN_RWRITE))){
      DEBUGMSGTL((MY_NAME, "rsuGnssNmeaNotifyInterval install FAIL.\n"));
      set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
  }
  /* 
   * There is a stutter step in alert level and error message here. A bit weird. 
   */
  /* 5.18.1.11 Certificate Error Message */
  if(MIB_REGISTERED_OK != netsnmp_register_scalar(netsnmp_create_handler_registration
                          ("rsuCertificateError",
                           handle_rsuCertificateError,
                           rsuCertificateError_oid,
                           OID_LENGTH(rsuCertificateError_oid),
                           HANDLER_CAN_RONLY))){
      DEBUGMSGTL((MY_NAME, "rsuCertificateError install FAIL.\n"));
      set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
  }
  /* 5.18.2.11: Odd one out: error level but no error msg like the others: RO: 1..4 */
  if(MIB_REGISTERED_OK != netsnmp_register_scalar(netsnmp_create_handler_registration
                          ("rsuAlertLevel",
                           handle_rsuAlertLevel,
                           rsuAlertLevel_oid,
                           OID_LENGTH(rsuAlertLevel_oid),
                           HANDLER_CAN_RONLY))){
      DEBUGMSGTL((MY_NAME, "rsuAlertLevel install FAIL.\n"));
      set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
  }
  /* 5.18.2.12 Certificate Error Message */
  if(MIB_REGISTERED_OK != netsnmp_register_scalar(netsnmp_create_handler_registration
                          ("rsuCertificateMsg",
                           handle_rsuCertificateMsg,
                           rsuCertificateMsg_oid,
                           OID_LENGTH(rsuCertificateMsg_oid),
                           HANDLER_CAN_RONLY))){
      DEBUGMSGTL((MY_NAME, "rsuCertificateMsg install FAIL.\n"));
      set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
  }
  /* 5.18.1.12 Denial of Service Error Message */
  if(MIB_REGISTERED_OK != netsnmp_register_scalar(netsnmp_create_handler_registration
                          ("rsuServiceDenialError",
                           handle_rsuServiceDenialError,
                           rsuServiceDenialError_oid,
                           OID_LENGTH(rsuServiceDenialError_oid),
                           HANDLER_CAN_RONLY))){
      DEBUGMSGTL((MY_NAME, "rsuServiceDenialError install FAIL.\n"));
      set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
  }
  if(MIB_REGISTERED_OK != netsnmp_register_scalar(netsnmp_create_handler_registration
                          ("rsuServiceDenialMsg",
                           handle_rsuServiceDenialMsg,
                           rsuServiceDenialMsg_oid,
                           OID_LENGTH(rsuServiceDenialMsg_oid),
                           HANDLER_CAN_RONLY))){
      DEBUGMSGTL((MY_NAME, "rsuServiceDenialMsg install FAIL.\n"));
      set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
  }
  /* 5.18.1.13 Watchdog Error Message */
  if(MIB_REGISTERED_OK != netsnmp_register_scalar(netsnmp_create_handler_registration
                          ("rsuWatchdogError",
                           handle_rsuWatchdogError,
                           rsuWatchdogError_oid,
                           OID_LENGTH(rsuWatchdogError_oid),
                           HANDLER_CAN_RONLY))){
      DEBUGMSGTL((MY_NAME, "rsuWatchdogError install FAIL.\n"));
      set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
  }
  if(MIB_REGISTERED_OK != netsnmp_register_scalar(netsnmp_create_handler_registration
                          ("rsuWatchdogMsg",
                           handle_rsuWatchdogMsg,
                           rsuWatchdogMsg_oid,
                           OID_LENGTH(rsuWatchdogMsg_oid),
                           HANDLER_CAN_RONLY))){
      DEBUGMSGTL((MY_NAME, "rsuWatchdogMsg install FAIL.\n"));
      set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
  }
  /* 5.18.1.14 Enclosure Environment Message: tamper detection */
  if(MIB_REGISTERED_OK != netsnmp_register_scalar(netsnmp_create_handler_registration
                          ("rsuEnvironError",
                           handle_rsuEnvironError,
                           rsuEnvironError_oid,
                           OID_LENGTH(rsuEnvironError_oid),
                           HANDLER_CAN_RONLY))){
      DEBUGMSGTL((MY_NAME, "rsuEnvironError install FAIL.\n"));
      set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
  }
  if(MIB_REGISTERED_OK != netsnmp_register_scalar(netsnmp_create_handler_registration
                          ("rsuEnvironMsg",
                           handle_rsuEnvironMsg,
                           rsuEnvironMsg_oid,
                           OID_LENGTH(rsuEnvironMsg_oid),
                           HANDLER_CAN_RONLY))){
      DEBUGMSGTL((MY_NAME, "rsuEnvironMsg install FAIL.\n"));
      set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
  }
  /* 5.18.3 Notification Repeat Interval */
  if(MIB_REGISTERED_OK != netsnmp_register_scalar(netsnmp_create_handler_registration
                          ("rsuNotificationRepeatInterval",
                           handle_rsuNotificationRepeatInterval,
                           rsuNotificationRepeatInterval_oid,
                           OID_LENGTH(rsuNotificationRepeatInterval_oid),
                           HANDLER_CAN_RWRITE))){
      DEBUGMSGTL((MY_NAME, "rsuNotificationRepeatInterval install FAIL.\n"));
      set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
  }
  /* 5.18.4 Notification Maximum Retries */
  if(MIB_REGISTERED_OK != netsnmp_register_scalar(netsnmp_create_handler_registration
                          ("rsuNotificationMaxRetries",
                           handle_rsuNotificationMaxRetries,
                           rsuNotificationMaxRetries_oid,
                           OID_LENGTH(rsuNotificationMaxRetries_oid),
                           HANDLER_CAN_RWRITE))){
      DEBUGMSGTL((MY_NAME, "rsuNotificationMaxRetries install FAIL.\n"));
      set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
  }
}
void destroy_rsuAsync(void)
{

}
/* App support: Start thread, get ip+port, open socket and fire away. */
bool_t mib_shmlock_trylockw(wsu_shmlock_t * lock)
{
    if (pthread_rwlock_trywrlock(&lock->rwlock) != EOK)
        return FALSE;

    return TRUE;
}
bool_t mib_shmlock_unlockw(wsu_shmlock_t * lock)
{

    if (pthread_rwlock_unlock(&lock->rwlock) != EOK)
        return FALSE;

    return TRUE;
}

/* Dupe from pal/wsu_sharedmem.h. */
bool_t mib_shmlock_kill( wsu_shmlock_t *lock )
{
	int32_t ret = TRUE;

	if (pthread_rwlock_destroy(&lock->rwlock) != EOK)
		ret = FALSE;
  lock->marker = OBJ_DESTROYED_MARKER;
	return ret;
}
/* Dupe from pal/wsu_sharedmem.h. */
static void * mib_share_init (size_t size, char_t *spath )
{
  void *shrm_start=NULL;
  int32_t fd=-1;

  fd = shm_open(spath, O_RDWR|O_CREAT|O_EXCL, 0664);
  if (fd >= 0) {
      if(-1 == ftruncate(fd, size)) {
          DEBUGMSGTL((MY_NAME,"mib_share_init: ftruncate Failed.\n"));
          set_error_states(RSEMIB_SHM_INIT_FAILED);
          return NULL;
      }
  } else {
      fd = shm_open(spath, O_RDWR, 0664);
      if (fd < 0) {
          DEBUGMSGTL((MY_NAME,"mib_share_init: shm_open Failed:[%s]\n", spath));
          set_error_states(RSEMIB_SHM_INIT_FAILED);
          return NULL;
      }
  }
  shrm_start = mmap(NULL, size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
  close(fd);
  // If you pass MAP_SHARED|MAP_ANON to mmap, it's an illegal combination, and it seems to
  //    return -1 as the error response.  So we test for -1 in addition to a null pointer,
  //    and return NULL so our caller doesn't have to test for two things.
  if ((int64_t) shrm_start == -1 || (int64_t) shrm_start == 0) {
      DEBUGMSGTL((MY_NAME,"mib_share_init: mmap failed for shm [%s] fd %d size %ld, returned %p\n", spath, fd, size, shrm_start));
      set_error_states(RSEMIB_SHM_INIT_FAILED);
      return NULL;
  }
  return shrm_start;
}
/* Dupe from pal/wsu_sharedmem.h. */
static void mib_share_kill( void *shrm_start, size_t shm_size)
{
  /* If this fails at shutdown, we don't care. But if we don't check, we get compile warning.*/
  if (munmap(shrm_start, shm_size) < 0) DEBUGMSGTL((MY_NAME_EXTRA,"wsu_share_kill: munmap failed, errno=%d\n", errno));
}
#if defined(ENABLE_ASYNC_BACKUP)
static uint16_t crc16(const uint8_t * data_p, size_t length)
{
    uint8_t x;
    uint16_t crc = 0xFFFF;

    while (length--){
        x = crc >> 8 ^ *data_p++;
        x ^= x>>4;
        crc = (crc << 8) ^ ((uint16_t)(x << 12)) ^ ((uint16_t)(x <<5)) ^ ((uint16_t)x);
    }
    return crc;
}
void  commit_rsuAsync_to_disk(rsuAsync_t * Async) 
{
  FILE    * file_out       = NULL;
  uint32_t  mib_byte_count = 0;
  uint32_t  write_count    = 0;
  int32_t   ret            = RSEMIB_OK;

  if(NULL == Async){
      ret = RSEMIB_BAD_INPUT; 
  } else {
      DEBUGMSGTL((MY_NAME, "commit_to_disk:fopen(%s).\n",Async->filePathName));
      if (NULL == (file_out = fopen(Async->filePathName, "wb"))){
          DEBUGMSGTL((MY_NAME, "commit_to_disk: fopen fail.\n"));
          ret = RSEMIB_FOPEN_FAIL;
      } else {
          Async->crc = (uint32_t)crc16((const uint8_t *)Async, sizeof(rsuAsync_t) - 4);
          mib_byte_count = sizeof(rsuAsync_t);
          write_count = fwrite((uint8_t *)Async, mib_byte_count, 1, file_out);
          if (write_count == 1){
              DEBUGMSGTL((MY_NAME, "commit_to_disk: SUCCESS: mib_byte_count=(0x%X).\n",mib_byte_count));
              ret = RSEMIB_OK;
          } else {
              DEBUGMSGTL((MY_NAME, "commit_to_disk: FAIL: write_count is 0: mib_byte_count=(0x%X): errno=(%d:%s).\n",mib_byte_count,errno,strerror(errno) ));
              ret = RSEMIB_FWRITE_FAIL;
          }
          fflush(file_out);
          fclose(file_out);
      }
  }
  if(RSEMIB_OK != ret){
      set_error_states(ret);
  }
}
#endif
// rsuAsync.c will clear Msg as ack that its has been serviced.
// So if not NULL, you know the message has not gone out yet.
// In that case, all you can do is throw your new message on the floor and set error state.
// TODO: add metrics for stats.
void *rsuAsyncThr(void __attribute__((unused)) *arg)
{
  uint32_t       half_second_counter = 0x0;
  rsuhealth_t *  shm_rsuhealth_ptr = NULL;
  rsuhealth_t    local_rsuhealth;
  uint32_t       async_shm_lockw_fail = 0;

  half_second_counter = 0x0;
  shm_rsuhealth_ptr  = NULL;
  memset(&local_rsuhealth,0x0,sizeof(local_rsuhealth));

  /* Set to DENSO empty cause zero'ed value is rsuAlertLevel_info */
  /* .1 to .14 exclduing .10 */
  local_rsuhealth.messageFileIntegrityError = rsuAlertLevel_denso_empty;
  local_rsuhealth.rsuSecStorageIntegrityError = rsuAlertLevel_denso_empty;
  local_rsuhealth.rsuAuthError = rsuAlertLevel_denso_empty;
  local_rsuhealth.rsuSignatureVerifyError = rsuAlertLevel_denso_empty;
  local_rsuhealth.rsuAccessError = rsuAlertLevel_denso_empty;
  local_rsuhealth.rsuTimeSourceLost = rsuAlertLevel_denso_empty;
  local_rsuhealth.rsuTimeSourceMismatch = rsuAlertLevel_denso_empty;
  local_rsuhealth.rsuGnssAnomaly = rsuAlertLevel_denso_empty;
  local_rsuhealth.rsuGnssDeviationError = rsuAlertLevel_denso_empty;
  local_rsuhealth.rsuCertificateError = rsuAlertLevel_denso_empty;
  local_rsuhealth.rsuServiceDenialError = rsuAlertLevel_denso_empty;
  local_rsuhealth.rsuWatchdogError = rsuAlertLevel_denso_empty;
  local_rsuhealth.rsuEnvironError = rsuAlertLevel_denso_empty;
  async_shm_lockw_fail = 0;

  while(mainloop) { /* Forever check SHM for any updates. */
      usleep(480 * 1000); /* TWICE, or more, the rate of RSUDIAG and others. */
      if(NULL == shm_rsuhealth_ptr) {
         if (NULL == (shm_rsuhealth_ptr = mib_share_init(sizeof(rsuhealth_t), RSUHEALTH_SHM_PATH))) {
              shm_rsuhealth_ptr = NULL; /* Keep going, not fatal. */
              set_error_states(RSEMIB_SHM_INIT_FAILED);
         } else {
              clear_error_states(RSEMIB_SHM_INIT_FAILED);
         }
      } 
      if(NULL != shm_rsuhealth_ptr) {
          if(WTRUE == mib_shmlock_trylockw(&shm_rsuhealth_ptr->h.ch_lock)) {
              clear_error_states(RSEMIB_LOCK_FAIL);
              if(WTRUE == shm_rsuhealth_ptr->h.ch_data_valid) {
                  clear_error_states(RSEMIB_SHM_NOT_VALID);
                  if (0 == shm_rsuhealth_ptr->trapsenabled){
                      shm_rsuhealth_ptr->trapsenabled = 1; /* Open for business */
#if defined(EXTRA_DEBUG)
                      I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"snmp traps enabled.\n");
#endif
                  }
                  #if defined(EXTRA_EXTRA_DEBUG)
                  if(half_second_counter < 120) I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"shm_rsuhealth_ptr->h.ch_data_valid\n");
                  #endif
                  /* Grab local copy and clear error to denote it has been serviced. */
                  memcpy(&local_rsuhealth, shm_rsuhealth_ptr, sizeof(local_rsuhealth));
                  /* .1 */
                  shm_rsuhealth_ptr->messageFileIntegrityError = rsuAlertLevel_denso_empty;
                  /* .2 */
                  shm_rsuhealth_ptr->rsuSecStorageIntegrityError = rsuAlertLevel_denso_empty;
                  /* .3 */
                  shm_rsuhealth_ptr->rsuAuthError = rsuAlertLevel_denso_empty;
                  /* .4 */
                  shm_rsuhealth_ptr->rsuSignatureVerifyError = rsuAlertLevel_denso_empty;
                  /* .5 */
                  shm_rsuhealth_ptr->rsuAccessError = rsuAlertLevel_denso_empty;
                  /* .6 */
                  shm_rsuhealth_ptr->rsuTimeSourceLost = rsuAlertLevel_denso_empty;
                  /* .7 */
                  shm_rsuhealth_ptr->rsuTimeSourceMismatch = rsuAlertLevel_denso_empty;
                  /* .8 */
                  shm_rsuhealth_ptr->rsuGnssAnomaly = rsuAlertLevel_denso_empty;
                  /* .9 */
                  shm_rsuhealth_ptr->rsuGnssDeviationError = rsuAlertLevel_denso_empty;
                  /* .10 rsuGnssOutput is not an error or notifcation */
                  /* .11 */
                  shm_rsuhealth_ptr->rsuCertificateError = rsuAlertLevel_denso_empty;
                  /* .12 */
                  shm_rsuhealth_ptr->rsuServiceDenialError = rsuAlertLevel_denso_empty;
                  /* .13 */
                  shm_rsuhealth_ptr->rsuWatchdogError = rsuAlertLevel_denso_empty;
                  /* .14 */
                  shm_rsuhealth_ptr->rsuEnvironError = rsuAlertLevel_denso_empty;
              } else {
                 /* Not fatal. */
                 set_error_states(RSEMIB_SHM_NOT_VALID);
              }
              if(WFALSE == mib_shmlock_unlockw(&shm_rsuhealth_ptr->h.ch_lock)) {
                  /* Not fatal. */
                  #if defined(EXTRA_DEBUG)
                  I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"wsu_shmlock_unlockw() failed.\n");
                  #endif                  
              }
          } else {
              #if defined(EXTRA_DEBUG)
              if(async_shm_lockw_fail < 10) { /* Should be rare event. */
                  I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"wsu_shmlock_lockw() failed(%d)\n",async_shm_lockw_fail);
              }
              #endif
              async_shm_lockw_fail++;
              set_error_states(RSEMIB_LOCK_FAIL);
          }
          /* We have copy of SHM now. Only work on that. */
          /* 5.18.1.1 File Integrity Check Error Message: AMH SAR & IMF: i.e. No valid message frame in UPER payload. */
          if(local_rsuhealth.messageFileIntegrityError != rsuAlertLevel_denso_empty) {
              if(strlen(local_rsuhealth.rsuMsgFileIntegrityMsg) > RSU_ALERT_MSG_LENGTH_MAX) {
                  set_messageFileIntegrity(
                      (rsuAlertLevel_e)local_rsuhealth.messageFileIntegrityError
                      ,(uint8_t *)local_rsuhealth.rsuMsgFileIntegrityMsg
                      ,RSU_ALERT_MSG_LENGTH_MAX);
              } else {
                  set_messageFileIntegrity(
                      (rsuAlertLevel_e)local_rsuhealth.messageFileIntegrityError
                      ,(uint8_t *)local_rsuhealth.rsuMsgFileIntegrityMsg
                      ,strlen(local_rsuhealth.rsuMsgFileIntegrityMsg));
              }
              local_rsuhealth.messageFileIntegrityError = rsuAlertLevel_denso_empty; /* Clear spot */
          }

          /* 5.18.1.2 Storage Integrity Error Message: Errors in file system. */
          if(local_rsuhealth.rsuSecStorageIntegrityError != rsuAlertLevel_denso_empty) {
              if(strlen(local_rsuhealth.rsuSecStorageIntegrityMsg) > RSU_ALERT_MSG_LENGTH_MAX) {
                  set_rsuSecStorageIntegrity(
                      (rsuAlertLevel_e)local_rsuhealth.rsuSecStorageIntegrityError
                      ,(uint8_t *)local_rsuhealth.rsuSecStorageIntegrityMsg
                      ,RSU_ALERT_MSG_LENGTH_MAX);
              } else {
                  set_rsuSecStorageIntegrity(
                      (rsuAlertLevel_e)local_rsuhealth.rsuSecStorageIntegrityError
                      ,(uint8_t *)local_rsuhealth.rsuSecStorageIntegrityMsg
                      ,strlen(local_rsuhealth.rsuSecStorageIntegrityMsg));
              }
              local_rsuhealth.rsuSecStorageIntegrityError = rsuAlertLevel_denso_empty; /* Clear spot */
          }
          /* 5.18.1.3 Authorization Error Message: invalid security creds: Is failed login attempt one? */
          if(local_rsuhealth.rsuAuthError != rsuAlertLevel_denso_empty) {
              if(strlen(local_rsuhealth.rsuAuthMsg) > RSU_ALERT_MSG_LENGTH_MAX) {
                  set_rsuAuth(
                      (rsuAlertLevel_e)local_rsuhealth.rsuAuthError
                      ,(uint8_t *)local_rsuhealth.rsuAuthMsg
                      ,RSU_ALERT_MSG_LENGTH_MAX);
              } else {
                  set_rsuAuth(
                      (rsuAlertLevel_e)local_rsuhealth.rsuAuthError
                      ,(uint8_t *)local_rsuhealth.rsuAuthMsg
                      ,strlen(local_rsuhealth.rsuAuthMsg));
              }
              local_rsuhealth.rsuAuthError = rsuAlertLevel_denso_empty; /* Clear spot */
          }
          /* 5.18.1.4 Signature Verification Error Message: Any failed signature on WSM's. */
          if(local_rsuhealth.rsuSignatureVerifyError != rsuAlertLevel_denso_empty) {
              if(strlen(local_rsuhealth.rsuSignatureVerifyMsg) > RSU_ALERT_MSG_LENGTH_MAX) {
                  set_rsuSignatureVerify(
                      (rsuAlertLevel_e)local_rsuhealth.rsuSignatureVerifyError
                      ,(uint8_t *)local_rsuhealth.rsuSignatureVerifyMsg
                      ,RSU_ALERT_MSG_LENGTH_MAX);
              } else {
                  set_rsuSignatureVerify(
                      (rsuAlertLevel_e)local_rsuhealth.rsuSignatureVerifyError
                      ,(uint8_t *)local_rsuhealth.rsuSignatureVerifyMsg
                      ,strlen(local_rsuhealth.rsuSignatureVerifyMsg));
              }
              local_rsuhealth.rsuSignatureVerifyError = rsuAlertLevel_denso_empty; /* Clear spot */
          }
          /* 5.18.1.5 Access Error Message: error or rejection due to a violation of the Access Control List.*/
          if(local_rsuhealth.rsuAccessError != rsuAlertLevel_denso_empty) {
              if(strlen(local_rsuhealth.rsuAccessMsg) > RSU_ALERT_MSG_LENGTH_MAX) {
                  set_rsuAccess(
                      (rsuAlertLevel_e)local_rsuhealth.rsuAccessError
                      ,(uint8_t *)local_rsuhealth.rsuAccessMsg
                      ,RSU_ALERT_MSG_LENGTH_MAX);
              } else {
                  set_rsuAccess(
                      (rsuAlertLevel_e)local_rsuhealth.rsuAccessError
                      ,(uint8_t *)local_rsuhealth.rsuAccessMsg
                      ,strlen(local_rsuhealth.rsuAccessMsg));
              }
              local_rsuhealth.rsuAccessError = rsuAlertLevel_denso_empty; /* Clear spot */
          }
          /* 5.18.1.6 Time Source Lost Message: Lost time source: ie lost GNSS fix. */
          if(local_rsuhealth.rsuTimeSourceLost != rsuAlertLevel_denso_empty) {
              if(strlen(local_rsuhealth.rsuTimeSourceLostMsg) > RSU_ALERT_MSG_LENGTH_MAX) {
                  set_rsuTimeSourceLost(
                      (rsuAlertLevel_e)local_rsuhealth.rsuTimeSourceLost
                      ,(uint8_t *)local_rsuhealth.rsuTimeSourceLostMsg
                      ,RSU_ALERT_MSG_LENGTH_MAX);
              } else {
                  set_rsuTimeSourceLost(
                      (rsuAlertLevel_e)local_rsuhealth.rsuTimeSourceLost
                      ,(uint8_t *)local_rsuhealth.rsuTimeSourceLostMsg
                      ,strlen(local_rsuhealth.rsuTimeSourceLostMsg));
              }
              local_rsuhealth.rsuTimeSourceLost = rsuAlertLevel_denso_empty; /* Clear spot */
          }
          /* 5.18.1.7 Time Source Mismatch Message: deviation between two time sources exceeds vendor-defined threshold.*/
          if(local_rsuhealth.rsuTimeSourceMismatch != rsuAlertLevel_denso_empty) {
              if(strlen(local_rsuhealth.rsuTimeSourceMismatchMsg) > RSU_ALERT_MSG_LENGTH_MAX) {
                  set_rsuTimeSourceMismatch(
                      (rsuAlertLevel_e)local_rsuhealth.rsuTimeSourceMismatch
                      ,(uint8_t *)local_rsuhealth.rsuTimeSourceMismatchMsg
                      ,RSU_ALERT_MSG_LENGTH_MAX);
              } else {
                  set_rsuTimeSourceMismatch(
                      (rsuAlertLevel_e)local_rsuhealth.rsuTimeSourceMismatch
                      ,(uint8_t *)local_rsuhealth.rsuTimeSourceMismatchMsg
                      ,strlen(local_rsuhealth.rsuTimeSourceMismatchMsg));
              }
              local_rsuhealth.rsuTimeSourceMismatch = rsuAlertLevel_denso_empty; /* Clear spot */
          }
          /* 5.18.1.8 GNSS Anomaly Message */
          if(local_rsuhealth.rsuGnssAnomaly != rsuAlertLevel_denso_empty) {
              if(strlen(local_rsuhealth.rsuGnssAnomalyMsg) > RSU_ALERT_MSG_LENGTH_MAX) {
                  set_rsuGnssAnomaly(
                      (rsuAlertLevel_e)local_rsuhealth.rsuGnssAnomaly
                      ,(uint8_t *)local_rsuhealth.rsuGnssAnomalyMsg
                      ,RSU_ALERT_MSG_LENGTH_MAX);
              } else {
                  set_rsuGnssAnomaly(
                      (rsuAlertLevel_e)local_rsuhealth.rsuGnssAnomaly
                      ,(uint8_t *)local_rsuhealth.rsuGnssAnomalyMsg
                      ,strlen(local_rsuhealth.rsuGnssAnomalyMsg));
              }
              local_rsuhealth.rsuGnssAnomaly = rsuAlertLevel_denso_empty; /* Clear spot */
          }
          /* 5.18.1.9 GNSS Deviation Error Message: GNSS position deviation greater than allowed. */
          /* NOTE: Done by rsuGnssOutput in MIB, not by RSUDIAG */
          if(local_rsuhealth.rsuGnssDeviationError != rsuAlertLevel_denso_empty) {
              if(strlen(local_rsuhealth.rsuGnssDeviationMsg) > RSU_ALERT_MSG_LENGTH_MAX) {
                  set_rsuGnssDeviation(
                      (rsuAlertLevel_e)local_rsuhealth.rsuGnssDeviationError
                      ,(uint8_t *)local_rsuhealth.rsuGnssDeviationMsg
                      ,RSU_ALERT_MSG_LENGTH_MAX);
              } else {
                  set_rsuGnssDeviation(
                      (rsuAlertLevel_e)local_rsuhealth.rsuGnssDeviationError
                      ,(uint8_t *)local_rsuhealth.rsuGnssDeviationMsg
                      ,strlen(local_rsuhealth.rsuGnssDeviationMsg));
              }
              local_rsuhealth.rsuGnssDeviationError = rsuAlertLevel_denso_empty; /* Clear spot */
          }

          /* 5.18.1.10 is not a trap or is it ? its the GGA string sent to the same port+ip. */
          /* NOTE: Trap Done by rsuGnssOutput in MIB, not by RSUDIAG or here/ */

          /* 5.18.1.11 Certificate Error Message */
          if(local_rsuhealth.rsuCertificateError != rsuAlertLevel_denso_empty) {
#if defined(EXTRA_DEBUG)
              I2V_DBG_LOG(MY_ERR_LEVEL, MY_I2V_NAME,"rsuCertificateError(%d)(%s)\n",
                  local_rsuhealth.rsuCertificateError,local_rsuhealth.rsuCertificateMsg);
#endif
              if(strlen(local_rsuhealth.rsuCertificateMsg) > RSU_ALERT_MSG_LENGTH_MAX) {
                  set_rsuCertificate(
                      (rsuAlertLevel_e)local_rsuhealth.rsuCertificateError
                      ,(uint8_t *)local_rsuhealth.rsuCertificateMsg
                      ,RSU_ALERT_MSG_LENGTH_MAX);
              } else {
                  set_rsuCertificate(
                      (rsuAlertLevel_e)local_rsuhealth.rsuCertificateError
                      ,(uint8_t *)local_rsuhealth.rsuCertificateMsg
                      ,strlen(local_rsuhealth.rsuCertificateMsg));
              }
              local_rsuhealth.rsuCertificateError = rsuAlertLevel_denso_empty; /* Clear spot */
          }
          /* 5.18.1.12 Denial of Service Error Message */
          if(local_rsuhealth.rsuServiceDenialError != rsuAlertLevel_denso_empty) {
              if(strlen(local_rsuhealth.rsuServiceDenialMsg) > RSU_ALERT_MSG_LENGTH_MAX) {
                  set_rsuServiceDenial(
                      (rsuAlertLevel_e)local_rsuhealth.rsuServiceDenialError
                      ,(uint8_t *)local_rsuhealth.rsuServiceDenialMsg
                      ,RSU_ALERT_MSG_LENGTH_MAX);
              } else {
                  set_rsuServiceDenial(
                      (rsuAlertLevel_e)local_rsuhealth.rsuServiceDenialError
                      ,(uint8_t *)local_rsuhealth.rsuServiceDenialMsg
                      ,strlen(local_rsuhealth.rsuServiceDenialMsg));
              }
              local_rsuhealth.rsuServiceDenialError = rsuAlertLevel_denso_empty; /* Clear spot */
          }
          /* 5.18.1.13 Watchdog Error Message */
          if(local_rsuhealth.rsuWatchdogError != rsuAlertLevel_denso_empty) {
              if(strlen(local_rsuhealth.rsuWatchdogMsg) > RSU_ALERT_MSG_LENGTH_MAX) {
                  set_rsuWatchdog(
                      (rsuAlertLevel_e)local_rsuhealth.rsuWatchdogError
                      ,(uint8_t *)local_rsuhealth.rsuWatchdogMsg
                      ,RSU_ALERT_MSG_LENGTH_MAX);
              } else {
                  set_rsuWatchdog(
                      (rsuAlertLevel_e)local_rsuhealth.rsuWatchdogError
                      ,(uint8_t *)local_rsuhealth.rsuWatchdogMsg
                      ,strlen(local_rsuhealth.rsuWatchdogMsg));
              }
              local_rsuhealth.rsuWatchdogError = rsuAlertLevel_denso_empty; /* Clear spot */
          }
          /* 5.18.1.14 Enclosure Environment Message: tamper detection */
          if(local_rsuhealth.rsuEnvironError != rsuAlertLevel_denso_empty) {
              if(strlen(local_rsuhealth.rsuEnvironMsg) > RSU_ALERT_MSG_LENGTH_MAX) {
                  set_rsuEnviron(
                      (rsuAlertLevel_e)local_rsuhealth.rsuEnvironError
                      ,(uint8_t *)local_rsuhealth.rsuEnvironMsg
                      ,RSU_ALERT_MSG_LENGTH_MAX);
              } else {
                  set_rsuEnviron(
                      (rsuAlertLevel_e)local_rsuhealth.rsuEnvironError
                      ,(uint8_t *)local_rsuhealth.rsuEnvironMsg
                      ,strlen(local_rsuhealth.rsuEnvironMsg));
              }
              local_rsuhealth.rsuEnvironError = rsuAlertLevel_denso_empty; /* Clear spot */
          }
          /* Call at ~1hz or interval logic will be wrong. */
          mib_send_notifications(local_rsuhealth.errorstates); /* If alert ready send snmptrap, write updated MIB to disk if needed. */
          half_second_counter++; 
      }
  } /* While */
  if(NULL != shm_rsuhealth_ptr) {
      mib_share_kill(shm_rsuhealth_ptr, sizeof(rsuhealth_t));
      shm_rsuhealth_ptr = NULL;
  }
  I2V_DBG_LOG(MY_ERR_LEVEL, MY_I2V_NAME,"Exit.");
  pthread_exit(NULL);
}
