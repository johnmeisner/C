/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Filename: v2x_radio_rx.c                                                   */
/*  Purpose: Test program that receives packets using the v2x_radio library   */
/*                                                                            */
/* Copyright (C) 2019 DENSO International America, Inc.  All Rights Reserved. */
/*                                                                            */
/* Revision History:                                                          */
/*                                                                            */
/* Date        Author        Comments                                         */
/* -------------------------------------------------------------------------- */
/* 2019-06-14  VROLLINGER    Initial revision.                                */
/*                                                                            */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/* Include Files                                                              */
/*----------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <signal.h>
#include "v2x_radio_api.h"

/*----------------------------------------------------------------------------*/
/* Macros                                                                     */
/*----------------------------------------------------------------------------*/
//#define DEBUG

#ifdef DEBUG
#define DEBUG_PRINTF(fmt, args...) \
    fprintf(stderr, fmt, ##args);
#else
#define DEBUG_PRINTF(fmt, args...)
#endif // DEBUG

/*----------------------------------------------------------------------------*/
/* Constants                                                                  */
/*----------------------------------------------------------------------------*/
#define DEFAULT_INTERFACE_NAME "rmnet_data1"
#define MAX_INTERFACE_NAME_LEN 32
/* After doing a v2x_radio_query_parameters(), the value of
 * link_non_ip_MTU_bytes is 2000 */
#define MAX_PACKET_SIZE_BYTES  2000
#define MAX_L2_ADDR_LEN        6

/*----------------------------------------------------------------------------*/
/* Local variables                                                            */
/*----------------------------------------------------------------------------*/
static char    interface_name[MAX_INTERFACE_NAME_LEN] = DEFAULT_INTERFACE_NAME;
static int     channel_center_khz                     = 5860000;
static int     channel_bandwidth_mhz                  = 10;
static int     tx_power_limit_decidbm                 = 230;
static int     qty_auto_retrans                       = 1;
static uint8_t l2_source_addr_length_bytes            = 3;
static uint8_t l2_source_addr_p[MAX_L2_ADDR_LEN]      =
    {0x0, 0x1, 0x2, 0x3, 0x4, 0x5};
static bool    set_macphy                             = false;
static char packet[MAX_PACKET_SIZE_BYTES];
static bool mainLoop = true;
static int  rx_sock  = -1;

/*----------------------------------------------------------------------------*/
/* Functions                                                                  */
/*----------------------------------------------------------------------------*/
static char *v2x_status_enum_type_to_str(v2x_status_enum_type status)
{
    char *retVal;
    static char unknown[40]; // Must be long enough to hold
                             // "Unknown V2X_STATUS (<number>)"

    switch (status) {
    case V2X_STATUS_SUCCESS:
        retVal = "V2X_STATUS_SUCCESS";
        break;

    case V2X_STATUS_FAIL:
        retVal = "V2X_STATUS_FAIL";
        break;

    case V2X_STATUS_ENO_MEMORY:
        retVal = "V2X_STATUS_ENO_MEMORY";
        break;

    case V2X_STATUS_EBADPARM:
        retVal = "V2X_STATUS_EBADPARM";
        break;

    case V2X_STATUS_EALREADY:
        retVal = "V2X_STATUS_EALREADY";
        break;

    case V2X_STATUS_KINETICS_PLACEHOLDER:
        retVal = "V2X_STATUS_KINETICS_PLACEHOLDER";
        break;

    case V2X_STATUS_RADIO_PLACEHOLDER:
        retVal = "V2X_STATUS_RADIO_PLACEHOLDER";
        break;

    case V2X_STATUS_ECHANNEL_UNAVAILABLE:
        retVal = "V2X_STATUS_ECHANNEL_UNAVAILABLE";
        break;

    case V2X_STATUS_VEHICLE_PLACEHOLDER:
        retVal = "V2X_STATUS_VEHICLE_PLACEHOLDER";
        break;

    default:
        retVal = unknown;
        snprintf(unknown, 32, "Unknown V2X_STATUS (%d)", (int)status);
        break;
    }
    
    return retVal;
}

static char *v2x_event_t_to_str(v2x_event_t event)
{
    char *retVal;
    static char unknown[40]; // Must be long enough to hold
                             // "Unknown V2X event (<number>)"

    switch (event) {
    case V2X_INACTIVE:
        retVal = "V2X_INACTIVE";
        break;

    case V2X_ACTIVE:
        retVal = "V2X_ACTIVE";
        break;

    case V2X_TX_SUSPENDED:
        retVal = "V2X_TX_SUSPENDED";
        break;

    case V2X_RX_SUSPENDED:
        retVal = "V2X_RX_SUSPENDED";
        break;

    case V2X_TXRX_SUSPENDED:
        retVal = "V2X_TXRX_SUSPENDED";
        break;

    default:
        retVal = unknown;
        snprintf(unknown, 32, "Unknown V2X event (%d)", (int)event);
        break;
    }
    
    return retVal;
}

/* v2x_radio_init() callbacks */
static void radio_init_complete(v2x_status_enum_type status, void *context)
{
    printf("%s: status  = %s\n", __func__, v2x_status_enum_type_to_str(status));
    printf("%s: context = %p\n", __func__, context);
}

static void radio_status_listener(v2x_event_t event, void *context)
{
    printf("%s: event   = %s\n", __func__, v2x_event_t_to_str(event));
    printf("%s: context = %p\n", __func__, context);
}

static void radio_chan_meas_listener(v2x_chan_measurements_t *measurements,
                                     void *context)
{
    printf("%s: channel_busy_percentage = %f\n",
            __func__, measurements->channel_busy_percentage);
    printf("%s: noise_floor             = %f\n",
           __func__, measurements->noise_floor);
    printf("%s: context                 = %p\n",
           __func__, context);
}

static void radio_l2_addr_changed_listener(int new_l2_address, void *context)
{
    printf("%s: new_l2_address = %d 0x%x\n",
           __func__, new_l2_address, new_l2_address);
    printf("%s: context        = %p\n", __func__, context);
}

static void radio_macphy_change_complete_cb(void *context)
{
    printf("%s: context = %p\n",      __func__, context);
}

#ifdef DEBUG
static void dump(const void *ptr, int len)
{
    unsigned char *p = (unsigned char *)ptr;
    int            l;
    int            i;
    char           str[18];

    while (len != 0) {
        l = ((len > 16) ? 16 : len);
        printf("%04lx ", p - (unsigned char *)ptr);

        for (i = 0; i < l; i++) {
            printf("%02x ", p[i]);
        }

        str[0] = ' ';

        for (i = 0; i < l; i++) {
            if ((p[i] >= ' ') && (p[i] <= '~')) {
                str[i + 1] = p[i];
            }
            else {
                str[i + 1] = '.';
            }
        }

        str[l + 1] = '\0';
        printf("%s\n", str);

        p += l;
        len -= l;
    }
}
#endif

int parseArg(int argc, char *argv[])
{
    int           i;
    int           byte, digit;
    int           value[2];
    int           argv_index = 1;
    uint16_t      pCommandArray_index;
    unsigned long temp;

    const char *pCommandArray[] = {
        "IFNAME",
        "CHANCENTER",
        "CHANBW",
        "TXPWRLIM",
        "QTYAUTORETRANS",
        "L2SRCADDRLEN",
        "L2SRCADDR",
    };

    /* Have MAX_CMD_BUF be the sze of the longest entry in pCommandArray */
    #define MAX_CMD_BUF sizeof("QTYAUTORETRANS")
    #define MAX_CMDS    (sizeof(pCommandArray) / sizeof(pCommandArray[0]))
    char tempBuf[MAX_CMD_BUF];

    while (argv_index < argc) {
        /* Be sure parameter begins with "--" */
        if ((argv[argv_index][0] != '-') || (argv[argv_index][1] != '-')) {
            return 0;
        }

        memset(tempBuf, 0, MAX_CMD_BUF);

        /* Get the parameter name, converted to all upper case, in tempBuf */
        for (i = 0; (i < MAX_CMD_BUF) && (i < strlen(&argv[argv_index][2])); i++) {
            tempBuf[i] = (char)toupper((int)(argv[argv_index][i + 2]));
        }

        tempBuf[i] = '\0';

        /* Find where the command is in pCommandArray */
        for (pCommandArray_index = 0; pCommandArray_index < MAX_CMDS;
             pCommandArray_index++) {
            if (strncmp((const char *)pCommandArray[pCommandArray_index],
                        (const char *)tempBuf,
                        strlen(pCommandArray[pCommandArray_index])) == 0) {
                break;
            }
        }

        /* Error if not found */
        if (pCommandArray_index == MAX_CMDS) {
            return 0;
        }

        /* Point to the value; error if command specified without a value */
        if (++argv_index >= argc) {
            return 0;
        }

        /* Convert from a string to an integer unless command is IFNAME or
         * L2SRCADDR */
        if ((pCommandArray_index != 0) && (pCommandArray_index != 6)) {
            temp = strtoul(argv[argv_index], NULL, 0);
        }

        /* Process the parameter */
        switch (pCommandArray_index) {
        case 0:     // IFNAME
            strncpy(interface_name, argv[argv_index], MAX_INTERFACE_NAME_LEN);
            break;

        case 1:     // CHANCENTER
            channel_center_khz = temp;
            set_macphy = true;
            break;

        case 2:     // CHANBW
            channel_bandwidth_mhz = temp;
            set_macphy = true;
            break;

        case 3:     // TXPWRLIM
            tx_power_limit_decidbm = temp;
            set_macphy = true;
            break;

        case 4:     // QTYAUTORETRANS
            qty_auto_retrans = temp;
            set_macphy = true;
            break;

        case 5:     // L2SRCADDRLEN
            l2_source_addr_length_bytes = temp;
            set_macphy = true;
            break;

        case 6:    // L2SRCADDR
            byte = digit = 0;

            for (i = 0; byte < MAX_L2_ADDR_LEN; i++) {
                if (((argv[argv_index][i] >= '0')   &&
                     (argv[argv_index][i] <= '9'))  ||
                    ((argv[argv_index][i] == ':')   ||
                     (argv[argv_index][i] == '\0')) ||
                    ((argv[argv_index][i] >= 'A')   &&
                     (argv[argv_index][i] <= 'F'))  ||
                    ((argv[argv_index][i] >= 'a')   &&
                     (argv[argv_index][i] <= 'f'))) {
                    /* If here, argv[argv_index][i] is 0-9, A-F, a-f, or : */

                    /* Check for end of 1 or 2 digit hex value */
                    if ((argv[argv_index][i] == ':') ||
                        (argv[argv_index][i] == '\0')) {
                        /* End of this value */
                        if (digit == 0) {
                            /* ERROR no digits */
                            return 0;
                        }

                        if (digit == 1) { // only 1 digit like 5:23:3f:....
                            l2_source_addr_p[byte] = value[0];
                        }
                        else {
                            l2_source_addr_p[byte] = value[0] * 16 + value[1];
                        }

                        byte++;
                        digit = 0;

                        if (argv[argv_index][i] == '\0') {
                            /* End of string; break out of for loop */
                            break;
                        }

                        continue;
                    }

                    if (digit == 2) {
                        /* ERROR too many digits */
                        return 0;
                    }

                    if ((argv[argv_index][i] >= 'A') &&
                        (argv[argv_index][i] <= 'F')) {
                        /* Convert from upper case to lower case */
                        argv[argv_index][i] = argv[argv_index][i] + 'a' -'A';
                    }

                    /* Convert to a binary digit value */
                    if ((argv[argv_index][i] >= '0') &&
                        (argv[argv_index][i] <= '9')) {
                        value[digit] = argv[argv_index][i] - '0';
                    }
                    else if ((argv[argv_index][i] >= 'a') &&
                             (argv[argv_index][i] <= 'f')) {
                        value[digit] = argv[argv_index][i] - 'a' + 0xa;
                    }

                    digit++;
                }
                else {
                    /* Illegal character */
                    return 0;
                }
            }

            set_macphy = true;
            break;

        default:
            return 0;
            break;
        } // switch (pCommandArray_index)

        /* Move past the value to the next parameter, if any */
        argv_index++;
    } // while (argv_index < argc)

    return 1;
}

void showSyntax(int argc, char *argv[])
{
    printf("Parameters to %s:\n"
           "  Parameter        Descruption              Units and/or type\n"
           "  ---------------- ------------------------ -----------------\n"
           "  --ifname         Interface name           string\n"
           "  --chancenter     Channel center           khz int\n"
           "  --chanbw         Channel bandwidth        mhz int\n"
           "  --txpwrlim       Tx power limit           decidbm int\n"
           "  --qtyautoretrans Auto retrans limit       int\n"
           "  --l2srcaddrlen   L2 source address length uint8_t\n"
           "  --l2srcaddr      L2 source address        1 or 2 hex digits per byte, separated by :\n",
           argv[0]);
}

void v2x_radio_rx_exit_handler(int signal)
{
    int result;

    if (mainLoop) {
        printf("%s is called.\n", __func__);
        mainLoop = false;

        /* Close the rx_sock here. This will allow the recv() routine to
         * return if it is waiting for a packet. */
        if (rx_sock >= 0) {
            /*
             * Close the Rx socket:
             */
            printf("Calling v2x_radio_sock_close()\n");
            result = v2x_radio_sock_close(&rx_sock);
            printf("Returned from v2x_radio_sock_close(): result = %d\n", result);

            if (result < 0) {
                /* v2x_radio_sock_close() returns the value of close(sock) */
                perror("v2x_radio_sock_close");
            }
        }
    }
}

int main(int argc, char **argv)
{
    v2x_radio_calls_t         radio_calls;
    v2x_radio_handle_t        handle;
    int                       test_ctx;
    v2x_status_enum_type      v2xStatus;
    struct sockaddr_in6       rx_sockaddr;
    int                       result;
    ssize_t                   bytes;
    v2x_chan_meas_params_t    meas_params;
    v2x_radio_macphy_params_t macphy;

    /* Process command line arguments */
    if (parseArg(argc, argv) == 0) {
        showSyntax(argc, argv);
        return 0;
    }

    /*
     * Initialize the radio interface:
     */
    radio_calls.v2x_radio_init_complete             = radio_init_complete;
    radio_calls.v2x_radio_status_listener           = radio_status_listener;
    radio_calls.v2x_radio_chan_meas_listener        = radio_chan_meas_listener;
    radio_calls.v2x_radio_l2_addr_changed_listener  = 
        radio_l2_addr_changed_listener;
    radio_calls.v2x_radio_macphy_change_complete_cb = 
        radio_macphy_change_complete_cb;
    printf("Calling v2x_radio_init(\"%s\", ...)\n", interface_name);
    handle = v2x_radio_init(interface_name, V2X_WWAN_NONCONCURRENT,
                            &radio_calls, &test_ctx);
    printf("Returned from v2x_radio_init(): handle = %d\n", handle);
    
    if (handle == V2X_RADIO_HANDLE_BAD) {
        fprintf(stderr, "Error: v2x_radio_init() returned "
                        "V2X_RADIO_HANDLE_BAD\n");
        goto v2x_radio_rx_exit;
    }

    /*
     * Optionally set MACPHY parameters
     */
    if (set_macphy) {
        macphy.channel_center_khz          = channel_center_khz;
        macphy.channel_bandwidth_mhz       = channel_bandwidth_mhz;
        macphy.tx_power_limit_decidbm      = tx_power_limit_decidbm;
        macphy.qty_auto_retrans            = qty_auto_retrans;
        macphy.l2_source_addr_length_bytes = l2_source_addr_length_bytes;
        macphy.l2_source_addr_p            = l2_source_addr_p;

        printf("Calling v2x_radio_set_macphy()\n");
        v2xStatus = v2x_radio_set_macphy(handle, &macphy, NULL);
        printf("v2x_radio_set_macphy() returned %s\n",
               v2x_status_enum_type_to_str(v2xStatus));

        if (v2xStatus != V2X_STATUS_SUCCESS) {
            fprintf(stderr,
                    "v2x_radio_set_macphy did not return V2X_STATUS_SUCCESS\n");
            goto v2x_radio_rx_exit;
        }
    }

    /*
     * Start channel meaurements
     */
    meas_params.channel_measurement_interval_us = 1000000;
    meas_params.rs_threshold_decidbm            = -950;
    printf("Calling v2x_radio_start_measurements()\n");
    v2xStatus = v2x_radio_start_measurements(handle, &meas_params);
    printf("Returned from v2x_radio_start_measurements(): v2xStatus = %s\n",
           v2x_status_enum_type_to_str(v2xStatus));

    if (v2xStatus != V2X_STATUS_SUCCESS) {
        fprintf(stderr, "Error: v2x_radio_start_measurements() did not return"
                        " V2X_STATUS_SUCCESS\n");
    }

    /*
     * Open a socket and initialize the socket address:
     */
    printf("Calling v2x_radio_rx_sock_create_and_bind()\n");
    result = v2x_radio_rx_sock_create_and_bind(handle, &rx_sock, &rx_sockaddr);
    printf("Returned from v2x_radio_rx_sock_create_and_bind(): "
           "result = %d; rx_sock = %d\n", result, rx_sock);

    if (result != 0) {
        perror("v2x_radio_rx_sock_create_and_bind:");
        goto v2x_radio_rx_exit;
    }

    /* Capture the termination signals */
    signal(SIGINT,  v2x_radio_rx_exit_handler);
    signal(SIGTERM, v2x_radio_rx_exit_handler);

    /*
     * (Rx main loop)
     */
    while (mainLoop) {
        /*
         * Receive data:
         */
        DEBUG_PRINTF("Calling recv()\n");
        bytes = recv(rx_sock, packet, MAX_PACKET_SIZE_BYTES, 0);
        DEBUG_PRINTF("Returned from recv(): bytes = %d\n", bytes);
        
        if (bytes < 0) {
            perror("recv");
            goto v2x_radio_rx_exit;
        }

        printf("bytes = %ld\n", bytes);
#ifdef DEBUG
        dump(packet, bytes);
#endif
    }

v2x_radio_rx_exit:
    /*
     * De-initialize the radio
     */
    if (handle != V2X_RADIO_HANDLE_BAD) {
        printf("Calling v2x_radio_deinit()\n");
        v2xStatus = v2x_radio_deinit(handle);
        printf("Returned from v2x_radio_deinit(): v2xStatus = %s\n",
               v2x_status_enum_type_to_str(v2xStatus));

        if (v2xStatus != V2X_STATUS_SUCCESS) {
            fprintf(stderr, "Error: v2x_radio_deinit() did not return"
                            " V2X_STATUS_SUCCESS\n");
        }
    }

    return 0;
}

