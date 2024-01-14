/*---------------------------------------------------------------------------*/
/*                                                                           */
/*    Filename: tpstestapp.c                                                 */
/*     Purpose: Application to test and debug the TPS process                */
/*  Department: DENSO North America Research & Development                   */
/*                                                                           */
/* Copyright (C) 2015 DENSO International America, Inc.                      */
/*                                                                           */
/*     Project: V2X Special Projects                                         */
/* Description: Presents user with a menu of requests to send to the TPS     */
/*              process to examine its state and the state of the gps unit.  */
/*---------------------------------------------------------------------------*/



#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>

#define GETTIMEOFDAY(x) gettimeofday((x),NULL)

/* fun terminal stuff */
#define ERESET          "\033[0m"                     /* Reset Txt Fmt */
#define EBOLDBLINKRED   "\033[1m\033[7m\033[31m"      /* Bold Inverse Red */ 

/* These list of include files must not be altered */
#include "dn_types.h"
#include "tps_api.h"
#ifdef ENABLE_SR
#include "sr_util.h"
#include "sr.h"
#include "sr_macros.h"
#include "shm_sr.h"
#endif
#include "shm_tps.h"
#include "wsu_sharedmem.h"  // for wsu_share_init() and friends
#include "tps_enum_to_str.h"

#define PRNT_CSV(fmt, args...)  if ((fcsv != NULL)) { fprintf(fcsv, fmt, ##args); fflush(fcsv); }

#define MAX_RAW_MSG_HDR_LEN 20
#define MAX_FILE_PATH_LEN  300 /*200 + 100*/
#define MAX_FILE_NAME_LEN  100
#define MAX_UNIQUE_STRING_LEN 6
    

static int32_t tps_error_type_list_full = 0;
static int32_t tps_error_type_write = 0;
static int32_t tps_error_type_read = 0;
static int32_t tps_error_type_nmea_cksum = 0;
static int32_t tps_error_type_parsing = 0;
static int32_t tps_no_nema_output = 0;
static int32_t tps_no_pps_output = 0;

static uint32_t GlobalDataPrinting = 1;
static uint32_t PrintDataToConsole = 0;
static uint32_t LoggingEnabled = 0;
static char_t   gps_title[1024];
static FILE     *fcsv=NULL;
shm_tps_t       *shm_tps_ptr;
#if defined(ENABLE_SR)
shm_sr_t        *shm_sr_ptr;
#endif
//Added for Average Position
static int32_t  count = 0; 
time_t          start_time; 
time_t          end_time; 

bool_t           tps_registered = FALSE;

/* User's application's own include files here */


/********* Example code for TPS applications *********/

// Appends time/date to a filename string
void appendTimeDateToFileName(char_t *filename)
{
    time_t     result1;
    struct tm  *timeptr, tm;
    char_t     timeDateString[40];
    
    memset(timeDateString,0,sizeof(timeDateString));

    result1 = time(NULL);
    timeptr = gmtime_r(&result1, &tm);

    sprintf(timeDateString, "_%d%.2d%.2d_%.2d%.2d%.2d",
      1900 + timeptr->tm_year,
      (timeptr->tm_mon + 1),
      timeptr->tm_mday, timeptr->tm_hour,
      timeptr->tm_min, timeptr->tm_sec);

    strcat(filename, timeDateString);
}

int64_t v_gettime()
{
    struct timeval sys_tv;
    int64_t        v;

    GETTIMEOFDAY( &sys_tv );
    v = sys_tv.tv_sec*1000LL + sys_tv.tv_usec/1000LL;
    return v;
}

static int32_t create_log_file()
{
    char_t  log_file_path[MAX_FILE_PATH_LEN + 1];
    char_t  log_file_name[MAX_FILE_NAME_LEN];
    char_t  tempStr[MAX_FILE_PATH_LEN + 20];
    char_t  uniqueStr[MAX_UNIQUE_STRING_LEN + 1];
    FILE    *fp;
    int32_t i = 0;

    memset( log_file_path, 0, sizeof(log_file_path));
    memset( log_file_name, 0, sizeof(log_file_name));
    memset( tempStr, 0, sizeof(tempStr));
    memset( uniqueStr, 0, sizeof(uniqueStr));
   
    /* Disable console printing for few seconds until creation of log file */
    GlobalDataPrinting = 0;

    printf("\nEnter Log  File storage  PATH (Max. 200 chars) (Or)\nPress Enter to use Default PATH(\"/mnt/rwflash/TPS/\"):");

    for (i = 0; i < MAX_FILE_PATH_LEN; i++) {
        scanf("%c",&log_file_path[i]);
        if (log_file_path[i] == '\n') {
            log_file_path[i] = '\0';
            break;
        }
    }
    
    if (i == MAX_FILE_PATH_LEN) {
        log_file_path[i] = '\0';
    }

    if (strlen(log_file_path) == 0) {
    /* Using default PATH for Log file storage */
        strcpy(log_file_path,"/mnt/rwflash/TPS");
    }
      
    /* If the path doesn't exist, create one */
    if ( (fp = fopen(log_file_path, "r")) == NULL ) {
        /* Path does not exist. Create */
        strcpy(tempStr, "mkdir -p ");
        strcat(tempStr, log_file_path);
        system(tempStr);
    } else {
        fclose(fp);
    }

    printf("\nEnter a unique identification string (Max. 6 chars)\nto  the  Log  file  name (Or)  Press  Enter if none:");

    for (i = 0; i < MAX_UNIQUE_STRING_LEN; i++) {
        scanf("%c",&uniqueStr[i]);
        if (uniqueStr[i] == '\n') {
            uniqueStr[i] = '\0';
            break;
        }
    }

    if (i == MAX_UNIQUE_STRING_LEN) {
        uniqueStr[i] = '\0';
    }

    /* Create File */ 
    
    if (log_file_path[strlen(log_file_path) - 1] == '/') { 
        strcpy(log_file_name,"tps");
    } else {
        strcpy(log_file_name,"/tps");
    }
  
    if (strlen(uniqueStr) > 0) {
        strcat(log_file_name,"_");
        strcat(log_file_name,uniqueStr);
    }

    appendTimeDateToFileName(log_file_name);            
    strcat(log_file_name,".csv"); 
  
    strcat(log_file_path,log_file_name);

    if ((fcsv = fopen(log_file_path, "w" )) == NULL) {
        
        printf("Unable to create log file\n");
        
        sleep(2);
        
        GlobalDataPrinting = 1;
        return 0;
    }
   
    printf("\nLog file created at PATH:%s\n",log_file_path);

    PRNT_CSV("%s",gps_title)
   
    sleep(2);
        
    GlobalDataPrinting = 1;

    return 1;
}

static void logTpsData(tpsDataType *data)
{
    tpsDataType tpsUserData;
    
    /* Copy the data and print it */
    memcpy(&tpsUserData, data, sizeof(tpsDataType));

    /////PRNT_CSV("%lu,%llu,%llu,%lu,%lld,%d,%d,%d,%s,%s,%.7f,%.7f,%.3f,%.4f,%.2f,%.2f,%.2f,%.3f,%.3f,%.3f,%.3f,%.4f,%d,%.2f,%d,%d,%d,%d,%d\n",
    PRNT_CSV("%u,%lu,%lu,%u,%ld,%d,%d,%d,%s,%s,%.7f,%.7f,%.3f,%.4f,%.2f,%.2f,%.2f,%.3f,%.3f,%.3f,%.3f,%.4f,%d,%.2f,%d,%d,%d,%d,%d\n",
           (uint32_t) tpsUserData.cur_count_time_adjustments_and_sets,
           tpsUserData.time_msec_at_recv_trigger_ublox_msg,
           tpsUserData.cpu_cycles_at_recv_trigger_ublox_msg,
           (uint32_t) tpsUserData.cur_count_empty_spi_reads,
           v_gettime(),
           tpsUserData.valid,
           tpsUserData.fixquality,
           tpsUserData.fix_mode,
           tpsUserData.date,
           tpsUserData.time,
           tpsUserData.latitude,
           tpsUserData.longitude,
           tpsUserData.altitude,
           tpsUserData.groundspeed,
           tpsUserData.course,
           tpsUserData.hdop,
           tpsUserData.pdop,
           tpsUserData.lat_err,
           tpsUserData.lon_err,
           tpsUserData.smjr_err,
           tpsUserData.smnr_err,
           tpsUserData.smjr_err_orient,
           tpsUserData.numsats,
           tpsUserData.diff_age,
           tps_error_type_nmea_cksum,
           tps_error_type_parsing,
           tps_error_type_list_full,
           tps_error_type_write,
           tps_error_type_read
           );
}

static void printTpsData(tpsDataType *data)
{
    tpsDataType tpsUserData;
    int32_t     secs;
#ifdef WSU_UCLINUX
    int32_t     i1, i2, i3, i4, i5, i6;
#endif

    time(&end_time);
    /* Copy the data and print it */
    memcpy(&tpsUserData, data, sizeof(tpsDataType));

    static float64_t avg_lat = 0.0;     /* i.e. 33.079996 deg */
    static float64_t avg_long= 0.0;     /* i.e. -117.136489 deg */
    static float64_t avg_alt = 0.0;     /* Height Above Ellipsoid (i.e. 131.0) */
    printf("-------------------------\n");
    printf("  valid:             %d\n",   tpsUserData.valid);
#if defined(HEAVY_DUTY)
    switch(tpsUserData.gps_data_source_message_type) {
        case GPS_DATA_SOURCE_MESSAGE_TYPE_NMEA:
            printf("  source_msg_type:   NMEA\n");
            break;
        case GPS_DATA_SOURCE_MESSAGE_TYPE_UBX:
            printf("  source_msg_type:   UBX\n");
            break;
        case (GPS_DATA_SOURCE_MESSAGE_TYPE_UBX|GPS_DATA_SOURCE_MESSAGE_TYPE_NMEA) :
            printf("  source_msg_type:   UBX & NMEA\n");
            break;
        default:
            printf("  source_msg_type:   Unknown=%d\n",tpsUserData.gps_data_source_message_type);
            break;
    }
#else
    printf("  source_msg_type:   %s\n",
        (tpsUserData.gps_data_source_message_type == GPS_DATA_SOURCE_MESSAGE_TYPE_NMEA) ? "NMEA"
            : (tpsUserData.gps_data_source_message_type == GPS_DATA_SOURCE_MESSAGE_TYPE_UBX) ? "UBX" : "???");
#endif
    printf("  fixquality:        %d\n",   tpsUserData.fixquality);
    printf("  fix-mode:          %d\n",   tpsUserData.fix_mode);
    printf("  date:              %.6s\n", tpsUserData.date);
    printf("  time:              %.9s\n", tpsUserData.time);
#ifdef WSU_UCLINUX
    /* uClinux does not allow printf's on floating point numbers in
     * pthread-launched threads. */
    i1 = (int32_t)tpsUserData.latitude;
    i2 = (int32_t)((tpsUserData.latitude - i1) * 10000000);
    if (i2 < 0) i2 = -i2;
    printf("  latitude:          %d.%07d\n", i1, i2);
    i1 = (int32_t)tpsUserData.longitude;
    i2 = (int32_t)((tpsUserData.longitude - i1) * 10000000);
    if (i2 < 0) i2 = -i2;
    printf("  longitude:         %d.%07d\n", i1, i2);
    i1 = (int32_t)tpsUserData.altitude;
    i2 = (int32_t)((tpsUserData.altitude - i1) * 100);
    if (i2 < 0) i2 = -i2;
    printf("  altitude:          %d.%02d\n", i1, i2);
    i1 = (int32_t)tpsUserData.groundspeed;
    i2 = (int32_t)((tpsUserData.groundspeed - i1) * 1000);
    if (i2 < 0) i2 = -i2;
    printf("  groundspeed:       %d.%03d\n", i1, i2);
    i1 = (int32_t)tpsUserData.course;
    i2 = (int32_t)((tpsUserData.course - i1) * 10);
    if (i2 < 0) i2 = -i2;
    printf("  course:            %d.%d\n", i1, i2);
    i1 = (int32_t)tpsUserData.hdop;
    i2 = (int32_t)((tpsUserData.hdop - i1) * 100);
    if (i2 < 0) i2 = -i2;
    printf("  hdop:              %d.%02d\n", i1, i2);
    i1 = (int32_t)tpsUserData.pdop;
    i2 = (int32_t)((tpsUserData.pdop - i1) * 100);
    if (i2 < 0) i2 = -i2;
    printf("  pdop:              %d.%02d\n", i1, i2);
    i1 = (int32_t)tpsUserData.lat_err;
    i2 = (int32_t)((tpsUserData.lat_err - i1) * 100);
    if (i2 < 0) i2 = -i2;
    printf("  laterr:            %d.%02d\n", i1, i2);
    i1 = (int32_t)tpsUserData.lon_err;
    i2 = (int32_t)((tpsUserData.lon_err - i1) * 100);
    if (i2 < 0) i2 = -i2;
    printf("  lonerr:            %d.%02d\n", i1, i2);
    i1 = (int32_t)tpsUserData.alt_err;
    i2 = (int32_t)((tpsUserData.alt_err - i1) * 100);
    if (i2 < 0) i2 = -i2;
    printf("  alterr:            %d.%02d\n", i1, i2);
    i1 = (int32_t)tpsUserData.smjr_err;
    i2 = (int32_t)((tpsUserData.smjr_err - i1) * 100);
    if (i2 < 0) i2 = -i2;
    printf("  semi-major_err:    %d.%02d\n", i1, i2);
    i1 = (int32_t)tpsUserData.smnr_err;
    i2 = (int32_t)((tpsUserData.smnr_err - i1) * 100);
    if (i2 < 0) i2 = -i2;
    printf("  semi-minor_err:    %d.%02d\n", i1, i2);
    i1 = (int32_t)tpsUserData.smjr_err_orient;
    i2 = (int32_t)((tpsUserData.smjr_err_orient - i1) * 10000);
    if (i2 < 0) i2 = -i2;
    printf("  semi-major_orient: %d.%04d\n", i1, i2);
    printf("  numsats:           %d\n"  , tpsUserData.numsats);
    i1 = (int32_t)tpsUserData.diff_age;
    i2 = (int32_t)((tpsUserData.diff_age - i1) * 100);
    if (i2 < 0) i2 = -i2;
    printf("  diffage:           %d.%02d\n"  , i1, i2);
#else
    printf("  latitude:          %.7f\n", tpsUserData.latitude);
    printf("  longitude:         %.7f\n", tpsUserData.longitude);
    printf("  altitude:          %.2f\n", tpsUserData.altitude);
    printf("  groundspeed:       %.3f\n", tpsUserData.groundspeed);
    printf("  course:            %.1f\n", tpsUserData.course);
    printf("  hdop:              %.2f\n", tpsUserData.hdop);
    printf("  pdop:              %.2f\n", tpsUserData.pdop);
    printf("  laterr:            %.2f\n", tpsUserData.lat_err);
    printf("  lonerr:            %.2f\n", tpsUserData.lon_err);
#ifndef WSU_5001
    printf("  alterr:            %.2f\n", tpsUserData.alt_err);
#endif
    printf("  semi-major_err:    %.2f\n", tpsUserData.smjr_err);
    printf("  semi-minor_err:    %.2f\n", tpsUserData.smnr_err);
    printf("  semi-major_orient: %.4f\n", tpsUserData.smjr_err_orient);
    printf("  numsats:           %d\n"  , tpsUserData.numsats);
    printf("  diffage:           %.2f\n", tpsUserData.diff_age);
    printf("  leap_seconds:      %d\n",   tpsUserData.curLeapSecs);
    printf("  coming_ls_change:  %d\n",   tpsUserData.upcomingLeapSecChange);
    printf("  secs_to_lschange:  %d\n",   tpsUserData.timeToLeapSecEvent_secs);
    printf("  ubx_fix_type:      %u  (0=None, 1=DR, 2=2D, 3=3D, 4=GNSS+DR, 5=TimeOnly)\n",   tpsUserData.ubx_fix_type);
    printf("  diff_corrections:  %u\n",   tpsUserData.flag_differential_corrections_applied);
#endif
    printf("\n");
    if(tpsUserData.valid)
    {
        count++;
        secs = end_time - start_time;
        printf("  Secs Passed: %d\n", secs);
        avg_lat  += tpsUserData.latitude;
        avg_long += tpsUserData.longitude;
        avg_alt  += tpsUserData.altitude;
  
#ifdef WSU_UCLINUX
        i1 = (int32_t)(avg_lat / count);
        i2 = (int32_t)(((avg_lat / count) - i1) * 10000000);
        if (i2 < 0) i2 = -i2;
        i3 = (int32_t)(avg_long / count);
        i4 = (int32_t)(((avg_long / count) - i3) * 10000000);
        if (i4 < 0) i4 = -i4;
        i5 = (int32_t)(avg_alt / count);
        i6 = (int32_t)(((avg_alt / count) - i5) * 100);
        if (i6 < 0) i6 = -i6;
        printf("  Average Position:  Lat:%d.%07d, Long: %d.%07d, Alt:%d.%02d\n", i1, i2, i3, i4, i5, i6);
#else
        printf("  Average Position:  Lat:%.7f, Long: %.7f, Alt:%.2f\n", avg_lat/count, avg_long/count, avg_alt/count);
#endif
    }
    else
    {
        printf("Current Position: No Fix\n");
        printf("Avg Position: No Fix\n");
    }

    //don't clear leap second info. Let it persist...for now.

    //tpsUserData.curLeapSecs                           = 0;
    //tpsUserData.upcomingLeapSecChange                 = 0;
    //tpsUserData.timeToLeapSecEvent_secs               = 0; 
    tpsUserData.ubx_fix_type                          = 0; 
    tpsUserData.flag_differential_corrections_applied = 0;
    tpsUserData.diff_age                              = 0.0f;

}

/* Call back function for Registering to get TPS data. If TPS goes No Nav, no CB and stale data forever :-S  */
static void printDataIndication(tpsDataType *data)
{
    /* Print message contents for debug. */
    if (PrintDataToConsole && GlobalDataPrinting) 
    {
        printf("\n");
        printf("TPS Error status: NMEA cksum:%d, Parsing:%d, Reglistfull:%d, Write:%d, Read:%d, No Output:%d, No PPS Output:%d \n",
                tps_error_type_nmea_cksum, tps_error_type_parsing, tps_error_type_list_full,
                tps_error_type_write, tps_error_type_read,tps_no_nema_output,tps_no_pps_output );
        printf("Type: TPS_DATA_IND\n");
        printTpsData(data);
    }
    
    if (LoggingEnabled)
        logTpsData(data);
}


/* Call back function for Requesting TPS data */
static void printDataConfirmation(tpsDataType *data)
{
    /* Print message contents for debug. */
    printf("\n");
    printf("Type: TPS_DATA_CFM\n");

    printTpsData(data);
}

static void errorHandler(tpsResultCodeType err)
{
    switch(err)
    {
    case TPS_CONN_LOST:
        printf("TPS Connection Lost. Exit this app.\n");

#ifdef ENABLE_SR
        if (tps_registered) {
            WSU_TPS_DEREGISTER();
        }

        WSU_SR_DEINIT_TPS(sr_tps_sockfd);
#else
        if (tps_registered) {
            wsuTpsDeregister();
        }
#endif

        exit(-1);
        break;
    case TPS_REG_LIST_FULL:
        printf("TPS Register List Full. Can't register now.\n");
        tps_error_type_list_full++;
        break;
    case GPS_WRITE_ERROR:
        printf("GPS Write Error.\n");
        tps_error_type_write++;
        break;
    case GPS_READ_ERROR_EXIT:
        printf("GPS READ Error. TPS Exits.\n");
        tps_error_type_read++;
        break;
    case GPS_NMEA_CKSUM_ERROR:
        printf("GPS NMEA message checksum verification failed\n");
        tps_error_type_nmea_cksum++;
        break;
    case GPS_NMEA_MSG_PARSE_ERROR:
        printf("GPS NMEA message parsing error\n");
        tps_error_type_parsing++;
        break;
    case GPS_NO_NMEA_OUTPUT:
        printf("GPS no NMEA output \n");
        tps_no_nema_output++;
        break;
    case GPS_NO_PPS_OUTPUT:
        printf("GPS no PPS output \n");
        tps_no_pps_output++;
        break;
    default:
        break;
    }
}

static void printRTCMData(char_t *data, int32_t len)
{
    int32_t ix;

    if (GlobalDataPrinting) {  
        printf("\n");
        for (ix = 0; ix < len; ix++)
        {
            if (ix > 0 && ix % 20 == 0)
            {
                printf("\n");
            }
            printf("%.2x ", data[ix]);
        }
        printf("\n");
    }
}

static void printRAWData(char_t *data, int32_t len)
{
#ifdef ENABLE_SR
    WSU_SEND_SR_MESSAGE_TPS_RAW(sr_tps_sockfd, data, len);
#endif

    if (GlobalDataPrinting) {
        printf("Data with length:%d:\n",len); 
        printf("%s\n",data); 
    }
}

static void stopLogging()
{
    /* Stop Logging */
    LoggingEnabled = 0;
    
    GlobalDataPrinting = 0;
    
    if (!PrintDataToConsole) {
#ifdef ENABLE_SR
        WSU_TPS_DEREGISTER();
#else
        wsuTpsDeregister();
#endif
    } 
    
    if (fcsv != NULL) {
        fclose(fcsv);
        fcsv = NULL;
        sync();
        printf("Logging Disabled\n");
    }
    
    sleep(2);

    GlobalDataPrinting = 1;
}

static void stopConsolePrint()
{
    /* Deregister this application from TPS */
    PrintDataToConsole = 0;
    if (!LoggingEnabled) {
#ifdef ENABLE_SR
        WSU_TPS_DEREGISTER();
#else
        wsuTpsDeregister();
#endif
    }

    if (tps_registered) {
#ifdef ENABLE_SR
        WSU_TPS_DEREGISTER();
#else
        wsuTpsDeregister();
#endif
        tps_registered = FALSE;
    }
}


// Returns the index of the median count in an uint32_t array of size y
int32_t getUInt32MedianIndex(uint32_t * x, uint32_t y)
{
    int32_t  i;
    uint32_t total_count = 0;
    for (i=0; i<y; i++) {
        total_count += x[i];
    }

    uint32_t xcount = 0;
    for (i=0; i<y; i++) {
        xcount += x[i];
        if (xcount >= total_count / 2) {
            return i;
        }
    }
    // Should never happen unless array is all zeros
    return 0;
}


#define SDHGPS_MAX_TRACKED_WRONG_RATE_HZ 20
#define SDHGPS_EXPECTED_RATE 10
#define SDHGPS_MAX_TRACKED_10HZ_INTERVAL 30   /* (300ms) */
#define SDHGPS_MAX_TRACKED_100HZ_INTERVAL 50  /* (5000ms=5secs) */
#define SDHGPS_MAX_TRACKED_1HZ_INTERVAL 120
#define SDHGPS_MIN_TRACKED_1HZ_INTERVAL 80

#define DELTADELTA_MAX_TRACKED_1HZ_INTERVAL 40
#define DELTADELTA_MIN_TRACKED_1HZ_INTERVAL -40

// Variables for tracking receive rate from tps
struct timeval prev_recv_timeval = {0};
uint16_t prev_recv_second = 999;
uint16_t cur_recv_count = 0;
uint16_t prev_recv_count = 0;
uint32_t count_seconds_of_bad_data_update_rate = 0;
uint32_t count_data_gaps_greater_than_one_second = 0;
uint32_t data_rate_counts[SDHGPS_MAX_TRACKED_WRONG_RATE_HZ + 2];   // counts for 1--N and N+1=EverythingMore
uint32_t data_interval_counts_w_10hz_steps[SDHGPS_MAX_TRACKED_10HZ_INTERVAL + 2];
uint32_t data_interval_counts_w_100hz_steps[SDHGPS_MAX_TRACKED_100HZ_INTERVAL + 2];
uint32_t data_interval_counts_w_1hz_steps[SDHGPS_MAX_TRACKED_1HZ_INTERVAL - SDHGPS_MIN_TRACKED_1HZ_INTERVAL + 3];
uint64_t prev_data_time_msec_at_recv = 0;
uint32_t trigger_time_interval_counts_w_10hz_steps[SDHGPS_MAX_TRACKED_10HZ_INTERVAL + 2];
uint32_t trigger_time_interval_counts_w_100hz_steps[SDHGPS_MAX_TRACKED_100HZ_INTERVAL + 2];
uint32_t trigger_time_interval_counts_w_1hz_steps[SDHGPS_MAX_TRACKED_1HZ_INTERVAL - SDHGPS_MIN_TRACKED_1HZ_INTERVAL + 3];
uint32_t delta_delta_counts_w_1hz_steps[DELTADELTA_MAX_TRACKED_1HZ_INTERVAL - DELTADELTA_MIN_TRACKED_1HZ_INTERVAL + 3];
int32_t  rate_report_interval = 0;
int32_t  rate_report_countdown = 0;


/* Prints a report on the data intervals */
void printDataRateReport()
{
    int32_t i;

    printf("Data Rate Testing Report:\n");

    printf("  Receive intervals of UBlox->TPS, counted in Buckets of:\n");
    printf("    1Hz: ");
    for (i=0; i<=SDHGPS_MAX_TRACKED_1HZ_INTERVAL - SDHGPS_MIN_TRACKED_1HZ_INTERVAL + 2; i++) {
        if (trigger_time_interval_counts_w_1hz_steps[i] > 0) {
            if (i == 0) {
                printf(" %u@<%dms", trigger_time_interval_counts_w_1hz_steps[i], SDHGPS_MIN_TRACKED_1HZ_INTERVAL);
            } else if (i > SDHGPS_MAX_TRACKED_1HZ_INTERVAL - SDHGPS_MIN_TRACKED_1HZ_INTERVAL + 1) {
                printf(" %u@>%dms", trigger_time_interval_counts_w_1hz_steps[i], SDHGPS_MAX_TRACKED_1HZ_INTERVAL);
            } else {
                printf(" %u@%dms", trigger_time_interval_counts_w_1hz_steps[i], i-1 + SDHGPS_MIN_TRACKED_1HZ_INTERVAL);
            }
        }
    }
    printf("  (median %dms)", getUInt32MedianIndex(&trigger_time_interval_counts_w_1hz_steps[1],    // Note [1] not [0] so we ignore the below-min-range entries
                                                   SDHGPS_MAX_TRACKED_1HZ_INTERVAL - SDHGPS_MIN_TRACKED_1HZ_INTERVAL + 1) // Note +1 not +3 bc  we are ignoring the <min, >max entries
                                 - 0 + SDHGPS_MIN_TRACKED_1HZ_INTERVAL);        // Note -0 not -1 because we are starting from [1].  ([0] is < min entries.  [1] is min entry)
    printf("\n");
    printf("    10Hz:");
    for (i=0; i<=SDHGPS_MAX_TRACKED_10HZ_INTERVAL+1; i++) {
        if (trigger_time_interval_counts_w_10hz_steps[i] > 0) {
            if (i<=SDHGPS_MAX_TRACKED_10HZ_INTERVAL) {
                printf(" %u@%d0ms", trigger_time_interval_counts_w_10hz_steps[i], i);
            } else {
                printf(" %u@>%d0ms", trigger_time_interval_counts_w_10hz_steps[i], SDHGPS_MAX_TRACKED_10HZ_INTERVAL);
            }
        }
    }
    printf("  (median %d0ms)", getUInt32MedianIndex(trigger_time_interval_counts_w_10hz_steps, SDHGPS_MAX_TRACKED_10HZ_INTERVAL+1)); // Note +1 not +2 to not count final out-of-range entry
    printf("\n");
    printf("    100Hz:");
    for (i=0; i<=SDHGPS_MAX_TRACKED_100HZ_INTERVAL+1; i++) {
        if (trigger_time_interval_counts_w_100hz_steps[i] > 0) {
            if (i<=SDHGPS_MAX_TRACKED_100HZ_INTERVAL) {
                printf(" %u@%d00ms", trigger_time_interval_counts_w_100hz_steps[i], i);
            } else {
                printf(" %u@>%d00ms", trigger_time_interval_counts_w_100hz_steps[i], SDHGPS_MAX_TRACKED_100HZ_INTERVAL);
            }
        }
    }
    printf("  (median %d00ms)", getUInt32MedianIndex(trigger_time_interval_counts_w_100hz_steps, SDHGPS_MAX_TRACKED_100HZ_INTERVAL+1)); // Also +1 not +2
    printf("\n");

    printf("  Receive intervals of TPS->TPSTestApp, counted in Buckets of:\n");
    printf("    1Hz: ");
    for (i=0; i<=SDHGPS_MAX_TRACKED_1HZ_INTERVAL - SDHGPS_MIN_TRACKED_1HZ_INTERVAL + 2; i++) {
        if (data_interval_counts_w_1hz_steps[i] > 0) {
            if (i == 0) {
                printf(" %u@<%dms", data_interval_counts_w_1hz_steps[i], SDHGPS_MIN_TRACKED_1HZ_INTERVAL);
            } else if (i > SDHGPS_MAX_TRACKED_1HZ_INTERVAL - SDHGPS_MIN_TRACKED_1HZ_INTERVAL + 1) {
                printf(" %u@>%dms", data_interval_counts_w_1hz_steps[i], SDHGPS_MAX_TRACKED_1HZ_INTERVAL);
            } else {
                printf(" %u@%dms", data_interval_counts_w_1hz_steps[i], i-1 + SDHGPS_MIN_TRACKED_1HZ_INTERVAL);
            }
        }
    }
    printf("  (median %dms)", getUInt32MedianIndex(&data_interval_counts_w_1hz_steps[1], SDHGPS_MAX_TRACKED_1HZ_INTERVAL - SDHGPS_MIN_TRACKED_1HZ_INTERVAL + 1)
                                 + SDHGPS_MIN_TRACKED_1HZ_INTERVAL);
    printf("\n");
    printf("    10Hz:");
    for (i=0; i<=SDHGPS_MAX_TRACKED_10HZ_INTERVAL+1; i++) {
        if (data_interval_counts_w_10hz_steps[i] > 0) {
            if (i<=SDHGPS_MAX_TRACKED_10HZ_INTERVAL) {
                printf(" %u@%d0ms", data_interval_counts_w_10hz_steps[i], i);
            } else {
                printf(" %u@>%d0ms", data_interval_counts_w_10hz_steps[i], SDHGPS_MAX_TRACKED_10HZ_INTERVAL);
            }
        }
    }
    printf("  (median %d0ms)", getUInt32MedianIndex(data_interval_counts_w_10hz_steps, SDHGPS_MAX_TRACKED_10HZ_INTERVAL+1));
    printf("\n");
    printf("    100Hz:");
    for (i=0; i<=SDHGPS_MAX_TRACKED_100HZ_INTERVAL+1; i++) {
        if (data_interval_counts_w_100hz_steps[i] > 0) {
            if (i<=SDHGPS_MAX_TRACKED_100HZ_INTERVAL) {
                printf(" %u@%d00ms", data_interval_counts_w_100hz_steps[i], i);
            } else {
                printf(" %u@>%d00ms", data_interval_counts_w_100hz_steps[i], SDHGPS_MAX_TRACKED_100HZ_INTERVAL);
            }
        }
    }
    printf("  (median %d00ms)", getUInt32MedianIndex(data_interval_counts_w_100hz_steps, SDHGPS_MAX_TRACKED_100HZ_INTERVAL+1));
    printf("\n");

    printf("  Delta between the two above recieve intervals for the same data:\n");
    printf("    1Hz: ");
    for (i=0; i<=DELTADELTA_MAX_TRACKED_1HZ_INTERVAL - DELTADELTA_MIN_TRACKED_1HZ_INTERVAL + 2; i++) {
        if (delta_delta_counts_w_1hz_steps[i] > 0) {
            if (i == 0) {
                printf(" %u@<%dms", delta_delta_counts_w_1hz_steps[i], DELTADELTA_MIN_TRACKED_1HZ_INTERVAL);
            } else if (i > DELTADELTA_MAX_TRACKED_1HZ_INTERVAL - DELTADELTA_MIN_TRACKED_1HZ_INTERVAL + 1) {
                printf(" %u@>%dms", delta_delta_counts_w_1hz_steps[i], DELTADELTA_MAX_TRACKED_1HZ_INTERVAL);
            } else {
                printf(" %u@%dms", delta_delta_counts_w_1hz_steps[i], i-1 + DELTADELTA_MIN_TRACKED_1HZ_INTERVAL);
            }
        }
    }
    printf("\n");

    printf("  Seconds-based Rate Tracking:\n");
    printf("    Last Second's Data Rate: %d\n", prev_recv_count);
    printf("    Bad Rate Count: %d secs     Multi-Second No Data Count: %d\n",
        count_seconds_of_bad_data_update_rate,
        count_data_gaps_greater_than_one_second);
    printf("    Bad Data Rates:");

    // List out counts for the incorrect rates.
    for (i=1; i<=SDHGPS_MAX_TRACKED_WRONG_RATE_HZ+1; i++) {
        if (data_rate_counts[i] > 0) {
            if (i <= SDHGPS_MAX_TRACKED_WRONG_RATE_HZ) {
                printf(" %u@%dHz", data_rate_counts[i], i);
            } else {
                printf(" %u@>%dHz", data_rate_counts[i], SDHGPS_MAX_TRACKED_WRONG_RATE_HZ);
            }
        }
    }
    printf("\n");
}


/* Call back function for Registering to get TPS data for data rate testing */
static void testDataIndicationRate(tpsDataType *data)
{
    struct timeval now_timeval;
    int32_t delta_ms =0;
    int32_t delta_10hz;
    int32_t delta_100hz;
    int32_t xd_tvsec;
    int32_t xd_tvusec;
    int32_t delta_ms_recv;
    int32_t delta_ms_trig;

    gettimeofday(&now_timeval, NULL);

    // Compute delta interval
    if (prev_recv_timeval.tv_sec > 0) {
        xd_tvsec = now_timeval.tv_sec - prev_recv_timeval.tv_sec;
        xd_tvusec = now_timeval.tv_usec - prev_recv_timeval.tv_usec;
        delta_ms = xd_tvsec * 1000 + xd_tvusec / 1000;
        // 10hz delta value
        /* uint32_t*/ delta_10hz = (delta_ms + 5) / 10;
        /* uint32_t */ delta_100hz = (delta_ms + 50) / 100;
        if (delta_10hz <= SDHGPS_MAX_TRACKED_10HZ_INTERVAL)
            ++data_interval_counts_w_10hz_steps[delta_10hz];
        else
            ++data_interval_counts_w_10hz_steps[SDHGPS_MAX_TRACKED_10HZ_INTERVAL+1];
        if (delta_100hz <= SDHGPS_MAX_TRACKED_100HZ_INTERVAL)
            ++data_interval_counts_w_100hz_steps[delta_100hz];
        else
            ++data_interval_counts_w_100hz_steps[SDHGPS_MAX_TRACKED_100HZ_INTERVAL];
        if (delta_ms < SDHGPS_MIN_TRACKED_1HZ_INTERVAL) {
            ++data_interval_counts_w_1hz_steps[0];
        } else if (delta_ms > SDHGPS_MAX_TRACKED_1HZ_INTERVAL) {
            ++data_interval_counts_w_1hz_steps[SDHGPS_MAX_TRACKED_1HZ_INTERVAL - SDHGPS_MIN_TRACKED_1HZ_INTERVAL + 2];
        } else {
            ++data_interval_counts_w_1hz_steps[delta_ms - SDHGPS_MIN_TRACKED_1HZ_INTERVAL + 1];
        }
    }
    delta_ms_recv = delta_ms;

    /* Tracking the GPS update rate */
    uint16_t cur_recv_second = now_timeval.tv_sec % 60;
    // If in the same second as last time, just tick the ind count
    if (prev_recv_second == cur_recv_second) {
        ++cur_recv_count;
    // We are in a new second
    } else {
        // If we are in new second and it's the next sequential one
        if (cur_recv_second == (prev_recv_second + 1) % 60) {
            // If the prev second's rate was sub-optimal, count a bad second
            //    *IF* the next oldest rate was above zero.  This lets us
            //    detect slow GPS but not count seconds on the edge of where
            //    GPS coverage was lost completely.  We want to detect bad
            //    GPS performance, not lost GPS coverage.
            // Note we are not counting a zero rate right now, because cur_recv_count > 0
            if (cur_recv_count != SDHGPS_EXPECTED_RATE && prev_recv_count > 0) {
                ++count_seconds_of_bad_data_update_rate;
                if (cur_recv_count <= SDHGPS_MAX_TRACKED_WRONG_RATE_HZ) {
                    ++data_rate_counts[cur_recv_count];
                } else {
                    ++data_rate_counts[SDHGPS_MAX_TRACKED_WRONG_RATE_HZ + 1];
                }
            }
            prev_recv_count = cur_recv_count;
            prev_recv_second = cur_recv_second;
            cur_recv_count = 1;
        // However, if there's been a bigger-than-second gap, we
        //    wipe the tracking data and start over.
        //  We will count this multi-second gap as one big gap,
        //    rather than counting a N-second gap as N-different
        //    bad seconds.
        // Note, the current second should get ignored as well,
        //    because the data is just getting restarted, and
        //    odds are 9/10 this is not at the start of the second,
        //    likely this new sec's recv_count will be < 10 even
        //    GPS is now at 10 hz.  So we set prev_recv_count = 0,
        //    which will cause the current sec's rate to not count
        //    as a low second once this second is over.
        } else {
            if (prev_recv_second != 999) // Except, don't count the gap when we are just starting out
                ++count_data_gaps_greater_than_one_second;
            prev_recv_count = 0;
            prev_recv_second = cur_recv_second;
            cur_recv_count = 1;
        }
    }


    // Compute delta interval in the data->time_msec_at_recv_trigger_ublox_msg of the message
    if (prev_data_time_msec_at_recv > 0) {
        uint64_t recv_trigger_time_delta_ms = data->time_msec_at_recv_trigger_ublox_msg - prev_data_time_msec_at_recv;

        delta_ms = (uint32_t) recv_trigger_time_delta_ms;
        delta_10hz = (delta_ms + 5) / 10;
        delta_100hz = (delta_ms + 50) / 100;

        if (delta_10hz <= SDHGPS_MAX_TRACKED_10HZ_INTERVAL)
            ++trigger_time_interval_counts_w_10hz_steps[delta_10hz];
        else
            ++trigger_time_interval_counts_w_10hz_steps[SDHGPS_MAX_TRACKED_10HZ_INTERVAL+1];
        if (delta_100hz <= SDHGPS_MAX_TRACKED_100HZ_INTERVAL)
            ++trigger_time_interval_counts_w_100hz_steps[delta_100hz];
        else
            ++trigger_time_interval_counts_w_100hz_steps[SDHGPS_MAX_TRACKED_100HZ_INTERVAL];
        if (delta_ms < SDHGPS_MIN_TRACKED_1HZ_INTERVAL) {
            ++trigger_time_interval_counts_w_1hz_steps[0];
        } else if (delta_ms > SDHGPS_MAX_TRACKED_1HZ_INTERVAL) {
            ++trigger_time_interval_counts_w_1hz_steps[SDHGPS_MAX_TRACKED_1HZ_INTERVAL - SDHGPS_MIN_TRACKED_1HZ_INTERVAL + 2];
        } else {
            ++trigger_time_interval_counts_w_1hz_steps[delta_ms - SDHGPS_MIN_TRACKED_1HZ_INTERVAL + 1];
        }
    }
    delta_ms_trig = delta_ms;

    // Observe the delta between the two delta_ms's recv & trig !!
    if (prev_recv_timeval.tv_sec > 0 && prev_data_time_msec_at_recv > 0) {
        delta_ms = delta_ms_recv - delta_ms_trig;
        if (delta_ms < DELTADELTA_MIN_TRACKED_1HZ_INTERVAL) {
            ++delta_delta_counts_w_1hz_steps[0];
        } else if (delta_ms > DELTADELTA_MAX_TRACKED_1HZ_INTERVAL) {
            ++delta_delta_counts_w_1hz_steps[DELTADELTA_MAX_TRACKED_1HZ_INTERVAL - DELTADELTA_MIN_TRACKED_1HZ_INTERVAL + 2];
        } else {
            ++delta_delta_counts_w_1hz_steps[delta_ms - DELTADELTA_MIN_TRACKED_1HZ_INTERVAL + 1];
        }
    }



    // Reporting
    if (rate_report_interval > 0) {
        --rate_report_countdown;
        if (rate_report_countdown <= 0) {
            rate_report_countdown = rate_report_interval;
            printDataRateReport();
        }
    }

    prev_recv_timeval.tv_sec = now_timeval.tv_sec;
    prev_recv_timeval.tv_usec = now_timeval.tv_usec;
    prev_data_time_msec_at_recv = data->time_msec_at_recv_trigger_ublox_msg;
}


/*
** @brief Prints a health / status report on TPS
** @description Saves a snapshot of the current tps shm to get deltas
**              at the next report point.  This report is pre-called
**              at startup so user's first report includes the
**              delta-dependent data.
*/
#define MAX_AGC_MONITOR_VALUE 8191.0    /* Largest value in this field, represents 100% */
void printStatusMonitorReport(int32_t want_full_report)
{
    static shm_tps_t prev_shm_tps;
    static struct    timeval prev_status_timeval = {0};
    static uint8_t   gave_help = 0;
    struct timeval   now_timeval;
    float64_t        delta_secs = 0;
    int32_t          i,j,cur_idx;

    gettimeofday(&now_timeval, NULL);

    if (want_full_report) {

        delta_secs = (now_timeval.tv_sec - prev_status_timeval.tv_sec) + ((now_timeval.tv_usec - prev_status_timeval.tv_usec)/1000000.0);
        // Force a minimum of one second time deltas
        if (delta_secs < 1) {
            sleep(1);
            gettimeofday(&now_timeval, NULL);
            delta_secs = (now_timeval.tv_sec - prev_status_timeval.tv_sec) + ((now_timeval.tv_usec - prev_status_timeval.tv_usec)/1000000.0);
        }

        if (!gave_help) {
            printf("Welcome to TPS Status Monitor Report!\n");
            printf("   Please note all non-time-related values represent totals since TPS started.\n");
            printf("   Please note time-related values represent current rates, not overall rates.\n");
            printf("   For example, if SR was recording but has since closed the file, you may\n");
            printf("   correctly see 1000 msgs sent to SR total, and a 0 msgs/sec rate.\n");
            gave_help = 1;
        }

        printf("TPS Status Monitor Report:\n");

        printf("  Time delta since last report: %.3lf sec, current tps_seqno %u", delta_secs, shm_tps_ptr->seqno);
        if (shm_tps_ptr->ublox_firmware_version_str[0]) {
            printf(", ublox firmware ver: \"%s\"", shm_tps_ptr->ublox_firmware_version_str);
        }
        if (shm_tps_ptr->ublox_config_version_str[0]) {
            printf(", ublox config: \"%s\"", shm_tps_ptr->ublox_config_version_str);
        }
        printf("\n");

        printf("  Reading data from GPS Hardware Module:\n");

        printf("     Reads: %u small %u large, read %u chars, %u chars dropped, %u usleeps, overall read/drop ratio: %.1lf\n",
            shm_tps_ptr->debug_cnt_spi_initial_reads,
            shm_tps_ptr->debug_cnt_spi_large_reads,
            shm_tps_ptr->debug_cnt_chars_read,
            shm_tps_ptr->debug_cnt_unused_chars,
            shm_tps_ptr->debug_cnt_spi_empty_usleeps,
            shm_tps_ptr->debug_cnt_unused_chars > 0 
                ? (float64_t) shm_tps_ptr->debug_cnt_chars_read / (float64_t) shm_tps_ptr->debug_cnt_unused_chars : 0);

        printf("     Reads/sec: %.0lf small %.0lf large  chars read/s: %.0lf "
               "drops/s: %.0lf  usleep/s: %.0lf   delta read/drop ratio: %.1lf\n",
            (shm_tps_ptr->debug_cnt_spi_initial_reads - prev_shm_tps.debug_cnt_spi_initial_reads) / delta_secs,
            (shm_tps_ptr->debug_cnt_spi_large_reads - prev_shm_tps.debug_cnt_spi_large_reads) / delta_secs,
            (shm_tps_ptr->debug_cnt_chars_read - prev_shm_tps.debug_cnt_chars_read) / delta_secs,
            (shm_tps_ptr->debug_cnt_unused_chars - prev_shm_tps.debug_cnt_unused_chars) / delta_secs,
            (shm_tps_ptr->debug_cnt_spi_empty_usleeps - prev_shm_tps.debug_cnt_spi_empty_usleeps) / delta_secs,
            (shm_tps_ptr->debug_cnt_unused_chars == prev_shm_tps.debug_cnt_unused_chars
                ? 0
                : (float64_t) (shm_tps_ptr->debug_cnt_chars_read - prev_shm_tps.debug_cnt_chars_read) / 
                  (float64_t) (shm_tps_ptr->debug_cnt_unused_chars - prev_shm_tps.debug_cnt_unused_chars))
        );

        // Read/Sleep details
        printf("     Large reads between usleeps:");
        for (i=0; i<=TPS_MAX_TRACKED_LARGE_READ_COUNT+1; i++) {
            if (shm_tps_ptr->debug_cnt_large_reads_between_usleeps[i] > 0) {
                if (i <= TPS_MAX_TRACKED_LARGE_READ_COUNT) {
                    printf(" %u@%d", shm_tps_ptr->debug_cnt_large_reads_between_usleeps[i], i);
                } else {
                    printf(" %u@>%d", shm_tps_ptr->debug_cnt_large_reads_between_usleeps[i], TPS_MAX_TRACKED_LARGE_READ_COUNT);
                }
            }
        }
        printf("  (median %d)", getUInt32MedianIndex(shm_tps_ptr->debug_cnt_large_reads_between_usleeps, TPS_MAX_TRACKED_LARGE_READ_COUNT+1));   // +2 to include the out-of-range cnt
        printf("\n");
        printf("     Usleeps between large reads:");
        for (i=0; i<=TPS_MAX_TRACKED_USLEEPS_BETWEEN_LARGE_READS+1; i++) {
            if (shm_tps_ptr->debug_cnt_usleeps_between_large_reads[i] > 0) {
                if (i <= TPS_MAX_TRACKED_USLEEPS_BETWEEN_LARGE_READS) {
                    printf(" %u@%d", shm_tps_ptr->debug_cnt_usleeps_between_large_reads[i], i);
                } else {
                    printf(" %u@>%d", shm_tps_ptr->debug_cnt_usleeps_between_large_reads[i], TPS_MAX_TRACKED_USLEEPS_BETWEEN_LARGE_READS);
                }
            }
        }
        printf("  (median %d)", getUInt32MedianIndex(shm_tps_ptr->debug_cnt_usleeps_between_large_reads, TPS_MAX_TRACKED_USLEEPS_BETWEEN_LARGE_READS+2));  // +2 includes out-of-range cnt
        printf("\n");

        printf("  Message parsing:\n");
        printf("     parsed %d+%d+%d cmds making %d dataInds (%d sent), %d parse aborts\n",
            shm_tps_ptr->debug_gps_cmd_count_nmea,
            shm_tps_ptr->debug_gps_cmd_count_rtcm,
            shm_tps_ptr->debug_gps_cmd_count_ubx,
            shm_tps_ptr->debug_cnt_comm_send_tps_data_ind,
            shm_tps_ptr->debug_cnt_inds_sent_to_tps,
            shm_tps_ptr->debug_buffer_aborts);

        printf("            cmds/sec: %.1lf  inds/s: %.1lf\n",
            (shm_tps_ptr->debug_gps_cmd_count_nmea + shm_tps_ptr->debug_gps_cmd_count_rtcm + shm_tps_ptr->debug_gps_cmd_count_ubx
             - prev_shm_tps.debug_gps_cmd_count_nmea - prev_shm_tps.debug_gps_cmd_count_rtcm - prev_shm_tps.debug_gps_cmd_count_ubx) / delta_secs,
            (shm_tps_ptr->debug_cnt_comm_send_tps_data_ind - prev_shm_tps.debug_cnt_comm_send_tps_data_ind) / delta_secs
        );

        printf("     msg types: %u nmea (%u bad-csum), %u rtcm, %u ubx (%u bad-csum, %u ignored, %u ack, %u nak, %u unknown)\n",
            shm_tps_ptr->debug_gps_cmd_count_nmea,
            shm_tps_ptr->debug_gps_cmd_count_nmea_bad_csum,
            shm_tps_ptr->debug_gps_cmd_count_rtcm,
            shm_tps_ptr->debug_gps_cmd_count_ubx,
            shm_tps_ptr->debug_gps_cmd_count_ubx_bad_csum,
            shm_tps_ptr->debug_gps_cmd_count_ubx_ignored,
            shm_tps_ptr->debug_gps_cmd_count_ubx_ack_ack,
            shm_tps_ptr->debug_gps_cmd_count_ubx_ack_nak,
            shm_tps_ptr->debug_gps_cmd_count_ubx_other);

        if (shm_tps_ptr->debug_gps_cmd_count_nmea > 0)
        {
            printf("     NMEA breakdown: RMC %u (%.1f/s), GGA %u (%.1f/s), GSA %u (%.1f/s), GST %u (ellipse_err: %u ok, %u no) (%.1f/s)\n",
                shm_tps_ptr->debug_gps_cmd_count_nmea_gprmc_valid_data,
                (shm_tps_ptr->debug_gps_cmd_count_nmea_gprmc_valid_data - prev_shm_tps.debug_gps_cmd_count_nmea_gprmc_valid_data) / delta_secs,
                shm_tps_ptr->debug_gps_cmd_count_nmea_gpgga_valid_data,
                (shm_tps_ptr->debug_gps_cmd_count_nmea_gpgga_valid_data - prev_shm_tps.debug_gps_cmd_count_nmea_gpgga_valid_data) / delta_secs,
                shm_tps_ptr->debug_gps_cmd_count_nmea_gpgsa_valid_data,
                (shm_tps_ptr->debug_gps_cmd_count_nmea_gpgsa_valid_data - prev_shm_tps.debug_gps_cmd_count_nmea_gpgsa_valid_data) / delta_secs,
                shm_tps_ptr->debug_gps_cmd_count_nmea_gpgst_valid_data,
                shm_tps_ptr->debug_gps_cmd_count_nmea_gpgst_w_valid_ellipse_err,
                shm_tps_ptr->debug_gps_cmd_count_nmea_gpgst_w_estim_ellipse_err,
                (shm_tps_ptr->debug_gps_cmd_count_nmea_gpgst_valid_data - prev_shm_tps.debug_gps_cmd_count_nmea_gpgst_valid_data) / delta_secs);
        }

        if (shm_tps_ptr->debug_gps_cmd_count_ubx > 0)
        {
            printf("     UBX breakdown: NAV-PVT %u (%.1f/s)",
                shm_tps_ptr->debug_gps_cmd_count_ubx_nav_pvt_total,
                (shm_tps_ptr->debug_gps_cmd_count_ubx_nav_pvt_total - prev_shm_tps.debug_gps_cmd_count_ubx_nav_pvt_total) / delta_secs);
            if (shm_tps_ptr->debug_gps_cmd_count_ubx_nav_pvt_unused > 0)
                printf(" (%u unused)", shm_tps_ptr->debug_gps_cmd_count_ubx_nav_pvt_unused);
            if (shm_tps_ptr->debug_gps_cmd_count_ubx_nav_pvt_no_fix > 0)
                printf(" (%u no fix)", shm_tps_ptr->debug_gps_cmd_count_ubx_nav_pvt_no_fix);
            if (shm_tps_ptr->debug_gps_cmd_count_ubx_nav_pvt_no_timeacc > 0)
                printf(" (%u no timeacc)", shm_tps_ptr->debug_gps_cmd_count_ubx_nav_pvt_no_timeacc);
            if (shm_tps_ptr->debug_gps_cmd_count_ubx_hnr_pvt_total > 0) {
                printf(", HNR-PVT %u (%.1f/s)",
                    shm_tps_ptr->debug_gps_cmd_count_ubx_hnr_pvt_total,
                    (shm_tps_ptr->debug_gps_cmd_count_ubx_hnr_pvt_total - prev_shm_tps.debug_gps_cmd_count_ubx_hnr_pvt_total) / delta_secs);
                if (shm_tps_ptr->debug_gps_cmd_count_ubx_hnr_pvt_unused > 0)
                    printf(" (%u unused)", shm_tps_ptr->debug_gps_cmd_count_ubx_hnr_pvt_unused);
            }
            printf(", TIMELS %u (%.1f/s), DOP %u (%.1f/s)",
                shm_tps_ptr->debug_gps_cmd_count_ubx_nav_timels,
                (shm_tps_ptr->debug_gps_cmd_count_ubx_nav_timels - prev_shm_tps.debug_gps_cmd_count_ubx_nav_timels) / delta_secs,
                shm_tps_ptr->debug_gps_cmd_count_ubx_nav_dop,
                (shm_tps_ptr->debug_gps_cmd_count_ubx_nav_dop - prev_shm_tps.debug_gps_cmd_count_ubx_nav_dop) / delta_secs);
            if (shm_tps_ptr->debug_gps_cmd_count_ubx_nav_status > 0) {
                printf(", STATUS %u (%.1f/s), SAT %u (%.1f/s)",
                    shm_tps_ptr->debug_gps_cmd_count_ubx_nav_status,
                    (shm_tps_ptr->debug_gps_cmd_count_ubx_nav_status - prev_shm_tps.debug_gps_cmd_count_ubx_nav_status) / delta_secs,
                    shm_tps_ptr->debug_gps_cmd_count_ubx_nav_sat,
                    (shm_tps_ptr->debug_gps_cmd_count_ubx_nav_sat - prev_shm_tps.debug_gps_cmd_count_ubx_nav_sat) / delta_secs);
            }
            if (shm_tps_ptr->debug_gps_cmd_count_ubx_mon_hw > 0) {
                printf(", MON-HW %u (%.1f/s)",
                    shm_tps_ptr->debug_gps_cmd_count_ubx_mon_hw,
                    ( shm_tps_ptr->debug_gps_cmd_count_ubx_mon_hw -
                      prev_shm_tps.debug_gps_cmd_count_ubx_mon_hw) / delta_secs);
            }
            if (shm_tps_ptr->debug_gps_cmd_count_ubx_nav_eell > 0) {
                printf(", EELL %u (%.1f/s)",
                shm_tps_ptr->debug_gps_cmd_count_ubx_nav_eell,
                (shm_tps_ptr->debug_gps_cmd_count_ubx_nav_eell - prev_shm_tps.debug_gps_cmd_count_ubx_nav_eell) / delta_secs);
            }
            if (shm_tps_ptr->debug_gps_cmd_count_ubx_esf_ins > 0 || shm_tps_ptr->debug_gps_cmd_count_ubx_esf_raw > 0) {
                printf(", ESF-INS %u (%.1f/s), ESF-RAW %u (%.1f/s)",
                    shm_tps_ptr->debug_gps_cmd_count_ubx_esf_ins,
                    (shm_tps_ptr->debug_gps_cmd_count_ubx_esf_ins - prev_shm_tps.debug_gps_cmd_count_ubx_esf_ins) / delta_secs,
                    shm_tps_ptr->debug_gps_cmd_count_ubx_esf_raw,
                    (shm_tps_ptr->debug_gps_cmd_count_ubx_esf_raw - prev_shm_tps.debug_gps_cmd_count_ubx_esf_raw) / delta_secs);
            }
            printf("\n");
            printf("     Ellipse_Error: Inds sent w ellipse error: %u from valid GST, %u from valid EELL, %u from computed estimates\n",
                            shm_tps_ptr->debug_gps_cnt_inds_built_from_ubx_w_valid_gst_ellipse_err,
                            shm_tps_ptr->debug_gps_cnt_inds_built_from_ubx_w_valid_eell_ellipse_err,
                            shm_tps_ptr->debug_gps_cnt_inds_built_from_ubx_w_estim_ellipse_err);
        }
        if (shm_tps_ptr->debug_gps_cmd_count_ubx_ack_nak > 0) {
            printf("     UBX-ACK-NAK: most recently on msg Class 0x%02x Id 0x%02x\n",
                shm_tps_ptr->debug_latest_nak_msg_classid, shm_tps_ptr->debug_latest_nak_msg_msgid);
        }
        if (shm_tps_ptr->debug_gps_cmd_count_ubx_inf_err > 0 ||
            shm_tps_ptr->debug_gps_cmd_count_ubx_inf_warn > 0 ||
            shm_tps_ptr->debug_gps_cmd_count_ubx_inf_rest > 0) {
                printf("  UBX-INFormation msgs: %u errors, %u warns, %u rest.  Text of last msg: %s\n",
                    shm_tps_ptr->debug_gps_cmd_count_ubx_inf_err,
                    shm_tps_ptr->debug_gps_cmd_count_ubx_inf_warn,
                    shm_tps_ptr->debug_gps_cmd_count_ubx_inf_rest,
                    shm_tps_ptr->debug_gps_last_inf_message);
        }
        if (shm_tps_ptr->debug_gps_cmd_count_ubx_mon_hw > 0) {
            printf("  Hardware Monitor:\n");
            printf("    RTC Calibrated: %s\n", shm_tps_ptr->mon_hw_rtc_is_calibrated ? "True" : "False");
            printf("    GNSS Noise Level: %u\n", shm_tps_ptr->mon_hw_noise_per_ms);
            printf("    AGC Monitor: %.1f%%\n", 100.0 * shm_tps_ptr->mon_hw_agc_count / MAX_AGC_MONITOR_VALUE);
            printf("    CW Jamming Indicator: %.1f%%\n", 100.0 * shm_tps_ptr->mon_hw_jamming_ind / 255.0);
            printf("    Jamming Status: %s\n", 
                tps_enum_to_str(TE_JAMMING_STATUS, shm_tps_ptr->mon_hw_jamming_status));
        }
        // NAV STATUS and SAT status
        printf("TPS Status:\n");
        printf("  Time State: %s   (%u nsec accuracy)\n", tps_enum_to_str(TE_TIME_SYNC_STATE, shm_tps_ptr->cur_tps_time_state), shm_tps_ptr->current_time_accuracy);
        printf("  Posn State: %s   (xx accuracy)\n", tps_enum_to_str(TE_POSN_SYNC_STATE, shm_tps_ptr->cur_tps_posn_state));
        printf("  NAV-STATUS:\n");
        printf("     gps_time_of_week: %u\n", shm_tps_ptr->nav_status.gps_time_of_week_msec);
        printf("     gps_fix_type: %u (%s)\n", shm_tps_ptr->nav_status.gps_fix_type,
                                                tps_enum_to_str(TE_GPS_FIX_TYPE, shm_tps_ptr->nav_status.gps_fix_type));
        printf("     gps_fix_ok: %s\n", shm_tps_ptr->nav_status.gps_fix_ok ? "True" : "False");
        printf("     differential_corrections_applied: %s\n", shm_tps_ptr->nav_status.differential_corrections_applied ? "True" : "False");
        printf("     week_number_valid: %s\n", shm_tps_ptr->nav_status.week_number_valid ? "True" : "False");
        printf("     time_of_week_valid: %s\n", shm_tps_ptr->nav_status.time_of_week_valid ? "True" : "False");
        printf("     differential_corrections_available: %s\n", shm_tps_ptr->nav_status.differential_corrections_available ? "True" : "False");
        printf("     carrier_solution_valid: %s\n", shm_tps_ptr->nav_status.carrier_solution_valid ? "True" : "False");
        printf("     map_matching_status: %s\n", tps_enum_to_str(TE_MAP_MATCH_STATUS, shm_tps_ptr->nav_status.map_matching_status));
        printf("     power_save_mode_state: %s\n", tps_enum_to_str(TE_POWER_SAVE_MODE_STATE, shm_tps_ptr->nav_status.power_save_mode_state));
        printf("     spoofing_detection_state: %s\n", tps_enum_to_str(TE_SPOOF_DETECT_STATE, shm_tps_ptr->nav_status.spoofing_detection_state));
        printf("     carrier_phase_range_solution_status: %s\n",tps_enum_to_str(TE_CARRIER_SOLN_STATUS, shm_tps_ptr->nav_status.carrier_phase_range_solution_status));
        printf("     time_to_first_fix = %u (ms)\n", shm_tps_ptr->nav_status.time_to_first_fix_msec);
        printf("     time_since_startup = %u (ms)\n", shm_tps_ptr->nav_status.time_since_startup_msec);
        printf("  Satellite Status:\n");
        printf("     gps_time_of_week: %u\n", shm_tps_ptr->nav_sat.gps_time_of_week_msec);
        printf("     number_of_satellites: %u  (PVT says it used %u)\n",
                        shm_tps_ptr->nav_sat.number_of_satellites,
                        shm_tps_ptr->debug_latest_number_of_sats);
        printf("     gnss identifier counts (id:cnt):");
        for (i=0; i<=MAX_SAT_GNSSID_VALUE; i++) {
            if (shm_tps_ptr->nav_sat.cnt_sats_w_gnss_XX[i] > 0) {
                printf(" %s:%u", tps_enum_to_str(TE_SAT_GNSSID, i), shm_tps_ptr->nav_sat.cnt_sats_w_gnss_XX[i]);
            }
        }
        printf("\n");
        printf("     carrier to noise ratios (cnr:cnt):");
        for (i=1; i<NUM_SAT_CNR_RATIOS; i++) {  // Start from 1, dont show CNR=0 not-ready-yet sats
            if (shm_tps_ptr->nav_sat.cnt_sats_w_carrier_to_noise_ratio_XX_dBHz[i] > 0) {
                printf(" %u0's:%u", i, shm_tps_ptr->nav_sat.cnt_sats_w_carrier_to_noise_ratio_XX_dBHz[i]);
            }
        }
        printf("\n");
        printf("     quality_indicator counts:\n");
        for (i=0; i<=MAX_SAT_QUALITY_INDICATOR_VALUE; i++) {
            if (shm_tps_ptr->nav_sat.cnt_sats_w_quality_ind_XX[i] > 0) {
                printf("       %3d @ %s  ", shm_tps_ptr->nav_sat.cnt_sats_w_quality_ind_XX[i],
                    tps_enum_to_str(TE_SAT_QUALITY_INDICATOR, i));
                for (j=0; j<=MAX_SAT_GNSSID_VALUE; j++) {
                    if (shm_tps_ptr->nav_sat.cnt_sats_w_quality_ind_XX_x_gnss_XX[i][j] > 0) {
                        printf(" %s:%u",
                            tps_enum_to_str(TE_SAT_GNSSID, j),
                            shm_tps_ptr->nav_sat.cnt_sats_w_quality_ind_XX_x_gnss_XX[i][j]);
                    }
                }
                printf("\n");
            }
        }
        printf("     sat health counts:\n");
        for (i=0; i<=MAX_SAT_HEALTH_VALUE; i++) {
            if (shm_tps_ptr->nav_sat.cnt_sats_w_health_XX[i] > 0) {
                printf("       %3d @ %s  ", shm_tps_ptr->nav_sat.cnt_sats_w_health_XX[i],
                    tps_enum_to_str(TE_SAT_HEALTH_FLAG, i));
                for (j=0; j<=MAX_SAT_GNSSID_VALUE; j++) {
                    if (shm_tps_ptr->nav_sat.cnt_sats_w_health_XX_x_gnss_XX[i][j] > 0) {
                        printf(" %s:%u", 
                            tps_enum_to_str(TE_SAT_GNSSID, j),
                            shm_tps_ptr->nav_sat.cnt_sats_w_health_XX_x_gnss_XX[i][j]);
                    }
                }
                printf("\n");
            }
        }
        printf("  Satellite Detail:\n");
        for (i=0; i<shm_tps_ptr->sat_detail.cnt_gps_sats; i++) {
            printf("      GPS %2d: used %u hlth %u qual %u cnr %2u prRes %5d elev %2d azim %3d\n",
                shm_tps_ptr->sat_detail.gps[i].svId,
                shm_tps_ptr->sat_detail.gps[i].svUsed,
                shm_tps_ptr->sat_detail.gps[i].health,
                shm_tps_ptr->sat_detail.gps[i].quality,
                shm_tps_ptr->sat_detail.gps[i].CNR,
                shm_tps_ptr->sat_detail.gps[i].prRes,
                shm_tps_ptr->sat_detail.gps[i].elev,
                shm_tps_ptr->sat_detail.gps[i].azim);
        }
        for (i=0; i<shm_tps_ptr->sat_detail.cnt_glo_sats; i++) {
            printf("      GLO %2d: used %u hlth %u qual %u cnr %2u prRes %5d elev %2d azim %3d\n",
                shm_tps_ptr->sat_detail.glo[i].svId,
                shm_tps_ptr->sat_detail.glo[i].svUsed,
                shm_tps_ptr->sat_detail.glo[i].health,
                shm_tps_ptr->sat_detail.glo[i].quality,
                shm_tps_ptr->sat_detail.glo[i].CNR,
                shm_tps_ptr->sat_detail.glo[i].prRes,
                shm_tps_ptr->sat_detail.glo[i].elev,
                shm_tps_ptr->sat_detail.glo[i].azim);
        }

        // NAV-DOP - Dilution of Precision
        printf("  NAV-DOP:\n");
        printf("     Geometric DOP: %.2f\n", shm_tps_ptr->nav_dop.gDOP / 100.0);
        printf("     Position DOP: %.2f\n", shm_tps_ptr->nav_dop.pDOP / 100.0);
        printf("     Time DOP: %.2f\n", shm_tps_ptr->nav_dop.tDOP / 100.0);
        printf("     Vertical DOP: %.2f\n", shm_tps_ptr->nav_dop.vDOP / 100.0);
        printf("     Horizontal DOP: %.2f\n", shm_tps_ptr->nav_dop.hDOP / 100.0);
        printf("     Northing DOP: %.2f\n", shm_tps_ptr->nav_dop.nDOP / 100.0);
        printf("     Easting DOP: %.2f\n", shm_tps_ptr->nav_dop.eDOP / 100.0);

        // Detail on losing our gps position fix
        if (shm_tps_ptr->debug_cnt_gps_fix_lost) {
            printf("  GPSS Fix Losses:\n");
            printf("     Number of times GNSS fix has been lost: %u\n", shm_tps_ptr->debug_cnt_gps_fix_lost);
            if (shm_tps_ptr->current_lost_fix_period_length > 0) {
                printf("   GNSS currently without fix, %d x no fix, started at uptime %lu secs\n",
                    shm_tps_ptr->current_lost_fix_period_length,
                    shm_tps_ptr->current_fix_loss_started_at_uptime);
            }
            if (shm_tps_ptr->latest_lost_fix_index != 0 || shm_tps_ptr->tracked_lost_fix_is_loss_number[0] != 0) {
                printf("    Past GNSS fix losses:\n");
                for (i=0; i<NUM_TRACKED_LOST_FIXES; i++) {
                    cur_idx = (NUM_TRACKED_LOST_FIXES + shm_tps_ptr->latest_lost_fix_index - i) % NUM_TRACKED_LOST_FIXES;
                    if (shm_tps_ptr->tracked_lost_fix_is_loss_number[i] != 0) {
                        printf("      Loss # %u, %u nofixes, duration %lu secs, started at %lus uptime\n",
                            shm_tps_ptr->tracked_lost_fix_is_loss_number[cur_idx],
                            shm_tps_ptr->tracked_lost_fix_period_length[cur_idx],
                            shm_tps_ptr->tracked_lost_fix_end_uptime[cur_idx] - 
                                shm_tps_ptr->tracked_lost_fix_start_uptime[cur_idx],
                            shm_tps_ptr->tracked_lost_fix_start_uptime[cur_idx]);
                    } else {
                        break;
                    }
                }
            }
        }
        // Detail on losing our time accuracy
        if (shm_tps_ptr->debug_cnt_timeacc_lost) {
            printf("  Time Accuracy Losses:\n");
            printf("     Number of times Time Accuracy has been lost: %u\n", shm_tps_ptr->debug_cnt_timeacc_lost);
            if (shm_tps_ptr->current_lost_fix_period_length > 0) {
                printf("   GNSS currently without time accuracy, %d x no timeacc, started at uptime %lu secs\n",
                    shm_tps_ptr->current_lost_timeacc_period_length,
                    shm_tps_ptr->current_timeacc_loss_started_at_uptime);
            }
            if (shm_tps_ptr->latest_lost_timeacc_index != 0 || shm_tps_ptr->tracked_lost_timeacc_is_loss_number[0] != 0) {
                printf("    Past Time Accuracy losses:\n");
                for (i=0; i<NUM_TRACKED_LOST_FIXES; i++) {
                    cur_idx = (NUM_TRACKED_LOST_FIXES + shm_tps_ptr->latest_lost_timeacc_index - i) % NUM_TRACKED_LOST_FIXES;
                    if (shm_tps_ptr->tracked_lost_fix_is_loss_number[i] != 0) {
                        printf("      Loss # %u, %u nofixes, duration %lu secs, started at %lus uptime\n",
                            shm_tps_ptr->tracked_lost_timeacc_is_loss_number[cur_idx],
                            shm_tps_ptr->tracked_lost_timeacc_period_length[cur_idx],
                            shm_tps_ptr->tracked_lost_timeacc_end_uptime[cur_idx] - 
                                shm_tps_ptr->tracked_lost_timeacc_start_uptime[cur_idx],
                            shm_tps_ptr->tracked_lost_timeacc_start_uptime[cur_idx]);
                    } else {
                        break;
                    }
                }
            }
        }
        // PPS
        if (delta_secs >= 1) {   // Must be > 1 because we are dividing by int(delta_secs)
            printf("  PPS Interupt handling:\n");
            printf("     totalcount=%u PPS/sec=%.1lf  CpuCycles=%lu, CpuCyc/sec=%lu\n",
                shm_tps_ptr->debug_pps_cnt_interrupts,
                (shm_tps_ptr->debug_pps_cnt_interrupts - prev_shm_tps.debug_pps_cnt_interrupts) / (float64_t) ((int32_t) delta_secs),
                shm_tps_ptr->index_of_most_recent == 1 ? shm_tps_ptr->clock_cycles_1 : shm_tps_ptr->clock_cycles_2,
                (
                    (shm_tps_ptr->index_of_most_recent == 1 ? shm_tps_ptr->clock_cycles_1 : shm_tps_ptr->clock_cycles_2)
                    - 
                    (prev_shm_tps.index_of_most_recent == 1 ? prev_shm_tps.clock_cycles_1 : prev_shm_tps.clock_cycles_2)
                ) / ((int32_t) delta_secs)
            );
        }
        printf("  Hourly PPS Interrupt counts (want 3600) (cur hour");
        if (shm_tps_ptr->uptime_hours_for_pps_prev_hour[shm_tps_ptr->index_for_latest_pps_prev_hour_data]>0) {
            printf(", then from %u full hours uptime",
                shm_tps_ptr->uptime_hours_for_pps_prev_hour[shm_tps_ptr->index_for_latest_pps_prev_hour_data]);
        }
        printf("):\n     %u", shm_tps_ptr->debug_pps_cnt_interrupts - shm_tps_ptr->prev_hour_pps_cnt);
        for (i=0; i<NUM_HOURS_TRACKED_PPS_INTERRUPTS; i++) {
            cur_idx = (NUM_HOURS_TRACKED_PPS_INTERRUPTS + shm_tps_ptr->index_for_latest_pps_prev_hour_data - i) % NUM_HOURS_TRACKED_PPS_INTERRUPTS;
            if (shm_tps_ptr->track_pps_interrupts_per_hour[cur_idx] > 0) {
                printf(" %u", shm_tps_ptr->track_pps_interrupts_per_hour[cur_idx]);
            }
        }
        printf("\n");
        // Time
        printf("  Time Maintenance:\n");
        printf("     Number of sats = %u\n", shm_tps_ptr->debug_latest_number_of_sats);
        if (shm_tps_ptr->debug_power_up_to_first_fix_acquired_secs > 0) {
            printf("     Time to Posn Fix: %lu seconds\n", shm_tps_ptr->debug_power_up_to_first_fix_acquired_secs);
        } else {
            printf("     Time to Posn Fix: Fix not acquired yet\n");
        }
        if (shm_tps_ptr->debug_power_up_to_first_timeacc_acquired_secs > 0) {
            printf("     Time to First Time Accuracy: %lu seconds\n", shm_tps_ptr->debug_power_up_to_first_timeacc_acquired_secs);
        } else {
            printf("     Time to First Time Accuracy: Time Accuracy has not been acquired yet\n");
        }
        if (shm_tps_ptr->debug_power_up_to_first_acceptable_timeacc_secs) {
            printf("     Time to Acceptable Time Accuracy: %lu seconds\n", shm_tps_ptr->debug_power_up_to_first_acceptable_timeacc_secs);
        } else {
            printf("     Time to Acceptable Time Accuracy: Acceptable Time Accuracy has not been acquired yet\n");
        }
        printf("     Number of UBX-MGA-GPS-UTC sent: %u\n", shm_tps_ptr->debug_gps_cmd_count_ubx_mga_gps);
        printf("     Number of times gps and system time out of sync: %u\n", shm_tps_ptr->debug_cnt_gps_time_and_system_time_out_of_sync);
        printf("     Number of time sets (want all to match): %u settimeofday, %u clocksettime, %u rtcsettime\n",
                        shm_tps_ptr->debug_cnt_set_time_with_settimeofday,
                        shm_tps_ptr->debug_cnt_set_time_with_clocksettime,
                        shm_tps_ptr->debug_cnt_set_time_with_rtcsettime);
        printf("     Number of time adjustments: %u\n", shm_tps_ptr->debug_cnt_time_adjustments);

        printf("     Time set & adjustment failures: %u (mktime) %u (settimeofday) %u (clocksettime) %u (getgmtime) %u (openrtc) %u (rtcsettime)\n",
            shm_tps_ptr->debug_cnt_time_adjustment_fail_mktime,
            shm_tps_ptr->debug_cnt_time_adjustment_fail_settimeofday,
            shm_tps_ptr->debug_cnt_time_adjustment_fail_clocksettime,
            shm_tps_ptr->debug_cnt_time_adjustment_fail_getgmtime,
            shm_tps_ptr->debug_cnt_time_adjustment_fail_openrtcdevice,
            shm_tps_ptr->debug_cnt_time_adjustment_fail_rtcsettime);
        // Tracked count of time corrections
        printf("  Time Maintenance Counts:  (Note, values are going backwards from present)\n");
        printf("    Hourly Sets,Adjs (cur hour");
        if (shm_tps_ptr->uptime_hours_for_timefix_prev_hour[shm_tps_ptr->index_for_latest_timefix_prev_hour_data] > 0) {
            printf(", then from %u full hours uptime", shm_tps_ptr->uptime_hours_for_timefix_prev_hour[shm_tps_ptr->index_for_latest_timefix_prev_hour_data]);
        }
        printf("):\n      %u,%u",
            shm_tps_ptr->debug_cnt_set_time_with_settimeofday - shm_tps_ptr->prev_hour_sets_count,
            shm_tps_ptr->debug_cnt_time_adjustments - shm_tps_ptr->prev_hour_adjs_count);
        for (i=0; i<NUM_HOURS_TRACKED_TIME_CORRECTIONS; i++) {
            cur_idx = (NUM_HOURS_TRACKED_TIME_CORRECTIONS + shm_tps_ptr->index_for_latest_timefix_prev_hour_data - i) % NUM_HOURS_TRACKED_TIME_CORRECTIONS;
            if (shm_tps_ptr->cnt_time_adjustments_in_prev_hour[cur_idx] > 0) {
                printf(" %u,%u",
                    shm_tps_ptr->cnt_set_times_in_prev_hour[cur_idx],
                    shm_tps_ptr->cnt_time_adjustments_in_prev_hour[cur_idx]);
            }
        }
        printf("\n");
        printf("    Daily Sets/Adjs (cur day");
        if (shm_tps_ptr->uptime_days_for_timefix_prev_day[shm_tps_ptr->index_for_latest_timefix_prev_day_data]>0) {
            printf(", then from %u days uptime", shm_tps_ptr->uptime_days_for_timefix_prev_day[shm_tps_ptr->index_for_latest_timefix_prev_day_data]);
        }
        printf("):\n      %u,%u",
            shm_tps_ptr->debug_cnt_set_time_with_settimeofday - shm_tps_ptr->prev_day_sets_count,
            shm_tps_ptr->debug_cnt_time_adjustments - shm_tps_ptr->prev_day_adjs_count);
        for (i=0; i<NUM_DAYS_TRACKED_TIME_CORRECTIONS; i++) {
            cur_idx = (NUM_DAYS_TRACKED_TIME_CORRECTIONS + shm_tps_ptr->index_for_latest_timefix_prev_day_data - i) % NUM_DAYS_TRACKED_TIME_CORRECTIONS;
            if (shm_tps_ptr->cnt_time_adjustments_in_prev_day[cur_idx] > 0) {
                printf(" %u,%u",
                    shm_tps_ptr->cnt_set_times_in_prev_day[cur_idx],
                    shm_tps_ptr->cnt_time_adjustments_in_prev_day[cur_idx]);
            }
        }
        printf("\n");
        // Tracked time accuracy
        printf("  Time Accuracy (nsec):  (Note, values are going backwards from present)\n");
        printf("    Current Time Accuracy: %u\n", shm_tps_ptr->current_time_accuracy);
        printf("    Min,Max since first fix: %u,%u (nsec)\n",
                        shm_tps_ptr->time_accuracy_min_since_first_fix,
                        shm_tps_ptr->time_accuracy_max_since_first_fix);
        printf("    Avg/Minute (from %u full mins uptime):\n", shm_tps_ptr->uptime_minutes_for_prev_minute[shm_tps_ptr->index_for_latest_prev_minute_data]);
        printf("     ");    // 5 spaces not 4 bc minute values are single-spaced apart
        if (shm_tps_ptr->current_minute_num_samples > 0) {
             printf(" %lu", shm_tps_ptr->current_minute_total_of_timeacc_samples / shm_tps_ptr->current_minute_num_samples);
        }
        if (shm_tps_ptr->uptime_minutes_for_prev_minute[shm_tps_ptr->index_for_latest_prev_minute_data] > 0) {
            for (i=0; i<NUM_MINS_TRACKED_TIME_ACCURACY; i++) {
                cur_idx = (NUM_MINS_TRACKED_TIME_ACCURACY + shm_tps_ptr->index_for_latest_prev_minute_data - i) % NUM_MINS_TRACKED_TIME_ACCURACY;
                if (shm_tps_ptr->time_accuracy_avg_in_prev_minute[cur_idx] > 0) {
                    printf(" %u", shm_tps_ptr->time_accuracy_avg_in_prev_minute[cur_idx]);
                }
            }
        }
        printf("\n");
        printf("    Min,Avg,Max per Hour (%u full hours uptime):\n", shm_tps_ptr->uptime_hours_for_prev_hour[shm_tps_ptr->index_for_latest_prev_hour_data]);
        printf("    "); // 4 space indent
        if (shm_tps_ptr->current_hour_num_samples > 0) {
             printf("  %u,%lu,%u",
                 shm_tps_ptr->current_hour_lowest_time_accuracy,
                 shm_tps_ptr->current_hour_total_of_timeacc_samples / shm_tps_ptr->current_hour_num_samples,
                 shm_tps_ptr->current_hour_highest_time_accuracy);
        }
        for (i=0; i<NUM_HOURS_TRACKED_TIME_ACCURACY; i++) {
            cur_idx = (NUM_HOURS_TRACKED_TIME_ACCURACY + shm_tps_ptr->index_for_latest_prev_hour_data - i) % NUM_HOURS_TRACKED_TIME_ACCURACY;
            if (shm_tps_ptr->time_accuracy_max_in_prev_hour[cur_idx] > 0) {
                printf("  %u,%u,%u",
                    shm_tps_ptr->time_accuracy_min_in_prev_hour[cur_idx],
                    shm_tps_ptr->time_accuracy_avg_in_prev_hour[cur_idx],
                    shm_tps_ptr->time_accuracy_max_in_prev_hour[cur_idx]);
            }
        }
        printf("\n");
        printf("    Min,Avg,Max per Day (%u full days uptime):\n", shm_tps_ptr->uptime_days_for_prev_day[shm_tps_ptr->index_for_latest_prev_day_data]);
        printf("    ");
        if (shm_tps_ptr->current_day_num_samples > 0) {
             printf("  %u,%lu,%u",
                 shm_tps_ptr->current_day_lowest_time_accuracy,
                 shm_tps_ptr->current_day_total_of_timeacc_samples / shm_tps_ptr->current_day_num_samples,
                 shm_tps_ptr->current_day_highest_time_accuracy);
        }
        for (i=0; i<NUM_DAYS_TRACKED_TIME_ACCURACY; i++) {
            cur_idx = (NUM_DAYS_TRACKED_TIME_ACCURACY + shm_tps_ptr->index_for_latest_prev_day_data - i) % NUM_DAYS_TRACKED_TIME_ACCURACY;
            if (shm_tps_ptr->time_accuracy_max_in_prev_day[cur_idx] > 0) {
                printf(" %u", shm_tps_ptr->time_accuracy_avg_in_prev_day[cur_idx]);
            }
        }
        printf("\n");
        printf("    UBX-NAV-PVT with Time Accuracy outside threshold: %u\n",
                        shm_tps_ptr->cnt_timeacc_exceeded_threshold);
        printf("    Seconds with Time Accuracy outside threshold: %u\n",
                        shm_tps_ptr->cnt_seconds_where_timeacc_exceeded_threshold);

        // ESF
        if (shm_tps_ptr->debug_gps_cmd_count_ubx_esf_raw - prev_shm_tps.debug_gps_cmd_count_ubx_esf_raw > 0) {
            printf("  ESF-RAW: Temp: %0.3lf  tow:%u\n",
                shm_tps_ptr->debug_esf_temp,
                shm_tps_ptr->debug_esf_iTOW);
            printf("           AngRates: X:%0.3lf (%06x)  Y:%0.3lf (%06x)  Z:%0.3lf (%06x)\n",
                shm_tps_ptr->debug_esf_x_ang_rate,
                shm_tps_ptr->debug_esf_x_ang_rate_hex,
                shm_tps_ptr->debug_esf_y_ang_rate,
                shm_tps_ptr->debug_esf_y_ang_rate_hex,
                shm_tps_ptr->debug_esf_z_ang_rate,
                shm_tps_ptr->debug_esf_z_ang_rate_hex);
            printf("           Accel: X:%0.3lf (%06x)  Y:%0.3lf (%06x)  Z:%0.3lf (%06x)\n",
                shm_tps_ptr->debug_esf_x_accel,
                shm_tps_ptr->debug_esf_x_accel_hex,
                shm_tps_ptr->debug_esf_y_accel,
                shm_tps_ptr->debug_esf_y_accel_hex,
                shm_tps_ptr->debug_esf_z_accel,
                shm_tps_ptr->debug_esf_z_accel_hex);
        }
#if defined(ENABLE_SR)
        // SR if we're in playback or recording mode
        if (shm_sr_ptr->sr_mode != SR_NONE_MODE_CHAR) {
            if (delta_secs >= 1) {   // Must be > 1 because we are dividing by int(delta_secs)
              printf("  SR: (mode='%c', recordingOn=%d)\n",
                      shm_sr_ptr->sr_mode, shm_sr_ptr->recordingON);
                printf("     raw msgs: %u sent (%.1f/s) to SR, %u recv (%.1f/s) from SR\n",
                    shm_tps_ptr->debug_cnt_sr_raw_sends,
                    (shm_tps_ptr->debug_cnt_sr_raw_sends - prev_shm_tps.debug_cnt_sr_raw_sends) / delta_secs,
                    shm_tps_ptr->debug_cnt_sr_raw_recvs,
                    (shm_tps_ptr->debug_cnt_sr_raw_recvs - prev_shm_tps.debug_cnt_sr_raw_recvs) / delta_secs);
                printf("     ind msgs: %u sent (%.1f/s) to SR, %u recv (%.1f/s) from SR\n",
                    shm_tps_ptr->debug_cnt_sr_ind_sends,
                    (shm_tps_ptr->debug_cnt_sr_ind_sends - prev_shm_tps.debug_cnt_sr_ind_sends) / delta_secs,
                    shm_tps_ptr->debug_cnt_sr_ind_recvs,
                    (shm_tps_ptr->debug_cnt_sr_ind_recvs - prev_shm_tps.debug_cnt_sr_ind_recvs) / delta_secs);
            }
        }
#endif
        // And how many Sent
        printf("  Sent: Parser thread assembled %u data indicators, %u sent to main thread, sent inds/s: %.1lf\n", 
            shm_tps_ptr->debug_cnt_comm_send_tps_data_ind,
            shm_tps_ptr->debug_cnt_inds_sent_to_tps,
            (shm_tps_ptr->debug_cnt_inds_sent_to_tps - prev_shm_tps.debug_cnt_inds_sent_to_tps) / delta_secs);
        // Leap Seconds
        printf("  LeapSeconds: Valid: %s  CurValue: %u   CurLeapSec Source: %s (%u)\n",
            shm_tps_ptr->leap_secs_valid ? "True" : "INVALID",
            shm_tps_ptr->curLeapSecs,
            shm_tps_ptr->tpsSrcOfCurLeapsecs == LSSRC_NONE ? "None"
               : (shm_tps_ptr->tpsSrcOfCurLeapsecs == LSSRC_UBLOX) ? "Live GPS"
               : (shm_tps_ptr->tpsSrcOfCurLeapsecs == LSSRC_SAVED_DATA) ? "Stored Data" : "InvalidValue",
            shm_tps_ptr->tpsSrcOfCurLeapsecs);
        printf("               Counts of LeapSec Sources Used for Tps Inds: %u Live GPS, %u cached, %u Stored Data\n",
            shm_tps_ptr->debug_gps_count_leapsec_src_ublox,
            shm_tps_ptr->debug_gps_count_leapsec_src_cache,
            shm_tps_ptr->debug_gps_count_leapsec_src_persist);
        if (shm_tps_ptr->debug_gps_cmd_count_ubx_nav_timels_invalid > 0) {
            printf("               Count of TIMELS msgs that UBlox sent as Invalid: %u\n",
                    shm_tps_ptr->debug_gps_cmd_count_ubx_nav_timels_invalid);
        }
    }

    // Save current time & stats for next time
    memcpy(&prev_shm_tps, shm_tps_ptr, sizeof(shm_tps_t));
    prev_status_timeval.tv_sec = now_timeval.tv_sec;
    prev_status_timeval.tv_usec = now_timeval.tv_usec;

    // Rate testing report
    if (tps_registered == TRUE) {
        printDataRateReport();
    }
}


static void helpMenu()
{
    printf("\n");
#if defined(ENABLE_SR)
    if (SR_IN_PLAYBACK_MODE(shm_sr_ptr)) {
        fprintf(stderr, "           %s [=====~~>     SR in Playback mode     <~~=====] %s\n\n", 
                EBOLDBLINKRED, ERESET);
    }
#endif
    printf("TPS Error status: NMEA cksum:%d, Parsing:%d, Reglistfull:%d, Write:%d, Read:%d\n",
            tps_error_type_nmea_cksum, tps_error_type_parsing, tps_error_type_list_full,
            tps_error_type_write, tps_error_type_read);
    printf("Enter:\n");
    printf("  r - Register for tps data ind without RTCM data\n");
    printf("  d - Deregister for tps data ind\n");
    printf("  q - reQuest tps data response\n");
    printf("  c - Register for tps data ind with RTCM data\n");
    printf("  w - Register for TPS raw data indication\n");   
    printf("  v - Deregister TPS raw data indication\n");   
    printf("  l - Log TPS data to CSV file without RTCM data\n");
    printf("  s - Stop Logging TPS data to CSV file\n");
    printf("\n");  
    printf("  n - DBG: display NMEA msgs from serial port on serial console\n"); 
    printf("  m - DBG: display RTCM msgs from serial port on serial console\n"); 
    printf("  t - DBG: display Time sync stats from TPS on serial console\n"); 
    printf("  z - DBG: display RAW Messages from TPS on serial console\n"); 
    printf("  a - DBG: show All debug msgs on serial console\n");
    printf("  y - DBG: Manually set value of TPS debug output mask\n");
    printf("  e - DBG: disablE all debug output\n");
    printf("  g - Manually send a GPS configuration command\n");
    printf("  k - Print TPS status summary\n");
#if defined(ENABLE_WHEELTIX)
    printf("  T - Send wheel ticks\n");
#endif
    printf("  R - Toggle Rate tracking mode on/off\n");
    printf("  h - Help (display this menu again)\n");
    printf("  f - Issue Cold Start to GNSS chip.\n");
//    printf("  i - Toggle Stop/Start GNSS.\n");
    printf("  j - Dump SPI information and stats.\n");
    printf("  o - Dump Ublox Chip H/W status.\n");
    printf("  p - Dump Ublox Config.\n");
    printf("  x - eXit\n");
}

/* Takes user's input to use the TPS services */
static void tps_user_input()
{
    char_t             ch;
    uint32_t           debugOutput = 0;
    uint32_t           i; 
    char_t             msgHeader[MAX_RAW_MSG_HDR_LEN];
    tpsResultCodeType  ret;
    char_t             cmdbuff[100];

    printf("TPS Test Application.  Enter 'h' for help.'\n");

#if defined(ENABLE_SR)
    if (SR_IN_PLAYBACK_MODE(shm_sr_ptr)) {
        fprintf(stderr, "           %s [=====~~>     SR in Playback mode     <~~=====] %s\n", 
                EBOLDBLINKRED, ERESET);
    }
#endif
    /* while loop is used to have this main thread alive,
       in addition to getting the TPS service operations (commands)
       going. */
    while (1)
    {
        /* Read New line character and dump it*/
        printf("-> ");
        if (fgets(cmdbuff, sizeof(cmdbuff), stdin) != NULL)
        {
            ch = cmdbuff[0];
        } else {
            ch = 0;
        }
        
        switch (ch)
        {
        case 'r': 
            if (LoggingEnabled) {
                PrintDataToConsole = 1;
            } else {
                if (!PrintDataToConsole) {
                    /* Register this app with the callback function.
                    Once the application is registered, the callback function passed
                    here will be called once per second with the TPS data. */
#ifdef ENABLE_SR
                    ret = WSU_TPS_REGISTER(printDataIndication);
#else
                    ret = wsuTpsRegister(printDataIndication);
#endif

                    if (ret != TPS_SUCCESS) {
                        printf("TPS register failed\n");
                    } else {
                        tps_registered = TRUE;
                        time(&start_time);
                        PrintDataToConsole = 1;
                    }
                }
            }
            break;
            
        case 'd': 
            if (PrintDataToConsole)
                stopConsolePrint();
            break;

        case 'q': 
            /* Request one TPS data with the callback function.
               The callback function passed here will be called
               with one TPS data. */
            wsuTpsRequest(printDataConfirmation);
            break;

        case 'c': 
            /* Register this app with the callback function.
               Once the application is registered, the callback function passed
               here will be called once per second with the TPS data. */
            wsuTpsRegisterRTCMHandler(printRTCMData);
#ifdef ENABLE_SR
            WSU_TPS_REGISTER(printDataIndication);
#else
            wsuTpsRegister(printDataIndication);
#endif
            tps_registered = TRUE;
            break;
        
        case 'w': 
            GlobalDataPrinting = 0;
                
            /* Register this app with the callback function.
            Once the application is registered, the callback function passed
            here will be called with RAW TPS data. */
            memset(msgHeader,0,sizeof(msgHeader));
            printf("Enter RAW Message header(NMEA) to register and press enter:");
           
            for (i=0;i<MAX_RAW_MSG_HDR_LEN;i++) {
                scanf("%c",&msgHeader[i]);
                if (msgHeader[i] == '\n') {
                    msgHeader[i] = '\0';
                    break;
                }
            }

            if (msgHeader != '\0') {            // KTODO: Are they meaning [0] != 0 ????  WHAT DOES THIS DO?
#ifdef ENABLE_SR
                WSU_TPS_REGISTER_RAW_MSG_HANDLER(printRAWData,msgHeader,strlen(msgHeader),0);
#else
                wsuTpsRegisterRawMsgHandler(printRAWData,msgHeader,strlen(msgHeader),0);
#endif
            }
            
            GlobalDataPrinting = 1;
            break;

         case 'v': 
            GlobalDataPrinting = 0;
             
            /* Register this app with the callback function.
               Once the application is registered, the callback function passed
               here will be called with RAW TPS data. */
            
            memset(msgHeader,0,sizeof(msgHeader));
            printf("Enter RAW Message header(NMEA) to deregister and press enter:");
           
            for (i=0;i<MAX_RAW_MSG_HDR_LEN;i++) {
                scanf("%c",&msgHeader[i]);
                if (msgHeader[i] == '\n') {
                    msgHeader[i] = '\0';
                    break;
                }
            }

            if (msgHeader != '\0') {
#ifdef ENABLE_SR
                WSU_TPS_DEREGISTER_RAW_MSG_HANDLER(msgHeader,strlen(msgHeader));
#else
                wsuTpsDeregisterRawMsgHandler(msgHeader,strlen(msgHeader));
#endif
            }
            
            GlobalDataPrinting = 1;
            break;

        case 'x': 
            if (PrintDataToConsole)
                stopConsolePrint();

            if (LoggingEnabled)
                stopLogging();

            if (tps_registered == TRUE)
                wsuTpsDeregister();
        
            /* Must call this to terminate the service */
#ifdef ENABLE_SR
            WSU_TPS_TERM();
            WSU_SR_DEINIT_TPS(sr_tps_sockfd);
#else
            wsuTpsTerm();
#endif
            exit (1);
            break;

        case 'n': 
           /* Show NMEA messages from serial port */ 
            debugOutput=DBG_OUTPUT_NMEA;  
            wsuTpsShowDebugMsg(debugOutput); 
	        break;

        case 'm': 
           /* Show NMEA messages from serial port */ 
            debugOutput=DBG_OUTPUT_RTCM;  
	        wsuTpsShowDebugMsg(debugOutput); 
	        break;

        case 't': 
           /* Show time sync stats from TPS */ 
            debugOutput=DBG_OUTPUT_TIME;  
    	    wsuTpsShowDebugMsg(debugOutput); 
            break;
#if defined(ENABLE_WHEELTIX)
        case 'T':
        {
            WheelTickData_t     tick_data;
            struct timeval      tv;

            memset(&tick_data, 0, sizeof(WheelTickData_t));

           /* Send Wheel Ticks */
            debugOutput=DBG_OUTPUT_ALL;
            for (i = 0; i < 255; i++)
            {
                //SEND WHEEL TICK INFORMATION TO IMU
                tick_data.wheel[TPS_LR_IDX].dir = FORWARD;
                tick_data.wheel[TPS_LR_IDX].tick = i;
                tick_data.wheel[TPS_LR_IDX].valid = TRUE;

                tick_data.wheel[TPS_RR_IDX].dir = FORWARD;
                tick_data.wheel[TPS_RR_IDX].tick = (i+50) % 256;
                tick_data.wheel[TPS_RR_IDX].valid = TRUE;

                tick_data.wheel[TPS_LF_IDX].valid = FALSE;
                tick_data.wheel[TPS_RF_IDX].valid = FALSE;

                // NOTE: This calculation is done to convert time tag to ms. If u-blox config changes
                //       this value may need to be changed.
                gettimeofday(&tv, NULL);
                tick_data.timeTag = tv.tv_sec * 1000 + tv.tv_usec / 1000;

                wsuTpsSendWheelTicks(tick_data);

                printf("Send wheel tick value = %d\n", i);

                usleep(30000);
            }
        }
        break;
#endif /* ENABLE_WHEELTIX */
        case 'a': 
           /* Show All debug messages from TPS */ 
            debugOutput=DBG_OUTPUT_ALL;  
	        wsuTpsShowDebugMsg(debugOutput); 
            break;


        case 'e': 
           /* Disable debug messages from TPS */ 
            debugOutput=DBG_OUTPUT_DISABLE;  
	        wsuTpsShowDebugMsg(debugOutput); 
            break;
        case 'f':
            wsuTpsColdStartuBlox();
            break;
//        case 'i':
//            wsuTpsStopuBlox();
//            break;
        case 'j':
            wsuTpsDumpSPIstats();
            break;
        case 'o':   
            wsuTpsDumpGNSSHWstatus(); 
            break;
        case 'p':
            wsuTpsDumpUbloxConfigs();
            break;
        case 'y': 
           /* Manually input uint32_t value for tps debug output mask */
            GlobalDataPrinting = 0;
            uint32_t curMask = wsuTpsGetDebugOutputMask();
            uint32_t newMask;

            printf("Current tpsDebugOutputMask = 0x%04x\n", curMask);
            printf ("Bit Masks for various TPS Debug Messages:  (from tps_types.h):\n");
            printf("           DBG_OUTPUT_NMEA     0x%04x\n", DBG_OUTPUT_NMEA);
            printf("           DBG_OUTPUT_TIME     0x%04x\n", DBG_OUTPUT_TIME);
            printf("           DBG_OUTPUT_RTCM     0x%04x\n", DBG_OUTPUT_RTCM);
            printf("           DBG_OUTPUT_RAW_MSG  0x%04x\n", DBG_OUTPUT_RAW_MSG);
            printf("           DBG_OUTPUT_ADJTIME  0x%04x\n", DBG_OUTPUT_ADJTIME);
            printf("           DBG_OUTPUT_PPS      0x%04x\n", DBG_OUTPUT_PPS);
            printf("           DBG_OUTPUT_TPSGEN   0x%04x\n", DBG_OUTPUT_TPSGEN);
            printf("           DBG_OUTPUT_SR       0x%04x\n", DBG_OUTPUT_SR);
            printf("\n");
            printf("Enter new tpsDebugOutput mask value: 0x");
           
            for (i=0;i<MAX_RAW_MSG_HDR_LEN;i++) {
                scanf("%c",&msgHeader[i]);
                if (msgHeader[i] == '\n') {
                    msgHeader[i] = '\0';
                    break;
                }
            }

            if (sscanf(msgHeader, "%x", &newMask) == 1)
            {
            	wsuTpsShowDebugMsg(newMask);

            	curMask = wsuTpsGetDebugOutputMask();
            	printf("Requested 0x%04x mask, tps confirms new value of 0x%04x\n", newMask, curMask);
            } else if (msgHeader[0] != 0) {
            	printf("Error parsing mask value 0f '%s', please try again.\n", msgHeader);
            }

            GlobalDataPrinting = 1;
            break;

        case 'z': 
           /* Disable debug messages from TPS */ 
            debugOutput=DBG_OUTPUT_RAW_MSG;  
	        wsuTpsShowDebugMsg(debugOutput); 
            break;

        case 'g':
           /* User inputs a string to be sent to the GPS device */
            GlobalDataPrinting = 0;

            memset(msgHeader,0,sizeof(msgHeader));
            printf("Enter NMEA GPS Command: ");

            for (i=0;i<MAX_RAW_MSG_HDR_LEN;i++) {
                scanf("%c",&msgHeader[i]);
                if (msgHeader[i] == '\n') {
                    msgHeader[i] = '\0';
                    break;
                }
            }

            if (msgHeader[0] != '\0') {
                wsuTpsSndGpsOutput(msgHeader, i);
            }

            GlobalDataPrinting = 1;
            break;

        case 'k': 
	        printStatusMonitorReport(1); 
            break;

        case 'R':
            if (tps_registered == FALSE) {
                printf("How often (secs) do you want the rate report (0 for doing it through 'k' command)? ");
                for (i=0;i<MAX_RAW_MSG_HDR_LEN;i++) {
                    scanf("%c",&msgHeader[i]);
                    if (msgHeader[i] == '\n') {
                        msgHeader[i] = '\0';
                        break;
                    }
                }
                int32_t input_int;
                if (sscanf(msgHeader, "%d", &input_int) != 1)
                {
                    rate_report_interval = 0;
                    printf("   --> Error parsing input '%s', please retry command again.\n", msgHeader);
                    printf("Rate testing was not enabled.\n");
                } else {
                    rate_report_interval = rate_report_countdown = SDHGPS_EXPECTED_RATE * input_int;     // Convert seconds to callbacks
                    wsuTpsRegister(testDataIndicationRate);
                    tps_registered = TRUE;
                    printf("Rate testing enabled.\n");
                }
            } else {
                wsuTpsDeregister();
                tps_registered = FALSE;
                printf("Rate testing disabled.\n");
            }
            break;

        case 'l':
           
            if (!LoggingEnabled) {
                /* Start logging */
                if (PrintDataToConsole) { 
                    if (create_log_file() == 0)
                        break;
                    LoggingEnabled = 1; 
                } else {
#ifdef ENABLE_SR
                    ret = WSU_TPS_REGISTER(printDataIndication);
#else
                    ret = wsuTpsRegister(printDataIndication);
#endif

                    if (ret != TPS_SUCCESS) {
                        printf("TPS register failed\n");
                        LoggingEnabled = 0; 
                    } else {
                        tps_registered = TRUE;
                        if (create_log_file() == 0)
                            break;
                        LoggingEnabled = 1; 
                    }
                }
            }
            break;

        case 's':
            if (LoggingEnabled)
                stopLogging(); 
            break;

        case 'h':
            helpMenu();
            break;
            
        default:
            printf("   Enter 'h' for the help menu.\n");
            break;
        
        }
    }
}


/* Main Test */
int32_t main(int32_t argc, char_t **argv)
{
    int32_t  ret;
    int32_t  report_mode_period = 0;

    /* Usage, args */
    if (argc >= 2 && (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0)) {
        printf("USAGE: tpstestapp [-h] [--rpt #secs]\n");
        exit(0);
    }
    if (argc == 3 && strcmp(argv[1], "--rpt") == 0) {
        report_mode_period = atoi(argv[2]);
        if (report_mode_period > 0) {
            printf("tpstestapp: report_mode activated, report period of %d seconds\n", report_mode_period);
        } else {
            printf("invalid report period.  Aborting.\n");
            exit(1);
        }
    }

    /* Initialize the TPS service. 
       This API function will generate a thread to read TPS data
       from the TPS services. */
#ifdef ENABLE_SR
    if ((ret = WSU_SR_INIT_TPS(&SR_in_record_mode, &SR_in_playback_mode, &sr_tps_sockfd)) != TPS_SUCCESS) {
        printf("srInitTPS failed %d\n", ret);
        exit (1);
    }
    ret = WSU_TPS_INIT();
#else
    ret = wsuTpsInit();
#endif
    wsuTpsRegisterErrorHandler(errorHandler);

    if (ret != TPS_SUCCESS)
    {
        printf("Init failed\n");
#ifdef ENABLE_SR
        WSU_SR_DEINIT_TPS(sr_tps_sockfd);
#endif
        exit (1);
    }
#if (ENABLE_SR)
    // Connect to SH shared memory
    if ((shm_sr_ptr = wsu_share_init(sizeof(shm_sr_t), SHM_SR_PATH)) == NULL) {
        printf("%s : Could not connect to SR shared memory.\n", __func__);
    }
#endif
    // Connect to TPS shared memory
    if ((shm_tps_ptr = wsu_share_init(sizeof(shm_tps_t), SHM_TPS_PATH)) == NULL) {
        printf("%s : Could not connect to TPS's shared memory.\n", __func__);
    }

     strcpy(gps_title, "NumTimeAdjs,Recv Time (ms),Recv Cycles,CurReadSleeps,");
     strcat(gps_title, "Timestamp (ms),NMEA Status,Fix Quality,Fix Mode,Date (ddmmyy),Time (hhmmss.ss),Latitude (deg),Longitude (deg),Altitude (m),Ground speed (mps),Course (Deg),HDOP,PDOP,Latitude Error (m),Longitude Error (m),Semi-major axis Error (m),Semi-minor axis Error (m),Orientation of Semi-major axis (Deg),Number of Satellites,Diff Age (Sec),TPS NMEA checksum error,TPS msg parsing error,TPS App list full error,TPS GPS port write error,TPS GPS port read error\n");

    // Grab a starting point for the status report; doesn't output anything
    printStatusMonitorReport(0);
    
    // command-line mode runs a period report
    if (report_mode_period > 0) {
        sleep(report_mode_period);
        printStatusMonitorReport(1); 

    // Interactive mode
    } else {
    /* This gets user's input to do each operation such as register,
       deregister, request, etc.  It's important to have this main
       thread alive if want to have the thread generated from
       wsuTpsInit() is alive and reading the TPS data from the TPS services.
       This sample app waits on user's input. */
        tps_user_input();
    }

    // Release SHM */
    wsu_share_kill(shm_tps_ptr, sizeof(shm_tps_t));
#if defined(ENABLE_SR)
    wsu_share_kill(shm_sr_ptr, sizeof(shm_sr_t));
#endif
    return 0;
}
