/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Filename: waitForRadioServicesBooted.c                                     */
/*                                                                            */
/* Copyright (C) 2020 DENSO International America, Inc.                       */
/*                                                                            */
/* Description: Waits for everything to be initialized except for the C-V2X   */
/*              radio, which may take several minutes.                        */
/* Revision History:                                                          */
/*                                                                            */
/* Date        Author        Comments                                         */
/* -------------------------------------------------------------------------- */
/* [2020-07-17][VROLLINGER]  Initial revision                                 */
/*----------------------------------------------------------------------------*/
#include <stdio.h>
#include "shm_rsk.h"
#include "wsu_sharedmem.h"

int main(int argc, char **argv)
{
    shm_rsk_t *shm_rsk_ptr;
    uint32_t wait_time = 0;
    uint32_t bootupState;

    /* Mount the Radio Services shared memory region */
    shm_rsk_ptr = (shm_rsk_t *)wsu_share_init(sizeof(shm_rsk_t), RSK_SHM_PATH);

    if (shm_rsk_ptr == NULL) {
        printf("Failed to create Radio_ns shared memory area\n");
        return -1;
    }

    bootupState = shm_rsk_ptr->bootupState;
    printf("waitForRadioServicesBooted: Initial bootupState = 0x%x\n",
           bootupState);

    /* Wait for radioServices shared memory to be initialized */
    while ((bootupState & 0xffff0000) != 0x12340000) {
        usleep(10000);
        wait_time += 10;
        bootupState = shm_rsk_ptr->bootupState;
    }

    printf("waitForRadioServicesBooted: Intermediate bootupState = 0x%x; wait_time = %dms\n",
           bootupState, wait_time);

    /* Wait for everything to be initialized except for the C-V2X
     * radio, which may take several minutes. */
    while ((bootupState & 0xf) < 0x6) {
        usleep(10000);
        wait_time += 10;
        bootupState = shm_rsk_ptr->bootupState;
    }

    printf("waitForRadioServicesBooted: Final bootupState = 0x%x; wait_time = %dms\n",
           shm_rsk_ptr->bootupState, wait_time);
    /* Release the Radio Services shared memory region */
    wsu_share_kill(shm_rsk_ptr, sizeof(*shm_rsk_ptr));
    return 0;
}

