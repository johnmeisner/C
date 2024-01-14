/**************************************************************************
 *                                                                        *
 *     File Name:  rsuReceivedMsg.c                                       *
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
#include "rsutable.h"
#include "ntcip-1218.h"
#include "i2v_util.h"
#if defined(MY_UNIT_TEST)
#include "stubs.h"
#endif

/*
 * Defines.
 */

#define WAIT_BOOT_TIME 30 /* Seconds. */

/* Ignore requests we have serviced. Quicker, but no harm if not enabled. */
#define USE_PROCESSED_FLAG

/* Mandatory: For SNMP debugging and output. */
#define MY_NAME       "RxMsg"
#define MY_NAME_EXTRA "XRxMsg"
/* Mandatory: For debugging and syslog output. */
#define MY_I2V_NAME    MY_NAME
#define MY_ERR_LEVEL   LEVEL_PRIV  /* from i2v_util.h */

/* rsuReceivedMsg.c specific: Create what you need to debug. */
#define RX_MSG_PORT_CONFLICT           -30
#define RX_MSG_IP_CONFLICT             -31
#define RX_MSG_IP_PORT_CONFLICT        -32

/* table column definitions. */
#define RX_MSG_INDEX_COLUMN         1
#define RX_MSG_PSID_COLUMN          2
#define RX_MSG_DEST_IP_COLUMN       3
#define RX_MSG_DEST_PORT_COLUMN     4
#define RX_MSG_PROTOCOL_COLUMN      5
#define RX_MSG_RSSI_COLUMN          6
#define RX_MSG_INTERVAL_COLUMN      7
#define RX_MSG_START_COLUMN         8
#define RX_MSG_STOP_COLUMN          9
#define RX_MSG_STATUS_COLUMN        10
#define RX_MSG_SECURE_COLUMN        11
#define RX_MSG_AUTH_INTERVAL_COLUMN 12
 /* Theoritical column ids. Make sure they follow in order with columns and unique. */
#define RX_MSG_MAX_OID              13

#define RX_MSG_INDEX_NAME     "rsuReceivedMsgIndex"
#define RX_MSG_PSID_NAME      "rsuReceivedMsgPsid"
#define RX_MSG_DEST_IP_NAME   "rsuReceivedMsgDestIpAddr"
#define RX_MSG_DEST_PORT_NAME "rsuReceivedMsgDestPort"
#define RX_MSG_PROTOCOL_NAME  "rsuReceivedMsgProtocol"
#define RX_MSG_RSSI_NAME      "rsuReceivedMsgRssi"
#define RX_MSG_INTERVAL_NAME  "rsuReceivedMsgInterval"
#define RX_MSG_START_NAME     "rsuReceivedMsgDeliveryStart"
#define RX_MSG_STOP_NAME      "rsuReceivedMsgDeliveryStop"
#define RX_MSG_STATUS_NAME    "rsuReceivedMsgStatus"
#define RX_MSG_SECURE_NAME    "rsuReceivedMsgSecure"
#define RX_MSG_AUTH_INTERVAL_NAME "rsuReceivedMsgAuthMsgInterval"
 /* Theoritical column ids. Make sure they follow in order with columns and unique. */
#define RX_MSG_MAX_NAME       "maxRsuReceivedMsgs"

/****************************************************************************** 
 * 5.6 (OTA) Received Messages(to fwd via socket): { rsu 5 }: rsuReceivedMsgTable
 *
 * typedef struct {
 *    int32_t    rsuReceivedMsgIndex;                                 RO: 1 to maxRsuReceivedMsgs.
 *    uint8_t    rsuReceivedMsgPsid;                                  RC: 1 to 4 OCTETS:
 *    uint8_t    rsuReceivedMsgDestIpAddr[RSU_DEST_IP_MAX];           RC: 0 to 64: IPv4 or IPv6.
 *    int32_t    rsuReceivedMsgDestPort;                              RC: 1024..65535.
 *    protocol_e rsuReceivedMsgProtocol;                              RC: 1 = other , 2 = udp: default is udp.
 *    int32_t    rsuReceivedMsgRssi;                                  RC: -100 to -60: no default.
 *    int32_t    rsuReceivedMsgInterval;                              RC: 0 to 10: 0 == disable. No default.
 *    uint8_t    rsuReceivedMsgDeliveryStart[MIB_DATEANDTIME_LENGTH]; RC:"2d-1d-1d,1d:1d:1d.1d": 8 OCTETS:DateAndTime:SNMPv2-TC.
 *    uint8_t    rsuReceivedMsgDeliveryStop[MIB_DATEANDTIME_LENGTH];  RC:"2d-1d-1d,1d:1d:1d.1d": 8 OCTETS:DateAndTime:SNMPv2-TC.
 *    int32_t    rsuReceivedMsgStatus;                                RC: Create (4) & (6) destroy:
 *
 * 
 *    A value of 0 indicates the RSU is to forward only the WSM message payload without security headers. 
 *    Specifically this means that either of the following is forwarded, 
 *    depending on whether the message is signed (a) or unsigned (b): 
 *    (a) Ieee1609Dot2Data.signedData.tbsData.payload.data.unsecuredData or 
 *    (b) Ieee1609Dot2Data.unsecuredData.
 *    A value of 1 indicates the RSU is to forward the entire WSM including the security headers. 
 *    Specifically this means that the entire Ieee1609Dot2Data frame is forwarded in COER format.
 *
 *    int32_t rsuReceivedMsgSecure;       RC: 0 == only fwd wsm without 1609.2, 1 == with header.
 *
 * 
 *    Interval with which the RSU authenticates messages received from a specific device over the V2X Interface 
 *    and to be forwarded to the Server (as controlled by rsuDsrcFwdMsgInterval). If enabled, the RSU authenticates 
 *    the first valid (e.g., as defined by rsuDsrcFwdRssi, rsuDsrcFwdDeliveryStart, rsuDsrcFwdDeliveryStop for this row) 
 *    message received from a specific device. For a value of 4, the RSU then authenticates every 4th message 
 *    (after the first message) for that specific device that is marked for forwarding (as determined by rsuDsrcFwdMsgInterval). 
 *    A value of 0 disables authentication of message to be forward for this particular entry. For example, an 
 *    RSU receives 12 messages that satisfies the criteria for this row (rsuDsrcFwdPsid, rsuDsrcFwdRssi, 
 *    rsuDsrcFwdDeliveryStart, rsuDsrcFwdDeliveryStop). Messages 1, 2, 5, 6, 7, 10 and 12 are from device A 
 *    and messages 3, 4, 8, 9 and 11 are from device B. Assuming rsuDsrcFwdMsgInterval has a value of 2, 
 *    only messages 1, 3, 5, 7, 9, and 11 are 'marked' for forwarding. Of these messages, 
 *    only messages 1 (the first message from device A), 3 (the first message from device B), 
 *    7 (the 2nd message from device A after the first message), and 11 (the 2nd message from device B after the first message) 
 *    are authenticated.
 *
 *    int32_t rsuReceivedMsgAuthMsgInterval; RC: 0 to 10: 0 == disable. No default.
 *  
 * } RsuReceivedMsgEntry_t;
 *
 ******************************************************************************/

const oid maxRsuReceivedMsgs_oid[]  = {1, 3, 6, 1, 4, 1, 1206, 4, 2, 18, 5 , 1};
const oid rsuReceivedMsgTable_oid[] = {1, 3, 6, 1, 4, 1, 1206, 4, 2, 18, 5 , 2};

extern int32_t  mainloop; /* For thread handling. */
extern ntcip_1218_mib_t  ntcip_1218_mib;

static netsnmp_table_registration_info * table_reg_info = NULL; /* snmp table reg pointer. */
static netsnmp_tdata * rsuReceivedMsgTable              = NULL; /* snmp table pointer. */
static uint32_t receivedMsg_error_states                = 0x0;  /* local error state. */

/*
 * Protos: Silence of the Warnings.
 */
static void rebuild_rsuReceivedMsg(void);
static void destroy_rsuReceivedMsgTable(void);

/* 
 * Function Bodies.
 */
static void set_error_states(int32_t error)
{
  /* -1 to -32 */
  if((error < 0) && (error > MIB_ERROR_BASE_2)){
      receivedMsg_error_states |= (uint32_t)(0x1) << (abs(error) - 1);
  }
}
uint32_t get_receivedMsg_error_states(void)
{
  return receivedMsg_error_states;
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
      case RX_MSG_INDEX_COLUMN:
          strcpy(column_name,RX_MSG_INDEX_NAME);
          break;
      case RX_MSG_PSID_COLUMN:
          strcpy(column_name,RX_MSG_PSID_NAME);
          break;
      case RX_MSG_DEST_IP_COLUMN:
          strcpy(column_name,RX_MSG_DEST_IP_NAME);
          break;
      case RX_MSG_DEST_PORT_COLUMN:
          strcpy(column_name,RX_MSG_DEST_PORT_NAME);
          break;
      case RX_MSG_PROTOCOL_COLUMN:
          strcpy(column_name,RX_MSG_PROTOCOL_NAME);
          break;
      case RX_MSG_RSSI_COLUMN:
          strcpy(column_name,RX_MSG_RSSI_NAME);
          break;
      case RX_MSG_INTERVAL_COLUMN:
          strcpy(column_name,RX_MSG_INTERVAL_NAME);
          break;
      case RX_MSG_START_COLUMN:
          strcpy(column_name,RX_MSG_START_NAME);
          break;
      case RX_MSG_STOP_COLUMN:
          strcpy(column_name,RX_MSG_STOP_NAME);
          break;
      case RX_MSG_STATUS_COLUMN:
          strcpy(column_name,RX_MSG_STATUS_NAME);
          break;
      case RX_MSG_SECURE_COLUMN:
          strcpy(column_name,RX_MSG_SECURE_NAME);
          break;
      case RX_MSG_AUTH_INTERVAL_COLUMN:
          strcpy(column_name,RX_MSG_AUTH_INTERVAL_NAME);
          break;
/* For scalars and like kind. */
      case RX_MSG_MAX_OID:
          strcpy(column_name,RX_MSG_MAX_NAME);
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

/* 5.6.1 */
static int32_t handle_maxRsuReceivedMsgs(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo,
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
              if(0 <= (ret = get_maxRsuReceivedMsgs())){
                  snmp_set_var_typed_integer(requests->requestvb, ASN_INTEGER, ret);
                  ret = RSEMIB_OK;
              }
              break;
          default:
              ret = RSEMIB_BAD_MODE;
              break;
      }
      /* scalars and like are always index 0 with unqiue column.*/
      print_request_result(0, RX_MSG_MAX_OID,requests,reqinfo->mode,ret);
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
/* 5.6.2.1: Although stored, we already know the 'index'. Already ranged checked beforehand. */
int32_t handle_rsuReceivedMsgIndex(int32_t index, netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret  = RSEMIB_OK;
  int32_t data = index;

  DEBUGMSGTL((MY_NAME, "%s: Entry:\n",__FUNCTION__));
  switch (reqinfo->mode){
      case MODE_GET:
#if 0
          if(RSEMIB_OK == (ret = get_rsuReceivedMsgIndex(index, &data)))
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
/* 5.6.2.2 */
static int32_t handle_rsuReceivedMsgPsid(int32_t index, netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret = RSEMIB_OK;
  uint8_t data[RSU_RADIO_PSID_SIZE];

  DEBUGMSGTL((MY_NAME, "%s: Entry:\n",__FUNCTION__));
  switch (reqinfo->mode){
      case MODE_GET:
          DEBUGMSGTL((MY_NAME, "GET:\n"));
          memset(data,0x0,sizeof(data));
          if(0 <= (ret = get_rsuReceivedMsgPsid(index,data))){
              snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR, data, ret);
              ret = RSEMIB_OK;
          }
          break;
      case MODE_SET_RESERVE1:
          DEBUGMSGTL((MY_NAME, "R1:\n"));
          if(RSEMIB_OK == (ret = preserve_rsuReceivedMsgPsid(index))){
              if(RSEMIB_OK == (ret = action_rsuReceivedMsgPsid(index,requests->requestvb->val.string,requests->requestvb->val_len))){
                  if(RSEMIB_OK == (ret = commit_rsuReceivedMsgPsid(index))){
                      DEBUGMSGTL((MY_NAME, "COMMIT DONE:\n"));
                  } else {
                      DEBUGMSGTL((MY_NAME, "COMMIT FAIL:\n"));
                  }
              } else {
                  DEBUGMSGTL((MY_NAME, "ACTION FAIL:\n"));
              }
              if(RSEMIB_OK != ret) {
                  if(RSEMIB_OK != undo_rsuReceivedMsgPsid(index)) { /* UNDO fails nothing we can do. */
                      DEBUGMSGTL((MY_NAME, "UNDO FAIL:\n"));
                  }
              }
          } else { /* If PRESERVE fails, don't call UNDO. */
              DEBUGMSGTL((MY_NAME, "PRESERVE FAIL:\n"));
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
/* 5.6.2.3 */
static int32_t handle_rsuReceivedMsgDestIpAddr(int32_t index, netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret = RSEMIB_OK;
  uint8_t data[RSU_DEST_IP_MAX];

  DEBUGMSGTL((MY_NAME, "%s: Entry:\n",__FUNCTION__));
  switch (reqinfo->mode){
      case MODE_GET:
          DEBUGMSGTL((MY_NAME, "GET:\n"));
          memset(data,0x0,sizeof(data));
          if(0 <= (ret = get_rsuReceivedMsgDestIpAddr(index,data))){
              snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR, data, ret);
              ret = RSEMIB_OK;
          }
          break;
      case MODE_SET_RESERVE1:
          DEBUGMSGTL((MY_NAME, "R1:\n"));
          if(RSEMIB_OK == (ret = preserve_rsuReceivedMsgDestIpAddr(index))){
              if(RSEMIB_OK == (ret = action_rsuReceivedMsgDestIpAddr(index,requests->requestvb->val.string,requests->requestvb->val_len))){
                  if(RSEMIB_OK == (ret = commit_rsuReceivedMsgDestIpAddr(index))){
                      DEBUGMSGTL((MY_NAME, "COMMIT DONE:\n"));
                  } else {
                      DEBUGMSGTL((MY_NAME, "COMMIT FAIL:\n"));
                  }
              } else {
                  DEBUGMSGTL((MY_NAME, "ACTION FAIL:\n"));
              }
              if(RSEMIB_OK != ret) {
                  if(RSEMIB_OK != undo_rsuReceivedMsgDestIpAddr(index)) { /* UNDO fails nothing we can do. */
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
/* 5.6.2.4 */
static int32_t handle_rsuReceivedMsgDestPort(int32_t index,netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret  = RSEMIB_OK;
  int32_t data = 0;

  DEBUGMSGTL((MY_NAME, "%s: Entry:\n",__FUNCTION__));
  switch (reqinfo->mode){
      case MODE_GET:
          DEBUGMSGTL((MY_NAME, "GET:\n"));
          if(RSEMIB_OK == (ret = get_rsuReceivedMsgDestPort(index,&data))){
              snmp_set_var_typed_integer(requests->requestvb, ASN_INTEGER, data);
          }
          break;
      case MODE_SET_RESERVE1: /* Short stack, all requests done in one. */
          DEBUGMSGTL((MY_NAME, "R1:\n"));
          if(RSEMIB_OK == (ret = preserve_rsuReceivedMsgDestPort(index))){
              if(RSEMIB_OK == (ret = action_rsuReceivedMsgDestPort(index,*(requests->requestvb->val.integer)))){
                  if(RSEMIB_OK == (ret = commit_rsuReceivedMsgDestPort(index))){
                      DEBUGMSGTL((MY_NAME, "COMMIT DONE:\n"));
                  } else {
                      DEBUGMSGTL((MY_NAME, "COMMIT FAIL:\n"));
                  }
              } else {
                  DEBUGMSGTL((MY_NAME, "ACTION FAIL:\n"));
              }
              if(RSEMIB_OK != ret) {
                  if(RSEMIB_OK != undo_rsuReceivedMsgDestPort(index)) { /* UNDO fails nothing we can do. */
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
/* 5.6.2.5 */
static int32_t handle_rsuReceivedMsgProtocol(int32_t index,netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret  = RSEMIB_OK;
  int32_t data = 0;

  DEBUGMSGTL((MY_NAME, "%s: Entry:\n",__FUNCTION__));
  switch (reqinfo->mode){
      case MODE_GET:
          DEBUGMSGTL((MY_NAME, "GET:\n"));
          if(RSEMIB_OK == (ret = get_rsuReceivedMsgProtocol(index,&data))){
              snmp_set_var_typed_integer(requests->requestvb, ASN_INTEGER, data);
          }
          break;
      case MODE_SET_RESERVE1: /* Short stack, all requests done in one. */
          DEBUGMSGTL((MY_NAME, "R1:\n"));
          if(RSEMIB_OK == (ret = preserve_rsuReceivedMsgProtocol(index))){
              if(RSEMIB_OK == (ret = action_rsuReceivedMsgProtocol(index,*(requests->requestvb->val.integer)))){
                  if(RSEMIB_OK == (ret = commit_rsuReceivedMsgProtocol(index))){
                      DEBUGMSGTL((MY_NAME, "COMMIT DONE:\n"));
                  } else {
                      DEBUGMSGTL((MY_NAME, "COMMIT FAIL:\n"));
                  }
              } else {
                  DEBUGMSGTL((MY_NAME, "ACTION FAIL:\n"));
              }
              if(RSEMIB_OK != ret) {
                  if(RSEMIB_OK != undo_rsuReceivedMsgProtocol(index)) { /* UNDO fails nothing we can do. */
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
/* 5.6.2.6 */
static int32_t handle_rsuReceivedMsgRssi(int32_t index,netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret  = RSEMIB_OK;
  int32_t data = 0;

  DEBUGMSGTL((MY_NAME, "%s: Entry:\n",__FUNCTION__));
  switch (reqinfo->mode){
      case MODE_GET:
          DEBUGMSGTL((MY_NAME, "GET:\n"));
          if(RSEMIB_OK == (ret = get_rsuReceivedMsgRssi(index,&data))){
              snmp_set_var_typed_integer(requests->requestvb, ASN_INTEGER, data);
          }
          break;
      case MODE_SET_RESERVE1: /* Short stack, all requests done in one. */
          DEBUGMSGTL((MY_NAME, "R1:\n"));
          if(RSEMIB_OK == (ret = preserve_rsuReceivedMsgRssi(index))){
              if(RSEMIB_OK == (ret = action_rsuReceivedMsgRssi(index,*(requests->requestvb->val.integer)))){
                  if(RSEMIB_OK == (ret = commit_rsuReceivedMsgRssi(index))){
                      DEBUGMSGTL((MY_NAME, "COMMIT DONE:\n"));
                  } else {
                      DEBUGMSGTL((MY_NAME, "COMMIT FAIL:\n"));
                  }
              } else {
                  DEBUGMSGTL((MY_NAME, "ACTION FAIL:\n"));
              }
              if(RSEMIB_OK != ret) {
                  if(RSEMIB_OK != undo_rsuReceivedMsgRssi(index)) { /* UNDO fails nothing we can do. */
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
/* 5.6.2.7 */
static int32_t handle_rsuReceivedMsgInterval(int32_t index,netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret  = RSEMIB_OK;
  int32_t data = 0;

  DEBUGMSGTL((MY_NAME, "%s: Entry:\n",__FUNCTION__));
  switch (reqinfo->mode){
      case MODE_GET:
          DEBUGMSGTL((MY_NAME, "GET:\n"));
          if(RSEMIB_OK == (ret = get_rsuReceivedMsgInterval(index,&data))){
              snmp_set_var_typed_integer(requests->requestvb, ASN_INTEGER, data);
          }
          break;
      case MODE_SET_RESERVE1: /* Short stack, all requests done in one. */
          DEBUGMSGTL((MY_NAME, "R1:\n"));
          if(RSEMIB_OK == (ret = preserve_rsuReceivedMsgInterval(index))){
              if(RSEMIB_OK == (ret = action_rsuReceivedMsgInterval(index,*(requests->requestvb->val.integer)))) {
                  if(RSEMIB_OK == (ret = commit_rsuReceivedMsgInterval(index))){
                      DEBUGMSGTL((MY_NAME, "COMMIT DONE:\n"));
                  } else {
                      DEBUGMSGTL((MY_NAME, "COMMIT FAIL:\n"));
                  }
              } else {
                  DEBUGMSGTL((MY_NAME, "ACTION FAIL:\n"));
              }
              if(RSEMIB_OK != ret) {
                  if(RSEMIB_OK != undo_rsuReceivedMsgInterval(index)) { /* UNDO fails nothing we can do. */
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
/* 5.6.2.8 */
static int32_t handle_rsuReceivedMsgDeliveryStart(int32_t index,netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret = RSEMIB_OK;
  uint8_t data[MIB_DATEANDTIME_LENGTH];

  DEBUGMSGTL((MY_NAME, "%s: Entry:\n",__FUNCTION__));
  switch (reqinfo->mode){
      case MODE_GET:
          DEBUGMSGTL((MY_NAME, "GET:\n"));
          memset(data,0x0,sizeof(data));
          if(0 <= (ret = get_rsuReceivedMsgDeliveryStart(index,data))){
              snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR, data, ret);
              ret = RSEMIB_OK;
          }
          break;
      case MODE_SET_RESERVE1: /* Short stack, all requests done in one. */
          DEBUGMSGTL((MY_NAME, "R1:\n"));
          if(RSEMIB_OK == (ret = preserve_rsuReceivedMsgDeliveryStart(index))){
              if(RSEMIB_OK == (ret = action_rsuReceivedMsgDeliveryStart(index,requests->requestvb->val.string,requests->requestvb->val_len))){
                  if(RSEMIB_OK == (ret = commit_rsuReceivedMsgDeliveryStart(index))){
                      DEBUGMSGTL((MY_NAME, "COMMIT DONE:\n"));
                  } else {
                      DEBUGMSGTL((MY_NAME, "COMMIT FAIL:\n"));
                  }
              } else {
                  DEBUGMSGTL((MY_NAME, "ACTION FAIL:\n"));
              }
              if(RSEMIB_OK != ret) {
                  if(RSEMIB_OK != undo_rsuReceivedMsgDeliveryStart(index)) { /* UNDO fails nothing we can do. */
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
/* 5.6.2.9 */
static int32_t handle_rsuReceivedMsgDeliveryStop(int32_t index,netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret = RSEMIB_OK;
  uint8_t data[MIB_DATEANDTIME_LENGTH];

  DEBUGMSGTL((MY_NAME, "%s: Entry:\n",__FUNCTION__));
  switch (reqinfo->mode){
      case MODE_GET:
          DEBUGMSGTL((MY_NAME, "GET:\n"));
          memset(data,0x0,sizeof(data));
          if(0 <= (ret = get_rsuReceivedMsgDeliveryStop(index,data))){
              snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR, data, ret); 
              ret = RSEMIB_OK;
          }
          break;
      case MODE_SET_RESERVE1: /* Short stack, all requests done in one. */
          DEBUGMSGTL((MY_NAME, "R1:\n"));
          if(RSEMIB_OK == (ret = preserve_rsuReceivedMsgDeliveryStop(index))){
              if(RSEMIB_OK == (ret = action_rsuReceivedMsgDeliveryStop(index,requests->requestvb->val.string,requests->requestvb->val_len))){
                  if(RSEMIB_OK == (ret = commit_rsuReceivedMsgDeliveryStop(index))){
                      DEBUGMSGTL((MY_NAME, "COMMIT DONE:\n"));
                  } else {
                      DEBUGMSGTL((MY_NAME, "COMMIT FAIL:\n"));
                  }
              } else {
                  DEBUGMSGTL((MY_NAME, "ACTION FAIL:\n"));
              }
              if(RSEMIB_OK != ret) {
                  if(RSEMIB_OK != undo_rsuReceivedMsgDeliveryStop(index)) { /* UNDO fails nothing we can do. */
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
/* 5.6.2.10 */
static int32_t handle_rsuReceivedMsgStatus(int32_t index,netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests, int32_t override)
{
  int32_t ret  = RSEMIB_OK;
  int32_t data = 0;
  int32_t row_status = SNMP_ROW_CREATEANDGO; /* If override is set then don't use requests's value. Use this one. */

  DEBUGMSGTL((MY_NAME, "%s: Entry:\n",__FUNCTION__));
  switch (reqinfo->mode){
      case MODE_GET:
          if(RSEMIB_OK == (ret = get_rsuReceivedMsgStatus(index,&data))){
              snmp_set_var_typed_integer(requests->requestvb, ASN_INTEGER, data);
          }
          break;
      case MODE_SET_RESERVE1: /* Short minion stack, all requests done in one. */
          if(0 == override){
              row_status = *(requests->requestvb->val.integer); /* Client index part of CREATE. */
          }
          DEBUGMSGTL((MY_NAME, "R1_BULK_SET: status = %d.\n",row_status));
          if(RSEMIB_OK == (ret = preserve_rsuReceivedMsgStatus(index))){
              if(RSEMIB_OK == (ret = action_rsuReceivedMsgStatus(index,row_status))){
                  if(RSEMIB_OK == (ret = commit_rsuReceivedMsgStatus(index))){
                      DEBUGMSGTL((MY_NAME, "COMMIT DONE:\n")); /* Minions unite, master will be happy! */
                  } else {
                      DEBUGMSGTL((MY_NAME, "COMMIT FAIL:\n"));
                  }
              } else {
                  DEBUGMSGTL((MY_NAME, "ACTION FAIL:\n"));
              }
              if(RSEMIB_OK != ret) { /* UNDO fails nothing we can do. */
                  if(RSEMIB_OK != undo_rsuReceivedMsgStatus(index)) {
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
/* 5.6.2.11 */
static int32_t handle_rsuReceivedMsgSecure(int32_t index,netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret  = RSEMIB_OK;
  int32_t data = 0;

  DEBUGMSGTL((MY_NAME, "%s: Entry:\n",__FUNCTION__));
  switch (reqinfo->mode){
      case MODE_GET:
          DEBUGMSGTL((MY_NAME, "GET:\n"));
          if(RSEMIB_OK == (ret = get_rsuReceivedMsgSecure(index,&data))){
              snmp_set_var_typed_integer(requests->requestvb, ASN_INTEGER, data);
          }
          break;
      case MODE_SET_RESERVE1: /* Short stack, all requests done in one. */
          DEBUGMSGTL((MY_NAME, "R1:\n"));
          if(RSEMIB_OK == (ret = preserve_rsuReceivedMsgSecure(index))){
              if(RSEMIB_OK == (ret = action_rsuReceivedMsgSecure(index,*(requests->requestvb->val.integer)))) {
                  if(RSEMIB_OK == (ret = commit_rsuReceivedMsgSecure(index))){
                      DEBUGMSGTL((MY_NAME, "COMMIT DONE:\n"));
                  } else {
                      DEBUGMSGTL((MY_NAME, "COMMIT FAIL:\n"));
                  }
              } else {
                  DEBUGMSGTL((MY_NAME, "ACTION FAIL:\n"));
              }
              if(RSEMIB_OK != ret) {
                  if(RSEMIB_OK != undo_rsuReceivedMsgSecure(index)) { /* UNDO fails nothing we can do. */
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
/* 5.6.2.12 */
static int32_t handle_rsuReceivedMsgAuthMsgInterval(int32_t index,netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret  = RSEMIB_OK;
  int32_t data = 0;

  DEBUGMSGTL((MY_NAME, "%s: Entry:\n",__FUNCTION__));
  switch (reqinfo->mode){
      case MODE_GET:
          DEBUGMSGTL((MY_NAME, "GET:\n"));
          if(RSEMIB_OK == (ret = get_rsuReceivedMsgAuthMsgInterval(index,&data))){
              snmp_set_var_typed_integer(requests->requestvb, ASN_INTEGER, data);
          }
          break;
      case MODE_SET_RESERVE1: /* Short stack, all requests done in one. */
          DEBUGMSGTL((MY_NAME, "R1:\n"));
          if(RSEMIB_OK == (ret = preserve_rsuReceivedMsgAuthMsgInterval(index))){
              if(RSEMIB_OK == (ret = action_rsuReceivedMsgAuthMsgInterval(index,*(requests->requestvb->val.integer)))) {
                  if(RSEMIB_OK == (ret = commit_rsuReceivedMsgAuthMsgInterval(index))){
                      DEBUGMSGTL((MY_NAME, "COMMIT DONE:\n"));
                  } else {
                      DEBUGMSGTL((MY_NAME, "COMMIT FAIL:\n"));
                  }
              } else {
                  DEBUGMSGTL((MY_NAME, "ACTION FAIL:\n"));
              }
              if(RSEMIB_OK != ret) {
                  if(RSEMIB_OK != undo_rsuReceivedMsgAuthMsgInterval(index)) { /* UNDO fails nothing we can do. */
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
/* 
 * 5.5.2: Table handler: Break it down to smaller requests.
 */
int32_t handle_rsuReceivedMsgTable( netsnmp_mib_handler          * handler
                                  , netsnmp_handler_registration * reginfo
                                  , netsnmp_agent_request_info   * reqinfo
                                  , netsnmp_request_info         * requests)
{
  netsnmp_request_info       * request       = NULL;
  uint32_t                     request_count = 0;
  RsuReceivedMsgEntry_t      * table_entry   = NULL;
  netsnmp_table_request_info * request_info  = NULL;
  int32_t                      ret           = SNMP_ERR_NOERROR;
  uint8_t                      raw_oid[MAX_OID_LEN + 2]; /* If table doesn't exist then helper can't help. Go fish for row index. */
  uint32_t                     row_index = 0; /* For readability. Note: index is 1..max */
  int32_t                      row_status = SNMP_ROW_NONEXISTENT;

  handler = handler; /* Silence of the Warnings. */
  reginfo = reginfo;

  DEBUGMSGTL((MY_NAME, "rsuReceivedMsgTable_handler: Entry:\n"));
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
      if(NULL != (table_entry = (RsuReceivedMsgEntry_t *)netsnmp_tdata_extract_entry(request))) {
          if((request_info->colnum < RX_MSG_INDEX_COLUMN) || (RX_MSG_AUTH_INTERVAL_COLUMN < request_info->colnum)) {
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
      if(RSEMIB_OK != (ret = get_rsuReceivedMsgStatus(row_index,&row_status))) {
          DEBUGMSGTL((MY_NAME, "get_rsuReceivedMsgStatus Failed: ret=%d\n",ret));
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
          if(RX_MSG_STATUS_COLUMN != request_info->colnum){/* If row doesn't exist then create now. */
              ret = handle_rsuReceivedMsgStatus(row_index, reqinfo, request, 1); /* Do not use value in request. Force createAndGo. */
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
          case RX_MSG_INDEX_COLUMN:
              ret = handle_rsuReceivedMsgIndex(row_index, reqinfo, request);
              break;
          case RX_MSG_PSID_COLUMN:
              ret = handle_rsuReceivedMsgPsid(row_index, reqinfo, request);
              break;
          case RX_MSG_DEST_IP_COLUMN:
              ret = handle_rsuReceivedMsgDestIpAddr(row_index, reqinfo, request);
              break;
          case RX_MSG_DEST_PORT_COLUMN:
              ret = handle_rsuReceivedMsgDestPort(row_index, reqinfo, request);
              break;
          case RX_MSG_PROTOCOL_COLUMN:
              ret = handle_rsuReceivedMsgProtocol(row_index, reqinfo, request);
              break;
          case RX_MSG_RSSI_COLUMN:
              ret = handle_rsuReceivedMsgRssi(row_index, reqinfo, request);
              break;
          case RX_MSG_INTERVAL_COLUMN:
              ret = handle_rsuReceivedMsgInterval(row_index, reqinfo, request);
              break;
          case RX_MSG_START_COLUMN:
              ret = handle_rsuReceivedMsgDeliveryStart(row_index, reqinfo, request);
              break;
          case RX_MSG_STOP_COLUMN:
              ret = handle_rsuReceivedMsgDeliveryStop(row_index, reqinfo, request);
              break;
          case RX_MSG_STATUS_COLUMN:
//If already created and status matches then who cares?
              ret = handle_rsuReceivedMsgStatus(row_index, reqinfo, request, 0);
              break;
          case RX_MSG_SECURE_COLUMN:
              ret = handle_rsuReceivedMsgSecure(row_index, reqinfo, request);
              break;
          case RX_MSG_AUTH_INTERVAL_COLUMN:
              ret = handle_rsuReceivedMsgAuthMsgInterval(row_index, reqinfo, request);
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

/* rsuReceivedMsgTable creation functions.*/
STATIC void rsuReceivedMsgTable_removeEntry(netsnmp_tdata * table_data, netsnmp_tdata_row * row)
{
  RsuReceivedMsgEntry_t * entry = NULL;

  if (NULL == row){
      DEBUGMSGTL((MY_NAME,"rsuReceivedMsgTable_removeEntry NULL input.\n"));
      return;
  }
  if (NULL != (entry = (RsuReceivedMsgEntry_t *)row->data)){ /* We allocate, so we free. */
      SNMP_FREE(entry);
  } else {
      DEBUGMSGTL((MY_NAME,"rsuReceivedMsgTable_removeEntry row data is NULL!.\n"));
  }
  if (table_data){
      netsnmp_tdata_remove_and_delete_row(table_data, row);
  } else {
      DEBUGMSGTL((MY_NAME,"rsuReceivedMsgTable_removeEntry NULL input.\n"));
      netsnmp_tdata_delete_row(row); 
  }
  return;
}
static void destroy_rsuReceivedMsgTable(void)
{
  netsnmp_tdata_row  * row     = NULL;
  netsnmp_tdata_row  * nextrow = NULL;
  int32_t              i       = 0;

  if(NULL != rsuReceivedMsgTable){
      if(NULL != (row = netsnmp_tdata_row_first(rsuReceivedMsgTable))){
          for(i=0;(row != NULL) && (i < RSU_RX_MSG_MAX); i++){
              nextrow = netsnmp_tdata_row_next(rsuReceivedMsgTable, row);
              rsuReceivedMsgTable_removeEntry(rsuReceivedMsgTable, row);
              row = nextrow;
          }
      } else {
          DEBUGMSGTL((MY_NAME, "destroy_rsuReceivedMsgTable: Table empty. Nothing to do.\n"));
      }
  } else {
      DEBUGMSGTL((MY_NAME, "destroy_rsuReceivedMsgTable: Table is NULL. Did we get called before install?\n"));
  }
}
static int32_t rsuReceivedMsg_createTable(netsnmp_tdata * table_data, RsuReceivedMsgEntry_t * rsuReceivedMsg)
{
  RsuReceivedMsgEntry_t * entry = NULL;
  netsnmp_tdata_row     * row   = NULL;
  int32_t                 i     = 0;
  int32_t                 valid_row_count = 0;

  if(NULL == table_data){
      DEBUGMSGTL((MY_NAME, "rsuReceivedMsg_createTable FAIL: table_data == NULL.\n"));
      return -1;
  }

  for(i=0; i < RSU_RX_MSG_MAX; i++){
      if((rsuReceivedMsg[i].rsuReceivedMsgIndex < 1) || (RSU_RX_MSG_MAX < rsuReceivedMsg[i].rsuReceivedMsgIndex)){
          return -1; /* Indices must be unique and greater than 1. */
      }
      if(RSEMIB_OK != valid_table_row(rsuReceivedMsg[i].rsuReceivedMsgStatus)){
          continue; /* Only add rows that have beeb created. */
      }
      if (NULL == (entry = SNMP_MALLOC_TYPEDEF(RsuReceivedMsgEntry_t))){
          DEBUGMSGTL((MY_NAME, "rsuReceivedMsg SNMP_MALLOC_TYPEDEF FAIL.\n"));
          return -1;
      }
      if (NULL == (row = netsnmp_tdata_create_row())){
          DEBUGMSGTL((MY_NAME, "rsuReceivedMsg netsnmp_tdata_create_row FAIL.\n"));
          SNMP_FREE(entry);
          return -1;
      }

      DEBUGMSGTL((MY_NAME_EXTRA, "rsuReceivedMsg make row i=%d index=%d.\n",i,rsuReceivedMsg[i].rsuReceivedMsgIndex));
      row->data = entry;
      memcpy(entry,&rsuReceivedMsg[i],sizeof(RsuReceivedMsgEntry_t));
      netsnmp_tdata_row_add_index(row, ASN_INTEGER , &rsuReceivedMsg[i].rsuReceivedMsgIndex, sizeof(int32_t));
      if(SNMPERR_SUCCESS != netsnmp_tdata_add_row(table_data, row)){
          DEBUGMSGTL((MY_NAME, "rsuReceivedMsg netsnmp_tdata_add_row FAIL i=%d.\n",i));
          return -1;
      }
      valid_row_count++;
  } /* for */
  DEBUGMSGTL((MY_NAME, "rsuReceivedMsg valid rows created = %d.\n",valid_row_count));
  return RSEMIB_OK;
}
static int32_t install_rsuReceivedMsgTable(void)
{
    netsnmp_handler_registration * reg = NULL;

    reg = netsnmp_create_handler_registration("rsuReceivedMsgTable"
                                              ,handle_rsuReceivedMsgTable
                                              ,rsuReceivedMsgTable_oid
                                              ,OID_LENGTH(rsuReceivedMsgTable_oid)
                                              ,HANDLER_CAN_RWRITE);

    if (NULL == (rsuReceivedMsgTable = netsnmp_tdata_create_table("rsuReceivedMsgTable", 0))){
        DEBUGMSGTL((MY_NAME, "rsuReceivedMsgTable FAIL.\n"));
        return RSEMIB_INSTALL_HANDLER_FAIL;
    }
    if (NULL == (table_reg_info = SNMP_MALLOC_TYPEDEF(netsnmp_table_registration_info))){
        DEBUGMSGTL((MY_NAME, "SNMP_MALLOC_TYPEDEF FAIL.\n"));
        return RSEMIB_INSTALL_HANDLER_FAIL;
    }
    netsnmp_table_helper_add_indexes(table_reg_info, ASN_INTEGER,0); /* index */                                 

    table_reg_info->min_column = RX_MSG_PSID_COLUMN; /* rsuReceivedMsgPsidd: Hide index by definition. */
    table_reg_info->max_column = RX_MSG_AUTH_INTERVAL_COLUMN; /* rsuReceivedMsgAuthMsgInterval: */

    if (SNMPERR_SUCCESS == netsnmp_tdata_register(reg, rsuReceivedMsgTable, table_reg_info)){
        return RSEMIB_OK; /* Table registered but still empty. rebuild_rsuReceivedMsg() will populate */
    } else {
        DEBUGMSGTL((MY_NAME, "netsnmp_tdata_register FAIL.\n"));
        return RSEMIB_INSTALL_HANDLER_FAIL;
    }
}
/* 
 * Assumed locked already!
 * Live pointer to db.
 * Faster, less safe, version of rebuild function below.
 */
int32_t rebuild_rsuReceivedMsg_live(RsuReceivedMsgEntry_t * rsuReceivedMsg)
{
  if(NULL == rsuReceivedMsg) {
      return RSEMIB_BAD_INPUT;
  }
  /* Remove rows but don't remove table reg. */
  destroy_rsuReceivedMsgTable();

  /* Initialize the contents of the table here.*/
  if (RSEMIB_OK != rsuReceivedMsg_createTable(rsuReceivedMsgTable, rsuReceivedMsg)){
      DEBUGMSGTL((MY_NAME, "rebuild_rsuReceivedMsg_live: ERROR: Null row.\n"));
      set_error_states(RSEMIB_REBUILD_FAIL);
      return RSEMIB_REBUILD_FAIL;
  }
  return RSEMIB_OK;
}
/*
 * Will lock DB. DO NOT call if locked already!
 * Slower but safe version of "live" function above.
 */
static void rebuild_rsuReceivedMsg(void)
{
  int32_t i = 0;
  RsuReceivedMsgEntry_t rsuReceivedMsg[RSU_RX_MSG_MAX];

  memset(rsuReceivedMsg, 0x0, sizeof(rsuReceivedMsg));
  for(i=0; i < RSU_RX_MSG_MAX; i++){
      if(RSEMIB_OK > get_rsuReceivedMsgIndex(i+1,&rsuReceivedMsg[i].rsuReceivedMsgIndex)){
          set_error_states(RSEMIB_REBUILD_FAIL);
          DEBUGMSGTL((MY_NAME, "rsuReceivedMsgIndexinstall rebuild FAIL: i = %d.\n",i));
      }
      if(RSEMIB_OK > get_rsuReceivedMsgPsid(i+1,rsuReceivedMsg[i].rsuReceivedMsgPsid)){
          set_error_states(RSEMIB_REBUILD_FAIL);
          DEBUGMSGTL((MY_NAME, "rsuReceivedMsgPsid rebuild FAIL: i = %d.\n",i));
      }
      if(RSEMIB_OK > get_rsuReceivedMsgDestIpAddr(i+1,rsuReceivedMsg[i].rsuReceivedMsgDestIpAddr)){
          set_error_states(RSEMIB_REBUILD_FAIL);
          DEBUGMSGTL((MY_NAME, "rsuReceivedMsgDestIpAddr rebuild FAIL: i = %d.\n",i));
      }
      if(RSEMIB_OK > get_rsuReceivedMsgDestPort(i+1,&rsuReceivedMsg[i].rsuReceivedMsgDestPort)){
          set_error_states(RSEMIB_REBUILD_FAIL);
          DEBUGMSGTL((MY_NAME, "rsuReceivedMsgDestPort rebuild FAIL: i = %d.\n",i));
      }
      if(RSEMIB_OK > get_rsuReceivedMsgProtocol(i+1,(int32_t *)&rsuReceivedMsg[i].rsuReceivedMsgProtocol)){
          set_error_states(RSEMIB_REBUILD_FAIL);
          DEBUGMSGTL((MY_NAME, "rsuReceivedMsgProtocol rebuild FAIL: i = %d.\n",i));
      }
      if(RSEMIB_OK > get_rsuReceivedMsgRssi(i+1,&rsuReceivedMsg[i].rsuReceivedMsgRssi)){
          set_error_states(RSEMIB_REBUILD_FAIL);
          DEBUGMSGTL((MY_NAME, "rsuReceivedMsgRssi rebuild FAIL: i = %d.\n",i));
      }
      if(RSEMIB_OK > get_rsuReceivedMsgInterval(i+1,&rsuReceivedMsg[i].rsuReceivedMsgInterval)){
          set_error_states(RSEMIB_REBUILD_FAIL);
          DEBUGMSGTL((MY_NAME, "rsuReceivedMsgInterval rebuild FAIL: i = %d.\n",i));
      }
      if(RSEMIB_OK > get_rsuReceivedMsgDeliveryStart(i+1,rsuReceivedMsg[i].rsuReceivedMsgDeliveryStart)){
          set_error_states(RSEMIB_REBUILD_FAIL);
          DEBUGMSGTL((MY_NAME, "rsuReceivedMsgDeliveryStart rebuild FAIL: i = %d.\n",i));
      }
      if(RSEMIB_OK > get_rsuReceivedMsgDeliveryStop(i+1,rsuReceivedMsg[i].rsuReceivedMsgDeliveryStop)){
          set_error_states(RSEMIB_REBUILD_FAIL);
          DEBUGMSGTL((MY_NAME, "rsuReceivedMsgDeliveryStop rebuild FAIL: i = %d.\n",i));
      }
      if(RSEMIB_OK > get_rsuReceivedMsgStatus(i+1,&rsuReceivedMsg[i].rsuReceivedMsgStatus)){
          set_error_states(RSEMIB_REBUILD_FAIL);
          DEBUGMSGTL((MY_NAME, "rsuReceivedMsgStatus rebuild FAIL: i = %d.\n",i));
      }
      if(RSEMIB_OK > get_rsuReceivedMsgSecure(i+1,&rsuReceivedMsg[i].rsuReceivedMsgSecure)){
          set_error_states(RSEMIB_REBUILD_FAIL);
          DEBUGMSGTL((MY_NAME, "rsuReceivedMsgSecure rebuild FAIL: i = %d.\n",i));
      }
      if(RSEMIB_OK > get_rsuReceivedMsgAuthMsgInterval(i+1,&rsuReceivedMsg[i].rsuReceivedMsgAuthMsgInterval)){
          set_error_states(RSEMIB_REBUILD_FAIL);
          DEBUGMSGTL((MY_NAME, "rsuReceivedMsgAuthMsgInterval rebuild FAIL: i = %d.\n",i));
      }
  }
  /* Remove rows but don't remove table reg. */
  destroy_rsuReceivedMsgTable();

  /* Initialize the contents of the table here. */
  if (RSEMIB_OK != rsuReceivedMsg_createTable(rsuReceivedMsgTable, &rsuReceivedMsg[0])){
      DEBUGMSGTL((MY_NAME, "rebuild_rsuReceivedMsg: ERROR: Null row.\n"));
      set_error_states(RSEMIB_REBUILD_FAIL);
  }
}
void install_rsuReceivedMsg_handlers(void)
{
  /* Reset statics across soft reset. */
  rsuReceivedMsgTable         = NULL;
  table_reg_info              = NULL;
  receivedMsg_error_states = 0x0;

  /* 5.6.1 */
  if(MIB_REGISTERED_OK != netsnmp_register_scalar(netsnmp_create_handler_registration
                          ("maxRsuReceivedMsgs", 
                           handle_maxRsuReceivedMsgs,
                           maxRsuReceivedMsgs_oid,
                           OID_LENGTH(maxRsuReceivedMsgs_oid),
                           HANDLER_CAN_RONLY))){
      DEBUGMSGTL((MY_NAME, "maxRsuReceivedMsgs install FAIL.\n"));
      set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
  }
  /* 5.6.2.1 - 5.6.2.12: Install table created with data from MIB. */
  if(RSEMIB_OK != install_rsuReceivedMsgTable()){
      DEBUGMSGTL((MY_NAME, "ReceivedMsgTable install FAIL.\n"));
      set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
  }

  /* Populate the table. */
  rebuild_rsuReceivedMsg();
}
void destroy_rsuReceivedMsg(void)
{
  destroy_rsuReceivedMsgTable();

  /* Free table info. */
  if (NULL != table_reg_info){
      SNMP_FREE(table_reg_info);
  }
  /* Free table reg. */
  if (NULL != rsuReceivedMsgTable){
      SNMP_FREE(rsuReceivedMsgTable);
  }
}
int32_t set_fwdmsgMask(int32_t i, uint8_t mask)
{
  int32_t  ret = RSEMIB_OK;
  char_t   request_buf[128]; /* If you need more then increase for your needs. */
  char_t   reply_buf[16];
  FILE    *fp = NULL;
  int32_t  fwdmsg_mask = 0x0;
  size_t   len = 0;

  if(RSEMIB_OK == ret) {
      memset(reply_buf,'\0',sizeof(reply_buf));
      memset(request_buf,'\0',sizeof(request_buf));
      fwdmsg_mask = 0x0;
      fp = NULL;
      len = 0;

      switch(i) {
      case 1:
          snprintf(request_buf,sizeof(request_buf), DCU_WRITE_FWDMSG_MASK_1,mask);
          break;
      case 2:
          snprintf(request_buf,sizeof(request_buf), DCU_WRITE_FWDMSG_MASK_2,mask);
          break;
      case 3:
          snprintf(request_buf,sizeof(request_buf), DCU_WRITE_FWDMSG_MASK_3,mask);
          break;
      case 4:
          snprintf(request_buf,sizeof(request_buf), DCU_WRITE_FWDMSG_MASK_4,mask);
          break;
      case 5:
          snprintf(request_buf,sizeof(request_buf), DCU_WRITE_FWDMSG_MASK_5,mask);
          break;
      default:
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"set_fwdmsgMask: RSEMIB_BAD_INDEX: Bad index(%d)\n",i);
          ret = RSEMIB_BAD_INDEX;
          break;
      }
  }
  if(RSEMIB_OK == ret) {
      if(NULL != (fp = popen(request_buf, "r"))){
          if(NULL != fgets(reply_buf, sizeof(reply_buf), fp)){
              len = strnlen(reply_buf,sizeof(reply_buf));
              if((0 != len) && (len <= sizeof(reply_buf))){
                  reply_buf[len-1] = '\0'; /* delete extra char added. */
              } else { 
                  reply_buf[0] = '\0'; /* force scanf to fail */
              }
              if(1 != sscanf(reply_buf,DCU_WRITE_FWDMSG_MASK_REPLY, &fwdmsg_mask)) {
                  I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"set_fwdmsgMask: SSCANF failed(%s)\n",strerror(errno));
                  ret = RSEMIB_SYSCALL_FAIL;
              }
          } else {
              I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"set_fwdmsgMask: FGETS failed(%s)\n",strerror(errno));
              ret = RSEMIB_SYSCALL_FAIL;
          }
          pclose(fp);
      } else {
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"set_fwdmsgMask: popen failed(%s)\n",strerror(errno));
          ret = RSEMIB_SYSCALL_FAIL;
      }
  }
  if(RSEMIB_OK == ret) {
     if(-1 == fwdmsg_mask) {
         ret = RSEMIB_BAD_DATA;
         I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"set_fwdmsgMask: RSEMIB_BAD_DATA: i(%d) mask(0x%x)\n",i,mask);
     }
  }
  if(ret < RSEMIB_OK)
      set_error_states(ret);
  return ret;
}
int32_t set_fwdmsgPort(int32_t i, uint16_t port)
{
  int32_t  ret = RSEMIB_OK;
  char_t   request_buf[128]; /* If you need more then increase for your needs. */
  char_t   reply_buf[16];
  FILE    *fp = NULL;
  int32_t  fwdmsg_port = 0x0;
  size_t   len = 0;

  if(RSEMIB_OK == ret) {
      memset(reply_buf,'\0',sizeof(reply_buf));
      memset(request_buf,'\0',sizeof(request_buf));
      fwdmsg_port = 0x0;
      fp = NULL;
      len = 0;

      switch(i) {
      case 1:
          snprintf(request_buf,sizeof(request_buf), DCU_WRITE_FWDMSG_PORT_1,port);
          break;
      case 2:
          snprintf(request_buf,sizeof(request_buf), DCU_WRITE_FWDMSG_PORT_2,port);
          break;
      case 3:
          snprintf(request_buf,sizeof(request_buf), DCU_WRITE_FWDMSG_PORT_3,port);
          break;
      case 4:
          snprintf(request_buf,sizeof(request_buf), DCU_WRITE_FWDMSG_PORT_4,port);
          break;
      case 5:
          snprintf(request_buf,sizeof(request_buf), DCU_WRITE_FWDMSG_PORT_5,port);
          break;
      default:
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"set_fwdmsgPort: RSEMIB_BAD_INDEX: Bad index(%d)\n",i);
          ret = RSEMIB_BAD_INDEX;
          break;
      }
  }
  if(RSEMIB_OK == ret) {
      if(NULL != (fp = popen(request_buf, "r"))){
          if(NULL != fgets(reply_buf, sizeof(reply_buf), fp)){
              len = strnlen(reply_buf,sizeof(reply_buf));
              if((0 != len) && (len <= sizeof(reply_buf))){
                  reply_buf[len-1] = '\0'; /* delete extra char added. */
              } else { 
                  reply_buf[0] = '\0'; /* force scanf to fail */
              }
              if(1 != sscanf(reply_buf,DCU_WRITE_FWDMSG_PORT_REPLY , &fwdmsg_port)) {
                  I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"set_fwdmsgPort: SSCANF failed(%s)\n",strerror(errno));
                  ret = RSEMIB_SYSCALL_FAIL;
              }
          } else {
              I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"set_fwdmsgPort: FGETS failed(%s)\n",strerror(errno));
              ret = RSEMIB_SYSCALL_FAIL;
          }
          pclose(fp);
      } else {
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"set_fwdmsgPort: popen failed(%s)\n",strerror(errno));
          ret = RSEMIB_SYSCALL_FAIL;
      }
  }
  if(RSEMIB_OK == ret) {
     if(-1 == fwdmsg_port) {
         ret = RSEMIB_BAD_DATA;
         I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"set_fwdmsgPort: RSEMIB_BAD_DATA: i(%d) port(%u)\n",i,port);
     }
  }
  if(ret < RSEMIB_OK)
      set_error_states(ret);
  return ret;
}
static int32_t get_fwdmsgPort(int32_t i, uint16_t * port)
{
  int32_t  ret = RSEMIB_OK;
  char_t   request_buf[128]; /* If you need more then increase for your needs. */
  char_t   reply_buf[16];
  FILE    *fp = NULL;
  uint32_t fwdmsg_port = 0x0;
  size_t   len = 0;

  if(NULL == port) {
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"get_fwdmsgPort: RSEMIB_BAD_INPUT: port(NULL).\n");
      ret = RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ret) {
      memset(reply_buf,'\0',sizeof(reply_buf));
      memset(request_buf,'\0',sizeof(request_buf));
      fwdmsg_port = 0x0;
      fp = NULL;
      len = 0;

      switch(i) {
      case 1:
          snprintf(request_buf,sizeof(request_buf), DCU_READ_FWDMSG_PORT_1);
          break;
      case 2:
          snprintf(request_buf,sizeof(request_buf), DCU_READ_FWDMSG_PORT_2);
          break;
      case 3:
          snprintf(request_buf,sizeof(request_buf), DCU_READ_FWDMSG_PORT_3);
          break;
      case 4:
          snprintf(request_buf,sizeof(request_buf), DCU_READ_FWDMSG_PORT_4);
          break;
      case 5:
          snprintf(request_buf,sizeof(request_buf), DCU_READ_FWDMSG_PORT_5);
          break;
      default:
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"get_fwdmsgPort: RSEMIB_BAD_INDEX: Bad index(%d)\n",i);
          ret = RSEMIB_BAD_INDEX;
          break;
      }
  }
  if(RSEMIB_OK == ret) {
      if(NULL != (fp = popen(request_buf, "r"))){
          if(NULL != fgets(reply_buf, sizeof(reply_buf), fp)){
              len = strnlen(reply_buf,sizeof(reply_buf));
              if((0 != len) && (len <= sizeof(reply_buf))){
                  reply_buf[len-1] = '\0'; /* delete extra char added. */
              } else { 
                  reply_buf[0] = '\0'; /* force scanf to fail */
              }
              if(1 != sscanf(reply_buf,DCU_READ_FWDMSG_PORT_REPLY , &fwdmsg_port)) {
                  I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"get_fwdmsgPort: SSCANF failed(%s)\n",strerror(errno));
                  ret = RSEMIB_SYSCALL_FAIL;
              }
          } else {
              I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"get_fwdmsgPort: FGETS failed(%s)\n",strerror(errno));
              ret = RSEMIB_SYSCALL_FAIL;
          }
          pclose(fp);
      } else {
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"get_fwdmsgPort: popen failed(%s)\n",strerror(errno));
          ret = RSEMIB_SYSCALL_FAIL;
      }
  }
  if(RSEMIB_OK == ret) {
     if((fwdmsg_port < RSU_RX_MSG_PORT_MIN) || (RSU_RX_MSG_PORT_MAX < fwdmsg_port)){
         ret = RSEMIB_BAD_DATA;
         I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"get_fwdmsgPort: RSEMIB_BAD_DATA: i(%d) port(%u)\n",i, fwdmsg_port);
     } else {
         *port = fwdmsg_port;
     }
  }
  if(ret < RSEMIB_OK)
      set_error_states(ret);
  return ret;
}
int32_t set_fwdmsgEnable(int32_t i, int32_t enable)
{
  int32_t   ret = RSEMIB_OK;
  FILE    * fp = NULL;
  char_t    request_buf[128]; /* If you need more then increase for your needs. */
  char_t    replyBuf[64]; /* Size according to your needs. */
  int32_t   result = 0;

  fp = NULL;
  memset(request_buf,'\0',sizeof(request_buf));
  memset(replyBuf,'\0',sizeof(replyBuf));
  switch(i) {
      case 1:
          snprintf(request_buf,sizeof(request_buf),DCU_WRITE_FWDMSG_ENABLE_1,enable);
          break;
      case 2:
          snprintf(request_buf,sizeof(request_buf),DCU_WRITE_FWDMSG_ENABLE_2,enable);
          break;
      case 3:
          snprintf(request_buf,sizeof(request_buf),DCU_WRITE_FWDMSG_ENABLE_3,enable);
          break;
      case 4:
          snprintf(request_buf,sizeof(request_buf),DCU_WRITE_FWDMSG_ENABLE_4,enable);
          break;
      case 5:
          snprintf(request_buf,sizeof(request_buf),DCU_WRITE_FWDMSG_ENABLE_5,enable);
          break;
      default:
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"set_fwdmsgEnable: RSEMIB_BAD_INPUT: i(%d)\n",i);
          ret = RSEMIB_BAD_INPUT;
          break;
  }

  if(RSEMIB_OK == ret){
      if(NULL != (fp = popen(request_buf, "r"))){
          if(NULL != fgets(replyBuf,sizeof(replyBuf), fp)){
              i = strnlen(replyBuf,sizeof(replyBuf));
              if((0 < i) && ((uint32_t)i <= sizeof(replyBuf))){
                  replyBuf[i-1] = '\0'; /* delete extra char added. */
              } else { 
                  replyBuf[0] = '\0'; /* force scanf to fail */
              }
              if(1 == sscanf(replyBuf,DCU_WRITE_FWDMSG_ENABLE_REPLY,&result)) {
                  if(-1 == result){
                      I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"set_fwdmsgEnable: RSEMIB_BAD_DATA: i(%d) enabled(%d)\n",i,result);
                      ret = RSEMIB_BAD_DATA;
                  }
              } else {
                  I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"set_fwdmsgEnable: RSEMIB_SYSCALL_FAIL: scanf failed errno(%s)\n",strerror(errno));
                  ret = RSEMIB_SYSCALL_FAIL;
              }
          } else {
              I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"set_fwdmsgEnable: RSEMIB_SYSCALL_FAIL: fgets failed errno(%s)\n",strerror(errno));
              ret = RSEMIB_SYSCALL_FAIL;
          }
          pclose(fp);
      } else {
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"set_fwdmsgEnable: RSEMIB_SYSCALL_FAIL: popen failed errno(%s)\n",strerror(errno));
          ret = RSEMIB_SYSCALL_FAIL;
      }
  }
  if(ret < RSEMIB_OK) {
      set_error_states(ret);
  }
  return ret;
}
static int32_t get_fwdmsgEnable(int32_t i)
{
  int32_t   ret = RSEMIB_OK;
  FILE    * fp = NULL;
  char_t    request_buf[128]; /* If you need more then increase for your needs. */
  char_t    replyBuf[64]; /* Size according to your needs. */
  int32_t   enabled = 0;

  fp = NULL;
  memset(request_buf,'\0',sizeof(request_buf));
  memset(replyBuf,'\0',sizeof(replyBuf));
  switch(i) {
      case 1:
          strncpy(request_buf,DCU_READ_FWDMSG_ENABLE_1,sizeof(request_buf));
          break;
      case 2:
          strncpy(request_buf,DCU_READ_FWDMSG_ENABLE_2,sizeof(request_buf));
          break;
      case 3:
          strncpy(request_buf,DCU_READ_FWDMSG_ENABLE_3,sizeof(request_buf));
          break;
      case 4:
          strncpy(request_buf,DCU_READ_FWDMSG_ENABLE_4,sizeof(request_buf));
          break;
      case 5:
          strncpy(request_buf,DCU_READ_FWDMSG_ENABLE_5,sizeof(request_buf));
          break;
      default:
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"get_fwdmsgEnable: RSEMIB_BAD_INPUT: i(%d)\n",i);
          ret = RSEMIB_BAD_INPUT;
          break;
  }

  if(RSEMIB_OK == ret){
      if(NULL != (fp = popen(request_buf, "r"))){
          if(NULL != fgets(replyBuf,sizeof(replyBuf), fp)){
              i = strnlen(replyBuf,sizeof(replyBuf));
              if((0 < i) && ((uint32_t)i <= sizeof(replyBuf))){
                  replyBuf[i-1] = '\0'; /* delete extra char added. */
              } else { 
                  replyBuf[0] = '\0'; /* force scanf to fail */
              }
              if(1 == sscanf(replyBuf,DCU_READ_FWDMSG_ENABLE_REPLY,&enabled)) {
                  if((0 != enabled) && (1 != enabled)){
                      I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"get_fwdmsgEnable: RSEMIB_BAD_DATA: enabled(%d)\n",enabled);
                      ret = RSEMIB_BAD_DATA;
                  }
              } else {
                  I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"get_fwdmsgEnable: RSEMIB_SYSCALL_FAIL: scanf failed errno(%s)\n",strerror(errno));
                  ret = RSEMIB_SYSCALL_FAIL;
              }
          } else {
              I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"get_fwdmsgEnable: RSEMIB_SYSCALL_FAIL: fgets failed errno(%s)\n",strerror(errno));
              ret = RSEMIB_SYSCALL_FAIL;
          }
          pclose(fp);
      } else {
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"get_fwdmsgEnable: RSEMIB_SYSCALL_FAIL: popen failed errno(%s)\n",strerror(errno));
          ret = RSEMIB_SYSCALL_FAIL;
      }
  }
  if(ret < RSEMIB_OK) {
      set_error_states(ret);
      enabled = ret;
  }
  return enabled;
}
int32_t set_fwdmsgIP(int32_t i, uint8_t * IpAddr)
{
  int32_t   ret = RSEMIB_OK;
  FILE    * fp = NULL;
  char_t    request_buf[128]; 
  char_t    replyBuf[RSU_DEST_IP_MAX+1]; /* Size according to your needs. */

  if(NULL == IpAddr){
      ret = RSEMIB_BAD_INPUT;
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"set_fwdmsgIP: RSEMIB_BAD_INPUT: NULL input.\n");
  } else {
      fp = NULL;
      memset(replyBuf,'\0',sizeof(replyBuf));
      memset(request_buf,'\0',sizeof(request_buf));
      switch(i) {
          case 1:
              snprintf(request_buf,sizeof(request_buf),DCU_WRITE_FWDMSG_IP_1,IpAddr);
              break;
          case 2:
              snprintf(request_buf,sizeof(request_buf),DCU_WRITE_FWDMSG_IP_2,IpAddr);
              break;
          case 3:
              snprintf(request_buf,sizeof(request_buf),DCU_WRITE_FWDMSG_IP_3,IpAddr);
              break;
          case 4:
              snprintf(request_buf,sizeof(request_buf),DCU_WRITE_FWDMSG_IP_4,IpAddr);
              break;
          case 5:
              snprintf(request_buf,sizeof(request_buf),DCU_WRITE_FWDMSG_IP_5,IpAddr);
              break;
          default:
              I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"set_fwdmsgIP: RSEMIB_BAD_INPUT: i(%d)\n",i);
              ret = RSEMIB_BAD_INPUT;
              break;
      }
  }
  if(RSEMIB_OK == ret){
      if(NULL != (fp = popen(request_buf, "r"))){
          if(NULL != fgets(replyBuf,sizeof(replyBuf), fp)){
              i = strnlen(replyBuf,sizeof(replyBuf));
              if((0 < i) && ((uint32_t)i <= sizeof(replyBuf))){
                  replyBuf[i-1] = '\0'; /* delete extra char added. */
              }
              if(NULL != strstr(replyBuf,"conf_agent: Error:")) {
                  I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"set_fwdmsgIP: RSEMIB_BAD_DATA: i(%u) ipaddr(%s)\n",i,IpAddr);
                  ret = RSEMIB_BAD_DATA;
              }
          } else {
              I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"set_fwdmsgIP: RSEMIB_SYSCALL_FAIL: fgets failed errno(%s)\n",strerror(errno));
              ret = RSEMIB_SYSCALL_FAIL;
          }
          pclose(fp);
      } else {
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"set_fwdmsgIP: RSEMIB_SYSCALL_FAIL: popen failed errno(%s)\n",strerror(errno));
          ret = RSEMIB_SYSCALL_FAIL;
      }
  }
  if(ret < RSEMIB_OK) {
      set_error_states(ret);
  }
  return ret;
}
static int32_t get_fwdmsgIP(int32_t i, uint8_t * IpAddr, int32_t * IpAddr_length)
{
  int32_t   ret = RSEMIB_OK;
  FILE    * fp = NULL;
  char_t    request_buf[128]; 
  char_t    replyBuf[RSU_DEST_IP_MAX+1]; /* Size according to your needs. */

  if((NULL == IpAddr) || (NULL == IpAddr_length)){
      ret = RSEMIB_BAD_INPUT;
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"get_fwdmsgIP: RSEMIB_BAD_INPUT: NULL input.\n");
  } else {
      fp = NULL;
      memset(replyBuf,'\0',sizeof(replyBuf));
      memset(request_buf,'\0',sizeof(request_buf));
      switch(i) {
          case 1:
              strncpy(request_buf,DCU_READ_FWDMSG_IP_1,sizeof(request_buf));
              break;
          case 2:
              strncpy(request_buf,DCU_READ_FWDMSG_IP_2,sizeof(request_buf));
              break;
          case 3:
              strncpy(request_buf,DCU_READ_FWDMSG_IP_3,sizeof(request_buf));
              break;
          case 4:
              strncpy(request_buf,DCU_READ_FWDMSG_IP_4,sizeof(request_buf));
              break;
          case 5:
              strncpy(request_buf,DCU_READ_FWDMSG_IP_5,sizeof(request_buf));
              break;
          default:
              I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"get_fwdmsgIP: RSEMIB_BAD_INPUT: i(%d)\n",i);
              ret = RSEMIB_BAD_INPUT;
              break;
      }
  }
  if(RSEMIB_OK == ret){
      if(NULL != (fp = popen(request_buf, "r"))){
          if(NULL != fgets(replyBuf,sizeof(replyBuf), fp)){
              i = strnlen(replyBuf,sizeof(replyBuf));
              if((0 < i) && ((uint32_t)i <= sizeof(replyBuf))){
                  replyBuf[i-1] = '\0'; /* delete extra char added. */
                  memcpy(IpAddr,replyBuf,RSU_DEST_IP_MAX);
                  *IpAddr_length = i-1;
              } else {
                  I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"get_fwdmsgIP: RSEMIB_BAD_DATA: id addr wrong length(%u)\n",i);
                  ret = RSEMIB_BAD_DATA;
              }
          } else {
              I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"get_fwdmsgIP: RSEMIB_SYSCALL_FAIL: fgets failed errno(%s)\n",strerror(errno));
              ret = RSEMIB_SYSCALL_FAIL;
          }
          pclose(fp);
      } else {
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"get_fwdmsgIP: RSEMIB_SYSCALL_FAIL: popen failed errno(%s)\n",strerror(errno));
          ret = RSEMIB_SYSCALL_FAIL;
      }
  }
  if(ret < RSEMIB_OK) {
      set_error_states(ret);
  }
  return ret;
}
static int32_t get_rsuIP(uint8_t * IP)
{
  int32_t ret = RSEMIB_OK;
  char_t  request_buf[150]; /* If you need more then increase for your needs. */
  char_t  reply_buf[RSU_DEST_IP_MAX+16]; /* Give some rope to see if we get bogus length */
  FILE   *fp;
  size_t  ip_length = 0;

  if(NULL == IP) {
      ret = RSEMIB_BAD_INPUT;
  } else { 
      memset(reply_buf,'\0',sizeof(reply_buf));
      snprintf(request_buf,sizeof(request_buf), DCU_READ_RSU_IP);
      if(NULL != (fp = popen(request_buf, "r"))){
          if(NULL != fgets(reply_buf, sizeof(reply_buf), fp)){
              ip_length = strnlen(reply_buf, sizeof(reply_buf));
              if(ip_length <= RSU_DEST_IP_MAX) {
                  memcpy(IP, reply_buf,ip_length);
                  ret = RSEMIB_OK;
              } else {
                  ret = RSEMIB_BAD_DATA;
              }
          } else {
              DEBUGMSGTL((MY_NAME, "get_rsuIP: READ fgets failed.\n"));
              ret = RSEMIB_SYSCALL_FAIL;
          }
          pclose(fp);
      } else {
          DEBUGMSGTL((MY_NAME, "get_rsuIP: READ request failed.\n"));
          ret = RSEMIB_SYSCALL_FAIL;
      }
  }
  if(ret < RSEMIB_OK)
      set_error_states(ret);
  return ret;
}
static int32_t get_scsIP(uint8_t * IP)
{
  int32_t ret = RSEMIB_OK;
  char_t  request_buf[150]; /* If you need more then increase for your needs. */
  char_t  reply_buf[RSU_DEST_IP_MAX+16]; /* Give some rope to see if we get bogus length */
  FILE   *fp;
  size_t  ip_length = 0;

  if(NULL == IP) {
      ret = RSEMIB_BAD_INPUT;
  } else { 
      memset(reply_buf,'\0',sizeof(reply_buf));
      snprintf(request_buf,sizeof(request_buf), DCU_READ_SCS_IP);
      if(NULL != (fp = popen(request_buf, "r"))){
          if(NULL != fgets(reply_buf, sizeof(reply_buf), fp)){
              ip_length = strnlen(reply_buf, sizeof(reply_buf));
              if(ip_length <= RSU_DEST_IP_MAX) {
                  memcpy(IP, reply_buf,ip_length);
                  ret = RSEMIB_OK;
              } else {
                  ret = RSEMIB_BAD_DATA;
              }
          } else {
              DEBUGMSGTL((MY_NAME, "get_scsIP: READ fgets failed.\n"));
              ret = RSEMIB_SYSCALL_FAIL;
          }
          pclose(fp);
      } else {
          DEBUGMSGTL((MY_NAME, "get_scsIP: READ request failed.\n"));
          ret = RSEMIB_SYSCALL_FAIL;
      }
  }
  if(ret < RSEMIB_OK)
      set_error_states(ret);
  return ret;
}
//TODO: ipv4 only.
//TODO: conf_manager is likely not exhaustive: SYSLOG, IFCLOG have ip's do we care?
static int32_t ip_conflict(uint8_t * ip)
{
  uint8_t scsIP[RSU_DEST_IP_MAX];
  uint8_t rsuIP[RSU_DEST_IP_MAX];

  if(NULL == ip){
      return RSEMIB_BAD_INPUT;
  }
  /* RSU. Only care about IP. */
  if(0 > get_rsuIP(rsuIP)){
      DEBUGMSGTL((MY_NAME, "ip_conflict: get_rsuIP failed.\n"));
      set_error_states(RSEMIB_SYSCALL_FAIL);
      return RSEMIB_SYSCALL_FAIL;
  }
  if(0 == memcmp(ip, rsuIP, RSU_DEST_IP_MAX)) {
      DEBUGMSGTL((MY_NAME, "ip_conflict: RSU conflict.\n"));
      set_error_states(RX_MSG_IP_PORT_CONFLICT);
      return RX_MSG_IP_PORT_CONFLICT;
  }
  /* SCS. Only care about IP.*/
  if(0 > get_scsIP(scsIP)){
      DEBUGMSGTL((MY_NAME, "ip_conflict: get_scsIP failed.\n"));
      set_error_states(RSEMIB_SYSCALL_FAIL);
      return RSEMIB_SYSCALL_FAIL;
  }
  if(0 == memcmp(ip, scsIP, RSU_DEST_IP_MAX)) {
      DEBUGMSGTL((MY_NAME, "ip_conflict: SCS conflict.\n"));
      set_error_states(RX_MSG_IP_PORT_CONFLICT);
      return RX_MSG_IP_PORT_CONFLICT;
  }
  return RSEMIB_OK;
}
/* SHM is not locked. */
static int32_t update_receivedMsg(RsuReceivedMsgEntry_t * ReceivedMsg)
{
  int32_t  i; /* For readability. */
  uint16_t port = 0x0;
  uint8_t  IpAddr[RSU_DEST_IP_MAX];
  int32_t  IpAddr_length = 0;
  int32_t  ret = RSEMIB_OK;

  if(NULL == ReceivedMsg) {
      DEBUGMSGTL((MY_NAME, "update_receivedMsg: NULL == ReceivedMsg.\n"));
      set_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;    
  }
  i = ReceivedMsg->rsuReceivedMsgIndex - 1; /* Index starts at 1 not 0 */
  if((i < 0) || (RSU_RX_MSG_MAX <= i)){
      DEBUGMSGTL((MY_NAME, "update_receivedMsg: RSEMIB_BAD_INDEX.\n"));
      set_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }

  memset(IpAddr,0x0,sizeof(IpAddr));
  IpAddr_length = 0;
  port = 0x0;

  if(1 == get_fwdmsgEnable(i+1)) {
      ReceivedMsg->rsuReceivedMsgStatus = SNMP_ROW_ACTIVE;
      if(RSEMIB_OK == (ret = get_fwdmsgPort(i+1, &port))) {
          ReceivedMsg->rsuReceivedMsgDestPort = port;
      } else {
          return ret; /* Error handling and debug handled in function. */
      }
      if(RSEMIB_OK == (ret = get_fwdmsgIP(i+1,IpAddr,&IpAddr_length))) {
          if(RSEMIB_OK != (ip_conflict(IpAddr))) {
              I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"RX_MSG_IP_PORT_CONFLICT: index(%d) ipaddr(%s)\n",i,IpAddr);
              set_error_states(RX_MSG_IP_PORT_CONFLICT);
              return RX_MSG_IP_PORT_CONFLICT;
          } else {
              memcpy(ReceivedMsg->rsuReceivedMsgDestIpAddr,IpAddr, RSU_DEST_IP_MAX);
              ReceivedMsg->rsuReceivedMsgDestIpAddr_length = IpAddr_length;
          }
      } else {
          return ret; /* Error handling and debug handled in function. */
      }
  }
  return RSEMIB_OK;
}
/* RSU App thread support for remainder of module. */
static void force_receivedMsg_update(void) 
{
  int32_t i;

  /* One time load from MIB to SHM. After that only client update will trigger update on a single row. */
  for(i=0;i<RSU_RX_MSG_MAX; i++){
      if(RSEMIB_OK != update_receivedMsg(&ntcip_1218_mib.rsuReceivedMsgTable[i])){
         set_error_states(RSEMIB_SHM_SEED_FAILED);
      }
  }
}
void *receivedMsgThr(void __attribute__((unused)) *arg)
{
  int32_t i = 0;

  DEBUGMSGTL((MY_NAME,"receivedMsgThr: Entry.\n"));
  for(i=0;(i<WAIT_BOOT_TIME) && mainloop;i++) /* Give RSU apps a chance to catch up before updating SHM. */
      sleep(1);
  force_receivedMsg_update(); /* Force MIB update of rows recovered from FWDMSG. */
  rebuild_rsuReceivedMsg();   /* Rebuild table in handler. */
  while(mainloop) { /* Forever check start/stop times. */
      sleep(1);
  }
  DEBUGMSGTL((MY_NAME,"receivedMsgThr: Exit."));
  pthread_exit(NULL);
}
