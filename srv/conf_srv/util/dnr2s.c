
/*
 * Denso Nor Read to String 
 *
 * Used for printing string contained in output of denso_nor_read command:
 *  0x0000: 44 45 4e 53 4f 2d 53 44 35 39 31 32 30 30 2d 30
 *  0x0010: 30 31 31 2d 30 35 2d 31 32 32 30 2d 30 30 33 31
 *  0x0020: 00 ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff
 *  0x0030: ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff
 *
 * Follows this logic from Trac 2571:
 *   If the bytes can be converted to ASCII, the GUI should convert and display
 *   them on the advanced status page (only the bytes that can be converted).
 *   If the bytes are not programmed or are non-ASCII (0xff) the GUI should
 *   display the hex data, repeated until 64 entries listed.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>  // for isprint()

int main(int argc, char **argv)
{
    FILE * fin;
    char buff[1000];
    int  hexint[16];
    int done = 0;
    int i, rc, trash;
    int ascii_mode = 1;
    int line_count = 0;

    if (argc != 2) {
        fprintf(stderr, "Usage: hexdump2string dumpefile\n");
        exit(1);
    }

    if (strcmp(argv[1],"-") == 0) {
        fin = stdin;
    } else {
        fin = fopen(argv[1], "r");
        if (!fin) {
            fprintf(stderr, "ERROR: Unable to open input file %s\n", argv[1]);
            exit(1);
        }
    }

    while (!done && fgets(buff, sizeof(buff), fin)) {
        ++line_count;
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
        // Does line 1 at least start with one printable character?  If not, binary mode please!
        if (line_count == 1 && !isprint(hexint[0])) {
            ascii_mode = 0;
        }
        // Print the line as ascii
        if (ascii_mode && !done) {
            for (i=0; i<rc-1; i++) {
                if (hexint[i] >= 0x20 && hexint[i] < 0xff) {
                    printf("%c", hexint[i]);
                } else {
                    done = 1;
                    break;
                }
            }
        }
        // Print the line as hex
        if (!ascii_mode && !done) {
            for (i=0; i<rc-1; i++) {
                printf("%02x ", hexint[i]);
            }
        }
    }

    printf("\n");
    fclose(fin);

    return 0;
}
