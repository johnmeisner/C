/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Filename: lpwrmonitor.c                                                    */
/* Purpose: Low Power Monitor                                                 */
/*                                                                            */
/* Copyright (C) 2021 DENSO International America, Inc.  All Rights Reserved. */
/*                                                                            */
/* Author(s)/Change History:                                                  */
/*     20210719 LN - Initial revision                                         */
/*                                                                            */
/*----------------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <mtd/mtd-user.h>
#include <sys/stat.h>
#include <time.h>
#include <pthread.h>
#include "dn_types.h"
#include "wsu_sharedmem.h"
#include "rsuhealth.h"
#include "i2v_util.h"

#if defined(MY_UNIT_TEST)
#include "stubs.h"
#define MAIN lpwr_main
#else
#define MAIN main
#endif

/*
 * Debug
 */
#define LPWR_INFO 0
#define LPWR_DEBUG 0
#define EXTRA_DEBUG 0
/*
 * debug macro
 */
#define DEBUG_PRINT 0
#define debug_print(fmt, ...) \
        do { if (DEBUG_PRINT) printf(fmt, ##__VA_ARGS__); } while (0)
                                
/* 
 * Errors
 */
#define PART_MAP_CLEAN      -1
#define PART_MAP_CORRUPT    -2
#define INCOMPLETE_ERASE    -3

/* ========= Low power patition - 128KB =============
 * 
 * - First 512 bytes are reserved for partition maps
 * - The remaining space -> 510 pages of size 256 bytes each
 *          ------------------------
 *          |     Partition Map    |
 *          |      (512 bytes      |
 *          ------------------------
 *          |  page 0 (256 bytes)  |    <- first page
 *          ------------------------
 *          |  page 2 (256 bytes)  |
 *          ------------------------
 *          |  page 3 (256 bytes)  |
 *          ------------------------
 *          |         ...          |
 *          |                      |
 *          ------------------------
 *          |  page 509 (256 bytes)|    <- last page
 *          ------------------------
 * 
 *  Partion map
 *  - Each byte in the partition map represent a coresponding page
 *  - Default value is 0xFF
 *  - A value is set to 0x01 to indicate the page has been marked for use
 *    to store low power info when low power is triggered
 *  Ex:
 *      Address
 *      -------  
 *             pg0 pg1 pg2 pg3 pg4 pg5
 *               |   |   |   |   |   |
 *               v   v   v   v   v   v
 *      0000000 01  01  01  01  01  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff
 *      0000010 ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff  ff
 * 
 *  This partition map showed that 5 pages has been marked 
 *  for used with last marked bytes showing pg4 is the current working page
 * 
 * Page - 256 bytes
 *  - 2 bytes at the end reserved for crc16
 * |  254 low power info | 2 bytes crc16 |
 * 
 * ex:
 * page 0:  | time stamp, reset cause, cpu temp, ... | crc16 of 252 low power info |
 * page 1:  | time stamp, reset cause, cpu temp, ... | crc16 of 252 low power info |
 *                     ...
 * page 9:  | time stamp, reset cause, cpu temp, ... | crc16 of 252 low power info |
 * 
 * 
 * Operation:
 * - On 1st boot, 
 *      - the partition  will be erased to ensure it is cleaned
 *      - page 0 will be marked as current working page
 * - subsequent boots
 *      - next page is marked as current working page
 *      - when all 510 the pages has been marked for used
 *        a wrap around condition is then detected in which
 *        an erase of the partion is performed and current page is reset to 0
 * 
 */
/* Mandatory logging defines. */
#if defined(MY_UNIT_TEST)
#define MY_ERR_LEVEL   LEVEL_DBG
#else
#define MY_ERR_LEVEL   LEVEL_PRIV /* syslog only, no serial. */
#endif
#define MY_NAME        "lpwr"

#define DEVID_MAXLEN    32
#define LOGMSG_MAXLEN   256
#define TS_MSGSIZE      30      /* Timestamp msg size */
/*
 * 230821 224251|pg 65|XXXXXXXX|(0x800000)CPU(-1,-1,-1)PMIC(-1,-1,-1)F1(-1,-1,-1)F2(-1,-1,-1)DCIN(-nan,-nan,-nan)DIAG(0xffffffffffffffff)GNSS(0xffffffff)CV2X(0xffffffff)SYS(0xffffffff)
 */
#define LOGMSG_FORMAT "%s|pg %d|%s|(0x%x)CPU(%d,%d,%d)PMIC(%d,%d,%d)F1(%d,%d,%d)F2(%d,%d,%d)DCIN(%.1f,%.1f,%.1f)DIAG(0x%lx)GNSS(0x%x)CV2X(0x%x)SYS(0x%x)"

#define PAGE_SIZE       256
#define PARTITION_SIZE  131072  /* 128 KB */
#if defined(PLATFORM_HD_RSU_5940)
#define NUM_PAGE        255     /* Number of pages in the partion - see above - workaround for 5940*/
#else
#define NUM_PAGE        510     /* Number of pages in the partion - see above*/
#endif
#define MAP_SIZE        512
#define STRPG_OFFSET    512     /* Offset into the 1st page */
#define SLEEP_TIMER     15

/* 
 * lpwr int interface 
 */
#if defined(MY_UNIT_TEST)
#define LPWR_INT         "/tmp/dev/lpwrint"
#define LPWR_PARTITION   "/tmp/dev/mtd10"
#define DEVID_PARTITION  "/tmp/dev/mtd9"
#define LOG_DIR          "/tmp/rwflash/logs"
#define LOG_DIR_DENSO    "/tmp/rwflash/logs/Denso"
#define LOG_FILE         "/tmp/rwflash/logs/Denso/pwrloss.log"
#define CNT_FILE         "/tmp/rwflash/logs/Denso/reset_cnt.log"
#define AERO_FILE        "/tmp/rwflash/aerolink-1.log"
#else
#define LPWR_INT         "/dev/lpwrint"
#define LPWR_PARTITION   "/dev/mtd10"
#define DEVID_PARTITION  "/dev/mtd9"
#define LOG_DIR          "/rwflash/logs"
#define LOG_DIR_DENSO    "/rwflash/logs/Denso"
#define LOG_FILE         "/rwflash/logs/Denso/pwrloss.log"
#define CNT_FILE         "/rwflash/logs/Denso/reset_cnt.log"
#define AERO_FILE        "/rwflash/aerolink-1.log"
#endif
#define U_RESET_TOKEN    "u_reset_cnt"
#define P_RESET_TOKEN    "p_reset_cnt"


/*
 * Files to be checked
 */
#define NUM_LOG_FILE 3
const char_t * const log_files[NUM_LOG_FILE] = {AERO_FILE,LOG_FILE,CNT_FILE};

/* 
 * Reset types 
 */
typedef enum {
    PWR_LOSS = 0,
    RH850_TIMEOUT = 1,
    IMX8_TIMEOUT = 2,
    SOFT_REBOOT = 3,
} reset_type_t;

typedef struct lpwr_info{
    uint64_t  timestamp;
    uint8_t   reset_cause;
    int32_t   cpu_temp;
    int32_t   cpu_temp_min;
    int32_t   cpu_temp_max;
    int32_t   pmic_temp;
    int32_t   pmic_temp_min;
    int32_t   pmic_temp_max;
    int32_t   fan1_temp;
    int32_t   fan1_temp_min;
    int32_t   fan1_temp_max;
    int32_t   fan2_temp;
    int32_t   fan2_temp_min;
    int32_t   fan2_temp_max;
    float32_t dcin_voltage;
    float32_t dcin_voltage_min;
    float32_t dcin_voltage_max;
    /*
     *rsu states 
     */
    uint32_t  error_states;
    uint64_t  rsudiag_states;
    uint32_t  gnss_states;
    uint32_t  cv2x_states;
    uint32_t  sys_states;
} lpwr_info;

/*
 * Globals
 */
static char_t  log_msg[LOGMSG_MAXLEN];
static uint8_t lpwr_buf[PAGE_SIZE];                  /* shared mem */
static lpwr_info *lwpr_info_256 = (lpwr_info *)lpwr_buf;   /* Map to shared mem */
static uint8_t partition_map[MAP_SIZE];
static int32_t log_exist = 1;                                  /* used to check if log exitst */
static int32_t mainloop = 1;
static int32_t unexpected_shutdown = 0;
static int32_t planned_shutdown = 0;
static pthread_t rsudiag_threadID;
static int32_t pwrloss_pg = 0;

/* attempts to lock RSUDIAG SHM before giving up for this iteration */
#define MAX_TRY_LOCK  10
/* seconds between sensor samples */
#define RSUDIAG_POLL_TIME  1
/* seconds to wait for LOGMGR to be ready. */
#define LOGMGR_WAIT_TIME   30

#define LPWR_SHM_INIT_FAIL            0x00000001
#define LPWR_SHM_LOCK_FAIL            0x00000002
#define LPWR_SHM_UNLOCK_FAIL          0x00000004
#define LPWR_SHM_VALID_FAIL           0x00000008

#define LPWR_MEMERASE_FAIL            0x00000010
#define LPWR_OPEN_FAIL                0x00000020
#define LPWR_SEEK_FAIL                0x00000040
#define LPWR_READ_FAIL                0x00000080

#define LPWR_MAP_CORRUPT              0x00000100
#define LPWR_INT_OPEN_FAIL            0x00000200
#define LPWR_READ_MAP_FAIL            0x00000400
#define LPWR_PAGE_CORRUPT             0x00000800

#define LPWR_SYSCALL_FAIL             0x00001000
#define LPWR_PWRLOSS_UNKNOWN          0x00002000
#define LPWR_OPEN_LOG_FAIL            0x00004000
#define LPWR_OPEN_CNT_FAIL            0x00008000

#define LPWR_LOG_CHECK_FAIL           0x00010000
#define LPWR_INIT_FAIL                0x00020000
#define LPWR_CRC_FAIL                 0x00040000
#define LPWR_WRITE_FAIL               0x00080000

#define LPWR_ONESHOT_SHM_INIT_FAIL    0x00100000
#define LPWR_ONESHOT_SHM_UNLOCK_FAIL  0x00200000
#define LPWR_ONESHOT_SHM_LOCK_FAIL    0x00400000
#define LPWR_ONESHOT_SHM_VALID_FAIL   0x00800000

#define LPWR_THREAD_FPRINTF_FAIL      0x01000000
#define LPWR_THREAD_FOPEN_FAIL        0x02000000
#define LPWR_EXIT_FOPEN_FAIL          0x04000000
#define LPWR_EXIT_FGETS_FAIL          0x08000000

#define LPWR_NULL_INPUT               0x40000000
#define LPWR_FORK_VIOLATION           0x80000000

/* If enabled then one-shot at start must wait for lock with RSUDIAG. May take time */
//TODO: Never get the lock at shutdown so dont bother. Likely tasks holding it is blocked. */
#define ENABLE_ONESHOT_SHM_LOCK 0

/* Sleep time between checks for valid SHM */
#define MAX_ONESHOT_USLEEP     1000

/* Max attempts of MAX_ONESHOT_USLEEP. A long time but not forever.  */
#define MAX_ONESHOT_TRY_VALID  50

/* Tens of seconds */
#define OUTPUT_MODULUS 3
static uint32_t error_states = 0x0;

static uint32_t amirunning = 0x0;
/*
 * ============ Helper functions ==============
 */

void init_statics(void)
{
  memset(log_msg,0x0,sizeof(log_msg));
  memset(lpwr_buf,0x0,sizeof(lpwr_buf));
  lwpr_info_256 = (lpwr_info *)lpwr_buf;
  memset(partition_map,0x0,sizeof(partition_map));
  log_exist = 1;
  mainloop = 1;
  unexpected_shutdown = 0;
  planned_shutdown = 0;
  error_states = 0x0;
  pwrloss_pg = 0;
}

void get_utc_time(uint64_t timestamp, char_t *buf, int32_t buf_size)
{
    time_t rawtime = timestamp/1000;
    struct tm  ts;
    char_t data[TS_MSGSIZE];
    uint32_t i;
    int32_t j;

    if(NULL == buf) {
        error_states |= LPWR_NULL_INPUT;
        return;
    }
    memset(data,0x0,sizeof(data));
    memset(buf,0x0,buf_size);

    // Format time, "ddd yyyy-mm-dd hh:mm:ss zzz"
    ts = *localtime(&rawtime);
    strftime(data, sizeof(data), "%a %Y-%m-%d %H:%M:%S %Z", &ts);

    //Now chop 'ddd yy' and ' zzz'. Chop '-' and ':'
    for(i=6,j=0;(i<(strlen(data)-4)) && j<buf_size;i++) {
        if('\n' == data[i]) {
            break;
        }
        if((':' != data[i]) && ('-' != data[i])) {
            buf[j] = data[i];
            j++;
        }
    }
}
#if 0
//TODO: Do once only at start: Need ascii hex values not bin values == 64 chars :-S
//    : slap on end: syslog max string less than 256 so need to be smart.
void get_device_id(uint8_t *buf)
{
    //system("read_device_id.sh");
    int32_t fd = -1;
    //uint8_t buf[DEVID_MAXLEN+1];

    memset(buf,0x00,DEVID_MAXLEN+1);
            
    fd = open(DEVID_PARTITION, O_SYNC | O_RDONLY);
    if (fd < 0) {
        printf("lpwr: get_device_id: %s could not be open\n", DEVID_PARTITION);
        return;
    }

    /*=== Read ===*/
    if (0 != lseek(fd, 0, SEEK_SET)) {
        printf("lpwr: get_device_id:lseek() failed\n");
        return;
    }
    read(fd, buf, DEVID_MAXLEN);

    //printf("lpwr: Dev ID: %s\n", buf);
    close(fd);
   
}
#endif

uint16_t crc16(const uint8_t* data_p, uint8_t length)
{
    uint8_t x = 0x0;
    uint16_t crc = 0xFFFF;

    while (length--){
        x = crc >> 8 ^ *data_p++;
        x ^= x>>4;
        crc = (crc << 8) ^ ((uint16_t)(x << 12)) ^ ((uint16_t)(x <<5)) ^ ((uint16_t)x);
    }
    return crc;
}

static int32_t memerase(int32_t fd, struct erase_info_user *erase)
{
#if defined(MY_UNIT_TEST)
  fd = fd;
  erase = erase;
  return 0;
#else
	return ioctl(fd, MEMERASE, erase);
#endif
}

int32_t erase_flash(int32_t fd, u_int32_t offset, u_int32_t bytes)
{
    int32_t err = -1;
    struct erase_info_user erase;
    erase.start = offset;
    erase.length = bytes;
    err = memerase(fd, &erase);
    if (err < 0) {
        printf("lpwr: erase_flash: MEMERASE failed\n");
        error_states |= LPWR_MEMERASE_FAIL;
        return 1;
    }
    return 0;
}

void write_byte(int32_t offset, uint8_t val)
{
#if defined(MY_UNIT_TEST)
    offset = offset;
    val = val;
#else
    int32_t fd = -1;
    uint8_t flag_byte[1] = {val};
    
    /* Open partition */
    if ((fd = open(LPWR_PARTITION, O_SYNC | O_RDWR)) < 0){
        printf("lpwr: %s could not be open\n", LPWR_PARTITION);
        error_states |= LPWR_OPEN_FAIL;
        return;
    }

    /* write  */
    if ( offset != lseek(fd, offset, SEEK_SET)) {
        printf("lpwr: Error lseek to %d \n", offset);
        error_states |= LPWR_SEEK_FAIL;
        return;
    }
    write(fd, flag_byte, 1);
    
    /* close partition */
    close(fd);
#endif
}

int32_t erase_lwpr_partition(void)
{
    int32_t ret = 0;
    int32_t fd = -1;


    /* Set start erase flag */
    write_byte(510, 0x02);
    
    /* Open partition */
    if ((fd = open(LPWR_PARTITION, O_SYNC | O_RDWR)) < 0){
        printf("lpwr: %s could not be open\n", LPWR_PARTITION);
        error_states |= LPWR_OPEN_FAIL;
        return errno;
    }

    ret = erase_flash(fd, 0, PARTITION_SIZE);

    /* close partition */
    close(fd);

    /* Set end erase flag */
    write_byte(511, 0x03);

    return ret;
}

int32_t read_lpwr_info(uint8_t *buf)
{
    int32_t fd = -1;
    int32_t ret = 0;

    memset(buf,0x00,PAGE_SIZE);
            
    /* Open partition */
    if ((fd = open(LPWR_PARTITION, O_SYNC | O_RDWR)) < 0){
        printf("lpwr: %s could not be open\n", LPWR_PARTITION);
        error_states |= LPWR_OPEN_FAIL;
        return errno;
    }

    /*=== Read ===*/
    if (0 != lseek(fd, 0, SEEK_SET)) {
        printf("lpwr: read_lpwr_info: lseek() failed\n");
        error_states |= LPWR_SEEK_FAIL;
        return 1;
    }
    if(read(fd, buf, PAGE_SIZE) <= 0){
        error_states |= LPWR_SYSCALL_FAIL;
    }

#if LPWR_DEBUG
    /* Print result */
    int32_t offset = 0;
    while (offset < PAGE_SIZE) {
            printf("0x%04x:", offset);
            for (int32_t i = offset; i < offset+16; i++)
                    printf(" %02x", buf[i]);
            printf("\n");
            offset += 16;
    }
#endif  

    /* close partition */
    close(fd);

    return ret;
}

int32_t read_pwrloss_page(uint8_t *buf, int32_t pg_num)
{
    int32_t fd = -1;
    int32_t ret = 0;
    int32_t pg_offset = STRPG_OFFSET; // set to start page offset

    memset(buf,0x00,PAGE_SIZE);
            
    /* Open partition */
    if ((fd = open(LPWR_PARTITION, O_SYNC | O_RDWR)) < 0){
        printf("lpwr: %s could not be open\n", LPWR_PARTITION);
        error_states |= LPWR_OPEN_FAIL;
        return errno;
    }

    /*=== Read ===*/
    pg_offset += pg_num * PAGE_SIZE;
    if ( pg_offset  != lseek(fd, pg_offset, SEEK_SET)) {
        printf("lpwr: read_pwrloss_page: lseek() failed\n");
        error_states |= LPWR_SEEK_FAIL;
        return -1;
    }
    if(read(fd, buf, PAGE_SIZE) <= 0){
        error_states |= LPWR_SYSCALL_FAIL;
    }
#if LPWR_DEBUG
    /* Print result */
    int32_t offset = 0;
    while (offset < PAGE_SIZE) {
            printf("0x%04x:", offset);
            for (int32_t i = offset; i < offset+16; i++)
                    printf(" %02x", buf[i]);
            printf("\n");
            offset += 16;
    }
#endif  

    /* close partition */
    close(fd);

    return ret;
}

int32_t read_map(uint8_t *buf, int32_t map_size)
{
    int32_t fd = -1;
    int32_t byte_read = 0;

    memset(buf,0x00,map_size);
            
    /* Open partition */
    if ((fd = open(LPWR_PARTITION, O_SYNC | O_RDWR)) < 0){
        error_states |= LPWR_OPEN_FAIL;
        printf("lpwr: %s could not be open\n", LPWR_PARTITION);
        return errno;
    }

    /*=== Read ===*/
    if ( 0 != lseek(fd, 0, SEEK_SET)) {
        error_states |= LPWR_SEEK_FAIL;
        printf("lpwr: read map: lseek() failed\n");
        return -1;
    }
    byte_read = read(fd, buf, map_size);

#if LPWR_DEBUG
#if !defined(MY_UNIT_TEST)
    /* Print result */
    printf("=== Map === %u bytes\n",byte_read);
    int32_t offset = 0;
    while (offset < map_size) {
            printf("0x%04x:", offset);
            for (int32_t i = offset; i < offset+16; i++)
                    printf(" %02x", buf[i]);
            printf("\n");
            offset += 16;
    }
#endif
#endif  
    /* close partition */
    close(fd);

    return byte_read;
}

int32_t partition_cleaned(uint8_t *buf)
{
    int32_t ret = 1;
    
    /* 
     * Run through read buf to check for cleaned data 'FF' 
     */
    for (int32_t i=0; i<PAGE_SIZE; i++){
        if (buf[i] != 0xFF){
            ret = 0;
            break;
        }
    }
    
    return ret;
}

int32_t page_cleaned(uint8_t *buf)
{
    int32_t ret = -1;
    
    /* 
     * Run through read buf to check for cleaned data 'FF' 
     */
    for (int32_t i=0; i<PAGE_SIZE; i++){
        if (buf[i] != 0xFF){
            ret = 0;
            break;
        }
    }
    
    return ret;
}

uint64_t get_timestamp()
{
    struct timeval tv;

    gettimeofday(&tv, NULL);

    uint64_t millisecondsSinceEpoch =
        (uint64_t)(tv.tv_sec) * 1000 +
        (uint64_t)(tv.tv_usec) / 1000;

    return millisecondsSinceEpoch;
}

int32_t get_working_page(void)
{
    int32_t ret = 0;
    uint8_t buf[PAGE_SIZE];
    int32_t fd = -1;

    memset(buf,0x00,PAGE_SIZE);
            
    /* Open partition
     * Traverse and return a clean page
     */
    
    /* Open partition */
    if ((fd = open(LPWR_PARTITION, O_SYNC | O_RDWR)) < 0){
        printf("lpwr: %s could not be open\n", LPWR_PARTITION);
        error_states |= LPWR_OPEN_FAIL;
        return errno;
    }

    for (int32_t i=0; i< NUM_PAGE; i++){

        /* skip to page and read */
        if ( (i*PAGE_SIZE) != lseek(fd, i*PAGE_SIZE, SEEK_SET)) {
            printf("lpwr: get_working_page: lseek() failed\n");
            error_states |= LPWR_SEEK_FAIL;
            return -1;
        }
        if(read(fd, buf, PAGE_SIZE) <= 0){
            error_states |= LPWR_SYSCALL_FAIL;
        }
        
        /* if page not mark then return current page*/
        if (page_cleaned(buf)){
            ret = i;
            break;
        }
    }

#if LPWR_DEBUG
    /* Print result */
    int32_t offset = 0;
    while (offset < PAGE_SIZE) {
            printf("0x%04x:", offset);
            for (int32_t i = offset; i < offset+16; i++)
                    printf(" %02x", buf[i]);
            printf("\n");
            offset += 16;
    }
#endif

    /* close partition */
    close(fd);

    return ret;
}

int32_t save_lpwr_info(int32_t pg_num)
{
    int32_t err = 0;
    int32_t fd = -1;     /* lpwr partition handle */
#if LPWR_INFO
    struct timeval stop, start;
#endif
    uint64_t time_stamp = 0;
    uint16_t *crc = (uint16_t *)&lpwr_buf[PAGE_SIZE-2];     /* map to the last 2 bytes of lwpr mem */
    int32_t pg_offset = STRPG_OFFSET; // set to start page offset
    
    
#if LPWR_INFO
    /* 
     * get start time - for benchmarking
     */
    gettimeofday(&start, NULL);
#endif

    /* Get timestamp */
    time_stamp = get_timestamp();
    lwpr_info_256->timestamp = time_stamp;
    
    /* compute crc16 */
    *crc = crc16(lpwr_buf, PAGE_SIZE-2);

    /* Open partition */
    if ((fd = open(LPWR_PARTITION, O_SYNC | O_RDWR)) < 0){
        printf("lpwr: lpwr partition %s could not be open\n", LPWR_PARTITION);
        error_states |= LPWR_OPEN_FAIL;
        return errno;
    }

    /* Skip to the working page and save */
    pg_offset += pg_num * PAGE_SIZE;
    if ( pg_offset  != lseek(fd, pg_offset, SEEK_SET)) {
        error_states |= LPWR_SEEK_FAIL;
        return -1;
    }
    if(write(fd, lpwr_buf, PAGE_SIZE) <= 0){
        error_states |= LPWR_SYSCALL_FAIL;
    }    
    close(fd);

    /* 
     * Sync all cached data 
     */
    //system("sync");

#if LPWR_INFO
    /* 
     * get end time - for benchmarking
     */
    gettimeofday(&stop, NULL);
    printf("lpwr: took %lu us\n", (stop.tv_sec - start.tv_sec) * 1000000 + stop.tv_usec - start.tv_usec);        
    printf("lpwr: reset timestamp %lu\n", lwpr_info_256->timestamp);    
    printf("lpwr: crc16 of lpwr bytes: 0x%04x\n", *crc);
#endif
    
    return err;    
    
}
/* Shutting down so pass or fail does not matteer now */
void save_log()
{
    size_t log_msg_len = strlen(log_msg);
    int32_t fd = open(LOG_FILE, O_DSYNC | O_RDWR);
    
    if(fd < 0)
       return;

    lseek(fd, 0, SEEK_END);
    
    /*
     * Write to log file
     */
    log_msg[log_msg_len] = '\n';
    if(write(fd, log_msg, log_msg_len+1) <= 0){
        error_states |= LPWR_SYSCALL_FAIL;
    }
    fsync(fd);
    
    close(fd);
}

void reset_cnt_init()
{
    char_t *log_u_cnt_msg = "u_reset_cnt = 0 \n";
    char_t *log_p_cnt_msg = "p_reset_cnt = 0 \n";
    int32_t u_len = strlen(log_u_cnt_msg);
    int32_t p_len = strlen(log_p_cnt_msg);
    int32_t fd = open(CNT_FILE, O_DSYNC | O_RDWR);
    
    if(fd < 0) {
       printf("lpwr: reset_cnt_init: open failed: errno(%u)\n",errno);
       error_states |= LPWR_OPEN_FAIL;
       return;
    }

    if (0 > lseek(fd, 0, SEEK_SET)){
       printf("lpwr: reset_cnt_init: seek failed: errno(%u)\n",errno);
       error_states |= LPWR_SEEK_FAIL;
       close(fd);
       return;
    }
    
    /*
     * Write to log file
     */
    if (0 > write(fd, log_u_cnt_msg, u_len)){
       printf("lpwr: reset_cnt_init: write failed: errno(%u)\n",errno);
       error_states |= LPWR_WRITE_FAIL;
       close(fd);
       return;
    }
    if (0 > write(fd, log_p_cnt_msg, p_len)){
       printf("lpwr: reset_cnt_init: write failed: errno(%u)\n",errno);
       error_states |= LPWR_WRITE_FAIL;
       close(fd);
       return;
    }
    fsync(fd);
    
    close(fd);
    fd = -1;
}

void get_reset_cnt()
{
    char_t *line1 = NULL, *line2 = NULL;
    char_t u_reset_token[16], u_equal_sign[2];
    char_t p_reset_token[16], p_equal_sign[2];
    int32_t  u_reset = -1, p_reset = -1;
    size_t len = 0;
    FILE *fp = fopen(CNT_FILE, "r+");
    int32_t ret = 0;
    if(NULL == fp) {
       printf("lpwr: get_reset_cnt: open failed: errno(%u)\n",errno);
       error_states |= LPWR_OPEN_FAIL;
       return;
    }

    /* First time, file is empty */
    if(getline(&line1, &len, fp) < 0) {
        ret = LPWR_SYSCALL_FAIL;
    } else {
        if(getline(&line2, &len, fp) < 0) {
            ret = LPWR_SYSCALL_FAIL;
        }
    }
    
    if((0 == ret) && (NULL != line1) && (NULL != line2)) {
        sscanf(line1, "%s %s %d", u_reset_token, u_equal_sign, &u_reset);
        sscanf(line2, "%s %s %d", p_reset_token, p_equal_sign, &p_reset);
#if EXTRA_DEBUG
        printf("lpwr: Read from %s\n", CNT_FILE);
        printf("lpwr: %s %s %d\n", u_reset_token, u_equal_sign, u_reset);
        printf("lpwr: %s %s %d\n", p_reset_token, p_equal_sign, p_reset);
#endif
        unexpected_shutdown = u_reset;
        planned_shutdown = p_reset;
        free(line1);
        free(line2);
        if(NULL != fp) {
            fclose(fp); 
            fp = NULL;
        }
    }
    if(0 < ret) {
        error_states |= ret;
    }
}


void save_msg_pg(char_t *msg, int32_t pg)
{
    uint64_t time_stamp = get_timestamp();
    char_t buf[sizeof(log_msg)-1];
    size_t msg_len = 0;

    memset(log_msg,0x00,sizeof(log_msg));
    memset(buf,0x0,sizeof(buf));

    if(NULL != msg) {
        msg_len = strnlen(msg,sizeof(buf));
        memcpy(buf,msg,msg_len);
    } else {
        error_states |= LPWR_NULL_INPUT;
    }
    get_utc_time(time_stamp, log_msg, TS_MSGSIZE);    /* get string format of timestamp */
    strncat(log_msg, "|",sizeof(log_msg)-1);
    //get_device_id(&log_msg[strlen(log_msg)]);
    //strncat(log_msg, "|",sizeof(log_msg)-1);
    sprintf(&log_msg[strlen(log_msg)], "pg %d", pg);
    strncat(log_msg, "|",sizeof(log_msg)-1);
    strncat(log_msg, buf, sizeof(log_msg) - 1);
    save_log();

    return;
}

void save_msg(char_t *msg)
{
    uint64_t time_stamp = get_timestamp();
    char_t buf[sizeof(log_msg)-1];
    size_t msg_len = 0;

    memset(log_msg,0x00,sizeof(log_msg));
    memset(buf,0x0,sizeof(buf));

    if(NULL != msg) {
        msg_len = strnlen(msg,sizeof(buf));
        memcpy(buf,msg,msg_len);
    } else {
        error_states |= LPWR_NULL_INPUT;
    }
    
    get_utc_time(time_stamp, log_msg, TS_MSGSIZE);    /* get string format of timestamp */
    strncat(log_msg, "|",sizeof(log_msg)-1);
    //get_device_id(&log_msg[strlen(log_msg)]);
    //strncat(log_msg, "|",sizeof(log_msg)-1);
    strncat(log_msg, buf, sizeof(log_msg) - 1);
    save_log();

    return;
}

int32_t init_lpwr(void)
{
    int32_t ret = 0;
    char_t log_folder[] = LOG_DIR;
    char_t log_folder_denso[] = LOG_DIR_DENSO;
    struct stat sb;
    FILE *log_file=NULL;
    FILE *cnt_file=NULL;
    int32_t fd = -1;
    /*
     * Make sure log directory and files exist
     */
    if (stat(log_folder, &sb) == 0 && S_ISDIR(sb.st_mode)) {
        #if EXTRA_DEBUG
        printf("lpwr: Dir %s exist\n", LOG_DIR);
        #endif
    } else {
        #if EXTRA_DEBUG
        printf("lpwr: %s does not exist\n", LOG_DIR);
        #endif
        mkdir(log_folder, 0777);   /* create folder if does not exit */
    }

    if (stat(log_folder_denso, &sb) == 0 && S_ISDIR(sb.st_mode)) {
        #if EXTRA_DEBUG
        printf("lpwr: Dir %s exist\n", LOG_DIR_DENSO);
        #endif
    } else {
        #if EXTRA_DEBUG
        printf("lpwr: %s does not exist\n", LOG_DIR_DENSO);
        #endif
        mkdir(log_folder_denso, 0777);   /* create folder if does not exit */
    }

    /* check log file */
    if ((log_file = fopen(LOG_FILE, "r")) == NULL) {
        if (errno == ENOENT) {
            #if EXTRA_DEBUG
            printf("lpwr: %s doesn't exist\n", LOG_FILE);
            #endif
            fd = open(LOG_FILE, O_RDWR|O_CREAT, 0777);   /* create log file if does not exist */
            log_exist = 0;
            close(fd);  
            /* Erase lwpr partition */
            if (erase_lwpr_partition()){
                printf("lpwr: Error erasing %s: errno(%d)\n", LPWR_PARTITION,errno);
                return -1;
            }
            
            /* Log Erase message */
            
        } else {
            // Check for other errors too, like EACCES and EISDIR
            printf("lpwr: Some other error occured");
            error_states |= LPWR_OPEN_LOG_FAIL;
        }
    } else {
        fclose(log_file);
    }

    /* check reset count file */
    if ((cnt_file = fopen(CNT_FILE, "r")) == NULL) {
        if (errno == ENOENT) {
            #if EXTRA_DEBUG
            printf("lpwr: %s doesn't exist\n", CNT_FILE);
            #endif
            fd = open(CNT_FILE, O_RDWR|O_CREAT, 0777);   /* create log file if does not exist */
            log_exist = 0;
            close(fd);  
            reset_cnt_init();
        } else {
            // Check for other errors too, like EACCES and EISDIR
            printf("lpwr: Some other error occured");
            error_states |= LPWR_OPEN_CNT_FAIL;
        }
    } else {
        fclose(cnt_file);
    }

    return ret;
}

int32_t get_pwrloss_page(uint8_t * map)
{
    int32_t ret_pg = PART_MAP_CLEAN;
    int32_t i = -1;
    
    
    /* 
     * Check for incomplete erase if any
     * - Note: the last 2 byte of the map is used for marking erase operations
     *      - byte 510 set to 0x02 prior to erase operation
     *      - byte 511 set to 0x03 once erase operation completed
     *      - success: if byte 511 = 0x03
     *      - incomplete : if byte 510 = 0x02 or byte 511 != 0x03
     */
#if 0    
    if (map[511] != 0x03){
        return INCOMPLETE_ERASE;
    }
    
    if (map[510] == 0x02){
        return INCOMPLETE_ERASE;
    }
#endif
    
    /*
     * Traverse map to check
     * - possible corruption - value must be 0x01 or 0xFF
     */
    
    for (i=0; i<NUM_PAGE; i++){
        /* check for corruption */
        //if ( (map[i] != 0xff) && (map[i] != 0x01) ){
        // 5940 workaround
        if ( (map[i*2] != 0xff) && (map[i*2] != 0x01) ){
            debug_print("Corruption found in partition map! - at loc %d, char %u\n", i, map[i]);
            error_states |= LPWR_MAP_CORRUPT;
            return PART_MAP_CORRUPT;
        }
        
        //if (map[i] == 0xff){
        // 5940 workaround
        if (map[i*2] == 0xff){
            ret_pg = i-1;
            break;
        }
        ret_pg = i;
    }
    
    /* continue to traverse when 0xff found
     * anything other than 0xff downstream 
     * would construe as corrupted
     */
    #if EXTRA_DEBUG
    printf("lpwr: ret_pg %d\n",ret_pg);
    #endif
    
#if 0
{
  int32_t j;
    for (j=ret_pg+1; j<NUM_PAGE; j++){
        if (map[j] != 0xff){
            debug_print("Corruption found in partition map! - after 0xff found at %d\n", j);
            error_states |= LPWR_MAP_CORRUPT;
            return PART_MAP_CORRUPT;
        }
        
    }
}
#endif    
    return ret_pg;
}

int32_t page_corrupted(uint8_t *page_buff)
{
    int32_t ret = 0;
    uint16_t *read_crc = (uint16_t*)&page_buff[PAGE_SIZE-2];      // last 2 bytes of buffer
    uint16_t cal_crc = 0x0000;
    
    cal_crc = crc16(page_buff, PAGE_SIZE-2);   /* calculate crc */
    
    if (cal_crc != *read_crc){
        printf("lpwr: Error! %s corruption detected -  crc read: 0x%04x | calculated crc: 0x%04x\n", LPWR_PARTITION, *read_crc, cal_crc);
        error_states |= LPWR_CRC_FAIL;
        ret = 1;
    }
    
    return ret;
}


void mark_page(int32_t page_num)
{
    int32_t fd = -1;
    uint8_t mask_byte[1] = {0x01};
    
    /* Open partition */
    if ((fd = open(LPWR_PARTITION, O_SYNC | O_RDWR)) < 0){
        printf("lpwr: mark_page: open failed: errno(%u)\n",errno);
        error_states |= LPWR_OPEN_FAIL;
        return;
    }
#if 0
    /* write 0x01 as mask */
    if ( page_num != lseek(fd, page_num, SEEK_SET)) {
        printf("lpwr: mark_page: seek to %d failed: errno(%u)\n",page_num,errno);
        error_states |= LPWR_SEEK_FAIL;
        return;
    }
#endif
    /* write 0x01 as mask - 5940 workaround */
    if ( page_num*2 != lseek(fd, page_num*2, SEEK_SET)) {
        printf("lpwr: mark_page: seek to %d failed: errno(%u)\n",page_num*2,errno);
        error_states |= LPWR_SEEK_FAIL;
        return;
    }
    if(0 > write(fd, mask_byte, 1)) {
        printf("lpwr: mark_page: write failed: errno(%u)\n",errno);
        error_states |= LPWR_WRITE_FAIL;
    }
    
    /* close partition */
    close(fd);
    fd = -1;
}


void check_file_integrity(void)
{
    int32_t fd = -1;
    int32_t i = 0;
    char_t tmp_msg[LOGMSG_MAXLEN];

    memset(tmp_msg,0x0,sizeof(tmp_msg));
    /* check log files*/
    for (i=0; i<NUM_LOG_FILE; i++){
        fd = open(log_files[i], O_RDONLY);
        // log error
//TODO: Purge corrupt files?
        if (fd == -1){
            printf("lpwr: Integrity check error %d on file %s\n", errno, log_files[i]);
            error_states |= LPWR_LOG_CHECK_FAIL;
            if (errno !=2) {
                sprintf(tmp_msg, "Integrity check error %d on file %s", errno, log_files[i]);
                save_msg(tmp_msg);
            }
        } else {
            #if EXTRA_DEBUG 
            printf("lpwr: Integrity check OK on file %s\n", log_files[i]);
            #endif
            close(fd);
            fd = -1;
        }
    }
}
/*
 * rsudiag_handler()
 *
 * Handler of rsudiag thread
 *
 */
static void *rsudiag_thread(void *ptr)
{
  int32_t i = 0;
  uint32_t iterations = 0;
  lpwr_info *lwpr_info_tmp = (lpwr_info *)lpwr_buf;
  uint32_t prior_error_states = 0x0;
  rsuhealth_t * shm_rsuhealth_ptr = NULL;
  char_t buf[LOGMSG_MAXLEN];
  char_t mydate[TS_MSGSIZE]; /* stncat: dest > src */

  iterations = 0;
  prior_error_states = 0x0;
  shm_rsuhealth_ptr = NULL;
  ptr = ptr; /* silence warning */

  while (mainloop) {
      sleep(RSUDIAG_POLL_TIME);
      iterations++;
      if(LOGMGR_WAIT_TIME == iterations) { /* LOGMGR will be up shortly. 30 seconds is long enough */
          i2vUtilEnableDebug(MY_NAME); /* Limit serial spew at boot so wait a bit */
          i2vUtilEnableSyslog(MY_ERR_LEVEL, MY_NAME);  /* If we attempt before LOGMGR then mq_open fail so just wait a bit */
      }
      if(NULL == shm_rsuhealth_ptr) {
          if (NULL == (shm_rsuhealth_ptr = wsu_share_init(sizeof(rsuhealth_t), RSUHEALTH_SHM_PATH))) {
              if(0x0 == (error_states & LPWR_SHM_INIT_FAIL)) {
                  error_states |= LPWR_SHM_INIT_FAIL;
                  I2V_DBG_LOG(LEVEL_CRIT, MY_NAME,"SHM init failed.\n");
              }
              shm_rsuhealth_ptr = NULL; /* Keep going, not fatal. */
              error_states |= LPWR_SHM_INIT_FAIL;
          } else {
              error_states &= ~LPWR_SHM_INIT_FAIL;
              #if EXTRA_DEBUG
              I2V_DBG_LOG(LEVEL_CRIT, MY_NAME,"SHM init success.\n");
              #endif
          }
      }
      if(NULL != shm_rsuhealth_ptr) {
          error_states &= ~LPWR_SHM_INIT_FAIL;
          for(i = 0; mainloop && (i < MAX_TRY_LOCK); i++) {
              if(WTRUE == wsu_shmlock_trylockr(&shm_rsuhealth_ptr->h.ch_lock)) {
                  error_states &= ~LPWR_SHM_LOCK_FAIL;
                  if(WTRUE == shm_rsuhealth_ptr->h.ch_data_valid) {
                      lwpr_info_tmp->cpu_temp       = (int32_t)shm_rsuhealth_ptr->temperature.cpu;
                      lwpr_info_tmp->cpu_temp_min   = (int32_t)shm_rsuhealth_ptr->temperature.cpumin;
                      lwpr_info_tmp->cpu_temp_max   = (int32_t)shm_rsuhealth_ptr->temperature.cpumax;
                      lwpr_info_tmp->pmic_temp      = (int32_t)shm_rsuhealth_ptr->temperature.cv2x;
                      lwpr_info_tmp->pmic_temp_min  = (int32_t)shm_rsuhealth_ptr->temperature.cv2xmin;
                      lwpr_info_tmp->pmic_temp_max  = (int32_t)shm_rsuhealth_ptr->temperature.cv2xmax;
                      lwpr_info_tmp->fan1_temp      = (int32_t)shm_rsuhealth_ptr->temperature.hsm;
                      lwpr_info_tmp->fan1_temp_min  = (int32_t)shm_rsuhealth_ptr->temperature.hsmmin;
                      lwpr_info_tmp->fan1_temp_max  = (int32_t)shm_rsuhealth_ptr->temperature.hsmmax;
                      lwpr_info_tmp->fan2_temp      = (int32_t)shm_rsuhealth_ptr->temperature.ambient;
                      lwpr_info_tmp->fan2_temp_min  = (int32_t)shm_rsuhealth_ptr->temperature.ambientmin;
                      lwpr_info_tmp->fan2_temp_max  = (int32_t)shm_rsuhealth_ptr->temperature.ambientmax;
                      lwpr_info_tmp->dcin_voltage     = (float32_t)shm_rsuhealth_ptr->volts.dcin;
                      lwpr_info_tmp->dcin_voltage_min = (float32_t)shm_rsuhealth_ptr->volts.dcinmin;
                      lwpr_info_tmp->dcin_voltage_max = (float32_t)shm_rsuhealth_ptr->volts.dcinmax;
                      lwpr_info_tmp->rsudiag_states = (uint64_t)shm_rsuhealth_ptr->errorstates;
                      lwpr_info_tmp->gnss_states    = (uint32_t)shm_rsuhealth_ptr->gnssstatus;
                      lwpr_info_tmp->cv2x_states    = (uint32_t)shm_rsuhealth_ptr->cv2xstatus;
                      lwpr_info_tmp->sys_states     = (uint32_t)shm_rsuhealth_ptr->systemstates;
                      if(WFALSE == wsu_shmlock_unlockr(&shm_rsuhealth_ptr->h.ch_lock)) {
                          if(0x0 == (error_states & LPWR_SHM_UNLOCK_FAIL)){
                              error_states |= LPWR_SHM_UNLOCK_FAIL;
                              I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"wsu_shmlock_unlockr() failed.\n");
                          }
                      }
                      error_states &= ~LPWR_SHM_VALID_FAIL;
                      break; /* Done. Exit loop now. */
                  } else {
                      if(0x0 == (error_states & LPWR_SHM_VALID_FAIL)) {
                          error_states |= LPWR_SHM_VALID_FAIL;
                          I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"shm not valid yet.\n");
                      }
                  }
                  if(WFALSE == wsu_shmlock_unlockr(&shm_rsuhealth_ptr->h.ch_lock)) {
                      if(0x0 == (error_states & LPWR_SHM_UNLOCK_FAIL)) {
                          error_states |= LPWR_SHM_UNLOCK_FAIL;
                          I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"wsu_shmlock_unlockr() failed.\n");
                      }
                  }
              }
              usleep(1000);
          }
          if (MAX_TRY_LOCK <= i) { /* Could happen, not end of the world. */
              if(0x0 == (error_states & LPWR_SHM_LOCK_FAIL)) {
                  error_states |= LPWR_SHM_LOCK_FAIL;
                  I2V_DBG_LOG(MY_ERR_LEVEL, MY_NAME,"wsu_shmlock_lockr() failed.\n");
              }
          }
      } else {
          if(0x0 == (error_states & LPWR_SHM_INIT_FAIL)) {
              error_states |= LPWR_SHM_INIT_FAIL;
              I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"NULL == shm_rsuhealth_ptr.\n");
          }
      }
      lwpr_info_tmp->error_states = error_states;
      memset(buf,0x0,sizeof(buf));
      memset(mydate,0x0,sizeof(mydate));
      get_utc_time(get_timestamp(), mydate, TS_MSGSIZE);
      snprintf(buf,sizeof(buf),
          LOGMSG_FORMAT
          ,mydate
          ,pwrloss_pg
          ,"Reboot"
          ,error_states
          ,lwpr_info_tmp->cpu_temp
          ,lwpr_info_tmp->cpu_temp_min
          ,lwpr_info_tmp->cpu_temp_max
          ,lwpr_info_tmp->pmic_temp
          ,lwpr_info_tmp->pmic_temp_min
          ,lwpr_info_tmp->pmic_temp_max
          ,lwpr_info_tmp->fan1_temp
          ,lwpr_info_tmp->fan1_temp_min
          ,lwpr_info_tmp->fan1_temp_max
          ,lwpr_info_tmp->fan2_temp
          ,lwpr_info_tmp->fan2_temp_min
          ,lwpr_info_tmp->fan2_temp_max
          ,lwpr_info_tmp->dcin_voltage
          ,lwpr_info_tmp->dcin_voltage_min
          ,lwpr_info_tmp->dcin_voltage_max
          ,lwpr_info_tmp->rsudiag_states
          ,lwpr_info_tmp->gnss_states
          ,lwpr_info_tmp->cv2x_states
          ,lwpr_info_tmp->sys_states);
#if EXTRA_DEBUG
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s)\n",buf);
#endif
      if(prior_error_states != error_states) { /* Output only on event change */
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%u): prior(0x%x) now(0x%x)\n",iterations,prior_error_states,error_states);
          prior_error_states = error_states;
      }
#if EXTRA_DEBUG
      if((0x0 == (iterations % OUTPUT_MODULUS)) && error_states) {
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%u): (0x%x)\n",iterations,error_states);
      }
#endif
  } /* while */
  I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"EXIT:(0x%x)\n",error_states);
  if(NULL != shm_rsuhealth_ptr) {
      wsu_share_kill(shm_rsuhealth_ptr, sizeof(rsuhealth_t));
      shm_rsuhealth_ptr = NULL;
  }
  /* May never get here but be polite and try to clean up. */
  i2vUtilDisableSyslog();
  i2vUtilDisableDebug();

  pthread_exit(0);
}
int32_t MAIN(int32_t argc, char_t *argv[])
{
    int32_t ret = 0;
    int32_t fd_int = 0;     /* lpwr interrupt file handle */
    //uint8_t buf[PAGE_SIZE];
    uint8_t tmp_buf[10];
    int32_t byte_read = 0;
    lpwr_info *lwpr_info_tmp = (lpwr_info *)lpwr_buf;
    int32_t curr_page = 0;      /* current working page */
    char_t tmp_msg[128];
    char_t mydate[TS_MSGSIZE];
    char_t myreboot[16];
    FILE *fp = NULL;

    argc = argc; /* silence warnings */
    argv = argv;

    if(0x0 == amirunning) {
        amirunning = 0x1;
    } else {
        error_states |= LPWR_FORK_VIOLATION;
        printf("lpwr: already running, terminating(%d)\n",getpid());
        return LPWR_FORK_VIOLATION;
    }

    init_statics();

    if(0 != init_lpwr()) {
        error_states |= LPWR_INIT_FAIL;
    }
    get_reset_cnt();
                
    /* Open lpwr interrupt interface */
    if ((fd_int = open(LPWR_INT, O_SYNC | O_RDWR)) < 0){
        printf("lpwr: %s could not be open\n", LPWR_INT);
        error_states |= LPWR_INT_OPEN_FAIL;
        mainloop = WFALSE; /* Signal thread we are done */
        return errno;
    }
    #if EXTRA_DEBUG
    else {
        printf("lpwr: %s opened\n", LPWR_INT);
    }
    #endif

    /* get partion map - and 2 additional erase flags bytes */
    if ( MAP_SIZE != read_map(partition_map, MAP_SIZE) ){
        printf("lpwr: Error reading partition map!\n");
        ret = -1;
        error_states |= LPWR_READ_MAP_FAIL;
        mainloop = WFALSE; /* Signal thread we are done */
        goto lpwr_exit;
    }
    
    /* return pwrloss page
     * return -1 if map is cleaned
     */
    pwrloss_pg = get_pwrloss_page(partition_map);
    #if EXTRA_DEBUG
    printf("lpwr: pwrloss_pg: %d\n", pwrloss_pg); 
    #endif
    /* Save pwr loss info to log */
    if (pwrloss_pg >= 0){
        read_pwrloss_page(lpwr_buf, pwrloss_pg);
        
        /* Perform check on page */
        if (!page_cleaned(lpwr_buf)){ /* Get from last shutdown and then output */
            /* if not clean - check for page corruption */
            if (!page_corrupted(lpwr_buf)){
                /* page not corrupted - contruct message and save to log */
                memset(mydate,0x0,sizeof(mydate));
                memset(myreboot,0x0,sizeof(myreboot));
                memset(log_msg,0x0,sizeof(log_msg));

                get_utc_time(lwpr_info_tmp->timestamp, mydate, TS_MSGSIZE);    /* get string format of timestamp */
                //get_device_id(mydeviceid);
                switch (lwpr_info_tmp->reset_cause){
                    case PWR_LOSS:
                        strncat(myreboot, "Power",sizeof(myreboot));
                        break;

                    case RH850_TIMEOUT:
                        strncat(myreboot, "RH850 to",sizeof(myreboot));
                        break;

                    case IMX8_TIMEOUT:
                        strncat(myreboot, "IMX8 to",sizeof(myreboot));
                        break;

                    case SOFT_REBOOT:
                        strncat(myreboot, "Reboot",sizeof(myreboot));
                        break;
                    default:
                        strncat(myreboot, "Unknown",sizeof(myreboot));
                        break;
                }

                snprintf(log_msg,sizeof(log_msg),
                    LOGMSG_FORMAT
                    ,mydate
                    ,pwrloss_pg
                    ,myreboot
                    ,error_states
                    ,lwpr_info_tmp->cpu_temp
                    ,lwpr_info_tmp->cpu_temp_min
                    ,lwpr_info_tmp->cpu_temp_max
                    ,lwpr_info_tmp->pmic_temp
                    ,lwpr_info_tmp->pmic_temp_min
                    ,lwpr_info_tmp->pmic_temp_max
                    ,lwpr_info_tmp->fan1_temp
                    ,lwpr_info_tmp->fan1_temp_min
                    ,lwpr_info_tmp->fan1_temp_max
                    ,lwpr_info_tmp->fan2_temp
                    ,lwpr_info_tmp->fan2_temp_min
                    ,lwpr_info_tmp->fan2_temp_max
                    ,lwpr_info_tmp->dcin_voltage
                    ,lwpr_info_tmp->dcin_voltage_min
                    ,lwpr_info_tmp->dcin_voltage_max
                    ,lwpr_info_tmp->rsudiag_states
                    ,lwpr_info_tmp->gnss_states
                    ,lwpr_info_tmp->cv2x_states
                    ,lwpr_info_tmp->sys_states);
                #if EXTRA_DEBUG      
                printf("lpwr: log_msg: %s\n", log_msg);
                #endif
                save_log();
                
                /* Save reset count */
                sprintf(tmp_msg, "sed -i 's/%s = %d/%s = %d/g' %s", U_RESET_TOKEN, unexpected_shutdown, U_RESET_TOKEN, unexpected_shutdown+1, CNT_FILE); 
                if (system(tmp_msg)) {
                    printf("lpwr: Failed to update %s\n", CNT_FILE);
                }
                unexpected_shutdown++;
                #if EXTRA_DEBUG
                printf("lpwr: unexpected_shutdown: %d\n", unexpected_shutdown);
                #endif
                
            } else{
                /* page corrupted - save msg accordingly */
                printf("lpwr: info page %d corrupted!\n", pwrloss_pg);
                error_states |= LPWR_PAGE_CORRUPT;
                save_msg_pg("lpwr info page corrupted!", pwrloss_pg);
            }
            
        } else{
            /* Page is cleaned which means RSU were reboot without low power condition
             * could means soft reset, watchdog time, others
             * Log accordingly
             */
            #if EXTRA_DEBUG 
            printf("lpwr: Shutdown on page %d - Soft Reboot\n", pwrloss_pg);
            #endif
            /* Save reset count */
            sprintf(tmp_msg, "sed -i 's/%s = %d/%s = %d/g' %s", P_RESET_TOKEN, planned_shutdown, P_RESET_TOKEN, planned_shutdown+1, CNT_FILE); 
            if (system(tmp_msg)) {
                #if EXTRA_DEBUG 
                printf("lpwr: Failed to update %s\n", CNT_FILE);
                #endif
                error_states |= LPWR_SYSCALL_FAIL;
            }

            planned_shutdown++;
            #if EXTRA_DEBUG 
            printf("lpwr: planned_shutdown: %d\n", planned_shutdown);
            #endif

            fp = fp;
            memset(log_msg,0x0,sizeof(log_msg));
            memset(mydate,0x0,sizeof(mydate));

//TODO: This has no sensor data from prior boot. Future work.
            get_utc_time(lwpr_info_tmp->timestamp, mydate, TS_MSGSIZE);    /* get string format of timestamp */
                snprintf(log_msg,sizeof(log_msg),
                    "%s|pg %d|%s|(0x%x)"
                    ,mydate
                    ,pwrloss_pg
                    ,"Reboot"
                    ,error_states);
            #if EXTRA_DEBUG     
            printf("lpwr: log_msg: %s\n", log_msg);
            #endif
            save_log();
        }   
    } else{
        /* This means 
         * PART_MAP_CLEAN - 1st time boot/setup or 
         * PART_MAP_CORRUPT - partition map is corrupted
         * INCOMPLETE_ERASE
         */
        switch (pwrloss_pg){
            case PART_MAP_CLEAN:
                debug_print("1st time boot\n");
                save_msg("1st time boot");
                break;
                
            case PART_MAP_CORRUPT:
                printf("lpwr: Partition map corrupted! - Erasing partition\n");
                save_msg("Partition map corrupted! - Erasing Partition");
                if (erase_lwpr_partition()){
                    printf("lpwr: Error erasing %s\n", LPWR_PARTITION);
                    ret = errno;
                    error_states |= LPWR_MAP_CORRUPT;
                    mainloop = WFALSE; /* Signal thread we are done */
                    goto lpwr_exit;
                }
                curr_page = 0;
                goto lpwr_cont;
                break;

            case INCOMPLETE_ERASE:
                printf("lpwr: Incomplete Erase! - Erasing partition again\n");
                save_msg("Incomplete Erase! - Erasing partition again");
                if (erase_lwpr_partition()){
                    printf("lpwr: Error erasing %s\n", LPWR_PARTITION);
                    ret = errno;
                    mainloop = WFALSE; /* Signal thread we are done */
                    goto lpwr_exit;
                }
                curr_page = 0;
                goto lpwr_cont;
                break;

            default:
                printf("lpwr: Power loss unexpected case: pg(%d) %s\n", pwrloss_pg, LPWR_PARTITION);
                error_states |= LPWR_PWRLOSS_UNKNOWN;
                break;
        }
    }
        

    /* 
     * Register for low power interrupt event
     */

    /* Advance current working page */
    curr_page = pwrloss_pg + 1;
    
    /* Erase if partition is full and reset working page to 0 */
    if (curr_page >= NUM_PAGE){
        /* Erase lwpr partition */
        debug_print("Lwpr Partition full! - Erasing Partition\n");
        save_msg("Lwpr Partition full! - Erasing Partition");
        if (erase_lwpr_partition()){
            printf("lpwr: Error erasing %s\n", LPWR_PARTITION);
            ret = errno;
            mainloop = WFALSE; /* Signal thread we are done */
            goto lpwr_exit;
        }
        curr_page = 0;
    }
    if(error_states) {
        printf("lpwr: state(0x%x)\n",error_states);
    }

lpwr_cont:

    /* Clear buffers */
    init_statics(); /* Whatever happened above is in log already so blow away. */
    lwpr_info_256->reset_cause = SOFT_REBOOT; /* Default */

    /* create rsudiag thread to poll for temps and dcin */
    if (0 == (ret = pthread_create(&rsudiag_threadID, NULL, (void *) &rsudiag_thread, NULL))) {
        pthread_detach(rsudiag_threadID);
    } else {
        printf("lpwr: failed to start RSUDIAG thread: ret(%d) errno(%d)\n",ret, errno);
        /* Not fatal, keep going. Just no temps or dcin stats */ 
    }

    /* Mark page in partition map as current working page */
    mark_page(curr_page);
    check_file_integrity();

    /* 
     * ==== Register with lpwr interrupt by doing a read ====
     */
    byte_read = read(fd_int, tmp_buf, 10);      /* Blocked till the interrupt send 10 bytes of data */
    mainloop = 0; /* Signal thread, may not exit in time, oh well */
    byte_read = byte_read; /* Silence warning, next stop reboot so doesn't matter the value read */
//TODO: soft reset(reboot) does not get here. No information preserved in partition for subsequent retrival */
    lwpr_info_256->reset_cause = PWR_LOSS;

    /* save lpwr info */
    ret = save_lpwr_info(curr_page);

lpwr_exit: /* Return values are not checked at ths point. It doesn't matter now, */
     /* signal thread to exit. */
    mainloop = 0;
    /* close lpwr */
    close(fd_int);

#if EXTRA_DEBUG
    printf("lpwr: %s closed: state(0x%x): Sync all data cache.\n", LPWR_INT,error_state);
#endif

#if !defined(MY_UNIT_TEST)
    system("sync");
    sleep(SLEEP_TIMER);
    #if EXTRA_DEBUG
    printf("lpwr: Shutting down!\n");
    #endif
    system("shutdown now");
#endif
    return ret;
}