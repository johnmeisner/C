/**************************************************************************
 *                                                                        *
 *     File Name:  pcapreplay.c                                           *
 *     Author:                                                            *
 *         DENSO International America, Inc.                              *
 *         North America Research Laboratory, California Office           *
 *         3252 Business Park Drive                                       *
 *         Vista, CA 92081                                                *
 *                                                                        *
 **************************************************************************/
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <net/if.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <fcntl.h>
#include <ctype.h>
#include <netdb.h>

#include "pcapreplay.h"

#define MY_CFG_FILE        "replay.conf"
#define CACHE_SIZE         50000

static BOOLEAN mainloop = TRUE;
static BOOLEAN haveCfgDir = FALSE;
static BOOLEAN iterate = FALSE;
static BOOLEAN portSet = FALSE;
static BOOLEAN bigendian = TRUE;
static int sendsock = -1;
static unsigned short targetPort = 0xFFFF;
static int fileoffset = 0;
static int recoffset = 0;         /* for records larger than input buffer */
static int cacheoffset = 0;
static int cachebytes = 0;
static int cacherectoobig = 0;
static char cache[CACHE_SIZE] = {0};
static char cfgdir[MAX_STR_LEN] = {0};
/* directory plus name allocation for pcapfile */
static char pcapfile[2 * MAX_STR_LEN] = {0};
static char targetIP[MAX_STR_LEN] = {0};

static void sigHandler(int sig)
{
    if (sig == SIGCHLD) {
        printf("Child exited\n");
    } else {
        printf("Received termination signal\n");
    }
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
    printf("\nUsage: %s [ -f <pcap file> -d <directory> -a <IP address> -p <port> -i ] | -h\n", printname);
    printf("\nPress Ctrl+C to exit.\n");
    printf("\n\tDuplicate parameters ignored.\n");
    printf("\n\tAll parameters OPTIONAL (replay.conf used if param absent):\n");
    printf("\t-f\tPCAP file - fully qualified path and file to use.\n");
    printf("\t-d\tConfig directory - directory with replay.conf file.\n");
    printf("\t\tIf config directory not specified, current directory\n");
    printf("\t\tassumed to have replay.conf file.\n");
    printf("\t-a\tTarget IP address - used to indicate where to send data.\n");
    printf("\t-p\tTarget port - port used for sent data.\n");
    printf("\t-i\tEnable iteration - restart from beginning upon end of pcap file.\n");
    printf("\t-h\tHelp - print this help message.\n\n");

    if (!useRegName) {
        free(basename);
    }
}

static BOOLEAN parseArgs(int argc, char *argv[])
{
    signed char c;

    while ((c=getopt(argc, argv, "a:d:f:hip:")) != -1) {
        switch (c) {
            case 'a':
                if (!strlen(targetIP)) {
                    strcpy(targetIP, optarg);
                }
                break;
            case 'd':
                haveCfgDir = TRUE;
                if (!strlen(cfgdir)) {
                    strcpy(cfgdir, optarg);
                }
                break;
            case 'f':
                if (!strlen(pcapfile)) {
                    strcpy(pcapfile, optarg);
                }
                break;
            case 'i':
                iterate = TRUE;
                break;
            case 'h':
                return FALSE;
            case 'p':
                portSet = TRUE;
                targetPort = (unsigned short)strtoul(optarg, NULL, 10);
                break;
            default:
                break;
        }
    }
    return TRUE;
}

static BOOLEAN validateArgs(void)
{
    BOOLEAN incomplete = FALSE;
 
    if (!strlen(pcapfile) || !strlen(targetIP) || !portSet) {
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
    char cfgfile[MAX_BYTE_STR + sizeof(MY_CFG_FILE)] = {0};
    char buf[MAX_LINE_LEN] = {0};
    char key[MAX_KEY_LEN] = {0};
    char value[MAX_BYTE_STR] = {0};
    char *ptr;
    struct stat st;

    if (!validateArgs()) {
        printf("\nUsing config file for missing parameters\n");
    } else {
        return TRUE;
    }

    if (strlen(cfgdir)) {
        sprintf(cfgfile, "%s/%s", cfgdir, MY_CFG_FILE);
    } else {
        /* try /rwflash/config as well (if nothing in local dir) */
        strcpy(cfgfile, MY_CFG_FILE);
        if (stat(cfgfile, &st)) {
            /* set file to /rwflash/config - not in local dir */
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
        if (!strlen(pcapfile) && !strcmp(key, "PCAPFILE")) {
            strcpy(pcapfile, value);
            /* lower case only support from config file */
            ptr = pcapfile;
            ptr += strlen(pcapfile) - 1;
            for (; ptr >= pcapfile; ptr--) {
                *ptr = tolower(*ptr);
            }
            continue;
        }
        if (!iterate && !strcmp(key, "ENABLEITERATE")) {
            if (!strcmp(value, "YES")) {
                iterate = TRUE;
            } else if (strcmp(value, "NO")) {
                /* a value other than no */
                printf("\nConfiguration Error: invalid iteration flag, ignoring\n");
            }
            continue;
        }
        if (!strlen(pcapfile) && !strcmp(key, "IPADDRESS")) {
            strcpy(targetIP, value);
            continue;
        }
        if (!portSet && !strcmp(key, "PORT")) {
            targetPort = (unsigned char)strtoul(value, NULL, 10);
            portSet = TRUE;
            continue;
        }
    }
    fclose(f);

    return validateArgs();
}

static BOOLEAN initSock(void)
{
    int var;
    struct sockaddr_in myaddr;

    if ((sendsock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
        printf("\nRuntime Error: socket creation failed\n");
        return FALSE;
    }

    memset(&myaddr, 0, sizeof(myaddr));
    inet_pton(AF_INET, targetIP, &myaddr.sin_addr);
    myaddr.sin_family = AF_INET;
    myaddr.sin_port = htons(targetPort);

    if (connect(sendsock, (struct sockaddr *)&myaddr, sizeof(myaddr)) < 0) {
        printf("\nRuntime error: unable to connect for response data\n");
        close(sendsock);
        sendsock = -1;
        return FALSE;
    }
    return TRUE;
}

/* 
    retrieves one packet at a time from a capture file
    bytesread is ONLY capture data bytes - don't include pcap header;
    bytesread may have an input value ONLY when there is moretoread
    iterate = self explanatory
    moretoread = assumes in value is always false UNLESS picking up from a previous
                 read that had too much data for buffer; val is only set TRUE in the
                 case of a packet that is more than bufsize (on exit)
*/
static BOOLEAN getData(FILE *f, char *buf, int bufsize, int inoffset, int *outoffset, int *bytesread, 
    BOOLEAN iterate, BOOLEAN *moretoread, int *timestampSec, int *timestampuSec)
{
    BOOLEAN hasCapLen    = FALSE;
    BOOLEAN refreshCache = FALSE;
#define PCAP_MAGIC_NUM 0xA1B2C3D4
#define PCAP_LE_MAGIC_NUM 0xD4C3B2A1
    if ((NULL == f) || (NULL == buf) || (NULL == outoffset) || (NULL == bytesread) || (NULL == moretoread)) {
        return FALSE;
    }
    if (*moretoread && (!inoffset || !recoffset)) {
        return FALSE;
    }

    if (!cachebytes) {
        /* read file */
        cachebytes = fread(cache, 1, CACHE_SIZE, f);
        /* determine file endianness */
        /* FIXME: really below should check for more than mismatch - mismatch
           could be endianness OR it could be libpcap version - for this
           implementation, will only support the existing libpcap */
        if (((pcap_hdr_t *)cache)->magicNum != PCAP_MAGIC_NUM) {
            if (((pcap_hdr_t *)cache)->magicNum == PCAP_LE_MAGIC_NUM) {
                printf("confirmed littleendian pcap file\n");
                bigendian = FALSE;
            }
            else {
                printf("invalid PCAP magic number - neither big or little endian\n");
            }             
        }
        inoffset = cacheoffset = sizeof(pcap_hdr_t);
        fileoffset = 0;
    }

    if (cacherectoobig && (inoffset >= cacherectoobig)) {
        cacherectoobig = 0;
        *moretoread = FALSE;   /* really an invalid state if set but ignore it */
    }

    if (inoffset >= (fileoffset + cachebytes)) {
        /* need to read more bytes */
        if (feof(f)) {
            if (*moretoread) {
                /* invalid state */
                return FALSE;
            }
            if (iterate) {
                /* reset inoffset to start of file after header */
                inoffset = (cacheoffset = sizeof(pcap_hdr_t));
                rewind(f);
                fileoffset = 0;
                cachebytes = 0;
                cacherectoobig = 0;
            } else {
                /* not a failure condition - just return no data */
                *bytesread = 0;
                if (NULL != outoffset) outoffset = 0;
                return TRUE;
            }
        }
        if (*moretoread) {
            if (cacherectoobig) {
                /* cacheoffset = 0 */
                cacheoffset = 0;
            } else {
                /* need to 'backup' to start of record first */
                fseek(f, (fileoffset + (recoffset - sizeof(pcaprec_hdr_t))), SEEK_SET);
                /* cacheoffset set to relative position 
                   it previously had with respect to the new fileoffset */
                cacheoffset -= (recoffset - sizeof(pcaprec_hdr_t));
                cachebytes = (recoffset - sizeof(pcaprec_hdr_t));
            }
        }
        fileoffset += cachebytes;
        cachebytes = fread(cache, 1, CACHE_SIZE, f);
    } else if (inoffset < fileoffset) {
        /* jump support - inoffset is less than what is in cache */
        if (!iterate || *moretoread) {
            /* invalid state */
            return FALSE;
        }
        fseek(f, inoffset, SEEK_SET);
        fileoffset = inoffset;
        cacheoffset = 0;
        cachebytes = fread(cache, 1, CACHE_SIZE, f);
    } else {
        /* position cacheoffset to desired inoffset relative to fileoffset */
        cacheoffset = (inoffset - fileoffset);
    }

    /* now need to check if cache has all bytes for this request */
    if (cacherectoobig) {
        /* special handling - cacherectoobig has offset for end of bytes
           to obtain for full record */
        if (feof(f)) {
            /* invalid state */
            return FALSE;
        }
        if ((cachebytes - cacheoffset) < bufsize) {
            /* read in more from file */
            fileoffset += cacheoffset;
            fseek(f, fileoffset, SEEK_SET);
            cachebytes = fread(cache, 1, CACHE_SIZE, f);
            cacheoffset = 0;
        }
        if ((cacheoffset + bufsize) >= (cacherectoobig - fileoffset)) {
            /* have reached end of too big record */
            cacherectoobig = 0;
            *moretoread = FALSE;
            *bytesread = ((cacherectoobig - cacheoffset) > bufsize) ? bufsize : (cacherectoobig - cacheoffset);
        } else {
            *moretoread = TRUE;
            *bytesread = bufsize;
            recoffset = *bytesread;  /* don't care value for cacherectoobig - needs to be > 0 for function */
        }
        memcpy(buf, &cache[cacheoffset], *bytesread);
        cacheoffset += *bytesread;
        if(NULL != outoffset) *outoffset = (cacheoffset + fileoffset);
        return TRUE;
    }
    if (*moretoread) {
        /* 'backup' to start of record (recoffset - sizeof(pcaprec_hdr_t)) */
        cacheoffset = recoffset - sizeof(pcaprec_hdr_t);
    } else {
        /* processing a new packet */
        if ((cacheoffset + (int) sizeof(pcaprec_hdr_t)) > cachebytes) {
            /* cache is missing packet header */
            refreshCache = TRUE;
        } else {
            /* cache contains packet header */
            if (bigendian) {
                *bytesread = ((pcaprec_hdr_t *)&cache[cacheoffset])->capLen;
            } else {
                *bytesread = (((((pcaprec_hdr_t *)&cache[cacheoffset])->capLen & 0xFF000000) >> 24) |
                             ((((pcaprec_hdr_t *)&cache[cacheoffset])->capLen & 0x00FF0000) >>   8) | 
                             ((((pcaprec_hdr_t *)&cache[cacheoffset])->capLen & 0x0000FF00) <<   8) | 
                             ((((pcaprec_hdr_t *)&cache[cacheoffset])->capLen & 0x000000FF) << 24)); 
            }
            hasCapLen = TRUE;

            if ((cacheoffset + (int) sizeof(pcaprec_hdr_t) + *bytesread) > cachebytes) {
                /* but cache is missing packet data */
                refreshCache = TRUE;
            }
        }
        if (refreshCache) {
            /* refresh cache as current packet was only partially stored */
            fileoffset += cacheoffset;
            fseek(f, fileoffset, SEEK_SET);
            cachebytes = fread(cache, 1, CACHE_SIZE, f);
            cacheoffset = 0;
            if (!hasCapLen) {
                if (bigendian) {
                    *bytesread = ((pcaprec_hdr_t *)&cache[cacheoffset])->capLen;
                } else {
                    *bytesread = (((((pcaprec_hdr_t *)&cache[cacheoffset])->capLen & 0xFF000000) >> 24) |
                         ((((pcaprec_hdr_t *)&cache[cacheoffset])->capLen & 0x00FF0000) >>   8) | 
                         ((((pcaprec_hdr_t *)&cache[cacheoffset])->capLen & 0x0000FF00) <<   8) | 
                         ((((pcaprec_hdr_t *)&cache[cacheoffset])->capLen & 0x000000FF) << 24)); 
                }
            }
        }
    }

    if (*bytesread > (int)(CACHE_SIZE - sizeof(pcaprec_hdr_t))) {
        /* too big cache rec */
        cacherectoobig = fileoffset + cacheoffset + sizeof(pcaprec_hdr_t) + *bytesread;
    }
    if ((NULL != timestampSec) && (NULL != timestampuSec)) {
        if (bigendian) {
            *timestampSec = ((pcaprec_hdr_t *)&cache[cacheoffset])->tsSec;
            *timestampuSec = ((pcaprec_hdr_t *)&cache[cacheoffset])->tsUsec;
        } else {
            *timestampSec = (((((pcaprec_hdr_t *)&cache[cacheoffset])->tsSec & 0xFF000000) >> 24) |
                             ((((pcaprec_hdr_t *)&cache[cacheoffset])->tsSec & 0x00FF0000) >>  8) | 
                             ((((pcaprec_hdr_t *)&cache[cacheoffset])->tsSec & 0x0000FF00) <<  8) | 
                             ((((pcaprec_hdr_t *)&cache[cacheoffset])->tsSec & 0x000000FF) << 24)); 

            *timestampuSec = (((((pcaprec_hdr_t *)&cache[cacheoffset])->tsUsec & 0xFF000000) >> 24) |
                              ((((pcaprec_hdr_t *)&cache[cacheoffset])->tsUsec & 0x00FF0000) >>  8) | 
                              ((((pcaprec_hdr_t *)&cache[cacheoffset])->tsUsec & 0x0000FF00) <<  8) | 
                              ((((pcaprec_hdr_t *)&cache[cacheoffset])->tsUsec & 0x000000FF) << 24)); 
        }
    }
    cacheoffset += sizeof(pcaprec_hdr_t);
    /* make sure cache has all of the bytes */
    if ((*bytesread > (cachebytes - cacheoffset)) && !cacherectoobig) {
        /* not enough - need to read - read from start of this record */
        fileoffset += (cacheoffset - sizeof(pcaprec_hdr_t));
        fseek(f, fileoffset, SEEK_SET);
        cachebytes = fread(cache, 1, CACHE_SIZE, f);
        cacheoffset = sizeof(pcaprec_hdr_t);
        if (*bytesread > bufsize) recoffset = cacheoffset;
    }
    if (*moretoread) {
        *bytesread -= (cacheoffset - recoffset);
    }
    if (*bytesread > bufsize) {
        *moretoread = TRUE;
        *bytesread = bufsize;
    } else {
        *moretoread = FALSE;
        recoffset = 0;
    }
    memcpy(buf, &cache[cacheoffset], *bytesread);
    cacheoffset += *bytesread;
    if (NULL != outoffset) *outoffset = (cacheoffset + fileoffset);
    return TRUE;

#undef PCAP_MAGIC_NUM
}

/* data retrieved from pcap file may be signed or unsigned and may not be
   BSMs, looking specifically for BSMs (80 01 02 81 26); not checking if data is
   signed - data will either be signed or unsigned based on input config;
   to make sure the BSM indication is valid, looking first for hex: 88 dc; */ 

static void mainProcess(void)
{
    char *ptr, buf[MAX_WSM], sendbuf[MAX_WSM]; 
    int i, bytesread, offset = 0, sendlength = 0;
    BOOLEAN tooBig = FALSE;
    int sleeptime, tsec = 0, tusec = 0;
    int ltsec = 0, ltusec = 0;   /* timestamp for last packet sent */
    struct timeval tv;
    struct tm t;
    FILE *f = NULL;

    if (NULL == (f=fopen(pcapfile, "r"))) {
        printf("\nRuntime Error: unable to open pcap file: %s\n", pcapfile);
        return;
    }

    if (!initSock()) {
        /* log messages already should be provided */
        return;
    }

    while (mainloop) {
        memset(buf, 0, sizeof(buf));
        memset(sendbuf, 0, sizeof(sendbuf));
        bytesread = 0;
        /* processing a new packet */
        if (!getData(f, buf, sizeof(buf), offset, &offset, &bytesread, 
                      iterate, &tooBig, &tsec, &tusec)) {
            if (iterate) {
                printf("Failure to obtain data from pcapfile\n");
                usleep(1000);
                continue;
            } else {
                printf("Failure to obtain data from pcapfile - exiting\n");
                return;
            }
        }
        if (tooBig) {
            /* toss and move on - tooBig = moretoread from getData */
            while (tooBig) {
                if (!getData(f, buf, sizeof(buf), offset, &offset, &bytesread,
                         iterate, &tooBig, NULL, NULL)) {
                    /* not considering iterate in this case - junk data indicated
                       by the fact it is too big (or wrong message type in pcap file)
                       - just exit */
                    printf("Failure to obtain data from capfile - exiting\n");
                    return;
                }
            }
            tooBig = FALSE;
            continue;
        }
        if (!bytesread) {
            if (iterate) {
                /* this should never happen */
                printf("invalid runtime state - continuing\n");
                usleep(1000);
                continue;
            } else {
                printf("no more bytes in file - exiting\n");
                return;
            }
        }
        /* now have data - data includes everything, not just UDP payload - begin 
           search as described above; note that getData protects against endianness,
           this function assumes network byte order and does not support endianness */
        /* courtesy wireshark, UDP data is 43 bytes offset from start header; length is capture length - 42 */
        ptr = &buf[42];   /* zero based */
        /* wait to send wsm to 'about' the difference between last packet and
           current packet from pcap - it isn't exact because of processing time */
        if (ltsec || ltusec) {
            /* calculate total sleep - don't forget about possible iterate (rollback) */
            if (tsec < ltsec) {
                /* file iteration, clock rolled back; just make it 1/10 second */
                sleeptime = ONE_SECOND/10;
            } else {
                sleeptime = (tsec - ltsec) * ONE_SECOND;
            }
            if (ltusec > tusec) {
                /* do not allow sleeptime to go negative */
                sleeptime = ((ltusec - tusec) > sleeptime) ? sleeptime : sleeptime - (ltusec - tusec);
            } else {
                sleeptime += (tusec - ltusec);
            }
            /* update last timestamp */
            ltusec = tusec;
            ltsec = tsec;
            if (mainloop && (sleeptime > 0)) usleep(sleeptime);
        } else {
            /* don't sleep (this is the first packet) */
            ltusec = tusec;
            ltsec = tsec;
        }
        if (!mainloop) break;

        sendlength = bytesread - 42;
        if (sendlength > MAX_WSM) {
            printf("\nRuntime error: invalid data length obtained\n");
            if (iterate) {
                continue;
            } else {
                printf("Exiting on error...\n");
                break;
            }
        }
        memcpy(sendbuf,ptr,sendlength);

        /* send data */
        if (sendlength != write(sendsock, sendbuf, sendlength)) {
            printf("\nRuntime error: send error (%s)\n", strerror(errno));
        }
    }

    close(sendsock);
    fclose(f);
}

int main(int argc, char *argv[])
{
    struct sigaction sa;
    struct timeval tv;

    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = sigHandler;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGCHLD, &sa, NULL);

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

    printf("\nRunning...\n");
    /* enter main process (will exit when program terminates) */
    mainProcess();

    printf("\nExiting...\n");

    return 0;
}

