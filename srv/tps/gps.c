/*---------------------------------------------------------------------------*/
/*                                                                           */
/*    Filename: gps.c                                                        */
/*     Purpose: GPS Functions for the Time and Position Service (tps) app    */
/*  Department: DENSO North America Research & Development                   */
/*                                                                           */
/* Copyright (C) 2020 DENSO International America, Inc.                      */
/*                                                                           */
/*     Project: V2X Special Projects                                         */
/* Description: GPS-related functions of TPS                                 */
/*                                                                           */
/*---------------------------------------------------------------------------*/
//#define EXTRA_DEBUG
#define DBG_SYSTEMTIMEUPDATE

//---CPNOTE:  The following line is required to use strptime()
/////#define _XOPEN_SOURCE  600

#define DBG_ENABLE_SEND_TO_UBLOX

/*----------------------------------------------------------------------------*/
/* Include Files                                                              */
/*----------------------------------------------------------------------------*/
#include <termios.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <syslog.h>
#include <pthread.h>
#include <math.h>           // for sqrt()
#include <unistd.h>                   // for sleep(),...

#if !defined(MY_UNIT_TEST) //for now dont include ssl in unit test
#include <openssl/sha.h>
#endif

#include <time.h>                     //for strptime(),...
#include <sys/time.h>                 //for gettimeofday(),....
#include <linux/rtc.h>
#include <sys/sysinfo.h>              // for struct sysinfo
#include "timepps.h"
#include "strl_utils.h"               //for local strlcpy(), strlcat()
#include "dn_types.h"
#include "wsu_util.h"
#include "tps_msg.h"
#include "tps_types.h"
#include "gps.h"
#include "tps.h"
#include "ipcsock.h"
#if defined(ENABLE_SR)
#include "shm_sr.h"         // for shm_sr_t and SR_PLAYBACK_MODE_CHAR
#endif
#include "shm_tps.h"
#include "v2x_common.h"
#include "i2v_util.h"
#include "nscfg.h"

/* mandatory logging defines */
#define MY_ERR_LEVEL   LEVEL_PRIV    /* from i2v_util.h */
#define MY_NAME        "gps"

uint64_t ClockCycles(void)        //Determine if need clockcycles in Hercules...
{
    return 0x00FF;
}

/*----------------------------------------------------------------------------*/
/* Constant Definitions                                                       */
/*----------------------------------------------------------------------------*/

/* The largest NMEA-0183 message is 82 bytes.  Round up to an even 100. */
#define MAX_NMEA_MESSASE_LEN       100
#define NMEA_MSG_MAX_TOKENS        20
#define NMEA_MSG_MAX_TOKEN_LENGTH  20

#define USECS_PER_SEC        1000000

#define MAX_RTCM_DATA_LEN    200    // 101 for RTCM1001, 25 for RTCM1005
#define RTCM_CRC_LEN         4

#define GPS_SERIAL_DEVICE_PATH "/dev/ttyLP2"
#define GPS_SERIAL_CONFIG_CMD  "/bin/stty -F /dev/ttyLP2 115200 raw -echo -echoe -echok"

#define GPS_PPS_SOURCE_PATH  "/dev/pps0"
#define INVALID_TIME_ACCURACY_VALUE 0xFFFFFFFF

/* Interrupt Vector for GPS's PPS interrupt */
#if defined(V2XSP_HW_VERSION_MK5) || defined(V2XSP_HW_VERSION_VMWARE)
//#include <sys/neutrino.h>
//#include <sys/syspage.h>
    #define GPIO_PPS_IRQ  192       /* MK5 */
#else
    #define GPIO_PPS_IRQ  306       /* PoC */
#endif

// Enum two inputs for data to update linux system type
#define UPDATE_TIME_FROM_NMEA_DATA 1
#define UPDATE_TIME_FROM_UBX_DATA 2

/* Stages for gps command receiver */
#define STAGE_0_AWAITING_START_OF_NEXT_COMMAND   0
#define STAGE_1_GETTING_NMEA_COMMAND             1
#define STAGE_2_GETTING_RTCM_COMMAND             2
#define STAGE_3_GETTING_UBX_COMMAND              3


#define timeval_normalize(t) {\
        if ((t)->tv_usec >= USECS_PER_SEC) { \
             (t)->tv_usec -= USECS_PER_SEC; \
         (t)->tv_sec++; \
        } else if ((t)->tv_usec < 0) { \
         (t)->tv_usec += USECS_PER_SEC; \
         (t)->tv_sec--; \
        } \
}

#define timeval_sub(t1, t2) do { \
           (t1)->tv_usec -= (t2)->tv_usec;  \
           (t1)->tv_sec -= (t2)->tv_sec; \
           timeval_normalize(t1);\
} while (0)

#if defined(EXTRA_DEBUG)
#define PRINT_MAX_N_TIMES(n, fmt, args...) \
{ \
    static int counter = 0; \
    if (counter < (n)) { \
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,fmt, ##args); \
        counter++; \
    } \
}
#endif

/*----------------------------------------------------------------------------*/
/* Global Variables                                                           */
/*----------------------------------------------------------------------------*/

uint32_t  tpsDebugOutput = 0; /* DBG_OUTPUT_ADJTIME|DBG_OUTPUT_TIME|DBG_OUTPUT_TPSGEN|DBG_OUTPUT_PPS;  */
int32_t   gpsFd = 0;
int32_t   gps_spi_fd = 0;

extern shm_tps_t * shm_tps_ptr;
extern bool_t       tpsMainLoop;

/* Config var that lives in main.c that we need to access */
extern bool_t               cfg_record_tps_as_raw;
extern bool_t               cfg_adjust_system_time;
extern bool_t               cfg_persistency_enable;
extern bool_t               cfg_startup_leapsec_share;
extern savedTpsVariables_t  savedTpsVariables;
extern bool_t               flag_request_to_save_tps_variables;
extern uint8_t              last_good_leapsec;
extern uint16_t             cfg_time_accuracy_threshold;

/* Constants for GPS NMEA messages from the receiver. */
static const char_t GPRMC[6] = {"$GPRMC"};
static const char_t GPGGA[6] = {"$GPGGA"};
static const char_t GPGST[6] = {"$GPGST"};
static const char_t GPGSA[6] = {"$GPGSA"};

/* Constants for GLONASS NMEA messages from the receiver. */
static const char_t GLRMC[6] = {"$GLRMC"};
static const char_t GLGGA[6] = {"$GLGGA"};
static const char_t GLGST[6] = {"$GLGST"};
static const char_t GLGSA[6] = {"$GLGSA"};

/* Constants for GPS/GLONASS NMEA messages from the receiver. */
static const char_t GNRMC[6] = {"$GNRMC"};
static const char_t GNGGA[6] = {"$GNGGA"};
static const char_t GNGST[6] = {"$GNGST"};
static const char_t GNGSA[6] = {"$GNGSA"};

static char_t       delim[] = ",";

/* 1 Knot = 0.513888888888889 Meters per Second */
static const float64_t  gpsKnots2MetersPerSecond = 0.513888888888889;

/* Static variables used by GPS Comm only */
static tpsDataType      gpsData;
static int32_t          gpsTpsSockFd;
static uint8_t          gpsCommandBuffer[MAX_GPS_MESSAGE_LEN+1];
static uint16_t         chars_in_buffer = 0;
static int32_t          msgMunchingStage = STAGE_0_AWAITING_START_OF_NEXT_COMMAND;
static int32_t          RTCMMsgTotalLen;
static gpsGGA_t         gpsGGA;
static gpsRMC_t         gpsRMC;
static gpsGPGSA_t       gpsGPGSA;
static gpsGPGST_t       gpsGPST;
static bool_t            gpsReceivedGGA;
static bool_t            gpsReceivedRMC;
static bool_t            gpsReceivedGPGSA;       // Flag indicates data in gpsGPGSA is valid
static bool_t            gpsReceivedGPGST;
static ubxNavTimeLS_t   gpsTIMELS;
static ubxNavPvt_t      gpsNAVPVT;
static ubxNavDop_t      gpsNAVDOP;
static ubxNavEell_t     gpsNAVEELL;
static ubxEsfIns_t      gpsESFINS;
static ubxEsfRaw_t      gpsESFRAW;
static ubxHnrIns_t      gpsHNRINS;
static ubxNavStatus_t   gpsNAVSTATUS;
static bool_t            gpsReceivedTIMELS;  // TODO: Add resets -- these are set & checked but never reset to false!
static bool_t            gpsReceivedNAVPVT;
static bool_t            gpsReceivedHNRPVT;
static bool_t            gpsReceivedNAVDOP;
static bool_t            gpsReceivedNAVEELL;
static bool_t            gpsReceivedESFINS;
static bool_t            gpsReceivedESFRAW;
static time_t           last_HNR_received_sec = 0;

/* variables used to trigger a backup of the AssistNow Data */
bool_t assistNowConfirmedIdle = FALSE;
bool_t startAssistNowBackup = FALSE; 
static time_t           last_AOPSTATUS_requested_sec = 0;
static time_t           last_AssistNowBackup_requested_sec = 0;


#ifdef DBG_ENABLE_SEND_TO_UBLOX
static time_t           last_RINV_requested_sec = 0;
static time_t           last_MONVER_requested_sec = 0;
static time_t           last_MGAGPS_sent_sec = 0;
#endif /*DBG_ENABLE_SEND_TO_UBLOX*/

static int32_t          isValidGroup=FALSE;

static int32_t          gpsGrpReftime;
#ifdef DBG_SYSTEMTIMEUPDATE
static int32_t          lastGpsGrpReftime=999999;
static uint8_t          lastPvtSecond=99;
#endif /*DBG_SYSTEMTIMEUPDATE*/

static int32_t          gpsGrpRef_deci;
extern request_list     req_list;
extern pthread_mutex_t  tpsRawData_mutex;

/* Variables Used for UBX Host Interface Signing Verification */
#define NAV_PVT_UBX_MSG_LENGTH (100)
#define HNR_PVT_UBX_MSG_LENGTH (80)
#define UBX_SEC_SIGN_SEEDS (16)

const  uint8_t  FIXED_SEED_HIGH[4] = {0xEF, 0xBE, 0xAD, 0xDE};
const  uint8_t  FIXED_SEED_LOW[4] = {0xEF, 0xBE, 0xAD, 0xDE};
static uint8_t  nav_pvt_sha256_hash[32];
static uint8_t  hnr_pvt_sha256_hash[32];
static uint8_t  nav_pvt_sec_sign_failed;
static uint8_t  nav_pvt_sec_sign_fail_count;
static uint8_t  hnr_pvt_sec_sign_failed;
static uint8_t  hnr_pvt_sec_sign_fail_count;


#ifdef GPS_UPDATE_LINUX_SYSTIME
#include <sys/mman.h>
#include <fcntl.h>
#include <pps_mmap.h>
#include <time.h>
volatile struct pps_tmstamp      pps_shrd_mem = {0};        // Used to be shared mem, now just a global var shared by threads

#ifdef DBG_SYSTEMTIMEUPDATE
static int32_t          isPPSTmStmpValid = FALSE;
static int32_t          PPSavailable = FALSE;
////TODO: prev is a BADLY used global, Should be localized to gpsUpdateLinuxSysTime() and passed to calculateDeltaTime()!!!
static struct timeval   prev;       // Used by calculateDeltaTime()
////TODO: Localize deltaSum to gpsUpdateLinuxSysTime()
static struct timeval   deltaSum;   // Used by doFreqAdjustment() & gpsUpdateLinuxSysTime()

#define TPS_LOWER_TIME_US 160       //Lower end of desired accruaccy (in msec).
static int32_t          ppsCount=1;
static uint32_t         TimeAdjCount,FreqAdjCount,TimeSetCount;
static uint32_t         isrLatencyCheck;
static int32_t          ppsValidCount;
#endif /*DBG_SYSTEMTIMEUPDATE*/

int32_t linuxSetSystemTimeOnce=0;

#endif  // GPS_UPDATE_LINUX_SYSTIME

#if defined(HEAVY_DUTY)
/* Gets cleared every epoch so maintain. */
static uint8_t  gps_data_source_message_type = 0x0;
#endif

/* Returns the time, in seconds, that the system has been powered up */
long get_cur_uptime()
{
    struct sysinfo s_info;
    return (sysinfo(&s_info) == 0) ? s_info.uptime : 0;
}

#if defined(ENABLE_SR)
extern shm_sr_t  * shm_sr_ptr;
static void gpsCommPlaybackActive(void);
extern int32_t sendSRMessage(uint8_t interfaceId, uint8_t messageId, uint8_t *data, uint32_t datalength); // in tps.c
#endif

/*----------------------------------------------------------------------------*/
/* Forward Declarations                                                       */
/*----------------------------------------------------------------------------*/

/* Function prototypes for GPS Comm */

static int32_t gpsCommInit(void);
static void gpsCommDenit(void);
static void gpsCommActive(void);
static void gpsCommMessageReceived(void);
static void gpsCommParseGprmc(void);
static void gpsCommParseGpgga(void);
static void gpsCommSendTpsDataIndFromUbxData(void);
static void gpsCommSendTpsDataIndFromNmeaData(void);
static void gpsCommParseGpgsa(void);
static void gpsCommParseGpgst(void);
static void gpsSendTpsRTCMDataInd(void);
static void gpsSendTpsRAWDataInd(tpsRawMsgRes *raw_data);
static bool_t gpsValidateNMEAChecksum(uint8_t *curmsg, uint32_t count);
static int32_t parse_gpsCmd_into_tokens( const uint8_t *str, char tokentab[][NMEA_MSG_MAX_TOKEN_LENGTH+1], const char *delimiters );

#ifdef DBG_ENABLE_SEND_TO_UBLOX
int32_t SendMsgToUblox(uint8_t *msg, uint16_t msglen);
#endif /*DBG_ENABLE_SEND_TO_UBLOX*/

void Compute8BitFletcherChecksums(uint8_t *buffer, uint16_t bufflen, uint8_t *csum1);
void printLastUblox();
void receivedUBX_NAV_TIMELS();
void receivedUBX_NAV_PVT();
void receivedUBX_NAV_DOP();
void receivedUBX_NAV_EELL();
void receivedUBX_SEC_SIGN();
void receivedUBX_HNR_INS();
void receivedUBX_HNR_PVT();
void receivedUBX_CFG_RINV(uint16_t payload_len);
void receivedUBX_MON_VER(uint16_t payload_len);
#if defined(ENABLE_WHEELTIX)
void receivedUBX_ESF_INS();
void receivedUBX_ESF_MEASURE(uint16_t payload_len);
void receivedUBX_ESF_STATUS(uint16_t payload_len);
#endif /* ENABLE_WHEELTIX */
int32_t I4Convert(uint8_t *buff);
int32_t I3Convert(uint8_t *buff);
uint32_t U4Convert(uint8_t *buff);
uint16_t U2Convert(uint8_t *buff);

#ifdef DBG_SYSTEMTIMEUPDATE
void gpsUpdateLinuxSysTime(int32_t x);
int ppsFindSource(char *path, pps_handle_t *handle, int *avail_mode);
int ppsFetchPpsSource(pps_handle_t handle, int avail_mode);
static void debug_print(time_t   time_gps, time_t sys_sec, int64_t sys_usec,
                        int32_t  prev_pps_used, int32_t latency_exceded, int64_t delta,
                        int64_t  avgoffset,     uint32_t adj_systime,
                        uint32_t freq_systime,  uint32_t set_systime );
#endif /*DBG_SYSTEMTIMEUPDATE*/

#if defined(HEAVY_DUTY)

void init_internal_statics(void)
{
    memset(&gpsData,0x0,sizeof(gpsData));
    gpsTpsSockFd = 0;
    memset(gpsCommandBuffer,'\0',sizeof(gpsCommandBuffer));
    chars_in_buffer = 0;
    msgMunchingStage = STAGE_0_AWAITING_START_OF_NEXT_COMMAND;
    RTCMMsgTotalLen = 0;
    memset(&gpsGGA,0x0,sizeof(gpsGGA));
    memset(&gpsRMC,0x0,sizeof(gpsRMC));
    memset(&gpsGPGSA,0x0,sizeof(gpsGPGSA));
    memset(&gpsGPST,0x0,sizeof(gpsGPST));

    gpsReceivedGGA   = FALSE;
    gpsReceivedRMC   = FALSE;
    gpsReceivedGPGSA = FALSE;
    gpsReceivedGPGST = FALSE;

    memset(&gpsTIMELS,0x0,sizeof(gpsTIMELS));
    memset(&gpsNAVPVT,0x0,sizeof(gpsNAVPVT));
    memset(&gpsNAVDOP,0x0,sizeof(gpsNAVDOP));
    memset(&gpsNAVEELL,0x0,sizeof(gpsNAVEELL));
    memset(&gpsESFINS,0x0,sizeof(gpsESFINS));
    memset(&gpsESFRAW,0x0,sizeof(gpsESFRAW));
    memset(&gpsHNRINS,0x0,sizeof(gpsHNRINS));

    gpsReceivedTIMELS  = FALSE;
    gpsReceivedNAVPVT  = FALSE;
    gpsReceivedHNRPVT  = FALSE;
    gpsReceivedNAVDOP  = FALSE;
    gpsReceivedNAVEELL = FALSE;
    gpsReceivedESFINS  = FALSE;
    gpsReceivedESFRAW  = FALSE;

    gps_data_source_message_type = 0x0;

    last_HNR_received_sec = 0;
    isValidGroup      = FALSE;
    gpsGrpReftime     = 0;
    lastGpsGrpReftime = 999999;
    lastPvtSecond     = 99;
    gpsGrpRef_deci    = 0;
}
#endif /* HEAVY_DUTY */

/**----------------------------------------------------------------------------
** @brief  Print NMEA string with length blocked 20 chars/line
** @param  unused
** @return unused
**---------------------------------------------------------------------------*/
#if defined(EXTRA_DEBUG)
static void printNMEAstringHex(char_t *data, int32_t len)
{
    int32_t ix;
    uint8_t  cs = 0;

        for (ix=0; ix<len; ix++)
        {
            if (ix > 0 && ix % 20 == 0)
            {
               I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"\n");
            }
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"%.2x ", data[ix]);
        }

        //Calc ^ Checksum
        for (ix=1; ix<len; ix++)
        {
            if (data[ix] != '*') {
                cs = cs ^ (data[ix]);
            } else {
                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Checksum=%d(0x%x), #chars=%d\n",cs,cs,ix);
                break;
            }
        }
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"\n");
}


/**----------------------------------------------------------------------------
** @brief  Print NMEA string with length on 1 line
** @param  unused
** @return unused
**---------------------------------------------------------------------------*/
static void printNMEAstring(char_t *data, int32_t len)
{
    int32_t ix;

        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(len=%02d): |",len);
        ////I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"|%s|<--\n",data);

        for (ix = 0; ix < len; ix++)
        {
            if ( data[ix] == 0x00 )
            {
                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,".\n");
            }
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"%c", data[ix]);
        }
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"|\n");
}

#endif
// Grab the current time in msecs.
uint64_t get_cur_msec_time()
{
    struct timeval sys_tv;
    uint64_t v;

	  gettimeofday(&sys_tv, NULL);
    v = sys_tv.tv_sec*1000LL + sys_tv.tv_usec/1000LL;
    return v;
}

/* GPS output check thread */

/**----------------------------------------------------------------------------
** @brief  Watchdog thread to look for output stalling and send error reports
** @param  unused
** @return unused
**---------------------------------------------------------------------------*/
void *gpsOutputCheckThreadFunc(void *arg)
{
    static int32_t  prevNumCharsRead        = -1;
    static uint8_t  sendPPSOkayReport       = TRUE;
    static uint32_t prev_pps_shrd_mem_count = 0;

    sleep(3);
    while (tpsMainLoop == TRUE)
    {
#if defined(ENABLE_SR)
        /* Don't do this error processing when in SR playback mode */
        if (   (NULL                  == shm_sr_ptr)
            || (SR_PLAYBACK_MODE_CHAR != shm_sr_ptr->sr_mode)
           )
#endif
        {
//TODO: Is it an error if there hasn't been 1 pps yet. Both equal zero?
            // Make sure the PPS count is increasing or throw an error report upstairs
            if (prev_pps_shrd_mem_count == pps_shrd_mem.count)
            {
                tpsErrorReport(GPS_NO_PPS_OUTPUT);
                sendPPSOkayReport = FALSE;
#if defined(EXTRA_DEBUG)
                ////if (tpsDebugOutput & DBG_OUTPUT_PPS)  ////DBGDBG
                {
                    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"%s:--------->tpsDebugOutput=0x%04x, PPS pulse count=%d\n",
                            __func__, tpsDebugOutput, pps_shrd_mem.count);
                }
#endif
                shm_tps_ptr->debug_cnt_1pps_halt_event++;
            }
            else
            {
                prev_pps_shrd_mem_count = pps_shrd_mem.count;
                if (!sendPPSOkayReport)
                {
                    tpsErrorReport(GPS_SUCCESS_PPS_OUTPUT);
                    sendPPSOkayReport = TRUE;
                }
#if defined(EXTRA_DEBUG)
                if (tpsDebugOutput & DBG_OUTPUT_PPS)
                {
                    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"%s:--------->[OK]  PPS pulse count=%d\n",__func__, pps_shrd_mem.count);  ////DBGDBG
                }
#endif
            }

            // No change since 3 seconds ago == bad
            if (prevNumCharsRead != -1 && prevNumCharsRead == shm_tps_ptr->debug_cnt_chars_read) {
                tpsErrorReport(GPS_NO_NMEA_OUTPUT);  // Actually applies to UBX msgs too now
                shm_tps_ptr->error_states |= GPS_MSG_HALT_ERROR; 
            }
        }

        prevNumCharsRead = shm_tps_ptr->debug_cnt_chars_read;

        sleep(3);
#if defined(MY_UNIT_TEST)
        tpsMainLoop = WFALSE;
#endif
    }
    return 0;
}

/**----------------------------------------------------------------------------
** @brief  Thread to get the system time when PPS signal received  & place in SHM.
** @param  unused
** @return unused
**---------------------------------------------------------------------------*/
void *gpsGetPPStimeThreadFunc(void *arg)
{
  pps_handle_t handle;
  int32_t avail_mode = 0;
  int32_t ret = 0;

  if ((ret = ppsFindSource(GPS_PPS_SOURCE_PATH, &handle, &avail_mode)) < 0){
#if defined(EXTRA_DEBUG)
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"GPS_FIND_1PPS_ERROR: ret(%d)\n",ret);
#endif
      shm_tps_ptr->error_states |= GPS_FIND_1PPS_ERROR; 
  } else {
#if defined(EXTRA_DEBUG)
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"%s: rtn=%d, handle=%d\n",__func__,ret,handle);
#endif
      while (tpsMainLoop == TRUE) {
          /* Get PPS time */
          ret = ppsFetchPpsSource(handle, avail_mode);
          if ((ret < 0) && (errno == EINTR)) {
              ret = 0;
              break;
          }
          if ((ret < 0) && (errno != ETIMEDOUT))
              break;

#if defined(MY_UNIT_TEST)
        tpsMainLoop = WFALSE;
#endif
      }
      time_pps_destroy(handle);
      sleep(3);
  }
  shm_tps_ptr->error_states |= GPS_1PPS_THREAD_EXIT;
#if defined(EXTRA_DEBUG)
  I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"gpsGetPPStimeThreadFunc exiting: ret(%d)\n",ret);
#endif
  return NULL;
}

/* GPS Comm Thread */
void *gpsCommThreadFunc(void* arg)
{
#if defined(EXTRA_DEBUG)
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"* TPS Built for RSU SERIAL_GPS\n");
#endif
    /* This will connect our GPS thread up to the TPS thread's receiving socket */
    gpsTpsSockFd = wsuConnectSocket(TPS_RECV_GPSDATA_PORT);

    if(0 == gpsCommInit()) {
        #if defined(ENABLE_SR)
        if (   (NULL                  != shm_sr_ptr)
            && (SR_PLAYBACK_MODE_CHAR == shm_sr_ptr->sr_mode)) {
            gpsCommPlaybackActive();
        }
        else
        #endif
        {
            gpsCommActive();
        }
    } else {
        shm_tps_ptr->error_states |= GPS_COMM_THREAD_EXIT;
#if defined(EXTRA_DEBUG)
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"gpsCommThreadFunc Exiting.\n");
#endif
    }
    shm_tps_ptr->error_states |= GPS_COMM_THREAD_EXIT;
#if defined(EXTRA_DEBUG)
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"gpsCommThreadFunc Exiting.\n");
#endif
    gpsCommDenit();
    pthread_exit(0);
}


/* AssistNow Backup Thread */
void *gpsAssistNowBackupThreadFunc(void* arg)
{
#if defined(EXTRA_DEBUG)
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"AssistNow Backup Thread started\n");
#endif    
    while (tpsMainLoop == TRUE) {
        if ((time(0L) - last_AssistNowBackup_requested_sec > 86400 ) && //Backup every 24 hours 
            (get_cur_uptime() > 1800)) // Backup after unit has been up for 30 minutes
        {
            last_AssistNowBackup_requested_sec = time(0L);
            startAssistNowBackup = TRUE;
        }
#if defined(MY_UNIT_TEST)
        tpsMainLoop = WFALSE;
#endif
        sleep(60);
    }
#if defined(EXTRA_DEBUG)
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"AssistNow Backup Thread ended\n");
#endif
    pthread_exit(0);
}

/**
 * @brief Enables/Disables the UBX and NMEA messages from the ublox gps
 * @description This is only for debugging, to switch between message sets.
 *     The real configuration should be done at build time with the large
 *     ubx configuration file and a config utility.
 */
void SendUbloxMessageSetup(int32_t rateUBX, int32_t rateNMEA, int32_t rateNMEA_GST)
{
    uint8_t msg_UBX_CFG_MSG[] = { // UBX_CFG_MSG w Set Rate for each Port
        0xB5, 0x62,             // Header
        0x06,                   // Class
        0x01,                   // ID
        0x08, 0x00,             // Length of 3, unsigned 16-bit little endian format
        // -- Payload --
        0x00,                   // Msg Class - 1 == UBX-NAV, F0=NMEA
        0x00,                   // Msg ID - done below
        0x00,                   // Msg Rate on I/O port 0   I2C
        0x00,                   // Msg Rate on I/O port 1   UART1
        0x00,                   // Msg Rate on I/O port 2   UART2
        0x00,                   // Msg Rate on I/O port 3   USB
        0x00,                   // Msg Rate on I/O port 4   SPI
        0x00,                   // Msg Rate on I/O port 5
        // -- CRC --
        0x00, 0x00             // CRC goes here
    };

    int32_t i;

    // RMC=0xF004 GGA=0xF000 GSA=0xF002 GST=xF007
    uint8_t NMEA_ClassMsgId_pairs[] = {0xF0, 0x04, 0xF0, 0x00, 0xF0, 0x02};

    // 0x0107=NAV-PVT 0x0126=NAV-TIMELS 0x0104=NAV-DOP 0x013D=NAV-EELL
    uint8_t UBX_ClassMsgId_pairs[] = {  0x01, 0x07
                                      , 0x01, 0x26
                                      , 0x01, 0x04
                                      , 0x01, 0x3D
                                      , 0x0A, 0x09
                                      , 0x0A, 0x04
                                     };
    uint8_t NMEA_GST_alone[] = {0xF0, 0x07}; // GST=xF007 is special .. needs to be in and UBX and NMEA modes!
    /*
    uint8_t DeadRec_ClassMsgId_pairs[] = {    // These msgs were part of UBX during development, unwanted for now, keeping around for future dev
                                      // -- These messages we'll get when Dead-Reckoning is activated
                                      0x28, 0x00,                          // 0x2800=HNR-PVT
                                      0x10, 0x03,
                                      0x10, 0x15,              // 0x1003=ESF-RAW 0x1015=ESF-INS
                                      // -- Unused msgs included in our original UBX config file --
                                      0x01, 0x35,
                                      0x01, 0x03,              // 0x0135=NAV-SAT 0x0103=NAV-STATUS
                                      0x10, 0x10,
                                      0x10, 0x14               // 0x1010=ESF-STATUS  0x1014=ESF-not_in_docs_msg
                                     };
    */

    /* UBX messages */
    for (i=0; i<sizeof(UBX_ClassMsgId_pairs)/sizeof(uint8_t); i+=2) {
        msg_UBX_CFG_MSG[6] = UBX_ClassMsgId_pairs[i]; // ClassId
        msg_UBX_CFG_MSG[7] = UBX_ClassMsgId_pairs[i+1]; // MsgId
        msg_UBX_CFG_MSG[8] = 0;         // Port 0 = I2C
        msg_UBX_CFG_MSG[9] = 0;         // Port 1 = UART1
        msg_UBX_CFG_MSG[10] = 0;        // Port 2 = UART1
        msg_UBX_CFG_MSG[11] = rateUBX;  // Port 3 = USB
        msg_UBX_CFG_MSG[12] = rateUBX;  // Port 4 = SPI
        SendMsgToUblox(msg_UBX_CFG_MSG, sizeof(msg_UBX_CFG_MSG));
    }

    /* NMEA messages */
    for (i=0; i<sizeof(NMEA_ClassMsgId_pairs)/sizeof(uint8_t); i+=2) {
        msg_UBX_CFG_MSG[6] = NMEA_ClassMsgId_pairs[i]; // ClassId
        msg_UBX_CFG_MSG[7] = NMEA_ClassMsgId_pairs[i+1]; // MsgId
        msg_UBX_CFG_MSG[8] = 0;         // Port 0 = I2C
        msg_UBX_CFG_MSG[9] = 0;         // Port 1 = UART1
        msg_UBX_CFG_MSG[10] = 0;        // Port 2 = UART1
        msg_UBX_CFG_MSG[11] = rateNMEA; // Port 3 = USB
        msg_UBX_CFG_MSG[12] = rateNMEA; // Port 4 = SPI
        SendMsgToUblox(msg_UBX_CFG_MSG, sizeof(msg_UBX_CFG_MSG));
    }

    /* And for Mr. Special NMEA himself, GST!! */
    msg_UBX_CFG_MSG[6] = NMEA_GST_alone[0]; // ClassId
    msg_UBX_CFG_MSG[7] = NMEA_GST_alone[1]; // MsgId
    msg_UBX_CFG_MSG[8] = 0;         // Port 0 = I2C
    msg_UBX_CFG_MSG[9] = 0;         // Port 1 = UART1
    msg_UBX_CFG_MSG[10] = 0;        // Port 2 = UART1
    msg_UBX_CFG_MSG[11] = rateNMEA_GST; // Port 3 = USB
    msg_UBX_CFG_MSG[12] = rateNMEA_GST; // Port 4 = SPI
    SendMsgToUblox(msg_UBX_CFG_MSG, sizeof(msg_UBX_CFG_MSG));
}

/**
 * @brief Sends a CFG-RINV request to the ublox.  UBX-CFG-RINV (0x06 0x34)
 */
void SendRinvRequest()
{
    uint8_t msg_UBX_CFG_MSG[] = { // UBX_CFG_MSG w Set Rate for each Port
        0xB5, 0x62,             // Header
        0x06,                   // Class
        0x34,                   // ID
        0x00, 0x00,             // Payload len of 0 makes this a poll message
        // -- CRC --
        0x00, 0x00             // CRC goes here
    };
    SendMsgToUblox(msg_UBX_CFG_MSG, sizeof(msg_UBX_CFG_MSG));
}

/*
 * Sends a UBX-MON-VER request to the ublox.  UBX-MON-VER (0x0A 0x04)
 */
void SendMonVerRequest()
{
    uint8_t msg_UBX_POLL_MSG[] = {
        0xB5, 0x62,             // Header
        0x0A,                   // Class
        0x04,                   // ID
        0x00, 0x00,             // Payload len of 0 makes this a poll message
        // -- CRC --
        0x00, 0x00             // CRC goes here
    };
    SendMsgToUblox(msg_UBX_POLL_MSG, sizeof(msg_UBX_POLL_MSG));
}

/*
 * Sends a UBX-MGA-GPS-UTC request to the ublox.  UBX-MGA-GPS-UTC (0x13 0x00 + 0x05 Type)
 */
void SendMgaGpsMessage(uint8_t leapsecs)
{
    uint8_t msg_UBX_MGA_GPS_UTC[] = {
        0xB5, 0x62,             // Header
        0x13,                   // Class
        0x00,                   // ID
        0x14, 0x00,             // Payload len of 20 bytes
        0x05,                   // Type
        0x00,                   // Version
        0x00,                   // Reserved
        0x00,                   // Reserved
        0x00, 0x00, 0x00, 0x00, // First parameter of UTC polynomial
        0x00, 0x00, 0x00, 0x00, // Second parameter of UTC polynomial
        leapsecs,               // Delta time due to current leap seconds
        0x00,                   // UTC parameters reference time of week (GPS time)
        0x00,                   // UTC parameters reference week number
        0x00,                   // Week number at the end of which future leap second is effective
        0x00,                   // Day number at the end of which future leapsec becomes effective
        leapsecs,               // Delta time due to future leap second
        0x00, 0x00,             // Reserved
        // -- CRC --
        0x00, 0x00             // CRC goes here
    };
    SendMsgToUblox(msg_UBX_MGA_GPS_UTC, sizeof(msg_UBX_MGA_GPS_UTC));
}


/**
 * @brief Sends a CFG-TIMELS request to the ublox.  UBX-CFG-TIMELS (0x06 0x34)
 */
void Send_TIMELS_Request(uint8_t desired_msg_rate)
{
    uint8_t msg_UBX_CFG_MSG[] = {   // UBX_CFG_MSG w Set Rate for each Port
        0xB5, 0x62,                 // Header
        0x06,                       // Class
        0x01,                       // ID
        0x08, 0x00,                 // Length of 8, unsigned 16-bit little endian format
        // -- Payload --
        0x01,                       // Msg Class - 1 == UBX-NAV
        0x26,                       // Msg ID - 26 - TIMELS
        0x00,                       // Msg Rate on I/O port 0   I2C
        0x00,                       // Msg Rate on I/O port 1   UART1
        0x00,                       // Msg Rate on I/O port 2   UART2
        desired_msg_rate,           // Msg Rate on I/O port 3   USB
        desired_msg_rate,           // Msg Rate on I/O port 4   SPI
        0x00,                       // Msg Rate on I/O port 5   Reserved
        // -- CRC --
        0x00, 0x00                  // CRC will be put here
    };

    SendMsgToUblox(msg_UBX_CFG_MSG, sizeof(msg_UBX_CFG_MSG));
}

/*
 * Sends a UBX-UPD-SOS AssistNow backup request to the ublox.  
 */
void SendAssistNowBackupRequest()
{
    uint8_t msg_Backup_AssistNow[] = {
        0xB5, 0x62,             // Header
        0x09,                   // Class
        0x14,                   // ID
        0x04, 0x00,             // Length of 4
        // -- Payload -- 
        0x00,
        0x00, 
        0x00, 
        0x00,
        // -- CRC --
        0x00, 0x00             // CRC goes here
    };
    SendMsgToUblox(msg_Backup_AssistNow, sizeof(msg_Backup_AssistNow));
}

/*
 * Sends a UBX-NAV-AOPSTATUS request to the ublox.  
 */
void SendAOPStatusRequest()
{
    uint8_t msg_AOPSTATUS_POLL_MSG[] = {
        0xB5, 0x62,             // Header
        0x01,                   // Class
        0x60,                   // ID
        0x00, 0x00,             // Payload len of 0 makes this a poll message
        // -- CRC --
        0x00, 0x00             // CRC goes here
    };
    SendMsgToUblox(msg_AOPSTATUS_POLL_MSG, sizeof(msg_AOPSTATUS_POLL_MSG));
}


#if defined(HEAVY_DUTY)
#if defined(EXTRA_DEBUG)
static uint32_t my_enable_hw_status = 0x0;
#endif
void SendDumpGNSSHWStatusRequest(void)
{
  //This requires UBX debug messages are enabled: option 'a'
  uint8_t my_msg_UBX_CFG_MSG[66];

  SendUbloxMessageSetup(1, 0, 0);
  usleep(1000);
  memset(my_msg_UBX_CFG_MSG,0x0,66);

  my_msg_UBX_CFG_MSG[0] = 0xB5; //Header
  my_msg_UBX_CFG_MSG[1] = 0x62;
  my_msg_UBX_CFG_MSG[2] = 0x0A; //Class
  my_msg_UBX_CFG_MSG[3] = 0x09; //ID

  SendMsgToUblox(my_msg_UBX_CFG_MSG, sizeof(my_msg_UBX_CFG_MSG));
}

void DumpUbloxGNSSConfigs(void)
{

  uint8_t CFG_GET_GNSS_MSG[] = { 0xB5
                                ,0x62
                                ,0x06
                                ,0x3E
                                ,0x00
                                ,0x00
                                ,0x00
                                ,0x00};

  SendMsgToUblox(CFG_GET_GNSS_MSG, sizeof(CFG_GET_GNSS_MSG));

}

void DumpUbloxConfigs(void) /* Make sure enabled */
{
  uint8_t my_msg_UBX_CFG_MSG[8];

  my_msg_UBX_CFG_MSG[0] = 0xB5;
  my_msg_UBX_CFG_MSG[1] = 0x62;
  my_msg_UBX_CFG_MSG[2] = 0x0A;
  my_msg_UBX_CFG_MSG[3] = 0x04;
  my_msg_UBX_CFG_MSG[4] = 0x00;
  my_msg_UBX_CFG_MSG[5] = 0x00;
  my_msg_UBX_CFG_MSG[6] = 0x0E;
  my_msg_UBX_CFG_MSG[7] = 0x34;

  SendMsgToUblox(my_msg_UBX_CFG_MSG, sizeof(my_msg_UBX_CFG_MSG));

  DumpUbloxGNSSConfigs();
}

void SendDefaultUbloxCfgs(void)
{

}

void SendGNSSStopRequest(void)
{
#if 0 /* HD Note: Stopped it...but could NOT re-start it. Avoid. */
  static uint8_t GNSS_stop_start = 0x1; /* Default assume we are started */
  uint8_t my_msg_UBX_CFG_MSG[12] = {
        0xB5,                   // Header
        0x62,                   //
        0x06,                   // Class
        0x04,                   // ID
        0x04,                   // len LSB
        0x00,                   // len MSB
        0xFF,                   // BBR factory reset, software reset
        0xFF,                   // BBR factory reset, software reset
        0x08,                   // reset type: 0x8 = GNSS STOP, 0x9 = GNSS START
        0x00,                   // reserved
        0x00, 0x00              // CRC goes here
  };

  if(1 == GNSS_stop_start) {
    my_msg_UBX_CFG_MSG[8] = 0x08; //Stop it
    GNSS_stop_start = 0;
  } else {
    my_msg_UBX_CFG_MSG[8] = 0x09;  //Start it
    GNSS_stop_start = 1;
  }

  SendMsgToUblox(my_msg_UBX_CFG_MSG, sizeof(my_msg_UBX_CFG_MSG));
#endif
}

void SendCFGRSTRequest(void)
{
  /* These are from TJ at uBlox: Cold start+Factory */
  /* Warning! If you erase NV contents you'll need to reload default cfgs! */
  uint8_t my_msg_UBX_CFG_MSG[12] = {
        0xB5,                   // Header
        0x62,                   //
        0x06,                   // Class
        0x04,                   // ID
        0x04,                   // len LSB
        0x00,                   // len MSB
//Does this clear config from chip?
        0xFF,                   // BBR factory reset, software reset
        0xFF,                   // BBR factory reset, software reset
        0x02,                   //  Controlled Software reset (GNSSonly)
        //0x01,                 //  Complete Reset?
       // 0x00,                 //  ruthless shutdown?
        0x00,                   // reserved
        0x00, 0x00              // CRC goes here
  };

  SendMsgToUblox(my_msg_UBX_CFG_MSG, sizeof(my_msg_UBX_CFG_MSG));
  sleep(1);
  gps_data_source_message_type = 0x0;

}
#endif /*HEAVY_DUTY*/

/**----------------------------------------------------------------------------
** @brief  GPS Comm Initialization State Processing
** @param  unused
** @return unused
**---------------------------------------------------------------------------*/
static int32_t gpsCommInit(void)
{
  int32_t ret = 0;
  int32_t rc = 0;

  /* Initialize variables. */
  gpsReceivedGGA = FALSE;  /* Initially indicate GGA has not been received. */
  gpsReceivedRMC = FALSE;  /* Initially indicate RMC has not been received. */

  /* Open the GPS serial port. */

  /* Configure ubloxM8 serial port for receiving UBX binary data. */
  if (EOK == (rc = system(GPS_SERIAL_CONFIG_CMD))){
#if defined(EXTRA_DEBUG)
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Configured ublox GNSS serial port (%s): %s\n",GPS_SERIAL_DEVICE_PATH,GPS_SERIAL_CONFIG_CMD);
#endif
      if (0 <= (gpsFd = open(GPS_SERIAL_DEVICE_PATH, O_RDWR))) {
          pps_shrd_mem.NewTimeStamp = 0; /* Reset PPS indicators */
          if (0 != (rc = pthread_mutex_init(&tpsRawData_mutex, NULL))){ /* Initialize our raw gps mutex */
#if defined(EXTRA_DEBUG)
              I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"GPS_MUTEX_INIT_ERROR: rc(%d) errno(%d)\n",rc, errno);
#endif
              shm_tps_ptr->error_states |= GPS_MUTEX_INIT_ERROR;
              ret = GPS_MUTEX_INIT_ERROR;
          }
      } else {
          gpsFd = -1;
#if defined(EXTRA_DEBUG)
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"GPS_SERIAL_OPEN_ERROR: rc(%d) errno(%d)\n",rc,errno);
#endif
          shm_tps_ptr->error_states |= GPS_SERIAL_OPEN_ERROR;
          ret = GPS_SERIAL_OPEN_ERROR;
      }
  } else {
#if defined(EXTRA_DEBUG)
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"GPS_SERIAL_CONFIG_CMD_ERROR: rc(%d) errno(%d)\n",rc,errno);
#endif
      shm_tps_ptr->error_states |= GPS_SERIAL_CONFIG_CMD_ERROR;
      ret = GPS_SERIAL_CONFIG_CMD_ERROR;
  }
  return ret;
}


/*-----------------------------------------------------------------------------
** @brief  GPS Comm De-Initialization / Shutdown Processing
** @param  unused
** @return unused
**---------------------------------------------------------------------------*/
static void gpsCommDenit(void)
{
    /* Clean up */
    close(gpsFd);
}




/*-----------------------------------------------------------------------------
** @brief Removes leftmost bytes from gpsCommandBuffer[] until a new string starts
** @description Used for removing incomplete command strings from buffer, usually
**              the first string at TPS startup is incomplete and needs to be
**              dropped.
**---------------------------------------------------------------------------*/
static void Dump_GPS_Buffer_Up_To_Next_String_Start()
{
    int32_t ii;

    // Look for a NMEA/RTCM/UBX start byte.  Note we are starting at i=1, not i=0,
    //    because any start byte at [0] must be invalid or we wouldn't be in here, so
    //    we need to delete at least the [0].
    int32_t buffer_cut_position = -1;


    for (ii=1; ii<chars_in_buffer && buffer_cut_position < 0; ii++)
    {

        if (gpsCommandBuffer[ii] == 0x24 || gpsCommandBuffer[ii] == 0xAA || gpsCommandBuffer[ii] == 0xB5)
        {
            buffer_cut_position = ii;
        }
    }

    /* If we didn't find anything, toss the entire buffer */
    if (buffer_cut_position < 0) {
        shm_tps_ptr->debug_cnt_unused_chars += chars_in_buffer;
        chars_in_buffer = 0;
    }

    /* Copy "keep" bytes to front */
    else {
        shm_tps_ptr->debug_cnt_unused_chars += buffer_cut_position;
        uint32_t new_num_chars_in_buffer = chars_in_buffer - buffer_cut_position;
           // v-- NOTE: Its ok if src & dest overlap because we are using memmove(), not memcpy() --v
        memmove(&gpsCommandBuffer[0], &gpsCommandBuffer[buffer_cut_position], new_num_chars_in_buffer);
        chars_in_buffer = new_num_chars_in_buffer;

    }
}



/*-----------------------------------------------------------------------------
** @brief       Removes N leftmost bytes from gpsCommandBuffer[]
** @description Used for removing processed commands
**---------------------------------------------------------------------------*/
static void Remove_First_N_Bytes_From_GPS_Buffer(uint8_t n)
{
    // Memmove or set len = 0?
    if (chars_in_buffer <= n) {
        chars_in_buffer = 0;   // Easy!
    // Memmove
    } else {
        uint8_t new_num_chars_in_buffer = chars_in_buffer - n;
            // v-- NOTE: Its ok if src & dest overlap because we are using memmove(), not memcpy() --v
        memmove(&gpsCommandBuffer[0], &gpsCommandBuffer[n], new_num_chars_in_buffer);
        chars_in_buffer = new_num_chars_in_buffer;
    }
}


/* GPS Message Processing Functions */

#define UBXHDRLEN 6


/*-----------------------------------------------------------------------------
** @brief  Process received UBX NAV-TIMELS message.
** @param  unused
** @return unused
**---------------------------------------------------------------------------*/
void receivedUBX_NAV_TIMELS()
{
    memset(&gpsTIMELS, 0, sizeof(gpsTIMELS));

    uint8_t valid_currLeapTs = gpsCommandBuffer[23 + UBXHDRLEN] & 0x01;

    if (valid_currLeapTs)
    {
        gpsTIMELS.curLeapSecs      = (int8_t) gpsCommandBuffer[9 + UBXHDRLEN];
        gpsTIMELS.srcOfCurLeapSecs = gpsCommandBuffer[8 + UBXHDRLEN];

        uint8_t valid_timeToLeapTsEvent = gpsCommandBuffer[23 + UBXHDRLEN] & 0x02;

        if (valid_timeToLeapTsEvent) {
            gpsTIMELS.upcomingLeapSecChange      = (int8_t) gpsCommandBuffer[11 + UBXHDRLEN];
            gpsTIMELS.timeToLeapSecEvent_secs    = I4Convert(&gpsCommandBuffer[12 + UBXHDRLEN]);
            gpsTIMELS.srcOfUpcomingLeapSecChange = gpsCommandBuffer[8 + UBXHDRLEN];

            shm_tps_ptr->upcomingLeapSecChange   = gpsTIMELS.upcomingLeapSecChange;
            shm_tps_ptr->timeToLeapSecEvent_secs = gpsTIMELS.timeToLeapSecEvent_secs;
        }

        gpsReceivedTIMELS             = TRUE;
        shm_tps_ptr->leap_secs_valid  = TRUE;
        shm_tps_ptr->curLeapSecs      = gpsTIMELS.curLeapSecs;
        shm_tps_ptr->srcOfCurLeapSecs = gpsTIMELS.srcOfCurLeapSecs;
        // Save the last good leapsec so we can fill it in later if TIMELS stops or goes invalid
        last_good_leapsec = gpsTIMELS.curLeapSecs;
        // If leapsecs changed and persistency is on, store it to disk
        if (cfg_persistency_enable && shm_tps_ptr->curLeapSecs != savedTpsVariables.curLeapSecs) {
            savedTpsVariables.curLeapSecs = shm_tps_ptr->curLeapSecs;
            savedTpsVariables.srcOfCurLeapSecs = shm_tps_ptr->srcOfCurLeapSecs;
            flag_request_to_save_tps_variables = TRUE;  // Set flag to save them to disk
#if defined(EXTRA_DEBUG)
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, " Requested save of TPS variables\n");
#endif
        }
    } else {
        ++shm_tps_ptr->debug_gps_cmd_count_ubx_nav_timels_invalid;
        gpsReceivedTIMELS             = FALSE;
        shm_tps_ptr->leap_secs_valid  = FALSE;
    }
}


// Monitors the history of the time accuracy estimate
//   - Tracks the time accuracy estimate and updates shm_tps with min,max,and avg values
//     for the time accuracy estimate, for the past seconds, minutes, and hours
//   - Called by receivedUBX_NAV_PVT()
void track_time_accuracy(uint32_t time_accuracy_est_nsec)
{
    static uint32_t previous_mins_uptime = 999;    // NOTE: 999 so loop's "if" test detects current hour as new hour on first time through
    static uint32_t previous_hours_uptime = 999;   // Same, for hours
    static uint32_t previous_days_uptime = 999;    // Same, for days
    static uint32_t previous_bad_timeacc_uptime = 0;

    uint32_t new_data_index;

    // First rule of Track Club: Don't track -1
    //   (It's the value u-blox gives for unknown/invalid time accuracy)
    if (time_accuracy_est_nsec == INVALID_TIME_ACCURACY_VALUE)
        return;

    // Get current uptime sec, min & hour & day
    uint32_t secs_uptime = get_cur_uptime();
    uint32_t mins_uptime = secs_uptime / 60;
    uint32_t hours_uptime = mins_uptime / 60;
    uint32_t days_uptime = hours_uptime / 24;

    /*** OVERALL MIN & MAX TRACKING ***/
    if (shm_tps_ptr->time_accuracy_min_since_first_fix > time_accuracy_est_nsec)
        shm_tps_ptr->time_accuracy_min_since_first_fix = time_accuracy_est_nsec;
    if (shm_tps_ptr->time_accuracy_max_since_first_fix < time_accuracy_est_nsec)
        shm_tps_ptr->time_accuracy_max_since_first_fix = time_accuracy_est_nsec;

    /*** EXCEEDING TIMEACC THRESHOLD ***/
    if (cfg_time_accuracy_threshold && (time_accuracy_est_nsec > cfg_time_accuracy_threshold)) {
        ++shm_tps_ptr->cnt_timeacc_exceeded_threshold;
        // Count seconds with an outside-threshold timeacc
        if (previous_bad_timeacc_uptime != secs_uptime) {
            ++shm_tps_ptr->cnt_seconds_where_timeacc_exceeded_threshold;
            previous_bad_timeacc_uptime = secs_uptime;
        }
    }

    /*** DAY-BY-DAY TRACKING ***/
    // If we are starting a new day, store summary data of previous day, restart data for current day
    if (days_uptime != previous_days_uptime) {
        // Store data from previous day
        if (shm_tps_ptr->current_day_num_samples > 0) {
            new_data_index = (shm_tps_ptr->index_for_latest_prev_day_data + 1) % NUM_DAYS_TRACKED_TIME_ACCURACY;
            shm_tps_ptr->time_accuracy_min_in_prev_day[new_data_index] = shm_tps_ptr->current_day_lowest_time_accuracy;
            shm_tps_ptr->time_accuracy_max_in_prev_day[new_data_index] = shm_tps_ptr->current_day_highest_time_accuracy;
            shm_tps_ptr->time_accuracy_avg_in_prev_day[new_data_index] = shm_tps_ptr->current_day_total_of_timeacc_samples / shm_tps_ptr->current_day_num_samples;
            // We are storing which uptime day the [new_data_index] set of data
            // is referring to.  But, we are adding one (1) to this value, as
            // we need the first day (0) to be non-zero, and the +1 actually makes
            // the day numbers human readable too.  Win, win!  NOTE: The prev_days+1 value
            // is the days_uptime value, so we use this to save the prev_day value.
            shm_tps_ptr->uptime_days_for_prev_day[new_data_index] = days_uptime;
            // Finally update the "latest index" so it points at our newly saved row
            shm_tps_ptr->index_for_latest_prev_day_data = new_data_index;
        }
        // Start current day
        shm_tps_ptr->current_day_lowest_time_accuracy = gpsNAVPVT.time_accuracy_est_nsec;
        shm_tps_ptr->current_day_highest_time_accuracy = gpsNAVPVT.time_accuracy_est_nsec;
        shm_tps_ptr->current_day_total_of_timeacc_samples = gpsNAVPVT.time_accuracy_est_nsec;
        shm_tps_ptr->current_day_num_samples = 1;
        previous_days_uptime = days_uptime;
    // If same day, add new data to current day trackers
    } else {
        shm_tps_ptr->current_day_total_of_timeacc_samples += gpsNAVPVT.time_accuracy_est_nsec;
        shm_tps_ptr->current_day_num_samples += 1;
        if (shm_tps_ptr->current_day_lowest_time_accuracy > gpsNAVPVT.time_accuracy_est_nsec) {
            shm_tps_ptr->current_day_lowest_time_accuracy = gpsNAVPVT.time_accuracy_est_nsec;
        }
        if (shm_tps_ptr->current_day_highest_time_accuracy < gpsNAVPVT.time_accuracy_est_nsec) {
            shm_tps_ptr->current_day_highest_time_accuracy = gpsNAVPVT.time_accuracy_est_nsec;
        }
    }


    /*** TODO: Make days, hours, and mins into a loop and ONE data structure.  For now, mins,hours a copy-paste of days code .. gasp ****/

    /*** HOUR-BY-HOUR TRACKING ***/
    // If we are starting a new hour, store summary data of previous hour, restart data for current hour
    if (hours_uptime != previous_hours_uptime) {
        // Store data from previous hour
        if (shm_tps_ptr->current_hour_num_samples > 0) {
            new_data_index = (shm_tps_ptr->index_for_latest_prev_hour_data + 1) % NUM_DAYS_TRACKED_TIME_ACCURACY;
            shm_tps_ptr->time_accuracy_min_in_prev_hour[new_data_index] = shm_tps_ptr->current_hour_lowest_time_accuracy;
            shm_tps_ptr->time_accuracy_max_in_prev_hour[new_data_index] = shm_tps_ptr->current_hour_highest_time_accuracy;
            shm_tps_ptr->time_accuracy_avg_in_prev_hour[new_data_index] = shm_tps_ptr->current_hour_total_of_timeacc_samples / shm_tps_ptr->current_hour_num_samples;
            // Store which uptime hour the [new_data_index] set of data is referring to.
            shm_tps_ptr->uptime_hours_for_prev_hour[new_data_index] = hours_uptime;
            // Finally update the "latest index" so it points at our newly saved row
            shm_tps_ptr->index_for_latest_prev_hour_data = new_data_index;
        }
        // Start current hour
        shm_tps_ptr->current_hour_lowest_time_accuracy = gpsNAVPVT.time_accuracy_est_nsec;
        shm_tps_ptr->current_hour_highest_time_accuracy = gpsNAVPVT.time_accuracy_est_nsec;
        shm_tps_ptr->current_hour_total_of_timeacc_samples = gpsNAVPVT.time_accuracy_est_nsec;
        shm_tps_ptr->current_hour_num_samples = 1;
        previous_hours_uptime = hours_uptime;
    // If same hour, add new data to current hour trackers
    } else {
        shm_tps_ptr->current_hour_total_of_timeacc_samples += gpsNAVPVT.time_accuracy_est_nsec;
        shm_tps_ptr->current_hour_num_samples += 1;
        if (shm_tps_ptr->current_hour_lowest_time_accuracy > gpsNAVPVT.time_accuracy_est_nsec) {
            shm_tps_ptr->current_hour_lowest_time_accuracy = gpsNAVPVT.time_accuracy_est_nsec;
        }
        if (shm_tps_ptr->current_hour_highest_time_accuracy < gpsNAVPVT.time_accuracy_est_nsec) {
            shm_tps_ptr->current_hour_highest_time_accuracy = gpsNAVPVT.time_accuracy_est_nsec;
        }
    }

    /*** MINUTE-BY-MINUTE TRACKING ***/
    // If we are starting a new minute, store summary data of previous minute, restart data for current minute
    if (mins_uptime != previous_mins_uptime) {
        // Store data from previous minute
        if (shm_tps_ptr->current_minute_num_samples > 0) {
            new_data_index = (shm_tps_ptr->index_for_latest_prev_minute_data + 1) % NUM_MINS_TRACKED_TIME_ACCURACY;
            shm_tps_ptr->time_accuracy_min_in_prev_minute[new_data_index] = shm_tps_ptr->current_minute_lowest_time_accuracy;
            shm_tps_ptr->time_accuracy_max_in_prev_minute[new_data_index] = shm_tps_ptr->current_minute_highest_time_accuracy;
            shm_tps_ptr->time_accuracy_avg_in_prev_minute[new_data_index] = shm_tps_ptr->current_minute_total_of_timeacc_samples / shm_tps_ptr->current_minute_num_samples;
            // Store which uptime minute the [new_data_index] set of data is referring to.
            shm_tps_ptr->uptime_minutes_for_prev_minute[new_data_index] = mins_uptime;
            // Finally update the "latest index" so it points at our newly saved row
            shm_tps_ptr->index_for_latest_prev_minute_data = new_data_index;
        }
        // Start current minute
        shm_tps_ptr->current_minute_lowest_time_accuracy = gpsNAVPVT.time_accuracy_est_nsec;
        shm_tps_ptr->current_minute_highest_time_accuracy = gpsNAVPVT.time_accuracy_est_nsec;
        shm_tps_ptr->current_minute_total_of_timeacc_samples = gpsNAVPVT.time_accuracy_est_nsec;
        shm_tps_ptr->current_minute_num_samples = 1;
        previous_mins_uptime = mins_uptime;
    // If same minute, add new data to current minute trackers
    } else {
        shm_tps_ptr->current_minute_total_of_timeacc_samples += gpsNAVPVT.time_accuracy_est_nsec;
        shm_tps_ptr->current_minute_num_samples += 1;
        if (shm_tps_ptr->current_minute_lowest_time_accuracy > gpsNAVPVT.time_accuracy_est_nsec) {
            shm_tps_ptr->current_minute_lowest_time_accuracy = gpsNAVPVT.time_accuracy_est_nsec;
        }
        if (shm_tps_ptr->current_minute_highest_time_accuracy < gpsNAVPVT.time_accuracy_est_nsec) {
            shm_tps_ptr->current_minute_highest_time_accuracy = gpsNAVPVT.time_accuracy_est_nsec;
        }
    }
}

/*-----------------------------------------------------------------------------
** @brief  Process received Backup Creation ACK message (UBX-UPD-SOS).
** @param  payload_len: Length of message payload
** @return unused
**---------------------------------------------------------------------------*/
void receivedAssistNowBackupAck(uint16_t payload_len)
{
    uint8_t i;
    char_t packet_data[255], tmp[10];
   
    packet_data[0]=0;
    for (i=0; i<payload_len+sizeof(ubxHeader_t); i++) 
    {
        sprintf(tmp, "%02x ", gpsCommandBuffer[i]);
        strcat(packet_data, tmp);
    }
#if defined(EXTRA_DEBUG)
    uint8_t response_byte_offset  
    // Determine if the response is ACK or NACK
    response_byte_offset=sizeof(ubxHeader_t)+4;
    if (gpsCommandBuffer[response_byte_offset] == 0x00) 
    {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"AssistNow Backup Creation Error \n");
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"AssistNow Backup Creation ACK message: %s \n", packet_data);
    }
    else if (gpsCommandBuffer[response_byte_offset] == 0x01) 
    {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"AssistNow successfully backed up\n");
    }
    else // this shouldnt happen
    {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"AssistNow Backup Creation message contains unexpected data %s \n", packet_data);
    }
#endif
}

/*-----------------------------------------------------------------------------
** @brief  Process received AssistNowAutonomous status message (UBX-NAV-AOPSTATUS).
** @param  payload_len: Length of message payload
** @return unused
**---------------------------------------------------------------------------*/
void receivedUBX_NAV_AOPSTATUS(uint16_t payload_len)
{
    uint8_t i, response_byte_offset;
    char_t packet_data[255], tmp[10];
   
    packet_data[0]=0;
    for (i=0; i<payload_len+sizeof(ubxHeader_t); i++) 
    {
        sprintf(tmp, "%02x ", gpsCommandBuffer[i]);
        strcat(packet_data, tmp);
    }
    
    // Determine if the subsystem is idle or not
    response_byte_offset=sizeof(ubxHeader_t)+5;
    if (gpsCommandBuffer[response_byte_offset] == 0x00) 
    {
#if defined(EXTRA_DEBUG)
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"AssistNow Subsystem is idle \n");
#endif
        assistNowConfirmedIdle = TRUE;
    }
    else 
    {
#if defined(EXTRA_DEBUG)
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"AssistNow Subsystem is busy \n");
#endif
        assistNowConfirmedIdle = FALSE;
    }
}

/*-----------------------------------------------------------------------------
** @brief  Process received UBX-NAV-STATUS message.
** @param  unused
** @return unused
**---------------------------------------------------------------------------*/
void receivedUBX_NAV_STATUS()
{
    memset(&gpsNAVSTATUS, 0, sizeof(gpsNAVSTATUS));

    gpsNAVSTATUS.gps_time_of_week_msec = U4Convert(&gpsCommandBuffer[0 + UBXHDRLEN]);
    gpsNAVSTATUS.gps_fix_type = gpsCommandBuffer[4 + UBXHDRLEN];
    gpsNAVSTATUS.gps_fix_ok                          = (gpsCommandBuffer[5 + UBXHDRLEN] & 0x01) != 0;    // gpsFixOk
    gpsNAVSTATUS.differential_corrections_applied    = (gpsCommandBuffer[5 + UBXHDRLEN] & 0x02) != 0;    // diffSoln
    gpsNAVSTATUS.week_number_valid                   = (gpsCommandBuffer[5 + UBXHDRLEN] & 0x04) != 0;    // wknSet
    gpsNAVSTATUS.time_of_week_valid                  = (gpsCommandBuffer[5 + UBXHDRLEN] & 0x08) != 0;    // towSet
    gpsNAVSTATUS.differential_corrections_available  = (gpsCommandBuffer[6 + UBXHDRLEN] & 0x01) != 0;    // diffCorr
    gpsNAVSTATUS.carrier_solution_valid              = (gpsCommandBuffer[6 + UBXHDRLEN] & 0x02) != 0;    // carrSolnValid
    gpsNAVSTATUS.map_matching_status                 = (gpsCommandBuffer[6 + UBXHDRLEN] >> 6) & 0x03;    // mapMatching
    gpsNAVSTATUS.power_save_mode_state               = (gpsCommandBuffer[7 + UBXHDRLEN]     ) & 0x03;    // psmState
    gpsNAVSTATUS.spoofing_detection_state            = (gpsCommandBuffer[7 + UBXHDRLEN] >> 3) & 0x03;    // spoofDetState
    gpsNAVSTATUS.carrier_phase_range_solution_status = (gpsCommandBuffer[7 + UBXHDRLEN] >> 6) & 0x03;    // carrSoln
    gpsNAVSTATUS.time_to_first_fix_msec = U4Convert(&gpsCommandBuffer[8 + UBXHDRLEN]); // Time to first fix (millisecond time tag)
    gpsNAVSTATUS.time_since_startup_msec = U4Convert(&gpsCommandBuffer[12 + UBXHDRLEN]);  // Milliseconds since Startup / Reset

    // Copy the data into tps shm
    memcpy(&shm_tps_ptr->nav_status, &gpsNAVSTATUS, sizeof(ubxNavStatus_t));
}

/*-----------------------------------------------------------------------------
** @brief  Process received UBX-NAV-SAT message.
** @param  unused
** @return unused
**---------------------------------------------------------------------------*/
void receivedUBX_NAV_SAT()
{
    ubxNavSat_t gpsNAVSAT;
    ubxNavSatDetail_t satDetail;
    uint8_t gnssId, cno, qualityInd, health;
    int n;
    oneNavSatDetail_t * cur_sat_detail;

    memset(&gpsNAVSAT, 0, sizeof(gpsNAVSAT));
    memset(&satDetail, 0, sizeof(satDetail));

    gpsNAVSAT.gps_time_of_week_msec = U4Convert(&gpsCommandBuffer[0 + UBXHDRLEN]);
    if (gpsCommandBuffer[4 + UBXHDRLEN] != 1) {
        fprintf(stderr, "WARNING: TPS excpecting version 1 of NAV-SAT but we are getting ver %d !!\n",
            gpsCommandBuffer[4 + UBXHDRLEN]);
    }
    gpsNAVSAT.number_of_satellites = gpsCommandBuffer[5 + UBXHDRLEN];

    for (n=0; n<gpsNAVSAT.number_of_satellites; n++) {
        gnssId     = gpsCommandBuffer[12*n +  8 + UBXHDRLEN];
        cno        = gpsCommandBuffer[12*n + 10 + UBXHDRLEN];
        qualityInd = gpsCommandBuffer[12*n + 16 + UBXHDRLEN] & 0x07;
        health     = (gpsCommandBuffer[12*n + 16 + UBXHDRLEN] >> 4) & 0x03;
        // Build up data for a histogram
        ++gpsNAVSAT.cnt_sats_w_gnss_XX[gnssId];
        ++gpsNAVSAT.cnt_sats_w_carrier_to_noise_ratio_XX_dBHz[cno / 10];
        ++gpsNAVSAT.cnt_sats_w_quality_ind_XX[qualityInd];
        ++gpsNAVSAT.cnt_sats_w_health_XX[health];
        ++gpsNAVSAT.cnt_sats_w_quality_ind_XX_x_gnss_XX[qualityInd][gnssId];
        ++gpsNAVSAT.cnt_sats_w_health_XX_x_gnss_XX[health][gnssId];
        // If GPS or GLONASS, add sat to detail data
        if (gnssId == 0 || gnssId == 6) {
            if (gnssId == 0) {
                cur_sat_detail = &satDetail.gps[satDetail.cnt_gps_sats++];
            } else {
                cur_sat_detail = &satDetail.glo[satDetail.cnt_glo_sats++];
            }
            cur_sat_detail->svId = gpsCommandBuffer[12*n +  9 + UBXHDRLEN];
            cur_sat_detail->CNR = cno;
            cur_sat_detail->quality = qualityInd;
            cur_sat_detail->health = health;
            cur_sat_detail->svUsed = (gpsCommandBuffer[12*n + 16 + UBXHDRLEN] >> 3) & 0x01;
            cur_sat_detail->elev = (int8_t) (gpsCommandBuffer[12*n + 11 + UBXHDRLEN]);
            cur_sat_detail->azim = (int16_t) U2Convert(&gpsCommandBuffer[12*n + 12 + UBXHDRLEN]);
            cur_sat_detail->prRes = (int16_t) U2Convert(&gpsCommandBuffer[12*n + 14 + UBXHDRLEN]);
        }
    }

    // Copy the data into tps shm
    memcpy(&shm_tps_ptr->nav_sat, &gpsNAVSAT, sizeof(ubxNavSat_t));
    memcpy(&shm_tps_ptr->sat_detail, &satDetail, sizeof(ubxNavSatDetail_t));
}


/*-----------------------------------------------------------------------------
** @brief  Process received UBX-MON-HW message.
** @param  unused
** @return unused
**---------------------------------------------------------------------------*/
void receivedUBX_MON_HW()
{
    shm_tps_ptr->mon_hw_noise_per_ms = U2Convert(&gpsCommandBuffer[16 + UBXHDRLEN]);
    shm_tps_ptr->mon_hw_agc_count = U2Convert(&gpsCommandBuffer[18 + UBXHDRLEN]);
    shm_tps_ptr->mon_hw_jamming_ind = gpsCommandBuffer[45 + UBXHDRLEN];
    shm_tps_ptr->mon_hw_rtc_is_calibrated = gpsCommandBuffer[22 + UBXHDRLEN] & 0x01;
    shm_tps_ptr->mon_hw_jamming_status = (gpsCommandBuffer[22 + UBXHDRLEN] >> 2) & 0x03;
}



/*-----------------------------------------------------------------------------
** @brief  Process received UBX NAV-PVT message. (Navigation PVT solution) (required)
** @param  unused
** @return unused
**---------------------------------------------------------------------------*/
void receivedUBX_NAV_PVT()
{
    static bool_t received_first_good_posn_fix = FALSE;
    static bool_t received_first_good_time_accuracy = FALSE;
    static uint8_t first_good_posn_fix_count = 0;
    static uint8_t first_good_time_acc_count = 0;
    static long first_good_posn_fix_started_at_uptime = 0;
    static long first_good_timeacc_started_at_uptime = 0;
    uint8_t previous_gnss_fix_ok = 0;
    uint8_t previous_timeacc_ok = 0;
    uint8_t gnss_fix_ok = 0;
    uint8_t time_accuracy_ok = 0;

#if defined(HEAVY_DUTY)
    memset(&gpsNAVPVT, 0, sizeof(gpsNAVPVT));/* do not allow stale fix to linger */
#endif

    gnss_fix_ok = gpsCommandBuffer[21 + UBXHDRLEN] & 0x01;
    gpsNAVPVT.time_accuracy_est_nsec = U4Convert(&gpsCommandBuffer[12 + UBXHDRLEN]);
    shm_tps_ptr->current_time_accuracy = gpsNAVPVT.time_accuracy_est_nsec;
    time_accuracy_ok = (gpsNAVPVT.time_accuracy_est_nsec != INVALID_TIME_ACCURACY_VALUE);
    if(time_accuracy_ok) {
        if(shm_tps_ptr->current_time_accuracy < shm_tps_ptr->time_accuracy_min) {
            shm_tps_ptr->time_accuracy_min = shm_tps_ptr->current_time_accuracy;
        }
        if(shm_tps_ptr->time_accuracy_max < shm_tps_ptr->current_time_accuracy) {
            shm_tps_ptr->time_accuracy_max = shm_tps_ptr->current_time_accuracy;
        }
        if(shm_tps_ptr->time_accuracy_threshold < shm_tps_ptr->current_time_accuracy) {
#if defined(EXTRA_DEBUG)
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"time_accuracy_threshold(%u) < current_time_accuracy(%u)\n",
            shm_tps_ptr->time_accuracy_threshold,shm_tps_ptr->current_time_accuracy);
#endif
            shm_tps_ptr->error_states |= GPS_TIME_ACCURACY_ERROR;
        } else {
            shm_tps_ptr->error_states &= ~GPS_TIME_ACCURACY_ERROR;
        }
    } else {
        shm_tps_ptr->error_states |= GPS_TIME_ACCURACY_ERROR;
    }
    shm_tps_ptr->uptime_at_nav_pvt = get_cur_uptime();

/* 
 * Fix or not PVT is coming out and incrementing timer.
 * All the data, all the time, please.
 */
#if defined(HEAVY_DUTY)
    memset(&gpsNAVPVT, 0, sizeof(gpsNAVPVT));

    gpsNAVPVT.time_of_week_msec      = U4Convert(&gpsCommandBuffer[0 + UBXHDRLEN]);
    gpsNAVPVT.year_utc               = U2Convert(&gpsCommandBuffer[4 + UBXHDRLEN]);
    gpsNAVPVT.month_utc              = gpsCommandBuffer[6 + UBXHDRLEN];
    gpsNAVPVT.day_utc                = gpsCommandBuffer[7 + UBXHDRLEN];
    gpsNAVPVT.hour_utc               = gpsCommandBuffer[8 + UBXHDRLEN];
    gpsNAVPVT.min_utc                = gpsCommandBuffer[9 + UBXHDRLEN];
    gpsNAVPVT.sec_utc                = gpsCommandBuffer[10 + UBXHDRLEN];
    gpsNAVPVT.nsec_utc               = I4Convert(&gpsCommandBuffer[16 + UBXHDRLEN]);

    gpsNAVPVT.fix_type                 = gpsCommandBuffer[20 + UBXHDRLEN];
    gpsNAVPVT.flag_gnss_fix_ok         = gpsCommandBuffer[21 + UBXHDRLEN] & 0x01;
    gpsNAVPVT.flag_differential_corrections_applied = gpsCommandBuffer[21 + UBXHDRLEN] & 0x02;
    gpsNAVPVT.flag_heading_valid       = gpsCommandBuffer[21 + UBXHDRLEN] & 0x20;
    gpsNAVPVT.flag_utc_fully_resolved  = gpsCommandBuffer[11 + UBXHDRLEN] & 0x04;
    gpsNAVPVT.num_sats                 = gpsCommandBuffer[23 + UBXHDRLEN];
    gpsNAVPVT.lon_deg_e7               = I4Convert(&gpsCommandBuffer[24 + UBXHDRLEN]);
    gpsNAVPVT.lat_deg_e7               = I4Convert(&gpsCommandBuffer[28 + UBXHDRLEN]);
    gpsNAVPVT.height_above_elipsoid_mm = U4Convert(&gpsCommandBuffer[32 + UBXHDRLEN]);
    gpsNAVPVT.height_above_MSL_mm      =  U4Convert(&gpsCommandBuffer[36 + UBXHDRLEN]);
    gpsNAVPVT.horiz_accuracy_est_mm    = U4Convert(&gpsCommandBuffer[40 + UBXHDRLEN]);
    gpsNAVPVT.vert_accuracy_est_mm     = U4Convert(&gpsCommandBuffer[44 + UBXHDRLEN]);
    gpsNAVPVT.velocity_north_mmsec     = I4Convert(&gpsCommandBuffer[48 + UBXHDRLEN]);
    gpsNAVPVT.velocity_east_mmsec      = I4Convert(&gpsCommandBuffer[52 + UBXHDRLEN]);
    gpsNAVPVT.velocity_down_mmsec      = I4Convert(&gpsCommandBuffer[56 + UBXHDRLEN]);
    gpsNAVPVT.ground_speed_mmsec       = I4Convert(&gpsCommandBuffer[60 + UBXHDRLEN]);
    gpsNAVPVT.speed_accuracy_est_mmsec = U4Convert(&gpsCommandBuffer[68 + UBXHDRLEN]);
    gpsNAVPVT.position_dilut_of_precision = U2Convert(&gpsCommandBuffer[76 + UBXHDRLEN]);
    if (gpsNAVPVT.flag_heading_valid) {
        gpsNAVPVT.heading_of_motion_deg_e5    = I4Convert(&gpsCommandBuffer[64 + UBXHDRLEN]);
        gpsNAVPVT.heading_of_vehicle_deg_e5   = I4Convert(&gpsCommandBuffer[84 + UBXHDRLEN]);
        gpsNAVPVT.heading_accuracy_est_deg_e5 = U4Convert(&gpsCommandBuffer[72 + UBXHDRLEN]);
    }

    // Copy PVT into SHM.
    if (shm_tps_ptr->pub_geodetic.last_updated_pvt_index == 0) {
        memcpy(&shm_tps_ptr->pub_geodetic.last_PVT[1], &gpsNAVPVT, sizeof(gpsNAVPVT));
        shm_tps_ptr->pub_geodetic.last_updated_pvt_index = 1;
    } else {
        memcpy(&shm_tps_ptr->pub_geodetic.last_PVT[0], &gpsNAVPVT, sizeof(gpsNAVPVT));
        shm_tps_ptr->pub_geodetic.last_updated_pvt_index = 0;
    }
    memset(&gpsNAVPVT, 0, sizeof(gpsNAVPVT)); /* Clear for below. Could be no harm in letting it fall through as is. */
#endif

    if (gnss_fix_ok)
    {
        memset(&gpsNAVPVT, 0, sizeof(gpsNAVPVT));

        gpsNAVPVT.debug_recv_cpu_cycles = ClockCycles();
        gpsNAVPVT.debug_recv_time_ms    = get_cur_msec_time();

        gpsNAVPVT.time_of_week_msec      = U4Convert(&gpsCommandBuffer[0 + UBXHDRLEN]);
        gpsNAVPVT.year_utc               = U2Convert(&gpsCommandBuffer[4 + UBXHDRLEN]);
        gpsNAVPVT.month_utc              = gpsCommandBuffer[6 + UBXHDRLEN];
        gpsNAVPVT.day_utc                = gpsCommandBuffer[7 + UBXHDRLEN];
        gpsNAVPVT.hour_utc               = gpsCommandBuffer[8 + UBXHDRLEN];
        gpsNAVPVT.min_utc                = gpsCommandBuffer[9 + UBXHDRLEN];
        gpsNAVPVT.sec_utc                = gpsCommandBuffer[10 + UBXHDRLEN];
        gpsNAVPVT.nsec_utc               = I4Convert(&gpsCommandBuffer[16 + UBXHDRLEN]);

        gpsNAVPVT.fix_type                 = gpsCommandBuffer[20 + UBXHDRLEN];
        gpsNAVPVT.flag_gnss_fix_ok         = gpsCommandBuffer[21 + UBXHDRLEN] & 0x01;
        gpsNAVPVT.flag_differential_corrections_applied = gpsCommandBuffer[21 + UBXHDRLEN] & 0x02;
        gpsNAVPVT.flag_heading_valid       = gpsCommandBuffer[21 + UBXHDRLEN] & 0x20;
        gpsNAVPVT.flag_utc_fully_resolved  = gpsCommandBuffer[11 + UBXHDRLEN] & 0x04;
        gpsNAVPVT.num_sats                 = gpsCommandBuffer[23 + UBXHDRLEN];
        gpsNAVPVT.lon_deg_e7               = I4Convert(&gpsCommandBuffer[24 + UBXHDRLEN]);
        gpsNAVPVT.lat_deg_e7               = I4Convert(&gpsCommandBuffer[28 + UBXHDRLEN]);
        gpsNAVPVT.height_above_elipsoid_mm = U4Convert(&gpsCommandBuffer[32 + UBXHDRLEN]);
        gpsNAVPVT.height_above_MSL_mm      =  U4Convert(&gpsCommandBuffer[36 + UBXHDRLEN]);
        gpsNAVPVT.horiz_accuracy_est_mm    = U4Convert(&gpsCommandBuffer[40 + UBXHDRLEN]);
        gpsNAVPVT.vert_accuracy_est_mm     = U4Convert(&gpsCommandBuffer[44 + UBXHDRLEN]);
        gpsNAVPVT.velocity_north_mmsec     = I4Convert(&gpsCommandBuffer[48 + UBXHDRLEN]);
        gpsNAVPVT.velocity_east_mmsec      = I4Convert(&gpsCommandBuffer[52 + UBXHDRLEN]);
        gpsNAVPVT.velocity_down_mmsec      = I4Convert(&gpsCommandBuffer[56 + UBXHDRLEN]);
        gpsNAVPVT.ground_speed_mmsec       = I4Convert(&gpsCommandBuffer[60 + UBXHDRLEN]);
        gpsNAVPVT.speed_accuracy_est_mmsec = U4Convert(&gpsCommandBuffer[68 + UBXHDRLEN]);
        gpsNAVPVT.position_dilut_of_precision = U2Convert(&gpsCommandBuffer[76 + UBXHDRLEN]);
        if (gpsNAVPVT.flag_heading_valid)
        {
            gpsNAVPVT.heading_of_motion_deg_e5    = I4Convert(&gpsCommandBuffer[64 + UBXHDRLEN]);
            gpsNAVPVT.heading_of_vehicle_deg_e5   = I4Convert(&gpsCommandBuffer[84 + UBXHDRLEN]);
            gpsNAVPVT.heading_accuracy_est_deg_e5 = U4Convert(&gpsCommandBuffer[72 + UBXHDRLEN]);
        }
        gpsReceivedNAVPVT = TRUE;
        // Copy the # of sats into SHM for user - Note This is # sats used in the fix solution, only gets populated when there's a fix
        shm_tps_ptr->debug_latest_number_of_sats = gpsNAVPVT.num_sats;

        // If this is our first good position fix, record the time needed to get this first fix
        if (received_first_good_posn_fix == FALSE) {
            // Record uptime when we got first one
            if (first_good_posn_fix_count == 0) {
                first_good_posn_fix_started_at_uptime = get_cur_uptime();
            }
            // Did we receive enough position fixes that we have a good fix for sure?
            if (++first_good_posn_fix_count >= MIN_SEQUENTIAL_GOOD_FIXES_TO_COUNT_AS_GOOD_FIX_FOUND) {
                received_first_good_posn_fix = TRUE;
                shm_tps_ptr->debug_power_up_to_first_fix_acquired_secs = first_good_posn_fix_started_at_uptime;
            }
        // If we previously had lost the fix, mark end of fix_lost time
        } else if (!previous_gnss_fix_ok) {
            // Record how long fix was lost, when loss started and ended
            if (++shm_tps_ptr->latest_lost_fix_index > NUM_TRACKED_LOST_FIXES) {
                shm_tps_ptr->latest_lost_fix_index = 0;
            }
            shm_tps_ptr->tracked_lost_fix_period_length[shm_tps_ptr->latest_lost_fix_index] =
                shm_tps_ptr->current_lost_fix_period_length;
            shm_tps_ptr->tracked_lost_fix_start_uptime[shm_tps_ptr->latest_lost_fix_index] =
                shm_tps_ptr->current_fix_loss_started_at_uptime;
            shm_tps_ptr->tracked_lost_fix_end_uptime[shm_tps_ptr->latest_lost_fix_index] = get_cur_uptime();
            shm_tps_ptr->tracked_lost_fix_is_loss_number[shm_tps_ptr->latest_lost_fix_index] =
                shm_tps_ptr->debug_cnt_gps_fix_lost;
            shm_tps_ptr->current_lost_fix_period_length = 0;
        }
        // Set our positition fix state
        shm_tps_ptr->cur_tps_posn_state = TGP_GPS_POSITION;
    }
    else    // No position fix
    {
#if defined(EXTRA_DEBUG)
        // TODO: REMOVE: TEMP: KENN: Some extra debug on weird time accuracy
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "no fix at %02u:%02u.%09u, PVT-NAV has time %02u:%02u.%09d time_acc %u (0x%x)\n",
                gpsNAVPVT.min_utc, gpsNAVPVT.sec_utc, gpsNAVPVT.nsec_utc,
                gpsCommandBuffer[9 + UBXHDRLEN],    // gpsNAVPVT.min_utc                = gpsCommandBuffer[9 + UBXHDRLEN];
                gpsCommandBuffer[10 + UBXHDRLEN],   // gpsNAVPVT.sec_utc                = gpsCommandBuffer[10 + UBXHDRLEN];
                I4Convert(&gpsCommandBuffer[16 + UBXHDRLEN]), // gpsNAVPVT.nsec_utc               = I4Convert(&gpsCommandBuffer[16 + UBXHDRLEN]);
                gpsNAVPVT.time_accuracy_est_nsec,
                gpsNAVPVT.time_accuracy_est_nsec);
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "  flags: validDate %d validTime %d fullyResolved %d"
                               " confirmIsAvail %d confirmedDate %d confirmedTime %d\n",
                gpsCommandBuffer[11 + UBXHDRLEN] & 0x01,
                gpsCommandBuffer[11 + UBXHDRLEN] & 0x02 ? 1 : 0,
                gpsCommandBuffer[11 + UBXHDRLEN] & 0x04 ? 1 : 0,
                gpsCommandBuffer[22 + UBXHDRLEN] & 0x80 ? 1 : 0,
                gpsCommandBuffer[22 + UBXHDRLEN] & 0x40 ? 1 : 0,
                gpsCommandBuffer[22 + UBXHDRLEN] & 0x20 ? 1 : 0);
        // END: TODO: REMOVE: TEMP: KENN: Some extra debug on weird time accuracy -- is something stomping the buffer?
#endif
        gpsReceivedNAVPVT = FALSE;
        ++shm_tps_ptr->debug_gps_cmd_count_ubx_nav_pvt_no_fix;
        // Track gnss fix losses, but only after we've had our first good fix
        if (received_first_good_posn_fix == TRUE) {
            // Note start of a new fix-lost period, and count fix losses
            if (previous_gnss_fix_ok) {
                ++shm_tps_ptr->debug_cnt_gps_fix_lost;
                shm_tps_ptr->current_lost_fix_period_length = 1;
                shm_tps_ptr->current_fix_loss_started_at_uptime = get_cur_uptime();
            }
            // Count length of lost fix
            else {
                ++shm_tps_ptr->current_lost_fix_period_length;
            }
        } else {
            first_good_posn_fix_count = 0;  // Reset to zero, good fixes must be sequential to count
        }
        // Set our positition fix state
        shm_tps_ptr->cur_tps_posn_state = TGP_NO_POSITION;
    }

    // Track the time accuracy separately from the position fix
    if (time_accuracy_ok) {
        // Note when we first recieve the time accuracies - first ever, and when first within acceptable threshold
        if (received_first_good_time_accuracy == FALSE) {
            // Note time if this is the first we've received
            if (shm_tps_ptr->debug_power_up_to_first_timeacc_acquired_secs == 0) {
                shm_tps_ptr->debug_power_up_to_first_timeacc_acquired_secs = get_cur_uptime();
            }
            // Is time accuracy acceptable?  Must be acceptable to count towards a good time fix
            if (cfg_time_accuracy_threshold && (gpsNAVPVT.time_accuracy_est_nsec > cfg_time_accuracy_threshold)) {
                // And make the acceptable time accuracy counter start over, in case we have
                // acceptables mixed in with out of bounds values.
                first_good_time_acc_count = 0;
            // It's acceptable!  Count it
            } else {
                // Note when we got the first one
                if (first_good_time_acc_count == 0) {
                    first_good_timeacc_started_at_uptime = get_cur_uptime();
                }
                // Did we receive enough in a row that we have a good fix for sure?
                if (++first_good_time_acc_count >= MIN_SEQUENTIAL_GOOD_FIXES_TO_COUNT_AS_GOOD_FIX_FOUND) {
                    received_first_good_time_accuracy = TRUE;
                    shm_tps_ptr->debug_power_up_to_first_acceptable_timeacc_secs = first_good_timeacc_started_at_uptime;
                    // Set the time accuracy overall max and min initial values here
                    shm_tps_ptr->time_accuracy_min_since_first_fix = gpsNAVPVT.time_accuracy_est_nsec;
                    shm_tps_ptr->time_accuracy_max_since_first_fix = gpsNAVPVT.time_accuracy_est_nsec;
                    // Zero the out-of-bounds counts, so they start from here, too
                    shm_tps_ptr->cnt_timeacc_exceeded_threshold = 0;
                    shm_tps_ptr->cnt_seconds_where_timeacc_exceeded_threshold = 0;
                }
            }
        // We're past getting the first time fix
        } else {
            // Record the time accuracy estimate, from when we have gotten our first timeacc fix, onwards
            track_time_accuracy(gpsNAVPVT.time_accuracy_est_nsec);
            // If we previously had lost the timeacc, mark end of timeacc_lost time
            if (!previous_timeacc_ok) {
                // Record how long timeacc was lost, when loss started and ended
                if (++shm_tps_ptr->latest_lost_timeacc_index > NUM_TRACKED_LOST_FIXES) {
                    shm_tps_ptr->latest_lost_timeacc_index = 0;
                }
                shm_tps_ptr->tracked_lost_timeacc_period_length[shm_tps_ptr->latest_lost_timeacc_index] =
                    shm_tps_ptr->current_lost_timeacc_period_length;
                shm_tps_ptr->tracked_lost_timeacc_start_uptime[shm_tps_ptr->latest_lost_timeacc_index] =
                    shm_tps_ptr->current_timeacc_loss_started_at_uptime;
                shm_tps_ptr->tracked_lost_timeacc_end_uptime[shm_tps_ptr->latest_lost_timeacc_index] = get_cur_uptime();
                shm_tps_ptr->tracked_lost_timeacc_is_loss_number[shm_tps_ptr->latest_lost_timeacc_index] =
                    shm_tps_ptr->debug_cnt_timeacc_lost;
            }
        }
    // Dont have a time accuracy
    } else {
        ++shm_tps_ptr->debug_gps_cmd_count_ubx_nav_pvt_no_timeacc;
        // Track timeacc losses, but only after we've had our first good timeacc value
        if (received_first_good_time_accuracy == TRUE) {
            // Note start of a new timeacc-lost period, and count timeacc losses
            if (previous_timeacc_ok) {
                ++shm_tps_ptr->debug_cnt_timeacc_lost;
                shm_tps_ptr->current_lost_timeacc_period_length = 1;
                shm_tps_ptr->current_timeacc_loss_started_at_uptime = get_cur_uptime();
            }
            // Count length of lost timeacc
            else {
                ++shm_tps_ptr->current_lost_timeacc_period_length;
            }
        // Reset our sequential counter for the first good time accuracy
        } else {
            first_good_time_acc_count = 0; // Reset to zero, good timeacc-es must be sequential to count
        }
    }

    // Compute SHA256 Hash for Host Interface Signature Verification
    if (nav_pvt_sha256_enable)
    {
        uint8_t data[NAV_PVT_UBX_MSG_LENGTH + UBX_SEC_SIGN_SEEDS];
#if !defined(MY_UNIT_TEST)
        SHA256_CTX ctx;
#endif
        // Memset data to 0x00
        memset(data, 0x00, NAV_PVT_UBX_MSG_LENGTH + UBX_SEC_SIGN_SEEDS);

        // Set High & Low Fixed Seeds
        memcpy(&data[0], FIXED_SEED_HIGH, 4);
        memcpy(&data[NAV_PVT_UBX_MSG_LENGTH + UBX_SEC_SIGN_SEEDS - 4], FIXED_SEED_LOW, 4);

        // Copy gpsCommandBuffer to data[8]
        memcpy(&data[8], gpsCommandBuffer, NAV_PVT_UBX_MSG_LENGTH);
#if !defined(MY_UNIT_TEST)
        // Generate SHA256 Hash
        SHA256_Init(&ctx);
        SHA256_Update(&ctx, data, NAV_PVT_UBX_MSG_LENGTH + UBX_SEC_SIGN_SEEDS);
        SHA256_Final(nav_pvt_sha256_hash, &ctx);
#endif
    }

    // Save our current fix_ok/timeacc_ok for next time around
    previous_gnss_fix_ok = gnss_fix_ok;
    previous_timeacc_ok = time_accuracy_ok;
}


/*-----------------------------------------------------------------------------
** @brief  Process received UBX ESF-RAW message.  (External Sensor Fusion, Raw data)
** @param  unused
** @return unused
**---------------------------------------------------------------------------*/
#if defined(ENABLE_WHEELTIX)
void receivedUBX_ESF_RAW()
{
    // Payload length is 4 + 8*N, 1N = 1 raw sensor measurement data
    uint16_t payload_len = U2Convert(&gpsCommandBuffer[4]);
    int32_t i;
    for (i=4; i<payload_len; i+=8) {
        int32_t dataField = I3Convert(&gpsCommandBuffer[i + 0 + UBXHDRLEN]);   // Bits 0-23 of a 32-bit sequence
        uint8_t dataType = gpsCommandBuffer[i + 3 + UBXHDRLEN] & 0x3F;  // Highest byte is last, want bits 0-5
        gpsESFRAW.most_recent_time_of_week_msec = U4Convert(&gpsCommandBuffer[i + 4 + UBXHDRLEN]);

        // Process our RAW data
        switch (dataType) {
            case 5:   // Z-Axis gyroscope angular rate - deg/s *2^-12 signed
                gpsESFRAW.z_ang_rate_deg_sec_2pow12 = dataField;
                gpsESFRAW.z_ang_rate_time_of_week_msec = gpsESFRAW.most_recent_time_of_week_msec;
                break;
            case 12:  // gyroscope temperature deg Celsius * 1e-2 signed
                gpsESFRAW.gyro_temp_deg_e2 = dataField;
                gpsESFRAW.gyro_temp_time_of_week_msec = gpsESFRAW.most_recent_time_of_week_msec;
                break;
            case 13:  // Y-axis gyroscope angular rate deg/s *2^-12 signed
                gpsESFRAW.y_ang_rate_deg_sec_2pow12 = dataField;
                gpsESFRAW.y_ang_rate_time_of_week_msec = gpsESFRAW.most_recent_time_of_week_msec;
                break;
            case 14:  // X-axis gyroscope angular rate deg/s *2^-12 signed
                gpsESFRAW.x_ang_rate_deg_sec_2pow12 = dataField;
                gpsESFRAW.x_ang_rate_time_of_week_msec = gpsESFRAW.most_recent_time_of_week_msec;
                break;
            case 16:  // X-axis accelerometer specific force m/s^2 *2^-10 signed
                gpsESFRAW.x_accel_mg = dataField;
                gpsESFRAW.x_accel_time_of_week_msec = gpsESFRAW.most_recent_time_of_week_msec;
                break;
            case 17:  // Y-axis accelerometer specific force m/s^2 *2^-10 signed
                gpsESFRAW.y_accel_mg = dataField;
                gpsESFRAW.y_accel_time_of_week_msec = gpsESFRAW.most_recent_time_of_week_msec;
                break;
            case 18:  // Z-axis accelerometer specific force m/s^2 *2^-10 signed
                gpsESFRAW.z_accel_mg = dataField;
                gpsESFRAW.z_accel_time_of_week_msec = gpsESFRAW.most_recent_time_of_week_msec;
                break;
            default:  // Unwanted sensor data
                break;
        }
    }
    gpsReceivedESFRAW = TRUE;

    // And fill up SHM for debugging for now
    shm_tps_ptr->debug_esf_x_ang_rate = ((float64_t) gpsESFRAW.x_ang_rate_deg_sec_2pow12) / 4096.0;
    shm_tps_ptr->debug_esf_y_ang_rate = ((float64_t) gpsESFRAW.y_ang_rate_deg_sec_2pow12) / 4096.0;
    shm_tps_ptr->debug_esf_z_ang_rate = ((float64_t) gpsESFRAW.z_ang_rate_deg_sec_2pow12) / 4096.0;
    shm_tps_ptr->debug_esf_x_accel = ((float64_t) gpsESFRAW.x_accel_mg) / 1024.0;
    shm_tps_ptr->debug_esf_y_accel = ((float64_t) gpsESFRAW.y_accel_mg) / 1024.0;
    shm_tps_ptr->debug_esf_z_accel = ((float64_t) gpsESFRAW.z_accel_mg) / 1024.0;
    shm_tps_ptr->debug_esf_temp = ((float64_t) gpsESFRAW.gyro_temp_deg_e2) / 100.0;
    shm_tps_ptr->debug_esf_iTOW = gpsESFRAW.most_recent_time_of_week_msec;

    shm_tps_ptr->debug_esf_x_ang_rate_hex = gpsESFRAW.x_ang_rate_deg_sec_2pow12;
    shm_tps_ptr->debug_esf_y_ang_rate_hex = gpsESFRAW.y_ang_rate_deg_sec_2pow12;
    shm_tps_ptr->debug_esf_z_ang_rate_hex = gpsESFRAW.z_ang_rate_deg_sec_2pow12;
    shm_tps_ptr->debug_esf_x_accel_hex    = gpsESFRAW.x_accel_mg;
    shm_tps_ptr->debug_esf_y_accel_hex    = gpsESFRAW.y_accel_mg;
    shm_tps_ptr->debug_esf_z_accel_hex    = gpsESFRAW.z_accel_mg;
    shm_tps_ptr->debug_esf_temp_hex       = gpsESFRAW.gyro_temp_deg_e2;
}


/*-----------------------------------------------------------------------------
** @brief  Process received UBX ESF-INS message. (External Sensor (Vehicle Dynamics)
** @param  unused
** @return unused
**---------------------------------------------------------------------------*/
void receivedUBX_ESF_INS()
{
    gpsESFINS.valid_bits            = U4Convert(&gpsCommandBuffer[0 + UBXHDRLEN]);
    gpsESFINS.time_of_week_msec     = U4Convert(&gpsCommandBuffer[8 + UBXHDRLEN]);
    gpsESFINS.x_ang_rate_deg_sec_e3 = I4Convert(&gpsCommandBuffer[12 + UBXHDRLEN]);
    gpsESFINS.y_ang_rate_deg_sec_e3 = I4Convert(&gpsCommandBuffer[16 + UBXHDRLEN]);
    gpsESFINS.z_ang_rate_deg_sec_e3 = I4Convert(&gpsCommandBuffer[20 + UBXHDRLEN]);
    gpsESFINS.x_accel_mg            = I4Convert(&gpsCommandBuffer[24 + UBXHDRLEN]);
    gpsESFINS.y_accel_mg            = I4Convert(&gpsCommandBuffer[28 + UBXHDRLEN]);
    gpsESFINS.z_accel_mg            = I4Convert(&gpsCommandBuffer[32 + UBXHDRLEN]);

    gpsESFINS.flag_ang_rates_valid = ((gpsESFINS.valid_bits & 0x700) == 0x700);   // Bits 8-10 (of 0-31)
    gpsESFINS.flag_accels_valid    = ((gpsESFINS.valid_bits & 0x3800) == 0x3800); // Bits 11-13 (of 0-31)

    gpsReceivedESFINS = TRUE;

    if (log_ubx_esf_ins)
    {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "================================================");
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "UBX-ESF-INS Message");
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "================================================");
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "Angular Rate X [deg/s] [%c] [%.2f]\n", (gpsESFINS.valid_bits & 0x0100) ? '*' : ' ', gpsESFINS.x_ang_rate_deg_sec_e3 * .001);
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "Angular Rate Y [deg/s] [%c] [%.2f]\n", (gpsESFINS.valid_bits & 0x0200) ? '*' : ' ', gpsESFINS.y_ang_rate_deg_sec_e3 * .001);
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "Angular Rate Z [deg/s] [%c] [%.2f]\n", (gpsESFINS.valid_bits & 0x0400) ? '*' : ' ', gpsESFINS.z_ang_rate_deg_sec_e3 * .001);
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "Acceleration X [m/s^2] [%c] [%d]\n",   (gpsESFINS.valid_bits & 0x0800) ? '*' : ' ', gpsESFINS.x_accel_mg);
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "Acceleration Y [m/s^2] [%c] [%d]\n",   (gpsESFINS.valid_bits & 0x1000) ? '*' : ' ', gpsESFINS.y_accel_mg);
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "Acceleration Z [m/s^2] [%c] [%d]\n",   (gpsESFINS.valid_bits & 0x2000) ? '*' : ' ', gpsESFINS.z_accel_mg);
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "================================================");
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "END of UBX-ESF-INS Message");
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "================================================");
    }
}
#endif /* ENABLE_WHEELTIX */

/*-----------------------------------------------------------------------------
** @brief  Process received UBX NAV-DOP message. (Dilution of Precision)
** @param  unused
** @return unused
**---------------------------------------------------------------------------*/
void receivedUBX_NAV_DOP()
{
    gpsNAVDOP.gDOP = U2Convert(&gpsCommandBuffer[ 4 + UBXHDRLEN]);  // Geometric DOP
    gpsNAVDOP.pDOP = U2Convert(&gpsCommandBuffer[ 6 + UBXHDRLEN]);  // Position DOP
    gpsNAVDOP.tDOP = U2Convert(&gpsCommandBuffer[ 8 + UBXHDRLEN]);  // Time DOP
    gpsNAVDOP.vDOP = U2Convert(&gpsCommandBuffer[10 + UBXHDRLEN]);  // Vertical DOP
    gpsNAVDOP.hDOP = U2Convert(&gpsCommandBuffer[12 + UBXHDRLEN]);  // Horizontal DOP
    gpsNAVDOP.nDOP = U2Convert(&gpsCommandBuffer[14 + UBXHDRLEN]);  // Northing DOP
    gpsNAVDOP.eDOP = U2Convert(&gpsCommandBuffer[16 + UBXHDRLEN]);  // Easting DOP

    gpsReceivedNAVDOP = TRUE;

    // Copy the data into tps shm
    memcpy(&shm_tps_ptr->nav_dop, &gpsNAVDOP, sizeof(ubxNavDop_t));
}


/*-----------------------------------------------------------------------------
** @brief  Process received UBX NAV-EELL message. (Position Error Ellipse)
**         (Currently use NMEA GST instead which has higher accuracy)
** @param  unused
** @return unused
**---------------------------------------------------------------------------*/
void receivedUBX_NAV_EELL()
{
    gpsNAVEELL.errEllipseOrient = U2Convert(&gpsCommandBuffer[6 + UBXHDRLEN]);
    gpsNAVEELL.errEllipseMajor =  U4Convert(&gpsCommandBuffer[8 + UBXHDRLEN]);
    gpsNAVEELL.errEllipseMinor = U4Convert(&gpsCommandBuffer[12 + UBXHDRLEN]);

    gpsReceivedNAVEELL = TRUE;
}


/*-----------------------------------------------------------------------------
** @brief  Process received UBX SEC-SIGN  (Security signing)
** @param  unused
** @return unused
**---------------------------------------------------------------------------*/
void receivedUBX_SEC_SIGN()
{
    ubxSecSign_t *ubx_sec_sign = (ubxSecSign_t *)gpsCommandBuffer;

    int32_t i;
    char_t  sec_sign_byte_buf[256], msg_byte_buf[256], tmp[10];
    memset(sec_sign_byte_buf, 0x00, 256);
    memset(msg_byte_buf, 0x00, 256);
    memset(tmp, 0x00, 10);

    switch (ubx_sec_sign->classID)
    {
        case 0x01:
            if (ubx_sec_sign->messageID == 0x07 && nav_pvt_sha256_enable)
            {
                if (memcmp(ubx_sec_sign->hash, nav_pvt_sha256_hash, 32) == 0)
                {
                    nav_pvt_sec_sign_failed = FALSE;
#if defined(EXTRA_DEBUG)
                    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "===============================================\n");
                    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "UBX-NAV-PVT Host Signature Verification: PASSED\n");
                    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "===============================================\n");
#endif
                }
                else
                {
                    nav_pvt_sec_sign_failed = TRUE;
                    nav_pvt_sec_sign_fail_count++;
#if defined(EXTRA_DEBUG)
                    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "===============================================\n");
                    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "UBX-NAV-PVT Host Signature Verification: FAILED\n");
#endif
                    msg_byte_buf[0] = 0;           ///TODO:Why was is set to NULL, if already memset?;
                    sec_sign_byte_buf[0] = 0;      ///TODO:Why was is set to NULL, if already memset?;
                    for (i = 0; i < 32; i++)
                    {
                        sprintf(tmp, "%02X", nav_pvt_sha256_hash[i]);
                        strcat(msg_byte_buf, tmp);
                        sprintf(tmp, "%02X", ubx_sec_sign->hash[i]);
                        strcat(sec_sign_byte_buf, tmp);
                    }
#if defined(EXTRA_DEBUG)
                    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "[NAV-PVT-HASH][%s]\n", msg_byte_buf);
                    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "[SEC-SIGN-HASH][%s]\n", sec_sign_byte_buf);
                    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "===============================================\n");
#endif
                }
            }
            break;

        case 0x28:
            if (ubx_sec_sign->messageID == 0x00 && hnr_pvt_sha256_enable)
            {
                if (memcmp(ubx_sec_sign->hash, hnr_pvt_sha256_hash, 32) == 0)
                {
                    hnr_pvt_sec_sign_failed = FALSE;
#if defined(EXTRA_DEBUG)
                    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "===============================================\n");
                    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "UBX-HNR-PVT Host Signature Verification: PASSED\n");
                    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "===============================================\n");
#endif
                }
                else
                {
                    hnr_pvt_sec_sign_failed = TRUE;
                    hnr_pvt_sec_sign_fail_count++;
#if defined(EXTRA_DEBUG)
                    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "===============================================\n");
                    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "UBX-HNR-PVT Host Signature Verification: FAILED\n");
#endif
                    msg_byte_buf[0] = 0;       ///TODO:Why was is set to NULL, if already memset?;
                    sec_sign_byte_buf[0] = 0;  ///TODO:Why was is set to NULL, if already memset?;
                    for (i = 0; i < 32; i++)
                    {
                        sprintf(tmp, "%02X", hnr_pvt_sha256_hash[i]);
                        strcat(msg_byte_buf, tmp);
                        sprintf(tmp, "%02X", ubx_sec_sign->hash[i]);
                        strcat(sec_sign_byte_buf, tmp);
                    }
#if defined(EXTRA_DEBUG)
                    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "[HNR-PVT-HASH][%s]\n", msg_byte_buf);
                    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "[SEC-SIGN-HASH][%s]\n", sec_sign_byte_buf);
                    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "===============================================\n");
#endif
                }
            }
            break;

        default:
            break;
    }
}


/*-----------------------------------------------------------------------------
** @brief  Process received UBX HNR-INS message. (High Navigation Rate Vechicle Dynamics)
** @param  unused
** @return unused
**---------------------------------------------------------------------------*/
void receivedUBX_HNR_INS()
{
    gpsHNRINS.valid_bits            = U4Convert(&gpsCommandBuffer[0 + UBXHDRLEN]);
    gpsHNRINS.time_of_week_msec     = U4Convert(&gpsCommandBuffer[8 + UBXHDRLEN]);
    gpsHNRINS.x_ang_rate_deg_sec_e3 = I4Convert(&gpsCommandBuffer[12 + UBXHDRLEN]);
    gpsHNRINS.y_ang_rate_deg_sec_e3 = I4Convert(&gpsCommandBuffer[16 + UBXHDRLEN]);
    gpsHNRINS.z_ang_rate_deg_sec_e3 = I4Convert(&gpsCommandBuffer[20 + UBXHDRLEN]);
    gpsHNRINS.x_accel_mg            = I4Convert(&gpsCommandBuffer[24 + UBXHDRLEN]);
    gpsHNRINS.y_accel_mg            = I4Convert(&gpsCommandBuffer[28 + UBXHDRLEN]);
    gpsHNRINS.z_accel_mg            = I4Convert(&gpsCommandBuffer[32 + UBXHDRLEN]);

    gpsReceivedESFINS = TRUE;
#if defined(EXTRA_DEBUG)
    if (log_ubx_hnr_ins)
    {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "================================================\n");
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "UBX-HNR-INS Message\n");
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "================================================\n");
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "Angular Rate X [deg/s] [%c] [%.2f]\n", (gpsESFINS.valid_bits & 0x0100) ? '*' : ' ', gpsESFINS.x_ang_rate_deg_sec_e3 * .001);
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "Angular Rate Y [deg/s] [%c] [%.2f]\n", (gpsESFINS.valid_bits & 0x0200) ? '*' : ' ', gpsESFINS.y_ang_rate_deg_sec_e3 * .001);
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "Angular Rate Z [deg/s] [%c] [%.2f]\n", (gpsESFINS.valid_bits & 0x0400) ? '*' : ' ', gpsESFINS.z_ang_rate_deg_sec_e3 * .001);
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "Acceleration X [m/s^2] [%c] [%d]\n",   (gpsESFINS.valid_bits & 0x0800) ? '*' : ' ', gpsESFINS.x_accel_mg);
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "Acceleration Y [m/s^2] [%c] [%d]\n",   (gpsESFINS.valid_bits & 0x1000) ? '*' : ' ', gpsESFINS.y_accel_mg);
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "Acceleration Z [m/s^2] [%c] [%d]\n",   (gpsESFINS.valid_bits & 0x2000) ? '*' : ' ', gpsESFINS.z_accel_mg);
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "================================================\n");
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "END of UBX-HNR-INS Message\n");
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "================================================\n");
    }
#endif
}


/*-----------------------------------------------------------------------------
** @brief  Process received UBX HNR-PVT message. (High Navigation Rate, Navigation PVT solution)
** @param  unused
** @return unused
**---------------------------------------------------------------------------*/
void receivedUBX_HNR_PVT()
{
    // Flags
    uint8_t gnss_fix_ok = gpsCommandBuffer[17 + UBXHDRLEN] & 0x01;  // 17 for HNR-PVT, was 21 for NAV-PVT

#if defined(HEAVY_DUTY)
    memset(&gpsNAVPVT, 0, sizeof(gpsNAVPVT)); /* Do not allow stale fix to linger */
#endif

    // We don't want to use HNR data until we get UBX-NAV-PVT's with a valid fix (which sets gnss_fix_ok)
    if (gnss_fix_ok)
    {
#if !defined(HEAVY_DUTY)
        memset(&gpsNAVPVT, 0, sizeof(gpsNAVPVT));
#endif
        gpsNAVPVT.time_of_week_msec = U4Convert(&gpsCommandBuffer[0 + UBXHDRLEN]);
        gpsNAVPVT.year_utc  = U2Convert(&gpsCommandBuffer[4 + UBXHDRLEN]);
        gpsNAVPVT.month_utc = gpsCommandBuffer[6 + UBXHDRLEN];
        gpsNAVPVT.day_utc   = gpsCommandBuffer[7 + UBXHDRLEN];
        gpsNAVPVT.hour_utc  = gpsCommandBuffer[8 + UBXHDRLEN];
        gpsNAVPVT.min_utc   = gpsCommandBuffer[9 + UBXHDRLEN];
        gpsNAVPVT.sec_utc   = gpsCommandBuffer[10 + UBXHDRLEN];
        gpsNAVPVT.nsec_utc  = I4Convert(&gpsCommandBuffer[12 + UBXHDRLEN]);

        gpsNAVPVT.fix_type  = gpsCommandBuffer[16 + UBXHDRLEN];

        gpsNAVPVT.flag_differential_corrections_applied = gpsCommandBuffer[17 + UBXHDRLEN] & 0x02;
        gpsNAVPVT.flag_heading_valid = gpsCommandBuffer[17 + UBXHDRLEN] & 0x10;  // 0x10 for HNR, was 0x20 for NAV-PVT
        gpsNAVPVT.flag_utc_fully_resolved = gpsCommandBuffer[11 + UBXHDRLEN] & 0x04; // pos 11 for both HNR & NAV, &0x04 for both

        gpsNAVPVT.lon_deg_e7 = I4Convert(&gpsCommandBuffer[20 + UBXHDRLEN]);
        gpsNAVPVT.lat_deg_e7 = I4Convert(&gpsCommandBuffer[24 + UBXHDRLEN]);
        gpsNAVPVT.height_above_elipsoid_mm = U4Convert(&gpsCommandBuffer[28 + UBXHDRLEN]);
        gpsNAVPVT.height_above_MSL_mm =  U4Convert(&gpsCommandBuffer[32 + UBXHDRLEN]);

        gpsNAVPVT.ground_speed_mmsec = I4Convert(&gpsCommandBuffer[36 + UBXHDRLEN]);

        if (gpsNAVPVT.flag_heading_valid)
        {
            gpsNAVPVT.heading_of_motion_deg_e5 = I4Convert(&gpsCommandBuffer[44 + UBXHDRLEN]);
            gpsNAVPVT.heading_of_vehicle_deg_e5 = I4Convert(&gpsCommandBuffer[48 + UBXHDRLEN]);
            gpsNAVPVT.heading_accuracy_est_deg_e5 = U4Convert(&gpsCommandBuffer[64 + UBXHDRLEN]);
        }

        gpsNAVPVT.horiz_accuracy_est_mm = U4Convert(&gpsCommandBuffer[52 + UBXHDRLEN]);
        gpsNAVPVT.vert_accuracy_est_mm = U4Convert(&gpsCommandBuffer[56 + UBXHDRLEN]);
        gpsNAVPVT.speed_accuracy_est_mmsec = U4Convert(&gpsCommandBuffer[60 + UBXHDRLEN]);

        gpsReceivedHNRPVT = TRUE;

        last_HNR_received_sec = time(0L);
    }
    else
    {
        gpsReceivedHNRPVT = FALSE;
    }

    // Compute SHA256 Hash for Host Interface Signature Verification
    if (hnr_pvt_sha256_enable)
    {
        uint8_t data[HNR_PVT_UBX_MSG_LENGTH + UBX_SEC_SIGN_SEEDS];
#if !defined(MY_UNIT_TEST)
        SHA256_CTX ctx;
#endif
        // Memset data to 0x00
        memset(data, 0x00, HNR_PVT_UBX_MSG_LENGTH + UBX_SEC_SIGN_SEEDS);

        // Set High & Low Fixed Seeds
        memcpy(&data[0], FIXED_SEED_HIGH, 4);
        memcpy(&data[HNR_PVT_UBX_MSG_LENGTH + UBX_SEC_SIGN_SEEDS - 4], FIXED_SEED_LOW, 4);

        // Memcpy gpsCommandBuffer to data[8]
        memcpy(&data[8], gpsCommandBuffer, HNR_PVT_UBX_MSG_LENGTH);
#if !defined(MY_UNIT_TEST)
        // Generate SHA256 Hash
        SHA256_Init(&ctx);
        SHA256_Update(&ctx, data, HNR_PVT_UBX_MSG_LENGTH + UBX_SEC_SIGN_SEEDS);
        SHA256_Final(hnr_pvt_sha256_hash, &ctx);
#endif
    }
}


/*-----------------------------------------------------------------------------
** @brief  Process received UBX CFG-RINV message. (Config - Remote Inventory)
** @param  payload_len: Length of message payload  (ublox config version string)
** @return unused
**---------------------------------------------------------------------------*/
void receivedUBX_CFG_RINV(uint16_t payload_len)
{
    // First byte is flags
    uint8_t flag_binary_data = gpsCommandBuffer[0 + UBXHDRLEN] & 0x02;
    if (payload_len <= 1) { // Note: "Empty" will be len 1 because payload always has the flag byte
        strcpy((char_t *) shm_tps_ptr->ublox_config_version_str, "--empty--");
    } else if (flag_binary_data) {
        strcpy((char_t *) shm_tps_ptr->ublox_config_version_str, "--binary data--");
    } else {
        memcpy(shm_tps_ptr->ublox_config_version_str, &gpsCommandBuffer[1 + UBXHDRLEN], payload_len-1);
        // Terminate string, jic
        shm_tps_ptr->ublox_config_version_str[payload_len-1] = 0;
    }
#if defined(EXTRA_DEBUG)
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, " received UBX_CFG_RINV: ubx cfg verstr is \"%s\"\n", shm_tps_ptr->ublox_config_version_str);
#endif
}


// Process received UBX MON-VER message
void receivedUBX_MON_VER(uint16_t payload_len)
{
    // First byte is flags
    char_t * found_ptr;
    if (payload_len == 0) {
        strcpy((char_t *) shm_tps_ptr->ublox_firmware_version_str, "--empty--");
    } else {    // Search for "FWVER=" and grab rest of string
        found_ptr = memmem(gpsCommandBuffer, payload_len, "FWVER=", 6);
        if (found_ptr) {
            memcpy((char_t *) shm_tps_ptr->ublox_firmware_version_str, found_ptr+6, MAX_FWVER_LENGTH);
        } else {    // UBlox sent us something unexpected in the payload ... help!
            strcpy((char_t *) shm_tps_ptr->ublox_firmware_version_str, "NEED FIXIN");
#if defined(EXTRA_DEBUG)
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, " received UBX_MON_VER w paylen=%d but didnt find 'FWVER='\n", payload_len);
#endif
#ifdef DEBUG
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"   gpsCommandBuffer = [\n");
            uint8_t i;
            for (i=0; i<payload_len; i++) {
                if (gpsCommandBuffer[i] >= 0x20 && gpsCommandBuffer[i] <= 0x6f)
                    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME," %02x(%c)\n", gpsCommandBuffer[i], gpsCommandBuffer[i]);
                else
                    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME," %02x\n", gpsCommandBuffer[i]);
            }
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME," ]\n");
#endif
        }
    }
#if defined(EXTRA_DEBUG)
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, " received UBX_MON_VER: ubx firmware ver is \"%s\"\n", shm_tps_ptr->ublox_firmware_version_str);
#endif
}



/*-----------------------------------------------------------------------------
** @brief  Process received UBX ESF-MEASURE message. (External Sensor Fusion - Measurement Data)
** @param  payload_len: (not currently used)
** @return unused
**---------------------------------------------------------------------------*/
#if defined(ENABLE_WHEELTIX)
void receivedUBX_ESF_MEASURE(uint16_t payload_len)
{
    if (log_ubx_esf_meas)
    {
        ubxEsfMeas_t            *ubx_esf_measure = (ubxEsfMeas_t *)gpsCommandBuffer;
        uint16_t                 payload_size, i, N;
        ubxEsfCalibTag_t         flags;
        uint32_t timetag,        calibtag=0;
        ubxEsfData_t             field;
        char_t packet_data[255], tmp[10];

        payload_size   = ubx_esf_measure->header.payload_size;
        timetag        = ubx_esf_measure->timetag;
        flags.data.val = ubx_esf_measure->flags;

        if (flags.data.bit.calib_tag_valid)
        {
            // If the calibTag is valid, then its at the end of the packet.
            // Note: tickData[N] may walk off the end of the tickData array, and
            // in this case, and this case only, that's OK.
            N = (payload_size - 12) / 4;
            calibtag = ubx_esf_measure->tickData[N].data.val;
        }
        else
        {
            // No calibTag included
            N = (payload_size - 8) / 4;
        }

        packet_data[0] = 0;      ///TODO:Why was is set to NULL, if already memset?;
        for (i = 0; i < payload_size + sizeof(ubxHeader_t); i++)
        {
            sprintf(tmp, "%02x ", gpsCommandBuffer[i]);
            strcat(packet_data, tmp);
        }
#if defined(EXTRA_DEBUG)
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "UBX_ESF_MEASURE packet: %s\n", packet_data);
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "UBX_ESF_MEASURE Timetag: %u  CalibTag: %u\n", timetag, calibtag);
#endif
        for (i = 0; i < N; i++)
        {
            field.data.val = ubx_esf_measure->tickData[i].data.val;
#if defined(EXTRA_DEBUG)
            switch (field.data.bit.datatype)
            {
                case ESF_GYRO_Z:
                    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "GYRO Z: %f deg/S\n", (float64_t)field.data.bit.datafield * 0.000244141);
                    break;

                case ESF_FRONT_LEFT_WT:
                    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "FRONT LEFT WT: %d  DIR: %s\n", field.data.bit_wt.datafield, (field.data.bit_wt.direction) ? "REVERSE" : "FORWARD" );
                    break;

                case ESF_FRONT_RIGHT_WT:
                    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "FRONT RIGHT WT: %d  DIR: %s\n", field.data.bit_wt.datafield, (field.data.bit_wt.direction) ? "REVERSE" : "FORWARD" );
                    break;

                case ESF_REAR_LEFT_WT:
                    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "REAR LEFT WT: %d  DIR: %s\n", field.data.bit_wt.datafield, (field.data.bit_wt.direction) ? "REVERSE" : "FORWARD" );
                    break;

                case ESF_REAR_RIGHT_WT:
                    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "REAR RIGHT WT: %d  DIR: %s\n", field.data.bit_wt.datafield, (field.data.bit_wt.direction) ? "REVERSE" : "FORWARD" );
                    break;

                case ESF_SPEED_TICK:
                    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "SPEED WT: %d  DIR: %s\n", field.data.bit_wt.datafield, (field.data.bit_wt.direction) ? "REVERSE" : "FORWARD" );
                    break;

                case ESF_SPEED:
                    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "SPEED WT: %d\n", field.data.bit.datafield);
                    break;

                case ESF_GYRO_TEMP:
                    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "GYRO TEMP: %f deg C\n", (float64_t)field.data.bit.datafield / 100);
                    break;

                case ESF_GYRO_Y:
                    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "GYRO Y: %f deg/S\n", (float64_t)field.data.bit.datafield * 0.000244141);
                    break;

                case ESF_GYRO_X:
                    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "GYRO X: %f deg/S\n", (float64_t)field.data.bit.datafield * 0.000244141);
                    break;

                case ESF_NOT_USED:
                    break;

                case ESF_ACCEL_X:
                    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "ACCEL X: %f m/s/s\n", (float64_t)field.data.bit.datafield * 0.000976563);
                    break;

                case ESF_ACCEL_Y:
                    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "ACCEL Y: %f m/s/s\n", (float64_t)field.data.bit.datafield * 0.000976563);
                    break;

                case ESF_ACCEL_Z:
                    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "ACCEL Z: %f m/s/s\n", (float64_t)field.data.bit.datafield * 0.000976563);
                    break;

                default:
                    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "ESF_MEASURE unhandled type received\n");
                    break;
            }
#endif
        }
    }
}


/*-----------------------------------------------------------------------------
** @brief  Process received UBX ESF-STATUS message. (External Sensor Fusion - Status)
** @param  payload_len: (not currently used)
** @return unused
**---------------------------------------------------------------------------*/
void receivedUBX_ESF_STATUS(uint16_t payload_len)
{
    if (log_ubx_esf_status)
    {
    }
}

#endif /* ENABLE_WHEEl */
/*-----------------------------------------------------------------------------
** @brief Process incoming chars from GNSS until we get a complete message
** @description Accepts one char at a time.
**---------------------------------------------------------------------------*/
static void Process_Incoming_GPS_Stream_Char(uint8_t ch)
{
    int32_t       rc;
    request_item *cur;
    tpsRawMsgRes  raw_data;
    int32_t       RTCMMsgHeaderLen = 0;
    int32_t       RTCMMsgBodyLen = 0;
    uint8_t       UBXMsgClassId = 0;
    uint8_t       UBXMsgId = 0;
    uint16_t      UBXMsgBodyLen = 0;
    uint16_t      UBXMsgTotalLen = 0;
#if defined(EXTRA_DEBUG)
    int32_t  i=0;
#endif

    /* Dont add an FF into an empty buffer */
    if (ch == 0xFF) {
        if (chars_in_buffer == 0) {
            ++shm_tps_ptr->debug_cnt_unused_chars;
            return;
        }
    }

    /* TODO: Take printf's out of gps.c .. the printfs are chat_ublox.c only code !! */
    /* Add char to incoming command buffer.  If we hit buffer limit, we flush & start over. */
    if (chars_in_buffer >= MAX_GPS_MESSAGE_LEN)
    {
#if defined(EXTRA_DEBUG)
        if (tpsDebugOutput > 0)
        {
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME," ERROR!  Hit %d char limit.  CurState=%d Here's the buffer: \n", MAX_GPS_MESSAGE_LEN, msgMunchingStage);
            int32_t i;
            for (i=0; i<chars_in_buffer; i++)
                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"%02x \n", gpsCommandBuffer[i] & 0xff);
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"\n");
        }
#endif
        ++shm_tps_ptr->debug_buffer_aborts;
        Dump_GPS_Buffer_Up_To_Next_String_Start();
    }


    // Add char to our buffer
    gpsCommandBuffer[chars_in_buffer] = ch;
    ++chars_in_buffer;

    /* COMMAND CHAR PROCESSING */

    switch (msgMunchingStage) {

        case STAGE_0_AWAITING_START_OF_NEXT_COMMAND:
            if (chars_in_buffer >= 3) {
                /* Detect NMEA command - 0x24 0x47 [NLP] are the start of its command characters. */
                if (       gpsCommandBuffer[0] == 0x24 &&
                           gpsCommandBuffer[1] == 0x47 &&
                               (gpsCommandBuffer[2] == 'N' ||
                                gpsCommandBuffer[2] == 'L' ||
                                gpsCommandBuffer[2] == 'P')) {
                    msgMunchingStage = STAGE_1_GETTING_NMEA_COMMAND;
                /* Detect RTCM command - 0xAA 0x44 0x12 are the start of its command characters. */
                } else if (gpsCommandBuffer[0] == 0xAA &&
                           gpsCommandBuffer[1] == 0x44 &&
                           gpsCommandBuffer[2] == 0x12) {
                    msgMunchingStage = STAGE_2_GETTING_RTCM_COMMAND;
                    RTCMMsgHeaderLen = 0;
                /* Detect UBX command - 0xB5 0x62 [ µ b ] are the start of its command characters. */
                } else if (gpsCommandBuffer[0] == 0xB5 &&
                           gpsCommandBuffer[1] == 0x62) {
                    msgMunchingStage = STAGE_3_GETTING_UBX_COMMAND;
                    UBXMsgClassId = 0;

                // We have neither, so garbage in buff, toss up to first start char of either NMEA RTCM or UBX
                } else {
                    Dump_GPS_Buffer_Up_To_Next_String_Start();
                }
            }
            break;

        case STAGE_1_GETTING_NMEA_COMMAND:
            if (ch == 0x0d || ch == 0x0a || ch == 0xb5)  // detect NMEA term chars, or a UBX starting immed. after!
            {
                // Woohoo, we caught one!
                ++shm_tps_ptr->debug_gps_cmd_count_nmea;
#if defined(ENABLE_SR)

                // Send it to SR to record it.  Note we are including the terminating character 0x0d 0x0a or 0xb5,
                // which is needed to indicate the end of the NMEA.  The 0xb5 will become an extra char in playback
                // but it is harmless to have 0xb5 0xb5 .. TPS will still find the start of the UBX message just fine.
                if (   (NULL                   != shm_sr_ptr)
                    && (SR_RECORDING_MODE_CHAR == shm_sr_ptr->sr_mode)
                    && (TRUE                   == shm_sr_ptr->recordingON)
                    && (TRUE                   == cfg_record_tps_as_raw)
                   )
                {
                    sendSRMessage(SR_CLIENTID_TPSRAW, 0, gpsCommandBuffer, chars_in_buffer);
                    ++shm_tps_ptr->debug_cnt_sr_raw_sends;
                }
#endif

#if defined(EXTRA_DEBUG)
                // Debug print
                if (tpsDebugOutput & DBG_OUTPUT_NMEA) {
                    gpsCommandBuffer[chars_in_buffer-1] = 0; // Null terminate our command for easy printfing
                    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"%s\n",gpsCommandBuffer);
                }
#endif
                // Process it
                gpsCommMessageReceived();

                // Someone's requested a RAW copy of this NMEA message
                if (req_list.head != NULL) {
                    // Lock the mutex controlling the request list -- KNOTE: NOT A GREAT IDEA AS MAKES US BLOCK
                    if ((rc = pthread_mutex_lock(&tpsRawData_mutex)) != 0)
                    {
#if defined(EXTRA_DEBUG)
                        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"TPS RAW Data Reg List Mutex Lock Error (%d)\n", rc);
#endif
                    }
                    cur = req_list.head;
                    while(cur != NULL) {
                        // If this NMEA message is in the list, lets send a copy of it
                        if (strncmp((char *)cur->raw_msg_header,(char *) gpsCommandBuffer,cur->hdrLen) == 0)
                        {
                            memset(&raw_data,0,sizeof(raw_data));
                            raw_data.msg_length = chars_in_buffer;
                            memcpy(raw_data.msgbuf, gpsCommandBuffer, chars_in_buffer);
                            raw_data.msgFilterIdx = cur->msgFilterIdx;
                            gpsSendTpsRAWDataInd(&raw_data);
                            break;
                        }
                        cur=cur->next;
                    }
                    if ((rc = pthread_mutex_unlock(&tpsRawData_mutex)) != 0)
                    {
#if defined(EXTRA_DEBUG)
                        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"TPS RAW Data Reg List Mutex Unlock Error (%d)\n", rc);
#endif
                    }
                }

                // And reset to await the next command
                chars_in_buffer = 0;
                msgMunchingStage = STAGE_0_AWAITING_START_OF_NEXT_COMMAND;
            }

            // Detect if we have a UBX msg immediately after a NMEA with no \n
            if (ch == 0xb5) {
                ++shm_tps_ptr->debug_gps_cmd_count_nmea_unterminated;

                // Put back the B5 .. at the front
                gpsCommandBuffer[0] = 0xb5;
                chars_in_buffer = 1;
            }
            break;


        case STAGE_2_GETTING_RTCM_COMMAND:

            /* Extract our lengths */
            if (RTCMMsgHeaderLen == 0 && chars_in_buffer >= 9)
            {
                RTCMMsgHeaderLen = gpsCommandBuffer[2];
                /* Multi-bytes are in little endian order. */
                RTCMMsgBodyLen =  ((gpsCommandBuffer[8] & 0xff) << 8) + (gpsCommandBuffer[7] & 0xff);

                RTCMMsgTotalLen = RTCMMsgHeaderLen + RTCMMsgBodyLen + RTCM_CRC_LEN;
            }
            /* Is it done? */
            if (RTCMMsgHeaderLen > 0 && chars_in_buffer >= RTCMMsgTotalLen - 1)     // ASK AARON: Why -1 ?   BUG?  CHECKSUM IGNORE?
            {
                // Woohoo, we caught another one!
                ++shm_tps_ptr->debug_gps_cmd_count_rtcm;
#if defined(ENABLE_SR)
                // Send it to SR to record it.
                if (   (NULL                   != shm_sr_ptr)
                    && (SR_RECORDING_MODE_CHAR == shm_sr_ptr->sr_mode)
                    && (TRUE                   == shm_sr_ptr->recordingON)
                    && (TRUE                   == cfg_record_tps_as_raw)
                   )
                {
                    sendSRMessage(SR_CLIENTID_TPSRAW, 0, gpsCommandBuffer, chars_in_buffer);
                    ++shm_tps_ptr->debug_cnt_sr_raw_sends;
                }
#endif
                // Send it off
                gpsSendTpsRTCMDataInd();
#if defined(EXTRA_DEBUG)
                if (tpsDebugOutput & DBG_OUTPUT_RTCM)
                {
                    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"RTCM Header Len: %d (0x%x), Body Len: %d (0x%x), Total Len: %d (0x%x)\n",
                           RTCMMsgHeaderLen, RTCMMsgHeaderLen,
                           RTCMMsgBodyLen, RTCMMsgBodyLen,
                           RTCMMsgTotalLen, RTCMMsgTotalLen);
                }
#endif
                // Remove msg from buffer
                Remove_First_N_Bytes_From_GPS_Buffer(RTCMMsgTotalLen);
                // And reset to await the next command
                RTCMMsgHeaderLen = 0;
                msgMunchingStage = STAGE_0_AWAITING_START_OF_NEXT_COMMAND;
            }
            break;

        case STAGE_3_GETTING_UBX_COMMAND:
            /* Extract our lengths */
            if (UBXMsgClassId == 0 && chars_in_buffer >= 6)
            {
                UBXMsgClassId  = gpsCommandBuffer[2];
                UBXMsgId       = gpsCommandBuffer[3];
                /* UBX Length is in little endian order. */
                UBXMsgBodyLen  = ((gpsCommandBuffer[5] & 0xff) << 8) + (gpsCommandBuffer[4] & 0xff);
                UBXMsgTotalLen = UBXMsgBodyLen + 6 /* Header len */ + 2 /* CRC Len */;
            }

            /* Is it done? */
            if (UBXMsgClassId > 0 && chars_in_buffer >= UBXMsgTotalLen)
            {
                // Woohoo, we caught another one!
                ++shm_tps_ptr->debug_gps_cmd_count_ubx;
#if defined(ENABLE_SR)
                // Send it to SR to record it.
                if (   (NULL                   != shm_sr_ptr)
                    && (SR_RECORDING_MODE_CHAR == shm_sr_ptr->sr_mode)
                    && (TRUE                   == shm_sr_ptr->recordingON)
                    && (TRUE                   == cfg_record_tps_as_raw)
                   )
                {
                    sendSRMessage(SR_CLIENTID_TPSRAW, 0, gpsCommandBuffer, chars_in_buffer);
                    ++shm_tps_ptr->debug_cnt_sr_raw_sends;
                }
#endif
                // Validate message checksum
                uint8_t csums[2];
                Compute8BitFletcherChecksums((uint8_t *)&gpsCommandBuffer[2], UBXMsgBodyLen + 4, csums);
                if (gpsCommandBuffer[UBXMsgTotalLen-2] != csums[0] || gpsCommandBuffer[UBXMsgTotalLen-1] != csums[1])
                {
                    ++shm_tps_ptr->debug_gps_cmd_count_ubx_bad_csum;
#if defined(EXTRA_DEBUG)
                    if (tpsDebugOutput & DBG_OUTPUT_TPSGEN) {
                        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"UBX CSum error: Msg %02x-%02x len %d wanted csum %02x%02x but we got %02x%02x !\n",
                                UBXMsgClassId, UBXMsgId,
                                UBXMsgTotalLen,
                                gpsCommandBuffer[UBXMsgTotalLen-2],
                                gpsCommandBuffer[UBXMsgTotalLen-1],
                                csums[0],
                                csums[1]);
                    }
#endif

                // Process It
                } else {
                    // UBX-NAV-* data messages and friends
                    if (   (UBXMsgClassId == 0x01 && (UBXMsgId == 0x07 || UBXMsgId == 0x26 || UBXMsgId == 0x04
                                                      || UBXMsgId == 0x3D || UBXMsgId == 0x03 || UBXMsgId == 0x35))
                        || (UBXMsgClassId == 0x28 && UBXMsgId == 0x00)
                        || (UBXMsgClassId == 0x0A && UBXMsgId == 0x09)
                        || (UBXMsgClassId == 0x10 && (UBXMsgId == 0x15 || UBXMsgId == 0x03)))
                    {
                        // Process, copy for debug output
                        if (UBXMsgId == 0x07) {
                            ++shm_tps_ptr->debug_gps_cmd_count_ubx_nav_pvt_total;
                            if (time(0L) - last_HNR_received_sec > 1) {
                                receivedUBX_NAV_PVT();
                            } else {
                                ++shm_tps_ptr->debug_gps_cmd_count_ubx_nav_pvt_unused;
#if defined(EXTRA_DEBUG)
                                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"PVT is too old, dropping it: dropped=0x%x.\n",shm_tps_ptr->debug_gps_cmd_count_ubx_nav_pvt_unused);
#endif
                            }
                        } else if (UBXMsgId == 0x26) {
                            receivedUBX_NAV_TIMELS();
                            ++shm_tps_ptr->debug_gps_cmd_count_ubx_nav_timels;
                        } else if (UBXMsgId == 0x04) {
                            receivedUBX_NAV_DOP();
                            ++shm_tps_ptr->debug_gps_cmd_count_ubx_nav_dop;
                        } else if (UBXMsgId == 0x3D) {
                            receivedUBX_NAV_EELL();
                            ++shm_tps_ptr->debug_gps_cmd_count_ubx_nav_eell;
                        } else if (UBXMsgId == 0x03) {
                            receivedUBX_NAV_STATUS();
                            ++shm_tps_ptr->debug_gps_cmd_count_ubx_nav_status;
                        } else if (UBXMsgId == 0x35) {
                            receivedUBX_NAV_SAT();
                            ++shm_tps_ptr->debug_gps_cmd_count_ubx_nav_sat;
                        } else if (UBXMsgId == 0x09) {
                            receivedUBX_MON_HW();
                            ++shm_tps_ptr->debug_gps_cmd_count_ubx_mon_hw;
                        } else if (UBXMsgId == 0x00) {
                            ++shm_tps_ptr->debug_gps_cmd_count_ubx_hnr_pvt_total;
                            receivedUBX_HNR_PVT();
                            if (gpsReceivedHNRPVT == FALSE) {
                                ++shm_tps_ptr->debug_gps_cmd_count_ubx_hnr_pvt_unused;
                            }
#if defined(ENABLE_WHEELTIX)
                        } else if (UBXMsgId == 0x15) {
                            receivedUBX_ESF_INS();
                            ++shm_tps_ptr->debug_gps_cmd_count_ubx_esf_ins;
                        } else if (UBXMsgId == 0x03 && UBXMsgClassId == 0x10) {
                            receivedUBX_ESF_RAW();
                            ++shm_tps_ptr->debug_gps_cmd_count_ubx_esf_raw;
#endif /* ENABLE_WHEELTIX */
                        } else {    // Really should never get here, "i need programming" type error!
                            ++shm_tps_ptr->debug_gps_cmd_count_ubx_other;
#if defined(EXTRA_DEBUG)
                            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME," received UNHANDLED UBX msg 0x%02x-0x%02x, len=%u\n", UBXMsgClassId, UBXMsgId, UBXMsgTotalLen);
#endif
                        }
#if !defined(ENABLE_NMEA_SOLUTION) /* Use UBX PVT or NMEA GGA, but not both. */
                        gpsCommSendTpsDataIndFromUbxData();
#endif
                        ++shm_tps_ptr->debug_cnt_comm_send_tps_data_ind;
                        /*
                         * Update system clock only when we have a good enough fix type.
                         */
                        if (gpsReceivedHNRPVT == TRUE || gpsReceivedNAVPVT == TRUE){ /* Really means is gnss_fix_ok is good. */
                            /* DEBUG EELL vs GST */
                            if (gpsReceivedNAVEELL == TRUE && gpsReceivedGPGST == TRUE && gpsReceivedNAVPVT == TRUE) {

#if defined(EXTRA_DEBUG)
                                if (shm_tps_ptr->debug_cnt_comm_send_tps_data_ind % 10 == 1) {
                                    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"UvN:   Major:    Minor:    Orient:   LatErr:  LonErr:  AltErr:\n");
                                }
                                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME," EELL: %9.4lf %9.4lf %9.4lf   %4.2lf     %4.2lf     %4.2lf\n",
                                    gpsNAVEELL.errEllipseMajor / 1000.0,     /* Milimeters to meters */
                                    gpsNAVEELL.errEllipseMinor / 1000.0,     /* Milimeters to meters */
                                    gpsNAVEELL.errEllipseOrient / 100.0,    /* 1E-2 scaling into degrees */
                                    gpsNAVPVT.horiz_accuracy_est_mm / 1000.0, /* To meters */
                                    gpsNAVPVT.horiz_accuracy_est_mm / 1000.0, /* To meters */
                                    gpsNAVPVT.vert_accuracy_est_mm / 1000.0);   /* To meters */
                                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME," GST:  %9.4lf %9.4lf %9.4lf   %4.2lf     %4.2lf     %4.2lf\n",
                                        gpsGPST.smjr_err, gpsGPST.smnr_err, gpsGPST.orient, gpsGPST.lat_err, gpsGPST.lon_err, gpsGPST.alt_err);
#endif
                            }                       

#ifdef DBG_SYSTEMTIMEUPDATE
                            // And update the linux system time if necessary  (from old gpsCommSendTpsDataInd() )
                            if (gpsNAVPVT.sec_utc != lastPvtSecond) {
                                gpsUpdateLinuxSysTime(UPDATE_TIME_FROM_UBX_DATA);
                                lastPvtSecond = gpsNAVPVT.sec_utc;
                            }
#endif /*DBG_SYSTEMTIMEUPDATE*/

                        }
                        // Reset all the msg received indicators
                        gpsReceivedNAVPVT = gpsReceivedHNRPVT = FALSE;
                        gpsReceivedTIMELS = gpsReceivedNAVDOP = gpsReceivedNAVEELL = FALSE;
                        // gpsReceivedESFINS = gpsReceivedESFRAW = FALSE;  // Someday
                    // UBX-CFG-RINV (0x06 0x34) message identifying UBlox's configuration via a string config put into RINV
                    } else if (UBXMsgClassId == 0x06 && UBXMsgId == 0x34) {
                        receivedUBX_CFG_RINV(UBXMsgBodyLen);
                        ++shm_tps_ptr->debug_gps_cmd_count_ubx_cfg_rinv;
#if !defined(HEAVY_DUTY)
                    // UBX-MON-VER (0x0A 0x04) message identifying UBlox's firmware version
                    } else if (UBXMsgClassId == 0x0A && UBXMsgId == 0x04) {
                        receivedUBX_MON_VER(UBXMsgBodyLen);
                        ++shm_tps_ptr->debug_gps_cmd_count_ubx_mon_ver;
#endif
                    // ACK's & NAK's from UBX unit
                    } else if (UBXMsgClassId == 0x05 && (UBXMsgId == 0x01 || UBXMsgId == 0x00)) {
//Need ack/nak block for config messages.
                        if (UBXMsgId == 0x01) {
                            ++shm_tps_ptr->debug_gps_cmd_count_ubx_ack_ack;
#if defined(EXTRA_DEBUG)
                            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, " received ACK-ACK msg for our 0x%02x 0x%02x request!\n",
                                     gpsCommandBuffer[UBXHDRLEN+0], gpsCommandBuffer[UBXHDRLEN+1]);
#endif
                        } else {
                            ++shm_tps_ptr->debug_gps_cmd_count_ubx_ack_nak;
                            shm_tps_ptr->debug_latest_nak_msg_classid = gpsCommandBuffer[UBXHDRLEN+0];
                            shm_tps_ptr->debug_latest_nak_msg_msgid = gpsCommandBuffer[UBXHDRLEN+1];
#if defined(EXTRA_DEBUG)
                            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, " received ACK-NAK msg for our 0x%02x 0x%02x request!\n",
                                    shm_tps_ptr->debug_latest_nak_msg_classid, shm_tps_ptr->debug_latest_nak_msg_msgid);
#endif
                        }
                    // UBX-INF messages - 0x00-ERROR 01-WARNING 02-NOTICE 03-TEST
                    } else if (UBXMsgClassId == 0x04 && UBXMsgId >= 0x00 && UBXMsgId <= 0x03) {
                        gpsCommandBuffer[6+UBXMsgBodyLen] = 0;  // Need to Null terminate msg.  No overrun risk as checksum bytes follow end of msg.
                        if (UBXMsgId == 0x00) {
                            ++shm_tps_ptr->debug_gps_cmd_count_ubx_inf_err;
#if defined(EXTRA_DEBUG)
                            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, " Received UBX-ERROR: %s", &gpsCommandBuffer[6]);
#endif
                        } else if (UBXMsgId == 0x01) {
                            ++shm_tps_ptr->debug_gps_cmd_count_ubx_inf_warn;
#if defined(EXTRA_DEBUG)
                            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, " Received UBX-WARNING: %s", &gpsCommandBuffer[6]);
#endif
                        } else {
                            ++shm_tps_ptr->debug_gps_cmd_count_ubx_inf_rest;
#if defined(EXTRA_DEBUG)
                            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, " received UBX-INF-%s: %s", UBXMsgId == 0x02 ? "NOTICE" : "TEST", &gpsCommandBuffer[6]);
#endif
                        }
                        // Copy msg to shm
                        if (UBXMsgBodyLen > MAX_UBX_INF_MSG_LEN) {
                            memcpy(shm_tps_ptr->debug_gps_last_inf_message, &gpsCommandBuffer[6], MAX_UBX_INF_MSG_LEN);
                        } else {
                            memcpy(shm_tps_ptr->debug_gps_last_inf_message, &gpsCommandBuffer[6], UBXMsgBodyLen);
                        }
#if defined(ENABLE_WHEELTIX)
                    // UBX-ESF-MEAS
                    } else if (UBXMsgClassId == 0x10 && UBXMsgId == 0x02) {
                        receivedUBX_ESF_MEASURE(UBXMsgBodyLen);
                        shm_tps_ptr->debug_gps_cmd_count_ubx_esf_measure++;
                    } else if (UBXMsgClassId == 0x10 && UBXMsgId == 0x10) {
                        receivedUBX_ESF_STATUS(UBXMsgBodyLen);
                        ++shm_tps_ptr->debug_gps_cmd_count_ubx_esf_status;
#endif /* ENABLE_WHEELTIX */
                    } else if (UBXMsgClassId == 0x28 && UBXMsgId == 0x02) {
                        receivedUBX_HNR_INS();
                        ++shm_tps_ptr->debug_gps_cmd_count_ubx_hnr_ins;
                    // UBX-SEC-SIGN
                    } else if (UBXMsgClassId == 0x27 && UBXMsgId == 0x01) {
                        receivedUBX_SEC_SIGN();
                    }
#if defined(HEAVY_DUTY)
                    // UBX-CFG-GNSS message identifying UBlox's configuration via a string config put into RINV
                    else if (UBXMsgClassId == 0x06 && UBXMsgId == 0x3E)
                    {

#if defined(EXTRA_DEBUG)
                        //[6] = msgver
                        //[7] = numTrkChHw
                        //[8] = numTrkChUse
                        //[9] = NumConfigBlocks

                        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"msgVer=%u,umTrkChHw=%u,numTrkChUse=%u,NumConfigBlocks=%u:\n",gpsCommandBuffer[6]
                                                                      ,gpsCommandBuffer[7]
                                                                      ,gpsCommandBuffer[8]
                                                                      ,gpsCommandBuffer[9]);
                        //[10] = gnssId
                        //[11] = resTrkCh
                        //[12] = maxTrkCh
                        //[13] = res
                        //[14..17] = flags

                        for(i=0; (i<7) && (i <= gpsCommandBuffer[9]);i++){

                            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"gnssid=%u,resTrkCh=%u,maxTrkCh=%u,flags=[0x%x,0x%x,0x%x,0x%x]\n",gpsCommandBuffer[10 + (8 * i)]
                                                                        ,gpsCommandBuffer[11 + (8 * i)]
                                                                        ,gpsCommandBuffer[12 + (8 * i)]
                                                                        ,gpsCommandBuffer[14 + (8 * i)]
                                                                        ,gpsCommandBuffer[15 + (8 * i)]
                                                                        ,gpsCommandBuffer[16 + (8 * i)]
                                                                        ,gpsCommandBuffer[17 + (8 * i)]);
                        }
#endif
                    }

                    // UBX-MON-VER
                    else if(( 0x0A == UBXMsgClassId ) && (0x04 == UBXMsgId)) 
                    {
#if 0
                        int32_t j=0;
                    #if 0 /* raw */
                        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"[");

                        for(i=0;i<chars_in_buffer;i++){
                            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"0x%x,",gpsCommandBuffer[i]);
                        }
                        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"]\n");

                    #else /* pretty print */

                        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"\n[");

                        for(i=6,j=0;i<chars_in_buffer;i++){

                            if(0x0 == gpsCommandBuffer[i]) {

                                if(0==j) {
                                    j=1;
                                    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"\n");
                                } else {
                                    //I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"");
                                }

                            } else {
                                j=0;
                                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"%c\n",gpsCommandBuffer[i]);
                            }
                        }
                    #endif
#endif
                        receivedUBX_MON_VER(UBXMsgBodyLen);
                        ++shm_tps_ptr->debug_gps_cmd_count_ubx_mon_ver;
                    // Expected UBX-MON-HW
                    }

                    else if(( 0x0A == UBXMsgClassId ) && (0x09 == UBXMsgId))
                    {
#if defined(EXTRA_DEBUG)
                        if(1 == my_enable_hw_status )
                        {
                            //uint32_t pin_select  = 0x0; //6
                            //uint32_t pin_bank    = 0x0; //10
                            //uint32_t pin_dir     = 0x0; //14
                            //uint32_t pin_val     = 0x0; //18
                            uint16_t noisePerMS  = 0x0; //22
                            uint16_t agcCnt      = 0x0; //24
                            uint8_t  antennaStat = 0x0; //26
                            uint8_t  antennaPwr  = 0x0; //27
                            uint8_t  flags       = 0x0; //28
                            //uint8_t  reserved    = 0x0; //29
                            uint8_t  JamInd      = 0x0; //51

                            my_enable_hw_status  = 0;

                            noisePerMS   = (0xff & gpsCommandBuffer[22]); //22
                            noisePerMS  |= (0xff & gpsCommandBuffer[23]) << 8;

                            agcCnt   = (0xff & gpsCommandBuffer[24]); //24
                            agcCnt  |= (0xff & gpsCommandBuffer[25]) << 8;

                            antennaStat = (0xff & gpsCommandBuffer[26]); //26
                            antennaPwr  = (0xff & gpsCommandBuffer[27]); //27
                            flags       = (0xff & gpsCommandBuffer[28]); //28

                            JamInd      = (0xff & gpsCommandBuffer[51]); //51

                            //I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"received UBX-MON-HW:0x%02x-0x%02x, len=%u\n", UBXMsgClassId, UBXMsgId, UBXMsgTotalLen);
                            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"UBX-MON-HW: noisePerMS  = %u\n",noisePerMS);
                            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"UBX-MON-HW: agcCount    = %u\n",agcCnt);
                            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"UBX-MON-HW: antennaStat = 0x%x\n",antennaStat);
                            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"UBX-MON-HW: antennaPwr  = 0x%x\n",antennaPwr);
                            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"UBX-MON-HW: flags       = [\n");

                            if(0x1 & flags) {
                                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"RTC CAL OK|");
                            }else {
                                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"RTC CAL BAD|");
                            }
                            if(0x10 & flags) {
                                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"RTC XTAL ABSCENT|");
                            } else {
                                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"RTC XTAL PRESENT|");
                            }

                            flags = (0xC & flags) >> 2;
                            switch(flags) {
                                case 0 : I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"JAM N/A"); break;
                                case 1 : I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"JAM OK"); break;
                                case 2 : I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"JAMMING PRSENT FIX OK"); break;
                                case 3 : I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"JAMMED NO FIX"); break;
                            }
                            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"]\n");
                            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"UBX-MON-HW: JamInd CW(0=no,255=hi) = 0x%x\n",JamInd);
                        }
#endif
                    }
#endif /* HEAVY_DUTY */
                    else if (   (UBXMsgClassId == 0x01 && (UBXMsgId == 0x35 || UBXMsgId == 0x03))  // NAV-SAT and NAV-STATUS
                               || (UBXMsgClassId == 0x10 && UBXMsgId == 0x14)) // ESF-undoc'd
                    {
                        ++shm_tps_ptr->debug_gps_cmd_count_ubx_ignored;
                    }
                    //UBX-UPD-SOS (AssistNow response)
                    else if ((UBXMsgClassId == 0x09) && (UBXMsgId == 0x14))
                    {
                        receivedAssistNowBackupAck(UBXMsgBodyLen);
                        ++shm_tps_ptr->debug_gps_cmd_count_ubx_upd_sos;
                    } 
                    // UBX-NAV-AOPSTATUS
                    else if ((UBXMsgClassId == 0x01) && (UBXMsgId == 0x60 ))
                    {
                        receivedUBX_NAV_AOPSTATUS(UBXMsgBodyLen);
                        ++shm_tps_ptr->debug_gps_cmd_count_ubx_nav_aopstatus;
                    }
                    // Unexpected UBX message -- whoops!
                    else {
                        ++shm_tps_ptr->debug_gps_cmd_count_ubx_other;
#if defined(EXTRA_DEBUG)
                        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, " received UNHANDLED UBX msg 0x%02x-0x%02x, len=%u\n", UBXMsgClassId, UBXMsgId, UBXMsgTotalLen);
#endif
                    }
                }

                // Remove msg from buffer
                Remove_First_N_Bytes_From_GPS_Buffer(UBXMsgTotalLen);

                // And reset to await the next command
                msgMunchingStage = STAGE_0_AWAITING_START_OF_NEXT_COMMAND;
            }
            break;

    }   // End switch (msgMunchingStage)
}


/*-----------------------------------------------------------------------------
** @brief  GPS Comm  Active State Processing
** @param  unused
** @return unused
**---------------------------------------------------------------------------*/
static void gpsCommActive(void)
{
    int8_t  ch = 0;
    uint8_t cur_cycle_num_chars_read = 0;

    tcflush(gpsFd,TCIFLUSH);
#if defined(EXTRA_DEBUG)
    if (tpsDebugOutput & DBG_OUTPUT_TPSGEN) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"%s: gpsThread is now in gpsCommActive().\n",__func__);
    }
#endif
    while (tpsMainLoop == TRUE)
    {
#if defined(MY_UNIT_TEST)
        tpsMainLoop = WFALSE;
#else
        /* Gets the next char from the serial port.  Will block. */
        if (read(gpsFd, &ch, 1) == -1)
        {
            if (errno != EINTR && errno != EAGAIN)
            //if (errno == EBADF  || errno == EFAULT || errno == EINVAL)
            {
                tpsErrorReport(GPS_READ_ERROR_EXIT);
#if defined(EXTRA_DEBUG)
                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"GPS read failed with errno: %d\n", errno);
#endif
                shm_tps_ptr->error_states |= GPS_READ_ERROR;
                sleep(1); /* Don't error out. Notify RSUHEALTH. */
            }
            continue;
        }
#endif
        cur_cycle_num_chars_read = 1;
        shm_tps_ptr->debug_cnt_chars_read += cur_cycle_num_chars_read;

/* Extreme debug. */
#if defined(HEAVY_DUTY) && defined(DUMP_ALL_UBLOX_SERIAL)
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"%c",(char_t)ch); /* NMEA and UBX. A lot of data, be careful. */
#endif
        /* Process */
        Process_Incoming_GPS_Stream_Char(ch);

#ifdef DBG_ENABLE_SEND_TO_UBLOX

        // See if we have RINV yet.  If not, request it
        if (shm_tps_ptr->ublox_config_version_str[0] == 0) {
            if (time(0L) - last_RINV_requested_sec > 1) {
                // Request RINV
                last_RINV_requested_sec = time(0L);
                SendRinvRequest();
            }
        }

        // See if we have MONVER yet.  If not, request it
        if (shm_tps_ptr->ublox_firmware_version_str[0] == 0) {
            if (time(0L) - last_MONVER_requested_sec > 1) {
                // Request MON-VER
                last_MONVER_requested_sec = time(0L);
                SendMonVerRequest();
            }
        }

        // See if we sent uBlox the stored leapsecs yet.  If not, send it
        if (cfg_startup_leapsec_share && savedTpsVariables.curLeapSecs != 0
            && shm_tps_ptr->leap_secs_valid == 0)
        {
            if (time(0L) - last_MGAGPS_sent_sec > 1) {
                // Send another UBX-MGA-GPS message
                last_MGAGPS_sent_sec = time(0L);
                SendMgaGpsMessage((uint8_t) savedTpsVariables.curLeapSecs);
                ++shm_tps_ptr->debug_gps_cmd_count_ubx_mga_gps;
#if defined(EXTRA_DEBUG)
                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, " Sent UBX_MGS_GPS message # %u\n", shm_tps_ptr->debug_gps_cmd_count_ubx_mga_gps);
#endif
            }
        }

        // See if we are due to backup the AssistNow data. If so, request it
        if (startAssistNowBackup == TRUE)
        {
            // First poll AOPSTATUS to check if the subsystem is idle
            if (time(0L) - last_AOPSTATUS_requested_sec > 1) 
            {
                last_AOPSTATUS_requested_sec = time(0L);
                SendAOPStatusRequest();
            }
            // If the system is idle, do the backup. 
            if (assistNowConfirmedIdle == TRUE)
            {
                assistNowConfirmedIdle = FALSE;
                startAssistNowBackup = FALSE;
                SendAssistNowBackupRequest();
            }
        }

#endif /*DBG_ENABLE_SEND_TO_UBLOX*/
    }
#if defined(EXTRA_DEBUG)
    if (tpsDebugOutput & DBG_OUTPUT_TPSGEN) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"%s: gpsThread now leaving gpsCommActive().\n",__func__);
    }
#endif
}

#if defined(ENABLE_SR)
/*-----------------------------------------------------------------------------
** @brief  GPS Comm Active processing, but in SR Playback mode.
** @param  unused
** @return unused
**---------------------------------------------------------------------------*/
static void gpsCommPlaybackActive(void)
{
    int32_t           fd;
    RcvDataType       rcv_data;
    RecordHeaderType *recordHeader;
    uint8_t          *recordData;
    uint32_t          datalength;
    static uint8_t    srRecord[MAX_SR_RECORD_SIZE];
    int32_t           i;

    /* Create our playback receiving socket */
    fd = wsuCreateSockServer(TPSRAW_RECV_SR_PORT);
    if (fd < 0) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "TPSRAW: ERROR: wsuCreateSockServer failed for TPSRAW_RECV_SR_PORT\n");
        exit(1);
    }

    while (tpsMainLoop == TRUE)
    {
#if defined(MY_UNIT_TEST)
        tpsMainLoop = WFALSE;
#endif
        rcv_data.available = FALSE;
        rcv_data.fd = fd;
        rcv_data.data = srRecord;
        rcv_data.size = MAX_SR_RECORD_SIZE;

        if(wsuReceiveData(100000, &rcv_data) < 0)
        {
            close(fd);
            fd = wsuCreateSockServer(TPSRAW_RECV_SR_PORT);
            usleep(5000);
            continue;
        }

        ++shm_tps_ptr->debug_cnt_sr_raw_recvs;

        // Send the raw msg to be processed
        if (rcv_data.available == TRUE)
        {
            recordHeader = (RecordHeaderType*)srRecord;
            datalength = recordHeader->totalLength - sizeof(RecordHeaderType);
            if(datalength > 0)
            {
                recordData = srRecord + sizeof(RecordHeaderType);

                shm_tps_ptr->debug_cnt_chars_read += datalength;

                if (tpsDebugOutput & DBG_OUTPUT_SR) {
                    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"%s:TPSRAW: Received RAW playback msg len %d w start bytes: 0x%02x 0x%02x"
                            "0x%02x ..\n",__func__, datalength, recordData[0], recordData[1], recordData[2]);
                }

                for (i=0; i<datalength; i++) {
                    Process_Incoming_GPS_Stream_Char(recordData[i]);
                }
            }
        }
    }

    // Close up
    close(fd);
}
#endif

/****************************************/
/* GPS Comm Message Received Processing */
/*                                      */
/*     NMEA parsed      Receiver type   */
/*   ===============    =============   */
/*   RMC,GGA,GST,GSA    OEMV            */
/*   RMC,GGA,GST,GSA    Topcon          */
/*   RMC,GGA,GSA        18lv            */
/*   RMC,GGA,GST,GSA    ublox           */
/****************************************/
/*-----------------------------------------------------------------------------
** @brief  Detect/Process received NMEA text message (GP, GL, and GN) versions of  RMC, GGA, GSA, & GST.
** @param  unused
** @return unused
**---------------------------------------------------------------------------*/
static void gpsCommMessageReceived(void)
{
    char_t * x0d;

    /* Determine which NMEA message was received and then call the   */
    /* appropriate parse function. Since we're combining fields from */
    /* the multiple messages, wait until required messages come in.  */

    /* Char [6] is always a comma, lets test it here */
    if (gpsCommandBuffer[6] != ',') {
#if defined(EXTRA_DEBUG)
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME," Warning: gpsCommMessageReceived says buff[6] != ',' !!\n");
#endif
        return;
    }

    if (strncmp((char *)gpsCommandBuffer, GPRMC, sizeof(GPRMC)) == 0 ||
        strncmp((char *)gpsCommandBuffer, GLRMC, sizeof(GLRMC)) == 0 ||
        strncmp((char *)gpsCommandBuffer, GNRMC, sizeof(GNRMC)) == 0)
    {
        gpsCommParseGprmc();
    }
    else
    if (strncmp((char *)gpsCommandBuffer, GPGGA, sizeof(GPGGA)) == 0 ||
        strncmp((char *)gpsCommandBuffer, GLGGA, sizeof(GLGGA)) == 0 ||
        strncmp((char *)gpsCommandBuffer, GNGGA, sizeof(GNGGA)) == 0)
    {
        gpsCommParseGpgga();
    }
    else
    if (strncmp((char *)gpsCommandBuffer, GPGSA, sizeof(GPGSA)) == 0 ||
        strncmp((char *)gpsCommandBuffer, GLGSA, sizeof(GLGSA)) == 0 ||
        strncmp((char *)gpsCommandBuffer, GNGSA, sizeof(GNGSA)) == 0)
    {
        gpsCommParseGpgsa();
    }
    else
    if (strncmp((char *)gpsCommandBuffer, GPGST, sizeof(GPGST)) == 0 ||
        strncmp((char *)gpsCommandBuffer, GLGST, sizeof(GLGST)) == 0 ||
        strncmp((char *)gpsCommandBuffer, GNGST, sizeof(GNGST)) == 0)
    {
        gpsCommParseGpgst();
    }
    // GPTXT message
    if (strncmp((char *)gpsCommandBuffer, "$GPTXT", 6) == 0) {
        // Truncate the string for printing
        x0d = index((char_t *) gpsCommandBuffer, 0x0d);
        if (x0d) *x0d = 0;
        x0d = index((char_t *) gpsCommandBuffer, 0x0a);
        if (x0d) *x0d = 0;
#if defined(EXTRA_DEBUG)
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME," Received GPTXT msg: %s\n", (char_t *) gpsCommandBuffer);
#endif
    }
#if defined(ENABLE_NMEA_SOLUTION) /* Use UBX PVT or NMEA GGA, but not both. */
    if (gpsReceivedRMC && gpsReceivedGGA)
    {
        ++shm_tps_ptr->debug_cnt_comm_send_tps_data_ind;
        gpsCommSendTpsDataIndFromNmeaData();
    }
#endif
}


/*-----------------------------------------------------------------------------
** @brief  Parse/regularize time string in NMEA text message.
** Note: If the NMEA UTC time string format is "215319" or "215319.0", then
**       convert time string in to "215319.00".
** @param  unused
** @return unused
**---------------------------------------------------------------------------*/
static bool_t parse_nmea_time_str(char_t *src_timestr, char_t *dst_timestr)
{
    int32_t i = 0;
    bool_t found_dot = FALSE;

    for(i = 0; i < NMEA_MSG_TIME_STRNG_LENGTH; i++)
    {
        if (src_timestr[i] == '\0')
        {
            if (found_dot == TRUE)
            {
                if (dst_timestr[i - 2] == '.')
                {
                    dst_timestr[i] = '0';
                    i++;
                }/* else
                {
                    return FALSE;
                }*/
            }
            dst_timestr[i] = '\0';
            break;
        }

        if (src_timestr[i] == '.')
        {
            if (i > (NMEA_MSG_TIME_STRNG_LENGTH - 4))
            {
                return FALSE;
            }
            else
            {
                found_dot = TRUE;
            }
        }

        dst_timestr[i] = src_timestr[i];
    }

    if ((dst_timestr[0] != '\0' ) && (found_dot == FALSE) && (i <= (NMEA_MSG_TIME_STRNG_LENGTH - 4)))
    {
        dst_timestr[i++]='.';
        dst_timestr[i++]='0';
        dst_timestr[i++]='0';
        dst_timestr[i]='\0';
        found_dot = TRUE;
    }
    return found_dot;
}


/*-----------------------------------------------------------------------------
** @brief  Process received NMEA RMC message.
** @param  unused
** @return unused
**---------------------------------------------------------------------------*/
#define GPRMC_UTC_TIME_IDX 1
#define GPRMC_DATA_STATUS_IDX 2
#define GPRMC_LAT_IDX 3
#define GPRMC_LATNS_IDX 4
#define GPRMC_LONG_IDX 5
#define GPRMC_LONGEW_IDX 6
#define GPRMC_GND_SPEED_IDX 7
#define GPRMC_COURSE_IDX 8
#define GPRMC_DATE_IDX 9

static void gpsCommParseGprmc(void)
{
    int32_t  gpsGrptime,deci;
    char_t   tokens[NMEA_MSG_MAX_TOKENS+1][NMEA_MSG_MAX_TOKEN_LENGTH+1];

    memset(tokens, 0, sizeof(tokens));
    memset(&gpsRMC, 0, sizeof(gpsRMC));

    if( !gpsValidateNMEAChecksum(gpsCommandBuffer, strlen((char_t *)gpsCommandBuffer)) )
    {
        tpsErrorReport(GPS_NMEA_CKSUM_ERROR);
        gpsReceivedRMC = FALSE;
        ++shm_tps_ptr->debug_gps_cmd_count_nmea_bad_csum;
        return;
    }

    gpsRMC.debug_recv_cpu_cycles = ClockCycles();
    gpsRMC.debug_recv_time_ms = get_cur_msec_time();

    parse_gpsCmd_into_tokens( gpsCommandBuffer, tokens, delim );

    gpsRMC.dataStatus = tokens[GPRMC_DATA_STATUS_IDX][0];

    // Mark RMC as invalid and silently return.  GPS simply lost our position;
    //    and this doesnt warrent an error msg sent to SDHGPS
    if (gpsRMC.dataStatus == 'V') {
       gpsReceivedRMC = FALSE;
       return;
    }

    if (gpsRMC.dataStatus != 'A')
        goto parse_error;

    if (parse_nmea_time_str(tokens[GPRMC_UTC_TIME_IDX],&gpsRMC.time[0]) != TRUE)
        goto parse_error;

    if (!sscanf(tokens[GPRMC_LAT_IDX], "%lf",&gpsRMC.latitude))
        goto parse_error;

    gpsRMC.latNS = tokens[GPRMC_LATNS_IDX][0];

    if (!sscanf(tokens[GPRMC_LONG_IDX], "%lf",&gpsRMC.longitude))
        goto parse_error;

    gpsRMC.longEW = tokens[GPRMC_LONGEW_IDX][0];

    sscanf(tokens[GPRMC_GND_SPEED_IDX], "%lf",&gpsRMC.groundspeed);

    sscanf(tokens[GPRMC_COURSE_IDX], "%lf",&gpsRMC.course);

    tps_strlcpy(&gpsRMC.date[0], (char *)tokens[GPRMC_DATE_IDX], NMEA_MSG_DATE_STRNG_LENGTH);

    gpsReceivedRMC = TRUE;

    sscanf(&gpsRMC.time[0],"%d.%d",&gpsGrptime,&deci);

    if (gpsReceivedGGA)
    {
        if ((gpsGrptime==gpsGrpReftime) && (deci==gpsGrpRef_deci))
        {
            isValidGroup=TRUE;
        }
        else
        {
            gpsReceivedGGA=FALSE;
            gpsGrpReftime=gpsGrptime;
            gpsGrpRef_deci=deci;
        }
    }
    else
    {
        gpsGrpReftime=gpsGrptime;
        gpsGrpRef_deci=deci;
    }

    ++shm_tps_ptr->debug_gps_cmd_count_nmea_gprmc_valid_data;

    return;

parse_error:
        gpsReceivedRMC = FALSE;
        tpsErrorReport(GPS_NMEA_MSG_PARSE_ERROR);
#if defined(EXTRA_DEBUG)
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"%s:  GPS_NMEA_MSG_PARSE_ERROR \n",__func__);
#endif
        return;

}


/*-----------------------------------------------------------------------------
** @brief  Process received NMEA GST message.
** @param  unused
** @return unused
**---------------------------------------------------------------------------*/
#define GPGST_SD_SMJR_AXIS_EELLIPSE_IDX 3
#define GPGST_SD_SMNR_AXIS_EELLIPSE_IDX 4
#define GPGST_ORTN_SMJR_AXIS_EELLIPSE_IDX 5
#define GPGST_LAT_ERR_IDX 6
#define GPGST_LON_ERR_IDX 7
#define GPGST_ALT_ERR_IDX 8

static void gpsCommParseGpgst(void)
{
    char_t tokens[NMEA_MSG_MAX_TOKENS+1][NMEA_MSG_MAX_TOKEN_LENGTH+1];

    memset(tokens, 0, sizeof(tokens));
    memset(&gpsGPST, 0, sizeof(gpsGPST));

    if( !gpsValidateNMEAChecksum(gpsCommandBuffer, strlen((char_t *)gpsCommandBuffer)) )
    {
        tpsErrorReport(GPS_NMEA_CKSUM_ERROR);
        gpsReceivedGPGST = FALSE;
        ++shm_tps_ptr->debug_gps_cmd_count_nmea_bad_csum;
        return;
    }

    parse_gpsCmd_into_tokens( gpsCommandBuffer, tokens, delim );

    if (!sscanf(tokens[GPGST_SD_SMJR_AXIS_EELLIPSE_IDX], "%lf",&gpsGPST.smjr_err))
        goto parse_error;

    if (!sscanf(tokens[GPGST_SD_SMNR_AXIS_EELLIPSE_IDX], "%lf",&gpsGPST.smnr_err))
        goto parse_error;

    if (!sscanf(tokens[GPGST_ORTN_SMJR_AXIS_EELLIPSE_IDX], "%lf",&gpsGPST.orient))
        goto parse_error;

    if (!sscanf(tokens[GPGST_LAT_ERR_IDX], "%lf",&gpsGPST.lat_err))
        goto parse_error;

    if(!sscanf(tokens[GPGST_LON_ERR_IDX], "%lf",&gpsGPST.lon_err))
        goto parse_error;

    sscanf(tokens[GPGST_ALT_ERR_IDX], "%lf",&gpsGPST.alt_err);

    gpsReceivedGPGST = TRUE;
    ++shm_tps_ptr->debug_gps_cmd_count_nmea_gpgst_valid_data;
    // Evaluate the ellipse error to see if we need to fudge it.
    // Older UBlox firmware didn't give smjr smnr err, so we just make a vector out of lat_err & lon_err.
    // This was previously done inside gpsCommSendTpsDataIndFromNmeaData() but
    //   now NMEA-GST is used in combination with UBX data so we evaluate ellipse upon arrival.
    // There is no issue of forgoing good UBX-NAV-EELL data for estimated NMEA-GPGST data because
    //    the old ublox firmware that gave empty ellipse error data didn't have UBX-NAV-EELL capability.
    if (gpsGPST.smjr_err != 0) {
        ++shm_tps_ptr->debug_gps_cmd_count_nmea_gpgst_w_valid_ellipse_err;
    } else {
        gpsGPST.smjr_err = sqrt(gpsGPST.lat_err * gpsGPST.lat_err + gpsGPST.lon_err * gpsGPST.lon_err);
        gpsGPST.smnr_err = gpsData.smjr_err;
        gpsGPST.orient = 0;
        ++shm_tps_ptr->debug_gps_cmd_count_nmea_gpgst_w_estim_ellipse_err;
    }
    return;

parse_error:
    //I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"GST parse error: %s\n",gpsCommandBuffer);
    gpsReceivedGPGST = FALSE;
    return;

}



/*-----------------------------------------------------------------------------
** @brief  Parse NMEA Message into tokens.
** @param  unused
** @return unused
**---------------------------------------------------------------------------*/
static int32_t parse_gpsCmd_into_tokens( const uint8_t *str, char_t tokentab[][NMEA_MSG_MAX_TOKEN_LENGTH+1], const char_t *delimiters )
{
    int32_t i=0, n_itms=0, t=0;

    // validation
    if ( !tokentab ) {
        return 0;
    }

    if ( !str ) {
        return 0;
    }
    if ( !delimiters ) {
        return 0;
    }

    t = 0;
    for ( i=0; i<strlen((char *)str); i++ ) {

        if (str[i] == '*' || (n_itms >= NMEA_MSG_MAX_TOKENS)) {
            tokentab[n_itms][t]='\0';
            break;
        }

        if (str[i] == *delimiters )  {
           tokentab[n_itms][t]='\0';
           n_itms++;
           t = 0;
        } else if(t < NMEA_MSG_MAX_TOKEN_LENGTH) {
           tokentab[n_itms][t] = str[i];
           t++;
        }
    }
    return( n_itms + 1 );
}


/*-----------------------------------------------------------------------------
** @brief  Process received NMEA GSA message.
** @param  unused
** @return unused
**---------------------------------------------------------------------------*/
#define GPGSA_FIXMODE_IDX  2
#define GPGSA_PDOP_IDX  15
#define GPGSA_HDOP_IDX  16
#define GPGSA_VDOP_IDX  17

// Parses GPS command string in global gpsCommandBuffer into global gpsGPGSA
static void gpsCommParseGpgsa(void)
{
    char_t tokens[NMEA_MSG_MAX_TOKENS+1][NMEA_MSG_MAX_TOKEN_LENGTH+1];

    memset(tokens, 0, sizeof(tokens));
    memset(&gpsGPGSA, 0, sizeof(gpsGPGSA));

    if( !gpsValidateNMEAChecksum(gpsCommandBuffer, strlen((char_t *)gpsCommandBuffer)) )
    {
        tpsErrorReport(GPS_NMEA_CKSUM_ERROR);
        gpsReceivedGPGSA = FALSE;
        ++shm_tps_ptr->debug_gps_cmd_count_nmea_bad_csum;
        return;
    }

    parse_gpsCmd_into_tokens( gpsCommandBuffer, tokens, delim );

    if (!sscanf(tokens[GPGSA_FIXMODE_IDX], "%d",&gpsGPGSA.fix_mode))
        goto parse_error;

    if (!sscanf(tokens[GPGSA_PDOP_IDX], "%lf",&gpsGPGSA.pdop))
        goto parse_error;

    if (!sscanf(tokens[GPGSA_HDOP_IDX], "%lf",&gpsGPGSA.hdop))
        goto parse_error;

    sscanf(tokens[GPGSA_VDOP_IDX], "%lf",&gpsGPGSA.vdop);

    gpsReceivedGPGSA = TRUE;
    ++shm_tps_ptr->debug_gps_cmd_count_nmea_gpgsa_valid_data;
    return;

parse_error:
    //I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"GSA parse error: %s\n",gpsCommandBuffer);
    gpsReceivedGPGSA = FALSE;
    return;
}


/*-----------------------------------------------------------------------------
** @brief  Process received NMEA GGA message.
** @param  unused
** @return unused
**---------------------------------------------------------------------------*/
#define GPGGA_UTC_TIME_IDX  1
#define GPGGA_LAT_IDX 2
#define GPGGA_LATNS_IDX 3
#define GPGGA_LONG_IDX 4
#define GPGGA_LONGEW_IDX 5
#define GPGGA_FIX_QTY_IDX 6
#define GPGGA_NUM_STATS_IDX 7
#define GPGGA_HDOP_IDX 8
#define GPGGA_ALT_IDX 9
#define GPGGA_ALT_UNITS_IDX 10
#define GPGGA_HOGAE_IDX 11
#define GPGGA_HOGAE_UNITS_IDX 12
#define GPGGA_DIFFAGE_IDX 13

/* GPS Comm Parse GGA Message */
static void gpsCommParseGpgga(void)
{
#if defined(HEAVY_DUTY) /* For rsuGnssOutputString only. */
    if( !gpsValidateNMEAChecksum(gpsCommandBuffer, strlen((char_t *)gpsCommandBuffer)) ){
        tpsErrorReport(GPS_NMEA_CKSUM_ERROR);
        ++shm_tps_ptr->debug_gps_cmd_count_nmea_bad_csum;
    } else {
        ++shm_tps_ptr->debug_gps_cmd_count_nmea_gpgga_valid_data;
    }
    // Copy GGA into SHM good or bad crc.
    if (shm_tps_ptr->pub_geodetic.last_updated_gga_index == 0) {
        memcpy(&shm_tps_ptr->pub_geodetic.last_GGA[1], &gpsCommandBuffer[0], MAX_NMEA_MESSASE_LEN);
        shm_tps_ptr->pub_geodetic.last_GGA[1][MAX_NMEA_MESSASE_LEN-1] = '\0'; /* Do everyone a favor. */
        shm_tps_ptr->pub_geodetic.last_updated_gga_index = 1;
    } else {
        memcpy(&shm_tps_ptr->pub_geodetic.last_GGA[0], &gpsCommandBuffer[0], MAX_NMEA_MESSASE_LEN);
        shm_tps_ptr->pub_geodetic.last_GGA[0][MAX_NMEA_MESSASE_LEN-1] = '\0'; /* Do everyone a favor. */
        shm_tps_ptr->pub_geodetic.last_updated_gga_index = 0;
    }
    memset(gpsCommandBuffer,'\0',sizeof(gpsCommandBuffer)); /* Done with it. */
#else
    int32_t  deci,gpsGrptime;
    char_t   tokens[NMEA_MSG_MAX_TOKENS+1][NMEA_MSG_MAX_TOKEN_LENGTH+1];

    memset(tokens, 0, sizeof(tokens));
    memset(&gpsGGA, 0, sizeof(gpsGGA));
    if( !gpsValidateNMEAChecksum(gpsCommandBuffer, strlen((char_t *)gpsCommandBuffer)) )
    {
        tpsErrorReport(GPS_NMEA_CKSUM_ERROR);
        gpsReceivedGGA = FALSE;
        ++shm_tps_ptr->debug_gps_cmd_count_nmea_bad_csum;
        return;
    }

    gpsGGA.debug_recv_cpu_cycles = ClockCycles();   // For measuring when TPS received this from UBLOX
    gpsGGA.debug_recv_time_ms = get_cur_msec_time();// For measuring when TPS received this from UBLOX

    parse_gpsCmd_into_tokens( gpsCommandBuffer, tokens, delim );

    if (!sscanf(tokens[GPGGA_FIX_QTY_IDX], "%d",&gpsGGA.fixQuality))
        goto parse_error;

    if(gpsGGA.fixQuality == 0)
        goto parse_error;

    if (parse_nmea_time_str(tokens[GPGGA_UTC_TIME_IDX],&gpsGGA.time[0]) != TRUE)
        goto parse_error;

    if (!sscanf(tokens[GPGGA_LAT_IDX], "%lf",&gpsGGA.latitude))
        goto parse_error;

    gpsGGA.latNS = tokens[GPGGA_LATNS_IDX][0];

    if (!sscanf(tokens[GPGGA_LONG_IDX], "%lf",&gpsGGA.longitude))
        goto parse_error;

    gpsGGA.longEW = tokens[GPGGA_LONGEW_IDX][0];

    sscanf(tokens[GPGGA_NUM_STATS_IDX], "%d",&gpsGGA.numSats);

    sscanf(tokens[GPGGA_HDOP_IDX], "%lf",&gpsGGA.hDop);

    sscanf(tokens[GPGGA_ALT_IDX], "%lf",&gpsGGA.altitude);

    gpsGGA.altUnits = tokens[GPGGA_ALT_UNITS_IDX][0];

    sscanf(tokens[GPGGA_HOGAE_IDX], "%lf",&gpsGGA.hogae);

    gpsGGA.hogaeUnits = tokens[GPGGA_HOGAE_UNITS_IDX][0];

    sscanf(tokens[GPGGA_DIFFAGE_IDX], "%d",&gpsGGA.diffAge);

    gpsReceivedGGA = TRUE;
    ++shm_tps_ptr->debug_gps_cmd_count_nmea_gpgga_valid_data;

    sscanf(&gpsGGA.time[0],"%d.%d",&gpsGrptime,&deci);

    if (gpsReceivedRMC)
    {
        if ((gpsGrptime==gpsGrpReftime) && (deci==gpsGrpRef_deci))
        {
            isValidGroup=TRUE;
        }
        else
        {
            gpsReceivedRMC=FALSE;
            gpsGrpReftime=gpsGrptime;
            gpsGrpRef_deci=deci;
        }
    }
    else
    {
        gpsGrpReftime=gpsGrptime;
        gpsGrpRef_deci=deci;
    }
parse_error:
#if defined(EXTRA_DEBUG)
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"\nGGA parse err: %s\n",gpsCommandBuffer);
#endif
    gpsReceivedGGA = FALSE;
#endif /* HEAVY_DUTY */
    return;
}



// ================================================================================
// == END OF PARSING===============================================================
// ================================================================================



/*-----------------------------------------------------------------------------
** @brief  Compute Checksume using 8-bit Fletcher Algorithm (256 Modulus).
** Note: sample code from PDF from u-blox document (See section 30.4 UBX Checksum)
**       Location:  https://www.u-blox.com/en/product/
**       Document:  "neo-m8l - download "u-blox 8 / u-blox M8 Receiver Description Including Protocol Specification"
** @param in  buffer - pointer to buffer for which to compute checksum
** @param in  buflen - length in bytes of buffer
** @param out csum1  - Resulting 8-bit Fletcher checksum
** @return unused
**---------------------------------------------------------------------------*/
void Compute8BitFletcherChecksums(uint8_t *buffer, uint16_t bufflen, uint8_t *csum1)  //, uint8_t * csum2)
{
    uint16_t i;
    uint8_t  sum1, sum2;

    sum1 = sum2 = 0;
    for (i=0; i<bufflen; i++)
    {
        sum1 += buffer[i];
        sum2 += sum1;
    }
    csum1[0] = sum1;
    csum1[1] = sum2;
}

#ifdef DBG_ENABLE_SEND_TO_UBLOX

/*-----------------------------------------------------------------------------
** @brief  Send UBX binary message to u-blox GNSS.
** @param  msg    - pointer to message to send to u-blox GNSS
** @param  msglen - length in bytes of msg.
** @return status - integer < 0 if error occured.
**---------------------------------------------------------------------------*/
int32_t SendMsgToUblox(uint8_t *msg, uint16_t msglen)
{
    int32_t rc;
#if defined(EXTRA_DEBUG)
    int32_t i;
#endif
    uint8_t outbuf[MAX_GPS_MESSAGE_LEN];

#if defined(HEAVY_DUTY)
    //clear it
    memset(outbuf,0x0,MAX_GPS_MESSAGE_LEN);

    //limit size just in case
    if( (MAX_GPS_MESSAGE_LEN -1) <= msglen) {
        msglen = MAX_GPS_MESSAGE_LEN -1;
    }
#endif

    // Copy into outbuf
    memcpy(outbuf, msg, msglen);

    // Fill in the checksums
    Compute8BitFletcherChecksums(&outbuf[2], msglen - 4, &outbuf[msglen - 2]);

    rc = write(gpsFd, outbuf, msglen);
    if (rc < 0) {
        if (errno != EINTR && errno != EAGAIN)
        {
#if defined(EXTRA_DEBUG)
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"GPS write failed with errno: %d\n", rc);
#endif
        }
    }
#if defined(EXTRA_DEBUG)
    else {
        if (tpsDebugOutput & DBG_OUTPUT_TPSGEN) {
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"%s:TPS sent to GPS:",__func__);
            for (i=0; i<msglen; i++) {
                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME," %02x", outbuf[i]);
            }
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"\n");
        }
    }
#endif
    return rc;
}

#endif /*DBG_ENABLE_SEND_TO_UBLOX*/


////TODO: Heed comment below and simplify  type conversion functions.
//
// Convert a 4-byte UBX I4 type to a native signed integer
//   Note this is not portable; takes advantage of CPU be little endian too.
//   Could also convertion with just casting... probably should have
//
/*-----------------------------------------------------------------------------
** @brief  Convert a 4-byte UBX I4 type to a native signed integer.
** NOTE:   This is not portable; takes advantage of CPU be little endian too.
**         Could also convert with just casting... probably should have
** @param  4-byte UBX I4 type
** @return value converted to int32_t
**---------------------------------------------------------------------------*/
int32_t I4Convert(uint8_t *buff)
{
#ifdef LITTLE_ENDIAN
    return *((int32_t *) buff);
#else
    union
    {
        int32_t integer;
        uint8_t byte[4];
    } foo;
    foo.byte[3] = buff[0];
    foo.byte[2] = buff[1];
    foo.byte[1] = buff[2];
    foo.byte[0] = buff[3];
    return foo.integer;
#endif
}

/*-----------------------------------------------------------------------------
** @brief  Convert a 4-byte UBX U4 type to a native unsigned integer.
** @param  4-byte UBX U4 type
** @return value converted to uint32_t
**---------------------------------------------------------------------------*/
uint32_t U4Convert(uint8_t *buff)
{
#ifdef LITTLE_ENDIAN
    return *((uint32_t *) buff);
#else
    union
    {
        uint32_t integer;
        uint8_t  byte[4];
    } foo;
    foo.byte[3] = buff[0];
    foo.byte[2] = buff[1];
    foo.byte[1] = buff[2];
    foo.byte[0] = buff[3];
    return foo.integer;
#endif
}

// Convert a 3-byte UBX I3 type to a native signed integer.  This is an odd
//   case from the ESF-RAW message which has a 3-byte signed integer.  We
//   are passing all 4-bytes of the UBX X4 so we can #ifdef endian in here
//   and contain having to figure out which 3 of the 4 bytes to pass in
//   an #ifdef out where we're being called.  Note the GPS data is always
//   little-endian so we're always going to use the first 3 bytes.
/*-----------------------------------------------------------------------------
** @brief  Convert a 3-byte UBX I3 type to a native signed integer.
** NOTE:   Conversion of 3-byte integer from UBX-ESF-RAW message (1st 3 bytes of input)
** @param  4-byte buffer containing 3-byte UBX I3 type
** @return value converted to int32_t
**---------------------------------------------------------------------------*/
int32_t I3Convert(uint8_t *buff)
{
    union
    {
        int32_t integer;
        uint8_t byte[4];
    } foo;
#ifdef LITTLE_ENDIAN
    foo.byte[0] = buff[0];
    foo.byte[1] = buff[1];
    foo.byte[2] = buff[2];
    if (buff[2] & 0x80) {       // Extend the neg bit if needed
        foo.byte[3] = 0xFF;
    } else {
        foo.byte[3] = 0;
    }
#else
    foo.byte[3] = buff[0];
    foo.byte[2] = buff[1];
    foo.byte[1] = buff[2];
    if (buff[2] & 0x80) {       // Extend the neg bit if needed
        foo.byte[0] = 0xFF;
    } else {
        foo.byte[0] = 0;
    }
#endif
    return foo.integer;
}


/*-----------------------------------------------------------------------------
** @brief  Convert a 2-byte UBX U2 type to a native unsigned integer.
** @param  2-byte UBX U2 type
** @return value converted to uint16_t
**---------------------------------------------------------------------------*/
uint16_t U2Convert(uint8_t *buff)
{
#ifdef LITTLE_ENDIAN
    return *((uint16_t *) buff);
#else
    union
    {
        uint16_t uint16;
        uint8_t  byte[2];
    } foo;
    foo.byte[1] = buff[0];
    foo.byte[0] = buff[1];
    return foo.uint16;
#endif
}


#ifdef DBG_SYSTEMTIMEUPDATE

#ifdef GPS_UPDATE_LINUX_SYSTIME

/*-----------------------------------------------------------------------------
** @brief  Determine if latency between system time and pps pulse exceed tolerance.
** @param  in  timeval -pointer to PPS time
** @param  in  timeval -pointer to system time
** @return TRUE if pps/system time latency within tolerance, otherwise FALSE.
**---------------------------------------------------------------------------*/
static bool_t checkTimeElapsedSinceLastPPS(struct timeval *pps, struct timeval *tps_tv)
{
    struct timeval  sys_tv;

    sys_tv.tv_usec =0;
    sys_tv.tv_sec = 0;
    gettimeofday(&sys_tv,NULL);

        if ((pps->tv_sec != sys_tv.tv_sec))
        {
            if (((pps->tv_sec + 1) == sys_tv.tv_sec)&& (sys_tv.tv_usec < pps->tv_usec))
            {
                tps_tv->tv_usec = (1000000 - pps->tv_usec) + sys_tv.tv_usec +1000;
                return TRUE;
            }
            else
            {
#if defined(EXTRA_DEBUG)
                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Latency exceeded:PPS:%lu.%lu and Sys time:%lu.%lu\n",
                       (uint64_t)pps->tv_sec,  (uint64_t)pps->tv_usec,
                       (uint64_t)sys_tv.tv_sec, (uint64_t)sys_tv.tv_usec);
#endif
                shm_tps_ptr->debug_cnt_1pps_latency_event++;
                return FALSE;
            }
        }
        else
        {
            tps_tv->tv_usec = sys_tv.tv_usec - pps->tv_usec + 1000;
            return TRUE;
        }
}



/*-----------------------------------------------------------------------------
** @brief  Calculate delta time between current PPS time, and previous call to function.
** @param  in  timeval - pointer to latest PPS time
** @param  out timeval - pointer to delta time between latest and previous PPS time.
**TODO: @param  out prev    - pointer to previous PPS time.  (currently modifies global VAR)
** @return  unused
**---------------------------------------------------------------------------*/
////TODO: pass in prev, shouldn't be global
static void calculateDeltaTime(struct timeval *pps, struct timeval *cur_delta)
{
    // Check whether the NewTimeStamp updated in shared memory

    // NOTE: Math between two time_t's has to be stored in a signed type,
    //       C defaults to time_t which can't be negative and all < -1 tests incorrectly pass !!
    //    - AHAH: " version 6 of the QNX operating system has an unsigned 32-bit time_t "
    //         (From Wikipedia: ht//en.wikipedia.org/wiki/Unix_time )
    //    - Test to show bug: { time_t x = 1; if (x < -1) { I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Bad, why is 1 < -1 ?!?!\n"); }  }
    //    - orig code:  if (((pps->tv_sec - prev.tv_sec) > 1) || ((pps->tv_sec - prev.tv_sec) < -1))

    int64_t tv_sec_delta = pps->tv_sec - prev.tv_sec;
    if (tv_sec_delta > 1 || tv_sec_delta < -1)
    {
#if defined(EXTRA_DEBUG)
        if (tpsDebugOutput & DBG_OUTPUT_ADJTIME) {
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME," calcDeltaTime RESET isrLatenChk to zero and prev to zero! "
                   "[tv_sec_delta=%ld]\n", tv_sec_delta);
        }
#endif
        prev.tv_sec=0;
        prev.tv_usec=0;
        isrLatencyCheck=0;
    }

    // TODO check to see current time is always greater than prev time.
    timeval_sub(cur_delta,&prev);

    // Here decide whether the drift is less than a second or more than second
    if (cur_delta->tv_sec==0)
    {
        cur_delta->tv_usec = -(1000000-cur_delta->tv_usec);
    }
#if defined(EXTRA_DEBUG)
    // I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Current Delta sec: %ld usec:%ld\n",cur_delta->tv_sec,cur_delta->tv_usec);
    if (tpsDebugOutput & DBG_OUTPUT_ADJTIME) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME," calcDeltaTime says %ld.%06ld - %ld.%06ld = curDelta = %ld.%06ld\n",
                (int64_t) pps->tv_sec,
                (int64_t) pps->tv_usec,
                (int64_t) prev.tv_sec,
                (int64_t) prev.tv_usec,
                (int64_t) cur_delta->tv_sec,
                (int64_t) cur_delta->tv_usec);
    }
#endif
}


/*-----------------------------------------------------------------------------
** @brief  Set system clock and RTC (realtime clock) based on GNSS/PPS.
** @param  in  prev_pps_used (only used in debug print)
** @param  in  gps_time (only used in debug print)
** @param  in  tps_tv - Time derived from GNSS/PPS.
** @param  in  gps_tm  (only used in debug print)
** @return unused
**---------------------------------------------------------------------------*/
static void tpsSetTimeOfDay(int32_t prev_pps_used,time_t gps_time, struct timeval *tps_tv, struct tm  *gps_tm)
{
    int ret = 0;
#if defined(EXTRA_DEBUG)
    if (tpsDebugOutput & DBG_OUTPUT_ADJTIME)
    {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"%s:----------------->Inside tpsSetTimeOfDay()!\n",__func__);
    }
#endif
    // 1. Set system clock
    if ((settimeofday(tps_tv,NULL))==-1)
    {
#if defined(EXTRA_DEBUG)
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"%s:settimeofday() failed: %s\n", __func__,strerror(errno));
#endif
        ++shm_tps_ptr->debug_cnt_time_adjustment_fail_settimeofday;
        shm_tps_ptr->cur_tps_time_state = TTS_NO_SYNC;  // MAYBE
        ret =-1;
    }
    else
    {
        TimeSetCount++;
        ++shm_tps_ptr->debug_cnt_set_time_with_settimeofday;
        linuxSetSystemTimeOnce = 0;
        debug_print(gps_time, tps_tv->tv_sec, tps_tv->tv_usec, prev_pps_used, 0, 0, 0,
                    TimeAdjCount, FreqAdjCount, TimeSetCount);

        // Set TPS timesync state
        if (shm_tps_ptr->leap_secs_valid) {
            shm_tps_ptr->cur_tps_time_state = TTS_GPS_SYNC;
        } else {
            shm_tps_ptr->cur_tps_time_state = TTS_NOLEAP_SYNC;
        }

        /* Adjust to user's desired microseconds */
        struct timespec ts2;

        ts2.tv_sec = tps_tv->tv_sec;
        ts2.tv_nsec = 1000 * tps_tv->tv_usec;

        // 2. Set CLOCK_REALTIME clock
        if (clock_settime(CLOCK_REALTIME, &ts2) < 0)
        {
#if defined(EXTRA_DEBUG)
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"%s:clock_settime() failed: %s\n", __func__,strerror(errno));
#endif
            ++shm_tps_ptr->debug_cnt_time_adjustment_fail_clocksettime;
            ret = -1;
        }
        else
        {
            // Track clock_settime worked
            ++shm_tps_ptr->debug_cnt_set_time_with_clocksettime;

            /*
             * Set the Linux RTC (Real-Time Clock) via Kernel.
             */
            struct rtc_time rtctime;
            struct tm       tmtime, *gmtime_ret;
            int             fd;

            gmtime_ret = gmtime_r(&tps_tv->tv_sec, &tmtime);
            if (!gmtime_ret)
            {
#if defined(EXTRA_DEBUG)
                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"gmtime_r() failed: %s\n", strerror(errno));
#endif
                ++shm_tps_ptr->debug_cnt_time_adjustment_fail_getgmtime;
                ret = -1;
            }
            else
            {
                memset(&rtctime, 0, sizeof(rtctime));
                rtctime.tm_sec   = tmtime.tm_sec;
                rtctime.tm_min   = tmtime.tm_min;
                rtctime.tm_hour  = tmtime.tm_hour;
                rtctime.tm_mday  = tmtime.tm_mday;
                rtctime.tm_mon   = tmtime.tm_mon;
                rtctime.tm_year  = tmtime.tm_year;
                rtctime.tm_wday  = tmtime.tm_wday;
                rtctime.tm_yday  = tmtime.tm_yday;
                rtctime.tm_isdst = tmtime.tm_isdst;

                fd = open("/dev/rtc0", O_RDWR);
                if (fd < 0)
                {
#if defined(EXTRA_DEBUG)
                    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "Failed to open the RTC device (%m)\n");
#endif
                    ++shm_tps_ptr->debug_cnt_time_adjustment_fail_openrtcdevice;
                    ret= -1;
                }
                else
                {
                    ret = ioctl(fd, RTC_SET_TIME, &rtctime);
                    if (ret < 0)
                    {
#if defined(EXTRA_DEBUG)
                        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"ERROR: RTC_SET_TIME ioctl failed: %s\n", strerror(errno));
#endif
                        ++shm_tps_ptr->debug_cnt_time_adjustment_fail_rtcsettime;
                    }
                    else
                    {
                        ++shm_tps_ptr->debug_cnt_set_time_with_rtcsettime;
#if defined(EXTRA_DEBUG)
                        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME," Settime RTC HW (%04d-%02d-%02d, %02d:%02d:%02d GMT)!!!!!"
                               " <<<========================================\n",
                               gps_tm->tm_year+1900, gps_tm->tm_mon+1, gps_tm->tm_mday,
                               gps_tm->tm_hour, gps_tm->tm_min, gps_tm->tm_sec);
#endif
                    }
                }

                close(fd);
            }
        }
    }
#if defined(EXTRA_DEBUG)
    if (ret < 0)
    {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"%s:  There was a failure attempting to set system/RTC time.\n",__func__);
    }
#endif
}


//
////TODO: Update comment to eliminate legacy information (once time synchronization has been fully vetted.)
//
/*Description: This function sets WSU system time from the PVT msgs.
 *If pps_gpio and ccl kernel modules are present and MMAP is available,
 *then this function sets the WSU system up to the usec resolution. This
 *is achieved through synchronization with PPS interrupt in ccl module.
 * If the MMAP from the pps_gpio driver is not present, then it ony sets WSU
 * time from the PVT msg only, which is upto sec resolution only.
 */
/*-----------------------------------------------------------------------------
** @brief  Updates WSU system time & RTC to synchronize with GNSS/PPS
** @param  in  time_source - indicates if synchronize using NMEA or UBX data
** @return unused
**---------------------------------------------------------------------------*/
void gpsUpdateLinuxSysTime(int32_t time_source)
{
    time_t              gps_time;
    struct timeval      tps_tv,cur_delta,pps;
    struct pps_tmstamp  local_pps_stmp = {0};
    struct tm           gps_tm;
    int32_t             isrLatencyFlag  = 0, ppsValidCountFlag = 0;
    int32_t             prev_pps_used   = 0;

#if defined(ENABLE_SR)
    // Dont adjust time if SR in playback mode
    if (   (NULL                  != shm_sr_ptr)
        && (SR_PLAYBACK_MODE_CHAR == shm_sr_ptr->sr_mode)
       )
    {
        return;
    }
#endif
    // Dont adjust time if config setting to do so is turned off
    if (!cfg_adjust_system_time)
        return;

    if (time_source == UPDATE_TIME_FROM_NMEA_DATA)
    {
        // Parse NMEA's strings into a tm struct

        char_t gps_dt[20];
        memset(gps_dt,'\0',sizeof(gps_dt));
        strcpy(gps_dt,gpsRMC.date);
        strcat(gps_dt,gpsGGA.time);

        if (strptime(gps_dt,UTC_STR_FRMT,&gps_tm) == NULL)
            return; // Abort on parse errors
    }
    else if (time_source == UPDATE_TIME_FROM_UBX_DATA)
    {
        // Populate a tm struct with the PVT data

        gps_tm.tm_sec   = gpsNAVPVT.sec_utc;
        gps_tm.tm_min   = gpsNAVPVT.min_utc;
        gps_tm.tm_hour  = gpsNAVPVT.hour_utc;
        gps_tm.tm_mday  = gpsNAVPVT.day_utc;
        gps_tm.tm_mon   = gpsNAVPVT.month_utc - 1;   // 0=Jan for unix, 1=Jan for GPS
        gps_tm.tm_year  = gpsNAVPVT.year_utc - 1900; // Unix time = years since 1900
        gps_tm.tm_wday  = 0;
        gps_tm.tm_yday  = 0;
        gps_tm.tm_isdst = 0;

    } else {
#if defined(EXTRA_DEBUG)
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,":gpsUpdateLinuxSysTime: Invalid input source!  I NEED PROGRAMMING!\n");
        sleep(1);
#endif
        shm_tps_ptr->error_states |= GPS_TIME_SOURCE_ERROR;
        return;
    }

    // Turn the tm struct into linux epoch time
    if ((gps_time = (time_t) mktime(&gps_tm))==-1)
    {
#if defined(EXTRA_DEBUG)
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"mktime failed\n");
#endif
        ++shm_tps_ptr->debug_cnt_time_adjustment_fail_mktime;
        return;
    }

    // Which is used to build a timeval struct
    tps_tv.tv_usec =0;
    tps_tv.tv_sec = gps_time;

    if (linuxSetSystemTimeOnce)
    {
#if defined(EXTRA_DEBUG)
        if (tpsDebugOutput & DBG_OUTPUT_ADJTIME) {
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"%s: SetTimeOnce:(gps_time=%ld)",__func__, gps_time);
        }
#endif
        if ( pps_shrd_mem.NewTimeStamp )
        {
            // Latency calculation between PPS timestamp and current
            // system time, if the latency is more than 1 sec, then
            // ignore setting system time from NMEA GPS time for this
            // iteration.

            // Copy PPS time stamp from Shared Memory to local variable
            memcpy(&local_pps_stmp, (void *) &pps_shrd_mem, sizeof(struct pps_tmstamp));
            pps.tv_sec = local_pps_stmp.pps_tv.tv_sec;
            pps.tv_usec= local_pps_stmp.pps_tv.tv_usec;

            PPSavailable = TRUE;

            isPPSTmStmpValid = checkTimeElapsedSinceLastPPS(&pps,&tps_tv);
#if defined(EXTRA_DEBUG)
            if (tpsDebugOutput & DBG_OUTPUT_ADJTIME) {
                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,":NewTimeStamp: pps.s=%ld, .usec=%ld, isPPSTSValid=%d\n", pps.tv_sec,pps.tv_usec,isPPSTmStmpValid);
            }
#endif
            pps_shrd_mem.NewTimeStamp=0;
        } else {
#if defined(EXTRA_DEBUG)
            if (tpsDebugOutput & DBG_OUTPUT_ADJTIME) {
                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"\n");
            }
#endif
            shm_tps_ptr->debug_cnt_timestamp_missing++;
        }
        // Based on the following flags this section calls settimeofday
        // syscall to set system time in single shot. This may result in
        // overshooting or under shooting of sysclock. So, this section
        // should be executed only when the system clock is far apart from
        // the GPS time.
        //
        // Follow the explanation of the used flags:
        // isPPSTmStmpValid : Indicates validity of the Time stamp taken at
        //                    the PPS ISR. It depends on execution latency
        //                    of TPS and PPS ISR.
        // PPSavailable     : Indicates availability of PPS signal.
        //                    If not, then sets the system time through time
        //                    from NMEA data.
        //
        if (isPPSTmStmpValid == TRUE)
        {
            tpsSetTimeOfDay(prev_pps_used,gps_time,&tps_tv,&gps_tm);
#if defined(EXTRA_DEBUG)
            if (tpsDebugOutput & DBG_OUTPUT_ADJTIME) {
                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME," tpsSetTimeOfDay called to adjust time\n");
            }
#endif
        }
#if defined(EXTRA_DEBUG)
        else  //DBGDBG
        {    if (tpsDebugOutput & DBG_OUTPUT_ADJTIME) {   //DBGDBG
                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME," tpsSetTimeOfDay NOT [isPPSTmStmpValid=%d]\n",isPPSTmStmpValid);   //DBGDBG
            }   //DBGDBG
        }   //DBGDBG
#endif

    }
    else
    {
        // This is a fine Tuning algorithm, which always maintains system
        // clock with in 250us at the arrival of each PPS interrupt.
        // That means system time is always ahead by a margin of <=250usec
        // when comparted to ideal PPS top of the second.

        if (!pps_shrd_mem.NewTimeStamp)
        {
            return;
        }

        // Copy PPS time stamp from Shared Memory to local variable
        memcpy(&local_pps_stmp, (void *) &pps_shrd_mem, sizeof(struct pps_tmstamp));
        pps.tv_sec = local_pps_stmp.pps_tv.tv_sec;
        pps.tv_usec= local_pps_stmp.pps_tv.tv_usec;
#if defined(EXTRA_DEBUG)
        if (tpsDebugOutput & DBG_OUTPUT_PPS) {
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME," PPS data being used#1 - pps_shrd_mem.tv=%ld.%06ld\n", pps.tv_sec, pps.tv_usec);
        }
#endif
        // Check for PPS ISR and TPS execution latency. If the latency is
        // out of bounds, it might be due to PPS ISR overwritten the shared memory.
        // So, use previous PPS timestamp from shared memory and process as usual.

        if ((pps.tv_usec > 10000) && (gps_time == pps.tv_sec))
        {
            pps.tv_usec = pps_shrd_mem.prev_pps_tv_usec;
            prev_pps_used = 1;
#if defined(EXTRA_DEBUG)
            if (tpsDebugOutput & DBG_OUTPUT_PPS) {
                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME," Latency OOB, previous PPS data being used - tv_usec=%ld\n", pps.tv_usec);
            }
#endif
            shm_tps_ptr->debug_cnt_1pps_tps_latency_event++;
        }

        // Copy PPS time stamp from Shared Memory to local variable
        cur_delta.tv_sec =  pps.tv_sec;
        cur_delta.tv_usec = pps.tv_usec;

        calculateDeltaTime(&pps, &cur_delta);   // KWHY: Why are we passing two things when they are always identical.  And are global?

        // The following IF logic compares the NMEA (GPS) and PPS time
        // second boundary:
        // 1. When both are in same second, Then verify micro second boundary
        //    with inthe same second.
        // 2. If not in the same second, then Finds out whether the time
        //    difference is less than 10msec. If yes, then it does the time
        //    adjustment instead of setting system time in single
        //    shot.
        // 3. If the second difference is out of bounds, then system time
        //    is set in single shot through settimeofday.
        //

        if (gps_time == pps.tv_sec)
        {
            deltaSum.tv_usec = deltaSum.tv_usec + cur_delta.tv_usec;

            if (prev.tv_sec == 0)
            {
                ppsCount=1;
                deltaSum.tv_usec=0;
            }


            if (ppsCount>=120)
            {
                ppsCount=1;
                deltaSum.tv_usec=0;
            }
            else
            {
                ppsCount++;
            }
        }
        else if (((gps_time - pps.tv_sec)==1) && (pps.tv_usec>990000))
        {
            //if gpstime is not equal to pps time stamp and
            //the diff is not more than 10msec -ve.
            //TPS_LOWER_TIME_US is the lower end of the accuracy desired.
            cur_delta.tv_sec=0;
            cur_delta.tv_usec=1000000-pps.tv_usec+TPS_LOWER_TIME_US;
            TimeAdjCount++;
            debug_print(gps_time, pps.tv_sec, pps.tv_usec, prev_pps_used, 0, cur_delta.tv_usec,
                        cur_delta.tv_usec, TimeAdjCount, FreqAdjCount, TimeSetCount);

            adjtime(&cur_delta,NULL);      //Adjust Time as was done on WSU 1.x
            ++shm_tps_ptr->debug_cnt_time_adjustments;
#if defined(EXTRA_DEBUG)
            if(shm_tps_ptr->debug_cnt_time_adjustments < 5) {
                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(g-p-delta:sec=1,pps.usec>10ms) usec>prev.tv_sec updated (%ld)\n", prev.tv_sec);
            }
#endif
            prev.tv_sec = pps.tv_sec;
            prev.tv_usec= cur_delta.tv_usec;
        }
        else
        {
            // If the difference between GPS and PPS time is out of bounds
            // then wait for 4 iterations before setting the system time.

            if (ppsValidCount == 2)
            {
                linuxSetSystemTimeOnce=1;
                isPPSTmStmpValid = FALSE;
#if defined(EXTRA_DEBUG)
                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME," GPS:%lu and system time not in sync:%lu.%lu\n",
                       (uint64_t)gps_time, (uint64_t)pps.tv_sec, (uint64_t)pps.tv_usec);
#endif
                ++shm_tps_ptr->debug_cnt_gps_time_and_system_time_out_of_sync;
            }

            debug_print(gps_time, pps.tv_sec, pps.tv_usec, prev_pps_used, 1, cur_delta.tv_usec,
                        0, TimeAdjCount, FreqAdjCount, TimeSetCount);

            ppsValidCount++;
            ppsValidCountFlag=1;
            prev.tv_sec = 0;
            prev.tv_usec= 0;
        } //End of gpstime comparision.

        pps_shrd_mem.NewTimeStamp = 0;

        // EXPLANATION NEEDED.  Why reset ValidCount every time it goes above 2?  [2021/11]
        if ((ppsValidCount > 2) ||(!ppsValidCountFlag))
        {
            ppsValidCount = 0;
        }

        // EXPLANATION NEEDED.  Why reset isrLatencyCheck every time it goes above 2?  [2021/11]
        if((isrLatencyCheck > 2)||(!isrLatencyFlag))
        {
            isrLatencyCheck = 0;
        }
    }
}



/*-----------------------------------------------------------------------------
** @brief  debug_print -- Print out time setting statistics (triggered by tpstestapp)
** @param  various calculated time values
** @return unused
**---------------------------------------------------------------------------*/
static void debug_print(time_t time_gps, time_t sys_sec, int64_t sys_usec, int32_t prev_pps_used, int32_t latency_exceded, int64_t delta, int64_t avgoffset, uint32_t adj_systime, uint32_t freq_systime, uint32_t set_systime )
{
#if defined(EXTRA_DEBUG)
    char_t  ch =' ', ch_1 =' ';
    static int8_t display_header=1;

    if (tpsDebugOutput & DBG_OUTPUT_TIME)
    {
        if (display_header <= 0)
        {
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"----------------------------------------------------------------------------------------------\n");
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"GPS_time(sec)    SYS_time.usec\t     Skip   Delta(usec)  Offset(usec) Adjust(No.) Settime(No.)\n");
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"\t\t\t\t\t\t\t\t     Time/Freq\n");
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"----------------------------------------------------------------------------------------------\n");

            display_header=20;
        } else {
        	--display_header;
        }

        if (prev_pps_used)
        {
            ch = '*';
        }

        if (latency_exceded)
        {
            ch_1 = '*';
        }

        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"%lu\t%c%ld.%06ld    %c\t\t%2ld\t%4ld\t\t%ld/%ld\t%4ld\n",
               (uint64_t)time_gps,ch, (uint64_t)sys_sec, sys_usec, ch_1, delta,
               avgoffset, (int64_t)adj_systime, (int64_t)freq_systime, (int64_t)set_systime);
    }
    else
    {
        display_header=1;
    }
#endif
}

#endif /*GPS_UPDATE_LINUX_SYSTIME*/
#endif /*DBG_SYSTEMTIMEUPDATE*/


/*-----------------------------------------------------------------------------
** @brief  Convert NMEA GGA Lat/Lon values from Degrees+Decimal Minutes, to Decimal Degrees
** @param  value - Lat(ddmm.mmmm) or Lon(dddmm.mmmm) to decimal degrees.
** @return value in decimal degrees.
**---------------------------------------------------------------------------*/
static float64_t gpsConvertToDegrees(float64_t value)
{
    float64_t val        = value/100.0;
    uint32_t  degreesInt = (uint32_t)val;
    float64_t minutes    = val - degreesInt;
    float64_t degrees    = degreesInt + (minutes/0.6);

    return degrees;
}



/*-----------------------------------------------------------------------------
** @brief  Send TPS Data Indication Message to TPS Main Process using UBX + maybe NMEA GST
** Note: Requires UBX-NAV-PVT, UBX-NAV-TIMELS, UBX-NAV-DOP, and
**       either UBX-NAV-EELL or NMEA-GST. (Latter used for higher accuracy.)
** @param  unused
** @return unused
**---------------------------------------------------------------------------*/
static void gpsCommSendTpsDataIndFromUbxData(void)
{
    tpsMsgBufType    msg;
    int32_t          len;
    int32_t rtnval;
    char_t dummy_data[16];

#if defined(HEAVY_DUTY)
    // Maintain message source.
    gps_data_source_message_type |= GPS_DATA_SOURCE_MESSAGE_TYPE_UBX;
    gpsData.gps_data_source_message_type |= gps_data_source_message_type;
#else
    gpsData.gps_data_source_message_type |= GPS_DATA_SOURCE_MESSAGE_TYPE_UBX;
#endif
    gpsData.valid = 1;

    // Populate the time-stutter debugging fields
    gpsData.time_msec_at_recv_trigger_ublox_msg = gpsNAVPVT.debug_recv_time_ms;
    gpsData.cpu_cycles_at_recv_trigger_ublox_msg = gpsNAVPVT.debug_recv_cpu_cycles;
    gpsData.cur_count_empty_spi_reads = shm_tps_ptr->debug_cnt_spi_empty_usleeps;

    // In NMEA format, ie DDMMYY
    memset(dummy_data,0x0,sizeof(dummy_data));
    rtnval = snprintf(dummy_data, sizeof(dummy_data),"%02d%02d%02d",
                 gpsNAVPVT.day_utc, gpsNAVPVT.month_utc, gpsNAVPVT.year_utc % 100);
    if(0 < rtnval) {
        strncpy(gpsData.date,dummy_data,7);
    }

    // Max 11 bytes:   HHMMSS(6) + .(1) + HHMM(4) = 11
    // Nano = 10^9  0-999,999,999 .. want 4 digits 0-9999 so / 10^5 = 100,000
    memset(dummy_data,0x0,sizeof(dummy_data));
    rtnval = snprintf(dummy_data, sizeof(dummy_data),"%02d%02d%02d.%04d",
                 gpsNAVPVT.hour_utc, gpsNAVPVT.min_utc, gpsNAVPVT.sec_utc,
                (gpsNAVPVT.nsec_utc >= 0 ?  gpsNAVPVT.nsec_utc / 100000 : 0) );
    if(0 < rtnval) {
        strncpy(gpsData.time,dummy_data,12);
    }

    gpsData.gps_msec = gpsNAVPVT.time_of_week_msec;

    gpsData.ubx_fix_type = gpsNAVPVT.fix_type;
    // Convert the PVT fixType into the fixquality and fix_mode fields of the old tpsData
    switch (gpsNAVPVT.fix_type)
    {
        case 1: // PVT - dead reckoning only
            gpsData.fix_mode = 3;       // 3 = 3D fix faked because WSU software won't transmit with a 2D fix */    // TODO: Debate if we want 3D or 2D after testing.
            gpsData.fixquality = 6;     // 6 = Dead reckoning mode
            break;
        case 2: // PVT - 2D-fix
            gpsData.fix_mode = 2;       // 2 = 2D fix
            if (gpsNAVPVT.flag_differential_corrections_applied) // Soln used either WAAS or SBAS
                gpsData.fixquality = 2;     // 2 = C/A differential GPS
            else
                gpsData.fixquality = 1;     // 1 = GPS fix
            break;
        case 3: // PVT - 3D-fix
            gpsData.fix_mode = 3;       // 3 = 3D fix */
            if (gpsNAVPVT.flag_differential_corrections_applied)
                gpsData.fixquality = 2;     // 2 = C/A differential GPS
            else
                gpsData.fixquality = 1;     // 1 = GPS fix
            break;
        case 4: // PVT - GNSS + dead reckoning combined
            gpsData.fix_mode = 3;       // 3 = 3D fix */
            if (gpsNAVPVT.flag_differential_corrections_applied)
                gpsData.fixquality = 12;     // GNSS + dead_recon + diff_soln .. YEESH!!  new quality 12 For debug only
            else
                gpsData.fixquality = 11;     // GNSS augmented w Dead Reckoning == new quality 11 YEESH!!  For debug only
            break;
        case 0: // PVT - no fix: We should not be in here if no fix btw
        case 5: // PVT - time only fix
        default:
            gpsData.fix_mode = 1;       // 1 = fix not available
            gpsData.fixquality = 0;     // 0 = fix not available or not C/A
            gpsData.valid = 0;
            break;
    }

    gpsData.latitude    = ((float64_t) gpsNAVPVT.lat_deg_e7) / 10000000.0;
    gpsData.longitude   = ((float64_t) gpsNAVPVT.lon_deg_e7) / 10000000.0;

    gpsData.altitude    = (float64_t) gpsNAVPVT.height_above_elipsoid_mm / 1000.0; /* to Meters */
    gpsData.groundspeed = (float64_t) gpsNAVPVT.ground_speed_mmsec / 1000.0;   /* to M/sec */

    if (gpsNAVPVT.flag_heading_valid) {
        gpsData.course      = (float64_t) gpsNAVPVT.heading_of_motion_deg_e5 / 100000.0;
    } else {
        gpsData.course      = 0;
    }

    // Copy position into SHM
    if (shm_tps_ptr->pub_pos.last_updated_index == 0) {
        shm_tps_ptr->pub_pos.lat[1] = gpsData.latitude;
        shm_tps_ptr->pub_pos.lon[1] = gpsData.longitude;
        shm_tps_ptr->pub_pos.alt[1] = gpsData.altitude;
        shm_tps_ptr->pub_pos.last_updated_index = 1;
    } else {
        shm_tps_ptr->pub_pos.lat[0] = gpsData.latitude;
        shm_tps_ptr->pub_pos.lon[0] = gpsData.longitude;
        shm_tps_ptr->pub_pos.alt[0] = gpsData.altitude;
        shm_tps_ptr->pub_pos.last_updated_index = 0;
    }


    // diff_age -- TODO -- NOT AVAILABLE IN PVT !!!
    //   gpsData.diff_age    = gpsGGA.diffAge;
    // From UBLOX NMEA Documentation: - diffAge - seconds, numeric - Age of differential corrections (blank when DGPS is not used)
    // We may want to add UBX_NAV-DGPS message for its age (I4-ms) field

    /* number of stats , lat err, lon err, semi-major axis err,
     * semi-minor axis err, orientation of semi-major axis err  */
    gpsData.numsats = gpsNAVPVT.num_sats;
    gpsData.lat_err = gpsNAVPVT.horiz_accuracy_est_mm / 1000.0; /* To meters */
    gpsData.lon_err = gpsNAVPVT.horiz_accuracy_est_mm / 1000.0; /* To meters */
    gpsData.alt_err = gpsNAVPVT.vert_accuracy_est_mm / 1000.0;   /* To meters */

    // Currently the NMEA GST message has better data than the UBX-NAV-EELL, so use that first if available
    if (gpsReceivedGPGST == TRUE) {
        gpsData.smjr_err = gpsGPST.smjr_err;
        gpsData.smnr_err = gpsGPST.smnr_err;
        gpsData.smjr_err_orient = gpsGPST.orient;
        gpsReceivedGPGST = FALSE;    // Since we used it from inside UBX function, we need to reset its flag here
        ++shm_tps_ptr->debug_gps_cnt_inds_built_from_ubx_w_valid_gst_ellipse_err;
    // Next best is EELL's 2Sigma error values
    } else if (gpsReceivedNAVEELL == TRUE) {
        gpsData.smjr_err = gpsNAVEELL.errEllipseMajor / 1000.0;   /* To meters */
        gpsData.smnr_err = gpsNAVEELL.errEllipseMinor / 1000.0;   /* To meters */
        gpsData.smjr_err_orient = gpsNAVEELL.errEllipseOrient / 100.0;   /* To degrees */
        ++shm_tps_ptr->debug_gps_cnt_inds_built_from_ubx_w_valid_eell_ellipse_err;
    } else {
        // Fudge them using horiz accuracy from NAVPVT
        gpsData.smjr_err = gpsNAVPVT.horiz_accuracy_est_mm / 1000.0; /* To meters */
        gpsData.smnr_err = gpsNAVPVT.horiz_accuracy_est_mm / 1000.0; /* To meters */
        gpsData.smjr_err_orient = 0;    /* Its a circle; any heading is ok */
        ++shm_tps_ptr->debug_gps_cnt_inds_built_from_ubx_w_estim_ellipse_err;
    }

    /* PDOP */
    gpsData.pdop = 0.01 * gpsNAVPVT.position_dilut_of_precision;   // 0.01 Scaling per UBX doc
    if (gpsReceivedNAVDOP == TRUE) {
        gpsData.hdop = 0.01 * gpsNAVDOP.hDOP;       // 0.01 Scaling per UBX doc
    }

    gpsData.flag_differential_corrections_applied = gpsNAVPVT.flag_differential_corrections_applied;

    // Sequence number
    ++shm_tps_ptr->seqno;

    // Date & time components
    gpsData.year_utc  = gpsNAVPVT.year_utc;
    gpsData.month_utc = gpsNAVPVT.month_utc;
    gpsData.day_utc   = gpsNAVPVT.day_utc;
    gpsData.hour_utc  = gpsNAVPVT.hour_utc;
    gpsData.min_utc   = gpsNAVPVT.min_utc;
    gpsData.sec_utc   = gpsNAVPVT.sec_utc;
    gpsData.nsec_utc  = gpsNAVPVT.nsec_utc;
    gpsData.flag_utc_fully_resolved = gpsNAVPVT.flag_utc_fully_resolved;

    /* UBLOX LeapSeconds */
    if (gpsReceivedTIMELS == TRUE) {
        gpsData.curLeapSecs             = gpsTIMELS.curLeapSecs;
        gpsData.srcOfCurLeapSecs        = gpsTIMELS.srcOfCurLeapSecs;
        gpsData.upcomingLeapSecChange   = gpsTIMELS.upcomingLeapSecChange;
        gpsData.timeToLeapSecEvent_secs = gpsTIMELS.timeToLeapSecEvent_secs;
        shm_tps_ptr->tpsSrcOfCurLeapsecs = LSSRC_UBLOX;
        ++shm_tps_ptr->debug_gps_count_leapsec_src_ublox;
    // See if our cache has a value
    } else if (last_good_leapsec != 0) {
        gpsData.curLeapSecs = last_good_leapsec;
        shm_tps_ptr->tpsSrcOfCurLeapsecs = LSSRC_UBLOX;
        ++shm_tps_ptr->debug_gps_count_leapsec_src_cache;
    // If persistency is enabled and we got a good value, use that
    } else if (cfg_persistency_enable && savedTpsVariables.curLeapSecs != 0) {
        gpsData.curLeapSecs = savedTpsVariables.curLeapSecs;
        gpsData.srcOfCurLeapSecs = savedTpsVariables.srcOfCurLeapSecs;
        shm_tps_ptr->tpsSrcOfCurLeapsecs = LSSRC_SAVED_DATA;
        ++shm_tps_ptr->debug_gps_count_leapsec_src_persist;
    }
    /* Send a TPS Data Indication message to the TPS Main Process. */
    msg.msgHdr.msgId = TPS_DATA_IND;
    msg.msgLen = sizeof(gpsData);

    memcpy(msg.mtext, (int8_t *)&gpsData, msg.msgLen);
    len = msg.msgLen + sizeof(msg.msgHdr) + sizeof(msg.msgLen);
    if (wsuSendData(gpsTpsSockFd, TPS_RECV_GPSDATA_PORT, &msg, len) == FALSE)
    {
#if defined(EXTRA_DEBUG)
        PRINT_MAX_N_TIMES(4, "%s: wsuSendData GPS send Ind: %s\n", __func__, strerror(errno));
#endif
    } else {
        ++shm_tps_ptr->debug_cnt_inds_sent_to_tps;
    }
}


/* GPS Comm Send TPS Data Indication Message to TPS Main Process */
/*-----------------------------------------------------------------------------
** @brief  Send TPS Data Indication Message to TPS Main Process using NMEA GGA, RMC,GSA,GST
** @param  unused
** @return unused
**---------------------------------------------------------------------------*/
static void gpsCommSendTpsDataIndFromNmeaData(void)
{
    tpsMsgBufType    msg;
    int32_t          len;
#if defined(HEAVY_DUTY)
    // Maintain message source.
    gps_data_source_message_type |= GPS_DATA_SOURCE_MESSAGE_TYPE_NMEA;
    gpsData.gps_data_source_message_type |= gps_data_source_message_type;
#else
    gpsData.gps_data_source_message_type |= GPS_DATA_SOURCE_MESSAGE_TYPE_NMEA;
#endif
    // Populate the time-stutter debugging fields
    // Pick the later of the two cycles.
    if (gpsGGA.debug_recv_time_ms > gpsRMC.debug_recv_time_ms) {
        gpsData.time_msec_at_recv_trigger_ublox_msg = gpsGGA.debug_recv_time_ms;
        gpsData.cpu_cycles_at_recv_trigger_ublox_msg = gpsGGA.debug_recv_cpu_cycles;
    } else {
        gpsData.cpu_cycles_at_recv_trigger_ublox_msg = gpsRMC.debug_recv_cpu_cycles;
        gpsData.time_msec_at_recv_trigger_ublox_msg = gpsRMC.debug_recv_time_ms;
    }
    gpsData.cur_count_empty_spi_reads = shm_tps_ptr->debug_cnt_spi_empty_usleeps;

    /* Set GPS Data structure with RMC and GGA data. */
    //memcpy(gpsData.date,  gpsRMC.date, sizeof(gpsData.date));
    //memcpy(gpsData.time,  gpsGGA.time, sizeof(gpsData.time));
    strcpy(gpsData.date,  gpsRMC.date);
    strcpy(gpsData.time,  gpsGGA.time);
    gpsData.fixquality  = gpsGGA.fixQuality;
    // gpsData.latitude    = gpsGGA.latitude;
    // gpsData.longitude   = gpsGGA.longitude;
    gpsData.latitude    = gpsRMC.latitude;   // For OEMV accuracy of lat and long values in GPRMC
    gpsData.longitude   = gpsRMC.longitude;  // is higher than the GPGGA message. i.e it has 3 more
                                             // decimal places
    gpsData.altitude    = gpsGGA.altitude;
    gpsData.groundspeed = gpsRMC.groundspeed;
    gpsData.course      = gpsRMC.course;
    gpsData.diff_age    = gpsGGA.diffAge;

    if (gpsReceivedGPGSA == TRUE) {
        if (gpsGPGSA.fix_mode == 3 && gpsGGA.fixQuality && (gpsRMC.dataStatus == 'A')) { /* 3 = 3D */
            gpsData.valid = 1;
        } else {
            gpsData.valid = 0;
        }
    } else {
        if (gpsGGA.fixQuality && (gpsRMC.dataStatus == 'A')) {
            gpsData.valid  =  1;
        } else {
            gpsData.valid  =  0;
        }
    }

/* HD Note: Need to detect version of desired NMEA string first. */
#if 0 //defined(HEAVY_DUTY)

   /*  Flags in GSA navMode NMEA 2.3 and above.
    *  https://www.u-blox.com/sites/default/files/products/documents/u-blox8-M8_ReceiverDescrProtSpec_%28UBX-13003221%29.pdf
    *
    *  1 = No fix, 2 = 2D fix, 3 = 3D fix
    */
    gpsData.gps_data_source_message_type |= GPS_DATA_SOURCE_MESSAGE_TYPE_NMEA;

    //GNSS Only use GSA string for quality

    gpsData.ubx_fix_type = gpsGPGSA.fix_mode;

    switch(gpsData.ubx_fix_type) {
        case 0:
            gpsData.valid = 0; //no msgs?
            break;
        case 1:
            gpsData.valid = 0; //No Fix.
            break;
        case 2:
            gpsData.valid = 0; //2D fix. LSQ fix: Alt fixed at sea level.
            break;
        case 3:
            gpsData.valid = 1; //3D fix. Kalman filter mode.
            break;
        default:
            gpsData.valid = 0; //no msgs?
            break;
    }
#endif

    if (isValidGroup)
    {

#ifdef DBG_SYSTEMTIMEUPDATE

#ifdef GPS_UPDATE_LINUX_SYSTIME
        if (gpsData.valid)
        {
            /* Each time the second rolls over, adjust the system time if necessary */
            if (gpsGrpReftime != lastGpsGrpReftime) {
                gpsUpdateLinuxSysTime(UPDATE_TIME_FROM_NMEA_DATA);
                lastGpsGrpReftime = gpsGrpReftime;
            }
        }
#endif /*GPS_UPDATE_LINUX_SYSTIME*/

#endif /*DBG_SYSTEMTIMEUPDATE*/

        gpsData.latitude = gpsConvertToDegrees(gpsData.latitude);
        gpsData.longitude = gpsConvertToDegrees(gpsData.longitude);

        /* Set the GPS Data LAT/LON sign. */
        if (gpsGGA.latNS  == 'S') gpsData.latitude  = -gpsData.latitude;
        if (gpsGGA.longEW == 'W') gpsData.longitude = -gpsData.longitude;

        /* Convert the GPS Data Groundspeed from Knots to Meters per Second. */
        gpsData.groundspeed *= gpsKnots2MetersPerSecond;

        /* Adjust GPS Data Altitude to be Height Above Ellipsoid. */
        gpsData.altitude += gpsGGA.hogae;

        // Copy position into SHM
        if (shm_tps_ptr->pub_pos.last_updated_index == 0) {
            shm_tps_ptr->pub_pos.lat[1] = gpsData.latitude;
            shm_tps_ptr->pub_pos.lon[1] = gpsData.longitude;
            shm_tps_ptr->pub_pos.alt[1] = gpsData.altitude;
            shm_tps_ptr->pub_pos.last_updated_index = 1;
        } else {
            shm_tps_ptr->pub_pos.lat[0] = gpsData.latitude;
            shm_tps_ptr->pub_pos.lon[0] = gpsData.longitude;
            shm_tps_ptr->pub_pos.alt[0] = gpsData.altitude;
            shm_tps_ptr->pub_pos.last_updated_index = 0;
        }

        /* number of sats, lat err, lon err, semi-major axis err,
         * semi-minor axis err, orientation of semi-major axis err  */
        gpsData.numsats  = gpsGGA.numSats;
        gpsData.lat_err  = gpsGPST.lat_err;
        gpsData.lon_err  = gpsGPST.lon_err;
        gpsData.smjr_err = gpsGPST.smjr_err;
        gpsData.smnr_err = gpsGPST.smnr_err;
        gpsData.smjr_err_orient = gpsGPST.orient;
        gpsData.alt_err  = gpsGPST.alt_err;

        /* PDOP */
        gpsData.pdop = gpsGPGSA.pdop;
        gpsData.hdop = gpsGPGSA.hdop;

        gpsData.fix_mode = gpsGPGSA.fix_mode;

        /* gps week, gps sec,solution age */
        //  gpsData.gps_week = gpsBestPos.gps_week;
        //  gpsData.gps_msec = gpsBestPos.gps_sec * 1000;
        //  gpsData.sol_age = gpsBestPos.sol_age;

        // Sequence number
        ++shm_tps_ptr->seqno;

        /* UBLOX LeapSeconds */
        if (gpsReceivedTIMELS == TRUE) {
            gpsData.curLeapSecs             = gpsTIMELS.curLeapSecs;
            gpsData.srcOfCurLeapSecs        = gpsTIMELS.srcOfCurLeapSecs;
            gpsData.upcomingLeapSecChange   = gpsTIMELS.upcomingLeapSecChange;
            gpsData.timeToLeapSecEvent_secs = gpsTIMELS.timeToLeapSecEvent_secs;
            shm_tps_ptr->tpsSrcOfCurLeapsecs = LSSRC_UBLOX;
            ++shm_tps_ptr->debug_gps_count_leapsec_src_ublox;
        // See if our ram cache has a value
        } else if (last_good_leapsec != 0) {
            gpsData.curLeapSecs = last_good_leapsec;
            shm_tps_ptr->tpsSrcOfCurLeapsecs = LSSRC_UBLOX;
            ++shm_tps_ptr->debug_gps_count_leapsec_src_cache;
        // If persistency is enabled and we got a good value, use that
        } else if (cfg_persistency_enable && savedTpsVariables.curLeapSecs != 0) {
            gpsData.curLeapSecs = savedTpsVariables.curLeapSecs;
            gpsData.srcOfCurLeapSecs = savedTpsVariables.srcOfCurLeapSecs;
            shm_tps_ptr->tpsSrcOfCurLeapsecs = LSSRC_SAVED_DATA;
            ++shm_tps_ptr->debug_gps_count_leapsec_src_persist;
        }
        /* Send a TPS Data Indication message to the TPS Main Process. */
        msg.msgHdr.msgId = TPS_DATA_IND;
        msg.msgLen = sizeof(gpsData);

        memcpy(msg.mtext, (int8_t *)&gpsData, msg.msgLen);
        len = msg.msgLen + sizeof(msg.msgHdr) + sizeof(msg.msgLen);

        //I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME," * * wsuSendData GPS send Ind\n");
        if (wsuSendData(gpsTpsSockFd, TPS_RECV_GPSDATA_PORT, &msg, len) == FALSE)
        {
#if defined(EXTRA_DEBUG)
            PRINT_MAX_N_TIMES(4, "%s: wsuSendData GPS send Ind: %s\n", __func__, strerror(errno));
#endif
        } else {
            ++shm_tps_ptr->debug_cnt_inds_sent_to_tps;
        }
    }
#if defined(EXTRA_DEBUG)
    else
    {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME," NMEA grouping Failed\n");
    } //End of isValidGroup loop
#endif
    /* Reset RMC and GGA Received flags. */
    gpsReceivedGGA   = FALSE;
    gpsReceivedRMC   = FALSE;
    gpsReceivedGPGST = FALSE;
    gpsReceivedGPGSA = FALSE;
    isValidGroup     = FALSE;
    gpsGrpReftime    =0;
    gpsGrpRef_deci   =0;
#if defined(HEAVY_DUTY)
    //clear data periodcially to remove stale fix data.
    memset(&gpsGGA,0x0,sizeof(gpsGGA));
    memset(&gpsRMC,0x0,sizeof(gpsRMC));
    memset(&gpsGPST,0x0,sizeof(gpsGPST));
    memset(&gpsGPGSA,0x0,sizeof(gpsGPGSA));
    memset(&gpsTIMELS,0x0,sizeof(gpsTIMELS));
#endif
}


/*-----------------------------------------------------------------------------
** @brief  Send TPS RTCM Indication Message to TPS Main Process.
** @param  unused
** @return unused
**---------------------------------------------------------------------------*/
static void gpsSendTpsRTCMDataInd(void)
{
    tpsMsgBufType    msg;
    int32_t          len;

    /* Send a TPS Data Indication message to the TPS Main Process. */
    msg.msgHdr.msgId = TPS_RTCM_IND;
    msg.msgLen = RTCMMsgTotalLen;

    memcpy(msg.mtext, gpsCommandBuffer, RTCMMsgTotalLen);
    len =  RTCMMsgTotalLen + sizeof(msg.msgHdr) + sizeof(msg.msgLen);

    if (wsuSendData(gpsTpsSockFd, TPS_RECV_GPSDATA_PORT, &msg, len) == FALSE)
    {
#if defined(EXTRA_DEBUG)
        PRINT_MAX_N_TIMES(4, "%s: wsuSendData GPS send Ind: %s\n", __func__, strerror(errno));
#endif
    }
}


/*-----------------------------------------------------------------------------
** @brief  Send TPS RAW Indication Message to TPS Main Process.
** @param  unused
** @return unused
**---------------------------------------------------------------------------*/
static void gpsSendTpsRAWDataInd(tpsRawMsgRes *raw_data)
{
    tpsMsgBufType    msg;
    int32_t          len;

    /* Send a TPS Data Indication message to the TPS Main Process. */
    msg.msgHdr.msgId = TPS_RAW_MSG_IND;
    msg.msgLen = sizeof(tpsRawMsgRes);

    memcpy(msg.mtext, raw_data, msg.msgLen);
    len =  msg.msgLen + sizeof(msg.msgHdr) + sizeof(msg.msgLen);

    if (wsuSendData(gpsTpsSockFd, TPS_RECV_GPSDATA_PORT, &msg, len) == FALSE)
    {
#if defined(EXTRA_DEBUG)
        PRINT_MAX_N_TIMES(4, "%s: wsuSendData GPS send Ind: %s\n", __func__, strerror(errno));
#endif
    }
}


/*-----------------------------------------------------------------------------
** @brief gpsValidateNMEAChecksum -- Calculate NMEA Checksum on NMEA message and validate.
** Note:  Checksum is calculated by taking a logical exclusive-OR operation of
**        the 8 bit message characters.  Checksum excludes the leading "$", checksum
**        delimiter "*"  and checksum itself.
** @param  curmsg - Ptr to the NMEA message received.
** @param  count  - Length of the NMEA message received.
** @return bool_t - Returns TRUE when the NMEA Msg checksum is equal to
**                  the calculated checksum else FALSE.
**---------------------------------------------------------------------------*/
static bool_t gpsValidateNMEAChecksum(uint8_t *curmsg, uint32_t count)
{
    uint8_t  cs = 0;
    uint32_t i;
    uint32_t  hexvalue=0;

    if (curmsg[0] != '$')
    {
#if defined(EXTRA_DEBUG)
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"ValNMEAChksum:A:Error: (NMEA string not start w/$) count=%d\n",count);   ///TODO:REMOVE
#endif
        return FALSE;
    }

    for (i=1; i<count; i++)
    {
        if (curmsg[i] != '*') {
            cs = cs ^ (curmsg[i]);
        } else {
            // Make sure we have enough chars left
            if (i+2 < count) {
                // Get the value of the hex digits
                if (sscanf((char *) &curmsg[i+1], "%02x", &hexvalue) == 1) {    // Need 02 to ignore any trailing valid hex digits
                    // Compare to checksum
                    if (cs == hexvalue) {
                        return TRUE;
                    }
#if defined(EXTRA_DEBUG)
                    else  { //DBGDBG
                      I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"ERROR: ValNMEAChksum:B: count=%d, i=%d (CSUM ERROR) [%c%c%c%c%c%c]"   ///TODO:REMOVE
                              " cs=%d(0x%x), hexval=%d(0x%x), chksumchars=%c%c\n", count, i,
                              curmsg[0],curmsg[1],curmsg[2],curmsg[3],curmsg[4],curmsg[5],
                              cs,cs, hexvalue,hexvalue, curmsg[i+1],curmsg[i+2] );
                    }
#endif
                }
            }
#if defined(EXTRA_DEBUG)
            else {
                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"ERROR: ValNMEAChksum:Z: count=%d, i=%d (* Too close to End):ERROR\n",count,i);
            }
#endif
            return FALSE;
        }
    }
    return FALSE;
}
#ifdef DBG_SYSTEMTIMEUPDATE

/*-----------------------------------------------------------------------------
** @brief  ppsFindSource -- Find LinuxPPS kernel device/mode and set parameters.
** @param  path       - Linux device path to kernel LinuxPPS device
** @param  handle     - Handle returned to opened device.
** @param  avail_mode - Available modes for device found.
** @return status     - 0 if successful, otherwise -1 on failure
**---------------------------------------------------------------------------*/
int ppsFindSource(char *path, pps_handle_t *handle, int *avail_mode)
{
    static int   mode = PPS_CAPTUREASSERT;     /*only supported mode*/
    pps_params_t params;
    int          ret;
#if defined(EXTRA_DEBUG)
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"trying PPS source \"%s\"\n", path);
#endif
    /*
     * Try to find the source by using the supplied "path" name
     */
    ret = open(path, O_RDWR);
    if (ret < 0)
    {
#if defined(EXTRA_DEBUG)
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"unable to open device \"%s\" (%m)\n", path);
#endif
        shm_tps_ptr->error_states |= GPS_OPEN_1PPS_ERROR;
        return ret;
    }

    /*
     * Open the PPS source (and check the file descriptor)
     */
    ret = time_pps_create(ret, handle);
    if (ret < 0)
    {
#if defined(EXTRA_DEBUG)
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"cannot create a PPS source from device \"%s\" (%m)\n", path);
#endif
        shm_tps_ptr->error_states |= GPS_CREATE_1PPS_ERROR;
        return -1;
    }
#if defined(EXTRA_DEBUG)
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"found PPS source \"%s\"\n", path);
#endif
    /*
     * Find out what features are supported
     */
    ret = time_pps_getcap(*handle, avail_mode);
    if (ret < 0)
    {
#if defined(EXTRA_DEBUG)
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "cannot get capabilities (%m)\n");
#endif
        shm_tps_ptr->error_states |= GPS_CAPABILITIES_1PPS_ERROR;
        return -1;
    }

    if (((*avail_mode) & mode) == mode)
    {
        /* Get current parameters */
        ret = time_pps_getparams(*handle, &params);
        if (ret < 0)
        {
#if defined(EXTRA_DEBUG)
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"cannot get parameters (%m)\n");
#endif
            shm_tps_ptr->error_states |= GPS_GET_PARAMS_1PPS_ERROR;
            return -1;
        }
        params.mode |= mode;
        ret = time_pps_setparams(*handle, &params);
        if (ret < 0)
        {
#if defined(EXTRA_DEBUG)
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"cannot set parameters (%m)\n");
#endif
            shm_tps_ptr->error_states |= GPS_SET_PARAMS_1PPS_ERROR;
            return -1;
        }
    }
    else
    {
#if defined(EXTRA_DEBUG)
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"selected mode not supported (%m)\n");
#endif
        shm_tps_ptr->error_states |= GPS_1PPS_MODE_BAD_ERROR;
        return -1;
    }

    fflush(stdout);
    return 0;
}


/*-----------------------------------------------------------------------------
** @brief  ppsFetchPpsSource -- Get system times at time of most recent PPS.
** @param  handle     - Handle returned to opened device.
** @param  avail_mode - Available modes for device found.
** @return status     - 0 if successful, otherwise -1 on failure
**---------------------------------------------------------------------------*/
int ppsFetchPpsSource(pps_handle_t handle, int avail_mode)
{
    struct timespec timeout;
    pps_info_t      infobuf;
    int             rtnval;

    /* create a zero-valued timeout */
    timeout.tv_sec = 3;
    timeout.tv_nsec = 0;

    if (avail_mode & PPS_CANWAIT) /* waits for the next event */
    {
        rtnval = time_pps_fetch(handle, PPS_TSFMT_TSPEC, &infobuf, &timeout);
    }
    else
    {
        sleep(1);
        rtnval = time_pps_fetch(handle, PPS_TSFMT_TSPEC, &infobuf, &timeout);
    }

    if (rtnval < 0)
    {
#if defined(EXTRA_DEBUG)
        if (errno == -EINTR)
        {
           I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"time_pps_fetch() got a signal!\n");
        }
        else
        {
           I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "time_pps_fetch() error %d (%m) [handle=%d]\n",rtnval,handle);
        }
#endif
        shm_tps_ptr->debug_cnt_1pps_fetch_event++;
        rtnval = -1;
    }
    else
    {
        // Save system clock at time of most recent PPS signal.
	      pps_shrd_mem.prev_pps_tv_usec = pps_shrd_mem.pps_tv.tv_usec;
	      pps_shrd_mem.pps_tv.tv_sec    = infobuf.assert_timestamp.tv_sec;
	      pps_shrd_mem.pps_tv.tv_usec   = infobuf.assert_timestamp.tv_nsec / 1000;

        // Counters and flags
        pps_shrd_mem.count       += 1;
        pps_shrd_mem.NewTimeStamp = 1;
        PPSavailable              = TRUE;

        // Copy PPS counter into shared memory for reporting
        shm_tps_ptr->debug_pps_cnt_interrupts = pps_shrd_mem.count;
#if defined(EXTRA_DEBUG)
        if (tpsDebugOutput & DBG_OUTPUT_PPS) {
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"#\n"); /* fetch PPS heartbeat */
        }
#endif
        rtnval = 0;
    }

    return rtnval;
}

#endif /*DBG_SYSTEMTIMEUPDATE*/


