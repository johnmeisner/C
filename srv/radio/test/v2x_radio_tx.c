/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Filename: v2x_radio_tx.c                                                   */
/*  Purpose: Test program that transmits packets using the v2x_radio library  */
/*                                                                            */
/* Copyright (C) 2019 DENSO International America, Inc.  All Rights Reserved. */
/*                                                                            */
/* Revision History:                                                          */
/*                                                                            */
/* Date        Author        Comments                                         */
/* -------------------------------------------------------------------------- */
/* 2019-06-13  VROLLINGER    Initial revision.                                */
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
/* Constants                                                                  */
/*----------------------------------------------------------------------------*/
#define SPS_SRC_PORT_NUM       2500 /* From "acme" */
#define EVENT_SRC_PORT_NUM     2600 /* From "acme" */
#define DEFAULT_INTERFACE_NAME "rmnet_data1"
#define MAX_INTERFACE_NAME_LEN 32
#define EVENT_TX_PRIO          5
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
static bool    mainLoop                               = true;

/* Functions not used in this program:
 *     v2x_convert_traffic_class_to_priority()
 *     v2x_radio_api_version()
 *     v2x_radio_query_parameters()
 *     v2x_radio_rx_sock_create_and_bind()
 *     v2x_radio_tx_reservation_change()
 *     v2x_radio_tx_flush()
 *     v2x_radio_start_measurements()
 *     v2x_radio_stop_measurements()
 */

/*----------------------------------------------------------------------------*/
/* Functions                                                                  */
/*----------------------------------------------------------------------------*/
static char *v2x_status_enum_type_to_str(v2x_status_enum_type status)
{
    char *retVal;
    static char unknown[40]; // Must be long enough to hold "Unknown V2X_STATUS (<number>)"

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
    static char unknown[40]; // Must be long enough to hold "Unknown V2X event (<number>)"

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

static void radio_chan_meas_listener(v2x_chan_measurements_t *measurements, void *context)
{
    printf("%s: channel_busy_percentage = %f\n", __func__, measurements->channel_busy_percentage);
    printf("%s: noise_floor             = %f\n", __func__, measurements->noise_floor);
    printf("%s: context                 = %p\n", __func__, context);
}

static void radio_l2_addr_changed_listener(int new_l2_address, void *context)
{
    printf("%s: new_l2_address = %d 0x%x\n", __func__, new_l2_address, new_l2_address);
    printf("%s: context        = %p\n",      __func__, context);
}

static void radio_macphy_change_complete_cb(void *context)
{
    printf("%s: context = %p\n",      __func__, context);
}

/* v2x_radio_tx_sps_sock_create_and_bind() callbacks */
static void radio_l2_reservation_change_complete_cb(void *context, v2x_sps_mac_details_t *details)
{
    printf("%s: context                           = %p\n", __func__, context);
    printf("%s: periodicity_in_use_ns             = %u\n", __func__, details->periodicity_in_use_ns);
    printf("%s: currently_reserved_periodic_bytes = %u\n", __func__, details->currently_reserved_periodic_bytes);
    printf("%s: tx_reservation_offset_ns          = %u\n", __func__, details->tx_reservation_offset_ns);
}

static void radio_sps_offset_changed(void *context, v2x_sps_mac_details_t *details)
{
    printf("%s: context                           = %p\n",      __func__, context);
    printf("%s: periodicity_in_use_ns             = %u\n", __func__, details->periodicity_in_use_ns);
    printf("%s: currently_reserved_periodic_bytes = %u\n", __func__, details->currently_reserved_periodic_bytes);
    printf("%s: tx_reservation_offset_ns          = %u\n", __func__, details->tx_reservation_offset_ns);
}

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

void v2x_radio_tx_exit_handler(int signal)
{
    if (mainLoop) {
        printf("%s is called.\n", __func__);
    }

    mainLoop = false;
}

int main(int argc, char **argv)
{
    v2x_radio_calls_t               v2x_radio_init_callbacks;
    v2x_per_sps_reservation_calls_t sps_function_calls;
    char packet[] =
    {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    };
    v2x_radio_handle_t              handle;
    v2x_status_enum_type            v2xStatus;
    struct sockaddr_in6             sps_sock_addr;
    int                             sps_sock     = -1;
    struct sockaddr_in6             event_sock_addr;
    int                             event_sock   = -1;
    v2x_tx_bandwidth_reservation_t  res_req;
    int                             bytes_sent   = 0;
    struct msghdr                   message      = {0};
    struct iovec                    iov[1]       = {0};
    struct cmsghdr                 *cmsghp;
    char                            control[CMSG_SPACE(sizeof(int))];
    int                             result;
    int                             packets_sent = 0;
    v2x_radio_macphy_params_t       macphy;

    /* Process command line arguments */
    if (parseArg(argc, argv) == 0) {
        showSyntax(argc, argv);
        return 0;
    }

    /*
     * Initialize the radio:
     */
    v2x_radio_init_callbacks.v2x_radio_init_complete             =
        radio_init_complete;
    v2x_radio_init_callbacks.v2x_radio_status_listener           =
        radio_status_listener;
    v2x_radio_init_callbacks.v2x_radio_chan_meas_listener        =
        radio_chan_meas_listener;
    v2x_radio_init_callbacks.v2x_radio_l2_addr_changed_listener  =
        radio_l2_addr_changed_listener;
    v2x_radio_init_callbacks.v2x_radio_macphy_change_complete_cb =
        radio_macphy_change_complete_cb;

    printf("Calling v2x_radio_init()\n");
    handle = v2x_radio_init(interface_name,
                            V2X_WWAN_NONCONCURRENT,
                            &v2x_radio_init_callbacks,
                            NULL); /* First parameter on the callback. ???? */
    printf("v2x_radio_init() returned %d\n", handle);

    if (handle == V2X_RADIO_HANDLE_BAD) {
        fprintf(stderr, "v2x_radio_init() failed\n");
        goto v2x_radio_tx_exit;
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
            goto v2x_radio_tx_exit;
        }
    }

    /*
     * Create an SPS flow:
     */
    res_req.v2xid                     = 1;          /* From "acme" */
    res_req.priority                  = V2X_PRIO_2; /* From "acme" */
    res_req.period_interval_ms        = 100;        /* From "acme" */
    res_req.tx_reservation_size_bytes = 287;        /* From "acme" */
    sps_function_calls.v2x_radio_l2_reservation_change_complete_cb =
        radio_l2_reservation_change_complete_cb;
    sps_function_calls.v2x_radio_sps_offset_changed = radio_sps_offset_changed;

    printf("Calling v2x_radio_tx_sps_sock_create_and_bind()\n");
    result = v2x_radio_tx_sps_sock_create_and_bind(handle,
                                                   &res_req,
                                                   &sps_function_calls,
                                                   SPS_SRC_PORT_NUM,
                                                   EVENT_SRC_PORT_NUM,
                                                   &sps_sock,
                                                   &sps_sock_addr,
                                                   &event_sock,
                                                   &event_sock_addr);
    printf("v2x_radio_tx_sps_sock_create_and_bind() returned %d; sps_sock = %d; event_sock = %d\n",
           result, sps_sock, event_sock);

    if (result != V2X_STATUS_SUCCESS) {
        fprintf(stderr, "v2x_radio_tx_sps_sock_create_and_bind() failed: %s\n",
               strerror(-result));
        sps_sock = -1;
        event_sock = -1;
        goto v2x_radio_tx_exit;
    }

#if 0
    /*
     * Create an event-driven socket:
     */
    printf("Calling v2x_radio_tx_event_sock_create_and_bind()\n");
    result = v2x_radio_tx_event_sock_create_and_bind(interface_name,
                                                     res_req.v2xid,
                                                     EVENT_SRC_PORT_NUM,
                                                     &event_sock_addr,
                                                     &event_sock);

    if (result != V2X_STATUS_SUCCESS) {
        fprintf(stderr, "v2x_radio_tx_event_sock_create_and_bind() failed: "
                        "%s\n", strerror(-result));
        event_sock = -1;
        goto v2x_radio_tx_exit;
    }

    printf("v2x_radio_tx_event_sock_create_and_bind() succeeded; "
           "event_sock = %d\n", event_sock);
#endif

    /* Capture the termination signals */
    signal(SIGINT,  v2x_radio_tx_exit_handler);
    signal(SIGTERM, v2x_radio_tx_exit_handler);

    /*
     * Send Tx data:
     */
    while (mainLoop) {
        /* Send data using sendmsg to provide IPV6_TCLASS per packet */
        iov[0].iov_base = packet;
        iov[0].iov_len  = sizeof(packet);

        message.msg_iov        = iov;
        message.msg_iovlen     = 1;
        message.msg_control    = control;
        message.msg_controllen = sizeof(control);

        /* Fill ancillary data */
        cmsghp = CMSG_FIRSTHDR(&message);
        cmsghp->cmsg_level = IPPROTO_IPV6;
        cmsghp->cmsg_type  = IPV6_TCLASS;
        cmsghp->cmsg_len   = CMSG_LEN(sizeof(int));

        *((int *)CMSG_DATA(cmsghp)) =
            v2x_convert_priority_to_traffic_class(EVENT_TX_PRIO);

        bytes_sent = sendmsg(sps_sock, &message, 0);

        if (!mainLoop) {
            break;
        }

        if (bytes_sent < 0) {
            fprintf(stderr, "Error sending message: %d\n", bytes_sent);
            break;
        }

        printf("Packets sent: %d\n", ++packets_sent);
        usleep(100000);
    }

v2x_radio_tx_exit:
    /*
     * Close the sockets:
     */
    if (sps_sock >= 0) {
        printf("Calling v2x_radio_sock_close(&sps_sock)\n");
        result = v2x_radio_sock_close(&sps_sock);
        printf("v2x_radio_sock_close(&sps_sock) returned %d\n", result);

        if (result < 0) {
            perror("v2x_radio_sock_close(&sps_sock)");
        }
    }

    if (event_sock >= 0) {
        printf("Calling v2x_radio_sock_close(&event_sock)\n");
        result = v2x_radio_sock_close(&event_sock);
        printf("v2x_radio_sock_close(&event_sock) returned %d\n", result);

        if (result < 0) {
            perror("v2x_radio_sock_close(&event_sock)");
        }
    }

    /*
     * De-initialize the radio:
     */
    if (handle != V2X_RADIO_HANDLE_BAD) {
        printf("Calling v2x_radio_deinit(%d)\n", handle);
        v2xStatus = v2x_radio_deinit(handle);
        printf("v2x_radio_deinit() returned %s\n",
               v2x_status_enum_type_to_str(v2xStatus));

        if (v2xStatus != V2X_STATUS_SUCCESS) {
            fprintf(stderr, 
                    "v2x_radio_deinit() did not return V2X_STATUS_SUCCESS\n");
        }
    }

    return 0;
}

