/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Filename: dnpasswd.c                                                       */
/* Purpose: Wrapper to prepare environment before and after calling passwd    */
/*                                                                            */
/* Copyright (C) 2021 DENSO International America, Inc.  All Rights Reserved. */
/*                                                                            */
/* Author(s)/Change History:                                                  */
/*     20211118 JJG - Initial revision                                        */
/*                                                                            */
/*----------------------------------------------------------------------------*/
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>

/* the path is already 123 characters long; this allows for future growth */
#define DIRNAMESIZE  50
#define CMDSIZE     150

#define PRIMARYPASSWD  "/rwflash/etc/passwd"
#define PRIMARYSHADOW  "/rwflash/etc/shadow"
#define ETCPASSWD      "/etc/passwd"
#define ETCSHADOW      "/etc/shadow"

static char mainloop = 1;    /* no need for true or false */
static char badexit = 0;

static void termHandler(int a, siginfo_t *b, void __attribute__((unused)) *c)
{
    mainloop = 0;

    if (a == SIGCHLD && (NULL != b)) {
        if (b->si_code == CLD_EXITED) {
            badexit = (b->si_status);
        }
    }
}

/*----------------------------------------------------------------------------*/
/* Explanation: the passwd command will fail by default because the           */
/* file system is locked; rather than unlock the file system and expose       */
/* potential issues such as file corruption of critical files, this utility   */
/* will create a writable snapshot of the key directory (/etc) and then       */
/* call passwd (no root privilege); upon completion this utility will then    */
/* clean up                                                                   */
/*----------------------------------------------------------------------------*/
int main(int argc, char *argv[])
{
    char tmpdir[DIRNAMESIZE] = {0}, cmd[CMDSIZE];
    unsigned char idx;
    char doinitonly = 0, docleanuponly = 0;
    FILE *pf;
    pid_t pid = -1;
    uid_t myid;
    struct sigaction act;

    if (argc > 2) {
        /* nothing to do */
        printf("Error: you may only change your password.\n");
        return 0;
    }

    if (argc == 2) {
        pid = getppid();
        /* only rsuhealth is allowed to call this program with arguments */
        snprintf(cmd, CMDSIZE, "ps -ef | grep %d | grep rsuh >>/dev/null", (int)pid);
        if (system(cmd)) {
            printf("Error: you may only change your password.\n");
            return 0;
        }
        if (!strcmp(argv[1], "firstlogin")) {
            doinitonly = 1;
        } else if (!strcmp(argv[1], "cleanup")) {
            docleanuponly = 1;
        } else if (!strcmp(argv[1], "reset")) {
            /* unmount and remove PRIMARYPASSWD and PRIMARYSHADOW */
            system("umount " ETCPASSWD);    /* not much can be done on error */
            system("umount " ETCSHADOW);    /* not much can be done on error */
            system("rm -f " PRIMARYPASSWD);    /* not much can be done on error */
            system("rm -f " PRIMARYSHADOW);    /* not much can be done on error */

            /* terminous action */
            return 0;
        } else {
            /* rsuhealth called with something else? error */
            return 0;
        }
    } else {
        /* check if root user; do not allow root password change */
        if (!system("whoami | grep root >>/dev/null")) {
            printf("Error: attempting to change root password.\n");
            return 0;
        }
    }

    if (!docleanuponly) {
        /* prepare for execution */
        if (!system("ls /tmp | grep dnpw")) {
            /* there is already a tmp folder in existence; don't proceed further */
            return 0;  /* clearly if a person plops any similar named file, this will stop as well;
                          but this is an acceptable risk since this is called during boot up */
        }
        if (NULL == (pf = popen("mktemp -d /tmp/dnpwXXXXXX", "r"))) {
            printf("Error: internal error; please retry.\n");
            return 0;
        }
        fgets(tmpdir, sizeof(char)*DIRNAMESIZE, pf);    /* this will be the temp directory name */
        pclose(pf);
        /* eliminate any '\n' */
        idx = strlen(tmpdir);
        if (idx && (tmpdir[--idx] == '\n')) {
            tmpdir[idx] = 0;
        }

        /* double check no error was returned */
        if (!strstr(tmpdir, "tmp")) {
            /* the directory wasn't created since no 'tmp' in output */
            printf("Error: system error; please retry.\n");
            return 0;
        }

        /* needs root permission */
        myid = getuid();
        if (setuid(0)) {
            printf("Error: %s\n", strerror(errno));
            snprintf(cmd, CMDSIZE, "rm -rf %s", tmpdir);
            system(cmd);
            return 0;
        }
        snprintf(cmd, CMDSIZE, "cp -r /etc %s", tmpdir);
        if (system(cmd)) {
            snprintf(cmd, CMDSIZE, "rm -rf %s", tmpdir);
            system(cmd);
            printf("Error: setup error; please retry.\n");
            return 0;
        }
        /* these must exist */
        snprintf(cmd, CMDSIZE, "cp " PRIMARYPASSWD " %s/etc", tmpdir);
        if (system(cmd)) {
            snprintf(cmd, CMDSIZE, "rm -rf %s", tmpdir);
            system(cmd);
            printf("Error: required file missing...reboot system before retrying.\n");
            return 0;
        }
        snprintf(cmd, CMDSIZE, "cp " PRIMARYSHADOW " %s/etc", tmpdir);
        if (system(cmd)) {
            snprintf(cmd, CMDSIZE, "rm -rf %s", tmpdir);
            system(cmd);
            printf("Error: required file missing...reboot system before retrying.\n");
            return 0;
        }

        snprintf(cmd, CMDSIZE, "mount -o bind %s/etc /etc", tmpdir);
        if (system(cmd)) {
            snprintf(cmd, CMDSIZE, "rm -rf %s", tmpdir);
            system(cmd);
            printf("Error: permissions error; please retry.\n");
            /* no need to change uid back */
            return 0;
        }

        if (doinitonly) {
            return 0;    /* done; no need to print */
        }

        /* do not set up handler until AFTER system calls */
        memset(&act, 0, sizeof(act));
        act.sa_sigaction = termHandler;
        sigemptyset(&act.sa_mask);
        act.sa_flags = SA_SIGINFO|SA_RESTART;
        sigaction(SIGCHLD, &act, NULL);
        sigaction(SIGTERM, &act, NULL);
        sigaction(SIGKILL, &act, NULL);

        /* setup complete, now call passwd */

        if ((pid = fork()) == -1) {
            /* fork failed */
            printf("Error: internal resource error; consider rebooting\n");
        } else if (pid == 0) {
            /* passwd thread */
            setuid(myid);    /* reset id so password changes correct user */
            if (execl("/usr/bin/passwd", "passwd", (const char *)0)) {
                printf("Error: unable to execute passwd (%s)\n", strerror(errno));
            }
            return 0;
        } else {
            /* parent must wait until passwd complete */
            while (mainloop) {
                sleep(1);
            }
        }
    } else {    /* docleanuponly */
        /* needs root permission; need to set it */
        myid = getuid();
        if (setuid(0)) {
            /* nothing to print; called by rsuhealth */
            return 0;    /* we failed */
        }
    }

    /* clean up - copy file (requires root); hence we didn't reset it earlier */
    if (docleanuponly) {
        /* we don't know the tmpdir; so just assume whatever /tmp/dnpw is correct; (could be a bad assumption) */
        snprintf(cmd, CMDSIZE, "cp -f /tmp/dnpw*/etc/shadow " PRIMARYSHADOW);
    } else {
        if (badexit) {
            /* do not save file if the passwd child thread had a bad exit; just perform normal cleanup */
            snprintf(cmd, CMDSIZE, "echo no action >>/dev/null");    /* do something for system cmd [that won't fail!] */
        } else {
            snprintf(cmd, CMDSIZE, "cp -f %s/etc/shadow " PRIMARYSHADOW, tmpdir); 
        }
    }
    if (system(cmd)) {
        printf("Error: unable to save password artifact; retry.\n");
        system("umount /etc");
        if (!docleanuponly) {
            snprintf(cmd, CMDSIZE, "rm -rf %s", tmpdir);
            system(cmd);
        }
        return 0;
    }
    if (docleanuponly) {
        /* we don't know the tmpdir; so just assume whatever /tmp/dnpw is correct; (could be a bad assumption) */
        snprintf(cmd, CMDSIZE, "cp -f /tmp/dnpw*/etc/passwd " PRIMARYPASSWD);
    } else {
        snprintf(cmd, CMDSIZE, "cp -f %s/etc/passwd " PRIMARYPASSWD, tmpdir); 
    }
    if (system(cmd)) {
        printf("Error: unable to save password artifact; retry.\n");
        system("umount /etc");
        snprintf(cmd, CMDSIZE, "rm -rf %s", tmpdir);
        system(cmd);
        return 0;
    }
    
    system("umount /etc");
    if (docleanuponly) {
        /* we don't know the tmpdir; so just assume whatever /tmp/dnpw is correct; (could be a bad assumption) */
        snprintf(cmd, CMDSIZE, "rm -rf /tmp/dnpw*");
    } else {
        snprintf(cmd, CMDSIZE, "rm -rf %s", tmpdir);
    }
    system(cmd);

    return 0;
}

