/*
 *  Filename: libcgipost.h
 *  Purpose: Library of utilities to handle CGI POST content
 *
 *  Copyright (C) 2020 DENSO International America, Inc.
 *
 *  Revision History:
 *
 *  Date        Author     Comments
 * --------------------------------------------------
 *  2020-11-17  KFRANKEL   Initial release.
 *
 */


#include <stdio.h>
#include <stdlib.h>

int process_post_string(char * line, int num_fields_wanted, char ** wanted_fields, char ** value_dest, size_t * max_value_sizes);

int receive_and_process_post(FILE * instream, int num_fields_wanted, char ** wanted_fields, char ** value_dest, size_t * max_value_sizes);

void uri_decode_string(char * uristr);
