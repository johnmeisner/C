#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

#define CV2X_ACTIVE   1
#define CV2X_INACTIVE 2
#define CV2X_UNKNOWN  3

#define MYSIG_REQUEST SIGRTMIN+1

#define QUERY_INDICATOR 0x00800000
#define QUERY_MASK      0x007FFFFF
#define QUERY_SHIFT     24

static unsigned char loop = 1;
static unsigned char status = 0;    /* set to 0 by default */

static void reqhandler(int a, siginfo_t *b, void __attribute__((unused)) *c)
{
    pid_t pid;
    union sigval val;

    if (NULL != b) {
        if ((b->si_value.sival_int & QUERY_MASK) > 0) {
            /* I am the service */
            pid = (b->si_value.sival_int & QUERY_MASK);
            val.sival_int = ((int)(status << QUERY_SHIFT));
            sigqueue(pid, MYSIG_REQUEST, val);   /* no error checking */
        } else {
            /* I am the listener awaiting response */
            status = (b->si_value.sival_int >> QUERY_SHIFT);
            if (!status) {
                status = CV2X_UNKNOWN;
            }
        }
    }
}

static void termhandler(int a, siginfo_t *b, void __attribute__((unused)) *c)
{
    loop = 0;
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

static void parseargs(int argc, char *argv[])
{
    signed char c, dostatus = 0, isdaemon = 0;
    char buf[100];
    union sigval val;
    unsigned short sleepcnt = 500;    /* 500 = 0.5 sec */
    pid_t pid = -1;
    FILE *pf;

    snprintf(buf, sizeof(buf), "pidof -s cv2x_get_status -o %d", getpid());
    if (NULL != (pf = popen(buf, "r"))) {
        memset(buf, 0, sizeof(buf));
        fgets(buf, sizeof(char)*sizeof(buf), pf);
        pid = (pid_t)strtoul(buf, NULL, 10);
        pclose(pf);
    } else {
        printf("Error attempting to determine state\n");
        exit(1);
    }

    if (argc < 2) {
        dostatus = 1;
    } else {
        while ((c = getopt(argc, argv, "D")) != -1) {
            switch (c) {
                case 'D':    /* daemonize */
                    isdaemon = 1;
                    break;
                default:    /* return status */
                    dostatus = 1;
                    break;
            }
        }
    }

    if (dostatus) {
        memset(&val, 0, sizeof(val));
        val.sival_int = QUERY_INDICATOR | getpid();
        if (sigqueue(pid, MYSIG_REQUEST, val)) {
            printf("Error: unable to obtain CV2X status (%d)\n", pid);
            exit(1);
        }
        while (sleepcnt-- && !status) {
            usleep(1000);
        }
        if (!sleepcnt && !status) {
            printf("Error: timeout awaiting CV2X status\n");
            exit(1);
        }
        switch (status) {
            case CV2X_ACTIVE:
                printf("RSU-5940 CV2X Status:\n");
                printf("  RX: Active\n");
                printf("  TX: Active\n");
                break;
            case CV2X_INACTIVE:
                printf("RSU-5940 CV2X Status:\n");
                printf("  RX: Inactive\n");
                printf("  TX: Inactive\n");
                break;
            default:
                printf("RSU-5940 CV2X Status:\n");
                printf("  RX: Unknown\n");
                printf("  TX: Unknown\n");
                break;
        }
        exit(0);    /* terminous operation */
    }

    if (isdaemon && (pid > 1)) {
        /* already another instance running */
        printf("Error: attempting to start second instance of service\n");
        exit(1);
    }

    /* continue on to start daemon mode */
}

int main(int argc, char *argv[])
{
    sighandler(termhandler, SIGTERM);
    sighandler(termhandler, SIGINT);
    sighandler(reqhandler, MYSIG_REQUEST);

    parseargs(argc, argv);

    /* reaching here means continue */
    while (loop) {
        /* this operation takes 1 second, so no need to add any sleep */
        if (system("tpstestapp --rpt 1 | grep _fix_ok | grep Tru >>/dev/null")) {
            /* no fix */
            status = CV2X_INACTIVE;
        } else {
            status = CV2X_ACTIVE;
        }
    }

    return 0;
}

