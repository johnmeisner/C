/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Filename: dncmdctl.c                                                       */
/* Purpose: Wrapper to allow only select commands with root access            */
/*                                                                            */
/* Copyright (C) 2021 DENSO International America, Inc.  All Rights Reserved. */
/*                                                                            */
/* Author(s)/Change History:                                                  */
/*     20211028 JJG - Initial revision                                        */
/*     20211105 JJG - Redesign; rather than list supported commands, ignore   */
/*                    commands that are undesired                             */
/*     20211117 JJG - Improvements on checking list (better code organization */
/*                    [thanks KF!] and more disallowed commands added)        */
/*                                                                            */
/*----------------------------------------------------------------------------*/
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>

/* disallowed commands must be listed here; strncmp used if the command has
   multiple derivates that begin with the same name */
static char *blockedStrncmpCmds[] = {
    "awk",
    "bash",
    "capture_pcap",
    "chgrp",
    "chown",
    "cp",
    "dd",
    "flash",
    "insmod",
    "link",
    "ln",
    "mkfifo",
    "mount",
    "mv",
    "passwd",
    "perl",
    "python",
    "rmmod",
    "scp",
    "sed",
    "ssh",
    "umount",
    "unlink",
    "vi",
};

/* disallowed commands with fixed names must be listed here; strcmp used
   when command has only one name */
static char *blockedStrcmpCmds[] = {
    "ash",
    "dnpasswd",
    "sh",
};

/* files that should not be removed with elevated privilege */
static char *restrictedFilesRM[] = {
    "passwd",
    "shadow",
};

/*----------------------------------------------------------------------------*/
/* Explanation: customers are now restricted to not accessing root; however   */
/*  the commands supported by this utility have options (maybe not all) that  */
/*  require root access; rather than give the customer the world and have     */
/*  them come and f over the system, we create this wrapper that allows only  */
/*  the supported commands to run.  To protect against someone trying to hack */
/*  (i.e. upload their own utility with the same name), this utility must     */
/*  use only the commands from the path location.                             */
/*----------------------------------------------------------------------------*/
int main(int argc, char *argv[])
{
    int i, j;

    if (argc < 2) {
        /* nothing to do */
        return 0;
    }

    /* check disallowed commands on strncmp list */
    for (i=0; i<sizeof(blockedStrncmpCmds)/sizeof(char *); i++) {
        if (!strncmp(argv[1], blockedStrncmpCmds[i], strlen(blockedStrncmpCmds[i]))) {
            printf("Error: command not allowed (%s)\n", argv[1]);
            return 0;
        }
    }

    /* check disallowed commands on strcmp list */
    for (i=0; i<sizeof(blockedStrcmpCmds)/sizeof(char *); i++) {
        if (!strcmp(argv[1], blockedStrcmpCmds[i])) {
            printf("Error: command not allowed (%s)\n", argv[1]);
            return 0;
        }
    }

    /* do not allow local utils; do not allow fully qualified names either; only
       support is for items in the path */
    if (strchr(argv[1], '/')) {
        printf("Error: command only supports items in environment; do not specify path (%s)\n", argv[1]);
        return 0;
    }

    /* rm check */
    if (!strcmp(argv[1], "rm")) {
        for (i=0; i<sizeof(restrictedFilesRM)/sizeof(char *); i++) {
            for (j=2; j<argc; j++) {
                if (strstr(argv[j], restrictedFilesRM[i])) {
                    /* match found in part; ignore if customer has similar named file; shouldn't use
                       this utility for customer files anyway */
                    printf("Error: file not allowed for rm command\n");
                    return 0;
                }
            }
        }
    }

    /* set environment - 20211202: do not use environment path; it can be changed by user; set hard coded path */
    if (putenv( "PATH=/usr/local/bin:/usr/bin:/bin:/usr/src/wnc:/usr/src/wnc/scripts:/usr/src/wnc/diagnostic"
                    ":/usr/src/wnc/dvt:/usr/local/sbin:/usr/sbin:/sbin:/usr/local/dnutils")) {
        printf("Error: %s\n", strerror(errno));
    }

    /* getting this far means something is about to run */
    if (setuid(0)) {
        printf("Error: %s\n", strerror(errno));
    }
    if (execvp(argv[1], &argv[1])) {
        printf("Error: (%s) %s\n", argv[1], strerror(errno));
    }
    return 0;
}

