/******************************************************************************
 *                                                                            *
 *     File Name:  quickspat.c                                                *
 *     Author:                                                                *
 *         DENSO International America, Inc.                                  *
 *         North America Research Laboratory, California Office               *
 *         3252 Business Park Drive                                           *
 *         Vista, CA 92081                                                    *
 *                                                                            *
 ******************************************************************************
 * (C) Copyright 2021 DENSO International America, Inc.  All rights reserved. * 
 ******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "dn_types.h"
#include "i2v_shm_master.h"
#include "i2v_scs_data.h"
#include "i2v_types.h"
#include "i2v_util.h"
#include "libtsmapreader.h"     // for TSMAP_VEHICLE
#include "i2v_types.h"          // for SPAT_MAX_APPROACHES



/* Global Declarations */
i2vShmMasterT   *shmPtr;


// Compare function and Struct definition to sort j2735 data by ascending siggrp id
struct twoints { int int1; int int2; };
// Sort function -- conforms to qsort requirement:  int (*compar)(const void *, const void *));
int compare_two_struct_twoints(const void * a, const void * b)
{
    return ((struct twoints *)a)->int1 - ((struct twoints *)b)->int1;
}


/* Function displaying SPaT data */
void printSpatData(void)
{
    scsSpatShmDataT scsSpatData;
    struct twoints sorted_siggrpids[SPAT_MAX_APPROACHES];
    struct tm tm;
    int i,ii;

    WSU_SHM_LOCKR(&shmPtr->scsSpatData.h.ch_lock);
    scsSpatData = shmPtr->scsSpatData;
    WSU_SHM_UNLOCKR(&shmPtr->scsSpatData.h.ch_lock);

    if (scsSpatData.h.ch_data_valid == WTRUE) {
        printf("True,\n");

        // Get timestamp of spat processing as HHMMSS and as sec-in-hour
        
        localtime_r(&scsSpatData.spat16_process_timestamp, &tm);
        printf("%02d:%02d:%02d,%d,\n",
            tm.tm_hour, tm.tm_min, tm.tm_sec, tm.tm_min * 60 + tm.tm_sec);

        // TSCBM Data - vehicles then overlap
        printf("%d,\n", scsSpatData.spat_info.flexSpat.numApproach);
        for (i=0; i<SPAT_MAX_APPROACHES; i++) {
            if ((i % (SPAT_MAX_APPROACHES/2)) <scsSpatData.spat_info.flexSpat.numApproach) {
                // phaseNumber = i+1   <-- DOESNT NEED TO BE PRINTED, Implied by data position
                // phaseColor  = scsSpatData.spat_info.flexSpat.spatApproach[i].curSigPhase
                // MinTTC      = scsSpatData.spat_info.flexSpat.spatApproach[i].timeNextPhase / 10.0
                // MaxTTC      = scsSpatData.spat_info.flexSpat.spatApproach[i].secondaryTimeNextPhase / 10.0
                // isFlashing  = scsSpatData.spat_info.flexSpat.spatApproach[i].curSigPhase
                printf("%s,%1.f,%1.f,%s,\n",
                    i2v_sigphase_to_traffic_light_color_string(
                        scsSpatData.spat_info.flexSpat.spatApproach[i].curSigPhase,
                        FALSE /* Dont add Flashing */),
                    (double) scsSpatData.spat_info.flexSpat.spatApproach[i].timeNextPhase / 10.0,
                    (double) scsSpatData.spat_info.flexSpat.spatApproach[i].secondaryTimeNextPhase / 10.0,
                    is_flashing_phase(scsSpatData.spat_info.flexSpat.spatApproach[i].curSigPhase) ? "Yes" : "No");
            } else {
                // Flesh out the rest of the phases, print something, so webgui has data to operate
                printf("0,0,0,X,\n");
            }
        }
        // J2735 Data
        printf("%d,\n", scsSpatData.output_copy.num_groups);
        // Sort the data indexes by their signal group id
        for (i=0; i<scsSpatData.output_copy.num_groups; i++) {
            sorted_siggrpids[i].int1 = scsSpatData.output_copy.signal_group_id[i];
            sorted_siggrpids[i].int2 = i;
        }
        qsort(&sorted_siggrpids[0], scsSpatData.output_copy.num_groups, sizeof(struct twoints), compare_two_struct_twoints);
               // void qsort(void *base, size_t nmemb, size_t size,
               //                   int (*compar)(const void *, const void *));

        // Print sorted data
        for (i=0; i<scsSpatData.output_copy.num_groups; i++) {
            ii = sorted_siggrpids[i].int2;
            printf("%d,%s,%1.0f,%1.0f,%s,%s %d,\n",
                scsSpatData.output_copy.signal_group_id[ii],
                i2v_movement_phase_state_to_string(scsSpatData.output_copy.final_event_state[ii]),
                (double) scsSpatData.output_copy.min_end_time[ii] / 10.0,
                (double) scsSpatData.output_copy.max_end_time[ii] / 10.0,
                i2v_movement_phase_state_to_traffic_light_color_string(
                    scsSpatData.output_copy.final_event_state[ii], TRUE /* Mention flashing */),
                // Combine phase source into a Human-friendly string
                scsSpatData.output_copy.tsc_phase_type[ii] == TSMAP_VEHICLE ? "Vehicle" : "Overlap",
                scsSpatData.output_copy.tsc_phase_number[ii]);
        }
        // TSCBM and J2735 Booleans (manual control, etc., 4 hex chars)
        printf("%04X,%04X,\n",
            scsSpatData.output_copy.TSCBMIntersectionStatus,
            scsSpatData.output_copy.IntersectionStatus);
        // MsgCount (revision in intersection state
        printf("%d,\n", scsSpatData.output_copy.revision_counter);
    } else {
        printf("False,\n");
    }
    printf("\n");
}




int main()
{
    if ((shmPtr = wsu_share_init(sizeof(i2vShmMasterT), I2V_SHM_PATH)) == NULL) 
    {
        printf("quickspat: Could not Allocate Shared Memory \n");
        return -1;
    }

    printSpatData();

    wsu_share_kill(shmPtr, sizeof(i2vShmMasterT));
    return 0;
}
