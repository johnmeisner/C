/*---------------------------------------------------------------------------*/
/*                                                                           */
/*    Filename: gps.h                                                        */
/*     Purpose: GPS Functions for the Time and Position Service (tps) app    */
/*  Department: DENSO North America Research & Development                   */
/*                                                                           */
/* Copyright (C) 2015 DENSO International America, Inc.                      */
/*                                                                           */
/*     Project: V2X Special Projects                                         */
/* Description: GPS-related functions of TPS                                 */
/*                                                                           */
/*---------------------------------------------------------------------------*/

#ifndef GPS_H
#define GPS_H

#include "tps_msg.h"  // for MAX_RAW_MSG_HDR_LEN

/* Max number of processes who can register for TPS Data Indications */
#define TPS_DATA_REG_SIZE    	4
#define TPS_SOCKRECV_TIMEOUT	2000000     /* In microseconds. */
#define MAX_SUPPORTED_RAW_MSGS 20


#define NMEA_MSG_TIME_STRNG_LENGTH 12 
#define NMEA_MSG_DATE_STRNG_LENGTH 8


/* The largest NMEA-0183 message is 82 bytes.  Round up to an even 100. */
#define MAX_NMEA_MESSASE_LEN       100
#define NMEA_MSG_MAX_TOKENS        20
#define NMEA_MSG_MAX_TOKEN_LENGTH  20

#define GPS_CONFIGURED       1 
#define USECS_PER_SEC        1000000

#define MAX_RTCM_DATA_LEN    200    // 101 for RTCM1001, 25 for RTCM1005
#define RTCM_CRC_LEN         4

#define MAX_GPS_MESSAGE_LEN  4000 /*4000 TEMP - see if it fixes 1-sat in NAV-SAT!! */ /* The largest NMEA-0183 message is 82 bytes.  Largest RTCM is 101 bytes.  Largest UBX is 580ish */


extern uint8_t  tps_use_wheel_ticks;
extern uint8_t  nav_pvt_sha256_enable;
extern uint8_t  hnr_pvt_sha256_enable;


/**
 * @enum ESF Type Definitions for decoding ESF_MEASURE or ESF_RAW
 */
typedef enum
{
    ESF_NONE = 0,
    ESF_RSV1,
    ESF_RSV2,
    ESF_RSV3,
    ESF_RSV4,
    ESF_GYRO_Z,
    ESF_FRONT_LEFT_WT,
    ESF_FRONT_RIGHT_WT,
    ESF_REAR_LEFT_WT,
    ESF_REAR_RIGHT_WT,
    ESF_SPEED_TICK,
    ESF_SPEED,
    ESF_GYRO_TEMP,
    ESF_GYRO_Y,
    ESF_GYRO_X,
    ESF_NOT_USED,
    ESF_ACCEL_X,
    ESF_ACCEL_Y,
    ESF_ACCEL_Z
} esfDataType_t;

typedef enum
{
    NO_FIX = 0,
    DR_ONLY,
    FIX_2D,
    FIX_3D,
    GPS_DR_COMBINED,
    TIME_ONLY_FIX
} gpsFixType_t;



typedef struct
{
    char_t    time[NMEA_MSG_TIME_STRNG_LENGTH];
    float64_t latitude;
    int8_t    latNS;
    float64_t longitude;
    int8_t    longEW;
    int32_t   fixQuality;
    int32_t   numSats;
    float64_t hDop;
    float64_t altitude;
    int8_t    altUnits;
    float64_t hogae;
    int8_t    hogaeUnits;
    int32_t   diffAge;
    uint64_t  debug_recv_cpu_cycles;
    uint64_t  debug_recv_time_ms;
} gpsGGA_t;

typedef struct
{
    //float64_t time;
    char_t    time[NMEA_MSG_TIME_STRNG_LENGTH];
    int8_t    dataStatus;
    float64_t latitude;
    int8_t    latNS;
    float64_t longitude;
    int8_t    longEW;
    float64_t groundspeed;
    float64_t course;
    char_t    date[NMEA_MSG_DATE_STRNG_LENGTH];
    uint64_t  debug_recv_cpu_cycles;
    uint64_t  debug_recv_time_ms;
} gpsRMC_t;

typedef struct __gpsGPGST_t__
{
    char_t    time[NMEA_MSG_TIME_STRNG_LENGTH];
    float64_t rms;
    float64_t smjr_err;
    float64_t smnr_err;
    float64_t orient;
    float64_t lat_err;
    float64_t lon_err;
    float64_t alt_err;
} gpsGPGST_t;

typedef struct __gpsGPGSA_t__
{
    int32_t   fix_mode;
    float64_t pdop;
    float64_t hdop;
    float64_t vdop;
} gpsGPGSA_t;


typedef struct
{
    int8_t  curLeapSecs;
    uint8_t srcOfCurLeapSecs;
    int8_t  upcomingLeapSecChange;
    int32_t timeToLeapSecEvent_secs;
    uint8_t srcOfUpcomingLeapSecChange;
} ubxNavTimeLS_t;


typedef struct __attribute__((__packed__))
{
    uint32_t time_of_week_msec;
    uint16_t year_utc;
    uint8_t  month_utc;
    uint8_t  day_utc;
    uint8_t  hour_utc;
    uint8_t  min_utc;
    uint8_t  sec_utc;
    int32_t  nsec_utc;
    uint32_t time_accuracy_est_nsec;
    uint8_t  fix_type;
    uint8_t  flag_gnss_fix_ok; 
    uint8_t  flag_heading_valid;
    uint8_t  flag_differential_corrections_applied;
    uint8_t  flag_utc_fully_resolved;
    uint8_t  num_sats;
    int32_t  lat_deg_e7;
    int32_t  lon_deg_e7;
    int32_t  height_above_elipsoid_mm;
    int32_t  height_above_MSL_mm;
    uint32_t horiz_accuracy_est_mm;
    uint32_t vert_accuracy_est_mm;
    int32_t  velocity_north_mmsec;
    int32_t  velocity_east_mmsec;
    int32_t  velocity_down_mmsec;
    int32_t  ground_speed_mmsec;
    uint32_t speed_accuracy_est_mmsec;
    int32_t  heading_of_motion_deg_e5;
    int32_t  heading_of_vehicle_deg_e5;
    uint32_t heading_accuracy_est_deg_e5;
    uint16_t position_dilut_of_precision;
    uint64_t debug_recv_cpu_cycles;
    uint64_t debug_recv_time_ms;
} ubxNavPvt_t;


typedef struct
{
    uint16_t gDOP;  // Geometric DOP
    uint16_t pDOP;  // Position DOP
    uint16_t tDOP;  // Time DOP
    uint16_t vDOP;  // Vertical DOP
    uint16_t hDOP;  // Horizontal DOP
    uint16_t nDOP;  // Northing DOP
    uint16_t eDOP;  // Easting DOP
} ubxNavDop_t;


typedef struct
{
    uint16_t errEllipseOrient;
    uint32_t errEllipseMajor;
    uint32_t errEllipseMinor;
} ubxNavEell_t;

typedef struct __attribute__((__packed__))
{
    uint8_t     sync1;
    uint8_t     sync2;
    uint8_t     class;
    uint8_t     id;
    uint16_t    payload_size;

} ubxHeader_t;

typedef struct __attribute__((__packed__))
{
    union
    {
        uint32_t    val;

        struct
        {
            uint32_t datafield: 23;
            uint32_t direction: 1;
            uint32_t datatype:  6;
            uint32_t unused: 2;
        } bit_wt;

        struct
        {
            int32_t datafield: 24;
            int32_t datatype:  6;
            int32_t unused: 2;
        } bit;
    } data;

} ubxEsfData_t;

typedef struct __attribute__((__packed__))
{
    union
    {
        uint16_t    val;

        struct
        {
            uint32_t time_mark_sent: 2;
            uint32_t time_mark_edge:  1;
            uint32_t calib_tag_valid: 1;
        } bit;

    } data;

} ubxEsfCalibTag_t;


typedef struct __attribute__((__packed__))
{
    ubxHeader_t     header;
    uint32_t        timetag;
    uint16_t        flags;
    uint16_t        providerID;
    ubxEsfData_t    tickData[4];
    uint32_t        calibTag[4];
    uint16_t        crc;

} ubxEsfMeas_t;

typedef struct __attribute__((__packed__))
{
    uint32_t valid_bits;
    uint8_t  reserved[4];
    uint32_t time_of_week_msec;
    int32_t  x_ang_rate_deg_sec_e3;
    int32_t  y_ang_rate_deg_sec_e3;
    int32_t  z_ang_rate_deg_sec_e3;
    int32_t  x_accel_mg;
    int32_t  y_accel_mg;
    int32_t  z_accel_mg;
    uint8_t  flag_ang_rates_valid;
    uint8_t  flag_accels_valid;
} ubxEsfIns_t;

typedef struct __attribute__((__packed__))
{
    int32_t  x_ang_rate_deg_sec_2pow12;
    int32_t  y_ang_rate_deg_sec_2pow12;
    int32_t  z_ang_rate_deg_sec_2pow12;
    int32_t  x_accel_mg;
    int32_t  y_accel_mg;
    int32_t  z_accel_mg;
    int32_t  gyro_temp_deg_e2;
    uint32_t most_recent_time_of_week_msec;
    uint32_t x_ang_rate_time_of_week_msec;
    uint32_t y_ang_rate_time_of_week_msec;
    uint32_t z_ang_rate_time_of_week_msec;
    uint32_t x_accel_time_of_week_msec;
    uint32_t y_accel_time_of_week_msec;
    uint32_t z_accel_time_of_week_msec;
    uint32_t gyro_temp_time_of_week_msec;
} ubxEsfRaw_t;

typedef struct __attribute__((__packed__))
{
    uint32_t valid_bits;
    uint8_t  reserved[4];
    uint32_t time_of_week_msec;
    int32_t  x_ang_rate_deg_sec_e3;
    int32_t  y_ang_rate_deg_sec_e3;
    int32_t  z_ang_rate_deg_sec_e3;
    int32_t  x_accel_mg;
    int32_t  y_accel_mg;
    int32_t  z_accel_mg;
} ubxHnrIns_t;

typedef struct __attribute__((__packed__))
{
    ubxHeader_t Header;
    uint32_t    time_of_week_msec;
    uint16_t    year_utc;
    uint8_t     month_utc;
    uint8_t     day_utc;
    uint8_t     hour_utc;
    uint8_t     min_utc;
    uint8_t     sec_utc;
    uint8_t     valid_flags;
    int32_t     nsec_utc;
    uint8_t     fix_type;
    uint8_t     fix_status_flags;
    uint8_t     reserved1[2];
    int32_t     lon_deg_e7;
    int32_t     lat_deg_e7;
    int32_t     height_above_elipsoid_mm;
    int32_t     height_above_MSL_mm;
    int32_t     ground_speed_mmsec;
    int32_t     speed_3d_mmsec;
    int32_t     heading_of_motion_deg_e5;
    int32_t     heading_of_vehicle_deg_e5;
    uint32_t    horiz_accuracy_est_mm;
    uint32_t    vert_accuracy_est_mm;
    uint32_t    speed_accuracy_est_mmsec;
    uint32_t    heading_accuracy_est_deg_e5;
    uint8_t     reserved2[4];
    uint8_t     crc[2];
} ubxHnrPvt_t;


typedef struct __attribute__((__packed__))
{
    ubxHeader_t  header;
    uint8_t      version;
    uint8_t      reserved[3];
    uint8_t      classID;
    uint8_t      messageID;
    uint16_t     checksum;
    uint8_t      hash[32];
    uint16_t     crc;
} ubxSecSign_t;



typedef struct __gpsBestPos_t__
{
    uint32_t  gps_week;
    float64_t gps_sec;
    float64_t sol_age;
} gpsBestPos_t;


typedef struct app_id_t app_id;
struct app_id_t {
    int32_t  port;    /* This is the app's sender_port, pulled from the app's DGRAM messages */
    uint32_t pid;
    app_id   *next;
};

typedef struct request_item_t request_item;
struct request_item_t{
    int32_t      hdrLen;
    char_t       raw_msg_header[MAX_RAW_MSG_HDR_LEN];
    int32_t      msgUpdateIntvl;
    uint32_t     msgFilterIdx;
    request_item *next;
    app_id       app_list[TPS_DATA_REG_SIZE];
    app_id       *app_list_head;
};

typedef struct request_list_t request_list;
struct request_list_t{
	request_item  req_item[MAX_SUPPORTED_RAW_MSGS];
	request_item  *head;
};


void *      gpsCommThreadFunc(void* arg);
void *      gpsOutputCheckThreadFunc(void *arg);
void *      gpsGetPPStimeThreadFunc(void *arg);
void *      gpsAssistNowBackupThreadFunc(void *arg);

int32_t     I4Convert(uint8_t *buff);
int32_t     I3Convert(uint8_t *buff);
uint32_t    U4Convert(uint8_t *buff);
uint16_t    U2Convert(uint8_t *buff);
int32_t     SendMsgToUblox(uint8_t *, uint16_t);
bool_t      TpsWheelTickInit(void);

#define COMMA                       ','
#define SEMI_COLON                  ';'
#define OEM_HDR_LOG_LEN             9

#endif
