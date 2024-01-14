/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Filename: rsuhealth.c                                                      */
/* Purpose: RSU Health Monitor                                                */
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
#include <errno.h>
#include <pthread.h>
#include <time.h>
#include "dn_types.h"
#include "wsu_sharedmem.h"
#include "i2v_util.h"
#include "ipcsock.h"
#include "rs.h"
#include "shm_tps.h"
#include "shm_sous.h"
#include "rsuhealth.h"
#include "rsudiagnostic.h"
#if defined(MY_UNIT_TEST)
#include "stubs.h"
#define MAIN my_main
#else
#define MAIN main
#endif

/* mandatory logging defines */
#define MY_ERR_LEVEL   LEVEL_PRIV /* syslog only, no serial. */
#define MY_NAME        "rsuhealth"

#define OUTPUT_MODULUS  1200      /* seconds, to serial & syslog */
#define SYSLOG_OUTPUT_MODULUS  30 /* seconds, to syslog only */


#if defined(MY_UNIT_TEST)
/* Parent thread waits for child thread to make mainloop WFALSE then exits.
 * This ensures test doesn't get stuck forever and should be 
 * more than the child wait time for orderly exit. 
 */
#define PARENT_TEST_ITERATION_MAX 30 /* # times through mainloop before normal exit mainloop==WFALSE */
#endif

/* the beginning of the health 'script' */

/* logging items */
#define RSULOG        "/rwflash/rsuhealth.log"
#define RSULOGOLD     "/rwflash/rsuhealth.log.old"

/* sorry, going to hard code eth0; original startup.sh had support for a quick change */
#define MNTRWFLASH    "/mnt/rwflash"
#define SYSETHFILE    "/etc/systemd/network/10-eth0.network"
#define BINDETHFILE   "/tmp/10-eth0.network"
#define TMPRDFILE     "/tmp/pernet"
#if defined(MY_UNIT_TEST)
#define STARTUP       "../../misc/startup.sh"
#else
#define STARTUP       "/rwflash/configs/startup.sh"
#endif
#define READYFILE     "/tmp/rsuhealthready"
#define SYSIFC        "eth0"

#define ETCPW         "/etc/passwd"
#define ETCSDW        "/etc/shadow"
#define LOGINFILEDIR  "/mnt/rwflash/etc"
#define LOGINPW       "/mnt/rwflash/etc/passwd"
#define LOGINSDW      "/mnt/rwflash/etc/shadow"

#define SSHFILEDIR    "/mnt/rwflash/etc/ssh"
#define ETCSSHDCONF   "/etc/ssh/sshd_config"
#define SSHDCONF      "/mnt/rwflash/etc/ssh/sshd_config"

#define SLEEPSEC       1
#define DEFAULTSLEEP   55
/* realtime sleep; uses usleep and 25 ms iterations */
#define RTSLEEP        25000
#define BLINKSLEEP    500000
#define KILL_LOCKOUT     120

#define TERMINATE  0x55A5

#define BIGLINE    200
#define MAXLINE    30

/* minimum static ip: w.x.y.z = 7 characters */
#define MINSTATICIPLEN    7

/* LED related */
/* for simplicity LEDs are in the upper nibble and actions are the lower nibble */
#define LED_ON        0x01
#define LED_OFF       0x02
#define LED_BLINK     0x04
#define LED_BLINKFAST 0x05
/* special management states */
#define LED_CLEANUP   0x08
#define LED_REVERT    0x09
#define IGNORE_COLOR  0x0F

#define LED_AMBER     0x10
#define LED_FAULT     0x20
#define LED_POWER     0x40
#define LED_ALL       0x80
#define IGNORE_ACTION 0xF0

#define SYSTEMFAULTON    "spi_test -D /dev/spidev0.1 -H -b 16 -G 0x0594 > /dev/null"
#define SYSTEMAMBERON    "spi_test -D /dev/spidev0.1 -H -b 16 -G 0x0586 > /dev/null"
#define SYSTEMGREENON    "spi_test -D /dev/spidev0.1 -H -b 16 -G 0x0592 > /dev/null"
#define SYSTEMLEDOFF     "spi_test -D /dev/spidev0.1 -H -b 16 -G 0x0596 > /dev/null"


/* hopefully, we never have a pid with a numeric ID this large (very unlikely) */
#define LEDQUERY_REQUEST  0x40000000
#define LEDQUERY_REQMASK  0x3FFFFFFF

#define UTILITY_REQUEST   0x40000000
#define UTILITY_REQMASK   0x3FFFFFFF
#define UTILITY_REQPID    0x003FFFFF

#if defined (PLATFORM_HD_RSU_5912) /* 5912 had different setup and we had to poll. */
/* push button */
#define ONESECONDINMILLIS       1000000
#define QUARTERSECINMILLIS       250000
#define PUSHBUTTONCNTDWNUNSET   (ONESECONDINMILLIS / RTSLEEP)
#define PUSHBUTTONCNTDWNSET     (QUARTERSECINMILLIS / RTSLEEP)
/* 60 quarter seconds = 15 seconds */
#define PUSHBUTTONTHRESHOLD     60
/* the check below is for the GPIO on the i.MX8 tied to GPIO3 */
#define PUSHBUTTONCHECK         " [ \"0\" == \"`cat /sys/class/gpio/gpio448/value`\" ] "
#endif

/* password related */
/* the below expression just checks if the password change field on rsuuser is 0 (change password) */
#define SYSTEMCRYPTICPASSWORDCHECK \
    "[ \"0\" == \"`cat /etc/shadow | grep rsuuser | sed 's/rsuuser:[^:]*:\\([0-9][0-9]*\\).*/\\1/'`\" ]"

/* GUI restart related */
#define GUIRESTARTFILE   "/tmp/gcol"
#define VGUIMANAGEFILE   "/tmp/vgol"

#define PRINTMAX(fmt, args...) \
            if (NULL != hlthfp) { fprintf(hlthfp, "%ld: ", time(NULL)); fprintf(hlthfp, fmt, ##args); fflush(hlthfp); } \
            if (moretimes) { printf(fmt, ##args); fflush(stdout); moretimes--; }


/* add any functions depending on rwflashrdy that are 'one-shot' functions
   into this list; this is called by preparepassword which sets rwflashrdy */
#define RWFLASHDEPENDENTFXNS \
    if (calldependentfxns) { \
    }

#if defined(MY_UNIT_TEST)
/* Child thread exits before parent thread. Should be less than parent for oredrerly exit */
#define CHILD_TEST_ITERATION_MAX 75  /* # times through mainloop before normal exit mainloop==WFALSE */
#endif

/* LED data structures; this is a less than optimal means of managing the LEDs
   (but better than predecessor platforms).  The map will track the state of
   the LED and how many requests (activate - LED_ON); realistically the PID of
   the calling thread should also be tracked such that corresponding LED_OFF
   requests are only decrimented from the count if matching the PID; but that's a
   lot more work for minimal value add and would only be suitable for the FAULT
   LED; maybe in the future consider making it better
*/
typedef struct {
    short status;
    short requestcnt;
    char  isactive;
} ledstate;

typedef struct {
    ledstate amber;
    ledstate fault;
    ledstate power;
    short    currentstate;
    short    laststate;
} ledstatemap;

/*----------------------------------------------------------------------------*/
/* Global variables                                                           */
/*----------------------------------------------------------------------------*/
static FILE *hlthfp = NULL;
char loop = 1;
static char ledqueryrdy = 0;    /* something has to get allocated for LED query but 8 bits is acceptable */
static char statequeryrdy = 0;    /* something has to get allocated for state query but 8 bits is acceptable */
static char moretimes = 80;   /* is 80 enough messages? */
static char ipgood = 0;
char i2vfailcnt;
static char usingdhcp = 0;
static char rwflashrdy = 0;
static char firstlogin = 0;
static char restartgui = 0;
static char terminatei2v = 0;  /* this exists to prevent rsuhealth from terminating i2v at boot up */
static char marshali2v = 1;    /* indicator that rsuhealth is controlling state of i2v (marshal);
                                  start as 1 to allow i2v first init during boot */
static int32_t logfile = 0;

static pthread_t rtthread;

static ledstatemap ledmap;
static char ledoverridectr = 0;
static char ledoverride    = 0;

static char guictl = 0;
//TODO: need to get this involved.
int rsusystem = RSUHSTATE_UNKNOWN;

/* set value to 0 if not a customer build; set to 1 if customer; moved here to support
   pushbutton implementation performed in realtime thread; this is used as a means to
   bypass the main thread's attempts to restore any password file deleted for reset */
static int platformtype = -1;

rsuhealth_t * shm_rsuhealth_ptr = NULL; /* RSUHEALTH SHM */
rsuhealth_t   localrsuhealth;
rsuhealth_t   priorlocalrsuhealth; /* keep track on change */
extern rskStatusType rskstatus;

led_diagnostics_t * shm_leddiag_ptr = NULL; // LED SHM
led_diagnostics_t local_leddiag;

static pthread_t OneSecThrdID;
static pthread_t SetLedsThrdID;

extern int32_t ready_set_fault_count;
extern shm_tps_t   localshmtps;
extern void initrsudiagnostics(int32_t debugEnable);
extern void *OneSecThrd(void *ptr);
extern void *SetLedsThrd(void *ptr);
extern void updatediagnosticshm(void);

static pthread_t WatcherThrdID;

#define ONESECTHREADFAULTMAX RSUHEALTH_BOOTUP_WAIT /* needs time to start */
extern uint32_t onesecondcounter; /* number of times rsudiag through mainloop */
static uint32_t prioronesecondcounter = 0; /* number of times rsudiag through mainloop */
static uint32_t onesecondcounterfault = 0;

#define REALTIMETHREADFAULTMAX 5 /* needs time to start */
static uint32_t realtimecounter = 0;
static uint32_t priorrealtimecounter = 0;
static uint32_t realtimecounterfault = 0;

static uint32_t rollingcounter = 0x0; /* modulus for output */

/* Network monitor thread */
static pthread_t NetMonThrdID;
extern void *networkEventThread(void *ptr);

/* Periodcally check customer folder and purge till below threshold. */
#define RSUHEALTH_ACTIVE_DIR        "/rwflash/I2V"
#define MANAGE_CUSTOMER_FOLDER_FILE "docustomerfolder"
static pthread_t cleancustomerfolderID;
extern void *cleancustomerfolderThrd(void *ptr);

extern int printFaultLEDStr();

/*----------------------------------------------------------------------------*/
/* Function protos                                                            */
/*----------------------------------------------------------------------------*/
static int prepareguiscript(void);
static int prepvguiscript(void);

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
static void initstatics(void)
{
  hlthfp = NULL;
  loop = 1;
  ledqueryrdy = 0;
  statequeryrdy = 0;
  moretimes = 80;
  ipgood = 0;
  usingdhcp = 0;
  rwflashrdy = 0;
  firstlogin = 0;
  i2vfailcnt = 0;
  restartgui = 0;
  terminatei2v = 0;
  marshali2v = 1;
  memset(&rtthread,0,sizeof(rtthread));
  memset(&ledmap,0,sizeof(ledmap));
  ledoverridectr = 0;
  ledoverride    = 0;
  guictl = 0;
  rsusystem = RSUHSTATE_UNKNOWN;
  platformtype = -1;
  memset(&OneSecThrdID,0,sizeof(OneSecThrdID));
  memset(&SetLedsThrdID,0,sizeof(SetLedsThrdID));
  memset(&WatcherThrdID,0,sizeof(WatcherThrdID));
  memset(&NetMonThrdID,0,sizeof(NetMonThrdID));
  shm_rsuhealth_ptr = NULL;
  memset(&localrsuhealth,0,sizeof(localrsuhealth));
  memset(&priorlocalrsuhealth,0,sizeof(priorlocalrsuhealth));
  prioronesecondcounter = 0; /* number of times rsudiag through mainloop */
  onesecondcounterfault = 0;
  realtimecounter = 0;
  priorrealtimecounter = 0;
  realtimecounterfault = 0;
  logfile = 0;
  i2vInitStatics();
}
/*----------------------------------------------------------------------------*/
/* Functions for signals                                                      */
/* These should execute quickly!                                              */
/*----------------------------------------------------------------------------*/
static void termhandler(int a, siginfo_t *b, void __attribute__((unused)) *c)
{
  a = a;
  b = b;
  loop = 0;
}

static void i2vhandler(int a, siginfo_t *b, void __attribute__((unused)) *c)
{
    /* a is the signal */
    if (a == RSUSIG_TERMI2V) {
        marshali2v = 1;
        if (NULL != b) {
            if (b->si_value.sival_int == TERMINATE) {
                terminatei2v = 1;
            }
        }
    } else {
        marshali2v = 0;
        terminatei2v = 0;
    }
}

static void ledhandler(int a, siginfo_t *b, void __attribute__((unused)) *c)
{
    char resetled = 0;

    a = a;

    if (NULL != b) {
        /* color first; determines supported actions */
        switch(b->si_value.sival_int & IGNORE_ACTION) {
            case LED_AMBER:
                /* only on or off supported 
                   only change state if new request is different from current state */
                switch (b->si_value.sival_int & IGNORE_COLOR) {
                    case LED_ON:
                        ledmap.amber.status = LED_ON;
                        if (ledmap.currentstate != (short)b->si_value.sival_int) {
                            ledmap.laststate = ledmap.currentstate;
                            ledmap.currentstate = (short)b->si_value.sival_int;
                        }
                        break;
                    case LED_OFF:
                        if (ledmap.currentstate != (short)b->si_value.sival_int) {
                            ledmap.amber.status = LED_OFF;
                            // Only if the current state is amber, turn amber off
                            // TODO Needs update when multiple LEDs are enabled
                            if ((ledmap.currentstate & IGNORE_ACTION) == LED_AMBER) {
                                ledmap.currentstate = (LED_AMBER | LED_OFF);
                            }
                        }
                        break;
                    default:
                        break;
                }
                break;
            case LED_FAULT:
                // if Amber is on, don't change the color, just set laststate 
                if (ledmap.currentstate == (LED_AMBER | LED_ON)) {
                    if (ledmap.laststate != (short)b->si_value.sival_int) {
                        ledmap.laststate = (short)b->si_value.sival_int;
                    }
                }    
                else {
                    /* On, off, blink, and fast blink supported
                    Only switch state if new request is different from current state */
                    switch (b->si_value.sival_int & IGNORE_COLOR) {
                        case LED_ON:
                            ledmap.fault.status = LED_ON;
                            if (ledmap.currentstate != (short)b->si_value.sival_int) {
                                ledmap.laststate = ledmap.currentstate;
                                ledmap.currentstate = (short)b->si_value.sival_int;
                            }
                            break;
                        case LED_OFF:
                            ledmap.fault.status = LED_OFF;
                            if (ledmap.currentstate != (short)b->si_value.sival_int) {
                                // Only if the current state is fault, turn fault off. 
                                // TODO Needs update when multiple LEDs are enabled
                                if ((ledmap.currentstate & IGNORE_ACTION) == LED_FAULT) {
                                    ledmap.currentstate = (LED_FAULT | LED_OFF);
                                }
                            }
                            break;
                        case LED_BLINK:
                            ledmap.fault.status = LED_BLINK;
                            if (ledmap.currentstate != (short)b->si_value.sival_int) {
                                ledmap.laststate = ledmap.currentstate;
                                ledmap.currentstate = (short)b->si_value.sival_int;
                            }
                            break;
                        case LED_BLINKFAST:
                            ledmap.fault.status = LED_BLINKFAST;
                            if (ledmap.currentstate != (short)b->si_value.sival_int) {
                                ledmap.laststate = ledmap.currentstate;
                                ledmap.currentstate = (short)b->si_value.sival_int;                        
                            }
                            break;
                        default:
                            break;
                    }
                }
                break;
            case LED_POWER:
               // if Amber is on, don't change the color, just set laststate 
               if (ledmap.currentstate == (LED_AMBER | LED_ON)) {
                   if (ledmap.laststate != (short)b->si_value.sival_int) {
                       ledmap.laststate = (short)b->si_value.sival_int;
                   }
               }
               else {
                   /* On, off, and blink supported
                    Only change state if new request is different from current state */
                    switch (b->si_value.sival_int & IGNORE_COLOR) {
                        case LED_ON:
                            ledmap.power.status = LED_ON;
                            if (ledmap.currentstate != (short)b->si_value.sival_int) {
                                ledmap.laststate = ledmap.currentstate;
                                ledmap.currentstate = (short)b->si_value.sival_int;
                            }
                            break;
                        case LED_OFF:
                            ledmap.power.status = LED_OFF;
                            if (ledmap.currentstate != (short)b->si_value.sival_int) {
                                // Only if the current state is power, turn power off
                                // TODO Needs update when multiple LEDs are enabled
                                if ((ledmap.currentstate & IGNORE_ACTION) == LED_POWER) {
                                    ledmap.currentstate = (LED_POWER | LED_OFF);
                                }
                            }
                            break;
                        case LED_BLINK:
                            ledmap.power.status = LED_BLINK;
                            if (ledmap.currentstate != (short)b->si_value.sival_int) {
                                ledmap.laststate = ledmap.currentstate;
                                ledmap.currentstate = (short)b->si_value.sival_int;
                            }
                            break;
                        default:
                            break;
                    }
                }
                break;
            case LED_ALL:
                switch (b->si_value.sival_int & IGNORE_COLOR) {
                    case LED_OFF:
                        ledmap.amber.status = LED_OFF;
                        ledmap.fault.status = LED_OFF;
                        ledmap.power.status = LED_OFF;
                        if (ledmap.currentstate != (short)b->si_value.sival_int) {
                            ledmap.laststate = ledmap.currentstate;
                            ledmap.currentstate = (short)b->si_value.sival_int;
                        }
                        break;
                    case LED_REVERT:
                        // swap variables
                        ledmap.currentstate = ledmap.currentstate + ledmap.laststate;
                        ledmap.laststate = ledmap.currentstate - ledmap.laststate;
                        ledmap.currentstate = ledmap.currentstate - ledmap.laststate;
                        resetled = 1;
                        break;
                
                }
                break;
            default:
                break;
        }

        if (resetled) {
            switch (ledmap.currentstate & IGNORE_ACTION) {
                /* Don't need to turn other LEDs off here, will be done in health fn */
                case LED_AMBER:
                    ledmap.amber.status = ledmap.currentstate & IGNORE_COLOR;
                    break;
                case LED_FAULT:
                    ledmap.fault.status = ledmap.currentstate & IGNORE_COLOR;
                    break;
                case LED_POWER:
                    ledmap.power.status = ledmap.currentstate & IGNORE_COLOR;
                    break;
                default:
                    /* all off; do nothing */
                    break;
            }
        }
    }
}


static void ledqueryhandler(int a, siginfo_t *b, void __attribute__((unused)) *c)
{
    pid_t pid;
    union sigval val;

    a = a;

    if (NULL != b) {
        if (b->si_value.sival_int & LEDQUERY_REQUEST) {
            /* i am service rsuhealth, i send response to pid passed in */
            pid = b->si_value.sival_int & LEDQUERY_REQMASK;
            if (pid <=1) {
                /* do nothing else */
                return;
            }
            /* amber, fault, power */
            val.sival_int = (ledmap.amber.status  << 16);
            val.sival_int |= (ledmap.fault.status  << 8);
            val.sival_int |= ledmap.power.status;
            sigqueue(pid, RSUSIG_LEDQUERY, val);    /* don't care about return errors */
        } else {
            /* i am requester obtaining response from service */
            ledmap.amber.status = (b->si_value.sival_int >> 16);
            ledmap.fault.status = (b->si_value.sival_int >> 8) & 0xFF;
            ledmap.power.status = (b->si_value.sival_int) & 0xFF;
            ledqueryrdy = 1;
        }
    }
}

static void statehandler(int a, siginfo_t *b, void __attribute__((unused)) *c)
{
    pid_t pid;
    union sigval val;

    a = a;

    if (NULL != b) {
        switch (b->si_value.sival_int) {
            case RSUHSTATE_LEDTESTSTART:
                ledoverride = 1;
                ledoverridectr = 10;  /* 10 iterations of main thread which is about 10 min */
                break;
            case RSUHSTATE_LEDTESTEND:
                ledoverride = 0;
                ledoverridectr = 0;
                break;
            case RSUHSTATE_NORMAL:
                rsusystem = RSUHSTATE_NORMAL;
                break;
            default:
                /* special case before default break */
                if ((b->si_value.sival_int & RSUHSTATE_QUERY) == RSUHSTATE_QUERY) {
                    /* query, determine if i am send or service */
                    if ((b->si_value.sival_int & QUERY_PID_MASK) > 0) {
                        /* I am the service obtaining a request */
                        pid = (b->si_value.sival_int & QUERY_PID_MASK);
                        val.sival_int = ((int)(rsusystem << QUERY_RESP_SHIFT)) | RSUHSTATE_QUERY;
                        sigqueue(pid, RSUSIG_STATE, val);    /* don't care about return errors */
                    } else {
                        /* i am the sender obtaining a response */
                        rsusystem = (b->si_value.sival_int >> QUERY_RESP_SHIFT);
                        statequeryrdy = 1;
                    }
                }
                break;
        }
    }
}

static void utilityhandler(int a, siginfo_t *b, void __attribute__((unused)) *c)
{
    pid_t pid;
    /* union sigval val; */
    unsigned char action;

    a = a;

    if (NULL != b) {
        if (b->si_value.sival_int & UTILITY_REQUEST) {
            /* i am service rsuhealth, i send response to pid passed in */
            pid = b->si_value.sival_int & UTILITY_REQPID;
            if (pid <=1) {
                /* do nothing else */
                return;
            }
            action = (unsigned int)((b->si_value.sival_int & UTILITY_REQMASK) >> 23);
            switch (action) {
                case RSUH_IPCHECK:
                    /* clear ip set state and select to disable gui */
                    ipgood = 0;
                    restartgui = 1;
                    PRINTMAX("Request received to recheck/reapply IP settings; GUI will restart\n")
                    break;
                default:
                    return;
            }
            /* if future utility actions require a response enable below (set val accordingly)
            sigqueue(pid, RSUSIG_LEDQUERY, val);    // don't care about return errors
            */
        /* if required in the future to handle a response
        } else {
            // i am requester obtaining response from service
        */
        }
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

/*----------------------------------------------------------------------------*/
/* blinkThread: simple thread (Green LED only) for blinking                   */
/*----------------------------------------------------------------------------*/
static void *blinkThread(void *ptr)
{
    char led_on_cmd[256];
    int blinksleep;

    ptr = ptr;

    if (ledmap.fault.status == LED_BLINK) {
        sprintf(led_on_cmd, "%s", SYSTEMFAULTON);
        blinksleep = BLINKSLEEP;
    } 
    else if (ledmap.fault.status == LED_BLINKFAST) {
       sprintf(led_on_cmd, "%s", SYSTEMFAULTON);
       blinksleep = BLINKSLEEP/2;
    }
    else {
        sprintf(led_on_cmd, "%s", SYSTEMGREENON);
        blinksleep = BLINKSLEEP;
    }
    
    while (loop) {
        system(led_on_cmd);

        usleep(blinksleep);
        system(SYSTEMLEDOFF);
        usleep(blinksleep);
    }

    pthread_exit(0);
}

/*----------------------------------------------------------------------------*/
/* i2vkiller: simple thread that won't block realtime thread for killing      */
/* i2v; i2v can take seconds to go down but there may be waiting users        */
/*----------------------------------------------------------------------------*/
static void *i2vkiller(void *ptr)
{
    pid_t *pid = (pid_t *)ptr;
    char sleepcnt = KILL_LOCKOUT;  /* 120 * 25 ms = 3 sec */
    union sigval val;

    if (*pid > 1) {
        /* realistically, it should be greater than 1000 but no real way to tell;
           MUST be greater than init which is 1 */
        if (sigqueue(*pid, SIGTERM, val)) {    /* val is a don't care */
            /* error received */
            PRINTMAX("Error attempting to terminate i2v: %d\n", errno)
        } else {
            while (sleepcnt--) {
                usleep(RTSLEEP);    /* give i2v time to shutdown; the way i2v is 
                                       coded, it could take over 1 min in worst
                                       case to go down; give it up to 3 seconds */
                if (sigqueue(*pid, 0, val)) {    /* val is a don't care */
                    /* process doesn't exist */
                    break;
                }
            }
            if (sigqueue(*pid, SIGKILL, val)) {    /* val is a don't care */
                /* error received */
                PRINTMAX("Error attempting to terminate i2v: %d\n", errno)
            }
        }

        /* now the fugly part; check for child apps that may linger just in case; this list is only
           as reliable as folks don't change app names (or add new ones) */
        system("kill -9 `ps -ef | grep iwmh | grep -v grep | awk '{print $2}'` 2> /dev/null");
        system("kill -9 `ps -ef | grep scs | grep -v grep | awk '{print $2}'` 2>/dev/null");
        system("kill -9 `ps -ef | grep spat16 | grep -v grep | awk '{print $2}'` 2>/dev/null");
        system("kill -9 `ps -ef | grep amh | grep -v grep | awk '{print $2}'` 2>/dev/null");
        system("kill -9 `ps -ef | grep ipb | grep -v grep | awk '{print $2}'` 2>/dev/null");
        system("kill -9 `ps -ef | grep srm | grep -v grep | awk '{print $2}'` 2>/dev/null");
    }

    PRINTMAX("finished execution of RSU services termination\n")

    pthread_exit(0);
}
/*----------------------------------------------------------------------------*/
/* Reset password for rsuuser ONLY. No return from this call.                 */
/*----------------------------------------------------------------------------*/
static void reset_rsuuser_pwd(void)
{
    struct timespec t;
 
    memset(&t, 0, sizeof(t));
    /* protection against this thread terminating while performing reset (which could lock a user out) */
    system("shutdown");    /* this will trigger a scheduled shutdown in 60 seconds */
    /* first stop the main thread from potentially undoing what we are deliberately about
       to do */
    platformtype = 0;   /* this will cause the main thread to stop caring about password files */
    t.tv_sec = 1;
    nanosleep(&t, NULL);
    /* sleep(1);*/    /* sleep can be called now because we are going to reboot; allow main thread
                         to clear out any operation that may have started with platformtype = 1 */
    /* trigger password reset */
    I2V_DBG_LOG(LEVEL_CRIT, MY_NAME,"calling reset of system on password reset button action\n");
    system("dnpasswd reset");
    /* reset system after a brief pause */
    system("sync");
    t.tv_sec = 1;
    nanosleep(&t, NULL);
    /* sleep(1); */
    system("reboot");   /* overrides shutdown; proceed to shutdown now */
}
/*----------------------------------------------------------------------------*/
/* realtimeHealthThrd: handles items that can't wait for the main thread's    */
/* sleeps to end; initial items: termination of i2v, led management           */
/*----------------------------------------------------------------------------*/
static void *realtimeHealthThrd(void *ptr)
{
    FILE *pf;
    char buf[100];    /* buf used for cmd and result */
    pid_t pid = -1;
    union sigval val;
    pthread_t i2vkillthr, blinkthr;
    int ret = 0;
    short terminatelockout = 0;    /* this gives time for killer thread to run */
#if defined (PLATFORM_HD_RSU_5912)
    unsigned char pbctr = PUSHBUTTONCNTDWNUNSET;
    unsigned char pbset = 0, pbactive = 0;
    struct timespec t;
    memset(&t, 0, sizeof(t));
#endif
   
    ptr = ptr;

    /* initial set up */
    memset(&val, 0, sizeof(val));

    while (loop) {
        usleep(RTSLEEP);

        /* i2v termination support; needs to run to completion; a user who tries to start and stop in quick
           succession can cause problems; perhaps a better implementation may be considered in the future 
           but i'm willing to live with this for now (max 3 second delay after issuing termination) */
        if (terminatei2v) {
            if (NULL != (pf = popen("pidof -s i2v_app", "r"))) {
                memset(buf, 0, sizeof(buf));
                fgets(buf, sizeof(char)*sizeof(buf), pf);
                pid = (pid_t)strtoul(buf, NULL, 10);
                pclose(pf);
                if (!terminatelockout && (pid > 1)) {
                    PRINTMAX("Request to terminate RSU services initiated (%d)\n", (int)pid)
                    /* the lockout must match thread max duration (with buffer) */
                    terminatelockout = KILL_LOCKOUT + 2;    /* 2 extra iterations added; actually not */
                    pthread_create(&i2vkillthr, NULL, i2vkiller, (void *)&pid);
                } else if (pid == 0) {
                    /* no i2v app */
                    terminatelockout = 0;
                }
            } else {
                PRINTMAX("Internal error attempting to determinte i2v presense\n")
            }

            if (terminatelockout) {
                terminatelockout--;    /* decrement for this iteration */
            }
            /* i2v not running or pid not 'found'; do not adjust terminatei2v */
        } else {
            terminatelockout = 0;
        }

        /* led support; there is a defect not overcome with code; turning off any
           led turns off all of them; the device used to control the pins has limited
           capability (seemingly?); device is NCV7718 and requires all bits shifted in
           for control; this means disabling LEDs or enabling them if the values are
           incorrect; consider performing some tests to see if any value can be specified
           to only turn off a single pin and not all (requires knowing register state) 
           NOTE: I think i tested this on the 5910 and it didn't work (LEDs misbehave)
           hence sticking with the proven good values for now */
         /* is active: 0 = not active 
                       1 = solid
                       2 = blinking regular speed
                       3 = fast blinking
        */

        /* amber */
        switch (ledmap.amber.status) {
            case LED_ON:
                /* only make adjustments if previously undone */
                if (!ledmap.amber.isactive) {
                    if (ledmap.power.isactive == 2 || ledmap.fault.isactive >= 2) {
                        if (0 != (ret = pthread_cancel(blinkthr))) {
                            PRINTMAX("Error disabling blinking\n")
                        } else {
                            if (ledmap.power.isactive == 2) {
                                ledmap.power.isactive = 0;
                            } else {
                                ledmap.fault.isactive = 0;
                            }
                        }
                    }
                    if (system(SYSTEMAMBERON)) {
                        PRINTMAX("Error turning on amber LED\n")
                    } else {
                        printf("Turning amber on\n");
                        ledmap.amber.isactive = 1;
                        /* remember that the others are now 'off'; set state, next iteration will adjust */
                        ledmap.fault.status = LED_CLEANUP;
                        ledmap.power.status = LED_CLEANUP;
                    }
                }
                break;
            case LED_OFF:
                /* just turn myself off */
                if (ledmap.amber.isactive) {
                    if (system(SYSTEMLEDOFF)) {
                        PRINTMAX("Error turning off amber LED\n")
                    } else {
                        ledmap.amber.isactive = 0;
                        /* remember that the others are now 'off'; set state, next iteration will adjust */
                        ledmap.fault.status = LED_CLEANUP;
                        ledmap.power.status = LED_CLEANUP;
                    }
                }
                break;
            case LED_CLEANUP:
                /* simple cleanup */
                ledmap.amber.status = LED_OFF;
                ledmap.amber.isactive = 0;
                break;
            default:
                break;
        }
        switch (ledmap.fault.status) {
            case LED_ON:
                /* only make adjustments if previously undone */
                if (ledmap.fault.isactive !=1) {
                    if (ledmap.power.isactive == 2 || ledmap.fault.isactive >= 2) {
                        if (0 != (ret = pthread_cancel(blinkthr))) {
                            PRINTMAX("Error disabling blinking\n")
                        } else {
                            if (ledmap.power.isactive == 2) {
                                ledmap.power.isactive = 0;
                            }
                        }
                    }
                    if (system(SYSTEMFAULTON)) {
                        PRINTMAX("Error turning on fault LED\n")
                    } else {
                        ledmap.fault.isactive = 1;
                        /* remember that the others are now 'off'; set state, next iteration will adjust */
                        ledmap.amber.status = LED_CLEANUP;
                        ledmap.power.status = LED_CLEANUP;
                    }
                }
                break;
            case LED_OFF:
                /* just turn myself off */
                if (ledmap.fault.isactive) {
                    if (ledmap.fault.isactive >= 2) {
                        if (0 != (ret = pthread_cancel(blinkthr))) {
                            PRINTMAX("Error disabling blinking\n")
                        }
                    }
                    if (system(SYSTEMLEDOFF)) {
                        PRINTMAX("Error turning off fault LED\n")
                    } else {
                        ledmap.fault.isactive = 0;
                        /* remember that the others are now 'off'; set state, next iteration will adjust */
                        ledmap.amber.status = LED_CLEANUP;
                        ledmap.power.status = LED_CLEANUP;
                    }
                }
                break;
            case LED_BLINK:
            case LED_BLINKFAST:
                 /* if fault is not blinking at the correct speed */
                if (! ( (ledmap.fault.isactive==2 && ledmap.fault.status==LED_BLINK) || 
                        (ledmap.fault.isactive==3 && ledmap.fault.status==LED_BLINKFAST))) {
                    // if blinkthread is power LED or wrong speed fault LED, kill it
                    if ((ledmap.power.isactive == 2) ||
                        (ledmap.fault.isactive == 2 && ledmap.fault.status == LED_BLINKFAST) ||
                        (ledmap.fault.isactive == 3 && ledmap.fault.status == LED_BLINK)){
                        if (0 != (ret = pthread_cancel(blinkthr))) {
                            PRINTMAX("Error disabling blinking \n")
                        } else { 
                            ledmap.power.isactive = ledmap.power.isactive==2? 0: ledmap.power.isactive; 
                        }
                    }
                    if (0 != (ret = pthread_create (&blinkthr, NULL, blinkThread, NULL))){
                        PRINTMAX("Error starting blinking \n")
                    } else {
                        ledmap.fault.isactive = ledmap.fault.status==LED_BLINKFAST? 3: 2;
                        /* remember that the others are now off; set state, next iteration will adjust */
                        ledmap.amber.status = LED_CLEANUP;
                        ledmap.power.status = LED_CLEANUP;
                    }
                }
                break;
            case LED_CLEANUP:
                /* simple cleanup */
                ledmap.fault.status = LED_OFF;
                ledmap.fault.isactive = 0;
                break;
            default:
                break;
        }
        /* power has to be handled slightly differently */
        switch (ledmap.power.status) {
            case LED_ON:
                if (ledmap.power.isactive != 1) {
                    if (ledmap.power.isactive == 2 || ledmap.fault.isactive >= 2) {    /* blink state */
                        if (0 != (ret = pthread_cancel(blinkthr))) {
                            PRINTMAX("error disabling blinking")
                        } else {
                            if (ledmap.fault.isactive == 2) {
                                ledmap.fault.isactive = 0;
                            }
                        }
                    }
                    if (system(SYSTEMGREENON)) {
                        PRINTMAX("Error turning on fault LED\n")
                    } else {
                        ledmap.power.isactive = 1;
                        /* remember that the others are now 'off'; set state, next iteration will adjust */
                        ledmap.amber.status = LED_CLEANUP;
                        ledmap.fault.status = LED_CLEANUP;
                    }
                }
                break;
            case LED_OFF:
                if (ledmap.power.isactive) {
                    if (ledmap.power.isactive == 2) {
                        if (0 != (ret = pthread_cancel(blinkthr))) {
                            PRINTMAX("Error disabling blinking\n")
                        }
                    }
                    if (system(SYSTEMLEDOFF)) {
                        PRINTMAX("Error turning off fault LED\n")
                    } else {
                        ledmap.power.isactive = 0;
                        /* remember that the others are now 'off'; set state, next iteration will adjust */
                        ledmap.amber.status = LED_CLEANUP;
                        ledmap.fault.status = LED_CLEANUP;
                    }
                }
                break;
            case LED_BLINK:
                if (ledmap.power.isactive != 2) {
                    if (ledmap.fault.isactive >= 2) {
                        if (0!= (ret = pthread_cancel(blinkthr))) {
                            PRINTMAX("Error disabling blinking \n")
                        } else {
                            ledmap.fault.isactive = 0;
                        }
                    }
                    if (0 != (ret = pthread_create(&blinkthr, NULL, blinkThread, NULL))) {
                        PRINTMAX("Error starting blinking\n")
                    } else {
                        ledmap.power.isactive = 2;
                        /* remember that the others are now 'off'; set state, next iteration will adjust */
                        ledmap.amber.status = LED_CLEANUP;
                        ledmap.fault.status = LED_CLEANUP;
                    }
                }
                break;
            case LED_CLEANUP:
                /* simple cleanup */
                ledmap.power.status = LED_OFF;
                ledmap.power.isactive = 0;
                break;
            default:
                break;
        }
#if defined (PLATFORM_HD_RSU_5912)
        /* push button support - logic: check once per second for activiation; if activated, then check
           4x per second just to avoid a user trying to get around 'holding' the button */
        if (!(--pbctr)) {
            pbctr = PUSHBUTTONCNTDWNUNSET;
            #if 0
            /* 20220922: it looks like the pin for the pushbutton is now an IRQ
            and cannot be enabled for polling; TODO: need to confirm if there
            is an IRQ handler that needs to be activeated */
            pbactive = !system(PUSHBUTTONCHECK);
            #else
            pbactive = 0;
            #endif
            if (pbactive) {
                pbctr = PUSHBUTTONCNTDWNSET;
                if (++pbset >= PUSHBUTTONTHRESHOLD) {
                    /* protection against this thread terminating while performing reset (which could lock a user out) */
                    system("shutdown");    /* this will trigger a scheduled shutdown in 60 seconds */
                    /* first stop the main thread from potentially undoing what we are deliberately about
                       to do */
                    platformtype = 0;   /* this will cause the main thread to stop caring about password files */
                    t.tv_sec = 1;
                    nanosleep(&t, NULL);
                    /* sleep(1);*/    /* sleep can be called now because we are going to reboot; allow main thread
                                    to clear out any operation that may have started with platformtype = 1 */
                    /* trigger password reset */
                    PRINTMAX("calling reset of system on password reset button action")   /* log this first */
                    I2V_DBG_LOG(LEVEL_CRIT, MY_NAME,"calling reset of system on password reset button action\n");
                    system("dnpasswd reset");

                    /* reset system after a brief pause */
                    system("sync");
                    t.tv_sec = 1;
                    nanosleep(&t, NULL);
                    /* sleep(1); */
                    system("reboot");   /* overrides shutdown; proceed to shutdown now */
                }
            }
        }
#endif
        realtimecounter++; /* rolling counter */
        #if defined(MY_UNIT_TEST)
        sleep(1);
        break; /* 1 iteration inits, 2nd does some work. */
        #endif
    }

    /* turn off LEDs */
    if (ledmap.power.isactive == 2 || ledmap.fault.isactive >= 2) {
        pthread_cancel(blinkthr);
    }
    system(SYSTEMLEDOFF);    /* all go off */

    pthread_exit(0);
}

/*----------------------------------------------------------------------------*/
/* lampcheck: blinks LEDs once: R(fault) -> A -> G                            */
/*----------------------------------------------------------------------------*/
static void lampcheck(void)
{
    /* don't care about returns, only sleep between on and off */
    system(SYSTEMFAULTON);    /* fault on */
    usleep(BLINKSLEEP * 5);
    system(SYSTEMLEDOFF);    /* fault off */
    system(SYSTEMAMBERON);    /* amber on */
    usleep(BLINKSLEEP * 5);
    system(SYSTEMLEDOFF);    /* amber off */
    system(SYSTEMGREENON);    /* green on */
    usleep(BLINKSLEEP * 5);
    system(SYSTEMLEDOFF);    /* green off */
}

/*----------------------------------------------------------------------------*/
/* ledquery: prints out LED status                                            */
/*----------------------------------------------------------------------------*/
#define NUM_LEDS 3
static void ledquery(pid_t rsuhealthpid, int want_output_in_webgui_format)
{
    union sigval val;
    short sleepcnt = 1000;   /* will result in 1s maximum wait */
    int i;

    if (rsuhealthpid <= 1) {
        /* invalid pid */
        PRINTMAX("Invalid internal input for LED query\n")
        if (want_output_in_webgui_format) {
            for (i=0; i<NUM_LEDS; i++) {
                printf(",");
            }
            printf("\n");
            fflush(stdout);
        }
        return;
    }

    val.sival_int = getpid() | LEDQUERY_REQUEST;
    /* why was signal chosen to exchange data? it's possible the led query may be
       called very infrequently (if ever), so why allocate shared memory or a
       message queue and waste the resource?  the signal can send data and doesn't
       waste resources if this query is never invoked
    */
    if (sigqueue(rsuhealthpid, RSUSIG_LEDQUERY, val)) {
        /* not logged */
        PRINTMAX("Failure querying for LED status\n")
        if (want_output_in_webgui_format) {
            for (i=0; i<NUM_LEDS; i++) {
                printf(",");
            }
            printf("\n");
            fflush(stdout);
        }
        return;
    }

    while (sleepcnt-- && !ledqueryrdy) {
        usleep(1000);    /* sleep 1 ms */
    }

    if (!sleepcnt && !ledqueryrdy) {
        printf("Timeout awaiting LED status!\n");
    }

    if (want_output_in_webgui_format) {
        int led_status_array[NUM_LEDS] = { ledmap.power.status, ledmap.amber.status, ledmap.fault.status };
        for (i=0; i<NUM_LEDS; i++) {
            if      (led_status_array[i] == LED_BLINK) { printf("Blink,"); }
            else if (led_status_array[i] == LED_ON)    { printf("Active,"); }
            else                                       { printf("Off,"); }
        }
        printf("\n");
        return;
    }
    
    switch (ledmap.amber.status) {
        case LED_BLINK:    /* technically unsupported; but print out anyway */
            printf("AMBER LED: Blinking\n");
            break;
        case LED_ON:
            printf("AMBER LED: On\n");
            break;
        case LED_OFF:
            printf("AMBER LED: Off\n");
            break;
        default:
            printf("AMBER LED: Unknown\n");
            break;
    }
    switch (ledmap.fault.status) {
        case LED_BLINK:    /* technically unsupported; but print out anyway */
            printf("FAULT LED: Blinking\n");
            break;
        case LED_ON:
            printf("FAULT LED: On\n");
            break;
        case LED_OFF:
            printf("FAULT LED: Off\n");
            break;
        default:
            printf("FAULT LED: Unknown\n");
            break;
    }
    switch (ledmap.power.status) {
        case LED_BLINK:
            printf("POWER LED: Blinking\n");
            break;
        case LED_ON:
            printf("POWER LED: On\n");
            break;
        case LED_OFF:
            printf("POWER LED: Off\n");
            break;
        default:
            printf("FAULT LED: Unknown\n");
            break;
    }
}

/*----------------------------------------------------------------------------*/
/* utilityquery: issues a generic request to service rsuhealth                */
/*      (see ledquery if future functions require waiting for a response)     */
/*----------------------------------------------------------------------------*/
static unsigned char utilityquery(pid_t rsuhealthpid, unsigned char requesttype)
{
    union sigval val;

    if (rsuhealthpid <= 1) {
        /* invalid pid */
        PRINTMAX("Invalid internal input for utility query\n")
        return 1;
    }

    val.sival_int = getpid() | UTILITY_REQUEST | (int)(requesttype << 23);

    if (sigqueue(rsuhealthpid, RSUSIG_UTIL, val)) {
        /* not logged */
        PRINTMAX("Failure querying for LED status\n")
        return 1;
    }

    return 0;
}
//
//TODO: Do NOT mix terminus options at same time you want daemon.
//    : If you want a feature on or off using terminus then do after daemon launched.
/*----------------------------------------------------------------------------*/
/* parseargs: handles input args and performs select actions                  */
/*     supported args:                                                        */
/*                    -a    antenna check [input vals]:                       */
/*                              cv2x on  (-acon)                              */
/*                              cv2x off (-acoff)                             */ 
/*                    -c    get_cv2x_status from RSUHEALTH SHM                */
/*                    -D    allow degraded operation (no input)               */ 
/*                    -d    enable logging (no input)                         */ 
/*                    -f    print list of faults detected                     */
/*                    -I    i2v control [input vals]:                         */ 
/*                              i  (-Ii)   ignore i2v (GUI only)              */ 
/*                              t  (-It)   terminate i2v                      */ 
/*                              r  (-Ir)   resume/restart i2v                 */ 
/*                    -i    force IP check (no input), good for IP changes    */ 
/*                    -L    led query [input vals]:                           */ 
/*                              w  (-Lw)   GUI ouput format (GUI only)        */ 
/*                              anything else: standard output format         */ 
/*                    -l    led control [input vals]:                         */ 
/*                              XX  (-lXX)  set LED value where 'XX' is:      */
/*                                  first character is LED                    */
/*                                  'a'mber, 'f'ault (red), 'p'ower (green)   */
/*                                  second character is action                */
/*                                  'b'link, 's'olid, 'o'ff                   */
/*                                  example: -lab   blink amber LED           */
/*                    -p    reset rsuuser pwd and reboot rsu                  */
/*                    -S    state control/query [input vals]:                 */ 
/*                              lts  (-Slts)   LED test (GUI) starting        */ 
/*                              ltc  (-Sltc)   LED test (GUI) complete        */ 
/*                              go   (-Sgo)    System ready to go after boot  */ 
/*                              q    (-Sq)     State query                    */
/*                    -t    temperature Celcius [input vals]:                 */
/*                              cv2x (-tcv2x)                                 */
/*                              hsm  (-thsm)                                  */ 
/*                                                                            */
/*----------------------------------------------------------------------------*/
//NOTE: I2V_DBG_LOG is not available yet.
static int32_t parseargs(int argc, char *argv[])
{
    signed char c;
    FILE *pf;
    char allowdegraded = 0, buf[100];    /* buf used for cmd and result */
    pid_t pid = -1;
#if !defined(MY_UNIT_TEST)
    struct stat st;
#endif
    union sigval val;
    int ret;
    unsigned char sleepcnt = 100;

    memset(&val, 0, sizeof(val));
    snprintf(buf, sizeof(buf), "pidof -s rsuhealth -o %d", getpid());
    if (NULL != (pf = popen(buf, "r"))) {
        memset(buf, 0, sizeof(buf));
        fgets(buf, sizeof(char)*sizeof(buf), pf);
        pid = (pid_t)strtoul(buf, NULL, 10);
        pclose(pf);
    } else {
        PRINTMAX("Internal error attempting to determinte state\n")
        return 1;
    }
#if defined(MY_UNIT_TEST)
    optind = 0; /* If calling more than once you must reset to zero for getopt.*/
#endif
    while ((c = getopt(argc, argv, "a:cDdpfI:iL:l:S:t:")) != -1) {
        switch (c) {
            case 'a':
                if ((shm_rsuhealth_ptr = wsu_share_init(sizeof(rsuhealth_t), RSUHEALTH_SHM_PATH)) == NULL) {
                    printf("%s|SHM init failed.\n",MY_NAME);
                    shm_rsuhealth_ptr = NULL; /* Keep going, not fatal. */
                }
                if(NULL != shm_rsuhealth_ptr) { 
                    if (!strcmp(optarg, "con")) {
                        if(WTRUE == wsu_shmlock_lockw(&shm_rsuhealth_ptr->h.ch_lock)) {
                            shm_rsuhealth_ptr->cv2xruntime |= RSUHEALTH_CV2X_ANTENNA_CHECK;
                            if(WFALSE == wsu_shmlock_unlockw(&shm_rsuhealth_ptr->h.ch_lock)) {
                                printf("wsu_shmlock_unlockw() failed.\n");
                            }
                        } else {
                            printf("wsu_shmlock_lockw() failed.\n");
                        }
                    } else if (!strcmp(optarg, "coff")) {
                        if(WTRUE == wsu_shmlock_lockw(&shm_rsuhealth_ptr->h.ch_lock)) {
                            shm_rsuhealth_ptr->cv2xruntime &= ~RSUHEALTH_CV2X_ANTENNA_CHECK;
                            if(WFALSE == wsu_shmlock_unlockw(&shm_rsuhealth_ptr->h.ch_lock)) {
                                printf("wsu_shmlock_unlockw() failed.\n");
                            }
                        } else {
                            printf("wsu_shmlock_lockw() failed.\n");
                        }
                    }
                }
                /* Close SHM. Don't destroy. Recover on start up.*/
                if(NULL != shm_rsuhealth_ptr) {
                    wsu_share_kill(shm_rsuhealth_ptr, sizeof(rsuhealth_t));
                    shm_rsuhealth_ptr = NULL;
                }
                return 1;/* this option is a terminus option */
                break;
            case 'c': /* get_cv2x_status from RSUHEALTH SHM. */
              {
                bool_t tx_ready = WFALSE;
                bool_t rx_ready = WFALSE;
                if ((shm_rsuhealth_ptr = wsu_share_init(sizeof(rsuhealth_t), RSUHEALTH_SHM_PATH)) == NULL) {
                    printf("%s|SHM init failed.\n",MY_NAME);
                    shm_rsuhealth_ptr = NULL; /* Keep going, not fatal. */
                }
                if(NULL != shm_rsuhealth_ptr) { 
                    if(WTRUE == wsu_shmlock_lockr(&shm_rsuhealth_ptr->h.ch_lock)) {
                        if (WTRUE == shm_rsuhealth_ptr->h.ch_data_valid) {
                            if(RSUHEALTH_CV2X_TX_READY & shm_rsuhealth_ptr->cv2xstatus) {
                                tx_ready = WTRUE;
                            }
                            if(RSUHEALTH_CV2X_RX_READY & shm_rsuhealth_ptr->cv2xstatus) {
                                rx_ready = WTRUE;
                            }
                        }
                        if(WFALSE == wsu_shmlock_unlockr(&shm_rsuhealth_ptr->h.ch_lock)) {
                            printf("%s|wsu_shmlock_unlockr() failed.\n",MY_NAME);
                        }
                    } else {
                        printf("%s|wsu_shmlock_lockr() failed.\n",MY_NAME);
                    }
                    /* Close SHM. Don't destroy. Recover on start up.*/
                    if(NULL != shm_rsuhealth_ptr) {
                       wsu_share_kill(shm_rsuhealth_ptr, sizeof(rsuhealth_t));
                       shm_rsuhealth_ptr = NULL;
                    }
                }

                printf("RSU-5940 CV2X Status:\n");
                if(tx_ready) {
                    printf("  TX: Active\n");
                } else {
                    printf("  TX: Inactive\n");
                }
                if(rx_ready) {
                    printf("  RX: Active\n");
                } else {
                    printf("  RX: Inactive\n");
                }
                /* this option is a terminus option */
                return 1;
              }
              break;
            case 'D':    /* allow degraded */
                allowdegraded = 1;
                break;
            case 'd':
                logfile = 1;
                break;
            case 'f':
                if ((shm_leddiag_ptr = wsu_share_init(sizeof(led_diagnostics_t), FAULTLED_SHM_PATH)) == NULL) {
                    printf("LED SHM init failed. \n");
                    shm_leddiag_ptr = NULL;
                }
                if (NULL != shm_leddiag_ptr) {
                    if (WTRUE == wsu_shmlock_lockr(&shm_leddiag_ptr->h.ch_lock)){
                        if (WTRUE == shm_leddiag_ptr->h.ch_data_valid) {
                            printf("Fault LED subsystem errors detected: \n");
                            if (shm_leddiag_ptr->faultledstate < 1) {
                                printf("No errors. \n");
                            } else {
                                if (shm_leddiag_ptr->gps_fix.error_level > 0){
                                    printf("%s error.\n", shm_leddiag_ptr->gps_fix.name);
                                }
                                if (shm_leddiag_ptr->time_accuracy.error_level > 0){
                                    printf("%s error.\n", shm_leddiag_ptr->time_accuracy.name);
                                }
                                if (shm_leddiag_ptr->gps_position.error_level > 0){
                                    printf("%s error.\n", shm_leddiag_ptr->gps_position.name);
                                }
                                if (shm_leddiag_ptr->gnss_antenna.error_level > 0){
                                    printf("%s error.\n", shm_leddiag_ptr->gnss_antenna.name);
                                }
                                if (shm_leddiag_ptr->cv2x_antenna.error_level > 0){
                                    printf("%s error.\n", shm_leddiag_ptr->cv2x_antenna.name);
                                }
                                if (shm_leddiag_ptr->battery_voltage.error_level > 0){
                                    printf("%s error.\n", shm_leddiag_ptr->battery_voltage.name);
                                }
                                if (shm_leddiag_ptr->network.error_level > 0){
                                    printf("%s error.\n", shm_leddiag_ptr->network.name);
                                }
                                if (shm_leddiag_ptr->temperature.error_level > 0){
                                    printf("%s error.\n", shm_leddiag_ptr->temperature.name);
                                }
                                if (shm_leddiag_ptr->hsm.error_level > 0){
                                    printf("%s error.\n", shm_leddiag_ptr->hsm.name);
                                }
                                if (shm_leddiag_ptr->cv2x_interface.error_level > 0){
                                    printf("%s error.\n", shm_leddiag_ptr->cv2x_interface.name);
                                }
                            }
                        }
                        if (WFALSE == wsu_shmlock_unlockr(&shm_leddiag_ptr->h.ch_lock)) {
                            printf("wsu_shmlock_unlockr failed. \n");
                        }
                    } else {
                        printf("wsu_shmlock_lockr failed. \n");
                    }
                } else {
                    printf("no LED SHM. \n");
                }

                /* this option is a terminus option */
                return 1;
            case 'I':    /* terminate/start i2v */
                /* print errors in this block do not get logged to a file */
                /* the following could be condensed, but the prints should be unique */
                if (!strcmp("i", optarg)) {
                    /* ignore i2v - primary use for GUI; i2v may be terminated externally in this state */
                    /* minimal validation */
                    if (pid > 1) {
                        /* realistically, it should be greater than 1000 but no real way to tell;
                           MUST be greater than init which is 1 */
                        val.sival_int = 0;
                        if (sigqueue(pid, RSUSIG_TERMI2V, val)) {
                            /* error received; this error isn't logged in a file if enabled */
                            PRINTMAX("Error attempting to ignore i2v: %d\n", errno)
                        }
                    } else {
                        PRINTMAX("Did not find health service; cannot ignore i2v\n")
                    }
                } else if (!strcmp("t", optarg)) {
                    /* terminate i2v */
                    /* minimal validation */
                    if (pid > 1) {
                        /* realistically, it should be greater than 1000 but no real way to tell;
                           MUST be greater than init which is 1 */
                        val.sival_int = TERMINATE;
                        if (sigqueue(pid, RSUSIG_TERMI2V, val)) {
                            /* error received; this error isn't logged in a file if enabled */
                            PRINTMAX("Error attempting to terminate i2v: %d\n", errno)
                        }
                    } else {
                        PRINTMAX("Did not find health service; cannot terminate i2v\n")
                    }
                } else if (!strcmp("r", optarg)) {
                    /* resume i2v */
                    /* minimal validation */
                    if (pid > 1) {
                        /* realistically, it should be greater than 1000 but no real way to tell;
                           MUST be greater than init which is 1 */
                        if (sigqueue(pid, RSUSIG_RESTOREI2V, val)) {    /* val is a don't care */
                            /* error received; this error isn't logged in a file if enabled */
                            PRINTMAX("Error attempting to restart i2v: %d\n", errno)
                        }
                    } else {
                        PRINTMAX("Did not find health service; cannot restart i2v\n")
                    }
                }

                /* this option is a terminus option */
                return 1;
            case 'i':    /* force IP check/change IP */
                /* this option is a terminus option */
                utilityquery(pid, RSUH_IPCHECK);
                return 1;
            case 'L':    /* LED state query */
                /* this option is a terminus option */
                ledquery(pid, !strcmp("w", optarg));
                return 1;
            case 'l':    /* LED management */
                /* print errors in this block do not get logged to a file */
                /* only care about the first character to identify LED; action is second */
                switch (optarg[0]) {
                    case 'a':    /* amber */
                        val.sival_int = LED_AMBER;
                        break;
                    case 'f':    /* fault */
                        val.sival_int = LED_FAULT;
                        break;
                    case 'p':    /* power */
                        val.sival_int = LED_POWER;
                        break;
                    case 'l':    /* all */
                        val.sival_int = LED_ALL;
                        break;
                    default:
                        PRINTMAX("Invalid led request\n")
                        return 1;
                }
                /* action */
                switch(optarg[1]) {
                    case 'b':    /* blink */
                        val.sival_int |= LED_BLINK;
                        break;
                    case 'f': /* blink fast */
                        val.sival_int |= LED_BLINKFAST;
                    case 's':    /* solid */
                        val.sival_int |= LED_ON;
                        break;
                    case 'o':    /* off */
                        val.sival_int |= LED_OFF;
                        break;
                    case 'r':   /* revert */
                        val.sival_int |= LED_REVERT;
                        break;
                    default:
                        PRINTMAX("Invalid led request\n")
                        return 1;
                }

                if (val.sival_int) {
                    if (sigqueue(pid, RSUSIG_LEDMGMT, val)) {
                        /* error received; this error isn't logged in a file if enabled */
                        PRINTMAX("Error attempting to adjust led: %d\n", errno)
                    }
                }

                /* this option is a terminus option */
                return 1;
            case 'p': /* No return from this call. Reboots RSU */
                reset_rsuuser_pwd();
                return 1;
            case 'S':    /* state */
                if (!strcmp(optarg, "lts")) {    /* LED test starting; ignore LED override */
                    val.sival_int = RSUHSTATE_LEDTESTSTART;
                    if (sigqueue(pid, RSUSIG_STATE, val)) {
                        /* error received; this error isn't logged in a file if enabled */
                        PRINTMAX("Error attempting to ignore LED state: %d\n", errno)
                    }
                } else if (!strcmp(optarg, "ltc")) {    /* LED test starting; ignore LED override */
                    val.sival_int = RSUHSTATE_LEDTESTEND;
                    if (sigqueue(pid, RSUSIG_STATE, val)) {
                        /* error received; this error isn't logged in a file if enabled */
                        PRINTMAX("Error attempting to ignore LED state: %d\n", errno)
                    }
                } else if (!strcmp(optarg, "go")) {    /* system is ready to go, startup complete */
                    val.sival_int = RSUHSTATE_NORMAL;
                    if (sigqueue(pid, RSUSIG_STATE, val)) {
                        /* error received; this error isn't logged in a file if enabled */
                        PRINTMAX("Failure setting system state normal\n")
                    }
                } else if (!strcmp(optarg, "q")) {    /* state query */
                    val.sival_int = RSUHSTATE_QUERY | getpid();
                    if (sigqueue(pid, RSUSIG_STATE, val)) {
                        /* error received; this error isn't logged in a file if enabled */
                        PRINTMAX("Failure querying system state\n")
                    } else {
                        while (sleepcnt-- && !statequeryrdy) {
                            usleep(1000);    /* sleep 1 ms */
                        }

                        if (!sleepcnt && !statequeryrdy) {
                            printf("Timeout awaiting state status!\n");
                        } else {
                            printf("System state: %d\n", rsusystem);
                        }
                    }
                }

                /* this option is a terminus option */
                return 1;
                break;
            case 't':
                if (!strcmp(optarg, "cv2x")) {
                    int32_t cv2xtemp = -99;
                    int32_t pmictemp = 0;
                    if ((shm_rsuhealth_ptr = wsu_share_init(sizeof(rsuhealth_t), RSUHEALTH_SHM_PATH)) == NULL) {
                        printf("%s|SHM init failed.\n",MY_NAME);
                        shm_rsuhealth_ptr = NULL; /* Keep going, not fatal. */
                    } else {
                    }
                    if(NULL != shm_rsuhealth_ptr) { 
                        if(WTRUE == wsu_shmlock_lockr(&shm_rsuhealth_ptr->h.ch_lock)) {
                            if (WTRUE == shm_rsuhealth_ptr->h.ch_data_valid) {
                                cv2xtemp = shm_rsuhealth_ptr->temperature.cv2x;
                            }
                            if(WFALSE == wsu_shmlock_unlockr(&shm_rsuhealth_ptr->h.ch_lock)) {
                                printf("%s|wsu_shmlock_unlockr() failed.\n",MY_NAME);
                            }
                        } else {
                            printf("%s|wsu_shmlock_lockr() failed.\n",MY_NAME);
                        }
                        /* Close SHM. Don't destroy. Recover on start up.*/
                        if(NULL != shm_rsuhealth_ptr) {
                           wsu_share_kill(shm_rsuhealth_ptr, sizeof(rsuhealth_t));
                           shm_rsuhealth_ptr = NULL;
                        }
                    }
                    /* WEB GUI expects cv2x and pmic temps comma sperated */
                    printf("%d,%d,\n",cv2xtemp,pmictemp);   //echo "69,66," > /tmp/zcache.radio.$CACHE_B_NUM.cv2x_temps
                } else if (!strcmp(optarg, "hsm")) {
                    float64_t hsmtemp = -99.99;
                    if ((shm_rsuhealth_ptr = wsu_share_init(sizeof(rsuhealth_t), RSUHEALTH_SHM_PATH)) == NULL) {
                        printf("%s|SHM init failed.\n",MY_NAME);
                        shm_rsuhealth_ptr = NULL; /* Keep going, not fatal. */
                    }
                    if(NULL != shm_rsuhealth_ptr) { 
                        if(WTRUE == wsu_shmlock_lockr(&shm_rsuhealth_ptr->h.ch_lock)) {
                            if (WTRUE == shm_rsuhealth_ptr->h.ch_data_valid) {
                                hsmtemp = shm_rsuhealth_ptr->temperature.hsm;
                            }
                            if(WFALSE == wsu_shmlock_unlockr(&shm_rsuhealth_ptr->h.ch_lock)) {
                                printf("%s|wsu_shmlock_unlockr() failed.\n",MY_NAME);
                            }
                        } else {
                            printf("%s|wsu_shmlock_lockr() failed.\n",MY_NAME);
                        }
                        /* Close SHM. Don't destroy. Recover on start up.*/
                        if(NULL != shm_rsuhealth_ptr) {
                           wsu_share_kill(shm_rsuhealth_ptr, sizeof(rsuhealth_t));
                           shm_rsuhealth_ptr = NULL;
                        }
                    }
                    /* WEB GUI expects hsm temp comma sperated */
                    printf("%3.2lf,\n",hsmtemp);    
                }
                return 1;/* this option is a terminus option */
                break;
            default:
                break;
        }
    }

    /* let's make sure an rsuhealth is not already running */
    if (pid > 1) {
        PRINTMAX("Another rsuhealth is running; do not attempt to run two instances...\n")
        return 1;
    }

    if (logfile) {
#if !defined(MY_UNIT_TEST)
        memset(&st, 0, sizeof(st));
        if (!stat(RSULOG, &st)) {
            /* file already exists, getting this far means no other user should be touching file;
               start up a new log */
            system("mv -f " RSULOG " " RSULOGOLD);
        }
        if (NULL == (hlthfp = fopen(RSULOG, "w+"))) {
            PRINTMAX("Error attempting to create health log; no log generated; restart if needed\n")
        }
#endif
    }
    lampcheck();
    /* now LED initialization can take place */
    memset(&ledmap, 0, sizeof(ledmap));
    ledmap.amber.status = LED_OFF;
    ledmap.fault.status = LED_OFF;
    ledmap.power.status = LED_OFF;

    /* create 'real time' thread */
    ret = pthread_create(&rtthread, NULL, &realtimeHealthThrd, NULL);
    if (ret) {
        PRINTMAX("Error creating real time thread: %d\n", ret)
        if (allowdegraded) {
            PRINTMAX("\n%s| Allowing degraded mode; no LED or i2v termination support\n",MY_NAME)
        } else {
            PRINTMAX("\n%s| Fatal error: realtimeHealthThrd failed. Exiting.\n",MY_NAME)
            return 1;
        }
    }

    /* attempt to start blinking */
    val.sival_int = LED_POWER | LED_BLINK;
    if (sigqueue(getpid(), RSUSIG_LEDMGMT, val)) {
        PRINTMAX("Failure starting LED blink\n")
    }
    return 0;
}

/*----------------------------------------------------------------------------*/
/* ipsetting: sets the IP (dhcp or static) and invokes utility for            */
/* applying routes                                                            */
/*                                                                            */
/* sets global ipgood if IP applied; exits quickly if ipgood already set      */
/*----------------------------------------------------------------------------*/
static void ipsetting(void)
{
#if !defined(MY_UNIT_TEST)
    unsigned char dhcpactive = 0;
    FILE *pf;
    char buf[MAXLINE] = {0}, cmd[BIGLINE] = {0};
    struct stat st;

    if (ipgood) {
        return;
    }

    memset(&st, 0, sizeof(st));
    if (stat(STARTUP, &st)) {
        /* could be file doesn't yet exist */
        return;
    }

    if (NULL != (pf = popen("cat " STARTUP " | grep ^DHCP_ENABLE | sed 's/.*=\\(.*\\)/\\1/'", "r"))) {
        fgets(buf, sizeof(char)*MAXLINE, pf);
        pclose(pf);
        if (!strncmp(buf, "TRUE", sizeof("TRUE") - 1)) {
            /* dhcp active */
            dhcpactive = 1;
        }
    } else {
        PRINTMAX("failed to evaluate IP settings; will retry later\n")
        return;
    }
    usingdhcp = dhcpactive;   /* set the global flag since the local variable is repurposed if static */

    /* in select error scenarios, the SYSETHFILE may already be bound to BINDETHFILE; attempt to unmount */
    system("umount " SYSETHFILE " >/dev/null 2>&1");

    if (system("cp -f " SYSETHFILE " " BINDETHFILE)) {
        PRINTMAX("failed to prepare IP settings; will retry later\n")
        return;
    }

    if (dhcpactive) {
        if (system("sed -i '/Address/d' " BINDETHFILE)) {
            PRINTMAX("failed to prepare dhcp enablement; will retry later\n")
            return;
        }
        if (system("sed -i 's/.*\\(DHCP.*\\)/\\1/' " BINDETHFILE)) {
            PRINTMAX("failed to finish dhcp enablement; will retry later\n")
            return;
        }
    } else {
        /* static IP */
        if (NULL == (pf = popen("cat " STARTUP " | grep ^IP_FOR_STATIC | sed s'/.*=\\(.*\\)/\\1/'", "r"))) {
            PRINTMAX("failed to prepare static ip enablement; will retry later\n")
            return;
        }
        fgets(buf, sizeof(char)*MAXLINE, pf);
        pclose(pf);
        /* eliminate any new lines; repurpose dhcpactive */
        dhcpactive = strlen(buf);
        if (dhcpactive && (buf[--dhcpactive] == '\n')) {
            buf[dhcpactive] = 0;
        }
        if (strlen(buf) < MINSTATICIPLEN) {
            PRINTMAX("invalid IP obtained from " STARTUP "; will retry later\n")
            return;
        }
        /* deliberately removing netmask */
        sprintf(cmd, "sed -i 's/\\(Address=\\).*\\(\\/.*\\)/\\1%s/' %s", buf, BINDETHFILE);
        /* PRINTMAX(cmd) */ /* debug support */
        if (system(cmd)) {
            PRINTMAX("failed to finish static ip enablement; will retry later\n")
            return;
        }
        /* netmask */
        if (NULL == (pf = popen("cat " STARTUP " | grep ^NETMASK_FOR_S | sed s'/.*=\\(.*\\)/\\1/'", "r"))) {
            PRINTMAX("failed to obtain static mask enablement; will retry later\n")
            return;
        }
        fgets(buf, sizeof(char)*MAXLINE, pf);
        pclose(pf);
        /* eliminate any new lines; repurpose dhcpactive */
        dhcpactive = strlen(buf);
        if (dhcpactive && (buf[--dhcpactive] == '\n')) {
            buf[dhcpactive] = 0;
        }
        /* desired netmask */
        sprintf(cmd, "sed -i 's/\\(Address=.*\\)/\\1\\/%s/' %s", buf, BINDETHFILE);
        /* PRINTMAX(cmd) */ /* debug support */
        if (system(cmd)) {
            PRINTMAX("failed to finish final static ip enablement; will retry later\n")
            return;
        }
    }

    /* now time to mount */
    if (system("mount -o bind " BINDETHFILE " " SYSETHFILE)) {
        PRINTMAX("failed to bind ip enablement; will retry later\n")
        return;
    }
    /* check to confirm the interface exists; do not try to set it if it doesn't */
    if (system("ip link | grep " SYSIFC)) {
        /* device not found; do not proceed with anything further */
        PRINTMAX("failed to find ip interface " SYSIFC "; will retry later\n")
        return;
    }
    /* start and stop ifc and stack */
    system("ifconfig " SYSIFC " down");   /* do not fail; the systemctl can proceed without this */
    if (system("systemctl restart systemd-networkd")) {
        /* no need to call ifconfig up because the restart failed; there's no ip */
        PRINTMAX("failed to bind ip enablement; will retry later\n")
        return;
    }
    system("ifconfig " SYSIFC " up");   /* do not fail; the systemctl can proceed without this */

    ipgood = 1;

    /* apply routing rules */
   if (system("dnnetcfg")) {
       PRINTMAX("failed to apply routing rules\n")
   }

   if (restartgui) {    /* no vlan support needed here */
       remove(GUIRESTARTFILE);    /* this may have old info now */
       if (prepareguiscript()) {
           PRINTMAX("failed to restore restart GUI mechanism; system reboot required!\n")
       }
       if (!system("ps -ef | grep bozo >/dev/null 2>&1")) {
           /* terminate the GUI and other function will restart it */
           if (system("killall -15 bozohttpd >/dev/null 2>&1")) {
               PRINTMAX("failed to enable restart of GUI after IP change\n")
           } else {
               restartgui = 0;
           }
       } else {
           restartgui = 0;
       }
   }
#endif
}

/*----------------------------------------------------------------------------*/
/* checkip:  detects if IP is correct and saves settings to NOR for future    */
/* use (additional function to be added); if a mismatch detected, calls       */
/* ipsetting to update current value                                          */
/*----------------------------------------------------------------------------*/
static void checkip(void)
{
#define PINGCNT 5

    struct stat st;
    char cmd[BIGLINE] = {0};
    static unsigned char pingcnt = 1;    /* start at 1 so first iteration invokes ping (max 1 min user has to wait) */

    memset(&st, 0, sizeof(stat));

    /* see runtime IP in startup.sh matches NOR partition value (ok, the whole file for simplicity) */
    if (stat(STARTUP, &st)) {
        PRINTMAX("failed obtaining network file details\n")
        return;
    }

    /* sadly need to have exact byte size so have to use byte size of 1 */
    sprintf(cmd, "dd if=/dev/mtd8 of=" TMPRDFILE " bs=1 skip=262144 count=%d >>/dev/null 2>&1", (int)(st.st_size));
    if (system(cmd)) {
        PRINTMAX("failed collecting persistent network file details\n")
        return;
    }

    if (system("diff " TMPRDFILE " " STARTUP " >>/dev/null")) {
        /* differences */
        if (system("flash_erase /dev/mtd8 0x40000 1 >> /dev/null 2>&1")) {
            PRINTMAX("unable to prepare for save of persistent network file\n")
            remove(TMPRDFILE);
            return;
        }
        if (system("dd if=" STARTUP " of=/dev/mtd8 bs=1024 seek=256 >> /dev/null 2>&1")) {
            PRINTMAX("unable to save persistent network file\n")
        }
        PRINTMAX("saved persistent network file\n")
    }
    remove(TMPRDFILE);

    /* now check if the current IP matches the BINDETHFILE (not startup); only if not using dhcp (exclude ping as well) */
    if (!usingdhcp) {
        if(system("ip -4 addr | grep `cat " BINDETHFILE " | grep Address | sed 's/.*=\\(.*\\)/\\1/'` >>/dev/null")) {
            /* the above will compare the current ip settings against the Address= in the systemd network file;
               reaching this point means there was no match found; reset ip, first clear ipgood */
            PRINTMAX("Warning: current IP setting not matching expected value; will retry to apply settings\n")
            ipgood = 0;
            ipsetting();
        } else {
            /* perform periodic ping; roughly once per 5 minutes is fine; this runs approximately 1 per min, hence countdown is 5 */
            if (!(--pingcnt)) {
                system("ping -c 1 `cat " BINDETHFILE " | grep Address | sed 's/.*=\\(.*\\)\\/.*/\\1/'` >>/dev/null");
                pingcnt = PINGCNT;
            }
        }
    }

    /* check on gateway */
    system("dnnetcfg " GWRST " >>/dev/null");    /* return is don't care */

#undef PINGCNT
}

/*----------------------------------------------------------------------------*/
/* checkgui: attempts restart of GUI if system has booted and gui not up      */
/*----------------------------------------------------------------------------*/
static void checkgui(void)
{
   struct stat st;

   memset(&st, 0, sizeof(st));
   if (stat(GUIRESTARTFILE, &st)) {
       if (prepareguiscript()) {
           return;    /* proceed no further */
       }
   }
   if (stat(VGUIMANAGEFILE, &st)) {
       if (prepvguiscript()) {
           return;    /* proceed no further */
       }    /* it may return 'success' but still no file exists if vlan not enabled */
   }
   /* gui check */
   if (ipgood && (rsusystem != RSUHSTATE_UNKNOWN)) {
       /* don't care about return */
       system("if [[ -z \"`pidof bozohttpd`\" ]]; then cd /home/root; source /etc/profile; " GUIRESTARTFILE " >>/dev/null 2>&1; fi");
       /* vlan support; if the file exists, launch */
       system("if [[ -e " VGUIMANAGEFILE " ]]; then vip=`cat " VGUIMANAGEFILE " | grep ^IP_FOR | tr '=' ' ' | awk '{print $NF}'`; if [[ -z \"`ps -ef | grep bozo | grep $vip`\" ]]; then cd /home/root; source /etc/profile; " VGUIMANAGEFILE " >>/dev/null 2>&1; fi; fi");
   }
}
#if defined(PLATFORM_HD_RSU_5912)
/*----------------------------------------------------------------------------*/
/* preparecv2x:  applies configuration settings to enable cv2x to work        */
/*----------------------------------------------------------------------------*/
static void preparecv2x(void)
{
#if !defined(MY_UNIT_TEST)
    unsigned char count = 10;    /* initial count set for 10 iterations; see below */

    FILE *pf;
    char buf[MAXLINE] = {0};
    struct stat st;
    unsigned char cv2xready = 0;
    struct timespec t;

    memset(&t, 0, sizeof(t));

    /* TODO: should health wait for cv2xready or should cv2xcfg do that?
       short term it makes more sense for cv2xcfg to do it, but long-term there
       may be additional stuff we stick here that may have dependency on the
       radio; i.e. applying cv2x firmware if we figure out a way; leave for now */
    while (!cv2xready && count) {
        if (NULL != (pf = popen("systemctl is-active cv2x", "r"))) {
            fgets(buf, sizeof(char)*MAXLINE, pf);
            pclose(pf);
            if (!strncmp(buf, "active", sizeof("active") - 1)) {   /* the -1 removes null term */
                PRINTMAX("Health waiting for cv2x\n")
                t.tv_sec = SLEEPSEC;
                nanosleep(&t, NULL);
               // sleep(SLEEPSEC);
                count--;
            } else {
                cv2xready = 1;
            }
        } else {
            PRINTMAX("Health ERROR: unable to check for cv2x active\n")
            t.tv_sec = SLEEPSEC;
            nanosleep(&t, NULL);
            //sleep(SLEEPSEC);
            count--;
        }
    }

    /* attempt to apply cv2xcfg; need rwflash/configs present or else it will just return OK 
       the problem in this case is that if rwflash/configs not present, there's no way to know 
       if cv2x radio mode is active and thus no way to decide if cv2xcfg should wait forever */

    memset(&st, 0, sizeof(st));
    /* wait a maximum of 10 seconds, using startup.sh as indication whether rwflash/configs ready */
    count = 10;   /* reset count in case used */
    while (stat(STARTUP, &st) && count) {
        t.tv_sec = SLEEPSEC;
        nanosleep(&t, NULL);
        //sleep(SLEEPSEC);
        count--;
    }

    /* results in a delay for allowing cv2xcfg to execute; if after 5
       seconds (technically six) there's still a problem, it won't get
       fixed by waiting any longer */
    count = 5;   /* reset to new count */
    while (system("cv2xcfg") && count) {
        printf("Health ERROR: unable to apply cv2x configuration (%d)\n", count);
        count--;
        t.tv_sec = SLEEPSEC;
        nanosleep(&t, NULL);
        //sleep(SLEEPSEC);
    }
#endif
}
#endif /* PLATFORM_HD_RSU_5912 */
/*----------------------------------------------------------------------------*/
/* preparecustom: dos2unix customer files                                     */ 
/*----------------------------------------------------------------------------*/
static void preparecustom(void)
{
#if !defined(MY_UNIT_TEST)
    system("for i in `ls /rwflash/customer`; do dos2unix /rwflash/customer/$i >/dev/null 2>&1; done");
    /* this next one is really overkill; amh will kill any crappy file; but just in case */
    system("for i in `ls /rwflash/I2V/amh`; do dos2unix /rwflash/I2V/amh/$i >/dev/null 2>&1; done");
    /* any future customer folders go here, maybe security? */
#endif
}


/*----------------------------------------------------------------------------*/
/* preparecheckpassword: set up user editable password files                  */
/*----------------------------------------------------------------------------*/
static void preparecheckpassword(void)
{
#if !defined(MY_UNIT_TEST)
    struct stat st;
    char calldependentfxns = 0;
    struct timespec t;

    memset(&t, 0, sizeof(t));

    /* check platform/build type */
    if (platformtype < 0) {
        if (system("cat /usr/share/BuildInfo.txt | grep Target | grep Customer >>/dev/null")) {
            /* either the BuildInfo.txt doesn't exist (f-ed build) or not a customer image */
            platformtype = 0;
        } else {
            /* customer image */
            platformtype = 1;
        }
    } 
    
    memset(&st, 0, sizeof(st));

    if (!rwflashrdy) {
        if (stat(STARTUP, &st)) {
            /* rwflash must be available */
            return;
        }
        rwflashrdy = 1;    /* only set by this function; any dependent functions need
                              to be called after this function */
        calldependentfxns = 1;

        /* relocated here from prior execution; do not overlook dependent functions! */
        if (!platformtype) {
            /* nothing to do; no password management except on customer build; doing
               anything on the other builds creates cleanup requirements before
               installing a customer image */
            system("[ ! -e /tmp/passwordplatform ] && echo not a supported build for password > /tmp/passwordplatform");
            RWFLASHDEPENDENTFXNS
            return;
        }
    } else if (!platformtype) {
        /* need rwflashdependentfxns to boot correctly, but now exit whenever this function is called to prevent password setup */
        system("[ ! -e /tmp/passwordplatform ] && echo not a supported build for password > /tmp/passwordplatform");
        return;
    }

    if (stat(LOGINPW, &st)) {
        /* no need to check result for this one; next command will fail */
        system("mkdir -p " LOGINFILEDIR);
        /* passwd file doesn't exist */
        if (system("cp " ETCPW " " LOGINPW)) {
            /* failure setting up; will retry when executed again */
            RWFLASHDEPENDENTFXNS
            return;
        }
    }

    if (stat(LOGINSDW, &st)) {
        /* shadow file doesn't exist */
        if (system("cp " ETCSDW" " LOGINSDW)) {
            /* failure setting up; will retry when executed again */
            RWFLASHDEPENDENTFXNS
            return;
        }
    }

    /* reaching here means files exist and can now be bind-mounted */
    if (system("mount | grep " ETCPW " >> /dev/null")) {
        /* bind mount the writeable file; the default hasn't been bound yet */
        if (system("mount -o bind " LOGINPW " " ETCPW " >> /dev/null")) {
            /* don't proceed to shadow; stop here */
            RWFLASHDEPENDENTFXNS
            return;    /* will retry on future attempt */
        }
    }
    if (system("mount | grep " ETCSDW " >> /dev/null")) {
        if (!system("mount -o bind " LOGINSDW " " ETCSDW " >> /dev/null")) {    /* will retry if needed if it fails */
            /* prepare for potential first time changing password if necessary; on first login dnpasswd not called */
            if (!system(SYSTEMCRYPTICPASSWORDCHECK)) {
                system("dnpasswd firstlogin");    /* one shot, if it doesn't work, user can use GUI to reboot */
                firstlogin = 1;
            }
        }
    } else {   /* this branch covers case if user rebooted rsu (GUI) but didn't change password */
        /* prepare for potential first time changing password if necessary; on first login dnpasswd not called */
        if (!firstlogin && !system(SYSTEMCRYPTICPASSWORDCHECK)) {
            system("dnpasswd firstlogin");    /* one shot, if it doesn't work, user can use GUI to reboot */
            firstlogin = 1;
        }
    }

    RWFLASHDEPENDENTFXNS

    /* final check - if we ran firstlogin, need to clean up; yes we will call this right after setting 
       indication that we need to perform firstlogin */
    if (firstlogin && system(SYSTEMCRYPTICPASSWORDCHECK)) {
        /* we set up for first login, but now the password has been changed with the pw check failing */
        system("dnpasswd cleanup");    /* mandatory; this saves the password change */
        PRINTMAX("calling reset on first user password change")
        t.tv_sec = 1;
        nanosleep(&t, NULL);
        //sleep(1);   /* allow a chance for log to write */
        system("reboot");
        firstlogin = 0;    /* meaningless but whatever; dnpasswd cleanup is executed
                              but somehow the system gets /etc in a bad state to enable the first password
                              change; /etc is now missing (it still exists) but the only recovery 
                              is to reboot; the system has the proper password and /etc is back to normal */
    }
#endif
}

/*----------------------------------------------------------------------------*/
/* preparechecksshconf: set up user editable sshd_config                      */
/*----------------------------------------------------------------------------*/
static void preparechecksshconf(void)
{
#if !defined(MY_UNIT_TEST)
    struct stat st;

    memset(&st, 0, sizeof(st));

    if (stat(SSHDCONF, &st)) {
        /* no need to check result for this one; next command will fail */
        system("mkdir -p " SSHFILEDIR);
        /* sshd_config file doesn't exist */
        if (system("cp " ETCSSHDCONF " " SSHDCONF)) {
            /* failure setting up; will retry when executed again */
            return;
        }
    }

    /* reaching here means sshd_config exists and can now be bind-mounted */
    if (system("mount | grep " ETCSSHDCONF " >> /dev/null")) {
        /* bind mount the writeable file; the default hasn't been bound yet */
        if (system("mount -o bind " SSHDCONF " " ETCSSHDCONF " >> /dev/null")) {
            return;    /* will retry on future attempt */
        }
    }
    
#endif
}

/*----------------------------------------------------------------------------*/
/* checki2v:  evaluates i2v alive and restarts if necessary                   */
/*----------------------------------------------------------------------------*/
static void checki2v(void)
{
    char i2vactive;

    /* need to evaluate whether i2v hasn't started yet; rsuhealth starts
       before i2v so don't try to launch i2v until i2v has already been 
       proven running */
    /* check if i2v is running */
    i2vactive = !system("ps -ef | grep i2v_app | grep -v grep >/dev/null");   /* result is 0 if success */

    if (i2vactive || marshali2v) {
        localrsuhealth.errorstates &= ~(uint64_t)RSUHEALTH_I2V_RESET;
        i2vfailcnt = 0;
        /* either i2v is running or rsuhealth has altered i2v state; both mean don't start i2v */
        return;
    }

    i2vfailcnt++;    /* future support for possible system reboot or maybe configs reset */

    /* i2v is inactive - launch i2v */
    /* this may change in the future */
    PRINTMAX("RSU services inactive; restarting\n")
    system("i2v_app &");
    localrsuhealth.errorstates |= RSUHEALTH_I2V_RESET;
}

/*----------------------------------------------------------------------------*/
/* checkled: ok, not really a 'check'; reset led to an on state               */
/*----------------------------------------------------------------------------*/
static void checkled(void)
{
    if (ledoverride && --ledoverridectr) {
        /* do not do anything */
        return;
    }
    /* JJG: maybe you should have a mutex to protect?; do it stupid i mean simple now */
    /* rather than check if set, just set; reaching here means the ledoverridectr is now 0 */
    ledoverride = 0;

    /* only going to worry about setting an led on; the GUI can activate
       an LED test that hoses the state; this will reset the physical leds */
    if (ledmap.currentstate & LED_ON) {
        switch (ledmap.currentstate & IGNORE_ACTION) {
            case LED_AMBER:
                system(SYSTEMAMBERON);
                break;
            case LED_FAULT:
                system(SYSTEMFAULTON);
                break;
            case LED_POWER:
                system(SYSTEMGREENON);
                break;
            default:
                localrsuhealth.errorstates |= RSUHEALTH_BOGUS_LED_STATE;
                break;
        }
    }
}

/*----------------------------------------------------------------------------*/
/* prepareguiscript: I'm sorry this maybe so fugly, but it has to be this way */
/*   the startup has the settings to enable the GUI, we don't want multiple   */
/*   files that the GUI needs to manage; so we just create a temp copy that   */
/*   can be run when needed -- when the gui isn't running                     */
/*                                                                            */
/*   NOTE: it's ok to leave the trash file in fail case; it won't be used     */
/*----------------------------------------------------------------------------*/
static int prepareguiscript(void)
{
#if defined(MY_UNIT_TEST)
    return 0;
#else
#define SOURCEFILE "/rwflash/configs/startup.sh"

    /* if you find a better way than system calls, please refactor */
    if (system("cat " SOURCEFILE " | grep ^DHCP > " GUIRESTARTFILE)) {
        PRINTMAX("unable to create gui restart file\n")
        remove(GUIRESTARTFILE);
        return -1;
    }
    if (system("cat " SOURCEFILE " | grep ^ETH_D >> " GUIRESTARTFILE)) {
        PRINTMAX("unable to create gui restart file\n")
        remove(GUIRESTARTFILE);
        return -1;
    }
    if (system("cat " SOURCEFILE " | grep ^IP_FOR_ST >> " GUIRESTARTFILE)) {
        PRINTMAX("unable to create gui restart file\n")
        remove(GUIRESTARTFILE);
        return -1;
    }
    if (system("cat " SOURCEFILE " | grep ^WEBGUI_HTTP >> " GUIRESTARTFILE)) {
        PRINTMAX("unable to create gui restart file\n")
        remove(GUIRESTARTFILE);
        return -1;
    }
    if (system("sed -n '/# Startup web/,/# Webgui/p'  " SOURCEFILE " >> " GUIRESTARTFILE)) {
        PRINTMAX("unable to create gui restart file\n")
        remove(GUIRESTARTFILE);
        return -1;
    }
    if (system("chmod +x " GUIRESTARTFILE)) {
        PRINTMAX("unable to create gui restart file\n")
        remove(GUIRESTARTFILE);
        return -1;
    }

    return 0;
#endif
}

/*----------------------------------------------------------------------------*/
/* prepvguiscript: I'm sorry this maybe so fugly (AGAIN), but it has to be    */
/*   this way; the startup has the settings to enable the GUI, we don't want  */
/*   multiple files that the GUI needs to manage; so we just create a temp    */
/*   copy that can be run when needed -- when the gui isn't running           */
/*                                                                            */
/*   NOTE: it's ok to leave the trash file in fail case; it won't be used     */
/*   NOTE: this function is for VLAN interface!!!                             */
/*----------------------------------------------------------------------------*/
static int prepvguiscript(void)
{
#if defined(MY_UNIT_TEST)
    return 0;
#else
    /* if you find a better way than system calls, please refactor */
    if (system("cat " SOURCEFILE " | grep VLAN_ENABLE | grep TRUE")) {
        /* vlan not enabled; no work to do */
        return 0;
    }

    /* for my non-bash readers, this collects the VLAN IP from the startup.sh, and creates a 
       string for the vlan version of the GUI restart file from the normal GUI restart file; 
       apologies for the length of the line */
    if (system("[[ -e " GUIRESTARTFILE " ]] && echo IP_FOR_STATIC_ADDRESS=`cat " SOURCEFILE " | grep ^VLAN_IP | tr '=' ' ' | awk '{print $NF}'` > " VGUIMANAGEFILE " && cat " GUIRESTARTFILE " >> " VGUIMANAGEFILE)) {
        PRINTMAX("unable to create Vgui manage file\n")
        return -1;
    }
    /* ensure DHCP_ENABLE is false */
    if (system("sed -i 's/^DHCP_E.*/DHCP_ENABLE=FALSE/' " VGUIMANAGEFILE)) {
        PRINTMAX("unable to prepare Vgui manage file\n")
        remove(VGUIMANAGEFILE);
        return -1;
    }
    /* now delete the extra (original) IP line */
    if (system("sed -i '1b;/^IP_FOR_/d' " VGUIMANAGEFILE)) {
        PRINTMAX("unable to finish Vgui manage file\n")
        remove(VGUIMANAGEFILE);
        return -1;
    }
    /* rename some stuff to avoid conflict */
    if (system("sed -i 's/webgui/vebgui/g' " VGUIMANAGEFILE)) {
        PRINTMAX("unable to distinguish Vgui manage file\n")
        remove(VGUIMANAGEFILE);
        return -1;
    }
    if (system("sed -i 's/WebGui/VebGui/g' " VGUIMANAGEFILE)) {
        PRINTMAX("unable to distinguish2 Vgui manage file\n")
        remove(VGUIMANAGEFILE);
        return -1;
    }
    if (system("chmod +x " VGUIMANAGEFILE)) {
        PRINTMAX("unable to enable Vgui manage file\n")
        remove(VGUIMANAGEFILE);
        return -1;
    }

    return 0;
#undef SOURCEFILE
#endif
}
/*----------------------------------------------------------------------------*/
/* check rsudiag is running.                                                  */
/* TODO: rev version of module in bsp modifified.                             */
/* TODO: slay and restart or just tell people is done?                        */
/*----------------------------------------------------------------------------*/
void checkdiagthr(void)
{            
  if(prioronesecondcounter != onesecondcounter) {
      prioronesecondcounter = onesecondcounter;
      onesecondcounterfault = 0;
      localrsuhealth.errorstates &= ~(uint64_t)RSUHEALTH_DIAG_THR_TO;
  } else {
      onesecondcounterfault++;
  }
  if(ONESECTHREADFAULTMAX < onesecondcounterfault) {
      if(0x0 == (localrsuhealth.errorstates & RSUHEALTH_DIAG_THR_TO)){
          localrsuhealth.errorstates |= RSUHEALTH_DIAG_THR_TO;
          localrsuhealth.errorstates |= RSUHEALTH_FAULT_MODE; /* Will cause 1218 MIB to issue reset if recovery enabled. */
          I2V_DBG_LOG(LEVEL_DBG, MY_NAME,"RSUHEALTH_DIAG_THR_TO\n");
//TODO: And since we lock it, we probably release lock or something just in case.
//unlock
          updatediagnosticshm(); /* Thread is gone so we have to call. */
      }
  }
}
/*----------------------------------------------------------------------------*/
/* check realtime thread is running.                                          */
/* TODO: rev version of module in bsp modifified.                             */
/* TODO: slay and restart or just tell people is done?                        */
/*----------------------------------------------------------------------------*/
void checkrealtimethr(void)
{            
  if(priorrealtimecounter != realtimecounter) {
      priorrealtimecounter = realtimecounter;
      realtimecounterfault = 0;
      localrsuhealth.errorstates &= ~(uint64_t)RSUHEALTH_REALTIME_THR_TO;
  } else {
      realtimecounterfault++;
  }
  if(REALTIMETHREADFAULTMAX < realtimecounterfault) {
      if(0x0 == (localrsuhealth.errorstates & RSUHEALTH_REALTIME_THR_TO)){
          localrsuhealth.errorstates |= RSUHEALTH_REALTIME_THR_TO;
          I2V_DBG_LOG(LEVEL_DBG, MY_NAME,"RSUHEALTH_REALTIME_THR_TO\n");
      }
  }
}
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
void * WatcherThrd(void *ptr)
{
  struct timespec t;

  ptr=ptr;
  t.tv_sec = 1; /* set once, always 1 second */

  sleep(RSUHEALTH_BOOTUP_WAIT + 10); /* start too soon and just a bunch of weird fails of no interest while smashing devices trying to get ready, */

  while (loop) {
      nanosleep(&t, NULL);
      checkrealtimethr();
      checkdiagthr();
      if(localrsuhealth.errorstates & RSUHEALTH_DIAG_THR_TO) {
          if(3 == (rollingcounter % OUTPUT_MODULUS)) {
            I2V_DBG_LOG(LEVEL_INFO, MY_NAME,"RSUHEALTH_DIAG_THR_TO: (0x%lx) gnss(0x%x,%u,%u) tps(0x%x) cv2x(%u:0x%x) rsk(0x%x) sys(0x%x) wsm(0x%x)\n",
            localrsuhealth.errorstates
            ,localrsuhealth.gnssstatus
            ,localshmtps.debug_cnt_gps_fix_lost
            ,localshmtps.debug_cnt_gps_time_and_system_time_out_of_sync
            ,localshmtps.error_states
            ,localrsuhealth.cv2xruntime
            ,localrsuhealth.cv2xstatus
            ,rskstatus.error_states
            ,localrsuhealth.systemstates
            ,rskstatus.wsm_error_states);
          }
      }
      rollingcounter++; /* rolling counter */
      #if defined(MY_UNIT_TEST)
      if(CHILD_TEST_ITERATION_MAX < onesecondcounter) {
          loop = 0; /* signal main time to end test */
      }
      #endif
  }
#if defined(EXTRA_DEBUG)
  I2V_DBG_LOG(LEVEL_PRIV, MY_NAME,"WatcherThrd: Exiting:\n");
#endif
  pthread_exit(0);
}
/*----------------------------------------------------------------------------*/
/* RSU Health Main Function:                                                  */
/*   Initially, the rsuhealth is to first set cv2x configurations (call       */
/*   cv2xcfg) and manage the IP interface; it creates a 'ready' file used     */
/*   by the startup.sh to know when it can proceed with starting things like  */
/*   i2v and the GUI; during its runtime, it evalutes the IP to make sure     */
/*   it isn't 'lost' (reported issue) or a mismatch.                          */
/*   Eventually, it will restore some of the 5910 features such as ensuring   */
/*   i2v is running and the radio is working, and maybe some other things as  */
/*   well.                                                                    */
/*----------------------------------------------------------------------------*/
//NOTE: I2V_DBG_LOG is not available right away,
//    : because blob not mounted till later and LOGMGR not ready for a bit.
int MAIN(int argc, char *argv[])
{
    /* initial sleep is 1 second */
#if !defined(MY_UNIT_TEST)//TODO: Only testing diagnostics.
    unsigned char i, sleeptime = 1, readyset = 0, reappliedrules = 0;
#endif
    struct timespec t;
    int32_t ret = 0;
    char_t cmd[256]; /* Size accordingly */
#if defined(MY_UNIT_TEST)
    int32_t run_counter = 0;
#endif

    /* init across soft reset. */
#if !defined(MY_UNIT_TEST)//TODO: Only testing diagnostics.
    i = 0;
    sleeptime = 1; 
    readyset = 0; 
    reappliedrules = 0;
#endif
    ret = 0;
    memset(cmd,0,sizeof(cmd));
    memset(&t,0,sizeof(t));
#if defined(MY_UNIT_TEST)
    run_counter = 0;
#endif
    initstatics();

    /* set signals first; even if this is not service thread mode */
    sighandler(termhandler, SIGTERM);
    sighandler(termhandler, SIGINT);
    sighandler(i2vhandler, RSUSIG_TERMI2V);
    sighandler(i2vhandler, RSUSIG_RESTOREI2V);
    sighandler(ledhandler, RSUSIG_LEDMGMT);
    sighandler(ledqueryhandler, RSUSIG_LEDQUERY);
    sighandler(statehandler, RSUSIG_STATE);
    sighandler(utilityhandler, RSUSIG_UTIL);

    /* handle arguments */
    if(1 == parseargs(argc, argv)){
        return 0; /* daemon service not requested, we're done */
    }
    /* Do once and only for daemon portion. */
    i2vInitStatics();
    initrsudiagnostics(logfile);
    i2vUtilEnableDebug(MY_NAME);
    /* IP address */
    ipsetting();
    PRINTMAX("ip setting startup complete: ipgood(%d)\n",ipgood)
#if defined(PLATFORM_HD_RSU_5912)
    /* cv2x preparation function */
    preparecv2x();
#endif
    /* clean up customer files */
    preparecustom();
    PRINTMAX("prepare custom startup complete\n")

    /* password maangement */
    preparecheckpassword();    /* call now (sooner the better; but will also call later */
    PRINTMAX("prepare checkpw startup complete\n")
    
    /* sshd_config file */
    preparechecksshconf();
    PRINTMAX("prepare sshd_config complete\n")

    /* gui restart file */
    if (!prepareguiscript() && !prepvguiscript()) {
        guictl = 1;
    } else {
        localrsuhealth.errorstates |= RSUHEALTH_GUI_SCRIPT_FAIL;
    }

    if (system("touch " READYFILE)) {
        PRINTMAX("Health ERROR: unable to set ready indication\n")
        printf("\n%s| Health ERROR: unable to set ready indication\n\n",MY_NAME);
        localrsuhealth.errorstates |= RSUHEALTH_READY_FILE_FAIL;
        /* reboot? */
    }
    /* tracking file for rsuhealth watcher */
    if (system("touch /tmp/six332")) {
        PRINTMAX("Health ERROR: unable to set monitor indication\n")
        printf("\n%s| Health ERROR: unable to set monitor indication\n\n",MY_NAME);
        localrsuhealth.errorstates |= RSUHEALTH_MONITOR_FILE_FAIL;
    }
    /* At this point DCU has mounted the blob and we can access /rwflash/configs.
     * LOGMGR still not started so have to open syslogs later.
     */
    /* create 1 second and greater thread */
    if (0 == (ret = pthread_create(&OneSecThrdID, NULL, OneSecThrd, NULL))) {
        pthread_detach(OneSecThrdID);
    } else {
        I2V_DBG_LOG(LEVEL_WARN,MY_NAME,"Failed to start 1 second thread:  ret(%d) errno(%d)\n",ret, errno);
        localrsuhealth.errorstates |= RSUHEALTH_SYSCALL_FAIL; /* Not fatal, keep going */
    }

    /* create Set LEDs thread */
    if (0 == (ret = pthread_create(&SetLedsThrdID, NULL, SetLedsThrd, NULL))) {
        pthread_detach(SetLedsThrdID);
    } else {
        I2V_DBG_LOG(LEVEL_WARN,MY_NAME,"Failed to start Set LEDs thread: errno=%d.\n",ret, errno);
    }

    /* create watcher thread */
    if (0 == (ret = pthread_create(&WatcherThrdID, NULL, WatcherThrd, NULL))) {
        pthread_detach(WatcherThrdID);
    } else {
        I2V_DBG_LOG(LEVEL_WARN,MY_NAME,"Failed to start Watcher thread: ret(%d) errno(%d)\n",ret, errno);
        localrsuhealth.errorstates |= RSUHEALTH_SYSCALL_FAIL; /* Not fatal, keep going */
    }

    /* create network monitor thread */
    if (0 == (ret = pthread_create(&NetMonThrdID, NULL, networkEventThread, NULL))) {
        pthread_detach(NetMonThrdID);
    } else {
        I2V_DBG_LOG(LEVEL_WARN,MY_NAME,"Failed to start Network Monitor thread: ret(%d) errno(%d)\n",ret, errno);
        localrsuhealth.errorstates |= RSUHEALTH_SYSCALL_FAIL; /* Not fatal, keep going */
    }

    /* create manage customer folder thread */
    memset(cmd,0x0, sizeof(cmd));
    snprintf(cmd,sizeof(cmd),"%s/%s", RSUHEALTH_ACTIVE_DIR, MANAGE_CUSTOMER_FOLDER_FILE);
    if(i2vCheckDirOrFile(cmd)) { 
        if (0 == (ret = pthread_create(&cleancustomerfolderID, NULL, cleancustomerfolderThrd, NULL))) {
            localrsuhealth.errorstates &= ~(uint64_t)RSUHEALTH_MANAGE_FOLDER_OFF;
            pthread_detach(cleancustomerfolderID);
        } else {
            I2V_DBG_LOG(LEVEL_WARN,MY_NAME,"Failed to start customer clean thread: ret(%d) errno(%d)\n",ret, errno);
            localrsuhealth.errorstates |= RSUHEALTH_SYSCALL_FAIL; /* Not fatal, keep going */
            localrsuhealth.errorstates |= RSUHEALTH_MANAGE_FOLDER_OFF;
        }
    } else {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Manage customer folder start file does not exist. Folder contents will not be pruned.\n",ret);
        localrsuhealth.errorstates |= RSUHEALTH_MANAGE_FOLDER_OFF;
    }

    if(0 == ret) {
        PRINTMAX("RSUHealth running in service mode(%d)\n",getpid());
        memset(&t, 0, sizeof(t));
        while (loop) {
            #if !defined(MY_UNIT_TEST)//TODO: Only testing diagnostics.
            if (ipgood) {
                /* 20211013: JJG: what was i doing here? no comment and I don't see
                   the logic of trying to touch the file again; it doesn't hurt but
                   this one is somewhat weird; not sure what issue i saw */
                if (!readyset && system("touch " READYFILE)) {
                    PRINTMAX("Health ERROR: unable to set ready indication\n")
                    localrsuhealth.errorstates |= RSUHEALTH_READY_SET_FAIL;
                    ready_set_fault_count++;
                    /* reboot? */
                } else {
                    readyset = 1;
                    localrsuhealth.errorstates &= ~(uint64_t)RSUHEALTH_READY_SET_FAIL;
                    ready_set_fault_count = 0;
                }
                /* i2v health */
                checki2v();
                sleeptime = DEFAULTSLEEP;
            }
            t.tv_sec = 1;
            for (i=0; i<sleeptime; i++) {
                nanosleep(&t, NULL);
            }
            if (!ipgood) {
                ipsetting();
                sync();
            } else {
                if (!reappliedrules) {
                    /* 20211013: i hate having to do this, BUT...for trac 2837 the real issue
                       is that the default gateway does not apply (and maybe other rules too;
                       only tested 2 rules); this only happens after a firmware flash and not
                       on a normal reboot; so, the logic is to execute the command to apply the
                       rules a second time after the system is booted (need the sleep) and to
                       only do it a single time; manually running the command resolves the 
                       problem after the system boots, so this is the 'hack' to do the same
                    */
                    system("dnnetcfg >> /dev/null");     /* ignore output and return, you'll get some
                                                           complaints if rules already applied */
                    reappliedrules = 1;    /* one shot attempt to reapply, if above fails, tough... */
                }
                checkip();
            }

            checkled();

            checkgui();

            preparecheckpassword();    /* this is the later call */
            /* to be continuted... */
            #else  // MY_UNIT_TEST
            if(PARENT_TEST_ITERATION_MAX < run_counter) {
                printf("\n  Main exiting before child thread.\n");
                loop = 0;
            }
            run_counter++;
            sleep(1);
            #endif
        }
    }

    if (NULL != hlthfp) {
        fclose(hlthfp);
    }
#if defined(EXTRA_DEBUG)
    I2V_DBG_LOG(LEVEL_PRIV, MY_NAME,"EXIT:(0x%lx) gnss(0x%x,%u,%u) tps(0x%x) cv2x(%u:0x%x) rsk(0x%x) sys(0x%x) wsm(0x%x)\n",
        localrsuhealth.errorstates
        ,localrsuhealth.gnssstatus
        ,localshmtps.debug_cnt_gps_fix_lost
        ,localshmtps.debug_cnt_gps_time_and_system_time_out_of_sync
        ,localshmtps.error_states
        ,localrsuhealth.cv2xruntime
        ,localrsuhealth.cv2xstatus
        ,rskstatus.error_states
        ,localrsuhealth.systemstates
        ,rskstatus.wsm_error_states);
#endif
    i2vUtilDisableSyslog();
    i2vUtilDisableDebug();

    return 0;
}

