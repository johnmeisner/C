/*
 *  Filename: file_val.c
 *  Purpose: File validate utility
 *
 *  Copyright (C) 2020 DENSO International America, Inc.
 *
 *  Revision History:
 *
 *  Date        Author        Comments
 * --------------------------------------------------
 *  02-14-20    LNGUYEN    Initial release.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int main (int argc, char **argv) {
    char *file_name;
    long offset;
    long file_size;
    char *val_string;
    char *fw_ver;
    char line[1000];
    int iRet = 0;
    
    FILE *fw_img = NULL;
    
    /* Four params
     *  - Filename
     *  - Offset
     *  - Validate string
     */
    
    if (argc < 4){
        printf("Error!\n");
        return 1;
    }
    
    file_name = argv[1];
    offset = strtol(argv[2], NULL, 16);
    val_string = argv[3];
    
    /* File check */
    fw_img=fopen(file_name,"r");
    
    /* check file size */
    
    if (fw_img == NULL){
        printf("Can't open file: %s\n", file_name);
        iRet = -1;
    } else {
        fseek(fw_img, 0L, SEEK_END);
        file_size = ftell(fw_img);
        //printf("%s Size: %ld\n", file_name, file_size);
        
        /* Check for proper format
         * - Move to offset
         * - Read 1st line and validate against val_string
         * - If passed, read second line for version number
         */
        if (offset < file_size) {
            /* Read 1st line */
            fseek(fw_img, offset, SEEK_SET);
            fscanf(fw_img, "%[^\n]", line);
            //printf("1st line:%s\n", line);
            if (strcmp(val_string, line) == 0) {
                //printf("File %s is Valid! \n", file_name);
                /* File Valid 
                 * read second line 
                 */
                fseek(fw_img, 1, SEEK_CUR);
                fscanf(fw_img, "%[^\n]", line);
                printf("%s\n", line);       // Second line is FW version
#if 0
                // Verify against validate string
                if (strcmp(fw_ver, line) == 0) {
                    printf("Ver Matched! - %s \n", fw_ver);
                } else {
                    printf("Ver Not Matched! \n CUR %s \n NEW %s \n", fw_ver, line);
                }
#endif
            } else {
                //printf("File %s is NOT valid! - % s\n", file_name, line );
                iRet = -1;

            }

        }

    }
    // extract inf from user  - who and why:?

file_val_exit:

    /* Close file */
    if (fw_img){
        fclose(fw_img);
    }

    return iRet;
}