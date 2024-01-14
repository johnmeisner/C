/*---------------------------------------------------------------------------*/
/*                                                                           */
/*    Filename: tps.h                                                        */
/*     Purpose: Main thread for the Time and Position Service (tps) app      */
/*  Department: DENSO North America Research & Development                   */
/*                                                                           */
/* Copyright (C) 2015 DENSO International America, Inc.                      */
/*                                                                           */
/*     Project: V2X Special Projects                                         */
/* Description: Main functions of the TPS app                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

#ifndef TPS_H
#define TPS_H

#include "tps_types.h"


/************************ Definitions **************************************/
#define TPS_CONFIG      "/rwflash/configs/tps.conf"
#if defined(HEAVY_DUTY)
#define TPS_DEFAULT  TPS_CONFIG
#else
#define TPS_DEFAULT     "/rwflash/default/tps.conf"
#endif

#define TPS_PERSISTENT_VALUES_FILE     "/rwflash/tps_persist.dat"

#define TPS_PERSISTENT_VALUES_FILE_VERSION  1

// At startup, need several sequential valid fixes before saying a good fix has been obtained
#define MIN_SEQUENTIAL_GOOD_FIXES_TO_COUNT_AS_GOOD_FIX_FOUND 3

/* Extern Variables for Logging */
extern bool_t log_ubx_hnr_ins;
extern bool_t log_ubx_hnr_pvt;
extern bool_t log_ubx_nav_pvt;

#if defined(ENABLE_WHEELTIX)
extern bool_t log_ubx_esf_ins;
extern bool_t log_ubx_esf_meas;
extern bool_t log_ubx_esf_status;
extern bool_t log_incoming_wt_msg;
extern bool_t log_outgoing_wt_ubx_esf_meas_msg;
extern bool_t log_outgoing_wt_ubx_packet;

// Wheel tick send to ublox timer
extern uint32_t wheel_tick_timer_ms;
#endif /* ENABLE_WHEELTIX */

/* Function prototype for TPS Main Process */
void tpsMainProcess(bool_t debugModeOn);
void tpsErrorReport(tpsResultCodeType errCode);

/* Saved variables structure */
typedef struct
{
    uint8_t version;
    int8_t  curLeapSecs;
    uint8_t srcOfCurLeapSecs;       //<! UBlox-defined value; which sat system gave current Leap Seconds value (1=Glonass-GPS,etc), */

} savedTpsVariables_t;

#endif
