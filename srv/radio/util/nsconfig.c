/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Filename: nsconfig.c                                                       */
/*  Purpose: Utility to configure the radio from the command line via the     */
/*           nsconfig command.                                                */
/*                                                                            */
/* Copyright (C) 2019 DENSO International America, Inc.  All Rights Reserved. */
/*                                                                            */
/* Revision History:                                                          */
/*                                                                            */
/* Date        Author        Comments                                         */
/* -------------------------------------------------------------------------- */
/* 2019-05-06  VROLLINGER    Initial revision.                                */
/*                                                                            */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/* Include Files                                                              */
/*----------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include "ris.h"
#include "ris_struct.h"
#include "type2str.h"

/*----------------------------------------------------------------------------*/
/* Constants                                                                  */
/*----------------------------------------------------------------------------*/
#define PERM_READ             0x01
#define PERM_WRITE            0x02
#define TYPE_UINT32              0
#define TYPE_FLOAT32             1
#define TYPE_MACADDR             2
#define TYPE_EDCA                3
#define TYPE_SSP                 4 /* for HD. */
#define false                    0
#define true                     1
#define MAX_VARIABLE_NAME_LEN   64

/*----------------------------------------------------------------------------*/
/* Typedefs                                                                   */
/*----------------------------------------------------------------------------*/
typedef struct
{
    cfgIdType  cfgId;
    char      *cfgIdStr;
    uint8_t    permissions;
    uint8_t    type;
} cfgIdTableEntryType;

/*----------------------------------------------------------------------------*/
/* Local variables                                                            */
/*----------------------------------------------------------------------------*/
/* Config Id table */
static const cfgIdTableEntryType cfgIdTableEntries[] = {
    {CFG_ID_CV2X_ENABLE,                               "CV2X_ENABLE",                               PERM_WRITE | PERM_READ, TYPE_UINT32},
    {CFG_ID_DSRC_ENABLE,                               "DSRC_ENABLE",                               PERM_WRITE | PERM_READ, TYPE_UINT32},
    {CFG_ID_STATION_ROLE,                              "STATION_ROLE",                              PERM_WRITE | PERM_READ, TYPE_UINT32},
    {CFG_ID_REPEATS,                                   "REPEATS",                                   PERM_WRITE | PERM_READ, TYPE_UINT32},
    {CFG_ID_CCH_NUM_RADIO_0,                           "CCH_NUM_RADIO_0",                           PERM_WRITE | PERM_READ, TYPE_UINT32},
    {CFG_ID_CCH_NUM_RADIO_1,                           "CCH_NUM_RADIO_1",                           PERM_WRITE | PERM_READ, TYPE_UINT32},
    {CFG_ID_SCH_NUM_RADIO_0,                           "SCH_NUM_RADIO_0",                                        PERM_READ, TYPE_UINT32},
    {CFG_ID_SCH_NUM_RADIO_1,                           "SCH_NUM_RADIO_1",                                        PERM_READ, TYPE_UINT32},
    {CFG_ID_MACADDRESS_RADIO_0,                        "MACADDRESS_RADIO_0",                                     PERM_READ, TYPE_MACADDR},
    {CFG_ID_MACADDRESS_RADIO_1,                        "MACADDRESS_RADIO_1",                                     PERM_READ, TYPE_MACADDR},
    {CFG_ID_SCH_TXPWRLEVEL,                            "SCH_TXPWRLEVEL",                                         PERM_READ, TYPE_UINT32},
    {CFG_ID_SCH_ADAPTABLE,                             "SCH_ADAPTABLE",                                          PERM_READ, TYPE_UINT32},
    {CFG_ID_SCH_DATARATE,                              "SCH_DATARATE",                                           PERM_READ, TYPE_UINT32},
    {CFG_ID_SERVICE_INACTIVITY_INTERVAL,               "SERVICE_INACTIVITY_INTERVAL",               PERM_WRITE | PERM_READ, TYPE_UINT32},
    {CFG_ID_WSA_TIMEOUT_INTERVAL,                      "WSA_TIMEOUT_INTERVAL",                      PERM_WRITE | PERM_READ, TYPE_UINT32},
    {CFG_ID_WSA_TIMEOUT_INTERVAL2,                     "WSA_TIMEOUT_INTERVAL2",                     PERM_WRITE | PERM_READ, TYPE_UINT32},
    {CFG_ID_RX_QUEUE_THRESHOLD,                        "RX_QUEUE_THRESHOLD",                        PERM_WRITE | PERM_READ, TYPE_UINT32},
    {CFG_ID_WSA_DATARATE,                              "WSA_DATARATE",                              PERM_WRITE | PERM_READ, TYPE_UINT32},
    {CFG_ID_WSA_TXPWRLEVEL,                            "WSA_TXPWRLEVEL",                            PERM_WRITE | PERM_READ, TYPE_UINT32},
    {CFG_ID_WSA_CHANNEL_INTERVAL,                      "WSA_CHANNEL_INTERVAL",                      PERM_WRITE | PERM_READ, TYPE_UINT32},
    {CFG_ID_WSA_PRIORITY,                              "WSA_PRIORITY",                              PERM_WRITE | PERM_READ, TYPE_UINT32},
    {CFG_ID_P2P_DATARATE,                              "P2P_DATARATE",                              PERM_WRITE | PERM_READ, TYPE_UINT32},
    {CFG_ID_P2P_TXPWRLEVEL,                            "P2P_TXPWRLEVEL",                            PERM_WRITE | PERM_READ, TYPE_UINT32},
    {CFG_ID_P2P_CHANNEL_INTERVAL,                      "P2P_CHANNEL_INTERVAL",                      PERM_WRITE | PERM_READ, TYPE_UINT32},
    {CFG_ID_P2P_PRIORITY,                              "P2P_PRIORITY",                              PERM_WRITE | PERM_READ, TYPE_UINT32},
    {CFG_ID_IPV6_RADIO_TYPE,                           "IPV6_RADIO_TYPE",                                        PERM_READ, TYPE_UINT32},
    {CFG_ID_IPV6_RADIO_NUM,                            "IPV6_RADIO_NUM",                                         PERM_READ, TYPE_UINT32},
    {CFG_ID_IPV6_TXPWRLEVEL,                           "IPV6_TXPWRLEVEL",                                        PERM_READ, TYPE_UINT32},
    {CFG_ID_IPV6_ADAPTABLE,                            "IPV6_ADAPTABLE",                                         PERM_READ, TYPE_UINT32},
    {CFG_ID_IPV6_DATARATE,                             "IPV6_DATARATE",                                          PERM_READ, TYPE_UINT32},
    {CFG_ID_IPV6_PRIORITY,                             "IPV6_PRIORITY",                                          PERM_READ, TYPE_UINT32},
    {CFG_ID_IPV6_PROVIDER_MODE,                        "IPV6_PROVIDER_MODE",                                     PERM_READ, TYPE_UINT32},
    {CFG_ID_CONDUCTED_2_RADIATED_POWER_OFFSET_RADIO_0, "CONDUCTED_2_RADIATED_POWER_OFFSET_RADIO_0", PERM_WRITE | PERM_READ, TYPE_FLOAT32},
    {CFG_ID_CONDUCTED_2_RADIATED_POWER_OFFSET_RADIO_1, "CONDUCTED_2_RADIATED_POWER_OFFSET_RADIO_1", PERM_WRITE | PERM_READ, TYPE_FLOAT32},
    {CFG_ID_CHANGE_MAC_ADDRESS_ENABLE,                 "CHANGE_MAC_ADDRESS_ENABLE",                 PERM_WRITE | PERM_READ, TYPE_UINT32},
    {CFG_ID_RCV_EU_FROM_ETH_ENABLE,                    "RCV_EU_FROM_ETH_ENABLE",                    PERM_WRITE | PERM_READ, TYPE_UINT32},
    {CFG_ID_RCV_EU_FROM_ETH_PORT_NUM,                  "RCV_EU_FROM_ETH_PORT_NUM",                  PERM_WRITE | PERM_READ, TYPE_UINT32},
    {CFG_ID_SSP,                                       "SSP",                                       PERM_WRITE | PERM_READ, TYPE_SSP},
    {CFG_ID_CV2X_PWR_CTRL_ENABLE,                      "CV2X_PWR_CTRL_ENABLE",                                   PERM_READ, TYPE_UINT32},
    {CFG_ID_DISCARD_UNSECURE_MSG,                      "DISCARD_UNSECURE_MSG",                      PERM_WRITE | PERM_READ, TYPE_UINT32},
    {CFG_ID_DISCARD_VERFAIL_MSG,                       "DISCARD_VERFAIL_MSG",                       PERM_WRITE | PERM_READ, TYPE_UINT32},
    {CFG_ID_PRIMARY_SERVICE_ID,                        "PRIMARY_SERVICE_ID",                        PERM_WRITE | PERM_READ, TYPE_UINT32},
    {CFG_ID_SECONDARY_SERVICE_ID,                      "SECONDARY_SERVICE_ID",                      PERM_WRITE | PERM_READ, TYPE_UINT32},
    {CFG_ID_EDCA_PARAMS,                               "AC_BE_CCH_RADIO_0",                         PERM_WRITE | PERM_READ, TYPE_EDCA},
    {CFG_ID_EDCA_PARAMS,                               "AC_BK_CCH_RADIO_0",                         PERM_WRITE | PERM_READ, TYPE_EDCA},
    {CFG_ID_EDCA_PARAMS,                               "AC_VI_CCH_RADIO_0",                         PERM_WRITE | PERM_READ, TYPE_EDCA},
    {CFG_ID_EDCA_PARAMS,                               "AC_VO_CCH_RADIO_0",                         PERM_WRITE | PERM_READ, TYPE_EDCA},
    {CFG_ID_EDCA_PARAMS,                               "AC_BE_CCH_RADIO_1",                         PERM_WRITE | PERM_READ, TYPE_EDCA},
    {CFG_ID_EDCA_PARAMS,                               "AC_BK_CCH_RADIO_1",                         PERM_WRITE | PERM_READ, TYPE_EDCA},
    {CFG_ID_EDCA_PARAMS,                               "AC_VI_CCH_RADIO_1",                         PERM_WRITE | PERM_READ, TYPE_EDCA},
    {CFG_ID_EDCA_PARAMS,                               "AC_VO_CCH_RADIO_1",                         PERM_WRITE | PERM_READ, TYPE_EDCA},
    {CFG_ID_EDCA_PARAMS,                               "AC_BE_SCH_RADIO_0",                         PERM_WRITE | PERM_READ, TYPE_EDCA},
    {CFG_ID_EDCA_PARAMS,                               "AC_BK_SCH_RADIO_0",                         PERM_WRITE | PERM_READ, TYPE_EDCA},
    {CFG_ID_EDCA_PARAMS,                               "AC_VI_SCH_RADIO_0",                         PERM_WRITE | PERM_READ, TYPE_EDCA},
    {CFG_ID_EDCA_PARAMS,                               "AC_VO_SCH_RADIO_0",                         PERM_WRITE | PERM_READ, TYPE_EDCA},
    {CFG_ID_EDCA_PARAMS,                               "AC_BE_SCH_RADIO_1",                         PERM_WRITE | PERM_READ, TYPE_EDCA},
    {CFG_ID_EDCA_PARAMS,                               "AC_BK_SCH_RADIO_1",                         PERM_WRITE | PERM_READ, TYPE_EDCA},
    {CFG_ID_EDCA_PARAMS,                               "AC_VI_SCH_RADIO_1",                         PERM_WRITE | PERM_READ, TYPE_EDCA},
    {CFG_ID_EDCA_PARAMS,                               "AC_VO_SCH_RADIO_1",                         PERM_WRITE | PERM_READ, TYPE_EDCA},
    {CFG_ID_ALSMI_DEBUG_LOG_ENABLE,                    "ALSMI_DEBUG_LOG_ENABLE",                    PERM_WRITE | PERM_READ, TYPE_UINT32},
};

#define NUM_CFG_IDS (sizeof(cfgIdTableEntries) / sizeof(cfgIdTableEntries[0]))

/*------------------------------------------------------------------------------
** Function:  cfgIdStrLookup
** @brief  Look up a string cfgId value and get an index into the
** @brief  cfgIdTableEntries table.
** @param  cfgIdStr -- [input]The string to look up in the table
** @return Index into the table where the string occurs. Returns -1 if not
**         found.
**
** Details: Looks at each cfgIdStr entry in the cfgIdTableEntries table, and
**          returns the index into the table once cfgIdStr looked for is found.
**
**          If it makes it all the way through the table without finding the
**          desired string, -1 is returned.
**----------------------------------------------------------------------------*/
static int cfgIdStrLookup(const char *cfgIdStr)
{
    int i;

    for (i = 0; i < NUM_CFG_IDS; i++) {
        if (strcmp(cfgIdStr, cfgIdTableEntries[i].cfgIdStr) == 0) {
            return i;
        }
    }

    return -1;
}

/*------------------------------------------------------------------------------
** Function:  parseVariableName
** @brief  Parse a string of the form "variable=value"
** @brief  cfgIdTableEntries table.
** @param  inStr        -- [input]The string to parse
** @param  variableName -- [output]Buffer to hold the variable name
** @return Pointer to the value portion of the string. Returns NULL if no
**         equals sign. Note that we may not have an equals sign if we just
**         want to print a single variable value; this is not an error.
**
** Details: Copies over all character in inStr over to variableName until a
**          space, tab, null character, or equals sign is seen. Return NULL if
**          we didn't see a variable name of if we reached the end of the
**          string. Otherwise, skip whitespaces until we see the equals sign.
**          Return NULL if no equals sign. Then skip the whitespaces after the
**          equals sign and return a pointer to the first non-whitespace.
**----------------------------------------------------------------------------*/
static char *parseVariableName(char *inStr, char *variableName)
{
    char *inPtr  = inStr;
    char *outPtr = variableName;
    int   count  = 0;

    /* Copy over characters until we see a whitespace, an equals sign, or the
     * end of the string */
    while (count <= MAX_VARIABLE_NAME_LEN) {
        if ((*inPtr == ' ')  || (*inPtr == '\t') ||
            (*inPtr == '\0') || (*inPtr == '=')) {
            break;
        }

        *outPtr++ = *inPtr++;
        count++;
    }

    /* Null terminate the variable name */
    variableName[count] = '\0';

    /* If no equals sign, or name length = 0, or name length too long, return
     * NULL */
    if ((*inPtr == '\0') || (count == 0) || (count > MAX_VARIABLE_NAME_LEN)) {
        return NULL;
    }

    /* Skip past whitespaces before '=' */
    while ((*inPtr == ' ')  || (*inPtr == '\t')) {
        inPtr++;
    }

    /* If no equals sign, return an error */
    if (*inPtr != '=') {
        return NULL;
    }

    inPtr++;

    /* Skip past whitespaces after '=' */
    while ((*inPtr == ' ')  || (*inPtr == '\t')) {
        inPtr++;
    }

    /* Return pointer to the value */
    return inPtr;
}

/*------------------------------------------------------------------------------
** Function:  getUint32Value
** @brief  Get a uint32_t value corresponding to the specified cfgIdType
** @param  cfgId -- [input]The cfgId to get to uint32_t value for
** @return The uint32_t value. Prints an error message and returns 0 if an
**         error occurs.
**
** Details: Calls wsuRisGetCfgReq() to get the value. If this fails, prints
**          an error message and returns 0. Otherwise, returns the uint32_t
**          value from the proper cfg.u. field depending on the cfgId value.
**          Converts the (perhaps boolean or enum) value to an integer if
**          necessary. Prints an error message and returns 0 if we get a cfgId
**          value we didn't expect.
**----------------------------------------------------------------------------*/
static uint32_t getUint32Value(cfgIdType cfgId)
{
    rsResultCodeType risRet;
    cfgType          cfg;

    /* For now, only support the DSRC radio. If we need to do this for the
     * C-V2X radio, we may need to re-think the configuration variable
     * names. */
    cfg.radioType = RT_DSRC;
    risRet = wsuRisGetCfgReq(cfgId, &cfg);

    if (risRet != RS_SUCCESS) {
        printf("Getting value for cfgId %s failed (%s)\n",
               cfgIdType2Str(cfgId), rsResultCodeType2Str(risRet));
        return 0;
    }

    switch(cfgId) {
    case CFG_ID_CV2X_ENABLE:
        return cfg.u.CV2XEnable;

    case CFG_ID_DSRC_ENABLE:
        return cfg.u.DSRCEnable;

    case CFG_ID_STATION_ROLE:
        return cfg.u.StationRole;

    case CFG_ID_REPEATS:
        return cfg.u.Repeats;

    case CFG_ID_CCH_NUM_RADIO_0:
    case CFG_ID_CCH_NUM_RADIO_1:
        return cfg.u.CCHNum;

    case CFG_ID_SCH_NUM_RADIO_0:
    case CFG_ID_SCH_NUM_RADIO_1:
        return cfg.u.SCHNum;

    case CFG_ID_IPV6_RADIO_TYPE:
        return (cfg.u.RadioType == RT_CV2X) ? 0 : 1;

    case CFG_ID_IPV6_RADIO_NUM:
        return (int)cfg.u.RadioNum;

    case CFG_ID_SCH_TXPWRLEVEL:
    case CFG_ID_WSA_TXPWRLEVEL:
    case CFG_ID_P2P_TXPWRLEVEL:
    case CFG_ID_IPV6_TXPWRLEVEL:
        return cfg.u.TxPwrLevel;

    case CFG_ID_SCH_ADAPTABLE:
    case CFG_ID_IPV6_ADAPTABLE:
        return cfg.u.Adaptable;

    case CFG_ID_SCH_DATARATE:
    case CFG_ID_WSA_DATARATE:
    case CFG_ID_P2P_DATARATE:
    case CFG_ID_IPV6_DATARATE:
        return cfg.u.DataRate;

    case CFG_ID_WSA_CHANNEL_INTERVAL:
    case CFG_ID_P2P_CHANNEL_INTERVAL:
        return cfg.u.ChannelInterval;

    case CFG_ID_WSA_PRIORITY:
    case CFG_ID_P2P_PRIORITY:
    case CFG_ID_IPV6_PRIORITY:
        return cfg.u.Priority;

    case CFG_ID_IPV6_PROVIDER_MODE:
        return (int)cfg.u.ProviderMode;

    case CFG_ID_SERVICE_INACTIVITY_INTERVAL:
        return cfg.u.ServiceInactivityInterval;

    case CFG_ID_RX_QUEUE_THRESHOLD:
        return cfg.u.RxQueueThreshold;

    case CFG_ID_WSA_TIMEOUT_INTERVAL:
    case CFG_ID_WSA_TIMEOUT_INTERVAL2:
        return cfg.u.WSATimeoutInterval;

    case CFG_ID_CHANGE_MAC_ADDRESS_ENABLE:
        return cfg.u.ChangeMacAddressEnable;

    case CFG_ID_RCV_EU_FROM_ETH_ENABLE:
        return cfg.u.RcvEUFromEthEnable;

    case CFG_ID_RCV_EU_FROM_ETH_PORT_NUM:
        return cfg.u.RcvEUFromEthPort;

    case CFG_ID_CV2X_PWR_CTRL_ENABLE:
        return cfg.u.CV2XPwrCtrlEnable;

    case CFG_ID_DISCARD_UNSECURE_MSG:
        return cfg.u.DiscardUnsecureMsg;

    case CFG_ID_DISCARD_VERFAIL_MSG:
        return cfg.u.DiscardVerFailMsg;

    case CFG_ID_PRIMARY_SERVICE_ID:
        return cfg.u.PrimaryServiceId;

    case CFG_ID_SECONDARY_SERVICE_ID:
        return cfg.u.SecondaryServiceId;

    case CFG_ID_ALSMI_DEBUG_LOG_ENABLE:
        return cfg.u.ALSMIDebugLogEnable;

    default:
        break;
    }

    printf("Invalid cfgId (%u)\n", cfgId);
    return 0;
}

/*------------------------------------------------------------------------------
** Function:  getFloat32Value
** @brief  Get a float32_t value corresponding to the specified cfgIdType
** @param  cfgId -- [input]The cfgId to get to float32_t value for
** @return The float32_t value. Prints an error message and returns 0 if an
**         error occurs.
**
** Details: Calls wsuRisGetCfgReq() to get the value. If this fails, prints
**          an error message and returns 0. Otherwise, returns the float32_t
**          value from the proper cfg.u. field depending on the cfgId value.
**          Prints an error message and returns 0 if we get a cfgId value we
**          didn't expect.
**----------------------------------------------------------------------------*/
static float32_t getFloat32Value(cfgIdType cfgId)
{
    rsResultCodeType risRet;
    cfgType          cfg;

    /* For now, only support the DSRC radio. If we need to do this for the
     * C-V2X radio, we may need to re-think the configuration variable
     * names. */
    cfg.radioType = RT_DSRC;
    risRet = wsuRisGetCfgReq(cfgId, &cfg);

    if (risRet != RS_SUCCESS) {
        printf("Getting value for cfgId %s failed (%s)\n",
               cfgIdType2Str(cfgId), rsResultCodeType2Str(risRet));
        return 0.0;
    }

    switch(cfgId) {
    case CFG_ID_CONDUCTED_2_RADIATED_POWER_OFFSET_RADIO_0:
    case CFG_ID_CONDUCTED_2_RADIATED_POWER_OFFSET_RADIO_1:
        return cfg.u.conducted2RadiatedPowerOffset;

    default:
        break;
    }

    printf("Invalid cfgId (%u)\n", cfgId);
    return 0.0;
}

/*------------------------------------------------------------------------------
** Function:  toHexDigit
** @brief  Convert an 8-bit value to a hexadecimal digit
** @param  digit -- [input]The 8-bit value to convert to a digit
** @return A character corresponding to the hex digit. Letters are upper case.
**
** Details: Converts the 8-bit binary value to a hexidecimal digit. If the
**          value is out of range, returns a question mark character.
**----------------------------------------------------------------------------*/
static char toHexDigit(uint8_t digit)
{
    if ((digit >= 0) && (digit <= 9)) {
        return digit + '0';
    }
    else if ((digit >= 0xA) && (digit <= 0xF)) {
        return digit - 0xA + 'A';
    }

    return '?';
}

/*------------------------------------------------------------------------------
** Function:  getSSPStringValue
** @brief  Get an SSP value as a string
** @param  cfgId -- [input]The cfgId to get to SSP value for
** @return A pointer to the string.
**
** Details: Calls wsuRisGetCfgReq() to get the value. If this fails, returns a
**          pointer to a null string (does not return NULL). Otherwise, it
**          gathers up the SSP string into a buffer and returns a pointer
**          to the buffer. Gets the SSP from cfg.u.SSP.SSPLen and
**          cfg.u.SSP.SSPValue. Returns a null string if cfg.u.SSP.SSPLen is 0.
**----------------------------------------------------------------------------*/
static char *getSSPStringValue(cfgIdType cfgId)
{
    rsResultCodeType risRet;
    cfgType          cfg;
    static char      str[(MAX_SSP_DATA * 2) + 1];
    int              i;

    if (cfgId == CFG_ID_SSP) {
        /* For now, only support the DSRC radio. If we need to do this for the
         * C-V2X radio, we may need to re-think the configuration variable
         * names. */
        cfg.radioType = RT_DSRC;
        risRet = wsuRisGetCfgReq(cfgId, &cfg);

        if (risRet != RS_SUCCESS) {
            printf("Getting value for cfgId %s failed (%s)\n",
                   cfgIdType2Str(cfgId), rsResultCodeType2Str(risRet));
            str[0] = '\0';
            return str;
        }

        if (cfg.u.SSP.SSPLen > MAX_SSP_DATA) {
            cfg.u.SSP.SSPLen = MAX_SSP_DATA;
        }

        for (i = 0; i < cfg.u.SSP.SSPLen; i++) {
            str[(i * 2) + 0] = toHexDigit((cfg.u.SSP.SSPValue[i] & 0xF0) >> 4);
            str[(i * 2) + 1] = toHexDigit((cfg.u.SSP.SSPValue[i] & 0x0F) >> 0);
        }

        str[i * 2] = '\0';
    }
    else {
        str[0] = '\0';
    }

    return str;
}

/*------------------------------------------------------------------------------
** Function:  getMacAddrStringValue
** @brief  Get a MAC address value as a string
** @param  cfgId -- [input]The cfgId to get to MAC address value for
** @return A pointer to the string.
**
** Details: Calls wsuRisGetCfgReq() to get the value. If this fails, returns a
**          pointer to a null string (does not return NULL). Otherwise, it
**          gathers up the MAC address string into a buffer and returns a
**          pointer to the buffer. Gets the MAC address from
**          cfg.u.RadioMacAddress.
**----------------------------------------------------------------------------*/
static char *getMacAddrStringValue(cfgIdType cfgId)
{
    rsResultCodeType risRet;
    cfgType          cfg;
    static char      str[LENGTH_MAC * 3];
    int              i;

    if ((cfgId == CFG_ID_MACADDRESS_RADIO_0) ||
        (cfgId == CFG_ID_MACADDRESS_RADIO_1)) {
        /* For now, only support the DSRC radio. If we need to do this for the
         * C-V2X radio, we may need to re-think the configuration variable
         * names. */
        cfg.radioType = RT_DSRC;
        risRet = wsuRisGetCfgReq(cfgId, &cfg);

        if (risRet != RS_SUCCESS) {
            printf("Getting value for cfgId %s failed (%s)\n",
                   cfgIdType2Str(cfgId), rsResultCodeType2Str(risRet));
            str[0] = '\0';
            return str;
        }

        for (i = 0; i < LENGTH_MAC; i++) {
            str[(i * 3) + 0] = toHexDigit((cfg.u.RadioMacAddress[i] & 0xF0) >> 4);
            str[(i * 3) + 1] = toHexDigit((cfg.u.RadioMacAddress[i] & 0x0F) >> 0);
            str[(i * 3) + 2] = ':';
        }

        str[(i * 3) - 1] = '\0';
    }
    else {
        str[0] = '\0';
    }

    return str;
}

/*------------------------------------------------------------------------------
** Function:  getEdcaValue
** @brief  Get an rsEdcaType value corresponding to the specified cfgIdType
**         and rsEdcaACType
** @param  cfgId     -- [input]The cfgId to get the EDCA value for
** @paran  cfgIdStr  -- [input]The cfgID string. This is used to figure out the
**                      radio number, access category, and channel type.
** @param  edcaValue -- [output]Pointer to the location to store the EDCA value
** @return void. Returns all 0's if an error occurs.
**
** Details: Calls wsuRisGetCfgReq() to get the value. If this fails, prints
**          an error message and returns all 0's. Otherwise, writes the EDCA
**          value in the structure pointed to by edcaValue.
**----------------------------------------------------------------------------*/
static void getEdcaValue(cfgIdType cfgId, char *cfgIdStr, rsEdcaType *edcaValue)
{
    rsResultCodeType risRet;
    cfgType          cfg;

    memset(edcaValue, 0, sizeof(*edcaValue));
    cfg.radioType             = RT_DSRC;
    cfg.u.edca.radioType      = RT_DSRC;
    cfg.u.edca.radioNum       = (cfgIdStr[16] == '0') ? 0 : 1;
    cfg.u.edca.accessCategory = (cfgIdStr[4] == 'E') ? AC_BE :
                                (cfgIdStr[4] == 'K') ? AC_BK :
                                (cfgIdStr[4] == 'I') ? AC_VI : AC_VO;
    cfg.u.edca.chnlType       = (cfgIdStr[6] == 'C') ? CHANTYPE_CCH :
                                                       CHANTYPE_SCH;
    risRet = wsuRisGetCfgReq(cfgId, &cfg);

    if (risRet != RS_SUCCESS) {
        printf("Getting value for %s failed (%s)\n",
               cfgIdStr, rsResultCodeType2Str(risRet));
        return;
    }

    *edcaValue = cfg.u.edca;
}

/*------------------------------------------------------------------------------
** Function:  setUInt32Value
** @brief  Set a uint32_t value for the specified cfgIdType
** @param  cfgId -- [input]The cfgId for which to set the uint32_t value
** @param  value -- [input]The value to set
** @return void
**
** Details: Sets the proper cfg.u. field to value depending on the cfgId value.
**          Then calls wsuRisSetCfgReq() to set the value. If this fails, an
**          error message is printed.
**----------------------------------------------------------------------------*/
static void setUInt32Value(cfgIdType cfgId, uint32_t value)
{
    rsResultCodeType risRet;
    cfgType          cfg;

    switch(cfgId) {
    case CFG_ID_CV2X_ENABLE:
        cfg.u.CV2XEnable = (value == 0) ? FALSE : TRUE;
        break;

    case CFG_ID_DSRC_ENABLE:
        cfg.u.DSRCEnable = (value == 0) ? FALSE : TRUE;
        break;

    case CFG_ID_STATION_ROLE:
        cfg.u.StationRole = value;
        break;

    case CFG_ID_REPEATS:
        cfg.u.Repeats = value;
        break;

    case CFG_ID_CCH_NUM_RADIO_0:
    case CFG_ID_CCH_NUM_RADIO_1:
        cfg.u.CCHNum = value;
        break;

    case CFG_ID_SCH_NUM_RADIO_0:
    case CFG_ID_SCH_NUM_RADIO_1:
        cfg.u.SCHNum = value;
        break;

    case CFG_ID_SCH_TXPWRLEVEL:
    case CFG_ID_WSA_TXPWRLEVEL:
    case CFG_ID_P2P_TXPWRLEVEL:
    case CFG_ID_IPV6_TXPWRLEVEL:
        cfg.u.TxPwrLevel = value;
        break;

    case CFG_ID_SCH_ADAPTABLE:
    case CFG_ID_IPV6_ADAPTABLE:
        cfg.u.Adaptable = value;
        break;

    case CFG_ID_SCH_DATARATE:
    case CFG_ID_WSA_DATARATE:
    case CFG_ID_P2P_DATARATE:
    case CFG_ID_IPV6_DATARATE:
        cfg.u.DataRate = value;
        break;

    case CFG_ID_WSA_CHANNEL_INTERVAL:
    case CFG_ID_P2P_CHANNEL_INTERVAL:
        cfg.u.ChannelInterval = value;
        break;

    case CFG_ID_WSA_PRIORITY:
    case CFG_ID_P2P_PRIORITY:
    case CFG_ID_IPV6_PRIORITY:
        cfg.u.Priority = value;
        break;

    case CFG_ID_IPV6_PROVIDER_MODE:
        cfg.u.ProviderMode = (value == 0) ? FALSE : TRUE;
        break;

    case CFG_ID_IPV6_RADIO_TYPE:
        cfg.u.RadioType = (value == 0) ? RT_CV2X : RT_DSRC;
        break;

    case CFG_ID_IPV6_RADIO_NUM:
        cfg.u.RadioNum = value;
        break;

    case CFG_ID_SERVICE_INACTIVITY_INTERVAL:
        cfg.u.ServiceInactivityInterval = value;
        break;

    case CFG_ID_RX_QUEUE_THRESHOLD:
        cfg.u.RxQueueThreshold = value;
        break;

    case CFG_ID_WSA_TIMEOUT_INTERVAL:
    case CFG_ID_WSA_TIMEOUT_INTERVAL2:
        cfg.u.WSATimeoutInterval = value;
        break;

    case CFG_ID_CHANGE_MAC_ADDRESS_ENABLE:
        cfg.u.ChangeMacAddressEnable = (value == 0) ? FALSE : TRUE;
        break;

    case CFG_ID_RCV_EU_FROM_ETH_ENABLE:
        cfg.u.RcvEUFromEthEnable = (value == 0) ? FALSE : TRUE;
        break;

    case CFG_ID_RCV_EU_FROM_ETH_PORT_NUM:
        cfg.u.RcvEUFromEthPort = value;
        break;

    case CFG_ID_CV2X_PWR_CTRL_ENABLE:
        cfg.u.CV2XPwrCtrlEnable = (value == 0) ? FALSE : TRUE;
        break;

    case CFG_ID_DISCARD_UNSECURE_MSG:
        cfg.u.DiscardUnsecureMsg = (value == 0) ? FALSE : TRUE;
        break;

    case CFG_ID_DISCARD_VERFAIL_MSG:
        cfg.u.DiscardVerFailMsg = (value == 0) ? FALSE : TRUE;
        break;

    case CFG_ID_PRIMARY_SERVICE_ID:
        cfg.u.PrimaryServiceId = value;
        break;

    case CFG_ID_SECONDARY_SERVICE_ID:
        cfg.u.SecondaryServiceId = value;
        break;

    case CFG_ID_ALSMI_DEBUG_LOG_ENABLE:
        cfg.u.ALSMIDebugLogEnable = (value == 0) ? FALSE : TRUE;
        break;

    default:
        printf("Invalid cfgId (%u)\n", cfgId);
        return;
    }

    /* For now, only support the DSRC radio. If we need to do this for the
     * C-V2X radio, we may need to re-think the configuration variable
     * names. */
    cfg.radioType = RT_DSRC;
    risRet = wsuRisSetCfgReq(cfgId, &cfg);

    if (risRet != RS_SUCCESS) {
        printf("Setting value for cfgId %s failed (%s)\n",
               cfgIdType2Str(cfgId), rsResultCodeType2Str(risRet));
    }
}

/*------------------------------------------------------------------------------
** Function:  setFloat32Value
** @brief  Set a float32_t value for the specified cfgIdType
** @param  cfgId -- [input]The cfgId for which to set the float32_t value
** @param  value -- [input]The value to set
** @return void
**
** Details: Sets the proper cfg.u. field to value depending on the cfgId value.
**          Then calls wsuRisSetCfgReq() to set the value. If this fails, an
**          error message is printed.
**----------------------------------------------------------------------------*/
static void setFloat32Value(cfgIdType cfgId, float32_t value)
{
    rsResultCodeType risRet;
    cfgType          cfg;

    switch(cfgId) {
    case CFG_ID_CONDUCTED_2_RADIATED_POWER_OFFSET_RADIO_0:
    case CFG_ID_CONDUCTED_2_RADIATED_POWER_OFFSET_RADIO_1:
        cfg.u.conducted2RadiatedPowerOffset = value;
        break;

    default:
        printf("Invalid cfgId (%u)\n", cfgId);
        return;
    }

    /* For now, only support the DSRC radio. If we need to do this for the
     * C-V2X radio, we may need to re-think the configuration variable
     * names. */
    cfg.radioType = RT_DSRC;
    risRet = wsuRisSetCfgReq(cfgId, &cfg);

    if (risRet != RS_SUCCESS) {
        printf("Setting value for cfgId %s failed (%s)\n",
               cfgIdType2Str(cfgId), rsResultCodeType2Str(risRet));
    }
}

/*------------------------------------------------------------------------------
** Function:  fromHexDigit
** @brief  Convert a hexadecimal digit to an 8-bit value
** @param  digit -- [input]The hex digit character to convert. Letter may be
**                  upper or lower case.
** @return An 8-bit value corresponding to the hex digit.
**
** Details: Converts the hexidecimal digit to an 8-bit binary value. If out of
**          range, zero is returned.
**----------------------------------------------------------------------------*/
uint8_t fromHexDigit(char digit)
{
    if ((digit >= '0') && (digit <= '9')) {
        return digit - '0';
    }
    else if ((digit >= 'A') && (digit <= 'F')) {
        return digit - 'A' + 0xA;
    }
    else if ((digit >= 'a') && (digit <= 'f')) {
        return digit - 'a' + 0xA;
    }

    return 0;
}

/*------------------------------------------------------------------------------
** Function:  setSSPValueFromString
** @brief  Set an SSP value from a string
** @param  cfgId -- [input]The cfgId to set to SSP value for
** @param  str   -- [input]The string to get the SSP value from
** @return void
**
** Details: Calculates the SSP length and puts it in cfg.u.SSP.SSPLen.
**          Then converts the cfg.u.SSP.SSPValue from the string. If an error
**          occurs, an error message is printed.
**
**          wsuRisSetCfgReq() is called to set the value.
**----------------------------------------------------------------------------*/
static void setSSPValueFromString(cfgIdType cfgId, char *str)
{
    rsResultCodeType risRet;
    cfgType          cfg;
    int              i;

    if (cfgId == CFG_ID_SSP) {
        cfg.u.SSP.SSPLen = strlen(str);

        if ((cfg.u.SSP.SSPLen % 2) != 0) {
            printf("Length of SSP byte string must be even\n");
            return;
        }

        cfg.u.SSP.SSPLen /= 2;
        
        if (cfg.u.SSP.SSPLen > MAX_SSP_DATA) {
            printf("SSP byte string length too long\n");
            return;
        }

        for (i = 0; i < cfg.u.SSP.SSPLen; i++) {
            cfg.u.SSP.SSPValue[i] = (fromHexDigit(str[(i * 2) + 0]) << 4) +
                                    (fromHexDigit(str[(i * 2) + 1]) << 0);
        }

        /* For now, only support the DSRC radio. If we need to do this for the
         * C-V2X radio, we may need to re-think the configuration variable
         * names. */
        cfg.radioType = RT_DSRC;
        risRet = wsuRisSetCfgReq(cfgId, &cfg);

        if (risRet != RS_SUCCESS) {
            printf("Setting value for cfgId %s failed (%s)\n",
                   cfgIdType2Str(cfgId), rsResultCodeType2Str(risRet));
        }
    }
    else {
        printf("Invalid cfgId (%u)\n", cfgId);
    }
}

/*------------------------------------------------------------------------------
** Function:  setEdcaValue
** @brief  Set an rsEdcaType value for the specified EDCA parameters
** @param  cfgId     -- [input]Should be CFG_ID_EDCA_PARAMS
** @param  cfgIdStr  -- [input]Used for the error message if necessary
** @param  edcaValue -- [input]Pointer to the location to read the EDCA values
**                      to set
** @return void
**
** Details: Sets the proper cfg.u. fields to value depending on the
**          input parameters. Then calls wsuRisSetCfgReq() to set the value.
**          If this fails, an error message is printed.
**----------------------------------------------------------------------------*/
static void setEdcaValue(cfgIdType cfgId, char *cfgIdStr, rsEdcaType *edcaValue)
{
    rsResultCodeType risRet;
    cfgType          cfg;

    if (cfgId != CFG_ID_EDCA_PARAMS) {
        printf("Invalid cfgId (%u)\n", cfgId);
        return;
    }

    /* For now, only support the DSRC radio. If we need to do this for the
     * C-V2X radio, we may need to re-think the configuration variable
     * names. */
    cfg.radioType             = RT_DSRC;
    cfg.u.edca.radioType      = edcaValue->radioType;
    cfg.u.edca.radioNum       = edcaValue->radioNum;
    cfg.u.edca.accessCategory = edcaValue->accessCategory;
    cfg.u.edca.chnlType       = edcaValue->chnlType;
    cfg.u.edca.ecw_min        = edcaValue->ecw_min;
    cfg.u.edca.ecw_max        = edcaValue->ecw_max;
    cfg.u.edca.aifs           = edcaValue->aifs;
    cfg.u.edca.txop           = edcaValue->txop;
    risRet = wsuRisSetCfgReq(cfgId, &cfg);

    if (risRet != RS_SUCCESS) {
        printf("Setting value for %s failed (%s)\n",
               cfgIdStr, rsResultCodeType2Str(risRet));
    }
}

/*------------------------------------------------------------------------------
** Function:  usage
** @brief  Prints the usage message
** @param  argc -- [input]From main()
** @param  argv -- [input]From main()
** @return void
**
** Details: Prints the usage message, using argv[0] as the program name.
**----------------------------------------------------------------------------*/
static void usage(int argc, char **argv)
{
    printf("Usage: %s <param_name>\n"
           "       %s -a\n"
           "       %s -w <param_name>=<value>\n",
           argv[0], argv[0], argv[0]);
}

/*------------------------------------------------------------------------------
** Function:  main
** @brief  The main function.
** @param  argc -- [input]Standard C command line arguments
** @param  argv -- [input]Standard C command line arguments
** @return 0 is successful, -1 if failure
**
** Details: Prints the usage message if command line arguments error. Otherwise
**          parses the command line arguments. If "-a" is specified, print the
**          values of all of the nsconfig variables. If "-w" is specified,
**          write out the value of the specified nsconfig variable. Otherwise,
**          print out the value of the specified nsconfig variable.
**----------------------------------------------------------------------------*/
int main(int argc, char **argv)
{
    int         index;
    bool_t      all   = FALSE;
    bool_t      write = FALSE;
    char        variableName[MAX_VARIABLE_NAME_LEN + 1];
    char       *valuePtr;
    uint32_t    ui32value;
    float32_t   f32value;
    rsEdcaType  edcaValue;
    bool_t      dsrcEnabled = FALSE;

    if (argc < 2) {
        usage(argc, argv);
        return -1;
    }

    /* Parse the -a or -w */
    if (argv[1][0] == '-') {
        if ((argv[1][1] == 'a') || (argv[1][1] == 'A')) {
            all = true;
        }
        else if ((argv[1][1] == 'w') || (argv[1][1] == 'W')) {
            write = true;

            if (argc < 3) {
                usage(argc, argv);
                return -1;
            }

            valuePtr = parseVariableName(argv[2], variableName);

            if (valuePtr == NULL) {
                usage(argc, argv);
                return -1;
            }

            index = cfgIdStrLookup(variableName);

            if (index < 0) {
                printf("Invalid variable name %s\n", variableName);
                return -1;
            }

            if (cfgIdTableEntries[index].type == TYPE_EDCA) {
                edcaValue.radioType      = RT_DSRC;
                edcaValue.radioNum       = (cfgIdTableEntries[index].cfgIdStr[16] == '0') ? 0 : 1;
                edcaValue.accessCategory = (cfgIdTableEntries[index].cfgIdStr[4] == 'E') ? AC_BE :
                                           (cfgIdTableEntries[index].cfgIdStr[4] == 'K') ? AC_BK :
                                           (cfgIdTableEntries[index].cfgIdStr[4] == 'I') ? AC_VI : AC_VO;
                edcaValue.chnlType       = (cfgIdTableEntries[index].cfgIdStr[6] == 'C') ? CHANTYPE_CCH : CHANTYPE_SCH;
                sscanf(valuePtr, "%u,%u,%u,%u", &edcaValue.ecw_min, &edcaValue.ecw_max, &edcaValue.aifs, &edcaValue.txop);
            }
            else if (cfgIdTableEntries[index].type == TYPE_FLOAT32) {
                sscanf(valuePtr, "%f", &f32value);
            }
            else {
                sscanf(valuePtr, "%u", &ui32value);
            }
        }
    }

    if (all) {
        /* If -a, print all values */
        for (index = 0; index < NUM_CFG_IDS; index++) {
            if (cfgIdTableEntries[index].permissions & PERM_READ) {
                switch (cfgIdTableEntries[index].type) {
                case TYPE_FLOAT32:
                    printf("%s = %.1f\n", cfgIdTableEntries[index].cfgIdStr,
                           getFloat32Value(cfgIdTableEntries[index].cfgId));
                    break;

                case TYPE_SSP:
                    printf("%s = %s\n", cfgIdTableEntries[index].cfgIdStr,
                           getSSPStringValue(cfgIdTableEntries[index].cfgId));
                    break;

                case TYPE_MACADDR:
                    /* MAC address values are for DSRC only. Do not get the
                     * values if DSRC is not enabled so as to avoid printing an
                     * error message on the console. */
                    if (dsrcEnabled) {
                        printf("%s = %s\n", cfgIdTableEntries[index].cfgIdStr,
                               getMacAddrStringValue(cfgIdTableEntries[index].cfgId));
                    }
                    break;

                case TYPE_UINT32:
                    ui32value = getUint32Value(cfgIdTableEntries[index].cfgId);
                    printf("%s = %u\n", cfgIdTableEntries[index].cfgIdStr,
                           ui32value);

                    /* If this parameter is DSRC_ENABLE, save its value.
                     * CV2X_ENABLE and DSRC_ENABLE are the first two values in
                     * the table, so the dsrcEnabled variable will be set
                     * before it is otherwise used. */
                    if (cfgIdTableEntries[index].cfgId == CFG_ID_DSRC_ENABLE) {
                        dsrcEnabled = (ui32value != 0);
                    }

                    break;

                case TYPE_EDCA:
                    /* EDCA values are for DSRC only. Do not get the values if
                     * DSRC is not enabled so as to avoid printing an error
                     * message on the console. */
                    if (dsrcEnabled) {
                        getEdcaValue(cfgIdTableEntries[index].cfgId,
                                     cfgIdTableEntries[index].cfgIdStr,
                                     &edcaValue);
                        printf("%s = %u,%u,%u,%u\n",
                               cfgIdTableEntries[index].cfgIdStr,
                               edcaValue.ecw_min,
                               edcaValue.ecw_max,
                               edcaValue.aifs,
                               edcaValue.txop);
                    }
                    break;

                default:
                    break;
                }
            }
        }
    }
    else if (write) {
        /* If -w, save the value */
        switch (cfgIdTableEntries[index].type) {
        case TYPE_FLOAT32:
            if (cfgIdTableEntries[index].permissions & PERM_WRITE) {
                setFloat32Value(cfgIdTableEntries[index].cfgId, f32value);
            }
            else {
                printf("%s is not writable.\n", variableName);
            }

            if (cfgIdTableEntries[index].permissions & PERM_READ) {
                printf("%s = %.1f\n", variableName,
                       getFloat32Value(cfgIdTableEntries[index].cfgId));

            }

            break;

        case TYPE_SSP:
            if (cfgIdTableEntries[index].permissions & PERM_WRITE) {
                setSSPValueFromString(cfgIdTableEntries[index].cfgId, valuePtr);
            }
            else {
                printf("%s is not writable.\n", variableName);
            }

            if (cfgIdTableEntries[index].permissions & PERM_READ) {
                printf("%s = %s\n", variableName,
                       getSSPStringValue(cfgIdTableEntries[index].cfgId));
            }

            break;

        case TYPE_UINT32:
            if (cfgIdTableEntries[index].permissions & PERM_WRITE) {
                setUInt32Value(cfgIdTableEntries[index].cfgId, ui32value);
            }
            else {
                printf("%s is not writable.\n", variableName);
            }

            if (cfgIdTableEntries[index].permissions & PERM_READ) {
                printf("%s = %u\n", variableName,
                       getUint32Value(cfgIdTableEntries[index].cfgId));
            }

            break;

        case TYPE_EDCA:
            if (cfgIdTableEntries[index].permissions & PERM_WRITE) {
                setEdcaValue(cfgIdTableEntries[index].cfgId,
                             cfgIdTableEntries[index].cfgIdStr,
                             &edcaValue);
            }
            else {
                printf("%s is not writable.\n", variableName);
            }

            if (cfgIdTableEntries[index].permissions & PERM_READ) {
                getEdcaValue(cfgIdTableEntries[index].cfgId,
                             cfgIdTableEntries[index].cfgIdStr,
                             &edcaValue);
                printf("%s = %u,%u,%u,%u\n",
                       cfgIdTableEntries[index].cfgIdStr,
                       edcaValue.ecw_min,
                       edcaValue.ecw_max,
                       edcaValue.aifs,
                       edcaValue.txop);
            }

            break;

        default:
            break;
        }
    }
    else {
        /* Otherwise, print the desired value */
        parseVariableName(argv[1], variableName);
        index = cfgIdStrLookup(variableName);

        if (index < 0) {
            printf("Invalid variable name %s\n", variableName);
            return -1;
        }

        if (cfgIdTableEntries[index].permissions & PERM_READ) {
            switch (cfgIdTableEntries[index].type) {
            case TYPE_FLOAT32:
                printf("%s = %.1f\n", cfgIdTableEntries[index].cfgIdStr,
                       getFloat32Value(cfgIdTableEntries[index].cfgId));
                break;

            case TYPE_SSP:
                printf("%s = %s\n", cfgIdTableEntries[index].cfgIdStr,
                       getSSPStringValue(cfgIdTableEntries[index].cfgId));
                break;

            case TYPE_MACADDR:
                printf("%s = %s\n", cfgIdTableEntries[index].cfgIdStr,
                       getMacAddrStringValue(cfgIdTableEntries[index].cfgId));
                break;

            case TYPE_UINT32:
                printf("%s = %u\n", cfgIdTableEntries[index].cfgIdStr,
                       getUint32Value(cfgIdTableEntries[index].cfgId));
                break;

            case TYPE_EDCA:
                getEdcaValue(cfgIdTableEntries[index].cfgId,
                             cfgIdTableEntries[index].cfgIdStr,
                             &edcaValue);
                printf("%s = %u,%u,%u,%u\n",
                       cfgIdTableEntries[index].cfgIdStr,
                       edcaValue.ecw_min,
                       edcaValue.ecw_max,
                       edcaValue.aifs,
                       edcaValue.txop);
                break;

            default:
                break;
            }
        }
    }

    return 0;
}

