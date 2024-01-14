/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Filename: quickradio.c                                                     */
/*  Purpose: Utility to collect tx/rx counts for webgui                       */
/*                                                                            */
/* Copyright (C) 2020 DENSO International America, Inc.  All Rights Reserved. */
/*                                                                            */
/* Revision History:                                                          */
/*                                                                            */
/* Date        Author        Comments                                         */
/* -------------------------------------------------------------------------- */
/* 2020-12-09  KFRANKEL      Created from srv/radio/util/nsstats.c            */
/*                                                                            */
/*----------------------------------------------------------------------------*/


#include <stdio.h>
#include "wsu_sharedmem.h"
#include "shm_rsk.h"

int main()
{
    shm_rsk_t *rskShmPtr;
    int32_t i,j;

    /* Mount Radio_ns shared memory */
    rskShmPtr = wsu_share_init(sizeof(shm_rsk_t), RSK_SHM_PATH);
    if (rskShmPtr == NULL) {
        printf("-1,-1,-1,-1,Radio SHM error\n");
        return 0;
    }

    /* Print TX and RX statistics: CV2X and DSRC.  Combine USA + EU numbers so code works in both continents */
    printf("%d,\n", rskShmPtr->RISReqCnt.SendWSMReq[RT_CV2X] + rskShmPtr->RISReqCnt.SendEUPktReq[RT_CV2X]); // CV2X Send WSM, USA + EU
    printf("%d,\n", rskShmPtr->RISReqCnt.SendWSMReq[RT_DSRC] + rskShmPtr->RISReqCnt.SendEUPktReq[RT_DSRC]); // DSRC Send WSM, USA + EU
    printf("%d,\n", rskShmPtr->RISIndCnt.ReceiveWSMData[RT_CV2X] + rskShmPtr->RISIndCnt.ReceiveEUData[RT_CV2X]); // CV2X Recv WSM, USA + EU
    printf("%d,\n", rskShmPtr->RISIndCnt.ReceiveWSMData[RT_DSRC] + rskShmPtr->RISIndCnt.ReceiveEUData[RT_DSRC]); // DSRC Recv WSM, USA + EU

    /* Print channels of radio service(s) */
    for (i=0; i < MAX_RADIO_TYPES; i++) {
        for (j=0; j < MAX_RADIOS; j++) {
            if (rskShmPtr->wsuNsCurService[i][j].ServiceRunning) {
                printf("%s:%d ", (i == 0) ? "CV2X" : "DSRC", rskShmPtr->wsuNsCurService[i][j].CurChannel);
            }
        }
    }
    printf(",\n");

    /* Unmount Radio_ns shared memory */
    wsu_share_kill(rskShmPtr, sizeof(shm_rsk_t));

    return 0;
}
