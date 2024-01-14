/**************************************************************************
 *                                                                        *
 *     File Name:  rsuMsgRepeat.c                                         *
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
#define MY_NAME       "MsgRepeat"  /* i.e. MIBS=ALL snmpd  -f -V -Lo -Dntcip_1218,MsgRepeat -C -c /mnt/rwflash/snmpd.conf */
#define MY_NAME_EXTRA "XMsgRepeat" /* Will create lots of output so use cautiously. */
/* Mandatory: For debugging and syslog output. */
#define MY_I2V_NAME    MY_NAME
#define MY_ERR_LEVEL   LEVEL_PRIV  /* from i2v_util.h */

/* table column definitions. */
#define MSG_REPEAT_INDEX_COLUMN       1
#define MSG_REPEAT_PSID_COLUMN        2
#define MSG_REPEAT_TX_CHANNEL_COLUMN  3
#define MSG_REPEAT_TX_INTERVAL_COLUMN 4
#define MSG_REPEAT_START_COLUMN       5
#define MSG_REPEAT_STOP_COLUMN        6
#define MSG_REPEAT_PAYLOAD_COLUMN     7
#define MSG_REPEAT_ENABLE_COLUMN      8
#define MSG_REPEAT_STATUS_COLUMN      9
#define MSG_REPEAT_PRIORITY_COLUMN    10
#define MSG_REPEAT_OPTIONS_COLUMN     11
 /* Theoritical column ids. Make sure they follow in order with columns and unique. */
#define MSG_REPEAT_MAX_MSG_OID        12
#define MSG_REPEAT_DELETE_ALL_OID     13

#define MSG_REPEAT_INDEX_NAME       "rsuMsgRepeatIndex"
#define MSG_REPEAT_PSID_NAME        "rsuMsgRepeatPsid"
#define MSG_REPEAT_TX_CHANNEL_NAME  "rsuMsgRepeatTxChannel"
#define MSG_REPEAT_TX_INTERVAL_NAME "rsuMsgRepeatTxInterval"
#define MSG_REPEAT_START_NAME       "rsuMsgRepeatDeliveryStart"
#define MSG_REPEAT_STOP_NAME        "rsuMsgRepeatDeliveryStop"
#define MSG_REPEAT_PAYLOAD_NAME     "rsuMsgRepeatPayload"
#define MSG_REPEAT_ENABLE_NAME      "rsuMsgRepeatEnable"
#define MSG_REPEAT_STATUS_NAME      "rsuMsgRepeatStatus"
#define MSG_REPEAT_PRIORITY_NAME    "rsuMsgRepeatPriority"
#define MSG_REPEAT_OPTIONS_NAME     "rsuMsgRepeatOptions"
#define MSG_REPEAT_MAX_MSG_NAME     "maxRsuMsgRepeat"
#define MSG_REPEAT_DELETE_ALL_NAME  "rsuMsgRepeatDeleteAll"

/****************************************************************************** 
 * 5.4 Store and Repeat Messages: { rsu 3 }: rsuMsgRepeatStatusTable
 *     Shared with AMH and i2v_types.h 
 * 
 * typedef struct {
 *     int32_t  rsuMsgRepeatIndex;                                 RO: 1 to 255 SAR messages.
 *     uint8_t  rsuMsgRepeatPsid[RSU_RADIO_PSID_SIZE];             RC: 1 to 4 OCTETS:
 *     int32_t  rsuMsgRepeatTxChannel;                             RC: RSU_RADIO_CHANNEL_MIN to RSU_RADIO_CHANNEL_MAX.
 *     int32_t  rsuMsgRepeatTxInterval;                            RC: msecs between tx: 1 to 0x7fffffff. 2 seconds seems slow?
 *     uint8_t  rsuMsgRepeatDeliveryStart[MIB_DATEANDTIME_LENGTH]; RC: "2d-1d-1d,1d:1d:1d.1d": 8 OCTETS: DateAndTime: SNMPv2-TC
 *     uint8_t  rsuMsgRepeatDeliveryStop[MIB_DATEANDTIME_LENGTH];  RC: Same as start above.
 *     uint8_t  rsuMsgRepeatPayload[RSU_MSG_PAYLOAD_MAX];          RC: 0..2302 OCTETS:IEEE1609dot3-MIB. UPER MessageFrame.       
 *     onOff_e  rsuMsgRepeatEnable;                                RC: 0=off, 1=on tx: 
 *     int32_t  rsuMsgRepeatStatus;                                RC: Create (4) & (6) destroy:
 *     int32_t  rsuMsgRepeatPriority;                              RC: (0..63) : DSRC see IEEE 1609.3-2016.
 *     uint8_t  rsuMsgRepeatOptions;   RC: bitmask: 32 bits
 *                                       bit0(0x0 = bypass, use THEIR 1609.2, 0x1 = use OUR 1609.2)
 *                                       bit1(0x0 = secure(Sign or Encrypt), 0x1 = unsecure) 
 *                                       bit2(0x0 = continue tx, 0x1 = stop when > rsuShortCommLossTime: default 0)
 *                                       bit3(0x0 = continue tx, 0x1 = stop when > rsuLongtCommLossTime: default 0)
 * } RsuMsgRepeatStatusEntry_t;
 *
 ******************************************************************************/

const oid maxRsuMsgRepeat_oid[]         = {1, 3, 6, 1, 4, 1, 1206, 4, 2, 18, 3 , 1};
const oid rsuMsgRepeatStatusTable_oid[] = {1, 3, 6, 1, 4, 1, 1206, 4, 2, 18, 3 , 2};
const oid rsuMsgRepeatDeleteAll_oid[]   = {1, 3, 6, 1, 4, 1, 1206, 4, 2, 18, 3 , 3};

static netsnmp_table_registration_info * table_reg_info = NULL; /* snmp table reg pointer. */
static netsnmp_tdata * rsuMsgRepeatStatusTable          = NULL; /* snmp table pointer. */
static uint32_t rsuMsgRepeat_error_states               = 0x0;  /* local error state. */

/*
 * Protos: Silence of the Warnings.
 */

/* 
 * Function Bodies.
 */
static void set_error_states(int32_t error)
{
  /* -1 to -32 */
  if((error < 0) && (error > MIB_ERROR_BASE_2)){
      rsuMsgRepeat_error_states |= (uint32_t)(0x1) << (abs(error) - 1);
  }
}
uint32_t get_rsuMsgRepeat_error_states(void)
{
  return rsuMsgRepeat_error_states;
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
  int32_t do_trap = 0;
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
      case MSG_REPEAT_INDEX_COLUMN:
          strcpy(column_name,MSG_REPEAT_INDEX_NAME);
          break;
      case MSG_REPEAT_PSID_COLUMN:
          strcpy(column_name,MSG_REPEAT_PSID_NAME);
          break;
      case MSG_REPEAT_TX_CHANNEL_COLUMN:
          strcpy(column_name,MSG_REPEAT_TX_CHANNEL_NAME);
          break;
      case MSG_REPEAT_TX_INTERVAL_COLUMN:
          strcpy(column_name,MSG_REPEAT_TX_INTERVAL_NAME);
          break;
      case MSG_REPEAT_START_COLUMN:
          strcpy(column_name,MSG_REPEAT_START_NAME);
          break;
      case MSG_REPEAT_STOP_COLUMN:
          strcpy(column_name,MSG_REPEAT_STOP_NAME);
          break;
      case MSG_REPEAT_PAYLOAD_COLUMN:
          strcpy(column_name,MSG_REPEAT_PAYLOAD_NAME);
          break;
      case MSG_REPEAT_ENABLE_COLUMN:
          strcpy(column_name,MSG_REPEAT_ENABLE_NAME);
          break;
      case MSG_REPEAT_STATUS_COLUMN:
          strcpy(column_name,MSG_REPEAT_STATUS_NAME);
          break;
      case MSG_REPEAT_PRIORITY_COLUMN:
          strcpy(column_name,MSG_REPEAT_PRIORITY_NAME);
          break;
      case MSG_REPEAT_OPTIONS_COLUMN:
          strcpy(column_name,MSG_REPEAT_OPTIONS_NAME);
          break;
/* For scalars and like kind. */
      case MSG_REPEAT_MAX_MSG_OID:
          strcpy(column_name,MSG_REPEAT_MAX_MSG_NAME);
          break;
      case MSG_REPEAT_DELETE_ALL_OID:
          strcpy(column_name,MSG_REPEAT_DELETE_ALL_NAME);
          break;
      default:
          strcpy(column_name,"UNKOWN");
          break;
  }
  if(RSEMIB_OK == ret) {
      strcpy(passOrFail,"succeeded");
      do_trap = 0;
  } else {
      strcpy(passOrFail,"failed");
      do_trap = 1;
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
  if(1 == do_trap) {
      set_messageFileIntegrity(rsuAlertLevel_error,(uint8_t *)alert_msg,RSU_ALERT_MSG_LENGTH_MAX);
  }
}

/*
 * OID Handlers
 */

/* 5.4.1 */
static int32_t handle_maxRsuMsgRepeat(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo,
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
              if(0 <= (ret = get_maxRsuMsgRepeat())){
                  snmp_set_var_typed_integer(requests->requestvb, ASN_INTEGER, ret);
                  ret = RSEMIB_OK;
              }
              break;
          default:
              ret = RSEMIB_BAD_MODE;
              break;
      }
      /* scalars and like are always index 0 with unqiue column.*/
      print_request_result(0, MSG_REPEAT_MAX_MSG_OID,requests,reqinfo->mode,ret);
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
/* 5.4.2.1: Although stored, we already know the 'index'. Already ranged checked beforehand. */
static int32_t handle_rsuMsgRepeatIndex(int32_t index, netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret  = RSEMIB_OK;
  int32_t data = index;

  DEBUGMSGTL((MY_NAME, "%s: Entry:\n",__FUNCTION__));
  switch (reqinfo->mode){
      case MODE_GET:
#if 0
          if(RSEMIB_OK == (ret = get_rsuMsgRepeatIndex(index, &data)))
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
/* 5.4.2.2 */
static int32_t handle_rsuMsgRepeatPsid(int32_t index, netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret = RSEMIB_OK;
  uint8_t data[RSU_RADIO_PSID_SIZE];

  DEBUGMSGTL((MY_NAME, "%s: Entry:\n",__FUNCTION__));
  switch (reqinfo->mode){
      case MODE_GET:
          DEBUGMSGTL((MY_NAME, "GET:\n"));
          memset(data,0x0,sizeof(data));
          if(0 <= (ret = get_rsuMsgRepeatPsid(index,data))){
              snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR, data, ret);
              ret = RSEMIB_OK;
          }
          break;
      case MODE_SET_RESERVE1: /* Short stack, all requests done in one. */
          DEBUGMSGTL((MY_NAME, "R1:\n"));
          if(RSEMIB_OK == (ret = preserve_rsuMsgRepeatPsid(index))){
              if(RSEMIB_OK == (ret = action_rsuMsgRepeatPsid(index,requests->requestvb->val.string,requests->requestvb->val_len))){
                  if(RSEMIB_OK == (ret = commit_rsuMsgRepeatPsid(index))){
                      DEBUGMSGTL((MY_NAME, "COMMIT DONE:\n"));
                  } else {
                      DEBUGMSGTL((MY_NAME, "COMMIT FAIL:\n"));
                  }
              } else {
                  DEBUGMSGTL((MY_NAME, "ACTION FAIL:\n"));
              }
              if(RSEMIB_OK != ret) {
                  if(RSEMIB_OK != undo_rsuMsgRepeatPsid(index)) { /* UNDO fails nothing we can do. */
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
/* 5.4.2.3 */
int32_t handle_rsuMsgRepeatTxChannel(int32_t index,netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret  = RSEMIB_OK;
  int32_t data = 0;

  DEBUGMSGTL((MY_NAME, "%s: Entry:\n",__FUNCTION__));
  switch (reqinfo->mode){
      case MODE_GET:
          DEBUGMSGTL((MY_NAME, "GET:\n"));
          if(RSEMIB_OK == (ret = get_rsuMsgRepeatTxChannel(index,&data))){
              snmp_set_var_typed_integer(requests->requestvb, ASN_INTEGER, data);
          }
          break;
      case MODE_SET_RESERVE1:
          DEBUGMSGTL((MY_NAME, "R1: TxChannel=%lu.\n",*(requests->requestvb->val.integer)));
          if(RSEMIB_OK == (ret = preserve_rsuMsgRepeatTxChannel(index))){
              if(RSEMIB_OK == (ret = action_rsuMsgRepeatTxChannel(index,*(requests->requestvb->val.integer)))){
                  if(RSEMIB_OK == (ret = commit_rsuMsgRepeatTxChannel(index))){
                      DEBUGMSGTL((MY_NAME, "COMMIT DONE:\n"));
                  } else {
                      DEBUGMSGTL((MY_NAME, "COMMIT FAIL:\n"));
                  }
              } else {
                  DEBUGMSGTL((MY_NAME, "ACTION FAIL:\n"));
              }
              if(RSEMIB_OK != ret) {
                  if(RSEMIB_OK != undo_rsuMsgRepeatTxChannel(index)) { /* UNDO fails nothing we can do. */
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
/* 5.4.2.4 */
static int32_t handle_rsuMsgRepeatTxInterval(int32_t index,netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret  = RSEMIB_OK;
  int32_t data = 0;

  DEBUGMSGTL((MY_NAME, "%s: Entry:\n",__FUNCTION__));
  switch (reqinfo->mode){
      case MODE_GET:
          DEBUGMSGTL((MY_NAME, "GET:\n"));
          if(RSEMIB_OK == (ret = get_rsuMsgRepeatTxInterval(index,&data))){
              snmp_set_var_typed_integer(requests->requestvb, ASN_INTEGER, data);
          }
          break;
      case MODE_SET_RESERVE1:
          DEBUGMSGTL((MY_NAME, "R1:\n"));
          if(RSEMIB_OK == (ret = preserve_rsuMsgRepeatTxInterval(index))){
              if(RSEMIB_OK == (ret = action_rsuMsgRepeatTxInterval(index,*(requests->requestvb->val.integer)))){
                  if(RSEMIB_OK == (ret = commit_rsuMsgRepeatTxInterval(index))){
                      DEBUGMSGTL((MY_NAME, "COMMIT DONE:\n"));
                  } else {
                      DEBUGMSGTL((MY_NAME, "COMMIT FAIL:\n"));
                  }
              } else {
                  DEBUGMSGTL((MY_NAME, "ACTION FAIL:\n"));
              }
              if(RSEMIB_OK != ret) {
                  if(RSEMIB_OK != undo_rsuMsgRepeatTxInterval(index)) { /* UNDO fails nothing we can do. */
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
/* 5.4.2.5 */
static int32_t handle_rsuMsgRepeatDeliveryStart(int32_t index,netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret  = RSEMIB_OK;
  uint8_t data[MIB_DATEANDTIME_LENGTH];

  DEBUGMSGTL((MY_NAME, "%s: Entry:\n",__FUNCTION__));
  switch (reqinfo->mode){
      case MODE_GET:
          DEBUGMSGTL((MY_NAME, "GET:\n"));
          memset(data,0x0,sizeof(data));
          if(0 <= (ret = get_rsuMsgRepeatDeliveryStart(index,data))){
              snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR, data, ret); 
              ret = RSEMIB_OK;
          }
          break;
      case MODE_SET_RESERVE1:
          DEBUGMSGTL((MY_NAME, "R1:\n"));
          if(RSEMIB_OK == (ret = preserve_rsuMsgRepeatDeliveryStart(index))){
              if(RSEMIB_OK == (ret = action_rsuMsgRepeatDeliveryStart(index,requests->requestvb->val.string,requests->requestvb->val_len))){
                  if(RSEMIB_OK == (ret = commit_rsuMsgRepeatDeliveryStart(index))){
                      DEBUGMSGTL((MY_NAME, "COMMIT DONE:\n"));
                  } else {
                      DEBUGMSGTL((MY_NAME, "COMMIT FAIL:\n"));
                  }
              } else {
                  DEBUGMSGTL((MY_NAME, "ACTION FAIL:\n"));
              }
              if(RSEMIB_OK != ret) {
                  if(RSEMIB_OK != undo_rsuMsgRepeatDeliveryStart(index)) { /* UNDO fails nothing we can do. */
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
/* 5.4.2.6 */
static int32_t handle_rsuMsgRepeatDeliveryStop(int32_t index,netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret = RSEMIB_OK;
  uint8_t data[MIB_DATEANDTIME_LENGTH];

  DEBUGMSGTL((MY_NAME, "%s: Entry:\n",__FUNCTION__));
  switch (reqinfo->mode){
      case MODE_GET:
          DEBUGMSGTL((MY_NAME, "GET:\n"));
          memset(data,0x0,sizeof(data));
          if(0 <= (ret = get_rsuMsgRepeatDeliveryStop(index,data))){
              snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR, data, ret); 
              ret = RSEMIB_OK;
          }
          break;
      case MODE_SET_RESERVE1:
          DEBUGMSGTL((MY_NAME, "R1:\n"));
          if(RSEMIB_OK == (ret = preserve_rsuMsgRepeatDeliveryStop(index))){
              if(RSEMIB_OK == (ret = action_rsuMsgRepeatDeliveryStop(index,requests->requestvb->val.string,requests->requestvb->val_len))){
                  if(RSEMIB_OK == (ret = commit_rsuMsgRepeatDeliveryStop(index))){
                      DEBUGMSGTL((MY_NAME, "COMMIT DONE:\n"));
                  } else {
                      DEBUGMSGTL((MY_NAME, "COMMIT FAIL:\n"));
                  }
              } else {
                  DEBUGMSGTL((MY_NAME, "ACTION FAIL:\n"));
              }
              if(RSEMIB_OK != ret) {
                  if(RSEMIB_OK != undo_rsuMsgRepeatDeliveryStop(index)) { /* UNDO fails nothing we can do. */
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
/* 5.4.2.7 */
static int32_t handle_rsuMsgRepeatPayload(int32_t index,netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret = RSEMIB_OK;
  uint8_t data[RSU_MSG_PAYLOAD_MAX];

  DEBUGMSGTL((MY_NAME, "%s: Entry:\n",__FUNCTION__));
  switch (reqinfo->mode){
      case MODE_GET:
          DEBUGMSGTL((MY_NAME, "GET:\n"));
          memset(data,0x0,sizeof(data));
          if(0 <= (ret = get_rsuMsgRepeatPayload(index,data))){
              snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR, data, ret); 
              ret = RSEMIB_OK;
          }
          break;
      case MODE_SET_RESERVE1:
          DEBUGMSGTL((MY_NAME, "R1:\n"));
          if(RSEMIB_OK == (ret = preserve_rsuMsgRepeatPayload(index))){
              if(RSEMIB_OK == (ret = action_rsuMsgRepeatPayload(index,requests->requestvb->val.string,requests->requestvb->val_len))){
                  if(RSEMIB_OK == (ret = commit_rsuMsgRepeatPayload(index))){
                      DEBUGMSGTL((MY_NAME, "COMMIT DONE:\n"));
                  } else {
                      DEBUGMSGTL((MY_NAME, "COMMIT FAIL:\n"));
                  }
              } else {
                  DEBUGMSGTL((MY_NAME, "ACTION FAIL:\n"));
              }
              if(RSEMIB_OK != ret) {
                  if(RSEMIB_OK != undo_rsuMsgRepeatPayload(index)) { /* UNDO fails nothing we can do. */
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
/* 5.4.2.8 */
static int32_t handle_rsuMsgRepeatEnable(int32_t index,netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret  = RSEMIB_OK;
  int32_t data = 0;

  DEBUGMSGTL((MY_NAME, "%s: Entry:\n",__FUNCTION__));
  switch (reqinfo->mode){
      case MODE_GET:
          DEBUGMSGTL((MY_NAME, "GET:\n"));
          if(RSEMIB_OK == (ret = get_rsuMsgRepeatEnable(index,&data))){
              snmp_set_var_typed_integer(requests->requestvb, ASN_INTEGER, data);
          }
          break;
      case MODE_SET_RESERVE1:
          DEBUGMSGTL((MY_NAME, "R1:\n"));
          if(RSEMIB_OK == (ret = preserve_rsuMsgRepeatEnable(index))){
              if(RSEMIB_OK == (ret = action_rsuMsgRepeatEnable(index,*(requests->requestvb->val.integer)))){
                  if(RSEMIB_OK == (ret = commit_rsuMsgRepeatEnable(index))){
                      DEBUGMSGTL((MY_NAME, "COMMIT DONE:\n"));
                  } else {
                      DEBUGMSGTL((MY_NAME, "COMMIT FAIL:\n"));
                  }
              } else {
                  DEBUGMSGTL((MY_NAME, "ACTION FAIL:\n"));
              }
              if(RSEMIB_OK != ret) {
                  if(RSEMIB_OK != undo_rsuMsgRepeatEnable(index)) { /* UNDO fails nothing we can do. */
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
/* 5.4.2.9 */
static int32_t handle_rsuMsgRepeatStatus(int32_t index,netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests, int32_t override)
{
  int32_t ret  = RSEMIB_OK;
  int32_t data = 0;
  int32_t row_status = SNMP_ROW_CREATEANDGO; /* If override is set then don't use requests's value. Use this one. */

  DEBUGMSGTL((MY_NAME, "%s: Entry:\n",__FUNCTION__));
  switch (reqinfo->mode){
      case MODE_GET:
          if(RSEMIB_OK == (ret = get_rsuMsgRepeatStatus(index,&data))){
              snmp_set_var_typed_integer(requests->requestvb, ASN_INTEGER, data);
          }
          break;
      case MODE_SET_RESERVE1: /* Short minion stack, all requests done in one. */
          if(0 == override){
              row_status = *(requests->requestvb->val.integer); /* Client index part of CREATE. */
          }
          DEBUGMSGTL((MY_NAME, "R1_BULK_SET: status = %d.\n",row_status));
          if(RSEMIB_OK == (ret = preserve_rsuMsgRepeatStatus(index))){
              if(RSEMIB_OK == (ret = action_rsuMsgRepeatStatus(index,row_status))){
                  if(RSEMIB_OK == (ret = commit_rsuMsgRepeatStatus(index))){
                      DEBUGMSGTL((MY_NAME, "COMMIT DONE:\n")); /* Minions unite, master will be happy! */
                  } else {
                      DEBUGMSGTL((MY_NAME, "COMMIT FAIL:\n"));
                  }
              } else {
                  DEBUGMSGTL((MY_NAME, "ACTION FAIL:\n"));
              }
              if(RSEMIB_OK != ret) { /* UNDO fails nothing we can do. */
                  if(RSEMIB_OK != undo_rsuMsgRepeatStatus(index)) {
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
/* 5.4.2.10 */
static int32_t handle_rsuMsgRepeatPriority(int32_t index,netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret  = RSEMIB_OK;
  int32_t data = 0;

  DEBUGMSGTL((MY_NAME, "%s: Entry:\n",__FUNCTION__));
  switch (reqinfo->mode){
      case MODE_GET:
          DEBUGMSGTL((MY_NAME, "GET:\n"));
          if(RSEMIB_OK == (ret = get_rsuMsgRepeatPriority(index,&data))){
              snmp_set_var_typed_integer(requests->requestvb, ASN_INTEGER, data);
          }
          break;
      case MODE_SET_RESERVE1:
          DEBUGMSGTL((MY_NAME, "R1:\n"));
          if(RSEMIB_OK == (ret = preserve_rsuMsgRepeatPriority(index))){
              if(RSEMIB_OK == (ret = action_rsuMsgRepeatPriority(index,*(requests->requestvb->val.integer)))){
                  if(RSEMIB_OK == (ret = commit_rsuMsgRepeatPriority(index))){
                      DEBUGMSGTL((MY_NAME, "COMMIT DONE:\n"));
                  } else {
                      DEBUGMSGTL((MY_NAME, "COMMIT FAIL:\n"));
                  }
              } else {
                  DEBUGMSGTL((MY_NAME, "ACTION FAIL:\n"));
              }
              if(RSEMIB_OK != ret) {
                  if(RSEMIB_OK != undo_rsuMsgRepeatPriority(index)) { /* UNDO fails nothing we can do. */
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
/* 5.4.2.11 */
static int32_t handle_rsuMsgRepeatOptions(int32_t index,netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret = RSEMIB_OK;
  uint8_t data[RSU_MSG_REPEAT_OPTIONS_LENGTH];

  DEBUGMSGTL((MY_NAME, "%s: Entry:\n",__FUNCTION__));
  switch (reqinfo->mode){
      case MODE_GET:
          DEBUGMSGTL((MY_NAME, "GET:\n"));
          if(RSU_MSG_REPEAT_OPTIONS_LENGTH == (ret = get_rsuMsgRepeatOptions(index,data))){
#if defined(ENABLE_ASN_BIT_STR)
              snmp_set_var_typed_value(requests->requestvb, ASN_BIT_STR, data, RSU_MSG_REPEAT_OPTIONS_LENGTH);
#else
              snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR, data, RSU_MSG_REPEAT_OPTIONS_LENGTH);
#endif
              ret = RSEMIB_OK;
          }
          break;
      case MODE_SET_RESERVE1: /* Short stack, all requests done in one. */
          DEBUGMSGTL((MY_NAME, "R1:\n"));
          if(RSEMIB_OK == (ret = preserve_rsuMsgRepeatOptions(index))){
              if(RSEMIB_OK == (ret = action_rsuMsgRepeatOptions(index,requests->requestvb->val.bitstring,requests->requestvb->val_len))){
                  if(RSEMIB_OK == (ret = commit_rsuMsgRepeatOptions(index))){
                      DEBUGMSGTL((MY_NAME, "COMMIT DONE:\n"));
                  } else {
                      DEBUGMSGTL((MY_NAME, "COMMIT FAIL:\n"));
                  }
              } else {
                  DEBUGMSGTL((MY_NAME, "ACTION FAIL:\n"));
              }
              if(RSEMIB_OK != ret) {
                  if(RSEMIB_OK != undo_rsuMsgRepeatOptions(index)) { /* UNDO fails nothing we can do. */
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
 * 5.4.2: Table handler: Break it down to smaller requests.
 */
int32_t handle_rsuMsgRepeatStatusTable( netsnmp_mib_handler          * handler
                                      , netsnmp_handler_registration * reginfo
                                      , netsnmp_agent_request_info   * reqinfo
                                      , netsnmp_request_info         * requests)
{
  netsnmp_request_info       * request       = NULL;
  uint32_t                     request_count = 0;
  RsuMsgRepeatStatusEntry_t  * table_entry   = NULL;
  netsnmp_table_request_info * request_info  = NULL;
  int32_t                      ret           = SNMP_ERR_NOERROR;
  uint8_t                      raw_oid[MAX_OID_LEN + 2]; /* If table doesn't exist then helper can't help. Go fish for row index. */
  uint32_t                     row_index = 0; /* For readability. Note: index is 1..max */
  int32_t                      row_status = SNMP_ROW_NONEXISTENT;

  handler = handler; /* Silence of the Warnings. */
  reginfo = reginfo;

  DEBUGMSGTL((MY_NAME, "rsuMsgRepeatStatusTable_handler: Entry:\n"));
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
      if(NULL != (table_entry = (RsuMsgRepeatStatusEntry_t *)netsnmp_tdata_extract_entry(request))) {
          if((request_info->colnum < MSG_REPEAT_INDEX_COLUMN) || (MSG_REPEAT_OPTIONS_COLUMN < request_info->colnum)) {
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
      if(RSEMIB_OK != (ret = get_rsuMsgRepeatStatus(row_index,&row_status))) {
          DEBUGMSGTL((MY_NAME, "get_rsuMsgRepeatStatus Failed: ret=%d\n",ret));
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
          if(MSG_REPEAT_STATUS_COLUMN != request_info->colnum){/* If row doesn't exist then create now. */
              ret = handle_rsuMsgRepeatStatus(row_index, reqinfo, request, 1); /* Do not use value in request. Force createAndGo. */
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
          case MSG_REPEAT_INDEX_COLUMN:
              ret = handle_rsuMsgRepeatIndex(row_index, reqinfo, request);
              break;
          case MSG_REPEAT_PSID_COLUMN:
              ret = handle_rsuMsgRepeatPsid(row_index, reqinfo, request);
              break;
          case MSG_REPEAT_TX_CHANNEL_COLUMN:
              ret = handle_rsuMsgRepeatTxChannel(row_index, reqinfo, request);
              break;
          case MSG_REPEAT_TX_INTERVAL_COLUMN:
              ret = handle_rsuMsgRepeatTxInterval(row_index, reqinfo, request);
              break;
          case MSG_REPEAT_START_COLUMN:
              ret = handle_rsuMsgRepeatDeliveryStart(row_index, reqinfo, request);
              break;
          case MSG_REPEAT_STOP_COLUMN:
              ret = handle_rsuMsgRepeatDeliveryStop(row_index, reqinfo, request);
              break;
          case MSG_REPEAT_PAYLOAD_COLUMN:
              ret = handle_rsuMsgRepeatPayload(row_index, reqinfo, request);
              break;
          case MSG_REPEAT_ENABLE_COLUMN:
              ret = handle_rsuMsgRepeatEnable(row_index, reqinfo, request);
              break;
          case MSG_REPEAT_STATUS_COLUMN:
//If already created and status matches then who cares?
              ret = handle_rsuMsgRepeatStatus(row_index, reqinfo, request, 0);
              break;
          case MSG_REPEAT_PRIORITY_COLUMN:
              ret = handle_rsuMsgRepeatPriority(row_index, reqinfo, request);
              break;
          case MSG_REPEAT_OPTIONS_COLUMN:
              ret = handle_rsuMsgRepeatOptions(row_index, reqinfo, request);
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
/* 5.4.3 */
int32_t handle_rsuMsgRepeatDeleteAll(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo,
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
              if(0 <= (ret = get_rsuMsgRepeatDeleteAll())){
                  snmp_set_var_typed_integer(requests->requestvb, ASN_INTEGER, ret);
                  ret = RSEMIB_OK;
              }
              break;
          case MODE_SET_RESERVE1:
              if(RSEMIB_OK == (ret = preserve_rsuMsgRepeatDeleteAll())){
                  if(RSEMIB_OK == (ret = action_rsuMsgRepeatDeleteAll(*(requests->requestvb->val.integer)))){
                      if(RSEMIB_OK == (ret = commit_rsuMsgRepeatDeleteAll())){
                          DEBUGMSGTL((MY_NAME, "COMMIT DONE:\n"));
                      } else {
                          DEBUGMSGTL((MY_NAME, "COMMIT FAIL:\n"));
                      }
                  } else {
                      DEBUGMSGTL((MY_NAME, "ACTION FAIL:\n"));
                  }
                  if(RSEMIB_OK != ret) { /* UNDO fails nothing we can do. */
                      if(RSEMIB_OK != undo_rsuMsgRepeatDeleteAll()) {
                          DEBUGMSGTL((MY_NAME, "UNDO FAIL:\n"));
                      }
                  }
              } else { /* If PRESERVE fails, don't call UNDO. */
                  DEBUGMSGTL((MY_NAME, "PRESERVE FAIL:\n"));
              } 
              if(RSEMIB_OK != ret) {
                  DEBUGMSGTL((MY_NAME, "R1_SET FAILED ret=%d.\n", ret));
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
      /* scalars and like are always index 0 with unqiue column.*/
      print_request_result(0, MSG_REPEAT_DELETE_ALL_OID,requests,reqinfo->mode,ret);
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

/* 
 * Table Creation.
 */

/* rsuMsgRepeatStatusTable creation functions. */
STATIC void rsuMsgRepeatStatusTable_removeEntry(netsnmp_tdata * table_data, netsnmp_tdata_row * row)
{
  RsuMsgRepeatStatusEntry_t * entry = NULL;

  if (NULL == row){
      DEBUGMSGTL((MY_NAME,"rsuMsgRepeatStatusTable_removeEntry NULL input.\n"));
      return;
  }
  if (NULL != (entry = (RsuMsgRepeatStatusEntry_t *)row->data)){ /* We allocate, so we free. */
      SNMP_FREE(entry);
  } else {
      DEBUGMSGTL((MY_NAME,"rsuRepeatStatusTable_removeEntry row data is NULL!.\n"));
  }
  if (table_data){
      netsnmp_tdata_remove_and_delete_row(table_data, row);
  } else {
      DEBUGMSGTL((MY_NAME,"rsuMsgRepeatStatusTable_removeEntry NULL input.\n"));
      netsnmp_tdata_delete_row(row); 
  }
  return;
}
void destroy_rsuMsgRepeatStatusTable(void)
{
  netsnmp_tdata_row  * row     = NULL;
  netsnmp_tdata_row  * nextrow = NULL;
  int32_t              i       = 0;

  if(NULL != rsuMsgRepeatStatusTable){
      if(NULL != (row = netsnmp_tdata_row_first(rsuMsgRepeatStatusTable))){
          for(i=0;(row != NULL) && (i < RSU_SAR_MESSAGE_MAX);i++){
              nextrow = netsnmp_tdata_row_next(rsuMsgRepeatStatusTable, row);
              rsuMsgRepeatStatusTable_removeEntry(rsuMsgRepeatStatusTable, row);
              row = nextrow;
          }
      } else {
          DEBUGMSGTL((MY_NAME, "destroy_MsgRepeatStatusTable: Table empty. Nothing to do.\n"));
      }
  } else {
      DEBUGMSGTL((MY_NAME, "destroy_MsgRepeatStatusTable: Table is NULL. Did we get called before install?\n"));
  }
}
static int32_t rsuMsgRepeat_createTable(netsnmp_tdata * table_data, RsuMsgRepeatStatusEntry_t * rsuMsgRepeat)
{
  RsuMsgRepeatStatusEntry_t * entry    = NULL;
  netsnmp_tdata_row         * row      = NULL;
  int32_t                     i        = 0;
  int32_t                     valid_row_count = 0;

  if(NULL == table_data){
      DEBUGMSGTL((MY_NAME, "rsuMsgRepeat_createTable FAIL: table_data == NULL.\n"));
      return -1;
  }

  for(i=0; i < RSU_SAR_MESSAGE_MAX; i++){
      if((rsuMsgRepeat[i].rsuMsgRepeatIndex < 1) || (RSU_SAR_MESSAGE_MAX < rsuMsgRepeat[i].rsuMsgRepeatIndex)){
          handle_rsemib_errors(RSEMIB_BAD_INDEX);
          return -1; /* Indices must be unique and greater than 1. */
      }
      if(RSEMIB_OK != valid_table_row(rsuMsgRepeat[i].rsuMsgRepeatStatus)){
          continue; /* Only add rows that have beeb created. */
      }
      if (NULL == (entry = SNMP_MALLOC_TYPEDEF(RsuMsgRepeatStatusEntry_t))){
          DEBUGMSGTL((MY_NAME, "rsuMsgRepeat SNMP_MALLOC_TYPEDEF FAILED.\n"));
          handle_rsemib_errors(RSEMIB_BAD_MODE);
          return -1;
      }
      if (NULL == (row = netsnmp_tdata_create_row())){
          DEBUGMSGTL((MY_NAME, "rsuMsgRepeat netsnmp_tdata_create_row FAILED.\n"));
          SNMP_FREE(entry);
          handle_rsemib_errors(RSEMIB_BAD_MODE);
          return -1;
      }

      DEBUGMSGTL((MY_NAME_EXTRA, "rsuMsgRepeat make row i=%d index=%d.\n",i,rsuMsgRepeat[i].rsuMsgRepeatIndex));
      row->data = entry;
      memcpy(entry,&rsuMsgRepeat[i],sizeof(RsuMsgRepeatStatusEntry_t));
      netsnmp_tdata_row_add_index(row, ASN_INTEGER , &rsuMsgRepeat[i].rsuMsgRepeatIndex, sizeof(int32_t));
      if(SNMPERR_SUCCESS != netsnmp_tdata_add_row(table_data, row)){
          DEBUGMSGTL((MY_NAME, "rsuMsgRepeat netsnmp_tdata_add_row FAILED i=%d.\n",i));
          handle_rsemib_errors(RSEMIB_BAD_MODE);
          return -1;
      }
      valid_row_count++;
  } /* for */
  DEBUGMSGTL((MY_NAME, "rsuMsgRepeat valid rows created = %d.\n",valid_row_count));
  return RSEMIB_OK;
}
static int32_t install_rsuMsgRepeatStatusTable(void)
{
    netsnmp_handler_registration * reg = NULL;

    reg = netsnmp_create_handler_registration("rsuMsgRepeatStatusTable"
                                              ,handle_rsuMsgRepeatStatusTable
                                              ,rsuMsgRepeatStatusTable_oid
                                              ,OID_LENGTH(rsuMsgRepeatStatusTable_oid)
                                              ,HANDLER_CAN_RWRITE);

    if (NULL == (rsuMsgRepeatStatusTable = netsnmp_tdata_create_table("rsuMsgRepeatStatusTable", 0))){
        DEBUGMSGTL((MY_NAME, "rsuMsgRepeatStatusTable FAILED.\n"));
        return RSEMIB_INSTALL_HANDLER_FAIL;
    }
    if (NULL == (table_reg_info = SNMP_MALLOC_TYPEDEF(netsnmp_table_registration_info))){
        DEBUGMSGTL((MY_NAME, "SNMP_MALLOC_TYPEDEF FAILED.\n"));
        return RSEMIB_INSTALL_HANDLER_FAIL;
    }
    netsnmp_table_helper_add_indexes(table_reg_info, ASN_INTEGER,0); /* index */                                 

    table_reg_info->min_column = MSG_REPEAT_PSID_COLUMN;    /* rsuMsgRepeatPsid: Hide index by definition. */
    table_reg_info->max_column = MSG_REPEAT_OPTIONS_COLUMN; /* rsuMsgRepeatOptions: */

    if (SNMPERR_SUCCESS == netsnmp_tdata_register(reg, rsuMsgRepeatStatusTable, table_reg_info)){
        return RSEMIB_OK; /* Table registered but still empty. rebuild_MsgRepeat() will populate */
    } else {
        DEBUGMSGTL((MY_NAME, "netsnmp_tdata_register FAILED.\n"));
        return RSEMIB_INSTALL_HANDLER_FAIL;
    }
}
int32_t rebuild_rsuMsgRepeat(RsuMsgRepeatStatusEntry_t * rsuMsgRepeat)
{
  if(NULL == rsuMsgRepeat) {
      return RSEMIB_BAD_INPUT;
  }
  /* Remove rows but don't remove table reg. */
  destroy_rsuMsgRepeatStatusTable();

  /* Initialize the contents of the table here.*/
  if (RSEMIB_OK != rsuMsgRepeat_createTable(rsuMsgRepeatStatusTable, rsuMsgRepeat)){
      DEBUGMSGTL((MY_NAME, "rsuMsgRepeat_createTable: Fail.\n"));
      set_error_states(RSEMIB_REBUILD_FAIL);
      return RSEMIB_REBUILD_FAIL;
  }
  return RSEMIB_OK;
}
void install_rsuMsgRepeat_handlers(void)
{
  int32_t i = 0;
  RsuMsgRepeatStatusEntry_t * live_rsuMsgRepeat = NULL;
  RsuMsgRepeatStatusEntry_t   rsuMsgRepeat[RSU_SAR_MESSAGE_MAX]; /* local copy for verification. */

  /* Reset statics across soft reset. */
  rsuMsgRepeatStatusTable   = NULL;
  table_reg_info            = NULL;
  rsuMsgRepeat_error_states = 0x0;

  /* 5.4.1 */
  if(MIB_REGISTERED_OK != netsnmp_register_scalar(netsnmp_create_handler_registration
                          ("maxRsuMsgRepeat", 
                           handle_maxRsuMsgRepeat,
                           maxRsuMsgRepeat_oid,
                           OID_LENGTH(maxRsuMsgRepeat_oid),
                           HANDLER_CAN_RONLY))){
      DEBUGMSGTL((MY_NAME, "maxRsuMsgRepeat install FAIL.\n"));
      set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
  }
  /* 5.4.2.1 - 5.4.2.11 : Create and register table helper. Not populated yet with any rows. */
  if(RSEMIB_OK != install_rsuMsgRepeatStatusTable()){
      DEBUGMSGTL((MY_NAME, "rsuMsgRepeatStatusTable install FAIL.\n"));
      set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
  }
  /* 5.4.3 */
  if(MIB_REGISTERED_OK != netsnmp_register_scalar(netsnmp_create_handler_registration
                          ("rsuMsgRepeatDeleteAll", 
                           handle_rsuMsgRepeatDeleteAll,
                           rsuMsgRepeatDeleteAll_oid,
                           OID_LENGTH(rsuMsgRepeatDeleteAll_oid),
                           HANDLER_CAN_RWRITE))){
      DEBUGMSGTL((MY_NAME, "rsuMsgRepeatDeleteAll install FAIL.\n"));
      set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
  }

  /* Client now has access. Use mutex beyond this point to be safe. Client should wait if causing conflict at this point.*/
  if(RSEMIB_OK == ntcip1218_lock()) {
      if(NULL != (live_rsuMsgRepeat = get_rsuMsgRepeatTable())){    /* Get pointer to live DB. Full r/w. */
          if(RSEMIB_OK != rebuild_rsuMsgRepeat(live_rsuMsgRepeat)){ /* Populate the table. */
              DEBUGMSGTL((MY_NAME, "rebuild_rsuMsgRepeat: FAIL.\n"));
              set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);      
          }    
      } else {
          DEBUGMSGTL((MY_NAME, "rsuMsgRepeatStatusTable populate FAIL.\n"));
          set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
      }
      ntcip1218_unlock();
  } else {
     DEBUGMSGTL((MY_NAME, "rsuMsgRepeatStatusTable lock FAIL.\n"));
     set_error_states(RSEMIB_LOCK_FAIL); /* Table is empty and error bit set. */
  }

  /* Test DB: get() should NOT fail unless contention with client causes conflict for mutex. Client should wait if causing conflict. */
  memset(rsuMsgRepeat, 0x0, sizeof(rsuMsgRepeat));
  for(i=0; i < RSU_SAR_MESSAGE_MAX; i++){
      if(RSEMIB_OK > get_rsuMsgRepeatIndex(i+1,&rsuMsgRepeat[i].rsuMsgRepeatIndex)){
          set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
          DEBUGMSGTL((MY_NAME, "rsuMsgRepeatIndex test FAIL: i = %d.\n",i));
      }
      if(RSEMIB_OK > get_rsuMsgRepeatPsid(i+1,rsuMsgRepeat[i].rsuMsgRepeatPsid)){
          set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
          DEBUGMSGTL((MY_NAME, "rsuMsgRepeatPsid test FAIL: i = %d.\n",i));
      }
      if(RSEMIB_OK > get_rsuMsgRepeatTxChannel(i+1,&rsuMsgRepeat[i].rsuMsgRepeatTxChannel)){
          set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
          DEBUGMSGTL((MY_NAME, "rsuMsgRepeatTxChannel test FAIL: i = %d.\n",i));
      }
      if(RSEMIB_OK > get_rsuMsgRepeatTxInterval(i+1,&rsuMsgRepeat[i].rsuMsgRepeatTxInterval)){
          set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
          DEBUGMSGTL((MY_NAME, "rsuMsgRepeatTxInterval test FAIL: i = %d.\n",i));
      }
      if(RSEMIB_OK > get_rsuMsgRepeatDeliveryStart(i+1,rsuMsgRepeat[i].rsuMsgRepeatDeliveryStart)){
          set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
          DEBUGMSGTL((MY_NAME, "rsuMsgRepeatDeliveryStart test FAIL: i = %d.\n",i));
      }
      if(RSEMIB_OK > get_rsuMsgRepeatDeliveryStop(i+1,rsuMsgRepeat[i].rsuMsgRepeatDeliveryStop)){
          set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
          DEBUGMSGTL((MY_NAME, "rsuMsgRepeatDeliveryStop test FAIL: i = %d.\n",i));
      }
      if(RSEMIB_OK > get_rsuMsgRepeatPayload(i+1,rsuMsgRepeat[i].rsuMsgRepeatPayload)){
          set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
          DEBUGMSGTL((MY_NAME, "rsuMsgRepeatPayload test FAIL: i = %d.\n",i));
      }
      if(RSEMIB_OK > get_rsuMsgRepeatEnable(i+1,(int32_t *)&rsuMsgRepeat[i].rsuMsgRepeatEnable)){
          set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
          DEBUGMSGTL((MY_NAME, "rsuMsgRepeatEnable test FAIL: i = %d.\n",i));
      }
      if(RSEMIB_OK > get_rsuMsgRepeatStatus(i+1,&rsuMsgRepeat[i].rsuMsgRepeatStatus)){
          set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
          DEBUGMSGTL((MY_NAME, "rsuMsgRepeatStatus test FAIL: i = %d.\n",i));
      }
      if(RSEMIB_OK > get_rsuMsgRepeatPriority(i+1,&rsuMsgRepeat[i].rsuMsgRepeatPriority)){
          set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
          DEBUGMSGTL((MY_NAME, "rsuMsgRepeatPriority test FAIL: i = %d.\n",i));
      }
      if(RSEMIB_OK > get_rsuMsgRepeatOptions(i+1,&rsuMsgRepeat[i].rsuMsgRepeatOptions)){
          set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
          DEBUGMSGTL((MY_NAME, "rsuMsgRepeatOptions test FAIL: i = %d.\n",i));
      }
  }
  /*
   * From this point ONLY rsuAMHUpdaterThr() will rebuild the table after AMH sends the approved list of msgs.
   *
   * If AMH not running data is dropped because AMH holds last word on what's valid. 
   * We can write it to disk and AMH may reject for a bunch of reasons above and beyond basic validation of data types. 
   * AMH not running is not the same as stand by mode. Stand by mode still means I2V and AMH there doing their job just no radio.
   */
}
void destroy_rsuMsgRepeat(void)
{
  destroy_rsuMsgRepeatStatusTable();

  /* Free table info. */
  if (NULL != table_reg_info){
      SNMP_FREE(table_reg_info);
  }
  /* Free table reg. */
  if (NULL != rsuMsgRepeatStatusTable){
      SNMP_FREE(rsuMsgRepeatStatusTable);
  }
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
int32_t commit_msgRepeat_to_disk(RsuMsgRepeatStatusEntry_t * msgRepeat) 
{
  FILE    * file_out       = NULL;
  uint32_t  mib_byte_count = 0;
  uint32_t  write_count    = 0;
  int32_t   ret            = RSEMIB_OK;

  if(NULL == msgRepeat){
      ret = RSEMIB_BAD_INPUT; 
  } else {
      DEBUGMSGTL((MY_NAME, "commit_to_disk:fopen(%s).\n",msgRepeat->filePathName));
      if ((file_out = fopen(msgRepeat->filePathName, "wb")) == NULL){
          DEBUGMSGTL((MY_NAME, "commit_to_disk: fopen fail.\n"));
          ret = RSEMIB_FOPEN_FAIL;
      } else {
          msgRepeat->crc = (uint32_t)crc16((const uint8_t *)msgRepeat, sizeof(RsuMsgRepeatStatusEntry_t) - 4);
          mib_byte_count = sizeof(RsuMsgRepeatStatusEntry_t);
          write_count = fwrite((uint8_t *)msgRepeat, mib_byte_count, 1, file_out);
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
  return ret;
}
