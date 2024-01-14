/*
 *  Filename: fw_dl.c
 *  Purpose: FW download utility
 *
 *  Copyright (C) 2019 DENSO International America, Inc.
 *
 *  Revision History:
 *
 *  Date        Author        Comments
 * --------------------------------------------------
 *  11-10-19    LNGUYEN    Initial release.
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>




#define MAX_READ    1024*1024       /* 1MB */
unsigned char read_buff[MAX_READ];
#define UPLOAD_FILE "fw_upload.zip"

static int debug_output = 0;             /* Debug OFF by default */
/*
 * Debug print
 */
#define DPRINT(fmt, ...) \
        do { if (debug_output) { printf( "%s | %d | %s(): " fmt, __FILE__, \
                                __LINE__, __func__, __VA_ARGS__); } } while (0)

#define DPRINT0(fmt) \
        do { if (debug_output) { printf("%s | %d | %s(): " fmt, __FILE__, \
                                __LINE__, __func__); } } while (0)


int save_file(void)
{
    FILE *fw_img;
    unsigned int length;
    unsigned int b_read = MAX_READ;
    unsigned char *pos;
    unsigned char *data;
    unsigned int writelength = MAX_READ;
    unsigned int i;

    /* Save upload file to upload.raw */
    fw_img=fopen("../fw_upload.imm","wb+");
    //fw_img=fopen("../fw_upload.zip","wb+");
    //fw_img=fopen(UPLOAD_FILE, "wb+");

    length=atol(getenv("CONTENT_LENGTH"));
    DPRINT("Content Length: %u\n",length);

    if (length < (unsigned long)b_read){
        writelength = b_read = (unsigned int) length;
    }
    
    /* 1st read */
    if (fread(read_buff, b_read, 1, stdin)) {
        DPRINT("1st Read %d bytes \n", b_read);
        length -= b_read;

        /* skip header */
        for (pos=read_buff; pos<(read_buff+length-4); pos++) 
        {
            writelength--; 
            if ( (pos[0]==0x0D) && (pos[1]==0x0A) && (pos[2]==0x0D) && (pos[3]==0x0A) ) {
                data=pos+4;
                pos=read_buff+length+2;
                DPRINT0("Found\n");
                writelength-=3;
            }
        }

        /* Save 1st chunk to file */
        DPRINT("Write 1st chunk - %d bytes \n", writelength);
        fwrite(data, 1, writelength, fw_img);
    }

    /* Read the rest */
    while (fread(read_buff, b_read, 1, stdin)) {
        DPRINT("Read %d bytes \n", b_read);
        length -= b_read;

        /* Check for last read and process tail */
        if (length <= b_read) {
            writelength = 0;
            pos = read_buff;
            for (i=0; i< (b_read - 8); i++) {
                if ( (pos[0]==0x0D) && (pos[1]==0x0A) && (pos[2]==0x2D) && (pos[3]==0x2D) &&  
                     (pos[4]==0x2D) && (pos[5]==0x2D) && (pos[6]==0x2D) && (pos[7]==0x2D) ) {
                    b_read = writelength;
                    break;
                }
                writelength++;
                pos++;
            }
        }

        DPRINT("Write %d bytes \n", b_read);
        fwrite(read_buff, 1, b_read, fw_img);
        if (length < (unsigned long)b_read) b_read = (unsigned int) length;
    }

    DPRINT0("Close file \n");
    fclose(fw_img);
    return 0;
}

int main(int argc, char *argv[])
{
   int c_arg;
   
   /* Process debug mode */
    while ((c_arg = getopt(argc, argv, "d:")) != -1) {
        switch (c_arg) {
            case 'd':           /* debug mode: 1-on; 0-off (default) */
                debug_output = atoi(optarg);
                DPRINT("Debug mode %d\n", debug_output);
                break;

            default:
            break;
        }
    }

    //DPRINT0("Content-type: text/html\n\n<HTML><PRE>");

    save_file();

    DPRINT0("Upload Complete");

    return 0; // exit
}
