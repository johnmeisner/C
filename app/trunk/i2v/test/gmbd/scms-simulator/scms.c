/**************************************************************************
 *                                                                        *
 *     File Name:  scms.c                                                 *
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

#include "defaults.h"

#define MY_CFG_FILE        "scms.conf"

#define PRINTF(fmt, args...)    if (debugOn && (dlog != NULL)) {fprintf(dlog, fmt, ##args); fflush(dlog);} printf(fmt, ##args);
#define LOGF(fmt, args...)      if (debugOn && (dlog != NULL)) {fprintf(dlog, fmt, ##args); fflush(dlog);}
#define CSVF(fmt, args...)      if (debugOn && (clog != NULL)) {fprintf(clog, fmt, ##args); fflush(clog);}

static BOOLEAN mainloop = TRUE;
static BOOLEAN debugOn = FALSE;
static BOOLEAN haveCfgDir = FALSE;
static BOOLEAN portSet = FALSE;
static unsigned char dbgcnt = 0;
static unsigned short int port = 0;
static unsigned char ipver = 0;
static short int numclients = -1;
static char cfgdir[MAX_STR_LEN] = {0};
static FILE *dlog = NULL, *clog = NULL;

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
    printf("\nUsage: %s [ -p <port> -v [ 4 | 6 ] -c <num clients> -d <directory> -D ] | -h\n", printname);
    printf("\nPress Ctrl+C to exit.\n");
    printf("\n\tDuplicate parameters ignored.\n");
    printf("\n\tAll parameters OPTIONAL (scms.conf used if param absent):\n");
    printf("\t-p\tPort - port to receive data on.\n");
    printf("\t-v\tIP Version - IP connection type to accept.\n");
    printf("\t\t4 = IPv4; 6 = IPv6; All others invalid.\n");
    printf("\t-c\tNumber of clients - max number of clients server supports.\n");
    printf("\t-d\tConfig directory - directory with scms.conf file.\n");
    printf("\t\tIf config directory not specified, current directory\n");
    printf("\t\tassumed to have scms.conf file.\n");
    printf("\t-D\tDebug option - debug logging.\n");
    printf("\t\tDebug option can be specified multiple times to turn up logging\n");
    printf("\t\t(ex: -D -D). Single entry enables scms.log in current directory.\n");
    printf("\t\tMore than one entry enables CSV logging (scms.csv).\n");
    printf("\t-h\tHelp - print this help message.\n\n");

    if (!useRegName) {
        free(basename);
    }
}

static BOOLEAN parseArgs(int argc, char *argv[])
{
    signed char c;

    while ((c=getopt(argc, argv, "c:Dd:hp:v:")) != -1) {
        switch (c) {
            case 'c':
                if (numclients < 0) {
                    numclients = (short int)strtol(optarg, NULL, 10);
                }
                break; 
            case 'D':
                debugOn = TRUE;
                if (dbgcnt < 0xFF) dbgcnt++;
                break;
            case 'd':
                haveCfgDir = TRUE;
                if (!strlen(cfgdir)) {
                    strcpy(cfgdir, optarg);
                }
                break;
            case 'p':
                if (!portSet) {
                    portSet = TRUE;
                    port = (unsigned short int)strtoul(optarg, NULL, 10);
                }
                break; 
            case 'h':
                return FALSE;
            case 'v':
                if (!ipver) {
                    ipver = (unsigned short int)strtoul(optarg, NULL, 10);
                    if ((ipver != 4) && (ipver != 6)) {
                        PRINTF("Warning: invalid IP version input ignored.\n");
                        ipver = 0;
                    }
                }
                break;
            default:
                break;
        }
    }
    return TRUE;
}

static BOOLEAN validateArgs(BOOLEAN nocfgfile)
{
    BOOLEAN incomplete = FALSE;
 
    /* port */
    if (!portSet && !nocfgfile) {
        PRINTF("\nWarning: port not specified, using default\n");
        port = DEFAULT_PORT;
    } else if (!portSet) {
        incomplete = TRUE;   /* no cmd line input, will wait for config file */
    } else {    /* input value from either cmd line or cfg file */
        /* really any above 49152 are 'reserved' as ephemeral ports */
        if (port == 0xFFFF) {
            PRINTF("\nInput error: reserved port specified [%d]; using default\n", port);
            port = DEFAULT_PORT;
        }
    }

    /* ip ver */
    if ((ipver != 4) && (ipver != 6)) {
        if (!nocfgfile) {
            PRINTF("\nWarning: IP version not specified or invalid [%d], using default\n", ipver);
            ipver = DEFAULT_IPVER;
        } else {
            incomplete = TRUE;
        }
    }

    /* num clients */
    if (numclients < 0) {
        if (!nocfgfile) {
            PRINTF("\nUsing default number of clients\n");
            numclients = DEFAULT_NUMCLIENTS;
        } else {
            incomplete = TRUE;
        }
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
            PRINTF("\nConfiguration Error: config file contents too long (%s %s %d)\n", key, value, count);
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
    char cfgfile[MAX_BYTE_STR + sizeof(MY_CFG_FILE)] = {0};
    char buf[MAX_LINE_LEN] = {0};
    char key[MAX_KEY_LEN] = {0};
    char value[MAX_BYTE_STR] = {0};

    if (!validateArgs(TRUE)) {
        PRINTF("\nUsing config file for missing parameters\n");
    } else {
        return TRUE;
    }

    if (strlen(cfgdir)) {
        sprintf(cfgfile, "%s/%s", cfgdir, MY_CFG_FILE);
    } else {
        strcpy(cfgfile, MY_CFG_FILE);
    }
    f = fopen(cfgfile, "r");
    if (NULL == f) {
        PRINTF("\nRuntime Error: cannot open config file (%s)\n", cfgfile);
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
        if (!portSet && !strcmp(key, "PORT")) {
            port = (unsigned short int)strtoul(value, NULL, 10);
            portSet = TRUE;
            continue;
        }
        if (!ipver && !strcmp(key, "VERSION")) {
            ipver = (unsigned char)strtoul(value, NULL, 10);
            continue;
        }
        if ((numclients < 0) && !strcmp(key, "CLIENTS")) {
            numclients = (short int)strtol(value, NULL, 10);
            continue;
        }
        if ((!debugOn) && !strcmp(key, "DEBUGMODE")) {
            dbgcnt = (unsigned char)strtoul(value, NULL, 10);
            debugOn = (dbgcnt) ? TRUE : FALSE;
            continue;
        }
    }
    fclose(f);

    return validateArgs(FALSE);
}

static BOOLEAN initSock(int *sock)
{
    int var = 0;
    struct sockaddr_in myaddr;
    struct sockaddr_in6 myaddr6;

    if (NULL == sock) return FALSE;

    if (ipver == 4) {
        if ((*sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            PRINTF("\nRuntime Error: socket creation failed\n");
            return FALSE;
        }
        memset(&myaddr, 0, sizeof(myaddr));
        myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
        myaddr.sin_family = AF_INET;
        myaddr.sin_port = htons(port);
    } else {
        if ((*sock = socket(AF_INET6, SOCK_STREAM, 0)) < 0) {
            PRINTF("\nRuntime Error: socket creation failed\n");
            return FALSE;
        }
        memset(&myaddr6, 0, sizeof(myaddr6));
        myaddr6.sin6_addr = in6addr_any;
        myaddr6.sin6_family = AF_INET6;
        myaddr6.sin6_port = htons(port);
    }

    /* this could be done in recvfrom call but here for any additional flags */
    if ((var = fcntl(*sock, F_GETFL, 0)) < 0) {
        PRINTF("\nRuntime Error: unable to modify socket\n");
        close(*sock);
        return FALSE;
    }
    var |= O_NONBLOCK;    /* graceful termination support */
    if (fcntl(*sock, F_SETFL, var) < 0) {
        PRINTF("\nRuntime Error: unable to modify socket\n");
        close(*sock);
        return FALSE;
    }

    if (ipver == 4) {
        if (bind(*sock, (struct sockaddr *)&myaddr, sizeof(myaddr)) < 0) {
            PRINTF("\nRuntime Error: could not bind to socket\n");
            close(*sock);
            return FALSE;
        }
    } else {
        if (bind(*sock, (struct sockaddr *)&myaddr6, sizeof(myaddr6)) < 0) {
            PRINTF("\nRuntime Error: could not bind to socket\n");
            close(*sock);
            return FALSE;
        }
    }
    if (listen(*sock, numclients) < 0) {
        PRINTF("\nRuntime Error: listen error\n");
        close(*sock);
        return FALSE;
    }
    return TRUE;
}

/* reused from i2v crc */
static unsigned short crcInternal(char *input, int size)
{
    unsigned char *data = (unsigned char *)input;

    unsigned short crctab[256] = {
        0x0000, 0x1189, 0x2312, 0x329B, 0x4624, 0x57AD, 0x6536, 0x74BF,
        0x8C48, 0x9DC1, 0xAF5A, 0xBED3, 0xCA6C, 0xDBE5, 0xE97E, 0xF8F7,
        0x1081, 0x0108, 0x3393, 0x221A, 0x56A5, 0x472C, 0x75B7, 0x643E,
        0x9CC9, 0x8D40, 0xBFDB, 0xAE52, 0xDAED, 0xCB64, 0xF9FF, 0xE876,
        0x2102, 0x308B, 0x0210, 0x1399, 0x6726, 0x76AF, 0x4434, 0x55BD,
        0xAD4A, 0xBCC3, 0x8E58, 0x9FD1, 0xEB6E, 0xFAE7, 0xC87C, 0xD9F5,
        0x3183, 0x200A, 0x1291, 0x0318, 0x77A7, 0x662E, 0x54B5, 0x453C,
        0xBDCB, 0xAC42, 0x9ED9, 0x8F50, 0xFBEF, 0xEA66, 0xD8FD, 0xC974,
        0x4204, 0x538D, 0x6116, 0x709F, 0x0420, 0x15A9, 0x2732, 0x36BB,
        0xCE4C, 0xDFC5, 0xED5E, 0xFCD7, 0x8868, 0x99E1, 0xAB7A, 0xBAF3,
        0x5285, 0x430C, 0x7197, 0x601E, 0x14A1, 0x0528, 0x37B3, 0x263A,
        0xDECD, 0xCF44, 0xFDDF, 0xEC56, 0x98E9, 0x8960, 0xBBFB, 0xAA72,
        0x6306, 0x728F, 0x4014, 0x519D, 0x2522, 0x34AB, 0x0630, 0x17B9,
        0xEF4E, 0xFEC7, 0xCC5C, 0xDDD5, 0xA96A, 0xB8E3, 0x8A78, 0x9BF1,
        0x7387, 0x620E, 0x5095, 0x411C, 0x35A3, 0x242A, 0x16B1, 0x0738,
        0xFFCF, 0xEE46, 0xDCDD, 0xCD54, 0xB9EB, 0xA862, 0x9AF9, 0x8B70,
        0x8408, 0x9581, 0xA71A, 0xB693, 0xC22C, 0xD3A5, 0xE13E, 0xF0B7,
        0x0840, 0x19C9, 0x2B52, 0x3ADB, 0x4E64, 0x5FED, 0x6D76, 0x7CFF,
        0x9489, 0x8500, 0xB79B, 0xA612, 0xD2AD, 0xC324, 0xF1BF, 0xE036,
        0x18C1, 0x0948, 0x3BD3, 0x2A5A, 0x5EE5, 0x4F6C, 0x7DF7, 0x6C7E,
        0xA50A, 0xB483, 0x8618, 0x9791, 0xE32E, 0xF2A7, 0xC03C, 0xD1B5,
        0x2942, 0x38CB, 0x0A50, 0x1BD9, 0x6F66, 0x7EEF, 0x4C74, 0x5DFD,
        0xB58B, 0xA402, 0x9699, 0x8710, 0xF3AF, 0xE226, 0xD0BD, 0xC134,
        0x39C3, 0x284A, 0x1AD1, 0x0B58, 0x7FE7, 0x6E6E, 0x5CF5, 0x4D7C,
        0xC60C, 0xD785, 0xE51E, 0xF497, 0x8028, 0x91A1, 0xA33A, 0xB2B3,
        0x4A44, 0x5BCD, 0x6956, 0x78DF, 0x0C60, 0x1DE9, 0x2F72, 0x3EFB,
        0xD68D, 0xC704, 0xF59F, 0xE416, 0x90A9, 0x8120, 0xB3BB, 0xA232,
        0x5AC5, 0x4B4C, 0x79D7, 0x685E, 0x1CE1, 0x0D68, 0x3FF3, 0x2E7A,
        0xE70E, 0xF687, 0xC41C, 0xD595, 0xA12A, 0xB0A3, 0x8238, 0x93B1,
        0x6B46, 0x7ACF, 0x4854, 0x59DD, 0x2D62, 0x3CEB, 0x0E70, 0x1FF9,
        0xF78F, 0xE606, 0xD49D, 0xC514, 0xB1AB, 0xA022, 0x92B9, 0x8330,
        0x7BC7, 0x6A4E, 0x58D5, 0x495C, 0x3DE3, 0x2C6A, 0x1EF1, 0x0F78
    };
    unsigned short crc = 0xFFFF;

    while (size--) {
        crc = (crc >> 8) ^ crctab[(crc ^ ((unsigned short)*data)) & 0xFF];
        data++;
    }

    return crc;
}

/* assumes 4 columns: time, time(usec), size, data */
static void writeDbgBytes(char *input, int size, struct timeval *tv)
{
#define WRITE_SIZE 10000
#define ARG_AND_SPACE 3
    char arg[ARG_AND_SPACE + 1], wbuf[WRITE_SIZE] = {0};
    int i, cnt;

    if ((NULL == input) || (NULL == tv)) return;

    CSVF("%d,%d,%d,",(int)tv->tv_sec, (int)tv->tv_usec, size);
    while (size > 0) {
        /* an fflush is called each CSVF, buffer data to minimize delays; ascii data with spaces */
        cnt = (size > WRITE_SIZE/ARG_AND_SPACE) ? WRITE_SIZE/ARG_AND_SPACE : size;
        for (i=0; i<cnt; i++, input++) {
            sprintf(arg, "%.2x ", *input);
            strcat(wbuf, arg);
        }
        if (size <= WRITE_SIZE/ARG_AND_SPACE) {
            wbuf[strlen(wbuf) - 1] = 0;    /* remove last space */
        }
        CSVF("%s",wbuf);
        size -= cnt;
    }
    CSVF(",\n");

#undef WRITE_SIZE
#undef ARG_AND_SPACE
}

static void process(int *sock, struct sockaddr *inclient)
{
#define CLIENT_TIMEOUT  2
    struct sockaddr_in *client;
    struct sockaddr_in6 *client6;
    char clientnm[MAX_STR_LEN] = {0};
    int input, initsec;
    unsigned short crc;
    char *ackptr, rdbuf[MAX_TCP_DATA];
    struct timeval tv;
    BOOLEAN keepgoing = TRUE;

    gettimeofday(&tv, NULL);
    if (dbgcnt > 1) {
        printf("\ndetected client\n");
        if (ipver == 4) {
            client = (struct sockaddr_in *)inclient;
            inet_ntop(AF_INET, &(client->sin_addr), clientnm, MAX_STR_LEN);
        } else {
            client6 = (struct sockaddr_in6 *)inclient;
            inet_ntop(AF_INET6, &(client6->sin6_addr), clientnm, MAX_STR_LEN);
        }
        LOGF("\nNew client: seconds - %d, microseconds - %d\n", (int)tv.tv_sec, (int)tv.tv_usec);
        LOGF("client data:\nidentity - %s\n", clientnm);
    }
    initsec = tv.tv_sec;

resume:
    if ((input = read(*sock, rdbuf, MAX_TCP_DATA)) <= 0) {
        if ((errno == EAGAIN) || (errno == EWOULDBLOCK) || !input) {
            gettimeofday(&tv, NULL);
            if ((tv.tv_sec - initsec) >= CLIENT_TIMEOUT) {
                /* timeout */
                if (dbgcnt > 1) LOGF("no more client data\n");
                close(*sock);
                return;
            } else {
                usleep(1000);
                goto resume;
            }
        } else if (input < 0) {
            if (dbgcnt) LOGF("Runtime error: failed in reading client data\n");
            close(*sock);
            return;
        }
    } else if (dbgcnt > 1) {
        gettimeofday(&tv, NULL);
        LOGF("client sent new data at %d %d\n", (int)tv.tv_sec, (int)tv.tv_usec);
    }
    /* ack is defined as last 16 bytes of received data returned back */
    if (input > ACK_LENGTH) {
        ackptr = &rdbuf[input - ACK_LENGTH];
        if (write(*sock, ackptr, ACK_LENGTH) < ACK_LENGTH) {
            if (dbgcnt) LOGF("Runtime error(%d %d(usec)): failed to write proper ack (%d) to client\n",
                             (int)tv.tv_sec, (int)tv.tv_usec, ACK_LENGTH);
            keepgoing = FALSE;
        }
    } else {
        /* will at least send something back */
        if (write(*sock, rdbuf, input) < input) {
            if (dbgcnt) LOGF("Runtime error(%d %d(usec)): failed to write proper ack (client limited size = %d) to client\n",
                             (int)tv.tv_sec, (int)tv.tv_usec, input);
            keepgoing = FALSE;
        }
    }
    crc = crcInternal(rdbuf, input);
    if (dbgcnt > 1) {
        LOGF("client bytes - %d\n", input);
        LOGF("crc of bytes - %#.2x\n", crc);
        switch(dbgcnt) {
            case 2:
                writeDbgBytes(rdbuf, input, &tv);
                break;
            /* add any new csv logging here */
            default:
                break;
        }
    }
    if (keepgoing) {
        /* update initsec to allow more time for client */
        gettimeofday(&tv, NULL);
        initsec = tv.tv_sec;
        goto resume;
    }

    /* close the sock (no further client interaction */
    close(*sock);
#undef CLIENT_TIMEOUT
}

int main(int argc, char *argv[])
{
    int size, var, csock, sock = -1;
    struct sigaction sa;
    struct sockaddr_in client;
    struct sockaddr_in6 client6;
    struct sockaddr *clientptr;
    struct timeval tv;

    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = sigHandler;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    if (!parseArgs(argc, argv)) {
        usage(argv[0]);
        return 1;
    }

    if (!parseCfg()) {
        /* validates args (even if passed in via cmd line */
        printf("\nConfiguration Error: invalid operating parameters\n");
        usage(argv[0]);
        return -1;
    }

    /* create log files if debug enabled */
    if (debugOn) {
        if (NULL == (dlog = fopen("scms.log", "w+"))) {
            printf("\nDebug Error: unable to create debug log\n");
        } else {
            gettimeofday(&tv, NULL);
            LOGF("\nSCMS simulator starting up.\n");
            LOGF("*******************************\n");
            LOGF("Port = %d   IP version = %d   Max clients = %d\n", port, ipver, numclients);
            LOGF("*******************************\n");
            LOGF("Initial time = %d (sec) %d (uSec)\n\n", (int)tv.tv_sec, (int)tv.tv_usec);
        }
        if (NULL == (clog = fopen("scms.csv", "w+"))) {
            printf("\nDebug Error: unable to create csv log\n");
        } else {
            /* csv header may vary per supported log level */
            switch (dbgcnt) {
                case 2:
                    CSVF("Time(Sec),Time(uSec),Data Size,Data,\n");
                    break;
                /* add any new csv logging here */
                default:
                    break;
            }
        }
    }

    printf("\nRunning...\n");

    if (ipver == 4) {
        size = sizeof(client);
        memset(&client, 0, sizeof(client));
        clientptr = (struct sockaddr *)&client;
    } else {
        size = sizeof(client6);
        memset(&client6, 0, sizeof(client6));
        clientptr = (struct sockaddr *)&client6;
    }

    if (!initSock(&sock)) {
        return -2;
    }

    while (mainloop) {
        if ((csock = accept(sock, clientptr, (socklen_t *)&size)) >= 0) {
            /* mark csock nonblocking */
            if ((var = fcntl(csock, F_GETFL, 0)) < 0) {
                if (debugOn) PRINTF("Internal runtime accept error...\n");
                close(csock);
                continue;
            }
            var |= O_NONBLOCK;
            if (fcntl(csock, F_SETFL, var) < 0) {
                if (debugOn) PRINTF("Internal runtime accept error 2...\n");
                close(csock);
                continue;
            }
            process(&csock, clientptr);
            continue;
        }

        if ((errno != EAGAIN) && (errno != EWOULDBLOCK)) {
            if (debugOn) {
                PRINTF("Internal runtime receive error...\n");
            }
        }
        usleep(1000);
    }

    PRINTF("\nExiting...\n");
    if (debugOn) {
        gettimeofday(&tv, NULL);
        LOGF("Closing time = %d (sec) %d (uSec)\n\n", (int)tv.tv_sec, (int)tv.tv_usec);
    }

    close(sock);
    if (debugOn) {
        if (NULL != dlog) fclose(dlog);
        if (NULL != clog) fclose(clog);
    }

    return 0;
}

