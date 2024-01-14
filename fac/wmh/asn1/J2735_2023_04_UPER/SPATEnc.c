/**
 * This file was generated by the Objective Systems ASN1C Compiler
 * (http://www.obj-sys.com).  Version: 7.7.0, Date: 25-May-2023.
 *
 * Command:  asn1c CooperativeControlMsg-v1-ph.asn J2735-AddGrpB.asn J2735-AddGrpC.asn J2735-BasicSafetyMessage-v4.asn J2735-CommonSafetyRequest.asn J2735-Common-v7.asn J2735-EmergencyVehicleAlert.asn J2735-IntersectionCollision.asn J2735-ITIS.asn J2735-MapData-v1.asn J2735-MessageFrame-v5.asn J2735-NMEAcorrections.asn J2735-NTCIP.asn J2735-PersonalSafetyMessage.asn J2735-ProbeDataManagement.asn J2735-ProbeVehicleData.asn J2735-REGION.asn J2735-RoadSideAlert.asn J2735-RTCMcorrections.asn J2735-SignalRequestMessage.asn J2735-SignalStatusMessage.asn J2735-SPAT-v2.asn J2735-TestMessage00.asn J2735-TestMessage01.asn J2735-TestMessage02.asn J2735-TestMessage03.asn J2735-TestMessage04.asn J2735-TestMessage05.asn J2735-TestMessage06.asn J2735-TestMessage07.asn J2735-TestMessage08.asn J2735-TestMessage09.asn J2735-TestMessage10.asn J2735-TestMessage11.asn J2735-TestMessage12.asn J2735-TestMessage13.asn J2735-TestMessage14.asn J2735-TestMessage15.asn J2735-TravelerInformation-v2.asn MnvrSharingCoordMsg-v1-ph.asn PersonalSafetyMsg2-v1-ph.asn ProbeDataConfig-v0.30.asn ProbeDataReport-v0.30.asn RoadGeoAttributes-v2-ph.asn RoadSafetyMsg-v1-ph.asn RoadWeatherMessage.asn SensorDataSharingMsg-v1-ph.asn SignalControlAndPrioritizationRequest-v1-ph.asn SignalControlAndPrioritizationStatus-v1-ph.asn TollAdvertisementMsg-v0.10-ph.asn TollUsageAckMsg-v0.10-ph.asn TollUsageMsg-v0.10-ph.asn TrafficSignalPhaseAndTiming-v1-ph.asn -c -per -compact
 */
#include "SPAT.h"
#include "REGION.h"
#include "Common.h"
#include "rtxsrc/rtxCharStr.h"

EXTERN int asn1PE_TimeIntervalConfidence (OSCTXT* pctxt, TimeIntervalConfidence value)
{
   int stat = 0;

   stat = pe_ConsUnsigned(pctxt, value,  0,  15);
   if (stat != 0) return stat;

   return (stat);
}

EXTERN int asn1PE_AdvisorySpeedType (OSCTXT* pctxt, AdvisorySpeedType value)
{
   int stat = 0;

   if (value >= 4) {
      return RTERR_INVENUM;
   }
   /* extension bit */

   rtxEncBit (pctxt, 0);

   stat = rtxEncBits(pctxt, value, 2);
   if (stat != 0) return stat;

   return (stat);
}

EXTERN int asn1PE_IntersectionStatusObject (OSCTXT* pctxt, IntersectionStatusObject* pvalue)
{
   int stat = 0;

   stat = rtxEncBitsFromByteArray (pctxt, pvalue->data, 16);
   if (stat != 0) return stat;

   return (stat);
}

EXTERN int asn1PE_MovementPhaseState (OSCTXT* pctxt, MovementPhaseState value)
{
   int stat = 0;

   if (value >= 10) {
      return RTERR_INVENUM;
   }
   stat = rtxEncBits(pctxt, value, 4);
   if (stat != 0) return stat;

   return (stat);
}

EXTERN int asn1PE_PedestrianBicycleDetect (OSCTXT* pctxt, PedestrianBicycleDetect value)
{
   int stat = 0;

   stat = rtxEncBit (pctxt, (OSBOOL)value);
   if (stat != 0) return stat;

   return (stat);
}

EXTERN int asn1PE_SpeedAdvice (OSCTXT* pctxt, SpeedAdvice value)
{
   int stat = 0;

   stat = pe_ConsUnsigned(pctxt, value,  0,  500);
   if (stat != 0) return stat;

   return (stat);
}

EXTERN int asn1PE_SPAT_TimeMark (OSCTXT* pctxt, SPAT_TimeMark value)
{
   int stat = 0;

   stat = pe_ConsUnsigned(pctxt, value,  0,  36111);
   if (stat != 0) return stat;

   return (stat);
}

EXTERN int asn1PE_WaitOnStopline (OSCTXT* pctxt, WaitOnStopline value)
{
   int stat = 0;

   stat = rtxEncBit (pctxt, (OSBOOL)value);
   if (stat != 0) return stat;

   return (stat);
}

EXTERN int asn1PE_ZoneLength (OSCTXT* pctxt, ZoneLength value)
{
   int stat = 0;

   stat = pe_ConsUnsigned(pctxt, value,  0,  10000);
   if (stat != 0) return stat;

   return (stat);
}

EXTERN int asn1PE_EnabledLaneList (OSCTXT* pctxt, EnabledLaneList* pvalue)
{
   int stat = 0;
   OSUINT32 xx1;

   /* encode length determinant */

   PU_SETSIZECONSTRAINT (pctxt, OSUINTCONST(1), OSUINTCONST(16), 0, 0);

   stat = pe_Length (pctxt, pvalue->n);
   if (stat < 0) return stat;

   /* encode elements */

   for (xx1 = 0; xx1 < pvalue->n; xx1++) {
      stat = asn1PE_LaneID (pctxt, pvalue->elem[xx1]);
      if (stat != 0) return stat;
   }

   return (stat);
}

EXTERN int asn1PE_TimeChangeDetails (OSCTXT* pctxt, TimeChangeDetails* pvalue)
{
   int stat = 0;

   rtxEncBit (pctxt, (OSBOOL)pvalue->m.startTimePresent);

   rtxEncBit (pctxt, (OSBOOL)pvalue->m.maxEndTimePresent);

   rtxEncBit (pctxt, (OSBOOL)pvalue->m.likelyTimePresent);

   rtxEncBit (pctxt, (OSBOOL)pvalue->m.confidencePresent);

   rtxEncBit (pctxt, (OSBOOL)pvalue->m.nextTimePresent);

   /* encode startTime */

   if (pvalue->m.startTimePresent) {
      stat = asn1PE_SPAT_TimeMark (pctxt, pvalue->startTime);
      if (stat != 0) return stat;
   }

   /* encode minEndTime */

   stat = asn1PE_SPAT_TimeMark (pctxt, pvalue->minEndTime);
   if (stat != 0) return stat;

   /* encode maxEndTime */

   if (pvalue->m.maxEndTimePresent) {
      stat = asn1PE_SPAT_TimeMark (pctxt, pvalue->maxEndTime);
      if (stat != 0) return stat;
   }

   /* encode likelyTime */

   if (pvalue->m.likelyTimePresent) {
      stat = asn1PE_SPAT_TimeMark (pctxt, pvalue->likelyTime);
      if (stat != 0) return stat;
   }

   /* encode confidence */

   if (pvalue->m.confidencePresent) {
      stat = asn1PE_TimeIntervalConfidence (pctxt, pvalue->confidence);
      if (stat != 0) return stat;
   }

   /* encode nextTime */

   if (pvalue->m.nextTimePresent) {
      stat = asn1PE_SPAT_TimeMark (pctxt, pvalue->nextTime);
      if (stat != 0) return stat;
   }

   return (stat);
}

EXTERN int asn1PE_AdvisorySpeed_regional (OSCTXT* pctxt, AdvisorySpeed_regional* pvalue)
{
   int stat = 0;
   OSRTDListNode* pnode;
   OSUINT32 xx1;

   /* encode length determinant */

   PU_SETSIZECONSTRAINT (pctxt, OSUINTCONST(1), OSUINTCONST(4), 0, 0);

   stat = pe_Length (pctxt, pvalue->count);
   if (stat < 0) return stat;

   /* encode elements */

   xx1 = 0;
   for (pnode = pvalue->head; pnode != 0 && xx1 < pvalue->count; pnode = pnode->next) {
      stat = asn1PE_RegionalExtension (pctxt, (RegionalExtension*)((RegionalExtension*)pnode->data));
      if (stat != 0) return stat;

      xx1++;
   }

   return (stat);
}

EXTERN int asn1PE_AdvisorySpeed (OSCTXT* pctxt, AdvisorySpeed* pvalue)
{
   int stat = 0;
   OSBOOL extbit = FALSE;

   extbit = (OSBOOL)(pvalue->extElem1.count > 0);

   /* extension bit */

   rtxEncBit (pctxt, extbit);

   rtxEncBit (pctxt, (OSBOOL)pvalue->m.speedPresent);

   rtxEncBit (pctxt, (OSBOOL)pvalue->m.confidencePresent);

   rtxEncBit (pctxt, (OSBOOL)pvalue->m.distancePresent);

   rtxEncBit (pctxt, (OSBOOL)pvalue->m.class_Present);

   rtxEncBit (pctxt, (OSBOOL)pvalue->m.regionalPresent);

   /* encode type */

   stat = asn1PE_AdvisorySpeedType (pctxt, pvalue->type);
   if (stat != 0) return stat;

   /* encode speed */

   if (pvalue->m.speedPresent) {
      stat = asn1PE_SpeedAdvice (pctxt, pvalue->speed);
      if (stat != 0) return stat;
   }

   /* encode confidence */

   if (pvalue->m.confidencePresent) {
      stat = asn1PE_SpeedConfidence (pctxt, pvalue->confidence);
      if (stat != 0) return stat;
   }

   /* encode distance */

   if (pvalue->m.distancePresent) {
      stat = asn1PE_ZoneLength (pctxt, pvalue->distance);
      if (stat != 0) return stat;
   }

   /* encode class_ */

   if (pvalue->m.class_Present) {
      stat = asn1PE_RestrictionClassID (pctxt, pvalue->class_);
      if (stat != 0) return stat;
   }

   /* encode regional */

   if (pvalue->m.regionalPresent) {
      stat = asn1PE_AdvisorySpeed_regional (pctxt, &pvalue->regional);
      if (stat != 0) return stat;
   }

   if (extbit) {

      /* encode extension optional bits length */

      stat = pe_SmallLength(pctxt, pvalue->extElem1.count);
      if (stat != 0) return stat;

      /* encode optional bits */

      stat = pe_OpenTypeExtBits(pctxt, &pvalue->extElem1);
      if (stat != 0) return stat;

      /* encode extension elements */

      if (pvalue->extElem1.count > 0) {
         stat = pe_OpenTypeExt(pctxt, &pvalue->extElem1);
         if (stat != 0) return stat;
      }
   }

   return (stat);
}

EXTERN int asn1PE_AdvisorySpeedList (OSCTXT* pctxt, AdvisorySpeedList* pvalue)
{
   int stat = 0;
   OSRTDListNode* pnode;
   OSUINT32 xx1;

   /* encode length determinant */

   PU_SETSIZECONSTRAINT (pctxt, OSUINTCONST(1), OSUINTCONST(16), 0, 0);

   stat = pe_Length (pctxt, pvalue->count);
   if (stat < 0) return stat;

   /* encode elements */

   xx1 = 0;
   for (pnode = pvalue->head; pnode != 0 && xx1 < pvalue->count; pnode = pnode->next) {
      stat = asn1PE_AdvisorySpeed (pctxt, ((AdvisorySpeed*)pnode->data));
      if (stat != 0) return stat;

      xx1++;
   }

   return (stat);
}

EXTERN int asn1PE_MovementEvent_regional (OSCTXT* pctxt, MovementEvent_regional* pvalue)
{
   int stat = 0;
   OSRTDListNode* pnode;
   OSUINT32 xx1;

   /* encode length determinant */

   PU_SETSIZECONSTRAINT (pctxt, OSUINTCONST(1), OSUINTCONST(4), 0, 0);

   stat = pe_Length (pctxt, pvalue->count);
   if (stat < 0) return stat;

   /* encode elements */

   xx1 = 0;
   for (pnode = pvalue->head; pnode != 0 && xx1 < pvalue->count; pnode = pnode->next) {
      stat = asn1PE_RegionalExtension (pctxt, (RegionalExtension*)((RegionalExtension*)pnode->data));
      if (stat != 0) return stat;

      xx1++;
   }

   return (stat);
}

EXTERN int asn1PE_MovementEvent (OSCTXT* pctxt, MovementEvent* pvalue)
{
   int stat = 0;
   OSBOOL extbit = FALSE;

   extbit = (OSBOOL)(pvalue->extElem1.count > 0);

   /* extension bit */

   rtxEncBit (pctxt, extbit);

   rtxEncBit (pctxt, (OSBOOL)pvalue->m.timingPresent);

   rtxEncBit (pctxt, (OSBOOL)pvalue->m.speedsPresent);

   rtxEncBit (pctxt, (OSBOOL)pvalue->m.regionalPresent);

   /* encode eventState */

   stat = asn1PE_MovementPhaseState (pctxt, pvalue->eventState);
   if (stat != 0) return stat;

   /* encode timing */

   if (pvalue->m.timingPresent) {
      stat = asn1PE_TimeChangeDetails (pctxt, &pvalue->timing);
      if (stat != 0) return stat;
   }

   /* encode speeds */

   if (pvalue->m.speedsPresent) {
      stat = asn1PE_AdvisorySpeedList (pctxt, &pvalue->speeds);
      if (stat != 0) return stat;
   }

   /* encode regional */

   if (pvalue->m.regionalPresent) {
      stat = asn1PE_MovementEvent_regional (pctxt, &pvalue->regional);
      if (stat != 0) return stat;
   }

   if (extbit) {

      /* encode extension optional bits length */

      stat = pe_SmallLength(pctxt, pvalue->extElem1.count);
      if (stat != 0) return stat;

      /* encode optional bits */

      stat = pe_OpenTypeExtBits(pctxt, &pvalue->extElem1);
      if (stat != 0) return stat;

      /* encode extension elements */

      if (pvalue->extElem1.count > 0) {
         stat = pe_OpenTypeExt(pctxt, &pvalue->extElem1);
         if (stat != 0) return stat;
      }
   }

   return (stat);
}

EXTERN int asn1PE_MovementEventList (OSCTXT* pctxt, MovementEventList* pvalue)
{
   int stat = 0;
   OSRTDListNode* pnode;
   OSUINT32 xx1;

   /* encode length determinant */

   PU_SETSIZECONSTRAINT (pctxt, OSUINTCONST(1), OSUINTCONST(16), 0, 0);

   stat = pe_Length (pctxt, pvalue->count);
   if (stat < 0) return stat;

   /* encode elements */

   xx1 = 0;
   for (pnode = pvalue->head; pnode != 0 && xx1 < pvalue->count; pnode = pnode->next) {
      stat = asn1PE_MovementEvent (pctxt, ((MovementEvent*)pnode->data));
      if (stat != 0) return stat;

      xx1++;
   }

   return (stat);
}

EXTERN int asn1PE_ConnectionManeuverAssist_regional (OSCTXT* pctxt, ConnectionManeuverAssist_regional* pvalue)
{
   int stat = 0;
   OSRTDListNode* pnode;
   OSUINT32 xx1;

   /* encode length determinant */

   PU_SETSIZECONSTRAINT (pctxt, OSUINTCONST(1), OSUINTCONST(4), 0, 0);

   stat = pe_Length (pctxt, pvalue->count);
   if (stat < 0) return stat;

   /* encode elements */

   xx1 = 0;
   for (pnode = pvalue->head; pnode != 0 && xx1 < pvalue->count; pnode = pnode->next) {
      stat = asn1PE_RegionalExtension (pctxt, (RegionalExtension*)((RegionalExtension*)pnode->data));
      if (stat != 0) return stat;

      xx1++;
   }

   return (stat);
}

EXTERN int asn1PE_ConnectionManeuverAssist (OSCTXT* pctxt, ConnectionManeuverAssist* pvalue)
{
   int stat = 0;
   OSBOOL extbit = FALSE;

   extbit = (OSBOOL)(pvalue->extElem1.count > 0);

   /* extension bit */

   rtxEncBit (pctxt, extbit);

   rtxEncBit (pctxt, (OSBOOL)pvalue->m.queueLengthPresent);

   rtxEncBit (pctxt, (OSBOOL)pvalue->m.availableStorageLengthPresent);

   rtxEncBit (pctxt, (OSBOOL)pvalue->m.waitOnStopPresent);

   rtxEncBit (pctxt, (OSBOOL)pvalue->m.pedBicycleDetectPresent);

   rtxEncBit (pctxt, (OSBOOL)pvalue->m.regionalPresent);

   /* encode connectionID */

   stat = asn1PE_LaneConnectionID (pctxt, pvalue->connectionID);
   if (stat != 0) return stat;

   /* encode queueLength */

   if (pvalue->m.queueLengthPresent) {
      stat = asn1PE_ZoneLength (pctxt, pvalue->queueLength);
      if (stat != 0) return stat;
   }

   /* encode availableStorageLength */

   if (pvalue->m.availableStorageLengthPresent) {
      stat = asn1PE_ZoneLength (pctxt, pvalue->availableStorageLength);
      if (stat != 0) return stat;
   }

   /* encode waitOnStop */

   if (pvalue->m.waitOnStopPresent) {
      stat = asn1PE_WaitOnStopline (pctxt, pvalue->waitOnStop);
      if (stat != 0) return stat;
   }

   /* encode pedBicycleDetect */

   if (pvalue->m.pedBicycleDetectPresent) {
      stat = asn1PE_PedestrianBicycleDetect (pctxt, pvalue->pedBicycleDetect);
      if (stat != 0) return stat;
   }

   /* encode regional */

   if (pvalue->m.regionalPresent) {
      stat = asn1PE_ConnectionManeuverAssist_regional (pctxt, &pvalue->regional);
      if (stat != 0) return stat;
   }

   if (extbit) {

      /* encode extension optional bits length */

      stat = pe_SmallLength(pctxt, pvalue->extElem1.count);
      if (stat != 0) return stat;

      /* encode optional bits */

      stat = pe_OpenTypeExtBits(pctxt, &pvalue->extElem1);
      if (stat != 0) return stat;

      /* encode extension elements */

      if (pvalue->extElem1.count > 0) {
         stat = pe_OpenTypeExt(pctxt, &pvalue->extElem1);
         if (stat != 0) return stat;
      }
   }

   return (stat);
}

EXTERN int asn1PE_ManeuverAssistList (OSCTXT* pctxt, ManeuverAssistList* pvalue)
{
   int stat = 0;
   OSRTDListNode* pnode;
   OSUINT32 xx1;

   /* encode length determinant */

   PU_SETSIZECONSTRAINT (pctxt, OSUINTCONST(1), OSUINTCONST(16), 0, 0);

   stat = pe_Length (pctxt, pvalue->count);
   if (stat < 0) return stat;

   /* encode elements */

   xx1 = 0;
   for (pnode = pvalue->head; pnode != 0 && xx1 < pvalue->count; pnode = pnode->next) {
      stat = asn1PE_ConnectionManeuverAssist (pctxt, ((ConnectionManeuverAssist*)pnode->data));
      if (stat != 0) return stat;

      xx1++;
   }

   return (stat);
}

EXTERN int asn1PE_MovementState_regional (OSCTXT* pctxt, MovementState_regional* pvalue)
{
   int stat = 0;
   OSRTDListNode* pnode;
   OSUINT32 xx1;

   /* encode length determinant */

   PU_SETSIZECONSTRAINT (pctxt, OSUINTCONST(1), OSUINTCONST(4), 0, 0);

   stat = pe_Length (pctxt, pvalue->count);
   if (stat < 0) return stat;

   /* encode elements */

   xx1 = 0;
   for (pnode = pvalue->head; pnode != 0 && xx1 < pvalue->count; pnode = pnode->next) {
      stat = asn1PE_RegionalExtension (pctxt, (RegionalExtension*)((RegionalExtension*)pnode->data));
      if (stat != 0) return stat;

      xx1++;
   }

   return (stat);
}

EXTERN int asn1PE_MovementState (OSCTXT* pctxt, MovementState* pvalue)
{
   int stat = 0;
   OSBOOL extbit = FALSE;

   extbit = (OSBOOL)(pvalue->extElem1.count > 0);

   /* extension bit */

   rtxEncBit (pctxt, extbit);

   rtxEncBit (pctxt, (OSBOOL)pvalue->m.movementNamePresent);

   rtxEncBit (pctxt, (OSBOOL)pvalue->m.maneuverAssistListPresent);

   rtxEncBit (pctxt, (OSBOOL)pvalue->m.regionalPresent);

   /* encode movementName */

   if (pvalue->m.movementNamePresent) {
      stat = asn1PE_DescriptiveName (pctxt, pvalue->movementName);
      if (stat != 0) return stat;
   }

   /* encode signalGroup */

   stat = asn1PE_SignalGroupID (pctxt, pvalue->signalGroup);
   if (stat != 0) return stat;

   /* encode state_time_speed */

   stat = asn1PE_MovementEventList (pctxt, &pvalue->state_time_speed);
   if (stat != 0) return stat;

   /* encode maneuverAssistList */

   if (pvalue->m.maneuverAssistListPresent) {
      stat = asn1PE_ManeuverAssistList (pctxt, &pvalue->maneuverAssistList);
      if (stat != 0) return stat;
   }

   /* encode regional */

   if (pvalue->m.regionalPresent) {
      stat = asn1PE_MovementState_regional (pctxt, &pvalue->regional);
      if (stat != 0) return stat;
   }

   if (extbit) {

      /* encode extension optional bits length */

      stat = pe_SmallLength(pctxt, pvalue->extElem1.count);
      if (stat != 0) return stat;

      /* encode optional bits */

      stat = pe_OpenTypeExtBits(pctxt, &pvalue->extElem1);
      if (stat != 0) return stat;

      /* encode extension elements */

      if (pvalue->extElem1.count > 0) {
         stat = pe_OpenTypeExt(pctxt, &pvalue->extElem1);
         if (stat != 0) return stat;
      }
   }

   return (stat);
}

EXTERN int asn1PE_MovementList (OSCTXT* pctxt, MovementList* pvalue)
{
   int stat = 0;
   OSRTDListNode* pnode;
   OSUINT32 xx1;

   /* encode length determinant */

   PU_SETSIZECONSTRAINT (pctxt, OSUINTCONST(1), OSUINTCONST(255), 0, 0);

   stat = pe_Length (pctxt, pvalue->count);
   if (stat < 0) return stat;

   /* encode elements */

   xx1 = 0;
   for (pnode = pvalue->head; pnode != 0 && xx1 < pvalue->count; pnode = pnode->next) {
      stat = asn1PE_MovementState (pctxt, ((MovementState*)pnode->data));
      if (stat != 0) return stat;

      xx1++;
   }

   return (stat);
}

EXTERN int asn1PE_IntersectionState_regional (OSCTXT* pctxt, IntersectionState_regional* pvalue)
{
   int stat = 0;
   OSRTDListNode* pnode;
   OSUINT32 xx1;

   /* encode length determinant */

   PU_SETSIZECONSTRAINT (pctxt, OSUINTCONST(1), OSUINTCONST(4), 0, 0);

   stat = pe_Length (pctxt, pvalue->count);
   if (stat < 0) return stat;

   /* encode elements */

   xx1 = 0;
   for (pnode = pvalue->head; pnode != 0 && xx1 < pvalue->count; pnode = pnode->next) {
      stat = asn1PE_RegionalExtension (pctxt, (RegionalExtension*)((RegionalExtension*)pnode->data));
      if (stat != 0) return stat;

      xx1++;
   }

   return (stat);
}

EXTERN int asn1PE_IntersectionState (OSCTXT* pctxt, IntersectionState* pvalue)
{
   int stat = 0;
   OSBOOL extbit = FALSE;
   OSUINT32 pos;
   void* pPerField;

   extbit = (OSBOOL)(pvalue->m.roadAuthorityIDPresent ||
   pvalue->extElem1.count > 0);

   /* extension bit */

   rtxEncBit (pctxt, extbit);

   rtxEncBit (pctxt, (OSBOOL)pvalue->m.namePresent);

   rtxEncBit (pctxt, (OSBOOL)pvalue->m.moyPresent);

   rtxEncBit (pctxt, (OSBOOL)pvalue->m.timeStampPresent);

   rtxEncBit (pctxt, (OSBOOL)pvalue->m.enabledLanesPresent);

   rtxEncBit (pctxt, (OSBOOL)pvalue->m.maneuverAssistListPresent);

   rtxEncBit (pctxt, (OSBOOL)pvalue->m.regionalPresent);

   /* encode name */

   if (pvalue->m.namePresent) {
      stat = asn1PE_DescriptiveName (pctxt, pvalue->name);
      if (stat != 0) return stat;
   }

   /* encode id */

   stat = asn1PE_IntersectionReferenceID (pctxt, (IntersectionReferenceID*)pvalue->id);
   if (stat != 0) return stat;

   /* encode revision */

   stat = asn1PE_Common_MsgCount (pctxt, pvalue->revision);
   if (stat != 0) return stat;

   /* encode status */

   stat = asn1PE_IntersectionStatusObject (pctxt, &pvalue->status);
   if (stat != 0) return stat;

   /* encode moy */

   if (pvalue->m.moyPresent) {
      stat = asn1PE_MinuteOfTheYear (pctxt, pvalue->moy);
      if (stat != 0) return stat;
   }

   /* encode timeStamp */

   if (pvalue->m.timeStampPresent) {
      stat = asn1PE_DSecond (pctxt, pvalue->timeStamp);
      if (stat != 0) return stat;
   }

   /* encode enabledLanes */

   if (pvalue->m.enabledLanesPresent) {
      stat = asn1PE_EnabledLaneList (pctxt, &pvalue->enabledLanes);
      if (stat != 0) return stat;
   }

   /* encode states */

   stat = asn1PE_MovementList (pctxt, &pvalue->states);
   if (stat != 0) return stat;

   /* encode maneuverAssistList */

   if (pvalue->m.maneuverAssistListPresent) {
      stat = asn1PE_ManeuverAssistList (pctxt, &pvalue->maneuverAssistList);
      if (stat != 0) return stat;
   }

   /* encode regional */

   if (pvalue->m.regionalPresent) {
      stat = asn1PE_IntersectionState_regional (pctxt, &pvalue->regional);
      if (stat != 0) return stat;
   }

   if (extbit) {

      /* encode extension optional bits length */

      stat = pe_SmallLength(pctxt, pvalue->extElem1.count + 1);
      if (stat != 0) return stat;

      /* encode optional bits */

      rtxEncBit (pctxt, (OSBOOL)pvalue->m.roadAuthorityIDPresent);

      stat = pe_OpenTypeExtBits(pctxt, &pvalue->extElem1);
      if (stat != 0) return stat;

      /* encode extension elements */

      if (pvalue->m.roadAuthorityIDPresent) {
         stat = pe_OpenTypeStart (pctxt, &pos, &pPerField);
         if (stat != 0) return stat;

         stat = asn1PE_RoadAuthorityID (pctxt, (RoadAuthorityID*)pvalue->roadAuthorityID);
         if (stat != 0) return stat;

         stat = pe_OpenTypeEnd (pctxt, pos, pPerField);
         if (stat != 0) return stat;
      }

      if (pvalue->extElem1.count > 0) {
         stat = pe_OpenTypeExt(pctxt, &pvalue->extElem1);
         if (stat != 0) return stat;
      }
   }

   return (stat);
}

EXTERN int asn1PE_IntersectionStateList (OSCTXT* pctxt, IntersectionStateList* pvalue)
{
   int stat = 0;
   OSRTDListNode* pnode;
   OSUINT32 xx1;

   /* encode length determinant */

   PU_SETSIZECONSTRAINT (pctxt, OSUINTCONST(1), OSUINTCONST(32), 0, 0);

   stat = pe_Length (pctxt, pvalue->count);
   if (stat < 0) return stat;

   /* encode elements */

   xx1 = 0;
   for (pnode = pvalue->head; pnode != 0 && xx1 < pvalue->count; pnode = pnode->next) {
      stat = asn1PE_IntersectionState (pctxt, ((IntersectionState*)pnode->data));
      if (stat != 0) return stat;

      xx1++;
   }

   return (stat);
}

EXTERN int asn1PE_SPAT_regional (OSCTXT* pctxt, SPAT_regional* pvalue)
{
   int stat = 0;
   OSRTDListNode* pnode;
   OSUINT32 xx1;

   /* encode length determinant */

   PU_SETSIZECONSTRAINT (pctxt, OSUINTCONST(1), OSUINTCONST(4), 0, 0);

   stat = pe_Length (pctxt, pvalue->count);
   if (stat < 0) return stat;

   /* encode elements */

   xx1 = 0;
   for (pnode = pvalue->head; pnode != 0 && xx1 < pvalue->count; pnode = pnode->next) {
      stat = asn1PE_RegionalExtension (pctxt, (RegionalExtension*)((RegionalExtension*)pnode->data));
      if (stat != 0) return stat;

      xx1++;
   }

   return (stat);
}

EXTERN int asn1PE_SPAT (OSCTXT* pctxt, SPAT* pvalue)
{
   int stat = 0;
   OSBOOL extbit = FALSE;

   extbit = (OSBOOL)(pvalue->extElem1.count > 0);

   /* extension bit */

   rtxEncBit (pctxt, extbit);

   rtxEncBit (pctxt, (OSBOOL)pvalue->m.timeStampPresent);

   rtxEncBit (pctxt, (OSBOOL)pvalue->m.namePresent);

   rtxEncBit (pctxt, (OSBOOL)pvalue->m.regionalPresent);

   /* encode timeStamp */

   if (pvalue->m.timeStampPresent) {
      stat = asn1PE_MinuteOfTheYear (pctxt, pvalue->timeStamp);
      if (stat != 0) return stat;
   }

   /* encode name */

   if (pvalue->m.namePresent) {
      stat = asn1PE_DescriptiveName (pctxt, pvalue->name);
      if (stat != 0) return stat;
   }

   /* encode intersections */

   stat = asn1PE_IntersectionStateList (pctxt, &pvalue->intersections);
   if (stat != 0) return stat;

   /* encode regional */

   if (pvalue->m.regionalPresent) {
      stat = asn1PE_SPAT_regional (pctxt, &pvalue->regional);
      if (stat != 0) return stat;
   }

   if (extbit) {

      /* encode extension optional bits length */

      stat = pe_SmallLength(pctxt, pvalue->extElem1.count);
      if (stat != 0) return stat;

      /* encode optional bits */

      stat = pe_OpenTypeExtBits(pctxt, &pvalue->extElem1);
      if (stat != 0) return stat;

      /* encode extension elements */

      if (pvalue->extElem1.count > 0) {
         stat = pe_OpenTypeExt(pctxt, &pvalue->extElem1);
         if (stat != 0) return stat;
      }
   }

   return (stat);
}

