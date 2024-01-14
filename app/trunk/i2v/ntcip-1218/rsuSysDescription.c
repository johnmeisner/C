/**************************************************************************
 *                                                                        *
 *     File Name:  rsuSysDescription.c                                    *
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
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
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
#define MY_NAME       "SysDesc"
/* Mandatory: For debugging and syslog output. */
#define MY_I2V_NAME   MY_NAME
#define MY_ERR_LEVEL  LEVEL_PRIV    /* from i2v_util.h */

/* rsuSysDescription.c specific. */
#define RSEMIB_FIRMWARE_NOT_READY   -31
#define SYS_DESC_VERIFY_UPDATE_FAIL -32

/* Theoritical column ids. Make sure they follow in order with columns and unique. */
#define RSU_MIB_VERSION_OID      1
#define RSU_FIRMWARE_VERSION_OID 2
#define RSU_LOCATION_DESC_OID    3
#define RSU_ID_OID               4
#define RSU_LAT_OID              5
#define RSU_LON_OID              6
#define RSU_ELV_OID              7
#define RSU_ELV_OFFSET_OID       8
#define RSU_INSTALL_UPDATE_OID   9
#define RSU_INSTALL_FILE_OID     10
#define RSU_INSTALL_PATH_OID     11
#define RSU_INSTALL_STATUS_OID   12
#define RSU_INSTALL_TIME_OID     13
#define RSU_INSTALL_STATUS_MSG_OID   14
#define RSU_SCHEDULE_INSTALL_TIME_OID     15

#define RSU_MIB_VERSION_NAME      "rsuMibVersion"
#define RSU_FIRMWARE_VERSION_NAME "rsuFirmwareVersion"
#define RSU_LOCATION_DESC_NAME    "rsuLocationDesc"
#define RSU_ID_NAME               "rsuID"
#define RSU_LAT_NAME              "rsuLocationLat"
#define RSU_LON_NAME              "rsuLocationLon"
#define RSU_ELV_NAME              "rsuLocationElv"
#define RSU_ELV_OFFSET_NAME       "rsuElevationOffset"
#define RSU_INSTALL_UPDATE_NAME   "rsuInstallUpdate"
#define RSU_INSTALL_FILE_NAME     "rsuInstallFile"
#define RSU_INSTALL_PATH_NAME     "rsuInstallPath"
#define RSU_INSTALL_STATUS_NAME   "rsuInstallStatus"
#define RSU_INSTALL_TIME_NAME     "rsuInstallTime"
#define RSU_INSTALL_STATUS_MSG_NAME   "rsuInstallStatusMessage"
#define RSU_SCHEDULE_INSTALL_TIME_NAME     "rsuScheduledInstallTime"

/******************************************************************************
 * 5.14.1 - 5.14.15: System Description: { rsu 13 }
 * 
 *  uint8_t            rsuMibVersion[RSUMIB_VERSION_LENGTH_MAX];
 *  uint8_t            rsuFirmwareVersion[RSU_FIRMWARE_VERSION_LENGTH_MAX];
 *  uint8_t            rsuLocationDesc[RSU_LOCATION_DESC_LENGTH_MAX];
 *  uint8_t            rsuID[RSU_ID_LENGTH_MAX];
 *  int32_t            rsuLocationLat;
 *  int32_t            rsuLocationLon;
 *  int32_t            rsuLocationElv;
 *  int32_t            rsuElevationOffset;
 *  int32_t            rsuInstallUpdate;
 *  uint8_t            rsuInstallFile[RSU_INSTALL_FILE_NAME_LENGTH_MAX];
 *  uint8_t            rsuInstallPath[RSU_INSTALL_PATH_LENGTH_MAX];
 *  rsuInstallStatus_e rsuInstallStatus;
 *  uint8_t            rsuInstallTime[MIB_DATEANDTIME_LENGTH]; 
 *  uint8_t            rsuInstallStatusMessage[RSU_INSTALL_STATUS_MSG_LENGTH_MAX];
 *  uint8_t            rsuScheduledInstallTime[MIB_DATEANDTIME_LENGTH]; 
 *
 ******************************************************************************/

const oid rsuMibVersion_oid[]           = {1, 3, 6, 1, 4, 1, 1206, 4, 2, 18, 13 , 1};
const oid rsuFirmwareVersion_oid[]      = {1, 3, 6, 1, 4, 1, 1206, 4, 2, 18, 13 , 2};
const oid rsuLocationDesc_oid[]         = {1, 3, 6, 1, 4, 1, 1206, 4, 2, 18, 13 , 3};
const oid rsuID_oid[]                   = {1, 3, 6, 1, 4, 1, 1206, 4, 2, 18, 13 , 4};
const oid rsuLocationLat_oid[]          = {1, 3, 6, 1, 4, 1, 1206, 4, 2, 18, 13 , 5};
const oid rsuLocationLon_oid[]          = {1, 3, 6, 1, 4, 1, 1206, 4, 2, 18, 13 , 6};
const oid rsuLocationElv_oid[]          = {1, 3, 6, 1, 4, 1, 1206, 4, 2, 18, 13 , 7};
const oid rsuElevationOffset_oid[]      = {1, 3, 6, 1, 4, 1, 1206, 4, 2, 18, 13 , 8};
const oid rsuInstallUpdate_oid[]        = {1, 3, 6, 1, 4, 1, 1206, 4, 2, 18, 13 , 9};
const oid rsuInstallFile_oid[]          = {1, 3, 6, 1, 4, 1, 1206, 4, 2, 18, 13 , 10};
const oid rsuInstallPath_oid[]          = {1, 3, 6, 1, 4, 1, 1206, 4, 2, 18, 13 , 11};
const oid rsuInstallStatus_oid[]        = {1, 3, 6, 1, 4, 1, 1206, 4, 2, 18, 13 , 12};
const oid rsuInstallTime_oid[]          = {1, 3, 6, 1, 4, 1, 1206, 4, 2, 18, 13 , 13};
const oid rsuInstallStatusMessage_oid[] = {1, 3, 6, 1, 4, 1, 1206, 4, 2, 18, 13 , 14};
const oid rsuScheduledInstallTime_oid[] = {1, 3, 6, 1, 4, 1, 1206, 4, 2, 18, 13 , 15};

static uint32_t rsuSysDescription_error_states = 0x0;

extern ntcip_1218_mib_t  ntcip_1218_mib; /* Lock it before using it. */

/* 
 * Function Bodies.
 */
static void set_error_states(int32_t error)
{
  /* -1 to -32 */
  if((error < 0) && (error > MIB_ERROR_BASE_2)){
      rsuSysDescription_error_states |= (uint32_t)(0x1) << (abs(error) - 1);
  }
}
uint32_t get_rsuSysDescription_error_states(void)
{
  return rsuSysDescription_error_states;
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
//TODO: Does not revert? uSe v2 values
//          ret = SNMP_ERR_BADVALUE;
          ret = SNMP_ERR_COMMITFAILED;
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
      case RSU_MIB_VERSION_OID:
          strcpy(column_name,RSU_MIB_VERSION_NAME);
          break;
      case RSU_FIRMWARE_VERSION_OID:
          strcpy(column_name,RSU_FIRMWARE_VERSION_NAME);
          break;
      case RSU_LOCATION_DESC_OID:
          strcpy(column_name,RSU_LOCATION_DESC_NAME);
          break;
      case RSU_ID_OID:
          strcpy(column_name,RSU_ID_NAME);
          break;
      case RSU_LAT_OID:
          strcpy(column_name,RSU_LAT_NAME);
          break;
      case RSU_LON_OID:
          strcpy(column_name,RSU_LON_NAME);
          break;
      case RSU_ELV_OID:
          strcpy(column_name,RSU_ELV_NAME);
          break;
      case RSU_ELV_OFFSET_OID:
          strcpy(column_name,RSU_ELV_OFFSET_NAME);
          break;
      case RSU_INSTALL_UPDATE_OID:
          strcpy(column_name,RSU_INSTALL_UPDATE_NAME);
          break;
      case RSU_INSTALL_FILE_OID:
          strcpy(column_name,RSU_INSTALL_FILE_NAME);
          break;
      case RSU_INSTALL_PATH_OID:
          strcpy(column_name,RSU_INSTALL_PATH_NAME);
          break;
      case RSU_INSTALL_STATUS_OID:
          strcpy(column_name,RSU_INSTALL_STATUS_NAME);
          break;
      case RSU_INSTALL_TIME_OID:
          strcpy(column_name,RSU_INSTALL_TIME_NAME);
          break;
      case RSU_INSTALL_STATUS_MSG_OID:
          strcpy(column_name,RSU_INSTALL_STATUS_MSG_NAME);
          break;
      case RSU_SCHEDULE_INSTALL_TIME_OID:
          strcpy(column_name,RSU_SCHEDULE_INSTALL_TIME_NAME);
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

/* 5.14.1 */
static int32_t handle_rsuMibVersion(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo,
                             netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret = RSEMIB_OK;
  uint8_t data[RSUMIB_VERSION_LENGTH_MAX];

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
              if(0 <= (ret = get_rsuMibVersion(data))){
                  snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR, data, ret);
                  ret = RSEMIB_OK;
              }
              break;
          default:
              ret = RSEMIB_BAD_MODE;
              break;
      }
      /* scalars and like are always index 0 with unqiue column.*/
      print_request_result(0, RSU_MIB_VERSION_OID,requests,reqinfo->mode,ret);
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
/* 5.14.2 */
static int32_t handle_rsuFirmwareVersion(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo,
                                 netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret = RSEMIB_OK;
  uint8_t data[RSU_FIRMWARE_VERSION_LENGTH_MAX];

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
              if(0 <= (ret = get_rsuFirmwareVersion(data))){
                  snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR, data, ret);
                  ret = RSEMIB_OK;
              }
              break;
          default:
              ret = RSEMIB_BAD_MODE;
              break;
      }
      /* scalars and like are always index 0 with unqiue column.*/
      print_request_result(0, RSU_FIRMWARE_VERSION_OID,requests,reqinfo->mode,ret);
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
/* 5.14.3 */
static int32_t handle_rsuLocationDesc(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo,
                                 netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret = RSEMIB_OK;
  uint8_t data[RSU_LOCATION_DESC_LENGTH_MAX];

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
              if(0 <= (ret = get_rsuLocationDesc(data))){
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
              if(RSEMIB_OK == (ret = preserve_rsuLocationDesc())){
                  ret = action_rsuLocationDesc(requests->requestvb->val.string,requests->requestvb->val_len);
              }
              break;
          case MODE_SET_COMMIT:
              DEBUGMSGTL((MY_NAME, "COMMIT:\n"));
              if(RSEMIB_OK == (ret = commit_rsuLocationDesc())){
                  /*
                   * Back end magic here.
                   */
              }
              break;
          case MODE_SET_UNDO:
              DEBUGMSGTL((MY_NAME, "UNDO:\n"));
              ret = undo_rsuLocationDesc();
              break;
          default:
              ret = RSEMIB_BAD_MODE;
              break;
      }
      /* scalars and like are always index 0 with unqiue column.*/
      print_request_result(0, RSU_LOCATION_DESC_OID,requests,reqinfo->mode,ret);
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
/* 5.14.4 */
static int32_t handle_rsuID(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo,
                                 netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret = RSEMIB_OK;
  uint8_t data[RSU_ID_LENGTH_MAX];

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
              if(0 <= (ret = get_rsuID(data))){
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
              if(RSEMIB_OK == (ret = preserve_rsuID())){
                  ret = action_rsuID(requests->requestvb->val.string,requests->requestvb->val_len);
              }
              break;
          case MODE_SET_COMMIT:
              DEBUGMSGTL((MY_NAME, "COMMIT:\n"));
              if(RSEMIB_OK == (ret = commit_rsuID())){
                  /*
                   * Back end magic here.
                   */
              }
              break;
          case MODE_SET_UNDO:
              DEBUGMSGTL((MY_NAME, "UNDO:\n"));
              ret = undo_rsuID();
              break;
          default:
              ret = RSEMIB_BAD_MODE;
              break;
      }
      /* scalars and like are always index 0 with unqiue column.*/
      print_request_result(0, RSU_ID_OID,requests,reqinfo->mode,ret);
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
/* 5.14.5 */
static int32_t handle_rsuLocationLat(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo,
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
              if(RSEMIB_OK == (ret = get_rsuLocationLat(&data))){
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
              if(RSEMIB_OK == (ret = preserve_rsuLocationLat())){
                  ret = action_rsuLocationLat( *(requests->requestvb->val.integer));
              }
              break;
          case MODE_SET_COMMIT:
              DEBUGMSGTL((MY_NAME, "COMMIT:\n"));
              if(RSEMIB_OK == (ret = commit_rsuLocationLat())){
                  /*
                   * Back end magic here.
                   */
              }
              break;
          case MODE_SET_UNDO:
              DEBUGMSGTL((MY_NAME, "UNDO:\n"));
              ret = undo_rsuLocationLat();
              break;
          default:
              ret = RSEMIB_BAD_MODE;
              break;
      }
      /* scalars and like are always index 0 with unqiue column.*/
      print_request_result(0, RSU_LAT_OID,requests,reqinfo->mode,ret);
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
/* 5.14.6 */
static int32_t handle_rsuLocationLon(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo,
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
              if(RSEMIB_OK == (ret = get_rsuLocationLon(&data))){
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
              if(RSEMIB_OK == (ret = preserve_rsuLocationLon())){
                  ret = action_rsuLocationLon( *(requests->requestvb->val.integer));
              }
              break;
          case MODE_SET_COMMIT:
              DEBUGMSGTL((MY_NAME, "COMMIT:\n"));
              if(RSEMIB_OK == (ret = commit_rsuLocationLon())){
                  /*
                   * Back end magic here.
                   */
              }
              break;
          case MODE_SET_UNDO:
              DEBUGMSGTL((MY_NAME, "UNDO:\n"));
              ret = undo_rsuLocationLon();
              break;
          default:
              ret = RSEMIB_BAD_MODE;
              break;
      }
      /* scalars and like are always index 0 with unqiue column.*/
      print_request_result(0, RSU_LON_OID,requests,reqinfo->mode,ret);
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
/* 5.14.7 */
static int32_t handle_rsuLocationElv(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo,
                                 netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret  = RSEMIB_OK;
  int32_t data = 0x0;

  handler = handler; /* Silence of the Warnings. */
  reginfo = reginfo;
  DEBUGMSGTL((MY_NAME, "%s: Entry:\n",__FUNCTION__));
  if((NULL == reqinfo) || (NULL == requests)) {
      ret = RSEMIB_BAD_INPUT;
  } else {
      switch (reqinfo->mode){
          case MODE_GET:
              DEBUGMSGTL((MY_NAME, "GET:\n"));
              if(RSEMIB_OK == (ret = get_rsuLocationElv(&data))){
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
              if(RSEMIB_OK == (ret = (preserve_rsuLocationElv()))){
                  ret = action_rsuLocationElv( *(requests->requestvb->val.integer));
              }
              break;
          case MODE_SET_COMMIT:
              DEBUGMSGTL((MY_NAME, "COMMIT:\n"));
              if(RSEMIB_OK == (ret = commit_rsuLocationElv())){
                  /*
                   * Back end magic here.
                   */
              }
              break;
          case MODE_SET_UNDO:
              DEBUGMSGTL((MY_NAME, "UNDO:\n"));
              ret = undo_rsuLocationElv();
              break;
          default:
              ret = RSEMIB_BAD_MODE;
              break;
      }
      /* scalars and like are always index 0 with unqiue column.*/
      print_request_result(0, RSU_ELV_OID,requests,reqinfo->mode,ret);
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
/* 5.14.8 */
static int32_t handle_rsuElevationOffset(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo,
                                 netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret  = RSEMIB_OK;
  int32_t data = 0x0;

  handler = handler; /* Silence of the Warnings. */
  reginfo = reginfo;
  DEBUGMSGTL((MY_NAME, "%s: Entry:\n",__FUNCTION__));
  if((NULL == reqinfo) || (NULL == requests)) {
      ret = RSEMIB_BAD_INPUT;
  } else {
      switch (reqinfo->mode){
          case MODE_GET:
              DEBUGMSGTL((MY_NAME, "GET:\n"));
              if(RSEMIB_OK == (ret = get_rsuElevationOffset(&data))){
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
              if(RSEMIB_OK == (ret = preserve_rsuElevationOffset())){
                  ret = action_rsuElevationOffset( *(requests->requestvb->val.integer));
              }
              break;
          case MODE_SET_COMMIT:
              DEBUGMSGTL((MY_NAME, "COMMIT:\n"));
              if(RSEMIB_OK == (ret = commit_rsuElevationOffset())){
                  /*
                   * Back end magic here.
                   */
              }
              break;
          case MODE_SET_UNDO:
              DEBUGMSGTL((MY_NAME, "UNDO:\n"));
              ret = undo_rsuElevationOffset();
              break;
          default:
              ret = RSEMIB_BAD_MODE;
              break;
      }
      /* scalars and like are always index 0 with unqiue column.*/
      print_request_result(0, RSU_ELV_OFFSET_OID,requests,reqinfo->mode,ret);
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
/* 5.14.9 */
static int32_t handle_rsuInstallUpdate(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo,
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
              if(RSEMIB_OK == (ret = get_rsuInstallUpdate(&data))){
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
              if(RSEMIB_OK == (ret = preserve_rsuInstallUpdate())){
                  ret = action_rsuInstallUpdate( *(requests->requestvb->val.integer));
              }
              break;
          case MODE_SET_COMMIT:
              DEBUGMSGTL((MY_NAME, "COMMIT:\n"));
              if(RSEMIB_OK == (ret = commit_rsuInstallUpdate())){
                  /*
                   * Back end magic here.
                   */
              }
              break;
          case MODE_SET_UNDO:
              DEBUGMSGTL((MY_NAME, "UNDO:\n"));
              ret = undo_rsuInstallUpdate();
              break;
          default:
              ret = RSEMIB_BAD_MODE;
              break;
      }
      /* scalars and like are always index 0 with unqiue column.*/
      print_request_result(0, RSU_INSTALL_UPDATE_OID,requests,reqinfo->mode,ret);
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
/* 5.14.10 */
static int32_t handle_rsuInstallFile(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo,
                                 netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret = RSEMIB_OK;
  uint8_t data[RSU_INSTALL_FILE_NAME_LENGTH_MAX];

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
              if(0 <= (ret = get_rsuInstallFile(data))){
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
              if(RSEMIB_OK == (ret = preserve_rsuInstallFile())){
                  ret = action_rsuInstallFile(requests->requestvb->val.string,requests->requestvb->val_len);
              }
              break;
          case MODE_SET_COMMIT:
              DEBUGMSGTL((MY_NAME, "COMMIT:\n"));
              if(RSEMIB_OK == (ret = commit_rsuInstallFile())){
                  /*
                   * Back end magic here.
                   */
              }
              break;
          case MODE_SET_UNDO:
              DEBUGMSGTL((MY_NAME, "UNDO:\n"));
              ret = undo_rsuInstallFile();
              break;
          default:
              ret = RSEMIB_BAD_MODE;
              break;
      }
      /* scalars and like are always index 0 with unqiue column.*/
      print_request_result(0, RSU_INSTALL_FILE_OID,requests,reqinfo->mode,ret);
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
/* 5.14.11 */
static int32_t handle_rsuInstallPath(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo,
                                 netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret = RSEMIB_OK;
  uint8_t data[RSU_INSTALL_PATH_LENGTH_MAX];

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
              if(0 <= (ret = get_rsuInstallPath(data))){
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
              if(RSEMIB_OK == (ret = preserve_rsuInstallPath())){
                  ret = action_rsuInstallPath(requests->requestvb->val.string,requests->requestvb->val_len);
              }
              break;
          case MODE_SET_COMMIT:
              DEBUGMSGTL((MY_NAME, "COMMIT:\n"));
              if(0 == (ret = commit_rsuInstallPath())){
                  /*
                   * Back end magic here.
                   */
              }
              break;
          case MODE_SET_UNDO:
              DEBUGMSGTL((MY_NAME, "UNDO:\n"));
              ret = undo_rsuInstallPath();
              break;
          default:
              ret = RSEMIB_BAD_MODE;
              break;
      }
      /* scalars and like are always index 0 with unqiue column.*/
      print_request_result(0, RSU_INSTALL_PATH_OID,requests,reqinfo->mode,ret);
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
/* 5.14.12 */
static int32_t handle_rsuInstallStatus(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo,
                                 netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret  = RSEMIB_OK;
  int32_t data = 0x0;

  handler = handler; /* Silence of the Warnings. */
  reginfo = reginfo;
  DEBUGMSGTL((MY_NAME, "%s: Entry:\n",__FUNCTION__));
  if((NULL == reqinfo) || (NULL == requests)) {
      ret = RSEMIB_BAD_INPUT;
  } else {
      switch (reqinfo->mode){
          case MODE_GET:
              DEBUGMSGTL((MY_NAME, "GET:\n"));
              if(RSEMIB_OK == (ret = get_rsuInstallStatus(&data))){
                  snmp_set_var_typed_integer(requests->requestvb, ASN_INTEGER, data);
              }
              break;
          default:
              ret = RSEMIB_BAD_MODE;
              break;
      }
      /* scalars and like are always index 0 with unqiue column.*/
      print_request_result(0, RSU_INSTALL_STATUS_OID,requests,reqinfo->mode,ret);
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
/* 5.14.13 */
static int32_t handle_rsuInstallTime(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo,
                                 netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret = RSEMIB_OK;
  uint8_t data[MIB_DATEANDTIME_LENGTH];

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
              if(RSEMIB_OK == (ret = get_rsuInstallTime(data))){
                  snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR, data, MIB_DATEANDTIME_LENGTH);
              }
              break;
          default:
              ret = RSEMIB_BAD_MODE;
              break;
      }
      /* scalars and like are always index 0 with unqiue column.*/
      print_request_result(0, RSU_INSTALL_TIME_OID,requests,reqinfo->mode,ret);
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
/* 5.14.14 */
static int32_t handle_rsuInstallStatusMessage(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo,
                                 netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret = RSEMIB_OK;
  uint8_t data[RSU_INSTALL_STATUS_MSG_LENGTH_MAX];

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
              if(0 <= (ret = get_rsuInstallStatusMessage(data))){
                  snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR, data, ret);
                  ret = RSEMIB_OK;
              }
              break;
          default:
              ret = RSEMIB_BAD_MODE;
              break;
      }
      /* scalars and like are always index 0 with unqiue column.*/
      print_request_result(0, RSU_INSTALL_STATUS_MSG_OID,requests,reqinfo->mode,ret);
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
/* 5.14.15 */
static int32_t handle_rsuScheduledInstallTime(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo,
                                 netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret = RSEMIB_OK;
  uint8_t data[MIB_DATEANDTIME_LENGTH];

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
              if(RSEMIB_OK == (ret = get_rsuScheduledInstallTime(data))){
                  snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR, data, MIB_DATEANDTIME_LENGTH);
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
              /* Back it up now in case anything goes wrong and UNDO invoked.*/
              if(RSEMIB_OK == (ret = preserve_rsuScheduledInstallTime())){
                  ret = action_rsuScheduledInstallTime(requests->requestvb->val.string,requests->requestvb->val_len);
              }
              break;
          case MODE_SET_COMMIT:
              DEBUGMSGTL((MY_NAME, "COMMIT:\n"));
              if(RSEMIB_OK == (ret = commit_rsuScheduledInstallTime())){
                  /*
                   * Back end magic here.
                   */
              }
              break;
          case MODE_SET_UNDO:
              DEBUGMSGTL((MY_NAME, "UNDO:\n"));
              ret = undo_rsuScheduledInstallTime();
              break;
          default:
              ret = RSEMIB_BAD_MODE;
              break;
      }
      /* scalars and like are always index 0 with unqiue column.*/
      print_request_result(0, RSU_SCHEDULE_INSTALL_TIME_OID,requests,reqinfo->mode,ret);
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
void install_rsuSysDescription_handlers(void)
{
  /* Clear local statics. */
  rsuSysDescription_error_states = 0x0;

  /* 5.14.1 */
  if(MIB_REGISTERED_OK != netsnmp_register_scalar(netsnmp_create_handler_registration
                          ("rsuMibVersion", 
                           handle_rsuMibVersion,
                           rsuMibVersion_oid,
                           OID_LENGTH(rsuMibVersion_oid),
                           HANDLER_CAN_RONLY))){
      DEBUGMSGTL((MY_NAME, "rsuMibVersion install FAIL.\n"));
      set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
  }
  /* 5.14.2 */
  if(MIB_REGISTERED_OK != netsnmp_register_scalar(netsnmp_create_handler_registration
                          ("rsuFirmwareVersion",
                           handle_rsuFirmwareVersion,
                           rsuFirmwareVersion_oid,
                           OID_LENGTH(rsuFirmwareVersion_oid),
                           HANDLER_CAN_RONLY))){
      DEBUGMSGTL((MY_NAME, "rsuFirmwareVersion install FAIL.\n"));
      set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
  }
  /* 5.14.3 */
  if(MIB_REGISTERED_OK != netsnmp_register_scalar(netsnmp_create_handler_registration
                          ("rsuLocationDesc",
                           handle_rsuLocationDesc,
                           rsuLocationDesc_oid,
                           OID_LENGTH(rsuLocationDesc_oid),
                           HANDLER_CAN_RWRITE))){
      DEBUGMSGTL((MY_NAME, "rsuLocationDesc install FAIL.\n"));
      set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
  }
  /* 5.14.4 */
  if(MIB_REGISTERED_OK != netsnmp_register_scalar(netsnmp_create_handler_registration
                          ("rsuID", 
                           handle_rsuID,
                           rsuID_oid,
                           OID_LENGTH(rsuID_oid),
                           HANDLER_CAN_RWRITE))){
      DEBUGMSGTL((MY_NAME, "rsuID install FAIL.\n"));
      set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
  }
  /* 5.14.5 */
  if(MIB_REGISTERED_OK != netsnmp_register_scalar(netsnmp_create_handler_registration
                          ("rsuLocationLat",
                           handle_rsuLocationLat,
                           rsuLocationLat_oid,
                           OID_LENGTH(rsuLocationLat_oid),
                           HANDLER_CAN_RWRITE))){
      DEBUGMSGTL((MY_NAME, "rsuLocationLat install FAIL.\n"));
      set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
  }
  /* 5.14.6 */
  if(MIB_REGISTERED_OK != netsnmp_register_scalar(netsnmp_create_handler_registration
                          ("rsuLocationLon",
                           handle_rsuLocationLon,
                           rsuLocationLon_oid,
                           OID_LENGTH(rsuLocationLon_oid),
                           HANDLER_CAN_RWRITE))){
      DEBUGMSGTL((MY_NAME, "rsuLocationLon install FAIL.\n"));
      set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
  }
  /* 5.14.7 */
  if(MIB_REGISTERED_OK != netsnmp_register_scalar(netsnmp_create_handler_registration
                          ("rsuLocationElv",
                           handle_rsuLocationElv,
                           rsuLocationElv_oid,
                           OID_LENGTH(rsuLocationElv_oid),
                           HANDLER_CAN_RWRITE))){
      DEBUGMSGTL((MY_NAME, "rsuLocationElv install FAIL.\n"));
      set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
  }
  /* 5.14.8 */
  if(MIB_REGISTERED_OK != netsnmp_register_scalar(netsnmp_create_handler_registration
                          ("rsuElevationOffset",
                           handle_rsuElevationOffset,
                           rsuElevationOffset_oid,
                           OID_LENGTH(rsuElevationOffset_oid),
                           HANDLER_CAN_RWRITE))){
      DEBUGMSGTL((MY_NAME, "rsuElevationOffset install FAIL.\n"));
      set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
  }
  /* 5.14.9 */
  if(MIB_REGISTERED_OK != netsnmp_register_scalar(netsnmp_create_handler_registration
                          ("rsuInstallUpdate",
                           handle_rsuInstallUpdate,
                           rsuInstallUpdate_oid,
                           OID_LENGTH(rsuInstallUpdate_oid),
                           HANDLER_CAN_RWRITE))){
      DEBUGMSGTL((MY_NAME, "rsuInstallUpdate install FAIL.\n"));
      set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
  }
  /* 5.14.10 */
  if(MIB_REGISTERED_OK != netsnmp_register_scalar(netsnmp_create_handler_registration
                          ("rsuInstallFile",
                           handle_rsuInstallFile,
                           rsuInstallFile_oid,
                           OID_LENGTH(rsuInstallFile_oid),
                           HANDLER_CAN_RWRITE))){
      DEBUGMSGTL((MY_NAME, "rsuInstallFile install FAIL.\n"));
      set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
  }
  /* 5.14.11 */
  if(MIB_REGISTERED_OK != netsnmp_register_scalar(netsnmp_create_handler_registration
                          ("rsuInstallPath",
                           handle_rsuInstallPath,
                           rsuInstallPath_oid,
                           OID_LENGTH(rsuInstallPath_oid),
                           HANDLER_CAN_RWRITE))){
      DEBUGMSGTL((MY_NAME, "rsuInstallPath install FAIL.\n"));
      set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
  }
  /* 5.14.12 */
  if(MIB_REGISTERED_OK != netsnmp_register_scalar(netsnmp_create_handler_registration
                          ("rsuInstallStatus",
                           handle_rsuInstallStatus,
                           rsuInstallStatus_oid,
                           OID_LENGTH(rsuInstallStatus_oid),
                           HANDLER_CAN_RONLY))){
      DEBUGMSGTL((MY_NAME, "rsuInstallStatus install FAIL.\n"));
      set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
  }
  /* 5.14.13 */
  if(MIB_REGISTERED_OK != netsnmp_register_scalar(netsnmp_create_handler_registration
                          ("rsuInstallTime",
                           handle_rsuInstallTime,
                           rsuInstallTime_oid,
                           OID_LENGTH(rsuInstallTime_oid),
                           HANDLER_CAN_RONLY))){
      DEBUGMSGTL((MY_NAME, "rsuInstallTime install FAIL.\n"));
      set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
  }
  /* 5.14.14 */
  if(MIB_REGISTERED_OK != netsnmp_register_scalar(netsnmp_create_handler_registration
                          ("rsuInstallStatusMessage",
                           handle_rsuInstallStatusMessage,
                           rsuInstallStatusMessage_oid,
                           OID_LENGTH(rsuInstallStatusMessage_oid),
                           HANDLER_CAN_RONLY))){
      DEBUGMSGTL((MY_NAME, "rsuInstallStatusMessage install FAIL.\n"));
      set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
  }
  /* 5.14.15 */
  if(MIB_REGISTERED_OK != netsnmp_register_scalar(netsnmp_create_handler_registration
                          ("rsuScheduledInstallTime",
                           handle_rsuScheduledInstallTime,
                           rsuScheduledInstallTime_oid,
                           OID_LENGTH(rsuScheduledInstallTime_oid),
                           HANDLER_CAN_RWRITE))){
      DEBUGMSGTL((MY_NAME, "rsuScheduledInstallTime install FAIL.\n"));
      set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
  }
}
void destroy_rsuSysDescription(void)
{
}
STATIC uint16_t crc16(const uint8_t * data_p, size_t length)
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
int32_t commit_sysDesc_to_disk(SystemDescription_t * sysDesc) 
{
  FILE    * file_out       = NULL;
  uint32_t  mib_byte_count = 0;
  uint32_t  write_count    = 0;
  int32_t   ret            = RSEMIB_OK;

  if(NULL == sysDesc){
      ret = RSEMIB_BAD_INPUT;
  } else {
      DEBUGMSGTL((MY_NAME, "commit_to_disk:fopen(%s).\n",SYS_DESC_PATHFILENAME));
      if ((file_out = fopen(SYS_DESC_PATHFILENAME, "wb")) == NULL){
          DEBUGMSGTL((MY_NAME, "commit_to_disk: fopen fail.\n"));
          ret = RSEMIB_FOPEN_FAIL;
      } else {
          sysDesc->crc = (uint32_t)crc16((const uint8_t *)sysDesc, sizeof(SystemDescription_t) - 4);
          mib_byte_count = sizeof(SystemDescription_t);
          write_count = fwrite((uint8_t *)sysDesc, mib_byte_count, 1, file_out);
          if (write_count == 1){
              DEBUGMSGTL((MY_NAME, "commit_to_disk: SUCCESS: mib_byte_count=(%u:0x%x).\n",mib_byte_count,sysDesc->crc));
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
/* 
 * Below is thread support for firmware install.
 */

#define  MAX_LOCK_WAIT_ITERATIONS 60 /* How many times installFirmware() will try to lock MIB before giving up. */

static int32_t DateAndTime_To_UTC_DSEC(uint8_t * DateAndTime, uint64_t * utcDsec)
{
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
      return RSEMIB_BAD_DATA;
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

  if((year < 0) || (65536 < year)){ /* Basic range check: */
      DEBUGMSGTL((MY_NAME, "DateAndTime_To_UTC_DSEC: Bad year.\n"));
      return RSEMIB_BAD_DATA;       /* From unsigned above so can't be less than zero really. */
  }
  year = year - 1900; /* adjust for mktime(). */
  if((month < 1) || (12 < month)){ /* Not exhaustive for month & day. See below for more. */
      DEBUGMSGTL((MY_NAME, "DateAndTime_To_UTC_DSEC: Bad month.\n"));
      return RSEMIB_BAD_DATA;
  }
  month = month - 1; /* adjust for mktime(). */
  if((day < 1) || (31 < day)){
      DEBUGMSGTL((MY_NAME, "DateAndTime_To_UTC_DSEC: Bad day.\n"));
      return RSEMIB_BAD_DATA;
  }
  if((hour < 0) || (23 < hour)){
      DEBUGMSGTL((MY_NAME, "DateAndTime_To_UTC_DSEC: Bad hour.\n"));
      return RSEMIB_BAD_DATA;
  }
  if((minute < 0) || (59 < minute)){
      DEBUGMSGTL((MY_NAME, "DateAndTime_To_UTC_DSEC: Bad minute.\n"));
      return RSEMIB_BAD_DATA;
  }
  if((second < 0) || (60 < second)){
      DEBUGMSGTL((MY_NAME, "DateAndTime_To_UTC_DSEC: Bad second.\n"));
      return RSEMIB_BAD_DATA;
  }
  if((dsec < 0) || (9 < dsec)){
      DEBUGMSGTL((MY_NAME, "DateAndTime_To_UTC_DSEC: Bad dsec.\n"));
      return RSEMIB_BAD_DATA;
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
      return RSEMIB_BAD_DATA;
  }
  testDsec *= (uint64_t)10;   /* Convert to deci-seconds and add final value. */
  testDsec += (uint64_t)dsec;
  *utcDsec = testDsec;
  return RSEMIB_OK;
}
static uint64_t rseUtilGetTimeInMs(void)
{
  struct timeval tv;
  uint64_t timenow;

  gettimeofday(&tv, NULL);
  timenow = tv.tv_sec;
  timenow *= 1000;
  timenow += tv.tv_usec/1000;
  return timenow;
}
static void rseUtilGetUTCTime(struct tm *clock)
{
  time_t    val;
  struct tm t, *tmptr;

  if (NULL == clock) return;
   
  val = time(NULL);
  tmptr = gmtime_r(&val, &t);

  clock->tm_year  = (1900 + tmptr->tm_year);
  clock->tm_mon   =  (1 + tmptr->tm_mon);
  clock->tm_mday  = tmptr->tm_mday;
  clock->tm_hour  = tmptr->tm_hour;
  clock->tm_min   = tmptr->tm_min;
  clock->tm_sec   = tmptr->tm_sec;
  clock->tm_yday  = tmptr->tm_yday;
  clock->tm_wday  = tmptr->tm_wday;
}
/* Assumes MIB is locked. RO status updates like below go to MIB only. They dont go to disk. */
STATIC int32_t timeToInstallFirmware(void)
{
  int32_t  ret = RSEMIB_OK;
  uint64_t ScheduledInstallTime_UTC_dsec = 0; /* UTC Dsec */
  uint64_t now_UTC_dsec = 0;
  int32_t  i;

  /*
   * If (rsuScheduledInstallTime == 0) then install update now. No time check needed.
   * If (0 < rsuScheduledInstallTime <= now) then install upgrade, else return not ready.
   */
  for(i=0;i<MIB_DATEANDTIME_LENGTH;i++) {
      if(0x0 != ntcip_1218_mib.rsuScheduledInstallTime[i]) {
          break; /* If non-zero then eval time time. */
      }
  }
  if(i < MIB_DATEANDTIME_LENGTH) {
      ret = DateAndTime_To_UTC_DSEC(ntcip_1218_mib.rsuScheduledInstallTime,&ScheduledInstallTime_UTC_dsec);
      if(RSEMIB_OK != ret) {
          DEBUGMSGTL((MY_NAME, "timeToInstallFirmware: Bad start date.\n"));
          ntcip_1218_mib.rsuInstallUpdate = 0; /* Rejected */
          ntcip_1218_mib.rsuInstallStatus = installRejected;
          strncpy((char_t *)ntcip_1218_mib.rsuInstallStatusMessage,
              "timeToInstallFirmware: Bad start date.", RSU_INSTALL_STATUS_MSG_LENGTH_MAX);
          ntcip_1218_mib.rsuInstallStatusMessage_length = 
              strnlen("timeToInstallFirmware: Bad start date.",RSU_INSTALL_STATUS_MSG_LENGTH_MAX);
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"timeToInstallFirmware: Bad start date.");
          ret = RSEMIB_BAD_DATA;
      }
      if(RSEMIB_OK == ret){
          now_UTC_dsec = (uint64_t)(rseUtilGetTimeInMs()/(uint64_t)100);
          if(ScheduledInstallTime_UTC_dsec <= now_UTC_dsec) {
              DEBUGMSGTL((MY_NAME,"timeToInstallFirmware:: Time has arrived: start(0x%lx) <= now(0x%lx)\n",ScheduledInstallTime_UTC_dsec,now_UTC_dsec));
              I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"timeToInstallFirmware:: Time has arrived: start(0x%lx) <= now(0x%lx)\n",ScheduledInstallTime_UTC_dsec,now_UTC_dsec);
              ret = RSEMIB_OK;
          } else {
              DEBUGMSGTL((MY_NAME,"timeToInstallFirmware:: Not time to install yet now(0x%lx) < start(0x%lx)\n",now_UTC_dsec,ScheduledInstallTime_UTC_dsec));
              ntcip_1218_mib.rsuInstallStatus = installOther; /* Not Ready */
              strncpy((char_t *)ntcip_1218_mib.rsuInstallStatusMessage,
                       "timeToInstallFirmware: Not time to install yet.", RSU_INSTALL_STATUS_MSG_LENGTH_MAX);
              ntcip_1218_mib.rsuInstallStatusMessage_length = 
                       strnlen("timeToInstallFirmware: Not time to install yet.",RSU_INSTALL_STATUS_MSG_LENGTH_MAX);
              ret = RSEMIB_FIRMWARE_NOT_READY;
          }
      }
  }
  return ret;
}
/* 
 * Look for "NOR flashing complete...Reboot system to activate new firmware." in /tmp/installUpdate.txt
 * Future: capture some basic errors and preseve for user in MIB. Today just pass or fail.
 */
STATIC int32_t VerifyInstallUpdate(void)
{
  int32_t ret = SYS_DESC_VERIFY_UPDATE_FAIL; /* Fail till proven success. */
  FILE *  fp;
  char_t  reply_buf[128];
  int32_t i;

  memset(reply_buf,'\0',sizeof(reply_buf));
  DEBUGMSGTL((MY_NAME, "VerifyInstallUpdate: SYS_DESC_INSTALLUPDATE_RESULT[%s].\n",SYS_DESC_INSTALLUPDATE_RESULT));
  if(NULL != (fp = popen(SYS_DESC_INSTALLUPDATE_RESULT, "r"))) { /* Use cat to search nor update output for our success string. */
      if(NULL != fgets(reply_buf, sizeof(reply_buf), fp)){ /* cat returned something. */
          DEBUGMSGTL((MY_NAME, "VerifyInstallUpdate: SYS_DESC_NOR_SUCCESS[%s].\n",SYS_DESC_NOR_SUCCESS));
          i = strnlen(reply_buf,sizeof(reply_buf));
          reply_buf[i-1] = '\0'; /* Avoid extra char at end of reply_buf. */
          if (0 == strncmp(reply_buf,SYS_DESC_NOR_SUCCESS, sizeof(SYS_DESC_NOR_SUCCESS)) ){ /* Verify what cat returned is what we want. */
              DEBUGMSGTL((MY_NAME, "VerifyInstallUpdate: Success.\n"));
              ret = RSEMIB_OK; /* Success. */
          } else {
              DEBUGMSGTL((MY_NAME, "VerifyInstallUpdate: Not done yet, reply[%s].\n",reply_buf));
          }
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"VerifyInstallUpdate: reply[%s].\n",reply_buf);
      } else { /* cat found no match. */
          DEBUGMSGTL((MY_NAME, "VerifyInstallUpdate: Not done yet.\n"));
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"VerifyInstallUpdate: Not done yet.\n");
      }
      pclose(fp);
  } else {
      DEBUGMSGTL((MY_NAME, "VerifyInstallUpdate: popen failed ret.\n"));
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"VerifyInstallUpdate: popen failed ret.\n");
      ret = RSEMIB_SYSCALL_FAIL;
  }
  return ret;
}
static int32_t GET_LOCK(void) 
{
  int32_t i;  
  for (i=0; i<MAX_LOCK_WAIT_ITERATIONS; i++) {
      if(RSEMIB_OK == ntcip1218_lock()){
          return RSEMIB_OK;
      } else {
          DEBUGMSGTL((MY_NAME, " GET_LOCK waiting %d seconds.\n",i));
          sleep(1);
      }
  }
  DEBUGMSGTL((MY_NAME, " GET_LOCK failed, waited %d seconds.\n",i));
  return RSEMIB_LOCK_FAIL;
}
void installFirmware(void)
{
  int32_t ret = RSEMIB_OK;
  int32_t i;
  FILE *fp;
  char_t command_buffer[100 + RSU_MSG_FILENAME_LENGTH_MAX]; /* If you need more than a 100 chars for your command then add them here. */
  struct tm clock;
  uint8_t  now[MIB_DATEANDTIME_LENGTH];
  SystemDescription_t sysDesc;

  if(RSEMIB_OK == ntcip1218_lock()){ /* If locked out, don't worry we'll be called again soon. */

      /* 
       * There is no way for user to make another request till this is finished. Dupe action not possible.
       * If this thread gets stuck the MIB is ok and user can't request another firmware update.
       */
      if((1 == ntcip_1218_mib.rsuInstallUpdate) && (RSEMIB_OK == (ret = timeToInstallFirmware()))) { 

          ntcip_1218_mib.rsuInstallUpdate = 0;      /* Clear request once. Whatever happens, we have serviced this request. */
          memset(ntcip_1218_mib.rsuScheduledInstallTime,0x0,MIB_DATEANDTIME_LENGTH); /* Clear start time. */
          memset(&clock, 0, sizeof(clock));
          rseUtilGetUTCTime(&clock); /* Timestamp once so its unique. */

          now[0] = (uint8_t)((0xff00 & clock.tm_year) >> 8);
          now[1] = (uint8_t) (0x00ff & clock.tm_year);
          now[2] = clock.tm_mon;
          now[3] = clock.tm_mday;
          now[4] = clock.tm_hour;
          now[5] = clock.tm_min;
          now[6] = clock.tm_sec;
          now[7] = 0;
          
          DEBUGMSGTL((MY_NAME, "installFirmware:now[%u %u %u %u %u %u %u %u]\n",now[0],now[1],now[2],now[3],now[4],now[5],now[6],now[7]));
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"installFirmware:now[%u %u %u %u %u %u %u %u]\n",now[0],now[1],now[2],now[3],now[4],now[5],now[6],now[7]);
          memcpy(ntcip_1218_mib.rsuInstallTime, now, MIB_DATEANDTIME_LENGTH); /* Copy unique Timestamp in for InstallTime. */
          ntcip_1218_mib.rsuInstallStatus = installProcessing; /* Tell client we are processing. */
          strncpy((char_t *)ntcip_1218_mib.rsuInstallStatusMessage,"installFirmware: installProcessing.", RSU_INSTALL_STATUS_MSG_LENGTH_MAX);
          ntcip_1218_mib.rsuInstallStatusMessage_length = strnlen( "installFirmware: installProcessing.", RSU_INSTALL_STATUS_MSG_LENGTH_MAX);

          /* 
           * Update on disk to avoid loop. If this fails then we'll be back here on next reset.
           * User can delete SYS_DESC_PATHFILENAME to clear request. Must wait for this function to timeout/exit first then delete.
           */
          if(RSEMIB_OK == (copy_sysDesc_to_skinny(&ntcip_1218_mib, &sysDesc))){
              if(RSEMIB_OK != commit_sysDesc_to_disk(&sysDesc)) {
                  DEBUGMSGTL((MY_NAME, "installFirmware: commit_sysDesc_to_disk failed.\n"));
                  I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"installFirmware: commit_sysDesc_to_disk failed.\n");
                  strncpy((char_t *)ntcip_1218_mib.rsuInstallStatusMessage,
                      "installFirmware: commit_sysDesc_to_disk failed.", RSU_INSTALL_STATUS_MSG_LENGTH_MAX);
                  ntcip_1218_mib.rsuInstallStatusMessage_length = 
                       strnlen("installFirmware: commit_sysDesc_to_disk failed.",RSU_INSTALL_STATUS_MSG_LENGTH_MAX);
                  ret = RSEMIB_SYSCALL_FAIL;
              }
          } else {
              DEBUGMSGTL((MY_NAME, "installFirmware: copy_sysDesc_to_skinny failed.\n"));
              I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"installFirmware: copy_sysDesc_to_skinny failed.\n");
              strncpy((char_t *)ntcip_1218_mib.rsuInstallStatusMessage,
                  "installFirmware: copy_sysDesc_to_skinnyfailed.", RSU_INSTALL_STATUS_MSG_LENGTH_MAX);
              ntcip_1218_mib.rsuInstallStatusMessage_length = 
                   strnlen("installFirmware: copy_sysDesc_to_skinny failed.",RSU_INSTALL_STATUS_MSG_LENGTH_MAX);
              ret = RSEMIB_SYSCALL_FAIL;
          }

          ntcip1218_unlock(); /* Free lock on MIB for now. */

          if(RSEMIB_OK == ret) {
              memset(command_buffer,'\0',sizeof(command_buffer));
              /* Create Install Update command. */
              snprintf(command_buffer, sizeof(command_buffer), "%s %s/%s > %s", SYS_DESC_NOR_CMD, (char_t *)sysDesc.rsuInstallPath
                  , (char_t *)sysDesc.rsuInstallFile, SYS_DESC_INSTALLUPDATE_OUTPUT);

              DEBUGMSGTL((MY_NAME, "rsuInstallFirmware: cmd=[%s]\n",command_buffer));
              I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"rsuInstallFirmware: cmd=[%s]\n",command_buffer);
              if(NULL != (fp = popen(command_buffer, "r"))) {

                  /* Check densonor output peridocally for complete. 15 minutes then give up. */
                  for(i=0, ret = SYS_DESC_VERIFY_UPDATE_FAIL; (i < SYS_DESC_INSTALLUPDATE_TIMEOUT) && (SYS_DESC_VERIFY_UPDATE_FAIL == ret); i++) {
                      sleep(10);
                      DEBUGMSGTL((MY_NAME, "installFirmware: Wait for Update to complete, seconds passed = %d.\n", (i+1) * 10));
                      ret = VerifyInstallUpdate();
                  }

                  if(RSEMIB_OK == ret) {
                      sysDesc.rsuInstallStatus = installSuccess;  /* Success. */
                      strncpy((char_t *)sysDesc.rsuInstallStatusMessage,"installFirmware: Success.", RSU_INSTALL_STATUS_MSG_LENGTH_MAX);
                      sysDesc.rsuInstallStatusMessage_length = strnlen("installFirmware: Success.",RSU_INSTALL_STATUS_MSG_LENGTH_MAX);

                      /* 
                       * No auto-reboot. WEB GUI forces user to reboot. User must OID 5.17.4 rsuReboot to apply update.  
                       */

                  } else { /* VerifyInstallUpdate failed. */
                      DEBUGMSGTL((MY_NAME, "installFirmware: VerifyInstallUpdate failed.\n"));
                      I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"installFirmware: VerifyInstallUpdate failed: installRejected.\n");
                      sysDesc.rsuInstallStatus = installRejected;
                      strncpy((char_t *)sysDesc.rsuInstallStatusMessage,"installFirmware: VerifyInstallUpdate failed.", RSU_INSTALL_STATUS_MSG_LENGTH_MAX);
                      sysDesc.rsuInstallStatusMessage_length = strnlen("installFirmware: VerifyInstallUpdate failed.",RSU_INSTALL_STATUS_MSG_LENGTH_MAX);
                      ret = RSEMIB_BAD_DATA;
                  }
                  if(-1 == pclose(fp)) { /* Courtesy, wait for densonor to finish. In theory, based on above, we are done. */
                      DEBUGMSGTL((MY_NAME, "installFirmware: pclose failed: errno=%d(%s)\n",errno,strerror(errno))); /* Is there anybody out there? */
                      I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"installFirmware: pclose failed: errno=%d(%s)\n",errno,strerror(errno));
                  }
              } else {
                  DEBUGMSGTL((MY_NAME, "installFirmware: popen failed.\n"));
                  I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"installFirmware: popen failed.\n");
                  strncpy((char_t *)sysDesc.rsuInstallStatusMessage,"installFirmware: popen failed.", RSU_INSTALL_STATUS_MSG_LENGTH_MAX);
                  sysDesc.rsuInstallStatusMessage_length = strnlen( "installFirmware: popen failed.", RSU_INSTALL_STATUS_MSG_LENGTH_MAX);
                  sysDesc.rsuInstallStatus = installRejected;  
                  ret = RSEMIB_SYSCALL_FAIL;
              } /* popen() */

              if(ret < RSEMIB_OK) {
                  set_error_states(ret);
              }
              /* Update MIB and write to disk on exit. */
              if(RSEMIB_OK == (ret = GET_LOCK())){

                  ntcip_1218_mib.rsuInstallStatus = sysDesc.rsuInstallStatus;
                  strncpy((char_t *)ntcip_1218_mib.rsuInstallStatusMessage,(char_t *)sysDesc.rsuInstallStatusMessage, RSU_INSTALL_STATUS_MSG_LENGTH_MAX);
                  ntcip_1218_mib.rsuInstallStatusMessage_length = sysDesc.rsuInstallStatusMessage_length;

                  /* 
                   * If write to disk fails that doesn't change the status of the install update. 
                   * Status on disk is bogus if user cares. We'll see it across reset as not complete.
                   */
                  if(RSEMIB_OK != commit_sysDesc_to_disk(&sysDesc)) {
                      DEBUGMSGTL((MY_NAME, "installFirmware: final commit_sysDesc_to_disk failed.\n"));
                      I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"installFirmware: final commit_sysDesc_to_disk failed.\n");
                      ret = RSEMIB_SYSCALL_FAIL;
                  }
              } else {
                  /* If this fails then both the MIB and disk contain bogus status. System problem at this point. */
                  sysDesc.rsuInstallStatusMessage[RSU_INSTALL_STATUS_MSG_LENGTH_MAX-1] = '\0';
                  DEBUGMSGTL((MY_NAME, "installFirmware: MIB locked indefinately. Can't update install status=%d[%s]\n",
                      sysDesc.rsuInstallStatus, (char_t *)sysDesc.rsuInstallStatusMessage));
                  I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"installFirmware: MIB locked indefinately. Can't update install status=%d[%s]\n",
                      sysDesc.rsuInstallStatus, (char_t *)sysDesc.rsuInstallStatusMessage);
              }
          }
      }
      if(RSEMIB_LOCK_FAIL != ret) { /* If we had a lock fail then nothing to unlock, in theory. */
          ntcip1218_unlock();
      }
  } else {
      ret = RSEMIB_LOCK_FAIL;
  }
  if(ret < RSEMIB_OK) {
      set_error_states(ret);
  }
}

