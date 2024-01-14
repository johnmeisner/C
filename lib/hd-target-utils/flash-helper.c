/*
*  Filename: flash-helper.c
*  Purpose: Utility for flash script to invoke cfgmgr to downgrade configuration
*
*  Copyright (C) 2020 DENSO International America, Inc.
*
*  The configuration downgrade is necessary for non-cfgmgr builds which the user
*  may try to install after upgrading to cfgmgr (during initial evaluation)
*
*/
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>     // for system()

#include "cfgmgr.h"

int main(int argc, char *argv[])
{
    char cmd[100] = {0};

    sprintf(cmd, "pidin -f Aa | grep %d | grep flash_nor >/dev/null 2>&1", getppid());
    if (!system(cmd)) {
        /* perform operation */
        system("dc -m " MASTERKEY " -v " CFGPREPPOSTPROC);
    }
    else {
        printf("flash-helper\n");
    }
    return 0;
}

