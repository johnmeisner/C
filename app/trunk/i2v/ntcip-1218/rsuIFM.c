/**************************************************************************
 *                                                                        *
 *     File Name:  rsuIFM.c                                               *
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
#include <mqueue.h>
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
#define MY_NAME       "IFM"
#define MY_NAME_EXTRA "XIFM"
/* Mandatory: For debugging and syslog output. */
#define MY_I2V_NAME    MY_NAME
#define MY_ERR_LEVEL   LEVEL_PRIV  /* from i2v_util.h */

/* table column definitions. */
#define IFM_INDEX_COLUMN       1
#define IFM_PSID_COLUMN        2
#define IFM_TX_CHANNEL_COLUMN  3
#define IFM_ENABLE_COLUMN      4
#define IFM_STATUS_COLUMN      5
#define IFM_PRIORITY_COLUMN    6
#define IFM_OPTIONS_COLUMN     7
#define IFM_PAYLOAD_COLUMN     8
 /* Theoretical column ids. Make sure they follow in order with columns and unique. */
#define IFM_MAX_MSG_OID        9

#define IFM_INDEX_NAME       "rsuIFMIndex"
#define IFM_PSID_NAME        "rsuIFMPsid"
#define IFM_TX_CHANNEL_NAME  "rsuIFMTxChannel"
#define IFM_ENABLE_NAME      "rsuIFMEnable"
#define IFM_STATUS_NAME      "rsuIFMtStatus"
#define IFM_PRIORITY_NAME    "rsuIFMPriority"
#define IFM_OPTIONS_NAME     "rsuIFMOptions"
#define IFM_PAYLOAD_NAME     "rsuIFMPayload"
#define IFM_MAX_MSG_NAME     "maxRsuIFMs"

/****************************************************************************** 
 * 5.5 Immediate Forward Messages: { rsu 4 }: rsuIFMStatusTable
 *
 * typedef struct {
 *     int32_t  rsuIFMIndex;                     RO: 1 to 255 IFM messages.
 *     uint8_t  rsuIFMPsid[RSU_RADIO_PSID_SIZE]; RC: 1 to 4 OCTETS:
 *     int32_t  rsuIFMTxChannel;                 RC: RSU_RADIO_CHANNEL_MIN to RSU_RADIO_CHANNEL_MAX.
 *     onOff_e  rsuIFMEnable;                    RC: 0=off, 1= on tx: 
 *     int32_t  rsuIFMStatus;                    RC: Create (4) & (6) destroy
 *     int32_t  rsuIFMPriority;                  RC: (0..63) : DSRC see IEEE 1609.3-2016.  
 *
 *     uint8_t rsuIFMOptions; RC: bitmask: 32 bits
 *                                bit0(0x0 = bypass, use THEIR 1609.2, 0x1 = use OUR 1609.2)
 *                                bit1(0x0 = secure(Sign or Encrypt), 0x1 = unsecure) 
 *                                bit2(0x0 = continue tx, 0x1 = stop when > rsuShortCommLossTime: default 0)
 *                                bit3(0x0 = continue tx, 0x1 = stop when > rsuLongtCommLossTime: default 0)
 *
 *    uint8_t rsuIFMPayload[RSU_MSG_PAYLOAD_MAX]; RC: 0..2302 OCTETS:IEEE1609dot3-MIB. UPER MessageFrame.
 *
 * } RsuIFMStatusEntry_t;
 *
 ******************************************************************************/

const oid maxRsuIFMs_oid[]        = {1, 3, 6, 1, 4, 1, 1206, 4, 2, 18, 4, 1};
const oid rsuIFMStatusTable_oid[] = {1, 3, 6, 1, 4, 1, 1206, 4, 2, 18, 4, 2};

static netsnmp_table_registration_info * table_reg_info = NULL; /* snmp table reg pointer. */
static netsnmp_tdata * rsuIFMStatusTable                = NULL; /* snmp table pointer. */
static uint32_t rsuIFM_error_states                     = 0x0;  /* local error state. */

static mqd_t ifmQueue;

/*
 * Protos: Silence of the Warnings.
 */
static void rebuild_rsuIFM(void);
static void destroy_rsuIFMStatusTable(void);

/* 
 * Function Bodies.
 */
static void set_error_states(int32_t error)
{
  /* -1 to -32 */
  if((error < 0) && (error > MIB_ERROR_BASE_2)){
      rsuIFM_error_states |= (uint32_t)(0x1) << (abs(error) - 1);
  }
}
uint32_t get_rsuIFM_error_states(void)
{
  return rsuIFM_error_states;
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
static void print_request_result(char_t * user_name, uint32_t row_index, uint32_t column, netsnmp_request_info * requests, int32_t mode, int32_t ret)
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
  int32_t do_trap = 0;
  char_t alert_msg[RSU_ALERT_MSG_LENGTH_MAX];

  memset(mode_name,0,sizeof(mode_name));
  memset(column_name,0,sizeof(column_name));
  memset(passOrFail,0,sizeof(passOrFail));
  memset(alert_msg,0,sizeof(alert_msg));

  if((MODE_SET_COMMIT != mode) && (RSEMIB_OK == ret)) {
      return; /* Dont bother with AOK GETS. Just noise. */
  }
  if(NULL != user_name){
      /* from types.h
       * 256 == SNMP_MAX_SEC_NAME_SIZE
       * char staticusername[256];
       */
      if(SNMP_MAX_SEC_NAME_SIZE <= strlen(user_name)){
          user_name[SNMP_MAX_SEC_NAME_SIZE-1] = '\0';
      }
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"securityName[%s]\n",user_name);
  } else {
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"securityName[unknown]\n");
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
      case IFM_INDEX_COLUMN:
          strcpy(column_name,IFM_INDEX_NAME);
          break;
      case IFM_PSID_COLUMN:
          strcpy(column_name,IFM_PSID_NAME);
          break;
      case IFM_TX_CHANNEL_COLUMN:
          strcpy(column_name,IFM_TX_CHANNEL_NAME);
          break;
      case IFM_ENABLE_COLUMN:
          strcpy(column_name,IFM_ENABLE_NAME);
          break;
      case IFM_STATUS_COLUMN:
          strcpy(column_name,IFM_STATUS_NAME);
          break;
      case IFM_PRIORITY_COLUMN:
          strcpy(column_name,IFM_PRIORITY_NAME);
          break;
      case IFM_OPTIONS_COLUMN:
          strcpy(column_name,IFM_OPTIONS_NAME);
          break;
      case IFM_PAYLOAD_COLUMN:
          strcpy(column_name,IFM_PAYLOAD_NAME);
          break;
/* For scalars and like kind. */
      case IFM_MAX_MSG_OID:
          strcpy(column_name,IFM_MAX_MSG_NAME);
          break;
      default:
          strcpy(column_name,"UNKOWN");
          break;
  }
  if(RSEMIB_OK == ret) {
      strcpy(passOrFail,"succeeded");
      do_trap = 0;
  } else {
      strcpy(passOrFail,"failed");
      do_trap = 1;
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
  if(1 == do_trap) {
      set_messageFileIntegrity(rsuAlertLevel_error,(uint8_t *)alert_msg,RSU_ALERT_MSG_LENGTH_MAX);
  }
}
/* Queue functions for back end app support of IFM in AMH. */
static int32_t open_IFM_queue(void)
{
  struct mq_attr attr;

  /* If the queue exists then unlink to remove. */
  ifmQueue = mq_open(IFM_MIB_QUEUE, O_RDWR);
  mq_unlink(IFM_MIB_QUEUE); /* unlink to signfy delete. */
  mq_close(ifmQueue);

  /* If queue doesn't exist then we may get error on above: errno = 9(Bad file descriptor). Ignore. */
  if(9 != errno){      
      DEBUGMSGTL((MY_NAME,"open_IFM_queue: errno = %d(%s)\n",errno,strerror(errno)));
      errno = 0;
  }
  attr.mq_flags     = IFM_QUEUE_DEFAULT_FLAGS; /* ignored on open, use with set */
  attr.mq_maxmsg    = IFM_QUEUE_DEPTH; 
  attr.mq_msgsize   = IFM_QUEUE_MSG_SIZE;
  attr.mq_curmsgs   = 0;
  /* Can't be blocking. MIB client can't wait. */
  if(-1 == (ifmQueue = mq_open(IFM_MIB_QUEUE, (O_RDWR|O_CREAT|O_NONBLOCK), 0644, &attr))){
      DEBUGMSGTL((MY_NAME,"open_IFM_queue: mq_open() failed: errno = %d(%s)\n",errno,strerror(errno)));
      return RSEMIB_MQ_FAIL;   
  }
  attr.mq_flags   = 0;
  attr.mq_maxmsg  = 0;
  attr.mq_msgsize = 0;
  attr.mq_curmsgs = 0;
  if(-1 == mq_getattr(ifmQueue,&attr) ) {
      DEBUGMSGTL((MY_NAME,"open_IFM_queue: mq_getattr() failed: errno = %d(%s)\n",errno,strerror(errno)));
      return RSEMIB_MQ_FAIL;
  }
  /* Check return values against our programmed values */
  if( (attr.mq_maxmsg != IFM_QUEUE_DEPTH) || (attr.mq_msgsize != IFM_QUEUE_MSG_SIZE)){
      DEBUGMSGTL((MY_NAME,"open_IFM_queue: Queue attributes wrong: errno = %d(%s)\n",errno,strerror(errno)));
      return RSEMIB_MQ_FAIL;
  }
  return RSEMIB_OK;
}
/* Send data to AMH for radio tx. */
int32_t ifm_enqueue(const char_t * data_in)
{
  if(NULL == data_in) {
      set_error_states(RSEMIB_ENQUEUE_FAIL);
      return RSEMIB_ENQUEUE_FAIL;    
  }
  if(-1 == mq_send(ifmQueue, data_in, IFM_QUEUE_MSG_SIZE, DEFAULT_MQ_PRIORITY)) {
      DEBUGMSGTL((MY_NAME,"ifm_enqueue: mq_send fail: errno = %d(%s)\n",errno,strerror(errno)));
      set_error_states(RSEMIB_ENQUEUE_FAIL);
      return RSEMIB_ENQUEUE_FAIL;
  }
  return RSEMIB_OK;
}

/*
 * OID Handlers
 */

/* 5.5.1 */
static int32_t handle_maxRsuIFMs(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo,
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
              if(0 <= (ret = get_maxRsuIFMs())){
                  snmp_set_var_typed_integer(requests->requestvb, ASN_INTEGER, ret);
                  ret = RSEMIB_OK;
              }
              break;
          default:
              ret = RSEMIB_BAD_MODE;
              break;
      }
#if 0 /* Not supported in net-snmp 5.8 */
      /* scalars and like are always index 0 with unqiue column.*/
      print_request_result(reqinfo->asp->pdu->staticusername,0, IFM_MAX_MSG_OID,requests,reqinfo->mode,ret);
#else
      print_request_result("rsutweiadmin",0, IFM_MAX_MSG_OID,requests,reqinfo->mode,ret);
#endif
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
/* 5.5.2.1: Although stored, we already know the 'index'. Already ranged checked beforehand. */
static int32_t handle_rsuIFMIndex(int32_t index, netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret  = RSEMIB_OK;
  int32_t data = index;

  DEBUGMSGTL((MY_NAME, "%s: Entry:\n",__FUNCTION__));
  switch (reqinfo->mode){
      case MODE_GET:
#if 0 /* Enable if you want to bother getting copy. */
          if(RSEMIB_OK == (ret = get_rsuIFMIndex(index, &data)))
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
/* 5.5.2.2 */
static int32_t handle_rsuIFMPsid(int32_t index, netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret = RSEMIB_OK;
  uint8_t data[RSU_RADIO_PSID_SIZE];

  DEBUGMSGTL((MY_NAME, "%s: Entry:\n",__FUNCTION__));
  switch (reqinfo->mode){
      case MODE_GET:
          DEBUGMSGTL((MY_NAME, "GET:\n"));
          memset(data,0x0,sizeof(data));
          if(0 <= (ret = get_rsuIFMPsid(index,data))){
              snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR, data, ret);
              ret = RSEMIB_OK;
          }
          break;
      case MODE_SET_RESERVE1: /* Short stack, all requests done in one. */
          DEBUGMSGTL((MY_NAME, "R1:\n"));
          if(RSEMIB_OK == (ret = preserve_rsuIFMPsid(index))){
              if(RSEMIB_OK == (ret = action_rsuIFMPsid(index,requests->requestvb->val.string,requests->requestvb->val_len))){
                  if(RSEMIB_OK == (ret = commit_rsuIFMPsid(index))){
                      DEBUGMSGTL((MY_NAME, "COMMIT DONE:\n"));
                  } else {
                      DEBUGMSGTL((MY_NAME, "COMMIT FAIL:\n"));
                  }
              } else {
                  DEBUGMSGTL((MY_NAME, "ACTION FAIL:\n"));
              }
              if(RSEMIB_OK != ret) {
                  if(RSEMIB_OK != undo_rsuIFMPsid(index)) { /* UNDO fails nothing we can do. */
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
/* 5.5.2.3 */
static int32_t handle_rsuIFMTxChannel(int32_t index,netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret  = RSEMIB_OK;
  int32_t data = 0;

  DEBUGMSGTL((MY_NAME, "%s: Entry:\n",__FUNCTION__));
  switch (reqinfo->mode){
      case MODE_GET:
          DEBUGMSGTL((MY_NAME, "GET:\n"));
          if(RSEMIB_OK == (ret = get_rsuIFMTxChannel(index,&data))){
              snmp_set_var_typed_integer(requests->requestvb, ASN_INTEGER, data);
          }
          break;
      case MODE_SET_RESERVE1: /* Short stack, all requests done in one. */
          DEBUGMSGTL((MY_NAME, "R1:TxChannel=%lu.\n",*(requests->requestvb->val.integer)));
          if(RSEMIB_OK == (ret = preserve_rsuIFMTxChannel(index))){
              if(RSEMIB_OK == (ret = action_rsuIFMTxChannel(index,*(requests->requestvb->val.integer)))){
                  if(RSEMIB_OK == (ret = commit_rsuIFMTxChannel(index))){
                      DEBUGMSGTL((MY_NAME, "COMMIT DONE:\n"));
                  } else {
                      DEBUGMSGTL((MY_NAME, "COMMIT FAIL:\n"));
                  }
              } else {
                  DEBUGMSGTL((MY_NAME, "ACTION FAIL:\n"));
              }
              if(RSEMIB_OK != ret) {
                  if(RSEMIB_OK != undo_rsuIFMTxChannel(index)) { /* UNDO fails nothing we can do. */
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
/* 5.5.2.4 */
static int32_t handle_rsuIFMEnable(int32_t index,netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret  = RSEMIB_OK;
  int32_t data = 0;

  DEBUGMSGTL((MY_NAME, "%s: Entry:\n",__FUNCTION__));
  switch (reqinfo->mode){
      case MODE_GET:
          DEBUGMSGTL((MY_NAME, "GET:\n"));
          if(RSEMIB_OK == (ret = get_rsuIFMEnable(index,&data))){
              snmp_set_var_typed_integer(requests->requestvb, ASN_INTEGER, data);
          }
          break;
      case MODE_SET_RESERVE1: /* Short stack, all requests done in one. */
          DEBUGMSGTL((MY_NAME, "R1:\n"));
          if(RSEMIB_OK == (ret = preserve_rsuIFMEnable(index))){
              if(RSEMIB_OK == (ret = action_rsuIFMEnable(index,*(requests->requestvb->val.integer)))){
                  if(RSEMIB_OK == (ret = commit_rsuIFMEnable(index))){
                      DEBUGMSGTL((MY_NAME, "COMMIT DONE:\n"));
                  } else {
                      DEBUGMSGTL((MY_NAME, "COMMIT FAIL:\n"));
                  }
              } else {
                  DEBUGMSGTL((MY_NAME, "ACTION FAIL:\n"));
              }
              if(RSEMIB_OK != ret) {
                  if(RSEMIB_OK != undo_rsuIFMEnable(index)) { /* UNDO fails nothing we can do. */
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
/* 5.5.2.5 */
static int32_t handle_rsuIFMStatus(int32_t index,netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests, int32_t override)
{
  int32_t ret  = RSEMIB_OK;
  int32_t data = 0;
  int32_t row_status = SNMP_ROW_CREATEANDGO; /* If override is set then don't use requests's value. Use this one. */

  DEBUGMSGTL((MY_NAME, "%s: Entry:\n",__FUNCTION__));
  switch (reqinfo->mode){
      case MODE_GET:
          if(RSEMIB_OK == (ret = get_rsuIFMStatus(index,&data))){
              snmp_set_var_typed_integer(requests->requestvb, ASN_INTEGER, data);
          }
          break;
      case MODE_SET_RESERVE1: /* Short minion stack, all requests done in one. */
          if(0 == override){
              row_status = *(requests->requestvb->val.integer); /* Client index part of CREATE. */
          }
          DEBUGMSGTL((MY_NAME, "R1_BULK_SET: status = %d.\n",row_status));
          if(RSEMIB_OK == (ret = preserve_rsuIFMStatus(index))){
              if(RSEMIB_OK == (ret = action_rsuIFMStatus(index,row_status))){
                  if(RSEMIB_OK == (ret = commit_rsuIFMStatus(index))){
                      DEBUGMSGTL((MY_NAME, "COMMIT DONE:\n")); /* Minions unite, master will be happy! */
                  } else {
                      DEBUGMSGTL((MY_NAME, "COMMIT FAIL:\n"));
                  }
              } else {
                  DEBUGMSGTL((MY_NAME, "ACTION FAIL:\n"));
              }
              if(RSEMIB_OK != ret) { /* UNDO fails nothing we can do. */
                  if(RSEMIB_OK != undo_rsuIFMStatus(index)) {
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
/* 5.5.2.6 */
static int32_t handle_rsuIFMPriority(int32_t index,netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret  = RSEMIB_OK;
  int32_t data = 0;

  DEBUGMSGTL((MY_NAME, "%s: Entry:\n",__FUNCTION__));
  switch (reqinfo->mode){
      case MODE_GET:
          DEBUGMSGTL((MY_NAME, "GET:\n"));
          if(RSEMIB_OK == (ret = get_rsuIFMPriority(index,&data))){
              snmp_set_var_typed_integer(requests->requestvb, ASN_INTEGER, data);
          }
          break;
      case MODE_SET_RESERVE1: /* Short stack, all requests done in one. */
          DEBUGMSGTL((MY_NAME, "R1:\n"));
          if(RSEMIB_OK == (ret = preserve_rsuIFMPriority(index))){
              if(RSEMIB_OK == (ret = action_rsuIFMPriority(index,*(requests->requestvb->val.integer)))) {
                  if(RSEMIB_OK == (ret = commit_rsuIFMPriority(index))){
                      DEBUGMSGTL((MY_NAME, "COMMIT DONE:\n"));
                  } else {
                      DEBUGMSGTL((MY_NAME, "COMMIT FAIL:\n"));
                  }
              } else {
                  DEBUGMSGTL((MY_NAME, "ACTION FAIL:\n"));
              }
              if(RSEMIB_OK != ret) {
                  if(RSEMIB_OK != undo_rsuIFMPriority(index)) { /* UNDO fails nothing we can do. */
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
/* 5.5.2.7 */
static int32_t handle_rsuIFMOptions(int32_t index,netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret = RSEMIB_OK;
  uint8_t data[RSU_MSG_REPEAT_OPTIONS_LENGTH];

  DEBUGMSGTL((MY_NAME, "%s: Entry:\n",__FUNCTION__));
  switch (reqinfo->mode){
      case MODE_GET:
          DEBUGMSGTL((MY_NAME, "GET:\n"));
          if(RSU_MSG_REPEAT_OPTIONS_LENGTH == (ret = get_rsuIFMOptions(index,data))){
#if defined(ENABLE_ASN_BIT_STR)
              snmp_set_var_typed_value(requests->requestvb, ASN_BIT_STR, data, RSU_MSG_REPEAT_OPTIONS_LENGTH);
#else
              snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR, data, RSU_MSG_REPEAT_OPTIONS_LENGTH);
#endif
              ret = RSEMIB_OK;
          }
          break;
      case MODE_SET_RESERVE1: /* Short stack, all requests done in one. */
          DEBUGMSGTL((MY_NAME, "R1:\n"));
          if(RSEMIB_OK == (ret = preserve_rsuIFMOptions(index))){
              if(RSEMIB_OK == (ret = action_rsuIFMOptions(index,requests->requestvb->val.bitstring,requests->requestvb->val_len))){
                  if(RSEMIB_OK == (ret = commit_rsuIFMOptions(index))){
                      DEBUGMSGTL((MY_NAME, "COMMIT DONE:\n"));
                  } else {
                      DEBUGMSGTL((MY_NAME, "COMMIT FAIL:\n"));
                  }
              } else {
                  DEBUGMSGTL((MY_NAME, "ACTION FAIL:\n"));
              }
              if(RSEMIB_OK != ret) {
                  if(RSEMIB_OK != undo_rsuIFMOptions(index)) { /* UNDO fails nothing we can do. */
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
/* 5.5.2.8 */
static int32_t handle_rsuIFMPayload(int32_t index,netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret = RSEMIB_OK;
  uint8_t data[RSU_MSG_PAYLOAD_MAX];

  DEBUGMSGTL((MY_NAME, "%s: Entry:\n",__FUNCTION__));
  switch (reqinfo->mode){
      case MODE_GET:
          DEBUGMSGTL((MY_NAME, "GET:\n"));
          memset(data,0x0,sizeof(data));
          if(0 <= (ret = get_rsuIFMPayload(index,data))){
              snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR, data, ret);
              ret = RSEMIB_OK;
          }
          break;
      case MODE_SET_RESERVE1: /* Short stack, all requests done in one. */
          DEBUGMSGTL((MY_NAME, "R1:\n"));
          if(RSEMIB_OK == (ret = preserve_rsuIFMPayload(index))){
              if(RSEMIB_OK == (ret = action_rsuIFMPayload(index,requests->requestvb->val.string,requests->requestvb->val_len))){
                  if(RSEMIB_OK == (ret = commit_rsuIFMPayload(index))){
                      DEBUGMSGTL((MY_NAME, "COMMIT DONE:\n"));
                  } else {
                      DEBUGMSGTL((MY_NAME, "COMMIT FAIL:\n"));
                  }
              } else {
                  DEBUGMSGTL((MY_NAME, "ACTION FAIL:\n"));
              }
              if(RSEMIB_OK != ret) {
                  if(RSEMIB_OK != undo_rsuIFMPayload(index)) { /* UNDO fails nothing we can do. */
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
#if 0
rsuIFM: request_info is good: COLUMN=7.
rsuIFM: name_loc=[0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x0].
rsuIFM: name_loc=[0x3,0x0,0x0,0x0,0x0,0x0,0x0,0x0].
rsuIFM: name_loc=[0x6,0x0,0x0,0x0,0x0,0x0,0x0,0x0].
rsuIFM: name_loc=[0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x0].
rsuIFM: name_loc=[0x4,0x0,0x0,0x0,0x0,0x0,0x0,0x0].
rsuIFM: name_loc=[0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x0].
rsuIFM: name_loc=[0xb6,0x4,0x0,0x0,0x0,0x0,0x0,0x0].
rsuIFM: name_loc=[0x4,0x0,0x0,0x0,0x0,0x0,0x0,0x0].
rsuIFM: name_loc=[0x2,0x0,0x0,0x0,0x0,0x0,0x0,0x0].
rsuIFM: name_loc=[0x12,0x0,0x0,0x0,0x0,0x0,0x0,0x0].
rsuIFM: name_loc=[0x4,0x0,0x0,0x0,0x0,0x0,0x0,0x0].
rsuIFM: name_loc=[0x2,0x0,0x0,0x0,0x0,0x0,0x0,0x0].
rsuIFM: name_loc=[0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x0]. // index length?
rsuIFM: name_loc=[0x7,0x0,0x0,0x0,0x0,0x0,0x0,0x0]. // colunm
rsuIFM: name_loc=[0x45,0x0,0x0,0x0,0x0,0x0,0x0,0x0].// row index
rsuIFM: name_loc=[0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0]. // 2nd byte of index?
XrsuIFM: raw oid index = 69.
rsuIFM: GO for handler ROW=69 COLUMN=7.
rsuIFM: handle_rsuIFMOptions_69: Entry:
#endif
#if defined(EXTRA_DEBUG) 
static void print_rawOID(uint8_t * raw_oid)
{
   int32_t i;

   if (NULL == raw_oid)
       return;

   for(i=0; i < MAX_OID_LEN;i = i + 8){
       DEBUGMSGTL((MY_NAME_EXTRA, "name_loc=[0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x]\n",
                  raw_oid[i+0], raw_oid[i+1], raw_oid[i+2], raw_oid[i+3]
                , raw_oid[i+4], raw_oid[i+5], raw_oid[i+6], raw_oid[i+7]));
   }
   DEBUGMSGTL((MY_NAME_EXTRA,"raw oid: row index = %u [%u,|%u,%u,%u,%u|%u] column = %d.\n",row_index, raw_oid[111],raw_oid[112],raw_oid[113],raw_oid[114],raw_oid[115],raw_oid[116],raw_oid[104]));
}
#endif
/* 
 * 5.5.2: Table handler: Break it down to smaller requests.
 */
int32_t handle_rsuIFMStatusTable( netsnmp_mib_handler          * handler
                                , netsnmp_handler_registration * reginfo
                                , netsnmp_agent_request_info   * reqinfo
                                , netsnmp_request_info         * requests)
{
  netsnmp_request_info       * request       = NULL;
  uint32_t                     request_count = 0;
  RsuIFMStatusEntry_t        * table_entry   = NULL;
  netsnmp_table_request_info * request_info  = NULL;
  int32_t                      ret           = SNMP_ERR_NOERROR;
  uint8_t                      raw_oid[MAX_OID_LEN + 2]; /* If table doesn't exist then helper can't help. Go fish for row index. */
  uint32_t                     row_index = 0; /* For readability. Note: index is 1..max */
  int32_t                      row_status = SNMP_ROW_NONEXISTENT;

  handler = handler; /* Silence of the Warnings. */
  reginfo = reginfo;

  DEBUGMSGTL((MY_NAME, "rsuIFMStatusTable_handler: Entry:\n"));
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
      if(NULL != (table_entry = (RsuIFMStatusEntry_t *)netsnmp_tdata_extract_entry(request))) {
          if((request_info->colnum < IFM_INDEX_COLUMN) || (IFM_PAYLOAD_COLUMN < request_info->colnum)) {
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
      print_rawOID(&raw_oid[0]);
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"raw oid: row index = %u [%u|%u,%u,%u,%u|%u] column = %d.\n",row_index, raw_oid[111],raw_oid[112],raw_oid[113],raw_oid[114],raw_oid[115],raw_oid[116],raw_oid[104]);
      #endif

      /* Do get() on rowStatus to see if it exists and error check row_index. */
      if(RSEMIB_OK != (ret = get_rsuIFMStatus(row_index,&row_status))) {
          DEBUGMSGTL((MY_NAME, "get_rsuIFMStatus Failed: ret=%d\n",ret));
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
          if(IFM_STATUS_COLUMN != request_info->colnum){/* If row doesn't exist then create now. */
              ret = handle_rsuIFMStatus(row_index, reqinfo, request, 1); /* Do not use value in request. Force createAndGo. */
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
          case IFM_INDEX_COLUMN:
              ret = handle_rsuIFMIndex(row_index, reqinfo, request);
              break;
          case IFM_PSID_COLUMN:
              ret = handle_rsuIFMPsid(row_index, reqinfo, request);
              break;
          case IFM_TX_CHANNEL_COLUMN:
              ret = handle_rsuIFMTxChannel(row_index, reqinfo, request);
              break;
          case IFM_ENABLE_COLUMN:
              ret = handle_rsuIFMEnable(row_index, reqinfo, request);
              break;
          case IFM_STATUS_COLUMN:
//If already created and status matches then who cares?
              ret = handle_rsuIFMStatus(row_index, reqinfo, request, 0);
              break;
          case IFM_PRIORITY_COLUMN:
              ret = handle_rsuIFMPriority(row_index, reqinfo, request);
              break;
          case IFM_OPTIONS_COLUMN:
              ret = handle_rsuIFMOptions(row_index, reqinfo, request);
              break;
          case IFM_PAYLOAD_COLUMN:
              ret = handle_rsuIFMPayload(row_index, reqinfo, request);
              break;
          default:
              ret = RSEMIB_BAD_REQUEST_COLUMN; /* Should never happen. */
              break;
      }
      if(ret < RSEMIB_OK) {
          set_error_states(ret);
      }
#if 0 /* Not supported in net-snmp 2.8 */
      print_request_result(reqinfo->asp->pdu->staticusername, row_index, request_info->colnum,requests,reqinfo->mode,ret);
#else
      print_request_result("staticusername", row_index, request_info->colnum,requests,reqinfo->mode,ret);

#endif
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

/* rsuIFMStatusTable creation functions.*/
STATIC void rsuIFMStatusTable_removeEntry(netsnmp_tdata * table_data, netsnmp_tdata_row * row)
{
  RsuIFMStatusEntry_t * entry = NULL;

  if (NULL == row){
      DEBUGMSGTL((MY_NAME,"rsuIFMStatusTable_removeEntry NULL input.\n"));
      return;
  }
  if (NULL != (entry = (RsuIFMStatusEntry_t *)row->data)){ /* We allocate, so we free. */
      SNMP_FREE(entry);
  } else {
      DEBUGMSGTL((MY_NAME,"rsuIFMStatusTable_removeEntry row data is NULL!.\n"));
  }
  if (table_data){
      netsnmp_tdata_remove_and_delete_row(table_data, row);
  } else {
      DEBUGMSGTL((MY_NAME,"rsuIFMStatusTable_removeEntry NULL input.\n"));
      netsnmp_tdata_delete_row(row); 
  }
  return;
}
static void destroy_rsuIFMStatusTable(void)
{
  netsnmp_tdata_row  * row     = NULL;
  netsnmp_tdata_row  * nextrow = NULL;
  int32_t              i       = 0;

  if(NULL != rsuIFMStatusTable){
      if(NULL != (row = netsnmp_tdata_row_first(rsuIFMStatusTable))){
          for(i=0;(row != NULL) && (i < RSU_IFM_MESSAGE_MAX);i++){
              nextrow = netsnmp_tdata_row_next(rsuIFMStatusTable, row);
              rsuIFMStatusTable_removeEntry(rsuIFMStatusTable, row);
              row = nextrow;
          }
      } else {
          DEBUGMSGTL((MY_NAME, "destroy_rsuIFMStatusTable: Table empty. Nothing to do.\n"));
      }
  } else {
      DEBUGMSGTL((MY_NAME, "destroy_rsuIFMStatusTable: Table is NULL. Did we get called before install?\n"));
  }
}
static int32_t rsuIFM_createTable(netsnmp_tdata * table_data, RsuIFMStatusEntry_t * rsuIFM)
{
  RsuIFMStatusEntry_t * entry    = NULL;
  netsnmp_tdata_row   * row      = NULL;
  uint8_t               i        = 0;
  int32_t               valid_row_count = 0;

  if(NULL == table_data){
      DEBUGMSGTL((MY_NAME, "rsuIFM_createTable FAIL: table_data == NULL.\n"));
      return -1;
  }

  for(i=0; i < RSU_IFM_MESSAGE_MAX; i++){
      if((rsuIFM[i].rsuIFMIndex < 1) || (RSU_IFM_MESSAGE_MAX < rsuIFM[i].rsuIFMIndex)){
          handle_rsemib_errors(RSEMIB_BAD_INDEX);
          return -1; /* Indices must be unique and greater than 1. */
      }
      if(RSEMIB_OK != valid_table_row(rsuIFM[i].rsuIFMStatus)){
          continue; /* Only add rows that have been created. */
      }
      if (NULL == (entry = SNMP_MALLOC_TYPEDEF(RsuIFMStatusEntry_t))){
          DEBUGMSGTL((MY_NAME, "rsuIFM SNMP_MALLOC_TYPEDEF FAIL.\n"));
          handle_rsemib_errors(RSEMIB_BAD_MODE);
          return -1;
      }
      if (NULL == (row = netsnmp_tdata_create_row())){
          DEBUGMSGTL((MY_NAME, "rsuIFM netsnmp_tdata_create_row FAIL.\n"));
          SNMP_FREE(entry);
          handle_rsemib_errors(RSEMIB_BAD_MODE);
          return -1;
      }

      DEBUGMSGTL((MY_NAME_EXTRA, "rsuIFM make row i=%d index=%d.\n",i,rsuIFM[i].rsuIFMIndex));
      row->data = entry;
      memcpy(entry,&rsuIFM[i],sizeof(RsuIFMStatusEntry_t));
      /* Trust but verify. If incorrect fix and note for us.*/
      if(i+1 != rsuIFM[i].rsuIFMIndex) {
          DEBUGMSGTL((MY_NAME, "rsuIFM bad rsuIFMIndex=%d.\n",rsuIFM[i].rsuIFMIndex));
          rsuIFM[i].rsuIFMIndex = i + 1;
          set_error_states(RSEMIB_BAD_INDEX);
      }
      netsnmp_tdata_row_add_index(row, ASN_INTEGER , &rsuIFM[i].rsuIFMIndex, sizeof(int32_t));
      if(SNMPERR_SUCCESS != netsnmp_tdata_add_row(table_data, row)){
          DEBUGMSGTL((MY_NAME, "rsuIFM netsnmp_tdata_add_row FAIL i=%d.\n",i));
          handle_rsemib_errors(RSEMIB_BAD_MODE);
          return -1;
      }
      valid_row_count++;
  } /* for */
  DEBUGMSGTL((MY_NAME, "rsuIFM valid rows created = %d.\n",valid_row_count));
  return RSEMIB_OK;
}
static int32_t install_rsuIFMStatusTable(void)
{
    netsnmp_handler_registration * reg = NULL;

    reg = netsnmp_create_handler_registration("rsuIFMStatusTable"
                                              ,handle_rsuIFMStatusTable
                                              ,rsuIFMStatusTable_oid
                                              ,OID_LENGTH(rsuIFMStatusTable_oid)
                                              ,HANDLER_CAN_RWRITE);

    if (NULL == (rsuIFMStatusTable = netsnmp_tdata_create_table("rsuIFMStatusTable", 0))){
        DEBUGMSGTL((MY_NAME, "rsuIFMStatusTable FAILED.\n"));
        return RSEMIB_INSTALL_HANDLER_FAIL;
    }
    if (NULL == (table_reg_info = SNMP_MALLOC_TYPEDEF(netsnmp_table_registration_info))){
        DEBUGMSGTL((MY_NAME, "SNMP_MALLOC_TYPEDEF FAILED.\n"));
        return RSEMIB_INSTALL_HANDLER_FAIL;
    }
    netsnmp_table_helper_add_indexes(table_reg_info, ASN_INTEGER,0); /* index */                                 

    table_reg_info->min_column = IFM_PSID_COLUMN;    /* rsuIFMPsid: Hide index by definition. */
    table_reg_info->max_column = IFM_PAYLOAD_COLUMN; /* rsuIFMPayload: */

    if (SNMPERR_SUCCESS == netsnmp_tdata_register(reg, rsuIFMStatusTable, table_reg_info)){
        return RSEMIB_OK; /* Table registered but still empty. rebuild_MsgRepeat() will populate */
    } else {
        DEBUGMSGTL((MY_NAME, "netsnmp_tdata_register FAIL.\n"));
        return RSEMIB_INSTALL_HANDLER_FAIL;
    }
}
/* TODO: Likely overkill and not needed. Will use lock. */
static void rebuild_rsuIFM(void)
{
  int32_t i = 0;
  RsuIFMStatusEntry_t rsuIFM[RSU_IFM_MESSAGE_MAX];

  memset(rsuIFM, 0x0, sizeof(rsuIFM));
  for(i=0; i < RSU_IFM_MESSAGE_MAX; i++){
      if(RSEMIB_OK > get_rsuIFMIndex(i+1,&rsuIFM[i].rsuIFMIndex)){
          set_error_states(RSEMIB_REBUILD_FAIL);
          DEBUGMSGTL((MY_NAME, "rsuIFMIndex rebuild FAIL: i = %d.\n",i));
      }
      if(RSEMIB_OK > get_rsuIFMPsid(i+1,rsuIFM[i].rsuIFMPsid)){
          set_error_states(RSEMIB_REBUILD_FAIL);
          DEBUGMSGTL((MY_NAME, "rsuIFMPsid rebuild FAIL: i = %d.\n",i));
      }
      if(RSEMIB_OK > get_rsuIFMTxChannel(i+1,&rsuIFM[i].rsuIFMTxChannel)){
          set_error_states(RSEMIB_REBUILD_FAIL);
          DEBUGMSGTL((MY_NAME, "rsuIFMTxChannel rebuild FAIL: i = %d.\n",i));
      }
      if(RSEMIB_OK > get_rsuIFMEnable(i+1,(int32_t *)&rsuIFM[i].rsuIFMEnable)){
          set_error_states(RSEMIB_REBUILD_FAIL);
          DEBUGMSGTL((MY_NAME, "rsuIFMEnable rebuild FAIL: i = %d.\n",i));
      }
      if(RSEMIB_OK > get_rsuIFMStatus(i+1,&rsuIFM[i].rsuIFMStatus)){
          set_error_states(RSEMIB_REBUILD_FAIL);
          DEBUGMSGTL((MY_NAME, "rsuIFMStatus rebuild FAIL: i = %d.\n",i));
      }
      if(RSEMIB_OK > get_rsuIFMPriority(i+1,&rsuIFM[i].rsuIFMPriority)){
          set_error_states(RSEMIB_REBUILD_FAIL);
          DEBUGMSGTL((MY_NAME, "rsuIFMPriority rebuild FAIL: i = %d.\n",i));
      }
      if(RSEMIB_OK > get_rsuIFMOptions(i+1,&rsuIFM[i].rsuIFMOptions)){
          set_error_states(RSEMIB_REBUILD_FAIL);
          DEBUGMSGTL((MY_NAME, "rsuIFMOptions rebuild FAIL: i = %d.\n",i));
      }
      if(RSEMIB_OK > get_rsuIFMPayload(i+1,rsuIFM[i].rsuIFMPayload)){
          set_error_states(RSEMIB_REBUILD_FAIL);
          DEBUGMSGTL((MY_NAME, "rsuIFMPayload rebuild FAIL: i = %d.\n",i));
      }
  }
  /* Remove rows but don't remove table reg. */
  destroy_rsuIFMStatusTable();

  /* Initialize the contents of the table here.*/
  if (RSEMIB_OK != rsuIFM_createTable(rsuIFMStatusTable, &rsuIFM[0])){
      DEBUGMSGTL((MY_NAME, "rebuild_rsuIFM: ERROR: Null row.\n"));
      set_error_states(RSEMIB_REBUILD_FAIL);
  }
}
/* Assumed locked already. Live pointer to db. */
int32_t rebuild_rsuIFM_live(RsuIFMStatusEntry_t * IFM)
{
  if(NULL == IFM) {
      return RSEMIB_BAD_INPUT;
  }
  /* Remove rows but don't remove table reg. */
  destroy_rsuIFMStatusTable();

  /* Initialise the contents of the table here.*/
  if (RSEMIB_OK != rsuIFM_createTable(rsuIFMStatusTable, IFM)){
      DEBUGMSGTL((MY_NAME, "rebuild_rsuIFM: ERROR: Null row.\n"));
      set_error_states(RSEMIB_REBUILD_FAIL);
      return RSEMIB_REBUILD_FAIL;
  }
  return RSEMIB_OK;
}
void install_rsuIFM_handlers(void)
{
  /* Reset statics across soft reset. */
  rsuIFMStatusTable   = NULL;
  table_reg_info      = NULL;
  rsuIFM_error_states = 0x0;

  /* 5.5.1 */
  if(MIB_REGISTERED_OK != netsnmp_register_scalar(netsnmp_create_handler_registration
                          ("maxRsuIFMs", 
                           handle_maxRsuIFMs,
                           maxRsuIFMs_oid,
                           OID_LENGTH(maxRsuIFMs_oid),
                           HANDLER_CAN_RONLY))){
      DEBUGMSGTL((MY_NAME, "maxRsuIFMs install FAIL.\n"));
      set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
  }

  /* 5.5.2.1 - 5.5.2.8: Install table created with data from MIB. */
  if(RSEMIB_OK != install_rsuIFMStatusTable()){
      DEBUGMSGTL((MY_NAME, "rsuIFMStatusTable install FAIL.\n"));
      set_error_states(RSEMIB_INSTALL_HANDLER_FAIL);
  }

  /* Populate the table. */
  rebuild_rsuIFM();

  if(RSEMIB_OK != open_IFM_queue()){ /* Open mq with IMF. */
      DEBUGMSGTL((MY_NAME, "open_IFM_queue FAIL.\n"));
      set_error_states(RSEMIB_MQ_FAIL);
  }
}
void destroy_rsuIFM(void)
{
  mq_close(ifmQueue);

  destroy_rsuIFMStatusTable();

  /* Free table info. */
  if (NULL != table_reg_info){
      SNMP_FREE(table_reg_info);
  }
  /* Free table reg. */
  if (NULL != rsuIFMStatusTable){
      SNMP_FREE(rsuIFMStatusTable);
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
/* Write IFM to disk but we don't need to worry about payload. */
int32_t commit_IFM_to_disk(RsuIFMStatusEntry_t * IFM) 
{
  FILE    * file_out       = NULL;
  uint32_t  mib_byte_count = 0;
  uint32_t  write_count    = 0;
  int32_t   ret            = RSEMIB_OK;
  IFMSkinny_t skinny_ifm;

  if(NULL == IFM){
      ret = RSEMIB_BAD_INPUT; 
  } else {
      /* Just the facts, no payload or index needed. */
      skinny_ifm.rsuIFMPsid[0]     = IFM->rsuIFMPsid[0];
      skinny_ifm.rsuIFMPsid[1]     = IFM->rsuIFMPsid[1];
      skinny_ifm.rsuIFMPsid[2]     = IFM->rsuIFMPsid[2];
      skinny_ifm.rsuIFMPsid[3]     = IFM->rsuIFMPsid[3];
      skinny_ifm.rsuIFMPsid_length = IFM->rsuIFMPsid_length; 
      skinny_ifm.rsuIFMTxChannel   = IFM->rsuIFMTxChannel;
      skinny_ifm.rsuIFMEnable      = IFM->rsuIFMEnable;
      skinny_ifm.rsuIFMStatus      = IFM->rsuIFMStatus;
      skinny_ifm.rsuIFMPriority    = IFM->rsuIFMPriority;
      skinny_ifm.rsuIFMOptions     = IFM->rsuIFMOptions;
      skinny_ifm.version           = IFM->version;
      skinny_ifm.reserved          = IFM->reserved;
      skinny_ifm.crc               = 0x0;

      if ((file_out = fopen(IFM->filePathName, "wb")) == NULL){
          DEBUGMSGTL((MY_NAME, "commit_to_disk: fopen fail.\n"));
          ret = RSEMIB_FOPEN_FAIL;
      } else {
          skinny_ifm.crc = (uint32_t)crc16((const uint8_t *)&skinny_ifm, sizeof(skinny_ifm) - 4);
          mib_byte_count = sizeof(skinny_ifm);
          write_count = fwrite((uint8_t *)&skinny_ifm, mib_byte_count, 1, file_out);
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
