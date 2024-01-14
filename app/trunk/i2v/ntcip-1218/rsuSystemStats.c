/**************************************************************************
 *                                                                        *
 *     File Name:  rsuSystemStats.c                                       *
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
#define MY_NAME       "SysStats"
#define MY_NAME_EXTRA "XSysStats"
/* Mandatory: For debugging and syslog output. */
#define MY_I2V_NAME    MY_NAME
#define MY_ERR_LEVEL   LEVEL_PRIV    /* from i2v_util.h */

/* table column definitions. */
#define COMM_RANGE_INDEX_COLUMN        1
#define COMM_RANGE_SECTOR_COLUMN       2
#define COMM_RANGE_MSG_ID_COLUMN       3
#define COMM_RANGE_FILTER_TYPE_COLUMN  4
#define COMM_RANGE_FILTER_VALUE_COLUMN 5
#define COMM_RANGE_1_MIN_COLUMN        6
#define COMM_RANGE_5_MIN_COLUMN        7
#define COMM_RANGE_15_MIN_COLUMN       8
#define COMM_RANGE_AVG_1_MIN_COLUMN    9
#define COMM_RANGE_AVG_5_MIN_COLUMN    10
#define COMM_RANGE_AVG_15_MIN_COLUMN   11
#define COMM_RANGE_STATUS_COLUMN       12
/* Theoritical column ids. Make sure they follow in order with columns and unique. */
#define SYSTEM_STATS_TIME_SINCE_POWER_ON_OID  13
#define SYSTEM_STATS_INT_TEMP_OID             14
#define SYSTEM_STATS_INT_TEMP_LOW_THRESH_OID  15
#define SYSTEM_STATS_INT_TEMP_HIGH_THRESH_OID 16
#define COMM_RANGE_MAX_OID                    17

#define COMM_RANGE_INDEX_NAME         "rsuCommRangeIndex"
#define COMM_RANGE_SECTOR_NAME        "rsuCommRangeSector"
#define COMM_RANGE_MSG_ID_NAME        "rsuCommRangeMsgId"
#define COMM_RANGE_FILTER_TYPE_NAME   "rsuCommRangeFilterType"
#define COMM_RANGE_FILTER_VALUE_NAME  "rsuCommRangeFilterValue"
#define COMM_RANGE_1_MIN_NAME         "rsuCommRange1Min"
#define COMM_RANGE_5_MIN_NAME         "rsuCommRange5Min"
#define COMM_RANGE_15_MIN_NAME        "rsuCommRange15Min"
#define COMM_RANGE_AVG_1_MIN_NAME     "rsuCommRangeAvg1Min"
#define COMM_RANGE_AVG_5_MIN_NAME     "rsuCommRangeAvg5Min"
#define COMM_RANGE_AVG_15_MIN_NAME    "rsuCommRangeAvg15Min"
#define COMM_RANGE_STATUS_NAME        "rsuCommRangeStatus"
/* Theoritical column ids. Make sure they follow in order with columns and unique. */
#define SYSTEM_STATS_TIME_SINCE_POWER_ON_NAME  "rsuTimeSincePowerOn"
#define SYSTEM_STATS_INT_TEMP_NAME              "rsuIntTemp"
#define SYSTEM_STATS_INT_TEMP_LOW_THRESH_NAME  "rsuIntTempLowThreshold"
#define SYSTEM_STATS_INT_TEMP_HIGH_THRESH_NAME  "rsuIntTempHighThreshold"
#define COMM_RANGE_MAX_NAME                     "maxRsuCommRange"

/******************************************************************************
 * 5.13 Systems Statistics: { rsu 12 }: rsuCommRangeTable
 * 
 * typedef struct {
 *     int32_t rsuCommRangeIndex;       RO: 1..maxRsuCommRange
 *     int32_t rsuCommRangeSector;      RC: 1..16 : sector = 22.5 deg: 16 sectors. 1 is north then east by 22.5 deg etc.
 *     int32_t rsuCommRangeMsgId;       RC: 0..32767: 0 = any: DE_DSRC_MessageID of J2735.
 *     int32_t rsuCommRangeFilterType;  RC: 1 = noFilter, 2 = vehicleType, 3 = vehicleClass: DE_VehicleType J2735.
 *     int32_t rsuCommRangeFilterValue; RC: 0..255: value of vehicleType or vehicleClass: addtional filter to rsuCommRangeMsgId.
 *     int32_t rsuCommRange1Min;        RC: 0..2001: 0 = no msg: 2001 = unknown: 2000 = 2000+: 1..1999 = 1..1999 metres:furtherest.
 *     int32_t rsuCommRange5Min;        RC: 0..2001: 0 = no msg: 2001 = unknown: 2000 = 2000+: 1..1999 = 1..1999 metres.furtherest.
 *     int32_t rsuCommRange15Min;       RC: 0..2001: 0 = no msg: 2001 = unknown: 2000 = 2000+: 1..1999 = 1..1999 metres.furtherest.
 *     int32_t rsuCommRangeAvg1Min;     RC: 0..2001: 0 = no msg: 2001 = unknown: 2000 = 2000+: 1..1999 = 1..1999 metres: avg.
 *     int32_t rsuCommRangeAvg5Min;     RC: 0..2001: 0 = no msg: 2001 = unknown: 2000 = 2000+: 1..1999 = 1..1999 metres: avg.
 *     int32_t rsuCommRangeAvg15Min;    RC: 0..2001: 0 = no msg: 2001 = unknown: 2000 = 2000+: 1..1999 = 1..1999 metres: avg.
 *     int32_t rsuCommRangeStatus;      RC: Create (4) & (6) destroy:   
 * } RsuCommRangeEntry_t;
 * 
 ******************************************************************************/
const oid rsuTimeSincePowerOn_oid[]     = {1, 3, 6, 1, 4, 1, 1206, 4, 2, 18, 12 , 1};
const oid rsuIntTemp_oid[]              = {1, 3, 6, 1, 4, 1, 1206, 4, 2, 18, 12 , 2};
const oid rsuIntTempLowThreshold_oid[]  = {1, 3, 6, 1, 4, 1, 1206, 4, 2, 18, 12 , 3};
const oid rsuIntTempHighThreshold_oid[] = {1, 3, 6, 1, 4, 1, 1206, 4, 2, 18, 12 , 4};
const oid maxRsuCommRange_oid[]         = {1, 3, 6, 1, 4, 1, 1206, 4, 2, 18, 12 , 5};
const oid rsuCommRangeTable_oid[]       = {1, 3, 6, 1, 4, 1, 1206, 4, 2, 18, 12 , 6};

extern int32_t  mainloop; /* For thread handling. */
extern ntcip_1218_mib_t  ntcip_1218_mib;

static netsnmp_table_registration_info * table_reg_info = NULL; /* snmp table reg pointer. */
static netsnmp_tdata * rsuCommRangeTable                = NULL; /* snmp table pointer. */
static uint32_t rsuSystemStats_error_states             = 0x0;  /* local error state. */

/*
 * Protos: Silence of the Warnings.
 */
static void rebuild_rsuCommRangeTable(void);
static void destroy_rsuCommRangeTable(void);

/* 
 * Function Bodies.
 */
static void set_error_states(int32_t error)
{
  /* -1 to -32 */
  if((error < 0) && (error > MIB_ERROR_BASE_2)){
      rsuSystemStats_error_states |= (uint32_t)(0x1) << (abs(error) - 1);
  }
}
uint32_t get_rsuSystemStats_error_states(void)
{
  return rsuSystemStats_error_states;
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
      case COMM_RANGE_INDEX_COLUMN:
          strcpy(column_name,COMM_RANGE_INDEX_NAME);
          break;
      case COMM_RANGE_SECTOR_COLUMN:
          strcpy(column_name,COMM_RANGE_SECTOR_NAME);
          break;
      case COMM_RANGE_MSG_ID_COLUMN:
          strcpy(column_name,COMM_RANGE_MSG_ID_NAME);
          break;
      case COMM_RANGE_FILTER_TYPE_COLUMN:
          strcpy(column_name,COMM_RANGE_FILTER_TYPE_NAME);
          break;
      case COMM_RANGE_FILTER_VALUE_COLUMN:
          strcpy(column_name,COMM_RANGE_FILTER_VALUE_NAME);
          break;
      case COMM_RANGE_1_MIN_COLUMN:
          strcpy(column_name,COMM_RANGE_1_MIN_NAME);
          break;
      case COMM_RANGE_5_MIN_COLUMN:
          strcpy(column_name,COMM_RANGE_5_MIN_NAME);
          break;
      case COMM_RANGE_15_MIN_COLUMN:
          strcpy(column_name,COMM_RANGE_15_MIN_NAME);
          break;
      case COMM_RANGE_AVG_1_MIN_COLUMN:
          strcpy(column_name,COMM_RANGE_AVG_1_MIN_NAME);
          break;
      case COMM_RANGE_AVG_5_MIN_COLUMN:
          strcpy(column_name,COMM_RANGE_AVG_5_MIN_NAME);
          break;
      case COMM_RANGE_AVG_15_MIN_COLUMN:
          strcpy(column_name,COMM_RANGE_AVG_15_MIN_NAME);
          break;
      case COMM_RANGE_STATUS_COLUMN:
          strcpy(column_name,COMM_RANGE_STATUS_NAME);
          break;
      case SYSTEM_STATS_TIME_SINCE_POWER_ON_OID:
          strcpy(column_name,SYSTEM_STATS_TIME_SINCE_POWER_ON_NAME);
          break;
      case SYSTEM_STATS_INT_TEMP_OID:
          strcpy(column_name,SYSTEM_STATS_INT_TEMP_NAME);
          break;
      case SYSTEM_STATS_INT_TEMP_LOW_THRESH_OID:
          strcpy(column_name,SYSTEM_STATS_INT_TEMP_LOW_THRESH_NAME);
          break;
      case SYSTEM_STATS_INT_TEMP_HIGH_THRESH_OID:
          strcpy(column_name,SYSTEM_STATS_INT_TEMP_HIGH_THRESH_NAME);
          break;
      case COMM_RANGE_MAX_OID:
          strcpy(column_name,COMM_RANGE_MAX_NAME);
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

/* 5.13.1 */
int32_t handle_rsuTimeSincePowerOn(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo,
                                   netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret = RSEMIB_OK;
  uint32_t data = 0;

  handler = handler; /* Silence of the Warnings. */
  reginfo = reginfo;
  DEBUGMSGTL((MY_NAME, "%s: Entry:\n",__FUNCTION__));
  if((NULL == reqinfo) || (NULL == requests)) {
      ret = RSEMIB_BAD_INPUT;
  } else {
      switch (reqinfo->mode){
          case MODE_GET:
              DEBUGMSGTL((MY_NAME, "GET:\n"));
              if(RSEMIB_OK == (ret = get_rsuTimeSincePowerOn(&data))){
                  snmp_set_var_typed_integer(requests->requestvb, ASN_COUNTER, data);
              }
              break;
          default:
              ret = RSEMIB_BAD_MODE;
              break;
      }
      /* scalars and like are always index 0 with unqiue column.*/
      print_request_result(0, SYSTEM_STATS_TIME_SINCE_POWER_ON_OID,requests,reqinfo->mode,ret);
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
/* 5.13.2 */
int32_t handle_rsuIntTemp(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo,
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
              if(RSEMIB_OK == (ret = get_rsuIntTemp(&data))){
                  snmp_set_var_typed_integer(requests->requestvb, ASN_INTEGER, data);
              }
              break;
          default:
              ret = RSEMIB_BAD_MODE;
              break;
      }
      /* scalars and like are always index 0 with unqiue column.*/
      print_request_result(0, SYSTEM_STATS_INT_TEMP_OID,requests,reqinfo->mode,ret);
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
/* 5.13.3 */
int32_t handle_rsuIntTempLowThreshold(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo,
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
              if(RSEMIB_OK == (ret = get_rsuIntTempLowThreshold(&data))){
                  snmp_set_var_typed_integer(requests->requestvb, ASN_INTEGER, data);
              }
              break;
          default:
              ret = RSEMIB_BAD_MODE;
              break;
      }
      /* scalars and like are always index 0 with unqiue column.*/
      print_request_result(0, SYSTEM_STATS_INT_TEMP_LOW_THRESH_OID,requests,reqinfo->mode,ret);
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
/* 5.13.4 */
int32_t handle_rsuIntTempHighThreshold(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo,
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
              if(RSEMIB_OK == (ret = get_rsuIntTempHighThreshold(&data))){
                  snmp_set_var_typed_integer(requests->requestvb, ASN_INTEGER, data);
              }
              break;
          default:
              ret = RSEMIB_BAD_MODE;
              break;
      }
      /* scalars and like are always index 0 with unqiue column.*/
      print_request_result(0, SYSTEM_STATS_INT_TEMP_HIGH_THRESH_OID,requests,reqinfo->mode,ret);
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
/* 5.13.5 */
int32_t handle_maxRsuCommRange(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo,
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
              if(RSEMIB_OK == (ret = get_maxRsuCommRange(&data))){
                  snmp_set_var_typed_integer(requests->requestvb, ASN_INTEGER, data);
              }
              break;
          default:
              ret = RSEMIB_BAD_MODE;
              break;
      }
      /* scalars and like are always index 0 with unqiue column.*/
      print_request_result(0, COMM_RANGE_MAX_OID,requests,reqinfo->mode,ret);
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
/* 5.13.6.1 */
int32_t handle_rsuCommRangeIndex(int32_t index,netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret  = RSEMIB_OK;
  int32_t data = index;

  DEBUGMSGTL((MY_NAME, "%s: Entry:\n",__FUNCTION__));
  switch (reqinfo->mode){
      case MODE_GET:
#if 0
          if(RSEMIB_OK == (ret = get_rsuCommRangeIndex(index, &data)))
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
/* 5.13.6.2 */
static int32_t handle_rsuCommRangeSector(int32_t index,netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret  = RSEMIB_OK;
  int32_t data = 0;

  DEBUGMSGTL((MY_NAME, "%s: Entry:\n",__FUNCTION__));
  switch (reqinfo->mode){
      case MODE_GET:
          DEBUGMSGTL((MY_NAME, "GET:\n"));
          if(RSEMIB_OK == (ret = get_rsuCommRangeSector(index,&data))){
              snmp_set_var_typed_integer(requests->requestvb, ASN_INTEGER, data);
          }
          break;
      case MODE_SET_RESERVE1: /* Short stack, all requests done in one. */
          DEBUGMSGTL((MY_NAME, "R1:\n"));
          if(RSEMIB_OK == (ret = preserve_rsuCommRangeSector(index))){
              if(RSEMIB_OK == (ret = action_rsuCommRangeSector(index,*(requests->requestvb->val.integer)))) {
                  if(RSEMIB_OK == (ret = commit_rsuCommRangeSector(index))){
                      DEBUGMSGTL((MY_NAME, "COMMIT DONE:\n"));
                  } else {
                      DEBUGMSGTL((MY_NAME, "COMMIT FAIL:\n"));
                  }
              } else {
                  DEBUGMSGTL((MY_NAME, "ACTION FAIL:\n"));
              }
              if(RSEMIB_OK != ret) {
                  if(RSEMIB_OK != undo_rsuCommRangeSector(index)) { /* UNDO fails nothing we can do. */
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
/* 5.13.6.3 */
static int32_t handle_rsuCommRangeMsgId(int32_t index,netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret  = RSEMIB_OK;
  int32_t data = 0;

  DEBUGMSGTL((MY_NAME, "%s: Entry:\n",__FUNCTION__));
  switch (reqinfo->mode){
      case MODE_GET:
          DEBUGMSGTL((MY_NAME, "GET:\n"));
          if(RSEMIB_OK == (ret = get_rsuCommRangeMsgId(index,&data))){
              snmp_set_var_typed_integer(requests->requestvb, ASN_INTEGER, data);
          }
          break;
      case MODE_SET_RESERVE1: /* Short stack, all requests done in one. */
          DEBUGMSGTL((MY_NAME, "R1:\n"));
          if(RSEMIB_OK == (ret = preserve_rsuCommRangeMsgId(index))){
              if(RSEMIB_OK == (ret = action_rsuCommRangeMsgId(index,*(requests->requestvb->val.integer)))) {
                  if(RSEMIB_OK == (ret = commit_rsuCommRangeMsgId(index))){
                      DEBUGMSGTL((MY_NAME, "COMMIT DONE:\n"));
                  } else {
                      DEBUGMSGTL((MY_NAME, "COMMIT FAIL:\n"));
                  }
              } else {
                  DEBUGMSGTL((MY_NAME, "ACTION FAIL:\n"));
              }
              if(RSEMIB_OK != ret) {
                  if(RSEMIB_OK != undo_rsuCommRangeMsgId(index)) { /* UNDO fails nothing we can do. */
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
/* 5.13.6.4 */
static int32_t handle_rsuCommRangeFilterType(int32_t index,netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret  = RSEMIB_OK;
  int32_t data = 0;

  DEBUGMSGTL((MY_NAME, "%s: Entry:\n",__FUNCTION__));
  switch (reqinfo->mode){
      case MODE_GET:
          if(RSEMIB_OK == (ret = get_rsuCommRangeFilterType(index,&data))){
              snmp_set_var_typed_integer(requests->requestvb, ASN_INTEGER, data);
          }
          break;
      case MODE_SET_RESERVE1: /* Short stack, all requests done in one. */
          DEBUGMSGTL((MY_NAME, "R1:\n"));
          if(RSEMIB_OK == (ret = preserve_rsuCommRangeFilterType(index))){
              if(RSEMIB_OK == (ret = action_rsuCommRangeFilterType(index,*(requests->requestvb->val.integer)))) {
                  if(RSEMIB_OK == (ret = commit_rsuCommRangeFilterType(index))){
                      DEBUGMSGTL((MY_NAME, "COMMIT DONE:\n"));
                  } else {
                      DEBUGMSGTL((MY_NAME, "COMMIT FAIL:\n"));
                  }
              } else {
                  DEBUGMSGTL((MY_NAME, "ACTION FAIL:\n"));
              }
              if(RSEMIB_OK != ret) {
                  if(RSEMIB_OK != undo_rsuCommRangeFilterType(index)) { /* UNDO fails nothing we can do. */
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
/* 5.13.6.5 */
static int32_t handle_rsuCommRangeFilterValue(int32_t index,netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret  = RSEMIB_OK;
  int32_t data = 0;

  DEBUGMSGTL((MY_NAME, "%s: Entry:\n",__FUNCTION__));
  switch (reqinfo->mode){
      case MODE_GET:
          DEBUGMSGTL((MY_NAME, "GET:\n"));
          if(RSEMIB_OK == (ret = get_rsuCommRangeFilterValue(index,&data))){
              snmp_set_var_typed_integer(requests->requestvb, ASN_INTEGER, data);
          }
          break;
      case MODE_SET_RESERVE1: /* Short stack, all requests done in one. */
          DEBUGMSGTL((MY_NAME, "R1:\n"));
          if(RSEMIB_OK == (ret = preserve_rsuCommRangeFilterValue(index))){
              if(RSEMIB_OK == (ret = action_rsuCommRangeFilterValue(index,*(requests->requestvb->val.integer)))) {
                  if(RSEMIB_OK == (ret = commit_rsuCommRangeFilterValue(index))){
                      DEBUGMSGTL((MY_NAME, "COMMIT DONE:\n"));
                  } else {
                      DEBUGMSGTL((MY_NAME, "COMMIT FAIL:\n"));
                  }
              } else {
                  DEBUGMSGTL((MY_NAME, "ACTION FAIL:\n"));
              }
              if(RSEMIB_OK != ret) {
                  if(RSEMIB_OK != undo_rsuCommRangeFilterValue(index)) { /* UNDO fails nothing we can do. */
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
/* 5.13.6.6 */
static int32_t handle_rsuCommRange1Min(int32_t index,netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret  = RSEMIB_OK;
  int32_t data = 0;

  DEBUGMSGTL((MY_NAME, "%s: Entry:\n",__FUNCTION__));
  switch (reqinfo->mode){
      case MODE_GET:
          if(RSEMIB_OK == (ret = get_rsuCommRange1Min(index,&data))){
              snmp_set_var_typed_integer(requests->requestvb, ASN_INTEGER, data);
          }
          break;
      default:
          ret = RSEMIB_BAD_MODE;
          break;
  }
  return ret;
}
/* 5.13.6.7 */
static int32_t handle_rsuCommRange5Min(int32_t index,netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret  = RSEMIB_OK;
  int32_t data = 0;

  DEBUGMSGTL((MY_NAME, "%s: Entry:\n",__FUNCTION__));
  switch (reqinfo->mode){
      case MODE_GET:
          DEBUGMSGTL((MY_NAME, "GET:\n"));
          if(RSEMIB_OK == (ret = get_rsuCommRange5Min(index,&data))){
              snmp_set_var_typed_integer(requests->requestvb, ASN_INTEGER, data);
          }
          break;
      default:
          ret = RSEMIB_BAD_MODE;
          break;
  }
  return ret;
}
/* 5.13.6.8 */
static int32_t handle_rsuCommRange15Min(int32_t index,netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret  = RSEMIB_OK;
  int32_t data = 0;

  DEBUGMSGTL((MY_NAME, "%s: Entry:\n",__FUNCTION__));
  switch (reqinfo->mode){
      case MODE_GET:
          DEBUGMSGTL((MY_NAME, "GET:\n"));
          if(RSEMIB_OK == (ret = get_rsuCommRange15Min(index,&data))){
              snmp_set_var_typed_integer(requests->requestvb, ASN_INTEGER, data);
          }
          break;
      default:
          ret = RSEMIB_BAD_MODE;
          break;
  }
  return ret;
}
/* 5.13.6.9 */
static int32_t handle_rsuCommRangeAvg1Min(int32_t index,netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret  = RSEMIB_OK;
  int32_t data = 0;

  DEBUGMSGTL((MY_NAME, "%s: Entry:\n",__FUNCTION__));
  switch (reqinfo->mode){
      case MODE_GET:
          DEBUGMSGTL((MY_NAME, "GET:\n"));
          if(RSEMIB_OK == (ret = get_rsuCommRangeAvg1Min(index,&data))){
              snmp_set_var_typed_integer(requests->requestvb, ASN_INTEGER, data);
          }
          break;
      default:
          ret = RSEMIB_BAD_MODE;
          break;
  }
  return ret;
}
/* 5.13.6.10 */
static int32_t handle_rsuCommRangeAvg5Min(int32_t index,netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret  = RSEMIB_OK;
  int32_t data = 0;

  DEBUGMSGTL((MY_NAME, "%s: Entry:\n",__FUNCTION__));
  switch (reqinfo->mode){
      case MODE_GET:
          DEBUGMSGTL((MY_NAME, "GET:\n"));
          if(RSEMIB_OK == (ret = get_rsuCommRangeAvg5Min(index,&data))){
              snmp_set_var_typed_integer(requests->requestvb, ASN_INTEGER, data);
          }
          break;
      default:
          ret = RSEMIB_BAD_MODE;
          break;
  }
  return ret;
}
/* 5.13.6.11 */
static int32_t handle_rsuCommRangeAvg15Min(int32_t index,netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret  = RSEMIB_OK;
  int32_t data = 0;

  DEBUGMSGTL((MY_NAME, "%s: Entry:\n",__FUNCTION__));
  switch (reqinfo->mode){
      case MODE_GET:
          DEBUGMSGTL((MY_NAME, "GET:\n"));
          if(RSEMIB_OK == (ret = get_rsuCommRangeAvg15Min(index,&data))){
              snmp_set_var_typed_integer(requests->requestvb, ASN_INTEGER, data);
          }
          break;
      default:
          ret = RSEMIB_BAD_MODE;
          break;
  }
  return ret;
}
/* 5.13.6.12 */
static int32_t handle_rsuCommRangeStatus(int32_t index,netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests, int32_t override)
{
  int32_t ret  = RSEMIB_OK;
  int32_t data = 0;
  int32_t row_status = SNMP_ROW_CREATEANDGO; /* If override is set then don't use requests's value. Use this one. */

  DEBUGMSGTL((MY_NAME, "%s: Entry:\n",__FUNCTION__));
  switch (reqinfo->mode){
      case MODE_GET:
          if(RSEMIB_OK == (ret = get_rsuCommRangeStatus(index,&data))){
              snmp_set_var_typed_integer(requests->requestvb, ASN_INTEGER, data);
          }
          break;
      case MODE_SET_RESERVE1: /* Short stack, all requests done in one. */
          DEBUGMSGTL((MY_NAME, "R1:\n"));
          if(0 == override){
              row_status = *(requests->requestvb->val.integer); /* Client index part of CREATE. */
          }
          if(RSEMIB_OK == (ret = preserve_rsuCommRangeStatus(index))){
              if(RSEMIB_OK == (ret = action_rsuCommRangeStatus(index,row_status))) {
                  if(RSEMIB_OK == (ret = commit_rsuCommRangeStatus(index))){
                      DEBUGMSGTL((MY_NAME, "COMMIT DONE:\n"));
                  } else {
                      DEBUGMSGTL((MY_NAME, "COMMIT FAIL:\n"));
                  }
              } else {
                  DEBUGMSGTL((MY_NAME, "ACTION FAIL:\n"));
              }
              if(RSEMIB_OK != ret) {
                  if(RSEMIB_OK != undo_rsuCommRangeStatus(index)) { /* UNDO fails nothing we can do. */
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
 * 5.13.2: Table handler: Break it down to smaller requests. 
 */
int32_t handle_rsuCommRangeTable( netsnmp_mib_handler          * handler
                                , netsnmp_handler_registration * reginfo
                                , netsnmp_agent_request_info   * reqinfo
                                , netsnmp_request_info         * requests)
{
  netsnmp_request_info       * request       = NULL;
  uint32_t                     request_count = 0;
  RsuCommRangeEntry_t        * table_entry   = NULL;
  netsnmp_table_request_info * request_info  = NULL;
  int32_t                      ret           = SNMP_ERR_NOERROR;
  uint8_t                      raw_oid[MAX_OID_LEN + 2]; /* If table doesn't exist then helper can't help. Go fish for row index. */
  uint32_t                     row_index = 0; /* For readability. Note: index is 1..max */
  int32_t                      row_status = SNMP_ROW_NONEXISTENT;

  handler = handler; /* Silence of the Warnings. */
  reginfo = reginfo;

  DEBUGMSGTL((MY_NAME, "rsuCommRangeTable_handler: Entry:\n"));
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
      if(NULL != (table_entry = (RsuCommRangeEntry_t *)netsnmp_tdata_extract_entry(request))) {
          if((request_info->colnum < COMM_RANGE_INDEX_COLUMN) || (COMM_RANGE_STATUS_COLUMN < request_info->colnum)) {
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
          if(COMM_RANGE_STATUS_COLUMN != request_info->colnum){/* If row doesn't exist then create now. */
              ret = handle_rsuCommRangeStatus(row_index, reqinfo, request, 1); /* Do not use value in request. Force createAndGo. */
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
          case COMM_RANGE_INDEX_COLUMN:
              ret = handle_rsuCommRangeIndex(row_index, reqinfo, request);
              break;
          case COMM_RANGE_SECTOR_COLUMN:
              ret = handle_rsuCommRangeSector(row_index, reqinfo, request);
              break;
          case COMM_RANGE_MSG_ID_COLUMN:
              ret = handle_rsuCommRangeMsgId(row_index, reqinfo, request);
              break;
          case COMM_RANGE_FILTER_TYPE_COLUMN:
              ret = handle_rsuCommRangeFilterType(row_index, reqinfo, request);
              break;
          case COMM_RANGE_FILTER_VALUE_COLUMN:
              ret = handle_rsuCommRangeFilterValue(row_index, reqinfo, request);
              break;
          case COMM_RANGE_1_MIN_COLUMN:
              ret = handle_rsuCommRange1Min(row_index, reqinfo, request);
              break;
          case COMM_RANGE_5_MIN_COLUMN:
              ret = handle_rsuCommRange5Min(row_index, reqinfo, request);
              break;
          case COMM_RANGE_15_MIN_COLUMN:
              ret = handle_rsuCommRange15Min(row_index, reqinfo, request);
              break;
          case COMM_RANGE_AVG_1_MIN_COLUMN:
              ret = handle_rsuCommRangeAvg1Min(row_index, reqinfo, request);
              break;
          case COMM_RANGE_AVG_5_MIN_COLUMN:
              ret = handle_rsuCommRangeAvg5Min(row_index, reqinfo, request);
              break;
          case COMM_RANGE_AVG_15_MIN_COLUMN:
              ret = handle_rsuCommRangeAvg15Min(row_index, reqinfo, request);
              break;
          case COMM_RANGE_STATUS_COLUMN:
//If already created and status matches then who cares?
              ret = handle_rsuCommRangeStatus(row_index, reqinfo, request,0);
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

/* rsuCommRangeTable creation functions.*/
STATIC void rsuCommRangeTable_removeEntry(netsnmp_tdata * table_data, netsnmp_tdata_row * row)
{
  RsuCommRangeEntry_t * entry = NULL;

  if (NULL == row){
      DEBUGMSGTL((MY_NAME,"rsuCommRangeTable_removeEntry NULL input.\n"));
      return;
  }
  if (NULL != (entry = (RsuCommRangeEntry_t *)row->data)){ /* We allocate, so we free. */
      SNMP_FREE(entry);
  } else {
      DEBUGMSGTL((MY_NAME,"rsuXmitMsgFwdingTable_removeEntry row data is NULL!.\n"));
  }
  if (table_data){
      netsnmp_tdata_remove_and_delete_row(table_data, row);
  } else {
      DEBUGMSGTL((MY_NAME,"rsuCommRangeTable_removeEntry NULL input.\n"));
      netsnmp_tdata_delete_row(row); 
  }
  return;
}
static void destroy_rsuCommRangeTable(void)
{
  netsnmp_tdata_row  * row     = NULL;
  netsnmp_tdata_row  * nextrow = NULL;
  int32_t              i       = 0;

  if(NULL != rsuCommRangeTable){
      if(NULL != (row = netsnmp_tdata_row_first(rsuCommRangeTable))){
          for(i=0;(row != NULL) && (i < RSU_COMM_RANGE_MAX); i++)   {
              nextrow = netsnmp_tdata_row_next(rsuCommRangeTable, row);
              rsuCommRangeTable_removeEntry(rsuCommRangeTable, row);
              row = nextrow;
          }
      } else {
          DEBUGMSGTL((MY_NAME, "destroy_rsuCommRangeTable: Table empty. Nothing to do.\n"));
      }
  } else {
      DEBUGMSGTL((MY_NAME, "destroy_rsuCommRangeTable: Table is NULL. Did we get called before install?\n"));
  }
}
static int32_t rsuCommRange_createTable(netsnmp_tdata * table_data, RsuCommRangeEntry_t * rsuCommRangeTable)
{
  RsuCommRangeEntry_t * entry = NULL;
  netsnmp_tdata_row   * row   = NULL;
  int32_t               i     = 0;
  int32_t               valid_row_count = 0;

  if(NULL == table_data){
      DEBUGMSGTL((MY_NAME, "rsuCommRange_createTable FAIL: table_data == NULL.\n"));
      return -1;
  }

  for(i=0; i < RSU_COMM_RANGE_MAX; i++){
      if((rsuCommRangeTable[i].rsuCommRangeIndex < 1) || (RSU_COMM_RANGE_MAX < rsuCommRangeTable[i].rsuCommRangeIndex)){
          return -1; /* Indices must be unique and greater than 1. */
      }
      if(RSEMIB_OK != valid_table_row(rsuCommRangeTable[i].rsuCommRangeStatus)){
          continue; /* Only add rows that have beeb created. */
      }
      if (NULL == (entry = SNMP_MALLOC_TYPEDEF(RsuCommRangeEntry_t))){
          DEBUGMSGTL((MY_NAME, "rsuCommRangeTable SNMP_MALLOC_TYPEDEF FAIL.\n"));
          return -1;
      }
      if (NULL == (row = netsnmp_tdata_create_row())){
          DEBUGMSGTL((MY_NAME, "rsuCommRangeTable netsnmp_tdata_create_row FAIL.\n"));
          SNMP_FREE(entry);
          return -1;
      }

      DEBUGMSGTL((MY_NAME, "rsuCommRangeTable make row i=%d index=%d.\n",i,rsuCommRangeTable[i].rsuCommRangeIndex));
      row->data = entry;
      memcpy(entry,&rsuCommRangeTable[i],sizeof(RsuCommRangeEntry_t));
      netsnmp_tdata_row_add_index(row, ASN_INTEGER , &rsuCommRangeTable[i].rsuCommRangeIndex, sizeof(int32_t));
      if(SNMPERR_SUCCESS != netsnmp_tdata_add_row(table_data, row)){
          DEBUGMSGTL((MY_NAME, "rsuCommRangeTable netsnmp_tdata_add_row FAIL i=%d.\n",i));
          return -1;
      }
      valid_row_count++;
  } /* for */
  DEBUGMSGTL((MY_NAME, "rsuCommRangeTable valid rows created = %d.\n",valid_row_count));
  return RSEMIB_OK;
}
static int32_t install_rsuCommRangeTable(void)
{
    netsnmp_handler_registration * reg = NULL;

    reg = netsnmp_create_handler_registration("rsuCommRangeTable"
                                              ,handle_rsuCommRangeTable
                                              ,rsuCommRangeTable_oid
                                              ,OID_LENGTH(rsuCommRangeTable_oid)
                                              ,HANDLER_CAN_RWRITE);

    if (NULL == (rsuCommRangeTable = netsnmp_tdata_create_table("rsuCommRangeTable", 0))){
        DEBUGMSGTL((MY_NAME, "rsuCommRangeTable FAIL.\n"));
        return RSEMIB_INSTALL_HANDLER_FAIL;
    }
    if (NULL == (table_reg_info = SNMP_MALLOC_TYPEDEF(netsnmp_table_registration_info))){
        DEBUGMSGTL((MY_NAME, "SNMP_MALLOC_TYPEDEF FAILED.\n"));
        return RSEMIB_INSTALL_HANDLER_FAIL;
    }
    netsnmp_table_helper_add_indexes(table_reg_info, ASN_INTEGER,0); /* index */                                 

    table_reg_info->min_column = COMM_RANGE_SECTOR_COLUMN; /* Hide index by definition. */
    table_reg_info->max_column = COMM_RANGE_STATUS_COLUMN;

    if (SNMPERR_SUCCESS == netsnmp_tdata_register(reg, rsuCommRangeTable, table_reg_info)){
        return RSEMIB_OK; /* Table registered but still empty. rebuild_rsuCommRangeTable() will populate */
    } else {
        DEBUGMSGTL((MY_NAME, "netsnmp_tdata_register FAIL.\n"));
        return RSEMIB_INSTALL_HANDLER_FAIL;
    }
}
/* Assumed locked already. Live pointer to db. */
int32_t rebuild_rsuCommRange_live(RsuCommRangeEntry_t * rsuCommRange)
{
  if(NULL == rsuCommRange) {
      return RSEMIB_BAD_INPUT;
  }
  /* Remove rows but don't remove table reg. */
  destroy_rsuCommRangeTable();

  /* Initialize the contents of the table here.*/
  if (RSEMIB_OK != rsuCommRange_createTable(rsuCommRangeTable, rsuCommRange)){
      DEBUGMSGTL((MY_NAME, "rebuild_rsuCommRange_live: ERROR: Null row.\n"));
      set_error_states(RSEMIB_REBUILD_FAIL);
      return RSEMIB_REBUILD_FAIL;
  }
  return RSEMIB_OK;
}
static void rebuild_rsuCommRangeTable(void)
{
  int32_t i = 0;
  RsuCommRangeEntry_t rsuCommRange[RSU_COMM_RANGE_MAX];

  memset(rsuCommRange,0x0,sizeof(rsuCommRange));
  for(i=0; i < RSU_COMM_RANGE_MAX; i++){
      if(RSEMIB_OK > get_rsuCommRangeIndex(i+1,&rsuCommRange[i].rsuCommRangeIndex)){
          set_error_states(RSEMIB_REBUILD_FAIL);
          DEBUGMSGTL((MY_NAME, "rsuCommRangeIndex rebuild FAIL: i = %d.\n",i));
      }
      if(RSEMIB_OK > get_rsuCommRangeSector(i+1,&rsuCommRange[i].rsuCommRangeSector)){
          set_error_states(RSEMIB_REBUILD_FAIL);
          DEBUGMSGTL((MY_NAME, "rsuCommRangeSector rebuild FAIL: i = %d.\n",i));
      }
      if(RSEMIB_OK > get_rsuCommRangeMsgId(i+1,&rsuCommRange[i].rsuCommRangeMsgId)){
          set_error_states(RSEMIB_REBUILD_FAIL);
          DEBUGMSGTL((MY_NAME, "rsuCommRangeMsgId rebuild FAIL: i = %d.\n",i));
      }
      if(RSEMIB_OK > get_rsuCommRangeFilterType(i+1,&rsuCommRange[i].rsuCommRangeFilterType)){
          set_error_states(RSEMIB_REBUILD_FAIL);
          DEBUGMSGTL((MY_NAME, "rsuCommRangeFilterType rebuild FAIL: i = %d.\n",i));
      }
      if(RSEMIB_OK > get_rsuCommRangeFilterValue(i+1,&rsuCommRange[i].rsuCommRangeFilterValue)){
          set_error_states(RSEMIB_REBUILD_FAIL);
          DEBUGMSGTL((MY_NAME, "rsuCommRangeFilterValue rebuild FAIL: i = %d.\n",i));
      }
      if(RSEMIB_OK > get_rsuCommRange1Min(i+1,&rsuCommRange[i].rsuCommRange1Min)){
          set_error_states(RSEMIB_REBUILD_FAIL);
          DEBUGMSGTL((MY_NAME, "rsuCommRange1Min rebuild FAIL: i = %d.\n",i));
      }
      if(RSEMIB_OK > get_rsuCommRange5Min(i+1,&rsuCommRange[i].rsuCommRange5Min)){
          set_error_states(RSEMIB_REBUILD_FAIL);
          DEBUGMSGTL((MY_NAME, "rsuCommRange5Min rebuild FAIL: i = %d.\n",i));
      }
      if(RSEMIB_OK > get_rsuCommRange15Min(i+1,&rsuCommRange[i].rsuCommRange15Min)){
          set_error_states(RSEMIB_REBUILD_FAIL);
          DEBUGMSGTL((MY_NAME, "rsuCommRange15Min rebuild FAIL: i = %d.\n",i));
      }
      if(RSEMIB_OK > get_rsuCommRangeAvg1Min(i+1,&rsuCommRange[i].rsuCommRangeAvg1Min)){
          set_error_states(RSEMIB_REBUILD_FAIL);
          DEBUGMSGTL((MY_NAME, "rsuCommRangeAvg1Minrebuild FAIL: i = %d.\n",i));
      }
      if(RSEMIB_OK > get_rsuCommRangeAvg5Min(i+1,&rsuCommRange[i].rsuCommRangeAvg5Min)){
          set_error_states(RSEMIB_REBUILD_FAIL);
          DEBUGMSGTL((MY_NAME, "rsuCommRangeAvg5in rebuild FAIL: i = %d.\n",i));
      }
      if(RSEMIB_OK > get_rsuCommRangeAvg15Min(i+1,&rsuCommRange[i].rsuCommRangeAvg15Min)){
          set_error_states(RSEMIB_REBUILD_FAIL);
          DEBUGMSGTL((MY_NAME, "rsuCommRangeAvg15Min rebuild FAIL: i = %d.\n",i));
      }
      if(RSEMIB_OK > get_rsuCommRangeStatus(i+1,&rsuCommRange[i].rsuCommRangeStatus)){
          set_error_states(RSEMIB_REBUILD_FAIL);
          DEBUGMSGTL((MY_NAME, "rsuCommRangeStatus rebuild FAIL: i = %d.\n",i));
      }
  }
  /* Remove rows but don't remove table reg. */
  destroy_rsuCommRangeTable();

  /* Initialize the contents of the table here.*/
  if (RSEMIB_OK != rsuCommRange_createTable(rsuCommRangeTable, &rsuCommRange[0])){
      DEBUGMSGTL((MY_NAME, "rebuild_rsuCommRange: ERROR: Null row.\n"));
      set_error_states(RSEMIB_REBUILD_FAIL);
  }
}
void install_rsuSystemStats_handlers(void)
{
  /* Reset statics across soft reset. */
  rsuCommRangeTable           = NULL;
  table_reg_info              = NULL;
  rsuSystemStats_error_states = 0x0;

  /* 5.13.1: */
  if(MIB_REGISTERED_OK != netsnmp_register_scalar(netsnmp_create_handler_registration
                          ("rsuTimeSincePowerOn", 
                           handle_rsuTimeSincePowerOn,
                           rsuTimeSincePowerOn_oid,
                           OID_LENGTH(rsuTimeSincePowerOn_oid),
                           HANDLER_CAN_RONLY))){
      DEBUGMSGTL((MY_NAME, "rsuTimeSincePowerOn install FAIL.\n"));
      set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
  }
  /* 5.13.2: */
  if(MIB_REGISTERED_OK != netsnmp_register_scalar(netsnmp_create_handler_registration
                          ("rsuIntTemp", 
                           handle_rsuIntTemp,
                           rsuIntTemp_oid,
                           OID_LENGTH(rsuIntTemp_oid),
                           HANDLER_CAN_RONLY))){
      DEBUGMSGTL((MY_NAME, "rsuIntTemp install FAIL.\n"));
      set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
  }
  /* 5.13.3: */
  if(MIB_REGISTERED_OK != netsnmp_register_scalar(netsnmp_create_handler_registration
                          ("rsuIntTempLowThreshold", 
                           handle_rsuIntTempLowThreshold,
                           rsuIntTempLowThreshold_oid,
                           OID_LENGTH(rsuIntTempLowThreshold_oid),
                           HANDLER_CAN_RONLY))){
      DEBUGMSGTL((MY_NAME, "rsuIntTempLowThreshold install FAIL.\n"));
      set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
  }
  /* 5.13.4: */
  if(MIB_REGISTERED_OK != netsnmp_register_scalar(netsnmp_create_handler_registration
                          ("rsuIntTempHighThreshold", 
                           handle_rsuIntTempHighThreshold,
                           rsuIntTempHighThreshold_oid,
                           OID_LENGTH(rsuIntTempHighThreshold_oid),
                           HANDLER_CAN_RONLY))){
      DEBUGMSGTL((MY_NAME, "rsuIntTempHighThreshold install FAIL.\n"));
      set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
  }
  /* 5.13.5: */
  if(MIB_REGISTERED_OK != netsnmp_register_scalar(netsnmp_create_handler_registration
                          ("maxRsuCommRange", 
                           handle_maxRsuCommRange,
                           maxRsuCommRange_oid,
                           OID_LENGTH(maxRsuCommRange_oid),
                           HANDLER_CAN_RONLY))){
      DEBUGMSGTL((MY_NAME, "maxRsuCommRange install FAIL.\n"));
      set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
  }
  /* 5.13.2.1 - 5.13.2.12: Install table created with data from MIB. */
  if( RSEMIB_OK != install_rsuCommRangeTable()){
      DEBUGMSGTL((MY_NAME, "rsuCommRangeStatusTable install FAIL.\n"));
      set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
  }

  /* Populate the table. */
  rebuild_rsuCommRangeTable();
}
void destroy_rsuSystemStats(void)
{
  destroy_rsuCommRangeTable();

  /* Free table info. */
  if (NULL != table_reg_info){
      SNMP_FREE(table_reg_info);
  }
  /* Free table reg. */
  if (NULL != rsuCommRangeTable){
      SNMP_FREE(rsuCommRangeTable);
  }
}
/* RSU App thread support. */

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
void *commRangeThr(void __attribute__((unused)) *arg)
{
  int32_t i = 0;
  CommRangeStats_t * shm_commRange_ptr = NULL;     
  size_t shm_commRange_size = sizeof(CommRangeStats_t) * RSU_COMM_RANGE_MAX;

  DEBUGMSGTL((MY_NAME,"commRangeThr: Entry.\n"));

//TODO: LOCK SHM.

  /* Open SHM. */
  if ((shm_commRange_ptr = mib_share_init(shm_commRange_size, COMM_RANGE_SHM_PATH)) == NULL) {
      DEBUGMSGTL((MY_NAME,"commRangeThr: SHM init failed.\n"));
  } else {
      while(mainloop) {
          sleep(1);  /* Not super critical, 1 hz tings tings is good. */
          if(RSEMIB_OK == ntcip1218_lock()){
              for(i=0;i<RSU_COMM_RANGE_MAX;i++){
                  if(ntcip_1218_mib.rsuCommRangeTable[i].rsuCommRangeStatus == SNMP_ROW_ACTIVE){
                      /* If RSU marked ACTIVE then grab stats. */
                      if(shm_commRange_ptr[i].CommRangeStatus == SNMP_ROW_ACTIVE) {
                          ntcip_1218_mib.rsuCommRangeTable[i].rsuCommRange1Min = shm_commRange_ptr[i].CommRange1Min_max;
                          ntcip_1218_mib.rsuCommRangeTable[i].rsuCommRange5Min = shm_commRange_ptr[i].CommRange5Min_max;
                          ntcip_1218_mib.rsuCommRangeTable[i].rsuCommRange15Min = shm_commRange_ptr[i].CommRange15Min_max;
                          ntcip_1218_mib.rsuCommRangeTable[i].rsuCommRangeAvg1Min = shm_commRange_ptr[i].CommRangeAvg1Min;
                          ntcip_1218_mib.rsuCommRangeTable[i].rsuCommRangeAvg5Min = shm_commRange_ptr[i].CommRangeAvg5Min;
                          ntcip_1218_mib.rsuCommRangeTable[i].rsuCommRangeAvg15Min = shm_commRange_ptr[i].CommRangeAvg15Min;
                      }
                  }

//TODO: syslog unsupported MsgIds

                  /* Only BSM is supported. FilterType & FilterValue don't cares.*/
                  shm_commRange_ptr[i].CommRangeStatus = ntcip_1218_mib.rsuCommRangeTable[i].rsuCommRangeStatus; 
                  shm_commRange_ptr[i].CommRangeSector = ntcip_1218_mib.rsuCommRangeTable[i].rsuCommRangeSector;
                  shm_commRange_ptr[i].CommRangeMsgId = ntcip_1218_mib.rsuCommRangeTable[i].rsuCommRangeMsgId;
                  shm_commRange_ptr[i].CommRangeFilterType = ntcip_1218_mib.rsuCommRangeTable[i].rsuCommRangeFilterType;
                  shm_commRange_ptr[i].CommRangeFilterValue = ntcip_1218_mib.rsuCommRangeTable[i].rsuCommRangeFilterValue;
              }
              ntcip1218_unlock();
          } else {
              /* Sooner or later it will get the lock. Not critical. */
              set_error_states(RSEMIB_LOCK_FAIL);
              DEBUGMSGTL((MY_NAME,"commRangeThr: RSEMIB_LOCK_FAIL.\n"));
          } 
          DEBUGMSGTL((MY_NAME_EXTRA,"commRangeThr: BEEP.\n"));
      }
      /* Close SHM. */
      mib_share_kill(shm_commRange_ptr, shm_commRange_size );
  }
  DEBUGMSGTL((MY_NAME,"commRangeThr: Exit."));
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
int32_t commit_commRange_to_disk(RsuCommRangeEntry_t * commRange) 
{
  FILE    * file_out       = NULL;
  uint32_t  mib_byte_count = 0;
  uint32_t  write_count    = 0;
  int32_t   ret            = RSEMIB_OK;

  if(NULL == commRange){
      ret = RSEMIB_BAD_INPUT; 
  } else {
      DEBUGMSGTL((MY_NAME, "commit_to_disk:fopen(%s).\n",commRange->filePathName));
      if ((file_out = fopen(commRange->filePathName, "wb")) == NULL){
          DEBUGMSGTL((MY_NAME, "commit_to_disk: fopen fail.\n"));
          ret = RSEMIB_FOPEN_FAIL;
      } else {
          commRange->crc = (uint32_t)crc16((const uint8_t *)commRange, sizeof(RsuCommRangeEntry_t) - 4);
          mib_byte_count = sizeof(RsuCommRangeEntry_t);
          write_count = fwrite((uint8_t *)commRange, mib_byte_count, 1, file_out);
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
