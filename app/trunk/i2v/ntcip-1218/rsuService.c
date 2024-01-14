/**************************************************************************
 *                                                                        *
 *     File Name:  rsuService.c                                           *
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
#define MY_NAME      "rsuService"
/* Mandatory: For debugging and syslog output. */
#define MY_I2V_NAME  "RsuSrv"
#define MY_ERR_LEVEL LEVEL_PRIV    /* from i2v_util.h */

/* table column definitions. */
#define RSU_SERVICE_ID_COLUMN          1
#define RSU_SERVICE_NAME_COLUMN        2
#define RSU_SERVICE_STATUS_COLUMN      3
#define RSU_SERVICE_STATUS_DESC_COLUMN 4
#define RSU_SERVICE_STATUS_TIME_COLUMN 5
/* Theoritical column ids. Make sure they follow in order with columns and unique. */
#define RSU_SERVICE_MAX_OID            6

#define RSU_SERVICE_ID_NAME          "rsuServiceID"
#define RSU_SERVICE_NAME_NAME        "rsuServiceName"
#define RSU_SERVICE_STATUS_NAME      "rsuServiceStatus"
#define RSU_SERVICE_STATUS_DESC_NAME "rsuServiceStatusDesc"
#define RSU_SERVICE_STATUS_TIME_NAME "rsuServiceStatusTime"
#define RSU_SERVICE_MAX_NAME         "maxRsuServices"
/******************************************************************************
 * 5.20 RSU Services: { rsu 19 }:
 *
 * 5.20.1 - 5.20.2 
 * int32_t           maxRsuServices;
 * RsuServiceEntry_t rsuServiceTable[RSU_SERVICES_RUNNING_MAX];
 *
 * typedef enum  {
 *     rsuServiceStatus_other    = 1, boot time?
 *     rsuServiceStatus_okay     = 2, fully operational with no errors
 *     rsuServiceStatus_warning  = 3, operating, but error detected which may(?) affect its operation, e.g., wrong config (how to know?)
 *     rsuServiceStatus_critical = 4, error is detected that has impact on the RSU operation e.g. you ignored the warning so now what?
 *     rsuServiceStatus_unknown  = 5, this is what comes after staying in critical for too long.
 * } rsuServiceStatus_e;
 * 
 * The name and assignments for the standard services are as follows:
 *         rsuServiceID   rsuServiceName   Description
 *               1        RSU system       system services on the RSU
 *               2        GNSS             GNSS services
 *               3        Time source      clock sources
 *               4        Storage          file and log storage available
 *               5        Firewall         access control services
 *               6        Network          network services
 *               7        Layers 1 and 2   radio access technology
 *               8        Layers 3 and 4   networking services
 *               9        Security         security services
 *               10       SCMS             SCMS services
 *             11-16      RESERVED         reserved for future services
 *             17-127     vendor-specific  for vendor-specific services
 *
 * typedef struct {
 *    int32_t            rsuServiceID;                                             RO: 1..maxRsuApps:
 *    uint8_t            rsuServiceName[RSU_SERVICE_NAME_LENGTH_MAX];              RW: 1..127 chars 
 *    rsuServiceStatus_e rsuServiceStatus;                                         RO: 1..5 enum: default is rsuServiceStatus_unknown
 *    uint8_t            rsuServiceStatusDesc[RSU_SERVICE_STATUS_DESC_LENGTH_MAX]; RO: 0..255 chars: details of status: ie "Fire,bad!"
 *    uint8_t            rsuServiceStatusTime[MIB_DATEANDTIME_LENGTH];             RO: 8 OCTETS:DateAndTime of status. 
 * } RsuServiceEntry_t;
 *
 ******************************************************************************/

const oid maxRsuServices_oid[]  = {1, 3, 6, 1, 4, 1, 1206, 4, 2, 18, 19 , 1};
const oid rsuServiceTable_oid[] = {1, 3, 6, 1, 4, 1, 1206, 4, 2, 18, 19 , 2};

/* snmp table pointers */
static netsnmp_table_registration_info * table_reg_info = NULL;
static netsnmp_tdata * rsuServiceTable = NULL;

static uint32_t rsuService_error_states = 0x0;

/* 
 * Function Bodies.
 */
static void set_error_states(int32_t error)
{
  /* -1 to -32 */
  if((error < 0) && (error > MIB_ERROR_BASE_2)){
      rsuService_error_states |= (uint32_t)(0x1) << (abs(error) - 1);
  }
}
uint32_t get_rsuService_error_states(void)
{
  return rsuService_error_states;
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
      case RSU_SERVICE_ID_COLUMN:
          strcpy(column_name,RSU_SERVICE_ID_NAME);
          break;
      case RSU_SERVICE_NAME_COLUMN:
          strcpy(column_name,RSU_SERVICE_NAME_NAME);
          break;
      case RSU_SERVICE_STATUS_COLUMN:
          strcpy(column_name,RSU_SERVICE_STATUS_NAME);
          break;
      case RSU_SERVICE_STATUS_DESC_COLUMN:
          strcpy(column_name,RSU_SERVICE_STATUS_DESC_NAME);
          break;
      case RSU_SERVICE_STATUS_TIME_COLUMN:
          strcpy(column_name,RSU_SERVICE_STATUS_TIME_NAME);
          break;
      case RSU_SERVICE_MAX_OID:
          strcpy(column_name,RSU_SERVICE_MAX_NAME);
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

/* 5.20.1: RO */
int32_t handle_maxRsuServices(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo,
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
              if(0 <= (ret = get_maxRsuServices())){
                  snmp_set_var_typed_integer(requests->requestvb, ASN_INTEGER, ret);
                  ret = RSEMIB_OK;
              }
              break;
          default:
              ret = RSEMIB_BAD_MODE;
              break;
      }
      /* scalars and like are always index 0 with unqiue column.*/
      print_request_result(0, RSU_SERVICE_MAX_OID,requests,reqinfo->mode,ret);
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
/* 5.20.2.1: RO */
static int32_t handle_rsuServiceID(int32_t index,netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret = RSEMIB_OK;

  DEBUGMSGTL((MY_NAME, "%s: Entry:\n",__FUNCTION__));
  switch (reqinfo->mode){
      case MODE_GET:
          if((1 <= index) && (index <= RSU_SERVICES_RUNNING_MAX)){
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
/* 5.20.2.2 */
int32_t handle_rsuServiceName(int32_t index,netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret = RSEMIB_OK;
  uint8_t data[RSU_SERVICE_NAME_LENGTH_MAX];

  DEBUGMSGTL((MY_NAME, "%s: Entry:\n",__FUNCTION__));
  switch (reqinfo->mode){
      case MODE_GET:
          DEBUGMSGTL((MY_NAME, "GET:\n"));
          memset(data,0x0,sizeof(data));
          if(0 <= (ret = get_rsuServiceName(index,data))){
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
          if(RSEMIB_OK == (ret = preserve_rsuServiceName(index))){
              ret = action_rsuServiceName(index, requests->requestvb->val.string, requests->requestvb->val_len);
          }
          break;
      case MODE_SET_COMMIT:
          DEBUGMSGTL((MY_NAME, "COMMIT:\n"));
          if(RSEMIB_OK == (ret = commit_rsuServiceName(index))){
              /*
               * Back end magic here.
               */
          }
          break;
      case MODE_SET_UNDO:
          DEBUGMSGTL((MY_NAME, "UNDO:\n"));
          ret = undo_rsuServiceName(index);
          break;
      default:
          ret = RSEMIB_BAD_MODE;
          break;
  }
  return ret;
}
/* 5.20.2.3: RO */
int32_t handle_rsuServiceStatus(int32_t index,netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret = RSEMIB_OK;

  DEBUGMSGTL((MY_NAME, "%s: Entry:\n",__FUNCTION__));
  switch (reqinfo->mode){
      case MODE_GET:
          DEBUGMSGTL((MY_NAME, "GET:\n"));
          if(0 <= (ret = get_rsuServiceStatus(index))){
              snmp_set_var_typed_integer(requests->requestvb, ASN_INTEGER, ret);
              ret = RSEMIB_OK;
          }
          break;
      default:
          ret = RSEMIB_BAD_MODE;
          break;
  }
  return ret;
}
/* 5.20.2.4 */
int32_t handle_rsuServiceStatusDesc(int32_t index, netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret = RSEMIB_OK;
  uint8_t data[RSU_SERVICE_STATUS_DESC_LENGTH_MAX];

  DEBUGMSGTL((MY_NAME, "%s: Entry:\n",__FUNCTION__));
  switch (reqinfo->mode){
      case MODE_GET:
          DEBUGMSGTL((MY_NAME, "GET:\n"));
          memset(data,0x0,sizeof(data));
          if(0 <= (ret = get_rsuServiceStatusDesc(index,data))){
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
/* 5.20.2.5 */
int32_t handle_rsuServiceStatusTime (int32_t index, netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret = RSEMIB_OK;
  uint8_t data[MIB_DATEANDTIME_LENGTH];

  DEBUGMSGTL((MY_NAME, "%s: Entry:\n",__FUNCTION__));
  switch (reqinfo->mode){
      case MODE_GET:
          DEBUGMSGTL((MY_NAME, "GET:\n"));
          memset(data,0x0,sizeof(data));
          if(0 <= (ret = get_rsuServiceStatusTime(index, data))){
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
/* 5.20.2: Table handler: Break it down to smaller requests. */
int32_t handle_rsuServiceTable( netsnmp_mib_handler          * handler
                              , netsnmp_handler_registration * reginfo
                              , netsnmp_agent_request_info   * reqinfo
                              , netsnmp_request_info         * requests)
{
  netsnmp_request_info       * request       = NULL;
  uint32_t                     request_count = 0;
  RsuServiceEntry_t          * table_entry   = NULL;
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

      table_entry  = (RsuServiceEntry_t *)netsnmp_tdata_extract_entry(request);
      request_info = netsnmp_extract_table_info(request);

      if((NULL == table_entry) || (NULL == request_info)){
          set_error_states(RSEMIB_BAD_REQUEST_TABLE_INFO);
          DEBUGMSGTL((MY_NAME, "NULL == request_info.\n"));
          ret = SNMP_ERR_INCONSISTENTVALUE;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"RSEMIB_BAD_REQUEST_TABLE_INFO.\n");
      } else {
           DEBUGMSGTL((MY_NAME, "COLUMN=%d.\n",request_info->colnum));
           switch (request_info->colnum){
              case RSU_SERVICE_ID_COLUMN:
                  ret = handle_rsuServiceID(table_entry->rsuServiceID, reqinfo, request);
                  break;
              case RSU_SERVICE_NAME_COLUMN:
                  ret = handle_rsuServiceName(table_entry->rsuServiceID, reqinfo, request);
                  break;
              case RSU_SERVICE_STATUS_COLUMN:
                  ret = handle_rsuServiceStatus(table_entry->rsuServiceID, reqinfo, request);
                  break;
              case RSU_SERVICE_STATUS_DESC_COLUMN:
                  ret = handle_rsuServiceStatusDesc(table_entry->rsuServiceID, reqinfo, request);
                  break;
              case RSU_SERVICE_STATUS_TIME_COLUMN:
                  ret = handle_rsuServiceStatusTime(table_entry->rsuServiceID, reqinfo, request);
                  break;
              default:
                  /* Ignore requests for columns outside defintion. Dont send error because it will kill walk.*/
                  ret = RSEMIB_BAD_REQUEST_COLUMN;
                  break;
          }
          if(ret < RSEMIB_OK) {
              set_error_states(ret);
          }
          print_request_result(table_entry->rsuServiceID, request_info->colnum,requests,reqinfo->mode,ret);
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

STATIC void remove_rsuServiceTableEntry(netsnmp_tdata * table_data, netsnmp_tdata_row * row)
{
  RsuServiceEntry_t * entry = NULL;

  if (NULL == row){
      DEBUGMSGTL((MY_NAME,"remove_rsuServiceTableEntry NULL input.\n"));
      return;
  }
  if (NULL != (entry = (RsuServiceEntry_t *)row->data)){
      SNMP_FREE(entry);
  } else {
      DEBUGMSGTL((MY_NAME,"rsuRadioTable_removeEntry row data is NULL!.\n"));
  }
  if (table_data){
      netsnmp_tdata_remove_and_delete_row(table_data, row);
  } else {
      DEBUGMSGTL((MY_NAME,"remove_rsuServiceTableEntry NULL input.\n"));
      netsnmp_tdata_delete_row(row); 
  }
  return;
}
void destroy_rsuServiceTable(void)
{
  netsnmp_tdata_row  * row     = NULL;
  netsnmp_tdata_row  * nextrow = NULL;
  int32_t              i       = 0;

  if(NULL != rsuServiceTable){
      if(NULL != (row = netsnmp_tdata_row_first(rsuServiceTable))){
          for(i=0;(row != NULL) && (i < RSU_SERVICES_RUNNING_MAX);i++){
              nextrow = netsnmp_tdata_row_next(rsuServiceTable, row);
              remove_rsuServiceTableEntry(rsuServiceTable, row);
              row = nextrow;
          }
      } else {
          DEBUGMSGTL((MY_NAME, "destroy_rsuServiceTable: Table empty. Nothing to do.\n"));
      }
  } else {
      DEBUGMSGTL((MY_NAME, "destroy_rsuServiceTable: Table is NULL. Did we get called before install?\n"));
  }
}
static netsnmp_tdata_row * create_rsuServiceTable(netsnmp_tdata * table_data, RsuServiceEntry_t * rsuServiceTable)
{
  RsuServiceEntry_t * entry = NULL;
  netsnmp_tdata_row * row   = NULL;
  int32_t             i     = 0;
  
  if(NULL == table_data){
      DEBUGMSGTL((MY_NAME, "create_rsuRadioTable FAIL: table_data == NULL.\n"));
      return NULL;
  }

  for(i=0; i < RSU_SERVICES_RUNNING_MAX; i++){
      if (NULL == (entry = SNMP_MALLOC_TYPEDEF(RsuServiceEntry_t))){
          DEBUGMSGTL((MY_NAME, "rsuService SNMP_MALLOC_TYPEDEF FAILED.\n"));
          return NULL;
      }
      if (NULL == (row = netsnmp_tdata_create_row())){
          DEBUGMSGTL((MY_NAME, "rsuService netsnmp_tdata_create_row FAILED.\n"));
          SNMP_FREE(entry);
          return NULL;
      }
      row->data = entry;
      memcpy(entry,&rsuServiceTable[i],sizeof(RsuServiceEntry_t));
      netsnmp_tdata_row_add_index(row, ASN_INTEGER , &rsuServiceTable[i].rsuServiceID, sizeof(int32_t));
      if(SNMPERR_SUCCESS != netsnmp_tdata_add_row(table_data, row)){
          DEBUGMSGTL((MY_NAME, "rsuService netsnmp_tdata_add_row FAILED.\n"));
          return NULL;
      }
  }
  return row;
}
static int32_t install_rsuServiceTable(RsuServiceEntry_t * rsuServices)
{
    netsnmp_handler_registration * reg = NULL;

    reg = netsnmp_create_handler_registration("rsuServiceTable"
                                              ,handle_rsuServiceTable
                                              ,rsuServiceTable_oid
                                              ,OID_LENGTH(rsuServiceTable_oid)
                                              ,HANDLER_CAN_RWRITE);

    if (NULL == (rsuServiceTable = netsnmp_tdata_create_table("rsuServiceTable", 0))){
        DEBUGMSGTL((MY_NAME, "rsuServiceTable FAILED.\n"));
        return RSEMIB_INSTALL_HANDLER_FAIL;
    }
    if (NULL == (table_reg_info = SNMP_MALLOC_TYPEDEF(netsnmp_table_registration_info))) {
        DEBUGMSGTL((MY_NAME, "SNMP_MALLOC_TYPEDEF FAILED.\n"));
        return RSEMIB_INSTALL_HANDLER_FAIL;
    }
    netsnmp_table_helper_add_indexes(table_reg_info, ASN_INTEGER,0); /* index */                                 

    table_reg_info->min_column = RSU_SERVICE_NAME_COLUMN; /* rsuServiceName:  Hide index by definition. */
    table_reg_info->max_column = RSU_SERVICE_STATUS_TIME_COLUMN; /* rsuServiceStatusTime: */

    netsnmp_tdata_register(reg, rsuServiceTable, table_reg_info);

    /* Initialize the contents of the table. */
    if (NULL == create_rsuServiceTable(rsuServiceTable, rsuServices)){
        DEBUGMSGTL((MY_NAME, "create_rsuServiceTable FAIL.\n"));
        return RSEMIB_INSTALL_HANDLER_FAIL;
    }
    return RSEMIB_OK;
}
void install_rsuService_handlers(void)
{
  int32_t i = 0;
  RsuServiceEntry_t rsuServices[RSU_SERVICES_RUNNING_MAX];  

  /* Clear statics across soft resets. */
  table_reg_info  = NULL;
  rsuServiceTable = NULL;
  rsuService_error_states = 0x0;
  memset(rsuServices,0x0,sizeof(rsuServices));

  /* Populate local copy to use while creating fixed tables below. */
  for(i=0;i <RSU_SERVICES_RUNNING_MAX;i++){
      rsuServices[i].rsuServiceID = i+1;
      if(0 > get_rsuServiceName(i+1,rsuServices[i].rsuServiceName)){
          set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
      }
      if(0 > (rsuServices[i].rsuServiceStatus = get_rsuServiceStatus(i+1))){
          set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
      }
      if(0 > get_rsuServiceStatusDesc(i+1,rsuServices[i].rsuServiceStatusDesc)){
          set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
      }
      if(0 > get_rsuServiceStatusTime(i+1,rsuServices[i].rsuServiceStatusTime)){
          set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
      }
  }

  /* 5.19.1 */
  if(MIB_REGISTERED_OK != netsnmp_register_scalar(netsnmp_create_handler_registration
                          ("maxRsuServices",
                           handle_maxRsuServices,
                           maxRsuServices_oid,
                           OID_LENGTH(maxRsuServices_oid),
                           HANDLER_CAN_RONLY))){
      DEBUGMSGTL((MY_NAME, "maxRsuServices install FAIL.\n"));
      set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
  }
  /* 5.19.2.1 - 5.19.2.6: Install table created with data from MIB. */
  if(RSEMIB_OK != install_rsuServiceTable(&rsuServices[0])){
      DEBUGMSGTL((MY_NAME, "rsuServiceTable install FAIL.\n"));
      set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
  }
}
void destroy_rsuService(void)
{
  destroy_rsuServiceTable();

  /* Free table info. */
  if (NULL != table_reg_info){
      SNMP_FREE(table_reg_info);
  }
  /* Free table reg. */
  if (NULL != rsuServiceTable){
      SNMP_FREE(rsuServiceTable);
  }
}
