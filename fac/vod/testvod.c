/**************************************************************************
 *                                                                        *
 *     File Name:  test_client.c                                          *
 *     Author:                                                            *
 *         DENSO International America, Inc.                              *
 *         North America Research Laboratory, California Office           *
 *         3252 Business Park Drive                                       *
 *         Vista, CA 92081                                                *
 *                                                                        *
 **************************************************************************/

#include <stdio.h>
#include <syslog.h>         // For LOG_ERR
#include <stdlib.h>         // For exit
#include <string.h>         // For memset, memcpy
#include "wsu_sharedmem.h"  // for gates & shmlocks
#include "v2x_common.h"     // for V2X_LOG
#include "vod_api.h"


#define APP_ABBREV "TESTVOD"

char Usage[] = "Usage: vod_test <number_of_vods>\n";

int main(int argc, char **argv)
{
    int i, rc, result;
    int requests_requested;
    int cnt_results[10];
    int8_t vod_request_handle;
    uint32_t vod_msg_seq_num;

    if (argc != 2) {
        exit(printf(Usage));
    }
    requests_requested = atoi(argv[1]);

    printf("Connecting to VOD application ...\n");

    rc = init_with_vod(APP_ABBREV);

    printf("Connected!\n");

    for (i=0; i<10; i++)
        cnt_results[i] = 0;


    printf("Running %d VOD requests ...\n", requests_requested);
    for (i=0; i<requests_requested; i++) {
        vod_msg_seq_num = i;
        vod_request_handle = request_msg_verification(vod_msg_seq_num, 10 + 9 *(i % 10));
        if (vod_request_handle < 0) { printf("testvod: ERROR: request_msg_verif() # %d failed\n", i); }
        rc = wait_for_vod_result(vod_request_handle);
        if (! rc) { printf("testvod: ERROR: wait_for_vod_result() # %d w req_handle %d failed\n", i, vod_request_handle); }
        result = retrieve_vod_result(vod_request_handle);
        ++cnt_results[result];
    }
    printf("Done with requests!\n");

    printf("From %d requests, I got\n", requests_requested);
    for (i=0; i<10; i++) {
        if (cnt_results[i] > 0) {
            printf("%8d x %d %s\n", cnt_results[i], i,
                        i == NOREQUEST ? "(NoRequest)"
                      : i == REQUESTED ? "(Requested)"
                      : i == INPROCESS ? "(InProcess)"
                      : i == SUCCESS ? "(Success)"
                      : i == FAIL ? "(Fail)"
                      : i == ERROR ? "(Error)"
                      : "(Unknown)");
        }
    }

    denit_with_vod();

    printf("Done.\n");
    return 0;
}
