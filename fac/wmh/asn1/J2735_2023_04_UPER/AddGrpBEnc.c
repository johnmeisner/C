/**
 * This file was generated by the Objective Systems ASN1C Compiler
 * (http://www.obj-sys.com).  Version: 7.7.0, Date: 25-May-2023.
 *
 * Command:  asn1c CooperativeControlMsg-v1-ph.asn J2735-AddGrpB.asn J2735-AddGrpC.asn J2735-BasicSafetyMessage-v4.asn J2735-CommonSafetyRequest.asn J2735-Common-v7.asn J2735-EmergencyVehicleAlert.asn J2735-IntersectionCollision.asn J2735-ITIS.asn J2735-MapData-v1.asn J2735-MessageFrame-v5.asn J2735-NMEAcorrections.asn J2735-NTCIP.asn J2735-PersonalSafetyMessage.asn J2735-ProbeDataManagement.asn J2735-ProbeVehicleData.asn J2735-REGION.asn J2735-RoadSideAlert.asn J2735-RTCMcorrections.asn J2735-SignalRequestMessage.asn J2735-SignalStatusMessage.asn J2735-SPAT-v2.asn J2735-TestMessage00.asn J2735-TestMessage01.asn J2735-TestMessage02.asn J2735-TestMessage03.asn J2735-TestMessage04.asn J2735-TestMessage05.asn J2735-TestMessage06.asn J2735-TestMessage07.asn J2735-TestMessage08.asn J2735-TestMessage09.asn J2735-TestMessage10.asn J2735-TestMessage11.asn J2735-TestMessage12.asn J2735-TestMessage13.asn J2735-TestMessage14.asn J2735-TestMessage15.asn J2735-TravelerInformation-v2.asn MnvrSharingCoordMsg-v1-ph.asn PersonalSafetyMsg2-v1-ph.asn ProbeDataConfig-v0.30.asn ProbeDataReport-v0.30.asn RoadGeoAttributes-v2-ph.asn RoadSafetyMsg-v1-ph.asn RoadWeatherMessage.asn SensorDataSharingMsg-v1-ph.asn SignalControlAndPrioritizationRequest-v1-ph.asn SignalControlAndPrioritizationStatus-v1-ph.asn TollAdvertisementMsg-v0.10-ph.asn TollUsageAckMsg-v0.10-ph.asn TollUsageMsg-v0.10-ph.asn TrafficSignalPhaseAndTiming-v1-ph.asn -c -per -compact
 */
#include "AddGrpB.h"
#include "SPAT.h"
#include "rtxsrc/rtxCharStr.h"

EXTERN int asn1PE_Angle (OSCTXT* pctxt, Angle value)
{
   int stat = 0;

   stat = pe_ConsUnsigned(pctxt, value,  0,  239);
   if (stat != 0) return stat;

   return (stat);
}

EXTERN int asn1PE_Day (OSCTXT* pctxt, Day value)
{
   int stat = 0;

   stat = pe_ConsUnsigned(pctxt, value,  0,  255);
   if (stat != 0) return stat;

   return (stat);
}

EXTERN int asn1PE_DayOfWeek (OSCTXT* pctxt, DayOfWeek value)
{
   int stat = 0;

   if (value >= 8) {
      return RTERR_INVENUM;
   }
   stat = rtxEncBits(pctxt, value, 3);
   if (stat != 0) return stat;

   return (stat);
}

EXTERN int asn1PE_DegreesLat (OSCTXT* pctxt, DegreesLat value)
{
   int stat = 0;

   stat = pe_ConsInteger(pctxt, value,  -90,  90);
   if (stat != 0) return stat;

   return (stat);
}

EXTERN int asn1PE_DegreesLong (OSCTXT* pctxt, DegreesLong value)
{
   int stat = 0;

   stat = pe_ConsInteger(pctxt, value,  -180,  180);
   if (stat != 0) return stat;

   return (stat);
}

EXTERN int asn1PE_Elevation (OSCTXT* pctxt, Elevation value)
{
   int stat = 0;

   stat = pe_ConsInteger(pctxt, value,  -32768,  32767);
   if (stat != 0) return stat;

   return (stat);
}

EXTERN int asn1PE_Holiday (OSCTXT* pctxt, Holiday value)
{
   int stat = 0;

   if (value >= 2) {
      return RTERR_INVENUM;
   }
   stat = rtxEncBits(pctxt, value, 1);
   if (stat != 0) return stat;

   return (stat);
}

EXTERN int asn1PE_Hour (OSCTXT* pctxt, Hour value)
{
   int stat = 0;

   stat = pe_ConsUnsigned(pctxt, value,  0,  255);
   if (stat != 0) return stat;

   return (stat);
}

EXTERN int asn1PE_LatitudeDMS (OSCTXT* pctxt, LatitudeDMS value)
{
   int stat = 0;

   stat = pe_ConsInteger(pctxt, value,  -32400000,  32400000);
   if (stat != 0) return stat;

   return (stat);
}

EXTERN int asn1PE_LongitudeDMS (OSCTXT* pctxt, LongitudeDMS value)
{
   int stat = 0;

   stat = pe_ConsInteger(pctxt, value,  -64800000,  64800000);
   if (stat != 0) return stat;

   return (stat);
}

EXTERN int asn1PE_MaxTimetoChange (OSCTXT* pctxt, MaxTimetoChange value)
{
   int stat = 0;

   stat = pe_ConsUnsigned(pctxt, value,  0,  2402);
   if (stat != 0) return stat;

   return (stat);
}

EXTERN int asn1PE_MinTimetoChange (OSCTXT* pctxt, MinTimetoChange value)
{
   int stat = 0;

   stat = pe_ConsUnsigned(pctxt, value,  0,  2402);
   if (stat != 0) return stat;

   return (stat);
}

EXTERN int asn1PE_Minute (OSCTXT* pctxt, Minute value)
{
   int stat = 0;

   stat = pe_ConsUnsigned(pctxt, value,  0,  255);
   if (stat != 0) return stat;

   return (stat);
}

EXTERN int asn1PE_MinutesAngle (OSCTXT* pctxt, MinutesAngle value)
{
   int stat = 0;

   stat = pe_ConsUnsigned(pctxt, value,  0,  59);
   if (stat != 0) return stat;

   return (stat);
}

EXTERN int asn1PE_Month (OSCTXT* pctxt, Month value)
{
   int stat = 0;

   stat = pe_ConsUnsigned(pctxt, value,  1,  255);
   if (stat != 0) return stat;

   return (stat);
}

EXTERN int asn1PE_MsgCount (OSCTXT* pctxt, MsgCount value)
{
   int stat = 0;

   stat = pe_ConsUnsigned(pctxt, value,  0,  255);
   if (stat != 0) return stat;

   return (stat);
}

EXTERN int asn1PE_Second (OSCTXT* pctxt, Second value)
{
   int stat = 0;

   stat = pe_ConsUnsigned(pctxt, value,  0,  60);
   if (stat != 0) return stat;

   return (stat);
}

EXTERN int asn1PE_SecondsAngle (OSCTXT* pctxt, SecondsAngle value)
{
   int stat = 0;

   stat = pe_ConsUnsigned(pctxt, value,  0,  5999);
   if (stat != 0) return stat;

   return (stat);
}

EXTERN int asn1PE_SummerTime (OSCTXT* pctxt, SummerTime value)
{
   int stat = 0;

   if (value >= 2) {
      return RTERR_INVENUM;
   }
   stat = rtxEncBits(pctxt, value, 1);
   if (stat != 0) return stat;

   return (stat);
}

EXTERN int asn1PE_TenthSecond (OSCTXT* pctxt, TenthSecond value)
{
   int stat = 0;

   stat = pe_ConsUnsigned(pctxt, value,  0,  9);
   if (stat != 0) return stat;

   return (stat);
}

EXTERN int asn1PE_TimeRemaining (OSCTXT* pctxt, TimeRemaining value)
{
   int stat = 0;

   stat = pe_ConsUnsigned(pctxt, value,  0,  9001);
   if (stat != 0) return stat;

   return (stat);
}

EXTERN int asn1PE_Year (OSCTXT* pctxt, Year value)
{
   int stat = 0;

   stat = pe_ConsUnsigned(pctxt, value,  1,  65535);
   if (stat != 0) return stat;

   return (stat);
}

EXTERN int asn1PE_LatitudeDMS2 (OSCTXT* pctxt, LatitudeDMS2* pvalue)
{
   int stat = 0;

   /* encode d */

   stat = asn1PE_DegreesLat (pctxt, pvalue->d);
   if (stat != 0) return stat;

   /* encode m_ */

   stat = asn1PE_MinutesAngle (pctxt, pvalue->m_);
   if (stat != 0) return stat;

   /* encode s */

   stat = asn1PE_SecondsAngle (pctxt, pvalue->s);
   if (stat != 0) return stat;

   return (stat);
}

EXTERN int asn1PE_LongitudeDMS2 (OSCTXT* pctxt, LongitudeDMS2* pvalue)
{
   int stat = 0;

   /* encode d */

   stat = asn1PE_DegreesLong (pctxt, pvalue->d);
   if (stat != 0) return stat;

   /* encode m_ */

   stat = asn1PE_MinutesAngle (pctxt, pvalue->m_);
   if (stat != 0) return stat;

   /* encode s */

   stat = asn1PE_SecondsAngle (pctxt, pvalue->s);
   if (stat != 0) return stat;

   return (stat);
}

EXTERN int asn1PE_Node_LLdms_48b (OSCTXT* pctxt, Node_LLdms_48b* pvalue)
{
   int stat = 0;

   /* encode lon */

   stat = asn1PE_LongitudeDMS (pctxt, pvalue->lon);
   if (stat != 0) return stat;

   /* encode lat */

   stat = asn1PE_LatitudeDMS (pctxt, pvalue->lat);
   if (stat != 0) return stat;

   return (stat);
}

EXTERN int asn1PE_Node_LLdms_80b (OSCTXT* pctxt, Node_LLdms_80b* pvalue)
{
   int stat = 0;

   /* encode lon */

   stat = asn1PE_LongitudeDMS2 (pctxt, &pvalue->lon);
   if (stat != 0) return stat;

   /* encode lat */

   stat = asn1PE_LatitudeDMS2 (pctxt, &pvalue->lat);
   if (stat != 0) return stat;

   return (stat);
}

EXTERN int asn1PE_LaneDataAttribute_addGrpB (OSCTXT* pctxt, LaneDataAttribute_addGrpB* pvalue)
{
   int stat = 0;
   OSBOOL extbit = FALSE;
   OS_UNUSED_ARG (pvalue) ;

   extbit = (OSBOOL)(pvalue->extElem1.count > 0);

   /* extension bit */

   rtxEncBit (pctxt, extbit);

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

EXTERN int asn1PE_MovementEvent_addGrpB (OSCTXT* pctxt, MovementEvent_addGrpB* pvalue)
{
   int stat = 0;
   OSBOOL extbit = FALSE;

   extbit = (OSBOOL)(pvalue->extElem1.count > 0);

   /* extension bit */

   rtxEncBit (pctxt, extbit);

   rtxEncBit (pctxt, (OSBOOL)pvalue->m.startTimePresent);

   rtxEncBit (pctxt, (OSBOOL)pvalue->m.maxEndTimePresent);

   rtxEncBit (pctxt, (OSBOOL)pvalue->m.likelyTimePresent);

   rtxEncBit (pctxt, (OSBOOL)pvalue->m.confidencePresent);

   rtxEncBit (pctxt, (OSBOOL)pvalue->m.nextTimePresent);

   /* encode startTime */

   if (pvalue->m.startTimePresent) {
      stat = asn1PE_TimeRemaining (pctxt, pvalue->startTime);
      if (stat != 0) return stat;
   }

   /* encode minEndTime */

   stat = asn1PE_MinTimetoChange (pctxt, pvalue->minEndTime);
   if (stat != 0) return stat;

   /* encode maxEndTime */

   if (pvalue->m.maxEndTimePresent) {
      stat = asn1PE_MaxTimetoChange (pctxt, pvalue->maxEndTime);
      if (stat != 0) return stat;
   }

   /* encode likelyTime */

   if (pvalue->m.likelyTimePresent) {
      stat = asn1PE_TimeRemaining (pctxt, pvalue->likelyTime);
      if (stat != 0) return stat;
   }

   /* encode confidence */

   if (pvalue->m.confidencePresent) {
      stat = asn1PE_TimeIntervalConfidence (pctxt, pvalue->confidence);
      if (stat != 0) return stat;
   }

   /* encode nextTime */

   if (pvalue->m.nextTimePresent) {
      stat = asn1PE_TimeRemaining (pctxt, pvalue->nextTime);
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

EXTERN int asn1PE_NodeOffsetPointXY_addGrpB (OSCTXT* pctxt, NodeOffsetPointXY_addGrpB* pvalue)
{
   int stat = 0;
   OSBOOL extbit = FALSE;

   extbit = (OSBOOL)(pvalue->t > 2);

   /* extension bit */

   rtxEncBit (pctxt, extbit);

   if (!extbit) {

      /* Encode choice index value */

      stat = rtxEncBits (pctxt, pvalue->t - 1, 1);
      if (stat != 0) return stat;

      /* Encode root element data value */

      switch (pvalue->t)
      {
         /* posA */
         case 1:
            stat = asn1PE_Node_LLdms_48b (pctxt, pvalue->u.posA);
            if (stat != 0) return stat;
            break;

         /* posB */
         case 2:
            stat = asn1PE_Node_LLdms_80b (pctxt, pvalue->u.posB);
            if (stat != 0) return stat;
            break;

         default:
            return RTERR_INVOPT;
      }
   }
   else {
      /* Encode extension choice index value */

      stat = pe_SmallNonNegWholeNumber (pctxt, pvalue->t - 3);
      if (stat != 0) return stat;

      /* Encode extension element data value */

      stat = pe_OpenType (pctxt, pvalue->u.extElem1->numocts, pvalue->u.extElem1->data);
      if (stat != 0) return stat;
   }

   return (stat);
}

EXTERN int asn1PE_Position3D_addGrpB (OSCTXT* pctxt, Position3D_addGrpB* pvalue)
{
   int stat = 0;
   OSBOOL extbit = FALSE;

   extbit = (OSBOOL)(pvalue->extElem1.count > 0);

   /* extension bit */

   rtxEncBit (pctxt, extbit);

   /* encode latitude */

   stat = asn1PE_LatitudeDMS2 (pctxt, &pvalue->latitude);
   if (stat != 0) return stat;

   /* encode longitude */

   stat = asn1PE_LongitudeDMS2 (pctxt, &pvalue->longitude);
   if (stat != 0) return stat;

   /* encode elevation */

   stat = asn1PE_Elevation (pctxt, pvalue->elevation);
   if (stat != 0) return stat;

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

EXTERN int asn1PE_TimeMark (OSCTXT* pctxt, TimeMark* pvalue)
{
   int stat = 0;

   /* encode year */

   stat = asn1PE_Year (pctxt, pvalue->year);
   if (stat != 0) return stat;

   /* encode month */

   stat = asn1PE_Month (pctxt, pvalue->month);
   if (stat != 0) return stat;

   /* encode day */

   stat = asn1PE_Day (pctxt, pvalue->day);
   if (stat != 0) return stat;

   /* encode summerTime */

   stat = asn1PE_SummerTime (pctxt, pvalue->summerTime);
   if (stat != 0) return stat;

   /* encode holiday */

   stat = asn1PE_Holiday (pctxt, pvalue->holiday);
   if (stat != 0) return stat;

   /* encode dayofWeek */

   stat = asn1PE_DayOfWeek (pctxt, pvalue->dayofWeek);
   if (stat != 0) return stat;

   /* encode hour */

   stat = asn1PE_Hour (pctxt, pvalue->hour);
   if (stat != 0) return stat;

   /* encode minute */

   stat = asn1PE_Minute (pctxt, pvalue->minute);
   if (stat != 0) return stat;

   /* encode second */

   stat = asn1PE_Second (pctxt, pvalue->second);
   if (stat != 0) return stat;

   /* encode tenthSecond */

   stat = asn1PE_TenthSecond (pctxt, pvalue->tenthSecond);
   if (stat != 0) return stat;

   return (stat);
}

