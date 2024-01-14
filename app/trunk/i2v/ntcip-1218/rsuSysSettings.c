/**************************************************************************
 *                                                                        *
 *     File Name:  rsuSysSettings.c                                       *
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
#include "rsuhealth.h"
#if defined(MY_UNIT_TEST)
#include "stubs.h"
#endif

/*
 * Defines.
 */

/* Mandatory: For SNMP debugging and output. */
#define MY_NAME       "SysSetts"
#define MY_NAME_EXTRA "XSysSetts"
/* Mandatory: For debugging and syslog output. */
#define MY_I2V_NAME    MY_NAME
#define MY_ERR_LEVEL   LEVEL_PRIV    /* from i2v_util.h */

/******************************************************************************
 * 5.15 rsuSysSettings: {rsu 14}
 * 
 *     uint8_t         rsuNotifyIpAddress[RSU_DEST_IP_MAX];
 *     int32_t         rsuNotifyPort;
 *     uint8_t         rsuSysLogQueryStart[RFC2579_DATEANDTIME_LENGTH];
 *     uint8_t         rsuSysLogQueryStop[RFC2579_DATEANDTIME_LENGTH];
 *     syslog_level_e  rsuSysLogQueryPriority;
 *     int32_t         rsuSysLogQueryGenerate;
 *     syslog_status_e rsuSysLogQueryStatus;
 *     int32_t         rsuSysLogCloseCommand;
 *     syslog_level_e  rsuSysLogSeverity;
 *     uint8_t         rsuSysConfigId[RSU_SYS_CONFIG_ID_LENGTH_MAX];
 *     int32_t         rsuSysRetries;
 *     int32_t         rsuSysRetryPeriod;
 *     int32_t         rsuShortCommLossTime;
 *     int32_t         rsuLongCommLossTime;
 *     uint8_t         rsuSysLogName[RSU_SYSLOG_NAME_AND_PATH_LENGTH_MAX];
 *     uint8_t         rsuSysDir[RSU_SYSDIR_NAME_LENGTH_MAX];
 *     int32_t         rsuLongCommLossReboot;
 *     uint8_t         rsuHostIpAddr[RSU_DEST_IP_MAX];
 *     uint8_t         rsuHostNetMask[RSU_DEST_IP_MAX];
 *     uint8_t         rsuHostGateway[RSU_DEST_IP_MAX];
 *     uint8_t         rsuHostDNS[RSU_DEST_IP_MAX];
 *     uint32_t        rsuHostDHCPEnable;
 *
 ******************************************************************************/

const oid rsuNotifyIpAddress_oid[]     = {1, 3, 6, 1, 4, 1, 1206, 4, 2, 18, 14 , 1};
const oid rsuNotifyPort_oid[]          = {1, 3, 6, 1, 4, 1, 1206, 4, 2, 18, 14 , 2};
const oid rsuSysLogQueryStart_oid[]    = {1, 3, 6, 1, 4, 1, 1206, 4, 2, 18, 14 , 3};
const oid rsuSysLogQueryStop_oid[]     = {1, 3, 6, 1, 4, 1, 1206, 4, 2, 18, 14 , 4};
const oid rsuSysLogQueryPriority_oid[] = {1, 3, 6, 1, 4, 1, 1206, 4, 2, 18, 14 , 5};
const oid rsuSysLogQueryGenerate_oid[] = {1, 3, 6, 1, 4, 1, 1206, 4, 2, 18, 14 , 6};
const oid rsuSysLogQueryStatus_oid[]   = {1, 3, 6, 1, 4, 1, 1206, 4, 2, 18, 14 , 7};
const oid rsuSysLogCloseCommand_oid[]  = {1, 3, 6, 1, 4, 1, 1206, 4, 2, 18, 14 , 8};
const oid rsuSysLogSeverity_oid[]      = {1, 3, 6, 1, 4, 1, 1206, 4, 2, 18, 14 , 9};
const oid rsuSysConfigId_oid[]         = {1, 3, 6, 1, 4, 1, 1206, 4, 2, 18, 14 , 10};
const oid rsuSysRetries_oid[]          = {1, 3, 6, 1, 4, 1, 1206, 4, 2, 18, 14 , 11};
const oid rsuSysRetryPeriod_oid[]      = {1, 3, 6, 1, 4, 1, 1206, 4, 2, 18, 14 , 12};
const oid rsuShortCommLossTime_oid[]   = {1, 3, 6, 1, 4, 1, 1206, 4, 2, 18, 14 , 13};
const oid rsuLongCommLossTime_oid[]    = {1, 3, 6, 1, 4, 1, 1206, 4, 2, 18, 14 , 14};
const oid rsuSysLogName_oid[]          = {1, 3, 6, 1, 4, 1, 1206, 4, 2, 18, 14 , 15};
const oid rsuSysDir_oid[]              = {1, 3, 6, 1, 4, 1, 1206, 4, 2, 18, 14 , 16};
const oid rsuLongCommLossReboot_oid[]  = {1, 3, 6, 1, 4, 1, 1206, 4, 2, 18, 14 , 17};
const oid rsuHostIpAddr_oid[]          = {1, 3, 6, 1, 4, 1, 1206, 4, 2, 18, 14 , 18};
const oid rsuHostNetMask_oid[]         = {1, 3, 6, 1, 4, 1, 1206, 4, 2, 18, 14 , 19};
const oid rsuHostGateway_oid[]         = {1, 3, 6, 1, 4, 1, 1206, 4, 2, 18, 14 , 20};
const oid rsuHostDNS_oid[]             = {1, 3, 6, 1, 4, 1, 1206, 4, 2, 18, 14 , 21};
const oid rsuHostDHCPEnable_oid[]      = {1, 3, 6, 1, 4, 1, 1206, 4, 2, 18, 14 , 22};

extern int32_t  mainloop; /* For thread handling. */
extern ntcip_1218_mib_t  ntcip_1218_mib;

static uint32_t rsuSysSettings_error_states = 0x0;
static sysSetts_t * shm_sysSetts_ptr        = NULL; /* SHM. */

/* 
 * Function Bodies.
 */
static void set_error_states(int32_t error)
{
  /* -1 to -32 */
  if((error < 0) && (error > MIB_ERROR_BASE_2)){
      rsuSysSettings_error_states |= (uint32_t)(0x1) << (abs(error) - 1);
  }
}
uint32_t get_rsuSysSettings_error_states(void)
{
  return rsuSysSettings_error_states;
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
 * OID Handlers
 */

/* 5.15.1 */
int32_t handle_rsuNotifyIpAddress(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo,
                                 netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret = RSEMIB_OK;
  uint8_t data[RSU_DEST_IP_MAX];

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
              if(0 <= (ret = get_rsuNotifyIpAddress(data))){
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
              if(RSEMIB_OK == (ret = preserve_rsuNotifyIpAddress())){
                  ret = action_rsuNotifyIpAddress(requests->requestvb->val.string,requests->requestvb->val_len);
              }
              break;
          case MODE_SET_COMMIT:
              DEBUGMSGTL((MY_NAME, "COMMIT:\n"));
              if(RSEMIB_OK == (ret = commit_rsuNotifyIpAddress())){
                  /*
                   * Back end magic here.
                   */
              }
              break;
          case MODE_SET_UNDO:
              DEBUGMSGTL((MY_NAME, "UNDO:\n"));
              ret = undo_rsuNotifyIpAddress();
              break;
          default:
              ret = RSEMIB_BAD_MODE;
              break;
      }
  }
  if(ret < RSEMIB_OK) {
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"%s: failed: ret=%d.\n",__FUNCTION__, ret);
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
/* 5.15.2 */
int32_t handle_rsuNotifyPort(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo,
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
              if(0 <= (ret = get_rsuNotifyPort())){
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
              if(RSEMIB_OK == (ret = preserve_rsuNotifyPort())){
                  ret = action_rsuNotifyPort( *(requests->requestvb->val.integer));
              }
              break;
          case MODE_SET_COMMIT:
              DEBUGMSGTL((MY_NAME, "COMMIT:\n"));
              if(RSEMIB_OK == (ret = commit_rsuNotifyPort())){
                  /*
                   * Back end magic here.
                   */
              }
              break;
          case MODE_SET_UNDO:
              DEBUGMSGTL((MY_NAME, "UNDO:\n"));
              ret = undo_rsuNotifyPort();
              break;
          default:
              ret = RSEMIB_BAD_MODE;
              break;
      }
  }
  if(ret < RSEMIB_OK) {
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"%s: failed: ret=%d.\n",__FUNCTION__, ret);
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
/* 5.15.3 */
int32_t handle_rsuSysLogQueryStart(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo,
                                 netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret = RSEMIB_OK;
  uint8_t data[RFC2579_DATEANDTIME_LENGTH];

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
              if(RSEMIB_OK == (ret = get_rsuSysLogQueryStart(data))){
                  snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR, data, RFC2579_DATEANDTIME_LENGTH);
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
              if(RSEMIB_OK == (ret = preserve_rsuSysLogQueryStart())){
                  ret = action_rsuSysLogQueryStart(requests->requestvb->val.string, requests->requestvb->val_len);
              }
              break;
          case MODE_SET_COMMIT:
              DEBUGMSGTL((MY_NAME, "COMMIT:\n"));
              if(RSEMIB_OK == (ret = commit_rsuSysLogQueryStart())){
                  /*
                   * Back end magic here.
                   */
              }
              break;
          case MODE_SET_UNDO:
              DEBUGMSGTL((MY_NAME, "UNDO:\n"));
              ret = undo_rsuSysLogQueryStart();
              break;
          default:
              ret = RSEMIB_BAD_MODE;
              break;
      }
  }
  if(ret < RSEMIB_OK) {
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"%s: failed: ret=%d.\n",__FUNCTION__, ret);
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
/* 5.15.4 */
int32_t handle_rsuSysLogQueryStop(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo,
                                 netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret = RSEMIB_OK;
  uint8_t data[RFC2579_DATEANDTIME_LENGTH];

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
              if(RSEMIB_OK == (ret = get_rsuSysLogQueryStop(data))){
                  snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR, data, RFC2579_DATEANDTIME_LENGTH);
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
              if(RSEMIB_OK == (ret = preserve_rsuSysLogQueryStop())){
                  ret = action_rsuSysLogQueryStop(requests->requestvb->val.string,requests->requestvb->val_len);
              }
              break;
          case MODE_SET_COMMIT:
              DEBUGMSGTL((MY_NAME, "COMMIT:\n"));
              if(RSEMIB_OK == (ret = commit_rsuSysLogQueryStop())){
                  /*
                   * Back end magic here.
                   */
              }
              break;
          case MODE_SET_UNDO:
              DEBUGMSGTL((MY_NAME, "UNDO:\n"));
              ret = undo_rsuSysLogQueryStop();
              break;
          default:
              ret = RSEMIB_BAD_MODE;
              break;
      }
  }
  if(ret < RSEMIB_OK) {
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"%s: failed: ret=%d.\n",__FUNCTION__, ret);
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
/* 5.15.5 */
int32_t handle_rsuSysLogQueryPriority(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo,
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
              if(0 <= (ret = get_rsuSysLogQueryPriority())){
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
              if(RSEMIB_OK == (ret = preserve_rsuSysLogQueryPriority())){
                  ret = action_rsuSysLogQueryPriority( *(requests->requestvb->val.integer));
              }
              break;
          case MODE_SET_COMMIT:
              DEBUGMSGTL((MY_NAME, "COMMIT:\n"));
              if(RSEMIB_OK == (ret = commit_rsuSysLogQueryPriority())){
                  /*
                   * Back end magic here.
                   */
              }
              break;
          case MODE_SET_UNDO:
              DEBUGMSGTL((MY_NAME, "UNDO:\n"));
              ret = undo_rsuSysLogQueryPriority();
              break;
          default:
              ret = RSEMIB_BAD_MODE;
              break;
      }
  }
  if(ret < RSEMIB_OK) {
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"%s: failed: ret=%d.\n",__FUNCTION__, ret);
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
/* 5.15.6 */
int32_t handle_rsuSysLogQueryGenerate(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo,
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
              if(0 <= (ret = get_rsuSysLogQueryGenerate())){
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
              if(RSEMIB_OK == (ret = preserve_rsuSysLogQueryGenerate())){
                  ret = action_rsuSysLogQueryGenerate( *(requests->requestvb->val.integer));
              }
              break;
          case MODE_SET_COMMIT:
              DEBUGMSGTL((MY_NAME, "COMMIT:\n"));
              if(RSEMIB_OK == (ret = commit_rsuSysLogQueryGenerate())){
                  /*
                   * Back end magic here.
                   */
              }
              break;
          case MODE_SET_UNDO:
              DEBUGMSGTL((MY_NAME, "UNDO:\n"));
              ret = undo_rsuSysLogQueryGenerate();
              break;
          default:
              ret = RSEMIB_BAD_MODE;
              break;
      }
  }
  if(ret < RSEMIB_OK) {
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"%s: failed: ret=%d.\n",__FUNCTION__, ret);
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
/* 5.15.7 */
int32_t handle_rsuSysLogQueryStatus(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo,
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
              if(0 <= (ret = get_rsuSysLogQueryStatus())){
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
              if(RSEMIB_OK == (ret = preserve_rsuSysLogQueryStatus())){
                  ret = action_rsuSysLogQueryStatus( *(requests->requestvb->val.integer));
              }
              break;
          case MODE_SET_COMMIT:
              DEBUGMSGTL((MY_NAME, "COMMIT:\n"));
              if(RSEMIB_OK == (ret = commit_rsuSysLogQueryStatus())){
                  /* 
                   * Back end magic here.
                   */
              }
              break;
          case MODE_SET_UNDO:
              DEBUGMSGTL((MY_NAME, "UNDO:\n"));
              ret = undo_rsuSysLogQueryStatus();
              break;
          default:
              ret = RSEMIB_BAD_MODE;
              break;
      }
  }
  if(ret < RSEMIB_OK) {
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"%s: failed: ret=%d.\n",__FUNCTION__, ret);
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
/* 5.15.8 */
int32_t handle_rsuSysLogCloseCommand(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo,
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
              if(0 <= (ret = get_rsuSysLogCloseCommand())){
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
              if(RSEMIB_OK == (ret = preserve_rsuSysLogCloseCommand())){
                  ret = action_rsuSysLogCloseCommand( *(requests->requestvb->val.integer));
              }
              break;
          case MODE_SET_COMMIT:
              DEBUGMSGTL((MY_NAME, "COMMIT:\n"));
              if(RSEMIB_OK == (ret = commit_rsuSysLogCloseCommand())){
                  /*
                   * Back end magic here.
                   */
              }
              break;
          case MODE_SET_UNDO:
              DEBUGMSGTL((MY_NAME, "UNDO:\n"));
              ret = undo_rsuSysLogCloseCommand();
              break;
          default:
              ret = RSEMIB_BAD_MODE;
              break;
      }
  }
  if(ret < RSEMIB_OK) {
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"%s failed: ret=%d.\n", __FUNCTION__, ret);
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
/* 5.15.9 */
int32_t handle_rsuSysLogSeverity(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo,
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
              if(0 <= (ret = get_rsuSysLogSeverity())){
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
              if(RSEMIB_OK == (ret = preserve_rsuSysLogSeverity())){
                  ret = action_rsuSysLogSeverity( *(requests->requestvb->val.integer));
              }
              break;
          case MODE_SET_COMMIT:
              DEBUGMSGTL((MY_NAME, "COMMIT:\n"));
              if(RSEMIB_OK == (ret = commit_rsuSysLogSeverity())){
                  /*
                   * Back end magic here.
                   */
              }
              break;
          case MODE_SET_UNDO:
              DEBUGMSGTL((MY_NAME, "UNDO:\n"));
              ret = undo_rsuSysLogSeverity();
              break;
          default:
              ret = RSEMIB_BAD_MODE;
              break;
      }
  }
  if(ret < RSEMIB_OK) {
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"%s: failed: ret=%d.\n",__FUNCTION__, ret);
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
/* 5.15.10 */
int32_t handle_rsuSysConfigId(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo,
                                 netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret = RSEMIB_OK;
  uint8_t data[RSU_SYS_CONFIG_ID_LENGTH_MAX];

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
              if(0 <= (ret = get_rsuSysConfigId(data))){
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
              if(RSEMIB_OK == (ret = preserve_rsuSysConfigId())){
                  ret = action_rsuSysConfigId(requests->requestvb->val.string, requests->requestvb->val_len);
              }
              break;
          case MODE_SET_COMMIT:
              DEBUGMSGTL((MY_NAME, "COMMIT:\n"));
              if(RSEMIB_OK == (ret = commit_rsuSysConfigId())){
                  /* 
                   * Back end magic here.
                   */
              }
              break;
          case MODE_SET_UNDO:
              DEBUGMSGTL((MY_NAME, "UNDO:\n"));
              ret = undo_rsuSysConfigId();
              break;
          default:
              ret = RSEMIB_BAD_MODE;
              break;
      }
  }
  if(ret < RSEMIB_OK) {
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"%s: failed: ret=%d.\n",__FUNCTION__, ret);
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
/* 5.15.11 */
int32_t handle_rsuSysRetries(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo,
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
              if(0 <= (ret = get_rsuSysRetries())){
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
              if(RSEMIB_OK == (ret = preserve_rsuSysRetries())){
                  ret = action_rsuSysRetries( *(requests->requestvb->val.integer));
              }
              break;
          case MODE_SET_COMMIT:
              DEBUGMSGTL((MY_NAME, "COMMIT:\n"));
              if(RSEMIB_OK == (ret = commit_rsuSysRetries())){
                  /*
                   * Back end magic here.
                   */
              }
              break;
          case MODE_SET_UNDO:
              DEBUGMSGTL((MY_NAME, "UNDO:\n"));
              ret = undo_rsuSysRetries();
              break;
          default:
              ret = RSEMIB_BAD_MODE;
              break;
      }
  }
  if(ret < RSEMIB_OK) {
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"%s: failed: ret=%d.\n",__FUNCTION__, ret);
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
/* 5.15.12 */
int32_t handle_rsuSysRetryPeriod(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo,
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
              if(0 <= (ret = get_rsuSysRetryPeriod())){
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
              if(RSEMIB_OK == (ret = preserve_rsuSysRetryPeriod())){
                  ret = action_rsuSysRetryPeriod( *(requests->requestvb->val.integer));
              }
              break;
          case MODE_SET_COMMIT:
              DEBUGMSGTL((MY_NAME, "COMMIT:\n"));
              if(RSEMIB_OK == (ret = commit_rsuSysRetryPeriod())){
                  /* 
                   * Back end magic here.
                   */
              }
              break;
          case MODE_SET_UNDO:
              DEBUGMSGTL((MY_NAME, "UNDO:\n"));
              ret = undo_rsuSysRetryPeriod();
              break;
          default:
              ret = RSEMIB_BAD_MODE;
              break;
      }
  }
  if(ret < RSEMIB_OK) {
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"%s: failed: ret=%d.\n",__FUNCTION__, ret);
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
/* 5.15.13 */ 
int32_t handle_rsuShortCommLossTime(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo,
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
              if(0 <= (ret = get_rsuShortCommLossTime())){
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
              if(RSEMIB_OK == (ret = preserve_rsuShortCommLossTime())){
                  ret = action_rsuShortCommLossTime( *(requests->requestvb->val.integer));
              }
              break;
          case MODE_SET_COMMIT:
              DEBUGMSGTL((MY_NAME, "COMMIT:\n"));
              if(RSEMIB_OK == (ret = commit_rsuShortCommLossTime())){
                  /*
                   * Back end magic here.
                   */
              }
              break;
          case MODE_SET_UNDO:
              DEBUGMSGTL((MY_NAME, "UNDO:\n"));
              ret = undo_rsuShortCommLossTime();
              break;
          default:
              ret = RSEMIB_BAD_MODE;
              break;
      }
  }
  if(ret < RSEMIB_OK) {
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"%s: failed: ret=%d.\n",__FUNCTION__, ret);
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
/* 5.15.14 */
int32_t handle_rsuLongCommLossTime(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo,
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
              if(0 <= (ret = get_rsuLongCommLossTime())){
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
              if(RSEMIB_OK == (ret = preserve_rsuLongCommLossTime())){
                  ret = action_rsuLongCommLossTime( *(requests->requestvb->val.integer));
              }
              break;
          case MODE_SET_COMMIT:
              DEBUGMSGTL((MY_NAME, "COMMIT:\n"));
              if(RSEMIB_OK == (ret = commit_rsuLongCommLossTime())){
                  /*
                   * Back end magic here.
                   */
              }
              break;
          case MODE_SET_UNDO:
              DEBUGMSGTL((MY_NAME, "UNDO:\n"));
              ret = undo_rsuLongCommLossTime();
              break;
          default:
              ret = RSEMIB_BAD_MODE;
              break;
      }
  }
  if(ret < RSEMIB_OK) {
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"AuthError failed: ret=%d.\n", ret);
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
/* 5.15.15 */
int32_t handle_rsuSysLogName(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo,
                                 netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret = RSEMIB_OK;
  uint8_t data[RSU_SYSLOG_NAME_AND_PATH_LENGTH_MAX];

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
              if(0 <= (ret = get_rsuSysLogName(data))){
                  snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR, data, ret);
                  ret = RSEMIB_OK;
              }
              break;
          default:
              ret = RSEMIB_BAD_MODE;
              break;
      }
  }
  if(ret < RSEMIB_OK) {
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"%s: failed: ret=%d.\n",__FUNCTION__, ret);
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
/* 5.15.16 */
int32_t handle_rsuSysDir(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo,
                                 netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret = RSEMIB_OK;
  uint8_t data[RSU_SYSDIR_NAME_LENGTH_MAX];

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
              if(0 <= (ret = get_rsuSysDir(data))){
                  snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR, data, ret);
                  ret = RSEMIB_OK;
                  data[RSU_SYSDIR_NAME_LENGTH_MAX-1] = '\0'; /* data used now so we can modify contents. */
                  DEBUGMSGTL((MY_NAME, "sysDir[%s]\n",data));
              }
              break;
          default:
              ret = RSEMIB_BAD_MODE;
              break;
      }
  }
  if(ret < RSEMIB_OK) {
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"%s: failed: ret=%d.\n",__FUNCTION__, ret);
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
/* 5.15.17 */
int32_t handle_rsuLongCommLossReboot(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo,
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
              if(0 <= (ret = get_rsuLongCommLossReboot())){
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
              if(RSEMIB_OK == (ret = preserve_rsuLongCommLossReboot())){
                  ret = action_rsuLongCommLossReboot( *(requests->requestvb->val.integer));
              }
              break;
          case MODE_SET_COMMIT:
              DEBUGMSGTL((MY_NAME, "COMMIT:\n"));
              if(RSEMIB_OK == (ret = commit_rsuLongCommLossReboot())){
                  /*
                   * Back end magic here.
                   */
              }
              break;
          case MODE_SET_UNDO:
              DEBUGMSGTL((MY_NAME, "UNDO:\n"));
              ret = undo_rsuLongCommLossReboot();
              break;
          default:
              ret = RSEMIB_BAD_MODE;
              break;
      }
  }
  if(ret < RSEMIB_OK) {
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"%s: failed: ret=%d.\n",__FUNCTION__, ret);
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
/* 5.15.18 */
int32_t handle_rsuHostIpAddr(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo,
                                 netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret = RSEMIB_OK;
  uint8_t data[RSU_DEST_IP_MAX];

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
              if(0 <= (ret = get_rsuHostIpAddr(data))){
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
              if(RSEMIB_OK == (ret = preserve_rsuHostIpAddr())){
                  ret = action_rsuHostIpAddr(requests->requestvb->val.string, requests->requestvb->val_len);
              }
              break;
          case MODE_SET_COMMIT:
              DEBUGMSGTL((MY_NAME, "COMMIT:\n"));
              if(RSEMIB_OK == (ret = commit_rsuHostIpAddr())){
                  /*
                   * Back end magic here.
                   */
              }
              break;
          case MODE_SET_UNDO:
              DEBUGMSGTL((MY_NAME, "UNDO:\n"));
              ret = undo_rsuHostIpAddr();
              break;
          default:
              ret = RSEMIB_BAD_MODE;
              break;
      }
  }
  if(ret < RSEMIB_OK) {
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"%s: failed: ret=%d.\n",__FUNCTION__, ret);
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
/* 5.15.19 */
int32_t handle_rsuHostNetMask(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo,
                                 netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret = RSEMIB_OK;
  uint8_t data[RSU_DEST_IP_MAX];

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
              if(0 <= (ret = get_rsuHostNetMask(data))){
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
              if(RSEMIB_OK == preserve_rsuHostNetMask()){
                  ret = action_rsuHostNetMask(requests->requestvb->val.string, requests->requestvb->val_len);
              }
              break;
          case MODE_SET_COMMIT:
              DEBUGMSGTL((MY_NAME, "COMMIT:\n"));
              if(RSEMIB_OK == (ret = commit_rsuHostNetMask())){
                  /* 
                   * Back end magic here.
                   */
              }
              break;
          case MODE_SET_UNDO:
              DEBUGMSGTL((MY_NAME, "UNDO:\n"));
              ret = undo_rsuHostNetMask();
              break;
          default:
              ret = RSEMIB_BAD_MODE;
              break;
      }
  }
  if(ret < RSEMIB_OK) {
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"%s: failed: ret=%d.\n",__FUNCTION__, ret);
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
/* 5.15.20  */
int32_t handle_rsuHostGateway(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo,
                                 netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret = RSEMIB_OK;
  uint8_t data[RSU_DEST_IP_MAX];

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
              if(0 <= (ret = get_rsuHostGateway(data))){
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
              if(RSEMIB_OK == (ret = preserve_rsuHostGateway())){
                  ret = action_rsuHostGateway(requests->requestvb->val.string, requests->requestvb->val_len);
              }
              break;
          case MODE_SET_COMMIT:
              DEBUGMSGTL((MY_NAME, "COMMIT:\n"));
              if(RSEMIB_OK == (ret = commit_rsuHostGateway())){
                  /* 
                   * Back end magic here.
                   */
              }
              break;
          case MODE_SET_UNDO:
              DEBUGMSGTL((MY_NAME, "UNDO:\n"));
              ret = undo_rsuHostGateway();
              break;
          default:
              ret = RSEMIB_BAD_MODE;
              break;
      }
  }
  if(ret < RSEMIB_OK) {
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"%s: failed: ret=%d.\n",__FUNCTION__, ret);
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
/* 5.15.21 */
int32_t handle_rsuHostDNS(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo,
                                 netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret = RSEMIB_OK;
  uint8_t data[RSU_DEST_IP_MAX];

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
              if(0 <= (ret = get_rsuHostDNS(data))){
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
              if(RSEMIB_OK == (ret = preserve_rsuHostDNS())){
                  ret = action_rsuHostDNS(requests->requestvb->val.string, requests->requestvb->val_len);
              }
              break;
          case MODE_SET_COMMIT:
              DEBUGMSGTL((MY_NAME, "COMMIT:\n"));
              if(RSEMIB_OK == (ret = commit_rsuHostDNS())){
                  /* 
                   * Back end magic here.
                   */
              }
              break;
          case MODE_SET_UNDO:
              DEBUGMSGTL((MY_NAME, "UNDO:\n"));
              ret = undo_rsuHostDNS();
              break;
          default:
              ret = RSEMIB_BAD_MODE;
              break;
      }
  }
  if(ret < RSEMIB_OK) {
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"%s: failed: ret=%d.\n",__FUNCTION__, ret);
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
/* 5.15.22 */
int32_t handle_rsuHostDHCPEnable(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo,
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
              if(0 <= (ret = get_rsuHostDHCPEnable ())){
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
              if(RSEMIB_OK == (ret = preserve_rsuHostDHCPEnable())){
                  ret = action_rsuHostDHCPEnable ( *(requests->requestvb->val.integer));
              }
              break;
          case MODE_SET_COMMIT:
              DEBUGMSGTL((MY_NAME, "COMMIT:\n"));
              if(RSEMIB_OK == (ret = commit_rsuHostDHCPEnable())){
                  /*
                   * Back end magic here.
                   */
              }
              break;
          case MODE_SET_UNDO:
              DEBUGMSGTL((MY_NAME, "UNDO:\n"));
              ret = undo_rsuHostDHCPEnable();
              break;
          default:
              ret = RSEMIB_BAD_MODE;
              break;
      }
  }
  if(ret < RSEMIB_OK) {
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"%s: failed: ret=%d.\n",__FUNCTION__, ret);
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

void install_rsuSysSettings_handlers(void)
{
  /* Clear local statics. */
  rsuSysSettings_error_states = 0x0;

  /* 5.15.1 */
  if(MIB_REGISTERED_OK != netsnmp_register_scalar(netsnmp_create_handler_registration
                          ("rsuNotifyIpAddress", 
                           handle_rsuNotifyIpAddress,
                           rsuNotifyIpAddress_oid,
                           OID_LENGTH(rsuNotifyIpAddress_oid),
                           HANDLER_CAN_RWRITE))){
      DEBUGMSGTL((MY_NAME, "rsuNotifyIpAddress install FAIL.\n"));
      set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
  }
  /* 5.15.2 */
  if(MIB_REGISTERED_OK != netsnmp_register_scalar(netsnmp_create_handler_registration
                          ("rsuNotifyPort", 
                           handle_rsuNotifyPort,
                           rsuNotifyPort_oid,
                           OID_LENGTH(rsuNotifyPort_oid),
                           HANDLER_CAN_RWRITE))){
      DEBUGMSGTL((MY_NAME, "rsuNotifyPort install FAIL.\n"));
      set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
  }
  /* 5.15.3 */
  if(MIB_REGISTERED_OK != netsnmp_register_scalar(netsnmp_create_handler_registration
                          ("rsuSysLogQueryStart", 
                           handle_rsuSysLogQueryStart,
                           rsuSysLogQueryStart_oid,
                           OID_LENGTH(rsuSysLogQueryStart_oid),
                           HANDLER_CAN_RWRITE))){
      DEBUGMSGTL((MY_NAME, "rsuSysLogQueryStart install FAIL.\n"));
      set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
  }
  /* 5.15.4 */
  if(MIB_REGISTERED_OK != netsnmp_register_scalar(netsnmp_create_handler_registration
                          ("rsuSysLogQueryStop", 
                           handle_rsuSysLogQueryStop,
                           rsuSysLogQueryStop_oid,
                           OID_LENGTH(rsuSysLogQueryStop_oid),
                           HANDLER_CAN_RWRITE))){
      DEBUGMSGTL((MY_NAME, "rsuSysLogQueryStop install FAIL.\n"));
      set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
  }
  /* 5.15.5 */
  if(MIB_REGISTERED_OK != netsnmp_register_scalar(netsnmp_create_handler_registration
                          ("rsuSysLogQueryPriority", 
                           handle_rsuSysLogQueryPriority,
                           rsuSysLogQueryPriority_oid,
                           OID_LENGTH(rsuSysLogQueryPriority_oid),
                           HANDLER_CAN_RWRITE))){
      DEBUGMSGTL((MY_NAME, "rsuSysLogQueryPriority install FAIL.\n"));
      set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
  }
  /* 5.15.6 */
  if(MIB_REGISTERED_OK != netsnmp_register_scalar(netsnmp_create_handler_registration
                          ("rsuSysLogQueryGenerate", 
                           handle_rsuSysLogQueryGenerate,
                           rsuSysLogQueryGenerate_oid,
                           OID_LENGTH(rsuSysLogQueryGenerate_oid),
                           HANDLER_CAN_RWRITE))){
      DEBUGMSGTL((MY_NAME, "rsuSysLogQueryGenerate install FAIL.\n"));
      set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
  }
  /* 5.15.7 */
  if(MIB_REGISTERED_OK != netsnmp_register_scalar(netsnmp_create_handler_registration
                          ("rsuSysLogQueryStatus", 
                           handle_rsuSysLogQueryStatus,
                           rsuSysLogQueryStatus_oid,
                           OID_LENGTH(rsuSysLogQueryStatus_oid),
                           HANDLER_CAN_RWRITE))){
      DEBUGMSGTL((MY_NAME, "rsuSysLogQueryStatus install FAIL.\n"));
      set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
  }
  /* 5.15.8 */
  if(MIB_REGISTERED_OK != netsnmp_register_scalar(netsnmp_create_handler_registration
                          ("rsuSysLogCloseCommand", 
                           handle_rsuSysLogCloseCommand,
                           rsuSysLogCloseCommand_oid,
                           OID_LENGTH(rsuSysLogCloseCommand_oid),
                           HANDLER_CAN_RWRITE))){
      DEBUGMSGTL((MY_NAME, "rsuSysLogCloseCommand install FAIL.\n"));
      set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
  }
  /* 5.15.9 */
  if(MIB_REGISTERED_OK != netsnmp_register_scalar(netsnmp_create_handler_registration
                          ("rsuSysLogSeverity", 
                           handle_rsuSysLogSeverity,
                           rsuSysLogSeverity_oid,
                           OID_LENGTH(rsuSysLogSeverity_oid),
                           HANDLER_CAN_RWRITE))){
      DEBUGMSGTL((MY_NAME, "rsuSysLogSeverity install FAIL.\n"));
      set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
  }
  /* 5.15.10 */
  if(MIB_REGISTERED_OK != netsnmp_register_scalar(netsnmp_create_handler_registration
                          ("rsuSysConfigId", 
                           handle_rsuSysConfigId,
                           rsuSysConfigId_oid,
                           OID_LENGTH(rsuSysConfigId_oid),
                           HANDLER_CAN_RWRITE))){
      DEBUGMSGTL((MY_NAME, "rsuSysConfigId install FAIL.\n"));
      set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
  }
  /* 5.15.11 */
  if(MIB_REGISTERED_OK != netsnmp_register_scalar(netsnmp_create_handler_registration
                          ("rsuSysRetries", 
                           handle_rsuSysRetries,
                           rsuSysRetries_oid,
                           OID_LENGTH(rsuSysRetries_oid),
                           HANDLER_CAN_RWRITE))){
      DEBUGMSGTL((MY_NAME, "rsuSysRetries install FAIL.\n"));
      set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
  }
  /* 5.15.12 */
  if(MIB_REGISTERED_OK != netsnmp_register_scalar(netsnmp_create_handler_registration
                          ("rsuSysRetryPeriod", 
                           handle_rsuSysRetryPeriod,
                           rsuSysRetryPeriod_oid,
                           OID_LENGTH(rsuSysRetryPeriod_oid),
                           HANDLER_CAN_RWRITE))){
      DEBUGMSGTL((MY_NAME, "rsuSysRetryPeriod install FAIL.\n"));
      set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
  }
  /* 5.15.13 */
  if(MIB_REGISTERED_OK != netsnmp_register_scalar(netsnmp_create_handler_registration
                          ("rsuShortCommLossTime", 
                           handle_rsuShortCommLossTime,
                           rsuShortCommLossTime_oid,
                           OID_LENGTH(rsuShortCommLossTime_oid),
                           HANDLER_CAN_RWRITE))){
      DEBUGMSGTL((MY_NAME, "rsuShortCommLossTime install FAIL.\n"));
      set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
  }
  /* 5.15.14 */
  if(MIB_REGISTERED_OK != netsnmp_register_scalar(netsnmp_create_handler_registration
                          ("rsuLongCommLossTime", 
                           handle_rsuLongCommLossTime,
                           rsuLongCommLossTime_oid,
                           OID_LENGTH(rsuLongCommLossTime_oid),
                           HANDLER_CAN_RWRITE))){
      DEBUGMSGTL((MY_NAME, "rsuLongCommLossTime install FAIL.\n"));
      set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
  }
  /* 5.15.15 */
  if(MIB_REGISTERED_OK != netsnmp_register_scalar(netsnmp_create_handler_registration
                          ("rsuSysLogName", 
                           handle_rsuSysLogName,
                           rsuSysLogName_oid,
                           OID_LENGTH(rsuSysLogName_oid),
                           HANDLER_CAN_RONLY))){
      DEBUGMSGTL((MY_NAME, "rsuSysLogName install FAIL.\n"));
      set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
  }
  /* 5.15.16 */
  if(MIB_REGISTERED_OK != netsnmp_register_scalar(netsnmp_create_handler_registration
                          ("rsuSysDir", 
                           handle_rsuSysDir,
                           rsuSysDir_oid,
                           OID_LENGTH(rsuSysDir_oid),
                           HANDLER_CAN_RONLY))){
      DEBUGMSGTL((MY_NAME, "rsuSysDir install FAIL.\n"));
      set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
  }
  /* 5.15.17 */
  if(MIB_REGISTERED_OK != netsnmp_register_scalar(netsnmp_create_handler_registration
                          ("rsuLongCommLossReboot", 
                           handle_rsuLongCommLossReboot,
                           rsuLongCommLossReboot_oid,
                           OID_LENGTH(rsuLongCommLossReboot_oid),
                           HANDLER_CAN_RWRITE))){
      DEBUGMSGTL((MY_NAME, "rsuLongCommLossReboot install FAIL.\n"));
      set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
  }
  /* 5.15.18 */
  if(MIB_REGISTERED_OK != netsnmp_register_scalar(netsnmp_create_handler_registration
                          ("rsuHostIpAddr", 
                           handle_rsuHostIpAddr,
                           rsuHostIpAddr_oid,
                           OID_LENGTH(rsuHostIpAddr_oid),
                           HANDLER_CAN_RWRITE))){
      DEBUGMSGTL((MY_NAME, "rsuHostIpAddr install FAIL.\n"));
      set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
  }
  /* 5.15.19 */
  if(MIB_REGISTERED_OK != netsnmp_register_scalar(netsnmp_create_handler_registration
                          ("rsuHostNetMask", 
                           handle_rsuHostNetMask,
                           rsuHostNetMask_oid,
                           OID_LENGTH(rsuHostNetMask_oid),
                           HANDLER_CAN_RWRITE))){
      DEBUGMSGTL((MY_NAME, "rsuHostNetMask install FAIL.\n"));
      set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
  }
  /* 5.15.20 */
  if(MIB_REGISTERED_OK != netsnmp_register_scalar(netsnmp_create_handler_registration
                          ("rsuHostGateway", 
                           handle_rsuHostGateway,
                           rsuHostGateway_oid,
                           OID_LENGTH(rsuHostGateway_oid),
                           HANDLER_CAN_RWRITE))){
      DEBUGMSGTL((MY_NAME, "rsuHostGateway install FAIL.\n"));
      set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
  }
  /* 5.15.21 */
  if(MIB_REGISTERED_OK != netsnmp_register_scalar(netsnmp_create_handler_registration
                          ("rsuHostDNS", 
                           handle_rsuHostDNS,
                           rsuHostDNS_oid,
                           OID_LENGTH(rsuHostDNS_oid),
                           HANDLER_CAN_RWRITE))){
      DEBUGMSGTL((MY_NAME, "rsuHostDNS install FAIL.\n"));
      set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
  }
  /* 5.15.22 */
  if(MIB_REGISTERED_OK != netsnmp_register_scalar(netsnmp_create_handler_registration
                          ("rsuHostDHCPEnable", 
                           handle_rsuHostDHCPEnable,
                           rsuHostDHCPEnable_oid,
                           OID_LENGTH(rsuHostDHCPEnable_oid),
                           HANDLER_CAN_RWRITE))){
      DEBUGMSGTL((MY_NAME, "rsuHostDHCPEnable install FAIL.\n"));
      set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
  }
}
void destroy_rsuSysSettings(void)
{
}

/* RSU App thread support. */

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
#if 0
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
      DEBUGMSGTL((MY_NAME, "check_stopStart_dates: stop < start time.\n"));
      return SNMP_ROW_NOTREADY;
  }
  if(StopTimeDsec < UTC_dsec){
      DEBUGMSGTL((MY_NAME, "check_stopStart_UTC_dsec: stop(0x%lx) < now(0x%lx): Expired & ready for destroy.\n", StopTimeDsec, UTC_dsec));
      /* We never destroy rows. Snmp client manages rows any way they want. */
      return SNMP_ROW_NOTREADY; /* Expired. Set to NOTREADY. */
  }
  if(StartTimeDsec <= UTC_dsec) { /* This message is active assuming it is well formed. */
      return SNMP_ROW_ACTIVE;
  }
  if(UTC_dsec < StartTimeDsec) {
      /* These files stick around so lots of debug if enabled. */
      DEBUGMSGTL((MY_NAME, "check_stopStart_UTC_dsec: now < start: Not in service yet.\n"));
      return SNMP_ROW_NOTINSERVICE; /* Time to start not here but valid nonetheless. */
  }
  DEBUGMSGTL((MY_NAME, "check_stopStart_UTC_dsec: internal error.\n"));
  return SNMP_ROW_NOTREADY; /* Should not be possible to get here. */
}
#endif
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
static void force_sysSettsShm_update(void) 
{
  /* One time load from MIB to SHM. After that only client update will trigger update on a single row. */
  if(RSEMIB_OK != update_rsuSysSettings_shm()){
      set_error_states(RSEMIB_SHM_SEED_FAILED);
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
int32_t commit_rsuSysSettings_to_disk(void)
{
  FILE    * file_out       = NULL;
  uint32_t  mib_byte_count = 0;
  uint32_t  write_count    = 0;
  int32_t   ret            = RSEMIB_OK;
  RsuSysSetts_t sysSetts;

  /* Update accordingly from MIB into pretty structure to write to disk. */
  memset(&sysSetts,0x0,sizeof(sysSetts));
  memcpy(sysSetts.rsuNotifyIpAddress, ntcip_1218_mib.rsuNotifyIpAddress,RSU_DEST_IP_MAX);
  sysSetts.rsuNotifyIpAddress_length = ntcip_1218_mib.rsuNotifyIpAddress_length;
  sysSetts.rsuNotifyPort = ntcip_1218_mib.rsuNotifyPort;
  memcpy(sysSetts.rsuSysLogQueryStart, ntcip_1218_mib.rsuSysLogQueryStart, RFC2579_DATEANDTIME_LENGTH);
  memcpy(sysSetts.rsuSysLogQueryStop, ntcip_1218_mib.rsuSysLogQueryStop, RFC2579_DATEANDTIME_LENGTH);
  sysSetts.rsuSysLogQueryPriority = ntcip_1218_mib.rsuSysLogQueryPriority;
  sysSetts.rsuSysLogQueryGenerate = ntcip_1218_mib.rsuSysLogQueryGenerate;
  sysSetts.rsuSysLogQueryStatus = ntcip_1218_mib.rsuSysLogQueryStatus;
  sysSetts.rsuSysLogCloseCommand = ntcip_1218_mib.rsuSysLogCloseCommand; 
  sysSetts.rsuSysLogSeverity = ntcip_1218_mib.rsuSysLogSeverity;
  memcpy(sysSetts.rsuSysConfigId, ntcip_1218_mib.rsuSysConfigId, RSU_SYS_CONFIG_ID_LENGTH_MAX);
  sysSetts.rsuSysConfigId_length = ntcip_1218_mib.rsuSysConfigId_length;
  sysSetts.rsuSysRetries = ntcip_1218_mib.rsuSysRetries;
  sysSetts.rsuSysRetryPeriod = ntcip_1218_mib.rsuSysRetryPeriod;
  sysSetts.rsuSysRetryAttempts = ntcip_1218_mib.rsuSysRetryAttempts;
  sysSetts.rsuSysRetryPeriodStart  = ntcip_1218_mib.rsuSysRetryPeriodStart;
  sysSetts.rsuShortCommLossTime = ntcip_1218_mib.rsuShortCommLossTime;
  sysSetts.rsuLongCommLossTime = ntcip_1218_mib.rsuLongCommLossTime;
  memcpy(sysSetts.rsuSysLogName, ntcip_1218_mib.rsuSysLogName, RSU_SYSLOG_NAME_AND_PATH_LENGTH_MAX);
  sysSetts.rsuSysLogName_length = ntcip_1218_mib.rsuSysLogName_length;
  memcpy(sysSetts.rsuSysDir, ntcip_1218_mib.rsuSysDir, RSU_SYSDIR_NAME_LENGTH_MAX);
  sysSetts.rsuSysDir_length = ntcip_1218_mib.rsuSysDir_length;
  sysSetts.rsuLongCommLossReboot = ntcip_1218_mib.rsuLongCommLossReboot;
  memcpy(sysSetts.rsuHostIpAddr, ntcip_1218_mib.rsuHostIpAddr, RSU_DEST_IP_MAX);
  sysSetts.rsuHostIpAddr_length = ntcip_1218_mib.rsuHostIpAddr_length;
  memcpy(sysSetts.rsuHostNetMask, ntcip_1218_mib.rsuHostNetMask, RSU_DEST_IP_MAX);
  sysSetts.rsuHostNetMask_length = ntcip_1218_mib.rsuHostNetMask_length;
  memcpy(sysSetts.rsuHostGateway, ntcip_1218_mib.rsuHostGateway, RSU_DEST_IP_MAX);
  sysSetts.rsuHostGateway_length = ntcip_1218_mib.rsuHostGateway_length;
  memcpy(sysSetts.rsuHostDNS, ntcip_1218_mib.rsuHostDNS, RSU_DEST_IP_MAX);
  sysSetts.rsuHostDNS_length = ntcip_1218_mib.rsuHostDNS_length;
  sysSetts.rsuHostDHCPEnable = ntcip_1218_mib.rsuHostDHCPEnable;
  sysSetts.version = SYSTEM_SETTINGS_VERSION;

  DEBUGMSGTL((MY_NAME, "commit_to_disk:fopen(%s).\n",SYS_SETTS_OUTPUT_LOCATION_NAME));
  if ((file_out = fopen(SYS_SETTS_OUTPUT_LOCATION_NAME, "wb")) == NULL){
      DEBUGMSGTL((MY_NAME, "commit_to_disk: fopen fail.\n"));
      ret = RSEMIB_FOPEN_FAIL;
  } else {
      sysSetts.crc = (uint32_t)crc16((const uint8_t *)&sysSetts, sizeof(RsuSysSetts_t) - 4);
      mib_byte_count = sizeof(RsuSysSetts_t);
      write_count = fwrite((uint8_t *)&sysSetts, mib_byte_count, 1, file_out);
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
  if(RSEMIB_OK != ret){
      set_error_states(ret);
  }
  return ret;
}
/*
 * TODO: Client needs to be smart and disable before editing else fun! 
 */
void *sysSettingsThr(void __attribute__((unused)) *arg)
{
  size_t shm_sysSetts_size = sizeof(sysSetts_t);
  int32_t i = 0;
  uint32_t utc_seconds = 0; /* If you have no GNSS how will you know how much time elapsed? */
  rsuhealth_t * shm_rsuhealth_ptr = NULL;
  rsuhealth_t   localrsuhealth;
  int32_t doitonce = 0;
  memset(&localrsuhealth, 0x0,sizeof(localrsuhealth));

  /* Open RSUHEALTH SHM, if fails not fatal. MIB and IWMH will suffer. */
  if ((shm_rsuhealth_ptr = wsu_share_init(sizeof(rsuhealth_t), RSUHEALTH_SHM_PATH)) == NULL) {
      I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"RSUHEALTH SHM init failed.\n");
      shm_rsuhealth_ptr = NULL; /* Keep going, not fatal. */
  }
  /* Open SYSETTS SHM. */
  if ((shm_sysSetts_ptr = mib_share_init(shm_sysSetts_size, SYS_SETTS_SHM_PATH)) == NULL) {
      I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"SYSETTS SHM init failed.\n");
      shm_sysSetts_ptr = NULL; /* Fatal. */
  } else {
      for(i=0;i<5;i++) /* Give RSU apps a chance to catch up before updating SHM. LOGMGR had issue? */
          sleep(1);
      force_sysSettsShm_update(); /* Force update of rows recovered from disk. */
      while(mainloop) {
          sleep(1);
          if((0 == doitonce) && (shm_rsuhealth_ptr)) {
//TODO: WSU_ NOt threadsafe
              WSU_SHMLOCK_LOCKR(&shm_rsuhealth_ptr->h.ch_lock);
              if(WTRUE == shm_rsuhealth_ptr->h.ch_data_valid) {
                  memcpy(&localrsuhealth,shm_rsuhealth_ptr,sizeof(localrsuhealth));
              } else {
                  localrsuhealth.h.ch_data_valid = WFALSE;
                  #if defined(EXTRA_DEBUG)
                  I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"RSUHEALTH SHM not ready.\n");
                  #endif
              }
              WSU_SHMLOCK_UNLOCKR(&shm_rsuhealth_ptr->h.ch_lock);
              if(WFALSE == localrsuhealth.h.ch_data_valid) {
                  continue; /* Nothing to do let logic fall through to bottom. */
              }
              if(RSEMIB_OK == ntcip1218_lock()){
                  /* If RSUHEALTH signals FAULT and user wants recovery attempt then reboot. */
                  if(   ntcip_1218_mib.rsuSysRetries
                     && ntcip_1218_mib.rsuSysRetryPeriod
                     && (localrsuhealth.errorstates & RSUHEALTH_FAULT_MODE)){ /* RSUHEALTH has requested reset. */
                      utc_seconds = i2vUtilGetUTCTimeInSec(); /* Start timer */
                      if(0 == utc_seconds) {
                          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"rsuSysRetries: utc is zero.\n");
                      }
                      I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"reboot request: utc sec(%u) attempt(%d)\n",utc_seconds,ntcip_1218_mib.rsuSysRetryAttempts);
                      if(0 == ntcip_1218_mib.rsuSysRetryAttempts) {
                          ntcip_1218_mib.rsuSysRetryPeriodStart = utc_seconds;
                      }
                      /* Clock went backwards or froze or bogus start. Cant do period check. Clock may be issue so proceed to reboot.*/
                      if ((0 == ntcip_1218_mib.rsuSysRetryPeriodStart) || (utc_seconds < ntcip_1218_mib.rsuSysRetryPeriodStart)) {
                          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"rsuSysRetries:recover period(%d) clock error: start(%u) >= end(%u)\n",
                                     ntcip_1218_mib.rsuSysRetryPeriod,ntcip_1218_mib.rsuSysRetryPeriodStart, utc_seconds);
                          if(ntcip_1218_mib.rsuSysRetryAttempts < ntcip_1218_mib.rsuSysRetries) {
                              ntcip_1218_mib.rsuSysRetryAttempts++;
                              commit_rsuSysSettings_to_disk(); /* Write to disk */
                              I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"rsuSysRetries(%d of %d), rebooting to recover.\n",ntcip_1218_mib.rsuSysRetryAttempts,ntcip_1218_mib.rsuSysRetries);
                              sleep(1);
                              doitonce = 1;
                              if(0 != (system("reboot"))){
                                  set_error_states(RSEMIB_SYSCALL_FAIL);
                                  I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"rsuSysRetries: reboot has failed. Manual reboot required.\n");
                              }
                              /* No return from here. */
                           } else {
                               I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"rsuSysRetries(%d of %d), recover attempts exhausted, not rebooting.\n",ntcip_1218_mib.rsuSysRetryAttempts,ntcip_1218_mib.rsuSysRetries);
                           }
                      } else {
                          if(ntcip_1218_mib.rsuSysRetryAttempts < ntcip_1218_mib.rsuSysRetries) {
                              if((int32_t)(utc_seconds - ntcip_1218_mib.rsuSysRetryPeriodStart) < (60 * ntcip_1218_mib.rsuSysRetryPeriod)){
                                  ntcip_1218_mib.rsuSysRetryAttempts++;
                                  commit_rsuSysSettings_to_disk();/* Write to disk */
                                  I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"rsuSysRetries(%d of %d), rebooting to recover.\n",ntcip_1218_mib.rsuSysRetryAttempts,ntcip_1218_mib.rsuSysRetries);
                                  sleep(1);
                                  doitonce = 1;
                                  if(0 != (system("reboot"))){
                                      set_error_states(RSEMIB_SYSCALL_FAIL);
                                      I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"rsuSysRetries: reboot has failed. Manual reboot required.\n");
                                  }
                                  /* No return from here. */
                              } else {
                                  I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"rsuSysRetries(%d of %d), recover period(%d) exhausted, not rebooting: start:end(%u,%u)\n",
                                      ntcip_1218_mib.rsuSysRetryAttempts,ntcip_1218_mib.rsuSysRetries
                                     ,ntcip_1218_mib.rsuSysRetryPeriod,ntcip_1218_mib.rsuSysRetryPeriodStart, utc_seconds);
                              }
                          } else {
                              I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"rsuSysRetries(%d of %d), recover attempts exhausted, not rebooting.\n",ntcip_1218_mib.rsuSysRetryAttempts,ntcip_1218_mib.rsuSysRetries);
                          }
                      } /* if clock froze */ 
                  } /* if reboot requested */
                  ntcip1218_unlock();/* MIB uptodate. */
              } else {
                 set_error_states(RSEMIB_LOCK_FAIL); /* Can happen so try again next iteration. */
              }
          }/* if shm_rsuhealth_ptr */
      } /* while */
      /* Close SHM. */
      if(NULL != shm_sysSetts_ptr) {
          mib_share_kill(shm_sysSetts_ptr, shm_sysSetts_size );
          shm_sysSetts_ptr = NULL; 
      }
      if(NULL != shm_rsuhealth_ptr) {
          wsu_share_kill(shm_rsuhealth_ptr, sizeof(rsuhealth_t));
          shm_rsuhealth_ptr = NULL;
      }
  }
  DEBUGMSGTL((MY_NAME,"sysSettsThr: Exit."));
  pthread_exit(NULL);
}
/* SHM is not locked. */
int32_t get_sysLogName(uint8_t * sysLogName, int32_t * length)
{
  if(NULL == shm_sysSetts_ptr) { /* Should never happen by order of startup. */
      DEBUGMSGTL((MY_NAME, "get_sysLogName: RSEMIB_SHM_INIT_FAILED.\n"));
      set_error_states(RSEMIB_SHM_INIT_FAILED);
      return RSEMIB_SHM_INIT_FAILED; 
  }
  if((NULL == sysLogName) || (NULL == length)) {
      DEBUGMSGTL((MY_NAME, "get_sysLogName: RSEMIB_BAD_DATA.\n"));
      set_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA; 
  }
  if((shm_sysSetts_ptr->rsuSysLogName_length < RSU_SYSLOG_NAME_AND_PATH_LENGTH_MIN) || (RSU_SYSLOG_NAME_AND_PATH_LENGTH_MAX < shm_sysSetts_ptr->rsuSysLogName_length)){
      set_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } else {
      memcpy(ntcip_1218_mib.rsuSysLogName,shm_sysSetts_ptr->rsuSysLogName,shm_sysSetts_ptr->rsuSysLogName_length);
      *length = shm_sysSetts_ptr->rsuSysLogName_length;
  }
  return RSEMIB_OK;
}
/* SHM is not locked. */
int32_t get_sysDir(uint8_t * sysDir, int32_t * length)
{
  if(NULL == shm_sysSetts_ptr) { /* Should never happen by order of startup. */
      DEBUGMSGTL((MY_NAME, "get_sysDir: RSEMIB_SHM_INIT_FAILED.\n"));
      set_error_states(RSEMIB_SHM_INIT_FAILED);
      return RSEMIB_SHM_INIT_FAILED; 
  }
  if((NULL == sysDir) || (NULL == length)) {
      DEBUGMSGTL((MY_NAME, "get_sysDir: RSEMIB_BAD_DATA.\n"));
      set_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA; 
  }
  if((shm_sysSetts_ptr->rsuSysDir_length < RSU_SYSDIR_NAME_LENGTH_MIN) || (RSU_SYSDIR_NAME_LENGTH_MAX < shm_sysSetts_ptr->rsuSysDir_length)){
      set_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } else {
      memcpy(ntcip_1218_mib.rsuSysDir,shm_sysSetts_ptr->rsuSysDir,shm_sysSetts_ptr->rsuSysDir_length);
      *length = shm_sysSetts_ptr->rsuSysDir_length;
  }
  return RSEMIB_OK;
}
/* SHM is not locked. */
int32_t get_sysLogQueryStatus(void)
{
  if(NULL == shm_sysSetts_ptr) { /* Should never happen by order of startup. */
      DEBUGMSGTL((MY_NAME, "get_sysDir: RSEMIB_SHM_INIT_FAILED.\n"));
      set_error_states(RSEMIB_SHM_INIT_FAILED);
      return RSEMIB_SHM_INIT_FAILED; 
  }
  ntcip_1218_mib.rsuSysLogQueryStatus = shm_sysSetts_ptr->rsuSysLogQueryStatus;
  return ntcip_1218_mib.rsuSysLogQueryStatus;
}
/* SHM is not locked. */
int32_t get_sysLogCloseCommand(void)
{
  if(NULL == shm_sysSetts_ptr) { /* Should never happen by order of startup. */
      DEBUGMSGTL((MY_NAME, "get_sysLogCloseCommand(: RSEMIB_SHM_INIT_FAILED.\n"));
      set_error_states(RSEMIB_SHM_INIT_FAILED);
      return RSEMIB_SHM_INIT_FAILED; 
  }
  ntcip_1218_mib.rsuSysLogCloseCommand = shm_sysSetts_ptr->rsuSysLogCloseCommand;
  return ntcip_1218_mib.rsuSysLogCloseCommand;
}
/* SHM is not locked. */
int32_t get_sysLogQueryGenerate(void)
{
  if(NULL == shm_sysSetts_ptr) { /* Should never happen by order of startup. */
      DEBUGMSGTL((MY_NAME, "get_sysLogQueryGenerate(: RSEMIB_SHM_INIT_FAILED.\n"));
      set_error_states(RSEMIB_SHM_INIT_FAILED);
      return RSEMIB_SHM_INIT_FAILED; 
  }
  ntcip_1218_mib.rsuSysLogQueryGenerate = shm_sysSetts_ptr->rsuSysLogQueryGenerate;
  return ntcip_1218_mib.rsuSysLogQueryGenerate;
}
/* SHM is not locked. */
int32_t update_rsuSysSettings_shm(void)
{
  uint64_t UTC_dsec = 0;

  if(NULL == shm_sysSetts_ptr) { /* Should never happen by order of startup. */
      DEBUGMSGTL((MY_NAME, "update_interfaceLog_shm: RSEMIB_SHM_INIT_FAILED.\n"));
      set_error_states(RSEMIB_SHM_INIT_FAILED);
      return RSEMIB_SHM_INIT_FAILED; 
  }
  /* Check if different than MIB contents and mark appropriate sections dirty if updated. */
  shm_sysSetts_ptr->dirty_syslog = 0;
  if(RSEMIB_OK != DateAndTime_To_UTC_DSEC(ntcip_1218_mib.rsuSysLogQueryStart, &UTC_dsec)){
      return RSEMIB_BAD_UTC;
  }
  shm_sysSetts_ptr->start_utc_dsec = UTC_dsec;
  if(RSEMIB_OK != DateAndTime_To_UTC_DSEC(ntcip_1218_mib.rsuSysLogQueryStop, &UTC_dsec)){
      return RSEMIB_BAD_UTC;
  }
  shm_sysSetts_ptr->stop_utc_dsec = UTC_dsec;
  shm_sysSetts_ptr->rsuSysLogQueryPriority = ntcip_1218_mib.rsuSysLogQueryPriority;
  shm_sysSetts_ptr->rsuSysLogQueryGenerate = ntcip_1218_mib.rsuSysLogQueryGenerate;
  shm_sysSetts_ptr->rsuSysLogCloseCommand = ntcip_1218_mib.rsuSysLogCloseCommand;
  shm_sysSetts_ptr->rsuSysLogSeverity = ntcip_1218_mib.rsuSysLogSeverity;
  shm_sysSetts_ptr->rsuSysLogQueryStatus = ntcip_1218_mib.rsuSysLogQueryStatus;
  memcpy(shm_sysSetts_ptr->rsuSysLogName,ntcip_1218_mib.rsuSysLogName,RSU_SYSLOG_NAME_AND_PATH_LENGTH_MAX);
  shm_sysSetts_ptr->rsuSysLogName_length = ntcip_1218_mib.rsuSysLogName_length;
  memcpy(shm_sysSetts_ptr->rsuSysDir,ntcip_1218_mib.rsuSysDir,RSU_SYSDIR_NAME_LENGTH_MAX);
  shm_sysSetts_ptr->rsuSysDir_length = ntcip_1218_mib.rsuSysDir_length;

  shm_sysSetts_ptr->dirty_syslog = 1;

#if 0
  //TODO: SYSCONF?: globalSetIDParameter.0? Back up configs and have versions to restore.
  shm_sysSetts_ptr->dirty_sysconf = 0;
  if(0 != memcmp(shm_sysSetts_ptr->rsuSysConfigId, ntcip_1218_mib.rsuSysConfigId,RSU_SYS_CONFIG_ID_LENGTH_MAX)) {
      memcpy(shm_sysSetts_ptr->rsuSysConfigId, ntcip_1218_mib.rsuSysConfigId, RSU_SYS_CONFIG_ID_LENGTH_MAX);
      shm_sysSetts_ptr->rsuSysConfigId_length = ntcip_1218_mib.rsuSysConfigId_length;
      shm_sysSetts_ptr->dirty_sysconf = 1;
  }
#endif

  return RSEMIB_OK;
}
