/*
 *  Filename: refresh_saver.c
 *  Purpose: Consolidate a stream of continually refreshed output to a
 *           single file, for asynchronous download by the webgui
 *
 *  Copyright (C) 2021 DENSO International America, Inc.
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>              // for O_CREAT and friends
#include "libtimefgets.h"

int debug_output = 0;             /* Debug OFF by default */

/*
 * Debug print
 */
#include <stdarg.h>  // for va_list
#include <errno.h>   // for errno
#include <string.h>  // for strerror
#define DPRINT(fmt, ...)  mylog("%s: " fmt, __FUNCTION__, ##__VA_ARGS__)
#define DEBUG_LOG_FILENAME  "/tmp/refresh_saver.log"
void mylog(char * fmt, ...)
{
    static char buffer[4000];
    static va_list argptr;
    static int fd;

    if (debug_output) {
        va_start(argptr, fmt);
        vsnprintf(buffer, sizeof(buffer), fmt, argptr);
        va_end(argptr);
        fd = open(DEBUG_LOG_FILENAME, O_CREAT|O_WRONLY|O_APPEND, 0644);
        if (fd >= 0) {
            write(fd, buffer, strlen(buffer));
            close(fd);
        } else {
            int errsv = errno;
            fprintf(stderr,"ERROR: Refresh_saver failed to append to its debug log! errno=%d=%s\n",
                errsv, strerror(errsv));
        }
    }
}

char * USAGE = "USAGE: refresh_saver [-d] [-s line_append_str] timeout_ms savefile";

int main(int argc, char *argv[])
{
   int c_arg;
   char * savefile;
   char tmpfile[1000];
   char read_buff[4000];
   FILE * fout;
   unsigned int wrote_byte_count;
   int timeout_ms;
   int bytes_read, total_bytes_read;
   char * line_append = "";
   int done = 0;

   /* Process options - currently only a switch for debug mode */
    while ((c_arg = getopt(argc, argv, "ds:")) != -1) {
        switch (c_arg) {
            case 'd':
                ++debug_output;
                DPRINT("Debug mode: %d\n", debug_output);
                break;

            case 's':
                line_append = optarg;
                DPRINT("Every output lines will now be appended with '%s'\n", line_append);
                break;

            default:
                printf("%s\n", USAGE);
                return -1;
                break;
        }
    }

    /* Detect usage */
    if (optind != argc - 2) {
        printf("%s\n", USAGE);
        return 0;
    }

    DPRINT("RefreshSaver compiled on " __DATE__ " at " __TIME__ "\n");

    timeout_ms = atoi(argv[optind]);

    DPRINT("RefreshSaver called with timeout %d (%s) and savefile %s\n",
        timeout_ms, argv[optind], argv[optind+1]);


    /* Start our refresh file */
    savefile = argv[optind+1];
    sprintf(tmpfile, "%s.tmp", savefile);
    fout = fopen(tmpfile,"wb");
    if (fout == NULL) {
        DPRINT("ERROR: Unable to create tmpfile %s\n", tmpfile);
        return -3;
    }
    total_bytes_read = 0;

    while (!done) {

        bytes_read = timefgets(read_buff, sizeof(read_buff), stdin, timeout_ms);
        if (bytes_read > 0) {
            total_bytes_read += bytes_read;
            wrote_byte_count = fprintf(fout, "%s%s\n", read_buff, line_append);
            if (wrote_byte_count < bytes_read) {
                DPRINT("ERROR: Failed to write %d bytes, only %d written!\n", bytes_read, wrote_byte_count);
                return -4;
            }
        } else {
            // Switch files if there's anything in the tmp file
            if (total_bytes_read > 0) {
                fclose(fout);
                DPRINT("Renaming %d-byte %s to %s\n", total_bytes_read, tmpfile, savefile);
                if (rename(tmpfile, savefile) != 0) {
                    DPRINT("ERROR: Rename of %d-byte tmp file failed with errno %d\n", total_bytes_read, errno);
                    return -5;
                }
                // And restart a fresh tmp file
                fout = fopen(tmpfile,"wb");
                if (fout == NULL) {
                    DPRINT("ERROR: Unable to create tmpfile %s\n", tmpfile);
                    return -6;
                }
                total_bytes_read = 0;
            }
            // If we hit EOF, let's exit
            if (bytes_read == TIMEFGETS_EOF) {
                DPRINT("Hit EOF on input, exiting!\n");
                done = 1;
            }
        }
    }

    fclose(fout);
    unlink(tmpfile);

    return 0;
}
