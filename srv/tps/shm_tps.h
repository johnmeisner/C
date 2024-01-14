/*---------------------------------------------------------------------------*/
/*                                                                           */
/*    Filename: shm_tps.h                                                    */
/*     Purpose: TPS SHM for microsecond-accurate time and debug tracking     */
/*  Department: DENSO North America Research & Development                   */
/*                                                                           */
/* Copyright (C) 2015 DENSO International America, Inc.                      */
/*                                                                           */
/*     Project: V2X Special Projects                                         */
/* Description: Structure containing ClockCycles data from PPS               */
/*                                                                           */
/*---------------------------------------------------------------------------*/


#ifndef _SHM_TPS_H
#define _SHM_TPS_H

#include <sys/time.h>
#include "gps.h"        // For ubxNavDop_t

#define SHM_TPS_PATH  "/shm_tps"
#define MAX_RINV_LENGTH 30
#define MAX_FWVER_LENGTH 10

#if defined(HEAVY_DUTY) /* This probably belongs somewhere else but here for now. */
#define UBX_FW_VERSION_STR "SPG 3.01"
#define UBX_CFG_VERSION_STR   "DENSO RSU-5940 v1.1.0 20220817"
#define UBX_CFG_VERSION_STR_2 "DENSO RSU-5940 v1.0.0 20220805"
#endif

#define TPS_MAX_TRACKED_LARGE_READ_COUNT  100
#define TPS_MAX_TRACKED_USLEEPS_BETWEEN_LARGE_READS 500
#define NUM_TRACKED_USLEEPS_BEFORE_MSGS 10
#define MAX_UBX_INF_MSG_LEN 50

// Structure to publish position
typedef struct {
    double lat[2], lon[2], alt[2];
    uint16_t last_updated_index;
} tps_pos_publish_t;

#if defined (HEAVY_DUTY)
typedef struct {
    ubxNavPvt_t last_PVT[2];
    uint16_t last_updated_pvt_index;
    char_t last_GGA[2][MAX_NMEA_MESSASE_LEN];
    uint16_t last_updated_gga_index;
} tps_geodetic_publish_t;
#endif

// Structure to hold data from UBX-NAV-STATUS messages
typedef struct
{
    uint32_t gps_time_of_week_msec;
    uint8_t  gps_fix_type;
    bool_t   gps_fix_ok; // 1 = position and velocity valid and within DOP and ACC Masks.
    bool_t   differential_corrections_applied;  // 1 = differential corrections were applied
    bool_t   week_number_valid;
    bool_t   time_of_week_valid;
    bool_t   differential_corrections_available;
    bool_t   carrier_solution_valid;
    uint8_t  map_matching_status;
                // mapMatching map matching status:
                //  00: none
                //  01: valid but not used, i.e. map matching data was received, but was too old
                //  10: valid and used, map matching data was applied
                //  11: valid and used, map matching data was applied. In case of sensor unavailability map matching
                //      data enables dead reckoning. This requires map matched latitude/longitude or heading data.
    uint8_t power_save_mode_state;
                // 0: ACQUISITION [or when psm disabled]
                // 1: TRACKING
                // 2: POWER OPTIMIZED TRACKING
                // 3: INACTIVE
    uint8_t spoofing_detection_state;
                // 0: Unknown or deactivated
                // 1: No spoofing indicated
                // 2: Spoofing indicated
                // 3: Multiple spoofing indications
    uint8_t  carrier_phase_range_solution_status;
                // 0: no carrier phase range solution
                // 1: carrier phase range solution with floating ambiguities
                // 2: carrier phase range solution with fixed ambiguities
    uint32_t time_to_first_fix_msec;
    uint32_t time_since_startup_msec;
} ubxNavStatus_t;


// Structure to hold data from UBX-NAV-SAT messages
#define MAX_SAT_GNSSID_VALUE 6
#define NUM_SAT_CNR_RATIOS 26
#define MAX_SAT_QUALITY_INDICATOR_VALUE 7
#define MAX_SAT_HEALTH_VALUE 2
typedef struct
{
    uint32_t gps_time_of_week_msec;
    uint8_t  number_of_satellites;
    uint8_t  cnt_sats_w_gnss_XX[MAX_SAT_GNSSID_VALUE+1];
    uint8_t  cnt_sats_w_carrier_to_noise_ratio_XX_dBHz[NUM_SAT_CNR_RATIOS]; // Not +1 as NUM_ not MAX
    uint8_t  cnt_sats_w_quality_ind_XX[MAX_SAT_QUALITY_INDICATOR_VALUE+1];
    uint8_t  cnt_sats_w_health_XX[MAX_SAT_HEALTH_VALUE+1];
    uint8_t  cnt_sats_w_quality_ind_XX_x_gnss_XX[MAX_SAT_QUALITY_INDICATOR_VALUE+1][MAX_SAT_GNSSID_VALUE+1];
    uint8_t  cnt_sats_w_health_XX_x_gnss_XX[MAX_SAT_HEALTH_VALUE+1][MAX_SAT_GNSSID_VALUE+1];
} ubxNavSat_t;


// Structure to hold one sat's detail ... more detail for searching for a way to predict timeacc lock
typedef struct
{
    uint8_t svId;
    uint8_t CNR;
    uint8_t quality;
    uint8_t health;
    uint8_t svUsed;
    int16_t elev;
    int16_t azim;
    int16_t prRes;
} oneNavSatDetail_t;
#define MAX_DETAIL_SATS 16
typedef struct
{
    uint8_t cnt_gps_sats;
    oneNavSatDetail_t gps[MAX_DETAIL_SATS];
    uint8_t cnt_glo_sats;
    oneNavSatDetail_t glo[MAX_DETAIL_SATS];
} ubxNavSatDetail_t;



/* TPS has its own shared memory */
typedef struct {
    uint32_t index_of_most_recent;
    uint64_t clock_cycles_1;
    struct timeval pps_timeval_1;
    uint64_t clock_cycles_2;
    struct timeval pps_timeval_2;
    uint32_t seqno;

    // Config version string from UBlox RINV storage
    uint8_t  ublox_config_version_str[MAX_RINV_LENGTH+1];
    uint8_t  ublox_firmware_version_str[MAX_FWVER_LENGTH+1];

    // Publishing posn in SHM so available without subscribing nor locking TPS SHM
    tps_pos_publish_t pub_pos;

    // TPS Current Time and Position status
    uint8_t  cur_tps_time_state;
    uint8_t  cur_tps_posn_state;

    // TPS Status counters
    uint32_t debug_gps_cmd_count_nmea;
    uint32_t debug_gps_cmd_count_nmea_unterminated;
    uint32_t debug_gps_cmd_count_nmea_bad_csum;
    uint32_t debug_gps_cmd_count_nmea_gprmc_valid_data;
    uint32_t debug_gps_cmd_count_nmea_gpgga_valid_data;
    uint32_t debug_gps_cmd_count_nmea_gpgsa_valid_data;
    uint32_t debug_gps_cmd_count_nmea_gpgst_w_valid_ellipse_err;
    uint32_t debug_gps_cmd_count_nmea_gpgst_w_estim_ellipse_err;
    uint32_t debug_gps_cmd_count_nmea_gpgst_valid_data;
    uint32_t debug_gps_cmd_count_rtcm;
    uint32_t debug_gps_cmd_count_ubx;
    uint32_t debug_gps_cmd_count_ubx_bad_csum;
    uint32_t debug_gps_cmd_count_ubx_nav_pvt_total;
    uint32_t debug_gps_cmd_count_ubx_nav_pvt_unused;
    uint32_t debug_gps_cmd_count_ubx_nav_pvt_no_fix;
    uint32_t debug_gps_cmd_count_ubx_nav_timels;
    uint32_t debug_gps_cmd_count_ubx_nav_timels_invalid;
    uint32_t debug_gps_cmd_count_ubx_nav_status;
    uint32_t debug_gps_cmd_count_ubx_nav_sat;
    uint32_t debug_gps_cmd_count_ubx_nav_dop;
    uint32_t debug_gps_cmd_count_ubx_nav_eell;
    uint32_t debug_gps_cmd_count_ubx_nav_aopstatus;
    uint32_t debug_gps_cmd_count_ubx_upd_sos;
    uint32_t debug_gps_cmd_count_ubx_hnr_ins;
    uint32_t debug_gps_cmd_count_ubx_hnr_pvt_total;
    uint32_t debug_gps_cmd_count_ubx_hnr_pvt_unused;
    uint32_t debug_gps_cmd_count_ubx_esf_ins;
    uint32_t debug_gps_cmd_count_ubx_esf_raw;
    uint32_t debug_gps_cmd_count_ubx_esf_measure;
    uint32_t debug_gps_cmd_count_ubx_esf_status;
    uint32_t debug_gps_cmd_count_ubx_cfg_rinv;
    uint32_t debug_gps_cmd_count_ubx_mon_ver;
    uint32_t debug_gps_cmd_count_ubx_mga_gps;
    uint32_t debug_gps_cmd_count_ubx_mon_hw;
    uint32_t debug_gps_cmd_count_ubx_ack_ack;
    uint32_t debug_gps_cmd_count_ubx_ack_nak;
    uint32_t debug_gps_cmd_count_ubx_inf_err;
    uint32_t debug_gps_cmd_count_ubx_inf_warn;
    uint32_t debug_gps_cmd_count_ubx_inf_rest;
    uint32_t debug_gps_cmd_count_ubx_ignored;
    uint32_t debug_gps_cmd_count_ubx_other;
    uint32_t debug_gps_cnt_inds_built_from_ubx_w_valid_gst_ellipse_err;
    uint32_t debug_gps_cnt_inds_built_from_ubx_w_valid_eell_ellipse_err;
    uint32_t debug_gps_cnt_inds_built_from_ubx_w_estim_ellipse_err;
    uint32_t debug_cnt_chars_read;
    uint32_t debug_cnt_unused_chars;
    uint32_t debug_buffer_aborts;
    uint32_t debug_cnt_comm_send_tps_data_ind;
    uint32_t debug_cnt_inds_sent_to_tps;
    uint32_t debug_pps_cnt_interrupts;
    uint32_t debug_cnt_spi_empty_usleeps;
    uint32_t debug_cnt_spi_initial_reads;
    uint32_t debug_cnt_spi_large_reads;
    uint32_t debug_cnt_large_reads_between_usleeps[TPS_MAX_TRACKED_LARGE_READ_COUNT+2];
    uint32_t debug_cnt_usleeps_between_large_reads[TPS_MAX_TRACKED_USLEEPS_BETWEEN_LARGE_READS+2];
    uint32_t debug_cnt_empty_usleeps_before_msg[NUM_TRACKED_USLEEPS_BEFORE_MSGS];
#if defined(ENABLE_SR)
    uint32_t debug_cnt_sr_ind_recvs;
    uint32_t debug_cnt_sr_ind_sends;
    uint32_t debug_cnt_sr_raw_recvs;
    uint32_t debug_cnt_sr_raw_sends;
#endif
    // Message that triggered last UBX-ACK-NAK response
    uint8_t  debug_latest_nak_msg_classid;
    uint8_t  debug_latest_nak_msg_msgid;
    // Last UBX-INF msg from UBlox
    uint8_t  debug_gps_last_inf_message[MAX_UBX_INF_MSG_LEN+1];
    // Tracking Time to first GNSS required data
    uint64_t debug_power_up_to_first_fix_acquired_secs;
    uint64_t debug_power_up_to_first_timeacc_acquired_secs;
    uint64_t debug_power_up_to_first_acceptable_timeacc_secs;
    // Time adjustments and PPS timing tracking
    uint32_t debug_cnt_set_time_with_settimeofday;
    uint32_t debug_cnt_set_time_with_clocksettime;
    uint32_t debug_cnt_set_time_with_rtcsettime;
    uint32_t debug_cnt_time_adjustments;
    uint32_t debug_cnt_time_adjustment_fail_mktime;
    uint32_t debug_cnt_time_adjustment_fail_settimeofday;
    uint32_t debug_cnt_time_adjustment_fail_clocksettime;
    uint32_t debug_cnt_time_adjustment_fail_getgmtime;
    uint32_t debug_cnt_time_adjustment_fail_openrtcdevice;
    uint32_t debug_cnt_time_adjustment_fail_rtcsettime;
    uint32_t debug_cnt_timestamp_missing;
    uint32_t debug_cnt_1pps_tps_latency_event;
    uint32_t debug_cnt_1pps_latency_event;
    uint32_t debug_cnt_1pps_fetch_event;
    uint32_t debug_cnt_1pps_halt_event;
    uint32_t debug_cnt_gps_fix_lost;
    uint32_t debug_cnt_gps_time_and_system_time_out_of_sync;
    // Time Accuracy aquisition and loss tracking
    uint32_t debug_cnt_timeacc_lost;
    uint32_t debug_gps_cmd_count_ubx_nav_pvt_no_timeacc;
    // Number of sats used for latest gnss solution
    uint32_t debug_latest_number_of_sats;
    // Tracking of time accuracy
    uint32_t time_accuracy_min_since_first_fix;
    uint32_t time_accuracy_max_since_first_fix;
    uint32_t time_accuracy_min;
    uint32_t time_accuracy_max;
    uint32_t time_accuracy_threshold;
    uint32_t current_time_accuracy;
    uint32_t cnt_timeacc_exceeded_threshold;
    uint32_t cnt_seconds_where_timeacc_exceeded_threshold;
    // For tracking Min, Max, Avg for current minute, hour, day
    uint32_t current_minute_lowest_time_accuracy;
    uint32_t current_minute_highest_time_accuracy;
    long current_minute_total_of_timeacc_samples;
    uint32_t current_minute_num_samples;
    uint32_t current_hour_lowest_time_accuracy;
    uint32_t current_hour_highest_time_accuracy;
    long current_hour_total_of_timeacc_samples;
    uint32_t current_hour_num_samples;
    uint32_t current_day_lowest_time_accuracy;
    uint32_t current_day_highest_time_accuracy;
    long current_day_total_of_timeacc_samples;
    uint32_t current_day_num_samples;
    // Historical records
    #define NUM_MINS_TRACKED_TIME_ACCURACY 60
    #define NUM_HOURS_TRACKED_TIME_ACCURACY 24
    #define NUM_DAYS_TRACKED_TIME_ACCURACY 7
    uint32_t time_accuracy_min_in_prev_minute[NUM_MINS_TRACKED_TIME_ACCURACY];
    uint32_t time_accuracy_max_in_prev_minute[NUM_MINS_TRACKED_TIME_ACCURACY];
    uint32_t time_accuracy_avg_in_prev_minute[NUM_MINS_TRACKED_TIME_ACCURACY];
    uint32_t index_for_latest_prev_minute_data;
    uint32_t uptime_minutes_for_prev_minute[NUM_MINS_TRACKED_TIME_ACCURACY];
    uint32_t time_accuracy_min_in_prev_hour[NUM_HOURS_TRACKED_TIME_ACCURACY];
    uint32_t time_accuracy_max_in_prev_hour[NUM_HOURS_TRACKED_TIME_ACCURACY];
    uint32_t time_accuracy_avg_in_prev_hour[NUM_HOURS_TRACKED_TIME_ACCURACY];
    uint32_t index_for_latest_prev_hour_data;
    uint32_t uptime_hours_for_prev_hour[NUM_HOURS_TRACKED_TIME_ACCURACY];
    uint32_t time_accuracy_min_in_prev_day[NUM_DAYS_TRACKED_TIME_ACCURACY];
    uint32_t time_accuracy_max_in_prev_day[NUM_DAYS_TRACKED_TIME_ACCURACY];
    uint32_t time_accuracy_avg_in_prev_day[NUM_DAYS_TRACKED_TIME_ACCURACY];
    uint32_t index_for_latest_prev_day_data;
    uint32_t uptime_days_for_prev_day[NUM_DAYS_TRACKED_TIME_ACCURACY];
    // Record of time sets/adjustments
    #define NUM_HOURS_TRACKED_TIME_CORRECTIONS 24
    #define NUM_DAYS_TRACKED_TIME_CORRECTIONS 10
    uint32_t index_for_latest_timefix_prev_hour_data;
    uint32_t index_for_latest_timefix_prev_day_data;
    uint32_t cnt_set_times_in_prev_hour[NUM_HOURS_TRACKED_TIME_CORRECTIONS];
    uint32_t cnt_time_adjustments_in_prev_hour[NUM_HOURS_TRACKED_TIME_CORRECTIONS];
    uint32_t cnt_set_times_in_prev_day[NUM_DAYS_TRACKED_TIME_CORRECTIONS];
    uint32_t cnt_time_adjustments_in_prev_day[NUM_DAYS_TRACKED_TIME_CORRECTIONS];
    uint32_t uptime_hours_for_timefix_prev_hour[NUM_HOURS_TRACKED_TIME_ACCURACY];   // For which hour is this set of [index] data
    uint32_t uptime_days_for_timefix_prev_day[NUM_HOURS_TRACKED_TIME_ACCURACY]; // For which day is this set of [index] data
    // Time sets/adjustments tracker variables that used to be in the
    //   metrics thread, but are exposed now so reporting can show
    //   the current-but-incomplete hour/day's values so far.
    uint32_t prev_day_sets_count;
    uint32_t prev_day_adjs_count;
    uint32_t prev_hour_sets_count;
    uint32_t prev_hour_adjs_count;
    // Record of PPS interrupt counts
    #define NUM_HOURS_TRACKED_PPS_INTERRUPTS 24
    uint32_t prev_hour_pps_cnt;
    uint32_t index_for_latest_pps_prev_hour_data;
    uint32_t track_pps_interrupts_per_hour[NUM_HOURS_TRACKED_PPS_INTERRUPTS];
    uint32_t uptime_hours_for_pps_prev_hour[NUM_HOURS_TRACKED_PPS_INTERRUPTS];  // The hour of this set of [index] data
    // Record of GNSS position fix losses - last N times gps fix was lost
    #define NUM_TRACKED_LOST_FIXES 20
    uint64_t current_fix_loss_started_at_uptime;
    uint32_t current_lost_fix_period_length;
    uint32_t latest_lost_fix_index;
    uint32_t tracked_lost_fix_is_loss_number[NUM_TRACKED_LOST_FIXES];
    uint32_t tracked_lost_fix_period_length[NUM_TRACKED_LOST_FIXES];
    uint64_t tracked_lost_fix_start_uptime[NUM_TRACKED_LOST_FIXES];
    uint64_t tracked_lost_fix_end_uptime[NUM_TRACKED_LOST_FIXES];
    // Record of GNSS time accuracy losses - last N times gps fix was lost
    uint64_t current_timeacc_loss_started_at_uptime;
    uint32_t current_lost_timeacc_period_length;
    uint32_t latest_lost_timeacc_index;
    uint32_t tracked_lost_timeacc_is_loss_number[NUM_TRACKED_LOST_FIXES];
    uint32_t tracked_lost_timeacc_period_length[NUM_TRACKED_LOST_FIXES];
    uint64_t tracked_lost_timeacc_start_uptime[NUM_TRACKED_LOST_FIXES];
    uint64_t tracked_lost_timeacc_end_uptime[NUM_TRACKED_LOST_FIXES];
    // ESF values
    uint32_t  debug_esf_iTOW;
    double    debug_esf_x_ang_rate;
    double    debug_esf_y_ang_rate;
    double    debug_esf_z_ang_rate;
    double    debug_esf_x_accel;
    double    debug_esf_y_accel;
    double    debug_esf_z_accel;
    double    debug_esf_temp;
    uint32_t  debug_esf_x_ang_rate_hex;
    uint32_t  debug_esf_y_ang_rate_hex;
    uint32_t  debug_esf_z_ang_rate_hex;
    uint32_t  debug_esf_x_accel_hex;
    uint32_t  debug_esf_y_accel_hex;
    uint32_t  debug_esf_z_accel_hex;
    uint32_t  debug_esf_temp_hex;

    // LeapSecond Values
    uint8_t  leap_secs_valid;            /* Boolean - leap seconds are valid */
    int8_t   curLeapSecs;                /* Current Leap Seconds between GPS time & UTC Time */
    uint8_t  srcOfCurLeapSecs;           /* UBlox-defined, which sat system gave current Leap Seconds value */
    int8_t   upcomingLeapSecChange;      /* Upcoming leap second change if scheduled (+1 or -1) */
    int32_t  timeToLeapSecEvent_secs;    /* Time in seconds until next leap second change, if scheduled */
    uint8_t  tpsSrcOfCurLeapsecs;        /* 0 = curLeapSecs was read from ublox, 1 = was read from file, 2 - local cache */
    uint32_t debug_gps_count_leapsec_src_ublox;     /* Count of times TpsDataInd used Leapseconds live from ublox */
    uint32_t debug_gps_count_leapsec_src_cache;     /* Count of times TpsDataInd used Leapseconds live from ublox */
    uint32_t debug_gps_count_leapsec_src_persist;   /* Count of times TpsDataInd used Leapseconds read from persistency file */

    // Precision data from ubx-nav-dop
    ubxNavDop_t nav_dop;

    // Status data from ubx-nav-status
    ubxNavStatus_t nav_status;

    // Status data from ubx-nav-sat
    ubxNavSat_t nav_sat;
    
    // Uptime at last nav-pvt reception, used for uptime value in csv logging
    uint32_t uptime_at_nav_pvt;

    // Detail on GPS and GLONASS sats
    ubxNavSatDetail_t sat_detail;

    // UBX-MON-HW fields of interest
    uint8_t mon_hw_rtc_is_calibrated;
    uint16_t mon_hw_noise_per_ms;
    uint16_t mon_hw_agc_count;
    uint8_t mon_hw_jamming_ind;
    uint8_t mon_hw_jamming_status;
#if defined(HEAVY_DUTY)
    tps_geodetic_publish_t pub_geodetic;
#endif

#define GPS_SERIAL_CONFIG_CMD_ERROR  0x1
#define GPS_SERIAL_OPEN_ERROR        0x2
#define GPS_MUTEX_INIT_ERROR         0x4
#define GPS_MSG_HALT_ERROR           0x10
#define GPS_FIND_1PPS_ERROR          0x20
#define GPS_1PPS_THREAD_EXIT         0x40
#define GPS_COMM_THREAD_EXIT         0x80
#define TPS_SAVE_PERSISTENT_ERROR    0x100
#define GPS_READ_ERROR               0x200
#define GPS_TIME_SOURCE_ERROR        0x400
#define GPS_OPEN_1PPS_ERROR          0x800
#define GPS_CREATE_1PPS_ERROR        0x1000
#define GPS_CAPABILITIES_1PPS_ERROR  0x2000
#define GPS_GET_PARAMS_1PPS_ERROR    0x4000
#define GPS_SET_PARAMS_1PPS_ERROR    0x8000
#define GPS_1PPS_MODE_BAD_ERROR      0x10000
#define GPS_TIME_ACCURACY_ERROR      0x20000
    uint32_t error_states; /* So RSUHEALTH can see what's going on. */
} shm_tps_t;

/* Enum for the various sources of leapsecond info - used above, in tpsSrcOfCurLeapsecs */
typedef enum
{
    LSSRC_NONE,
    LSSRC_UBLOX,
    LSSRC_CACHED,
    LSSRC_SAVED_DATA,
} tpsLeapsecSource_e;

/* Enum for the stages of TPS Time Synchonization (TTS) */
typedef enum {
    TTS_UNINITIALIZED,  // -- TPS crashed at startup or is still starting up
    TTS_NO_SYNC,        // -- TPS has started but date is in 1970 ?
    TTS_RTC_SYNC,       // -- If RTC set system time at boot ... ?
    TTS_NOLEAP_SYNC,    // -- If getting PVT time but no TIMELS leapsec (and no /rwflash stored leapsec)
    TTS_GPS_SYNC        // -- When gps.c calls settimeofday() *AND* we have a valid TIMELS leapsec
} tpsTimeSyncState_e;

/* Enum for the stages of TPS GPS Position (TGP) */
typedef enum {
    TGP_UNINITIALIZED,
    TGP_NO_POSITION,    // -- TPS started
    TGP_GPS_POSITION    // -- When get valid PVT position
} tpsPositionSyncState_e;


#endif
