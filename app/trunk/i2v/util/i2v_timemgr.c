/**************************************************************************
 *                                                                        *
 *     File Name:   i2v_timemgr.c                                         *
 *     Author:                                                            *
 *         DENSO International America, Inc.                              *
 *         North America Research Laboratory, California Office           *
 *         3252 Business Park Drive                                       *
 *         Vista, CA 92081                                                *
 *                                                                        *
 **************************************************************************/
/* this module should only have a single instance (implemented as such)
 * its initial purpose is just to override the system clock with a cfg
 * value - it supports two modes: updating (incrementing an internal clock)
 * or using the fixed value continually
 *
 * the module has to overwrite what GPS provides (updating each second)
 */
#include <stdlib.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <pthread.h>
#include <errno.h>
#include "i2v_util.h"
#include "i2v_general.h"
#include "i2v_shm_master.h"
#if defined(MY_UNIT_TEST)
#include "stubs.h"
#endif
/* every 10 ms - much faster than gps which updates
   on the second boundary - however gps used to do it
   at 1 ms frequency (fyi);  primary purpose of fast freq
   is to prevent (or artificially create if timeout isn't
   exact) drift when in incrementing mode */
#define  TIME_MGR_TIMEOUT     10000
#define  ONE_SEC_MICRO        1000000

static pthread_t timeMgrThrId;
static bool_t    looper;
static bool_t    isCounter;
static int32_t   cfgsec;
static int32_t   cfgusec;

/* utility function for use when parsing cfg file - almost
   a duplicate of original found in amh */

/**
 * Convert an epoch MS time stamp to a log-formatted time string
 * 
 * @author  DENSO International America, Inc.
 * 
 * @param timeStr           (o) output buffer for conversion  
 * @param size              (i) size of output buffer
 * @param epochMS           (i) epoch in MS 
 * 
 * @return char_t* 
 */ 
char_t *i2vTimeEpochMSToLogString(char_t *timeStr, int32_t size, uint64_t epochMS)
{
    struct tm epochTime;
    time_t  epochSeconds = 0;
    uint32_t ms;
    char_t   msStr[8];

    if(NULL == timeStr)
        return NULL;

    ms = epochMS % 1000;
    sprintf(msStr, ".%03d", ms);
    epochSeconds = (time_t)(epochMS / 1000);
    gmtime_r(&epochSeconds, &epochTime);
    strftime(timeStr, size - strlen(msStr), "%Y/%m/%d-%H:%M:%S", &epochTime);
    strcat(timeStr, msStr);

    return timeStr;
}

void i2vUtilUpdateTimeStr(char_t GCC_UNUSED_VAR *tag, char_t GCC_UNUSED_VAR *parsedItem, char_t *value,
                          void *configItem, uint32_t *status)
{
    i2vUtilTimeParams *data;
    struct tm tval;
    char_t *tok = NULL;
    bool_t valid = WFALSE;

    if (NULL == status) return;

    if ((NULL == value) || (NULL == configItem)) {
        *status = I2V_RETURN_FAIL;
        return;
    }

    data = (i2vUtilTimeParams *)configItem;
    /* assumed string format: MM/DD/YYYY,mm:hh:ss:[uuuuuu]<optional usec> 
       the junk below is just to use the CONVERT_TO_UTC function which needs yday
       that is set by mktime */
    memset(&tval, 0, sizeof(tval));

    tok = strtok((char_t *)value, "/");
    if (NULL != tok) {
        tval.tm_mon = (int)(strtoul(tok, NULL, 10) - 1);
        tok = strtok(NULL, "/");
    } else {
        goto setresult;
    }
    if (NULL != tok) {
        tval.tm_mday = (int)(strtoul(tok, NULL, 10));
        tok = strtok(NULL, ",");
    } else {
        goto setresult;
    }
    if (NULL != tok) {
        tval.tm_year = (int)(strtoul(tok, NULL, 10) - 1900);
        tok = strtok(NULL, ":");
    } else {
        goto setresult;
    }
    if (NULL != tok) {
        tval.tm_hour = (int)(strtoul(tok, NULL, 10));
        tok = strtok(NULL, ":");
    } else {
        goto setresult;
    }
    if (NULL != tok) {
        tval.tm_sec = (int)(strtoul(tok, NULL, 10));
        tok = strtok(NULL, ":");
        valid = WTRUE;
    } else {
        goto setresult;
    }
    /* optional param */
    if (NULL != tok) {
        data->epochusecs = (int)(strtoul(tok, NULL, 10));
        if (data->epochusecs >= ONE_SEC_MICRO) {
            printf("%d-%s: invalid usec value: %d; reducing to real value\n", getpid(), __FUNCTION__, data->epochusecs);
            data->epochusecs = ONE_SEC_MICRO - 1;
        }
    }

    mktime(&tval);
    data->epochsecs = CONVERT_TO_UTC((tval.tm_year + 1900), tval.tm_yday, tval.tm_hour, tval.tm_min, tval.tm_sec);

setresult:
    *status = (valid) ? I2V_RETURN_OK : I2V_RETURN_FAIL;
}

/* to update the system clock at hard coded frequency */
static void *timeMgrThread(void __attribute__((unused)) *arg)
{
    int32_t origclocksec = 0, origclockusec = 0;
    int32_t countersec = 0, counterusec = 0;
    struct  timeval tv;
    struct  timezone tz;   /* just for safety */

    memset(&tv, 0, sizeof(tv));
    memset(&tz, 0, sizeof(tz));

    /* get original time - used later for restoration */
    if (gettimeofday(&tv, &tz) < 0) {
        printf("i2v time mgr failed to properly initialize - will not do anything to system time\n");
        pthread_exit(NULL);
    }

    origclocksec = tv.tv_sec;
    origclockusec = tv.tv_usec;
    /* set tv with update value */
    tv.tv_sec = cfgsec;
    tv.tv_usec = cfgusec;

    while (looper) {
        if (settimeofday(&tv, &tz) < 0) {

        }
        if (isCounter) {
            usleep(TIME_MGR_TIMEOUT - 4);   /* assuming 4 us for operations below - this is a guess */
        } else {
            usleep(TIME_MGR_TIMEOUT - 2);   /* assuming 2 us for operations below - this is a guess */
        }
        /* always maintain track of seconds */
        counterusec += TIME_MGR_TIMEOUT;
        if (counterusec >= ONE_SEC_MICRO) {
            countersec++;
            counterusec -= ONE_SEC_MICRO;
        }
        if (isCounter) {
            tv.tv_sec = cfgsec + countersec;
            tv.tv_usec = cfgusec + counterusec;
            if (tv.tv_usec >= ONE_SEC_MICRO) {
                tv.tv_sec++;
                tv.tv_usec -= ONE_SEC_MICRO;
            }
        }
    }

    /* consider restoring? - yes just in case */
    tv.tv_sec = origclocksec + countersec;
    tv.tv_usec = origclockusec + counterusec;
    if (tv.tv_usec >= ONE_SEC_MICRO) {
        tv.tv_sec++;
        tv.tv_usec -= ONE_SEC_MICRO;
    }
    if (settimeofday(&tv, &tz) < 0) {
        printf("\ni2v time mgr unable to restore system clock - sorry...\n");
    }

    pthread_exit(NULL);
}

/* input shared mem path and fixedTrue (using fixed value
   when WTRUE) */
bool_t timeMgrActivate(char_t *shmpath, i2vUtilTimeParams *data, bool_t fixedTrue)
{
    i2vShmMasterT *shmPtr;
    bool_t result = WFALSE;

    if (NULL == shmpath) {
        return WFALSE;
    }

    if (NULL == (shmPtr = wsu_share_init(sizeof(i2vShmMasterT), (char_t *)shmpath))) {
        printf("i2v timemgr failed to init shm\n");
        return WFALSE;
    }

    WSU_SEM_LOCKR(&shmPtr->cfgData.h.ch_lock);
    result = shmPtr->cfgData.tmgrRunning;
    WSU_SEM_UNLOCKR(&shmPtr->cfgData.h.ch_lock);

    if (!result) {
        looper = WTRUE;
        isCounter = !fixedTrue;
        cfgsec = data->epochsecs;
        cfgusec = data->epochusecs;
        result = (pthread_create(&timeMgrThrId, NULL, timeMgrThread, NULL) == -1) ? 
            WFALSE : WTRUE;

        if (result) {
            /* 20190208: QNX pthread_join fails; detach instead (will properly exit) */
            pthread_detach(timeMgrThrId);
            WSU_SEM_LOCKR(&shmPtr->cfgData.h.ch_lock);
            shmPtr->cfgData.tmgrRunning = result;
            WSU_SEM_UNLOCKR(&shmPtr->cfgData.h.ch_lock);
        }
    }

    wsu_share_kill(shmPtr, sizeof(i2vShmMasterT));

    return result;
}

void timeMgrDeactive(void)
{
    looper = WFALSE;

    usleep(1000);  /* sleep 1 ms to allow shutdown of thread */
}

