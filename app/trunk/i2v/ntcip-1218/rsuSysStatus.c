/**************************************************************************
 *                                                                        *
 *     File Name:  rsuSysStatus.c                                         *
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
#if defined(MY_UNIT_TEST)
#include "stubs.h"
#endif

/*
 * Defines.
 */

/* Mandatory: For SNMP debugging and output. */
#define MY_NAME       "SysStatus"
/* Mandatory: For debugging and syslog output. */
#define MY_I2V_NAME   MY_NAME
#define MY_ERR_LEVEL  LEVEL_PRIV    /* from i2v_util.h */

/* Theoritical column ids. Make sure they follow in order with columns and unique. */
#define RSU_CHAN_STATUS_OID               1
#define RSU_MODE_OID                      2
#define RSU_MODE_STATUS_OID               3
#define RSU_REBOOT_OID                    4
#define RSU_CLOCK_SOURCE_OID              5
#define RSU_CLOCK_SOURCE_STATUS_OID       6
#define RSU_CLOCK_SOURCE_TO_OID           7
#define RSU_CLOCK_SOURCE_FAILED_QUERY_OID 8
#define RSU_CLOCK_DEV_TOLERANCE_OID       9
#define RSU_STATUS_OID                    10

#define RSU_CHAN_STATUS_NAME               "rsuChanStatus"
#define RSU_MODE_NAME                      "rsuMode"
#define RSU_MODE_STATUS_NAME               "rsuModeStatus"
#define RSU_REBOOT_NAME                    "rsuReboot"
#define RSU_CLOCK_SOURCE_NAME              "rsuClockSource"
#define RSU_CLOCK_SOURCE_STATUS_NAME       "rsuClockSourceStatus"
#define RSU_CLOCK_SOURCE_TO_NAME           "rsuClockSourceTimeout"
#define RSU_CLOCK_SOURCE_FAILED_QUERY_NAME "rsuClockSourceFailedQuery"
#define RSU_CLOCK_DEV_TOLERANCE_NAME       "rsuClockDeviationTolerance"
#define RSU_STATUS_NAME                    "rsuStatus"

/******************************************************************************
 * 5.17.1 - 5.17.10: rsuSystemStatus : { rsu 16 }
 *
 *    rsuChanStatus_e     rsuChanStatus;
 *    rsuMode_e           rsuMode;
 *    rsuModeStatus_e     rsuModeStatus;
 *    int32_t             rsuReboot;
 *    clockSource_e       rsuClockSource;
 *    clockSourceStatus_e rsuClockSourceStatus;
 *    int32_t             rsuClockSourceTimeout;
 *    int32_t             rsuClockSourceFailedQuery;
 *    int32_t             rsuClockDeviationTolerance;
 *    rsuStatus_e         rsuStatus;
 *
 ******************************************************************************/
const oid rsuChanStatus_oid[]               = {1, 3, 6, 1, 4, 1, 1206, 4, 2, 18, 16 , 1};
const oid rsuMode_oid[]                     = {1, 3, 6, 1, 4, 1, 1206, 4, 2, 18, 16 , 2};
const oid rsuModeStatus_oid[]               = {1, 3, 6, 1, 4, 1, 1206, 4, 2, 18, 16 , 3};
const oid rsuReboot_oid[]                   = {1, 3, 6, 1, 4, 1, 1206, 4, 2, 18, 16 , 4};
const oid rsuClockSource_oid[]              = {1, 3, 6, 1, 4, 1, 1206, 4, 2, 18, 16 , 5};
const oid rsuClockSourceStatus_oid[]        = {1, 3, 6, 1, 4, 1, 1206, 4, 2, 18, 16 , 6};
const oid rsuClockSourceTimeout_oid[]       = {1, 3, 6, 1, 4, 1, 1206, 4, 2, 18, 16 , 7};
const oid rsuClockSourceFailedQuery_oid[]   = {1, 3, 6, 1, 4, 1, 1206, 4, 2, 18, 16 , 8};
const oid rsuClockDeviationTolerance_oid[]  = {1, 3, 6, 1, 4, 1, 1206, 4, 2, 18, 16 , 9};
const oid rsuStatus_oid[]                   = {1, 3, 6, 1, 4, 1, 1206, 4, 2, 18, 16 , 10};

static uint32_t rsuSysStatus_error_states = 0x0;

/* 
 * Function Bodies.
 */
static void set_error_states(int32_t error)
{
  /* -1 to -32 */
  if((error < 0) && (error > MIB_ERROR_BASE_2)){
      rsuSysStatus_error_states |= (uint32_t)(0x1) << (abs(error) - 1);
  }
}
uint32_t get_rsuSysStatus_error_states(void)
{
  return rsuSysStatus_error_states;
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
      case RSU_CHAN_STATUS_OID:
          strcpy(column_name,RSU_CHAN_STATUS_NAME);
          break;
      case RSU_MODE_OID:
          strcpy(column_name,RSU_MODE_NAME);
          break;
      case RSU_MODE_STATUS_OID:
          strcpy(column_name,RSU_MODE_STATUS_NAME);
          break;
      case RSU_REBOOT_OID:
          strcpy(column_name,RSU_REBOOT_NAME);
          break;
      case RSU_CLOCK_SOURCE_OID:
          strcpy(column_name,RSU_CLOCK_SOURCE_NAME);
          break;
      case RSU_CLOCK_SOURCE_STATUS_OID:
          strcpy(column_name,RSU_CLOCK_SOURCE_STATUS_NAME);
          break;
      case RSU_CLOCK_SOURCE_TO_OID:
          strcpy(column_name,RSU_CLOCK_SOURCE_TO_NAME);
          break;
      case RSU_CLOCK_SOURCE_FAILED_QUERY_OID:
          strcpy(column_name,RSU_CLOCK_SOURCE_FAILED_QUERY_NAME);
          break;
      case RSU_CLOCK_DEV_TOLERANCE_OID:
          strcpy(column_name,RSU_CLOCK_DEV_TOLERANCE_NAME);
          break;
      case RSU_STATUS_OID:
          strcpy(column_name,RSU_STATUS_NAME);
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

/* 5.17.1 */
static int32_t handle_rsuChanStatus(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo,
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
              if(0 <= (ret = get_rsuChanStatus())){
                  snmp_set_var_typed_integer(requests->requestvb, ASN_INTEGER, ret);              
                  ret = RSEMIB_OK;
              }
              break;
          default:
              ret = RSEMIB_BAD_MODE;
              break;
      }
      /* scalars and like are always index 0 with unqiue column.*/
      print_request_result(0, RSU_CHAN_STATUS_OID,requests,reqinfo->mode,ret);
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
/* 5.17.2 */
static int32_t handle_rsuMode(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo,
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
              if(0 <= (ret = get_rsuMode ())){
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
              if(RSEMIB_OK == (ret = preserve_rsuMode())){
                  ret = action_rsuMode( *(requests->requestvb->val.integer) );
              }
              break;
          case MODE_SET_COMMIT:
              DEBUGMSGTL((MY_NAME, "COMMIT:\n"));
              if(RSEMIB_OK == (ret = commit_rsuMode())){
                  /*
                   * Back end magic here.
                   */
              }
              break;
          case MODE_SET_UNDO:
              DEBUGMSGTL((MY_NAME, "UNDO:\n"));
              ret = undo_rsuMode();
              break;
          default:
              ret = RSEMIB_BAD_MODE;
              break;
      }
      /* scalars and like are always index 0 with unqiue column.*/
      print_request_result(0, RSU_MODE_OID,requests,reqinfo->mode,ret);
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
/* 5.17.3 */
static int32_t handle_rsuModeStatus(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo,
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
             if(0 <= (ret = get_rsuModeStatus())){
                  snmp_set_var_typed_integer(requests->requestvb, ASN_INTEGER, ret);
                  ret = RSEMIB_OK;
              }
              break;
          default:
              ret = RSEMIB_BAD_MODE;
              break;
      }
      /* scalars and like are always index 0 with unqiue column.*/
      print_request_result(0, RSU_MODE_STATUS_OID,requests,reqinfo->mode,ret);
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
/* 5.17.4 */
static int32_t handle_rsuReboot(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo,
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
              if(0 <= (ret = get_rsuReboot ())){
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
              if(RSEMIB_OK == (ret = preserve_rsuReboot())){
                  ret = action_rsuReboot ( *(requests->requestvb->val.integer) );
              }
              break;
          case MODE_SET_COMMIT:
              DEBUGMSGTL((MY_NAME, "COMMIT:\n"));
              if(RSEMIB_OK == (ret = commit_rsuReboot())){
                  /*
                   * Back end magic here.
                   */
              }
              break;
          case MODE_SET_UNDO:
              DEBUGMSGTL((MY_NAME, "UNDO:\n"));
              ret = undo_rsuReboot();
              break;
          default:
              ret = RSEMIB_BAD_MODE;
              break;
      }
      /* scalars and like are always index 0 with unqiue column.*/
      print_request_result(0, RSU_REBOOT_OID,requests,reqinfo->mode,ret);
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
/* 5.17.5 */
static int32_t handle_rsuClockSource(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo,
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
              if(0 <= (ret = get_rsuClockSource())){
                  snmp_set_var_typed_integer(requests->requestvb, ASN_INTEGER, ret);
                  ret = RSEMIB_OK;
              }
              break;
          default:
              ret = RSEMIB_BAD_MODE;
              break;
      }
      /* scalars and like are always index 0 with unqiue column.*/
      print_request_result(0, RSU_CLOCK_SOURCE_OID,requests,reqinfo->mode,ret);
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
/* 5.17.6 */
static int32_t handle_rsuClockSourceStatus(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo,
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
              if(0 <= (ret = get_rsuClockSourceStatus())){
                  snmp_set_var_typed_integer(requests->requestvb, ASN_INTEGER, ret);
                  ret = RSEMIB_OK;
              }
              break;
          default:
              ret = RSEMIB_BAD_MODE;
              break;
      }
      /* scalars and like are always index 0 with unqiue column.*/
      print_request_result(0, RSU_CLOCK_SOURCE_STATUS_OID,requests,reqinfo->mode,ret);
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
/* 5.17.7 */
static int32_t handle_rsuClockSourceTimeout(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo,
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
              if(0 <= (ret = get_rsuClockSourceTimeout())){
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
              if(RSEMIB_OK == (ret = preserve_rsuClockSourceTimeout())){
                  ret = action_rsuClockSourceTimeout( *(requests->requestvb->val.integer) );
              }
              break;
          case MODE_SET_COMMIT:
              DEBUGMSGTL((MY_NAME, "COMMIT:\n"));
              if(RSEMIB_OK == (ret = commit_rsuClockSourceTimeout())){
                  /*
                   * Back end magic here.
                   */
              }
              break;
          case MODE_SET_UNDO:
              DEBUGMSGTL((MY_NAME, "UNDO:\n"));
              ret = undo_rsuClockSourceTimeout();
              break;
          default:
              ret = RSEMIB_BAD_MODE;
              break;
      }
      /* scalars and like are always index 0 with unqiue column.*/
      print_request_result(0, RSU_CLOCK_SOURCE_TO_OID,requests,reqinfo->mode,ret);
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
/* 5.17.8 */
static int32_t handle_rsuClockSourceFailedQuery(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo,
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
              if(0 <= (ret = get_rsuClockSourceFailedQuery ())){
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
              if(RSEMIB_OK == (ret = preserve_rsuClockSourceFailedQuery())){
                  ret = action_rsuClockSourceFailedQuery( *(requests->requestvb->val.integer) );
              }
              break;
          case MODE_SET_COMMIT:
              DEBUGMSGTL((MY_NAME, "COMMIT:\n"));
              if(RSEMIB_OK == (ret = commit_rsuClockSourceFailedQuery())){
                  /*
                   * Back end magic here.
                   */
              }
              break;
          case MODE_SET_UNDO:
              DEBUGMSGTL((MY_NAME, "UNDO:\n"));
              ret = undo_rsuClockSourceFailedQuery();
              break;
          default:
              ret = RSEMIB_BAD_MODE;
              break;
      }
      /* scalars and like are always index 0 with unqiue column.*/
      print_request_result(0, RSU_CLOCK_SOURCE_FAILED_QUERY_OID,requests,reqinfo->mode,ret);
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
/* 5.17.9 */
static int32_t handle_rsuClockDeviationTolerance(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo,
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
              if(0 <= (ret = get_rsuClockDeviationTolerance ())){
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
              if(RSEMIB_OK == (ret = preserve_rsuClockDeviationTolerance())){
                  ret = action_rsuClockDeviationTolerance ( *(requests->requestvb->val.integer) );
              }
              break;
          case MODE_SET_COMMIT:
              DEBUGMSGTL((MY_NAME, "COMMIT:\n"));
              if(RSEMIB_OK == (ret = commit_rsuClockDeviationTolerance())){
                  /*
                   * Back end magic here.
                   */
              }
              break;
          case MODE_SET_UNDO:
              DEBUGMSGTL((MY_NAME, "UNDO:\n"));
              ret = undo_rsuClockDeviationTolerance();
              break;
          default:
              ret = RSEMIB_BAD_MODE;
              break;
      }
      /* scalars and like are always index 0 with unqiue column.*/
      print_request_result(0, RSU_CLOCK_DEV_TOLERANCE_OID,requests,reqinfo->mode,ret);
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
/* 5.17.10 */
static int32_t handle_rsuStatus(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo,
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
              if(0 <= (ret = get_rsuStatus())){
                  snmp_set_var_typed_integer(requests->requestvb, ASN_INTEGER, ret);
                  ret = RSEMIB_OK;
              }
              break;
          default:
              ret = RSEMIB_BAD_MODE;
              break;
      }
      /* scalars and like are always index 0 with unqiue column.*/
      print_request_result(0, RSU_STATUS_OID,requests,reqinfo->mode,ret);
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

void install_rsuSysStatus_handlers(void)
{
  /* Clear local statics. */
  rsuSysStatus_error_states = 0x0;

  /* 5.17.1 */
  if(MIB_REGISTERED_OK != netsnmp_register_scalar(netsnmp_create_handler_registration
                          ("rsuChanStatus", 
                           handle_rsuChanStatus,
                           rsuChanStatus_oid,
                           OID_LENGTH(rsuChanStatus_oid),
                           HANDLER_CAN_RONLY))){
      DEBUGMSGTL((MY_NAME, "rsuChanStatus install FAIL.\n"));
      set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
  }
  /* 5.17.2 */
  if(MIB_REGISTERED_OK != netsnmp_register_scalar(netsnmp_create_handler_registration
                          ("rsuMode",
                           handle_rsuMode,
                           rsuMode_oid,
                           OID_LENGTH(rsuMode_oid),
                           HANDLER_CAN_RWRITE))){
      DEBUGMSGTL((MY_NAME, "rsuMode install FAIL.\n"));
      set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
  }
  /* 5.17.3 */
  if(MIB_REGISTERED_OK != netsnmp_register_scalar(netsnmp_create_handler_registration
                          ("rsuModeStatus", 
                           handle_rsuModeStatus,
                           rsuModeStatus_oid,
                           OID_LENGTH(rsuModeStatus_oid),
                           HANDLER_CAN_RONLY))){
      DEBUGMSGTL((MY_NAME, "rsuModeStatus install FAIL.\n"));
      set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
  }
  /* 5.17.4 */
  if(MIB_REGISTERED_OK != netsnmp_register_scalar(netsnmp_create_handler_registration
                          ("rsuReboot",
                           handle_rsuReboot,
                           rsuReboot_oid,
                           OID_LENGTH(rsuReboot_oid),
                           HANDLER_CAN_RWRITE))){
      DEBUGMSGTL((MY_NAME, "rsuReboot install FAIL.\n"));
      set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
  }
  /* 5.17.5 */
  if(MIB_REGISTERED_OK != netsnmp_register_scalar(netsnmp_create_handler_registration
                          ("rsuClockSource",
                           handle_rsuClockSource,
                           rsuClockSource_oid,
                           OID_LENGTH(rsuClockSource_oid),
                           HANDLER_CAN_RONLY))){
      DEBUGMSGTL((MY_NAME, "rsuClockSource install FAIL.\n"));
      set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
  }
  /* 5.17.6 */
  if(MIB_REGISTERED_OK != netsnmp_register_scalar(netsnmp_create_handler_registration
                          ("rsuClockSourceStatus",
                           handle_rsuClockSourceStatus,
                           rsuClockSourceStatus_oid,
                           OID_LENGTH(rsuClockSourceStatus_oid),
                           HANDLER_CAN_RONLY))){
      DEBUGMSGTL((MY_NAME, "rsuClockSourceStatus install FAIL.\n"));
      set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
  }
  /* 5.17.7 */
  if(MIB_REGISTERED_OK != netsnmp_register_scalar(netsnmp_create_handler_registration
                          ("rsuClockSourceTimeout",
                           handle_rsuClockSourceTimeout,
                           rsuClockSourceTimeout_oid,
                           OID_LENGTH(rsuClockSourceTimeout_oid),
                           HANDLER_CAN_RWRITE))){
      DEBUGMSGTL((MY_NAME, "rsuClockSourceTimeout install FAIL.\n"));
      set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
  }
  /* 5.17.8 */
  if(MIB_REGISTERED_OK != netsnmp_register_scalar(netsnmp_create_handler_registration
                          ("rsuClockSourceFailedQuery",
                           handle_rsuClockSourceFailedQuery,
                           rsuClockSourceFailedQuery_oid,
                           OID_LENGTH(rsuClockSourceFailedQuery_oid),
                           HANDLER_CAN_RWRITE))){
      DEBUGMSGTL((MY_NAME, "rsuClockSourceFailedQuery install FAIL.\n"));
      set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
  }
  /* 5.17.9 */
  if(MIB_REGISTERED_OK != netsnmp_register_scalar(netsnmp_create_handler_registration
                          ("rsuClockDeviationTolerance",
                           handle_rsuClockDeviationTolerance,
                           rsuClockDeviationTolerance_oid,
                           OID_LENGTH(rsuClockDeviationTolerance_oid),
                           HANDLER_CAN_RWRITE))){
      DEBUGMSGTL((MY_NAME, "rsuClockDeviationTolerance install FAIL.\n"));
      set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
  }
  /* 5.17.10 */
  if(MIB_REGISTERED_OK != netsnmp_register_scalar(netsnmp_create_handler_registration
                          ("rsuStatus",
                           handle_rsuStatus,
                           rsuStatus_oid,
                           OID_LENGTH(rsuStatus_oid),
                           HANDLER_CAN_RONLY))){
      DEBUGMSGTL((MY_NAME, "rsuStatus install FAIL.\n"));
      set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
  }
}
void destroy_rsuSysStatus(void)
{
}

