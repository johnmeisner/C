/**************************************************************************
 *                                                                        *
 *     File Name:  rsuWsaConfig.c                                         *
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

/* Ignore requests we have serviced. Quicker, but no harm if not enabled. */
#define USE_PROCESSED_FLAG

/* Mandatory: For debugging and syslog output. */
#define MY_NAME "WsaConfig"
#define MY_NAME_EXTRA "XWsaConfig"

/* rsuWsaConfig.c specific: Create what you need to debug. */
#define WSA_CONFIG_INSTALL_FAIL            RSEMIB_INSTALL_HANDLER_FAIL
#define WSA_CONFIG_BAD_INPUT               RSEMIB_BAD_INPUT
#define WSA_CONFIG_LOCK_FAIL               RSEMIB_LOCK_FAIL
#define WSA_CONFIG_BAD_DATA                RSEMIB_BAD_DATA
#define WSA_CONFIG_BAD_MODE                RSEMIB_BAD_MODE
#define WSA_CONFIG_BAD_INDEX               RSEMIB_BAD_INDEX

#define WSA_CONFIG_BAD_REQUEST_TABLE_INFO  -18
#define WSA_CONFIG_REBUILD_FAIL            -19
#define WSA_CONFIG_ROW_EMPTY               -20
#define WSA_CONFIG_BAD_REQUEST_TABLE_ENTRY -21
#define WSA_CONFIG_OPTIONS_LENGTH_ERROR    -22
#define WSA_CONFIG_BAD_REQUEST_COLUMN      -23

#define WSA_CHANNEL_BAD_REQUEST_TABLE_INFO  -24 /* This module could be split in 2. */
#define WSA_CHANNEL_REBUILD_FAIL            -25
#define WSA_CHANNEL_INSTALL_FAIL            -26
#define WSA_CHANNEL_BAD_REQUEST_TABLE_ENTRY -27
#define WSA_CHANNEL_BAD_REQUEST_COLUMN      -28
#define WSA_CHANNEL_BAD_INDEX               -29

/* table column definitions. */
#define WSA_INDEX_COLUMN                 1
#define WSA_PSID_COLUMN                  2
#define WSA_PRIORITY_COLUMN              3
#define WSA_PSC_COLUMN                   4
#define WSA_IP_ADDRESS_COLUMN            5
#define WSA_PORT_COLUMN                  6
#define WSA_CHANNEL_COLUMN               7
#define WSA_STATUS_COLUMN                8
#define WSA_MAC_ADDRESS_COLUMN           9
#define WSA_OPTIONS_COLUMN               10
#define WSA_RCPI_THRESH_COLUMN           11
#define WSA_COUNT_THRESH_COLUMN          12
#define WSA_COUNT_THRESH_INTERVAL_COLUMN 13
#define WSA_REPEAT_RATE_COLUMN           14
#define WSA_AD_IDENTIFIER_COLUMN         15
#define WSA_ENABLE_COLUMN                16

#define WSA_CHANNEL_INDEX_COLUMN    1
#define WSA_CHANNEL_PSID_COLUMN     2
#define WSA_CHANNEL_NUMBER_COLUMN   3
#define WSA_CHANNEL_TX_POWER_COLUMN 4
#define WSA_CHANNEL_ACCESS_COLUMN   5
#define WSA_CHANNEL_STATUS_COLUMN   6

/******************************************************************************
 * 5.10 WAVE Service Advertisement: { rsu 9 }: rsuWsaConfig.
 * 
 *   5.10.1               Max WAVE Service Ad: maxRsuWsaServices.
 *   5.10.2.2 - 5.10.2.16 WSA Table          : rsuWsaServiceTable.
 *   5.10.3.1 - 5.10.3.6  WSA Chan Table     : rsuWsaChannelTable.
 *   5.10.4               WSA Version        : rsuWsaVersion.  
 * 
 * typedef struct {
 *     int32_t rsuWsaIndex;                            RO: 1 to maxRsuWsaServices. Must be 1 or greater.
 *     uint8_t rsuWsaPsid[RSU_RADIO_PSID_SIZE];        RC: 1..4 OCTETS: not fixed length. 
 *     int32_t rsuWsaPriority;                         RC: 1..7: no default.
 *     uint8_t rsuWsaPSC[WSA_PSC_LENGTH_MAX];          RC: 0..31: 0 = omit from WSA.
 *     uint8_t rsuWsaIpAddress[RSU_DEST_IP_MAX];       RC: 0..64: ie '2031:0:130F::9C0:876A:130B':if empty then?
 *     int32_t rsuWsaPort;                             RC: 1024..65535
 *     int32_t rsuWsaChannel;                          RC: 0..255: dsrc is 172 to 184: cv2x is Table 7 of NTCIP 1218 v01.
 *     int32_t rsuWsaStatus;                           RC: Create (4) & (6) destroy: notInService & notReady?
 *     uint8_t rsuWsaMacAddress[RSU_RADIO_MAC_LENGTH]; RC: IEEE 1609.3-2016 dot3ProviderMacAddress
 * 
 *
 *     IEEE 1609.3-2016 dot3WsaType 
 *     IEEE 1609.3-2016 dot3ProviderIpService 
 *     IEEE 1609.3-2016 dot3ProviderRepeatRate 
 *     IEEE 1609.3-2016 dot3ProviderRcpiThreshold
 *     
 *     BIT 0: Security   : 0=no, 1=yes: Use Security.
 *     BIT 1: WRA        : 0=no, 1=yes: Tx WAVE Routing Advertisement with WSA.
 *     BIT 2: Repeat Rate: 0=no, 1=yes: Tx repeat rate sent with WSA.
 *     BIT 3: 2DLoc      : 0=no, 1=yes: Tx rsuLocationLat, rsuLocationLong: Only if Bit 4 not set. 
 *     BIT 4: 3DLoc      : 0=no, 1=yes: Tx rsuLocationLat, rsuLocationLong,rsuLocationElv.
 *     BIT 5: RcpiThresh : 0=no, 1=yes: Tx RcpiThreshold sent in WSA Header.
 *     BIT 6: Reserved
 *     BIT 7: Reserved
 *     
 *     uint8_t  rsuWsaOptions;        RC: bitmap 0 to 7
 * 
 *     int32_t  rsuWsaRcpiThreshold;  RC:0..255:coded per IEEE Std 802.11-2012/18.3.1  
 * 
 *     The recommended number of received WSAs within the number of 100 ms intervals defined by rsuWsaCountThresholdInterval 
 *     before accepting the advertised service.
 *        
 *     int32_t  rsuWsaCountThreshold; RC:0..255:msecs 
 * 
 *     The number of 100 millisecond intervals over which to count received WSAs before accepting the advertised service. 
 *     Used with rsuWsaCountThreshold. A value of 0 indicates the WSA count threshold and WSA count threshold interval 
 *     is not transmitted in the IEEE 1609.3 WSA header.
 *     
 *     int32_t  rsuWsaCountThresholdInterval;                    RC:0..255:msecs
 * 
 *     int32_t  rsuWsaRepeatRate;                                RC:0..255: wsa count per 5 seconds
 * 
 *     uint8_t  rsuWsaAdvertiserIdentifier[WSA_AD_ID_LENGTH_MAX]; RC:0..31: SIZE(0) = do not send.
 *  
 *     1 = RSU will do consistency check rsuWsaChannelTable (5.10.4), determined by PSID(rsuWsaChannelPsid). 
 *     If fails then set to 0.'genError' is returned, and an error is logged.
 *     
 *     int32_t rsuWsaEnable;  RC: 1|0 : 1 = row enabled, send WSA after validation, 0 = disabled.
 *
 * } RsuWsaServiceEntry_t;
 *
 * typedef struct {
 *     int32_t rsuWsaChannelIndex;                     RO: 1 to maxRsuWsaServices = RSU_WSA_SERVICES_MAX.
 *     uint8_t rsuWsaChannelPsid[RSU_RADIO_PSID_SIZE]; RC: 1..4 OCTETS: not fixed length. See 5.10.2 rsuWsaPsid.
 *     int32_t rsuWsaChannelNumber;                    RC: 0..255: dsrc 172 to 184: cv2x Table 7 of NTCIP 1218 v01.
 *     int32_t rsuWsaChannelTxPowerLevel;              RC: -127..128:dbm:IEEE 1609.3-2016 dot3ProviderChannelInfoTransmitPowerLevel.
 *     int32_t rsuWsaChannelAccess;                    RC: 0=both,1=slot0,2=slot1,3=notUsed:IEEE1609.3-2016 dot3ProviderChannelAccess.
 *     int32_t rsuWsaChannelStatus;                    RC: Create (4) & (6) destroy:
 * } RsuWsaChannelEntry_t;
 * 
 ******************************************************************************/

const oid maxRsuWsaServices_oid[]  = {1, 3, 6, 1, 4, 1, 1206, 4, 2, 18, 9, 1};
const oid rsuWsaServiceTable_oid[] = {1, 3, 6, 1, 4, 1, 1206, 4, 2, 18, 9, 2};
const oid rsuWsaChannelTable_oid[] = {1, 3, 6, 1, 4, 1, 1206, 4, 2, 18, 9, 3};
const oid rsuWsaVersion_oid[]      = {1, 3, 6, 1, 4, 1, 1206, 4, 2, 18, 9, 4};

static netsnmp_table_registration_info * service_table_reg_info = NULL; /* snmp table reg pointers. */
static netsnmp_table_registration_info * channel_table_reg_info = NULL;
static netsnmp_tdata                   * rsuWsaServiceTable     = NULL; /* snmp table pointers. */
static netsnmp_tdata                   * rsuWsaChannelTable     = NULL;

static uint32_t rsuWsaConfig_error_states = 0x0; /* Local error states. */

/*
 * Protos: Silence of the Warnings. 
 */
static void rebuild_rsuWsaServiceTable(void);
static void rebuild_rsuWsaChannelTable(void);
static void destroy_rsuWsaServiceTable(void);
static void destroy_rsuWsaChannelTable(void);

/* 
 * Function Bodies.
 */
static void set_rsuWsaConfig_error_states(int32_t rsuWsaConfig_error)
{
  int32_t dummy = 0;

  dummy = abs(rsuWsaConfig_error);

  /* -1 to -31 */
  if((rsuWsaConfig_error < 0) && (rsuWsaConfig_error > MIB_ERROR_BASE_2)){
      rsuWsaConfig_error_states |= (uint32_t)(0x1) << (dummy - 1);
  }
}
uint32_t get_rsuWsaConfig_error_states(void)
{
  return rsuWsaConfig_error_states;
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
          set_rsuWsaConfig_error_states(WSA_CONFIG_BAD_INPUT);
          ret = SNMP_ERR_BADVALUE;
          break;
      case RSEMIB_BAD_INDEX:
          set_rsuWsaConfig_error_states(WSA_CONFIG_BAD_INDEX);
          ret = SNMP_ERR_BADVALUE;
          break;
      case RSEMIB_LOCK_FAIL:
          set_rsuWsaConfig_error_states(WSA_CONFIG_LOCK_FAIL); 
          ret = SNMP_ERR_NOACCESS;   
          break;
/* need GET VS PUT BAD DATA. */
      case RSEMIB_BAD_DATA:
          set_rsuWsaConfig_error_states(WSA_CONFIG_BAD_DATA);
          ret = SNMP_ERR_BADVALUE;
          break;
      case RSEMIB_ROW_EMPTY:
          set_rsuWsaConfig_error_states(WSA_CONFIG_ROW_EMPTY);
          ret = SNMP_ERR_NOERROR; /* Row has not been created so nothing to do. */
          break;
      case WSA_CONFIG_BAD_MODE:
          set_rsuWsaConfig_error_states(WSA_CONFIG_BAD_MODE);
          ret = SNMP_ERR_INCONSISTENTVALUE;
          break;
      default:
          DEBUGMSGTL((MY_NAME_EXTRA, "handle_rsemib_error: bogus error=%d.\n",input));
          set_rsuWsaConfig_error_states(WSA_CONFIG_BAD_MODE);
          ret = SNMP_ERR_NOACCESS;
          break;
  }
  return ret;
}

/*
 * OID Handlers
 */

/* 5.10.1 */
int32_t handle_maxRsuWsaServices(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo,
                            netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret = RSEMIB_OK;

  handler = handler; /* Silence of the Warnings. */
  reginfo = reginfo;

  DEBUGMSGTL((MY_NAME, "handle_maxRsuWsaServices: Entry:\n"));
  switch (reqinfo->mode){
      case MODE_GET:
          DEBUGMSGTL((MY_NAME, "GET:\n"));
          if(0 <= (ret = get_maxRsuWsaServices())){
              snmp_set_var_typed_integer(requests->requestvb, ASN_INTEGER, ret);
              ret = RSEMIB_OK;
          }
          break;
      default:
          DEBUGMSGTL((MY_NAME, "unknown mode (%d):\n",reqinfo->mode));
          ret = WSA_CONFIG_BAD_MODE;
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

/* 5.10.2.1 */
int32_t handle_rsuWsaIndex(int32_t index, netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret  = RSEMIB_OK;
  int32_t data = index;

  DEBUGMSGTL((MY_NAME, "handle_rsuWsaIndex_%d: Entry:\n", index));
  switch (reqinfo->mode){
      case MODE_GET:
#if 0
          if(RSEMIB_OK == (ret = get_rsuWsaIndex(index, &data)))
#endif
          {
              snmp_set_var_typed_integer(requests->requestvb, ASN_INTEGER, data);
              ret = RSEMIB_OK;
          }
          break;
      default:
          DEBUGMSGTL((MY_NAME, "unknown mode (%d):\n",reqinfo->mode));
          ret = WSA_CONFIG_BAD_MODE;
          break;
  }
  return ret;
}
/* 5.10.2.2 */
int32_t handle_rsuWsaPsid(int32_t index, netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret = RSEMIB_OK;
  uint8_t data[RSU_RADIO_PSID_SIZE];

  DEBUGMSGTL((MY_NAME, "handle_rsuWsaPsid_%d: Entry:\n", index));
  switch (reqinfo->mode){
      case MODE_GET:
          DEBUGMSGTL((MY_NAME, "GET:\n"));
          memset(data,0x0,sizeof(data));
          if(0 <= (ret = get_rsuWsaPsid(index,data))){
              snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR, data, ret); 
              ret = RSEMIB_OK;
          }
          break;
      case MODE_SET_RESERVE1:
          DEBUGMSGTL((MY_NAME, "R1:\n"));
          if(RSEMIB_OK == (ret = preserve_rsuWsaPsid(index))){
              if(RSEMIB_OK == (ret = action_rsuWsaPsid(index,requests->requestvb->val.string,requests->requestvb->val_len))){
                  if(RSEMIB_OK == (ret = commit_rsuWsaPsid(index))){
                      DEBUGMSGTL((MY_NAME, "COMMIT DONE:\n"));
                  } else {
                      DEBUGMSGTL((MY_NAME, "COMMIT FAIL:\n"));
                  }
              } else {
                  DEBUGMSGTL((MY_NAME, "ACTION FAIL:\n"));
              }
              if(RSEMIB_OK != ret) {
                  if(RSEMIB_OK != undo_rsuWsaPsid(index)) { /* UNDO fails nothing we can do. */
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
          DEBUGMSGTL((MY_NAME, "unknown mode (%d):\n",reqinfo->mode));
          ret = WSA_CONFIG_BAD_MODE;
          break;
  }
  return ret;
}
/* 5.10.2.3 */
static int32_t handle_rsuWsaPriority(int32_t index,netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret  = RSEMIB_OK;
  int32_t data = 0;

  DEBUGMSGTL((MY_NAME, "handle_rsuWsaPriority_%d: Entry:\n", index));
  switch (reqinfo->mode){
      case MODE_GET:
          DEBUGMSGTL((MY_NAME, "GET:\n"));
          if(RSEMIB_OK == (ret = get_rsuWsaPriority(index,&data))){
              snmp_set_var_typed_integer(requests->requestvb, ASN_INTEGER, data);
          }
          break;
      case MODE_SET_RESERVE1: /* Short stack, all requests done in one. */
          DEBUGMSGTL((MY_NAME, "R1:\n"));
          if(RSEMIB_OK == (ret = preserve_rsuWsaPriority(index))){
              if(RSEMIB_OK == (ret = action_rsuWsaPriority(index,*(requests->requestvb->val.integer)))){
                  if(RSEMIB_OK == (ret = commit_rsuWsaPriority(index))){
                      DEBUGMSGTL((MY_NAME, "COMMIT DONE:\n"));
                  } else {
                      DEBUGMSGTL((MY_NAME, "COMMIT FAIL:\n"));
                  }
              } else {
                  DEBUGMSGTL((MY_NAME, "ACTION FAIL:\n"));
              }
              if(RSEMIB_OK != ret) {
                  if(RSEMIB_OK != undo_rsuWsaPriority(index)) { /* UNDO fails nothing we can do. */
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
          DEBUGMSGTL((MY_NAME, "unknown mode (%d):\n",reqinfo->mode));
          ret = WSA_CONFIG_BAD_MODE;
          break;
  }
  return ret;
}
/* 5.10.2.4 */
int32_t handle_rsuWsaPSC(int32_t index, netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t  ret  = RSEMIB_OK;
  uint8_t  data[WSA_PSC_LENGTH_MAX];

  DEBUGMSGTL((MY_NAME, "handle_rsuWsaPSC_%d: Entry:\n", index));
  switch (reqinfo->mode){
      case MODE_GET:
          DEBUGMSGTL((MY_NAME, "GET:\n"));
          memset(data,0x0,sizeof(data));
          if(0 <= (ret = get_rsuWsaPSC(index,data))){
              snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR, data, ret); 
              ret = RSEMIB_OK;
          }
          break;
      case MODE_SET_RESERVE1:
          DEBUGMSGTL((MY_NAME, "R1:\n"));
          if(RSEMIB_OK == (ret = preserve_rsuWsaPSC(index))){
              if(RSEMIB_OK == (ret = action_rsuWsaPSC(index,requests->requestvb->val.string,requests->requestvb->val_len))){
                  if(RSEMIB_OK == (ret = commit_rsuWsaPSC(index))){
                      DEBUGMSGTL((MY_NAME, "COMMIT DONE:\n"));
                  } else {
                      DEBUGMSGTL((MY_NAME, "COMMIT FAIL:\n"));
                  }
              } else {
                  DEBUGMSGTL((MY_NAME, "ACTION FAIL:\n"));
              }
              if(RSEMIB_OK != ret) {
                  if(RSEMIB_OK != undo_rsuWsaPSC(index)) { /* UNDO fails nothing we can do. */
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
          DEBUGMSGTL((MY_NAME, "unknown mode (%d):\n",reqinfo->mode));
          ret = WSA_CONFIG_BAD_MODE;
          break;
  }
  return ret;
}
/* 5.10.2.5 */
int32_t handle_rsuWsaIpAddress(int32_t index, netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret  = 0;
  uint8_t data[RSU_DEST_IP_MAX];

  DEBUGMSGTL((MY_NAME, "handle_rsuWsaIpAddress_%d: Entry:\n", index));
  switch (reqinfo->mode){
      case MODE_GET:
          DEBUGMSGTL((MY_NAME, "GET:\n"));
          memset(data,0x0,sizeof(data));
          if(0 <= (ret = get_rsuWsaIpAddress(index,data))){
              snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR, data, ret); 
              ret = RSEMIB_OK;
          }
          break;
      case MODE_SET_RESERVE1:
          DEBUGMSGTL((MY_NAME, "R1:\n"));
          if(RSEMIB_OK == (ret = preserve_rsuWsaIpAddress(index))){
              if(RSEMIB_OK == (ret = action_rsuWsaIpAddress(index,requests->requestvb->val.string,requests->requestvb->val_len))){
                  if(RSEMIB_OK == (ret = commit_rsuWsaIpAddress(index))){
                      DEBUGMSGTL((MY_NAME, "COMMIT DONE:\n"));
                  } else {
                      DEBUGMSGTL((MY_NAME, "COMMIT FAIL:\n"));
                  }
              } else {
                  DEBUGMSGTL((MY_NAME, "ACTION FAIL:\n"));
              }
              if(RSEMIB_OK != ret) {
                  if(RSEMIB_OK != undo_rsuWsaIpAddress(index)) { /* UNDO fails nothing we can do. */
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
          DEBUGMSGTL((MY_NAME, "unknown mode (%d):\n",reqinfo->mode));
          ret = WSA_CONFIG_BAD_MODE;
          break;
  }
  return ret;
}
/* 5.10.2.6 */
static int32_t handle_rsuWsaPort(int32_t index,netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret  = RSEMIB_OK;
  int32_t data = 0;

  DEBUGMSGTL((MY_NAME, "handle_rsuWsaPort_%d: Entry:\n", index));
  switch (reqinfo->mode){
      case MODE_GET:
          DEBUGMSGTL((MY_NAME, "GET:\n"));
          if(RSEMIB_OK == (ret = get_rsuWsaPort(index,&data))){
              snmp_set_var_typed_integer(requests->requestvb, ASN_INTEGER, data);
          }
          break;
      case MODE_SET_RESERVE1: /* Short stack, all requests done in one. */
          DEBUGMSGTL((MY_NAME, "R1:\n"));
          if(RSEMIB_OK == (ret = preserve_rsuWsaPort(index))){
              if(RSEMIB_OK == (ret = action_rsuWsaPort(index,*(requests->requestvb->val.integer)))){
                  if(RSEMIB_OK == (ret = commit_rsuWsaPort(index))){
                      DEBUGMSGTL((MY_NAME, "COMMIT DONE:\n"));
                  } else {
                      DEBUGMSGTL((MY_NAME, "COMMIT FAIL:\n"));
                  }
              } else {
                  DEBUGMSGTL((MY_NAME, "ACTION FAIL:\n"));
              }
              if(RSEMIB_OK != ret) {
                  if(RSEMIB_OK != undo_rsuWsaPort(index)) { /* UNDO fails nothing we can do. */
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
          DEBUGMSGTL((MY_NAME, "unknown mode (%d):\n",reqinfo->mode));
          ret = WSA_CONFIG_BAD_MODE;
          break;
  }
  return ret;
}
/* 5.10.2.7 */
static int32_t handle_rsuWsaChannel(int32_t index,netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret  = RSEMIB_OK;
  int32_t data = 0;

  DEBUGMSGTL((MY_NAME, "handle_rsuWsaChannel_%d: Entry:\n", index));
  switch (reqinfo->mode){
      case MODE_GET:
          DEBUGMSGTL((MY_NAME, "GET:\n"));
          if(RSEMIB_OK == (ret = get_rsuWsaChannel(index,&data))){
              snmp_set_var_typed_integer(requests->requestvb, ASN_INTEGER, data);
          }
          break;
      case MODE_SET_RESERVE1: /* Short stack, all requests done in one. */
          DEBUGMSGTL((MY_NAME, "R1:\n"));
          if(RSEMIB_OK == (ret = preserve_rsuWsaChannel(index))){
              if(RSEMIB_OK == (ret = action_rsuWsaChannel(index,*(requests->requestvb->val.integer)))){
                  if(RSEMIB_OK == (ret = commit_rsuWsaChannel(index))){
                      DEBUGMSGTL((MY_NAME, "COMMIT DONE:\n"));
                  } else {
                      DEBUGMSGTL((MY_NAME, "COMMIT FAIL:\n"));
                  }
              } else {
                  DEBUGMSGTL((MY_NAME, "ACTION FAIL:\n"));
              }
              if(RSEMIB_OK != ret) {
                  if(RSEMIB_OK != undo_rsuWsaChannel(index)) { /* UNDO fails nothing we can do. */
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
          DEBUGMSGTL((MY_NAME, "unknown mode (%d):\n",reqinfo->mode));
          ret = WSA_CONFIG_BAD_MODE;
          break;
  }
  return ret;
}

/* 5.10.2.8 */
static int32_t handle_rsuWsaStatus(int32_t index,netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests, int32_t override)
{
  int32_t ret  = RSEMIB_OK;
  int32_t data = 0;
  int32_t row_status = SNMP_ROW_CREATEANDGO; /* If override is set then don't use requests's value. Use this one. */

  DEBUGMSGTL((MY_NAME, "handle_rsuWsaStatus_%d: Entry:\n", index));
  switch (reqinfo->mode){
      case MODE_GET:
          if(RSEMIB_OK == (ret = get_rsuWsaStatus(index,&data))){
              snmp_set_var_typed_integer(requests->requestvb, ASN_INTEGER, data);
          }
          break;
      case MODE_SET_RESERVE1: /* Short stack, all requests done in one. */
          if(0 == override){
              row_status = *(requests->requestvb->val.integer); /* Client index part of CREATE. */
          }
          DEBUGMSGTL((MY_NAME, "R1_BULK_SET: status = %d.\n",row_status));
          if(RSEMIB_OK == (ret = preserve_rsuWsaStatus(index))){
              if(RSEMIB_OK == (ret = action_rsuWsaStatus(index, row_status))){
                  if(RSEMIB_OK == (ret = commit_rsuWsaStatus(index))){
                      DEBUGMSGTL((MY_NAME, "COMMIT DONE:\n"));
                  } else {
                      DEBUGMSGTL((MY_NAME, "COMMIT FAIL:\n"));
                  }
              } else {
                  DEBUGMSGTL((MY_NAME, "ACTION FAIL:\n"));
              }
              if(RSEMIB_OK != ret) {
                  if(RSEMIB_OK != undo_rsuWsaStatus(index)) { /* UNDO fails nothing we can do. */
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
          DEBUGMSGTL((MY_NAME, "unknown mode (%d):\n",reqinfo->mode));
          ret = WSA_CONFIG_BAD_MODE;
          break;
  }
  return ret;
}
/* 5.10.2.9 */
int32_t handle_rsuWsaMacAddress(int32_t index, netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret = RSEMIB_OK;
  uint8_t data[RSU_DEST_IP_MAX];

  DEBUGMSGTL((MY_NAME, "handle_rsuWsaMacAddress_%d: Entry:\n", index));
  switch (reqinfo->mode){
      case MODE_GET:
          DEBUGMSGTL((MY_NAME, "GET:\n"));
          memset(data,0x0,sizeof(data));
          if(0 <= (ret = get_rsuWsaMacAddress(index,data))){
              snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR, data, ret); 
              ret = RSEMIB_OK;
          }
          break;
      case MODE_SET_RESERVE1:
          DEBUGMSGTL((MY_NAME, "R1:\n"));
          if(RSEMIB_OK == (ret = preserve_rsuWsaMacAddress(index))){
              if(RSEMIB_OK == (ret = action_rsuWsaMacAddress(index,requests->requestvb->val.string,requests->requestvb->val_len))){
                  if(RSEMIB_OK == (ret = commit_rsuWsaMacAddress(index))){
                      DEBUGMSGTL((MY_NAME, "COMMIT DONE:\n"));
                  } else {
                      DEBUGMSGTL((MY_NAME, "COMMIT FAIL:\n"));
                  }
              } else {
                  DEBUGMSGTL((MY_NAME, "ACTION FAIL:\n"));
              }
              if(RSEMIB_OK != ret) {
                  if(RSEMIB_OK != undo_rsuWsaMacAddress(index)) { /* UNDO fails nothing we can do. */
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
          DEBUGMSGTL((MY_NAME, "unknown mode (%d):\n",reqinfo->mode));
          ret = WSA_CONFIG_BAD_MODE;
          break;
  }
  return ret;
}
/* 5.8.2.10 */
static int32_t handle_rsuWsaOptions(int32_t index,netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret = RSEMIB_OK;
  uint8_t data[WSA_OPTIONS_LENGTH];

  DEBUGMSGTL((MY_NAME, "handle_rsuWsaOptions_%d: Entry:\n", index));
  switch (reqinfo->mode){
      case MODE_GET:
          DEBUGMSGTL((MY_NAME, "GET:\n"));
          if(WSA_OPTIONS_LENGTH == (ret = get_rsuWsaOptions(index,data))){
#if defined(ENABLE_ASN_BIT_STR)
              snmp_set_var_typed_value(requests->requestvb, ASN_BIT_STR, data, WSA_OPTIONS_LENGTH);
#else
              snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR, data, WSA_OPTIONS_LENGTH);
#endif
              ret = RSEMIB_OK;
          }
          break;
      case MODE_SET_RESERVE1: /* Short stack, all requests done in one. */
          DEBUGMSGTL((MY_NAME, "R1:\n"));
          if(RSEMIB_OK == (ret = preserve_rsuWsaOptions(index))){
              if(RSEMIB_OK == (ret = action_rsuWsaOptions(index,requests->requestvb->val.bitstring,requests->requestvb->val_len))){
                  if(RSEMIB_OK == (ret = commit_rsuWsaOptions(index))){
                      DEBUGMSGTL((MY_NAME, "COMMIT DONE:\n"));
                  } else {
                      DEBUGMSGTL((MY_NAME, "COMMIT FAIL:\n"));
                  }
              } else {
                  DEBUGMSGTL((MY_NAME, "ACTION FAIL:\n"));
              }
              if(RSEMIB_OK != ret) {
                  if(RSEMIB_OK != undo_rsuWsaOptions(index)) { /* UNDO fails nothing we can do. */
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
          DEBUGMSGTL((MY_NAME, "unknown mode (%d):\n",reqinfo->mode));
          ret = WSA_CONFIG_BAD_MODE;
          break;
  }
  return ret;
}

/* 5.10.2.11 */
static int32_t handle_rsuWsaRcpiThreshold(int32_t index,netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret  = RSEMIB_OK;
  int32_t data = 0;

  DEBUGMSGTL((MY_NAME, "handle_rsuWsaRcpiThreshold_%d: Entry:\n", index));
  switch (reqinfo->mode){
      case MODE_GET:
          DEBUGMSGTL((MY_NAME, "GET:\n"));
          if(RSEMIB_OK == (ret = get_rsuWsaRcpiThreshold(index,&data))){
              snmp_set_var_typed_integer(requests->requestvb, ASN_INTEGER, data);
          }
          break;
      case MODE_SET_RESERVE1: /* Short stack, all requests done in one. */
          DEBUGMSGTL((MY_NAME, "R1:\n"));
          if(RSEMIB_OK == (ret = preserve_rsuWsaRcpiThreshold(index))){
              if(RSEMIB_OK == (ret = action_rsuWsaRcpiThreshold(index,*(requests->requestvb->val.integer)))){
                  if(RSEMIB_OK == (ret = commit_rsuWsaRcpiThreshold(index))){
                      DEBUGMSGTL((MY_NAME, "COMMIT DONE:\n"));
                  } else {
                      DEBUGMSGTL((MY_NAME, "COMMIT FAIL:\n"));
                  }
              } else {
                  DEBUGMSGTL((MY_NAME, "ACTION FAIL:\n"));
              }
              if(RSEMIB_OK != ret) {
                  if(RSEMIB_OK != undo_rsuWsaRcpiThreshold(index)) { /* UNDO fails nothing we can do. */
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
          DEBUGMSGTL((MY_NAME, "unknown mode (%d):\n",reqinfo->mode));
          ret = WSA_CONFIG_BAD_MODE;
          break;
  }
  return ret;
}

/* 5.10.2.12 */
static int32_t handle_rsuWsaCountThreshold(int32_t index,netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret  = RSEMIB_OK;
  int32_t data = 0;

  DEBUGMSGTL((MY_NAME, "handle_rsuWsaCountThreshold_%d: Entry:\n", index));
  switch (reqinfo->mode){
      case MODE_GET:
          DEBUGMSGTL((MY_NAME, "GET:\n"));
          if(RSEMIB_OK == (ret = get_rsuWsaCountThreshold(index,&data))){
              snmp_set_var_typed_integer(requests->requestvb, ASN_INTEGER, data);
          }
          break;
      case MODE_SET_RESERVE1: /* Short stack, all requests done in one. */
          DEBUGMSGTL((MY_NAME, "R1:\n"));
          if(RSEMIB_OK == (ret = preserve_rsuWsaCountThreshold(index))){
              if(RSEMIB_OK == (ret = action_rsuWsaCountThreshold(index,*(requests->requestvb->val.integer)))){
                  if(RSEMIB_OK == (ret = commit_rsuWsaCountThreshold(index))){
                      DEBUGMSGTL((MY_NAME, "COMMIT DONE:\n"));
                  } else {
                      DEBUGMSGTL((MY_NAME, "COMMIT FAIL:\n"));
                  }
              } else {
                  DEBUGMSGTL((MY_NAME, "ACTION FAIL:\n"));
              }
              if(RSEMIB_OK != ret) {
                  if(RSEMIB_OK != undo_rsuWsaCountThreshold(index)) { /* UNDO fails nothing we can do. */
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
          DEBUGMSGTL((MY_NAME, "unknown mode (%d):\n",reqinfo->mode));
          ret = WSA_CONFIG_BAD_MODE;
          break;
  }
  return ret;
}

/* 5.10.2.13 */
static int32_t handle_rsuWsaCountThresholdInterval(int32_t index,netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret  = RSEMIB_OK;
  int32_t data = 0;

  DEBUGMSGTL((MY_NAME, "handle_rsuWsaCountThresholdInterval_%d: Entry:\n", index));
  switch (reqinfo->mode){
      case MODE_GET:
          DEBUGMSGTL((MY_NAME, "GET:\n"));
          if(RSEMIB_OK == (ret = get_rsuWsaCountThresholdInterval(index,&data))){
              snmp_set_var_typed_integer(requests->requestvb, ASN_INTEGER, data);
          }
          break;
      case MODE_SET_RESERVE1: /* Short stack, all requests done in one. */
          DEBUGMSGTL((MY_NAME, "R1:\n"));
          if(RSEMIB_OK == (ret = preserve_rsuWsaCountThresholdInterval(index))){
              if(RSEMIB_OK == (ret = action_rsuWsaCountThresholdInterval(index,*(requests->requestvb->val.integer)))){
                  if(RSEMIB_OK == (ret = commit_rsuWsaCountThresholdInterval(index))){
                      DEBUGMSGTL((MY_NAME, "COMMIT DONE:\n"));
                  } else {
                      DEBUGMSGTL((MY_NAME, "COMMIT FAIL:\n"));
                  }
              } else {
                  DEBUGMSGTL((MY_NAME, "ACTION FAIL:\n"));
              }
              if(RSEMIB_OK != ret) {
                  if(RSEMIB_OK != undo_rsuWsaCountThresholdInterval(index)) { /* UNDO fails nothing we can do. */
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
          DEBUGMSGTL((MY_NAME, "unknown mode (%d):\n",reqinfo->mode));
          ret = WSA_CONFIG_BAD_MODE;
          break;
  }
  return ret;
}

/* 5.10.2.14 */
static int32_t handle_rsuWsaRepeatRate(int32_t index,netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret  = RSEMIB_OK;
  int32_t data = 0;

  DEBUGMSGTL((MY_NAME, "handle_rsuWsaRepeatRate_%d: Entry:\n", index));
  switch (reqinfo->mode){
      case MODE_GET:
          DEBUGMSGTL((MY_NAME, "GET:\n"));
          if(RSEMIB_OK == (ret = get_rsuWsaRepeatRate(index,&data))){
              snmp_set_var_typed_integer(requests->requestvb, ASN_INTEGER, data);
          }
          break;
      case MODE_SET_RESERVE1: /* Short stack, all requests done in one. */
          DEBUGMSGTL((MY_NAME, "R1:\n"));
          if(RSEMIB_OK == (ret = preserve_rsuWsaRepeatRate(index))){
              if(RSEMIB_OK == (ret = action_rsuWsaRepeatRate(index,*(requests->requestvb->val.integer)))){
                  if(RSEMIB_OK == (ret = commit_rsuWsaRepeatRate(index))){
                      DEBUGMSGTL((MY_NAME, "COMMIT DONE:\n"));
                  } else {
                      DEBUGMSGTL((MY_NAME, "COMMIT FAIL:\n"));
                  }
              } else {
                  DEBUGMSGTL((MY_NAME, "ACTION FAIL:\n"));
              }
              if(RSEMIB_OK != ret) {
                  if(RSEMIB_OK != undo_rsuWsaRepeatRate(index)) { /* UNDO fails nothing we can do. */
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
          DEBUGMSGTL((MY_NAME, "unknown mode (%d):\n",reqinfo->mode));
          ret = WSA_CONFIG_BAD_MODE;
          break;
  }
  return ret;
}

/* 5.10.2.15 */
int32_t handle_rsuWsaAdvertiserIdentifier(int32_t index, netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret = RSEMIB_OK;
  uint8_t data[WSA_AD_ID_LENGTH_MAX];

  DEBUGMSGTL((MY_NAME, "handle_rsuWsaAdvertiserIdentifier_%d: Entry:\n", index));
  switch (reqinfo->mode){
      case MODE_GET:
          DEBUGMSGTL((MY_NAME, "GET:\n"));
          memset(data,0x0,sizeof(data));
          if(0 <= (ret = get_rsuWsaAdvertiserIdentifier(index,data))){
              snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR, data, ret);
              ret = RSEMIB_OK;
          }
          break;
      case MODE_SET_RESERVE1:
          DEBUGMSGTL((MY_NAME, "R1:\n"));
          if(RSEMIB_OK == (ret = preserve_rsuWsaAdvertiserIdentifier(index))){
              if(RSEMIB_OK == (ret = action_rsuWsaAdvertiserIdentifier(index,requests->requestvb->val.string,requests->requestvb->val_len))){
                  if(RSEMIB_OK == (ret = commit_rsuWsaAdvertiserIdentifier(index))){
                      DEBUGMSGTL((MY_NAME, "COMMIT DONE:\n"));
                  } else {
                      DEBUGMSGTL((MY_NAME, "COMMIT FAIL:\n"));
                  }
              } else {
                  DEBUGMSGTL((MY_NAME, "ACTION FAIL:\n"));
              }
              if(RSEMIB_OK != ret) {
                  if(RSEMIB_OK != undo_rsuWsaAdvertiserIdentifier(index)) { /* UNDO fails nothing we can do. */
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
          DEBUGMSGTL((MY_NAME, "unknown mode (%d):\n",reqinfo->mode));
          ret = WSA_CONFIG_BAD_MODE;
          break;
  }
  return ret;
}

/* 5.10.2.16 */
static int32_t handle_rsuWsaEnable(int32_t index, netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret  = RSEMIB_OK;
  int32_t data = 0;

  DEBUGMSGTL((MY_NAME, "handle_rsuWsaEnable_%d: Entry:\n", index));
  switch (reqinfo->mode){
      case MODE_GET:
          DEBUGMSGTL((MY_NAME, "GET:\n"));
          if(RSEMIB_OK == (ret = get_rsuWsaEnable(index,&data))){
              snmp_set_var_typed_integer(requests->requestvb, ASN_INTEGER, data);
          }
          break;
      case MODE_SET_RESERVE1: /* Short stack, all requests done in one. */
          DEBUGMSGTL((MY_NAME, "R1:\n"));
          if(RSEMIB_OK == (ret = preserve_rsuWsaEnable(index))){
              if(RSEMIB_OK == (ret = action_rsuWsaEnable(index,*(requests->requestvb->val.integer)))){
                  if(RSEMIB_OK == (ret = commit_rsuWsaEnable(index))){
                      DEBUGMSGTL((MY_NAME, "COMMIT DONE:\n"));
                  } else {
                      DEBUGMSGTL((MY_NAME, "COMMIT FAIL:\n"));
                  }
              } else {
                  DEBUGMSGTL((MY_NAME, "ACTION FAIL:\n"));
              }
              if(RSEMIB_OK != ret) {
                  if(RSEMIB_OK != undo_rsuWsaEnable(index)) { /* UNDO fails nothing we can do. */
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
          DEBUGMSGTL((MY_NAME, "unknown mode (%d):\n",reqinfo->mode));
          ret = WSA_CONFIG_BAD_MODE;
          break;
  }
  return ret;
}
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
 }
#endif
/* 5.10.2: Table handler: Break it down to smaller requests. */
int32_t handle_rsuWsaServiceTable(  netsnmp_mib_handler          * handler
                                  , netsnmp_handler_registration * reginfo
                                  , netsnmp_agent_request_info   * reqinfo
                                  , netsnmp_request_info         * requests)
{
  netsnmp_request_info       * request       = NULL;
  uint32_t                     request_count = 0;
  RsuWsaServiceEntry_t       * table_entry   = NULL;
  netsnmp_table_request_info * request_info  = NULL;
  int32_t                      ret           = SNMP_ERR_NOERROR;
  uint8_t                      raw_oid[MAX_OID_LEN + 2]; /* If table doesn't exist then helper can't help. Go fish for row index. */
  uint8_t                      row_index = 0; /* For readability. Note: index is 1..maxRsuIFMs. */
  int32_t                      row_status = SNMP_ROW_NONEXISTENT;

  handler = handler; /* Silence of the Warnings. */
  reginfo = reginfo;

  DEBUGMSGTL((MY_NAME, "rsuWsaServiceTable_handler: Entry:\n"));
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
          set_rsuWsaConfig_error_states(WSA_CONFIG_BAD_REQUEST_TABLE_INFO);
          DEBUGMSGTL((MY_NAME, "NULL == request_info.\n"));
          ret = SNMP_ERR_INCONSISTENTVALUE;
          break;
      }
      if(NULL != (table_entry = (RsuWsaServiceEntry_t *)netsnmp_tdata_extract_entry(request))) {
          if((request_info->colnum < WSA_INDEX_COLUMN) || (WSA_ENABLE_COLUMN < request_info->colnum)) {
              set_rsuWsaConfig_error_states(WSA_CONFIG_BAD_REQUEST_COLUMN);
              ret = SNMP_ERR_INCONSISTENTVALUE;
              break;
          }
      }

      /* Grab row index from request. */
      memset(raw_oid,0x0,sizeof(raw_oid));
      memcpy(raw_oid,&(request->requestvb->name_loc[0]),MAX_OID_LEN);
      #if defined(EXTRA_DEBUG)
      print_rawOID(&raw_oid[0]);
      #endif
      DEBUGMSGTL((MY_NAME_EXTRA, "raw oid: row index = %d, column = %d.\n",raw_oid[112],raw_oid[104]));

      /* Do get() on rowStatus to see if it exists and error check index. */
      if(RSEMIB_OK != (ret = get_rsuWsaStatus(raw_oid[112],&row_status))) {
          DEBUGMSGTL((MY_NAME, "get_rsuWsaStatus Failed: ret=%d\n",ret));
          set_rsuWsaConfig_error_states(WSA_CONFIG_BAD_INDEX);
          ret = SNMP_ERR_INCONSISTENTVALUE;
          break; /* Bad request. We are done. */
      } else {
          row_index = raw_oid[112];
      }

      /* Depending on what you do the row must exist. */
      switch (reqinfo->mode){
          case MODE_GET:
#if 0 /* read+create: Helper prevents us from getting here. */
              if (SNMP_ROW_NONEXISTENT == row_status){ /* Can't GET if it doesn't exist. */
                  set_rsuWsaConfig_error_states(WSA_CONFIG_BAD_REQUEST_TABLE_ENTRY);
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
                  set_rsuWsaConfig_error_states(WSA_CONFIG_BAD_REQUEST_TABLE_ENTRY);
                  DEBUGMSGTL((MY_NAME, "UNDO: No request info.\n"));
                  ret = SNMP_ERR_INCONSISTENTVALUE;
              }
              break;
          default: /* Doesth thou really care? Unanticipated request type.*/
              set_rsuWsaConfig_error_states(WSA_CONFIG_BAD_MODE);
              DEBUGMSGTL((MY_NAME, "unrecognized mode.\n"));
              ret = SNMP_ERR_INCONSISTENTVALUE;  
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
          if(WSA_STATUS_COLUMN != request_info->colnum){/* If row doesn't exist then create now. */
              ret = handle_rsuWsaStatus(row_index, reqinfo, request, 1); /* Do not use value in request. Force createAndGo. */
              if(RSEMIB_OK != ret) {
                  DEBUGMSGTL((MY_NAME, "Create failed for ROW=%d COLUMN %d.\n", row_index, request_info->colnum));
                  set_rsuWsaConfig_error_states(WSA_CONFIG_BAD_INDEX);
                  ret = SNMP_ERR_BADVALUE;
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
      DEBUGMSGTL((MY_NAME, "GO for handler ROW=%d COLUMN=%d.\n",row_index,request_info->colnum));
      switch (request_info->colnum){
          case WSA_INDEX_COLUMN:
              ret = handle_rsuWsaIndex(row_index, reqinfo, request);
              break;
          case WSA_PSID_COLUMN:
              ret = handle_rsuWsaPsid(row_index, reqinfo, request);
              break;
          case WSA_PRIORITY_COLUMN:
              ret = handle_rsuWsaPriority(row_index, reqinfo, request);
              break;
          case WSA_PSC_COLUMN:
              ret = handle_rsuWsaPSC(row_index, reqinfo, request);
              break;
          case WSA_IP_ADDRESS_COLUMN:
              ret = handle_rsuWsaIpAddress(row_index, reqinfo, request);
              break;
          case WSA_PORT_COLUMN:
              ret = handle_rsuWsaPort(row_index, reqinfo, request);
              break;
          case WSA_CHANNEL_COLUMN:
              ret = handle_rsuWsaChannel(row_index, reqinfo, request);
              break;
          case WSA_STATUS_COLUMN:
//If already created and status matches then who cares?
              ret = handle_rsuWsaStatus(row_index, reqinfo, request,0);
              break;
          case WSA_MAC_ADDRESS_COLUMN:
              ret = handle_rsuWsaMacAddress(row_index, reqinfo, request);
              break;
          case WSA_OPTIONS_COLUMN:
              ret = handle_rsuWsaOptions(row_index, reqinfo, request);
              break;
          case WSA_RCPI_THRESH_COLUMN:
              ret = handle_rsuWsaRcpiThreshold(row_index, reqinfo, request);
              break;
          case WSA_COUNT_THRESH_COLUMN:
              ret = handle_rsuWsaCountThreshold(row_index, reqinfo, request);
              break;
          case WSA_COUNT_THRESH_INTERVAL_COLUMN:
              ret = handle_rsuWsaCountThresholdInterval(row_index, reqinfo, request);
              break;
          case WSA_REPEAT_RATE_COLUMN:
              ret = handle_rsuWsaRepeatRate(row_index, reqinfo, request);
              break;
          case WSA_AD_IDENTIFIER_COLUMN:
              ret = handle_rsuWsaAdvertiserIdentifier(row_index, reqinfo, request);
              break;
          case WSA_ENABLE_COLUMN:
              ret = handle_rsuWsaEnable(row_index, reqinfo, request);
              break;
          default:
             /* Ignore requests for columns outside defintion. Dont send error because it will kill snmpwalks. */
             break;
      }
      /* Convert from module error to SNMP request error. */
      ret = handle_rsemib_errors(ret);
  }/* for */

  /* 
   * Only top handler need do this. Set SNMP request error once. 
   * SNMP error is greater than zero unlike MIB which is less than zero.
   */
  if(SNMP_ERR_NOERROR < ret){
      netsnmp_set_request_error(reqinfo, requests, ret);
  }
#if defined(USE_PROCESSED_FLAG)
  requests->processed = 1; /* Good or bad, we handle request, GET or SET(R1 shortstack), only once. */
#endif

  DEBUGMSGTL((MY_NAME, "ret=%d:Exit.\n",ret));
  return ret;
}

/* 5.10.3.1 */
int32_t handle_rsuWsaChannelIndex(int32_t index, netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret  = RSEMIB_OK;
  int32_t data = 0;

  DEBUGMSGTL((MY_NAME, "handle_rsuWsaChannelIndex_%d: Entry:\n", index));
  switch (reqinfo->mode){
      case MODE_GET:
          if(RSEMIB_OK == (ret = get_rsuWsaChannelIndex(index, &data))){
              snmp_set_var_typed_integer(requests->requestvb, ASN_INTEGER, data);
          }
          break;
      default:
          ret = WSA_CONFIG_BAD_MODE;
          break;
  }
  DEBUGMSGTL((MY_NAME, "ret=%d:Exit.\n",ret));
  return ret;
}
/* 5.10.2.2 */
int32_t handle_rsuWsaChannelPsid(int32_t index, netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret = RSEMIB_OK;
  uint8_t data[RSU_RADIO_PSID_SIZE];

  DEBUGMSGTL((MY_NAME, "handle_rsuWsaChannelPsid_%d: Entry:\n", index));
  switch (reqinfo->mode){
      case MODE_GET:
          DEBUGMSGTL((MY_NAME, "GET:\n"));
          memset(data,0x0,sizeof(data));
          if(0 <= (ret = get_rsuWsaChannelPsid(index,data))){
              snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR, data, ret);
              ret = RSEMIB_OK;
          }
          break;
      case MODE_SET_RESERVE1:
          DEBUGMSGTL((MY_NAME, "R1:\n"));
          if(RSEMIB_OK == (ret = preserve_rsuWsaChannelPsid(index))){
              if(RSEMIB_OK == (ret = action_rsuWsaChannelPsid(index,requests->requestvb->val.string,requests->requestvb->val_len))){
                  if(RSEMIB_OK == (ret = commit_rsuWsaChannelPsid(index))){
                      DEBUGMSGTL((MY_NAME, "COMMIT DONE:\n"));
                  } else {
                      DEBUGMSGTL((MY_NAME, "COMMIT FAIL:\n"));
                  }
              } else {
                  DEBUGMSGTL((MY_NAME, "ACTION FAIL:\n"));
              }
              if(RSEMIB_OK != ret) {
                  if(RSEMIB_OK != undo_rsuWsaChannelPsid(index)) { /* UNDO fails nothing we can do. */
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
          DEBUGMSGTL((MY_NAME, "unknown mode (%d):\n",reqinfo->mode));
          ret = WSA_CONFIG_BAD_MODE;
          break;
  }
  return ret;
}
/* 5.10.3.3 RW */
static int32_t handle_rsuWsaChannelNumber(int32_t index,netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret  = RSEMIB_OK;
  int32_t data = 0;

  DEBUGMSGTL((MY_NAME, "handle_rsuWsaChanneNumber_%d: Entry:\n", index));
  switch (reqinfo->mode){
      case MODE_GET:
          DEBUGMSGTL((MY_NAME, "GET:\n"));
          if(RSEMIB_OK == (ret = get_rsuWsaChannelNumber(index,&data))){
              snmp_set_var_typed_integer(requests->requestvb, ASN_INTEGER, data);
          }
          break;
      case MODE_SET_RESERVE1: /* Short stack, all requests done in one. */
          DEBUGMSGTL((MY_NAME, "R1:\n"));
          if(RSEMIB_OK == (ret = preserve_rsuWsaChannelNumber(index))){
              if(RSEMIB_OK == (ret = action_rsuWsaChannelNumber(index,*(requests->requestvb->val.integer)))){
                  if(RSEMIB_OK == (ret = commit_rsuWsaChannelNumber(index))){
                      DEBUGMSGTL((MY_NAME, "COMMIT DONE:\n"));
                  } else {
                      DEBUGMSGTL((MY_NAME, "COMMIT FAIL:\n"));
                  }
              } else {
                  DEBUGMSGTL((MY_NAME, "ACTION FAIL:\n"));
              }
              if(RSEMIB_OK != ret) {
                  if(RSEMIB_OK != undo_rsuWsaChannelNumber(index)) { /* UNDO fails nothing we can do. */
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
          DEBUGMSGTL((MY_NAME, "unknown mode (%d):\n",reqinfo->mode));
          ret = WSA_CONFIG_BAD_MODE;
          break;
  }
  return ret;
}
/* 5.10.3.4 RW */
static int32_t handle_rsuWsaChannelTxPowerLevel(int32_t index,netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret  = RSEMIB_OK;
  int32_t data = 0;

  DEBUGMSGTL((MY_NAME, "handle_rsuWsaChannelTxPowerLevel_%d: Entry:\n", index));
  switch (reqinfo->mode){
      case MODE_GET:
          DEBUGMSGTL((MY_NAME, "GET:\n"));
          if(RSEMIB_OK == (ret = get_rsuWsaChannelTxPowerLevel(index,&data))){
              snmp_set_var_typed_integer(requests->requestvb, ASN_INTEGER, data);
          }
          break;
      case MODE_SET_RESERVE1: /* Short stack, all requests done in one. */
          DEBUGMSGTL((MY_NAME, "R1:\n"));
          if(RSEMIB_OK == (ret = preserve_rsuWsaChannelTxPowerLevel(index))){
              if(RSEMIB_OK == (ret = action_rsuWsaChannelTxPowerLevel(index,*(requests->requestvb->val.integer)))){
                  if(RSEMIB_OK == (ret = commit_rsuWsaChannelTxPowerLevel(index))){
                      DEBUGMSGTL((MY_NAME, "COMMIT DONE:\n"));
                  } else {
                      DEBUGMSGTL((MY_NAME, "COMMIT FAIL:\n"));
                  }
              } else {
                  DEBUGMSGTL((MY_NAME, "ACTION FAIL:\n"));
              }
              if(RSEMIB_OK != ret) {
                  if(RSEMIB_OK != undo_rsuWsaChannelTxPowerLevel(index)) { /* UNDO fails nothing we can do. */
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
          DEBUGMSGTL((MY_NAME, "unknown mode (%d):\n",reqinfo->mode));
          ret = WSA_CONFIG_BAD_MODE;
          break;
  }
  return ret;
}
/* 5.10.3.5 RW */
static int32_t handle_rsuWsaChannelAccess(int32_t index,netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret  = RSEMIB_OK;
  int32_t data = 0;

  DEBUGMSGTL((MY_NAME, "handle_rsuWsaChannelAccess_%d: Entry:\n", index));
  switch (reqinfo->mode){
      case MODE_GET:
          DEBUGMSGTL((MY_NAME, "GET:\n"));
          if(RSEMIB_OK == (ret = get_rsuWsaChannelAccess(index, &data))){
              snmp_set_var_typed_integer(requests->requestvb, ASN_INTEGER, data);
          }
          break;
      case MODE_SET_RESERVE1: /* Short stack, all requests done in one. */
          DEBUGMSGTL((MY_NAME, "R1:\n"));
          if(RSEMIB_OK == (ret = preserve_rsuWsaChannelAccess(index))){
              if(RSEMIB_OK == (ret = action_rsuWsaChannelAccess(index,*(requests->requestvb->val.integer)))){
                  if(RSEMIB_OK == (ret = commit_rsuWsaChannelAccess(index))){
                      DEBUGMSGTL((MY_NAME, "COMMIT DONE:\n"));
                  } else {
                      DEBUGMSGTL((MY_NAME, "COMMIT FAIL:\n"));
                  }
              } else {
                  DEBUGMSGTL((MY_NAME, "ACTION FAIL:\n"));
              }
              if(RSEMIB_OK != ret) {
                  if(RSEMIB_OK != undo_rsuWsaChannelAccess(index)) { /* UNDO fails nothing we can do. */
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
          DEBUGMSGTL((MY_NAME, "unknown mode (%d):\n",reqinfo->mode));
          ret = WSA_CONFIG_BAD_MODE;
          break;
  }
  return ret;
}
/* 5.10.3.6 */
static int32_t handle_rsuWsaChannelStatus(int32_t index,netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests, int32_t override)
{
  int32_t ret  = RSEMIB_OK;
  int32_t data = 0;
  int32_t row_status = SNMP_ROW_CREATEANDGO; /* If override is set then don't use requests's value. Use this one. */

  DEBUGMSGTL((MY_NAME, "handle_rsuWsaChannelStatus_%d: Entry:\n", index));
  switch (reqinfo->mode){
      case MODE_GET:
          if(RSEMIB_OK == (ret = get_rsuWsaChannelStatus(index,&data))){
              snmp_set_var_typed_integer(requests->requestvb, ASN_INTEGER, data);
          }
          break;
      case MODE_SET_RESERVE1: /* Short stack, all requests done in one. */
          if(0 == override){
              row_status = *(requests->requestvb->val.integer); /* Client index part of CREATE. */
          }
          DEBUGMSGTL((MY_NAME, "R1_BULK_SET: status = %d.\n",row_status));
          if(RSEMIB_OK == (ret = preserve_rsuWsaChannelStatus(index))){
              if(RSEMIB_OK == (ret = action_rsuWsaChannelStatus(index, row_status))){
                  if(RSEMIB_OK == (ret = commit_rsuWsaChannelStatus(index))){
                      DEBUGMSGTL((MY_NAME, "COMMIT DONE:\n"));
                  } else {
                      DEBUGMSGTL((MY_NAME, "COMMIT FAIL:\n"));
                  }
              } else {
                  DEBUGMSGTL((MY_NAME, "ACTION FAIL:\n"));
              }
              if(RSEMIB_OK != ret) {
                  if(RSEMIB_OK != undo_rsuWsaChannelStatus(index)) { /* UNDO fails nothing we can do. */
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
          DEBUGMSGTL((MY_NAME, "unknown mode (%d):\n",reqinfo->mode));
          ret = WSA_CONFIG_BAD_MODE;
          break;
  }
  return ret;
}
/* 5.10.3: Table handler: Break it down to smaller requests. */
int32_t handle_rsuWsaChannelTable( netsnmp_mib_handler          * handler
                                 , netsnmp_handler_registration * reginfo
                                 , netsnmp_agent_request_info   * reqinfo
                                 , netsnmp_request_info         * requests)
{
  netsnmp_request_info       * request       = NULL;
  uint32_t                     request_count = 0;
  RsuWsaChannelEntry_t       * table_entry   = NULL;
  netsnmp_table_request_info * request_info  = NULL;
  int32_t                      ret           = SNMP_ERR_NOERROR;
  uint8_t                      raw_oid[MAX_OID_LEN + 2]; /* If table doesn't exist then helper can't help. Go fish for row index. */
  uint8_t                      row_index = 0; /* For readability. Note: index is 1..maxRsuIFMs. */
  int32_t                      row_status = SNMP_ROW_NONEXISTENT;

  handler = handler; /* Silence of the Warnings. */
  reginfo = reginfo;

  DEBUGMSGTL((MY_NAME, "rsuWsaChannelTable_handler: Entry:\n"));
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
          set_rsuWsaConfig_error_states(WSA_CHANNEL_BAD_REQUEST_TABLE_INFO);
          DEBUGMSGTL((MY_NAME, "NULL == request_info.\n"));
          ret = SNMP_ERR_INCONSISTENTVALUE;
          break;
      }
      if(NULL != (table_entry = (RsuWsaChannelEntry_t  *)netsnmp_tdata_extract_entry(request))) {
          if((request_info->colnum < WSA_CHANNEL_INDEX_COLUMN) || (WSA_CHANNEL_STATUS_COLUMN < request_info->colnum)) {
              set_rsuWsaConfig_error_states(WSA_CHANNEL_BAD_REQUEST_COLUMN);
              ret = SNMP_ERR_INCONSISTENTVALUE;
              break;
          }
      }

      /* Grab row index from request. */
      memset(raw_oid,0x0,sizeof(raw_oid));
      memcpy(raw_oid,&(request->requestvb->name_loc[0]),MAX_OID_LEN);
      #if defined(EXTRA_DEBUG)
      print_rawOID(&raw_oid[0]);
      DEBUGMSGTL((MY_NAME_EXTRA, "raw oid: row index = %d, column = %d.\n",raw_oid[112],raw_oid[104]));
      #endif
      /* Do get() on rowStatus to see if it exists and error check index. */
      if(RSEMIB_OK != (ret = get_rsuWsaChannelStatus(raw_oid[112],&row_status))) {
          DEBUGMSGTL((MY_NAME, "get_rsuWsaChannelStatus Failed: ret=%d\n",ret));
          set_rsuWsaConfig_error_states(WSA_CHANNEL_BAD_INDEX);
          ret = SNMP_ERR_INCONSISTENTVALUE;
          break; /* Bad request. We are done. */
      } else {
          row_index = raw_oid[112];
      }

      /* Depending on what you do the row must exist. */
      switch (reqinfo->mode){
          case MODE_GET:
#if 0 /* read+create: Helper prevents us from getting here. */
              if (SNMP_ROW_NONEXISTENT == row_status){ /* Can't GET if it doesn't exist. */
                  set_rsuWsaConfig_error_states(WSA_CHANNEL_BAD_REQUEST_TABLE_ENTRY);
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
                  set_rsuWsaConfig_error_states(WSA_CHANNEL_BAD_REQUEST_TABLE_ENTRY);
                  DEBUGMSGTL((MY_NAME, "UNDO: No request info.\n"));
                  ret = SNMP_ERR_INCONSISTENTVALUE;
              }
              break;
          default: /* Doesth thou really care? Unanticipated request type.*/
              set_rsuWsaConfig_error_states(WSA_CONFIG_BAD_MODE);
              DEBUGMSGTL((MY_NAME, "unrecognized mode.\n"));
              ret = SNMP_ERR_INCONSISTENTVALUE;  
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
          if(WSA_STATUS_COLUMN != request_info->colnum){/* If row doesn't exist then create now. */
              ret = handle_rsuWsaChannelStatus(row_index, reqinfo, request, 1); /* Do not use value in request. Force createAndGo. */
              if(RSEMIB_OK != ret) {
                  DEBUGMSGTL((MY_NAME, "Create failed for ROW=%d COLUMN %d.\n", row_index, request_info->colnum));
                  set_rsuWsaConfig_error_states(WSA_CHANNEL_BAD_INDEX);
                  ret = SNMP_ERR_BADVALUE;
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
      DEBUGMSGTL((MY_NAME, "GO for handler ROW=%d COLUMN=%d.\n",row_index,request_info->colnum));
      switch (request_info->colnum){
          case WSA_CHANNEL_INDEX_COLUMN:
              ret = handle_rsuWsaChannelIndex(row_index, reqinfo, request);
              break;
          case WSA_CHANNEL_PSID_COLUMN:
              ret = handle_rsuWsaChannelPsid(row_index, reqinfo, request);
              break;
          case WSA_CHANNEL_NUMBER_COLUMN:
              ret = handle_rsuWsaChannelNumber(row_index, reqinfo, request);
              break;
          case WSA_CHANNEL_TX_POWER_COLUMN:
              ret = handle_rsuWsaChannelTxPowerLevel(row_index, reqinfo, request);
              break;
          case WSA_CHANNEL_ACCESS_COLUMN:
              ret = handle_rsuWsaChannelAccess(row_index, reqinfo, request);
              break;
          case WSA_CHANNEL_STATUS_COLUMN:
              ret = handle_rsuWsaChannelStatus(row_index, reqinfo, request,0);
              break;
          default:
             /* Ignore requests for columns outside defintion. Dont send error because it will kill snmpwalks. */
             break;
      }
      /* Convert from module error to SNMP request error. */
      ret = handle_rsemib_errors(ret);
  }/* for */

  /* 
   * Only top handler need do this. Set SNMP request error once. 
   * SNMP error is greater than zero unlike MIB which is less than zero.
   */
  if(SNMP_ERR_NOERROR < ret){
      netsnmp_set_request_error(reqinfo, requests, ret);
  }
#if defined(USE_PROCESSED_FLAG)
  requests->processed = 1; /* Good or bad, we handle request, GET or SET(R1 shortstack), only once. */
#endif

  DEBUGMSGTL((MY_NAME, "ret=%d:Exit.\n",ret));
  return ret;
}
/* 5.10.4 */
int32_t handle_rsuWsaVersion(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo,
                            netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret = 0;

  handler = handler; /* Silence of the Warnings. */
  reginfo = reginfo;

  DEBUGMSGTL((MY_NAME, "handle_rsuWsaVersion: Entry:\n"));
  switch (reqinfo->mode){
      case MODE_GET:
          DEBUGMSGTL((MY_NAME, "GET:\n"));
          if(0 <= (ret = get_rsuWsaVersion())){
              snmp_set_var_typed_integer(requests->requestvb, ASN_INTEGER, ret);
              ret = RSEMIB_OK;
          }
          break;
      default:
          DEBUGMSGTL((MY_NAME, "unknown mode (%d):\n",reqinfo->mode));
          ret = WSA_CONFIG_BAD_MODE;
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

/* 
 * Table Creation.
 */

/* 5.10.2..3: rsuWsaServiceTable & rsuWsaChannelTable creation functions. */
STATIC void rsuWsaServiceTable_removeEntry(netsnmp_tdata * table_data, netsnmp_tdata_row * row)
{
  RsuWsaServiceEntry_t * entry = NULL;

  if (NULL == row){
      DEBUGMSGTL((MY_NAME,"rsuWsaServiceTable_removeEntry NULL input.\n"));
      return;
  }
  if(NULL != (entry = (RsuWsaServiceEntry_t *)row->data)){
      SNMP_FREE(entry);
  } else {
      DEBUGMSGTL((MY_NAME,"rsuWsaServiceTable_removeEntry row data is NULL!.\n"));
  }
  if (table_data){
      netsnmp_tdata_remove_and_delete_row(table_data, row);
  } else {
      DEBUGMSGTL((MY_NAME,"rsuWsaServiceTable_removeEntry NULL input.\n"));
      netsnmp_tdata_delete_row(row); 
  }
  return;
}
STATIC void rsuWsaChannelTable_removeEntry(netsnmp_tdata * table_data, netsnmp_tdata_row * row)
{
  RsuWsaChannelEntry_t * entry = NULL;

  if (NULL == row){
      DEBUGMSGTL((MY_NAME,"rsuWsaChannelTable_removeEntry NULL input.\n"));
      return;
  }
  if (NULL != (entry = (RsuWsaChannelEntry_t *)row->data)){
      SNMP_FREE(entry);
  } else {
      DEBUGMSGTL((MY_NAME,"rsuWsaChannelTable_removeEntry row data is NULL!.\n"));
  }
  if (table_data){
      netsnmp_tdata_remove_and_delete_row(table_data, row);
  } else {
      DEBUGMSGTL((MY_NAME,"rsuWsaChannelTable_removeEntry NULL input.\n"));
      netsnmp_tdata_delete_row(row); 
  }
}
static void destroy_rsuWsaServiceTable(void)
{
  netsnmp_tdata_row  * row     = NULL;
  netsnmp_tdata_row  * nextrow = NULL;
  int32_t              i       = 0;

  if(NULL != rsuWsaServiceTable){
      if(NULL != (row = netsnmp_tdata_row_first(rsuWsaServiceTable))){
          for(i=0;(row != NULL) && (i < RSU_WSA_SERVICES_MAX); i++)   {
              nextrow = netsnmp_tdata_row_next(rsuWsaServiceTable, row);
              rsuWsaServiceTable_removeEntry(rsuWsaServiceTable, row);
              row = nextrow;
          }
      } else {
          DEBUGMSGTL((MY_NAME, "destroy_rsuWsaServiceTable: Table empty. Nothing to do.\n"));
       }
  } else {
      DEBUGMSGTL((MY_NAME, "destroy_rsuWsaServiceTable: Table is NULL. Did we get called before install?\n"));
  }
}
static void destroy_rsuWsaChannelTable(void)
{
  netsnmp_tdata_row  * row     = NULL;
  netsnmp_tdata_row  * nextrow = NULL;
  int32_t              i       = 0;
  if(NULL != rsuWsaChannelTable){
      if(NULL != (row = netsnmp_tdata_row_first(rsuWsaChannelTable))){
          for(i=0;(row != NULL) && (i < RSU_WSA_SERVICES_MAX); i++)   {
              nextrow = netsnmp_tdata_row_next(rsuWsaChannelTable, row);
              rsuWsaChannelTable_removeEntry(rsuWsaChannelTable, row);
              row = nextrow;
          }
      } else {
          DEBUGMSGTL((MY_NAME, "destroy_rsuWsaChannelTable: Table empty. Nothing to do.\n"));
      }
  } else {
      DEBUGMSGTL((MY_NAME, "destroy_rsuWsaChannelTable: Table is NULL. Did we get called before install?\n"));
  }
}
static int32_t rsuWsaService_createTable(netsnmp_tdata * table_data, RsuWsaServiceEntry_t * rsuWsaService)
{
  RsuWsaServiceEntry_t * entry    = NULL;
  netsnmp_tdata_row    * row      = NULL;
  int32_t                i        = 0;
  int32_t                valid_row_count = 0;

  if(NULL == table_data){
      DEBUGMSGTL((MY_NAME, "rsuWsaService_createTable FAIL: table_data == NULL.\n"));
      return -1;
  }

  for(i=0; i < RSU_WSA_SERVICES_MAX; i++){
      if((rsuWsaService[i].rsuWsaIndex < RSU_WSA_SERVICES_MIN) || (RSU_WSA_SERVICES_MAX < rsuWsaService[i].rsuWsaIndex)){
          return -1; /* Indices must be unique and greater than 1. */
      }
      if(RSEMIB_OK != valid_table_row(rsuWsaService[i].rsuWsaStatus)){
          continue; /* Only add rows that have beeb created. */
      }
      if (NULL == (entry = SNMP_MALLOC_TYPEDEF(RsuWsaServiceEntry_t))){
          DEBUGMSGTL((MY_NAME, "rsuWsaService SNMP_MALLOC_TYPEDEF FAIL.\n"));
          return -1;
      }
      if (NULL == (row = netsnmp_tdata_create_row())){
          DEBUGMSGTL((MY_NAME, "rsuWsaService netsnmp_tdata_create_row FAIL.\n"));
          SNMP_FREE(entry);
          return -1;
      }

      DEBUGMSGTL((MY_NAME, "rsuWsaService make row i=%d index=%d.\n",i,rsuWsaService[i].rsuWsaIndex));

      row->data = entry;
      memcpy(entry,&rsuWsaService[i],sizeof(RsuWsaServiceEntry_t));
      netsnmp_tdata_row_add_index(row, ASN_INTEGER , &rsuWsaService[i].rsuWsaIndex, sizeof(int32_t));
      if(SNMPERR_SUCCESS != netsnmp_tdata_add_row(table_data, row)){
          DEBUGMSGTL((MY_NAME, "rsuWsaService netsnmp_tdata_add_row FAIL i=%d.\n",i));
          return -1;
      }
      valid_row_count++;
  }
  DEBUGMSGTL((MY_NAME, "rsuWsaService valid rows created = %d.\n",valid_row_count));
  return RSEMIB_OK;
}
static int32_t rsuWsaChannel_createTable(netsnmp_tdata * table_data, RsuWsaChannelEntry_t * rsuWsaChannel)
{
  RsuWsaChannelEntry_t * entry    = NULL;
  netsnmp_tdata_row    * row      = NULL;
  int32_t                i        = 0;
  int32_t                valid_row_count = 0;

  if(NULL == table_data){
      DEBUGMSGTL((MY_NAME, "rsuWsaChannel_createTable FAIL: table_data == NULL.\n"));
      return -1;
  }

  for(i=0; i < RSU_WSA_SERVICES_MAX; i++){
      if((rsuWsaChannel[i].rsuWsaChannelIndex < RSU_WSA_SERVICES_MIN) || (RSU_WSA_SERVICES_MAX < rsuWsaChannel[i].rsuWsaChannelIndex)){
          return -1; /* Indices must be unique and greater than 1. */
      }
      if(RSEMIB_OK != valid_table_row(rsuWsaChannel[i].rsuWsaChannelStatus)){
          continue; /* Only add rows that have beeb created. */
      }
      if (NULL == (entry = SNMP_MALLOC_TYPEDEF(RsuWsaChannelEntry_t))){
          DEBUGMSGTL((MY_NAME, "rsuWsaChannel SNMP_MALLOC_TYPEDEF FAIL.\n"));
          return -1;
      }
      if (NULL == (row = netsnmp_tdata_create_row())){
          DEBUGMSGTL((MY_NAME, "rsuWsaChannel netsnmp_tdata_create_row FAIL.\n"));
          SNMP_FREE(entry);
          return -1;
      }

      DEBUGMSGTL((MY_NAME, "rsuWsaChannel make row i=%d index=%d.\n",i,rsuWsaChannel[i].rsuWsaChannelIndex));

      row->data = entry;
      memcpy(entry,&rsuWsaChannel[i],sizeof(RsuWsaChannelEntry_t));
      netsnmp_tdata_row_add_index(row, ASN_INTEGER , &rsuWsaChannel[i].rsuWsaChannelIndex, sizeof(int32_t));
      if(SNMPERR_SUCCESS != netsnmp_tdata_add_row(table_data, row)){
          DEBUGMSGTL((MY_NAME, "rsuWsaChannel netsnmp_tdata_add_row FAIL i=%d.\n",i));
          return -1;
      }
      valid_row_count++;
  } /* for */
  DEBUGMSGTL((MY_NAME, "rsuWsaChannel valid rows created = %d.\n",valid_row_count));
  return RSEMIB_OK;
}
static int32_t install_rsuWsaServiceTable(void)
{
    netsnmp_handler_registration * reg = NULL;

    DEBUGMSGTL((MY_NAME_EXTRA, "install_rsuWsaServiceTable: Entry.\n"));

    reg = netsnmp_create_handler_registration("rsuWsaServiceTable"
                                              ,handle_rsuWsaServiceTable
                                              ,rsuWsaServiceTable_oid
                                              ,OID_LENGTH(rsuWsaServiceTable_oid)
                                              ,HANDLER_CAN_RWRITE);

    if (NULL == (rsuWsaServiceTable = netsnmp_tdata_create_table("rsuWsaServiceTable", 0))){
        DEBUGMSGTL((MY_NAME, "rsuWsaServiceTable FAIL.\n"));
        return WSA_CONFIG_INSTALL_FAIL;
    }
    if (NULL == (service_table_reg_info = SNMP_MALLOC_TYPEDEF(netsnmp_table_registration_info))){
        DEBUGMSGTL((MY_NAME, "SNMP_MALLOC_TYPEDEF FAIL.\n"));
        return WSA_CONFIG_INSTALL_FAIL;
    }
    netsnmp_table_helper_add_indexes(service_table_reg_info, ASN_INTEGER,0); /* index */                                 

    service_table_reg_info->min_column = WSA_PSID_COLUMN;   /* Columns 2 to 16; Hide index by definition. */
    service_table_reg_info->max_column = WSA_ENABLE_COLUMN;

    if (SNMPERR_SUCCESS == netsnmp_tdata_register(reg, rsuWsaServiceTable, service_table_reg_info)){
        return RSEMIB_OK; /* Table registered but still empty. rebuild will populate */
    } else {
        DEBUGMSGTL((MY_NAME, "netsnmp_tdata_register FAIL.\n"));
        return WSA_CONFIG_INSTALL_FAIL;
    }
}
static int32_t install_rsuWsaChannelTable(void)
{
    netsnmp_handler_registration * reg = NULL;

    reg = netsnmp_create_handler_registration("rsuWsaChannelTable"
                                              ,handle_rsuWsaChannelTable
                                              ,rsuWsaChannelTable_oid
                                              ,OID_LENGTH(rsuWsaChannelTable_oid)
                                              ,HANDLER_CAN_RWRITE);

    if (NULL == (rsuWsaChannelTable = netsnmp_tdata_create_table("rsuWsaChannelTable", 0))){
        DEBUGMSGTL((MY_NAME, "rsuWsaChannelTable FAIL.\n"));
        return WSA_CONFIG_INSTALL_FAIL;
    }
    if (NULL == (channel_table_reg_info = SNMP_MALLOC_TYPEDEF(netsnmp_table_registration_info))){
        DEBUGMSGTL((MY_NAME, "SNMP_MALLOC_TYPEDEF FAIL.\n"));
        return WSA_CONFIG_INSTALL_FAIL;
    }
    netsnmp_table_helper_add_indexes(channel_table_reg_info, ASN_INTEGER,0); /* index */                                 

    channel_table_reg_info->min_column = WSA_CHANNEL_PSID_COLUMN;   /* Columns 2 to 6; Hide index by definition. */
    channel_table_reg_info->max_column = WSA_CHANNEL_STATUS_COLUMN;

    if (SNMPERR_SUCCESS == netsnmp_tdata_register(reg, rsuWsaChannelTable, channel_table_reg_info)){
        return RSEMIB_OK; /* Table registered but still empty. rebuild_MsgRepeat() will populate */
    } else {
        DEBUGMSGTL((MY_NAME, "netsnmp_tdata_register FAIL.\n"));
        return WSA_CONFIG_INSTALL_FAIL;
    }
}
int32_t rebuild_rsuWsaService_live(RsuWsaServiceEntry_t * rsuWsaService)
{
  if(NULL == rsuWsaService) {
      return WSA_CONFIG_BAD_INPUT;
  }
  /* Remove rows but don't remove table reg. */
  destroy_rsuWsaServiceTable();

  /* Initialise the contents of the table here.*/
  if (RSEMIB_OK != rsuWsaService_createTable(rsuWsaServiceTable,rsuWsaService)){
      DEBUGMSGTL((MY_NAME, "rebuild_rsuWsaServiceTable_live: ERROR: Null row.\n"));
      set_rsuWsaConfig_error_states(WSA_CONFIG_REBUILD_FAIL);
      return WSA_CONFIG_REBUILD_FAIL;
  }
  return RSEMIB_OK;
}
int32_t rebuild_rsuWsaChannel_live(RsuWsaChannelEntry_t * rsuWsaChannel)
{
  if(NULL == rsuWsaChannel) {
      return WSA_CONFIG_BAD_INPUT;
  }
  /* Remove rows but don't remove table reg. */
  destroy_rsuWsaChannelTable();

  /* Initialise the contents of the table here.*/
  if (RSEMIB_OK != rsuWsaChannel_createTable(rsuWsaChannelTable,rsuWsaChannel)){
      DEBUGMSGTL((MY_NAME, "rebuild_rsuWsaChannelTable_live: ERROR: Null row.\n"));
      set_rsuWsaConfig_error_states(WSA_CHANNEL_REBUILD_FAIL);
      return WSA_CHANNEL_REBUILD_FAIL;
  }
  return RSEMIB_OK;
}
static void rebuild_rsuWsaServiceTable(void)
{
  int32_t i = 0;
  RsuWsaServiceEntry_t rsuWsaService[RSU_WSA_SERVICES_MAX];

  memset(rsuWsaService, 0x0, sizeof(rsuWsaService));
  for(i=0; i < RSU_WSA_SERVICES_MAX; i++){
      if(RSEMIB_OK > get_rsuWsaIndex(i+1,&rsuWsaService[i].rsuWsaIndex)){
          set_rsuWsaConfig_error_states(WSA_CONFIG_REBUILD_FAIL);
          DEBUGMSGTL((MY_NAME, "rsuWsaIndex rebuild FAIL: i = %d.\n",i));
      }
      if(RSEMIB_OK > get_rsuWsaPsid(i+1,(uint8_t *)rsuWsaService[i].rsuWsaPsid)){
          set_rsuWsaConfig_error_states(WSA_CONFIG_REBUILD_FAIL);
          DEBUGMSGTL((MY_NAME, "rsuWsaPsid rebuild FAIL: i = %d.\n",i));
      }
      if(RSEMIB_OK > get_rsuWsaPSC(i+1,rsuWsaService[i].rsuWsaPSC)){
          set_rsuWsaConfig_error_states(WSA_CONFIG_REBUILD_FAIL);
          DEBUGMSGTL((MY_NAME, "rsuWsaPSC rebuild FAIL: i = %d.\n",i));
      }
      if(RSEMIB_OK > get_rsuWsaIpAddress(i+1,rsuWsaService[i].rsuWsaIpAddress)){
          set_rsuWsaConfig_error_states(WSA_CONFIG_REBUILD_FAIL);
          DEBUGMSGTL((MY_NAME, "rsuWsaIpAddress rebuild FAIL: i = %d.\n",i));
      }
      if(RSEMIB_OK > get_rsuWsaPort(i+1,&rsuWsaService[i].rsuWsaPort)){
          set_rsuWsaConfig_error_states(WSA_CONFIG_REBUILD_FAIL);
          DEBUGMSGTL((MY_NAME, "rsuWsaPort rebuild FAIL: i = %d.\n",i));
      }
      if(RSEMIB_OK > get_rsuWsaChannel(i+1,&rsuWsaService[i].rsuWsaChannel)){
          set_rsuWsaConfig_error_states(WSA_CONFIG_REBUILD_FAIL);
          DEBUGMSGTL((MY_NAME, "rsuWsaChannel rebuild FAIL: i = %d.\n",i));
      }
      if(RSEMIB_OK > get_rsuWsaStatus(i+1,&rsuWsaService[i].rsuWsaStatus)){
          set_rsuWsaConfig_error_states(WSA_CONFIG_REBUILD_FAIL);
          DEBUGMSGTL((MY_NAME, "rsuWsaStatus rebuild FAIL: i = %d.\n",i));
      }
      if(RSEMIB_OK > get_rsuWsaMacAddress(i+1,rsuWsaService[i].rsuWsaMacAddress)){
          set_rsuWsaConfig_error_states(WSA_CONFIG_REBUILD_FAIL);
          DEBUGMSGTL((MY_NAME, "rsuWsaMacAddress rebuild FAIL: i = %d.\n",i));
      }
      if(RSEMIB_OK > get_rsuWsaOptions(i+1, &rsuWsaService[i].rsuWsaOptions)){
          set_rsuWsaConfig_error_states(WSA_CONFIG_REBUILD_FAIL);
          DEBUGMSGTL((MY_NAME, "rsuWsaOptions rebuild FAIL: i = %d.\n",i));
      }
      if(RSEMIB_OK > get_rsuWsaRcpiThreshold(i+1,&rsuWsaService[i].rsuWsaRcpiThreshold)){
          set_rsuWsaConfig_error_states(WSA_CONFIG_REBUILD_FAIL);
          DEBUGMSGTL((MY_NAME, "rsuWsaRcpiThreshold rebuild FAIL: i = %d.\n",i));
      }
      if(RSEMIB_OK > get_rsuWsaCountThreshold(i+1, &rsuWsaService[i].rsuWsaCountThreshold)){
          set_rsuWsaConfig_error_states(WSA_CONFIG_REBUILD_FAIL);
          DEBUGMSGTL((MY_NAME, "rsuWsaCountThreshold rebuild FAIL: i = %d.\n",i));
      }
      if(RSEMIB_OK > get_rsuWsaCountThresholdInterval(i+1, &rsuWsaService[i].rsuWsaCountThresholdInterval)){
          set_rsuWsaConfig_error_states(WSA_CONFIG_REBUILD_FAIL);
          DEBUGMSGTL((MY_NAME, "rsuWsaCountThresholdInterval rebuild FAIL: i = %d.\n",i));
      }
      if(RSEMIB_OK > get_rsuWsaRepeatRate(i+1, &rsuWsaService[i].rsuWsaRepeatRate)){
          set_rsuWsaConfig_error_states(WSA_CONFIG_REBUILD_FAIL);
          DEBUGMSGTL((MY_NAME, "rsuWsaRepeatRate rebuild FAIL: i = %d.\n",i));
      }
      if(RSEMIB_OK > get_rsuWsaAdvertiserIdentifier(i+1, rsuWsaService[i].rsuWsaAdvertiserIdentifier)){
          set_rsuWsaConfig_error_states(WSA_CONFIG_REBUILD_FAIL);
          DEBUGMSGTL((MY_NAME, "rsuWsaAdvertiserIdentifier rebuild FAIL: i = %d.\n",i));
      }
      if(RSEMIB_OK > get_rsuWsaEnable(i+1, &rsuWsaService[i].rsuWsaEnable)){
          set_rsuWsaConfig_error_states(WSA_CONFIG_REBUILD_FAIL);
          DEBUGMSGTL((MY_NAME, "rsuWsaEnable rebuild FAIL: i = %d.\n",i));
      }
  }

  /* Remove rows but don't remove helper regisrty etc. */
  destroy_rsuWsaServiceTable();

  /* Initialize the contents of the table here. */
  if (RSEMIB_OK != rsuWsaService_createTable(rsuWsaServiceTable, &rsuWsaService[0])){
      DEBUGMSGTL((MY_NAME, "rsuWsaService_createTable FAIL.\n"));
      set_rsuWsaConfig_error_states(WSA_CONFIG_REBUILD_FAIL);
  }
}
static void rebuild_rsuWsaChannelTable(void)
{
  int32_t i = 0;
  RsuWsaChannelEntry_t rsuWsaChannel[RSU_WSA_SERVICES_MAX];

  memset(rsuWsaChannel, 0x0, sizeof(rsuWsaChannel));
  for(i=0; i < RSU_WSA_SERVICES_MAX; i++){
      if(RSEMIB_OK > get_rsuWsaChannelIndex(i+1,&rsuWsaChannel[i].rsuWsaChannelIndex)){
          set_rsuWsaConfig_error_states(WSA_CHANNEL_REBUILD_FAIL);
          DEBUGMSGTL((MY_NAME, "rsuWsaChannelIndex rebuild FAIL: i = %d.\n",i));
      }
      if(RSEMIB_OK > get_rsuWsaChannelPsid(i+1,rsuWsaChannel[i].rsuWsaChannelPsid)){
          set_rsuWsaConfig_error_states(WSA_CHANNEL_REBUILD_FAIL);
          DEBUGMSGTL((MY_NAME, "rsuWsaChannelPsid rebuild FAIL: i = %d.\n",i));
      }
      if(RSEMIB_OK > get_rsuWsaChannelNumber(i+1,&rsuWsaChannel[i].rsuWsaChannelNumber)){
          set_rsuWsaConfig_error_states(WSA_CHANNEL_REBUILD_FAIL);
          DEBUGMSGTL((MY_NAME, "rsuWsaChannelNumber rebuild FAIL: i = %d.\n",i));
      }
      if(RSEMIB_OK > get_rsuWsaChannelTxPowerLevel(i+1,&rsuWsaChannel[i].rsuWsaChannelTxPowerLevel)){
          set_rsuWsaConfig_error_states(WSA_CHANNEL_REBUILD_FAIL);
          DEBUGMSGTL((MY_NAME, "rsuWsaChannelTxPowerLevel rebuild FAIL: i = %d.\n",i));
      }
      if(RSEMIB_OK > get_rsuWsaChannelAccess(i+1,&rsuWsaChannel[i].rsuWsaChannelAccess)){
          set_rsuWsaConfig_error_states(WSA_CHANNEL_REBUILD_FAIL);
          DEBUGMSGTL((MY_NAME, "rsuWsaChannelAccess rebuild FAIL: i = %d.\n",i));
      }
      if(RSEMIB_OK > get_rsuWsaChannelStatus(i+1,&rsuWsaChannel[i].rsuWsaChannelStatus)){
          set_rsuWsaConfig_error_states(WSA_CHANNEL_REBUILD_FAIL);
          DEBUGMSGTL((MY_NAME, "rsuWsaChannelStatus rebuild FAIL: i = %d.\n",i));
      }
  }

  /* Remove rows but don't remove helper regisrty etc. */
  destroy_rsuWsaChannelTable();

  /* Initialize the contents of the table here. */
  if (RSEMIB_OK != rsuWsaChannel_createTable(rsuWsaChannelTable, &rsuWsaChannel[0])){
      DEBUGMSGTL((MY_NAME, "rsuWsaChannel_createTable FAIL.\n"));
      set_rsuWsaConfig_error_states(WSA_CHANNEL_REBUILD_FAIL);
  }
}
void install_rsuWsaConfig_handlers(void)
{
  /* Reset statics across soft reset. */
  service_table_reg_info    = NULL;
  channel_table_reg_info    = NULL;
  rsuWsaServiceTable        = NULL;
  rsuWsaChannelTable        = NULL;
  rsuWsaConfig_error_states = 0x0;

  /* 5.10.1. */
  if(MIB_REGISTERED_OK != netsnmp_register_scalar(netsnmp_create_handler_registration
                          ("maxRsuWsaServices", 
                           handle_maxRsuWsaServices,
                           maxRsuWsaServices_oid,
                           OID_LENGTH(maxRsuWsaServices_oid),
                           HANDLER_CAN_RONLY))){
      DEBUGMSGTL((MY_NAME, "maxRsuWsaServices install FAIL.\n"));
      set_rsuWsaConfig_error_states(WSA_CONFIG_INSTALL_FAIL);
  }
  /* 5.10.2.1 - 5.10.2.16: Install table created with data from MIB. */
  if( RSEMIB_OK != install_rsuWsaServiceTable()){
      DEBUGMSGTL((MY_NAME, "rsuWsaServiceTable install FAIL.\n"));
      set_rsuWsaConfig_error_states(WSA_CONFIG_INSTALL_FAIL);
  }
  /* 5.10.3.1 - 5.10.3.6: Install table created with data from MIB. */
  if( RSEMIB_OK != install_rsuWsaChannelTable()){
      DEBUGMSGTL((MY_NAME, "rsuWsaChannelTable install FAIL.\n"));
      set_rsuWsaConfig_error_states(WSA_CHANNEL_INSTALL_FAIL);
  }
  /* 5.10.4. */
  if(MIB_REGISTERED_OK != netsnmp_register_scalar(netsnmp_create_handler_registration
                          ("rsuWsaVersion", 
                           handle_rsuWsaVersion,
                           rsuWsaVersion_oid,
                           OID_LENGTH(rsuWsaVersion_oid),
                           HANDLER_CAN_RONLY))){
      DEBUGMSGTL((MY_NAME, "rsuWsaVersion install FAIL.\n"));
      set_rsuWsaConfig_error_states(WSA_CONFIG_INSTALL_FAIL);
  }

  /* Populate the table. */
  rebuild_rsuWsaServiceTable();
  rebuild_rsuWsaChannelTable();
}
void destroy_rsuWsaConfig(void)
{
  destroy_rsuWsaServiceTable();
  destroy_rsuWsaChannelTable();

  /* Free table info. */
  if (NULL != service_table_reg_info){
      SNMP_FREE(service_table_reg_info);
  }
  /* Free table reg. */
  if (NULL != rsuWsaServiceTable){
      SNMP_FREE(rsuWsaServiceTable);
  }
  /* Free table info. */
  if (NULL != channel_table_reg_info){
      SNMP_FREE(channel_table_reg_info);
  }
  /* Free table reg. */
  if (NULL != rsuWsaChannelTable){
      SNMP_FREE(rsuWsaChannelTable);
  }
}
