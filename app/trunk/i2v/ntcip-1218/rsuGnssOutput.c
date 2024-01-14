/**************************************************************************
 *                                                                        *
 *     File Name:  rsuGnssOutput.c                                        *
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
#include <sys/stat.h>
#include <pthread.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <errno.h>
#include <math.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "rsutable.h"
#include "ntcip-1218.h"
#include "shm_tps.h"
#include "i2v_util.h"
#include "rsuhealth.h"
#if defined(MY_UNIT_TEST)
#include "stubs.h"
#endif

/*
 * Defines.
 */

/* Mandatory: For SNMP debugging and output. */
#define MY_NAME       "GnssOutput"
#define MY_NAME_EXTRA "XGnssOutput"
/* Mandatory: For debugging and syslog output. */
#define MY_I2V_NAME    "GnssOut"
#define MY_ERR_LEVEL   LEVEL_PRIV    /* from i2v_util.h */

/* rsuGnssOutput.c specific. */
#define GNSS_OUTPUT_FWD_WRITE_FAIL     -25
#define GNSS_OUTPUT_FWD_CONNECT_FAIL   -26
#define GNSS_OUTPUT_FWD_INVALID_IP     -27
#define GNSS_OUTPUT_FWD_SOCK_FAIL      -28
#define GNSS_OUTPUT_SHM_INIT_FAILED    -29
#define GNSS_OUTPUT_BAD_GNSS_DATA      -30
#define GNSS_OUTPUT_BAD_GNSS_ERR_DATA  -31
#define GNSS_OUTPUT_INTERVAL_ROLL      -32

/* Theoritical column ids. Make sure they follow in order with columns and unique. */
#define GNSS_OUTPUT_PORT_OID        1
#define GNSS_OUTPUT_ADDR_OID        2
#define GNSS_OUTPUT_INTERFACE_OID   3
#define GNSS_OUTPUT_INTERVAL_OID    4
#define GNSS_OUTPUT_STRING_OID      5
#define GNSS_OUTPUT_LAT_OID         6
#define GNSS_OUTPUT_LON_OID         7
#define GNSS_OUTPUT_ELV_OID         8
#define GNSS_OUTPUT_MAX_DEV_OID     9
#define GNSS_OUTPUT_LOC_DEV_OID     10
#define GNSS_OUTPUT_POS_ERROR_OID   11

#define GNSS_OUTPUT_PORT_NAME        "rsuGnssOutputPort"
#define GNSS_OUTPUT_ADDR_NAME        "rsuGnssOutputAddress"
#define GNSS_OUTPUT_INTERFACE_NAME   "rsuGnssOutputInterface"
#define GNSS_OUTPUT_INTERVAL_NAME    "rsuGnssOutputInterval"
#define GNSS_OUTPUT_STRING_NAME      "rsuGnssOutputString"
#define GNSS_OUTPUT_LAT_NAME         "rsuGnssLat"
#define GNSS_OUTPUT_LON_NAME         "rsuGnssLon"
#define GNSS_OUTPUT_ELV_NAME         "rsuGnssElv"
#define GNSS_OUTPUT_MAX_DEV_NAME     "rsuGnssMaxDeviation"
#define GNSS_OUTPUT_LOC_DEV_NAME     "rsuLocationDeviation"
#define GNSS_OUTPUT_POS_ERROR_NAME   "rsuGnssPositionError"

/****************************************************************************** 
 * 5.7.1 - 5.7.11 GNSS Output: { rsu 6 }
 *
 *  int32_t rsuGnssOutputPort;
 *  uint8_t rsuGnssOutputAddress[RSU_DEST_IP_MAX];
 *  uint8_t rsuGnssOutputInterface[RSU_GNSS_OUTPUT_INTERFACE_MAX];
 *  int32_t rsuGnssOutputInterval;
 *  uint8_t rsuGnssOutputString[RSU_GNSS_OUTPUT_STRING_MAX];
 *  int32_t rsuGnssLat;
 *  int32_t rsuGnssLon;
 *  int32_t rsuGnssElv;
 *  int32_t rsuGnssMaxDeviation;
 *  int32_t rsuLocationDeviation;
 *  int32_t rsuGnssPositionError;
 ******************************************************************************/

const oid rsuGnssOutputPort_oid[]      = {1, 3, 6, 1, 4, 1, 1206, 4, 2, 18, 6 , 1};
const oid rsuGnssOutputAddress_oid[]   = {1, 3, 6, 1, 4, 1, 1206, 4, 2, 18, 6 , 2};
const oid rsuGnssOutputInterface_oid[] = {1, 3, 6, 1, 4, 1, 1206, 4, 2, 18, 6 , 3};
const oid rsuGnssOutputInterval_oid[]  = {1, 3, 6, 1, 4, 1, 1206, 4, 2, 18, 6 , 4};
const oid rsuGnssOutputString_oid[]    = {1, 3, 6, 1, 4, 1, 1206, 4, 2, 18, 6 , 5};
const oid rsuGnssLat_oid[]             = {1, 3, 6, 1, 4, 1, 1206, 4, 2, 18, 6 , 6};
const oid rsuGnssLon_oid[]             = {1, 3, 6, 1, 4, 1, 1206, 4, 2, 18, 6 , 7};
const oid rsuGnssElv_oid[]             = {1, 3, 6, 1, 4, 1, 1206, 4, 2, 18, 6 , 8};
const oid rsuGnssMaxDeviation_oid[]    = {1, 3, 6, 1, 4, 1, 1206, 4, 2, 18, 6 , 9};
const oid rsuLocationDeviation_oid[]   = {1, 3, 6, 1, 4, 1, 1206, 4, 2, 18, 6 , 10};
const oid rsuGnssPositionError_oid[]   = {1, 3, 6, 1, 4, 1, 1206, 4, 2, 18, 6 , 11};

extern int32_t  mainloop; /* For thread handling. */
extern ntcip_1218_mib_t  ntcip_1218_mib;

static uint32_t rsuGnssOutput_error_states = 0x0;
/* For thread update_gnssOutput. */
static uint32_t update_gnssOutput_counter = 0; /* Rolling counter. */
static int32_t  current_interval_count = 1; /* The first interval is now. */
static int32_t  gnssFwdSock = -1;
static int32_t  gnssOutputPort = 0;
static uint8_t  gnssOutputAddress[RSU_DEST_IP_MAX];
static int32_t  gnssOutputInterval = 0;

static shm_tps_t * shm_tps_ptr = NULL;
static rsuhealth_t * shm_rsuhealth_ptr = NULL;
static uint32_t      local_gnssstatus = 0x0;       /* bitmask */
/* 
 * Function Bodies.
 */
static void set_error_states(int32_t error)
{
  /* -1 to -32 */
  if((error < 0) && (error > MIB_ERROR_BASE_2)){
      rsuGnssOutput_error_states |= (uint32_t)(0x1) << (abs(error) - 1);
  }
}
uint32_t get_rsuGnssOutput_error_states(void)
{
  return rsuGnssOutput_error_states;
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
      case GNSS_OUTPUT_PORT_OID:
          strcpy(column_name,GNSS_OUTPUT_PORT_NAME);
          break;
      case GNSS_OUTPUT_ADDR_OID:
          strcpy(column_name,GNSS_OUTPUT_ADDR_NAME);
          break;
      case GNSS_OUTPUT_INTERFACE_OID:
          strcpy(column_name,GNSS_OUTPUT_INTERFACE_NAME);
          break;
      case GNSS_OUTPUT_INTERVAL_OID:
          strcpy(column_name,GNSS_OUTPUT_INTERVAL_NAME);
          break;
      case GNSS_OUTPUT_STRING_OID:
          strcpy(column_name,GNSS_OUTPUT_STRING_NAME);
          break;
      case GNSS_OUTPUT_LAT_OID:
          strcpy(column_name,GNSS_OUTPUT_LAT_NAME);
          break;
      case GNSS_OUTPUT_LON_OID:
          strcpy(column_name,GNSS_OUTPUT_LON_NAME);
          break;
      case GNSS_OUTPUT_ELV_OID:
          strcpy(column_name,GNSS_OUTPUT_ELV_NAME);
          break;
      case GNSS_OUTPUT_MAX_DEV_OID:
          strcpy(column_name,GNSS_OUTPUT_MAX_DEV_NAME);
          break;
      case GNSS_OUTPUT_LOC_DEV_OID:
          strcpy(column_name,GNSS_OUTPUT_LOC_DEV_NAME);
          break;
      case GNSS_OUTPUT_POS_ERROR_OID:
          strcpy(column_name,GNSS_OUTPUT_POS_ERROR_NAME);
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

/* 5.7.1 */
static int32_t handle_rsuGnssOutputPort(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo,
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
              if(0 <= (ret = get_rsuGnssOutputPort())){
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
              if(RSEMIB_OK == (ret = preserve_rsuGnssOutputPort())){
                  ret = action_rsuGnssOutputPort( *(requests->requestvb->val.integer));
              }
              break;
          case MODE_SET_COMMIT:
              DEBUGMSGTL((MY_NAME, "COMMIT:\n"));
              if(RSEMIB_OK == (ret = commit_rsuGnssOutputPort())){
                  /*
                   * Back end magic here.
                   */
              }
              break;
          case MODE_SET_UNDO:
              DEBUGMSGTL((MY_NAME, "UNDO:\n"));
              ret = undo_rsuGnssOutputPort();
              break;
          default:
              ret = RSEMIB_BAD_MODE;
              break;
      }
      /* scalars and like are always index 0 with unqiue column.*/
      print_request_result(0, GNSS_OUTPUT_PORT_OID,requests,reqinfo->mode,ret);
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
/* 5.7.2 */
static int32_t handle_rsuGnssOutputAddress(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo,
                                     netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret = RSEMIB_OK;
  uint8_t data[RSU_DEST_IP_MAX];

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
              if(0 <= (ret = get_rsuGnssOutputAddress(data))){
                  snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR, data, ret);
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
              if(RSEMIB_OK == (ret = preserve_rsuGnssOutputAddress())){
                  ret = action_rsuGnssOutputAddress(requests->requestvb->val.string, requests->requestvb->val_len);
              }
              break;
          case MODE_SET_COMMIT:
              DEBUGMSGTL((MY_NAME, "COMMIT:\n"));
              if(RSEMIB_OK == (ret = commit_rsuGnssOutputAddress())){
                  /*
                   * Back end magic here.
                   */
              }
              break;
          case MODE_SET_UNDO:
              DEBUGMSGTL((MY_NAME, "UNDO:\n"));
              ret = undo_rsuGnssOutputAddress();
              break;
          default:
              ret = RSEMIB_BAD_MODE;
              break;
      }
      /* scalars and like are always index 0 with unqiue column.*/
      print_request_result(0, GNSS_OUTPUT_ADDR_OID,requests,reqinfo->mode,ret);
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
/* 5.7.3 */
static int32_t handle_rsuGnssOutputInterface(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo,
                                 netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret = RSEMIB_OK;
  uint8_t data[RSU_GNSS_OUTPUT_INTERFACE_MAX];

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
              if(0 <= (ret = get_rsuGnssOutputInterface(data))){
                  snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR, data, ret);
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
              if(RSEMIB_OK == (ret = preserve_rsuGnssOutputInterface())){
                  ret = action_rsuGnssOutputInterface(requests->requestvb->val.string, requests->requestvb->val_len);
              }
              break;
          case MODE_SET_COMMIT:
              DEBUGMSGTL((MY_NAME, "COMMIT:\n"));
              if(RSEMIB_OK == (ret = commit_rsuGnssOutputInterface())){
                  /*
                   * Back end magic here.
                   */
              }
              break;
          case MODE_SET_UNDO:
              DEBUGMSGTL((MY_NAME, "UNDO:\n"));
              ret = undo_rsuGnssOutputInterface();
              break;
          default:
              ret = RSEMIB_BAD_MODE;
              break;
      }
      /* scalars and like are always index 0 with unqiue column.*/
      print_request_result(0, GNSS_OUTPUT_INTERFACE_OID,requests,reqinfo->mode,ret);
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
/* 5.7.4 */
static int32_t handle_rsuGnssOutputInterval(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo,
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
              if(0 <= (ret = get_rsuGnssOutputInterval())){
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
              if(RSEMIB_OK == (ret = preserve_rsuGnssOutputInterval())){
                  ret = action_rsuGnssOutputInterval( *(requests->requestvb->val.integer));
              }
              break;
          case MODE_SET_COMMIT:
              DEBUGMSGTL((MY_NAME, "COMMIT:\n"));
              if(RSEMIB_OK == (ret = commit_rsuGnssOutputInterval())){
                  /*
                   * Back end magic here.
                   */
              }
              break;
          case MODE_SET_UNDO:
              DEBUGMSGTL((MY_NAME, "UNDO:\n"));
              ret = undo_rsuGnssOutputInterval();
              break;
          default:
              ret = RSEMIB_BAD_MODE;
              break;
      }
      /* scalars and like are always index 0 with unqiue column.*/
      print_request_result(0, GNSS_OUTPUT_INTERVAL_OID,requests,reqinfo->mode,ret);
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
/* 5.7.5 : ro: GGA */
static int32_t handle_rsuGnssOutputString(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo,
                                 netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret = RSEMIB_OK;
  uint8_t data[RSU_GNSS_OUTPUT_STRING_MAX];

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
              if(0 <= (ret = get_rsuGnssOutputString(data))){
                  snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR, data, ret);
                  DEBUGMSGTL((MY_NAME, "SUCCESS: size=%d[%s]:\n",ret,data));
                  ret = RSEMIB_OK;
              }
              break;
          default:
              ret = RSEMIB_BAD_MODE;
              break;
      }
      /* scalars and like are always index 0 with unqiue column.*/
      print_request_result(0, GNSS_OUTPUT_STRING_OID,requests,reqinfo->mode,ret);
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
/* 5.7.6 : ro: degrees scaled to 10^7. */
static int32_t handle_rsuGnssLat(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo,
                                 netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret  = RSEMIB_OK;
  int32_t data = 0;

  handler = handler; /* Silence of the Warnings. */
  reginfo = reginfo;
  DEBUGMSGTL((MY_NAME, "%s: Entry:\n",__FUNCTION__));
  if((NULL == reqinfo) || (NULL == requests)) {
      ret = RSEMIB_BAD_INPUT;
  } else {
      switch (reqinfo->mode){
          case MODE_GET:
              DEBUGMSGTL((MY_NAME, "GET:\n"));
              if(RSEMIB_OK == (ret = get_rsuGnssLat(&data))){
                  snmp_set_var_typed_integer(requests->requestvb, ASN_INTEGER, data);
              }
              break;
          default:
              ret = RSEMIB_BAD_MODE;
              break;
      }
      /* scalars and like are always index 0 with unqiue column.*/
      print_request_result(0, GNSS_OUTPUT_LAT_OID,requests,reqinfo->mode,ret);
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
/* 5.7.7 : ro: degrees scaled to 10^7. */
static int32_t handle_rsuGnssLon(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo,
                                 netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret  = RSEMIB_OK;
  int32_t data = 0;

  handler = handler; /* Silence of the Warnings. */
  reginfo = reginfo;
  DEBUGMSGTL((MY_NAME, "%s: Entry:\n",__FUNCTION__));
  if((NULL == reqinfo) || (NULL == requests)) {
      ret = RSEMIB_BAD_INPUT;
  } else {
      switch (reqinfo->mode){
          case MODE_GET:
              DEBUGMSGTL((MY_NAME, "GET:\n"));
              if(RSEMIB_OK == (ret = get_rsuGnssLon(&data))){
                  snmp_set_var_typed_integer(requests->requestvb, ASN_INTEGER, data);
              }
              break;
          default:
              ret = RSEMIB_BAD_MODE;
              break;
      }
      /* scalars and like are always index 0 with unqiue column.*/
      print_request_result(0, GNSS_OUTPUT_LON_OID,requests,reqinfo->mode,ret);
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
/* 5.7.8 from elipsoid in cm. */
static int32_t handle_rsuGnssElv(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo,
                                 netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret  = RSEMIB_OK;
  int32_t data = 0;

  handler = handler; /* Silence of the Warnings. */
  reginfo = reginfo;
  DEBUGMSGTL((MY_NAME, "%s: Entry:\n",__FUNCTION__));
  if((NULL == reqinfo) || (NULL == requests)) {
      ret = RSEMIB_BAD_INPUT;
  } else {
      switch (reqinfo->mode){
          case MODE_GET:
              DEBUGMSGTL((MY_NAME, "GET:\n"));
              if(RSEMIB_OK == (ret = get_rsuGnssElv(&data))){
                  snmp_set_var_typed_integer(requests->requestvb, ASN_INTEGER, data);
              }
              break;
          default:
              ret = RSEMIB_BAD_MODE;
              break;
      }
      /* scalars and like are always index 0 with unqiue column.*/
      print_request_result(0, GNSS_OUTPUT_ELV_OID,requests,reqinfo->mode,ret);
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
/* 5.7.9 : rw: metre */
static int32_t handle_rsuGnssMaxDeviation(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo,
                                 netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret = RSEMIB_OK;
  int32_t data = 0;

  handler = handler; /* Silence of the Warnings. */
  reginfo = reginfo;
  DEBUGMSGTL((MY_NAME, "%s: Entry:\n",__FUNCTION__));
  if((NULL == reqinfo) || (NULL == requests)) {
      ret = RSEMIB_BAD_INPUT;
  } else {
      switch (reqinfo->mode){
          case MODE_GET:
              DEBUGMSGTL((MY_NAME, "GET:\n"));
              if(RSEMIB_OK == (ret = get_rsuGnssMaxDeviation(&data))){
                  snmp_set_var_typed_integer(requests->requestvb, ASN_INTEGER, data);
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
              if(RSEMIB_OK == (ret = preserve_rsuGnssMaxDeviation())){
                  ret = action_rsuGnssMaxDeviation( *(requests->requestvb->val.integer));
              }
              break;
          case MODE_SET_COMMIT:
              DEBUGMSGTL((MY_NAME, "COMMIT:\n"));
              if(RSEMIB_OK == (ret = commit_rsuGnssMaxDeviation())){
                  /*
                   * Back end magic here.
                   */
              }
              break;
          case MODE_SET_UNDO:
              DEBUGMSGTL((MY_NAME, "UNDO:\n"));
              ret = undo_rsuGnssMaxDeviation();
              break;
          default:
              ret = RSEMIB_BAD_MODE;
              break;
      }
      /* scalars and like are always index 0 with unqiue column.*/
      print_request_result(0, GNSS_OUTPUT_MAX_DEV_OID,requests,reqinfo->mode,ret);
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
/* 5.7.10 metre */
static int32_t handle_rsuLocationDeviation(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo,
                                 netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret  = RSEMIB_OK;
  int32_t data = 0;

  handler = handler; /* Silence of the Warnings. */
  reginfo = reginfo;
  DEBUGMSGTL((MY_NAME, "handle_rsuLocationDeviation: Entry:\n"));
  if((NULL == reqinfo) || (NULL == requests)) {
      ret = RSEMIB_BAD_INPUT;
  } else {
      switch (reqinfo->mode){
          case MODE_GET:
              DEBUGMSGTL((MY_NAME, "GET:\n"));
              if(RSEMIB_OK == (ret = get_rsuLocationDeviation(&data))){
                  snmp_set_var_typed_integer(requests->requestvb, ASN_INTEGER, data);
              }
              break;
          default:
              ret = RSEMIB_BAD_MODE;
              break;
      }
      /* scalars and like are always index 0 with unqiue column.*/
      print_request_result(0, GNSS_OUTPUT_LOC_DEV_OID,requests,reqinfo->mode,ret);
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
/* 5.7.11 : decimetre */
static int32_t handle_rsuGnssPositionError(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo,
                                 netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret  = RSEMIB_OK;
  int32_t data = 0;

  handler = handler; /* Silence of the Warnings. */
  reginfo = reginfo;
  DEBUGMSGTL((MY_NAME, "%s: Entry:\n",__FUNCTION__));
  if((NULL == reqinfo) || (NULL == requests)) {
      ret = RSEMIB_BAD_INPUT;
  } else {
      switch (reqinfo->mode){
          case MODE_GET:
              DEBUGMSGTL((MY_NAME, "GET:\n"));
              if(RSEMIB_OK == (ret = get_rsuGnssPositionError(&data))){
                  snmp_set_var_typed_integer(requests->requestvb, ASN_INTEGER, data);
              }
              break;
          default:
              ret = RSEMIB_BAD_MODE;
              break;
      }
      /* scalars and like are always index 0 with unqiue column.*/
      print_request_result(0, GNSS_OUTPUT_POS_ERROR_OID,requests,reqinfo->mode,ret);
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
void install_rsuGnssOutput_handlers(void)
{
  /* Clear error states. */
  rsuGnssOutput_error_states = 0x0;

  /* 5.7.1 */
  if(MIB_REGISTERED_OK != netsnmp_register_scalar(netsnmp_create_handler_registration
                          ("rsuGnssOutputPort", 
                           handle_rsuGnssOutputPort,
                           rsuGnssOutputPort_oid,
                           OID_LENGTH(rsuGnssOutputPort_oid),
                           HANDLER_CAN_RWRITE))){
      DEBUGMSGTL((MY_NAME, "rsuGnssOutputPort install FAIL.\n"));
      set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
  }
  /* 5.7.2 */
  if(MIB_REGISTERED_OK != netsnmp_register_scalar(netsnmp_create_handler_registration
                          ("rsuGnssOutputAddress",
                           handle_rsuGnssOutputAddress,
                           rsuGnssOutputAddress_oid,
                           OID_LENGTH(rsuGnssOutputAddress_oid),
                           HANDLER_CAN_RWRITE))){
      DEBUGMSGTL((MY_NAME, "rsuGnssOutputPort install FAIL.\n"));
      set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
  }
  /* 5.7.3 */
  if(MIB_REGISTERED_OK != netsnmp_register_scalar(netsnmp_create_handler_registration
                          ("rsuGnssOutputInterface",
                           handle_rsuGnssOutputInterface,
                           rsuGnssOutputInterface_oid,
                           OID_LENGTH(rsuGnssOutputInterface_oid),
                           HANDLER_CAN_RWRITE))){
      DEBUGMSGTL((MY_NAME, "rsuGnssOutputInterface install FAIL.\n"));
      set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
  }
  /* 5.7.4 */
  if(MIB_REGISTERED_OK != netsnmp_register_scalar(netsnmp_create_handler_registration
                          ("rsuGnssOutputInterval",
                           handle_rsuGnssOutputInterval,
                           rsuGnssOutputInterval_oid,
                           OID_LENGTH(rsuGnssOutputInterval_oid),
                           HANDLER_CAN_RWRITE))){
      DEBUGMSGTL((MY_NAME, "rsuGnssOutputInterval install FAIL.\n"));
      set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
  }
  /* 5.7.5 */
  if(MIB_REGISTERED_OK != netsnmp_register_scalar(netsnmp_create_handler_registration
                          ("rsuGnssOutputString",
                           handle_rsuGnssOutputString,
                           rsuGnssOutputString_oid,
                           OID_LENGTH(rsuGnssOutputString_oid),
                           HANDLER_CAN_RONLY))){
      DEBUGMSGTL((MY_NAME, "rsuGnssOutputString install FAIL.\n"));
      set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
  }
  /* 5.7.6 */
  if(MIB_REGISTERED_OK != netsnmp_register_scalar(netsnmp_create_handler_registration
                          ("rsuGnssLat",
                           handle_rsuGnssLat,
                           rsuGnssLat_oid,
                           OID_LENGTH(rsuGnssLat_oid),
                           HANDLER_CAN_RONLY))){
      DEBUGMSGTL((MY_NAME, "rsuGnssLat install FAIL.\n"));
      set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
  }
  /* 5.7.7 */
  if(MIB_REGISTERED_OK != netsnmp_register_scalar(netsnmp_create_handler_registration
                          ("rsuGnssLon",
                           handle_rsuGnssLon,
                           rsuGnssLon_oid,
                           OID_LENGTH(rsuGnssLon_oid),
                           HANDLER_CAN_RONLY))){
      DEBUGMSGTL((MY_NAME, "rsuGnssLon install FAIL.\n"));
      set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
  }
  /* 5.7.8 */
  if(MIB_REGISTERED_OK != netsnmp_register_scalar(netsnmp_create_handler_registration
                          ("rsuGnssElv",
                           handle_rsuGnssElv,
                           rsuGnssElv_oid,
                           OID_LENGTH(rsuGnssElv_oid),
                           HANDLER_CAN_RONLY))){
      DEBUGMSGTL((MY_NAME, "rsuGnssElv install FAIL.\n"));
      set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
  }
  /* 5.7.9 */
  if(MIB_REGISTERED_OK != netsnmp_register_scalar(netsnmp_create_handler_registration
                          ("rsuGnssMaxDeviation",
                           handle_rsuGnssMaxDeviation,
                           rsuGnssMaxDeviation_oid,
                           OID_LENGTH(rsuGnssMaxDeviation_oid),
                           HANDLER_CAN_RWRITE))){
      DEBUGMSGTL((MY_NAME, "rsuGnssMaxDeviation install FAIL.\n"));
      set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
  }
  /* 5.7.10 */
  if(MIB_REGISTERED_OK != netsnmp_register_scalar(netsnmp_create_handler_registration
                          ("rsuLocationDeviation",
                           handle_rsuLocationDeviation,
                           rsuLocationDeviation_oid,
                           OID_LENGTH(rsuLocationDeviation_oid),
                           HANDLER_CAN_RONLY))){
      DEBUGMSGTL((MY_NAME, "rsuLocationDeviation install FAIL.\n"));
      set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
  }
  /* 5.7.11 */
  if(MIB_REGISTERED_OK != netsnmp_register_scalar(netsnmp_create_handler_registration
                          ("rsuGnssPositionError",
                           handle_rsuGnssPositionError,
                           rsuGnssPositionError_oid,
                           OID_LENGTH(rsuGnssPositionError_oid),
                           HANDLER_CAN_RONLY))){
      DEBUGMSGTL((MY_NAME, "rsuGnssPositionErrorinstall FAIL.\n"));
      set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
  }
}
void destroy_rsuGnssOutput(void)
{
  /* Nothing to destroy for scalar helper registration...seemingly. */
  if(-1 != gnssFwdSock)
      close(gnssFwdSock);
}

/* Thread Support. */
#define DEG_TO_RAD(a)       ((a)*M_PI/180)
#define UTIL_EARTH_MEAN_RADIUS_M     (6371.0 * 1000.0)
/*
 *******************************************************************************
 *
 *  Function :  UtilCalcRangeHaversine
 *
 *  Purpose  :  Calculates range from a reference point (ref) to another 
 *                  point (p) using the haversine algorithm
 *
 *  Argument :  ref_lat_deg       Reference latitude in degrees
 *              ref_long_deg      Reference longtitude in degrees
 *              ref_heading_deg   Heading in degrees (unused)
 *              p_lat_deg         Point p latitude in degrees
 *              p_long_deg        Point p longtitude in degrees
 *                
 *  Output   :  None
 *
 *  Return   :  Range in meters
 *
 *  Notes    :  haversine is well conditioned for numerical computation.
 *
 *******************************************************************************
 */
float64_t UtilCalcRangeHaversine(float64_t ref_lat_deg, float64_t ref_long_deg, 
        float64_t ref_heading_deg, float64_t p_lat_deg, float64_t p_lon_deg )
{
    float64_t refLatRad;
    float64_t pLatRad;
    float64_t dLatRad;
    float64_t dLongRad;
    float64_t halfChordSqrdRad;
    float64_t angularDistanceRad;
    float64_t rangeM = 0.0;

    ref_heading_deg = ref_heading_deg; /* By definition, not used. */

    dLatRad = DEG_TO_RAD((p_lat_deg - ref_lat_deg));
    dLongRad = DEG_TO_RAD((p_lon_deg - ref_long_deg));
    refLatRad = DEG_TO_RAD(ref_lat_deg);
    pLatRad = DEG_TO_RAD(p_lat_deg);

    halfChordSqrdRad = ( sin(dLatRad / 2.0) * sin(dLatRad / 2.0)) +
        (( sin(dLongRad / 2.0) * sin(dLongRad / 2.0) * 
        ( cos(refLatRad) * cos(pLatRad))));

    angularDistanceRad = 2 * atan2(sqrt(halfChordSqrdRad), sqrt(1 - halfChordSqrdRad));

    rangeM = angularDistanceRad * UTIL_EARTH_MEAN_RADIUS_M;

    return rangeM;
}
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
int32_t commit_gnssOutput_to_disk(GnssOutput_t * gnssOutput) 
{
  FILE    * file_out       = NULL;
  uint32_t  mib_byte_count = 0;
  uint32_t  write_count    = 0;
  int32_t   ret            = RSEMIB_OK;

  if(NULL == gnssOutput){
      ret = RSEMIB_BAD_INPUT;
  } else {
      DEBUGMSGTL((MY_NAME, "commit_to_disk:fopen(%s).\n",GNSS_OUTPUT_PATHFILENAME));
      if ((file_out = fopen(GNSS_OUTPUT_PATHFILENAME, "wb")) == NULL){
          DEBUGMSGTL((MY_NAME, "commit_to_disk: fopen fail.\n"));
          ret = RSEMIB_FOPEN_FAIL;
      } else {
          gnssOutput->crc = (uint32_t)crc16((const uint8_t *)gnssOutput, sizeof(GnssOutput_t) - 4);
          mib_byte_count = sizeof(GnssOutput_t);
          write_count = fwrite((uint8_t *)gnssOutput, mib_byte_count, 1, file_out);
          if (write_count == 1){
              DEBUGMSGTL((MY_NAME, "commit_to_disk: SUCCESS: mib_byte_count=(%u:0x%x).\n",mib_byte_count,gnssOutput->crc));
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
  return ret;
}

#define  MAX_LOCK_WAIT_ITERATIONS 10
#define  LOCK_WAIT_TIME 10000  /* 10 msec */
static int32_t GET_LOCK(void) 
{
  int32_t i;  
  for (i=0; i<MAX_LOCK_WAIT_ITERATIONS; i++) {
      if(RSEMIB_OK == ntcip1218_lock()){
          return RSEMIB_OK;
      } else {
          usleep(LOCK_WAIT_TIME);
      }
  }
  DEBUGMSGTL((MY_NAME, " GET_LOCK failed, waited %d msec.\n",i * LOCK_WAIT_TIME));
  return RSEMIB_LOCK_FAIL;
}
/* Shared with IMF so need to lock. */
#define MAX_TRY_LOCK 10
void sendgnssnotification(char_t * msg, int32_t level)
{
  int32_t ret = RSEMIB_OK;
  char_t  cmd[1024]; /* OIDs can be huge, size accordingly */

  if(NULL == msg) {
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"sendgnssnotification: NULL input.\n");
      set_error_states(RSEMIB_BAD_INPUT);
  } else {
      if((level < rsuAlertLevel_info) || (rsuAlertLevel_critical < level)){
          I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"sendgnssnotification: bad level(%d)\n", level);
          level = rsuAlertLevel_info;
          set_error_states(RSEMIB_BAD_DATA);
      }
      if(RSUHEALTH_ALERT_MSG_LENGTH_MAX < strnlen(msg,RSUHEALTH_ALERT_MSG_LENGTH_MAX)) {
          msg[RSUHEALTH_ALERT_MSG_LENGTH_MAX-1] = '\0';
          set_error_states(RSEMIB_BAD_DATA);
      }
      memset(cmd,0x0,sizeof(cmd));

      #if defined(ENABLE_SMNP_V3_SHA)
      ret = snprintf(cmd,sizeof(cmd),"snmptrap -v 3 -u rsutweiadmin -l authPriv -a SHA -A rsutweiauthpass -x AES-256 -X rsutweiprivpass -mNTCIP1218-v01 udp:%s:%d '' rsuGnssNmeaNotify.0 rsuAlertLevel i %d  rsuGnssOutputString.0 s \"%s\"",
      #else
      ret = snprintf(cmd,sizeof(cmd),"snmptrap -v 3 -u rsutweiadmin -l authPriv -a SHA-512 -A rsutweiauthpass -x AES-256 -X rsutweiprivpass -mNTCIP1218-v01 udp:%s:%d '' rsuGnssNmeaNotify.0 rsuAlertLevel i %d  rsuGnssOutputString.0 s \"%s\"",
      #endif
            ntcip_1218_mib.rsuNotifyIpAddress,ntcip_1218_mib.rsuNotifyPort,level,msg);

#if defined(EXTRA_DEBUG)
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"sendgnssnotification: msg(%s)\n",msg);
#endif
      if(0 < ret) {
          if(system(cmd)) {
              I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"sendgnssnotification: failed: errno(%s)\n",strerror(errno));
              set_error_states(RSEMIB_SYSCALL_FAIL);
          } 
#if defined(EXTRA_DEBUG)
          else { 
              I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"sendgnssnotification: success.\n");
         }
#endif
      }
  }
}

void sendgnssdeviationnotification(char_t * msg, int32_t level)
{
  int32_t  ret = RSEMIB_OK;
  int32_t  i = 0;
  char_t   rsuGnssDeviationMsg[RSUHEALTH_ALERT_MSG_LENGTH_MAX];

  /* Send notification to ntcip-1218 who will send trap for us. */
  if(NULL != shm_rsuhealth_ptr) {

      /* Clear variables and statics. */
      memset(rsuGnssDeviationMsg,0x0,sizeof(rsuGnssDeviationMsg));

      /* send to syslog first. */
      if(NULL == msg) { /* zero length msg is legal in ntcip-1218 MIB.  */
          ret = RSEMIB_BAD_INPUT;
      } else {
          if(RSUHEALTH_ALERT_MSG_LENGTH_MAX < strlen(msg)) {          
              strncpy(rsuGnssDeviationMsg,msg,sizeof(rsuGnssDeviationMsg));
          } else {
              strncpy(rsuGnssDeviationMsg,msg,strlen(msg));
          }
      }
      if((level < rsuAlertLevel_info) || (rsuAlertLevel_critical < level)){
          #if defined(EXTRA_DEBUG)
          I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"sendnotification: bad level(%d)\n", level);
          #endif
          level = rsuAlertLevel_info;
          ret = RSEMIB_BAD_DATA;
      }
      for(i = 0; i < MAX_TRY_LOCK; i++) {
          if(WTRUE == wsu_shmlock_trylockw(&shm_rsuhealth_ptr->h.ch_lock)) {
              if(WTRUE == shm_rsuhealth_ptr->h.ch_data_valid) {
                  shm_rsuhealth_ptr->rsuGnssDeviationError = level;
                  strncpy(shm_rsuhealth_ptr->rsuGnssDeviationMsg,rsuGnssDeviationMsg,sizeof(rsuGnssDeviationMsg));
                  ntcip_1218_mib.Async.rsuGnssDeviation_length = sizeof(rsuGnssDeviationMsg);
                  if(rsuAlertLevel_critical == level) {
                      shm_rsuhealth_ptr->gnssstatus |= RSUHEALTH_GNSS_BAD_LOC_DEV;
                  } else {
                      shm_rsuhealth_ptr->gnssstatus &= ~RSUHEALTH_GNSS_BAD_LOC_DEV;
                  }
                  local_gnssstatus = shm_rsuhealth_ptr->gnssstatus;

                  if(WFALSE == wsu_shmlock_unlockw(&shm_rsuhealth_ptr->h.ch_lock)) {
                      #if defined(EXTRA_DEBUG)
                      I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"wsu_shmlock_unlockw() failed.\n");
                      #endif
                      ret = RSEMIB_SYSCALL_FAIL;
                  }
                  #if defined(EXTRA_DEBUG)
                  I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"sendnotification success: (%d)(%s)\n",level,shm_rsuhealth_ptr->rsuGnssDeviationMsg);
                  #endif

                  break;
              } else {
                  #if defined(EXTRA_DEBUG) 
                  I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"shm not valid yet.\n");
                  #endif
                  ret = RSEMIB_SHM_INIT_FAILED;
              }
              if(WFALSE == wsu_shmlock_unlockw(&shm_rsuhealth_ptr->h.ch_lock)) {
                  #if defined(EXTRA_DEBUG) 
                  I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"wsu_shmlock_unlockw() failed.\n");
                  #endif
                  ret = RSEMIB_SYSCALL_FAIL;
              }
          }
          usleep(1000);
      } if (MAX_TRY_LOCK <= i) { /* Could happen. */
          #if defined(EXTRA_DEBUG)
          I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"wsu_shmlock_trylockw() failed.\n");
          #endif
          ret = RSEMIB_SYSCALL_FAIL;
      }
  } else {
      #if defined(EXTRA_DEBUG)
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"NULL == shm_rsuhealth_ptr.\n");
      #endif
      ret = RSEMIB_SHM_INIT_FAILED;
  }
  if(ret < RSEMIB_OK) 
      set_error_states(ret);

}
/* 
 * Update rsuGnssOutput & rsuGnssStatus.
 * TODO: There is only 1 interface so not using rsuGnssOutputInterface yet.
 * TODO: Enable snmptraps.
 */
void update_gnssOutput(void)
{
  int32_t ret = RSEMIB_OK;
  int32_t ret_deviation = RSEMIB_OK; /* So deviation test issues don't prevent notify. */
  int32_t ret_notify = RSEMIB_OK; /* So notify issues don't prevent deviation test. */
  int32_t  i,j;
  tps_geodetic_publish_t tps_pub_geo;
  uint16_t index = 0;
  uint8_t rsuGnssOutputString[RSU_GNSS_OUTPUT_STRING_MAX];
  int32_t rsuGnssOutputString_length = 0;
  GnssOutput_t gnssOutput; /* Local copy of latest. */
  float64_t rsuGnssLat_degrees = 0.0; /* Current scaled. */
  float64_t rsuGnssLon_degrees = 0.0;
  float64_t rsuGnssLat_ref = 0.0; /* User provided. */
  float64_t rsuGnssLon_ref = 0.0;
  #if defined(ENABLE_GNSS_ELV) /* For 3D if needed. */
  float64_t rsuGnssElv_metre = 0.0;  /* Current scaled. */
  float64_t rsuGnssElv_ref = 0.0;    /* User provided. */
  #endif
  float64_t ranger = 0.0;
  struct sockaddr_in dest;
  char_t cmd[RSUHEALTH_ALERT_MSG_LENGTH_MAX];

  /* Update from SHM. */
  memcpy(&tps_pub_geo, &shm_tps_ptr->pub_geodetic, sizeof(tps_geodetic_publish_t));
  index = tps_pub_geo.last_updated_pvt_index;
  if(NULL == shm_rsuhealth_ptr) {
      if ((shm_rsuhealth_ptr = wsu_share_init(sizeof(rsuhealth_t), RSUHEALTH_SHM_PATH)) == NULL) {
          I2V_DBG_LOG(MY_ERR_LEVEL, MY_I2V_NAME,"RSUHEALTH SHM init failed.\n");
          shm_rsuhealth_ptr = NULL; /* Keep going, not fatal. */
      }
  }
  /* 
   * Lock MIB, update gnssStatus & gnssOutput from TPS SHM, grab local copy of gnssOutput, unlock asap. 
   * Proceed to notify and deviation test. 
   */
  if(RSEMIB_OK == GET_LOCK()){
      /* Update rsuGnssOutputString: GGA: Add <LF> thats missing. Seems ublox doesn't add it or we dropped it. */
      j = strnlen(tps_pub_geo.last_GGA[index],MAX_NMEA_MESSASE_LEN);
      for(i=0;i<(j-1);i++) {
          if((char_t)0xD == tps_pub_geo.last_GGA[index][i]) {
              tps_pub_geo.last_GGA[index][i+1] = (char_t)0xA;            
              tps_pub_geo.last_GGA[index][i+2] = '\0';
              break;
          }
      }
      memset(ntcip_1218_mib.rsuGnssOutputString,0x0,RSU_GNSS_OUTPUT_STRING_MAX);
      memcpy(ntcip_1218_mib.rsuGnssOutputString,tps_pub_geo.last_GGA[index],RSU_GNSS_OUTPUT_STRING_MAX);
      ntcip_1218_mib.rsuGnssOutputString[RSU_GNSS_OUTPUT_STRING_MAX-1] = '\0';
      ntcip_1218_mib.rsuGnssOutputString_length = strnlen((char_t *)ntcip_1218_mib.rsuGnssOutputString, RSU_GNSS_OUTPUT_STRING_MAX);
      /* Update local copy of message here. */
      memcpy(rsuGnssOutputString,ntcip_1218_mib.rsuGnssOutputString, ntcip_1218_mib.rsuGnssOutputString_length);
      rsuGnssOutputString_length = ntcip_1218_mib.rsuGnssOutputString_length;
      /* Update lat,lon,elv. */
      ntcip_1218_mib.rsuGnssLat = tps_pub_geo.last_PVT[index].lat_deg_e7;
      if((ntcip_1218_mib.rsuGnssLat < RSU_LAT_MIN) || (RSU_LAT_MAX < ntcip_1218_mib.rsuGnssLat)) {
          ntcip_1218_mib.rsuGnssLat = RSU_LAT_UNKNOWN;
          DEBUGMSGTL((MY_NAME, "update_gnssOutput: bad rsuGnssLat.\n"));
          ret_deviation = GNSS_OUTPUT_BAD_GNSS_DATA;
      }
      ntcip_1218_mib.rsuGnssLon = tps_pub_geo.last_PVT[index].lon_deg_e7;
      if((ntcip_1218_mib.rsuGnssLon < RSU_LON_MIN) || (RSU_LON_MAX < ntcip_1218_mib.rsuGnssLon)) {
          ntcip_1218_mib.rsuGnssLon = RSU_LON_UNKNOWN;
          DEBUGMSGTL((MY_NAME, "update_gnssOutput: bad rsuGnssLon.\n"));
          ret_deviation = GNSS_OUTPUT_BAD_GNSS_DATA;
      }
      ntcip_1218_mib.rsuGnssElv = tps_pub_geo.last_PVT[index].height_above_elipsoid_mm/10; /* cm. */
      if((ntcip_1218_mib.rsuGnssElv < RSU_ELV_MIN) || (RSU_ELV_MAX < ntcip_1218_mib.rsuGnssElv)) {
          ntcip_1218_mib.rsuGnssElv = RSU_ELV_UNKNOWN;
          DEBUGMSGTL((MY_NAME, "update_gnssOutput: bad rsuGnssElv.\n"));
          ret_deviation = GNSS_OUTPUT_BAD_GNSS_DATA;
      }
      /* Get Pos Err. */
      ntcip_1218_mib.rsuGnssPositionError = RSU_GNSS_OUTPUT_POS_ERROR_UNKNOWN;
      if(0 < tps_pub_geo.last_PVT[index].fix_type) { /* Until there's a fix, don't bother. */
          ntcip_1218_mib.rsuGnssPositionError = tps_pub_geo.last_PVT[index].horiz_accuracy_est_mm/100; /* decimetre. */
          if(  (ntcip_1218_mib.rsuGnssPositionError  < RSU_GNSS_OUTPUT_POS_ERROR_MIN) 
             || (RSU_GNSS_OUTPUT_POS_ERROR_MAX < ntcip_1218_mib.rsuGnssPositionError)){
              ntcip_1218_mib.rsuGnssPositionError = RSU_GNSS_OUTPUT_POS_ERROR_UNKNOWN;
              DEBUGMSGTL((MY_NAME, "update_gnssOutput: bad rsuGnssPositionError.\n")); 
          }
      }
      /* Get local copy rsuGnssOutput from MIB. This has the recent updates the client made to the MIB. */
      memset(&gnssOutput,0x0,sizeof(gnssOutput));
      ret = copy_gnssOutput_to_skinny(&gnssOutput);
      /* Compare ip+port to see if it changed. If so close port so it can be re-opened with new values. */
      if(   (gnssOutputPort != gnssOutput.rsuGnssOutputPort) 
         || (0 != memcmp(gnssOutputAddress, gnssOutput.rsuGnssOutputAddress,RSU_DEST_IP_MAX))){
          gnssOutputPort = gnssOutput.rsuGnssOutputPort; /* Update to reflect new values from client. */
          memcpy(gnssOutputAddress,gnssOutput.rsuGnssOutputAddress,RSU_DEST_IP_MAX);
          if(-1 != gnssFwdSock) {
              close(gnssFwdSock);
              gnssFwdSock = -1;
          }
      }
      /* Calculate current deviation and update MIB. */
      ntcip_1218_mib.rsuLocationDeviation = RSU_GNSS_OUTPUT_DEVIATION_UNKNOWN;
      if((RSEMIB_OK == ret_deviation) && (0 < tps_pub_geo.last_PVT[index].fix_type)) { /* Until there's a fix, don't bother. */
          /* For deviation test. */
          rsuGnssLat_degrees  = ((float64_t)tps_pub_geo.last_PVT[index].lat_deg_e7) / (float64_t)(1.0e7); /* degree. */
          rsuGnssLon_degrees  = ((float64_t)tps_pub_geo.last_PVT[index].lon_deg_e7) / (float64_t)(1.0e7);
          #if defined(ENABLE_GNSS_ELV) 
          rsuGnssElv_metre    = ((float64_t)tps_pub_geo.last_PVT[index].height_above_elipsoid_mm) / (float64_t)(1000.0);  /* metre. */
          #endif
          /* From sysDescription, for deviation test. */
          rsuGnssLat_ref = ((float64_t)ntcip_1218_mib.rsuLocationLat) / (float64_t)(1.0e7);
          rsuGnssLon_ref = ((float64_t)ntcip_1218_mib.rsuLocationLon) / (float64_t)(1.0e7);
          #if defined(ENABLE_GNSS_ELV) 
          rsuGnssElv_ref = ((float64_t)ntcip_1218_mib.rsuLocationElv) / (float64_t)(1000.0); /* metre. */
          #endif
          if((0 < gnssOutput.rsuGnssMaxDeviation) && (RSEMIB_OK == ret) && (RSEMIB_OK == ret_deviation)){
              ranger = UtilCalcRangeHaversine(rsuGnssLat_ref, rsuGnssLon_ref, 45.0, rsuGnssLat_degrees, rsuGnssLon_degrees);
              if((RSU_GNSS_OUTPUT_DEVIATION_MIN <= ranger) && (ranger <= RSU_GNSS_OUTPUT_DEVIATION_MAX)){
                  ntcip_1218_mib.rsuLocationDeviation = ranger;
              }
          }
      }
      if(gnssOutputInterval != gnssOutput.rsuGnssOutputInterval) {
          gnssOutputInterval = gnssOutput.rsuGnssOutputInterval;
          current_interval_count = 1; /* Reset interval count. */
      }
      /* Update 5.3 rsuGnss while we are here. Report fails but don't halt other functions.*/
      ntcip_1218_mib.rsuGnssStatus = tps_pub_geo.last_PVT[index].num_sats;
      if((ntcip_1218_mib.rsuGnssStatus < GNSS_STATUS_MIN) || (GNSS_STATUS_MAX < ntcip_1218_mib.rsuGnssStatus)){
          ntcip_1218_mib.rsuGnssStatus = GNSS_STATUS_MIN;
          DEBUGMSGTL((MY_NAME, "update_gnssOutput: bad rsuGnssStatus.\n"));
          set_error_states(GNSS_OUTPUT_BAD_GNSS_DATA);
      }
      ntcip_1218_mib.rsuGnssAugmentation = tps_pub_geo.last_PVT[index].flag_differential_corrections_applied;
      /* Bit 2 claims diff correction applied. Assume DGPS. */
      if((0x2 & tps_pub_geo.last_PVT[index].flag_differential_corrections_applied) && (0 < tps_pub_geo.last_PVT[index].fix_type)){
          ntcip_1218_mib.rsuGnssAugmentation = rsuGnssAugmentation_other; /* DGPS is not WAAS. */
      } else {
          ntcip_1218_mib.rsuGnssAugmentation = rsuGnssAugmentation_none;
      }
      /* MIB uptodate and we have local copies we need.  */
      ntcip1218_unlock();
      /* Output rsuGnssOutputString to client ip+port. Fix or not send.*/
      if((0 < gnssOutput.rsuGnssOutputInterval) && (RSEMIB_OK == ret)){
          /* If Interval */
          if(gnssOutput.rsuGnssOutputInterval <= current_interval_count) { /* If user changes it to less on us then will see it. */
              /* GGA has newline in it so dont add one to log. */
              I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"%s",rsuGnssOutputString); /* Send to log regardless of socket state. */

              /* Send trap. If it fails, don't resend. Always next second. */
              sendgnssnotification((char_t *)rsuGnssOutputString, rsuAlertLevel_notice);

              /* If socket not open, then open. */
              if (-1 == gnssFwdSock) { 
                  /* Set up forwarding sock. */
                  if ((gnssFwdSock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
                      ret_notify = GNSS_OUTPUT_FWD_SOCK_FAIL;
                  } else {
                      if (!inet_aton((char_t *)gnssOutput.rsuGnssOutputAddress, &dest.sin_addr)) {
                          gnssOutput.rsuGnssOutputAddress[RSU_DEST_IP_MAX-1] = '\0'; /* Since its bad anyways force it to be a string for display.*/
                          DEBUGMSGTL((MY_NAME, "update_gnssOutput: bad IP=[%s]:errno=%d(%s).\n",(char_t *)gnssOutput.rsuGnssOutputAddress, errno, strerror(errno)));
                          ret_notify = GNSS_OUTPUT_FWD_INVALID_IP;
                          close(gnssFwdSock);
                          gnssFwdSock = -1;
                      } else {
                          dest.sin_family = AF_INET;
                          dest.sin_port = htons(gnssOutput.rsuGnssOutputPort);
                          if (connect(gnssFwdSock, (struct sockaddr *)&dest, sizeof(dest)) < 0) {
                              DEBUGMSGTL((MY_NAME_EXTRA, "update_gnssOutput: connect failed.\n"));
                              ret_notify = GNSS_OUTPUT_FWD_CONNECT_FAIL;
                              close(gnssFwdSock);
                              gnssFwdSock = -1;
                          }
                      }
                  }
              }
              /* Everything is good, send GNSS Output String. The write can fail periodically so don't close socket.*/
              if((-1 != gnssFwdSock) && (RSEMIB_OK == ret)) {
                  if(rsuGnssOutputString_length != write(gnssFwdSock, rsuGnssOutputString, rsuGnssOutputString_length)) {
                      DEBUGMSGTL((MY_NAME_EXTRA, "update_gnssOutput: write failed: errno=%d(%s).\n", errno, strerror(errno) ));
                      ret_notify = GNSS_OUTPUT_FWD_WRITE_FAIL;
                  } 
                  DEBUGMSGTL((MY_NAME_EXTRA, "update_gnssOutput: write[%s]\n",rsuGnssOutputString));
              }
              current_interval_count = 1;  /* Done with this interval. */
          } else {
              if(current_interval_count < 18000) {
                  current_interval_count++;
              } else {
                  current_interval_count = 1; /* Should not happen. Report but continue. */
                  DEBUGMSGTL((MY_NAME_EXTRA, "update_gnssOutput: current_interval_count rolled. That's odd.\n"));
                  set_error_states(GNSS_OUTPUT_INTERVAL_ROLL);
              }
          }
      } else {
          if(0 == (update_gnssOutput_counter % 10)) {
              DEBUGMSGTL((MY_NAME_EXTRA, "rsuGnssOutputInterval=%d, ret=%d: Disabled.\n", gnssOutput.rsuGnssOutputInterval, ret));
          }
          current_interval_count = 1;
          /* If port open then close now. */
          if(-1 != gnssFwdSock) {
              close(gnssFwdSock);
              gnssFwdSock = -1;
          }
      }

      /*----------------------------------------------------------------------------*/
      /* gnss deviation: JIRA 845                                                   */
      /*   Check rsuLocationDeviation against rsuGnssMaxDeviation in metres.        */
      /*   If rsuGnssMaxDeviation is zero then feature disabled.                    */
      /*   Smaller the deviation(<10m), more likely false detects.                  */
      /*----------------------------------------------------------------------------*/

      /* Do deviation test on current location. 2D is all that's required. Until there's a fix, don't bother. */
      if(   (0 < gnssOutput.rsuGnssMaxDeviation) 
         && (RSEMIB_OK == ret) && (RSEMIB_OK == ret_deviation) 
         && (0 < tps_pub_geo.last_PVT[index].fix_type)){
          if((float64_t)(gnssOutput.rsuGnssMaxDeviation) < ranger) { /* Trouble. Send notification. */
#if defined(EXTRA_DEBUG)
              I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"update_gnssOutput: ref:(%lf, %lf) live:(%lf %lf)\n",rsuGnssLat_ref, rsuGnssLon_ref,rsuGnssLat_degrees, rsuGnssLon_degrees);
              I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"update_gnssOutput: GnssMaxDeviation(%.3lf) < ranger(%.3lf)\n",(float64_t)(gnssOutput.rsuGnssMaxDeviation),ranger);
#endif
              /* Send trap once only. */
              if(0x0 == (local_gnssstatus & RSUHEALTH_GNSS_BAD_LOC_DEV)) {
                  memset(cmd,0x0,sizeof(cmd));
                  snprintf(cmd,sizeof(cmd),"GnssMaxDeviation(%.3lf) < RSU deviation(%.3lf)",(float64_t)(gnssOutput.rsuGnssMaxDeviation),ranger);
                  sendgnssdeviationnotification(cmd, rsuAlertLevel_critical);
              }
          } else {
              /* Send trap only once. */
              if(local_gnssstatus & RSUHEALTH_GNSS_BAD_LOC_DEV) {
                  memset(cmd,0x0,sizeof(cmd));
                  snprintf(cmd,sizeof(cmd),"No location deviation.");
                  sendgnssdeviationnotification(cmd, rsuAlertLevel_info);
              }
          }
      } else {
          /* Send trap only once. */
          if(local_gnssstatus & RSUHEALTH_GNSS_BAD_LOC_DEV) {
              memset(cmd,0x0,sizeof(cmd));
              snprintf(cmd,sizeof(cmd),"Location deviation test disabled.");
              sendgnssdeviationnotification(cmd, rsuAlertLevel_info);
          }
      }
  } else {
      ret = RSEMIB_LOCK_FAIL;
  }
  if(ret < RSEMIB_OK)
      set_error_states(ret);
  if(ret_deviation < RSEMIB_OK)
      set_error_states(ret_deviation);
  if(ret_notify < RSEMIB_OK)
      set_error_states(ret_notify);

  update_gnssOutput_counter++;
  if(0 == (update_gnssOutput_counter % 10)){
      DEBUGMSGTL((MY_NAME_EXTRA, "update_gnssOutput_counter=%d.\n",update_gnssOutput_counter));
  }
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
void *gnssOutputThr(void __attribute__((unused)) *arg)
{
  /* Clear statics used in update_gnssOutput() */
  update_gnssOutput_counter = 0; /* Rolling counter. */
  current_interval_count = 1; /* First interval is now. */
  gnssFwdSock = -1;
  gnssOutputPort = 0;
  memset(gnssOutputAddress,0x0,RSU_DEST_IP_MAX);
  gnssOutputInterval = 0;
  local_gnssstatus = 0x0; 
  DEBUGMSGTL((MY_NAME,"gnssOutputThr: Entry.\n"));
  /* Open TPS SHM */
  if ((shm_tps_ptr = mib_share_init(sizeof(shm_tps_t), SHM_TPS_PATH)) == NULL) {
      I2V_DBG_LOG(MY_ERR_LEVEL, MY_I2V_NAME,"TPS SHM init failed.\n");
  } else {
      while (mainloop) {
          sleep(1);            /* Once a second is a good rate. */
          rsuGnssOutput_error_states = 0x0; /* Reset, and if its a problem it will persist */
          update_gnssOutput(); /* Update OID with latest GNSS info, send GNSS message to client, deviation testa. */
          DEBUGMSGTL((MY_NAME_EXTRA,"gnssOutputThr: BEEP.\n"));
      }
      if(NULL != shm_rsuhealth_ptr) {
          mib_share_kill(shm_rsuhealth_ptr, sizeof(rsuhealth_t));
      }
      /* Close TPS SHM. */
      mib_share_kill(shm_tps_ptr, sizeof(shm_tps_t));
  }
  DEBUGMSGTL((MY_NAME,"gnssOutputThr: Exit."));
  I2V_DBG_LOG(MY_ERR_LEVEL, MY_I2V_NAME,"gnssOutputThr: Exit.\n");
  pthread_exit(NULL);
}
