/**
 * This file was generated by the Objective Systems ASN1C Compiler
 * (http://www.obj-sys.com).  Version: 7.7.0, Date: 25-May-2023.
 *
 * Command:  asn1c CooperativeControlMsg-v1-ph.asn J2735-AddGrpB.asn J2735-AddGrpC.asn J2735-BasicSafetyMessage-v4.asn J2735-CommonSafetyRequest.asn J2735-Common-v7.asn J2735-EmergencyVehicleAlert.asn J2735-IntersectionCollision.asn J2735-ITIS.asn J2735-MapData-v1.asn J2735-MessageFrame-v5.asn J2735-NMEAcorrections.asn J2735-NTCIP.asn J2735-PersonalSafetyMessage.asn J2735-ProbeDataManagement.asn J2735-ProbeVehicleData.asn J2735-REGION.asn J2735-RoadSideAlert.asn J2735-RTCMcorrections.asn J2735-SignalRequestMessage.asn J2735-SignalStatusMessage.asn J2735-SPAT-v2.asn J2735-TestMessage00.asn J2735-TestMessage01.asn J2735-TestMessage02.asn J2735-TestMessage03.asn J2735-TestMessage04.asn J2735-TestMessage05.asn J2735-TestMessage06.asn J2735-TestMessage07.asn J2735-TestMessage08.asn J2735-TestMessage09.asn J2735-TestMessage10.asn J2735-TestMessage11.asn J2735-TestMessage12.asn J2735-TestMessage13.asn J2735-TestMessage14.asn J2735-TestMessage15.asn J2735-TravelerInformation-v2.asn MnvrSharingCoordMsg-v1-ph.asn PersonalSafetyMsg2-v1-ph.asn ProbeDataConfig-v0.30.asn ProbeDataReport-v0.30.asn RoadGeoAttributes-v2-ph.asn RoadSafetyMsg-v1-ph.asn RoadWeatherMessage.asn SensorDataSharingMsg-v1-ph.asn SignalControlAndPrioritizationRequest-v1-ph.asn SignalControlAndPrioritizationStatus-v1-ph.asn TollAdvertisementMsg-v0.10-ph.asn TollUsageAckMsg-v0.10-ph.asn TollUsageMsg-v0.10-ph.asn TrafficSignalPhaseAndTiming-v1-ph.asn -c -per -compact
 */
#include "TravelerInformation.h"
#include "REGION.h"
#include "Common.h"

/* DirectionOfUse */

const OSEnumItem DirectionOfUse_ENUMTAB[] = {
   { OSUTF8("both"), 3, 4, 3 },
   { OSUTF8("forward"), 1, 7, 1 },
   { OSUTF8("reverse"), 2, 7, 2 },
   { OSUTF8("unavailable"), 0, 11, 0 }
} ;

const OSUTF8CHAR* DirectionOfUse_ToString (OSINT32 value)
{
   OSINT32 idx = value;

   if (idx >= 0 && idx < DirectionOfUse_ENUMTABSIZE) {
      return DirectionOfUse_ENUMTAB
         [DirectionOfUse_ENUMTAB[idx].transidx].name;
   }
   else
      return OSUTF8("_UNKNOWN_");
}

int DirectionOfUse_ToEnum (OSCTXT* pctxt,
   const OSUTF8CHAR* value, DirectionOfUse* pvalue)
{
   OSSIZE valueLen = rtxUTF8LenBytes (value);
   return DirectionOfUse_ToEnum2 (pctxt, value, valueLen, pvalue);
}

int DirectionOfUse_ToEnum2 (OSCTXT* pctxt,
   const OSUTF8CHAR* value, OSSIZE valueLen, DirectionOfUse* pvalue)
{
   OSINT32 idx = rtxLookupEnum (value, valueLen, DirectionOfUse_ENUMTAB, 
      DirectionOfUse_ENUMTABSIZE);

   if (idx >= 0) {
      *pvalue = (DirectionOfUse)DirectionOfUse_ENUMTAB[idx].value;
      return 0;
   }
   else {
      return RTERR_INVENUM;
   }
}

/* DistanceUnits */

const OSEnumItem DistanceUnits_ENUMTAB[] = {
   { OSUTF8("centimeter"), 0, 10, 0 },
   { OSUTF8("cm2-5"), 1, 5, 1 },
   { OSUTF8("decimeter"), 2, 9, 2 },
   { OSUTF8("foot"), 5, 4, 5 },
   { OSUTF8("kilometer"), 4, 9, 4 },
   { OSUTF8("meter"), 3, 5, 3 },
   { OSUTF8("mile"), 7, 4, 7 },
   { OSUTF8("yard"), 6, 4, 6 }
} ;

const OSUTF8CHAR* DistanceUnits_ToString (OSINT32 value)
{
   OSINT32 idx = value;

   if (idx >= 0 && idx < DistanceUnits_ENUMTABSIZE) {
      return DistanceUnits_ENUMTAB[DistanceUnits_ENUMTAB[idx].transidx].name;
   }
   else
      return OSUTF8("_UNKNOWN_");
}

int DistanceUnits_ToEnum (OSCTXT* pctxt,
   const OSUTF8CHAR* value, DistanceUnits* pvalue)
{
   OSSIZE valueLen = rtxUTF8LenBytes (value);
   return DistanceUnits_ToEnum2 (pctxt, value, valueLen, pvalue);
}

int DistanceUnits_ToEnum2 (OSCTXT* pctxt,
   const OSUTF8CHAR* value, OSSIZE valueLen, DistanceUnits* pvalue)
{
   OSINT32 idx = rtxLookupEnum (value, valueLen, DistanceUnits_ENUMTAB, 
      DistanceUnits_ENUMTABSIZE);

   if (idx >= 0) {
      *pvalue = (DistanceUnits)DistanceUnits_ENUMTAB[idx].value;
      return 0;
   }
   else {
      return RTERR_INVENUM;
   }
}

/* ITIStextPhrase */

/* MinutesDuration */

/* MsgCRC */

int asn1Init_MsgCRC (MsgCRC* pvalue)
{
   pvalue->numocts = 0;
   return 0;
}

/* MUTCDCode */

const OSEnumItem MUTCDCode_ENUMTAB[] = {
   { OSUTF8("guide"), 5, 5, 3 },
   { OSUTF8("maintenance"), 3, 11, 5 },
   { OSUTF8("motoristService"), 4, 15, 6 },
   { OSUTF8("none"), 0, 4, 1 },
   { OSUTF8("rec"), 6, 3, 2 },
   { OSUTF8("regulatory"), 1, 10, 0 },
   { OSUTF8("warning"), 2, 7, 4 }
} ;

const OSUTF8CHAR* MUTCDCode_ToString (OSINT32 value)
{
   OSINT32 idx = value;

   if (idx >= 0 && idx < MUTCDCode_ENUMTABSIZE) {
      return MUTCDCode_ENUMTAB[MUTCDCode_ENUMTAB[idx].transidx].name;
   }
   else
      return OSUTF8("_UNKNOWN_");
}

int MUTCDCode_ToEnum (OSCTXT* pctxt,
   const OSUTF8CHAR* value, MUTCDCode* pvalue)
{
   OSSIZE valueLen = rtxUTF8LenBytes (value);
   return MUTCDCode_ToEnum2 (pctxt, value, valueLen, pvalue);
}

int MUTCDCode_ToEnum2 (OSCTXT* pctxt,
   const OSUTF8CHAR* value, OSSIZE valueLen, MUTCDCode* pvalue)
{
   OSINT32 idx = rtxLookupEnum (value, valueLen, MUTCDCode_ENUMTAB, 
      MUTCDCode_ENUMTABSIZE);

   if (idx >= 0) {
      *pvalue = (MUTCDCode)MUTCDCode_ENUMTAB[idx].value;
      return 0;
   }
   else {
      return RTERR_INVENUM;
   }
}

/* NodeAttributeLL */

const OSEnumItem NodeAttributeLL_ENUMTAB[] = {
   { OSUTF8("closedToTraffic"), 8, 15, 7 },
   { OSUTF8("curbPresentAtStepOff"), 10, 20, 11 },
   { OSUTF8("divergePoint"), 5, 12, 8 },
   { OSUTF8("downstreamStartNode"), 7, 19, 9 },
   { OSUTF8("downstreamStopLine"), 6, 18, 6 },
   { OSUTF8("hydrantPresent"), 11, 14, 2 },
   { OSUTF8("mergePoint"), 4, 10, 4 },
   { OSUTF8("reserved"), 0, 8, 3 },
   { OSUTF8("roundedCapStyleA"), 2, 16, 0 },
   { OSUTF8("roundedCapStyleB"), 3, 16, 10 },
   { OSUTF8("safeIsland"), 9, 10, 1 },
   { OSUTF8("stopLine"), 1, 8, 5 }
} ;

const OSUTF8CHAR* NodeAttributeLL_ToString (OSINT32 value)
{
   OSINT32 idx = value;

   if (idx >= 0 && idx < NodeAttributeLL_ENUMTABSIZE) {
      return NodeAttributeLL_ENUMTAB
         [NodeAttributeLL_ENUMTAB[idx].transidx].name;
   }
   else
      return OSUTF8("_UNKNOWN_");
}

int NodeAttributeLL_ToEnum (OSCTXT* pctxt,
   const OSUTF8CHAR* value, NodeAttributeLL* pvalue)
{
   OSSIZE valueLen = rtxUTF8LenBytes (value);
   return NodeAttributeLL_ToEnum2 (pctxt, value, valueLen, pvalue);
}

int NodeAttributeLL_ToEnum2 (OSCTXT* pctxt,
   const OSUTF8CHAR* value, OSSIZE valueLen, NodeAttributeLL* pvalue)
{
   OSINT32 idx = rtxLookupEnum (value, valueLen, NodeAttributeLL_ENUMTAB, 
      NodeAttributeLL_ENUMTABSIZE);

   if (idx >= 0) {
      *pvalue = (NodeAttributeLL)NodeAttributeLL_ENUMTAB[idx].value;
      return 0;
   }
   else {
      return RTERR_INVENUM;
   }
}

/* OffsetLL_B12 */

/* OffsetLL_B14 */

/* OffsetLL_B16 */

/* OffsetLL_B22 */

/* OffsetLL_B24 */

/* Radius_B12 */

/* SegmentAttributeLL */

const OSEnumItem SegmentAttributeLL_ENUMTAB[] = {
   { OSUTF8("adaptiveTimingPresent"), 25, 21, 21 },
   { OSUTF8("adjacentBikeLaneOnLeft"), 13, 22, 10 },
   { OSUTF8("adjacentBikeLaneOnRight"), 14, 23, 37 },
   { OSUTF8("adjacentParkingOnLeft"), 11, 21, 16 },
   { OSUTF8("adjacentParkingOnRight"), 12, 22, 17 },
   { OSUTF8("audibleSignalingPresent"), 24, 23, 8 },
   { OSUTF8("bikeBoxInFront"), 16, 14, 9 },
   { OSUTF8("costToPark"), 35, 10, 13 },
   { OSUTF8("curbOnLeft"), 5, 10, 14 },
   { OSUTF8("curbOnRight"), 6, 11, 34 },
   { OSUTF8("doNotBlock"), 1, 10, 35 },
   { OSUTF8("freeParking"), 33, 11, 3 },
   { OSUTF8("headInParking"), 32, 13, 4 },
   { OSUTF8("loadingzoneOnLeft"), 7, 17, 1 },
   { OSUTF8("loadingzoneOnRight"), 8, 18, 2 },
   { OSUTF8("lowCurbsPresent"), 22, 15, 25 },
   { OSUTF8("mergingLaneLeft"), 3, 15, 6 },
   { OSUTF8("mergingLaneRight"), 4, 16, 32 },
   { OSUTF8("midBlockCurbPresent"), 36, 19, 33 },
   { OSUTF8("parallelParking"), 31, 15, 31 },
   { OSUTF8("partialCurbIntrusion"), 27, 20, 26 },
   { OSUTF8("reserved"), 0, 8, 24 },
   { OSUTF8("rfSignalRequestPresent"), 26, 22, 15 },
   { OSUTF8("rumbleStripPresent"), 23, 18, 23 },
   { OSUTF8("safeIsland"), 21, 10, 5 },
   { OSUTF8("sharedBikeLane"), 15, 14, 0 },
   { OSUTF8("sharedWithTrackedVehicle"), 20, 24, 22 },
   { OSUTF8("taperToCenterLine"), 30, 17, 20 },
   { OSUTF8("taperToLeft"), 28, 11, 28 },
   { OSUTF8("taperToRight"), 29, 12, 29 },
   { OSUTF8("timeRestrictionsOnParking"), 34, 25, 27 },
   { OSUTF8("transitStopInLane"), 19, 17, 19 },
   { OSUTF8("transitStopOnLeft"), 17, 17, 12 },
   { OSUTF8("transitStopOnRight"), 18, 18, 11 },
   { OSUTF8("turnOutPointOnLeft"), 9, 18, 30 },
   { OSUTF8("turnOutPointOnRight"), 10, 19, 7 },
   { OSUTF8("unEvenPavementPresent"), 37, 21, 18 },
   { OSUTF8("whiteLine"), 2, 9, 36 }
} ;

const OSUTF8CHAR* SegmentAttributeLL_ToString (OSINT32 value)
{
   OSINT32 idx = value;

   if (idx >= 0 && idx < SegmentAttributeLL_ENUMTABSIZE) {
      return SegmentAttributeLL_ENUMTAB
         [SegmentAttributeLL_ENUMTAB[idx].transidx].name;
   }
   else
      return OSUTF8("_UNKNOWN_");
}

int SegmentAttributeLL_ToEnum (OSCTXT* pctxt,
   const OSUTF8CHAR* value, SegmentAttributeLL* pvalue)
{
   OSSIZE valueLen = rtxUTF8LenBytes (value);
   return SegmentAttributeLL_ToEnum2 (pctxt, value, valueLen, pvalue);
}

int SegmentAttributeLL_ToEnum2 (OSCTXT* pctxt,
   const OSUTF8CHAR* value, OSSIZE valueLen, SegmentAttributeLL* pvalue)
{
   OSINT32 idx = rtxLookupEnum (value, valueLen, SegmentAttributeLL_ENUMTAB, 
      SegmentAttributeLL_ENUMTABSIZE);

   if (idx >= 0) {
      *pvalue = (SegmentAttributeLL)SegmentAttributeLL_ENUMTAB[idx].value;
      return 0;
   }
   else {
      return RTERR_INVENUM;
   }
}

/* SignPrority */

/* TravelerInfoType */

const OSEnumItem TravelerInfoType_ENUMTAB[] = {
   { OSUTF8("advisory"), 1, 8, 3 },
   { OSUTF8("commercialSignage"), 3, 17, 0 },
   { OSUTF8("roadSignage"), 2, 11, 2 },
   { OSUTF8("unknown"), 0, 7, 1 }
} ;

const OSUTF8CHAR* TravelerInfoType_ToString (OSINT32 value)
{
   OSINT32 idx = value;

   if (idx >= 0 && idx < TravelerInfoType_ENUMTABSIZE) {
      return TravelerInfoType_ENUMTAB
         [TravelerInfoType_ENUMTAB[idx].transidx].name;
   }
   else
      return OSUTF8("_UNKNOWN_");
}

int TravelerInfoType_ToEnum (OSCTXT* pctxt,
   const OSUTF8CHAR* value, TravelerInfoType* pvalue)
{
   OSSIZE valueLen = rtxUTF8LenBytes (value);
   return TravelerInfoType_ToEnum2 (pctxt, value, valueLen, pvalue);
}

int TravelerInfoType_ToEnum2 (OSCTXT* pctxt,
   const OSUTF8CHAR* value, OSSIZE valueLen, TravelerInfoType* pvalue)
{
   OSINT32 idx = rtxLookupEnum (value, valueLen, TravelerInfoType_ENUMTAB, 
      TravelerInfoType_ENUMTABSIZE);

   if (idx >= 0) {
      *pvalue = (TravelerInfoType)TravelerInfoType_ENUMTAB[idx].value;
      return 0;
   }
   else {
      return RTERR_INVENUM;
   }
}

/* UniqueMSGID */

int asn1Init_UniqueMSGID (UniqueMSGID* pvalue)
{
   pvalue->numocts = 0;
   return 0;
}

/* URL_Base */

/* URL_Short */

/* Zoom */

/* RoadSignID */

int asn1Init_RoadSignID (RoadSignID* pvalue)
{
   OSCRTLMEMSET (&pvalue->m, 0, sizeof (pvalue->m));
   pvalue->position = 0;
   pvalue->viewAngle = 0;
   asn1Init_MsgCRC (&pvalue->crc);
   return 0;
}

/* TravelerDataFrame_msgId */

int asn1Init_TravelerDataFrame_msgId (TravelerDataFrame_msgId* pvalue)
{
   pvalue->t = 0;
   OSCRTLMEMSET (&pvalue->u, 0, sizeof(pvalue->u));
   return 0;
}

/* Node_LL_24B */

/* Node_LL_28B */

/* Node_LL_32B */

/* Node_LL_36B */

/* Node_LL_44B */

/* Node_LL_48B */

/* NodeOffsetPointLL */

int asn1Init_NodeOffsetPointLL (NodeOffsetPointLL* pvalue)
{
   pvalue->t = 0;
   OSCRTLMEMSET (&pvalue->u, 0, sizeof(pvalue->u));
   return 0;
}

/* NodeAttributeLLList */

int asn1Init_NodeAttributeLLList (NodeAttributeLLList* pvalue)
{
   pvalue->n = 0;
   return 0;
}

/* SegmentAttributeLLList */

int asn1Init_SegmentAttributeLLList (SegmentAttributeLLList* pvalue)
{
   pvalue->n = 0;
   return 0;
}

/* NodeAttributeSetLL_regional */

int asn1Init_NodeAttributeSetLL_regional (NodeAttributeSetLL_regional* pvalue)
{
   rtxDListFastInit (pvalue);
   return 0;
}

/* NodeAttributeSetLL */

int asn1Init_NodeAttributeSetLL (NodeAttributeSetLL* pvalue)
{
   OSCRTLMEMSET (&pvalue->m, 0, sizeof (pvalue->m));
   asn1Init_NodeAttributeLLList (&pvalue->localNode);
   asn1Init_SegmentAttributeLLList (&pvalue->disabled);
   asn1Init_SegmentAttributeLLList (&pvalue->enabled);
   asn1Init_LaneDataAttributeList ((LaneDataAttributeList*)&pvalue->data);
   asn1Init_NodeAttributeSetLL_regional (&pvalue->regional);
   rtxDListFastInit (&pvalue->extElem1);
   return 0;
}

/* NodeLL */

int asn1Init_NodeLL (NodeLL* pvalue)
{
   OSCRTLMEMSET (&pvalue->m, 0, sizeof (pvalue->m));
   asn1Init_NodeOffsetPointLL (&pvalue->delta);
   asn1Init_NodeAttributeSetLL (&pvalue->attributes);
   rtxDListFastInit (&pvalue->extElem1);
   return 0;
}

/* NodeSetLL */

int asn1Init_NodeSetLL (NodeSetLL* pvalue)
{
   rtxDListFastInit (pvalue);
   return 0;
}

/* NodeListLL */

int asn1Init_NodeListLL (NodeListLL* pvalue)
{
   pvalue->t = 0;
   OSCRTLMEMSET (&pvalue->u, 0, sizeof(pvalue->u));
   return 0;
}

/* OffsetSystem_offset */

int asn1Init_OffsetSystem_offset (OffsetSystem_offset* pvalue)
{
   pvalue->t = 0;
   OSCRTLMEMSET (&pvalue->u, 0, sizeof(pvalue->u));
   return 0;
}

/* OffsetSystem */

int asn1Init_OffsetSystem (OffsetSystem* pvalue)
{
   OSCRTLMEMSET (&pvalue->m, 0, sizeof (pvalue->m));
   asn1Init_OffsetSystem_offset (&pvalue->offset);
   return 0;
}

/* Circle */

int asn1Init_Circle (Circle* pvalue)
{
   pvalue->center = 0;
   return 0;
}

/* GeometricProjection_regional */

int asn1Init_GeometricProjection_regional (
   GeometricProjection_regional* pvalue)
{
   rtxDListFastInit (pvalue);
   return 0;
}

/* GeometricProjection */

int asn1Init_GeometricProjection (GeometricProjection* pvalue)
{
   OSCRTLMEMSET (&pvalue->m, 0, sizeof (pvalue->m));
   pvalue->direction = 0;
   asn1Init_Circle (&pvalue->circle);
   asn1Init_GeometricProjection_regional (&pvalue->regional);
   rtxDListFastInit (&pvalue->extElem1);
   return 0;
}

/* ShapePointSet */

int asn1Init_ShapePointSet (ShapePointSet* pvalue)
{
   OSCRTLMEMSET (&pvalue->m, 0, sizeof (pvalue->m));
   pvalue->anchor = 0;
   pvalue->nodeList = 0;
   rtxDListFastInit (&pvalue->extElem1);
   return 0;
}

/* RegionOffsets */

int asn1Init_RegionOffsets (RegionOffsets* pvalue)
{
   OSCRTLMEMSET (&pvalue->m, 0, sizeof (pvalue->m));
   return 0;
}

/* RegionList */

int asn1Init_RegionList (RegionList* pvalue)
{
   rtxDListFastInit (pvalue);
   return 0;
}

/* RegionPointSet */

int asn1Init_RegionPointSet (RegionPointSet* pvalue)
{
   OSCRTLMEMSET (&pvalue->m, 0, sizeof (pvalue->m));
   pvalue->anchor = 0;
   asn1Init_RegionList (&pvalue->nodeList);
   rtxDListFastInit (&pvalue->extElem1);
   return 0;
}

/* ValidRegion_area */

int asn1Init_ValidRegion_area (ValidRegion_area* pvalue)
{
   pvalue->t = 0;
   OSCRTLMEMSET (&pvalue->u, 0, sizeof(pvalue->u));
   return 0;
}

/* ValidRegion */

int asn1Init_ValidRegion (ValidRegion* pvalue)
{
   OSCRTLMEMSET (&pvalue->m, 0, sizeof (pvalue->m));
   pvalue->direction = 0;
   asn1Init_ValidRegion_area (&pvalue->area);
   return 0;
}

/* GeographicalPath_description */

int asn1Init_GeographicalPath_description (
   GeographicalPath_description* pvalue)
{
   pvalue->t = 0;
   OSCRTLMEMSET (&pvalue->u, 0, sizeof(pvalue->u));
   return 0;
}

/* GeographicalPath_regional */

int asn1Init_GeographicalPath_regional (GeographicalPath_regional* pvalue)
{
   rtxDListFastInit (pvalue);
   return 0;
}

/* GeographicalPath */

int asn1Init_GeographicalPath (GeographicalPath* pvalue)
{
   OSCRTLMEMSET (&pvalue->m, 0, sizeof (pvalue->m));
   pvalue->name = 0;
   pvalue->id = 0;
   pvalue->anchor = 0;
   pvalue->direction = 0;
   asn1Init_GeographicalPath_description (&pvalue->description);
   asn1Init_GeographicalPath_regional (&pvalue->regional);
   rtxDListFastInit (&pvalue->extElem1);
   return 0;
}

/* TravelerDataFrame_regions */

int asn1Init_TravelerDataFrame_regions (TravelerDataFrame_regions* pvalue)
{
   rtxDListFastInit (pvalue);
   return 0;
}

/* WorkZone_element_item */

int asn1Init_WorkZone_element_item (WorkZone_element_item* pvalue)
{
   pvalue->t = 0;
   OSCRTLMEMSET (&pvalue->u, 0, sizeof(pvalue->u));
   return 0;
}

/* WorkZone_element */

int asn1Init_WorkZone_element (WorkZone_element* pvalue)
{
   asn1Init_WorkZone_element_item (&pvalue->item);
   return 0;
}

/* WorkZone */

int asn1Init_WorkZone (WorkZone* pvalue)
{
   rtxDListFastInit (pvalue);
   return 0;
}

/* GenericSignage_element_item */

int asn1Init_GenericSignage_element_item (GenericSignage_element_item* pvalue)
{
   pvalue->t = 0;
   OSCRTLMEMSET (&pvalue->u, 0, sizeof(pvalue->u));
   return 0;
}

/* GenericSignage_element */

int asn1Init_GenericSignage_element (GenericSignage_element* pvalue)
{
   asn1Init_GenericSignage_element_item (&pvalue->item);
   return 0;
}

/* GenericSignage */

int asn1Init_GenericSignage (GenericSignage* pvalue)
{
   rtxDListFastInit (pvalue);
   return 0;
}

/* SpeedLimit_element_item */

int asn1Init_SpeedLimit_element_item (SpeedLimit_element_item* pvalue)
{
   pvalue->t = 0;
   OSCRTLMEMSET (&pvalue->u, 0, sizeof(pvalue->u));
   return 0;
}

/* SpeedLimit_element */

int asn1Init_SpeedLimit_element (SpeedLimit_element* pvalue)
{
   asn1Init_SpeedLimit_element_item (&pvalue->item);
   return 0;
}

/* SpeedLimit */

int asn1Init_SpeedLimit (SpeedLimit* pvalue)
{
   rtxDListFastInit (pvalue);
   return 0;
}

/* ExitService_element_item */

int asn1Init_ExitService_element_item (ExitService_element_item* pvalue)
{
   pvalue->t = 0;
   OSCRTLMEMSET (&pvalue->u, 0, sizeof(pvalue->u));
   return 0;
}

/* ExitService_element */

int asn1Init_ExitService_element (ExitService_element* pvalue)
{
   asn1Init_ExitService_element_item (&pvalue->item);
   return 0;
}

/* ExitService */

int asn1Init_ExitService (ExitService* pvalue)
{
   rtxDListFastInit (pvalue);
   return 0;
}

/* TravelerDataFrame_content */

int asn1Init_TravelerDataFrame_content (TravelerDataFrame_content* pvalue)
{
   pvalue->t = 0;
   OSCRTLMEMSET (&pvalue->u, 0, sizeof(pvalue->u));
   return 0;
}

/* TravelerDataFrameNewPartIIIContent */

int asn1Init_TravelerDataFrameNewPartIIIContent (
   TravelerDataFrameNewPartIIIContent* pvalue)
{
   pvalue->t = 0;
   OSCRTLMEMSET (&pvalue->u, 0, sizeof(pvalue->u));
   return 0;
}

/* TravelerDataFrame */

int asn1Init_TravelerDataFrame (TravelerDataFrame* pvalue)
{
   OSCRTLMEMSET (&pvalue->m, 0, sizeof (pvalue->m));
   asn1Init_TravelerDataFrame_msgId (&pvalue->msgId);
   asn1Init_TravelerDataFrame_regions (&pvalue->regions);
   asn1Init_TravelerDataFrame_content (&pvalue->content);
   pvalue->url = 0;
   asn1Init_TravelerDataFrameNewPartIIIContent (&pvalue->contentNew);
   rtxDListFastInit (&pvalue->extElem1);
   return 0;
}

/* TravelerDataFrameList */

int asn1Init_TravelerDataFrameList (TravelerDataFrameList* pvalue)
{
   rtxDListFastInit (pvalue);
   return 0;
}

/* TravelerInformation_regional */

int asn1Init_TravelerInformation_regional (
   TravelerInformation_regional* pvalue)
{
   rtxDListFastInit (pvalue);
   return 0;
}

/* TravelerInformation */

int asn1Init_TravelerInformation (TravelerInformation* pvalue)
{
   OSCRTLMEMSET (&pvalue->m, 0, sizeof (pvalue->m));
   asn1Init_UniqueMSGID (&pvalue->packetID);
   pvalue->urlB = 0;
   asn1Init_TravelerDataFrameList (&pvalue->dataFrames);
   asn1Init_TravelerInformation_regional (&pvalue->regional);
   rtxDListFastInit (&pvalue->extElem1);
   return 0;
}
