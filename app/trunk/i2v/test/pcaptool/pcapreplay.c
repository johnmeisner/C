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
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <fcntl.h>
#include <ctype.h>

#include "ris.h"
#include "ris_struct.h"
#include "smiifdefs.h"
#include "smicommon.h"
#include "smi.h"
#include "smiif.h"
#include "tps_api.h"

#include "pcapreplay.h"

#define MY_CFG_FILE        "resend.conf"
#define CACHE_SIZE         50000

#define PRINTF(fmt, args...)    if (debugOn && (dlog != NULL)) {fprintf(dlog, fmt, ##args); fflush(dlog);} printf(fmt, ##args);
#define LOGF(fmt, args...)      if (debugOn && (dlog != NULL)) {fprintf(dlog, fmt, ##args); fflush(dlog);}
#define CSVF(fmt, args...)      if (debugOn && (clog != NULL)) {fprintf(clog, fmt, ##args); fflush(clog);}

static BOOLEAN mainloop = TRUE;
static BOOLEAN debugOn = FALSE;
static BOOLEAN haveCfgDir = FALSE;
static BOOLEAN useSecurity = FALSE;
static BOOLEAN iterate = FALSE;
static BOOLEAN radioSet = FALSE;
static BOOLEAN bigendian = TRUE;
static BOOLEAN usedbgloc = FALSE;
static BOOLEAN usepsid = FALSE;
static unsigned char radio = 0;
static unsigned char channel = 0;
static unsigned char dbgcnt = 0;
static unsigned int overridepsid = 0;
static double dbglatitude = -1;
static double dbglongitude = -1;
static double dbgelevation = -1;
static wsmTypes wsmtype = BSM;    /* hard coded for now - add config support when new wsms to be processed are included */
static int fileoffset = 0;
static int recoffset = 0;         /* for records larger than input buffer */
static int cacheoffset = 0;
static int cachebytes = 0;
static int cacherectoobig = 0;
static char cache[CACHE_SIZE] = {0};
static char cfgdir[MAX_STR_LEN] = {0};
static char secdir[MAX_STR_LEN] = {0};
/* directory plus name allocation for pcapfile */
static char pcapfile[2 * MAX_STR_LEN] = {0};
static FILE *dlog = NULL, *clog = NULL;

static void sigHandler(int sig)
{
    if (sig == SIGCHLD) {
        PRINTF("Child exited\n");
    } else {
        PRINTF("Received termination signal\n");
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
    printf("\nUsage: %s [ -f <pcap file> -d <directory> -i -p <psid> -s\n", printname);
    printf("\t -r <radio num> -c <radio channel> -e <security directory> -D\n");
    printf("\t -g -x longitude -y latitude -z elevation ] | -h\n");
    printf("\nPress Ctrl+C to exit.\n");
    printf("\n\tDuplicate parameters ignored.\n");
    printf("\n\tAll parameters OPTIONAL (resend.conf used if param absent):\n");
    printf("\t-f\tPCAP file - fully qualified path and file to use.\n");
    printf("\t-d\tConfig directory - directory with resend.conf file.\n");
    printf("\t\tIf config directory not specified, current directory\n");
    printf("\t\tassumed to have resend.conf file.\n");
    printf("\t-i\tEnable iteration - restart from beginning upon end of pcap file.\n");
    printf("\t-p\tPSID - override PSID for all broadcast messages.\n");
    printf("\t-s\tEnable security - WSMs will be signed and then broadcast.\n");
    printf("\t-r\tRadio Number - WSU radio to use.\n");
    printf("\t-c\tRadio Channel - Channel to broadcast messages.\n");
    printf("\t-e\tSecurity Directory - only valid if security enabled.\n");
    printf("\t-D\tDebug option - debug logging.\n");
    printf("\t\tDebug option can be specified multiple times to turn up logging.\n");
    printf("\t\t(ex: -D -D). Single entry makes resend.log in current directory.\n");
    printf("\t\tMore than one entry enables CSV logging (resend.csv).\n");
    printf("\t-g\tDebug GPS - use hardcoded GPS location.\n");
    printf("\t-x\tLongitude - for debug GPS (only valid when -g set first).\n");
    printf("\t-y\tLatitude - for debug GPS (only valid when -g set first).\n");
    printf("\t-z\tElevation - for debug GPS (only valid when -g set first).\n");
    printf("\t-h\tHelp - print this help message.\n\n");

    if (!useRegName) {
        free(basename);
    }
}

static BOOLEAN parseArgs(int argc, char *argv[])
{
    signed char c;

    while ((c=getopt(argc, argv, "c:Dd:e:f:ghip:r:sx:y:z:")) != -1) {
        switch (c) {
            case 'c':
                if (!channel) channel = (unsigned char)strtoul(optarg, NULL, 10);
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
            case 'e':
                if (!strlen(secdir)) {
                    strcpy(secdir, optarg);
                }
                break;
            case 'f':
                if (!strlen(pcapfile)) {
                    strcpy(pcapfile, optarg);
                }
                break;
            case 'g':
                usedbgloc = TRUE;
                break;
            case 'i':
                iterate = TRUE;
                break;
            case 'p':
                if (!usepsid) {
                    overridepsid = strtoul(optarg, NULL, 16);
                    usepsid = TRUE;
                }
                break;
            case 'r':
                if (!radioSet) {
                    radio = (unsigned char)strtoul(optarg, NULL, 10);
                    radioSet = TRUE;
                }
                break;
            case 's':
                useSecurity = TRUE;
                break;
            case 'x':
                if (usedbgloc && (dbglongitude < -1)) {
                    dbglongitude = strtod(optarg, NULL);
                }
                break;
            case 'y':
                if (usedbgloc && (dbglatitude < -1)) {
                    dbglatitude = strtod(optarg, NULL);
                }
                break;
            case 'z':
                if (usedbgloc && (dbgelevation < -1)) {
                    dbgelevation = strtod(optarg, NULL);
                }
                break;
            case 'h':
                return FALSE;
            default:
                break;
        }
    }
    return TRUE;
}

static BOOLEAN validateArgs(void)
{
    BOOLEAN incomplete = FALSE;
 
    if (!strlen(pcapfile) || !radioSet || !channel) {
        incomplete = TRUE;
    }
    /* consider validating channel here??? */

    /* hopefully user doesn't list everything on cmd line excluding
       security mode */

    if (usedbgloc && ((dbglongitude < -1) || (dbglatitude < -1) || (dbgelevation < -1))) {
        incomplete = TRUE;
    }

#ifndef WSU_5001
    if (useSecurity && !strlen(secdir)) {
        incomplete = TRUE;
    }
#endif /* WSU_5001 */

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
    char *ptr;
    struct stat st;

    if (!validateArgs()) {
        PRINTF("\nUsing config file for missing parameters\n");
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
        if (!useSecurity && !strcmp(key, "SECURITYMODE")) {
            if (!strcmp(value, "ON")) {
                useSecurity = TRUE;
            } else if (strcmp(value, "OFF")) {
                /* a value other than off */
                PRINTF("\nConfiguration Error: invalid security mode, ignoring\n");
            }
            continue;
        }
        if (!iterate && !strcmp(key, "ENABLEITERATE")) {
            if (!strcmp(value, "YES")) {
                iterate = TRUE;
            } else if (strcmp(value, "NO")) {
                /* a value other than no */
                PRINTF("\nConfiguration Error: invalid iteration flag, ignoring\n");
            }
            continue;
        }
        if (!usepsid && !strcmp(key, "USEPSID")) {
            if (!strcmp(value, "YES")) {
                usepsid = TRUE;
            } else if (strcmp(value, "NO")) {
                PRINTF("\nConfiguration Error: invalid psid flag, ignoring\n");
            }
            continue;
        }
        if (usepsid && !strcmp(key, "PSID")) {
            overridepsid = strtoul(value, NULL, 16);
            continue;
        }
        if (!radioSet && !strcmp(key, "RADIO")) {
            radio = (unsigned char)strtoul(value, NULL, 10);
            radioSet = TRUE;
            continue;
        }
        if (!channel && !strcmp(key, "CHANNEL")) {
            channel = (unsigned char)strtoul(value, NULL, 10);
            continue;
        }
        if (!strlen(secdir) && !strcmp(key, "SECURITYDIRECTORY")) {
            strcpy(secdir, value);
            /* lower case only support from config file */
            ptr = secdir;
            ptr += strlen(secdir) - 1;
            for (; ptr >= secdir; ptr--) {
                *ptr = tolower(*ptr);
            }
            continue;
        }
        if ((!debugOn) && !strcmp(key, "DEBUGMODE")) {
            dbgcnt = (unsigned char)strtoul(value, NULL, 10);
            debugOn = (dbgcnt) ? TRUE : FALSE;
            continue;
        }
        if (!usedbgloc && !strcmp(key, "DEBUGGPS")) {
            if (!strcmp(value, "ON")) {
                usedbgloc = TRUE;
            } else if (strcmp(value, "OFF")) {
                PRINTF("\nConfiguration Error: invalid debug GPS setting, ignoring\n");
            }
            continue;
        }
        if (usedbgloc) {
            if ((dbglatitude < -1) && !strcmp(key, "LATITUDE")) {
                dbglatitude = strtod(value, NULL);
            }
            continue;
            if ((dbglongitude < -1) && !strcmp(key, "LONGITUDE")) {
                dbglongitude = strtod(value, NULL);
            }
            continue;
            if ((dbgelevation < -1) && !strcmp(key, "ELEVATION")) {
                dbgelevation = strtod(value, NULL);
            }
            continue;
        }
    }
    fclose(f);

    return validateArgs();
}

#ifndef WSU_5001
static int gpsLocation(double *latitude, double *longitude, double *elevation)
{
    /* misnomer - the lat, long, elev are not always dbg */
    *latitude = dbglatitude;
    *longitude = dbglongitude;
    *elevation = dbgelevation;
    return 0;
}

static void tpsCallback(tpsDataType *data)
{
    if (!usedbgloc && data->valid) {
        dbglatitude = data->latitude;
        dbglongitude = data->longitude;
        dbgelevation = data->altitude;
    }
}
#endif /* !WSU_5001 */

static BOOLEAN initSecurity(void)
{
    int ret;
    wsmConfigType wsmcfg;
    smiConfigParams smicfg;
    BOOLEAN loop = FALSE;
    /* a counter instead of infinite wait */
    int numretries = 0xFFFF;

    if (useSecurity) {
        /* initialization if required */
        if (dbglatitude < 0) dbglatitude = 0;
        if (dbglongitude < 0) dbglongitude = 0;
        if (dbgelevation < 0) dbgelevation = 0;

#ifndef WSU_5001
        wsuTpsInit();
        if ((ret = wsuTpsRegister(tpsCallback)) != TPS_SUCCESS) {
            PRINTF("TPS registration failed: %d\n", ret);
            return FALSE;
        }
#endif /* !WSU_5001 */
        memset(&smicfg, 0, sizeof(smicfg));
        memset(&wsmcfg, 0, sizeof(wsmcfg));
        wsmcfg.securityEnable = 1;
        wsmcfg.radioNum = radio;

        smicfg.wsm_version = 2;
        smicfg.txSecurity = 1;
        smicfg.enableTxRx = 1;    /* only care about sending */
        smicfg.forceSmToSendCerts = 3;
        smicfg.msgValDist = 1000;
        smicfg.genLocHorzConfMul = 1000;
        smicfg.genTimeConfMul = 1000;
        smicfg.messageExp_us = 1000000;
        smicfg.messageValPeriod_us = 1000000;
        smicfg.numSecPsids = 1;
        smicfg.PSID_sec[0] = 0x20;   /* using a default value */
        smicfg.useGenTime = 1;

#ifdef WSU_5001
        ret = smiInitV2(
              radio,
              FALSE,    /* debug */
              NULL,     /* debug file name */
              smi_verify_on_demand,
              NULL,     /* VOD function */
              smicfg.numUnsecPsids,
              smicfg.PSID_unsec,
              smicfg.numSecPsids,
              smicfg.PSID_sec,
              TRUE,     /* J2735 BSM format */
              NULL      /* cert change response function */
        );
#else
        ret = smiInitialize(
              &wsmcfg,
              gpsLocation,
              FALSE,    /* debug flag */
              2,        /* use tcp */
              NULL,     /* VOD function */
              smi_verify_unclassified,
              NULL,     /* wmh_chng_vehid callback */
              &smicfg,
              NULL,     /* smi_get_sign_results callback */
              FALSE     /* supportPerRVStats */
        );
#endif /* WSU_5001 */

        if (ret) {
            PRINTF("Failed to connect to security server: %d\n", ret);
            return FALSE;
        }

        while (!loop) {
            if (smiIsOBEConnectedToSM()) {
                loop = TRUE;
                usleep(10000);  /* race issue observed in i2v with SM; copying delay code */
            } else {
                sleep(2);
                if (numretries) {
                    numretries--;
                } else {
                    break;
                }
            }
        }
        if (!loop) {
            PRINTF("Timed out waiting for security server\n");
            return FALSE;
        }
    }

    return TRUE;
}

static BOOLEAN initRadio(void)
{
    int ret;
    appCredentialsV2Type appCredential = {
        .cbServiceAvailable = NULL,
        .cbReceiveWSMV2 = NULL,
    };

    if ((ret = wsuRisInitRadioCommV2(&appCredential)) != RS_SUCCESS) {
        PRINTF("failed to init radio: %d\n", ret);
        return FALSE;
    }

    if (!initSecurity()) {
        /* error message already printed */
        wsuRisTerminateRadioComm();
    }
    return TRUE;
}

/* assumes 4 columns: time, time(usec), size, data */
static void writeDbgBytes(unsigned char *input, int size, struct timeval *tv)
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
                LOGF("invalid PCAP magic number - neither big or little endian\n");
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

static void processBSM(FILE *f)
{
#define MAX_REGISTRATIONS 20
    char *ptr, buf[MAX_WSM], bsmhex[] = {0x80, 0x01, 0x02, 0x81, 0x26}; 
    int i, bytesread, offset = 0;
    BOOLEAN tooBig = FALSE, bigBSM = FALSE;
    int sleeptime, secdelta = 0, newdelta = 0, tsec = 0, tusec = 0;
    int ltsec = 0, ltusec = 0;   /* timestamp for last packet sent */
    struct timeval tv;
    struct tm t;
    struct {
        unsigned int psid;
        BOOLEAN valid;
    } psidreg[MAX_REGISTRATIONS];
    UserServiceV2Type uService = {
        .radioNum = radio,
        .action = ADD,
        .userAccess = AUTO_ACCESS_UNCONDITIONAL,
        .servicePriority = 20,    /* hard coded (don't care */
        .wsaSecurity = (useSecurity) ? SECURED_WSA : UNSECURED_WSA,
        .lengthPsc = 0,
        .channelNumber = channel,
        .lengthAdvertiseId = 0,
        .linkQuality = 20,
        .immediateAccess = 0,
        .extendedAccess = 0,
    };
    outWSMV2Type wsm = {
        .channelNumber = channel,
        .dataRate = DR_6_MBPS,
        .txPwrLevel = 20,
        .txPriority = 2,
        .wsmpExpireTime = 0,
        .radioNum = radio,
        .channelInterval = 0,
        .security = (useSecurity) ? SECURITY_SIGNED : SECURITY_UNSECURED,
        .securityFlag = 1,   /* use full cert all the time */
        .peerMacAddress = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff},
        .wsmpHeaderExt = 0,

    };
    if (NULL == f) return;

    /* create csv log first - then perform setup to enter main loop */
    /* csv header may vary per supported log level */
    switch (dbgcnt) {
        case 2:
            CSVF("Time(Sec),Time(uSec),Data Size,Data,\n");
            break;
        /* add any new csv logging here */
        default:
            break;
    }

    memset(&psidreg, 0, sizeof(psidreg));

    while (mainloop) {
        memset(buf, 0, sizeof(buf));
        bytesread = 0;
        /* processing a new packet */
        if (!getData(f, buf, sizeof(buf), offset, &offset, &bytesread, 
                      iterate, &tooBig, &tsec, &tusec)) {
            if (iterate) {
                PRINTF("Failure to obtain data from pcapfile\n");
                usleep(1000);
                continue;
            } else {
                PRINTF("Failure to obtain data from pcapfile - exiting\n");
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
                    PRINTF("Failure to obtain data from capfile - exiting\n");
                    return;
                }
            }
            tooBig = FALSE;
            continue;
        }
        if (!bytesread) {
            if (iterate) {
                /* this should never happen */
                LOGF("invalid runtime state - continuing\n");
                usleep(1000);
                continue;
            } else {
                LOGF("no more bytes in file - exiting\n");
                return;
            }
        }
        /* now have data - data includes everything, not just wsm payload - begin 
           search as described above; note that getData protects against endianness,
           this function assumes network byte order and does not support endianness */
        /* 88dc SHOULD be at offset 20h - will look directly there */
        ptr = &buf[32];
        if ((*((unsigned short *)((void *)ptr))) != WSM_MSG_TYPE) {
            /* toss it */
            if (dbgcnt > 1) LOGF("ignoring invalid message type: %#.4x\n", (*((unsigned short *)((void *)&ptr))));
            continue;
        }
        /* after 88dc, 1 byte version, and then psid - psid can be up to 4 bytes but bsms supported
           psid is 0, 10, 20 (all hex) - if security enabled, overwrite psid 0 with 20 */
        ptr += 3;    /* remember ptr is at first byte of two byte 88dc */
        if (usepsid) {
            wsm.psid = uService.psid = overridepsid;
        } else {
            wsm.psid = *ptr;
            if (useSecurity && !wsm.psid) wsm.psid = 0x20;
            /* if security disabled, overwrite psid to 0 */
            if (!useSecurity) wsm.psid = 0;
            uService.psid = wsm.psid;
        }
        /* Now search for message set MSG_BasicSafetyMessage (BSM) in the 
           packet data. To find this message, look for a tag-length-value set 
           in the data that describes the message, expect sequence tag (0x30), 
           a 1-2 byte length field, depending on if its a short BSM or a 
           big BSM, and a value containing a BSM.  All BSMs start with Part I 
           data. Part I of a BSM has two sequence elements in it, "msgID" and 
           "blob1". The "msgID" sequence element must have bytes 
           {0x80, 0x01, 0x02} for its tag-length-value set.  The "blob1" 
           sequence element must have bytes {0x81, 0x26} for its tag-length.  
           The ptr position is advanced so on a match ptr[0] will have the 
           message set MSG_BasicSafetyMessage (BSM) length, and ptr+1 will be
           at the start of the MSB_BasicSafetyMessage (BSM) set. */
        for (ptr += 2; (ptr - buf) <= bytesread; ptr++) {
			/* short BSM (< 127) check */
			if ((ptr[1] == 0x30) && (!memcmp(&ptr[3], bsmhex, sizeof(bsmhex)))){
				goto senddata;
			}

			/* greater than 127 check */
			if ((ptr[1] == 0x30) && (!memcmp(&ptr[4], bsmhex, sizeof(bsmhex)))){
				bigBSM = TRUE;
				goto senddata;
			}
        }
        /* did not find data */
        if (dbgcnt > 1) LOGF("ignoring wsm; not a bsm\n");
        continue;
senddata:
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
            /* get secdelta (secdelta time from time of packet), ptr[13] is start of 2 byte secdelta;
               if bigBSM, ptr[16] is start of 2 byte secdelta */
            memset(&t, 0, sizeof(t));
            gmtime_r(((time_t *)((void *)&tsec)), &t);
            secdelta = t.tm_sec * ONE_SECOND + tusec;
            secdelta /= 1000;    /* get to milliseconds */
            if (bigBSM) {
                secdelta -= (*(unsigned short *)((void *)&ptr[16]));
            } else {
                secdelta -= (*(unsigned short *)((void *)&ptr[13]));
            }
            if (secdelta < 0) secdelta = 0;
            if (secdelta > 60000) secdelta = 60000;    /* no support for leap seconds - j2735 max without it */
            if (mainloop && (sleeptime > 0)) usleep(sleeptime);
        } else {
            /* don't sleep (this is the first packet) */
            ltusec = tusec;
            ltsec = tsec;
        }
        if (!mainloop) break;
        for (i=0; i<MAX_REGISTRATIONS; i++) {
            if (!psidreg[i].valid) {
                break;
            }
            if (psidreg[i].psid == uService.psid) {
                goto wsmsend;
            }
        }
        /* getting here means psid not already registered; i is index of next registration slot */
        if (i >= MAX_REGISTRATIONS) {
            LOGF("too many registrations; unable to support psid: %#x\n", uService.psid);
            continue;
        }
        /* make user request to add service and then send wsm */
        if (RS_SUCCESS != wsuRisUserServiceRequest(&uService)) {
            LOGF("failed to register service for sending wsm\n");
            continue;
        }
        psidreg[i].valid = TRUE;
        psidreg[i].psid = uService.psid;
wsmsend:
        /* ptr[0] has the MSG_BasicSafetyMessage (BSM) set length 
           ptr[1] is the start of the MSB_BasicSafetyMessage (BSM) set */
        memset(wsm.data, 0, sizeof(wsm.dataLength));
        wsm.dataLength = ptr[0];
		memcpy(wsm.data,&ptr[1], wsm.dataLength);

        /* overwrite secmark in bsm blob with current ms time - delta of orig time vs orig secmark */
        memset(&t, 0, sizeof(t));
        gettimeofday(&tv, NULL);
        gmtime_r(&tv.tv_sec, &t);
        if (secdelta > ((t.tm_sec * 1000) + (tv.tv_usec / 1000))) {
            /* protect against incorrect rollover */
            newdelta = 60000 + ((t.tm_sec * 1000) + (tv.tv_usec / 1000) - secdelta);
        } else {
            newdelta = (t.tm_sec * 1000) + (tv.tv_usec / 1000) - secdelta;
        }
        if (bigBSM) {
            (*(unsigned short *)((void *)&wsm.data[13])) = newdelta;  
        } else {
            (*(unsigned short *)((void *)&wsm.data[12])) = newdelta;
        }
        if (RS_SUCCESS != wsuRisSendWSMV2Req(&wsm)) {
            LOGF("failed to send bsm data\n");
        } else if (dbgcnt > 1) {
            gettimeofday(&tv, NULL);
            writeDbgBytes(wsm.data, wsm.dataLength, &tv);
        }

        bigBSM = FALSE;    /* always reset */
    }

#undef MAX_REGISTRATIONS
}

/* to handle common tasks */
static void mainProcess(void)
{
    FILE *f = NULL;
#ifndef WSU_5001
    pid_t pid = -1;
    union sigval val;
    int dummy;
#endif /* !WSU_5001 */

    if (NULL == (f=fopen(pcapfile, "r"))) {
        PRINTF("\nRuntime Error: unable to open pcap file: %s\n", pcapfile);
        return;
    }
#ifndef WSU_5001
    if (useSecurity) {
        /* start security module */
        if ((pid = fork()) == -1) {
            PRINTF("\nRuntime Error: unable to start security module\n");
            return;
        } else if (!pid) {
            /* child - start security module */
            if (execlp("EscSMPC.bin", "EscSMPC.bin", "-c", secdir, (char *)NULL) == -1) {
                PRINTF("\nRuntime Error: unable to launch security module: %s\n", strerror(errno));
                exit(-1);
            }
        } else {
            /* parent will pause to allow security module a chance to get going */
            sleep(2);
        }
    }
#endif /* !WSU_5001 */

    if (!initRadio()) {
        /* log messages already should be provided */
        return;
    }

    /* for now there is only one type of wsm supported - for future
       add a function similar to processBSM and include in this switch */
    switch (wsmtype) {
        case BSM: processBSM(f); break;
        default:
            PRINTF("\nRuntime Error: unsupported WSM type\n");
            break;
    }
    fclose(f);

#ifndef WSU_5001
    if (useSecurity) {
        val.sival_int = 0;
        sigqueue(pid, SIGTERM, val);
        /* wait 1 sec, then kill */
        sleep(1);
        kill(pid, SIGKILL);
        waitpid(pid, &dummy, 0);
    }
#endif /* !WSU_5001 */

    wsuRisTerminateRadioComm();
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

    /* create log files if debug enabled */
    if (debugOn) {
        if (NULL == (dlog = fopen("resend.log", "w+"))) {
            printf("\nDebug Error: unable to create debug log\n");
        } else {
            gettimeofday(&tv, NULL);
            LOGF("\nPCAP Resend starting up.\n");
            LOGF("*******************************\n");
            LOGF("using file %s", pcapfile);
            if (iterate) {
                LOGF(": with iteration enabled\n");
            } else {
                LOGF("\n");
            }
            switch (wsmtype) {
                case BSM: LOGF("sending BSMs only\n"); break;
                default: LOGF("invalid wsm type\n"); break;
            }
            if (useSecurity) {
                LOGF("Security enabled - all messages are signed\n");
            } else {
                LOGF("Security disabled\n");
            }
            LOGF("*******************************\n");
            LOGF("Initial time = %d (sec) %d (uSec)\n\n", (int)tv.tv_sec, (int)tv.tv_usec);
        }
        if (NULL == (clog = fopen("resend.csv", "w+"))) {
            PRINTF("\nDebug Error: unable to create csv log\n");
        }
    }

    printf("\nRunning...\n");
    /* enter main process (will exit when program terminates) */
    mainProcess();

    PRINTF("\nExiting...\n");
    if (debugOn) {
        gettimeofday(&tv, NULL);
        LOGF("Closing time = %d (sec) %d (uSec)\n\n", (int)tv.tv_sec, (int)tv.tv_usec);
    }

    if (debugOn) {
        if (NULL != dlog) fclose(dlog);
        if (NULL != clog) fclose(clog);
    }

    return 0;
}

