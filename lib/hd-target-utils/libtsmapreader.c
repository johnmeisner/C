/*
*  Filename: libtsmapreader.c
*  Purpose: TSMC Traffic Signal Map Reader
*
*  Copyright (C) 2021 DENSO International America, Inc.
*
*/

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>        // for index()
#include <string.h>         // for strcpy
#include <ctype.h>          // for tolower()
#include "libtsmapreader.h" // for TSMAP_XXXX constants


static FILE * fin = NULL;
/* OPEN
 * returns true on success, zero on failure
 */
int tsmr_open(char * filename)
{
    fin = fopen(filename, "r");
    return fin != NULL;
}

/* READ
 * Reads the next valid line of data
 * Returns 1 on success, 0 on end-of-file, or -1 on error
 */
int tsmr_readnext(char * v_or_o, int * src_int, int * dest_int, char * r_or_e)
{
    char buffer[1000];
    char tmpstr1[100], tmpstr2[100];
    char *comment_start;
    int rc;

    while (!feof(fin)) {
        if (fgets(buffer, sizeof(buffer), fin) != NULL) {
            // Truncate out comments
            if ((comment_start = index(buffer, '#')) != NULL) {
                *comment_start = '\0';
            }
            // See we we can parse this line
            rc = sscanf(buffer,"%[^,],%d,%d,%s", tmpstr1, src_int, dest_int, tmpstr2);
            if (rc != 4) {
                // Old three-column format, we will fill in 4th column's default value
                rc = sscanf(buffer,"%[^,],%d,%d", tmpstr1, src_int, dest_int);
                if (rc == 3) {
                    strcpy(tmpstr2, "protECTED");  // weird so future debugger can tell it was auto-filled
                    ++rc;
                }
            }
            if (rc == 4) {
                // Good sscanf, pull first char from tmpstr1 and convert to lower case if needed
                *v_or_o = tolower(tmpstr1[0]);
                // Pull second char from tmpstr2. This gives 'r' for Protected and 'e' for Permitted
                *r_or_e = tolower(tmpstr2[1]);  
                /* Now do a little data validation */
                if (*v_or_o == TSMAP_VEHICLE || *v_or_o == TSMAP_OVERLAP) {
                    if (*src_int >= 1 && *src_int <= 16) {
                        if (*dest_int >= 0 && *dest_int <= 255) {
                            if (*r_or_e == TSMAP_PROTECTED || *r_or_e == TSMAP_PERMITTED) {
                                /* All Good !! */
//                              printf("DEBUG: Accepting values %c,%d,%d,%c from line: %s\n", *v_or_o, *src_int, *dest_int, *r_or_e, buffer);
                                return 1;
                            }
                        }
                    }
                }
            }
        }
    }

    return 0;
}

/* CLOSE
 * Closes the tsmr_read file descriptor if it is open
 */
void tsmr_close()
{
    if (fin != NULL) {
        fclose(fin);
    }
    fin = NULL;
}
