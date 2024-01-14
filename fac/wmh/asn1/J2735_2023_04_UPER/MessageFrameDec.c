/**
 * This file was generated by the Objective Systems ASN1C Compiler
 * (http://www.obj-sys.com).  Version: 7.7.0, Date: 25-May-2023.
 *
 * Command:  asn1c CooperativeControlMsg-v1-ph.asn J2735-AddGrpB.asn J2735-AddGrpC.asn J2735-BasicSafetyMessage-v4.asn J2735-CommonSafetyRequest.asn J2735-Common-v7.asn J2735-EmergencyVehicleAlert.asn J2735-IntersectionCollision.asn J2735-ITIS.asn J2735-MapData-v1.asn J2735-MessageFrame-v5.asn J2735-NMEAcorrections.asn J2735-NTCIP.asn J2735-PersonalSafetyMessage.asn J2735-ProbeDataManagement.asn J2735-ProbeVehicleData.asn J2735-REGION.asn J2735-RoadSideAlert.asn J2735-RTCMcorrections.asn J2735-SignalRequestMessage.asn J2735-SignalStatusMessage.asn J2735-SPAT-v2.asn J2735-TestMessage00.asn J2735-TestMessage01.asn J2735-TestMessage02.asn J2735-TestMessage03.asn J2735-TestMessage04.asn J2735-TestMessage05.asn J2735-TestMessage06.asn J2735-TestMessage07.asn J2735-TestMessage08.asn J2735-TestMessage09.asn J2735-TestMessage10.asn J2735-TestMessage11.asn J2735-TestMessage12.asn J2735-TestMessage13.asn J2735-TestMessage14.asn J2735-TestMessage15.asn J2735-TravelerInformation-v2.asn MnvrSharingCoordMsg-v1-ph.asn PersonalSafetyMsg2-v1-ph.asn ProbeDataConfig-v0.30.asn ProbeDataReport-v0.30.asn RoadGeoAttributes-v2-ph.asn RoadSafetyMsg-v1-ph.asn RoadWeatherMessage.asn SensorDataSharingMsg-v1-ph.asn SignalControlAndPrioritizationRequest-v1-ph.asn SignalControlAndPrioritizationStatus-v1-ph.asn TollAdvertisementMsg-v0.10-ph.asn TollUsageAckMsg-v0.10-ph.asn TollUsageMsg-v0.10-ph.asn TrafficSignalPhaseAndTiming-v1-ph.asn -c -per -compact
 */
#include "MessageFrame.h"
#include "BasicSafetyMessage.h"
#include "MapData.h"
#include "SPAT.h"
#include "CommonSafetyRequest.h"
#include "EmergencyVehicleAlert.h"
#include "IntersectionCollision.h"
#include "NMEAcorrections.h"
#include "ProbeDataManagement.h"
#include "ProbeVehicleData.h"
#include "RoadSideAlert.h"
#include "RTCMcorrections.h"
#include "SignalRequestMessage.h"
#include "SignalStatusMessage.h"
#include "TravelerInformation.h"
#include "PersonalSafetyMessage.h"
#include "RoadWeatherMessage.h"
#include "ProbeDataConfig.h"
#include "ProbeDataReport.h"
#include "TestMessage00.h"
#include "TestMessage01.h"
#include "TestMessage02.h"
#include "TestMessage03.h"
#include "TestMessage04.h"
#include "TestMessage05.h"
#include "TestMessage06.h"
#include "TestMessage07.h"
#include "TestMessage08.h"
#include "TestMessage09.h"
#include "TestMessage10.h"
#include "TestMessage11.h"
#include "TestMessage12.h"
#include "TestMessage13.h"
#include "TestMessage14.h"
#include "TestMessage15.h"
#include "rtxsrc/rtxCharStr.h"

EXTERN int asn1PD_DSRCmsgID (OSCTXT* pctxt, DSRCmsgID* pvalue)
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

EXTERN int asn1PD_MessageFrame (OSCTXT* pctxt, MessageFrame* pvalue)
{
   int stat = 0;
   ASN1OpenType openType;
   ASN1OpenType* pOpenType;
   OSUINT32 i_;
   OSBOOL extbit = FALSE;

   /* extension bit */

   rtxDecBit(pctxt, &extbit);

   rtxDListInit (&pvalue->extElem1);

   /* decode root elements */
   /* decode messageId */
   pctxt->level++;

   stat = asn1PD_DSRCmsgID (pctxt, &pvalue->messageId);
   if (stat != 0) return stat;

   pctxt->level--;

   /* decode value */
   pctxt->level++;

   stat = pd_OpenType (pctxt, &pvalue->value.data, &pvalue->value.numocts);
   if (stat != 0) return stat;

   pctxt->level--;

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

