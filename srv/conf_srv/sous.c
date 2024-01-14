/*---------------------------------------------------------------------------*/
/*                                                                           */
/*    Filename: shared_opaque_utility_server.c                               */
/*     Purpose: Share output from opaque third-party utilities               */
/*  Department: DENSO North America Research & Development                   */
/*                                                                           */
/* Copyright (C) 2022 DENSO International America, Inc.                      */
/*                                                                           */
/*     Project: V2X Special Projects                                         */
/* Description: Run once read many service for opaque third-party utilities  */
/*                                                                           */
/*---------------------------------------------------------------------------*/

/* NOTES:
 *   Extending SOUS to handle a new opaque data-gathering utility:
 *     * Create a sous_xyz.h to define the memory structure and any needed state enums
 *     * In shm_sous.h, include the .h, add xyz_data to shm_sous and add an xyz_update_counter
 *     * Create a sous_xyz.c, and create a new ReaderThread__xyz() reader thread
 *     * In sous.c, Pthread_create() the new xyz reader thread
 */


#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>         // for sleep()

#include "wsu_sharedmem.h"
#include "dn_types.h"
#include "shm_sous.h"
#include "i2v_util.h"

/* mandatory logging defines */
#define MY_ERR_LEVEL LEVEL_PRIV
#define MY_NAME "SOUS"

// Globals shared by all threads
int keep_running = 1;
shm_sous_t * shm_sous_ptr;
int flag_debug_output = 0;
int ant_broadcast_period = 50;
int dcin_broadcast_period = 10;

// The reader threads, defined in their own .c's, that are being linked in
extern void * ReaderThread__rh850(void * arg);

int main(int argc, char ** argv)
{
    pthread_t reader_threadid[10];
    signed char c;

#if 0 /* Open later */
    i2vUtilEnableSyslog(MY_ERR_LEVEL, MY_NAME);
#endif
    i2vUtilEnableDebug(MY_NAME);

    while (( c = getopt(argc, argv, "hdD:A:")) != -1) {
        switch (c) {
            case 'h':
                printf("Usage: sous [ -h | -d | -D <DCIN broadcast period> | A <Antenna broadcast period> ]\n");
                printf("\t-h: Show this help message.\n");
                printf("\t-d: Print debug statements.\n");
                printf("\t-D <DCIN broadcast period>: period is in units of 100ms. e.g.: -D 10 is a period of 1s.\n"); 
                printf("\t-A <Antenna broadcast period>: period is in units of 100ms. e.g.: -A 10 is a period of 1s.\n"); 
                return 0;
            case 'd':
                flag_debug_output = 1;
                break;
            case 'D':
                if (atoi (optarg) != 0) {
                    dcin_broadcast_period = atoi(optarg);
                }
                break;
            case 'A':
                if (atoi (optarg) != 0) {
                    ant_broadcast_period = atoi(optarg);
                }
                break;
        }
    }

    // Create our shared memory //
    if ((shm_sous_ptr = wsu_share_init(sizeof(shm_sous_t), SHM_SOUS_PATH)) == NULL) {
        printf("ERROR : Could not open sous's shared memory\n");
        exit(1);
    }

    // Start our reader threads
    if (pthread_create(&reader_threadid[1], NULL, &ReaderThread__rh850, NULL) != 0)
    {
        fprintf(stderr, "RH850 Reader pthread failed to create!\n");
        exit(1);
    }

    // Main Loop
    while (keep_running) {
        // Main Loop Sleep
        sleep(1);
        // Main Loop debug trickle
        if (flag_debug_output) {
            printf("sous: RH850 %u\n",
                shm_sous_ptr->rh850_update_count);
        }
    }
    i2vUtilDisableSyslog();
    i2vUtilDisableDebug();
    // Release shared memory
    wsu_share_kill(shm_sous_ptr, sizeof(shm_sous_t));
    wsu_share_delete(SHM_SOUS_PATH);

    return 0;
}
