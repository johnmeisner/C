/**************************************************************************
 *                                                                        *
 *     File Name:  rsuXmitMsgFwding.c                                     *
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
#include <mqueue.h>
#include "rsutable.h"
#include "ntcip-1218.h"
#include "i2v_util.h"
#if defined(MY_UNIT_TEST)
#include "stubs.h"
#endif

/*
 * Defines.
 */

/* Ignore requests we have serviced. Quicker, but no harm if not enabled. */
#define USE_PROCESSED_FLAG

/* Mandatory: For SNMP debugging and output. */
#define MY_NAME       "XmitMsg"
#define MY_NAME_EXTRA "XXmitMsg"
/* Mandatory: For debugging and syslog output. */
#define MY_I2V_NAME    MY_NAME
#define MY_ERR_LEVEL   LEVEL_PRIV  /* from i2v_util.h */

/* table column definitions. */
#define XMIT_MSG_FWD_INDEX_COLUMN      1
#define XMIT_MSG_FWD_PSID_COLUMN       2
#define XMIT_MSG_FWD_DEST_IP_COLUMN    3
#define XMIT_MSG_FWD_DEST_PORT_COLUMN  4
#define XMIT_MSG_FWD_PROTOCOL_COLUMN   5
#define XMIT_MSG_FWD_START_COLUMN      6
#define XMIT_MSG_FWD_STOP_COLUMN       7
#define XMIT_MSG_FWD_SECURE_COLUMN     8
#define XMIT_MSG_FWD_STATUS_COLUMN     9
/* Theoritical column ids. Make sure they follow in order with columns and unique. */
#define XMIT_MSG_FWD_MAX_OID           10

#define XMIT_MSG_FWD_INDEX_NAME    "rsuXmitMsgFwdingIndex"
#define XMIT_MSG_FWD_PSID_NAME     "rsuXmitMsgFwdingPsid"
#define XMIT_MSG_FWD_DEST_IP_NAME  "rsuXmitMsgFwdingDestIpAddr"
#define XMIT_MSG_FWD_DEST_PORT_NAME "rsuXmitMsgFwdingDestPort"
#define XMIT_MSG_FWD_PROTOCOL_NAME "rsuXmitMsgFwdingProtocol"
#define XMIT_MSG_FWD_START_NAME    "rsuXmitMsgFwdingDeliveryStart"
#define XMIT_MSG_FWD_STOP_NAME     "rsuXmitMsgFwdingDeliveryStop"
#define XMIT_MSG_FWD_SECURE_NAME   "rsuXmitMsgFwdingSecure"
#define XMIT_MSG_FWD_STATUS_NAME   "rsuXmitMsgFwdingStatus"
#define XMIT_MSG_FWD_MAX_NAME      "maxXmitMsgFwding"

/******************************************************************************
 * 5.21 Transmitted Messages For Forwarding: { rsu 20 }:
 * 
 * typedef struct {
 *     int32_t    rsuXmitMsgFwdingIndex;                                 RO: 1 to RSU_XMIT_MSG_COUNT_MAX
 *     uint8_t    rsuXmitMsgFwdingPsid[RSU_RADIO_PSID_SIZE];             RC: 1 to 4 OCTETS:
 *     uint8_t    rsuXmitMsgFwdingDestIpAddr[RSU_DEST_IP_MAX];           RC: 0 to 64: IPv4 or IPv6. simplified.
 *     int32_t    rsuXmitMsgFwdingDestPort;                              RC: 1024..65535.
 *     protocol_e rsuXmitMsgFwdingProtocol;                              RC: 1 = other, 2 = udp: default is udp.
 *     uint8_t    rsuXmitMsgFwdingDeliveryStart[MIB_DATEANDTIME_LENGTH]; RC: 8 OCTETS:DateAndTime:SNMPv2-TC.
 *     uint8_t    rsuXmitMsgFwdingDeliveryStop[MIB_DATEANDTIME_LENGTH];  RC: 8 OCTETS:DateAndTime:SNMPv2-TC.
 * 
 *     A value of 0 indicates the RSU is to forward only the WSM message payload without security headers. 
 *     Specifically this means that either of the following is forwarded, depending on whether the message is signed 
 *     (a) or unsigned (b): 
 *     (a) Ieee1609Dot2Data.signedData.tbsData.payload.data.unsecuredData 
 *     (b) Ieee1609Dot2Data.unsecuredData.
 *      
 *     A value of 1 indicates the RSU is to forward the entire WSM including the security headers. 
 *     Specifically this means that the entire Ieee1609Dot2Data frame is forwarded in COER format.
 *     
 *     int32_t rsuXmitMsgFwdingSecure; RC: 0|1.
 * 
 *     int32_t rsuXmitMsgFwdingStatus; RC: Create (4) & (6) destroy:
 *
 * } RsuXmitMsgFwdingEntry_t;
 * 
 ******************************************************************************/

const oid maxXmitMsgFwding_oid[]      = {1, 3, 6, 1, 4, 1, 1206, 4, 2, 18, 20, 1 };
const oid rsuXmitMsgFwdingTable_oid[] = {1, 3, 6, 1, 4, 1, 1206, 4, 2, 18, 20, 2 };

static netsnmp_table_registration_info * table_reg_info         = NULL; /* snmp table reg pointer. */
static netsnmp_tdata                   * rsuXmitMsgFwdingTable  = NULL; /* snmp table pointer. */
static uint32_t rsuXmitMsgFwding_error_states                   = 0x0;  /* local error state. */

/*
 * Protos: Silence of the Warnings.
 */
static void rebuild_rsuXmitMsgFwding(void);
static void destroy_rsuXmitMsgFwdingTable(void);

/* 
 * Function Bodies.
 */
static void set_error_states(int32_t error)
{
  /* -1 to -32 */
  if((error < 0) && (error > MIB_ERROR_BASE_2)){
      rsuXmitMsgFwding_error_states |= (uint32_t)(0x1) << (abs(error) - 1);
  }
}
uint32_t get_rsuXmitMsgFwding_error_states(void)
{
  return rsuXmitMsgFwding_error_states;
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
      case XMIT_MSG_FWD_INDEX_COLUMN:
          strcpy(column_name,XMIT_MSG_FWD_INDEX_NAME);
          break;
      case XMIT_MSG_FWD_PSID_COLUMN:
          strcpy(column_name,XMIT_MSG_FWD_PSID_NAME);
          break;
      case XMIT_MSG_FWD_DEST_IP_COLUMN:
          strcpy(column_name,XMIT_MSG_FWD_DEST_IP_NAME);
          break;
      case XMIT_MSG_FWD_DEST_PORT_COLUMN:
          strcpy(column_name,XMIT_MSG_FWD_DEST_PORT_NAME);
          break;
      case XMIT_MSG_FWD_PROTOCOL_COLUMN:
          strcpy(column_name,XMIT_MSG_FWD_PROTOCOL_NAME);
          break;
      case XMIT_MSG_FWD_START_COLUMN:
          strcpy(column_name,XMIT_MSG_FWD_START_NAME);
          break;
      case XMIT_MSG_FWD_STOP_COLUMN:
          strcpy(column_name,XMIT_MSG_FWD_STOP_NAME);
          break;
      case XMIT_MSG_FWD_SECURE_COLUMN:
          strcpy(column_name,XMIT_MSG_FWD_SECURE_NAME);
          break;
      case XMIT_MSG_FWD_STATUS_COLUMN:
          strcpy(column_name,XMIT_MSG_FWD_STATUS_NAME);
          break;
      case XMIT_MSG_FWD_MAX_OID:
          strcpy(column_name,XMIT_MSG_FWD_MAX_NAME);
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

/* 5.21.1 */
static int32_t handle_maxXmitMsgFwding(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo,
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
              if(0 <= (ret = get_maxXmitMsgFwding())){
                  snmp_set_var_typed_integer(requests->requestvb, ASN_INTEGER, ret);
                  ret = RSEMIB_OK;
              }
              break;
          default:
              ret = RSEMIB_BAD_MODE;
              break;
      }
      /* scalars and like are always index 0 with unqiue column.*/
      print_request_result(0, XMIT_MSG_FWD_MAX_OID,requests,reqinfo->mode,ret);
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
/* 5.21.2.1 */
static int32_t handle_rsuXmitMsgFwdingIndex(int32_t index, netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret  = RSEMIB_OK;
  int32_t data = index;

  DEBUGMSGTL((MY_NAME, "%s: Entry:\n",__FUNCTION__));
  switch (reqinfo->mode){
      case MODE_GET:
#if 0
          if(RSEMIB_OK == (ret = get_rsuXmitMsgFwdingIndex(index, &data)))
#endif
          {
              snmp_set_var_typed_integer(requests->requestvb, ASN_INTEGER, data);
              ret = RSEMIB_OK;
          }
          break;
      default:
          ret = RSEMIB_BAD_MODE;
          break;
  }
  return ret;
}
/* 5.21.2.2 */
static int32_t handle_rsuXmitMsgFwdingPsid(int32_t index, netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret = RSEMIB_OK;
  uint8_t data[RSU_RADIO_PSID_SIZE];

  DEBUGMSGTL((MY_NAME, "%s: Entry:\n",__FUNCTION__));
  switch (reqinfo->mode){
      case MODE_GET:
          DEBUGMSGTL((MY_NAME, "GET:\n"));
          memset(data,0x0,sizeof(data));
          if(0 <= (ret = get_rsuXmitMsgFwdingPsid(index,data))){
              snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR, data, ret);
              ret = RSEMIB_OK;
          }
          break;
      case MODE_SET_RESERVE1:
          DEBUGMSGTL((MY_NAME, "R1:\n"));
          if(RSEMIB_OK == (ret = preserve_rsuXmitMsgFwdingPsid(index))){
              if(RSEMIB_OK == (ret = action_rsuXmitMsgFwdingPsid(index,requests->requestvb->val.string,requests->requestvb->val_len))){
                  if(RSEMIB_OK == (ret = commit_rsuXmitMsgFwdingPsid(index))){
                      DEBUGMSGTL((MY_NAME, "COMMIT DONE:\n"));
                  } else {
                      DEBUGMSGTL((MY_NAME, "COMMIT FAIL:\n"));
                  }
              } else {
                  DEBUGMSGTL((MY_NAME, "ACTION FAIL:\n"));
              }
              if(RSEMIB_OK != ret) {
                  if(RSEMIB_OK != undo_rsuXmitMsgFwdingPsid(index)) { /* UNDO fails nothing we can do. */
                      DEBUGMSGTL((MY_NAME, "UNDO FAIL:\n"));
                  }
              }
          } else { /* If PRESERVE fails, don't call UNDO. */
              DEBUGMSGTL((MY_NAME, "PRESERVE FAIL:\n"));
          }
          if(RSEMIB_OK != ret) {
              DEBUGMSGTL((MY_NAME, "R1 FAILED ret=%d.\n", ret));
          }
          break;
      case MODE_SET_RESERVE2: /* These are anticpated requests. */
      case MODE_SET_ACTION:
      case MODE_SET_COMMIT:
      case MODE_SET_FREE:
      case MODE_SET_UNDO:
          ret = RSEMIB_OK; /* Done already. If we are using the 'processed' flag we shouldn't be here. */
          break;
      default:
          ret = RSEMIB_BAD_MODE;
          break;
  }
  return ret;
}
/* 5.21.2.3 */
int32_t handle_rsuXmitMsgFwdingDestIpAddr(int32_t index,netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret = RSEMIB_OK;
  uint8_t data[RSU_DEST_IP_MAX];

  DEBUGMSGTL((MY_NAME, "%s: Entry:\n",__FUNCTION__));
  switch (reqinfo->mode){
      case MODE_GET:
          DEBUGMSGTL((MY_NAME, "GET:\n"));
          memset(data,0x0,sizeof(data));
          if(0 <= (ret = get_rsuXmitMsgFwdingDestIpAddr(index,data))){
              snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR, data, ret);
              ret = RSEMIB_OK;
          }
          break;
      case MODE_SET_RESERVE1:
          DEBUGMSGTL((MY_NAME, "R1:\n"));
          if(RSEMIB_OK == (ret = preserve_rsuXmitMsgFwdingDestIpAddr(index))){
              if(RSEMIB_OK == (ret = action_rsuXmitMsgFwdingDestIpAddr(index,requests->requestvb->val.string,requests->requestvb->val_len))){
                  if(RSEMIB_OK == (ret = commit_rsuXmitMsgFwdingDestIpAddr(index))){
                      DEBUGMSGTL((MY_NAME, "COMMIT DONE:\n"));
                  } else {
                      DEBUGMSGTL((MY_NAME, "COMMIT FAIL:\n"));
                  }
              } else {
                  DEBUGMSGTL((MY_NAME, "ACTION FAIL:\n"));
              }
              if(RSEMIB_OK != ret) {
                  if(RSEMIB_OK != undo_rsuXmitMsgFwdingDestIpAddr(index)) { /* UNDO fails nothing we can do. */
                      DEBUGMSGTL((MY_NAME, "UNDO FAIL:\n"));
                  }
              }
          } else { /* If PRESERVE fails, don't call UNDO. */
              DEBUGMSGTL((MY_NAME, "PRESERVE FAIL:\n"));
          }
          if(RSEMIB_OK != ret) {
              DEBUGMSGTL((MY_NAME, "R1 FAILED ret=%d.\n", ret));
          }
          break;
      case MODE_SET_RESERVE2: /* These are anticpated requests. */
      case MODE_SET_ACTION:
      case MODE_SET_COMMIT:
      case MODE_SET_FREE:
      case MODE_SET_UNDO:
          ret = RSEMIB_OK; /* Done already. If we are using the 'processed' flag we shouldn't be here. */
          break;
      default:
          ret = RSEMIB_BAD_MODE;
          break;
  }
  return ret;
}
/* 5.21.2.4 */
static int32_t handle_rsuXmitMsgFwdingDestPort(int32_t index,netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret  = RSEMIB_OK;
  int32_t data = 0;

  DEBUGMSGTL((MY_NAME, "%s: Entry:\n",__FUNCTION__));
  switch (reqinfo->mode){
      case MODE_GET:
          DEBUGMSGTL((MY_NAME, "GET:\n"));
          if(RSEMIB_OK == (ret = get_rsuXmitMsgFwdingDestPort(index,&data))){
              snmp_set_var_typed_integer(requests->requestvb, ASN_INTEGER, data);
          }
          break;
      case MODE_SET_RESERVE1: /* Short stack, all requests done in one. */
          DEBUGMSGTL((MY_NAME, "R1:\n"));
          if(RSEMIB_OK == (ret = preserve_rsuXmitMsgFwdingDestPort(index))){
              if(RSEMIB_OK == (ret = action_rsuXmitMsgFwdingDestPort(index,*(requests->requestvb->val.integer)))) {
                  if(RSEMIB_OK == (ret = commit_rsuXmitMsgFwdingDestPort(index))){
                      DEBUGMSGTL((MY_NAME, "COMMIT DONE:\n"));
                  } else {
                      DEBUGMSGTL((MY_NAME, "COMMIT FAIL:\n"));
                  }
              } else {
                  DEBUGMSGTL((MY_NAME, "ACTION FAIL:\n"));
              }
              if(RSEMIB_OK != ret) {
                  if(RSEMIB_OK != undo_rsuXmitMsgFwdingDestPort(index)) { /* UNDO fails nothing we can do. */
                      DEBUGMSGTL((MY_NAME, "UNDO FAIL:\n"));
                  }
              }
          } else { /* If PRESERVE fails, don't call UNDO. */
              DEBUGMSGTL((MY_NAME, "PRESERVE FAIL:\n"));
          }
          if(RSEMIB_OK != ret) {
              DEBUGMSGTL((MY_NAME, "R1 FAILED ret=%d.\n", ret));
          }
          break;
      case MODE_SET_RESERVE2: /* These are anticpated requests. */
      case MODE_SET_ACTION:
      case MODE_SET_COMMIT:
      case MODE_SET_FREE:
      case MODE_SET_UNDO:
          ret = RSEMIB_OK; /* Done already. If we are using the 'processed' flag we shouldn't be here. */
          break;
      default:
          ret = RSEMIB_BAD_MODE;
          break;
  }
  return ret;
}
/* 5.21.2.5 */
static int32_t handle_rsuXmitMsgFwdingProtocol(int32_t index,netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret  = RSEMIB_OK;
  int32_t data = 0;

  DEBUGMSGTL((MY_NAME, "%s: Entry:\n",__FUNCTION__));
  switch (reqinfo->mode){
      case MODE_GET:
          DEBUGMSGTL((MY_NAME, "GET:\n"));
          if(RSEMIB_OK == (ret = get_rsuXmitMsgFwdingProtocol(index,&data))){
              snmp_set_var_typed_integer(requests->requestvb, ASN_INTEGER, data);
          }
          break;
      case MODE_SET_RESERVE1: /* Short stack, all requests done in one. */
          DEBUGMSGTL((MY_NAME, "R1:\n"));
          if(RSEMIB_OK == (ret = preserve_rsuXmitMsgFwdingProtocol(index))){
              if(RSEMIB_OK == (ret = action_rsuXmitMsgFwdingProtocol(index,*(requests->requestvb->val.integer)))) {
                  if(RSEMIB_OK == (ret = commit_rsuXmitMsgFwdingProtocol(index))){
                      DEBUGMSGTL((MY_NAME, "COMMIT DONE:\n"));
                  } else {
                      DEBUGMSGTL((MY_NAME, "COMMIT FAIL:\n"));
                  }
              } else {
                  DEBUGMSGTL((MY_NAME, "ACTION FAIL:\n"));
              }
              if(RSEMIB_OK != ret) {
                  if(RSEMIB_OK != undo_rsuXmitMsgFwdingProtocol(index)) { /* UNDO fails nothing we can do. */
                      DEBUGMSGTL((MY_NAME, "UNDO FAIL:\n"));
                  }
              }
          } else { /* If PRESERVE fails, don't call UNDO. */
              DEBUGMSGTL((MY_NAME, "PRESERVE FAIL:\n"));
          }
          if(RSEMIB_OK != ret) {
              DEBUGMSGTL((MY_NAME, "R1 FAILED ret=%d.\n", ret));
          }
          break;
      case MODE_SET_RESERVE2: /* These are anticpated requests. */
      case MODE_SET_ACTION:
      case MODE_SET_COMMIT:
      case MODE_SET_FREE:
      case MODE_SET_UNDO:
          ret = RSEMIB_OK; /* Done already. If we are using the 'processed' flag we shouldn't be here. */
          break;
      default:
          ret = RSEMIB_BAD_MODE;
          break;
  }
  return ret;
}
/* 5.21.2.6 */
static int32_t handle_rsuXmitMsgFwdingDeliveryStart(int32_t index,netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret = RSEMIB_OK;
  uint8_t data[MIB_DATEANDTIME_LENGTH];

  DEBUGMSGTL((MY_NAME, "%s: Entry:\n",__FUNCTION__));
  switch (reqinfo->mode){
      case MODE_GET:
          DEBUGMSGTL((MY_NAME, "GET:\n"));
          memset(data,0x0,sizeof(data));
          if(0 <= (ret = get_rsuXmitMsgFwdingDeliveryStart(index,data))){
              snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR, data, ret);
              ret = RSEMIB_OK;
          }
          break;
      case MODE_SET_RESERVE1:
          DEBUGMSGTL((MY_NAME, "R1:\n"));
          if(RSEMIB_OK == (ret = preserve_rsuXmitMsgFwdingDeliveryStart(index))){
              if(RSEMIB_OK == (ret = action_rsuXmitMsgFwdingDeliveryStart(index,requests->requestvb->val.string,requests->requestvb->val_len))){
                  if(RSEMIB_OK == (ret = commit_rsuXmitMsgFwdingDeliveryStart(index))){
                      DEBUGMSGTL((MY_NAME, "COMMIT DONE:\n"));
                  } else {
                      DEBUGMSGTL((MY_NAME, "COMMIT FAIL:\n"));
                  }
              } else {
                  DEBUGMSGTL((MY_NAME, "ACTION FAIL:\n"));
              }
              if(RSEMIB_OK != ret) {
                  if(RSEMIB_OK != undo_rsuXmitMsgFwdingDeliveryStart(index)) { /* UNDO fails nothing we can do. */
                      DEBUGMSGTL((MY_NAME, "UNDO FAIL:\n"));
                  }
              }
          } else { /* If PRESERVE fails, don't call UNDO. */
              DEBUGMSGTL((MY_NAME, "PRESERVE FAIL:\n"));
          }
          if(RSEMIB_OK != ret) {
              DEBUGMSGTL((MY_NAME, "R1 FAILED ret=%d.\n", ret));
          }
          break;
      case MODE_SET_RESERVE2: /* These are anticpated requests. */
      case MODE_SET_ACTION:
      case MODE_SET_COMMIT:
      case MODE_SET_FREE:
      case MODE_SET_UNDO:
          ret = RSEMIB_OK; /* Done already. If we are using the 'processed' flag we shouldn't be here. */
          break;
      default:
          ret = RSEMIB_BAD_MODE;
          break;
  }
  return ret;
}
/* 5.21.2.7 */
static int32_t handle_rsuXmitMsgFwdingDeliveryStop(int32_t index,netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret = RSEMIB_OK;
  uint8_t data[MIB_DATEANDTIME_LENGTH];

  DEBUGMSGTL((MY_NAME, "%s: Entry:\n",__FUNCTION__));
  switch (reqinfo->mode){
      case MODE_GET:
          DEBUGMSGTL((MY_NAME, "GET:\n"));
          memset(data,0x0,sizeof(data));
          if(0 <= (ret = get_rsuXmitMsgFwdingDeliveryStop(index,data))){
              snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR, data, ret);
              ret = RSEMIB_OK;
          }
          break;
      case MODE_SET_RESERVE1:
          DEBUGMSGTL((MY_NAME, "R1:\n"));
          if(RSEMIB_OK == (ret = preserve_rsuXmitMsgFwdingDeliveryStop(index))){
              if(RSEMIB_OK == (ret = action_rsuXmitMsgFwdingDeliveryStop(index,requests->requestvb->val.string,requests->requestvb->val_len))){
                  if(RSEMIB_OK == (ret = commit_rsuXmitMsgFwdingDeliveryStop(index))){
                      DEBUGMSGTL((MY_NAME, "COMMIT DONE:\n"));
                  } else {
                      DEBUGMSGTL((MY_NAME, "COMMIT FAIL:\n"));
                  }
              } else {
                  DEBUGMSGTL((MY_NAME, "ACTION FAIL:\n"));
              }
              if(RSEMIB_OK != ret) {
                  if(RSEMIB_OK != undo_rsuXmitMsgFwdingDeliveryStop(index)) { /* UNDO fails nothing we can do. */
                      DEBUGMSGTL((MY_NAME, "UNDO FAIL:\n"));
                  }
              }
          } else { /* If PRESERVE fails, don't call UNDO. */
              DEBUGMSGTL((MY_NAME, "PRESERVE FAIL:\n"));
          }
          if(RSEMIB_OK != ret) {
              DEBUGMSGTL((MY_NAME, "R1 FAILED ret=%d.\n", ret));
          }
          break;
      case MODE_SET_RESERVE2: /* These are anticpated requests. */
      case MODE_SET_ACTION:
      case MODE_SET_COMMIT:
      case MODE_SET_FREE:
      case MODE_SET_UNDO:
          ret = RSEMIB_OK; /* Done already. If we are using the 'processed' flag we shouldn't be here. */
          break;
      default:
          ret = RSEMIB_BAD_MODE;
          break;
  }
  return ret;
}
/* 5.21.2.8 */
static int32_t handle_rsuXmitMsgFwdingSecure(int32_t index,netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret  = RSEMIB_OK;
  int32_t data = 0;

  DEBUGMSGTL((MY_NAME, "%s: Entry:\n",__FUNCTION__));
  switch (reqinfo->mode){
      case MODE_GET:
          DEBUGMSGTL((MY_NAME, "GET:\n"));
          if(RSEMIB_OK == (ret = get_rsuXmitMsgFwdingSecure(index,&data))){
              snmp_set_var_typed_integer(requests->requestvb, ASN_INTEGER, data);
          }
          break;
      case MODE_SET_RESERVE1: /* Short stack, all requests done in one. */
          DEBUGMSGTL((MY_NAME, "R1:\n"));
          if(RSEMIB_OK == (ret = preserve_rsuXmitMsgFwdingSecure(index))){
              if(RSEMIB_OK == (ret = action_rsuXmitMsgFwdingSecure(index,*(requests->requestvb->val.integer)))) {
                  if(RSEMIB_OK == (ret = commit_rsuXmitMsgFwdingSecure(index))){
                      DEBUGMSGTL((MY_NAME, "COMMIT DONE:\n"));
                  } else {
                      DEBUGMSGTL((MY_NAME, "COMMIT FAIL:\n"));
                  }
              } else {
                  DEBUGMSGTL((MY_NAME, "ACTION FAIL:\n"));
              }
              if(RSEMIB_OK != ret) {
                  if(RSEMIB_OK != undo_rsuXmitMsgFwdingSecure(index)) { /* UNDO fails nothing we can do. */
                      DEBUGMSGTL((MY_NAME, "UNDO FAIL:\n"));
                  }
              }
          } else { /* If PRESERVE fails, don't call UNDO. */
              DEBUGMSGTL((MY_NAME, "PRESERVE FAIL:\n"));
          }
          if(RSEMIB_OK != ret) {
              DEBUGMSGTL((MY_NAME, "R1 FAILED ret=%d.\n", ret));
          }
          break;
      case MODE_SET_RESERVE2: /* These are anticpated requests. */
      case MODE_SET_ACTION:
      case MODE_SET_COMMIT:
      case MODE_SET_FREE:
      case MODE_SET_UNDO:
          ret = RSEMIB_OK; /* Done already. If we are using the 'processed' flag we shouldn't be here. */
          break;
      default:
          ret = RSEMIB_BAD_MODE;
          break;
  }
  return ret;
}
/* 5.21.2.9 */
static int32_t handle_rsuXmitMsgFwdingStatus(int32_t index,netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests, int32_t override)
{
  int32_t ret  = RSEMIB_OK;
  int32_t data = 0;
  int32_t row_status = SNMP_ROW_CREATEANDGO; /* If override is set then don't use requests's value. Use this one. */

  DEBUGMSGTL((MY_NAME, "%s: Entry:\n",__FUNCTION__));
  switch (reqinfo->mode){
      case MODE_GET:
          if(RSEMIB_OK == (ret = get_rsuXmitMsgFwdingStatus(index,&data))){
              snmp_set_var_typed_integer(requests->requestvb, ASN_INTEGER, data);
          }
          break;
      case MODE_SET_RESERVE1: /* Short minion stack, all requests done in one. */
          if(0 == override){
              row_status = *(requests->requestvb->val.integer); /* Client index part of CREATE. */
          }
          DEBUGMSGTL((MY_NAME, "R1_BULK_SET: status = %d.\n",row_status));
          if(RSEMIB_OK == (ret = preserve_rsuXmitMsgFwdingStatus(index))){
              if(RSEMIB_OK == (ret = action_rsuXmitMsgFwdingStatus(index,row_status))){
                  if(RSEMIB_OK == (ret = commit_rsuXmitMsgFwdingStatus(index))){
                      DEBUGMSGTL((MY_NAME, "COMMIT DONE:\n")); /* Minions unite, master will be happy! */
                  } else {
                      DEBUGMSGTL((MY_NAME, "COMMIT FAIL:\n"));
                  }
              } else {
                  DEBUGMSGTL((MY_NAME, "ACTION FAIL:\n"));
              }
              if(RSEMIB_OK != ret) { /* UNDO fails nothing we can do. */
                  if(RSEMIB_OK != undo_rsuXmitMsgFwdingStatus(index)) {
                      DEBUGMSGTL((MY_NAME, "UNDO FAIL:\n"));
                  }
              }
          } else { /* If PRESERVE fails, don't call UNDO. */
              DEBUGMSGTL((MY_NAME, "PRESERVE FAIL:\n"));
          }
          if(RSEMIB_OK != ret) {
              DEBUGMSGTL((MY_NAME, "R1_BULK_SET FAILED ret=%d.\n", ret)); /* Minions weep, master will be sad, so sad. */
          }
          break;
      case MODE_SET_RESERVE2: /* These are anticpated requests. */
      case MODE_SET_ACTION:
      case MODE_SET_COMMIT:
      case MODE_SET_FREE:
      case MODE_SET_UNDO:
          ret = RSEMIB_OK; /* Done already. If we are using the 'processed' flag we shouldn't be here. Minions sigh at tedium, carry on. */
          break;
      default:
          ret = RSEMIB_BAD_MODE;
          break;
  }
  return ret;
}
/* 
 * 5.21.2: Table handler: Break it down to smaller requests.
 */
int32_t handle_rsuXmitMsgFwdingTable( netsnmp_mib_handler          * handler
                                    , netsnmp_handler_registration * reginfo
                                    , netsnmp_agent_request_info   * reqinfo
                                    , netsnmp_request_info         * requests)
{
  netsnmp_request_info       * request       = NULL;
  uint32_t                     request_count = 0;
  RsuXmitMsgFwdingEntry_t    * table_entry   = NULL;
  netsnmp_table_request_info * request_info  = NULL;
  int32_t                      ret           = SNMP_ERR_NOERROR;
  uint8_t                      raw_oid[MAX_OID_LEN + 2]; /* If table doesn't exist then helper can't help. Go fish for row index. */
  uint32_t                     row_index = 0; /* For readability. Note: index is 1..max */
  int32_t                      row_status = SNMP_ROW_NONEXISTENT;

  handler = handler; /* Silence of the Warnings. */
  reginfo = reginfo;

  DEBUGMSGTL((MY_NAME, "rsuXmitMsgFwdingTable_handler: Entry:\n"));
  /* Service each request and provide netsnmp_set_request_error() result for each. */
  for (request = requests;
          (SNMP_ERR_NOERROR == ret) && request && (request_count < MAX_REQ_COUNT); 
               request = request->next,request_count++){ /* Limit run away request from client.*/

      if (request->processed){
          DEBUGMSGTL((MY_NAME, "request->processed.\n"));
          continue;
      }
      DEBUGMSGTL((MY_NAME_EXTRA, "New request:request_count=%d.\n",request_count));

      /* Do request & table wellness check first. */
      if(NULL == (request_info = netsnmp_extract_table_info(request))){
          set_error_states(RSEMIB_BAD_REQUEST_TABLE_INFO);
          DEBUGMSGTL((MY_NAME, "NULL == request_info.\n"));
          ret = SNMP_ERR_INCONSISTENTVALUE;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"RSEMIB_BAD_REQUEST_TABLE_INFO.\n");
          break;
      }
      if(NULL != (table_entry = (RsuXmitMsgFwdingEntry_t *)netsnmp_tdata_extract_entry(request))) {
          if((request_info->colnum < XMIT_MSG_FWD_INDEX_COLUMN) || (XMIT_MSG_FWD_STATUS_COLUMN < request_info->colnum)) {
              set_error_states(RSEMIB_BAD_REQUEST_COLUMN);
              ret = SNMP_ERR_INCONSISTENTVALUE;
              I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"RSEMIB_BAD_REQUEST_COLUMN.\n");
              break;
          }
      }

      /* Grab row index from request. */
      memset(raw_oid,0x0,sizeof(raw_oid));
      memcpy(raw_oid,&(request->requestvb->name_loc[0]),MAX_OID_LEN);
      row_index = raw_oid[115];
      row_index = (row_index << 8) | raw_oid[114];
      row_index = (row_index << 8) | raw_oid[113];
      row_index = (row_index << 8) | raw_oid[112];
      #if defined(EXTRA_DEBUG)
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"raw oid: row index = %u [%u|%u,%u,%u,%u|%u] column = %d.\n",row_index, raw_oid[111],raw_oid[112],raw_oid[113],raw_oid[114],raw_oid[115],raw_oid[116],raw_oid[104]);
      #endif

      /* Do get() on rowStatus to see if it exists and error check row_index. */
      if(RSEMIB_OK != (ret = get_rsuXmitMsgFwdingStatus(row_index,&row_status))) {
          DEBUGMSGTL((MY_NAME, "get_rsuXmitMsgFwdingStatus Failed: ret=%d\n",ret));
          set_error_states(RSEMIB_BAD_INDEX);
          ret = SNMP_ERR_INCONSISTENTVALUE;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"RSEMIB_BAD_INDEX.\n");
          break; /* Bad request. We are done. */
      }

      /* Depending on what you do the row must exist. */
      switch (reqinfo->mode){
          case MODE_GET:
#if 0 /* read+create: Helper prevents us from getting here. */
              if (SNMP_ROW_NONEXISTENT == row_status){ /* Can't GET if it doesn't exist. */
                  set_error_states(RSEMIB_BAD_REQUEST_TABLE_ENTRY);
                  DEBUGMSGTL((MY_NAME, "GET: No table entry for request.\n"));
                  ret = SNMP_ERR_INCONSISTENTVALUE;
              }
#endif
              break;
          case MODE_SET_RESERVE1:
          case MODE_SET_RESERVE2: 
          case MODE_SET_FREE:
          case MODE_SET_ACTION:
          case MODE_SET_COMMIT:
              break;
          case MODE_SET_UNDO:
              if (SNMP_ROW_NONEXISTENT == row_status){ /* Can't UNDO if it doesn't exist. */
                  set_error_states(RSEMIB_BAD_REQUEST_TABLE_ENTRY);
                  DEBUGMSGTL((MY_NAME, "UNDO: No request info.\n"));
                  ret = SNMP_ERR_INCONSISTENTVALUE;
                  I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"RSEMIB_BAD_REQUEST_TABLE_ENTRY.\n");
              }
              break;
          default: /* Doesth thou really care? Unanticipated request type.*/
              set_error_states(RSEMIB_BAD_MODE);
              DEBUGMSGTL((MY_NAME, "unrecognized mode.\n"));
              ret = SNMP_ERR_INCONSISTENTVALUE;
              I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"RSEMIB_BAD_MODE.\n");
              break;
      }
      if(SNMP_ERR_NOERROR != ret){
          DEBUGMSGTL((MY_NAME, "Bad SNMP request: ret=%d\n",ret));
          break; /* Bad request. We are done. */
      }
      DEBUGMSGTL((MY_NAME, "request_info is good: COLUMN=%u.\n",request_info->colnum));

      /* If it doesn't exist then any column can createAndWait. */
      if(SNMP_ROW_NONEXISTENT == row_status) {
          /* If this is the status column then let it drop through to call below with no override. */
          if(XMIT_MSG_FWD_STATUS_COLUMN != request_info->colnum){/* If row doesn't exist then create now. */
              ret = handle_rsuXmitMsgFwdingStatus(row_index, reqinfo, request, 1); /* Do not use value in request. Force createAndGo. */
              if(RSEMIB_OK != ret) {
                  DEBUGMSGTL((MY_NAME, "Create failed for ROW=%u COLUMN=%d.\n", row_index, request_info->colnum));
                  set_error_states(RSEMIB_CREATE_ROW_FAIL);
                  ret = SNMP_ERR_BADVALUE;
                  I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"RSEMIB_CREATE_ROW_FAIL: ROW=%u COLUMN=%d.\n", row_index, request_info->colnum);
                  break;
              }
          }
      }
      if(SNMP_ERR_NOERROR != ret){ /* Shouldn't need this, but just in case. */
          DEBUGMSGTL((MY_NAME, "Bad SNMP request: ret=%d\n",ret));
          break; /* Bad request. We are done. */
      }

      /*
       * Call our minion sub handlers to do our bidding.
       */
      DEBUGMSGTL((MY_NAME, "GO for handler ROW=%u COLUMN=%d.\n",row_index,request_info->colnum));
      switch (request_info->colnum){
          case XMIT_MSG_FWD_INDEX_COLUMN:
              ret = handle_rsuXmitMsgFwdingIndex(row_index, reqinfo, request);
              break;
          case XMIT_MSG_FWD_PSID_COLUMN:
              ret = handle_rsuXmitMsgFwdingPsid(row_index, reqinfo, request);
              break;
          case XMIT_MSG_FWD_DEST_IP_COLUMN:
              ret = handle_rsuXmitMsgFwdingDestIpAddr(row_index, reqinfo, request);
              break; 
          case XMIT_MSG_FWD_DEST_PORT_COLUMN:
              ret = handle_rsuXmitMsgFwdingDestPort(row_index, reqinfo, request);
              break; 
          case XMIT_MSG_FWD_PROTOCOL_COLUMN:
              ret = handle_rsuXmitMsgFwdingProtocol(row_index, reqinfo, request);
              break;
          case XMIT_MSG_FWD_START_COLUMN:
              ret = handle_rsuXmitMsgFwdingDeliveryStart(row_index, reqinfo, request);
              break;
          case XMIT_MSG_FWD_STOP_COLUMN:
              ret = handle_rsuXmitMsgFwdingDeliveryStop(row_index, reqinfo, request);
              break;
          case XMIT_MSG_FWD_SECURE_COLUMN:
              ret = handle_rsuXmitMsgFwdingSecure(row_index, reqinfo, request);
              break;
//If already created and status matches then who cares?
          case XMIT_MSG_FWD_STATUS_COLUMN:
              ret = handle_rsuXmitMsgFwdingStatus(row_index, reqinfo, request, 0);
              break;
          default:
              ret = RSEMIB_BAD_REQUEST_COLUMN; /* Should never happen. */
              break;
      }
      if(ret < RSEMIB_OK) {
          set_error_states(ret);
      }
      print_request_result(row_index, request_info->colnum,requests,reqinfo->mode,ret);
      /* Convert from MIB(negative) error to SNMP(positive) request error. */
      ret = handle_rsemib_errors(ret);
  }/* for */

  if(SNMP_ERR_NOERROR < ret){ /* Set once and only if error. Only top handler need do this. */
      netsnmp_set_request_error(reqinfo, requests, ret);
  }
#if defined(USE_PROCESSED_FLAG)
  requests->processed = 1; /* Good or bad, we handle request, GET or SET(R1 shortstack), only once. */
#endif
  DEBUGMSGTL((MY_NAME, "ret=%d:Exit.\n",ret));
  return ret;
}

/* 
 * Table Creation.
 */

/* 5.21.2: rsuXmitMsgFwdingTable creation functions. */
STATIC void rsuXmitMsgFwdingTable_removeEntry(netsnmp_tdata * table_data, netsnmp_tdata_row * row)
{
  RsuXmitMsgFwdingEntry_t * entry = NULL;

  if (NULL == row){
      DEBUGMSGTL((MY_NAME,"rsuXmitMsgFwdingTable_removeEntry NULL input.\n"));
      return;
  }
  if (NULL != (entry = (RsuXmitMsgFwdingEntry_t *)row->data)){ /* We allocate, so we free. */
      SNMP_FREE(entry);
  } else {
      DEBUGMSGTL((MY_NAME,"rsuXmitMsgFwdingTable_removeEntry row data is NULL!.\n"));
  }
  if (table_data){
      netsnmp_tdata_remove_and_delete_row(table_data, row);
  } else {
      DEBUGMSGTL((MY_NAME,"rsuXmitMsgFwdingTable_removeEntry NULL input.\n"));
      netsnmp_tdata_delete_row(row); 
  }
  return;
}
static void destroy_rsuXmitMsgFwdingTable(void)
{
  netsnmp_tdata_row  * row     = NULL;
  netsnmp_tdata_row  * nextrow = NULL;
  int32_t              i       = 0;

  if(NULL != rsuXmitMsgFwdingTable){
      if(NULL != (row = netsnmp_tdata_row_first(rsuXmitMsgFwdingTable))){
          for(i=0;(row != NULL) && (i < RSU_XMIT_MSG_COUNT_MAX); i++){
              nextrow = netsnmp_tdata_row_next(rsuXmitMsgFwdingTable, row);
              rsuXmitMsgFwdingTable_removeEntry(rsuXmitMsgFwdingTable, row);
              row = nextrow;
          }
      } else {
          DEBUGMSGTL((MY_NAME, "destroy_rsuXmitMsgFwdingTable: Table empty. Nothing to do.\n"));
      }
  } else {
      DEBUGMSGTL((MY_NAME, "destroy_rsuXmitMsgFwdingTable: Table is NULL. Did we get called before install?\n"));
  }
}

static int32_t rsuXmitMsgFwding_createTable(netsnmp_tdata * table_data, RsuXmitMsgFwdingEntry_t * rsuXmitMsgFwding)
{
  RsuXmitMsgFwdingEntry_t * entry = NULL;
  netsnmp_tdata_row       * row   = NULL;
  int32_t                   i     = 0;
  int32_t                   valid_row_count = 0;

  if(NULL == table_data){
      DEBUGMSGTL((MY_NAME, "rsuXmitMsgFwding_createTable FAIL: table_data == NULL.\n"));
      return -1;
  }

  for(i=0; i < RSU_XMIT_MSG_COUNT_MAX; i++){
      if((rsuXmitMsgFwding[i].rsuXmitMsgFwdingIndex < 1) || (RSU_XMIT_MSG_COUNT_MAX < rsuXmitMsgFwding[i].rsuXmitMsgFwdingIndex)){
          return -1; /* Indices must be unique and greater than 1. */
      }
      if(RSEMIB_OK != valid_table_row(rsuXmitMsgFwding[i].rsuXmitMsgFwdingStatus)){
          continue; /* Only add rows that have been created. */
      }
      if (NULL == (entry = SNMP_MALLOC_TYPEDEF(RsuXmitMsgFwdingEntry_t))){
          DEBUGMSGTL((MY_NAME, "rsuXmitMsgFwding SNMP_MALLOC_TYPEDEF FAIL.\n"));
          return -1;
      }
      if (NULL == (row = netsnmp_tdata_create_row())){
          DEBUGMSGTL((MY_NAME, "rsuXmitMsgFwding netsnmp_tdata_create_row FAIL.\n"));
          SNMP_FREE(entry);
          handle_rsemib_errors(RSEMIB_BAD_MODE);
          return -1;
      }

      DEBUGMSGTL((MY_NAME_EXTRA, "rsuXmitMsgFwding make row i=%d index=%d.\n",i,rsuXmitMsgFwding[i].rsuXmitMsgFwdingIndex));
      row->data = entry;
      memcpy(entry,&rsuXmitMsgFwding[i],sizeof(RsuXmitMsgFwdingEntry_t));
      netsnmp_tdata_row_add_index(row, ASN_INTEGER , &rsuXmitMsgFwding[i].rsuXmitMsgFwdingIndex, sizeof(int32_t));
      if(SNMPERR_SUCCESS != netsnmp_tdata_add_row(table_data, row)){
          DEBUGMSGTL((MY_NAME, "rsuXmitMsgFwding netsnmp_tdata_add_row FAIL i=%d.\n",i));
          handle_rsemib_errors(RSEMIB_BAD_MODE);
          return -1;
      }
      valid_row_count++;
  } /* for */
  DEBUGMSGTL((MY_NAME, "rsuXmitMsgFwding valid rows created = %d.\n",valid_row_count));
  return RSEMIB_OK;
}
static int32_t install_rsuXmitMsgFwdingTable(void)
{
    netsnmp_handler_registration * reg = NULL;

    reg = netsnmp_create_handler_registration("rsuXmitMsgFwdingTable"
                                              ,handle_rsuXmitMsgFwdingTable
                                              ,rsuXmitMsgFwdingTable_oid
                                              ,OID_LENGTH(rsuXmitMsgFwdingTable_oid)
                                              ,HANDLER_CAN_RWRITE);

    if (NULL == (rsuXmitMsgFwdingTable = netsnmp_tdata_create_table("rsuXmitMsgFwdingTable", 0))){
        DEBUGMSGTL((MY_NAME, "rsuXmitMsgFwdingTable FAIL.\n"));
        return RSEMIB_INSTALL_HANDLER_FAIL;
    }

    table_reg_info = SNMP_MALLOC_TYPEDEF(netsnmp_table_registration_info);

    if (NULL == table_reg_info){
        DEBUGMSGTL((MY_NAME, "SNMP_MALLOC_TYPEDEF FAIL.\n"));
        return RSEMIB_INSTALL_HANDLER_FAIL;
    }
    netsnmp_table_helper_add_indexes(table_reg_info, ASN_INTEGER, 0); /* index */                                 

    table_reg_info->min_column = XMIT_MSG_FWD_PSID_COLUMN;   /* Columns 2 to 9; Hide index by definition. */
    table_reg_info->max_column = XMIT_MSG_FWD_STATUS_COLUMN;

    if (SNMPERR_SUCCESS == netsnmp_tdata_register(reg, rsuXmitMsgFwdingTable, table_reg_info)){
        return RSEMIB_OK; /* Table registered but still empty. rebuild_rsuXmitMsgFwding() will populate */
    } else {
        DEBUGMSGTL((MY_NAME, "netsnmp_tdata_register FAIL.\n"));
        return RSEMIB_INSTALL_HANDLER_FAIL;
    }
}
/* Assumed locked already. Live pointer to db. */
int32_t rebuild_rsuXmitMsgFwding_live(RsuXmitMsgFwdingEntry_t * rsuXmitMsgFwding)
{
  if(NULL == rsuXmitMsgFwding) {
      return RSEMIB_BAD_INPUT;
  }
  /* Remove rows but don't remove table reg. */
  destroy_rsuXmitMsgFwdingTable();

  /* Initialise the contents of the table here.*/
  if (RSEMIB_OK != rsuXmitMsgFwding_createTable(rsuXmitMsgFwdingTable, rsuXmitMsgFwding)){
      DEBUGMSGTL((MY_NAME, "rebuild_rsuXmitMsgFwding_live: ERROR: Null row.\n"));
      set_error_states(RSEMIB_REBUILD_FAIL);
      return RSEMIB_REBUILD_FAIL;
  }
  return RSEMIB_OK;
}
static void rebuild_rsuXmitMsgFwding(void)
{
  int32_t i = 0;
  RsuXmitMsgFwdingEntry_t rsuXmitMsgFwding[RSU_XMIT_MSG_COUNT_MAX];

  memset(rsuXmitMsgFwding, 0x0, sizeof(rsuXmitMsgFwding));
  for(i=0; i < RSU_XMIT_MSG_COUNT_MAX; i++){
      if(RSEMIB_OK > get_rsuXmitMsgFwdingIndex(i+1,&rsuXmitMsgFwding[i].rsuXmitMsgFwdingIndex)){
          set_error_states(RSEMIB_REBUILD_FAIL);
          DEBUGMSGTL((MY_NAME, "rsuXmitMsgFwdingIndex rebuild FAIL: i = %d.\n",i));
      }
      if(RSEMIB_OK > get_rsuXmitMsgFwdingPsid(i+1,rsuXmitMsgFwding[i].rsuXmitMsgFwdingPsid)){
          set_error_states(RSEMIB_REBUILD_FAIL);
          DEBUGMSGTL((MY_NAME, "rsuXmitMsgFwdingPsid rebuild FAIL: i = %d.\n",i));
      }
      if(RSEMIB_OK > get_rsuXmitMsgFwdingDestIpAddr (i+1,rsuXmitMsgFwding[i].rsuXmitMsgFwdingDestIpAddr )){
          set_error_states(RSEMIB_REBUILD_FAIL);
          DEBUGMSGTL((MY_NAME, "rsuXmitMsgFwdingDestIpAddr  rebuild FAIL: i = %d.\n",i));
      }
      if(RSEMIB_OK > get_rsuXmitMsgFwdingDestPort(i+1,&rsuXmitMsgFwding[i].rsuXmitMsgFwdingDestPort)){
          set_error_states(RSEMIB_REBUILD_FAIL);
          DEBUGMSGTL((MY_NAME, "rsuXmitMsgFwdingDestPort rebuild FAIL: i = %d.\n",i));
      }
      if(RSEMIB_OK > get_rsuXmitMsgFwdingProtocol(i+1,(int32_t *)&rsuXmitMsgFwding[i].rsuXmitMsgFwdingProtocol)){
          set_error_states(RSEMIB_REBUILD_FAIL);
          DEBUGMSGTL((MY_NAME, "rsuXmitMsgFwdingProtocol rebuild FAIL: i = %d.\n",i));
      }
      if(RSEMIB_OK > get_rsuXmitMsgFwdingDeliveryStart(i+1,rsuXmitMsgFwding[i].rsuXmitMsgFwdingDeliveryStart)){
          set_error_states(RSEMIB_REBUILD_FAIL);
          DEBUGMSGTL((MY_NAME, "rsuXmitMsgFwdingDeliveryStart rebuild FAIL: i = %d.\n",i));
      }
      if(RSEMIB_OK > get_rsuXmitMsgFwdingDeliveryStop(i+1,rsuXmitMsgFwding[i].rsuXmitMsgFwdingDeliveryStop)){
          set_error_states(RSEMIB_REBUILD_FAIL);
          DEBUGMSGTL((MY_NAME, "rsuXmitMsgFwdingDeliveryStop rebuild FAIL: i = %d.\n",i));
      }
      if(RSEMIB_OK > get_rsuXmitMsgFwdingSecure (i+1,&rsuXmitMsgFwding[i].rsuXmitMsgFwdingSecure)){
          set_error_states(RSEMIB_REBUILD_FAIL);
          DEBUGMSGTL((MY_NAME, "rsuXmitMsgFwdingSecure rebuild FAIL: i = %d.\n",i));
      }
      if(RSEMIB_OK > get_rsuXmitMsgFwdingStatus(i+1,&rsuXmitMsgFwding[i].rsuXmitMsgFwdingStatus)){
          set_error_states(RSEMIB_REBUILD_FAIL);
          DEBUGMSGTL((MY_NAME, "rsuXmitMsgFwdingStatus rebuild FAIL: i = %d.\n",i));
      }
  }
  /* Remove rows but don't remove table reg. */
  destroy_rsuXmitMsgFwdingTable();

  /* Initialize the contents of the table here.*/
  if (RSEMIB_OK != rsuXmitMsgFwding_createTable(rsuXmitMsgFwdingTable, &rsuXmitMsgFwding[0])){
      DEBUGMSGTL((MY_NAME, "rebuild_rsuXmitMsgFwding: ERROR: Null row.\n"));
      set_error_states(RSEMIB_CREATE_TABLE_FAIL); /* FAIL. Table NOT ready. */
  }
}
void install_rsuXmitMsgFwding_handlers(void)
{
  /* Reset statics across soft reset. */
  rsuXmitMsgFwdingTable         = NULL;
  table_reg_info                = NULL;
  rsuXmitMsgFwding_error_states = 0x0;

  /* 5.21.1 */
  if(MIB_REGISTERED_OK != netsnmp_register_scalar(netsnmp_create_handler_registration
                          ("maxXmitMsgFwding", 
                           handle_maxXmitMsgFwding,
                           maxXmitMsgFwding_oid,
                           OID_LENGTH(maxXmitMsgFwding_oid),
                           HANDLER_CAN_RONLY))){
      DEBUGMSGTL((MY_NAME, "maxXmitMsgFwding install FAIL.\n"));
      set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
  }
  /* 5.21.2.1 - 5.21.2.9: Install & register table handler. Does not populate table with data. */
  if(RSEMIB_OK != install_rsuXmitMsgFwdingTable()){
      DEBUGMSGTL((MY_NAME, "rsuXmitMsgFwdingTable install FAIL.\n"));
      set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
  }

  /* Populate the table. */
  rebuild_rsuXmitMsgFwding();
}
void destroy_rsuXmitMsgFwding(void)
{
  destroy_rsuXmitMsgFwdingTable();

  /* Free table info. */
  if (NULL != table_reg_info){
      SNMP_FREE(table_reg_info);
  }
  /* Free table reg. */
  if (NULL != rsuXmitMsgFwdingTable){
      SNMP_FREE(rsuXmitMsgFwdingTable);
  }
}
