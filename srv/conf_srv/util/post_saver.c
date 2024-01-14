/*
 *  Filename: post_saver.c
 *  Purpose: Utility for a cgi-bin script to save POST content to a file
 *
 *  Copyright (C) 2021 DENSO International America, Inc.
 */

#include <stdio.h>
#include <string.h>
#include "libtimefgets.h"

#define FGETS_WAIT_MSEC 50

int main (int argc, char **argv)
{
    size_t bytes_wrote;
    char buffer[10000];
    int bytes_read;
    FILE * fd;

    if (argc != 2 || (argc > 1 && (!strcmp(argv[1], "-h") || !strncmp(argv[1],"--h",3))))
    {
        printf("Usage: post_saver filename_to_save_post_into\n");
        return 0;
    }


    fd = fopen(argv[1], "w");
    if (fd == NULL) {
        printf("ERROR: Unable to create file %s\n", argv[1]);
        return -1;
    }

    bytes_read = timefgets(buffer, sizeof(buffer), stdin, FGETS_WAIT_MSEC);
    if (bytes_read > 0) {
        bytes_wrote = fwrite(buffer, 1, bytes_read, fd);
        fwrite("\n", 1, 1, fd);     // Need to add back the newline eaten by timefgets
        printf("Read %d bytes, wrote %lu bytes to %s\n", bytes_read, bytes_wrote, argv[1]);
    } else if (bytes_read == 0) {
        printf("Read zero bytes, nothing written to %s\n", argv[1]);
    } else if (bytes_read == TIMEFGETS_EOF) {
        printf("Read got EOF, nothing written to %s\n", argv[1]);
    } else {
        printf("Read returned error %d, nothing written to %s\n", bytes_read, argv[1]);
    }
    fclose(fd);

}

