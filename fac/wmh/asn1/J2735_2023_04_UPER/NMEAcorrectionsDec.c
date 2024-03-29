/**
 * This file was generated by the Objective Systems ASN1C Compiler
 * (http://www.obj-sys.com).  Version: 7.7.0, Date: 25-May-2023.
 *
 * Command:  asn1c CooperativeControlMsg-v1-ph.asn J2735-AddGrpB.asn J2735-AddGrpC.asn J2735-BasicSafetyMessage-v4.asn J2735-CommonSafetyRequest.asn J2735-Common-v7.asn J2735-EmergencyVehicleAlert.asn J2735-IntersectionCollision.asn J2735-ITIS.asn J2735-MapData-v1.asn J2735-MessageFrame-v5.asn J2735-NMEAcorrections.asn J2735-NTCIP.asn J2735-PersonalSafetyMessage.asn J2735-ProbeDataManagement.asn J2735-ProbeVehicleData.asn J2735-REGION.asn J2735-RoadSideAlert.asn J2735-RTCMcorrections.asn J2735-SignalRequestMessage.asn J2735-SignalStatusMessage.asn J2735-SPAT-v2.asn J2735-TestMessage00.asn J2735-TestMessage01.asn J2735-TestMessage02.asn J2735-TestMessage03.asn J2735-TestMessage04.asn J2735-TestMessage05.asn J2735-TestMessage06.asn J2735-TestMessage07.asn J2735-TestMessage08.asn J2735-TestMessage09.asn J2735-TestMessage10.asn J2735-TestMessage11.asn J2735-TestMessage12.asn J2735-TestMessage13.asn J2735-TestMessage14.asn J2735-TestMessage15.asn J2735-TravelerInformation-v2.asn MnvrSharingCoordMsg-v1-ph.asn PersonalSafetyMsg2-v1-ph.asn ProbeDataConfig-v0.30.asn ProbeDataReport-v0.30.asn RoadGeoAttributes-v2-ph.asn RoadSafetyMsg-v1-ph.asn RoadWeatherMessage.asn SensorDataSharingMsg-v1-ph.asn SignalControlAndPrioritizationRequest-v1-ph.asn SignalControlAndPrioritizationStatus-v1-ph.asn TollAdvertisementMsg-v0.10-ph.asn TollUsageAckMsg-v0.10-ph.asn TollUsageMsg-v0.10-ph.asn TrafficSignalPhaseAndTiming-v1-ph.asn -c -per -compact
 */
#include "NMEAcorrections.h"
#include "REGION.h"
#include "Common.h"
#include "rtxsrc/rtxCharStr.h"

EXTERN int asn1PD_NMEA_MsgType (OSCTXT* pctxt, NMEA_MsgType* pvalue)
{
   int stat;

   PD_BYTE_ALIGN(pctxt);
   stat = rtxDecBitsToUInt16(pctxt, pvalue, pctxt->buffer.aligned ? 16 : 15);
   if (stat != 0) return stat;

   if (*pvalue > 32767) {
      return RTERR_CONSVIO;
   }

   return stat;
}

EXTERN int asn1PD_NMEA_Payload (OSCTXT* pctxt, NMEA_Payload* pvalue)
{
   int stat = 0;

   PU_SETSIZECONSTRAINT (pctxt, OSUINTCONST(1), OSUINTCONST(1023), 0, 0);

   stat = pd_OctetString (pctxt, &pvalue->numocts, pvalue->data, sizeof(pvalue->data));
   if (stat != 0) return stat;

   return (stat);
}

EXTERN int asn1PD_NMEA_Revision (OSCTXT* pctxt, NMEA_Revision* pvalue)
{
   int stat = 0;
   OSBOOL extbit = FALSE;

   /* extension bit */

   rtxDecBit(pctxt, &extbit);

   if (extbit) {
      stat = pd_SmallNonNegWholeNumber(pctxt, NULL);
      if (stat != 0) return stat;

      *pvalue = (OSUINT8)-1;
   }
   else {
      {
         OSUINT32 tmpval;
         stat = pd_ConsUnsigned (pctxt, &tmpval, 0, OSUINTCONST(6));
         if (stat != 0) return stat;
         *pvalue = (OSUINT8)tmpval;
      }

      if (*pvalue > 6) {
         return RTERR_CONSVIO;
      }
   }

   return (stat);
}

EXTERN int asn1PD_ObjectCount (OSCTXT* pctxt, ObjectCount* pvalue)
{
   int stat;

   PD_BYTE_ALIGN(pctxt);
   stat = rtxDecBitsToUInt16(pctxt, pvalue, pctxt->buffer.aligned ? 16 : 10);
   if (stat != 0) return stat;

   if (*pvalue > 1023) {
      return RTERR_CONSVIO;
   }

   return stat;
}

EXTERN int asn1PD_NMEAcorrections_regional (OSCTXT* pctxt, NMEAcorrections_regional* pvalue)
{
   int stat = 0;
   OSRTDListNode* pnode;
   OSSIZE count = 0;
   OSSIZE xx1 = 0;

   /* decode length determinant */

   PU_SETSIZECONSTRAINT (pctxt, OSUINTCONST(1), OSUINTCONST(4), 0, 0);

   stat = pd_Length64 (pctxt, &count);
   if (stat != 0) return stat;

   /* decode elements */

   rtxDListInit (pvalue);

   for (xx1 = 0; xx1 < count; xx1++) {
      RegionalExtension* pdata;
      pctxt->level++;

      pdata = rtxMemAllocType (pctxt, RegionalExtension);
      if(pdata)  {
         pnode = (OSRTDListNode*) rtxMemAlloc (pctxt, sizeof(OSRTDListNode));
         if ( pnode != 0 ) pnode->data = pdata;
      }
      else pnode = 0;

      if (pnode == NULL)
         return RTERR_NOMEM;

      asn1Init_RegionalExtension ((RegionalExtension*)pdata);

      rtxDListAppendNode (pvalue, pnode);
      stat = asn1PD_RegionalExtension (pctxt, (RegionalExtension*)pdata);
      if (stat != 0) return stat;

      pctxt->level--;
   }

   return (stat);
}

EXTERN int asn1PD_NMEAcorrections (OSCTXT* pctxt, NMEAcorrections* pvalue)
{
   int stat = 0;
   ASN1OpenType openType;
   ASN1OpenType* pOpenType;
   OSUINT32 i_;
   OSBOOL extbit = FALSE;
   OSBOOL optbits[5];
   OSUINT32 j_ = 0;

   /* extension bit */

   rtxDecBit(pctxt, &extbit);

   OSCRTLMEMSET (&pvalue->m, 0, sizeof(pvalue->m));

   rtxDListInit (&pvalue->extElem1);

   /* optional bits */

   for (i_ = 0; i_ < 5; i_++) {
      stat = DEC_BIT (pctxt, &optbits[i_]);
      if (stat != 0) return stat;
   }

   /* decode root elements */
   for (i_ = 0; i_ < 6; i_++) {
      switch (i_) {
         case 0:
         /* decode timeStamp */
         if (j_ < sizeof(optbits) && optbits[j_++]) {
            pctxt->level++;

            pvalue->m.timeStampPresent = 1;

            stat = asn1PD_MinuteOfTheYear (pctxt, (MinuteOfTheYear*)&pvalue->timeStamp);

            pctxt->level--;
         }
         break;

         case 1:
         /* decode rev */
         if (j_ < sizeof(optbits) && optbits[j_++]) {
            pctxt->level++;

            pvalue->m.revPresent = 1;

            stat = asn1PD_NMEA_Revision (pctxt, &pvalue->rev);

            pctxt->level--;
         }
         break;

         case 2:
         /* decode msg */
         if (j_ < sizeof(optbits) && optbits[j_++]) {
            pctxt->level++;

            pvalue->m.msgPresent = 1;

            stat = asn1PD_NMEA_MsgType (pctxt, &pvalue->msg);

            pctxt->level--;
         }
         break;

         case 3:
         /* decode wdCount */
         if (j_ < sizeof(optbits) && optbits[j_++]) {
            pctxt->level++;

            pvalue->m.wdCountPresent = 1;

            stat = asn1PD_ObjectCount (pctxt, &pvalue->wdCount);

            pctxt->level--;
         }
         break;

         case 4:
         /* decode payload */
         pctxt->level++;

         stat = asn1PD_NMEA_Payload (pctxt, &pvalue->payload);

         pctxt->level--;
         break;

         case 5:
         /* decode regional */
         if (j_ < sizeof(optbits) && optbits[j_++]) {
            pctxt->level++;

            pvalue->m.regionalPresent = 1;

            stat = asn1PD_NMEAcorrections_regional (pctxt, &pvalue->regional);

            pctxt->level--;
         }
         break;

         default: j_++;
      }
      if (0 != stat) return stat;
   }

   /* decode extension elements */
   if (extbit) {
      OSDynOctStr extOptBits;

      /* decode extension optional bits */

      stat = pd_ExtOptElemBits(pctxt, &extOptBits);
      if (stat != 0) return stat;
      for (i_ = 0; i_ < extOptBits.numocts; i_++) {
         if (stat != 0) break;
         if (extOptBits.data[i_]) {
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

      rtxMemFreePtr (pctxt, extOptBits.data);
   }

   if (stat == 0) {
      stat = rtxErrCheckNonFatal (pctxt);
   }
   return (stat);
}

