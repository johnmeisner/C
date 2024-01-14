/**
 * This file was generated by the Objective Systems ASN1C Compiler
 * (http://www.obj-sys.com).  Version: 7.7.0, Date: 25-May-2023.
 *
 * Command:  asn1c CooperativeControlMsg-v1-ph.asn J2735-AddGrpB.asn J2735-AddGrpC.asn J2735-BasicSafetyMessage-v4.asn J2735-CommonSafetyRequest.asn J2735-Common-v7.asn J2735-EmergencyVehicleAlert.asn J2735-IntersectionCollision.asn J2735-ITIS.asn J2735-MapData-v1.asn J2735-MessageFrame-v5.asn J2735-NMEAcorrections.asn J2735-NTCIP.asn J2735-PersonalSafetyMessage.asn J2735-ProbeDataManagement.asn J2735-ProbeVehicleData.asn J2735-REGION.asn J2735-RoadSideAlert.asn J2735-RTCMcorrections.asn J2735-SignalRequestMessage.asn J2735-SignalStatusMessage.asn J2735-SPAT-v2.asn J2735-TestMessage00.asn J2735-TestMessage01.asn J2735-TestMessage02.asn J2735-TestMessage03.asn J2735-TestMessage04.asn J2735-TestMessage05.asn J2735-TestMessage06.asn J2735-TestMessage07.asn J2735-TestMessage08.asn J2735-TestMessage09.asn J2735-TestMessage10.asn J2735-TestMessage11.asn J2735-TestMessage12.asn J2735-TestMessage13.asn J2735-TestMessage14.asn J2735-TestMessage15.asn J2735-TravelerInformation-v2.asn MnvrSharingCoordMsg-v1-ph.asn PersonalSafetyMsg2-v1-ph.asn ProbeDataConfig-v0.30.asn ProbeDataReport-v0.30.asn RoadGeoAttributes-v2-ph.asn RoadSafetyMsg-v1-ph.asn RoadWeatherMessage.asn SensorDataSharingMsg-v1-ph.asn SignalControlAndPrioritizationRequest-v1-ph.asn SignalControlAndPrioritizationStatus-v1-ph.asn TollAdvertisementMsg-v0.10-ph.asn TollUsageAckMsg-v0.10-ph.asn TollUsageMsg-v0.10-ph.asn TrafficSignalPhaseAndTiming-v1-ph.asn -c -per -compact
 */
#include "EmergencyVehicleAlert.h"
#include "REGION.h"
#include "RoadSideAlert.h"
#include "Common.h"
#include "rtxsrc/rtxCharStr.h"

EXTERN int asn1PE_EmergencyVehicleAlert_regional (OSCTXT* pctxt, EmergencyVehicleAlert_regional* pvalue)
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

EXTERN int asn1PE_EmergencyVehicleAlert (OSCTXT* pctxt, EmergencyVehicleAlert* pvalue)
{
   int stat = 0;
   OSBOOL extbit = FALSE;

   extbit = (OSBOOL)(pvalue->extElem1.count > 0);

   /* extension bit */

   rtxEncBit (pctxt, extbit);

   rtxEncBit (pctxt, (OSBOOL)pvalue->m.timeStampPresent);

   rtxEncBit (pctxt, (OSBOOL)pvalue->m.idPresent);

   rtxEncBit (pctxt, (OSBOOL)pvalue->m.responseTypePresent);

   rtxEncBit (pctxt, (OSBOOL)pvalue->m.detailsPresent);

   rtxEncBit (pctxt, (OSBOOL)pvalue->m.massPresent);

   rtxEncBit (pctxt, (OSBOOL)pvalue->m.basicTypePresent);

   rtxEncBit (pctxt, (OSBOOL)pvalue->m.vehicleTypePresent);

   rtxEncBit (pctxt, (OSBOOL)pvalue->m.responseEquipPresent);

   rtxEncBit (pctxt, (OSBOOL)pvalue->m.responderTypePresent);

   rtxEncBit (pctxt, (OSBOOL)pvalue->m.regionalPresent);

   /* encode timeStamp */

   if (pvalue->m.timeStampPresent) {
      stat = asn1PE_MinuteOfTheYear (pctxt, pvalue->timeStamp);
      if (stat != 0) return stat;
   }

   /* encode id */

   if (pvalue->m.idPresent) {
      stat = asn1PE_TemporaryID (pctxt, (TemporaryID*)pvalue->id);
      if (stat != 0) return stat;
   }

   /* encode rsaMsg */

   stat = asn1PE_RoadSideAlert (pctxt, (RoadSideAlert*)pvalue->rsaMsg);
   if (stat != 0) return stat;

   /* encode responseType */

   if (pvalue->m.responseTypePresent) {
      stat = asn1PE_ResponseType (pctxt, pvalue->responseType);
      if (stat != 0) return stat;
   }

   /* encode details */

   if (pvalue->m.detailsPresent) {
      stat = asn1PE_EmergencyDetails (pctxt, (EmergencyDetails*)pvalue->details);
      if (stat != 0) return stat;
   }

   /* encode mass */

   if (pvalue->m.massPresent) {
      stat = asn1PE_VehicleMass (pctxt, pvalue->mass);
      if (stat != 0) return stat;
   }

   /* encode basicType */

   if (pvalue->m.basicTypePresent) {
      stat = asn1PE_VehicleType (pctxt, pvalue->basicType);
      if (stat != 0) return stat;
   }

   /* encode vehicleType */

   if (pvalue->m.vehicleTypePresent) {
      stat = asn1PE_VehicleGroupAffected (pctxt, pvalue->vehicleType);
      if (stat != 0) return stat;
   }

   /* encode responseEquip */

   if (pvalue->m.responseEquipPresent) {
      stat = asn1PE_IncidentResponseEquipment (pctxt, pvalue->responseEquip);
      if (stat != 0) return stat;
   }

   /* encode responderType */

   if (pvalue->m.responderTypePresent) {
      stat = asn1PE_ResponderGroupAffected (pctxt, pvalue->responderType);
      if (stat != 0) return stat;
   }

   /* encode regional */

   if (pvalue->m.regionalPresent) {
      stat = asn1PE_EmergencyVehicleAlert_regional (pctxt, &pvalue->regional);
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

