/*---------------------------------------------------------------------------*/
/*                                                                           */
/*    Filename: main.c                                                       */
/*     Purpose: Entry point main for Time and Position Service (tps) app     */
/*  Department: DENSO North America Research & Development                   */
/*                                                                           */
/* Copyright (C) 2020 DENSO International America, Inc.                      */
/*                                                                           */
/*     Project: V2X Special Projects                                         */
/* Description: TPS app execution start point                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>  // for O_RDONLY, O_WRONLY
#include <unistd.h> // for unlink(), read(), write() close()
#include "dn_types.h"
#include "tps.h"
#include "i2v_util.h"
#include "nscfg.h"
#if defined(MY_UNIT_TEST)
#include "stubs.h"
#define MAIN    tps_main
#else
#define MAIN    main
#endif
/* mandatory logging defines */
#if defined(MY_UNIT_TEST)
#define MY_ERR_LEVEL   LEVEL_DBG   /* to serial & syslog  */
#else
#define MY_ERR_LEVEL   LEVEL_PRIV    /* to syslog only */
#endif
#define MY_NAME        "tps_main"

uint8_t nav_pvt_sha256_enable  = FALSE;
uint8_t hnr_pvt_sha256_enable  = FALSE;
bool_t  log_ubx_hnr_ins        = FALSE;
bool_t  log_ubx_hnr_pvt        = FALSE;
bool_t  log_ubx_nav_pvt        = FALSE;
bool_t  cfg_record_tps_as_raw  = FALSE;
bool_t  cfg_adjust_system_time = TRUE; // Default is on(TRUE), help out ppl that didnt update their tps.conf
bool_t  cfg_persistency_enable = TRUE;
bool_t  cfg_startup_leapsec_share = TRUE;
bool_t  cfg_assist_now_backup_enable = FALSE;
uint16_t cfg_time_accuracy_threshold = 100;
// Leap second values that have been read from persistency file
bool_t              flag_have_read_stored_leap_secs    = FALSE; // Flag that values in structure are valid, they were read from disk, or set & saved to persistency
savedTpsVariables_t savedTpsVariables;                          // This holds the persistent variables.
bool_t              flag_request_to_save_tps_variables = FALSE; // Flag that values have changed, please save them
uint8_t             last_good_leapsec                  = 0;     // Caches the last good leapsecond value

//protodefs
int32_t save_persistent_tps_values(void);//called by tps.c

/**
 * read the saved persistent values file.
 */
static void read_persistent_tps_values(void)
{
    int fd;
    ssize_t size;

    flag_have_read_stored_leap_secs = FALSE;

    fd = open(TPS_PERSISTENT_VALUES_FILE, O_RDONLY);
    if(fd < 0 ) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Unable to open file %s: %s\n", TPS_PERSISTENT_VALUES_FILE, strerror(errno));
        unlink(TPS_PERSISTENT_VALUES_FILE);     // Just in case
        return;
    }

    size = read(fd, &savedTpsVariables, sizeof(savedTpsVariables_t));
    close(fd);
    if(size != sizeof(savedTpsVariables_t)) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Error reading %s: wanted sized %lu but got size %lu\n", TPS_PERSISTENT_VALUES_FILE, sizeof(savedTpsVariables_t), size);
        unlink(TPS_PERSISTENT_VALUES_FILE);
        return;
    }

    if (savedTpsVariables.version != TPS_PERSISTENT_VALUES_FILE_VERSION) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Error reading %s: wanted version %u but got version %u\n", TPS_PERSISTENT_VALUES_FILE, TPS_PERSISTENT_VALUES_FILE_VERSION, savedTpsVariables.version);
        unlink(TPS_PERSISTENT_VALUES_FILE);
        return;
    }
    
    flag_have_read_stored_leap_secs = TRUE;
#if defined(EXTRA_DEBUG)
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Successfully read persistent values file\n");
#endif
}

/**
 * Save the saved persistent values file.
 */
int32_t save_persistent_tps_values(void)
{
    int fd;
    ssize_t size;

    fd = open(TPS_PERSISTENT_VALUES_FILE, O_WRONLY|O_CREAT|O_TRUNC, 0666);
    if (fd < 0) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Unable to create file %s: %s\n", TPS_PERSISTENT_VALUES_FILE, strerror(errno));
        unlink(TPS_PERSISTENT_VALUES_FILE);     // Just in case
        return -1;
    }

    // Set our version number
    savedTpsVariables.version = TPS_PERSISTENT_VALUES_FILE_VERSION;

    // Save
    size = write(fd, &savedTpsVariables, sizeof(savedTpsVariables_t));
    close(fd);
    if(size != sizeof(savedTpsVariables_t)) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Error writing %s: wanted saved bytes %lu but saved size %lu\n", TPS_PERSISTENT_VALUES_FILE, sizeof(savedTpsVariables_t), size);
        unlink(TPS_PERSISTENT_VALUES_FILE);
        return -1;
    }

    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Successfully saved persistent variables to %s\n", TPS_PERSISTENT_VALUES_FILE);
    return 0;
}
int MAIN(int argc, char_t *argv[])
{
    int32_t c = 0;
    bool_t debugModeOn = WFALSE;

    /* Init statics across soft reset */
    nav_pvt_sha256_enable  = FALSE;
    hnr_pvt_sha256_enable  = FALSE;
    log_ubx_hnr_ins        = FALSE;
    log_ubx_hnr_pvt        = FALSE;
    log_ubx_nav_pvt        = FALSE;
    cfg_record_tps_as_raw  = FALSE;
    cfg_adjust_system_time = TRUE; 
    cfg_persistency_enable = TRUE;
    cfg_startup_leapsec_share = TRUE;
    cfg_assist_now_backup_enable = FALSE;
    cfg_time_accuracy_threshold = 100;
    flag_have_read_stored_leap_secs    = FALSE;
    memset(&savedTpsVariables,0x0,sizeof(savedTpsVariables));
    flag_request_to_save_tps_variables = FALSE;
    last_good_leapsec = 0;

    #if defined(MY_UNIT_TEST)
    optind = 0; /* If calling more than once you must reset to zero for getopt.*/
    #endif

    while ((c=getopt(argc,argv, "A:DLPSE")) != -1) {
        switch (c) {
            case 'A':
                cfg_time_accuracy_threshold = (uint16_t)strtoul(optarg, NULL, 10);
                printf("%s|Set time_accuracy_threshold=%u \n",MY_NAME,cfg_time_accuracy_threshold);
                break;
            case 'D':
                debugModeOn = WTRUE;
                i2vUtilEnableDebug(MY_NAME);
                break;
            case 'E':
                cfg_assist_now_backup_enable = WTRUE;
                break;
            case 'L':
                cfg_startup_leapsec_share = WTRUE;
                break;
            case 'P':
                cfg_persistency_enable= WTRUE;
                break;
            case 'S':
                nav_pvt_sha256_enable = WTRUE;
                break;
            default:
                printf("%s|Command line option %c not recognized.\n",MY_NAME, c);
                break;
        }
    }

    //Note: lat/lon/alt: When unit is moved do not restore otherwise stuck in prior location for awhile.
    // Read persistent values if enabled, otherwise make sure save file doesnt exist
    if (cfg_persistency_enable) {
        read_persistent_tps_values();
    } else {
        unlink(TPS_PERSISTENT_VALUES_FILE);
    }

    /* Start the TPS Main Process. */
//TODO: get rid of exit()'s
    tpsMainProcess(debugModeOn);

    i2vUtilDisableSyslog();
    i2vUtilDisableDebug();
    return 0;
}
