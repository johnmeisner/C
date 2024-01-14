/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Filename: nscfg.c                                                          */
/*  Purpose: Handles NS configuration information for radioServices           */
/*                                                                            */
/* Copyright (C) 2019 DENSO International America, Inc.  All Rights Reserved. */
/*                                                                            */
/* Revision History:                                                          */
/*                                                                            */
/* Date        Author        Comments                                         */
/* -------------------------------------------------------------------------- */
/* 2019-05-07  VROLLINGER    Initial revision.                                */
/*                                                                            */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/* Include Files                                                              */
/*----------------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/param.h>
#include "nscfg.h"
#include "wme.h"
#include "type2str.h"
#include "rsu_defs.h"
#include "alsmi.h"
#include "i2v_util.h"
#include "nscfg.h"

/* mandatory logging defines */
#define MY_ERR_LEVEL   LEVEL_PRIV    /* from i2v_util.h */
#define MY_NAME        "nscfg"

extern bool_t  cv2x_started;
/*----------------------------------------------------------------------------*/
/* Constants                                                                  */
/*----------------------------------------------------------------------------*/
#ifdef CHINA_FREQS_ONLY
static uint8_t g_valid_ch_nums[] = {147, 149, 151, 153, 155, 157, 159, 161, 163, 165, 167, 169};
#elif defined(US_11A_FREQS)
static uint8_t g_valid_ch_nums[] = {178, 172, 173, 174, 175, 176, 177, 179, 180, 181, 182, 183, 184, 156, 164, 168, 76, 84, 88, 160, 162};
#else
static uint8_t g_valid_ch_nums[] = {178, 172, 173, 174, 175, 176, 177, 179, 180, 181, 182, 183, 184, 156, 164, 168};
#endif

/*----------------------------------------------------------------------------*/
/* Typedefs                                                                   */
/*----------------------------------------------------------------------------*/
typedef struct {
    uint8_t  TxAntenna;                    // 0 = Auto, 1 = Tx Antenna 1, 2 = Tx Antenna 2
    uint8_t  RxAntenna;                    // 1 = Rx Antenna 1, 2 = Rx Antenna 2
    uint8_t  RxDiversity;                  // 0 = Disable Rx diversity, 1 = Enable Rx diversity
    uint8_t  RadioMacAddr[LENGTH_MAC];     // Mac address to be configured
    uint64_t MK5RadioMacAddr;              // Keep MK5 format to prevent retranslation every pktTx.
    uint8_t  RadioMacAddrOrig[LENGTH_MAC]; // Original Mac address to be configured
} ChanConfigType;

/*
 * Notes on Porting to QNX and to be used with the MK5-LLC.
 *   * There the Configurable values and their meanings for the MK5 Radio/Cohda-LLC-API
 *     are different in some respects from the WSU/Atheros based Radio under Linux.
 *   * The(initial) general Idea is that the RIS API will be the same, possibly
 *     expanded in the future.
 *   * nscfg.c configuration values stored will be stored based on the RIS configuration
 *     values.  These values will be converted as needed when they are sent used with the
 *     LLC/MK5 radio.
 *   * A number of configuration values in RIS for the WSU/Atheros based radio under
 *     Linux were set/get directly with the CCL, MLME, &  Atheros driver, directly
 *     using sysctrl (/proc/dev/...).  For the LLC/MK5 these have to all be sent to the
 *     NS layer which will set/get the values using the MK5 LLC API as appropriate.
 *   * The WSU had  2 radios, while the MK5 has 2 radios, each with 2 channels
 *     providing 4 radio configurations to be maintained instead of 2.
 *   *
 */
typedef struct {
    bool_t         cv2x_enable;
    roleType       station_role;
    uint32_t       repeats;
    uint8_t        cch_num_radio0;
    uint8_t        sch_num_radio0;
    uint8_t        cch_num_radio1;
    uint8_t        sch_num_radio1;
    bool_t         sch_adaptable;
    dataRateType   sch_datarate;
    uint32_t       sch_txpwrlevel;
    uint32_t       service_inactivity_interval;
    uint32_t       wsa_timeout_interval;
    uint32_t       wsa_timeout_interval2;
    uint8_t        wsa_channel_interval;
    uint8_t        wsa_priority;
    dataRateType   wsa_datarate;
    uint8_t        wsa_txpwrlevel;
    uint8_t        p2p_channel_interval;
    uint8_t        p2p_priority;
    dataRateType   p2p_datarate;
    uint8_t        p2p_txpwrlevel;
    uint8_t        ipv6_priority;
    dataRateType   ipv6_datarate;
    uint8_t        ipv6_txpwrlevel;
    bool_t         ipv6_adaptable;
    bool_t         ipv6_providermode;
    rsRadioType    ipv6_radioType;
    uint8_t        ipv6_radioNum;
    int16_t        conducted_2_radiated_power_offset_radio_0;
    int16_t        conducted_2_radiated_power_offset_radio_1;
    uint32_t       rx_queue_threshold;
    bool_t         change_mac_address_enable;
    bool_t         rcv_EU_from_eth_enable;
    uint16_t       rcv_EU_from_eth_port_num;
    uint8_t        sspLen;
    uint8_t        ssp[MAX_SSP_DATA];
    bool_t         cv2x_pwr_ctrl_enable;
    bool_t         discard_unsecure_msg;
    bool_t         discard_ver_fail_msg;
    uint32_t       primary_service_id;
    uint32_t       secondary_service_id;
    uint32_t       alsmi_debug_log_enable;
    uint32_t       alsmi_sign_verify_enable;
    uint32_t       i2v_syslog_enable;
    /*
     * The first array index is for the radio number. The second array index is
     * for the MK5Channel (SCH or CCH).
     */
    ChanConfigType ChanCfg[MAX_RADIOS][2];
} CfgType;

/*----------------------------------------------------------------------------*/
/* Function prototypes                                                        */
/*----------------------------------------------------------------------------*/
static rsResultCodeType setCV2XEnable(bool_t enable);
static rsResultCodeType setStationRole(roleType station_role);
static rsResultCodeType setRepeats(uint32_t repeats);
static rsResultCodeType setCCHNumber(uint8_t radioNum, uint8_t cch_num);
static rsResultCodeType setSCHAdaptable(bool_t sch_adaptable);
static rsResultCodeType setSCHDataRate(dataRateType sch_datarate);
static rsResultCodeType setSCHTxPowerLevel(uint32_t sch_txpwrlevel);
static rsResultCodeType setServiceInactivityInterval(uint32_t service_inactivity_interval);
static rsResultCodeType setWSATimeoutInterval(uint32_t wsa_timeout_interval);
static rsResultCodeType setWSATimeoutInterval2(uint32_t wsa_timeout_interval2);
static rsResultCodeType setWSAChannelInterval(uint8_t channelInterval);
static rsResultCodeType setWSAPriority(uint8_t priority);
static rsResultCodeType setWSADataRate(dataRateType datarate);
static rsResultCodeType setWSATxPowerLevel(uint8_t txpwrlevel);
static rsResultCodeType setP2PChannelInterval(uint8_t channelInterval);
static rsResultCodeType setP2PPriority(uint8_t priority);
static rsResultCodeType setP2PDataRate(dataRateType datarate);
static rsResultCodeType setP2PTxPowerLevel(uint8_t txpwrlevel);
static rsResultCodeType setIPv6Priority(uint8_t priority);
static rsResultCodeType setIPv6DataRate(dataRateType datarate);
static rsResultCodeType setIPv6TxPowerLevel(uint8_t txpwrlevel);
static rsResultCodeType setIPv6Adaptable(bool_t adaptable);
static rsResultCodeType setIPv6ProviderMode(bool_t providermode);
static rsResultCodeType setIPv6RadioType(rsRadioType radioType);
static rsResultCodeType setIPv6RadioNum(uint8_t radioNum);
static rsResultCodeType setConducted2RadiatedPowerOffset(uint8_t radio, int16_t offset);
static rsResultCodeType setRxQueueThreshold(uint32_t rx_queue_threshold);
static rsResultCodeType setChangeMacAddressEnable(bool_t changeMacAddressEnable);
static rsResultCodeType setRcvEUFromEthEnable(bool_t rcv_EU_from_eth_enable);
static rsResultCodeType setRcvEUFromEthPortNum(uint16_t rcv_EU_from_eth_port_num);
static rsResultCodeType setSSP(uint8_t *ssp, uint8_t sspLen);
static rsResultCodeType setCV2XPwrCtrlEnable(bool_t enable);
static rsResultCodeType setDiscardUnsecureMsg(bool_t discardUnsecureMsg);
static rsResultCodeType setDiscardVerFailMsg(bool_t discardVerFailMsg);
static rsResultCodeType setPrimaryServiceId(uint32_t primaryServiceId);
static rsResultCodeType setSecondaryServiceId(uint32_t secondaryServiceId);
static rsResultCodeType setALSMIDebugLogEnable(uint32_t ALSMIDebugLogEnable);
static rsResultCodeType setALSMISignVerifyEnable(uint32_t ALSMISignVerifyEnable);
static rsResultCodeType setI2vSyslogEnable(uint32_t I2vSyslogEnable);
bool_t   getRcvEUFromEthEnable(void);
uint16_t getRcvEUFromEthPortNum(void);
int16_t  getConducted2RadiatedPowerOffset(uint8_t radioNum);

/*----------------------------------------------------------------------------*/
/* Local variables                                                            */
/*----------------------------------------------------------------------------*/
/* VJR WILLBEREVISITED Most if not all of the following variables apply to the
 * DSRC radio only. There may be a need to somehow have these variables apply
 * to the CV2X radio as well. */
static CfgType cfg =
{
    .cv2x_enable                 = TRUE,
    .station_role                = ROLE_USER,
    .repeats                     = MIN_REPEATS,
    .cch_num_radio0              = CCH_NUM_DEFAULT,
    .cch_num_radio1              = CCH_NUM_DEFAULT,
    .sch_num_radio0              = 0,                /* not SCH_NUM_DEFAULT, SCH must be explicitely configured */
    .sch_num_radio1              = 0,                /* not SCH_NUM_DEFAULT, SCH must be explicitely configured */
    .sch_adaptable               = FALSE,
    .sch_datarate                = DR_3_MBPS,
    .sch_txpwrlevel              = MAX_TX_PWR_LEVEL, /* NOTE: Currently constant set to MK5-Radio's value */
    .service_inactivity_interval = (SRVC_INACTIVITY_INTRVL * HZ) / 1000,
    .wsa_timeout_interval        = (WSA_TIMEOUT_INTRVL * HZ) / 1000,
    .wsa_timeout_interval2       = (WSA_TIMEOUT_INTRVL2 * HZ) / 1000,
    .wsa_channel_interval        = 0,
    .wsa_priority                = 5,
    .wsa_datarate                = DR_6_MBPS,
    .wsa_txpwrlevel              = 20,
    .p2p_channel_interval        = 0,
    .p2p_priority                = 3,
    .p2p_datarate                = DR_6_MBPS,
    .p2p_txpwrlevel              = 20,
    .ipv6_priority               = 3,
    .ipv6_datarate               = DR_6_MBPS,
    .ipv6_txpwrlevel             = 20,
    .ipv6_adaptable              = FALSE,
    .ipv6_providermode           = FALSE,
    .ipv6_radioType              = RT_CV2X,
    .ipv6_radioNum               = 1,
    .rx_queue_threshold          = 10000,
    .change_mac_address_enable   = TRUE,
    .rcv_EU_from_eth_enable      = FALSE,
    .rcv_EU_from_eth_port_num    = 4210,
    .sspLen                      = 0,
    .cv2x_pwr_ctrl_enable        = FALSE,
    .discard_unsecure_msg        = FALSE,
    .discard_ver_fail_msg        = FALSE,
    .primary_service_id          = 0x20,
    .secondary_service_id        = 0x21,
    .alsmi_debug_log_enable      = 0,
    .alsmi_sign_verify_enable    = 0,
    .i2v_syslog_enable           = 0,

    /*
     * The only time both MK5Channel 0 & 1 could be used at the same time is
     * during switched mode, and while there are separate configurations for
     * each MK5Channel, currently both would be set to the same MACAddress.
     *
     * The fields below are:  .ChanCfg[MK5RadioNum][MK5ChannelNum].*
     */
     .ChanCfg = {
                  [0] ={
                         [0]= {       /*radio0-channel0*/
                                .TxAntenna = 0,
                                .RxAntenna = 0,
                                .RxDiversity = 0,
                                .RadioMacAddr = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
                                .MK5RadioMacAddr = 0x0ULL,
                                .RadioMacAddrOrig = { 0xAA, 0x00, 0x00, 0x00, 0x00, 0x00 }
                              },
                         [1]= {      /*radio0-channel1*/
                                .TxAntenna = 0,
                                .RxAntenna = 0,
                                .RxDiversity = 0,
                                .RadioMacAddr = { 0xAA, 0x00, 0x00, 0x00, 0x00, 0x00 },
                                .MK5RadioMacAddr = 0x0ULL,
                                .RadioMacAddrOrig = { 0xAA, 0x00, 0x00, 0x00, 0x00, 0x00 }
                              }
                       },
                  [1] ={
                         [0]= {     /*radio1-channel0*/
                                .TxAntenna = 0,
                                .RxAntenna = 0,
                                .RxDiversity = 0,
                                .RadioMacAddr = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
                                .MK5RadioMacAddr = 0x0ULL,
                                .RadioMacAddrOrig = { 0xAA, 0x00, 0x00, 0x00, 0x00, 0x01 }
                              },
                         [1]= {    /*radio1-channel1*/
                                .TxAntenna = 0,
                                .RxAntenna = 0,
                                .RxDiversity = 0,
                                .RadioMacAddr = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
                                .MK5RadioMacAddr = 0x0ULL,
                                .RadioMacAddrOrig = { 0xAA, 0x00, 0x00, 0x00, 0x00, 0x01 }
                              }
                       }
                }
};

/* Set the following variable to TRUE when parsing the config file. In this
 * way, only the values of the variables will be set without performing any
 * other action (e.g. actually enabling the radio if setting either CV2X_ENABLE
 * pr DSRC_ENABLE to TRUE). This variable should be set to FALSE otherwise. */
bool_t parsingConfigFile = FALSE;

/*----------------------------------------------------------------------------*/
/* Functions                                                                  */
/*----------------------------------------------------------------------------*/

void initNsCfg(uint32_t debugEnable)
{
  parsingConfigFile = FALSE;
  cfg.cv2x_enable = FALSE;
  cfg.i2v_syslog_enable = 0;
}

/**
** Function:  nsSetCfgValue
** @brief  Set the value of an NS configuration item based on the config ID
** @brief  and the value.
** @param  cfgId -- [input]The config ID
** @param  value -- [input]The value to set
** @return RS_SUCCESS for success, error code otherwise
**
** Details: Checks cfgId. Then based on its value, calls the proper routine to
**          set the value.
**/
static rsResultCodeType nsSetCfgValue(cfgIdType cfgId, uint32_t value)
{
    switch (cfgId) {
    case CFG_ID_CV2X_ENABLE:
        return setCV2XEnable(value);

    case CFG_ID_STATION_ROLE:
        return setStationRole(value);

    case CFG_ID_REPEATS:
        return setRepeats(value);

    case CFG_ID_CCH_NUM_RADIO_0:
        return setCCHNumber(0, value);

    case CFG_ID_CCH_NUM_RADIO_1:
        return setCCHNumber(1, value);

    case CFG_ID_SCH_NUM_RADIO_0:
        return setSCHNumber(0, value);

    case CFG_ID_SCH_NUM_RADIO_1:
        return setSCHNumber(1, value);

    case CFG_ID_SCH_ADAPTABLE:
        return setSCHAdaptable((bool_t)value);

    case CFG_ID_SCH_DATARATE:
        return setSCHDataRate(value);

    case CFG_ID_SCH_TXPWRLEVEL:
        return setSCHTxPowerLevel(value);

    case CFG_ID_SERVICE_INACTIVITY_INTERVAL:
        return setServiceInactivityInterval(value);

    case CFG_ID_WSA_TIMEOUT_INTERVAL:
        return setWSATimeoutInterval(value);

    case CFG_ID_WSA_TIMEOUT_INTERVAL2:
        return setWSATimeoutInterval2(value);

    case CFG_ID_RX_QUEUE_THRESHOLD:
        return setRxQueueThreshold(value);

    case CFG_ID_WSA_CHANNEL_INTERVAL:
        return setWSAChannelInterval(value);

    case CFG_ID_WSA_PRIORITY:
        return setWSAPriority(value);

    case CFG_ID_WSA_DATARATE:
        return setWSADataRate(value);

    case CFG_ID_WSA_TXPWRLEVEL:
        return setWSATxPowerLevel(value);

    case CFG_ID_P2P_CHANNEL_INTERVAL:
        return setP2PChannelInterval(value);

    case CFG_ID_P2P_PRIORITY:
        return setP2PPriority(value);

    case CFG_ID_P2P_DATARATE:
        return setP2PDataRate(value);

    case CFG_ID_P2P_TXPWRLEVEL:
        return setP2PTxPowerLevel(value);

    case CFG_ID_IPV6_PRIORITY:
        return setIPv6Priority(value);

    case CFG_ID_IPV6_DATARATE:
        return setIPv6DataRate(value);

    case CFG_ID_IPV6_TXPWRLEVEL:
        return setIPv6TxPowerLevel(value);

    case CFG_ID_IPV6_ADAPTABLE:
        return setIPv6Adaptable((bool_t)value);

    case CFG_ID_IPV6_PROVIDER_MODE:
        return setIPv6ProviderMode((bool_t)value);

    case CFG_ID_IPV6_RADIO_TYPE:
        return setIPv6RadioType(value);

    case CFG_ID_IPV6_RADIO_NUM:
        return setIPv6RadioNum(value);

    case CFG_ID_CONDUCTED_2_RADIATED_POWER_OFFSET_RADIO_0:
        return setConducted2RadiatedPowerOffset(0, value);

    case CFG_ID_CONDUCTED_2_RADIATED_POWER_OFFSET_RADIO_1:
        return setConducted2RadiatedPowerOffset(1, value);

    case CFG_ID_CHANGE_MAC_ADDRESS_ENABLE:
        return setChangeMacAddressEnable((bool_t)value);

    case CFG_ID_RCV_EU_FROM_ETH_ENABLE:
        return setRcvEUFromEthEnable((bool_t)value);

    case CFG_ID_RCV_EU_FROM_ETH_PORT_NUM:
        return setRcvEUFromEthPortNum(value);

    case CFG_ID_CV2X_PWR_CTRL_ENABLE:
        return setCV2XPwrCtrlEnable(value);

    case CFG_ID_DISCARD_UNSECURE_MSG:
        return setDiscardUnsecureMsg((bool_t)value);

    case CFG_ID_DISCARD_VERFAIL_MSG:
        return setDiscardVerFailMsg((bool_t)value);

    case CFG_ID_PRIMARY_SERVICE_ID:
        return setPrimaryServiceId(value);

    case CFG_ID_SECONDARY_SERVICE_ID:
        return setSecondaryServiceId(value);

    case CFG_ID_ALSMI_DEBUG_LOG_ENABLE:
        return setALSMIDebugLogEnable(value);

    case CFG_ID_ALSMI_SIGN_VERIFY_ENABLE:
        return setALSMISignVerifyEnable(value);

    case CFG_ID_I2V_SYSLOG_ENABLE:
        return setI2vSyslogEnable(value);

    default:
        return RS_EINVID;
    }
}

/**
** Function:  moveLeft1
** @brief  Deletes the first character of a string and moves the remaining
** @brief  characters to the left 1 place.
** @param  ptr -- [input/output]The string to process
** @return void
**
** Details: Deletes the first character of a string and moves the remaining
**          characters to the left 1 place using byte pointers 1 byte at a
**          time. Note that you should not use strcpy() or memcpy() or else
**          data corruption can result.
**/
static void moveLeft1(char *ptr)
{
    char *in  = ptr + 1;
    char *out = ptr;

    while (*in != '\0')  {
        *out++ = *in++;
    }

    *out = '\0';
}

/**
** Function:  nsParseConfigFile1
** @brief  Parses the /rwflash/configs/nsconfig.conf file and initializes
** @brief  the nsconfig variables with values from that file.
** @brief  Does everything except the EDCA values (see nsParseConfigFile2).
** @brief  This must be executed before DSRC is initialized.
** @return void
**
** Details: Opens the file /rwflash/configs/nsconfig.conf for reading; returns
**          if failure. Then it goes through reading lines from the file. For
**          each line, it is parsed to see if it is of the form "var=val"
**          taking comments and white spaces into account. All lines not of that
**          form are ignored. If we have a proper line, its variable value is
**          calcluated and the proper "set" routine is called via a call to
**          nsSetCfgValue().
**
**          This routine is called from rsk.c early when radioServices is first
**          brought up.
**/

#if defined(MY_UNIT_TEST)
#define NS_CFG_LOCATION  "./nsconfig.conf"
#else
#define NS_CFG_LOCATION "/rwflash/configs/nsconfig.conf"
#endif
void nsParseConfigFile1(void)
{
    FILE      *f;
    char       buf[150];
    char      *ptr;
    char      *var_str;
    char      *val_str;
    uint32_t        i;
    float32_t  fvalue;
    #define TYPE_BOOL  0
    #define TYPE_UINT  1
    #define TYPE_FLOAT 2
    #define TYPE_EDCA  3
    struct {
        char      *cfgIdStr;
        uint8_t    type;
        cfgIdType  cfgId;
    } cfgItems[] =
    {
        {"CV2X_ENABLE",                               TYPE_BOOL,  CFG_ID_CV2X_ENABLE},
        {"STATION_ROLE",                              TYPE_UINT,  CFG_ID_STATION_ROLE},
        {"REPEATS",                                   TYPE_UINT,  CFG_ID_REPEATS},
        {"CCH_NUM_RADIO_0",                           TYPE_UINT,  CFG_ID_CCH_NUM_RADIO_0},
        {"CCH_NUM_RADIO_1",                           TYPE_UINT,  CFG_ID_CCH_NUM_RADIO_1},
        {"SCH_TXPWRLEVEL",                            TYPE_UINT,  CFG_ID_SCH_TXPWRLEVEL},
        {"SCH_ADAPTABLE",                             TYPE_BOOL,  CFG_ID_SCH_ADAPTABLE},
        {"SCH_DATARATE",                              TYPE_UINT,  CFG_ID_SCH_DATARATE},
        {"SERVICE_INACTIVITY_INTERVAL",               TYPE_UINT,  CFG_ID_SERVICE_INACTIVITY_INTERVAL},
        {"WSA_TIMEOUT_INTERVAL",                      TYPE_UINT,  CFG_ID_WSA_TIMEOUT_INTERVAL},
        {"WSA_TIMEOUT_INTERVAL2",                     TYPE_UINT,  CFG_ID_WSA_TIMEOUT_INTERVAL2},
        {"RX_QUEUE_THRESHOLD",                        TYPE_UINT,  CFG_ID_RX_QUEUE_THRESHOLD},
        {"WSA_DATARATE",                              TYPE_UINT,  CFG_ID_WSA_DATARATE},
        {"WSA_TXPWRLEVEL",                            TYPE_UINT,  CFG_ID_WSA_TXPWRLEVEL},
        {"WSA_CHANNEL_INTERVAL",                      TYPE_UINT,  CFG_ID_WSA_CHANNEL_INTERVAL},
        {"WSA_PRIORITY",                              TYPE_UINT,  CFG_ID_WSA_PRIORITY},
        {"P2P_DATARATE",                              TYPE_UINT,  CFG_ID_P2P_DATARATE},
        {"P2P_TXPWRLEVEL",                            TYPE_UINT,  CFG_ID_P2P_TXPWRLEVEL},
        {"P2P_CHANNEL_INTERVAL",                      TYPE_UINT,  CFG_ID_P2P_CHANNEL_INTERVAL},
        {"P2P_PRIORITY",                              TYPE_UINT,  CFG_ID_P2P_PRIORITY},
        {"IPV6_RADIO_TYPE",                           TYPE_UINT,  CFG_ID_IPV6_RADIO_TYPE},
        {"IPV6_RADIO_NUM",                            TYPE_UINT,  CFG_ID_IPV6_RADIO_NUM},
        {"IPV6_TXPWRLEVEL",                           TYPE_UINT,  CFG_ID_IPV6_TXPWRLEVEL},
        {"IPV6_ADAPTABLE",                            TYPE_BOOL,  CFG_ID_IPV6_ADAPTABLE},
        {"IPV6_DATARATE",                             TYPE_UINT,  CFG_ID_IPV6_DATARATE},
        {"IPV6_PRIORITY",                             TYPE_UINT,  CFG_ID_IPV6_PRIORITY},
        {"IPV6_PROVIDER_MODE",                        TYPE_BOOL,  CFG_ID_IPV6_PROVIDER_MODE},
        {"CONDUCTED_2_RADIATED_POWER_OFFSET_RADIO_0", TYPE_FLOAT, CFG_ID_CONDUCTED_2_RADIATED_POWER_OFFSET_RADIO_0},
        {"CONDUCTED_2_RADIATED_POWER_OFFSET_RADIO_1", TYPE_FLOAT, CFG_ID_CONDUCTED_2_RADIATED_POWER_OFFSET_RADIO_1},
        {"CHANGE_MAC_ADDRESS_ENABLE",                 TYPE_BOOL,  CFG_ID_CHANGE_MAC_ADDRESS_ENABLE},
        {"RCV_EU_FROM_ETH_ENABLE",                    TYPE_BOOL,  CFG_ID_RCV_EU_FROM_ETH_ENABLE},
        {"RCV_EU_FROM_ETH_PORT_NUM",                  TYPE_UINT,  CFG_ID_RCV_EU_FROM_ETH_PORT_NUM},
        {"CV2X_PWR_CTRL_ENABLE",                      TYPE_BOOL,  CFG_ID_CV2X_PWR_CTRL_ENABLE},
        {"DISCARD_UNSECURE_MSG",                      TYPE_BOOL,  CFG_ID_DISCARD_UNSECURE_MSG},
        {"DISCARD_VERFAIL_MSG",                       TYPE_BOOL,  CFG_ID_DISCARD_VERFAIL_MSG},
        {"PRIMARY_SERVICE_ID",                        TYPE_UINT,  CFG_ID_PRIMARY_SERVICE_ID},
        {"SECONDARY_SERVICE_ID",                      TYPE_UINT,  CFG_ID_SECONDARY_SERVICE_ID},
        {"ALSMI_DEBUG_LOG_ENABLE",                    TYPE_UINT,  CFG_ID_ALSMI_DEBUG_LOG_ENABLE},
        {"ALSMI_SIGN_VERIFY_ENABLE",                  TYPE_UINT,  CFG_ID_ALSMI_SIGN_VERIFY_ENABLE},
        {"I2V_SYSLOG_ENABLE",                         TYPE_UINT,  CFG_ID_I2V_SYSLOG_ENABLE},
    };
    #define NUM_CFG_ITEMS (sizeof(cfgItems) / sizeof(cfgItems[0]))

    /* Open the configuration file */
    if ((f = fopen(NS_CFG_LOCATION, "r")) == NULL) {
        return;
    }

    parsingConfigFile = TRUE;
    //What if not EOL? How does the parsing recover? 
    while (fgets(buf, 150, f)) {
        /* Remove white space */
        while ((ptr = strpbrk(buf, " \t\n\r")) != NULL) {
            moveLeft1(ptr);
        }

        /* Ignore comment lines */
        if (buf[0] == '#') {
            continue;
        }

        /* Identify and remove comments from line */
        if ((ptr = strrchr(buf, '#')) != NULL) {
            *ptr = '\0';
        }

        /* Be sure line contains an equals sign */
        if ((ptr = strrchr(buf, '=')) == NULL) {
            continue;
        }

        *ptr = '\0';
        var_str = buf;
        val_str = ptr + 1;

        /* Identify Configuration Item(s) */
        for (i = 0; i < NUM_CFG_ITEMS; i++) {
            if (strncmp(var_str, cfgItems[i].cfgIdStr,
                        strlen(cfgItems[i].cfgIdStr)) == 0) {
                switch(cfgItems[i].type) {
                case TYPE_BOOL:
                    nsSetCfgValue(cfgItems[i].cfgId,
                                  (atoi(val_str) == 0) ? 0 : 1);
                    break;

                case TYPE_UINT:
                    nsSetCfgValue(cfgItems[i].cfgId, (uint32_t)atoi(val_str));
                    break;

                case TYPE_FLOAT:
                    fvalue = (float32_t)atof(val_str);
                    nsSetCfgValue(cfgItems[i].cfgId, (uint32_t)fvalue);
                    break;
                }

                break;
            }
        }
    } // while (fgets(buf, 150, f))

    parsingConfigFile = FALSE;
    fclose(f);
}


/**
** Function:  nsGetCfg
** @brief  Get the value of an NS configuration item
** @param  cfg -- [input/output]The cfgId field of cfg specifies which
**                configuration item to get. The result is stored in cfg.
** @return RS_SUCCESS for success, error code otherwise
**
** Details: Checks the cfgId field. Then based on its value, calls the proper
**          routine to get the value. The value is stored in the proper cfg->u.
**          field.
**/
rsResultCodeType nsGetCfg(cfgType *cfg)
{
    switch (cfg->cfgId) {
    case CFG_ID_CV2X_ENABLE:
        cfg->u.CV2XEnable = getCV2XEnable();
        break;

    case CFG_ID_STATION_ROLE:
        cfg->u.StationRole = getStationRole();
        break;

    case CFG_ID_REPEATS:
        cfg->u.Repeats = getRepeats();
        break;

    case CFG_ID_CCH_NUM_RADIO_0:
        cfg->u.CCHNum = getCCHNumber(0);
        break;

    case CFG_ID_CCH_NUM_RADIO_1:
        cfg->u.CCHNum = getCCHNumber(1);
        break;

    case CFG_ID_SCH_NUM_RADIO_0:
        cfg->u.SCHNum = getSCHNumber(0);
        break;

    case CFG_ID_SCH_NUM_RADIO_1:
        cfg->u.SCHNum = getSCHNumber(1);
        break;

    case CFG_ID_SCH_ADAPTABLE:
        cfg->u.Adaptable = getSCHAdaptable();
        break;

    case CFG_ID_SCH_DATARATE:
        cfg->u.DataRate = getSCHDataRate();
        break;

    case CFG_ID_SCH_TXPWRLEVEL:
        cfg->u.TxPwrLevel = getSCHTxPowerLevel();
        break;

    case CFG_ID_SERVICE_INACTIVITY_INTERVAL:
        cfg->u.ServiceInactivityInterval = getServiceInactivityInterval();
        break;

    case CFG_ID_WSA_TIMEOUT_INTERVAL:
        cfg->u.WSATimeoutInterval = getWSATimeoutInterval();
        break;

    case CFG_ID_WSA_TIMEOUT_INTERVAL2:
        cfg->u.WSATimeoutInterval = getWSATimeoutInterval2();
        break;

    case CFG_ID_RX_QUEUE_THRESHOLD:
        cfg->u.RxQueueThreshold = getRxQueueThreshold();
        break;

    case CFG_ID_WSA_CHANNEL_INTERVAL:
        cfg->u.ChannelInterval = getWSAChannelInterval();
        break;

    case CFG_ID_WSA_PRIORITY:
        cfg->u.Priority = getWSAPriority();
        break;

    case CFG_ID_WSA_DATARATE:
        cfg->u.DataRate = getWSADataRate();
        break;

    case CFG_ID_WSA_TXPWRLEVEL:
        cfg->u.TxPwrLevel = getWSATxPowerLevel();
        break;

    case CFG_ID_P2P_CHANNEL_INTERVAL:
        cfg->u.ChannelInterval = getP2PChannelInterval();
        break;

    case CFG_ID_P2P_PRIORITY:
        cfg->u.Priority = getP2PPriority();
        break;

    case CFG_ID_P2P_DATARATE:
        cfg->u.DataRate = getP2PDataRate();
        break;

    case CFG_ID_P2P_TXPWRLEVEL:
        cfg->u.TxPwrLevel = getP2PTxPowerLevel();
        break;

    case CFG_ID_IPV6_PRIORITY:
        cfg->u.Priority = getIPv6Priority();
        break;

    case CFG_ID_IPV6_DATARATE:
        cfg->u.DataRate = getIPv6DataRate();
        break;

    case CFG_ID_IPV6_TXPWRLEVEL:
        cfg->u.TxPwrLevel = getIPv6TxPowerLevel();
        break;

    case CFG_ID_IPV6_ADAPTABLE:
        cfg->u.Adaptable = getIPv6Adaptable();
        break;

    case CFG_ID_IPV6_PROVIDER_MODE:
        cfg->u.ProviderMode = getIPv6ProviderMode();
        break;

    case CFG_ID_IPV6_RADIO_TYPE:
        cfg->u.RadioType = getIPv6RadioType();
        break;

    case CFG_ID_IPV6_RADIO_NUM:
        cfg->u.RadioNum = getIPv6RadioNum();
        break;

    case CFG_ID_CONDUCTED_2_RADIATED_POWER_OFFSET_RADIO_0:
        cfg->u.conducted2RadiatedPowerOffset =
            getConducted2RadiatedPowerOffset(0);
        break;

    case CFG_ID_CONDUCTED_2_RADIATED_POWER_OFFSET_RADIO_1:
        cfg->u.conducted2RadiatedPowerOffset =
            getConducted2RadiatedPowerOffset(1);
        break;

    case CFG_ID_CHANGE_MAC_ADDRESS_ENABLE:
        cfg->u.ChangeMacAddressEnable = getChangeMacAddressEnable();
        break;

    case CFG_ID_RCV_EU_FROM_ETH_ENABLE:
        cfg->u.RcvEUFromEthEnable = getRcvEUFromEthEnable();
        break;

    case CFG_ID_RCV_EU_FROM_ETH_PORT_NUM:
        cfg->u.RcvEUFromEthPort = getRcvEUFromEthPortNum();
        break;

    case CFG_ID_SSP:
        cfg->u.SSP.SSPLen = getSSP(cfg->u.SSP.SSPValue);
        break;

    case CFG_ID_CV2X_PWR_CTRL_ENABLE:
        cfg->u.CV2XPwrCtrlEnable = getCV2XPwrCtrlEnable();
        break;

    case CFG_ID_DISCARD_UNSECURE_MSG:
        cfg->u.DiscardUnsecureMsg = getDiscardUnsecureMsg();
        break;

    case CFG_ID_DISCARD_VERFAIL_MSG:
        cfg->u.DiscardVerFailMsg = getDiscardVerFailMsg();
        break;

    case CFG_ID_PRIMARY_SERVICE_ID:
        cfg->u.PrimaryServiceId = getPrimaryServiceId();
        break;

    case CFG_ID_SECONDARY_SERVICE_ID:
        cfg->u.SecondaryServiceId = getSecondaryServiceId();
        break;

    case CFG_ID_ALSMI_DEBUG_LOG_ENABLE:
        cfg->u.ALSMIDebugLogEnable = getALSMIDebugLogEnable();
        break;

    case CFG_ID_ALSMI_SIGN_VERIFY_ENABLE:
        cfg->u.ALSMISignVerifyEnable = getALSMISignVerifyEnable();
        break;

    case CFG_ID_I2V_SYSLOG_ENABLE:
        cfg->u.I2vSyslogEnable = getI2vSyslogEnable();                        
        break;

    default:
        return RS_EINVID;
    }

    return RS_SUCCESS;
}

/**
** Function:  nsSetCfg
** @brief  Set the value of an NS configuration item
** @param  cfg -- [input]The cfgId field of cfg specifies which configuration
**                item to set. The value to set is in the proper cfg->u. field.
** @return RS_SUCCESS for success, error code otherwise
**
** Details: Checks the cfgId field. Then based on its value, calls the proper
**          routine to set the value. The value is gotten from the proper
**          cfg->u. field.
**/
rsResultCodeType nsSetCfg(cfgType *cfg)
{
    switch (cfg->cfgId) {
    case CFG_ID_CV2X_ENABLE:
        return setCV2XEnable(cfg->u.CV2XEnable);

    case CFG_ID_STATION_ROLE:
        return setStationRole(cfg->u.StationRole);

    case CFG_ID_REPEATS:
        return setRepeats(cfg->u.Repeats);

    case CFG_ID_CCH_NUM_RADIO_0:
        return setCCHNumber(0, cfg->u.CCHNum);

    case CFG_ID_CCH_NUM_RADIO_1:
        return setCCHNumber(1, cfg->u.CCHNum);

    case CFG_ID_SCH_NUM_RADIO_0:
        return setSCHNumber(0, cfg->u.SCHNum);

    case CFG_ID_SCH_NUM_RADIO_1:
        return setSCHNumber(1, cfg->u.SCHNum);

    case CFG_ID_SCH_ADAPTABLE:
        return setSCHAdaptable(cfg->u.Adaptable);

    case CFG_ID_SCH_DATARATE:
        return setSCHDataRate(cfg->u.DataRate);

    case CFG_ID_SCH_TXPWRLEVEL:
        return setSCHTxPowerLevel(cfg->u.TxPwrLevel);

    case CFG_ID_SERVICE_INACTIVITY_INTERVAL:
        return setServiceInactivityInterval(cfg->u.ServiceInactivityInterval);

    case CFG_ID_WSA_TIMEOUT_INTERVAL:
        return setWSATimeoutInterval(cfg->u.WSATimeoutInterval);

    case CFG_ID_WSA_TIMEOUT_INTERVAL2:
        return setWSATimeoutInterval2(cfg->u.WSATimeoutInterval);

    case CFG_ID_RX_QUEUE_THRESHOLD:
        return setRxQueueThreshold(cfg->u.RxQueueThreshold);

    case CFG_ID_WSA_CHANNEL_INTERVAL:
        return setWSAChannelInterval(cfg->u.ChannelInterval);

    case CFG_ID_WSA_PRIORITY:
        return setWSAPriority(cfg->u.Priority);

    case CFG_ID_WSA_DATARATE:
        return setWSADataRate(cfg->u.DataRate);

    case CFG_ID_WSA_TXPWRLEVEL:
        return setWSATxPowerLevel(cfg->u.TxPwrLevel);

    case CFG_ID_P2P_CHANNEL_INTERVAL:
        return setP2PChannelInterval(cfg->u.ChannelInterval);

    case CFG_ID_P2P_PRIORITY:
        return setP2PPriority(cfg->u.Priority);

    case CFG_ID_P2P_DATARATE:
        return setP2PDataRate(cfg->u.DataRate);

    case CFG_ID_P2P_TXPWRLEVEL:
        return setP2PTxPowerLevel(cfg->u.TxPwrLevel);

    case CFG_ID_IPV6_PRIORITY:
        return setIPv6Priority(cfg->u.Priority);

    case CFG_ID_IPV6_DATARATE:
        return setIPv6DataRate(cfg->u.DataRate);

    case CFG_ID_IPV6_TXPWRLEVEL:
        return setIPv6TxPowerLevel(cfg->u.TxPwrLevel);

    case CFG_ID_IPV6_ADAPTABLE:
        return setIPv6Adaptable(cfg->u.Adaptable);

    case CFG_ID_IPV6_PROVIDER_MODE:
        return setIPv6ProviderMode(cfg->u.ProviderMode);

    case CFG_ID_IPV6_RADIO_TYPE:
        return setIPv6RadioType(cfg->u.RadioType);

    case CFG_ID_IPV6_RADIO_NUM:
        return setIPv6RadioNum(cfg->u.RadioNum);

    case CFG_ID_CONDUCTED_2_RADIATED_POWER_OFFSET_RADIO_0:
        return setConducted2RadiatedPowerOffset(0, cfg->u.conducted2RadiatedPowerOffset);

    case CFG_ID_CONDUCTED_2_RADIATED_POWER_OFFSET_RADIO_1:
        return setConducted2RadiatedPowerOffset(1, cfg->u.conducted2RadiatedPowerOffset);

    case CFG_ID_CHANGE_MAC_ADDRESS_ENABLE:
        return setChangeMacAddressEnable(cfg->u.ChangeMacAddressEnable);

    case CFG_ID_RCV_EU_FROM_ETH_ENABLE:
        return setRcvEUFromEthEnable(cfg->u.RcvEUFromEthEnable);

    case CFG_ID_RCV_EU_FROM_ETH_PORT_NUM:
        return setRcvEUFromEthPortNum(cfg->u.RcvEUFromEthPort);

    case CFG_ID_SSP:
        return setSSP(cfg->u.SSP.SSPValue, cfg->u.SSP.SSPLen);

    case CFG_ID_CV2X_PWR_CTRL_ENABLE:
        return setCV2XPwrCtrlEnable(cfg->u.CV2XPwrCtrlEnable);

    case CFG_ID_DISCARD_UNSECURE_MSG:
        return setDiscardUnsecureMsg(cfg->u.DiscardUnsecureMsg);

    case CFG_ID_DISCARD_VERFAIL_MSG:
        return setDiscardVerFailMsg(cfg->u.DiscardVerFailMsg);

    case CFG_ID_PRIMARY_SERVICE_ID:
        return setPrimaryServiceId(cfg->u.PrimaryServiceId);

    case CFG_ID_SECONDARY_SERVICE_ID:
        return setSecondaryServiceId(cfg->u.SecondaryServiceId);

    case CFG_ID_ALSMI_DEBUG_LOG_ENABLE:
        return setALSMIDebugLogEnable(cfg->u.ALSMIDebugLogEnable);

    case CFG_ID_ALSMI_SIGN_VERIFY_ENABLE:
        return setALSMISignVerifyEnable(cfg->u.ALSMISignVerifyEnable);

    case CFG_ID_I2V_SYSLOG_ENABLE:
        return setI2vSyslogEnable(cfg->u.I2vSyslogEnable);

    default:
        return RS_EINVID;
    }
}

/**
** Function:  getCV2XEnable
** @brief  Get the C-V2X enable flag value
** @return The C-V2X enable flag value
**
** Details: Returns the C-V2X enable flag value stored in cfg.cv2x_enable
**/
bool_t getCV2XEnable(void)
{
    return cfg.cv2x_enable;
}

/**
** Function:  setCV2XEnable
** @brief  Set the C-V2X enable flag value
** @param  enable -- [input]TRUE to enable the C-V2X radio
** @return RS_SUCCESS for success, error code otherwise
**
** Details: Saves the station role value in cfg.cv2x_enable
**/
static rsResultCodeType setCV2XEnable(bool_t enable)
{
    rsResultCodeType risRet = RS_SUCCESS;
    int result = 0;

    /* Only do this if the value of cfg.cv2x_enable changes */
    if (cfg.cv2x_enable != enable) {
        cfg.cv2x_enable = enable;

        if (!parsingConfigFile) {
            if (enable) {
                /* Start the C-V2X subsystem */
                result = start_cv2x();

                if (result < 0) {
                    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"start_cv2x() failed\n");
                    risRet = RS_ECV2XRAD;
                }
                else {
#if defined(EXTRA_DEBUG)
                    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"start_cv2x() succeeded\n");
#endif
                    cv2x_started = TRUE;
                }
            }
            else {
                /* Stop the C-V2X subsystem */
                if (cv2x_started) { /* In case a previous call to start_cv2x()
                                     * failed */
                    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Calling stop_cv2x()\n");
                    stop_cv2x();
                    cv2x_started = FALSE;
                }
            }
        }
    }

    return risRet;
}
/**
** Function:  getStationRole
** @brief  Get the station role value
** @return The station role value
**
** Details: Returns the station role value stored in cfg.station_role
**/
roleType getStationRole(void)
{
    return (roleType)cfg.station_role;
}

/**
** Function:  setStationRole
** @brief  Set the station role value
** @param  station_role -- [input]The station role value to set
** @return RS_SUCCESS for success, error code otherwise
**
** Details: Range checks the station role value and returns RS_ERANGE if the
**          value is invalid. Otherwise, saves the station role value in
**          cfg.station_role.
**/
static rsResultCodeType setStationRole(roleType station_role)
{
    if ((station_role != ROLE_PROVIDER) && (station_role != ROLE_USER)) {
        return RS_ERANGE;
    }

    cfg.station_role = (uint8_t)station_role;
    return RS_SUCCESS;
}

/**
** Function:  getRepeats
** @brief  Get the repeats value
** @return The repeats value
**
** Details: Returns the repeats value stored in cfg.repeats
**/
int32_t getRepeats(void)
{
    return cfg.repeats;
}

/**
** Function:  setRepeats
** @brief  Set the repeats value
** @param  repeats -- [input]The repeats value to set
** @return RS_SUCCESS for success, error code otherwise
**
** Details: Range checks the repeats value and returns RS_ERANGE if the value
**          is invalid. Otherwise, saves the station role value in cfg.repeats.
**/
static rsResultCodeType setRepeats(uint32_t repeats)
{
    if (repeats <= MAX_REPEATS) {
        cfg.repeats = repeats;
        return RS_SUCCESS;
    }

    return RS_ERANGE;
}

/**
** Function:  getCCHNumber
** @brief  Get the control channel number
** @param  radioNum -- [input]Radio number
** @return The control channel number for the specified radio number
**
** Details: Returns the control channel number stored in cfg.cch_num_radio0 or
**          cfg.cch_num_radio1 depending on the value of radioNum
**/
uint8_t getCCHNumber(uint8_t radioNum)
{
    if (radioNum == 1) {
        return cfg.cch_num_radio1;
    }
    else {
        return cfg.cch_num_radio0;
    }
}

/**
** Function:  setCCHNumber
** @brief  Set the control channel number for the specified radio
** @param  radioNum -- [input]The radio number
** @param  cch_num  -- [input]The control channel number to set
** @return RS_SUCCESS for success, error code otherwise
**
** Details: Range checks the control channel number and returns RS_ERANGE if
**          the value is invalid. Otherwise, saves the control channel number
**          in cfg.cch_num_radio0 or cfg.cch_num_radio1 depending on the value
**          of radioNum
**/
static rsResultCodeType setCCHNumber(uint8_t radioNum, uint8_t cch_num)
{
    uint32_t i;

    for (i = 0; i < sizeof(g_valid_ch_nums) / sizeof(g_valid_ch_nums[0]); i++) {
        if (cch_num == g_valid_ch_nums[i]) {
            if (radioNum == 1) {
                cfg.cch_num_radio1 = cch_num;
            }
            else {
                cfg.cch_num_radio0 = cch_num;
            }

            return RS_SUCCESS;
        }
    }

    return RS_ERANGE;
}

/**
** Function:  getSCHNumber
** @brief  Get the service channel number
** @param  radioNum -- [input]Radio number
** @return The service channel number for the specified radio number
**
** Details: Returns the service channel number stored in cfg.sch_num_radio0 or
**          cfg.sch_num_radio1 depending on the value of radioNum
**/
uint8_t getSCHNumber(uint8_t radioNum)
{
    if (radioNum == 1) {
        return cfg.sch_num_radio1;
    }
    else {
        return cfg.sch_num_radio0;
    }
}

/**
** Function:  setSCHNumber
** @brief  Set the service channel number for the specified radio
** @param  radioNum -- [input]The radio number
** @param  sch_num  -- [input]The service channel number to set
** @return RS_SUCCESS for success, error code otherwise
**
** Details: Range checks the service channel number and returns RS_ERANGE if
**          the value is invalid. Otherwise, saves the service channel number
**          in cfg.sch_num_radio0 or cfg.sch_num_radio1 depending on the value
**          of radioNum
**/
rsResultCodeType setSCHNumber(uint8_t radioNum, uint8_t sch_num)
{
    uint32_t i;

    for (i = 0; i < sizeof(g_valid_ch_nums) / sizeof(g_valid_ch_nums[0]); i++) {
        if (sch_num == g_valid_ch_nums[i]) {
            if (radioNum == 1) {
                cfg.sch_num_radio1 = sch_num;
            }
            else {
                cfg.sch_num_radio0 = sch_num;
            }
            return RS_SUCCESS;
        }
    }

    return RS_ERANGE;
}

/**
** Function:  getSCHAdaptable
** @brief  Get the service channel adaptable value
** @return The service channel adaptable value
**
** Details: Returns the service channel adaptable value stored in
**          cfg.sch_adaptable
**/
bool_t getSCHAdaptable(void)
{
    return cfg.sch_adaptable;
}

/**
** Function:  setSCHAdaptable
** @brief  Set the service channel adaptable value
** @param  sch_adaptable -- [input]The service channel adaptable value to set
** @return RS_SUCCESS for success, error code otherwise
**
** Details: Set cfg.sch_adaptable to FALSE, since this is not supported
**/
static rsResultCodeType setSCHAdaptable(bool_t sch_adaptable)
{
    /* Always set this to FALSE, since we do not support adaptable = TRUE */
    cfg.sch_adaptable = FALSE;
    return RS_SUCCESS;
}

/**
** Function:  getSCHDataRate
** @brief  Get the service channel data rate value
** @return The service channel data rate value
**
** Details: Returns the service channel data rate value stored in
**          cfg.sch_datarate
**/
dataRateType getSCHDataRate(void)
{
    return (dataRateType)cfg.sch_datarate;
}

/**
** Function:  setSCHDataRate
** @brief  Set the service channel data rate value
** @param  sch_datarate -- [input]The service channel data rate value to set
** @return RS_SUCCESS for success, error code otherwise
**
** Details: Range checks the service channel data rate value and returns
**          RS_ERANGE if the value is invalid. Otherwise, saves the service
**          channel data rate value in cfg.sch_datarate.
**/
static rsResultCodeType setSCHDataRate(dataRateType sch_datarate)
{
    dataRateType dr[] = {
        DR_3_MBPS,  DR_4_5_MBPS, DR_6_MBPS,
        DR_9_MBPS,  DR_12_MBPS,  DR_18_MBPS,
        DR_24_MBPS, DR_27_MBPS,
    };
    uint32_t i;

    for (i = 0; i < sizeof(dr)/sizeof(dataRateType); i++) {
        if (sch_datarate == dr[i]) {
            cfg.sch_datarate = (uint8_t)sch_datarate;
            return RS_SUCCESS;
        }
    }

    return RS_ERANGE;
}

/**
** Function:  getSCHTxPowerLevel
** @brief  Get the service channel tx power level value
** @return The service channel tx power level value
**
** Details: Returns the service channel tx power level value stored in
**          cfg.sch_txpwrlevel
**/
int32_t getSCHTxPowerLevel(void)
{
    return cfg.sch_txpwrlevel;
}

/**
** Function:  setSCHTxPowerLevel
** @brief  Set the service channel tx power level value
** @param  sch_txpwrlevel -- [input]The service channel tx power level value to
**                           set
** @return RS_SUCCESS for success, error code otherwise
**
** Details: Range checks the service channel tx power level value and returns
**          RS_ERANGE if the value is invalid. Otherwise, saves the service
**          channel tx power level value in cfg.sch_txpwrlevel.
**/
static rsResultCodeType setSCHTxPowerLevel(uint32_t sch_txpwrlevel)
{
    if (sch_txpwrlevel <= MAX_TX_PWR_LEVEL) {
        cfg.sch_txpwrlevel = sch_txpwrlevel;
        return RS_SUCCESS;
    }

    return RS_ERANGE;
}

/**
** Function:  getServiceInactivityInterval
** @brief  Get the service inactivity interval value
** @return The service inactivity interval value
**
** Details: Returns the service inactivity interval value stored in
**          cfg.sch_service_inactivity_interval after scaling it
**/
int32_t getServiceInactivityInterval(void)
{
    return ((cfg.service_inactivity_interval * 1000) / HZ);
}

/**
** Function:  setServiceInactivityInterval
** @brief  Set the service inactivity interval value
** @param  service_inactivity_interval -- [input]The service inactivity
**                                        interval value to set
** @return RS_SUCCESS for success, error code otherwise
**
** Details: Range checks the service inactivity interval value and returns
**          RS_ERANGE if the value is invalid. Otherwise, saves the service
**          inactivity interval value in cfg.service_inactivity_interval after
**          scaling it.
**/
static rsResultCodeType setServiceInactivityInterval(uint32_t service_inactivity_interval)
{
    cfg.service_inactivity_interval = ((service_inactivity_interval * HZ) / 1000);
    return RS_SUCCESS;
}

/**
** Function:  getWSATimeoutInterval
** @brief  Get the WSA timeout interval value. This is the value where the
**         last seen WSA in the table is timed out, resulting in no WSA's in
**         the table.
** @return The WSA timeout interval value
**
** Details: Returns the WSA timeout interval value stored in
**          cfg.wsa_timeout_interval after scaling it
**/
int32_t getWSATimeoutInterval(void)
{
    return ((cfg.wsa_timeout_interval * 1000) / HZ);
}

/**
** Function:  setWSATimeoutInterval
** @brief  Set the WSA timeout interval value. This is the value where the
**         last seen WSA in the table is timed out, resulting in no WSA's in
**         the table.
** @param  wsa_timeout_interval -- [input]The WSA timeout interval value to set
** @return RS_SUCCESS for success, error code otherwise
**
** Details: Range checks the WSA timeout interval value and returns RS_ERANGE
**          if the value is invalid. Otherwise, saves the WSA timeout interval
**          value in cfg.wsa_timeout_interval after scaling it.
**/
static rsResultCodeType setWSATimeoutInterval(uint32_t wsa_timeout_interval)
{
    cfg.wsa_timeout_interval = ((wsa_timeout_interval * HZ) / 1000);
    return RS_SUCCESS;
}

/**
** Function:  getWSATimeoutInterval2
** @brief  Get the second WSA timeout interval value. This is the value where
**         a WSA that is not the last one in the table is timed out, resulting
**         in at least one WSA still left in the table.
** @return The second WSA timeout interval value
**
** Details: Returns the WSA timeout interval value stored in
**          cfg.wsa_timeout_interval2 after scaling it
**/
int32_t getWSATimeoutInterval2(void)
{
    return ((cfg.wsa_timeout_interval2 * 1000) / HZ);
}

/**
** Function:  setWSATimeoutInterval2
** @brief  Set the second WSA timeout interval value. This is the value where
**         a WSA that is not the last one in the table is timed out, resulting
**         in at least one WSA still left in the table.
** @param  wsa_timeout_interval2 -- [input]The WSA timeout interval value to set
** @return RS_SUCCESS for success, error code otherwise
**
** Details: Range checks the WSA timeout interval value and returns RS_ERANGE
**          if the value is invalid. Otherwise, saves the WSA timeout interval
**          value in cfg.wsa_timeout_interval2 after scaling it.
**/
static rsResultCodeType setWSATimeoutInterval2(uint32_t wsa_timeout_interval2)
{
    cfg.wsa_timeout_interval2 = ((wsa_timeout_interval2 * HZ) / 1000);
    return RS_SUCCESS;
}

/**
** Function:  getWSAChannelInterval
** @brief  Get the WSA channel interval value. 0=auto; 1=CCH; 2=SCH
** @return The WSA channel interval value
**
** Details: Returns the WSA channel interval value stored in
**          cfg.wsa_channel_interval
**/
uint8_t getWSAChannelInterval(void)
{
    return cfg.wsa_channel_interval;
}

/**
** Function:  setWSAChannelInterval
** @brief  Set the second WSA channel interval value. 0=auto; 1=CCH; 2=SCH
** @param  channelInterval -- [input]The WSA channel interval value to set
** @return RS_SUCCESS for success, error code otherwise
**
** Details: Range checks the WSA channel interval value and returns RS_ERANGE
**          if the value is invalid. Otherwise, saves the WSA channel interval
**          value in cfg.wsa_channel_interval.
**/
static rsResultCodeType setWSAChannelInterval(uint8_t channelInterval)
{
    if (channelInterval <= 2) {
        cfg.wsa_channel_interval = channelInterval;
        return RS_SUCCESS;
    }

    return RS_ERANGE;
}

/**
** Function:  getWSAPriority
** @brief  Get the WSA priority value
** @return The WSA priority value
**
** Details: Returns the WSA priority value stored in cfg.wsa_priority
**/
uint8_t getWSAPriority(void)
{
    return cfg.wsa_priority;
}

/**
** Function:  setWSAPriority
** @brief  Set the WSA priority value
** @param  priority -- [input]The WSA priority value to set
** @return RS_SUCCESS for success, error code otherwise
**
** Details: Range checks the WSA priority value and returns RS_ERANGE if the
**          value is invalid. Otherwise, saves the WSA priority value in
**          cfg.wsa_priority.
**/
static rsResultCodeType setWSAPriority(uint8_t priority)
{
    if (priority <= 7) {
        cfg.wsa_priority = priority;
        return RS_SUCCESS;
    }

    return RS_ERANGE;
}

/**
** Function:  getWSADataRate
** @brief  Get the WSA data rate value
** @return The WSA data rate value
**
** Details: Returns the WSA data rate value stored in cfg.wsa_datarate
**/
dataRateType getWSADataRate(void)
{
    return (dataRateType)cfg.wsa_datarate;
}

/**
** Function:  setWSADataRate
** @brief  Set the WSA data rate value
** @param  datarate -- [input]The WSA data rate value to set
** @return RS_SUCCESS for success, error code otherwise
**
** Details: Range checks the WSA data rate value and returns RS_ERANGE if the
**          value is invalid. Otherwise, saves the WSA data rate value in
**          cfg.wsa_datarate.
**/
static rsResultCodeType setWSADataRate(dataRateType datarate)
{
    dataRateType dr[] = {
        DR_3_MBPS,  DR_4_5_MBPS, DR_6_MBPS,
        DR_9_MBPS,  DR_12_MBPS,  DR_18_MBPS,
        DR_24_MBPS, DR_27_MBPS,
    };
    uint32_t i;

    for (i = 0; i < sizeof(dr) / sizeof(dataRateType); i++) {
        if (datarate == dr[i]) {
            cfg.wsa_datarate = (uint8_t)datarate;
            return RS_SUCCESS;
        }
    }

    return RS_ERANGE;
}

/**
** Function:  getWSATxPowerLevel
** @brief  Get the WSA tx power level value
** @return The WSA tx power level value
**
** Details: Returns the WSA data rate value stored in cfg.wsa_txpwrlevel
**/
uint8_t getWSATxPowerLevel(void)
{
    return cfg.wsa_txpwrlevel;
}

/**
** Function:  setWSATxPowerLevel
** @brief  Set the WSA tx power level value
** @param  txpwrlevel -- [input]The WSA tx power level value to set
** @return RS_SUCCESS for success, error code otherwise
**
** Details: Range checks the WSA tx power level value and returns RS_ERANGE if
**          the value is invalid. Otherwise, saves the WSA tx power level value
**          in cfg.wsa_txpwrlevel.
**/
static rsResultCodeType setWSATxPowerLevel(uint8_t txpwrlevel)
{
    if (txpwrlevel <= MAX_TX_PWR_LEVEL) {
        cfg.wsa_txpwrlevel = txpwrlevel;
        return RS_SUCCESS;
    }

    return RS_ERANGE;
}

/**
** Function:  getP2PChannelInterval
** @brief  Get the peer to peer channel interval value. 0=auto; 1=CCH; 2=SCH
** @return The peer to peer channel interval value
**
** Details: Returns the peer to peer data rate value stored in
**          cfg.p2p_channel_interval
**/
uint8_t getP2PChannelInterval(void)
{
    return cfg.p2p_channel_interval;
}

/**
** Function:  setP2PChannelInterval
** @brief  Set the peer to peer channel interval value. 0=auto; 1=CCH; 2=SCH
** @param  channelInterval -- [input]The peer to peer channel interval value to
**                            set
** @return RS_SUCCESS for success, error code otherwise
**
** Details: Range checks the peer to peer channel interval value and returns
**          RS_ERANGE if the value is invalid. Otherwise, saves the peer to
**          peer channel interval value in cfg.p2p_channel_interval.
**/
static rsResultCodeType setP2PChannelInterval(uint8_t channelInterval)
{
    if (channelInterval <= 2) {
        cfg.p2p_channel_interval = channelInterval;
        return RS_SUCCESS;
    }

    return RS_ERANGE;
}

/**
** Function:  getP2PPriority
** @brief  Get the peer to peer priority value
** @return The peer to peer priority value
**
** Details: Returns the peer to peer priority value stored in cfg.p2p_priority
**/
uint8_t getP2PPriority(void)
{
    return cfg.p2p_priority;
}

/**
** Function:  setP2PPriority
** @brief  Set the peer to peer priority value
** @param  priority -- [input]The peer to peer priority value to set
** @return RS_SUCCESS for success, error code otherwise
**
** Details: Range checks the peer to peer priority value and returns RS_ERANGE
**          if the value is invalid. Otherwise, saves the peer to peer priority
**          value in cfg.p2p_priority.
**/
static rsResultCodeType setP2PPriority(uint8_t priority)
{
    if (priority <= 7) {
        cfg.p2p_priority = priority;
        return RS_SUCCESS;
    }

    return RS_ERANGE;
}

/**
** Function:  getP2PDataRate
** @brief  Get the peer to peer data rate value
** @return The peer to peer data rate value
**
** Details: Returns the peer to peer data rate value stored in cfg.p2p_datarate
**/
dataRateType getP2PDataRate(void)
{
    return (dataRateType)cfg.p2p_datarate;
}

/**
** Function:  setP2PDataRate
** @brief  Set the peer to peer data rate value
** @param  datarate -- [input]The peer to peer data rate value to set
** @return RS_SUCCESS for success, error code otherwise
**
** Details: Range checks the peer to peer data rate value and returns RS_ERANGE
**          if the value is invalid. Otherwise, saves the peer to peer data
**          rate value in cfg.p2p_datarate.
**/
static rsResultCodeType setP2PDataRate(dataRateType datarate)
{
    dataRateType dr[] = {
        DR_3_MBPS,  DR_4_5_MBPS, DR_6_MBPS,
        DR_9_MBPS,  DR_12_MBPS,  DR_18_MBPS,
        DR_24_MBPS, DR_27_MBPS,
    };
    uint32_t i;

    for (i = 0; i < sizeof(dr) / sizeof(dataRateType); i++) {
        if (datarate == dr[i]) {
            cfg.p2p_datarate = (uint8_t)datarate;
            return RS_SUCCESS;
        }
    }

    return RS_ERANGE;
}

/**
** Function:  getP2PTxPowerLevel
** @brief  Get the peer to peer tx power level value
** @return The peer to peer tx power level value
**
** Details: Returns the peer to peer tx power level value stored in
**          cfg.p2p_txpwrlevel
**/
uint8_t getP2PTxPowerLevel(void)
{
    return cfg.p2p_txpwrlevel;
}

/**
** Function:  setP2PTxPowerLevel
** @brief  Set the peer to peer tx power level value
** @param  txpwrlevel -- [input]The peer to peer tx power level value to set
** @return RS_SUCCESS for success, error code otherwise
**
** Details: Range checks the peer to peer tx power level value and returns
**          RS_ERANGE if the value is invalid. Otherwise, saves the peer to
**          peer tx power level value in cfg.p2p_txpwrlevel.
**/
static rsResultCodeType setP2PTxPowerLevel(uint8_t txpwrlevel)
{
    if (txpwrlevel <= MAX_TX_PWR_LEVEL) {
        cfg.p2p_txpwrlevel = txpwrlevel;
        return RS_SUCCESS;
    }

    return RS_ERANGE;
}

/**
** Function:  getIPv6Priority
** @brief  Get the IPv6 priority value
** @return The IPv6 priority value
**
** Details: Returns the IPv6 priority value stored in cfg.ipv6_priority
**/
uint8_t getIPv6Priority(void)
{
    return cfg.ipv6_priority;
}

/**
** Function:  setIPv6Priority
** @brief  Set the IPv6 priority value
** @param  priority -- [input]The IPv6 priority value to set
** @return RS_SUCCESS for success, error code otherwise
**
** Details: Range checks the IPv6 priority value and returns RS_ERANGE if the
**          value is invalid. Otherwise, saves the IPv6 priority value in
**          cfg.ipv6_priority.
**/
static rsResultCodeType setIPv6Priority(uint8_t priority)
{
    if (priority <= 7) {
        cfg.ipv6_priority = priority;
        return RS_SUCCESS;
    }

    return RS_ERANGE;
}

/**
** Function:  getIPv6DataRate
** @brief  Get the IPv6 data rate value
** @return The IPv6 data rate value
**
** Details: Returns the IPv6 data rate value stored in cfg.ipv6_datarate
**/
dataRateType getIPv6DataRate(void)
{
    return (dataRateType)cfg.ipv6_datarate;
}

/**
** Function:  setIPv6DataRate
** @brief  Set the IPv6 data rate value
** @param  datarate -- [input]The IPv6 data rate value to set
** @return RS_SUCCESS for success, error code otherwise
**
** Details: Range checks the IPv6 data rate value and returns RS_ERANGE if the
**          value is invalid. Otherwise, saves the IPv6 data rate value in
**          cfg.ipv6_datarate.
**/
static rsResultCodeType setIPv6DataRate(dataRateType datarate)
{
#if !defined(HEAVY_DUTY)
    uint8_t MCS;
    char cmd[64];

    dataRateType dr[] = {
        DR_3_MBPS,  DR_4_5_MBPS, DR_6_MBPS,
        DR_9_MBPS,  DR_12_MBPS,  DR_18_MBPS,
        DR_24_MBPS, DR_27_MBPS,
    };
    uint32_t i;

    for (i = 0; i < sizeof(dr) / sizeof(dataRateType); i++) {
        if (datarate == dr[i]) {
            cfg.ipv6_datarate = (uint8_t)datarate;
            /* Convert data rate to MCS */
            MCS = (datarate == DR_27_MBPS)  ? MKXMCS_R34QAM64 :
                  (datarate == DR_24_MBPS)  ? MKXMCS_R23QAM64 :
                  (datarate == DR_18_MBPS)  ? MKXMCS_R34QAM16 :
                  (datarate == DR_12_MBPS)  ? MKXMCS_R12QAM16 :
                  (datarate == DR_9_MBPS)   ? MKXMCS_R34QPSK  :
                  (datarate == DR_6_MBPS)   ? MKXMCS_R12QPSK  :
                  (datarate == DR_4_5_MBPS) ? MKXMCS_R34BPSK  :
                /*(datarate == DR_3_MBPS)*/   MKXMCS_R12BPSK;
            sprintf(cmd, "echo %u > /sys/module/cw_llc/parameters/IPv6MCS", MCS);
            system(cmd);
            return RS_SUCCESS;
        }
    }

    return RS_ERANGE;
#else
    return RS_SUCCESS;
#endif
}

/**
** Function:  getIPv6TxPowerLevel
** @brief  Get the IPv6 tx power level value
** @return The IPv6 tx power level value
**
** Details: Returns the IPv6 tx power level value stored in cfg.ipv6_txpwrlevel
**/
uint8_t getIPv6TxPowerLevel(void)
{
    return cfg.ipv6_txpwrlevel;
}

/**
** Function:  setIPv6TxPowerLevel
** @brief  Set the IPv6 tx power level value
** @param  txpwrlevel -- [input]The IPv6 tx power level value to set
** @return RS_SUCCESS for success, error code otherwise
**
** Details: Range checks the IPv6 tx power level value and returns RS_ERANGE if
**          the value is invalid. Otherwise, saves the IPv6 tx power level
**          value in cfg.ipv6_txpwrlevel.
**/
static rsResultCodeType setIPv6TxPowerLevel(uint8_t txpwrlevel)
{
#if !defined(HEAVY_DUTY)
    char cmd[64];

    if (txpwrlevel <= MAX_TX_PWR_LEVEL) {
        cfg.ipv6_txpwrlevel = txpwrlevel;
        /* Note * 2 because MK5 Tx power level is in 1/2 dBm units */
        sprintf(cmd, "echo %u > /sys/module/cw_llc/parameters/IPv6TxPwr",
                txpwrlevel * 2);
        system(cmd);
        return RS_SUCCESS;
    }

    return RS_ERANGE;
#else
    return RS_SUCCESS;
#endif
}

/**
** Function:  getIPv6Adaptable
** @brief  Get the IPv6 adaptable value
** @return The IPv6 adaptable value
**
** Details: Returns the IPv6 adaptable value stored in cfg.ipv6_adaptable
**/
bool_t getIPv6Adaptable(void)
{
    return cfg.ipv6_adaptable;
}

/**
** Function:  setIPv6Adaptable
** @brief  Set the service channel adaptable value
** @param  adaptable -- [input]The service channel adaptable value to set
** @return RS_SUCCESS for success, error code otherwise
**
** Details: Set cfg.ipv6_adaptable to FALSE, since this is not supported
**/
static rsResultCodeType setIPv6Adaptable(bool_t adaptable)
{
    /* Always set this to FALSE, since we do not support adaptable = TRUE */
    cfg.ipv6_adaptable = FALSE;
    return RS_SUCCESS;
}

/**
** Function:  getIPv6ProviderMode
** @brief  Get the IPv6 provider mode value. This it TRUE if we are using IPv6
**         for a provider service.
** @return The IPv6 provider mode value
**
** Details: Returns the IPv6 provider mode value stored in
**          cfg.ipv6_providermode
**/
bool_t getIPv6ProviderMode(void)
{
    return cfg.ipv6_providermode;
}

/**
** Function:  setIPv6ProviderMode
** @brief  Set the IPv6 provider mode value. This it TRUE if we are using IPv6
**         for a provider service.
** @param  providermode -- [input]The IPv6 provider mode value to set
** @return RS_SUCCESS for success, error code otherwise
**
** Details: Saves the IPv6 provider mode value in cfg.ipv6_providermode.
**/
static rsResultCodeType setIPv6ProviderMode(bool_t providermode)
{
    cfg.ipv6_providermode = providermode;
    return RS_SUCCESS;
}

/**
** Function:  getIPv6RadioType
** @brief  Get the IPv6 radio type value
** @return The IPv6 radio type value
**
** Details: Returns the IPv6 radio type value stored in cfg.ipv6_radioType
**/
rsRadioType getIPv6RadioType(void)
{
    return (rsRadioType)cfg.ipv6_radioType;
}

/**
** Function:  setIPv6RadioType
** @brief  Set the IPv6 radio type value.
** @param  radioType -- [input]The IPv6 radio type value to set
** @return RS_SUCCESS for success, error code otherwise
**
** Details: Range checks the IPv6 radio type value and returns RS_ERANGE if
**          the value is invalid. Otherwise, saves the IPv6 txradio type value
**          in cfg.ipv6_radioType.
**/
static rsResultCodeType setIPv6RadioType(rsRadioType radioType)
{
    if (radioType < MAX_RADIO_TYPES) {
        cfg.ipv6_radioType = (uint8_t)radioType;
        return RS_SUCCESS;
    }

    return RS_ERANGE;
}

/**
** Function:  getIPv6RadioNum
** @brief  Get the IPv6 radio number value
** @return The IPv6 radio number value
**
** Details: Returns the IPv6 radio number value stored in cfg.ipv6_radioNum
**/
uint8_t getIPv6RadioNum(void)
{
    return cfg.ipv6_radioNum;
}

/**
** Function:  setIPv6RadioNum
** @brief  Set the IPv6 radio number value.
** @param  radioNum -- [input]The IPv6 radio number value to set
** @return RS_SUCCESS for success, error code otherwise
**
** Details: Range checks the IPv6 radio number value and returns RS_ERANGE if
**          the value is invalid. Otherwise, saves the IPv6 txradio number
**          value in cfg.ipv6_radioNum.
**/
static rsResultCodeType setIPv6RadioNum(uint8_t radioNum)
{
    if (radioNum < MAX_RADIOS) {
        cfg.ipv6_radioNum = radioNum;
        return RS_SUCCESS;
    }

    return RS_ERANGE;
}

/**
** Function:  getConducted2RadiatedPowerOffset
** @brief  Get the conducted power to radiated power offset value for the
**         specified radio.
** @param  radioNum -- [input]The radio number
** @return The conducted power to radiated power offset value
**
** Details: Returns the conducted power to radiated power offset value stored
**          in cfg.conducted_2_radiated_power_offset_radio_0 or
**          cfg.conducted_2_radiated_power_offset_radio_1 depending on the
**          value of radioNum.
**/
int16_t getConducted2RadiatedPowerOffset(uint8_t radioNum)
{
    return (radioNum == 0) ? cfg.conducted_2_radiated_power_offset_radio_0 :
                             cfg.conducted_2_radiated_power_offset_radio_1;
}

/**
** Function:  setConducted2RadiatedPowerOffset
** @brief  Set the conducted power to radiated power offset value for the
**         specified radio.
** @param  radioNum -- [input]The IPv6 radio number value to set
** @param  offset   -- [input]The conducted power to radiated power offset
**                     value to set for the specified radio
** @return RS_SUCCESS for success, error code otherwise
**
** Details: Range checks the conducted power to radiated power offset value and
**          returns RS_ERANGE if the value is invalid. Otherwise, saves the
**          conducted power to radiated power offset value in
**          conducted_2_radiated_power_offset_radio_0 or
**          conducted_2_radiated_power_offset_radio_1 depending on the value of
**          radioNum.
**/
static rsResultCodeType setConducted2RadiatedPowerOffset(uint8_t radioNum, int16_t offset)
{
    if (radioNum >= MAX_RADIOS) {
        return RS_ERANGE;
    }

    if ((offset >= -200) && (offset <= 200)) {
        if (radioNum == 0) {
            cfg.conducted_2_radiated_power_offset_radio_0 = offset;
        }
        else {
            cfg.conducted_2_radiated_power_offset_radio_1 = offset;
        }

        return RS_SUCCESS;
    }

    return RS_ERANGE;
}

/**
** Function:  getChangeMacAddressEnable
** @brief  Get the change MAC address enable flag value
** @return The change MAC address enable flag value
**
** Details: Returns the change MAC address enable flag value stored in
**          cfg.change_mac_address_enable
**/
bool_t getChangeMacAddressEnable(void)
{
    return cfg.change_mac_address_enable;
}

/**
** Function:  setChangeMacAddressEnable
** @brief  Set the change MAC address enable flag value
** @param  change_mac_address_enable -- [input]The MAC address enable flag
**                                      value to set
** @return RS_SUCCESS for success, error code otherwise
**
** Details: Saves the change MAC address enable flag value in
**          cfg.change_mac_address_enable
**/
static rsResultCodeType setChangeMacAddressEnable(bool_t change_mac_address_enable)
{
    cfg.change_mac_address_enable = change_mac_address_enable;
    return RS_SUCCESS;
}

/**
** Function:  getRcvEUFromEthEnable
** @brief  Get the receive EU packets from the Ethernet port enable flag value
** @return The receive EU packets from the Ethernet port enable flag value
**
** Details: Returns the receive EU packets from the Ethernet port enable flag
**          value stored in cfg.rcv_EU_from_eth_enable
**/
bool_t getRcvEUFromEthEnable(void)
{
    return cfg.rcv_EU_from_eth_enable;
}

/**
** Function:  setRcvEUFromEthEnable
** @brief  Set the receive EU packets from the Ethernet port enable flag value
** @param  rcv_EU_from_eth_enable -- [input]The receive EU packets from the
**                                   Ethernet port enable flag value to set
** @return RS_SUCCESS for success, error code otherwise
**
** Details: Saves the receive EU packets from the Ethernet port enable flag
**          value in cfg.rcv_EU_from_eth_enable if the value has changed. If
**          changing from FALSE to TRUE, starts the receive EU from the
**          Ethernet port processing.
**/
static rsResultCodeType setRcvEUFromEthEnable(bool_t rcv_EU_from_eth_enable)
{
    /* Do this only if enable value changed */
    if (cfg.rcv_EU_from_eth_enable != rcv_EU_from_eth_enable) {
        /* Set the new enable value */
        cfg.rcv_EU_from_eth_enable = rcv_EU_from_eth_enable;

        if (!parsingConfigFile) {
            /* If enabling receive EU from Ethernet, start EU from Ethernet
             * processing */
            if (rcv_EU_from_eth_enable) {
                /* VJR WILLBEREVISITED Implement the functionality below */
//                StartReceiveEUFromEthernetProcessing();
            }
        }
    }

    return RS_SUCCESS;
}

/**
** Function:  getRcvEUFromEthPortNum
** @brief  Get the receive EU packets from the Ethernet port number value
** @return The receive EU packets from the Ethernet port number value
**
** Details: Returns the receive EU packets from the Ethernet port number value
**          stored in cfg.rcv_EU_from_eth_port_num
**/
uint16_t getRcvEUFromEthPortNum(void)
{
    return cfg.rcv_EU_from_eth_port_num;
}

/**
** Function:  setRcvEUFromEthPortNum
** @brief  Set the port number to receive EU packets from the Ethernet port on
** @param  rcv_EU_from_eth_port_num -- [input]The port number to receive EU
**                                     packets from the Ethernet port on
** @return RS_SUCCESS for success, error code otherwise
**
** Details: Saves the port number in cfg.rcv_EU_from_eth_port_num
**/
static rsResultCodeType setRcvEUFromEthPortNum(uint16_t rcv_EU_from_eth_port_num)
{
    cfg.rcv_EU_from_eth_port_num = rcv_EU_from_eth_port_num;
    return RS_SUCCESS;
}

/**
** Function:  getSSP
** @brief  Get what SSP value to include in signed packets
** @param  ssp -- [output]Where to store the SSP value
** @return The SSP length
**
** Details: Returns the SSP length from cfg.sspLen and writes out the SSP value
**          from cfg.ssp
**/
uint8_t getSSP(uint8_t *ssp)
{
    if (cfg.sspLen > 0) {
        memcpy(ssp, cfg.ssp, cfg.sspLen);
    }

    return cfg.sspLen;
}

/**
** Function:  setSSP
** @brief  Set what SSP value to include in signed packets
** @param  ssp    -- [input]Value of the SSP to include in the signed packets
** @param  sspLen -- [input]Length of the SSP to include in the signed packets
** @return RS_SUCCESS for success, error code otherwise
**
** Details: Saves the SSP value in cfg.ssp and cfg.sspLen
**/
static rsResultCodeType setSSP(uint8_t *ssp, uint8_t sspLen)
{
    if (sspLen > MAX_SSP_DATA) {
        return RS_ERANGE;
    }

    memcpy(cfg.ssp, ssp, sspLen);
    cfg.sspLen = sspLen;
    return RS_SUCCESS;
}

/**
** Function: get_random_bytes
** @brief  Generate a requested number of random bytes.
** @param  ranbuf -- [output]Pointer byte array containing where random
**                   bytes are written.
** @param  nbytes -- [input]Number of bytes to write to ranbuf array.
** @return void
**
** Details: In a for loop, calls random() nbytes times to generate nbytes
**          random numbers and stores them in ranbuf.
**          In linux this function is declared in linux/random.h
**/
static inline void get_random_bytes(uint8_t *ranbuf, int nbytes)
{
    int32_t  i;
    int32_t  ranpos;
    int32_t  ranval;
    uint8_t *pranval = (uint8_t *)&ranval;

    ranpos = 0;

    for (i = 0; i < nbytes; i++) {
        if ((i % sizeof(int32_t)) == 0) {
            ranval = random();
            ranpos = 0;
        }

        ranbuf[i] = pranval[ranpos];
        ranpos++;
    }

#ifdef RSK_LOG_DEBUG
    printf("get_random_bytes: sizeof(long)=%d (%x),nbytes=%d,  ranval=%lx, ",
                              sizeof(long),sizeof(long),  nbytes, ranval   );
    printf("ranbuf=%2X:",ranbuf[0]);

    for (i = 1; i < nbytes; i++) {
        printf("%2X:", ranbuf[i]);
    }

    printf("\n");
    fflush(stdout);
#endif
}

/**
** Function: eth_random_addr
** @brief  Generate a software assigned random Ethernet address(MAC)
** @brief  that is not multicast and has the local assigned bit set.
** @param  addr -- [output]Pointer to a six-byte array to contain the
**                 generated Ethernet address
** @return void
**
** Details: Generates 6 random bytes. Then clears the multicast bit and sets
**          the local assignment bit for the first byte.
**          In linux this is a macro in linux/etherdevice.h
**/
static inline void eth_random_addr(uint8_t *addr)
{
    get_random_bytes(addr, LENGTH_MAC);
    addr[0] &= 0xfe;    /* clear multicast bit */
    addr[0] |= 0x02;    /* set local assignment bit (IEEE802) */
}

#define random_ether_addr(addr) eth_random_addr(addr)

/**
** Function: nsSetRandomMacAddress
** @brief  Set Random MAC Address
** @param  radioType -- [input]The radio type (RT_CV2X or RT_DSRC)
** @param  radioNum  -- [input]The radio number for which to set the MacAddress.
** @return void
**
** Details: Only does this is the radio type is RT_DSRC. Generates a random MAC
**          address by calling random_ether_addr and sets the MAC address for
**          the specified radio by calling dsrc_set_cfg().
**/
void nsSetRandomMacAddress(rsRadioType radioType, uint8_t radioNum)
{
    cfgType cfg;

    if((radioType == RT_CV2X) && (getCV2XEnable())) {

        /* Create a Random MAC Address */
        random_ether_addr(cfg.u.RadioMacAddress);

        ///I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"%s:Radio[type=%d radio num=%u] [%02x:%02x:%02x:%02x:%02x:%02x]\n",
        ///           __func__, radioType, radioNum,
        ///           cfg.u.RadioMacAddress[0], cfg.u.RadioMacAddress[1],
        ///           cfg.u.RadioMacAddress[2], cfg.u.RadioMacAddress[3],
        ///           cfg.u.RadioMacAddress[4], cfg.u.RadioMacAddress[5] );

        /* Set MAC Addr for RX address matching */
        cfg.cfgId = (radioNum == 1) ? CFG_ID_MACADDRESS_RADIO_1 :
                                      CFG_ID_MACADDRESS_RADIO_0;


       /* 
        * We cant set MAC on CV2X can we? Not seeing call in ns_cv2x.cpp?
        * So just return MACS to put in WSM's for CV2X to send? Sounds wrong...
        */
 
        /* if (RT_CV2X == radioType) { ... */
        

#if defined(HEAVY_DUTY) 
        /* 
         * Set the MACS internal to this module only. CV2X & DSRC no matter.
         */
        nsSetSrcMac(radioNum,cfg.u.RadioMacAddress);
#else
        /* Note: No need to read the "hardware" MAC address at startup and
         * store it. The Hercules has no "hardware" MAC address for the radios.
         * Instead, the radio MAC addresses must be set via software.
         * radioServices must assign each radio an initial random MAC address
         * at boot-up. */
#endif
    }
}

/**
** Function: nsSetMK5SrcMac
** @brief  Set the MK5 source MAC address value for the specified radio and
** @brief  MK5 channel as a uint64_t value.
** @param  radio    -- [input]The radio number for which to set the MAC Address
** @param  channel  -- [input]The MK5 channel for which to set the MAC Address
** @return void
**
** Details: Saves the uint64_t MAC address value in
**          cfg.ChanCfg[radio][channel].RadioMacAddr
**/
int32_t nsSetMK5SrcMac(uint8_t radio, uint8_t channel, uint64_t srcMacAddr)
{
    cfg.ChanCfg[radio][channel].MK5RadioMacAddr = srcMacAddr;
    return 0;
}

/**
** Function: nsGetSrcMac
** @brief  Get the MK5 source MAC address value for the specified radio and
** @brief  MK5 channel as a uint64_t value.
** @param  radio   -- [input]The radio number for which to set the MAC Address
** @param  channel -- [input]The MK5 channel for which to set the MAC Address
** @return The uint64_t MAC address value
**
** Details: Returns the uint64_t MAC address value in
**          cfg.ChanCfg[radio][channel].RadioMacAddr
**/
uint8_t *nsGetSrcMac(uint8_t radio, uint8_t channel)
{
    return cfg.ChanCfg[radio][channel].RadioMacAddr;
}

/**
** Function: nsSetSrcMac
** @brief  Set the source MAC address value for the specified radio
** @param  radioNum   -- [input]The radio number for which to set the MAC Address
** @param  srcMacAddr -- [input]The MAC address as an array of bytes
** @return void
**
** Details: Saves the MAC address value in
**          cfg.ChanCfg[radioNum][channel].RadioMacAddr for channel = 0 and 1
**/
void nsSetSrcMac(uint8_t radioNum, uint8_t *srcMacAddr)
{
    /*
     * Both channels of a radio will always have the same MAC address
     * (Set both channels to same MAC & all GetSrcMac functions will ignore channel.)
     */
    memcpy(cfg.ChanCfg[radioNum][0].RadioMacAddr, srcMacAddr, LENGTH_MAC);
    memcpy(cfg.ChanCfg[radioNum][1].RadioMacAddr, srcMacAddr, LENGTH_MAC);
}

/**
** Function: nsGetSrcMacString
** @brief  Get the source MAC address value for the specified radio number and
**         MK5 channel number as a string
** @param  radio   -- [input]The radio number for which to get the MAC Address
** @param  channel -- [input]The MK5 channel for which to get the MAC Address
** @return A pointer to the string value.
**
** Details: Generates the MAC address value for the specified radio and channel
**          in the local static array MacStr. Note that the value returned by
**          nsGetSrcMacString must be finished being used before
**          nsGetSrcMacString is called again.
**/
static char *nsGetSrcMacString(uint8_t radio, uint8_t channel)
{
    static char MacStr[20];

    snprintf(MacStr,20, "%02x:%02x:%02x:%02x:%02x:%02x",
            (cfg.ChanCfg[radio][channel].RadioMacAddr[0]),
            (cfg.ChanCfg[radio][channel].RadioMacAddr[1]),
            (cfg.ChanCfg[radio][channel].RadioMacAddr[2]),
            (cfg.ChanCfg[radio][channel].RadioMacAddr[3]),
            (cfg.ChanCfg[radio][channel].RadioMacAddr[4]),
            (cfg.ChanCfg[radio][channel].RadioMacAddr[5]));

    return MacStr;
}

/**
** Function: getNSCfgInfo
** @brief  Get the NS configuration information as a very long string.
** @param  buf      -- [output]Pointer to the buffer to store the very long
**                     string in.
** @param  buf_size -- [input]The length of buf in bytes
** @return The length of the very long string generated
**
** Details: Generates a very long string containing the NS configuration
**          information in order to eventually display it on the console for
**          debugging purposes. Used by the "nsstats -c" command.
**/
int getNSCfgInfo(char *buf, uint32_t buf_size)
{
    typedef struct {
        dataRateType  DataRate;
        char         *strDataRate;
    } dr_t;

    dr_t dr[] =
    {
        {DR_3_MBPS,   "3 Mbps"  },
        {DR_4_5_MBPS, "4.5 Mbps"},
        {DR_6_MBPS,   "6 Mbps"  },
        {DR_9_MBPS,   "9 Mbps"  },
        {DR_12_MBPS,  "12 Mbps" },
        {DR_18_MBPS,  "18 Mbps" },
        {DR_24_MBPS,  "24 Mbps" },
        {DR_27_MBPS,  "27 Mbps" },
    };

    uint32_t   i;
    char *buf_ptr = buf;

    buf_ptr += snprintf((char *)buf_ptr, buf + buf_size - buf_ptr,
                        "*** NS Configuration ***\n");
    buf_ptr += snprintf((char *)buf_ptr, buf + buf_size - buf_ptr,
                        "Version:\t\t\t%s\n", getNSVersion());
    buf_ptr += snprintf((char *)buf_ptr, buf + buf_size - buf_ptr,
                        "Build Time:\t\t\t%s\n", getNSBuildTime());
    buf_ptr += snprintf((char *)buf_ptr, buf + buf_size - buf_ptr,
                        "Station Role:\t\t\t%s\n",
                        ((getStationRole() == ROLE_PROVIDER) ? "Provider" :
                        ((getStationRole() == ROLE_USER) ? "User" : "Unknown")));
    buf_ptr += snprintf((char *)buf_ptr, buf + buf_size - buf_ptr,
                        "Repeats:\t\t\t%d\n", getRepeats());
    buf_ptr += snprintf((char *)buf_ptr, buf + buf_size - buf_ptr,
                        "Service Inactivity Interval:\t%d ms\n",
                        getServiceInactivityInterval());
    buf_ptr += snprintf((char *)buf_ptr, buf + buf_size - buf_ptr,
                        "WSA Timeout Interval:\t\t%d ms\n",
                        getWSATimeoutInterval());
    buf_ptr += snprintf((char *)buf_ptr, buf + buf_size - buf_ptr,
                        "WSA Timeout Interval2:\t\t%d ms\n",
                        getWSATimeoutInterval2());

    if (getWSAChannelInterval() <= 2) {
        buf_ptr += snprintf((char *)buf_ptr, buf + buf_size - buf_ptr,
                            "WSA Channel Interval:\t\t%s\n",
                            (getWSAChannelInterval() == 0) ? "Auto" :
                            (getWSAChannelInterval() == 1) ? "CCH" : "SCH");
    }
    else {
        buf_ptr += snprintf((char *)buf_ptr, buf + buf_size - buf_ptr,
                            "WSA Channel Interval:\t\tUnknown (%u)\n",
                            getWSAChannelInterval());
    }

    buf_ptr += snprintf((char *)buf_ptr, buf + buf_size - buf_ptr,
                        "WSA Tx Priority:\t\t%d\n", getWSAPriority());

    for (i = 0; i < sizeof(dr)/sizeof(dr_t); i++) {
        if (dr[i].DataRate == getWSADataRate()) {
            buf_ptr += snprintf((char *)buf_ptr, buf + buf_size - buf_ptr,
                                "WSA Data Rate:\t\t\t%d (%s)\n",
                                dr[i].DataRate, dr[i].strDataRate);
            break;
        }
    }

    if (i == sizeof(dr)/sizeof(dr_t)) {
        buf_ptr += snprintf((char *)buf_ptr, buf + buf_size - buf_ptr,
                            "WSA Data Rate:\t\t\t%d (%s)\n",
                            getWSADataRate(), "Unknown");
    }

    buf_ptr += snprintf((char *)buf_ptr, buf + buf_size - buf_ptr,
                        "WSA Tx Power Level:\t\t%d dBm\n",
                        getWSATxPowerLevel());
    buf_ptr += snprintf((char *)buf_ptr, buf + buf_size - buf_ptr,
                        "CCH Number:\t\t\t%d\t\t%d\n",
                        getCCHNumber(0), getCCHNumber(1) );
    buf_ptr += snprintf((char *)buf_ptr, buf + buf_size - buf_ptr,
                        "SCH Number:\t\t\t%d\t\t%d\n",
                        getSCHNumber(0), getSCHNumber(1) );
    buf_ptr += snprintf((char *)buf_ptr, buf + buf_size - buf_ptr,
                        "SCH Adaptable:\t\t\t%s\n",
                        ((getSCHAdaptable() == FALSE) ? "False" : "True"));

    for (i = 0; i < sizeof(dr)/sizeof(dr_t); i++) {
        if (dr[i].DataRate == getSCHDataRate()) {
            buf_ptr += snprintf((char *)buf_ptr, buf + buf_size - buf_ptr,
                                "SCH Data Rate:\t\t\t%d (%s)\n",
                                dr[i].DataRate, dr[i].strDataRate);
            break;
        }
    }

    if (i == sizeof(dr)/sizeof(dr_t)) {
        buf_ptr += snprintf((char *)buf_ptr, buf + buf_size - buf_ptr,
                            "SCH Data Rate:\t\t\t%d (%s)\n",
                            getSCHDataRate(), "Unknown");
    }

    buf_ptr += snprintf((char *)buf_ptr, buf + buf_size - buf_ptr,
                        "SCH Tx Power Level:\t\t%d dBm\n",
                        getSCHTxPowerLevel());

    if (getP2PChannelInterval() <= 2) {
        buf_ptr += snprintf((char *)buf_ptr, buf + buf_size - buf_ptr,
                            "P2P Channel Interval:\t\t%s\n",
                            (getP2PChannelInterval() == 0) ? "Auto" :
                            (getP2PChannelInterval() == 1) ? "CCH" : "SCH");
    }
    else {
        buf_ptr += snprintf((char *)buf_ptr, buf + buf_size - buf_ptr,
                            "P2P Channel Interval:\t\tUnknown (%u)\n",
                            getP2PChannelInterval());
    }

    buf_ptr += snprintf((char *)buf_ptr, buf + buf_size - buf_ptr,
                        "P2P Tx Priority:\t\t%d\n", getP2PPriority());

    for (i = 0; i < sizeof(dr)/sizeof(dr_t); i++) {
        if (dr[i].DataRate == getP2PDataRate()) {
            buf_ptr += snprintf((char *)buf_ptr, buf + buf_size - buf_ptr,
                                "P2P Data Rate:\t\t\t%d (%s)\n",
                                dr[i].DataRate, dr[i].strDataRate);
            break;
        }
    }

    if (i == sizeof(dr)/sizeof(dr_t)) {
        buf_ptr += snprintf((char *)buf_ptr, buf + buf_size - buf_ptr,
                            "P2P Data Rate:\t\t\t%d (%s)\n",
                            getIPv6DataRate(), "Unknown");
    }

    buf_ptr += snprintf((char *)buf_ptr, buf + buf_size - buf_ptr,
                        "P2P Tx Power Level:\t\t%d dBm\n",
                        getP2PTxPowerLevel());
    buf_ptr += snprintf((char *)buf_ptr, buf + buf_size - buf_ptr,
                        "IPv6 Adaptable:\t\t\t%s\n",
                        ((getIPv6Adaptable() == FALSE) ? "False" : "True"));

    for (i = 0; i < sizeof(dr)/sizeof(dr_t); i++) {
        if (dr[i].DataRate == getIPv6DataRate()) {
            buf_ptr += snprintf((char *)buf_ptr, buf + buf_size - buf_ptr,
                                "IPv6 Data Rate:\t\t\t%d (%s)\n",
                                dr[i].DataRate, dr[i].strDataRate);
            break;
        }
    }

    if (i == sizeof(dr)/sizeof(dr_t)) {
        buf_ptr += snprintf((char *)buf_ptr, buf + buf_size - buf_ptr,
                            "IPv6 Data Rate:\t\t\t%d (%s)\n",
                            getIPv6DataRate(), "Unknown");
    }

    buf_ptr += snprintf((char *)buf_ptr, buf + buf_size - buf_ptr,
                        "IPv6 Tx Power Level:\t\t%d dBm\n",
                        getIPv6TxPowerLevel());
    buf_ptr += snprintf((char *)buf_ptr, buf + buf_size - buf_ptr,
                        "IPv6 Tx Priority:\t\t%d\n", getIPv6Priority());
    buf_ptr += snprintf((char *)buf_ptr, buf + buf_size - buf_ptr,
                        "IPv6 Provider Mode:\t\t%s\n",
                        ((getIPv6ProviderMode() == FALSE) ? "False" : "True"));
    buf_ptr += snprintf((char *)buf_ptr, buf + buf_size - buf_ptr,
                        "IPv6 Radio Number:\t\t%d\n", getIPv6RadioNum());
    buf_ptr += snprintf((char *)buf_ptr, buf + buf_size - buf_ptr,
                        "Conducted to Radiated Power Offset Radio 0:\t%.1f\n",
                        (float)getConducted2RadiatedPowerOffset(0) / 10.0);
    buf_ptr += snprintf((char *)buf_ptr, buf + buf_size - buf_ptr,
                        "Conducted to Radiated Power Offset Radio 1:\t%.1f\n",
                        (float)getConducted2RadiatedPowerOffset(1) / 10.0);
    buf_ptr += snprintf((char *)buf_ptr, buf + buf_size - buf_ptr,
                        "Radio A MAC Address:\t\t%s\n",
                        nsGetSrcMacString(0, 0));
    buf_ptr += snprintf((char *)buf_ptr, buf + buf_size - buf_ptr,
                        "Radio B MAC Address:\t\t%s\n",
                        nsGetSrcMacString(1, 0));
    buf_ptr += snprintf((char *)buf_ptr, buf + buf_size - buf_ptr, "\n");
    return buf_ptr - buf;
}

/**
** Function:  getRxQueueThreshold
** @brief  Get the Rx queue threshold value
** @return The Rx queue threshold value
**
** Details: Returns the Rx queue threshold value stored in
**          cfg.rx_queue_threshold
**/
uint32_t getRxQueueThreshold(void)
{
    return cfg.rx_queue_threshold;
}

/**
** Function:  setRxQueueThreshold
** @brief  Set the Rx queue threshold value
** @param  rx_queue_threshold -- [input]The Rx queue threshold value to set
** @return RS_SUCCESS for success, error code otherwise
**
** Details: Saves the Rx queue threshold value in cfg.rx_queue_threshold
**/
static rsResultCodeType setRxQueueThreshold(uint32_t rx_queue_threshold)
{
    cfg.rx_queue_threshold = rx_queue_threshold;
    return RS_SUCCESS;
}

/**
** Function:  getCV2XPwrCtrlEnable
** @brief  Get the C-V2X power control enable flag value
** @return The C-V2X power control enable flag value
**
** Details: Returns the C-V2X power control enable flag value stored in
**          cfg.cv2x_pwr_ctrl_enable
**/
bool_t getCV2XPwrCtrlEnable(void)
{
    return cfg.cv2x_pwr_ctrl_enable;
}

/**
** Function:  setCV2XPwrCtrlEnable
** @brief  Set the C-V2X power control enable flag value
** @param  enable -- [input]TRUE to enable the C-V2X power control
** @return RS_SUCCESS for success, error code otherwise
**
** Details: Saves the station role value in cfg.cv2x_pwr_ctrl_enable
**/
static rsResultCodeType setCV2XPwrCtrlEnable(bool_t enable)
{
    cfg.cv2x_pwr_ctrl_enable = enable;
    return RS_SUCCESS;
}

/**
** Function:  getDiscardUnsecureMsg
** @brief  Get the discard unsecured messages value
** @return The discard unsecured messages value
**
** Details: Returns the discard unsecured messages value stored in
**          cfg.discard_unsecure_msg
**/
bool_t getDiscardUnsecureMsg(void)
{
    return cfg.discard_unsecure_msg;
}

/**
** Function:  setDiscardUnsecureMsg
** @brief  Set the discard unsecured messages value
** @param  discardUnsecureMsg -- [input]TRUE to enable the discarding of
**                               unsecured messages
** @return RS_SUCCESS for success, error code otherwise
**
** Details: Saves the station role value in cfg.discard_unsecure_msg
**/
static rsResultCodeType setDiscardUnsecureMsg(bool_t discardUnsecureMsg)
{
    cfg.discard_unsecure_msg = discardUnsecureMsg;
    return RS_SUCCESS;
}

/**
** Function:  getDiscardVerFailMsg
** @brief  Get the discard verification failed messages value
** @return The discard verification failed messages value
**
** Details: Returns the discard verification failed messages value stored in
**          cfg.discard_ver_fail_msg
**/
bool_t getDiscardVerFailMsg(void)
{
    return cfg.discard_ver_fail_msg;
}

/**
** Function:  setDiscardVerFailMsg
** @brief  Set the discard verification failure messages value
** @param  discardVerFailMsg -- [input]TRUE to enable the discarding of
**                              verification failed messages
** @return RS_SUCCESS for success, error code otherwise
**
** Details: Saves the station role value in cfg.discard_ver_fail_msg
**/
static rsResultCodeType setDiscardVerFailMsg(bool_t discardVerFailMsg)
{
    cfg.discard_ver_fail_msg = discardVerFailMsg;
    return RS_SUCCESS;
}

/**
** Function:  getPrimaryServiceId
** @brief  Get the primary service ID value
** @return The primary service ID value
**
** Details: Returns the primary service ID value stored in
**          cfg.primary_service_id
**/
uint32_t getPrimaryServiceId(void)
{
    return cfg.primary_service_id;
}

/**
** Function:  setPrimaryServiceId
** @brief  Set the primary service ID value
** @param  primaryServiceId -- [input]Primary service ID value
** @return RS_SUCCESS for success, error code otherwise
**
** Details: Saves the station role value in cfg.primary_service_id
**/
static rsResultCodeType setPrimaryServiceId(uint32_t primaryServiceId)
{
    cfg.primary_service_id = primaryServiceId;
    return RS_SUCCESS;
}

/**
** Function:  getSecondaryServiceId
** @brief  Get the secondary service ID value
** @return The secondary service ID value
**
** Details: Returns the secondary service ID value stored in
**          cfg.secondary_service_id
**/
uint32_t getSecondaryServiceId(void)
{
    return cfg.secondary_service_id;
}

/**
** Function:  setSecondaryServiceId
** @brief  Set the secondary service ID value
** @param  secondaryServiceId -- [input]Secondary service ID value
** @return RS_SUCCESS for success, error code otherwise
**
** Details: Saves the station role value in cfg.secondary_service_id
**/
static rsResultCodeType setSecondaryServiceId(uint32_t secondaryServiceId)
{
    cfg.secondary_service_id = secondaryServiceId;
    return RS_SUCCESS;
}

/**
** Function:  getALSMIDebugLogEnable
** @brief  Get the ALSMI debug log enable flag value
** @return The ALSMI debug log enable flag value
**
** Details: Returns the ALSMI debug log enable flag value stored in
**          cfg.alsmi_debug_log_enable
**/
uint32_t getALSMIDebugLogEnable(void)
{
    return cfg.alsmi_debug_log_enable;
}

/**
** Function:  setALSMIDebugLogEnable
** @brief  Set the ALSMI debug log enable flag value
** @param  alsmi_debug_log_enable -- [input]The ALSMI debug log enable flag
**                                   value to set
** @return RS_SUCCESS for success, error code otherwise
**
** Details: Saves the ALSMI debug log enable flag value in
**          cfg.alsmi_debug_log_enable
**/
static rsResultCodeType setALSMIDebugLogEnable(uint32_t alsmi_debug_log_enable)
{
#if !defined(NO_SECURITY)
    cfg.alsmi_debug_log_enable = alsmi_debug_log_enable;
    alsmiSetDebugLogValue();
#else
    cfg.alsmi_debug_log_enable = 0;
#endif
    return RS_SUCCESS;
}

uint32_t getALSMISignVerifyEnable(void)
{
    return cfg.alsmi_sign_verify_enable;
}
static rsResultCodeType setALSMISignVerifyEnable(uint32_t alsmi_sign_verify_enable)
{
    cfg.alsmi_sign_verify_enable = alsmi_sign_verify_enable;
    return RS_SUCCESS;
}

uint32_t getI2vSyslogEnable(void)
{
    return cfg.i2v_syslog_enable;
}
static rsResultCodeType setI2vSyslogEnable(uint32_t i2v_syslog_enable)
{
    cfg.i2v_syslog_enable = i2v_syslog_enable;
    return RS_SUCCESS;
}
