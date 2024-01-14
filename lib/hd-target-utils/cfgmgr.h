#ifndef CFGMGR_H
#define CFGMGR_H

#define MASTERKEY       "masterkey"
// #define MASTERKEY       "xXo$vq_.?l#3~"
// #define SYSTEMMASTERKEY "xXo\\$vq_.?l#3~"

/* management operations */
//star wars alert
#define RESETDFLTS         "r2d"
#define RESETDFLTSBUTSAVENET  "bb8"
#define RESETCUSTOMER    "r2c"
#define CUST_CFG_BLOB       "/rwflash/customer_config_blob"
//zrm #define CFGPREPPOSTPROC    "c3p"
/* is this a cfgmgr option? this should be in diagmgr; keep temporarily */
#define STOPI2VTERMHLTH    "sith"
#define HLTHACTVTYNIX      "han"

/* network operations; making this portable now in preparation */
#define RSU_DHCP_ENABLE    "rde"
#define RSU_GET_MODE       "rgm"
#define RSU_GET_IP         "rgi"
#define RSU_GET_NETMASK    "rgn"
#define RSU_SET_STATIC_IP  "rss"
#define RSU_SET_NETMASK    "rsn"
#define RSU_STATIC_ENABLE  "rse"
#define OBU_DHCP_ENABLE    "o_de"
#define OBU_GET_MODE       "o_gm"
#define OBU_GET_IP         "o_gi"
#define OBU_GET_NETMASK    "o_gn"
#define OBU_SET_STATIC_IP  "o_ss"
#define OBU_SET_NETMASK    "o_sn"
#define OBU_STATIC_ENABLE  "o_se"
#define RSU_SET_I2V        "rsi"
#define RSU_SET_HEALTH     "rsr"
#define RSU_SET_SYSTEM_MON "rssm"
#define RSU_GET_HTTP_PORT  "rghp"
#define RSU_GET_HTTPS_PORT "rgsp"
#define RSU_SET_HTTP_PORT  "rshp"
#define RSU_SET_HTTPS_PORT "rssp"
#define TSCBM_UPDATE       "tscu"
#define RSU_GET_MTU        "rgmt"
#define RSU_SET_MTU        "rsmt"
#define RSU_GET_VLAN_EN    "rgve"
#define RSU_SET_VLAN_EN    "rsve"
#define RSU_GET_VLAN_IP    "rgva"
#define RSU_SET_VLAN_IP    "rsva"
#define RSU_GET_VLAN_MASK  "rgvm"
#define RSU_SET_VLAN_MASK  "rsvm"
#define RSU_GET_VLAN_ID    "rgvi"
#define RSU_SET_VLAN_ID    "rsvi"

#define CFG_STATIC         "STATIC"
#define CFG_DHCP           "DHCP"
#define CFG_ON             "ON"
#define CFG_OFF            "OFF"

#endif /* CFGMGR_H */

