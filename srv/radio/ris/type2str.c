/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Filename: type2str.c                                                       */
/*                                                                            */
/* Copyright (C) 2019 DENSO International America, Inc.                       */
/*                                                                            */
/* Description: Converts various enum types to their string equivalents.      */
/* Revision History:                                                          */
/*                                                                            */
/* Date        Author        Comments                                         */
/* -------------------------------------------------------------------------- */
/* [2019-03-07][VROLLINGER]  Initial revision                                 */
/*----------------------------------------------------------------------------*/
#include "type2str.h"

/*----------------------------------------------------------------------------*/
/* Functions                                                                  */
/*----------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
** Function:  cfgIdType2Str
** @brief  Converts a cfgIdType to a string for the purpose of printing
**         more easily understandable error messages.
** @param  cfgId -- [input]The cfgIdType to convert.
** @return The string equivalent of the cfgIdType.
**----------------------------------------------------------------------------*/
char *cfgIdType2Str(cfgIdType cfgId)
{
    /* Must have room for "Unknown cfgIdType (%u)" with %u expanded to a 32-bit
     * unsigned integer. */
    #define UNKNOWN_CFGIDTYPE_MSG_SIZE 32
    static char buffer[UNKNOWN_CFGIDTYPE_MSG_SIZE];

    switch (cfgId) {
    case CFG_ID_CV2X_ENABLE:
        return "CFG_ID_CV2X_ENABLE";
    case CFG_ID_DSRC_ENABLE:
        return "CFG_ID_DSRC_ENABLE";
    case CFG_ID_STATION_ROLE:
        return "CFG_ID_STATION_ROLE";
    case CFG_ID_REPEATS:
        return "CFG_ID_REPEATS";
    case CFG_ID_CCH_NUM_RADIO_0:
        return "CFG_ID_CCH_NUM_RADIO_0";
    case CFG_ID_CCH_NUM_RADIO_1:
        return "CFG_ID_CCH_NUM_RADIO_1";
    case CFG_ID_SCH_NUM_RADIO_0:
        return "CFG_ID_SCH_NUM_RADIO_0";
    case CFG_ID_SCH_NUM_RADIO_1:
        return "CFG_ID_SCH_NUM_RADIO_1";
    case CFG_ID_SCH_ADAPTABLE:
        return "CFG_ID_SCH_ADAPTABLE";
    case CFG_ID_SCH_DATARATE:
        return "CFG_ID_SCH_DATARATE";
    case CFG_ID_SCH_TXPWRLEVEL:
        return "CFG_ID_SCH_TXPWRLEVEL";
    case CFG_ID_SERVICE_INACTIVITY_INTERVAL:
        return "CFG_ID_SERVICE_INACTIVITY_INTERVAL";
    case CFG_ID_WSA_TIMEOUT_INTERVAL:
        return "CFG_ID_WSA_TIMEOUT_INTERVAL";
    case CFG_ID_WSA_TIMEOUT_INTERVAL2:
        return "CFG_ID_WSA_TIMEOUT_INTERVAL2";
    case CFG_ID_ANTENNA:
        return "CFG_ID_ANTENNA";
    case CFG_ID_ANTENNA_RADIO_0:
        return "CFG_ID_ANTENNA_RADIO_0";
    case CFG_ID_ANTENNA_RADIO_1:
        return "CFG_ID_ANTENNA_RADIO_1";
    case CFG_ID_ANTENNA_TXOVERRIDE_RADIO_0:
        return "CFG_ID_ANTENNA_TXOVERRIDE_RADIO_0";
    case CFG_ID_ANTENNA_TXOVERRIDE_RADIO_1:
        return "CFG_ID_ANTENNA_TXOVERRIDE_RADIO_1";
    case CFG_ID_MACADDRESS_RADIO_0:
        return "CFG_ID_MACADDRESS_RADIO_0";
    case CFG_ID_MACADDRESS_RADIO_1:
        return "CFG_ID_MACADDRESS_RADIO_1";
    case CFG_ID_MACADDRESS_ORIG_RADIO_0:
        return "CFG_ID_MACADDRESS_ORIG_RADIO_0";
    case CFG_ID_MACADDRESS_ORIG_RADIO_1:
        return "CFG_ID_MACADDRESS_ORIG_RADIO_1";
    case CFG_ID_RANDOM_MAC_COUNT:
        return "CFG_ID_RANDOM_MAC_COUNT";
    case CFG_ID_CCH_BSSID_RADIO_0:
        return "CFG_ID_CCH_BSSID_RADIO_0";
    case CFG_ID_CCH_BSSID_RADIO_1:
        return "CFG_ID_CCH_BSSID_RADIO_1";
    case CFG_ID_NODEQOS_RADIO_0:
        return "CFG_ID_NODEQOS_RADIO_0";
    case CFG_ID_NODEQOS_RADIO_1:
        return "CFG_ID_NODEQOS_RADIO_1";
    case CFG_ID_DUAL_RADIO_ECHO_ENABLE:
        return "CFG_ID_DUAL_RADIO_ECHO_ENABLE";
    case CFG_ID_ALLOW_DUAL_WAVE_CONFIG:
        return "CFG_ID_ALLOW_DUAL_WAVE_CONFIG";
    case CFG_ID_RECEIVER_MODE_RADIO_0:
        return "CFG_ID_RECEIVER_MODE_RADIO_0";
    case CFG_ID_RECEIVER_MODE_RADIO_1:
        return "CFG_ID_RECEIVER_MODE_RADIO_1";
    case CFG_ID_EDCA_PARAMS:
        return "CFG_ID_EDCA_PARAMS";
    case CFG_ID_TX_MAX_POWER_LEVEL:
        return "CFG_ID_TX_MAX_POWER_LEVEL";
    case CFG_ID_TX_MAX_POWER_LEVEL_RADIO_0:
        return "CFG_ID_TX_MAX_POWER_LEVEL_RADIO_0";
    case CFG_ID_TX_MAX_POWER_LEVEL_RADIO_1:
        return "CFG_ID_TX_MAX_POWER_LEVEL_RADIO_1";
    case CFG_ID_RESET_RADIO_0:
        return "CFG_ID_RESET_RADIO_0";
    case CFG_ID_RESET_RADIO_1:
        return "CFG_ID_RESET_RADIO_1";
    case CFG_ID_RX_QUEUE_THRESHOLD:
        return "CFG_ID_RX_QUEUE_THRESHOLD";
    case CFG_ID_WSA_CHANNEL_INTERVAL:
        return "CFG_ID_WSA_CHANNEL_INTERVAL";
    case CFG_ID_WSA_PRIORITY:
        return "CFG_ID_WSA_PRIORITY";
    case CFG_ID_WSA_DATARATE:
        return "CFG_ID_WSA_DATARATE";
    case CFG_ID_WSA_TXPWRLEVEL:
        return "CFG_ID_WSA_TXPWRLEVEL";
    case CFG_ID_P2P_CHANNEL_INTERVAL:
        return "CFG_ID_P2P_CHANNEL_INTERVAL";
    case CFG_ID_P2P_PRIORITY:
        return "CFG_ID_P2P_PRIORITY";
    case CFG_ID_P2P_DATARATE:
        return "CFG_ID_P2P_DATARATE";
    case CFG_ID_P2P_TXPWRLEVEL:
        return "CFG_ID_P2P_TXPWRLEVEL";
    case CFG_ID_IPV6_PRIORITY:
        return "CFG_ID_IPV6_PRIORITY";
    case CFG_ID_IPV6_DATARATE:
        return "CFG_ID_IPV6_DATARATE";
    case CFG_ID_IPV6_TXPWRLEVEL:
        return "CFG_ID_IPV6_TXPWRLEVEL";
    case CFG_ID_IPV6_ADAPTABLE:
        return "CFG_ID_IPV6_ADAPTABLE";
    case CFG_ID_IPV6_PROVIDER_MODE:
        return "CFG_ID_IPV6_PROVIDER_MODE";
    case CFG_ID_IPV6_RADIO_TYPE:
        return "CFG_ID_IPV6_RADIO_TYPE";
    case CFG_ID_IPV6_RADIO_NUM:
        return "CFG_ID_IPV6_RADIO_NUM";
    case CFG_ID_CONDUCTED_2_RADIATED_POWER_OFFSET_RADIO_0:
        return "CFG_ID_CONDUCTED_2_RADIATED_POWER_OFFSET_RADIO_0";
    case CFG_ID_CONDUCTED_2_RADIATED_POWER_OFFSET_RADIO_1:
        return "CFG_ID_CONDUCTED_2_RADIATED_POWER_OFFSET_RADIO_1";
    case CFG_ID_CHANGE_MAC_ADDRESS_ENABLE:
        return "CFG_ID_CHANGE_MAC_ADDRESS_ENABLE";
    case CFG_ID_RCV_EU_FROM_ETH_ENABLE:
        return "CFG_ID_RCV_EU_FROM_ETH_ENABLE";
    case CFG_ID_RCV_EU_FROM_ETH_PORT_NUM:
        return "CFG_ID_RCV_EU_FROM_ETH_PORT_NUM";
    case CFG_ID_SSP:
        return "CFG_ID_SSP";
    case CFG_ID_CV2X_PWR_CTRL_ENABLE:
        return "CFG_ID_CV2X_PWR_CTRL_ENABLE";
    case CFG_ID_DISCARD_UNSECURE_MSG:
        return "CFG_ID_DISCARD_UNSECURE_MSG";
    case CFG_ID_DISCARD_VERFAIL_MSG:
        return "CFG_ID_DISCARD_VERFAIL_MSG";
    case CFG_ID_ALSMI_DEBUG_LOG_ENABLE:
        return "CFG_ID_ALSMI_DEBUG_LOG_ENABLE";
    default:
        snprintf(buffer, UNKNOWN_CFGIDTYPE_MSG_SIZE, "Unknown cfgIdType (%u)", cfgId);
        return buffer;
    }
}

/*------------------------------------------------------------------------------
** Function:  rsResultCodeType2Str
** @brief  Converts an rsResultCodeType to a string for the purpose of printing
**         more easily understandable error messages.
** @param result -- [input]The rsResultCodeType to convert.
** @return The string equivalent of the rsResultCodeType.
**----------------------------------------------------------------------------*/
char *rsResultCodeType2Str(rsResultCodeType result)
{
    /* Must have room for "Unknown rsResultCodeType (%d)" with %d expanded to a
     * 32-bit unsigned integer. */
    #define UNKNOWN_RESULTCODETYPE_MSG_SIZE 40
    static char buffer[UNKNOWN_RESULTCODETYPE_MSG_SIZE];

    switch (result) {
    case RS_SUCCESS:
        return "RS_SUCCESS";
    case RS_ETHREAD:
        return "RS_ETHREAD";
    case RS_EPROCFS:
        return "RS_EPROCFS";
    case RS_EIOCTL:
        return "RS_EIOCTL";
    case RS_ESOCKOPER:
        return "RS_ESOCKOPER";
    case RS_ESOCKCRE:
        return "RS_ESOCKCRE";
    case RS_ESOCKSEND:
        return "RS_ESOCKSEND";
    case RS_ESOCKRCV:
        return "RS_ESOCKRCV";
    case RS_ENOSUPPORT:
        return "RS_ENOSUPPORT";
    case RS_ENOCFM:
        return "RS_ENOCFM";
    case RS_ERANGE:
        return "RS_ERANGE";
    case RS_EINVID:
        return "RS_EINVID";
    case RS_ENOROOM:
        return "RS_ENOROOM";
    case RS_EWRNGMODE:
        return "RS_EWRNGMODE";
    case RS_EWRNGROLE:
        return "RS_EWRNGROLE";
    case RS_EWRNGSTATE:
        return "RS_EWRNGSTATE";
    case RS_ENOPID:
        return "RS_ENOPID";
    case RS_EDUPPID:
        return "RS_EDUPPID";
    case RS_ENOSRVC:
        return "RS_ENOSRVC";
    case RS_EDUPSRVC:
        return "RS_EDUPSRVC";
    case RS_ENOWBSS:
        return "RS_ENOWBSS";
    case RS_EWBSS:
        return "RS_EWBSS";
    case RS_EWRNGVER:
        return "RS_EWRNGVER";
    case RS_EMATCH:
        return "RS_EMATCH";
    case RS_EMIB:
        return "RS_EMIB";
    case RS_ELICENSE:
        return "RS_ELICENSE";
    case RS_ESECPTR:
        return "RS_ESECPTR";
    case RS_ESECLIC:
        return "RS_ESECLIC";
    case RS_ESECNENABLED:
        return "RS_ESECNENABLED";
    case RS_SECURITY_LENGTH_TOO_LONG:
        return "RS_SECURITY_LENGTH_TOO_LONG";
    case RS_ESECOBENC:
        return "RS_ESECOBENC";
    case RS_ESECSMIRTSSF:
        return "RS_ESECSMIRTSSF";
    case RS_ESECSBF:
        return "RS_ESECSBF";
    case RS_ESECIWTSBF:
        return "RS_ESECIWTSBF";
    case RS_ESECFTRWFSF:
        return "RS_ESECFTRWFSF";
    case RS_ESECSMDSDF:
        return "RS_ESECSMDSDF";
    case RS_EDEPRECATED:
        return "RS_EDEPRECATED";
    case RS_EINTERN:
        return "RS_EINTERN";
    case RS_EINVACT:
        return "RS_EINVACT";
    case RS_ECV2XRAD:
        return "RS_ECV2XRAD";
    case RS_EWSMPHDRERR:
        return "RS_EWSMPHDRERR";
    case RS_UNKNTPID:
        return "RS_UNKNTPID";
    case RS_PSIDENCERR:
        return "RS_PSIDENCERR";
    case RS_NOTENABLED:
        return "RS_NOTENABLED";
    case RS_ESECADDF:
        return "RS_ESECADDF";
    case RS_ESECADCF:
        return "RS_ESECADCF";
    case RS_ESECSIGNF:
        return "RS_ESECSIGNF";
    case RS_ESECA16092F:
        return "RS_ESECA16092F";
    case RS_ESECINVSEQN:
        return "RS_ESECINVSEQN";
    case RS_ESECCSMP:
        return "RS_ESECCSMP";
    case RS_ESECINITF:
        return "RS_ESECINITF";
    case RS_ESECTERMF:
        return "RS_ESECTERMF";
    case RS_ESECCERTCHF:
        return "RS_ESECCERTCHF";
    case RS_EGETWSAINFO:
        return "RS_EGETWSAINFO";
    case RS_ECHNLCONFL:
        return "RS_ECHNLCONFL";
    case RS_ECCL:
        return "RS_ECCL";
    case RS_MKXSTAT_FAIL_INTERN_ERR:
        return "RS_MKXSTAT_FAIL_INTERN_ERR";
    case RS_MKXSTAT_FAIL_INV_HANDLE:
        return "RS_MKXSTAT_FAIL_INV_HANDLE";
    case RS_MKXSTAT_FAIL_INV_CFG:
        return "RS_MKXSTAT_FAIL_INV_CFG";
    case RS_MKXSTAT_FAIL_INV_PARAM:
        return "RS_MKXSTAT_FAIL_INV_PARAM";
    case RS_MKXSTAT_FAIL_AUTOCAL_REJ_SIMUL:
        return "RS_MKXSTAT_FAIL_AUTOCAL_REJ_SIMUL";
    case RS_MKXSTAT_FAIL_AUTOCAL_REJ_UNCFG:
        return "RS_MKXSTAT_FAIL_AUTOCAL_REJ_UNCFG";
    case RS_MKXSTAT_FAIL_RADIOCFG_MAX:
        return "RS_MKXSTAT_FAIL_RADIOCFG_MAX";
    case RS_MKXSTAT_FAIL_RADIOCFG_MIN:
        return "RS_MKXSTAT_FAIL_RADIOCFG_MIN";
    case RS_MKXSTAT_TX_FAIL_TTL:
        return "RS_MKXSTAT_TX_FAIL_TTL";
    case RS_MKXSTAT_TX_FAIL_RETR:
        return "RS_MKXSTAT_TX_FAIL_RETR";
    case RS_MKXSTAT_TX_FAIL_QFULL:
        return "RS_MKXSTAT_TX_FAIL_QFULL";
    case RS_MKXSTAT_TX_FAIL_RADIO_NOT_PRES:
        return "RS_MKXSTAT_TX_FAIL_RADIO_NOT_PRES";
    case RS_MKXSTAT_TX_FAIL_MALFORMED:
        return "RS_MKXSTAT_TX_FAIL_MALFORMED";
    case RS_MKXSTAT_TX_FAIL_RADIO_UNCFG:
        return "RS_MKXSTAT_TX_FAIL_RADIO_UNCFG";
    case RS_MKXSTAT_TX_FAIL_PKT_TOO_LONG:
        return "RS_MKXSTAT_TX_FAIL_PKT_TOO_LONG";
    case RS_MKXSTAT_SEC_ACC_NOT_PRES:
        return "RS_MKXSTAT_SEC_ACC_NOT_PRES";
    case RS_MKXSTAT_SEC_FIFO_FULL:
        return "RS_MKXSTAT_SEC_FIFO_FULL";
    case RS_MKXSTAT_RESERVED:
        return "RS_MKXSTAT_RESERVED";
    case RS_MKXSTAT_UNKNOWN:
        return "RS_MKXSTAT_UNKNOWN";
    case RS_ESECUPKTT:
        return "RS_ESECUPKTT";
    case RS_ESECINVPSID:
        return "RS_ESECINVPSID";
    case RS_ESECCERTCHGIP:
        return "RS_ESECCERTCHGIP";
    case RS_EVERWSAFAIL:
        return "RS_EVERWSAFAIL";
    default:
        snprintf(buffer, UNKNOWN_RESULTCODETYPE_MSG_SIZE, "Unknown rsResultCodeType (%d)", result);
        return buffer;
    }
}

