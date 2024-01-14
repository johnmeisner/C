
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>              // for O_CREAT and friends



#define MAX_READ    1024*64       /* 64KiB */

int debug_output = 0;             /* Debug OFF by default */
/*
 * Debug print
 */
#include <stdarg.h>  // for va_list
#include <errno.h>   // for errno
#include <string.h>  // for strerror
#define DPRINT(fmt, ...)  mylog("%s: " fmt, __FUNCTION__, ##__VA_ARGS__)
#define DEBUG_LOG_FILENAME  "/tmp/upload_saver.log"
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
            fprintf(stderr,"ERROR: Upload_saver failed to append to its debug log! errno=%d=%s\n",
                errsv, strerror(errsv));
        }
        if (debug_output > 1) {
            fprintf(stderr, "%s", buffer);
        }
    }
}

/* Save stdin to the savefile */
int save_file(char *savefile)
{
    FILE * fout;
    long content_length = 0;
    long bytes_remaining;
    size_t next_read_size;
    unsigned int read_byte_count, wrote_byte_count;
    unsigned int tot_bytes_read=0, tot_bytes_wrote=0;
    char * contentlen_str_ptr;
    char read_buff[MAX_READ];

    // Get content length from environment.
    if ((contentlen_str_ptr = getenv("CONTENT_LENGTH")) != NULL) {
        content_length=atol(contentlen_str_ptr);
        DPRINT("Content Length: %u\n", content_length);
    } else {
        DPRINT("Error: CONTENT_LENGTH not available\n");
        return -1;
    }

    // Start savefile
    fout=fopen(savefile,"wb");
    if (fout == NULL) {
        DPRINT("ERROR: Unable to create savefile %s\n", savefile);
        return -1;
    }

    /* Read stdin and save to file */
    do {
        // Compute how many bytes to read
        bytes_remaining = content_length - tot_bytes_read;
        next_read_size = bytes_remaining < MAX_READ ? bytes_remaining : MAX_READ;
        // Read next chunk
        read_byte_count = fread(read_buff, 1, next_read_size, stdin);
        if (read_byte_count > 0) {
            tot_bytes_read += read_byte_count;
            DPRINT("Read %d bytes (wanted %d), %u total, bytes_remaining=%d\n",
                read_byte_count, next_read_size, tot_bytes_read, content_length - tot_bytes_read);
            wrote_byte_count = fwrite(read_buff, 1, read_byte_count, fout);
            tot_bytes_wrote += wrote_byte_count;
            DPRINT("Wrote %d bytes (want %d), %u total\n", wrote_byte_count, read_byte_count, tot_bytes_wrote);
        }
    } while (tot_bytes_read < content_length && read_byte_count > 0);

    DPRINT("Input completed. Total bytes read %u\n", tot_bytes_read);
    DPRINT("Saved %u bytes to file %s\n", tot_bytes_wrote, savefile);
    fclose(fout);

    // Check that we read the correct amount and wrote the correct amount
    if (tot_bytes_read != content_length) {
        DPRINT("Error: Read %d total bytes, but content length was %d\n", tot_bytes_read, content_length);
        return -1;
    }
    if (tot_bytes_wrote != tot_bytes_read) {
        DPRINT("Error: Read %d total bytes, but write total is %d\n", tot_bytes_read, tot_bytes_wrote);
        return -1;
    }

    return 0;
}

char * USAGE = "USAGE: upload_saver [-d] savefile";

int main(int argc, char *argv[])
{
   int c_arg;

   /* Process debug mode */
    while ((c_arg = getopt(argc, argv, "d")) != -1) {
        switch (c_arg) {
            case 'd':
                ++debug_output;
                DPRINT("Debug mode: %d\n", debug_output);
                break;

            default:
                printf("%s\n", USAGE);
                return -2;
                break;
        }
    }

    if (optind != argc - 1) {
        printf("%s\n", USAGE);
        return -3;
    }

    DPRINT("UploadSaver compiled on " __DATE__ " at " __TIME__ "\n");

    if (save_file(argv[optind]) != 0) {
        DPRINT("ERROR: Failed to save file!\n");
        return -4;
    }

    DPRINT("File Upload Save Complete\n");
    return 0; // exit
}
