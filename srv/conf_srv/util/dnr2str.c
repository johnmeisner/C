
/*
 * Denso Nor Read to String 
 *
 * Used for stringify-ing output of the denso_nor_read command:
 *  0x0000: 44 45 4e 53 4f 2d 53 44 35 39 31 32 30 30 2d 30
 *  0x0010: 30 31 31 2d 30 35 2d 31 32 32 30 2d 30 30 33 31
 *  0x0020: 00 ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff
 *  0x0030: ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff
 *
 * The 64 bytes of NOR that denso_nor_read outputs is split into two halves,
 *    the first half for the DENSO device id, the second for the customer's.
 *
 * INPUT ASSUMPTIONS:
 *   Assumes input is always in the format above; a hex address followed by 16 hex bytes
 *   Assumes strings of length 32 bytes are not null-terminated; assumes shorter ones are.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>  // for isprint()

char * USAGE = "Usage: dnr2str input-lines-to-skip -|inputfile";

int main(int argc, char **argv)
{
    FILE * fin;
    char buff[1000];
    int hexint[16];
    int done = 0;
    int i, rc, trash;
    int line_count = 0;
    int input_lines_to_skip;

    if (argc != 3) {
        fprintf(stderr, "%s\n", USAGE);
        exit(1);
    }

    if ((sscanf(argv[1], "%d", &input_lines_to_skip) != 1) || input_lines_to_skip < 0) {
        printf("Error: Param lines_to_skip '%s' is invalid\n%s\n", argv[1], USAGE);
        exit(2);
    }

    if (strcmp(argv[2],"-") == 0) {
        fin = stdin;
    } else {
        fin = fopen(argv[2], "r");
        if (!fin) {
            fprintf(stderr, "Error: Unable to open input file %s\n%s\n", argv[2], USAGE);
            exit(3);
        }
    }

    while (!done && fgets(buff, sizeof(buff), fin)) {
        ++line_count;
        if (line_count > input_lines_to_skip) {
            rc = sscanf(buff, "0x%x: %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x",
                &trash,
                &hexint[0], 
                &hexint[1], 
                &hexint[2], 
                &hexint[3], 
                &hexint[4], 
                &hexint[5], 
                &hexint[6], 
                &hexint[7], 
                &hexint[8], 
                &hexint[9], 
                &hexint[10], 
                &hexint[11], 
                &hexint[12], 
                &hexint[13], 
                &hexint[14], 
                &hexint[15]);
            if (rc != 17) {
                done = 1;
                break;
            }
            // Print the line as string
            if (!done) {
                for (i=0; i<rc-1; i++) {
                    if (isprint(hexint[i])) {
                        printf("%c", hexint[i]);
                    } else {
                        done = 1;
                        break;
                    }
                }
            }
            // If we hit max 32 bytes (two printed lines), be done
            if (!done && (line_count - input_lines_to_skip) >= 2) {
                done = 1;
            }
        }
    }

    printf("\n");
    fclose(fin);

    return 0;
}
