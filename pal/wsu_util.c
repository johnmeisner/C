/**************************************************************************
 *                                                                        *
 *     File Name: wsu_util.c                                              *
 *     Author:                                                            *
 *         DENSO International America, Inc.                              *
 *         North America Research Laboratory, California Office           *
 *         3252 Business Park Drive                                       *
 *         Vista, CA 92081                                                *
 *                                                                        *
 *   Copyright (C) 2019 by DENSO LA Labs                                   *
 *                                                                        *
 **************************************************************************/
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
#include <sys/stat.h>
#include <dirent.h>

#include "dn_types.h"
#include "wsu_util.h"

/**-----------------------------------------------------------------------------
** @brief  Utility to setup signal handler
** @param  handler - signal handler function address
** @param  sig - signal ID number
** @param  flags - signal initialization flags
** @return  nothing tested, nothing returned
**----------------------------------------------------------------------------*/
void wsuUtilSetupSignalHandler(void (*handler)(int), int sig, int flags)
{
    struct sigaction sa;
    sigset_t mask;

    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handler;
    sa.sa_flags = flags;
    sigaction(sig, &sa, NULL);

    sigemptyset(&mask);
    sigaddset(&mask, sig);
    pthread_sigmask(SIG_UNBLOCK, &mask, NULL);
}


/* End of source */
