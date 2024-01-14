/****************************************************************************
 *                                                                          *
 *  File Name: i2v_ipb_data.h                                               *
 *  Author:                                                                 *
 *      DENSO International America, Inc.                                   *
 *      North America Research Laboratory, California Office                *
 *      3252 Business Park Drive                                            *
 *      Vista, CA 92081                                                     *
 *                                                                          *
 ****************************************************************************/

#ifndef I2V_IPB_DATA_H
#define I2V_IPB_DATA_H

#include "wsu_shm_inc.h"
#include "i2v_general.h"
#include "ris_struct.h"

#define LENGTH_IPV4 4
/* RSU spec defines value as 4 bytes + 1 for null */
#define LENGTH_IPB_PSC  5

typedef struct {
    wsu_shm_hdr_t h;
    uint8_t    radioNum;
    uint8_t    wsaPriority;
    uint8_t    wsaTxRate;
    uint8_t    dataRate;
    float32_t  txPwr; /* 0 - 23 dBm; 0.5 increments */
    float32_t  wsaPwr;/* 0 - 23 dBm; 0.5 increments */
    bool_t     forceIPBCfgPwr;
    bool_t     secDebug;
    bool_t     usegps;
    bool_t     isContinuous;
    bool_t     isNotCCH;
    uint8_t    sattime;
    uint8_t    minSats;
    uint16_t   gpstime;
    uint32_t   psid;
    uint8_t    channelNumber;
    uint8_t    wsaChannel;
    float64_t  latitude;
    float64_t  longitude;
    int16_t    elevation;
    uint8_t    confidence;
    char_t     ipv6addr[LENGTH_IPV6]; 
    uint16_t   port;
    char_t     svcMac[LENGTH_MAC];
    bool_t     uselocal;
    bool_t     enableBridge;
    bool_t     ipbBridgeOnce;
    bool_t     ipv4bridge;
    uint8_t    bridgeClientNum;
    uint8_t    cnxnTOSec;
    bool_t     delayRcvr;
    char_t     ipv4addr[LENGTH_IPV4];
    uint16_t   ipv4port;
    char_t     srvrIpv6addr[LENGTH_IPV6]; 
    char_t     pscval[I2V_CFG_MAX_STR_LEN];
    bool_t     enableEDCA;
    char_t     advertiseID[I2V_CFG_MAX_STR_LEN];
} ipbCfgItemsT;

#endif /* I2V_IPB_DATA_H */
