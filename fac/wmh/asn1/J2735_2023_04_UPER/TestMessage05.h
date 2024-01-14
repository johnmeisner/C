/**
 * This file was generated by the Objective Systems ASN1C Compiler
 * (http://www.obj-sys.com).  Version: 7.7.0, Date: 25-May-2023.
 *
 * Command:  asn1c CooperativeControlMsg-v1-ph.asn J2735-AddGrpB.asn J2735-AddGrpC.asn J2735-BasicSafetyMessage-v4.asn J2735-CommonSafetyRequest.asn J2735-Common-v7.asn J2735-EmergencyVehicleAlert.asn J2735-IntersectionCollision.asn J2735-ITIS.asn J2735-MapData-v1.asn J2735-MessageFrame-v5.asn J2735-NMEAcorrections.asn J2735-NTCIP.asn J2735-PersonalSafetyMessage.asn J2735-ProbeDataManagement.asn J2735-ProbeVehicleData.asn J2735-REGION.asn J2735-RoadSideAlert.asn J2735-RTCMcorrections.asn J2735-SignalRequestMessage.asn J2735-SignalStatusMessage.asn J2735-SPAT-v2.asn J2735-TestMessage00.asn J2735-TestMessage01.asn J2735-TestMessage02.asn J2735-TestMessage03.asn J2735-TestMessage04.asn J2735-TestMessage05.asn J2735-TestMessage06.asn J2735-TestMessage07.asn J2735-TestMessage08.asn J2735-TestMessage09.asn J2735-TestMessage10.asn J2735-TestMessage11.asn J2735-TestMessage12.asn J2735-TestMessage13.asn J2735-TestMessage14.asn J2735-TestMessage15.asn J2735-TravelerInformation-v2.asn MnvrSharingCoordMsg-v1-ph.asn PersonalSafetyMsg2-v1-ph.asn ProbeDataConfig-v0.30.asn ProbeDataReport-v0.30.asn RoadGeoAttributes-v2-ph.asn RoadSafetyMsg-v1-ph.asn RoadWeatherMessage.asn SensorDataSharingMsg-v1-ph.asn SignalControlAndPrioritizationRequest-v1-ph.asn SignalControlAndPrioritizationStatus-v1-ph.asn TollAdvertisementMsg-v0.10-ph.asn TollUsageAckMsg-v0.10-ph.asn TollUsageMsg-v0.10-ph.asn TrafficSignalPhaseAndTiming-v1-ph.asn -c -per -compact
 */
#ifndef TESTMESSAGE05_H
#define TESTMESSAGE05_H

#ifdef __cplusplus
extern "C" {
#endif

#include "rtkey.h"
#include "rtpersrc/asn1per.h"

/**
 * Header file for ASN.1 module TestMessage05
 */
/**************************************************************/
/*                                                            */
/*  TestMessage05                                             */
/*                                                            */
/**************************************************************/
/*
TestMessage05 ::= SEQUENCE {
   header [0] Header OPTIONAL,
   regional [1] RegionalExtension OPTIONAL,
   ...
}
*/
typedef struct EXTERN TestMessage05 {
   struct {
      OSUINT8 headerPresent : 1;
      OSUINT8 regionalPresent : 1;
   } m;
   struct Header *header;
   struct RegionalExtension *regional;
   OSRTDList extElem1;
} TestMessage05;

EXTERN int asn1PE_TestMessage05 (OSCTXT* pctxt, TestMessage05* pvalue);

EXTERN int asn1PD_TestMessage05 (OSCTXT* pctxt, TestMessage05* pvalue);

EXTERN int asn1Init_TestMessage05 (TestMessage05* pvalue);

#ifdef __cplusplus
}
#endif

#endif
