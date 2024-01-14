/*
 *  Filename: conf_table.h
 *  Purpose: Configuration for Web interface
 *
 *  Copyright (C) 2019 DENSO International America, Inc.
 *
 *  Revision History:
 *
 *  Date        Author        Comments
 * --------------------------------------------------
 *  07-01-19    LNGUYEN    Initial release.
 */

#ifndef CONF_TABLE_H
#define CONF_TABLE_H

#include "dn_types.h"
#include "conf_manager.h"
/* 
 * Item state - lower 7 bit of the byte 
 * most significant bit is custom flag bit is the valid indicator per custom setup
 *
 *  enum {
 *    NONE,      => item does not exist
 *    UNINIT,    => not initialized - cache need to be updated
 *    INIT,      => already initialized
 *    MOD,       => modified - storage need to be update
 *    REFRESH,   => special case - cache need to be update
 *  };
 *
 */

#define NONE        0x00
#define UNINIT      0x01
#define INIT        0x02    /* Loaded conf(DCU) value into cache. */
#if 0
#define MOD         0x03
#define REFRESH     0x04
#endif
#define BAD_VAL     0x04    /* Failed to get conf(DCU) value. Using default(conf_element_t) in cache. */
#define ITEM_VALID  0x80    /* Most significant bit for valid flag */
/* We can create confs from scratch using compiled table. Then modify as needed by user. */
typedef struct{
    char_t name[MAX_NAME_SIZE];
    char_t val[MAX_TOKEN_SIZE];
    uint8_t state; /* valid AND (uninitialized or initialized or bad value or none) */
#if 0
    uint8_t attr;            /* on/off 0:1 , range value, ... */
#endif
} conf_element_t;

/*
 * ===== Configuration table =====
 * Collection of all config itens - RSU
 * 
 * i2v.conf
 * amh.conf
 * ipb.conf
 * scs.conf
 * spat16.conf
 * srm_rx.conf
 * fwdmsg.conf
 * snmpd.conf
 * nsconfig.conf
 * rsinfo stuff
 * syslog.conf: TODO, not in cfgmgr yet
 */

/* 
 * Shared I2V Conf Constants:
 *   All calls to conf_agent <=> conf_manager <=> cfgmgrapi <=> cfgmgr(dcu) are string based.
 *   Therefore, strlen() an easy method to know goodness of conf value immediately.
 */

//TODO: Not to be confused with RSU_RADIOS_MIN & RSU_RADIOS_MIN from ntcip-1218.h
#define I2V_RADIO_MIN 0 
#define I2V_RADIO_MAX 0
#define I2V_RADIO_DEFAULT 0
#define I2V_RADIO_MIN_S "0" 
#define I2V_RADIO_MAX_S "0"
#define I2V_RADIO_DEFAULT_S "0"

#define I2V_RADIO_CHANNEL_MIN      183
#define I2V_RADIO_CHANNEL_MAX      183
#define I2V_RADIO_CHANNEL_DEFAULT  183
#define I2V_RADIO_CHANNEL_MIN_S      "183"
#define I2V_RADIO_CHANNEL_MAX_S      "183"
#define I2V_RADIO_CHANNEL_DEFAULT_S  "183"

#define I2V_PSID_MIN_S "0x0"
#define I2V_PSID_MAX_S "0xEFFFFFFF"

#define I2V_SPAT_PSID_DEFAULT_S "0x8002"
#define I2V_MAP_PSID_DEFAULT_S  "0xe0000017"
#define I2V_TIM_PSID_DEFAULT_S  "0x8003"
#define I2V_IPB_PSID_DEFAULT_S  "0xEFFFFD01"
#define I2V_BSM_PSID_DEFAULT_S  "0x20"
#define SRMRX_PSID_DEFAULT_S    "0xe0000016"

#define I2V_SPAT_PSID_DEFAULT 0x8002
#define I2V_MAP_PSID_DEFAULT  0xe0000017
#define I2V_TIM_PSID_DEFAULT  0x8003
#define I2V_IPB_PSID_DEFAULT  0x23
#define I2V_BSM_PSID_DEFAULT  0x20
#define SRMRX_PSID_DEFAULT    0xe0000016

#define PSID_ENCODE_MASK_1BYTE 0x0       /* PSID 1 bytes (bit 7) 0 (bit 6) 0 (bit 5) 0 (bit 4)0 */
#define PSID_ENCODE_MASK_2BYTE 0x80      /* PSID 2 bytes (bit 7) 1 (bit 6) 0 (bit 5) x (bit 4)x */
#define PSID_ENCODE_MASK_3BYTE 0xC0      /* PSID 3 bytes (bit 7) 1 (bit 6) 1 (bit 5) 0 (bit 4)x */
#define PSID_ENCODE_MASK_4BYTE 0xE0      /* PSID 4 bytes (bit 7) 1 (bit 6) 1 (bit 5) 1 (bit 4)0 */
#define PSID_1BYTE_MAX_VALUE  0x7F       /* max value for 1 byte encoding  (0  encoding) */
#define PSID_2BYTE_MIN_VALUE  0x8000     /* min value for 2 byte encoding  (10 encoding) */
#define PSID_2BYTE_MAX_VALUE  0xBFFF     /* max value for 2 byte encoding  (10 encoding) */
#define PSID_3BYTE_MIN_VALUE  0xC00000   /* min value for 3 byte encoding (110 encoding) */
#define PSID_3BYTE_MAX_VALUE  0xDFFFFF   /* max value for 3 byte encoding (110 encoding) */
#define PSID_4BYTE_MIN_VALUE  0xE0000000 /* min value for 4 byte encoding (1110 encoding) */
#define PSID_4BYTE_MAX_VALUE  0xEFFFFFFF /* max value for 4 byte encoding (1110 encoding) */

#define MIN_SSP_DATA    0
#define MIN_SSP_DATA_S "0"
#define MAX_SSP_DATA_S "32"

#define I2V_SPAT_SSP_ENABLE_DEFAULT              1
#define I2V_SPAT_BITMAPPED_SSP_ENABLE_DEFAULT    0
#define I2V_SPAT_SSP_ENABLE_DEFAULT_S           "1"
#define I2V_SPAT_BITMAPPED_SSP_ENABLE_DEFAULT_S "0"
#define I2V_SPAT_SSP_STRING_DEFAULT_S           "0080013040"
#define I2V_SPAT_SSP_MASK_STRING_DEFAULT_S      "0080013040"

#define I2V_MAP_SSP_ENABLE_DEFAULT              1
#define I2V_MAP_BITMAPPED_SSP_ENABLE_DEFAULT    0
#define I2V_MAP_SSP_ENABLE_DEFAULT_S           "1"
#define I2V_MAP_BITMAPPED_SSP_ENABLE_DEFAULT_S "0"
#define I2V_MAP_SSP_STRING_DEFAULT_S           "0080012040"
#define I2V_MAP_SSP_MASK_STRING_DEFAULT_S      "0080012040"

#define I2V_TIM_SSP_ENABLE_DEFAULT              1
#define I2V_TIM_BITMAPPED_SSP_ENABLE_DEFAULT    0
#define I2V_TIM_SSP_ENABLE_DEFAULT_S           "1"
#define I2V_TIM_BITMAPPED_SSP_ENABLE_DEFAULT_S "0"
#define I2V_TIM_SSP_STRING_DEFAULT_S           "008001F040"
#define I2V_TIM_SSP_MASK_STRING_DEFAULT_S      "008001F040"

#define I2V_SECURITY_VTP_MSG_RATE_MIN           0
#define I2V_SECURITY_VTP_MSG_RATE_MAX           65535
#define I2V_SECURITY_VTP_MSG_RATE_DEFAULT       65535
#define I2V_SECURITY_VTP_MSG_RATE_MIN_S         "0"
#define I2V_SECURITY_VTP_MSG_RATE_MAX_S         "65535"
#define I2V_SECURITY_VTP_MSG_RATE_DEFAULT_S     "65535"

#define I2V_ENABLE_VOD_DEFAULT_S            "0"
#define I2V_ENABLE_VOD_DEFAULT              0

#define I2V_LAT_DEG_MIN -90.0  /* float64_t */
#define I2V_LAT_DEG_MAX  90.0
#define I2V_LAT_DEG_MIN_S "-90.0"  /* float64_t */
#define I2V_LAT_DEG_MAX_S  "90.0"

#define I2V_LON_DEG_MIN -180.0   /* float64_t */
#define I2V_LON_DEG_MAX  180.0
#define I2V_LON_DEG_MIN_S "-180.0"   /* float64_t */
#define I2V_LON_DEG_MAX_S  "180.0"

#define I2V_ELV_M_MIN   -100  /* int16_t */
#define I2V_ELV_M_MAX    1500
#define I2V_ELV_M_MIN_S  "-100"  /* int16_t */
#define I2V_ELV_M_MAX_S  "1500"

/* 
 * 4 chars "0.0" to "23.0" in "0.5" half steps only.
 * ntcip-1218 this is an integer and no half steps.
 */
#define I2V_TX_POWER_MIN       0
#define I2V_TX_POWER_MAX       23.0
#define I2V_TX_POWER_DEFAULT   22.0

#define I2V_TX_POWER_MIN_S       "0"
#define I2V_TX_POWER_MAX_S       "23.0"
#define I2V_TX_POWER_DEFAULT_S   "22.0" 

/* Type of IP addr being requested. */
#define I2V_IP_ADDR_V4 0
#define I2V_IP_ADDR_V6 1

#define I2V_DEST_IP_MIN        0 /* IPv4 and IPv6 */
#define I2V_DEST_IP_MAX       64
#define I2V_DEST_IP_MIN_S     "" /* IPv4 and IPv6 */
#define I2V_DEST_IP_MAX_S     "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF"

#define I2V_DEST_PORT_MIN      0
#define I2V_DEST_PORT_MAX      65535
#define I2V_DEST_PORT_MIN_S    "0"
#define I2V_DEST_PORT_MAX_S    "65535"

#define I2V_ENABLE_MIN 0
#define I2V_ENABLE_MAX 1
#define I2V_ENABLE_MIN_S "0"
#define I2V_ENABLE_MAX_S "1"

/*
 * ===============================  
 * ============ RSUINFO ==========
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

/* Some runtime values, startup.sh values and some confs. Irregular. */
#define I2V_RSU_NETWORK_MODE_MIN       0
#define I2V_RSU_NETWORK_MODE_MAX       1
#define I2V_RSU_NETWORK_MODE_MIN_S     "0"
#define I2V_RSU_NETWORK_MODE_MAX_S     "1"
#define I2V_RSU_NETWORK_MODE_DEFAULT_S "0"
  
#define I2V_RSU_ETH0_IP_DEFAULT_S "192.168.2.80"

#define I2V_RSU_NETMASK_MIN_S     "1"
#define I2V_RSU_NETMASK_MAX_S     "31"
#define I2V_RSU_NETMASK_DEFAULT_S "24"

#define I2V_APP_ENABLE_DEFAULT_S "1"

#define I2V_RSUHEALTH_ENABLE_DEFAULT_S "1"

#define I2V_SYS_MON_ENABLE_DEFAULT_S "1"

#define WEBGUI_HTTP_SERVICE_PORT_MIN_S      I2V_DEST_PORT_MIN_S
#define WEBGUI_HTTPS_SERVICE_PORT_MIN_S     I2V_DEST_PORT_MIN_S
#define WEBGUI_HTTP_SERVICE_PORT_MAX_S      I2V_DEST_PORT_MAX_S
#define WEBGUI_HTTPS_SERVICE_PORT_MAX_S     I2V_DEST_PORT_MAX_S
#define WEBGUI_HTTP_SERVICE_PORT_DEFAULT_S  "5908"
#define WEBGUI_HTTPS_SERVICE_PORT_DEFAULT_S "4934"

#define I2V_RSU_MTU_DEFAULT_S          "1500"
#define I2V_RSU_VLAN_ENABLE_DEFAULT_S  "0"
#define I2V_RSU_VLAN_IP_DEFAULT_S      "192.168.3.80"
#define I2V_RSU_VLAN_MASK_DEFAULT_S    "24"
#define I2V_RSU_VLAN_ID_DEFAULT_S      "0"

/*
 * ===============================  
 * ========== NSCONFIG ===========
 * ===============================
 */
//typedef struct EDAC_VAL
//{
//    rsRadioType  radioType;           // RT_CV2X or RT_DSRC
//    uint8_t      radioNum;            // radio number (0-1)
//    rsEdcaACType accessCategory;      // Access Category (0-3)
//    CHANType     chnlType;            // CHANTYPE_CCH or CHANTYPE_SCH
//    uint32_t     ecw_min;             // CWmin Exponent (2-10)
//    uint32_t     ecw_max;             // CWmax Exponent (2-10)
//    uint32_t     aifs;                // Arbitration Interframe Spacing (AIFS) (1-15)
//    uint32_t     txop;                // Transmit Opportunity (Txop) (normally 0)
//} rsEdcaType;

#define AC_BE_CCH_RADIO_0_DEFAULT_S "15,1023,6,0"
#define AC_BK_CCH_RADIO_0_DEFAULT_S "15,1023,9,0"
#define AC_VO_CCH_RADIO_0_DEFAULT_S "15,1023,4,0"
#define AC_VI_CCH_RADIO_0_DEFAULT_S "3,7,2,0"
#define AC_BE_SCH_RADIO_0_DEFAULT_S "15,1023,6,0"
#define AC_BK_SCH_RADIO_0_DEFAULT_S "15,1023,9,0"
#define AC_VO_SCH_RADIO_0_DEFAULT_S "15,1023,4,0"
#define AC_VI_SCH_RADIO_0_DEFAULT_S "3,7,2,0"
#define AC_BE_CCH_RADIO_1_DEFAULT_S "15,1023,6,0"
#define AC_BK_CCH_RADIO_1_DEFAULT_S "15,1023,9,0"
#define AC_VO_CCH_RADIO_1_DEFAULT_S "15,1023,4,0"
#define AC_VI_CCH_RADIO_1_DEFAULT_S "3,7,2,0"
#define AC_BE_SCH_RADIO_1_DEFAULT_S "15,1023,6,0"
#define AC_BK_SCH_RADIO_1_DEFAULT_S "15,1023,9,0"
#define AC_VO_SCH_RADIO_1_DEFAULT_S "15,1023,4,0"
#define AC_VI_SCH_RADIO_1_DEFAULT_S "3,7,2,0"

/*
 * ===============================  
 * ============ I2V ==============
 * ===============================
 */
#define I2V_SCS_ENABLE_DEFAULT    1
#define I2V_SCS_ENABLE_DEFAULT_S "1"

#define I2V_SRM_ENABLE_DEFAULT    1
#define I2V_SRM_ENABLE_DEFAULT_S "1"

#define I2V_IPB_ENABLE_DEFAULT    0
#define I2V_IPB_ENABLE_DEFAULT_S "0"

#define I2V_AMH_ENABLE_DEFAULT    1
#define I2V_AMH_ENABLE_DEFAULT_S "1"

#define I2V_IWMH_ENABLE_DEFAULT    1
#define I2V_IWMH_ENABLE_DEFAULT_S "1"

#define I2V_FWDMSG_ENABLE_DEFAULT  1
#define I2V_FWDMSG_ENABLE_DEFAULT_S  "1"

#define I2V_SMNPD_ENABLE_DEFAULT    1
#define I2V_SNMPD_ENABLE_DEFAULT_S "1"

#define I2V_GLOBAL_DEBUG_DEFAULT     1
#define I2V_GLOBAL_DEBUG_DEFAULT_S  "1"

#define I2V_DEBUG_DEFAULT     1
#define I2V_DEBUG_DEFAULT_S  "1"

#define I2V_GPS_OVERRIDE_ENABLE_DEFAULT    0
#define I2V_GPS_OVERRIDE_ENABLE_DEFAULT_S "0"

#define I2V_GPS_LAT_DEFAULT    37.418797
#define I2V_GPS_LON_DEFAULT   -122.135260
#define I2V_GPS_ELEV_DEFAULT   30
#define I2V_GPS_LAT_DEFAULT_S  "37.418797"
#define I2V_GPS_LON_DEFAULT_S  "-122.135260"
#define I2V_GPS_ELEV_DEFAULT_S "30"

#define I2VRSUID_MIN 0
#define I2VRSUID_MAX 999
#define I2VRSUID_DEFAULT 1
#define I2VRSUID_MIN_S "0"
#define I2VRSUID_MAX_S "999"
#define I2VRSUID_DEFAULT_S "1"

#define I2V_CONTROL_RADIO_MIN 0
#define I2V_CONTROL_RADIO_MAX 3
#define I2V_CONTROL_RADIO_DEFAULT 0
#define I2V_CONTROL_RADIO_MIN_S "0"
#define I2V_CONTROL_RADIO_MAX_S "3"
#define I2V_CONTROL_RADIO_DEFAULT_S "0"

#define I2V_SEC_TIME_BUFFER_MIN 0
#define I2V_SEC_TIME_BUFFER_MAX 30
#define I2V_SEC_TIME_BUFFER_DEFAULT 0
#define I2V_SEC_TIME_BUFFER_MIN_S "0"
#define I2V_SEC_TIME_BUFFER_MAX_S "30"
#define I2V_SEC_TIME_BUFFER_DEFAULT_S "0"

#define I2V_CERT_ATTACH_RATE_MIN 1
#define I2V_CERT_ATTACH_RATE_MAX 100
#define I2V_CERT_ATTACH_RATE_DEFAULT 20

#define I2V_CERT_ATTACH_RATE_MIN_S "1"
#define I2V_CERT_ATTACH_RATE_MAX_S "100"
#define I2V_CERT_ATTACH_RATE_DEFAULT_S "20"

#define I2V_RADIO_TYPE_MIN 1
#define I2V_RADIO_TYPE_MAX 1
#define I2V_RADIO_TYPE_DEFAULT 1

#define I2V_RADIO_TYPE_MIN_S "1"
#define I2V_RADIO_TYPE_MAX_S "1"
#define I2V_RADIO_TYPE_DEFAULT_S "1"

#define I2V_ENABLE_IPV6_DEFAULT    0
#define I2V_ENABLE_IPV6_DEFAULT_S "0"

#define I2V_ETH_IPV6_ADDR_DEFAULT_S "2001:1890:12e2:9900::b"

#define I2V_ETH_IPV6_SCOPE_DEFAULT   64
#define I2V_ETH_IPV6_SCOPE_DEFAULT_S "64"

#define I2V_IPV6_GATEWAY_DEFAULT_S "2001:1890:12e2:9900::1"

#define I2V_ETH_IPV6_SCOPE_MIN      1 
#define I2V_ETH_IPV6_SCOPE_MAX      128 
#define I2V_ETH_IPV6_SCOPE_DEFAULT  64

#define I2V_ETH_IPV6_SCOPE_MIN_S      "1"
#define I2V_ETH_IPV6_SCOPE_MAX_S      "128"
#define I2V_ETH_IPV6_SCOPE_DEFAULT_S  "64"

#define I2V_FORCE_ALIVE_DEFAULT    0
#define I2V_FORCE_ALIVE_DEFAULT_S "0"

#define I2V_RADIO_SEC_VERIFY_DEFAULT    0
#define I2V_RADIO_SEC_VERIFY_DEFAULT_S "0"

#define I2V_RADIO0_ENABLE_DEFAULT    1
#define I2V_RADIO0_ENABLE_DEFAULT_S "1"
#define I2V_RADIO0_MODE_DEFAULT    1
#define I2V_RADIO0_MODE_DEFAULT_S "1"

#define I2V_RADIO1_ENABLE_DEFAULT    0
#define I2V_RADIO1_ENABLE_DEFAULT_S "0"
#define I2V_RADIO1_MODE_DEFAULT    1
#define I2V_RADIO1_MODE_DEFAULT_S "1"

#define I2V_RADIO2_ENABLE_DEFAULT    0
#define I2V_RADIO2_ENABLE_DEFAULT_S "0"
#define I2V_RADIO2_MODE_DEFAULT    1
#define I2V_RADIO2_MODE_DEFAULT_S "1"

#define I2V_RADIO3_ENABLE_DEFAULT    0
#define I2V_RADIO3_ENABLE_DEFAULT_S "0"
#define I2V_RADIO3_MODE_DEFAULT    1
#define I2V_RADIO3_MODE_DEFAULT_S "1"

#define I2V_BROADCAST_UNIFIED_DEFAULT    1
#define I2V_BROADCAST_UNIFIED_DEFAULT_S "1"

#define I2V_SECURITY_ENABLE_DEFAULT    0
#define I2V_SECURITY_ENABLE_DEFAULT_S "0"

#define I2V_GEN_DEFAULT_CERTS_DEFAULT    0 
#define I2V_GEN_DEFAULT_CERTS_DEFAULT_S "0"

#define I2V_AUTO_GEN_CERT_RESTART_DEFAULT    0
#define I2V_AUTO_GEN_CERT_RESTART_DEFAULT_S "0"

#define I2V_STORAGE_BYPASS_DEFAULT    0
#define I2V_STORAGE_BYPASS_DEFAULT_S "0"

#define I2V_WSA_ENABLE_DEFAULT  1
#define I2V_WSA_ENABLE_DEFAULT_S "1"

#define I2V_WSA_UNIFIED_PRIORITY_DEFAULT  20
#define I2V_WSA_UNIFIED_PRIORITY_DEFAULT_S "20"

#define I2V_USE_DEVICE_ID_DEFAULT 0
#define I2V_USE_DEVICE_ID_DEFAULT_S "0"

#define I2V_USE_GPS_TX_CONTROL_DEFAULT 1
#define I2V_USE_GPS_TX_CONTROL_DEFAULT_S "1"

#define I2V_CONSOLE_CLEANUP_DEFAULT 0
#define I2V_CONSOLE_CLEANUP_DEFAULT_S "0"

#define I2V_DISABLE_SYSLOG_DEFAULT 0
#define I2V_DISABLE_SYSLOG_DEFAULT_S "0"

#define I2V_SNMP_ENABLE_DEFAULT    0
#define I2V_SNMP_ENABLE_DEFAULT_S "0"

#define I2V_BROADCAST_TIMESHIFT_DEFAULT    1
#define I2V_BROADCAST_TIMESHIFT_DEFAULT_S "1"

#define I2V_IFACE_LOG_MASK_DEFAULT    0
#define I2V_IFACE_LOG_MASK_DEFAULT_S "0"

#define I2V_SECURITY_DIR_DEFAULT_S "/rwflash/security"

#define I2V_DEFAULT_UTC_SECONDS         1673143634
#define I2V_DEFAULT_UTC_USECONDS         0

#define I2V_CONFIG_TIME_MODE_DEFAULT    0
#define I2V_CONFIG_TIME_MODE_DEFAULT_S "0"
 
/*
 * ===============================
 * ============ AMH ==============
 * ===============================
 */

//RO: compile time
#if defined(MY_UNIT_TEST)
#define AMH_ACTIVE_DIR "../stubs/amh"
#else
#define AMH_ACTIVE_DIR "/rwflash/I2V/amh"
#endif
#define AMH_ACTIVE_DIR_MIN_S ""
 /* 64 chars. */
#define AMH_ACTIVE_DIR_MAX_S "0123456789012345678901234567890123456789012345678901234567891234"

//Not used.
#define AMH_MAX_STORE_MESSAGES_MIN_S "0"
#define AMH_MAX_STORE_MESSAGES_MAX_S "100"
#define AMH_MAX_STORE_MESSAGES_DEFAULT_S "100"

//Legacy 4.1 not 1218
#define AMH_ENABLE_VERIFICATION_DEFAULT    0 
#define AMH_ENABLE_VERIFICATION_DEFAULT_S "0" 

#define AMH_IMF_IP_FILTER_DEFAULT 0 
#define AMH_IMF_IP_FILTER_DEFAULT_S "0" 

#define AMH_ENABLE_SAR_DEFAULT 1 
#define AMH_ENABLE_SAR_DEFAULT_S "1" 

#define AMH_ENABLE_IMF_DEFAULT 1 
#define AMH_ENABLE_IMF_DEFAULT_S "1"

#define AMH_IMF_IP_DEFAULT_S "192.168.2.47"

#define I2V_IMF_PORT_DEFAULT 1516
#define I2V_IMF_PORT_DEFAULT_S "1516"

//Legacy 4.1 not 1218
#define AMH_FWD_IP_DEFAULT_S "192.168.2.47"

#define I2V_AMH_FWD_PORT_DEFAULT 11001
#define I2V_AMH_FWD_PORT_DEFAULT_S "11001"

#define I2V_AMH_FORWARD_ENABLE_MIN      0
#define I2V_AMH_FORWARD_ENABLE_MAX      0xFFFFFFFF
#define I2V_AMH_FORWARD_ENABLE_DEFAULT  0x7FFFFFFF
#define I2V_AMH_FORWARD_ENABLE_MIN_S      "0"
#define I2V_AMH_FORWARD_ENABLE_MAX_S      "0xFFFFFFFF"
#define I2V_AMH_FORWARD_ENABLE_DEFAULT_S  "0"

#define AMH_SETTINGS_OVERRIDE_DEFAULT 0 
#define AMH_SETTINGS_OVERRIDE_DEFAULT_S "0" 

#define AMH_FORCE_PSID_DEFAULT 0 
#define AMH_FORCE_PSID_DEFAULT_S "0"

#define AMH_SEND_OVERRIDE_DEFAULT 0 
#define AMH_SEND_OVERRIDE_DEFAULT_S "0"

#define AMH_WSM_EXTENSION_DEFAULT 0 
#define AMH_WSM_EXTENSION_DEFAULT_S "0"

#define AMH_INTERVAL_SELECT_DEFAULT 1 
#define AMH_INTERVAL_SELECT_DEFAULT_S "1"

/*
 * ===============================
 * ============ IPB ==============
 * ===============================
 */

//TODO: Making IPB priority the default for everyone else since this is the ony WSA we support?

/* Conflicts with NTCIP-1218: 5.10.2.1 - 5.10.2.16: WSA Table: rsuWsaServiceTable? 1 to 7 there. */
#define I2V_WSA_PRIORITY_MIN 0
#define I2V_WSA_PRIORITY_MAX 62
#define I2V_WSA_PRIORITY_DEFAULT 31
#define I2V_WSA_PRIORITY_MIN_S "0"
#define I2V_WSA_PRIORITY_MAX_S "62"
#define I2V_WSA_PRIORITY_DEFAULT_S "31"

/* Unique to IPB. */
#define IPB_OVERRIDE_POWER_DEFAULT    0
#define IPB_OVERRIDE_POWER_DEFAULT_S "0"

#define IPB_ENABLE_CONT_DEFAULT    1
#define IPB_ENABLE_CONT_DEFAULT_S "1"

#define IPB_OVERRIDE_CHANNEL_ENABLE_DEFAULT    0
#define IPB_OVERRIDE_CHANNEL_ENABLE_DEFAULT_S "0"

#define IPB_SECURITY_DEBUG_ENABLE_DEFAULT    0
#define IPB_SECURITY_DEBUG_ENABLE_DEFAULT_S "0"

#define IPB_ENABLE_EDCA_DEFAULT    0
#define IPB_ENABLE_EDCA_DEFAULT_S "0"

#define IPB_GNSS_OVERRIDE_DEFAULT    0
#define IPB_GNSS_OVERRIDE_DEFAULT_S "0"

#define IPB_IPV6_USE_LOCAL_LINK_DEFAULT    0
#define IPB_IPV6_USE_LOCAL_LINK_DEFAULT_S "0"

#define IPB_ENABLE_BRIDGE_DEFAULT    1
#define IPB_ENABLE_BRIDGE_DEFAULT_S "1"

#define IPB_BRIDGE_ONCE_DEFAULT     0
#define IPB_BRIDGE_ONCE_DEFAULT_S  "0"

#define IPB_IPV4_BRIDGE_DEFAULT    1
#define IPB_IPV4_BRIDGE_DEFAULT_S "1"

#define IPB_BRIDGE_DELAY_CLIENTS_DEFAULT    0
#define IPB_BRIDGE_DELAY_CLIENTS_DEFAULT_S "0"

#define I2V_WSA_TX_RATE_MIN       0
#define I2V_WSA_TX_RATE_MAX       50
#define I2V_WSA_TX_RATE_DEFAULT   50
#define I2V_WSA_TX_RATE_MIN_S     "0"
#define I2V_WSA_TX_RATE_MAX_S     "50"
#define I2V_WSA_TX_RATE_DEFAULT_S "50"

#define I2V_WSA_DATA_RATE_MIN       3
#define I2V_WSA_DATA_RATE_MAX       54
#define I2V_WSA_DATA_RATE_DEFAULT   18
#define I2V_WSA_DATA_RATE_MIN_S     "3"
#define I2V_WSA_DATA_RATE_MAX_S     "54"
#define I2V_WSA_DATA_RATE_DEFAULT_S "18"

#define WSA_ADVERTISE_ID_MIN 0
#define WSA_ADVERTISE_ID_MAX WSA_ADVERTISE_ID_LEN
#define IPB_AD_ID_DEFAULT_S "TWRSU"
#define WSA_ADVERTISE_ID_MIN_S "0"
#define WSA_ADVERTISE_ID_MAX_S "32"

#define IPB_SV_COUNT_MIN       1
#define IPB_SV_COUNT_MAX       10
#define IPB_SV_COUNT_DEFAULT   3
#define IPB_SV_COUNT_MIN_S     "1"
#define IPB_SV_COUNT_MAX_S     "10"
#define IPB_SV_COUNT_DEFAULT_S "3"

#define IPB_GNSS_TO_MIN     1
#define IPB_GNSS_TO_MAX     60
#define IPB_GNSS_TO_DEFAULT 45
#define IPB_GNSS_TO_MIN_S     "1"
#define IPB_GNSS_TO_MAX_S     "60"
#define IPB_GNSS_TO_DEFAULT_S "45"

#define IPB_GNSS_SETTLE_TIME_MIN       20
#define IPB_GNSS_SETTLE_TIME_MAX       300
#define IPB_GNSS_SETTLE_TIME_DEFAULT   120
#define IPB_GNSS_SETTLE_TIME_MIN_S     "20"
#define IPB_GNSS_SETTLE_TIME_MAX_S     "300"
#define IPB_GNSS_SETTLE_TIME_DEFAULT_S "120"

#define IPB_POS_CONFIDENCE_MIN        0
#define IPB_POS_CONFIDENCE_MAX        0xFF
#define IPB_POS_CONFIDENCE_DEFAULT    0x66
#define IPB_POS_CONFIDENCE_MIN_S      "0"
#define IPB_POS_CONFIDENCE_MAX_S      "0xFF"
#define IPB_POS_CONFIDENCE_DEFAULT_S  "0x66"

#define IPB_IPV6_PREFIX_DEFAULT_S "2000:0:0:0"
#define IPB_IPV6_ID_DEFAULT_S     "0:0:0:1"
#define IPB_IPV6_PORT_DEFAULT      22
#define IPB_IPV6_PORT_DEFAULT_S   "22"
#define IPB_IPV6_SERVER_PREFIX_DEFAULT_S "2000:0:0:0"
#define IPB_IPV6_SERVER_ID_DEFAULT_S "0:0:0:10"

#define IPB_BRIDGE_CLIENT_MIN        1
#define IPB_BRIDGE_CLIENT_MAX        5
#define IPB_BRIDGE_CLIENT_DEFAULT    5
#define IPB_BRIDGE_CLIENT_MIN_S     "1"
#define IPB_BRIDGE_CLIENT_MAX_S     "5"
#define IPB_BRIDGE_CLIENT_DEFAULT_S "5"

#define IPB_PROVIDER_CTXT_MIN         1
#define IPB_PROVIDER_CTXT_MAX         4
#define IPB_PROVIDER_CTXT_MIN_S      "1"
#define IPB_PROVIDER_CTXT_MAX_S      "4"
#define IPB_PROVIDER_CTXT_DEFAULT_S  "SCMS"

#define IPB_CONNECT_TO_MIN       1
#define IPB_CONNECT_TO_MAX       60
#define IPB_CONNECT_TO_DEFAULT   20
#define IPB_CONNECT_TO_MIN_S     "1"
#define IPB_CONNECT_TO_MAX_S     "60"
#define IPB_CONNECT_TO_DEFAULT_S "20"

#define IPB_SERVER_URL_MIN       0
#define IPB_SERVER_URL_MAX       MAX_TOKEN_SIZE
#define IPB_SERVER_URL_MIN_S     ""
#define IPB_SERVER_URL_MAX_S     "65"
#define IPB_SERVER_URL_DEFAULT_S ""

#define IPB_SRV_MAC_MIN       17
#define IPB_SRV_MAC_MAX       17
#define IPB_SRV_MAC_MIN_S     "17"
#define IPB_SRV_MAC_MAX_S     "17"
#define IPB_SRV_MAC_DEFAULT_S "00:50:B6:0D:99:C4"

#define IPB_IPV4_SERVER_IP_DEFAULT_S   "141.211.92.230"
#define IPB_IPV4_SERVER_PORT_DEFAULT    2222
#define IPB_IPV4_SERVER_PORT_DEFAULT_S "2222"

/*
 * ===============================
 * ============ SCS ==============
 * ===============================
 */
#define SCS_INTERFACE_TOM_DEFAULT    0
#define SCS_INTERFACE_TOM_DEFAULT_S "0"

#define SCS_USE_MAX_TIMETO_CHANGE_DEFAULT    0
#define SCS_USE_MAX_TIMETO_CHANGE_DEFAULT_S "0"

#define SCS_HEARTBEAT_ENABLE_DEFAULT    0
#define SCS_HEARTBEAT_ENABLE_DEFAULT_S "0"

#define SRMRX_FWD_TEST_DEFAULT 0
#define SRMRX_FWD_TEST_DEFAULT_S "0"

#define SRMRX_FWD_TEST_FILE_S "/rwflash/srm/srm_out.csv"

#define SRMRX_FWD_ENABLE_DEFAULT    1
#define SRMRX_FWD_ENABLE_DEFAULT_S "1"

#define SRMRX_REQUIRE_TTI_DEFAULT    0
#define SRMRX_REQUIRE_TTI_DEFAULT_S "0"

#define SCS_LOCAL_CONTROLLER_ENABLE_DEFAULT    1
#define SCS_LOCAL_CONTROLLER_ENABLE_DEFAULT_S "1"

#define SCS_LOCAL_CONTROLLER_IS_SNMP_DEFAULT    0
#define SCS_LOCAL_CONTROLLER_IS_SNMP_DEFAULT_S "0"

#define SCS_BYPASS_SIGNAL_CONTROLLER_NTCIP_DEFAULT    1
#define SCS_BYPASS_SIGNAL_CONTROLLER_NTCIP_DEFAULT_S "1"

#define I2V_SCS_LOCAL_CONTROLLER_IP_DEFAULT_S   "192.168.2.32"
#define I2V_SCS_LOCAL_CONTROLLER_PORT_DEFAULT    6053
#define I2V_SCS_LOCAL_CONTROLLER_PORT_DEFAULT_S "6053"
#define I2V_SCS_LOCAL_SNMP_IP_DEFAULT            501
#define I2V_SCS_LOCAL_SNMP_IP_DEFAULT_S         "501"

#define SRMRX_HEARTBEAT_INTERVAL_MIN        1
#define SRMRX_HEARTBEAT_INTERVAL_MAX        60
#define SRMRX_HEARTBEAT_INTERVAL_DEFAULT    3
#define SRMRX_HEARTBEAT_INTERVAL_MIN_S     "1"
#define SRMRX_HEARTBEAT_INTERVAL_MAX_S     "60"
#define SRMRX_HEARTBEAT_INTERVAL_DEFAULT_S "3"

#define SRMRX_FWD_INTERVAL_MIN       1000
#define SRMRX_FWD_INTERVAL_MAX       1000
#define SRMRX_FWD_INTERVAL_DEFAULT   1000
#define SRMRX_FWD_INTERVAL_MIN_S     "1000"
#define SRMRX_FWD_INTERVAL_MAX_S     "1000"
#define SRMRX_FWD_INTERVAL_DEFAULT_S "1000"

#define SRMRX_FWD_IP_DEFAULT_S "192.168.2.47"

#define SRMRX_FWD_PORT_DEFAULT    9091
#define SRMRX_FWD_PORT_DEFAULT_S "9091"

#define SRMRX_FWD_INTERFACE_MIN        1
#define SRMRX_FWD_INTERFACE_MAX        3
#define SRMRX_FWD_INTERFACE_DEFAULT    1
#define SRMRX_FWD_INTERFACE_MIN_S     "1"
#define SRMRX_FWD_INTERFACE_MAX_S     "3"
#define SRMRX_FWD_INTERFACE_DEFAULT_S "1"

//TODO: strange min max? GS2 hangover?
#define SCS_LOCAL_SIGNAL_INT_ID_MIN       147483700
#define SCS_LOCAL_SIGNAL_INT_ID_MAX       147483705
#define SCS_LOCAL_SIGNAL_INT_ID_DEFAULT   147483700 
#define SCS_LOCAL_SIGNAL_INT_ID_MIN_S     "147483700"
#define SCS_LOCAL_SIGNAL_INT_ID_MAX_S     "147483705"
#define SCS_LOCAL_SIGNAL_INT_ID_DEFAULT_S "147483700"

#define SCS_LOCAL_SIGNAL_HW_MIN        0
#define SCS_LOCAL_SIGNAL_HW_MAX        0xFF
#define SCS_LOCAL_SIGNAL_HW_DEFAULT    0
#define SCS_LOCAL_SIGNAL_HW_MIN_S     "0"
#define SCS_LOCAL_SIGNAL_HW_MAX_S     "0xFF"
#define SCS_LOCAL_SIGNAL_HW_DEFAULT_S "0"

#define SCS_SPAT_SNMP_ENABLE_COMMAND_MIN   0 
#define SCS_SPAT_SNMP_ENABLE_COMMAND_MAX    I2V_CFG_MAX_STR_LEN
#define SCS_SPAT_SNMP_ENABLE_COMMAND_DEFAULT_S "1.3.6.1.4.1.1206.3.5.2.9.44.1.1"

#define SCS_SPAT_COMMAND_MIN        0
#define SCS_SPAT_COMMAND_MAX        I2V_CFG_MAX_STR_LEN
#define SCS_SPAT_COMMAND_DEFAULT_S "6"

#define SCS_BYPASS_YELLOW_DURATION_MIN       1
#define SCS_BYPASS_YELLOW_DURATION_MAX       255
#define SCS_BYPASS_YELLOW_DURATION_DEFAULT   3
#define SCS_BYPASS_YELLOW_DURATION_MIN_S     "1"
#define SCS_BYPASS_YELLOW_DURATION_MAX_S     "255"
#define SCS_BYPASS_YELLOW_DURATION_DEFAULT_S "3"

#define NTCIP_YELLOW_DURATION_MIN       0
#define NTCIP_YELLOW_DURATION_MAX       I2V_CFG_MAX_STR_LEN
#define NTCIP_YELLOW_DURATION_DEFAULT_S "1.3.6.1.4.1.1206.4.2.1.1.2.1.8.1"

/* among other things provides phase enable */
#define NTCIP_PHASE_OPTIONS_DEFAULT_S     "1.3.6.1.4.1.1206.4.2.1.1.2.1.21"
#define NTCIP_PHASE_GREENS_DEFAULT_S      "1.3.6.1.4.1.1206.4.2.1.1.4.1.4"
#define NTCIP_PHASE_YELLOWS_DEFAULT_S     "1.3.6.1.4.1.1206.4.2.1.1.4.1.3"
#define NTCIP_PHASE_REDS_DEFAULT_S        "1.3.6.1.4.1.1206.4.2.1.1.4.1.2"
/* the following are private Controller specific SNMP commands (Battelle) */
#define NTCIP_PHASE_NUMBER_DEFAULT_S          "1.3.6.1.4.1.1206.3.47.1.1.1"
#define NTCIP_PHASE_MAX_COUNTDOWN_DEFAULT_S   "1.3.6.1.4.1.1206.3.47.1.1.3"
#define NTCIP_PHASE_MIN_COUNTDOWN_DEFAULT_S   "1.3.6.1.4.1.1206.3.47.1.1.2"

/*
 * ===============================
 * ============ SPAT ==============
 * ===============================
 */
#define BSM_RX_ENABLE_DEFAULT     1
#define BSM_RX_ENABLE_DEFAULT_S  "1"

#define SPAT_WSM_EXTENSION_ENABLE_DEFAULT    0
#define SPAT_WSM_EXTENSION_ENABLE_DEFAULT_S "0"

#define BSM_FWD_IP_DEFAULT_S "192.168.2.47"

#define BSM_FWD_PORT_DEFAULT    10001
#define BSM_FWD_PORT_DEFAULT_S "10001"

#define BSM_PSM_FWD_ENABLE_DEFAULT    1
#define BSM_PSM_FWD_ENABLE_DEFAULT_S "1"

#define SPAT_FWD_ENABLE_DEFAULT    0
#define SPAT_FWD_ENABLE_DEFAULT_S "0"

#define SPAT_WSA_PRIORITY_DEFAULT     20
#define SPAT_WSA_PRIORITY_DEFAULT_S  "20"

#define SPAT_INTEROP_MODE_MIN       0
#define SPAT_INTEROP_MODE_MAX       0xFF
#define SPAT_INTEROP_MODE_DEFAULT   0x7
#define SPAT_INTEROP_MODE_MIN_S     "0"
#define SPAT_INTEROP_MODE_MAX_S     "0xFF"
#define SPAT_INTEROP_MODE_DEFAULT_S "0x7"

#define SPAT_INT_ID_MIN       1
#define SPAT_INT_ID_MAX       2147483648
#define SPAT_INT_ID_DEFAULT   21020
#define SPAT_INT_ID_MIN_S     "1"
#define SPAT_INT_ID_MAX_S     "2147483648"
#define SPAT_INT_ID_DEFAULT_S "21020"

#define SPAT_BSM_RX_FWD_MIN      0
#define SPAT_BSM_RX_FWD_MAX      3
#define SPAT_BSM_RX_FWD_DEFAULT  0
#define SPAT_BSM_RX_FWD_MIN_S      "0"
#define SPAT_BSM_RX_FWD_MAX_S      "3"
#define SPAT_BSM_RX_FWD_DEFAULT_S  "0"

/*
 * ===============================
 * ============ SRM_RX ==============
 * ===============================
 */
#define SRM_PERMISSIVE_DEFAULT    1
#define SRM_PERMISSIVE_DEFAULT_S "1"

#define SRM_RX_FWD_ENABLE_DEFAULT   1
#define SRM_RX_FWD_ENABLE_DEFAULT_S "1"

#define SRM_SECURITY_DEBUG_ENABLE_DEFAULT    0
#define SRM_SECURITY_DEBUG_ENABLE_DEFAULT_S "0"

#define SRMRX_DECODE_METHOD_MIN       0
#define SRMRX_DECODE_METHOD_MAX       1
#define SRMRX_DECODE_METHOD_DEFAULT   1
#define SRMRX_DECODE_METHOD_MIN_S     "0"
#define SRMRX_DECODE_METHOD_MAX_S     "1"
#define SRMRX_DECODE_METHOD_DEFAULT_S "1"

#define SRMRX_ASN1_DECODE_METHOD_MIN       0
#define SRMRX_ASN1_DECODE_METHOD_MAX       2
#define SRMRX_ASN1_DECODE_METHOD_DEFAULT   0
#define SRMRX_ASN1_DECODE_METHOD_MIN_S     "0"
#define SRMRX_ASN1_DECODE_METHOD_MAX_S     "2"
#define SRMRX_ASN1_DECODE_METHOD_DEFAULT_S "0"

#define SRMTX_VEHICLE_ROLE_MIN       0
#define SRMTX_VEHICLE_ROLE_MAX       0xFFFFFFFF
#define SRMTX_VEHICLE_ROLE_DEFAULT   0xFFFFFFFF
#define SRMTX_VEHICLE_ROLE_MIN_S     "0"
#define SRMTX_VEHICLE_ROLE_MAX_S     "0xFFFFFFFF"
#define SRMTX_VEHICLE_ROLE_DEFAULT_S "0xFFFFFFFF"

#define SRM_VOD_MSG_VERIFY_COUNT_MIN        1
#define SRM_VOD_MSG_VERIFY_COUNT_MAX        65535
#define SRM_VOD_MSG_VERIFY_COUNT_DEFAULT    10
#define SRM_VOD_MSG_VERIFY_COUNT_MIN_S      "1"
#define SRM_VOD_MSG_VERIFY_COUNT_MAX_S      "65535"
#define SRM_VOD_MSG_VERIFY_COUNT_DEFAULT_S  "10"

/*
 * ===============================
 * ============ FWDMSG ===========
 * ===============================
 */
#define FWDMSG_ENABLE1_DEFAULT  0
#define FWDMSG_ENABLE1_DEFAULT_S  "0"
#define FWDMSG_IP1_DEFAULT_S "192.168.2.100"
#define FWDMSG_PORT1_DEFAULT 11000
#define FWDMSG_PORT1_DEFAULT_S "11000"
#define FWDMSG_MASK1_DEFAULT 0
#define FWDMSG_MASK1_MIN 0
#define FWDMSG_MASK1_MAX 0xFF
#define FWDMSG_MASK1_DEFAULT_S "0"
#define FWDMSG_MASK1_MIN_S "0"
#define FWDMSG_MASK1_MAX_S "0xFF"

#define FWDMSG_ENABLE2_DEFAULT  0
#define FWDMSG_ENABLE2_DEFAULT_S  "0"
#define FWDMSG_IP2_DEFAULT_S "192.168.2.101"
#define FWDMSG_PORT2_DEFAULT 11001
#define FWDMSG_PORT2_DEFAULT_S "11001"
#define FWDMSG_MASK2_DEFAULT 0
#define FWDMSG_MASK2_MIN 0
#define FWDMSG_MASK2_MAX 0xFF
#define FWDMSG_MASK2_DEFAULT_S "0"
#define FWDMSG_MASK2_MIN_S "0"
#define FWDMSG_MASK2_MAX_S "0xFF"

#define FWDMSG_ENABLE3_DEFAULT  0
#define FWDMSG_ENABLE3_DEFAULT_S  "0"
#define FWDMSG_IP3_DEFAULT_S "192.168.2.102"
#define FWDMSG_PORT3_DEFAULT 11002
#define FWDMSG_PORT3_DEFAULT_S "11002"
#define FWDMSG_MASK3_DEFAULT 0
#define FWDMSG_MASK3_MIN 0
#define FWDMSG_MASK3_MAX 0xFF
#define FWDMSG_MASK3_DEFAULT_S "0"
#define FWDMSG_MASK3_MIN_S "0"
#define FWDMSG_MASK3_MAX_S "0xFF"

#define FWDMSG_ENABLE4_DEFAULT  0
#define FWDMSG_ENABLE4_DEFAULT_S  "0"
#define FWDMSG_IP4_DEFAULT_S "192.168.2.103"
#define FWDMSG_PORT4_DEFAULT 11003
#define FWDMSG_PORT4_DEFAULT_S "11003"
#define FWDMSG_MASK4_DEFAULT 0
#define FWDMSG_MASK4_MIN 0
#define FWDMSG_MASK4_MAX 0xFF
#define FWDMSG_MASK4_DEFAULT_S "0"
#define FWDMSG_MASK4_MIN_S "0"
#define FWDMSG_MASK4_MAX_S "0xFF"

#define FWDMSG_ENABLE5_DEFAULT  0
#define FWDMSG_ENABLE5_DEFAULT_S  "0"
#define FWDMSG_IP5_DEFAULT_S "192.168.2.104"
#define FWDMSG_PORT5_DEFAULT 11004
#define FWDMSG_PORT5_DEFAULT_S "11004"
#define FWDMSG_MASK5_DEFAULT 0
#define FWDMSG_MASK5_MIN 0
#define FWDMSG_MASK5_MAX 0xFF
#define FWDMSG_MASK5_DEFAULT_S "0"
#define FWDMSG_MASK5_MIN_S "0"
#define FWDMSG_MASK5_MAX_S "0xFF"

/*
 * ===============================  
 * ============ SNMPD ============
 * ===============================
 */
#define SNMP_AGENT_ADDRESS_MIN        0
#define SNMP_AGENT_ADDRESS_MAX        65535
#define SNMP_AGENT_ADDRESS_DEFAULT    161
#define SNMP_AGENT_ADDRESS_MIN_S      "0"
#define SNMP_AGENT_ADDRESS_MAX_S      "65535"
#define SNMP_AGENT_ADDRESS_DEFAULT_S  "161"


/*
 * ===============================  
 * ============ LOGMGR ===========
 * ===============================
 */

/* Maximum number of days to keep a log file open
 * 0 means no max (size will determine life)
 */
#define LOGMGR_LOG_LIFE_MIN       0
#define LOGMGR_LOG_LIFE_MAX       60
#define LOGMGR_LOG_LIFE_DEFAULT   0
#define LOGMGR_LOG_LIFE_MIN_S     "0"
#define LOGMGR_LOG_LIFE_MAX_S     "60"
#define LOGMGR_LOG_LIFE_DEFAULT_S "0"

/* size in MB for max size of log file 
 * 0 means no max (directory size max determines)
 */
#define LOGMGR_LOG_SIZE_MIN       0
#define LOGMGR_LOG_SIZE_MAX       100
#define LOGMGR_LOG_SIZE_DEFAULT   5
#define LOGMGR_LOG_SIZE_MIN_S     "0"
#define LOGMGR_LOG_SIZE_MAX_S     "100"
#define LOGMGR_LOG_SIZE_DEFAULT_S "5"

/* (RFC5424)level of messages added to system log
 * 7(debug) is the lowest level, 0(emerg) is highest (only emergency messages)
 */
#define LOGMGR_LOG_MIN_PRIORITY_MIN       0
#define LOGMGR_LOG_MIN_PRIORITY_MAX       7
#define LOGMGR_LOG_MIN_PRIORITY_DEFAULT   7
#define LOGMGR_LOG_MIN_PRIORITY_MIN_S     "0"
#define LOGMGR_LOG_MIN_PRIORITY_MAX_S     "7"
#define LOGMGR_LOG_MIN_PRIORITY_DEFAULT_S "7"

#define LOGMGR_LOG_DIR_DEFAULT_S  "/rwflash/I2V/syslog"

/* Size in MB for log directory. */
#define LOGMGR_LOG_DIR_SIZE_MIN       250
#define LOGMGR_LOG_DIR_SIZE_MAX       1000
#define LOGMGR_LOG_DIR_SIZE_DEFAULT   250
#define LOGMGR_LOG_DIR_SIZE_MIN_S     "250"
#define LOGMGR_LOG_DIR_SIZE_MAX_S     "1000"
#define LOGMGR_LOG_DIR_SIZE_DEFAULT_S "250"

/* Directory size threshold limit in MB - this value
 * is used to delete old log files when the directory
 * consumed space is within the threshold of directory
 * size - if the open log file is filling up the space
 * the threshold is ignored but the open log file will
 * be closed and deleted when LogDirSize is reached
 */
#define LOGMGR_LOG_DIR_THRESHOLD_MIN       1
#define LOGMGR_LOG_DIR_THRESHOLD_MAX       5
#define LOGMGR_LOG_DIR_THRESHOLD_DEFAULT   1
#define LOGMGR_LOG_DIR_THRESHOLD_MIN_S     "1"
#define LOGMGR_LOG_DIR_THRESHOLD_MAX_S     "5"
#define LOGMGR_LOG_DIR_THRESHOLD_DEFAULT_S "1"

/* Log files older than this number of days will be
 * deleted every Monday at 12:10 am (RSE requirement)
 */
#define LOGMGR_LOG_DURATION_MIN       1
#define LOGMGR_LOG_DURATION_MAX       365
#define LOGMGR_LOG_DURATION_DEFAULT   28
#define LOGMGR_LOG_DURATION_MIN_S     "1"
#define LOGMGR_LOG_DURATION_MAX_S     "365"
#define LOGMGR_LOG_DURATION_DEFAULT_S "28"

/* if enabled(1) logs will be transmitted using IP config
 * settings below when I2V shuts down
 */
#define LOGMGR_LOG_XMIT_ONHALT_ENABLE_DEFAULT    0
#define LOGMGR_LOG_XMIT_ONHALT_ENABLE_DEFAULT_S "0"

/* if enabled(1) will use IPv4 server manager instead of ipv6. */
#define LOGMGR_LOG_XMIT_USE_IPV4_DEFAULT    0
#define LOGMGR_LOG_XMIT_USE_IPV4_DEFAULT_S "0"

/* 5 minute intervals for auto transfer of log to management
 * server; 288 = 24 hours, 0 = disabled; 2000 is about once/wk
 * when this is enabled (not 0) user defined must have proper
 * ssh keys created for scp to work - passwords are not supported
 */
#define LOGMGR_LOG_XMIT_INTERVAL_MIN         0
#define LOGMGR_LOG_XMIT_INTERVAL_MAX         2000
#define LOGMGR_LOG_XMIT_INTERVAL_DEFAULT     0
#define LOGMGR_LOG_XMIT_INTERVAL_MIN_S      "0"
#define LOGMGR_LOG_XMIT_INTERVAL_MAX_S      "2000"
#define LOGMGR_LOG_XMIT_INTERVAL_DEFAULT_S  "0"

/* MB value for max file size of interface logs */
#define LOGMGR_IFACE_LOG_SIZE_MIN         1
#define LOGMGR_IFACE_LOG_SIZE_MAX         40
#define LOGMGR_IFACE_LOG_SIZE_DEFAULT     40
#define LOGMGR_IFACE_LOG_SIZE_MIN_S       "1"
#define LOGMGR_IFACE_LOG_SIZE_MAX_S       "40"
#define LOGMGR_IFACE_LOG_SIZE_DEFAULT_S   "40"

/* value in seconds for max interface log lifetime
 * a value of 86400 is 24 hours, max value is about 2 days
 */
#define LOGMGR_IFACE_LIFE_MIN         3600
#define LOGMGR_IFACE_LIFE_MAX         172800
#define LOGMGR_IFACE_LIFE_DEFAULT     172800
#define LOGMGR_IFACE_LIFE_MIN_S       "3600"
#define LOGMGR_IFACE_LIFE_MAX_S       "172800"
#define LOGMGR_IFACE_LIFE_DEFAULT_S   "172800"
   
/* IPv6 prefix for backend server receiving logs */
#define LOGMGR_SVR_MGR_IPV6_PREFIX_DEFAULT_S  "2001:0470:E0FB:9999"

/* IPv6 identifier (second part of address) */   
#define LOGMGR_SVR_MGR_IPV6_ID_DEFAULT_S "0:0:0:1f"

/* IP address, user & dir for IPv4 server. */
#define LOGMGR_SVR_MGR_IPV4_DEFAULT_S "192.168.3.100"         
#define LOGMGR_SVR_MGR_USER_DEFAULT_S "root"
#define LOGMGR_SVR_MGR_DIR_DEFAULT_S "/logs"

#endif  /* CONF_TABLE_H */ 

