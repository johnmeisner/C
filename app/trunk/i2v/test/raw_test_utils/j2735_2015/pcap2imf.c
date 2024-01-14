/**************************************************************************
 *                                                                        *
 *     File Name:  pcap2imf.c  (immediate forward send utility)           *
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
//#include <net/if.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#define MAX_STR_LEN   200
#define MAX_FILE_SIZE 4096   /* current max for i2v imf */

#define MAX_WSM_PAYLOAD  1400

#define PCAP_MAGIC_NUM     0xA1B2C3D4
#define PCAP_LE_MAGIC_NUM  0xD4C3B2A1

/* arbitraty number below */
#define MAX_PCAP_RECORD  2000

#define MAIN_CLEANUP   free(databuf); \
                       fclose(f);

#define SLEEP_100_MS    100000
#define SLEEP_1_SEC     1000000
#define SLEEP_5_SEC     5000000
#define SLEEP_10_SEC    10000000
#define SLEEP_20_SEC    20000000

typedef enum {
    IFALSE = 0,
    ITRUE
} IBOOL;

/* courtesy wireshark */
typedef struct pcap_hdr_s {
    unsigned int magicNum;
    unsigned short versionMaj;
    unsigned short versionMin;
    signed int thiszone;
    unsigned int sigfigs;
    unsigned int snaplen;
    unsigned int network;
} __attribute__((packed)) pcap_hdr_t;

typedef struct pcaprec_hdr_s {
    unsigned int tsSec;
    unsigned int tsUsec;
    unsigned int capLen;
    unsigned int origLen;
} __attribute__((packed)) pcaprec_hdr_t;

static IBOOL sendOnce = ITRUE;
static int intervaltime = 0;
static struct sockaddr_in targetsvr;
static unsigned short int port = 0;
static char filename[MAX_STR_LEN] = {0};
static char msgfile[] = "pcap2imfmsg.amh";
static IBOOL mainloop = ITRUE;
static int fbcount = 0;    /* file bytes */
static char *databuf = NULL;
static IBOOL encoding;
static IBOOL bigendian = ITRUE;

static void sigHandler(int __attribute__((unused)) sig)
{
    mainloop = IFALSE;
}

static inline void adjustBE32(int *value)
{
    if (!bigendian) {
        *value = (((*value & 0xFF000000) >> 24) |
                  ((*value & 0x00FF0000) >> 8) | 
                  ((*value & 0x0000FF00) << 8) |
                  ((*value & 0x000000FF) << 24));
    }
}

static void usage(char *name) 
{
    char *idx = &name[strlen(name) - 1];
    int count = 0;
    char *printname, *basename = NULL;
    IBOOL useRegName = ITRUE;

    /* print name beautification */
    while (idx != name) {
        if (*idx == '/') {
            basename = (char *)malloc(count);
            if (basename != NULL) {
                memset(basename, 0, count);
                strcpy(basename, ++idx);
                useRegName = IFALSE;
            }
            break;
        }
        count++;
        idx--;
    }
    printname = (useRegName) ? name : basename;

    /* now to the actual usage */
    printf("\nUsage: %s -f <input file> -t <target IP> -p <target port> [ -F <1..5> ]\n", printname);
    printf("\n\twhere mandatory input args are:\n");
    printf("\t-f\tInput file name of pcap file data to send\n");
    printf("\t-t\tIP address of RSE\n");
    printf("\t-p\tListening port of RSE\n");
    printf("\t-e\tPCAP contents encoding\n");
    printf("\t\tSupported encoding = 0 (DER) or 1 (UPER)\n");
    printf("\n\tOptional args:\n");
    printf("\t-F\tInterval to use when repeating send\n");
    printf("\t\tif specifying an interval, use Ctrl+C to end send\n\n");

    if (!useRegName) {
        free(basename);
    }
}

static IBOOL parseArgs(int argc, char *argv[])
{
    char c;
    unsigned char interval = 0;
    IBOOL invalid = IFALSE;
    IBOOL validIP = IFALSE;
    IBOOL portSet = IFALSE;
    IBOOL fileSet = IFALSE;

    while ((c=getopt(argc, argv, "e:F:f:hp:t:")) != -1) {
        switch (c) {
            case 'e':
                /* repurposing interval */
                interval = (unsigned char)strtoul(optarg, NULL, 10);
                if (interval > 1) {
                    printf("\nInvalid encoding type: %s\n", optarg);
                    invalid = ITRUE;
                } else {
                    encoding = (interval) ? ITRUE : IFALSE;
                }
                break;
            case 'F':
                interval = atoi(optarg);
                if ((interval) && (interval <= 5)) {
                    sendOnce = IFALSE;
                    switch (interval) {
                        case 1: intervaltime = SLEEP_100_MS; break;
                        case 2: intervaltime = SLEEP_1_SEC; break;
                        case 3: intervaltime = SLEEP_5_SEC; break;
                        case 4: intervaltime = SLEEP_10_SEC; break;
                        case 5: intervaltime = SLEEP_20_SEC; break;
                    }
                } else {
                    invalid = ITRUE;
                }
                break;
            case 'f':   /* file */
                if (strlen(optarg) > MAX_STR_LEN - 1) {    /* need a null term at the end */
                    printf("\nInvalid file name - file name too long\n");
                    invalid = ITRUE;
                } else {
                    strcpy(filename, optarg);
                    fileSet = ITRUE;
                }
                break;
            case 'p':   /* port */
                port = (unsigned short int)strtoul(optarg, NULL, 0);
                portSet = ITRUE;
                break;
            case 't':   /* IP addr */
                if (!inet_aton(optarg, &targetsvr.sin_addr)) {
                    printf("\nInvalid address\n");
                    invalid = ITRUE;
                } else {
                    validIP = ITRUE;
                }
                break;
            case 'h':   /* help - but suppress getopt invalid arg print */
            default:
                invalid = ITRUE;
                break;
        }
        if (invalid) {
            return IFALSE;
        }
    }

    if (!validIP) {
        printf("\nYou must specify a RSE IP\n");
        return IFALSE;
    }

    if (!portSet) {
        printf("\nYou must specify a port\n");
        return IFALSE;
    }

    if (!fileSet) {
        printf("\nYou must specify a file\n");
        return IFALSE;
    }

    printf("\nusing mandatory args: ip[%s], port[%d] \n\tfile[%s]\n", inet_ntoa(targetsvr.sin_addr), port, filename);

    return ITRUE;
}

static IBOOL initSock(int *sock)
{
    if ((*sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
        printf("\nsocket creation failed\n");
        return IFALSE;
    }

    /* address already set */
    targetsvr.sin_family = AF_INET;
    targetsvr.sin_port = htons(port);

    if (connect(*sock, (struct sockaddr *)&targetsvr, sizeof(targetsvr)) < 0) {
        printf("\nconnect error to RSE\n");
        close(*sock);
        return IFALSE;
    }

    return ITRUE;
}

static IBOOL createDataFile(unsigned char *data, int length)
{
    FILE *f;
    int i, j;

    remove(msgfile);

    if (NULL == (f=fopen(msgfile, "w"))) {
        return IFALSE;
    }

    fprintf(f, "Version=0.6\n");
    if (!encoding) {
        /* use SPAT amh data type */
        fprintf(f, "Type=SPAT\nPSID=0x8002\n");
    } else {
        /* use TIM amh data type */
        fprintf(f, "Type=TIM\nPSID=0x8FF6\n");   /* fix later */
    }
    fprintf(f, "Priority=7\nTxMode=ALT\nTxChannel=SCH\nTxInterval=1\n");
    fprintf(f, "DeliveryStart=08/22/2015, 01:01\n");
    fprintf(f, "DeliveryStop=12/12/2016, 12:12\n");
    fprintf(f, "Signature=False\nEncryption=False\n");   /* fix later */

    /* payload bytes - don't want a really really really long string */
    for (i=0, j=1; i<length; i++, j++) {
        if (j==1) fprintf(f, "Payload1=");
        fprintf(f, "%.2X", data[i]);
        if (j < 20) {
            fprintf(f, ",");
        } else {
            fprintf(f, "\n");
            j = 0;   /* reset j for next line in file */
        }
    }
    if (j > 1) fprintf(f, "\n");   /* if j = 1, just printed \n */

    fclose(f);

    return ITRUE;
}

static IBOOL stageData(void)
{
    FILE *f;
    long size = 0;

    if (NULL == (f=fopen(msgfile, "r"))) {
        printf("\nError opening file %s\n", filename);
        return IFALSE;
    }

    fseek(f, 0, SEEK_END);
    if ((size = ftell(f)) > MAX_FILE_SIZE) {
        printf("\nFile %s is too big; max file size is %d bytes\n", filename, MAX_FILE_SIZE);
        fclose(f);
        return IFALSE;
    }

    if (NULL == (databuf = (char *)malloc(size))) {
        printf("\nUnable to allocate memory for transfer\n");
        fclose(f);
        return IFALSE;
    }
    memset(databuf, 0, size);

    rewind(f);
    if (size != fread(databuf, 1, size, f)) {
        printf("\nError reading file %s\n", filename);
        size = 0;
    }
    fclose(f);
    fbcount = (int)size;

    return (size) ? ITRUE: IFALSE;
}

/* hard coded offset for now */
static IBOOL getWsmUper(unsigned char *rawdat, int length, unsigned char *wsmdat, int *wsmlen)
{
    unsigned char *ptr = rawdat;
    int datalen;

    for (;ptr < rawdat + length; ptr++) {
        if ((ptr[0] == 0x88) && (ptr[1] == 0xdc) && (ptr[3] == 0xbf) &&
            ((ptr[4] == 0xe6) || (ptr[4] == 0xf6))) {
            /* assume valid package for BIM */
            datalen = *((unsigned short *)((void *)&ptr[15]));
            datalen = ntohs(datalen);
            if (datalen <= MAX_WSM_PAYLOAD) {
                memcpy(wsmdat, &ptr[17], datalen);
                *wsmlen = datalen;
                return ITRUE;
            }
        }
    }
    return IFALSE;
}

/* this may not support little endian */
/* MODUS OPERANDI -- look for WSM type (0x80)
   followed by 2 byte length of message
   followed by 0x30 followed by:
   if length <= 127, 1 byte j2735 length 2 bytes less than length
   if length > 127, 8X (X = num j2735 length bytes) and length modified bytes
*/
static IBOOL getWsmDer(unsigned char *rawdat, int length, unsigned char *wsmdat, int *wsmlen)
{
    unsigned char *ptr = rawdat;
    int datalen, multilen;

    for (;ptr < rawdat + length; ptr++) {
        if ((ptr[0] == 0x80) && (ptr[3] == 0x30)) {
            datalen = *((unsigned short *)((void *)&ptr[1]));
            datalen = ntohs(datalen);
            if (datalen > 127) {
                if ((ptr[4] | 0x8F) != 0x8F) {
                    /* greater than 127 needs 0x8X after 0x30, this not the wsm data */
                    continue;
                }
                multilen = ptr[4] & 0xF;
                if ((multilen > 2) || (!multilen)) {
                    /* another invalid case, too big wsm data, or no data */
                    continue;
                }
                *wsmlen = (multilen == 1) ? ptr[5] : *((unsigned short *)((void *)&ptr[5]));
                if ((*wsmlen + multilen + 2) == datalen) {    /* 2 bytes + X, length modified bytes in func preamble */
                    /* valid packet */
                    memcpy(wsmdat, &ptr[3], datalen);
                    *wsmlen = datalen;   /* this is the actual data length of the wsm */
                    return ITRUE;
                }
            } else {
                /* less than 127 */
                *wsmlen = ptr[4];
                if ((*wsmlen + 2) == datalen) {
                    /* valid packet */
                    memcpy(wsmdat, &ptr[3], datalen);
                    *wsmlen = datalen;   /* this is the actual data length of the wsm */
                    return ITRUE;
                }
            }
        }
    }

    return IFALSE;
}

static IBOOL getPcapData(FILE *f, unsigned char *databuf, int *datalen)
{
    IBOOL haveGood = IFALSE;
    unsigned char buf[sizeof(pcaprec_hdr_t)], tmpbuf[MAX_PCAP_RECORD];
    pcaprec_hdr_t *rec = (pcaprec_hdr_t *)buf;

    /* look for a record, assuming file positioned at start of a record */

    while (!haveGood && !feof(f)) {
        fread(buf, 1, sizeof(pcaprec_hdr_t), f);
        /* read size of record into databuf */
        adjustBE32(&rec->capLen);
        if (rec->capLen <= MAX_PCAP_RECORD) {
            fread(tmpbuf, 1, rec->capLen, f);
            haveGood = (encoding) ? getWsmUper(tmpbuf, rec->capLen, databuf, datalen) : 
                                    getWsmDer(tmpbuf, rec->capLen, databuf, datalen);
        } else {
            /* move file to next record */
            fseek(f, rec->capLen, SEEK_CUR);
        } 
    }

    return haveGood;
}

/* iterate through the pcap file */
static IBOOL mainprocess(int sock)
{
    FILE *f;
    unsigned char buf[MAX_WSM_PAYLOAD];
    int datalen, filesize;

    if (NULL == (f=fopen(filename, "r"))) {
        printf("\nError opening file %s\n", filename);
        return IFALSE;
    }
    fseek(f, 0, SEEK_END);
    filesize = ftell(f);
    fseek(f, 0, SEEK_SET);
    /* get pcap header */
    fread(buf, 1, sizeof(pcap_hdr_t), f);
    if (((pcap_hdr_t *)buf)->magicNum == PCAP_LE_MAGIC_NUM) {
        bigendian = IFALSE;
    }
    memset(buf, 0, sizeof(buf));   /* clear it out */

    while (!feof(f)) {
        /* get data and then create an amh file */
        if (!getPcapData(f, buf, &datalen)) {
            printf("\nError retreiving data from file %s\n", filename);
            MAIN_CLEANUP
            return IFALSE;
        }
        if (!createDataFile(buf, datalen)) {
            printf("\nInternal error\n");
            MAIN_CLEANUP
            return IFALSE;
        }

        /* package data in memory */
        if (!stageData()) {
            printf("\nFile or Memory error\n\n");
            MAIN_CLEANUP
            return IFALSE;
        }

        if (!fbcount) {
            printf("\nInternal error - invalid file - contents blank\n\n");
            MAIN_CLEANUP
            return IFALSE;
        }

        /* send data to rse */
        if (fbcount != write(sock, databuf, fbcount)) {
            printf("error sending...\n");
        } else {
            printf("send successful\n");
        }

        if (filesize == ftell(f)) break;   /* maybe only needed for cygwin/windows */
    }
    MAIN_CLEANUP

    return ITRUE;
}

int main(int argc, char *argv[])
{
    int sock = -1;
    struct sigaction sa;

    memset(&targetsvr, 0, sizeof(targetsvr));

    if (!parseArgs(argc, argv)) {
        usage(argv[0]);
        return IFALSE;
    }

    if (!initSock(&sock)) {
        printf("\nFailed to initialize socket\n\n");
        return IFALSE;
    }
    printf("\n");

    if (sendOnce) {
        mainprocess(sock);
    } else {
        /* register for interrupt signals */
        memset(&sa, 0, sizeof(sa));
        sa.sa_handler = sigHandler;
        sigaction(SIGINT, &sa, NULL);
        sigaction(SIGTERM, &sa, NULL);

        while (mainloop) {
            usleep(intervaltime);
            mainprocess(sock);
        }
    }

    close(sock);

    printf("\nExiting...\n\n");

    return ITRUE;
}

