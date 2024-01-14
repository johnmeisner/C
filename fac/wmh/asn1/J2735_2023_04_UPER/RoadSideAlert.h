/**
 * This file was generated by the Objective Systems ASN1C Compiler
 * (http://www.obj-sys.com).  Version: 7.7.0, Date: 25-May-2023.
 *
 * Command:  asn1c CooperativeControlMsg-v1-ph.asn J2735-AddGrpB.asn J2735-AddGrpC.asn J2735-BasicSafetyMessage-v4.asn J2735-CommonSafetyRequest.asn J2735-Common-v7.asn J2735-EmergencyVehicleAlert.asn J2735-IntersectionCollision.asn J2735-ITIS.asn J2735-MapData-v1.asn J2735-MessageFrame-v5.asn J2735-NMEAcorrections.asn J2735-NTCIP.asn J2735-PersonalSafetyMessage.asn J2735-ProbeDataManagement.asn J2735-ProbeVehicleData.asn J2735-REGION.asn J2735-RoadSideAlert.asn J2735-RTCMcorrections.asn J2735-SignalRequestMessage.asn J2735-SignalStatusMessage.asn J2735-SPAT-v2.asn J2735-TestMessage00.asn J2735-TestMessage01.asn J2735-TestMessage02.asn J2735-TestMessage03.asn J2735-TestMessage04.asn J2735-TestMessage05.asn J2735-TestMessage06.asn J2735-TestMessage07.asn J2735-TestMessage08.asn J2735-TestMessage09.asn J2735-TestMessage10.asn J2735-TestMessage11.asn J2735-TestMessage12.asn J2735-TestMessage13.asn J2735-TestMessage14.asn J2735-TestMessage15.asn J2735-TravelerInformation-v2.asn MnvrSharingCoordMsg-v1-ph.asn PersonalSafetyMsg2-v1-ph.asn ProbeDataConfig-v0.30.asn ProbeDataReport-v0.30.asn RoadGeoAttributes-v2-ph.asn RoadSafetyMsg-v1-ph.asn RoadWeatherMessage.asn SensorDataSharingMsg-v1-ph.asn SignalControlAndPrioritizationRequest-v1-ph.asn SignalControlAndPrioritizationStatus-v1-ph.asn TollAdvertisementMsg-v0.10-ph.asn TollUsageAckMsg-v0.10-ph.asn TollUsageMsg-v0.10-ph.asn TrafficSignalPhaseAndTiming-v1-ph.asn -c -per -compact
 */
#ifndef ROADSIDEALERT_H
#define ROADSIDEALERT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "rtkey.h"
#include "rtpersrc/asn1per.h"

#include "ITIS.h"

/**
 * Header file for ASN.1 module RoadSideAlert
 */
/**************************************************************/
/*                                                            */
/*  RoadSideAlert_description                                 */
/*                                                            */
/**************************************************************/
/*
Type was extracted from 'RoadSideAlert'
*/
typedef struct EXTERN RoadSideAlert_description {
   OSSIZE n;
   ITIScodes elem[8];
} RoadSideAlert_description;

EXTERN int asn1PE_RoadSideAlert_description (OSCTXT* pctxt, RoadSideAlert_description* pvalue);

EXTERN int asn1PD_RoadSideAlert_description (OSCTXT* pctxt, RoadSideAlert_description* pvalue);

EXTERN int asn1Init_RoadSideAlert_description (RoadSideAlert_description* pvalue);

/**************************************************************/
/*                                                            */
/*  RoadSideAlert_regional                                    */
/*                                                            */
/**************************************************************/
/*
Type was extracted from 'RoadSideAlert'
*/
struct EXTERN RegionalExtension;

/* List of RegionalExtension */
typedef OSRTDList RoadSideAlert_regional;

EXTERN int asn1PE_RoadSideAlert_regional (OSCTXT* pctxt, RoadSideAlert_regional* pvalue);

EXTERN int asn1PD_RoadSideAlert_regional (OSCTXT* pctxt, RoadSideAlert_regional* pvalue);

EXTERN int asn1Init_RoadSideAlert_regional (RoadSideAlert_regional* pvalue);

/**************************************************************/
/*                                                            */
/*  RoadSideAlert                                             */
/*                                                            */
/**************************************************************/
/*
RoadSideAlert ::= SEQUENCE {
   msgCnt [0] MsgCount,
   timeStamp [1] MinuteOfTheYear OPTIONAL,
   typeEvent [2] ITIScodes,
                 -- a category and an item from that category
                 -- all ITS stds use the same types here
                 -- to explain the type of  the
                 -- alert / danger / hazard involved
   description [3] SEQUENCE (SIZE (1..8)) OF ITIScodes OPTIONAL,
                 -- up to eight ITIS code set entries to further
                 -- describe the event, give advice, or any
                 -- other ITIS codes
   priority [4] Priority OPTIONAL,
                 -- the urgency of this message, a relative
                 -- degree of merit compared with other
                 -- similar messages for this type (not other
                 -- messages being sent by the device), nor a
                 -- priority of display urgency
   heading [5] HeadingSlice OPTIONAL,
                 -- Applicable headings/direction
   extent [6] Extent OPTIONAL,
                 -- the spatial distance over which this
                 -- message applies and should be presented
                 -- to the driver
   position [7] FullPositionVector OPTIONAL,
                 -- a compact summary of the position,
                 -- heading, speed, etc. of the
                 -- event in question. Including stationary
                 -- and wide area events.
   furtherInfoID [8] FurtherInfoID OPTIONAL,
                 -- an index link to any other incident
                 -- information data that may be available
                 -- in the normal ATIS incident description
                 -- or other messages
                 -- 1~2 octets in length
   regional [9] SEQUENCE (SIZE (1..4)) OF RegionalExtension OPTIONAL,
   ...
}
*/
typedef struct EXTERN RoadSideAlert {
   struct {
      OSUINT8 timeStampPresent : 1;
      OSUINT8 descriptionPresent : 1;
      OSUINT8 priorityPresent : 1;
      OSUINT8 headingPresent : 1;
      OSUINT8 extentPresent : 1;
      OSUINT8 positionPresent : 1;
      OSUINT8 furtherInfoIDPresent : 1;
      OSUINT8 regionalPresent : 1;
   } m;
   OSUINT8 msgCnt;
   OSUINT32 timeStamp;
   ITIScodes typeEvent;
   RoadSideAlert_description description;
   struct Priority *priority;
   struct HeadingSlice *heading;
   OSUINT8 extent;
   struct FullPositionVector *position;
   struct FurtherInfoID *furtherInfoID;
   RoadSideAlert_regional regional;
   OSRTDList extElem1;
} RoadSideAlert;

EXTERN int asn1PE_RoadSideAlert (OSCTXT* pctxt, RoadSideAlert* pvalue);

EXTERN int asn1PD_RoadSideAlert (OSCTXT* pctxt, RoadSideAlert* pvalue);

EXTERN int asn1Init_RoadSideAlert (RoadSideAlert* pvalue);

#ifdef __cplusplus
}
#endif

#endif