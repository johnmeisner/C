/*
*  Filename: test_tsmapreader.c
*  Purpose: Test out TSMC Traffic Signal Map Reader
*
*  Copyright (C) 2021 DENSO International America, Inc.
*
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>      // for strcpy
#include <ctype.h>       // for tolower
#include "libtsmapreader.h"

#define TESTFILE "/tmp/tsmap_test.txt"
#define NUM_TEST_LINES 10


#define NUM_BAD_LINES 7
void print_bad_line3(FILE * fout, int bad_index)
{
    switch(bad_index % NUM_BAD_LINES) {
        case 0:
            fprintf(fout, "%c,1,1\n", 'a' + (rand() % 13)); /* bad random letter, max 'n' */
            break;
        case 1:
            fprintf(fout, "yohellothere,2,3,4\n");    /* phase type doesnt start with v or o */
            break;
        case 2:
            fprintf(fout, "v,2,\n");    /* Short line 1 */
            break;
        case 3:
            fprintf(fout, "v,,2\n");    /* Short line 2 */
            break;
        case 4:
            fprintf(fout, "v,2,    #7 in a comment\n");    /* Short line 3 */
            break;
        case 5:
            fprintf(fout, "v,2,%d\n", -bad_index);    /* Invalid value */
            break;
        case 6:
            fprintf(fout, "v,%d,2\n", -bad_index);    /* Invalid value */
            break;
    }
}
    
    
#define NUM_BAD_4COL_LINES 8
void print_bad_line4(FILE * fout, int bad_index)
{
    switch(bad_index % NUM_BAD_4COL_LINES) {
        case 0:
            fprintf(fout, "vehicle,1,1,p\n"); /* Short 4th value*/
            break;
        case 1:
            fprintf(fout, "Vehicle,1,1,P%cotected\n", 'f' + (rand() % 12)); /* random bad letter, f-q */
            break;
        case 2:
            fprintf(fout, ",1,2,protected\n");    /* Short line 1 */
            break;
        case 3:
            fprintf(fout, "vehicle,,2,permitted\n");    /* Short line 2 */
            break;
        case 4:
            fprintf(fout, "v,2,    #7,protected - in a comment\n");    /* Short line 3 */
            break;
        case 5:
            fprintf(fout, "v,2    #,7,protected - in a comment\n");    /* Short line 4 */
            break;
        case 6:
            fprintf(fout, "v,2,%d,perm\n", -bad_index);    /* Invalid value */
            break;
        case 7:
            fprintf(fout, "v,%d,2,pro\n", -bad_index);    /* Invalid value */
            break;
    }
}
    
    


/* TEST1: Good [vo],n,n lines mixed with bad lines and comments */
/* Returns 0 for success, non-zero for errors */
int test1()
{
    FILE * fout;
    char test_vo[NUM_TEST_LINES];
    int test_src_int[NUM_TEST_LINES];
    int test_dest_int[NUM_TEST_LINES];
    char check_vo, check_er;
    int check_src_int, check_dest_int;
    int i, num_lines;
    int num_bad_lines = 0;
    int num_errors = 0;

    for (i=0; i<NUM_TEST_LINES; i++) {
        test_vo[i] = (rand() % 100 >= 50) ? 'v' : 'o';
        test_src_int[i] = rand() % 16 + 1;
        test_dest_int[i] = rand() % 256;
    }

    fout = fopen(TESTFILE, "w");
    fprintf(fout, "# This is a test of the tsmap reader\n");
    fprintf(fout, "#\n");
    fprintf(fout, "# Hope it works on this config file!\n\n");
    fprintf(fout, "#\n");

    for (num_lines=i=0; i<NUM_TEST_LINES; ) {
        ++num_lines;
        // Write a comment, a data, a blank line, or a line with bad data
        switch ((rand() / 1000) % 4) {
            case 0: /* Comment Line */
                fprintf(fout, "# This is line # %d and %d is the number of lines so far\n", num_lines, num_lines);
                break;
            case 1: /* Data line */
                fprintf(fout, "%c,%d,%d\n", test_vo[i], test_src_int[i], test_dest_int[i]);
                ++i;
                break;
            case 2: /* Blank line */
                fprintf(fout,"\n");
                break;
            case 3: /* Bad Line */
                print_bad_line3(fout, num_bad_lines++);
                break;
        }
    }

    /* Add any remaining bad lines */
    while (num_bad_lines < NUM_BAD_LINES) {
        print_bad_line3(fout, num_bad_lines++);
    }

    /* Close up */
    fclose(fout);

    /* Read and compare */
    if (! tsmr_open(TESTFILE)) {
        printf("FAIL: Test1 Couldn't open TESTFILE %s\n", TESTFILE);
        return 1;
    }
    for (i=0; i<NUM_TEST_LINES; i++) {
        if (tsmr_readnext(&check_vo, &check_src_int, &check_dest_int, &check_er) <= 0) {
            printf("FAIL: Test1 read failure on data line # %d\n", i+1);
            ++num_errors;
        } else {
            if (check_vo != test_vo[i]) {
                ++num_errors;
                printf("FAIL: Test1 line # %d vo check failed, got %c wanted %c\n",
                    i+1, check_vo, test_vo[i]);
            }
            if (check_src_int != test_src_int[i]) {
                ++num_errors;
                printf("FAIL: Test1 line # %d src_int check failed, got %d wanted %d\n",
                    i+1, check_src_int, test_src_int[i]);
            }
            if (check_dest_int != test_dest_int[i]) {
                ++num_errors;
                printf("FAIL: Test1 line # %d dest_int check failed, got %d wanted %d\n",
                    i+1, check_dest_int, test_dest_int[i]);
            }
            if (check_er != TSMAP_PROTECTED) {
                ++num_errors;
                printf("FAIL: Test1 line # %d er check failed, got %c wanted %c\n",
                    i+1, check_vo, TSMAP_PROTECTED);
            }
        }
    }

    /* See if we can read an extra data line */
    if (tsmr_readnext(&check_vo, &check_src_int, &check_dest_int, &check_er) > 0) {
        ++num_errors;
        printf("FAIL: Test1 extra line of data read!  Should only get %d lines!\n", NUM_TEST_LINES);
    }

    tsmr_close();

    return num_errors;
}


/* Returns 0 for success, non-zero for errors */
/* TEST2: Full words allowed in the vehicle/overlap column */
int test2()
{
    FILE * fout;
    char test_vo[NUM_TEST_LINES][10];
    int test_src_int[NUM_TEST_LINES];
    int test_dest_int[NUM_TEST_LINES];
    char check_vo, check_er;
    int check_src_int, check_dest_int;
    int i, num_lines;
    int num_bad_lines = 0;
    int num_errors = 0;

    for (i=0; i<NUM_TEST_LINES; i++) {
        strcpy(test_vo[i], rand() % 100 >= 50 ? (rand() % 100 >= 50 ? "vehicle" : "overlap")
                                              : (rand() % 100 >= 50 ? "Vehicle" : "Overlap"));
        test_src_int[i] = rand() % 16 + 1;
        test_dest_int[i] = rand() % 256;
    }

    fout = fopen(TESTFILE, "w");
    fprintf(fout, "# This is a test of the tsmap reader\n");
    fprintf(fout, "#\n");
    fprintf(fout, "# Hope it works on this config file!\n\n");
    fprintf(fout, "#\n");

    for (num_lines=i=0; i<NUM_TEST_LINES; ) {
        ++num_lines;
        // Write a comment, a data, a blank line, or a line with bad data
        switch ((rand() / 1000) % 4) {
            case 0: /* Comment Line */
                fprintf(fout, "# This is line # %d and %d is the number of lines so far\n", num_lines, num_lines);
                break;
            case 1: /* Data line */
                fprintf(fout, "%s,%d,%d\n", test_vo[i], test_src_int[i], test_dest_int[i]);
                ++i;
                break;
            case 2: /* Blank line */
                fprintf(fout,"\n");
                break;
            case 3: /* Bad Line */
                print_bad_line3(fout, num_bad_lines++);
                break;
        }
    }

    /* Add any remaining bad lines */
    while (num_bad_lines < NUM_BAD_LINES) {
        print_bad_line3(fout, num_bad_lines++);
    }

    /* Close up */
    fclose(fout);

    /* Read and compare */
    if (! tsmr_open(TESTFILE)) {
        printf("FAIL: Test2 Couldn't open TESTFILE %s\n", TESTFILE);
        return 1;
    }
    for (i=0; i<NUM_TEST_LINES; i++) {
        if (tsmr_readnext(&check_vo, &check_src_int, &check_dest_int, &check_er) <= 0) {
            printf("FAIL: Test2 read failure on data line # %d\n", i+1);
            ++num_errors;
        } else {
            if (check_vo != tolower(test_vo[i][0])) {
                ++num_errors;
                printf("FAIL: Test2 line # %d vo check failed, got %c wanted %c, orig vo str is %s\n",
                    i+1, check_vo, tolower(test_vo[i][0]), test_vo[i]);
            }
            if (check_src_int != test_src_int[i]) {
                ++num_errors;
                printf("FAIL: Test2 line # %d src_int check failed, got %d wanted %d\n",
                    i+1, check_src_int, test_src_int[i]);
            }
            if (check_dest_int != test_dest_int[i]) {
                ++num_errors;
                printf("FAIL: Test2 line # %d dest_int check failed, got %d wanted %d\n",
                    i+1, check_dest_int, test_dest_int[i]);
            }
            if (check_er != TSMAP_PROTECTED) {
                ++num_errors;
                printf("FAIL: Test2 line # %d er check failed, got %c wanted %c\n",
                    i+1, check_vo, TSMAP_PROTECTED);
            }
        }
    }

    /* See if we can read an extra data line */
    if (tsmr_readnext(&check_vo, &check_src_int, &check_dest_int, &check_er) > 0) {
        ++num_errors;
        printf("FAIL: Test2 extra line of data read!  Should only get %d lines!\n", NUM_TEST_LINES);
    }

    tsmr_close();

    return num_errors;
}



/* Returns 0 for success, non-zero for errors */
/* TEST3: Four-column data */
int test3(int flag_include_3column_bad_data)
{
    FILE * fout;
    char test_vo[NUM_TEST_LINES][10];
    int test_src_int[NUM_TEST_LINES];
    int test_dest_int[NUM_TEST_LINES];
    char test_er[NUM_TEST_LINES][20];
    char check_vo, check_er;
    int check_src_int, check_dest_int;
    int i, num_lines;
    int num_bad_lines = 0;
    int num_errors = 0;

    // Populate test values
    for (i=0; i<NUM_TEST_LINES; i++) {
        strcpy(test_vo[i], rand() % 100 >= 50 ? (rand() % 100 >= 50 ? "vehicle" : "overlap")
                                              : (rand() % 100 >= 50 ? "Vehicle" : "Overlap"));
        test_src_int[i] = rand() % 16 + 1;
        test_dest_int[i] = rand() % 256;
        strcpy(test_er[i], rand() % 100 >= 50 ? (rand() % 100 >= 50 ? "protected" : "permitted")
                                              : (rand() % 100 >= 50 ? "Protected" : "Permitted"));
    }

    fout = fopen(TESTFILE, "w");
    fprintf(fout, "# This is a test of the tsmap reader\n");
    fprintf(fout, "#\n");
    fprintf(fout, "# Hope it works on this config file!\n\n");
    fprintf(fout, "#\n");

    for (num_lines=i=0; i<NUM_TEST_LINES; ) {
        ++num_lines;
        // Write a comment, a data, a blank line, or a line with bad data
        switch ((rand() / 1000) % 4) {
            case 0: /* Comment Line */
                fprintf(fout, "# This is line # %d and %d is the number of lines so far\n", num_lines, num_lines);
                break;
            case 1: /* Data line */
                fprintf(fout, "%s,%d,%d,%s\n", test_vo[i], test_src_int[i], test_dest_int[i], test_er[i]);
                ++i;
                break;
            case 2: /* Blank line */
                fprintf(fout,"\n");
                break;
            case 3: /* Bad Line */
                print_bad_line4(fout, num_bad_lines);
                if (flag_include_3column_bad_data) {
                    print_bad_line3(fout, num_bad_lines);
                }
                ++num_bad_lines;
                break;
        }
    }

    /* Add any remaining bad lines */
    while (num_bad_lines < NUM_BAD_4COL_LINES) {
        print_bad_line4(fout, num_bad_lines);
        if (flag_include_3column_bad_data) {
            print_bad_line3(fout, num_bad_lines);
        }
        ++num_bad_lines;
    }

    /* Close up */
    fclose(fout);

    /* Read and compare */
    if (! tsmr_open(TESTFILE)) {
        printf("FAIL: Test3 Couldn't open TESTFILE %s\n", TESTFILE);
        return 1;
    }
    for (i=0; i<NUM_TEST_LINES; i++) {
        if (tsmr_readnext(&check_vo, &check_src_int, &check_dest_int, &check_er) <= 0) {
            printf("FAIL: Test3 read failure on data line # %d\n", i+1);
            ++num_errors;
        } else {
            if (check_vo != tolower(test_vo[i][0])) {
                ++num_errors;
                printf("FAIL: Test3 line # %d vo check failed, got %c wanted %c, orig vo str is %s\n",
                    i+1, check_vo, tolower(test_vo[i][0]), test_vo[i]);
            }
            if (check_src_int != test_src_int[i]) {
                ++num_errors;
                printf("FAIL: Test3 line # %d src_int check failed, got %d wanted %d\n",
                    i+1, check_src_int, test_src_int[i]);
            }
            if (check_dest_int != test_dest_int[i]) {
                ++num_errors;
                printf("FAIL: Test3 line # %d dest_int check failed, got %d wanted %d\n",
                    i+1, check_dest_int, test_dest_int[i]);
            }
            if (check_er != tolower(test_er[i][1])) {
                ++num_errors;
                printf("FAIL: Test3 line # %d er check failed, got %c wanted %c, orig er str is %s\n",
                    i+1, check_er, tolower(test_er[i][1]), test_er[i]);
            }
        }
    }

    /* See if we can read an extra data line */
    if (tsmr_readnext(&check_vo, &check_src_int, &check_dest_int, &check_er) > 0) {
        ++num_errors;
        printf("FAIL: Test3 extra line of data read!  Should only get %d lines!\n", NUM_TEST_LINES);
    }

    tsmr_close();

    return num_errors;
}


#define NUM_TEST_REPEATS 500
int main()
{
    int i, rc;
    int num_passed=0;

    printf("test_tsmapreader built on "__DATE__" at "__TIME__"\n");

    for (i=0; i<NUM_TEST_REPEATS; i++) {
        if ((rc = test1()) != 0) {
            printf("==> Got a test1 failure with %d errors!\n", rc);
        } else {
            ++num_passed;
        }
        if ((rc = test2()) != 0) {
            printf("==> Got a test2 failure with %d errors!\n", rc);
        } else {
            ++num_passed;
        }
        if ((rc = test3(0)) != 0) {
            printf("==> Got a test3(0) failure with %d errors!\n", rc);
        } else {
            ++num_passed;
        }
        if ((rc = test3(1)) != 0) {
            printf("==> Got a test3(1) failure with %d errors!\n", rc);
        } else {
            ++num_passed;
        }
    }

    printf("%d/%d test cycles passed!\n", num_passed, 4 * NUM_TEST_REPEATS);
}
