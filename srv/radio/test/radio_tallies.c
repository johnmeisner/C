/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Filename: radio_tallies.c                                                  */
/*  Purpose: Test program that receives WSMs                                  */
/*                                                                            */
/* Copyright (C) 2019 DENSO International America, Inc.  All Rights Reserved. */
/*                                                                            */
/* Revision History:                                                          */
/*                                                                            */
/* Date        Author        Comments                                         */
/* -------------------------------------------------------------------------- */
/* 2019-08-08  VROLLINGER    Initial revision.                                */
/*                                                                            */
/*----------------------------------------------------------------------------*/
/*
 * Display tallies of various counters (including errors) from Radio Stack.
 * The command line is as follows:
 *
 * radio_tallies [-t <radio_type>][-i <interface>]
 * <radio_type>: cv2x        --C-V2X radio
 *               dsrc        --MK5 radio
 * <interface>:  radio0      --MK5 radio 0
 *               wifi0       --MK5 radio 0
 *               radio1      --MK5 radio 1
 *               wifi1       --MK5 radio 1
 *               <default>   --MK5 radio 0  (displays device name)
 */

/*----------------------------------------------------------------------------*/
/* Include Files                                                              */
/*----------------------------------------------------------------------------*/
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <err.h>
#include "ris.h"
#include "type2str.h"

/*----------------------------------------------------------------------------*/
/* Local Variables                                                            */
/*----------------------------------------------------------------------------*/
static char_t ifname[8];

/*----------------------------------------------------------------------------*/
/* Functions                                                                  */
/*----------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
** Function:  getRisDataRateStr
** @brief  Verify
** @brief  The MK5 refers to the DataRate by it's corresponding modulation and coding scheme
** @param  RisDataRate  --RIS Datarate  (enum dataRateType:  DR_3_MBPS...)
** @return drstr        --pointer to a static string representing the DataRate (MCS)
**----------------------------------------------------------------------------*/
char_t *getRisDataRateStr(dataRateType RisDataRate)
{
    static char_t  drStr[16];  /*returns pointer to static string -- non-reentrant code*/

    switch (RisDataRate) {
    case  DR_4_5_MBPS:
         snprintf(drStr,16,"%2d(%3.1f Mbs)", RisDataRate, (RisDataRate/2.));
         break;
    case  DR_6_MBPS:
    case  DR_3_MBPS:
    case  DR_9_MBPS:
    case  DR_12_MBPS:
    case  DR_18_MBPS:
    case  DR_24_MBPS:
    case  DR_27_MBPS:
         snprintf(drStr,16,"%2d(%2d Mbs) ", RisDataRate, (RisDataRate/2));
         break;
    default:
         snprintf(drStr,16,"%2d *       ", RisDataRate);
         break;
    }

    return drStr;
}


static void print_dn_tallies(rsRadioType radioType, DeviceTallyType *tallies)
{
    float64_t error_rate = 0.0;

    printf("=================== %s %s RADIO STATS =========================\n",
           (radioType == RT_CV2X) ? "C-V2X" : "DSRC", ifname);
    printf("Rcvd Good Pkt     : %-10d     ", tallies->rx_good);
    printf("Xmit Good Pkt     : %-10d\n", tallies->tx_good);

    printf("Rcvd Good Bytes   : %-10d     ", tallies->rx_good_bytes);
    printf("Xmit Good Bytes   : %-10d\n", tallies->tx_good_bytes);

    printf("Rcvd Rate         : %s    ", getRisDataRateStr(tallies->rx_rate));
    printf("Xmit Rate         : %s\n",   getRisDataRateStr(tallies->tx_rate));

    printf("Rcvd Ant 1        : %-10d     ", tallies->ant_rx[0]);
    printf("Xmit Ant 1        : %-10d\n", tallies->ant_tx[0]);

    printf("Rcvd Ant 2        : %-10d     ", tallies->ant_rx[1]);
    printf("Xmit Ant 2        : %-10d\n", tallies->ant_tx[1]);

    printf("-------------------\n");

    printf("Rcvd Unicast Pkt  : %-10d     ", tallies->rx_ucast);
    printf("Xmit Unicast Pkt  : %-10d\n", tallies->tx_ucast);

    printf("Rcvd Broadcast Pkt: %-10d     ", tallies->rx_bcast);
    printf("Xmit Broadcast Pkt: %-10d\n", tallies->tx_bcast);

    printf("Rcvd Multicast Pkt: %-10d     ", tallies->rx_mcast);
    printf("Xmit Multicast Pkt: %-10d\n", tallies->tx_mcast);

    printf("-------------------\n");
    printf("Rcvd Data Pkt     : %-10d     ", tallies->rx_data);
    printf("Xmit Data Pkt     : %-10d\n", tallies->tx_data);

    printf("Rcvd Control Pkt  : %-10d     ", tallies->rx_ctrl);
    printf("Xmit Ctrl Pkt     : %-10d\n", tallies->tx_ctrl);

    printf("Rcvd Mgmt Pkt     : %-10d     ", tallies->rx_mgt);
    printf("Xmit Mgmt Pkt     : %-10d\n", tallies->tx_mgt);

    printf("-------------------\n");
    printf("Rcvd Total Errors : %-10d     ", tallies->rx_err);
    printf("Xmit Total Errors : %-10d\n", tallies->tx_err);

    printf("Rcvd CRC Errors   : %-10d     ", tallies->rx_crc_err);
    printf("Xmit Dropped      : %-10d\n", tallies->tx_drop);

    printf("Rcvd Phy Errors   : %-10d     ", tallies->rx_phy_err);
    printf("Xmit Retries      : %-10d\n", tallies->tx_retries);

    printf("-------------------\n");
    ///Used on WSU:   printf("Rx RSSI           : -%d dBm        ", 100 - tallies->rssi);
    /*
     * For MK5, return the raw Antenna Power given by MK5 LLC (already in dBm).
     * For MK5, add existing tally "last received antenna" since available and most values aren't.
     */
    printf("Rx Last RSSI      : %-3d dBm        ", tallies->rssi);
    printf("Tx Last Power     : %-10.1f\n",         ((float)tallies->tx_power/2.));

    printf("Rx Last Ant       : %d              ", tallies->rx_ant);     /*Added for MK5 since available.*/
    printf("----------------------  \n");
    printf("Rx Overflow       : %-10d     ", tallies->rx_overrun);
    printf("MK5 RX Bad Pkt    : %-10d\n",    tallies->rx_mkx_bad_pkts);
    printf("Dup Seq Errors    : %-10d     ", tallies->rx_dup_seq_err);
    printf("MK5 RX Short Pkt  : %-10d\n",    tallies->rx_mkx_short_pkts);
    printf("Rx Ant Switch Cnt : %-10d     ", tallies->rxant_sw_cnt);
    printf("TX Bad Channel    : %-10d\n",    tallies->tx_bad_channel);
    printf("-------------------\n");
    printf("Raw Chan Busy (%%) : %-10d     \n",
            tallies->rawchan_busyratio);
    printf("WAVE CCH Busy (%%) : %-10d\n", tallies->wavecch_busyratio);
    printf("WAVE SCH Busy (%%) : %-10d\n", tallies->wavesch_busyratio);

    printf("-------------------\n");
    printf("Rx Queue Depth RD : %-10d\n", tallies->rx_queue_depth_radio_driver);
    printf("Tx Queue Depth AC0: %-10d     ", tallies->tx_queue_depth[0]);
    printf("Tx Queue Drop AC0 : %-10d\n", tallies->tx_queue_drop[0]);
    printf("Tx Queue Depth AC1: %-10d     ", tallies->tx_queue_depth[1]);
    printf("Tx Queue Drop AC1 : %-10d\n", tallies->tx_queue_drop[1]);
    printf("Tx Queue Depth AC2: %-10d     ", tallies->tx_queue_depth[2]);
    printf("Tx Queue Drop AC2 : %-10d\n", tallies->tx_queue_drop[2]);
    printf("Tx Queue Depth AC3: %-10d     ", tallies->tx_queue_depth[3]);
    printf("Tx Queue Drop AC3 : %-10d\n", tallies->tx_queue_drop[3]);
    printf("Rx Queue Depth    : %-10d     ", tallies->rx_queue_depth);
    printf("Rx Queue Dropped  : %-10d\n", tallies->rx_queue_drop);

    error_rate = tallies->rx_phy_err + tallies->rx_crc_err;

    if (tallies->rx_phy_err + tallies->rx_crc_err + tallies->rx_good != 0) {
        error_rate /= tallies->rx_phy_err + tallies->rx_crc_err + tallies->rx_good;
    }

    printf("\nRx Error rate     :  %3.2f%%\n\n", error_rate * 100);

    /* Additional receive detail for Autotalks CV2X */
    if (radioType == RT_CV2X) {
        printf("CV2X RX Success Detail:\n");
        printf("  l2 messages received: %u\n", tallies->cv2x_rx_l2_messages_received_successfully);
        printf("  rlc messages reassembled: %u\n", tallies->cv2x_rx_rlc_messages_reassembled_successfully);
        printf("  pdcp messages received: %u\n", tallies->cv2x_rx_pdcp_messages_received_successfully);
        printf("  svc messages received: %u\n", tallies->cv2x_rx_svc_messages_received_successfully);
        printf("  mac messages received: %u\n", tallies->cv2x_rx_mac_messages_received_successfully);
        printf("CV2X RX Error Detail:\n");
        printf("  l2_messages_dropped_crc_errors = %u\n", tallies->cv2x_rx_l2_messages_dropped_crc_errors);
        printf("  l2_messages_dropped_queue_full = %u\n", tallies->cv2x_rx_l2_messages_dropped_queue_full);
        printf("  l2_messages_dropped_exceeding_max_rx_size = %u\n", tallies->cv2x_rx_l2_messages_dropped_exceeding_max_rx_size);
        printf("  rlc_messages_reassembly_timed_out = %u\n", tallies->cv2x_rx_rlc_messages_reassembly_timed_out);
        printf("  rlc_messages_duplicated = %u\n", tallies->cv2x_rx_rlc_messages_duplicated);
        printf("  pdcp_messages_dropped_parsing_error = %u\n", tallies->cv2x_rx_pdcp_messages_dropped_parsing_error);
        printf("  svc_messages_dropped = %u\n", tallies->cv2x_rx_svc_messages_dropped);
        printf("  mac_messages_dropped_invalid_destination_l2id = %u\n", tallies->cv2x_rx_mac_messages_dropped_invalid_destination_l2id);
        printf("  mac_messages_dropped_queue_full = %u\n", tallies->cv2x_rx_mac_messages_dropped_queue_full);
        printf("  mac_messages_dropped_invalid_struct = %u\n", tallies->cv2x_rx_mac_messages_dropped_invalid_struct);
        printf("  mac_messages_dropped_invalid_version = %u\n", tallies->cv2x_rx_mac_messages_dropped_invalid_version);
    }
}

/*------------------------------------------------------------------------------
** Function:  usage
** @brief     Display usage information the radio_tallies utility.
** @param     argv    --standard commandline parameters array.
** @return    void
**----------------------------------------------------------------------------*/
void usage(char_t *argv[])
{
    printf("%s --Provide tallies of various operations and errors\n", argv[0]);
    printf("                collected by the Radio Stack.\n");

    printf("usage:    %s [Options]\n", argv[0]);
    printf("Options:\n");
    printf("   -i <interface>  : Specify which radio interface  [Default=radio0]\n");
    printf("                     radio0 or wifi0  =MK5 Radio0 \n");
    printf("                     radio1 or wifi1  =MK5 Radio1 \n");
    printf("   -t <radio_type> : Specify the radio type  [Default=cv2x]\n");
    printf("                     cv2x  =C-V2X Radio\n");
    printf("                     dsrc  =DSRC Radio\n");
    printf("   -h      : Display this help\n");
    printf("\n");
}

int32_t main(int32_t argc, char_t *argv[])
{
    rsRadioType      radioType = RT_CV2X;
    uint8_t          radioNum  = 0;
    DeviceTallyType  tallies;
    rsResultCodeType risRet;
    int32_t          i = 1;

    strcpy(ifname, "radio0");

    while (i < argc) {
        if ((argc > i) && (strcmp(argv[i], "-h") == 0)) {
            usage(argv);
            return 0;
        }
        else if (argc > i && strcmp(argv[i], "-i") == 0) {
            i++;

            if (argc <= i) {
                fprintf(stderr, "%s: missing interface name for -i\n", argv[0]);
                exit(-1);
            }

            strncpy(ifname, argv[i], 6);
            ifname[6] = '\0';

            if (strcmp("wifi0",  ifname) == 0 ||
                strcmp("radio0", ifname) == 0) {
                radioNum = 0;
            }
            else if (strcmp("wifi1",  ifname) == 0 ||
                     strcmp("radio1", ifname) == 0) {
                radioNum = 1;
            }
            else {
                fprintf(stderr, "%s: no such interface name for -i\n", argv[0]);
                return -1;
            }

            i++;
        }
        else if (argc > i && strcmp(argv[1], "-t") == 0) {
            i++;

            if (argc <= i) {
                fprintf(stderr, "%s: missing radio type for -t\n", argv[0]);
                exit(-1);
            }

            if ((strcmp("CV2X",  argv[i]) == 0) ||
                (strcmp("cv2x",  argv[i]) == 0) ||
                (strcmp("C-V2X", argv[i]) == 0) ||
                (strcmp("c-v2x", argv[i]) == 0)) {
                radioType = RT_CV2X;
            }

            if ((strcmp("DSRC",  argv[i]) == 0) ||
                (strcmp("dsrc",  argv[i]) == 0)) {
                radioType = RT_DSRC;
            }

            i++;
        }
        else {
            usage(argv);
            return 0;
        }
    }


    memset(&tallies, 0x00, sizeof(tallies));
    risRet = wsuRisGetRadioTallies(radioType, radioNum, &tallies);

    if (risRet != RS_SUCCESS) {
        printf("wsuRisGetRadioTallies failed (%s)\n",
               rsResultCodeType2Str(risRet));
        return -1;
    }

    print_dn_tallies(radioType, &tallies);
    return 0;
}

