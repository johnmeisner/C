/**************************************************************************
 *                                                                        *
 *     File Name:  rsuSecurity.c                                          *
 *     Author:                                                            *
 *         DENSO International America, Inc.                              *
 *         North America Research Laboratory, California Office           *
 *         Rutherford Road                                                *
 *         Carlsbad, 92008                                                *
 **************************************************************************/
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <pthread.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <errno.h>
#include "rsutable.h"
#include "ntcip-1218.h"
#if defined(MY_UNIT_TEST)
#include "stubs.h"
#endif

/*
 * Defines.
 */

/* Mandatory: For debugging and syslog output. */
#define MY_NAME "rsuSecurity"

/* rsuSecurity.c specific: Create what you need to debug. */
#define SECURITY_INSTALL_FAIL      -1
#define SECURITY_BAD_INPUT         -2
#define SECURITY_LOCK_FAIL         -3

#define SECURITY_BAD_DATA          -5
#define SECURITY_BAD_MODE          -6
#define SECURITY_BAD_INDEX         -7
/******************************************************************************
 * 5.9.1 - 5.9.16:{ rsu 8 }: Security: rsuSecAppCertTable and rsuSecProfileTable
 *   
 * typedef struct{
 *     int32_t  rsuSecAppCertIndex;                      RO: 1..maxRsuSecAppCerts.
 *     uint8_t  rsuSecAppCertPsid[RSU_SEC_APP_PSID_MAX]; RO: 1..255: "up to 64": should be 4..252?: 64 * 4 = 256? Padded to 4 octets.
 *     int32_t  rsuSecAppCertState;                      RO: 1..4: 1 = other, 2 = valid, 3 = notValid, 4 = future: validty= time + region.
 *     int32_t  rsuSecAppCertExpiration;                 RO: 0..255: hours: 0 = expire today, 255 = expired, 1..54 = hours till expire.
 *     int32_t  rsuSecAppCertReq;                        RO: 0..655535: hours before rsuSecAppCertExpiration RSU make req new creds.
 * } RsuSecAppCertEntry_t;
 * 
 * typedef struct{
 *     int32_t rsuSecProfileIndex;                                       RO: 1 to maxRsuSecProfiles: index must be 1 or greater. 
 *     uint8_t  rsuSecProfileName[RSU_SECURITY_PROFILE_NAME_LENGTH_MAX];  RO: 1..127: name of profile.
 *     uint8_t  rsuSecProfileDesc[RSU_SECURITY_PROFILE_DESC_LENGTH_MAX];  RO: 0..255: description. 
 * } RsuSecProfileEntry_t;
 * 
 * 5.9.1
 * int32_t                  rsuSecCredReq;
 * rsuSecEnrollCertStatus_e rsuSecEnrollCertStatus;
 * int32_t                  rsuSecEnrollCertValidRegion;
 *
 * 5.9.4 Enrollment Certificate Source Domain: URI-TC-MIB: Uri255: 0..255: OCTET STRING: Normalized(see below):
 *   A Uniform Resource Identifier (URI) as defined by STD 66. Objects using this TEXTUAL-CONVENTION MUST be in US-ASCII encoding, 
 *   and MUST be normalized as described by RFC 3986 Sections 6.2.1, 6.2.2.1, and 6.2.2.2. All unnecessary percent-encoding is removed, 
 *   and all case-insensitive characters are set to lowercase except for hexadecimal digits, which are normalized to uppercase 
 *   as described in Section 6.2.2.1. The purpose of this normalization is to help provide unique URIs. Note that this normalization 
 *   is not sufficient to provide uniqueness. Two URIs that are textually distinct after this normalization may still be equivalent. 
 *   Objects using this TEXTUAL-CONVENTION MAY restrict the schemes that they permit. For example, 'data:' and 'urn:' schemes might 
 *   not be appropriate. A zero-length URI is not a valid URI. This can be used to express 'URI absent' where required, for example 
 *   when used as an index field. STD 66 URIs are of unlimited length. Objects using this TEXTUAL-CONVENTION impose a length limit 
 *   on the URIs that they can represent. Where no length restriction is required, objects SHOULD use the 'Uri' TEXTUAL-CONVENTION instead.
 *   Objects used as indices SHOULD subtype the 'Uri' TEXTUAL-CONVENTION.
 *
 * uint8_t                  rsuSecEnrollCertUrl[URI255_LENGTH_MAX];
 * uint8_t                  rsuSecEnrollCertId[RSU_SECURITY_ENROLL_CERT_ID_LENGTH_MAX];
 * uint8_t                  rsuSecEnrollCertExpiration[MIB_DATEANDTIME_LENGTH];
 * rsuSecuritySource_e      rsuSecuritySource;
 * uint8_t                  rsuSecAppCertUrl[URI1024_LENGTH_MAX];
 *
 * 5.9.9
 * int32_t                  maxRsuSecAppCerts;
 * RsuSecAppCertEntry_t     rsuSecAppCertTable[RSU_SEC_APP_CERTS_MAX];
 *
 * 5.9.11
 * uint8_t                  rsuSecCertRevocationUrl[URI255_LENGTH_MAX];
 * uint8_t                  rsuSecCertRevocationTime[MIB_DATEANDTIME_LENGTH];
 * int32_t                  rsuSecCertRevocationInterval;
 * int32_t                  rsuSecCertRevocationUpdate;
 *
 * 5.9.15
 * int32_t                  maxRsuSecProfiles;
 * RsuSecProfileEntry_t     rsuSecProfileTable[RSU_SECURITY_PROFILES_MAX];
 * 
 ******************************************************************************/

const oid rsuSecCredReq_oid[]                = {1, 3, 6, 1, 4, 1, 1206, 4, 2, 18, 8 , 1};
const oid rsuSecEnrollCertStatus_oid[]       = {1, 3, 6, 1, 4, 1, 1206, 4, 2, 18, 8 , 2};
const oid rsuSecEnrollCertValidRegion_oid[]  = {1, 3, 6, 1, 4, 1, 1206, 4, 2, 18, 8 , 3};
const oid rsuSecEnrollCertUrl_oid[]          = {1, 3, 6, 1, 4, 1, 1206, 4, 2, 18, 8 , 4};
const oid rsuSecEnrollCertId_oid[]           = {1, 3, 6, 1, 4, 1, 1206, 4, 2, 18, 8 , 5};
const oid rsuSecEnrollCertExpiration_oid[]   = {1, 3, 6, 1, 4, 1, 1206, 4, 2, 18, 8 , 6};
const oid rsuSecuritySource_oid[]            = {1, 3, 6, 1, 4, 1, 1206, 4, 2, 18, 8 , 7};
const oid rsuSecAppCertUrl_oid[]             = {1, 3, 6, 1, 4, 1, 1206, 4, 2, 18, 8 , 8};
const oid maxRsuSecAppCerts_oid[]            = {1, 3, 6, 1, 4, 1, 1206, 4, 2, 18, 8 , 9};
const oid rsuSecAppCertTable_oid[]           = {1, 3, 6, 1, 4, 1, 1206, 4, 2, 18, 8 , 10};
const oid rsuSecCertRevocationUrl_oid[]      = {1, 3, 6, 1, 4, 1, 1206, 4, 2, 18, 8 , 11};
const oid rsuSecCertRevocationTime_oid[]     = {1, 3, 6, 1, 4, 1, 1206, 4, 2, 18, 8 , 12};
const oid rsuSecCertRevocationInterval_oid[] = {1, 3, 6, 1, 4, 1, 1206, 4, 2, 18, 8 , 13};
const oid rsuSecCertRevocationUpdate_oid[]   = {1, 3, 6, 1, 4, 1, 1206, 4, 2, 18, 8 , 14};
const oid maxRsuSecProfiles_oid[]            = {1, 3, 6, 1, 4, 1, 1206, 4, 2, 18, 8 , 15};
const oid rsuSecProfileTable_oid[]           = {1, 3, 6, 1, 4, 1, 1206, 4, 2, 18, 8 , 16};

static netsnmp_table_registration_info * app_table_reg_info = NULL;    /* snmp table reg pointers. */
static netsnmp_table_registration_info * profile_table_reg_info = NULL;
static netsnmp_tdata * rsuSecAppCertTable = NULL;                      /* snmp table pointers. */
static netsnmp_tdata * rsuSecProfileTable = NULL;

static uint32_t rsuSecurity_error_states = 0x0;

void set_rsuSecurity_error_states(int32_t rsuSecurity_error)
{
  int32_t dummy = 0;

  dummy = abs(rsuSecurity_error);

  /* -1 to -31 */
  if((rsuSecurity_error < 0) && (rsuSecurity_error > MIB_ERROR_BASE_2)){
      rsuSecurity_error_states |= (uint32_t)(0x1) << (dummy - 1);
  }
}
uint32_t get_rsuSecurity_error_states(void)
{
  return rsuSecurity_error_states;
}
/* Unified way to report back to SNMP requests. For handlers only. */
static int32_t handle_rsemib_errors(int32_t input)
{
  int32_t ret = SNMP_ERR_NOERROR;

  switch(input){
      case RSEMIB_OK:
          ret = SNMP_ERR_NOERROR;
          break;
      case RSEMIB_BAD_INPUT:
          set_rsuSecurity_error_states(SECURITY_BAD_INPUT);
          ret = SNMP_ERR_BADVALUE;
          break;
      case RSEMIB_BAD_INDEX:
          set_rsuSecurity_error_states(SECURITY_BAD_INDEX); 
          ret = SNMP_ERR_BADVALUE;
          break;
      case RSEMIB_LOCK_FAIL:
          set_rsuSecurity_error_states(SECURITY_LOCK_FAIL); 
          ret = SNMP_ERR_NOACCESS;   
          break;
/* need GET VS PUT BAD DATA. */
      case RSEMIB_BAD_DATA:
          set_rsuSecurity_error_states(SECURITY_BAD_DATA);
          ret = SNMP_ERR_BADVALUE;
          break;
      case SECURITY_BAD_MODE:
          set_rsuSecurity_error_states(SECURITY_BAD_MODE);
          ret = SNMP_ERR_INCONSISTENTVALUE;
          break;
      default:
          set_rsuSecurity_error_states(SECURITY_BAD_MODE);
          ret = SNMP_ERR_NOACCESS;
          break;
  }
  return ret;
}
/*
 * OID Handlers
 */

/* 5.9.1 */
int32_t handle_rsuSecCredReq(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo,
                            netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret = 0;

  handler = handler; /* Silence of the Warnings. */
  reginfo = reginfo;

  DEBUGMSGTL((MY_NAME, "handle_rsuSecCredReq: Entry:\n"));
  switch (reqinfo->mode){
      case MODE_GET:
          DEBUGMSGTL((MY_NAME, "GET:\n"));
          if(0 <= (ret = get_rsuSecCredReq())){
              snmp_set_var_typed_integer(requests->requestvb, ASN_INTEGER, ret);
              ret = RSEMIB_OK;
          }
          break;
      case MODE_SET_RESERVE1:
          break;
      case MODE_SET_RESERVE2:
          break; 
      case MODE_SET_FREE:
          break;
      case MODE_SET_ACTION:
          DEBUGMSGTL((MY_NAME, "ACTION:\n"));
          if(RSEMIB_OK == (ret = preserve_rsuSecCredReq())){
              ret = action_rsuSecCredReq( *(requests->requestvb->val.integer));
          }
          break;
      case MODE_SET_COMMIT:
          DEBUGMSGTL((MY_NAME, "COMMIT:\n"));
          if(RSEMIB_OK == (ret = commit_rsuSecCredReq())){
              /*
               * Back end magic here.
               */
          }
          break;
      case MODE_SET_UNDO:
          DEBUGMSGTL((MY_NAME, "UNDO:\n"));
          ret = undo_rsuSecCredReq();
          break;
      default:
          DEBUGMSGTL((MY_NAME, "unknown mode (%d):\n",reqinfo->mode));
          ret = SECURITY_BAD_MODE;
          break;
  }
  /* Set once and only if error. Only top handler need do this. */
  if(RSEMIB_OK != ret){
      ret = handle_rsemib_errors(ret); /* Convert from module error to SNMP request error. */
      netsnmp_set_request_error(reqinfo, requests, ret);
  }
  DEBUGMSGTL((MY_NAME, "ret=%d:Exit.\n",ret));
  return ret;
}
/* 5.9.2 */
int32_t handle_rsuSecEnrollCertStatus(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo,
                            netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret = 0;

  handler = handler; /* Silence of the Warnings. */
  reginfo = reginfo;

  DEBUGMSGTL((MY_NAME, "handle_rsuSecEnrollCertStatus: Entry:\n"));
  switch (reqinfo->mode){
      case MODE_GET:
          DEBUGMSGTL((MY_NAME, "GET:\n"));
          if(0 <= (ret = get_rsuSecEnrollCertStatus())){
              snmp_set_var_typed_integer(requests->requestvb, ASN_INTEGER, ret);
              ret = RSEMIB_OK;
          }
          break;
      default:
          DEBUGMSGTL((MY_NAME, "unknown mode (%d):\n",reqinfo->mode));
          ret = SECURITY_BAD_MODE;
          break;
  }
  /* Set once and only if error. Only top handler need do this. */
  if(RSEMIB_OK != ret){
      ret = handle_rsemib_errors(ret); /* Convert from module error to SNMP request error. */
      netsnmp_set_request_error(reqinfo, requests, ret);
  }
  DEBUGMSGTL((MY_NAME, "ret=%d:Exit.\n",ret));
  return ret;
}
/* 5.9.3 */
int32_t handle_rsuSecEnrollCertValidRegion(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo,
                            netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret = 0;

  handler = handler; /* Silence of the Warnings. */
  reginfo = reginfo;

  DEBUGMSGTL((MY_NAME, "handle_rsuSecEnrollCertValidRegion: Entry:\n"));
  switch (reqinfo->mode){
      case MODE_GET:
          DEBUGMSGTL((MY_NAME, "GET:\n"));
          if(0 <= (ret = get_rsuSecEnrollCertValidRegion())){
              snmp_set_var_typed_integer(requests->requestvb, ASN_INTEGER, ret);
              ret = RSEMIB_OK;
          }
          break;
      default:
          DEBUGMSGTL((MY_NAME, "unknown mode (%d):\n",reqinfo->mode));
          ret = SECURITY_BAD_MODE;
          break;
  }
  /* Set once and only if error. Only top handler need do this. */
  if(RSEMIB_OK != ret){
      ret = handle_rsemib_errors(ret); /* Convert from module error to SNMP request error. */
      netsnmp_set_request_error(reqinfo, requests, ret);
  }
  DEBUGMSGTL((MY_NAME, "ret=%d:Exit.\n",ret));
  return ret;
}
/* 5.9.4 */
int32_t handle_rsuSecEnrollCertUrl(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo,
                            netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret = 0;
  uint8_t data[URI255_LENGTH_MAX];

  handler = handler; /* Silence of the Warnings. */
  reginfo = reginfo;

  DEBUGMSGTL((MY_NAME, "handle_rsuSecEnrollCertUrl: Entry:\n"));
  switch (reqinfo->mode){
      case MODE_GET:
          DEBUGMSGTL((MY_NAME, "GET:\n"));
          memset(data,0x0,sizeof(data));
          if(0 <= (ret = get_rsuSecEnrollCertUrl(data))){
              snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR, data, ret);
              ret = RSEMIB_OK;
          }
          break;
      default:
          DEBUGMSGTL((MY_NAME, "unknown mode (%d):\n",reqinfo->mode));
          ret = SECURITY_BAD_MODE;
          break;
  }
  /* Set once and only if error. Only top handler need do this. */
  if(RSEMIB_OK != ret){
      ret = handle_rsemib_errors(ret); /* Convert from module error to SNMP request error. */
      netsnmp_set_request_error(reqinfo, requests, ret);
  }
  DEBUGMSGTL((MY_NAME, "ret=%d:Exit.\n",ret));
  return ret;
}
/* 5.9.5 */
int32_t handle_rsuSecEnrollCertId(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo,
                            netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret = 0;
  uint8_t data[RSU_SECURITY_ENROLL_CERT_ID_LENGTH_MAX];

  handler = handler; /* Silence of the Warnings. */
  reginfo = reginfo;

  DEBUGMSGTL((MY_NAME, "handle_rsuSecEnrollCertId: Entry:\n"));
  switch (reqinfo->mode){
      case MODE_GET:
          DEBUGMSGTL((MY_NAME, "GET:\n"));
          memset(data,0x0,sizeof(data));
          if(0 <= (ret = get_rsuSecEnrollCertId(data))){
              snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR, data, ret);
              ret = RSEMIB_OK;
          }
          break;
      default:
          DEBUGMSGTL((MY_NAME, "unknown mode (%d):\n",reqinfo->mode));
          ret = SECURITY_BAD_MODE;
          break;
  }
  /* Set once and only if error. Only top handler need do this. */
  if(RSEMIB_OK != ret){
      ret = handle_rsemib_errors(ret); /* Convert from module error to SNMP request error. */
      netsnmp_set_request_error(reqinfo, requests, ret);
  }
  DEBUGMSGTL((MY_NAME, "ret=%d:Exit.\n",ret));
  return ret;
}
/* 5.9.6 */
int32_t handle_rsuSecEnrollCertExpiration(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo,
                            netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret = 0;
  uint8_t data[MIB_DATEANDTIME_LENGTH];

  handler = handler; /* Silence of the Warnings. */
  reginfo = reginfo;

  DEBUGMSGTL((MY_NAME, "handle_rsuSecEnrollCertExpiration: Entry:\n"));
  switch (reqinfo->mode){
      case MODE_GET:
          DEBUGMSGTL((MY_NAME, "GET:\n"));
          memset(data,0x0,sizeof(data));
          if(RSEMIB_OK == (ret = get_rsuSecEnrollCertExpiration(data))){
              snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR, data, MIB_DATEANDTIME_LENGTH);
          }
          break;
      default:
          DEBUGMSGTL((MY_NAME, "unknown mode (%d):\n",reqinfo->mode));
          ret = SECURITY_BAD_MODE;
          break;
  }
  /* Set once and only if error. Only top handler need do this. */
  if(RSEMIB_OK != ret){
      ret = handle_rsemib_errors(ret); /* Convert from module error to SNMP request error. */
      netsnmp_set_request_error(reqinfo, requests, ret);
  }
  DEBUGMSGTL((MY_NAME, "ret=%d:Exit.\n",ret));
  return ret;
}
/* 5.9.7 */
int32_t handle_rsuSecuritySource(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo,
                            netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret = 0;

  handler = handler; /* Silence of the Warnings. */
  reginfo = reginfo;

  DEBUGMSGTL((MY_NAME, "handle_rsuSecuritySource: Entry:\n"));
  switch (reqinfo->mode){
      case MODE_GET:
          DEBUGMSGTL((MY_NAME, "GET:\n"));
          if(0 <= (ret = get_rsuSecuritySource())){
              snmp_set_var_typed_integer(requests->requestvb, ASN_INTEGER, ret);
              ret = RSEMIB_OK;
          }
          break;
      default:
          DEBUGMSGTL((MY_NAME, "unknown mode (%d):\n",reqinfo->mode));
          ret = SECURITY_BAD_MODE;
          break;
  }
  /* Set once and only if error. Only top handler need do this. */
  if(RSEMIB_OK != ret){
      ret = handle_rsemib_errors(ret); /* Convert from module error to SNMP request error. */
      netsnmp_set_request_error(reqinfo, requests, ret);
  }
  DEBUGMSGTL((MY_NAME, "ret=%d:Exit.\n",ret));
  return ret;
}
/* 5.9.8 */
int32_t handle_rsuSecAppCertUrl(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo,
                            netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret = 0;
  uint8_t data[URI1024_LENGTH_MAX];

  handler = handler; /* Silence of the Warnings. */
  reginfo = reginfo;

  DEBUGMSGTL((MY_NAME, "handle_rsuSecAppCertUrl: Entry:\n"));
  switch (reqinfo->mode){
      case MODE_GET:
          DEBUGMSGTL((MY_NAME, "GET:\n"));
          memset(data,0x0,sizeof(data));
          if(0 <= (ret = get_rsuSecAppCertUrl(data))){
              snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR, data, ret);
              ret = RSEMIB_OK;
          }
          break;
      default:
          DEBUGMSGTL((MY_NAME, "unknown mode (%d):\n",reqinfo->mode));
          ret = SECURITY_BAD_MODE;
          break;
  }
  /* Set once and only if error. Only top handler need do this. */
  if(RSEMIB_OK != ret){
      ret = handle_rsemib_errors(ret); /* Convert from module error to SNMP request error. */
      netsnmp_set_request_error(reqinfo, requests, ret);
  }
  DEBUGMSGTL((MY_NAME, "ret=%d:Exit.\n",ret));
  return ret;
}
/* 5.9.9 */
int32_t handle_maxRsuSecAppCerts(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo,
                            netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret = 0;

  handler = handler; /* Silence of the Warnings. */
  reginfo = reginfo;

  DEBUGMSGTL((MY_NAME, "handle_maxRsuSecAppCerts: Entry:\n"));
  switch (reqinfo->mode){
      case MODE_GET:
          DEBUGMSGTL((MY_NAME, "GET:\n"));
          if(0 <= (ret = get_maxRsuSecAppCerts())){
              snmp_set_var_typed_integer(requests->requestvb, ASN_INTEGER, ret);
              ret = RSEMIB_OK;
          }
          break;
      default:
          DEBUGMSGTL((MY_NAME, "unknown mode (%d):\n",reqinfo->mode));
          ret = SECURITY_BAD_MODE;
          break;
  }
  /* Set once and only if error. Only top handler need do this. */
  if(RSEMIB_OK != ret){
      ret = handle_rsemib_errors(ret); /* Convert from module error to SNMP request error. */
      netsnmp_set_request_error(reqinfo, requests, ret);
  }
  DEBUGMSGTL((MY_NAME, "ret=%d:Exit.\n",ret));
  return ret;
}
/* 
 * 5.9.10.1 - 5.9.10.5
 *
 * typedef struct{
 *      int32_t rsuSecAppCertIndex;                      RO: 1 to maxRsuSecAppCerts. Must be 1 or greater.
 *      uint8_t rsuSecAppCertPsid[RSU_SEC_APP_PSID_MAX]; RO: 1 to 255: "up to 64": should be 4 to 252?: 64 * 4 = 256? Padded to 4 octet.
 *      int32_t rsuSecAppCertState;                      RO: 1..4: 1 = other, 2 = valid, 3 = notValid, 4 = future: validty= time + region.
 *      int32_t rsuSecAppCertExpiration;                 RO: 0 to 255: hours: 0 = expire today, 255 = expired, 1 to 254 = till expire.
 *      int32_t rsuSecAppCertReq;                        RO: 0 to 655535: hours before rsuSecAppCertExpiration RSU make req new creds. 
 *  } RsuSecAppCertEntry_t;
 *
 */
/* 5.9.10.1 */
static int32_t handle_rsuSecAppCertIndex(int32_t index,netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret = 0;

  DEBUGMSGTL((MY_NAME, "handle_rsuSecAppCertIndex_%d: Entry:\n", index));
  switch (reqinfo->mode){
      case MODE_GET:
          if((1 <= index) && (index <= RSU_SEC_APP_CERTS_MAX)){
              snmp_set_var_typed_integer(requests->requestvb, ASN_INTEGER, index);
              ret = RSEMIB_OK;
          } else {
              ret = SECURITY_BAD_INDEX;
          }
          break;
      default:
          DEBUGMSGTL((MY_NAME, "unknown mode (%d):\n",reqinfo->mode));
          ret = SECURITY_BAD_MODE;
          break;
  }
  DEBUGMSGTL((MY_NAME, "ret=%d:Exit.\n",ret));
  return ret;
}
/* 5.9.10.2 */
static int32_t handle_rsuSecAppCertPsid(int32_t index, netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret = 0;
  uint8_t data[RSU_SEC_APP_PSID_MAX];

  DEBUGMSGTL((MY_NAME, "handle_rsuSecAppCertPsid_%d: Entry:\n", index));
  switch (reqinfo->mode){
      case MODE_GET:
          DEBUGMSGTL((MY_NAME, "GET:\n"));
          memset(data,0x0,sizeof(data));
          if(0 <= (ret = get_rsuSecAppCertPsid(index, data))){
#if 1
              snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR, data, ret);
#else
{
  uint8_t  packed_data[RSU_SEC_APP_PSID_MAX];
  int32_t  i = 0,j = 0;
              memset(packed_data,0x0,sizeof(packed_data));
              #if 1 /* MSB first */
              for(i=0,j=0;(i < RSU_SEC_APP_PSID_MAX) && (i < ret);i++){
                  packed_data[j++] = (uint8_t) ( (0xff000000 & data[i]) >> 24);
                  packed_data[j++] = (uint8_t) ( (0x00ff0000 & data[i]) >> 16);
                  packed_data[j++] = (uint8_t) ( (0x0000ff00 & data[i]) >> 8);
                  packed_data[j++] = (uint8_t) ( (0x000000ff & data[i]));
              }
              #else /* LSB first */
              for(i=0,j=0;(i < RSU_SEC_APP_PSID_MAX) && (i < ret);i++){
                  packed_data[j++] = (uint8_t) ( (0x000000ff & data[i]));
                  packed_data[j++] = (uint8_t) ( (0x0000ff00 & data[i]) >> 8);
                  packed_data[j++] = (uint8_t) ( (0x00ff0000 & data[i]) >> 16);
                  packed_data[j++] = (uint8_t) ( (0xff000000 & data[i]) >> 24);
              }
              #endif
              snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR, packed_data, (4 * ret)); 
}
#endif

              ret = RSEMIB_OK;
          }
          break;
      default:
          DEBUGMSGTL((MY_NAME, "unknown mode (%d):\n",reqinfo->mode));
          ret = SECURITY_BAD_MODE;
          break;
  }
  DEBUGMSGTL((MY_NAME, "ret=%d:Exit.\n",ret));
  return ret;
}
/* 5.9.10.3 */
static int32_t handle_rsuSecAppCertState(int32_t index,netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret = 0;

  DEBUGMSGTL((MY_NAME, "handle_rsuSecAppCertState_%d: Entry:\n", index));
  switch (reqinfo->mode){
      case MODE_GET:
          if(0 <= (ret = get_rsuSecAppCertState(index))){
              snmp_set_var_typed_integer(requests->requestvb, ASN_INTEGER, ret);
              ret = RSEMIB_OK;
          }
          break;
      default:
          DEBUGMSGTL((MY_NAME, "unknown mode (%d):\n",reqinfo->mode));
          ret = SECURITY_BAD_MODE;
          break;
  }
  DEBUGMSGTL((MY_NAME, "ret=%d:Exit.\n",ret));
  return ret;
}
/* 5.9.10.4 */
static int32_t handle_rsuSecAppCertExpiration(int32_t index,netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret = 0;

  DEBUGMSGTL((MY_NAME, "handle_rsuSecAppCertExpiration_%d: Entry:\n", index));
  switch (reqinfo->mode){
      case MODE_GET:
          if(0 <= (ret = get_rsuSecAppCertExpiration(index))){
              snmp_set_var_typed_integer(requests->requestvb, ASN_INTEGER, ret);
              ret = RSEMIB_OK;
          }
          break;
      default:
          DEBUGMSGTL((MY_NAME, "unknown mode (%d):\n",reqinfo->mode));
          ret = SECURITY_BAD_MODE;
          break;
  }
  DEBUGMSGTL((MY_NAME, "ret=%d:Exit.\n",ret));
  return ret;
}
/* 5.9.10.5 */
static int32_t handle_rsuSecAppCertReq(int32_t index,netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret = 0;

  DEBUGMSGTL((MY_NAME, "handle_rsuSecAppCertReq_%d: Entry:\n", index));
  switch (reqinfo->mode){
      case MODE_GET:
          if (0 <= (ret = get_rsuSecAppCertReq(index))){
              snmp_set_var_typed_integer(requests->requestvb, ASN_INTEGER, ret);
              ret = RSEMIB_OK;
          }
          break;
      case MODE_SET_RESERVE1:
          break;
      case MODE_SET_RESERVE2:
          break; 
      case MODE_SET_FREE:
          break;
      case MODE_SET_ACTION:
          DEBUGMSGTL((MY_NAME, "ACTION:\n"));
          if(RSEMIB_OK == (ret = preserve_rsuSecAppCertReq(index))){
              ret = action_rsuSecAppCertReq(index, *(requests->requestvb->val.integer));
          }
          break;
      case MODE_SET_COMMIT:
          DEBUGMSGTL((MY_NAME, "COMMIT:\n"));
          if(RSEMIB_OK == (ret = commit_rsuSecAppCertReq(index))){
              /*
               * Back end magic here.
               */
          }
          break;
      case MODE_SET_UNDO:
          DEBUGMSGTL((MY_NAME, "UNDO:\n"));
          ret = undo_rsuSecAppCertReq(index);
          break;
      default:
          DEBUGMSGTL((MY_NAME, "unknown mode (%d):\n",reqinfo->mode));
          ret = SECURITY_BAD_MODE;
          break;
  }
  DEBUGMSGTL((MY_NAME, "ret=%d:Exit.\n",ret));
  return ret;
}
int32_t handle_rsuSecAppCertTable( netsnmp_mib_handler      * handler
                             , netsnmp_handler_registration * reginfo
                             , netsnmp_agent_request_info   * reqinfo
                             , netsnmp_request_info         * requests)
{
  netsnmp_request_info       * request       = NULL;
  uint32_t                     request_count = 0;
  RsuSecAppCertEntry_t       * table_entry   = NULL;
  netsnmp_table_request_info * request_info  = NULL;
  int32_t                      ret           = SNMP_ERR_NOERROR;

  handler = handler; /* Silence of the Warnings. */
  reginfo = reginfo;

  DEBUGMSGTL((MY_NAME, "rsuSecAppCertTable_handler: Entry:\n"));

  for (request = requests, request_count = 0; 
          (SNMP_ERR_NOERROR == ret) && request && (request_count < MAX_REQ_COUNT); 
               request = request->next,request_count++){

      if (request->processed){
          DEBUGMSGTL((MY_NAME, "request->processed.\n"));
          continue;
      }

      table_entry  = (RsuSecAppCertEntry_t  *)netsnmp_tdata_extract_entry(request);
      request_info = netsnmp_extract_table_info(request);

      if ((NULL == table_entry) || (NULL == request_info)){
          ret = SNMP_NOSUCHINSTANCE;
      } else {
           DEBUGMSGTL((MY_NAME, "COLUMN=%d.\n",request_info->colnum));
           switch (request_info->colnum){
              case 1:
                  ret = handle_rsuSecAppCertIndex(table_entry->rsuSecAppCertIndex, reqinfo, request);
                  break;
              case 2:
                  ret = handle_rsuSecAppCertPsid(table_entry->rsuSecAppCertIndex, reqinfo, request);
                  break;
              case 3:
                  ret = handle_rsuSecAppCertState(table_entry->rsuSecAppCertIndex, reqinfo, request);
                  break;
              case 4:
                  ret = handle_rsuSecAppCertExpiration(table_entry->rsuSecAppCertIndex, reqinfo, request);
                  break;
              case 5:
                  ret = handle_rsuSecAppCertReq(table_entry->rsuSecAppCertIndex, reqinfo, request);
                  break;
              default:
                  /*
                   * Ignore requests for columns outside defintion.
                   * Dont send error because it will kill walk == bad!
                   */
                  break;
          }
          /* Convert from module error to SNMP request error. */
          ret = handle_rsemib_errors(ret);
      } /* if */
  } /* for */
  /* Set once and only if error. Only top handler need do this. */
  if(SNMP_ERR_NOERROR != ret){
      netsnmp_set_request_error(reqinfo, requests, ret);
  }
  DEBUGMSGTL((MY_NAME, "ret=%d:Exit.\n",ret));
  return ret;
}
/* 5.9.11 */
int32_t handle_rsuSecCertRevocationUrl(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo,
                            netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret = 0;
  uint8_t data[URI255_LENGTH_MAX];

  handler = handler; /* Silence of the Warnings. */
  reginfo = reginfo;

  DEBUGMSGTL((MY_NAME, "handle_rsuSecCertRevocationUrl: Entry:\n"));
  switch (reqinfo->mode){
      case MODE_GET:
          DEBUGMSGTL((MY_NAME, "GET:\n"));
          memset(data,0x0,sizeof(data));
          if(0 <= (ret = get_rsuSecCertRevocationUrl(data))){
              snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR, data, ret);
              ret = RSEMIB_OK;
          }
          break;
      default:
          DEBUGMSGTL((MY_NAME, "unknown mode (%d):\n",reqinfo->mode));
          ret = SECURITY_BAD_MODE;
          break;
  }
  /* Set once and only if error. Only top handler need do this. */
  if(RSEMIB_OK != ret){
      ret = handle_rsemib_errors(ret); /* Convert from module error to SNMP request error. */
      netsnmp_set_request_error(reqinfo, requests, ret);
  }
  DEBUGMSGTL((MY_NAME, "ret=%d:Exit.\n",ret));
  return ret;
}
/* 5.9.12 */
int32_t handle_rsuSecCertRevocationTime(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo,
                            netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret  = SNMP_ERR_NOERROR;
  uint8_t data[MIB_DATEANDTIME_LENGTH];

  handler = handler; /* Silence of the Warnings. */
  reginfo = reginfo;

  DEBUGMSGTL((MY_NAME, "handle_rsuSecCertRevocationTime: Entry:\n"));
  switch (reqinfo->mode){
      case MODE_GET:
          DEBUGMSGTL((MY_NAME, "GET:\n"));
          memset(data,0x0,sizeof(data));
          if(RSEMIB_OK == (ret = get_rsuSecCertRevocationTime(data))){
              snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR,data,MIB_DATEANDTIME_LENGTH);
          }
          break;
      default:
          DEBUGMSGTL((MY_NAME, "unknown mode (%d):\n",reqinfo->mode));
          ret = SECURITY_BAD_MODE;
          break;
  }
  /* Set once and only if error. Only top handler need do this. */
  if(RSEMIB_OK != ret){
      ret = handle_rsemib_errors(ret); /* Convert from module error to SNMP request error. */
      netsnmp_set_request_error(reqinfo, requests, ret);
  }
  DEBUGMSGTL((MY_NAME, "ret=%d:Exit.\n",ret));
  return ret;
}
/* 5.9.13 */
int32_t handle_rsuSecCertRevocationInterval(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo,
                            netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret = 0;

  handler = handler; /* Silence of the Warnings. */
  reginfo = reginfo;

  DEBUGMSGTL((MY_NAME, "handle_rsuSecCertRevocationInterval: Entry:\n"));
  switch (reqinfo->mode){
      case MODE_GET:
          DEBUGMSGTL((MY_NAME, "GET:\n"));
          if(0 <= (ret = get_rsuSecCertRevocationInterval())){
              snmp_set_var_typed_integer(requests->requestvb, ASN_INTEGER, ret);
              ret = RSEMIB_OK;
          }
          break;
      case MODE_SET_RESERVE1:
          break;
      case MODE_SET_RESERVE2:
          break; 
      case MODE_SET_FREE:
          break;
      case MODE_SET_ACTION:
          DEBUGMSGTL((MY_NAME, "ACTION:\n"));
          if(RSEMIB_OK == (ret = preserve_rsuSecCertRevocationInterval())){
              ret = action_rsuSecCertRevocationInterval( *(requests->requestvb->val.integer));
          }
          break;
      case MODE_SET_COMMIT:
          DEBUGMSGTL((MY_NAME, "COMMIT:\n"));
          if(RSEMIB_OK == (ret = commit_rsuSecCertRevocationInterval())){
              /*
               * Back end magic here.
               */
              ret = SNMP_ERR_NOERROR;
          }
          break;
      case MODE_SET_UNDO:
          DEBUGMSGTL((MY_NAME, "UNDO:\n"));
          ret = undo_rsuSecCertRevocationInterval();
          break;
      default:
          DEBUGMSGTL((MY_NAME, "unknown mode (%d):\n",reqinfo->mode));
          ret = SECURITY_BAD_MODE;
          break;
  }
  /* Set once and only if error. Only top handler need do this. */
  if(RSEMIB_OK != ret){
      ret = handle_rsemib_errors(ret); /* Convert from module error to SNMP request error. */
      netsnmp_set_request_error(reqinfo, requests, ret);
  }
  DEBUGMSGTL((MY_NAME, "ret=%d:Exit.\n",ret));
  return ret;
}
/* 5.9.14 */
int32_t handle_rsuSecCertRevocationUpdate(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo,
                            netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret = 0;

  handler = handler; /* Silence of the Warnings. */
  reginfo = reginfo;

  DEBUGMSGTL((MY_NAME, "handle_rsuSecCertRevocationUpdate: Entry:\n"));
  switch (reqinfo->mode){
      case MODE_GET:
          DEBUGMSGTL((MY_NAME, "GET:\n"));
          if(0 <= (ret = get_rsuSecCertRevocationUpdate())){
              snmp_set_var_typed_integer(requests->requestvb, ASN_INTEGER, ret);
              ret = RSEMIB_OK;
          }
          break;
      case MODE_SET_RESERVE1:
          break;
      case MODE_SET_RESERVE2:
          break; 
      case MODE_SET_FREE:
          break;
      case MODE_SET_ACTION:
          DEBUGMSGTL((MY_NAME, "ACTION:\n"));
          if(RSEMIB_OK == (ret = preserve_rsuSecCertRevocationUpdate())){
              ret = action_rsuSecCertRevocationUpdate( *(requests->requestvb->val.integer));
          }
          break;
      case MODE_SET_COMMIT:
          DEBUGMSGTL((MY_NAME, "COMMIT:\n"));
          if(RSEMIB_OK == (ret = commit_rsuSecCertRevocationUpdate())){
              /*
               * Back end magic here.
               */
          }
          break;
      case MODE_SET_UNDO:
          DEBUGMSGTL((MY_NAME, "UNDO:\n"));
          ret = undo_rsuSecCertRevocationUpdate();
          break;
      default:
          DEBUGMSGTL((MY_NAME, "unknown mode (%d):\n",reqinfo->mode));
          ret = SECURITY_BAD_MODE;
          break;
  }
  /* Set once and only if error. Only top handler need do this. */
  if(RSEMIB_OK != ret){
      ret = handle_rsemib_errors(ret); /* Convert from module error to SNMP request error. */
      netsnmp_set_request_error(reqinfo, requests, ret);
  }
  DEBUGMSGTL((MY_NAME, "ret=%d:Exit.\n",ret));
  return ret;
}
/* 5.9.15 */
int32_t handle_maxRsuSecProfiles(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo,
                            netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret = 0;

  handler = handler; /* Silence of the Warnings. */
  reginfo = reginfo;

  DEBUGMSGTL((MY_NAME, "handle_maxRsuSecProfiles: Entry:\n"));
  switch (reqinfo->mode){
      case MODE_GET:
          DEBUGMSGTL((MY_NAME, "GET:\n"));
          if(0 <= (ret = get_maxRsuSecProfiles())){
              snmp_set_var_typed_integer(requests->requestvb, ASN_INTEGER, ret);
              ret = RSEMIB_OK;
          }
          break;
      default:
          DEBUGMSGTL((MY_NAME, "unknown mode (%d):\n",reqinfo->mode));
          ret = SECURITY_BAD_MODE;
          break;
  }
  /* Set once and only if error. Only top handler need do this. */
  if(RSEMIB_OK != ret){
      ret = handle_rsemib_errors(ret); /* Convert from module error to SNMP request error. */
      netsnmp_set_request_error(reqinfo, requests, ret);
  }
  DEBUGMSGTL((MY_NAME, "ret=%d:Exit.\n",ret));
  return ret;
}

/* 
 * 5.9.16: RO.
 *
 *  typedef struct{
 *      int32_t rsuSecProfileIndex;                                        1 to maxRsuSecProfiles: index must be 1 or greater. 
 *      uint8_t  rsuSecProfileName[RSU_SECURITY_PROFILE_NAME_LENGTH_MAX];   1..127: name of profile. 
 *      uint8_t  rsuSecProfileDesc[RSU_SECURITY_PROFILE_DESC_LENGTH_MAX];   0..255: description. 
 *  } RsuSecProfileEntry_t;
 *
 */

/* 5.9.16.1 */
static int32_t handle_rsuSecProfileIndex(int32_t index,netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret = 0;

  DEBUGMSGTL((MY_NAME, "handle_rsuSecProfileIndex_%d: Entry:\n", index));
  switch (reqinfo->mode){
      case MODE_GET:
          if((1 <= index) && (index <= RSU_SECURITY_PROFILES_MAX)){
              snmp_set_var_typed_integer(requests->requestvb, ASN_INTEGER, index);
              ret = RSEMIB_OK;
          }
          break;
      default:
          DEBUGMSGTL((MY_NAME, "unknown mode (%d):\n",reqinfo->mode));
          ret = SECURITY_BAD_MODE;
          break;
  }
  DEBUGMSGTL((MY_NAME, "ret=%d:Exit.\n",ret));
  return ret;
}
/* 5.9.16.2 */
static int32_t handle_rsuSecProfileName(int32_t index, netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret = 0;
  uint8_t data[RSU_SECURITY_PROFILE_NAME_LENGTH_MAX];

  DEBUGMSGTL((MY_NAME, "handle_rsuSecProfileName_%d: Entry:\n", index));
  switch (reqinfo->mode){
      case MODE_GET:
          DEBUGMSGTL((MY_NAME, "GET:\n"));
          memset(data,0x0,sizeof(data));
          if(0 <= (ret = get_rsuSecProfileName(index,data))){
              snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR, data, ret); 
              ret = RSEMIB_OK;
          }
          break;
      default:
          DEBUGMSGTL((MY_NAME, "unknown mode (%d):\n",reqinfo->mode));
          ret = SECURITY_BAD_MODE;
          break;
  }
  DEBUGMSGTL((MY_NAME, "ret=%d:Exit.\n",ret));
  return ret;
}
/* 5.9.16.3 */
static int32_t handle_rsuSecProfileDesc(int32_t index, netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
  int32_t ret = 0;
  uint8_t data[RSU_SECURITY_PROFILE_DESC_LENGTH_MAX];

  DEBUGMSGTL((MY_NAME, "handle_rsuSecProfileDesc_%d: Entry:\n", index));
  switch (reqinfo->mode){
      case MODE_GET:
          DEBUGMSGTL((MY_NAME, "GET:\n"));
          memset(data,0x0,sizeof(data));
          if(0 <= (ret = get_rsuSecProfileDesc(index, data))){
              snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR, data, ret); 
              ret = RSEMIB_OK;
          }
          break;
      default:
          DEBUGMSGTL((MY_NAME, "unknown mode (%d):\n",reqinfo->mode));
          ret = SECURITY_BAD_MODE;
          break;
  }
  DEBUGMSGTL((MY_NAME, "ret=%d:Exit.\n",ret));
  return ret;
}
int32_t handle_rsuSecProfileTable( netsnmp_mib_handler           * handler
                                  , netsnmp_handler_registration * reginfo
                                  , netsnmp_agent_request_info   * reqinfo
                                  , netsnmp_request_info         * requests)
{
  netsnmp_request_info       * request       = NULL;
  uint32_t                     request_count = 0;
  RsuSecProfileEntry_t       * table_entry   = NULL;
  netsnmp_table_request_info * request_info  = NULL;
  int32_t                      ret           = SNMP_ERR_NOERROR;

  handler = handler; /* Silence of the Warnings. */
  reginfo = reginfo;

  DEBUGMSGTL((MY_NAME, "rsuSecProfileTable_handler: Entry:\n"));

/* Must have a semaphore between the update and the handler so requests dont have helper nuked under it.*/

  /* Service each request and provide netsnmp_set_request_error() result for each. */
  for (request = requests;
          (SNMP_ERR_NOERROR == ret) && request && (request_count < MAX_REQ_COUNT); 
               request = request->next, request_count++){

      if (request->processed){
          DEBUGMSGTL((MY_NAME, "request->processed.\n"));
          continue;
      }

      table_entry  = (RsuSecProfileEntry_t *)netsnmp_tdata_extract_entry(request);
      request_info = netsnmp_extract_table_info(request);

      if((NULL == table_entry) || (NULL == request_info)){
          ret = SNMP_NOSUCHINSTANCE;
      } else {
           DEBUGMSGTL((MY_NAME, "COLUMN=%d.\n",request_info->colnum));
           switch (request_info->colnum){
              case 1:
                  ret = handle_rsuSecProfileIndex(table_entry->rsuSecProfileIndex, reqinfo, request);
                  break;
              case 2:
                  ret = handle_rsuSecProfileName(table_entry->rsuSecProfileIndex, reqinfo, request);
                  break;
              case 3:
                  ret = handle_rsuSecProfileDesc(table_entry->rsuSecProfileIndex, reqinfo, request);
                  break;
              default:
                  /* Ignore requests for columns outside defintion. Dont send error because it will kill snmpwalks. */
                  break;
          }
          /* Convert from module error to SNMP request error. */
          ret = handle_rsemib_errors(ret);
      } /* if */
  } /* for */
  /* Set once and only if error. Only top handler need do this. */
  if(SNMP_ERR_NOERROR != ret){
      netsnmp_set_request_error(reqinfo, requests, ret);
  }
  DEBUGMSGTL((MY_NAME, "ret=%d:Exit.\n",ret));
  return ret;
}

/* 
 * Table Creation.
 */

/* rsuSecAppCertTable AND rsuSecProfileTable creation functions. */
STATIC void remove_rsuSecAppCertTableEntry(netsnmp_tdata * table_data, netsnmp_tdata_row * row)
{
  RsuSecAppCertEntry_t * entry = NULL;

  if (NULL == row){
      DEBUGMSGTL((MY_NAME,"remove_rsuSecAppCertTableEntry NULL input.\n"));
      return;
  }
  if(NULL != (entry = (RsuSecAppCertEntry_t *)row->data)){
      SNMP_FREE(entry);
  } else {
      DEBUGMSGTL((MY_NAME,"rsuRadioTable_removeEntry row data is NULL!.\n"));
  }
  if (table_data){
      netsnmp_tdata_remove_and_delete_row(table_data, row);
  } else {
      DEBUGMSGTL((MY_NAME,"remove_rsuSecAppCertTableEntry NULL input.\n"));
      netsnmp_tdata_delete_row(row); 
  }
  return;
}
STATIC void profile_rsuSecProfileTableEntry(netsnmp_tdata * table_data, netsnmp_tdata_row * row)
{
  RsuSecProfileEntry_t * entry = NULL;

  if (NULL == row){
      DEBUGMSGTL((MY_NAME,"profile_rsuSecProfileTableEntry NULL input.\n"));
      return;
  }
  if(NULL != (entry = (RsuSecProfileEntry_t *)row->data)){
      SNMP_FREE(entry);
  } else {
      DEBUGMSGTL((MY_NAME,"rsuRadioTable_removeEntry row data is NULL!.\n"));
  }
  if (table_data){
      netsnmp_tdata_remove_and_delete_row(table_data, row);
  } else {
      DEBUGMSGTL((MY_NAME,"profile_rsuSecProfileTableEntry NULL input.\n"));
      netsnmp_tdata_delete_row(row); 
  }
  return;
}
STATIC void destroy_rsuSecAppCertTable(void)
{
  netsnmp_tdata_row  * row     = NULL;
  netsnmp_tdata_row  * nextrow = NULL;
  int32_t              i       = 0;
  if(NULL != rsuSecAppCertTable){
      if(NULL != (row = netsnmp_tdata_row_first(rsuSecAppCertTable))){
          for(i=0;(row != NULL) && (i < RSU_SEC_APP_CERTS_MAX);i++)  {
              nextrow = netsnmp_tdata_row_next(rsuSecAppCertTable, row);
              remove_rsuSecAppCertTableEntry(rsuSecAppCertTable, row);
              row = nextrow;
          }
      } else {
          DEBUGMSGTL((MY_NAME, "destroy_rsuSecAppCertTable: Table empty. Nothing to do.\n"));
      }
  } else {
      DEBUGMSGTL((MY_NAME, "destroy_rsuSecAppCertTable: Table is NULL. Did we get called before install?\n"));
  }
}
STATIC void destroy_rsuSecProfileTable(void)
{
  netsnmp_tdata_row  * row     = NULL;
  netsnmp_tdata_row  * nextrow = NULL;
  int32_t              i       = 0;

  if(NULL != rsuSecProfileTable){
      if(NULL != (row = netsnmp_tdata_row_first(rsuSecProfileTable))){
          for(i=0;(row != NULL) && (i < RSU_SECURITY_PROFILES_MAX);i++)  {
              nextrow = netsnmp_tdata_row_next(rsuSecProfileTable, row);
              remove_rsuSecAppCertTableEntry(rsuSecProfileTable, row);
              row = nextrow;
          }
      } else {
          DEBUGMSGTL((MY_NAME, "destroy_rsuSecProfileTable: Table empty. Nothing to do.\n"));
      }
  } else {
      DEBUGMSGTL((MY_NAME, "destroy_rsuSecProfileTable: Table is NULL. Did we get called before install?\n"));
  }
}
static netsnmp_tdata_row * create_rsuSecAppCertTable(netsnmp_tdata * table_data, RsuSecAppCertEntry_t * rsuSecAppCertTable)
{
  RsuSecAppCertEntry_t * entry = NULL;
  netsnmp_tdata_row    * row   = NULL;
  int32_t                i     = 0;
  
  if(NULL == table_data){
      DEBUGMSGTL((MY_NAME, "create_rsuSecAppCertTable FAIL: table_data == NULL.\n"));
      return NULL;
  }

  for(i=0; i < RSU_SEC_APP_CERTS_MAX; i++){
      if (NULL == (entry = SNMP_MALLOC_TYPEDEF(RsuSecAppCertEntry_t))){
          DEBUGMSGTL((MY_NAME, "rsuSecAppCert SNMP_MALLOC_TYPEDEF FAILED.\n"));
          return NULL;
      }
      if (NULL == (row = netsnmp_tdata_create_row())){
          DEBUGMSGTL((MY_NAME, "rsuSecAppCert netsnmp_tdata_create_row FAILED.\n"));
          SNMP_FREE(entry);
          return NULL;
      }
      row->data = entry;
      memcpy(entry,&rsuSecAppCertTable[i],sizeof(RsuSecAppCertEntry_t));
      netsnmp_tdata_row_add_index(row, ASN_INTEGER , &rsuSecAppCertTable[i].rsuSecAppCertIndex, sizeof(int32_t));
      if(SNMPERR_SUCCESS != netsnmp_tdata_add_row(table_data, row)){
          DEBUGMSGTL((MY_NAME, "rsuSecAppCert netsnmp_tdata_add_row FAILED.\n"));
          return NULL;
      }
  }
  return row;
}
static netsnmp_tdata_row * create_rsuSecProfileTable(netsnmp_tdata * table_data, RsuSecProfileEntry_t * rsuSecProfileTable)
{
  RsuSecProfileEntry_t * entry = NULL;
  netsnmp_tdata_row    * row   = NULL;
  int32_t                i     = 0;
  
  if(NULL == table_data){
      DEBUGMSGTL((MY_NAME, "create_rsuSecProfileCertTable FAIL: table_data == NULL.\n"));
      return NULL;
  }

  for(i=0;i<RSU_SECURITY_PROFILES_MAX;i++){
      if (NULL == (entry = SNMP_MALLOC_TYPEDEF(RsuSecProfileEntry_t))){
          DEBUGMSGTL((MY_NAME, "rsuSecProfile SNMP_MALLOC_TYPEDEF FAILED.\n"));
          return NULL;
      }
      if (NULL == (row = netsnmp_tdata_create_row())){
          DEBUGMSGTL((MY_NAME, "rsuSecProfile netsnmp_tdata_create_row FAILED.\n"));
          SNMP_FREE(entry);
          return NULL;
      }
      row->data = entry;
      memcpy(entry,&rsuSecProfileTable[i],sizeof(RsuSecProfileEntry_t));
      netsnmp_tdata_row_add_index(row, ASN_INTEGER , &rsuSecProfileTable[i].rsuSecProfileIndex, sizeof(int32_t));
      if(SNMPERR_SUCCESS != netsnmp_tdata_add_row(table_data, row)){
          DEBUGMSGTL((MY_NAME, "rsuSecProfile netsnmp_tdata_add_row FAILED.\n"));
          return NULL;
      }
  }
  return row;
}
STATIC int32_t install_rsuSecAppCertTable(RsuSecAppCertEntry_t * rsuSecAppCerts)
{
    netsnmp_handler_registration * reg = NULL;

    reg = netsnmp_create_handler_registration("rsuSecAppCertTable"
                                              ,handle_rsuSecAppCertTable
                                              ,rsuSecAppCertTable_oid
                                              ,OID_LENGTH(rsuSecAppCertTable_oid)
                                              ,HANDLER_CAN_RWRITE);

    if (NULL == (rsuSecAppCertTable = netsnmp_tdata_create_table("rsuSecAppCertTable", 0))){
        DEBUGMSGTL((MY_NAME, "rsuSecAppCertTable FAILED.\n"));
        return SECURITY_INSTALL_FAIL;
    }
    if (NULL == (app_table_reg_info = SNMP_MALLOC_TYPEDEF(netsnmp_table_registration_info))){
        DEBUGMSGTL((MY_NAME, "SNMP_MALLOC_TYPEDEF FAILED.\n"));
        return SECURITY_INSTALL_FAIL;
    }
    netsnmp_table_helper_add_indexes(app_table_reg_info, ASN_INTEGER,0); /* index */                                 

    app_table_reg_info->min_column = 2; /* rsuSecAppCertPsid:  Hide index by definition. */
    app_table_reg_info->max_column = 5; /* rsuSecAppCertReq: */

    netsnmp_tdata_register(reg, rsuSecAppCertTable, app_table_reg_info);

    /* Initialize the contents of the table. */
    if (NULL == create_rsuSecAppCertTable(rsuSecAppCertTable, rsuSecAppCerts)){
        DEBUGMSGTL((MY_NAME, "initialize rsuSecAppCertTable: ERROR: Null row.\n"));
        return SECURITY_INSTALL_FAIL;
    }
    return RSEMIB_OK;
}
STATIC int32_t install_rsuSecProfileTable(RsuSecProfileEntry_t * rsuSecProfiles)
{
    netsnmp_handler_registration * reg = NULL;

    reg = netsnmp_create_handler_registration("rsuSecProfileTable"
                                              ,handle_rsuSecProfileTable
                                              ,rsuSecProfileTable_oid
                                              ,OID_LENGTH(rsuSecProfileTable_oid)
                                              ,HANDLER_CAN_RWRITE);

    if (NULL == (rsuSecProfileTable = netsnmp_tdata_create_table("rsuSecProfileTable", 0))){
        DEBUGMSGTL((MY_NAME, "rsuSecProfileTable FAILED.\n"));
        return SECURITY_INSTALL_FAIL;
    }
    if (NULL == (profile_table_reg_info = SNMP_MALLOC_TYPEDEF(netsnmp_table_registration_info))){
        DEBUGMSGTL((MY_NAME, "SNMP_MALLOC_TYPEDEF FAILED.\n"));
        return SECURITY_INSTALL_FAIL;
    }
    netsnmp_table_helper_add_indexes(profile_table_reg_info, ASN_INTEGER,0); /* index */                                 

    profile_table_reg_info->min_column = 2; /* rsuSecProfileName  Hide index by definition. */
    profile_table_reg_info->max_column = 3; /* rsuSecProfileDesc: */

    netsnmp_tdata_register(reg, rsuSecProfileTable, profile_table_reg_info);

    /* Initialize the contents of the table. */
    if (NULL == create_rsuSecProfileTable(rsuSecProfileTable, rsuSecProfiles)){
        DEBUGMSGTL((MY_NAME, "initialize rsuSecProfileTable: ERROR: Null row.\n"));
        return SECURITY_INSTALL_FAIL;
    }
    return RSEMIB_OK;
}
void install_rsuSecurity_handlers(void)
{
  int32_t i = 0;
  RsuSecAppCertEntry_t rsuSecAppCerts[RSU_SEC_APP_CERTS_MAX];
  RsuSecProfileEntry_t rsuSecProfiles[RSU_SECURITY_PROFILES_MAX];

  /* Clear statics across soft reset. */
  app_table_reg_info       = NULL;
  profile_table_reg_info   = NULL;
  rsuSecAppCertTable       = NULL;
  rsuSecProfileTable       = NULL;
  rsuSecurity_error_states = 0x0;

  memset(rsuSecAppCerts,0x0,sizeof(rsuSecAppCerts));
  memset(rsuSecProfiles,0x0,sizeof(rsuSecProfiles));

  /* Using the functions available to handlers populate table and verify. */
  for(i=0;i<RSU_SEC_APP_CERTS_MAX;i++){
      rsuSecAppCerts[i].rsuSecAppCertIndex = i+1; /* index must be 1 or greater. */
      if(0 > get_rsuSecAppCertPsid(i+1, rsuSecAppCerts[i].rsuSecAppCertPsid)){
          DEBUGMSGTL((MY_NAME, "get_rsuSecAppCertPsid install FAIL.\n"));
          set_rsuSecurity_error_states(SECURITY_INSTALL_FAIL);
      } 
      if(1 > (rsuSecAppCerts[i].rsuSecAppCertState = get_rsuSecAppCertState(i+1))){
          DEBUGMSGTL((MY_NAME, "get_rsuSecAppCertState install FAIL.\n"));
          set_rsuSecurity_error_states(SECURITY_INSTALL_FAIL);
      }
      if(0 > (rsuSecAppCerts[i].rsuSecAppCertExpiration = get_rsuSecAppCertExpiration(i+1))){
          DEBUGMSGTL((MY_NAME, "get_rsuSecAppCertExpiration install FAIL.\n"));
          set_rsuSecurity_error_states(SECURITY_INSTALL_FAIL);
      }
      if(0 > (rsuSecAppCerts[i].rsuSecAppCertReq = get_rsuSecAppCertReq(i+1))){
          DEBUGMSGTL((MY_NAME, "get_rsuSecAppCertReq install FAIL.\n"));
          set_rsuSecurity_error_states(SECURITY_INSTALL_FAIL);
      }
  }
  DEBUGMSGTL((MY_NAME, "Fill App Security ALL Done.\n"));

  for(i=0;i<RSU_SECURITY_PROFILES_MAX;i++){
      memset(&rsuSecProfiles[i],0x0,sizeof(RsuSecProfileEntry_t));
      rsuSecProfiles[i].rsuSecProfileIndex = i+1; /* index must be 1 or greater. */
      if(0 > get_rsuSecProfileName(i+1, rsuSecProfiles[i].rsuSecProfileName)){
          DEBUGMSGTL((MY_NAME, "rsuSecProfileName install FAIL.\n"));
          set_rsuSecurity_error_states(SECURITY_INSTALL_FAIL);
      } 
      if(0 > get_rsuSecProfileDesc(i+1, rsuSecProfiles[i].rsuSecProfileDesc)){
          DEBUGMSGTL((MY_NAME, "rsuSecProfileDesc install FAIL.\n"));
          set_rsuSecurity_error_states(SECURITY_INSTALL_FAIL);
      } 
  }
  DEBUGMSGTL((MY_NAME, "Fill Profile Security Done.\n"));

  /* 5.9.1 */
  if(MIB_REGISTERED_OK != netsnmp_register_scalar(netsnmp_create_handler_registration
                          ("rsuSecCredReq", 
                           handle_rsuSecCredReq,
                           rsuSecCredReq_oid,
                           OID_LENGTH(rsuSecCredReq_oid),
                           HANDLER_CAN_RWRITE))){
      DEBUGMSGTL((MY_NAME, "rsuSecCredReq install FAIL.\n"));
      set_rsuSecurity_error_states(SECURITY_INSTALL_FAIL);
  }
  /* 5.9.2 */
  if(MIB_REGISTERED_OK != netsnmp_register_scalar(netsnmp_create_handler_registration
                          ("rsuSecEnrollCertStatus", 
                           handle_rsuSecEnrollCertStatus,
                           rsuSecEnrollCertStatus_oid,
                           OID_LENGTH(rsuSecEnrollCertStatus_oid),
                           HANDLER_CAN_RONLY))){
      DEBUGMSGTL((MY_NAME, "rsuSecEnrollCertStatus install FAIL.\n"));
      set_rsuSecurity_error_states(SECURITY_INSTALL_FAIL);
  }
  /* 5.9.3 */
  if(MIB_REGISTERED_OK != netsnmp_register_scalar(netsnmp_create_handler_registration
                          ("rsuSecEnrollCertValidRegion", 
                           handle_rsuSecEnrollCertValidRegion,
                           rsuSecEnrollCertValidRegion_oid,
                           OID_LENGTH(rsuSecEnrollCertValidRegion_oid),
                           HANDLER_CAN_RONLY))){
      DEBUGMSGTL((MY_NAME, "rsuSecEnrollCertValidRegion install FAIL.\n"));
      set_rsuSecurity_error_states(SECURITY_INSTALL_FAIL);
  }
  /* 5.9.4 */
  if(MIB_REGISTERED_OK != netsnmp_register_scalar(netsnmp_create_handler_registration
                          ("rsuSecEnrollCertUrl", 
                           handle_rsuSecEnrollCertUrl,
                           rsuSecEnrollCertUrl_oid,
                           OID_LENGTH(rsuSecEnrollCertUrl_oid),
                           HANDLER_CAN_RONLY))){
      DEBUGMSGTL((MY_NAME, "rsuSecEnrollCertUrl install FAIL.\n"));
      set_rsuSecurity_error_states(SECURITY_INSTALL_FAIL);
  }
  /* 5.9.5 */
  if(MIB_REGISTERED_OK != netsnmp_register_scalar(netsnmp_create_handler_registration
                          ("rsuSecEnrollCertId", 
                           handle_rsuSecEnrollCertId,
                           rsuSecEnrollCertId_oid,
                           OID_LENGTH(rsuSecEnrollCertId_oid),
                           HANDLER_CAN_RONLY))){
      DEBUGMSGTL((MY_NAME, "rsuSecEnrollCertId install FAIL.\n"));
      set_rsuSecurity_error_states(SECURITY_INSTALL_FAIL);
  }
  /* 5.9.6 */
  if(MIB_REGISTERED_OK != netsnmp_register_scalar(netsnmp_create_handler_registration
                          ("rsuSecEnrollCertExpiration", 
                           handle_rsuSecEnrollCertExpiration,
                           rsuSecEnrollCertExpiration_oid,
                           OID_LENGTH(rsuSecEnrollCertExpiration_oid),
                           HANDLER_CAN_RONLY))) {
      DEBUGMSGTL((MY_NAME, "rsuSecEnrollCertExpiration install FAIL.\n"));
      set_rsuSecurity_error_states(SECURITY_INSTALL_FAIL);
  }
  /* 5.9.7 */
  if(MIB_REGISTERED_OK != netsnmp_register_scalar(netsnmp_create_handler_registration
                          ("rsuSecuritySource", 
                           handle_rsuSecuritySource,
                           rsuSecuritySource_oid,
                           OID_LENGTH(rsuSecuritySource_oid),
                           HANDLER_CAN_RONLY))) {
      DEBUGMSGTL((MY_NAME, "rsuSecuritySource install FAIL.\n"));
      set_rsuSecurity_error_states(SECURITY_INSTALL_FAIL);
  }
  /* 5.9.8 */
  if(MIB_REGISTERED_OK != netsnmp_register_scalar(netsnmp_create_handler_registration
                          ("rsuSecAppCertUrl", 
                           handle_rsuSecAppCertUrl,
                           rsuSecAppCertUrl_oid,
                           OID_LENGTH(rsuSecAppCertUrl_oid),
                           HANDLER_CAN_RONLY))) {
      DEBUGMSGTL((MY_NAME, "rsuSecAppCertUrl install FAIL.\n"));
      set_rsuSecurity_error_states(SECURITY_INSTALL_FAIL);
  }
  /* 5.9.9 */
  if(MIB_REGISTERED_OK != netsnmp_register_scalar(netsnmp_create_handler_registration
                          ("maxRsuSecAppCerts", 
                           handle_maxRsuSecAppCerts,
                           maxRsuSecAppCerts_oid,
                           OID_LENGTH(maxRsuSecAppCerts_oid),
                           HANDLER_CAN_RONLY))) {
      DEBUGMSGTL((MY_NAME, "maxRsuSecAppCerts install FAIL.\n"));
      set_rsuSecurity_error_states(SECURITY_INSTALL_FAIL);
  }
  /* 5.9.10.1 - 5.9.10.5: Install table created with data from MIB. */
  if(RSEMIB_OK != install_rsuSecAppCertTable(&rsuSecAppCerts[0])){
      DEBUGMSGTL((MY_NAME, "rsuSecAppCertTable install FAIL.\n"));
      set_rsuSecurity_error_states(SECURITY_INSTALL_FAIL);
  }
  /* 5.9.11 */
  if(MIB_REGISTERED_OK != netsnmp_register_scalar(netsnmp_create_handler_registration
                          ("rsuSecCertRevocationUrl", 
                           handle_rsuSecCertRevocationUrl,
                           rsuSecCertRevocationUrl_oid,
                           OID_LENGTH(rsuSecCertRevocationUrl_oid),
                           HANDLER_CAN_RONLY))) {
      DEBUGMSGTL((MY_NAME, "rsuSecCertRevocationUrl install FAIL.\n"));
      set_rsuSecurity_error_states(SECURITY_INSTALL_FAIL);
  }
  /* 5.9.12 */
  if(MIB_REGISTERED_OK != netsnmp_register_scalar(netsnmp_create_handler_registration
                          ("rsuSecCertRevocationTime", 
                           handle_rsuSecCertRevocationTime,
                           rsuSecCertRevocationTime_oid,
                           OID_LENGTH(rsuSecCertRevocationTime_oid),
                           HANDLER_CAN_RONLY))) {
      DEBUGMSGTL((MY_NAME, "rsuSecCertRevocationTime install FAIL.\n"));
      set_rsuSecurity_error_states(SECURITY_INSTALL_FAIL);
  }
  /* 5.9.13 */
  if(MIB_REGISTERED_OK != netsnmp_register_scalar(netsnmp_create_handler_registration
                          ("rsuSecCertRevocationInterval", 
                           handle_rsuSecCertRevocationInterval,
                           rsuSecCertRevocationInterval_oid,
                           OID_LENGTH(rsuSecCertRevocationInterval_oid),
                           HANDLER_CAN_RWRITE))) {
      DEBUGMSGTL((MY_NAME, "rsuSecCertRevocationInterval install FAIL.\n"));
      set_rsuSecurity_error_states(SECURITY_INSTALL_FAIL);
  }
  /* 5.9.14 */
  if(MIB_REGISTERED_OK != netsnmp_register_scalar(netsnmp_create_handler_registration
                          ("rsuSecCertRevocationUpdate", 
                           handle_rsuSecCertRevocationUpdate,
                           rsuSecCertRevocationUpdate_oid,
                           OID_LENGTH(rsuSecCertRevocationUpdate_oid),
                           HANDLER_CAN_RWRITE))) {
      DEBUGMSGTL((MY_NAME, "rsuSecCertRevocationUpdate install FAIL.\n"));
      set_rsuSecurity_error_states(SECURITY_INSTALL_FAIL);
  }
  /* 5.9.15 */
  if(MIB_REGISTERED_OK != netsnmp_register_scalar(netsnmp_create_handler_registration
                          ("maxRsuSecProfiles", 
                           handle_maxRsuSecProfiles,
                           maxRsuSecProfiles_oid,
                           OID_LENGTH(maxRsuSecProfiles_oid),
                           HANDLER_CAN_RONLY))) {
      DEBUGMSGTL((MY_NAME, "maxRsuSecProfiles install FAIL.\n"));
      set_rsuSecurity_error_states(SECURITY_INSTALL_FAIL);
  }
  /* 5.9.16.1 - 5.9.16.3: Install table created with data from MIB. */
  if( RSEMIB_OK != install_rsuSecProfileTable(&rsuSecProfiles[0])){
      DEBUGMSGTL((MY_NAME, "rsuSecProfileTable install FAIL.\n"));
      set_rsuSecurity_error_states(SECURITY_INSTALL_FAIL);
  }
}
void destroy_rsuSecurity(void)
{
  destroy_rsuSecAppCertTable();
  destroy_rsuSecProfileTable();

  /* Free table info. */
  if (NULL != app_table_reg_info){
      SNMP_FREE(app_table_reg_info);
  }
  /* Free table reg. */
  if (NULL != rsuSecAppCertTable){
      SNMP_FREE(rsuSecAppCertTable);
  }
  /* Free table info. */
  if (NULL != profile_table_reg_info){
      SNMP_FREE(profile_table_reg_info);
  }
  /* Free table reg. */
  if (NULL != rsuSecProfileTable){
      SNMP_FREE(rsuSecProfileTable);
  }
}

