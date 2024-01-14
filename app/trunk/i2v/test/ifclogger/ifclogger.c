/**************************************************************************
 *                                                                        *
 *     File Name:  ifclogger.c                                            *
 *     Author:                                                            *
 *         DENSO International America, Inc.                              *
 *         North America Research Laboratory, California Office           *
 *         3252 Business Park Drive                                       *
 *         Vista, CA 92081                                                *
 *                                                                        *
 **************************************************************************/

/* the purpose of this file is to decide to use either capture_pcap or 
   capture_eth_pcap (eth ifc) to log all packets on the interface - this
   utility will then parse the resulting pcap file and format it 
   according to the RSU function spec v4 requirement of showing the
   time stamp and the packet data - the final file will not be supported 
   by programs like wireshark - it will contain packet data only but
   will list the files that wireshark supports */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <signal.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include "i2v_general.h"
#include "i2v_util.h"
#include "i2v_shm_master.h"
#include "ntcip-1218.h"
#if defined(MY_UNIT_TEST)
#define MAIN ifclog_main
#else
#define MAIN main
#endif

/*
 *  Normal exit state:
 *     | ifclogger | INFO |EXIT: error states: ret(0) ifclog(0x10800)
 */
#define IFCLOG_AOK  0
#define IFCLOG_SHM_INIT_FAIL -1
#define IFCLOG_ARGC_FAIL -2
#define IFCLOG_PCAP_CREATE_LINK_FAIL -3
//#define   -4
//#define   -5
//#define   -6
#define IFCLOG_IFC_UNKNOWN -7
#define IFCLOG_FINAL_LOG_FAIL -8
#define IFCLOG_FINAL_PARSE_LOG_FAIL  -9
#define IFCLOG_MKDIRP_FAIL -10
#define IFCLOG_RUNCAP_FAIL  -11
#define IFCLOG_SIG_HIT  -12
#define IFCLOG_NEXT_REC_NULL_PARMS  -13
#define IFCLOG_START_FORK_FAIL -14
#define IFCLOG_START_OPEN_FAIL -15
#define IFCLOG_START_PCAP_FAIL -16
#define IFCLOG_TERM_CHILD  -17
#define IFCLOG_ETH0_FAIL -18
#define IFCLOG_ETH1_FAIL -19
#define IFCLOG_DSRC0_FAIL  -20
#define IFCLOG_DSRC1_FAIL  -21
#define IFCLOG_CV2X0_FAIL  -22
#define IFCLOG_CV2X1_FAIL  -23
#define IFCLOG_FOPEN_OUTPUT_FAIL  -24
#define IFCLOG_FOPEN_INPUT_FAIL  -25
#define IFCLOG_GET_LOGNAME_NULL_PARMS  -26
#define IFCLOG_INPUT_DIR_TOO_LONG  -27
#define IFCLOG_IGNORE_IFC_ARG  -28
#define IFCLOG_IGNORE_RSU_ARG  -29
#define IFCLOG_IGNORE_TRACK_LINK_ARG  -30
#define IFCLOG_INVALID_IFC_INDEX  -31
#define IFCLOG_FREAD_FAIL    -32
#define IFCLOG_MALLOC_FAIL    -33
#define IFCLOG_IGNORE_RSUID_ARG -34

/* mandatory logging defines */
#define MY_ERR_LEVEL   LEVEL_PRIV
#define MY_NAME  "ifclogger"

#define DEFAULT_DIRECTORY IFC_COMPLETED_LOGS_DIR /* logmgr will manage at boot via conf. */
#define DEFAULT_TIMEOUT     1000000   /* 1 sec */
#define LOGGING_DIRECTORY IFC_INPROGRESS_LOGS_DIR /* ifclog will purge this at boot. */

#define MAX_DIR_FILE_SZ     RSU_IFCLOG_STORE_PATH_MAX  /* file name and path */
#define MIN_FILE_NM_SZ      RSU_IFCLOG_NAME_SIZE_MAX
#define MAX_DIR_NM_SZ        (MAX_DIR_FILE_SZ - MIN_FILE_NM_SZ - 2)  /* extra 2 for null and '/' */

/* Huge to make sure enough. 1218 allows for huge paths and names.*/
#define MAX_CMD_SIZE  (RSU_IFCLOG_STORE_PATH_MAX + RSU_IFCLOG_NAME_SIZE_MAX + 512)

#define INVALID_IFC_IDX      255

/* eth0 & eth1 supported. Therefore 6 interfaces as in i2v_types.h::I2V_NUM_SUPPORTED_IFCS. */
#define DEV_ETH   "eth"
#define DEV_DSRC0 "dsrc0"
#define DEV_DSRC1 "dsrc1"
#define DEV_CV2X0 "cv2x0" /* Only one CV2X radio in capture_pcap. */
#define DEV_CV2X1 "cv2x1"

#if defined(EXTRA_DEBUG)
#define OUTPUT_MODULUS  30  /* Seconds. */
#else
#define OUTPUT_MODULUS  300
#endif

typedef enum {
    LFALSE = 0,
    LTRUE = 1
} LBOOL;

typedef enum {
    IFC_NONE,
    IFC_ETH,
    IFC_DSRC0,
    IFC_DSRC1,
    IFC_CV2X0,
    IFC_CV2X1
} IFC_LIST;

/* courtesy wireshark */
typedef struct pcap_hdr_s {
    unsigned int magic_number;     /* magic number */
    unsigned short version_major;  /* major version number */
    unsigned short version_minor;  /* minor version number */
    signed int thiszone;           /* GMT to local correction */
    unsigned int sigfigs;          /* accuracy of timestamps */
    unsigned int snaplen;          /* max length of captured packets in octets */
    unsigned int network;          /* data link type */
} __attribute__((packed)) pcap_hdr_t;

/* courtesy wireshark */
typedef struct pcaprec_hdr_s {
    unsigned int ts_sec;       /* timestamp seconds */
    unsigned int ts_usec;      /* timestamp microseconds */
    unsigned int incl_len;     /* number of octets of packet saved in file */
    unsigned int orig_len;     /* actual length of packet */
} __attribute__((packed)) pcaprec_hdr_t;

static cfgItemsT parentcfg;
static IFC_LIST loggedifc = IFC_NONE;
static unsigned char ifcnum = INVALID_IFC_IDX;
static uint8_t ifcdir = 3; /* -m3 option : 1 = tx, 0 & 2 = rx, 3 = rx_tx. */
static LBOOL mainloop = LTRUE;
static char dirname[MAX_DIR_NM_SZ];
static char trackname[MAX_DIR_NM_SZ];
static int bytesRead = 0; 
static LBOOL noParse = LTRUE;
static LBOOL createTracker = LFALSE;

static interfaceLog_t * shm_interfaceLog_ptr = NULL; /* SHM. */
static int32_t current_iface_index = 0;
static int32_t sizeofrecording = 0;  /* KB */
static int32_t timeofrecording = 0;  /* minutes */
static int32_t directionofrecording = 0; /* in,out, combined, seperate */

static char_t rsuname[RSU_ID_LENGTH_MAX+1];
static int32_t debugEnable = 1;

//todo:have logmgr start us with globaldebug.
STATIC uint64_t ifclog_error_states = 0x0;

static void set_my_error_state(int32_t my_error)
{
  int32_t dummy = 0;
  dummy = abs(my_error);
  if((dummy < 64) && (0 < dummy)) {
      ifclog_error_states |= (uint64_t)(0x1) << (dummy - 1);
  }
  return;
}

static void init_ifclog_statics(void)
{
    strncpy(dirname, DEFAULT_DIRECTORY, sizeof(dirname));
    memset(trackname,'\0',MAX_DIR_NM_SZ);
    memset(&parentcfg,0x0,sizeof(parentcfg));
    loggedifc = IFC_NONE;
    ifcnum = INVALID_IFC_IDX;
    ifcdir = 3;
    mainloop = LTRUE;
    bytesRead = 0; 
    noParse = LTRUE;
    createTracker = LFALSE;
    current_iface_index = 0;
    ifclog_error_states=0x0;
    sizeofrecording = 0;  /* KB */
    timeofrecording = 0;  /* minutes */
    directionofrecording = 0;
    memset(rsuname,0x0,sizeof(rsuname));
    debugEnable = 1;
}

static void usage(char *myname) 
{
    /* this is a developer who wants things just a certain way */
    char *basename = myname, *cptr = myname + strlen(myname) - 1;

    for (; cptr != myname; cptr--) {
        if (*cptr == '/') {
            basename = cptr+1;
            break;
        }
    } 

    printf("\n\nusage:\t%s -i <interface> -n <interface index>\n", basename);
    printf("\n\t<interface> can be 'eth', 'dsrc0', or 'cv2x0'\n");
    printf("\t<interface index> is the nic number [0|1] for eth interfaces or\n");
    printf("\tif eth is the interface, interface index is 0 for eth0 or 1 for eth1\n");
    printf("\n\tOPTIONAL args: -d <directory> -r <RSU ID> -s -t <link name>\n");
    printf("\n\twhere <directory> is the desired output directory for the log file\n");
    printf("\tand <RSU ID> is the desired integer value [up to 999] for RSU ID\n");
    printf("\t-s will suppress creating a csv file with the pcap data\n");
    printf("\tand -t will create a tracking link at the input fully qualfied\n");
    printf("\t link name to the data file logging pcap data\n\n");
}

static void exitHandler(int __attribute__((unused)) sig)
{
    mainloop = LFALSE;
    set_my_error_state(IFCLOG_SIG_HIT);
}

/* returns LTRUE if more processing needed */
static LBOOL nextRecord(FILE *in_f, FILE *out_f)
{
    /* first column is blank in csv, second is timestamp, third is timestamp us, fourth is raw data */
    char *databuf = NULL;
    char *asciibuf = NULL;
    char *p = NULL;
    char timestamp[30];
    pcaprec_hdr_t headerbuf;
    time_t epochSeconds;
    struct tm epochTime;
    unsigned int i;

    if ((NULL == in_f) || (NULL == out_f)) {
        set_my_error_state(IFCLOG_NEXT_REC_NULL_PARMS);
        return LFALSE;
    }

    if (feof(in_f)) {
        /* done */
        return LFALSE;
    }

    if (!bytesRead) {
        fseek(in_f, sizeof(pcap_hdr_t), SEEK_SET);
    }

    if(0 == fread(&headerbuf, 1, (sizeof(pcaprec_hdr_t)), in_f)) {
        set_my_error_state(IFCLOG_FREAD_FAIL);
        return LFALSE;
    }
    bytesRead += sizeof(pcaprec_hdr_t);
    headerbuf.ts_sec = ntohl(headerbuf.ts_sec);
    headerbuf.ts_usec = ntohl(headerbuf.ts_usec);
    headerbuf.incl_len = ntohl(headerbuf.incl_len);
    headerbuf.orig_len = ntohl(headerbuf.orig_len);

    epochSeconds = (time_t)headerbuf.ts_sec;
    gmtime_r(&epochSeconds, &epochTime);
    strftime(timestamp, 30, "%Y%m%d-%H:%M:%S", &epochTime);

    if (NULL == (databuf=malloc(headerbuf.incl_len))) {
        I2V_DBG_LOG(LEVEL_INFO, MY_NAME, "error processing data file\n");
        set_my_error_state(IFCLOG_MALLOC_FAIL);
        return LFALSE;
    }
    if (NULL == (asciibuf=malloc(headerbuf.incl_len * 3 + 1))) {  /* 2 characters + 1 space */
        I2V_DBG_LOG(LEVEL_INFO, MY_NAME, "error processing data file\n");
        free(databuf);
        set_my_error_state(IFCLOG_MALLOC_FAIL);
        return LFALSE;
    }
    memset(databuf, 0, headerbuf.incl_len);
    memset(asciibuf, 0, headerbuf.incl_len * 3 + 1);

    if(0 == fread(databuf, 1, headerbuf.incl_len, in_f)){
        set_my_error_state(IFCLOG_FREAD_FAIL);
        free(databuf);
        free(asciibuf);
        return LFALSE;
    }
    bytesRead += headerbuf.incl_len;

    p = asciibuf;
    for (i=0; i<headerbuf.incl_len; i++) {
        sprintf(p, "%.2X ", databuf[i]);
        p += 3;
    }

    fprintf(out_f, " ,%s,0.%8d,%s\n", timestamp, headerbuf.ts_usec, asciibuf);

    free(databuf);
    free(asciibuf);
    
    return LTRUE;
}

static LBOOL startCmd(char_t *cmd, char_t *pcmd[], pid_t *pid)
{
#if defined(MY_UNIT_TEST)
    cmd = cmd;
    pcmd = pcmd;
    pid = pid;
#else
    int fd = -1;

    if ((*pid = fork()) == -1) {
        set_my_error_state(IFCLOG_START_FORK_FAIL);
        return LFALSE;
    } else if (*pid == 0) {   /* child */
        if (open(cmd, O_RDONLY) == -1) {
            I2V_DBG_LOG(LEVEL_INFO, MY_NAME, "Failed to initiate logging binary (%s)\n", cmd);
            set_my_error_state(IFCLOG_START_OPEN_FAIL);
            return LFALSE;
        }
        if ((fd=open("/dev/null", O_RDWR)) != -1) {
            dup2(fd, 0);
            dup2(fd, 1);
            dup2(fd, 2);
        }
        if (execvp(cmd, (char *const *)pcmd) == -1) {
            I2V_DBG_LOG(LEVEL_INFO, MY_NAME, "error starting pcap logging\n");
            set_my_error_state(IFCLOG_START_PCAP_FAIL);
            return LFALSE;
        }
        return LTRUE;
    }/* else {
        / * parent * /
    } */
#endif
    return LTRUE;
}

static void termCommand(pid_t *pid)
{
    union sigval val;
    int stat;

    val.sival_int = 0;
    sigqueue(*pid, SIGTERM, val);
    usleep(1000000);  /* wait 1 sec */
    sigqueue(*pid, SIGKILL, val);
    waitpid(*pid, &stat, WNOHANG);
    set_my_error_state(IFCLOG_TERM_CHILD);
}
static LBOOL createLink(char *workfile)
{
    char cmd[300] = {0};

    /* Create symbolic link and force it if exists. */
    snprintf(cmd,sizeof(cmd), "ln -f -s %s/%s %s", LOGGING_DIRECTORY, workfile, trackname);
    return (system(cmd)) ? LFALSE : LTRUE;
}
/**********************************************************/
// Both use /tmp so if loss of power will lose log.
// Need to check option bits to delete oldest or stop on full output dir.
//
//capture_pcap: 
//  will produce YYYY_MM_DD_HH_MM_DD_cv2x0_tx_rx_%d.pcap
//  need to change for 1218 spec
//  Will stop by itself with time or size.
//  Will move complete log to user location and prune dir size on start.
//  We could add a non-daemonized option to let it prune output folder for us.
//
// max 4096k or 1 min. 3 minutes total time
//    capture_pcap -m 3 -i DEV_CV2X0 -s 4096 -t 1 -T 3 &
//
// max 512K or 20 min per file. Total size of 8M or 60 minutes.
//    capture_pcap -m 3 -i DEV_CV2X0 -s 512 -x 8 -t 20 -T 60 &
//
// max 512K max allowed time. Total size 8M Max allowed time
//    capture_pcap -m 3 -i DEV_CV2X0 -s 512 -x 8 &
//
// max 1 min max allowed size. Max total size allowed, 3 minutes total time.
//    capture_pcap -m 3 -i DEV_CV2X0 -t 1 -T 3 & 
//
//tcpdump:
//  Will not stop and needs to be slain.
//  Time format and name of file is perfect
//  Capture only the amount we with auto roll over of logs.
//  Will not manage location folder and doesnt know of it. we need to manage.
//  We could call capture_pcap to do output file management.
//
// max 3600 seconds or 1 meg then update file with new timestamp. 10 copies of pcap used.
//     /usr/sbin/tcpdump -i eth0  -G 3600 -C 1m -W10  -s 0  -w /tmp/rsuID_eth0_Both_%Y%m%d_%H%M%S.pcap&
//
/**********************************************************/

//TODO: use argc, argv to enable/disable DEBUG since no I2V SHM here.
static int runCap(char *workfile, IFC_LIST loggedifc)
{
    int32_t ret = IFCLOG_AOK;
    pid_t pid;
    uint32_t debug_counter = 0; /* rolling. */
    char_t sizeKB_s[16];
    char_t time_s[16];
    char_t sizeMB_s[16];
    char_t direction_s[16];
    char_t location_name[RSU_IFCLOG_NAME_SIZE_MAX+16];
    char_t ethname[] = "/usr/sbin/tcpdump";
    char_t *ethcmd[] = {"tcpdump", 
                        "-i","eth0",
#if 0 /* LOGMGR manages size and time. */
                        "-G",time_s,
                        "-C","1m",
                        "-W10",
                        "-s","0",
                        "-w","/tmp/ifclogs/rsuID_eth0_Both_%Y%m%d_%H%M%S.pcap&",
#else
                        "-w",location_name ,
#endif
                        (char *)0};
    char radioname[] = "/usr/bin/capture_pcap";
    char *radiocmd[] = {"capture_pcap",
                        "-m", direction_s,
                        "-i", "DEV_CV2X0",
#if 0  /* LOGMGR manages size and time. */
                        "-s", sizeKB_s,
                        "-x", sizeMB_s,
                        "-t", time_s,
                        "-T", time_s,
#endif
                        "--filename", workfile,
                        (char *)0};

    memset(location_name,0x0,sizeof(location_name));
    snprintf(location_name,sizeof(location_name),"%s/%s",LOGGING_DIRECTORY,workfile);

    snprintf(sizeKB_s,sizeof(sizeKB_s),"%d", sizeofrecording * 1024);
    snprintf(sizeMB_s,sizeof(sizeMB_s),"%d", sizeofrecording);
    snprintf(time_s,sizeof(time_s),"%d", timeofrecording);

    switch(directionofrecording) {
        case 1: //in
           snprintf(direction_s,sizeof(direction_s),"%d",2);
        break;
        case 2: //out
           snprintf(direction_s,sizeof(direction_s),"%d",1);
        break;
//TODO: We cant do bi.
        case 3:  //bi-seperate
#if 0
           snprintf(direction_s,sizeof(direction_s),"%d", 4+2+1);
        break;
#endif
        case 4: //combined
           snprintf(direction_s,sizeof(direction_s),"%d", 2+1);
        break;
        default:
           I2V_DBG_LOG(LEVEL_INFO, MY_NAME, "Bad direction(%d) default to combined.\n", directionofrecording);
           snprintf(direction_s,sizeof(direction_s),"%d", 2+1);
        break;
    }
#if defined(EXTRA_DEBUG)
{
    int32_t i = 0;
    for(i=0;i<13 && (NULL != radiocmd[i]);i++) {
        I2V_DBG_LOG(LEVEL_INFO, MY_NAME, "radio[%d][%s]\n",i,radiocmd[i]);
    }
    for(i=0;i<13 && (NULL != ethcmd[i]);i++) {
        I2V_DBG_LOG(LEVEL_INFO, MY_NAME, "eth0[%d][%s]\n",i,ethcmd[i]);
    }
}
#endif
    /* create child thread for capture */
    switch(loggedifc) {
        case IFC_ETH: /* eth0 or eth1. */
            if (!startCmd(ethname, ethcmd, &pid)) {
                if(0 == ifcnum)
                    ret = IFCLOG_ETH0_FAIL;
                else
                    ret = IFCLOG_ETH1_FAIL;
            }
            break;
        case IFC_DSRC0: /* Cont mode. */
            if (!startCmd(radioname, radiocmd, &pid)) {
                ret = IFCLOG_DSRC0_FAIL;
            }
            break;
        case IFC_DSRC1: /* Alt mode only.*/
            if (!startCmd(radioname, radiocmd, &pid)) {
                ret = IFCLOG_DSRC1_FAIL;
            }
            break;
        case IFC_CV2X0:/* 5912 CV2X only has one radio. */
            if (!startCmd(radioname, radiocmd, &pid)) {
                ret = IFCLOG_CV2X0_FAIL;
            }
            break;
        case IFC_CV2X1:/* Doesn't exist today. */
            if (!startCmd(radioname, radiocmd, &pid)) {
                ret = IFCLOG_CV2X1_FAIL;
            }
            break;
        default:
            ret = IFCLOG_IFC_UNKNOWN;
            break;
    }

    if(ret == IFCLOG_AOK) {
        usleep(DEFAULT_TIMEOUT);   /* need to let logging process start */
        if ((createTracker) && (!createLink(workfile))) {
            I2V_DBG_LOG(LEVEL_INFO, MY_NAME, "Unable to create tracking link: workfile[%s]\n",workfile);
            termCommand(&pid);
            ret = IFCLOG_PCAP_CREATE_LINK_FAIL;
        }
    }

    if(ret == IFCLOG_AOK) {
        while (mainloop) {
            /* Since the logging will end at some point maybe overkill having this.*/
            if(0 == (debug_counter % OUTPUT_MODULUS)) {
                if(ifclog_error_states) { /* Nothing wrong so stay quiet. */
                    I2V_DBG_LOG(LEVEL_INFO, MY_NAME,"ifclog(0x%lx)\n",ifclog_error_states);
                }
            }
            debug_counter++; 
            /* nothing to do - will exit when catching term sig */
            usleep(DEFAULT_TIMEOUT);
#if defined(MY_UNIT_TEST)
            if(5 < debug_counter)
                mainloop = WFALSE;
#endif
        }

        /* moved to before child term - link needs to be removed immediately */
        if (createTracker) {
            remove(trackname);
        }

        /* terminate child */
        termCommand(&pid);

        /* any required file cleanup after stopping child */
        /* if (createTracker) {
            remove(trackname);
           } */
    }

    if (IFCLOG_AOK != ret) set_my_error_state(ret);
    return ret;
}

static LBOOL parseAndGenerateLog(char *datfile, char *outlog)
{
    /* datfile assumed in LOGGING_DIRECTORY */
    char infile[100];
    FILE *in_f, *out_f;

    sprintf(infile, "%s/%s", LOGGING_DIRECTORY, datfile);

    if (NULL == (in_f=fopen(infile, "r"))) {
        I2V_DBG_LOG(LEVEL_INFO, MY_NAME, "unable to read data file for final parsing\n");
        set_my_error_state(IFCLOG_FOPEN_INPUT_FAIL);
        return LFALSE;
    }
    if (NULL == (out_f=fopen(outlog, "w"))) {
        I2V_DBG_LOG(LEVEL_INFO, MY_NAME, "unable to create final file for final parsing\n");
        fclose(in_f);
        set_my_error_state(IFCLOG_FOPEN_OUTPUT_FAIL);
        return LFALSE;
    }

    /* write header */
    fprintf(out_f, "Dat file: %s,Timestamp,Timestamp Milliseconds,Data\n",infile);
    while (nextRecord(in_f, out_f));

    fclose(in_f);
    fclose(out_f);

    return LTRUE;
}

static int32_t mkdirP(char_t *dirname)
{
  int32_t ret = IFCLOG_AOK;
  char_t cmd[MAX_DIR_NM_SZ + 10];

  if(NULL == dirname) {
      I2V_DBG_LOG(LEVEL_INFO, MY_NAME, "mkdirP: NULL input.\n");
      ret = IFCLOG_MKDIRP_FAIL;
  } else {
#if defined(EXTRA_DEBUG)
      I2V_DBG_LOG(LEVEL_INFO, MY_NAME, "mkdirP: dirname[%s]\n",dirname);
#endif
      memset(cmd,0x0,sizeof(cmd));
      if(snprintf(cmd,sizeof(cmd), "mkdir -p %s", dirname) < 0) {
          I2V_DBG_LOG(LEVEL_INFO, MY_NAME, "mkdirP: snprintf() failed.\n");
          ret = IFCLOG_MKDIRP_FAIL;
      } else {
          if(system(cmd) < 0) {
              I2V_DBG_LOG(LEVEL_INFO, MY_NAME, "mkdirP: system() failed.\n");
              ret = IFCLOG_MKDIRP_FAIL;
          }
      }
  }
  if(ret < IFCLOG_AOK)
      set_my_error_state(ret);
  return ret;
}

static char *getLogName(IFC_LIST ifc, char *logname, LBOOL isPcap)
{
    time_t val;
    struct tm t, *tmptr;
    char sufx[5];   /* either csv or pcap */
    char_t direction[16];
    char_t ifcname[16];

    if (NULL == logname) {
        set_my_error_state(IFCLOG_GET_LOGNAME_NULL_PARMS);
        return logname;
    }
    switch(ifc) {
        case IFC_ETH:
            strcpy(ifcname,"eth0");
            break;
        case IFC_CV2X0:
            strcpy(ifcname,"cv2x0");
            break;
        default:
            strcpy(ifcname,"unknown");
        break;
    }
    switch(directionofrecording) {
        case 1: //in
          snprintf(direction,sizeof(direction),"%s","In");
        break;
        case 2: //out
           snprintf(direction,sizeof(direction),"%s","Out");
        break;
//TODO not yet supported.
        case 3:  //bi-seperate
#if 0
           snprintf(direction,sizeof(direction),"%s", "Seperate");
        break;
#endif
        case 4: //combined
           snprintf(direction,sizeof(direction),"%s", "Both");
        break;
        default:
           I2V_DBG_LOG(LEVEL_INFO, MY_NAME, "Bad direction(%d) default to combined.\n", directionofrecording);
           snprintf(direction,sizeof(direction),"%s", "Unknown");
        break;
    }
    if (isPcap) {
        strcpy(sufx, "pcap");
    } else {
        strcpy(sufx, "csv");
    }

    val = time(NULL);
    tmptr = gmtime_r(&val, &t);
    
    /* Ensure strings terminated. */
    rsuname[sizeof(rsuname)-1]     = '\0';
    ifcname[sizeof(ifcname)-1]     = '\0';
    direction[sizeof(direction)-1] = '\0';
    sufx[sizeof(sufx)-1]           = '\0';

    snprintf(logname,MIN_FILE_NM_SZ, "%s_%s_%s_%d%02d%02d_%02d%02d%02d.%s", 
                rsuname,
                ifcname,
                direction,
                (1900 + tmptr->tm_year),
                (1 + tmptr->tm_mon),
                tmptr->tm_mday,
                tmptr->tm_hour,
                tmptr->tm_min,
                tmptr->tm_sec,
                sufx);

    return logname;
}

static LBOOL parseArgs(int argc, char *argv[])
{
    int i;

    for (i=0; i<argc; i++) {
        if (argv[i][0] == '-') {
            switch (argv[i][1]) {
                case 'd':    /* directory */
                    if (strlen(argv[i+1]) < MAX_DIR_NM_SZ ) {
                        strncpy(dirname, argv[i+1], strlen(argv[i+1]));
                    } else {
                        set_my_error_state(IFCLOG_INPUT_DIR_TOO_LONG);
                        I2V_DBG_LOG(LEVEL_INFO, MY_NAME, "NOTICE: input directory name is too long, using default\n");
                    }
                    i++;
                    break;
                case 'i':    /* interface */
                    if (!strncmp(argv[i+1], DEV_ETH, 3)) {   /* only comparing 'eth' */
                        loggedifc = IFC_ETH;
                        i++;
                    } else if (!strncmp(argv[i+1], DEV_DSRC0, 6)) { /* "dsrc0" */
                        loggedifc = IFC_DSRC0;
                        ifcnum = 0; /* doesn't matter. */
                        i++;
                    } else if (!strncmp(argv[i+1], DEV_DSRC1, 6)) { /* "dsrc1" */
                        loggedifc = IFC_DSRC1;
                        ifcnum = 0; /* doesn't matter. */
                        i++;
                    } else if (!strncmp(argv[i+1], DEV_CV2X0, 6)) { /* "cv2x0" */
                        loggedifc = IFC_CV2X0;
                        ifcnum = 0; /* doesn't matter. */
                        i++;
                    } else if (!strncmp(argv[i+1], DEV_CV2X1, 6)) { /* "cv2x1" */
                        loggedifc = IFC_CV2X1;
                        ifcnum = 0; /* doesn't matter. */
                        i++;
                    } else {
                        I2V_DBG_LOG(LEVEL_INFO, MY_NAME, "ignoring interface arg %s\n", argv[i+1]);
                        set_my_error_state(IFCLOG_IGNORE_IFC_ARG);
                        i++;
                    }
                    break;
                case 'm':
                    ifcdir = (unsigned char)strtoul(argv[i+1], NULL, 10);
                    i++;
                    break;
                case 'n':    /* nic index */
                    ifcnum = (unsigned char)strtoul(argv[i+1], NULL, 10);
                    i++;
                    break;
                case 's':    /* suppress csv */
                    noParse = LTRUE;
                    break;
                case 't':    /* tracking link */
                    if (strlen(argv[i+1]) < MAX_DIR_NM_SZ) {
                        strncpy(trackname, argv[i+1], strlen(argv[i+1]));
                        createTracker = LTRUE;
                    } else {
                        I2V_DBG_LOG(LEVEL_INFO, MY_NAME, "NOTICE: input link name is too long, ignoring tracking option\n");
                        set_my_error_state(IFCLOG_IGNORE_TRACK_LINK_ARG);
                    }
                    i++;
                    break;
                case 'D':
                    directionofrecording = atoi(argv[i+1]);
                    i++;
                    break;
                case 'N':
                    if (strlen(argv[i+1]) < sizeof(rsuname)) {
                        strncpy(rsuname, argv[i+1], strlen(argv[i+1]));
                    } else { /* This means not NULL terminated and we have to truncate to include NULL. */
                        I2V_DBG_LOG(LEVEL_INFO, MY_NAME, "NOTICE: rsuname is too long, truncating\n");
                        strncpy(rsuname, argv[i+1], sizeof(rsuname));
                        rsuname[sizeof(rsuname)-1] = '\0';
                        set_my_error_state(IFCLOG_IGNORE_RSUID_ARG);
                    }
                    i++;
                    break;
                case 'S':
                    sizeofrecording = atoi(argv[i+1]);
                    if (sizeofrecording > 40) {
                        sizeofrecording = 40; //Meg
                        I2V_DBG_LOG(LEVEL_INFO, MY_NAME, "invalid file size specified - using default(%d)\n",sizeofrecording);
                        set_my_error_state(IFCLOG_IGNORE_RSU_ARG);
                    }
                    i++;
                    break;
                case 'T':
                    timeofrecording = atoi(argv[i+1]);
                    if (timeofrecording > 48) {
                        timeofrecording = 48; //Hours
                        I2V_DBG_LOG(LEVEL_INFO, MY_NAME, "invalid file time specified - using default(%d)\n",timeofrecording);
                        set_my_error_state(IFCLOG_IGNORE_RSU_ARG);
                    }
                    timeofrecording *= 60; //Hours to minutes
                    i++;
                    break;
                case 'X':
                    debugEnable = atoi(argv[i+1]);
                    i++;
                    break;
                default:
                    break;
            }
        }
    }

    if ((loggedifc == IFC_NONE) || (ifcnum == INVALID_IFC_IDX)) {
        I2V_DBG_LOG(LEVEL_INFO, MY_NAME, "invalid interface (%d) or index (%d)\n", loggedifc, ifcnum);
        set_my_error_state(IFCLOG_INVALID_IFC_INDEX);
        return LFALSE;
    }

    /* add trailing / for directory */
    i = strlen(dirname);
    if (dirname[i-1] != '/') {
        dirname[i] = '/';
        dirname[i+1] = 0;
    }

    return LTRUE;
}
void sendMIBfilename(char_t * logname)
{
//TODO: need to match by direction too.
  if((NULL != logname) && (NULL != shm_interfaceLog_ptr)){

            for(current_iface_index=0;current_iface_index<RSU_IFCLOG_MAX;current_iface_index++){
                if(0 == memcmp(shm_interfaceLog_ptr[current_iface_index].ifaceName, "eth0", 4)) {
#if defined(EXTRA_DEBUG)
                    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"interfaceLog: Found iface, index=%d.\n",current_iface_index);
#endif
                    shm_interfaceLog_ptr[current_iface_index].ifaceLogName_length = 0;
                    memset(shm_interfaceLog_ptr[current_iface_index].ifaceLogName,'\0',RSU_IFCLOG_NAME_SIZE_MAX);
                    if(strlen(logname) <= RSU_IFCLOG_NAME_SIZE_MAX) {
                        strncpy((char_t *)shm_interfaceLog_ptr[current_iface_index].ifaceLogName,logname,strlen(logname));
                        shm_interfaceLog_ptr[current_iface_index].ifaceLogName_length = strlen(logname);
                    }else{
                        strncpy((char_t *)shm_interfaceLog_ptr[current_iface_index].ifaceLogName,logname,RSU_IFCLOG_NAME_SIZE_MAX);
                        shm_interfaceLog_ptr[current_iface_index].ifaceLogName_length = RSU_IFCLOG_NAME_SIZE_MAX;
                    }
#if defined(EXTRA_DEBUG)
                    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"New ifclog[%d:%s]\n",current_iface_index,logname);
#endif
                    break;
                } else if(0 == memcmp(shm_interfaceLog_ptr[current_iface_index].ifaceName, "cv2x0", 5)) {
#if defined(EXTRA_DEBUG)
                    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"interfaceLog: Found iface, index=%d.\n",current_iface_index);
#endif
                    shm_interfaceLog_ptr[current_iface_index].ifaceLogName_length = 0;
                    memset(shm_interfaceLog_ptr[current_iface_index].ifaceLogName,'\0',RSU_IFCLOG_NAME_SIZE_MAX);
                    if(strlen(logname) <= RSU_IFCLOG_NAME_SIZE_MAX) {
                        strncpy((char_t *)shm_interfaceLog_ptr[current_iface_index].ifaceLogName,logname,strlen(logname));
                        shm_interfaceLog_ptr[current_iface_index].ifaceLogName_length = strlen(logname);
                    }else{
                        strncpy((char_t *)shm_interfaceLog_ptr[current_iface_index].ifaceLogName,logname,RSU_IFCLOG_NAME_SIZE_MAX);
                        shm_interfaceLog_ptr[current_iface_index].ifaceLogName_length = RSU_IFCLOG_NAME_SIZE_MAX;
                    }
#if defined(EXTRA_DEBUG)
                    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"New ifclog[%d:%s]\n",current_iface_index,logname);
#endif
                    break;
                }
            } /* for */
            if(RSU_IFCLOG_MAX <= current_iface_index) {
                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"interfaceLog: Did not find supported iface.\n");
                set_my_error_state(IFCLOG_IFC_UNKNOWN);
            }
  }
}
int MAIN(int argc, char *argv[])
{
    struct sigaction sa;
    char finalFile[MAX_DIR_FILE_SZ] = {0};
    char workFile[MAX_DIR_FILE_SZ] = {0};
    char logname[MIN_FILE_NM_SZ];
    char cmd[MAX_CMD_SIZE];
    int  ret = IFCLOG_AOK; /* AOK till proven FAIL or ERR */
    size_t shm_interfaceLog_size = sizeof(interfaceLog_t) * RSU_IFCLOG_MAX;

    init_ifclog_statics();

    i2vUtilEnableDebug(MY_NAME);
    i2vUtilEnableSyslog(LEVEL_INFO, MY_NAME);

    if (!parseArgs(argc, argv)) {
        usage(argv[0]);
        ret = IFCLOG_ARGC_FAIL;
        I2V_DBG_LOG(LEVEL_INFO, MY_NAME, "Bad argc & argv input: ret=%d\n",ret);
        set_my_error_state(ret);
    } else {
#if 0 /* Output is minimal and mostly to syslog. Leave enabled. */
        if(0 == debugEnable) {
           i2vUtilDisableDebug();
           i2vUtilDisableSyslog();
        } else {
           i2vUtilEnableDebug(MY_NAME);
           i2vUtilEnableSyslog(LEVEL_INFO, MY_NAME);
        }
#endif
        /* Final destination */
        if(IFCLOG_AOK != (ret = mkdirP(dirname))) {
            return abs(ret);
        }
        strncpy(workFile, getLogName(loggedifc, logname, LTRUE),sizeof(workFile));
        /* On going logging */
        if(IFCLOG_AOK != (ret = mkdirP(LOGGING_DIRECTORY))) {
            return abs(ret);
        }

        /* Open SHM to MIB. If it fails it's dont care. */
        if ((shm_interfaceLog_ptr = wsu_share_init(shm_interfaceLog_size, INTERFACE_LOG_SHM_PATH)) == NULL) {
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"interfaceLog: SHM init failed.\n");
            set_my_error_state(IFCLOG_SHM_INIT_FAIL);
            shm_interfaceLog_ptr = NULL;
        } else {
            sleep(1);
        }

        memset(&sa, 0, sizeof(sa));
        sa.sa_handler = exitHandler;
        sigaction(SIGINT, &sa, NULL);
        sigaction(SIGTERM, &sa, NULL);

        if (IFCLOG_AOK == (ret = runCap(workFile, loggedifc))) {
            /* Send MIB file name now that it is complete. */
            sendMIBfilename(logname);
            /* at this point there are log files - create RSU spec log file if parsing enabled */
            if (!noParse) {
                sprintf(finalFile, "%s%s", dirname, getLogName(loggedifc, logname, LFALSE));
                if (parseAndGenerateLog(workFile, finalFile)) {
                    I2V_DBG_LOG(LEVEL_INFO, MY_NAME, "Interface log file at %s\n", finalFile);
                } else {
                    I2V_DBG_LOG(LEVEL_INFO, MY_NAME, "Final interface log failed creation - data file at %s/%s\n", LOGGING_DIRECTORY, workFile);
                    set_my_error_state(IFCLOG_FINAL_PARSE_LOG_FAIL);
                }
            } else {
                /* Move file to final location for user. Name is decided at start time. */
                sprintf(finalFile, "%s%s", dirname, logname);
                memset(cmd, 0, sizeof(cmd));
                snprintf(cmd,sizeof(cmd), "mv %s/%s %s", LOGGING_DIRECTORY, workFile, finalFile);
                if(system(cmd) < 0) {
                    I2V_DBG_LOG(LEVEL_INFO, MY_NAME, "Final interface log failed creation - data file at %s/%s\n", LOGGING_DIRECTORY, workFile);
                    set_my_error_state(IFCLOG_FINAL_LOG_FAIL);
                } else {
                    I2V_DBG_LOG(LEVEL_INFO, MY_NAME, "Interface log file at %s\n", finalFile);
                }
            } /* If noParse */
        } else {
            if(mainloop) {
                I2V_DBG_LOG(LEVEL_INFO, MY_NAME, "runCap failed: ret=%d\n",ret);
                set_my_error_state(IFCLOG_RUNCAP_FAIL);
            }
        } /* If run capture ok */
    } /* If parse args ok */
    if(NULL != shm_interfaceLog_ptr) { /* Kill but not destroy. */
        wsu_share_kill(shm_interfaceLog_ptr, shm_interfaceLog_size);
    }
    I2V_DBG_LOG(LEVEL_INFO, MY_NAME, "Finished: ret(%d) ifclog(0x%lx)\n",ret,ifclog_error_states);
    /* Stop I2V_DBG_LOG output. Last chance to output to syslog. */
    i2vUtilDisableSyslog();
    i2vUtilDisableDebug();

    return abs(ret);
}

