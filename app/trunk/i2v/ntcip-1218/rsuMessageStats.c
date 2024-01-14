/**************************************************************************
 *                                                                        *
 *     File Name:  rsuMessageStats.c                                      *
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

/* Ignore requests we have serviced. Quicker, but no harm if not enabled. */
#define USE_PROCESSED_FLAG

/* Mandatory: For SNMP debugging and output. */
#define MY_NAME       "MsgStat"
#define MY_NAME_EXTRA "XMsgStat"
/* Mandatory: For debugging and syslog output. */
#define MY_I2V_NAME    MY_NAME
#define MY_ERR_LEVEL   LEVEL_PRIV  /* from i2v_util.h */

/* table column definitions. */
#define MSG_COUNTS_BYPSID_INDEX_COLUMN    1
#define MSG_COUNTS_BYPSID_ID_COLUMN       2
#define MSG_COUNTS_BYPSID_CHANNEL_COLUMN  3
#define MSG_COUNTS_BYPSID_DIR_COLUMN      4
#define MSG_COUNTS_BYPSID_TIME_COLUMN     5
#define MSG_COUNTS_BYPSID_COUNT_COLUMN    6
#define MSG_COUNTS_BYPSID_STATUS_COLUMN   7
/* Theoritical column ids. Make sure they follow in order with columns and unique. */
#define MSG_COUNTS_BYPSID_MAX_OID         8

#define MSG_COUNTS_BYPSID_INDEX_NAME    "rsuMessageCountsByPsidIndex"
#define MSG_COUNTS_BYPSID_ID_NAME       "rsuMessageCountsByPsidId"
#define MSG_COUNTS_BYPSID_CHANNEL_NAME  "rsuMessageCountsByChannel"
#define MSG_COUNTS_BYPSID_DIR_NAME      "rsuMessageCountsDirection"
#define MSG_COUNTS_BYPSID_TIME_NAME     "rsuMessageCountsByPsidTime"
#define MSG_COUNTS_BYPSID_COUNT_NAME    "rsuMessageCountsByPsidCounts"
#define MSG_COUNTS_BYPSID_STATUS_NAME   "rsuMessageCountsByPsidRowStatus"
#define MSG_COUNTS_BYPSID_MAX_NAME       "maxRsuMessageCountsByPsid"


/******************************************************************************
 * 5.12.1 - 5.12.2 Message Statistics: { rsu 11 }: rsuMessageCountsByPsidTable: tx&rx per PSID.
 *
 *   int32_t                       maxRsuMessageCountsByPsid;
 *   RsuMessageCountsByPsidEntry_t rsuMessageCountsByPsidTable[RSU_PSID_TRACKED_STATS_MAX];
 *
 *   typedef struct {
 *       int32_t  rsuMessageCountsByPsidIndex;                        RO: 1..maxRsuMessageCountsByPsid.
 *       uint8_t  rsuMessageCountsByPsidId[RSU_RADIO_PSID_SIZE];      RC: 0..4 OCTETS: 0x0 = any PSID.
 *       int32_t  rsuMessageCountsByChannel;                          RC: 0..255: dsrc is 172 to 184: 0 = all *used* channels.
 *       int32_t  rsuMessageCountsDirection;                          RC: 1 = inbound, 2 = outbound, 3 = bothdir.
 *       uint8_t  rsuMessageCountsByPsidTime[MIB_DATEANDTIME_LENGTH]; RC:"2d-1d-1d,1d:1d:1d.1d": 8 OCTETS:DateAndTime:SNMPv2-TC.
 *       uint32_t rsuMessageCountsByPsidCounts;                       RO: count since boot: Counter32 == rolls over.
 *       int32_t  rsuMessageCountsByPsidRowStatus;                    RC: Create (4) & (6) destroy:
 *   } RsuMessageCountsByPsidEntry_t;
 *
 ******************************************************************************/

const oid maxRsuMessageCountsByPsid_oid[]   = {1, 3, 6, 1, 4, 1, 1206, 4, 2, 18, 11 , 1};
const oid rsuMessageCountsByPsidTable_oid[] = {1, 3, 6, 1, 4, 1, 1206, 4, 2, 18, 11 , 2};

extern int32_t  mainloop; /* For thread handling. */
extern ntcip_1218_mib_t  ntcip_1218_mib;

static netsnmp_table_registration_info * table_reg_info       = NULL; /* snmp table reg pointer. */
static netsnmp_tdata                   * rsuMessageStatsTable = NULL; /* snmp table pointer. */
static uint32_t rsuMessageStats_error_states                  = 0x0;  /* local error state. */
static messageStats_t * shm_messageStats_ptr                  = NULL; /* SHM. */

/*
 * Protos: Silence of the Warnings.
 */
static void rebuild_rsuMessageStats(void);
static void destroy_rsuMessageStatsTable(void);

/* 
 * Function Bodies.
 */
static void set_error_states(int32_t error)
{
  /* -1 to -32 */
  if((error < 0) && (error > MIB_ERROR_BASE_2)){
      rsuMessageStats_error_states |= (uint32_t)(0x1) << (abs(error) - 1);
  }
}
uint32_t get_rsuMessageStats_error_states(void)
{
  return rsuMessageStats_error_states;
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
      case MSG_COUNTS_BYPSID_INDEX_COLUMN:
          strcpy(column_name,MSG_COUNTS_BYPSID_INDEX_NAME);
          break;
      case MSG_COUNTS_BYPSID_ID_COLUMN:
          strcpy(column_name,MSG_COUNTS_BYPSID_ID_NAME);
          break;
      case MSG_COUNTS_BYPSID_CHANNEL_COLUMN:
          strcpy(column_name,MSG_COUNTS_BYPSID_CHANNEL_NAME);
          break;
      case MSG_COUNTS_BYPSID_DIR_COLUMN:
          strcpy(column_name,MSG_COUNTS_BYPSID_DIR_NAME);
          break;
      case MSG_COUNTS_BYPSID_TIME_COLUMN:
          strcpy(column_name,MSG_COUNTS_BYPSID_TIME_NAME);
          break;
      case MSG_COUNTS_BYPSID_COUNT_COLUMN:
          strcpy(column_name,MSG_COUNTS_BYPSID_COUNT_NAME);
          break;
      case MSG_COUNTS_BYPSID_STATUS_COLUMN:
          strcpy(column_name,MSG_COUNTS_BYPSID_STATUS_NAME);
          break;
      case MSG_COUNTS_BYPSID_MAX_OID:
          strcpy(column_name,MSG_COUNTS_BYPSID_MAX_NAME);
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

/* 5.12.1 */
static int32_t handle_maxRsuMessageCountsByPsid(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo,
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
              if(0 <= (ret = get_maxRsuMessageCountsByPsid())){
                  snmp_set_var_typed_integer(requests->requestvb, ASN_INTEGER, ret);
                  ret = RSEMIB_OK;
              }
              break;
          default:
              ret = RSEMIB_BAD_MODE;
              break;
      }
      /* scalars and like are always index 0 with unqiue column.*/
      print_request_result(0, MSG_COUNTS_BYPSID_MAX_OID,requests,reqinfo->mode,ret);
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
/* 5.12.2.1 */
static int32_t handle_rsuMessageCountsByPsidIndex(int32_t index,netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret  = RSEMIB_OK;
  int32_t data = index;

  DEBUGMSGTL((MY_NAME, "%s: Entry:\n",__FUNCTION__));
  switch (reqinfo->mode){
      case MODE_GET:
#if 0
          if(RSEMIB_OK == (ret = get_rsuMessageCountsByPsidIndex(index, &data)))
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
/* 5.12.2.2 */
static int32_t handle_rsuMessageCountsByPsidId(int32_t index, netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret = RSEMIB_OK;
  uint8_t data[RSU_RADIO_PSID_SIZE];

  DEBUGMSGTL((MY_NAME, "%s: Entry:\n",__FUNCTION__));
  switch (reqinfo->mode){
      case MODE_GET:
          DEBUGMSGTL((MY_NAME, "GET:\n"));
          memset(data,0x0,sizeof(data));
          if(0 <= (ret = get_rsuMessageCountsByPsidId(index,data))){
              snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR, data, ret);
              ret = RSEMIB_OK;
          }
          break;
      case MODE_SET_RESERVE1:
          DEBUGMSGTL((MY_NAME, "R1:\n"));
          if(RSEMIB_OK == (ret = preserve_rsuMessageCountsByPsidId(index))){
              if(RSEMIB_OK == (ret = action_rsuMessageCountsByPsidId(index,requests->requestvb->val.string,requests->requestvb->val_len))){
                  if(RSEMIB_OK == (ret = commit_rsuMessageCountsByPsidId(index))){
                      DEBUGMSGTL((MY_NAME, "COMMIT DONE:\n"));
                  } else {
                      DEBUGMSGTL((MY_NAME, "COMMIT FAIL:\n"));
                  }
              } else {
                  DEBUGMSGTL((MY_NAME, "ACTION FAIL:\n"));
              }
              if(RSEMIB_OK != ret) {
                  if(RSEMIB_OK != undo_rsuMessageCountsByPsidId(index)) { /* UNDO fails nothing we can do. */
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
/* 5.12.2.3 */
static int32_t handle_rsuMessageCountsByChannel(int32_t index,netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret  = RSEMIB_OK;
  int32_t data = 0;

  DEBUGMSGTL((MY_NAME, "%s: Entry:\n",__FUNCTION__));
  switch (reqinfo->mode){
      case MODE_GET:
          DEBUGMSGTL((MY_NAME, "GET:\n"));
          if(RSEMIB_OK == (ret = get_rsuMessageCountsByChannel(index,&data))){
              snmp_set_var_typed_integer(requests->requestvb, ASN_INTEGER, data);
          }
          break;
      case MODE_SET_RESERVE1: /* Short stack, all requests done in one. */
          DEBUGMSGTL((MY_NAME, "R1:\n"));
          if(RSEMIB_OK == (ret = preserve_rsuMessageCountsByChannel(index))){
              if(RSEMIB_OK == (ret = action_rsuMessageCountsByChannel(index,*(requests->requestvb->val.integer)))) {
                  if(RSEMIB_OK == (ret = commit_rsuMessageCountsByChannel(index))){
                      DEBUGMSGTL((MY_NAME, "COMMIT DONE:\n"));
                  } else {
                      DEBUGMSGTL((MY_NAME, "COMMIT FAIL:\n"));
                  }
              } else {
                  DEBUGMSGTL((MY_NAME, "ACTION FAIL:\n"));
              }
              if(RSEMIB_OK != ret) {
                  if(RSEMIB_OK != undo_rsuMessageCountsByChannel(index)) { /* UNDO fails nothing we can do. */
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
/* 5.12.2.4 */
static int32_t handle_rsuMessageCountsDirection(int32_t index,netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret  = RSEMIB_OK;
  int32_t data = 0;

  DEBUGMSGTL((MY_NAME, "%s: Entry:\n",__FUNCTION__));
  switch (reqinfo->mode){
      case MODE_GET:
          DEBUGMSGTL((MY_NAME, "GET:\n"));
          if(RSEMIB_OK == (ret = get_rsuMessageCountsDirection(index,&data))){
              snmp_set_var_typed_integer(requests->requestvb, ASN_INTEGER, data);
          }
          break;
      case MODE_SET_RESERVE1: /* Short stack, all requests done in one. */
          DEBUGMSGTL((MY_NAME, "R1:\n"));
          if(RSEMIB_OK == (ret = preserve_rsuMessageCountsDirection(index))){
              if(RSEMIB_OK == (ret = action_rsuMessageCountsDirection(index,*(requests->requestvb->val.integer)))) {
                  if(RSEMIB_OK == (ret = commit_rsuMessageCountsDirection(index))){
                      DEBUGMSGTL((MY_NAME, "COMMIT DONE:\n"));
                  } else {
                      DEBUGMSGTL((MY_NAME, "COMMIT FAIL:\n"));
                  }
              } else {
                  DEBUGMSGTL((MY_NAME, "ACTION FAIL:\n"));
              }
              if(RSEMIB_OK != ret) {
                  if(RSEMIB_OK != undo_rsuMessageCountsDirection(index)) { /* UNDO fails nothing we can do. */
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
/* 5.12.2.5 */
static int32_t handle_rsuMessageCountsByPsidTime(int32_t index,netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret = RSEMIB_OK;
  uint8_t data[MIB_DATEANDTIME_LENGTH];

  DEBUGMSGTL((MY_NAME, "%s: Entry:\n",__FUNCTION__));
  switch (reqinfo->mode){
      case MODE_GET:
          DEBUGMSGTL((MY_NAME, "GET:\n"));
          memset(data,0x0,sizeof(data));
          if(0 <= (ret = get_rsuMessageCountsByPsidTime(index,data))){
              snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR, data, ret);
              ret = RSEMIB_OK;
          }
          break;
      case MODE_SET_RESERVE1:
          DEBUGMSGTL((MY_NAME, "R1:\n"));
          if(RSEMIB_OK == (ret = preserve_rsuMessageCountsByPsidTime(index))){
              if(RSEMIB_OK == (ret = action_rsuMessageCountsByPsidTime(index,requests->requestvb->val.string,requests->requestvb->val_len))){
                  if(RSEMIB_OK == (ret = commit_rsuMessageCountsByPsidTime(index))){
                      DEBUGMSGTL((MY_NAME, "COMMIT DONE:\n"));
                  } else {
                      DEBUGMSGTL((MY_NAME, "COMMIT FAIL:\n"));
                  }
              } else {
                  DEBUGMSGTL((MY_NAME, "ACTION FAIL:\n"));
              }
              if(RSEMIB_OK != ret) {
                  if(RSEMIB_OK != undo_rsuMessageCountsByPsidTime(index)) { /* UNDO fails nothing we can do. */
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
/* 5.12.2.6 */
static int32_t handle_rsuMessageCountsByPsidCounts(int32_t index,netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret = RSEMIB_OK;
  uint32_t data = 0;

  DEBUGMSGTL((MY_NAME, "%s: Entry:\n",__FUNCTION__));
  switch (reqinfo->mode){
      case MODE_GET:
          if(RSEMIB_OK == (ret = get_rsuMessageCountsByPsidCounts(index,&data))){
              snmp_set_var_typed_integer(requests->requestvb, ASN_COUNTER, data);
          }
          break;
      default:
          ret = RSEMIB_BAD_MODE;
          break;
  }
  return ret;
}
/* 5.12.2.7 */
static int32_t handle_rsuMessageCountsByPsidRowStatus(int32_t index,netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests, int32_t override)
{
  int32_t ret  = RSEMIB_OK;
  int32_t data = 0;
  int32_t row_status = SNMP_ROW_CREATEANDGO; /* If override is set then don't use requests's value. Use this one. */

  DEBUGMSGTL((MY_NAME, "%s: Entry:\n",__FUNCTION__));
  switch (reqinfo->mode){
      case MODE_GET:
          if(RSEMIB_OK == (ret = get_rsuMessageCountsByPsidRowStatus(index,&data))){
              snmp_set_var_typed_integer(requests->requestvb, ASN_INTEGER, data);
          }
          break;
      case MODE_SET_RESERVE1: /* Short stack, all requests done in one. */
          DEBUGMSGTL((MY_NAME, "R1:\n"));
          if(0 == override){
              row_status = *(requests->requestvb->val.integer); /* Client index part of CREATE. */
          }
          DEBUGMSGTL((MY_NAME, "R1_BULK_SET: status = %d.\n",row_status));
          if(RSEMIB_OK == (ret = preserve_rsuMessageCountsByPsidRowStatus(index))){
              if(RSEMIB_OK == (ret = action_rsuMessageCountsByPsidRowStatus(index, row_status))) {
                  if(RSEMIB_OK == (ret = commit_rsuMessageCountsByPsidRowStatus(index))){
                      DEBUGMSGTL((MY_NAME, "COMMIT DONE:\n"));
                  } else {
                      DEBUGMSGTL((MY_NAME, "COMMIT FAIL:\n"));
                  }
              } else {
                  DEBUGMSGTL((MY_NAME, "ACTION FAIL:\n"));
              }
              if(RSEMIB_OK != ret) {
                  if(RSEMIB_OK != undo_rsuMessageCountsByPsidRowStatus(index)) { /* UNDO fails nothing we can do. */
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
 * 5.12.2: Table handler: Break it down to smaller requests.
 */
int32_t handle_rsuMessageStatsTable( netsnmp_mib_handler          * handler
                                   , netsnmp_handler_registration * reginfo
                                   , netsnmp_agent_request_info   * reqinfo
                                   , netsnmp_request_info         * requests)
{
  netsnmp_request_info          * request       = NULL;
  uint32_t                        request_count = 0;
  RsuMessageCountsByPsidEntry_t * table_entry   = NULL;
  netsnmp_table_request_info    * request_info  = NULL;
  int32_t                         ret           = SNMP_ERR_NOERROR;
  uint8_t                      raw_oid[MAX_OID_LEN + 2]; /* If table doesn't exist then helper can't help. Go fish for row index. */
  uint32_t                     row_index = 0; /* For readability. Note: index is 1..max */
  int32_t                      row_status = SNMP_ROW_NONEXISTENT;

  handler = handler; /* Silence of the Warnings. */
  reginfo = reginfo;

  DEBUGMSGTL((MY_NAME, "rsuMessageStatsTable_handler: Entry:\n"));
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
      if(NULL != (table_entry = (RsuMessageCountsByPsidEntry_t *)netsnmp_tdata_extract_entry(request))) {
          if((request_info->colnum < MSG_COUNTS_BYPSID_INDEX_COLUMN) || (MSG_COUNTS_BYPSID_STATUS_COLUMN < request_info->colnum)) {
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
      if(RSEMIB_OK != (ret = get_rsuMessageCountsByPsidRowStatus(row_index,&row_status))) {
          DEBUGMSGTL((MY_NAME, "get_rsuMessageCountsByPsidRowStatus Failed: ret=%d\n",ret));
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
          if(MSG_COUNTS_BYPSID_STATUS_COLUMN != request_info->colnum){/* If row doesn't exist then create now. */
              ret = handle_rsuMessageCountsByPsidRowStatus(row_index, reqinfo, request, 1); /* Do not use value in request. Force createAndGo. */
              if(RSEMIB_OK != ret) {
                  DEBUGMSGTL((MY_NAME, "Create failed for ROW=%d COLUMN %d.\n", row_index, request_info->colnum));
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
          case MSG_COUNTS_BYPSID_INDEX_COLUMN:
              ret = handle_rsuMessageCountsByPsidIndex(row_index, reqinfo, request);
              break;
          case MSG_COUNTS_BYPSID_ID_COLUMN:
              ret = handle_rsuMessageCountsByPsidId(row_index, reqinfo, request);
              break;
          case MSG_COUNTS_BYPSID_CHANNEL_COLUMN:
              ret = handle_rsuMessageCountsByChannel(row_index, reqinfo, request);
              break;
          case MSG_COUNTS_BYPSID_DIR_COLUMN:
              ret = handle_rsuMessageCountsDirection(row_index, reqinfo, request);
              break;
          case MSG_COUNTS_BYPSID_TIME_COLUMN:
              ret = handle_rsuMessageCountsByPsidTime(row_index, reqinfo, request);
              break;
          case MSG_COUNTS_BYPSID_COUNT_COLUMN:
              ret = handle_rsuMessageCountsByPsidCounts(row_index, reqinfo, request);
              break;
          case MSG_COUNTS_BYPSID_STATUS_COLUMN:
//If already created and status matches then who cares?
              ret = handle_rsuMessageCountsByPsidRowStatus(row_index, reqinfo, request, 0);
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

/* rsuMessageStatsTable creation functions.*/
STATIC void rsuMessageStatsTable_removeEntry(netsnmp_tdata * table_data, netsnmp_tdata_row * row)
{
  RsuMessageCountsByPsidEntry_t * entry = NULL;

  if (NULL == row){
      DEBUGMSGTL((MY_NAME,"rsuMessageStatsTable_removeEntry NULL input.\n"));
      return;
  }
  if (NULL != (entry = (RsuMessageCountsByPsidEntry_t *)row->data)){ /* We allocate, so we free. */
      SNMP_FREE(entry);
  } else {
      DEBUGMSGTL((MY_NAME,"rsuXmitMsgFwdingTable_removeEntry row data is NULL!.\n"));
  }
  if (table_data){
      netsnmp_tdata_remove_and_delete_row(table_data, row);
  } else {
      DEBUGMSGTL((MY_NAME,"rsuMessageStatsTable_removeEntry NULL input.\n"));
      netsnmp_tdata_delete_row(row); 
  }
  return;
}
static void destroy_rsuMessageStatsTable(void)
{
  netsnmp_tdata_row  * row     = NULL;
  netsnmp_tdata_row  * nextrow = NULL;
  int32_t              i       = 0;

  if(NULL != rsuMessageStatsTable){
      if(NULL != (row = netsnmp_tdata_row_first(rsuMessageStatsTable))){
          for(i=0;(row != NULL) && (i < RSU_PSID_TRACKED_STATS_MAX);i++){
              nextrow = netsnmp_tdata_row_next(rsuMessageStatsTable, row);
              rsuMessageStatsTable_removeEntry(rsuMessageStatsTable, row);
              row = nextrow;
          }
      } else {
          DEBUGMSGTL((MY_NAME, "rsuMessageStatsTable netsnmp_tdata_row_first FAIL.\n"));
      }
  } else {
      DEBUGMSGTL((MY_NAME, "destroy_rsuCommRangeTable: Table is NULL. Did we get called before install?\n"));
  }
}

static int32_t rsuMessageStats_createTable(netsnmp_tdata * table_data, RsuMessageCountsByPsidEntry_t * rsuMessageStats)
{
  RsuMessageCountsByPsidEntry_t * entry = NULL;
  netsnmp_tdata_row             * row   = NULL;
  int32_t                         i     = 0;
  int32_t                         valid_row_count = 0;

  if(NULL == table_data){
      DEBUGMSGTL((MY_NAME, "rsuMessageStats_createTable FAIL: table_data == NULL.\n"));
      return -1;
  }

  for(i=0; i < RSU_PSID_TRACKED_STATS_MAX; i++){
      if((rsuMessageStats[i].rsuMessageCountsByPsidIndex < 1) || (RSU_PSID_TRACKED_STATS_MAX < rsuMessageStats[i].rsuMessageCountsByPsidIndex)){
          return -1; /* Indices must be unique and greater than 1. */
      }
      if(RSEMIB_OK != valid_table_row(rsuMessageStats[i].rsuMessageCountsByPsidRowStatus)){
          continue; /* Only add rows that have been created. */
      }
      if (NULL == (entry = SNMP_MALLOC_TYPEDEF(RsuMessageCountsByPsidEntry_t))){
          DEBUGMSGTL((MY_NAME, "rsuMessageStats SNMP_MALLOC_TYPEDEF FAIL.\n"));
          return -1;
      }
      if (NULL == (row = netsnmp_tdata_create_row())){
          DEBUGMSGTL((MY_NAME, "rsuMessageStats netsnmp_tdata_create_row FAIL.\n"));
          SNMP_FREE(entry);
          return -1;
      }

      DEBUGMSGTL((MY_NAME_EXTRA, "rsuMessageStats make row i=%d index=%d.\n",i,rsuMessageStats[i].rsuMessageCountsByPsidIndex));
      row->data = entry;
      memcpy(entry,&rsuMessageStats[i],sizeof(RsuMessageCountsByPsidEntry_t));
      netsnmp_tdata_row_add_index(row, ASN_INTEGER , &rsuMessageStats[i].rsuMessageCountsByPsidIndex, sizeof(int32_t));
      if(SNMPERR_SUCCESS != netsnmp_tdata_add_row(table_data, row)){
          DEBUGMSGTL((MY_NAME, "rsuMessageStats netsnmp_tdata_add_row FAIL  i=%d.\n",i));
          return -1;
      }
      valid_row_count++;
  } /* for */
  DEBUGMSGTL((MY_NAME, "rsuMessageStats valid rows created = %d.\n",valid_row_count));
  return RSEMIB_OK;
}
static int32_t install_rsuMessageStatsTable(void)
{
    netsnmp_handler_registration * reg = NULL;

    reg = netsnmp_create_handler_registration("rsuMessageStatsTable"
                                              ,handle_rsuMessageStatsTable
                                              ,rsuMessageCountsByPsidTable_oid
                                              ,OID_LENGTH(rsuMessageCountsByPsidTable_oid)
                                              ,HANDLER_CAN_RWRITE);

    if (NULL == (rsuMessageStatsTable = netsnmp_tdata_create_table("rsuMessageStatsTable", 0))){
        DEBUGMSGTL((MY_NAME, "rsuMessageStatsTable FAIL.\n"));
        return RSEMIB_INSTALL_HANDLER_FAIL;
    }
    if (NULL == (table_reg_info = SNMP_MALLOC_TYPEDEF(netsnmp_table_registration_info))){
        DEBUGMSGTL((MY_NAME, "SNMP_MALLOC_TYPEDEF FAIL.\n"));
        return RSEMIB_INSTALL_HANDLER_FAIL;
    }
    netsnmp_table_helper_add_indexes(table_reg_info, ASN_INTEGER,0); /* index */                                 

    table_reg_info->min_column = MSG_COUNTS_BYPSID_ID_COLUMN; /* rsuMessageCountsByPsidId: Hide index by definition. */
    table_reg_info->max_column = MSG_COUNTS_BYPSID_STATUS_COLUMN; /* rsuMessageCountsByPsidRowStatus: */

    if (SNMPERR_SUCCESS == netsnmp_tdata_register(reg, rsuMessageStatsTable, table_reg_info)){
        return RSEMIB_OK; /* Table registered but still empty. rebuild_rsuMessageStats() will populate */
    } else {
        DEBUGMSGTL((MY_NAME, "netsnmp_tdata_register FAIL.\n"));
        return RSEMIB_INSTALL_HANDLER_FAIL;
    }
}
/* Assumed locked already. Live pointer to db. */
int32_t rebuild_rsuMessageStats_live(RsuMessageCountsByPsidEntry_t * MessageStats)
{
  if(NULL == MessageStats) {
      return RSEMIB_BAD_INPUT;
  }
  /* Remove rows but don't remove table reg. */
  destroy_rsuMessageStatsTable();

  /* Initialize the contents of the table here.*/
  if (RSEMIB_OK != rsuMessageStats_createTable(rsuMessageStatsTable, MessageStats)){
      DEBUGMSGTL((MY_NAME, "rebuild_rsuMessageStats_live: ERROR: Null row.\n"));
      set_error_states(RSEMIB_REBUILD_FAIL);
      return RSEMIB_REBUILD_FAIL;
  }
  return RSEMIB_OK;
}
static void rebuild_rsuMessageStats(void)
{
  int32_t i = 0;
  RsuMessageCountsByPsidEntry_t rsuMessageStats[RSU_PSID_TRACKED_STATS_MAX];

  memset(rsuMessageStats,0x0,sizeof(rsuMessageStats));
  for(i=0; i < RSU_PSID_TRACKED_STATS_MAX; i++){
      if(RSEMIB_OK > get_rsuMessageCountsByPsidIndex(i+1, &rsuMessageStats[i].rsuMessageCountsByPsidIndex)){
          set_error_states(RSEMIB_REBUILD_FAIL);
          DEBUGMSGTL((MY_NAME, "rsuMessageCountsByPsidIndex rebuild FAIL: i = %d.\n",i));
      }
      if(RSEMIB_OK > get_rsuMessageCountsByPsidId(i+1, rsuMessageStats[i].rsuMessageCountsByPsidId)){
          set_error_states(RSEMIB_REBUILD_FAIL);
          DEBUGMSGTL((MY_NAME, "rsuMessageCountsByPsidId rebuild FAIL: i = %d.\n",i));
      }
      if(RSEMIB_OK > get_rsuMessageCountsByChannel(i+1, &rsuMessageStats[i].rsuMessageCountsByChannel)){
          set_error_states(RSEMIB_REBUILD_FAIL);
          DEBUGMSGTL((MY_NAME, "rsuMessageCountsByChannel rebuild FAIL: i = %d.\n",i));
      }
      if(RSEMIB_OK > get_rsuMessageCountsDirection(i+1, &rsuMessageStats[i].rsuMessageCountsDirection)){
          set_error_states(RSEMIB_REBUILD_FAIL);
          DEBUGMSGTL((MY_NAME, "rsuMessageCountsDirection rebuild FAIL: i = %d.\n",i));
      }
      if(RSEMIB_OK > get_rsuMessageCountsByPsidTime(i+1, rsuMessageStats[i].rsuMessageCountsByPsidTime)){
          set_error_states(RSEMIB_REBUILD_FAIL);
          DEBUGMSGTL((MY_NAME, "rsuMessageCountsByPsidTime rebuild FAIL: i = %d.\n",i));
      }
      if(RSEMIB_OK > get_rsuMessageCountsByPsidCounts(i+1, &rsuMessageStats[i].rsuMessageCountsByPsidCounts)){
          set_error_states(RSEMIB_REBUILD_FAIL);
          DEBUGMSGTL((MY_NAME, "rsuMessageCountsByPsidCounts rebuild FAIL: i = %d.\n",i));
      }
      if(RSEMIB_OK > get_rsuMessageCountsByPsidRowStatus(i+1, &rsuMessageStats[i].rsuMessageCountsByPsidRowStatus)){
          set_error_states(RSEMIB_REBUILD_FAIL);
          DEBUGMSGTL((MY_NAME, "rsuMessageCountsByPsidRowStatus rebuild FAIL: i = %d.\n",i));
      }
  }
  /* Remove rows but don't remove table reg. */
  destroy_rsuMessageStatsTable();

  /* Initialize the contents of the table here.*/
  if (RSEMIB_OK != rsuMessageStats_createTable(rsuMessageStatsTable, &rsuMessageStats[0])){
      DEBUGMSGTL((MY_NAME, "rebuild_rsuMessageStats: ERROR: Null row.\n"));
      set_error_states(RSEMIB_REBUILD_FAIL);
  }
}
void install_rsuMessageStats_handlers(void)
{
  /* Reset statics across soft reset. */
  rsuMessageStatsTable         = NULL;
  table_reg_info               = NULL;
  rsuMessageStats_error_states = 0x0;
  shm_messageStats_ptr = NULL; /* SHM. */

  /* 5.12.1: */
  if(MIB_REGISTERED_OK != netsnmp_register_scalar(netsnmp_create_handler_registration
                          ("maxRsuMessageCountsByPsid", 
                           handle_maxRsuMessageCountsByPsid,
                           maxRsuMessageCountsByPsid_oid,
                           OID_LENGTH(maxRsuMessageCountsByPsid_oid),
                           HANDLER_CAN_RONLY))) {
      DEBUGMSGTL((MY_NAME, "maxRsuMessageCountsByPsid install FAIL.\n"));
      set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
  }
  /* 5.12.2.1 - 5.19.2.7: Install & register table handler. Does not populate table with data. */
  if( RSEMIB_OK != install_rsuMessageStatsTable()){
      DEBUGMSGTL((MY_NAME, "rsuMessageStatsTable install FAIL.\n"));
      set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
  }

  /* Populate the table. */
  rebuild_rsuMessageStats();
}
void destroy_rsuMessageStats(void)
{
  destroy_rsuMessageStatsTable();

  /* Free table info. */
  if (NULL != table_reg_info){
      SNMP_FREE(table_reg_info);
  }
  /* Free table reg. */
  if (NULL != rsuMessageStatsTable){
      SNMP_FREE(rsuMessageStatsTable);
  }
}
/* RSU App thread support. */
int32_t get_mib_DateAndTime(uint8_t * DateAndTime)
{
  int32_t ret = RSEMIB_OK;
  time_t  result;
  struct tm * timeptr, tm;

  if(NULL == DateAndTime) {
      ret = RSEMIB_BAD_INPUT;
  } else {
      result = time(NULL);
      timeptr = gmtime_r(&result, &tm);
      DateAndTime[0] = 0xff & ((1900 + timeptr->tm_year) >> 8);
      DateAndTime[1] = 0xff & (1900 + timeptr->tm_year);
      DateAndTime[2] = timeptr->tm_mon + 1;
      DateAndTime[3] = timeptr->tm_mday;
      DateAndTime[4] = timeptr->tm_hour;
      DateAndTime[5] = timeptr->tm_min;
      DateAndTime[6] = timeptr->tm_sec;
      DateAndTime[7] = 0x0;
  }
  return ret;
}
/* 
 * Return negative if bogus:
 *
 * from ntcip-1218.h: MIB_DATEANDTIME_LENGTH: "2d-1d-1d,1d:1d:1d.1d": 8 OCTETS: DateAndTime: SNMPv2-TC
 *
 * from SNMPv2-TC.txt: A date-time specification.
 *
 *        field  octets  contents                  range
 *        -----  ------  --------                  -----
 *          1      1-2   year*                     0..65536
 *          2       3    month                     1..12
 *          3       4    day                       1..31
 *          4       5    hour                      0..23
 *          5       6    minutes                   0..59
 *          6       7    seconds                   0..60
 *                       (use 60 for leap-second)
 *          7       8    deci-seconds              0..9
 *
 *  Not supported, ignore.
 *  
 *          8       9    direction from UTC        '+' / '-'
 *          9      10    hours from UTC*           0..13
 *         10      11    minutes from UTC          0..59
 *
 *        Notes:
 *        - the value of year is in network-byte order
 *        - daylight saving time in New Zealand is +13
 *
 *   Tuesday May 26, 1992 at 1:30:15 PM EDT == 1992-5-26,13:30:15.0,-4:0
 */
static int32_t DateAndTime_To_UTC_DSEC(uint8_t * DateAndTime, uint64_t * utcDsec)
{
  int32_t ret         = RSEMIB_OK;
  int32_t year        = 0;
  int32_t month       = 0;
  int32_t day         = 0;
  int32_t hour        = 0;
  int32_t minute      = 0;
  int32_t second      = 0;
  int32_t dsec        = 0;
  uint64_t testDsec   = 0;
  struct tm mib_tm;

  if(NULL == utcDsec) {
      ret = RSEMIB_BAD_DATA;
      goto exit_out;
  }
  year   = (int32_t)DateAndTime[0]; /* Year */
  year   = year << 8;
  year  += (int32_t)DateAndTime[1]; 
  month  = (int32_t)DateAndTime[2]; /* Month */
  day    = (int32_t)DateAndTime[3]; /* Day */
  hour   = (int32_t)DateAndTime[4]; /* Hour */
  minute = (int32_t)DateAndTime[5]; /* Minute */
  second = (int32_t)DateAndTime[6]; /* Second */
  dsec   = (int32_t)DateAndTime[7]; /* Dsecond */
  /* Assume bad till proven good at end. */
  ret = RSEMIB_BAD_UTC;
  if((year < 0) || (65536 < year)){ /* Basic range check: */
      DEBUGMSGTL((MY_NAME, "DateAndTime_To_UTC_DSEC: Bad year.\n"));
             /* From unsigned above so can't be less than zero really. */
      goto exit_out;
  }
  year = year - 1900; /* adjust for mktime(). */
  if((month < 1) || (12 < month)){ /* Not exhaustive for month & day. See below for more. */
      DEBUGMSGTL((MY_NAME, "DateAndTime_To_UTC_DSEC: Bad month.\n"));
      goto exit_out;  
  }
  month = month - 1; /* adjust for mktime(). */
  if((day < 1) || (31 < day)){
      DEBUGMSGTL((MY_NAME, "DateAndTime_To_UTC_DSEC: Bad day.\n"));
      goto exit_out;
  }
  if((hour < 0) || (23 < hour)){
      DEBUGMSGTL((MY_NAME, "DateAndTime_To_UTC_DSEC: Bad hour.\n"));
      goto exit_out;
  }
  if((minute < 0) || (59 < minute)){
      DEBUGMSGTL((MY_NAME, "DateAndTime_To_UTC_DSEC: Bad minute.\n"));
      goto exit_out;
  }
  if((second < 0) || (60 < second)){
      DEBUGMSGTL((MY_NAME, "DateAndTime_To_UTC_DSEC: Bad second.\n"));
      goto exit_out;
  }
  if((dsec < 0) || (9 < dsec)){
      DEBUGMSGTL((MY_NAME, "DateAndTime_To_UTC_DSEC: Bad dsec.\n"));
      goto exit_out;
  }
  mib_tm.tm_sec   = second;
  mib_tm.tm_min   = minute;
  mib_tm.tm_hour  = hour;
  mib_tm.tm_mday  = day;
  mib_tm.tm_mon   = month;
  mib_tm.tm_year  = year;
  mib_tm.tm_wday  = 0;
  mib_tm.tm_yday  = 0;
  mib_tm.tm_isdst = 0;
  /* Assumming our error checking above is correct this can't fail. */
  testDsec = (uint64_t)mktime(&mib_tm);
  if ((time_t)-1 == (time_t)testDsec){
      DEBUGMSGTL((MY_NAME, "DateAndTime_To_UTC_DSEC: mktime() failed.\n"));
  } else {
      testDsec *= (uint64_t)10;   /* Convert to deci-seconds and add final value. */
      testDsec += (uint64_t)dsec;
      *utcDsec = testDsec;
      ret = RSEMIB_OK; /* Only place where it can return OK. */
  } 
exit_out:
  if(ret < RSEMIB_OK) {
      set_error_states(ret);
  }
  return ret;
}
static uint64_t get_utcMsec(void)
{
  struct timeval tv;
  uint64_t timenow;

  gettimeofday(&tv, NULL);
  timenow = tv.tv_sec;
  timenow *= 1000;
  timenow += tv.tv_usec/1000;
  return timenow;
}
static int32_t check_Start_UTC_dsec(uint64_t StartTimeDsec)
{
  uint64_t UTC_dsec = (uint64_t)(get_utcMsec()/(uint64_t)100);

  /* Wellness checks. */
  if(StartTimeDsec <= UTC_dsec) { /* This message is active assuming it is well formed. */
      return SNMP_ROW_ACTIVE;
  } else {
      /* These files stick around so lots of debug if enabled. */
      DEBUGMSGTL((MY_NAME, "check_Start_UTC_dsec: now < start: Not in service yet.\n"));
      return SNMP_ROW_NOTINSERVICE; /* Time to start not here but valid nonetheless. */
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
static void force_messageStatShm_update(void) 
{
  int32_t i;

  /* One time load from MIB to SHM. After that only client update will trigger update on a single row. */
  for(i=0;i<RSU_PSID_TRACKED_STATS_MAX; i++){
      if(RSEMIB_OK != update_messageStats_shm(&ntcip_1218_mib.rsuMessageCountsByPsidTable[i])){
         set_error_states(RSEMIB_SHM_SEED_FAILED);
      }
  }
}
/*
 * TODO: Client needs to be smart and disable before editing else fun! 
 */
void *messageStatsThr(void __attribute__((unused)) *arg)
{
  size_t shm_messageStat_size = sizeof(messageStats_t) * RSU_PSID_TRACKED_STATS_MAX;
  int32_t i = 0;
  int32_t row_status = SNMP_ROW_NOTREADY;
  uint8_t DateAndTime[MIB_DATEANDTIME_LENGTH];

  DEBUGMSGTL((MY_NAME,"messageStatThr: Entry.\n")); 
  /* Open SHM: Assumption MIB opens first to setup. */
  if ((shm_messageStats_ptr = mib_share_init(shm_messageStat_size, MSG_STATS_SHM_PATH)) == NULL) {
      DEBUGMSGTL((MY_NAME,"messageStatThr: SHM init failed.\n"));
  } else {
       /*
        *  Give RSU apps a chance to register. 
        *  Handshake would be better but we don't know how many apps will reg. 
        */
      for(i=0;i<30;i++)
          sleep(1);

      /* Time master: Set starts to now. Hopefully good gps now. */
      if(RSEMIB_OK != get_mib_DateAndTime(DateAndTime)) {
          DEBUGMSGTL((MY_NAME,"messageStatThr: get_mib_DateAndTime failed.\n"));
         /* default start == 07e6-01-01:00:00:00.1  ==  2022-01-01:00:00:00.1 */
         DateAndTime[0] = 0x07;
         DateAndTime[1] = 0xe6;
         DateAndTime[2] = 0x01;
         DateAndTime[3] = 0x01;
         DateAndTime[4] = 0x00;
         DateAndTime[5] = 0x00;
         DateAndTime[6] = 0x00;
         DateAndTime[7] = 0x01;
      }
      for(i=0;i<RSU_PSID_TRACKED_STATS_MAX;i++){
          memcpy(ntcip_1218_mib.rsuMessageCountsByPsidTable[i].rsuMessageCountsByPsidTime,DateAndTime,MIB_DATEANDTIME_LENGTH);
      }
      /* Force update from MIB one time to load contents and signal apps we are open.*/
      force_messageStatShm_update();
      while(mainloop) {
          sleep(1);
          /* Update stats and check status of rows. */
          for(i=0;i<RSU_PSID_TRACKED_STATS_MAX; i++){ 
              if(SNMP_ROW_ACTIVE == shm_messageStats_ptr[i].rowStatus) {
                  ntcip_1218_mib.rsuMessageCountsByPsidTable[i].rsuMessageCountsByPsidCounts = shm_messageStats_ptr[i].count;
              }
              if(  (SNMP_ROW_ACTIVE == shm_messageStats_ptr[i].rowStatus)
                 ||(SNMP_ROW_NOTINSERVICE == shm_messageStats_ptr[i].rowStatus)) {

                  /* No stop time, only start. If start <= now */
                  row_status = check_Start_UTC_dsec(shm_messageStats_ptr[i].start_utc_dsec);
                  if(shm_messageStats_ptr[i].rowStatus != row_status) {
                      shm_messageStats_ptr[i].rowStatus = row_status;
                      shm_messageStats_ptr[i].count = 0;
                      ntcip_1218_mib.rsuMessageCountsByPsidTable[i].rsuMessageCountsByPsidCounts = 0;
                      shm_messageStats_ptr[i].dirty = 1;
                  }
                  if(SNMP_ROW_ACTIVE != row_status){ /* There could be lots of messages not ready to send yet so watch out. */
                      DEBUGMSGTL((MY_NAME,"receivedMsgThr: Request for active row=%d denied. Bad or not ready time to start.\n",i));
                  }
              }
          }
          for(i=0;i<10;i++){
              DEBUGMSGTL((MY_NAME_EXTRA,"MsgStatShm[%d]: dirty=%u, status=%d, count=%u, psid=0x%x, chan=%d, dir=%d, start=0x%lx.\n"
                          , i
                          , shm_messageStats_ptr[i].dirty
                          , shm_messageStats_ptr[i].rowStatus
                          , shm_messageStats_ptr[i].count
                          , shm_messageStats_ptr[i].psid
                          , shm_messageStats_ptr[i].channel
                          , shm_messageStats_ptr[i].direction
                          , shm_messageStats_ptr[i].start_utc_dsec));
          }
      }
      /* Close SHM. */
      mib_share_kill(shm_messageStats_ptr, shm_messageStat_size );
      shm_messageStats_ptr = NULL; 
  }
  DEBUGMSGTL((MY_NAME,"messageStatThr: Exit."));
  pthread_exit(NULL);
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
int32_t commit_rsuMessageStats_to_disk(RsuMessageCountsByPsidEntry_t * rsuMessageStat)
{
  FILE    * file_out       = NULL;
  uint32_t  mib_byte_count = 0;
  uint32_t  write_count    = 0;
  int32_t   ret            = RSEMIB_OK;

  if(NULL == rsuMessageStat){
      ret = RSEMIB_BAD_INPUT; 
  } else {
      DEBUGMSGTL((MY_NAME, "commit_to_disk:fopen(%s).\n",rsuMessageStat->filePathName));
      if ((file_out = fopen(rsuMessageStat->filePathName, "wb")) == NULL){
          DEBUGMSGTL((MY_NAME, "commit_to_disk: fopen fail.\n"));
          ret = RSEMIB_FOPEN_FAIL;
      } else {
          rsuMessageStat->crc = (uint32_t)crc16((const uint8_t *)rsuMessageStat, sizeof(RsuMessageCountsByPsidEntry_t) - 4);
          mib_byte_count = sizeof(RsuMessageCountsByPsidEntry_t);
          write_count = fwrite((uint8_t *)rsuMessageStat, mib_byte_count, 1, file_out);
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
/* SHM is not locked. */
int32_t update_messageStats_shm(RsuMessageCountsByPsidEntry_t * MessageStat)
{
  int32_t i; /* For readability. */
  uint32_t psid = 0x0;
  uint64_t UTC_dsec = 0;

  if(NULL == shm_messageStats_ptr) { /* Should never happen by order of startup. */
      DEBUGMSGTL((MY_NAME, "update_messageStats_shm: RSEMIB_SHM_INIT_FAILED.\n"));
      set_error_states(RSEMIB_SHM_INIT_FAILED);
      return RSEMIB_SHM_INIT_FAILED; 
  }
  if(NULL == MessageStat) {
      DEBUGMSGTL((MY_NAME, "update_messageStats_shm: NULL == MessageStat.\n"));
      set_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;    
  }
  i = MessageStat->rsuMessageCountsByPsidIndex - 1;
  if((i < 0) || (RSU_PSID_TRACKED_STATS_MAX <= i)){
      DEBUGMSGTL((MY_NAME, "update_messageStats_shm: RSEMIB_BAD_INDEX.\n"));
      set_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }

  shm_messageStats_ptr[i].dirty = 0;
  psid = 0;
  if(MessageStat->rsuMessageCountsByPsidId_length >= 1) {
      psid = (uint32_t)(MessageStat->rsuMessageCountsByPsidId[0]);
  }
  if(MessageStat->rsuMessageCountsByPsidId_length >= 2) {
      psid  = psid << 8;
      psid |= (uint32_t)(MessageStat->rsuMessageCountsByPsidId[1]);
  }
  if(MessageStat->rsuMessageCountsByPsidId_length >= 3) {
      psid  = psid << 8;
      psid |= (uint32_t)(MessageStat->rsuMessageCountsByPsidId[2]);
  }
  if(MessageStat->rsuMessageCountsByPsidId_length == 4) {
      psid  = psid << 8;
      psid |= (uint32_t)(MessageStat->rsuMessageCountsByPsidId[3]);
  }
  shm_messageStats_ptr[i].psid      = psid;
  shm_messageStats_ptr[i].channel   = MessageStat->rsuMessageCountsByChannel;
  shm_messageStats_ptr[i].direction = MessageStat->rsuMessageCountsDirection;
  shm_messageStats_ptr[i].count     = 0;
  shm_messageStats_ptr[i].rowStatus = MessageStat->rsuMessageCountsByPsidRowStatus;
  if(RSEMIB_OK != DateAndTime_To_UTC_DSEC((uint8_t *)&MessageStat->rsuMessageCountsByPsidTime[0], &UTC_dsec)){
      return RSEMIB_BAD_UTC;
  }
  shm_messageStats_ptr[i].start_utc_dsec = UTC_dsec;
  /* Client doesnt decide if row is ready to send yet. We do, based on time. */ 
  if(SNMP_ROW_ACTIVE == MessageStat->rsuMessageCountsByPsidRowStatus) {
      shm_messageStats_ptr[i].rowStatus = check_Start_UTC_dsec(shm_messageStats_ptr[i].start_utc_dsec);
  }
  shm_messageStats_ptr[i].dirty = 1;
  return RSEMIB_OK;
}
