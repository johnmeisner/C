/***************************************************************************
 *   Copyright (C) 2019 by DENSO LA Labs                                   *
 *                                                                         *
 *            W I R E L E S S   S A F E T Y   U N I T   ( W S U )          *
 *                                                                         *
 *                      L I N U X   U T I L I T I E S                      *
 *                                                                         *
 ***************************************************************************/
#ifndef WSU_UTIL_H
#define WSU_UTIL_H

#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <signal.h>

#include "dn_types.h"

#define WSU_APP_LOGS_PATH "/tmp/applogs"
#define WSU_DBG_LOGS_PATH "/tmp/dbglogs"

#define FRAME_BUFFER_ZERO_DEVICE    "/dev/fb0"
// #define WSU_PLATFORM_1_0             1
// #define WSU_PLATFORM_1_5             2
// #define WSU_PLATFORM_MINI            3
// #define WSU_PLATFORM_WSU_5001        4
// #define WSU_PLATFORM_AUTOTALKS_GM    5

#define WSU_LED_MODE_CAMP               0
#define WSU_LED_MODE_HIA                1
#define WSU_LED_MODE_ASD                2

#define ETH_DEV_NAME   "eth0"
#define ATH0_DEV_NAME  "ath0"
#define ATH1_DEV_NAME  "ath1"
#define WIFI0_DEV_NAME "wifi0"
#define WIFI1_DEV_NAME "wifi1"
#define CW_LLC0_DEV_NAME "cw-llc0"
#define CW_LLC1_DEV_NAME "cw-llc1"

#define WSU_DBG_LOG(fmt, args...)  if (dbgFlag && (flog != NULL)) { fprintf(flog,"%s: ",wsuTimeEpochMSToLogStringMacro()); fprintf(flog, fmt, ##args); fflush(flog);}
#define WSU_CSV_LOG(fmt, args...)  if (logFlag && (fcsv != NULL)) { fprintf(fcsv, fmt, ##args); fflush(fcsv); }


enum wsuUtilReturnsE {
    WSU_UTIL_OK = 0,
    WSU_UTIL_FAILED
};


enum wsuCreateFileOptionsE {
    wsuCreateFileNone           = 0,
    wsuCreateFileAddDateTime    = 1,
    wsuCreateBinaryFile         = 4
};

typedef struct {
    int app_enable;
    int create_log;
} wsuUtilLogStateT;



/**
 * @brief Returns the milliseconds in minute.
 * @param [in] epoMs  milliseconds since epoch 
 * @return Returns the milliseconds in minute.
 */
uint16_t wsuUtilGetMsInMinute(uint64_t epoMs);

/**
 * @brief This function returns the number of milliseconds since epoch.
 * @param [in] tv  Optional pointer to a struct timeval containing time, or NULL to use current time.
 * @return This function returns the number of milliseconds since epoch.
 */
uint64_t wsuUtilGetTime(struct timeval *tv);

/** 
 * @brief   Convert milliseconds-in-minute to milliseconds-since-epoch
 * @return  Returns the number of milliseconds-since-epoch
 */
uint64_t wsuUtilMsInMin2EpochMs(uint64_t curTime, uint32_t msInMin);


/**
 * @brief Utility function 
 *
 * new support for V2V
 * Sets up and starts a periodic timer
 *
 * This utility function creates a repeating, fixed-duration timer that
 * first fires deltaTimeMs into the future, and continues firing every
 * deltaTimeMs after that.
 *
 * @param [in ] deltaTimeMs timer duration in millseconds 
 * @param [out ] timerID    pointer to receive the timer ID for created timer
 * @param [in ] handler     address of signal handler
 * @param [out] tv          pointer to hold the itimerspec for the created timer
 * 
 * @return Returns WSU_UTIL_OK or WSU_UTIL_FAILED depending on timer creation & setup.
 * @note  Converted to use thread timers
 * @note  Handler is defined as type
 * 		void			(*__sigev_notify_function)(union sigval);
 *    in usr/include/sys/siginfo.h
 */
unsigned int wsuUtilSetupTimer(uint32_t deltaTimeMs,
                               timer_t *timerID,
                               void (*handler)(int), 
                               struct itimerspec *tv);

/**
 * @brief Utility function
 *
 * Restarts periodic countdowns on a previously-existing but stopped timer
 *
 * This utility function restarts a stopped, pre-existing timer.  It
 * schedules the timer to first fire deltaTimeMs into the future, and to
 * continue firing every deltaTimeMs after that.
 *
 * @param [in] deltaTimeMs  timer duration in millseconds
 * @param [in] timerID      timer ID for stopped timer
 *
 * @note Note that timerID is not a pointer, whereas in wsuUtilSetupTimer() it is a pointer.
 * 
 * @return Returns WSU_UTIL_OK, or WSU_UTIL_FAILED if timer restart failed.
 */
unsigned int wsuUtilRestartTimer(uint32_t deltaTimeMs, timer_t timerID);

/**
 * @brief Utility function
 *
 * Stops the countdown on a timer
 *
 * This utility function stops a pre-existing timer.
 *
 * @param [in] timerID      timer ID for stopped timer
 *
 * @note Note that timerID is not a pointer, whereas in wsuUtilSetupTimer() it is a pointer.
 *
 * @return Returns WSU_UTIL_OK, or WSU_UTIL_FAILED if timer stop failed.
 */
unsigned int wsuUtilStopTimer(timer_t timerID);



/**
 * Append a date/time stamp to a file name
 * 
 * @author  DENSO International America, Inc.
 * 
 * @param filename 
 */
void wsuUtilAppendTimeDateToFileName(char *filename);


/**
 * Create a file
 * 
 * @author  DENSO International America, Inc.
 * 
 * @param fd           (o) return File pointer 
 * @param app_str       (i) app string 
 * @param path          (i) file path
 * @param suffix        (i) file suffix (appended to filename)
 * @param options       (i) creation options 
 */
unsigned int wsuUtilCreateFile(FILE **fd,
                       char_t * commonLogFilePrefix,
                       char_t * appStr, 
                       char_t * path, 
                       char_t * suffix, 
                       uint8_t options);

/**
 * close a file
 * 
 * @author  DENSO International America, Inc.
 * 
 * @param fd        (i) pointer to a file descriptor
 */
void wsuUtilCloseFile(FILE **fd);


/**
 * create a log file. 
 * Moved over from V2V utils. 
 * 
 * @author  DENSO International America, Inc.
 * 
 * @param logt                  (i) pointer to filled 
 *                              wsuUtilLogStateT
 * @param csvlog 
 * @param dbglog 
 * @param commonLogFilePrefix 
 * @param app_enable 
 * @param logflg 
 * @param debuglog 
 * @param global_log 
 * @param global_debuglog 
 * @param title 
 * @param app_str 
 */
void wsuUtilCreateNewLogFiles(wsuUtilLogStateT *logt,
                              FILE **csvlog,
                              FILE **dbglog,
                              char_t * commonLogFilePrefix,
                              bool_t app_enable,
                              bool_t logflg,
                              bool_t debuglog,
                              bool_t global_log,
                              bool_t global_debuglog,
                              char_t * title,
                              char_t * app_str,
                              char_t * applogs_dirname,
                              char_t * logs_dirname
                              );

/*
 * Returns CPU statistics.
 * The function should not be called
 * more than once a second, otherwise
 * the values returned will not be very accurate
 * 
 * If an error occurs or successive calls
 * are too fast then 0 is returned. Otherwise
 * 1 is returned.
 *
 * It will always return an error the very first time
 * it is called during a program invocation because
 * the first call is needed to seed the variables
 * containing the cpu statistics. 
 */
unsigned int 
wsu_getCpuUtil (char         * timestamp,
                float32_t        * user,
                float32_t        * sys, 
                float32_t        * nice,
                float32_t        * idle,
                float32_t        * busy,
                float32_t        * iowait,
                float32_t        * handling_ints,
                float32_t        * soft_irqs);

/**-----------------------------------------------------------------------------
** @brief  Utility to setup signal handler
** @param  handler - signal handler function address
** @param  sig - signal ID number
** @param  flags - signal initialization flags
**----------------------------------------------------------------------------*/
void wsuUtilSetupSignalHandler(void (*handler)(int), int sig, int flags);

#endif
