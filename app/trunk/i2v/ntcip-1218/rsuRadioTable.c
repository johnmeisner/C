/**************************************************************************
 *                                                                        *
 *     File Name:  rsuRadioTable.c                                        *
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
#define MY_NAME     "rsuRadio"
/* Mandatory: For debugging and syslog output. */
#define MY_I2V_NAME  MY_NAME
#define MY_ERR_LEVEL LEVEL_PRIV  /* from i2v_util.h */

/* table column definitions. */
#define RADIO_INDEX_COLUMN      1
#define RADIO_DESC_COLUMN       2
#define RADIO_ENABLE_COLUMN     3
#define RADIO_TYPE_COLUMN       4
#define RADIO_MAC1_COLUMN       5
#define RADIO_MAC2_COLUMN       6
#define RADIO_CHAN_MODE_COLUMN  7
#define RADIO_CHAN1_COULMN      8
#define RADIO_CHAN2_COULMN      9
#define RADIO_TX_POWER1_COLUMN  10
#define RADIO_TX_POWER2_COLUMN  11
/* Theoritical column ids. Make sure they follow in order with columns and unique. */
#define RADIO_MAX_OID           12

#define RADIO_INDEX_NAME     "rsuRadioIndex"
#define RADIO_DESC_NAME      "rsuRadioDesc"
#define RADIO_ENABLE_NAME    "rsuRadioEnable"
#define RADIO_TYPE_NAME      "rsuRadioType"
#define RADIO_MAC1_NAME      "rsuRadioMacAddress1"
#define RADIO_MAC2_NAME      "rsuRadioMacAddress2"
#define RADIO_CHAN_MODE_NAME "rsuRadioChanMode"
#define RADIO_CHAN1_NAME      "rsuRadioCh1"
#define RADIO_CHAN2_NAME     "rsuRadioCh2"
#define RADIO_TX_POWER1_NAME "rsuRadioTxPower1"
#define RADIO_TX_POWER2_NAME "rsuRadioTxPower2"
#define RADIO_MAX_NAME       "maxRsuRadios"

/******************************************************************************
 * 5.2.1 - 5.2.2: RSU Radios:{ rsu 1 }
 *   int32_t          maxRsuRadios; RO
 *   RsuRadioEntry_t  rsuRadioTable[RSU_RADIOS_MAX];
 *
 *  typedef struct {
 *    int32_t rsuRadioIndex;                             RO : 1 to 16(RSU_RADIOS_MAX)
 *    uint8_t  rsuRadioDesc[RSU_RADIO_DESC_MAX];         R/W: 0 to 144
 *    onOff_e rsuRadioEnable;                            R/W: 0ff == 0, On == 1
 *    int32_t rsuRadioType;                              R0 : 1 == other, 2 == dsrc, 3 == pC5
 *    uint8_t rsuRadioMacAddress1[RSU_RADIO_MAC_LENGTH]; RO : radio A(0). If unknown set to all 1's
 *    uint8_t rsuRadioMacAddress2[RSU_RADIO_MAC_LENGTH]; RO : radio B(1). If unknown set to all 1's. ALT mode only.
 *    int32_t rsuRadioChanMode;                          R/W: other == 1, 2 == uknown, 3 == cont, 4 == alt, 5 == immediate
 *    int32_t rsuRadioCh1;                               R/W: DSRC is 172 to 184. For pC5?
 *    int32_t rsuRadioCh2;                               R/W: DSRC is 172 to 184. For pC5?
 *    int32_t rsuRadioTxPower1;                          R/W: Dbm (-128..127): Default is -128
 *    int32_t rsuRadioTxPower2;                          R/W: Dbm (-128..127): Default is -128
 * } RsuRadioEntry_t;
 *
 ******************************************************************************/

const oid maxRsuRadios_oid[]  = {1, 3, 6, 1, 4, 1, 1206, 4, 2, 18, 1 , 1};
const oid rsuRadioTable_oid[] = {1, 3, 6, 1, 4, 1, 1206, 4, 2, 18, 1 , 2};

/* snmp table pointers */
static netsnmp_table_registration_info * table_reg_info = NULL;
static netsnmp_tdata * rsuRadioTable = NULL;
static uint32_t rsuRadioTable_error_states = 0x0;

/* 
 * Function Bodies.
 */
static void set_error_states(int32_t error)
{
  /* -1 to -32 */
  if((error < 0) && (error > MIB_ERROR_BASE_2)){
      rsuRadioTable_error_states |= (uint32_t)(0x1) << (abs(error) - 1);
  }
}
uint32_t get_rsuRadioTable_error_states(void)
{
  return rsuRadioTable_error_states;
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
      case RADIO_INDEX_COLUMN:
          strcpy(column_name,RADIO_INDEX_NAME);
          break;
      case RADIO_DESC_COLUMN:
          strcpy(column_name,RADIO_DESC_NAME);
          break;
      case RADIO_ENABLE_COLUMN:
          strcpy(column_name,RADIO_ENABLE_NAME);
          break;
      case RADIO_TYPE_COLUMN:
          strcpy(column_name,RADIO_TYPE_NAME);
          break;
      case RADIO_MAC1_COLUMN:
          strcpy(column_name,RADIO_MAC1_NAME);
          break;
      case RADIO_MAC2_COLUMN:
          strcpy(column_name,RADIO_MAC2_NAME);
          break;
      case RADIO_CHAN_MODE_COLUMN:
          strcpy(column_name,RADIO_CHAN_MODE_NAME);
          break;
      case RADIO_CHAN1_COULMN:
          strcpy(column_name,RADIO_CHAN1_NAME);
          break;
      case RADIO_CHAN2_COULMN:
          strcpy(column_name,RADIO_CHAN2_NAME);
          break;
      case RADIO_TX_POWER1_COLUMN:
          strcpy(column_name,RADIO_TX_POWER1_NAME);
          break;
      case RADIO_TX_POWER2_COLUMN:
          strcpy(column_name,RADIO_TX_POWER2_NAME);
          break;
      case RADIO_MAX_OID:
          strcpy(column_name,RADIO_MAX_NAME);
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

/* 5.2.1: RO: This method is the most straight forward. No DB access needed. */
static int32_t handle_maxRsuRadios(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo,
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
              snmp_set_var_typed_integer(requests->requestvb, ASN_INTEGER, RSU_RADIOS_MAX);
              ret = RSEMIB_OK;
              break;
          default:
              ret = RSEMIB_BAD_MODE;
              break;
      }
      /* scalars and like are always index 0 with unqiue column.*/
      print_request_result(0, RADIO_MAX_OID,requests,reqinfo->mode,ret);
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
/* 5.2.2.1: There is no access for the OID but just in case. */
static int32_t handle_rsuRadioIndex(int32_t radio, netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret = RSEMIB_OK;

  DEBUGMSGTL((MY_NAME, "%s: Entry:\n",__FUNCTION__));
  switch (reqinfo->mode){
      case MODE_GET:
          if((1 <= radio) && (radio <= RSU_RADIOS_MAX)){
              snmp_set_var_typed_integer(requests->requestvb, ASN_INTEGER, radio);
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
/* 5.2.2.2 */
static int32_t handle_rsuRadioDesc(int32_t radio,netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret = RSEMIB_OK;
  uint8_t data[RSU_RADIO_DESC_MAX];

  DEBUGMSGTL((MY_NAME, "%s: Entry:\n",__FUNCTION__));
  switch (reqinfo->mode){
      case MODE_GET:
          DEBUGMSGTL((MY_NAME, "GET:\n"));
          memset(data,0x0,sizeof(data));
          if(0 <= (ret = get_rsuRadioDesc(radio,data))){
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
          if(RSEMIB_OK == (ret = preserve_rsuRadioDesc(radio))){
              ret = action_rsuRadioDesc(radio,requests->requestvb->val.string, requests->requestvb->val_len);
          }
          break;
      case MODE_SET_COMMIT:
          DEBUGMSGTL((MY_NAME, "COMMIT:\n"));
          if(RSEMIB_OK == (ret = commit_rsuRadioDesc(radio))){
              /*
               * Back end magic here.
               */
          }
          break;
      case MODE_SET_UNDO:
          DEBUGMSGTL((MY_NAME, "UNDO:\n"));
          ret = undo_rsuRadioDesc(radio);
          break;
      default:
          ret = RSEMIB_BAD_MODE;
          break;
  }
  return ret;
}
/* 5.2.2.3 */
static int32_t handle_rsuRadioEnable(int32_t radio,netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret = RSEMIB_OK;

  DEBUGMSGTL((MY_NAME, "%s: Entry:\n",__FUNCTION__));
  switch (reqinfo->mode){
      case MODE_GET:
          DEBUGMSGTL((MY_NAME, "GET:\n"));
          if(0 <= (ret = get_rsuRadioEnable(radio))){
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
          if(RSEMIB_OK == (ret = preserve_rsuRadioEnable(radio))){
              ret = action_rsuRadioEnable(radio, *(requests->requestvb->val.integer));
          }
          break;
      case MODE_SET_COMMIT:
          DEBUGMSGTL((MY_NAME, "COMMIT:\n"));
          if(RSEMIB_OK == (ret = commit_rsuRadioEnable(radio))){
              /*
               * Back end magic here.
               */
          }
          break;
      case MODE_SET_UNDO:
          DEBUGMSGTL((MY_NAME, "UNDO:\n"));
          ret = undo_rsuRadioEnable(radio);
          break;
      default:
          ret = RSEMIB_BAD_MODE;
          break;
  }
  return ret;
}
/* 5.2.2.4: RO */
static int32_t handle_rsuRadioType(int32_t radio,netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret = RSEMIB_OK;

  DEBUGMSGTL((MY_NAME, "%s: Entry:\n",__FUNCTION__));
  switch (reqinfo->mode){
      case MODE_GET:
          DEBUGMSGTL((MY_NAME, "GET:\n"));
          if(0 <= (ret = get_rsuRadioType(radio))){
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
/* 5.2.2.5 */
static int32_t handle_rsuRadioMacAddr1(int32_t radio, netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret = RSEMIB_OK;
  uint8_t data[RSU_RADIO_MAC_LENGTH];

  DEBUGMSGTL((MY_NAME, "%s: Entry:\n",__FUNCTION__));
  switch (reqinfo->mode){
      case MODE_GET:
          DEBUGMSGTL((MY_NAME, "GET:\n"));
          memset(data,0x0,sizeof(data));
          if(RSEMIB_OK == (ret = get_rsuRadioMacAddress1(radio, data))){
              snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR, data, RSU_RADIO_MAC_LENGTH);
              ret = RSEMIB_OK;
          }
          break;
      default:
          ret = RSEMIB_BAD_MODE;
          break;
  }
  return ret;
}
/* 5.2.2.6 */
static int32_t handle_rsuRadioMacAddr2(int32_t radio, netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret = RSEMIB_OK;
  uint8_t data[RSU_RADIO_MAC_LENGTH];

  DEBUGMSGTL((MY_NAME, "%s: Entry:\n",__FUNCTION__));
  switch (reqinfo->mode){
      case MODE_GET:
          DEBUGMSGTL((MY_NAME, "GET:\n"));
          memset(data,0x0,sizeof(data));
          if(RSEMIB_OK == (ret = get_rsuRadioMacAddress2(radio, data))){
              snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR, data, RSU_RADIO_MAC_LENGTH);
              ret = RSEMIB_OK;
          }
          break;
      default:
          ret = RSEMIB_BAD_MODE;
          break;
  }
  return ret;
}
/* 5.2.2.7 */
static int32_t handle_rsuRadioChanMode(int32_t radio,netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret = RSEMIB_OK;

  DEBUGMSGTL((MY_NAME, "%s: Entry:\n",__FUNCTION__));
  switch (reqinfo->mode){
      case MODE_GET:
          DEBUGMSGTL((MY_NAME, "GET:\n"));
          if(0 <= (ret = get_rsuRadioChanMode(radio))){
              snmp_set_var_typed_integer(requests->requestvb, ASN_INTEGER, ret);
              ret = RSEMIB_OK;
          }
          break;
#if 0 /* RSU only support CONT mode. */
      case MODE_SET_RESERVE1:
          break;
      case MODE_SET_RESERVE2:
          break; 
      case MODE_SET_FREE:
          break;
      case MODE_SET_ACTION:
          DEBUGMSGTL((MY_NAME, "ACTION:\n"));
          if(RSEMIB_OK == (ret = preserve_rsuRadioChanMode(radio))){
              ret = action_rsuRadioChanMode(radio, *(requests->requestvb->val.integer));
          }
          break;
      case MODE_SET_COMMIT:
          DEBUGMSGTL((MY_NAME, "COMMIT:\n"));
          if(RSEMIB_OK == (ret = commit_rsuRadioChanMode(radio))){
              /*
               * Back end magic here. 
               */
          }
          break;
      case MODE_SET_UNDO:
          DEBUGMSGTL((MY_NAME, "UNDO:\n"));
          ret = undo_rsuRadioChanMode(radio);
          break;
#endif
      default:
          ret = RSEMIB_BAD_MODE;
          break;
  }
  return ret;
}
/* 5.2.2.8 */
static int32_t handle_rsuRadioCh1(int32_t radio,netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret = RSEMIB_OK;

  DEBUGMSGTL((MY_NAME, "%s: Entry:\n",__FUNCTION__));
  switch (reqinfo->mode){
      case MODE_GET:
          DEBUGMSGTL((MY_NAME, "GET:\n"));
          if(0 <= (ret = get_rsuRadioCh1(radio))){
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
          if(RSEMIB_OK == (ret = preserve_rsuRadioCh1(radio))){
              ret = action_rsuRadioCh1(radio, *(requests->requestvb->val.integer));
          }
          break;
      case MODE_SET_COMMIT:
          DEBUGMSGTL((MY_NAME, "COMMIT:\n"));
          if(RSEMIB_OK == (ret = commit_rsuRadioCh1(radio))){
              /*
               * Back end magic here.
               */
          }
          break;
      case MODE_SET_UNDO:
          DEBUGMSGTL((MY_NAME, "UNDO:\n"));
          ret = undo_rsuRadioCh1(radio);
          break;
      default:
          ret = RSEMIB_BAD_MODE;
          break;
  }
  return ret;
}
/* 5.2.2.9 */
static int32_t handle_rsuRadioCh2(int32_t radio,netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret = RSEMIB_OK;

  DEBUGMSGTL((MY_NAME, "%s: Entry:\n",__FUNCTION__));
  switch (reqinfo->mode){
      case MODE_GET:
          DEBUGMSGTL((MY_NAME, "GET:\n"));
          if(0 <= (ret = get_rsuRadioCh2(radio))){
              snmp_set_var_typed_integer(requests->requestvb, ASN_INTEGER, ret);
              ret = RSEMIB_OK;
          }
          break;
      /* Ch2 unified with Ch1 in i2v.conf:I2VUnifiedChannelNumber. */
      case MODE_SET_RESERVE1:
          break;
      case MODE_SET_RESERVE2:
          break; 
      case MODE_SET_FREE:
          break;
      case MODE_SET_ACTION:
          DEBUGMSGTL((MY_NAME, "ACTION:\n"));
          if(RSEMIB_OK == (ret = preserve_rsuRadioCh2(radio))){
              ret = action_rsuRadioCh2(radio, *(requests->requestvb->val.integer));
          }
          break;
      case MODE_SET_COMMIT:
          DEBUGMSGTL((MY_NAME, "COMMIT:\n"));
          if(RSEMIB_OK == (ret = commit_rsuRadioCh2(radio))){
              /*
               * Back end magic here.
               */
          }
          break;
      case MODE_SET_UNDO:
          DEBUGMSGTL((MY_NAME, "UNDO:\n"));
          ret = undo_rsuRadioCh2(radio);
          break;
      default:
          ret = RSEMIB_BAD_MODE;
          break;
  }
  return ret;
}
/* 5.2.2.10 */
static int32_t handle_rsuRadioTxPower1(int32_t radio,netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret  = RSEMIB_OK; /* NOERROR till proven otherwise. */
  int32_t data = 0;

  DEBUGMSGTL((MY_NAME, "%s: Entry:\n",__FUNCTION__));
  switch (reqinfo->mode){
      case MODE_GET:
          DEBUGMSGTL((MY_NAME, "GET:\n"));
          if(0 <= (ret = get_rsuRadioTxPower1(radio,&data))){
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
          if(RSEMIB_OK == (ret = preserve_rsuRadioTxPower1(radio))){
              ret = action_rsuRadioTxPower1(radio, *(requests->requestvb->val.integer));
          }
          break;
      case MODE_SET_COMMIT:
          DEBUGMSGTL((MY_NAME, "COMMIT:\n"));
          if(RSEMIB_OK == (ret = commit_rsuRadioTxPower1(radio))){
              /*
               * Back end magic here.
               */
          }
          break;
      case MODE_SET_UNDO:
          DEBUGMSGTL((MY_NAME, "UNDO:\n"));
          ret = undo_rsuRadioTxPower1(radio);
          break;
      default:
          ret = RSEMIB_BAD_MODE;
          break;
  }
  return ret;
}
/* 5.2.2.11 */
static int32_t handle_rsuRadioTxPower2(int32_t radio,netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret  = RSEMIB_OK;
  int32_t data = 0;

  DEBUGMSGTL((MY_NAME, "%s: Entry:\n",__FUNCTION__));
  switch (reqinfo->mode){
      case MODE_GET:
          DEBUGMSGTL((MY_NAME, "GET:\n"));
          if(0 <= (ret = get_rsuRadioTxPower2(radio,&data))){
              snmp_set_var_typed_integer(requests->requestvb, ASN_INTEGER, data);
              ret = RSEMIB_OK;
          }
          break;
#if 0 /* TxPower2 is unified with TxPower1 in I2V.CONF:I2VTransmitPower. */
      case MODE_SET_RESERVE1:
          break;
      case MODE_SET_RESERVE2:
          break; 
      case MODE_SET_FREE:
          break;
      case MODE_SET_ACTION:
          DEBUGMSGTL((MY_NAME, "ACTION:\n"));
          if(RSEMIB_OK == (ret = preserve_rsuRadioTxPower2(radio))){
              ret = action_rsuRadioTxPower2(radio, *(requests->requestvb->val.integer));
          }
          break;
      case MODE_SET_COMMIT:
          DEBUGMSGTL((MY_NAME, "COMMIT:\n"));
          if(RSEMIB_OK == (ret = commit_rsuRadioTxPower2(radio))){
              /*
               * Back end magic here.
               */
          }
          break;
      case MODE_SET_UNDO:
          DEBUGMSGTL((MY_NAME, "UNDO:\n"));
          ret = undo_rsuRadioTxPower2(radio);
          break;
#endif
      default:
          ret = RSEMIB_BAD_MODE;
          break;
  }
  return ret;
}
/* 
 * 5.1.2: RW Table: fixed length: DSRC Radio always index 1: CV2X Radio always index 2. 
 */
int32_t handle_rsuRadioTable( netsnmp_mib_handler           * handler
                             , netsnmp_handler_registration * reginfo
                             , netsnmp_agent_request_info   * reqinfo
                             , netsnmp_request_info         * requests)
{
  netsnmp_request_info       * request       = NULL;
  uint32_t                     request_count = 0;
  RsuRadioEntry_t            * table_entry   = NULL;
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

      table_entry  = (RsuRadioEntry_t *)netsnmp_tdata_extract_entry(request);
      request_info = netsnmp_extract_table_info(request);

      if((NULL == table_entry) || (NULL == request_info)){
          set_error_states(RSEMIB_BAD_REQUEST_TABLE_INFO);
          DEBUGMSGTL((MY_NAME, "NULL == request_info.\n"));
          ret = SNMP_ERR_INCONSISTENTVALUE;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"RSEMIB_BAD_REQUEST_TABLE_INFO.\n");
      } else {
#if defined(PLATFORM_HD_RSU_5940)
           if(RSU_CV2X_RADIO != table_entry->rsuRadioIndex) {
               ret = RSEMIB_BAD_INDEX;
           }
#endif
           if(RSEMIB_OK == ret) {
               DEBUGMSGTL((MY_NAME, "COLUMN=%d.\n",request_info->colnum));
               switch (request_info->colnum){
                  case RADIO_INDEX_COLUMN:
                      ret = handle_rsuRadioIndex(table_entry->rsuRadioIndex, reqinfo, request);
                      break;
                  case RADIO_DESC_COLUMN:
                      ret = handle_rsuRadioDesc(table_entry->rsuRadioIndex, reqinfo, request);
                      break;
                  case RADIO_ENABLE_COLUMN:
                      ret = handle_rsuRadioEnable(table_entry->rsuRadioIndex, reqinfo, request);
                      break;
                  case RADIO_TYPE_COLUMN:
                      ret = handle_rsuRadioType(table_entry->rsuRadioIndex, reqinfo, request);
                      break;
                  case RADIO_MAC1_COLUMN:
                      ret = handle_rsuRadioMacAddr1(table_entry->rsuRadioIndex, reqinfo, request);
                      break;
                  case RADIO_MAC2_COLUMN:
                      ret = handle_rsuRadioMacAddr2(table_entry->rsuRadioIndex, reqinfo, request);
                      break;
                  case RADIO_CHAN_MODE_COLUMN:
                      ret = handle_rsuRadioChanMode(table_entry->rsuRadioIndex, reqinfo, request);
                      break;
                  case RADIO_CHAN1_COULMN:
                      ret = handle_rsuRadioCh1(table_entry->rsuRadioIndex, reqinfo, request);
                      break;
                  case RADIO_CHAN2_COULMN:
                      ret = handle_rsuRadioCh2(table_entry->rsuRadioIndex, reqinfo, request);
                      break;
                  case RADIO_TX_POWER1_COLUMN:
                      ret = handle_rsuRadioTxPower1(table_entry->rsuRadioIndex, reqinfo, request);
                      break;
                  case RADIO_TX_POWER2_COLUMN:
                      ret = handle_rsuRadioTxPower2(table_entry->rsuRadioIndex, reqinfo, request);
                      break;
                  default:
                     ret = RSEMIB_BAD_REQUEST_COLUMN;
                     break;
              }
          }
          if(ret < RSEMIB_OK) {
              set_error_states(ret);
          }
          print_request_result(table_entry->rsuRadioIndex, request_info->colnum,requests,reqinfo->mode,ret);
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

STATIC void remove_rsuRadioEntry(netsnmp_tdata * table_data, netsnmp_tdata_row * row)
{
  RsuRadioEntry_t * entry = NULL;

  if (NULL == row){
      DEBUGMSGTL((MY_NAME,"remove_rsuRadioEntry NULL input.\n"));
      return;
  }
  if (NULL != (entry = (RsuRadioEntry_t *)row->data)){ /* We allocate, so we free. */
      SNMP_FREE(entry);
  } else {
      DEBUGMSGTL((MY_NAME,"rsuRadioTable_removeEntry row data is NULL!.\n"));
  }
  if (table_data){
      netsnmp_tdata_remove_and_delete_row(table_data, row);
  } else {
      DEBUGMSGTL((MY_NAME,"remove_rsuRadioEntry NULL input.\n"));
      netsnmp_tdata_delete_row(row); 
  }
  return;
}
static void destroy_rsuRadioTable(void)
{
  netsnmp_tdata_row  * row     = NULL;
  netsnmp_tdata_row  * nextrow = NULL;
  int32_t              i       = 0;

  if(NULL != rsuRadioTable){
      if(NULL != (row = netsnmp_tdata_row_first(rsuRadioTable))){
          for(i=0;(row != NULL) && (i < RSU_RADIOS_MAX);i++){
              nextrow = netsnmp_tdata_row_next(rsuRadioTable, row);
              remove_rsuRadioEntry(rsuRadioTable, row);
              row = nextrow;
          }
      } else {
          DEBUGMSGTL((MY_NAME, "destroy_rsuRadioTable: Table empty. Nothing to do.\n"));
      }
  } else {
      DEBUGMSGTL((MY_NAME, "destroy_rsuRadioTable: Table is NULL. Did we get called before install?\n"));
  }
}
static netsnmp_tdata_row * create_rsuRadioTable(netsnmp_tdata * table_data, RsuRadioEntry_t * RsuRadio)
{
  RsuRadioEntry_t   * entry = NULL;
  netsnmp_tdata_row * row   = NULL;
  int32_t             i     = 0;
  
  if(NULL == table_data){
      DEBUGMSGTL((MY_NAME, "create_rsuRadioTable FAIL: table_data == NULL.\n"));
      return NULL;
  }

  for(i=0; i < RSU_RADIOS_MAX; i++){
      if (NULL == (entry = SNMP_MALLOC_TYPEDEF(RsuRadioEntry_t))){
          DEBUGMSGTL((MY_NAME, "SNMP_MALLOC_TYPEDEF FAILED.\n"));
          return NULL;
      }
      if (NULL == (row = netsnmp_tdata_create_row())){
          DEBUGMSGTL((MY_NAME, "netsnmp_tdata_create_row FAILED.\n"));
          SNMP_FREE(entry);
          return NULL;
      }
      row->data = entry;
      memcpy(entry,&RsuRadio[i],sizeof(RsuRadioEntry_t));
      netsnmp_tdata_row_add_index(row, ASN_INTEGER , &RsuRadio[i].rsuRadioIndex, sizeof(int32_t));
      if(SNMPERR_SUCCESS != netsnmp_tdata_add_row(table_data, row)){
          DEBUGMSGTL((MY_NAME, "netsnmp_tdata_add_row FAILED.\n"));
          return NULL;
      }
  }
  return row;
}
static int32_t install_rsuRadioTable(RsuRadioEntry_t * rsuRadio)
{
    netsnmp_handler_registration * reg = NULL;

    reg = netsnmp_create_handler_registration("rsuRadioTable"
                                              ,handle_rsuRadioTable
                                              ,rsuRadioTable_oid
                                              ,OID_LENGTH(rsuRadioTable_oid)
                                              ,HANDLER_CAN_RWRITE);

    if (NULL == (rsuRadioTable = netsnmp_tdata_create_table("rsuRadioTable", 0))){
        DEBUGMSGTL((MY_NAME, "install_rsuRadioTable FAILED.\n"));
        return RSEMIB_INSTALL_HANDLER_FAIL;
    }
    if (NULL == (table_reg_info = SNMP_MALLOC_TYPEDEF(netsnmp_table_registration_info))){
        DEBUGMSGTL((MY_NAME, "SNMP_MALLOC_TYPEDEF FAILED.\n"));
        return RSEMIB_INSTALL_HANDLER_FAIL;
    }
    netsnmp_table_helper_add_indexes(table_reg_info, ASN_INTEGER,0); /* index */                                 

    table_reg_info->min_column = RADIO_DESC_COLUMN; /* rsuRadioDesc:  Hide index by definition. */
    table_reg_info->max_column = RADIO_TX_POWER2_COLUMN; /* rsuRadioTxPower2: */

    netsnmp_tdata_register(reg, rsuRadioTable, table_reg_info);

    /* Initialize the contents of the table. */
    if (NULL == create_rsuRadioTable(rsuRadioTable, rsuRadio)){
        DEBUGMSGTL((MY_NAME, "create_rsuRadioTable FAIL.\n"));
        return RSEMIB_INSTALL_HANDLER_FAIL;
    }
    return RSEMIB_OK;
}
void install_rsuRadioTable_handlers(void)
{
  int32_t ret = 0;
  RsuRadioEntry_t rsuRadio[RSU_RADIOS_MAX]; /* Populate local copy to seed fixed ro table we are about to create. */ 

  /* Clear statics across soft reset. */
  table_reg_info = NULL;
  rsuRadioTable  = NULL;
  rsuRadioTable_error_states = 0x0;
  memset(rsuRadio,0x0,sizeof(rsuRadio));

  /* Install scalar. */
  if(MIB_REGISTERED_OK != netsnmp_register_scalar(netsnmp_create_handler_registration
                          ("maxRsuRadios", 
                           handle_maxRsuRadios,
                           maxRsuRadios_oid,
                           OID_LENGTH(maxRsuRadios_oid),
                           HANDLER_CAN_RONLY))) {
      DEBUGMSGTL((MY_NAME, "maxRsuRadios install FAIL.\n"));
      set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
  }
#if defined (PLATFORM_HD_RSU_5940)
  rsuRadio[RSU_CV2X_RADIO-1].rsuRadioIndex = 1; /* Must end at RSU_RADIOS_MAX.     */
//TODO: does it matter?
//  ret = get_rsuRadioDesc(1, rsuRadio[0].rsuRadioDesc);
  if ((RSU_RADIO_DESC_MIN <= ret) && (ret <= RSU_RADIO_DESC_MAX)){
  } else {
      set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
      DEBUGMSGTL((MY_NAME, "rsuRadioDesc.1 install FAIL.\n"));
  }
  if (0 <= (rsuRadio[0].rsuRadioType = get_rsuRadioType(1))){
  } else {
      set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
      DEBUGMSGTL((MY_NAME, "rsuRadioType.1 install FAIL.\n"));
  }
  if (0 <= (rsuRadio[0].rsuRadioChanMode = get_rsuRadioChanMode(1))){
  } else {
      set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
      DEBUGMSGTL((MY_NAME, "rsuRadioChanMode.1 install FAIL.\n"));
  }
#else
  /* Using the functions available to handlers populate table and verify. */
  rsuRadio[RSU_DSRC_RADIO-1].rsuRadioIndex = 1; /* Table index always starts at 1. */
  rsuRadio[RSU_CV2X_RADIO-1].rsuRadioIndex = 2; /* Must end at RSU_RADIOS_MAX.     */

  ret = get_rsuRadioDesc(1, rsuRadio[0].rsuRadioDesc);
  if ((RSU_RADIO_DESC_MIN <= ret) && (ret <= RSU_RADIO_DESC_MAX)){
  } else {
      set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
      DEBUGMSGTL((MY_NAME, "rsuRadioDesc.1 install FAIL.\n"));
  }
  ret = get_rsuRadioDesc(2, rsuRadio[1].rsuRadioDesc);
  if ((RSU_RADIO_DESC_MIN <= ret) && (ret <= RSU_RADIO_DESC_MAX)){
  } else {
      set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
      DEBUGMSGTL((MY_NAME, "rsuRadioDesc.2 install FAIL.\n"));
  }
#if 0 /* Do not make syscall to radio here. Too soon.*/
  if (0 <= (rsuRadio[0].rsuRadioEnable = get_rsuRadioEnable(1))){
  } else {
      set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
  }
  if (0 <= (rsuRadio[1].rsuRadioEnable = get_rsuRadioEnable(2))){
  } else {
      set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
  }
#endif
  if (0 <= (rsuRadio[0].rsuRadioType = get_rsuRadioType(1))){
  } else {
      set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
      DEBUGMSGTL((MY_NAME, "rsuRadioType.1 install FAIL.\n"));
  }
  if (0 <= (rsuRadio[1].rsuRadioType = get_rsuRadioType(2))){
  } else {
      set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
      DEBUGMSGTL((MY_NAME, "rsuRadioType.2 install FAIL.\n"));
  }
#if 0 /* Do not make syscall to radio here. Too soon. */
  if(RSEMIB_OK == get_rsuRadioMacAddress1(1,rsuRadio[0].rsuRadioMacAddress1)){
  } else {
      set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
  }
  if(RSEMIB_OK == get_rsuRadioMacAddress1(2,rsuRadio[1].rsuRadioMacAddress1)){
  } else {
      set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
  }
  if(RSEMIB_OK == get_rsuRadioMacAddress2(1,rsuRadio[0].rsuRadioMacAddress2)){
  } else {
      set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
  }
  if(RSEMIB_OK == get_rsuRadioMacAddress2(2,rsuRadio[1].rsuRadioMacAddress2)){
  } else {
      set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
  }
#endif
  if (0 <= (rsuRadio[0].rsuRadioChanMode = get_rsuRadioChanMode(1))){
  } else {
      set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
      DEBUGMSGTL((MY_NAME, "rsuRadioChanMode.1 install FAIL.\n"));
  }
  if (0 <= (rsuRadio[1].rsuRadioChanMode = get_rsuRadioChanMode(2))){
  } else {
      set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
     DEBUGMSGTL((MY_NAME, "rsuRadioChanMode.2 install FAIL.\n"));
  }
#if 0 /* Do not make syscall to radio here. Too soon. */
  if (0 <= (rsuRadio[0].rsuRadioCh1 = get_rsuRadioCh1(1))){
  } else {
      set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
  }
  if (0 <= (rsuRadio[0].rsuRadioCh2 = get_rsuRadioCh2(1))){
  } else {
      set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
  }
  if (0 <= (rsuRadio[1].rsuRadioCh1 = get_rsuRadioCh1(2))){
  } else {
      set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
  }
  if (0 <= (rsuRadio[1].rsuRadioCh2 = get_rsuRadioCh2(2))){
  } else {
      set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
  }
  if (0 <= get_rsuRadioTxPower1(1,&rsuRadio[0].rsuRadioTxPower1)){
  } else {
      set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
  }
  if (0 <= get_rsuRadioTxPower2(1,&rsuRadio[0].rsuRadioTxPower2)){
  } else {
      set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
  }
  if (0 <= get_rsuRadioTxPower1(2,&rsuRadio[1].rsuRadioTxPower1)){
  } else {
      set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
  }
  if (0 <= get_rsuRadioTxPower2(2,&rsuRadio[1].rsuRadioTxPower2)){
  } else {
      set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
  }
#endif
#endif /* 5940 */
  /* Install table created with data from MIB. */
  if(RSEMIB_OK != install_rsuRadioTable(&rsuRadio[0])){
      DEBUGMSGTL((MY_NAME, "rsuRadioTable install FAIL.\n"));
      set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
  }
}
void destroy_rsuRadio(void)
{
  destroy_rsuRadioTable();

  /* Free table info. */
  if (NULL != table_reg_info){
      SNMP_FREE(table_reg_info);
  }
  /* Free table reg. */
  if (NULL != rsuRadioTable){
      SNMP_FREE(rsuRadioTable);
  }
}
