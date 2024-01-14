/******************************************************************************
 *                                                                            *
 *     File Name:  i2vmonitor.c                                               *
 *     Author:                                                                *
 *         DENSO International America, Inc.                                  *
 *         North America Research Laboratory, California Office               *
 *         3252 Business Park Drive                                           *
 *         Vista, CA 92081                                                    *
 *                                                                            *
 * This program reads the i2v shared memory and displays information on the   *
 * terminal in real time.  The user can change to a different display         *
 * by selecting the desired screen.                                           *  
 *                                                                            *
 ******************************************************************************
 * (C) Copyright 2021 DENSO International America, Inc.  All rights reserved. * 
 ******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <time.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <limits.h>

#include "wsu_util.h"
#include "wsu_shm.h"

#include "i2v_general.h"
#include "i2v_util.h"
#include "i2v_shm_master.h"
#include "libtsmapreader.h"

/* Global Declarations */
i2vShmMasterT   *shmPtr;
cfgItemsT           cfg;
volatile sig_atomic_t mainloop;
volatile sig_atomic_t repeatloop;

/* Gets and display shm SPaT data
 */
void display_spat_data(void)
{
    scsSpatShmDataT scsSpatData;
    wuint32         i;
    scsCfgItemsT    my_scs_cfg;
    spatCfgItemsT       my_spat_cfg;

    memset(&my_spat_cfg,0x0,sizeof(my_spat_cfg));
    memset(&scsSpatData,0x0,sizeof(scsSpatData));
    memset(&my_scs_cfg,0,sizeof(my_scs_cfg));

    WSU_SHM_LOCKR(&shmPtr->scsSpatData.h.ch_lock);
    memcpy(&scsSpatData,&shmPtr->scsSpatData,sizeof(scsSpatData));
    WSU_SHM_UNLOCKR(&shmPtr->scsSpatData.h.ch_lock);

    WSU_SHM_LOCKR(&shmPtr->scsCfgData.h.ch_lock);
    memcpy(&my_scs_cfg, &shmPtr->scsCfgData,sizeof(my_scs_cfg));
    WSU_SHM_UNLOCKR(&shmPtr->scsCfgData.h.ch_lock);

    WSU_SHM_LOCKR(&shmPtr->spatCfgData.h.ch_lock);
    memcpy(&my_spat_cfg, &shmPtr->spatCfgData,sizeof(my_spat_cfg));
    WSU_SHM_UNLOCKR(&shmPtr->spatCfgData.h.ch_lock);


    printf("----------------------------------------------------------------\n");
    printf("SPaT STATUS: Movement States = %d\n", scsSpatData.spat_info.flexSpat.numApproach);
    printf("IntID=%d Fill Cnt=%d Blob Cnt=0x%lx\n",
           my_spat_cfg.IntersectionID,
           scsSpatData.metrics.cnt_asn1fill_spat_calls,
           scsSpatData.packetCount);

    printf("----------------------------------------------------------------\n");
    printf("Phase:SGI:GM    Conf TTNP  Yel   Signal Phase\n");
    printf("------------    ---- ----  ---   ------------\n");
    printf("  Phase     Phase\n");
    printf("  #  Type   Color            MinTTC  MaxTTC  Flashing\n");
    printf("---- ----   ---------------  ------  ------  --------\n");

    for (i=0; i<SPAT_MAX_APPROACHES; i++) {
        if ((i % (SPAT_MAX_APPROACHES/2)) <scsSpatData.spat_info.flexSpat.numApproach) {
            if (scsSpatData.spat_info.flexSpat.spatApproach[i].curSigPhase != SIG_PHASE_UNKNOWN
                && scsSpatData.spat_info.flexSpat.spatApproach[i].curSigPhase != SIG_PHASE_DARK)
            {

                // phaseNumber = i+1   <-- DOESNT NEED TO BE PRINTED, Implied by data position
                // phaseColor  = scsSpatData.spat_info.flexSpat.spatApproach[i].curSigPhase
                // MinTTC      = scsSpatData.spat_info.flexSpat.spatApproach[i].timeNextPhase / 10.0
                // MaxTTC      = scsSpatData.spat_info.flexSpat.spatApproach[i].secondaryTimeNextPhase / 10.0
                // isFlashing  = scsSpatData.spat_info.flexSpat.spatApproach[i].curSigPhase
                printf("%3d  %4s  %15s  %6.1f  %6.1f  %6s\n",
                    1 + i%16,
                    i < 16 ? "veh" : "olap",
                    i2v_sigphase_to_traffic_light_color_string(
                        scsSpatData.spat_info.flexSpat.spatApproach[i].curSigPhase,
                        FALSE /* Dont add Flashing */),
                    (double) scsSpatData.spat_info.flexSpat.spatApproach[i].timeNextPhase / 10.0,
                    (double) scsSpatData.spat_info.flexSpat.spatApproach[i].secondaryTimeNextPhase / 10.0,
                    is_flashing_phase(scsSpatData.spat_info.flexSpat.spatApproach[i].curSigPhase) ? "Yes" : "No");
            }
        }
    }

    // J2735 Data
    printf("J2735 Data, numGroups = %d,\n", scsSpatData.output_copy.num_groups);
    printf("Signal                   Signal                                          Phase  Phase\n");
    printf("Group  Signal Color      Movement Phase State           MinTTC  MaxTTC     #     Type\n");
    printf("-----  ---------------   ---------------------------    ------  ------   -----  -----\n");
    for (i=0; i<scsSpatData.output_copy.num_groups; i++) {
        printf(" %3d   %-15s   %-30s %6.1f  %6.1f   %3d     %4s\n",
            scsSpatData.output_copy.signal_group_id[i],
            i2v_sigphase_to_traffic_light_color_string(scsSpatData.output_copy.signal_phase[i], TRUE /* Mention flashing */),
            i2v_movement_phase_state_to_string(scsSpatData.output_copy.final_event_state[i]),
            (double) scsSpatData.output_copy.min_end_time[i] / 10.0,
            (double) scsSpatData.output_copy.max_end_time[i] / 10.0,
            scsSpatData.output_copy.tsc_phase_number[i],
            scsSpatData.output_copy.tsc_phase_type[i] == TSMAP_VEHICLE ? "veh" : "olap");
    }

    printf("----------------------------------------------------------------\n");
}

static void reservedFunction(void)
{

    printf("\033[2J\033[1;1H");
    printf("----------------------------------------------------------------\n");
    printf("      RESERVED FUNCTION    \n");
    printf("----------------------------------------------------------------\n");
    printf("function soon to exist - nothing defined now\n");
    printf("\n\n\n");
}


/* Signal handler for v2vmonitor */
static void sigint_handler( int GCC_UNUSED_VAR sig)
{
    mainloop = WFALSE;
    repeatloop = INT_MAX;
}

/* This is main function for v2vmonitor.  It accepts keyboard input and waits in the loop 
   until killed or terminated or interrupted.  */
int main()
{
    unsigned char input,old_input = 'A';
    unsigned char dummy;
    pid_t i2vPid;
    char_t  *cmd = "pidof i2v";
    FILE    *pidofi2v;
    char_t  pids[100];

    fd_set set;
    struct timeval timeout;
    int retval;	

    FD_ZERO (&set);
    FD_SET (0, &set);

    if ( ! (system("pidof i2v_app > /dev/null") == 0) ) 
    {
        printf("I2V is not Running\n");
        return -1;
    }

    /* Get one of the PIDs of i2v instances */
    if ((pidofi2v = popen(cmd,"r")) != NULL)
        fgets(pids,sizeof (char) * 100,pidofi2v);

    /* Store one PID to find out i2v is running or not */
    sscanf(pids,"%d",&i2vPid);
    pclose(pidofi2v);

    if ((shmPtr = wsu_share_init(sizeof(i2vShmMasterT), I2V_SHM_PATH)) == NULL) 
    {
        printf("i2vmonitor: Could not Allocate Shared Memory \n");
        return -1;
    }

    /* Init the loop parameters */
    repeatloop = WFALSE;
    mainloop = WTRUE;

    // Set up sigaction to trap SIGINT & SIGTERM
    wsuUtilSetupSignalHandler(sigint_handler, SIGTERM, 0);
    wsuUtilSetupSignalHandler(sigint_handler, SIGINT, 0);

    printf("\033[2J\033[1;1H");

    printf("----------------------------------------------------------------\n");
    printf("				INFRASTRUCTURE-TO-VEHICLE (I2V)\n");
    printf("----------------------------------------------------------------\n");
    printf("				        STATUS MONITOR\n");
    printf("----------------------------------------------------------------\n");
    printf("       This application monitors status in 500ms interval\n"); 
    printf("----------------------------------------------------------------\n"); 
    printf("\n\n\n");

    while(mainloop) {

        FD_ZERO (&set);
        FD_SET (0, &set);
        /* Refreshes every 500ms second */
        timeout.tv_sec = 0;
        timeout.tv_usec = 500000;

        printf ("<Select # hit enter> 1: Display SPaT Stats  9: To Exit (or Ctrl-C)\n");

        /* Wait here for the input */
        if (repeatloop == WFALSE) {
            read (STDIN_FILENO, &input,1);
            read (STDIN_FILENO, &dummy,1);
        }
        else if (repeatloop == WTRUE){
            /* Wait here for the input. If there's none after 500ms */
            /* use the previous input */
            retval = select (1, &set, NULL, NULL, &timeout);
            if (retval == 1) {
                read (STDIN_FILENO, &input,1);
                read (STDIN_FILENO, &dummy,1);
            }
            else if (retval == 0){
                input = old_input;	
            }
            else {
            }
        }
        else    {
            input = '9';
        }

        /* kill is sent with signal '0' to check if the i2v process is running. */
        /* Signal '0' won't send the signal to the process but the error checking is still performed. */
        /* If it's in error and errno is ESRCH (No such process), that means i2v is no longer running. */
        /* Please refer kill(2) for more details. */

        if((kill (i2vPid,0)) == -1 ) {
            if (errno == ESRCH) {
                printf("I2V is not Running. Exiting i2vmonitor.\n");
                mainloop = WFALSE;
                repeatloop = INT_MAX;
                input = '9';
            }
        }

        switch (input) {
            case '1':       /* Display SPaT Data */
                display_spat_data();
                repeatloop = WTRUE;
                old_input = input;
                break;
            case '2':       /* Reserved function - replace when needed */
                reservedFunction();
                repeatloop = WTRUE;
                old_input = input;
                break;
            case '3':       /* Reserved function - replace when needed */
                reservedFunction();
                repeatloop = WTRUE;
                old_input = input;
                break;
            case '4':      /* Reserved function - replace when needed */
                reservedFunction();
                repeatloop = WTRUE;
                old_input = input;
                break;
            case '9':       /* Exit */
                mainloop = WFALSE;
                break;
            default:
                printf("Invalid option entered. Enter a valid option or previous valid option will be used if available. \n");
                break;
        }

    }
    wsu_share_kill(shmPtr, sizeof(i2vShmMasterT));
    return 0;
}
