/*---------------------------------------------------------------------------*/
/*                                                                           */
/*    Filename: sous_print.c                                                 */
/*     Purpose: Print state and all current shared data of sous service      */
/*  Department: DENSO North America Research & Development                   */
/*                                                                           */
/* Copyright (C) 2022 DENSO International America, Inc.                      */
/*                                                                           */
/*     Project: V2X Special Projects                                         */
/* Description: Print contents of sous's shared memory                       */
/*                                                                           */
/*---------------------------------------------------------------------------*/

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>         // for sleep()

#include "wsu_sharedmem.h"
#include "dn_types.h"
#include "shm_sous.h"

char * ant_status_enum_to_str(uint8_t antenna_status_enum)
{
    static char buff[100];
    switch (antenna_status_enum) {
        case ANTCON_UNSET: return "Unset";
        case ANTCON_SUPPLY_FAIL: return "Supply Fail";
        case ANTCON_NOT_CONNECTED: return "Not Connected";
        case ANTCON_SHORT: return "Short";
        case ANTCON_CONNECTED: return "Connected";
        case ANTCON_UNKNOWN_VALUE: return "Unknown Value";
    }
    snprintf(buff, sizeof(buff), "Illegal Value %d", antenna_status_enum);
    return buff;
}

int main(int argc, char **argv)
{
    shm_sous_t * shm_sous_ptr;
    uint32_t update_count;
    int flag_webgui_format=0;
    int i;

    if (argc > 1 && !(strncmp(argv[1],"-h",2) && strncmp(argv[1],"--h",3))) {
        printf("Usage: sous_report [-h|--help] [--webgui-format]\n");
        exit(0);
    }

    if (argc > 1 && !strcmp(argv[1],"--webgui-format")) {
        ++flag_webgui_format;
    }

    // Connect to sous's shared memory
    if ((shm_sous_ptr = wsu_share_init(sizeof(shm_sous_t), SHM_SOUS_PATH)) == NULL) {
        printf("ERROR : Could not open SOUS's shared memory\n");
        exit(1);
    }

    // Copy RH850 data and re-copy if it updated while we were copying it
    rh850_data_t rh850_data;
    do {
        update_count = shm_sous_ptr->rh850_update_count;
        memcpy(&rh850_data, &shm_sous_ptr->rh850_data, sizeof(rh850_data_t));
    } while (update_count != shm_sous_ptr->rh850_update_count);

    // Release shared memory
    wsu_share_kill(shm_sous_ptr, sizeof(shm_sous_t));

    // Print RH850 data
    if (flag_webgui_format) {
        printf("%u,%u,%s,%.1f,",
            update_count, rh850_data.cnt_opens, rh850_data.fw_version, rh850_data.dcin_voltage);
        printf("%s,%s,%s,",
            ant_status_enum_to_str(rh850_data.gps_ant2_status),
            ant_status_enum_to_str(rh850_data.cv2x_ant1_status),
            ant_status_enum_to_str(rh850_data.cv2x_ant2_status));
        for (i=0; i<NUM_RAW_RH850_DATA; i++) {
            printf("%04x,", rh850_data.raw_data[i]);
        }
        printf("\n");
    } else {
        printf("RH850: %u updates\n", update_count);
        printf("    Open Count: %u\n", rh850_data.cnt_opens);
        printf("    FW Version: %s\n", rh850_data.fw_version);
        printf("    Watchdog Timeout Count: %d\n", rh850_data.wd_timeout_count);
        printf("    Input Voltage: %.1f v\n", rh850_data.dcin_voltage);
        printf("    GNSS Antenna   : %s\n", ant_status_enum_to_str(rh850_data.gps_ant2_status));
        printf("    CV2X Antenna 1 : %s\n", strcmp(ant_status_enum_to_str(rh850_data.cv2x_ant1_status), "Short")? 
                                                   ant_status_enum_to_str(rh850_data.cv2x_ant1_status): "Connected");
        printf("    CV2X Antenna 2 : %s\n", strcmp(ant_status_enum_to_str(rh850_data.cv2x_ant2_status), "Short")?
                                                   ant_status_enum_to_str(rh850_data.cv2x_ant2_status): "Connected");
    }

    return 0;
}
