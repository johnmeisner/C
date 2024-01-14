/*---------------------------------------------------------------------------*/
/*                                                                           */
/*    Filename: quickpos.c                                                   */
/*     Purpose: Test tps shm-published position                              */
/*  Department: DENSO North America Research & Development                   */
/*                                                                           */
/* Copyright (C) 2020 DENSO International America, Inc.                      */
/*                                                                           */
/*     Project: V2X Special Projects                                         */
/* Description: Demonstration of getting tps's gps position through SHM      */
/*                                                                           */
/*---------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------*/
/* Include Files                                                              */
/*----------------------------------------------------------------------------*/
#include <string.h>
#include <stdlib.h>

#include "wsu_sharedmem.h"
#include "dn_types.h"
#include "wsu_util.h"
#include "tps_msg.h"
#include "tps_types.h"
#include "gps.h"
#include "tps.h"
#include "shm_tps.h"


int main(int argc, char ** argv)
{
    shm_tps_t * shm_tps_ptr;
    tps_pos_publish_t tps_pub_pos;
    shm_tps_t tps_shm;
    int flag_webgui_format = 0;
    int flag_debug_output = 0;

    // Arg processing
    if (argc == 2 && (!strcmp(argv[1], "-h") || !strcmp(argv[1], "--help"))) {
        printf("Usage: quickpos [--webgui-format|--debug]\n");
        exit(0);
    }
    if (argc == 2 && !strcmp(argv[1], "--webgui-format")) {
        flag_webgui_format = 1;
    }
    if (argc == 2 && !strcmp(argv[1], "--debug")) {
        flag_debug_output = 1;
    }
        

    // Create our shared memory //
    if ((shm_tps_ptr = wsu_share_init(sizeof(shm_tps_t), SHM_TPS_PATH)) == NULL) {
        printf("ERROR : Could not open TPS's shared memory\n");
        exit(1);
    }

    // Copy pos info
    memcpy(&tps_pub_pos, &shm_tps_ptr->pub_pos, sizeof(tps_pos_publish_t));
    
    // DEBUG - copy all of shm
    if (!flag_webgui_format) {
        memcpy(&tps_shm, shm_tps_ptr, sizeof(shm_tps_t));
    }

    // Release TPS's shared memory
    wsu_share_kill(shm_tps_ptr, sizeof(shm_tps_t));

    // Print
    if (flag_webgui_format) {
        printf("%lf,%lf,%lf,\n",
            tps_pub_pos.lat[tps_pub_pos.last_updated_index],
            tps_pub_pos.lon[tps_pub_pos.last_updated_index],
            tps_pub_pos.alt[tps_pub_pos.last_updated_index]);
    } else {

        // Print pos
        printf("Cur Position: %lf Lat, %lf Lon, %lf Alt\n",
            tps_pub_pos.lat[tps_pub_pos.last_updated_index],
            tps_pub_pos.lon[tps_pub_pos.last_updated_index],
            tps_pub_pos.alt[tps_pub_pos.last_updated_index]);

        if (flag_debug_output) {

            // Print debug
            printf(" TPS : seqno = %u\n", tps_shm.seqno);
            printf("     : count_nmea = %u\n", tps_shm.debug_gps_cmd_count_nmea);
            printf("     : count_ubx = %u\n", tps_shm.debug_gps_cmd_count_ubx);
            printf("     : parsed %d+%d+%d cmds making %d dataInds (%d sent), %d parse aborts\n",
                    tps_shm.debug_gps_cmd_count_nmea,
                    tps_shm.debug_gps_cmd_count_rtcm,
                    tps_shm.debug_gps_cmd_count_ubx,
                    tps_shm.debug_cnt_comm_send_tps_data_ind,
                    tps_shm.debug_cnt_inds_sent_to_tps,
                    tps_shm.debug_buffer_aborts);
        }
    }

    return 0;
}
