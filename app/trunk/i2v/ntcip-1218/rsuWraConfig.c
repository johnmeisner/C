/**************************************************************************
 *                                                                        *
 *     File Name:  rsuWraConfig .c                                        *
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
#if defined(MY_UNIT_TEST)
#include "stubs.h"
#endif

/*
 * Defines.
 */

/* Mandatory: For debugging and syslog output. */
#define MY_NAME "WraConfig"

/* rsuWraConfig.c specific. */
#define WRA_CONFIG_INSTALL_FAIL      -1
#define WRA_CONFIG_BAD_INPUT         -2
#define WRA_CONFIG_LOCK_FAIL         -3

#define WRA_CONFIG_BAD_DATA          -5
#define WRA_CONFIG_BAD_MODE          -6

/******************************************************************************
 * 5.11.1 - 5.11.7: WAVE Router AD Configuration:{ rsu 10 }: IPv6 & MAC info
 *
 *  uint8_t rsuWraIpPrefix[RSU_DEST_IP_MAX];
 *  uint8_t rsuWraIpPrefixLength;
 *  uint8_t rsuWraGateway[RSU_DEST_IP_MAX];
 *  unit8_t rsuWraPrimaryDns[RSU_DEST_IP_MAX];
 *  unit8_t rsuWraSecondaryDns[RSU_DEST_IP_MAX];
 *  uint8_t rsuWraGatewayMacAddress[RSU_RADIO_MAC_LENGTH];
 *  int32_t rsuWraLifetime;
 *
 ******************************************************************************/
const oid rsuWraIpPrefix_oid[]          = {1, 3, 6, 1, 4, 1, 1206, 4, 2, 18, 10 , 1};
const oid rsuWraIpPrefixLength_oid[]    = {1, 3, 6, 1, 4, 1, 1206, 4, 2, 18, 10 , 2};
const oid rsuWraGateway_oid[]           = {1, 3, 6, 1, 4, 1, 1206, 4, 2, 18, 10 , 3};
const oid rsuWraPrimaryDns_oid[]        = {1, 3, 6, 1, 4, 1, 1206, 4, 2, 18, 10 , 4};
const oid rsuWraSecondaryDns_oid[]      = {1, 3, 6, 1, 4, 1, 1206, 4, 2, 18, 10 , 5};
const oid rsuWraGatewayMacAddress_oid[] = {1, 3, 6, 1, 4, 1, 1206, 4, 2, 18, 10 , 6};
const oid rsuWraLifetime_oid[]          = {1, 3, 6, 1, 4, 1, 1206, 4, 2, 18, 10 , 7};

static uint32_t rsuWraConfig_error_states = 0x0;

void set_rsuWraConfig_error_states(int32_t rsuWraConfig_error)
{
  int32_t dummy = 0;

  dummy = abs(rsuWraConfig_error);

  /* -1 to -31 */
  if((rsuWraConfig_error < 0) && (rsuWraConfig_error > MIB_ERROR_BASE_2)){
      rsuWraConfig_error_states |= (uint32_t)(0x1) << (dummy - 1);
  }
}
uint32_t get_rsuWraConfig_error_states(void)
{
  return rsuWraConfig_error_states;
}
/* Unified way to report back to SNMP requests. For handlers only. */
static int32_t handle_rsemib_errors(int32_t input)
{
  int32_t ret = SNMP_ERR_NOERROR;

  switch(input){
      case RSEMIB_OK:
          ret = SNMP_ERR_NOERROR;
          break;
      case RSEMIB_BAD_INPUT:
          set_rsuWraConfig_error_states(WRA_CONFIG_BAD_INPUT);
          ret = SNMP_ERR_BADVALUE;
          break;
      case RSEMIB_LOCK_FAIL:
          set_rsuWraConfig_error_states(WRA_CONFIG_LOCK_FAIL); 
          ret = SNMP_ERR_NOACCESS;   
          break;
/* need GET VS PUT BAD DATA. */
      case RSEMIB_BAD_DATA:
          set_rsuWraConfig_error_states(WRA_CONFIG_BAD_DATA);
          ret = SNMP_ERR_BADVALUE;
          break;
      case WRA_CONFIG_BAD_MODE:
          set_rsuWraConfig_error_states(WRA_CONFIG_BAD_MODE);
          ret = SNMP_ERR_INCONSISTENTVALUE;
          break;
      default:
          set_rsuWraConfig_error_states(WRA_CONFIG_BAD_MODE);
          ret = SNMP_ERR_NOACCESS;
          break;
  }
  return ret;
}

/*
 * OID Handlers
 */

/* 5.11.1 */
int32_t handle_rsuWraIpPrefix(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo,
                              netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret = 0;
  uint8_t data[RSU_DEST_IP_MAX];

  handler = handler; /* Silence of the Warnings. */
  reginfo = reginfo;

  DEBUGMSGTL((MY_NAME, "handle_rsuWraIpPrefix: Entry:\n"));
  switch (reqinfo->mode){
      case MODE_GET:
          DEBUGMSGTL((MY_NAME, "GET:\n"));
          memset(data,0x0,sizeof(data));
          if(0 <= (ret = get_rsuWraIpPrefix(data))){
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
          if(RSEMIB_OK == (ret = preserve_rsuWraIpPrefix())){
              ret = action_rsuWraIpPrefix(requests->requestvb->val.string, requests->requestvb->val_len);
          }
          break;
      case MODE_SET_COMMIT:
          DEBUGMSGTL((MY_NAME, "COMMIT:\n"));
          if(RSEMIB_OK == (ret = commit_rsuWraIpPrefix())){
              /*
               * Back end magic here.
               */
          }
          break;
      case MODE_SET_UNDO:
          DEBUGMSGTL((MY_NAME, "UNDO:\n"));
          ret = undo_rsuWraIpPrefix();
          break;
      default:
          DEBUGMSGTL((MY_NAME, "unknown mode (%d):\n",reqinfo->mode));
          ret = WRA_CONFIG_BAD_MODE;
          break;
  }
  /* Set once and only if error. Only top handler need do this. */
  if(RSEMIB_OK != ret){
      ret = handle_rsemib_errors(ret); /* Convert from module error to SNMP request error. */
      netsnmp_set_request_error(reqinfo, requests, ret);
  }
  DEBUGMSGTL((MY_NAME, "ret=%d:Exit.\n",ret));
  return ret;
}
/* 5.11.2 */
int32_t handle_rsuWraIpPrefixLength(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo,
                                 netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret  = 0;
  uint8_t data = 0x0;

  handler = handler; /* Silence of the Warnings. */
  reginfo = reginfo;

  DEBUGMSGTL((MY_NAME, "handle_rsuWraIpPrefixLength: Entry:\n"));
  switch (reqinfo->mode){
      case MODE_GET:
          DEBUGMSGTL((MY_NAME, "GET:\n"));
          if(RSEMIB_OK == (ret = get_rsuWraIpPrefixLength(&data))){
              snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR, &data, 1);
          }
          break;
      case MODE_SET_RESERVE1:
          break;
      case MODE_SET_RESERVE2:
          break; 
      case MODE_SET_FREE:
          break;
      case MODE_SET_ACTION:
          DEBUGMSGTL((MY_NAME, "ACTION:length=%lu.\n",requests->requestvb->val_len));
          if(RSEMIB_OK == (ret = preserve_rsuWraIpPrefixLength())){
              ret = action_rsuWraIpPrefixLength( requests->requestvb->val.string, requests->requestvb->val_len);
          }
          break;
      case MODE_SET_COMMIT:
          DEBUGMSGTL((MY_NAME, "COMMIT:\n"));
          if(RSEMIB_OK == (ret = commit_rsuWraIpPrefixLength())){
              /*
               * Back end magic here.
               */
          }
          break;
      case MODE_SET_UNDO:
          DEBUGMSGTL((MY_NAME, "UNDO:\n"));
          ret = undo_rsuWraIpPrefixLength();
          break;
      default:
          DEBUGMSGTL((MY_NAME, "unknown mode (%d):\n",reqinfo->mode));
          ret = WRA_CONFIG_BAD_MODE;
          break;
  }
  /* Set once and only if error. Only top handler need do this. */
  if(RSEMIB_OK != ret){
      ret = handle_rsemib_errors(ret); /* Convert from module error to SNMP request error. */
      netsnmp_set_request_error(reqinfo, requests, ret);
  }
  DEBUGMSGTL((MY_NAME, "ret=%d:Exit.\n",ret));
  return ret;
}
/* 5.11.3 */
int32_t handle_rsuWraGateway(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo,
                              netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret = 0;
  uint8_t data[RSU_DEST_IP_MAX];

  handler = handler; /* Silence of the Warnings. */
  reginfo = reginfo;

  DEBUGMSGTL((MY_NAME, "handle_rsuWraGateway: Entry:\n"));
  switch (reqinfo->mode){
      case MODE_GET:
          DEBUGMSGTL((MY_NAME, "GET:\n"));
          memset(data,0x0,sizeof(data));
          if(0 <= (ret = get_rsuWraGateway(data))){
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
          if(RSEMIB_OK == (ret = preserve_rsuWraGateway())){
              ret = action_rsuWraGateway(requests->requestvb->val.string,requests->requestvb->val_len);
          }
          break;
      case MODE_SET_COMMIT:
          DEBUGMSGTL((MY_NAME, "COMMIT:\n"));
          if(RSEMIB_OK == (ret = commit_rsuWraGateway())){
              /*
               * Back end magic here.
               */
          }
          break;
      case MODE_SET_UNDO:
          DEBUGMSGTL((MY_NAME, "UNDO:\n"));
          ret = undo_rsuWraGateway();
          break;
      default:
          DEBUGMSGTL((MY_NAME, "unknown mode (%d):\n",reqinfo->mode));
          ret = WRA_CONFIG_BAD_MODE;
          break;
  }
  /* Set once and only if error. Only top handler need do this. */
  if(RSEMIB_OK != ret){
      ret = handle_rsemib_errors(ret); /* Convert from module error to SNMP request error. */
      netsnmp_set_request_error(reqinfo, requests, ret);
  }
  DEBUGMSGTL((MY_NAME, "ret=%d:Exit.\n",ret));
  return ret;
}
/* 5.11.4 */
int32_t handle_rsuWraPrimaryDns(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo,
                              netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret = 0;
  uint8_t data[RSU_DEST_IP_MAX];

  handler = handler; /* Silence of the Warnings. */
  reginfo = reginfo;

  DEBUGMSGTL((MY_NAME, "handle_rsuWraPrimaryDns: Entry:\n"));
  switch (reqinfo->mode){
      case MODE_GET:
          DEBUGMSGTL((MY_NAME, "GET:\n"));
          memset(data,0x0,sizeof(data));
          if(0 <= (ret = get_rsuWraPrimaryDns(data))){
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
          if(RSEMIB_OK == (ret = preserve_rsuWraPrimaryDns())){
              ret = action_rsuWraPrimaryDns(requests->requestvb->val.string,requests->requestvb->val_len);
          }
          break;
      case MODE_SET_COMMIT:
          DEBUGMSGTL((MY_NAME, "COMMIT:\n"));
          if(RSEMIB_OK == (ret = commit_rsuWraPrimaryDns())){
              /*
               * Back end magic here.
               */
          }
          break;
      case MODE_SET_UNDO:
          DEBUGMSGTL((MY_NAME, "UNDO:\n"));
          ret = undo_rsuWraPrimaryDns();
          break;
      default:
          DEBUGMSGTL((MY_NAME, "unknown mode (%d):\n",reqinfo->mode));
          ret = WRA_CONFIG_BAD_MODE;
          break;
  }
  /* Set once and only if error. Only top handler need do this. */
  if(RSEMIB_OK != ret){
      ret = handle_rsemib_errors(ret); /* Convert from module error to SNMP request error. */
      netsnmp_set_request_error(reqinfo, requests, ret);
  }
  DEBUGMSGTL((MY_NAME, "ret=%d:Exit.\n",ret));
  return ret;
}
/* 5.11.5 */
int32_t handle_rsuWraSecondaryDns(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo,
                              netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret = 0;
  uint8_t data[RSU_DEST_IP_MAX];

  handler = handler; /* Silence of the Warnings. */
  reginfo = reginfo;

  DEBUGMSGTL((MY_NAME, "handle_rsuWraSecondaryDns: Entry:\n"));
  switch (reqinfo->mode){
      case MODE_GET:
          DEBUGMSGTL((MY_NAME, "GET:\n"));
          memset(data,0x0,sizeof(data));
          if(0 <= (ret = get_rsuWraSecondaryDns(data))){
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
          if(RSEMIB_OK == (ret = preserve_rsuWraSecondaryDns())){
              ret = action_rsuWraSecondaryDns(requests->requestvb->val.string,requests->requestvb->val_len);
          }
          break;
      case MODE_SET_COMMIT:
          DEBUGMSGTL((MY_NAME, "COMMIT:\n"));
          if(RSEMIB_OK == (ret = commit_rsuWraSecondaryDns())){
              /*
               * Back end magic here.
               */
              ret = SNMP_ERR_NOERROR;
          }
          break;
      case MODE_SET_UNDO:
          DEBUGMSGTL((MY_NAME, "UNDO:\n"));
          ret = undo_rsuWraSecondaryDns();
          break;
      default:
          DEBUGMSGTL((MY_NAME, "unknown mode (%d):\n",reqinfo->mode));
          ret = WRA_CONFIG_BAD_MODE;
          break;
  }
  /* Set once and only if error. Only top handler need do this. */
  if(RSEMIB_OK != ret){
      ret = handle_rsemib_errors(ret); /* Convert from module error to SNMP request error. */
      netsnmp_set_request_error(reqinfo, requests, ret);
  }
  DEBUGMSGTL((MY_NAME, "ret=%d:Exit.\n",ret));
  return ret;
}
/* 5.11.6 */
int32_t handle_rsuWraGatewayMacAddress(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo,
                              netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret = 0;
  uint8_t data[RSU_RADIO_MAC_LENGTH];

  handler = handler; /* Silence of the Warnings. */
  reginfo = reginfo;

  DEBUGMSGTL((MY_NAME, "handle_rsuWraGatewayMacAddress: Entry:\n"));
  switch (reqinfo->mode){
      case MODE_GET:
          DEBUGMSGTL((MY_NAME, "GET:\n"));
          memset(data,0x0,sizeof(data));
          if(0 <= (ret = get_rsuWraGatewayMacAddress(data))){
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
          if(RSEMIB_OK == (ret = preserve_rsuWraGatewayMacAddress())){
              ret = action_rsuWraGatewayMacAddress(requests->requestvb->val.string, requests->requestvb->val_len);
          }
          break;
      case MODE_SET_COMMIT:
          DEBUGMSGTL((MY_NAME, "COMMIT:\n"));
          if(RSEMIB_OK == (ret = commit_rsuWraGatewayMacAddress())){
              /*
               * Back end magic here.
               */
          }
          break;
      case MODE_SET_UNDO:
          DEBUGMSGTL((MY_NAME, "UNDO:\n"));
          ret = undo_rsuWraGatewayMacAddress();
          break;
      default:
          DEBUGMSGTL((MY_NAME, "unknown mode (%d):\n",reqinfo->mode));
          ret = WRA_CONFIG_BAD_MODE;
          break;
  }
  /* Set once and only if error. Only top handler need do this. */
  if(RSEMIB_OK != ret){
      ret = handle_rsemib_errors(ret); /* Convert from module error to SNMP request error. */
      netsnmp_set_request_error(reqinfo, requests, ret);
  }
  DEBUGMSGTL((MY_NAME, "ret=%d:Exit.\n",ret));
  return ret;
}
/* 5.11.7 */
int32_t handle_rsuWraLifetime(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo,
                                 netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret = 0;

  handler = handler; /* Silence of the Warnings. */
  reginfo = reginfo;

  DEBUGMSGTL((MY_NAME, "handle_rsuWraLifetime: Entry:\n"));
  switch (reqinfo->mode){
      case MODE_GET:
          DEBUGMSGTL((MY_NAME, "GET:\n"));
          if(0 <= (ret = get_rsuWraLifetime())){
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
          if(RSEMIB_OK == (ret = preserve_rsuWraLifetime())){
              ret = action_rsuWraLifetime( *(requests->requestvb->val.integer));
          }
          break;
      case MODE_SET_COMMIT:
          DEBUGMSGTL((MY_NAME, "COMMIT:\n"));
          if(RSEMIB_OK == (ret = commit_rsuWraLifetime())){
              /*
               * Back end magic here.
               */
          }
          break;
      case MODE_SET_UNDO:
          DEBUGMSGTL((MY_NAME, "UNDO:\n"));
          ret = undo_rsuWraLifetime();
          break;
      default:
          DEBUGMSGTL((MY_NAME, "unknown mode (%d):\n",reqinfo->mode));
          ret = WRA_CONFIG_BAD_MODE;
          break;
  }
  /* Set once and only if error. Only top handler need do this. */
  if(RSEMIB_OK != ret){
      ret = handle_rsemib_errors(ret); /* Convert from module error to SNMP request error. */
      netsnmp_set_request_error(reqinfo, requests, ret);
  }
  DEBUGMSGTL((MY_NAME, "ret=%d:Exit.\n",ret));
  return ret;
}
void install_rsuWraConfig_handlers(void)
{
  /* Clear error states. */
  rsuWraConfig_error_states = 0x0;

  /* 5.11.1 */
  if(MIB_REGISTERED_OK != netsnmp_register_scalar(netsnmp_create_handler_registration
                          ("rsuWraIpPrefix", 
                           handle_rsuWraIpPrefix,
                           rsuWraIpPrefix_oid,
                           OID_LENGTH(rsuWraIpPrefix_oid),
                           HANDLER_CAN_RWRITE))){
      DEBUGMSGTL((MY_NAME, "rsuWraIpPrefix install FAIL.\n"));
      set_rsuWraConfig_error_states(WRA_CONFIG_INSTALL_FAIL);
  }
  /* 5.11.2 */
  if(MIB_REGISTERED_OK != netsnmp_register_scalar(netsnmp_create_handler_registration
                          ("rsuWraIpPrefixLength", 
                           handle_rsuWraIpPrefixLength,
                           rsuWraIpPrefixLength_oid,
                           OID_LENGTH(rsuWraIpPrefixLength_oid),
                           HANDLER_CAN_RWRITE))){
      DEBUGMSGTL((MY_NAME, "rsuWraIpPrefixLength install FAIL.\n"));
      set_rsuWraConfig_error_states(WRA_CONFIG_INSTALL_FAIL);
  }
  /* 5.11.3 */
  if(MIB_REGISTERED_OK != netsnmp_register_scalar(netsnmp_create_handler_registration
                          ("rsuWraGateway", 
                           handle_rsuWraGateway,
                           rsuWraGateway_oid,
                           OID_LENGTH(rsuWraGateway_oid),
                           HANDLER_CAN_RWRITE))){
      DEBUGMSGTL((MY_NAME, "rsuWraGateway install FAIL.\n"));
      set_rsuWraConfig_error_states(WRA_CONFIG_INSTALL_FAIL);
  }
  /* 5.11.4 */
  if(MIB_REGISTERED_OK != netsnmp_register_scalar(netsnmp_create_handler_registration
                          ("rsuWraPrimaryDns", 
                           handle_rsuWraPrimaryDns,
                           rsuWraPrimaryDns_oid,
                           OID_LENGTH(rsuWraPrimaryDns_oid),
                           HANDLER_CAN_RWRITE))){
      DEBUGMSGTL((MY_NAME, "rsuWraPrimaryDns install FAIL.\n"));
      set_rsuWraConfig_error_states(WRA_CONFIG_INSTALL_FAIL);
  }
  /* 5.11.5 */
  if(MIB_REGISTERED_OK != netsnmp_register_scalar(netsnmp_create_handler_registration
                          ("rsuWraSecondaryDns", 
                           handle_rsuWraSecondaryDns,
                           rsuWraSecondaryDns_oid,
                           OID_LENGTH(rsuWraSecondaryDns_oid),
                           HANDLER_CAN_RWRITE))){
      DEBUGMSGTL((MY_NAME, "rsuWraSecondaryDns install FAIL.\n"));
      set_rsuWraConfig_error_states(WRA_CONFIG_INSTALL_FAIL);
  }
  /* 5.11.6 */
  if(MIB_REGISTERED_OK != netsnmp_register_scalar(netsnmp_create_handler_registration
                          ("rsuWraGatewayMacAddress", 
                           handle_rsuWraGatewayMacAddress,
                           rsuWraGatewayMacAddress_oid,
                           OID_LENGTH(rsuWraGatewayMacAddress_oid),
                           HANDLER_CAN_RWRITE))){
      DEBUGMSGTL((MY_NAME, "rsuWraGatewayMacAddress install FAIL.\n"));
      set_rsuWraConfig_error_states(WRA_CONFIG_INSTALL_FAIL);
  }
  /* 5.11.7 */
  if(MIB_REGISTERED_OK != netsnmp_register_scalar(netsnmp_create_handler_registration
                          ("rsuWraLifetime", 
                           handle_rsuWraLifetime,
                           rsuWraLifetime_oid,
                           OID_LENGTH(rsuWraLifetime_oid),
                           HANDLER_CAN_RWRITE))){
      DEBUGMSGTL((MY_NAME, "rsuWraLifetime install FAIL.\n"));
      set_rsuWraConfig_error_states(WRA_CONFIG_INSTALL_FAIL);
  }
}
void destroy_rsuWraConfig(void)
{

}
