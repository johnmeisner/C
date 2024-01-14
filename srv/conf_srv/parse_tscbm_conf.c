
/*
 *  Filename: parse_tscbm_conf.c
 *  Purpose: A simple program to munch tscbm.conf into pieces for the webgui
 *
 *  Copyright (C) 2021 DENSO International America, Inc.
 *
 *  NOTE: The output format is:
 *          line 1:  # of data rows
 *          lines2+: 4-column data lines of phasetype phaseno SignalGroupId GreenMeaning
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "libtsmapreader.h"

int main(int argc, char **argv)
{
    char value_vo, value_er;
    int value_src_int, value_dest_int;
    char tmpbuff[100];
    char outbuff[10000];
    int num_lines;

    if (argc != 2) {
        printf("Usage: parse_tcsbm_conf path_to_tcsvm.conf_file\n");
        exit(0);
    }

    /* Read and compare */
    if (! tsmr_open(argv[1])) {
        printf("FAIL: Test1 Couldn't open file %s\n", argv[1]);
        return 1;
    }
    num_lines = 0;
    outbuff[0] = 0;
    while (tsmr_readnext(&value_vo, &value_src_int, &value_dest_int, &value_er) > 0) { 
        ++num_lines;
        sprintf(tmpbuff, "%c %d %d %c\n", value_vo, value_src_int, value_dest_int, value_er);
        strcat(outbuff, tmpbuff);
    }

    printf("%d\n", num_lines);
    printf("%s", outbuff);

    return 0;
}
