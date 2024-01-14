/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Filename: capture_pcap.c                                                   */
/*  Purpose: PCAP capture utility                                             */
/*                                                                            */
/* Copyright (C) 2023 DENSO International America, Inc.                       */
/*                                                                            */
/* 050123 - Added -T option to limit total log time.                          */
/*        - Altered -x option to limit total bytes logged.                    */
/*        - Prior  -s and -t allowed indefinate logging which could be bad.   */
/*        - RSU can be on for months and don't want allow by accident.        */
/*        - OBU different since unlikely to operate for more than a day.      */
/*        - Added macros RECORDING_LOG_DIR & FINISHED_LOG_DIR.                */
/*        - RECORDING_LOG_DIR default is /tmp.                                */
/*        - FINISHED_LOG_DIR is /rwflash/I2V/ifclogs.                         */
/*        - Moving to /tmp spares emmc device but now can lose on reset.      */
/*----------------------------------------------------------------------------*/

/*
 * Program to capture PCAP data from the radio drivers.
 * The command line is as follows:
 *
 * capture_pcap [-i device][-m mask][-p prepend_to_file_name]
 *              [--path output_path][--filename absolute_output_file_name]
 *              [-s max_size_in_KB][-x max_total_size_in_MB]
 *              [-t max_log_file_open_time_in_minutes]
 *              [-T max_total_log_time_in_minutes]
 *              [-a server_ip_address][-b server_port_number]
 *              [-c server_ip_address_for_tx][-d server_port_number_for_tx]
 *     where mask = 1 for Tx, 2 for Rx, or 3 for both.
 *     Use mask = 7 to log Tx and Rx to separate files.
 *     Add 8 to the mask to ignore the -i parameter and log data from all
 *     devices together.
 *     Add 16 to the mask to add prism headers to the captured packets.
 *
 * The valid device names are anything 15 characters or less ending with a
 * digit (e.g. dsrc0, dsrc1). The digit corresponds to the radio number.
 * Currently, the Hercules supports radio 0 and radio 1. Any device name
 * starting with a character other than 'C' or 'c' specifies the DSRC radio.
 *
 * To specify the C-V2X radio, specify a device name starting with a 'C' or a
 * 'c'.
 *
 * For all of the comments that follow, I am going to use dsrc0 and dsrc1 only
 * to prevent them from becoming unbelievably compilcated.
 *
 * The optional switches are as follows:
 *
 * -a The IP address to send captured packets to. Specifying this option implies
 *    sending captured packets as UDP packets to a destination instead of
 *    logging them to a file.
 * -b The port number to send the UDP packets to. Must specify the -a switch as
 *    well. Defaults to 8023.
 * -c The IP address to send captured Tx packets to if logging of Tx and Rx
 *    packets is specified. Must specify the -a switch as well. If not
 *    specified, the same IP address will be used for Tx and Rx packets.
 * -d The port number fo send captured Tx packets to if logging of Tx and Rx
 *    packets is specified. Must specify the -a switch as well. If not
 *    specified, defaults to the number specified in -b plus one. If -b not
 *    specified, defaults to 8024.
 * -i The name of the device to capture from. Default is dsrc0. This is ignored
 *    if bit 3 is set in the "m" parameter (see description below), since in
 *    this case we capture from all devices. Use cv2x0 to specify the C-V2X
 *    radio.
 * --path     Output path name. Defaults to FINISHED_LOG_DIR.
 * --filename Absolute file name. The PCAP log is written to this exact file
 *            name. The automatic adding of the the date, time, device name,
 *            file number, etc. to the output file name is disabled. Also,
 *            the automatic closing of old files, opening of new files, and
 *            automatic deletion of files is disabled.
 * -p File name prepend. The string associated with this parameter is prepended
 *    to the automatically generated output file name of the pcap log(s).
 * -s Maximum file size in KB. Zero (default) indicates no maximum file size. If
 *    a maximum file size is specified, a "_000" will be put in the file name
 *    before the ".pcap". Before the log file gets bigger than the maximum size,
 *    the file will be closed, and another file will be opened in its place,
 *    only with "_001" in place of the "_000". This will continue, increasing
 *    the number each time a new file is opened. Valid values are 0, and 16
 *    through 4194303.
 * -x Maximum total file size in MB. This is the maximum total size of all pcap
 *    log files created by this program. If continuing to log pcap data would
 *    cause this limit to be exceeded, pcap log files will automatically be
 *    deleted, starting with the oldest first. Once this much data has been logged program will exit.
 * -t Maximum number of minutes to keep the log file open before closing the
 *    current log file and opening another one. A value of 0 means to the max total minutes allowed in -T.
 *    If both -s and -t are specified, the existing log file will be closed and
 *    a new one opened when the maximum size is reached or the time limit is
 *    reached, whichever happens first. Valid values are 0 through 1440.
 * -T Maximim total number of minutes to log before exiting program.
 *    Valid values are 1 through 1440.
 * -m PCAP option bitmask. Each of the 4 low order bits in this mask enables
 *    or disables a certain option. The default value is 1 (log Tx only from a
 *    single device). The bits are
 *    assigned as follows:
 *
 *    Bit Value Meaning
 *    --- ----- -------
 *    0   1     Log Tx
 *    1   2     Log Rx
 *    2   4     Log Tx and Rx to separate files
 *    3   8     Capture from all DSRC devices, and ignore the "i" parameter
 *    4   16    Add the prism header
 *
 *    The behavior is as follows, based on the value of the "m" parameter
 *
 *    Value Meaning
 *    ----- -------
 *    1     Log Tx only from a single device. Log file name is of the form
 *          <filename>_<device>_tx.pcap
 *    2     Log Rx only from a single device. Log file name is of the form
 *          <filename>_<device>_rx.pcap
 *    3     Log both Tx and Rx from a single device to a common log file. Log
 *          file name is of the form <filename>_<device>_tx_rx.pcap
 *    7     Log both Tx and Rx from a single device to two separate log files.
 *          Two log file names will be  of the form <filename>_<device>_tx.pcap
 *          and <filename>_<device>_rx.pcap
 *    9     Log Tx only from all DSRC devices to a common log file. Log file
 *          name is of the form <filename>_dsrc0_dsrc1_tx.pcap
 *    10    Log Rx only from all DSRC devices to a common log file. Log file
 *          name is of the form <filename>_dsrc0_dsrc1_rx.pcap
 *    11    Log both Tx and Rx from all DSRC devices to a common log file. Log
 *          file name is of the form <filename>_dsrc0_dsrc1_tx_rx.pcap
 *    15    Log both Tx and Rx from all DSRC devices to two separate log files,
 *          one for Tx and one for Rx. Two log file names will be  of the form
 *          <filename>_dsrc0_dsrc1_tx.pcap and <filename>_dsrc0_dsrc1_rx.pcap
 *    16+x  Where x is 1, 2, 3, 7, 9, 10, 11, or 15. Same as above, except
 *          add a prism header to the captured packets.
 *
 *    Other values of "m" are invalid. File names will have a sequence number
 *    added before the ".pcap" if a maximum file size is specified.
 *
 *    Note: To log from both dsrc0 and dsrc1 at the same time, and have the data
 *    from dsrc0 and dsrc1 go into separate files, run two instances of
 *    capture_pcap at once, and set "m" to 1, 2, 3, or 7 in each case. Example:
 *
 *      capture_pcap -i dsrc0 -m 7 &
 *      capture_pcap -i dsrc1 -m 7 &
 *
 *    (Will produce a total of 4 log files, with Tx and Rx separated, and with
 *    dsrc0 and dsrc1 separated).
 *
 *    Running two or more instances of capture_pcap with the same interface
 *    name (-i), or with one of the instances having bit 3 set in the "m"
 *    parameter, will have unpredictable (and possibly catastrophic) results.
 */
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <dirent.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>
#include "ris.h"
#include "ris_struct.h"
#include "rs.h"
#include "ns_pcap.h"
#include "type2str.h"

#if defined(MY_UNIT_TEST)
#include "stubs.h"
#define MAIN capture_pcap_main
#else
#define MAIN main
#endif

/* Options
 * Uncomment these defines to enable the options
 */
//#define DEBUG_PRINTF            /* Print debugging messages */
//#define PRINTF_ON_DELETE        /* Print a message whenever a file is deleted
//                                 * to make room */
#define DEL_USING_GET_DIR_SIZE  /* Uses the get_directory_size() to determine
                                 * if enough space exists in the
                                 * output directory to write new PCAP log
                                 * files. If not, old PCAP files are deleted
                                 * until there is room. If this symbol is not
                                 * defined, then the total file size written by
                                 * this instance 0f capture_pcap is used
                                 * instead of the amount of space that
                                 * get_directory_size() reports from the output
                                 * directory. */
#define PARTIAL_BUFFER_WRITES   /* If a maximum file size is specfied, and
                                 * writing an entire buffer will make the file
                                 * size too large, write as may records as will
                                 * fit before going to a new file */

/*
 * Defines
 */
#define RUNNING        0
#define EXIT_REQUESTED 1
#define FINISHING_OFF  2

#define CLOSE_OUT_FILES \
    if (out_file_fd != -1) { \
        close(out_file_fd); \
        out_file_fd = -1; \
        chmod(out_file_name, 0644); \
    } \
    if (out_file_fd_tx != -1) { \
        close(out_file_fd_tx); \
        out_file_fd_tx = -1; \
        chmod(out_file_name_tx, 0644); \
    }

/* copy_uint32_t  allows assigning one uint32_t to another without causing an
 * alignment trap. */
#define copy_uint32_t(destination, source) \
    dest = (uint8_t *)&(destination); \
    src = (uint8_t *)&(source); \
    *dest++ = *src++; \
    *dest++ = *src++; \
    *dest++ = *src++; \
    *dest++ = *src++;

//TODO: test for another instance of self running
//TODO: disable partial files
#if defined(MY_UNIT_TEST)
#define RECORDING_LOG_DIR  "/tmp/ifclogs"
#define FINISHED_LOG_DIR   "/tmp/ifclogs_done"
#else
#define RECORDING_LOG_DIR "/tmp/ifclogs"         /* Capture here. Moving off /rwflash on purpose */
#define FINISHED_LOG_DIR  "/rwflash/I2V/ifclogs" /* Move to here when done. Do size & age maint here too. */
#endif
#define MAX_LOGGING_MINUTES_OVERALL 2880
#define STORAGE_CAPCITY_MAX (1024 * 1024 * 512) /* bytes: 512M seems like plenty */
/*
 * Typedefs
 */
/* PCAP file header */
typedef struct pcap_hdr_s {
    uint32_t magic_number;
    uint16_t version_major;
    uint16_t version_minor;
    uint32_t thiszone;
    uint32_t sigfigs;
    uint32_t snaplen;
    uint32_t network;
} pcap_hdr_t;

/* PCAP record header */
typedef struct pcaprec_hdr_s {
    uint32_t ts_sec;
    uint32_t ts_usec;
    uint32_t incl_len;
    uint32_t orig_len;
} pcaprec_hdr_t;

/*
 * Global variables
 */
static int exit_state = RUNNING;           /* The program execution state */

#define FILE_NAME_SIZE 1024
static char out_path[FILE_NAME_SIZE];      /* The output path */
static char absolute_out_file_name[FILE_NAME_SIZE];
                                           /* The absolute output file name */
static char out_file_name_prefix[FILE_NAME_SIZE];
                                           /* The output file name, minus the .pcap */
static char out_file_name_tx_prefix[FILE_NAME_SIZE];
                                           /* The Tx output file name, minus .pcap */
static char out_file_name[FILE_NAME_SIZE]; /* The output file name */
static char out_file_name_tx[FILE_NAME_SIZE];
                                           /* The Tx output file name */
static char out_file_name_prepend[FILE_NAME_SIZE];
                                           /* Prepended to the output file name(s) */
static int out_file_fd = -1;               /* The output file descriptor */
static int out_file_fd_tx = -1;            /* The Tx output file descriptor */
static bool_t network = FALSE;             /* Set to TRUE if output UDP packets to
                                            * network instead of to log file */
static bool_t use_absolute_out_file_name = FALSE;
                                           /* Set to TRUE if using an absolute output
                                            * file name. If so, disable automatically
                                            * adding the date, time, device name,
                                            * file number, etc. to the output file
                                            * name. Also, disable automatic closing of
                                            * old files, opening of new files, and
                                            * automatic deletion of files. */
static int sock = -1;                      /* Network output socket */
static int sock_tx = -1;                   /* Network output socket for Tx */
static char server_ip[16];                 /* Server IP address string */
static uint16_t server_port = 8023;         /* Server port number */
static struct sockaddr_in si;              /* Destination network address */
static char server_ip_tx[16];              /* Server IP address string for Tx */
static uint16_t server_port_tx = 8024;      /* Server port number  for Tx*/
static struct sockaddr_in si_tx;           /* Destination network address for Tx */
static bool_t c_switch_specified = FALSE;  /* Set to TRUE if -c switch specified */
static bool_t d_switch_specified = FALSE;  /* Set to TRUE if -d switch specified */

static uint8_t pcapBuffer[PCAP_LOG_BUF_SIZE];

static uint8_t pcap_mask = PCAP_TX;         /* The PCAP mask */

static uint32_t max_file_size = 0;          /* The maximum PCAP file size.
                                            * 0 ==> no limit */
static uint64_t total_bytes_written = 0x0;  /* Total bytes allowed before exiting program */

static uint32_t max_file_time = 0;          /* Maximum time to keep log file open,
                                            * 0 ==> no limit */
static uint32_t max_log_time = MAX_LOGGING_MINUTES_OVERALL; /* Total minutes allowed before exiting program */
static struct timeval file_open_time;      /* Time that the log file was opened */
static uint32_t current_file_size;
static uint32_t current_file_size_tx;
static uint16_t next_file_to_write = 0;
#ifndef DEL_USING_GET_DIR_SIZE
static uint16_t next_file_to_delete = 0;
#endif
//TODO: use CFGMGR to get from syslogdot.conf
static uint64_t max_total_file_size = STORAGE_CAPCITY_MAX;   /* The maximum total size of all files
                                            * written. 0 ==> no limit */
static uint64_t total_file_size = 0;       /* The total size of all files written */

static bool_t pcap_tx_rx_separate = FALSE; /* Set to TRUE if logging Tx and Rx to
                                            * separate files */

static int idx = 1; /* Used to parse argv */

static pcap_hdr_t pcap_hdr = {
    .magic_number = 0xa1b2c3d4,
    .version_major = 2,
    .version_minor = 4,
    .thiszone = 0,
    .sigfigs = 0,
    .snaplen = 65535,
    .network = 0x69
};                                         /* The PCAP header */

static char cmd[FILE_NAME_SIZE];           /* Buffer for a system command */

/*
 * Functions
 */
/**
 * init_statics - Reset statics across soft reset.
 *
 * Returns: None
 */
static void init_statics(void)
{
  exit_state = RUNNING;
  memset(out_path,0,sizeof(out_path));
  memset(absolute_out_file_name,0,sizeof(absolute_out_file_name));
  memset(out_file_name_prefix,0,sizeof(out_file_name_prefix));
  memset(out_file_name_tx_prefix,0,sizeof(out_file_name_tx_prefix));
  memset(out_file_name,0,sizeof(out_file_name));
  memset(out_file_name_tx,0,sizeof(out_file_name_tx));
  memset(out_file_name_prepend,0,sizeof(out_file_name_prepend));
  out_file_fd = -1;
  out_file_fd_tx = -1;
  network = FALSE;
  use_absolute_out_file_name = FALSE;
  sock = -1;
  sock_tx = -1;
  memset(server_ip,0,sizeof(server_ip));
  server_port = 8023;
  memset(&si,0,sizeof(si));
  memset(&server_ip_tx,0,sizeof(server_ip_tx));
  server_port_tx = 8024;
  memset(&si_tx,0,sizeof(si_tx));
  c_switch_specified = FALSE;
  d_switch_specified = FALSE;
  memset(pcapBuffer,0,sizeof(pcapBuffer));
  pcap_mask = PCAP_TX;
  max_file_size = 0;
  total_bytes_written = 0x0;
  max_file_time = 0; 
  max_log_time = MAX_LOGGING_MINUTES_OVERALL;
  memset(&file_open_time,0,sizeof(file_open_time));
  current_file_size = 0;
  current_file_size_tx = 0;
  next_file_to_write = 0;
#ifndef DEL_USING_GET_DIR_SIZE
  next_file_to_delete = 0;
#endif
  max_total_file_size = STORAGE_CAPCITY_MAX;
  total_file_size = 0;
  pcap_tx_rx_separate = FALSE;
  pcap_hdr.magic_number = 0xa1b2c3d4;
  pcap_hdr.version_major = 2;
  pcap_hdr.version_minor = 4;
  pcap_hdr.thiszone = 0;
  pcap_hdr.sigfigs = 0;
  pcap_hdr.snaplen = 65535;
  pcap_hdr.network = 0x69;
  memset(cmd,0,sizeof(cmd));
  idx = 1;
}
/**
 * move_logs - Move completed logs to user location.
 *
 * Returns: None
 */
static void move_logs(void)
{
#define MOVE_FINISHED_LOGS  "mv %s/*.pcap %s/."
  char cmd[512];
  memset(cmd,0x0,sizeof(cmd));
  snprintf(cmd,sizeof(cmd),MOVE_FINISHED_LOGS,RECORDING_LOG_DIR,FINISHED_LOG_DIR);
  if(0 != system(cmd)){
      printf("move logs: cmd[%s] failed: EXIT_REQUESTED.\n",cmd);
      exit_state = EXIT_REQUESTED;
  } else {
      printf("move logs: cmd[%s]\n",cmd);
  }
#undef MOVE_FINISHED_LOGS
}
static int32_t open_output_file(void)
{
#if defined(MY_UNIT_TEST)
  if ((out_file_fd = open(out_file_name, O_WRONLY | O_CREAT | O_TRUNC,777)) == -1) {
#else
  if ((out_file_fd = open(out_file_name, O_WRONLY | O_CREAT | O_TRUNC, 777)) == -1) {
#endif
      fprintf(stderr, "capture_pcap: Error creating output file %s: %s\n", out_file_name, strerror(errno));
      out_file_fd = -1;
      return -1;
  }
  return 0;
}
static int32_t open_output_file_tx(void)
{
#if defined(MY_UNIT_TEST)
  if ((out_file_fd_tx = open(out_file_name_tx, O_WRONLY | O_CREAT | O_TRUNC,777)) == -1) {
#else
  if ((out_file_fd_tx =  open(out_file_name_tx, O_WRONLY | O_CREAT | O_TRUNC, 777)) == -1) {
#endif
      fprintf(stderr, "capture_pcap: Error creating Tx output file %s: %s\n", out_file_name_tx, strerror(errno)); 
      close(out_file_fd); 
      out_file_fd = -1; 
      out_file_fd_tx = -1;
      chmod(out_file_name, 0644); 
      return -1;
  }
  return 0;
}

/**
 * exit_handler - Called when an exit signal is received
 *
 * @dummy: Not used, but required
 *
 * Returns: None
 *
 * This function is called when a control-C is hit, or the program is otherwise
 * killed. It sets the exit_state to EXIT_REQUESTED; the main loop then takes
 * care of the cleanup before the program exits.
 */
static void exit_handler(int dummy)
{
#ifdef DEBUG_PRINTF
    printf("DEBUG: In %s\n", __func__);
#endif
    dummy = dummy;
    exit_state = EXIT_REQUESTED;
}

/**
 * defaultFileName - Create a default file name
 *
 * @prepend: This string is prepended to the file name
 * @file_name: Where to store the file name created
 * @length: Length of the buffer pointed to by file_name
 * @radioNum: The radio number
 *
 * Returns: None
 * Get values from cmd line argc when called by logmg
 * Creates the default output file name of the form rsuID_ifaceName_direction_YYYYMMDD_HHMMSS
 */
static void defaultFileName(char *prepend, char *file_name, int length,
                            rsRadioType radioType, uint8_t radioNum)
{
    time_t     result  = time(NULL);
    struct tm *timeptr = localtime(&result);
    char_t     direction[16];

    switch(pcap_mask) {
        case 1 :
            strncpy(direction,"Out",sizeof(direction));
            break;
        case 2 :
            strncpy(direction,"In",sizeof(direction));
            break;
        case 3 :
            strncpy(direction,"Both",sizeof(direction));
            break;
        case 4 :
            strncpy(direction,"Seperate",sizeof(direction));
            break;
        default :
            strncpy(direction,"Unknown",sizeof(direction));
            break;
    }

    if (prepend[0] == '\0') {
        snprintf(file_name, length, "%s/%s_%s%u_%s_%04d%02d%02d_%02d%02d%02d",
                 out_path, "rsuID", (radioType == RT_CV2X) ? "cv2x" : "dsrc",radioNum,direction
                 ,timeptr->tm_year + 1900, timeptr->tm_mon + 1,
                 timeptr->tm_mday, timeptr->tm_hour, timeptr->tm_min,timeptr->tm_sec);
    }
    else {
        snprintf(file_name, length, "%s/%s_%s_%s%u_%s_%04d%02d%02d_%02d%02d%02d",
                 out_path, prepend, "rsuID", (radioType == RT_CV2X) ? "cv2x" : "dsrc",radioNum,direction
                 ,timeptr->tm_year + 1900, timeptr->tm_mon + 1,
                 timeptr->tm_mday, timeptr->tm_hour, timeptr->tm_min,timeptr->tm_sec);
    }

    if (pcap_mask & PCAP_ALL_DEVICES_SHARED) {
        strncat(file_name, "_dsrc1", length - 1);
    }

#ifdef DEBUG_PRINTF
    printf("DEBUG: %s: Created default file name %s\n", __func__, file_name);
#endif
}

/**
 * usage - Print usage message
 *
 * @argc: Passed in from main
 * @argv: Passed in from main
 *
 * Returns: None
 *
 * Prints the usage message.
 */
void usage(int argc, char *argv[])
{
    fprintf(stderr, "usage: capture_pcap [-i device][-m mask][-p prepend_to_file_name]\n" \
                    "                    [--path output_path][--filename absolute_output_file_name]\n" \
                    "                    [-s max_size_in_KB][-x max_total_size_in_MB]\n" \
                    "                    [-t max_log_file_open_time_in_minutes]\n" \
                    "                    [-T max_total_log_time_in_minutes]\n"\
                    "                    [-a server_ip_address][-b server_port_number]\n" \
                    "                    [-c server_ip_address_for_tx][-d server_port_number_for_tx]\n");
    fprintf(stderr, "        where mask = 1 for Tx, 2 for Rx, or 3 for both.\n");
    fprintf(stderr, "        Use mask = 7 to log Tx and Rx to separate files.\n");
    fprintf(stderr, "        Add 8 to the mask to ignore the -i parameter and\n");
    fprintf(stderr, "        log data from all devices together.\n");
    fprintf(stderr, "        Add 16 to the mask to add prism headers to the captured packets.\n");
    argc = argc;
    argv = argv;
}

/**
 * get_file_size - Get file size in bytes
 *
 * @file_name: The name of the file
 *
 * Returns: File size in bytes
 *
 */
#ifndef DEL_USING_GET_DIR_SIZE
uint32_t get_file_size(char *file_name)
{
    struct stat buf;

    stat(file_name, &buf);
    return (uint32_t)buf.st_size;
}
#endif

#ifdef PARTIAL_BUFFER_WRITES
/**
 * get_partial_buffer_size - Calculate the maximum number of bytes that can be
 *                           safely written
 *
 * @buffer: Pointer to the data to be logged
 * @total_len: The total number of bytes in the above buffer
 * @max_len: The maximum number of bytes that can be safely written. The number
 *           returned from this function will be less than or equal to this
 *
 * Returns: The number of bytes that can be safely written.
 *
 * This function traverses the buffer and adds the length of each record to the
 * size. When we get to the point where adding one more record will make the
 * size too large, we return. This way, writing out the number of bytes this
 * function specifies will not cause a record to be cut in half across file
 * boundaries.
 */
uint32_t get_partial_buffer_size(uint8_t const *buffer, uint32_t total_len,
                                uint32_t max_len)
{
    pcaprec_hdr_t *hdr;
    uint32_t        size = 0;
    uint32_t        record_len;

    if (total_len <= max_len) {
#ifdef DEBUG_PRINTF
        printf("DEBUG: %s: total_len (%u) < max_len (%u)\n",
               __func__, total_len, max_len);
#endif
        return total_len;
    }

    while(1) {
        hdr = (pcaprec_hdr_t *)buffer;
        /* The following statement counts on the fact that the same CPU will
         * have written the incl_len field as is running the capture_pcap
         * program. Therefore, nothing like le16_to_cpu() is needed.
         */
        record_len = hdr->incl_len + sizeof(pcaprec_hdr_t);

        if (size + record_len > max_len) {
            break;
        }

        buffer += record_len;
        size   += record_len;
#ifdef DEBUG_PRINTF
        printf("DEBUG: %s: size = %u\n", __func__, size);
#endif
    }

#ifdef DEBUG_PRINTF
    printf("DEBUG: %s: Returning %u\n", __func__, size);
#endif
    return size;
}
#endif

#ifndef DEL_USING_GET_DIR_SIZE
/**
 * delete_old_pcap_files - Delete old pcap files(s) to create more room
 *
 * Returns: None
 *
 * This function deletes the oldest PCAP file in order to create more room for
 * other PCAP files. If capturing Tx and Rx to separate files, both the Tx and
 * rx version of the file are deleted. next_file_to_delete is incremented.
 * total_file_size is decremented.
 */
void delete_old_pcap_files(void)
{
    uint32_t file_size;

    /* Delete the oldest PCAP file */
    snprintf(cmd, sizeof(cmd), "%s_%03d.pcap", out_file_name_prefix,
             next_file_to_delete);
    file_size = get_file_size(cmd);
    snprintf(cmd, sizeof(cmd), "rm %s_%03d.pcap", out_file_name_prefix,
             next_file_to_delete);
#ifdef PRINTF_ON_DELETE
    printf("%s: %s\n", __func__, cmd);
#endif
    system(cmd);
    total_file_size -= (uint64_t)file_size;
#ifdef DEBUG_PRINTF
    printf("DEBUG: %s: total_file_size = %lu\n", __func__, total_file_size);
#endif

    /* Also delete the Tx version of the PCAP file if appropriate */
    if (pcap_tx_rx_separate) {
        snprintf(cmd, sizeof(cmd), "%s_%03d.pcap",
                 out_file_name_tx_prefix, next_file_to_delete);
        file_size = get_file_size(cmd);
        snprintf(cmd, sizeof(cmd), "rm %s_%03d.pcap",
                 out_file_name_tx_prefix, next_file_to_delete);
#ifdef PRINTF_ON_DELETE
        printf("%s: %s\n", __func__, cmd);
#endif
        system(cmd);
        total_file_size -= (uint64_t)file_size;
#ifdef DEBUG_PRINTF
        printf("DEBUG: %s: total_file_size = %lu\n", __func__, total_file_size);
#endif
    }

    next_file_to_delete++;
}
#endif

/**
 * output_to_network - Write capture data out to the network
 *
 * @buffer: Pointer to the data to be logged
 * @len: Length of the data to be logged
 * @tx: Set to TRUE if this is Tx data to be sent separate from the Rx data.
 *      Set to FALSE otherwise.
 *
 * Returns: The length of the data written if successful (should always be
 *          equal to len). Returns -1 if error, and errno will contain the
 *          error code.
 *
 * This function writes PCAP data to the network.
 */
int output_to_network(uint8_t const *buffer, uint32_t len, uint8_t tx)
{
    uint8_t const  *packet_addr = buffer;
    uint32_t        packet_len;
    pcaprec_hdr_t *pcap;
    int            retval;
    int            total_len = 0;
    uint32_t        value;
    uint8_t        *dest, *src;

    /* Code is written so as not to cause alignment trap */
    while (len > 0) {
        pcap = (pcaprec_hdr_t *)packet_addr;
        copy_uint32_t(packet_len, pcap->incl_len);
        packet_len += sizeof(pcaprec_hdr_t);

        if (packet_len > len) {
            packet_len = len;
        }

        copy_uint32_t(value, pcap->ts_sec);
        value = htonl(value);
        copy_uint32_t(pcap->ts_sec, value);

        copy_uint32_t(value, pcap->ts_usec);
        value = htonl(value);
        copy_uint32_t(pcap->ts_usec, value);

        copy_uint32_t(value, pcap->incl_len);
        value = htonl(value);
        copy_uint32_t(pcap->incl_len, value);

        copy_uint32_t(value, pcap->orig_len);
        value = htonl(value);
        copy_uint32_t(pcap->orig_len, value);

        retval = (int)sendto(tx ? sock_tx : sock, packet_addr, packet_len, 0,
            tx ? (struct sockaddr *)&si_tx : (struct sockaddr *)&si,
            sizeof(struct sockaddr_in));

        if (retval < 0) {
            return retval;
        }

        total_len += retval;
        packet_addr += packet_len;
        len -= packet_len;
    }
    if(total_len){
        total_bytes_written += (uint64_t)total_len;
    }
    return total_len;
}

#ifdef DEL_USING_GET_DIR_SIZE
/**
 * get_directory_size
 *
 * Returns: The total size of all of the regular files in the specified
 * directory.
 *
 */
static uint64_t get_directory_size(char *dirname)
{
    DIR *dirp;
    struct dirent *direntp;
    struct stat fstat;
    char filename[FILE_NAME_SIZE];
    uint64_t total = 0LL;

    /* Open the output directory */
    dirp = opendir(dirname);

    if (dirp == NULL) {
        /* Possbily directory does not exist */
        fprintf(stderr, "capture_pcap: EXIT_REQUESTED: Can't open %s: %s\n", dirname, strerror(errno));
        exit_state = EXIT_REQUESTED;
    }
    else {
        /* Traverse each entry in the directory */
        while (1) {
            /* Get info on the next directory entry */
            direntp = readdir(dirp);

            if (direntp == NULL) {
                break;
            }

            /* Get information about this directory entry */
            sprintf(filename, "%s/%s", dirname, direntp->d_name);
            stat(filename, &fstat);

            /* If this is a regular file, add in its size */
            if (fstat.st_mode & S_IFREG) {
                total += (uint64_t)fstat.st_size;
            }
        }

        closedir(dirp);
    }

    return total;
}

/**
 * check_space_for_log
 *
 * Returns: None.
 *
 * This function checks whether "du" in the output directory exceeds
 * max_total_file_size. If it does  old files are deleted until
 * FINISHED_LOG_DIR "du" is less than max_total_file_size.
 */
void check_space_for_log(void)
{
    DIR           *dirP;
    struct dirent *file_name;
    struct stat    file_status;
    time_t         file_mod_time;
    char           curr_file_name[FILE_NAME_SIZE];
    char           last_file_name[FILE_NAME_SIZE];

    /* if the max_total_file_size is non-zero, check if the output directory
     * size  is less than max_total_file_size */
    dirP = NULL;
    if (max_total_file_size != 0) {
        while (1) {
            last_file_name[0] = '\0';

            /* If directory size is less than max_total_file_size, exit the
             * loop */
            /* logging in /tmp there will not be old files after reset. */
            if (get_directory_size(FINISHED_LOG_DIR) < max_total_file_size) {
                break;
            }

            /* Find out the oldest file to delete in the output directory */
            file_mod_time = time(NULL);

            if ((dirP = opendir(FINISHED_LOG_DIR)) != NULL ) {
                while((file_name = readdir(dirP))) {
                    /* Don't process non-"*.pcap" files */
                    if ((strlen(file_name->d_name) >= 5) &&
                        (memcmp(&(file_name->d_name[strlen(file_name->d_name) - 5]), ".pcap", 5) == 0)) {
                        snprintf(curr_file_name, sizeof(curr_file_name),
                                 "%s/%s", FINISHED_LOG_DIR, file_name->d_name);

                        /* Get the information about the file */
                        if (stat(curr_file_name, &file_status) == 0) {
                            /* If the current file modification time is less
                             * than previous one, copy the current one as last
                             * file */
                            if (file_status.st_mtime < file_mod_time) {
                                file_mod_time = file_status.st_mtime;
                                strncpy(last_file_name, curr_file_name,
                                        sizeof(last_file_name));
                            } // if
                        } // if
                    } // else
                } // while
                /* Close the directory before deleting any files from that
                 * directory */
                closedir(dirP);
                
                if (last_file_name[0] != '\0') {
                    if ((remove(last_file_name)) == -1) {
                        fprintf(stderr, "capture_pcap: Error removing PCAP log file %s: %s\n",
                            last_file_name, strerror(errno));
                    }
#ifdef PRINTF_ON_DELETE
                    else {
                        /* Remove the oldest file */
                        printf("Disk usage is greater than max storage size for PCAP Logs\n");
                        printf("Deleting.......%s\n", last_file_name);
                    }
#endif
                }
            } else {
                printf("opendir(%s) failed.\n",FINISHED_LOG_DIR);
            }
        }
    }
}
#endif

/**
 * write_out_file_data1 - Write capture data out to a PCAP capture file
 *
 * @fd: The file descriptor to write to
 * @buffer: Pointer to the data to be logged
 * @len: Length of the data to be logged
 *
 * Returns: The length of the data written if successful (should always be
 *          equal to len). Returns -1 if error, and errno will contain the
 *          error code.
 *
 * This function writes PCAP data to the proper PCAP file.
 * If we run out of room on the output device, and we have a maximum file size,
 * the oldest log files (both normal and Tx versions if appropriate) are deleted
 * in order to make room. Going to a new file if the write is too largs does
 * not happen here; the write_out_file_data() function below handles this.
 */
int write_out_file_data1(int fd, uint8_t const *buffer, uint32_t len)
{
    ssize_t bytes_left = (ssize_t)len;
    ssize_t bytes_written = 0;

#ifndef DEL_USING_GET_DIR_SIZE
    /* If necessary, delete old PCAP file to keep the total size below the
     * maximum total size */
    if (max_total_file_size != 0) {
        while (total_file_size + (uint64_t)bytes_left > max_total_file_size) {
#ifdef DEBUG_PRINTF
            printf("DEBUG: %s: total_file_size = ", __func__);
            printf("       %lu; bytes_left = %lu, max_total_file_size = ",
                   total_file_size, bytes_left);
            printf("       %lu\n", max_total_file_size);
#endif
            delete_old_pcap_files();
        }
    }
#endif

    while (((bytes_written = write(fd, buffer, bytes_left)) == -1) ||
           (bytes_written < bytes_left)) {
#ifdef DEBUG_PRINTF
        printf("DEBUG: %s: bytes_written = %ld; bytes_left = %lu\n",
               __func__, bytes_written, bytes_left);
#endif
        /* If no maximum file size, return "not enough space". In this case, we
         * always want to return -1 so the main loop will abort */
        if (max_file_size == 0) {
            errno = ENOSPC;
            return -1;
        }

        /* If a real error other than "no space", return with the error */
        if ((bytes_written == -1) && (errno != ENOSPC)) {
#ifdef DEBUG_PRINTF
            printf("DEBUG: capture_pcap: write_out_file_data1");
#endif
            return -1;
        }

#ifndef DEL_USING_GET_DIR_SIZE
        /* Delete previously written PCAP file(s) to make room */
        delete_old_pcap_files();
#endif

        /* If partial write, prepare to write out the rest */
        if (bytes_written != -1) {
            buffer     += bytes_written;
            bytes_left -= bytes_written;
#ifdef PRINTF_ON_DELETE
            printf("%s: bytes_left = %ld\n", __func__, bytes_left);
#endif
        }
    }
    if(len){
        total_bytes_written += (uint64_t)len;
    }
    return len;
}

/**
 * get_time_since_open - Gets the amount of time elapsed since the log file
 * were opened.
 *
 * @tv: Pointer to where the elapsed time is to be written.
 *
 * Returns: None.
 */
void get_time_since_open(struct timeval *tv)
{
    gettimeofday(tv, NULL);

    if (tv->tv_usec < file_open_time.tv_usec) {
        tv->tv_usec += 1000000;
        tv->tv_sec--;
    }

    tv->tv_sec -= file_open_time.tv_sec;
    tv->tv_usec -= file_open_time.tv_usec;
}

/**
 * write_out_file_data - Write capture data out to a PCAP capture file or to
 *                       the network as UDP packets.
 *
 * @buffer: Pointer to the data to be logged
 * @len: Length of the data to be logged
 * @tx: Set to TRUE if this is Tx data to be logged to a file separate from the
 *      Rx data. Set to FALSE otherwise.
 *
 * Returns: The length of the data written if successful (should always be
 *          equal to len). Returns -1 if error, and errno will contain the
 *          error code.
 *
 * This function writes PCAP data to the proper PCAP file. If we have a
 * maximum file size, and the write would make the PCAP file size too large,
 * both the normal PCAP file and the Tx version of the PCAP file (if we are
 * logging Tx and Rx data separately) are closed, and new files are opened.
 * If we run out of room on the output device, and we have a maximum file size,
 * the oldest log files (both normal and Tx versions if appropriate) are deleted
 * in order to make room.
 *
 * If network == 1, then output to the network instead of to the log file.
 */
int write_out_file_data(uint8_t const *buffer, uint32_t len, uint8_t tx)
{
    int            *fd_ptr = NULL;
    uint32_t        file_size = 0;
#ifdef PARTIAL_BUFFER_WRITES
    uint32_t        orig_len = 0;
#endif
    bool_t          max_file_size_reached = FALSE;
    bool_t          max_file_time_expired = FALSE;
    struct timeval  tv;

    /* Test if output to network */
    if (network) {
        int size = (int)output_to_network(buffer, len, tx);

        if (size == -1) {
            printf("capture_pcap: Error writing packet to socket");
            exit_state = EXIT_REQUESTED;
        }

        return size;
    }

    fd_ptr    = tx ? &out_file_fd_tx      : &out_file_fd;
    file_size = tx ? current_file_size_tx : current_file_size;
#ifdef PARTIAL_BUFFER_WRITES
    orig_len  = len;
#endif

    if (max_file_size != 0) {
        /* See if maximum file size reached */
        max_file_size_reached = (file_size + (uint64_t)len > max_file_size) ? TRUE :
                                                                    FALSE;

#ifdef DEBUG_PRINTF
        if (max_file_size_reached == TRUE) {
            printf("DEBUG: %s: file_size = %u; len = %u; max_file_size = %u\n",
                   __func__, file_size, len, max_file_size);
        }
#endif

    }

    if (max_file_time != 0) {
        /* Get elapsed time since file opened */
        get_time_since_open(&tv);

        /* See if maximum file time expired */
        max_file_time_expired = ((tv.tv_sec / 60) >= max_file_time) ? TRUE :
                                                                      FALSE;

#ifdef DEBUG_PRINTF
        if (max_file_time_expired == TRUE) {
            printf("DEBUG: %s: minutes elapsed = %lu; max_file_time = %u\n",
                   __func__, (tv.tv_sec / 60), max_file_time);
        }
#endif
    }

    /* If the write will make file too large, or file time has expired,
     * go to the next file */
    if ((max_file_size_reached == TRUE) || (max_file_time_expired == TRUE)) {
        char  suffix[12];
#ifdef PARTIAL_BUFFER_WRITES
        uint32_t   partial_len;

        /* If the maximum file time has not expired (implying the maximum file
         * size was reached), Find out how many bytes we can safely write,
         * then write them out */
        if (max_file_time_expired == FALSE) {
            partial_len = get_partial_buffer_size(buffer, len,
                                                  max_file_size - file_size);

#ifdef DEBUG_PRINTF
            printf("DEBUG: %s: partial_len = %u\n", __func__, partial_len);
#endif
            if ((partial_len > 0) &&
                (write_out_file_data1(*fd_ptr, buffer, partial_len) == -1)) {
#ifdef DEBUG_PRINTF
                printf("DEBUG: capture_pcap: write_out_file_data: write_out_file_data1 failed(1)");
#endif
                return -1;
            }

            /* Adjust to write out the rest of the buffer */
            buffer += partial_len;
            len    -= partial_len;
#ifdef DEBUG_PRINTF
            printf("DEBUG: %s: Remaining length = %u\n", __func__, len);
#endif
        }
#endif /* PARTIAL_BUFFER_WRITES */

        /* Close the current PCAP file */
#ifdef DEBUG_PRINTF
        printf("DEBUG: %s: Closing %s\n", __func__, out_file_name);
#endif
        CLOSE_OUT_FILES;
        if(!use_absolute_out_file_name) {
            move_logs();
        }

#ifdef DEL_USING_GET_DIR_SIZE
        /* Check if we have enough space before a another file */
        check_space_for_log();
#endif

        /* Generate new file name */
        snprintf(suffix, sizeof(suffix), "_%03d.pcap", next_file_to_write++);
        strncpy(out_file_name, out_file_name_prefix, sizeof(out_file_name));
        strncat(out_file_name, suffix, sizeof(out_file_name) - 1);

        if (max_file_time_expired == TRUE) {
            /* If maximum file time expires, keep increment the file open time
             * by the proper number of minutes until the number of minutes
             * since file open is less than the maximum file time. This is
             * necessary because the file time is only checked when we receive
             * a packet, and a long time may elapse without us receiving a
             * packet. */
            do {
                file_open_time.tv_sec += max_file_time * 60;
#ifdef DEBUG_PRINTF
                printf("DEBUG: %s: File open time incremented to %lu,%lu\n", __func__,
                       file_open_time.tv_sec, file_open_time.tv_usec);
#endif
                get_time_since_open(&tv);
            }
            while ((tv.tv_sec / 60) >= max_file_time);
        }
        else if ((max_file_size_reached == TRUE) && (max_file_time != 0)) {
            /* If the maximum file time has not yet expired, but the maximum
             * file size has been reached, and we have a maximum file time,
             * reset the file open time to now. */
            gettimeofday(&file_open_time, NULL);
#ifdef DEBUG_PRINTF
            printf("DEBUG: %s: File open time reset to %lu,%lu\n", __func__,
                   file_open_time.tv_sec, file_open_time.tv_usec);
#endif
        }

        /* Open new file for writing */
#ifdef DEBUG_PRINTF
        printf("DEBUG: %s: Opening %s\n", __func__, out_file_name);
#endif
        if(0 != open_output_file()) {
            return -1;
        }

        /* Write the PCAP header to the file */
        if((out_file_fd<0) || (write_out_file_data1(out_file_fd, (uint8_t *)&pcap_hdr,
            sizeof(pcap_hdr_t)) == -1)) {
            fprintf(stderr, "capture_pcap: Error writing PCAP header to output file %s: %s\n",
                    out_file_name, strerror(errno));
            CLOSE_OUT_FILES;
            return -1;
        }

        current_file_size = sizeof(pcap_hdr_t);
        total_file_size += (uint64_t)sizeof(pcap_hdr_t);

        /* Do the same for the Tx PCAP file if necessary */
        if (pcap_tx_rx_separate) {
#ifdef DEBUG_PRINTF
            printf("DEBUG: %s: Closing %s\n", __func__, out_file_name_tx);
#endif
            strncpy(out_file_name_tx, out_file_name_tx_prefix, sizeof(out_file_name_tx));
            strncat(out_file_name_tx, suffix, sizeof(out_file_name_tx) - 1);

#ifdef DEBUG_PRINTF
            printf("DEBUG: %s: Opening %s\n", __func__, out_file_name_tx);
#endif
            if(0 != open_output_file_tx()) {
                return -1;
            }

            if (write_out_file_data1(out_file_fd_tx, (uint8_t *)&pcap_hdr,
                sizeof(pcap_hdr_t)) == -1) {
                fprintf(stderr, "capture_pcap: Error writing PCAP header to output Tx file %s: %s\n",
                        out_file_name_tx, strerror(errno));
                CLOSE_OUT_FILES;
                return -1;
            }

            current_file_size_tx = sizeof(pcap_hdr_t);
            total_file_size += (uint64_t)sizeof(pcap_hdr_t);
        }
    }

    /* Write out the data; delete old log files if necessary */
    if (write_out_file_data1(*fd_ptr, buffer, len) == -1) {
#ifdef DEBUG_PRINTF
        printf("DEBUG: write_out_file_data: write_out_file_data1 failed(2)");
#endif
        return -1;
    }

    /* Count up the size of this write */
    if (tx) {
        current_file_size_tx += len;
    }
    else {
        current_file_size += len;
    }

    total_file_size += (uint64_t)len;
#ifdef PARTIAL_BUFFER_WRITES
    return orig_len;
#else
    return len;
#endif
}

/**
 * Parses one command line argument. To "param" is written the letter after
 * the single minus sign ("-") in a parameter name. The value returned is
 * a pointer to the string corresponding to the parameters value.
 *
 * If a double minus sign ("--") is followed by "path" or "filename", the
 * values returned will be as above, except that "param" will be set to 0 or 1
 * respectively.
 *
 * If "param" is set to -1, then an error has occurred.
 *
 * If "param" is not set to -1, and NULL is returned, we have reached the end
 * of the command line.
 *
 * The first time parse_arg() is called, it returns results for the first
 * parameter. The second time parse_arg() is called, it returns results for the
 * second parameter, etc. Contunue calling parse_arg() until it returns NULL,
 * then check "param" to see whether or not an error has occurred.
 *
 * An ampersand ('&') indicates end-of-line.
 *
 * @argc: Argument count
 * @argv: Argument length
 * @param: Assuming the function returns non-NULL, a character is written here
 *         for parameters with a minus sign ("-") followed by a character; 0
 *         for "--path" and 1 for "--filename".
 *
 * Returns: A pointer to the string value of the parameter. NULL on error or
 *          end of line.
 *
 */
static char *parse_arg(int argc, char **argv, char *param)
{
    if ((idx == argc) || (argv[idx][0] == '&')) {
        /* End of line */
        *param = 0;
        return NULL;
    }

    if (idx == argc - 1) {
        fprintf(stderr, "capture_pcap: Parameter (%s) idx=%d, without value\n", argv[argc - 1],idx);
    }
    else {
        /* Parameter with value */
        if (argv[idx][0] == '-') {
            if (argv[idx][1] != '-') {
                /* Single minus sign ("-") */
                *param = argv[idx][1];
                idx += 2;
                return argv[idx - 1];
            }

            /* Double minus sign ("--") */
            if (strncmp(&argv[idx][2], "path", 4) == 0) {
                *param = 0;
                idx += 2;
                return argv[idx - 1];
            }

            if (strncmp(&argv[idx][2], "filename", 8) == 0) {
                *param = 1;
                idx += 2;
                return argv[idx - 1];
            }

            fprintf(stderr, "capture_pcap: Invalid parameter (%s)\n", argv[idx]);
            /* Fall thru to error case */
        }
    }

    *param = (char)-1;
    return NULL;
}

/**
 * pcapReadData - Read the contents of the PCAP buffer.
 *
 * @pcapBuffer: Pointer to the buffer to hold the data that is read.
 * @maxSize: Size of pcapBuffer.
 * @radioNum: The radio number.
 * @tx: Set to TRUE to read the Tx PCAP buffer (used only if Tx and Rx PCAP
 *      are separate). Set to FALSE otherwise.
 *
 * Returns: Length of the data read.
 */
int32_t pcapReadData(uint8_t *pcapBuffer, int maxSize, rsRadioType radioType,
                      uint8_t radioNum, bool_t tx)
{
    rsPcapReadType   pcapRead;
    int32_t          size = 0;
    rsResultCodeType risRet = 0;;

    /* Read the PCAP data */
    pcapRead.radioType = radioType;
    pcapRead.radioNum  = radioNum;
    pcapRead.tx        = tx;
    risRet = wsuRisPcapRead(&pcapRead, pcapBuffer, maxSize, &size);

    if (risRet != RS_SUCCESS) {
        fprintf(stderr, "capture_pcap: wsuRisPcapRead() failed: %s\n",
                rsResultCodeType2Str(risRet));
        size = -1;
    }

    return size;
}

/**
 * main - Main routine for the capture_pcap program
 *
 * @argc: Argument count
 * @argv: Argument length
 *
 * Returns: -1 if abnormal termination. 0 otherwise.
 *
 * This function is the main loop for the capture_pcap program. If first
 * processes the command line, and does some validation of the arguments.
 * It then generates the output file names. It then opens the files and writes
 * a header to the files. It then sets itself to capture a control-C or a kill
 * command. The last part of the initialization is to enable PCAP logging on
 * the devices to have data captured.
 *
 * The main loop of the program continuously reads PCAP data from the devices
 * and writes the data to the log files. If no data is read, there is a short
 * delay to allow the devices to accumulate some data.
 *
 * When control-C is hit, or the process is otherwise killed, then PCAP logging
 * is disabled for the devices, and a 1-second delay allows the devices to
 * finish up whatever logging is in progress. Then the last of the data is
 * written to the log files, and they are closed.
 */
int MAIN(int argc, char *argv[])
{
    char              *value = NULL;
    char               param;
    int                sleep_time = 20000;
    int                broadcast = 1;
    char               suffix[12];
    FILE              *logFileP = NULL;
    rsRadioType        radioType = RT_DSRC;
    uint8_t            radioNum = 0;
    rsPcapEnableType   pcapEnable;
    rsPcapDisableType  pcapDisable;
    int32_t            len = 0;
    int32_t            len_tx = 0;
    int                ret = 0;
    rsResultCodeType   risRet = 0;
    cfgType            getCfg;
    bool_t             cv2x_enable = FALSE;
    bool_t             dsrc_enable = FALSE;
    struct timeval     now_time;
    struct timeval     start_time;

    /* Init local and static vars. Set default output path */
    init_statics();
    memset(suffix,'\0',sizeof(suffix));
    memset(&pcapEnable,0,sizeof(pcapEnable));
    memset(&pcapDisable,0,sizeof(pcapDisable));
    memset(&getCfg,0,sizeof(getCfg));
    memset(&now_time,0,sizeof(now_time));
    memset(&start_time,0,sizeof(start_time));
    strncpy(out_path, RECORDING_LOG_DIR, sizeof(out_path));

    /* Parse the command line */
    while (1) {
        value = parse_arg(argc, argv, &param);

        if (value == NULL) {
            if (param == (char)-1) {
                /* Error */
                usage(argc, argv);
                return -1;
            }

            /* End of line */
#ifdef DEBUG_PRINTF
            printf("DEBUG: %s: End of line\n", __func__);
#endif
            break;
        }

        switch(param) {
        case 0: // "path"
            strcpy(out_path, value);

            if (out_path[strlen(out_path) - 1] == '/') {
                out_path[strlen(out_path) - 1] = '\0';
            }

#ifdef DEBUG_PRINTF
            printf("DEBUG: %s: --path %s; out_path = \"%s\"\n", __func__, value, out_path);
#endif
            break;

        case 1: // "filename"
            strncpy(absolute_out_file_name, value,
                    sizeof(absolute_out_file_name));
            use_absolute_out_file_name = TRUE;
#ifdef DEBUG_PRINTF
            printf("DEBUG: %s: --filename %s; absolute_out_file_name = \"%s\"\n", __func__, value, absolute_out_file_name);
#endif
            break;

        case 'a': /* output to network IP address */
            network = TRUE;
            strncpy(server_ip, value, sizeof(server_ip));
#ifdef DEBUG_PRINTF
            printf("DEBUG: %s: -a %s; server_ip = \"%s\"\n", __func__, value, server_ip);
#endif

            if (!c_switch_specified) {
                strncpy(server_ip_tx, server_ip, sizeof(server_ip_tx));
#ifdef DEBUG_PRINTF
            printf("DEBUG: %s: -a %s; server_ip_tx = \"%s\"\n", __func__, value, server_ip_tx);
#endif
            }

            break;

        case 'b': /* output to network port */
            server_port = atoi(value);
#ifdef DEBUG_PRINTF
            printf("DEBUG: %s: -b %s; server_port = %u\n", __func__, value, server_port);
#endif

            if (!d_switch_specified) {
                server_port_tx = server_port + 1;
#ifdef DEBUG_PRINTF
                printf("DEBUG: %s: -b %s; server_port_tx = %u\n", __func__, value, server_port_tx);
#endif
            }

            break;

        case 'c': /* output IP address for Tx file */
            c_switch_specified = TRUE;
            strncpy(server_ip_tx, value, sizeof(server_ip_tx));
#ifdef DEBUG_PRINTF
            printf("DEBUG: %s: -c %s; server_ip_tx = \"%s\"\n", __func__, value, server_ip_tx);
#endif
            break;

        case 'd': /* output port for Tx file */
            d_switch_specified = TRUE;
            server_port_tx = atoi(value);
#ifdef DEBUG_PRINTF
            printf("DEBUG: %s: -d %s; server_port_tx = %u\n", __func__, value, server_port_tx);
#endif
            break;

        case 'i': /* radio interface name */
            /* Ignore this if previously entered -m with
             * PCAP_ALL_DEVICES_SHARED set */
            if ((pcap_mask & PCAP_ALL_DEVICES_SHARED) == 0) {
                if ((value[0] == 'C') || (value[0] == 'c')) {
                    radioType = RT_CV2X;
                    radioNum  = 0;
                }
                else {
                    /* DSRC radio: Use the last character of the device name as
                     * the radio number */
                    radioType = RT_DSRC;
                    radioNum  = value[strlen(value) - 1] - '0';

                    if ((radioNum != 0) && (radioNum != 1)) {
                        fprintf(stderr, "capture_pcap: Invalid device name: %s\n", value);
                        return -1;
                    }
                }

#ifdef DEBUG_PRINTF
                printf("DEBUG: %s: -i %s; radioNum = %u\n", __func__, value, radioNum);
#endif
            }
            break;

        case 'm': /* PCAP mask */
            pcap_mask = atoi(value) &
                (PCAP_TX | PCAP_RX | PCAP_TX_RX_SEPARATE |
                 PCAP_ALL_DEVICES_SHARED | PCAP_PRISM_HDR);

            /* Invalid mask if capturing neither Tx nor Rx */
            if ((pcap_mask & (PCAP_TX | PCAP_RX)) == 0) {
                printf("capture_pcap: Invalid mask if capturing neither Tx nor Rx\n");
                usage(argc, argv);
                return -1;
            }

            /* Invalid mask is capturing Tx and Rx to separate file, and not
             * capturing both Tx and Rx */
            if (((pcap_mask & PCAP_TX_RX_SEPARATE) != 0) &&
                ((pcap_mask & (PCAP_TX | PCAP_RX)) != (PCAP_TX | PCAP_RX))) {
                printf("capture_pcap:(-m %d) Invalid mask, Tx and Rx to separate file, and not capturing both Tx and Rx\n",pcap_mask);
                usage(argc, argv);
                return -1;
            }

            /* Only sleep for 10000ms if no PCAP data received if capturing
             * Rx data */
            if (pcap_mask & PCAP_RX) {
                sleep_time = 10000;
            }

            /* If prism header, indicate this in the output file */
            if (pcap_mask & PCAP_PRISM_HDR) {
                pcap_hdr.network = 0x77;
            }

            /* If PCAP_ALL_DEVICES_SHARED, set radio type to RT_DSRC and radio
             * number to 0 */
            if (pcap_mask & PCAP_ALL_DEVICES_SHARED) {
                radioType = RT_DSRC;
                radioNum  = 0;
            }

#ifdef DEBUG_PRINTF
            printf("DEBUG: %s: -m %s; pcap_mask = %u\n", __func__, value, pcap_mask);
#endif
            break;

        case 'p': /* prepend to output file name */
            strncpy(out_file_name_prepend, value, sizeof(out_file_name_prepend));
#ifdef DEBUG_PRINTF
            printf("DEBUG: %s: -p %s; out_file_name_prefix = \"%s\"\n", __func__, value, out_file_name_prefix);
#endif
            break;

        case 's': /* max log file size; 0 = no limit */
            max_file_size = atoi(value);

            if (max_file_size != 0) {
                max_file_size *= 1024; /* to bytes */
                if ((max_file_size < (16 * 1024)) || (max_file_size > STORAGE_CAPCITY_MAX)) {
                    printf("-s valid range is 16 to %d Megs.\n",STORAGE_CAPCITY_MAX);
                    usage(argc, argv);
                    return -1;
                }
            }

#ifdef DEBUG_PRINTF
            printf("DEBUG: %s: -s %s; max_file_size = %u\n", __func__, value, max_file_size);
#endif
            break;

        case 't': /* max time in minutes to keep log file open; 0 = no limit */
            max_file_time = atoi(value);

            if (max_file_time > MAX_LOGGING_MINUTES_OVERALL) {
                max_file_time = 0;
            }

#ifdef DEBUG_PRINTF
            printf("DEBUG: %s: -t %s; max_file_time = %u\n", __func__, value, max_file_time);
#endif
            break;
        case 'T': /* max overall time in minutes to keep logging; 0 = no limit */
            max_log_time = atoi(value);

            if (max_total_file_size != 0) {
                if (max_log_time > MAX_LOGGING_MINUTES_OVERALL) {
                    printf("-T valid range is 0 to %d minutes.\n",MAX_LOGGING_MINUTES_OVERALL);
                    usage(argc, argv);
                    return -1;
                }
            }
#ifdef DEBUG_PRINTF
            printf("DEBUG: %s: -T %s; max_log_time = %u\n", __func__, value, max_log_time);
#endif
            break;

        case 'x': /* max total file size: in MEGS */
            max_total_file_size = (uint64_t)((uint64_t)atoi(value) * (uint64_t)1048576); /* to bytes */

            if (max_total_file_size != 0) {
                /* The max_total_file_size is limited to 512M */
                if (max_total_file_size > (uint64_t)STORAGE_CAPCITY_MAX) {
                    printf("-x valid range is 0 to %d Megs.\n",STORAGE_CAPCITY_MAX);
                    usage(argc, argv);
                    return -1;
                }
            }

#ifdef DEBUG_PRINTF
            printf("DEBUG: %s: -x %s; max_total_file_size = %lu\n", __func__, value, max_total_file_size);
#endif
            break;

        default:
            usage(argc, argv);
            return -1;
        }
    }

    /* Get the value of the nsconfig variable CV2X_ENABLE */
    risRet = wsuRisGetCfgReq(CFG_ID_CV2X_ENABLE, &getCfg);

    if (risRet == RS_SUCCESS) {
        cv2x_enable = getCfg.u.CV2XEnable;
    }

    /* Get the value of the nsconfig variable DSRC_ENABLE */
    risRet = wsuRisGetCfgReq(CFG_ID_DSRC_ENABLE, &getCfg);

    if (risRet == RS_SUCCESS) {
        dsrc_enable = getCfg.u.DSRCEnable;
    }

    /* Error if no radios enabled */
    if (!cv2x_enable && !dsrc_enable) {
        printf("capture_pcap: No radios enabled! Enable either the C-V2X or the DSRC radio.\n");
        return -1;
    }

    /* If one radio is enabled, but not the other, override the radioType value */
    if (cv2x_enable && !dsrc_enable) {
        radioType = RT_CV2X;
    }
    else if (!cv2x_enable && dsrc_enable) {
        radioType = RT_DSRC;
    }

    /* Special processing if capturing from C-V2X. */
    if (radioType == RT_CV2X) {
        pcap_mask &= ~PCAP_PRISM_HDR; /* No PRISM header */
        pcap_hdr.network = 0x01;      /* Pretend we captured from Ethernet */
    }
    /* Create the output directory if it doesn't exist */
    if ((logFileP = fopen(FINISHED_LOG_DIR, "r")) == NULL) {
#ifdef DEBUG_PRINTF
        printf("DEBUG: %s: Creating directory %s\n", __func__, FINISHED_LOG_DIR);
#endif
        snprintf(cmd, sizeof(cmd), "mkdir -p %s", FINISHED_LOG_DIR);
        system(cmd);
        system("sleep 3");
    } else {
        fclose(logFileP);
    }
    /* If using absolute output file name, disable any conflicting options. */
    if (!network) {
        if (use_absolute_out_file_name) {
            snprintf(out_file_name, sizeof(out_file_name), "%s/%s",
                     out_path, absolute_out_file_name);
            pcap_mask &= ~PCAP_TX_RX_SEPARATE;
            max_file_size = 0;
            max_file_time = 0;
            max_total_file_size = 0;
        }
        else {
            defaultFileName(out_file_name_prepend, out_file_name_prefix,
                            sizeof(out_file_name_prefix), radioType, radioNum);
#ifdef DEL_USING_GET_DIR_SIZE
            check_space_for_log();
#endif
        }
    }
    /* If PCAP_TX_RX_SEPARATE is set in pcap_mask, use separate files for Tx
     * and Rx captures */
    if (pcap_mask & PCAP_TX_RX_SEPARATE) {
        pcap_tx_rx_separate = TRUE;
        defaultFileName(out_file_name_prepend, out_file_name_tx_prefix,
                            sizeof(out_file_name_prefix), radioType, radioNum);
    }
    /* Open the output PCAP file(s) or network socket(s)*/
    if (network) { /* Output to socket(s) */
        if ((sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
            printf("capture_pcap: Error opening output socket");
            goto err_exit;
        }

        if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &broadcast,
                       sizeof(broadcast)) == -1) {
            printf("setsockopt (SO_BROADCAST)");
            goto err_exit;
        }

        memset((char *)&si, 0, sizeof(struct sockaddr_in));
        si.sin_family = AF_INET;
        si.sin_port = htons(server_port);

        if (inet_aton(server_ip, &si.sin_addr) == 0) {
            fprintf(stderr, "capture_pcap: inet_aton() failed\n");
            goto err_exit;
        }
#ifdef DEBUG_PRINTF
        printf("DEBUG: %s: Network socket opened; sock = %d\n", __func__, sock);
#endif

        if (pcap_tx_rx_separate) {
            if ((sock_tx = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
                printf("capture_pcap: Error opening output Tx socket");
                goto err_exit;
            }

            if (setsockopt(sock_tx, SOL_SOCKET, SO_BROADCAST, &broadcast,
                           sizeof broadcast) == -1) {
                printf("setsockopt (SO_BROADCAST) Tx");
                goto err_exit;
            }

            memset((char *)&si_tx, 0, sizeof(struct sockaddr_in));
            si_tx.sin_family = AF_INET;
            si_tx.sin_port = htons(server_port_tx);

            if (inet_aton(server_ip_tx, &si_tx.sin_addr) == 0) {
                fprintf(stderr, "capture_pcap: inet_aton() failed Tx\n");
                goto err_exit;
            }
#ifdef DEBUG_PRINTF
            printf("DEBUG: %s: Network Tx socket opened; sock_tx = %d\n", __func__, sock_tx);
#endif
        }
    }
    else { /* Output to PCAP file(s) */
        if (!use_absolute_out_file_name) {
            /* Add suffix to file name */
            if ((max_file_size != 0) || (max_file_time != 0)) {
                snprintf(suffix, sizeof(suffix), "_%03d.pcap", next_file_to_write++);
            }
            else {
                strncpy(suffix, ".pcap", sizeof(suffix));
            }

            strncpy(out_file_name, out_file_name_prefix, sizeof(out_file_name));
            strncat(out_file_name, suffix, sizeof(out_file_name) - 1);

            if (pcap_tx_rx_separate) {
                strncpy(out_file_name_tx, out_file_name_tx_prefix, sizeof(out_file_name_tx));
                strncat(out_file_name_tx, suffix, sizeof(out_file_name_tx) - 1);
            }
        }

        /* Create the output directory if it doesn't exist */
        if ((logFileP = fopen(out_path, "r")) == NULL) {
#ifdef DEBUG_PRINTF
            printf("DEBUG: %s: Creating directory %s\n", __func__, out_path);
#endif
            snprintf(cmd, sizeof(cmd), "mkdir -p %s", out_path);
            system(cmd);
            system("sleep 3");
        }
        else {
            fclose(logFileP);
        }

#ifdef DEBUG_PRINTF
        printf("DEBUG: %s: Opening %s\n", __func__, out_file_name);
#endif
        if(0 != open_output_file()) {
            return -1;
        }

        /* Get the file open time */
        if (max_file_time != 0) {
            gettimeofday(&file_open_time, NULL);
#ifdef DEBUG_PRINTF
            printf("DEBUG: %s: File open time set to %lu,%lu\n", __func__,
                   file_open_time.tv_sec, file_open_time.tv_usec);
#endif
        }

        if (pcap_tx_rx_separate) {
#ifdef DEBUG_PRINTF
            printf("DEBUG: %s: Opening %s\n", __func__, out_file_name_tx);
#endif
            if(0 != open_output_file_tx()) {
                return -1;
            }
        }

        /* Write the PCAP header to the file */
        if((out_file_fd<0) || (write_out_file_data1(out_file_fd, (uint8_t *)&pcap_hdr,
            sizeof(pcap_hdr_t)) == -1)) {
            fprintf(stderr, "capture_pcap: Error writing PCAP header to output file %s: %s\n", out_file_name, strerror(errno));
            goto err_exit;
        }
#ifdef DEBUG_PRINTF
        printf("DEBUG: %s: PCAP header written\n", __func__);
#endif

        current_file_size = sizeof(pcap_hdr_t);
        total_file_size += (uint64_t)sizeof(pcap_hdr_t);

        if (pcap_tx_rx_separate) {
            if (write_out_file_data1(out_file_fd_tx, (uint8_t *)&pcap_hdr,
                sizeof(pcap_hdr_t)) == -1) {
                fprintf(stderr, "capture_pcap: Error writing PCAP header to output Tx file %s: %s\n", out_file_name, strerror(errno));
                goto err_exit;
            }
#ifdef DEBUG_PRINTF
            printf("DEBUG: %s: PCAP header written to Tx file\n", __func__);
#endif

            current_file_size_tx = sizeof(pcap_hdr_t);
            total_file_size += (uint64_t)sizeof(pcap_hdr_t);
        }
    } /* Network socket(s) or PCAP file(s) */

    /* Enable PCAP for this device. If PCAP_ALL_DEVICES_SHARED is set in
     * pcap_mask, all devices will be enabled */
    pcapEnable.radioType = radioType;
    pcapEnable.radioNum  = radioNum;
    pcapEnable.mask      = pcap_mask;
    risRet = wsuRisPcapEnable(&pcapEnable);

    if (risRet != RS_SUCCESS) {
        fprintf(stderr, "capture_pcap: wsuRisPcapEnable() failed: %s\n",
                rsResultCodeType2Str(risRet));
        goto err_exit;
    }
#ifdef DEBUG_PRINTF
    printf("DEBUG: %s: PCAP enabled\n", __func__);
#endif

    /* Capture program termination (control-C) */
    signal(SIGINT,  exit_handler);
    signal(SIGTERM, exit_handler);
    signal(SIGKILL, exit_handler);

    gettimeofday(&start_time, NULL);
    /* Main loop: Get the data from the driver and write it to the file.
     * When the program is terminated, stop the capture and write out the
     * remaining data. */
    do {
        if(max_log_time) {
            gettimeofday(&now_time, NULL);
            if(start_time.tv_sec + (int64_t)(max_log_time * 60) <= now_time.tv_sec) {
                exit_state = EXIT_REQUESTED;
                printf("Max log time expired. Exit called.\n");
            }
#ifdef DEBUG_PRINTF
            if(0 == (now_time.tv_sec % 10)) {
                printf("max_log_time(%u) total_log_time(%lu) \n",
                max_log_time * 60, now_time.tv_sec - start_time.tv_sec);
            }
#endif
        }
        if((0 < max_total_file_size) && (max_total_file_size <= total_bytes_written)){
            exit_state = EXIT_REQUESTED;
#ifdef DEBUG_PRINTF
            printf("Max log byte count reached. Exit called. max(0x%lx) total(0x%lx)\n",max_total_file_size,total_bytes_written);
#endif
        }
        if (exit_state == EXIT_REQUESTED) {
            /* Exit of program requested. Disable PCAP for this device. If
             * PCAP_ALL_DEVICES_SHARED is set in pcap_mask, all devices will be
             * disabled. Loop will exit when all of the data is read and is
             * written to the file or network port. */
            pcapDisable.radioType = radioType;
            pcapDisable.radioNum  = radioNum;
            risRet = wsuRisPcapDisable(&pcapDisable);

            if (risRet != RS_SUCCESS) {
                fprintf(stderr, "capture_pcap: wsuRisPcapDisable() failed: %s\n",
                        rsResultCodeType2Str(risRet));
                goto err_exit;
            }
#ifdef DEBUG_PRINTF
            printf("DEBUG: %s: PCAP disabled\n", __func__);
#endif

            exit_state = FINISHING_OFF;
        }

        /* Attempt to read PCAP data. */
        len = pcapReadData(pcapBuffer, PCAP_LOG_BUF_SIZE, radioType, radioNum, FALSE);
        if((PCAP_LOG_BUF_SIZE < len) || (len < 0)){
            goto err_exit1;
        }
        /* Note that if we are logging two interfaces similtaneously, we only
         * need to get data from 1 interface. Both interfaces are logging to
         * the same file, so getting data from that file gets data for both
         * interfaces. */

        if ((len > 0) && (exit_state == RUNNING)) {
#ifdef DEBUG_PRINTF
            printf("DEBUG: %s: Read %d PCAP data bytes\n", __func__, len);
#endif
            /* We got data; write it to the output file */
            if (write_out_file_data(pcapBuffer, (uint32_t)len, FALSE) == -1) {
                if (network) {
                    fprintf(stderr, "capture_pcap: Error writing PCAP data to network: %s\n", strerror(errno));
                }
                else {
                    fprintf(stderr, "capture_pcap: Error writing PCAP data to output file %s: %s\n", out_file_name, strerror(errno));
                }

                goto err_exit1;
            }
#ifdef DEBUG_PRINTF
            printf("DEBUG: %s: Wrote PCAP data bytes\n", __func__);
#endif
        }

        if (pcap_tx_rx_separate) {
            /* Attempt to read Tx PCAP data */
            len_tx = pcapReadData(pcapBuffer, PCAP_TX_LOG_BUF_SIZE, radioType, radioNum, TRUE);

            if ((len_tx > 0) && (exit_state == RUNNING)) {
#ifdef DEBUG_PRINTF
                printf("DEBUG: %s: Read %d PCAP data bytes for Tx\n", __func__, len_tx);
#endif
                /* We got data; write it to the output file */
                if (write_out_file_data(pcapBuffer, (uint32_t)len_tx, TRUE) == -1) {
                    if (network) {
                        fprintf(stderr, "capture_pcap: Error writing PCAP data to network: %s\n", strerror(errno));
                    }
                    else {
                        fprintf(stderr, "capture_pcap: Error writing PCAP data to output file %s: %s\n", out_file_name, strerror(errno));
                    }
                    goto err_exit1;
                }
#ifdef DEBUG_PRINTF
                printf("DEBUG: %s: Wrote PCAP data bytes for Tx\n", __func__);
#endif
            }
        }
#if defined(MY_UNIT_TEST)
        usleep(100 * 1000); /* Always sleep to avoid VM getting too busy */
#else
        if (len == 0 && len_tx == 0) {
            if (exit_state == RUNNING) {
                /* If no data was read, but we are still running, sleep for
                 * either 10 (with Rx) or 20 (without Rx) milliseconds */
                usleep(sleep_time);
            }
        }
#endif
    } while (exit_state != FINISHING_OFF );

    /* Close the output file and exit */
    ret = 0;
    goto common_exit;

err_exit1:
    /* Disable PCAP */
    pcapDisable.radioType = radioType;
    pcapDisable.radioNum  = radioNum;
    risRet = wsuRisPcapDisable(&pcapDisable);

    if (risRet != RS_SUCCESS) {
        fprintf(stderr, "capture_pcap: wsuRisPcapDisable() failed: %s\n",
                rsResultCodeType2Str(risRet));
    }
#ifdef DEBUG_PRINTF
    printf("DEBUG: %s: err_exit1: PCAP disabled\n", __func__);
#endif

err_exit:
#ifdef DEBUG_PRINTF
    printf("DEBUG: %s: err_exit\n", __func__);
#endif
    ret = -1;

common_exit:
    if (network) {
        if (sock >= 0) {
            close(sock);
#ifdef DEBUG_PRINTF
            printf("DEBUG: %s: Network socket closed\n", __func__);
#endif
        }

        if (sock_tx >= 0) {
            close(sock_tx);
#ifdef DEBUG_PRINTF
            printf("DEBUG: %s: Network Tx socket closed\n", __func__);
#endif
        }
    }
    else {
#ifdef DEBUG_PRINTF
        printf("DEBUG: %s: Closing %s\n", __func__, out_file_name);

        if (pcap_tx_rx_separate) {
            printf("DEBUG: %s: Closing %s\n", __func__, out_file_name_tx);
        }
#endif
        CLOSE_OUT_FILES;
        if(!use_absolute_out_file_name) {
            move_logs();
        }
    }

    return ret;
}

