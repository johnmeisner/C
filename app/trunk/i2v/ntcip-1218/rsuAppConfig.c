/**************************************************************************
 *                                                                        *
 *     File Name:  rsuAppConfig.c                                         *
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
#define MY_NAME        "rsuAppConfig"
/* Mandatory: For debugging and syslog output. */
#define MY_I2V_NAME    "AppCfg"
#define MY_ERR_LEVEL   LEVEL_PRIV    /* from i2v_util.h */

/* table column definitions. */
#define APP_CONFIG_ID_COLUMN      1
#define APP_CONFIG_NAME_COLUMN    2
#define APP_CONFIG_STARTUP_COLUMN 3
#define APP_CONFIG_STATE_COLUMN   4
#define APP_CONFIG_START_COLUMN   5
#define APP_CONFIG_STOP_COLUMN    6
/* Theoritical column ids. Make sure they follow in order with columns and unique. */
#define APP_MAX_OID               7

#define APP_CONFIG_ID_NAME      "rsuAppConfigID"
#define APP_CONFIG_NAME_NAME    "rsuAppConfigName"
#define APP_CONFIG_STARTUP_NAME "rsuAppConfigStartup"
#define APP_CONFIG_STATE_NAME   "rsuAppConfigState"
#define APP_CONFIG_START_NAME   "rsuAppConfigStart"
#define APP_CONFIG_STOP_NAME    "rsuAppConfigStop"
#define APP_MAX_NAME            "maxRsuApps"
/******************************************************************************
 * 5.19 RSU Applications: { rsu 18 }: 32 DSRC MSG ID's.
 *
 *  5.19.1 - 5.19.2:
 * 
 *  int32_t maxRsuApps;
 *  RsuAppConfigEntry_t rsuAppConfigTable[RSU_APPS_RUNNING_MAX];
 *
 *  typedef enum  { 
 *    rsuAppConfigStartup_other      = 1,  When not ready? Like boot time?
 *    rsuAppConfigStartup_onStartup  = 2,  Start at boot automatically.
 *    rsuAppConfigStartup_notStartup = 3,  Do not start automatically at boot.
 *  }rsuAppConfigStartup_e; 
 *
 *  typedef struct  {
 *    int32_t               rsuAppConfigID;                                 RO: 1..RSU_APPS_RUNNING_MAX: index must be 1 or greater.
 *    uint8_t               rsuAppConfigName[RSU_APP_CONF_NAME_LENGTH_MAX]; RO: 1..127 char: name of app and version.
 *    rsuAppConfigStartup_e rsuAppConfigStartup;                            RW: SET to 1(rsuAppConfigStartup_other) returns badValue
 *    int32_t               rsuAppConfigState;                              RO: 0..1: 0 = started, 1 = stopped.
 *    int32_t               rsuAppConfigStart;                              RW: 0..1: SET 1 = start then return to 0. toggle.
 *    int32_t               rsuAppConfigStop;                               RW: 0..1: SET 1 = stop then return to 0. toggle.
 *  } RsuAppConfigEntry_t;
 *
 ******************************************************************************/

const oid maxRsuApps_oid[]        = {1, 3, 6, 1, 4, 1, 1206, 4, 2, 18, 18 , 1 };
const oid rsuAppConfigTable_oid[] = {1, 3, 6, 1, 4, 1, 1206, 4, 2, 18, 18 , 2 };

static netsnmp_table_registration_info * app_table_reg_info = NULL; /* snmp table reg pointer. */
static netsnmp_tdata * rsuAppConfigTable = NULL;                    /* snmp table pointer. */
static uint32_t rsuAppConfig_error_states = 0x0;

/* 
 * Function Bodies.
 */
static void set_error_states(int32_t error)
{
  /* -1 to -32 */
  if((error < 0) && (error > MIB_ERROR_BASE_2)){
      rsuAppConfig_error_states |= (uint32_t)(0x1) << (abs(error) - 1);
  }
}
uint32_t get_rsuAppConfig_error_states(void)
{
  return rsuAppConfig_error_states;
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
      case APP_CONFIG_ID_COLUMN:
          strcpy(column_name,APP_CONFIG_ID_NAME);
          break;
      case APP_CONFIG_NAME_COLUMN:
          strcpy(column_name,APP_CONFIG_NAME_NAME);
          break;
      case APP_CONFIG_STARTUP_COLUMN:
          strcpy(column_name,APP_CONFIG_STARTUP_NAME);
          break;
      case APP_CONFIG_STATE_COLUMN:
          strcpy(column_name,APP_CONFIG_STATE_NAME);
          break;
      case APP_CONFIG_START_COLUMN:
          strcpy(column_name,APP_CONFIG_START_NAME);
          break;
      case APP_CONFIG_STOP_COLUMN:
          strcpy(column_name,APP_CONFIG_STOP_NAME);
          break;
      case APP_MAX_OID:
          strcpy(column_name,APP_MAX_NAME);
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

/* 5.19.1 */
int32_t handle_maxRsuApps(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo,
                            netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret = RSEMIB_OK;

  handler = handler; /* Silence of the Warnings. */
  reginfo = reginfo;
  DEBUGMSGTL((MY_NAME, "%s: Entry:\n",__FUNCTION__));
  if((NULL == reqinfo) || (NULL ==requests)) {
      ret = RSEMIB_BAD_INPUT;
  } else {
      switch (reqinfo->mode){
          case MODE_GET:
              if(0 <= (ret = get_maxRsuApps())){
                  snmp_set_var_typed_integer(requests->requestvb, ASN_INTEGER, ret);
                  ret = RSEMIB_OK;
              }
              break;
          default:
              ret = RSEMIB_BAD_MODE;
              break;
      }
      /* scalars and like are always index 0 with unqiue column.*/
      print_request_result(0, APP_MAX_OID,requests,reqinfo->mode,ret);
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
/* 5.19.2.1 */
static int32_t handle_rsuAppConfigID(int32_t index,netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret = RSEMIB_OK;

  DEBUGMSGTL((MY_NAME, "%s: Entry:\n",__FUNCTION__));
  switch (reqinfo->mode){
      case MODE_GET:
          if((1 <= index) && (index <= RSU_APPS_RUNNING_MAX)){
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
/* 5.19.2.2 */
int32_t handle_rsuAppConfigName(int32_t index,netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret = RSEMIB_OK;
  uint8_t data[RSU_APP_CONF_NAME_LENGTH_MAX];

  DEBUGMSGTL((MY_NAME, "%s: Entry:\n",__FUNCTION__));
  switch (reqinfo->mode){
      case MODE_GET:
          DEBUGMSGTL((MY_NAME, "GET:\n"));
          memset(data,0x0,sizeof(data));
          if(0 <= (ret = get_rsuAppConfigName(index,data))){
              snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR, data, ret);
              ret = RSEMIB_OK;
          }
          break;
      default:
          ret = RSEMIB_BAD_MODE;
          break;
  }
  return ret;
}
/* 5.19.2.3 */
int32_t handle_rsuAppConfigStartup(int32_t index, netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret = RSEMIB_OK;
  int32_t data = 0;

  DEBUGMSGTL((MY_NAME, "%s: Entry:\n",__FUNCTION__));
  switch (reqinfo->mode){
      case MODE_GET:
          DEBUGMSGTL((MY_NAME, "GET:\n"));
          if(RSEMIB_OK == (ret = get_rsuAppConfigStartup(index,&data))){
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
          if(RSEMIB_OK == (ret = preserve_rsuAppConfigStartup(index))){
              ret = action_rsuAppConfigStartup(index, *(requests->requestvb->val.integer));
          }
          break;
      case MODE_SET_COMMIT:
          DEBUGMSGTL((MY_NAME, "COMMIT:\n"));
          if(RSEMIB_OK == (ret = commit_rsuAppConfigStartup(index))){
              /*
               * Back end magic here.
               */
          }
          break;
      case MODE_SET_UNDO:
          DEBUGMSGTL((MY_NAME, "UNDO:\n"));
          ret = undo_rsuAppConfigStartup(index);
          break;
      default:
          ret = RSEMIB_BAD_MODE;
          break;
  }
  return ret;
}
/* 5.19.2.4 */
int32_t handle_rsuAppConfigState(int32_t index,netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret = RSEMIB_OK;
  int32_t data = 0;

  DEBUGMSGTL((MY_NAME, "%s: Entry:\n",__FUNCTION__));
  switch (reqinfo->mode){
      case MODE_GET:
          DEBUGMSGTL((MY_NAME, "GET:\n"));
          if(RSEMIB_OK == (ret = get_rsuAppConfigState(index,&data))){
              snmp_set_var_typed_integer(requests->requestvb, ASN_INTEGER, data);
          }
          break;
      default:
          ret = RSEMIB_BAD_MODE;
          break;
  }
  return ret;
}
/* 5.19.2.5 */
int32_t handle_rsuAppConfigStart(int32_t index,netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret = RSEMIB_OK;
  int32_t data = 0;

  DEBUGMSGTL((MY_NAME, "%s: Entry:\n",__FUNCTION__));
  switch (reqinfo->mode){
      case MODE_GET:
          DEBUGMSGTL((MY_NAME, "GET:\n"));
          if(RSEMIB_OK == (ret = get_rsuAppConfigStart(index,&data))){
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
          if(RSEMIB_OK == (ret = preserve_rsuAppConfigStart(index))){
              ret = action_rsuAppConfigStart(index,*(requests->requestvb->val.integer));
          }
          break;
      case MODE_SET_COMMIT:
          DEBUGMSGTL((MY_NAME, "COMMIT:\n"));
          if(RSEMIB_OK == (ret = commit_rsuAppConfigStart(index))){
              /*
               * Back end magic here.
               */
          }
          break;
      case MODE_SET_UNDO:
          DEBUGMSGTL((MY_NAME, "UNDO:\n"));
          ret = undo_rsuAppConfigStart(index);
          break;
      default:
          ret = RSEMIB_BAD_MODE;
          break;
  }
  return ret;
}
/* 5.19.2.6 */
int32_t handle_rsuAppConfigStop(int32_t index,netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret = RSEMIB_OK;
  int32_t data = 0;

  DEBUGMSGTL((MY_NAME, "%s: Entry:\n",__FUNCTION__));
  switch (reqinfo->mode){
      case MODE_GET:
          DEBUGMSGTL((MY_NAME, "GET:\n"));
          if(RSEMIB_OK == (ret = get_rsuAppConfigStop(index,&data))){
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
          if(RSEMIB_OK == (ret = preserve_rsuAppConfigStop(index))){
              ret = action_rsuAppConfigStop(index, *(requests->requestvb->val.integer));
          }
          break;
      case MODE_SET_COMMIT:
          DEBUGMSGTL((MY_NAME, "COMMIT:\n"));
          if(RSEMIB_OK == (ret = commit_rsuAppConfigStop(index))){
              /*
               * Back end magic here.
               */
          }
          break;
      case MODE_SET_UNDO:
          DEBUGMSGTL((MY_NAME, "UNDO:\n"));
          ret = undo_rsuAppConfigStop(index);
          break;
      default:
          ret = RSEMIB_BAD_MODE;
          break;
  }
  return ret;
}
/* 5.19.2: Table handler: Break it down to smaller requests. */
int32_t handle_rsuAppConfigTable( netsnmp_mib_handler          * handler
                                , netsnmp_handler_registration * reginfo
                                , netsnmp_agent_request_info   * reqinfo
                                , netsnmp_request_info         * requests)
{
  netsnmp_request_info       * request       = NULL;
  uint32_t                     request_count = 0;
  RsuAppConfigEntry_t        * table_entry   = NULL;
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

      table_entry  = (RsuAppConfigEntry_t *)netsnmp_tdata_extract_entry(request);
      request_info = netsnmp_extract_table_info(request);

      if((NULL == table_entry) || (NULL == request_info)){
          set_error_states(RSEMIB_BAD_REQUEST_TABLE_INFO);
          DEBUGMSGTL((MY_NAME, "NULL == request_info.\n"));
          ret = SNMP_ERR_INCONSISTENTVALUE;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"RSEMIB_BAD_REQUEST_TABLE_INFO.\n");
      } else {
           DEBUGMSGTL((MY_NAME, "COLUMN=%d.\n",request_info->colnum));
           switch (request_info->colnum){
              case APP_CONFIG_ID_COLUMN:
                  ret = handle_rsuAppConfigID(table_entry->rsuAppConfigID, reqinfo, request);
                  break;
              case APP_CONFIG_NAME_COLUMN:
                  ret = handle_rsuAppConfigName(table_entry->rsuAppConfigID, reqinfo, request);
                  break;
              case APP_CONFIG_STARTUP_COLUMN:
                  ret = handle_rsuAppConfigStartup(table_entry->rsuAppConfigID, reqinfo, request);
                  break;
              case APP_CONFIG_STATE_COLUMN:
                  ret = handle_rsuAppConfigState(table_entry->rsuAppConfigID, reqinfo, request);
                  break;
              case APP_CONFIG_START_COLUMN:
                  ret = handle_rsuAppConfigStart(table_entry->rsuAppConfigID, reqinfo, request);
                  break;
              case APP_CONFIG_STOP_COLUMN:
                  ret = handle_rsuAppConfigStop(table_entry->rsuAppConfigID, reqinfo, request);
                  break;
              default:
                  /* Ignore requests for columns outside defintion. Dont send error because it will kill walk.*/
                  ret = RSEMIB_BAD_REQUEST_COLUMN;
                  break;
          }
          if(ret < RSEMIB_OK) {
              set_error_states(ret);
          }
          print_request_result(table_entry->rsuAppConfigID, request_info->colnum,requests,reqinfo->mode,ret);
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

STATIC void remove_rsuAppConfigTableEntry(netsnmp_tdata * table_data, netsnmp_tdata_row * row)
{
  RsuAppConfigEntry_t * entry = NULL;

  if (NULL == row){
      DEBUGMSGTL((MY_NAME,"remove_rsuAppConfigTableEntry NULL input.\n"));
      return;
  }
  if (NULL != (entry = (RsuAppConfigEntry_t *)row->data)){
      SNMP_FREE(entry);
  } else {
      DEBUGMSGTL((MY_NAME,"rsuRadioTable_removeEntry row data is NULL!.\n"));
  }
  if (table_data){
      netsnmp_tdata_remove_and_delete_row(table_data, row);
  } else {
      DEBUGMSGTL((MY_NAME,"remove_rsuAppConfigTableEntry NULL input.\n"));
      netsnmp_tdata_delete_row(row); 
  }
  return;
}
void destroy_rsuAppConfigTable(void)
{
  netsnmp_tdata_row  * row     = NULL;
  netsnmp_tdata_row  * nextrow = NULL;
  int32_t              i       = 0;

  if(NULL != rsuAppConfigTable){
      if(NULL != (row = netsnmp_tdata_row_first(rsuAppConfigTable))){
          for(i=0;(row != NULL) && (i < RSU_APPS_RUNNING_MAX);i++){
              nextrow = netsnmp_tdata_row_next(rsuAppConfigTable, row);
              remove_rsuAppConfigTableEntry(rsuAppConfigTable, row);
              row = nextrow;
          }
      } else {
          DEBUGMSGTL((MY_NAME, "destroy: Table empty. Nothing to do.\n"));
      }
  } else {
      DEBUGMSGTL((MY_NAME, "destroy: Table is NULL. Did we get called before install?\n"));
  }
}
static netsnmp_tdata_row * create_rsuAppConfigTable(netsnmp_tdata * table_data, RsuAppConfigEntry_t * rsuAppConfigTable)
{
  RsuAppConfigEntry_t * entry = NULL;
  netsnmp_tdata_row   * row   = NULL;
  int32_t               i     = 0;
  
  if(NULL == table_data){
      DEBUGMSGTL((MY_NAME, "create_rsuRadioTable FAIL: table_data == NULL.\n"));
      return NULL;
  }

  for(i=0; i < RSU_APPS_RUNNING_MAX; i++){
      if (NULL == (entry = SNMP_MALLOC_TYPEDEF(RsuAppConfigEntry_t))){
          DEBUGMSGTL((MY_NAME, "rsuAppConfig SNMP_MALLOC_TYPEDEF FAILED.\n"));
          return NULL;
      }
      if (NULL == (row = netsnmp_tdata_create_row())){
          DEBUGMSGTL((MY_NAME, "rsuAppConfig netsnmp_tdata_create_row FAILED.\n"));
          SNMP_FREE(entry);
          return NULL;
      }
      row->data = entry;
      memcpy(entry,&rsuAppConfigTable[i],sizeof(RsuAppConfigEntry_t));
      netsnmp_tdata_row_add_index(row, ASN_INTEGER , &rsuAppConfigTable[i].rsuAppConfigID, sizeof(int32_t));
      if(SNMPERR_SUCCESS != netsnmp_tdata_add_row(table_data, row)){
          DEBUGMSGTL((MY_NAME, "rsuAppConfig netsnmp_tdata_add_row FAILED.\n"));
          return NULL;
      }
  }
  return row;
}
static int32_t install_rsuAppConfigTable(RsuAppConfigEntry_t * rsuAppConfigs)
{
    netsnmp_handler_registration * reg = NULL;

    reg = netsnmp_create_handler_registration("rsuAppConfigTable"
                                              ,handle_rsuAppConfigTable
                                              ,rsuAppConfigTable_oid
                                              ,OID_LENGTH(rsuAppConfigTable_oid)
                                              ,HANDLER_CAN_RWRITE);

    if (NULL == (rsuAppConfigTable = netsnmp_tdata_create_table("rsuAppConfigTable", 0))){
        DEBUGMSGTL((MY_NAME, "rsuAppConfigTable FAILED.\n"));
        return RSEMIB_INSTALL_HANDLER_FAIL;
    }
    if (NULL == (app_table_reg_info = SNMP_MALLOC_TYPEDEF(netsnmp_table_registration_info))){
        DEBUGMSGTL((MY_NAME, "SNMP_MALLOC_TYPEDEF FAILED.\n"));
        return RSEMIB_INSTALL_HANDLER_FAIL;
    }
    netsnmp_table_helper_add_indexes(app_table_reg_info, ASN_INTEGER,0); /* index */                                 

    app_table_reg_info->min_column = APP_CONFIG_NAME_COLUMN; /* rsuAppConfigName:  Hide index by definition. */
    app_table_reg_info->max_column = APP_CONFIG_STOP_COLUMN; /* rsuAppConfigStop: */

    netsnmp_tdata_register(reg, rsuAppConfigTable, app_table_reg_info);

    /* Initialize the contents of the table. */
    if (NULL == create_rsuAppConfigTable(rsuAppConfigTable, rsuAppConfigs)){
        DEBUGMSGTL((MY_NAME, "initialize rsuAppConfigTable: ERROR: Null row.\n"));
        return RSEMIB_INSTALL_HANDLER_FAIL;
    }
    return RSEMIB_OK;
}
void install_rsuAppConfig_handlers(void)
{
  int32_t i = 0;
  RsuAppConfigEntry_t rsuAppConfigs[RSU_APPS_RUNNING_MAX];  

  /* Clear local statics. */
  app_table_reg_info = NULL;
  rsuAppConfigTable  = NULL;
  rsuAppConfig_error_states = 0x0;
  memset(rsuAppConfigs,0x0,sizeof(rsuAppConfigs));

  /* Populate local copy to use while creating fixed tables below. */
  for(i=0;i <RSU_APPS_RUNNING_MAX;i++){
      rsuAppConfigs[i].rsuAppConfigID = i+1;
      if(0 > get_rsuAppConfigName(i+1,rsuAppConfigs[i].rsuAppConfigName)){
          DEBUGMSGTL((MY_NAME, "Install rsuAppConfigName FAIL.\n"));
          set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
      }
      if(RSEMIB_OK != get_rsuAppConfigStartup(i+1,(int32_t *)&rsuAppConfigs[i].rsuAppConfigStartup)){
          DEBUGMSGTL((MY_NAME, "Install rsuAppConfigStartup FAIL.\n"));
          set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
      }
      if(RSEMIB_OK != get_rsuAppConfigState(i+1,&rsuAppConfigs[i].rsuAppConfigState)){
          DEBUGMSGTL((MY_NAME, "Install rsuAppConfigState FAIL.\n"));
          set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
      }
      if(RSEMIB_OK != get_rsuAppConfigStart(i+1,&rsuAppConfigs[i].rsuAppConfigStart)){
          DEBUGMSGTL((MY_NAME, "Install rsuAppConfigStart FAIL.\n"));
          set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
      }
      if(RSEMIB_OK != get_rsuAppConfigStop(i+1,&rsuAppConfigs[i].rsuAppConfigStop)){
          DEBUGMSGTL((MY_NAME, "Install rsuAppConfigStop FAIL.\n"));
          set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
      }
  }

  /* 5.19.1 */
  if(MIB_REGISTERED_OK != netsnmp_register_scalar(netsnmp_create_handler_registration
                          ("maxRsuApps",
                           handle_maxRsuApps,
                           maxRsuApps_oid,
                           OID_LENGTH(maxRsuApps_oid),
                           HANDLER_CAN_RONLY))){
      DEBUGMSGTL((MY_NAME, "maxRsuApps install FAIL.\n"));
      set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
  }
  /* 5.19.2.1 - 5.19.2.6: Install table created with data from MIB. */
  if(RSEMIB_OK != install_rsuAppConfigTable(&rsuAppConfigs[0])){
      DEBUGMSGTL((MY_NAME, "rsuAppConfigTable install FAIL.\n"));
      set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
  }
}
void destroy_rsuAppConfig(void)
{
  destroy_rsuAppConfigTable();

  /* Free table info. */
  if (NULL != app_table_reg_info){
      SNMP_FREE(app_table_reg_info);
  }
  /* Free table reg. */
  if (NULL != rsuAppConfigTable){
      SNMP_FREE(rsuAppConfigTable);
  }
}
