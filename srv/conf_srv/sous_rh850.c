/*---------------------------------------------------------------------------*/
/*                                                                           */
/*    Filename: sours_rh850.c                                                */
/*     Purpose: Share output from opaque third-party utilities               */
/*  Department: DENSO North America Research & Development                   */
/*                                                                           */
/* Copyright (C) 2022 DENSO International America, Inc.                      */
/*                                                                           */
/*     Project: V2X Special Projects                                         */
/* Description: Thread to parse and share the RH850 utility                  */
/*                                                                           */
/*---------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------*/
/* Include Files                                                              */
/*----------------------------------------------------------------------------*/
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>     // for usleep(), sleep()
#include <sys/time.h>   // for timeval, gettimeofday()

#include "wsu_sharedmem.h"
#include "libtimefgets.h"
#include "dn_types.h"
#include "sous_rh850.h"
#include "shm_sous.h"
#include "sous.h"
#include "i2v_util.h"

/* mandatory logging defines */
#define MY_ERR_LEVEL LEVEL_PRIV   /* from i2v_util.h */
#define MY_NAME "rh850"

#define RH850_SERIAL_DEVICE_PATH "/dev/ttyLP3"
#define RH850_SERIAL_CONFIG_CMD  "/bin/stty -F /dev/ttyLP3 115200 sane"

#define RH850_TIMEOUT_MSEC 2000
#define WD_TIMEOUT_LOGFILE "/rwflash/watchdog_timeout.log"
#define RH850_WD_TIMEOUT_STR "Count of watchdog timeout resets since last power cycle: "
#define TOTAL_WD_TIMEOUT_STR "Count of total watchdog timeout resets: "

/* Definitions from Joe's RH850_chk_status.sh script */
// INDEX values -- which antenna goes with which RH850 "AP0_#" volt data
#define GPSANT1_INDEX  0     /* AP0_0 */
#define GPSANT2_INDEX  1     /* AP0_1 */
#define CV2XANT1_INDEX 2     /* AP0_2 */
#define CV2XANT2_INDEX 3     /* AP0_3 */
#define DCIN_INDEX     12    /* AP0_12 */
// Highest AP0_# index that we get from RH850
#define MAX_DATA_INDEX 12
//Number of antenna values that we get from RH850
#define NUM_ANTS 4
// Min and max values for the GPS Antennas
#define GPS_DIAG2_DIAG1_DELTA_MIN   0x0085  // 3mA antenna load (110mV delta)
#define GPS_DIAG2_DIAG1_DELTA_MAX   0x089E  // 50mA antenna load (1.83V delta)
#define GPS_DIAG2_MIN   0x0ACD  // TP3710 = 4.9V
#define GPS_DIAG2_MAX   0x0D7C  // TP3710 = 5.1V
// Min and max values for all DSRC, CV2X, and RFSW antennas
#define RADIO_ANT_MIN   0x0100
#define RADIO_ANT_MAX   0x0200
// Min and max voltages for DC input
#define DC_INPUT_MIN   0x0637 // 10.8V (COUNT / 4095 * 27.8)
#define DC_INPUT_MAX   0x0798 // 13.2V (COUNT / 4095 * 27.8)

// The AP0_# inputs we are interested in, and their text-output labels
uint8_t data_indexes[NUM_RAW_RH850_DATA] = {
        GPSANT1_INDEX,
        GPSANT2_INDEX,
        CV2XANT1_INDEX,
        CV2XANT2_INDEX,
        DCIN_INDEX};


// Translate the raw RH850 value to an antenna status enum
uint16_t raw_data_to_ant_status(uint32_t raw_value)
{
    uint16_t ant_status;
    if (raw_value < RADIO_ANT_MIN) {
        ant_status = ANTCON_SHORT;
    } else if (raw_value > RADIO_ANT_MAX) {
        ant_status = ANTCON_NOT_CONNECTED;
    } else {
        ant_status = ANTCON_CONNECTED;
    }
    return ant_status;
}

// Compute the checksum and compare with the reported checksum value
int checksum_compare(char buffer[], unsigned int checksum_value)
{
    unsigned int sum1=0;
    unsigned int sum2=0;
    int i;

    for (i=0; i<strlen(buffer)-6; i++) {
        sum1 = (sum1 + buffer[i]) % 255;
        sum2 = (sum2 + sum1) % 255;
    }
    sum2 = (sum2 << 8) | sum1;
    
    if (flag_debug_output) {
        printf("Checksum value: %x\n", checksum_value);
        printf("Calculated checksum value: %x\n", sum2);
    }
    return checksum_value == sum2;
}

// Calculate the number of watchdog timeouts from the previous logs and the current value reported by the RH850
int calculate_watchdog_timeout_count(int32_t rh850_count )
{
    int32_t total_count, prior_total_count, prior_rh850_count;
    FILE *fp; 
    char cmd[256], line[256];
    char *token, *priortoken;


    // Get Prior RH850 Watchdog Timeout Count from log
    snprintf(cmd, sizeof(cmd), "cat %s | grep \"%s\"", WD_TIMEOUT_LOGFILE, RH850_WD_TIMEOUT_STR);
    if (NULL != (fp = popen(cmd, "r"))) {
        memset(line, 0, sizeof(line));
        fgets(line, sizeof(line), fp);
        pclose(fp);
        // parse line
        if (strcmp(line, "")) {
            line[strcspn(line, "\n")] = 0; //remove newline from end of line
            token = strtok(line, " ");
            priortoken = token;
            while ((token = strtok(NULL, " ")) != NULL) {
                priortoken = token;
            }
            prior_rh850_count = (int32_t)strtol(priortoken, NULL, 10);
        }
        else {
            prior_rh850_count = 0;
        }
    }
    else {
        prior_rh850_count = 0;
    }

    // Get Prior Total Watchdog Timeout Count from log
    snprintf(cmd, sizeof(cmd), "cat %s | grep -a \"%s\"", WD_TIMEOUT_LOGFILE, TOTAL_WD_TIMEOUT_STR);
    if (NULL != (fp = popen(cmd, "r"))) {
        memset(line, 0, sizeof(line));
        fgets(line, sizeof(line), fp);
        pclose(fp);
        // parse line
        if (strcmp(line, "")) {
            line[strcspn(line, "\n")] = 0; //remove newline from end of line
            token = strtok(line, " ");
            priortoken = token;
            while ((token = strtok(NULL, " ")) != NULL) {
                priortoken = token;
            }
            prior_total_count = (int32_t)strtol(priortoken, NULL, 10);
        }
        else {
            prior_total_count = 0;
        }
    }
    else {
        prior_total_count = 0;
    }

    // Calculate new total count 
    total_count = (rh850_count > prior_rh850_count)? prior_total_count + rh850_count - prior_rh850_count : 
                                                     prior_total_count;
    // Overwrite log file with new counts 
    if (NULL != (fp = fopen(WD_TIMEOUT_LOGFILE, "w"))) {
        fprintf(fp, "%s %d\n", RH850_WD_TIMEOUT_STR, rh850_count);
        fprintf(fp, "%s %d\n", TOTAL_WD_TIMEOUT_STR, total_count);
        fclose(fp);   
    }

    if (flag_debug_output) {
        printf("prior RH850 watchdog timeout count: %d\n", prior_rh850_count);
        printf("prior Total watchdog timeout count: %d\n", prior_total_count);
        printf("current RH850 watchdog timeout count: %d\n", rh850_count);
        printf("current Total watchdog timeout count: %d\n", total_count);
    }

    return total_count;
    
}

void * ReaderThread__rh850(void * arg)
{
    rh850_data_t shm_data;
    FILE * fd = NULL;
    char buffer[1000];
    int rc, bytes_read;
    uint32_t done = 0;
    struct timeval tv_start, tv_now;
    bool_t flag__need_firmware_version = 1;
    bool_t flag__need_wd_timeout_count = 1;
    int bits;
    unsigned int ant_hex_values[NUM_ANTS];
    unsigned int dcin_hex_value;
    unsigned int checksum_value;
    int checksum_ok; 
    int checksum_failures = 0;
    uint32_t raw_data[MAX_DATA_INDEX+1] = {0};
    float32_t ant_delta;
    uint8_t di;
    uint32_t nothing_counter = 0;
    int i;
    int32_t rh850_wd_timeout;

    /* Configure ubloxM8 serial port for receiving UBX binary data. */
    rc = system(RH850_SERIAL_CONFIG_CMD);
    if (rc != 0) {
        printf("ERROR: Failed to configure RH850 serial port (%s): %s\n",
               RH850_SERIAL_DEVICE_PATH, RH850_SERIAL_CONFIG_CMD);
    } else {
        if (flag_debug_output) {
            printf("Configured RH850 serial port (%s): %s\n",
                   RH850_SERIAL_DEVICE_PATH, RH850_SERIAL_CONFIG_CMD);
        }
    }
    
    /* Create and execute the 'A' command
       Activates the RH850 broadcasting Antenna Diagnostics 
    */
    sprintf(buffer, "echo -e 'ZA,%d' > %s", ant_broadcast_period, RH850_SERIAL_DEVICE_PATH);
    if (flag_debug_output) {
        printf("system(\"%s\")\n", buffer);
    }
    system(buffer);
    
    /* Create and execute the 'D' command 
       Activates the RH850 broadcasting DC input
    */
    sprintf(buffer, "echo -e 'ZD,%d' > %s", dcin_broadcast_period,  RH850_SERIAL_DEVICE_PATH);
    if (flag_debug_output) {
        printf("system(\"%s\")\n", buffer);
    }
    system(buffer);
        

    while (keep_running) {

        /* It's a new cycle! */
        done = 0;

        /* Open the RH850 serial port, if needed; Reuse when open from previous loops */
        if (!fd) {
            ++shm_data.cnt_opens;
            fd = fopen(RH850_SERIAL_DEVICE_PATH,"r");
            if (fd == NULL) {
                done = 4;
            }
        }

        // Create and execute the 'V' command
        if (!done) {
            if (flag__need_firmware_version) {
                sprintf(buffer, "echo -e 'ZV' > %s", RH850_SERIAL_DEVICE_PATH);
                if (flag_debug_output) {
                    printf("system(\"%s\")\n", buffer);
                }
                system(buffer);
            }
        }
        
        // Create and execute the 'W' command
        if (!done) {
            if (flag__need_wd_timeout_count) {
                sprintf(buffer, "echo -e 'ZW' > %s", RH850_SERIAL_DEVICE_PATH);
                if (flag_debug_output) {
                    printf("system(\"%s\")\n", buffer);
                }
                system(buffer);
            }
        }

        /* Start receiving and storing results */
        gettimeofday(&tv_start, NULL);
        while (!done) {
            bytes_read = timefgets(buffer, sizeof(buffer), fd, RH850_TIMEOUT_MSEC);
            if (bytes_read <= 0) {
                if (flag_debug_output) {
                    ++nothing_counter;
                    printf("   ... 0 bytes read # %d\n", nothing_counter);
                }
            } else {
                if (flag_debug_output) {
                    printf("Got %d bytes: '%s'\n", bytes_read, buffer);
                    nothing_counter = 0;
                }
                if (!strncmp(buffer, "AP0_12=", 7)) {
                    if (flag_debug_output) {
                        printf("Got DCIN value\n");
                    }
                    bits = sscanf(buffer, "AP0_12=%x,%x", &dcin_hex_value, &checksum_value);
                    if (bits != 2) {
                        printf("Parse Error: Parsed %d values (want 2)\n", bits);
                    }
                    else {
                        if (flag_debug_output) {
                            printf("DCIN value=0x%04x\n", dcin_hex_value);
                        }
                        // CHECKSUM
                        checksum_ok = checksum_compare(buffer, checksum_value);
                        if (flag_debug_output) {
                            printf("Checksum ok: %d\n", checksum_ok);
                        }
                        if (!checksum_ok) {
                            ++checksum_failures;
                            printf("Error: DCIN checksum failure\n");
                        }
                        // Save the raw_data from this line for later
                        raw_data[12] = dcin_hex_value;
                    }
                }
                else if (!strncmp(buffer, "AP0_0=", 6)) {
                    if (!flag__need_firmware_version) {  // Wait till FW version is known

                        if (flag_debug_output) {
                            printf("Got antenna values\n");
                        }
                        // String format depends on fw version. 
                        if (strstr(shm_data.fw_version, "2.0.2")) {
                            bits = sscanf(buffer, "AP0_0=%x,AP0_1=%x,AP0_2=%x,AP0_3=%x,AP0_4=%*x,AP0_5=%*x,%x",
                                    &ant_hex_values[0], &ant_hex_values[1], &ant_hex_values[2], 
                                    &ant_hex_values[3], &checksum_value);
                        } else {
                            bits = sscanf(buffer, "AP0_0=%x,AP0_1=%x,AP0_2=%x,AP0_3=%x,%x",
                                    &ant_hex_values[0], &ant_hex_values[1], &ant_hex_values[2], 
                                    &ant_hex_values[3], &checksum_value);
                        }
                        if (bits != NUM_ANTS+1) {
                            printf("Parse Error: Parsed %d values (want %d or 7)\n", bits, NUM_ANTS+1);
                        } else {
                            if (flag_debug_output) {
                                for (i=0; i<NUM_ANTS; i++) {
                                    printf("AP0_%d value: 0x%04x\n", i, ant_hex_values[i]);
                                }
                            }
                            //CHECKSUM
                            checksum_ok = checksum_compare(buffer, checksum_value);
                            if (flag_debug_output) {
                                printf("Checksum ok: %d\n", checksum_ok);
                            }
                            if (!checksum_ok) {
                                ++checksum_failures;
                                printf("Error: Antenna checksum failure\n");
                            }
                            // Save raw_data for later
                            for (i=0; i<NUM_ANTS; i++) {
                                raw_data[i] = ant_hex_values[i];
                            }
                        }
                    }
                }
                else if (!strncmp(buffer, "Received Cmd :", 14)) {
                    if (flag_debug_output) {
                        printf("RH850 confirms our command\n");
                    }
                }
               
                // If requesting, the next non--command-confirmation line is the firmware version
                else if (flag__need_firmware_version || flag__need_wd_timeout_count) {
                    /* Must have the "Firmware Vresion : " preface, and strip it out */
                    if (! strncmp(buffer, "FW Version : ", 13)) {
                        strncpy(shm_data.fw_version, buffer + 13, sizeof(shm_data.fw_version));
                        flag__need_firmware_version = 0;
                        done = 1;
                        if (flag_debug_output) {
                            printf("Received version string: '%s'\n", shm_data.fw_version);
                        }
                    }
                    /* Must have the "i.MX8 Watchdog Reset Count " preface, and strip it out */
                    if (! strncmp(buffer, "i.MX8 Watchdog Reset Count = ", 29)) {
                        rh850_wd_timeout = (int32_t)strtol(buffer+29, NULL, 10);
                        shm_data.wd_timeout_count = calculate_watchdog_timeout_count(rh850_wd_timeout);

                        flag__need_wd_timeout_count = 0;
                        done = 1;
                        if(0 == i2vUtilEnableSyslog(MY_ERR_LEVEL, MY_NAME)) {
                            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"%s %d\n", RH850_WD_TIMEOUT_STR, rh850_wd_timeout);
                            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"%s %d\n", TOTAL_WD_TIMEOUT_STR, shm_data.wd_timeout_count);
                        }
                        i2vUtilDisableSyslog();
                        if (flag_debug_output) {
                            printf("%s %d\n", RH850_WD_TIMEOUT_STR, rh850_wd_timeout);
                            printf("%s %d\n", TOTAL_WD_TIMEOUT_STR, shm_data.wd_timeout_count);
                        }
                    } else {
                        if (flag_debug_output) {
                            printf("Wanted FWVer or Watchdog Timeout, but ignoring unexpected line: %s\n", buffer);
                        }
                    }
                // What on earth did we get?
                } else {
                    if (flag_debug_output) {
                        printf("Skipping unexpected text from RH850: %s\n", buffer);
                    }
                }
            }

            /* If more than 2 seconds of waiting on RH850, we're done */
            if (!done) {
                gettimeofday(&tv_now, NULL);
                if (tv_now.tv_sec - tv_start.tv_sec >= 2) {
                    if (tv_now.tv_sec - tv_start.tv_sec > 2) {
                        done = 2;
                        if (flag_debug_output) {
                            printf("Done waiting #1, done, secdelta = %lu\n",
                                tv_now.tv_sec - tv_start.tv_sec);
                        }
                    } else if (tv_now.tv_usec >= tv_start.tv_usec) {
                        done = 3;
                        if (flag_debug_output) {
                            printf("Done waiting #2, done, usecdleta= %lu\n", 
                                tv_now.tv_usec - tv_start.tv_usec);
                        }
                    }
                }
            }
        }

        /* Close up if we got a timeout, otherwise keep reusing the descriptor */
        if (done > 1 && fd != NULL) {
            fclose(fd);
            fd = NULL;
            if (flag_debug_output) {
                printf("Closed RH850 file descriptor as done = %d (want 1)\n", done);
            }
        }

        /* DEBUG Timing */
        if (flag_debug_output) {
            gettimeofday(&tv_now, NULL);
            if (tv_now.tv_usec >= tv_start.tv_usec) {
                printf("Done in %lu.%06lu sec\n",
                    tv_now.tv_sec - tv_start.tv_sec,
                    tv_now.tv_usec - tv_start.tv_usec);
            } else {
                printf("Done in %lu.%06lu sec\n",
                    tv_now.tv_sec - tv_start.tv_sec - 1,
                    1000000 + tv_now.tv_usec - tv_start.tv_usec);
            }
        }

        /* Process the saved raw_data and store into shared memory */
        /*
        ** Build output results -- AP0_[0,1,2,3,12]
        */
        for (i=0; i<NUM_RAW_RH850_DATA; i++) {
            di = data_indexes[i];
            // Interpret raw into a meaningful value
            switch (di) {
                // DCIN 
                case DCIN_INDEX:
                    shm_data.dcin_voltage = raw_data[di] * 27.8 / 4095;
                    break;
                // Antennas
                case CV2XANT1_INDEX:
                    shm_data.cv2x_ant1_status = raw_data_to_ant_status(raw_data[di]);
                    break;
                case CV2XANT2_INDEX:
                    shm_data.cv2x_ant2_status = raw_data_to_ant_status(raw_data[di]);
                    break;
                // There are two measurements we use to get one GNSS result
                // The difference between them, and the second one vs min&max
                case GPSANT1_INDEX:
                    // We only included GPSANT1 so it has a position in raw_data
                    //     to hold its raw value to give to the testing team
                    break;
                case GPSANT2_INDEX:
                    if (raw_data[GPSANT2_INDEX] > raw_data[GPSANT1_INDEX]) {
		                    ant_delta = raw_data[GPSANT2_INDEX] - raw_data[GPSANT1_INDEX];
		                } else {
			                  ant_delta = raw_data[GPSANT1_INDEX] - raw_data[GPSANT2_INDEX];
		                }
                    if (raw_data[GPSANT2_INDEX] < GPS_DIAG2_MIN ||
                        raw_data[GPSANT2_INDEX] > GPS_DIAG2_MAX) {
                        shm_data.gps_ant2_status = ANTCON_SUPPLY_FAIL;
                    } else if (ant_delta < GPS_DIAG2_DIAG1_DELTA_MIN) {
                        shm_data.gps_ant2_status = ANTCON_NOT_CONNECTED;
                    } else if (ant_delta > GPS_DIAG2_DIAG1_DELTA_MAX) {
                        shm_data.gps_ant2_status = ANTCON_SHORT;
                    } else {
                        shm_data.gps_ant2_status = ANTCON_CONNECTED;
                    }
                    break;
                default:
                    break;
            }
        }

        // Put a copy of the raw_data into our shm for the testing team
        // Note the local raw_data[] has MAX_DATA_INDEX+1 values, and
        //    the shm's raw_data has NUM_RAW_RH850_DATA values
        for (i=0; i<NUM_RAW_RH850_DATA; i++) {
            shm_data.raw_data[i] = raw_data[data_indexes[i]];
        }

        // Update shm
        memcpy(&shm_sous_ptr->rh850_data, &shm_data, sizeof(rh850_data_t));
        ++shm_sous_ptr->rh850_update_count;
    }
    return NULL;
}
