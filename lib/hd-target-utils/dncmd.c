/*
*  Filename: dncmd.c
*  Purpose: Utility to Execute DENSO proprietary ops
*
*  Copyright (C) 2019 DENSO International America, Inc.
*
*  The purpose of this utility is to provide DENSO
*  proprietary features, such as enabling read/write
*  rwflash
*
*  Revision History:
*
*  Date        Author        Comments
* --------------------------------------------------
*  12-17-19    JJG        Initial release.
*
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>

#include "cfgmgrapi.h"

#define RWFLASH   "/rwflash/configs/"
/* the following need to be kept in sync with cfgmgr */
#define MNTPNTPERM "/mnt/qparc"
#define MNTPNTTMP  "/mnt/.1"
#define BLOBFILE   "/mnt/cflash/.ihr"
/* the following must not conflict with cfgmgr tmp blobs */
#define TMPBLOB    "/mnt/cflash/.dnhr"

/* requires root permission to execute */

int rwunlock(void)
{
    struct stat st;

    /* first check to see if necessary files present; checking blob & mntpnt */
    memset(&st, 0, sizeof(st));
    if (!stat(BLOBFILE, &st) && !stat(MNTPNTPERM, &st)) {
        /* file present; perform action */
        if (!stat(TMPBLOB, &st)) {
            printf("\nWARNING: already performed unlock; make sure to avoid contention.\n");
        } else if (!system("cp " BLOBFILE " " TMPBLOB)) {
            if (!system("umount " MNTPNTPERM)) {
                /* unlock cfgmgr; don't care about return */
                cfgUnlock();
                return system("mount -tdos " TMPBLOB " " MNTPNTPERM);
            }
        }
    }

    return -1;
}

int rwlock(int save)
{
    struct stat st;

    /* check if this is a cfgmgr platform */
    if (!system("whence dc >/dev/null")) {
        /* cfgmgr */
        if (save) {
            /* copy contents of rwflash to tmp mnt pnt if mounted */
            memset(&st, 0, sizeof(st));
            if (!stat(MNTPNTTMP, &st)) {
                if (!system("cp -rf " RWFLASH "* " MNTPNTTMP)) {
                    /* reset original mount point */
                    if (!system("umount " MNTPNTPERM)) {
                        /* intermittent action; just warn if a problem */
                        if (remove(TMPBLOB)) {
                            printf("\nWARNING: purge is needed if this action completes successfully.\n");
                        }
                        if (!system("mount -r -tdos " BLOBFILE " " MNTPNTPERM)) {
                            /* call cfgmgr to save results */
                            return cfgCommit();
                        }
                    }
                } else {
                    printf("\nERROR on copy\n");
                }
            } else {
                printf("\nWARNING: system rwflash not unlocked; unlock first to save changes.\n");
            }
        } else {
            /* ignore changes */
            cfgRevert();    /* this will unmount cfgmgr staging */
            /* now try to reset rwflash to readonly */
            if (!system("umount " MNTPNTPERM)) {
                /* intermittent action; just warn if a problem */
                if (remove(TMPBLOB) && (errno != ENOENT)) {
                    printf("\nWARNING: purge is needed if this action completes successfully.\n");
                }
                return system("mount -r -tdos " BLOBFILE " " MNTPNTPERM);
            }
        }
    } else {
        printf("\nWARNING: not a config manager enabled build; you should not use this utility.\n");
    }

    return -1;
}

void purge(void)
{
    int error = 0;

    /* delete any dncmd artifacts that may interfere with an operation */
    if (remove(TMPBLOB) && (errno != ENOENT)) {
        printf("\nERROR on purge: %s\n", strerror(errno));
    }

    if (error) {
        printf("\nFailed to purge everything; try again after correcting errors.\n");
    } else {
        printf("\nPurged all supporting items; retry any operation previously blocked.\n");
    }
}

/**
* main()
*
* Main function
*
* returns 0 (success); -1 (failure)
*
*/
int main(int argc, char *argv[])
{ 
    signed char c;

    /* check if I'm root; if not exit */
    if (system("id -un | grep \"^root$\" >> /dev/null")) {
        /* not the root user */
        return -1;
    }

    /* now check args */
    while ((c = getopt(argc, argv, "fprs")) != -1) {
        switch (c) {
            case 'f':    /* set rwflash to write */
                if (!rwunlock()) {
                    printf("\nSuccessfully unlocked rwflash.\n");
                } else {
                    printf("\nFAILED at unlocking rwflash.  Please try again.\n");
                }
                break;
            case 'p':    /* purge */
                purge();
                break;
            case 'r':    /* revert */
                if (!rwlock(0)) {    /* magic 0 says to revert */
                    printf("\nSuccessfully locked rwflash and discarded permanent changes.\n");
                } else {
                    printf("\nFAILED at locking rwflash; revert incomplete.  Please try again.\n");
                }
                break;
            case 's':    /* save */
                if (!rwlock(1)) {    /* magic 1 says to save */
                    printf("\nSuccessfully locked rwflash and saved permanent changes.  Changes take effect after reboot.\n");
                } else {
                    printf("\nFAILED at locking rwflash; save incomplete.  Please try again.\n");
                }
                break;
            default:
                break;
        }
    }

    return 0;
}

