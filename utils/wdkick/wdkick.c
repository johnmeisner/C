#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

/* these magic numbers correspond to the memory mapped values for the pins to be asserted */
#define M_RUN "474"
#define M_WDP "475"

#define MRUNSYSDIR "/sys/class/gpio/gpio" M_RUN
#define MWDPSYSDIR "/sys/class/gpio/gpio" M_WDP

/* commands */
#define ENABLEMRUN "echo out > /sys/class/gpio/gpio" M_RUN "/direction; echo 1 > /sys/class/gpio/gpio" M_RUN "/value"
#define DISABLEMRUN "echo 0 > /sys/class/gpio/gpio" M_RUN "/value"
#define ENABLEMWDP "echo out > /sys/class/gpio/gpio" M_WDP "/direction"
#define SETWDP "echo 1 > /sys/class/gpio/gpio" M_WDP "/value"
#define CLRWDP "echo 0 > /sys/class/gpio/gpio" M_WDP "/value"
#define ENABLESYSMRUN "echo " M_RUN " > /sys/class/gpio/export"
#define ENABLESYSMWDP "echo " M_WDP " > /sys/class/gpio/export"

#define CHECKMRUN "cat /sys/class/gpio/gpio" M_RUN "/value | grep 1 >>/dev/null"

/* 100 ms */
#define PULSEASSERTMS 100000

/* 200 ms; just a delay for timing if needed */
#define INITSLEEPMS 200000

/* 3.9 seconds total for sleep */
#define SECONDSSLEEP 3
#define SLEEPMS 900000

static char loop = 1;
static char shutdown_started = 0;

static void termhandler(int a, siginfo_t *b, void __attribute__((unused)) *c)
{
    printf("Ouch: terminating watchdog kick...???\n");
    loop = 0;
    /* If wdkick received the signal from dnlast, get ready for shutdown/reboot */
    if (SIGUSR1 == a) {
        shutdown_started = 1;
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

/* called separately such that st is released */
static char checkgpios(void)
{
    struct stat st;
    char result = 0;

    memset(&st, 0, sizeof(st));
    if (stat(MRUNSYSDIR, &st)) {
        result = system(ENABLESYSMRUN);
    }
    if (stat(MWDPSYSDIR, &st)) {
        result |= system(ENABLESYSMWDP);
    }

    /* result should be zero; if an error happened
       it will be modified above */
    return result;
}

int main(int argc, char *argv[])
{
    sighandler(termhandler, SIGTERM);
    sighandler(termhandler, SIGINT);
    sighandler(termhandler, SIGUSR1);

    /* the set of the gpios may not work the first time */
    while (loop && checkgpios()) {
        system("echo prep trouble >> /tmp/wdkick");
        usleep(INITSLEEPMS);
    }

    /* the enable may not always work the first time */
    while (loop && system(ENABLEMRUN)) {
        system("echo set m_run trouble >> /tmp/wdkick");
        usleep(INITSLEEPMS);
    }
    system("echo set m_run >> /tmp/wdkick");

    while (loop) {
        /* assert and deassert, then wait (once enabling m_run the assertion must take place first time before sleeping) */
        system(SETWDP);
        usleep(PULSEASSERTMS);
        system(CLRWDP);
        sleep(SECONDSSLEEP);
        usleep(SLEEPMS);
        if (system(CHECKMRUN)) {
            system(ENABLEMRUN);
        }
    }

    /* this will never be reached (unless termination signal received) */
    if (shutdown_started) {
        system(DISABLEMRUN);
        printf("WDKICK DISABLING M_RUN FOR SHUTDOWN\n");
    }
    return 0;
}

