/*
 * Takes a CSV file and writes it into half of the nor reserved for device-ids
 *
 * The 64 bytes of NOR is split into two halves, the first half for the DENSO
 *    device id, the second for the customer's.
 *
 * Example Input:
 *    44, 45, 4E, 53, 4F, 2D, 53, 44, 35, 39, 31, 32, 30, 30, 2D, 30, 30, 31, 31, 2D, 30, 35, 2D, 31, 32, 32, 30, 2D, 30, 30, 31, 38,
 *
 * INPUT ASSUMPTIONS:
 *   Input is two-character hex values, separated by commas (semi-colon separated is also ok)
 *   Multi-line input is OK, but last values need a trailing comma
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>      // for unlink()

char * USAGE = "Usage: write_cust_devid inputfile_hex.csv";

#define TMPFILE1 "/tmp/ztmp.wcd.1.txt"
#define TMPFILE2 "/tmp/ztmp.wcd.2.txt"

int main(int argc, char **argv)
{
    FILE * fin, * fout;
    char buffer[1000];
    int i, rc, input_value_count;

    if (argc != 2) {
        printf("%s\n", USAGE);
        exit(1);
    }

    // Check that the user's file exists
    fin = fopen(argv[1], "r");
    if (!fin) {
        printf("ERROR: Unable to open input file %s\n%s\n", argv[1], USAGE);
        exit(3);
    }
    fclose(fin);

    // Check that a reasonable number of values are in there
    // NOTE: We turn the commas/semicolons into spaces to be able to count
    // words in the case there's no spaces after the commas/semicolons, 
    sprintf(buffer, "sed -e 's/[,;]/ /g' %s | wc -w > %s", argv[1], TMPFILE1);
    system(buffer);
    fin = fopen(TMPFILE1, "r");
    fgets(buffer, sizeof(buffer), fin);
    input_value_count = atoi(buffer);
    fclose(fin);
    if (input_value_count < 3) {
        printf("ERROR: input has only %d values, must have at least 3 hex values", input_value_count);
        exit(4);
    }
    if (input_value_count > 32) {
        printf("ERROR: input file has %d values, but can have at most 32 values\n", input_value_count);
        exit(5);
    }

    // Remove any existing TMPFILE1 for WRITES_DESNO_DEVID
    unlink(TMPFILE2);

#ifdef WRITES_CUST_DEVID
    // Save the existing denso id as hex with semicolons
    system("/usr/local/dnutils/denso_nor_read | head -2 | sed -e 's/.*: //' -e 's/ /; /g' -e 's/$/;/' > " TMPFILE2);
#endif

    // Append user's file to our tmpfile
    // We must santize when process user's input file -- extremely important
    // that the last value has a trailing semicolon or else when we write the
    // denso device id, it'll shift the customer device id by one byte.
    // STEPS EXPLAINED:
    //    * First sed
    //          - Convert commas to semicolons as that's what denso_nor_write wants
    //          - Add a trailing semicolon to all lines
    //    * TR - joins all the lines together
    //    * Second sed
    //          - Remove spacing so I can detect redundant semicolons added by first sed
    //          - Remove any redundant semicolons
    sprintf(buffer, "sed -e 's/,/;/g' -e 's/$/;/' %s | tr \\n ' ' | sed -e 's/ //g' -e 's/;;*/;/g' >> %s", argv[1], TMPFILE2);
    rc = system(buffer);
    // Abort if we had a failure
    if (rc != 0) {
        printf("ERROR: Unknown error creating update file - aborting.\n");
        exit(-1);
    }

#ifdef WRITES_DENSO_DEVID
    // Pad the number of data to 32
    fout = fopen(TMPFILE2, "a");
    for (i=input_value_count; i<32; i++) {
        fprintf(fout, "ff;\n");
    }
    fclose(fout);
        
    // Append the existing customer device id as hex with semicolons
    system("/usr/local/dnutils/denso_nor_read | tail -2 | sed -e 's/.*: //' -e 's/ /; /g' -e 's/$/;/' >> " TMPFILE2);
#endif

    // Unlock the nor
    system("/usr/local/dnutils/denso_nor_unlock");
    // Write it
    rc = system("/usr/local/dnutils/denso_nor_write " TMPFILE2);
    if (rc == 0) {
        printf("Done.  Write successful!\n");
    } else {
        printf("ERROR: NOR write failure: error code %d returned\n", rc);
    }
    // Clean up tmp files
    unlink(TMPFILE1);
    unlink(TMPFILE2);
    // Return
    return rc;
}
