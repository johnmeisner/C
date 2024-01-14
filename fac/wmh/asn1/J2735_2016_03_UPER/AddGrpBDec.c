/**
 * This file was generated by the Objective Systems ASN1C Compiler
 * (http://www.obj-sys.com).  Version: 7.2.4, Date: 06-Mar-2019.
 */
#include "AddGrpB.h"
#include "rtxsrc/rtxCommon.h"

EXTERN int asn1PD_AddGrpB_Angle (OSCTXT* pctxt, AddGrpB_Angle* pvalue)
{
   int stat;

   stat = rtxDecBitsToByte (pctxt, pvalue, 8);
   if (stat != 0) return stat;

   if (*pvalue > 239) return RTERR_CONSVIO;

   return stat;
}

EXTERN int asn1PD_Day (OSCTXT* pctxt, Day* pvalue)
{
   int stat;

   if (pctxt->buffer.aligned) {
      int stat2 = PD_BYTE_ALIGN (pctxt);
      if (stat2 != 0) return LOG_RTERR (pctxt, stat2);
   }
   stat = rtxDecBitsToByte (pctxt, pvalue, 8);
   if (stat != 0) return stat;

   return stat;
}

EXTERN int asn1PD_DayOfWeek (OSCTXT* pctxt, DayOfWeek* pvalue)
{
   return rtxDecBits (pctxt, pvalue, 3);
}

EXTERN int asn1PD_DegreesLat (OSCTXT* pctxt, DegreesLat* pvalue)
{
   return pd_ConsInt8 (pctxt, pvalue, -90, 90);
}

EXTERN int asn1PD_DegreesLong (OSCTXT* pctxt, DegreesLong* pvalue)
{
   return pd_ConsInt16 (pctxt, pvalue, -180, 180);
}

EXTERN int asn1PD_AddGrpB_Elevation (OSCTXT* pctxt, AddGrpB_Elevation* pvalue)
{
   return pd_ConsInt16 (pctxt, pvalue, -32768, 32767);
}

EXTERN int asn1PD_Holiday (OSCTXT* pctxt, Holiday* pvalue)
{
   return rtxDecBits (pctxt, pvalue, 1);
}

EXTERN int asn1PD_Hour (OSCTXT* pctxt, Hour* pvalue)
{
   int stat;

   if (pctxt->buffer.aligned) {
      int stat2 = PD_BYTE_ALIGN (pctxt);
      if (stat2 != 0) return LOG_RTERR (pctxt, stat2);
   }
   stat = rtxDecBitsToByte (pctxt, pvalue, 8);
   if (stat != 0) return stat;

   return stat;
}

EXTERN int asn1PD_LatitudeDMS (OSCTXT* pctxt, LatitudeDMS* pvalue)
{
   return pd_ConsInteger (pctxt, pvalue, -32400000, 32400000);
}

EXTERN int asn1PD_LongitudeDMS (OSCTXT* pctxt, LongitudeDMS* pvalue)
{
   return pd_ConsInteger (pctxt, pvalue, -64800000, 64800000);
}

EXTERN int asn1PD_MaxTimetoChange (OSCTXT* pctxt, MaxTimetoChange* pvalue)
{
   int stat;

   if (pctxt->buffer.aligned) {
      int stat2 = PD_BYTE_ALIGN (pctxt);
      if (stat2 != 0) return LOG_RTERR (pctxt, stat2);
   }
   stat = rtxDecBitsToUInt16 (pctxt, pvalue, pctxt->buffer.aligned ? 16 : 12);
   if (stat != 0) return stat;

   if (*pvalue > 2402) return RTERR_CONSVIO;

   return stat;
}

EXTERN int asn1PD_MinTimetoChange (OSCTXT* pctxt, MinTimetoChange* pvalue)
{
   int stat;

   if (pctxt->buffer.aligned) {
      int stat2 = PD_BYTE_ALIGN (pctxt);
      if (stat2 != 0) return LOG_RTERR (pctxt, stat2);
   }
   stat = rtxDecBitsToUInt16 (pctxt, pvalue, pctxt->buffer.aligned ? 16 : 12);
   if (stat != 0) return stat;

   if (*pvalue > 2402) return RTERR_CONSVIO;

   return stat;
}

EXTERN int asn1PD_Minute (OSCTXT* pctxt, Minute* pvalue)
{
   int stat;

   if (pctxt->buffer.aligned) {
      int stat2 = PD_BYTE_ALIGN (pctxt);
      if (stat2 != 0) return LOG_RTERR (pctxt, stat2);
   }
   stat = rtxDecBitsToByte (pctxt, pvalue, 8);
   if (stat != 0) return stat;

   return stat;
}

EXTERN int asn1PD_MinutesAngle (OSCTXT* pctxt, MinutesAngle* pvalue)
{
   int stat;

   stat = rtxDecBitsToByte (pctxt, pvalue, 6);
   if (stat != 0) return stat;

   if (*pvalue > 59) return RTERR_CONSVIO;

   return stat;
}

EXTERN int asn1PD_Month (OSCTXT* pctxt, Month* pvalue)
{
   int stat;

   stat = rtxDecBitsToByte (pctxt, pvalue, 8);
   if (stat != 0) return stat;

   if (*pvalue > 254) return RTERR_CONSVIO;

   *pvalue += 1;

   return stat;
}

EXTERN int asn1PD_AddGrpB_MsgCount (OSCTXT* pctxt, AddGrpB_MsgCount* pvalue)
{
   int stat;

   if (pctxt->buffer.aligned) {
      int stat2 = PD_BYTE_ALIGN (pctxt);
      if (stat2 != 0) return LOG_RTERR (pctxt, stat2);
   }
   stat = rtxDecBitsToByte (pctxt, pvalue, 8);
   if (stat != 0) return stat;

   return stat;
}

EXTERN int asn1PD_Second (OSCTXT* pctxt, Second* pvalue)
{
   int stat;

   stat = rtxDecBitsToByte (pctxt, pvalue, 6);
   if (stat != 0) return stat;

   if (*pvalue > 60) return RTERR_CONSVIO;

   return stat;
}

EXTERN int asn1PD_SecondsAngle (OSCTXT* pctxt, SecondsAngle* pvalue)
{
   int stat;

   if (pctxt->buffer.aligned) {
      int stat2 = PD_BYTE_ALIGN (pctxt);
      if (stat2 != 0) return LOG_RTERR (pctxt, stat2);
   }
   stat = rtxDecBitsToUInt16 (pctxt, pvalue, pctxt->buffer.aligned ? 16 : 13);
   if (stat != 0) return stat;

   if (*pvalue > 5999) return RTERR_CONSVIO;

   return stat;
}

EXTERN int asn1PD_SummerTime (OSCTXT* pctxt, SummerTime* pvalue)
{
   return rtxDecBits (pctxt, pvalue, 1);
}

EXTERN int asn1PD_TenthSecond (OSCTXT* pctxt, TenthSecond* pvalue)
{
   int stat;

   stat = rtxDecBitsToByte (pctxt, pvalue, 4);
   if (stat != 0) return stat;

   if (*pvalue > 9) return RTERR_CONSVIO;

   return stat;
}

EXTERN int asn1PD_TimeRemaining (OSCTXT* pctxt, TimeRemaining* pvalue)
{
   int stat;

   if (pctxt->buffer.aligned) {
      int stat2 = PD_BYTE_ALIGN (pctxt);
      if (stat2 != 0) return LOG_RTERR (pctxt, stat2);
   }
   stat = rtxDecBitsToUInt16 (pctxt, pvalue, pctxt->buffer.aligned ? 16 : 14);
   if (stat != 0) return stat;

   if (*pvalue > 9001) return RTERR_CONSVIO;

   return stat;
}

EXTERN int asn1PD_Year (OSCTXT* pctxt, Year* pvalue)
{
   int stat;

   if (pctxt->buffer.aligned) {
      int stat2 = PD_BYTE_ALIGN (pctxt);
      if (stat2 != 0) return LOG_RTERR (pctxt, stat2);
   }
   stat = rtxDecBitsToUInt16 (pctxt, pvalue, pctxt->buffer.aligned ? 16 : 16);
   if (stat != 0) return stat;

   if (*pvalue > 65534) return RTERR_CONSVIO;

   *pvalue += 1;

   return stat;
}

EXTERN int asn1PD_LatitudeDMS2 (OSCTXT* pctxt, LatitudeDMS2* pvalue)
{
   int stat = 0;

   RTDIAGSTRM2 (pctxt,"asn1PD_LatitudeDMS2: start\n");

   /* decode root elements */
   /* decode d */
   stat = asn1PD_DegreesLat (pctxt, &pvalue->d);
   if (stat != 0) return stat;

   /* decode m_ */
   stat = asn1PD_MinutesAngle (pctxt, &pvalue->m_);
   if (stat != 0) return stat;

   /* decode s */
   stat = asn1PD_SecondsAngle (pctxt, &pvalue->s);
   if (stat != 0) return stat;

   RTDIAGSTRM2 (pctxt,"asn1PD_LatitudeDMS2: end\n");

   return (stat);
}

EXTERN int asn1PD_LongitudeDMS2 (OSCTXT* pctxt, LongitudeDMS2* pvalue)
{
   int stat = 0;

   RTDIAGSTRM2 (pctxt,"asn1PD_LongitudeDMS2: start\n");

   /* decode root elements */
   /* decode d */
   stat = asn1PD_DegreesLong (pctxt, &pvalue->d);
   if (stat != 0) return stat;

   /* decode m_ */
   stat = asn1PD_MinutesAngle (pctxt, &pvalue->m_);
   if (stat != 0) return stat;

   /* decode s */
   stat = asn1PD_SecondsAngle (pctxt, &pvalue->s);
   if (stat != 0) return stat;

   RTDIAGSTRM2 (pctxt,"asn1PD_LongitudeDMS2: end\n");

   return (stat);
}

EXTERN int asn1PD_Node_LLdms_48b (OSCTXT* pctxt, Node_LLdms_48b* pvalue)
{
   int stat = 0;

   RTDIAGSTRM2 (pctxt,"asn1PD_Node_LLdms_48b: start\n");

   /* decode root elements */
   /* decode lon */
   stat = asn1PD_LongitudeDMS (pctxt, &pvalue->lon);
   if (stat != 0) return stat;

   /* decode lat */
   stat = asn1PD_LatitudeDMS (pctxt, &pvalue->lat);
   if (stat != 0) return stat;

   RTDIAGSTRM2 (pctxt,"asn1PD_Node_LLdms_48b: end\n");

   return (stat);
}

EXTERN int asn1PD_Node_LLdms_80b (OSCTXT* pctxt, Node_LLdms_80b* pvalue)
{
   int stat = 0;

   RTDIAGSTRM2 (pctxt,"asn1PD_Node_LLdms_80b: start\n");

   /* decode root elements */
   /* decode lon */
   stat = asn1PD_LongitudeDMS2 (pctxt, &pvalue->lon);
   if (stat != 0) return stat;

   /* decode lat */
   stat = asn1PD_LatitudeDMS2 (pctxt, &pvalue->lat);
   if (stat != 0) return stat;

   RTDIAGSTRM2 (pctxt,"asn1PD_Node_LLdms_80b: end\n");

   return (stat);
}

EXTERN int asn1PD_LaneDataAttribute_addGrpB (OSCTXT* pctxt, LaneDataAttribute_addGrpB* pvalue)
{
   int stat = 0;
   ASN1OpenType openType;
   ASN1OpenType* pOpenType;
   OSUINT32 bitcnt;
   OSUINT32 i_;
   OSBOOL extbit = FALSE;
   OS_UNUSED_ARG (pvalue) ;

   RTDIAGSTRM2 (pctxt,"asn1PD_LaneDataAttribute_addGrpB: start\n");

   /* extension bit */

   PU_NEWFIELD (pctxt, "extension marker");

   stat = DEC_BIT (pctxt, &extbit);
   if (stat != 0) return stat;

   PU_SETBITCOUNT (pctxt);

   rtxDListInit (&pvalue->extElem1);

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

   RTDIAGSTRM2 (pctxt,"asn1PD_LaneDataAttribute_addGrpB: end\n");

   return (stat);
}

EXTERN int asn1PD_MovementEvent_addGrpB (OSCTXT* pctxt, MovementEvent_addGrpB* pvalue)
{
   int stat = 0;
   ASN1OpenType openType;
   ASN1OpenType* pOpenType;
   OSUINT32 bitcnt;
   OSUINT32 i_;
   OSBOOL extbit = FALSE;
   OSBOOL optbits[5];
   OSUINT32 j_ = 0;

   RTDIAGSTRM2 (pctxt,"asn1PD_MovementEvent_addGrpB: start\n");

   /* extension bit */

   PU_NEWFIELD (pctxt, "extension marker");

   stat = DEC_BIT (pctxt, &extbit);
   if (stat != 0) return stat;

   PU_SETBITCOUNT (pctxt);

   OSCRTLMEMSET (&pvalue->m, 0, sizeof(pvalue->m));

   rtxDListInit (&pvalue->extElem1);

   /* optional bits */

   PU_NEWFIELD (pctxt, "optional bits");

   for (i_ = 0; i_ < 5; i_++) {
      stat = DEC_BIT (pctxt, &optbits[i_]);
      if (stat != 0) return stat;
   }

   PU_SETBITCOUNT (pctxt);

   /* decode root elements */
   for (i_ = 0; i_ < 6; i_++) {
      switch (i_) {
         case 0:
         /* decode startTime */
         if (j_ < sizeof(optbits) && optbits[j_++]) {
            pvalue->m.startTimePresent = 1;

            stat = asn1PD_TimeRemaining (pctxt, &pvalue->startTime);

         }
         break;

         case 1:
         /* decode minEndTime */
         stat = asn1PD_MinTimetoChange (pctxt, &pvalue->minEndTime);

         break;

         case 2:
         /* decode maxEndTime */
         if (j_ < sizeof(optbits) && optbits[j_++]) {
            pvalue->m.maxEndTimePresent = 1;

            stat = asn1PD_MaxTimetoChange (pctxt, &pvalue->maxEndTime);

         }
         break;

         case 3:
         /* decode likelyTime */
         if (j_ < sizeof(optbits) && optbits[j_++]) {
            pvalue->m.likelyTimePresent = 1;

            stat = asn1PD_TimeRemaining (pctxt, &pvalue->likelyTime);

         }
         break;

         case 4:
         /* decode confidence */
         if (j_ < sizeof(optbits) && optbits[j_++]) {
            pvalue->m.confidencePresent = 1;

            stat = asn1PD_TimeIntervalConfidence (pctxt, &pvalue->confidence);

         }
         break;

         case 5:
         /* decode nextTime */
         if (j_ < sizeof(optbits) && optbits[j_++]) {
            pvalue->m.nextTimePresent = 1;

            stat = asn1PD_TimeRemaining (pctxt, &pvalue->nextTime);

         }
         break;

         default: j_++;
      }
      if (0 != stat) return stat;
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

   RTDIAGSTRM2 (pctxt,"asn1PD_MovementEvent_addGrpB: end\n");

   return (stat);
}

EXTERN int asn1PD_NodeOffsetPointXY_addGrpB (OSCTXT* pctxt, NodeOffsetPointXY_addGrpB* pvalue)
{
   int stat = 0;
   OSUINT32 ui;
   OSBOOL extbit = FALSE;
   ASN1OpenType openType;

   RTDIAGSTRM2 (pctxt,"asn1PD_NodeOffsetPointXY_addGrpB: start\n");

   /* extension bit */

   PU_NEWFIELD (pctxt, "extension marker");

   stat = DEC_BIT (pctxt, &extbit);
   if (stat != 0) return stat;

   PU_SETBITCOUNT (pctxt);

   if (!extbit) {
      RTXCTXTPUSHELEMNAME (pctxt, "t");

      {
         OSBOOL b = 0;
         DEC_BIT (pctxt, &b);
         ui = b;
      }
      pvalue->t = ui + 1;

      RTXCTXTPOPELEMNAME (pctxt);

      switch (ui) {
         /* posA */
         case 0:
            pvalue->u.posA = rtxMemAllocTypeZ (pctxt, Node_LLdms_48b);

            if (pvalue->u.posA == NULL)
               return RTERR_NOMEM;

            stat = asn1PD_Node_LLdms_48b (pctxt, pvalue->u.posA);
            if (stat != 0) return stat;

            break;

         /* posB */
         case 1:
            pvalue->u.posB = rtxMemAllocTypeZ (pctxt, Node_LLdms_80b);

            if (pvalue->u.posB == NULL)
               return RTERR_NOMEM;

            stat = asn1PD_Node_LLdms_80b (pctxt, pvalue->u.posB);
            if (stat != 0) return stat;

            break;

         default:
            return RTERR_INVOPT;
      }
   }
   else {
      PU_NEWFIELD (pctxt, "choice index");

      stat = pd_SmallNonNegWholeNumber (pctxt, &ui);
      if (stat != 0) return stat;
      else pvalue->t = ui + 3;

      if (pvalue->t < 3) return RTERR_INVOPT;

      PU_SETBITCOUNT (pctxt);

      stat = PD_BYTE_ALIGN (pctxt);
      if (stat != 0) return stat;

      stat = pd_OpenType (pctxt, &openType.data, &openType.numocts);
      if (stat != 0) return stat;

      pvalue->u.extElem1 = rtxMemAllocType (pctxt, ASN1OpenType);

      if (pvalue->u.extElem1 == NULL)
         return RTERR_NOMEM;

      pvalue->u.extElem1->data = openType.data;
      pvalue->u.extElem1->numocts = openType.numocts;

   }

   RTDIAGSTRM2 (pctxt,"asn1PD_NodeOffsetPointXY_addGrpB: end\n");

   return (stat);
}

EXTERN int asn1PD_Position3D_addGrpB (OSCTXT* pctxt, Position3D_addGrpB* pvalue)
{
   int stat = 0;
   ASN1OpenType openType;
   ASN1OpenType* pOpenType;
   OSUINT32 bitcnt;
   OSUINT32 i_;
   OSBOOL extbit = FALSE;

   RTDIAGSTRM2 (pctxt,"asn1PD_Position3D_addGrpB: start\n");

   /* extension bit */

   PU_NEWFIELD (pctxt, "extension marker");

   stat = DEC_BIT (pctxt, &extbit);
   if (stat != 0) return stat;

   PU_SETBITCOUNT (pctxt);

   rtxDListInit (&pvalue->extElem1);

   /* decode root elements */
   /* decode latitude */
   stat = asn1PD_LatitudeDMS2 (pctxt, &pvalue->latitude);
   if (stat != 0) return stat;

   /* decode longitude */
   stat = asn1PD_LongitudeDMS2 (pctxt, &pvalue->longitude);
   if (stat != 0) return stat;

   /* decode elevation */
   stat = asn1PD_AddGrpB_Elevation (pctxt, &pvalue->elevation);
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

   RTDIAGSTRM2 (pctxt,"asn1PD_Position3D_addGrpB: end\n");

   return (stat);
}

EXTERN int asn1PD_AddGrpB_TimeMark (OSCTXT* pctxt, AddGrpB_TimeMark* pvalue)
{
   int stat = 0;

   RTDIAGSTRM2 (pctxt,"asn1PD_AddGrpB_TimeMark: start\n");

   /* decode root elements */
   /* decode year */
   stat = asn1PD_Year (pctxt, &pvalue->year);
   if (stat != 0) return stat;

   /* decode month */
   stat = asn1PD_Month (pctxt, &pvalue->month);
   if (stat != 0) return stat;

   /* decode day */
   stat = asn1PD_Day (pctxt, &pvalue->day);
   if (stat != 0) return stat;

   /* decode summerTime */
   stat = asn1PD_SummerTime (pctxt, &pvalue->summerTime);
   if (stat != 0) return stat;

   /* decode holiday */
   stat = asn1PD_Holiday (pctxt, &pvalue->holiday);
   if (stat != 0) return stat;

   /* decode dayofWeek */
   stat = asn1PD_DayOfWeek (pctxt, &pvalue->dayofWeek);
   if (stat != 0) return stat;

   /* decode hour */
   stat = asn1PD_Hour (pctxt, &pvalue->hour);
   if (stat != 0) return stat;

   /* decode minute */
   stat = asn1PD_Minute (pctxt, &pvalue->minute);
   if (stat != 0) return stat;

   /* decode second */
   stat = asn1PD_Second (pctxt, &pvalue->second);
   if (stat != 0) return stat;

   /* decode tenthSecond */
   stat = asn1PD_TenthSecond (pctxt, &pvalue->tenthSecond);
   if (stat != 0) return stat;

   RTDIAGSTRM2 (pctxt,"asn1PD_AddGrpB_TimeMark: end\n");

   return (stat);
}
