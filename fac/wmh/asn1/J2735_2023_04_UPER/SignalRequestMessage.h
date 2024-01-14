/**
 * This file was generated by the Objective Systems ASN1C Compiler
 * (http://www.obj-sys.com).  Version: 7.7.0, Date: 25-May-2023.
 *
 * Command:  asn1c CooperativeControlMsg-v1-ph.asn J2735-AddGrpB.asn J2735-AddGrpC.asn J2735-BasicSafetyMessage-v4.asn J2735-CommonSafetyRequest.asn J2735-Common-v7.asn J2735-EmergencyVehicleAlert.asn J2735-IntersectionCollision.asn J2735-ITIS.asn J2735-MapData-v1.asn J2735-MessageFrame-v5.asn J2735-NMEAcorrections.asn J2735-NTCIP.asn J2735-PersonalSafetyMessage.asn J2735-ProbeDataManagement.asn J2735-ProbeVehicleData.asn J2735-REGION.asn J2735-RoadSideAlert.asn J2735-RTCMcorrections.asn J2735-SignalRequestMessage.asn J2735-SignalStatusMessage.asn J2735-SPAT-v2.asn J2735-TestMessage00.asn J2735-TestMessage01.asn J2735-TestMessage02.asn J2735-TestMessage03.asn J2735-TestMessage04.asn J2735-TestMessage05.asn J2735-TestMessage06.asn J2735-TestMessage07.asn J2735-TestMessage08.asn J2735-TestMessage09.asn J2735-TestMessage10.asn J2735-TestMessage11.asn J2735-TestMessage12.asn J2735-TestMessage13.asn J2735-TestMessage14.asn J2735-TestMessage15.asn J2735-TravelerInformation-v2.asn MnvrSharingCoordMsg-v1-ph.asn PersonalSafetyMsg2-v1-ph.asn ProbeDataConfig-v0.30.asn ProbeDataReport-v0.30.asn RoadGeoAttributes-v2-ph.asn RoadSafetyMsg-v1-ph.asn RoadWeatherMessage.asn SensorDataSharingMsg-v1-ph.asn SignalControlAndPrioritizationRequest-v1-ph.asn SignalControlAndPrioritizationStatus-v1-ph.asn TollAdvertisementMsg-v0.10-ph.asn TollUsageAckMsg-v0.10-ph.asn TollUsageMsg-v0.10-ph.asn TrafficSignalPhaseAndTiming-v1-ph.asn -c -per -compact
 */
#ifndef SIGNALREQUESTMESSAGE_H
#define SIGNALREQUESTMESSAGE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "rtkey.h"
#include "rtpersrc/asn1per.h"

/**
 * Header file for ASN.1 module SignalRequestMessage
 */
/**************************************************************/
/*                                                            */
/*  DeltaTime                                                 */
/*                                                            */
/**************************************************************/
/*
DeltaTime ::= INTEGER (-122..121)
    Supporting a range of +/- 20 minute in steps of 10 seconds
    the value of -121 shall be used when more than -20 minutes
    the value of +120 shall be used when more than +20 minutes
    the value -122 shall be used when the value is unavailable

*/
typedef OSINT8 DeltaTime;

EXTERN int asn1PE_DeltaTime (OSCTXT* pctxt, DeltaTime value);

EXTERN int asn1PD_DeltaTime (OSCTXT* pctxt, DeltaTime* pvalue);

#include "rtxsrc/rtxEnum.h"
/**************************************************************/
/*                                                            */
/*  PriorityRequestType                                       */
/*                                                            */
/**************************************************************/
/*
PriorityRequestType ::= ENUMERATED { priorityRequestTypeReserved(0), priorityRequest(1), priorityRequestUpdate(2), priorityCancellation(3), ... }
*/
typedef enum {
   priorityRequestTypeReserved = 0,
   priorityRequest = 1,
   priorityRequestUpdate = 2,
   priorityCancellation = 3
} PriorityRequestType_Root;

typedef OSUINT8 PriorityRequestType;

EXTERN int asn1PE_PriorityRequestType (OSCTXT* pctxt, PriorityRequestType value);

EXTERN int asn1PD_PriorityRequestType (OSCTXT* pctxt, PriorityRequestType* pvalue);

EXTERN extern const OSEnumItem PriorityRequestType_ENUMTAB[];
#define PriorityRequestType_ENUMTABSIZE 4

EXTERN const OSUTF8CHAR* PriorityRequestType_ToString (OSINT32 value);

EXTERN int PriorityRequestType_ToEnum (OSCTXT* pctxt,
   const OSUTF8CHAR* value, PriorityRequestType* pvalue);

EXTERN int PriorityRequestType_ToEnum2 (OSCTXT* pctxt,
   const OSUTF8CHAR* value, OSSIZE valueLen, PriorityRequestType* pvalue);

/**************************************************************/
/*                                                            */
/*  TransitVehicleOccupancy                                   */
/*                                                            */
/**************************************************************/
/*
TransitVehicleOccupancy ::= ENUMERATED { occupancyUnknown(0), occupancyEmpty(1), occupancyVeryLow(2), occupancyLow(3), occupancyMed(4), occupancyHigh(5), occupancyNearlyFull(6), occupancyFull(7) }
*/
typedef enum {
   occupancyUnknown = 0,
   occupancyEmpty = 1,
   occupancyVeryLow = 2,
   occupancyLow = 3,
   occupancyMed = 4,
   occupancyHigh = 5,
   occupancyNearlyFull = 6,
   occupancyFull = 7
} TransitVehicleOccupancy_Root;

typedef OSUINT8 TransitVehicleOccupancy;

EXTERN int asn1PE_TransitVehicleOccupancy (OSCTXT* pctxt, TransitVehicleOccupancy value);

EXTERN int asn1PD_TransitVehicleOccupancy (OSCTXT* pctxt, TransitVehicleOccupancy* pvalue);

EXTERN extern const OSEnumItem TransitVehicleOccupancy_ENUMTAB[];
#define TransitVehicleOccupancy_ENUMTABSIZE 8

EXTERN const OSUTF8CHAR* TransitVehicleOccupancy_ToString (OSINT32 value);

EXTERN int TransitVehicleOccupancy_ToEnum (OSCTXT* pctxt,
   const OSUTF8CHAR* value, TransitVehicleOccupancy* pvalue);

EXTERN int TransitVehicleOccupancy_ToEnum2 (OSCTXT* pctxt,
   const OSUTF8CHAR* value, OSSIZE valueLen, TransitVehicleOccupancy* pvalue);

/**************************************************************/
/*                                                            */
/*  TransitVehicleStatus                                      */
/*                                                            */
/**************************************************************/
/*
TransitVehicleStatus ::= BIT STRING  { loading(0), anADAuse(1), aBikeLoad(2), doorOpen(3), charging(4), atStopLine(5) } (SIZE (8))
*/
/* Named bit constants */

#define TransitVehicleStatus_loading                         0
#define TransitVehicleStatus_anADAuse                        1
#define TransitVehicleStatus_aBikeLoad                       2
#define TransitVehicleStatus_doorOpen                        3
#define TransitVehicleStatus_charging                        4
#define TransitVehicleStatus_atStopLine                      5

typedef struct TransitVehicleStatus {
   OSUINT32 numbits;
   OSOCTET data[1];
} TransitVehicleStatus;

EXTERN int asn1PE_TransitVehicleStatus (OSCTXT* pctxt, TransitVehicleStatus* pvalue);

EXTERN int asn1PD_TransitVehicleStatus (OSCTXT* pctxt, TransitVehicleStatus* pvalue);

EXTERN int asn1Init_TransitVehicleStatus (TransitVehicleStatus* pvalue);

/**************************************************************/
/*                                                            */
/*  SignalRequest_regional                                    */
/*                                                            */
/**************************************************************/
/*
Type was extracted from 'SignalRequest'
*/
struct EXTERN RegionalExtension;

/* List of RegionalExtension */
typedef OSRTDList SignalRequest_regional;

EXTERN int asn1PE_SignalRequest_regional (OSCTXT* pctxt, SignalRequest_regional* pvalue);

EXTERN int asn1PD_SignalRequest_regional (OSCTXT* pctxt, SignalRequest_regional* pvalue);

EXTERN int asn1Init_SignalRequest_regional (SignalRequest_regional* pvalue);

/**************************************************************/
/*                                                            */
/*  SignalRequest                                             */
/*                                                            */
/**************************************************************/
/*
SignalRequest ::= SEQUENCE {
   -- the unique ID of the target intersection
   id [0] IntersectionReferenceID,
   -- The unique requestID used by the requestor
   requestID [1] RequestID,
   -- The type of request or cancel for priority or preempt use
   -- when a prior request is canceled, only the requestID is needed
   requestType [2] PriorityRequestType,
   -- In typical use either an approach or a lane number would
   -- be given, this indicates the requested
   -- path through the intersection to the degree it is known.
   inBoundLane [3] EXPLICIT IntersectionAccessPoint,
                 -- desired entry approach or lane
   outBoundLane [4] EXPLICIT IntersectionAccessPoint OPTIONAL,
                 -- desired exit approach or lane
                 -- the values zero is used to indicate
                 -- intent to stop within the intersection
   regional [5] SEQUENCE (SIZE (1..4)) OF RegionalExtension OPTIONAL,
   ...
}
*/
typedef struct EXTERN SignalRequest {
   struct {
      OSUINT8 outBoundLanePresent : 1;
      OSUINT8 regionalPresent : 1;
   } m;
   struct IntersectionReferenceID *id;
   OSUINT8 requestID;
   PriorityRequestType requestType;
   struct IntersectionAccessPoint *inBoundLane;
   struct IntersectionAccessPoint *outBoundLane;
   SignalRequest_regional regional;
   OSRTDList extElem1;
} SignalRequest;

EXTERN int asn1PE_SignalRequest (OSCTXT* pctxt, SignalRequest* pvalue);

EXTERN int asn1PD_SignalRequest (OSCTXT* pctxt, SignalRequest* pvalue);

EXTERN int asn1Init_SignalRequest (SignalRequest* pvalue);

/**************************************************************/
/*                                                            */
/*  SignalRequestPackage_regional                             */
/*                                                            */
/**************************************************************/
/*
Type was extracted from 'SignalRequestPackage'
*/
struct EXTERN RegionalExtension;

/* List of RegionalExtension */
typedef OSRTDList SignalRequestPackage_regional;

EXTERN int asn1PE_SignalRequestPackage_regional (OSCTXT* pctxt, SignalRequestPackage_regional* pvalue);

EXTERN int asn1PD_SignalRequestPackage_regional (OSCTXT* pctxt, SignalRequestPackage_regional* pvalue);

EXTERN int asn1Init_SignalRequestPackage_regional (
   SignalRequestPackage_regional* pvalue);

/**************************************************************/
/*                                                            */
/*  SignalRequestPackage                                      */
/*                                                            */
/**************************************************************/
/*
SignalRequestPackage ::= SEQUENCE {
   request [0] SignalRequest,
                  -- The specific request to the intersection
                  -- contains IntersectionID, request type,
                  -- requested action (approach/lane request)
   -- The Estimated Time of Arrival (ETA) when the service is requested
   minute [1] MinuteOfTheYear OPTIONAL,
   second [2] DSecond OPTIONAL,
   duration [3] DSecond OPTIONAL,
                  -- The duration value is used to provide a short interval that
                  -- extends the ETA so that the requesting vehicle can arrive at
                  -- the point of service with uncertainty or with some desired
                  -- duration of service. This concept can be used to avoid needing
                  -- to frequently update the request.
                  -- The requester must update the ETA and duration values if the
                  -- period of services extends beyond the duration time.
                  -- It should be assumed that if the vehicle does not clear the
                  -- intersection when the duration is reached, the request will
                  -- be cancelled and the intersection will revert to
                  -- normal operation.
   regional [4] SEQUENCE (SIZE (1..4)) OF RegionalExtension OPTIONAL,
   ...
}
*/
typedef struct EXTERN SignalRequestPackage {
   struct {
      OSUINT8 minutePresent : 1;
      OSUINT8 secondPresent : 1;
      OSUINT8 durationPresent : 1;
      OSUINT8 regionalPresent : 1;
   } m;
   SignalRequest request;
   OSUINT32 minute;
   OSUINT16 second;
   OSUINT16 duration;
   SignalRequestPackage_regional regional;
   OSRTDList extElem1;
} SignalRequestPackage;

EXTERN int asn1PE_SignalRequestPackage (OSCTXT* pctxt, SignalRequestPackage* pvalue);

EXTERN int asn1PD_SignalRequestPackage (OSCTXT* pctxt, SignalRequestPackage* pvalue);

EXTERN int asn1Init_SignalRequestPackage (SignalRequestPackage* pvalue);

/**************************************************************/
/*                                                            */
/*  SignalRequestList                                         */
/*                                                            */
/**************************************************************/
/*
SignalRequestList ::= SEQUENCE (SIZE (1..32)) OF SignalRequestPackage
*/
/* List of SignalRequestPackage */
typedef OSRTDList SignalRequestList;

EXTERN int asn1PE_SignalRequestList (OSCTXT* pctxt, SignalRequestList* pvalue);

EXTERN int asn1PD_SignalRequestList (OSCTXT* pctxt, SignalRequestList* pvalue);

EXTERN int asn1Init_SignalRequestList (SignalRequestList* pvalue);

/**************************************************************/
/*                                                            */
/*  RequestorPositionVector                                   */
/*                                                            */
/**************************************************************/
/*
RequestorPositionVector ::= SEQUENCE {
   position [0] Position3D,
   heading [1] Angle OPTIONAL,
   speed [2] TransmissionAndSpeed OPTIONAL,
   ...
}
*/
typedef struct EXTERN RequestorPositionVector {
   struct {
      OSUINT8 headingPresent : 1;
      OSUINT8 speedPresent : 1;
   } m;
   struct Position3D *position;
   OSUINT16 heading;
   struct TransmissionAndSpeed *speed;
   OSRTDList extElem1;
} RequestorPositionVector;

EXTERN int asn1PE_RequestorPositionVector (OSCTXT* pctxt, RequestorPositionVector* pvalue);

EXTERN int asn1PD_RequestorPositionVector (OSCTXT* pctxt, RequestorPositionVector* pvalue);

EXTERN int asn1Init_RequestorPositionVector (RequestorPositionVector* pvalue);

/**************************************************************/
/*                                                            */
/*  RequestorDescription_regional                             */
/*                                                            */
/**************************************************************/
/*
Type was extracted from 'RequestorDescription'
*/
struct EXTERN RegionalExtension;

/* List of RegionalExtension */
typedef OSRTDList RequestorDescription_regional;

EXTERN int asn1PE_RequestorDescription_regional (OSCTXT* pctxt, RequestorDescription_regional* pvalue);

EXTERN int asn1PD_RequestorDescription_regional (OSCTXT* pctxt, RequestorDescription_regional* pvalue);

EXTERN int asn1Init_RequestorDescription_regional (
   RequestorDescription_regional* pvalue);

/**************************************************************/
/*                                                            */
/*  RequestorDescription                                      */
/*                                                            */
/**************************************************************/
/*
RequestorDescription ::= SEQUENCE {
   id [0] EXPLICIT VehicleID,
                     -- The ID used in the BSM or CAM of the requestor
                     -- This ID is presumed not to change
                     -- during the exchange
   type [1] RequestorType OPTIONAL,
                     -- Information regarding all type and class data
                     -- about the requesting vehicle
   position [2] RequestorPositionVector OPTIONAL,
                     -- The location of the requesting vehicle
   name [3] DescriptiveName OPTIONAL,
                     -- A human readable name for debugging use
   -- Support for Transit requests
   routeName [4] DescriptiveName OPTIONAL,
                     -- A string for transit operations use
   transitStatus [5] TransitVehicleStatus OPTIONAL,
                     -- current vehicle state (loading, etc.)
   transitOccupancy [6] TransitVehicleOccupancy OPTIONAL,
                     -- current vehicle occupancy
   transitSchedule [7] DeltaTime OPTIONAL,
                     -- current vehicle schedule adherence
   regional [8] SEQUENCE (SIZE (1..4)) OF RegionalExtension OPTIONAL,
   ...
}
*/
typedef struct EXTERN RequestorDescription {
   struct {
      OSUINT8 typePresent : 1;
      OSUINT8 positionPresent : 1;
      OSUINT8 namePresent : 1;
      OSUINT8 routeNamePresent : 1;
      OSUINT8 transitStatusPresent : 1;
      OSUINT8 transitOccupancyPresent : 1;
      OSUINT8 transitSchedulePresent : 1;
      OSUINT8 regionalPresent : 1;
   } m;
   struct VehicleID *id;
   struct RequestorType *type;
   RequestorPositionVector position;
   const char* name;
   const char* routeName;
   TransitVehicleStatus transitStatus;
   TransitVehicleOccupancy transitOccupancy;
   DeltaTime transitSchedule;
   RequestorDescription_regional regional;
   OSRTDList extElem1;
} RequestorDescription;

EXTERN int asn1PE_RequestorDescription (OSCTXT* pctxt, RequestorDescription* pvalue);

EXTERN int asn1PD_RequestorDescription (OSCTXT* pctxt, RequestorDescription* pvalue);

EXTERN int asn1Init_RequestorDescription (RequestorDescription* pvalue);

/**************************************************************/
/*                                                            */
/*  SignalRequestMessage_regional                             */
/*                                                            */
/**************************************************************/
/*
Type was extracted from 'SignalRequestMessage'
*/
struct EXTERN RegionalExtension;

/* List of RegionalExtension */
typedef OSRTDList SignalRequestMessage_regional;

EXTERN int asn1PE_SignalRequestMessage_regional (OSCTXT* pctxt, SignalRequestMessage_regional* pvalue);

EXTERN int asn1PD_SignalRequestMessage_regional (OSCTXT* pctxt, SignalRequestMessage_regional* pvalue);

EXTERN int asn1Init_SignalRequestMessage_regional (
   SignalRequestMessage_regional* pvalue);

/**************************************************************/
/*                                                            */
/*  SignalRequestMessage                                      */
/*                                                            */
/**************************************************************/
/*
SignalRequestMessage ::= SEQUENCE {
   timeStamp [0] MinuteOfTheYear OPTIONAL,
   second [1] DSecond,
   sequenceNumber [2] MsgCount OPTIONAL,
   requests [3] SignalRequestList OPTIONAL,
                   -- Request Data for one or more signalized
                   -- intersections that support SRM dialogs
   requestor [4] RequestorDescription,
                   -- Requesting Device and other User Data
                   -- contains vehicle ID (if from a vehicle)
                   -- as well as type data and current position
                   -- and may contain additional transit data
   regional [5] SEQUENCE (SIZE (1..4)) OF RegionalExtension OPTIONAL,
   ...
}
*/
typedef struct EXTERN SignalRequestMessage {
   struct {
      OSUINT8 timeStampPresent : 1;
      OSUINT8 sequenceNumberPresent : 1;
      OSUINT8 requestsPresent : 1;
      OSUINT8 regionalPresent : 1;
   } m;
   OSUINT32 timeStamp;
   OSUINT16 second;
   OSUINT8 sequenceNumber;
   SignalRequestList requests;
   RequestorDescription requestor;
   SignalRequestMessage_regional regional;
   OSRTDList extElem1;
} SignalRequestMessage;

EXTERN int asn1PE_SignalRequestMessage (OSCTXT* pctxt, SignalRequestMessage* pvalue);

EXTERN int asn1PD_SignalRequestMessage (OSCTXT* pctxt, SignalRequestMessage* pvalue);

EXTERN int asn1Init_SignalRequestMessage (SignalRequestMessage* pvalue);

#ifdef __cplusplus
}
#endif

#endif