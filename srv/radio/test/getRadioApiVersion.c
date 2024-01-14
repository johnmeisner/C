/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Filename: getRadioApiVersion.c                                             */
/*  Purpose: Test program that gets the Radio API version                     */
/*                                                                            */
/* Copyright (C) 2019 DENSO International America, Inc.  All Rights Reserved. */
/*                                                                            */
/* Revision History:                                                          */
/*                                                                            */
/* Date        Author        Comments                                         */
/* -------------------------------------------------------------------------- */
/* 2019-06-14  VROLLINGER    Initial revision.                                */
/*                                                                            */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/* Include Files                                                              */
/*----------------------------------------------------------------------------*/
#include <stdio.h>
#include "v2x_radio_api.h"

/*----------------------------------------------------------------------------*/
/* Functions                                                                  */
/*----------------------------------------------------------------------------*/
int main(int argc, char **argv)
{
    v2x_api_ver_t result;

    result =  v2x_radio_api_version();
    printf("version_num       = %u\n", result.version_num);
    printf("build_date_str    = \"%s\"\n", result.build_date_str);
    printf("build_time_str    = \"%s\"\n", result.build_time_str);
    printf("build_details_str = \"%s\"\n", result.build_details_str);

    return 0;
}

