/**************************************************************************
 *                                                                        *
 *     File Name:  rsuInterfaceLog.c                                      *
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
#include <ctype.h>
#include "rsutable.h"
#include "ntcip-1218.h"
#include "i2v_util.h"
#if defined(MY_UNIT_TEST)
#include "stubs.h"
#endif

/*
 * Defines.
 */
#define OUTPUT_MODULUS  1200 /* Every 20 minutes. */

/* Ignore requests we have serviced. Quicker, but no harm if not enabled. */
#define USE_PROCESSED_FLAG

/* Mandatory: For SNMP debugging and output. */
#define MY_NAME       "rsuIfcLog"
#define MY_NAME_EXTRA "XIfcLog"
#define MY_I2V_NAME    MY_NAME
#define MY_ERR_LEVEL   LEVEL_PRIV  /* from i2v_util.h */

/* table column definitions. */
#define INTERFACE_LOG_INDEX_COLUMN          1
#define INTERFACE_LOG_GENERATE_COLUMN       2
#define INTERFACE_LOG_MAX_FILE_SIZE_COLUMN  3
#define INTERFACE_LOG_MAX_FILE_TIME_COLUMN  4
#define INTERFACE_LOG_BY_DIR_COLUMN         5
#define INTERFACE_LOG_IFC_NAME_COLUMN       6
#define INTERFACE_LOG_STORE_PATH_COLUMN     7
#define INTERFACE_LOG_NAME_COLUMN           8
#define INTERFACE_LOG_START_COLUMN          9
#define INTERFACE_LOG_STOP_COLUMN           10
#define INTERFACE_LOG_OPTIONS_COLUMN        11
#define INTERFACE_LOG_STATUS_COLUMN         12
 /* Theoritical column ids. Make sure they follow in order with columns and unique. */
#define INTERFACE_LOG_MAX_OID               13

#define INTERFACE_LOG_INDEX_NAME          "rsuIfaceLogIndex"
#define INTERFACE_LOG_GENERATE_NAME       "rsuIfaceGenerate"
#define INTERFACE_LOG_MAX_FILE_SIZE_NAME  "rsuIfaceMaxFileSize"
#define INTERFACE_LOG_MAX_FILE_TIME_NAME  "rsuIfaceMaxFileTime"
#define INTERFACE_LOG_BY_DIR_NAME         "rsuIfaceLogByDir"
#define INTERFACE_LOG_IFC_NAME_NAME       "rsuIfaceName"
#define INTERFACE_LOG_STORE_PATH_NAME     "rsuIfaceStoragePath"
#define INTERFACE_LOG_NAME_NAME           "rsuIfaceLogName"
#define INTERFACE_LOG_START_NAME          "rsuIfaceLogStart"
#define INTERFACE_LOG_STOP_NAME           "rsuIfaceLogStop"
#define INTERFACE_LOG_OPTIONS_NAME        "rsuIfaceLogOptions"
#define INTERFACE_LOG_STATUS_NAME         "rsuIfaceLogStatus"
 /* Theoritical column ids. Make sure they follow in order with columns and unique. */
#define INTERFACE_LOG_MAX_NAME           "maxRsuInterfaceLogs"

/******************************************************************************
 * 5.8 Interface Log: { rsu 7 }: rsuInterfaceLogTable.
 * 
 * typedef struct {
 *     int32_t     rsuIfaceLogIndex;                     RO: 1 to maxRsuInterfaceLogs: 5940 has eth0, dsrc1 & cv2x1.
 *     onOff_e     rsuIfaceGenerate;                     RC: 0=off, 1=on.
 *     int32_t     rsuIfaceMaxFileSize;                  RC: 1..40 megabytes: 5 is default.
 *     int32_t     rsuIfaceMaxFileTime;                  RC: 1..48 hours: 24 is default.
 *     ifclogDir_e rsuIfaceLogByDir;                     RC: 1=inBound,2=outBound,3=biSeperate, 4=biTogether: default is 1?
 *     uint8_t      rsuIfaceName[RSU_INTERFACE_NAME_MAX]; RC: 0..127: "eth0", "dsrc0", "cv2x0". if its "" then what?
 *
 *     Storage path of ifclogs. Relative to the base directory (rsuSysDir). base dir is specified by  '/':  sub dir '/mysubdir'.
 *
 *     uint8_t rsuIfaceStoragePath[RSU_IFCLOG_STORE_PATH_MAX];  RC:1..255: path to ifclogs relative to rsuSysDir.
 * 
 * 
 *     Filename for ifclogs:
 *         <identifier>_<interface>_<direction>_<time> and contain only the following case-sensitive field names in chevrons (<>):
 *              <identifier>: Identifier of the RSU. See rsuID.
 *              <interface>: Identifier of the interface. See rsuIfaceName.
 *              <direction>: Direction of communications. In for Inbound, Out for Outbound), and Both for inbound and outbound.
 *              <time>: In UTC. The format is YYYYMMDD_hhmmss, where YYYY is the four-digit year, 
 *                              MM is the two-digit month, DD is the two-digit day, hh is the two-digit hour, 
 *                              mm is the two-digit minute, and ss is the two-digit seconds.
 *      
 *       Example: If the rsuID = rsu1, rsuIfaceName = dsrc1, both directions are collected in the same file, 
 *                and the date and time the file was created is January 3, 2019, at 01:35:03, 
 *                the filename would be rsu1_dsrc1_Both_20190103_013503.
 *      
 *     uint8_t rsuIfaceLogName[RSU_IFCLOG_NAME_SIZE_MAX]; RC: 12..172: filename for ifclogs ie rsu1_dsrc1_Both_20190103_013503.
 * 
 *     uint8_t rsuIfaceLogStart[MIB_DATEANDTIME_LENGTH]; RC:"2d-1d-1d,1d:1d:1d.1d": 8 OCTETS:DateAndTime:SNMPv2-TC.           
 *     uint8_t rsuIfaceLogStop[MIB_DATEANDTIME_LENGTH];  RC:"2d-1d-1d,1d:1d:1d.1d": 8 OCTETS:DateAndTime:SNMPv2-TC. 
 * 
 * 
 *     A bit-mapped value as defined below for configuring the message.
 *       Bit 0        0=Delete oldest if disk full, 1=Stop if disk full
 *       Bit 1        0=DoNotDelete, 1=Delete data log
 *     
 *       Bit 0 - Indicates if the RSU is to delete the oldest interface data log file if the disk space for 
 *             storing interface data logs is full, or to stop interface data logging. Default is to delete the oldest file.
 *       Bit 1 - Indicates if the RSU is to delete the corresponding interface data log file when an entry is destroyed in 
 *             the rsuInterfaceLogTable. Default is not to delete the log file.
 *     
 *    int32_t rsuIfaceLogOptions; RC: bit 0 == diskFull, bit 1 == deleteentry: default is 0x00.
 * 
 *     int32_t rsuIfaceLogStatus;  RC: Create (4) & (6) destroy: notInService & notReady?
 * } RsuInterfaceLogEntry_t;
 *
 ******************************************************************************/

const oid maxRsuInterfaceLogs_oid[]  = {1, 3, 6, 1, 4, 1, 1206, 4, 2, 18, 7 , 1};
const oid rsuInterfaceLogTable_oid[] = {1, 3, 6, 1, 4, 1, 1206, 4, 2, 18, 7 , 2};

extern int32_t  mainloop; /* For thread handling. */
extern ntcip_1218_mib_t  ntcip_1218_mib;

static netsnmp_table_registration_info * table_reg_info = NULL; /* snmp table reg pointer. */
static netsnmp_tdata * rsuInterfaceLogTable             = NULL; /* snmp table pointer. */
static uint32_t interfaceLog_error_states               = 0x0;  /* local error state. */
static uint32_t prior_error_states                      = 0x0;  /* local error state. */
static interfaceLog_t * shm_interfaceLog_ptr            = NULL; /* SHM. */

static uint32_t rolling_counter = 0;

/*
 * Protos: Silence of the Warnings.
 */
static void rebuild_rsuInterfaceLog(void);
static void destroy_rsuInterfaceLogTable(void);

/*
 * Function Bodies.
 */
static void set_error_states(int32_t error)
{
  /* -1 to -32 */
  if((error < 0) && (error > MIB_ERROR_BASE_2)){
      interfaceLog_error_states |= (uint32_t)(0x1) << (abs(error) - 1);
  }
}
uint32_t get_interfaceLog_error_states(void)
{
  return interfaceLog_error_states;
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
      case INTERFACE_LOG_INDEX_COLUMN:
          strcpy(column_name,INTERFACE_LOG_INDEX_NAME);
          break;
      case INTERFACE_LOG_GENERATE_COLUMN:
          strcpy(column_name,INTERFACE_LOG_GENERATE_NAME);
          break;
      case INTERFACE_LOG_MAX_FILE_SIZE_COLUMN:
          strcpy(column_name,INTERFACE_LOG_MAX_FILE_SIZE_NAME);
          break;
      case INTERFACE_LOG_MAX_FILE_TIME_COLUMN:
          strcpy(column_name,INTERFACE_LOG_MAX_FILE_TIME_NAME);
          break;
      case INTERFACE_LOG_BY_DIR_COLUMN:
          strcpy(column_name,INTERFACE_LOG_BY_DIR_NAME);
          break;
      case INTERFACE_LOG_IFC_NAME_COLUMN:
          strcpy(column_name,INTERFACE_LOG_IFC_NAME_NAME);
          break;
      case INTERFACE_LOG_STORE_PATH_COLUMN:
          strcpy(column_name,INTERFACE_LOG_STORE_PATH_NAME);
          break;
      case INTERFACE_LOG_NAME_COLUMN:
          strcpy(column_name,INTERFACE_LOG_NAME_NAME);
          break;
      case INTERFACE_LOG_START_COLUMN:
          strcpy(column_name,INTERFACE_LOG_START_NAME);
          break;
      case INTERFACE_LOG_STOP_COLUMN:
          strcpy(column_name,INTERFACE_LOG_STOP_NAME);
          break;
      case INTERFACE_LOG_OPTIONS_COLUMN:
          strcpy(column_name,INTERFACE_LOG_OPTIONS_NAME);
          break;
      case INTERFACE_LOG_STATUS_COLUMN:
          strcpy(column_name,INTERFACE_LOG_STATUS_NAME);
          break;
/* For scalars and like kind. */
      case INTERFACE_LOG_MAX_OID:
          strcpy(column_name,INTERFACE_LOG_MAX_NAME);
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

/* 5.8.1 */
int32_t handle_maxRsuInterfaceLogs(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo,
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
              if(0 <= (ret = get_maxRsuInterfaceLogs())){
                  snmp_set_var_typed_integer(requests->requestvb, ASN_INTEGER, ret);
                  ret = RSEMIB_OK;
              }
              break;
          default:
              ret = RSEMIB_BAD_MODE;
              break;
      }
      /* scalars and like are always index 0 with unqiue column.*/
      print_request_result(0, INTERFACE_LOG_MAX_OID,requests,reqinfo->mode,ret);
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
/* 5.8.2.1 */
int32_t handle_rsuIfaceLogIndex(int32_t index, netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret  = RSEMIB_OK;
  int32_t data = index;

  DEBUGMSGTL((MY_NAME, "%s: Entry:\n",__FUNCTION__));
  switch (reqinfo->mode){
      case MODE_GET:
#if 0
          if(RSEMIB_OK == (ret = get_rsuIfaceLogIndex(index, &data)))
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
/* 5.8.2.2 */
static int32_t handle_rsuIfaceGenerate(int32_t index, netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret = RSEMIB_OK;
  int32_t data = 0;

  DEBUGMSGTL((MY_NAME, "%s: Entry:\n",__FUNCTION__));
  switch (reqinfo->mode){
      case MODE_GET:
          DEBUGMSGTL((MY_NAME, "GET:\n"));
          if(RSEMIB_OK == (ret = get_rsuIfaceGenerate(index,&data))){
              snmp_set_var_typed_integer(requests->requestvb, ASN_INTEGER, data);
          }
          break;
      case MODE_SET_RESERVE1: /* Short stack, all requests done in one. */
          DEBUGMSGTL((MY_NAME, "R1:\n"));
          if(RSEMIB_OK == (ret = preserve_rsuIfaceGenerate(index))){
              if(RSEMIB_OK == (ret = action_rsuIfaceGenerate(index,*(requests->requestvb->val.integer)))) {
                  if(RSEMIB_OK == (ret = commit_rsuIfaceGenerate(index))){
                      DEBUGMSGTL((MY_NAME, "COMMIT DONE:\n"));
                  } else {
                      DEBUGMSGTL((MY_NAME, "COMMIT FAIL:\n"));
                  }
              } else {
                  DEBUGMSGTL((MY_NAME, "ACTION FAIL:\n"));
              }
              if(RSEMIB_OK != ret) {
                  if(RSEMIB_OK != undo_rsuIfaceGenerate(index)) { /* UNDO fails nothing we can do. */
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
static int32_t handle_rsuIfaceMaxFileSize(int32_t index, netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret = RSEMIB_OK;
  int32_t data = 0;

  DEBUGMSGTL((MY_NAME, "%s: Entry:\n",__FUNCTION__));
  switch (reqinfo->mode){
      case MODE_GET:
          DEBUGMSGTL((MY_NAME, "GET:\n"));
          if(RSEMIB_OK == (ret = get_rsuIfaceMaxFileSize(index,&data))){
              snmp_set_var_typed_integer(requests->requestvb, ASN_INTEGER, data);
          }
          break;
      case MODE_SET_RESERVE1: /* Short stack, all requests done in one. */
          DEBUGMSGTL((MY_NAME, "R1:\n"));
          if(RSEMIB_OK == (ret = preserve_rsuIfaceMaxFileSize(index))){
              if(RSEMIB_OK == (ret = action_rsuIfaceMaxFileSize(index,*(requests->requestvb->val.integer)))) {
                  if(RSEMIB_OK == (ret = commit_rsuIfaceMaxFileSize(index))){
                      DEBUGMSGTL((MY_NAME, "COMMIT DONE:\n"));
                  } else {
                      DEBUGMSGTL((MY_NAME, "COMMIT FAIL:\n"));
                  }
              } else {
                  DEBUGMSGTL((MY_NAME, "ACTION FAIL:\n"));
              }
              if(RSEMIB_OK != ret) {
                  if(RSEMIB_OK != undo_rsuIfaceMaxFileSize(index)) { /* UNDO fails nothing we can do. */
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
static int32_t handle_rsuIfaceMaxFileTime(int32_t index,netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret  = RSEMIB_OK;
  int32_t data = 0;

  DEBUGMSGTL((MY_NAME, "%s: Entry:\n",__FUNCTION__));
  switch (reqinfo->mode){
      case MODE_GET:
          DEBUGMSGTL((MY_NAME, "GET:\n"));
          if(RSEMIB_OK == (ret = get_rsuIfaceMaxFileTime(index,&data))){
              snmp_set_var_typed_integer(requests->requestvb, ASN_INTEGER, data);
          }
          break;
      case MODE_SET_RESERVE1: /* Short stack, all requests done in one. */
          DEBUGMSGTL((MY_NAME, "R1:\n"));
          if(RSEMIB_OK == (ret = preserve_rsuIfaceMaxFileTime(index))){
              if(RSEMIB_OK == (ret = action_rsuIfaceMaxFileTime(index,*(requests->requestvb->val.integer)))) {
                  if(RSEMIB_OK == (ret = commit_rsuIfaceMaxFileTime(index))){
                      DEBUGMSGTL((MY_NAME, "COMMIT DONE:\n"));
                  } else {
                      DEBUGMSGTL((MY_NAME, "COMMIT FAIL:\n"));
                  }
              } else {
                  DEBUGMSGTL((MY_NAME, "ACTION FAIL:\n"));
              }
              if(RSEMIB_OK != ret) {
                  if(RSEMIB_OK != undo_rsuIfaceMaxFileTime(index)) { /* UNDO fails nothing we can do. */
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
/* 5.8.2.5 */
static int32_t handle_rsuIfaceLogByDir(int32_t index,netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret  = RSEMIB_OK;
  int32_t data = 0;

  DEBUGMSGTL((MY_NAME, "%s: Entry:\n",__FUNCTION__));
  switch (reqinfo->mode){
      case MODE_GET:
          DEBUGMSGTL((MY_NAME, "GET:\n"));
          if(RSEMIB_OK == (ret = get_rsuIfaceLogByDir(index,&data))){
              snmp_set_var_typed_integer(requests->requestvb, ASN_INTEGER, data);
          }
          break;
      case MODE_SET_RESERVE1: /* Short stack, all requests done in one. */
          DEBUGMSGTL((MY_NAME, "R1:\n"));
          if(RSEMIB_OK == (ret = preserve_rsuIfaceLogByDir(index))){
              if(RSEMIB_OK == (ret = action_rsuIfaceLogByDir(index,*(requests->requestvb->val.integer)))) {
                  if(RSEMIB_OK == (ret = commit_rsuIfaceLogByDir(index))){
                      DEBUGMSGTL((MY_NAME, "COMMIT DONE:\n"));
                  } else {
                      DEBUGMSGTL((MY_NAME, "COMMIT FAIL:\n"));
                  }
              } else {
                  DEBUGMSGTL((MY_NAME, "ACTION FAIL:\n"));
              }
              if(RSEMIB_OK != ret) {
                  if(RSEMIB_OK != undo_rsuIfaceLogByDir(index)) { /* UNDO fails nothing we can do. */
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
static int32_t handle_rsuIfaceName(int32_t index,netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret = RSEMIB_OK;
  uint8_t data[RSU_INTERFACE_NAME_MAX];

  DEBUGMSGTL((MY_NAME, "%s: Entry:\n",__FUNCTION__));
  switch (reqinfo->mode){
      case MODE_GET:
          DEBUGMSGTL((MY_NAME, "GET:\n"));
          memset(data,0x0,sizeof(data));
          if(0 <= (ret = get_rsuIfaceName(index,data))){
              snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR, data, ret);
              ret = RSEMIB_OK;
          }
          break;
      case MODE_SET_RESERVE1:
          DEBUGMSGTL((MY_NAME, "R1:\n"));
          if(RSEMIB_OK == (ret = preserve_rsuIfaceName(index))){
              if(RSEMIB_OK == (ret = action_rsuIfaceName(index,requests->requestvb->val.string,requests->requestvb->val_len))){
                  if(RSEMIB_OK == (ret = commit_rsuIfaceName(index))){
                      DEBUGMSGTL((MY_NAME, "COMMIT DONE:\n"));
                  } else {
                      DEBUGMSGTL((MY_NAME, "COMMIT FAIL:\n"));
                  }
              } else {
                  DEBUGMSGTL((MY_NAME, "ACTION FAIL:\n"));
              }
              if(RSEMIB_OK != ret) {
                  if(RSEMIB_OK != undo_rsuIfaceName(index)) { /* UNDO fails nothing we can do. */
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
static int32_t handle_rsuIfaceStoragePath(int32_t index,netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret  = RSEMIB_OK;
  uint8_t data[RSU_IFCLOG_STORE_PATH_MAX];

  DEBUGMSGTL((MY_NAME, "%s: Entry:\n",__FUNCTION__));
  switch (reqinfo->mode){
      case MODE_GET:
          DEBUGMSGTL((MY_NAME, "GET:\n"));
          memset(data,0x0,sizeof(data));
          if(0 <= (ret = get_rsuIfaceStoragePath(index,data))){
              snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR, data, ret);
              ret = RSEMIB_OK;
          }
          break;
#if 0 /* Read Only */
      case MODE_SET_RESERVE1:
          DEBUGMSGTL((MY_NAME, "R1:\n"));
          if(RSEMIB_OK == (ret = preserve_rsuIfaceStoragePath(index))){
              if(RSEMIB_OK == (ret = action_rsuIfaceStoragePath(index,requests->requestvb->val.string,requests->requestvb->val_len))){
                  if(RSEMIB_OK == (ret = commit_rsuIfaceStoragePath(index))){
                      DEBUGMSGTL((MY_NAME, "COMMIT DONE:\n"));
                  } else {
                      DEBUGMSGTL((MY_NAME, "COMMIT FAIL:\n"));
                  }
              } else {
                  DEBUGMSGTL((MY_NAME, "ACTION FAIL:\n"));
              }
              if(RSEMIB_OK != ret) {
                  if(RSEMIB_OK != undo_rsuIfaceStoragePath(index)) { /* UNDO fails nothing we can do. */
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
#endif
      default:
          ret = RSEMIB_BAD_MODE;
          break;
  }
  return ret;
}
/* 5.6.2.8 */
static int32_t handle_rsuIfaceLogName(int32_t index,netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret = RSEMIB_OK;
  uint8_t data[RSU_IFCLOG_NAME_SIZE_MAX];

  DEBUGMSGTL((MY_NAME, "%s: Entry:\n",__FUNCTION__));
  switch (reqinfo->mode){
      case MODE_GET:
          DEBUGMSGTL((MY_NAME, "GET:\n"));
          memset(data,0x0,sizeof(data));
          if(0 <= (ret = get_rsuIfaceLogName(index,data))){
              snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR, data, ret); 
              ret = RSEMIB_OK;
          }
          break;
#if 0 /* Read Only */
      case MODE_SET_RESERVE1:
          DEBUGMSGTL((MY_NAME, "R1:\n"));
          if(RSEMIB_OK == (ret = preserve_rsuIfaceLogName(index))){
              if(RSEMIB_OK == (ret = action_rsuIfaceLogName(index,requests->requestvb->val.string,requests->requestvb->val_len))){
                  if(RSEMIB_OK == (ret = commit_rsuIfaceLogName(index))){
                      DEBUGMSGTL((MY_NAME, "COMMIT DONE:\n"));
                  } else {
                      DEBUGMSGTL((MY_NAME, "COMMIT FAIL:\n"));
                  }
              } else {
                  DEBUGMSGTL((MY_NAME, "ACTION FAIL:\n"));
              }
              if(RSEMIB_OK != ret) {
                  if(RSEMIB_OK != undo_rsuIfaceLogName(index)) { /* UNDO fails nothing we can do. */
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
#endif
      default:
          ret = RSEMIB_BAD_MODE;
          break;
  }
  return ret;
}
/* 5.8.2.9 */
static int32_t handle_rsuIfaceLogStart(int32_t index,netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret = RSEMIB_OK;
  uint8_t data[MIB_DATEANDTIME_LENGTH];

  DEBUGMSGTL((MY_NAME, "%s: Entry:\n",__FUNCTION__));
  switch (reqinfo->mode){
      case MODE_GET:
          DEBUGMSGTL((MY_NAME, "GET:\n"));
          memset(data,0x0,sizeof(data));
          if(0 <= (ret = get_rsuIfaceLogStart(index,data))){
              snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR, data, ret);
              ret = RSEMIB_OK;
          }
          break;
      case MODE_SET_RESERVE1:
          DEBUGMSGTL((MY_NAME, "R1:\n"));
          if(RSEMIB_OK == (ret = preserve_rsuIfaceLogStart(index))){
              if(RSEMIB_OK == (ret = action_rsuIfaceLogStart(index,requests->requestvb->val.string,requests->requestvb->val_len))){
                  if(RSEMIB_OK == (ret = commit_rsuIfaceLogStart(index))){
                      DEBUGMSGTL((MY_NAME, "COMMIT DONE:\n"));
                  } else {
                      DEBUGMSGTL((MY_NAME, "COMMIT FAIL:\n"));
                  }
              } else {
                  DEBUGMSGTL((MY_NAME, "ACTION FAIL:\n"));
              }
              if(RSEMIB_OK != ret) {
                  if(RSEMIB_OK != undo_rsuIfaceLogStart(index)) { /* UNDO fails nothing we can do. */
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
/* 5.8.2.10 */
static int32_t handle_rsuIfaceLogStop(int32_t index,netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret = RSEMIB_OK;
  uint8_t data[MIB_DATEANDTIME_LENGTH];

  DEBUGMSGTL((MY_NAME, "%s: Entry:\n",__FUNCTION__));
  switch (reqinfo->mode){
      case MODE_GET:
          DEBUGMSGTL((MY_NAME, "GET:\n"));
          memset(data,0x0,sizeof(data));
          if(0 <= (ret = get_rsuIfaceLogStop(index,data))){
              snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR, data, ret);
              ret = RSEMIB_OK;
          }
          break;
      case MODE_SET_RESERVE1:
          DEBUGMSGTL((MY_NAME, "R1:\n"));
          if(RSEMIB_OK == (ret = preserve_rsuIfaceLogStop(index))){
              if(RSEMIB_OK == (ret = action_rsuIfaceLogStop(index,requests->requestvb->val.string,requests->requestvb->val_len))){
                  if(RSEMIB_OK == (ret = commit_rsuIfaceLogStop(index))){
                      DEBUGMSGTL((MY_NAME, "COMMIT DONE:\n"));
                  } else {
                      DEBUGMSGTL((MY_NAME, "COMMIT FAIL:\n"));
                  }
              } else {
                  DEBUGMSGTL((MY_NAME, "ACTION FAIL:\n"));
              }
              if(RSEMIB_OK != ret) {
                  if(RSEMIB_OK != undo_rsuIfaceLogStop(index)) { /* UNDO fails nothing we can do. */
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
/* 5.8.2.11 */
static int32_t handle_rsuIfaceLogOptions(int32_t index,netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret = RSEMIB_OK;
  uint8_t data[RSU_IFCLOG_OPTIONS_LENGTH];

  DEBUGMSGTL((MY_NAME, "%s: Entry:\n",__FUNCTION__));
  switch (reqinfo->mode){
      case MODE_GET:
          DEBUGMSGTL((MY_NAME, "GET:\n"));
          if(RSU_IFCLOG_OPTIONS_LENGTH == (ret = get_rsuIfaceLogOptions(index,data))){
#if defined(ENABLE_ASN_BIT_STR)
              snmp_set_var_typed_value(requests->requestvb, ASN_BIT_STR, data, RSU_IFCLOG_OPTIONS_LENGTH);
#else
              snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR, data, RSU_IFCLOG_OPTIONS_LENGTH);
#endif
              ret = RSEMIB_OK;
          }
          break;
      case MODE_SET_RESERVE1: /* Short stack, all requests done in one. */
          DEBUGMSGTL((MY_NAME, "R1:\n"));
          if(RSEMIB_OK == (ret = preserve_rsuIfaceLogOptions(index))){
              if(RSEMIB_OK == (ret = action_rsuIfaceLogOptions(index,requests->requestvb->val.bitstring,requests->requestvb->val_len))){
                  if(RSEMIB_OK == (ret = commit_rsuIfaceLogOptions(index))){
                      DEBUGMSGTL((MY_NAME, "COMMIT DONE:\n"));
                  } else {
                      DEBUGMSGTL((MY_NAME, "COMMIT FAIL:\n"));
                  }
              } else {
                  DEBUGMSGTL((MY_NAME, "ACTION FAIL:\n"));
              }
              if(RSEMIB_OK != ret) {
                  if(RSEMIB_OK != undo_rsuIfaceLogOptions(index)) { /* UNDO fails nothing we can do. */
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
/* 5.8.2.12 */
static int32_t handle_rsuIfaceLogStatus(int32_t index,netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests, int32_t override)
{
  int32_t ret  = RSEMIB_OK;
  int32_t data = 0;
  int32_t row_status = SNMP_ROW_CREATEANDGO; /* If override is set then don't use requests's value. Use this one. */

  DEBUGMSGTL((MY_NAME, "%s: Entry:\n",__FUNCTION__));
  switch (reqinfo->mode){
      case MODE_GET:
          if(RSEMIB_OK == (ret = get_rsuIfaceLogStatus(index,&data))){
              snmp_set_var_typed_integer(requests->requestvb, ASN_INTEGER, data);
          }
          break;
      case MODE_SET_RESERVE1: /* Short minion stack, all requests done in one. */
          if(0 == override){
              row_status = *(requests->requestvb->val.integer); /* Client index part of CREATE. */
          }
          DEBUGMSGTL((MY_NAME, "R1_BULK_SET: status = %d.\n",row_status));
          if(RSEMIB_OK == (ret = preserve_rsuIfaceLogStatus(index))){
              if(RSEMIB_OK == (ret = action_rsuIfaceLogStatus(index,row_status))){
                  if(RSEMIB_OK == (ret = commit_rsuIfaceLogStatus(index))){
                      DEBUGMSGTL((MY_NAME, "COMMIT DONE:\n")); /* Minions unite, master will be happy! */
                  } else {
                      DEBUGMSGTL((MY_NAME, "COMMIT FAIL:\n"));
                  }
              } else {
                  DEBUGMSGTL((MY_NAME, "ACTION FAIL:\n"));
              }
              if(RSEMIB_OK != ret) { /* UNDO fails nothing we can do. */
                  if(RSEMIB_OK != undo_rsuIfaceLogStatus(index)) {
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
 * 5.8.2: Table handler: Break it down to smaller requests.
 */
int32_t handle_rsuInterfaceLogTable( netsnmp_mib_handler          * handler
                                   , netsnmp_handler_registration * reginfo
                                   , netsnmp_agent_request_info   * reqinfo
                                   , netsnmp_request_info         * requests)
{
  netsnmp_request_info       * request       = NULL;
  uint32_t                     request_count = 0;
  RsuInterfaceLogEntry_t     * table_entry   = NULL;
  netsnmp_table_request_info * request_info  = NULL;
  int32_t                      ret           = SNMP_ERR_NOERROR;
  uint8_t                      raw_oid[MAX_OID_LEN + 2]; /* If table doesn't exist then helper can't help. Go fish for row index. */
  uint32_t                     row_index = 0; /* For readability. Note: index is 1..max */
  int32_t                      row_status = SNMP_ROW_NONEXISTENT;

  handler = handler; /* Silence of the Warnings. */
  reginfo = reginfo;

  DEBUGMSGTL((MY_NAME, "rsuInterfaceLogTable_handler: Entry:\n"));
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
      if(NULL != (table_entry = (RsuInterfaceLogEntry_t *)netsnmp_tdata_extract_entry(request))) {
          if((request_info->colnum < INTERFACE_LOG_INDEX_COLUMN) || (INTERFACE_LOG_STATUS_COLUMN < request_info->colnum)) {
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
      if(RSEMIB_OK != (ret = get_rsuIfaceLogStatus(row_index,&row_status))) {
          DEBUGMSGTL((MY_NAME, "get_rsuIfaceLogStatus Failed: ret=%d\n",ret));
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
          if(INTERFACE_LOG_STATUS_COLUMN != request_info->colnum){/* If row doesn't exist then create now. */
              ret = handle_rsuIfaceLogStatus(row_index, reqinfo, request, 1); /* Do not use value in request. Force createAndGo. */
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
          case INTERFACE_LOG_INDEX_COLUMN:
              ret = handle_rsuIfaceLogIndex(row_index, reqinfo, request);
              break;
          case INTERFACE_LOG_GENERATE_COLUMN:
              ret = handle_rsuIfaceGenerate(row_index, reqinfo, request);
              break;
          case INTERFACE_LOG_MAX_FILE_SIZE_COLUMN:
              ret = handle_rsuIfaceMaxFileSize(row_index, reqinfo, request);
              break;
          case INTERFACE_LOG_MAX_FILE_TIME_COLUMN:
              ret = handle_rsuIfaceMaxFileTime(row_index, reqinfo, request);
              break;
          case INTERFACE_LOG_BY_DIR_COLUMN:
              ret = handle_rsuIfaceLogByDir(row_index, reqinfo, request);
              break;
          case INTERFACE_LOG_IFC_NAME_COLUMN:
              ret = handle_rsuIfaceName(row_index, reqinfo, request);
              break;
          case INTERFACE_LOG_STORE_PATH_COLUMN:
              ret = handle_rsuIfaceStoragePath(row_index, reqinfo, request);
              break;
          case INTERFACE_LOG_NAME_COLUMN:
              ret = handle_rsuIfaceLogName(row_index, reqinfo, request);
              break;
          case INTERFACE_LOG_START_COLUMN:
              ret = handle_rsuIfaceLogStart(row_index, reqinfo, request);
              break;
          case INTERFACE_LOG_STOP_COLUMN:
              ret = handle_rsuIfaceLogStop(row_index, reqinfo, request);
              break;
          case INTERFACE_LOG_OPTIONS_COLUMN:
              ret = handle_rsuIfaceLogOptions(row_index, reqinfo, request);
              break;
          case INTERFACE_LOG_STATUS_COLUMN:
//If already created and status matches then who cares?
              ret = handle_rsuIfaceLogStatus(row_index, reqinfo, request, 0);
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

/* rsuInterfaceLogTable creation functions.*/
STATIC void rsuInterfaceLogTable_removeEntry(netsnmp_tdata * table_data, netsnmp_tdata_row * row)
{
  RsuInterfaceLogEntry_t * entry = NULL;

  if (NULL == row){
      DEBUGMSGTL((MY_NAME,"rsuInterfaceLogTable_removeEntry NULL input.\n"));
      return;
  }
  if (NULL != (entry = (RsuInterfaceLogEntry_t *)row->data)){ /* We allocate, so we free. */
      SNMP_FREE(entry);
  } else {
      DEBUGMSGTL((MY_NAME,"rsuIFMStatusTable_removeEntry row data is NULL!.\n"));
  }
  if (table_data){
      netsnmp_tdata_remove_and_delete_row(table_data, row);
  } else {
      DEBUGMSGTL((MY_NAME,"rsuInterfaceLogTable_removeEntry NULL input.\n"));
      netsnmp_tdata_delete_row(row); 
  }
  return;
}
static void destroy_rsuInterfaceLogTable(void)
{
  netsnmp_tdata_row  * row     = NULL;
  netsnmp_tdata_row  * nextrow = NULL;
  int32_t              i       = 0;

  if(NULL != rsuInterfaceLogTable){
      if(NULL != (row = netsnmp_tdata_row_first(rsuInterfaceLogTable))){
          for(i=0;(row != NULL) && (i < RSU_IFCLOG_MAX); i++){
              nextrow = netsnmp_tdata_row_next(rsuInterfaceLogTable, row);
              rsuInterfaceLogTable_removeEntry(rsuInterfaceLogTable, row);
              row = nextrow;
          }
      } else {
          DEBUGMSGTL((MY_NAME, "destroy_rsuInterfaceLogTable: Table empty. Nothing to do.\n"));
      }
  } else {
      DEBUGMSGTL((MY_NAME, "destroy_rsuInterfaceLogTable: Table is NULL. Did we get called before install?\n"));
  }
}
static int32_t rsuInterfaceLog_createTable(netsnmp_tdata * table_data, RsuInterfaceLogEntry_t * rsuInterfaceLog)
{
  RsuInterfaceLogEntry_t * entry = NULL;
  netsnmp_tdata_row      * row   = NULL;
  int32_t                  i     = 0;
  int32_t                  valid_row_count = 0;

  if(NULL == table_data){
      DEBUGMSGTL((MY_NAME, "rsuInterfaceLog_createTable FAIL: table_data == NULL.\n"));
      return -1;
  }

  for(i=0; i < RSU_IFCLOG_MAX; i++){
      if((rsuInterfaceLog[i].rsuIfaceLogIndex < 1) || (RSU_IFCLOG_MAX < rsuInterfaceLog[i].rsuIfaceLogIndex)){
          return -1; /* Indices must be unique and greater than 1. */
      }
      if(RSEMIB_OK != valid_table_row(rsuInterfaceLog[i].rsuIfaceLogStatus)){
          continue; /* Only add rows that have beeb created. */
      }
      if (NULL == (entry = SNMP_MALLOC_TYPEDEF(RsuInterfaceLogEntry_t))){
          DEBUGMSGTL((MY_NAME, "rsuInterfaceLog SNMP_MALLOC_TYPEDEF FAIL.\n"));
          return -1;
      }
      if (NULL == (row = netsnmp_tdata_create_row())){
          DEBUGMSGTL((MY_NAME, "rsuInterfaceLog netsnmp_tdata_create_row FAIL.\n"));
          SNMP_FREE(entry);
          return -1;
      }

      DEBUGMSGTL((MY_NAME, "rsuInterfaceLog make row i=%d index=%d.\n",i,rsuInterfaceLog[i].rsuIfaceLogIndex));
      row->data = entry;
      memcpy(entry,&rsuInterfaceLog[i],sizeof(RsuInterfaceLogEntry_t));
      netsnmp_tdata_row_add_index(row, ASN_INTEGER , &rsuInterfaceLog[i].rsuIfaceLogIndex, sizeof(int32_t));
      if(SNMPERR_SUCCESS != netsnmp_tdata_add_row(table_data, row)){
          DEBUGMSGTL((MY_NAME, "rsuInterfaceLog netsnmp_tdata_add_row FAIL i=%d.\n",i));
          return -1;
      }
      valid_row_count++;
  } /* for */
  DEBUGMSGTL((MY_NAME, "rsuInterfaceLog valid rows created = %d.\n",valid_row_count));
  return RSEMIB_OK;
}
static int32_t install_rsuInterfaceLogTable(void)
{
    netsnmp_handler_registration * reg = NULL;

    reg = netsnmp_create_handler_registration("rsuInterfaceLogTable"
                                              ,handle_rsuInterfaceLogTable
                                              ,rsuInterfaceLogTable_oid
                                              ,OID_LENGTH(rsuInterfaceLogTable_oid)
                                              ,HANDLER_CAN_RWRITE);

    if (NULL == (rsuInterfaceLogTable = netsnmp_tdata_create_table("rsuInterfaceLogTable", 0))){
        DEBUGMSGTL((MY_NAME, "rsuInterfaceLogTable FAIL.\n"));
        return RSEMIB_INSTALL_HANDLER_FAIL;
    }
    if (NULL == (table_reg_info = SNMP_MALLOC_TYPEDEF(netsnmp_table_registration_info))){
        DEBUGMSGTL((MY_NAME, "SNMP_MALLOC_TYPEDEF FAIL.\n"));
        return RSEMIB_INSTALL_HANDLER_FAIL;
    }
    netsnmp_table_helper_add_indexes(table_reg_info, ASN_INTEGER,0); /* index */                                 

    table_reg_info->min_column = INTERFACE_LOG_GENERATE_COLUMN; /* Columns 2 to 12; Hide index by definition. */
    table_reg_info->max_column = INTERFACE_LOG_STATUS_COLUMN;

    if (SNMPERR_SUCCESS == netsnmp_tdata_register(reg, rsuInterfaceLogTable, table_reg_info)){
        return RSEMIB_OK; /* Table registered but still empty. rebuild_rsuInterfaceLog() will populate */
    } else {
        DEBUGMSGTL((MY_NAME, "netsnmp_tdata_register FAIL.\n"));
        return RSEMIB_INSTALL_HANDLER_FAIL;
    }
}
/* Assumed locked already. Live pointer to db. */
int32_t rebuild_interfaceLog_live(RsuInterfaceLogEntry_t * InterfaceLog)
{
  if(NULL == InterfaceLog) {
      return RSEMIB_BAD_INPUT;
  }
  /* Remove rows but don't remove table reg. */
  destroy_rsuInterfaceLogTable();

  /* Initialize the contents of the table here.*/
  if (RSEMIB_OK != rsuInterfaceLog_createTable(rsuInterfaceLogTable, InterfaceLog)){
      DEBUGMSGTL((MY_NAME, "rebuild_rsuInterfaceLog_live: ERROR: Null row.\n"));
      set_error_states(RSEMIB_REBUILD_FAIL);
      return RSEMIB_REBUILD_FAIL;
  }
  return RSEMIB_OK;
}
static void rebuild_rsuInterfaceLog(void)
{
  int32_t i = 0;
  RsuInterfaceLogEntry_t rsuInterfaceLog[RSU_IFCLOG_MAX];

  memset(rsuInterfaceLog, 0x0, sizeof(rsuInterfaceLog));
  for(i=0; i < RSU_IFCLOG_MAX; i++){
      if(RSEMIB_OK > get_rsuIfaceLogIndex(i+1,&rsuInterfaceLog[i].rsuIfaceLogIndex)){
          set_error_states(RSEMIB_REBUILD_FAIL);
          DEBUGMSGTL((MY_NAME, "rsuIfaceLogIndex rebuild FAIL: i = %d.\n",i));
      }
      if(RSEMIB_OK > get_rsuIfaceGenerate(i+1,(int32_t *)&rsuInterfaceLog[i].rsuIfaceGenerate)){
          set_error_states(RSEMIB_REBUILD_FAIL);
          DEBUGMSGTL((MY_NAME, "rsuIfaceGenerate rebuild FAIL: i = %d.\n",i));
      }
      if(RSEMIB_OK > get_rsuIfaceMaxFileSize(i+1,&rsuInterfaceLog[i].rsuIfaceMaxFileSize)){
          set_error_states(RSEMIB_REBUILD_FAIL);
          DEBUGMSGTL((MY_NAME, "rsuInterfaceLogDestIpAddr rebuild FAIL: i = %d.\n",i));
      }
      if(RSEMIB_OK > get_rsuIfaceMaxFileTime(i+1,&rsuInterfaceLog[i].rsuIfaceMaxFileTime)){
          set_error_states(RSEMIB_REBUILD_FAIL);
          DEBUGMSGTL((MY_NAME, "rsuIfaceMaxFileTime rebuild FAIL: i = %d.\n",i));
      }
      if(RSEMIB_OK > get_rsuIfaceLogByDir(i+1,(int32_t *)&rsuInterfaceLog[i].rsuIfaceLogByDir)){
          set_error_states(RSEMIB_REBUILD_FAIL);
          DEBUGMSGTL((MY_NAME, "rsuIfaceLogByDir rebuild FAIL: i = %d.\n",i));
      }
      if(RSEMIB_OK > get_rsuIfaceName(i+1,rsuInterfaceLog[i].rsuIfaceName)){
          set_error_states(RSEMIB_REBUILD_FAIL);
          DEBUGMSGTL((MY_NAME, "rsuIfaceName rebuild FAIL: i = %d.\n",i));
      }
      if(RSEMIB_OK > get_rsuIfaceStoragePath(i+1,rsuInterfaceLog[i].rsuIfaceStoragePath)){
          set_error_states(RSEMIB_REBUILD_FAIL);
          DEBUGMSGTL((MY_NAME, "rsuIfaceStoragePath rebuild FAIL: i = %d.\n",i));
      }
      if(RSEMIB_OK > get_rsuIfaceLogName(i+1,rsuInterfaceLog[i].rsuIfaceLogName)){
          set_error_states(RSEMIB_REBUILD_FAIL);
          DEBUGMSGTL((MY_NAME, "rsuIfaceLogName rebuild FAIL: i = %d.\n",i));
      }
      if(RSEMIB_OK > get_rsuIfaceLogStart(i+1,rsuInterfaceLog[i].rsuIfaceLogStart)){
          set_error_states(RSEMIB_REBUILD_FAIL);
          DEBUGMSGTL((MY_NAME, "rsuIfaceLogStart rebuild FAIL: i = %d.\n",i));
      }
      if(RSEMIB_OK > get_rsuIfaceLogStop(i+1,rsuInterfaceLog[i].rsuIfaceLogStop)){
          set_error_states(RSEMIB_REBUILD_FAIL);
          DEBUGMSGTL((MY_NAME, "rsuIfaceLogStop rebuild FAIL: i = %d.\n",i));
      }
      if(RSEMIB_OK > get_rsuIfaceLogOptions(i+1,&rsuInterfaceLog[i].rsuIfaceLogOptions)){
          set_error_states(RSEMIB_REBUILD_FAIL);
          DEBUGMSGTL((MY_NAME, "rsuIfaceLogOptions rebuild FAIL: i = %d.\n",i));
      }
      if(RSEMIB_OK > get_rsuIfaceLogStatus(i+1,&rsuInterfaceLog[i].rsuIfaceLogStatus)){
          set_error_states(RSEMIB_REBUILD_FAIL);
          DEBUGMSGTL((MY_NAME, "rsuIfaceLogStatus rebuild FAIL: i = %d.\n",i));
      }
  }
  /* Remove rows but don't remove table reg. */
  destroy_rsuInterfaceLogTable();

  /* Initialize the contents of the table here.*/
  if (RSEMIB_OK != rsuInterfaceLog_createTable(rsuInterfaceLogTable, &rsuInterfaceLog[0])){
      DEBUGMSGTL((MY_NAME, "rsuInterfaceLog_createTable FAIL.\n"));
      set_error_states(RSEMIB_REBUILD_FAIL);
  }
}
void install_rsuInterfaceLog_handlers(void)
{
  /* Reset statics across soft reset. */
  rsuInterfaceLogTable         = NULL;
  table_reg_info               = NULL;
  interfaceLog_error_states    = 0x0;
  prior_error_states           = 0x0;
  shm_interfaceLog_ptr         = NULL;
  rolling_counter              = 0;

  /* 5.8.1. */
  if(MIB_REGISTERED_OK != netsnmp_register_scalar(netsnmp_create_handler_registration
                          ("maxRsuInterfaceLogs", 
                           handle_maxRsuInterfaceLogs,
                           maxRsuInterfaceLogs_oid,
                           OID_LENGTH(maxRsuInterfaceLogs_oid),
                           HANDLER_CAN_RONLY))){
      DEBUGMSGTL((MY_NAME, "maxRsuInterfaceLogs install FAIL.\n"));
      set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
  }
  /* 5.8.2.1 - 5.6.2.12: Install table created with data from MIB. */
  if(RSEMIB_OK != install_rsuInterfaceLogTable()){
      DEBUGMSGTL((MY_NAME, "InterfaceLogTable install FAIL.\n"));
      set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
  }

  /* Populate the table. */
  rebuild_rsuInterfaceLog();
}
void destroy_rsuInterfaceLog(void)
{
  destroy_rsuInterfaceLogTable();

  /* Free table info. */
  if (NULL != table_reg_info){
      SNMP_FREE(table_reg_info);
  }
  /* Free table reg. */
  if (NULL != rsuInterfaceLogTable){
      SNMP_FREE(rsuInterfaceLogTable);
  }
}
/* RSU App thread support. */
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
static int32_t check_stopStart_UTC_dsec(uint64_t StartTimeDsec, uint64_t StopTimeDsec)
{
  uint64_t UTC_dsec = (uint64_t)(get_utcMsec()/(uint64_t)100);

  /* Wellness checks. */
  if(StopTimeDsec <= StartTimeDsec){
      if(0 == (rolling_counter % OUTPUT_MODULUS)) {
          I2V_DBG_LOG(MY_ERR_LEVEL, MY_I2V_NAME,"check_stopStart_dates: stop <= start time: 0x%lu vs 0x%lu.\n");
      }
      return SNMP_ROW_NOTREADY;
  }
  if(StopTimeDsec < UTC_dsec){
      DEBUGMSGTL((MY_NAME, "check_stopStart_UTC_dsec: stop(0x%lx) < now(0x%lx): Expired & ready for destroy.\n", StopTimeDsec, UTC_dsec));
      if(0 == (rolling_counter % OUTPUT_MODULUS)) {
          I2V_DBG_LOG(MY_ERR_LEVEL, MY_I2V_NAME,"check_stopStart_UTC_dsec: stop(0x%lx) < now(0x%lx): Expired & ready for destroy.\n", StopTimeDsec, UTC_dsec);
      }
      /* We never destroy rows. Snmp client manages rows any way they want. */
      return SNMP_ROW_NOTREADY; /* Expired. Set to NOTREADY. */
  }
  if(StartTimeDsec <= UTC_dsec) { /* This message is active assuming it is well formed. */
      return SNMP_ROW_ACTIVE;
  }
  if(UTC_dsec < StartTimeDsec) {
      /* These files stick around so lots of debug if enabled. */
      if(0 == (rolling_counter % OUTPUT_MODULUS)) {
          I2V_DBG_LOG(MY_ERR_LEVEL, MY_I2V_NAME,"check_stopStart_UTC_dsec: now < start: Not in service yet.\n");
      }
      return SNMP_ROW_NOTINSERVICE; /* Time to start not here but valid nonetheless. */
  }
  if(0 == (rolling_counter % OUTPUT_MODULUS)) {
      I2V_DBG_LOG(MY_ERR_LEVEL, MY_I2V_NAME,"check_stopStart_UTC_dsec: internal error.\n");
  }
  return SNMP_ROW_NOTREADY; /* Should not be possible to get here. */
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
static void force_interfaceLogShm_update(void) 
{
  int32_t i;

  /* One time load from MIB to SHM. After that only client update will trigger update on a single row. */
  for(i=0;i<RSU_IFCLOG_MAX; i++){
      if(RSEMIB_OK != update_interfaceLog_shm(&ntcip_1218_mib.rsuInterfaceLogTable[i])){
         set_error_states(RSEMIB_SHM_SEED_FAILED);
      }
  }
}
void *interfaceLogThr(void __attribute__((unused)) *arg)
{
  size_t   shm_interfaceLog_size = sizeof(interfaceLog_t) * RSU_IFCLOG_MAX;
  int32_t  i = 0, j = 0;
  int32_t  row_status = SNMP_ROW_NOTREADY;
  int32_t  dirty = 1; /* Always force update to start */
  int32_t  weareactive = 0;
  /* Open SHM. */
  if ((shm_interfaceLog_ptr = mib_share_init(shm_interfaceLog_size, INTERFACE_LOG_SHM_PATH)) == NULL) {
      DEBUGMSGTL((MY_NAME,"interfaceLogThr: SHM init failed.\n"));
  } else {
      /* Because zero is a valid socket. */
      for(i=0; i<RSU_IFCLOG_MAX; i++){
          shm_interfaceLog_ptr[i].ifaceLogStatus = SNMP_ROW_NONEXISTENT;
      }
      for(i=0;i<5;i++) /* Give RSU apps a chance to catch up before updating SHM. */
          sleep(1);
      while(mainloop) {
          sleep(1);
          for(i=0,weareactive=0; i<RSU_IFCLOG_MAX; i++){
              if(RSEMIB_OK == ntcip1218_lock()){
                  /* Check start/stop times for active. */
                  if(  (SNMP_ROW_ACTIVE == ntcip_1218_mib.rsuInterfaceLogTable[i].rsuIfaceLogStatus)
                     ||(SNMP_ROW_NOTREADY == ntcip_1218_mib.rsuInterfaceLogTable[i].rsuIfaceLogStatus)
                     ||(SNMP_ROW_NOTINSERVICE == ntcip_1218_mib.rsuInterfaceLogTable[i].rsuIfaceLogStatus)) {
                      row_status = check_stopStart_UTC_dsec(ntcip_1218_mib.rsuInterfaceLogTable[i].start_utc_dsec,ntcip_1218_mib.rsuInterfaceLogTable[i].stop_utc_dsec);
                      //I2V_DBG_LOG(MY_ERR_LEVEL, MY_I2V_NAME,"check_stopStart_UTC_dsec: now:status(%d) vs shm:status(%d)\n", ntcip_1218_mib.rsuInterfaceLogTable[i].rsuIfaceLogStatus, row_status);
                      if(ntcip_1218_mib.rsuInterfaceLogTable[i].rsuIfaceLogStatus != row_status) {
                          I2V_DBG_LOG(MY_ERR_LEVEL, MY_I2V_NAME,"check_stopStart_UTC_dsec: status(%d) change to status(%d)\n", ntcip_1218_mib.rsuInterfaceLogTable[i].rsuIfaceLogStatus, row_status);
                          ntcip_1218_mib.rsuInterfaceLogTable[i].rsuIfaceLogStatus = row_status;
                          /* Store generate complete to disk, else next boot will restart. */
                          commit_interfaceLog_to_disk(&ntcip_1218_mib.rsuInterfaceLogTable[i]);
                          dirty = 1;
                      }
                      if(SNMP_ROW_ACTIVE != row_status){ /* There could be lots of messages not ready to send yet so watch out. */
                          DEBUGMSGTL((MY_NAME_EXTRA,"interfaceLogThr: Request for active row=%d denied. Bad or not ready time to start.\n",i));
                      }
                      weareactive = 1; /* a row exists that the user is editing. */
                  }
                  if(SNMP_ROW_ACTIVE == ntcip_1218_mib.rsuInterfaceLogTable[i].rsuIfaceLogStatus){
                      /* User can change rsuID anytime. */
                      if(0 != memcmp(shm_interfaceLog_ptr[i].rsuID,ntcip_1218_mib.rsuID,RSU_ID_LENGTH_MAX)) {
                          memcpy(shm_interfaceLog_ptr[i].rsuID, ntcip_1218_mib.rsuID,RSU_ID_LENGTH_MAX);
                          if(RSU_ID_LENGTH_MAX <= ntcip_1218_mib.rsuID_length) {
                              ntcip_1218_mib.rsuID_length = RSU_ID_LENGTH_MAX; 
                          }
                          shm_interfaceLog_ptr[i].rsuID_length = ntcip_1218_mib.rsuID_length;
                          /* Limit the ID in case user gets crazy with special chars. */
                          for(j=0;(j<ntcip_1218_mib.rsuID_length);j++) {
                              if(0 == isalnum(shm_interfaceLog_ptr[i].rsuID[j])){
                                  shm_interfaceLog_ptr[i].rsuID[j] = '_';
                              }
                          }
                          dirty = 1;
                      }
                      /* ifclog will only update MIB once log is finished. */
                      if(   (RSU_IFCLOG_NAME_SIZE_MIN <= shm_interfaceLog_ptr[i].ifaceLogName_length)
                         && (0 != memcmp(shm_interfaceLog_ptr[i].ifaceLogName,
                                         ntcip_1218_mib.rsuInterfaceLogTable[i].rsuIfaceLogName,
                                         RSU_IFCLOG_NAME_SIZE_MAX))) {
                          if(shm_interfaceLog_ptr[i].ifaceLogName_length < RSU_IFCLOG_NAME_SIZE_MAX) {
                              ntcip_1218_mib.rsuInterfaceLogTable[i].rsuIfaceLogName_length = RSU_IFCLOG_NAME_SIZE_MAX;
                          } else {
                              ntcip_1218_mib.rsuInterfaceLogTable[i].rsuIfaceLogName_length = shm_interfaceLog_ptr[i].ifaceLogName_length;
                          }
                          memcpy( ntcip_1218_mib.rsuInterfaceLogTable[i].rsuIfaceLogName
                                 ,shm_interfaceLog_ptr[i].ifaceLogName
                                 ,ntcip_1218_mib.rsuInterfaceLogTable[i].rsuIfaceLogName_length);
                          commit_interfaceLog_to_disk(&ntcip_1218_mib.rsuInterfaceLogTable[i]);
                      }
                  }
                  if(dirty) {
                      force_interfaceLogShm_update();  /* Force update of rows recovered from disk at least once at start. */
                      dirty = 0;
                  }
                  ntcip1218_unlock();
              }
          } /* for */

          /* If an event occurs then output immediately. Otherwise only if row exists. */
          if(   (interfaceLog_error_states != prior_error_states) 
             || ((weareactive) && (0 == (rolling_counter % OUTPUT_MODULUS)))) {
              I2V_DBG_LOG(MY_ERR_LEVEL, MY_I2V_NAME,"(0x%x)\n",interfaceLog_error_states);
          }
          prior_error_states = interfaceLog_error_states;
          rolling_counter++;
      }
      /* Close SHM. */
      mib_share_kill(shm_interfaceLog_ptr, shm_interfaceLog_size );
      shm_interfaceLog_ptr = NULL; 
  }
  DEBUGMSGTL((MY_NAME,"interfaceLogThr: Exit."));
  I2V_DBG_LOG(MY_ERR_LEVEL, MY_I2V_NAME,"EXIT: (0x%x)\n",interfaceLog_error_states);
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
int32_t commit_interfaceLog_to_disk(RsuInterfaceLogEntry_t * InterfaceLog)
{
  FILE    * file_out       = NULL;
  uint32_t  mib_byte_count = 0;
  uint32_t  write_count    = 0;
  int32_t   ret            = RSEMIB_OK;

  if(NULL == InterfaceLog){
      ret = RSEMIB_BAD_INPUT; 
  } else {
      DEBUGMSGTL((MY_NAME, "commit_to_disk:fopen(%s).\n",InterfaceLog->filePathName));
      if ((file_out = fopen(InterfaceLog->filePathName, "wb")) == NULL){
          DEBUGMSGTL((MY_NAME, "commit_to_disk: fopen fail.\n"));
          ret = RSEMIB_FOPEN_FAIL;
      } else {
          InterfaceLog->crc = (uint32_t)crc16((const uint8_t *)InterfaceLog, sizeof(RsuInterfaceLogEntry_t) - 4);
          mib_byte_count = sizeof(RsuInterfaceLogEntry_t);
          write_count = fwrite((uint8_t *)InterfaceLog, mib_byte_count, 1, file_out);
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
int32_t update_interfaceLog_shm(RsuInterfaceLogEntry_t * InterfaceLog)
{
  int32_t i; /* For readability. */

  if(NULL == shm_interfaceLog_ptr) { /* Should never happen by order of startup. */
      DEBUGMSGTL((MY_NAME, "update_interfaceLog_shm: RSEMIB_SHM_INIT_FAILED.\n"));
      set_error_states(RSEMIB_SHM_INIT_FAILED);
      return RSEMIB_SHM_INIT_FAILED; 
  }
  if(NULL == InterfaceLog) {
      DEBUGMSGTL((MY_NAME, "update_interfaceLog_shm: NULL == InterfaceLog.\n"));
      set_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;    
  }
  i = InterfaceLog->rsuIfaceLogIndex - 1;
  if((i < 0) || (RSU_IFCLOG_MAX <= i)){
      DEBUGMSGTL((MY_NAME, "update_interfaceLog_shm: RSEMIB_BAD_INDEX.\n"));
      set_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }

  shm_interfaceLog_ptr[i].dirty = 0;
  shm_interfaceLog_ptr[i].ifaceGenerate = InterfaceLog->rsuIfaceGenerate;
  shm_interfaceLog_ptr[i].ifaceMaxFileSize = InterfaceLog->rsuIfaceMaxFileSize;
  shm_interfaceLog_ptr[i].ifaceMaxFileTime = InterfaceLog->rsuIfaceMaxFileTime;
  shm_interfaceLog_ptr[i].ifaceLogByDir = InterfaceLog->rsuIfaceLogByDir;

  memcpy(shm_interfaceLog_ptr[i].ifaceName, InterfaceLog->rsuIfaceName, RSU_INTERFACE_NAME_MAX);
  shm_interfaceLog_ptr[i].ifaceName_length = InterfaceLog->rsuIfaceName_length;

  memcpy(shm_interfaceLog_ptr[i].ifaceStoragePath,InterfaceLog->rsuIfaceStoragePath,RSU_IFCLOG_STORE_PATH_MAX);
  shm_interfaceLog_ptr[i].ifaceStoragePath_length = InterfaceLog->rsuIfaceStoragePath_length;

  memcpy(shm_interfaceLog_ptr[i].ifaceLogName,InterfaceLog->rsuIfaceLogName, RSU_IFCLOG_NAME_SIZE_MAX);
  shm_interfaceLog_ptr[i].ifaceLogName_length = InterfaceLog->rsuIfaceLogName_length;

  shm_interfaceLog_ptr[i].ifaceLogOptions = InterfaceLog->rsuIfaceLogOptions;
  shm_interfaceLog_ptr[i].ifaceLogStatus = InterfaceLog->rsuIfaceLogStatus;

  shm_interfaceLog_ptr[i].dirty = 1;

  return RSEMIB_OK;
}