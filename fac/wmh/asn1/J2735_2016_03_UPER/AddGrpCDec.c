/**
 * This file was generated by the Objective Systems ASN1C Compiler
 * (http://www.obj-sys.com).  Version: 7.2.4, Date: 06-Mar-2019.
 */
#include "AddGrpC.h"
#include "rtxsrc/rtxCommon.h"

EXTERN int asn1PD_AltitudeConfidence (OSCTXT* pctxt, AltitudeConfidence* pvalue)
{
   return rtxDecBits (pctxt, pvalue, 4);
}

EXTERN int asn1PD_AltitudeValue (OSCTXT* pctxt, AltitudeValue* pvalue)
{
   return pd_ConsInteger (pctxt, pvalue, -100000, 800001);
}

EXTERN int asn1PD_EmissionType (OSCTXT* pctxt, EmissionType* pvalue)
{
   int stat = 0;
   OSUINT32 ui;
   OSBOOL extbit = FALSE;

   RTDIAGSTRM2 (pctxt,"asn1PD_EmissionType: start\n");

   /* extension bit */

   PU_NEWFIELD (pctxt, "extension marker");

   stat = DEC_BIT (pctxt, &extbit);
   if (stat != 0) return stat;

   PU_SETBITCOUNT (pctxt);

   if (extbit) {
      PU_NEWFIELD (pctxt, "extension value");

      stat = pd_SmallNonNegWholeNumber (pctxt, &ui);
      if (stat != 0) return stat;

      PU_SETBITCOUNT (pctxt);

      *pvalue = 0x7fffffff;
   }
   else {
      stat = pd_ConsUnsigned (pctxt, pvalue, 0, OSUINTCONST(4));
      if (stat != 0) return stat;
   }

   RTDIAGSTRM2 (pctxt,"asn1PD_EmissionType: end\n");

   return (stat);
}

EXTERN int asn1PD_Altitude (OSCTXT* pctxt, Altitude* pvalue)
{
   int stat = 0;

   RTDIAGSTRM2 (pctxt,"asn1PD_Altitude: start\n");

   /* decode root elements */
   /* decode value */
   stat = asn1PD_AltitudeValue (pctxt, &pvalue->value);
   if (stat != 0) return stat;

   /* decode confidence */
   stat = asn1PD_AltitudeConfidence (pctxt, &pvalue->confidence);
   if (stat != 0) return stat;

   RTDIAGSTRM2 (pctxt,"asn1PD_Altitude: end\n");

   return (stat);
}

EXTERN int asn1PD_PrioritizationResponse (OSCTXT* pctxt, PrioritizationResponse* pvalue)
{
   int stat = 0;
   ASN1OpenType openType;
   ASN1OpenType* pOpenType;
   OSUINT32 bitcnt;
   OSUINT32 i_;
   OSBOOL extbit = FALSE;

   RTDIAGSTRM2 (pctxt,"asn1PD_PrioritizationResponse: start\n");

   /* extension bit */

   PU_NEWFIELD (pctxt, "extension marker");

   stat = DEC_BIT (pctxt, &extbit);
   if (stat != 0) return stat;

   PU_SETBITCOUNT (pctxt);

   rtxDListInit (&pvalue->extElem1);

   /* decode root elements */
   /* decode stationID */
   stat = asn1PD_StationID (pctxt, &pvalue->stationID);
   if (stat != 0) return stat;

   /* decode priorState */
   stat = asn1PD_PrioritizationResponseStatus (pctxt, &pvalue->priorState);
   if (stat != 0) return stat;

   /* decode signalGroup */
   stat = asn1PD_SignalGroupID (pctxt, &pvalue->signalGroup);
   if (stat != 0) return stat;

   /* decode extension elements */
   if (extbit) {
      OSOCTET* poptbits;

      /* decode extension optional bits length */

      PU_NEWFIELD (pctxt, "ext opt bits len");

      stat = pd_SmallLength (pctxt, &bitcnt);
      if (stat != 0) return stat;

      PU_SETBITCOUNT (pctxt);

      poptbits = (OSOCTET*) rtxMemAlloc (pctxt, bitcnt);
      if (0 == poptbits) return RTERR_NOMEM;

      PU_NEWFIELD (pctxt, "ext opt bit mask");

      for (i_ = 0; i_ < bitcnt; i_++) {
         stat = DEC_BIT (pctxt, &poptbits[i_]);
         if (stat != 0)  {
            rtxMemFreePtr (pctxt, poptbits);
            return stat;
         }
      }

      PU_SETBITCOUNT (pctxt);

      for (i_ = 0; i_ < bitcnt; i_++) {
         if (stat != 0) break;
         if (poptbits[i_]) {
            /* set ext elem name for trace */
            switch (i_) {
               case 0: default:;
            }

            stat = pd_OpenType (pctxt, &openType.data, &openType.numocts);

            if (0 == stat) {
               pOpenType = rtxMemAllocType (pctxt, ASN1OpenType);
               if (0 != pOpenType) {
                  pOpenType->numocts = openType.numocts;
                  pOpenType->data = openType.data;
                  rtxDListAppend (pctxt, &pvalue->extElem1, pOpenType);
               }
               else stat = RTERR_NOMEM;
            }
            else {
               LOG_RTERR (pctxt, stat);
               break;
            }
         }
         else {  /* unknown element */
            rtxDListAppend (pctxt, &pvalue->extElem1, 0);
         }
      }

      rtxMemFreePtr (pctxt, poptbits);
   }

   RTDIAGSTRM2 (pctxt,"asn1PD_PrioritizationResponse: end\n");

   return (stat);
}

EXTERN int asn1PD_PrioritizationResponseList (OSCTXT* pctxt, PrioritizationResponseList* pvalue)
{
   int stat = 0;
   OSRTDListNode* pnode;
   OSSIZE count = 0;
   OSSIZE xx1 = 0;

   RTDIAGSTRM2 (pctxt,"asn1PD_PrioritizationResponseList: start\n");

   /* decode length determinant */

   PU_SETSIZECONSTRAINT (pctxt, OSUINTCONST(1), OSUINTCONST(10), 0, 0);

   stat = pd_Length64 (pctxt, &count);
   if (stat != 0) return stat;

   /* decode elements */

   rtxDListInit (pvalue);

   for (xx1 = 0; xx1 < count; xx1++) {
      PrioritizationResponse* pdata;
      rtxDListAllocNodeAndData (pctxt, PrioritizationResponse, &pnode, &pdata);

      if (pnode == NULL)
         return RTERR_NOMEM;

      asn1Init_PrioritizationResponse (pdata);

      rtxDListAppendNode (pvalue, pnode);
      stat = asn1PD_PrioritizationResponse (pctxt, pdata);
      if (stat != 0) return stat;

   }

   RTDIAGSTRM2 (pctxt,"asn1PD_PrioritizationResponseList: end\n");

   return (stat);
}

EXTERN int asn1PD_VehicleToLanePosition (OSCTXT* pctxt, VehicleToLanePosition* pvalue)
{
   int stat = 0;
   ASN1OpenType openType;
   ASN1OpenType* pOpenType;
   OSUINT32 bitcnt;
   OSUINT32 i_;
   OSBOOL extbit = FALSE;

   RTDIAGSTRM2 (pctxt,"asn1PD_VehicleToLanePosition: start\n");

   /* extension bit */

   PU_NEWFIELD (pctxt, "extension marker");

   stat = DEC_BIT (pctxt, &extbit);
   if (stat != 0) return stat;

   PU_SETBITCOUNT (pctxt);

   rtxDListInit (&pvalue->extElem1);

   /* decode root elements */
   /* decode stationID */
   stat = asn1PD_StationID (pctxt, &pvalue->stationID);
   if (stat != 0) return stat;

   /* decode laneID */
   stat = asn1PD_LaneID (pctxt, &pvalue->laneID);
   if (stat != 0) return stat;

   /* decode extension elements */
   if (extbit) {
      OSOCTET* poptbits;

      /* decode extension optional bits length */

      PU_NEWFIELD (pctxt, "ext opt bits len");

      stat = pd_SmallLength (pctxt, &bitcnt);
      if (stat != 0) return stat;

      PU_SETBITCOUNT (pctxt);

      poptbits = (OSOCTET*) rtxMemAlloc (pctxt, bitcnt);
      if (0 == poptbits) return RTERR_NOMEM;

      PU_NEWFIELD (pctxt, "ext opt bit mask");

      for (i_ = 0; i_ < bitcnt; i_++) {
         stat = DEC_BIT (pctxt, &poptbits[i_]);
         if (stat != 0)  {
            rtxMemFreePtr (pctxt, poptbits);
            return stat;
         }
      }

      PU_SETBITCOUNT (pctxt);

      for (i_ = 0; i_ < bitcnt; i_++) {
         if (stat != 0) break;
         if (poptbits[i_]) {
            /* set ext elem name for trace */
            switch (i_) {
               case 0: default:;
            }

            stat = pd_OpenType (pctxt, &openType.data, &openType.numocts);

            if (0 == stat) {
               pOpenType = rtxMemAllocType (pctxt, ASN1OpenType);
               if (0 != pOpenType) {
                  pOpenType->numocts = openType.numocts;
                  pOpenType->data = openType.data;
                  rtxDListAppend (pctxt, &pvalue->extElem1, pOpenType);
               }
               else stat = RTERR_NOMEM;
            }
            else {
               LOG_RTERR (pctxt, stat);
               break;
            }
         }
         else {  /* unknown element */
            rtxDListAppend (pctxt, &pvalue->extElem1, 0);
         }
      }

      rtxMemFreePtr (pctxt, poptbits);
   }

   RTDIAGSTRM2 (pctxt,"asn1PD_VehicleToLanePosition: end\n");

   return (stat);
}

EXTERN int asn1PD_VehicleToLanePositionList (OSCTXT* pctxt, VehicleToLanePositionList* pvalue)
{
   int stat = 0;
   OSRTDListNode* pnode;
   OSSIZE count = 0;
   OSSIZE xx1 = 0;

   RTDIAGSTRM2 (pctxt,"asn1PD_VehicleToLanePositionList: start\n");

   /* decode length determinant */

   PU_SETSIZECONSTRAINT (pctxt, OSUINTCONST(1), OSUINTCONST(5), 0, 0);

   stat = pd_Length64 (pctxt, &count);
   if (stat != 0) return stat;

   /* decode elements */

   rtxDListInit (pvalue);

   for (xx1 = 0; xx1 < count; xx1++) {
      VehicleToLanePosition* pdata;
      rtxDListAllocNodeAndData (pctxt, VehicleToLanePosition, &pnode, &pdata);

      if (pnode == NULL)
         return RTERR_NOMEM;

      asn1Init_VehicleToLanePosition (pdata);

      rtxDListAppendNode (pvalue, pnode);
      stat = asn1PD_VehicleToLanePosition (pctxt, pdata);
      if (stat != 0) return stat;

   }

   RTDIAGSTRM2 (pctxt,"asn1PD_VehicleToLanePositionList: end\n");

   return (stat);
}

EXTERN int asn1PD_ConnectionManeuverAssist_addGrpC (OSCTXT* pctxt, ConnectionManeuverAssist_addGrpC* pvalue)
{
   int stat = 0;
   OSUINT32 i_;
   OSBOOL optbits[1];

   RTDIAGSTRM2 (pctxt,"asn1PD_ConnectionManeuverAssist_addGrpC: start\n");

   OSCRTLMEMSET (&pvalue->m, 0, sizeof(pvalue->m));

   /* optional bits */

   PU_NEWFIELD (pctxt, "optional bits");

   for (i_ = 0; i_ < 1; i_++) {
      stat = DEC_BIT (pctxt, &optbits[i_]);
      if (stat != 0) return stat;
   }

   PU_SETBITCOUNT (pctxt);

   /* decode root elements */
   /* decode vehicleToLanePositions */
   stat = asn1PD_VehicleToLanePositionList (pctxt, &pvalue->vehicleToLanePositions);
   if (stat != 0) return stat;

   /* decode rsuDistanceFromAnchor */
   if (optbits[0]) {
      pvalue->m.rsuDistanceFromAnchorPresent = 1;

      stat = asn1PD_NodeOffsetPointXY (pctxt, &pvalue->rsuDistanceFromAnchor);
      if (stat != 0) return stat;

   }

   RTDIAGSTRM2 (pctxt,"asn1PD_ConnectionManeuverAssist_addGrpC: end\n");

   return (stat);
}

EXTERN int asn1PD_IntersectionState_addGrpC (OSCTXT* pctxt, IntersectionState_addGrpC* pvalue)
{
   int stat = 0;
   ASN1OpenType openType;
   ASN1OpenType* pOpenType;
   OSUINT32 bitcnt;
   OSUINT32 i_;
   OSBOOL extbit = FALSE;
   OSBOOL optbits[1];

   RTDIAGSTRM2 (pctxt,"asn1PD_IntersectionState_addGrpC: start\n");

   /* extension bit */

   PU_NEWFIELD (pctxt, "extension marker");

   stat = DEC_BIT (pctxt, &extbit);
   if (stat != 0) return stat;

   PU_SETBITCOUNT (pctxt);

   OSCRTLMEMSET (&pvalue->m, 0, sizeof(pvalue->m));

   rtxDListInit (&pvalue->extElem1);

   /* optional bits */

   PU_NEWFIELD (pctxt, "optional bits");

   for (i_ = 0; i_ < 1; i_++) {
      stat = DEC_BIT (pctxt, &optbits[i_]);
      if (stat != 0) return stat;
   }

   PU_SETBITCOUNT (pctxt);

   /* decode root elements */
   /* decode activePrioritizations */
   if (optbits[0]) {
      pvalue->m.activePrioritizationsPresent = 1;

      stat = asn1PD_PrioritizationResponseList (pctxt, &pvalue->activePrioritizations);
      if (stat != 0) return stat;

   }

   /* decode extension elements */
   if (extbit) {
      OSOCTET* poptbits;

      /* decode extension optional bits length */

      PU_NEWFIELD (pctxt, "ext opt bits len");

      stat = pd_SmallLength (pctxt, &bitcnt);
      if (stat != 0) return stat;

      PU_SETBITCOUNT (pctxt);

      poptbits = (OSOCTET*) rtxMemAlloc (pctxt, bitcnt);
      if (0 == poptbits) return RTERR_NOMEM;

      PU_NEWFIELD (pctxt, "ext opt bit mask");

      for (i_ = 0; i_ < bitcnt; i_++) {
         stat = DEC_BIT (pctxt, &poptbits[i_]);
         if (stat != 0)  {
            rtxMemFreePtr (pctxt, poptbits);
            return stat;
         }
      }

      PU_SETBITCOUNT (pctxt);

      for (i_ = 0; i_ < bitcnt; i_++) {
         if (stat != 0) break;
         if (poptbits[i_]) {
            /* set ext elem name for trace */
            switch (i_) {
               case 0: default:;
            }

            stat = pd_OpenType (pctxt, &openType.data, &openType.numocts);

            if (0 == stat) {
               pOpenType = rtxMemAllocType (pctxt, ASN1OpenType);
               if (0 != pOpenType) {
                  pOpenType->numocts = openType.numocts;
                  pOpenType->data = openType.data;
                  rtxDListAppend (pctxt, &pvalue->extElem1, pOpenType);
               }
               else stat = RTERR_NOMEM;
            }
            else {
               LOG_RTERR (pctxt, stat);
               break;
            }
         }
         else {  /* unknown element */
            rtxDListAppend (pctxt, &pvalue->extElem1, 0);
         }
      }

      rtxMemFreePtr (pctxt, poptbits);
   }

   RTDIAGSTRM2 (pctxt,"asn1PD_IntersectionState_addGrpC: end\n");

   return (stat);
}

EXTERN int asn1PD_SignalHeadLocation (OSCTXT* pctxt, SignalHeadLocation* pvalue)
{
   int stat = 0;
   ASN1OpenType openType;
   ASN1OpenType* pOpenType;
   OSUINT32 bitcnt;
   OSUINT32 i_;
   OSBOOL extbit = FALSE;

   RTDIAGSTRM2 (pctxt,"asn1PD_SignalHeadLocation: start\n");

   /* extension bit */

   PU_NEWFIELD (pctxt, "extension marker");

   stat = DEC_BIT (pctxt, &extbit);
   if (stat != 0) return stat;

   PU_SETBITCOUNT (pctxt);

   rtxDListInit (&pvalue->extElem1);

   /* decode root elements */
   /* decode node */
   stat = asn1PD_NodeOffsetPointXY (pctxt, &pvalue->node);
   if (stat != 0) return stat;

   /* decode signalGroupID */
   stat = asn1PD_SignalGroupID (pctxt, &pvalue->signalGroupID);
   if (stat != 0) return stat;

   /* decode extension elements */
   if (extbit) {
      OSOCTET* poptbits;

      /* decode extension optional bits length */

      PU_NEWFIELD (pctxt, "ext opt bits len");

      stat = pd_SmallLength (pctxt, &bitcnt);
      if (stat != 0) return stat;

      PU_SETBITCOUNT (pctxt);

      poptbits = (OSOCTET*) rtxMemAlloc (pctxt, bitcnt);
      if (0 == poptbits) return RTERR_NOMEM;

      PU_NEWFIELD (pctxt, "ext opt bit mask");

      for (i_ = 0; i_ < bitcnt; i_++) {
         stat = DEC_BIT (pctxt, &poptbits[i_]);
         if (stat != 0)  {
            rtxMemFreePtr (pctxt, poptbits);
            return stat;
         }
      }

      PU_SETBITCOUNT (pctxt);

      for (i_ = 0; i_ < bitcnt; i_++) {
         if (stat != 0) break;
         if (poptbits[i_]) {
            /* set ext elem name for trace */
            switch (i_) {
               case 0: default:;
            }

            stat = pd_OpenType (pctxt, &openType.data, &openType.numocts);

            if (0 == stat) {
               pOpenType = rtxMemAllocType (pctxt, ASN1OpenType);
               if (0 != pOpenType) {
                  pOpenType->numocts = openType.numocts;
                  pOpenType->data = openType.data;
                  rtxDListAppend (pctxt, &pvalue->extElem1, pOpenType);
               }
               else stat = RTERR_NOMEM;
            }
            else {
               LOG_RTERR (pctxt, stat);
               break;
            }
         }
         else {  /* unknown element */
            rtxDListAppend (pctxt, &pvalue->extElem1, 0);
         }
      }

      rtxMemFreePtr (pctxt, poptbits);
   }

   RTDIAGSTRM2 (pctxt,"asn1PD_SignalHeadLocation: end\n");

   return (stat);
}

EXTERN int asn1PD_SignalHeadLocationList (OSCTXT* pctxt, SignalHeadLocationList* pvalue)
{
   int stat = 0;
   OSRTDListNode* pnode;
   OSSIZE count = 0;
   OSSIZE xx1 = 0;

   RTDIAGSTRM2 (pctxt,"asn1PD_SignalHeadLocationList: start\n");

   /* decode length determinant */

   PU_SETSIZECONSTRAINT (pctxt, OSUINTCONST(1), OSUINTCONST(20), 0, 0);

   stat = pd_Length64 (pctxt, &count);
   if (stat != 0) return stat;

   /* decode elements */

   rtxDListInit (pvalue);

   for (xx1 = 0; xx1 < count; xx1++) {
      SignalHeadLocation* pdata;
      rtxDListAllocNodeAndData (pctxt, SignalHeadLocation, &pnode, &pdata);

      if (pnode == NULL)
         return RTERR_NOMEM;

      asn1Init_SignalHeadLocation (pdata);

      rtxDListAppendNode (pvalue, pnode);
      stat = asn1PD_SignalHeadLocation (pctxt, pdata);
      if (stat != 0) return stat;

   }

   RTDIAGSTRM2 (pctxt,"asn1PD_SignalHeadLocationList: end\n");

   return (stat);
}

EXTERN int asn1PD_MapData_addGrpC (OSCTXT* pctxt, MapData_addGrpC* pvalue)
{
   int stat = 0;
   ASN1OpenType openType;
   ASN1OpenType* pOpenType;
   OSUINT32 bitcnt;
   OSUINT32 i_;
   OSBOOL extbit = FALSE;
   OSBOOL optbits[1];

   RTDIAGSTRM2 (pctxt,"asn1PD_MapData_addGrpC: start\n");

   /* extension bit */

   PU_NEWFIELD (pctxt, "extension marker");

   stat = DEC_BIT (pctxt, &extbit);
   if (stat != 0) return stat;

   PU_SETBITCOUNT (pctxt);

   OSCRTLMEMSET (&pvalue->m, 0, sizeof(pvalue->m));

   rtxDListInit (&pvalue->extElem1);

   /* optional bits */

   PU_NEWFIELD (pctxt, "optional bits");

   for (i_ = 0; i_ < 1; i_++) {
      stat = DEC_BIT (pctxt, &optbits[i_]);
      if (stat != 0) return stat;
   }

   PU_SETBITCOUNT (pctxt);

   /* decode root elements */
   /* decode signalHeadLocations */
   if (optbits[0]) {
      pvalue->m.signalHeadLocationsPresent = 1;

      stat = asn1PD_SignalHeadLocationList (pctxt, &pvalue->signalHeadLocations);
      if (stat != 0) return stat;

   }

   /* decode extension elements */
   if (extbit) {
      OSOCTET* poptbits;

      /* decode extension optional bits length */

      PU_NEWFIELD (pctxt, "ext opt bits len");

      stat = pd_SmallLength (pctxt, &bitcnt);
      if (stat != 0) return stat;

      PU_SETBITCOUNT (pctxt);

      poptbits = (OSOCTET*) rtxMemAlloc (pctxt, bitcnt);
      if (0 == poptbits) return RTERR_NOMEM;

      PU_NEWFIELD (pctxt, "ext opt bit mask");

      for (i_ = 0; i_ < bitcnt; i_++) {
         stat = DEC_BIT (pctxt, &poptbits[i_]);
         if (stat != 0)  {
            rtxMemFreePtr (pctxt, poptbits);
            return stat;
         }
      }

      PU_SETBITCOUNT (pctxt);

      for (i_ = 0; i_ < bitcnt; i_++) {
         if (stat != 0) break;
         if (poptbits[i_]) {
            /* set ext elem name for trace */
            switch (i_) {
               case 0: default:;
            }

            stat = pd_OpenType (pctxt, &openType.data, &openType.numocts);

            if (0 == stat) {
               pOpenType = rtxMemAllocType (pctxt, ASN1OpenType);
               if (0 != pOpenType) {
                  pOpenType->numocts = openType.numocts;
                  pOpenType->data = openType.data;
                  rtxDListAppend (pctxt, &pvalue->extElem1, pOpenType);
               }
               else stat = RTERR_NOMEM;
            }
            else {
               LOG_RTERR (pctxt, stat);
               break;
            }
         }
         else {  /* unknown element */
            rtxDListAppend (pctxt, &pvalue->extElem1, 0);
         }
      }

      rtxMemFreePtr (pctxt, poptbits);
   }

   RTDIAGSTRM2 (pctxt,"asn1PD_MapData_addGrpC: end\n");

   return (stat);
}

EXTERN int asn1PD_Position3D_addGrpC (OSCTXT* pctxt, Position3D_addGrpC* pvalue)
{
   int stat = 0;
   ASN1OpenType openType;
   ASN1OpenType* pOpenType;
   OSUINT32 bitcnt;
   OSUINT32 i_;
   OSBOOL extbit = FALSE;

   RTDIAGSTRM2 (pctxt,"asn1PD_Position3D_addGrpC: start\n");

   /* extension bit */

   PU_NEWFIELD (pctxt, "extension marker");

   stat = DEC_BIT (pctxt, &extbit);
   if (stat != 0) return stat;

   PU_SETBITCOUNT (pctxt);

   rtxDListInit (&pvalue->extElem1);

   /* decode root elements */
   /* decode altitude */
   stat = asn1PD_Altitude (pctxt, &pvalue->altitude);
   if (stat != 0) return stat;

   /* decode extension elements */
   if (extbit) {
      OSOCTET* poptbits;

      /* decode extension optional bits length */

      PU_NEWFIELD (pctxt, "ext opt bits len");

      stat = pd_SmallLength (pctxt, &bitcnt);
      if (stat != 0) return stat;

      PU_SETBITCOUNT (pctxt);

      poptbits = (OSOCTET*) rtxMemAlloc (pctxt, bitcnt);
      if (0 == poptbits) return RTERR_NOMEM;

      PU_NEWFIELD (pctxt, "ext opt bit mask");

      for (i_ = 0; i_ < bitcnt; i_++) {
         stat = DEC_BIT (pctxt, &poptbits[i_]);
         if (stat != 0)  {
            rtxMemFreePtr (pctxt, poptbits);
            return stat;
         }
      }

      PU_SETBITCOUNT (pctxt);

      for (i_ = 0; i_ < bitcnt; i_++) {
         if (stat != 0) break;
         if (poptbits[i_]) {
            /* set ext elem name for trace */
            switch (i_) {
               case 0: default:;
            }

            stat = pd_OpenType (pctxt, &openType.data, &openType.numocts);

            if (0 == stat) {
               pOpenType = rtxMemAllocType (pctxt, ASN1OpenType);
               if (0 != pOpenType) {
                  pOpenType->numocts = openType.numocts;
                  pOpenType->data = openType.data;
                  rtxDListAppend (pctxt, &pvalue->extElem1, pOpenType);
               }
               else stat = RTERR_NOMEM;
            }
            else {
               LOG_RTERR (pctxt, stat);
               break;
            }
         }
         else {  /* unknown element */
            rtxDListAppend (pctxt, &pvalue->extElem1, 0);
         }
      }

      rtxMemFreePtr (pctxt, poptbits);
   }

   RTDIAGSTRM2 (pctxt,"asn1PD_Position3D_addGrpC: end\n");

   return (stat);
}

EXTERN int asn1PD_RestrictionUserType_addGrpC (OSCTXT* pctxt, RestrictionUserType_addGrpC* pvalue)
{
   int stat = 0;
   ASN1OpenType openType;
   ASN1OpenType* pOpenType;
   OSUINT32 bitcnt;
   OSUINT32 i_;
   OSBOOL extbit = FALSE;
   OSBOOL optbits[1];

   RTDIAGSTRM2 (pctxt,"asn1PD_RestrictionUserType_addGrpC: start\n");

   /* extension bit */

   PU_NEWFIELD (pctxt, "extension marker");

   stat = DEC_BIT (pctxt, &extbit);
   if (stat != 0) return stat;

   PU_SETBITCOUNT (pctxt);

   OSCRTLMEMSET (&pvalue->m, 0, sizeof(pvalue->m));

   rtxDListInit (&pvalue->extElem1);

   /* optional bits */

   PU_NEWFIELD (pctxt, "optional bits");

   for (i_ = 0; i_ < 1; i_++) {
      stat = DEC_BIT (pctxt, &optbits[i_]);
      if (stat != 0) return stat;
   }

   PU_SETBITCOUNT (pctxt);

   /* decode root elements */
   /* decode emission */
   if (optbits[0]) {
      pvalue->m.emissionPresent = 1;

      stat = asn1PD_EmissionType (pctxt, &pvalue->emission);
      if (stat != 0) return stat;

   }

   /* decode extension elements */
   if (extbit) {
      OSOCTET* poptbits;

      /* decode extension optional bits length */

      PU_NEWFIELD (pctxt, "ext opt bits len");

      stat = pd_SmallLength (pctxt, &bitcnt);
      if (stat != 0) return stat;

      PU_SETBITCOUNT (pctxt);

      poptbits = (OSOCTET*) rtxMemAlloc (pctxt, bitcnt);
      if (0 == poptbits) return RTERR_NOMEM;

      PU_NEWFIELD (pctxt, "ext opt bit mask");

      for (i_ = 0; i_ < bitcnt; i_++) {
         stat = DEC_BIT (pctxt, &poptbits[i_]);
         if (stat != 0)  {
            rtxMemFreePtr (pctxt, poptbits);
            return stat;
         }
      }

      PU_SETBITCOUNT (pctxt);

      for (i_ = 0; i_ < bitcnt; i_++) {
         if (stat != 0) break;
         if (poptbits[i_]) {
            /* set ext elem name for trace */
            switch (i_) {
               case 0: default:;
            }

            stat = pd_OpenType (pctxt, &openType.data, &openType.numocts);

            if (0 == stat) {
               pOpenType = rtxMemAllocType (pctxt, ASN1OpenType);
               if (0 != pOpenType) {
                  pOpenType->numocts = openType.numocts;
                  pOpenType->data = openType.data;
                  rtxDListAppend (pctxt, &pvalue->extElem1, pOpenType);
               }
               else stat = RTERR_NOMEM;
            }
            else {
               LOG_RTERR (pctxt, stat);
               break;
            }
         }
         else {  /* unknown element */
            rtxDListAppend (pctxt, &pvalue->extElem1, 0);
         }
      }

      rtxMemFreePtr (pctxt, poptbits);
   }

   RTDIAGSTRM2 (pctxt,"asn1PD_RestrictionUserType_addGrpC: end\n");

   return (stat);
}
