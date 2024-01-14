/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Filename: GetAerolinkVersion.c                                             */
/*  Purpose: Displays the current Aerolink version number                     */
/*                                                                            */
/* Copyright (C) 2019 DENSO International America, Inc.  All Rights Reserved. */
/*                                                                            */
/* Revision History:                                                          */
/*                                                                            */
/* Date        Author        Comments                                         */
/* -------------------------------------------------------------------------- */
/* 2019-07-01  VROLLINGER    Initial revision.                                */
/*                                                                            */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/* Include Files                                                              */
/*----------------------------------------------------------------------------*/
#include <stdio.h>
#include "alsmi_api.h"

int main(int argc, char **argv)
{
    char buf[16];

    if (smiGetAerolinkVersion(buf, 16) < 0) {
        printf("smiGetAerolinkVersion() failed\n");
        return -1;
    }

    printf("%s\n", buf);
    return 0;
}

