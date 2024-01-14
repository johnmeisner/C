/**************************************************************************
 *                                                                        *
 *     File Name:  obe.c                                                  *
 *     Author:                                                            *
 *         DENSO International America, Inc.                              *
 *         North America Research Laboratory, California Office           *
 *         3252 Business Park Drive                                       *
 *         Vista, CA 92081                                                *
 *                                                                        *
 **************************************************************************/
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <net/if.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <pthread.h>
#include <fcntl.h>
#include <ctype.h>
#include <time.h>
#include <dirent.h>

#define RD_CFG_FILE        "/rwflash/configs/wsu_routerd.conf"
#define MY_CFG_FILE        "obe.conf"

#define ONE_SECOND         1000000
#define MAX_RESPONSE_TIME  3
#define RESPONSE_DELAY     500000

#define MAX_STR_LEN      101
#define MAX_BYTE_SIZE    100
#define MAX_BYTE_STR     2 * MAX_BYTE_SIZE + 1
#define MAX_KEY_LEN      26
#define MAX_TCP_DATA     0xFFFF

/* has to be long enough for byte string + key name and '=' 
   with only a single 'null' the MAX_KEY_LEN covers the '=' 
   need to add room for spaces (MAX_BYTE_SIZE) */
#define MAX_LINE_LEN     MAX_KEY_LEN + MAX_BYTE_STR + MAX_BYTE_SIZE

/* default values */
#define DEFAULT_PORT          16094
#define DEFAULT_IPVER         6
#define DEFAULT_NUMCLIENTS    10

#define ACK_LENGTH            16

typedef enum {
    FALSE = 0,
    TRUE 
} __attribute__((packed)) BOOLEAN;

static BOOLEAN mainloop = TRUE;
static BOOLEAN portSet = FALSE;
static BOOLEAN antennaSet = FALSE;
static BOOLEAN radioSet = FALSE;
static BOOLEAN powerSet = FALSE;
static BOOLEAN routerdOn = FALSE;
static BOOLEAN needRDCfg = FALSE;
static BOOLEAN verbose = FALSE;
static BOOLEAN nodelete = FALSE;
static BOOLEAN validIP = FALSE;
static BOOLEAN usingipv6 = FALSE;
static unsigned short int port = 0;
static unsigned char antenna = 0;
static unsigned char channel = 0;
static unsigned char txrate = 0;
static unsigned char radionum = 0;
static unsigned char txpower = 0;
static char gatewayip[MAX_STR_LEN] = {0};
static char ipaddr[MAX_STR_LEN] = {0};
static char subnet[MAX_STR_LEN] = {0};
static char radioip[MAX_STR_LEN] = {0};
static char cfgdir[MAX_STR_LEN] = {0};
static char mbrdir[MAX_STR_LEN] = {0};
static struct sockaddr_in targetsvr;
static struct sockaddr_in6 targetsvr6;

static void sigHandler(int __attribute__((unused)) sig)
{
    mainloop = FALSE;
}

static void usage(char *name)
{
    char *idx = &name[strlen(name) - 1];
    int count = 0;
    char *printname, *basename = NULL;
    BOOLEAN useRegName = TRUE;

    /* print name beautification */
    while (idx != name) {
        if ((*idx == '/') && (count)) {
            basename = (char *)malloc(count);
            if (basename != NULL) {
                memset(basename, 0, count);
                strcpy(basename, ++idx);
                useRegName = FALSE;
            }
            break;
        }
        count++;
        idx--;
    }
    printname = (useRegName) ? name : basename;

    /* now the actual usage */
    printf("\nUsage: %s [ -p <port> -s <server ip> -i <IP address>\n", printname);
    printf("\t--g <gateway> -n <target subnet> -r <radio> -a <antenna>\n");
    printf("\t-c <channel> -t <transmit power> -f <transmit rate>\n");
    printf("\t-w <wireless ip> -d <directory> -m <MBR directory> ] | -h\n");
    printf("\nUses wsu_routerd for sending.\n");
    printf("Will generate wsu_routerd.conf file if not found using values\n");
    printf("from obe.conf (or command line if specified).\n");
    printf("Will start wsu_routerd if not running (skips conf generation\n");
    printf("if wsu_routerd running).\n");
    printf("Will transmit MBR files found in MBR directory (and delete\n");
    printf("them once sent).\n");
    printf("\nPress Ctrl+C to exit\n");
    printf("\n\tAll parameters OPTIONAL (obe.conf used if param absent):\n");
    printf("\t-p\tPort - port to send data on\n");
    printf("\t-s\tServer IP - the IPv6 MBR server (SCMS)\n");
    printf("\t-i\twsu_routerd: IP address (must match specified version)\n");
    printf("\t\t(ex: IPv6 - 2003::10,  IPv4 - 192.168.1.2)\n");
    printf("\t-g\twsu_routerd: Gateway IP (the other wsu - must match IP version)\n");
    printf("\t-n\twsu_routerd: Target subnet (must match IP version)\n");
    printf("\t\t(ex: IPv6 - 2004::/64,  IPv4 - 192.168.102.0/24)\n");
    printf("\t-r\twsu_routerd: Radio - wsu radio number 0 or 1\n");
    printf("\t-a\twsu_routerd: Antenna - 0, 1, or 2\n");
    printf("\t-c\twsu_routerd: Channel - 172 - 184\n");
    printf("\t-t\twsu_routerd: Transmit Power - 0 - 32\n");
    printf("\t-f\twsu_routerd: Transmit rate - 6,9,12,18,24,36,48,or 54\n");
    printf("\t-w\twsu_routerd: Wireless IP for radio (must match IP version)\n");
    printf("\t-d\tDirectory - where obe.conf file is located (if not default)\n");
    printf("\t-m\tMBR Directory - where MBR files are kept\n");
    printf("\t-h\tHelp - print this help message\n\n");

    if (!useRegName) {
        free(basename);
    }
}

static BOOLEAN parseArgs(int argc, char *argv[])
{
    signed char c;

    while ((c=getopt(argc, argv, "a:c:d:f:g:hi:m:n:p:r:s:t:Vw:x")) != -1) {
        switch (c) {
            case 'a':
                if (!antenna && !antennaSet) {
                    antenna = (unsigned char)strtoul(optarg, NULL, 10);
                    antennaSet = TRUE;
                }
                break;
            case 'c':
                if (!channel) {
                    channel = (unsigned char)strtoul(optarg, NULL, 10);
                }
                break;
            case 'd':
                if (!strlen(cfgdir)) {
                    strcpy(cfgdir, optarg);
                }
                break;
            case 'f':
                if (!txrate) {
                    txrate = (unsigned char)strtoul(optarg, NULL, 10);
                }
                break;
            case 'g':
                if (!strlen(gatewayip)) {
                    strcpy(gatewayip, optarg);
                }
                break;
            case 'i':
                if (!strlen(ipaddr)) {
                    strcpy(ipaddr, optarg);
                }
                break;
            case 'm':
                if (!strlen(mbrdir)) {
                    strcpy(mbrdir, optarg);
                }
                break;
            case 'n':
                if (!strlen(subnet)) {
                    strcpy(subnet, optarg);
                }
                break;
            case 'p':
                if (!port) {
                    port = (unsigned short int)strtoul(optarg, NULL, 10);
                }
                break; 
            case 'r':
                if (!radioSet && !radionum) {
                    radionum = (unsigned char)strtoul(optarg, NULL, 10);
                    radioSet = TRUE;
                }
                break; 
            case 's':
                if (!validIP) {
                    if (1 == inet_pton(AF_INET, optarg, &targetsvr.sin_addr)) {
                        validIP = TRUE;
                    } else if (1 == inet_pton(AF_INET6, optarg, &targetsvr6.sin6_addr)) {
                        validIP = TRUE;
                        usingipv6 = TRUE;
                    } else {
                        printf("\nInput Error: invalid IP address\n");
                    }
                }
                break;
            case 't':
                if (!powerSet && !txpower) {
                    txpower = (unsigned char)strtoul(optarg, NULL, 10);
                    powerSet = TRUE;
                }
                break; 
            case 'V':
                verbose = TRUE;
                break;
            case 'w':
                if (!strlen(radioip)) {
                    strcpy(radioip, optarg);
                }
                break;
            case 'x':
                nodelete = TRUE;
                break;
            case 'h':
                return FALSE;
            default:
                break;
        }
    }
    return TRUE;
}

static BOOLEAN validateArgs(BOOLEAN nocfgfile)
{
    BOOLEAN validfreq = FALSE, incomplete = FALSE;
    unsigned char allowedfreq[] = {6,9,12,18,24,36,48,54};
    unsigned int i;

    /* port */
    if (!portSet && !nocfgfile) {
        printf("\nWarning: port not specified, using default\n");
        port = DEFAULT_PORT;
    } else if (!portSet) {
        incomplete = TRUE;   /* no cmd line input, will wait for config file */
    } else {    /* input value from either cmd line or cfg file */
        /* really any above 49152 are 'reserved' as ephemeral ports */
        if (port == 0xFFFF) {
            printf("\nConfiguration error: reserved port specified [%d] - failing\n", port);
            return FALSE;
        }
    }

    if (!strlen(mbrdir) || !validIP) {
        incomplete = TRUE;
    }
 
    if (routerdOn || !needRDCfg) {   /* only care about server ip, port and mbrdir if routerdOn */
        return (!incomplete);
    }
 
    /* frequency (txrate) */
    for (i=0; i<(sizeof(allowedfreq)/sizeof(char)); i++) {
        if (txrate == allowedfreq[i]) {
            validfreq = TRUE;
            break;
        }
    }
    if (!validfreq) {
        if (nocfgfile) {
            if (txrate) {
                /* user gets a pass if cmd line entry was '0' */
                printf("\nInput Error: invalid rate(frequency) specified [%d]\n", txrate);
                txrate = 0;
            }
            incomplete = TRUE;
        } else {
            printf("\nConfiguration Error: invalid rate(frequency) specified [%d] - failing\n", txrate);
            return FALSE;
        }
    }

    /* power */
    if (powerSet) {
        if ((txpower > 32) && (!nocfgfile)) {
            printf("\nConfiguration Error: invalid power specified [%d] - failing\n", txpower);
            return FALSE;
        }
        if (txpower > 32) {
            printf("\nInput Error: power specified invalid[%d]\n", txpower);
            incomplete = TRUE;
            powerSet = FALSE;
        }
    } else {
        incomplete = TRUE;
    }

    /* antenna */
    if (antennaSet) {
        if ((antenna > 2) && (!nocfgfile)) {
            printf("\nConfiguration Error: invalid antenna value specified [%d] - failing\n", antenna);
            return FALSE;
        }
        if (antenna > 2) {
            printf("\nInput Error: antenna value specified invalid[%d]\n", antenna);
            incomplete = TRUE;
            antennaSet = FALSE;
        }
    } else {
        incomplete = TRUE;
    }

    /* radio */
    if (radioSet) {
        if ((radionum > 1) && (!nocfgfile)) {
            printf("\nConfiguration Error: invalid radio specified [%d] - failing\n", radionum);
            return FALSE;
        }
        if (radionum > 1) {
            printf("\nInput Error: radio specified invalid[%d]\n", radionum);
            incomplete = TRUE;
            radioSet = FALSE;
        }
    } else {
        incomplete = TRUE;
    }

    /* channel */
    if (channel) {
        if ((channel < 172) || (channel > 184)) {
            if (nocfgfile) {
                printf("\nInput Error: invalid channel specified [%d]\n", channel);
                incomplete = TRUE;
                channel = 0;
            } else {
                printf("\nConfiguration Error: invalid channel specified [%d] - failing\n", channel);
                return FALSE;
            }
        }
    } else {
        incomplete = TRUE;
    }

    /* string IP params */
    if (!strlen(ipaddr) || !strlen(gatewayip) || !strlen(subnet) || !strlen(radioip)) {
        incomplete = TRUE;
    }

    return (!incomplete);
}

/* input key and value should be memset to 0 before passed in */
static BOOLEAN getKeyAndValue(FILE *f, char *buf, char *key, char *value)
{
    char *ptr = buf, *dest = key, *whichkey=key;
    int count = 0, sz;

    if ((NULL == buf) || (NULL == key) || (NULL == value)) {
        return FALSE;
    }
    sz = strlen(buf);

    while ((ptr != 0) && (count < sz)) {
        if (((whichkey == key) && (count > (MAX_KEY_LEN - 1))) || 
           ((whichkey == value) && (count > (MAX_BYTE_STR - 1)))) {
            printf("\nConfiguration Error: config file contents too long (%s %s %d)\n", key, value, count);
            return FALSE;
        }
        if (*ptr == '#') {
            if ((sz == (MAX_LINE_LEN - 1)) && (buf[sz -1] != '\n')) {
                while ('\n' != fgetc(f));
            }
            return FALSE;   /* do not support comments */
        }
        if (*ptr == '=') {  /* a second equal is ignored - maybe should be flagged as invalid */
            if (whichkey == key) count = 0;
            dest = whichkey = value;
            ptr++;
            continue;
        }
        /* remove all white spaces */
        if ((*ptr != '\n') && (*ptr != '\t') && (*ptr != ' ')) {
            *dest++ = toupper(*ptr);
        } else {
            count--;   /* do not count spaces */
        }
        ptr++;
        count++;
    }

    return TRUE;
}

static BOOLEAN parseCfg(void)
{
    FILE *f = NULL;
    char buf[MAX_LINE_LEN] = {0};
    char key[MAX_KEY_LEN] = {0};
    char value[MAX_BYTE_STR] = {0};
    char cfgfile[MAX_STR_LEN + sizeof(MY_CFG_FILE)] = {0};
    char *ptr;
    struct stat st;

    if (!validateArgs(TRUE)) {
        printf("\nUsing config file for missing parameters\n");
    } else {
        return TRUE;
    }

    /* check current directory first, then /rwflash/configs IF no input cfg directory */
    if (strlen(cfgdir)) {
        sprintf(cfgfile, "%s/%s", cfgdir, MY_CFG_FILE);
    } else {
        if (!stat(MY_CFG_FILE, &st)) {
            /* use current directory file */
            strcpy(cfgfile, MY_CFG_FILE);
        } else {
            /* try /rwflash/configs - don't need to stat */
            sprintf(cfgfile, "/rwflash/configs/" MY_CFG_FILE);
        }
    }
    f = fopen(cfgfile, "r");
    if (NULL == f) {
        printf("\nRuntime Error: cannot open config file (%s)\n", cfgfile);
        return FALSE;
    }

    while (NULL != fgets(buf, MAX_LINE_LEN, f)) {
        memset(key, 0, MAX_KEY_LEN);
        memset(value, 0, MAX_BYTE_STR);
        if (!getKeyAndValue(f, buf, key, value)) {
            memset(buf, 0, MAX_LINE_LEN);
            continue;
        }
        /* expected config items */
        if (!validIP && !strcmp(key, "SERVER_IP")) {
            if (1 == inet_pton(AF_INET, value, &targetsvr.sin_addr)) {
                validIP = TRUE;
            } else if (1 == inet_pton(AF_INET6, value, &targetsvr6.sin6_addr)) {
                validIP = TRUE;
                usingipv6 = TRUE;
            } else {
                printf("\nConfiguration Error: invalid IP address\n");
                return FALSE;
            }
            continue;
        }
        if (!port && !strcmp(key, "PORT")) {
            port = (unsigned short int)strtoul(value, NULL, 10);
            if (routerdOn) {
                break;
            } else {
                continue;
            }
        }
        if (!strlen(mbrdir) && !strcmp(key, "MBR_DIRECTORY")) {
            strcpy(mbrdir, value);
            /* set it to all lowers - config file limitation (no support for upper case) */
            ptr = mbrdir;
            ptr += strlen(mbrdir) - 1;
            for (; ptr >= mbrdir; ptr--) {
                *ptr = tolower(*ptr);
            }
            continue;
        }
        if (!strlen(ipaddr) && !strcmp(key, "IP_ADDRESS")) {
            strcpy(ipaddr, value);
            continue;
        }
        if (!strlen(gatewayip) && !strcmp(key, "GATEWAY")) {
            strcpy(gatewayip, value);
            continue;
        }
        if (!strlen(subnet) && !strcmp(key, "SUBNET")) {
            strcpy(subnet, value);
            continue;
        }
        if (!strlen(radioip) && !strcmp(key, "RADIO_IP")) {
            strcpy(radioip, value);
            continue;
        }
        if (!antennaSet && !strcmp(key, "ANTENNA")) {
            antenna = (unsigned char)strtoul(value, NULL, 10);
            antennaSet = TRUE;
            continue;
        }
        if (!radioSet && !strcmp(key, "RADIO")) {
            radionum = (unsigned char)strtoul(value, NULL, 10);
            radioSet = TRUE;
            continue;
        }
        if (!powerSet && !strcmp(key, "POWER")) {
            txpower = (unsigned char)strtoul(value, NULL, 10);
            powerSet = TRUE;
            continue;
        }
        if (!txrate && !strcmp(key, "FREQUENCY")) {
            txrate = (unsigned char)strtoul(value, NULL, 10);
            continue;
        }
        if (!channel && !strcmp(key, "CHANNEL")) {
            channel = (unsigned char)strtoul(value, NULL, 10);
            continue;
        }
    }
    fclose(f);

    return validateArgs(FALSE);
}

static BOOLEAN checkRouterd(void)
{
    int ret = -1;
    FILE *f;

    ret = system("pidof wsu_routerd > /dev/null");

    if (ret < 0) {
        return FALSE;
    } else if (!ret) {
        routerdOn = TRUE;
        return TRUE;
    }

    if (NULL == (f=fopen(RD_CFG_FILE, "r"))) {
        /* assume file missing */
        needRDCfg = TRUE;
    } else {
        fclose(f);
    }

    return TRUE;
}

static BOOLEAN startRouterd(pid_t *pid)
{
    /* TODO: don't forget about 5001 (/usr/sbin)*/
    char buf[] = "/usr/sbin/wsu_routerd";
    FILE *f = NULL;
    char *rd[] = {"wsu_routerd", (char *)0};
    int fd = -1;
    char ipver = (usingipv6) ? 6 : 4;

    if (routerdOn) {
        return TRUE;
    }

    if (needRDCfg) {
        /* create cfg file */
        if (NULL == (f=fopen(RD_CFG_FILE, "w"))) {
            printf("\nRuntime Error: could not create wsu_routerd.conf\n");
            return FALSE;
        }
        fprintf(f, "ip_version=%d\n", ipver);
        fprintf(f, "ethernet_ip=%s\n", ipaddr);
        fprintf(f, "wireless_ip=%s\n", radioip);
        fprintf(f, "gateway=%s via %s\n", subnet, gatewayip);
        fprintf(f, "radio=%d\n", radionum);
        fprintf(f, "antenna=%d\n", antenna);
        fprintf(f, "channel=%d\n", channel);
        fprintf(f, "txpower=%d\n", txpower);
        fprintf(f, "txrate=%d\n", txrate);
        fclose(f);
    }

    /* start wsu_routerd - assuming location /usr/sbin */
    if ((*pid = fork()) == -1) {
        printf("\nRuntime Error: internal failure\n");
        return FALSE;
    } else if (*pid == 0) {
        if (open(buf, O_RDONLY) < 0) {
            printf("\nRuntime Error: unable to find wsu_routerd\n");
            exit(-1);
        }
        if (!verbose) {
            if ((fd=open("/dev/null", O_RDWR)) != -1) {
                dup2(fd, 0);
                dup2(fd, 1);
                dup2(fd, 2);
            }
        }
        if (execvp(buf, (char *const *)rd) == -1) {
            printf("\nRuntime Error: unable to start wsu_routerd\n");
            exit(-1);
        }
    }

    return TRUE;
}

static void stopRouterd(pid_t *pid)
{
    union sigval val;

    if (routerdOn) {
        return;   /* if already on do nothing */
    }

    memset(&val, 0, sizeof(val));
    val.sival_int = 0;
    sigqueue(*pid, SIGTERM, val);
    usleep(100000);  /* wait 100 ms */
    kill(*pid, SIGKILL);

    if (needRDCfg) {
        remove(RD_CFG_FILE);  /* delete what we created */
    }
}

static BOOLEAN startSock(int *sock)
{
#define RETRY_TIMEOUT  1000

    int dummy = (usingipv6) ? AF_INET6 : AF_INET;    /* reused */
    struct sockaddr *ptr = NULL;
    unsigned char numretries = 5;

    if (NULL == sock) return FALSE;

    if ((*sock = socket(dummy, SOCK_STREAM, 0)) < 0) {
        printf("\nRuntime Error: socket creation failed\n");
        return FALSE;
    }

    /* address already set */
    if (usingipv6) {
        targetsvr6.sin6_family = AF_INET6;
        targetsvr6.sin6_port = htons(port);
        ptr = (struct sockaddr *)&targetsvr6;
    } else {
        targetsvr.sin_family = AF_INET;
        targetsvr.sin_port = htons(port);
        ptr = (struct sockaddr *)&targetsvr;
    }

    /* mark sock non blocking */
    if ((dummy = fcntl(*sock, F_GETFL, 0)) < 0) {
        printf("\nRuntime Error: unable to modify socket\n");
        close(*sock);
        return FALSE;
    }
    dummy |= O_NONBLOCK;
    if (fcntl(*sock, F_SETFL, dummy) < 0) {
        printf("\nRuntime Error: unable to modify socket\n");
        close(*sock);
        return FALSE;
    }
    dummy = (usingipv6) ? sizeof(targetsvr6) : sizeof(targetsvr);

doover:
    if (connect(*sock, ptr, dummy) < 0) {
        if (((errno == EINPROGRESS) || (errno == EALREADY)) && numretries) {
            numretries--;
            usleep(RETRY_TIMEOUT);
            goto doover;
        }
        printf("\nRuntime Error: could not connect to target IP\n");
        close(*sock);
        return FALSE;
    }

    return TRUE;

#undef RETRY_TIMEOUT
}

static void process(void)
{
#define NUM_RETRIES 5
#define RETRY_TIMEOUT 1000

    int ackcount, count, sock;
    struct stat st;
    DIR *dirp = NULL;
    struct dirent *dptr = NULL;
    char filename[400];   /* should be more than enough */
    FILE *f = NULL;
    char data[MAX_TCP_DATA];
    char ack[ACK_LENGTH];
    int initsec;
    struct timeval tv;
    BOOLEAN stopsending = FALSE;
    unsigned char numretries;

    if (stat(mbrdir, &st)) {
        /* failure to get mbr directory status */
        if (verbose) printf("\nRuntime Error: unable to get MBR directory status: %s\n", mbrdir);
        return;
    }

    if (!S_ISDIR(st.st_mode)) {
        /* not a directory */
        if (verbose) printf("\nConfiguration Error (causing runtime error):\nInvalid MBR directory: %s\n", mbrdir);
        return;
    }

    /* get directory contents and process */
    if (NULL == (dirp = opendir(mbrdir))) {
        if (verbose) printf("\nRuntime Error: unable to open MBR directory\n");
        return;
    }
    if (!startSock(&sock)) {
        if (verbose) printf("\nRuntime Error: unable to connect to server\n");
        return;
    }
    while (mainloop && !stopsending && (NULL != (dptr = readdir(dirp)))) {
        if (verbose) printf("File in MBR directory: %s\n", dptr->d_name);
        /* make sure item is a file */
        snprintf(filename, sizeof(filename), "%s/%s", mbrdir, dptr->d_name);
        if (stat(filename, &st)) {
            /* error on file - wait till next time */
            if (verbose) printf("skipping file, could not stat\n");
            break;
        } 
        if (!S_ISREG(st.st_mode)) {
            /* not a regular file - skip it */
            if (verbose) printf("skipping file, not a regular file\n");
            continue;
        }
        
        /* read in file contents to send */
        if (NULL == (f = fopen(filename, "r"))) {
            /* unable to access file - skip it */
            if (verbose) printf("skipping file, cannot open file\n");
            continue;
        }
        count = fread(data, 1, MAX_TCP_DATA, f);
        fclose(f);
        numretries = NUM_RETRIES;

writeover:
        /* write data, then listen for ack */
        if (write(sock, data, count) < count) {
            if (((errno == EAGAIN) || (errno == EWOULDBLOCK)) && numretries) {
                usleep(RETRY_TIMEOUT);
                numretries--;
                goto writeover;
            }
            /* failure on write - stop processing */
            if (verbose) printf("failure to write file\n");
            break;
        }

        /* read the return data */
        gettimeofday(&tv, NULL);
        initsec = tv.tv_sec;
        while (TRUE) {
            if ((ackcount = read(sock, ack, ACK_LENGTH)) <= 0) {
                if ((errno == EAGAIN) || (errno == EWOULDBLOCK) || !ackcount) {
                    /* continue waiting while program still active or timeout */
                    if (mainloop) {
                        gettimeofday(&tv, NULL);
                        if ((tv.tv_sec - initsec) > MAX_RESPONSE_TIME) {
                            /* timeout - will retry sending file later; break
                               all send attempts */
                            if (verbose) printf("timeout from server for ack\n");
                            stopsending = TRUE;
                            break;
                        } else {
                            usleep(RESPONSE_DELAY);
                        }
                    } else {    /* !mainloop - program done */
                        break;
                    }
                    /* getting here means retry the read */
                    continue;
                }
            } else if (ackcount < 0) {
                /* some other error */
                if (verbose) printf("Runtime Error: %s\n", strerror(errno));
                stopsending = TRUE;
                break;
            } else {
                /* have data - process it (delete file if a match) */
                if (ackcount < ACK_LENGTH) {
                    if (verbose) printf("got back a bad ack: %d\n", ackcount);
                    break;
                }
                if (!memcmp(ack, &data[count - ACK_LENGTH], ACK_LENGTH)) {
                    if (verbose) printf("got back a matching ack\n");
                    if (!nodelete) remove(filename);
                } else if (verbose) {
                    printf("got a mismatch ack\n");
                }
                break;
            }
        }
    }
    close(sock);
    closedir(dirp);

#undef NUM_RETRIES
#undef RETRY_TIMEOUT
}

int main(int argc, char *argv[])
{
    struct sigaction sa;
    pid_t pid;
    struct timeval tv;

    memset(&sa, 0, sizeof(sa));
    memset(&pid, 0, sizeof(pid)); /* TM !!TODO!! to avoid warning...*/
    sa.sa_handler = sigHandler;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    if (!parseArgs(argc, argv)) {
        usage(argv[0]);
        return 1;
    }

    if (!checkRouterd()) {
        printf("\nRuntime Error: could not determine status of wsu_routerd -failing\n");
        return -3;
    }

    if (!parseCfg()) {
        /* validates args (even if passed in via cmd line */
        printf("\nConfiguration Error: invalid operating parameters\n");
        usage(argv[0]);
        return -1;
    }

    if (!startRouterd(&pid)) {
        printf("\nRuntime Error: could not properly start wsu_routerd - failing\n");
        return -4;
    }

    printf("\nStarting OBE MBR sender\n");
    while (mainloop) {
        usleep(ONE_SECOND);
        if (!mainloop) break;
        if (verbose) {
            gettimeofday(&tv, NULL);
            printf("\n*******************************************\n");
            printf("Processing MBR directory: time in %d\n", (int)tv.tv_sec);
        }
        process();
        if (verbose) {
            gettimeofday(&tv, NULL);
            printf("Done processing MBR directory: time out %d\n", (int)tv.tv_sec);
            printf("*******************************************\n\n");
        }
    }
    printf("\nExiting OBE MBR sender\n");

    stopRouterd(&pid);

    return 0;
}

