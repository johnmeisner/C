/*---------------------------------------------------------------------------*/
/*                                                                           */
/*    Filename: tps.c                                                        */
/*     Purpose: Main thread for the Time and Position Service (tps) app      */
/*  Department: DENSO North America Research & Development                   */
/*                                                                           */
/* Copyright (C) 2015 DENSO International America, Inc.                      */
/*                                                                           */
/*     Project: V2X Special Projects                                         */
/* Description: Main functions of the TPS app                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <syslog.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <errno.h>
#include <time.h>
#include <string.h>   // for memcpy(), strcmp(), etc.
#include <unistd.h>   // for close(), write(), etc
#include "dn_types.h"
#include "wsu_util.h"
#include "tps_msg.h"
#include "tps_types.h"
#include "tps.h"
#include "gps.h"
#include "ipcsock.h"
#include "wsu_sharedmem.h"
#include "v2x_common.h"
#if defined(ENABLE_SR)
#include "shm_sr.h"
#endif
#include "shm_tps.h"
#include "tps_api.h"
#include "i2v_util.h"
#include "nscfg.h"

/* mandatory logging defines */
#if defined(MY_UNIT_TEST)
#define MY_ERR_LEVEL   LEVEL_DBG    /* from i2v_util.h */
#else
#define MY_ERR_LEVEL   LEVEL_PRIV    /* from i2v_util.h */
#endif
#define MY_NAME        "tps"

#if defined(EXTRA_DEBUG)
#define OUTPUT_MODULUS  30  /* Seconds. */
#else
#define OUTPUT_MODULUS  1200
#endif

#define NMEA_HDR 6

typedef struct
{
    int32_t  sender_port;  // <-- Port to sent to to send to process
    uint32_t pid;        // <-- This isnt useless, processes send in their pid() via tpsapi commands!
} tpsDataRegListType;

static uint32_t msgFilterIdx;
request_list    req_list;

/* two variables that live in gps.c that we need to access */
extern uint32_t tpsDebugOutput;      /* Bit-mask of flags controlling debug outputs */
extern int32_t  gpsFd;               /* File descriptor for GPS_SERIAL_DEVICE_PATH */

extern bool_t              flag_request_to_save_tps_variables;
extern savedTpsVariables_t savedTpsVariables;
extern bool_t              flag_have_read_stored_leap_secs;
extern bool_t              cfg_persistency_enable;
extern bool_t              cfg_assist_now_backup_enable;
extern uint16_t            cfg_time_accuracy_threshold;
/* Static variables used by TPS only */
static int32_t              tpsMainSockFd;                      /* Socket descriptor for main TPS thread (sockReadThread)'s socket */
static tpsDataRegListType   tpsDataRegList[TPS_DATA_REG_SIZE];  /* Holds registered apps info */
static tpsDataType          tpsData;                            /* TPS Data structure */
static pthread_mutex_t      tpsDataRegList_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t      tpsData_mutex        = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t             tpsRawData_mutex     = PTHREAD_MUTEX_INITIALIZER;
static wsu_gate_t           tpsSetDate_gate;
bool_t                      tpsMainLoop          = TRUE;
static pthread_t            sockReadThreadID;
static pthread_t            gpsCommThreadID;
static pthread_t            gpsOutputCheckThreadID;
static pthread_t            gpsGetPPStimeThreadID;
static pthread_t            gpsAssistNowBackupThreadID;
static pthread_t            shutdownSetDateThreadID;
static pthread_t            metricsThreadID;

/* Function prototypes for TPS Main states */
static void tpsMainInit(void);
static void tpsMainDenit();
static void *sockReadThread(void *arg);

/* Handles saving TPS persistent variables in main thread */
extern int32_t save_persistent_tps_values(void);

shm_tps_t * shm_tps_ptr = NULL;

static void           find_and_delete_req_item(char_t* raw_msg_hdr,int32_t hdrLen, int32_t sockfd, uint32_t pid);
static request_item * find_and_update_req_item(char_t* raw_msg_hdr,int32_t hdrLen,int32_t sockfd, uint32_t pid);
static request_item * find_req_item_by_name(char_t* raw_msg_hdr, int32_t hdrLen);
static void           sendRawDataInd(tpsMsgBufType *buf, bool_t sendDataF);
static void           get_free(request_list *ptr_req_list, request_item * ptr_req_item, request_item ** free_item, app_id ** free_app_id);

#if defined(ENABLE_SR)
#define MAX_SR_WAIT_MS 1000         // TPS will wait for SR for 1000 msecs = 1 sec
/* Config var that lives in main.c that we need to access */
extern bool_t cfg_record_tps_as_raw;
static pthread_t   srThreadID;
shm_sr_t         * shm_sr_ptr  = NULL;
static void      * srPlaybackReceiverThread (void *arg);
int32_t     sendSRMessage(uint8_t interfaceId, uint8_t messageId, uint8_t *data, uint32_t datalength);
#endif /* ENABLE_SR */

#if defined(HEAVY_DUTY) /* Silence of the warnings. */
extern void init_internal_statics(void);
extern void SendCFGRSTRequest(void);
extern void SendGNSSStopRequest(void);
extern void SendDefaultUbloxCfgs(void);
extern void DumpUbloxConfigs(void);
extern void DumpUbloxGNSSConfigs(void);
extern void SendDumpGNSSHWStatusRequest(void);
#endif

#if defined(ENABLE_WHEELTIX)

static uint16_t rr_accum = 0;
static uint16_t rl_accum = 0;
uint32_t                wheel_tick_timer_ms = 100;

pthread_rwlockattr_t    tps_wt_rwlock_attr;
pthread_rwlock_t        tps_wt_rwlock;;
timer_t                 TpsWheelTickTimer;
sem_t                   TpsWheelTickSem;

static pthread_t            TpsWheelTickThreadID;
extern uint8_t              tps_use_wheel_ticks;

ubxEsfMeas_t            UbxEsfMeasure;
uint16_t                UbxEsfMeasureMsgLen = 0;

void * TpsWheelTickFunc(void *arg);
void * TpsWheelTickFunc(void *arg)
{
    struct timespec ts;

    while (tpsMainLoop)
    {
        sem_wait(&TpsWheelTickSem);

        clock_gettime(CLOCK_MONOTONIC, &ts);
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "WT Timer call ms : %ld\n", ts.tv_nsec / 1000000);

        // We are going to update the timetag, so get the write lock
        pthread_rwlock_wrlock(&tps_wt_rwlock);

        // Update the timetag
        clock_gettime(CLOCK_MONOTONIC, &ts);
        UbxEsfMeasure.timetag = ts.tv_sec * 1000 + ts.tv_nsec / 1e6;

        if (UbxEsfMeasureMsgLen > sizeof(ubxHeader_t) + sizeof(uint16_t))
        {
#ifdef DBG_ENABLE_SEND_TO_UBLOX
            SendMsgToUblox((uint8_t *)&UbxEsfMeasure, UbxEsfMeasureMsgLen);
#else
           I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"TpsWheelTickFunc: SendMsgToUblox()  not implemented!!!\n");
#endif /*DBG_ENABLE_SEND_TO_UBLOX*/
        }

        if (pthread_rwlock_unlock(&tps_wt_rwlock)!=0) {
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"WRITEUNLOCK pthread_rwlock_wrlock()  FAILED!\n");
        }
    }

    return NULL;
}

/**
 * TPS Wheel Tick Timer Handler
 *
 * @param sig           (i) signal number
 *
 */
static void TpsWheelTickTimerHandler(int32_t sig)
{

    sem_post(&TpsWheelTickSem);
}
#endif /* ENABLE_WHEELTIX */

/* TPS Main Process */
void tpsMainProcess(bool_t debugModeOn)
{
    uint32_t onesecondcounter=0;
    int32_t syslog_open = 0;

    tpsMainInit(); /* TPS SHM created here. */
    while (tpsMainLoop == TRUE) {
        sleep(1);
        if((WTRUE == debugModeOn) && (0 == syslog_open) && (30  < onesecondcounter)) {
            if(0 == i2vUtilEnableSyslog(MY_ERR_LEVEL, MY_NAME)){
                syslog_open = 1;
            } else {
                if(onesecondcounter < 40) {
                    printf("TPS: syslog enable fail(%d)\n",onesecondcounter);
                }
                i2vUtilDisableSyslog();
            }
        }
        if (flag_request_to_save_tps_variables) {
            if (save_persistent_tps_values() == 0) {
                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "TPS: Successfully saved persistent tps values\n");
            } else {
                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "TPS: WARNING: Failed to save persistent tps values!\n");
                shm_tps_ptr->error_states |= TPS_SAVE_PERSISTENT_ERROR;
            }
            flag_request_to_save_tps_variables = FALSE;
        }
        if((debugModeOn) && (0 == (onesecondcounter % OUTPUT_MODULUS))) {
          I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"gnss:fixE:syncE(%u,%u) settimeofday=%u clocksettime=%u rtcsettime=%u\n",
              shm_tps_ptr->debug_cnt_gps_fix_lost,shm_tps_ptr->debug_cnt_gps_time_and_system_time_out_of_sync
              ,shm_tps_ptr->debug_cnt_set_time_with_settimeofday, shm_tps_ptr->debug_cnt_set_time_with_clocksettime
              ,shm_tps_ptr->debug_cnt_set_time_with_rtcsettime);
          I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"time_adjustments=%u fail_mktime=%u fail_settimeofday=%u\n",
              shm_tps_ptr->debug_cnt_time_adjustments, shm_tps_ptr->debug_cnt_time_adjustment_fail_mktime
              ,shm_tps_ptr->debug_cnt_time_adjustment_fail_settimeofday);
          I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"fail_clocksettime=%u fail_getgmtime=%u fail_openrtcdevice=%u fail_rtcsettime=%u\n", 
              shm_tps_ptr->debug_cnt_time_adjustment_fail_clocksettime,shm_tps_ptr->debug_cnt_time_adjustment_fail_getgmtime
              ,shm_tps_ptr->debug_cnt_time_adjustment_fail_openrtcdevice,shm_tps_ptr->debug_cnt_time_adjustment_fail_rtcsettime);
          I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"tstamp_miss=%u 1pps_tps_lat=%u 1pps_lat=%u 1pps_fetch_err=%u 1pps_halt_err=%u\n",
              shm_tps_ptr->debug_cnt_timestamp_missing,shm_tps_ptr->debug_cnt_1pps_tps_latency_event
              ,shm_tps_ptr->debug_cnt_1pps_latency_event,shm_tps_ptr->debug_cnt_1pps_fetch_event
              ,shm_tps_ptr->debug_cnt_1pps_halt_event);
          I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"no_timeacc_cnt=%u timeacc_exceed_cnt=%u sec_exceed=%u curr=%u min=%u max=%u\n",
             shm_tps_ptr->debug_gps_cmd_count_ubx_nav_pvt_no_timeacc
             ,shm_tps_ptr->cnt_timeacc_exceeded_threshold
             ,shm_tps_ptr->cnt_seconds_where_timeacc_exceeded_threshold
             ,shm_tps_ptr->current_time_accuracy
             ,shm_tps_ptr->time_accuracy_min
             ,shm_tps_ptr->time_accuracy_max);
        }
#if defined(MY_UNIT_TEST)
        if(2 < onesecondcounter){
            tpsMainLoop = WFALSE;
            break;
        }
#endif
        onesecondcounter++;
    }
    tpsMainDenit();
}

#if 0
void tpsSendInitComplete(void)
{
    key_t            msgkey;
    int32_t          easQid;      /* EAS queue ID */
    easMsgBuf_t      msg;

    /* Create a message queue for IPC by generating our key value and 
       opening/creating the queue.  This qid is to send the EAS init 
       complete message. */
    msgkey = ftok(EAS_PATH, EAS_PROJ_ID);
    if (msgkey == -1)
    {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "EAS msgkey\n");
        exit(1);
    }

    /* Connect to the queue */
    if ((easQid = msgget(msgkey, 0660)) == -1)
    {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "EAS openQueue\n");
        exit(1);
    }

    //I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "Qid for EAS: %d\n\n", easQid);

    /* Send the init complete message */
    msg.msgHdr.msgId = EAS_PROC_INIT_COMPLETE;
    msg.msgData = EAS_TPS_INIT_COMPLETE;          

    if (sendMessage(easQid, (void *)&msg, sizeof(easMsgBuf_t)-sizeof(int32_t)) == -1)
    {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "sendMessage to EAS\n");
        exit(1);
    }
}
#endif

#ifdef LITTLE_ENDIAN
static float64_t double_swab(const float64_t const_d)
{
    float64_t     d = const_d;
    unsigned char *p = (unsigned char *)&d;
    unsigned char t;

    t = p[0];
    p[0] = p[7];
    p[7] = t;
    t = p[1];
    p[1] = p[6];
    p[6] = t;
    t = p[2];
    p[2] = p[5];
    p[5] = t;
    t = p[3];
    p[3] = p[4];
    p[4] = t;
    return d;
}
#endif

static void sendGPSDataInd(tpsMsgBufType *buf)
{
    uint32_t        len, ix;

    /* Send a TPS Data Indication to all registered processes. */
    //msg.msgHdr.msgId = TPS_DATA_IND;
    //memcpy(msg.mtext, (int8_t *)&buf->mtext, sizeof(tpsDataType));
    //len = sizeof(tpsDataType) + sizeof(msg.msgHdr);
    len = buf->msgLen + sizeof(buf->msgHdr) + sizeof(buf->msgLen);

    // Send data to our subscribers.
    /* this single record will be sent to all registered applications.  */
    for (ix = 0; ix < TPS_DATA_REG_SIZE; ix++)
    {
        if (tpsDataRegList[ix].sender_port != -1)
        {
              if ((wsuSendData(tpsMainSockFd, tpsDataRegList[ix].sender_port, buf, len)) == FALSE)
              {
                 I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "Error wsuSendData to apps IND\n");
                 //exit(1); /* Do not exit if a client screwed up..? */
              }
        }
    }

#if defined(ENABLE_SR)
    // Record a copy if SR is in recording mode and is currently recording and is recording cooked indicators
    if (   (NULL                   != shm_sr_ptr) 
        && (SR_RECORDING_MODE_CHAR == shm_sr_ptr->sr_mode)
        && (TRUE                   == shm_sr_ptr->recordingON)
        && (FALSE                  == cfg_record_tps_as_raw)
        )
    {
#ifdef LITTLE_ENDIAN
        // Little-Endian CPUs need to switch to network endian for SR's format
        tpsMsgBufType msg;

        msg.msgHdr.msgId      = hton_uint32(buf->msgHdr.msgId);
        msg.msgHdr.appPid     = hton_uint32(buf->msgHdr.appPid);
        msg.msgHdr.errorCode  = hton_uint32(buf->msgHdr.errorCode);
        msg.msgLen = hton_uint32(buf->msgLen);

        if (buf->msgHdr.msgId == TPS_DATA_IND) {
            tpsDataType *src  = (tpsDataType *)buf->mtext;
            tpsDataType *dest = (tpsDataType *)msg.mtext;

            dest->valid       = hton_uint32(src->valid);
            memcpy(dest->time, src->time, MAX_TPS_TIME_BYTES);
            memcpy(dest->date, src->date, MAX_TPS_DATE_BYTES);
            dest->latitude    = double_swab(src->latitude);
            dest->longitude   = double_swab(src->longitude);
            dest->altitude    = double_swab(src->altitude);
            dest->groundspeed = double_swab(src->groundspeed);
            dest->course      = double_swab(src->course);
            dest->hdop        = double_swab(src->hdop);
            dest->pdop        = double_swab(src->pdop);
            dest->lat_err     = double_swab(src->lat_err);
            dest->lon_err     = double_swab(src->lon_err);
            dest->alt_err     = double_swab(src->alt_err);
            dest->gps_msec    = double_swab(src->gps_msec);
            dest->diff_age    = double_swab(src->diff_age);
            dest->sol_age     = double_swab(src->sol_age);
            dest->fixquality  = hton_uint32(src->fixquality);
            dest->numsats     = hton_uint32(src->numsats);
            dest->gps_week    = hton_uint32(src->gps_week);
            dest->smjr_err    = double_swab(src->smjr_err);
            dest->smnr_err    = double_swab(src->smnr_err);
            dest->smjr_err_orient = double_swab(src->smjr_err_orient);
            dest->fix_mode    = hton_uint32(src->fix_mode);
            // Dont forget the leap second fields
            dest->curLeapSecs             = src->curLeapSecs;
            dest->srcOfCurLeapSecs        = src->srcOfCurLeapSecs;
            dest->upcomingLeapSecChange   = src->upcomingLeapSecChange;
            dest->timeToLeapSecEvent_secs = src->timeToLeapSecEvent_secs;
        }
        else {
            /* Should be a TPS_RTCM_IND. This is an ASCII message
             * received from the GPS. Just copy it over. */
            memcpy(msg.mtext, buf->mtext, buf->msgLen - sizeof(buf->msgLen));
        }
        sendSRMessage(SR_CLIENTID_TPS, TPS_DATA, (uint8_t*)&msg, len);
#else
        sendSRMessage(SR_CLIENTID_TPS, TPS_DATA, (uint8_t*)buf, len);
#endif
        ++shm_tps_ptr->debug_cnt_sr_ind_sends;
    }
#endif /* ENABLE_SR */
}

/*
 * @brief  Sends an error message containing the passed error code
 * @note   We use tpsMainSocket for sending err reports 
 * @note   This is called directly, and used in a loop from tpsErrorReport() which is called directly
 */
static void tpsSendErrorCode(int32_t port, tpsResultCodeType errCode, int32_t flag_send_to_sr)
{
    tpsMsgBufType msg;

    /* Check signal and get to the coresponding QID's MsgID data list */
    if (port < 0)
    {
        return;
    }

    /* Send a VIS Data Indication message */
    msg.msgHdr.msgId     = TPS_ERROR_CODE;
    msg.msgHdr.errorCode = errCode;
    msg.msgLen = 0;


    if (tpsDebugOutput & DBG_OUTPUT_TPSGEN) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "tps: Sending errorcode %d to port %d\n", errCode, port);
    }
#if defined(ENABLE_SR)
    if (flag_send_to_sr) {
            sendSRMessage(SR_CLIENTID_TPS, TPS_ERROR, (uint8_t*)&msg, sizeof(msg.msgHdr) + sizeof(msg.msgLen));
    } else
#endif 
    {
       if ((wsuSendData(tpsMainSockFd, port, &msg, sizeof(msg.msgHdr) + sizeof(msg.msgLen))) == FALSE)
       {
           I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "tpsSendErrorCode to registered App\n");
       }
    }
}


void tpsExitSigHandler(int32_t sig)
{
    if (tpsMainLoop == TRUE) {
        tpsMainLoop = FALSE;
        if (wsu_open_gate(&tpsSetDate_gate) != TRUE) {
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "tpsExitSigHandler: ERROR: wsu_open_gate(tpsSetDate_gate) FAILED!\n");
        }
    }
}

// Sets the realtime clock from the system time at shutdown
void * ShutdownSetDateThreadFunc(void * arg)
{
#if !defined(MY_UNIT_TEST)
    /* Wait until the shutdown gate is opened */
    wsu_wait_at_gate(&tpsSetDate_gate);
#endif

    if (tpsDebugOutput & DBG_OUTPUT_TPSGEN) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "TPS::ShutdownSetDateThreadFunc: I went through the shutdown gate!!\n");
    }
/* QNX and OBU related. RSU does not need to do this. */
#if !defined(HEAVY_DUTY) 
    // Sigh.  Call the "rtc" command which DOES set the hardware RTC   [20161014]
    int32_t rc = system("/proc/boot/rtc -s -l hw\n");
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "TPS: system(/proc/boot/rtc -s -l hw) returned %d (want 0)\n", rc);
#endif

    if (tpsDebugOutput & DBG_OUTPUT_TPSGEN) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "TPS::ShutdownSetDateThreadFunc: Done!\n");
    }

    return 0;
}

// Aggregates time-based metrics for TPS health and performance
#include <sys/sysinfo.h>
void * MetricsThreadFunc(void * arg)
{
    struct sysinfo s_info;
    uint32_t cur_idx, cur_uptime_hour, cur_uptime_day;
    uint32_t prev_uptime_hour = 0, prev_uptime_day = 0;

    while (tpsMainLoop == TRUE) {
        sleep(1);
        if (sysinfo(&s_info) == 0) {
            cur_uptime_hour = (s_info.uptime / 3600) % 24;
            cur_uptime_day = s_info.uptime / (24 * 3600);

            // If we are in a new day, record the past one
            if (cur_uptime_day != prev_uptime_day) {
                cur_idx = (shm_tps_ptr->index_for_latest_timefix_prev_day_data + 1) % NUM_DAYS_TRACKED_TIME_CORRECTIONS;
                shm_tps_ptr->cnt_set_times_in_prev_day[cur_idx] = 
                    shm_tps_ptr->debug_cnt_set_time_with_settimeofday - shm_tps_ptr->prev_day_sets_count;
                shm_tps_ptr->cnt_time_adjustments_in_prev_day[cur_idx] =
                    shm_tps_ptr->debug_cnt_time_adjustments - shm_tps_ptr->prev_day_adjs_count;
                shm_tps_ptr->uptime_days_for_timefix_prev_day[cur_idx] = cur_uptime_day;    // NOTE: We want prev_uptime_day+1, to present day=0 as 1st day. Since prev+1 = cur, we set to cur_uptime_day's value
                shm_tps_ptr->index_for_latest_timefix_prev_day_data = cur_idx;
                shm_tps_ptr->prev_day_sets_count = shm_tps_ptr->debug_cnt_set_time_with_settimeofday;
                shm_tps_ptr->prev_day_adjs_count = shm_tps_ptr->debug_cnt_time_adjustments;
                prev_uptime_day = cur_uptime_day;
            }


            // If we are in a new hour, record the past one
            if (cur_uptime_hour != prev_uptime_hour) {
                cur_idx = (shm_tps_ptr->index_for_latest_timefix_prev_hour_data + 1) % NUM_HOURS_TRACKED_TIME_CORRECTIONS;
                shm_tps_ptr->cnt_set_times_in_prev_hour[cur_idx] = 
                    shm_tps_ptr->debug_cnt_set_time_with_settimeofday - shm_tps_ptr->prev_hour_sets_count;
                shm_tps_ptr->cnt_time_adjustments_in_prev_hour[cur_idx] =
                    shm_tps_ptr->debug_cnt_time_adjustments - shm_tps_ptr->prev_hour_adjs_count;
                shm_tps_ptr->uptime_hours_for_timefix_prev_hour[cur_idx] = cur_uptime_hour; // See NOTE about uptime_days_for_timefix_prev_day
                shm_tps_ptr->index_for_latest_timefix_prev_hour_data = cur_idx;
                shm_tps_ptr->prev_hour_sets_count = shm_tps_ptr->debug_cnt_set_time_with_settimeofday;
                shm_tps_ptr->prev_hour_adjs_count = shm_tps_ptr->debug_cnt_time_adjustments;
                // Also record hourly pps counts
                cur_idx = (shm_tps_ptr->index_for_latest_pps_prev_hour_data + 1) % NUM_HOURS_TRACKED_PPS_INTERRUPTS;
                shm_tps_ptr->track_pps_interrupts_per_hour[cur_idx] =
                    shm_tps_ptr->debug_pps_cnt_interrupts - shm_tps_ptr->prev_hour_pps_cnt;
                shm_tps_ptr->uptime_hours_for_pps_prev_hour[cur_idx] = cur_uptime_hour; // See NOTE about uptime_days_for_timefix_prev_day
                shm_tps_ptr->index_for_latest_pps_prev_hour_data = cur_idx;
                shm_tps_ptr->prev_hour_pps_cnt = shm_tps_ptr->debug_pps_cnt_interrupts;
                // Update prev hour
                prev_uptime_hour = cur_uptime_hour;
            }
        }
        #if defined(MY_UNIT_TEST)
            tpsMainLoop = WFALSE;
        #endif
    }
#if defined(EXTRA_DEBUG)
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "MetricsThreadFunc: Done\n");
#endif
    return 0;
}


#if defined(ENABLE_WHEELTIX)
bool_t TpsWheelTickInit(void)
{
    struct itimerspec tv;

    pthread_rwlockattr_init(&tps_wt_rwlock_attr);

///TODO:WHY DID TPSCODE HAVE  (use mutex setpshared function for rwlock when rwlock setpshared exists?????
///TODO: VERIFY WHAT IS CORRECT HERE!!!!!!!!!!!!!!!!!!!!  -Chet 2/2019
///TODO: VERIFY THIS CODE!!!!!!!
///
///     THIS:            pthread_mutexattr_setpshared( &tps_wt_rwlock_attr, PTHREAD_PROCESS_SHARED);
///     instead of this: pthread_rwlockattr_setpshared( &tps_wt_rwlock_attr, PTHREAD_PROCESS_SHARED);
///
///  pthread_mutexattr_setpshared( &tps_wt_rwlock_attr, PTHREAD_PROCESS_SHARED);
     pthread_rwlockattr_setpshared ( &tps_wt_rwlock_attr, PTHREAD_PROCESS_SHARED);

    // Init our shmlock2's rwlock var
    if (pthread_rwlock_init(&tps_wt_rwlock, &tps_wt_rwlock_attr) != 0) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "TPS wheel tick RW lock init failed! error=%d=%s,pid=%d\n", errno, strerror( errno ), getpid() );
        return FALSE;
    }

    if (sem_init(&TpsWheelTickSem, TRUE, 1))
    {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "Wheel Tick Tx sem init failed\n");
        exit(EXIT_FAILURE);
    }

    wsuUtilSetupTimer(wheel_tick_timer_ms, &TpsWheelTickTimer, TpsWheelTickTimerHandler, &tv);

    return TRUE;
}
#endif /* ENABLE_WHEELTIX */

/* TPS Main Initialization State Processing */
static void tpsMainInit(void)
{
    int32_t          ix;
    sigset_t         mask;

    // Set our common looping variable
    tpsMainLoop = TRUE;

    /* Initialize variables. */
    memset(&tpsDataRegList, 0, sizeof(tpsDataRegList));
    for (ix = 0; ix < TPS_DATA_REG_SIZE; ix++)
    {
        tpsDataRegList[ix].sender_port = -1;
    }
    memset(&tpsData, 0, sizeof(tpsData));

#if defined(HEAVY_DUTY)
    init_internal_statics(); //handle soft reset. RSU vs OBU different.
#endif

    // Create our shared memory //
    if ((shm_tps_ptr = wsu_share_init(sizeof(shm_tps_t), SHM_TPS_PATH)) == NULL)
    {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "%s : Could not create TPS's shared memory, tps startup aborted\n", __func__);
        exit(1);
    }
    // Wipe our shared memory.  Normally this isn't an issue, but during development we
    //   keep killing tps & restarting a new version, and our counters continue from
    //   the existing values, which give incorrect data.  Fixed by memset 0.
    memset(shm_tps_ptr, 0, sizeof(shm_tps_t));

    // If persistency read stored data, copy leapSec values into tps's shared memory locations for those values
    // This is to handle case where GPS doesn't get a fix quickly at startup, GPS won't send a good TIMELS
    // until then, so radioStack can get valid leapsecs sooner.
    if (   (TRUE == cfg_persistency_enable)
        && (TRUE == flag_have_read_stored_leap_secs)
       ) 
    {
        shm_tps_ptr->curLeapSecs         = savedTpsVariables.curLeapSecs;
        shm_tps_ptr->srcOfCurLeapSecs    = savedTpsVariables.srcOfCurLeapSecs;
        shm_tps_ptr->leap_secs_valid     = TRUE;
        shm_tps_ptr->tpsSrcOfCurLeapsecs = LSSRC_SAVED_DATA;
    }

    shm_tps_ptr->time_accuracy_min = 1000;
    shm_tps_ptr->time_accuracy_max = 0;
    shm_tps_ptr->time_accuracy_threshold = (uint32_t)cfg_time_accuracy_threshold;

    // Set the initial state of our time sync state and gps position state
    shm_tps_ptr->cur_tps_posn_state = TGP_NO_POSITION;
    if (time(0L) > 31536000) {  // Seconds in a year; if we have a date >= 1971 the RTC set system time at boot
        shm_tps_ptr->cur_tps_time_state = TTS_RTC_SYNC;
    } else {
        shm_tps_ptr->cur_tps_time_state = TTS_NO_SYNC;
    }
    shm_tps_ptr->current_time_accuracy = -1;    // sets it to maxint32 .. very unaccurate


#if defined(ENABLE_SR)
    // Connect to SR's shared memory
    if ((shm_sr_ptr = wsu_share_init(sizeof(shm_sr_t), SHM_SR_PATH)) == NULL)
    {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "%s : Could not connect to SR's shared memory.\n", __func__);
    }

    // Wait for SR to populate its mode into its shared memory.  We must know this before
    //    we exit tpsMainInit() because it decides starting our SR playback thread.
    uint32_t  sr_wait_counter = 0;
    for ( sr_wait_counter = 0; 
          !(SR_IS_DONE_INITIALIZING(shm_sr_ptr)) && (sr_wait_counter < MAX_SR_WAIT_MS);
          ++sr_wait_counter )
    {
        usleep(0);      // The smallest sleep available on our QNX system, 1 ms
    }

    /* Default to SRMode = none if SR startup not detected */
    if (!(SR_IS_DONE_INITIALIZING(shm_sr_ptr)))
    {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "SR startup was not detected, defaulting to SRMode = none\n");
        wsu_share_kill(shm_sr_ptr, sizeof(shm_sr_t));
        shm_sr_ptr = NULL;
    }
#endif /* ENABLE_SR */

    /* Create our main socket */
    tpsMainSockFd = wsuCreateSockServer(TPS_RECV_DATAREQ_PORT);
    if (tpsMainSockFd < 0)
    {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "wsuCreateSockServer failed\n");
        exit(1);
    }


    /* Start the GPS Communication (Processing)  Thread. */
    if (pthread_create(&gpsCommThreadID, NULL, &gpsCommThreadFunc, NULL) != 0)
    {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "gpsCommThread creation failed\n");
        exit(1);
    }
#if defined(EXTRA_DEBUG)
    else
    {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "Thread Created: pthread_create(gpsCommThread)()...\n");
    }
#endif

    /* Start the GPS output check Thread. */
    if (pthread_create(&gpsOutputCheckThreadID, NULL, &gpsOutputCheckThreadFunc, NULL) != 0)
    {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "gpsOutputCheckThread creation failed\n");
        exit(1);
    }
#if defined(EXTRA_DEBUG)
    else
    {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "Thread Created: pthread_create(gpsOutputCheckThread)()...\n");
    }
#endif

#if defined(ENABLE_SR)
    /* Start the GPS output check Thread. */
    if (shm_sr_ptr == NULL || shm_sr_ptr->sr_mode != SR_PLAYBACK_MODE_CHAR) 
#endif
    {
        if (pthread_create(&gpsGetPPStimeThreadID, NULL, &gpsGetPPStimeThreadFunc, NULL) != 0)
        {
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "gpsGetPPStimeThread creation failed\n");
            exit(1);
        }
#if defined(EXTRA_DEBUG)
        else
        {
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "Thread Created: pthread_create(gpsGetPPStimeThread)()...\n");
        }
#endif
    }


    /* Start the thread that handles the GPS data coming from the GPS processing thread */
    if (pthread_create(&sockReadThreadID, NULL, &sockReadThread, NULL) != 0)
    {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "gpsCommThread creation failed\n");
        exit(1);
    }
#if defined(EXTRA_DEBUG)
    else
    {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "Thread Created: pthread_create(sockReadThread)()...\n");
    }
#endif
    /* Init the ShutdownSetDate gate.  A pthread_mutex didn't do the trick as
     *    I couldn't find a way for the signal handler context to have permission 
     *    to unlock the mutex, even with PTHREAD_PROCESS_SHARED set. */
    wsu_init_gate(&tpsSetDate_gate);


    /* Start the ShutdownSetDate thread */
    if (pthread_create(&shutdownSetDateThreadID, NULL, &ShutdownSetDateThreadFunc, NULL) != 0)
    {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "shutdownSetDateThread creation failed\n");
        exit(1);
    }
#if defined(EXTRA_DEBUG)
    else
    {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "Thread Created: pthread_create(shutdownSetDateThread)()...\n");
    }
#endif

    /* Start the Quality and Performance Metrics thread */
    if (pthread_create(&metricsThreadID, NULL, &MetricsThreadFunc, NULL) != 0)
    {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "metricsThread creation failed\n");
        exit(1);
    }
#if defined(EXTRA_DEBUG) 
    else 
    {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "Thread Created: pthread_create(metricsThread)()...\n");
    }
#endif    
    /* Start the Periodic Backup of AssistNow GPS Data thread */
    if (TRUE == cfg_assist_now_backup_enable)
    {
        if (pthread_create(&gpsAssistNowBackupThreadID, NULL, &gpsAssistNowBackupThreadFunc, NULL) != 0)
        {
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "gpsAssistNowBackupThread creation failed\n");
            exit(1);
        } else {
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "Thread Created: pthread_create(gpsAssistNowBackupThread)()...\n");
        }
    }

#if defined(ENABLE_SR)
    // Startup TPS's Playback thread to handle data coming from SR
    if (SR_IN_PLAYBACK_MODE(shm_sr_ptr))
    {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "tpsMainInit(): Starting srPlaybackReceiverThread thread!\n");
        if (pthread_create(&srThreadID, NULL, &srPlaybackReceiverThread, NULL) != 0)
        {
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "Thread srPlaybackReceiverThread creation failed\n");
        }
    }
#endif
    /* Set up the mask such that the created threads will not handle any signals by default.*/
    sigfillset(&mask);
    pthread_sigmask(SIG_BLOCK, &mask, NULL);

    /* Setup our shutdown signal handler */
    wsuUtilSetupSignalHandler(tpsExitSigHandler, SIGUSR1, 0);
    wsuUtilSetupSignalHandler(tpsExitSigHandler, SIGTERM, 0);
    wsuUtilSetupSignalHandler(tpsExitSigHandler, SIGKILL, 0);

#if !defined(HEAVY_DUTY)
    if (tps_use_wheel_ticks)
    {
        /* Start the Wheel Tick Timer handler thread */
        if (pthread_create(&TpsWheelTickThreadID, NULL, &TpsWheelTickFunc, NULL) != 0)
        {
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "gpsOutputCheckThread creation failed\n");
            exit(1);
        }

        TpsWheelTickInit();
    }
#endif

#ifdef GPS_UPDATE_LINUX_SYSTIME
    /* Tell the gpcComm thread to set the realtime clock the first time it gets a fix */
    extern int32_t linuxSetSystemTimeOnce;
    linuxSetSystemTimeOnce=1;
#endif

}


static void tpsMainDenit()
{
    /* Must wait for the shutdownSetDateThread to finish, so CLOCK_REALTIME gets set! */
    pthread_join(shutdownSetDateThreadID, NULL);

    // Need to use SIGUSR1 to get some threads out of the recvfrom() and sleep() calls
    pthread_kill(sockReadThreadID, SIGUSR1);
    pthread_kill(gpsCommThreadID, SIGUSR1);
    pthread_kill(gpsOutputCheckThreadID, SIGUSR1);
    pthread_kill(metricsThreadID, SIGUSR1);
    if (TRUE == cfg_assist_now_backup_enable) 
    {
        pthread_kill(gpsAssistNowBackupThreadID, SIGUSR1);
    }
#if defined(ENABLE_SR)
    if (   (NULL                  == shm_sr_ptr)  
        || (SR_PLAYBACK_MODE_CHAR != shm_sr_ptr->sr_mode)
       ) 
#endif
    {
        pthread_kill(gpsGetPPStimeThreadID, SIGUSR1);
    }

#if defined(ENABLE_SR)
    if (shm_sr_ptr != NULL && SR_IN_PLAYBACK_MODE(shm_sr_ptr)) {
        pthread_kill(srThreadID, SIGUSR1);
    }
#endif
    // Joins when threads finish
    pthread_join(gpsOutputCheckThreadID, NULL);
#if defined(ENABLE_SR)
    if (   (NULL                  == shm_sr_ptr) 
        || (SR_PLAYBACK_MODE_CHAR != shm_sr_ptr->sr_mode)
       ) 
#endif
    {
        pthread_join(gpsGetPPStimeThreadID, NULL);
    }
    pthread_join(shutdownSetDateThreadID, NULL);
    if (TRUE == cfg_assist_now_backup_enable) 
    {
        pthread_join(gpsAssistNowBackupThreadID, NULL);
    }
    pthread_join(metricsThreadID, NULL);
    pthread_join(gpsCommThreadID, NULL);
    pthread_join(sockReadThreadID, NULL);

#if defined(ENABLE_SR)
    if (   (NULL != shm_sr_ptr)  
        && (SR_IN_PLAYBACK_MODE(shm_sr_ptr) )
       ) 
    {
        pthread_join(srThreadID, NULL);
    }

    // Release SR's shared memory
    if (NULL != shm_sr_ptr) {
        wsu_share_kill(shm_sr_ptr, sizeof(shm_sr_t));
    }
#endif
    if(shm_tps_ptr!= NULL) {
        wsu_share_kill(shm_tps_ptr, sizeof(shm_tps_t));
        wsu_share_delete(SHM_TPS_PATH);
    }

    /* Close socket */
    close(tpsMainSockFd);
}


static void removePidFromRegList(uint32_t pid)
{
    int32_t  ix;

    for (ix = 0; ix < TPS_DATA_REG_SIZE; ix++)
    {
        if (pid == tpsDataRegList[ix].pid)
        {
            tpsDataRegList[ix].sender_port = -1;
            tpsDataRegList[ix].pid = 0;
        }
    }
}

static void tpsDeregister(tpsMsgBufType *buf, int32_t port_num)
{
    uint32_t pid = 0;
    int32_t  numPids, ix;
    int32_t  rc;

    if (buf != NULL)
    {
        pid = buf->msgHdr.appPid;     /* Get registering app's PID. */

        /* Deregister requestor for TPS Data Indications. */
        numPids = 0;
        if ((rc = pthread_mutex_lock(&tpsDataRegList_mutex)) != 0)
        {
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "TPS Data Reg List Mutex Lock Error (%d)\n", rc);
        }
        for (ix = 0; ix < TPS_DATA_REG_SIZE; ix++)
        {
            if (pid == tpsDataRegList[ix].pid)
            {
                numPids++;
                if (port_num == tpsDataRegList[ix].sender_port)
                {
                    tpsDataRegList[ix].sender_port = -1;
                    //pid = tpsDataRegList[ix].pid;
                    tpsDataRegList[ix].pid = 0;
                    numPids--;
                    //break;
                }
            }
            /* Sanity check */
            else if (port_num == tpsDataRegList[ix].sender_port)
            {
                /* This will only happen when Dereg msg sends wrong PID. */
                tpsDataRegList[ix].sender_port = -1;
                tpsDataRegList[ix].pid = 0;
            }
        }
        if ((rc = pthread_mutex_unlock(&tpsDataRegList_mutex)) != 0)
        {
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "TPS Data Reg List Mutex Unlock Error (%d)\n", rc);
        }
        if (numPids == 0)
        {
            //removePid(pid);
        }
    }
    else    // The socket connection went away
    {
        if ((rc = pthread_mutex_lock(&tpsDataRegList_mutex)) != 0)
        {
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "TPS Data Reg List Mutex Lock Error (%d)\n", rc);
        }
        for (ix = 0; ix < TPS_DATA_REG_SIZE; ix++)
        {
            //if (port_num == tpsDataRegList[ix].sender_port)
            if (port_num == tpsDataRegList[ix].sender_port)
            {
                tpsDataRegList[ix].sender_port = -1;
                //pid = tpsDataRegList[ix].pid;
                pid = tpsDataRegList[ix].pid;
                tpsDataRegList[ix].pid = 0;
                break;
            }
        }
        if (pid != 0)
        {
            //removePid(pid);
            removePidFromRegList(pid);
        }
        if ((rc = pthread_mutex_unlock(&tpsDataRegList_mutex)) != 0)
        {
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "TPS Data Reg List Mutex Unlock Error (%d)\n", rc);
        }
    }
}

/**
 * @brief This thread handles data bubbling up from the gps thread and
 *   services interactions with outside processes, such as commands for
 *   the GPS device (TPS_GPS_OUTPUT_REQ), data requests (TPS_DATA_REQ),
 *   subscription requests for data (TPS_DATA_REG, TPS_RAW_MSG_REQ), or
 *   subscription cancelations (TPS_DATA_DEREG, TPS_RAW_MSG_DEREG).
 * @note  This is the main tps thread.  Maybe we should rename it
 *   to something better than sockReadThread ...  tpsMainThread?  MasterThread?
 *   tpsMainSwitchboardThread   MiddleMan
 */
static void * sockReadThread(void *arg)
{
    request_item *free_item;
    app_id * free_app_id;
    tpsRawMsgReq *tps_api_req;
    int32_t sender_port;   // We use sender_port not sockfd to track our customers since DGRAM conversion
    int32_t       ret, ix;
    int32_t       len, rcvd_len;
    int32_t       rc;
    tpsMsgBufType buf;
    bool_t registered = FALSE;
    RcvDataType   rcv_data = {
        FALSE, tpsMainSockFd, &buf, sizeof(buf), TPS_RECV_GPSDATA_PORT   //KTODO: Merge with TPS_RECV_DATAREQ_PORT ??
    };

    while (tpsMainLoop == TRUE)
    {
        if ((rcvd_len = wsuReceiveData(TPS_SOCKRECV_TIMEOUT, &rcv_data)) == -1)
        {
        	// KNOTE: Connectionless DGRAMS do not drop connection.  If we
        	//   get a wsuReceiveData failure, we could restart this entire
        	//   thread and close+open socket, and/or dropping everyone that has
            //   subscribed and forcing everyone to reconnect and resubscribe. 
        	//   I am unsure of what causes wsuReceiveData failures in DGRAM mode,
        	//   so for now we are going to ignore DGRAM receive errors other
        	//   that outputting error messages.  If we're going to drop our
            //   socket, we should move the socket creation code into this
            //   thread so we can redo it after we close on an error.  20150527
        	//I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "sockReadThread: wsuReceiveData failure!  I NEED PROGRAMMING!\n");
   
        	I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Socket Error: wsuReceiveData() rtn=%d (<0), errno=%d[%s] fd=%d\n",rcvd_len,errno,strerror(errno),rcv_data.fd);
            continue;
        }
#ifdef DEBUG_SOCKET_EINTR
        if (rcvd_len == 0)
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Socket WARN: wsuReceiveData() rtn=%d (==0), errno=%d[%s] fd=%d\n",rcvd_len,errno,strerror(errno),rcv_data.fd);
#endif /*DEBUG_SOCKET_EINTR*/

        if (tpsDebugOutput & DBG_OUTPUT_TPSGEN) {
            if (buf.msgHdr.msgId != TPS_DATA_IND && buf.msgHdr.msgId != TPS_RTCM_IND)
                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"tpsMasterThread: Received TPS Command id=%d: len=%d, avail=%d, sender_port: %d\n",
                         buf.msgHdr.msgId, rcvd_len, rcv_data.available, rcv_data.sender_port);
        }

        if (rcv_data.available)
        {
        	sender_port = rcv_data.sender_port;   // Who sent us the request

            switch (buf.msgHdr.msgId)
            {
                /* TPS Data Registration - Register the requestor for TPS Data
                   Indications. */
            case TPS_DATA_REG:
                //I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "TPS_DATA_REG: Process Registered\n");

                /* First check if already registered.  If so, do nothing. */
                registered = FALSE;
                if ((rc = pthread_mutex_lock(&tpsDataRegList_mutex)) != 0)
                {
                    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "TPS Data Reg List Mutex Lock Error (%d)\n", rc);
                }
                for (ix = 0; ix < TPS_DATA_REG_SIZE; ix++)
                {
                    if (sender_port == tpsDataRegList[ix].sender_port)
                    {
                        registered = TRUE;
                        break;
                    }
                }

                /* Register requestor for TPS Data Indications. */
                if (!registered)
                {
                    for (ix = 0; ix < TPS_DATA_REG_SIZE; ix++)
                    {
                        if (tpsDataRegList[ix].sender_port == -1)
                        {
                            tpsDataRegList[ix].sender_port = sender_port;
                            tpsDataRegList[ix].pid = buf.msgHdr.appPid;
                            //tpsPidList[ix] = buf.msgHdr.appPid;
                            break;
                        }
                    }
                    if (ix == TPS_DATA_REG_SIZE)
                    {
                        /* No room to register */
                        // Send error code to the app for no room
                        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "TPS Register List is full. Can't register.\n");
                        tpsSendErrorCode(sender_port, TPS_REG_LIST_FULL, 0);
                    }
                }
                if ((rc = pthread_mutex_unlock(&tpsDataRegList_mutex)) != 0)
                {
                    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "TPS Data Reg List Mutex Unlock Error (%d)\n", rc);
                }

                break;

                /* TPS Data Deregistration - Deregister the requestor for TPS
                   Data Indications. */
            case TPS_DATA_DEREG:
                //I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "Process DERegistered\n");
                tpsDeregister(&buf, sender_port);

                break;

                /* TPS Data Indication - Reset alarm and then store/forward    */
                /* this message to all registered processes.                   */
            case TPS_DATA_IND:
                //I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "*** TPS IND Received\n");

                if ((rc = pthread_mutex_lock(&tpsData_mutex)) != 0)
                {
                    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "TPS Data Mutex Lock Error (%d)\n", rc);
                }

                memcpy((int8_t *)&tpsData, (int8_t *)buf.mtext, sizeof(tpsData));

                if ((rc = pthread_mutex_unlock(&tpsData_mutex)) != 0)
                {
                    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "TPS Data Mutex Unlock Error (%d)\n", rc);
                }

                /* Send a TPS Data Indication to all registered processes. */
                //sendGPSDataInd(&buf, sizeof(tpsData));
                sendGPSDataInd(&buf);

                break;

            case TPS_DATA_REQ:

                /* Send a TPS Data Response message. */
                buf.msgHdr.msgId = TPS_DATA_CFM;
                if ((rc = pthread_mutex_lock(&tpsData_mutex)) != 0)
                {
                    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "TPS Data Mutex Lock Error (%d)\n", rc);
                }
                memcpy(buf.mtext, (int8_t *)&tpsData, sizeof(tpsDataType));
                if ((rc = pthread_mutex_unlock(&tpsData_mutex)) != 0)
                {
                    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "TPS Data Mutex Unlock Error (%d)\n", rc);
                }
                buf.msgLen = sizeof(tpsData);
                len = buf.msgLen + sizeof(buf.msgHdr) + sizeof(buf.msgLen);
                if ((wsuSendData(tpsMainSockFd, sender_port, &buf, len)) == FALSE)
                {
                   I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "wsuSendData error for REQ/CFM\n");
		            }

                break;

            // This is for sending commands into the GPS device through the serial port
            case TPS_GPS_OUTPUT_REQ:

                if (gpsFd > 0)
                {
                    ret = write(gpsFd, (int8_t *)buf.mtext, buf.msgLen);
                    if (ret < 0)
                    {
                        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "Write system call\n");
                        tpsSendErrorCode(sender_port, GPS_WRITE_ERROR, 0);
                        //I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "TPS String: %s, Return value :%d\n",(int8_t *)buf.mtext, ret);
                    } else if (tpsDebugOutput & DBG_OUTPUT_TPSGEN) {
                        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "tps: Successfully sent %d-byte command to GPS device\n", buf.msgLen);
                    }
                }

                break;

            // Update the debug output mask with a new value
            case TPS_DEBUG_OUTPUT:

                memcpy((uint32_t *)&tpsDebugOutput, (uint32_t *)buf.mtext, sizeof(uint32_t));
                if (tpsDebugOutput & DBG_OUTPUT_TPSGEN) {
                    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "set tpsDebugOutputMask = 0x%04x\n", tpsDebugOutput);
                }
                break;

            // Request for the debug output mask's current value
            case TPS_GPS_OUTPUT_MASK_REQ:

                /* Send a TPS Debug Output Mask Value Response message. */
                buf.msgHdr.msgId = TPS_GPS_OUTPUT_MASK_IND;
                memcpy((uint32_t *)buf.mtext, (uint32_t *)&tpsDebugOutput, sizeof(uint32_t));
                buf.msgLen = sizeof(uint32_t);
                len = buf.msgLen + sizeof(buf.msgHdr) + sizeof(buf.msgLen);
                if ((wsuSendData(tpsMainSockFd, sender_port, &buf, len)) == FALSE)
                {
                   I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "wsuSendData error for REQ/tpsDbgOutputMask\n");
		            } else if (tpsDebugOutput & DBG_OUTPUT_TPSGEN) {
                    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "KDEBUG: wsuSendData() Sent mask value 0x%04x in %d msg\n", tpsDebugOutput, buf.msgHdr.msgId);
                }
                break;


            case TPS_RTCM_IND:
                //I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "*** TPS RTCM IND Received\n");

                /* Send a TPS Data Indication to all registered processes. */
                sendGPSDataInd(&buf);

                break;

             // RAW NMEA messages.  Not for RAW UBX messages as they're not in ASCII
             case TPS_RAW_MSG_REQ:
                tps_api_req = (tpsRawMsgReq *)buf.mtext;

                if ((rc = pthread_mutex_lock(&tpsRawData_mutex)) != 0)
                {
                    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "TPS RAW Data Reg List Mutex Lock Error (%d)\n", rc);
                }

                if ((free_item = find_and_update_req_item(tps_api_req->rawMsgHeader,tps_api_req->hdrLen,sender_port,buf.msgHdr.appPid)) != NULL)                 {
                    goto finish;
                } else {
                    get_free(&req_list,NULL,&free_item,NULL); 

                    if (free_item != NULL) {  
                        strcpy(free_item->raw_msg_header,tps_api_req->rawMsgHeader);
                        free_item->msgUpdateIntvl = tps_api_req->msgUpdateIntvl;
                        free_item->hdrLen = tps_api_req->hdrLen;
                        free_item->msgFilterIdx = msgFilterIdx;
                        msgFilterIdx++;
                        get_free(NULL,free_item,NULL,&free_app_id);
                        if (free_app_id != NULL) {
                            free_app_id->port = sender_port;
                            free_app_id->pid = buf.msgHdr.appPid; 

                        } else {
                            goto no_room;
                        }
                           
                    } else {
no_room:
                        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "RAW MSG registration full\n");
                    }
                }
finish:                
                if ((rc = pthread_mutex_unlock(&tpsRawData_mutex)) != 0)
                {
                    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "TPS RAW Data Reg List Mutex unlock Error (%d)\n", rc);
                }
                break;
          
             case TPS_RAW_MSG_IND:
               
                if ((rc = pthread_mutex_lock(&tpsRawData_mutex)) != 0)
                {
                    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "TPS RAW Data Reg List Mutex Lock Error (%d)\n", rc);
                }
                
                // Note : The received Data is in Host Order endian at this point. 
                sendRawDataInd(&buf, FALSE);
              
                if ((rc = pthread_mutex_unlock(&tpsRawData_mutex)) != 0)
                {
                    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "TPS RAW Data Reg List Mutex unlock Error (%d)\n", rc);
                }
                break;

             case TPS_RAW_MSG_DEREG:
                
                tps_api_req = (tpsRawMsgReq *)buf.mtext;

                if ((rc = pthread_mutex_lock(&tpsRawData_mutex)) != 0)
                {
                    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "TPS RAW Data Reg List Mutex Lock Error (%d)\n", rc);
                }

                find_and_delete_req_item(tps_api_req->rawMsgHeader,tps_api_req->hdrLen,sender_port,buf.msgHdr.appPid);

                if ((rc = pthread_mutex_unlock(&tpsRawData_mutex)) != 0)
                {
                    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "TPS RAW Data Reg List Mutex Unlock Error (%d)\n", rc);
                }
                break;

            case TPS_WHEEL_TICK:
#if defined(ENABLE_WHEELTIX)
                {
                 tpsWheelTickMsg_t *WheelTickMsg = (tpsWheelTickMsg_t *)&buf;
                 uint16_t       len;
                 int32_t        i,j, k;
                 char           tmpbuf[255];
                 char           line[32];
                 static uint8_t rl_last_count;
                 static uint8_t rr_last_count;

                 // Initialize the header information of the UBX msg to send
                 ubxEsfMeas_t ubx_esf_measure =
                 {
                         .header.sync1          = 0xb5,
                         .header.sync2          = 0x62,
                         .header.class          = 0x10,
                         .header.id             = 0x02,
                         .header.payload_size   = 0,
                         .flags                 = 0,
                         .providerID            = 0,
                         .timetag               = 0,
                         .tickData[0].data.val  = 0,
                         .tickData[1].data.val  = 0,
                         .tickData[2].data.val  = 0,
                         .tickData[3].data.val  = 0,
                         .crc                   = 0
                 };

                 ubx_esf_measure.timetag = WheelTickMsg->tick_data.timeTag;

                 // Set the wheel data by the reported ticks
                 for (i = 0, j = 0; i < NUM_WHEELS; i++)
                 {
                     if (WheelTickMsg->tick_data.wheel[i].valid)
                     {
                         ubx_esf_measure.tickData[j].data.bit_wt.direction = (WheelTickMsg->tick_data.wheel[i].dir) ? 1 : 0;

                         switch (i)
                         {
                             case 2:
                                 // Don't accumulate tick count unless it changed
                                 if (WheelTickMsg->tick_data.wheel[i].tick != rl_last_count)
                                 {
                                    rl_accum += (uint8_t)(WheelTickMsg->tick_data.wheel[i].tick - rl_last_count);
                                    rl_last_count = WheelTickMsg->tick_data.wheel[i].tick;
                                 }
                                 ubx_esf_measure.tickData[j].data.bit_wt.datafield = rl_accum;
                                 break;

                             case 3:
                                 // Don't accumulate tick count unless it changed
                                 if (WheelTickMsg->tick_data.wheel[i].tick != rr_last_count)
                                 {
                                    rr_accum += (uint8_t)(WheelTickMsg->tick_data.wheel[i].tick - rr_last_count);
                                    rr_last_count = WheelTickMsg->tick_data.wheel[i].tick;
                                 }
                                 ubx_esf_measure.tickData[j].data.bit_wt.datafield = rr_accum;
                                 break;

                             default:
                                 break;
                         }

                         // The datatype here is from UBlox receiver protocol and identifies the wheel
                         ubx_esf_measure.tickData[j].data.bit_wt.datatype = i + (uint32_t)TPS_LEFT_FRONT;

                         j++;
                     }
                 }

                 // Set the UBX payload length in the message header
                 ////TODO:IS CAST BELOW RIGHT?????? ubx_esf_measure.header.payload_size = (uint32_t)&ubx_esf_measure.tickData[j] - (uint32_t)&ubx_esf_measure.timetag;
                 ubx_esf_measure.header.payload_size = (uint64_t)&ubx_esf_measure.tickData[j] - (uint64_t)&ubx_esf_measure.timetag;

                 // Total packet length including header and crc
                 // J is the number of wheel tick entries
                 len = (uint64_t)(&ubx_esf_measure.tickData[j]) - (uint64_t)(&ubx_esf_measure) + sizeof(ubx_esf_measure.crc);

                 // Log the incoming WheelTickMsg
                 if (log_incoming_wt_msg)
                 {
                     I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "================================================\n");
                     I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "%s WHEEL TICK MESSAGE\n", WheelTickMsg->appAbbrev);
                     I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "================================================\n");
                     for (i = TPS_LF_IDX; i < TPS_NUM_WHEELS; i++)
                     {
                         I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "TimeTag: 0x%08x Wheel: %d, Dir: %d, Tick: %d, Valid: %s\n",
                                 WheelTickMsg->tick_data.timeTag,
                                 i,
                                 WheelTickMsg->tick_data.wheel[i].dir,
                                 WheelTickMsg->tick_data.wheel[i].tick,
                                 WheelTickMsg->tick_data.wheel[i].valid ? "TRUE" : "FALSE");
                     }
                     I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "================================================\n");
                     I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "END of Wheel Tick Message\n");
                     I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "================================================\n");
                 }

                 // Log the UBX message buffer
                 if (log_outgoing_wt_ubx_esf_meas_msg)
                 {
                     I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "================================================\n");
                     I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "UBX ESF Measure Message\n");
                     I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "================================================\n");
                     for (i = TPS_LF_IDX; i < j; i++)
                     {
                         I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "Wheel: %d, Dir: %d, Tick: %d, Type: %d VAL: 0x%08x\n",
                                 i,
                                 ubx_esf_measure.tickData[i].data.bit_wt.direction,
                                 ubx_esf_measure.tickData[i].data.bit_wt.datafield,
                                 ubx_esf_measure.tickData[i].data.bit_wt.datatype,
                                 ubx_esf_measure.tickData[i].data.val
                                 );
                     }
                     I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "================================================\n");
                     I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "END of UBX ESF Measure Message\n");
                     I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "================================================\n");
                 }

                 if (log_outgoing_wt_ubx_packet)
                 {
                     I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "================================================\n");
                     I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "UBX Packet to UBLOX\n");
                     I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "================================================\n");
                     tmpbuf[0] = line[0] = 0;
                     for (k = 0; k < len; k++)
                     {
                         sI2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,line, "%02X \n", (uint8_t)(((uint8_t *)&ubx_esf_measure)[k]));
                         strncat(tmpbuf, line, 5);
                     }
                     I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "%s\n", tmpbuf);
                     I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "================================================\n");
                     I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "END of UBX Packet to UBLOX\n");
                     I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "================================================\n");
                 }


                 // Copy the most recent wheel tick accumulator data to the message buffer.
                 // The 10 mS timer handler will send the message to ublox
                 pthread_rwlock_wrlock(&tps_wt_rwlock);
                 memcpy(&UbxEsfMeasure, &ubx_esf_measure, sizeof(ubxEsfMeas_t));
                 UbxEsfMeasureMsgLen = len;
                 pthread_rwlock_unlock(&tps_wt_rwlock);
                }
                break; //dont fall into default by accident.
#endif /* ENABLE_WHEELTIX */
            case TPS_COLD_START:
                SendCFGRSTRequest();
                break;
            case TPS_STOP_GNSS:
                SendGNSSStopRequest();
                break;
            case TPS_GNSS_HW_STAT:
                SendDumpGNSSHWStatusRequest();
                break;
            case TPS_SET_UBLOX_CFG:
                SendDefaultUbloxCfgs();
                break;
            case TPS_DUMP_UBLOX_CFG:
                DumpUbloxConfigs();
                break;
            default:
                break;
            }
        }
#if defined(MY_UNIT_TEST)
        tpsMainLoop = WFALSE;
#endif
    }
    return 0;
}

/* in playback mode, if sendDataF is true, then the message is 
 * sent to the registered app. This was done to avoid making
 * a copy of this function just to send the message to apps during
 * playback. 
 * KENN: TODO: Why was this an issue?  We get RAW from two
 *   different sources (gps + SR-play), but a simple check of
 *   srRecordMode tells you if you should forward to SR.  Did
 *   they not figure that out, or was there a harder
 *   problem they were trying to solve?
 */
static void sendRawDataInd(tpsMsgBufType *buf, bool_t sendDataF)
{
    int32_t       len;
    tpsRawMsgRes  *raw_data;
    request_item  *req_item = NULL;
    app_id        *cur;
#ifdef ENABLE_SR
    bool_t         sentRecordData = FALSE;
#endif
    char_t        rawMsgHdr[20] = {0};

    raw_data = (tpsRawMsgRes *) buf->mtext;

    // Send to all subscribers
    memcpy(rawMsgHdr, raw_data->msgbuf, NMEA_HDR );
    req_item = find_req_item_by_name(rawMsgHdr, NMEA_HDR); 

    if (req_item != NULL) {

        cur = req_item->app_list_head; 
        // This optimisizes sending the gps data size to the actual message size 
        // which is now approx 100 bytes vs the whole (unfilled) 400 byte structure.

        buf->msgLen = raw_data->msg_length + sizeof(raw_data->msgFilterIdx) 
                                           + sizeof(raw_data->msg_length);

        len = buf->msgLen + sizeof(buf->msgHdr) + sizeof(buf->msgLen);

        while (cur != NULL) {
#if !defined(ENABLE_SR)
            if ((wsuSendData(tpsMainSockFd, cur->port, buf, len)) == FALSE) {
                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "Error wsuSendData to apps RAW IND\n");
            }

            cur = cur->next;
#else
            if (  (NULL                  == shm_sr_ptr)
                ||(SR_PLAYBACK_MODE_CHAR != shm_sr_ptr->sr_mode)) {
                if (sentRecordData == FALSE) {

                    // For TPS Raw Data SR Record mode :
                    // We need to format the data in Network Order (Big endian)
                    // to send to SR for recording.
                    // Data is assumed to be in Host order before getting here.

                    raw_data->msgFilterIdx = hton_uint32(raw_data->msgFilterIdx);
                    raw_data->msg_length   = hton_uint32(raw_data->msg_length);

                    buf->msgHdr.msgId     = hton_uint32(buf->msgHdr.msgId);
                    buf->msgHdr.appPid    = hton_uint32(buf->msgHdr.appPid);
                    buf->msgHdr.errorCode = hton_uint32(buf->msgHdr.errorCode);
                    buf->msgLen           = hton_uint32(buf->msgLen);

                    sendSRMessage(SR_CLIENTID_TPS, TPS_RAW_DATA_V2, (uint8_t*)buf, len);
                    sentRecordData = TRUE;

                    // TPS Raw Data SR Record mode :
                    // Now change it back here, to Host Order, for the app to use. 

                    buf->msgHdr.msgId      = ntoh_uint32(buf->msgHdr.msgId);
                    buf->msgHdr.appPid     = ntoh_uint32(buf->msgHdr.appPid);
                    buf->msgHdr.errorCode  = ntoh_uint32(buf->msgHdr.errorCode);
                    buf->msgLen            = ntoh_uint32(buf->msgLen);

                    raw_data->msgFilterIdx = ntoh_uint32(raw_data->msgFilterIdx);
                    raw_data->msg_length   = ntoh_uint32(raw_data->msg_length);
                }

                if ((wsuSendData(tpsMainSockFd, cur->port, buf, len)) == FALSE) {
                    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "Error wsuSendData to apps RAW IND\n");
                }
            } else {

                if (sendDataF == TRUE) {

                    // TPS Raw Data Playback mode :
                    // The data is assumed to be in Host Order already
                    // so no need to change anything.

                    if ((wsuSendData(tpsMainSockFd, cur->port, buf, len)) == FALSE) {
                        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "Error wsuSendData to apps RAW IND\n");
                    }
                }
            }
            cur = cur->next;
#endif /* ENABLE_SR */
        } /* while */
    } /* if */
}

request_item * find_req_item_by_name (char_t *raw_msg_hdr, int32_t hdrLen)
{
    request_item *cur;

    cur = req_list.head;
    while(cur != NULL) {
        if(strncmp(raw_msg_hdr,cur->raw_msg_header,hdrLen)==0){
            return cur;
        }
        cur=cur->next;
    }
    return NULL;
}

request_item * find_and_update_req_item(char_t *raw_msg_hdr,int32_t hdrLen,int32_t port_num, uint32_t pid)
{
    request_item *cur;
    app_id       *app_cur;

    cur = req_list.head;
    while(cur != NULL) {
        if(strncmp(raw_msg_hdr,cur->raw_msg_header,hdrLen)==0){ 
            //Update socketID and AppID 
            app_cur = cur->app_list_head;
            while(app_cur != NULL) {
                if (app_cur->port == port_num){
                    return NULL;
                }
                app_cur=app_cur->next;
            }

            get_free(NULL,cur,NULL,&app_cur);            
            if (app_cur != NULL) {
                 app_cur->port = port_num;
                 app_cur->pid = pid; 
                 return cur;
            }else{
                return NULL;
            }
        }
        cur=cur->next;
    }
    return NULL;
}

void find_and_delete_req_item(char_t *raw_msg_hdr,int32_t hdrLen, int32_t port_num, uint32_t pid)
{
    request_item  *cur,  *prev;
    app_id        *app_cur, *app_prev;
    
    cur = req_list.head;
    prev = NULL;        /*Silence compiler warning: var may be uninitialized.*/
    while(cur != NULL) {
        if(strncmp(raw_msg_hdr, cur->raw_msg_header,hdrLen)==0){ 
            //delete socketID and AppID 
            app_cur = cur->app_list_head;
            while(app_cur != NULL) {
                if (app_cur->port == port_num){
                    if (app_cur == cur->app_list_head){
                        cur->app_list_head = app_cur->next; 
                    } else {
                        app_prev->next = app_cur->next; 
                    }
                    break;
                }
                app_prev = app_cur;
                app_cur=app_cur->next;
            }

            if (cur->app_list_head == NULL) {
                if (cur == req_list.head) {
                    req_list.head = cur->next;
                } else {
                    prev->next = cur->next;     
                }
                break;
            }
        }
        prev = cur;
        cur=cur->next;
    }
}

void get_free(request_list *ptr_req_list, request_item * ptr_req_item, request_item ** free_item, app_id ** free_app_id)
{
    int32_t      i;
    request_item *cur;
    app_id       *app_cur;

    if (ptr_req_list != NULL) {
        *free_item = NULL;
        cur = ptr_req_list->head;
        for (i=0; i<MAX_SUPPORTED_RAW_MSGS; i++) {
            if (cur != &ptr_req_list->req_item[i]){
                if(i==0) {
                    ptr_req_list->head = &ptr_req_list->req_item[i];
                    ptr_req_list->req_item[i].next = cur;
                } else {
                    ptr_req_list->req_item[i-1].next = &ptr_req_list->req_item[i];
                    ptr_req_list->req_item[i].next = cur; 		
                }
                *free_item = &ptr_req_list->req_item[i];
                return;
            } else {
                cur = cur->next; 		   
            }
        }
    } else if (ptr_req_item != NULL) {
        *free_app_id = NULL; 
        app_cur = ptr_req_item->app_list_head;

        for (i=0; i<TPS_DATA_REG_SIZE; i++) {
            if (app_cur != &ptr_req_item->app_list[i]){
                if(i==0) {
                    ptr_req_item->app_list_head = &ptr_req_item->app_list[i];
                    ptr_req_item->app_list[i].next = app_cur;
                } else {
                    ptr_req_item->app_list[i-1].next = &ptr_req_item->app_list[i];
                    ptr_req_item->app_list[i].next = app_cur; 		
                }
                *free_app_id = &ptr_req_item->app_list[i]; 
                return;
            } else {
                app_cur = app_cur->next; 		   
            }
        }
    }
}

/*
 * @brief  Send an error code to all our subscribers.
 * @note   Called in several places in gps.c to report problems with NMEA parsing or PPS
 */
void tpsErrorReport(tpsResultCodeType errCode)
{
    int32_t ix;

    // Send to subscribers
    for (ix = 0; ix < TPS_DATA_REG_SIZE; ix++)
    {
        if (tpsDataRegList[ix].sender_port > 0)
        {
            tpsSendErrorCode(tpsDataRegList[ix].sender_port, errCode, 0);
        }
    }
#if defined(ENABLE_SR)
    // And send one for SR
    tpsSendErrorCode(0, errCode, 1);
#endif
}


#if 0
/* This signal handler is called periodically to check to see if PID of
   registered application is terminated.  If so, this function will remove
   the PID from the register list.

   Note: This function may not be necessary if socket connection is closed
   from the application and the socket read routine can detect it. */
static void tpsCheckPidTimerHandler(int32_t sig)
{
    int32_t ix;

    for (ix = 0; ix < TPS_DATA_REG_SIZE; ix++)
    {
        if (tpsPidList[ix] != 0)
        {
            //if (isPidTerminated(tpsPidList[ix]))
            //{
            //    removePidFromRegList(tpsPidList[ix]);
            //    tpsPidList[ix] = 0;
            //}
        }
    }
}
#endif

#if defined(ENABLE_SR)

/**
 * @brief This function is for receiving and processing SR playback data.
 */
static void * srPlaybackReceiverThread (void *arg)
{
    int32_t           fd;
    RcvDataType       rcv_data;
    RecordHeaderType  *recordHeader;
    uint8_t           *recordData;
    uint32_t          datalength;
    int32_t           status;
    int32_t           ix;
    static uint8_t    srRecord[MAX_SR_RECORD_SIZE];

    tpsRawMsgRes      *raw_data;

    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "TPS: srPlaybackReceiverThread thread created\n");

    fd = wsuCreateSockServer(TPS_RECV_SR_PORT);   // Was ("/tmp/sr/tps_skt");
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "TPS socket %d created for SR to listen on port %d\n", fd, TPS_RECV_SR_PORT);

    while (tpsMainLoop == TRUE)
    {
        rcv_data.available = FALSE;
        rcv_data.fd = fd;
        rcv_data.data = srRecord;
        rcv_data.size = MAX_SR_RECORD_SIZE;

        if((status = wsuReceiveData(100000, &rcv_data)) < 0)
        {
        	I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Socket Error: wsuReceiveData() rtn=%d (<0), errno=%d[%s] fd=%d\n",status,errno,strerror(errno),rcv_data.fd);
            close(fd);
            fd = wsuCreateSockServer(TPS_RECV_SR_PORT);    // Was ("/tmp/sr/tps_skt");
            continue;
        }

        //  RETRIEVE THE NEXT RECORD HEADER
        if (rcv_data.available == TRUE) 
        {
            recordHeader = (RecordHeaderType*)srRecord;

            // RETRIEVE THE RECORD DATA 
            datalength = recordHeader->totalLength - sizeof(RecordHeaderType);

            if(datalength > 0)
            {
                recordData = srRecord + sizeof(RecordHeaderType);

                ++shm_tps_ptr->debug_cnt_sr_ind_recvs;
                  
                //  Send RECORD DATA to appropriate callbacks
                switch(recordHeader->messageId)
                {
                    case TPS_DATA:
                    case TPS_ERROR:
                    {
#ifdef LITTLE_ENDIAN
                            tpsMsgBufType *msg = (tpsMsgBufType *)recordData;

                            msg->msgHdr.msgId        = ntoh_uint32(msg->msgHdr.msgId);
                            msg->msgHdr.appPid       = ntoh_uint32(msg->msgHdr.appPid);
                            msg->msgHdr.errorCode    = ntoh_uint32(msg->msgHdr.errorCode);
                            msg->msgLen = ntoh_uint32(msg->msgLen);

                            if (recordHeader->messageId == TPS_DATA) {
                                tpsDataType *tpsData = (tpsDataType *)msg->mtext;

                                tpsData->valid       = ntoh_uint32(tpsData->valid);
                                tpsData->latitude    = double_swab(tpsData->latitude);
                                tpsData->longitude   = double_swab(tpsData->longitude);
                                tpsData->altitude    = double_swab(tpsData->altitude);
                                tpsData->groundspeed = double_swab(tpsData->groundspeed);
                                tpsData->course      = double_swab(tpsData->course);
                                tpsData->hdop        = double_swab(tpsData->hdop);
                                tpsData->pdop        = double_swab(tpsData->pdop);
                                tpsData->lat_err     = double_swab(tpsData->lat_err);
                                tpsData->lon_err     = double_swab(tpsData->lon_err);
                                tpsData->alt_err     = double_swab(tpsData->alt_err);
                                tpsData->gps_msec    = double_swab(tpsData->gps_msec);
                                tpsData->diff_age    = double_swab(tpsData->diff_age);
                                tpsData->sol_age     = double_swab(tpsData->sol_age);
                                tpsData->fixquality  = ntoh_uint32(tpsData->fixquality);
                                tpsData->numsats     = ntoh_uint32(tpsData->numsats);
                                tpsData->gps_week    = ntoh_uint32(tpsData->gps_week);
                                tpsData->smjr_err    = double_swab(tpsData->smjr_err);
                                tpsData->smnr_err    = double_swab(tpsData->smnr_err);
                                tpsData->smjr_err_orient = double_swab(tpsData->smjr_err_orient);
                                tpsData->fix_mode    = ntoh_uint32(tpsData->fix_mode);
                            }
#endif
                            for (ix = 0; ix < TPS_DATA_REG_SIZE; ix++)
                            {
                                if (tpsDataRegList[ix].sender_port != -1)
                                {
                                    if ((wsuSendData(tpsMainSockFd, tpsDataRegList[ix].sender_port,
                                         (tpsMsgBufType *)recordData, datalength)) == FALSE)
                                        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "Error wsuSendData to apps IND\n");
                                }
                            }
                        } // case TPS_DATA or TPS_ERROR
                        break;

                    case TPS_RAW_DATA_V2:
                        {
                            // If we are here, then we have to be in SR playback mode.
                            // The previously recorded SR file is always in Big Endian format,
                            // so we want to change it to Host order, to send it
                            // to the apps.

                            // create some pointers so we can manipulate some data.
                            tpsMsgBufType *rawmsg = (tpsMsgBufType *)recordData;
                            raw_data = (tpsRawMsgRes *) rawmsg->mtext;

                            // only these need some Host Order attention
                            rawmsg->msgHdr.msgId     = ntoh_uint32(rawmsg->msgHdr.msgId);
                            rawmsg->msgHdr.appPid    = ntoh_uint32(rawmsg->msgHdr.appPid);
                            rawmsg->msgHdr.errorCode = ntoh_uint32(rawmsg->msgHdr.errorCode);
                            rawmsg->msgLen           = hton_uint32(rawmsg->msgLen);

                            raw_data->msgFilterIdx   = ntoh_uint32(raw_data->msgFilterIdx);
                            raw_data->msg_length     = ntoh_uint32(raw_data->msg_length);

                            sendRawDataInd((tpsMsgBufType *)recordData, TRUE);
                        }
                        break;

                    default:
                        // unexpected message Id
                        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "unexpected TPS Message ID %d\n", recordHeader->messageId);
                        break;
                } // switch(recordHeader->messageId)
            }  // if (datalength > 0)
 
            continue;  // check for another message
        } // if header retrieved
    }

    /* It should not come over here ever */
    close(fd);
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "Returning from srPlaybackReceiverThread thread\n");
    return NULL;
}



/**
 * @brief Sends data to SR to be recorded.
 * @return Returns 0 if successful, -1 if socket send fails, -2 if malloc fails
 */
int32_t sendSRMessage(uint8_t interfaceId, uint8_t messageId, uint8_t *data, uint32_t datalength)
{
    uint8_t          *sr_record;
    RecordHeaderType *sr_hdr;
    uint8_t          *sr_data;
    uint32_t         port;
    struct timespec  utcCurrent;

    /* Check for Record Mode and recording started */
    if (   (NULL                   != shm_sr_ptr)
        && (SR_RECORDING_MODE_CHAR == shm_sr_ptr->sr_mode)
        && (TRUE                   == shm_sr_ptr->recordingON)
       )
    {
       // NOTE: We are mallocing and freeing sr_record  each time instead of
       // using a heap variable because its datalength changes each call.
       sr_record = malloc(sizeof(RecordHeaderType)+datalength);
       if(sr_record == NULL) 
       {
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "sr_record  malloc failed\n");
          return -2;
   	   }

       sr_hdr = (RecordHeaderType*)sr_record;
       sr_data = sr_record + sizeof(RecordHeaderType);

       sr_hdr->totalLength = sizeof(RecordHeaderType) + datalength;
       sr_hdr->messageId = messageId;
       sr_hdr->interfaceId = interfaceId;

       // Generate timestamp
       ////TODO:TOBEREMOVED:clock_gettime(CLOCK_REALTIME, &sr_hdr->timestamp);
       
       clock_gettime(CLOCK_REALTIME, &utcCurrent);
       sr_hdr->timestamp_tv_sec  = utcCurrent.tv_sec;
       sr_hdr->timestamp_tv_nsec = utcCurrent.tv_nsec;

       memcpy(sr_data, data, datalength);

       // Switch between SR_RECV_TPS_PORT and SR_RECV_TPSRAW_PORT based on interfaceId
       if (interfaceId == SR_CLIENTID_TPS) { port = SR_RECV_TPS_PORT; }
       if (interfaceId == SR_CLIENTID_TPSRAW) { port = SR_RECV_TPSRAW_PORT; }

       // Send data to SR
       if (wsuSendData(tpsMainSockFd, port, sr_record, sr_hdr->totalLength) == FALSE)
       {
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME, "SR socket send failure\n");
          free(sr_record);
          return -1;
       }

       free(sr_record);
    }

    return 0;
}
#endif /* ENABLE_SR */
