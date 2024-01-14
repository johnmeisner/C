/*---------------------------------------------------------------------------*/
/*                                                                           */
/* Filename: tps_types.h                                                     */
/*                                                                           */
/* Copyright (C) 2019 DENSO International America, Inc.                      */
/*                                                                           */
/* Description: Define data structure used to pass parsed tps data,          */
/*              plus debug flag #defines                                     */
/*                                                                           */
/*---------------------------------------------------------------------------*/


/*********** BEGIN  WSU/miniWSU Specific Definitions **********/
#ifndef TPS_TYPES_H
#define TPS_TYPES_H

#include "dn_types.h"
#include "v2x_common.h"    // For MAX_TPS_(DATE,TIME)_BYTES
#include "tps_msg.h"

/* Bit Masks for various TPS Debug Messages */
#define DBG_OUTPUT_DISABLE  0x00
#define DBG_OUTPUT_NMEA     0x01
#define DBG_OUTPUT_TIME     0x02
#define DBG_OUTPUT_RTCM     0x04
#define DBG_OUTPUT_RAW_MSG  0x08
#define DBG_OUTPUT_ADJTIME  0x10
#define DBG_OUTPUT_PPS      0x20
#define DBG_OUTPUT_TPSGEN   0x40
#define DBG_OUTPUT_SR       0x80
#define DBG_OUTPUT_ALL      0xffffffff

/* TPS Data Type */
#define MAX_TPS_TIME_BYTES  12
#define MAX_TPS_DATE_BYTES  8

#define UTC_STR_FRMT                "%d%m%y%H%M%S"


/* Type definition of Result Codes, returned by the APIs */
typedef enum {
    TPS_SUCCESS         = 0,    /* Success */
    TPS_FAILURE         = (-1),  /* Failure */
    TPS_CONN_LOST       = (-2),
    TPS_REG_LIST_FULL   = (-3),
    GPS_WRITE_ERROR     = (-4),
    GPS_READ_ERROR_EXIT = (-5),
    GPS_NMEA_CKSUM_ERROR = (-6),
    GPS_NMEA_MSG_PARSE_ERROR = (-7),
    GPS_NO_NMEA_OUTPUT       = (-8),
    GPS_NO_PPS_OUTPUT        = (-9),
    GPS_SUCCESS_PPS_OUTPUT   = (-10),
    GPS_UBX_CKSUM_ERROR      = (-11)
} tpsResultCodeType;


/**
 * @enum Wheel definitions for sending wheel ticks to U-Blox
 */
#if defined(ENABLE_WHEELTIX)
#define NUM_WHEELS          4
typedef enum
{
    TPS_LF_IDX = 0,         //!< Left Front Wheel ID
    TPS_RF_IDX,             //!< Right Front Wheel ID
    TPS_LR_IDX,             //!< Left Rear Wheel ID
    TPS_RR_IDX,             //!< Right Rear Wheel ID

    TPS_NUM_WHEELS = 4,

    TPS_LEFT_FRONT = 6,     //!< Left Front Wheel ID
    TPS_RIGHT_FRONT,        //!< Right Front Wheel ID
    TPS_LEFT_REAR,          //!< Left Rear Wheel ID
    TPS_RIGHT_REAR,         //!< Right Rear Wheel ID

} tpsWheelID_t;


/**
 * @enum Definition of the wheel rotation direction
 */
typedef enum
{
    FORWARD = 0,
    BACK

} tpsWheelDir_t;

/**
 * @struct  Structure binding for wheel ID and tick counter
 * @brief   Wheel tick data struct to bind ticks to wheel ID
 *
 * @var     id      - The wheel identifier
 * @var     tick    - The wheel's tick counter
 */
typedef struct
{
    uint8_t         valid;  //!< Is this field populated with a vlue?
    tpsWheelDir_t   dir;    //!< The wheel rotation direction
    uint32_t        tick;   //!< The current tick counter for this wheel

} tpsWheelTick_t;

/**
 * @struct  tpsWheelTickData_t
 * @brief   Data structure containing all wheel tick data for this interval
 *
 * @var     dir     - The direction of travel of the wheel.
 * @var     wheel   - Array of all wheel's tick data
 */
typedef struct
{
    uint32_t        timeTag;                //!< This member variable is filled out byt the API call.
    tpsWheelTick_t  wheel[TPS_NUM_WHEELS];  //!< WHeel tick data

} WheelTickData_t;

/**
 * @struct  tpsWheelTickData_t
 * @brief   Data structure containing all wheel tick data for this interval
 *
 * @var     dir     - The direction of travel of the wheel.
 * @var     wheel   - Array of all wheel's tick data
 */
typedef struct
{
    msgHeader_t msgHdr;
    char_t      appAbbrev[APP_ABBREV_SIZE];
    int32_t     msgLen;

    WheelTickData_t  tick_data;  //!< Wheel tick data

} tpsWheelTickMsg_t;
#endif /* ENABLE_WHEELTIX */

// Enumerations for gps_data_source_message_type
#define GPS_DATA_SOURCE_MESSAGE_TYPE_NMEA 1
#define GPS_DATA_SOURCE_MESSAGE_TYPE_UBX  2

typedef struct {
  int32_t valid;         /* Indicates that this TPS data is valid */
  char_t time[MAX_TPS_TIME_BYTES];      /* Null terminated string that contains GPS (UTC) Time in HHMMSS.XX format (i.e. 235535.34). */
  char_t date[MAX_TPS_DATE_BYTES];      /* Null terminated string that contains GPS Date in DDMMYY format (i.e. 120307->3/12/07) */
  float64_t latitude;      /* i.e. 33.079996 deg */
  float64_t longitude;     /* i.e. -117.136489 deg */
  float64_t altitude;      /* Height Above Ellipsoid (i.e. 131.0) */
  float64_t groundspeed;   /* Speed over the ground in m/s (converted from knots by TPS) */
  float64_t course;        /* Track angle in degrees true (Course Made Good, degrees true) */
                        /* course is in 0-360 degress from north */
  float64_t hdop;          /* Horizontal dilution of precision  */
  float64_t pdop;          /* Position dilution of precision */ 
  float64_t lat_err;       /* Standard deviation of latitude error (m) x.x */
  float64_t lon_err;       /* Standard deviation of longitude error (m) x.x  */
  float64_t gps_msec;      /* TBD */
  float64_t diff_age;      /* Age of Differential GPS data (in seconds)        */
  float64_t sol_age;       /* diff in seconds between system time and GPS info time*/
  int32_t fixquality;    /* GPS Quality indicator
                           0 = fix not available or invalid
                           1 = GPS fix
                           2 = C/A differential GPS, OmniSTAR HP,
                                OmniSTAR XP, OmniSTAR VBS,
                                or CDGPS
                           4 = RTK fixed ambiguity solution (RT2)
                           5 = RTK floating ambiguity solution (RT20),
                                OmniSTAR HP or OmniSTAR XP
                           6 = Dead reckoning mode
                           7 = Manual input mode (fixed position)
                           8 = Simulator mode
                           9 = WAAS
                           */
  int32_t numsats;       /* Number of satellites in use (00-12).
                           May be different to the number in view   */ 
  int32_t gps_week;      /* TBD */

  float64_t smjr_err;          /* Standard deviation of semi-major axis of error ellipse (m) x.x */
  float64_t smnr_err;          /* Standard deviation of semi-minor axis of error ellipse (m) x.x */
  float64_t smjr_err_orient;   /* Orientation of semi-major axis of error ellipse (Degrees from True north) */
  int32_t fix_mode;      /* Mode of GPS fix
                           1 = fix not available
                           2 = 2D fix
                           3 = 3D fix */
  float64_t alt_err;       /* Standard deviation of altitude error (m) x.x */
  uint8_t  gps_data_source_message_type;   /* 1=NMEA 2=UBX */
  // THE FOLLOWING Fields are only available when gps_data_source_message_type == 2 (UBX)
  uint16_t year_utc;
  uint8_t  month_utc;
  uint8_t  day_utc;
  uint8_t  hour_utc;
  uint8_t  min_utc;
  uint8_t  sec_utc;
  int32_t  nsec_utc;
  int8_t   curLeapSecs;                /* Current Leap Seconds between GPS time & UTC Time */
  uint8_t  srcOfCurLeapSecs;           /* Source of current Leap Seconds value (0=Firmware Hardcoded, 1=From comparing GPS vs GLONASS, 2=GPS, 3=SBAS, 4=BeiDou, 5=Galileo, 6=Aided data, 7=Configured, 255=Unknown) */
  int8_t   upcomingLeapSecChange;      /* Upcoming leap second change if scheduled (+1 or -1) */
  int32_t  timeToLeapSecEvent_secs;    /* Time in seconds until next leap second change, if scheduled */
  uint8_t  ubx_fix_type;               /* FixType from PVT message */
  uint8_t  flag_utc_fully_resolved;    /* UTC Time of Day has been fully resolved (no seconds uncertainty) */
  uint8_t  flag_differential_corrections_applied; /* diffSoln bit from PVT message's flags-bitfield byte */
#if defined(ENABLE_WHEELTIX)
  uint32_t  lf_wheel_tick_count;        //!< LF Wheel tick count sent from vehicle
  uint32_t  rf_wheel_tick_count;        //!< RF Wheel tick count sent from vehicle
  uint32_t  lr_wheel_tick_count;        //!< LR Wheel tick count sent from vehicle
  uint32_t  rr_wheel_tick_count;        //!< RR Wheel tick count sent from vehicle
#endif /* ENABLE_WHEELTIX */
  uint64_t  time_msec_at_recv_trigger_ublox_msg;
  uint64_t  cpu_cycles_at_recv_trigger_ublox_msg;
  uint32_t  cur_count_empty_spi_reads;
  uint32_t  cur_count_time_adjustments_and_sets;

} tpsDataType;


/*********** END Specific Definitions **********/
#endif
