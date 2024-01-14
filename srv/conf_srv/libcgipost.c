/*
 *  Filename: libcgipost.c   
 *  Purpose: Library of utilities to handle CGI POST content
 *
 *  Copyright (C) 2020 DENSO International America, Inc.
 *
 *  Revision History:
 *
 *  Date        Author     Comments
 * --------------------------------------------------
 *  2020-11-11  KFRANKEL   Initial release.
 *
 *
 *  NOTES:
 *    Based on code from:
 *       https://stackoverflow.com/questions/59659968/how-to-get-key-values-from-http-post-request-using-c-and-boost-asio
 *          (Second example)
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>      // for isxdigit()


// Uncomment this line to add debug output to parsing
#define DEBUG_OUTPUT_FILE  "/tmp/xcgi-login-debug.txt"

#ifdef DEBUG_OUTPUT_FILE
#include <time.h>       // for time, localtime
#include <stdarg.h>     // for va_list and friends
void lib_debug_printf(char * fmt, ...)
{
    //Declare a va_list macro and initialize it with va_start
    va_list l_Arg;

    // Lead with timestamp
    FILE * fout = fopen(DEBUG_OUTPUT_FILE, "a");
    time_t xx = time(0);
    struct tm * x = localtime(&xx);
    fprintf(fout, "%d%02d%02d-%02d%02d%02d: ", 1900+x->tm_year,x->tm_mon,x->tm_mday,x->tm_hour,x->tm_min,x->tm_sec);

    va_start(l_Arg,fmt);         // Last param BEFORE the "..."
    vfprintf(fout, fmt, l_Arg);  // Use the variable list all at once
    va_end(l_Arg);               // Close the va list

    fclose(fout);
}
#endif

// Decodes-in-place an URI-encoded string
void uri_decode_string(char * uristr)
{
    char * pread, * pwrite;
    int  hexvalue;
    char c;

    for (pread = uristr, pwrite = uristr; *pread; ++pread, ++pwrite) {

        c = *pread;
        if (c == '+') c = ' ';
        else if (c == '%') {
            if (isxdigit(*(pread+1)) && isxdigit(*(pread+2))) {     // isxdigit == ishex
                sscanf(pread+1, "%2x", &hexvalue);
                c = hexvalue;
                pread += 2; // Consume the extra 2 chars
            }
        }
        *pwrite = c;
    }
    // Null terminate the string again as it shrinks with decoding
    *pwrite = '\0';
}

// Process the query string line from a POST to extract wanted fields
int process_post_string(char * line, int num_fields_wanted, char ** wanted_fields, char ** value_dest, size_t * max_value_sizes)
{
    int i;
    int fields_populated_count = 0;
    char fieldtmp[100];
    char * x;

    // Var "line" contains the param line now.  Let's parse out the param data

    // Chomp out the \r\n at the end
    while (strlen(line) > 0 && line[strlen(line)-1] < ' ') {
        line[strlen(line)-1] = 0;
    }

    // field line length
    size_t field_line_len = strlen(line);
    char * last_line_char = line + field_line_len - 1;
    size_t line_chars_available;
    size_t max_copy_len;

    // Extract the wanted fields
    for (i=0; i < num_fields_wanted; i++) {
        if (strlen(wanted_fields[i]) > sizeof(fieldtmp) - 2) {
            printf("ERROR: Unable to parse excessively long field name '%s', need programming!\n",
                wanted_fields[i]);
        } else {
            strcpy(fieldtmp, wanted_fields[i]);
            strcat(fieldtmp, "=");
            // See if we can find that field
            x = strstr(line, fieldtmp);
            if (x) {
                // Copy value
                line_chars_available = last_line_char - x;
                if (line_chars_available < max_value_sizes[i]) {
                    max_copy_len = line_chars_available;
                } else {
                    max_copy_len = max_value_sizes[i];
                }
                strncpy(value_dest[i], x + strlen(fieldtmp), max_copy_len);
                // If any '&' truncate there to chop off following params
                x = index(value_dest[i], '&');
                if (x) {
                    *x = 0;
                }
                // Decode the encoded special chars
                uri_decode_string(value_dest[i]);
                // Count it as gotted
                ++fields_populated_count;
            // Didnt find the field, mark it empty
            } else {
                value_dest[i][0] = 0;
            }
        }
    }

    return fields_populated_count;
}


int receive_and_process_post(FILE * instream, int num_fields_wanted, char ** wanted_fields, char ** value_dest, size_t * max_value_sizes)
{
    char *gets(char *s);
    char line[1000];
    int done = 0;
    int phase = 0;
    int line_count = 0;
    char * fgets_result;

#ifdef DEBUG_OUTPUT_FILE
    lib_debug_printf("f_munch_post started\n");
#endif

    // parsing the headers
    while (!done) {
#ifdef DEBUG_OUTPUT_FILE
        lib_debug_printf("Reading line %d ...\n", line_count+1);
#endif
        fgets_result = fgets(line, sizeof(line), instream);
#ifdef DEBUG_OUTPUT_FILE
        lib_debug_printf("Read line %d, s=%s\n", line_count+1, fgets_result == NULL ? "NULL" : "Not-NULL");
#endif
        if (fgets_result != NULL) {
            ++line_count;
            if (phase < 2) {
                if (strncmp(line, "POST ", 5) == 0) {
#ifdef DEBUG_OUTPUT_FILE
                    lib_debug_printf("Received initial POST line, advancing to next parsing stage\n");
#endif
                    phase = 1;
                } else if (index(line, ':') != 0) {
#ifdef DEBUG_OUTPUT_FILE
                    lib_debug_printf("Ignoring header line %s", line);
#endif
                    phase = 1;
                } else if (phase == 1 && line[0] < ' ') {    // The \r\n blank line
#ifdef DEBUG_OUTPUT_FILE
                    lib_debug_printf("Reached blank line, jumping to parsing phase 2\n");
                    lib_debug_printf("strlen(line) = %d\n", (int) strlen(line));
#endif
                    phase = 2;
                }
            } else {
                // Parse out params from POST line
#ifdef DEBUG_OUTPUT_FILE
                lib_debug_printf("POST data: %s\n", line);
#endif
                done = 1;
            }
        } else {
            printf("WARNING: Failed to complete cgi-bin processing!\n");
            done = 2;
        }
    }

    // Var "line" contains the post query-string param-line now.
    
    // Call process_post_string to parse out the param data
    return process_post_string(line, num_fields_wanted, wanted_fields, value_dest, max_value_sizes);
}
