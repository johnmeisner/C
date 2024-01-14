/*
*  Filename: cfgmgrapi.c
*  Purpose: Configuration Manager API
*
*  Copyright (C) 2021 DENSO International America, Inc.
*
*  Cfgmgr is a stateless utility; it is not a service.
*
*  Revision History:
*
*  Date        Author        Comments
* --------------------------------------------------
*  07-16-19    JJG         Initial release.
*  12-08-2020  KF          Changes from Util "dc" renamed to "jcmu"
*  01-04-2021  KF          Util "jcmu" renamed to "dcu", renamed in here too
*  02-01-2021  KF          Added some error checking and error output
*  02-18-2021  KF          Renamed netCfg functions to more specific, dcu debug -d removed
*
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include <netinet/in.h>
#include <net/if.h>
#include <netdb.h>

#include "dn_types.h"
#include "i2v_general.h"
#include "ris_struct.h"
#include "cfgmgr.h"
#include "cfgmgrapi.h"
#include "conf_table.h"
#include "i2v_util.h"
#include "ntcip-1218.h"
/* temporary */
#define DIAG_OPS 1

STATIC int32_t cfgCheckI2V(char_t * param, char_t * value) 
{
  int32_t  ret = CFG_AOK; 
  uint32_t i2vRet = I2V_RETURN_OK;
  uint32_t status = I2V_RETURN_OK; /* Don't care, check return of function. */
  uint64_t configItem = 0; /* Dont care, just needs to be big enough to fit conf value. */
  char_t   ip_addr[I2V_DEST_IP_MAX + 1];
  char_t   sspString[MAX_SSP_DATA + 1];

  if ((NULL == param) || (NULL == value)) {
      ret = CFG_PARAM_FAIL;
  } else {
      if((0 == strlen(value)) || (I2V_CFG_MAX_STR_LEN < strlen(value))){
          ret = CFG_ITEM_LENGTH_BAD; /* Empty or larger than what I2V confs can handle in I2V_UTIL. */
      }
  }
  if(CFG_AOK == ret) {
      if(0 == strcmp("I2VUnifiedChannelNumber",param)) {
          i2vRet = i2vUtilUpdateUint8Value(&configItem,value,I2V_RADIO_CHANNEL_MIN_S,I2V_RADIO_CHANNEL_MAX_S,NULL,&status,NULL);
      } else if(0 == strcmp("I2VTransmitPower",param)) {
          ret = i2vCheckTxPower(value);
      } else if(0 == strcmp("I2VRSUID",param)) {
          i2vRet = i2vUtilUpdateUint16Value(&configItem,value,I2VRSUID_MIN_S,I2VRSUID_MAX_S,NULL,&status,NULL);
      } else if(0 == strcmp("I2VSCSAppEnable",param)) {
          i2vRet = i2vUtilUpdateWBOOLValue(&configItem,value,I2V_ENABLE_MIN_S,I2V_ENABLE_MAX_S,NULL,&status,NULL);
      } else if(0 == strcmp("I2VSRMAppEnable",param)) {
          i2vRet = i2vUtilUpdateWBOOLValue(&configItem,value,I2V_ENABLE_MIN_S,I2V_ENABLE_MAX_S,NULL,&status,NULL);
      } else if(0 == strcmp("I2VIPBAppEnable",param)) {
          i2vRet = i2vUtilUpdateWBOOLValue(&configItem,value,I2V_ENABLE_MIN_S,I2V_ENABLE_MAX_S,NULL,&status,NULL);
      } else if(0 == strcmp("I2VAMHAppEnable",param)) {
          i2vRet = i2vUtilUpdateWBOOLValue(&configItem,value,I2V_ENABLE_MIN_S,I2V_ENABLE_MAX_S,NULL,&status,NULL);
      } else if(0 == strcmp("I2VIWMHAppEnable",param)) {
          i2vRet = i2vUtilUpdateWBOOLValue(&configItem,value,I2V_ENABLE_MIN_S,I2V_ENABLE_MAX_S,NULL,&status,NULL);
      } else if(0 == strcmp("I2VFWDMSGAppEnable",param)) {
          i2vRet = i2vUtilUpdateWBOOLValue(&configItem,value,I2V_ENABLE_MIN_S,I2V_ENABLE_MAX_S,NULL,&status,NULL);
      } else if(0 == strcmp("I2VGlobalDebugFlag",param)) {
          i2vRet = i2vUtilUpdateWBOOLValue(&configItem,value,I2V_ENABLE_MIN_S,I2V_ENABLE_MAX_S,NULL,&status,NULL);
      } else if(0 == strcmp("I2VDebugFlag",param)) {
          i2vRet = i2vUtilUpdateWBOOLValue(&configItem,value,I2V_ENABLE_MIN_S,I2V_ENABLE_MAX_S,NULL,&status,NULL);
      } else if(0 == strcmp("I2VGPSOverrideEnable",param)) {
          i2vRet = i2vUtilUpdateWBOOLValue(&configItem,value,I2V_ENABLE_MIN_S,I2V_ENABLE_MAX_S,NULL,&status,NULL);
      } else if(0 == strcmp("I2VGPSOverrideLat",param)) {
          i2vRet = i2vUtilUpdateDoubleValue(&configItem,value,I2V_LAT_DEG_MIN_S,I2V_LAT_DEG_MAX_S,NULL,&status,NULL);
      } else if(0 == strcmp("I2VGPSOverrideLong",param)) {
          i2vRet = i2vUtilUpdateDoubleValue(&configItem,value,I2V_LON_DEG_MIN_S,I2V_LON_DEG_MAX_S,NULL,&status,NULL);
      } else if(0 == strcmp("I2VGPSOverrideElev",param)) {
          i2vRet = i2vUtilUpdateInt16Value(&configItem,value,I2V_ELV_M_MIN_S,I2V_ELV_M_MAX_S,NULL,&status,NULL);
      } else if(0 == strcmp("I2VEnableEthIPv6",param)) {
          i2vRet = i2vUtilUpdateWBOOLValue(&configItem,value,I2V_ENABLE_MIN_S,I2V_ENABLE_MAX_S,NULL,&status,NULL);
      } else if(0 == strcmp("I2VEthIPv6Addr",param)) {
          if(I2V_RETURN_OK == (i2vRet = i2vUtilUpdateStrValue(&ip_addr,value,I2V_DEST_IP_MIN_S,I2V_DEST_IP_MAX_S,NULL,&status,NULL))){
            ret = i2vCheckIPAddr(value,I2V_IP_ADDR_V6);
          }
      } else if(0 == strcmp("I2VEthIPv6Scope",param)) {
          i2vRet = i2vUtilUpdateUint8Value(&configItem,value,I2V_ETH_IPV6_SCOPE_MIN_S,I2V_ETH_IPV6_SCOPE_MAX_S,NULL,&status,NULL);
      } else if(0 == strcmp("I2VIPv6DefaultGateway",param)) {
          if(I2V_RETURN_OK == (i2vRet = i2vUtilUpdateStrValue(&ip_addr,value,I2V_DEST_IP_MIN_S,I2V_DEST_IP_MAX_S,NULL,&status,NULL))){
            ret = i2vCheckIPAddr(value,I2V_IP_ADDR_V6);
          }
      } else if(0 == strcmp("I2VForceAlive",param)) {
          i2vRet = i2vUtilUpdateWBOOLValue(&configItem,value,I2V_ENABLE_MIN_S,I2V_ENABLE_MAX_S,NULL,&status,NULL);
      } else if(0 == strcmp("I2VControlRadio",param)) {
          i2vRet = i2vUtilUpdateUint8Value(&configItem,value,I2V_CONTROL_RADIO_MIN_S,I2V_CONTROL_RADIO_MAX_S,NULL,&status,NULL);
      } else if(0 == strcmp("I2VRadioSecVerify",param)) {
          i2vRet = i2vUtilUpdateWBOOLValue(&configItem,value,I2V_ENABLE_MIN_S,I2V_ENABLE_MAX_S,NULL,&status,NULL);
      } else if(0 == strcmp("I2VRadio0Enable",param)) {
          i2vRet = i2vUtilUpdateWBOOLValue(&configItem,value,I2V_ENABLE_MIN_S,I2V_ENABLE_MAX_S,NULL,&status,NULL);
      } else if(0 == strcmp("I2VRadio0Mode",param)) {
          i2vRet = i2vUtilUpdateWBOOLValue(&configItem,value,I2V_ENABLE_MIN_S,I2V_ENABLE_MAX_S,NULL,&status,NULL);
      } else if(0 == strcmp("I2VRadio0Channel",param)) {
          i2vRet = i2vUtilUpdateUint8Value(&configItem,value,I2V_RADIO_CHANNEL_MIN_S,I2V_RADIO_CHANNEL_MAX_S,NULL,&status,NULL);
      } else if(0 == strcmp("I2VRadio1Enable",param)) {
          i2vRet = i2vUtilUpdateWBOOLValue(&configItem,value,I2V_ENABLE_MIN_S,I2V_ENABLE_MAX_S,NULL,&status,NULL);
      } else if(0 == strcmp("I2VRadio1Mode",param)) {
          i2vRet = i2vUtilUpdateWBOOLValue(&configItem,value,I2V_ENABLE_MIN_S,I2V_ENABLE_MAX_S,NULL,&status,NULL);
      } else if(0 == strcmp("I2VRadio1Channel",param)) {
          i2vRet = i2vUtilUpdateUint8Value(&configItem,value,I2V_RADIO_CHANNEL_MIN_S,I2V_RADIO_CHANNEL_MAX_S,NULL,&status,NULL);
      } else if(0 == strcmp("I2VRadio2Enable",param)) {
          i2vRet = i2vUtilUpdateWBOOLValue(&configItem,value,I2V_ENABLE_MIN_S,I2V_ENABLE_MAX_S,NULL,&status,NULL);
      } else if(0 == strcmp("I2VRadio2Mode",param)) {
          i2vRet = i2vUtilUpdateWBOOLValue(&configItem,value,I2V_ENABLE_MIN_S,I2V_ENABLE_MAX_S,NULL,&status,NULL);
      } else if(0 == strcmp("I2VRadio2Channel",param)) {
          i2vRet = i2vUtilUpdateUint8Value(&configItem,value,I2V_RADIO_CHANNEL_MIN_S,I2V_RADIO_CHANNEL_MAX_S,NULL,&status,NULL);
      } else if(0 == strcmp("I2VRadio3Enable",param)) {
          i2vRet = i2vUtilUpdateWBOOLValue(&configItem,value,I2V_ENABLE_MIN_S,I2V_ENABLE_MAX_S,NULL,&status,NULL);
      } else if(0 == strcmp("I2VRadio3Mode",param)) {
          i2vRet = i2vUtilUpdateWBOOLValue(&configItem,value,I2V_ENABLE_MIN_S,I2V_ENABLE_MAX_S,NULL,&status,NULL);
      } else if(0 == strcmp("I2VRadio3Channel",param)) {
          i2vRet = i2vUtilUpdateUint8Value(&configItem,value,I2V_RADIO_CHANNEL_MIN_S,I2V_RADIO_CHANNEL_MAX_S,NULL,&status,NULL);
      } else if(0 == strcmp("I2VBroadcastUnified",param)) {
          i2vRet = i2vUtilUpdateWBOOLValue(&configItem,value,I2V_ENABLE_MIN_S,I2V_ENABLE_MAX_S,NULL,&status,NULL);
      } else if(0 == strcmp("I2VUnifiedRadioNumber",param)) {
          i2vRet = i2vUtilUpdateUint8Value(&configItem,value,I2V_RADIO_MIN_S,I2V_RADIO_MAX_S,NULL,&status,NULL);
      } else if(0 == strcmp("I2VUnifiedWSAPriority",param)) { 
          i2vRet = i2vUtilUpdateUint8Value(&configItem,value,I2V_WSA_PRIORITY_MIN_S,I2V_WSA_PRIORITY_MAX_S,NULL,&status,NULL);
      } else if(0 == strcmp("I2VSecurityEnable",param)) { 
          i2vRet = i2vUtilUpdateWBOOLValue(&configItem,value,I2V_ENABLE_MIN_S,I2V_ENABLE_MAX_S,NULL,&status,NULL);
      } else if(0 == strcmp("I2VSecurityTimeBuffer",param)) { 
          i2vRet = i2vUtilUpdateUint8Value(&configItem,value,I2V_SEC_TIME_BUFFER_MIN_S,I2V_SEC_TIME_BUFFER_MAX_S,NULL,&status,NULL);
      } else if(0 == strcmp("I2VCertAttachRate",param)) {
          i2vRet = i2vUtilUpdateUint8Value(&configItem,value,I2V_CERT_ATTACH_RATE_MIN_S,I2V_CERT_ATTACH_RATE_MAX_S,NULL,&status,NULL);
      } else if(0 == strcmp("I2VGenDefaultCert",param)) { 
          i2vRet = i2vUtilUpdateWBOOLValue(&configItem,value,I2V_ENABLE_MIN_S,I2V_ENABLE_MAX_S,NULL,&status,NULL);
      } else if(0 == strcmp("I2VAutoGenCertRestart",param)) { 
          i2vRet = i2vUtilUpdateWBOOLValue(&configItem,value,I2V_ENABLE_MIN_S,I2V_ENABLE_MAX_S,NULL,&status,NULL);
      } else if(0 == strcmp("I2VStorageBypass",param)) { 
          i2vRet = i2vUtilUpdateWBOOLValue(&configItem,value,I2V_ENABLE_MIN_S,I2V_ENABLE_MAX_S,NULL,&status,NULL);
      } else if(0 == strcmp("I2VDisableSysLog",param)) { 
          i2vRet = i2vUtilUpdateWBOOLValue(&configItem,value,I2V_ENABLE_MIN_S,I2V_ENABLE_MAX_S,NULL,&status,NULL);
      } else if(0 == strcmp("I2VConsoleCleanup",param)) { 
          i2vRet = i2vUtilUpdateWBOOLValue(&configItem,value,I2V_ENABLE_MIN_S,I2V_ENABLE_MAX_S,NULL,&status,NULL);
      } else if(0 == strcmp("I2VSPATSecPSID",param)) {
          if(I2V_RETURN_OK == (i2vRet = i2vUtilUpdateUint32Value(&configItem,value,I2V_PSID_MIN_S,I2V_PSID_MAX_S,NULL,&status,NULL))){
              ret = i2vCheckPSID(value);
          }
      } else if(0 == strcmp("I2VMAPSecPSID",param)) { 
          if(I2V_RETURN_OK == (i2vRet = i2vUtilUpdateUint32Value(&configItem,value,I2V_PSID_MIN_S,I2V_PSID_MAX_S,NULL,&status,NULL))){
              ret = i2vCheckPSID(value);
          }
      } else if(0 == strcmp("I2VTIMSecPSID",param)) { 
          if(I2V_RETURN_OK == (i2vRet = i2vUtilUpdateUint32Value(&configItem,value,I2V_PSID_MIN_S,I2V_PSID_MAX_S,NULL,&status,NULL))){
              ret = i2vCheckPSID(value);
          }
      } else if(0 == strcmp("I2VIPBSecPSID",param)) { 
          if(I2V_RETURN_OK == (i2vRet = i2vUtilUpdateUint32Value(&configItem,value,I2V_PSID_MIN_S,I2V_PSID_MAX_S,NULL,&status,NULL))){
              ret = i2vCheckPSID(value);
          }
      } else if(0 == strcmp("BSMRxPSIDDER",param)) { 
          if(I2V_RETURN_OK == (i2vRet = i2vUtilUpdateUint32Value(&configItem,value,I2V_PSID_MIN_S,I2V_PSID_MAX_S,NULL,&status,NULL))){
              ret = i2vCheckPSID(value);
          }
      } else if(0 == strcmp("BSMRxPSIDPERA",param)) { 
          if(I2V_RETURN_OK == (i2vRet = i2vUtilUpdateUint32Value(&configItem,value,I2V_PSID_MIN_S,I2V_PSID_MAX_S,NULL,&status,NULL))){
              ret = i2vCheckPSID(value);
          }
      } else if(0 == strcmp("BSMRxPSIDPERU",param)) { 
          if(I2V_RETURN_OK == (i2vRet = i2vUtilUpdateUint32Value(&configItem,value,I2V_PSID_MIN_S,I2V_PSID_MAX_S,NULL,&status,NULL))){
              ret = i2vCheckPSID(value);
          }
      } else if(0 == strcmp("I2VDirectASCSNMP",param)) { 
          i2vRet = i2vUtilUpdateWBOOLValue(&configItem,value,I2V_ENABLE_MIN_S,I2V_ENABLE_MAX_S,NULL,&status,NULL);
      } else if(0 == strcmp("I2VWSAEnable",param)) { 
          i2vRet = i2vUtilUpdateWBOOLValue(&configItem,value,I2V_ENABLE_MIN_S,I2V_ENABLE_MAX_S,NULL,&status,NULL);
      } else if(0 == strcmp("I2VRadioType",param)) {
          i2vRet = i2vUtilUpdateUint32Value(&configItem,value,I2V_RADIO_TYPE_MIN_S,I2V_RADIO_TYPE_MAX_S,NULL,&status,NULL);
      } else if(0 == strcmp("I2VSPATSSPEnable",param)) { 
          i2vRet = i2vUtilUpdateWBOOLValue(&configItem,value,I2V_ENABLE_MIN_S,I2V_ENABLE_MAX_S,NULL,&status,NULL);
      } else if(0 == strcmp("I2VSPATBITMAPPEDSSPEnable",param)) { 
          i2vRet = i2vUtilUpdateWBOOLValue(&configItem,value,I2V_ENABLE_MIN_S,I2V_ENABLE_MAX_S,NULL,&status,NULL);
      } else if(0 == strcmp("I2VSPATSspString",param)) {
          if(I2V_RETURN_OK == (i2vRet = i2vUtilUpdateStrValue(&sspString,value,MIN_SSP_DATA_S,MAX_SSP_DATA_S,NULL,&status,NULL))){
              ret = i2vCheckSspString(value);
          }
      } else if(0 == strcmp("I2VSPATSspMaskString",param)) { 
          if(I2V_RETURN_OK == (i2vRet = i2vUtilUpdateStrValue(&sspString,value,MIN_SSP_DATA_S,MAX_SSP_DATA_S,NULL,&status,NULL))){
              ret = i2vCheckSspString(value);
          }
      } else if(0 == strcmp("I2VMAPSSPEnable",param)) { 
          i2vRet = i2vUtilUpdateWBOOLValue(&configItem,value,I2V_ENABLE_MIN_S,I2V_ENABLE_MAX_S,NULL,&status,NULL);
      } else if(0 == strcmp("I2VMAPBITMAPPEDSSPEnable",param)) { 
          i2vRet = i2vUtilUpdateWBOOLValue(&configItem,value,I2V_ENABLE_MIN_S,I2V_ENABLE_MAX_S,NULL,&status,NULL);
      } else if(0 == strcmp("I2VMAPSspString",param)) { 
          if(I2V_RETURN_OK == (i2vRet = i2vUtilUpdateStrValue(&sspString,value,MIN_SSP_DATA_S,MAX_SSP_DATA_S,NULL,&status,NULL))){
              ret = i2vCheckSspString(value);
          }
      } else if(0 == strcmp("I2VMAPSspMaskString",param)) { 
          if(I2V_RETURN_OK == (i2vRet = i2vUtilUpdateStrValue(&sspString,value,MIN_SSP_DATA_S,MAX_SSP_DATA_S,NULL,&status,NULL))){
              ret = i2vCheckSspString(value);
          }
      } else if(0 == strcmp("I2VTIMSSPEnable",param)) { 
          i2vRet = i2vUtilUpdateWBOOLValue(&configItem,value,I2V_ENABLE_MIN_S,I2V_ENABLE_MAX_S,NULL,&status,NULL);
      } else if(0 == strcmp("I2VTIMBITMAPPEDSSPEnable",param)) { 
          i2vRet = i2vUtilUpdateWBOOLValue(&configItem,value,I2V_ENABLE_MIN_S,I2V_ENABLE_MAX_S,NULL,&status,NULL);
      } else if(0 == strcmp("I2VTIMSspString",param)) { 
          if(I2V_RETURN_OK == (i2vRet = i2vUtilUpdateStrValue(&sspString,value,MIN_SSP_DATA_S,MAX_SSP_DATA_S,NULL,&status,NULL))){
              ret = i2vCheckSspString(value);
          }
      } else if(0 == strcmp("I2VTIMSspMaskString",param)) { 
          if(I2V_RETURN_OK == (i2vRet = i2vUtilUpdateStrValue(&sspString,value,MIN_SSP_DATA_S,MAX_SSP_DATA_S,NULL,&status,NULL))){
              ret = i2vCheckSspString(value);
          }
      } else if(0 == strcmp("I2VUseDeviceID",param)) { 
          i2vRet = i2vUtilUpdateWBOOLValue(&configItem,value,I2V_ENABLE_MIN_S,I2V_ENABLE_MAX_S,NULL,&status,NULL);
      } else if(0 == strcmp("I2VUseGPSTXControl",param)) { 
          i2vRet = i2vUtilUpdateWBOOLValue(&configItem,value,I2V_ENABLE_MIN_S,I2V_ENABLE_MAX_S,NULL,&status,NULL);
      } else if(0 == strcmp("I2VEnableVOD",param)) { 
          i2vRet = i2vUtilUpdateWBOOLValue(&configItem,value,I2V_ENABLE_MIN_S,I2V_ENABLE_MAX_S,NULL,&status,NULL);
      } else if(0 == strcmp("I2VSecurityVTPMsgRateMs",param)) { 
          i2vRet = i2vUtilUpdateUint16Value(&configItem,value,I2V_SECURITY_VTP_MSG_RATE_MIN_S,I2V_SECURITY_VTP_MSG_RATE_MAX_S,NULL,&status,NULL);
      } else {
          ret = CFG_VALUE_UNKOWN_PARAM;
      }
      if ((CFG_AOK == ret) && (I2V_RETURN_OK != i2vRet)){
          ret = CFG_SET_VALUE_RANGE_ERROR;
      }
  }
  return ret;
}
STATIC int32_t cfgCheckAMH(char_t * param, char_t * value) 
{
  int32_t  ret = CFG_AOK; /* SUCCESS till proven FAIL. No match is a don't care, handled elsewhere. */
  uint32_t i2vRet = I2V_RETURN_OK;
  uint32_t status = I2V_RETURN_OK; /* Don't care, check return of function. */
  uint64_t configItem = 0; /* Dont care, just needs to be big enough to fit conf value. */
  char_t   anyString[MAX_TOKEN_SIZE];
  char_t   ip_addr[I2V_DEST_IP_MAX + 1];

  if ((NULL == param) || (NULL == value)) {
      ret = CFG_PARAM_FAIL;
  } else {
      if((0 == strlen(value)) || (I2V_CFG_MAX_STR_LEN < strlen(value))){
          ret = CFG_ITEM_LENGTH_BAD; /* Empty or larger than what I2V confs can handle in I2V_UTIL. */
      }
  }
  if(CFG_AOK == ret) {
      if(0 == strcmp("RadioNum",param)) {
          i2vRet = i2vUtilUpdateUint8Value(&configItem,value,I2V_RADIO_MIN_S,I2V_RADIO_MAX_S,NULL,&status,NULL);
      } else if(0 == strcmp("ChannelNumber",param)) {
          i2vRet = i2vUtilUpdateUint8Value(&configItem,value,I2V_RADIO_CHANNEL_MIN_S,I2V_RADIO_CHANNEL_MAX_S,NULL,&status,NULL);
      } else if(0 == strcmp("AMHBroadcastDir",param)) {
          i2vRet = i2vUtilUpdateStrValue(&anyString,value,AMH_ACTIVE_DIR_MIN_S,AMH_ACTIVE_DIR_MAX_S,NULL,&status,NULL);
      } else if(0 == strcmp("EnableValidation",param)) { 
          i2vRet = i2vUtilUpdateWBOOLValue(&configItem,value,I2V_ENABLE_MIN_S,I2V_ENABLE_MAX_S,NULL,&status,NULL);
      } else if(0 == strcmp("EnableStoreAndRepeat",param)) { 
          i2vRet = i2vUtilUpdateWBOOLValue(&configItem,value,I2V_ENABLE_MIN_S,I2V_ENABLE_MAX_S,NULL,&status,NULL);
      } else if(0 == strcmp("MaxStoreMessages",param)) {
          i2vRet = i2vUtilUpdateUint8Value(&configItem,value,AMH_MAX_STORE_MESSAGES_MIN_S,AMH_MAX_STORE_MESSAGES_MAX_S,NULL,&status,NULL);
      } else if(0 == strcmp("SettingsOverride",param)) { 
          i2vRet = i2vUtilUpdateWBOOLValue(&configItem,value,I2V_ENABLE_MIN_S,I2V_ENABLE_MAX_S,NULL,&status,NULL);
      } else if(0 == strcmp("ForcePSIDMatch",param)) { 
          i2vRet = i2vUtilUpdateWBOOLValue(&configItem,value,I2V_ENABLE_MIN_S,I2V_ENABLE_MAX_S,NULL,&status,NULL);
      } else if(0 == strcmp("SendOverride",param)) { 
          i2vRet = i2vUtilUpdateWBOOLValue(&configItem,value,I2V_ENABLE_MIN_S,I2V_ENABLE_MAX_S,NULL,&status,NULL);
      } else if(0 == strcmp("WSMExtension",param)) { 
          i2vRet = i2vUtilUpdateWBOOLValue(&configItem,value,I2V_ENABLE_MIN_S,I2V_ENABLE_MAX_S,NULL,&status,NULL);
      } else if(0 == strcmp("ImmediateEnable",param)) { 
          i2vRet = i2vUtilUpdateWBOOLValue(&configItem,value,I2V_ENABLE_MIN_S,I2V_ENABLE_MAX_S,NULL,&status,NULL);
      } else if(0 == strcmp("ImmediateIPFilter",param)) { 
          i2vRet = i2vUtilUpdateWBOOLValue(&configItem,value,I2V_ENABLE_MIN_S,I2V_ENABLE_MAX_S,NULL,&status,NULL);
      } else if(0 == strcmp("ImmediateIP",param)) { /* IPv4. */
          if(I2V_RETURN_OK == (i2vRet = i2vUtilUpdateStrValue(&ip_addr,value,I2V_DEST_IP_MIN_S,I2V_DEST_IP_MAX_S,NULL,&status,NULL))){
            ret = i2vCheckIPAddr(value,I2V_IP_ADDR_V4);
          }
      } else if(0 == strcmp("ImmediatePort",param)) {
          i2vRet = i2vUtilUpdateUint16Value(&configItem,value,I2V_DEST_PORT_MIN_S,I2V_DEST_PORT_MAX_S,NULL,&status,NULL);
      } else if(0 == strcmp("AMHForwardEnable",param)) {
          i2vRet = i2vUtilUpdateUint32Value(&configItem,value,I2V_AMH_FORWARD_ENABLE_MIN_S,I2V_AMH_FORWARD_ENABLE_MAX_S ,NULL,&status,NULL);
      } else {
          ret = CFG_VALUE_UNKOWN_PARAM;
      }
      if ((CFG_AOK == ret) && (I2V_RETURN_OK != i2vRet)){
          ret = CFG_SET_VALUE_RANGE_ERROR;
      }
  }
  return ret;
}
STATIC int32_t cfgCheckIPB(char_t * param, char_t * value) 
{
  int32_t ret = CFG_AOK; /* SUCCESS till proven FAIL. No match is a don't care, handled elsewhere. */
  uint32_t i2vRet = I2V_RETURN_OK;
  uint32_t status = I2V_RETURN_OK; /* Don't care, check return of function. */
  uint64_t configItem = 0; /* Dont care, just needs to be big enough to fit conf value. */
  char_t   anyString[MAX_TOKEN_SIZE];
  char_t   ip_addr[I2V_DEST_IP_MAX + 1];

  if ((NULL == param) || (NULL == value)) {
      ret = CFG_PARAM_FAIL;
  }
#if 0 /* Some values below are allowed to be empty. Handle on per item basis below. */
  else {
      if((0 == strlen(value)) || (I2V_CFG_MAX_STR_LEN < strlen(value))){
          ret = CFG_ITEM_LENGTH_BAD; /* Empty or larger than what I2V confs can handle in I2V_UTIL. */
      }
  }
#endif
  if(CFG_AOK == ret) {
      if(0 == strcmp("RadioNum",param)) {
          i2vRet = i2vUtilUpdateUint8Value(&configItem,value,I2V_RADIO_MIN_S,I2V_RADIO_MAX_S,NULL,&status,NULL);
      } else if(0 == strcmp("WSAPriority",param)) { 
          i2vRet = i2vUtilUpdateUint8Value(&configItem,value,I2V_WSA_PRIORITY_MIN_S,I2V_WSA_PRIORITY_MAX_S,NULL,&status,NULL);
      } else if(0 == strcmp("WSATxRate",param)) { 
          i2vRet = i2vUtilUpdateUint8Value(&configItem,value,I2V_WSA_TX_RATE_MIN_S,I2V_WSA_TX_RATE_MAX_S,NULL,&status,NULL);
      } else if(0 == strcmp("DataRate",param)) {
          if(I2V_RETURN_OK == (i2vRet = i2vUtilUpdateUint8Value(&configItem,value,I2V_WSA_DATA_RATE_MIN_S,I2V_WSA_DATA_RATE_MAX_S,NULL,&status,NULL))){
              switch((uint8_t)strtol((const char_t *) value,NULL,0)) {
                  case 3:
                  case 6: 
                  case 9: 
                  case 12: 
                  case 18: 
                  case 24:
                  case 36:
                  case 48: 
                  case 54:
                      break;
                  default:
                      ret = CFG_SET_VALUE_RANGE_ERROR;
                      break;
              }
          }
      } else if(0 == strcmp("TransmitPower",param)) {
          ret = i2vCheckTxPower(value);
      } else if(0 == strcmp("WSAPower",param)) {
          ret = i2vCheckTxPower(value);
      } else if(0 == strcmp("OverridePower",param)) {
          i2vRet = i2vUtilUpdateWBOOLValue(&configItem,value,I2V_ENABLE_MIN_S,I2V_ENABLE_MAX_S,NULL,&status,NULL);
      } else if(0 == strcmp("EnableContinuous",param)) { 
          i2vRet = i2vUtilUpdateWBOOLValue(&configItem,value,I2V_ENABLE_MIN_S,I2V_ENABLE_MAX_S,NULL,&status,NULL);
      } else if(0 == strcmp("OverrideCCH",param)) { 
          i2vRet = i2vUtilUpdateWBOOLValue(&configItem,value,I2V_ENABLE_MIN_S,I2V_ENABLE_MAX_S,NULL,&status,NULL);
      } else if(0 == strcmp("CCHChannel",param)) {
          i2vRet = i2vUtilUpdateUint8Value(&configItem,value,I2V_RADIO_CHANNEL_MIN_S,I2V_RADIO_CHANNEL_MAX_S,NULL,&status,NULL);
      } else if(0 == strcmp("ChannelNumber",param)) {
          i2vRet = i2vUtilUpdateUint8Value(&configItem,value,I2V_RADIO_CHANNEL_MIN_S,I2V_RADIO_CHANNEL_MAX_S,NULL,&status,NULL);
      } else if(0 == strcmp("SecurityDebugEnable",param)) { 
          i2vRet = i2vUtilUpdateWBOOLValue(&configItem,value,I2V_ENABLE_MIN_S,I2V_ENABLE_MAX_S,NULL,&status,NULL);
      } else if(0 == strcmp("IPBEnableEDCA",param)) { 
         i2vRet = i2vUtilUpdateWBOOLValue(&configItem,value,I2V_ENABLE_MIN_S,I2V_ENABLE_MAX_S,NULL,&status,NULL);
      } else if(0 == strcmp("IPBGPSOverride",param)) { 
          i2vRet = i2vUtilUpdateWBOOLValue(&configItem,value,I2V_ENABLE_MIN_S,I2V_ENABLE_MAX_S,NULL,&status,NULL);
      } else if(0 == strcmp("IPBIPv6UseLinkLocal",param)) { 
          i2vRet = i2vUtilUpdateWBOOLValue(&configItem,value,I2V_ENABLE_MIN_S,I2V_ENABLE_MAX_S,NULL,&status,NULL);
      } else if(0 == strcmp("IPBEnableBridge",param)) { 
          i2vRet = i2vUtilUpdateWBOOLValue(&configItem,value,I2V_ENABLE_MIN_S,I2V_ENABLE_MAX_S,NULL,&status,NULL);
      } else if(0 == strcmp("IPBBridgeOnce",param)) { 
          i2vRet = i2vUtilUpdateWBOOLValue(&configItem,value,I2V_ENABLE_MIN_S,I2V_ENABLE_MAX_S,NULL,&status,NULL);
      } else if(0 == strcmp("IPBIPv4Bridge",param)) { 
          i2vRet = i2vUtilUpdateWBOOLValue(&configItem,value,I2V_ENABLE_MIN_S,I2V_ENABLE_MAX_S,NULL,&status,NULL);
      } else if(0 == strcmp("BridgeDelayClients",param)) { 
          i2vRet = i2vUtilUpdateWBOOLValue(&configItem,value,I2V_ENABLE_MIN_S,I2V_ENABLE_MAX_S,NULL,&status,NULL);
      } else if(0 == strcmp("IPBPSID",param)) { 
          ret = i2vCheckPSID(value);
      } else if(0 == strcmp("IPBAdvertiseID",param)) {
          i2vRet = i2vUtilUpdateStrValue(&anyString,value,WSA_ADVERTISE_ID_MIN_S,WSA_ADVERTISE_ID_MAX_S,NULL,&status,NULL);
      } else if(0 == strcmp("IPBMinSatellites",param)) { 
          i2vRet = i2vUtilUpdateUint8Value(&configItem,value,IPB_SV_COUNT_MIN_S,IPB_SV_COUNT_MAX_S,NULL,&status,NULL);
      } else if(0 == strcmp("IPBSatelliteTO",param)) {
          i2vRet = i2vUtilUpdateUint8Value(&configItem,value,IPB_GNSS_TO_MIN_S,IPB_GNSS_TO_MAX_S,NULL,&status,NULL);
      } else if(0 == strcmp("IPBGPSSettleTime",param)) {
          i2vRet = i2vUtilUpdateUint16Value(&configItem,value,IPB_GNSS_SETTLE_TIME_MIN_S,IPB_GNSS_SETTLE_TIME_MAX_S,NULL,&status,NULL);
      } else if(0 == strcmp("Latitude",param)) {
          i2vRet = i2vUtilUpdateDoubleValue(&configItem,value,I2V_LAT_DEG_MIN_S,I2V_LAT_DEG_MAX_S,NULL,&status,NULL);
      } else if(0 == strcmp("Longitude",param)) {
          i2vRet = i2vUtilUpdateDoubleValue(&configItem,value,I2V_LON_DEG_MIN_S,I2V_LON_DEG_MAX_S,NULL,&status,NULL);
      } else if(0 == strcmp("Elevation",param)) {
          i2vRet = i2vUtilUpdateInt16Value(&configItem,value,I2V_ELV_M_MIN_S,I2V_ELV_M_MAX_S,NULL,&status,NULL);
      } else if(0 == strcmp("PosConfidence",param)) {
          i2vRet = i2vUtilUpdateUint8Value(&configItem,value,IPB_POS_CONFIDENCE_MIN_S,IPB_POS_CONFIDENCE_MAX_S,NULL,&status,NULL);
      } else if(0 == strcmp("IPBIPv6Prefix",param)) {
          if(I2V_RETURN_OK == (i2vRet = i2vUtilUpdateStrValue(&ip_addr,value,I2V_DEST_IP_MIN_S,I2V_DEST_IP_MAX_S,NULL,&status,NULL))){
            ret = i2vCheckIPAddr(value,I2V_IP_ADDR_V6);
          }
      } else if(0 == strcmp("IPBIPv6Identifier",param)) {
          if(I2V_RETURN_OK == (i2vRet = i2vUtilUpdateStrValue(&ip_addr,value,I2V_DEST_IP_MIN_S,I2V_DEST_IP_MAX_S,NULL,&status,NULL))){
            ret = i2vCheckIPAddr(value,I2V_IP_ADDR_V6);
          }
      } else if(0 == strcmp("IPv6ServerPrefix",param)) {
          if(I2V_RETURN_OK == (i2vRet = i2vUtilUpdateStrValue(&ip_addr,value,I2V_DEST_IP_MIN_S,I2V_DEST_IP_MAX_S,NULL,&status,NULL))){
            ret = i2vCheckIPAddr(value,I2V_IP_ADDR_V6);
          }
      } else if(0 == strcmp("IPv6ServerID",param)) {
          if(I2V_RETURN_OK == (i2vRet = i2vUtilUpdateStrValue(&ip_addr,value,I2V_DEST_IP_MIN_S,I2V_DEST_IP_MAX_S,NULL,&status,NULL))){
            ret = i2vCheckIPAddr(value,I2V_IP_ADDR_V6);
          }
      } else if(0 == strcmp("IPv4ServerIP",param)) {
          if(I2V_RETURN_OK == (i2vRet = i2vUtilUpdateStrValue(&ip_addr,value,I2V_DEST_IP_MIN_S,I2V_DEST_IP_MAX_S,NULL,&status,NULL))){
            ret = i2vCheckIPAddr(value,I2V_IP_ADDR_V4);
          }
      } else if(0 == strcmp("IPBIPv6Port",param)) {
          i2vRet = i2vUtilUpdateUint16Value(&configItem,value,I2V_DEST_PORT_MIN_S,I2V_DEST_PORT_MAX_S,NULL,&status,NULL);
      } else if(0 == strcmp("IPv4ServerPort",param)) {
          i2vRet = i2vUtilUpdateUint16Value(&configItem,value,I2V_DEST_PORT_MIN_S,I2V_DEST_PORT_MAX_S,NULL,&status,NULL);
      } else if(0 == strcmp("IPBProviderCtxt",param)) {
          if(I2V_RETURN_OK == (i2vRet = i2vUtilUpdateStrValue(&anyString,value,IPB_PROVIDER_CTXT_MIN_S,IPB_PROVIDER_CTXT_MAX_S,NULL,&status,NULL))){
          }
      } else if(0 == strcmp("BridgeMaxClients",param)) {
          i2vRet = i2vUtilUpdateUint8Value(&configItem,value,IPB_BRIDGE_CLIENT_MIN_S,IPB_BRIDGE_CLIENT_MAX_S,NULL,&status,NULL);
      } else if(0 == strcmp("ConnectionTimeout",param)) {
          i2vRet = i2vUtilUpdateUint8Value(&configItem,value,IPB_CONNECT_TO_MIN_S,IPB_CONNECT_TO_MAX_S,NULL,&status,NULL);
      } else if(0 == strcmp("IPBServerURL",param)) {
         if(0 == strlen(value)) {
             ret = CFG_AOK;
         } else {
             i2vRet = i2vUtilUpdateStrValue(&anyString,value,IPB_SERVER_URL_MIN_S,IPB_SERVER_URL_MAX_S,NULL,&status,NULL);
         }
      } else if(0 == strcmp("IPBServiceMac",param)) {
             if(I2V_RETURN_OK == (i2vRet = i2vUtilUpdateStrValue(&anyString,value,IPB_SRV_MAC_MIN_S,IPB_SRV_MAC_MAX_S,NULL,&status,NULL))){
                 ret = i2vCheckMac(value);
             }
      } else {
          ret = CFG_VALUE_UNKOWN_PARAM;
      }
      if ((CFG_AOK == ret) && (I2V_RETURN_OK != i2vRet)){
          ret = CFG_SET_VALUE_RANGE_ERROR;
      }
  }
  return ret;
}

/* 
 * These are irregular items handled at conf_manager.c level.
 * They have not been properly integerated into cfgmgrapi.
 * Therefore, this isn't being called today. Maybe in future.
 */
STATIC int32_t cfgCheckRSUINFO(char_t * param, char_t * value) 
{
  int32_t ret = CFG_AOK; /* SUCCESS till proven FAIL. No match is a don't care, handled elsewhere. */
  uint32_t i2vRet = I2V_RETURN_OK;
  uint32_t status = I2V_RETURN_OK; /* Don't care, check return of function. */
  uint64_t configItem = 0; /* Dont care, just needs to be big enough to fit conf value. */
  char_t   ip_addr[I2V_DEST_IP_MAX + 1];

  if ((NULL == param) || (NULL == value)) {
      ret = CFG_PARAM_FAIL;
  } else {
      if((0 == strlen(value)) || (I2V_CFG_MAX_STR_LEN < strlen(value))){
          ret = CFG_ITEM_LENGTH_BAD; /* Empty or larger than what I2V confs can handle in I2V_UTIL. */
      }
  }
  if(CFG_AOK == ret) {
      if(0 == strcmp("SC0IPADDRESS",param)) { 
          if(I2V_RETURN_OK == (i2vRet = i2vUtilUpdateStrValue(&ip_addr,value,I2V_DEST_IP_MIN_S,I2V_DEST_IP_MAX_S,NULL,&status,NULL))){
            ret = i2vCheckIPAddr(value,I2V_IP_ADDR_V4);
          }
      } else if(0 == strcmp("NETWORK_MODE",param)) { 
          i2vRet = i2vUtilUpdateUint8Value(&configItem,value,I2V_RSU_NETWORK_MODE_MIN_S,I2V_RSU_NETWORK_MODE_MAX_S,NULL,&status,NULL);
      } else if(0 == strcmp("NETMASK",param)) {
         if(0 == strlen(value)) {
             ret = CFG_AOK;
         } else {
             i2vRet = i2vUtilUpdateUint8Value(&configItem,value,I2V_RSU_NETMASK_MIN_S,I2V_RSU_NETMASK_MAX_S,NULL,&status,NULL);
         }
      } else if(0 == strcmp("I2V_APP_ENABLE",param)) {
          i2vRet = i2vUtilUpdateWBOOLValue(&configItem,value,I2V_ENABLE_MIN_S,I2V_ENABLE_MAX_S,NULL,&status,NULL);
      } else if(0 == strcmp("RSU_HEALTH_ENABLE",param)) {
          i2vRet = i2vUtilUpdateWBOOLValue(&configItem,value,I2V_ENABLE_MIN_S,I2V_ENABLE_MAX_S,NULL,&status,NULL);
      } else if(0 == strcmp("VLAN_IP",param)) { 
          if(I2V_RETURN_OK == (i2vRet = i2vUtilUpdateStrValue(&ip_addr,value,I2V_DEST_IP_MIN_S,I2V_DEST_IP_MAX_S,NULL,&status,NULL))){
            ret = i2vCheckIPAddr(value,I2V_IP_ADDR_V4);
          }
#if 0
      } else if(0 == strcmp("RSU_SYSTEM_MONITOR_ENABLE",param)) {
          i2vRet = i2vUtilUpdateWBOOLValue(&configItem,value,I2V_ENABLE_MIN_S,I2V_ENABLE_MAX_S,NULL,&status,NULL);
#endif
      } else if(0 == strcmp("WEBGUI_HTTP_SERVICE_PORT",param)) {
          i2vRet = i2vUtilUpdateUint16Value(&configItem,value,I2V_DEST_PORT_MIN_S,I2V_DEST_PORT_MAX_S,NULL,&status,NULL);
      } else if(0 == strcmp("WEBGUI_HTTPS_SERVICE_PORT",param)) {
          i2vRet = i2vUtilUpdateUint16Value(&configItem,value,I2V_DEST_PORT_MIN_S,I2V_DEST_PORT_MAX_S,NULL,&status,NULL);
      } else {
          ret = CFG_VALUE_UNKOWN_PARAM;
      }
      if ((CFG_AOK == ret) && (I2V_RETURN_OK != i2vRet)){
          ret = CFG_SET_VALUE_RANGE_ERROR;
      }
  }
  return ret;
}
STATIC int32_t cfgCheckSPAT(char_t * param, char_t * value) 
{
  int32_t  ret = CFG_AOK; /* SUCCESS till proven FAIL. No match is a don't care, handled elsewhere. */
  uint32_t i2vRet = I2V_RETURN_OK;
  uint32_t status = I2V_RETURN_OK; /* Don't care, check return of function. */
  uint64_t configItem = 0; /* Dont care, just needs to be big enough to fit conf value. */

  if ((NULL == param) || (NULL == value)) {
      ret = CFG_PARAM_FAIL;
  } else {
      if((0 == strlen(value)) || (I2V_CFG_MAX_STR_LEN < strlen(value))){
          ret = CFG_ITEM_LENGTH_BAD; /* Empty or larger than what I2V confs can handle in I2V_UTIL. */
      }
  }
  if(CFG_AOK == ret) {
      if(0 == strcmp("RadioCfg",param)) {
          i2vRet = i2vUtilUpdateUint8Value(&configItem,value,I2V_RADIO_MIN_S,I2V_RADIO_MAX_S,NULL,&status,NULL);
      } else if(0 == strcmp("WSAEnable",param)) { 
          i2vRet = i2vUtilUpdateWBOOLValue(&configItem,value,I2V_ENABLE_MIN_S,I2V_ENABLE_MAX_S,NULL,&status,NULL);
      } else if(0 == strcmp("WSAPriority",param)) { 
          i2vRet = i2vUtilUpdateUint8Value(&configItem,value,I2V_WSA_PRIORITY_MIN_S,I2V_WSA_PRIORITY_MAX_S,NULL,&status,NULL);
      } else if(0 == strcmp("WSMExtension",param)) {
          i2vRet = i2vUtilUpdateWBOOLValue(&configItem,value,I2V_ENABLE_MIN_S,I2V_ENABLE_MAX_S,NULL,&status,NULL);
      } else if(0 == strcmp("SPATTOMPSID",param)) {
          ret = i2vCheckPSID(value);
      } else if(0 == strcmp("SPATChannelNumber",param)) {
          i2vRet = i2vUtilUpdateUint8Value(&configItem,value,I2V_RADIO_CHANNEL_MIN_S,I2V_RADIO_CHANNEL_MAX_S,NULL,&status,NULL);
      } else if(0 == strcmp("BSMRxEnable",param)) {
          i2vRet = i2vUtilUpdateWBOOLValue(&configItem,value,I2V_ENABLE_MIN_S,I2V_ENABLE_MAX_S,NULL,&status,NULL);
      }  else if(0 == strcmp("BSMRxForward",param)) { 
          i2vRet = i2vUtilUpdateUint8Value(&configItem,value,SPAT_BSM_RX_FWD_MIN_S,SPAT_BSM_RX_FWD_MAX_S,NULL,&status,NULL);
      } else if(0 == strcmp("BSMUnsecurePSIDPERU",param)) { 
          ret = i2vCheckPSID(value);
      } else if(0 == strcmp("SPATInteroperabilityMode",param)) { 
          i2vRet = i2vUtilUpdateUint8Value(&configItem,value,SPAT_INTEROP_MODE_MIN_S,SPAT_INTEROP_MODE_MAX_S,NULL,&status,NULL);
      } else if(0 == strcmp("IntersectionID",param)) {
          i2vRet = i2vUtilUpdateUint32Value(&configItem,value,SPAT_INT_ID_MIN_S,SPAT_INT_ID_MAX_S,NULL,&status,NULL);
      } else {
          ret = CFG_VALUE_UNKOWN_PARAM;
      }
      if ((CFG_AOK == ret) && (I2V_RETURN_OK != i2vRet)){
          ret = CFG_SET_VALUE_RANGE_ERROR;
      }
  }
  return ret;
}
STATIC int32_t cfgCheckSCS(char_t * param, char_t * value) 
{
  int32_t  ret = CFG_AOK; /* SUCCESS till proven FAIL. No match is a don't care, handled elsewhere. */
  uint32_t i2vRet = I2V_RETURN_OK;
  uint32_t status = I2V_RETURN_OK; /* Don't care, check return of function. */
  uint64_t configItem = 0; /* Dont care, just needs to be big enough to fit conf value. */
  char_t   ip_addr[I2V_DEST_IP_MAX + 1];
  char_t   anyString[MAX_TOKEN_SIZE];

  if ((NULL == param) || (NULL == value)) {
      ret = CFG_PARAM_FAIL;
  } else {
      if((0 == strlen(value)) || (I2V_CFG_MAX_STR_LEN < strlen(value))){
          ret = CFG_ITEM_LENGTH_BAD; /* Empty or larger than what I2V confs can handle in I2V_UTIL. */
      }
  }
  if(CFG_AOK == ret) {
      if(0 == strcmp("InterfaceTOM",param)) { 
          i2vRet = i2vUtilUpdateWBOOLValue(&configItem,value,I2V_ENABLE_MIN_S,I2V_ENABLE_MAX_S,NULL,&status,NULL);
      } else if(0 == strcmp("UseMaxTimeToChange",param)) { 
          i2vRet = i2vUtilUpdateWBOOLValue(&configItem,value,I2V_ENABLE_MIN_S,I2V_ENABLE_MAX_S,NULL,&status,NULL);
      } else if(0 == strcmp("RSUHeartBeatEnable",param)) { 
          i2vRet = i2vUtilUpdateWBOOLValue(&configItem,value,I2V_ENABLE_MIN_S,I2V_ENABLE_MAX_S,NULL,&status,NULL);
      } else if(0 == strcmp("RSUHeartBeatInt",param)) {
          i2vRet = i2vUtilUpdateUint16Value(&configItem,value,SRMRX_HEARTBEAT_INTERVAL_MIN_S,SRMRX_HEARTBEAT_INTERVAL_MAX_S,NULL,&status,NULL);
      } else if(0 == strcmp("SRMFwdInterval",param)) {
          i2vRet = i2vUtilUpdateUint16Value(&configItem,value,SRMRX_FWD_INTERVAL_MIN_S,SRMRX_FWD_INTERVAL_MAX_S,NULL,&status,NULL);
      } else if(0 == strcmp("SRMFwdEnable",param)) { 
          i2vRet = i2vUtilUpdateWBOOLValue(&configItem,value,I2V_ENABLE_MIN_S,I2V_ENABLE_MAX_S,NULL,&status,NULL);
      } else if(0 == strcmp("SRMFwdIP",param)) {
          if(I2V_RETURN_OK == (i2vRet = i2vUtilUpdateStrValue(&ip_addr,value,I2V_DEST_IP_MIN_S,I2V_DEST_IP_MAX_S,NULL,&status,NULL))){
            ret = i2vCheckIPAddr(value,I2V_IP_ADDR_V4);
          }
      } else if(0 == strcmp("SRMFwdPort",param)) {
          i2vRet = i2vUtilUpdateUint16Value(&configItem,value,I2V_DEST_PORT_MIN_S,I2V_DEST_PORT_MAX_S,NULL,&status,NULL);
      } else if(0 == strcmp("SRMFwdInterface",param)) {
          i2vRet = i2vUtilUpdateUint8Value(&configItem,value,SRMRX_FWD_INTERFACE_MIN_S,SRMRX_FWD_INTERFACE_MAX_S,NULL,&status,NULL);
      } else if(0 == strcmp("SRMFwdRequireTTI",param)) {
          i2vRet = i2vUtilUpdateWBOOLValue(&configItem,value,I2V_ENABLE_MIN_S,I2V_ENABLE_MAX_S,NULL,&status,NULL);
      } else if(0 == strcmp("LocalSignalControllerIP",param)) {
          if(I2V_RETURN_OK == (i2vRet = i2vUtilUpdateStrValue(&ip_addr,value,I2V_DEST_IP_MIN_S,I2V_DEST_IP_MAX_S,NULL,&status,NULL))){
            ret = i2vCheckIPAddr(value,I2V_IP_ADDR_V4);
          }
      } else if(0 == strcmp("LocalSignalSNMPPort",param)) {
          i2vRet = i2vUtilUpdateUint16Value(&configItem,value,I2V_DEST_PORT_MIN_S,I2V_DEST_PORT_MAX_S,NULL,&status,NULL);
      } else if(0 == strcmp("LocalSignalControllerPort",param)) {
          i2vRet = i2vUtilUpdateUint16Value(&configItem,value,I2V_DEST_PORT_MIN_S,I2V_DEST_PORT_MAX_S,NULL,&status,NULL);
      } else if(0 == strcmp("LocalSignalIntersectionID",param)) { 
          i2vRet = i2vUtilUpdateUint32Value(&configItem,value,SCS_LOCAL_SIGNAL_INT_ID_MIN_S,SCS_LOCAL_SIGNAL_INT_ID_MAX_S,NULL,&status,NULL);
      } else if(0 == strcmp("LocalSignalControllerEnable",param)) { 
          i2vRet = i2vUtilUpdateWBOOLValue(&configItem,value,I2V_ENABLE_MIN_S,I2V_ENABLE_MAX_S,NULL,&status,NULL);
      } else if(0 == strcmp("LocalSignalControllerIsSnmp",param)) { 
          i2vRet = i2vUtilUpdateWBOOLValue(&configItem,value,I2V_ENABLE_MIN_S,I2V_ENABLE_MAX_S,NULL,&status,NULL);
      } else if(0 == strcmp("LocalSignalControllerHW",param)) { 
          i2vRet = i2vUtilUpdateUint8Value(&configItem,value,SCS_LOCAL_SIGNAL_HW_MIN_S,SCS_LOCAL_SIGNAL_HW_MAX_S,NULL,&status,NULL);
      } else if(0 == strcmp("SNMPEnableSpatCommand",param)) {
          i2vRet = i2vUtilUpdateStrValue(&anyString,value,"0", "0",NULL,&status,NULL);
      } else if(0 == strcmp("SPATEnableValue",param)) {
          i2vRet = i2vUtilUpdateStrValue(&anyString,value,"0", "0",NULL,&status,NULL);
      } else if(0 == strcmp("BypassSignalControllerNTCIP",param)) { 
          i2vRet = i2vUtilUpdateWBOOLValue(&configItem,value,I2V_ENABLE_MIN_S,I2V_ENABLE_MAX_S,NULL,&status,NULL);
      } else if(0 == strcmp("BypassYellowDuration",param)) {
          i2vRet = i2vUtilUpdateUint8Value(&configItem,value,SCS_BYPASS_YELLOW_DURATION_MIN_S,SCS_BYPASS_YELLOW_DURATION_MAX_S,NULL,&status,NULL);
      } else if(0 == strcmp("NTCIP_YELLOW_DURATION",param)) {
          i2vRet = i2vUtilUpdateStrValue(&anyString,value,"0", "0",NULL,&status,NULL);
      } else {
          ret = CFG_VALUE_UNKOWN_PARAM;
      }
      if ((CFG_AOK == ret) && (I2V_RETURN_OK != i2vRet)){
          ret = CFG_SET_VALUE_RANGE_ERROR;
      }
  }
  return ret;
}
STATIC int32_t cfgCheckSRMRX(char_t * param, char_t * value) 
{
  int32_t  ret = CFG_AOK; /* SUCCESS till proven FAIL. No match is a don't care, handled elsewhere. */
  uint32_t i2vRet = I2V_RETURN_OK;
  uint32_t status = I2V_RETURN_OK; /* Don't care, check return of function. */
  uint64_t configItem = 0; /* Dont care, just needs to be big enough to fit conf value. */

  if ((NULL == param) || (NULL == value)) {
      ret = CFG_PARAM_FAIL;
  } else {
      if((0 == strlen(value)) || (I2V_CFG_MAX_STR_LEN < strlen(value))){
          ret = CFG_ITEM_LENGTH_BAD; /* Empty or larger than what I2V confs can handle in I2V_UTIL. */
      }
  }
  if(CFG_AOK == ret) {
      if(0 == strcmp("RadioCfg",param)) {
          i2vRet = i2vUtilUpdateUint8Value(&configItem,value,I2V_RADIO_MIN_S,I2V_RADIO_MAX_S,NULL,&status,NULL);
      } else if(0 == strcmp("SecurityDebugEnable",param)) { 
          i2vRet = i2vUtilUpdateWBOOLValue(&configItem,value,I2V_ENABLE_MIN_S,I2V_ENABLE_MAX_S,NULL,&status,NULL);
      } else if(0 == strcmp("SRMDecodeMethod",param)) {
          i2vRet = i2vUtilUpdateUint8Value(&configItem,value,SRMRX_DECODE_METHOD_MIN_S,SRMRX_DECODE_METHOD_MAX_S,NULL,&status,NULL);
      } else if(0 == strcmp("SRMASN1DecodeMethod",param)) {
          i2vRet = i2vUtilUpdateUint8Value(&configItem,value,SRMRX_ASN1_DECODE_METHOD_MIN_S,SRMRX_ASN1_DECODE_METHOD_MAX_S,NULL,&status,NULL);
      } else if(0 == strcmp("SRMChannelNumber",param)) {
          i2vRet = i2vUtilUpdateUint8Value(&configItem,value,I2V_RADIO_CHANNEL_MIN_S,I2V_RADIO_CHANNEL_MAX_S,NULL,&status,NULL);
      } else if(0 == strcmp("SRMRxForward",param)) { 
          i2vRet = i2vUtilUpdateWBOOLValue(&configItem,value,I2V_ENABLE_MIN_S,I2V_ENABLE_MAX_S,NULL,&status,NULL);
      }  else if(0 == strcmp("SRMUnsecurePSIDDER",param)) { 
          ret = i2vCheckPSID(value);
      }  else if(0 == strcmp("SRMUnsecurePSIDPERA",param)) { 
          ret = i2vCheckPSID(value);
      }  else if(0 == strcmp("SRMUnsecurePSIDPERU",param)) { 
          ret = i2vCheckPSID(value);
      } else if(0 == strcmp("SRMPermissive",param)) { 
          i2vRet = i2vUtilUpdateWBOOLValue(&configItem,value,I2V_ENABLE_MIN_S,I2V_ENABLE_MAX_S,NULL,&status,NULL);
      } else if(0 == strcmp("SRMTXVehBasicRole",param)) {
          i2vRet = i2vUtilUpdateUint32Value(&configItem,value,SRMTX_VEHICLE_ROLE_MIN_S,SRMTX_VEHICLE_ROLE_MAX_S,NULL,&status,NULL);
      } else if(0 == strcmp("SRMVODMsgVerifyCount",param)) {
          i2vRet = i2vUtilUpdateUint16Value(&configItem,value,SRM_VOD_MSG_VERIFY_COUNT_MIN_S,SRM_VOD_MSG_VERIFY_COUNT_MAX_S,NULL,&status,NULL);
      } else {
          ret = CFG_VALUE_UNKOWN_PARAM;
      }
      if ((CFG_AOK == ret) && (I2V_RETURN_OK != i2vRet)){
          ret = CFG_SET_VALUE_RANGE_ERROR;
      }
  }
  return ret;
}
STATIC int32_t cfgCheckFWDMSG(char_t * param, char_t * value) 
{
    int32_t  ret = CFG_AOK; /* SUCCESS till proven FAIL. No match is a don't care, handled elsewhere. */
    uint32_t i2vRet = I2V_RETURN_OK;
    uint32_t status = I2V_RETURN_OK; /* Don't care, check return of function. */
    uint64_t configItem = 0; /* Dont care, just needs to be big enough to fit conf value. */
    char_t   ip_addr[I2V_DEST_IP_MAX + 1];

    if ((NULL == param) || (NULL == value)) {
        ret = CFG_PARAM_FAIL;
    } else {
        if((0 == strlen(value)) || (I2V_CFG_MAX_STR_LEN < strlen(value))){
          ret = CFG_ITEM_LENGTH_BAD; /* Empty or larger than what I2V confs can handle in I2V_UTIL. */
        }
    }
    if(CFG_AOK == ret) {
        if(0 == strcmp("ForwardMessageEnable1",param)) { 
            i2vRet = i2vUtilUpdateWBOOLValue(&configItem,value,I2V_ENABLE_MIN_S,I2V_ENABLE_MAX_S,NULL,&status,NULL);
        }
        else if(0 == strcmp("ForwardMessageEnable2",param)) { 
            i2vRet = i2vUtilUpdateWBOOLValue(&configItem,value,I2V_ENABLE_MIN_S,I2V_ENABLE_MAX_S,NULL,&status,NULL);
        }
        else if(0 == strcmp("ForwardMessageEnable3",param)) { 
            i2vRet = i2vUtilUpdateWBOOLValue(&configItem,value,I2V_ENABLE_MIN_S,I2V_ENABLE_MAX_S,NULL,&status,NULL);
        }
        else if(0 == strcmp("ForwardMessageEnable4",param)) { 
            i2vRet = i2vUtilUpdateWBOOLValue(&configItem,value,I2V_ENABLE_MIN_S,I2V_ENABLE_MAX_S,NULL,&status,NULL);
        }
        else if(0 == strcmp("ForwardMessageEnable5",param)) { 
            i2vRet = i2vUtilUpdateWBOOLValue(&configItem,value,I2V_ENABLE_MIN_S,I2V_ENABLE_MAX_S,NULL,&status,NULL);
        }
        else if(0 == strcmp("ForwardMessageIP1",param)) { 
          if(I2V_RETURN_OK == (i2vRet = i2vUtilUpdateStrValue(&ip_addr,value,I2V_DEST_IP_MIN_S,I2V_DEST_IP_MAX_S,NULL,&status,NULL))){
            ret = i2vCheckIPAddr(value,I2V_IP_ADDR_V4);
          }
        } 
        else if(0 == strcmp("ForwardMessageIP2",param)) { 
          if(I2V_RETURN_OK == (i2vRet = i2vUtilUpdateStrValue(&ip_addr,value,I2V_DEST_IP_MIN_S,I2V_DEST_IP_MAX_S,NULL,&status,NULL))){
            ret = i2vCheckIPAddr(value,I2V_IP_ADDR_V4);
          }
        } 
        else if(0 == strcmp("ForwardMessageIP3",param)) { 
          if(I2V_RETURN_OK == (i2vRet = i2vUtilUpdateStrValue(&ip_addr,value,I2V_DEST_IP_MIN_S,I2V_DEST_IP_MAX_S,NULL,&status,NULL))){
            ret = i2vCheckIPAddr(value,I2V_IP_ADDR_V4);
          }
        } 
        else if(0 == strcmp("ForwardMessageIP4",param)) { 
          if(I2V_RETURN_OK == (i2vRet = i2vUtilUpdateStrValue(&ip_addr,value,I2V_DEST_IP_MIN_S,I2V_DEST_IP_MAX_S,NULL,&status,NULL))){
            ret = i2vCheckIPAddr(value,I2V_IP_ADDR_V4);
          }
        } 
        else if(0 == strcmp("ForwardMessageIP5",param)) { 
          if(I2V_RETURN_OK == (i2vRet = i2vUtilUpdateStrValue(&ip_addr,value,I2V_DEST_IP_MIN_S,I2V_DEST_IP_MAX_S,NULL,&status,NULL))){
            ret = i2vCheckIPAddr(value,I2V_IP_ADDR_V4);
          }
        } 
        else if(0 == strcmp("ForwardMessagePort1",param)) { 
            i2vRet = i2vUtilUpdateUint16Value(&configItem,value,I2V_DEST_PORT_MIN_S,I2V_DEST_PORT_MAX_S,NULL,&status,NULL);
        }
        else if(0 == strcmp("ForwardMessagePort2",param)) { 
            i2vRet = i2vUtilUpdateUint16Value(&configItem,value,I2V_DEST_PORT_MIN_S,I2V_DEST_PORT_MAX_S,NULL,&status,NULL);
        }
        else if(0 == strcmp("ForwardMessagePort3",param)) { 
            i2vRet = i2vUtilUpdateUint16Value(&configItem,value,I2V_DEST_PORT_MIN_S,I2V_DEST_PORT_MAX_S,NULL,&status,NULL);
        }
        else if(0 == strcmp("ForwardMessagePort4",param)) { 
            i2vRet = i2vUtilUpdateUint16Value(&configItem,value,I2V_DEST_PORT_MIN_S,I2V_DEST_PORT_MAX_S,NULL,&status,NULL);
        }
        else if(0 == strcmp("ForwardMessagePort5",param)) { 
            i2vRet = i2vUtilUpdateUint16Value(&configItem,value,I2V_DEST_PORT_MIN_S,I2V_DEST_PORT_MAX_S,NULL,&status,NULL);
        }
        else if(0 == strcmp("ForwardMessageMask1",param)) { 
            i2vRet = i2vUtilUpdateUint8Value(&configItem,value,FWDMSG_MASK2_MIN_S,FWDMSG_MASK2_MAX_S,NULL,&status,NULL);
        }
        else if(0 == strcmp("ForwardMessageMask2",param)) { 
            i2vRet = i2vUtilUpdateUint8Value(&configItem,value,FWDMSG_MASK2_MIN_S,FWDMSG_MASK2_MAX_S,NULL,&status,NULL);
        }
        else if(0 == strcmp("ForwardMessageMask3",param)) { 
            i2vRet = i2vUtilUpdateUint8Value(&configItem,value,FWDMSG_MASK2_MIN_S,FWDMSG_MASK2_MAX_S,NULL,&status,NULL);
        }
        else if(0 == strcmp("ForwardMessageMask4",param)) { 
            i2vRet = i2vUtilUpdateUint8Value(&configItem,value,FWDMSG_MASK2_MIN_S,FWDMSG_MASK2_MAX_S,NULL,&status,NULL);
        }
        else if(0 == strcmp("ForwardMessageMask5",param)) { 
            i2vRet = i2vUtilUpdateUint8Value(&configItem,value,FWDMSG_MASK2_MIN_S,FWDMSG_MASK2_MAX_S,NULL,&status,NULL);
        } else {
            ret = CFG_VALUE_UNKOWN_PARAM;
        }
        if ((CFG_AOK == ret) && (I2V_RETURN_OK != i2vRet)){
            ret = CFG_SET_VALUE_RANGE_ERROR;
        }
    }
    return ret;
}
//TODO: Not really sure how radio uses these so further error checking on hold.
STATIC int32_t cfgCheckNSCONFIG(char_t * param, char_t * value) 
{
  int32_t  ret = CFG_AOK; /* SUCCESS till proven FAIL. No match is a don't care, handled elsewhere. */
  uint32_t i2vRet = I2V_RETURN_OK;
  uint32_t status = I2V_RETURN_OK; /* Don't care, check return of function. */
  char_t   anyString[MAX_TOKEN_SIZE];

  if ((NULL == param) || (NULL == value)) {
      ret = CFG_PARAM_FAIL;
  } else {
      /* All confs value here have same constraint. */
      if((0 == strlen(value)) || (I2V_CFG_MAX_STR_LEN < strlen(value))){
          ret = CFG_ITEM_LENGTH_BAD; /* Empty or larger than what I2V confs can handle in I2V_UTIL. */
      }
  }
  if(CFG_AOK == ret) {
      if(0 == strcmp("AC_BE_CCH_RADIO_0",param)) {
          i2vRet = i2vUtilUpdateStrValue(&anyString,value,"0", "0",NULL,&status,NULL);
      } else if(0 == strcmp("AC_BK_CCH_RADIO_0",param)) {
          i2vRet = i2vUtilUpdateStrValue(&anyString,value,"0", "0",NULL,&status,NULL);
      } else if(0 == strcmp("AC_VO_CCH_RADIO_0",param)) {
          i2vRet = i2vUtilUpdateStrValue(&anyString,value,"0", "0",NULL,&status,NULL);
      } else if(0 == strcmp("AC_VI_CCH_RADIO_0",param)) {
          i2vRet = i2vUtilUpdateStrValue(&anyString,value,"0", "0",NULL,&status,NULL);
      } else if(0 == strcmp("AC_BE_SCH_RADIO_0",param)) {
          i2vRet = i2vUtilUpdateStrValue(&anyString,value,"0", "0",NULL,&status,NULL);
      } else if(0 == strcmp("AC_BK_SCH_RADIO_0",param)) {
          i2vRet = i2vUtilUpdateStrValue(&anyString,value,"0", "0",NULL,&status,NULL);
      } else if(0 == strcmp("AC_VO_SCH_RADIO_0",param)) {
          i2vRet = i2vUtilUpdateStrValue(&anyString,value,"0", "0",NULL,&status,NULL);
      } else if(0 == strcmp("AC_VI_SCH_RADIO_0",param)) {
          i2vRet = i2vUtilUpdateStrValue(&anyString,value,"0", "0",NULL,&status,NULL);
      } else if(0 == strcmp("AC_BE_CCH_RADIO_1",param)) {
          i2vRet = i2vUtilUpdateStrValue(&anyString,value,"0", "0",NULL,&status,NULL);
      } else if(0 == strcmp("AC_BK_CCH_RADIO_1",param)) {
          i2vRet = i2vUtilUpdateStrValue(&anyString,value,"0", "0",NULL,&status,NULL);
      } else if(0 == strcmp("AC_VO_CCH_RADIO_1",param)) {
          i2vRet = i2vUtilUpdateStrValue(&anyString,value,"0", "0",NULL,&status,NULL);
      } else if(0 == strcmp("AC_VI_CCH_RADIO_1",param)) {
          i2vRet = i2vUtilUpdateStrValue(&anyString,value,"0", "0",NULL,&status,NULL);
      } else if(0 == strcmp("AC_BE_SCH_RADIO_1",param)) {
          i2vRet = i2vUtilUpdateStrValue(&anyString,value,"0", "0",NULL,&status,NULL);
      } else if(0 == strcmp("AC_BK_SCH_RADIO_1",param)) {
          i2vRet = i2vUtilUpdateStrValue(&anyString,value,"0", "0",NULL,&status,NULL);
      } else if(0 == strcmp("AC_VO_SCH_RADIO_1",param)) {
          i2vRet = i2vUtilUpdateStrValue(&anyString,value,"0", "0",NULL,&status,NULL);
      } else if(0 == strcmp("AC_VI_SCH_RADIO_1",param)) {
          i2vRet = i2vUtilUpdateStrValue(&anyString,value,"0", "0",NULL,&status,NULL);
      } else {
          ret = CFG_VALUE_UNKOWN_PARAM;
      }
      if ((CFG_AOK == ret) && (I2V_RETURN_OK != i2vRet)){
          ret = CFG_SET_VALUE_RANGE_ERROR;
      }
  }
  return ret;
}
STATIC int32_t cfgCheckSNMPD(char_t * param, char_t * value) 
{
  int32_t ret = CFG_AOK; /* SUCCESS till proven FAIL. No match is a don't care, handled elsewhere. */
  uint32_t i2vRet = I2V_RETURN_OK;
  uint32_t status = I2V_RETURN_OK; /* Don't care, check return of function. */
  uint64_t configItem = 0; /* Dont care, just needs to be big enough to fit conf value. */

  if ((NULL == param) || (NULL == value)) {
      ret = CFG_PARAM_FAIL;
  } else {
      if((0 == strlen(value)) || (I2V_CFG_MAX_STR_LEN < strlen(value))){
          ret = CFG_ITEM_LENGTH_BAD; /* Empty or larger than what I2V confs can handle in I2V_UTIL. */
      }
  }
  if(CFG_AOK == ret) {
      if(0 == strcmp("agentaddress",param)) {
          i2vRet = i2vUtilUpdateUint16Value(&configItem,value,I2V_DEST_PORT_MIN_S,I2V_DEST_PORT_MAX_S,NULL,&status,NULL);
      } else {
          ret = CFG_VALUE_UNKOWN_PARAM;
      }
      if ((CFG_AOK == ret) && (I2V_RETURN_OK != i2vRet)){
          ret = CFG_SET_VALUE_RANGE_ERROR;
      }
  }
  return ret;
}
/*
 * cfgCheckParam()
 *
 * Required for changing values, requires unlocked config
 *
 * returns CFG_AOK on success
 *
 */
int32_t cfgCheckParam(char_t * group, char_t * param, char_t * value)
{
  int32_t ret = CFG_AOK; /* SUCCESS till proven FAIL. */

  if ((NULL == group) || (NULL == param) || (NULL == value)) {
      ret = CFG_PARAM_FAIL;
  } else {
      if(0 == strcmp(I2V_GRP,group)) {
          ret = cfgCheckI2V(param,value);
      } else if (0 == strcmp(AMH_GRP,group)) {
          ret = cfgCheckAMH(param,value);
      } else if (0 == strcmp(IPB_GRP,group)) {
          ret = cfgCheckIPB(param,value);
      } else if (0 == strcmp(RSUINFO_GRP,group)) {
          ret = cfgCheckRSUINFO(param,value); 
      } else if (0 == strcmp(SPAT_GRP,group)) {
          ret = cfgCheckSPAT(param,value);  
      } else if (0 == strcmp(SCS_GRP,group)) {
          ret = cfgCheckSCS(param,value); 
      } else if (0 == strcmp(SRMRX_GRP,group)) {
          ret = cfgCheckSRMRX(param,value);  
      } else if (0 == strcmp(FWDMSG_GRP,group)) {
          ret = cfgCheckFWDMSG(param,value);  
      } else if (0 == strcmp(SNMPD_GRP,group)) {
          ret = cfgCheckSNMPD(param,value);  
      } else if (0 == strcmp(NSCONFIG_GRP,group)) {
          ret = cfgCheckNSCONFIG(param,value); 
      } else {
          ret = CFG_AOK; /* Group is not for us, don't care. */
      }
  }
  return ret;
}

/*
 * cfgGetParam()
 *
 * Required for obtaining values, does not require unlocked config
 *
 * returns 0 on success
 *
 */
int32_t cfgGetParam(char_t *param, char_t *group, char_t *result)
{
    FILE *fp;
    char_t *ptr, pcmd[100];
    int32_t ret = CFG_AOK;
    if ((NULL == param) || (NULL == result)) {
        ret = CFG_PARAM_FAIL;
    } else {

        if (NULL != group) {
            if (!strlen(group)) {
                group = NULL;
            }
        }
        if (NULL != group) {
            snprintf(pcmd, sizeof(pcmd), "dcu -g %s -f %s.conf", param, group);
        } else {
            snprintf(pcmd, sizeof(pcmd), "dcu -g %s", param);
        }
        if (NULL != (fp = popen(pcmd, "r"))) {
            if(NULL != fgets(result, MAX_TOKEN_SIZE, fp)) {
                pclose(fp);
                if(0 == strlen(result)) {
                    ret = CFG_DCU_RESPONSE_ERROR;
                } else {
                    /* protect against fgets adding a '\n' */
                    ptr = result + strlen(result) - 1;
                    if (*ptr == '\n') *ptr = '\0';    /* NULL terminate there */

                    ret = CFG_AOK;
                }
            } else {
                ret = CFG_ITEM_NOT_FOUND;
            }
        } else {
            ret = CFG_FOPEN_FAIL;
        }
    }
//TODO: Disabled. Let junk come through for user to see and correct. Is that the right thing to do?
#if 0
    /* Range check the groups we know. */
    if((NULL != group) && (CFG_AOK == ret)){
        if(CFG_AOK != (ret = cfgCheckParam(group, param, result))) {
            //I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"cfgCheckParam failed.\n");
        }
    }
#endif
    return ret;    
}
/*
 * cfgSetParam()
 *
 * Required for changing values, requires unlocked config
 *
 * returns 0 on success
 *
 */
int32_t cfgSetParam(char_t *param, char_t *group, char_t *value)
{
    char_t cmd[100];
    int32_t ret = CFG_AOK;
    if ((NULL == param) || (NULL == value)) {
        ret = CFG_PARAM_FAIL;
    } else {
        if (NULL != group) {
            if (!strlen(group)) {
                group = NULL;
            }
        }

        /* Check range of value prior before proceeding. */
        if((NULL != group) && (CFG_AOK != (ret = cfgCheckParam(group, param, value)))){

        }
 
        if(CFG_AOK == ret) {
            if (NULL != group) {
                snprintf(cmd, sizeof(cmd), "dcu -s %s -f %s.conf -v %s", param, group, value);
            } else {
                snprintf(cmd, sizeof(cmd), "dcu -s %s -v %s", param, value);
            }
            if (system(cmd)) {
                ret = CFG_SYSTEM_CALL_FAILED;
            }
        }
    }
    return ret;    
}

/**
 * cfgGetOptionalParam()
 *
 * Required for obtaining optional values, does not require unlocked config
 * Optional params may or may not be enabled or exist (not mandatory for operation)
 *
 * returns -1 (failure or no param), 0 (param exists, but disabled), 1 (param enabled)
 *
 */
int32_t cfgGetOptionalParam(char_t *param, char_t *file, char_t *result)
{
    /* unlike regular get/set, need a more elaborate shell command because
       command exit also indicates if param enabled or not */
    FILE *fp = NULL;
    char_t *ptr = NULL, pcmd[200]; 
    char_t poutput[2 * MAX_TOKEN_SIZE + 10];    /* gross overestimate for output but need to consider long url */
    int32_t cmdresult = -1;
    int32_t ret = CFG_AOK;

    if ((NULL == param) || (NULL == result)) {
        ret = CFG_PARAM_FAIL;
    } else {

        memset(pcmd, '\0', sizeof(pcmd));
        memset(poutput, '\0', sizeof(poutput));

        if (NULL != file) {
            if (!strlen(file)) {
                file = NULL;
            }
        }

        if (NULL != file) {
            snprintf(pcmd, sizeof(pcmd), "{ f=`dcu -o %s -f %s -v g:; echo \" rslt: $? \"`; echo $f; }", param, file);
        } else {
            snprintf(pcmd, sizeof(pcmd), "{ f=`dcu -o %s -v g:; echo \" rslt: $? \"`; echo $f; }", param);
        }
        if (NULL != (fp = popen(pcmd, "r"))) {
            if(NULL != fgets(poutput, MAX_TOKEN_SIZE, fp)) {
                pclose(fp);
                /* protect against fgets adding a '\n' */
                ptr = poutput + strlen(poutput) - 1;
                if (*ptr == '\n') *ptr = '\0';    /* NULL terminate there */
                ptr = strtok(poutput, " ");
                if (NULL != ptr) {
                    if (!strcmp(ptr, "rslt:")) {
                        /* oops - param not found, no value, went straight to result; return error */
                        ret = CFG_PARAM_FAIL;
                    } else {
                        strcpy(result, ptr);    /* set the result */
                        ptr = strtok(NULL, " ");    /* should be rslt: */
                        if ((NULL != ptr) && (NULL != (ptr = strtok(NULL, " ")))) {
                            cmdresult = strtol(ptr, NULL, 10);
                            switch (cmdresult) {
                                case 0: 
                                    ret = 0;
                                    break;
                                case 1: 
                                    ret = 1;
                                    break;
                                case 255: 
                                    ret = CFG_PARAM_FAIL;  /* QNX doesn't support negative shell results; 255 = -1 */
                                    break;
                                default: 
                                    ret = CFG_PARAM_FAIL;
                                    break;
                            }
                        }
                    }
                }
            } else {
                ret = CFG_OPT_ITEM_NOT_FOUND;
            }
        } else {
            ret = CFG_FOPEN_FAIL;
        }
    }
    return ret;    
}

/**
 * cfgSetOptionalParam()
 *
 * Required for changing optional values, requires unlocked config
 * Optional params may or may not be enabled or exist (not mandatory for operation)
 *
 * returns -1 (failure or no param), 0 (param exists, but disabled), 1 (param enabled)
 *
 */
int32_t cfgSetOptionalParam(char_t *param, char_t *file, char_t *value, int32_t doEnable)
{
    /* QNX really pisses me off some times; all the below handling is because the system
       command does not behave like it should; see the svn history for original implementation */
    FILE *fp;
    char_t pcmd[200]; 
    char_t poutput[MAX_TOKEN_SIZE] = {0};    /* gross overestimate for output but need to consider long url */

    if ((NULL == param) || (NULL == value)) {
        return -1;
    }

    if (NULL != file) {
        if (!strlen(file)) {
            file = NULL;
        }
    }

    if (NULL != file) {
        snprintf(pcmd, sizeof(pcmd), "{ dcu -o %s -f %s -v %s:%s; echo $?; }", param, file, (doEnable) ? "e" : "s", value);
    } else {
        snprintf(pcmd, sizeof(pcmd), "{ dcu -o %s -v %s:%s; echo $?; }", param, (doEnable) ? "e" : "s", value);
    }
    if (NULL != (fp = popen(pcmd, "r"))) {
        if(NULL != fgets(poutput, MAX_TOKEN_SIZE, fp)){
            pclose(fp);
            switch (strtoul(poutput, NULL, 10)) {
                case 0: return 0;
                case 1: return 1;
                /* case 255: return -1; */    /* QNX doesn't support negative shell results; 255 = -1 */
                default: return -1;
            }
        } else {
            pclose(fp);
        }
    }

    return -1;
}

/**
 * cfgOptionalParamCtl()
 *
 * Required for changing optional values enabled state, requires unlocked config
 * Optional params may or may not be enabled or exist (not mandatory for operation)
 *
 * returns -1 (failure or no param), 0 (param disabled), 1 (param enabled)
 *
 */
int32_t cfgOptionalParamCtl(char_t *param, char_t *file, int32_t enableState)
{
    /* see cfgSetOptional for why this crap exists */
    FILE *fp;
    char_t pcmd[200]; 
    char_t poutput[MAX_TOKEN_SIZE] = {0};    /* gross overestimate for output but need to consider long url */

    if (NULL == param) {
        return -1;
    }

    if (NULL != file) {
        if (!strlen(file)) {
            file = NULL;
        }
    }

    if (NULL != file) {
        snprintf(pcmd, sizeof(pcmd), "{ dcu -o %s -f %s -v %s:; echo $?; }", param, file, (enableState) ? "e" : "d");
    } else {
        snprintf(pcmd, sizeof(pcmd), "{ dcu -o %s -v %s:; echo $?; }", param, (enableState) ? "e" : "d");
    }
    if (NULL != (fp = popen(pcmd, "r"))) {
        if(NULL != fgets(poutput, MAX_TOKEN_SIZE, fp)){
            pclose(fp);
            switch (strtoul(poutput, NULL, 10)) {
                case 0: return 0;
                case 1: return 1;
                /* case 255: return -1; */    /* QNX doesn't support negative shell results; 255 = -1 */
                default: return -1;
            }
        } else {
            pclose(fp);
        }
    }

    return -1;
}

/*
 * cfgCommit()
 *
 * Required to save the config changes
 *
 * returns 0 on success
 *
 */
int32_t cfgCommit(void)
{
  int32_t ret = CFG_AOK;

  if(0 != system("dcu -c " MASTERKEY)){
#if !defined(MY_UNIT_TEST)
      ret = CFG_COMMIT_FAIL;
#endif
  }
  return ret;
}

/*
 * cfgRevert()
 *
 * Cancels any config changes in progress; locks the config so no further editing
 *
 * returns 0 on success
 *
 */
int32_t cfgRevert(void)
{
  int32_t ret = CFG_AOK;

  if(0 != system("dcu -r" )){
#if !defined(MY_UNIT_TEST)
      ret = CFG_REVERT_FAIL;
#endif
  }
  return ret;
}

/*
 * cfgUnlock()
 *
 * Required for editing files; first step
 *
 * returns 0 on success
 *
 */
int32_t cfgUnlock(void)
{
  int32_t ret = CFG_AOK;

  if(0 != system("dcu -u " MASTERKEY)){
#if !defined(MY_UNIT_TEST)
      ret = CFG_UNLOCK_FAIL;
#endif 
  }
  return ret;
}

/**

   network cfg operations 

**/


/*
 * cfgNetworkGetMode()
 *
 * Retrieves ONLY current mode active on system
 *
 * MODE means DHCP (1) or and Static (0)
 *
 * NOTE: MODE's value means the same thing as DHCP_ENABLED's value in startup.sh
 *
 * returns mode (0 or 1) on success, negative on error
 *
 */
int32_t cfgNetworkGetMode(int32_t isOBU)
{
#define RESULTLNGTH 10
    FILE *fp;
    char_t pcmd[100], result[RESULTLNGTH];    /* result is either DHCP or STATIC, 10 chars is more than enough */

    if (isOBU) {
        snprintf(pcmd, sizeof(pcmd), "dcu -n " OBU_GET_MODE);
    } else {
        snprintf(pcmd, sizeof(pcmd), "dcu -n " RSU_GET_MODE);
    }
    if (NULL != (fp = popen(pcmd, "r"))) {
        if(NULL != fgets(result, RESULTLNGTH, fp)){
            pclose(fp);
            if (NULL != strstr(result, CFG_DHCP)) {
                return 1;
            } else {
                return 0;
            }
        } else {
            pclose(fp);
        }
    }

    return -1;    
}

/*
 * cfgNetworkSetModeDHCPEnabled()
 *
 * Enables DHCP mode; requires unlocked configuration
 *
 * returns 0 on success
 *
 */
int32_t cfgNetworkSetModeDHCPEnabled(int32_t isOBU)
{
    if (isOBU) {
        if (!system("dcu -n " OBU_DHCP_ENABLE)) {
            return 0;
        }
    } else {
        if (!system("dcu -n " RSU_DHCP_ENABLE)) {
            return 0;
        }
    }

    return CFG_DHCP_ENABLE_FAIL;
}

/*
 * cfgNetworkSetModeStaticEnabled()
 *
 * Enables static IP setting, and optional updates IP; disables DHCP; requires unlocked configuration
 *
 * returns 0 on success
 *
 */
int32_t cfgNetworkSetModeStaticEnabled(char_t *staticIP, int32_t isOBU)
{
    char_t cmd[100];

    /* staticIP is optional */
    if (NULL != staticIP) {
        if (!strlen(staticIP)) {
            staticIP = NULL;
        }
    }

    if (isOBU) {
        if (NULL != staticIP) {
            snprintf(cmd, sizeof(cmd), "dcu -n " OBU_STATIC_ENABLE " -v %s", staticIP);
        } else {
            snprintf(cmd, sizeof(cmd), "dcu -n " OBU_STATIC_ENABLE);
        }
    } else {
        if (NULL != staticIP) {
            snprintf(cmd, sizeof(cmd), "dcu -n " RSU_STATIC_ENABLE " -v %s", staticIP);
        } else {
            snprintf(cmd, sizeof(cmd), "dcu -n " RSU_STATIC_ENABLE);
        }
    }
    if (!system(cmd)) {
        return 0;
    }

    return -1;
}

/*
 * cfgNetworkStaticIP()
 *
 * Sets static IP but does not change current mode; requires unlocked configuration
 *
 * returns 0 on success
 *
 */


int32_t cfgNetworkSetStaticIP(char_t *staticIP, int32_t isOBU)
{
    char_t cmd[100];

    /* staticIP is required */
    if (NULL != staticIP) {
        if (!strlen(staticIP)) {
            return -1;
        }
    }
    if(CFG_AOK == cfgCheckRSUINFO("SC0IPADDRESS", staticIP)) {
        if (isOBU) {
            snprintf(cmd, sizeof(cmd), "dcu -n " OBU_SET_STATIC_IP " -v %s", staticIP);
        } else {
            snprintf(cmd, sizeof(cmd), "dcu -n " RSU_SET_STATIC_IP " -v %s", staticIP);
        }
        if (!system(cmd)) {
            return 0;
        }
    }

    return -1;
}

/**
 * cfgNetworkGetIPcfgNetworkGetIP()
 *
 * Retrieves static IP set in configuration; this IP may not match the active IP (if DHCP is enabled)
 *
 * returns 0 on success
 *
 */


int32_t cfgNetworkGetIP(char_t *result, int32_t isFECIP)
{
    FILE *fp;
    char_t *ptr, pcmd[100];

    if (isFECIP) {
        snprintf(pcmd, sizeof(pcmd), "dcu -n " OBU_GET_IP);
    } else {
        snprintf(pcmd, sizeof(pcmd), "dcu -n " RSU_GET_IP);
    }
    if (NULL != (fp = popen(pcmd, "r"))) {
        if(NULL != fgets(result, MAX_TOKEN_SIZE, fp)) {
            pclose(fp);
            /* protect against fgets adding a '\n' */
            if (strlen(result) > 0) {
                ptr = result + strlen(result) - 1;
                if (*ptr == '\n') *ptr = '\0';    /* NULL terminate there */
            }
        } else {
            pclose(fp);
        }
        return 0;
    }

    return -1;
}

/**
 * cfgNetworkGetMask()
 *
 * Retrieves static IP mask set in configuration; this mask may not match the active mask (if DHCP is enabled)
 *
 * returns mask on success
 *
 */
int32_t cfgNetworkGetMask(int32_t isFECIP)
{
    FILE   * fp = NULL;
    char_t   *ptr=NULL,result[10], pcmd[100];
    int32_t  val = CFG_GET_NETMASK_FAIL; /* FAIL till proven SUCCESS. */
    uint32_t status = I2V_RETURN_OK;

    memset(result,0,sizeof(result));
    memset(pcmd,0,sizeof(pcmd));

    if (isFECIP) {
        snprintf(pcmd, sizeof(pcmd), "dcu -n " OBU_GET_NETMASK);
    } else {
        snprintf(pcmd, sizeof(pcmd), "dcu -n " RSU_GET_NETMASK);
    }
    if (NULL != (fp = popen(pcmd, "r"))) {
        if(NULL != fgets(result, sizeof(result), fp)) {
            /* protect against fgets adding a '\n' */
            if (strlen(result) > 0) {
                ptr = result + strlen(result) - 1;
                if (*ptr == '\n') *ptr = '\0';    /* NULL terminate there */
            }
            /* Ensures we dont get fooled by junk magically being a valid mask ie "20foo" */
            if(I2V_RETURN_OK != i2vUtilUpdateInt32Value(&val,result,I2V_RSU_NETMASK_MIN_S,I2V_RSU_NETMASK_MAX_S,NULL,&status,NULL)){
                val = CFG_GET_NETMASK_FAIL;
            }
        }
        pclose(fp);
    }
    return val;
}
/**
 * cfgNetworkSetMask()
 *
 * Sets static IP mask set in configuration; this mask may not match the active mask (if DHCP is enabled)
 * Requires unlocked config
 *
 * returns 0 on success
 *
 */
int32_t cfgNetworkSetMask(int32_t mask, int32_t isFECIP)
{
  int32_t ret = CFG_AOK;
  char_t cmd[100];

  /* only allow valid range */
  if ((mask <= 31) && (mask >= 1)) {
      if (isFECIP) {
          snprintf(cmd, sizeof(cmd), "dcu -n " OBU_SET_NETMASK " -v %d", mask);
      } else {
          snprintf(cmd, sizeof(cmd), "dcu -n " RSU_SET_NETMASK " -v %d", mask);
      }
      if(0 != system(cmd)){
          ret = CFG_SET_NETMASK_FAIL; 
      }
  } else {
      ret = CFG_PARAM_FAIL;
  }
  return ret;
}

/*
 * cfgNetworkGetMtu()
 *
 * Retrieves ONLY current mtu active on system
 *
 * returns mtu on success, negative on error
 *
 */
int32_t cfgNetworkGetMtu(void)
{
#define MIN_MTU 68
#define MAX_MTU 1500
    FILE *fp;
    char_t pcmd[100], result[RESULTLNGTH] = {0};    /* result should be no greater than 1500, 10 chars is more than enough */
    int32_t mtu = CFG_GET_MTU_FAIL;

    snprintf(pcmd, sizeof(pcmd), "dcu -n " RSU_GET_MTU);
    if (NULL != (fp = popen(pcmd, "r"))) {
        if(NULL != fgets(result, sizeof(result), fp)){
            mtu = atoi(result);
            if ((mtu < MIN_MTU) || (MAX_MTU < mtu)) {
                mtu = CFG_GET_MTU_FAIL;
            }
        }
        pclose(fp);
    }
    return mtu;
}

/*
 * cfgNetworkSetMtu()
 *
 * Sets MTU; requires unlocked configuration
 *
 * returns 0 on success
 *
 */
int32_t cfgNetworkSetMtu(int32_t mtu)
{
    int32_t ret = CFG_AOK;
    char_t cmd[100];

    if (mtu >= MIN_MTU && mtu < MAX_MTU) {
        snprintf(cmd, sizeof(cmd), "dcu -n " RSU_SET_MTU " -v %d", mtu);
        if(0 != system(cmd)){
            ret = CFG_SET_MTU_FAIL; 
        }
    } else {
        ret = CFG_PARAM_FAIL;
    }
    return ret;
}

/*
 * cfgNetworkGetVLANEnable()
 *
 * Retrieves ONLY current VLAN mode active on system
 *
 * MODE means ON (1) or and OFF (0)
 *
 * returns mode (0 or 1) on success, negative on error
 *
 */
int32_t cfgNetworkGetVLANEnable(void)
{
    FILE *fp;
    char_t pcmd[100], result[RESULTLNGTH];    /* result is either ON or OFF, 10 chars is more than enough */

    snprintf(pcmd, sizeof(pcmd), "dcu -n " RSU_GET_VLAN_EN);
    if (NULL != (fp = popen(pcmd, "r"))) {
        if(NULL != fgets(result, RESULTLNGTH, fp)){
            pclose(fp);
            if (NULL != strstr(result, CFG_ON)) {
                return 1;
            } else {
                return 0;
            }
        } else {
            pclose(fp);
        }
    }

    return CFG_GET_VLAN_ENABLE_FAIL;    
}

/*
 * cfgNetworkSetVLANEnable()
 *
 * Sets VLAN mode; requires unlocked configuration
 *
 * returns 0 on success
 *
 */
int32_t cfgNetworkSetVLANEnable(int32_t enable)
{
    int32_t ret = CFG_AOK;
    char_t cmd[100];

    if (enable == 0 || enable == 1) {
        snprintf(cmd, sizeof(cmd), "dcu -n " RSU_SET_VLAN_EN " -v %d", enable);
        if(0 != system(cmd)){
            ret = CFG_SET_VLAN_ENABLE_FAIL; 
        }
    } else {
        ret = CFG_PARAM_FAIL;
    }
    return ret;
}

/**
 * cfgNetworkGetVLANIP()
 *
 * Retrieves VLAN IP set in configuration
 *
 * returns 0 on success
 *
 */
int32_t cfgNetworkGetVLANIP(char_t *result)
{
    FILE *fp;
    char_t *ptr, pcmd[100];

    snprintf(pcmd, sizeof(pcmd), "dcu -n " RSU_GET_VLAN_IP);
    if (NULL != (fp = popen(pcmd, "r"))) {
        if(NULL != fgets(result, MAX_TOKEN_SIZE, fp)) {
            pclose(fp);
            /* protect against fgets adding a '\n' */
            if (strlen(result) > 0) {
                ptr = result + strlen(result) - 1;
                if (*ptr == '\n') *ptr = '\0';    /* NULL terminate there */
            }
        } else {
            pclose(fp);
        }
        return 0;
    }

    return -1;
}

/*
 * cfgNetworkSetVLANIP()
 *
 * Sets VLAN IP; requires unlocked configuration
 *
 * returns 0 on success
 *
 */
int32_t cfgNetworkSetVLANIP(char_t *staticIP)
{
    char_t cmd[100];

    /* staticIP is required */
    if (NULL != staticIP) {
        if (!strlen(staticIP)) {
            return -1;
        }
    }
    if(CFG_AOK == cfgCheckRSUINFO("VLAN_IP", staticIP)) {
        snprintf(cmd, sizeof(cmd), "dcu -n " RSU_SET_VLAN_IP " -v %s", staticIP);
        if (!system(cmd)) {
            return 0;
        }
    }

    return -1;
}

/**
 * cfgNetworkGetVLANMask()
 *
 * Retrieves VLAN IP mask set in configuration
 *
 * returns mask on success
 *
 */
int32_t cfgNetworkGetVLANMask(void)
{
    FILE   * fp = NULL;
    char_t   *ptr=NULL,result[10], pcmd[100];
    int32_t  val = CFG_GET_VLAN_MASK_FAIL; /* FAIL till proven SUCCESS. */
    uint32_t status = I2V_RETURN_OK;

    memset(result,0,sizeof(result));
    memset(pcmd,0,sizeof(pcmd));

    snprintf(pcmd, sizeof(pcmd), "dcu -n " RSU_GET_VLAN_MASK);
    if (NULL != (fp = popen(pcmd, "r"))) {
        if(NULL != fgets(result, sizeof(result), fp)) {
            /* protect against fgets adding a '\n' */
            if (strlen(result) > 0) {
                ptr = result + strlen(result) - 1;
                if (*ptr == '\n') *ptr = '\0';    /* NULL terminate there */
            }
            /* Ensures we dont get fooled by junk magically being a valid mask ie "20foo" -- applies for VLAN mask too */
            if(I2V_RETURN_OK != i2vUtilUpdateInt32Value(&val,result,I2V_RSU_NETMASK_MIN_S,I2V_RSU_NETMASK_MAX_S,NULL,&status,NULL)){
                val = CFG_GET_VLAN_MASK_FAIL;
            }
        }
        pclose(fp);
    }
    return val;
}
/**
 * cfgNetworkSetVLANMask()
 *
 * Sets VLAN IP mask set in configuration
 * Requires unlocked config
 *
 * returns 0 on success
 *
 */
int32_t cfgNetworkSetVLANMask(int32_t mask)
{
  int32_t ret = CFG_AOK;
  char_t cmd[100];

  /* only allow valid range */
  if ((mask <= 31) && (mask >= 1)) {
      snprintf(cmd, sizeof(cmd), "dcu -n " RSU_SET_VLAN_MASK " -v %d", mask);
      if(0 != system(cmd)){
          ret = CFG_SET_VLAN_MASK_FAIL; 
      }
  } else {
      ret = CFG_PARAM_FAIL;
  }
  return ret;
}


/*
 * cfgNetworkGetVLANID()
 *
 * Retrieves ONLY current VLAN ID active on system
 *
 * returns VLAN ID on success, negative on error
 *
 */
int32_t cfgNetworkGetVLANID(void)
{
/* MIN_VLAN allowed to be 0; min can set is 1; MAX_VLAN is 4095 */
#define MIN_VLAN 0
#define MAX_VLAN 4096
    FILE *fp;
    char_t pcmd[100], result[RESULTLNGTH] = {0};    /* result should be no greater than 65535, 10 chars is more than enough */
    int32_t vlanid = CFG_GET_VLAN_ID_FAIL;

    snprintf(pcmd, sizeof(pcmd), "dcu -n " RSU_GET_VLAN_ID);
    if (NULL != (fp = popen(pcmd, "r"))) {
        if(NULL != fgets(result, sizeof(result), fp)){
            vlanid = atoi(result);
            if ((vlanid < MIN_VLAN) || (MAX_VLAN <= vlanid)) {
                vlanid = CFG_GET_VLAN_ID_FAIL;
            }
        }
        pclose(fp);
    }
    return vlanid;
}

/*
 * cfgNetworkSetVLANID()
 *
 * Sets VLAN ID; requires unlocked configuration
 *
 * returns 0 on success
 *
 */
int32_t cfgNetworkSetVLANID(int32_t vlanid)
{
    int32_t ret = CFG_AOK;
    char_t cmd[100];

    /* see comment above for why MIN_VLAN is not >= */
    if (vlanid > MIN_VLAN && vlanid < MAX_VLAN) {
        snprintf(cmd, sizeof(cmd), "dcu -n " RSU_SET_VLAN_ID " -v %d", vlanid);
        if(0 != system(cmd)){
            ret = CFG_SET_VLAN_ID_FAIL; 
        }
    } else {
        ret = CFG_PARAM_FAIL;
    }
    return ret;
}


/**
 * cfgNetworkGetHttpPort()
 * cfgNetworkGetHttpsPort()
 *
 * Returns the port number for the webgui http and https services.
 */

int32_t cfgNetworkGetHttpPort(void)
{
    FILE *fp;
    char_t result[10] = {0}, pcmd[100];
    int32_t port = CFG_GET_HTTP_PORT_FAIL;

    snprintf(pcmd, sizeof(pcmd), "dcu -n " RSU_GET_HTTP_PORT);
    if (NULL != (fp = popen(pcmd, "r"))) {
        if(NULL != fgets(result, sizeof(result), fp)){
            port = atoi(result);
            if ((port < I2V_DEST_PORT_MIN) || (I2V_DEST_PORT_MAX <= port)) {
                port = CFG_GET_HTTP_PORT_FAIL;
            }
        }
        pclose(fp);
    }
    return port;
}

//check
int32_t cfgNetworkGetHttpsPort(void)
{
    FILE *fp;
    char_t result[10] = {0}, pcmd[100];
    int32_t port = CFG_GET_HTTPS_PORT_FAIL;

    snprintf(pcmd, sizeof(pcmd), "dcu -n " RSU_GET_HTTPS_PORT);
    if (NULL != (fp = popen(pcmd, "r"))) {
        if(NULL != fgets(result, sizeof(result), fp)){
            port = atoi(result);
            if ((port < I2V_DEST_PORT_MIN) || (I2V_DEST_PORT_MAX <= port)) {
                port = CFG_GET_HTTPS_PORT_FAIL;
            }
        }
        pclose(fp);
    }
    return port;
}

/**
 * cfgNetworkSetHttpPort()
 * cfgNetworkSetHttpsPort()
 *
 * Sets the port number for the webgui http and https services.
 * Requires unlocked config
 *
 * returns 0 on success
 *
 */
int32_t cfgNetworkSetHttpPort(int32_t port)
{
  int32_t ret = CFG_AOK;
  char_t cmd[100];

  if (port >= I2V_DEST_PORT_MIN && port < I2V_DEST_PORT_MAX) {
      snprintf(cmd, sizeof(cmd), "dcu -n " RSU_SET_HTTP_PORT " -v %d", port);
      if(0 != system(cmd)){
          ret = CFG_SET_HTTP_PORT_FAIL; 
      }
  } else {
      ret = CFG_PARAM_FAIL;
  }
  return ret;
}
int32_t cfgNetworkSetHttpsPort(int32_t port)
{
  int32_t ret = CFG_AOK;
  char_t cmd[100];

  if (port >= I2V_DEST_PORT_MIN && port < I2V_DEST_PORT_MAX) {
      snprintf(cmd, sizeof(cmd), "dcu -n " RSU_SET_HTTPS_PORT " -v %d", port);
      if(0 != system(cmd)){
          ret = CFG_SET_HTTPS_PORT_FAIL; 
      }
  } else {
      ret = CFG_PARAM_FAIL;
  }
  return ret;
}



/**
 * cfgStartupSet()
 *
 * Sets i2v, rsuhealth, and system monitor in startup.sh script.
 *
 * returns 0 on success
 * 
 */
int32_t cfgStartupSet(int32_t value, char_t * conf) 
{
  int32_t ret = CFG_AOK;
  char_t cmd[200];

  if((NULL == conf) || ((0 != value) && (1 != value))) {
      ret = CFG_PARAM_FAIL;
  } else {
      if(0 == strcmp(conf, "I2V_APP_ENABLE"))    {
          snprintf(cmd, sizeof(cmd), "dcu -n " RSU_SET_I2V  " -v %d", value);
      } else if( 0 == strcmp(conf, "RSU_HEALTH_ENABLE"))    {
          snprintf(cmd, sizeof(cmd), "dcu -n " RSU_SET_HEALTH " -v %d", value);
#if 0
      } else if( 0 == strcmp(conf, "RSU_SYSTEM_MONITOR_ENABLE"))    {
          snprintf(cmd, sizeof(cmd), "dcu -n " RSU_SET_SYSTEM_MON  " -v %d", value);
#endif
      } else {
          ret = CFG_WRITE_ITEM_NOT_FOUND;
      }
      if((CFG_AOK == ret) && (0 != system(cmd)))  {
          ret = CFG_SET_NETMASK_FAIL; 
      }
  }
  return ret;
}


/**

   management operations 

**/


/**
 * cfgResetToDefault(int bool_preserve_networking)
 *
 * Resets configuration to default deployed in firmware;
 * Two modes: everything reverts (including IPs), or
 *            everything but networking reverts.
 * This will interrupt any in progress config operations
 *
 * returns 0 on success
 *
 */
int32_t cfgResetToDefault(int32_t bool_preserve_networking)
{
    char_t cmd[512];
    int32_t rc;
    //TODO: Need to restore FWDMSG defaults using dcu journal below.
    if (bool_preserve_networking) {
        rc = system("dcu -m " MASTERKEY " -v " RESETDFLTSBUTSAVENET);
    } else {
        rc = system("dcu -m " MASTERKEY " -v " RESETDFLTS);
    }
    /* These resets do not require MASTERKEY unlocking. Keep out of DCU for now. */
    if (!rc) {
        /* Remove AMH files for legacy 4.1 SAR and ntcip-1218 rsuMsgRepeat. */
        memset(cmd,0x0,sizeof(cmd));
        snprintf(cmd,sizeof(cmd),"/bin/rm -rf %s",AMH_ACTIVE_DIR);
        rc = system(cmd);
    }
    if (!rc) {
        /* Remove ntcip-1218 rsuIFM. Not at root of ntcip-1218 MIB. */
        memset(cmd,0x0,sizeof(cmd));
        snprintf(cmd,sizeof(cmd),"/bin/rm -rf %s",IFM_ACTIVE_DIR);
        rc = system(cmd);
    }
    if (!rc) {
        /* Remove the remainder of ntcip-1218 MIB.SYS_DESC_PATH is at root of MIB. */
        memset(cmd,0x0,sizeof(cmd));
        snprintf(cmd,sizeof(cmd),"/bin/rm -rf %s",SYS_DESC_PATH);
        rc = system(cmd);
    }
    if (!rc) {
        return 0;
    } else {
        return -1;
    }
}

/**
 * cfgReplaceTscbm()
 *
 * Copies content of passed file into the tscbm.conf config file
 *
 * NOTE: Not really a network configuration, but we shoehorn it into dcu -n
 *       because all dcu's network ops do complex operations already and we
 *       don't want to create a new complex-op dc param just for tscbm.
 *
 * returns 0 on success
 */
int32_t cfgReplaceTscbm(char_t * filepath)
{
  int32_t ret = CFG_AOK;
  char_t cmd[500];
  
  if((NULL == filepath) || (450 < strlen(filepath))){
      ret = CFG_PARAM_FAIL;
  } else {
      snprintf(cmd,500, "dcu -n " TSCBM_UPDATE " -v %s", filepath);
      if(0 != system(cmd)){
          ret = CFG_TSCBM_UPDATE_FAIL; /* Fatal to RSU. */
      }
  }
  return ret;
}

/**
 * customerBackup()
 *
 * Creates a backup blob of the current configs. 
 * Doesn't do anything to AMH files, etc, outside of the blob. 
 *
 * returns 0 on success
 *
 */
int32_t customerBackup(void)
{
    int32_t rc;
    
    rc = system("dcu -b");
    
    if (!rc) {
        return 0;
    } else {
        return -1;
    }
}

/**
 * cfgResetToCustomer()
 *
 * Resets configuration to the customer-created backup point. 
 * Doesn't do anything to AMH files, etc, outside of the blob. 
 * If no customer blob exists, does nothing. 
 *
 * returns 0 on success
 *
 */
int32_t cfgResetToCustomer(void)
{
    char_t cmd[512];
    int32_t rc;
    
    sprintf(cmd, "[[ -e " CUST_CFG_BLOB " ]]");
    rc = system(cmd);
    if (rc) {
        printf("Error: No customer backup found. \n");
        return -1;
    }
    
    rc = system("dcu -m " MASTERKEY " -v " RESETCUSTOMER);
    
    if (!rc) {
        return 0;
    } else {
        return -1;
    }
}

#ifdef DIAG_OPS

/**
 * cfgTermRSUHealth()
 *
 * Terminates RSU health script
 *
 * returns 0 on success
 *
 */
int32_t cfgTermRSUHealth(void)
{
    if (!system("dcu -m " MASTERKEY " -v " HLTHACTVTYNIX)) {
        return 0;
    }

    return -1;
}

/**
 * cfgTermRSUSvcs()
 *
 * Terminates RSU health script AND i2v
 *
 * returns 0 on success
 *
 */
int32_t cfgTermRSUSvcs(void)
{
    if (!system("dcu -m " MASTERKEY " -v " STOPI2VTERMHLTH)) {
        return 0;
    }

    return -1;
}

#endif /* DIAG_OPS */

