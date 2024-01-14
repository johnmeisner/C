/**
 * This file was generated by the Objective Systems ASN1C Compiler
 * (http://www.obj-sys.com).  Version: 7.7.0, Date: 25-May-2023.
 *
 * Command:  asn1c CooperativeControlMsg-v1-ph.asn J2735-AddGrpB.asn J2735-AddGrpC.asn J2735-BasicSafetyMessage-v4.asn J2735-CommonSafetyRequest.asn J2735-Common-v7.asn J2735-EmergencyVehicleAlert.asn J2735-IntersectionCollision.asn J2735-ITIS.asn J2735-MapData-v1.asn J2735-MessageFrame-v5.asn J2735-NMEAcorrections.asn J2735-NTCIP.asn J2735-PersonalSafetyMessage.asn J2735-ProbeDataManagement.asn J2735-ProbeVehicleData.asn J2735-REGION.asn J2735-RoadSideAlert.asn J2735-RTCMcorrections.asn J2735-SignalRequestMessage.asn J2735-SignalStatusMessage.asn J2735-SPAT-v2.asn J2735-TestMessage00.asn J2735-TestMessage01.asn J2735-TestMessage02.asn J2735-TestMessage03.asn J2735-TestMessage04.asn J2735-TestMessage05.asn J2735-TestMessage06.asn J2735-TestMessage07.asn J2735-TestMessage08.asn J2735-TestMessage09.asn J2735-TestMessage10.asn J2735-TestMessage11.asn J2735-TestMessage12.asn J2735-TestMessage13.asn J2735-TestMessage14.asn J2735-TestMessage15.asn J2735-TravelerInformation-v2.asn MnvrSharingCoordMsg-v1-ph.asn PersonalSafetyMsg2-v1-ph.asn ProbeDataConfig-v0.30.asn ProbeDataReport-v0.30.asn RoadGeoAttributes-v2-ph.asn RoadSafetyMsg-v1-ph.asn RoadWeatherMessage.asn SensorDataSharingMsg-v1-ph.asn SignalControlAndPrioritizationRequest-v1-ph.asn SignalControlAndPrioritizationStatus-v1-ph.asn TollAdvertisementMsg-v0.10-ph.asn TollUsageAckMsg-v0.10-ph.asn TollUsageMsg-v0.10-ph.asn TrafficSignalPhaseAndTiming-v1-ph.asn -c -per -compact
 */
#include "ProbeVehicleData.h"
#include "REGION.h"
#include "Common.h"
#include "rtxsrc/rtxCharStr.h"

EXTERN int asn1PE_AccelerationConfidence (OSCTXT* pctxt, AccelerationConfidence value)
{
   int stat = 0;

   if (value >= 8) {
      return RTERR_INVENUM;
   }
   stat = rtxEncBits(pctxt, value, 3);
   if (stat != 0) return stat;

   return (stat);
}

EXTERN int asn1PE_BrakeAppliedPressure (OSCTXT* pctxt, BrakeAppliedPressure value)
{
   int stat = 0;

   if (value >= 16) {
      return RTERR_INVENUM;
   }
   stat = rtxEncBits(pctxt, value, 4);
   if (stat != 0) return stat;

   return (stat);
}

EXTERN int asn1PE_DrivingWheelAngle (OSCTXT* pctxt, DrivingWheelAngle value)
{
   int stat = 0;

   stat = pe_ConsInteger(pctxt, value,  -128,  127);
   if (stat != 0) return stat;

   return (stat);
}

EXTERN int asn1PE_AxleLocation (OSCTXT* pctxt, AxleLocation value)
{
   int stat = 0;

   stat = pe_ConsUnsigned(pctxt, value,  0,  255);
   if (stat != 0) return stat;

   return (stat);
}

EXTERN int asn1PE_AxleWeight (OSCTXT* pctxt, AxleWeight value)
{
   int stat = 0;

   stat = pe_ConsUnsigned(pctxt, value,  0,  64255);
   if (stat != 0) return stat;

   return (stat);
}

EXTERN int asn1PE_CargoWeight (OSCTXT* pctxt, CargoWeight value)
{
   int stat = 0;

   stat = pe_ConsUnsigned(pctxt, value,  0,  64255);
   if (stat != 0) return stat;

   return (stat);
}

EXTERN int asn1PE_DriveAxleLiftAirPressure (OSCTXT* pctxt, DriveAxleLiftAirPressure value)
{
   int stat = 0;

   stat = pe_ConsUnsigned(pctxt, value,  0,  1000);
   if (stat != 0) return stat;

   return (stat);
}

EXTERN int asn1PE_DriveAxleLocation (OSCTXT* pctxt, DriveAxleLocation value)
{
   int stat = 0;

   stat = pe_ConsUnsigned(pctxt, value,  0,  255);
   if (stat != 0) return stat;

   return (stat);
}

EXTERN int asn1PE_DriveAxleLubePressure (OSCTXT* pctxt, DriveAxleLubePressure value)
{
   int stat = 0;

   stat = pe_ConsUnsigned(pctxt, value,  0,  250);
   if (stat != 0) return stat;

   return (stat);
}

EXTERN int asn1PE_DriveAxleTemperature (OSCTXT* pctxt, DriveAxleTemperature value)
{
   int stat = 0;

   stat = pe_ConsInteger(pctxt, value,  -40,  210);
   if (stat != 0) return stat;

   return (stat);
}

EXTERN int asn1PE_SteeringAxleLubePressure (OSCTXT* pctxt, SteeringAxleLubePressure value)
{
   int stat = 0;

   stat = pe_ConsUnsigned(pctxt, value,  0,  250);
   if (stat != 0) return stat;

   return (stat);
}

EXTERN int asn1PE_SteeringAxleTemperature (OSCTXT* pctxt, SteeringAxleTemperature value)
{
   int stat = 0;

   stat = pe_ConsInteger(pctxt, value,  -40,  210);
   if (stat != 0) return stat;

   return (stat);
}

EXTERN int asn1PE_TireLeakageRate (OSCTXT* pctxt, TireLeakageRate value)
{
   int stat = 0;

   stat = pe_ConsUnsigned(pctxt, value,  0,  64255);
   if (stat != 0) return stat;

   return (stat);
}

EXTERN int asn1PE_TireLocation (OSCTXT* pctxt, TireLocation value)
{
   int stat = 0;

   stat = pe_ConsUnsigned(pctxt, value,  0,  255);
   if (stat != 0) return stat;

   return (stat);
}

EXTERN int asn1PE_TirePressureThresholdDetection (OSCTXT* pctxt, TirePressureThresholdDetection value)
{
   int stat = 0;

   if (value >= 8) {
      return RTERR_INVENUM;
   }
   stat = rtxEncBits(pctxt, value, 3);
   if (stat != 0) return stat;

   return (stat);
}

EXTERN int asn1PE_TirePressure (OSCTXT* pctxt, TirePressure value)
{
   int stat = 0;

   stat = pe_ConsUnsigned(pctxt, value,  0,  250);
   if (stat != 0) return stat;

   return (stat);
}

EXTERN int asn1PE_TireTemp (OSCTXT* pctxt, TireTemp value)
{
   int stat = 0;

   stat = pe_ConsInteger(pctxt, value,  -8736,  55519);
   if (stat != 0) return stat;

   return (stat);
}

EXTERN int asn1PE_WheelEndElectFault (OSCTXT* pctxt, WheelEndElectFault value)
{
   int stat = 0;

   if (value >= 4) {
      return RTERR_INVENUM;
   }
   stat = rtxEncBits(pctxt, value, 2);
   if (stat != 0) return stat;

   return (stat);
}

EXTERN int asn1PE_WheelSensorStatus (OSCTXT* pctxt, WheelSensorStatus value)
{
   int stat = 0;

   if (value >= 4) {
      return RTERR_INVENUM;
   }
   stat = rtxEncBits(pctxt, value, 2);
   if (stat != 0) return stat;

   return (stat);
}

EXTERN int asn1PE_ProbeSegmentNumber (OSCTXT* pctxt, ProbeSegmentNumber value)
{
   int stat = 0;

   stat = pe_ConsUnsigned(pctxt, value,  0,  32767);
   if (stat != 0) return stat;

   return (stat);
}

EXTERN int asn1PE_RainSensor (OSCTXT* pctxt, RainSensor value)
{
   int stat = 0;

   if (value >= 8) {
      return RTERR_INVENUM;
   }
   stat = rtxEncBits(pctxt, value, 3);
   if (stat != 0) return stat;

   return (stat);
}

EXTERN int asn1PE_SteeringWheelAngleConfidence (OSCTXT* pctxt, SteeringWheelAngleConfidence value)
{
   int stat = 0;

   if (value >= 4) {
      return RTERR_INVENUM;
   }
   stat = rtxEncBits(pctxt, value, 2);
   if (stat != 0) return stat;

   return (stat);
}

EXTERN int asn1PE_SteeringWheelAngleRateOfChange (OSCTXT* pctxt, SteeringWheelAngleRateOfChange value)
{
   int stat = 0;

   stat = pe_ConsInteger(pctxt, value,  -127,  127);
   if (stat != 0) return stat;

   return (stat);
}

EXTERN int asn1PE_SunSensor (OSCTXT* pctxt, SunSensor value)
{
   int stat = 0;

   stat = pe_ConsUnsigned(pctxt, value,  0,  1000);
   if (stat != 0) return stat;

   return (stat);
}

EXTERN int asn1PE_ThrottlePosition (OSCTXT* pctxt, ThrottlePosition value)
{
   int stat = 0;

   stat = pe_ConsUnsigned(pctxt, value,  0,  200);
   if (stat != 0) return stat;

   return (stat);
}

EXTERN int asn1PE_VINstring (OSCTXT* pctxt, VINstring* pvalue)
{
   int stat = 0;

   PU_SETSIZECONSTRAINT (pctxt, OSUINTCONST(1), OSUINTCONST(17), 0, 0);

   stat = pe_OctetString (pctxt, pvalue->numocts, pvalue->data);
   if (stat != 0) return stat;

   return (stat);
}

EXTERN int asn1PE_YawRateConfidence (OSCTXT* pctxt, YawRateConfidence value)
{
   int stat = 0;

   if (value >= 8) {
      return RTERR_INVENUM;
   }
   stat = rtxEncBits(pctxt, value, 3);
   if (stat != 0) return stat;

   return (stat);
}

EXTERN int asn1PE_VehicleIdent_vehicleClass (OSCTXT* pctxt, VehicleIdent_vehicleClass* pvalue)
{
   int stat = 0;

   /* Encode choice index value */

   stat = rtxEncBits (pctxt, pvalue->t - 1, 2);
   if (stat != 0) return stat;

   /* Encode root element data value */

   switch (pvalue->t)
   {
      /* vGroup */
      case 1:
         stat = asn1PE_VehicleGroupAffected (pctxt, pvalue->u.vGroup);
         if (stat != 0) return stat;
         break;

      /* rGroup */
      case 2:
         stat = asn1PE_ResponderGroupAffected (pctxt, pvalue->u.rGroup);
         if (stat != 0) return stat;
         break;

      /* rEquip */
      case 3:
         stat = asn1PE_IncidentResponseEquipment (pctxt, pvalue->u.rEquip);
         if (stat != 0) return stat;
         break;

      default:
         return RTERR_INVOPT;
   }

   return (stat);
}

EXTERN int asn1PE_VehicleIdent (OSCTXT* pctxt, VehicleIdent* pvalue)
{
   int stat = 0;
   OSBOOL extbit = FALSE;

   extbit = (OSBOOL)(pvalue->extElem1.count > 0);

   /* extension bit */

   rtxEncBit (pctxt, extbit);

   rtxEncBit (pctxt, (OSBOOL)pvalue->m.namePresent);

   rtxEncBit (pctxt, (OSBOOL)pvalue->m.vinPresent);

   rtxEncBit (pctxt, (OSBOOL)pvalue->m.ownerCodePresent);

   rtxEncBit (pctxt, (OSBOOL)pvalue->m.idPresent);

   rtxEncBit (pctxt, (OSBOOL)pvalue->m.vehicleTypePresent);

   rtxEncBit (pctxt, (OSBOOL)pvalue->m.vehicleClassPresent);

   /* encode name */

   if (pvalue->m.namePresent) {
      stat = asn1PE_DescriptiveName (pctxt, pvalue->name);
      if (stat != 0) return stat;
   }

   /* encode vin */

   if (pvalue->m.vinPresent) {
      stat = asn1PE_VINstring (pctxt, &pvalue->vin);
      if (stat != 0) return stat;
   }

   /* encode ownerCode */

   if (pvalue->m.ownerCodePresent) {
      PU_SETSIZECONSTRAINT (pctxt, OSUINTCONST(1), OSUINTCONST(32), 0, 0);

      stat = pe_ConstrainedStringEx (pctxt, pvalue->ownerCode, 0, 8, 7, 7);
      if (stat != 0) return stat;
   }

   /* encode id */

   if (pvalue->m.idPresent) {
      stat = asn1PE_VehicleID (pctxt, (VehicleID*)pvalue->id);
      if (stat != 0) return stat;
   }

   /* encode vehicleType */

   if (pvalue->m.vehicleTypePresent) {
      stat = asn1PE_VehicleType (pctxt, pvalue->vehicleType);
      if (stat != 0) return stat;
   }

   /* encode vehicleClass */

   if (pvalue->m.vehicleClassPresent) {
      stat = asn1PE_VehicleIdent_vehicleClass (pctxt, &pvalue->vehicleClass);
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

EXTERN int asn1PE_VehicleStatus_steering (OSCTXT* pctxt, VehicleStatus_steering* pvalue)
{
   int stat = 0;

   rtxEncBit (pctxt, (OSBOOL)pvalue->m.confidencePresent);

   rtxEncBit (pctxt, (OSBOOL)pvalue->m.ratePresent);

   rtxEncBit (pctxt, (OSBOOL)pvalue->m.wheelsPresent);

   /* encode angle */

   stat = asn1PE_SteeringWheelAngle (pctxt, pvalue->angle);
   if (stat != 0) return stat;

   /* encode confidence */

   if (pvalue->m.confidencePresent) {
      stat = asn1PE_SteeringWheelAngleConfidence (pctxt, pvalue->confidence);
      if (stat != 0) return stat;
   }

   /* encode rate */

   if (pvalue->m.ratePresent) {
      stat = asn1PE_SteeringWheelAngleRateOfChange (pctxt, pvalue->rate);
      if (stat != 0) return stat;
   }

   /* encode wheels */

   if (pvalue->m.wheelsPresent) {
      stat = asn1PE_DrivingWheelAngle (pctxt, pvalue->wheels);
      if (stat != 0) return stat;
   }

   return (stat);
}

EXTERN int asn1PE_AccelSteerYawRateConfidence (OSCTXT* pctxt, AccelSteerYawRateConfidence* pvalue)
{
   int stat = 0;

   /* encode yawRate */

   stat = asn1PE_YawRateConfidence (pctxt, pvalue->yawRate);
   if (stat != 0) return stat;

   /* encode acceleration */

   stat = asn1PE_AccelerationConfidence (pctxt, pvalue->acceleration);
   if (stat != 0) return stat;

   /* encode steeringWheelAngle */

   stat = asn1PE_SteeringWheelAngleConfidence (pctxt, pvalue->steeringWheelAngle);
   if (stat != 0) return stat;

   return (stat);
}

EXTERN int asn1PE_ConfidenceSet (OSCTXT* pctxt, ConfidenceSet* pvalue)
{
   int stat = 0;
   OSBOOL extbit = FALSE;

   extbit = (OSBOOL)(pvalue->extElem1.count > 0);

   /* extension bit */

   rtxEncBit (pctxt, extbit);

   rtxEncBit (pctxt, (OSBOOL)pvalue->m.accelConfidencePresent);

   rtxEncBit (pctxt, (OSBOOL)pvalue->m.speedConfidencePresent);

   rtxEncBit (pctxt, (OSBOOL)pvalue->m.timeConfidencePresent);

   rtxEncBit (pctxt, (OSBOOL)pvalue->m.posConfidencePresent);

   rtxEncBit (pctxt, (OSBOOL)pvalue->m.steerConfidencePresent);

   rtxEncBit (pctxt, (OSBOOL)pvalue->m.headingConfidencePresent);

   rtxEncBit (pctxt, (OSBOOL)pvalue->m.throttleConfidencePresent);

   /* encode accelConfidence */

   if (pvalue->m.accelConfidencePresent) {
      stat = asn1PE_AccelSteerYawRateConfidence (pctxt, &pvalue->accelConfidence);
      if (stat != 0) return stat;
   }

   /* encode speedConfidence */

   if (pvalue->m.speedConfidencePresent) {
      stat = asn1PE_SpeedandHeadingandThrottleConfidence (pctxt, (SpeedandHeadingandThrottleConfidence*)pvalue->speedConfidence);
      if (stat != 0) return stat;
   }

   /* encode timeConfidence */

   if (pvalue->m.timeConfidencePresent) {
      stat = asn1PE_TimeConfidence (pctxt, pvalue->timeConfidence);
      if (stat != 0) return stat;
   }

   /* encode posConfidence */

   if (pvalue->m.posConfidencePresent) {
      stat = asn1PE_PositionConfidenceSet (pctxt, (PositionConfidenceSet*)pvalue->posConfidence);
      if (stat != 0) return stat;
   }

   /* encode steerConfidence */

   if (pvalue->m.steerConfidencePresent) {
      stat = asn1PE_SteeringWheelAngleConfidence (pctxt, pvalue->steerConfidence);
      if (stat != 0) return stat;
   }

   /* encode headingConfidence */

   if (pvalue->m.headingConfidencePresent) {
      stat = asn1PE_HeadingConfidence (pctxt, pvalue->headingConfidence);
      if (stat != 0) return stat;
   }

   /* encode throttleConfidence */

   if (pvalue->m.throttleConfidencePresent) {
      stat = asn1PE_ThrottleConfidence (pctxt, pvalue->throttleConfidence);
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

EXTERN int asn1PE_VehicleStatus_accelSets (OSCTXT* pctxt, VehicleStatus_accelSets* pvalue)
{
   int stat = 0;

   rtxEncBit (pctxt, (OSBOOL)pvalue->m.accel4wayPresent);

   rtxEncBit (pctxt, (OSBOOL)pvalue->m.vertAccelThresPresent);

   rtxEncBit (pctxt, (OSBOOL)pvalue->m.yawRateConPresent);

   rtxEncBit (pctxt, (OSBOOL)pvalue->m.hozAccelConPresent);

   rtxEncBit (pctxt, (OSBOOL)pvalue->m.confidenceSetPresent);

   /* encode accel4way */

   if (pvalue->m.accel4wayPresent) {
      stat = asn1PE_AccelerationSet4Way (pctxt, (AccelerationSet4Way*)pvalue->accel4way);
      if (stat != 0) return stat;
   }

   /* encode vertAccelThres */

   if (pvalue->m.vertAccelThresPresent) {
      stat = asn1PE_VerticalAccelerationThreshold (pctxt, (VerticalAccelerationThreshold*)pvalue->vertAccelThres);
      if (stat != 0) return stat;
   }

   /* encode yawRateCon */

   if (pvalue->m.yawRateConPresent) {
      stat = asn1PE_YawRateConfidence (pctxt, pvalue->yawRateCon);
      if (stat != 0) return stat;
   }

   /* encode hozAccelCon */

   if (pvalue->m.hozAccelConPresent) {
      stat = asn1PE_AccelerationConfidence (pctxt, pvalue->hozAccelCon);
      if (stat != 0) return stat;
   }

   /* encode confidenceSet */

   if (pvalue->m.confidenceSetPresent) {
      stat = asn1PE_ConfidenceSet (pctxt, &pvalue->confidenceSet);
      if (stat != 0) return stat;
   }

   return (stat);
}

EXTERN int asn1PE_VehicleStatus_object (OSCTXT* pctxt, VehicleStatus_object* pvalue)
{
   int stat = 0;

   /* encode obDist */

   stat = asn1PE_ObstacleDistance (pctxt, pvalue->obDist);
   if (stat != 0) return stat;

   /* encode obDirect */

   stat = asn1PE_Common_Angle (pctxt, pvalue->obDirect);
   if (stat != 0) return stat;

   /* encode dateTime */

   stat = asn1PE_DDateTime (pctxt, (DDateTime*)pvalue->dateTime);
   if (stat != 0) return stat;

   return (stat);
}

EXTERN int asn1PE_VehicleStatus_vehicleData (OSCTXT* pctxt, VehicleStatus_vehicleData* pvalue)
{
   int stat = 0;

   /* encode height */

   stat = asn1PE_VehicleHeight (pctxt, pvalue->height);
   if (stat != 0) return stat;

   /* encode bumpers */

   stat = asn1PE_BumperHeights (pctxt, (BumperHeights*)pvalue->bumpers);
   if (stat != 0) return stat;

   /* encode mass */

   stat = asn1PE_VehicleMass (pctxt, pvalue->mass);
   if (stat != 0) return stat;

   /* encode trailerWeight */

   stat = asn1PE_TrailerWeight (pctxt, pvalue->trailerWeight);
   if (stat != 0) return stat;

   /* encode type */

   stat = asn1PE_VehicleType (pctxt, pvalue->type);
   if (stat != 0) return stat;

   return (stat);
}

EXTERN int asn1PE_TireData (OSCTXT* pctxt, TireData* pvalue)
{
   int stat = 0;
   OSBOOL extbit = FALSE;

   extbit = (OSBOOL)(pvalue->extElem1.count > 0);

   /* extension bit */

   rtxEncBit (pctxt, extbit);

   rtxEncBit (pctxt, (OSBOOL)pvalue->m.locationPresent);

   rtxEncBit (pctxt, (OSBOOL)pvalue->m.pressurePresent);

   rtxEncBit (pctxt, (OSBOOL)pvalue->m.tempPresent);

   rtxEncBit (pctxt, (OSBOOL)pvalue->m.wheelSensorStatusPresent);

   rtxEncBit (pctxt, (OSBOOL)pvalue->m.wheelEndElectFaultPresent);

   rtxEncBit (pctxt, (OSBOOL)pvalue->m.leakageRatePresent);

   rtxEncBit (pctxt, (OSBOOL)pvalue->m.detectionPresent);

   /* encode location */

   if (pvalue->m.locationPresent) {
      stat = asn1PE_TireLocation (pctxt, pvalue->location);
      if (stat != 0) return stat;
   }

   /* encode pressure */

   if (pvalue->m.pressurePresent) {
      stat = asn1PE_TirePressure (pctxt, pvalue->pressure);
      if (stat != 0) return stat;
   }

   /* encode temp */

   if (pvalue->m.tempPresent) {
      stat = asn1PE_TireTemp (pctxt, pvalue->temp);
      if (stat != 0) return stat;
   }

   /* encode wheelSensorStatus */

   if (pvalue->m.wheelSensorStatusPresent) {
      stat = asn1PE_WheelSensorStatus (pctxt, pvalue->wheelSensorStatus);
      if (stat != 0) return stat;
   }

   /* encode wheelEndElectFault */

   if (pvalue->m.wheelEndElectFaultPresent) {
      stat = asn1PE_WheelEndElectFault (pctxt, pvalue->wheelEndElectFault);
      if (stat != 0) return stat;
   }

   /* encode leakageRate */

   if (pvalue->m.leakageRatePresent) {
      stat = asn1PE_TireLeakageRate (pctxt, pvalue->leakageRate);
      if (stat != 0) return stat;
   }

   /* encode detection */

   if (pvalue->m.detectionPresent) {
      stat = asn1PE_TirePressureThresholdDetection (pctxt, pvalue->detection);
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

EXTERN int asn1PE_TireDataList (OSCTXT* pctxt, TireDataList* pvalue)
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
      stat = asn1PE_TireData (pctxt, ((TireData*)pnode->data));
      if (stat != 0) return stat;

      xx1++;
   }

   return (stat);
}

EXTERN int asn1PE_AxleWeightSet (OSCTXT* pctxt, AxleWeightSet* pvalue)
{
   int stat = 0;
   OSBOOL extbit = FALSE;

   extbit = (OSBOOL)(pvalue->extElem1.count > 0);

   /* extension bit */

   rtxEncBit (pctxt, extbit);

   rtxEncBit (pctxt, (OSBOOL)pvalue->m.locationPresent);

   rtxEncBit (pctxt, (OSBOOL)pvalue->m.weightPresent);

   /* encode location */

   if (pvalue->m.locationPresent) {
      stat = asn1PE_AxleLocation (pctxt, pvalue->location);
      if (stat != 0) return stat;
   }

   /* encode weight */

   if (pvalue->m.weightPresent) {
      stat = asn1PE_AxleWeight (pctxt, pvalue->weight);
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

EXTERN int asn1PE_AxleWeightList (OSCTXT* pctxt, AxleWeightList* pvalue)
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
      stat = asn1PE_AxleWeightSet (pctxt, ((AxleWeightSet*)pnode->data));
      if (stat != 0) return stat;

      xx1++;
   }

   return (stat);
}

EXTERN int asn1PE_J1939data (OSCTXT* pctxt, J1939data* pvalue)
{
   int stat = 0;
   OSBOOL extbit = FALSE;

   extbit = (OSBOOL)(pvalue->extElem1.count > 0);

   /* extension bit */

   rtxEncBit (pctxt, extbit);

   rtxEncBit (pctxt, (OSBOOL)pvalue->m.tiresPresent);

   rtxEncBit (pctxt, (OSBOOL)pvalue->m.axlesPresent);

   rtxEncBit (pctxt, (OSBOOL)pvalue->m.trailerWeightPresent);

   rtxEncBit (pctxt, (OSBOOL)pvalue->m.cargoWeightPresent);

   rtxEncBit (pctxt, (OSBOOL)pvalue->m.steeringAxleTemperaturePresent);

   rtxEncBit (pctxt, (OSBOOL)pvalue->m.driveAxleLocationPresent);

   rtxEncBit (pctxt, (OSBOOL)pvalue->m.driveAxleLiftAirPressurePresent);

   rtxEncBit (pctxt, (OSBOOL)pvalue->m.driveAxleTemperaturePresent);

   rtxEncBit (pctxt, (OSBOOL)pvalue->m.driveAxleLubePressurePresent);

   rtxEncBit (pctxt, (OSBOOL)pvalue->m.steeringAxleLubePressurePresent);

   /* encode tires */

   if (pvalue->m.tiresPresent) {
      stat = asn1PE_TireDataList (pctxt, &pvalue->tires);
      if (stat != 0) return stat;
   }

   /* encode axles */

   if (pvalue->m.axlesPresent) {
      stat = asn1PE_AxleWeightList (pctxt, &pvalue->axles);
      if (stat != 0) return stat;
   }

   /* encode trailerWeight */

   if (pvalue->m.trailerWeightPresent) {
      stat = asn1PE_TrailerWeight (pctxt, pvalue->trailerWeight);
      if (stat != 0) return stat;
   }

   /* encode cargoWeight */

   if (pvalue->m.cargoWeightPresent) {
      stat = asn1PE_CargoWeight (pctxt, pvalue->cargoWeight);
      if (stat != 0) return stat;
   }

   /* encode steeringAxleTemperature */

   if (pvalue->m.steeringAxleTemperaturePresent) {
      stat = asn1PE_SteeringAxleTemperature (pctxt, pvalue->steeringAxleTemperature);
      if (stat != 0) return stat;
   }

   /* encode driveAxleLocation */

   if (pvalue->m.driveAxleLocationPresent) {
      stat = asn1PE_DriveAxleLocation (pctxt, pvalue->driveAxleLocation);
      if (stat != 0) return stat;
   }

   /* encode driveAxleLiftAirPressure */

   if (pvalue->m.driveAxleLiftAirPressurePresent) {
      stat = asn1PE_DriveAxleLiftAirPressure (pctxt, pvalue->driveAxleLiftAirPressure);
      if (stat != 0) return stat;
   }

   /* encode driveAxleTemperature */

   if (pvalue->m.driveAxleTemperaturePresent) {
      stat = asn1PE_DriveAxleTemperature (pctxt, pvalue->driveAxleTemperature);
      if (stat != 0) return stat;
   }

   /* encode driveAxleLubePressure */

   if (pvalue->m.driveAxleLubePressurePresent) {
      stat = asn1PE_DriveAxleLubePressure (pctxt, pvalue->driveAxleLubePressure);
      if (stat != 0) return stat;
   }

   /* encode steeringAxleLubePressure */

   if (pvalue->m.steeringAxleLubePressurePresent) {
      stat = asn1PE_SteeringAxleLubePressure (pctxt, pvalue->steeringAxleLubePressure);
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

EXTERN int asn1PE_VehicleStatus_weatherReport (OSCTXT* pctxt, VehicleStatus_weatherReport* pvalue)
{
   int stat = 0;

   rtxEncBit (pctxt, (OSBOOL)pvalue->m.rainRatePresent);

   rtxEncBit (pctxt, (OSBOOL)pvalue->m.precipSituationPresent);

   rtxEncBit (pctxt, (OSBOOL)pvalue->m.solarRadiationPresent);

   rtxEncBit (pctxt, (OSBOOL)pvalue->m.frictionPresent);

   /* encode isRaining */

   stat = asn1PE_EssPrecipYesNo (pctxt, pvalue->isRaining);
   if (stat != 0) return stat;

   /* encode rainRate */

   if (pvalue->m.rainRatePresent) {
      stat = asn1PE_EssPrecipRate (pctxt, pvalue->rainRate);
      if (stat != 0) return stat;
   }

   /* encode precipSituation */

   if (pvalue->m.precipSituationPresent) {
      stat = asn1PE_EssPrecipSituation (pctxt, pvalue->precipSituation);
      if (stat != 0) return stat;
   }

   /* encode solarRadiation */

   if (pvalue->m.solarRadiationPresent) {
      stat = asn1PE_EssSolarRadiation (pctxt, pvalue->solarRadiation);
      if (stat != 0) return stat;
   }

   /* encode friction */

   if (pvalue->m.frictionPresent) {
      stat = asn1PE_EssMobileFriction (pctxt, pvalue->friction);
      if (stat != 0) return stat;
   }

   return (stat);
}

EXTERN int asn1PE_VehicleStatus (OSCTXT* pctxt, VehicleStatus* pvalue)
{
   int stat = 0;
   OSBOOL extbit = FALSE;

   extbit = (OSBOOL)(pvalue->extElem1.count > 0);

   /* extension bit */

   rtxEncBit (pctxt, extbit);

   rtxEncBit (pctxt, (OSBOOL)pvalue->m.lightsPresent);

   rtxEncBit (pctxt, (OSBOOL)pvalue->m.lightBarPresent);

   rtxEncBit (pctxt, (OSBOOL)pvalue->m.wipersPresent);

   rtxEncBit (pctxt, (OSBOOL)pvalue->m.brakeStatusPresent);

   rtxEncBit (pctxt, (OSBOOL)pvalue->m.brakePressurePresent);

   rtxEncBit (pctxt, (OSBOOL)pvalue->m.roadFrictionPresent);

   rtxEncBit (pctxt, (OSBOOL)pvalue->m.sunDataPresent);

   rtxEncBit (pctxt, (OSBOOL)pvalue->m.rainDataPresent);

   rtxEncBit (pctxt, (OSBOOL)pvalue->m.airTempPresent);

   rtxEncBit (pctxt, (OSBOOL)pvalue->m.airPresPresent);

   rtxEncBit (pctxt, (OSBOOL)pvalue->m.steeringPresent);

   rtxEncBit (pctxt, (OSBOOL)pvalue->m.accelSetsPresent);

   rtxEncBit (pctxt, (OSBOOL)pvalue->m.objectPresent);

   rtxEncBit (pctxt, (OSBOOL)pvalue->m.fullPosPresent);

   rtxEncBit (pctxt, (OSBOOL)pvalue->m.throttlePosPresent);

   rtxEncBit (pctxt, (OSBOOL)pvalue->m.speedHeadCPresent);

   rtxEncBit (pctxt, (OSBOOL)pvalue->m.speedCPresent);

   rtxEncBit (pctxt, (OSBOOL)pvalue->m.vehicleDataPresent);

   rtxEncBit (pctxt, (OSBOOL)pvalue->m.vehicleIdentPresent);

   rtxEncBit (pctxt, (OSBOOL)pvalue->m.j1939dataPresent);

   rtxEncBit (pctxt, (OSBOOL)pvalue->m.weatherReportPresent);

   rtxEncBit (pctxt, (OSBOOL)pvalue->m.gnssStatusPresent);

   /* encode lights */

   if (pvalue->m.lightsPresent) {
      stat = asn1PE_ExteriorLights (pctxt, (ExteriorLights*)pvalue->lights);
      if (stat != 0) return stat;
   }

   /* encode lightBar */

   if (pvalue->m.lightBarPresent) {
      stat = asn1PE_LightbarInUse (pctxt, pvalue->lightBar);
      if (stat != 0) return stat;
   }

   /* encode wipers */

   if (pvalue->m.wipersPresent) {
      stat = asn1PE_WiperSet (pctxt, (WiperSet*)pvalue->wipers);
      if (stat != 0) return stat;
   }

   /* encode brakeStatus */

   if (pvalue->m.brakeStatusPresent) {
      stat = asn1PE_BrakeSystemStatus (pctxt, (BrakeSystemStatus*)pvalue->brakeStatus);
      if (stat != 0) return stat;
   }

   /* encode brakePressure */

   if (pvalue->m.brakePressurePresent) {
      stat = asn1PE_BrakeAppliedPressure (pctxt, pvalue->brakePressure);
      if (stat != 0) return stat;
   }

   /* encode roadFriction */

   if (pvalue->m.roadFrictionPresent) {
      stat = asn1PE_CoefficientOfFriction (pctxt, pvalue->roadFriction);
      if (stat != 0) return stat;
   }

   /* encode sunData */

   if (pvalue->m.sunDataPresent) {
      stat = asn1PE_SunSensor (pctxt, pvalue->sunData);
      if (stat != 0) return stat;
   }

   /* encode rainData */

   if (pvalue->m.rainDataPresent) {
      stat = asn1PE_RainSensor (pctxt, pvalue->rainData);
      if (stat != 0) return stat;
   }

   /* encode airTemp */

   if (pvalue->m.airTempPresent) {
      stat = asn1PE_AmbientAirTemperature (pctxt, pvalue->airTemp);
      if (stat != 0) return stat;
   }

   /* encode airPres */

   if (pvalue->m.airPresPresent) {
      stat = asn1PE_AmbientAirPressure (pctxt, pvalue->airPres);
      if (stat != 0) return stat;
   }

   /* encode steering */

   if (pvalue->m.steeringPresent) {
      stat = asn1PE_VehicleStatus_steering (pctxt, &pvalue->steering);
      if (stat != 0) return stat;
   }

   /* encode accelSets */

   if (pvalue->m.accelSetsPresent) {
      stat = asn1PE_VehicleStatus_accelSets (pctxt, &pvalue->accelSets);
      if (stat != 0) return stat;
   }

   /* encode object */

   if (pvalue->m.objectPresent) {
      stat = asn1PE_VehicleStatus_object (pctxt, &pvalue->object);
      if (stat != 0) return stat;
   }

   /* encode fullPos */

   if (pvalue->m.fullPosPresent) {
      stat = asn1PE_FullPositionVector (pctxt, (FullPositionVector*)pvalue->fullPos);
      if (stat != 0) return stat;
   }

   /* encode throttlePos */

   if (pvalue->m.throttlePosPresent) {
      stat = asn1PE_ThrottlePosition (pctxt, pvalue->throttlePos);
      if (stat != 0) return stat;
   }

   /* encode speedHeadC */

   if (pvalue->m.speedHeadCPresent) {
      stat = asn1PE_SpeedandHeadingandThrottleConfidence (pctxt, (SpeedandHeadingandThrottleConfidence*)pvalue->speedHeadC);
      if (stat != 0) return stat;
   }

   /* encode speedC */

   if (pvalue->m.speedCPresent) {
      stat = asn1PE_SpeedConfidence (pctxt, pvalue->speedC);
      if (stat != 0) return stat;
   }

   /* encode vehicleData */

   if (pvalue->m.vehicleDataPresent) {
      stat = asn1PE_VehicleStatus_vehicleData (pctxt, &pvalue->vehicleData);
      if (stat != 0) return stat;
   }

   /* encode vehicleIdent */

   if (pvalue->m.vehicleIdentPresent) {
      stat = asn1PE_VehicleIdent (pctxt, &pvalue->vehicleIdent);
      if (stat != 0) return stat;
   }

   /* encode j1939data */

   if (pvalue->m.j1939dataPresent) {
      stat = asn1PE_J1939data (pctxt, &pvalue->j1939data);
      if (stat != 0) return stat;
   }

   /* encode weatherReport */

   if (pvalue->m.weatherReportPresent) {
      stat = asn1PE_VehicleStatus_weatherReport (pctxt, &pvalue->weatherReport);
      if (stat != 0) return stat;
   }

   /* encode gnssStatus */

   if (pvalue->m.gnssStatusPresent) {
      stat = asn1PE_GNSSstatus (pctxt, (GNSSstatus*)pvalue->gnssStatus);
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

EXTERN int asn1PE_Snapshot (OSCTXT* pctxt, Snapshot* pvalue)
{
   int stat = 0;
   OSBOOL extbit = FALSE;

   extbit = (OSBOOL)(pvalue->extElem1.count > 0);

   /* extension bit */

   rtxEncBit (pctxt, extbit);

   rtxEncBit (pctxt, (OSBOOL)pvalue->m.safetyExtPresent);

   rtxEncBit (pctxt, (OSBOOL)pvalue->m.dataSetPresent);

   /* encode thePosition */

   stat = asn1PE_FullPositionVector (pctxt, (FullPositionVector*)pvalue->thePosition);
   if (stat != 0) return stat;

   /* encode safetyExt */

   if (pvalue->m.safetyExtPresent) {
      stat = asn1PE_VehicleSafetyExtensions (pctxt, (VehicleSafetyExtensions*)pvalue->safetyExt);
      if (stat != 0) return stat;
   }

   /* encode dataSet */

   if (pvalue->m.dataSetPresent) {
      stat = asn1PE_VehicleStatus (pctxt, &pvalue->dataSet);
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

EXTERN int asn1PE_ProbeVehicleData_snapshots (OSCTXT* pctxt, ProbeVehicleData_snapshots* pvalue)
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
      stat = asn1PE_Snapshot (pctxt, ((Snapshot*)pnode->data));
      if (stat != 0) return stat;

      xx1++;
   }

   return (stat);
}

EXTERN int asn1PE_ProbeVehicleData_regional (OSCTXT* pctxt, ProbeVehicleData_regional* pvalue)
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

EXTERN int asn1PE_ProbeVehicleData (OSCTXT* pctxt, ProbeVehicleData* pvalue)
{
   int stat = 0;
   OSBOOL extbit = FALSE;

   extbit = (OSBOOL)(pvalue->extElem1.count > 0);

   /* extension bit */

   rtxEncBit (pctxt, extbit);

   rtxEncBit (pctxt, (OSBOOL)pvalue->m.timeStampPresent);

   rtxEncBit (pctxt, (OSBOOL)pvalue->m.segNumPresent);

   rtxEncBit (pctxt, (OSBOOL)pvalue->m.probeIDPresent);

   rtxEncBit (pctxt, (OSBOOL)pvalue->m.regionalPresent);

   /* encode timeStamp */

   if (pvalue->m.timeStampPresent) {
      stat = asn1PE_MinuteOfTheYear (pctxt, pvalue->timeStamp);
      if (stat != 0) return stat;
   }

   /* encode segNum */

   if (pvalue->m.segNumPresent) {
      stat = asn1PE_ProbeSegmentNumber (pctxt, pvalue->segNum);
      if (stat != 0) return stat;
   }

   /* encode probeID */

   if (pvalue->m.probeIDPresent) {
      stat = asn1PE_VehicleIdent (pctxt, &pvalue->probeID);
      if (stat != 0) return stat;
   }

   /* encode startVector */

   stat = asn1PE_FullPositionVector (pctxt, (FullPositionVector*)pvalue->startVector);
   if (stat != 0) return stat;

   /* encode vehicleType */

   stat = asn1PE_VehicleClassification (pctxt, (VehicleClassification*)pvalue->vehicleType);
   if (stat != 0) return stat;

   /* encode snapshots */

   stat = asn1PE_ProbeVehicleData_snapshots (pctxt, &pvalue->snapshots);
   if (stat != 0) return stat;

   /* encode regional */

   if (pvalue->m.regionalPresent) {
      stat = asn1PE_ProbeVehicleData_regional (pctxt, &pvalue->regional);
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

