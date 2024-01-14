/*---------------------------------------------------------------------------*/
/*                                                                           */
/*    Filename: quickrh850                                                   */
/*     Purpose: Pull the RH850 data from SOUS for the webgui                 */
/*  Department: DENSO North America Research & Development                   */
/*                                                                           */
/* Copyright (C) 2022 DENSO International America, Inc.                      */
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


void usage()
{
        printf("USAGE: quickrh850 [-h] status|fwver|all\n");
        printf("   status - prints input voltage and antennas status\n");
        printf("   fwver  - prints firmware version of rh850\n");
        printf("   -h     - Prints this usage\n");
}

int main(int argc, char **argv)
{
    shm_sous_t * shm_sous_ptr;
    uint32_t update_count;

    if (argc < 2 || strcmp(argv[1],"-h")==0) {
        usage();
        exit(0);
    }
    if (strcmp(argv[1],"status") && strcmp(argv[1],"fwver")) {
        printf("ERROR: Illegal parameter '%s'\n", argv[1]);
        usage();
        exit(1);
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

    // Print Basic RH850 data
    if (strcmp(argv[1],"status") == 0) {
        printf("%.1f,", rh850_data.dcin_voltage);
        printf("%s,", ant_status_enum_to_str(rh850_data.gps_ant2_status));
        printf("%s,", strcmp(ant_status_enum_to_str(rh850_data.cv2x_ant1_status), "Short")?
                             ant_status_enum_to_str(rh850_data.cv2x_ant1_status): "Connected");
        printf("%s,", strcmp(ant_status_enum_to_str(rh850_data.cv2x_ant2_status), "Short")?
                             ant_status_enum_to_str(rh850_data.cv2x_ant2_status): "Connected");
        printf("%s,",   "888");
        printf("%s,",   "887");
        printf("%s,",   "886");
        printf("%s,\n", "885");
    // Print RH850 firmware version string
    //   NOTE: Printed without trailing comma, unlike all the other "quickXX"
    //         webgui utils, as only customer of fwver wants no comma. No point
    //         in adding it to remove it in the bash. Though, if it's needed
    //         by another customer in the future, revisit adding the comma.
    } else {
        printf("%s\n", rh850_data.fw_version);
    }

    // Release shared memory
    wsu_share_kill(shm_sous_ptr, sizeof(shm_sous_t));

    return 0;
}
