/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Filename: dnlast.c                                                         */
/* Purpose: A purposeful purposeless application                              */
/*                                                                            */
/* Copyright (C) 2021 DENSO International America, Inc.  All Rights Reserved. */
/*                                                                            */
/* Author(s)/Change History:                                                  */
/*     20210528 JJG - Initial revision                                        */
/*                                                                            */
/*----------------------------------------------------------------------------*/
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include "dn_types.h"
#include "i2v_util.h"
#include "ipcsock.h"
#include "rs.h"

#define MY_ERR_LEVEL   LEVEL_INFO
#define MY_NAME        "dnlast"

/* very simple purpose: do nothing if called; it's needed as a crutch for the 5912 */

static char loop = 1;

static void termhandler(int a, siginfo_t *b, void __attribute__((unused)) *c)
{
	loop = 0;
    FILE *pf;
    char buf[100];
    pid_t wdkick_pid = -1;

    /* Get Watchdog Kick PID and send it a termination signal*/
    if (NULL != (pf = popen("pidof -s wdkick", "r"))) {
        memset(buf, 0, sizeof(buf));
        fgets(buf, sizeof(buf), pf);
        wdkick_pid = (pid_t)strtol(buf, NULL, 10);
        pclose(pf);

        kill(wdkick_pid, SIGUSR1);
    }

}

static void sighandler(void (*handler)(int, siginfo_t *, void*), int sig)
{
    struct sigaction act;

    memset(&act, 0, sizeof(act));
    act.sa_sigaction = handler;
    sigemptyset(&act.sa_mask);
    act.sa_flags = SA_SIGINFO|SA_RESTART;
    sigaction(sig, &act, NULL);
}

int main(int argc, char *argv[])
{
	sighandler(termhandler, SIGTERM);
	sighandler(termhandler, SIGINT);

    i2vUtilEnableDebug(MY_NAME);
    i2vUtilEnableSyslog(MY_ERR_LEVEL, MY_NAME);

	while (loop) {
		sleep(30);    /* maybe make longer */
        /* rsuhealth check added 20231017: make sure the file name remains in sync with rsuhealth */
        if (!system("[[ -e /tmp/six332 ]]") && system("[[ ! -z \"`pidof -s rsuhealth`\" ]]")) {
            /* the above are separated out so that the following is prevented if someone manually deletes the file */
            I2V_DBG_LOG(LEVEL_CRIT, MY_NAME, "Alert: rsuhealth is not detected after previous activity; rebooting the system\n");
            sleep(3);   /* allow some time for the syslog to catch up if busy */
            system("reboot");
        }
	}

	/* this will never be reached (unless termination signal received) */
	return 0;
}

