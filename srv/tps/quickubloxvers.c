/*---------------------------------------------------------------------------*/
/*                                                                           */
/*    Filename: quickubloxvers.c                                             */
/*     Purpose: Report what versions TPS has collected                       */
/*                                                                           */
/* Copyright (C) 2021 DENSO International America, Inc.                      */
/*                                                                           */
/*---------------------------------------------------------------------------*/


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>


/* These list of include files must not be altered */
#include "dn_types.h"
// #include "tps_api.h"
#include "shm_tps.h"
#include "wsu_sharedmem.h"  // for wsu_share_init() and friends


int32_t main(int32_t argc, char_t **argv)
{
    shm_tps_t       *shm_tps_ptr;

    // Connect to TPS shared memory
    if ((shm_tps_ptr = wsu_share_init(sizeof(shm_tps_t), SHM_TPS_PATH)) == NULL) {
        printf("%s : Could not connect to TPS's shared memory.\n", __func__);
    }

    if (shm_tps_ptr->ublox_firmware_version_str[0]) {
        printf("UBlox Firmware Version: %s\n", shm_tps_ptr->ublox_firmware_version_str);
    }
    if (shm_tps_ptr->ublox_config_version_str[0]) {
        printf("UBlox Configuration Version: %s\n", shm_tps_ptr->ublox_config_version_str);
    }

    // Release SHM */
    wsu_share_kill(shm_tps_ptr, sizeof(shm_tps_t));

    return 0;
}

