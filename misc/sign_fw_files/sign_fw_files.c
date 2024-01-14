/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Filename: sign_fw_files.c                                                  */
/* Purpose: signs the firmware files                                          */
/*                                                                            */
/* Copyright (C) 2021 DENSO International America, Inc.  All Rights Reserved. */
/*                                                                            */
/* Author(s)/Change History:                                                  */
/*     20220117 SB initial revision                                           */
/*                                                                            */
/*----------------------------------------------------------------------------*/
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>

#define NOR_PARTS 5
#define CMD_SIZE 100
#define KEY_DIR "../sign_fw_files/"
#define DIR_SIZE 100
#define FNAME_SIZE 100


static char *nornames[] = {
    "uboot",
    "dtb",
    "kernel",
    "rootfs",
    "sea.l"     /* special file */
};



/*----------------------------------------------------------------------------*/
/* Explanation: Signs the necessary firmware files using openssl              */
/*----------------------------------------------------------------------------*/

int main(int argc, char *argv[])
{
    signed char c;
    int i; 
    char cmd[CMD_SIZE];
    char privatekey[FNAME_SIZE];
    char fwfile[FNAME_SIZE];


    /* Verify the private key file exists */
    snprintf(privatekey, FNAME_SIZE, "%sprivatekey.pem",KEY_DIR);
    if (access(privatekey, F_OK)) {
        printf("ERROR: Private Key %s does not exist. \n", privatekey);
        return -1;
    }

    /* Sign each firmware file */
    for (i=0; i<NOR_PARTS; i++) {
        snprintf(fwfile, FNAME_SIZE, "%s", nornames[i]);
        
        /* Verify the firmware file exists */
        if (!access(fwfile, F_OK)) {

            /* Sign it */
            snprintf(cmd, CMD_SIZE, "openssl dgst -sha1 -sign %s -out %s.sign %s\n", privatekey, fwfile, fwfile);
            if (system(cmd)){
                printf("ERROR: unknown error creating signature for %s file. \n", nornames[i]);
                return -1;
            }
        }
    }
    
    return 1;
}

