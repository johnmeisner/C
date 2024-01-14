/*
 *  Filename: conf_manager.c
 *  Purpose: Configuration manager for Web interface
 *
 *  Copyright (C) 2021 DENSO International America, Inc.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <mqueue.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <signal.h>
#include <time.h>
#include <pthread.h>
#include "dn_types.h"
#include "cfgmgrapi.h"
#include "conf_manager.h"
#include "conf_table.h"
#include "i2v_util.h"

#if defined(MY_UNIT_TEST)
#include "stubs.h"
#define MAIN conf_manager_main
#else
#define MAIN main
#endif

#if defined(EXTRA_DEBUG)
#define OUTPUT_MODULUS  30  /* Seconds. */
#else
#define OUTPUT_MODULUS  1200
#endif

//TODO: Need syslog only level to avoid STDOUT, STDERR. Those will break web gui return values.
/* mandatory logging defines */
#define MY_ERR_LEVEL   LEVEL_INFO    /* from i2v_util.h */
#define MY_NAME        "cfgmgr"

#define POLL_TIMER_INT_SECS 5
#define SESSION_TIMEOUT  (10*60)          /* default to 10 minutes */

/* Globals */
STATIC session_t conf_session = {LOG_OFF, 30, "0.0.0.0"};    /* Config session default */
STATIC bool_t mainloop = WTRUE;
/* Main loop */
STATIC bool_t loop = WTRUE;
STATIC pthread_t config_mq_thread;          /* Config manager MQ thread */
STATIC pthread_t timeout_thread;            /* Timeout thread */
STATIC char_t mq_in_buffer [MSG_BUFFER_SIZE];
/* Message Queue */
STATIC mqd_t qd_server;
STATIC struct mq_attr attr;

/*
 * ===============================
 * ============ RSU_INFO =========
 * ===============================
 * 
 * RSU Info
 *      - Network info
 *      - CPU Tempurature
 *      - Up time
 *      - others
 * 
 * Network related APIs
 *      Enables DHCP mode; requires unlocked configuration
 *      - int cfgNetworkDHCPEnable(int isOBU);
 * 
 *      Retrieves ONLY current mode active on system
 *      - int cfgNetworkMode(int isOBU);          
 *      
 *      Enables static IP setting, and optional updates IP; disables DHCP; requires unlocked configuration
 *      - int cfgNetworkStaticEnable(char *staticIP, int isOBU);  
 * 
 *      Sets static IP but does not change current mode; requires unlocked configuration
 *      - int cfgNetworkStaticIP(char *staticIP, int isOBU);
 *
 */ 
//TODO: Irregulars: from startup.sh, runtime calls but not from I2V confs

STATIC conf_element_t RSU_INFO_Table[] = {
    {"NETWORK_MODE", I2V_RSU_NETWORK_MODE_DEFAULT_S, UNINIT},
    {"SC0IPADDRESS", I2V_RSU_ETH0_IP_DEFAULT_S, UNINIT},
    {"NETMASK", I2V_RSU_NETMASK_DEFAULT_S, UNINIT},
    {"I2V_APP_ENABLE", I2V_APP_ENABLE_DEFAULT_S, UNINIT},
    {"RSU_HEALTH_ENABLE", I2V_RSUHEALTH_ENABLE_DEFAULT_S, UNINIT},
    {"MTU", I2V_RSU_MTU_DEFAULT_S, UNINIT},
    {"VLAN_ENABLE", I2V_RSU_VLAN_ENABLE_DEFAULT_S, UNINIT},
    {"VLAN_IP", I2V_RSU_VLAN_IP_DEFAULT_S, UNINIT},
    {"VLAN_MASK", I2V_RSU_VLAN_MASK_DEFAULT_S, UNINIT},
    {"VLAN_ID", I2V_RSU_VLAN_ID_DEFAULT_S, UNINIT},
#if 0
    {"RSU_SYSTEM_MONITOR_ENABLE", I2V_SYS_MON_ENABLE_DEFAULT_S, UNINIT},
#endif
    {"WEBGUI_HTTP_SERVICE_PORT", WEBGUI_HTTP_SERVICE_PORT_DEFAULT_S, UNINIT},
    {"WEBGUI_HTTPS_SERVICE_PORT", WEBGUI_HTTPS_SERVICE_PORT_DEFAULT_S, UNINIT},
};

/*
 * ===============================  
 * ============ I2V ==============
 * ===============================
 */
STATIC conf_element_t I2V_Table[] = {
    {"I2VRSUID", I2VRSUID_DEFAULT_S, UNINIT},
/* I2V App Enable Flags:SPAT is enabled via SCS */
    {"I2VSCSAppEnable", I2V_SCS_ENABLE_DEFAULT_S, UNINIT},
    {"I2VSRMAppEnable", I2V_SRM_ENABLE_DEFAULT_S, UNINIT},
    {"I2VIPBAppEnable", I2V_IPB_ENABLE_DEFAULT_S, UNINIT},
    {"I2VAMHAppEnable", I2V_AMH_ENABLE_DEFAULT_S, UNINIT},
    {"I2VIWMHAppEnable", I2V_IWMH_ENABLE_DEFAULT_S, UNINIT},
    {"I2VFWDMSGAppEnable", I2V_IWMH_ENABLE_DEFAULT_S, UNINIT},
    {"I2VGlobalDebugFlag", I2V_GLOBAL_DEBUG_DEFAULT_S, UNINIT},
    {"I2VDebugFlag", I2V_DEBUG_DEFAULT_S, UNINIT},
    {"I2VGPSOverrideEnable", I2V_GPS_OVERRIDE_ENABLE_DEFAULT_S, UNINIT},
    {"I2VGPSOverrideLat", I2V_GPS_LAT_DEFAULT_S, UNINIT},
    {"I2VGPSOverrideLong", I2V_GPS_LON_DEFAULT_S, UNINIT},
    {"I2VGPSOverrideElev", I2V_GPS_ELEV_DEFAULT_S, UNINIT},
    {"I2VEnableEthIPv6", I2V_ENABLE_IPV6_DEFAULT_S, UNINIT},
    {"I2VEthIPv6Addr", I2V_ETH_IPV6_ADDR_DEFAULT_S, UNINIT},
    {"I2VEthIPv6Scope",I2V_ETH_IPV6_SCOPE_DEFAULT_S, UNINIT},
    {"I2VIPv6DefaultGateway", I2V_IPV6_GATEWAY_DEFAULT_S, UNINIT},
    {"I2VForceAlive", I2V_FORCE_ALIVE_DEFAULT_S, NONE},
    {"I2VControlRadio", I2V_CONTROL_RADIO_DEFAULT_S, UNINIT},
    {"I2VRadioSecVerify", I2V_RADIO_SEC_VERIFY_DEFAULT_S, NONE},
    {"I2VRadio0Enable", I2V_RADIO0_ENABLE_DEFAULT_S, UNINIT},
    {"I2VRadio0Mode", I2V_RADIO0_MODE_DEFAULT_S, UNINIT},
    {"I2VRadio0Channel", I2V_RADIO_CHANNEL_DEFAULT_S, UNINIT},
    {"I2VRadio1Enable", I2V_RADIO1_ENABLE_DEFAULT_S, UNINIT},
    {"I2VRadio1Mode", I2V_RADIO1_MODE_DEFAULT_S, UNINIT},
    {"I2VRadio1Channel", I2V_RADIO_CHANNEL_DEFAULT_S, UNINIT},
    {"I2VRadio2Enable", I2V_RADIO2_ENABLE_DEFAULT_S, UNINIT},
    {"I2VRadio2Mode", I2V_RADIO2_MODE_DEFAULT_S, UNINIT},
    {"I2VRadio2Channel", I2V_RADIO_CHANNEL_DEFAULT_S, UNINIT},
    {"I2VRadio3Enable", I2V_RADIO3_ENABLE_DEFAULT_S, UNINIT},
    {"I2VRadio3Mode", I2V_RADIO3_MODE_DEFAULT_S, UNINIT},
    {"I2VRadio3Channel", I2V_RADIO_CHANNEL_DEFAULT_S, UNINIT},
    {"I2VBroadcastUnified", I2V_BROADCAST_UNIFIED_DEFAULT_S, UNINIT},
    {"I2VUnifiedChannelNumber", I2V_RADIO_CHANNEL_DEFAULT_S, UNINIT},
    {"I2VUnifiedRadioNumber", I2V_RADIO_DEFAULT_S, UNINIT},
/* Conflicts with NTCIP-1218: 5.10.2.1 - 5.10.2.16: WSA Table: rsuWsaServiceTable? 1 to 7 there. */
    {"I2VUnifiedWSAPriority", I2V_WSA_UNIFIED_PRIORITY_DEFAULT_S, UNINIT},
    {"I2VSecurityEnable", I2V_SECURITY_ENABLE_DEFAULT_S, UNINIT},
    {"I2VTransmitPower", I2V_TX_POWER_DEFAULT_S, UNINIT},
    {"I2VSecurityTimeBuffer", I2V_SEC_TIME_BUFFER_DEFAULT_S, NONE},
    {"I2VCertAttachRate", I2V_CERT_ATTACH_RATE_DEFAULT_S, UNINIT},
    {"I2VGenDefaultCert", I2V_GEN_DEFAULT_CERTS_DEFAULT_S, UNINIT},
    {"I2VAutoGenCertRestart", I2V_AUTO_GEN_CERT_RESTART_DEFAULT_S, UNINIT},
    {"I2VStorageBypass", I2V_STORAGE_BYPASS_DEFAULT_S, UNINIT},
    {"I2VDisableSysLog", I2V_DISABLE_SYSLOG_DEFAULT_S, UNINIT},
    {"I2VConsoleCleanup", I2V_CONSOLE_CLEANUP_DEFAULT_S, UNINIT},
    {"I2VMAPSecPSID", I2V_MAP_PSID_DEFAULT_S, UNINIT},
    {"I2VSPATSecPSID",I2V_SPAT_PSID_DEFAULT_S, UNINIT},
    {"I2VTIMSecPSID", I2V_TIM_PSID_DEFAULT_S, UNINIT},
    {"I2VIPBSecPSID", I2V_IPB_PSID_DEFAULT_S, UNINIT},
    {"BSMRxPSIDDER",  I2V_BSM_PSID_DEFAULT_S, UNINIT},
    {"BSMRxPSIDPERA", I2V_BSM_PSID_DEFAULT_S, UNINIT},
    {"BSMRxPSIDPERU", I2V_BSM_PSID_DEFAULT_S, UNINIT},
    {"I2VWSAEnable", I2V_WSA_ENABLE_DEFAULT_S, UNINIT},
    {"I2VRadioType", I2V_RADIO_TYPE_DEFAULT_S, UNINIT},
    {"I2VSPATSSPEnable",I2V_SPAT_SSP_ENABLE_DEFAULT_S, UNINIT},
    {"I2VSPATBITMAPPEDSSPEnable",I2V_SPAT_BITMAPPED_SSP_ENABLE_DEFAULT_S, UNINIT},
    {"I2VSPATSspString", I2V_SPAT_SSP_STRING_DEFAULT_S, UNINIT},
    {"I2VSPATSspMaskString", I2V_SPAT_SSP_MASK_STRING_DEFAULT_S, UNINIT},
    {"I2VMAPSSPEnable",I2V_MAP_SSP_ENABLE_DEFAULT_S, UNINIT},
    {"I2VMAPBITMAPPEDSSPEnable",I2V_MAP_BITMAPPED_SSP_ENABLE_DEFAULT_S, UNINIT},
    {"I2VMAPSspString", I2V_MAP_SSP_STRING_DEFAULT_S, UNINIT},
    {"I2VMAPSspMaskString", I2V_MAP_SSP_MASK_STRING_DEFAULT_S, UNINIT},
    {"I2VTIMSSPEnable",I2V_TIM_SSP_ENABLE_DEFAULT_S, UNINIT},
    {"I2VTIMBITMAPPEDSSPEnable",I2V_TIM_BITMAPPED_SSP_ENABLE_DEFAULT_S, UNINIT},
    {"I2VTIMSspString", I2V_TIM_SSP_STRING_DEFAULT_S, UNINIT},
    {"I2VTIMSspMaskString", I2V_TIM_SSP_MASK_STRING_DEFAULT_S, UNINIT},
    {"I2VUseDeviceID", I2V_USE_DEVICE_ID_DEFAULT_S, UNINIT},
    {"I2VUseGPSTXControl", I2V_USE_GPS_TX_CONTROL_DEFAULT_S, UNINIT},
    {"I2VEnableVOD", I2V_ENABLE_VOD_DEFAULT_S, UNINIT},
    {"I2VSecurityVTPMsgRateMs", I2V_SECURITY_VTP_MSG_RATE_DEFAULT_S, UNINIT},
};

/*
 * ===============================
 * ============ AMH ==============
 * ===============================
 */
STATIC conf_element_t AMH_Table[] = {
    {"RadioNum", I2V_RADIO_DEFAULT_S, UNINIT},
    {"ChannelNumber",I2V_RADIO_CHANNEL_DEFAULT_S , UNINIT},
    {"AMHBroadcastDir",AMH_ACTIVE_DIR, UNINIT}, /* User can change it but in reality its fixed at compile time. */
    {"EnableValidation",AMH_ENABLE_VERIFICATION_DEFAULT_S, UNINIT},
    {"EnableStoreAndRepeat", AMH_ENABLE_SAR_DEFAULT_S, UNINIT},
    {"MaxStoreMessages", AMH_MAX_STORE_MESSAGES_DEFAULT_S, UNINIT},
    {"SettingsOverride", AMH_SETTINGS_OVERRIDE_DEFAULT_S, UNINIT},
    {"ForcePSIDMatch",AMH_FORCE_PSID_DEFAULT_S , UNINIT},
    {"SendOverride", AMH_SEND_OVERRIDE_DEFAULT_S, UNINIT},
    {"WSMExtension", AMH_WSM_EXTENSION_DEFAULT_S, UNINIT},
    {"ImmediateEnable", AMH_ENABLE_IMF_DEFAULT_S, UNINIT},
    {"ImmediateIPFilter", AMH_IMF_IP_FILTER_DEFAULT_S, UNINIT},
    {"ImmediateIP", AMH_IMF_IP_DEFAULT_S, UNINIT},
    {"ImmediatePort", I2V_IMF_PORT_DEFAULT_S, UNINIT},
    {"AMHForwardEnable",I2V_AMH_FORWARD_ENABLE_DEFAULT_S, UNINIT},
    {"AMHIntervalSelect",AMH_INTERVAL_SELECT_DEFAULT_S,UNINIT},
};

/*
 * ===============================
 * ============ IPB ==============
 * ===============================
 */
STATIC conf_element_t IPB_Table[] = {
    {"RadioNum", I2V_RADIO_DEFAULT_S, UNINIT},
    {"WSAPriority", I2V_WSA_PRIORITY_DEFAULT_S, UNINIT},
    {"WSATxRate", I2V_WSA_TX_RATE_DEFAULT_S, UNINIT},
    {"DataRate", I2V_WSA_DATA_RATE_DEFAULT_S, UNINIT},
    {"TransmitPower", I2V_TX_POWER_DEFAULT_S, UNINIT},
    {"WSAPower", I2V_TX_POWER_DEFAULT_S, UNINIT},
    {"OverridePower", IPB_OVERRIDE_POWER_DEFAULT_S, UNINIT},
    {"EnableContinuous", IPB_ENABLE_CONT_DEFAULT_S, UNINIT},
    {"OverrideCCH", IPB_OVERRIDE_CHANNEL_ENABLE_DEFAULT_S, UNINIT},
    {"CCHChannel", I2V_RADIO_CHANNEL_DEFAULT_S, UNINIT},
    {"SecurityDebugEnable", IPB_SECURITY_DEBUG_ENABLE_DEFAULT_S, UNINIT},
    {"IPBPSID", I2V_IPB_PSID_DEFAULT_S, UNINIT},
    {"ChannelNumber", I2V_RADIO_CHANNEL_DEFAULT_S, UNINIT},
    {"IPBEnableEDCA", IPB_ENABLE_EDCA_DEFAULT_S, UNINIT},
    {"IPBAdvertiseID", IPB_AD_ID_DEFAULT_S, UNINIT},
    {"IPBMinSatellites", IPB_SV_COUNT_DEFAULT_S, UNINIT},
    {"IPBSatelliteTO", IPB_GNSS_TO_DEFAULT_S, UNINIT},
    {"IPBGPSOverride", IPB_GNSS_OVERRIDE_DEFAULT_S, UNINIT},
    {"IPBGPSSettleTime", IPB_GNSS_SETTLE_TIME_DEFAULT_S, UNINIT},
    {"Elevation", I2V_GPS_ELEV_DEFAULT_S, UNINIT},
    {"Latitude", I2V_GPS_LAT_DEFAULT_S, UNINIT},
    {"Longitude", I2V_GPS_LON_DEFAULT_S, UNINIT},
    {"PosConfidence", IPB_POS_CONFIDENCE_DEFAULT_S, UNINIT},
    {"IPBIPv6Prefix", IPB_IPV6_PREFIX_DEFAULT_S, UNINIT},
    {"IPBIPv6Identifier", IPB_IPV6_ID_DEFAULT_S , UNINIT},
    {"IPBIPv6Port", IPB_IPV6_PORT_DEFAULT_S, UNINIT},
    {"IPv6ServerPrefix", IPB_IPV6_SERVER_PREFIX_DEFAULT_S, UNINIT},
    {"IPv6ServerID", IPB_IPV6_SERVER_ID_DEFAULT_S, UNINIT},
    {"IPBServiceMac", IPB_SRV_MAC_DEFAULT_S, UNINIT},
    {"IPBProviderCtxt", IPB_PROVIDER_CTXT_DEFAULT_S, UNINIT},
    {"IPBIPv6UseLinkLocal", IPB_IPV6_USE_LOCAL_LINK_DEFAULT_S, UNINIT},
    {"IPBEnableBridge", IPB_ENABLE_BRIDGE_DEFAULT_S, UNINIT},
    {"IPBBridgeOnce", IPB_BRIDGE_ONCE_DEFAULT_S, UNINIT},
    {"IPBIPv4Bridge", IPB_IPV4_BRIDGE_DEFAULT_S, UNINIT},
    {"BridgeMaxClients",IPB_BRIDGE_CLIENT_DEFAULT_S, UNINIT},
    {"ConnectionTimeout", IPB_CONNECT_TO_DEFAULT_S, UNINIT},
    {"BridgeDelayClients", IPB_BRIDGE_DELAY_CLIENTS_DEFAULT_S, UNINIT},
    {"IPv4ServerIP", IPB_IPV4_SERVER_IP_DEFAULT_S, UNINIT},
    {"IPv4ServerPort", IPB_IPV4_SERVER_PORT_DEFAULT_S, UNINIT},
    {"IPBServerURL", IPB_SERVER_URL_DEFAULT_S, UNINIT},     
};


/*
 * ===============================
 * ============ SCS ==============
 * ===============================
 */
STATIC conf_element_t SCS_Table[] = {
    {"InterfaceTOM", SCS_INTERFACE_TOM_DEFAULT_S, UNINIT},
    {"UseMaxTimeToChange",SCS_USE_MAX_TIMETO_CHANGE_DEFAULT_S, UNINIT},
    {"RSUHeartBeatEnable", SCS_HEARTBEAT_ENABLE_DEFAULT_S, UNINIT},
    {"RSUHeartBeatInt", SRMRX_HEARTBEAT_INTERVAL_DEFAULT_S, UNINIT},
    {"SRMFwdIP",SRMRX_FWD_IP_DEFAULT_S, UNINIT},
    {"SRMFwdPort",SRMRX_FWD_PORT_DEFAULT_S, UNINIT},
    {"SRMFwdInterval", SRMRX_FWD_INTERVAL_DEFAULT_S, UNINIT},
    {"SRMFwdEnable", SRMRX_FWD_ENABLE_DEFAULT_S, UNINIT},
    {"SRMFwdInterface", SRMRX_FWD_INTERFACE_DEFAULT_S, UNINIT},
    {"SRMFwdRequireTTI", SRMRX_REQUIRE_TTI_DEFAULT_S, UNINIT},
    {"LocalSignalControllerIP", I2V_SCS_LOCAL_CONTROLLER_IP_DEFAULT_S, UNINIT},
    {"LocalSignalSNMPPort", I2V_SCS_LOCAL_SNMP_IP_DEFAULT_S, UNINIT},
    {"LocalSignalControllerPort", I2V_SCS_LOCAL_CONTROLLER_PORT_DEFAULT_S, UNINIT},
    {"LocalSignalIntersectionID", SCS_LOCAL_SIGNAL_INT_ID_DEFAULT_S, UNINIT},
    {"LocalSignalControllerEnable", SCS_LOCAL_CONTROLLER_ENABLE_DEFAULT_S, UNINIT},
    {"LocalSignalControllerIsSnmp", SCS_LOCAL_CONTROLLER_IS_SNMP_DEFAULT_S, UNINIT},
    {"LocalSignalControllerHW", SCS_LOCAL_SIGNAL_HW_DEFAULT_S, UNINIT},
    {"SNMPEnableSpatCommand", SCS_SPAT_SNMP_ENABLE_COMMAND_DEFAULT_S, UNINIT},
    {"SPATEnableValue", SCS_SPAT_COMMAND_DEFAULT_S, UNINIT},
    {"BypassSignalControllerNTCIP", SCS_BYPASS_SIGNAL_CONTROLLER_NTCIP_DEFAULT_S, UNINIT},
    {"BypassYellowDuration", SCS_BYPASS_YELLOW_DURATION_DEFAULT_S, UNINIT},
    {"NTCIP_YELLOW_DURATION", NTCIP_YELLOW_DURATION_DEFAULT_S, UNINIT},
};

/*
 * ===============================
 * ============ SPAT16 ===========
 * ===============================
 */

STATIC conf_element_t SPAT16_Table[] = {
    {"RadioCfg", I2V_RADIO_DEFAULT_S, UNINIT},
    {"WSAPriority", SPAT_WSA_PRIORITY_DEFAULT_S, UNINIT},
    {"WSMExtension", SPAT_WSM_EXTENSION_ENABLE_DEFAULT_S, UNINIT},
    {"SPATChannelNumber", I2V_RADIO_CHANNEL_DEFAULT_S, UNINIT},
    {"BSMRxEnable", BSM_RX_ENABLE_DEFAULT_S, UNINIT},
    {"BSMUnsecurePSIDPERU", I2V_BSM_PSID_DEFAULT_S, UNINIT},
    {"SPATInteroperabilityMode", SPAT_INTEROP_MODE_DEFAULT_S, UNINIT},
    {"IntersectionID", SPAT_INT_ID_DEFAULT_S, UNINIT},
};

/*
 * ===============================
 * ============ SRM RX ===========
 * ===============================
 */
STATIC conf_element_t SRM_RX_Table[] = {
    {"RadioCfg", I2V_RADIO_DEFAULT_S, UNINIT},
    {"SecurityDebugEnable", SRM_SECURITY_DEBUG_ENABLE_DEFAULT_S, UNINIT},
    {"SRMDecodeMethod",SRMRX_DECODE_METHOD_DEFAULT_S, UNINIT},
    {"SRMASN1DecodeMethod", SRMRX_ASN1_DECODE_METHOD_DEFAULT_S, UNINIT},
    {"SRMChannelNumber", I2V_RADIO_CHANNEL_DEFAULT_S , UNINIT},
    {"SRMRxForward", SRM_RX_FWD_ENABLE_DEFAULT_S, UNINIT},
    {"SRMUnsecurePSIDDER", SRMRX_PSID_DEFAULT_S, UNINIT},
    {"SRMUnsecurePSIDPERA", SRMRX_PSID_DEFAULT_S, UNINIT},
    {"SRMUnsecurePSIDPERU", SRMRX_PSID_DEFAULT_S, UNINIT},
    {"SRMPermissive", SRM_PERMISSIVE_DEFAULT_S, UNINIT},
    {"SRMTXVehBasicRole",SRMTX_VEHICLE_ROLE_DEFAULT_S, UNINIT},
    {"SRMVODMsgVerifyCount",SRM_VOD_MSG_VERIFY_COUNT_DEFAULT_S, UNINIT},
};

/*
 * ===============================
 * ============ FWDMSG ===========
 * ===============================
 */
STATIC conf_element_t FWDMSG_Table[] = {
    {"ForwardMessageEnable1", FWDMSG_ENABLE1_DEFAULT_S, UNINIT},
    {"ForwardMessageIP1", FWDMSG_IP1_DEFAULT_S, UNINIT},
    {"ForwardMessagePort1",FWDMSG_PORT1_DEFAULT_S, UNINIT},
    {"ForwardMessageMask1", FWDMSG_MASK1_DEFAULT_S, UNINIT},
    {"ForwardMessageEnable2", FWDMSG_ENABLE2_DEFAULT_S, UNINIT},
    {"ForwardMessageIP2", FWDMSG_IP2_DEFAULT_S, UNINIT},
    {"ForwardMessagePort2",FWDMSG_PORT2_DEFAULT_S, UNINIT},
    {"ForwardMessageMask2", FWDMSG_MASK2_DEFAULT_S, UNINIT},
    {"ForwardMessageEnable3", FWDMSG_ENABLE3_DEFAULT_S, UNINIT},
    {"ForwardMessageIP3", FWDMSG_IP3_DEFAULT_S, UNINIT},
    {"ForwardMessagePort3",FWDMSG_PORT3_DEFAULT_S, UNINIT},
    {"ForwardMessageMask3", FWDMSG_MASK3_DEFAULT_S, UNINIT},
    {"ForwardMessageEnable4", FWDMSG_ENABLE4_DEFAULT_S, UNINIT},
    {"ForwardMessageIP4", FWDMSG_IP4_DEFAULT_S, UNINIT},
    {"ForwardMessagePort4",FWDMSG_PORT4_DEFAULT_S, UNINIT},
    {"ForwardMessageMask4", FWDMSG_MASK4_DEFAULT_S, UNINIT},
    {"ForwardMessageEnable5", FWDMSG_ENABLE5_DEFAULT_S, UNINIT},
    {"ForwardMessageIP5", FWDMSG_IP5_DEFAULT_S, UNINIT},
    {"ForwardMessagePort5",FWDMSG_PORT5_DEFAULT_S, UNINIT},
    {"ForwardMessageMask5", FWDMSG_MASK5_DEFAULT_S, UNINIT},
};


/*
 * ===============================  
 * ============ SNMP =============
 * ===============================
 */

STATIC conf_element_t SNMP_Table[] = {
    {"agentaddress", SNMP_AGENT_ADDRESS_DEFAULT_S, UNINIT},
};


/*
 * ===============================  
 * ========== NSCONFIG ===========
 * ===============================
 */

STATIC conf_element_t NSCONFIG_Table[] = {
    {"AC_BE_CCH_RADIO_0", AC_BE_CCH_RADIO_0_DEFAULT_S, UNINIT},
    {"AC_BK_CCH_RADIO_0", AC_BK_CCH_RADIO_0_DEFAULT_S, UNINIT},
    {"AC_VO_CCH_RADIO_0", AC_VO_CCH_RADIO_0_DEFAULT_S, UNINIT},
    {"AC_VI_CCH_RADIO_0", AC_VI_CCH_RADIO_0_DEFAULT_S, UNINIT},
    {"AC_BE_SCH_RADIO_0", AC_BE_SCH_RADIO_0_DEFAULT_S, UNINIT},
    {"AC_BK_SCH_RADIO_0", AC_BK_SCH_RADIO_0_DEFAULT_S, UNINIT},
    {"AC_VO_SCH_RADIO_0", AC_VO_SCH_RADIO_0_DEFAULT_S, UNINIT},
    {"AC_VI_SCH_RADIO_0", AC_VI_SCH_RADIO_0_DEFAULT_S, UNINIT},
    {"AC_BE_CCH_RADIO_1", AC_BE_CCH_RADIO_1_DEFAULT_S, UNINIT},
    {"AC_BK_CCH_RADIO_1", AC_BK_CCH_RADIO_1_DEFAULT_S, UNINIT},
    {"AC_VO_CCH_RADIO_1", AC_VO_CCH_RADIO_1_DEFAULT_S, UNINIT},
    {"AC_VI_CCH_RADIO_1", AC_VI_CCH_RADIO_1_DEFAULT_S, UNINIT},
    {"AC_BE_SCH_RADIO_1", AC_BE_SCH_RADIO_1_DEFAULT_S, UNINIT},
    {"AC_BK_SCH_RADIO_1", AC_BK_SCH_RADIO_1_DEFAULT_S, UNINIT},
    {"AC_VO_SCH_RADIO_1", AC_VO_SCH_RADIO_1_DEFAULT_S, UNINIT},
    {"AC_VI_SCH_RADIO_1", AC_VI_SCH_RADIO_1_DEFAULT_S, UNINIT},
};

STATIC uint64_t my_error_states = 0x0;

void set_my_error_state(int32_t my_error)
{
  int32_t dummy = 0;

  dummy = abs(my_error);
  if((dummy < 64) && (0 < dummy)) {
      my_error_states |= (uint64_t)(0x1) << (dummy - 1);
  }
}
                                
/*
 * QNX build command line build
 *      # qcc -g -w9 -DPLATFORM_QNX -Vgcc_ntoarmv7le -DWSU_LITTLE_ENDIAN  -g conf_agent.c -o conf_agent
 *
 * Config files:
 *      - I2V
 *      - IPB
 *      - AMH
 *      - SCS
 *      - SPAT16
 *      - SRM_RX
 *      - SYSLOG => TODO
 *      - SNMP
 *      - NSCONFIG
 */

/*
 * extract_Token_n()
 * Extract a string given a separator "key"
 */
//TODO: his can fail if passed somethimg too big
STATIC int32_t extract_Token_n(char_t * source_str, char_t * Token, char_t  key, int32_t n)
{
  int32_t i = 0;
  int32_t n_cnt = 1;
  int32_t ret = CFG_AOK;
  int32_t length;

  if((NULL == source_str) || (NULL == Token) || (MAX_TOKENS < n)) {
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"extract_Token_n: CFG_PARAM_FAIL: NULL input or n too large: n=%d.\n",n);
      ret = CFG_PARAM_FAIL;
  } else {
      if( MAX_MSG_SIZE < (length = strlen(source_str))) {
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"extract_Token_n: CFG_PARAM_FAIL: msg too big:key=%c,n=%d,length=%d.\n",key,n,length);
          ret = CFG_PARAM_FAIL;
      }
  }
  //I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"extract_Token_n:key=%c,n=%d,source(%d)=(%s)\n",key,n,length,source_str);
  if(CFG_AOK == ret) {
      for(i=0,n_cnt=1; '\0' != *source_str; i++) {
          if (n_cnt == n) 
              break;
          if (*source_str == key) 
              n_cnt++; 
          source_str++; 
      }
      if(length == i) {
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"extract_Token_n: CFG_TOKEN_PARSE_KEY_FAIL.\n");
          ret = CFG_TOKEN_PARSE_KEY_FAIL;
      } else {
          /* Keep track of total source consumed. */
          for(i=0,*Token=0;(i<MAX_TOKEN_SIZE) && ('\0' != *source_str);i++) {
              if (*source_str == key) {
                  break; 
              } else {
                  Token[0] = *source_str;
                  Token[1] = 0;
                  Token++;
                  source_str++;
              }
          }
          if(MAX_TOKEN_SIZE == i) {
              I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"extract_Token_n: CFG_TOKEN_PARSE_VALUE_FAIL.\n");
              ret = CFG_TOKEN_PARSE_VALUE_FAIL;
          }          
      } 
  }
  if(CFG_AOK != ret){
      set_my_error_state(ret);
  }
  return ret;
}

/**** Utilities ****/

/*
 * get_element()
 * Search and return config item of a given config group
 * 
 */
conf_element_t* get_element(char_t  *conf_group, char_t  *conf_item)
{
    conf_element_t *pRet = NULL;
    conf_element_t *conf_table = NULL;
    int32_t item_num = 0;
    int32_t i;

    /* Error check */
    if ( (NULL == conf_group) || (NULL == conf_item) ){
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"get_element: CFG_PARAM_FAIL.\n");
        set_my_error_state(CFG_PARAM_FAIL);        
        return pRet;
    }

    /* get ptr of config conf_table */
    if (0 == (strcmp(RSUINFO_GRP, conf_group) )) {
        conf_table = &RSU_INFO_Table[0];
        item_num = sizeof(RSU_INFO_Table)/sizeof(conf_element_t);
    }

    if (0 == (strcmp(I2V_GRP, conf_group) )) {
        conf_table = &I2V_Table[0];
        item_num = sizeof(I2V_Table)/sizeof(conf_element_t);
    }

    if (0 == (strcmp(IPB_GRP, conf_group) )) {
        conf_table = &IPB_Table[0];
        item_num = sizeof(IPB_Table)/sizeof(conf_element_t);
    }
    
    if (0 == (strcmp(AMH_GRP, conf_group) )) {
        conf_table = &AMH_Table[0];
        item_num = sizeof(AMH_Table)/sizeof(conf_element_t);
    }
    
    if (0 == (strcmp(SCS_GRP, conf_group) )) {
        conf_table = &SCS_Table[0];
        item_num = sizeof(SCS_Table)/sizeof(conf_element_t);
    }
    
    if (0 == (strcmp(SPAT_GRP, conf_group) )) {
        conf_table = &SPAT16_Table[0];
        item_num = sizeof(SPAT16_Table)/sizeof(conf_element_t);
    }
    
    if (0 == (strcmp(SRMRX_GRP, conf_group) )) {
        conf_table = &SRM_RX_Table[0];
        item_num = sizeof(SRM_RX_Table)/sizeof(conf_element_t);
    }

    if (0 == (strcmp(FWDMSG_GRP, conf_group) )) {
        conf_table = &FWDMSG_Table[0];
        item_num = sizeof(FWDMSG_Table)/sizeof(conf_element_t);
    }

    if (0 == (strcmp(SNMPD_GRP, conf_group) )) {
        conf_table = &SNMP_Table[0];
        item_num = sizeof(SNMP_Table)/sizeof(conf_element_t);
    }
    
    if (0 == (strcmp(NSCONFIG_GRP, conf_group) )) {
        conf_table = &NSCONFIG_Table[0];
        item_num = sizeof(NSCONFIG_Table)/sizeof(conf_element_t);
    }
    
    /* Error out if no group found */
    if (!conf_table){
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"get_element: CFG_GROUP_NOT_FOUND: (%s)\n", conf_group);
        set_my_error_state(CFG_GROUP_NOT_FOUND);
    } else {
        /* Search for conf item */
        for (i=0; i < item_num; i++,conf_table++){
            if (0 == (strcmp(conf_item, conf_table->name) )) {
                /* Found item - copy content and return */
                pRet = (conf_element_t*)conf_table;
                break;
            }
        }
        if(item_num == i) {
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"get_element: CFG_ITEM_NOT_FOUND: (%s:%s)\n", conf_group,conf_item);
            set_my_error_state(CFG_ITEM_NOT_FOUND);
        }
    }
    return pRet;
}

/*
 * update_cache()
 * Update local cache for fast access to config items
 * 
 */
//TODO: perhaps we should call this periodically rather than just once?
//    : Doesn't work for irregulars cause they are not in confs and not intergrated with cfgmgrapi.
void update_cache(char_t  *conf_group,  conf_element_t *conf_table, int32_t ecount)
{
    int32_t i;
    char_t  cfg_result[MAX_TOKEN_SIZE];
    int32_t ret;

    for (i=0; i<ecount; i++){
        memset(cfg_result, 0, sizeof(cfg_result)); /* Clear prior result */
        /* Update only of flagged for valid */
        if (NONE != conf_table->state) {
            if (conf_table->state & ITEM_VALID) {
                if (0 == (ret = cfgGetParam(conf_table->name, conf_group, cfg_result))) {
                    memcpy(conf_table->val, cfg_result, MAX_TOKEN_SIZE);
                    conf_table->state &= ~(UNINIT);
                    conf_table->state |= INIT;
                } else {
                    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"update_cache: Element %d Failed: ret(%d) name(%s) value(%d)(%s)\n", 
                        i, ret, conf_table->name, strlen(cfg_result),cfg_result);
                    conf_table->state &= ~(UNINIT);
                    conf_table->state |= BAD_VAL;
                    set_my_error_state(ret);
                }
            } else {
                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"update_cache: Element %d : %s not Flagged to be updated!\n", i, conf_table->name);
                set_my_error_state(CFG_ELEMENT_NOT_FLAGGED);
            }
        }
        conf_table++;
    }
}

/*
 * get_rsu_ip()
 * Return RSU IP address
 * 
 */
bool_t get_rsu_ip(char_t  *ip_addr)
{
  bool_t wbRet = WFALSE;

  if(NULL != ip_addr){
      /* 0 = RSU, 1 = OBU. eth0 vs sc0. */
      if (0 == (cfgNetworkGetIP(ip_addr, 0) ) ) {
          wbRet = WTRUE;
      }
  } else {
      set_my_error_state(CFG_PARAM_FAIL);
  }
  return wbRet;
}

/*
 * get_startup_script()
 * returns  i2v,rsuhealth, and systemmonitor enable. 
 */
bool_t get_startup_script(char_t  *item, char_t  *result)
{
    bool_t wbRet = WTRUE;
    FILE *fp;
    char_t  cmd[200],var[100],varvar[32]; /* Set size accordingly. */
    int32_t i = 0;

    memset(cmd,0,sizeof(cmd));
    memset(var,0,sizeof(var));
    memset(varvar,0,sizeof(varvar));

    /* Be careful grep returns only the line you want and not dupe comment lines. */
    if (0 == strcmp(item, "I2V_APP_ENABLE")){
        sprintf(cmd, "grep \"i2v\" /rwflash/configs/startup.sh");
        sprintf(varvar,"i2v_app");
    } else if (0 == strcmp(item, "RSU_HEALTH_ENABLE")){
        sprintf(cmd, "grep \"/tmp/rsuhealth\" /rwflash/configs/startup.sh");
        sprintf(varvar,"/tmp/rsuhealth");
#if 0 /* Does not exist. */
    } else if (0 == strcmp(item, "RSU_SYSTEM_MONITOR_ENABLE")){
        sprintf(cmd, "grep \"/tmp/systemmonitor\" /rwflash/configs/startup.sh");
        sprintf(varvar,"/tmp/systemmonitor");
#endif
    } else {
        wbRet = WFALSE; /* Could be NULL or anything so dont print item. */
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"get_startup_script: Invalid item, fail.\n");
        set_my_error_state(CFG_PARAM_FAIL);
    }
    if(WTRUE == wbRet) {
        wbRet = WFALSE;
        if(NULL != (fp = popen(cmd,"r"))){
            for(i=0; ((i < I2V_STARTUP_LINE_SEARCH_MAX) && (fgets(var, sizeof(var),fp) != NULL)); i++) {
                if (strstr(var,varvar))   {
                    if (strstr(var,"#")) {
                        sprintf(result,"0");
                    } else {
                        sprintf(result,"1");
                    }
                    wbRet = WTRUE;
                    break;
                } 
            }
            pclose(fp);
            if(WFALSE == wbRet) {
                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"get_startup_script: Search for item(%s) failed.\n");
                set_my_error_state(CFG_SEARCH_STARTUP_FAIL);
            }
        } else {
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"get_startup_script: popen() fail.\n");
            set_my_error_state(CFG_FOPEN_FAIL);
        }
    }
    return wbRet;
}

void flag_item(char_t  *conf_group, char_t  *conf_item)
{
    conf_element_t *c_element;
    
    if ((c_element = get_element(conf_group, conf_item))){
        c_element->state |= ITEM_VALID;
    }
}

/*
 * set_default_custom()
 * Flag items as valid
 * 
 */
void set_default_custom(void)
{
  int32_t item_num = 0;
  int32_t i = 0;

  /* AMH */
  item_num = sizeof(AMH_Table)/sizeof(conf_element_t);
  for(i=0;i<item_num;i++){
      flag_item(AMH_GRP,AMH_Table[i].name);
  }

  /* I2V */
  item_num = sizeof(I2V_Table)/sizeof(conf_element_t);
  for(i=0;i<item_num;i++){
      flag_item(I2V_GRP,I2V_Table[i].name);
  }

  /* IPB */
  item_num = sizeof(IPB_Table)/sizeof(conf_element_t);
  for(i=0;i<item_num;i++){
      flag_item(IPB_GRP,IPB_Table[i].name);
  }

  /* SCS */
  item_num = sizeof(SCS_Table)/sizeof(conf_element_t);
  for(i=0;i<item_num;i++){
      flag_item(SCS_GRP,SCS_Table[i].name);
  }

  /* SPAT16 */
  item_num = sizeof(SPAT16_Table)/sizeof(conf_element_t);
  for(i=0;i<item_num;i++){
      flag_item(SPAT_GRP,SPAT16_Table[i].name);
  }

  /* SRM_RX */
  item_num = sizeof(SRM_RX_Table)/sizeof(conf_element_t);
  for(i=0;i<item_num;i++){
      flag_item(SRMRX_GRP,SRM_RX_Table[i].name);
  }

  /* FWDMSG */
  item_num = sizeof(FWDMSG_Table)/sizeof(conf_element_t);
  for(i=0;i<item_num;i++){
      flag_item(FWDMSG_GRP,FWDMSG_Table[i].name);
  }

  /* SNMP */
  item_num = sizeof(SNMP_Table)/sizeof(conf_element_t);
  for(i=0;i<item_num;i++){
      flag_item(SNMPD_GRP,SNMP_Table[i].name);
  }

  /* NSCONFIG */
  item_num = sizeof(NSCONFIG_Table)/sizeof(conf_element_t);
  for(i=0;i<item_num;i++){
      flag_item(NSCONFIG_GRP,NSCONFIG_Table[i].name);
  }

  /* RSUINFO */
  item_num = sizeof(RSU_INFO_Table)/sizeof(conf_element_t);
  for(i=0;i<item_num;i++){
      flag_item(RSUINFO_GRP,RSU_INFO_Table[i].name);
  }

//TODO: Need SYSLOG
}
/*
 * process_update_all_request()
 */
void process_update_all_request(void)
{
    /* Reload items with valid flags cache */
    /*
     * === Read and populate cache 
     */
    update_cache(I2V_GRP,  &I2V_Table[0], sizeof(I2V_Table)/sizeof(conf_element_t));
    update_cache(IPB_GRP,  &IPB_Table[0], sizeof(IPB_Table)/sizeof(conf_element_t));
    update_cache(AMH_GRP,  &AMH_Table[0], sizeof(AMH_Table)/sizeof(conf_element_t));
    update_cache(SCS_GRP,  &SCS_Table[0], sizeof(SCS_Table)/sizeof(conf_element_t));
    update_cache(SPAT_GRP,  &SPAT16_Table[0], sizeof(SPAT16_Table)/sizeof(conf_element_t));
    update_cache(SRMRX_GRP,  &SRM_RX_Table[0], sizeof(SRM_RX_Table)/sizeof(conf_element_t));
    update_cache(FWDMSG_GRP,  &FWDMSG_Table[0], sizeof(FWDMSG_Table)/sizeof(conf_element_t));
    update_cache(SNMPD_GRP,  &SNMP_Table[0], sizeof(SNMP_Table)/sizeof(conf_element_t));
    update_cache(NSCONFIG_GRP,  &NSCONFIG_Table[0], sizeof(NSCONFIG_Table)/sizeof(conf_element_t));
//TODO:This won't work for irregulars. Not properly integrated into cfgmgrapi. Maybe one day.
#if 0
    update_cache(RSUINFO_GRP,  &RSU_INFO_Table[0], sizeof(RSU_INFO_Table)/sizeof(conf_element_t));
#endif
}
/*
 * conf_init()
 * Initialize config manager
 * 
 */
void conf_init(void)
{
    conf_element_t *c_element;
    
    /*
     * === Read values and populate cache 
     */

    /*
     * Start of irregulars. Not part of cfgmgrapi yet. From startup.sh etc
     */
    /* SC0IPADDRESS */
    if ((c_element = get_element("RSU_INFO", "SC0IPADDRESS"))){
        c_element->state &= ~(UNINIT);
        if(WTRUE == get_rsu_ip((char_t *)&(c_element->val))){
            c_element->state |= INIT;
        } else {
            set_my_error_state(CFG_GET_IP_FAIL);
            c_element->state |= BAD_VAL;
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"conf_init: CFG_GET_IP_FAIL.\n");
        }
    }
    /* Netmask */
    if ((c_element = get_element("RSU_INFO", "NETMASK"))){
        int32_t rsu_netmask = cfgNetworkGetMask(RSU_DEVICE);
        c_element->state &= ~(UNINIT);
        if(CFG_GET_NETMASK_FAIL != rsu_netmask) {
            c_element->state |= INIT;
        } else {
            set_my_error_state(CFG_GET_NETMASK_FAIL);
            c_element->state |= BAD_VAL;
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"conf_init: CFG_GET_MASK_FAIL.\n");
        }
        sprintf(c_element->val, "%d", rsu_netmask);
    }
    /* MTU */
    if ((c_element = get_element("RSU_INFO", "MTU"))){
        int32_t rsu_mtu = cfgNetworkGetMtu();
        c_element->state &= ~(UNINIT);
        if(CFG_GET_MTU_FAIL != rsu_mtu) {
            c_element->state |= INIT;
        } else {
            //set_my_error_state(CFG_GET_MTU_FAIL);
            c_element->state |= BAD_VAL;
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"conf_init: CFG_GET_MTU_FAIL.\n");
        }
        sprintf(c_element->val, "%d", rsu_mtu);
    }
    /* VLAN_ENABLE */
    if ((c_element = get_element("RSU_INFO", "VLAN_ENABLE"))){
        int32_t rsu_vlanEnable = cfgNetworkGetVLANEnable();
        c_element->state &= ~(UNINIT);
        if(CFG_GET_VLAN_ENABLE_FAIL != rsu_vlanEnable) {
            c_element->state |= INIT;
        } else {
            //set_my_error_state(CFG_GET_VLAN_ENABLE_FAIL);
            c_element->state |= BAD_VAL;
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"conf_init: CFG_GET_VLAN_ENABLE_FAIL.\n");
        }
        sprintf(c_element->val, "%d", rsu_vlanEnable);
    }
    /* VLAN_IP */
    if ((c_element = get_element("RSU_INFO", "VLAN_IP"))){
        c_element->state &= ~(UNINIT);
        /* return of next function is 0 if passing */
        if(!cfgNetworkGetVLANIP((char_t *)&(c_element->val))){
            c_element->state |= INIT;
        } else {
            //set_my_error_state(CFG_GET_VLAN_IP_FAIL);
            c_element->state |= BAD_VAL;
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"conf_init: CFG_GET_VLAN_IP_FAIL.\n");
        }
    }
    /* VLAN_MASK */
    if ((c_element = get_element("RSU_INFO", "VLAN_MASK"))){
        int32_t rsu_vlanMask = cfgNetworkGetVLANMask();
        c_element->state &= ~(UNINIT);
        if(CFG_GET_VLAN_MASK_FAIL != rsu_vlanMask) {
            c_element->state |= INIT;
        } else {
            //set_my_error_state(CFG_GET_VLAN_MASK_FAIL);
            c_element->state |= BAD_VAL;
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"conf_init: CFG_GET_VLAN_MASK_FAIL.\n");
        }
        sprintf(c_element->val, "%d", rsu_vlanMask);
    }
    /* VLAN_ID */
    if ((c_element = get_element("RSU_INFO", "VLAN_ID"))){
        int32_t rsu_vlanID = cfgNetworkGetVLANID();
        c_element->state &= ~(UNINIT);
        if(CFG_GET_VLAN_ID_FAIL != rsu_vlanID) {
            c_element->state |= INIT;
        } else {
           // set_my_error_state(CFG_GET_VLAN_ID_FAIL);
            c_element->state |= BAD_VAL;
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"conf_init: CFG_GET_VLAN_ID_FAIL.\n");
        }
        sprintf(c_element->val, "%d", rsu_vlanID);
    }
    /* Http and https ports */
    if ((c_element = get_element("RSU_INFO", "WEBGUI_HTTP_SERVICE_PORT"))) {
        int32_t tmpint = cfgNetworkGetHttpPort();
        c_element->state &= ~(UNINIT);
        if(CFG_GET_HTTP_PORT_FAIL != tmpint) {
            c_element->state |= INIT;
        } else {
            set_my_error_state(CFG_GET_HTTP_PORT_FAIL);
            c_element->state |= BAD_VAL;
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"conf_init: CFG_GET_HTTP_PORT_FAIL.\n");
        }
        sprintf(c_element->val, "%d", tmpint);
    }
    if ((c_element = get_element("RSU_INFO", "WEBGUI_HTTPS_SERVICE_PORT"))) {
        int32_t tmpint = cfgNetworkGetHttpsPort();
        c_element->state &= ~(UNINIT);
        if(CFG_GET_HTTPS_PORT_FAIL != tmpint) {
            c_element->state |= INIT;
        } else {
            set_my_error_state(CFG_GET_HTTPS_PORT_FAIL);
            c_element->state |= BAD_VAL;
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"conf_init: CFG_GET_HTTPS_PORT_FAIL.\n");
        }
        sprintf(c_element->val, "%d", tmpint);
    }
    /* I2V, RSU_HEALTH, SYS_MONITOR. */
    if ((c_element = get_element("RSU_INFO", "I2V_APP_ENABLE"))){
        c_element->state &= ~(UNINIT);
        if(WTRUE == get_startup_script((char_t *)c_element->name,(char_t *)&(c_element->val))){
            c_element->state |= INIT;
        } else {
            set_my_error_state(CFG_GET_I2V_ENABLE_FAIL);
            c_element->state |= BAD_VAL;
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"conf_init: Get (%s) failed.\n",c_element->name);
        }
    }
#if 0
    if (c_element = get_element("RSU_INFO", "RSU_SYSTEM_MONITOR_ENABLE")) {
        c_element->state &= ~(UNINIT);
        if(WTRUE == get_startup_script((char_t *)c_element->name,(char_t *)&(c_element->val))){
            c_element->state |= INIT;
        } else {
            set_my_error_state(CFG_GET_MONITOR_ENABLE_FAIL);
            c_element->state |= BAD_VAL;
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"conf_init: Get (%s) failed.\n",c_element->name);
        }
   }
#endif
   if ((c_element = get_element("RSU_INFO", "RSU_HEALTH_ENABLE"))){
        c_element->state &= ~(UNINIT);
        if(WTRUE == get_startup_script((char_t *)c_element->name,(char_t *)&(c_element->val))){
            c_element->state |= INIT;
        } else {
            set_my_error_state(CFG_GET_HEALTH_ENABLE_FAIL);
            c_element->state |= BAD_VAL;
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"conf_init: Get (%s) failed.\n",c_element->name);
        }
    }
    /* Get network mode: DHCP/Static */
    if ((c_element = get_element("RSU_INFO", "NETWORK_MODE"))){
        int32_t tmpint = cfgNetworkGetMode(RSU_DEVICE);
        c_element->state &= ~(UNINIT);
        if((I2V_RSU_NETWORK_MODE_MIN <= tmpint) && (tmpint <= I2V_RSU_NETWORK_MODE_MAX)) {
            c_element->state |= INIT;
        } else {
            set_my_error_state(CFG_GET_NETWORK_MODE_FAIL);
            c_element->state |= BAD_VAL;
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"conf_init: CFG_GET_NETWORK_MODE_FAIL.\n");
        }
        sprintf(c_element->val, "%d", tmpint);
    }
    /*
     * End of irregulars.
     */
    
    /* Set custom items */
    set_default_custom();

    process_update_all_request();
}


/* 
 * exit_handler()
 * This function gets called when a signal is caught by the application, such as Ctrl-C 
 */
void exit_handler(void)
{
    loop = WFALSE;
    return;
}

/* 
 * clean_up() 
 */
void clean_up(void)
{
}

/*
 * ==========================================================================================
 * MQ Stuff
 * ===========================================================================================
 */

/*
 * confSigHandler()
 *
 * Signal handler when registered signals caught
 *
 */
STATIC void configSigHandler(int32_t __attribute__((unused)) sig)
{
    mainloop = WFALSE;
}

/*
 * process_read_request()
 */
void process_read_request(char_t* in_msg, char_t* ret_msg)
{
  char_t conf_group[MAX_TOKEN_SIZE] = {0};
  char_t conf_item[MAX_TOKEN_SIZE]  = {0};
  conf_element_t *c_element;
    
  if((NULL != in_msg) && (NULL != ret_msg)) {
      /* clear mem */
      memset(conf_group, 0, sizeof(conf_group));
      memset(conf_item, 0, sizeof(conf_item));

      /* Assume failure till proven success. */
      ret_msg[0] = '-';
      ret_msg[1] = '1';
      ret_msg[2] = '\0';

      /* Get config group & item */
      if(    (CFG_AOK == extract_Token_n(in_msg, conf_group, '|' , 2))
          && (CFG_AOK == extract_Token_n(in_msg, conf_item, '|', 3))) {    
          /* Get pointer to config element */
          if ((c_element = get_element((char_t *)&conf_group,(char_t *) &conf_item))){
              /*=== Handling for normal config items ===*/
              if (c_element->state & (ITEM_VALID|INIT)) {
                  memcpy(ret_msg, c_element->val, sizeof(c_element->val));
              } else {
                  I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"CFG_READ_ITEM_NOT_VALID: %s : %s Not Valid!, return %s \n", conf_group, conf_item, ret_msg);
                  set_my_error_state(CFG_READ_ITEM_NOT_VALID);
              }
          } else {
              I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"CFG_READ_ITEM_NOT_FOUND: %s; Item: %s\n", conf_group, conf_item);
              set_my_error_state(CFG_READ_ITEM_NOT_FOUND);
          }
      } else {
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"process_read_request: extract token fail.\n");
      }
  } else {
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"process_read_request: CFG_PARAM_FAIL.\n");
      set_my_error_state(CFG_PARAM_FAIL);
  } 
}


/*
 * process_update_request(): Not implemented fot this version.
 */
void process_update_request(char_t* in_msg, char_t* ret_msg)
{
    in_msg = in_msg;
    ret_msg = ret_msg;    
}

/*
 * process_write_request()
 * @in_msg can not be NULL.
 * @ret_msg can be NULL if you dont want a return value.
 * If write fails cache remains unchanged.
 * TODO: If someone changes confs outside of cfgmgr then cache is stale :-S
 */
void process_write_request(char_t* in_msg, char_t* ret_msg)
{
  int32_t rc = CFG_AOK;
  char_t conf_group[MAX_TOKEN_SIZE];
  char_t conf_item[MAX_TOKEN_SIZE];
  char_t conf_value[MAX_TOKEN_SIZE];
  conf_element_t *c_element = NULL;

  memset(conf_group, 0, sizeof(conf_group));
  memset(conf_item, 0, sizeof(conf_item));
  memset(conf_value, 0, sizeof(conf_value));

  if(NULL == in_msg) {
      rc = CFG_PARAM_FAIL;
  } else {
      /* Get config group and item */
      if(CFG_AOK != (rc = extract_Token_n(in_msg, conf_group, '|', 2))) {
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"process_write_request: extract group failed.\n");
      } else {
          if(CFG_AOK != (rc = extract_Token_n(in_msg, conf_item, '|', 3))) {
              I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"process_write_request:conf group (%s): extract item failed.\n",conf_group);
          } else {
                if(CFG_AOK != (rc = extract_Token_n(in_msg, conf_value, '|', 4))){
                    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"process_write_request: group(%s) item(%s): extract value failed.\n",conf_group,conf_item);
                }
          }
      }
  }
  if(CFG_AOK == rc) {
      if(CFG_AOK != (rc = cfgUnlock())){
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"process_write_request: cfgUnlock failed.\n");
      }
  }
  if(CFG_AOK == rc) {
      /* SPECIAL MY_MAGIC item TSCBM_CONF - handled before get_element since it has no element! */
      if (0 == strcmp(conf_item, "TSCBM_CONF")) { /* Not cached. */
          if(CFG_AOK != (rc = cfgReplaceTscbm(conf_value))){
              I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"process_write_request: cfgReplaceTscbm failed.\n");
          }
      } else {
          /* RSUINFO irregular: handle a bit different with DCU options. Needs seperate handling for now.
           * Maybe one day these will be properly integrated into cfgmgrapi.
           */
          if ((c_element = get_element((char_t *)&conf_group, (char_t *)&conf_item))){ /* Get pointer to config element */
              if (0 == strcmp(conf_group, "RSU_INFO")){
                  if (0 == strcmp(conf_item, "NETWORK_MODE")){ /* NETWORK_MODE */
#if defined(ENABLED_CACHED_WRITES)
                      if (0 != strcmp(c_element->val, conf_value))
#endif
                      { /* Check cache for changes - update if changed */
                          if (!strcmp(conf_value, "1")) {  /* Set NetMode via enabling DHCP or enabling Static */
                              if(CFG_AOK != (rc = cfgNetworkSetModeDHCPEnabled(0))){
                                  I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"process_write_request: Set ModeDHCPEnabled failed.\n");
                              }
                          } else {
                              if(CFG_AOK != (rc = cfgNetworkSetModeStaticEnabled(NULL, 0))){  // NULL=enable only;dont change static IP
                                  I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"process_write_request: Set ModeStaticEnabled failed.\n");
                              }
                          }
                      }
                  } else if (0 == strcmp(conf_item, "SC0IPADDRESS")){  /* RSU network */
#if defined(ENABLED_CACHED_WRITES)
                      if (0 != strcmp(c_element->val, conf_value))
#endif
                      {  /* Check cache for changes - update if changed */
                          if(CFG_AOK != (rc = cfgNetworkSetStaticIP(conf_value, 0))){
                              I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"process_write_request: SC0IPADDRESS failed.\n");
                          }
                      }
                  } else if (0 == strcmp(conf_item, "NETMASK")){
#if defined(ENABLED_CACHED_WRITES)
                      if (0 != strcmp(c_element->val, conf_value))
#endif
                      { /* Check cache for changes - update if changed */
                          if(CFG_AOK != (rc = cfgNetworkSetMask(atoi(conf_value), 0))){
                              I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"process_write_request: Set NetworkSetMask failed.\n");
                          }
                      }
                  } else if (0 == strcmp(conf_item, "MTU")){
#if defined(ENABLED_CACHED_WRITES)
                      if (0 != strcmp(c_element->val, conf_value))
#endif
                      { /* Check cache for changes - update if changed */
                          if(CFG_AOK != (rc = cfgNetworkSetMtu(atoi(conf_value)))){
                              I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"process_write_request: Set MTU failed.\n");
                          }
                      }
                  } else if (0 == strcmp(conf_item, "VLAN_ENABLE")){
#if defined(ENABLED_CACHED_WRITES)
                      if (0 != strcmp(c_element->val, conf_value))
#endif
                      { /* Check cache for changes - update if changed */
                          if(CFG_AOK != (rc = cfgNetworkSetVLANEnable(atoi(conf_value)))){
                              I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"process_write_request: Set VLAN_ENABLE failed.\n");
                          }
                      }
                  } else if (0 == strcmp(conf_item, "VLAN_IP")){
#if defined(ENABLED_CACHED_WRITES)
                      if (0 != strcmp(c_element->val, conf_value))
#endif
                      { /* Check cache for changes - update if changed */
                          if(CFG_AOK != (rc = cfgNetworkSetVLANIP(conf_value))){
                              I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"process_write_request: Set VLAN_IP failed.\n");
                          }
                      }
                  } else if (0 == strcmp(conf_item, "VLAN_MASK")){
#if defined(ENABLED_CACHED_WRITES)
                      if (0 != strcmp(c_element->val, conf_value))
#endif
                      { /* Check cache for changes - update if changed */
                          if(CFG_AOK != (rc = cfgNetworkSetVLANMask(atoi(conf_value)))){
                              I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"process_write_request: Set VLAN_MASK failed.\n");
                          }
                      }
                  } else if (0 == strcmp(conf_item, "VLAN_ID")){
#if defined(ENABLED_CACHED_WRITES)
                      if (0 != strcmp(c_element->val, conf_value))
#endif
                      { /* Check cache for changes - update if changed */
                          if(CFG_AOK != (rc = cfgNetworkSetVLANID(atoi(conf_value)))){
                              I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"process_write_request: Set VLAN_ID failed.\n");
                          }
                      }
                  } else if (0 == strcmp(conf_item, "I2V_APP_ENABLE")){
#if defined(ENABLED_CACHED_WRITES)
                      if (0 != strcmp(c_element->val, conf_value))
#endif
                      { /* Check the cache for changes */
                          if(CFG_AOK != (rc = cfgStartupSet(atoi(conf_value),conf_item))) {
                              I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"process_write_request: I2V_APP_ENABLE failed.\n");
                          }
                      }
                  } else if (0 == strcmp(conf_item, "RSU_HEALTH_ENABLE")){
#if defined(ENABLED_CACHED_WRITES)
                      if (0 != strcmp(c_element->val, conf_value)) /* Check the cache for changes */
#endif
                      { /* Check the cache for changes */
                          if(CFG_AOK != (rc = cfgStartupSet(atoi(conf_value),conf_item))){
                              I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"process_write_request: RSU_HEALTH_ENABLE failed.\n");
                          }
                      }
#if 0
                  } else if (0 == strcmp(conf_item, "RSU_SYSTEM_MONITOR_ENABLE")){
#if defined(ENABLED_CACHED_WRITES)
                      if (0 != strcmp(c_element->val, conf_value)) /* Check the cache for changes */
#endif
                      {
                          if(CFG_AOK != (rc = cfgStartupSet(atoi(conf_value),conf_item))){
                              I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"process_write_request: RSU_SYSTEM_MONITOR_ENABLE failed.\n");
                          }
                      }
#endif
                  } else if (!strcmp(conf_item, "WEBGUI_HTTP_SERVICE_PORT")) {
#if defined(ENABLED_CACHED_WRITES)
                      if (0 != strcmp(c_element->val, conf_value)) /* Check the cache for changes */
#endif
                      {
                          if(CFG_AOK != (rc = cfgNetworkSetHttpPort(atoi(conf_value))))  {
                              I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"process_write_request: WEBGUI_HTTP_SERVICE_PORT failed.\n");
                          }
                      }
                  } else if (!strcmp(conf_item, "WEBGUI_HTTPS_SERVICE_PORT")) {
#if defined(ENABLED_CACHED_WRITES)
                      if (0 != strcmp(c_element->val, conf_value)) /* Check the cache for changes */
#endif
                      {
                          if(CFG_AOK != (rc = cfgNetworkSetHttpsPort(atoi(conf_value)))) {
                              I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"process_write_request: WEBGUI_HTTPS_SERVICE_PORT failed.\n");
                          }
                      }
                  } else {
                      I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Unknown conf_item %s, ignoring request\n", conf_item);
                      rc = CFG_WRITE_ITEM_NOT_FOUND;
                  }
                  if (CFG_AOK != rc){
                      I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"cfgSetParam: REJECTED ret = %d: item: %s; group: %s; value: %s\n",
                                  rc, conf_item, conf_group, conf_value);
                      set_my_error_state(CFG_SET_VALUE_RANGE_ERROR);
                  } else {
                      if(NULL != ret_msg) {
                          memcpy(ret_msg, conf_value, sizeof(conf_value));
                      }
                  }
                  /* End of RSUINFO */ 
              } else {
                  /*=== Handling for normal config items ===*/
                  if (c_element->state & ITEM_VALID) {
#if defined(ENABLED_CACHED_WRITES)
                      if (0 != strcmp(c_element->val, conf_value)) /* Check cache for changes - update if changed */
#endif
                      {
                          if (CFG_AOK != (rc = cfgSetParam(conf_item, conf_group, conf_value))){
                              I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"cfgSetParam: REJECTED ret = %d: item: %s; group: %s; value: %s\n",
                                  rc, conf_item, conf_group, conf_value);
                              set_my_error_state(CFG_SET_VALUE_RANGE_ERROR);
                          } else {
                              if(NULL != ret_msg) {
                                  memcpy(ret_msg, conf_value, sizeof(conf_value));
                              }
                          }
                      }
#if defined(ENABLED_CACHED_WRITES)
                      else {
/* TODO: Need to return OK or 1 even if write is cached value. Otherwise how do we know if fail or success? */
                          if(NULL != ret_msg) {
                              memcpy(ret_msg, conf_value, sizeof(conf_value));
                          }
                      }
#endif
                  } else {
                      I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"conf_item: %s is not Valid! Skip write! \n", conf_item);
                      rc = CFG_WRITE_ITEM_NOT_VALID;
                  }
              }
          } else {  /* didnt find element */
              I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Failed to find element for group %s item %s, ignoring write request\n", conf_group, conf_item);
              rc = CFG_WRITE_ITEM_NOT_FOUND;
          }
          if(CFG_AOK == rc) { /* Update cache. */
              memcpy(c_element->val, conf_value, sizeof(c_element->val));
          }
      }
      if(CFG_AOK == rc) { /* commit(unlock?). */
          if(CFG_AOK != cfgCommit()){
              I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"process_write_request: cfgCommit failed.\n");
              set_my_error_state(CFG_SET_COMMIT_FAIL);
          } else {
              I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"cfgSetParam: ACCEPTED: ret=%d item: %s; group: %s; value: %s\n",
                  rc,conf_item, conf_group, conf_value);
          }
      } else {
          if(CFG_AOK != cfgRevert()){ /* revert(unlock?). */
              I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"process_write_request: cfgRevert failed.\n");
              set_my_error_state(CFG_SET_REVERT_FAIL);
          }
      }
  } /* End of lock */
  if(CFG_AOK != rc) {
      if (NULL != ret_msg){
          ret_msg[0] = '-';
          ret_msg[1] = '1';
          ret_msg[2] = '\0';
      }
  }
}

/*
 * process_login_request()
 */
void process_login_request(char_t* in_msg, char_t* ret_msg)
{
  char_t session_id[MAX_NAME_SIZE] = {0};

  /* 
   *  Login will preempt current session
   *      - Update current session ID
   *      - Reset timeout
   */
  if((NULL == in_msg) || (NULL == ret_msg)){
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"session_id: CFG_PARAM_FAIL.\n"); 
      set_my_error_state(CFG_PARAM_FAIL);
  } else {
      /* Get session ID */
      extract_Token_n(in_msg, session_id, '|' , 2);
      //DPRINT("Log in Session ID: %s\n", (char_t *)session_id);

      /* Update current session ID */
      memcpy(conf_session.user_id, session_id, sizeof(session_id));

      /* Reset time out */
      conf_session.duration = SESSION_TIMEOUT;  /* Set to 10 minutes */
      conf_session.log_state = LOG_ON;

      memcpy(ret_msg, conf_session.user_id, sizeof(conf_session.user_id));
  }
}

/*
 * process_logout_request()
 */
void process_logout_request(char_t* in_msg, char_t* ret_msg)
{
  char_t session_id[MAX_NAME_SIZE] = {0};

  if((NULL == in_msg) || (NULL == ret_msg)){
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"process_login_request: CFG_PARAM_FAIL.\n"); 
      set_my_error_state(CFG_PARAM_FAIL);
  } else {
      /* Get session ID */
      extract_Token_n(in_msg, session_id, '|' , 2);
      /* Update current session ID */
      memcpy(conf_session.user_id, "0.0.0.0", strlen("0.0.0.0"));

      /* Set state to log off */
      conf_session.log_state = LOG_OFF;

      memcpy(ret_msg, conf_session.user_id, sizeof(conf_session.user_id));
  }
}

/*
 * process_factory_reset_request()
 */
void process_factory_reset_request(char_t* in_msg, char_t* ret_msg)
{
  char_t confirm_str[100];

    ret_msg = ret_msg; /* Silence of the Warnings. */
    extract_Token_n(in_msg, confirm_str, '|' , 2);

    if (!strcmp(confirm_str,"Yes")) {
        cfgResetToDefault(0);       // 0==reset everything with no network preserving
    } else {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Incorrect confirmation, returning without action\n");
        set_my_error_state(CFG_FACTORY_REQUEST_FAIL);
    }
}

/*
 * process_factory_reset_but_presrv_net_request()
 */
void process_factory_reset_but_presrv_net_request(char_t* in_msg, char_t* ret_msg)
{
    char_t confirm_str[100];
    ret_msg = ret_msg; /* Silence of the Warnings. */
    extract_Token_n(in_msg, confirm_str, '|' , 2);

    if (!strcmp(confirm_str,"Yes")) {
        cfgResetToDefault(1);       // 1==preserve network
    } else {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Incorrect confirmation, returning without action\n");
        set_my_error_state(CFG_FACTORY_PRESERVE_REQUEST_FAIL);
    }
}

/*
 * process_customer_reset_request() 
 */
void process_customer_reset_request(char_t* in_msg, char_t* ret_msg)
{
  char_t confirm_str[100];

    ret_msg = ret_msg; /* Silence of the Warnings. */
    extract_Token_n(in_msg, confirm_str, '|' , 2);

    if (!strcmp(confirm_str,"Yes")) {
        cfgResetToCustomer();       
    } else {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Incorrect confirmation, returning without action\n");
        //set_my_error_state(CFG_CUSTOMER_RESET_FAIL);
    }
}

/*
 * process_customer_backup_request() 
 */
void process_customer_backup_request(char_t* in_msg, char_t* ret_msg)
{
  char_t confirm_str[100];

    ret_msg = ret_msg; /* Silence of the Warnings. */
    extract_Token_n(in_msg, confirm_str, '|' , 2);

    if (!strcmp(confirm_str,"Yes")) {
        customerBackup();       
    } else {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Incorrect confirmation, returning without action\n");
        //set_my_error_state(CFG_CUSTOMER_BACKUP_FAIL);
    }
}

/*
 * process_session_chk_request()
 */
void process_session_chk_request(char_t* in_msg, char_t* ret_msg)
{
  char_t session_id[MAX_NAME_SIZE] = {0};

  /*
   * Return current session ID
   * If timed out, returns 0.0.0.0
   * Check ID against current ID - if match reset timer
   */
  if((NULL == in_msg) || (NULL == ret_msg)){
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"process_login_request: CFG_PARAM_FAIL.\n"); 
      set_my_error_state(CFG_PARAM_FAIL);
  } else {
      /* Get session ID */
      extract_Token_n(in_msg, session_id, '|' , 2);
      if (0 == strcmp(conf_session.user_id, session_id)){
          conf_session.duration = SESSION_TIMEOUT;  /* Set to 10 minutes */
      } else {
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Session ID: %s Does not matched with Current Session ID: %s\n", (char_t *)session_id, (char_t *)conf_session.user_id);
          set_my_error_state(CFG_SESSION_CHECK_FAIL);
      }

      memcpy(ret_msg, conf_session.user_id, sizeof(conf_session.user_id));
  }
}

/*
 * process_request_msg()
 * Process request 
 */
void process_request_msg(char_t* in_msg, char_t* ret_msg)
{
    mq_hdr_t *mq_hdr;

    /* 
        *  Check for Message operation:
        *      - CONF_NONE,
        *      - CONF_READ,
        *      - CONF_WRITE,
        *      - CONF_LOGIN,
        *      - CONF_LOGOUT
        *      - CONF_SESSION_CHK,
        */
    /* header point to in packet */
    mq_hdr = (mq_hdr_t *)&mq_in_buffer[0];
    switch (mq_hdr->msg_attr.msg_op)
    {
        case CONF_READ:     // THIS IS A REQUEST TO READ AND RETURN A VALUE FROM CONF_MANAGER'S CACHE
            process_read_request(in_msg, ret_msg);
            /* Reset timeout timer */
            conf_session.duration = SESSION_TIMEOUT;  /* Set to 10 minutes */
            break;

        case CONF_WRITE:
            process_write_request(in_msg, ret_msg);
            /* Reset timeout timer */
            conf_session.duration = SESSION_TIMEOUT;  /* Set to 10 minutes */
            break;
//TODO: Not used?
        case CONF_UPDATE:       // THIS IS A REQUEST TO UPDATE CONF_MANAGER'S CACHE
            process_update_request(in_msg, ret_msg);
            break;
//TODO: Not used?
        case CONF_UPDATE_ALL:
            process_update_all_request();
            break;

        case CONF_LOGIN:
            process_login_request(in_msg, ret_msg);
            break;

        case CONF_LOGOUT:
            process_logout_request(in_msg, ret_msg);
            break;

        case CONF_SESSION_CHK:
            process_session_chk_request(in_msg, ret_msg);
            break;

        case FACTORY_RESET:
            process_factory_reset_request(in_msg, ret_msg);
            break;

        case FACTORY_RESET_BUT_PRESRV_NET:
            process_factory_reset_but_presrv_net_request(in_msg, ret_msg);
            break;
        
        case CUSTOMER_RESET:
            process_customer_reset_request(in_msg, ret_msg);
            break;
        
        case CUSTOMER_BACKUP:
            process_customer_backup_request(in_msg, ret_msg);
            break;

        case CONF_NONE:
            set_my_error_state(CFG_NONE_REQUEST);
            break;

        default:
            set_my_error_state(CFG_UNKNOWN_REQUEST);
            break;
    }
}

/*
 * process_info_msg()
 * Process info msg 
 */
void process_info_msg(char_t* in_msg, char_t* ret_msg)
{
  // Do nothing for now
  in_msg = in_msg; /* Silence of the Warnings. */
  ret_msg = ret_msg;
}

/*
 * process_response_msg()
 * Process response  
 */
void process_response_msg(char_t* in_msg, char_t* ret_msg)
{
  // Do nothing for now
  in_msg = in_msg; /* Silence of the Warnings. */
  ret_msg = ret_msg;
}

/*
 * config_mq_handler()
 *
 * MQ thread handler - listening to incoming messages
 * 
 * Note: LN - Barely any error checking here
 * TBD: error checking for 
 *          MAX lenght
 */
static void *config_mq_handler(void *ptr)
{
    mq_hdr_t *mq_hdr;
    mqd_t qd_client;
    int32_t msg_size = 0;
    char_t msg_to_client [MSG_BUFFER_SIZE] = {0};
    char_t client_queue_name [MAX_NAME_SIZE] = {0};
    char_t *client_msg = &mq_in_buffer[sizeof(mq_hdr_t)];
    char_t conf_cmd[MAX_MSG_SIZE] = {0};
    char_t   cfg_result[MAX_TOKEN_SIZE];

    // DPRINT(" Process ID: %d\n", getpid());
    ptr = ptr; /* Silence of the Warnings. */

    while (mainloop) {
        memset(mq_in_buffer, 0, sizeof(mq_in_buffer));
        memset(conf_cmd, 0 , MAX_MSG_SIZE);

        /* Pop a message from queue with highest priority */
        if ((msg_size = (mq_receive (qd_server, mq_in_buffer, MSG_BUFFER_SIZE, NULL)) ) == -1)  {
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"CFG_MQ_READ_FAIL.\n");
            set_my_error_state(CFG_MQ_READ_FAIL);
            break;
        }

        mq_hdr = (mq_hdr_t *)&mq_in_buffer[0];
        //DPRINT("Got an mq message with len %d; Process: %d ; Thread: %d\n", msg_size, mq_hdr->proc_id, mq_hdr->thrd_id);
        //DPRINT("Config cmd: %s\n", (char_t *)client_msg);

        /* clear response message */
        memset(cfg_result, 0, MAX_TOKEN_SIZE);             /* reset result msg */

        /* 
         *  Check for Message Types:
         *      - MSG_INFO,
         *      - MSG_REQUEST,
         *      - MSG_RESPONSE,
         */
        switch (mq_hdr->msg_attr.msg_type)
        {
            case MSG_INFORM:
                process_info_msg(client_msg, cfg_result);
                break;

            case MSG_REQUEST:
                process_request_msg(client_msg, cfg_result);
                break;

            case MSG_RESPONSE:
                process_response_msg(client_msg, cfg_result);
                break;

            default:
                set_my_error_state(CFG_MQ_MSG_UNKNOWN);
                break;
        }

//client_response:  Label not used

        /*
         * Response back to client
         */
        sprintf (client_queue_name, "/%d%d", mq_hdr->proc_id, mq_hdr->thrd_id);
        if ((qd_client = mq_open (client_queue_name, O_WRONLY)) == -1) {
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"CFG_MQ_RESP_OPEN_FAIL.\n");
            set_my_error_state(CFG_MQ_RESP_OPEN_FAIL);
        }
        //DPRINT("Responding back to client %s\n", client_queue_name);

        memset(msg_to_client, 0, sizeof(msg_to_client));
            
        mq_hdr = (mq_hdr_t *)&msg_to_client[0];
        mq_hdr->proc_id = getpid();
        mq_hdr->thrd_id = pthread_self();
        mq_hdr->msg_attr.msg_type = MSG_RESPONSE;
        sprintf((char_t *)&msg_to_client[sizeof(mq_hdr_t)], "%s", cfg_result);
        
        /* send message to client */
        if (mq_send (qd_client, msg_to_client, MSG_BUFFER_SIZE, 0) == -1) {
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"CFG_MQ_RESP_SEND_FAIL.\n");
            set_my_error_state(CFG_MQ_RESP_SEND_FAIL);
        }
        
        if (mq_close (qd_client) == -1) {
#if !defined(MY_UNIT_TEST)
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"CFG_MQ_RESP_CLOSE_FAIL.\n");
#endif
            set_my_error_state(CFG_MQ_RESP_CLOSE_FAIL);
        }
    }

    pthread_exit(0);

}


/*
 * timeout_handler()
 *
 * Timeout thread handler - Session log out when 
 * 
 */
static void *timeout_handler(void *ptr)
{  
    /* Default session settings */
    //conf_session.log_state = LOG_OFF;
    ptr = ptr; /* Silence of the Warnings. */
    conf_session.duration = SESSION_TIMEOUT;  /* Set to 10 minutes */

    while (mainloop) {
        /* Wake up every minute to check for timeout */
        sleep(60);
        
        if (conf_session.log_state == LOG_ON) {
            conf_session.duration -= 60;
            if (conf_session.duration <= 0){
                /* Reset session */
                conf_session.log_state = LOG_OFF;
                conf_session.duration = SESSION_TIMEOUT;
                memcpy(conf_session.user_id, "0.0.0.0", sizeof("0.0.0.0"));
                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"CFG_SESSION_TIMEOUT.\n");
                set_my_error_state(CFG_SESSION_TIMEOUT);
            } else {
                //DPRINT("Session timing out in %d seconds \n", conf_session.duration);
            }
        } else {
            //DPRINT0("Session state: LOG_OFF\n");
        }
    }

    pthread_exit(0);

}

STATIC void init_statics(void)
{
  conf_session.log_state = LOG_OFF;
  conf_session.duration = 30;
  //strncpy(conf_session.user_id,"0.0.0.0",MAX_NAME_SIZE);
  mainloop = WTRUE;
  loop = WTRUE;
  memset(&config_mq_thread,0x0,sizeof(config_mq_thread));
  memset(&timeout_thread,0x0, sizeof(timeout_thread));
  memset(mq_in_buffer,'\0', MSG_BUFFER_SIZE);
  my_error_states = 0x0;
}

/* 
 * Warn of errors in tables: Strings null terminated, be initialized and have no bad values found.
 * TODO: We could also re-check each value againt I2V_UTIL update functions here but seems like overkill.
 */
STATIC int32_t dump_faulty_conf_table(conf_element_t * conf_table, char_t * conf_name, uint32_t element_count)
{
  int32_t  ret = CFG_AOK;
  uint32_t i = 0;
  int32_t  name_length = 0;
  int32_t  val_length = 0;
  uint8_t  string_error = 0;

  if((NULL == conf_table) || (NULL == conf_name)) {
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"dump_conf_table: CFG_PARAM_FAIL.\n");
      ret = CFG_PARAM_FAIL;
  } else {
      if(256 < (element_count) || (0 == element_count)) {    
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"dump_%s.conf: ERROR: element count is bogus = %u.\n",conf_name,element_count);  
          ret = CFG_PARAM_FAIL;
      } else {
          for(i=0,string_error=0;i<element_count;i++,string_error=0) {
              /* This ensures 'name' is null terminated string. */
              name_length = strlen(conf_table[i].name);
              if((MAX_TOKEN_SIZE < name_length) || (0 == name_length)) {
                  I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"dump_%s.conf[%d] ERROR: name length illegal=%d.",conf_name,i,name_length);
                  string_error |= 0x1;
                  ret = CFG_CONF_TABLE_CHECK_FAIL;
              }
              /* This ensures 'val' is null terminated string. */
              val_length = strlen(conf_table[i].val);
              if((MAX_TOKEN_SIZE < name_length) || (0 == name_length)) {
                  I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"dump_%s.conf[%d] ERROR: value length illegal=%d.",conf_name,i,val_length);
                  string_error |= 0x2;
                  ret = CFG_CONF_TABLE_CHECK_FAIL;
              }
              /* Every element must be initialzed. */
              if((0 == string_error) && (0 == (INIT & conf_table[i].state))) {
                  I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"dump_%s.conf[%d]: ERROR: no init: name(%u)(%s) value(%u)(%s) state=(0x%x)\n",
                      conf_name,i, name_length, conf_table[i].name, val_length ,conf_table[i].val,conf_table[i].state);
                  ret = CFG_CONF_TABLE_CHECK_FAIL;
              }
              /* No bad values allowed in elements. */
              if((0 == string_error) && ((BAD_VAL & conf_table[i].state))) {
                  I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"dump_%s.conf[%d]: ERROR: bad value: name(%u)(%s) value(%u)(%s) state=(0x%x)\n",
                      conf_name,i, name_length, conf_table[i].name, val_length, conf_table[i].val,conf_table[i].state);
                  ret = CFG_CONF_TABLE_CHECK_FAIL;
              }
          }
      }
  }
  if(CFG_AOK != ret) {
      set_my_error_state(ret);
  }
  return ret;
}
STATIC void display_faulty_conf_tables(void)
{
  int32_t conf_fail = 0;

  if(CFG_AOK != dump_faulty_conf_table(I2V_Table, "i2v", sizeof(I2V_Table)/sizeof(conf_element_t))){
      conf_fail++;
  }
  if(CFG_AOK != dump_faulty_conf_table(IPB_Table, "ipb", sizeof(IPB_Table)/sizeof(conf_element_t))){
      conf_fail++;
  }
  if(CFG_AOK != dump_faulty_conf_table(AMH_Table, "amh", sizeof(AMH_Table)/sizeof(conf_element_t))){
      conf_fail++;
  }
  if(CFG_AOK != dump_faulty_conf_table(SCS_Table, "scs", sizeof(SCS_Table)/sizeof(conf_element_t))){
      conf_fail++;
  }
  if(CFG_AOK != dump_faulty_conf_table(SPAT16_Table, "spat16", sizeof(SPAT16_Table)/sizeof(conf_element_t))){
      conf_fail++;
  }
  if(CFG_AOK != dump_faulty_conf_table(SRM_RX_Table, "srmrx", sizeof(SRM_RX_Table)/sizeof(conf_element_t))){
      conf_fail++;
  }
  if(CFG_AOK != dump_faulty_conf_table(SNMP_Table, "snmpd", sizeof(SNMP_Table)/sizeof(conf_element_t))){
      conf_fail++;
  }
  if(CFG_AOK != dump_faulty_conf_table(NSCONFIG_Table, "nsconfig", sizeof(NSCONFIG_Table)/sizeof(conf_element_t))){
      conf_fail++;
  }
  if(CFG_AOK != dump_faulty_conf_table(RSU_INFO_Table, "rsuinfo", sizeof(RSU_INFO_Table)/sizeof(conf_element_t))){
      conf_fail++;
  }
  if(0 < conf_fail) {
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"check_faulty_conf_tables: %d fails.\n",conf_fail);
  }
}
/* 
 * cfgmgrapi test 
 * 
 * int32_t cfgGetParam(char_t *param, char_t *group, char_t *result);
 * int32_t cfgSetParam(char_t *param, char_t *group, char_t *value);
 * int32_t cfgCommit(void);
 * int32_t cfgRevert(void);
 * int32_t cfgUnlock(void);
 * int32_t cfgNetworkDHCPEnable(int32_t isOBU);
 * int32_t cfgNetworkMode(int32_t isOBU);
 * int32_t cfgNetworkStaticEnable(char_t *staticIP, int32_t isOBU);
 * int32_t cfgNetworkStaticIP(char_t *staticIP, int32_t isOBU);
 * 
 * 
 * Note: conf_manager will read all config setting at startup
 *      keeping a cache copy 
 *      all operation is done off the cache
 *      will only update when user confirm changes.
 * Note: Not using syslog on purpose. Maybe one day.
 * 
 */
int32_t MAIN(int32_t argc, char_t *argv[])
{
  int32_t ret = CFG_AOK;
  int32_t c_arg = 0;
  struct sigaction sa;
  uint32_t rolling_counter = 0;
  int32_t enable_debug = 0; /* off by default */

  init_statics();

  /* 
   * WARNING: Debug output to stdout will clobber output for web gui and it will fail.
   * Enable with caution.
   */
#if 0
  i2vUtilEnableDebug(MY_NAME);
#endif

  /* catch SIGINT/SIGTERM */
  memset(&sa, 0, sizeof(sa));
  sa.sa_handler = configSigHandler;
  sigaction(SIGINT, &sa, NULL);
  sigaction(SIGTERM, &sa, NULL);

  /* Register signal handlers to catch Ctrl-C and other termination conditions */
  signal( SIGINT,  (__sighandler_t) exit_handler);
  signal( SIGTERM, (__sighandler_t) exit_handler);
  signal( SIGKILL, (__sighandler_t) exit_handler);

  /* Set up Config Manager message queue interface. */
  attr.mq_flags = 0;
  attr.mq_maxmsg = MAX_MESSAGES;
  attr.mq_msgsize = MSG_BUFFER_SIZE;
  attr.mq_curmsgs = 0;

  /* Default state setting */
  conf_session.log_state = LOG_OFF;// Set to log off on startup
  memcpy(conf_session.user_id, "0.0.0.0", sizeof("0.0.0.0"));

  /* Process debug mode, serial port */
  for(rolling_counter=0,ret=CFG_AOK;(-1 != (c_arg = getopt(argc, argv, "D")));rolling_counter++) {
      switch (c_arg) {
          case 'D':
              enable_debug = 1;
              break;
          default:
              break;
      }
      if(1000 < rolling_counter) {
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"CFG_GETOPT_FAIL.\n");
          ret = CFG_GETOPT_FAIL;
          break;
      }
  }
  if(CFG_AOK == ret) {
      if(0 == enable_debug) {
          i2vUtilDisableDebug();
      } else {
          i2vUtilEnableDebug(MY_NAME); /* Enable DEBUG until i2v.conf turns us off. */
      }
      if ((qd_server = mq_open(QUEUE_NAME, O_RDONLY | O_CREAT, QUEUE_PERMISSIONS, &attr)) == -1) {
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"CFG_MQ_OPEN_FAIL.\n");
          ret = CFG_MQ_OPEN_FAIL;
      }
  }
  if(CFG_AOK == ret) {
      /* Start MQ thread - handling incomming msg. */
      if (pthread_create( &config_mq_thread, NULL, (void *) &config_mq_handler, NULL)) {
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"CFG_HANDLING_THREAD_FAIL.\n");
          ret = CFG_HANDLING_THREAD_FAIL;
      } else {
          pthread_detach( config_mq_thread);
      }
  }
  if(CFG_AOK == ret) {
      conf_init();
      /* Thread to monitor log in timeout */
      if (pthread_create( &timeout_thread, NULL, (void *) &timeout_handler, NULL)) {
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"CFG_MONITOR_THREAD_FAI.\n");
          ret = CFG_MONITOR_THREAD_FAIL;
      } else {
          pthread_detach( timeout_thread);
      }
  }
  if(CFG_AOK == ret) {
      rolling_counter = 0;
      while(loop) {
          if(0 == (rolling_counter % OUTPUT_MODULUS)){
              display_faulty_conf_tables();
              I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(0x%lx)\n", my_error_states);
          }
          rolling_counter++;
          sleep(1);
          #if defined(MY_UNIT_TEST)
          break;
          #endif
      }
  }
#if defined(EXTRA_DEBUG)
  /* Dump final state to syslog. */
  I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"EXIT: ret(%d) (0x%lx)\n", ret, my_error_states);
#endif
  /* Stop I2V_DBG_LOG output. Last chance to output to syslog. */
  i2vUtilDisableDebug();

  /* Wait for threads end */
  if(CFG_HANDLING_THREAD_FAIL != ret) {
      pthread_cancel(config_mq_thread);
  }
  if(CFG_MONITOR_THREAD_FAIL != ret) {
      pthread_cancel(timeout_thread);
  }
  clean_up();
  return abs(ret);
}
