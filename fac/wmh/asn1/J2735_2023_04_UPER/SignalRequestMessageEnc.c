/**
 * This file was generated by the Objective Systems ASN1C Compiler
 * (http://www.obj-sys.com).  Version: 7.7.0, Date: 25-May-2023.
 *
 * Command:  asn1c CooperativeControlMsg-v1-ph.asn J2735-AddGrpB.asn J2735-AddGrpC.asn J2735-BasicSafetyMessage-v4.asn J2735-CommonSafetyRequest.asn J2735-Common-v7.asn J2735-EmergencyVehicleAlert.asn J2735-IntersectionCollision.asn J2735-ITIS.asn J2735-MapData-v1.asn J2735-MessageFrame-v5.asn J2735-NMEAcorrections.asn J2735-NTCIP.asn J2735-PersonalSafetyMessage.asn J2735-ProbeDataManagement.asn J2735-ProbeVehicleData.asn J2735-REGION.asn J2735-RoadSideAlert.asn J2735-RTCMcorrections.asn J2735-SignalRequestMessage.asn J2735-SignalStatusMessage.asn J2735-SPAT-v2.asn J2735-TestMessage00.asn J2735-TestMessage01.asn J2735-TestMessage02.asn J2735-TestMessage03.asn J2735-TestMessage04.asn J2735-TestMessage05.asn J2735-TestMessage06.asn J2735-TestMessage07.asn J2735-TestMessage08.asn J2735-TestMessage09.asn J2735-TestMessage10.asn J2735-TestMessage11.asn J2735-TestMessage12.asn J2735-TestMessage13.asn J2735-TestMessage14.asn J2735-TestMessage15.asn J2735-TravelerInformation-v2.asn MnvrSharingCoordMsg-v1-ph.asn PersonalSafetyMsg2-v1-ph.asn ProbeDataConfig-v0.30.asn ProbeDataReport-v0.30.asn RoadGeoAttributes-v2-ph.asn RoadSafetyMsg-v1-ph.asn RoadWeatherMessage.asn SensorDataSharingMsg-v1-ph.asn SignalControlAndPrioritizationRequest-v1-ph.asn SignalControlAndPrioritizationStatus-v1-ph.asn TollAdvertisementMsg-v0.10-ph.asn TollUsageAckMsg-v0.10-ph.asn TollUsageMsg-v0.10-ph.asn TrafficSignalPhaseAndTiming-v1-ph.asn -c -per -compact
 */
#include "SignalRequestMessage.h"
#include "REGION.h"
#include "Common.h"
#include "rtxsrc/rtxCharStr.h"

EXTERN int asn1PE_DeltaTime (OSCTXT* pctxt, DeltaTime value)
{
   int stat = 0;

   stat = pe_ConsInteger(pctxt, value,  -122,  121);
   if (stat != 0) return stat;

   return (stat);
}

EXTERN int asn1PE_PriorityRequestType (OSCTXT* pctxt, PriorityRequestType value)
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

EXTERN int asn1PE_TransitVehicleOccupancy (OSCTXT* pctxt, TransitVehicleOccupancy value)
{
   int stat = 0;

   if (value >= 8) {
      return RTERR_INVENUM;
   }
   stat = rtxEncBits(pctxt, value, 3);
   if (stat != 0) return stat;

   return (stat);
}

EXTERN int asn1PE_TransitVehicleStatus (OSCTXT* pctxt, TransitVehicleStatus* pvalue)
{
   int stat = 0;

   stat = rtxEncBitsFromByteArray (pctxt, pvalue->data, 8);
   if (stat != 0) return stat;

   return (stat);
}

EXTERN int asn1PE_SignalRequest_regional (OSCTXT* pctxt, SignalRequest_regional* pvalue)
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

EXTERN int asn1PE_SignalRequest (OSCTXT* pctxt, SignalRequest* pvalue)
{
   int stat = 0;
   OSBOOL extbit = FALSE;

   extbit = (OSBOOL)(pvalue->extElem1.count > 0);

   /* extension bit */

   rtxEncBit (pctxt, extbit);

   rtxEncBit (pctxt, (OSBOOL)pvalue->m.outBoundLanePresent);

   rtxEncBit (pctxt, (OSBOOL)pvalue->m.regionalPresent);

   /* encode id */

   stat = asn1PE_IntersectionReferenceID (pctxt, (IntersectionReferenceID*)pvalue->id);
   if (stat != 0) return stat;

   /* encode requestID */

   stat = asn1PE_RequestID (pctxt, pvalue->requestID);
   if (stat != 0) return stat;

   /* encode requestType */

   stat = asn1PE_PriorityRequestType (pctxt, pvalue->requestType);
   if (stat != 0) return stat;

   /* encode inBoundLane */

   stat = asn1PE_IntersectionAccessPoint (pctxt, (IntersectionAccessPoint*)pvalue->inBoundLane);
   if (stat != 0) return stat;

   /* encode outBoundLane */

   if (pvalue->m.outBoundLanePresent) {
      stat = asn1PE_IntersectionAccessPoint (pctxt, (IntersectionAccessPoint*)pvalue->outBoundLane);
      if (stat != 0) return stat;
   }

   /* encode regional */

   if (pvalue->m.regionalPresent) {
      stat = asn1PE_SignalRequest_regional (pctxt, &pvalue->regional);
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

EXTERN int asn1PE_SignalRequestPackage_regional (OSCTXT* pctxt, SignalRequestPackage_regional* pvalue)
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

EXTERN int asn1PE_SignalRequestPackage (OSCTXT* pctxt, SignalRequestPackage* pvalue)
{
   int stat = 0;
   OSBOOL extbit = FALSE;

   extbit = (OSBOOL)(pvalue->extElem1.count > 0);

   /* extension bit */

   rtxEncBit (pctxt, extbit);

   rtxEncBit (pctxt, (OSBOOL)pvalue->m.minutePresent);

   rtxEncBit (pctxt, (OSBOOL)pvalue->m.secondPresent);

   rtxEncBit (pctxt, (OSBOOL)pvalue->m.durationPresent);

   rtxEncBit (pctxt, (OSBOOL)pvalue->m.regionalPresent);

   /* encode request */

   stat = asn1PE_SignalRequest (pctxt, &pvalue->request);
   if (stat != 0) return stat;

   /* encode minute */

   if (pvalue->m.minutePresent) {
      stat = asn1PE_MinuteOfTheYear (pctxt, pvalue->minute);
      if (stat != 0) return stat;
   }

   /* encode second */

   if (pvalue->m.secondPresent) {
      stat = asn1PE_DSecond (pctxt, pvalue->second);
      if (stat != 0) return stat;
   }

   /* encode duration */

   if (pvalue->m.durationPresent) {
      stat = asn1PE_DSecond (pctxt, pvalue->duration);
      if (stat != 0) return stat;
   }

   /* encode regional */

   if (pvalue->m.regionalPresent) {
      stat = asn1PE_SignalRequestPackage_regional (pctxt, &pvalue->regional);
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

EXTERN int asn1PE_SignalRequestList (OSCTXT* pctxt, SignalRequestList* pvalue)
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
      stat = asn1PE_SignalRequestPackage (pctxt, ((SignalRequestPackage*)pnode->data));
      if (stat != 0) return stat;

      xx1++;
   }

   return (stat);
}

EXTERN int asn1PE_RequestorPositionVector (OSCTXT* pctxt, RequestorPositionVector* pvalue)
{
   int stat = 0;
   OSBOOL extbit = FALSE;

   extbit = (OSBOOL)(pvalue->extElem1.count > 0);

   /* extension bit */

   rtxEncBit (pctxt, extbit);

   rtxEncBit (pctxt, (OSBOOL)pvalue->m.headingPresent);

   rtxEncBit (pctxt, (OSBOOL)pvalue->m.speedPresent);

   /* encode position */

   stat = asn1PE_Position3D (pctxt, (Position3D*)pvalue->position);
   if (stat != 0) return stat;

   /* encode heading */

   if (pvalue->m.headingPresent) {
      stat = asn1PE_Common_Angle (pctxt, pvalue->heading);
      if (stat != 0) return stat;
   }

   /* encode speed */

   if (pvalue->m.speedPresent) {
      stat = asn1PE_TransmissionAndSpeed (pctxt, (TransmissionAndSpeed*)pvalue->speed);
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

EXTERN int asn1PE_RequestorDescription_regional (OSCTXT* pctxt, RequestorDescription_regional* pvalue)
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

EXTERN int asn1PE_RequestorDescription (OSCTXT* pctxt, RequestorDescription* pvalue)
{
   int stat = 0;
   OSBOOL extbit = FALSE;

   extbit = (OSBOOL)(pvalue->extElem1.count > 0);

   /* extension bit */

   rtxEncBit (pctxt, extbit);

   rtxEncBit (pctxt, (OSBOOL)pvalue->m.typePresent);

   rtxEncBit (pctxt, (OSBOOL)pvalue->m.positionPresent);

   rtxEncBit (pctxt, (OSBOOL)pvalue->m.namePresent);

   rtxEncBit (pctxt, (OSBOOL)pvalue->m.routeNamePresent);

   rtxEncBit (pctxt, (OSBOOL)pvalue->m.transitStatusPresent);

   rtxEncBit (pctxt, (OSBOOL)pvalue->m.transitOccupancyPresent);

   rtxEncBit (pctxt, (OSBOOL)pvalue->m.transitSchedulePresent);

   rtxEncBit (pctxt, (OSBOOL)pvalue->m.regionalPresent);

   /* encode id */

   stat = asn1PE_VehicleID (pctxt, (VehicleID*)pvalue->id);
   if (stat != 0) return stat;

   /* encode type */

   if (pvalue->m.typePresent) {
      stat = asn1PE_RequestorType (pctxt, (RequestorType*)pvalue->type);
      if (stat != 0) return stat;
   }

   /* encode position */

   if (pvalue->m.positionPresent) {
      stat = asn1PE_RequestorPositionVector (pctxt, &pvalue->position);
      if (stat != 0) return stat;
   }

   /* encode name */

   if (pvalue->m.namePresent) {
      stat = asn1PE_DescriptiveName (pctxt, pvalue->name);
      if (stat != 0) return stat;
   }

   /* encode routeName */

   if (pvalue->m.routeNamePresent) {
      stat = asn1PE_DescriptiveName (pctxt, pvalue->routeName);
      if (stat != 0) return stat;
   }

   /* encode transitStatus */

   if (pvalue->m.transitStatusPresent) {
      stat = asn1PE_TransitVehicleStatus (pctxt, &pvalue->transitStatus);
      if (stat != 0) return stat;
   }

   /* encode transitOccupancy */

   if (pvalue->m.transitOccupancyPresent) {
      stat = asn1PE_TransitVehicleOccupancy (pctxt, pvalue->transitOccupancy);
      if (stat != 0) return stat;
   }

   /* encode transitSchedule */

   if (pvalue->m.transitSchedulePresent) {
      stat = asn1PE_DeltaTime (pctxt, pvalue->transitSchedule);
      if (stat != 0) return stat;
   }

   /* encode regional */

   if (pvalue->m.regionalPresent) {
      stat = asn1PE_RequestorDescription_regional (pctxt, &pvalue->regional);
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

EXTERN int asn1PE_SignalRequestMessage_regional (OSCTXT* pctxt, SignalRequestMessage_regional* pvalue)
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

EXTERN int asn1PE_SignalRequestMessage (OSCTXT* pctxt, SignalRequestMessage* pvalue)
{
   int stat = 0;
   OSBOOL extbit = FALSE;

   extbit = (OSBOOL)(pvalue->extElem1.count > 0);

   /* extension bit */

   rtxEncBit (pctxt, extbit);

   rtxEncBit (pctxt, (OSBOOL)pvalue->m.timeStampPresent);

   rtxEncBit (pctxt, (OSBOOL)pvalue->m.sequenceNumberPresent);

   rtxEncBit (pctxt, (OSBOOL)pvalue->m.requestsPresent);

   rtxEncBit (pctxt, (OSBOOL)pvalue->m.regionalPresent);

   /* encode timeStamp */

   if (pvalue->m.timeStampPresent) {
      stat = asn1PE_MinuteOfTheYear (pctxt, pvalue->timeStamp);
      if (stat != 0) return stat;
   }

   /* encode second */

   stat = asn1PE_DSecond (pctxt, pvalue->second);
   if (stat != 0) return stat;

   /* encode sequenceNumber */

   if (pvalue->m.sequenceNumberPresent) {
      stat = asn1PE_Common_MsgCount (pctxt, pvalue->sequenceNumber);
      if (stat != 0) return stat;
   }

   /* encode requests */

   if (pvalue->m.requestsPresent) {
      stat = asn1PE_SignalRequestList (pctxt, &pvalue->requests);
      if (stat != 0) return stat;
   }

   /* encode requestor */

   stat = asn1PE_RequestorDescription (pctxt, &pvalue->requestor);
   if (stat != 0) return stat;

   /* encode regional */

   if (pvalue->m.regionalPresent) {
      stat = asn1PE_SignalRequestMessage_regional (pctxt, &pvalue->regional);
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
