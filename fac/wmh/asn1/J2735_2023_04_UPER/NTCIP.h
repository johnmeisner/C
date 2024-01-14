/**
 * This file was generated by the Objective Systems ASN1C Compiler
 * (http://www.obj-sys.com).  Version: 7.7.0, Date: 25-May-2023.
 *
 * Command:  asn1c CooperativeControlMsg-v1-ph.asn J2735-AddGrpB.asn J2735-AddGrpC.asn J2735-BasicSafetyMessage-v4.asn J2735-CommonSafetyRequest.asn J2735-Common-v7.asn J2735-EmergencyVehicleAlert.asn J2735-IntersectionCollision.asn J2735-ITIS.asn J2735-MapData-v1.asn J2735-MessageFrame-v5.asn J2735-NMEAcorrections.asn J2735-NTCIP.asn J2735-PersonalSafetyMessage.asn J2735-ProbeDataManagement.asn J2735-ProbeVehicleData.asn J2735-REGION.asn J2735-RoadSideAlert.asn J2735-RTCMcorrections.asn J2735-SignalRequestMessage.asn J2735-SignalStatusMessage.asn J2735-SPAT-v2.asn J2735-TestMessage00.asn J2735-TestMessage01.asn J2735-TestMessage02.asn J2735-TestMessage03.asn J2735-TestMessage04.asn J2735-TestMessage05.asn J2735-TestMessage06.asn J2735-TestMessage07.asn J2735-TestMessage08.asn J2735-TestMessage09.asn J2735-TestMessage10.asn J2735-TestMessage11.asn J2735-TestMessage12.asn J2735-TestMessage13.asn J2735-TestMessage14.asn J2735-TestMessage15.asn J2735-TravelerInformation-v2.asn MnvrSharingCoordMsg-v1-ph.asn PersonalSafetyMsg2-v1-ph.asn ProbeDataConfig-v0.30.asn ProbeDataReport-v0.30.asn RoadGeoAttributes-v2-ph.asn RoadSafetyMsg-v1-ph.asn RoadWeatherMessage.asn SensorDataSharingMsg-v1-ph.asn SignalControlAndPrioritizationRequest-v1-ph.asn SignalControlAndPrioritizationStatus-v1-ph.asn TollAdvertisementMsg-v0.10-ph.asn TollUsageAckMsg-v0.10-ph.asn TollUsageMsg-v0.10-ph.asn TrafficSignalPhaseAndTiming-v1-ph.asn -c -per -compact
 */
#ifndef NTCIP_H
#define NTCIP_H

#ifdef __cplusplus
extern "C" {
#endif

#include "rtkey.h"
#include "rtpersrc/asn1per.h"

/**
 * Header file for ASN.1 module NTCIP
 */
#include "rtxsrc/rtxEnum.h"
/**************************************************************/
/*                                                            */
/*  EssPrecipYesNo                                            */
/*                                                            */
/**************************************************************/
/*
EssPrecipYesNo ::= ENUMERATED { precip(1), noPrecip(2), error(3) }
*/
typedef enum {
   precip = 1,
   noPrecip = 2,
   error = 3
} EssPrecipYesNo_Root;

typedef OSUINT8 EssPrecipYesNo;

EXTERN int asn1PE_EssPrecipYesNo (OSCTXT* pctxt, EssPrecipYesNo value);

EXTERN int asn1PD_EssPrecipYesNo (OSCTXT* pctxt, EssPrecipYesNo* pvalue);

EXTERN extern const OSEnumItem EssPrecipYesNo_ENUMTAB[];
#define EssPrecipYesNo_ENUMTABSIZE 3

EXTERN const OSUTF8CHAR* EssPrecipYesNo_ToString (OSINT32 value);

EXTERN int EssPrecipYesNo_ToEnum (OSCTXT* pctxt,
   const OSUTF8CHAR* value, EssPrecipYesNo* pvalue);

EXTERN int EssPrecipYesNo_ToEnum2 (OSCTXT* pctxt,
   const OSUTF8CHAR* value, OSSIZE valueLen, EssPrecipYesNo* pvalue);

/**************************************************************/
/*                                                            */
/*  EssPrecipRate                                             */
/*                                                            */
/**************************************************************/
/*
EssPrecipRate ::= INTEGER (0..65535)
*/
typedef OSUINT16 EssPrecipRate;

EXTERN int asn1PE_EssPrecipRate (OSCTXT* pctxt, EssPrecipRate value);

EXTERN int asn1PD_EssPrecipRate (OSCTXT* pctxt, EssPrecipRate* pvalue);

/**************************************************************/
/*                                                            */
/*  EssPrecipSituation                                        */
/*                                                            */
/**************************************************************/
/*
EssPrecipSituation ::= ENUMERATED { other(1), unknown(2), noPrecipitation(3), unidentifiedSlight(4), unidentifiedModerate(5), unidentifiedHeavy(6), snowSlight(7), snowModerate(8), snowHeavy(9), rainSlight(10), rainModerate(11), rainHeavy(12), frozenPrecipitationSlight(13), frozenPrecipitationModerate(14), frozenPrecipitationHeavy(15) }
*/
typedef enum {
   other_1 = 1,
   unknown_5 = 2,
   noPrecipitation = 3,
   unidentifiedSlight = 4,
   unidentifiedModerate = 5,
   unidentifiedHeavy = 6,
   snowSlight = 7,
   snowModerate = 8,
   snowHeavy = 9,
   rainSlight = 10,
   rainModerate = 11,
   rainHeavy = 12,
   frozenPrecipitationSlight = 13,
   frozenPrecipitationModerate = 14,
   frozenPrecipitationHeavy = 15
} EssPrecipSituation_Root;

typedef OSUINT8 EssPrecipSituation;

EXTERN int asn1PE_EssPrecipSituation (OSCTXT* pctxt, EssPrecipSituation value);

EXTERN int asn1PD_EssPrecipSituation (OSCTXT* pctxt, EssPrecipSituation* pvalue);

EXTERN extern const OSEnumItem EssPrecipSituation_ENUMTAB[];
#define EssPrecipSituation_ENUMTABSIZE 15

EXTERN const OSUTF8CHAR* EssPrecipSituation_ToString (OSINT32 value);

EXTERN int EssPrecipSituation_ToEnum (OSCTXT* pctxt,
   const OSUTF8CHAR* value, EssPrecipSituation* pvalue);

EXTERN int EssPrecipSituation_ToEnum2 (OSCTXT* pctxt,
   const OSUTF8CHAR* value, OSSIZE valueLen, EssPrecipSituation* pvalue);

/**************************************************************/
/*                                                            */
/*  EssSolarRadiation                                         */
/*                                                            */
/**************************************************************/
/*
EssSolarRadiation ::= INTEGER (0..65535)
*/
typedef OSUINT16 EssSolarRadiation;

EXTERN int asn1PE_EssSolarRadiation (OSCTXT* pctxt, EssSolarRadiation value);

EXTERN int asn1PD_EssSolarRadiation (OSCTXT* pctxt, EssSolarRadiation* pvalue);

/**************************************************************/
/*                                                            */
/*  EssMobileFriction                                         */
/*                                                            */
/**************************************************************/
/*
EssMobileFriction ::= INTEGER (0..101)
*/
typedef OSUINT8 EssMobileFriction;

EXTERN int asn1PE_EssMobileFriction (OSCTXT* pctxt, EssMobileFriction value);

EXTERN int asn1PD_EssMobileFriction (OSCTXT* pctxt, EssMobileFriction* pvalue);

#ifdef __cplusplus
}
#endif

#endif
