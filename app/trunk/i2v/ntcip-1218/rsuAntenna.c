/**************************************************************************
 *                                                                        *
 *     File Name:  rsuAntenna.c                                           *
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
#define MY_NAME       "Antenna"
#define MY_NAME_EXTRA "XAntenna"
/* Mandatory: For debugging and syslog output. */
#define MY_I2V_NAME    MY_NAME
#define MY_ERR_LEVEL   LEVEL_PRIV    /* from i2v_util.h */

/* table column definitions. */
#define ANTENNA_INDEX_COLUMN 1
#define ANTENNA_LAT_COLUMN   2
#define ANTENNA_LON_COLUMN   3
#define ANTENNA_ELV_COLUMN   4
#define ANTENNA_GAIN_COLUMN  5
#define ANTENNA_DIR_COLUMN   6
/* Theoritical column ids. Make sure they follow in order with columns and unique. */
#define ANTENNA_MAX_OID      7

#define ANTENNA_INDEX_NAME "rsuAntennaIndex"
#define ANTENNA_LAT_NAME   "rsuAntLat"
#define ANTENNA_LON_NAME   "rsuAntLong"
#define ANTENNA_ELV_NAME   "rsuAntElv"
#define ANTENNA_GAIN_NAME  "rsuAntGain"
#define ANTENNA_DIR_NAME   "rsuAntDirection"
#define ANTENNA_MAX_NAME   "maxRsuAntennas"

/******************************************************************************
 * 5.16 Antenna Settings:{ rsu 15 }: rsuAntennaTable: really fixed scalars.
 *
 *  5.16.1 - 5.16.2:
 *
 *    int32_t           maxRsuAntennas;                     RO: RSU_ANTENNA_MAX
 *    RsuAntennaEntry_t rsuAntennaTable[RSU_ANTENNA_MAX];
 *
 * 5.16.2.1 - 5.16.2.6:
 *
 *    typedef struct{
 *        int32_t rsuAntennaIndex; RO: 1..maxRsuAntennas: Must be 1 or greater. 
 *        int32_t rsuAntLat;       RW: -900000000..900000001:   degrees: float scaled to int: 10^-7: 900000001  = unknown = default.
 *        int32_t rsuAntLong;      RW: -1800000000..1800000001: degrees: float scaled to int: 10^-7: 1800000001 = unknown = default.
 *        int32_t rsuAntElv;       RW: -100000..1000001: cm: 1000001 = unknown = default.
 *        int32_t rsuAntGain;      RW: -128..127: dbm 100th's: includes any cable loss: -128 = unknown =default.
 *        int32_t rsuAntDirection; RW: 0..361: deg: 0=north, 90=east etc: 360=uniform gain in horizontal plane: 361=unknown=default.
 *    } RsuAntennaEntry_t;
 *                                  
 ******************************************************************************/

const oid maxRsuAntennas_oid[]  = {1, 3, 6, 1, 4, 1, 1206, 4, 2, 18, 15 , 1};
const oid rsuAntennaTable_oid[] = {1, 3, 6, 1, 4, 1, 1206, 4, 2, 18, 15 , 2};

static netsnmp_table_registration_info * antenna_table_reg_info = NULL;
static netsnmp_tdata * rsuAntennaTable = NULL;
static uint32_t rsuAntenna_error_states = 0x0;

/* 
 * Function Bodies.
 */
static void set_error_states(int32_t error)
{
  /* -1 to -32 */
  if((error < 0) && (error > MIB_ERROR_BASE_2)){
      rsuAntenna_error_states |= (uint32_t)(0x1) << (abs(error) - 1);
  }
}
uint32_t get_rsuAntenna_error_states(void)
{
  return rsuAntenna_error_states;
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
      case ANTENNA_INDEX_COLUMN:
          strcpy(column_name,ANTENNA_INDEX_NAME);
          break;
      case ANTENNA_LAT_COLUMN:
          strcpy(column_name,ANTENNA_LAT_NAME);
          break;
      case ANTENNA_LON_COLUMN:
          strcpy(column_name,ANTENNA_LON_NAME);
          break;
      case ANTENNA_ELV_COLUMN:
          strcpy(column_name,ANTENNA_ELV_NAME);
          break;
      case ANTENNA_GAIN_COLUMN:
          strcpy(column_name,ANTENNA_GAIN_NAME);
          break;
      case ANTENNA_DIR_COLUMN:
          strcpy(column_name,ANTENNA_DIR_NAME);
          break;
      case ANTENNA_MAX_OID:
          strcpy(column_name,ANTENNA_MAX_NAME);
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

/* 5.16.1 */
int32_t handle_maxRsuAntennas(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo,
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
              snmp_set_var_typed_integer(requests->requestvb, ASN_INTEGER, RSU_ANTENNA_MAX);
              ret = RSEMIB_OK;
              break;
          default:
              ret = RSEMIB_BAD_MODE;
              break;
      }
      /* scalars and like are always index 0 with unqiue column.*/
      print_request_result(0, ANTENNA_MAX_OID,requests,reqinfo->mode,ret);
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
/* 5.16.2.1 */
static int32_t handle_rsuAntennaIndex(int32_t index,netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret = RSEMIB_OK;

  DEBUGMSGTL((MY_NAME, "%s: Entry:\n",__FUNCTION__));
  switch (reqinfo->mode){
      case MODE_GET:
          if((1 <= index) && (index <= RSU_ANTENNA_MAX)){
              snmp_set_var_typed_integer(requests->requestvb, ASN_INTEGER, index);
              ret = RSEMIB_OK;
          } else {
              ret = RSEMIB_BAD_INDEX;
          }
          break;
      default:
          ret = RSEMIB_BAD_MODE;
          break;
  }
  return ret;
}
/* 5.16.2.2 */
static int32_t handle_rsuAntLat(int32_t index,netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret  = RSEMIB_OK;
  int32_t data = 0;

  DEBUGMSGTL((MY_NAME, "%s: Entry:\n",__FUNCTION__));
  switch (reqinfo->mode){
      case MODE_GET:
          DEBUGMSGTL((MY_NAME, "GET:\n"));
          if(RSEMIB_OK == (ret = get_rsuAntLat(index,&data))){
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
          if(RSEMIB_OK == (ret = preserve_rsuAntLat(index))){
              ret = action_rsuAntLat(index, *(requests->requestvb->val.integer));
          }
          break;
      case MODE_SET_COMMIT:
          DEBUGMSGTL((MY_NAME, "COMMIT:\n"));
          if(RSEMIB_OK == (ret = commit_rsuAntLat(index))){
              /*
               * Back end magic here.
               */
          }
          break;
      case MODE_SET_UNDO:
          DEBUGMSGTL((MY_NAME, "UNDO:\n"));
          ret = undo_rsuAntLat(index);
          break;
      default:
          ret = RSEMIB_BAD_MODE;
          break;
  }
  return ret;
}
/* 5.16.2.3 */
static int32_t handle_rsuAntLong(int32_t index,netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret  = RSEMIB_OK;
  int32_t data = 0;

  DEBUGMSGTL((MY_NAME, "%s: Entry:\n",__FUNCTION__));
  switch (reqinfo->mode){
      case MODE_GET:
          DEBUGMSGTL((MY_NAME, "GET:\n"));
          if(RSEMIB_OK == (ret = get_rsuAntLong(index,&data))){
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
          if(RSEMIB_OK == (ret = preserve_rsuAntLong(index))){
              ret = action_rsuAntLong(index, *(requests->requestvb->val.integer));
          }
          break;
      case MODE_SET_COMMIT:
          DEBUGMSGTL((MY_NAME, "COMMIT:\n"));
          if(RSEMIB_OK == (ret = commit_rsuAntLong(index))){
              /*
               * Back end magic here.
               */
          }
          break;
      case MODE_SET_UNDO:
          DEBUGMSGTL((MY_NAME, "UNDO:\n"));
          ret = undo_rsuAntLong(index);
          break;
      default:
          ret = RSEMIB_BAD_MODE;
          break;
  }
  return ret;
}
/* 5.16.2.4 */
static int32_t handle_rsuAntElv(int32_t index,netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret  = RSEMIB_OK;
  int32_t data = 0;

  DEBUGMSGTL((MY_NAME, "%s: Entry:\n",__FUNCTION__));
  switch (reqinfo->mode){
      case MODE_GET:
          DEBUGMSGTL((MY_NAME, "GET:\n"));
          if(RSEMIB_OK == (ret = get_rsuAntElv(index,&data))){
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
          if(RSEMIB_OK == (ret = preserve_rsuAntElv(index))){
              ret = action_rsuAntElv(index, *(requests->requestvb->val.integer));
          }
          break;
      case MODE_SET_COMMIT:
          DEBUGMSGTL((MY_NAME, "COMMIT:\n"));
          if(RSEMIB_OK == (ret = commit_rsuAntElv(index))){
              /*
               * Back end magic here.
               */
          }
          break;
      case MODE_SET_UNDO:
          DEBUGMSGTL((MY_NAME, "UNDO:\n"));
          ret = undo_rsuAntElv(index);
          break;
      default:
          ret = RSEMIB_BAD_MODE;
          break;
  }
  return ret;
}
/* 5.16.2.5 */
static int32_t handle_rsuAntGain(int32_t index,netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret  = RSEMIB_OK;
  int32_t data = 0;

  DEBUGMSGTL((MY_NAME, "%s: Entry:\n",__FUNCTION__));
  switch (reqinfo->mode){
      case MODE_GET:
          DEBUGMSGTL((MY_NAME, "GET:\n"));
          if(RSEMIB_OK == (ret = get_rsuAntGain(index,&data))){
              snmp_set_var_typed_integer(requests->requestvb, ASN_INTEGER, data);
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
          if(RSEMIB_OK == (ret = preserve_rsuAntGain(index))){
              ret = action_rsuAntGain(index, *(requests->requestvb->val.integer));
          }
          break;
      case MODE_SET_COMMIT:
          DEBUGMSGTL((MY_NAME, "COMMIT:\n"));
          if(RSEMIB_OK == (ret = commit_rsuAntGain(index))){
              /*
               * Back end magic here.
               */
          }
          break;
      case MODE_SET_UNDO:
          DEBUGMSGTL((MY_NAME, "UNDO:\n"));
          ret = undo_rsuAntGain(index);
          break;
      default:
          ret = RSEMIB_BAD_MODE;
          break;
  }
  return ret;
}
/* 5.16.2.6 */
static int32_t handle_rsuAntDirection(int32_t index,netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret = RSEMIB_OK;

  DEBUGMSGTL((MY_NAME, "%s: Entry:\n",__FUNCTION__));
  switch (reqinfo->mode){
      case MODE_GET:
          DEBUGMSGTL((MY_NAME, "GET:\n"));
          if(0 <= (ret = get_rsuAntDirection(index))){
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
          if(RSEMIB_OK == (ret = preserve_rsuAntDirection(index))){
              ret = action_rsuAntDirection(index, *(requests->requestvb->val.integer));
          }
          break;
      case MODE_SET_COMMIT:
          DEBUGMSGTL((MY_NAME, "COMMIT:\n"));
          if(RSEMIB_OK == (ret = commit_rsuAntDirection(index))){
              /*
               * Back end magic here.
               */
          }
          break;
      case MODE_SET_UNDO:
          DEBUGMSGTL((MY_NAME, "UNDO:\n"));
          ret = undo_rsuAntDirection(index);
          break;
      default:
          ret = RSEMIB_BAD_MODE;
          break;
  }
  return ret;
}
/* 5.16.2 */
int32_t handle_rsuAntennaTable( netsnmp_mib_handler          * handler
                              , netsnmp_handler_registration * reginfo
                              , netsnmp_agent_request_info   * reqinfo
                              , netsnmp_request_info         * requests)
{
  netsnmp_request_info       * request       = NULL;
  uint32_t                     request_count = 0;
  RsuAntennaEntry_t          * table_entry   = NULL;
  netsnmp_table_request_info * request_info  = NULL;
  int32_t                      ret           = SNMP_ERR_NOERROR;

  handler = handler; /* Silence of the Warnings. */
  reginfo = reginfo;

  DEBUGMSGTL((MY_NAME, "%s: Entry:\n",__FUNCTION__));

  /* Service each request and provide netsnmp_set_request_error() result for each. */
  for (request = requests;
          (SNMP_ERR_NOERROR == ret) && request && (request_count < MAX_REQ_COUNT);
               request = request->next,request_count++){

      if (request->processed){
          DEBUGMSGTL((MY_NAME, "request->processed.\n"));
          continue;
      }

      table_entry  = (RsuAntennaEntry_t *)netsnmp_tdata_extract_entry(request);
      request_info = netsnmp_extract_table_info(request);

      if((NULL == table_entry) || (NULL == request_info)){
          set_error_states(RSEMIB_BAD_REQUEST_TABLE_INFO);
          DEBUGMSGTL((MY_NAME, "NULL == request_info.\n"));
          ret = SNMP_ERR_INCONSISTENTVALUE;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"RSEMIB_BAD_REQUEST_TABLE_INFO.\n");
      } else {
           DEBUGMSGTL((MY_NAME, "COLUMN=%d.\n",request_info->colnum));
           switch (request_info->colnum){
              case ANTENNA_INDEX_COLUMN:
                  ret = handle_rsuAntennaIndex(table_entry->rsuAntennaIndex, reqinfo, request);
                  break;
              case ANTENNA_LAT_COLUMN:
                  ret = handle_rsuAntLat(table_entry->rsuAntennaIndex, reqinfo, request);
                  break;
              case ANTENNA_LON_COLUMN:
                  ret = handle_rsuAntLong(table_entry->rsuAntennaIndex, reqinfo, request);
                  break;
              case ANTENNA_ELV_COLUMN:
                  ret = handle_rsuAntElv(table_entry->rsuAntennaIndex, reqinfo, request);
                  break;
              case ANTENNA_GAIN_COLUMN:
                  ret = handle_rsuAntGain(table_entry->rsuAntennaIndex, reqinfo, request);
                  break;
              case ANTENNA_DIR_COLUMN:
                  ret = handle_rsuAntDirection(table_entry->rsuAntennaIndex, reqinfo, request);
                  break;
              default:
                  /* Ignore requests for columns outside defintion. Dont send error because it will kill walk.*/
                  ret = RSEMIB_BAD_REQUEST_COLUMN;
                  break;
          }
          if(ret < RSEMIB_OK) {
              set_error_states(ret);
          }
          print_request_result(table_entry->rsuAntennaIndex, request_info->colnum,requests,reqinfo->mode,ret);
          ret = handle_rsemib_errors(ret); /* Convert from MIB(negative) error to SNMP(positive) request error. */
      } /* if */
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

STATIC void remove_rsuAntennaEntry(netsnmp_tdata * table_data, netsnmp_tdata_row * row)
{
  RsuAntennaEntry_t * entry = NULL;

  if (NULL == row){
      DEBUGMSGTL((MY_NAME,"remove_rsuAntennaEntry NULL input.\n"));
      return;
  }
  if (NULL != (entry = (RsuAntennaEntry_t *)row->data)){
      SNMP_FREE(entry);
  } else {
      DEBUGMSGTL((MY_NAME,"remove_rsuAntennaEntry row data is NULL!.\n"));
  }
  if (table_data){
      netsnmp_tdata_remove_and_delete_row(table_data, row);
  } else {
      DEBUGMSGTL((MY_NAME,"remove_rsuAntennaEntry NULL input.\n"));
      netsnmp_tdata_delete_row(row); 
  }
  return;
}
void destroy_rsuAntennaTable(void)
{
  netsnmp_tdata_row  * row     = NULL;
  netsnmp_tdata_row  * nextrow = NULL;
  int32_t              i       = 0;

  if(NULL != rsuAntennaTable){
      if(NULL != (row = netsnmp_tdata_row_first(rsuAntennaTable))){
          for(i=0;(row != NULL) && (i < RSU_ANTENNA_MAX);i++){
              nextrow = netsnmp_tdata_row_next(rsuAntennaTable, row);
              remove_rsuAntennaEntry(rsuAntennaTable, row);
              row = nextrow;
          }
      } else {
          DEBUGMSGTL((MY_NAME, "destroy_rsuAntennaTable: Table empty. Nothing to do.\n"));
      }
  } else {
      DEBUGMSGTL((MY_NAME, "destroy_rsuAntennaTable: Table is NULL. Did we get called before install?\n"));
  }
}
static netsnmp_tdata_row * create_rsuAntennaTable(netsnmp_tdata * table_data, RsuAntennaEntry_t * rsuAntennaTable)
{
  RsuAntennaEntry_t * entry = NULL;
  netsnmp_tdata_row * row   = NULL;
  int32_t             i     = 0;
  int32_t             valid_row_count = 0;

  if(NULL == table_data){
      DEBUGMSGTL((MY_NAME, "create_rsuAntennaTable FAIL: table_data == NULL.\n"));
      return NULL;
  }

  for(i=0; i < RSU_ANTENNA_MAX; i++){
      if (NULL == (entry = SNMP_MALLOC_TYPEDEF(RsuAntennaEntry_t))){
          DEBUGMSGTL((MY_NAME, "SNMP_MALLOC_TYPEDEF FAILED.\n"));
          return NULL;
      }
      if (NULL == (row = netsnmp_tdata_create_row())){
          DEBUGMSGTL((MY_NAME, "netsnmp_tdata_create_row FAILED.\n"));
          SNMP_FREE(entry);
          return NULL;
      }
      DEBUGMSGTL((MY_NAME, "rsuAntenna make row i=%d index=%d.\n",i,rsuAntennaTable[i].rsuAntennaIndex));
      row->data = entry;
      memcpy(entry,&rsuAntennaTable[i],sizeof(RsuAntennaEntry_t));
      netsnmp_tdata_row_add_index(row, ASN_INTEGER , &rsuAntennaTable[i].rsuAntennaIndex, sizeof(int32_t));
      if(SNMPERR_SUCCESS != netsnmp_tdata_add_row(table_data, row)){
          DEBUGMSGTL((MY_NAME, "rsuAntenna netsnmp_tdata_add_row FAILED.\n"));
          return NULL;
      }
      valid_row_count++;
  }
  DEBUGMSGTL((MY_NAME, "rsuAntenna valid rows created = %d.\n",valid_row_count));
  return row;
}
static int32_t install_rsuAntennaTable(RsuAntennaEntry_t * rsuAntennas)
{
    netsnmp_handler_registration * reg = NULL;

    reg = netsnmp_create_handler_registration("rsuAntennaTable"
                                              ,handle_rsuAntennaTable
                                              ,rsuAntennaTable_oid
                                              ,OID_LENGTH(rsuAntennaTable_oid)
                                              ,HANDLER_CAN_RWRITE);

    if (NULL == (rsuAntennaTable = netsnmp_tdata_create_table("rsuAntennaTable", 0))){
        DEBUGMSGTL((MY_NAME, "rsuAntennaTable FAILED.\n"));
        return RSEMIB_INSTALL_HANDLER_FAIL;
    }
    if (NULL == (antenna_table_reg_info = SNMP_MALLOC_TYPEDEF(netsnmp_table_registration_info))){
        DEBUGMSGTL((MY_NAME, "SNMP_MALLOC_TYPEDEF FAILED.\n"));
        return RSEMIB_INSTALL_HANDLER_FAIL;
    }
    netsnmp_table_helper_add_indexes(antenna_table_reg_info, ASN_INTEGER,0); /* index */                                 

    antenna_table_reg_info->min_column = ANTENNA_LAT_COLUMN; /* rsuAntLat:  Hide index by definition. */
    antenna_table_reg_info->max_column = ANTENNA_DIR_COLUMN; /* rsuAntDirection: */

    netsnmp_tdata_register(reg, rsuAntennaTable, antenna_table_reg_info);

    /* Initialize the contents of the table. */
    if (NULL == create_rsuAntennaTable(rsuAntennaTable, rsuAntennas)){
        DEBUGMSGTL((MY_NAME, "initialize rsuAntennaTable: ERROR: Null row.\n"));
        return RSEMIB_INSTALL_HANDLER_FAIL;
    }
    return RSEMIB_OK;
}
void install_rsuAntenna_handlers(void)
{
  int32_t i = 0;
  RsuAntennaEntry_t rsuAntennas[RSU_ANTENNA_MAX]; /* Populate local copy to seed fixed ro table we are about to create. */ 
  int32_t dummy = 0;
  DEBUGMSGTL((MY_NAME, "install_rsuAntenna_handlers.\n"));

  /* Clear statics across soft reset. */
  antenna_table_reg_info  = NULL;
  rsuAntennaTable         = NULL;
  rsuAntenna_error_states = 0x0;
  memset(rsuAntennas,0x0,sizeof(rsuAntennas));

  /* Using the functions available to handlers populate table and verify. */
  for(i=0;i<RSU_ANTENNA_MAX;i++){
      rsuAntennas[i].rsuAntennaIndex = i+1; /* index must be 1 or greater. */
      if(RSEMIB_OK != (get_rsuAntLat(i+1,&dummy))){
          set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
      } else {
          rsuAntennas[i].rsuAntLat = dummy;
      }
      if(RSEMIB_OK != (get_rsuAntLong(i+1,&dummy))){
          set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
      } else {
          rsuAntennas[i].rsuAntLong = dummy;
      }
      if(RSEMIB_OK != (get_rsuAntElv(i+1,&dummy))){
          set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
      } else {
          rsuAntennas[i].rsuAntElv = dummy;
      }
      if(RSEMIB_OK != (get_rsuAntGain(i+1,&dummy))){
          set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
      } else {
          rsuAntennas[i].rsuAntGain = dummy;
      }
      if(0 > (rsuAntennas[i].rsuAntDirection = get_rsuAntDirection(i+1))){
          set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
      }
  }
  /* 5.16.1 */
  if(MIB_REGISTERED_OK != netsnmp_register_scalar(netsnmp_create_handler_registration
                          ("maxRsuAntennas",
                           handle_maxRsuAntennas,
                           maxRsuAntennas_oid,
                           OID_LENGTH(maxRsuAntennas_oid),
                           HANDLER_CAN_RONLY))){
      DEBUGMSGTL((MY_NAME, "maxRsuAntennas install FAIL.\n"));
      set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
  }
  DEBUGMSGTL((MY_NAME, "install_rsuAntennaTable.\n"));
  /* 5.16.2.1 - 5.16.2.6: Install table created with data from MIB. */
  if(RSEMIB_OK != install_rsuAntennaTable(&rsuAntennas[0])){
      DEBUGMSGTL((MY_NAME, "rsuAntennaTable install FAIL.\n"));
      set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
  }
}
void destroy_rsuAntenna(void)
{
  destroy_rsuAntennaTable();

  /* Free table info. */
  if (NULL != antenna_table_reg_info){
      SNMP_FREE(antenna_table_reg_info);
  }
  /* Free table reg. */
  if (NULL != rsuAntennaTable){
      SNMP_FREE(rsuAntennaTable);
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
int32_t commit_rsuAntenna_to_disk(RsuAntennaEntry_t * rsuAntenna)
{
  FILE    * file_out       = NULL;
  uint32_t  mib_byte_count = 0;
  uint32_t  write_count    = 0;
  int32_t   ret            = RSEMIB_OK;

  if(NULL == rsuAntenna){
      ret = RSEMIB_BAD_INPUT; 
  } else {
      DEBUGMSGTL((MY_NAME, "commit_to_disk:fopen(%s).\n",rsuAntenna->filePathName));
      if ((file_out = fopen(rsuAntenna->filePathName, "wb")) == NULL){
          DEBUGMSGTL((MY_NAME, "commit_to_disk: fopen fail.\n"));
          ret = RSEMIB_FOPEN_FAIL;
      } else {
          rsuAntenna->crc = (uint32_t)crc16((const uint8_t *)rsuAntenna, sizeof(RsuAntennaEntry_t) - 4);
          mib_byte_count = sizeof(RsuAntennaEntry_t);
          write_count = fwrite((uint8_t *)rsuAntenna, mib_byte_count, 1, file_out);
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

