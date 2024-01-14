#ifndef _NSCFG_H
#define _NSCFG_H
/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Filename: nscfg.h                                                            */
/*  Purpose: Include file for the WME module of the Radio Stack NS layer.     */
/*                                                                            */
/* Copyright (C) 2019 DENSO International America, Inc.  All Rights Reserved. */
/*                                                                            */
/* Revision History:                                                          */
/*                                                                            */
/* Date        Author        Comments                                         */
/* -------------------------------------------------------------------------- */
/* [2019-03-18][VROLLINGER]  Initial revision.                                */
/*                                                                            */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/* Include Files                                                              */
/*----------------------------------------------------------------------------*/
#include "rs.h"

/*----------------------------------------------------------------------------*/
/* Constans                                                                   */
/*----------------------------------------------------------------------------*/

#ifdef NS_VERSION
#define getNSVersion() NS_VERSION
#else
#define getNSVersion() "Unknown"
#endif

#ifdef NS_BUILDTIME
#define getNSBuildTime() NS_BUILDTIME
#else
#define getNSBuildTime() "Unknown"
#endif

/*----------------------------------------------------------------------------*/
/* Function prototypes                                                        */
/*----------------------------------------------------------------------------*/
void nsParseConfigFile1(void);

rsResultCodeType nsGetCfg(cfgType *cfg);
rsResultCodeType nsSetCfg(cfgType *cfg);

bool_t        getCV2XEnable(void);
bool_t        getDSRCEnable(void);
roleType      getStationRole(void);
int32_t       getRepeats(void);
uint8_t       getCCHNumber (uint8_t radioNum);
uint8_t       getSCHNumber (uint8_t radioNum);
bool_t        getSCHAdaptable(void);
dataRateType  getSCHDataRate(void);
int32_t       getSCHTxPowerLevel(void);
int32_t       getServiceInactivityInterval(void);
int32_t       getWSATimeoutInterval(void);
int32_t       getWSATimeoutInterval2(void);
uint8_t       getWSAChannelInterval(void);
uint8_t       getWSAPriority(void);
dataRateType  getWSADataRate(void);
uint8_t       getWSATxPowerLevel(void);
uint8_t       getP2PChannelInterval(void);
uint8_t       getP2PPriority(void);
dataRateType  getP2PDataRate(void);
uint8_t       getP2PTxPowerLevel(void);
uint8_t       getIPv6ChannelInterval(void);
uint8_t       getIPv6Priority(void);
dataRateType  getIPv6DataRate(void);
uint8_t       getIPv6TxPowerLevel(void);
bool_t        getIPv6Adaptable(void);
bool_t        getIPv6ProviderMode(void);
rsRadioType   getIPv6RadioType(void);
uint8_t       getIPv6RadioNum(void);
int16_t       getConducted2RadiatedPowerOffset(uint8_t radioNum);
int           getNSCfgInfo (char *buf, uint32_t buf_size);
uint32_t      getRxQueueThreshold(void);
bool_t        getChangeMacAddressEnable(void);
bool_t        getRcvEUFromEthEnable(void);
uint16_t      getRcvEUFromEthPortNum(void);
uint8_t      *nsGetSrcMac(uint8_t radio, uint8_t channel);
uint8_t       getSSP(uint8_t *ssp);
bool_t        getCV2XPwrCtrlEnable(void);
bool_t        getDiscardUnsecureMsg(void);
bool_t        getDiscardVerFailMsg(void);
uint32_t      getPrimaryServiceId(void);
uint32_t      getSecondaryServiceId(void);
uint32_t      getALSMIDebugLogEnable(void);
uint32_t      getALSMISignVerifyEnable(void);
uint32_t      getI2vSyslogEnable(void);
rsResultCodeType setSCHNumber (uint8_t radioNum, uint8_t sch_num);
void             nsSetRandomMacAddress(rsRadioType radioType, uint8_t radioNum);
int32_t          nsSetMK5SrcMac(uint8_t radio, uint8_t channel, uint64_t srcMacAddr);
void             nsSetSrcMac(uint8_t radio, uint8_t *srcMacAddr);
#endif  /*_NSCFG_H*/
