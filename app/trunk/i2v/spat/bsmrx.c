/**************************************************************************
 *                                                                        *
 *     File Name:  bsmrx.c                                                *
 *     Author:                                                            *
 *         DENSO International America, Inc.                              *
 *         North America Research Laboratory, California Office           *
 *         Rutherford road                                                *
 *         Carlsbad, CA 92008                                             *
 *                                                                        *
 **************************************************************************/
/* the purpose of this code is to receive a BSM and decode it for the vehicle
   type and heading to determine if a signal phase change needs to be passed
   to the signal controller -- within this module will be the ASN1 handling
   for decoding a BSM

   it is assumed that this function is only called after the security module
   has validated the BSM received is genuine - this function once called
   will parse what it receives and if the parse is successful and the contents
   valid it will pass it to the scs handler
   */


/* TODO:   031914: this code is incomplete - peagle requests are untested
 *      because whatever documentation for the econolite controller snmp
 *      doesn't exist (or someone has it but didn't provide to code author)
 *      this also affects scs.c 
 *      
 *      functionality has been verified up to bsms being checked for speed.
 *      at a person's desk the wsu isn't moving so the OBE GPS is reporting
 *      no speed which is broadcast in the OBE BSM - to continue testing
 *      requires modifying dwmh on v2vi to provide an arbitrary value or 
 *      there may be functionality already available to do this but the 
 *      author of this code is on the RSE side and is completely ignorant
 *      of the tips and tricks with v2vi - because of other work this bsm
 *      handling is being put on hold so the author is not going to 
 *      investigate the OBE side for now.  
 * 
 *      See the FIXMEs in this file - there are some minor todos that have
 *      to be addressed
 *
 *      This code is checked in and will not break other functionality 
 *
 *      Update 20160922: MFM: This code has been extended to support the
 *      GridSmart Demo in Arizona in September 2016. There is a GS2_SUPPORT
 *      build option that modifies the BSM processing algorithm to implement
 *      Dilemma Zone processing. In order to test, you can enable
 *      GS2DemoBSMTestMode and have an OBE broadcast BSMs. This will
 *      override the BSM position with an increasing latitude to simulate
 *      the vehicle approaching the GS2 test intersection. The code also
 *      slightly randomizes the overridden vehicle heading to test the
 *      approach heading threshold detection. This code could be leveraged
 *      for other purposes where processing data from a moving vehicle is
 *      required for testing. As-is it is pretty specific to the GS2
 *      demo as it uses the GS2Demo parameters from the spat.conf file for
 *      setup of the faked position.
 *
 *      Note also that this GS2 demo is bypassing much of the intersection
 *      heading matching. We encountered inconsistencies in matching the
 *      approach headings in actual tests with a vehicle, even though the
 *      lane width was made very large and we reduced the number of
 *      approaches. Something in the logic is jumping between different
 *      approaches for successive tests with the same instance of i2v and
 *      the same vehicle. So for now the code does a minimum check and
 *      forces the vehicle approach to be the demo approach.
 *
 *      20170605: JJG -- more modifications to support demos; this one with
 *      TrafficCast.  The new functionality will simply listen for BSMs and
 *      forward them to a configured IP.  The data will remain ASN1 encoded
 *      and is not checked for validity.
 *
 */
#include <sys/types.h>
#include <signal.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <string.h>
#include <math.h>
#include "wsu_shm.h"
#include "i2v_general.h"
#include "i2v_util.h"
#include "i2v_shm_master.h"
#include "spat.h"
#if defined(J2735_2016)
#include "DSRC.h"
#elif(J2735_2023)
#include "MessageFrame.h"
#include "Common.h"
#include "ProbeVehicleData.h"
#include "BasicSafetyMessage.h"
#include "PersonalSafetyMessage.h" /* 1 or 2? */
#else
#ERROR: You must define  J2735_2016 or J2735_2023 in v2xcommon.mk
#endif
#include "ntcip-1218.h"
#include "v2v_general.h"
#include "shm_tps.h"
#if defined(MY_UNIT_TEST)
#include "stubs.h"
#endif

/* Mild amount of DEBUG. */
/*#define EXTRA_DEBUG*/
/* Intense amount of DEBUG, be careful enabling. */
/*#define EXTRA_EXTRA_DEBUG*/

#if defined(EXTRA_DEBUG)
#define OUTPUT_MODULUS  30  /* Seconds. */
#else
#define OUTPUT_MODULUS  1200
#endif

#define MY_NAME  "bsmrcv"
#define MY_ERR_LEVEL LEVEL_PRIV

/* the values below are a bitmask designed to avoid current configs to have
   to change; LTE was added for BSM forwarding but should be turn on for
   platforms that support it; not by default; without adding a new config
   param just use existing, ETH will be default when enabled */

extern spatCfgItemsT   spat_cfg;
extern bool_t mainloop;
extern cfgItemsT cfg;

/* Limits forwarding debug output. */
int32_t busyLogger = 5;
extern mqd_t fwdmsgfd;

fwdmsgData  bsmpsmFwdmsgData; 
char        bsmpsmSendBuf[sizeof(bsmpsmFwdmsgData)];
fwdmsgData  bsmFwdmsgData; 
char        bsmSendBuf[sizeof(bsmFwdmsgData)];
extern bool_t  bsmForward;
extern pthread_mutex_t fwdmsg_send_lock;

extern uint32_t spat_tx_count;
extern uint32_t spat_tx_err_count;
extern uint64_t my_error_states;
extern void set_my_error_state(wint32 my_error);

#if defined(BSM_PSM_FORWARDING)
extern i2vShmMasterT * shm_ptr;
struct sockaddr_in bsmpsmdest;
#define VEHID_SIZE  (4)
#define MPS_KPH(a)  ((a)*3.6)
#define BSM_PACKET  (0x21)
#define PSM_PACKET  (0x22)
OSCTXT bsmpsmCtxt;

typedef struct {
    unsigned char       personalDeviceUserType;     // PSM Only 
    unsigned char       msgCnt;
    unsigned char       vehID[VEHID_SIZE];
    unsigned long long  utc_millisec;
    unsigned short int  secMarkMs;
    double              latitudeDeg;
    double              longitudeDeg;
    float               elevationM;
    unsigned char       transmissionState;
    float               vehSpeedKph;
    float               headingDeg;
    float               steeringWhlAngle;
    float               longAccel;
    float               yawRate;
    unsigned char       brakeActive;
    unsigned char       tractionCtl;
    unsigned char       ABS;
    unsigned char       SCS;
}__attribute__((packed))  bsmpsmDataT;

typedef struct {
    unsigned char       msgType;
    bsmpsmDataT         bsmpsmData;
} __attribute__((packed))  bsmpsmPacketT;
#endif

STATIC void bsmFwdProcess(inWSMType *wsm);
STATIC void bsmRxProcess(inWSMType *wsm);

uint64_t time_utc_msecs = 0; /* mainloop will update ~once a second. */
STATIC uint32_t rx_msg_connection_reset_count = 0;
STATIC uint32_t rx_msg_write_count = 0;
STATIC uint32_t rx_msg_write_error_count = 0;

/* 1218 MIB: 5.12 rsuMessageStats. */
messageStats_t messageStats[RSU_PSID_TRACKED_STATS_MAX];
STATIC messageStats_t * shm_messageStats_ptr = NULL; /* SHM. */

/* 1218 MIB: 5.13 rsuSystemStats. */
STATIC CommRangeStats_t CommRangeStats[RSU_COMM_RANGE_MAX];
STATIC uint32_t commRangeBsmCount = 0x0;
STATIC float64_t RSU_lat = 0.0; /* Thread will update peridocally for when BSM comes in.*/
STATIC float64_t RSU_lon = 0.0;

static uint32_t bsm_extract_lle_fail = 0x0;

void Init_BSMRX_Statics(void)
{
#if defined(BSM_PSM_FORWARDING)
    memset(&bsmpsmdest,0,sizeof(bsmpsmdest));
#endif
    commRangeBsmCount = 0x0;
    RSU_lat = 0.0;
    RSU_lon = 0.0;
    rx_msg_connection_reset_count = 0;
    rx_msg_write_count = 0;
    rx_msg_write_error_count = 0;
    time_utc_msecs = 0;
    memset(&bsmpsmFwdmsgData,0x0,sizeof(bsmpsmFwdmsgData));
    memset(bsmpsmSendBuf,0x0,sizeof(bsmpsmFwdmsgData));
    memset(&bsmFwdmsgData,0x0,sizeof(bsmFwdmsgData));
    memset(bsmSendBuf,0x0,sizeof(bsmSendBuf));
    bsm_extract_lle_fail = 0x0;
    busyLogger = 5;
}

STATIC void bsmFwdProcess(inWSMType *wsm)
{
    /* at this point, send data over established port */
    pthread_mutex_lock(&fwdmsg_send_lock);
    memset(&bsmFwdmsgData,0,sizeof(fwdmsgData));
    bsmFwdmsgData.fwdmsgType = BSM;
    memcpy(bsmFwdmsgData.fwdmsgData,&wsm->data,wsm->dataLength);
    bsmFwdmsgData.fwdmsgDataLen = wsm->dataLength;

    memcpy(bsmSendBuf,&bsmFwdmsgData,sizeof(fwdmsgData));

    if ( -1 == mq_send(fwdmsgfd, bsmSendBuf,sizeof(fwdmsgData),DEFAULT_MQ_PRIORITY))
    {
        set_my_error_state(SPAT16_FWDMSG_MQ_SEND_ERROR);
    }
    if (busyLogger) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Forwarding %d bytes to fwdmsg process \n", wsm->dataLength);
        busyLogger--;
    }
    pthread_mutex_unlock(&fwdmsg_send_lock);

    #if defined(ENABLE_BSM_RX_DEBUG)

    if(0 == (bsmFwdProcessCount % 1000)) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"BSMrx16(%d,%d)::Forwarding(b,r,p,c)(%d, %d, %d, %d) to %s %d\n",bsm16_rx_err_count
                                                          , bsmFwdProcessCount
                                                          , wsm->dataLength
                                                          , wsm->radioNum 
                                                          , wsm->psid
                                                          , wsm->channelNumber
                                                          , spat_cfg.bsmFwdIp
                                                          , spat_cfg.bsmFwdPort);
    }

    bsmFwdProcessCount++;

    #endif /* ENABLE_BSM_RX_DEBUG */

}

STATIC void bsmRxProcess(inWSMType *wsm)
{
    if (bsmForward) {
        /* for simplicity, no other processing supported with forwarding;
           however this will likely be revisited in the future */
        if (bsmForward & BSMFWD_ETH) {
            bsmFwdProcess(wsm);
        }
    } else {
        set_my_error_state(SPAT16_BSM_FWD_DROPPED); /* Friendly warning: Precious BSMs not being FWD. */
    }
    return;
}
/*
 * BSM_ExtractJ2735_LLE: Extract the lat, lon, & elev from BSM.
 *
 * Input parameters:
 *             latDeg: Pointer to location to store the latitude in units of
 *                     degrees * 1e7.
 *             lonDeg: Pointer to location to store the longitude in units of
 *                     degrees * 1e7.
 *             elevM: Pointer to location to store the elevation in units of
 *                    meters * 10.
 *                    0x0000 to 0xefff represents 0 to 6143.9.
 *                    0xf001 to 0xffff represents -409.5 to -0.1.
 *                    0xf000 represents unknown.
 *             bsmData: Pointer to the BSM data.
 *             bsmDataLen: BSM data length.
 * Output parameters:
 *             Latitude, longitude, and elevation written to locations pointed
 *             to by latDeg, lonDeg, and elevM respectively.
 * Return Value:  SUCCESS on Success BSM_DECODE_FAILED on failure.
 */
STATIC int32_t BSM_ExtractJ2735_LLE(int32_t *latDeg, int32_t *lonDeg, uint16_t *elevM,const uint8_t *bsmData, uint16_t bsmDataLen)
{
    int32_t  lat;
    int32_t  lon;
    uint16_t elev;

    /* PER_U Unblobbed ONLY */
    uint32_t ulat;
    uint32_t ulon;
    uint8_t  uperBSMBOI = (bsmData[2] & 0x80) ? 1 : 0;

    if (bsmDataLen < (uperBSMBOI + 21)) {
        #if defined(EXTRA_DEBUG)
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"BSM_ExtractJ2735_LLE: Decode fail: len=%u.\n", bsmDataLen);
        #endif
        set_my_error_state(SPAT16_BSM_DECODE_FAILED);
        bsm_extract_lle_fail++;
        return SPAT16_BSM_DECODE_FAILED;
    }

    /* Signed values are encoded subtracting lower bound value */
    /* Encoded as 31 bit unsigned value (fit the range) for latitude */
    /* So use uint32_t to copy the bits and then >> by 1 */
    ulat = (              (((bsmData[10 + uperBSMBOI] & 0x3F) << 2) |
                           ((bsmData[11 + uperBSMBOI] & 0xC0) >> 6)));
    ulat = ((ulat << 8) | (((bsmData[11 + uperBSMBOI] & 0x3F) << 2) |
                           ((bsmData[12 + uperBSMBOI] & 0xC0) >> 6)));
    ulat = ((ulat << 8) | (((bsmData[12 + uperBSMBOI] & 0x3F) << 2) |
                           ((bsmData[13 + uperBSMBOI] & 0xC0) >> 6)));
    ulat = ((ulat << 8) | (((bsmData[13 + uperBSMBOI] & 0x3F) << 2) |
                               ((bsmData[14 + uperBSMBOI] & 0x80) >> 6)));
    ulat >>= 1;
    lat = ulat - 900000000;

    ulon = (              (((bsmData[14 + uperBSMBOI] & 0x7F) << 1) |
                           ((bsmData[15 + uperBSMBOI] & 0x80) >> 7)));
    ulon = ((ulon << 8) | (((bsmData[15 + uperBSMBOI] & 0x7F) << 1) |
                           ((bsmData[16 + uperBSMBOI] & 0x80) >> 7)));
    ulon = ((ulon << 8) | (((bsmData[16 + uperBSMBOI] & 0x7F) << 1) |
                           ((bsmData[17 + uperBSMBOI] & 0x80) >> 7)));
    ulon = ((ulon << 8) | (((bsmData[17 + uperBSMBOI] & 0x7F) << 1) |
                           ((bsmData[18 + uperBSMBOI] & 0x80) >> 7)));
    lon = ulon - 1799999999;

    elev = (              (((bsmData[18 + uperBSMBOI] & 0x7F) << 1) |
                           ((bsmData[19 + uperBSMBOI] & 0x80) >> 7)));
    elev = ((elev << 8) | (((bsmData[19 + uperBSMBOI] & 0x7F) << 1) |
                           ((bsmData[20 + uperBSMBOI] & 0x80) >> 7)));
    elev -= 4096;
 
    if (lat == 900000001) {
        lat = -900000001;
    }
    else if (lon == 1800000001) {
        lon = -1800000001;
    }

    *latDeg = lat;
    *lonDeg = lon;
    *elevM  = elev;
    return SPAT16_AOK;
}
/*
 * 0.0..359.99 East of North in North America only, Not global solution.
 * Return of 360.1 is an error indicator.
 * This gives the RSU bearing to OBU which then makes sector search of OBU easy.
 */
STATIC float64_t getBearing(float64_t lat1, float64_t lon1, float64_t lat2, float64_t lon2)
{
  float64_t delta_lon, tc1;

  if ((fabs(lat1) > 90.0) || (fabs(lat2) > 90.0)) {
      #if defined(EXTRA_EXTRA_DEBUG)
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Lat must be between -90 and 90 degrees (% .7f)(% .7f).\n",lat1,lat2);
      #endif
      set_my_error_state(SPAT16_BSM_BAD_LOCATION);
      return 360.1;
  }
  if ((fabs(lon1) > 180.0) || (fabs(lon2) > 180.0)) {
      #if defined(EXTRA_EXTRA_DEBUG)
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Lon must be between -180 and 180 degrees (% .7f)(% .7f).\n", lon1,lon2);
      #endif
      set_my_error_state(SPAT16_BSM_BAD_LOCATION);
      return 360.1;
  }
  if ((lat1 == lat2) && (lon1 == lon2)) {
    #if defined(EXTRA_EXTRA_DEBUG)
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"BSM and RSU in same spot.\n");
    #endif
    set_my_error_state(SPAT16_BSM_RSU_SAME_LOCATION); /* Could happen. */
    return 0.0; /* In the same spot so who knows? */
  }
  delta_lon = DEG_TO_RAD(lon2 - lon1);
  tc1 = fmod(atan2(sin(delta_lon)*cos(DEG_TO_RAD(lat2)),cos(DEG_TO_RAD(lat1))*sin(DEG_TO_RAD(lat2))-sin(DEG_TO_RAD(lat1))*cos(DEG_TO_RAD(lat2))*cos(delta_lon)),2*M_PI);
  tc1 = RAD_TO_DEG(tc1);
  tc1 -= 180.0;
  if(tc1 < 0.0) 
      tc1 = 360.0+tc1;

  return tc1; /* degrees from East of true North. */
}

#define UTIL_EARTH_MEAN_RADIUS_M   (6371.0 * 1000.0)
/*******************************************************************************
 *
 *  Function :  UtilCalcRangeHaversine
 *
 *  Purpose  :  Calculates range from a reference point (ref) to another 
 *                  point (p) using the haversine algorithm
 *
 *  Argument :  ref_lat_deg       Reference latitude in degrees
 *              ref_long_deg      Reference longtitude in degrees
 *              ref_heading_deg   Heading in degrees (unused)
 *              p_lat_deg         Point p latitude in degrees
 *              p_long_deg        Point p longtitude in degrees
 *                
 *  Output   :  None
 *
 *  Return   :  Range in meters
 *
 *  Notes    :  haversine is well conditioned for numerical computation.
 *
 *******************************************************************************/
STATIC float64_t UtilCalcRangeHaversine(float64_t ref_lat_deg, float64_t ref_long_deg, float64_t ref_heading_deg, float64_t p_lat_deg, float64_t p_lon_deg )
{
    float64_t refLatRad;
    float64_t pLatRad;
    float64_t dLatRad;
    float64_t dLongRad;
    float64_t halfChordSqrdRad;
    float64_t angularDistanceRad;
    float64_t rangeM = 0.0;

    ref_heading_deg = ref_heading_deg; /* By definition, not used. */

    dLatRad = DEG_TO_RAD((p_lat_deg - ref_lat_deg));
    dLongRad = DEG_TO_RAD((p_lon_deg - ref_long_deg));
    refLatRad = DEG_TO_RAD(ref_lat_deg);
    pLatRad = DEG_TO_RAD(p_lat_deg);

    halfChordSqrdRad = ( sin(dLatRad / 2.0) * sin(dLatRad / 2.0)) +
        (( sin(dLongRad / 2.0) * sin(dLongRad / 2.0) * 
        ( cos(refLatRad) * cos(pLatRad))));

    angularDistanceRad = 2 * atan2(sqrt(halfChordSqrdRad), sqrt(1 - halfChordSqrdRad));

    rangeM = angularDistanceRad * UTIL_EARTH_MEAN_RADIUS_M;

    return rangeM;
}

/* Take RSU bearing to BSM and calculate sector BSM is in. */
STATIC int32_t BSM_bearingToSector(float64_t BSM_bearing)
{
  if((BSM_bearing < 0.0) || (360.0 <= BSM_bearing)){
      return 17; /* This sector indicates an error. */
  }
  if(BSM_bearing < 22.5) { /* 0.0..22.49. */
      return 1;
  } else { 
      return ((BSM_bearing/22.5) + 1) ; /* 22.5 .. 359.99 */
  }
}

#if defined(EXTRA_DEBUG)
STATIC void print_wsm(inWSMType *wsm)
{
  int32_t i;
  uint8_t data[MAX_WSM_DATA];
  static uint32_t rolling_counter = 0x0;

  if(0 == (rolling_counter % 100)){
      memset(data,'\0',MAX_WSM_DATA);
      for(i=0; (i < MAX_WSM_DATA) && (i < wsm->dataLength); i++) {
          data[i] = wsm->data[i];
      }
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"BSM_ExtractJ2735_LLE: Len=%d.\n",wsm->dataLength);
      for(i=0; (i < MAX_WSM_DATA) && (i < MAX_WSM_DATA); i += 16) {
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"%02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X\n",
              data[i + 0],data[i + 1],data[i + 2],data[i + 3]  
              ,data[i + 4],data[i + 5],data[i + 6],data[i + 7]
              ,data[i + 8],data[i + 9],data[i + 10],data[i + 11]
              ,data[i + 12],data[i + 13],data[i + 14],data[i + 15]);
          if(wsm->dataLength <= i+1)
              break;
      }
  }
  rolling_counter++;
}
#endif
/* TODO: FilterType, FileterValue not used yet. */
STATIC void commRange_tallyBsm(inWSMType *wsm)
{
  int32_t   ret = SPAT16_AOK;
  int32_t   i = 0;
  int32_t   BSM_latDeg = 0; 
  int32_t   BSM_lonDeg = 0;
  float64_t BSM_lat = 0.0; 
  float64_t BSM_lon = 0.0;
  float64_t BSM_bearing = 0.0;
  int32_t   BSM_distance = 0;
  int32_t   BSM_sector = 0;
  uint16_t  BSM_elevM = 0;

  if(NULL == wsm){
      #if defined(EXTRA_DEBUG)
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"BSM_ExtractJ2735_LLE: wsm NULL input.\n");
      #endif
      ret = SPAT16_BAD_INPUT;
  } else {
      if(0 == wsm->dataLength) {
          #if defined(EXTRA_DEBUG)
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"BSM_ExtractJ2735_LLE: BSM is zero length.\n"); /* Keep going, don't care. */
          #endif
          ret = SPAT16_BSM_ZERO_LENGTH;
      }
      if(SPAT16_AOK == ret){
          #if defined(EXTRA_DEBUG)
          print_wsm(wsm);
          #endif
          if(ASN1V_basicSafetyMessage != wsm->data[1]) {
              #if defined(EXTRA_DEBUG)
              I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"BSM_ExtractJ2735_LLE: BSM MsgId is wrong=%u.\n", wsm->data[1]); /* Keep going, don't care. */
              #endif
              ret = SPAT16_BSM_DSRC_MSG_ID_BAD;
          }
      }
      if(SPAT16_AOK == ret){
          if(SPAT16_AOK == (ret = BSM_ExtractJ2735_LLE(&BSM_latDeg, &BSM_lonDeg, &BSM_elevM, wsm->data,wsm->dataLength))){
              /* Calculate bearing and distance. */
              BSM_lat = (float64_t)(BSM_latDeg)/(float64_t)(1.0e7);
              BSM_lon = (float64_t)(BSM_lonDeg)/(float64_t)(1.0e7);
              /* OBU is ref point and RSU is to point. */
              BSM_distance = (int32_t)UtilCalcRangeHaversine(BSM_lat, BSM_lon, 0.0, RSU_lat, RSU_lon);
              if(BSM_distance < RSU_COMM_RANGE_MINUTES_MIN) {
                  #if defined(EXTRA_DEBUG)
                  I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"RSU_COMM_RANGE_MINUTES_MIN warning.\n"); /* Keep going, don't care. */
                  #endif
                  set_my_error_state(SPAT16_BSM_DISTANCE_WARNING);
                  BSM_distance = abs(BSM_distance); /* 2001 == unknown. */
              }
              /* Normalize. Anything equal or greater. */
              if((RSU_COMM_RANGE_MINUTES_MAX - 1) < BSM_distance) {
                  BSM_distance = RSU_COMM_RANGE_MINUTES_MAX - 1; /* peg at 2000 metres. */
                  #if defined(EXTRA_DEBUG)
                  I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"RSU_COMM_RANGE_MINUTES_MAX warning.\n"); /* Keep going, don't care. */
                  #endif
                  set_my_error_state(SPAT16_BSM_DISTANCE_WARNING);
              }
              /* OBU is x1,y1 RSU is x2,y2. */
              BSM_bearing = getBearing(BSM_lat, BSM_lon, RSU_lat, RSU_lon);
              if((BSM_bearing < 0.0) || (360.0 <= BSM_bearing)){
                  #if defined(EXTRA_DEBUG)
                  I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"SPAT16_BSM_BEARING_WARNING=%lf.\n",BSM_bearing); /* Keep going, don't care. */
                  #endif
                  set_my_error_state(SPAT16_BSM_BEARING_WARNING);
                  BSM_bearing = 360.1;
                  BSM_sector  = 17; /* This BSM won't get counted now. */
              } else {
                  BSM_sector = BSM_bearingToSector(BSM_bearing);
              }
              #if defined(EXTRA_DEBUG)
              {
                  static uint32_t rolling_counter = 0x0;
                  if(0 == (rolling_counter % 100)) {
                      I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"BSM bear/sector/dist(M)=(%lf,%d,%d): BSM(%lf,%lf) RSU(%lf,%lf)\n",
                          BSM_bearing, BSM_sector, BSM_distance,BSM_lat, BSM_lon, RSU_lat, RSU_lon);
                  }
                  rolling_counter++;
              }
              #endif
              /* 
               * No distance means we are in the same spot and Sector can't be known. Assume sector 1 for now.
               * Now cycle through table. See if any row is active, matches sector, and matches msg id for BSM.
               */
              for(i=0;(i<RSU_COMM_RANGE_MAX) && (CommRangeStats[i].CommRangeStatus == SNMP_ROW_ACTIVE);i++) {
                  if(   (CommRangeStats[i].CommRangeSector == BSM_sector)
                     && (ASN1V_basicSafetyMessage == CommRangeStats[i].CommRangeMsgId)) {
                     /* Find max and update. */
                     if(CommRangeStats[i].CommRange1Min_max < BSM_distance){
                         CommRangeStats[i].CommRange1Min_max = BSM_distance;
                     }
                     if(CommRangeStats[i].CommRange5Min_max < BSM_distance){
                         CommRangeStats[i].CommRange5Min_max = BSM_distance;
                     }
                     if(CommRangeStats[i].CommRange15Min_max < BSM_distance){
                         CommRangeStats[i].CommRange15Min_max = BSM_distance;
                     }
                     /* For average, inc count and add to total. */
                     CommRangeStats[i].CommRange1Min_count++; /* Average is total divided by count. */
                     CommRangeStats[i].CommRange5Min_count++;
                     CommRangeStats[i].CommRange15Min_count++;
                     CommRangeStats[i].CommRange1Min_total  += BSM_distance;
                     CommRangeStats[i].CommRange5Min_total  += BSM_distance;
                     CommRangeStats[i].CommRange15Min_total += BSM_distance;
                  }
              } /* for */
          }
          #if defined(EXTRA_DEBUG)
          else {
              I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"BSM_ExtractJ2735_LLE: failed.\n"); /* Keep going, don't care. */
          }
          #endif
      }
  }
  if(ret < SPAT16_AOK) {
      set_my_error_state(ret);
  }
}

STATIC void reset_commRange_stats(CommRangeStats_t * shm_commRange, CommRangeStats_t * commRangeStats)
{
  if((NULL == shm_commRange) || (NULL == commRangeStats)) {
      set_my_error_state(SPAT16_BAD_INPUT);
  } else {

  shm_commRange->CommRange1Min_max = 0;
  shm_commRange->CommRange5Min_max = 0;
  shm_commRange->CommRange15Min_max = 0;
  shm_commRange->CommRangeAvg1Min = 0;
  shm_commRange->CommRangeAvg5Min = 0;
  shm_commRange->CommRangeAvg15Min = 0;
  shm_commRange->CommRange1Min_count = 0;
  shm_commRange->CommRange5Min_count = 0;
  shm_commRange->CommRange15Min_count = 0;
  shm_commRange->CommRange1Min_total = 0;
  shm_commRange->CommRange5Min_total = 0;
  shm_commRange->CommRange15Min_total = 0;

  commRangeStats->CommRange1Min_max = 0;
  commRangeStats->CommRange5Min_max = 0;
  commRangeStats->CommRange15Min_max = 0;
  commRangeStats->CommRangeAvg1Min = 0;
  commRangeStats->CommRangeAvg5Min = 0;
  commRangeStats->CommRangeAvg15Min = 0;
  commRangeStats->CommRange1Min_count = 0;
  commRangeStats->CommRange5Min_count = 0;
  commRangeStats->CommRange15Min_count = 0;
  commRangeStats->CommRange1Min_total = 0;
  commRangeStats->CommRange5Min_total = 0;
  commRangeStats->CommRange15Min_total = 0;

  /* 
   * Tells mainloop when to post results and reset for next test epoch.
   * mainloop will reset time after posting results and clearing stats.
   */
  commRangeStats->CommRange1Min_time  = time_utc_msecs + (1  * 60000);
  commRangeStats->CommRange5Min_time  = time_utc_msecs + (5  * 60000);
  commRangeStats->CommRange15Min_time = time_utc_msecs + (15 * 60000);
  }
}

/* callback function to ris services for wsm receive */
//TODO: Add check of DSRC Msg ID
//TODO: Dump wsms to circular buffer for processing and let radio handler return. Getting too big.
void bsmRxHandler(inWSMType *wsm)
{
  int32_t i;

    if(WFALSE == mainloop) /* At shutdown maybe we still get called if CB not de-reg in time? So dbl check. */
        return;

    if(NULL == wsm) {
        set_my_error_state(SPAT16_BSMRX_NULL_WSM); /* Trust but verify radio stack. */
    } else {
        if(cfg.bsmPsidPerU == wsm->psid) {
            #if defined(EXTRA_DEBUG)
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"BSM rx. Count=%u\n",commRangeBsmCount);
            #endif
            commRangeBsmCount++;
            commRange_tallyBsm(wsm);
            bsmRxProcess(wsm);

            for(i=0;i<RSU_PSID_TRACKED_STATS_MAX;i++){
                if((wsm->psid == messageStats[i].psid) && (SNMP_ROW_ACTIVE == messageStats[i].rowStatus)) {
                    messageStats[i].count++;
                }
            }

        } else {
            set_my_error_state(SPAT16_BSMRX_PSID_MISMATCH); /* We reg for only one PSID so this ideally does not happen. */
        }
    }
}

#if defined(BSM_PSM_FORWARDING)
/* Little endian or not */
STATIC bool_t littleEndian(void)
{
    unsigned int i = 0x1;
    unsigned char *p = (unsigned char *)&i;
    return (p[0] == 1);
}

// swap the bytes in a U2
STATIC wuint16 swapU2(wuint16 i)
{
    wuint16 o;
    unsigned char* po = (unsigned char *)&o;
    unsigned char* pi = (unsigned char *)&i;
    po[0] = pi[1];
    po[1] = pi[0];

    return o;
}

/* swap the bytes in a float */
STATIC float swapf(float f)
{
    float a;
    unsigned char *dst = (unsigned char *)&a;
    unsigned char *src = (unsigned char *)&f;
    dst[0] = src[3];
    dst[1] = src[2];
    dst[2] = src[1];
    dst[3] = src[0];

    return a;
}

/* swap bytes in an unsigned long long */
STATIC wulong64 swapu64(wulong64 ul)
{
    wulong64 a;
    unsigned char *dst = (unsigned char *)&a;
    unsigned char *src = (unsigned char *)&ul;

    dst[0] = src[7];
    dst[1] = src[6];
    dst[2] = src[5];
    dst[3] = src[4];
    dst[4] = src[3];
    dst[5] = src[2];
    dst[6] = src[1];
    dst[7] = src[0];

    return a;
}

/* swap bytes in a double */
STATIC double swapd(double d)
{
    double a;
    unsigned char *dst = (unsigned char *)&a;
    unsigned char *src = (unsigned char *)&d;
    dst[0] = src[7];
    dst[1] = src[6];
    dst[2] = src[5];
    dst[3] = src[4];
    dst[4] = src[3];
    dst[5] = src[2];
    dst[6] = src[1];
    dst[7] = src[0];

    return a;
}

// Reorder the bytes for char string copied from unsigned int.
// This is a special handling when copying from an unsigned int
// to a unsigned char array for little endian. 
STATIC void swapU32Char(wuint8 *i)
{
    wuint8 temp[VEHID_SIZE];

    memcpy(&temp,i,sizeof(VEHID_SIZE));

    *i++    = temp[3];
    *i++    = temp[2];
    *i++    = temp[1];
    *i      = temp[0];
}

/* Puts bytes in the BigEndian format for a LittleEndian Machine */
STATIC void swapBsmPsmPacket(bsmpsmPacketT *psmToSend)
{
    swapU32Char(psmToSend->bsmpsmData.vehID);
    psmToSend->bsmpsmData.secMarkMs = swapU2(psmToSend->bsmpsmData.secMarkMs);
    psmToSend->bsmpsmData.utc_millisec = swapu64(psmToSend->bsmpsmData.utc_millisec);
    psmToSend->bsmpsmData.latitudeDeg  = swapd(psmToSend->bsmpsmData.latitudeDeg);
    psmToSend->bsmpsmData.longitudeDeg  = swapd(psmToSend->bsmpsmData.longitudeDeg);
    psmToSend->bsmpsmData.elevationM  = swapf(psmToSend->bsmpsmData.elevationM);
    psmToSend->bsmpsmData.headingDeg  = swapf(psmToSend->bsmpsmData.headingDeg);
    psmToSend->bsmpsmData.vehSpeedKph  = swapf(psmToSend->bsmpsmData.vehSpeedKph);
    psmToSend->bsmpsmData.steeringWhlAngle  = swapf(psmToSend->bsmpsmData.steeringWhlAngle);
    psmToSend->bsmpsmData.longAccel  = swapf(psmToSend->bsmpsmData.longAccel);
    psmToSend->bsmpsmData.yawRate  = swapf(psmToSend->bsmpsmData.yawRate);
}

/* Parse BSM */
STATIC void parseBSM(BasicSafetyMessage *bsm, bsmpsmPacketT *bsmToSend)
{
    wuint16 uShortVal;
    wint16  shortVal;
    wint32  intVal;
    wint8   charVal;
    wuint8  brkRevBits;
    wuint8  brkAct;
    wuint16 size;

    memset(bsmToSend, 0, sizeof(bsmpsmPacketT));

    bsmToSend->msgType = BSM_PACKET;
    bsmToSend->bsmpsmData.utc_millisec = i2vUtilGetTimeInMs();
#if defined(J2735_2016)
    bsmToSend->bsmpsmData.msgCnt = bsm->coreData.msgCnt;

    memcpy(bsmToSend->bsmpsmData.vehID, bsm->coreData.id.data,bsm->coreData.id.numocts);

    uShortVal = bsm->coreData.secMark;
    bsmToSend->bsmpsmData.secMarkMs = uShortVal;

    /* Latitude */
    intVal = bsm->coreData.lat;
    if (intVal != 900000001)    {
        bsmToSend->bsmpsmData.latitudeDeg = intVal / 1e7;
    }

    /* Longitude */
    intVal = bsm->coreData.long_;
    if (intVal != 1800000001)    {
        bsmToSend->bsmpsmData.longitudeDeg = intVal / 1e7;
    }

    /* Elevation */
    uShortVal = bsm->coreData.elev;
    if (uShortVal != 0xF000)    {
        if (uShortVal > 0xF000) {
            shortVal = uShortVal; /* negative values */
            bsmToSend->bsmpsmData.elevationM = shortVal / 10.0;
        }
        else {
            bsmToSend->bsmpsmData.elevationM = uShortVal / 10.0;
        }
    }

    bsmToSend->bsmpsmData.transmissionState = bsm->coreData.transmission;

    /* Speed */
    uShortVal = bsm->coreData.speed;
    if (uShortVal != 8191)   {
        bsmToSend->bsmpsmData.vehSpeedKph = uShortVal * 0.02;
        bsmToSend->bsmpsmData.vehSpeedKph = MPS_KPH(bsmToSend->bsmpsmData.vehSpeedKph);
    }

    /* Heading */
    uShortVal = bsm->coreData.heading;
    if (uShortVal != 28800) {
        bsmToSend->bsmpsmData.headingDeg = uShortVal * 0.0125;
    }

    /* Angle */ 
    charVal = bsm->coreData.angle;
    if (charVal !=127)  {
        bsmToSend->bsmpsmData.steeringWhlAngle = charVal * 1.5;
    }

    /* LongAccel */
    shortVal = bsm->coreData.accelSet.long_;
    if (shortVal != 2001)   {
        bsmToSend->bsmpsmData.longAccel = shortVal * 0.01;
    }

    /* YawRate */
    shortVal = bsm->coreData.accelSet.yaw;
    if (shortVal != 32767)  {
        bsmToSend->bsmpsmData.yawRate = shortVal * 0.01;
    }

    /* Brake Active */
    brkRevBits = brkAct = bsm->coreData.brakes.wheelBrakes.data[0];
    /* The brake bits are 5 bits from MSB. Reverse bits */
    size = sizeof(brkRevBits) * 8 - 1;

    for (brkRevBits >>= 1 ; brkRevBits; brkRevBits >>= 1) {
        brkAct <<= 1;
        brkAct |= brkRevBits & 1;
        size--;
    }

    brkAct <<= size;
    /* Check if any of the bits are valid */
    bsmToSend->bsmpsmData.brakeActive = (brkAct & 0x1E) ? 1 : 0;

    /* TCS */
    bsmToSend->bsmpsmData.tractionCtl = bsm->coreData.brakes.traction;
    if (bsmToSend->bsmpsmData.tractionCtl == 1)    {
        bsmToSend->bsmpsmData.tractionCtl = 0;
    }
    else if ((bsmToSend->bsmpsmData.tractionCtl == 2) || (bsmToSend->bsmpsmData.tractionCtl ==3))    {
        bsmToSend->bsmpsmData.tractionCtl = 1;
    }

    bsmToSend->bsmpsmData.ABS = bsm->coreData.brakes.abs_;
    if (bsmToSend->bsmpsmData.ABS == 1)    {
        bsmToSend->bsmpsmData.ABS = 0;
    }
    else if ((bsmToSend->bsmpsmData.ABS == 2) || (bsmToSend->bsmpsmData.ABS ==3))    {
        bsmToSend->bsmpsmData.ABS = 1;
    }

    bsmToSend->bsmpsmData.SCS = bsm->coreData.brakes.scs;
    if (bsmToSend->bsmpsmData.SCS == 1)    {
        bsmToSend->bsmpsmData.SCS = 0;
    }
    else if (bsmToSend->bsmpsmData.SCS ==3)    {
        bsmToSend->bsmpsmData.SCS = 1;
    }

    /* Add the message type to the packet */
    bsmToSend->msgType = BSM_PACKET;
#else //(J2735_2023)
    if(NULL != bsm->coreData){
        bsmToSend->bsmpsmData.msgCnt = bsm->coreData->msgCnt;
        memcpy(bsmToSend->bsmpsmData.vehID, bsm->coreData->id.data,bsm->coreData->id.numocts);
        bsmToSend->bsmpsmData.secMarkMs = bsm->coreData->secMark;
        /* Latitude */
        intVal = bsm->coreData->lat;
        if (intVal != 900000001)    {
            bsmToSend->bsmpsmData.latitudeDeg = intVal / 1e7;
        }
        /* Longitude */
        intVal = bsm->coreData->long_;
        if (intVal != 1800000001)    {
            bsmToSend->bsmpsmData.longitudeDeg = intVal / 1e7;
        }
        /* Elevation */
        uShortVal = bsm->coreData->elev;
        if (uShortVal != 0xF000)    {
            if (uShortVal > 0xF000) {
                shortVal = uShortVal; /* negative values */
                bsmToSend->bsmpsmData.elevationM = shortVal / 10.0;
            } else {
                bsmToSend->bsmpsmData.elevationM = uShortVal / 10.0;
            }
        }
        bsmToSend->bsmpsmData.transmissionState = bsm->coreData->transmission;
        /* Speed */    
        uShortVal = bsm->coreData->speed;
        if (uShortVal != 8191)   {
            bsmToSend->bsmpsmData.vehSpeedKph = uShortVal * 0.02;
            bsmToSend->bsmpsmData.vehSpeedKph = MPS_KPH(bsmToSend->bsmpsmData.vehSpeedKph);
        }
        /* Heading */
        uShortVal = bsm->coreData->heading;
        if (uShortVal != 28800) {
            bsmToSend->bsmpsmData.headingDeg = uShortVal * 0.0125;
        }
        /* Angle */ 
        charVal = bsm->coreData->angle;
        if (charVal !=127)  {
            bsmToSend->bsmpsmData.steeringWhlAngle = charVal * 1.5;
        }
        /* LongAccel */
        shortVal = bsm->coreData->accelSet.long_;
        if (shortVal != 2001)   {
            bsmToSend->bsmpsmData.longAccel = shortVal * 0.01;
        }
        /* YawRate */
        shortVal = bsm->coreData->accelSet.yaw;
        if (shortVal != 32767)  {
            bsmToSend->bsmpsmData.yawRate = shortVal * 0.01;
        }
        /* Brake Active */
        brkRevBits = brkAct = bsm->coreData->brakes.wheelBrakes.data[0];
        /* The brake bits are 5 bits from MSB. Reverse bits */
        size = sizeof(brkRevBits) * 8 - 1;
        for (brkRevBits >>= 1 ; brkRevBits; brkRevBits >>= 1) {
            brkAct <<= 1;
            brkAct |= brkRevBits & 1;
            size--;
        }
        brkAct <<= size;
        /* Check if any of the bits are valid */
        bsmToSend->bsmpsmData.brakeActive = (brkAct & 0x1E) ? 1 : 0;
        /* TCS */
        bsmToSend->bsmpsmData.tractionCtl = bsm->coreData->brakes.traction;
        if (bsmToSend->bsmpsmData.tractionCtl == 1)    {
            bsmToSend->bsmpsmData.tractionCtl = 0;
        } else if ((bsmToSend->bsmpsmData.tractionCtl == 2) || (bsmToSend->bsmpsmData.tractionCtl ==3))    {
            bsmToSend->bsmpsmData.tractionCtl = 1;
        }
        bsmToSend->bsmpsmData.ABS = bsm->coreData->brakes.abs_;
        if (bsmToSend->bsmpsmData.ABS == 1)    {
            bsmToSend->bsmpsmData.ABS = 0;
        } else if ((bsmToSend->bsmpsmData.ABS == 2) || (bsmToSend->bsmpsmData.ABS ==3))    {
            bsmToSend->bsmpsmData.ABS = 1;
        }
        bsmToSend->bsmpsmData.SCS = bsm->coreData->brakes.scs;
        if (bsmToSend->bsmpsmData.SCS == 1)    {
            bsmToSend->bsmpsmData.SCS = 0;
        } else if (bsmToSend->bsmpsmData.SCS ==3)    {
            bsmToSend->bsmpsmData.SCS = 1;
        }
    }
#endif
}

STATIC void decodeAMHBSM(void)
{
    uint8_t         bsmData[MAX_WSM_DATA];
    MessageFrame    bsmMessageFrame;
    BasicSafetyMessage bsm;
    bsmpsmPacketT   bsmToSend;
    uint32_t        bsmDataLen;
#if defined(EXTRA_DEBUG)
    uint32_t        decodedBsmLen;
#endif
    uint8_t *       pos;
    uint8_t         udpPktBsm[MAX_WSM_DATA];
    bsmpsmPacketT   *bsmToSendPtr;

    /* Check if the received BSM data is new */
    WSU_SEM_LOCKW(&shm_ptr->amhImmediateBsm.h.ch_lock);
    if (shm_ptr->amhImmediateBsm.newmsg == WTRUE)    {
        /* Copy the encoded BSM to a local buffer */
        bsmDataLen = shm_ptr->amhImmediateBsm.count; 
        memcpy(&bsmData,shm_ptr->amhImmediateBsm.data,bsmDataLen);
        shm_ptr->amhImmediateBsm.newmsg = WFALSE;  /* ported from 5910; must be set otherwise messages go out for eternity */
    }
    else {
#if defined(EXTRA_DEBUG)
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"BSM Data from AMH not available.\n");
#endif
        WSU_SEM_UNLOCKW(&shm_ptr->amhImmediateBsm.h.ch_lock);
        return;
    }
    WSU_SEM_UNLOCKW(&shm_ptr->amhImmediateBsm.h.ch_lock);

    /* Start decoding the BSM */
    asn1Init_MessageFrame(&bsmMessageFrame);

    /* Set the buffer for unaligned PER */
    pu_setBuffer(&bsmpsmCtxt, bsmData, bsmDataLen,0); 

    /* Decode the message Frame */
    if (asn1PD_MessageFrame(&bsmpsmCtxt,&bsmMessageFrame) != 0)   {
#if defined(EXTRA_DEBUG)
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"BSM Message Frame decode failed %u \n",bsmDataLen);
#endif
        rtxMemReset(&bsmpsmCtxt);
        rtxErrReset(&bsmpsmCtxt);
        set_my_error_state(SPAT16_BSM_DECODE_MSG_FRAME_FAIL);
        return;
    }
#if defined(EXTRA_DEBUG)
    /* Obtain the decoded message frame length */
    decodedBsmLen = pu_getMsgLen(&bsmpsmCtxt);
#endif
#if defined(MY_UNIT_TEST)
    bsmMessageFrame.messageId = ASN1V_basicSafetyMessage;
#endif
    /* Check if the message received by AMH is BSM */
    if (bsmMessageFrame.messageId != ASN1V_basicSafetyMessage)  {
#if defined(EXTRA_DEBUG)
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Decoded message (len %u) is not ASN1V_BasicSafetyMessage \n",decodedBsmLen);
#endif
        rtxMemReset(&bsmpsmCtxt);
        rtxErrReset(&bsmpsmCtxt);
        set_my_error_state(SPAT16_BSM_DECODE_MSG_ID_FAIL);
        return;
    }

    /* Decode the BSM Part */
    asn1Init_BasicSafetyMessage(&bsm);

    /* Set buffer */
    pu_setBuffer(&bsmpsmCtxt,(wuint8 *)bsmMessageFrame.value.data,bsmMessageFrame.value.numocts,0);

    /* Decode the BSM from AMH */
    if (asn1PD_BasicSafetyMessage(&bsmpsmCtxt,&bsm) != 0) {
#if defined(EXTRA_DEBUG)
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"BSM decode failed \n");
#endif
        rtxMemReset(&bsmpsmCtxt);
        rtxErrReset(&bsmpsmCtxt);
        set_my_error_state(SPAT16_BSM_DECODE_BODY_FAIL);
        return;
    }
    
    parseBSM(&bsm,&bsmToSend);

    /* Convert the struct to buffer to send it over UDP */
    pos = udpPktBsm;

    bsmToSendPtr = (bsmpsmPacketT *) pos;

    bsmToSendPtr->msgType = bsmToSend.msgType;
    bsmToSendPtr->bsmpsmData.personalDeviceUserType = bsmToSend.bsmpsmData.personalDeviceUserType;
    bsmToSendPtr->bsmpsmData.msgCnt = bsmToSend.bsmpsmData.msgCnt;
    memcpy(bsmToSendPtr->bsmpsmData.vehID,bsmToSend.bsmpsmData.vehID,VEHID_SIZE);
    bsmToSendPtr->bsmpsmData.utc_millisec = bsmToSend.bsmpsmData.utc_millisec;
    bsmToSendPtr->bsmpsmData.secMarkMs = bsmToSend.bsmpsmData.secMarkMs;
    bsmToSendPtr->bsmpsmData.latitudeDeg = bsmToSend.bsmpsmData.latitudeDeg;
    bsmToSendPtr->bsmpsmData.longitudeDeg = bsmToSend.bsmpsmData.longitudeDeg;
    bsmToSendPtr->bsmpsmData.elevationM = bsmToSend.bsmpsmData.elevationM;
    bsmToSendPtr->bsmpsmData.transmissionState = bsmToSend.bsmpsmData.transmissionState;
    bsmToSendPtr->bsmpsmData.vehSpeedKph = bsmToSend.bsmpsmData.vehSpeedKph;
    bsmToSendPtr->bsmpsmData.headingDeg = bsmToSend.bsmpsmData.headingDeg;
    bsmToSendPtr->bsmpsmData.steeringWhlAngle = bsmToSend.bsmpsmData.steeringWhlAngle;
    bsmToSendPtr->bsmpsmData.longAccel = bsmToSend.bsmpsmData.longAccel;
    bsmToSendPtr->bsmpsmData.yawRate = bsmToSend.bsmpsmData.yawRate;
    bsmToSendPtr->bsmpsmData.brakeActive = bsmToSend.bsmpsmData.brakeActive;
    bsmToSendPtr->bsmpsmData.tractionCtl = bsmToSend.bsmpsmData.tractionCtl;
    bsmToSendPtr->bsmpsmData.ABS = bsmToSend.bsmpsmData.ABS;
    bsmToSendPtr->bsmpsmData.SCS = bsmToSend.bsmpsmData.SCS;

    /* Check if the byte swapping needed */
    if (littleEndian()) {
        swapBsmPsmPacket(bsmToSendPtr);
    }

    /* move the pointer */
    pos+= sizeof(bsmpsmPacketT);

    pthread_mutex_lock(&fwdmsg_send_lock);
    memset(&bsmpsmFwdmsgData,0,sizeof(fwdmsgData));
    bsmpsmFwdmsgData.fwdmsgType = AMH;
    memcpy(bsmpsmFwdmsgData.fwdmsgData,udpPktBsm,(pos-udpPktBsm));
    bsmpsmFwdmsgData.fwdmsgDataLen = pos-udpPktBsm;

    memcpy(bsmpsmSendBuf,&bsmpsmFwdmsgData,sizeof(fwdmsgData));

    if ( -1 == mq_send(fwdmsgfd, bsmpsmSendBuf,sizeof(fwdmsgData),DEFAULT_MQ_PRIORITY))
    {
        set_my_error_state(SPAT16_FWDMSG_MQ_SEND_ERROR);
    }
    pthread_mutex_unlock(&fwdmsg_send_lock);

}

STATIC void parsePSM(PersonalSafetyMessage *psm, bsmpsmPacketT *psmToSend)
{
    wuint16 uShortVal;
    wint16  shortVal;
    wint32  intVal;

    memset(psmToSend, 0, sizeof(bsmpsmPacketT));

    psmToSend->msgType = PSM_PACKET;
    psmToSend->bsmpsmData.utc_millisec = i2vUtilGetTimeInMs();
#if defined(J2735_2016)
    psmToSend->bsmpsmData.personalDeviceUserType = psm->basicType;

    psmToSend->bsmpsmData.msgCnt = psm->msgCnt; 

    memcpy(psmToSend->bsmpsmData.vehID, psm->id.data,psm->id.numocts);

    uShortVal = psm->secMark;
    if (uShortVal != 65535) {
        psmToSend->bsmpsmData.secMarkMs = uShortVal;
    }

    /* Position 3D lat */
    intVal = psm->position.lat;
    if (intVal != 900000001)    {
        psmToSend->bsmpsmData.latitudeDeg = intVal / 1e7;
    }
    
    /* Longitude */
    intVal = psm->position.long_;
    if (intVal != 1800000001)    {
        psmToSend->bsmpsmData.longitudeDeg = intVal / 1e7;
    }

    /* Elevation, if present */
    if (psm->position.m.elevationPresent)   {

        uShortVal = psm->position.elevation;
        if (uShortVal != 0xF000)    {
            if (uShortVal > 0xF000) {
                shortVal = uShortVal; /* negative values */
                psmToSend->bsmpsmData.elevationM = shortVal / 10.0;
            }
            else {
                psmToSend->bsmpsmData.elevationM = uShortVal / 10.0;
            }
        }
    }

    /* Heading */
    uShortVal = psm->heading;
    if (uShortVal != 28800) {
        psmToSend->bsmpsmData.headingDeg = uShortVal * 0.0125;
    }

    /* Speed */
    uShortVal = psm->speed;
    if (uShortVal != 8191)   {
        psmToSend->bsmpsmData.vehSpeedKph = uShortVal * 0.02;
        psmToSend->bsmpsmData.vehSpeedKph = MPS_KPH(psmToSend->bsmpsmData.vehSpeedKph);
    }

    if (psm->m.accelSetPresent)  {
        /* LongAccel */
        shortVal = psm->accelSet.long_;
        if (shortVal != 2001)   {
            psmToSend->bsmpsmData.longAccel = shortVal * 0.01;
        }

        /* YawRate */
        shortVal = psm->accelSet.yaw;
        if (shortVal != 32767)  {
            psmToSend->bsmpsmData.yawRate = shortVal * 0.01;
        }
    }
#else //(J2735_2023)
    psmToSend->bsmpsmData.personalDeviceUserType = psm->basicType;
    psmToSend->bsmpsmData.msgCnt = psm->msgCnt; 
    if(NULL != psm->id) {
        memcpy(psmToSend->bsmpsmData.vehID, psm->id->data,psm->id->numocts);
    }
    uShortVal = psm->secMark;
    if (uShortVal != 65535) {
        psmToSend->bsmpsmData.secMarkMs = uShortVal;
    }
    /* Position 3D lat */
    if(NULL != psm->position) {
    intVal = psm->position->lat;
    if (intVal != 900000001)    {
        psmToSend->bsmpsmData.latitudeDeg = intVal / 1e7;
    }
    /* Longitude */
    intVal = psm->position->long_;
    if (intVal != 1800000001)    {
        psmToSend->bsmpsmData.longitudeDeg = intVal / 1e7;
    }
    /* Elevation, if present */
    if (psm->position->m.elevationPresent)   {
        uShortVal = psm->position->elevation;
        if (uShortVal != 0xF000)    {
            if (uShortVal > 0xF000) {
                shortVal = uShortVal; /* negative values */
                psmToSend->bsmpsmData.elevationM = shortVal / 10.0;
            } else {
                psmToSend->bsmpsmData.elevationM = uShortVal / 10.0;
            }
        }
    }
    }
    /* Heading */
    uShortVal = psm->heading;
    if (uShortVal != 28800) {
        psmToSend->bsmpsmData.headingDeg = uShortVal * 0.0125;
    }
    /* Speed */
    uShortVal = psm->speed;
    if (uShortVal != 8191)   {
        psmToSend->bsmpsmData.vehSpeedKph = uShortVal * 0.02;
        psmToSend->bsmpsmData.vehSpeedKph = MPS_KPH(psmToSend->bsmpsmData.vehSpeedKph);
    }
    if (psm->m.accelSetPresent)  {
        if(NULL != psm->accelSet) {
        /* LongAccel */
        shortVal = psm->accelSet->long_;
        if (shortVal != 2001)   {
            psmToSend->bsmpsmData.longAccel = shortVal * 0.01;
        }
        /* YawRate */
        shortVal = psm->accelSet->yaw;
        if (shortVal != 32767)  {
            psmToSend->bsmpsmData.yawRate = shortVal * 0.01;
        }
        }
    }
#endif
}
static void decodeAMHPSM()
{
    uint8_t         psmData[MAX_WSM_DATA];
    MessageFrame    psmMessageFrame;
    PersonalSafetyMessage psm;
    bsmpsmPacketT   psmToSend;
    uint32_t        psmDataLen;
#if defined(EXTRA_DEBUG)
    uint32_t        decodedPsmLen;
#endif
    uint8_t *       pos;
    uint8_t         udpPktPsm[MAX_WSM_DATA];
    bsmpsmPacketT   *psmToSendPtr;

    /* Check if the received PSM data is new */
    WSU_SEM_LOCKW(&shm_ptr->amhImmediatePsm.h.ch_lock);
    if (shm_ptr->amhImmediatePsm.newmsg == WTRUE)    {
        /* Copy the encoded PSM to a local buffer */
        psmDataLen = shm_ptr->amhImmediatePsm.count; 
        memcpy(&psmData,shm_ptr->amhImmediatePsm.data,psmDataLen);
        shm_ptr->amhImmediatePsm.newmsg = WFALSE;  /* ported from 5910; must be set otherwise messages go out for eternity */
    }
    else {
#if defined(EXTRA_DEBUG)
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"PSM Data from AMH not available.\n");
#endif
        WSU_SEM_UNLOCKW(&shm_ptr->amhImmediatePsm.h.ch_lock);
        return;
    }
    WSU_SEM_UNLOCKW(&shm_ptr->amhImmediatePsm.h.ch_lock);

    /* Start decoding the PSM */
    asn1Init_MessageFrame(&psmMessageFrame);

    /* Set the buffer for unaligned PER */
    pu_setBuffer(&bsmpsmCtxt, (wuint8 *)psmData, psmDataLen,0); 

    /* Decode the message Frame */
    if (asn1PD_MessageFrame(&bsmpsmCtxt,&psmMessageFrame) != 0)   {
#if defined(EXTRA_DEBUG)
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"PSM Message Frame Decode Failed\n");
#endif
        rtxMemReset(&bsmpsmCtxt);
        rtxErrReset(&bsmpsmCtxt);
        set_my_error_state(SPAT16_PSM_DECODE_MSG_FRAME_FAIL);
        return;
    }
#if defined(EXTRA_DEBUG)
    /* Obtain the decoded message frame length */
    decodedPsmLen = pu_getMsgLen(&bsmpsmCtxt);
#endif
#if defined(MY_UNIT_TEST)
    psmMessageFrame.messageId = ASN1V_personalSafetyMessage;
#endif
    /* Check if the message received by AMH is PSM */
    if (psmMessageFrame.messageId != ASN1V_personalSafetyMessage)  {
#if defined(EXTRA_DEBUG)
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Received Message (len %u) is not PSM \n",decodedPsmLen);
#endif
        rtxMemReset(&bsmpsmCtxt);
        rtxErrReset(&bsmpsmCtxt);
        set_my_error_state(SPAT16_PSM_DECODE_MSG_ID_FAIL);
        return;
    }

    /* Decode the PSM Part */
    asn1Init_PersonalSafetyMessage(&psm);

    /* Set buffer */
    pu_setBuffer(&bsmpsmCtxt, (wuint8 *)psmMessageFrame.value.data, psmMessageFrame.value.numocts, 0);

    /* Decode the BSM from AMH */
    if (asn1PD_PersonalSafetyMessage(&bsmpsmCtxt,&psm) != 0) {
#if defined(EXTRA_DEBUG)
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"PSM message decode failed \n");
#endif
        rtxMemReset(&bsmpsmCtxt);
        rtxErrReset(&bsmpsmCtxt);
        set_my_error_state(SPAT16_PSM_DECODE_BODY_FAIL);
        return;
    }

    /* Parse PSM */
    parsePSM(&psm,&psmToSend);

    /* Convert the struct into buffer to send it over the socket */
    pos = udpPktPsm;

    psmToSendPtr = (bsmpsmPacketT *) pos;

    psmToSendPtr->msgType = psmToSend.msgType;
    psmToSendPtr->bsmpsmData.personalDeviceUserType = psmToSend.bsmpsmData.personalDeviceUserType;
    psmToSendPtr->bsmpsmData.msgCnt = psmToSend.bsmpsmData.msgCnt;
    memcpy(psmToSendPtr->bsmpsmData.vehID,psmToSend.bsmpsmData.vehID,VEHID_SIZE);
    psmToSendPtr->bsmpsmData.utc_millisec = psmToSend.bsmpsmData.utc_millisec;
    psmToSendPtr->bsmpsmData.secMarkMs = psmToSend.bsmpsmData.secMarkMs;
    psmToSendPtr->bsmpsmData.latitudeDeg = psmToSend.bsmpsmData.latitudeDeg;
    psmToSendPtr->bsmpsmData.longitudeDeg = psmToSend.bsmpsmData.longitudeDeg;
    psmToSendPtr->bsmpsmData.elevationM = psmToSend.bsmpsmData.elevationM;
    psmToSendPtr->bsmpsmData.transmissionState = psmToSend.bsmpsmData.transmissionState;
    psmToSendPtr->bsmpsmData.vehSpeedKph = psmToSend.bsmpsmData.vehSpeedKph;
    psmToSendPtr->bsmpsmData.headingDeg = psmToSend.bsmpsmData.headingDeg;
    psmToSendPtr->bsmpsmData.steeringWhlAngle = psmToSend.bsmpsmData.steeringWhlAngle;
    psmToSendPtr->bsmpsmData.longAccel = psmToSend.bsmpsmData.longAccel;
    psmToSendPtr->bsmpsmData.yawRate = psmToSend.bsmpsmData.yawRate;
    psmToSendPtr->bsmpsmData.brakeActive = psmToSend.bsmpsmData.brakeActive;
    psmToSendPtr->bsmpsmData.tractionCtl = psmToSend.bsmpsmData.tractionCtl;
    psmToSendPtr->bsmpsmData.ABS = psmToSend.bsmpsmData.ABS;
    psmToSendPtr->bsmpsmData.SCS = psmToSend.bsmpsmData.SCS;

    /* swap the data if needed */
    if (littleEndian()) {
        swapBsmPsmPacket(psmToSendPtr);
    }

    pos += sizeof(bsmpsmPacketT);

    pthread_mutex_lock(&fwdmsg_send_lock);
    /* Write the messages onto the fwdmsg process queue */
    memset(&bsmpsmFwdmsgData,0,sizeof(fwdmsgData));
    bsmpsmFwdmsgData.fwdmsgType = AMH;
    memcpy(bsmpsmFwdmsgData.fwdmsgData,udpPktPsm,(pos-udpPktPsm));
    bsmpsmFwdmsgData.fwdmsgDataLen = pos-udpPktPsm;

    memcpy(bsmpsmSendBuf,&bsmpsmFwdmsgData,sizeof(fwdmsgData));

    if ( -1 == mq_send(fwdmsgfd, bsmpsmSendBuf,sizeof(fwdmsgData),DEFAULT_MQ_PRIORITY))
    {
        set_my_error_state(SPAT16_FWDMSG_MQ_SEND_ERROR);
    }
    pthread_mutex_unlock(&fwdmsg_send_lock);
}

STATIC void bsmpsmForward(void)
{
    decodeAMHBSM(); 
    decodeAMHPSM();
    /* Free the memory when done */
    rtxMemFree(&bsmpsmCtxt);
    return;
}

void *bsmpsmForwardThread(void __attribute__((unused)) *arg)
{
  /* Socket Ready. Initialize the ASN1 Context */
  if (0 != rtInitContext(&bsmpsmCtxt)) {
#if defined(EXTRA_DEBUG)
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"BSM/PSM context not initialized properly\n");
#endif
      set_my_error_state(SPAT16_RTINIT_FAIL);
  } else {
      while (mainloop)    {
          usleep(100 * 1000);     // For every 100ms, will miss some for sure.
          bsmpsmForward();
          #if defined(MY_UNIT_TEST)
          break;
          #endif      
      }
      /* Free Context */
      rtxFreeContext(&bsmpsmCtxt);
  }
  pthread_exit(NULL);
}
#endif

void *CommRangeThread(void __attribute__((unused)) *arg)
{
  int32_t i = 0;
  CommRangeStats_t       * shm_commRange_ptr = NULL;
  shm_tps_t              * shm_tps_ptr = NULL;
  tps_geodetic_publish_t   tps_pub_geo;
  uint16_t                 index = 0;
  /* For readability. */
  size_t shm_commRange_size  = sizeof(CommRangeStats_t) * RSU_COMM_RANGE_MAX;
  size_t commRangeStats_size = sizeof(CommRangeStats_t) * RSU_COMM_RANGE_MAX;

  /* MIB will update periodscally in SHM. We act on it and populate SHM back. */
  memset(CommRangeStats,0x0,commRangeStats_size);

  if ((shm_commRange_ptr = wsu_share_init(shm_commRange_size, COMM_RANGE_SHM_PATH)) == NULL) {
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"CommRangeThread: SHM init failed.\n");
      set_my_error_state(SPAT16_SHM_FAIL);
  } else {
      if ((shm_tps_ptr = wsu_share_init(sizeof(shm_tps_t), SHM_TPS_PATH)) == NULL) {
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"CommRangeThread: TPS SHM init failed.\n");
          set_my_error_state(SPAT16_SHM_FAIL);
      } else {
          while (mainloop) {
              /*
               * Update time once for everyone. Grab RSU lat/lon.
               */
              time_utc_msecs = i2vUtilGetTimeInMs();
              memcpy(&tps_pub_geo, &shm_tps_ptr->pub_geodetic, sizeof(tps_geodetic_publish_t));
              index = tps_pub_geo.last_updated_pvt_index;
              RSU_lat = ((float64_t)tps_pub_geo.last_PVT[index].lat_deg_e7) / (float64_t)(1.0e7); /* degree. */
              RSU_lon = ((float64_t)tps_pub_geo.last_PVT[index].lon_deg_e7) / (float64_t)(1.0e7);

//TODO: create mutex for SHM

              /*
               * Update & service client requests for commRange.
               */
              for(i=0;i<RSU_COMM_RANGE_MAX;i++){

                  /* Client has active row. */
                  if (shm_commRange_ptr[i].CommRangeStatus == SNMP_ROW_ACTIVE) {

                      /* 
                       * If new request then setup local table for operation. 
                       * Only support BSM. Type and value not used today. 
                       */
                      if (CommRangeStats[i].CommRangeStatus == SNMP_ROW_NONEXISTENT) {

                          CommRangeStats[i].CommRangeStatus = SNMP_ROW_ACTIVE;
                          CommRangeStats[i].CommRangeSector = shm_commRange_ptr[i].CommRangeSector;
                          CommRangeStats[i].CommRangeMsgId = shm_commRange_ptr[i].CommRangeMsgId;
                          CommRangeStats[i].CommRangeFilterType = shm_commRange_ptr[i].CommRangeFilterType;
                          CommRangeStats[i].CommRangeFilterValue = shm_commRange_ptr[i].CommRangeFilterValue;

                          reset_commRange_stats(&shm_commRange_ptr[i], &CommRangeStats[i]);
                      }
                      /* If client requests new sector on active row then update & reset. */
                      if(CommRangeStats[i].CommRangeSector != shm_commRange_ptr[i].CommRangeSector) {
                          CommRangeStats[i].CommRangeSector = shm_commRange_ptr[i].CommRangeSector;
                          reset_commRange_stats(&shm_commRange_ptr[i], &CommRangeStats[i]);
                      }

//TODO: warning on significant lag in timing.

                      /* 1 min update. */
                      if(CommRangeStats[i].CommRange1Min_time <= time_utc_msecs) {
                          #if defined(EXTRA_DEBUG)
                          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"1Min_time(%d) <= time_utc_msecs(%lu) cnt=%d total=%d.\n",
                              i,time_utc_msecs,CommRangeStats[i].CommRange1Min_count,CommRangeStats[i].CommRange1Min_total);
                          #endif
                          shm_commRange_ptr[i].CommRange1Min_max = CommRangeStats[i].CommRange1Min_max;
                          CommRangeStats[i].CommRange1Min_max = 0;

                          shm_commRange_ptr[i].CommRangeAvg1Min = 0;
                          shm_commRange_ptr[i].CommRange1Min_count = CommRangeStats[i].CommRange1Min_count;
                          shm_commRange_ptr[i].CommRange1Min_total = CommRangeStats[i].CommRange1Min_total;
                          if(0 <CommRangeStats[i].CommRange1Min_count) {
                              shm_commRange_ptr[i].CommRangeAvg1Min = (CommRangeStats[i].CommRange1Min_total/CommRangeStats[i].CommRange1Min_count);
                          }
                          CommRangeStats[i].CommRange1Min_count = 0;
                          CommRangeStats[i].CommRange1Min_total = 0;

                          /* Reset clock counter. */
                          CommRangeStats[i].CommRange1Min_time = time_utc_msecs + (1 * 60000);
                      }

                      /* 5 min update. */
                      if(CommRangeStats[i].CommRange5Min_time <= time_utc_msecs) {
                          #if defined(EXTRA_DEBUG)
                          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"5Min_time(%d) <= time_utc_msecs(%lu) cnt=%d total=%d.\n",
                              i,time_utc_msecs,CommRangeStats[i].CommRange5Min_count,CommRangeStats[i].CommRange5Min_total);
                          #endif
                          shm_commRange_ptr[i].CommRange5Min_max = CommRangeStats[i].CommRange5Min_max;
                          CommRangeStats[i].CommRange5Min_max = 0;

                          shm_commRange_ptr[i].CommRange5Min_count = CommRangeStats[i].CommRange5Min_count;
                          shm_commRange_ptr[i].CommRange5Min_total = CommRangeStats[i].CommRange5Min_total;
                          shm_commRange_ptr[i].CommRangeAvg5Min = 0;
                          if(0 < CommRangeStats[i].CommRange5Min_count) {
                              shm_commRange_ptr[i].CommRangeAvg5Min = (CommRangeStats[i].CommRange5Min_total/CommRangeStats[i].CommRange5Min_count);
                          }
                          CommRangeStats[i].CommRange5Min_count = 0;
                          CommRangeStats[i].CommRange5Min_total = 0;

                          /* Reset clock counter. */
                          CommRangeStats[i].CommRange5Min_time = time_utc_msecs + (5 * 60000);
                      }

                      /* 15 min update. */
                      if(CommRangeStats[i].CommRange15Min_time <= time_utc_msecs) {
                          #if defined(EXTRA_DEBUG)
                          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"15Min_time(%d) <= time_utc_msecs(%lu) cnt=%d total=%d.\n",
                              i,time_utc_msecs,CommRangeStats[i].CommRange15Min_count,CommRangeStats[i].CommRange15Min_total);
                          #endif
                          shm_commRange_ptr[i].CommRange15Min_max = CommRangeStats[i].CommRange15Min_max;
                          CommRangeStats[i].CommRange15Min_max = 0;

                          shm_commRange_ptr[i].CommRange15Min_count = CommRangeStats[i].CommRange15Min_count;
                          shm_commRange_ptr[i].CommRange15Min_total = CommRangeStats[i].CommRange15Min_total;
                          shm_commRange_ptr[i].CommRangeAvg15Min = 0;
                          if(0 < CommRangeStats[i].CommRange15Min_count) {
                              shm_commRange_ptr[i].CommRangeAvg15Min = (CommRangeStats[i].CommRange15Min_total/CommRangeStats[i].CommRange15Min_count);
                          }
                          CommRangeStats[i].CommRange15Min_count = 0;
                          CommRangeStats[i].CommRange15Min_total = 0;

                          /* Reset clock counter. */
                          CommRangeStats[i].CommRange15Min_time = time_utc_msecs + (15 * 60000);
                      }
                  }

                  /* If nothing to do, zero out once. */
                  if (   (shm_commRange_ptr[i].CommRangeStatus == SNMP_ROW_NONEXISTENT) 
                      && (CommRangeStats[i].CommRangeStatus == SNMP_ROW_ACTIVE)) {

                      CommRangeStats[i].CommRangeStatus = SNMP_ROW_NONEXISTENT;
                      CommRangeStats[i].CommRangeSector = 0;
                      CommRangeStats[i].CommRangeMsgId = 0;
                      CommRangeStats[i].CommRangeFilterType = 0;
                      CommRangeStats[i].CommRangeFilterValue = 0;

                      reset_commRange_stats(&shm_commRange_ptr[i], &CommRangeStats[i]);
                  }
              }
              #if defined(EXTRA_DEBUG)
              {
                static uint32_t rolling_counter=0x0;
                  if(0 == (rolling_counter % 15)) {
                      for(i=0;i<RSU_COMM_RANGE_MAX;i++){
                          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"CommRng:%d:st=%d,sec=%d,id=0x%x,1-5-15 M:A:C:T(%d,%d,%d,%d)(%d,%d,%d,%d)(%d,%d,%d,%d).\n"
                              , i
                              , shm_commRange_ptr[i].CommRangeStatus
                              , shm_commRange_ptr[i].CommRangeSector
                              , shm_commRange_ptr[i].CommRangeMsgId
                              , shm_commRange_ptr[i].CommRange1Min_max
                              , shm_commRange_ptr[i].CommRangeAvg1Min
                              , shm_commRange_ptr[i].CommRange1Min_count
                              , shm_commRange_ptr[i].CommRange1Min_total
                              , shm_commRange_ptr[i].CommRange5Min_max
                              , shm_commRange_ptr[i].CommRangeAvg5Min
                              , shm_commRange_ptr[i].CommRange5Min_count
                              , shm_commRange_ptr[i].CommRange5Min_total
                              , shm_commRange_ptr[i].CommRange15Min_max
                              , shm_commRange_ptr[i].CommRangeAvg15Min
                              , shm_commRange_ptr[i].CommRange15Min_count
                              , shm_commRange_ptr[i].CommRange15Min_total);
                      }
                  }
                  rolling_counter++;
              }
              #endif
              sleep(1); /* Not ideal. We could over shot the stats countdown by a second or so. */
              #if defined(MY_UNIT_TEST)
              break;
              #endif
          }
      }
  }
  set_my_error_state(SPAT16_COMM_RANGE_THREAD_EXIT); /* Not Fatal. */
  /* Do not destroy as other threads and MIB can be still using. Recover on start up. */
  if(NULL != shm_commRange_ptr) {
      wsu_share_kill(shm_commRange_ptr, shm_commRange_size);
  }
  if(NULL != shm_tps_ptr) {
      wsu_share_kill(shm_tps_ptr, sizeof(shm_tps_t));
  }
  pthread_exit(NULL);
}
/* 
 * SPAT is blocked on a gate from SCS. This is our pseudo mainloop in lieu. 
 * So the output below is redundent but useful if no blobs or issue with scs.
 */
void *ReceivedMsgThread(void __attribute__((unused)) *arg)
{
  uint32_t rolling_counter = 0x0;

  while(mainloop) {
      sleep(1);
      if(3 == (rolling_counter % OUTPUT_MODULUS)){
          I2V_DBG_LOG(LEVEL_INFO,MY_NAME,"(0x%lx) SPAT tx,E(%u,%u) BSM rx,E(%u, %u)\n", 
              my_error_states,spat_tx_count,spat_tx_err_count,commRangeBsmCount,bsm_extract_lle_fail);
      }
//todo: grab tps shm fix status and gate this bsm handling. Need first fix then after that if we lose we lose.
//todo: eval error conditions and clear. If not a valid fix then nobody cares.
      //set_my_error_state(SPAT16_BSM_DISTANCE_WARNING);
      rolling_counter++;

      #if defined(MY_UNIT_TEST)
      break;
      #endif   
  }
  set_my_error_state(SPAT16_RX_MSG_THREAD_EXIT);  /* Not Fatal. */
  pthread_exit(NULL);
}
/* Tailor for the PSIDs and directions the particular module needs. */
void *MessageStatsThread(void __attribute__((unused)) *arg)
{
  size_t shm_messageStats_size = sizeof(messageStats_t) * RSU_PSID_TRACKED_STATS_MAX;
  int32_t i;
  uint32_t rolling_counter = 0;

  /* Open SHM. */
  if (NULL == (shm_messageStats_ptr = wsu_share_init(shm_messageStats_size, MSG_STATS_SHM_PATH))) {
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"messageStatsThr: SHM init failed.\n");
      set_my_error_state(SPAT16_SHM_FAIL);
  } else {
      memset(&messageStats[0],0x0,shm_messageStats_size);
      while(mainloop) {
          sleep(1);
          if(0 == (rolling_counter % MSG_STATS_APP_SHM_UPDATE_RATE)) {
              for(i=0;i<RSU_PSID_TRACKED_STATS_MAX;i++){
                  /* If dirty then update from client. */
                  if(0x1 == shm_messageStats_ptr[i].dirty) {
                      /* 
                       * Only add to local copy what we want to support.
                       * BSM and SPAT: Not sure what PSID is PSM?
                       * BSM direction is IN or BOTH. SPAT is OUT or BOTH.
                       */
                      if(  (RSU_CV2X_CHANNEL_DEFAULT == shm_messageStats_ptr[i].channel)
                         &&(   ((cfg.bsmPsidPerU == shm_messageStats_ptr[i].psid) && (RSU_MESSAGE_COUNT_DIRECTION_OUT != shm_messageStats_ptr[i].direction))
                            || ((cfg.secSpatPsid == shm_messageStats_ptr[i].psid) && (RSU_MESSAGE_COUNT_DIRECTION_IN  != shm_messageStats_ptr[i].direction))
                           )){
                          if (SNMP_ROW_ACTIVE == shm_messageStats_ptr[i].rowStatus) {
                              messageStats[i].psid = shm_messageStats_ptr[i].psid;
                              messageStats[i].channel = shm_messageStats_ptr[i].channel;
                              messageStats[i].direction = shm_messageStats_ptr[i].direction;
                              messageStats[i].start_utc_dsec = shm_messageStats_ptr[i].start_utc_dsec;
                          } else {
                              messageStats[i].psid = 0x0;
                              messageStats[i].channel = 0x0;
                              messageStats[i].direction = 0x0;
                              messageStats[i].start_utc_dsec = 0x0;
                          }
                          messageStats[i].count = 0; /* Do not clear SHM count. MIB does that. */
                          messageStats[i].rowStatus = shm_messageStats_ptr[i].rowStatus;
                          shm_messageStats_ptr[i].dirty = 0x0;
                      }
                  } else {
                      /* 
                       * If row is active and matches our local then update.
                       * Don't overwrite total, only add to it.
                       * Some PSID's shared like SPAT and MAP.
                       */
                      if(SNMP_ROW_ACTIVE == shm_messageStats_ptr[i].rowStatus) {
                          if (   (SNMP_ROW_ACTIVE == messageStats[i].rowStatus) 
                              && (messageStats[i].psid == shm_messageStats_ptr[i].psid)
                              && (0 < messageStats[i].count)){ 
                              shm_messageStats_ptr[i].count += messageStats[i].count;
                              messageStats[i].count = 0; /* Clear for next iteration. */
                          }
                      }
                  }
              } /* for */
          }
          #if defined(EXTRA_DEBUG)
          if(0 == (rolling_counter % OUTPUT_MODULUS)){
              for(i=0;i < 5;i++){
                  I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"MsgStatShm[%d]: dirty=%u, status=%d, count=%u, psid=0x%x, chan=%d, dir=%d, start=0x%lx.\n"
                      ,i,messageStats[i].dirty,messageStats[i].rowStatus,messageStats[i].count
                      ,messageStats[i].psid,messageStats[i].channel,messageStats[i].direction,messageStats[i].start_utc_dsec);
              }
          }
          #endif
          rolling_counter++;
          #if defined(MY_UNIT_TEST)
          break;
          #endif   
      } /* While */
  }
  set_my_error_state(SPAT16_MSG_STATS_THREAD_EXIT); /* Not Fatal. */
  /* Do not destroy as other threads and MIB can be still using. Recover on start up. */
  if(NULL != shm_messageStats_ptr) {
      wsu_share_kill(shm_messageStats_ptr, shm_messageStats_size);
  }
  pthread_exit(NULL);
}
