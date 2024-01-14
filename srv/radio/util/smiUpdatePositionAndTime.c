/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Filename: smiUpdatePositionAndTime.c                                       */
/*  Purpose: Utility to update the current position and time for Aerolink     */
/*                                                                            */
/* Copyright (C) 2019 DENSO International America, Inc.  All Rights Reserved. */
/*                                                                            */
/* Revision History:                                                          */
/*                                                                            */
/* Date        Author        Comments                                         */
/* -------------------------------------------------------------------------- */
/* 2019-06-28  VROLLINGER    Initial revision.                                */
/*                                                                            */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/* Include Files                                                              */
/*----------------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <sys/time.h>
#include "rs.h"
#include "tps_api.h"
#include "alsmi_api.h"
#include "wsu_sharedmem.h"
#include "shm_tps.h"

/*----------------------------------------------------------------------------*/
/* Constants                                                                  */
/*----------------------------------------------------------------------------*/
#define LEAP_SECONDS_BEFORE_2004 13

/*----------------------------------------------------------------------------*/
/* Local variables                                                            */
/*----------------------------------------------------------------------------*/
static bool_t     mainLoop = TRUE;
static shm_tps_t *shm_tps_ptr;

/*----------------------------------------------------------------------------*/
/* Functions                                                                  */
/*----------------------------------------------------------------------------*/

/**
** Function:  tpsCallback
** @brief  The TPS callback function
** @param  tpsData -- [input]Data from TPS
** @return void
**
** Details: After registering for TPS service, this callback function is called
**          periodically. It contains information about the position and time.
**/
void tpsCallback(tpsDataType *tpsData)
{
    static bool_t   firstTime = TRUE;
    static int16_t  leapSeconds;
    static uint64_t lastUpdateSecs = 0;
    struct timeval  tv;

    if (!tpsData->valid) {
        return;
    }

    gettimeofday(&tv, NULL);

    if (tv.tv_sec == lastUpdateSecs) {
        return;
    }

    lastUpdateSecs = tv.tv_sec;

    if (shm_tps_ptr->leap_secs_valid) {
        /* Get leap seconds from TPS shared memory if available */
        leapSeconds = shm_tps_ptr->curLeapSecs - LEAP_SECONDS_BEFORE_2004;
    }
    else if (firstTime) {
        /* If leap seconds not available from TPS shared memory, calculate
         * them */
        struct timeval tv;

        firstTime = FALSE;
        gettimeofday(&tv, NULL);

        if (tv.tv_sec < 1483228800) { /* Jan 1, 2017, 00:00:00 UTC */
            leapSeconds = 4;
        }
        else {
            leapSeconds = 5;
        }
    }

    /* VJR WILLBEREVISITED Use 0x348 for the country code for now; it's what
     * all of the OBS examples use */
    smiUpdatePositionAndTime(tpsData->latitude, tpsData->longitude,
                             tpsData->altitude, leapSeconds, 0x348);
}

/**
** Function:  exitHandler
** @brief  Program exit handler
** @param  signal -- [input]The signal used to terminate the program.
** @return void
**
** Details: If the routine has not been previously called, it sets mainLoop to
**          FALSE so that the program will terminate.
**/
void exitHandler(int signal)
{
    if (mainLoop) {
        printf("smiUpdatePositionAndTime: Exit handler is called.\n");
        mainLoop = FALSE;
    }
}

int main(int argc, char **argv)
{
    tpsResultCodeType tpsRet;

    /* Delay to allow radioServices and tps to come up */
    sleep(2);

    /* Connect to TPS shared memory */
    shm_tps_ptr = (shm_tps_t *)wsu_share_init(sizeof(shm_tps_t), SHM_TPS_PATH);

    if (shm_tps_ptr == NULL) {
        printf("%s : Could not connect to TPS's shared memory.\n", __func__);
    }

    /* Initialize TPS */
    tpsRet = wsuTpsInit();

    if (tpsRet != TPS_SUCCESS) {
        printf("smiUpdatePositionAndTime: TPS init failed (%s)\n",
                tpsResultCodeType2Str(tpsRet));
        return -1;
    }

    /* Register for TPS */
    tpsRet = wsuTpsRegister(tpsCallback);

    if (tpsRet != TPS_SUCCESS) {
        printf("smiUpdatePositionAndTime: TPS register failed (%s)\n",
               tpsResultCodeType2Str(tpsRet));
        wsuTpsTerm();
        return -1;
    }

    signal(SIGINT,  exitHandler);
    signal(SIGTERM, exitHandler);

    /* Just wait for the termination signal */
    while (mainLoop) {
        sleep(1);
    }

    /* De-register from TPS */
    tpsRet = wsuTpsDeregister();

    if (tpsRet != TPS_SUCCESS) {
        printf("smiUpdatePositionAndTime: TPS deregister failed (%s)\n",
               tpsResultCodeType2Str(tpsRet));
    }

    /* Terminate TPS */
    wsuTpsTerm();
    /* Disconnect from TPS shared memory */
    wsu_share_kill(shm_tps_ptr, sizeof(shm_tps_t));
    return 0;
}

