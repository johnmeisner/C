/**************************************************************************
 *                                                                        *
 *     File Name:  capture_scs_pcap.c                                     *
 *     Author:                                                            *
 *         DENSO International America, Inc.                              *
 *         North America Research Laboratory, California Office           *
 *         3252 Business Park Drive                                       *
 *         Vista, CA 92081                                                *
 *                                                                        *
 **************************************************************************/

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include "wsu_sharedmem.h"
#include "wsu_shm.h"
#include "i2v_shm_master.h"
#include "i2v_general.h"
#include "i2v_util.h"
#include "conf_table.h"
#if defined(MY_UNIT_TEST)
#include "stubs.h"
#define MAIN my_capture_eth_pcap_main
#else
#define MAIN main
#endif

#define DEFAULT_SCS_IP    "10.7.96.25"
/*tcpdump src net *IP* -i eth0 -w jtest -U -s 0 */
#define DEFAULT_COMMAND   "/usr/sbin/tcpdump"
#define DEFAULT_LOG_DIR   "/rwflash/pktlogs/"
#define STAT_FILE         "/rwflash/scs_cap_dat"  /* provides command line arguments input to tcpdump for debug */
#define DEFAULT_IFC       "eth0"
#define IFC_LEN   8  /* more than enough */
#define MAX_STR_IP   48  /* future support for IPv6 */

#define RELEASE_ARGS  \
        for (i=0; i<numargs; i++) { \
            free(capcmd[i+1]); \
        }

enum cmdOptionBits {
    CFG_PATH = 0x01,
    CFG_FILE = 0x02,
    CFG_IO = 0x04,
    CFG_IFC = 0x08,
    CFG_IP  = 0x10,
    CFG_SCS_EN = 0x20,
    CFG_FNAME = 0x40,
};

typedef struct {
    char_t configDir[I2V_CFG_MAX_STR_LEN];
    char_t configFname[I2V_CFG_MAX_STR_LEN];
    char_t outName[I2V_CFG_MAX_STR_LEN];
    char_t ifcname[IFC_LEN];
    char_t ipAddrStr[MAX_STR_IP];
    char_t dataDir;
    bool_t  useApp;
    uint32_t cmdopts;
} __attribute__((packed)) cmdOptions;

static char_t controllerIP[I2V_CFG_MAX_STR_LEN] = {0};
static bool_t mainloop = WTRUE;

static void termHandler(int __attribute__((unused)) sig)
{
    mainloop = WFALSE;
}

/* following fxns only care about IP of sig controller */
static WBOOL loadSCSDataFromI2V(void)
{
    i2vShmMasterT *shmPtr = NULL;

    if (system("pidof i2v > /dev/null")) {
        /* I2V not running */
        return WFALSE;
    }

    shmPtr = wsu_share_init(sizeof(i2vShmMasterT), I2V_SHM_PATH);
    if (shmPtr == NULL) {
        return WFALSE;
    }

    WSU_SEM_LOCKR(&shmPtr->scsCfgData.h.ch_lock);
    strcpy(controllerIP, shmPtr->scsCfgData.localSignalControllerIP);
    WSU_SEM_UNLOCKR(&shmPtr->scsCfgData.h.ch_lock);

    return WTRUE;
}

static bool_t loadSCSDataFromCfg(char_t *fname)
{
    FILE *f;
    cfgItemsTypeT cfgItems[] = {
        {"LocalSignalControllerIP",(void *)i2vUtilUpdateStrValue,controllerIP,NULL,(ITEM_VALID| UNINIT)},
    };

    if ((NULL == fname) || ((f=fopen(fname, "r")) == NULL)) {
        return WFALSE;
    }
//TODO: if something goes wrong way down here hard to see. Need event log item.
    if(I2V_RETURN_OK != i2vUtilParseConfFile(f, cfgItems, NUMITEMS(cfgItems), WTRUE, NULL)){
        if (   ((ITEM_VALID|BAD_VAL) == cfgItems[0].state) 
            || ((ITEM_VALID|UNINIT) == cfgItems[0].state)) {
            strcpy(controllerIP,I2V_SCS_LOCAL_CONTROLLER_IP_DEFAULT_S);
        }
    }
    fclose(f);

    return (strlen(controllerIP)) ? WTRUE: WFALSE;
}

/* returns 0 if there is an error */
static uint8_t formatCommand(cmdOptions *cmdOpts, char_t *cmd, char_t *args)
{
    bool_t ipset = WFALSE, addheader;
    char_t cfgfile[2 * I2V_CFG_MAX_STR_LEN + 1];
    char_t dirappend[6] = {0};  /* "tx" or "rx" or "tx_rx" */
    i2vTimeT clock;
    char_t logfile[I2V_CFG_MAX_STR_LEN + 50] = {0};  /* extra space in case out filename passed in */
    uint8_t numargs = 0;
    FILE *f;

    if (!(cmdOpts->cmdopts & CFG_IFC)) {
        strcpy(cmdOpts->ifcname, DEFAULT_IFC);
    }

    if (cmdOpts->cmdopts & CFG_SCS_EN) {
        if (cmdOpts->useApp) {
            ipset = loadSCSDataFromI2V();
        }

        if (!ipset && (cmdOpts->cmdopts & CFG_PATH)) {
            memset(cfgfile, 0, sizeof(cfgfile));
            sprintf(cfgfile, "%s%s", cmdOpts->configDir, cmdOpts->configFname);
            ipset = loadSCSDataFromCfg(cfgfile);
            if (!ipset) {
                printf("%s could not be read\n", cfgfile);
                return 0;
            }
        }

        if (!ipset && (cmdOpts->cmdopts & CFG_IP)) {
            strcpy(controllerIP, cmdOpts->ipAddrStr);
            ipset = WTRUE;
        }

        if (!ipset) {
            strcpy(controllerIP, DEFAULT_SCS_IP);
        }

        if (cmdOpts->cmdopts & CFG_IO) {
            switch (cmdOpts->dataDir) {
                case 1:
                    sprintf(dirappend, "tx");
                    sprintf(args, "dst net %s -i %s -U -s 0 -w ", controllerIP, cmdOpts->ifcname);
                    numargs = 10;  /* count the args above */
                    break;
                case 0:
                case 2:
                    sprintf(dirappend, "rx");
                    sprintf(args, "src net %s -i %s -U -s 0 -w ", controllerIP, cmdOpts->ifcname);
                    numargs = 10;
                    break;
                case 3:
                    sprintf(dirappend, "tx_rx");
                    sprintf(args, "dst or src net %s -i %s -U -s 0 -w ", controllerIP, cmdOpts->ifcname);
                    numargs = 12;
                    break;
                default:
                    printf("invalid data direction\n");
                    return 0;
            }
        } else {
            sprintf(dirappend, "rx");
            sprintf(args, "src net %s -i %s -U -s 0 -w ", controllerIP, cmdOpts->ifcname);
            numargs = 10;
        }
    } else {
        sprintf(dirappend, "tx_rx");
        sprintf(args, "-i %s -U -s 0 -w ", cmdOpts->ifcname);
        numargs = 7;
    }

    if (cmdOpts->cmdopts & CFG_FNAME) {
        sprintf(logfile, DEFAULT_LOG_DIR "%s", cmdOpts->outName);
    } else {
        /* YYYY_MM_DD_HH_MM_SS_eth0_rx.pcap */
        i2vUtilGetUTCTime(&clock);
        sprintf(logfile, DEFAULT_LOG_DIR "%d_%.2d_%.2d_%.2d_%.2d_%.2d_%s_%s.pcap", clock.year, clock.month,
                clock.day, clock.hour, clock.min, clock.sec, cmdOpts->ifcname, dirappend);
    }

    strcat(args, logfile);

    sprintf(cmd, DEFAULT_COMMAND);

    addheader = i2vCheckDirOrFile(STAT_FILE);

    if (NULL != (f=fopen(STAT_FILE, "w"))) {
        if (addheader) {
            fprintf(f, "Command Line Arguments\n");
        }
        fprintf(f, "%s\n", args);
        fclose(f);
    }

    return numargs;
}

static WBOOL parseArgs(int argc, char *argv[], cmdOptions *cmdOpts)
{
    int32_t c;

#if defined(MY_UNIT_TEST)
    optind = 0; /* If calling more than once you must reset to zero for getopt.*/
#endif
    while ((c=getopt(argc, argv, "F:d:f:hi:m:st:u")) != -1) {
        switch (c) {
            case 'F': /* output file name */
                if (strlen(optarg) > I2V_CFG_MAX_STR_LEN) {
                    printf("output filename too long\n");
                    return WFALSE;
                }
                strncpy(cmdOpts->outName, optarg, I2V_CFG_MAX_STR_LEN);
                cmdOpts->cmdopts |= CFG_FNAME;
                break;
            case 'd': /* config directory */
                if (strlen(optarg) > I2V_CFG_MAX_STR_LEN) {
                    printf("directory path too long\n");
                    return WFALSE;
                }
                strncpy(cmdOpts->configDir, optarg, I2V_CFG_MAX_STR_LEN);
                cmdOpts->cmdopts |= CFG_PATH;
                break;
            case 'f': /* config file name parameter */
                if (strlen(optarg) > I2V_CFG_MAX_STR_LEN) {
                    printf("filename too long\n");
                    return WFALSE;
                }
                strncpy(cmdOpts->configFname, optarg, I2V_CFG_MAX_STR_LEN);
                cmdOpts->cmdopts |= CFG_FILE;
                break;
            case 'h': /* help */
                return WFALSE;  /* this will result in usage message */
            case 'i': /* ifc */
                if (strlen(optarg) > IFC_LEN) {
                    printf("ifc name too long\n");
                    return WFALSE;
                }
                strncpy(cmdOpts->ifcname, optarg, IFC_LEN);
                cmdOpts->cmdopts |= CFG_IFC;
                break;
            case 'm': /* direction */
                cmdOpts->dataDir = (wuint8)atoi(optarg);  /* transmit direction */
                if (cmdOpts->dataDir > 3) {
                    printf("invalid data direction\n");
                    return WFALSE;
                }
                cmdOpts->cmdopts |= CFG_IO;
                break;
            case 's': /* scs parm */
                cmdOpts->cmdopts |= CFG_SCS_EN;
                break;
            case 't': /* scs IP */
                if (strlen(optarg) > MAX_STR_IP) {
                    printf("IP too long\n");
                    return WFALSE;
                }
                strncpy(cmdOpts->ipAddrStr, optarg, MAX_STR_IP);
                cmdOpts->cmdopts |= CFG_IP;
                break;
            case 'u': /* use i2v */
                cmdOpts->useApp = WTRUE;
                break;
            default:
                break;
        }
    }

    if (((cmdOpts->cmdopts & CFG_PATH) && !(cmdOpts->cmdopts & CFG_FILE)) ||
        (!(cmdOpts->cmdopts & CFG_PATH) && (cmdOpts->cmdopts & CFG_FILE))) {
        printf("invalid options for -d and -f\n");
        return WFALSE;
    }

    if ((!cmdOpts->useApp) && !(cmdOpts->cmdopts & CFG_PATH) && !(cmdOpts->cmdopts & CFG_IP)) {
        cmdOpts->useApp = WTRUE;   /* no cfg file or ip as input, set for i2v by default */
    }

    return WTRUE;
}

static void usage(char_t *name)
{
    while ((*name == '.')||(*name == '/')||(*name == ' ')) name++;

    printf("\nUsage: %s [ -i <interface name> ] [ -m <1|2|3> ] | -h\n", name); 
    printf("  [ -s [ -d <config directory> -f <config file> ][ -u ][ -t <IP address> ]]\n\n" );
    printf("OPTIONAL parameters:\n");
    printf(" -h   print this help message.\n");
    printf(" -i   the interface to listen on (ex: eth0).\n");
    printf(" -m   used to specify the direction (1: TX  2: RX  3: Both).\n\n");
    printf(" -s   signal controller option - restrict logging to target IP ethernet data.\n");
    printf("      Enables the following options (ignored without the -s option):\n");
    printf("      -d   use the config file (-f) for loading IP data.\n");
    printf("      -u   use I2V shared memory (highest precedence) for IP data.\n");
    printf("      -t   the target IP to monitor.\n\n");
    printf(" When the -s option is used alone, defaults set to listen (RX) on eth0\n");
    printf(" and load I2V shared memory if I2V running or use address of " DEFAULT_SCS_IP ".\n");
    printf(" The -s option can monitor any desired IP with the -t option.\n\n");
}

static WBOOL startCommand(char_t *cmd, char_t *args, const uint8_t numargs, pid_t *pid)
{
/* probably a waste */
#define ARG_BYTES   60
    char_t *capcmd[numargs + 2];
    uint8_t i;
    char *tok;
    int32_t fd = -1;

    capcmd[0] = cmd;
    tok = strtok(args, " ");
    for (i=0; i<numargs; i++) {
        if (NULL != tok) {
            capcmd[i+1] = malloc(ARG_BYTES);
            strcpy(capcmd[i+1], tok);
            tok = strtok(NULL, " ");
        } else {
            for (; i; i--) {
                free(capcmd[i]);
            }
            return WFALSE;
        }
    }
    capcmd[numargs + 1] = (char_t *)0;

    if ((*pid = fork()) == -1) {
        RELEASE_ARGS;
        return WFALSE;
    } else if (*pid == 0) {
        if (open(cmd, O_RDONLY) == -1) {
            printf("Cannot run cmd: %s (%s/%d)\n", cmd, strerror(errno), errno);
            exit(-1);
        }
        if ((fd=open("/dev/null", O_RDWR)) != -1) {
            dup2(fd, 0);
            dup2(fd, 1);  /* suppress console output */
            dup2(fd, 2);  /* suppress console output */
        }
        if (execvp(cmd, (char_t *const *)capcmd) == -1) {
            printf("error starting pcap logging\n");  /* may not get printed */
            return WFALSE;
        }
        return WTRUE;       /* this code is never reached. */
    } else {
        RELEASE_ARGS;
    }
    return WTRUE;
#undef ARG_BYTES
}

static void termCommand(pid_t *pid)
{
    union sigval val;

    val.sival_int = 0;
    sigqueue(*pid, SIGTERM, val);
    usleep(1000000);  /* wait 1 sec */
    sigqueue(*pid, SIGKILL, val);
}
static int32_t MakeDir(char_t *dirname)
{
    FILE *f;
    char_t syscmd[256 + 20]; /* mkdir -p dirname. Adjust accordingly for your needs. */

    if (NULL == dirname){
        return -1;
    }
    if(256 < strlen(dirname)) {
        return -2;
    }
    if ((f=fopen(dirname, "r")) == NULL) {
        strcpy(syscmd, "mkdir -p ");
        strcat(syscmd, dirname);
        return (system(syscmd) != -1) ? 0 : -3;
    } else {
        fclose(f);
    }

    return 0;
}
//TODO: use argc/argv to enable DEBUG. No I2V SHM.
int32_t MAIN(int32_t argc, char_t *argv[])
{
    cmdOptions cmdOpts;
    struct sigaction sa;
    char_t cmd[40];       /* extra space */
    char_t cmdargs[300];  /* should be plenty */
    pid_t  pid;
    uint8_t numargs;
    int32_t ret;

    memset(&cmdOpts, 0, sizeof(cmdOpts));

    if (!parseArgs(argc, argv, &cmdOpts)) {
        usage(argv[0]);
        return 1;
    }

    if (0 == (numargs = formatCommand(&cmdOpts, cmd, cmdargs))) {
        usage(argv[0]);
        return 2;
    }

    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = termHandler;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGCHLD, &sa, NULL);

    /* If it doesn't exist then create it. */
    if (0 != (ret = MakeDir(DEFAULT_LOG_DIR))) {
       printf("Capture_eth_pcap: MakeDir(%s) failed: ret=%d.\n", DEFAULT_LOG_DIR, ret);
       return 3;
    }

    if (!startCommand(cmd, cmdargs, numargs, &pid)) {
        return 4;
    }

    while (mainloop) {
        usleep(1000000);  /* nothing to do */
    }

    termCommand(&pid);

    return 0;
}
