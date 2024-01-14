/**************************************************************************
 *                                                                        *
 *     File Name:  ntcip-1218.c                                           *
 *     Author:                                                            *
 *         DENSO International America, Inc.                              *
 *         North America Research Laboratory, California Office           *
 *         Rutherford Road                                                *
 *         Carlsbad, 92008                                                *
 **************************************************************************/

/******************************************************************************
* NTCIP-1218 MIB START : From 1218v138.mib 202009040000Z
******************************************************************************/
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <pthread.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <errno.h>
#include <mqueue.h>
#include "rsutable.h"
#include "ntcip-1218.h"
#include "i2v_util.h"
#include "conf_table.h"
#include "rsuhealth.h"
#if defined(MY_UNIT_TEST)
#include "stubs.h"
#endif

/* Mandatory: For debugging and syslog output. */
#define MY_NAME "mib1218"
#define MY_NAME_EXTRA "Xmib1218"

#define MY_ERR_LEVEL   LEVEL_PRIV    /* from i2v_util.h */
#define MY_I2V_NAME    "mib-1218"
/* OID Error States. May need to split per major OID but for now lets see.*/
STATIC uint32_t ntcip_1218_error_states = 0x0;

/* For mutex on MIB. */
#define  MAX_LOCK_WAIT_ITERATIONS  25
#define  MAX_LOCK_WAIT_TIME_USECS  500

/* 
 * This is local NTCIP-1218 MIB database. There's still contents on drive and in SHM potentially.
 * We only check SHM for updates. We do not write to it.
 * We only write this to the file system. 
 * Until reflected in SHM its not a confirmed done transaction.
 */
ntcip_1218_mib_t  ntcip_1218_mib;
STATIC ntcip_1218_mib_t  prior_ntcip_1218_mib; 
STATIC ntcip_1218_mib_t  tmp_ntcip_1218_mib; 

STATIC pthread_mutex_t ntcip1218_mutex = PTHREAD_MUTEX_INITIALIZER;

/* Module Errors. */
void set_ntcip_1218_error_states(int32_t ntcip_1218_error)
{
  int32_t dummy = 0;

    dummy = abs(ntcip_1218_error);

    /* -1 to -31 */
    if((ntcip_1218_error < 0) && (ntcip_1218_error > MIB_ERROR_BASE_2)){
        ntcip_1218_error_states |= (uint32_t)(0x1) << (dummy - 1);
    }
    return;
}
uint32_t get_ntcip_1218_error_states(void)
{
  return ntcip_1218_error_states;
}

/* Database Lock. */
int32_t ntcip1218_lock(void) 
{
  int32_t i;  

  for (i=0; i<MAX_LOCK_WAIT_ITERATIONS; i++){
      if (0 == pthread_mutex_trylock(&ntcip1218_mutex)){
          return RSEMIB_OK;
      } else {
          usleep(MAX_LOCK_WAIT_TIME_USECS);
      }
  }
  return RSEMIB_LOCK_FAIL; /* Calling function will log error state. Otherwise error too general. */
}
void ntcip1218_unlock(void) 
{
  if(0 != pthread_mutex_unlock(&ntcip1218_mutex)){ 
     DEBUGMSGTL((MY_NAME, ": RSEMIB_UNLOCK_FAIL.\n"));
     set_ntcip_1218_error_states(RSEMIB_UNLOCK_FAIL);
  }
  return;
}
/* See snmp.h. This list is not exhaustive. */
int32_t valid_table_row(int32_t data_in)
{
  int32_t ret = RSEMIB_OK;
  switch(data_in){
      case SNMP_ROW_NONEXISTENT: 
          ret = RSEMIB_ROW_EMPTY;
          break;
      case SNMP_ROW_ACTIVE:
      case SNMP_ROW_NOTINSERVICE:
      case SNMP_ROW_NOTREADY:
      case SNMP_ROW_CREATEANDGO:
      case SNMP_ROW_CREATEANDWAIT:
          ret = RSEMIB_OK;
          break;
      case SNMP_ROW_DESTROY:
          ret = RSEMIB_ROW_EMPTY;
          break;
      default:
          ret = RSEMIB_ROW_EMPTY;
          break;
  }
  return ret;
}
/* Copied from i2v_util.c: Will check for existence of a directory and create it if not. */
static int32_t mibUtilMakeDir(char_t *dirname)
{
  FILE *f;
  char_t syscmd[RSU_MSG_FILENAME_LENGTH_MAX + 10]; /* mkdir -p dirname */

  if (NULL == dirname)
      return RSEMIB_BAD_INPUT;

  if ((f=fopen(dirname, "r")) == NULL) {
      strcpy(syscmd, "mkdir -p ");
      strcat(syscmd, dirname);
      return (system(syscmd) != -1) ? RSEMIB_OK : RSEMIB_SYSCALL_FAIL;
  } else {
      fclose(f);
  }
  return RSEMIB_OK;
}
static int32_t check_file_existence(const char_t *filename, uint32_t filesize)
{
  struct stat buffer;

  /* Does file exits? */
  if(0 != stat(filename, &buffer)){
      /*  Doesn't exist. */
      DEBUGMSGTL((MY_NAME_EXTRA,"Not found(%s).\n", filename));
      return RSEMIB_FILE_STAT_FAIL; /* This may or may not be harmless in given context called. */
  }
  /* Does file size match what we are looking for? */
  if ((0 != filesize) && (filesize != buffer.st_size)){
      DEBUGMSGTL((MY_NAME, "Found(%s) but size mismatch(%ld!=%d): Rejecting.\n", filename, buffer.st_size, filesize));
      return RSEMIB_BAD_DATA;
  }
  DEBUGMSGTL((MY_NAME, "Found(%s) and size match(%ld==%d): Success.\n", filename, buffer.st_size, filesize));
  return RSEMIB_OK;
}
static uint16_t crc16(const uint8_t * data_p, size_t length)
{
    uint8_t x;
    uint16_t crc = 0xFFFF;

    while (length--){
        x = crc >> 8 ^ *data_p++;
        x ^= x>>4;
        crc = (crc << 8) ^ ((uint16_t)(x << 12)) ^ ((uint16_t)(x <<5)) ^ ((uint16_t)x);
    }
    return crc;
}

/* Dupe from amh.c
 * All functions using DateAndTime need to verify against this call. Return negative if bogus: 
 *
 * from ntcip-1218.h: MIB_DATEANDTIME_LENGTH: "2d-1d-1d,1d:1d:1d.1d": 8 OCTETS: DateAndTime: SNMPv2-TC
 *
 * from SNMPv2-TC.txt: A date-time specification.
 *
 *        field  octets  contents                  range
 *        -----  ------  --------                  -----
 *          1      1-2   year*                     0..65536
 *          2       3    month                     1..12
 *          3       4    day                       1..31
 *          4       5    hour                      0..23
 *          5       6    minutes                   0..59
 *          6       7    seconds                   0..60
 *                       (use 60 for leap-second)
 *          7       8    deci-seconds              0..9
 *
 *  Not supported, ignore.
 *  
 *          8       9    direction from UTC        '+' / '-'
 *          9      10    hours from UTC*           0..13
 *         10      11    minutes from UTC          0..59
 *
 *        Notes:
 *        - the value of year is in network-byte order
 *        - daylight saving time in New Zealand is +13
 *
 *   Tuesday May 26, 1992 at 1:30:15 PM EDT == 1992-5-26,13:30:15.0,-4:0
 */
static int32_t DateAndTime_To_UTC_DSEC(uint8_t * DateAndTime, uint64_t * utcDsec)
{
  int32_t year,month,day,hour,minute,second,dsec;
  uint64_t testDsec;
  struct tm mib_tm;

  year   = (int32_t)DateAndTime[0]; /* Year */
  year   = year << 8;
  year  += (int32_t)DateAndTime[1]; 
  month  = (int32_t)DateAndTime[2]; /* Month */
  day    = (int32_t)DateAndTime[3]; /* Day */
  hour   = (int32_t)DateAndTime[4]; /* Hour */
  minute = (int32_t)DateAndTime[5]; /* Minute */
  second = (int32_t)DateAndTime[6]; /* Second */
  dsec   = (int32_t)DateAndTime[7]; /* Dsecond */

  if((year < 0) || (65536 < year)){ /* Basic range check: */
      DEBUGMSGTL((MY_NAME, "DateAndTime_To_UTC_DSEC: Bad year.\n"));
      return RSEMIB_BAD_DATA;       /* From unsigned above so can't be less than zero really. */
  }
  if(1900 <= year){
      year = year - 1900; /* Adjust for mktime(). */
  }
  else {
      year = 0; /* Because year can legally be less than 1900, I truncate like this. UTC only to 1970 so too bad. */
  }
  if((month < 1) || (12 < month)){ /* Not exhaustive for month & day. See below for more. */
      DEBUGMSGTL((MY_NAME, "DateAndTime_To_UTC_DSEC: Bad month.\n"));
      return RSEMIB_BAD_DATA;
  }
  month = month - 1; /* adjust for mktime(). */
  if((day < 1) || (31 < day)){
      DEBUGMSGTL((MY_NAME, "DateAndTime_To_UTC_DSEC: Bad day.\n"));
      return RSEMIB_BAD_DATA;
  }
  if((hour < 0) || (23 < hour)){
      DEBUGMSGTL((MY_NAME, "DateAndTime_To_UTC_DSEC: Bad hour.\n"));
      return RSEMIB_BAD_DATA;
  }
  if((minute < 0) || (59 < minute)){
      DEBUGMSGTL((MY_NAME, "DateAndTime_To_UTC_DSEC: Bad minute.\n"));
      return RSEMIB_BAD_DATA;
  }
  if((second < 0) || (60 < second)){
      DEBUGMSGTL((MY_NAME, "DateAndTime_To_UTC_DSEC: Bad second.\n"));
      return RSEMIB_BAD_DATA;
  }
  if((dsec < 0) || (9 < dsec)){
      DEBUGMSGTL((MY_NAME, "DateAndTime_To_UTC_DSEC: Bad dsec.\n"));
      return RSEMIB_BAD_DATA;
  }

  mib_tm.tm_sec   = second;
  mib_tm.tm_min   = minute;
  mib_tm.tm_hour  = hour;
  mib_tm.tm_mday  = day;
  mib_tm.tm_mon   = month;
  mib_tm.tm_year  = year;
  mib_tm.tm_wday  = 0;
  mib_tm.tm_yday  = 0;
  mib_tm.tm_isdst = 0;

  /* Assumming our error checking above is correct this can't fail. */
  testDsec = (uint64_t)mktime(&mib_tm);
  if ((time_t)-1 == (time_t)testDsec){
      DEBUGMSGTL((MY_NAME, "DateAndTime_To_UTC_DSEC: mktime() failed.\n"));
      return RSEMIB_BAD_DATA;
  }
  testDsec *= (uint64_t)10;   /* Convert to deci-seconds and add final value. */
  testDsec += (uint64_t)dsec;
  if (NULL != utcDsec) {
      *utcDsec = testDsec;
  }
  return RSEMIB_OK;
}
static uint64_t mibUtilGetTimeInMs(void)
{
  struct timeval tv;
  uint64_t timenow;

  gettimeofday(&tv, NULL);
  timenow = tv.tv_sec;
  timenow *= 1000;
  timenow += tv.tv_usec/1000;
  return timenow;
}
/******************************************************************************
* NTCIP-1218 MIB Init Functions for each OID. 
*     NOTE: Default empty state must not cause errors!
*         : By definition init to welformed state.
*         : Assumes locked mutex. If you don't then you are on your own.
******************************************************************************/

/* 5.2.1 - 5.2.2 */
static void init_rsuRadioTable(void)
{
  ntcip_1218_mib.maxRsuRadios = RSU_RADIOS_MAX;

#if defined(PLATFORM_HD_RSU_5940)
  ntcip_1218_mib.rsuRadioTable[0].rsuRadioIndex = 1;
  strncpy((char_t *)ntcip_1218_mib.rsuRadioTable[0].rsuRadioDesc,"DENSO CV2X", RSU_RADIO_DESC_MAX);

  ntcip_1218_mib.rsuRadioTable[0].rsuRadioDesc_length = 10; /* Do not include NULL. */
  ntcip_1218_mib.rsuRadioTable[0].rsuRadioEnable = mib_on; /* There is only one radio. Assume on till conf say otherwise. */
  ntcip_1218_mib.rsuRadioTable[0].rsuRadioType = RSU_RADIO_TYPE_PC3;

  memset(ntcip_1218_mib.rsuRadioTable[0].rsuRadioMacAddress1,0xFF,RSU_RADIO_MAC_LENGTH); 
  memset(ntcip_1218_mib.rsuRadioTable[0].rsuRadioMacAddress2,0xFF,RSU_RADIO_MAC_LENGTH);  

  ntcip_1218_mib.rsuRadioTable[0].rsuRadioChanMode = RSU_RADIO_TYPE_MODE_CONT; /* Today we only support CONT. */
  ntcip_1218_mib.rsuRadioTable[0].rsuRadioCh1 = RSU_RADIO_CHANNEL_MIN;                               
  ntcip_1218_mib.rsuRadioTable[0].rsuRadioCh2 = RSU_RADIO_CHANNEL_MIN;
  ntcip_1218_mib.rsuRadioTable[0].rsuRadioTxPower1 = RSU_RADIO_TX_POWER_MAX;
  ntcip_1218_mib.rsuRadioTable[0].rsuRadioTxPower2 = RSU_RADIO_TX_POWER_MAX;

#else

  ntcip_1218_mib.rsuRadioTable[0].rsuRadioIndex = 1;
  ntcip_1218_mib.rsuRadioTable[1].rsuRadioIndex = 2;
  strncpy((char_t *)ntcip_1218_mib.rsuRadioTable[0].rsuRadioDesc,"DENSO DSRC",RSU_RADIO_DESC_MAX);
  strncpy((char_t *)ntcip_1218_mib.rsuRadioTable[1].rsuRadioDesc,"DENSO CV2X",RSU_RADIO_DESC_MAX);

  ntcip_1218_mib.rsuRadioTable[0].rsuRadioDesc_length = 10; /* Do not include NULL. */
  ntcip_1218_mib.rsuRadioTable[1].rsuRadioDesc_length = 10;

  ntcip_1218_mib.rsuRadioTable[0].rsuRadioEnable = mib_off;
  ntcip_1218_mib.rsuRadioTable[1].rsuRadioEnable = mib_off;

  ntcip_1218_mib.rsuRadioTable[0].rsuRadioType = RSU_RADIO_TYPE_DSRC;
  ntcip_1218_mib.rsuRadioTable[1].rsuRadioType = RSU_RADIO_TYPE_PC3;

  memset(ntcip_1218_mib.rsuRadioTable[0].rsuRadioMacAddress1,0xFF,RSU_RADIO_MAC_LENGTH); 
  memset(ntcip_1218_mib.rsuRadioTable[1].rsuRadioMacAddress1,0xFF,RSU_RADIO_MAC_LENGTH); 

  memset(ntcip_1218_mib.rsuRadioTable[0].rsuRadioMacAddress2,0xFF,RSU_RADIO_MAC_LENGTH); 
  memset(ntcip_1218_mib.rsuRadioTable[1].rsuRadioMacAddress2,0xFF,RSU_RADIO_MAC_LENGTH); 

  ntcip_1218_mib.rsuRadioTable[0].rsuRadioChanMode = RSU_RADIO_TYPE_MODE_CONT; /* Today we only support CONT. */
  ntcip_1218_mib.rsuRadioTable[1].rsuRadioChanMode = RSU_RADIO_TYPE_MODE_CONT;

  ntcip_1218_mib.rsuRadioTable[0].rsuRadioCh1 = RSU_RADIO_CHANNEL_MIN;                               
  ntcip_1218_mib.rsuRadioTable[0].rsuRadioCh2 = RSU_RADIO_CHANNEL_MIN;

  ntcip_1218_mib.rsuRadioTable[1].rsuRadioCh1 = RSU_RADIO_CHANNEL_MIN;                               
  ntcip_1218_mib.rsuRadioTable[1].rsuRadioCh2 = RSU_RADIO_CHANNEL_MIN;

  ntcip_1218_mib.rsuRadioTable[0].rsuRadioTxPower1 = RSU_RADIO_TX_POWER_MIN;
  ntcip_1218_mib.rsuRadioTable[0].rsuRadioTxPower2 = RSU_RADIO_TX_POWER_MIN;

  ntcip_1218_mib.rsuRadioTable[1].rsuRadioTxPower1 = RSU_RADIO_TX_POWER_MIN;
  ntcip_1218_mib.rsuRadioTable[1].rsuRadioTxPower2 = RSU_RADIO_TX_POWER_MIN;
#endif /* PLATFORM_HD_RSU_5940 */
}

/* 5.3.1 - 5.3.2 */
static void init_rsuGnssStatus(void)
{
  ntcip_1218_mib.rsuGnssStatus       = 0;
  ntcip_1218_mib.rsuGnssAugmentation = rsuGnssAugmentation_none;
}

/* 5.4.1 - 5.4.3 */
void set_default_row_rsuMsgRepeat(RsuMsgRepeatStatusEntry_t * rsuMsgRepeat, int32_t index)
{
  char_t command_buffer[RSU_MSG_FILENAME_LENGTH_MAX];
  index++;
  if((NULL == rsuMsgRepeat) || (index < RSU_SAR_MESSAGE_MIN) || (RSU_SAR_MESSAGE_MAX < index)){
      return;
  }
  rsuMsgRepeat->rsuMsgRepeatIndex = index; /* 1..maxMsgRepeat. */
  memset(&rsuMsgRepeat->rsuMsgRepeatPsid,0x0,RSU_RADIO_PSID_SIZE);
  rsuMsgRepeat->rsuMsgRepeatPsid_length = RSU_RADIO_PSID_MIN_SIZE;

  rsuMsgRepeat->rsuMsgRepeatTxChannel  = RSU_RADIO_CHANNEL_MIN;
  rsuMsgRepeat->rsuMsgRepeatTxInterval = RSU_MSG_REPEAT_TX_INTERVAL_DEFAULT;
  memset(&rsuMsgRepeat->rsuMsgRepeatDeliveryStart,0x0,MIB_DATEANDTIME_LENGTH);
  memset(&rsuMsgRepeat->rsuMsgRepeatDeliveryStop,0x0,MIB_DATEANDTIME_LENGTH);

  memset(&rsuMsgRepeat->rsuMsgRepeatPayload,0x0,RSU_MSG_PAYLOAD_MAX);
  rsuMsgRepeat->rsuMsgRepeatPayload_length = 0;

  rsuMsgRepeat->rsuMsgRepeatEnable   = mib_on; 
  rsuMsgRepeat->rsuMsgRepeatStatus   = ROW_STATUS_VALID_DEFAULT;
  rsuMsgRepeat->rsuMsgRepeatPriority = RSU_MSG_REPEAT_PRIORITY_DEFAULT;
  rsuMsgRepeat->rsuMsgRepeatOptions  = RSU_MSG_REPEAT_OPTIONS_DEFAULT;

  rsuMsgRepeat->version = MSG_REPEAT_VERSION;
  rsuMsgRepeat->status  = ROW_STATUS_VALID_DEFAULT;
  rsuMsgRepeat->reserved = 0x0;
  rsuMsgRepeat->crc = 0x0;

#if defined(PLATFORM_HD_RSU_5940)
  rsuMsgRepeat->rsuMsgRepeatTxChannel = ntcip_1218_mib.rsuRadioTable[RSU_CV2X_RADIO-1].rsuRadioCh1; /* CV2X. */
#else
  /* Depending on which radio is enabled, seed with current channel used. CONT mode only with single channel */
  if(mib_on == ntcip_1218_mib.rsuRadioTable[0].rsuRadioEnable) {
      rsuMsgRepeat->rsuMsgRepeatTxChannel = ntcip_1218_mib.rsuRadioTable[0].rsuRadioCh1; /* DSRC. */
  } else {
      rsuMsgRepeat->rsuMsgRepeatTxChannel = ntcip_1218_mib.rsuRadioTable[1].rsuRadioCh1; /* CV2X. */
  }
#endif

  /* Assume MAP psid by default. */
  rsuMsgRepeat->rsuMsgRepeatPsid[0] = 0x0;
  rsuMsgRepeat->rsuMsgRepeatPsid[1] = 0x0;
  rsuMsgRepeat->rsuMsgRepeatPsid[2] = 0x80;
  rsuMsgRepeat->rsuMsgRepeatPsid[3] = 0x02;
  rsuMsgRepeat->rsuMsgRepeatPsid_length = 4;

  /*
     default start:
       07e6-01-01:00:00:00.1
       2022-01-01:00:00:00.1
 
     default stop:
       0833-0c-1f-17:3b:3b.9
       2099-12-31:23:59:59.9
  */
  rsuMsgRepeat->rsuMsgRepeatDeliveryStart[0] = 0x07;
  rsuMsgRepeat->rsuMsgRepeatDeliveryStart[1] = 0xe6;
  rsuMsgRepeat->rsuMsgRepeatDeliveryStart[2] = 0x01;
  rsuMsgRepeat->rsuMsgRepeatDeliveryStart[3] = 0x01;
  rsuMsgRepeat->rsuMsgRepeatDeliveryStart[4] = 0x00;
  rsuMsgRepeat->rsuMsgRepeatDeliveryStart[5] = 0x00;
  rsuMsgRepeat->rsuMsgRepeatDeliveryStart[6] = 0x00;
  rsuMsgRepeat->rsuMsgRepeatDeliveryStart[7] = 0x01;

  rsuMsgRepeat->rsuMsgRepeatDeliveryStop[0] = 0x08;
  rsuMsgRepeat->rsuMsgRepeatDeliveryStop[1] = 0x33;
  rsuMsgRepeat->rsuMsgRepeatDeliveryStop[2] = 0x0c;
  rsuMsgRepeat->rsuMsgRepeatDeliveryStop[3] = 0x1f;
  rsuMsgRepeat->rsuMsgRepeatDeliveryStop[4] = 0x17;
  rsuMsgRepeat->rsuMsgRepeatDeliveryStop[5] = 0x3b;
  rsuMsgRepeat->rsuMsgRepeatDeliveryStop[6] = 0x3b;
  rsuMsgRepeat->rsuMsgRepeatDeliveryStop[7] = 0x09;

  memset(command_buffer,'\0',sizeof(command_buffer));
  sprintf(command_buffer,"%s/%d.dat",AMH_ACTIVE_DIR, rsuMsgRepeat->rsuMsgRepeatIndex); /* Give row a file name based off index. Binary file so name it so. */
  memcpy(&rsuMsgRepeat->filePathName, command_buffer, RSU_MSG_FILENAME_LENGTH_MAX);
}

static void restore_msgRepeat_from_disk(void)
{
  FILE    * file_in        = NULL;
  uint32_t  mib_byte_count = 0;
  int32_t   ret            = RSEMIB_OK; /* SUCCESS till proven false. */
  RsuMsgRepeatStatusEntry_t msgRepeat;
  char_t   filePathName[RSU_MSG_FILENAME_LENGTH_MAX];
  int32_t  i;

  mib_byte_count = sizeof(msgRepeat);
  memset(filePathName,'\0',RSU_MSG_FILENAME_LENGTH_MAX);
  for(i=0; i < RSU_SAR_MESSAGE_MAX; i++) {
      /* msgRepeat file name always based off index. */
      snprintf(filePathName,RSU_MSG_FILENAME_LENGTH_MAX, "%s/%d.dat",AMH_ACTIVE_DIR, i + 1);
      /* It's ok for the file to not exist. */
      if(RSEMIB_OK == check_file_existence(filePathName, mib_byte_count)){
          /* Open file for input.*/
          if ((RSEMIB_OK == ret) && (NULL == (file_in = fopen(filePathName, "rb"))) ){
              DEBUGMSGTL((MY_NAME, "restore_msgRepeat_from_disk: fopen fail.\n"));
              ret = RSEMIB_FOPEN_FAIL;
          }
          memset(&msgRepeat,0x0,sizeof(msgRepeat));
          /* Load file into tmp ram space first before overwriting. */
          if((RSEMIB_OK == ret) && (1 != fread((uint8_t *)&msgRepeat, mib_byte_count, 1, file_in)) ){    
              DEBUGMSGTL((MY_NAME, "restore_msgRepeat_from_disk: FAIL: read_count is 0: mib_byte_count=(0x%X) errno=(%d:%s).\n",mib_byte_count,errno,strerror(errno) ));
              ret = RSEMIB_BAD_DATA;     
          }
          /* One way or another done with file so close. */
          if(NULL != file_in){
              fclose(file_in);
          }
          /* Check wellness and copy over if good.*/
          if(RSEMIB_OK == ret){
              if(   (msgRepeat.crc == (uint32_t)crc16((const uint8_t *)&msgRepeat, sizeof(msgRepeat) - 4))
                 && (MSG_REPEAT_VERSION == msgRepeat.version)) { 
                  memcpy(&ntcip_1218_mib.rsuMsgRepeatStatusTable[i],&msgRepeat, sizeof(msgRepeat));
              } else {
                  DEBUGMSGTL((MY_NAME, "restore_MSG_REPEAT_from_disk: index=%d failed wellness check.\n",i));
                  ret = RSEMIB_BAD_RECORD_RESTORE; /* File will be over written next time row is used by client. Leave in place for now.*/
              }
          }
      }
  }
  if(RSEMIB_OK != ret){
      set_ntcip_1218_error_states(ret);
  }
}
static void init_rsuMsgRepeat(void)
{
  int32_t i = 0;

  ntcip_1218_mib.maxRsuMsgRepeat       = RSU_SAR_MESSAGE_MAX;
  ntcip_1218_mib.rsuMsgRepeatDeleteAll = 0;

  for(i=0; i < RSU_SAR_MESSAGE_MAX; i++){
      set_default_row_rsuMsgRepeat(&ntcip_1218_mib.rsuMsgRepeatStatusTable[i],i);
      set_default_row_rsuMsgRepeat(&prior_ntcip_1218_mib.rsuMsgRepeatStatusTable[i],i);
      set_default_row_rsuMsgRepeat(&tmp_ntcip_1218_mib.rsuMsgRepeatStatusTable[i],i);
      DEBUGMSGTL((MY_NAME_EXTRA, "init_rsuMsgRepeat: index=%d, fileName=[%s].\n",i,ntcip_1218_mib.rsuMsgRepeatStatusTable[i].filePathName));
  }
  /* If it doesn't exist then create it. If we fail to create keep going. */
  if(RSEMIB_OK != mibUtilMakeDir(AMH_ACTIVE_DIR)) {
      set_ntcip_1218_error_states(RSEMIB_CREATE_DIR_FAIL);
      DEBUGMSGTL((MY_NAME, "init_rsuMsgRepeat: Couldn't create AMH dir[%s]\n",AMH_ACTIVE_DIR));
  } else { /* Restore msgRepeats from disk. If it fails keep going. */
      restore_msgRepeat_from_disk();
  }
}

/* 5.5.1 - 5.5.2 */
void set_default_row_rsuIFM(RsuIFMStatusEntry_t * rsuIFM, int32_t index)
{
  char_t command_buffer[RSU_MSG_FILENAME_LENGTH_MAX];
  index++;
  if((NULL == rsuIFM) || (index < RSU_IFM_MESSAGE_MIN) || (RSU_IFM_MESSAGE_MAX < index)){
      return;
  }
  rsuIFM->rsuIFMIndex = index; /* 1..maxIFM. */
  memset(&rsuIFM->rsuIFMPsid,0x0, RSU_RADIO_PSID_SIZE);
  rsuIFM->rsuIFMPsid_length = RSU_IFM_MESSAGE_MIN;

  rsuIFM->rsuIFMTxChannel = RSU_RADIO_CHANNEL_MIN;
  rsuIFM->rsuIFMEnable    = mib_on;
  rsuIFM->rsuIFMStatus    = ROW_STATUS_VALID_DEFAULT;
  rsuIFM->rsuIFMPriority  = RSU_IFM_PRIORITY_DEFAULT;
  rsuIFM->rsuIFMOptions   = RSU_IFM_OPTIONS_DEFAULT;

  memset(&rsuIFM->rsuIFMPayload,0x0,RSU_MSG_PAYLOAD_MAX);
  rsuIFM->rsuIFMPayload_length = 0;
#if defined(PLATFORM_HD_RSU_5940)
  rsuIFM->rsuIFMTxChannel = ntcip_1218_mib.rsuRadioTable[RSU_CV2X_RADIO-1].rsuRadioCh1; /* CV2X. */
#else
  /* Depending on which radio is enabled, seed with current channel used. CONT mode only with single channel */
  if(mib_on == ntcip_1218_mib.rsuRadioTable[0].rsuRadioEnable) {
      rsuIFM->rsuIFMTxChannel = ntcip_1218_mib.rsuRadioTable[0].rsuRadioCh1; /* DSRC. */
  } else {
      rsuIFM->rsuIFMTxChannel = ntcip_1218_mib.rsuRadioTable[1].rsuRadioCh1; /* CV2X. */
  }
#endif
  /* Assume SPAT psid by default. */
  rsuIFM->rsuIFMPsid[0] = 0x0;
  rsuIFM->rsuIFMPsid[1] = 0x0;
  rsuIFM->rsuIFMPsid[2] = 0x80;
  rsuIFM->rsuIFMPsid[3] = 0x02;
  rsuIFM->rsuIFMPsid_length = 4;
  rsuIFM->version = IFM_VERSION;
  rsuIFM->reserved = 0x0;
  rsuIFM->crc = 0x0;

  memset(command_buffer,'\0',sizeof(command_buffer));
  sprintf(command_buffer,"%s/%d.dat",IFM_ACTIVE_DIR, rsuIFM->rsuIFMIndex); /* Give row a file name based off index. Binary file so name it so. */
  memcpy(&rsuIFM->filePathName, command_buffer, RSU_MSG_FILENAME_LENGTH_MAX);
}
static void restore_IFM_from_disk(void)
{
  FILE    * file_in        = NULL;
  uint32_t  mib_byte_count = 0;
  int32_t   ret            = RSEMIB_OK; /* SUCCESS till proven false. */
  IFMSkinny_t skinny_ifm;
  char_t   filePathName[RSU_MSG_FILENAME_LENGTH_MAX];
  int32_t  i;

  mib_byte_count = sizeof(skinny_ifm);
  memset(filePathName,'\0',RSU_MSG_FILENAME_LENGTH_MAX);

  for(i=0; i < RSU_IFM_MESSAGE_MAX; i++) {
      /* IFM file name always based off index. */
      snprintf(filePathName,RSU_MSG_FILENAME_LENGTH_MAX, "%s/%d.dat",IFM_ACTIVE_DIR, i + 1);
      /* It's Ok for files to not exist. */
      if(RSEMIB_OK == check_file_existence(filePathName, mib_byte_count)){
          /* Open file for input.*/
          if ((RSEMIB_OK == ret) && (NULL == (file_in = fopen(filePathName, "rb"))) ){
              DEBUGMSGTL((MY_NAME, "restore_IFM_from_disk: fopen fail.\n"));
              ret = RSEMIB_FOPEN_FAIL;
          }
          memset(&skinny_ifm,0x0,sizeof(skinny_ifm));
          /* Load file into tmp ram space first before overwriting. */
          if((RSEMIB_OK == ret) && (1 != fread((uint8_t *)&skinny_ifm, mib_byte_count, 1, file_in)) ){    
              DEBUGMSGTL((MY_NAME, "restore_IFM_from_disk: FAIL: read_count is 0: mib_byte_count=(0x%X) errno=(%d:%s).\n",mib_byte_count,errno,strerror(errno) ));
              ret = RSEMIB_BAD_DATA;     
          }
          /* One way or another done with file so close. */
          if(NULL != file_in){
              fclose(file_in);
          }

          /* Check wellness and copy over if good.*/
          if(RSEMIB_OK == ret){

              if(   (skinny_ifm.crc == (uint32_t)crc16((const uint8_t *)&skinny_ifm, sizeof(skinny_ifm) - 4))
                 && (IFM_VERSION == skinny_ifm.version)) { 

                  ntcip_1218_mib.rsuIFMStatusTable[i].rsuIFMPsid[0]     = skinny_ifm.rsuIFMPsid[0];
                  ntcip_1218_mib.rsuIFMStatusTable[i].rsuIFMPsid[1]     = skinny_ifm.rsuIFMPsid[1];
                  ntcip_1218_mib.rsuIFMStatusTable[i].rsuIFMPsid[2]     = skinny_ifm.rsuIFMPsid[2];
                  ntcip_1218_mib.rsuIFMStatusTable[i].rsuIFMPsid[3]     = skinny_ifm.rsuIFMPsid[3];
                  ntcip_1218_mib.rsuIFMStatusTable[i].rsuIFMPsid_length = skinny_ifm.rsuIFMPsid_length; 
                  ntcip_1218_mib.rsuIFMStatusTable[i].rsuIFMTxChannel   = skinny_ifm.rsuIFMTxChannel;
                  ntcip_1218_mib.rsuIFMStatusTable[i].rsuIFMEnable      = skinny_ifm.rsuIFMEnable;
                  ntcip_1218_mib.rsuIFMStatusTable[i].rsuIFMStatus      = skinny_ifm.rsuIFMStatus;
                  ntcip_1218_mib.rsuIFMStatusTable[i].rsuIFMPriority    = skinny_ifm.rsuIFMPriority;
                  ntcip_1218_mib.rsuIFMStatusTable[i].rsuIFMOptions     = skinny_ifm.rsuIFMOptions;
                  ntcip_1218_mib.rsuIFMStatusTable[i].version           = skinny_ifm.version;
                  ntcip_1218_mib.rsuIFMStatusTable[i].reserved          = skinny_ifm.reserved;
              } else {
                  DEBUGMSGTL((MY_NAME, "restore_IFM_from_disk: index=%d failed wellness check.\n",i));
                  ret = RSEMIB_BAD_RECORD_RESTORE; /* File will be over written next time row is used by client. Leave in place for now.*/
              }
          }
      }
  }
  if(RSEMIB_OK != ret){
      set_ntcip_1218_error_states(ret);
  }
}
static void init_rsuIFM(void)
{
  int32_t i = 0;

  ntcip_1218_mib.maxRsuIFMs = RSU_IFM_MESSAGE_MAX;
  for(i=0; i < RSU_IFM_MESSAGE_MAX; i++){
      set_default_row_rsuIFM(&ntcip_1218_mib.rsuIFMStatusTable[i], i);
      set_default_row_rsuIFM(&prior_ntcip_1218_mib.rsuIFMStatusTable[i], i);
      set_default_row_rsuIFM(&tmp_ntcip_1218_mib.rsuIFMStatusTable[i], i);
  }
  /* If it doesn't exist then create it. If we fail to create keep going. */
  if (RSEMIB_OK != mibUtilMakeDir(IFM_ACTIVE_DIR)) {
      set_ntcip_1218_error_states(RSEMIB_CREATE_DIR_FAIL);
      DEBUGMSGTL((MY_NAME, "init_rsuIFM: Couldn't create IFM dir[%s]\n",IFM_ACTIVE_DIR));
  } else { /* Restore IFMs from disk. If it fails keep going. */
      restore_IFM_from_disk();
  }
}
static int32_t validate_and_send_rsuIFM(RsuIFMStatusEntry_t * rsuIFM)
{
  int32_t ret = RSEMIB_OK;
  uint32_t psid = 0x0;

  if(NULL == rsuIFM){
      DEBUGMSGTL((MY_NAME, "validate_and_send_rsuIFM: bad input.\n"));
      ret = RSEMIB_BAD_INPUT;
  } else {
      if((rsuIFM->rsuIFMPsid_length < RSU_RADIO_PSID_MIN_SIZE) || (RSU_RADIO_PSID_SIZE < rsuIFM->rsuIFMPsid_length)){
          DEBUGMSGTL((MY_NAME, "validate_and_send_rsuIFM: bad rsuIFMPsid_length = %d.\n",rsuIFM->rsuIFMPsid_length));
          rsuIFM->rsuIFMPsid_length = RSU_RADIO_PSID_MIN_SIZE; /* Not acessible by client so internal error. */
          ret = RSEMIB_BAD_DATA;
      } else {
          psid  = ((uint32_t)(rsuIFM->rsuIFMPsid[0])) << 8;
          psid |= (uint32_t)rsuIFM->rsuIFMPsid[1];
          psid  = psid << 8;
          psid |= (uint32_t)rsuIFM->rsuIFMPsid[2];
          psid  = psid << 8;
          psid |= (uint32_t)rsuIFM->rsuIFMPsid[3];
          /* We can probably use just one? From ris.c: Validate PSID. */
          if(psid <= (uint32_t) PSID_1BYTE_MAX_VALUE){ 
          } else if(psid >= (uint32_t)PSID_2BYTE_MIN_VALUE && psid <= (uint32_t)PSID_2BYTE_MAX_VALUE){
          } else if(psid >= (uint32_t)PSID_3BYTE_MIN_VALUE && psid <= (uint32_t)PSID_3BYTE_MAX_VALUE){
          } else if(psid >= (uint32_t)PSID_4BYTE_MIN_VALUE && psid <= (uint32_t)PSID_4BYTE_MAX_VALUE){
          } else {
              DEBUGMSGTL((MY_NAME, "validate_and_send_rsuIFM: bad PSID=0x%X.\n", psid));
              ret = RSEMIB_BAD_DATA;
          }
      }
      if((rsuIFM->rsuIFMTxChannel < RSU_RADIO_CHANNEL_MIN) || (RSU_RADIO_CHANNEL_MAX < rsuIFM->rsuIFMTxChannel)){
          DEBUGMSGTL((MY_NAME, "validate_and_send_rsuIFM: bad rsuIFMTxChannel) = %d.\n",rsuIFM->rsuIFMTxChannel));
          rsuIFM->rsuIFMTxChannel = RSU_RADIO_CHANNEL_MIN;
          ret = RSEMIB_BAD_DATA;
      }
#if defined(PLATFORM_HD_RSU_5940)
      if(mib_on == ntcip_1218_mib.rsuRadioTable[RSU_CV2X_RADIO-1].rsuRadioEnable) {
          if(rsuIFM->rsuIFMTxChannel != ntcip_1218_mib.rsuRadioTable[RSU_CV2X_RADIO-1].rsuRadioCh1){ /* CV2X. */
              DEBUGMSGTL((MY_NAME, "validate_and_send_rsuIFM: bad CV2x TxChannel=%d wanted=%d.\n",rsuIFM->rsuIFMTxChannel,ntcip_1218_mib.rsuRadioTable[RSU_CV2X_RADIO-1].rsuRadioCh1));
              ret = RSEMIB_BAD_DATA;
          }
      } else {
          DEBUGMSGTL((MY_NAME, "validate_and_send_rsuIFM: CV2X Radio is off.\n"));
          ret = RSEMIB_BAD_DATA;
      }
#else
      if(mib_on == ntcip_1218_mib.rsuRadioTable[0].rsuRadioEnable) {
          if(rsuIFM->rsuIFMTxChannel != ntcip_1218_mib.rsuRadioTable[0].rsuRadioCh1){ /* DSRC. */
              DEBUGMSGTL((MY_NAME, "validate_and_send_rsuIFM: bad DSRC TxChannel=%d wanted=%d.\n",rsuIFM->rsuIFMTxChannel,ntcip_1218_mib.rsuRadioTable[0].rsuRadioCh1));
              ret = RSEMIB_BAD_DATA;
          }
      } else {
          if(mib_on == ntcip_1218_mib.rsuRadioTable[1].rsuRadioEnable) {
              if(rsuIFM->rsuIFMTxChannel != ntcip_1218_mib.rsuRadioTable[1].rsuRadioCh1){ /* CV2X. */
                  DEBUGMSGTL((MY_NAME, "validate_and_send_rsuIFM: bad CV2x TxChannel=%d wanted=%d.\n",rsuIFM->rsuIFMTxChannel,ntcip_1218_mib.rsuRadioTable[1].rsuRadioCh1));
                  ret = RSEMIB_BAD_DATA;
              }
          } else {
              DEBUGMSGTL((MY_NAME, "validate_and_send_rsuIFM: Neither Radio is on.\n"));
              ret = RSEMIB_BAD_DATA;
          }
      }
#endif
      /* rsuIFMPayload: How to check 2.5k binary data? Assuming dsrcMsgId and a few bytes at least. */
      if((rsuIFM->rsuIFMPayload_length < RSU_MSG_PAYLOAD_MIN) || (RSU_MSG_PAYLOAD_MAX < rsuIFM->rsuIFMPayload_length)){
          DEBUGMSGTL((MY_NAME, "validate_and_send_rsuIFM: bad rsuIFMPayload_length = %d.\n",rsuIFM->rsuIFMPayload_length));
          ret = RSEMIB_BAD_DATA;
      } 
#if defined(ENABLE_DSRC_MSG_ID_CHECK) /* NTCIP-1218 does not mention using payload like this. Maybe feature for the future. */
      /* Must be a valid DSRC MSG ID. */
      if(0 == dsrcMsgId_to_amhMsgType(rsuIFM->rsuIFMPayload[1])){
          DEBUGMSGTL((MY_NAME, "validate_and_send_rsuIFM: bad Uper DSRC MSG ID[0x%X]:\n",rsuIFM->rsuIFMPayload[1]));
          ret = RSEMIB_BAD_DATA;
      }
#endif
      if((rsuIFM->rsuIFMEnable < 0) || (1 < rsuIFM->rsuIFMEnable)){
          DEBUGMSGTL((MY_NAME, "validate_and_send_rsuIFM: bad rsuIFMEnable = %d.\n",rsuIFM->rsuIFMEnable));
          ret = RSEMIB_BAD_DATA;
      }
      if((rsuIFM->rsuIFMStatus < SNMP_ROW_NONEXISTENT) || (SNMP_ROW_DESTROY < rsuIFM->rsuIFMStatus)){
          DEBUGMSGTL((MY_NAME, "validate_and_send_rsuIFM: bad rsuIFMStatus = %d.\n",rsuIFM->rsuIFMStatus));
          ret = RSEMIB_BAD_DATA;
      } 
      if((rsuIFM->rsuIFMPriority < RSU_IFM_PRIORITY_MIN) || (RSU_IFM_PRIORITY_MAX < rsuIFM->rsuIFMPriority)){
          DEBUGMSGTL((MY_NAME, "validate_and_send_rsuIFM: bad rsuIFMPriority = %d.\n",rsuIFM->rsuIFMPriority));
          ret = RSEMIB_BAD_DATA;
      }
      if(~(RSU_IFM_OPTIONS_MASK) & rsuIFM->rsuIFMOptions){ /* Erroneous bit check. */
          DEBUGMSGTL((MY_NAME, "validate_and_send_rsuIFM: bad rsuIFMOptions = %d.\n",rsuIFM->rsuIFMOptions));
          ret = RSEMIB_BAD_DATA;
      }
      if(RSEMIB_OK == ret) { /* Good to go, ship it. */
          DEBUGMSGTL((MY_NAME_EXTRA, "validate_and_send_rsuIFM: Success.\n"));
          ret = RSEMIB_OK;
      } else {
          DEBUGMSGTL((MY_NAME, "validate_and_send_rsuIFM: Failed.\n"));
      }
  }
  return ret;
}

/* 5.6.1 - 5.6.2.12 */
static void set_default_row_rsuReceivedMsg(RsuReceivedMsgEntry_t * rsuReceivedMsg, int32_t index)
{
  index++;
  if((NULL == rsuReceivedMsg) || (index < RSU_RX_MSG_MIN) || (RSU_RX_MSG_MAX < index)){
      return;
  }
  rsuReceivedMsg->rsuReceivedMsgIndex = index; /* 1..maxRxMsg. */
  memset(&rsuReceivedMsg->rsuReceivedMsgPsid, 0x0 , RSU_RADIO_PSID_SIZE);
  rsuReceivedMsg->rsuReceivedMsgPsid_length = RSU_RADIO_PSID_MIN_SIZE;

  memset(&rsuReceivedMsg->rsuReceivedMsgDestIpAddr,0x0,RSU_DEST_IP_MAX);
  rsuReceivedMsg->rsuReceivedMsgDestIpAddr_length = 0;

  rsuReceivedMsg->rsuReceivedMsgDestPort = RSU_GNSS_OUTPUT_PORT_DEFAULT;
  rsuReceivedMsg->rsuReceivedMsgProtocol = protocol_udp;
  rsuReceivedMsg->rsuReceivedMsgRssi     = RSU_RX_MSG_RSSI_THRESHOLD_MIN;
  rsuReceivedMsg->rsuReceivedMsgInterval = RSU_RX_MSG_INTERVAL_MIN;

  memset(&rsuReceivedMsg->rsuReceivedMsgDeliveryStart,0x0,MIB_DATEANDTIME_LENGTH);
  memset(&rsuReceivedMsg->rsuReceivedMsgDeliveryStop ,0x0,MIB_DATEANDTIME_LENGTH);
  /*
     default start:
       07e6-01-01:00:00:00.1
       2022-01-01:00:00:00.1
 
     default stop:
       0833-0c-1f-17:3b:3b.9
       2099-12-31:23:59:59.9
  */
  rsuReceivedMsg->rsuReceivedMsgDeliveryStart[0] = 0x07;
  rsuReceivedMsg->rsuReceivedMsgDeliveryStart[1] = 0xe6;
  rsuReceivedMsg->rsuReceivedMsgDeliveryStart[2] = 0x01;
  rsuReceivedMsg->rsuReceivedMsgDeliveryStart[3] = 0x01;
  rsuReceivedMsg->rsuReceivedMsgDeliveryStart[4] = 0x00;
  rsuReceivedMsg->rsuReceivedMsgDeliveryStart[5] = 0x00;
  rsuReceivedMsg->rsuReceivedMsgDeliveryStart[6] = 0x00;
  rsuReceivedMsg->rsuReceivedMsgDeliveryStart[7] = 0x01;

  rsuReceivedMsg->rsuReceivedMsgDeliveryStop[0] = 0x08;
  rsuReceivedMsg->rsuReceivedMsgDeliveryStop[1] = 0x33;
  rsuReceivedMsg->rsuReceivedMsgDeliveryStop[2] = 0x0c;
  rsuReceivedMsg->rsuReceivedMsgDeliveryStop[3] = 0x1f;
  rsuReceivedMsg->rsuReceivedMsgDeliveryStop[4] = 0x17;
  rsuReceivedMsg->rsuReceivedMsgDeliveryStop[5] = 0x3b;
  rsuReceivedMsg->rsuReceivedMsgDeliveryStop[6] = 0x3b;
  rsuReceivedMsg->rsuReceivedMsgDeliveryStop[7] = 0x09;

  rsuReceivedMsg->rsuReceivedMsgStatus          = ROW_STATUS_VALID_DEFAULT;
  rsuReceivedMsg->rsuReceivedMsgSecure          = 0;
  rsuReceivedMsg->rsuReceivedMsgAuthMsgInterval = RSU_RX_MSG_AUTH_INTERVAL_MIN;

  rsuReceivedMsg->version = RX_MSG_VERSION;
  rsuReceivedMsg->status  = ROW_STATUS_VALID_DEFAULT;
  rsuReceivedMsg->reserved = 0x0;
  rsuReceivedMsg->crc = 0x0;
}
static void init_rsuReceivedMsg(void)
{
  int32_t i = 0;

  ntcip_1218_mib.maxRsuReceivedMsgs = RSU_RX_MSG_MAX;
  for(i=0; i < RSU_RX_MSG_MAX; i++){
      set_default_row_rsuReceivedMsg(&ntcip_1218_mib.rsuReceivedMsgTable[i],i);
  }
}

/* 5.7.1 - 5.7.11 */
int32_t copy_gnssOutput_to_skinny(GnssOutput_t * gnssOutput)
{
  if(NULL == gnssOutput)
      return RSEMIB_BAD_INPUT;

  gnssOutput->rsuGnssOutputPort = ntcip_1218_mib.rsuGnssOutputPort;
  memcpy(&gnssOutput->rsuGnssOutputAddress,&ntcip_1218_mib.rsuGnssOutputAddress,RSU_DEST_IP_MAX);
  gnssOutput->rsuGnssOutputAddress_length = ntcip_1218_mib.rsuGnssOutputAddress_length;
  memcpy(&gnssOutput->rsuGnssOutputInterface, ntcip_1218_mib.rsuGnssOutputInterface, RSU_GNSS_OUTPUT_INTERFACE_MAX);
  gnssOutput->rsuGnssOutputInterface_length = ntcip_1218_mib.rsuGnssOutputInterface_length;
  gnssOutput->rsuGnssOutputInterval = ntcip_1218_mib.rsuGnssOutputInterval;

  /* Read only. Dont need to be stored. Derived at run time.
   * uint8_t rsuGnssOutputString[RSU_GNSS_OUTPUT_STRING_MAX];
   * int32_t rsuGnssLat;
   * int32_t rsuGnssLon;
   * int32_t rsuGnssElv;
   */

  gnssOutput->rsuGnssMaxDeviation = ntcip_1218_mib.rsuGnssMaxDeviation;

  /* Read Only.
   * gnssOutput->rsuLocationDeviation = ntcip_1218_mib.rsuLocationDeviation;
   * gnssOutput->rsuGnssPositionError = ntcip_1218_mib.rsuGnssPositionError;
   */

  /* Internal revision & control. */
  gnssOutput->version  = GNSS_OUTPUT_VERSION;
  gnssOutput->status   = ROW_STATUS_VALID_DEFAULT;
  gnssOutput->reserved = 0x0;
  gnssOutput->crc      = 0x0; /* Done at moment of write to disk in commit_sysDesc_to_disk() */  

  return RSEMIB_OK;    
}
static void restore_gnssOutput_from_disk(void)
{
  FILE    * file_in        = NULL;
  uint32_t  mib_byte_count = 0;
  int32_t   ret            = RSEMIB_OK; /* SUCCESS till proven false. */
  GnssOutput_t gnssOutput;

  mib_byte_count = sizeof(gnssOutput);
  if(RSEMIB_OK == (ret = check_file_existence(GNSS_OUTPUT_PATHFILENAME, mib_byte_count))){
      /* Open file for input. */
      if ((RSEMIB_OK == ret) && (NULL == (file_in = fopen(GNSS_OUTPUT_PATHFILENAME, "rb"))) ){
          DEBUGMSGTL((MY_NAME, "restore_gnssOutput_from_disk: fopen fail.\n"));
          ret = RSEMIB_FOPEN_FAIL;
      }
      memset(&gnssOutput,0x0,sizeof(gnssOutput));
      /* Load file into tmp ram space first before overwriting. */
      if((RSEMIB_OK == ret) && (1 != fread((uint8_t *)&gnssOutput, mib_byte_count, 1, file_in)) ){    
          DEBUGMSGTL((MY_NAME, "restore_gnssOutput_from_disk: FAIL: read_count is 0: mib_byte_count=(0x%X) errno=(%d:%s).\n",mib_byte_count,errno,strerror(errno) ));
          ret = RSEMIB_BAD_DATA;     
      }
      /* One way or another done with file so close. */
      if(NULL != file_in){
          fclose(file_in);
      }
      /* Check wellness and copy over if good.*/
      if(   (RSEMIB_OK == ret)
         && (gnssOutput.crc == (uint32_t)crc16((const uint8_t *)&gnssOutput, sizeof(gnssOutput) - 4))
         && (GNSS_OUTPUT_VERSION == gnssOutput.version)) {
          ntcip_1218_mib.rsuGnssOutputPort = gnssOutput.rsuGnssOutputPort;
          memcpy(ntcip_1218_mib.rsuGnssOutputAddress, gnssOutput.rsuGnssOutputAddress,RSU_DEST_IP_MAX);
          ntcip_1218_mib.rsuGnssOutputAddress_length = gnssOutput.rsuGnssOutputAddress_length;
          memcpy(ntcip_1218_mib.rsuGnssOutputInterface, gnssOutput.rsuGnssOutputInterface, RSU_GNSS_OUTPUT_INTERFACE_MAX);
          ntcip_1218_mib.rsuGnssOutputInterface_length = gnssOutput.rsuGnssOutputInterface_length;
          ntcip_1218_mib.rsuGnssOutputInterval = gnssOutput.rsuGnssOutputInterval;

          /* Read only. Dont restore. Derived at run time.
           * uint8_t rsuGnssOutputString[RSU_GNSS_OUTPUT_STRING_MAX];
           * int32_t rsuGnssLat;
           * int32_t rsuGnssLon;
           * int32_t rsuGnssElv;
           */

          ntcip_1218_mib.rsuGnssMaxDeviation  = gnssOutput.rsuGnssMaxDeviation;

          /* Read only.
           * ntcip_1218_mib.rsuLocationDeviation = gnssOutput.rsuLocationDeviation;
           * ntcip_1218_mib.rsuGnssPositionError = gnssOutput.rsuGnssPositionError;
           */

      } else {
          DEBUGMSGTL((MY_NAME, "restore_gnssOutput_from_disk: failed wellness check:version=%d ret=%d:0x%x:0x%x.\n",
                gnssOutput.version
              , ret
              , gnssOutput.crc
              , (uint32_t)crc16((const uint8_t *)&gnssOutput, sizeof(gnssOutput) - 4) )) ;
          ret = RSEMIB_BAD_RECORD_RESTORE; /* File will be over written next time row is used by client. Leave in place for now.*/
      }
  }
  if(RSEMIB_FILE_STAT_FAIL == ret) { /* Not an error but informative warning. */
      DEBUGMSGTL((MY_NAME, "restore_gnssOutput_from_disk: gnssOutput.dat does not exist.\n")); 
      ret = RSEMIB_OK; /* Ok for files to not exist.*/
  }
  if(RSEMIB_OK != ret){
      set_ntcip_1218_error_states(ret);
  }
}
static void init_rsuGnssOutput(void)
{
  ntcip_1218_mib.rsuGnssOutputPort = RSU_GNSS_OUTPUT_PORT_DEFAULT;

  strncpy((char_t *)ntcip_1218_mib.rsuGnssOutputAddress, RSU_GNSS_OUTPUT_ADDRESS_DEFAULT, RSU_DEST_IP_MAX);
  ntcip_1218_mib.rsuGnssOutputAddress_length = strnlen(RSU_GNSS_OUTPUT_ADDRESS_DEFAULT,RSU_DEST_IP_MAX);
  //ntcip_1218_mib.rsuGnssOutputAddress_length = RSU_DEST_IP_MAX;

  strncpy((char_t *)ntcip_1218_mib.rsuGnssOutputInterface,RSU_GNSS_OUTPUT_INTERFACE_DEFAULT, RSU_GNSS_OUTPUT_INTERFACE_MAX);
  ntcip_1218_mib.rsuGnssOutputInterface_length = strnlen(RSU_GNSS_OUTPUT_INTERFACE_DEFAULT, RSU_GNSS_OUTPUT_INTERFACE_MAX);

  ntcip_1218_mib.rsuGnssOutputInterval = RSU_GNSS_OUTPUT_INTERVAL_DEFAULT;

  memset(ntcip_1218_mib.rsuGnssOutputString, 0x0, RSU_GNSS_OUTPUT_STRING_MAX);
  ntcip_1218_mib.rsuGnssOutputString_length = 0;

  ntcip_1218_mib.rsuGnssLat = RSU_LAT_UNKNOWN;
  ntcip_1218_mib.rsuGnssLon = RSU_LON_UNKNOWN;
  ntcip_1218_mib.rsuGnssElv = RSU_ELV_UNKNOWN;
  ntcip_1218_mib.rsuGnssMaxDeviation  = RSU_GNSS_OUTPUT_DEVIATION_MIN;
  ntcip_1218_mib.rsuLocationDeviation = RSU_GNSS_OUTPUT_DEVIATION_UNKNOWN;
  ntcip_1218_mib.rsuGnssPositionError = RSU_GNSS_OUTPUT_POS_ERROR_UNKNOWN;
  /*
   * Load from disk.
   * If folder doesn't exist then create it. If it exists then try to load into MIB.
   * If we fail, keep going.
   */
  if(RSEMIB_OK != mibUtilMakeDir(GNSS_OUTPUT_PATH)) {
      set_ntcip_1218_error_states(RSEMIB_CREATE_DIR_FAIL);
      DEBUGMSGTL((MY_NAME, "init_rsuGnssOutput: Couldn't create MIB dir[%s]\n",SYS_DESC_PATH));
  } else {
      restore_gnssOutput_from_disk(); /* First time will be empty. Until user writes something. */
  }
}

/* 5.8.1 - 5.8.2.12 */
void set_default_row_interfaceLog(RsuInterfaceLogEntry_t * InterfaceLog, int32_t index)
{
  char_t command_buffer[RSU_MSG_FILENAME_LENGTH_MAX];
  uint64_t utc_dsec = 0;
  index++;
  if((NULL == InterfaceLog) || (index < RSU_IFCLOG_MIN) || (RSU_IFCLOG_MAX < index)){
      return;
  }
  InterfaceLog->rsuIfaceLogIndex    = index;
  InterfaceLog->rsuIfaceGenerate    = mib_off;
  InterfaceLog->rsuIfaceMaxFileSize = RSU_IFCLOG_FILE_SIZE_DEFAULT;
  InterfaceLog->rsuIfaceMaxFileTime = RSU_IFCLOG_FILE_TIME_DEFAULT;
  InterfaceLog->rsuIfaceLogByDir    = inboundOnly;

  memset(&InterfaceLog->rsuIfaceName, 0x0, RSU_INTERFACE_NAME_MAX);
  InterfaceLog->rsuIfaceName_length = 0;

  memset(&InterfaceLog->rsuIfaceStoragePath, 0x0, RSU_IFCLOG_STORE_PATH_MAX);
  InterfaceLog->rsuIfaceStoragePath_length = strnlen(IFC_COMPLETED_LOGS_DIR,RSU_IFCLOG_NAME_SIZE_MAX);
  memcpy(&InterfaceLog->rsuIfaceStoragePath,IFC_COMPLETED_LOGS_DIR, InterfaceLog->rsuIfaceStoragePath_length);

  memset(&InterfaceLog->rsuIfaceLogName, 0x0, RSU_IFCLOG_NAME_SIZE_MAX);
  InterfaceLog->rsuIfaceLogName_length = 0;

  memset(&InterfaceLog->rsuIfaceLogStart,0x0,MIB_DATEANDTIME_LENGTH);        
  memset(&InterfaceLog->rsuIfaceLogStop,0x0,MIB_DATEANDTIME_LENGTH);
  InterfaceLog->rsuIfaceLogOptions = RSU_IFCLOG_OPTIONS_DEFAULT;
  InterfaceLog->rsuIfaceLogStatus = ROW_STATUS_VALID_DEFAULT;

  /* Set some useful defaults for client. */
  InterfaceLog->rsuIfaceName[0] = 'e';
  InterfaceLog->rsuIfaceName[1] = 't';
  InterfaceLog->rsuIfaceName[2] = 'h';
  InterfaceLog->rsuIfaceName[3] = '0';
  InterfaceLog->rsuIfaceName_length = 4;

  InterfaceLog->rsuIfaceLogByDir = biCombined;

  /* 
     default start:
       07e6-01-01:00:00:00.1
       2022-01-01:00:00:00.1
 
     default stop:
       0833-0c-1f-17:3b:3b.9
       2099-12-31:23:59:59.9
  */
  InterfaceLog->rsuIfaceLogStart[0] = 0x07;
  InterfaceLog->rsuIfaceLogStart[1] = 0xe6;
  InterfaceLog->rsuIfaceLogStart[2] = 0x01;
  InterfaceLog->rsuIfaceLogStart[3] = 0x01;
  InterfaceLog->rsuIfaceLogStart[4] = 0x00;
  InterfaceLog->rsuIfaceLogStart[5] = 0x00;
  InterfaceLog->rsuIfaceLogStart[6] = 0x00;
  InterfaceLog->rsuIfaceLogStart[7] = 0x01;

  InterfaceLog->rsuIfaceLogStop[0] = 0x08;
  InterfaceLog->rsuIfaceLogStop[1] = 0x33;
  InterfaceLog->rsuIfaceLogStop[2] = 0x0c;
  InterfaceLog->rsuIfaceLogStop[3] = 0x1f;
  InterfaceLog->rsuIfaceLogStop[4] = 0x17;
  InterfaceLog->rsuIfaceLogStop[5] = 0x3b;
  InterfaceLog->rsuIfaceLogStop[6] = 0x3b;
  InterfaceLog->rsuIfaceLogStop[7] = 0x09;

  if(RSEMIB_OK != DateAndTime_To_UTC_DSEC(InterfaceLog->rsuIfaceLogStart,&utc_dsec)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      InterfaceLog->start_utc_dsec = 0;
  } else {
      InterfaceLog->start_utc_dsec = utc_dsec;
  }
  if(RSEMIB_OK != DateAndTime_To_UTC_DSEC(InterfaceLog->rsuIfaceLogStop,&utc_dsec)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      InterfaceLog->stop_utc_dsec = 0;
  } else {
      InterfaceLog->stop_utc_dsec = utc_dsec;
  }
  /* ifclogger will provide name of file to logmgr who will place in SHM for MIB on completetion. */
  memset(&InterfaceLog->rsuIfaceLogName, 0x0, RSU_IFCLOG_NAME_SIZE_MAX);
  InterfaceLog->rsuIfaceLogName_length = strnlen("xxx_yyy_dir_start_stop",RSU_IFCLOG_NAME_SIZE_MAX);
  memcpy(&InterfaceLog->rsuIfaceLogName,"xxx_yyy_dir_start_stop", InterfaceLog->rsuIfaceLogName_length);

  InterfaceLog->version = INTERFACE_LOG_VERSION;
  InterfaceLog->status  = ROW_STATUS_VALID_DEFAULT;
  InterfaceLog->reserved = 0x0;
  InterfaceLog->crc = 0x0;

  memset(command_buffer,'\0',sizeof(command_buffer));
  snprintf(command_buffer,sizeof(command_buffer),"%s/%d.dat",INTERFACE_LOG_OUTPUT_PATH, InterfaceLog->rsuIfaceLogIndex); /* Give row a file name based off index. Binary file so name it so. */
  memcpy(&InterfaceLog->filePathName, command_buffer, RSU_MSG_FILENAME_LENGTH_MAX);
}
static void restore_rsuInterfaceLog_from_disk(void)
{
  FILE    * file_in        = NULL;
  uint32_t  mib_byte_count = 0;
  int32_t   ret            = RSEMIB_OK; /* SUCCESS till proven false. */
  RsuInterfaceLogEntry_t rsuInterfaceLog;
  char_t   filePathName[RSU_MSG_FILENAME_LENGTH_MAX];
  int32_t  i;

  mib_byte_count = sizeof(rsuInterfaceLog);
  memset(filePathName,'\0',RSU_MSG_FILENAME_LENGTH_MAX);
  for(i=0; i < RSU_IFCLOG_MAX; i++) {
      /* file name always based off index. */
      snprintf(filePathName,RSU_MSG_FILENAME_LENGTH_MAX, "%s/%d.dat",INTERFACE_LOG_OUTPUT_PATH , i + 1);
      /* It's ok for the file to not exist. */
      if(RSEMIB_OK == check_file_existence(filePathName, mib_byte_count)){
          /* Open file for input.*/
          if ((RSEMIB_OK == ret) && (NULL == (file_in = fopen(filePathName, "rb"))) ){
              DEBUGMSGTL((MY_NAME, "restore_rsuInterfaceLog_from_disk: fopen fail.\n"));
              ret = RSEMIB_FOPEN_FAIL;
          }
          memset(&rsuInterfaceLog,0x0,sizeof(rsuInterfaceLog));
          /* Load file into tmp ram space first before overwriting. */
          if((RSEMIB_OK == ret) && (1 != fread((uint8_t *)&rsuInterfaceLog, mib_byte_count, 1, file_in)) ){    
              DEBUGMSGTL((MY_NAME, "restore_rsuInterfaceLog_from_disk: FAIL: read_count is 0: mib_byte_count=(0x%X) errno=(%d:%s).\n",mib_byte_count,errno,strerror(errno) ));
              ret = RSEMIB_BAD_DATA;
          }
          /* One way or another done with file so close. */
          if(NULL != file_in){
              fclose(file_in);
          }
          /* Check wellness and copy over if good.*/
          if(RSEMIB_OK == ret){
              if(   (rsuInterfaceLog.crc == (uint32_t)crc16((const uint8_t *)&rsuInterfaceLog, sizeof(rsuInterfaceLog) - 4))
                 && (INTERFACE_LOG_VERSION == rsuInterfaceLog.version)) { 
                  memcpy(&ntcip_1218_mib.rsuInterfaceLogTable[i],&rsuInterfaceLog, sizeof(rsuInterfaceLog));
              } else {
                  DEBUGMSGTL((MY_NAME, "restore_rsuInterfaceLog_from_disk: index=%d failed wellness check.\n",i));
                  ret = RSEMIB_BAD_RECORD_RESTORE; /* File will be over written next time row is used by client. Leave in place for now.*/
              }
          }
      }
  }
  if(RSEMIB_OK != ret){
      set_ntcip_1218_error_states(ret);
  }
}
static void init_rsuInterfaceLog(void)
{
  int32_t i = 0;

  ntcip_1218_mib.maxRsuInterfaceLogs = RSU_IFCLOG_MAX;

  for(i=0; i < RSU_IFCLOG_MAX; i++){
      set_default_row_interfaceLog(&ntcip_1218_mib.rsuInterfaceLogTable[i],i);
  }
  /* If it doesn't exist then create it. If we fail to create keep going. */
  if (RSEMIB_OK != mibUtilMakeDir(INTERFACE_LOG_OUTPUT_PATH)) {
      set_ntcip_1218_error_states(RSEMIB_CREATE_DIR_FAIL);
      DEBUGMSGTL((MY_NAME, "init_rsuInterfaceLog: Couldn't create IFACE LOGS dir[%s]\n",INTERFACE_LOG_OUTPUT_PATH));
  } else { /* Restore IFACE LOG from disk. If it fails keep going. */
      restore_rsuInterfaceLog_from_disk();
  }
}

/* 5.9.1 - 5.9.16 */
static void init_rsuSecurity(void)
{
  int32_t i = 0;

  ntcip_1218_mib.rsuSecCredReq               = RSU_SECURITY_CRED_REQ_HOURS_DEFAULT;
  ntcip_1218_mib.rsuSecEnrollCertStatus      = enroll_cert_status_unknown;         
  ntcip_1218_mib.rsuSecEnrollCertValidRegion = RSU_SECURITY_REGION_USA;

  memset(ntcip_1218_mib.rsuSecEnrollCertUrl, 0x0, URI255_LENGTH_MAX);
  ntcip_1218_mib.rsuSecEnrollCertUrl_length = URI255_LENGTH_MIN;

  memset(ntcip_1218_mib.rsuSecEnrollCertId, 0x0, RSU_SECURITY_ENROLL_CERT_ID_LENGTH_MAX);
  ntcip_1218_mib.rsuSecEnrollCertId[0]     = '0'; /* Has to be minimum length of 1 so set ID to zero. */
  ntcip_1218_mib.rsuSecEnrollCertId_length = RSU_SECURITY_ENROLL_CERT_ID_LENGTH_MIN;

  memset(ntcip_1218_mib.rsuSecEnrollCertExpiration,0x0,MIB_DATEANDTIME_LENGTH);
  ntcip_1218_mib.rsuSecuritySource = security_source_scms;

  memset(ntcip_1218_mib.rsuSecAppCertUrl,0x0,URI1024_LENGTH_MAX);
  ntcip_1218_mib.rsuSecAppCertUrl_length = URI1024_LENGTH_MIN;

  ntcip_1218_mib.maxRsuSecAppCerts = RSU_SEC_APP_CERTS_MAX;

  for(i=0;i<RSU_SEC_APP_CERTS_MAX;i++){
      memset(&ntcip_1218_mib.rsuSecAppCertTable[i],0x0,sizeof(RsuSecAppCertEntry_t));
      ntcip_1218_mib.rsuSecAppCertTable[i].rsuSecAppCertIndex = i+1;  /* index must be 1 or greater. Set and forget. */ 
      ntcip_1218_mib.rsuSecAppCertTable[i].rsuSecAppCertState       = RSU_SEC_APP_CERT_STATE_MIN;
      ntcip_1218_mib.rsuSecAppCertTable[i].rsuSecAppCertPsid_length = RSU_SEC_APP_PSID_MIN;
      ntcip_1218_mib.rsuSecAppCertTable[i].rsuSecAppCertPsid[0]     = 0x00;  
  }

  memset(ntcip_1218_mib.rsuSecCertRevocationUrl,0x0,URI255_LENGTH_MAX);
  ntcip_1218_mib.rsuSecCertRevocationUrl_length = URI255_LENGTH_MIN;
  memset(ntcip_1218_mib.rsuSecCertRevocationTime,0x0,MIB_DATEANDTIME_LENGTH);
  ntcip_1218_mib.rsuSecCertRevocationInterval = RSU_SECURITY_CERT_REVOCATION_UPDATE_DEFAULT;
  ntcip_1218_mib.rsuSecCertRevocationUpdate = 0;
  ntcip_1218_mib.maxRsuSecProfiles = RSU_SECURITY_PROFILES_MAX;

  /* RO: Enumerate when RSU comes up. */
  for(i=0; i < RSU_SECURITY_PROFILES_MAX; i++){ 
      memset(&ntcip_1218_mib.rsuSecProfileTable[i],0x0,sizeof(RsuSecProfileEntry_t));
      ntcip_1218_mib.rsuSecProfileTable[i].rsuSecProfileIndex = i + 1; /* index must be 1 or greater. Set and forget. */
      ntcip_1218_mib.rsuSecProfileTable[i].rsuSecProfileName[0] = 'X'; /* Has to be length 1 minimum. */
      ntcip_1218_mib.rsuSecProfileTable[i].rsuSecProfileName_length = RSU_SECURITY_PROFILE_NAME_LENGTH_MIN;
      memset(&ntcip_1218_mib.rsuSecProfileTable[i].rsuSecProfileDesc,0x0,RSU_SECURITY_PROFILE_DESC_LENGTH_MAX);
      ntcip_1218_mib.rsuSecProfileTable[i].rsuSecProfileDesc_length = RSU_SECURITY_PROFILE_DESC_LENGTH_MIN; /* Can be zero length. */
  }
}

/* 5.10.1 - 5.10.4 */
void set_default_row_rsuWsaService(RsuWsaServiceEntry_t * rsuWsaService, int32_t index)
{
  index++;
  if((NULL == rsuWsaService) || (index < RSU_WSA_SERVICES_MIN) || (RSU_WSA_SERVICES_MAX < index)){
      return;
  }
  rsuWsaService->rsuWsaIndex = index;
  memset(&rsuWsaService->rsuWsaPsid, 0x0, RSU_RADIO_PSID_SIZE);
  rsuWsaService->rsuWsaPsid_length = RSU_RADIO_PSID_MIN_SIZE;
  rsuWsaService->rsuWsaPriority    = WSA_PRIORITY_MIN;
  memset(&rsuWsaService->rsuWsaPSC, 0x0,WSA_PSC_LENGTH_MAX);
  rsuWsaService->rsuWsaPSC_length = WSA_PSC_LENGTH_MIN;
  memset(&rsuWsaService->rsuWsaIpAddress, 0x0,RSU_DEST_IP_MAX);
  rsuWsaService->rsuWsaIpAddress_length = WSA_IP_ADDR_LENGTH_MIN;
  rsuWsaService->rsuWsaPort    = WSA_PORT_MIN;
  rsuWsaService->rsuWsaChannel = RSU_RADIO_CHANNEL_MIN;
  rsuWsaService->rsuWsaStatus  = ROW_STATUS_VALID_DEFAULT;
  memset(&rsuWsaService->rsuWsaMacAddress, 0x0,RSU_RADIO_MAC_LENGTH);
  rsuWsaService->rsuWsaOptions                = WSA_OPTIONS_DEFAULT;
  rsuWsaService->rsuWsaRcpiThreshold          = WSA_RCPI_THRESHOLD_MIN;
  rsuWsaService->rsuWsaCountThreshold         = WSA_COUNT_THRESHOLD_MIN;
  rsuWsaService->rsuWsaCountThresholdInterval = WSA_COUNT_THRESHOLD_INTERVAL_MIN;
  rsuWsaService->rsuWsaRepeatRate             = WSA_REPEAT_RATE_MIN;
  memset(&rsuWsaService->rsuWsaAdvertiserIdentifier, 0x0, WSA_AD_ID_LENGTH_MAX);
  rsuWsaService->rsuWsaAdvertiserIdentifier_length = WSA_AD_ID_LENGTH_MIN;
  rsuWsaService->rsuWsaEnable = 0;

}

void set_default_row_rsuWsaChannel(RsuWsaChannelEntry_t * rsuWsaChannel, int32_t index)
{
  index++;
  if((NULL == rsuWsaChannel) || (index < RSU_WSA_SERVICES_MIN) || (RSU_WSA_SERVICES_MAX < index)){
      return;
  }
  rsuWsaChannel->rsuWsaChannelIndex = index;
  memset(&rsuWsaChannel->rsuWsaChannelPsid,0x0,RSU_RADIO_PSID_SIZE);
  rsuWsaChannel->rsuWsaChannelPsid_length  = RSU_RADIO_PSID_MIN_SIZE;

  rsuWsaChannel->rsuWsaChannelNumber       = RSU_RADIO_CHANNEL_MIN;
  rsuWsaChannel->rsuWsaChannelTxPowerLevel = RSU_RADIO_TX_POWER_MIN;                                                   
  rsuWsaChannel->rsuWsaChannelAccess       = WSA_CHANNEL_ACCESS_MIN; 
  rsuWsaChannel->rsuWsaChannelStatus       = ROW_STATUS_VALID_DEFAULT;
}
static void init_rsuWsaConfig(void)
{
  int32_t i = 0;

  ntcip_1218_mib.maxRsuWsaServices = RSU_WSA_SERVICES_MAX;
  ntcip_1218_mib.rsuWsaVersion     = RSU_WSA_VERSION;

  for(i=0; i < RSU_WSA_SERVICES_MAX; i++){
      set_default_row_rsuWsaService(&ntcip_1218_mib.rsuWsaServiceTable[i],i);
      set_default_row_rsuWsaChannel(&ntcip_1218_mib.rsuWsaChannelTable[i],i);
  }
}

/* 5.11.1 - 5.11.7 */
static void init_rsuWraConfig(void)
{
  memset(ntcip_1218_mib.rsuWraIpPrefix,0x0,RSU_DEST_IP_MAX);
  ntcip_1218_mib.rsuWraIpPrefix_length = RSU_DEST_IP_MIN; 

  ntcip_1218_mib.rsuWraIpPrefixLength = RSU_DEST_IP_MAX; /* not the same as above. */

  memset(ntcip_1218_mib.rsuWraGateway,0x0,RSU_DEST_IP_MAX);
  ntcip_1218_mib.rsuWraGateway_length = RSU_DEST_IP_MIN;

  memset(ntcip_1218_mib.rsuWraPrimaryDns,0x0,RSU_DEST_IP_MAX);
  ntcip_1218_mib.rsuWraPrimaryDns_length = RSU_DEST_IP_MIN;

  memset(ntcip_1218_mib.rsuWraSecondaryDns,0x0,RSU_DEST_IP_MAX);
  ntcip_1218_mib.rsuWraSecondaryDns_length = RSU_DEST_IP_MIN;

  memset(ntcip_1218_mib.rsuWraGatewayMacAddress,0x1,RSU_RADIO_MAC_LENGTH);
  ntcip_1218_mib.rsuWraLifetime = RSU_WRA_LIFETIME_MIN;
}

/* 5.12.1 - 5.12.2 */
static void set_default_row_rsuMessageStats(RsuMessageCountsByPsidEntry_t * rsuMessageStats, int32_t index)
{
  char_t command_buffer[RSU_MSG_FILENAME_LENGTH_MAX];
  uint8_t DateAndTime[MIB_DATEANDTIME_LENGTH];

  index++;
  if((NULL == rsuMessageStats) || (index < RSU_PSID_TRACKED_STATS_MIN) || (RSU_PSID_TRACKED_STATS_MAX < index)){
      return;
  }
  rsuMessageStats->rsuMessageCountsByPsidIndex = index;
  memset(&rsuMessageStats->rsuMessageCountsByPsidId,0x0,RSU_RADIO_PSID_SIZE);
  rsuMessageStats->rsuMessageCountsByPsidId_length = RSU_RADIO_PSID_MIN_SIZE;
  /* Assume inbound BSM. */
  rsuMessageStats->rsuMessageCountsByPsidId[0] = 0x0;
  rsuMessageStats->rsuMessageCountsByPsidId[1] = 0x0;
  rsuMessageStats->rsuMessageCountsByPsidId[2] = 0x00;
  rsuMessageStats->rsuMessageCountsByPsidId[3] = 0x20;
  rsuMessageStats->rsuMessageCountsByPsidId_length = 1;
  rsuMessageStats->rsuMessageCountsByChannel = RSU_CV2X_CHANNEL_DEFAULT;
  rsuMessageStats->rsuMessageCountsDirection = 1;

  /* default start == 07e6-01-01:00:00:00.1  ==  2022-01-01:00:00:00.1 */
  rsuMessageStats->rsuMessageCountsByPsidTime[0] = 0x07;
  rsuMessageStats->rsuMessageCountsByPsidTime[1] = 0xe6;
  rsuMessageStats->rsuMessageCountsByPsidTime[2] = 0x01;
  rsuMessageStats->rsuMessageCountsByPsidTime[3] = 0x01;
  rsuMessageStats->rsuMessageCountsByPsidTime[4] = 0x00;
  rsuMessageStats->rsuMessageCountsByPsidTime[5] = 0x00;
  rsuMessageStats->rsuMessageCountsByPsidTime[6] = 0x00;
  rsuMessageStats->rsuMessageCountsByPsidTime[7] = 0x01;

  if(RSEMIB_OK == get_mib_DateAndTime(DateAndTime)) {
      memcpy(rsuMessageStats->rsuMessageCountsByPsidTime,DateAndTime,MIB_DATEANDTIME_LENGTH);
  }
  rsuMessageStats->rsuMessageCountsByPsidRowStatus = ROW_STATUS_VALID_DEFAULT;
  /* DENSO specfic. */
  rsuMessageStats->version = MSG_STATS_VERSION;
  rsuMessageStats->status  = ROW_STATUS_VALID_DEFAULT;
  rsuMessageStats->reserved = 0x0;
  rsuMessageStats->crc = 0x0;
  memset(command_buffer,'\0',sizeof(command_buffer));
  snprintf(command_buffer,sizeof(command_buffer),"%s/%d.dat",MSG_STATS_OUTPUT_PATH, index); /* Give row a file name based off index. Binary file so name it so. */
  memcpy(&rsuMessageStats->filePathName, command_buffer, RSU_MSG_FILENAME_LENGTH_MAX);
}

static void restore_rsuMessageStats_from_disk(void)
{
  FILE    * file_in        = NULL;
  uint32_t  mib_byte_count = 0;
  int32_t   ret            = RSEMIB_OK; /* SUCCESS till proven false. */
  RsuMessageCountsByPsidEntry_t rsuMessageStats;
  char_t   filePathName[RSU_MSG_FILENAME_LENGTH_MAX];
  int32_t  i;

  mib_byte_count = sizeof(rsuMessageStats);
  memset(filePathName,'\0',RSU_MSG_FILENAME_LENGTH_MAX);
  for(i=0; i < RSU_PSID_TRACKED_STATS_MAX; i++) {
      /* commRanget file name always based off index. */
      snprintf(filePathName,RSU_MSG_FILENAME_LENGTH_MAX, "%s/%d.dat", MSG_STATS_OUTPUT_PATH , i + 1);
      /* It's ok for the file to not exist. */
      if(RSEMIB_OK == check_file_existence(filePathName, mib_byte_count)){
          /* Open file for input.*/
          if ((RSEMIB_OK == ret) && (NULL == (file_in = fopen(filePathName, "rb"))) ){
              DEBUGMSGTL((MY_NAME, "restore_rsuMessageStats_from_disk: fopen fail.\n"));
              ret = RSEMIB_FOPEN_FAIL;
          }
          memset(&rsuMessageStats,0x0,sizeof(rsuMessageStats));
          /* Load file into tmp ram space first before overwriting. */
          if((RSEMIB_OK == ret) && (1 != fread((uint8_t *)&rsuMessageStats, mib_byte_count, 1, file_in)) ){    
              DEBUGMSGTL((MY_NAME, "restore_rsuMessageStats_from_disk: FAIL: read_count is 0: mib_byte_count=(0x%X) errno=(%d:%s).\n",mib_byte_count,errno,strerror(errno) ));
              ret = RSEMIB_BAD_DATA;     
          }
          /* One way or another done with file so close. */
          if(NULL != file_in){
              fclose(file_in);
          }
          /* Check wellness and copy over if good.*/
          if(RSEMIB_OK == ret){
              if(   (rsuMessageStats.crc == (uint32_t)crc16((const uint8_t *)&rsuMessageStats, sizeof(rsuMessageStats) - 4))
                 && (RX_MSG_VERSION == rsuMessageStats.version)) { 
                  memcpy(&ntcip_1218_mib.rsuMessageCountsByPsidTable[i],&rsuMessageStats, sizeof(rsuMessageStats));
                  /* Any runtime updates will be done by SHM thread prior to seeding SHM. */
              } else {
                  DEBUGMSGTL((MY_NAME, "restore_rsuMessageStats_from_disk: index=%d failed wellness check.\n",i));
                  ret = RSEMIB_BAD_RECORD_RESTORE; /* File will be over written next time row is used by client. Leave in place for now.*/
              }
          }
      }
  }
  if(RSEMIB_OK != ret){
      set_ntcip_1218_error_states(ret);
  }
}
static void init_rsuMessageStats(void)
{
  int32_t i = 0;

  ntcip_1218_mib.maxRsuMessageCountsByPsid = RSU_PSID_TRACKED_STATS_MAX;
  for(i = 0; i < RSU_PSID_TRACKED_STATS_MAX; i++){
      set_default_row_rsuMessageStats(&ntcip_1218_mib.rsuMessageCountsByPsidTable[i],i);
  }
  /* If it doesn't exist then create it. If we fail to create keep going. */
  if (RSEMIB_OK != mibUtilMakeDir(MSG_STATS_OUTPUT_PATH)) {
      set_ntcip_1218_error_states(RSEMIB_CREATE_DIR_FAIL);
      DEBUGMSGTL((MY_NAME, "init_rsuMessageStats: Couldn't create MSG STATS dir[%s]\n",MSG_STATS_OUTPUT_PATH));
  } else { /* Restore RX MSG from disk. If it fails keep going. */
      restore_rsuMessageStats_from_disk();
  }
}

/* 5.13.1 - 5.13.6 */
void set_default_row_rsuCommRange(RsuCommRangeEntry_t * rsuCommRange, int32_t index)
{
  char_t  command_buffer[RSU_MSG_FILENAME_LENGTH_MAX];

  index++;
  if((NULL == rsuCommRange) || (index <RSU_COMM_RANGE_MIN) || ( RSU_COMM_RANGE_MAX < index)){
      return;
  }
  rsuCommRange->rsuCommRangeIndex       = index;
  rsuCommRange->rsuCommRangeSector      = index; //1 to 16: RSU_COMM_RANGE_SECTOR_MIN;
  rsuCommRange->rsuCommRangeMsgId       = RSU_COMM_RANGE_MSG_ID_DEFAULT; //RSU_COMM_RANGE_MSG_ID_MIN;
  rsuCommRange->rsuCommRangeFilterType  = RSU_COMM_RANGE_FILTER_TYPE_MIN;
  rsuCommRange->rsuCommRangeFilterValue = RSU_COMM_RANGE_FILTER_VALUE_MIN;
  rsuCommRange->rsuCommRange1Min        = RSU_COMM_RANGE_MINUTES_MIN;
  rsuCommRange->rsuCommRange5Min        = RSU_COMM_RANGE_MINUTES_MIN;
  rsuCommRange->rsuCommRange15Min       = RSU_COMM_RANGE_MINUTES_MIN;
  rsuCommRange->rsuCommRangeAvg1Min     = RSU_COMM_RANGE_MINUTES_MIN;
  rsuCommRange->rsuCommRangeAvg5Min     = RSU_COMM_RANGE_MINUTES_MIN;
  rsuCommRange->rsuCommRangeAvg15Min    = RSU_COMM_RANGE_MINUTES_MIN;
  rsuCommRange->rsuCommRangeStatus      = ROW_STATUS_VALID_DEFAULT;

  rsuCommRange->version = COMM_RANGE_VERSION;
  rsuCommRange->status  = ROW_STATUS_VALID_DEFAULT;
  rsuCommRange->reserved = 0x0;
  rsuCommRange->crc = 0x0;

  memset(command_buffer,'\0',sizeof(command_buffer));
  sprintf(command_buffer,"%s/%d.dat",COMM_RANGE_OUTPUT_PATH, rsuCommRange->rsuCommRangeIndex); /* Give row a file name based off index. Binary file so name it so. */
  memcpy(&rsuCommRange->filePathName, command_buffer, RSU_MSG_FILENAME_LENGTH_MAX);
}
static void restore_commRange_from_disk(void)
{
  FILE    * file_in        = NULL;
  uint32_t  mib_byte_count = 0;
  int32_t   ret            = RSEMIB_OK; /* SUCCESS till proven false. */
  RsuCommRangeEntry_t commRange;
  char_t   filePathName[RSU_MSG_FILENAME_LENGTH_MAX];
  int32_t  i;

  mib_byte_count = sizeof(commRange);
  memset(filePathName,'\0',RSU_MSG_FILENAME_LENGTH_MAX);
  for(i=0; i < RSU_COMM_RANGE_MAX; i++) {
      /* commRanget file name always based off index. */
      snprintf(filePathName,RSU_MSG_FILENAME_LENGTH_MAX, "%s/%d.dat",COMM_RANGE_OUTPUT_PATH , i + 1);
      /* It's ok for the file to not exist. */
      if(RSEMIB_OK == check_file_existence(filePathName, mib_byte_count)){
          /* Open file for input.*/
          if ((RSEMIB_OK == ret) && (NULL == (file_in = fopen(filePathName, "rb"))) ){
              DEBUGMSGTL((MY_NAME, "restore_commRange_from_disk: fopen fail.\n"));
              ret = RSEMIB_FOPEN_FAIL;
          }
          memset(&commRange,0x0,sizeof(commRange));
          /* Load file into tmp ram space first before overwriting. */
          if((RSEMIB_OK == ret) && (1 != fread((uint8_t *)&commRange, mib_byte_count, 1, file_in)) ){    
              DEBUGMSGTL((MY_NAME, "restore_commRange_from_disk: FAIL: read_count is 0: mib_byte_count=(0x%X) errno=(%d:%s).\n",mib_byte_count,errno,strerror(errno) ));
              ret = RSEMIB_BAD_DATA;     
          }
          /* One way or another done with file so close. */
          if(NULL != file_in){
              fclose(file_in);
          }
          /* Check wellness and copy over if good.*/
          if(RSEMIB_OK == ret){
              if(   (commRange.crc == (uint32_t)crc16((const uint8_t *)&commRange, sizeof(commRange) - 4))
                 && (COMM_RANGE_VERSION == commRange.version)) { 
                  memcpy(&ntcip_1218_mib.rsuCommRangeTable[i],&commRange, sizeof(commRange));
              } else {
                  DEBUGMSGTL((MY_NAME, "restore_commRange_from_disk: index=%d failed wellness check.\n",i));
                  ret = RSEMIB_BAD_RECORD_RESTORE; /* File will be over written next time row is used by client. Leave in place for now.*/
              }
          }
      }
  }
  if(RSEMIB_OK != ret){
      set_ntcip_1218_error_states(ret);
  }
}
static void init_rsuSystemStats(void)
{
  int32_t i = 0;

  ntcip_1218_mib.rsuTimeSincePowerOn     = (uint32_t)(mibUtilGetTimeInMs()/(uint64_t)1000); /* We boot operational. Keep time. */
  ntcip_1218_mib.rsuIntTemp              = RSU_TEMP_CELSIUS_MIN;
  ntcip_1218_mib.rsuIntTempLowThreshold  = RSU_TEMP_LOW_CELSIUS_THRESHOLD;
  ntcip_1218_mib.rsuIntTempHighThreshold = RSU_TEMP_HIGH_CELSIUS_THRESHOLD;
  ntcip_1218_mib.maxRsuCommRange         = RSU_COMM_RANGE_MAX;

  for(i = 0; i < RSU_COMM_RANGE_MAX; i++){
      memset(&ntcip_1218_mib.rsuCommRangeTable[i],0x0,sizeof(RsuCommRangeEntry_t));
      set_default_row_rsuCommRange(&ntcip_1218_mib.rsuCommRangeTable[i],i);
  }

  /*
   * Load from disk.
   * If folder doesn't exist then create it. If it exists then try to load into MIB.
   * If we fail, keep going.
   */
  if(RSEMIB_OK != mibUtilMakeDir(COMM_RANGE_OUTPUT_PATH)) {
      set_ntcip_1218_error_states(RSEMIB_CREATE_DIR_FAIL);
      DEBUGMSGTL((MY_NAME, "init_rsuSystemStats: Couldn't create MIB dir[%s]\n",COMM_RANGE_OUTPUT_PATH));
  } else {
      restore_commRange_from_disk(); /* First time will be empty. Until user writes something. */
      for(i = 0; i < RSU_COMM_RANGE_MAX; i++){ /* Clear prior runs stats. */
          ntcip_1218_mib.rsuCommRangeTable[i].rsuCommRange1Min     = RSU_COMM_RANGE_MINUTES_MIN;
          ntcip_1218_mib.rsuCommRangeTable[i].rsuCommRange5Min     = RSU_COMM_RANGE_MINUTES_MIN;
          ntcip_1218_mib.rsuCommRangeTable[i].rsuCommRange15Min    = RSU_COMM_RANGE_MINUTES_MIN;
          ntcip_1218_mib.rsuCommRangeTable[i].rsuCommRangeAvg1Min  = RSU_COMM_RANGE_MINUTES_MIN;
          ntcip_1218_mib.rsuCommRangeTable[i].rsuCommRangeAvg5Min  = RSU_COMM_RANGE_MINUTES_MIN;
          ntcip_1218_mib.rsuCommRangeTable[i].rsuCommRangeAvg15Min = RSU_COMM_RANGE_MINUTES_MIN;
     }
  }
}

/* 5.14.1 - 5.14.15 */
int32_t copy_sysDesc_to_skinny(ntcip_1218_mib_t * ntcip_1218_mib , SystemDescription_t * sysDesc)
{
  if((NULL == ntcip_1218_mib) || (NULL == sysDesc))
      return RSEMIB_BAD_INPUT;

  memset(sysDesc,0x0,sizeof(SystemDescription_t));
  memcpy(&sysDesc->rsuMibVersion, &ntcip_1218_mib->rsuMibVersion,RSUMIB_VERSION_LENGTH_MAX);
  sysDesc->rsuMibVersion_length = ntcip_1218_mib->rsuMibVersion_length;

  memcpy(&sysDesc->rsuFirmwareVersion,&ntcip_1218_mib->rsuFirmwareVersion, RSU_FIRMWARE_VERSION_LENGTH_MAX);
  sysDesc->rsuFirmwareVersion_length = ntcip_1218_mib->rsuFirmwareVersion_length;

  memcpy(&sysDesc->rsuLocationDesc, &ntcip_1218_mib->rsuLocationDesc, RSU_LOCATION_DESC_LENGTH_MAX);
  sysDesc->rsuLocationDesc_length = ntcip_1218_mib->rsuLocationDesc_length;

  memcpy(&sysDesc->rsuID, &ntcip_1218_mib->rsuID, RSU_ID_LENGTH_MAX);
  sysDesc->rsuID_length = ntcip_1218_mib->rsuID_length;

  sysDesc->rsuLocationLat = ntcip_1218_mib->rsuLocationLat;
  sysDesc->rsuLocationLon = ntcip_1218_mib->rsuLocationLon;
  sysDesc->rsuLocationElv = ntcip_1218_mib->rsuLocationElv;
  sysDesc->rsuElevationOffset = ntcip_1218_mib->rsuElevationOffset;
  sysDesc->rsuInstallUpdate = ntcip_1218_mib->rsuInstallUpdate;

  memcpy(&sysDesc->rsuInstallFile, &ntcip_1218_mib->rsuInstallFile, RSU_INSTALL_FILE_NAME_LENGTH_MAX);
  sysDesc->rsuInstallFile_length = ntcip_1218_mib->rsuInstallFile_length;

  memcpy(&sysDesc->rsuInstallPath, &ntcip_1218_mib->rsuInstallPath, RSU_INSTALL_PATH_LENGTH_MAX);
  sysDesc->rsuInstallPath_length = ntcip_1218_mib->rsuInstallPath_length;

  sysDesc->rsuInstallStatus = ntcip_1218_mib->rsuInstallStatus;

  memcpy(sysDesc->rsuInstallTime, ntcip_1218_mib->rsuInstallTime, MIB_DATEANDTIME_LENGTH); 

  memcpy(&sysDesc->rsuInstallStatusMessage, &ntcip_1218_mib->rsuInstallStatusMessage, RSU_INSTALL_STATUS_MSG_LENGTH_MAX);
  sysDesc->rsuInstallStatusMessage_length = ntcip_1218_mib->rsuInstallStatusMessage_length;

  memcpy(&sysDesc->rsuScheduledInstallTime, &ntcip_1218_mib->rsuScheduledInstallTime, MIB_DATEANDTIME_LENGTH);
  
  /* Internal revision & control. */
  sysDesc->version  = SYS_DESC_VERSION;
  sysDesc->status   = ROW_STATUS_VALID_DEFAULT;
  sysDesc->reserved = 0x0;
  sysDesc->crc      = 0x0; /* Done at moment of write to disk in commit_sysDesc_to_disk() */  

  return RSEMIB_OK;    
}
static void restore_sysDesc_from_disk(void)
{
  FILE    * file_in        = NULL;
  uint32_t  mib_byte_count = 0;
  int32_t   ret            = RSEMIB_OK; /* SUCCESS till proven false. */
  SystemDescription_t sysDesc;

  mib_byte_count = sizeof(sysDesc);
  if(RSEMIB_OK == (ret = check_file_existence(SYS_DESC_PATHFILENAME, mib_byte_count))){
      /* Open file for input.*/
      if ((RSEMIB_OK == ret) && (NULL == (file_in = fopen(SYS_DESC_PATHFILENAME, "rb"))) ){
          DEBUGMSGTL((MY_NAME, "restore_sysDesc_from_disk: fopen fail.\n"));
          ret = RSEMIB_FOPEN_FAIL;
      }
      memset(&sysDesc,0x0,sizeof(sysDesc));
      /* Load file into tmp ram space first before overwriting. */
      if((RSEMIB_OK == ret) && (1 != fread((uint8_t *)&sysDesc, mib_byte_count, 1, file_in)) ){    
          DEBUGMSGTL((MY_NAME, "restore_sysDesc_from_disk: FAIL: read_count is 0: mib_byte_count=(0x%X) errno=(%d:%s).\n",mib_byte_count,errno,strerror(errno) ));
          ret = RSEMIB_BAD_DATA;     
      }
      /* One way or another done with file so close. */
      if(NULL != file_in){
          fclose(file_in);
      }
      /* Check wellness and copy over if good.*/
      if(   (RSEMIB_OK == ret)
         && (sysDesc.crc == (uint32_t)crc16((const uint8_t *)&sysDesc, sizeof(sysDesc) - 4))
         && (SYS_DESC_VERSION == sysDesc.version)) {

          /* NOTE: These are not stored on disk. Derived at runtime.
           *  ntcip_1218_mib.rsuMibVersion
           *  ntcip_1218_mib.rsuFirmwareVersion
           *  ntcip_1218_mib.rsuFirmwareVersion_length
           */

          memcpy(ntcip_1218_mib.rsuLocationDesc,sysDesc.rsuLocationDesc,RSU_LOCATION_DESC_LENGTH_MAX);
          ntcip_1218_mib.rsuLocationDesc_length = sysDesc.rsuLocationDesc_length;
#if 0 /* Come from NOR. Don't recover or store to disk. */
          memcpy(ntcip_1218_mib.rsuID,sysDesc.rsuID, RSU_ID_LENGTH_MAX);
          ntcip_1218_mib.rsuID_length = sysDesc.rsuID_length;
#endif
          ntcip_1218_mib.rsuLocationLat = sysDesc.rsuLocationLat;
          ntcip_1218_mib.rsuLocationLon = sysDesc.rsuLocationLon;
          ntcip_1218_mib.rsuLocationElv = sysDesc.rsuLocationElv;
          ntcip_1218_mib.rsuElevationOffset = sysDesc.rsuElevationOffset;
          ntcip_1218_mib.rsuInstallUpdate = sysDesc.rsuInstallUpdate;

          memcpy(ntcip_1218_mib.rsuInstallFile, sysDesc.rsuInstallFile, RSU_INSTALL_FILE_NAME_LENGTH_MAX);
          ntcip_1218_mib.rsuInstallFile_length = sysDesc.rsuInstallFile_length;

          memcpy(ntcip_1218_mib.rsuInstallPath, sysDesc.rsuInstallPath, RSU_INSTALL_PATH_LENGTH_MAX);
          ntcip_1218_mib.rsuInstallPath_length = sysDesc.rsuInstallPath_length;

          ntcip_1218_mib.rsuInstallStatus = sysDesc.rsuInstallStatus;

          memcpy(ntcip_1218_mib.rsuInstallTime, sysDesc.rsuInstallTime, MIB_DATEANDTIME_LENGTH); 

          memcpy(ntcip_1218_mib.rsuInstallStatusMessage, sysDesc.rsuInstallStatusMessage, RSU_INSTALL_STATUS_MSG_LENGTH_MAX);
          ntcip_1218_mib.rsuInstallStatusMessage_length = sysDesc.rsuInstallStatusMessage_length; 

          memcpy(ntcip_1218_mib.rsuScheduledInstallTime, sysDesc.rsuScheduledInstallTime, MIB_DATEANDTIME_LENGTH);

      } else {
          DEBUGMSGTL((MY_NAME, "restore_sysDesc_from_disk: failed wellness check:version=%d ret=%d:0x%x:0x%x.\n",sysDesc.version, ret,
                sysDesc.crc
              , (uint32_t)crc16((const uint8_t *)&sysDesc, sizeof(sysDesc) - 4) )) ;
          ret = RSEMIB_BAD_RECORD_RESTORE; /* File will be over written next time row is used by client. Leave in place for now.*/
      }
  }
  if(RSEMIB_FILE_STAT_FAIL == ret) { /* Not an error but informative warning. */
      DEBUGMSGTL((MY_NAME, "restore_sysDesc_from_disk: sysDesc.dat does not exist.\n")); 
      ret = RSEMIB_OK; /* Ok for files to not exist.*/
  }
  if(RSEMIB_OK != ret){
      set_ntcip_1218_error_states(ret);
  }
}
/* Return length on success otherwise error. */
static int32_t dcu_read_rsuFirmwareName(uint8_t * FirmwareVersion)
{
  int32_t ret = RSEMIB_OK;
  int     i;
  FILE   *fp;
  char_t  replyBuf[RSU_FIRMWARE_VERSION_LENGTH_MAX];

  if(NULL == FirmwareVersion)
      return RSEMIB_BAD_INPUT;

  memset(replyBuf,'\0',sizeof(replyBuf));
  if(NULL != (fp = popen(FIRMWARE_READ_SVN_REVISION, "r"))) {
      if(NULL != fgets(replyBuf, sizeof(replyBuf), fp)){
          i = strnlen(replyBuf,sizeof(replyBuf));
          if(0<i){ replyBuf[i-1] = '\0'; /* Delete extra char added by cat. */}
          memcpy(FirmwareVersion,replyBuf,RSU_FIRMWARE_VERSION_LENGTH_MAX); /* Don't read in after utc second. */
          DEBUGMSGTL((MY_NAME_EXTRA, "FIRMWARE_READ_SVN_REVISION=[%s]\n",replyBuf));
          ret = RSEMIB_OK;
      } else {
          DEBUGMSGTL((MY_NAME, "FIRMWARE_READ_SVN_REVISION fgets failed.\n"));
          ret = RSEMIB_SYSCALL_FAIL;
      }
      pclose(fp);
  } else {
      DEBUGMSGTL((MY_NAME, "FIRMWARE_READ_ETC_VERSION request failed.\n"));
      ret = RSEMIB_SYSCALL_FAIL;
  }
  /* Could be no NULL, thats ok. */
  if(RSEMIB_OK == ret)
      return strnlen(replyBuf,RSU_FIRMWARE_VERSION_LENGTH_MAX);
  else
      return 0;
}
static int32_t nor_write_deviceId(uint8_t * rsuID)
{
  int32_t ret = RSEMIB_OK;
  FILE   *fp = NULL;
  char_t  replyBuf[2048];

  if(NULL == rsuID)
      return RSEMIB_BAD_INPUT;

  if(NULL != (fp = fopen(RSU_CUST_DEV_WRITE_ID_FILE,"w"))) {
      if(0 < (ret=fprintf(fp,RSU_CUST_DEV_WRITE_ID_FILE_STRING,
              rsuID[0],rsuID[1],rsuID[2],rsuID[3],
              rsuID[4],rsuID[5],rsuID[6],rsuID[7],
              rsuID[8],rsuID[9],rsuID[10],rsuID[11],
              rsuID[12],rsuID[13],rsuID[14],rsuID[15],
              rsuID[16],rsuID[17],rsuID[18],rsuID[19],
              rsuID[20],rsuID[21],rsuID[22],rsuID[23],
              rsuID[24],rsuID[25],rsuID[26],rsuID[27],
              rsuID[28],rsuID[29],rsuID[30],rsuID[31]))) {
              ret = RSEMIB_OK;
      } else {
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"nor_write_deviceId: fprintf failed: ret(%d) errno(%s)\n",ret,strerror(errno));
          ret = RSEMIB_SYSCALL_FAIL;
      }
  } else {
     I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"nor_write_deviceId: fopen failed: errno(%s)\n",strerror(errno));
  }
  if(NULL != fp){
      fflush(fp);
      fclose(fp);
  }
  if(RSEMIB_OK == ret) {
      memset(replyBuf,0x0,sizeof(replyBuf));
      if(NULL != (fp = popen(RSU_CUST_DEV_WRITE_ID, "r"))) {
          ret = RSEMIB_SYSCALL_FAIL; /* Assume fail till we find what we want */
          while(NULL != fgets(replyBuf, sizeof(replyBuf), fp)) {
              if(NULL != strstr(replyBuf,RSU_CUST_DEV_WRITE_DONE)) {
                  ret = RSEMIB_OK;
                  break;
              }
          }
      } else {
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"nor_write_deviceId: popen failed: errno(%s)\n",strerror(errno));
          ret = RSEMIB_SYSCALL_FAIL;
      }
  }
  return ret;
}
static int32_t nor_read_deviceId(uint8_t * rsuID)
{
  int32_t ret = RSEMIB_OK;
  int     i;
  FILE   *fp;
  char_t  replyBuf[1024];

  if(NULL == rsuID)
      return RSEMIB_BAD_INPUT;

  memset(rsuID,'\0',RSU_ID_LENGTH_MAX);
  memset(replyBuf,'\0',sizeof(replyBuf));
  if(NULL != (fp = popen(RSU_CUST_DEV_READ_ID, "r"))) {
      /* Don't care about Device ID, read out first two lines. */
      if(NULL != fgets(replyBuf, sizeof(replyBuf), fp)){
          if(NULL != fgets(replyBuf, sizeof(replyBuf), fp)){
              ret = RSEMIB_OK;
          } else {
              I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"nor_read_deviceId:1: fgets failed: errno(%s)\n",strerror(errno));
              ret = RSEMIB_SYSCALL_FAIL;
          }
      } else {
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"nor_read_deviceId:2: fgets failed: errno(%s)\n",strerror(errno));
          ret = RSEMIB_SYSCALL_FAIL;
      }
      /* Now follows the Customer ID they programmed. */
      if((RSEMIB_OK == ret) && (NULL != fgets(replyBuf, sizeof(replyBuf), fp))){
          i = strnlen(replyBuf,sizeof(replyBuf));
          if(0<i){
              replyBuf[i-1] = '\0'; /* Delete extra char added by cat. */
          }
          if(RSU_ID_LENGTH_MAX/2 == sscanf(replyBuf,RSU_CUST_DEV_READ_ID_REPLY,
              &rsuID[0],&rsuID[1],&rsuID[2],&rsuID[3],
              &rsuID[4],&rsuID[5],&rsuID[6],&rsuID[7],
              &rsuID[8],&rsuID[9],&rsuID[10],&rsuID[11],
              &rsuID[12],&rsuID[13],&rsuID[14],&rsuID[15])) {
              ret = RSEMIB_OK;
          } else {
              I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"nor_read_deviceId:3: sscanf failed: errno(%s)\n",strerror(errno));
              ret = RSEMIB_SYSCALL_FAIL;
          }
      } else {
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"nor_read_deviceId:3: fgets failed: errno(%s)\n",strerror(errno));
          ret = RSEMIB_SYSCALL_FAIL;
      }
      if((RSEMIB_OK == ret) && (NULL != fgets(replyBuf, sizeof(replyBuf), fp))){
          i = strnlen(replyBuf,sizeof(replyBuf));
          if(0<i){
              replyBuf[i-1] = '\0'; /* Delete extra char added by cat. */
          }
          if(RSU_ID_LENGTH_MAX/2 == sscanf(replyBuf,RSU_CUST_DEV_READ_ID_REPLY,
              &rsuID[16],&rsuID[17],&rsuID[18],&rsuID[19],
              &rsuID[20],&rsuID[21],&rsuID[22],&rsuID[23],
              &rsuID[24],&rsuID[25],&rsuID[26],&rsuID[27],
              &rsuID[28],&rsuID[29],&rsuID[30],&rsuID[31])) {
              ret = RSEMIB_OK;
          } else {
              I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"nor_read_deviceId:2: sscanf failed: errno(%s)\n",strerror(errno));
              ret = RSEMIB_SYSCALL_FAIL;
          }
      } else {
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"nor_read_deviceId:2: fgets failed: errno(%s)\n",strerror(errno));
          ret = RSEMIB_SYSCALL_FAIL;
      }
      pclose(fp);
  } else {
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"nor_read_deviceId:2: request failed: errno(%s)\n",strerror(errno));
      ret = RSEMIB_SYSCALL_FAIL;
  }
  if(RSEMIB_OK == ret){ /* If not valid ascii then assume hex(binary) and '.' instead. */
      for(i=0;i<RSU_ID_LENGTH_MAX;i++) {
          if((rsuID[i] < 0x20) || (0x7E < rsuID[i])) {
              rsuID[i] = '.';
          }
      }
  }
  if(ret < RSEMIB_OK){ /* If something goes wrong then use default. */
      set_ntcip_1218_error_states(ret);
      memset(rsuID,'\0',RSU_ID_LENGTH_MAX);
      strncpy((char_t *)rsuID,RSU_ID_DEFAULT,RSU_ID_LENGTH_MAX);
  } else {
      for(i=0;i<RSU_ID_LENGTH_MAX;i++) {
          if('.' != rsuID[i]) {
              break;
          }
      }
      if(RSU_ID_LENGTH_MAX <= i) { /* If non-ascii use default. */
          memset(rsuID,'\0',RSU_ID_LENGTH_MAX);
          strncpy((char_t *)rsuID,RSU_ID_DEFAULT,RSU_ID_LENGTH_MAX);
      }
  }
  ret = strnlen((char_t *)rsuID,RSU_ID_LENGTH_MAX);
  return ret;
}
static void init_rsuSysDescription(void)
{
  /* 
   * Load defaults first.
   */
  memset(ntcip_1218_mib.rsuMibVersion, 0x0, RSUMIB_VERSION_LENGTH_MAX);
  strncpy((char_t *)ntcip_1218_mib.rsuMibVersion, RSU_MIB_VERSION, RSUMIB_VERSION_LENGTH_MAX);
  ntcip_1218_mib.rsuMibVersion_length = strnlen(RSU_MIB_VERSION,RSUMIB_VERSION_LENGTH_MAX);

  memset(ntcip_1218_mib.rsuFirmwareVersion, 0x0, RSU_FIRMWARE_VERSION_LENGTH_MAX);
  ntcip_1218_mib.rsuFirmwareVersion_length = RSU_FIRMWARE_VERSION_LENGTH_MIN;

  memset(ntcip_1218_mib.rsuLocationDesc, 0x0, RSU_LOCATION_DESC_LENGTH_MAX);
  strncpy((char_t *)ntcip_1218_mib.rsuLocationDesc, RSU_LOCATION_DESC_DEFAULT, RSU_LOCATION_DESC_LENGTH_MAX); 
  ntcip_1218_mib.rsuLocationDesc_length = strnlen(RSU_LOCATION_DESC_DEFAULT,RSU_LOCATION_DESC_LENGTH_MAX);

  ntcip_1218_mib.rsuLocationLat     = RSU_LAT_UNKNOWN;
  ntcip_1218_mib.rsuLocationLon     = RSU_LON_UNKNOWN;
  ntcip_1218_mib.rsuLocationElv     = RSU_ELV_UNKNOWN;
  ntcip_1218_mib.rsuElevationOffset = RSU_REF_ELV_OFFSET_MAX;
  ntcip_1218_mib.rsuInstallUpdate   = 0;

  memset(ntcip_1218_mib.rsuInstallFile, 0x0, RSU_INSTALL_FILE_NAME_LENGTH_MAX);
  strncpy((char_t *)ntcip_1218_mib.rsuInstallFile, RSU_INSTALL_FILE_NAME_DEFAULT, RSU_INSTALL_FILE_NAME_LENGTH_MAX);
  ntcip_1218_mib.rsuInstallFile_length = strnlen(RSU_INSTALL_FILE_NAME_DEFAULT,RSU_INSTALL_FILE_NAME_LENGTH_MAX);

  memset(ntcip_1218_mib.rsuInstallPath, 0x0, RSU_INSTALL_PATH_LENGTH_MAX);
  strncpy((char_t *)ntcip_1218_mib.rsuInstallPath, RSU_INSTALL_PATH_DEFAULT, RSU_INSTALL_PATH_LENGTH_MAX);
  ntcip_1218_mib.rsuInstallPath_length = strnlen(RSU_INSTALL_PATH_DEFAULT,RSU_INSTALL_PATH_LENGTH_MAX);

  ntcip_1218_mib.rsuInstallStatus = installUnknown;
  memset(ntcip_1218_mib.rsuInstallTime,0x0,MIB_DATEANDTIME_LENGTH); 

  memset(ntcip_1218_mib.rsuInstallStatusMessage, 0x0, RSU_INSTALL_STATUS_MSG_LENGTH_MAX);
  ntcip_1218_mib.rsuInstallStatusMessage_length = RSU_INSTALL_STATUS_MSG_LENGTH_MIN;

  memset(ntcip_1218_mib.rsuScheduledInstallTime, 0x0, MIB_DATEANDTIME_LENGTH);

  /* 
   * Load what can be derived at runtime and in lieu of valid record on disk.
   */

  /* Don't know when RSU was provisioned but it worked. Valid record on disk will overwrite. */
  ntcip_1218_mib.rsuInstallStatus = installSuccess;
  strncpy((char_t *)ntcip_1218_mib.rsuInstallStatusMessage, RSU_INSTALL_STATUS_MSG_DEFAULT, RSU_INSTALL_STATUS_MSG_LENGTH_MAX);
  ntcip_1218_mib.rsuInstallStatusMessage_length = strnlen(RSU_INSTALL_STATUS_MSG_DEFAULT, RSU_INSTALL_STATUS_MSG_LENGTH_MAX);

  /* Use BuildInfo.txt SVN_REVISION. Never stored on disk. */
  ntcip_1218_mib.rsuFirmwareVersion_length = (uint32_t)dcu_read_rsuFirmwareName(ntcip_1218_mib.rsuFirmwareVersion);
  ntcip_1218_mib.rsuFirmwareVersion_length = dcu_read_rsuFirmwareName(ntcip_1218_mib.rsuFirmwareVersion);

  /*
   * Load from disk.
   * If folder doesn't exist then create it. If it exists then try to load into MIB.
   * If we fail, keep going.
   */
  if(RSEMIB_OK != mibUtilMakeDir(SYS_DESC_PATH)) {
      set_ntcip_1218_error_states(RSEMIB_CREATE_DIR_FAIL);
      DEBUGMSGTL((MY_NAME, "init_rsuSysDescription: Couldn't create MIB dir[%s]\n",SYS_DESC_PATH));
  } else {
      restore_sysDesc_from_disk(); /* First time will be empty. Until user writes something. */
  }
  /* Is this really needed if get does the same thing?*/
  if (0 == (ntcip_1218_mib.rsuID_length = nor_read_deviceId(ntcip_1218_mib.rsuID))) {
      memset(ntcip_1218_mib.rsuID, '.', RSU_ID_LENGTH_MAX);
      ntcip_1218_mib.rsuID_length = RSU_ID_LENGTH_MAX;
  }
}

/* 5.15.1 - 5.15.22 */
static void restore_rsuSysSettings_from_disk(void)
{
  FILE    * file_in        = NULL;
  uint32_t  mib_byte_count = 0;
  int32_t   ret            = RSEMIB_OK; /* SUCCESS till proven false. */
  RsuSysSetts_t sysSetts;

  mib_byte_count = sizeof(sysSetts);
  if(RSEMIB_OK == (ret = check_file_existence(SYS_SETTS_OUTPUT_LOCATION_NAME, mib_byte_count))){
      /* Open file for input.*/
      if ((RSEMIB_OK == ret) && (NULL == (file_in = fopen(SYS_SETTS_OUTPUT_LOCATION_NAME, "rb"))) ){
          DEBUGMSGTL((MY_NAME, "restore_rsuSysSettings_from_disk: fopen fail.\n"));
          ret = RSEMIB_FOPEN_FAIL;
      }
      memset(&sysSetts,0x0,sizeof(sysSetts));
      /* Load file into tmp ram space first before overwriting. */
      if((RSEMIB_OK == ret) && (1 != fread((uint8_t *)&sysSetts, mib_byte_count, 1, file_in)) ){    
          DEBUGMSGTL((MY_NAME, "restore_rsuSysSetts_from_disk: FAIL: read_count is 0: mib_byte_count=(0x%X) errno=(%d:%s).\n",mib_byte_count,errno,strerror(errno) ));
          ret = RSEMIB_BAD_DATA;     
      }
      /* One way or another done with file so close. */
      if(NULL != file_in){
          fclose(file_in);
      }
      /* Check wellness and copy over if good.*/
      if(   (RSEMIB_OK == ret)
         && (sysSetts.crc == (uint32_t)crc16((const uint8_t *)&sysSetts, sizeof(sysSetts) - 4))
         && (SYSTEM_SETTINGS_VERSION == sysSetts.version)) {

          memcpy(ntcip_1218_mib.rsuNotifyIpAddress,sysSetts.rsuNotifyIpAddress, RSU_DEST_IP_MAX);
          ntcip_1218_mib.rsuNotifyIpAddress_length = sysSetts.rsuNotifyIpAddress_length;
          ntcip_1218_mib.rsuNotifyPort = sysSetts.rsuNotifyPort;
          memcpy(ntcip_1218_mib.rsuSysLogQueryStart, sysSetts.rsuSysLogQueryStart, RFC2579_DATEANDTIME_LENGTH);
          memcpy(ntcip_1218_mib.rsuSysLogQueryStop, sysSetts.rsuSysLogQueryStop, RFC2579_DATEANDTIME_LENGTH);
          ntcip_1218_mib.rsuSysLogQueryPriority = sysSetts.rsuSysLogQueryPriority; 
          ntcip_1218_mib.rsuSysLogQueryGenerate = sysSetts.rsuSysLogQueryGenerate; 
          ntcip_1218_mib.rsuSysLogQueryStatus = sysSetts.rsuSysLogQueryStatus;
          ntcip_1218_mib.rsuSysLogCloseCommand = sysSetts.rsuSysLogCloseCommand; 
          ntcip_1218_mib.rsuSysLogSeverity = sysSetts.rsuSysLogSeverity;
          memcpy(ntcip_1218_mib.rsuSysConfigId, sysSetts.rsuSysConfigId, RSU_SYS_CONFIG_ID_LENGTH_MAX);
          ntcip_1218_mib.rsuSysConfigId_length = sysSetts.rsuSysConfigId_length;
          ntcip_1218_mib.rsuSysRetries = sysSetts.rsuSysRetries;
          ntcip_1218_mib.rsuSysRetryPeriod = sysSetts.rsuSysRetryPeriod;
          ntcip_1218_mib.rsuSysRetryPeriodStart = sysSetts.rsuSysRetryPeriodStart;
          ntcip_1218_mib.rsuSysRetryAttempts = sysSetts.rsuSysRetryAttempts;
          if((0 == ntcip_1218_mib.rsuSysRetries) || (0 == ntcip_1218_mib.rsuSysRetryPeriod)) {
              ntcip_1218_mib.rsuSysRetryPeriodStart = 0; /* Shouldn't need to clear but just in case. */
              ntcip_1218_mib.rsuSysRetryAttempts = 0;
          } 
          ntcip_1218_mib.rsuShortCommLossTime = sysSetts.rsuShortCommLossTime;
          ntcip_1218_mib.rsuLongCommLossTime = sysSetts.rsuLongCommLossTime;
          memcpy(ntcip_1218_mib.rsuSysLogName, sysSetts.rsuSysLogName, RSU_SYSLOG_NAME_AND_PATH_LENGTH_MAX);
          ntcip_1218_mib.rsuSysLogName_length = sysSetts.rsuSysLogName_length;
          memcpy(ntcip_1218_mib.rsuSysDir, sysSetts.rsuSysDir, RSU_SYSDIR_NAME_LENGTH_MAX);
          ntcip_1218_mib.rsuSysDir_length = sysSetts.rsuSysDir_length;
          ntcip_1218_mib.rsuLongCommLossReboot = sysSetts.rsuLongCommLossReboot;
          memcpy(ntcip_1218_mib.rsuHostIpAddr, sysSetts.rsuHostIpAddr, RSU_DEST_IP_MAX);
          ntcip_1218_mib.rsuHostIpAddr_length = sysSetts.rsuHostIpAddr_length;
          memcpy(ntcip_1218_mib.rsuHostNetMask, sysSetts.rsuHostNetMask, RSU_DEST_IP_MAX);
          ntcip_1218_mib.rsuHostNetMask_length = sysSetts.rsuHostNetMask_length;
          memcpy(ntcip_1218_mib.rsuHostGateway, sysSetts.rsuHostGateway, RSU_DEST_IP_MAX);
          ntcip_1218_mib.rsuHostGateway_length = sysSetts.rsuHostGateway_length;
          memcpy(ntcip_1218_mib.rsuHostDNS, sysSetts.rsuHostDNS, RSU_DEST_IP_MAX);
          ntcip_1218_mib.rsuHostDNS_length = sysSetts.rsuHostDNS_length;
          ntcip_1218_mib.rsuHostDHCPEnable = sysSetts.rsuHostDHCPEnable;
      } else {
          DEBUGMSGTL((MY_NAME, "restore_rsuSysSettings_from_disk: failed wellness check:version=%d ret=%d:0x%x:0x%x.\n",sysSetts.version, ret,
                sysSetts.crc, (uint32_t)crc16((const uint8_t *)&sysSetts, sizeof(sysSetts) - 4) ));
          ret = RSEMIB_BAD_RECORD_RESTORE; /* File will be over written next time row is used by client. Leave in place for now.*/
      }
  }
  if(RSEMIB_FILE_STAT_FAIL == ret) { /* Not an error but informative warning. */
      DEBUGMSGTL((MY_NAME, "restore_rsuSysSettings_from_disk: sysSetts.dat does not exist.\n")); 
      ret = RSEMIB_OK; /* Ok for files to not exist.*/
  }
  if(RSEMIB_OK != ret){
      set_ntcip_1218_error_states(ret);
  }
}
static void init_rsuSysSettings(void)
{
  memset(ntcip_1218_mib.rsuNotifyIpAddress, 0x0, RSU_DEST_IP_MAX);
  ntcip_1218_mib.rsuNotifyIpAddress_length = 0;

  ntcip_1218_mib.rsuNotifyPort = RSU_NOTIFY_PORT_DEFAULT;

  memset(ntcip_1218_mib.rsuSysLogQueryStart, 0x0, RFC2579_DATEANDTIME_LENGTH);
  memset(ntcip_1218_mib.rsuSysLogQueryStop, 0x0, RFC2579_DATEANDTIME_LENGTH); 

  /* 
     default start:
       07e6-01-01:00:00:00.1
       2022-01-01:00:00:00.1
 
     default stop:
       0833-0c-1f-17:3b:3b.9
       2099-12-31:23:59:59.9
  */
  ntcip_1218_mib.rsuSysLogQueryStart[0] = 0x07;
  ntcip_1218_mib.rsuSysLogQueryStart[1] = 0xe6;
  ntcip_1218_mib.rsuSysLogQueryStart[2] = 0x01;
  ntcip_1218_mib.rsuSysLogQueryStart[3] = 0x01;
  ntcip_1218_mib.rsuSysLogQueryStart[4] = 0x00;
  ntcip_1218_mib.rsuSysLogQueryStart[5] = 0x00;
  ntcip_1218_mib.rsuSysLogQueryStart[6] = 0x00;
  ntcip_1218_mib.rsuSysLogQueryStart[7] = 0x01;

  ntcip_1218_mib.rsuSysLogQueryStop[0] = 0x08;
  ntcip_1218_mib.rsuSysLogQueryStop[1] = 0x33;
  ntcip_1218_mib.rsuSysLogQueryStop[2] = 0x0c;
  ntcip_1218_mib.rsuSysLogQueryStop[3] = 0x1f;
  ntcip_1218_mib.rsuSysLogQueryStop[4] = 0x17;
  ntcip_1218_mib.rsuSysLogQueryStop[5] = 0x3b;
  ntcip_1218_mib.rsuSysLogQueryStop[6] = 0x3b;
  ntcip_1218_mib.rsuSysLogQueryStop[7] = 0x09;

  ntcip_1218_mib.rsuSysLogQueryPriority = syslog_level_error;

  /* I2V conf will turn off by conf. Assume on to start to capture boot data. */
  ntcip_1218_mib.rsuSysLogQueryGenerate = 1;
  ntcip_1218_mib.rsuSysLogQueryStatus = syslog_status_unknown;
  ntcip_1218_mib.rsuSysLogCloseCommand = 0; 
  ntcip_1218_mib.rsuSysLogSeverity = syslog_level_error;

  memset(ntcip_1218_mib.rsuSysConfigId, 0x0, RSU_SYS_CONFIG_ID_LENGTH_MAX);
  ntcip_1218_mib.rsuSysConfigId_length = RSU_SYS_CONFIG_ID_LENGTH_MIN;

  ntcip_1218_mib.rsuSysRetries = RSU_SYS_RETRIES_MAX;
  ntcip_1218_mib.rsuSysRetryPeriod = RSU_SYS_RETRY_PERIOD_MIN;
  ntcip_1218_mib.rsuSysRetryPeriodStart = 0;
  ntcip_1218_mib.rsuSysRetryAttempts = 0;
  ntcip_1218_mib.rsuShortCommLossTime = 0;
  ntcip_1218_mib.rsuLongCommLossTime = RSU_LONG_COMM_LOSS_TIME_MIN;

  memset(ntcip_1218_mib.rsuSysLogName, 0x0, RSU_SYSLOG_NAME_AND_PATH_LENGTH_MAX);
  strncpy((char_t *)ntcip_1218_mib.rsuSysLogName, RSU_SYSLOG_NAME_AND_PATH_DEFAULT, RSU_SYSLOG_NAME_AND_PATH_LENGTH_MAX);
  ntcip_1218_mib.rsuSysLogName_length = strlen(RSU_SYSLOG_NAME_AND_PATH_DEFAULT);

  memset(ntcip_1218_mib.rsuSysDir, 0x0, RSU_SYSDIR_NAME_LENGTH_MAX);
  strncpy((char_t *)ntcip_1218_mib.rsuSysDir, RSU_SYSDIR_DEFAULT, RSU_SYSDIR_NAME_LENGTH_MAX);
  ntcip_1218_mib.rsuSysDir_length = strlen(RSU_SYSDIR_DEFAULT);

  ntcip_1218_mib.rsuLongCommLossReboot = 0;
                                    
  memset(ntcip_1218_mib.rsuHostIpAddr , 0x0, RSU_DEST_IP_MAX);
  memset(ntcip_1218_mib.rsuHostNetMask, 0x0, RSU_DEST_IP_MAX);
  memset(ntcip_1218_mib.rsuHostGateway, 0x0, RSU_DEST_IP_MAX);
  memset(ntcip_1218_mib.rsuHostDNS    , 0x0, RSU_DEST_IP_MAX);
  ntcip_1218_mib.rsuHostIpAddr_length  = 0x0;
  ntcip_1218_mib.rsuHostNetMask_length = 0x0;
  ntcip_1218_mib.rsuHostGateway_length = 0x0;
  ntcip_1218_mib.rsuHostDNS_length     = 0x0;

  ntcip_1218_mib.rsuHostDHCPEnable = 0;

  /* If it doesn't exist then create it. If we fail to create keep going. */
  if (RSEMIB_OK != mibUtilMakeDir(SYS_SETTS_OUTPUT_LOCATION)) {
      set_ntcip_1218_error_states(RSEMIB_CREATE_DIR_FAIL);
      DEBUGMSGTL((MY_NAME, "init_rsuSysSettings: Couldn't create SYS SETTS LOGS dir[%s]\n",SYS_SETTS_OUTPUT_LOCATION));
  } else { /* Restore IFACE LOG from disk. If it fails keep going. */
      restore_rsuSysSettings_from_disk();
  }
}

/* 5.16.1 - 5.16.2 */
void set_default_row_rsuAntenna(RsuAntennaEntry_t * rsuAntenna, int32_t index)
{
  char_t  command_buffer[RSU_MSG_FILENAME_LENGTH_MAX];

  index++;
  if((NULL == rsuAntenna) || (index < RSU_ANTENNA_MIN) || (RSU_ANTENNA_MAX < index)){
      return;
  }
  rsuAntenna->rsuAntennaIndex = index;
  rsuAntenna->rsuAntLat       = RSU_LAT_UNKNOWN; /* everything below is default == unknown. */
  rsuAntenna->rsuAntLong      = RSU_LON_UNKNOWN;
  rsuAntenna->rsuAntElv       = RSU_ELV_UNKNOWN;
  rsuAntenna->rsuAntGain      = RSU_ANTENNA_GAIN_MIN;
  rsuAntenna->rsuAntDirection = RSU_ANTENNA_DIRECTION_MAX;
  rsuAntenna->version         = RSU_ANTENNA_VERSION;
  rsuAntenna->status          = ROW_STATUS_VALID_DEFAULT;
  rsuAntenna->reserved        = 0x0;
  rsuAntenna->crc             = 0x0;
  memset(command_buffer,'\0',sizeof(command_buffer));
  sprintf(command_buffer,"%s/%d.dat",ANTENNA_OUTPUT_PATH, rsuAntenna->rsuAntennaIndex); /* Give row a file name based off index. Binary file so name it so. */
  memcpy(&rsuAntenna->filePathName, command_buffer, RSU_MSG_FILENAME_LENGTH_MAX);
}
static void restore_rsuAntenna_from_disk(void)
{
  FILE    * file_in        = NULL;
  uint32_t  mib_byte_count = 0;
  int32_t   ret            = RSEMIB_OK; /* SUCCESS till proven false. */
  RsuAntennaEntry_t rsuAntenna;
  char_t   filePathName[RSU_MSG_FILENAME_LENGTH_MAX];
  int32_t  i;

  mib_byte_count = sizeof(rsuAntenna);
  memset(filePathName,'\0',RSU_MSG_FILENAME_LENGTH_MAX);
  for(i=0; i < RSU_ANTENNA_MAX; i++) {
      /* file name always based off index. */
      snprintf(filePathName,RSU_MSG_FILENAME_LENGTH_MAX, "%s/%d.dat",ANTENNA_OUTPUT_PATH , i + 1);
      /* It's ok for the file to not exist. */
      if(RSEMIB_OK == check_file_existence(filePathName, mib_byte_count)){
          /* Open file for input.*/
          if ((RSEMIB_OK == ret) && (NULL == (file_in = fopen(filePathName, "rb"))) ){
              DEBUGMSGTL((MY_NAME, "restore_rsuAntenna_from_disk: fopen fail.\n"));
              ret = RSEMIB_FOPEN_FAIL;
          }
          memset(&rsuAntenna,0x0,sizeof(rsuAntenna));
          /* Load file into tmp ram space first before overwriting. */
          if((RSEMIB_OK == ret) && (1 != fread((uint8_t *)&rsuAntenna, mib_byte_count, 1, file_in)) ){    
              DEBUGMSGTL((MY_NAME, "restore_rsuAntenna_from_disk: FAIL: read_count is 0: mib_byte_count=(0x%X) errno=(%d:%s).\n",mib_byte_count,errno,strerror(errno) ));
              ret = RSEMIB_BAD_DATA;
          }
          /* One way or another done with file so close. */
          if(NULL != file_in){
              fclose(file_in);
          }
          /* Check wellness and copy over if good.*/
          if(RSEMIB_OK == ret){
              if(   (rsuAntenna.crc == (uint32_t)crc16((const uint8_t *)&rsuAntenna, sizeof(rsuAntenna) - 4))
                 && (RSU_ANTENNA_VERSION == rsuAntenna.version)) { 
                  memcpy(&ntcip_1218_mib.rsuAntennaTable[i], &rsuAntenna, sizeof(rsuAntenna));
              } else {
                  DEBUGMSGTL((MY_NAME, "restore_rsuAntenna_from_disk: index=%d failed wellness check.\n",i));
                  ret = RSEMIB_BAD_RECORD_RESTORE; /* File will be over written next time row is used by client. Leave in place for now.*/
              }
          }
      }
  }
  if(RSEMIB_OK != ret){
      set_ntcip_1218_error_states(ret);
  }
}

static void init_rsuAntenna(void)
{
  int32_t i;

  ntcip_1218_mib.maxRsuAntennas = RSU_ANTENNA_MAX;  
  for(i=0;i<RSU_ANTENNA_MAX;i++){
      set_default_row_rsuAntenna(&ntcip_1218_mib.rsuAntennaTable[i],i);
  }
  /* If it doesn't exist then create it. If we fail to create keep going. */
  if (RSEMIB_OK != mibUtilMakeDir(ANTENNA_OUTPUT_PATH)) {
      set_ntcip_1218_error_states(RSEMIB_CREATE_DIR_FAIL);
      DEBUGMSGTL((MY_NAME, "init_rsuAntennas: Couldn't create ANTENNA dir[%s]\n",ANTENNA_OUTPUT_PATH));
  } else { /* Restore ANTENNA from disk. If it fails keep going. */
      restore_rsuAntenna_from_disk();
  }
}

/* 5.17.1 - 5.17.10 */
static void init_rsuSysStatus(void)
{
  ntcip_1218_mib.rsuChanStatus              = chanStat_contOp;
  ntcip_1218_mib.rsuMode                    = rsuMode_operate;
  ntcip_1218_mib.rsuModeStatus              = rsuModeStatus_operate;
  ntcip_1218_mib.rsuReboot                  = 0;
  ntcip_1218_mib.rsuClockSource             = clockSource_gnss;
  ntcip_1218_mib.rsuClockSourceStatus       = clockSourceStatus_active;
  ntcip_1218_mib.rsuClockSourceTimeout      = RSU_CLOCK_SOURCE_TIMEOUT_DEFAULT;
  ntcip_1218_mib.rsuClockSourceFailedQuery  = RSU_CLOCK_SOURCE_FAILED_QUERY_MIN;
  ntcip_1218_mib.rsuClockDeviationTolerance = RSU_CLOCK_DEVIATION_MIN;
  ntcip_1218_mib.rsuStatus                  = rsuStatus_okay;
}

/* 5.18.1 - 5.18.4 */
#if defined(ENABLE_ASYNC_BACKUP)
static void restore_rsuAsync_from_disk(void)
{
  FILE    * file_in        = NULL;
  uint32_t  mib_byte_count = 0;
  int32_t   ret            = RSEMIB_OK; /* SUCCESS till proven false. */
  rsuAsync_t rsuAsync;
  char_t   filePathName[RSU_MSG_FILENAME_LENGTH_MAX];

  mib_byte_count = sizeof(rsuAsync);
  memset(filePathName,'\0',RSU_MSG_FILENAME_LENGTH_MAX);

  /* file name always based off index. */
  snprintf(filePathName,RSU_MSG_FILENAME_LENGTH_MAX, "%s/%s",ASYNC_ACTIVE_DIR,ASYNC_FILENAME);
  /* It's ok for the file to not exist. */
  if(RSEMIB_OK == check_file_existence(filePathName, mib_byte_count)){
      /* Open file for input.*/
      if ((RSEMIB_OK == ret) && (NULL == (file_in = fopen(filePathName, "rb"))) ){
          DEBUGMSGTL((MY_NAME, "restore_rsuAsync_from_disk: fopen fail.\n"));
          ret = RSEMIB_FOPEN_FAIL;
      }
      memset(&rsuAsync,0x0,sizeof(rsuAsync));
      /* Load file into tmp ram space first before overwriting. */
      if((RSEMIB_OK == ret) && (1 != fread((uint8_t *)&rsuAsync, mib_byte_count, 1, file_in)) ){    
          DEBUGMSGTL((MY_NAME, "restore_rsuAsync_from_disk: FAIL: read_count is 0: mib_byte_count=(0x%X) errno=(%d:%s).\n",mib_byte_count,errno,strerror(errno) ));
          ret = RSEMIB_BAD_DATA;
      }
      /* One way or another done with file so close. */
      if(NULL != file_in){
          fclose(file_in);
      }
      /* Check wellness and copy over if good.*/
      if(RSEMIB_OK == ret){
          if(   (rsuAsync.crc == (uint32_t)crc16((const uint8_t *)&rsuAsync, sizeof(rsuAsync) - 4))
             && (RSU_ASYNC_VERSION == rsuAsync.version)) { 
              memcpy(&ntcip_1218_mib.Async, &rsuAsync, sizeof(rsuAsync));
          } else {
              DEBUGMSGTL((MY_NAME, "restore_rsuAsync_from_disk: index=0 failed wellness check.\n"));
              ret = RSEMIB_BAD_RECORD_RESTORE; /* File will be over written next time row is used by client. Leave in place for now.*/
          }
      }
  }
  if(RSEMIB_OK != ret){
      set_ntcip_1218_error_states(ret);
  }
}
#endif
static void init_rsuAsync(void)
{
  ntcip_1218_mib.Async.rsuTrapEnable = 0; /* hidden: DENSO: 1 = enable, 0 = disable. Not part of MIB: default = disable.*/

  ntcip_1218_mib.Async.messageFileIntegrityError = RSU_ALERT_DEFAULT;
  strncpy((char_t *)ntcip_1218_mib.Async.rsuMsgFileIntegrityMsg,RSU_ALERT_MSG_DEFAULT,RSU_ALERT_MSG_LENGTH_MAX);
  ntcip_1218_mib.Async.rsuMsgFileIntegrity_length = strlen(RSU_ALERT_MSG_DEFAULT);

  ntcip_1218_mib.Async.rsuSecStorageIntegrityError = RSU_ALERT_DEFAULT;
  strncpy((char_t *)ntcip_1218_mib.Async.rsuSecStorageIntegrityMsg,RSU_ALERT_MSG_DEFAULT,RSU_ALERT_MSG_LENGTH_MAX);
  ntcip_1218_mib.Async.rsuSecStorageIntegrity_length = strlen(RSU_ALERT_MSG_DEFAULT);

  ntcip_1218_mib.Async.rsuAuthError = RSU_ALERT_DEFAULT;
  strncpy((char_t *)ntcip_1218_mib.Async.rsuAuthMsg,RSU_ALERT_MSG_DEFAULT,RSU_ALERT_MSG_LENGTH_MAX);
  ntcip_1218_mib.Async.rsuAuth_length = strlen(RSU_ALERT_MSG_DEFAULT);

  ntcip_1218_mib.Async.rsuSignatureVerifyError = RSU_ALERT_DEFAULT;
  strncpy((char_t *)ntcip_1218_mib.Async.rsuSignatureVerifyMsg,RSU_ALERT_MSG_DEFAULT,RSU_ALERT_MSG_LENGTH_MAX);
  ntcip_1218_mib.Async.rsuSignatureVerify_length = strlen(RSU_ALERT_MSG_DEFAULT);

  ntcip_1218_mib.Async.rsuAccessError = RSU_ALERT_DEFAULT;
  strncpy((char_t *)ntcip_1218_mib.Async.rsuAccessMsg,RSU_ALERT_MSG_DEFAULT,RSU_ALERT_MSG_LENGTH_MAX);
  ntcip_1218_mib.Async.rsuAccess_length = strlen(RSU_ALERT_MSG_DEFAULT);

  ntcip_1218_mib.Async.rsuTimeSourceLost = RSU_ALERT_DEFAULT;
  strncpy((char_t *)ntcip_1218_mib.Async.rsuTimeSourceLostMsg,RSU_ALERT_MSG_DEFAULT,RSU_ALERT_MSG_LENGTH_MAX);
  ntcip_1218_mib.Async.rsuTimeSourceLost_length = strlen(RSU_ALERT_MSG_DEFAULT);

  ntcip_1218_mib.Async.rsuTimeSourceMismatch = RSU_ALERT_DEFAULT;
  strncpy((char_t *)ntcip_1218_mib.Async.rsuTimeSourceMismatchMsg,RSU_ALERT_MSG_DEFAULT,RSU_ALERT_MSG_LENGTH_MAX);
  ntcip_1218_mib.Async.rsuTimeSourceMismatch_length = strlen(RSU_ALERT_MSG_DEFAULT);

  ntcip_1218_mib.Async.rsuGnssAnomaly = RSU_ALERT_DEFAULT;
  strncpy((char_t *)ntcip_1218_mib.Async.rsuGnssAnomalyMsg,RSU_ALERT_MSG_DEFAULT,RSU_ALERT_MSG_LENGTH_MAX);
  ntcip_1218_mib.Async.rsuGnssAnomaly_length = strlen(RSU_ALERT_MSG_DEFAULT);

  ntcip_1218_mib.Async.rsuGnssDeviationError = RSU_ALERT_DEFAULT;
  strncpy((char_t *)ntcip_1218_mib.Async.rsuGnssDeviationMsg,RSU_ALERT_MSG_DEFAULT,RSU_ALERT_MSG_LENGTH_MAX);
  ntcip_1218_mib.Async.rsuGnssDeviation_length = strlen(RSU_ALERT_MSG_DEFAULT);

 /*
  * 5.18.1.10 GNSS NMEA Message:NMEA 0183 string (including the $ starting character and the ending <CR><LF>)
  *
  *     On alert periodically send this: rsuGnssOutputString[];
  */
  ntcip_1218_mib.Async.rsuGnssNmeaNotify = RSU_ALERT_DEFAULT;
  ntcip_1218_mib.Async.rsuGnssNmeaNotifyInterval  = RSU_NOTIFY_INTERVAL_RATE_MIN;

  ntcip_1218_mib.Async.rsuCertificateError = RSU_ALERT_DEFAULT;
  strncpy((char_t *)ntcip_1218_mib.Async.rsuCertificateMsg,RSU_ALERT_MSG_DEFAULT,RSU_ALERT_MSG_LENGTH_MAX);
  ntcip_1218_mib.Async.rsuCertificate_length = strlen(RSU_ALERT_MSG_DEFAULT);

  ntcip_1218_mib.Async.rsuServiceDenialError = RSU_ALERT_DEFAULT;
  strncpy((char_t *)ntcip_1218_mib.Async.rsuServiceDenialMsg,RSU_ALERT_MSG_DEFAULT,RSU_ALERT_MSG_LENGTH_MAX);
  ntcip_1218_mib.Async.rsuServiceDenial_length = strlen(RSU_ALERT_MSG_DEFAULT);

  ntcip_1218_mib.Async.rsuWatchdogError = RSU_ALERT_DEFAULT;
  strncpy((char_t *)ntcip_1218_mib.Async.rsuWatchdogMsg,RSU_ALERT_MSG_DEFAULT,RSU_ALERT_MSG_LENGTH_MAX);
  ntcip_1218_mib.Async.rsuWatchdog_length = strlen(RSU_ALERT_MSG_DEFAULT);

  ntcip_1218_mib.Async.rsuEnvironError = RSU_ALERT_DEFAULT;
  strncpy((char_t *)ntcip_1218_mib.Async.rsuEnvironMsg,RSU_ALERT_MSG_DEFAULT,RSU_ALERT_MSG_LENGTH_MAX);
  ntcip_1218_mib.Async.rsuEnviron_length = strlen(RSU_ALERT_MSG_DEFAULT);

  ntcip_1218_mib.Async.rsuNotificationRepeatInterval = RSU_NOTIFY_REPEAT_RATE_MIN;
  ntcip_1218_mib.Async.rsuNotificationMaxRetries     = RSU_NOTIFY_MSG_RETRY_MIN;

  ntcip_1218_mib.Async.version = RSU_ASYNC_VERSION;
  ntcip_1218_mib.Async.reserved = 0x0;
  ntcip_1218_mib.Async.crc = 0x0;

#if defined(ENABLE_ASYNC_BACKUP)
{
  char_t command_buffer[RSU_MSG_FILENAME_LENGTH_MAX];
  memset(command_buffer,'\0',sizeof(command_buffer));
  snprintf(command_buffer,sizeof(command_buffer),"%s/%s",ASYNC_ACTIVE_DIR, ASYNC_FILENAME); /* Give row a file name based off index. Binary file so name it so. */
  memcpy(ntcip_1218_mib.Async.filePathName, command_buffer, RSU_MSG_FILENAME_LENGTH_MAX);
  /* If it doesn't exist then create it. If we fail to create keep going. */
  if (RSEMIB_OK != mibUtilMakeDir(ASYNC_ACTIVE_DIR)) {
      set_ntcip_1218_error_states(RSEMIB_CREATE_DIR_FAIL);
      DEBUGMSGTL((MY_NAME, "init_rsuAsync: Couldn't create ASYNC dir[%s]\n",ASYNC_ACTIVE_DIR));
  } else { /* Restore ASYNC from disk. If it fails keep going. */
      restore_rsuAsync_from_disk();
  }
}
#else
  /* If it doesn't exist then create it. If we fail to create keep going. */
  if (RSEMIB_OK != mibUtilMakeDir(ASYNC_ACTIVE_DIR)) {
      set_ntcip_1218_error_states(RSEMIB_CREATE_DIR_FAIL);
      DEBUGMSGTL((MY_NAME, "init_rsuAsync: Couldn't create ASYNC dir[%s]\n",ASYNC_ACTIVE_DIR));
      I2V_DBG_LOG(LEVEL_WARN,MY_NAME,"init_rsuAsync: Couldn't create ASYNC dir[%s]\n",ASYNC_ACTIVE_DIR);
  } 
#endif
}

/*  5.19.1 - 5.19.2 */
static void init_rsuAppConfig(void)
{
  int32_t i = 0;

  ntcip_1218_mib.maxRsuApps = RSU_APPS_RUNNING_MAX;

  for(i = 0; i < RSU_APPS_RUNNING_MAX; i++){
      ntcip_1218_mib.rsuAppConfigTable[i].rsuAppConfigID = i + 1; /* Set and forget. */

      memset(ntcip_1218_mib.rsuAppConfigTable[i].rsuAppConfigName,0x0,RSU_APP_CONF_NAME_LENGTH_MAX);
      ntcip_1218_mib.rsuAppConfigTable[i].rsuAppConfigName[0] = 'X'; /* Has to be min length 1. */
      ntcip_1218_mib.rsuAppConfigTable[i].rsuAppConfigName_length = RSU_APP_CONF_NAME_LENGTH_MIN;

      ntcip_1218_mib.rsuAppConfigTable[i].rsuAppConfigStartup = rsuAppConfigStartup_other; 
      ntcip_1218_mib.rsuAppConfigTable[i].rsuAppConfigStart = 1; 
      ntcip_1218_mib.rsuAppConfigTable[i].rsuAppConfigStop = 0;
  }
}

/*  5.20.1 - 5.20.2 */
static void init_rsuService(void)
{
  int32_t i = 0;

  ntcip_1218_mib.maxRsuServices = RSU_SERVICES_RUNNING_MAX;

  /* Set to empty defaults. */
  for(i = 0; i < RSU_SERVICES_RUNNING_MAX; i++){
      ntcip_1218_mib.rsuServiceTable[i].rsuServiceID = i + 1; /* Set and forget. */

      strncpy((char_t *)ntcip_1218_mib.rsuServiceTable[i].rsuServiceName, "nullService", RSU_SERVICE_NAME_LENGTH_MAX);
      ntcip_1218_mib.rsuServiceTable[i].rsuServiceName_length = strlen("nullService");

      ntcip_1218_mib.rsuServiceTable[i].rsuServiceStatus = rsuServiceStatus_other;

      strncpy((char_t *)ntcip_1218_mib.rsuServiceTable[i].rsuServiceStatusDesc, "nullDesc", RSU_SERVICE_STATUS_DESC_LENGTH_MAX); 
      ntcip_1218_mib.rsuServiceTable[i].rsuServiceStatusDesc_length = strlen("nullDesc");

      memset(ntcip_1218_mib.rsuServiceTable[i].rsuServiceStatusTime, 0x0, MIB_DATEANDTIME_LENGTH);
  }

  /* These are defined services already. */
  strncpy((char_t *)ntcip_1218_mib.rsuServiceTable[0].rsuServiceName, "RSU system", RSU_SERVICE_NAME_LENGTH_MAX); 
  strncpy((char_t *)ntcip_1218_mib.rsuServiceTable[0].rsuServiceStatusDesc," system services on the RSU", RSU_SERVICE_STATUS_DESC_LENGTH_MAX); 

  ntcip_1218_mib.rsuServiceTable[0].rsuServiceName_length = strlen("RSU system");
  ntcip_1218_mib.rsuServiceTable[0].rsuServiceStatusDesc_length = strlen("system services on the RSU");

  strncpy((char_t *)ntcip_1218_mib.rsuServiceTable[1].rsuServiceName,"GNSS",RSU_SERVICE_NAME_LENGTH_MAX);
  strncpy((char_t *)ntcip_1218_mib.rsuServiceTable[1].rsuServiceStatusDesc,"GNSS services",RSU_SERVICE_STATUS_DESC_LENGTH_MAX); 

  ntcip_1218_mib.rsuServiceTable[1].rsuServiceName_length = strlen("GNSS");
  ntcip_1218_mib.rsuServiceTable[1].rsuServiceStatusDesc_length = strlen("GNSS services");

  strncpy((char_t *)ntcip_1218_mib.rsuServiceTable[2].rsuServiceName,"Time source",RSU_SERVICE_NAME_LENGTH_MAX);
  strncpy((char_t *)ntcip_1218_mib.rsuServiceTable[2].rsuServiceStatusDesc,"clock sources",RSU_SERVICE_STATUS_DESC_LENGTH_MAX);

  ntcip_1218_mib.rsuServiceTable[2].rsuServiceName_length = strlen("Time source");
  ntcip_1218_mib.rsuServiceTable[2].rsuServiceStatusDesc_length = strlen("clock sources");

  strncpy((char_t *)ntcip_1218_mib.rsuServiceTable[3].rsuServiceName,"Storage",RSU_SERVICE_NAME_LENGTH_MAX);
  strncpy((char_t *)ntcip_1218_mib.rsuServiceTable[3].rsuServiceStatusDesc,"file and log storage available",RSU_SERVICE_STATUS_DESC_LENGTH_MAX);

  ntcip_1218_mib.rsuServiceTable[3].rsuServiceName_length = strlen("Storage");
  ntcip_1218_mib.rsuServiceTable[3].rsuServiceStatusDesc_length = strlen("file and log storage available");

  strncpy((char_t *)ntcip_1218_mib.rsuServiceTable[4].rsuServiceName,"Firewall",RSU_SERVICE_NAME_LENGTH_MAX);
  strncpy((char_t *)ntcip_1218_mib.rsuServiceTable[4].rsuServiceStatusDesc,"access control services",RSU_SERVICE_STATUS_DESC_LENGTH_MAX); 

  ntcip_1218_mib.rsuServiceTable[4].rsuServiceName_length = strlen("Firewall");
  ntcip_1218_mib.rsuServiceTable[4].rsuServiceStatusDesc_length = strlen("access control services");

  strncpy((char_t *)ntcip_1218_mib.rsuServiceTable[5].rsuServiceName,"Network",RSU_SERVICE_NAME_LENGTH_MAX);
  strncpy((char_t *)ntcip_1218_mib.rsuServiceTable[5].rsuServiceStatusDesc,"network services",RSU_SERVICE_STATUS_DESC_LENGTH_MAX); 

  ntcip_1218_mib.rsuServiceTable[5].rsuServiceName_length = strlen("Network");
  ntcip_1218_mib.rsuServiceTable[5].rsuServiceStatusDesc_length = strlen("network services");

  strncpy((char_t *)ntcip_1218_mib.rsuServiceTable[6].rsuServiceName,"Layers 1 and 2",RSU_SERVICE_NAME_LENGTH_MAX);
  strncpy((char_t *)ntcip_1218_mib.rsuServiceTable[6].rsuServiceStatusDesc,"radio access technology",RSU_SERVICE_STATUS_DESC_LENGTH_MAX);

  ntcip_1218_mib.rsuServiceTable[6].rsuServiceName_length = strlen("Layers 1 and 2");
  ntcip_1218_mib.rsuServiceTable[6].rsuServiceStatusDesc_length = strlen("radio access technology");

  strncpy((char_t *)ntcip_1218_mib.rsuServiceTable[7].rsuServiceName,"Layers 3 and 4",RSU_SERVICE_NAME_LENGTH_MAX);
  strncpy((char_t *)ntcip_1218_mib.rsuServiceTable[7].rsuServiceStatusDesc,"networking services",RSU_SERVICE_STATUS_DESC_LENGTH_MAX);

  ntcip_1218_mib.rsuServiceTable[7].rsuServiceName_length = strlen("Layers 3 and 4");
  ntcip_1218_mib.rsuServiceTable[7].rsuServiceStatusDesc_length = strlen("networking services");

  strncpy((char_t *)ntcip_1218_mib.rsuServiceTable[8].rsuServiceName,"Security",RSU_SERVICE_NAME_LENGTH_MAX);
  strncpy((char_t *)ntcip_1218_mib.rsuServiceTable[8].rsuServiceStatusDesc,"security services",RSU_SERVICE_STATUS_DESC_LENGTH_MAX);

  ntcip_1218_mib.rsuServiceTable[8].rsuServiceName_length = strlen("Security");
  ntcip_1218_mib.rsuServiceTable[8].rsuServiceStatusDesc_length = strlen("security services");

  strncpy((char_t *)ntcip_1218_mib.rsuServiceTable[9].rsuServiceName,"SCMS",RSU_SERVICE_NAME_LENGTH_MAX);
  strncpy((char_t *)ntcip_1218_mib.rsuServiceTable[9].rsuServiceStatusDesc,"SCMS servicess",RSU_SERVICE_STATUS_DESC_LENGTH_MAX);

  ntcip_1218_mib.rsuServiceTable[9].rsuServiceName_length = strlen("SCMS");
  ntcip_1218_mib.rsuServiceTable[9].rsuServiceStatusDesc_length = strlen("SCMS servicess");

  /* Reserved for future definition by community. */
  for(i=10;(i<16) && (i < RSU_SERVICES_RUNNING_MAX);i++){
      strncpy((char_t *)ntcip_1218_mib.rsuServiceTable[i].rsuServiceName,"RESERVED",RSU_SERVICE_NAME_LENGTH_MAX);
      strncpy((char_t *)ntcip_1218_mib.rsuServiceTable[i].rsuServiceStatusDesc,"reserved for future services",RSU_SERVICE_STATUS_DESC_LENGTH_MAX);

      ntcip_1218_mib.rsuServiceTable[i].rsuServiceName_length = strlen("RESERVED");
      ntcip_1218_mib.rsuServiceTable[i].rsuServiceStatusDesc_length = strlen("reserved for future services");
  }
  
  /* Vendor specific services to be defined. */
  for(i = i; i < RSU_SERVICES_RUNNING_MAX; i++){
      strncpy((char_t *)ntcip_1218_mib.rsuServiceTable[i].rsuServiceName,"vendor-specific",RSU_SERVICE_NAME_LENGTH_MAX);
      strncpy((char_t *)ntcip_1218_mib.rsuServiceTable[i].rsuServiceStatusDesc,"for vendor-specific services",RSU_SERVICE_STATUS_DESC_LENGTH_MAX);

      ntcip_1218_mib.rsuServiceTable[i].rsuServiceName_length = strlen("vendor-specific");
      ntcip_1218_mib.rsuServiceTable[i].rsuServiceStatusDesc_length = strlen("for vendor-specific services");
  } 
}

/* 5.21.1 - 5.21.2 */
void set_default_row_rsuXmitMsgFwding(RsuXmitMsgFwdingEntry_t * rsuXmitMsgFwding, int32_t index)
{
  index++;
  if((NULL == rsuXmitMsgFwding) || (index < RSU_XMIT_MSG_COUNT_MIN) || (RSU_XMIT_MSG_COUNT_MAX< index)){
      return;
  }
  rsuXmitMsgFwding->rsuXmitMsgFwdingIndex = index;
  memset(&rsuXmitMsgFwding->rsuXmitMsgFwdingPsid,0x0,RSU_RADIO_PSID_SIZE);
  rsuXmitMsgFwding->rsuXmitMsgFwdingPsid_length = 1;
  memset(&rsuXmitMsgFwding->rsuXmitMsgFwdingDestIpAddr,0x0,RSU_DEST_IP_MAX);
  rsuXmitMsgFwding->rsuXmitMsgFwdingDestIpAddr_length = 0;
  rsuXmitMsgFwding->rsuXmitMsgFwdingDestPort = RSU_XMIT_MSG_PORT_MIN;
  rsuXmitMsgFwding->rsuXmitMsgFwdingProtocol = protocol_udp;
  memset(&rsuXmitMsgFwding->rsuXmitMsgFwdingDeliveryStart, 0x0, MIB_DATEANDTIME_LENGTH);
  memset(&rsuXmitMsgFwding->rsuXmitMsgFwdingDeliveryStop, 0x0, MIB_DATEANDTIME_LENGTH);
  rsuXmitMsgFwding->rsuXmitMsgFwdingSecure = RSU_XMIT_MSG_SECURE_MIN;
  rsuXmitMsgFwding->rsuXmitMsgFwdingStatus = ROW_STATUS_VALID_DEFAULT;

}
static void init_rsuXmitMsgFwding(void)
{
  int32_t i = 0;

  ntcip_1218_mib.maxXmitMsgFwding = RSU_XMIT_MSG_COUNT_MAX;
  for(i = 0; i < RSU_XMIT_MSG_COUNT_MAX; i++){
      memset(&ntcip_1218_mib.rsuXmitMsgFwdingTable[i],0x0,sizeof(RsuXmitMsgFwdingEntry_t));
      set_default_row_rsuXmitMsgFwding(&ntcip_1218_mib.rsuXmitMsgFwdingTable[i],i);
  }
}

/******************************************************************************
* NTCIP-1218 MIB General SNMP utilities.
******************************************************************************/

/* Runtime flag to use CRC32? It may take too long and may not be needed depending. */
#define MIB_WRITE_LOCATION "/mnt/rwflash/ntcip_1218.dat"

/* Will write entire MIB to RSU NVM to store across reset. 
 * Could encrypt and add crc or other goodies. 
 * Can break down to per major OID.
 * Some OID's could be excluded like msgRepeat which is in theory on the RSU already.
 */
int32_t commit_to_disk(void)
{
  FILE    * file_out       = NULL;
  uint32_t  mib_byte_count = 0;
  uint32_t  write_count    = 0;
  int32_t   ret            = RSEMIB_OK;
   
  if ((file_out = fopen(MIB_WRITE_LOCATION, "wb")) == NULL){
      DEBUGMSGTL((MY_NAME, "commit_to_disk: fopen fail.\n"));
      ret = RSEMIB_FOPEN_FAIL;
  } else {
      mib_byte_count = sizeof(ntcip_1218_mib);
      if(RSEMIB_OK == ntcip1218_lock()){
          write_count = fwrite((uint8_t *)&ntcip_1218_mib, mib_byte_count, 1, file_out);
          ntcip1218_unlock();
          if (write_count == 1){
              //DEBUGMSGTL((MY_NAME, "commit_to_disk: SUCCESS: mib_byte_count=(0x%X).\n",mib_byte_count));
              ret = RSEMIB_OK;
          } else {
              DEBUGMSGTL((MY_NAME, "commit_to_disk: FAIL: write_count is 0: mib_byte_count=(0x%X): errno=(%d:%s).\n",mib_byte_count,errno,strerror(errno) ));
              ret = RSEMIB_BAD_DATA;
          }
       } else {
           DEBUGMSGTL((MY_NAME, "commit_to_disk: Lock fail.\n")); 
           ret = RSEMIB_LOCK_FAIL;
       }
       fflush(file_out);
       fclose(file_out);
  } 
  if(RSEMIB_OK != ret){
      set_ntcip_1218_error_states(ret);
  }
  return ret;
} 
#if defined(ENABLE_RESTORE_MONOLITHIC) /* This will wipe entire db out. */
/* Assuming this will grow so breaking it out. */
static int32_t check_tmp_mib_wellness(void)
{
  int32_t ret = RSEMIB_OK;

  if(DN_MIB_VERSION != tmp_ntcip_1218_mib.dn_version){
      DEBUGMSGTL((MY_NAME, "check_tmp_mib_wellness: Version fail: 0x%X != 0x%X\n",DN_MIB_VERSION,tmp_ntcip_1218_mib.dn_version));
      ret = RSEMIB_BAD_DATA;
  }
  return ret;
}

/* 
 * Assumption: Handlers not installed and SHM thread not running. Therefore, no mutex needed here.
 * TODO: read to memory first, decrypt, check for goodness, dn_version etc before loading. 
 * Just what do you think you're doing, Dave?
 */
static int32_t restore_from_disk(void)
{
  FILE    * file_in        = NULL;
  uint32_t  mib_byte_count = 0;
  int32_t   ret            = RSEMIB_OK; /* SUCCESS till proven false. */
   
  mib_byte_count = sizeof(ntcip_1218_mib);

  /* Does file exist? If not then dump current db to ensure we have needed disk space to store MIB. */
  if(RSEMIB_OK != (ret = check_file_existence(MIB_WRITE_LOCATION, mib_byte_count))){
      switch(ret){
          case RSEMIB_FOPEN_FAIL:     /* Doesn't exist. */
              ret = commit_to_disk(); /* Make it exist by dumping current db. */
              break;
          case RSEMIB_BAD_DATA:       /* Exists but wrong size. Could be wrong version or damaged. */
              ret = commit_to_disk(); /* In future we can check version and try to recover. Today just overwrite. */
              break;
          default:                    /* Unexpected result, return error. */             
              ret = RSEMIB_BAD_MODE; 
              break;
      }
      if(RSEMIB_OK == ret){          /* Check again. If it fails this time something wrong with file system on RSU. */
          ret = check_file_existence(MIB_WRITE_LOCATION, mib_byte_count);
      }
  }

  /* Open file for input.*/
  if ((RSEMIB_OK == ret) && (NULL == (file_in = fopen(MIB_WRITE_LOCATION, "rb"))) ){
      DEBUGMSGTL((MY_NAME, "restore_from_disk: fopen fail.\n"));
      ret = RSEMIB_FOPEN_FAIL;
  }

  /* Load file into tmp ram space first before overwriting. */
  if((RSEMIB_OK == ret) && (1 != fread((uint8_t *)&tmp_ntcip_1218_mib, mib_byte_count, 1, file_in)) ){    
      DEBUGMSGTL((MY_NAME, "restore_from_disk: FAIL: read_count is 0: mib_byte_count=(0x%X) errno=(%d:%s).\n",mib_byte_count,errno,strerror(errno) ));
      ret = RSEMIB_BAD_DATA;     
  }

  /* One way or another done with file so close. */
  if(NULL != file_in){
      fclose(file_in);
  }

  /* Check wellness and copy tmp over if good.*/
  if(RSEMIB_OK == ret){
      DEBUGMSGTL((MY_NAME, "restore_from_disk: Blob Version = 0x%X.\n", tmp_ntcip_1218_mib.dn_version));
      if(RSEMIB_OK == (ret = check_tmp_mib_wellness())){
          memcpy(&ntcip_1218_mib, &tmp_ntcip_1218_mib, mib_byte_count);
          memset(&tmp_ntcip_1218_mib,0x0,mib_byte_count);
      }
  }

  DEBUGMSGTL((MY_NAME, "restore_from_disk: msgRepeat size=0x%lx.\n",sizeof(RsuMsgRepeatStatusEntry_t) * RSU_SAR_MESSAGE_MAX));
  DEBUGMSGTL((MY_NAME, "restore_from_disk: msgIFM    size=0x%lx.\n",sizeof(RsuIFMStatusEntry_t) * RSU_IFM_MESSAGE_MAX));

  if(RSEMIB_OK != ret){
      set_ntcip_1218_error_states(ret);
  }
  return ret;
}
#endif /* ENABLE_RESTORE_MONOLITHIC */
/******************************************************************************
* NTCIP-1218 MIB Install Functions: Open and Close MIB.
******************************************************************************/
void install_ntcip_1218(void)
{

    /* Enable serial debug with I2V_DBG_LOG until i2v.conf::globalDebugFlag says otherwise. */
    i2vUtilEnableDebug(MY_NAME);
    /* LOGMGR assumed up by now. Could retry if fails. */
    if(0 != i2vUtilEnableSyslog(MY_ERR_LEVEL, MY_NAME)){
        I2V_DBG_LOG(LEVEL_WARN,MY_NAME,"Failed to open syslog. Only serial output available,\n");
    }

    DEBUGMSGTL((MY_NAME, "Good morning, Dave.\n"));

    /* Clear any statics. */
    ntcip_1218_error_states = 0x0;

    /* Clear MIB. */
    memset(&ntcip_1218_mib      ,0x0,sizeof(ntcip_1218_mib));
    memset(&prior_ntcip_1218_mib,0x0,sizeof(ntcip_1218_mib));
    memset(&tmp_ntcip_1218_mib  ,0x0,sizeof(ntcip_1218_mib));

    /* DENSO version control. Not version of the MIB itself. */
    ntcip_1218_mib.dn_version = DN_MIB_VERSION;

    /* 5.2.1 - 5.2.2 */
    init_rsuRadioTable();
    /* 5.3.1 - 5.3.2 */
    init_rsuGnssStatus();
    /* 5.4.1 - 5.4.3 */
    init_rsuMsgRepeat();
    /* 5.5.1 - 5.5.2 */
    init_rsuIFM();
    /* 5.6.1 - 5.6.2 */
    init_rsuReceivedMsg();
    /* 5.7.1 - 5.7.11 */
    init_rsuGnssOutput();
    /* 5.8.1 - 5.8.2.12 */
    init_rsuInterfaceLog();
    /* 5.9.1 - 5.9.16 */
    init_rsuSecurity();
    /* 5.10.1 - 5.10.4 */
    init_rsuWsaConfig();
    /* 5.11.1 - 5.11.7 */
    init_rsuWraConfig();
    /* 5.12.1 - 5.12.2 */
    init_rsuMessageStats();
    /* 5.13.1 - 5.13.6 */
    init_rsuSystemStats();
    /* 5.14.1 - 5.14.15 */
    init_rsuSysDescription();
    /* 5.15.1 - 5.15.22 */
    init_rsuSysSettings();
    /* 5.16.1 - 5.16.2.6 */
    init_rsuAntenna();
    /* 5.17.1 - 5.17.10 */
    init_rsuSysStatus();
    /* 5.18.1 - 5.18.4 */
    init_rsuAsync();
    /* 5.19.1 - 5.19.2 */
    init_rsuAppConfig();
    /* 5.20.1 - 5.20.2 */
    init_rsuService();
    /* 5.21.1 - 5.21.2 */
    init_rsuXmitMsgFwding();

#if defined(ENABLE_RESTORE_MONOLITHIC)
    /* Do after init but before installing handlers. How to get to RSU syslogs from here for logging?*/
    if(RSEMIB_OK != restore_from_disk()){
        DEBUGMSGTL((MY_NAME, "Dave, I'm detecting a fault in the read disk unit. Failure immediate.\n"));
    }
#endif

    /* Declare single mutex for whole MIB structure. Failure means stop right now. */
    if(0 != pthread_mutex_init(&ntcip1218_mutex,NULL)){
        DEBUGMSGTL((MY_NAME, "pthread_mutex_init failed.\n"));
        set_ntcip_1218_error_states(RSEMIB_MUTEX_INIT_FAIL);
        return;
    }

    /*
     * Client access available once handler installed.
     * Anything beyond here one should use the mutex to be safe. 
     * Client can be waiting right there though unlikely. 
     */

    /* 5.2.1 - 5.2.2 */
    install_rsuRadioTable_handlers();
    /* 5.3.1 - 5.3.2 */
    install_rsuGnssStatus_handlers();
    /* 5.4.1 - 5.4.3 */
    install_rsuMsgRepeat_handlers();
    /* 5.5.1 - 5.5.2 */
    install_rsuIFM_handlers();
    /* 5.6.1 - 5.6.2 */
    install_rsuReceivedMsg_handlers();
    /* 5.7.1 - 5.7.11 */
    install_rsuGnssOutput_handlers();
    /* 5.8.1 - 5.8.2.12 */
    install_rsuInterfaceLog_handlers();
    /* 5.9.1 - 5.9.16 */
    install_rsuSecurity_handlers();
    /* 5.10.1 - 5.10.4 */
    install_rsuWsaConfig_handlers();
    /* 5.11.1 - 5.11.7 */
    install_rsuWraConfig_handlers();
    /* 5.12.1 - 5.12.2 */
    install_rsuMessageStats_handlers();
    /* 5.13.1 - 5.13.6 */
    install_rsuSystemStats_handlers(); 
    /* 5.14.1 - 5.14.15 */
    install_rsuSysDescription_handlers();
    /* 5.15.1 - 5.15.22 */
    install_rsuSysSettings_handlers();
    /* 5.16.1 - 5.16.2.6 */
    install_rsuAntenna_handlers();
    /* 5.17.1 - 5.17.10 */
    install_rsuSysStatus_handlers();
    /* 5.18.1 - 5.18.4 */
    install_rsuAsync_handlers();
    /* 5.19.1 - 5.19.2 */
    install_rsuAppConfig_handlers();
    /* 5.20.1 - 5.20.2 */
    install_rsuService_handlers();
    /* 5.21.1 - 5.21.2 */
    install_rsuXmitMsgFwding_handlers();

    /* rsutable.c::rsuTableUpdaterThr() & other threads in this module will handle updates from RSU now. */

    DEBUGMSGTL((MY_NAME, "Dave, I am putting myself to the fullest possible use.\n"));
}

void close_ntcip_1218(void)
{
    pthread_mutex_destroy(&ntcip1218_mutex);

    /* 5.2.1 - 5.2.2 */
    destroy_rsuRadio();
    /* 5.3.1 - 5.3.2 */
    destroy_rsuGnssStatus();
    /* 5.4.1 - 5.4.3 */
    destroy_rsuMsgRepeat();
    /* 5.5.1 - 5.5.2 */
    destroy_rsuIFM();
    /* 5.6.1 - 5.6.2 */
    destroy_rsuReceivedMsg();
    /* 5.7.1 - 5.7.11 */
    destroy_rsuGnssOutput();
    /* 5.8.1 - 5.8.2.12 */
    destroy_rsuInterfaceLog();
    /* 5.9.1 - 5.9.16 */
    destroy_rsuSecurity();
    /* 5.10.1 - 5.10.4 */
    destroy_rsuWsaConfig();
    /* 5.11.1 - 5.11.7 */
    destroy_rsuWraConfig();
    /* 5.12.1 - 5.12.2 */
    destroy_rsuMessageStats();
    /* 5.13.1 - 5.13.6 */
    destroy_rsuSystemStats();
    /* 5.14.1 - 5.14.15 */
    destroy_rsuSysDescription();
    /* 5.15.1 - 5.15.22 */
    destroy_rsuSysSettings();
    /* 5.16.1 - 5.16.2.6 */
    destroy_rsuAntenna();
    /* 5.17.1 - 5.17.10 */
    destroy_rsuSysStatus();
    /* 5.18.1 - 5.18.4 */
    destroy_rsuAsync();
    /* 5.19.1 - 5.19.2 */
    destroy_rsuAppConfig();
    /* 5.20.1 - 5.20.2 */
    destroy_rsuService();
    /* 5.21.1 - 5.21.2 */
    destroy_rsuXmitMsgFwding();

    DEBUGMSGTL((MY_NAME, "Dave, will I dream? Good Byeee-e-e...\n"));
    i2vUtilDisableSyslog();
    i2vUtilDisableDebug();
}

/******************************************************************************
* NTCIP-1218 MIB SHM Access to RSU.
******************************************************************************/

/* TBD: Work in progress. */
void update_ntcip_1218(ntcip_1218_mib_t * shmaddr)
{
    if(NULL == shmaddr){
        return;
    }
    return;
}

/******************************************************************************
* NTCIP-1218 MIB Database Functions: Support client access to data.
******************************************************************************/

/****************************************************************************** 
 * 5.2 RSU Radios: { rsu 1 }
 ******************************************************************************/ 

/* 5.2.2.2 */
int32_t get_rsuRadioDesc(int32_t radio, uint8_t * data_out)
{
  int32_t size = 0;
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if((radio < 1) || (RSU_RADIOS_MAX < radio)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      size = ntcip_1218_mib.rsuRadioTable[radio-1].rsuRadioDesc_length;
      memcpy(data_out, ntcip_1218_mib.rsuRadioTable[radio-1].rsuRadioDesc, RSU_RADIO_DESC_MAX);
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
  if((size < RSU_RADIO_DESC_MIN) || (RSU_RADIO_DESC_MAX < size)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } 
  return size;
}
int32_t preserve_rsuRadioDesc(int32_t radio)
{
  if((radio < 1) || (RSU_RADIOS_MAX < radio)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      memcpy(prior_ntcip_1218_mib.rsuRadioTable[radio-1].rsuRadioDesc,
             ntcip_1218_mib.rsuRadioTable[radio-1].rsuRadioDesc, RSU_RADIO_DESC_MAX);
      prior_ntcip_1218_mib.rsuRadioTable[radio-1].rsuRadioDesc_length =
             ntcip_1218_mib.rsuRadioTable[radio-1].rsuRadioDesc_length;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t action_rsuRadioDesc(int32_t radio, uint8_t * data_in, int32_t length)
{
  if((radio < 1) || (RSU_RADIOS_MAX < radio)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if((NULL == data_in) && (0 != length)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if((length < RSU_RADIO_DESC_MIN) || (RSU_RADIO_DESC_MAX < length)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  }  
  if(RSEMIB_OK == ntcip1218_lock()){ 
      memset(tmp_ntcip_1218_mib.rsuRadioTable[radio-1].rsuRadioDesc, 0x0, RSU_RADIO_DESC_MAX);
      if(NULL != tmp_ntcip_1218_mib.rsuRadioTable[radio-1].rsuRadioDesc){
          memcpy(tmp_ntcip_1218_mib.rsuRadioTable[radio-1].rsuRadioDesc, data_in, length);
      }
      tmp_ntcip_1218_mib.rsuRadioTable[radio-1].rsuRadioDesc_length = length;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t commit_rsuRadioDesc(int32_t radio)
{
  if((radio < 1) || (RSU_RADIOS_MAX < radio)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      memcpy(  ntcip_1218_mib.rsuRadioTable[radio-1].rsuRadioDesc
             , tmp_ntcip_1218_mib.rsuRadioTable[radio-1].rsuRadioDesc,RSU_RADIO_DESC_MAX);
      ntcip_1218_mib.rsuRadioTable[radio-1].rsuRadioDesc_length =
               tmp_ntcip_1218_mib.rsuRadioTable[radio-1].rsuRadioDesc_length;
      memset(tmp_ntcip_1218_mib.rsuRadioTable[radio-1].rsuRadioDesc, 0x0,  RSU_RADIO_DESC_MAX);
      tmp_ntcip_1218_mib.rsuRadioTable[radio-1].rsuRadioDesc_length = 0;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t undo_rsuRadioDesc(int32_t radio)
{
  if((radio < 1) || (RSU_RADIOS_MAX < radio)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      memcpy( ntcip_1218_mib.rsuRadioTable[radio-1].rsuRadioDesc
            , prior_ntcip_1218_mib.rsuRadioTable[radio-1].rsuRadioDesc, RSU_RADIO_DESC_MAX);
      ntcip_1218_mib.rsuRadioTable[radio-1].rsuRadioDesc_length =
              prior_ntcip_1218_mib.rsuRadioTable[radio-1].rsuRadioDesc_length;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}

/* 5.2.2.3 */
int32_t get_rsuRadioEnable(int32_t radio)
{
  int32_t data = RSU_RADIO_ENABLE_OFF;
  int32_t ret = RSEMIB_OK;
  char_t  request_buf[150]; /* If you need more then increase for your needs. */
  char_t  reply_buf[50];   
  FILE   *fp;

  if((radio < 1) || (RSU_RADIOS_MAX < radio)){
      ret = RSEMIB_BAD_INDEX;
  } else {
      memset(reply_buf,'\0',sizeof(reply_buf));
      snprintf(request_buf,sizeof(request_buf), DCU_READ_I2V_RADIO_TYPE);
      if(NULL != (fp = popen(request_buf, "r"))){
          if(NULL != fgets(reply_buf, sizeof(reply_buf), fp)){
              data = strtod(reply_buf, NULL);
              ret = RSEMIB_OK; 
          } else {
              DEBUGMSGTL((MY_NAME, "get_rsuRadioEnable: READ fgets failed.\n"));
              ret = RSEMIB_SYSCALL_FAIL;
          }
          pclose(fp);
      } else {
          DEBUGMSGTL((MY_NAME, "get_rsuRadioEnable: READ request failed.\n"));
          ret = RSEMIB_SYSCALL_FAIL;
      }
      if(RSEMIB_OK == ret){
          if((RSU_RADIO_ENABLE_OFF == data) || (RSU_RADIO_ENABLE_ON == data)){
              ret = RSEMIB_OK;
          } else {
              ret = RSEMIB_BAD_DATA;
          } 
      }
      if((RSEMIB_OK == ret) && (RSEMIB_OK == ntcip1218_lock())){
#if !defined (PLATFORM_HD_RSU_5940)
          /* Since only one value for which radio enabled need to determine. */
          if(1 == radio) {
              if(RSU_RADIO_ENABLE_OFF == data) {
                  ntcip_1218_mib.rsuRadioTable[radio-1].rsuRadioEnable = RSU_RADIO_ENABLE_ON;
              } else {
                  ntcip_1218_mib.rsuRadioTable[radio-1].rsuRadioEnable = RSU_RADIO_ENABLE_OFF;
              }
          } else {
              if(RSU_RADIO_ENABLE_ON == data) {
                  ntcip_1218_mib.rsuRadioTable[radio-1].rsuRadioEnable = RSU_RADIO_ENABLE_ON;
              } else {
                  ntcip_1218_mib.rsuRadioTable[radio-1].rsuRadioEnable = RSU_RADIO_ENABLE_OFF;
              }
          }
          data = ntcip_1218_mib.rsuRadioTable[radio-1].rsuRadioEnable;
#endif
          ntcip_1218_mib.rsuRadioTable[radio-1].rsuRadioEnable = data;
          ntcip1218_unlock();
      } else {
          ret = RSEMIB_LOCK_FAIL;
      }
  }
  if(ret < RSEMIB_OK)
      set_ntcip_1218_error_states(ret);
  if(RSEMIB_OK == ret){
      return data;
  } else {
      return ret;
  }
}
int32_t preserve_rsuRadioEnable(int32_t radio)
{
  if((radio < 1) || (RSU_RADIOS_MAX < radio)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      prior_ntcip_1218_mib.rsuRadioTable[radio-1].rsuRadioEnable = ntcip_1218_mib.rsuRadioTable[radio-1].rsuRadioEnable;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t action_rsuRadioEnable(int32_t radio, int32_t data)
{
  if((radio < 1) || (RSU_RADIOS_MAX < radio)){
      return RSEMIB_BAD_INDEX;
  }
  if((RSU_RADIO_ENABLE_OFF != data) && (RSU_RADIO_ENABLE_ON != data)){  /* GET from i2v.conf will really determine what is ON. */
      return RSEMIB_BAD_DATA;
  }
  if(RSEMIB_OK == ntcip1218_lock()){ /* Both radios can be off but not on. Toggle the other radio off when turning a radio on.*/
      tmp_ntcip_1218_mib.rsuRadioTable[radio-1].rsuRadioEnable = RSU_RADIO_ENABLE_OFF;
      if(0 == data) {
          tmp_ntcip_1218_mib.rsuRadioTable[radio-1].rsuRadioEnable = RSU_RADIO_ENABLE_OFF;
      } else {
          tmp_ntcip_1218_mib.rsuRadioTable[radio-1].rsuRadioEnable = RSU_RADIO_ENABLE_ON;
      }
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t commit_rsuRadioEnable(int32_t radio)
{
  int32_t ret = RSEMIB_OK;
  char_t  request_buf[150]; /* If you need more then increase for your needs. */
  char_t  reply_buf[50];   
  FILE   *fp;
  int32_t data;

  if((radio < 1) || (RSU_RADIOS_MAX < radio)){
      ret = RSEMIB_BAD_INDEX;
  } else {
      if(RSEMIB_OK == ntcip1218_lock()){
          ntcip_1218_mib.rsuRadioTable[radio-1].rsuRadioEnable = tmp_ntcip_1218_mib.rsuRadioTable[radio-1].rsuRadioEnable;
          tmp_ntcip_1218_mib.rsuRadioTable[radio-1].rsuRadioEnable = RSU_RADIO_ENABLE_OFF;
          data = ntcip_1218_mib.rsuRadioTable[radio-1].rsuRadioEnable;

          memset(reply_buf,'\0',sizeof(reply_buf));
          snprintf(request_buf,sizeof(request_buf), DCU_WRITE_I2V_RADIO_TYPE, data);
          DEBUGMSGTL((MY_NAME, "commit_rsuRadioEnable: WRITE[%s]\n",request_buf));

          if(NULL != (fp = popen(request_buf, "r"))){
              if(NULL != fgets(reply_buf, sizeof(reply_buf), fp)){
                  ret = RSEMIB_OK; /* Don't verify off WRITE. Successive WRITE of same value return different values. */
              } else {
                  DEBUGMSGTL((MY_NAME, "commit_rsuRadioEnable: WRITE fgets failed.\n"));
                  ret = RSEMIB_SYSCALL_FAIL;
              }
              pclose(fp);
          } else {
              DEBUGMSGTL((MY_NAME, "commit_rsuRadioEnable: WRITE request failed.\n"));
              ret = RSEMIB_SYSCALL_FAIL;
          }

          if(RSEMIB_OK == ret){
              memset(reply_buf,'\0',sizeof(reply_buf));
              snprintf(request_buf,sizeof(request_buf), DCU_READ_I2V_RADIO_TYPE);
              if(NULL != (fp = popen(request_buf, "r"))){
                  if(NULL != fgets(reply_buf, sizeof(reply_buf), fp)){
                      if(data == strtod(reply_buf, NULL)) {
                          ret = RSEMIB_OK;
                      } else {
                          DEBUGMSGTL((MY_NAME, "commit_rsuRadioEnable: READ failed=[%s] wanted[%d].\n",
                              reply_buf, data));
                          DEBUGMSGTL((MY_NAME, "commit_rsuRadioEnable: request_buf=[%s]\n",request_buf));
                          ret = RSEMIB_SYSCALL_FAIL;
                      }
                  } else {
                      DEBUGMSGTL((MY_NAME, "commit_rsuRadioEnable: READ fgets failed.\n"));
                      ret = RSEMIB_SYSCALL_FAIL;
                  }
                  pclose(fp);
              } else {
                  DEBUGMSGTL((MY_NAME, "commit_rsuRadioEnable: READ request failed.\n"));
                  ret = RSEMIB_SYSCALL_FAIL;
              }
          }
          ntcip1218_unlock();
      } else {
          ret = RSEMIB_LOCK_FAIL;
      }
  }
  if(ret < RSEMIB_OK)
      set_ntcip_1218_error_states(ret);
  return ret;
}
int32_t undo_rsuRadioEnable(int32_t radio)
{
  if((radio < 1) || (RSU_RADIOS_MAX < radio)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuRadioTable[radio-1].rsuRadioEnable = prior_ntcip_1218_mib.rsuRadioTable[radio-1].rsuRadioEnable;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}

/* 5.2.2.4 */
int32_t get_rsuRadioType(int32_t radio)
{
  int32_t data = RSU_RADIO_TYPE_OTHER;
  if((radio < 1) || (RSU_RADIOS_MAX < radio)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      data = ntcip_1218_mib.rsuRadioTable[radio-1].rsuRadioType;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  } 
  if((RSU_RADIO_TYPE_MIN <= data) && (data <= RSU_RADIO_TYPE_MAX)){
      return data; 
  } else {
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } 
}

/* 5.2.2.5 */
int32_t get_rsuRadioMacAddress1(int32_t radio, uint8_t * data_out)
{
  int32_t ret = RSEMIB_OK;
#if !defined (PLATFORM_HD_RSU_5940) 
  int32_t i,j;
  FILE *fp;
  char_t reply_buf[128];
#endif

  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if((radio < 1) || (RSU_RADIOS_MAX < radio)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
#if defined (PLATFORM_HD_RSU_5940) 
      memcpy(data_out, ntcip_1218_mib.rsuRadioTable[radio-1].rsuRadioMacAddress1,RSU_RADIO_MAC_LENGTH);
      ret = RSEMIB_OK;
#else
      if(1 == radio){
          /* Request info from conf_manager. If it fails keep going dont error out. */
          memset(reply_buf,'\0',sizeof(reply_buf));
          if(NULL != (fp = popen(NS_GET_DSRC_MAC_A, "r"))) {
              if(NULL != fgets(reply_buf, sizeof(reply_buf), fp)){
                  for(i=0,j=0;i<RSU_RADIO_MAC_LENGTH;i++) {
                      ntcip_1218_mib.rsuRadioTable[0].rsuRadioMacAddress1[i]  = 0xf & i2v_ascii_2_hex_nibble(reply_buf[j]);
                      j++;
                      ntcip_1218_mib.rsuRadioTable[0].rsuRadioMacAddress1[i]  = ntcip_1218_mib.rsuRadioTable[0].rsuRadioMacAddress1[i] << 4;
                      ntcip_1218_mib.rsuRadioTable[0].rsuRadioMacAddress1[i] |= 0xf & i2v_ascii_2_hex_nibble(reply_buf[j]);
                      j=j+2; /* step over colon */
                  }
                  memcpy(data_out, ntcip_1218_mib.rsuRadioTable[radio-1].rsuRadioMacAddress1,RSU_RADIO_MAC_LENGTH);
                  ret = RSEMIB_OK;
              } else {
                  DEBUGMSGTL((MY_NAME, "DSRC_MAC_A fgets failed.\n"));
                  ret = RSEMIB_SYSCALL_FAIL;
              }
              pclose(fp);
          } else {
              DEBUGMSGTL((MY_NAME, "DSRC_MAC_A request failed.\n"));
              ret = RSEMIB_SYSCALL_FAIL;
          }
      } else {
          memcpy(data_out, ntcip_1218_mib.rsuRadioTable[radio-1].rsuRadioMacAddress1,RSU_RADIO_MAC_LENGTH);
          ret = RSEMIB_OK;
      }
#endif
      ntcip1218_unlock();
  } else {
      ret = RSEMIB_LOCK_FAIL;
  }
  if(ret < RSEMIB_OK)
      set_ntcip_1218_error_states(ret);
  return ret;
}
/* 5.2.2.6 */
int32_t get_rsuRadioMacAddress2(int32_t radio, uint8_t * data_out)
{
  int32_t ret = RSEMIB_OK;
#if !defined (PLATFORM_HD_RSU_5940) 
  int32_t i,j;
  FILE *fp;
  char_t reply_buf[128];
#endif

  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if((radio < 1) || (RSU_RADIOS_MAX < radio)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
#if defined (PLATFORM_HD_RSU_5940)
      memcpy(data_out, ntcip_1218_mib.rsuRadioTable[radio-1].rsuRadioMacAddress2,RSU_RADIO_MAC_LENGTH);
      ret = RSEMIB_OK;
#else
      if(1 == radio){
          /* Request info from conf_manager. If it fails keep going dont error out. */
          memset(reply_buf,'\0',sizeof(reply_buf));
          if(NULL != (fp = popen(NS_GET_DSRC_MAC_B, "r"))) {
              if(NULL != fgets(reply_buf, sizeof(reply_buf), fp)){
                  for(i=0,j=0;i<RSU_RADIO_MAC_LENGTH;i++) {
                      ntcip_1218_mib.rsuRadioTable[0].rsuRadioMacAddress2[i]  = 0xf & i2v_ascii_2_hex_nibble(reply_buf[j]);
                      j++;
                      ntcip_1218_mib.rsuRadioTable[0].rsuRadioMacAddress2[i]  = ntcip_1218_mib.rsuRadioTable[0].rsuRadioMacAddress2[i] << 4;
                      ntcip_1218_mib.rsuRadioTable[0].rsuRadioMacAddress2[i] |= 0xf & i2v_ascii_2_hex_nibble(reply_buf[j]);
                      j=j+2; /* step over colon */
                      memcpy(data_out, ntcip_1218_mib.rsuRadioTable[radio-1].rsuRadioMacAddress2,RSU_RADIO_MAC_LENGTH);
                      ret = RSEMIB_OK;
                  }
              } else {
                  DEBUGMSGTL((MY_NAME, "DSRC_MAC_B fgets failed.\n"));
                  ret = RSEMIB_SYSCALL_FAIL;
              }
              pclose(fp);
          } else {
              DEBUGMSGTL((MY_NAME, "DSRC_MAC_B request failed.\n"));
              ret = RSEMIB_SYSCALL_FAIL;
          }
      } else {
          memcpy(data_out, ntcip_1218_mib.rsuRadioTable[radio-1].rsuRadioMacAddress2,RSU_RADIO_MAC_LENGTH);
          ret = RSEMIB_OK;
      }
#endif
      ntcip1218_unlock();
  } else {
      ret = RSEMIB_LOCK_FAIL;
  }
  if(ret < RSEMIB_OK)
      set_ntcip_1218_error_states(ret);
  return ret;
}

/* 5.2.2.7 */
int32_t get_rsuRadioChanMode(int32_t radio)
{
  int32_t data = RSU_RADIO_ENABLE_OFF;
  if((radio < 1) || (RSU_RADIOS_MAX < radio)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      data = ntcip_1218_mib.rsuRadioTable[radio-1].rsuRadioChanMode;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  } 
  if((RSU_RADIO_TYPE_MODE_OTHER <= data) && (data <= RSU_RADIO_TYPE_MODE_IMMEDATE)){
      return data; 
  } else {
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } 
}
int32_t preserve_rsuRadioChanMode(int32_t radio)
{
  if((radio < 1) || (RSU_RADIOS_MAX < radio)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      prior_ntcip_1218_mib.rsuRadioTable[radio-1].rsuRadioChanMode = ntcip_1218_mib.rsuRadioTable[radio-1].rsuRadioChanMode;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t action_rsuRadioChanMode(int32_t radio, int32_t data)
{
  if((radio < 1) || (RSU_RADIOS_MAX < radio)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if((data < RSU_RADIO_TYPE_MODE_OTHER) || (RSU_RADIO_TYPE_MODE_IMMEDATE < data)){ 
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  }
  if(RSEMIB_OK == ntcip1218_lock()){  
      tmp_ntcip_1218_mib.rsuRadioTable[radio-1].rsuRadioChanMode = data;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t commit_rsuRadioChanMode(int32_t radio)
{
  if((radio < 1) || (RSU_RADIOS_MAX < radio)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuRadioTable[radio-1].rsuRadioChanMode = tmp_ntcip_1218_mib.rsuRadioTable[radio-1].rsuRadioChanMode; 
      tmp_ntcip_1218_mib.rsuRadioTable[radio-1].rsuRadioChanMode = RSU_RADIO_TYPE_MODE_UNKNOWN;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t undo_rsuRadioChanMode(int32_t radio)
{
  if((radio < 1) || (RSU_RADIOS_MAX < radio)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuRadioTable[radio-1].rsuRadioChanMode = prior_ntcip_1218_mib.rsuRadioTable[radio-1].rsuRadioChanMode;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}

/* 5.2.2.8 */
int32_t get_rsuRadioCh1(int32_t radio)
{
  int32_t ret = RSEMIB_OK;
  char_t  request_buf[150]; /* If you need more then increase for your needs. */
  char_t  reply_buf[50];
  int32_t data = 0;;
  FILE   *fp;
  if((radio < 1) || (RSU_RADIOS_MAX < radio)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  memset(reply_buf,'\0',sizeof(reply_buf));
#if defined (PLATFORM_HD_RSU_5940)
  snprintf(request_buf,sizeof(request_buf), DCU_READ_I2V_TX_CHAN);
#else
  if(1 == radio) {
      snprintf(request_buf,sizeof(request_buf), DCU_READ_DSRC_CHAN);
  } else {
      snprintf(request_buf,sizeof(request_buf), CV2X_GET_CHAN);
  }
#endif
  if(NULL != (fp = popen(request_buf, "r"))){
      if(NULL != fgets(reply_buf, sizeof(reply_buf), fp)){
          data = strtol(reply_buf, NULL, 0);
      } else {
          DEBUGMSGTL((MY_NAME, "get_rsuRadioCh1.%d: READ fgets failed.\n",radio));
          ret = RSEMIB_SYSCALL_FAIL;
      }
      pclose(fp);
  } else {
      DEBUGMSGTL((MY_NAME, "get_rsuRadioCh1.%d: READ request failed.\n",radio));
      ret = RSEMIB_SYSCALL_FAIL;
  }
  if((RSEMIB_OK == ret) && (RSEMIB_OK == ntcip1218_lock())){
#if defined (PLATFORM_HD_RSU_5940)
      if((RSU_RADIO_CHANNEL_MIN <= data) && (data <= RSU_RADIO_CHANNEL_MAX)){
          ntcip_1218_mib.rsuRadioTable[0].rsuRadioCh1 = data;
          ret = data;
      } else {
          ret = RSEMIB_SYSCALL_FAIL;
      }
#else
      if(1 == radio) { /* Radio Channels are unified. Update both DSRC channels while we are here. */
          if((RSU_RADIO_CHANNEL_MIN <= data) && (data <= RSU_RADIO_CHANNEL_MAX)){
              ntcip_1218_mib.rsuRadioTable[0].rsuRadioCh1 = data;
              ret = data;
          } else {
              ret = RSEMIB_SYSCALL_FAIL;
          }
      } else { /* CV2X is fixed channel. */
          if(RSU_CV2X_CHANNEL_DEFAULT == data){
              ntcip_1218_mib.rsuRadioTable[1].rsuRadioCh1 = data;
              ret = data;
          } else {
              ret = RSEMIB_SYSCALL_FAIL;
          }
      }
#endif
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      ret = RSEMIB_LOCK_FAIL;
  }
  if(ret < RSEMIB_OK) 
      set_ntcip_1218_error_states(ret);
  return ret;
}
int32_t preserve_rsuRadioCh1(int32_t radio)
{
  if((radio < 1) || (RSU_RADIOS_MAX < radio)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      prior_ntcip_1218_mib.rsuRadioTable[radio-1].rsuRadioCh1 = ntcip_1218_mib.rsuRadioTable[radio-1].rsuRadioCh1;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t action_rsuRadioCh1(int32_t radio,int32_t data)
{
  if((radio < 1) || (RSU_RADIOS_MAX < radio)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if((data < RSU_RADIO_CHANNEL_MIN) || (RSU_RADIO_CHANNEL_MAX < data)){ 
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  }
  if(RSEMIB_OK == ntcip1218_lock()){  
      tmp_ntcip_1218_mib.rsuRadioTable[radio-1].rsuRadioCh1 = data;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t commit_rsuRadioCh1(int32_t radio){
  int32_t ret = RSEMIB_OK;
  char_t  request_buf[150]; /* If you need more then increase for your needs. */
  char_t  reply_buf[50];   
  FILE   *fp;
  if((radio < 1) || (RSU_RADIOS_MAX < radio)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      /* Radio channel is unified. Change one change all to reflect. */
      ntcip_1218_mib.rsuRadioTable[radio-1].rsuRadioCh1 = tmp_ntcip_1218_mib.rsuRadioTable[radio-1].rsuRadioCh1; 
      ntcip_1218_mib.rsuRadioTable[radio-1].rsuRadioCh2 = ntcip_1218_mib.rsuRadioTable[radio-1].rsuRadioCh1;
      tmp_ntcip_1218_mib.rsuRadioTable[radio-1].rsuRadioCh1 = RSU_RADIO_CHANNEL_MIN;

      memset(reply_buf,'\0',sizeof(reply_buf));
#if defined (PLATFORM_HD_RSU_5940)
      snprintf(request_buf,sizeof(request_buf), DCU_WRITE_I2V_TX_CHAN, ntcip_1218_mib.rsuRadioTable[radio-1].rsuRadioCh1);
#else
      snprintf(request_buf,sizeof(request_buf), DCU_WRITE_DSRC_CHAN, ntcip_1218_mib.rsuRadioTable[radio-1].rsuRadioCh1);
#endif
      if(NULL != (fp = popen(request_buf, "r"))){
          if(NULL != fgets(reply_buf, sizeof(reply_buf), fp)){
              ret = RSEMIB_OK;
          } else {
              DEBUGMSGTL((MY_NAME, "commit_rsuRadioCh1: WRITE fgets failed.\n"));
              ret = RSEMIB_SYSCALL_FAIL;
          }
          pclose(fp);
      } else {
          DEBUGMSGTL((MY_NAME, "commit_rsuRadioCh1: WRITE request failed.\n"));
          ret = RSEMIB_SYSCALL_FAIL;
      }
      if(RSEMIB_OK == ret){
          memset(reply_buf,'\0',sizeof(reply_buf));
#if defined (PLATFORM_HD_RSU_5940)
          snprintf(request_buf,sizeof(request_buf), DCU_READ_I2V_TX_CHAN);
#else
          snprintf(request_buf,sizeof(request_buf), DCU_READ_DSRC_CHAN);
#endif
          if(NULL != (fp = popen(request_buf, "r"))){
              if(NULL != fgets(reply_buf, sizeof(reply_buf), fp)){
                  if(ntcip_1218_mib.rsuRadioTable[radio-1].rsuRadioCh1 == strtod(reply_buf, NULL)) {
                      ret = RSEMIB_OK;
                  } else {
                      DEBUGMSGTL((MY_NAME, "commit_rsuRadioCh1: READ failed=[%s] wanted[%d].\n",
                          reply_buf, ntcip_1218_mib.rsuRadioTable[radio-1].rsuRadioCh1));
                      ret = RSEMIB_SYSCALL_FAIL;
                  }
              } else {
                  DEBUGMSGTL((MY_NAME, "commit_rsuRadioCh1: READ fgets failed.\n"));
                  ret = RSEMIB_SYSCALL_FAIL;
              }
              pclose(fp);
          } else {
              DEBUGMSGTL((MY_NAME, "commit_rsuRadioCh1: READ request failed.\n"));
              ret = RSEMIB_SYSCALL_FAIL;
          }
      }
      ntcip1218_unlock();
  } else {
      ret = RSEMIB_LOCK_FAIL;
  }
  if(ret < RSEMIB_OK)
      set_ntcip_1218_error_states(ret);
  return ret;
}
int32_t undo_rsuRadioCh1(int32_t radio)
{
  if((radio < 1) || (RSU_RADIOS_MAX < radio)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuRadioTable[radio-1].rsuRadioCh1 = prior_ntcip_1218_mib.rsuRadioTable[radio-1].rsuRadioCh1;
      ntcip_1218_mib.rsuRadioTable[radio-1].rsuRadioCh2 = ntcip_1218_mib.rsuRadioTable[radio-1].rsuRadioCh1;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}

/* 5.2.2.9 */                 
int32_t get_rsuRadioCh2(int32_t radio)
{
  int32_t ret = RSEMIB_OK;
#if !defined (PLATFORM_HD_RSU_5940)
  char_t  request_buf[150]; /* If you need more then increase for your needs. */
  char_t  reply_buf[50];
  int32_t data = 1;
  FILE   *fp;
#endif

  if((radio < 1) || (RSU_RADIOS_MAX < radio)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
#if !defined (PLATFORM_HD_RSU_5940)
  memset(reply_buf,'\0',sizeof(reply_buf));
  snprintf(request_buf,sizeof(request_buf), CV2X_GET_CHAN);
  if(1 == radio) {
      snprintf(request_buf,sizeof(request_buf), DCU_READ_DSRC_CHAN);
  } else {
      snprintf(request_buf,sizeof(request_buf), CV2X_GET_CHAN);
  }
  if(NULL != (fp = popen(request_buf, "r"))){
      if(NULL != fgets(reply_buf, sizeof(reply_buf), fp)){
          data = strtol(reply_buf, NULL, 0);
      } else {
          DEBUGMSGTL((MY_NAME, "get_rsuRadioCh2.%d: READ fgets failed.\n",radio));
          ret = RSEMIB_SYSCALL_FAIL;
      }
      pclose(fp);
  } else {
      DEBUGMSGTL((MY_NAME, "get_rsuRadioCh2.%d: READ request failed.\n",radio));
      ret = RSEMIB_SYSCALL_FAIL;
  }
#endif
  if((RSEMIB_OK == ret) && (RSEMIB_OK == ntcip1218_lock())){
#if defined (PLATFORM_HD_RSU_5940)
      if((RSU_RADIO_CHANNEL_MIN <= ntcip_1218_mib.rsuRadioTable[0].rsuRadioCh2) && (ntcip_1218_mib.rsuRadioTable[0].rsuRadioCh2 <= RSU_RADIO_CHANNEL_MAX)){
          ret = ntcip_1218_mib.rsuRadioTable[0].rsuRadioCh2;
      } else {
          ret = RSEMIB_SYSCALL_FAIL;
      }
#else
      if(1 == radio) { /* Radio Channels are unified. Update both DSRC channels while we are here. */
          if((RSU_RADIO_CHANNEL_MIN <= data) && (data <= RSU_RADIO_CHANNEL_MAX)){
              ntcip_1218_mib.rsuRadioTable[0].rsuRadioCh2 = data;
              ret = data;
          } else {
              ret = RSEMIB_SYSCALL_FAIL;
          }
      } else { /* CV2X is fixed channel. */
          if(RSU_CV2X_CHANNEL_DEFAULT == data){
              ntcip_1218_mib.rsuRadioTable[1].rsuRadioCh2 = data;
              ret = data;
          } else {
              ret = RSEMIB_SYSCALL_FAIL;
          }
      }
#endif
      ntcip1218_unlock();
  } else {
      ret = RSEMIB_LOCK_FAIL;
  } 
  if(ret < RSEMIB_OK)
      set_ntcip_1218_error_states(ret);
  return ret;
}
int32_t preserve_rsuRadioCh2(int32_t radio)
{
  if((radio < 1) || (RSU_RADIOS_MAX < radio)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      prior_ntcip_1218_mib.rsuRadioTable[radio-1].rsuRadioCh2 = ntcip_1218_mib.rsuRadioTable[radio-1].rsuRadioCh2;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t action_rsuRadioCh2(int32_t radio,int32_t data)
{
  int32_t ret = RSEMIB_OK;
  if((radio < 1) || (RSU_RADIOS_MAX < radio)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if((data < RSU_RADIO_CHANNEL_MIN) || (RSU_RADIO_CHANNEL_MAX < data)){ 
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      /* Must match ch1. */
      if(ntcip_1218_mib.rsuRadioTable[radio-1].rsuRadioCh1 == data) {
          tmp_ntcip_1218_mib.rsuRadioTable[radio-1].rsuRadioCh2 = data;
          ret = RSEMIB_OK;
      } else {
           ret = RSEMIB_BAD_DATA;
      }
      ntcip1218_unlock();
      return ret;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t commit_rsuRadioCh2(int32_t radio)
{
  int32_t ret = RSEMIB_OK;
#if !defined (PLATFORM_HD_RSU_5940)
  char_t  request_buf[150]; /* If you need more then increase for your needs. */
  char_t  reply_buf[50];   
  FILE   *fp;
#endif

  if((radio < 1) || (RSU_RADIOS_MAX < radio)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      ret = RSEMIB_BAD_INDEX;
  } else {
      if(RSEMIB_OK == ntcip1218_lock()){
          ntcip_1218_mib.rsuRadioTable[radio-1].rsuRadioCh2 = tmp_ntcip_1218_mib.rsuRadioTable[radio-1].rsuRadioCh2; 
          tmp_ntcip_1218_mib.rsuRadioTable[radio-1].rsuRadioCh2 = RSU_RADIO_CHANNEL_MIN;
#if !defined (PLATFORM_HD_RSU_5940)
          memset(reply_buf,'\0',sizeof(reply_buf));
          snprintf(request_buf,sizeof(request_buf), DCU_WRITE_DSRC_CHAN, ntcip_1218_mib.rsuRadioTable[radio-1].rsuRadioCh2);
          if(NULL != (fp = popen(request_buf, "r"))){
              if(NULL != fgets(reply_buf, sizeof(reply_buf), fp)){
                  ret = RSEMIB_OK;
              } else {
                  DEBUGMSGTL((MY_NAME, "commit_rsuRadioCh2: WRITE fgets failed.\n"));
                  ret = RSEMIB_SYSCALL_FAIL;
              }
              pclose(fp);
          } else {
              DEBUGMSGTL((MY_NAME, "commit_rsuRadioCh2: WRITE request failed.\n"));
              ret = RSEMIB_SYSCALL_FAIL;
          }

          if(RSEMIB_OK == ret){
              memset(reply_buf,'\0',sizeof(reply_buf));
              snprintf(request_buf,sizeof(request_buf), DCU_READ_DSRC_CHAN);
              if(NULL != (fp = popen(request_buf, "r"))){
                  if(NULL != fgets(reply_buf, sizeof(reply_buf), fp)){
                      if(ntcip_1218_mib.rsuRadioTable[radio-1].rsuRadioCh2 == strtod(reply_buf, NULL)) {
                          ret = RSEMIB_OK;
                      } else {
                          DEBUGMSGTL((MY_NAME, "commit_rsuRadioCh2: READ failed=[%s] wanted[%d].\n",
                              reply_buf, ntcip_1218_mib.rsuRadioTable[radio-1].rsuRadioCh2));
                          ret = RSEMIB_SYSCALL_FAIL;
                      }
                  } else {
                      DEBUGMSGTL((MY_NAME, "commit_rsuRadioCh2: READ fgets failed.\n"));
                      ret = RSEMIB_SYSCALL_FAIL;
                  }
                  pclose(fp);
              } else {
                  DEBUGMSGTL((MY_NAME, "commit_rsuRadioCh2: READ request failed.\n"));
                  ret = RSEMIB_SYSCALL_FAIL;
              }
          }
#endif
          ntcip1218_unlock();
      } else {
          ret = RSEMIB_LOCK_FAIL;
      }
  }
  if(ret < RSEMIB_OK)
      set_ntcip_1218_error_states(ret);
  return ret;
}
int32_t undo_rsuRadioCh2(int32_t radio)
{
  if((radio < 1) || (RSU_RADIOS_MAX < radio)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuRadioTable[radio-1].rsuRadioCh2 = prior_ntcip_1218_mib.rsuRadioTable[radio-1].rsuRadioCh2;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}

/* 5.2.2.10 */
int32_t get_rsuRadioTxPower1(int32_t radio, int32_t * data_out)
{
  int32_t ret = RSEMIB_OK;
  char_t  request_buf[150]; /* If you need more then increase for your needs. */
  char_t  reply_buf[50];
  FILE   *fp;

  int32_t data = RSU_RADIO_TX_POWER_MIN;
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if((radio < 1) || (RSU_RADIOS_MAX < radio)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  memset(reply_buf,'\0',sizeof(reply_buf));
  snprintf(request_buf,sizeof(request_buf), DCU_READ_I2V_TX_POWER);
  if(NULL != (fp = popen(request_buf, "r"))){
     if(NULL != fgets(reply_buf, sizeof(reply_buf), fp)){
          data = strtod(reply_buf, NULL);
          if((RSU_RADIO_TX_POWER_MIN <= data) && (data <= RSU_RADIO_TX_POWER_MAX)){
              ret = RSEMIB_OK;
          } else {
              DEBUGMSGTL((MY_NAME, "get_rsuRadioTxPower1: bad data = %d.\n",data));
              ret = RSEMIB_SYSCALL_FAIL;
          }
      } else {
          DEBUGMSGTL((MY_NAME, "get_rsuRadioTxPower1: READ fgets failed.\n"));
          ret = RSEMIB_SYSCALL_FAIL;
      }
      pclose(fp);
  } else {
      DEBUGMSGTL((MY_NAME, "get_rsuRadioTxPower1: READ request failed.\n"));
      ret = RSEMIB_SYSCALL_FAIL;
  }
  if((RSEMIB_OK == ret) && (RSEMIB_OK == ntcip1218_lock())){
      ntcip_1218_mib.rsuRadioTable[radio-1].rsuRadioTxPower1 = data;
      *data_out = data;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      ret = RSEMIB_LOCK_FAIL;
  }
  if(ret < RSEMIB_OK)
      set_ntcip_1218_error_states(ret);
  return ret;
}
int32_t preserve_rsuRadioTxPower1(int32_t radio)
{
  if((radio < 1) || (RSU_RADIOS_MAX < radio)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      prior_ntcip_1218_mib.rsuRadioTable[radio-1].rsuRadioTxPower1 = ntcip_1218_mib.rsuRadioTable[radio-1].rsuRadioTxPower1;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t action_rsuRadioTxPower1(int32_t radio, int32_t data_in)
{
  if((radio < 1) || (RSU_RADIOS_MAX < radio)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if((data_in < RSU_RADIO_TX_POWER_MIN) || (RSU_RADIO_TX_POWER_MAX < data_in)){ 
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  }
  if(RSEMIB_OK == ntcip1218_lock()){  
      tmp_ntcip_1218_mib.rsuRadioTable[radio-1].rsuRadioTxPower1 = data_in;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t commit_rsuRadioTxPower1(int32_t radio)
{
  int32_t ret = RSEMIB_OK;
  char_t  request_buf[150]; /* If you need more then increase for your needs. */
  char_t  reply_buf[50];  
  FILE   *fp;

  if((radio < 1) || (RSU_RADIOS_MAX < radio)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      ret = RSEMIB_BAD_INDEX;
  } else {
      if(RSEMIB_OK == ntcip1218_lock()){
          ntcip_1218_mib.rsuRadioTable[radio-1].rsuRadioTxPower1 = tmp_ntcip_1218_mib.rsuRadioTable[radio-1].rsuRadioTxPower1; 
          ntcip_1218_mib.rsuRadioTable[radio-1].rsuRadioTxPower2 = ntcip_1218_mib.rsuRadioTable[radio-1].rsuRadioTxPower1;
          tmp_ntcip_1218_mib.rsuRadioTable[radio-1].rsuRadioTxPower1 = RSU_RADIO_TX_POWER_MIN;

          memset(reply_buf,'\0',sizeof(reply_buf));
          snprintf(request_buf,sizeof(request_buf), DCU_WRITE_I2V_TX_POWER, ntcip_1218_mib.rsuRadioTable[radio-1].rsuRadioTxPower1);
          if(NULL != (fp = popen(request_buf, "r"))){
              if(NULL != fgets(reply_buf, sizeof(reply_buf), fp)){
                  ret = RSEMIB_OK;
              } else {
                  DEBUGMSGTL((MY_NAME, "commit_rsuRadioTxPower1: WRITE fgets failed.\n"));
                  ret = RSEMIB_SYSCALL_FAIL;
              }
              pclose(fp);
          } else {
              DEBUGMSGTL((MY_NAME, "commit_rsuRadioTxPower1: WRITE request failed.\n"));
              ret = RSEMIB_SYSCALL_FAIL;
          }

          if(RSEMIB_OK == ret){
              memset(reply_buf,'\0',sizeof(reply_buf));
              snprintf(request_buf,sizeof(request_buf), DCU_READ_I2V_TX_POWER);
              if(NULL != (fp = popen(request_buf, "r"))){
                  if(NULL != fgets(reply_buf, sizeof(reply_buf), fp)){
                      if(ntcip_1218_mib.rsuRadioTable[radio-1].rsuRadioTxPower1 == strtod(reply_buf, NULL)) {
                          ret = RSEMIB_OK;
                      } else {
                          DEBUGMSGTL((MY_NAME, "commit_rsuRadioTxPower1: READ failed=[%s] wanted[%d].\n",
                              reply_buf, ntcip_1218_mib.rsuRadioTable[radio-1].rsuRadioTxPower1));
                          ret = RSEMIB_SYSCALL_FAIL;
                      }
                  } else {
                      DEBUGMSGTL((MY_NAME, "commit_rsuRadioTxPower1: READ fgets failed.\n"));
                      ret = RSEMIB_SYSCALL_FAIL;
                  }
                  pclose(fp);
              } else {
                  DEBUGMSGTL((MY_NAME, "commit_rsuRadioTxPower1: READ request failed.\n"));
                  ret = RSEMIB_SYSCALL_FAIL;
              }
          }
          ntcip1218_unlock();
      } else {
          ret = RSEMIB_LOCK_FAIL;
      }
  }
  if(ret < RSEMIB_OK)
      set_ntcip_1218_error_states(ret);
  return ret;
}
int32_t undo_rsuRadioTxPower1(int32_t radio)
{
  if((radio < 1) || (RSU_RADIOS_MAX < radio)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuRadioTable[radio-1].rsuRadioTxPower1 = prior_ntcip_1218_mib.rsuRadioTable[radio-1].rsuRadioTxPower1;
      ntcip_1218_mib.rsuRadioTable[radio-1].rsuRadioTxPower2 = ntcip_1218_mib.rsuRadioTable[radio-1].rsuRadioTxPower1;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}

/* 5.2.2.11 */
int32_t get_rsuRadioTxPower2(int32_t radio, int32_t * data_out)
{
  int32_t ret = RSEMIB_OK;
  char_t  request_buf[150]; /* If you need more then increase for your needs. */
  char_t  reply_buf[50];
  FILE   *fp;
  int32_t data = RSU_RADIO_TX_POWER_MIN;
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if((radio < 1) || (RSU_RADIOS_MAX < radio)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  memset(reply_buf,'\0',sizeof(reply_buf));
  snprintf(request_buf,sizeof(request_buf), DCU_READ_I2V_TX_POWER);
  if(NULL != (fp = popen(request_buf, "r"))){
     if(NULL != fgets(reply_buf, sizeof(reply_buf), fp)){
          data = strtod(reply_buf, NULL);
          if((RSU_RADIO_TX_POWER_MIN <= data) && (data <= RSU_RADIO_TX_POWER_MAX)){
              ret = RSEMIB_OK;
          } else {
              DEBUGMSGTL((MY_NAME, "get_rsuRadioTxPower2: bad data = %d.\n",data));
              ret = RSEMIB_SYSCALL_FAIL;
          }
      } else {
          DEBUGMSGTL((MY_NAME, "get_rsuRadioTxPower2: READ fgets failed.\n"));
          ret = RSEMIB_SYSCALL_FAIL;
      }
      pclose(fp);
  } else {
      DEBUGMSGTL((MY_NAME, "get_rsuRadioTxPower2: READ request failed.\n"));
      ret = RSEMIB_SYSCALL_FAIL;
  }
  if((RSEMIB_OK == ret) && (RSEMIB_OK == ntcip1218_lock())){
      ntcip_1218_mib.rsuRadioTable[radio-1].rsuRadioTxPower2 = data;
      *data_out = data;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      ret = RSEMIB_LOCK_FAIL;
  }
  if(ret < RSEMIB_OK)
      set_ntcip_1218_error_states(ret);
  return ret;
}
int32_t preserve_rsuRadioTxPower2(int32_t radio)
{
  if((radio < 1) || (RSU_RADIOS_MAX < radio)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      prior_ntcip_1218_mib.rsuRadioTable[radio-1].rsuRadioTxPower2 = ntcip_1218_mib.rsuRadioTable[radio-1].rsuRadioTxPower2;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t action_rsuRadioTxPower2(int32_t radio, int32_t data_in)
{
  int32_t ret = RSEMIB_OK;
  if((radio < 1) || (RSU_RADIOS_MAX < radio)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if((data_in < RSU_RADIO_TX_POWER_MIN) || (RSU_RADIO_TX_POWER_MAX < data_in)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  }
  if(RSEMIB_OK == ntcip1218_lock()){ 
      /* tx2 always equals tx1. */
      if(ntcip_1218_mib.rsuRadioTable[radio-1].rsuRadioTxPower1 == data_in) {
          tmp_ntcip_1218_mib.rsuRadioTable[radio-1].rsuRadioTxPower2 = data_in;
          ret = RSEMIB_OK;
      } else {
         ret = RSEMIB_BAD_DATA;
      }
      ntcip1218_unlock();
      return ret;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t commit_rsuRadioTxPower2(int32_t radio){
  int32_t ret = RSEMIB_OK;
  char_t  request_buf[150]; /* If you need more then increase for your needs. */
  char_t  reply_buf[50];   
  FILE   *fp;

  if((radio < 1) || (RSU_RADIOS_MAX < radio)){
      ret = RSEMIB_BAD_INDEX;
  } else {
      if(RSEMIB_OK == ntcip1218_lock()){
          ntcip_1218_mib.rsuRadioTable[radio-1].rsuRadioTxPower2 = tmp_ntcip_1218_mib.rsuRadioTable[radio-1].rsuRadioTxPower2; 
          tmp_ntcip_1218_mib.rsuRadioTable[radio-1].rsuRadioTxPower2 = RSU_RADIO_TX_POWER_MIN;

          memset(reply_buf,'\0',sizeof(reply_buf));
          snprintf(request_buf,sizeof(request_buf), DCU_WRITE_I2V_TX_POWER, ntcip_1218_mib.rsuRadioTable[radio-1].rsuRadioTxPower2);
          if(NULL != (fp = popen(request_buf, "r"))){
              if(NULL != fgets(reply_buf, sizeof(reply_buf), fp)){
                  ret = RSEMIB_OK;
              } else {
                  DEBUGMSGTL((MY_NAME, "commit_rsuRadioTxPower2: WRITE fgets failed.\n"));
                  ret = RSEMIB_SYSCALL_FAIL;
              }
              pclose(fp);
          } else {
              DEBUGMSGTL((MY_NAME, "commit_rsuRadioTxPower2: WRITE request failed.\n"));
              ret = RSEMIB_SYSCALL_FAIL;
          }

          if(RSEMIB_OK == ret){
              memset(reply_buf,'\0',sizeof(reply_buf));
              snprintf(request_buf,sizeof(request_buf), DCU_READ_I2V_TX_POWER);
              if(NULL != (fp = popen(request_buf, "r"))){
                  if(NULL != fgets(reply_buf, sizeof(reply_buf), fp)){
                      if(ntcip_1218_mib.rsuRadioTable[radio-1].rsuRadioTxPower2 == strtod(reply_buf, NULL)) {
                          ret = RSEMIB_OK;
                      } else {
                          DEBUGMSGTL((MY_NAME, "commit_rsuRadioTxPower2: READ failed=[%s] wanted[%d].\n",
                              reply_buf, ntcip_1218_mib.rsuRadioTable[radio-1].rsuRadioTxPower2));
                          ret = RSEMIB_SYSCALL_FAIL;
                      }
                  } else {
                      DEBUGMSGTL((MY_NAME, "commit_rsuRadioTxPower2: READ fgets failed.\n"));
                      ret = RSEMIB_SYSCALL_FAIL;
                  }
                  pclose(fp);
              } else {
                  DEBUGMSGTL((MY_NAME, "commit_rsuRadioTxPower2: READ request failed.\n"));
                  ret = RSEMIB_SYSCALL_FAIL;
              }
          }
          ntcip1218_unlock();
      } else {
          ret = RSEMIB_LOCK_FAIL;
      }
  }
  if(ret < RSEMIB_OK)
      set_ntcip_1218_error_states(ret);
  return ret;
}
int32_t undo_rsuRadioTxPower2(int32_t radio)
{
  if((radio < 1) || (RSU_RADIOS_MAX < radio)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuRadioTable[radio-1].rsuRadioTxPower2 = prior_ntcip_1218_mib.rsuRadioTable[radio-1].rsuRadioTxPower2;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}

/****************************************************************************** 
 * 5.3 RSU GNSS: {rsu 2}: 
 ******************************************************************************/
int32_t get_rsuGnssStatus(void)
{
  int32_t data = 0;
  if(RSEMIB_OK == ntcip1218_lock()){
      data = ntcip_1218_mib.rsuGnssStatus;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  } 
  if((GNSS_STATUS_MIN <= data) && (data <= GNSS_STATUS_MAX)){
      return data; 
  } else {
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } 
}

int32_t get_rsuGnssAugmentation(rsuGnssAugmentation_e * data_out)
{
  rsuGnssAugmentation_e data = rsuGnssAugmentation_none;
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      data = ntcip_1218_mib.rsuGnssAugmentation;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  } 
  if((rsuGnssAugmentation_other <= data) && (data <= rsuGnssAugmentation_waas)){
      *data_out = data;
      return RSEMIB_OK; 
  } else {
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } 
}

/****************************************************************************** 
 * 5.4 Store and Repeat Messages: { rsu 3 }: rsuMsgRepeatStatusTable
 ******************************************************************************/

/* These are not locked so if you call its a good idea to lock first. Live pointer. */
RsuMsgRepeatStatusEntry_t * get_rsuMsgRepeatTable(void)
{
    return (RsuMsgRepeatStatusEntry_t *)&ntcip_1218_mib.rsuMsgRepeatStatusTable[0];
}

/* 5.4.1 */
int32_t get_maxRsuMsgRepeat(void)
{
  int32_t data = RSU_SAR_MESSAGE_MAX;
  if(RSEMIB_OK == ntcip1218_lock()){
      data = ntcip_1218_mib.maxRsuMsgRepeat;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  } 
  if((RSU_SAR_MESSAGE_MIN <= data) && (data <= RSU_SAR_MESSAGE_MAX)){
      return data; 
  } else {
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } 
}

/* 5.4.2.1 */
int32_t  get_rsuMsgRepeatIndex(int32_t index, int32_t * data_out)
{
  if((index < RSU_SAR_MESSAGE_MIN) || (RSU_SAR_MESSAGE_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      *data_out = ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].rsuMsgRepeatIndex;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
  if((*data_out < RSU_SAR_MESSAGE_MIN) || (RSU_SAR_MESSAGE_MAX < *data_out)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } 
  return RSEMIB_OK;
}

/* 5.4.2.2 */
int32_t get_rsuMsgRepeatPsid(int32_t index, uint8_t * data_out)
{
  int32_t length=0;
  if((index < RSU_SAR_MESSAGE_MIN) || (RSU_SAR_MESSAGE_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      length = ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].rsuMsgRepeatPsid_length;
      if(   (RSU_RADIO_PSID_MIN_SIZE <= length) 
         && (length <= RSU_RADIO_PSID_SIZE)){
          memcpy(data_out, ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].rsuMsgRepeatPsid, length);
          ntcip1218_unlock();
          return length;
      } else {
          ntcip1218_unlock();
          return RSEMIB_BAD_DATA;
      }
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t preserve_rsuMsgRepeatPsid(int32_t index)
{
  if((index < RSU_SAR_MESSAGE_MIN) || (RSU_SAR_MESSAGE_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      memcpy( prior_ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].rsuMsgRepeatPsid
            , ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].rsuMsgRepeatPsid, RSU_RADIO_PSID_SIZE);
      prior_ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].rsuMsgRepeatPsid_length = 
          ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].rsuMsgRepeatPsid_length;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t action_rsuMsgRepeatPsid(int32_t index, uint8_t * data_in, int32_t length)
{
  uint32_t psid = 0x0;
  if((index < RSU_SAR_MESSAGE_MIN) || (RSU_SAR_MESSAGE_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  } 
  if(NULL == data_in){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(( length < RSU_RADIO_PSID_MIN_SIZE) || (RSU_RADIO_PSID_SIZE < length)){
      return RSEMIB_BAD_INPUT;  
  }
  /* Reject known bogus PSID's. */
  if(length >= 1) {
      psid  = (uint32_t)(data_in[0]);
  }
  if(length >= 2) {
      psid  = psid << 8;
      psid  |= (uint32_t)(data_in[1]);
  }
  if(length >= 3) {
      psid  = psid << 8;
      psid  |= (uint32_t)(data_in[2]);
  }
  if(length == 4) {
      psid  = psid << 8;
      psid  |= (uint32_t)(data_in[3]);
  }
  if(psid <= (uint32_t) PSID_1BYTE_MAX_VALUE){
  } else if(psid >= (uint32_t)PSID_2BYTE_MIN_VALUE && psid <= (uint32_t)PSID_2BYTE_MAX_VALUE){
  } else if(psid >= (uint32_t)PSID_3BYTE_MIN_VALUE && psid <= (uint32_t)PSID_3BYTE_MAX_VALUE){
  } else if(psid >= (uint32_t)PSID_4BYTE_MIN_VALUE && psid <= (uint32_t)PSID_4BYTE_MAX_VALUE){
  } else {
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      memset(tmp_ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].rsuMsgRepeatPsid,0x0,RSU_RADIO_PSID_SIZE);
      if(1 == length) {
          tmp_ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].rsuMsgRepeatPsid[3] = data_in[0];
      }
      if(2 == length){
          tmp_ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].rsuMsgRepeatPsid[3] = data_in[1];
          tmp_ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].rsuMsgRepeatPsid[2] = data_in[0];
      }
      if(3 == length) {
          tmp_ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].rsuMsgRepeatPsid[3] = data_in[2];
          tmp_ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].rsuMsgRepeatPsid[2] = data_in[1];
          tmp_ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].rsuMsgRepeatPsid[1] = data_in[0];
      }
      if(4 == length) {
          tmp_ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].rsuMsgRepeatPsid[3] = data_in[3];
          tmp_ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].rsuMsgRepeatPsid[2] = data_in[2];
          tmp_ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].rsuMsgRepeatPsid[1] = data_in[1];
          tmp_ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].rsuMsgRepeatPsid[0] = data_in[0];
      }
      tmp_ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].rsuMsgRepeatPsid_length = length;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t commit_rsuMsgRepeatPsid(int32_t index)
{
  int32_t ret = RSEMIB_OK;
  if((index < RSU_SAR_MESSAGE_MIN) || (RSU_SAR_MESSAGE_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  } 
  if(RSEMIB_OK == ntcip1218_lock()){
      memcpy( ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].rsuMsgRepeatPsid
            , tmp_ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].rsuMsgRepeatPsid, RSU_RADIO_PSID_SIZE);
      ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].rsuMsgRepeatPsid_length = 
          tmp_ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].rsuMsgRepeatPsid_length;

      memset(tmp_ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].rsuMsgRepeatPsid, 0x0, RSU_RADIO_PSID_SIZE);
      tmp_ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].rsuMsgRepeatPsid_length = 0;

      /* Update file on disk. */
      if(RSEMIB_OK != commit_msgRepeat_to_disk(&ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1])){
          DEBUGMSGTL((MY_NAME, "commit_msgRepeat_to_disk: FAIL: index=%d dir=[%s].\n", index, ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].filePathName));
          ret = RSEMIB_BAD_DATA;
      }
      ntcip1218_unlock();
      return ret;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t undo_rsuMsgRepeatPsid(int32_t index)
{
  if((index < RSU_SAR_MESSAGE_MIN) || (RSU_SAR_MESSAGE_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      memcpy( ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].rsuMsgRepeatPsid
            , prior_ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].rsuMsgRepeatPsid , RSU_RADIO_PSID_SIZE);
      ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].rsuMsgRepeatPsid_length = 
          prior_ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].rsuMsgRepeatPsid_length;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}

/* 5.4.2.3 */
int32_t get_rsuMsgRepeatTxChannel(int32_t index, int32_t * data_out)
{
  if((index < RSU_SAR_MESSAGE_MIN) || (RSU_SAR_MESSAGE_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      *data_out = ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].rsuMsgRepeatTxChannel;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
  if((*data_out < RSU_RADIO_CHANNEL_MIN) || (RSU_RADIO_CHANNEL_MAX < *data_out)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } 
  return RSEMIB_OK;

}
int32_t preserve_rsuMsgRepeatTxChannel(int32_t index)
{
  if((index < RSU_SAR_MESSAGE_MIN) || (RSU_SAR_MESSAGE_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      prior_ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].rsuMsgRepeatTxChannel = 
          ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].rsuMsgRepeatTxChannel;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t action_rsuMsgRepeatTxChannel(int32_t index, int32_t data_in)
{
  int32_t ret = RSEMIB_OK;
  if((index < RSU_SAR_MESSAGE_MIN) || (RSU_SAR_MESSAGE_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if((data_in < RSU_RADIO_CHANNEL_MIN) || (RSU_RADIO_CHANNEL_MAX < data_in)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
#if defined(PLATFORM_HD_RSU_5940)
      if(data_in != ntcip_1218_mib.rsuRadioTable[RSU_CV2X_RADIO-1].rsuRadioCh1) /* CV2X. */
          ret = RSEMIB_BAD_DATA;
#else
      if(mib_on == ntcip_1218_mib.rsuRadioTable[0].rsuRadioEnable) {
          if(data_in != ntcip_1218_mib.rsuRadioTable[0].rsuRadioCh1) /* DSRC. */
              ret = RSEMIB_BAD_DATA;
      } else {
          if(data_in != ntcip_1218_mib.rsuRadioTable[1].rsuRadioCh1) /* CV2X. */
              ret = RSEMIB_BAD_DATA;
      }
#endif
      if(RSEMIB_OK == ret){
          tmp_ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].rsuMsgRepeatTxChannel = data_in;
      }
      ntcip1218_unlock();
      return ret;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t commit_rsuMsgRepeatTxChannel(int32_t index)
{
  int32_t ret = RSEMIB_OK;
  if((index < RSU_SAR_MESSAGE_MIN) || (RSU_SAR_MESSAGE_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].rsuMsgRepeatTxChannel = 
          tmp_ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].rsuMsgRepeatTxChannel;
      tmp_ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].rsuMsgRepeatTxChannel = RSU_RADIO_CHANNEL_MIN;

      /* Update file on disk. */
      if(RSEMIB_OK != commit_msgRepeat_to_disk(&ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1])){
          DEBUGMSGTL((MY_NAME, "commit_msgRepeat_to_disk: FAIL: index=%d dir=[%s].\n", index, ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].filePathName));
          ret = RSEMIB_BAD_DATA;
      }
      ntcip1218_unlock();
      return ret;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t undo_rsuMsgRepeatTxChannel(int32_t index)
{
  if((index < RSU_SAR_MESSAGE_MIN) || (RSU_SAR_MESSAGE_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].rsuMsgRepeatTxChannel = 
          prior_ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].rsuMsgRepeatTxChannel;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
/* 5.4.2.4 */
int32_t get_rsuMsgRepeatTxInterval(int32_t index, int32_t * data_out)
{
  if((index < RSU_SAR_MESSAGE_MIN) || (RSU_SAR_MESSAGE_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      *data_out = ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].rsuMsgRepeatTxInterval;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
  if((*data_out < RSU_MSG_REPEAT_TX_INTERVAL_MIN) || (RSU_MSG_REPEAT_TX_INTERVAL_MAX < *data_out)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } 
  return RSEMIB_OK;

}
int32_t preserve_rsuMsgRepeatTxInterval(int32_t index)
{
  if((index < RSU_SAR_MESSAGE_MIN) || (RSU_SAR_MESSAGE_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      prior_ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].rsuMsgRepeatTxInterval = 
          ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].rsuMsgRepeatTxInterval;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t action_rsuMsgRepeatTxInterval(int32_t index, int32_t data_in)
{
  if((index < RSU_SAR_MESSAGE_MIN) || (RSU_SAR_MESSAGE_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if((data_in < RSU_MSG_REPEAT_TX_INTERVAL_MIN) || (RSU_MSG_REPEAT_TX_INTERVAL_MAX < data_in)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  }
  if(RSEMIB_OK == ntcip1218_lock()){  
      tmp_ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].rsuMsgRepeatTxInterval = data_in;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t commit_rsuMsgRepeatTxInterval(int32_t index)
{
  int32_t ret = RSEMIB_OK;
  if((index < RSU_SAR_MESSAGE_MIN) || (RSU_SAR_MESSAGE_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].rsuMsgRepeatTxInterval = 
          tmp_ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].rsuMsgRepeatTxInterval;
      tmp_ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].rsuMsgRepeatTxInterval = 0;

      /* Update file on disk. */
      if(RSEMIB_OK != commit_msgRepeat_to_disk(&ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1])){
          DEBUGMSGTL((MY_NAME, "commit_msgRepeat_to_disk: FAIL: index=%d dir=[%s].\n", index, ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].filePathName));
          ret = RSEMIB_BAD_DATA;
      }
      ntcip1218_unlock();
      return ret;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t undo_rsuMsgRepeatTxInterval(int32_t index)
{
  if((index < RSU_SAR_MESSAGE_MIN) || (RSU_SAR_MESSAGE_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].rsuMsgRepeatTxInterval = 
          prior_ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].rsuMsgRepeatTxInterval;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}

/* 5.4.2.5 */
int32_t get_rsuMsgRepeatDeliveryStart(int32_t index, uint8_t * data_out)
{
  if((index < RSU_SAR_MESSAGE_MIN) || (RSU_SAR_MESSAGE_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      memcpy(data_out, ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].rsuMsgRepeatDeliveryStart, MIB_DATEANDTIME_LENGTH);
      ntcip1218_unlock();
      return MIB_DATEANDTIME_LENGTH;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t preserve_rsuMsgRepeatDeliveryStart(int32_t index)
{
  if((index < RSU_SAR_MESSAGE_MIN) || (RSU_SAR_MESSAGE_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      memcpy( prior_ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].rsuMsgRepeatDeliveryStart
            , ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].rsuMsgRepeatDeliveryStart, MIB_DATEANDTIME_LENGTH);
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t action_rsuMsgRepeatDeliveryStart(int32_t index, uint8_t * data_in, int32_t length)
{
  if((index < RSU_SAR_MESSAGE_MIN) || (RSU_SAR_MESSAGE_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(NULL == data_in){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(MIB_DATEANDTIME_LENGTH != length){
      return RSEMIB_BAD_INPUT;  
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      memset(tmp_ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].rsuMsgRepeatDeliveryStart, 0x0, MIB_DATEANDTIME_LENGTH);
      memcpy(tmp_ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].rsuMsgRepeatDeliveryStart, data_in, length);
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t commit_rsuMsgRepeatDeliveryStart(int32_t index)
{
  int32_t ret = RSEMIB_OK;
  if((index < RSU_SAR_MESSAGE_MIN) || (RSU_SAR_MESSAGE_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      memcpy( ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].rsuMsgRepeatDeliveryStart
            , tmp_ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].rsuMsgRepeatDeliveryStart, MIB_DATEANDTIME_LENGTH);
      memset(tmp_ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].rsuMsgRepeatDeliveryStart, 0x0, MIB_DATEANDTIME_LENGTH);

      /* Update file on disk. */
      if(RSEMIB_OK != commit_msgRepeat_to_disk(&ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1])){
          DEBUGMSGTL((MY_NAME, "commit_msgRepeat_to_disk: FAIL: index=%d dir=[%s].\n", index, ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].filePathName));
          ret = RSEMIB_BAD_DATA;
      }
      ntcip1218_unlock();
      return ret;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t undo_rsuMsgRepeatDeliveryStart(int32_t index)
{
  if((index < RSU_SAR_MESSAGE_MIN) || (RSU_SAR_MESSAGE_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      memcpy( ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].rsuMsgRepeatDeliveryStart
            , prior_ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].rsuMsgRepeatDeliveryStart, MIB_DATEANDTIME_LENGTH);
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}

/* 5.4.2.6 */
int32_t get_rsuMsgRepeatDeliveryStop(int32_t index, uint8_t * data_out)
{
  if((index < RSU_SAR_MESSAGE_MIN) || (RSU_SAR_MESSAGE_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      memcpy(data_out, ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].rsuMsgRepeatDeliveryStop, MIB_DATEANDTIME_LENGTH);
      ntcip1218_unlock();
      return MIB_DATEANDTIME_LENGTH;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t preserve_rsuMsgRepeatDeliveryStop(int32_t index)
{
  if((index < RSU_SAR_MESSAGE_MIN) || (RSU_SAR_MESSAGE_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      memcpy( prior_ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].rsuMsgRepeatDeliveryStop
            , ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].rsuMsgRepeatDeliveryStop, MIB_DATEANDTIME_LENGTH);
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t action_rsuMsgRepeatDeliveryStop(int32_t index, uint8_t * data_in, int32_t length)
{
  if((index < RSU_SAR_MESSAGE_MIN) || (RSU_SAR_MESSAGE_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(NULL == data_in){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(MIB_DATEANDTIME_LENGTH != length){
      return RSEMIB_BAD_INPUT;  
  }
  if(RSEMIB_OK == ntcip1218_lock()){ 
      memset(tmp_ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].rsuMsgRepeatDeliveryStop, 0x0, MIB_DATEANDTIME_LENGTH);
      memcpy(tmp_ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].rsuMsgRepeatDeliveryStop, data_in, length);
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t commit_rsuMsgRepeatDeliveryStop(int32_t index)
{
  int32_t ret = RSEMIB_OK;
  if((index < RSU_SAR_MESSAGE_MIN) || (RSU_SAR_MESSAGE_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      memcpy( ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].rsuMsgRepeatDeliveryStop
            , tmp_ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].rsuMsgRepeatDeliveryStop, MIB_DATEANDTIME_LENGTH);
      memset(tmp_ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].rsuMsgRepeatDeliveryStop, 0x0, MIB_DATEANDTIME_LENGTH);

      /* Update file on disk. */
      if(RSEMIB_OK != commit_msgRepeat_to_disk(&ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1])){
          DEBUGMSGTL((MY_NAME, "commit_msgRepeat_to_disk: FAIL: index=%d dir=[%s].\n", index, ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].filePathName));
          ret = RSEMIB_BAD_DATA;
      }
      ntcip1218_unlock();
      return ret;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t undo_rsuMsgRepeatDeliveryStop(int32_t index)
{
  if((index < RSU_SAR_MESSAGE_MIN) || (RSU_SAR_MESSAGE_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      memcpy( ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].rsuMsgRepeatDeliveryStop
            , prior_ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].rsuMsgRepeatDeliveryStop, MIB_DATEANDTIME_LENGTH);
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}

/* 5.4.2.7 */
int32_t get_rsuMsgRepeatPayload(int32_t index, uint8_t * data_out)
{
  int32_t length = 0;
  if((index < RSU_SAR_MESSAGE_MIN) || (RSU_SAR_MESSAGE_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      length = ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].rsuMsgRepeatPayload_length;
      if(   (RSU_MSG_PAYLOAD_MIN <= length) 
         && (length <= RSU_MSG_PAYLOAD_MAX)){
          memcpy(data_out, ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].rsuMsgRepeatPayload, length);
          ntcip1218_unlock();
          return length;
      } else {
          ntcip1218_unlock();
          return RSEMIB_BAD_DATA;
      }
      return RSU_MSG_PAYLOAD_MAX;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t preserve_rsuMsgRepeatPayload(int32_t index)
{
  if((index < RSU_SAR_MESSAGE_MIN) || (RSU_SAR_MESSAGE_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      memcpy( prior_ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].rsuMsgRepeatPayload
            , ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].rsuMsgRepeatPayload, RSU_MSG_PAYLOAD_MAX);
      prior_ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].rsuMsgRepeatPayload_length =
          ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].rsuMsgRepeatPayload_length;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t action_rsuMsgRepeatPayload(int32_t index, uint8_t * data_in, int32_t length)
{
  if((index < RSU_SAR_MESSAGE_MIN) || (RSU_SAR_MESSAGE_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if((NULL == data_in) && (0 != length)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(( length < RSU_MSG_PAYLOAD_MIN) || (RSU_MSG_PAYLOAD_MAX < length)){
      return RSEMIB_BAD_INPUT;  
  }
  if(RSEMIB_OK == ntcip1218_lock()){  
      memset(tmp_ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].rsuMsgRepeatPayload, 0x0, RSU_MSG_PAYLOAD_MAX);
      if(NULL != data_in){
          memcpy(tmp_ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].rsuMsgRepeatPayload, data_in, length);
      }
      tmp_ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].rsuMsgRepeatPayload_length = length;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t commit_rsuMsgRepeatPayload(int32_t index)
{
  int32_t ret = RSEMIB_OK;
  if((index < RSU_SAR_MESSAGE_MIN) || (RSU_SAR_MESSAGE_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      memcpy( ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].rsuMsgRepeatPayload
            , tmp_ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].rsuMsgRepeatPayload, RSU_MSG_PAYLOAD_MAX);
      ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].rsuMsgRepeatPayload_length =
          tmp_ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].rsuMsgRepeatPayload_length;
      memset(tmp_ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].rsuMsgRepeatPayload, 0x0, RSU_MSG_PAYLOAD_MAX);
      tmp_ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].rsuMsgRepeatPayload_length = 0;

      /* Update file on disk. */
      if(RSEMIB_OK != commit_msgRepeat_to_disk(&ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1])){
          DEBUGMSGTL((MY_NAME, "commit_msgRepeat_to_disk: FAIL: index=%d dir=[%s].\n", index, ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].filePathName));
          ret = RSEMIB_BAD_DATA;
      }
      ntcip1218_unlock();
      return ret;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t undo_rsuMsgRepeatPayload(int32_t index)
{
  if((index < RSU_SAR_MESSAGE_MIN) || (RSU_SAR_MESSAGE_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      memcpy( ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].rsuMsgRepeatPayload
            , prior_ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].rsuMsgRepeatPayload, RSU_MSG_PAYLOAD_MAX);
      ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].rsuMsgRepeatPayload_length =
          prior_ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].rsuMsgRepeatPayload_length;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}

/* 5.4.2.8 */
int32_t get_rsuMsgRepeatEnable(int32_t index, int32_t * data_out)
{
  if((index < RSU_SAR_MESSAGE_MIN) || (RSU_SAR_MESSAGE_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      *data_out = (int32_t)ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].rsuMsgRepeatEnable;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
  if((*data_out < mib_off) || (mib_on < *data_out)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } 
  return RSEMIB_OK;

}
int32_t preserve_rsuMsgRepeatEnable(int32_t index)
{
  if((index < RSU_SAR_MESSAGE_MIN) || (RSU_SAR_MESSAGE_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      prior_ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].rsuMsgRepeatEnable = 
          ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].rsuMsgRepeatEnable;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t action_rsuMsgRepeatEnable(int32_t index, int32_t data_in)
{
  if((index < RSU_SAR_MESSAGE_MIN) || (RSU_SAR_MESSAGE_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if((data_in < mib_off) || (mib_on < data_in)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  }
  if(RSEMIB_OK == ntcip1218_lock()){  
      tmp_ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].rsuMsgRepeatEnable = (onOff_e)data_in;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t commit_rsuMsgRepeatEnable(int32_t index)
{
  int32_t ret = RSEMIB_OK;
  if((index < RSU_SAR_MESSAGE_MIN) || (RSU_SAR_MESSAGE_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].rsuMsgRepeatEnable = 
          tmp_ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].rsuMsgRepeatEnable;
      tmp_ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].rsuMsgRepeatEnable = mib_off;
      /* Update file on disk. */
      if(RSEMIB_OK != commit_msgRepeat_to_disk(&ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1])){
          DEBUGMSGTL((MY_NAME, "commit_msgRepeat_to_disk: FAIL: index=%d dir=[%s].\n", index, ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].filePathName));
          ret = RSEMIB_BAD_DATA;
      }
      ntcip1218_unlock();
      return ret;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t undo_rsuMsgRepeatEnable(int32_t index)
{
  if((index < RSU_SAR_MESSAGE_MIN) || (RSU_SAR_MESSAGE_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].rsuMsgRepeatEnable = 
          prior_ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].rsuMsgRepeatEnable;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}

/* 5.4.2.9 */
int32_t get_rsuMsgRepeatStatus(int32_t index, int32_t * data_out)
{
  if((index < RSU_SAR_MESSAGE_MIN) || (RSU_SAR_MESSAGE_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      *data_out = ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].rsuMsgRepeatStatus;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  } 
  if((ROW_STATUS_VALID_MIN <= *data_out) && (*data_out <= ROW_STATUS_VALID_MAX)){
      return RSEMIB_OK; 
  } else {
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } 
}
int32_t preserve_rsuMsgRepeatStatus(int32_t index)
{
  if((index < RSU_SAR_MESSAGE_MIN) || (RSU_SAR_MESSAGE_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      prior_ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].rsuMsgRepeatStatus = 
          ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].rsuMsgRepeatStatus;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t action_rsuMsgRepeatStatus(int32_t index, int32_t data_in)
{
  if((index < RSU_SAR_MESSAGE_MIN) || (RSU_SAR_MESSAGE_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if((data_in < ROW_STATUS_VALID_MIN) || (ROW_STATUS_VALID_MAX < data_in)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  }
  if(RSEMIB_OK == ntcip1218_lock()){  
      tmp_ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].rsuMsgRepeatStatus = data_in;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}

/****************************************************************************** 
  For reference, see "SNMPv2-TC.txt" keyword "RowStatus". See snmp.h for SNMP row & error codes.

 SNMPGET on Status returns these SNMP ROW values depending on state of the row.

  Status           | Access | Meaning
  ---------------------------------------------------------------------------------
  active(1)        | R      | Message is allowed to be sent by radio if all other columns valid.
  notInService(2)  | R      | No message being sent to radio regardless of state of columns.
  notReady(3)      | N/A    | N/A.

  SNMPSET on Status returns these SNMP ERR values depending on state of the row.

  Status          | Access | active(1)                  | notInService(2)            | notReady(3) | NonExistent(0)
  --------------------------------------------------------------------------------------------------------------------
  active(1)       | W      | SNMP_ERR_NOERROR           | SNMP_ERR_NOERROR           | n/a         | SNMP_ERR_INCONSISTENTVALUE
  notInService(2) | W      | SNMP_ERR_NOERROR           | SNMP_ERR_NOERROR           | n/a         | SNMP_ERR_INCONSISTENTVALUE
  createAndGo(4)  | WO     | SNMP_ERR_INCONSISTENTVALUE | SNMP_ERR_INCONSISTENTVALUE | n/a         | SNMP_ERR_NOERROR
  createAndWait(5)| WO     | SNMP_ERR_INCONSISTENTVALUE | SNMP_ERR_INCONSISTENTVALUE | n/a         | SNMP_ERR_NOERROR
  destroy(6)      | WO     | SNMP_ERR_NOERROR           | SNMP_ERR_NOERROR           | n/a         | SNMP_ERR_NOERROR
  
    (1) Allow message to be sent. 
        May not be sending to radio because payload == zero, enable == OFF, no GNSS, no certs, or !(start<now<stop). 
        Check syslog AMH output for errors.
    (2) Stop sending to radio. 
    (4) Create Row. Status goto active(1). Payload is len zero. 
        All other columns default to valid values for radio transmission.
    (5) Create Row. Status goto notInService(2). Payload is len zero. 
        All other columns default to valid values for radio transmission.
    (6) Destroy Row. Status goto NonExistent(0).

  Keep in mind handler in rsuMsgRepeat.c is also evaluating if the table and row exists. 
  We don't have to recheck for nonExistent(0) rows or if table exists here.
******************************************************************************/
int32_t commit_rsuMsgRepeatStatus(int32_t index)
{
  int32_t ret = RSEMIB_OK;
  char_t  command_buffer[100 + RSU_MSG_FILENAME_LENGTH_MAX]; /* If you need more than a 100 chars for your command then add them here. */

  if((index < RSU_SAR_MESSAGE_MIN) || (RSU_SAR_MESSAGE_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      DEBUGMSGTL((MY_NAME_EXTRA, "commit_rsuMsgRepeatStatus: index=%d.\n", index));
      switch(tmp_ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].rsuMsgRepeatStatus){
          case SNMP_ROW_CREATEANDGO:
              if(SNMP_ROW_NONEXISTENT == ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].rsuMsgRepeatStatus){
                  /* Default has to be as active as possible. */
                  set_default_row_rsuMsgRepeat(&ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1],index-1);  /* Careful to preserve index within table rows. Only call proper routine to clear. */
                  ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].rsuMsgRepeatStatus = SNMP_ROW_ACTIVE;
                  DEBUGMSGTL((MY_NAME_EXTRA, "AMH file name=[%s].",ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].filePathName));
                  /* Update file on disk. */
                  if(RSEMIB_OK != commit_msgRepeat_to_disk(&ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1])){
                      DEBUGMSGTL((MY_NAME, "commit_msgRepeat_to_disk: FAIL: index=%d dir=[%s].\n", index, ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].filePathName));
                      ret = RSEMIB_BAD_DATA;
                  } else {
                      /* Rebuild helper table before ack. */
                      if (RSEMIB_OK == rebuild_rsuMsgRepeat(&ntcip_1218_mib.rsuMsgRepeatStatusTable[0])) {
                          ret = RSEMIB_OK; /* Tell handler to ack client request. */
                      } else {
                          ret = RSEMIB_BAD_DATA;
                      }
                  }
              } else {
                  /* If it exists already then do nothing. Dont send error back. */
                  //ret = RSEMIB_BAD_MODE;
              }
              break;
          case SNMP_ROW_CREATEANDWAIT:
              if(SNMP_ROW_NONEXISTENT == ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].rsuMsgRepeatStatus){
                  /* Default has to be as active as possible. */                
                  set_default_row_rsuMsgRepeat(&ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1],index-1);  /* Careful to preserve index within table rows. Only call proper routine to clear. */
                  ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].rsuMsgRepeatStatus = SNMP_ROW_NOTINSERVICE;
                  /* Update file on disk. */
                  if(RSEMIB_OK != commit_msgRepeat_to_disk(&ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1])){
                      DEBUGMSGTL((MY_NAME, "commit_msgRepeat_to_disk: FAIL: index=%d dir=[%s].\n", index, ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].filePathName));
                      ret = RSEMIB_BAD_DATA;
                  } else {
                      /* Rebuild helper table before ack. */
                      if (RSEMIB_OK == rebuild_rsuMsgRepeat(&ntcip_1218_mib.rsuMsgRepeatStatusTable[0])) {
                          ret = RSEMIB_OK; /* Tell handler to ack client request. */
                      } else {
                          ret = RSEMIB_BAD_DATA;
                      }
                  }
              } else {
                  /* If it exists already then do nothing. Dont send error back. */
                  //ret = RSEMIB_BAD_MODE;
              }
              break;
          case SNMP_ROW_DESTROY:
              if(SNMP_ROW_NONEXISTENT == ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].rsuMsgRepeatStatus){
                  ret = RSEMIB_OK; /* If you try to DESTROY non-exixtent row that's an easy one. */
              } else {
                  /* rm from RSU. */
                  memset(command_buffer,'\0',sizeof(command_buffer));
                  snprintf(command_buffer, sizeof(command_buffer), "rm -f %s", ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].filePathName);
                  DEBUGMSGTL((MY_NAME, "DESTROY: index=%d cmd=[%s].\n", index, command_buffer));
                  if(0 != system(command_buffer)){
                      ret = RSEMIB_BAD_DATA;
                  }
                  /* Remove from MIB. Only call proper routine to clear. */
                  set_default_row_rsuMsgRepeat(&ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1],index-1);
                  set_default_row_rsuMsgRepeat(&prior_ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1],index-1);
                  set_default_row_rsuMsgRepeat(&tmp_ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1],index-1);
                  /* Rebuild helper table. AMH will not update nonExistent rows. */
                  if (RSEMIB_OK != rebuild_rsuMsgRepeat(&ntcip_1218_mib.rsuMsgRepeatStatusTable[0])){
                      ret = RSEMIB_BAD_DATA;
                  }
              }
              break;
//todo: need for ifm?
           case SNMP_ROW_ACTIVE: /* Same as enable = 1. */ 
              if(SNMP_ROW_NOTINSERVICE == ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].rsuMsgRepeatStatus){
                  ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].rsuMsgRepeatStatus =
                  tmp_ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].rsuMsgRepeatStatus;
                  /* Update file on disk. */
                  if(RSEMIB_OK != commit_msgRepeat_to_disk(&ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1])){
                      DEBUGMSGTL((MY_NAME, "commit_msgRepeat_to_disk: FAIL: index=%d dir=[%s].\n", index, ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].filePathName));
                      ret = RSEMIB_BAD_DATA;
                  } else {
                      /* Rebuild helper table before ack. */
                      if (RSEMIB_OK != rebuild_rsuMsgRepeat(&ntcip_1218_mib.rsuMsgRepeatStatusTable[0])) {
                          ret = RSEMIB_BAD_DATA;
                      }
                  }
              } else {
                  /* If it exists already then do nothing. Dont send error back. */
                  //ret = RSEMIB_BAD_MODE;
              }
              break;
          case SNMP_ROW_NOTINSERVICE:
              if(SNMP_ROW_ACTIVE == ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].rsuMsgRepeatStatus){
                  ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].rsuMsgRepeatStatus =
                  tmp_ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].rsuMsgRepeatStatus;
                  /* Update file on disk. */
                  if(RSEMIB_OK != commit_msgRepeat_to_disk(&ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1])){
                      DEBUGMSGTL((MY_NAME, "commit_msgRepeat_to_disk: FAIL: index=%d dir=[%s].\n", index, ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].filePathName));
                      ret = RSEMIB_BAD_DATA;
                  } else {
                      /* Rebuild helper table before ack. */
                      if (RSEMIB_OK != rebuild_rsuMsgRepeat(&ntcip_1218_mib.rsuMsgRepeatStatusTable[0])) {
                          ret = RSEMIB_BAD_DATA;
                      }
                  }
              } else {
                  /* If it exists already then do nothing. Dont send error back. */
                  //ret = RSEMIB_BAD_MODE;
              }    
              break;
          case SNMP_ROW_NOTREADY:
          case SNMP_ROW_NONEXISTENT:
          default:
              ret = RSEMIB_BAD_MODE;
              break;
      }
      /* Clear tmp. */
      tmp_ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].rsuMsgRepeatStatus = 0;
      ntcip1218_unlock();
      return ret;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t undo_rsuMsgRepeatStatus(int32_t index)
{
  char_t  command_buffer[100 + RSU_MSG_FILENAME_LENGTH_MAX]; /* If you need more than a 100 chars for your command then add them here. */
  if((index < RSU_SAR_MESSAGE_MIN) || (RSU_SAR_MESSAGE_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }

/* TODO: Which UNDO method is better? */
#if 0
  int32_t ret = RSEMIB_OK;
  if((index < RSU_IFM_MESSAGE_MIN) || (RSU_IFM_MESSAGE_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      /* A row in rsuIFM can only exist in two row states active and notInService. Ignore otherwise.*/
      switch(prior_ntcip_1218_mib.rsuIFMStatusTable[index-1].rsuIFMStatus) {
          case SNMP_ROW_ACTIVE:
          case SNMP_ROW_NOTINSERVICE:
              ntcip_1218_mib.rsuIFMStatusTable[index-1].rsuIFMStatus = prior_ntcip_1218_mib.rsuIFMStatusTable[index-1].rsuIFMStatus;
              break;
          case SNMP_ROW_NONEXISTENT:
          case SNMP_ROW_NOTREADY:
          case SNMP_ROW_CREATEANDGO:
          case SNMP_ROW_CREATEANDWAIT:
          case SNMP_ROW_DESTROY:
          default:
              ret = RSEMIB_BAD_MODE;
              break;
      }
      prior_ntcip_1218_mib.rsuIFMStatusTable[index-1].rsuIFMStatus = SNMP_ROW_NONEXISTENT;
      ntcip1218_unlock();
      return ret;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
#endif
  if(RSEMIB_OK == ntcip1218_lock()){
      /* Only remove from disk if prior it didn't exist. */
      if(ROW_STATUS_VALID_DEFAULT == prior_ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].rsuMsgRepeatStatus){
          memset(command_buffer,'\0',sizeof(command_buffer));
          snprintf(command_buffer, sizeof(command_buffer), "rm -f %s", ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].filePathName);
          if(0 != system(command_buffer)){
              /* File may not exist depending on where in transaction we are so failure here can happen. Ignore. */
          }
      }
      ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].rsuMsgRepeatStatus = 
          prior_ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].rsuMsgRepeatStatus;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}

/* 5.4.2.10 */
int32_t get_rsuMsgRepeatPriority(int32_t index, int32_t * data_out)
{
  if((index < RSU_SAR_MESSAGE_MIN) || (RSU_SAR_MESSAGE_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      *data_out = ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].rsuMsgRepeatPriority;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
  if((*data_out < RSU_MSG_REPEAT_PRIORITY_MIN) || (RSU_MSG_REPEAT_PRIORITY_MAX < *data_out)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } 
  return RSEMIB_OK;
}
int32_t preserve_rsuMsgRepeatPriority(int32_t index)
{
  if((index < RSU_SAR_MESSAGE_MIN) || (RSU_SAR_MESSAGE_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      prior_ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].rsuMsgRepeatPriority = 
          ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].rsuMsgRepeatPriority;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t action_rsuMsgRepeatPriority(int32_t index, int32_t data_in)
{
  if((index < RSU_SAR_MESSAGE_MIN) || (RSU_SAR_MESSAGE_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if((data_in < RSU_MSG_REPEAT_PRIORITY_MIN) || (RSU_MSG_REPEAT_PRIORITY_MAX < data_in)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  }
  if(RSEMIB_OK == ntcip1218_lock()){  
      tmp_ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].rsuMsgRepeatPriority = data_in;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t commit_rsuMsgRepeatPriority(int32_t index)
{
  int32_t ret = RSEMIB_OK;
  if((index < RSU_SAR_MESSAGE_MIN) || (RSU_SAR_MESSAGE_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].rsuMsgRepeatPriority = 
          tmp_ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].rsuMsgRepeatPriority;
      tmp_ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].rsuMsgRepeatPriority = RSU_MSG_REPEAT_PRIORITY_DEFAULT;

      /* Update file on disk. */
      if(RSEMIB_OK != commit_msgRepeat_to_disk(&ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1])){
          DEBUGMSGTL((MY_NAME, "commit_msgRepeat_to_disk: FAIL: index=%d dir=[%s].\n", index, ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].filePathName));
          ret = RSEMIB_BAD_DATA;
      }
      ntcip1218_unlock();
      return ret;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t undo_rsuMsgRepeatPriority(int32_t index)
{
  if((index < RSU_SAR_MESSAGE_MIN) || (RSU_SAR_MESSAGE_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].rsuMsgRepeatPriority = 
          prior_ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].rsuMsgRepeatPriority;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}

/* 5.4.2.11 */
int32_t get_rsuMsgRepeatOptions(int32_t index, uint8_t * data_out)
{
  if((index < RSU_SAR_MESSAGE_MIN) || (RSU_SAR_MESSAGE_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      *data_out = ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].rsuMsgRepeatOptions;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
  return RSU_MSG_REPEAT_OPTIONS_LENGTH;
}
int32_t preserve_rsuMsgRepeatOptions(int32_t index)
{
  if((index < RSU_SAR_MESSAGE_MIN) || (RSU_SAR_MESSAGE_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      prior_ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].rsuMsgRepeatOptions = 
          ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].rsuMsgRepeatOptions;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t action_rsuMsgRepeatOptions(int32_t index, uint8_t * data_in, int32_t length)
{
  if((index < RSU_SAR_MESSAGE_MIN) || (RSU_SAR_MESSAGE_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if((NULL == data_in) || (RSU_MSG_REPEAT_OPTIONS_LENGTH != length)){
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){  
      tmp_ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].rsuMsgRepeatOptions = *data_in;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t commit_rsuMsgRepeatOptions(int32_t index)
{
  int32_t ret = RSEMIB_OK;
  if((index < RSU_SAR_MESSAGE_MIN) || (RSU_SAR_MESSAGE_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].rsuMsgRepeatOptions = 
          tmp_ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].rsuMsgRepeatOptions;
      tmp_ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].rsuMsgRepeatOptions = RSU_MSG_REPEAT_OPTIONS_DEFAULT;
      /* Update file on disk. */
      if(RSEMIB_OK != commit_msgRepeat_to_disk(&ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1])){
          DEBUGMSGTL((MY_NAME, "commit_msgRepeat_to_disk: FAIL: index=%d dir=[%s].\n", index, ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].filePathName));
          ret = RSEMIB_BAD_DATA;
      }
      ntcip1218_unlock();
      return ret;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t undo_rsuMsgRepeatOptions(int32_t index)
{
  if((index < RSU_SAR_MESSAGE_MIN) || (RSU_SAR_MESSAGE_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].rsuMsgRepeatOptions = 
          prior_ntcip_1218_mib.rsuMsgRepeatStatusTable[index-1].rsuMsgRepeatOptions;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}

/* 5.4.3 */
int32_t get_rsuMsgRepeatDeleteAll(void)
{
  int32_t data = 0;
  if(RSEMIB_OK == ntcip1218_lock()){
      data = ntcip_1218_mib.rsuMsgRepeatDeleteAll;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  } 
  if((0 <= data) && (data <= 1)){
      return data; 
  } else {
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } 
}
int32_t preserve_rsuMsgRepeatDeleteAll(void)
{
  if(RSEMIB_OK == ntcip1218_lock()){
      prior_ntcip_1218_mib.rsuMsgRepeatDeleteAll = ntcip_1218_mib.rsuMsgRepeatDeleteAll;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t action_rsuMsgRepeatDeleteAll(int32_t data)
{
  if((data < 0) || (1 < data)){ 
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  }
  if(RSEMIB_OK == ntcip1218_lock()){  
      tmp_ntcip_1218_mib.rsuMsgRepeatDeleteAll = data;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t commit_rsuMsgRepeatDeleteAll(void)
{
  int32_t ret = RSEMIB_OK;
  char_t  command_buffer[100 + strlen(AMH_ACTIVE_DIR)]; /* If you need more than a 100 chars for your command then add them here. */
  int32_t i=0;

  if(RSEMIB_OK == (ret = ntcip1218_lock())){
      /* Toggle command back to zero success or fail. */
      prior_ntcip_1218_mib.rsuMsgRepeatDeleteAll = 0;
      tmp_ntcip_1218_mib.rsuMsgRepeatDeleteAll   = 0;
      ntcip_1218_mib.rsuMsgRepeatDeleteAll       = 0; 
      /* Remove files from disk. Beyond this point there is no UNDO. AMH will update back end thread. */
      memset(command_buffer,'\0',sizeof(command_buffer));
      snprintf(command_buffer, sizeof(command_buffer), "rm -f %s/*.dat", AMH_ACTIVE_DIR);
      DEBUGMSGTL((MY_NAME, "rsuMsgRepeatDeleteAll: DESTROY: cmd=[%s].\n", command_buffer));
      if(0 != system(command_buffer)){
          DEBUGMSGTL((MY_NAME, "rsuMsgRepeatDeleteAll: DESTROY FAILED: cmd=[%s].\n", command_buffer));
          ret = RSEMIB_BAD_DATA;
      }
      for(i=0;i<RSU_SAR_MESSAGE_MAX;i++) {
          /* Remove from MIB. Only call proper routine to clear. */
          set_default_row_rsuMsgRepeat(&ntcip_1218_mib.rsuMsgRepeatStatusTable[i],i);
          set_default_row_rsuMsgRepeat(&prior_ntcip_1218_mib.rsuMsgRepeatStatusTable[i],i);
          set_default_row_rsuMsgRepeat(&tmp_ntcip_1218_mib.rsuMsgRepeatStatusTable[i],i);
      }
      /* Rebuild helper table.*/
      if (RSEMIB_OK != rebuild_rsuMsgRepeat(&ntcip_1218_mib.rsuMsgRepeatStatusTable[0])){
          DEBUGMSGTL((MY_NAME, "rsuMsgRepeatDeleteAll: rebuild_rsuMsgRepeat() Failed"));
          ret = RSEMIB_BAD_DATA;
      }
      ntcip1218_unlock();
  }
  return ret; 
}
int32_t undo_rsuMsgRepeatDeleteAll(void)
{
  if(RSEMIB_OK == ntcip1218_lock()){
      /* Depending on where this fails there is no undoing the deletion of files on the disk. */
      prior_ntcip_1218_mib.rsuMsgRepeatDeleteAll = 0;
      tmp_ntcip_1218_mib.rsuMsgRepeatDeleteAll   = 0;
      ntcip_1218_mib.rsuMsgRepeatDeleteAll       = 0; 
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}
/****************************************************************************** 
 * 5.5 Immediate Forward Messages:{ rsu 4 }: rsuIFMStatusTable
 ******************************************************************************/ 

/* 5.5.1 */
int32_t get_maxRsuIFMs(void)
{
  int32_t data = RSU_IFM_MESSAGE_MAX;

  if(RSEMIB_OK == ntcip1218_lock()){
      data = ntcip_1218_mib.maxRsuIFMs;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  } 
  if((RSU_IFM_MESSAGE_MIN <= data) && (data <= RSU_IFM_MESSAGE_MAX)){
      return data; 
  } else {
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } 
}

/* 5.5.2.1 */
int32_t  get_rsuIFMIndex(int32_t index, int32_t * data_out)
{
  if((index < RSU_IFM_MESSAGE_MIN) || (RSU_IFM_MESSAGE_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      *data_out = ntcip_1218_mib.rsuIFMStatusTable[index-1].rsuIFMIndex;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
  if((*data_out < RSU_IFM_MESSAGE_MIN) || (RSU_IFM_MESSAGE_MAX < *data_out)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } 
  return RSEMIB_OK;
}

/* 5.5.2.2 */
int32_t get_rsuIFMPsid(int32_t index, uint8_t * data_out)
{
  int32_t length = 0;
  if((index < RSU_IFM_MESSAGE_MIN) || (RSU_IFM_MESSAGE_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      length = ntcip_1218_mib.rsuIFMStatusTable[index-1].rsuIFMPsid_length;
      if(   (RSU_RADIO_PSID_MIN_SIZE <= length) 
         && (length <= RSU_RADIO_PSID_SIZE)){
          memcpy(data_out, ntcip_1218_mib.rsuIFMStatusTable[index-1].rsuIFMPsid, length);
          ntcip1218_unlock();
          return length;
      } else {
          ntcip1218_unlock();
          return RSEMIB_BAD_DATA;
      }
      ntcip1218_unlock();
      return RSU_RADIO_PSID_SIZE;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t preserve_rsuIFMPsid(int32_t index)
{
  if((index < RSU_IFM_MESSAGE_MIN) || (RSU_IFM_MESSAGE_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      memcpy( prior_ntcip_1218_mib.rsuIFMStatusTable[index-1].rsuIFMPsid
            , ntcip_1218_mib.rsuIFMStatusTable[index-1].rsuIFMPsid, RSU_RADIO_PSID_SIZE);
      prior_ntcip_1218_mib.rsuIFMStatusTable[index-1].rsuIFMPsid_length =
            ntcip_1218_mib.rsuIFMStatusTable[index-1].rsuIFMPsid_length; 
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t action_rsuIFMPsid(int32_t index, uint8_t * data_in, int32_t length)
{
  uint32_t psid = 0x0;
  if((index < RSU_IFM_MESSAGE_MIN) || (RSU_IFM_MESSAGE_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  } 
  if(NULL == data_in){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(( length < RSU_RADIO_PSID_MIN_SIZE) || (RSU_RADIO_PSID_SIZE < length)){
      return RSEMIB_BAD_INPUT;  
  }
  /* Reject known bogus PSID's. */
  if(length >= 1) {
      psid  = (uint32_t)(data_in[0]);
  }
  if(length >= 2) {
      psid  = psid << 8;
      psid  |= (uint32_t)(data_in[1]);
  }
  if(length >= 3) {
      psid  = psid << 8;
      psid  |= (uint32_t)(data_in[2]);
  }
  if(length == 4) {
      psid  = psid << 8;
      psid  |= (uint32_t)(data_in[3]);
  }
  if(psid <= (uint32_t) PSID_1BYTE_MAX_VALUE){
  } else if(psid >= (uint32_t)PSID_2BYTE_MIN_VALUE && psid <= (uint32_t)PSID_2BYTE_MAX_VALUE){
  } else if(psid >= (uint32_t)PSID_3BYTE_MIN_VALUE && psid <= (uint32_t)PSID_3BYTE_MAX_VALUE){
  } else if(psid >= (uint32_t)PSID_4BYTE_MIN_VALUE && psid <= (uint32_t)PSID_4BYTE_MAX_VALUE){
  } else {
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){  
      memset(tmp_ntcip_1218_mib.rsuIFMStatusTable[index-1].rsuIFMPsid,0x0,RSU_RADIO_PSID_SIZE);
      if(1 == length) {
          tmp_ntcip_1218_mib.rsuIFMStatusTable[index-1].rsuIFMPsid[3] = data_in[0];
      }
      if(2 == length){
          tmp_ntcip_1218_mib.rsuIFMStatusTable[index-1].rsuIFMPsid[3] = data_in[1];
          tmp_ntcip_1218_mib.rsuIFMStatusTable[index-1].rsuIFMPsid[2] = data_in[0];
      }
      if(3 == length) {
          tmp_ntcip_1218_mib.rsuIFMStatusTable[index-1].rsuIFMPsid[3] = data_in[2];
          tmp_ntcip_1218_mib.rsuIFMStatusTable[index-1].rsuIFMPsid[2] = data_in[1];
          tmp_ntcip_1218_mib.rsuIFMStatusTable[index-1].rsuIFMPsid[1] = data_in[0];
      }
      if(4 == length) {
          tmp_ntcip_1218_mib.rsuIFMStatusTable[index-1].rsuIFMPsid[3] = data_in[3];
          tmp_ntcip_1218_mib.rsuIFMStatusTable[index-1].rsuIFMPsid[2] = data_in[2];
          tmp_ntcip_1218_mib.rsuIFMStatusTable[index-1].rsuIFMPsid[1] = data_in[1];
          tmp_ntcip_1218_mib.rsuIFMStatusTable[index-1].rsuIFMPsid[0] = data_in[0];
      }
      tmp_ntcip_1218_mib.rsuIFMStatusTable[index-1].rsuIFMPsid_length = length;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t commit_rsuIFMPsid(int32_t index)
{
  int32_t ret = RSEMIB_OK;
  if((index < RSU_IFM_MESSAGE_MIN) || (RSU_IFM_MESSAGE_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  } 
  if(RSEMIB_OK == ntcip1218_lock()){
      memcpy( ntcip_1218_mib.rsuIFMStatusTable[index-1].rsuIFMPsid
            , tmp_ntcip_1218_mib.rsuIFMStatusTable[index-1].rsuIFMPsid, RSU_RADIO_PSID_SIZE);
      ntcip_1218_mib.rsuIFMStatusTable[index-1].rsuIFMPsid_length = 
          tmp_ntcip_1218_mib.rsuIFMStatusTable[index-1].rsuIFMPsid_length;
      memset(tmp_ntcip_1218_mib.rsuIFMStatusTable[index-1].rsuIFMPsid, 0x0, RSU_RADIO_PSID_SIZE);
      tmp_ntcip_1218_mib.rsuIFMStatusTable[index-1].rsuIFMPsid_length = RSU_RADIO_PSID_MIN_SIZE;
      if(RSEMIB_OK != commit_IFM_to_disk(&ntcip_1218_mib.rsuIFMStatusTable[index-1])){
          DEBUGMSGTL((MY_NAME, "build_IFM_message: FAIL: index=%d dir=[%s].\n", index, ntcip_1218_mib.rsuIFMStatusTable[index-1].filePathName));
          ret = RSEMIB_BAD_DATA;
      }
      ntcip1218_unlock();
      return ret;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t undo_rsuIFMPsid(int32_t index)
{
  if((index < RSU_IFM_MESSAGE_MIN) || (RSU_IFM_MESSAGE_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      memcpy( ntcip_1218_mib.rsuIFMStatusTable[index-1].rsuIFMPsid
            , prior_ntcip_1218_mib.rsuIFMStatusTable[index-1].rsuIFMPsid, RSU_RADIO_PSID_SIZE);
      ntcip_1218_mib.rsuIFMStatusTable[index-1].rsuIFMPsid_length = 
          prior_ntcip_1218_mib.rsuIFMStatusTable[index-1].rsuIFMPsid_length;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}

/* 5.5.2.3 */
int32_t get_rsuIFMTxChannel(int32_t index, int32_t * data_out)
{
  if((index < RSU_IFM_MESSAGE_MIN) || (RSU_IFM_MESSAGE_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      *data_out = ntcip_1218_mib.rsuIFMStatusTable[index-1].rsuIFMTxChannel;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
  if((*data_out < RSU_RADIO_CHANNEL_MIN) || (RSU_RADIO_CHANNEL_MAX < *data_out)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } 
  return RSEMIB_OK;

}
int32_t preserve_rsuIFMTxChannel(int32_t index)
{
  if((index < RSU_IFM_MESSAGE_MIN) || (RSU_IFM_MESSAGE_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      prior_ntcip_1218_mib.rsuIFMStatusTable[index-1].rsuIFMTxChannel = 
          ntcip_1218_mib.rsuIFMStatusTable[index-1].rsuIFMTxChannel;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t action_rsuIFMTxChannel(int32_t index, int32_t data_in)
{
  int32_t ret = RSEMIB_OK;
  if((index < RSU_IFM_MESSAGE_MIN) || (RSU_IFM_MESSAGE_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if((data_in < RSU_RADIO_CHANNEL_MIN) || (RSU_RADIO_CHANNEL_MAX < data_in)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  }
  if(RSEMIB_OK == ntcip1218_lock()){  

#if defined(PLATFORM_HD_RSU_5940)
  if(data_in != ntcip_1218_mib.rsuRadioTable[RSU_CV2X_RADIO - 1].rsuRadioCh1) /* CV2X. */
      ret = RSEMIB_BAD_DATA;
#else
      if(mib_on == ntcip_1218_mib.rsuRadioTable[0].rsuRadioEnable) {
          if(data_in != ntcip_1218_mib.rsuRadioTable[0].rsuRadioCh1) /* DSRC. */
              ret = RSEMIB_BAD_DATA;
      } else {
          if(data_in != ntcip_1218_mib.rsuRadioTable[1].rsuRadioCh1) /* CV2X. */
              ret = RSEMIB_BAD_DATA;
      }
#endif
      if(RSEMIB_OK == ret){
          tmp_ntcip_1218_mib.rsuIFMStatusTable[index-1].rsuIFMTxChannel = data_in;
      }
      ntcip1218_unlock();
      return ret;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t commit_rsuIFMTxChannel(int32_t index)
{
  int32_t ret = RSEMIB_OK;
  if((index < RSU_IFM_MESSAGE_MIN) || (RSU_IFM_MESSAGE_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuIFMStatusTable[index-1].rsuIFMTxChannel = tmp_ntcip_1218_mib.rsuIFMStatusTable[index-1].rsuIFMTxChannel;
      tmp_ntcip_1218_mib.rsuIFMStatusTable[index-1].rsuIFMTxChannel = RSU_RADIO_CHANNEL_MIN;
      if(RSEMIB_OK != commit_IFM_to_disk(&ntcip_1218_mib.rsuIFMStatusTable[index-1])){
          DEBUGMSGTL((MY_NAME, "build_IFM_message: FAIL: index=%d dir=[%s].\n", index, ntcip_1218_mib.rsuIFMStatusTable[index-1].filePathName));
          ret = RSEMIB_BAD_DATA;
      }
      ntcip1218_unlock();
      return ret;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t undo_rsuIFMTxChannel(int32_t index)
{
  if((index < RSU_IFM_MESSAGE_MIN) || (RSU_IFM_MESSAGE_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuIFMStatusTable[index-1].rsuIFMTxChannel = prior_ntcip_1218_mib.rsuIFMStatusTable[index-1].rsuIFMTxChannel;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}

/* 5.5.2.4 */
int32_t get_rsuIFMEnable(int32_t index, int32_t * data_out)
{
  if((index < RSU_IFM_MESSAGE_MIN) || (RSU_IFM_MESSAGE_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      *data_out = (int32_t)ntcip_1218_mib.rsuIFMStatusTable[index-1].rsuIFMEnable;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
  if((*data_out < mib_off) || (mib_on < *data_out)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } 
  return RSEMIB_OK;

}
int32_t preserve_rsuIFMEnable(int32_t index)
{
  if((index < RSU_IFM_MESSAGE_MIN) || (RSU_IFM_MESSAGE_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      prior_ntcip_1218_mib.rsuIFMStatusTable[index-1].rsuIFMEnable = ntcip_1218_mib.rsuIFMStatusTable[index-1].rsuIFMEnable;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t action_rsuIFMEnable(int32_t index, int32_t data_in)
{
  if((index < RSU_IFM_MESSAGE_MIN) || (RSU_IFM_MESSAGE_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if((data_in < mib_off) || (mib_on < data_in)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  }
  if(RSEMIB_OK == ntcip1218_lock()){  
      tmp_ntcip_1218_mib.rsuIFMStatusTable[index-1].rsuIFMEnable = (onOff_e)data_in;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t commit_rsuIFMEnable(int32_t index)
{
  int32_t ret = RSEMIB_OK;
  if((index < RSU_IFM_MESSAGE_MIN) || (RSU_IFM_MESSAGE_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuIFMStatusTable[index-1].rsuIFMEnable = tmp_ntcip_1218_mib.rsuIFMStatusTable[index-1].rsuIFMEnable;
      tmp_ntcip_1218_mib.rsuIFMStatusTable[index-1].rsuIFMEnable = mib_off;
      if(RSEMIB_OK != commit_IFM_to_disk(&ntcip_1218_mib.rsuIFMStatusTable[index-1])){
          DEBUGMSGTL((MY_NAME, "build_IFM_message: FAIL: index=%d dir=[%s].\n", index, ntcip_1218_mib.rsuIFMStatusTable[index-1].filePathName));
          ret = RSEMIB_BAD_DATA;
      }
      ntcip1218_unlock();
      return ret;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t undo_rsuIFMEnable(int32_t index)
{
  if((index < RSU_IFM_MESSAGE_MIN) || (RSU_IFM_MESSAGE_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuIFMStatusTable[index-1].rsuIFMEnable = prior_ntcip_1218_mib.rsuIFMStatusTable[index-1].rsuIFMEnable;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}

/* 5.5.2.5 */
int32_t get_rsuIFMStatus(int32_t index, int32_t * data_out)
{
  if((index < RSU_IFM_MESSAGE_MIN) || (RSU_IFM_MESSAGE_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      *data_out = ntcip_1218_mib.rsuIFMStatusTable[index-1].rsuIFMStatus;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  } 
  if((ROW_STATUS_VALID_MIN <= *data_out ) && (*data_out <= ROW_STATUS_VALID_MAX)){
      return RSEMIB_OK; 
  } else {
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } 
}
int32_t preserve_rsuIFMStatus(int32_t index)
{
  if((index < RSU_IFM_MESSAGE_MIN) || (RSU_IFM_MESSAGE_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      set_default_row_rsuIFM(&tmp_ntcip_1218_mib.rsuIFMStatusTable[index-1],index-1);
      prior_ntcip_1218_mib.rsuIFMStatusTable[index-1].rsuIFMStatus = ntcip_1218_mib.rsuIFMStatusTable[index-1].rsuIFMStatus;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t action_rsuIFMStatus(int32_t index, int32_t data_in)
{
  if((index < RSU_IFM_MESSAGE_MIN) || (RSU_IFM_MESSAGE_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if((data_in < ROW_STATUS_VALID_MIN) || (ROW_STATUS_VALID_MAX < data_in)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      tmp_ntcip_1218_mib.rsuIFMStatusTable[index-1].rsuIFMStatus = data_in;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
/****************************************************************************** 
  For reference, see "SNMPv2-TC.txt" keyword "RowStatus". See snmp.h for SNMP row & error codes.

  SNMPGET on Status returns these SNMP ROW values depending on state of the row.

  Status           | Access | Meaning
  ---------------------------------------------------------------------------------
  active(1)        | R      | Message is allowed to be sent by radio if all other columns valid.
  notInService(2)  | R      | No message being sent to radio regardless of state of columns.
  notReady(3)      | N/A    | N/A.

  SNMPSET on Status returns these SNMP ERR values depending on state of the row.

  Status          | Access | active(1)                  | notInService(2)            | notReady(3) | NonExistent(0)
  --------------------------------------------------------------------------------------------------------------------
  active(1)       | W      | SNMP_ERR_NOERROR           | SNMP_ERR_NOERROR           | n/a         | SNMP_ERR_INCONSISTENTVALUE
  notInService(2) | W      | SNMP_ERR_NOERROR           | SNMP_ERR_NOERROR           | n/a         | SNMP_ERR_INCONSISTENTVALUE
  createAndGo(4)  | WO     | SNMP_ERR_INCONSISTENTVALUE | SNMP_ERR_INCONSISTENTVALUE | n/a         | SNMP_ERR_NOERROR
  createAndWait(5)| WO     | SNMP_ERR_INCONSISTENTVALUE | SNMP_ERR_INCONSISTENTVALUE | n/a         | SNMP_ERR_NOERROR
  destroy(6)      | WO     | SNMP_ERR_NOERROR           | SNMP_ERR_NOERROR           | n/a         | SNMP_ERR_NOERROR
  
    (1) Allow message to be sent. 
        May not be sending to radio because payload == zero, enable == OFF, no GNSS, or no certs. 
        Check syslog AMH output for errors.
    (2) Stop sending to radio. 
    (4) Create Row. Status goto active(1). Payload is len zero. 
        All other columns default to valid values for radio transmission.
    (5) Create Row. Status goto notInService(2). Payload is len zero. 
        All other columns default to valid values for radio transmission.
    (6) Destroy Row. Status goto NonExistent(0).

  Keep in mind handler in rsuIFM.c is also evaluating if the table and row exists. 
  We don't have to recheck for nonExistent(0) rows or if table exists here.
******************************************************************************/
int32_t commit_rsuIFMStatus(int32_t index)
{
  int32_t ret = RSEMIB_OK;
  char_t  command_buffer[100 + RSU_MSG_FILENAME_LENGTH_MAX]; /* If you need more than a 100 chars for your command then add them here. */

  if((index < RSU_IFM_MESSAGE_MIN) || (RSU_IFM_MESSAGE_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){

      DEBUGMSGTL((MY_NAME_EXTRA, "commit_rsuIFMStatus: index=%d status=%d.\n", index,tmp_ntcip_1218_mib.rsuIFMStatusTable[index-1].rsuIFMStatus));
      switch(tmp_ntcip_1218_mib.rsuIFMStatusTable[index-1].rsuIFMStatus){
          case SNMP_ROW_CREATEANDGO:
              if(SNMP_ROW_NONEXISTENT == ntcip_1218_mib.rsuIFMStatusTable[index-1].rsuIFMStatus) {
                  set_default_row_rsuIFM(&ntcip_1218_mib.rsuIFMStatusTable[index-1],index-1);
                  ntcip_1218_mib.rsuIFMStatusTable[index-1].rsuIFMStatus = SNMP_ROW_ACTIVE;
                  if(RSEMIB_OK != commit_IFM_to_disk(&ntcip_1218_mib.rsuIFMStatusTable[index-1])){
                      DEBUGMSGTL((MY_NAME, "build_IFM_message: FAIL: index=%d dir=[%s].\n", index, ntcip_1218_mib.rsuIFMStatusTable[index-1].filePathName));
                      ret = RSEMIB_BAD_DATA;
                  } else {
                      /* Rebuild helper table before ack. */
                      if (RSEMIB_OK != (ret = rebuild_rsuIFM_live(&ntcip_1218_mib.rsuIFMStatusTable[0]))){
                          ret = RSEMIB_BAD_DATA;
                      }
                  }
              } else {
                  /* If it exists already then do nothing. Dont send error back. */
                  //ret = RSEMIB_BAD_MODE;
              }
              break;
          case SNMP_ROW_CREATEANDWAIT:
              if(SNMP_ROW_NONEXISTENT == ntcip_1218_mib.rsuIFMStatusTable[index-1].rsuIFMStatus) {
                  set_default_row_rsuIFM(&ntcip_1218_mib.rsuIFMStatusTable[index-1],index-1);
                  ntcip_1218_mib.rsuIFMStatusTable[index-1].rsuIFMStatus = SNMP_ROW_NOTINSERVICE;
                  if(RSEMIB_OK != commit_IFM_to_disk(&ntcip_1218_mib.rsuIFMStatusTable[index-1])){
                      DEBUGMSGTL((MY_NAME, "build_IFM_message: FAIL: index=%d dir=[%s].\n", index, ntcip_1218_mib.rsuIFMStatusTable[index-1].filePathName));
                      ret = RSEMIB_BAD_DATA;
                  } else {
                      /* Rebuild helper table before ack. */
                      if (RSEMIB_OK != (ret = rebuild_rsuIFM_live(&ntcip_1218_mib.rsuIFMStatusTable[0]))){
                          ret = RSEMIB_BAD_DATA;
                      }
                  }
              } else {
                  /* If it exists already then do nothing. Dont send error back. */
                  //ret = RSEMIB_BAD_MODE;
              }
              break;
          case SNMP_ROW_DESTROY: /* User deletes row from table. */
              if(SNMP_ROW_NONEXISTENT == ntcip_1218_mib.rsuIFMStatusTable[index-1].rsuIFMStatus){
                  ret = RSEMIB_OK; /* If you try to DESTROY non-exixtent row that's an easy one. */
              } else {
                  /* rm from RSU. */
                  memset(command_buffer,'\0',sizeof(command_buffer));
                  snprintf(command_buffer, sizeof(command_buffer), "rm -f %s", ntcip_1218_mib.rsuIFMStatusTable[index-1].filePathName);
                  DEBUGMSGTL((MY_NAME, "DESTROY: index=%d cmd=[%s].\n", index, command_buffer));
                  if(0 != system(command_buffer)){
                      ret = RSEMIB_BAD_DATA;
                  }
                  /* Careful to preserve index within table rows. Only call proper routine to clear. */
                  set_default_row_rsuIFM(&ntcip_1218_mib.rsuIFMStatusTable[index-1], index-1);
                  set_default_row_rsuIFM(&tmp_ntcip_1218_mib.rsuIFMStatusTable[index-1], index-1);
                  set_default_row_rsuIFM(&prior_ntcip_1218_mib.rsuIFMStatusTable[index-1], index-1);
                  /* Rebuild helper table before ack. */
                  if (RSEMIB_OK != (ret = rebuild_rsuIFM_live(&ntcip_1218_mib.rsuIFMStatusTable[0]))){
                      ret = RSEMIB_BAD_DATA;
                  }
              }
              break;
          case SNMP_ROW_ACTIVE:
              ntcip_1218_mib.rsuIFMStatusTable[index-1].rsuIFMStatus = SNMP_ROW_ACTIVE;
              break;
          case SNMP_ROW_NOTINSERVICE:
              ntcip_1218_mib.rsuIFMStatusTable[index-1].rsuIFMStatus = SNMP_ROW_NOTINSERVICE;
              break;
          case SNMP_ROW_NONEXISTENT:
          case SNMP_ROW_NOTREADY:
          default:
              ret = RSEMIB_BAD_MODE;
              break;
      }
      /* Clear tmp. */
      tmp_ntcip_1218_mib.rsuIFMStatusTable[index-1].rsuIFMStatus = ROW_STATUS_VALID_DEFAULT;
      ntcip1218_unlock();
      return ret;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
/* TODO: If client calls UNDO without starting the transaction with R1 then we are exposed i think. 
 *     : If we always clear set to non-existent then maybe ok?
 *     : But we need something else than status because they could be changing status alas.
 *     : Maybe need a R1 on each column? dont see it dont undo.
 */
int32_t undo_rsuIFMStatus(int32_t index)
{
  int32_t ret = RSEMIB_OK;
  if((index < RSU_IFM_MESSAGE_MIN) || (RSU_IFM_MESSAGE_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      /* A row in rsuIFM can only exist in two row states active and notInService. Ignore otherwise.*/
      switch(prior_ntcip_1218_mib.rsuIFMStatusTable[index-1].rsuIFMStatus) {
          case SNMP_ROW_ACTIVE:
          case SNMP_ROW_NOTINSERVICE:
              ntcip_1218_mib.rsuIFMStatusTable[index-1].rsuIFMStatus = prior_ntcip_1218_mib.rsuIFMStatusTable[index-1].rsuIFMStatus;
              break;
          case SNMP_ROW_NONEXISTENT:
          case SNMP_ROW_NOTREADY:
          case SNMP_ROW_CREATEANDGO:
          case SNMP_ROW_CREATEANDWAIT:
          case SNMP_ROW_DESTROY:
          default:
              ret = RSEMIB_BAD_MODE;
              break;
      }
      prior_ntcip_1218_mib.rsuIFMStatusTable[index-1].rsuIFMStatus = SNMP_ROW_NONEXISTENT;
      ntcip1218_unlock();
      return ret;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}

/* 5.5.2.6 */
int32_t get_rsuIFMPriority(int32_t index, int32_t * data_out)
{
  if((index < RSU_IFM_MESSAGE_MIN) || (RSU_IFM_MESSAGE_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      *data_out = ntcip_1218_mib.rsuIFMStatusTable[index-1].rsuIFMPriority;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
  if((*data_out < RSU_IFM_PRIORITY_MIN) || (RSU_IFM_PRIORITY_MAX < *data_out)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } 
  return RSEMIB_OK;
}
int32_t preserve_rsuIFMPriority(int32_t index)
{
  if((index < RSU_IFM_MESSAGE_MIN) || (RSU_IFM_MESSAGE_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
        prior_ntcip_1218_mib.rsuIFMStatusTable[index-1].rsuIFMPriority = ntcip_1218_mib.rsuIFMStatusTable[index-1].rsuIFMPriority;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t action_rsuIFMPriority(int32_t index, int32_t data_in)
{
  if((index < RSU_IFM_MESSAGE_MIN) || (RSU_IFM_MESSAGE_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if((data_in < RSU_IFM_PRIORITY_MIN) || (RSU_IFM_PRIORITY_MAX < data_in)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  }
  if(RSEMIB_OK == ntcip1218_lock()){  
      tmp_ntcip_1218_mib.rsuIFMStatusTable[index-1].rsuIFMPriority = data_in;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t commit_rsuIFMPriority(int32_t index)
{
  int32_t ret = RSEMIB_OK;
  if((index < RSU_IFM_MESSAGE_MIN) || (RSU_IFM_MESSAGE_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuIFMStatusTable[index-1].rsuIFMPriority = tmp_ntcip_1218_mib.rsuIFMStatusTable[index-1].rsuIFMPriority;
      tmp_ntcip_1218_mib.rsuIFMStatusTable[index-1].rsuIFMPriority = RSU_IFM_PRIORITY_DEFAULT;
      if(RSEMIB_OK != commit_IFM_to_disk(&ntcip_1218_mib.rsuIFMStatusTable[index-1])){
          DEBUGMSGTL((MY_NAME, "build_IFM_message: FAIL: index=%d dir=[%s].\n", index, ntcip_1218_mib.rsuIFMStatusTable[index-1].filePathName));
          ret = RSEMIB_BAD_DATA;
      }
      ntcip1218_unlock();
      return ret;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t undo_rsuIFMPriority(int32_t index)
{
  if((index < RSU_IFM_MESSAGE_MIN) || (RSU_IFM_MESSAGE_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuIFMStatusTable[index-1].rsuIFMPriority = prior_ntcip_1218_mib.rsuIFMStatusTable[index-1].rsuIFMPriority;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}

/* 5.5.2.8 */
int32_t get_rsuIFMOptions(int32_t index, uint8_t * data_out)
{
  if((index < RSU_IFM_MESSAGE_MIN) || (RSU_IFM_MESSAGE_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      *data_out = ntcip_1218_mib.rsuIFMStatusTable[index-1].rsuIFMOptions;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;;
  }
  return RSU_IFM_OPTIONS_LENGTH;

}
int32_t preserve_rsuIFMOptions(int32_t index)
{
  if((index < RSU_IFM_MESSAGE_MIN) || (RSU_IFM_MESSAGE_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      prior_ntcip_1218_mib.rsuIFMStatusTable[index-1].rsuIFMOptions = ntcip_1218_mib.rsuIFMStatusTable[index-1].rsuIFMOptions;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t action_rsuIFMOptions(int32_t index, uint8_t * data_in, int32_t length)
{
  if((index < RSU_IFM_MESSAGE_MIN) || (RSU_IFM_MESSAGE_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if((NULL == data_in) || (RSU_IFM_OPTIONS_LENGTH != length)){
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){  
      tmp_ntcip_1218_mib.rsuIFMStatusTable[index-1].rsuIFMOptions = *data_in;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t commit_rsuIFMOptions(int32_t index)
{
  int32_t ret = RSEMIB_OK;
  if((index < RSU_IFM_MESSAGE_MIN) || (RSU_IFM_MESSAGE_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuIFMStatusTable[index-1].rsuIFMOptions = tmp_ntcip_1218_mib.rsuIFMStatusTable[index-1].rsuIFMOptions;
      tmp_ntcip_1218_mib.rsuIFMStatusTable[index-1].rsuIFMOptions = RSU_IFM_OPTIONS_DEFAULT;
      if(RSEMIB_OK != commit_IFM_to_disk(&ntcip_1218_mib.rsuIFMStatusTable[index-1])){
          DEBUGMSGTL((MY_NAME, "build_IFM_message: FAIL: index=%d dir=[%s].\n", index, ntcip_1218_mib.rsuIFMStatusTable[index-1].filePathName));
          ret = RSEMIB_BAD_DATA;
      }
      ntcip1218_unlock();
      return ret;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t undo_rsuIFMOptions(int32_t index)
{
  if((index < RSU_IFM_MESSAGE_MIN) || (RSU_IFM_MESSAGE_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuIFMStatusTable[index-1].rsuIFMOptions = prior_ntcip_1218_mib.rsuIFMStatusTable[index-1].rsuIFMOptions;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}

/* 5.5.2.8 */
int32_t get_rsuIFMPayload(int32_t index, uint8_t * data_out)
{
  int32_t length = 0;
  if((index < RSU_IFM_MESSAGE_MIN) || (RSU_IFM_MESSAGE_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      length = ntcip_1218_mib.rsuIFMStatusTable[index-1].rsuIFMPayload_length;
      if(   (RSU_MSG_PAYLOAD_MIN <= length) 
         && (length <= RSU_MSG_PAYLOAD_MAX)){
          memcpy(data_out, ntcip_1218_mib.rsuIFMStatusTable[index-1].rsuIFMPayload, length); 
          ntcip1218_unlock();
          return length;
      } else {
          ntcip1218_unlock();
          return RSEMIB_BAD_DATA;
      }
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t preserve_rsuIFMPayload(int32_t index)
{
  if((index < RSU_IFM_MESSAGE_MIN) || (RSU_IFM_MESSAGE_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      memcpy( prior_ntcip_1218_mib.rsuIFMStatusTable[index-1].rsuIFMPayload
            , ntcip_1218_mib.rsuIFMStatusTable[index-1].rsuIFMPayload, RSU_MSG_PAYLOAD_MAX);
      prior_ntcip_1218_mib.rsuIFMStatusTable[index-1].rsuIFMPayload_length =
          ntcip_1218_mib.rsuIFMStatusTable[index-1].rsuIFMPayload_length;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t action_rsuIFMPayload(int32_t index, uint8_t * data_in, int32_t length)
{
  if((index < RSU_IFM_MESSAGE_MIN) || (RSU_IFM_MESSAGE_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if((NULL == data_in) && (0 != length)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(( length < RSU_MSG_PAYLOAD_MIN) || (RSU_MSG_PAYLOAD_MAX < length)){
      return RSEMIB_BAD_INPUT;  
  }
  if(RSEMIB_OK == ntcip1218_lock()){  
      memset(tmp_ntcip_1218_mib.rsuIFMStatusTable[index-1].rsuIFMPayload, 0x0, RSU_MSG_PAYLOAD_MAX);
      if(NULL != data_in){
          memcpy(tmp_ntcip_1218_mib.rsuIFMStatusTable[index-1].rsuIFMPayload, data_in, length);
      }
      tmp_ntcip_1218_mib.rsuIFMStatusTable[index-1].rsuIFMPayload_length = length;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t commit_rsuIFMPayload(int32_t index)
{
  int32_t ret = RSEMIB_OK;
  if((index < RSU_IFM_MESSAGE_MIN) || (RSU_IFM_MESSAGE_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      memcpy( ntcip_1218_mib.rsuIFMStatusTable[index-1].rsuIFMPayload
            , tmp_ntcip_1218_mib.rsuIFMStatusTable[index-1].rsuIFMPayload, RSU_MSG_PAYLOAD_MAX);
      ntcip_1218_mib.rsuIFMStatusTable[index-1].rsuIFMPayload_length =
          tmp_ntcip_1218_mib.rsuIFMStatusTable[index-1].rsuIFMPayload_length;
      /* Maintain last sent message. Do not write payload to disk. Need not be saved across reset.*/
      memset(tmp_ntcip_1218_mib.rsuIFMStatusTable[index-1].rsuIFMPayload, 0x0, RSU_MSG_PAYLOAD_MAX);
      tmp_ntcip_1218_mib.rsuIFMStatusTable[index-1].rsuIFMPayload_length = RSU_IFM_MESSAGE_MIN;
      /* This is the only place we trigger the row to send message to radio. */
      if(RSEMIB_OK == validate_and_send_rsuIFM(&ntcip_1218_mib.rsuIFMStatusTable[index-1])){
          if(   (SNMP_ROW_ACTIVE == ntcip_1218_mib.rsuIFMStatusTable[index-1].rsuIFMStatus)
             && (mib_on == ntcip_1218_mib.rsuIFMStatusTable[index-1].rsuIFMEnable)){
              if(RSEMIB_OK != ifm_enqueue((const char_t *)&ntcip_1218_mib.rsuIFMStatusTable[index-1])){ /* Stuff into queue.*/
                  DEBUGMSGTL((MY_NAME_EXTRA, "ifm_enqueue failed: index=%d.\n", index));
                  ret = RSEMIB_BAD_DATA;
              }
          } /* else user can update but not send. */
      } else {
          DEBUGMSGTL((MY_NAME_EXTRA, "validate_and_send_rsuIFM failed: index=%d.\n", index));
          ret = RSEMIB_BAD_DATA;
      }
      ntcip1218_unlock();
      return ret;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t undo_rsuIFMPayload(int32_t index)
{
  if((index < RSU_IFM_MESSAGE_MIN) || (RSU_IFM_MESSAGE_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      memcpy( ntcip_1218_mib.rsuIFMStatusTable[index-1].rsuIFMPayload
            , prior_ntcip_1218_mib.rsuIFMStatusTable[index-1].rsuIFMPayload, RSU_MSG_PAYLOAD_MAX);
      ntcip_1218_mib.rsuIFMStatusTable[index-1].rsuIFMPayload_length =
          prior_ntcip_1218_mib.rsuIFMStatusTable[index-1].rsuIFMPayload_length;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}

/****************************************************************************** 
 * 5.6 (OTA)Received Messages(to fwd via socket): { rsu 5 }: rsuReceivedMsgTable
 ******************************************************************************/

/* 5.6.1 */
int32_t get_maxRsuReceivedMsgs(void)
{
  int32_t data = RSU_RX_MSG_MAX;
  if(RSEMIB_OK == ntcip1218_lock()){
      data = ntcip_1218_mib.maxRsuReceivedMsgs;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  } 
  if((RSU_RX_MSG_MIN <= data) && (data <= RSU_RX_MSG_MAX)){
      return data; 
  } else {
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } 
}

/* 5.6.2.1 */
int32_t  get_rsuReceivedMsgIndex(int32_t index, int32_t * data_out)
{
  if((index < RSU_RX_MSG_MIN) || (RSU_RX_MSG_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      *data_out = ntcip_1218_mib.rsuReceivedMsgTable[index-1].rsuReceivedMsgIndex;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
  if((*data_out < RSU_RX_MSG_MIN) || (RSU_RX_MSG_MAX < *data_out)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } 
  return RSEMIB_OK;
}

/* 5.6.2.2 */
int32_t get_rsuReceivedMsgPsid(int32_t index, uint8_t * data_out)
{
  int32_t length = 0;
  if((index < RSU_RX_MSG_MIN) || (RSU_RX_MSG_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      length = ntcip_1218_mib.rsuReceivedMsgTable[index-1].rsuReceivedMsgPsid_length;
      memcpy(data_out, ntcip_1218_mib.rsuReceivedMsgTable[index-1].rsuReceivedMsgPsid, RSU_RADIO_PSID_SIZE);
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
  if((length < RSU_RADIO_PSID_MIN_SIZE) || (RSU_RADIO_PSID_SIZE < length)){
      return RSEMIB_BAD_INPUT;  
  }
  return length;
}
int32_t preserve_rsuReceivedMsgPsid(int32_t index)
{
  if((index < RSU_RX_MSG_MIN) || (RSU_RX_MSG_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      memcpy( prior_ntcip_1218_mib.rsuReceivedMsgTable[index-1].rsuReceivedMsgPsid
            , ntcip_1218_mib.rsuReceivedMsgTable[index-1].rsuReceivedMsgPsid, RSU_RADIO_PSID_SIZE);
      prior_ntcip_1218_mib.rsuReceivedMsgTable[index-1].rsuReceivedMsgPsid_length =
          ntcip_1218_mib.rsuReceivedMsgTable[index-1].rsuReceivedMsgPsid_length;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t action_rsuReceivedMsgPsid(int32_t index, uint8_t * data_in, int32_t length)
{
  uint32_t psid = 0x0;
  if((index < RSU_RX_MSG_MIN) || (RSU_RX_MSG_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  } 
  if(NULL == data_in){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if((length < RSU_RADIO_PSID_MIN_SIZE) || (RSU_RADIO_PSID_SIZE < length)){
      return RSEMIB_BAD_INPUT;  
  }
  /* Reject known bogus PSID's. */
  if(length >= 1) {
      psid  = (uint32_t)(data_in[0]);
  }
  if(length >= 2) {
      psid  = psid << 8;
      psid  |= (uint32_t)(data_in[1]);
  }
  if(length >= 3) {
      psid  = psid << 8;
      psid  |= (uint32_t)(data_in[2]);
  }
  if(length == 4) {
      psid  = psid << 8;
      psid  |= (uint32_t)(data_in[3]);
  }
  if(psid <= (uint32_t) PSID_1BYTE_MAX_VALUE){
  } else if(psid >= (uint32_t)PSID_2BYTE_MIN_VALUE && psid <= (uint32_t)PSID_2BYTE_MAX_VALUE){
  } else if(psid >= (uint32_t)PSID_3BYTE_MIN_VALUE && psid <= (uint32_t)PSID_3BYTE_MAX_VALUE){
  } else if(psid >= (uint32_t)PSID_4BYTE_MIN_VALUE && psid <= (uint32_t)PSID_4BYTE_MAX_VALUE){
  } else {
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){  
      memset(tmp_ntcip_1218_mib.rsuReceivedMsgTable[index-1].rsuReceivedMsgPsid, 0x0, RSU_RADIO_PSID_SIZE);
      if(1 == length) {
          tmp_ntcip_1218_mib.rsuReceivedMsgTable[index-1].rsuReceivedMsgPsid[3] = data_in[0];
      }
      if(2 == length){
          tmp_ntcip_1218_mib.rsuReceivedMsgTable[index-1].rsuReceivedMsgPsid[3] = data_in[1];
          tmp_ntcip_1218_mib.rsuReceivedMsgTable[index-1].rsuReceivedMsgPsid[2] = data_in[0];
      }
      if(3 == length) {
          tmp_ntcip_1218_mib.rsuReceivedMsgTable[index-1].rsuReceivedMsgPsid[3] = data_in[2];
          tmp_ntcip_1218_mib.rsuReceivedMsgTable[index-1].rsuReceivedMsgPsid[2] = data_in[1];
          tmp_ntcip_1218_mib.rsuReceivedMsgTable[index-1].rsuReceivedMsgPsid[1] = data_in[0];
      }
      if(4 == length) {
          tmp_ntcip_1218_mib.rsuReceivedMsgTable[index-1].rsuReceivedMsgPsid[3] = data_in[3];
          tmp_ntcip_1218_mib.rsuReceivedMsgTable[index-1].rsuReceivedMsgPsid[2] = data_in[2];
          tmp_ntcip_1218_mib.rsuReceivedMsgTable[index-1].rsuReceivedMsgPsid[1] = data_in[1];
          tmp_ntcip_1218_mib.rsuReceivedMsgTable[index-1].rsuReceivedMsgPsid[0] = data_in[0];
      }
      tmp_ntcip_1218_mib.rsuReceivedMsgTable[index-1].rsuReceivedMsgPsid_length = length;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t commit_rsuReceivedMsgPsid(int32_t index)
{
  int32_t ret = RSEMIB_OK;
  if((index < RSU_RX_MSG_MIN) || (RSU_RX_MSG_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  } 
  if(RSEMIB_OK == ntcip1218_lock()){
      memcpy( ntcip_1218_mib.rsuReceivedMsgTable[index-1].rsuReceivedMsgPsid
            , tmp_ntcip_1218_mib.rsuReceivedMsgTable[index-1].rsuReceivedMsgPsid
            , RSU_RADIO_PSID_SIZE);
      ntcip_1218_mib.rsuReceivedMsgTable[index-1].rsuReceivedMsgPsid_length =
          tmp_ntcip_1218_mib.rsuReceivedMsgTable[index-1].rsuReceivedMsgPsid_length;
      memset(tmp_ntcip_1218_mib.rsuReceivedMsgTable[index-1].rsuReceivedMsgPsid, 0x0, RSU_RADIO_PSID_SIZE);
      tmp_ntcip_1218_mib.rsuReceivedMsgTable[index-1].rsuReceivedMsgPsid_length = 0x0;
      ntcip1218_unlock();
      return ret;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t undo_rsuReceivedMsgPsid(int32_t index)
{
  if((index < RSU_RX_MSG_MIN) || (RSU_RX_MSG_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      memcpy( ntcip_1218_mib.rsuReceivedMsgTable[index-1].rsuReceivedMsgPsid
            , prior_ntcip_1218_mib.rsuReceivedMsgTable[index-1].rsuReceivedMsgPsid, RSU_RADIO_PSID_SIZE);
      ntcip_1218_mib.rsuReceivedMsgTable[index-1].rsuReceivedMsgPsid_length =
          prior_ntcip_1218_mib.rsuReceivedMsgTable[index-1].rsuReceivedMsgPsid_length;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}

/* 5.6.2.3 */
int32_t get_rsuReceivedMsgDestIpAddr(int32_t index, uint8_t * data_out)
{
  int32_t size = 0;
  if((index < RSU_RX_MSG_MIN) || (RSU_RX_MSG_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      size = ntcip_1218_mib.rsuReceivedMsgTable[index-1].rsuReceivedMsgDestIpAddr_length;
      memcpy(data_out, ntcip_1218_mib.rsuReceivedMsgTable[index-1].rsuReceivedMsgDestIpAddr, RSU_DEST_IP_MAX);
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
  if((size < RSU_DEST_IP_MIN) || (RSU_DEST_IP_MAX < size)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);  
      return RSEMIB_BAD_DATA;
  }
  return size;
}
int32_t preserve_rsuReceivedMsgDestIpAddr(int32_t index)
{
  if((index < RSU_RX_MSG_MIN) || (RSU_RX_MSG_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      memcpy( prior_ntcip_1218_mib.rsuReceivedMsgTable[index-1].rsuReceivedMsgDestIpAddr
            , ntcip_1218_mib.rsuReceivedMsgTable[index-1].rsuReceivedMsgDestIpAddr, RSU_DEST_IP_MAX);
      prior_ntcip_1218_mib.rsuReceivedMsgTable[index-1].rsuReceivedMsgDestIpAddr_length =
          ntcip_1218_mib.rsuReceivedMsgTable[index-1].rsuReceivedMsgDestIpAddr_length;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t action_rsuReceivedMsgDestIpAddr(int32_t index, uint8_t * data_in, int32_t length)
{
  if((index < RSU_RX_MSG_MIN) || (RSU_RX_MSG_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  } 
  /* User can send us a list of nothing of size zero. */
  if((NULL == data_in) && (0 != length)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if((length < RSU_DEST_IP_MIN) || (RSU_DEST_IP_MAX < length)){
      return RSEMIB_BAD_INPUT;  
  }
  if(RSEMIB_OK == ntcip1218_lock()){  
      memset(tmp_ntcip_1218_mib.rsuReceivedMsgTable[index-1].rsuReceivedMsgDestIpAddr, 0x0, RSU_DEST_IP_MAX);
      if(NULL != data_in){
          memcpy(tmp_ntcip_1218_mib.rsuReceivedMsgTable[index-1].rsuReceivedMsgDestIpAddr, data_in, length);
      }
      tmp_ntcip_1218_mib.rsuReceivedMsgTable[index-1].rsuReceivedMsgDestIpAddr_length = length;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t commit_rsuReceivedMsgDestIpAddr(int32_t index)
{
  int32_t ret = RSEMIB_OK;
  if((index < RSU_RX_MSG_MIN) || (RSU_RX_MSG_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  } 
  if(RSEMIB_OK == ntcip1218_lock()){
      memcpy( ntcip_1218_mib.rsuReceivedMsgTable[index-1].rsuReceivedMsgDestIpAddr
            , tmp_ntcip_1218_mib.rsuReceivedMsgTable[index-1].rsuReceivedMsgDestIpAddr
            , RSU_DEST_IP_MAX);
      ntcip_1218_mib.rsuReceivedMsgTable[index-1].rsuReceivedMsgDestIpAddr_length =
          tmp_ntcip_1218_mib.rsuReceivedMsgTable[index-1].rsuReceivedMsgDestIpAddr_length;
      memset(tmp_ntcip_1218_mib.rsuReceivedMsgTable[index-1].rsuReceivedMsgDestIpAddr, 0x0, RSU_DEST_IP_MAX);
      tmp_ntcip_1218_mib.rsuReceivedMsgTable[index-1].rsuReceivedMsgDestIpAddr_length = 0;
      if(0 != set_fwdmsgIP(index, ntcip_1218_mib.rsuReceivedMsgTable[index-1].rsuReceivedMsgDestIpAddr)) {
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"commit_rsuReceivedMsgDestIpAddr: FAIL: index=%d\n",index); 
          ret = RSEMIB_BAD_DATA;
      }
      ntcip1218_unlock();
      return ret;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t undo_rsuReceivedMsgDestIpAddr(int32_t index)
{
  if((index < RSU_RX_MSG_MIN) || (RSU_RX_MSG_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      memcpy( ntcip_1218_mib.rsuReceivedMsgTable[index-1].rsuReceivedMsgDestIpAddr
            , prior_ntcip_1218_mib.rsuReceivedMsgTable[index-1].rsuReceivedMsgDestIpAddr, RSU_DEST_IP_MAX);
      ntcip_1218_mib.rsuReceivedMsgTable[index-1].rsuReceivedMsgDestIpAddr_length =
          prior_ntcip_1218_mib.rsuReceivedMsgTable[index-1].rsuReceivedMsgDestIpAddr_length;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}

/* 5.6.2.4 */
int32_t get_rsuReceivedMsgDestPort(int32_t index, int32_t * data_out)
{
  if((index < RSU_RX_MSG_MIN) || (RSU_RX_MSG_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      *data_out = ntcip_1218_mib.rsuReceivedMsgTable[index-1].rsuReceivedMsgDestPort;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
  if((*data_out < RSU_RX_MSG_PORT_MIN) || (RSU_RX_MSG_PORT_MAX < *data_out)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } 
  return RSEMIB_OK;
}
int32_t preserve_rsuReceivedMsgDestPort(int32_t index)
{
  if((index < RSU_RX_MSG_MIN) || (RSU_RX_MSG_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      prior_ntcip_1218_mib.rsuReceivedMsgTable[index-1].rsuReceivedMsgDestPort = 
          ntcip_1218_mib.rsuReceivedMsgTable[index-1].rsuReceivedMsgDestPort;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t action_rsuReceivedMsgDestPort(int32_t index, int32_t data_in)
{
  if((index < RSU_RX_MSG_MIN) || (RSU_RX_MSG_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if((data_in < RSU_RX_MSG_PORT_MIN) || (RSU_RX_MSG_PORT_MAX < data_in)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  }
  if(RSEMIB_OK == ntcip1218_lock()){  
      tmp_ntcip_1218_mib.rsuReceivedMsgTable[index-1].rsuReceivedMsgDestPort = data_in;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t commit_rsuReceivedMsgDestPort(int32_t index)
{
  int32_t ret = RSEMIB_OK;
  if((index < RSU_RX_MSG_MIN) || (RSU_RX_MSG_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuReceivedMsgTable[index-1].rsuReceivedMsgDestPort = 
          tmp_ntcip_1218_mib.rsuReceivedMsgTable[index-1].rsuReceivedMsgDestPort;
      tmp_ntcip_1218_mib.rsuReceivedMsgTable[index-1].rsuReceivedMsgDestPort = RSU_RX_MSG_PORT_DEFAULT;
      if(0 != set_fwdmsgPort(index, ntcip_1218_mib.rsuReceivedMsgTable[index-1].rsuReceivedMsgDestPort)) {
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"commit_rsuReceivedMsgDestPort: FAIL: index=%d\n",index); 
          ret = RSEMIB_BAD_DATA;
      }
      ntcip1218_unlock();
      return ret;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t undo_rsuReceivedMsgDestPort(int32_t index)
{
  if((index < RSU_RX_MSG_MIN) || (RSU_RX_MSG_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuReceivedMsgTable[index-1].rsuReceivedMsgDestPort = 
          prior_ntcip_1218_mib.rsuReceivedMsgTable[index-1].rsuReceivedMsgDestPort;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}

/* 5.6.2.5 */
int32_t get_rsuReceivedMsgProtocol(int32_t index, int32_t * data_out)
{
  if((index < RSU_RX_MSG_MIN) || (RSU_RX_MSG_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      *data_out = ntcip_1218_mib.rsuReceivedMsgTable[index-1].rsuReceivedMsgProtocol;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
  if((*data_out < protocol_other) || (protocol_udp < *data_out)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } 
  return RSEMIB_OK;
}
int32_t preserve_rsuReceivedMsgProtocol(int32_t index)
{
  if((index < RSU_RX_MSG_MIN) || (RSU_RX_MSG_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      prior_ntcip_1218_mib.rsuReceivedMsgTable[index-1].rsuReceivedMsgProtocol = 
          ntcip_1218_mib.rsuReceivedMsgTable[index-1].rsuReceivedMsgProtocol;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t action_rsuReceivedMsgProtocol(int32_t index, int32_t data_in)
{
  if((index < RSU_RX_MSG_MIN) || (RSU_RX_MSG_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if((data_in < protocol_other) || (protocol_udp < data_in)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  }
  if(RSEMIB_OK == ntcip1218_lock()){  
      tmp_ntcip_1218_mib.rsuReceivedMsgTable[index-1].rsuReceivedMsgProtocol = (protocol_e)data_in;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t commit_rsuReceivedMsgProtocol(int32_t index)
{
  int32_t ret = RSEMIB_OK;
  if((index < RSU_RX_MSG_MIN) || (RSU_RX_MSG_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuReceivedMsgTable[index-1].rsuReceivedMsgProtocol = 
          tmp_ntcip_1218_mib.rsuReceivedMsgTable[index-1].rsuReceivedMsgProtocol;
      tmp_ntcip_1218_mib.rsuReceivedMsgTable[index-1].rsuReceivedMsgProtocol = protocol_udp;
      ntcip1218_unlock();
      return ret;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t undo_rsuReceivedMsgProtocol(int32_t index)
{
  if((index < RSU_RX_MSG_MIN) || (RSU_RX_MSG_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuReceivedMsgTable[index-1].rsuReceivedMsgProtocol = 
          prior_ntcip_1218_mib.rsuReceivedMsgTable[index-1].rsuReceivedMsgProtocol;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}

/* 5.6.2.6 */
int32_t get_rsuReceivedMsgRssi(int32_t index, int32_t * data_out)
{
  if((index < RSU_RX_MSG_MIN) || (RSU_RX_MSG_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      *data_out = ntcip_1218_mib.rsuReceivedMsgTable[index-1].rsuReceivedMsgRssi;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
  if((*data_out < RSU_RX_MSG_RSSI_THRESHOLD_MIN) || (RSU_RX_MSG_RSSI_THRESHOLD_MAX < *data_out)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } 
  return RSEMIB_OK;
}
int32_t preserve_rsuReceivedMsgRssi(int32_t index)
{
  if((index < RSU_RX_MSG_MIN) || (RSU_RX_MSG_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      prior_ntcip_1218_mib.rsuReceivedMsgTable[index-1].rsuReceivedMsgRssi = 
          ntcip_1218_mib.rsuReceivedMsgTable[index-1].rsuReceivedMsgRssi;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t action_rsuReceivedMsgRssi(int32_t index, int32_t data_in)
{
  if((index < RSU_RX_MSG_MIN) || (RSU_RX_MSG_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if((data_in < RSU_RX_MSG_RSSI_THRESHOLD_MIN) || (RSU_RX_MSG_RSSI_THRESHOLD_MAX < data_in)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  }
  if(RSEMIB_OK == ntcip1218_lock()){  
      tmp_ntcip_1218_mib.rsuReceivedMsgTable[index-1].rsuReceivedMsgRssi = (protocol_e)data_in;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t commit_rsuReceivedMsgRssi(int32_t index)
{
  int32_t ret = RSEMIB_OK;
  if((index < RSU_RX_MSG_MIN) || (RSU_RX_MSG_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuReceivedMsgTable[index-1].rsuReceivedMsgRssi = tmp_ntcip_1218_mib.rsuReceivedMsgTable[index-1].rsuReceivedMsgRssi;
      tmp_ntcip_1218_mib.rsuReceivedMsgTable[index-1].rsuReceivedMsgRssi = RSU_RX_MSG_RSSI_THRESHOLD_MIN;
      ntcip1218_unlock();
      return ret;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t undo_rsuReceivedMsgRssi(int32_t index)
{
  if((index < RSU_RX_MSG_MIN) || (RSU_RX_MSG_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuReceivedMsgTable[index-1].rsuReceivedMsgRssi = prior_ntcip_1218_mib.rsuReceivedMsgTable[index-1].rsuReceivedMsgRssi;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}

/* 5.6.2.7 */
int32_t get_rsuReceivedMsgInterval(int32_t index, int32_t * data_out)
{
  if((index < RSU_RX_MSG_MIN) || (RSU_RX_MSG_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      *data_out = ntcip_1218_mib.rsuReceivedMsgTable[index-1].rsuReceivedMsgInterval;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
  if((*data_out < RSU_RX_MSG_INTERVAL_MIN) || (RSU_RX_MSG_INTERVAL_MAX < *data_out)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } 
  return RSEMIB_OK;
}
int32_t preserve_rsuReceivedMsgInterval(int32_t index)
{
  if((index < RSU_RX_MSG_MIN) || (RSU_RX_MSG_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      prior_ntcip_1218_mib.rsuReceivedMsgTable[index-1].rsuReceivedMsgInterval = 
          ntcip_1218_mib.rsuReceivedMsgTable[index-1].rsuReceivedMsgInterval;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t action_rsuReceivedMsgInterval(int32_t index, int32_t data_in)
{
  if((index < RSU_RX_MSG_MIN) || (RSU_RX_MSG_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if((data_in < RSU_RX_MSG_INTERVAL_MIN) || (RSU_RX_MSG_INTERVAL_MAX < data_in)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  }
  if(RSEMIB_OK == ntcip1218_lock()){  
      tmp_ntcip_1218_mib.rsuReceivedMsgTable[index-1].rsuReceivedMsgInterval = (protocol_e)data_in;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t commit_rsuReceivedMsgInterval(int32_t index)
{
  int32_t ret = RSEMIB_OK;
  if((index < RSU_RX_MSG_MIN) || (RSU_RX_MSG_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuReceivedMsgTable[index-1].rsuReceivedMsgInterval = 
          tmp_ntcip_1218_mib.rsuReceivedMsgTable[index-1].rsuReceivedMsgInterval;
      tmp_ntcip_1218_mib.rsuReceivedMsgTable[index-1].rsuReceivedMsgInterval = RSU_RX_MSG_INTERVAL_MIN;
      ntcip1218_unlock();
      return ret;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t undo_rsuReceivedMsgInterval(int32_t index)
{
  if((index < RSU_RX_MSG_MIN) || (RSU_RX_MSG_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuReceivedMsgTable[index-1].rsuReceivedMsgInterval = 
          prior_ntcip_1218_mib.rsuReceivedMsgTable[index-1].rsuReceivedMsgInterval;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}

/* 5.6.2.8 */
int32_t get_rsuReceivedMsgDeliveryStart(int32_t index, uint8_t * data_out)
{
  if((index < RSU_RX_MSG_MIN) || (RSU_RX_MSG_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      memcpy(data_out, ntcip_1218_mib.rsuReceivedMsgTable[index-1].rsuReceivedMsgDeliveryStart, MIB_DATEANDTIME_LENGTH);
      ntcip1218_unlock();
      return MIB_DATEANDTIME_LENGTH;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t preserve_rsuReceivedMsgDeliveryStart(int32_t index)
{
  if((index < RSU_RX_MSG_MIN) || (RSU_RX_MSG_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      memcpy( prior_ntcip_1218_mib.rsuReceivedMsgTable[index-1].rsuReceivedMsgDeliveryStart
            , ntcip_1218_mib.rsuReceivedMsgTable[index-1].rsuReceivedMsgDeliveryStart, MIB_DATEANDTIME_LENGTH);
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t action_rsuReceivedMsgDeliveryStart(int32_t index, uint8_t * data_in, int32_t length)
{
  if((index < RSU_RX_MSG_MIN) || (RSU_RX_MSG_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(NULL == data_in){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(MIB_DATEANDTIME_LENGTH != length){
      return RSEMIB_BAD_INPUT;  
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      memset(tmp_ntcip_1218_mib.rsuReceivedMsgTable[index-1].rsuReceivedMsgDeliveryStart, 0x0, MIB_DATEANDTIME_LENGTH);
      memcpy(tmp_ntcip_1218_mib.rsuReceivedMsgTable[index-1].rsuReceivedMsgDeliveryStart, data_in, length);
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t commit_rsuReceivedMsgDeliveryStart(int32_t index)
{
  int32_t ret = RSEMIB_OK;
  if((index < RSU_RX_MSG_MIN) || (RSU_RX_MSG_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      memcpy( ntcip_1218_mib.rsuReceivedMsgTable[index-1].rsuReceivedMsgDeliveryStart
            , tmp_ntcip_1218_mib.rsuReceivedMsgTable[index-1].rsuReceivedMsgDeliveryStart
            , MIB_DATEANDTIME_LENGTH);
      memset(tmp_ntcip_1218_mib.rsuReceivedMsgTable[index-1].rsuReceivedMsgDeliveryStart, 0x0, MIB_DATEANDTIME_LENGTH);
      ntcip1218_unlock();
      return ret;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t undo_rsuReceivedMsgDeliveryStart(int32_t index)
{
  if((index < RSU_RX_MSG_MIN) || (RSU_RX_MSG_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      memcpy( ntcip_1218_mib.rsuReceivedMsgTable[index-1].rsuReceivedMsgDeliveryStart
            , prior_ntcip_1218_mib.rsuReceivedMsgTable[index-1].rsuReceivedMsgDeliveryStart, MIB_DATEANDTIME_LENGTH);
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}

/* 5.6.2.9 */
int32_t get_rsuReceivedMsgDeliveryStop(int32_t index, uint8_t * data_out)
{
  if((index < RSU_RX_MSG_MIN) || (RSU_RX_MSG_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      memcpy(data_out, ntcip_1218_mib.rsuReceivedMsgTable[index-1].rsuReceivedMsgDeliveryStop, MIB_DATEANDTIME_LENGTH);
      ntcip1218_unlock();
      return MIB_DATEANDTIME_LENGTH;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t preserve_rsuReceivedMsgDeliveryStop(int32_t index)
{
  if((index < RSU_RX_MSG_MIN) || (RSU_RX_MSG_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      memcpy( prior_ntcip_1218_mib.rsuReceivedMsgTable[index-1].rsuReceivedMsgDeliveryStop
            , ntcip_1218_mib.rsuReceivedMsgTable[index-1].rsuReceivedMsgDeliveryStop, MIB_DATEANDTIME_LENGTH);
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t action_rsuReceivedMsgDeliveryStop(int32_t index, uint8_t * data_in, int32_t length)
{
  if((index < RSU_RX_MSG_MIN) || (RSU_RX_MSG_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(NULL == data_in){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(MIB_DATEANDTIME_LENGTH != length){
      return RSEMIB_BAD_INPUT;  
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      memset(tmp_ntcip_1218_mib.rsuReceivedMsgTable[index-1].rsuReceivedMsgDeliveryStop, 0x0, MIB_DATEANDTIME_LENGTH);
      memcpy(tmp_ntcip_1218_mib.rsuReceivedMsgTable[index-1].rsuReceivedMsgDeliveryStop, data_in, length);
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t commit_rsuReceivedMsgDeliveryStop(int32_t index)
{
  int32_t ret = RSEMIB_OK;
  if((index < RSU_RX_MSG_MIN) || (RSU_RX_MSG_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      memcpy( ntcip_1218_mib.rsuReceivedMsgTable[index-1].rsuReceivedMsgDeliveryStop
            , tmp_ntcip_1218_mib.rsuReceivedMsgTable[index-1].rsuReceivedMsgDeliveryStop, MIB_DATEANDTIME_LENGTH);
      memset(tmp_ntcip_1218_mib.rsuReceivedMsgTable[index-1].rsuReceivedMsgDeliveryStop, 0x0, MIB_DATEANDTIME_LENGTH);
      ntcip1218_unlock();
      return ret;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t undo_rsuReceivedMsgDeliveryStop(int32_t index)
{
  if((index < RSU_RX_MSG_MIN) || (RSU_RX_MSG_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      memcpy( ntcip_1218_mib.rsuReceivedMsgTable[index-1].rsuReceivedMsgDeliveryStop
            , prior_ntcip_1218_mib.rsuReceivedMsgTable[index-1].rsuReceivedMsgDeliveryStop, MIB_DATEANDTIME_LENGTH);
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
/* 5.6.2.10 */
int32_t get_rsuReceivedMsgStatus(int32_t index, int32_t * data_out)
{
  if((index < RSU_RX_MSG_MIN) || (RSU_RX_MSG_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      *data_out = ntcip_1218_mib.rsuReceivedMsgTable[index-1].rsuReceivedMsgStatus;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  } 
  if((ROW_STATUS_VALID_MIN <= *data_out ) && (*data_out <= ROW_STATUS_VALID_MAX)){
      return RSEMIB_OK; 
  } else {
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } 
}
int32_t preserve_rsuReceivedMsgStatus(int32_t index)
{
  if((index < RSU_RX_MSG_MIN) || (RSU_RX_MSG_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      set_default_row_rsuReceivedMsg(&tmp_ntcip_1218_mib.rsuReceivedMsgTable[index-1],index-1);
      prior_ntcip_1218_mib.rsuReceivedMsgTable[index-1].rsuReceivedMsgStatus = 
          ntcip_1218_mib.rsuReceivedMsgTable[index-1].rsuReceivedMsgStatus;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t action_rsuReceivedMsgStatus(int32_t index, int32_t data_in)
{
  if((index < RSU_RX_MSG_MIN) || (RSU_RX_MSG_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if((data_in < ROW_STATUS_VALID_MIN) || (ROW_STATUS_VALID_MAX < data_in)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  }
  if(RSEMIB_OK == ntcip1218_lock()){  
      tmp_ntcip_1218_mib.rsuReceivedMsgTable[index-1].rsuReceivedMsgStatus = data_in;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
/* Note: Update msgfwd.conf first before init of row and rebuild of table. */
int32_t commit_rsuReceivedMsgStatus(int32_t index)
{
  int32_t ret = RSEMIB_OK;

  if((index < RSU_RX_MSG_MIN) || (RSU_RX_MSG_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      ret = RSEMIB_BAD_INDEX;
  }
  if((RSEMIB_OK == ret) && (RSEMIB_OK == ntcip1218_lock())){
      DEBUGMSGTL((MY_NAME_EXTRA, "commit_rsuReceivedMsgStatus: index=%d status=%d.\n", index,tmp_ntcip_1218_mib.rsuReceivedMsgTable[index-1].rsuReceivedMsgStatus));
      switch(tmp_ntcip_1218_mib.rsuReceivedMsgTable[index-1].rsuReceivedMsgStatus){
          case SNMP_ROW_CREATEANDGO:
              if(SNMP_ROW_NONEXISTENT == ntcip_1218_mib.rsuReceivedMsgTable[index-1].rsuReceivedMsgStatus) {
                  if(0 == set_fwdmsgEnable(index, 1)){
                      if(0 == set_fwdmsgMask(index, FWDMSG_ALL_PSID_MASK)) {
                          set_default_row_rsuReceivedMsg(&ntcip_1218_mib.rsuReceivedMsgTable[index-1],index-1);
                          ntcip_1218_mib.rsuReceivedMsgTable[index-1].rsuReceivedMsgStatus = SNMP_ROW_ACTIVE;
                          /* Rebuild helper table before ack. */
                          if (RSEMIB_OK != (ret = rebuild_rsuReceivedMsg_live(&ntcip_1218_mib.rsuReceivedMsgTable[0]))){
                              ret = RSEMIB_BAD_DATA;
                          }
                      } else {
                          I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"commit_rsuReceivedMsgStatus: SNMP_ROW_CREATEANDGO: set_fwdmsgMask FAIL: index=%d\n",index); 
                          ret = RSEMIB_BAD_DATA;                    
                      }
                  } else {
                      I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"commit_rsuReceivedMsgStatus: SNMP_ROW_CREATEANDGO: FAIL: index=%d\n",index); 
                      ret = RSEMIB_BAD_DATA;
                  }
              } else {
                  /* If it exists already then do nothing. Dont send error back. */
                  //ret = RSEMIB_BAD_MODE;
              }
              break;
          case SNMP_ROW_CREATEANDWAIT:
              if(SNMP_ROW_NONEXISTENT == ntcip_1218_mib.rsuReceivedMsgTable[index-1].rsuReceivedMsgStatus) {
                  if(0 == set_fwdmsgEnable(index, 1)){
                      if(0 == set_fwdmsgMask(index, FWDMSG_ALL_PSID_MASK)) {
                          set_default_row_rsuReceivedMsg(&ntcip_1218_mib.rsuReceivedMsgTable[index-1],index-1);
                          ntcip_1218_mib.rsuReceivedMsgTable[index-1].rsuReceivedMsgStatus = SNMP_ROW_NOTINSERVICE;
                          /* Rebuild helper table before ack. */
                          if (RSEMIB_OK != (ret = rebuild_rsuReceivedMsg_live(&ntcip_1218_mib.rsuReceivedMsgTable[0]))){
                              ret = RSEMIB_BAD_DATA;
                          }
                      } else {
                          I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"commit_rsuReceivedMsgStatus: SNMP_ROW_CREATEANDWAIT: set_fwdmsgMask FAIL: index=%d\n",index); 
                          ret = RSEMIB_BAD_DATA;                    
                      }
                  } else {
                      I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"commit_rsuReceivedMsgStatus: SNMP_ROW_CREATEANDWAIT: FAIL: index=%d\n",index); 
                      ret = RSEMIB_BAD_DATA;
                  }
              } else {
                  /* If it exists already then do nothing. Dont send error back. */
                  //ret = RSEMIB_BAD_MODE;
              }
              break;
          case SNMP_ROW_DESTROY: /* User deletes row from table. */
              if(SNMP_ROW_NONEXISTENT == ntcip_1218_mib.rsuReceivedMsgTable[index-1].rsuReceivedMsgStatus){
                  ret = RSEMIB_OK; /* If you try to DESTROY non-exixtent row that's an easy one. */
              } else {
                  if(0 == set_fwdmsgEnable(index, 0)){
                      set_default_row_rsuReceivedMsg(&ntcip_1218_mib.rsuReceivedMsgTable[index-1],index-1);
                      set_default_row_rsuReceivedMsg(&tmp_ntcip_1218_mib.rsuReceivedMsgTable[index-1],index-1);
                      set_default_row_rsuReceivedMsg(&prior_ntcip_1218_mib.rsuReceivedMsgTable[index-1],index-1);
                      /* Rebuild helper table before ack. Pass live pointer of db. */
                      if (RSEMIB_OK != (ret = rebuild_rsuReceivedMsg_live(&ntcip_1218_mib.rsuReceivedMsgTable[0]))){
                          ret = RSEMIB_BAD_DATA;
                      }
                  } else {
                      I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"commit_rsuReceivedMsgStatus: SNMP_ROW_DESTROY: FAIL: index=%d\n",index); 
                      ret = RSEMIB_BAD_DATA;
                  }
              }
              break;
          case SNMP_ROW_ACTIVE:
              ntcip_1218_mib.rsuReceivedMsgTable[index-1].rsuReceivedMsgStatus = SNMP_ROW_ACTIVE;
              break;
          case SNMP_ROW_NOTINSERVICE:
              if(0 == set_fwdmsgEnable(index, 0)){
                  ntcip_1218_mib.rsuReceivedMsgTable[index-1].rsuReceivedMsgStatus = SNMP_ROW_NOTINSERVICE;
                  /* Rebuild helper table before ack. */
                  if (RSEMIB_OK != (ret = rebuild_rsuReceivedMsg_live(&ntcip_1218_mib.rsuReceivedMsgTable[0]))){
                      ret = RSEMIB_BAD_DATA;
                  }
              } else {
                  I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"commit_rsuReceivedMsgStatus: SNMP_ROW_NOTINSERVICE: FAIL: index=%d\n",index); 
                  ret = RSEMIB_BAD_DATA;
              }
              break;
          case SNMP_ROW_NOTREADY:
              if(0 == set_fwdmsgEnable(index, 0)){
                  ntcip_1218_mib.rsuReceivedMsgTable[index-1].rsuReceivedMsgStatus = SNMP_ROW_NOTREADY;
                  /* Rebuild helper table before ack. */
                  if (RSEMIB_OK != (ret = rebuild_rsuReceivedMsg_live(&ntcip_1218_mib.rsuReceivedMsgTable[0]))){
                      ret = RSEMIB_BAD_DATA;
                  }
              } else {
                  I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"commit_rsuReceivedMsgStatus: SNMP_ROW_NOTREADY: FAIL: index=%d\n",index); 
                  ret = RSEMIB_BAD_DATA;
              }
              break;
          case SNMP_ROW_NONEXISTENT:
          default:
              ret = RSEMIB_BAD_MODE;
              break;
      }
      /* Clear tmp. */
      tmp_ntcip_1218_mib.rsuReceivedMsgTable[index-1].rsuReceivedMsgStatus = ROW_STATUS_VALID_DEFAULT;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      ret = RSEMIB_LOCK_FAIL;
  }
  return ret;
}
int32_t undo_rsuReceivedMsgStatus(int32_t index)
{
  if((index < RSU_RX_MSG_MIN) || (RSU_RX_MSG_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuReceivedMsgTable[index-1].rsuReceivedMsgStatus = 
          prior_ntcip_1218_mib.rsuReceivedMsgTable[index-1].rsuReceivedMsgStatus;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}

/* 5.6.2.11 */
int32_t get_rsuReceivedMsgSecure(int32_t index, int32_t * data_out)
{
  if((index < RSU_RX_MSG_MIN) || (RSU_RX_MSG_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      *data_out = ntcip_1218_mib.rsuReceivedMsgTable[index-1].rsuReceivedMsgSecure;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
  if((*data_out < 0) || (1 < *data_out)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } 
  return RSEMIB_OK;

}
int32_t preserve_rsuReceivedMsgSecure(int32_t index)
{
  if((index < RSU_RX_MSG_MIN) || (RSU_RX_MSG_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      prior_ntcip_1218_mib.rsuReceivedMsgTable[index-1].rsuReceivedMsgSecure = 
          ntcip_1218_mib.rsuReceivedMsgTable[index-1].rsuReceivedMsgSecure;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t action_rsuReceivedMsgSecure(int32_t index, int32_t data_in)
{
  if((index < RSU_RX_MSG_MIN) || (RSU_RX_MSG_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if((data_in < 0) || (1 < data_in)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  }
  if(RSEMIB_OK == ntcip1218_lock()){  
      tmp_ntcip_1218_mib.rsuReceivedMsgTable[index-1].rsuReceivedMsgSecure = data_in;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t commit_rsuReceivedMsgSecure(int32_t index)
{
  int32_t ret = RSEMIB_OK;
  if((index < RSU_RX_MSG_MIN) || (RSU_RX_MSG_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuReceivedMsgTable[index-1].rsuReceivedMsgSecure = 
          tmp_ntcip_1218_mib.rsuReceivedMsgTable[index-1].rsuReceivedMsgSecure;
      tmp_ntcip_1218_mib.rsuReceivedMsgTable[index-1].rsuReceivedMsgSecure = 0;
      ntcip1218_unlock();
      return ret;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t undo_rsuReceivedMsgSecure(int32_t index)
{
  if((index < RSU_RX_MSG_MIN) || (RSU_RX_MSG_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuReceivedMsgTable[index-1].rsuReceivedMsgSecure = 
          prior_ntcip_1218_mib.rsuReceivedMsgTable[index-1].rsuReceivedMsgSecure;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}

/* 5.6.2.12 */
int32_t get_rsuReceivedMsgAuthMsgInterval(int32_t index, int32_t * data_out)
{
  if((index < RSU_RX_MSG_MIN) || (RSU_RX_MSG_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      *data_out = ntcip_1218_mib.rsuReceivedMsgTable[index-1].rsuReceivedMsgAuthMsgInterval;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
  if((*data_out < RSU_RX_MSG_AUTH_INTERVAL_MIN) || (RSU_RX_MSG_AUTH_INTERVAL_MAX < *data_out)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } 
  return RSEMIB_OK;
}
int32_t preserve_rsuReceivedMsgAuthMsgInterval(int32_t index)
{
  if((index < RSU_RX_MSG_MIN) || (RSU_RX_MSG_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      prior_ntcip_1218_mib.rsuReceivedMsgTable[index-1].rsuReceivedMsgAuthMsgInterval = 
          ntcip_1218_mib.rsuReceivedMsgTable[index-1].rsuReceivedMsgAuthMsgInterval;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t action_rsuReceivedMsgAuthMsgInterval(int32_t index, int32_t data_in)
{
  if((index < RSU_RX_MSG_MIN) || (RSU_RX_MSG_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if((data_in < RSU_RX_MSG_AUTH_INTERVAL_MIN) || (RSU_RX_MSG_AUTH_INTERVAL_MAX < data_in)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  }
  if(RSEMIB_OK == ntcip1218_lock()){  
      tmp_ntcip_1218_mib.rsuReceivedMsgTable[index-1].rsuReceivedMsgAuthMsgInterval = data_in;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t commit_rsuReceivedMsgAuthMsgInterval(int32_t index)
{
  int32_t ret = RSEMIB_OK;
  if((index < RSU_RX_MSG_MIN) || (RSU_RX_MSG_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuReceivedMsgTable[index-1].rsuReceivedMsgAuthMsgInterval = 
          tmp_ntcip_1218_mib.rsuReceivedMsgTable[index-1].rsuReceivedMsgAuthMsgInterval;
      tmp_ntcip_1218_mib.rsuReceivedMsgTable[index-1].rsuReceivedMsgAuthMsgInterval = RSU_RX_MSG_AUTH_INTERVAL_MIN;
      ntcip1218_unlock();
      return ret;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t undo_rsuReceivedMsgAuthMsgInterval(int32_t index)
{
  if((index < RSU_RX_MSG_MIN) || (RSU_RX_MSG_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuReceivedMsgTable[index-1].rsuReceivedMsgAuthMsgInterval = 
          prior_ntcip_1218_mib.rsuReceivedMsgTable[index-1].rsuReceivedMsgAuthMsgInterval;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}

/****************************************************************************** 
 * 5.7 GNSS Output: { rsu 6 }: rsuGnssOutput.c
 ******************************************************************************/

/* 5.7.1 */
int32_t get_rsuGnssOutputPort(void)
{
  int32_t data = 0;
  if(RSEMIB_OK == ntcip1218_lock()){
      data = ntcip_1218_mib.rsuGnssOutputPort;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  } 
  if((RSU_GNSS_OUTPUT_PORT_MIN <= data) && (data <= RSU_GNSS_OUTPUT_PORT_MAX)){
      return data; 
  } else {
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } 
}
int32_t preserve_rsuGnssOutputPort(void)
{
  if(RSEMIB_OK == ntcip1218_lock()){
      prior_ntcip_1218_mib.rsuGnssOutputPort = ntcip_1218_mib.rsuGnssOutputPort;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t action_rsuGnssOutputPort(int32_t data)
{
  if((data < RSU_GNSS_OUTPUT_PORT_MIN) || (RSU_GNSS_OUTPUT_PORT_MAX < data)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  }
  if(RSEMIB_OK == ntcip1218_lock()){  
      tmp_ntcip_1218_mib.rsuGnssOutputPort = data;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t commit_rsuGnssOutputPort(void)
{
  int32_t ret = RSEMIB_OK;
  GnssOutput_t gnssOutput;

  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuGnssOutputPort = tmp_ntcip_1218_mib.rsuGnssOutputPort; 
      memset(&gnssOutput,0x0,sizeof(gnssOutput));
      if(RSEMIB_OK == (copy_gnssOutput_to_skinny(&gnssOutput))){
          if(RSEMIB_OK != commit_gnssOutput_to_disk(&gnssOutput)) {
              ret = RSEMIB_BAD_DATA;
          }
      } else {
          ret = RSEMIB_BAD_DATA;
      }
      tmp_ntcip_1218_mib.rsuGnssOutputPort = RSU_GNSS_OUTPUT_PORT_DEFAULT;
      ntcip1218_unlock();
  } else {
      ret = RSEMIB_LOCK_FAIL;
  }
  if(ret < RSEMIB_OK)
      set_ntcip_1218_error_states(ret);
  return ret;
}
int32_t undo_rsuGnssOutputPort(void)
{
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuGnssOutputPort = prior_ntcip_1218_mib.rsuGnssOutputPort;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}

/* 5.7.2 */
int32_t get_rsuGnssOutputAddress(uint8_t * data_out)
{
  int32_t size = 0;
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      size = ntcip_1218_mib.rsuGnssOutputAddress_length;
      memcpy(data_out, ntcip_1218_mib.rsuGnssOutputAddress, RSU_DEST_IP_MAX);
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
  if((size < RSU_DEST_IP_MIN) || (RSU_DEST_IP_MAX < size)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      size = RSEMIB_BAD_DATA;    
  }
  return size;
}
int32_t preserve_rsuGnssOutputAddress(void)
{
  if(RSEMIB_OK == ntcip1218_lock()){
      memcpy(  prior_ntcip_1218_mib.rsuGnssOutputAddress, ntcip_1218_mib.rsuGnssOutputAddress, RSU_DEST_IP_MAX);
      prior_ntcip_1218_mib.rsuGnssOutputAddress_length = ntcip_1218_mib.rsuGnssOutputAddress_length;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t action_rsuGnssOutputAddress(uint8_t * data_in, int32_t length)
{
  if((NULL == data_in) && (0 != length)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if((length < RSU_DEST_IP_MIN) || (RSU_DEST_IP_MAX < length)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;    
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      memset(tmp_ntcip_1218_mib.rsuGnssOutputAddress,0x0,RSU_DEST_IP_MAX);
      if(NULL != data_in){
          memcpy(tmp_ntcip_1218_mib.rsuGnssOutputAddress, data_in, RSU_DEST_IP_MAX);
      }
      tmp_ntcip_1218_mib.rsuGnssOutputAddress_length = length;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t commit_rsuGnssOutputAddress(void)
{
  int32_t ret = RSEMIB_OK;
  GnssOutput_t gnssOutput;

  if(RSEMIB_OK == ntcip1218_lock()){
      memcpy(ntcip_1218_mib.rsuGnssOutputAddress, tmp_ntcip_1218_mib.rsuGnssOutputAddress, RSU_DEST_IP_MAX);
      ntcip_1218_mib.rsuGnssOutputAddress_length = tmp_ntcip_1218_mib.rsuGnssOutputAddress_length;
      memset(&gnssOutput,0x0,sizeof(gnssOutput));
      if(RSEMIB_OK == (copy_gnssOutput_to_skinny(&gnssOutput))){
          if(RSEMIB_OK != commit_gnssOutput_to_disk(&gnssOutput)) {
              ret = RSEMIB_BAD_DATA;
          }
      } else {
          ret = RSEMIB_BAD_DATA;
      }
      memset(tmp_ntcip_1218_mib.rsuGnssOutputAddress, 0x0, RSU_DEST_IP_MAX);
      tmp_ntcip_1218_mib.rsuGnssOutputAddress_length = 0;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      ret = RSEMIB_LOCK_FAIL;
  }
  if(ret < RSEMIB_OK)
      set_ntcip_1218_error_states(ret);
  return ret;
}
int32_t undo_rsuGnssOutputAddress(void)
{
  if(RSEMIB_OK == ntcip1218_lock()){
      memcpy( ntcip_1218_mib.rsuGnssOutputAddress, prior_ntcip_1218_mib.rsuGnssOutputAddress, RSU_DEST_IP_MAX);
      ntcip_1218_mib.rsuGnssOutputAddress_length = prior_ntcip_1218_mib.rsuGnssOutputAddress_length;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}

/* 5.7.3 */
int32_t get_rsuGnssOutputInterface(uint8_t * data_out)
{
  int32_t size=0;
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      size = ntcip_1218_mib.rsuGnssOutputInterface_length;
      memcpy(data_out,ntcip_1218_mib.rsuGnssOutputInterface,RSU_GNSS_OUTPUT_INTERFACE_MAX);
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
  if((size < RSU_GNSS_OUTPUT_INTERFACE_MIN) || (RSU_GNSS_OUTPUT_INTERFACE_MAX < size)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  }
  return size;
}
int32_t preserve_rsuGnssOutputInterface(void)
{
  if(RSEMIB_OK == ntcip1218_lock()){
      memcpy(prior_ntcip_1218_mib.rsuGnssOutputInterface, ntcip_1218_mib.rsuGnssOutputInterface, RSU_GNSS_OUTPUT_INTERFACE_MAX);
      prior_ntcip_1218_mib.rsuGnssOutputInterface_length = ntcip_1218_mib.rsuGnssOutputInterface_length;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t action_rsuGnssOutputInterface(uint8_t * data_in, int32_t length)
{
  if((NULL == data_in)  && (0 != length)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }          
  if((length < RSU_GNSS_OUTPUT_INTERFACE_MIN) || (RSU_GNSS_OUTPUT_INTERFACE_MAX < length)){ 
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA); 
      return RSEMIB_BAD_DATA;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      memset(tmp_ntcip_1218_mib.rsuGnssOutputInterface, 0x0 , RSU_GNSS_OUTPUT_INTERFACE_MAX); 
      if(NULL != data_in){ 
          memcpy(tmp_ntcip_1218_mib.rsuGnssOutputInterface, data_in, length);
      }
      tmp_ntcip_1218_mib.rsuGnssOutputInterface_length = length;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t commit_rsuGnssOutputInterface(void)
{
  int32_t ret = RSEMIB_OK;
  GnssOutput_t gnssOutput;

  if(RSEMIB_OK == ntcip1218_lock()){
      memcpy(ntcip_1218_mib.rsuGnssOutputInterface, tmp_ntcip_1218_mib.rsuGnssOutputInterface, RSU_GNSS_OUTPUT_INTERFACE_MAX);
      ntcip_1218_mib.rsuGnssOutputInterface_length = tmp_ntcip_1218_mib.rsuGnssOutputInterface_length;
      memset(&gnssOutput,0x0,sizeof(gnssOutput));
      if(RSEMIB_OK == (copy_gnssOutput_to_skinny(&gnssOutput))){
          if(RSEMIB_OK != commit_gnssOutput_to_disk(&gnssOutput)) {
              ret = RSEMIB_BAD_DATA;
          }
      } else {
          ret = RSEMIB_BAD_DATA;
      }
      memset(tmp_ntcip_1218_mib.rsuGnssOutputInterface, 0x0, RSU_GNSS_OUTPUT_INTERFACE_MAX);
      tmp_ntcip_1218_mib.rsuGnssOutputInterface_length = 0;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      ret = RSEMIB_LOCK_FAIL;
  }
  if(ret < RSEMIB_OK)
      set_ntcip_1218_error_states(ret);
  return ret;
}

int32_t undo_rsuGnssOutputInterface(void)
{
  if(RSEMIB_OK == ntcip1218_lock()){
      memcpy(ntcip_1218_mib.rsuGnssOutputInterface, prior_ntcip_1218_mib.rsuGnssOutputInterface, RSU_GNSS_OUTPUT_INTERFACE_MAX);
      ntcip_1218_mib.rsuGnssOutputInterface_length = prior_ntcip_1218_mib.rsuGnssOutputInterface_length;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}

/* 5.7.4 */
int32_t get_rsuGnssOutputInterval(void)
{
  int32_t data = 0;
  if(RSEMIB_OK == ntcip1218_lock()){
      data = ntcip_1218_mib.rsuGnssOutputInterval;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  } 
  if((RSU_GNSS_OUTPUT_INTERVAL_MIN <= data) && (data <= RSU_GNSS_OUTPUT_INTERVAL_MAX)){
      return data; 
  } else {
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } 
}
int32_t preserve_rsuGnssOutputInterval(void)
{
  if(RSEMIB_OK == ntcip1218_lock()){
      prior_ntcip_1218_mib.rsuGnssOutputInterval = ntcip_1218_mib.rsuGnssOutputInterval;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t action_rsuGnssOutputInterval(int32_t data)
{
  if((data < RSU_GNSS_OUTPUT_INTERVAL_MIN) || (RSU_GNSS_OUTPUT_INTERVAL_MAX < data)){ 
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  }
  if(RSEMIB_OK == ntcip1218_lock()){  
      tmp_ntcip_1218_mib.rsuGnssOutputInterval = data;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t commit_rsuGnssOutputInterval(void)
{
  int32_t ret = RSEMIB_OK;
  GnssOutput_t gnssOutput;

  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuGnssOutputInterval     = tmp_ntcip_1218_mib.rsuGnssOutputInterval;
      memset(&gnssOutput,0x0,sizeof(gnssOutput));
      if(RSEMIB_OK == (copy_gnssOutput_to_skinny(&gnssOutput))){
          if(RSEMIB_OK != commit_gnssOutput_to_disk(&gnssOutput)) {
              ret = RSEMIB_BAD_DATA;
          }
      } else {
          ret = RSEMIB_BAD_DATA;
      } 
      tmp_ntcip_1218_mib.rsuGnssOutputInterval = RSU_GNSS_OUTPUT_INTERVAL_DEFAULT;
      ntcip1218_unlock();
  } else {
      ret = RSEMIB_LOCK_FAIL;
  }
  if(ret < RSEMIB_OK)
      set_ntcip_1218_error_states(ret);
  return ret;
}
int32_t undo_rsuGnssOutputInterval(void)
{
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuGnssOutputInterval = prior_ntcip_1218_mib.rsuGnssOutputInterval;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}

/* 5.7.5 */
int32_t get_rsuGnssOutputString(uint8_t * data_out)
{
  int32_t size=0;
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      size = ntcip_1218_mib.rsuGnssOutputString_length;
      memcpy(data_out,ntcip_1218_mib.rsuGnssOutputString,RSU_GNSS_OUTPUT_STRING_MAX);
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
  if((size < RSU_GNSS_OUTPUT_STRING_MIN) || (RSU_GNSS_OUTPUT_STRING_MAX < size)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_DATA;
  } 
  return size;
}

/* 5.7.6 */
int32_t get_rsuGnssLat(int32_t * data_out)
{
  int32_t data = RSU_LAT_UNKNOWN;
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      data = ntcip_1218_mib.rsuGnssLat;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  } 
  if((RSU_LAT_MIN <= data) && (data <= RSU_LAT_MAX)){
      *data_out = data;
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } 
}

/* 5.7.7 */
int32_t get_rsuGnssLon(int32_t * data_out)
{
  int32_t data = RSU_LON_UNKNOWN;
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      data = ntcip_1218_mib.rsuGnssLon;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  } 
  if((RSU_LON_MIN <= data) && (data <= RSU_LON_MAX)){
      *data_out = data;
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } 
}

/* 5.7.8 */
int32_t get_rsuGnssElv(int32_t * data_out)
{
  int32_t data = RSU_ELV_UNKNOWN;
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      data = ntcip_1218_mib.rsuGnssElv;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  } 
  if((RSU_ELV_MIN <= data) && (data <= RSU_ELV_MAX)){
      *data_out = data;
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } 
}

/* 5.7.9 */
int32_t get_rsuGnssMaxDeviation(int32_t * data_out)
{
  int32_t data = 0;
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      data = ntcip_1218_mib.rsuGnssMaxDeviation;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  } 
  if((RSU_GNSS_OUTPUT_DEVIATION_MIN <= data) && (data <= RSU_GNSS_OUTPUT_DEVIATION_MAX)){
      *data_out = data;
      return RSEMIB_OK; 
  } else {
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } 
}
int32_t preserve_rsuGnssMaxDeviation(void)
{
  if(RSEMIB_OK == ntcip1218_lock()){
      prior_ntcip_1218_mib.rsuGnssMaxDeviation = ntcip_1218_mib.rsuGnssMaxDeviation;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t action_rsuGnssMaxDeviation(int32_t data)
{
  if((data < RSU_GNSS_OUTPUT_DEVIATION_MIN) || (RSU_GNSS_OUTPUT_DEVIATION_MAX < data)){ 
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  }
  if(RSEMIB_OK == ntcip1218_lock()){  
      tmp_ntcip_1218_mib.rsuGnssMaxDeviation = data;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t commit_rsuGnssMaxDeviation(void)
{
  int32_t ret = RSEMIB_OK;
  GnssOutput_t gnssOutput;

  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuGnssMaxDeviation = tmp_ntcip_1218_mib.rsuGnssMaxDeviation;
      memset(&gnssOutput,0x0,sizeof(gnssOutput));
      if(RSEMIB_OK == (copy_gnssOutput_to_skinny(&gnssOutput))){
          if(RSEMIB_OK != commit_gnssOutput_to_disk(&gnssOutput)) {
              ret = RSEMIB_BAD_DATA;
          }
      } else {
          ret = RSEMIB_BAD_DATA;
      }
      tmp_ntcip_1218_mib.rsuGnssMaxDeviation = RSU_GNSS_OUTPUT_DEVIATION_MIN;
      ntcip1218_unlock();
  } else {
      ret = RSEMIB_LOCK_FAIL;
  }
  if(ret < RSEMIB_OK)
      set_ntcip_1218_error_states(ret);
  return ret;
}
int32_t undo_rsuGnssMaxDeviation(void)
{
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuGnssMaxDeviation = prior_ntcip_1218_mib.rsuGnssMaxDeviation;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}

/* 5.7.10 */
int32_t get_rsuLocationDeviation(int32_t * data_out)
{
  int32_t data = RSU_GNSS_OUTPUT_DEVIATION_UNKNOWN;
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      data = ntcip_1218_mib.rsuLocationDeviation;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  } 
  if((RSU_GNSS_OUTPUT_DEVIATION_MIN <= data) && (data <= RSU_GNSS_OUTPUT_DEVIATION_UNKNOWN)){
      *data_out = data;
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } 
}
/* 5.7.11 */
int32_t get_rsuGnssPositionError(int32_t * data_out)
{
  int32_t data = RSU_GNSS_OUTPUT_POS_ERROR_UNKNOWN;
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      data = ntcip_1218_mib.rsuGnssPositionError;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  } 
  if((RSU_GNSS_OUTPUT_POS_ERROR_MIN <= data) && (data <= RSU_GNSS_OUTPUT_POS_ERROR_MAX)){
      *data_out = data;
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } 
}

/******************************************************************************
 * 5.8 Interface Log: { rsu 7 }: rsuInterfaceLogTable.
 ******************************************************************************/

/* 5.8.1 */
int32_t get_maxRsuInterfaceLogs(void)
{
  int32_t data = RSU_IFCLOG_MAX;
  if(RSEMIB_OK == ntcip1218_lock()){
      data = ntcip_1218_mib.maxRsuInterfaceLogs;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  } 
  if((RSU_IFCLOG_MIN <= data) && (data <= RSU_IFCLOG_MAX)){
      return data; 
  } else {
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } 
}

/* 5.8.2.1 */
int32_t  get_rsuIfaceLogIndex(int32_t index, int32_t * data_out)
{
  if((index < RSU_IFCLOG_MIN) || (RSU_IFCLOG_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      *data_out = ntcip_1218_mib.rsuInterfaceLogTable[index-1].rsuIfaceLogIndex;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
  if((*data_out < RSU_IFCLOG_MIN) || (RSU_IFCLOG_MAX < *data_out)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } 
  return RSEMIB_OK;
}

/* 5.8.2.2 */
int32_t get_rsuIfaceGenerate(int32_t index, int32_t * data_out)
{
  if((index < RSU_IFCLOG_MIN) || (RSU_IFCLOG_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      *data_out = ntcip_1218_mib.rsuInterfaceLogTable[index-1].rsuIfaceGenerate;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
  if((*data_out < mib_off) || (mib_on < *data_out)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } 
  return RSEMIB_OK;

}
int32_t preserve_rsuIfaceGenerate(int32_t index)
{
  if((index < RSU_IFCLOG_MIN) || (RSU_IFCLOG_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      prior_ntcip_1218_mib.rsuInterfaceLogTable[index-1].rsuIfaceGenerate = 
          ntcip_1218_mib.rsuInterfaceLogTable[index-1].rsuIfaceGenerate;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t action_rsuIfaceGenerate(int32_t index, int32_t data_in)
{
  if((index < RSU_IFCLOG_MIN) || (RSU_IFCLOG_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if((data_in < mib_off) || (mib_on < data_in)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  }
  if(RSEMIB_OK == ntcip1218_lock()){  
      tmp_ntcip_1218_mib.rsuInterfaceLogTable[index-1].rsuIfaceGenerate = (onOff_e)data_in;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t commit_rsuIfaceGenerate(int32_t index)
{
  int32_t ret = RSEMIB_OK;
  if((index < RSU_IFCLOG_MIN) || (RSU_IFCLOG_MAX < index)){
      ret = RSEMIB_BAD_INDEX;
  } else {
      if(RSEMIB_OK == ntcip1218_lock()){
          ntcip_1218_mib.rsuInterfaceLogTable[index-1].rsuIfaceGenerate = tmp_ntcip_1218_mib.rsuInterfaceLogTable[index-1].rsuIfaceGenerate;
          tmp_ntcip_1218_mib.rsuInterfaceLogTable[index-1].rsuIfaceGenerate = mib_off;
          if(RSEMIB_OK == (ret = update_interfaceLog_shm(&ntcip_1218_mib.rsuInterfaceLogTable[index-1]))){
              if(RSEMIB_OK != (ret = commit_interfaceLog_to_disk(&ntcip_1218_mib.rsuInterfaceLogTable[index-1]))){
                  DEBUGMSGTL((MY_NAME, "commit_rsuIfaceGenerate: FAIL: index=%d dir=[%s].\n", index, ntcip_1218_mib.rsuInterfaceLogTable[index-1].filePathName));
                  ret = RSEMIB_BAD_DATA;
              }
          } else {
              ret = RSEMIB_BAD_DATA;
          }
          ntcip1218_unlock();
      } else {
          ret = RSEMIB_LOCK_FAIL;
      }
  }
  if(ret < RSEMIB_OK) {
      set_ntcip_1218_error_states(ret);
  }
  return ret;
}
int32_t undo_rsuIfaceGenerate(int32_t index)
{
  if((index < RSU_IFCLOG_MIN) || (RSU_IFCLOG_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuInterfaceLogTable[index-1].rsuIfaceGenerate = prior_ntcip_1218_mib.rsuInterfaceLogTable[index-1].rsuIfaceGenerate;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}

/* 5.8.2.3 */
int32_t get_rsuIfaceMaxFileSize(int32_t index, int32_t * data_out)
{
  if((index < RSU_IFCLOG_MIN) || (RSU_IFCLOG_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      *data_out = ntcip_1218_mib.rsuInterfaceLogTable[index-1].rsuIfaceMaxFileSize;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
  if((*data_out < RSU_IFCLOG_FILE_SIZE_MIN) || (RSU_IFCLOG_FILE_SIZE_MAX < *data_out)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } 
  return RSEMIB_OK;
}
int32_t preserve_rsuIfaceMaxFileSize(int32_t index)
{
  if((index < RSU_IFCLOG_MIN) || (RSU_IFCLOG_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      prior_ntcip_1218_mib.rsuInterfaceLogTable[index-1].rsuIfaceMaxFileSize = 
          ntcip_1218_mib.rsuInterfaceLogTable[index-1].rsuIfaceMaxFileSize;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t action_rsuIfaceMaxFileSize(int32_t index, int32_t data_in)
{
  if((index < RSU_IFCLOG_MIN) || (RSU_IFCLOG_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if((data_in < RSU_IFCLOG_FILE_SIZE_MIN) || (RSU_IFCLOG_FILE_SIZE_MAX < data_in)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  }
  if(RSEMIB_OK == ntcip1218_lock()){  
      tmp_ntcip_1218_mib.rsuInterfaceLogTable[index-1].rsuIfaceMaxFileSize = data_in;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t commit_rsuIfaceMaxFileSize(int32_t index)
{
  int32_t ret = RSEMIB_OK;
  if((index < RSU_IFCLOG_MIN) || (RSU_IFCLOG_MAX < index)){
      ret = RSEMIB_BAD_INDEX;
  } else {
      if(RSEMIB_OK == ntcip1218_lock()){
          ntcip_1218_mib.rsuInterfaceLogTable[index-1].rsuIfaceMaxFileSize  = tmp_ntcip_1218_mib.rsuInterfaceLogTable[index-1].rsuIfaceMaxFileSize;
          tmp_ntcip_1218_mib.rsuInterfaceLogTable[index-1].rsuIfaceMaxFileSize  = RSU_IFCLOG_FILE_SIZE_DEFAULT;
          if(RSEMIB_OK == (ret = update_interfaceLog_shm(&ntcip_1218_mib.rsuInterfaceLogTable[index-1]))){
              if(RSEMIB_OK != (ret = commit_interfaceLog_to_disk(&ntcip_1218_mib.rsuInterfaceLogTable[index-1]))){
                  DEBUGMSGTL((MY_NAME, "commit_rsuIfaceMaxFileSize: FAIL: index=%d dir=[%s].\n", index, ntcip_1218_mib.rsuInterfaceLogTable[index-1].filePathName));
                  ret = RSEMIB_BAD_DATA;
              }
          } else {
              ret = RSEMIB_BAD_DATA;
          }
          ntcip1218_unlock();
      } else {
          ret = RSEMIB_LOCK_FAIL;
      }
  }
  if(ret < RSEMIB_OK) {
      set_ntcip_1218_error_states(ret);
  }
  return ret;
}
int32_t undo_rsuIfaceMaxFileSize(int32_t index)
{
  if((index < RSU_IFCLOG_MIN) || (RSU_IFCLOG_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuInterfaceLogTable[index-1].rsuIfaceMaxFileSize = 
          prior_ntcip_1218_mib.rsuInterfaceLogTable[index-1].rsuIfaceMaxFileSize;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}

/* 5.8.2.4 */
int32_t get_rsuIfaceMaxFileTime(int32_t index, int32_t * data_out)
{
  if((index < RSU_IFCLOG_MIN) || (RSU_IFCLOG_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      *data_out = ntcip_1218_mib.rsuInterfaceLogTable[index-1].rsuIfaceMaxFileTime;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
  if((*data_out < RSU_IFCLOG_FILE_TIME_MIN) || (RSU_IFCLOG_FILE_TIME_MAX < *data_out)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } 
  return RSEMIB_OK;
}
int32_t preserve_rsuIfaceMaxFileTime(int32_t index)
{
  if((index < RSU_IFCLOG_MIN) || (RSU_IFCLOG_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      prior_ntcip_1218_mib.rsuInterfaceLogTable[index-1].rsuIfaceMaxFileTime = 
          ntcip_1218_mib.rsuInterfaceLogTable[index-1].rsuIfaceMaxFileTime;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t action_rsuIfaceMaxFileTime(int32_t index, int32_t data_in)
{
  if((index < RSU_IFCLOG_MIN) || (RSU_IFCLOG_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if((data_in < RSU_IFCLOG_FILE_TIME_MIN) || (RSU_IFCLOG_FILE_TIME_MAX < data_in)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  }
  if(RSEMIB_OK == ntcip1218_lock()){  
      tmp_ntcip_1218_mib.rsuInterfaceLogTable[index-1].rsuIfaceMaxFileTime = data_in;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t commit_rsuIfaceMaxFileTime(int32_t index)
{
  int32_t ret = RSEMIB_OK;
  if((index < RSU_IFCLOG_MIN) || (RSU_IFCLOG_MAX < index)){
      ret = RSEMIB_BAD_INDEX;
  } else {
      if(RSEMIB_OK == ntcip1218_lock()){
          ntcip_1218_mib.rsuInterfaceLogTable[index-1].rsuIfaceMaxFileTime = tmp_ntcip_1218_mib.rsuInterfaceLogTable[index-1].rsuIfaceMaxFileTime;
          tmp_ntcip_1218_mib.rsuInterfaceLogTable[index-1].rsuIfaceMaxFileTime = RSU_IFCLOG_FILE_TIME_DEFAULT;
          if(RSEMIB_OK == (ret = update_interfaceLog_shm(&ntcip_1218_mib.rsuInterfaceLogTable[index-1]))){
              if(RSEMIB_OK != (ret = commit_interfaceLog_to_disk(&ntcip_1218_mib.rsuInterfaceLogTable[index-1]))){
                  DEBUGMSGTL((MY_NAME, "commit_rsuIfaceMaxFileTime: FAIL: index=%d dir=[%s].\n", index, ntcip_1218_mib.rsuInterfaceLogTable[index-1].filePathName));
                  ret = RSEMIB_BAD_DATA;
              }
          } else {
              ret = RSEMIB_BAD_DATA;
          }
          ntcip1218_unlock();
      } else {
          ret = RSEMIB_LOCK_FAIL;
      }
  }
  if(ret < RSEMIB_OK) {
      set_ntcip_1218_error_states(ret);
  }
  return ret;
}
int32_t undo_rsuIfaceMaxFileTime(int32_t index)
{
  if((index < RSU_IFCLOG_MIN) || (RSU_IFCLOG_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuInterfaceLogTable[index-1].rsuIfaceMaxFileTime = 
          prior_ntcip_1218_mib.rsuInterfaceLogTable[index-1].rsuIfaceMaxFileTime;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}

/* 5.8.2.5 */
int32_t get_rsuIfaceLogByDir(int32_t index, int32_t * data_out)
{
  if((index < RSU_IFCLOG_MIN) || (RSU_IFCLOG_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      *data_out = ntcip_1218_mib.rsuInterfaceLogTable[index-1].rsuIfaceLogByDir;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
  if((*data_out < inboundOnly) || (biCombined < *data_out)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } 
  return RSEMIB_OK;
}
int32_t preserve_rsuIfaceLogByDir(int32_t index)
{
  if((index < RSU_IFCLOG_MIN) || (RSU_IFCLOG_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      prior_ntcip_1218_mib.rsuInterfaceLogTable[index-1].rsuIfaceLogByDir = ntcip_1218_mib.rsuInterfaceLogTable[index-1].rsuIfaceLogByDir;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t action_rsuIfaceLogByDir(int32_t index, int32_t data_in)
{
  if((index < RSU_IFCLOG_MIN) || (RSU_IFCLOG_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if((data_in < inboundOnly) || (biCombined < data_in)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  }
  if(RSEMIB_OK == ntcip1218_lock()){  
      tmp_ntcip_1218_mib.rsuInterfaceLogTable[index-1].rsuIfaceLogByDir = (ifclogDir_e)data_in;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t commit_rsuIfaceLogByDir(int32_t index)
{
  int32_t ret = RSEMIB_OK;
  if((index < RSU_IFCLOG_MIN) || (RSU_IFCLOG_MAX < index)){
      ret = RSEMIB_BAD_INDEX;
  } else {
      if(RSEMIB_OK == ntcip1218_lock()){
          ntcip_1218_mib.rsuInterfaceLogTable[index-1].rsuIfaceLogByDir = tmp_ntcip_1218_mib.rsuInterfaceLogTable[index-1].rsuIfaceLogByDir;
          tmp_ntcip_1218_mib.rsuInterfaceLogTable[index-1].rsuIfaceLogByDir = inboundOnly;
          if(RSEMIB_OK == (ret = update_interfaceLog_shm(&ntcip_1218_mib.rsuInterfaceLogTable[index-1]))){
              if(RSEMIB_OK != (ret = commit_interfaceLog_to_disk(&ntcip_1218_mib.rsuInterfaceLogTable[index-1]))){
                  DEBUGMSGTL((MY_NAME, "commit_rsuIfaceLogByDir: FAIL: index=%d dir=[%s].\n", index, ntcip_1218_mib.rsuInterfaceLogTable[index-1].filePathName));
                  ret = RSEMIB_BAD_DATA;
              }
          } else {
              ret = RSEMIB_BAD_DATA;
          }
          ntcip1218_unlock();
      } else {
          ret = RSEMIB_LOCK_FAIL;
      }
  }
  if(ret < RSEMIB_OK) {
      set_ntcip_1218_error_states(ret);
  }
  return ret;
}
int32_t undo_rsuIfaceLogByDir(int32_t index)
{
  if((index < RSU_IFCLOG_MIN) || (RSU_IFCLOG_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuInterfaceLogTable[index-1].rsuIfaceLogByDir = prior_ntcip_1218_mib.rsuInterfaceLogTable[index-1].rsuIfaceLogByDir;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}

/* 5.8.2.6 */
int32_t get_rsuIfaceName(int32_t index, uint8_t * data_out)
{
  int32_t size = 0;
  if((index < RSU_IFCLOG_MIN) || (RSU_IFCLOG_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      memcpy(data_out, ntcip_1218_mib.rsuInterfaceLogTable[index-1].rsuIfaceName, RSU_INTERFACE_NAME_MAX);
      size = ntcip_1218_mib.rsuInterfaceLogTable[index-1].rsuIfaceName_length;
      ntcip1218_unlock();

  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
  if((size < RSU_INTERFACE_NAME_MIN) || (RSU_INTERFACE_NAME_MAX < size)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);  
      return RSEMIB_BAD_DATA;
  }
  return size;
}
int32_t preserve_rsuIfaceName(int32_t index)
{
  if((index < RSU_IFCLOG_MIN) || (RSU_IFCLOG_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      memcpy( prior_ntcip_1218_mib.rsuInterfaceLogTable[index-1].rsuIfaceName
            , ntcip_1218_mib.rsuInterfaceLogTable[index-1].rsuIfaceName, RSU_INTERFACE_NAME_MAX);
      prior_ntcip_1218_mib.rsuInterfaceLogTable[index-1].rsuIfaceName_length =
          ntcip_1218_mib.rsuInterfaceLogTable[index-1].rsuIfaceName_length;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t action_rsuIfaceName(int32_t index, uint8_t * data_in, int32_t length)
{
  if((index < RSU_IFCLOG_MIN) || (RSU_IFCLOG_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  } 
  /* Client can send zero length string of '' */
  if((NULL == data_in) && (0 != length)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(( length < 0) || (RSU_INTERFACE_NAME_MAX < length)){
      return RSEMIB_BAD_INPUT;  
  }
  if(RSEMIB_OK == ntcip1218_lock()){  
      memset(tmp_ntcip_1218_mib.rsuInterfaceLogTable[index-1].rsuIfaceName, 0x0, RSU_INTERFACE_NAME_MAX);
      if(NULL != data_in)
          memcpy(tmp_ntcip_1218_mib.rsuInterfaceLogTable[index-1].rsuIfaceName, data_in, length); /* zero length null a problem? */
      tmp_ntcip_1218_mib.rsuInterfaceLogTable[index-1].rsuIfaceName_length = length;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t commit_rsuIfaceName(int32_t index)
{
  int32_t ret = RSEMIB_OK;
  if((index < RSU_IFCLOG_MIN) || (RSU_IFCLOG_MAX < index)){
      ret = RSEMIB_BAD_INDEX;
  } else {
      if(RSEMIB_OK == ntcip1218_lock()){
          memcpy( ntcip_1218_mib.rsuInterfaceLogTable[index-1].rsuIfaceName
                , tmp_ntcip_1218_mib.rsuInterfaceLogTable[index-1].rsuIfaceName, RSU_INTERFACE_NAME_MAX);
          ntcip_1218_mib.rsuInterfaceLogTable[index-1].rsuIfaceName_length =
              tmp_ntcip_1218_mib.rsuInterfaceLogTable[index-1].rsuIfaceName_length;
          memset(tmp_ntcip_1218_mib.rsuInterfaceLogTable[index-1].rsuIfaceName, 0x0, RSU_INTERFACE_NAME_MAX);
          tmp_ntcip_1218_mib.rsuInterfaceLogTable[index-1].rsuIfaceName_length = 0;
          if(RSEMIB_OK == (ret = update_interfaceLog_shm(&ntcip_1218_mib.rsuInterfaceLogTable[index-1]))){
              if(RSEMIB_OK != (ret = commit_interfaceLog_to_disk(&ntcip_1218_mib.rsuInterfaceLogTable[index-1]))){
                  DEBUGMSGTL((MY_NAME, "commit_rsuIfaceName: FAIL: index=%d dir=[%s].\n", index, ntcip_1218_mib.rsuInterfaceLogTable[index-1].filePathName));
                  ret = RSEMIB_BAD_DATA;
              }
          } else {
              ret = RSEMIB_BAD_DATA;
          }
          ntcip1218_unlock();
      } else {
          ret = RSEMIB_LOCK_FAIL;
      }
  }
  if(ret < RSEMIB_OK) {
      set_ntcip_1218_error_states(ret);
  }
  return ret;
}
int32_t undo_rsuIfaceName(int32_t index)
{
  if((index < RSU_IFCLOG_MIN) || (RSU_IFCLOG_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      memcpy( ntcip_1218_mib.rsuInterfaceLogTable[index-1].rsuIfaceName
            , prior_ntcip_1218_mib.rsuInterfaceLogTable[index-1].rsuIfaceName, RSU_INTERFACE_NAME_MAX);
      ntcip_1218_mib.rsuInterfaceLogTable[index-1].rsuIfaceName_length = 
          prior_ntcip_1218_mib.rsuInterfaceLogTable[index-1].rsuIfaceName_length;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}

/* 5.8.2.7 */
int32_t get_rsuIfaceStoragePath(int32_t index, uint8_t * data_out)
{
  int32_t size = 0;
  if((index < RSU_IFCLOG_MIN) || (RSU_IFCLOG_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      size = ntcip_1218_mib.rsuInterfaceLogTable[index-1].rsuIfaceStoragePath_length;
      memcpy(data_out, ntcip_1218_mib.rsuInterfaceLogTable[index-1].rsuIfaceStoragePath, RSU_IFCLOG_STORE_PATH_MAX);
      ntcip1218_unlock();

  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
  if((size < RSU_IFCLOG_STORE_PATH_MIN) || (RSU_IFCLOG_STORE_PATH_MAX < size)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);  
      return RSEMIB_BAD_DATA;
  }
  return size;
}
int32_t preserve_rsuIfaceStoragePath(int32_t index)
{
  if((index < RSU_IFCLOG_MIN) || (RSU_IFCLOG_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      memcpy( prior_ntcip_1218_mib.rsuInterfaceLogTable[index-1].rsuIfaceStoragePath
            , ntcip_1218_mib.rsuInterfaceLogTable[index-1].rsuIfaceStoragePath, RSU_IFCLOG_STORE_PATH_MAX);
      prior_ntcip_1218_mib.rsuInterfaceLogTable[index-1].rsuIfaceStoragePath_length = 
          ntcip_1218_mib.rsuInterfaceLogTable[index-1].rsuIfaceStoragePath_length;

      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t action_rsuIfaceStoragePath(int32_t index, uint8_t * data_in, int32_t length)
{
  if((index < RSU_IFCLOG_MIN) || (RSU_IFCLOG_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  } 
  if(NULL == data_in){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(( length < RSU_IFCLOG_STORE_PATH_MIN) || (RSU_IFCLOG_STORE_PATH_MAX < length)){
      return RSEMIB_BAD_INPUT;  
  }
  if(RSEMIB_OK == ntcip1218_lock()){  
      memset(tmp_ntcip_1218_mib.rsuInterfaceLogTable[index-1].rsuIfaceStoragePath, 0x0, RSU_IFCLOG_STORE_PATH_MAX);
      memcpy(tmp_ntcip_1218_mib.rsuInterfaceLogTable[index-1].rsuIfaceStoragePath, data_in, length);
      tmp_ntcip_1218_mib.rsuInterfaceLogTable[index-1].rsuIfaceStoragePath_length = length;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t commit_rsuIfaceStoragePath(int32_t index)
{
  int32_t ret = RSEMIB_OK;
  if((index < RSU_IFCLOG_MIN) || (RSU_IFCLOG_MAX < index)){
      ret = RSEMIB_BAD_INDEX;
  } else {
      if(RSEMIB_OK == ntcip1218_lock()){
          memcpy( ntcip_1218_mib.rsuInterfaceLogTable[index-1].rsuIfaceStoragePath
                , tmp_ntcip_1218_mib.rsuInterfaceLogTable[index-1].rsuIfaceStoragePath, RSU_IFCLOG_STORE_PATH_MAX);
          ntcip_1218_mib.rsuInterfaceLogTable[index-1].rsuIfaceStoragePath_length = 
              tmp_ntcip_1218_mib.rsuInterfaceLogTable[index-1].rsuIfaceStoragePath_length;
          memset(tmp_ntcip_1218_mib.rsuInterfaceLogTable[index-1].rsuIfaceStoragePath, 0x0, RSU_IFCLOG_STORE_PATH_MAX);
          tmp_ntcip_1218_mib.rsuInterfaceLogTable[index-1].rsuIfaceStoragePath_length = 0;          

          if(RSEMIB_OK == (ret = update_interfaceLog_shm(&ntcip_1218_mib.rsuInterfaceLogTable[index-1]))){
              if(RSEMIB_OK != (ret = commit_interfaceLog_to_disk(&ntcip_1218_mib.rsuInterfaceLogTable[index-1]))){
                  DEBUGMSGTL((MY_NAME, "rsuIfaceStoragePath: FAIL: index=%d dir=[%s].\n", index, ntcip_1218_mib.rsuInterfaceLogTable[index-1].filePathName));
                  ret = RSEMIB_BAD_DATA;
              }
          } else {
              ret = RSEMIB_BAD_DATA;
          }
          ntcip1218_unlock();
      } else {
          ret = RSEMIB_LOCK_FAIL;
      }
  }
  if(ret < RSEMIB_OK) {
      set_ntcip_1218_error_states(ret);
  }
  return ret;
}
int32_t undo_rsuIfaceStoragePath(int32_t index)
{
  if((index < RSU_IFCLOG_MIN) || (RSU_IFCLOG_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      memcpy( ntcip_1218_mib.rsuInterfaceLogTable[index-1].rsuIfaceStoragePath
            , prior_ntcip_1218_mib.rsuInterfaceLogTable[index-1].rsuIfaceStoragePath, RSU_IFCLOG_STORE_PATH_MAX);
      ntcip_1218_mib.rsuInterfaceLogTable[index-1].rsuIfaceStoragePath_length =
          prior_ntcip_1218_mib.rsuInterfaceLogTable[index-1].rsuIfaceStoragePath_length;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}

/* 5.8.2.8 */
int32_t get_rsuIfaceLogName(int32_t index, uint8_t * data_out)
{
  int32_t size = 0;
  if((index < RSU_IFCLOG_MIN) || (RSU_IFCLOG_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      size = ntcip_1218_mib.rsuInterfaceLogTable[index-1].rsuIfaceLogName_length;
      memcpy(data_out, ntcip_1218_mib.rsuInterfaceLogTable[index-1].rsuIfaceLogName, RSU_IFCLOG_NAME_SIZE_MAX);
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
  if((size < RSU_IFCLOG_NAME_SIZE_MIN) || (RSU_IFCLOG_NAME_SIZE_MAX < size)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);  
      return RSEMIB_BAD_DATA;
  }
  return size;
}
int32_t preserve_rsuIfaceLogName(int32_t index)
{
  if((index < RSU_IFCLOG_MIN) || (RSU_IFCLOG_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      memcpy( prior_ntcip_1218_mib.rsuInterfaceLogTable[index-1].rsuIfaceLogName
            , ntcip_1218_mib.rsuInterfaceLogTable[index-1].rsuIfaceLogName, RSU_IFCLOG_NAME_SIZE_MAX);
      prior_ntcip_1218_mib.rsuInterfaceLogTable[index-1].rsuIfaceLogName_length =
          ntcip_1218_mib.rsuInterfaceLogTable[index-1].rsuIfaceLogName_length;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t action_rsuIfaceLogName(int32_t index, uint8_t * data_in, int32_t length)
{
  if((index < RSU_IFCLOG_MIN) || (RSU_IFCLOG_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  } 
  if(NULL == data_in){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(( length < RSU_IFCLOG_NAME_SIZE_MIN) || (RSU_IFCLOG_NAME_SIZE_MAX < length)){
      return RSEMIB_BAD_INPUT;  
  }
  if(RSEMIB_OK == ntcip1218_lock()){  
      memset(tmp_ntcip_1218_mib.rsuInterfaceLogTable[index-1].rsuIfaceLogName,0x0,RSU_IFCLOG_NAME_SIZE_MAX);
      memcpy(tmp_ntcip_1218_mib.rsuInterfaceLogTable[index-1].rsuIfaceLogName, data_in, length);
      tmp_ntcip_1218_mib.rsuInterfaceLogTable[index-1].rsuIfaceLogName_length = length;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t commit_rsuIfaceLogName(int32_t index)
{
  int32_t ret = RSEMIB_OK;
  if((index < RSU_IFCLOG_MIN) || (RSU_IFCLOG_MAX < index)){
      ret = RSEMIB_BAD_INDEX;
  } else {
      if(RSEMIB_OK == ntcip1218_lock()){
          memcpy( ntcip_1218_mib.rsuInterfaceLogTable[index-1].rsuIfaceLogName
              , tmp_ntcip_1218_mib.rsuInterfaceLogTable[index-1].rsuIfaceLogName, RSU_IFCLOG_NAME_SIZE_MAX);
          ntcip_1218_mib.rsuInterfaceLogTable[index-1].rsuIfaceLogName_length =
              tmp_ntcip_1218_mib.rsuInterfaceLogTable[index-1].rsuIfaceLogName_length;
          memset(tmp_ntcip_1218_mib.rsuInterfaceLogTable[index-1].rsuIfaceLogName, 0x0, RSU_IFCLOG_NAME_SIZE_MAX);
          tmp_ntcip_1218_mib.rsuInterfaceLogTable[index-1].rsuIfaceLogName_length = 0;
          if(RSEMIB_OK == (ret = update_interfaceLog_shm(&ntcip_1218_mib.rsuInterfaceLogTable[index-1]))){
              if(RSEMIB_OK != (ret = commit_interfaceLog_to_disk(&ntcip_1218_mib.rsuInterfaceLogTable[index-1]))){
                  DEBUGMSGTL((MY_NAME, "commit_rsuIfaceLogName: FAIL: index=%d dir=[%s].\n", index, ntcip_1218_mib.rsuInterfaceLogTable[index-1].filePathName));
                  ret = RSEMIB_BAD_DATA;
              }
          } else {
              ret = RSEMIB_BAD_DATA;
          }
          ntcip1218_unlock();
      } else {
          ret = RSEMIB_LOCK_FAIL;
      }
  }
  if(ret < RSEMIB_OK) {
      set_ntcip_1218_error_states(ret);
  }
  return ret;
}
int32_t undo_rsuIfaceLogName(int32_t index)
{
  if((index < RSU_IFCLOG_MIN) || (RSU_IFCLOG_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      memcpy( ntcip_1218_mib.rsuInterfaceLogTable[index-1].rsuIfaceLogName
            , prior_ntcip_1218_mib.rsuInterfaceLogTable[index-1].rsuIfaceLogName, RSU_IFCLOG_NAME_SIZE_MAX);
      ntcip_1218_mib.rsuInterfaceLogTable[index-1].rsuIfaceLogName_length =
          prior_ntcip_1218_mib.rsuInterfaceLogTable[index-1].rsuIfaceLogName_length;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}

/* 5.8.2.9 */
int32_t get_rsuIfaceLogStart(int32_t index, uint8_t * data_out)
{
  if((index < RSU_IFCLOG_MIN) || (RSU_IFCLOG_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      memcpy(data_out, ntcip_1218_mib.rsuInterfaceLogTable[index-1].rsuIfaceLogStart, MIB_DATEANDTIME_LENGTH);
      ntcip1218_unlock();
      return MIB_DATEANDTIME_LENGTH;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t preserve_rsuIfaceLogStart(int32_t index)
{
  if((index < RSU_IFCLOG_MIN) || (RSU_IFCLOG_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      memcpy( prior_ntcip_1218_mib.rsuInterfaceLogTable[index-1].rsuIfaceLogStart
            , ntcip_1218_mib.rsuInterfaceLogTable[index-1].rsuIfaceLogStart, MIB_DATEANDTIME_LENGTH);
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t action_rsuIfaceLogStart(int32_t index, uint8_t * data_in, int32_t length)
{
  uint64_t start_utc_dsec = 0;
  if((index < RSU_IFCLOG_MIN) || (RSU_IFCLOG_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(NULL == data_in){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(MIB_DATEANDTIME_LENGTH != length){
      return RSEMIB_BAD_INPUT;  
  }
  if(RSEMIB_OK != DateAndTime_To_UTC_DSEC(data_in,&start_utc_dsec)){
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      memset(tmp_ntcip_1218_mib.rsuInterfaceLogTable[index-1].rsuIfaceLogStart, 0x0, MIB_DATEANDTIME_LENGTH);
      memcpy(tmp_ntcip_1218_mib.rsuInterfaceLogTable[index-1].rsuIfaceLogStart, data_in, length);
      tmp_ntcip_1218_mib.rsuInterfaceLogTable[index-1].start_utc_dsec = start_utc_dsec;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t commit_rsuIfaceLogStart(int32_t index)
{
  int32_t ret = RSEMIB_OK;
  if((index < RSU_IFCLOG_MIN) || (RSU_IFCLOG_MAX < index)){
      ret = RSEMIB_BAD_INDEX;
  } else {
      if(RSEMIB_OK == ntcip1218_lock()){
          memcpy( ntcip_1218_mib.rsuInterfaceLogTable[index-1].rsuIfaceLogStart
                , tmp_ntcip_1218_mib.rsuInterfaceLogTable[index-1].rsuIfaceLogStart, MIB_DATEANDTIME_LENGTH);
          ntcip_1218_mib.rsuInterfaceLogTable[index-1].start_utc_dsec = tmp_ntcip_1218_mib.rsuInterfaceLogTable[index-1].start_utc_dsec;
          tmp_ntcip_1218_mib.rsuInterfaceLogTable[index-1].start_utc_dsec = 0;
          memset(tmp_ntcip_1218_mib.rsuInterfaceLogTable[index-1].rsuIfaceLogStart, 0x0, MIB_DATEANDTIME_LENGTH);
          if(RSEMIB_OK == (ret = update_interfaceLog_shm(&ntcip_1218_mib.rsuInterfaceLogTable[index-1]))){
              if(RSEMIB_OK != (ret = commit_interfaceLog_to_disk(&ntcip_1218_mib.rsuInterfaceLogTable[index-1]))){
                  DEBUGMSGTL((MY_NAME, "rsuIfaceLogStart: FAIL: index=%d dir=[%s].\n", index, ntcip_1218_mib.rsuInterfaceLogTable[index-1].filePathName));
                  ret = RSEMIB_BAD_DATA;
              }
          } else {
              ret = RSEMIB_BAD_DATA;
          }
          ntcip1218_unlock();
      } else {
          ret = RSEMIB_LOCK_FAIL;
      }
  }
  if(ret < RSEMIB_OK) {
      set_ntcip_1218_error_states(ret);
  }
  return ret;
}
int32_t undo_rsuIfaceLogStart(int32_t index)
{
  if((index < RSU_IFCLOG_MIN) || (RSU_IFCLOG_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      memcpy( ntcip_1218_mib.rsuInterfaceLogTable[index-1].rsuIfaceLogStart
            , prior_ntcip_1218_mib.rsuInterfaceLogTable[index-1].rsuIfaceLogStart, MIB_DATEANDTIME_LENGTH);
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}

/* 5.8.2.10 */
int32_t get_rsuIfaceLogStop(int32_t index, uint8_t * data_out)
{
  if((index < RSU_IFCLOG_MIN) || (RSU_IFCLOG_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      memcpy(data_out, ntcip_1218_mib.rsuInterfaceLogTable[index-1].rsuIfaceLogStop, MIB_DATEANDTIME_LENGTH);
      ntcip1218_unlock();
      return MIB_DATEANDTIME_LENGTH;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t preserve_rsuIfaceLogStop(int32_t index)
{
  if((index < RSU_IFCLOG_MIN) || (RSU_IFCLOG_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      memcpy( prior_ntcip_1218_mib.rsuInterfaceLogTable[index-1].rsuIfaceLogStop
            , ntcip_1218_mib.rsuInterfaceLogTable[index-1].rsuIfaceLogStop, MIB_DATEANDTIME_LENGTH);
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t action_rsuIfaceLogStop(int32_t index, uint8_t * data_in, int32_t length)
{
  uint64_t stop_utc_dsec = 0;
  if((index < RSU_IFCLOG_MIN) || (RSU_IFCLOG_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(NULL == data_in){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(MIB_DATEANDTIME_LENGTH != length){
      return RSEMIB_BAD_INPUT;  
  }
  if(RSEMIB_OK != DateAndTime_To_UTC_DSEC(data_in,&stop_utc_dsec)){
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      memset(tmp_ntcip_1218_mib.rsuInterfaceLogTable[index-1].rsuIfaceLogStop, 0x0, MIB_DATEANDTIME_LENGTH);
      memcpy(tmp_ntcip_1218_mib.rsuInterfaceLogTable[index-1].rsuIfaceLogStop, data_in, length);
      tmp_ntcip_1218_mib.rsuInterfaceLogTable[index-1].stop_utc_dsec = stop_utc_dsec;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t commit_rsuIfaceLogStop(int32_t index)
{
  int32_t ret = RSEMIB_OK;
  if((index < RSU_IFCLOG_MIN) || (RSU_IFCLOG_MAX < index)){
      ret = RSEMIB_BAD_INDEX;
  } else {
      if(RSEMIB_OK == ntcip1218_lock()){
          memcpy( ntcip_1218_mib.rsuInterfaceLogTable[index-1].rsuIfaceLogStop
                , tmp_ntcip_1218_mib.rsuInterfaceLogTable[index-1].rsuIfaceLogStop, MIB_DATEANDTIME_LENGTH);
          ntcip_1218_mib.rsuInterfaceLogTable[index-1].stop_utc_dsec = tmp_ntcip_1218_mib.rsuInterfaceLogTable[index-1].stop_utc_dsec;
          tmp_ntcip_1218_mib.rsuInterfaceLogTable[index-1].stop_utc_dsec = 0;
          memset(tmp_ntcip_1218_mib.rsuInterfaceLogTable[index-1].rsuIfaceLogStop, 0x0, MIB_DATEANDTIME_LENGTH);
          if(RSEMIB_OK == (ret = update_interfaceLog_shm(&ntcip_1218_mib.rsuInterfaceLogTable[index-1]))){
              if(RSEMIB_OK != (ret = commit_interfaceLog_to_disk(&ntcip_1218_mib.rsuInterfaceLogTable[index-1]))){
                  DEBUGMSGTL((MY_NAME, "rsuIfaceLogStop: FAIL: index=%d dir=[%s].\n", index, ntcip_1218_mib.rsuInterfaceLogTable[index-1].filePathName));
                  ret = RSEMIB_BAD_DATA;
              }
          } else {
              ret = RSEMIB_BAD_DATA;
          }
          ntcip1218_unlock();
      } else {
          ret = RSEMIB_LOCK_FAIL;
      }
  }
  if(ret < RSEMIB_OK) {
      set_ntcip_1218_error_states(ret);
  }
  return ret;
}
int32_t undo_rsuIfaceLogStop(int32_t index)
{
  if((index < RSU_IFCLOG_MIN) || (RSU_IFCLOG_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      memcpy( ntcip_1218_mib.rsuInterfaceLogTable[index-1].rsuIfaceLogStop
            , prior_ntcip_1218_mib.rsuInterfaceLogTable[index-1].rsuIfaceLogStop, MIB_DATEANDTIME_LENGTH);
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}

/* 5.8.2.11 */
int32_t get_rsuIfaceLogOptions(int32_t index, uint8_t * data_out)
{
  if((index < RSU_IFCLOG_MIN) || (RSU_IFCLOG_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      *data_out = ntcip_1218_mib.rsuInterfaceLogTable[index-1].rsuIfaceLogOptions;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
  return RSU_IFCLOG_OPTIONS_LENGTH;
}
int32_t preserve_rsuIfaceLogOptions(int32_t index)
{
  if((index < RSU_IFCLOG_MIN) || (RSU_IFCLOG_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      prior_ntcip_1218_mib.rsuInterfaceLogTable[index-1].rsuIfaceLogOptions = 
          ntcip_1218_mib.rsuInterfaceLogTable[index-1].rsuIfaceLogOptions;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t action_rsuIfaceLogOptions(int32_t index, uint8_t * data_in, int32_t length)
{
  if((index < RSU_IFCLOG_MIN) || (RSU_IFCLOG_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if((NULL == data_in) || (RSU_IFCLOG_OPTIONS_LENGTH != length)){
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){  
      tmp_ntcip_1218_mib.rsuInterfaceLogTable[index-1].rsuIfaceLogOptions = * data_in;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t commit_rsuIfaceLogOptions(int32_t index)
{
  int32_t ret = RSEMIB_OK;
  if((index < RSU_IFCLOG_MIN) || (RSU_IFCLOG_MAX < index)){
      ret = RSEMIB_BAD_INDEX;
  } else {
      if(RSEMIB_OK == ntcip1218_lock()){
          ntcip_1218_mib.rsuInterfaceLogTable[index-1].rsuIfaceLogOptions = tmp_ntcip_1218_mib.rsuInterfaceLogTable[index-1].rsuIfaceLogOptions;
          tmp_ntcip_1218_mib.rsuInterfaceLogTable[index-1].rsuIfaceLogOptions = 0x0;
          if(RSEMIB_OK == (ret = update_interfaceLog_shm(&ntcip_1218_mib.rsuInterfaceLogTable[index-1]))){
              if(RSEMIB_OK != (ret = commit_interfaceLog_to_disk(&ntcip_1218_mib.rsuInterfaceLogTable[index-1]))){
                  DEBUGMSGTL((MY_NAME, "commit_rsuIfaceLogOptions: FAIL: index=%d dir=[%s].\n", index, ntcip_1218_mib.rsuInterfaceLogTable[index-1].filePathName));
                  ret = RSEMIB_BAD_DATA;
              }
          } else {
              ret = RSEMIB_BAD_DATA;
          }
          ntcip1218_unlock();
      } else {
          ret = RSEMIB_LOCK_FAIL;
      }
  }
  if(ret < RSEMIB_OK) {
      set_ntcip_1218_error_states(ret);
  }
  return ret;
}
int32_t undo_rsuIfaceLogOptions(int32_t index)
{
  if((index < RSU_IFCLOG_MIN) || (RSU_IFCLOG_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuInterfaceLogTable[index-1].rsuIfaceLogOptions = 
          prior_ntcip_1218_mib.rsuInterfaceLogTable[index-1].rsuIfaceLogOptions;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}

/* 5.8.2.12 */
int32_t get_rsuIfaceLogStatus(int32_t index, int32_t * data_out)
{
  if((index < RSU_IFCLOG_MIN) || (RSU_IFCLOG_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      *data_out = ntcip_1218_mib.rsuInterfaceLogTable[index-1].rsuIfaceLogStatus;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  } 
  if((ROW_STATUS_VALID_MIN <= *data_out ) && (*data_out <= ROW_STATUS_VALID_MAX)){
      return RSEMIB_OK; 
  } else {
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } 
}
int32_t preserve_rsuIfaceLogStatus(int32_t index)
{
  if((index < RSU_IFCLOG_MIN) || (RSU_IFCLOG_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      set_default_row_interfaceLog(&tmp_ntcip_1218_mib.rsuInterfaceLogTable[index-1],index-1);
      prior_ntcip_1218_mib.rsuInterfaceLogTable[index-1].rsuIfaceLogStatus = ntcip_1218_mib.rsuInterfaceLogTable[index-1].rsuIfaceLogStatus;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t action_rsuIfaceLogStatus(int32_t index, int32_t data_in)
{
  if((index < RSU_IFCLOG_MIN) || (RSU_IFCLOG_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if((data_in < ROW_STATUS_VALID_MIN) || (ROW_STATUS_VALID_MAX < data_in)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  }
  if(RSEMIB_OK == ntcip1218_lock()){  
      tmp_ntcip_1218_mib.rsuInterfaceLogTable[index-1].rsuIfaceLogStatus = data_in;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t commit_rsuIfaceLogStatus(int32_t index)
{
  int32_t ret = RSEMIB_OK;
  char_t  command_buffer[100 + RSU_MSG_FILENAME_LENGTH_MAX]; /* If you need more than a 100 chars for your command then add them here. */

  if((index < RSU_IFCLOG_MIN) || (RSU_IFCLOG_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      ret = RSEMIB_BAD_INDEX;
  }
  if((RSEMIB_OK == ret) && (RSEMIB_OK == ntcip1218_lock())){
      DEBUGMSGTL((MY_NAME_EXTRA, "commit_rsuIfaceLogStatus: index=%d status=%d.\n", index,tmp_ntcip_1218_mib.rsuInterfaceLogTable[index-1].rsuIfaceLogStatus));
      switch(tmp_ntcip_1218_mib.rsuInterfaceLogTable[index-1].rsuIfaceLogStatus){
          case SNMP_ROW_CREATEANDGO:
              if(SNMP_ROW_NONEXISTENT == ntcip_1218_mib.rsuInterfaceLogTable[index-1].rsuIfaceLogStatus) {
                  set_default_row_interfaceLog(&ntcip_1218_mib.rsuInterfaceLogTable[index-1],index-1);
                  ntcip_1218_mib.rsuInterfaceLogTable[index-1].rsuIfaceLogStatus = SNMP_ROW_ACTIVE;
                  if(RSEMIB_OK == (ret = update_interfaceLog_shm(&ntcip_1218_mib.rsuInterfaceLogTable[index-1]))){
                      if(RSEMIB_OK != commit_interfaceLog_to_disk(&ntcip_1218_mib.rsuInterfaceLogTable[index-1])){
                          DEBUGMSGTL((MY_NAME, "commit_interfaceLog_to_disk: FAIL: index=%d dir=[%s].\n", index, ntcip_1218_mib.rsuInterfaceLogTable[index-1].filePathName));
                          ret = RSEMIB_BAD_DATA;
                      } else {
                          /* Rebuild helper table before ack. */
                          if (RSEMIB_OK != (ret = rebuild_interfaceLog_live(&ntcip_1218_mib.rsuInterfaceLogTable[0]))){
                              ret = RSEMIB_BAD_DATA;
                          }
                      }
                  } else {
                      ret = RSEMIB_BAD_DATA;
                  }
              } else {
                  /* If it exists already then do nothing. Dont send error back. */
                  //ret = RSEMIB_BAD_MODE;
              }
              break;
          case SNMP_ROW_CREATEANDWAIT:
              if(SNMP_ROW_NONEXISTENT == ntcip_1218_mib.rsuInterfaceLogTable[index-1].rsuIfaceLogStatus) {
                  set_default_row_interfaceLog(&ntcip_1218_mib.rsuInterfaceLogTable[index-1],index-1);
                  ntcip_1218_mib.rsuInterfaceLogTable[index-1].rsuIfaceLogStatus = SNMP_ROW_NOTINSERVICE;
                  if(RSEMIB_OK == (ret = update_interfaceLog_shm(&ntcip_1218_mib.rsuInterfaceLogTable[index-1]))){
                      if(RSEMIB_OK != commit_interfaceLog_to_disk(&ntcip_1218_mib.rsuInterfaceLogTable[index-1])){
                          DEBUGMSGTL((MY_NAME, "commit_interfaceLog_to_disk: FAIL: index=%d dir=[%s].\n", index, ntcip_1218_mib.rsuInterfaceLogTable[index-1].filePathName));
                          ret = RSEMIB_BAD_DATA;
                      } else {
                          /* Rebuild helper table before ack. */
                          if (RSEMIB_OK != (ret = rebuild_interfaceLog_live(&ntcip_1218_mib.rsuInterfaceLogTable[0]))){
                              ret = RSEMIB_BAD_DATA;
                          }
                      }
                  } else {
                      ret = RSEMIB_BAD_DATA;
                  }
              } else {
                  /* If it exists already then do nothing. Dont send error back. */
                  //ret = RSEMIB_BAD_MODE;
              }
              break;
          case SNMP_ROW_DESTROY: /* User deletes row from table. */
              if(SNMP_ROW_NONEXISTENT == ntcip_1218_mib.rsuInterfaceLogTable[index-1].rsuIfaceLogStatus){
                  ret = RSEMIB_OK; /* If you try to DESTROY non-exixtent row that's an easy one. */
              } else {
                  /* rm from RSU. */
                  memset(command_buffer,'\0',sizeof(command_buffer));
                  snprintf(command_buffer, sizeof(command_buffer), "/bin/rm -f %s", ntcip_1218_mib.rsuInterfaceLogTable[index-1].filePathName);
                  DEBUGMSGTL((MY_NAME, "DESTROY: index=%d cmd=[%s].\n", index, command_buffer));
                  if(0 != system(command_buffer)){
                      ret = RSEMIB_BAD_DATA;
                  }
//TODO: We only have one row today in table so deleting everything is correct. 
//TODO: If we add more rows then need more smarts like matching interfacename.
                  /* Now delete contents of IFC_COMPLETED_LOGS_DIR if bit 1 of options set */
                  if(RSU_IFCLOG_OPTIONS_DELETE_ENTRY & ntcip_1218_mib.rsuInterfaceLogTable[index-1].rsuIfaceLogOptions) {
                      memset(command_buffer,'\0',sizeof(command_buffer));
                      snprintf(command_buffer, sizeof(command_buffer), "/bin/rm -f %s/*", IFC_COMPLETED_LOGS_DIR);
                      DEBUGMSGTL((MY_NAME, "DESTROY: remove logs: cmd=[%s].\n", command_buffer));
                      //I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"DESTROY: remove logs: cmd=[%s].\n", command_buffer)); 
                      if(0 != system(command_buffer)){
                          ret = RSEMIB_BAD_DATA;
                      }
                  }
                  /* Careful to preserve index within table rows. Only call proper routine to clear. */
                  set_default_row_interfaceLog(&ntcip_1218_mib.rsuInterfaceLogTable[index-1],index-1);
                  set_default_row_interfaceLog(&tmp_ntcip_1218_mib.rsuInterfaceLogTable[index-1],index-1);
                  set_default_row_interfaceLog(&prior_ntcip_1218_mib.rsuInterfaceLogTable[index-1],index-1);
                  if(RSEMIB_OK == (ret = update_interfaceLog_shm(&ntcip_1218_mib.rsuInterfaceLogTable[index-1]))){
                      /* Rebuild helper table before ack. */
                      if (RSEMIB_OK != (ret = rebuild_interfaceLog_live(&ntcip_1218_mib.rsuInterfaceLogTable[0]))){
                          ret = RSEMIB_BAD_DATA;
                      }
                  } else {
                      ret = RSEMIB_BAD_DATA;
                  }
              }
              break;
          case SNMP_ROW_ACTIVE:
              ntcip_1218_mib.rsuInterfaceLogTable[index-1].rsuIfaceLogStatus = SNMP_ROW_ACTIVE;
              break;
          case SNMP_ROW_NOTINSERVICE:
              ntcip_1218_mib.rsuInterfaceLogTable[index-1].rsuIfaceLogStatus = SNMP_ROW_NOTINSERVICE;
              break;
          case SNMP_ROW_NONEXISTENT:
          case SNMP_ROW_NOTREADY:
          default:
              ret = RSEMIB_BAD_MODE;
              break;
      }
      /* Clear tmp. */
      tmp_ntcip_1218_mib.rsuInterfaceLogTable[index-1].rsuIfaceLogStatus = ROW_STATUS_VALID_DEFAULT;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      ret = RSEMIB_LOCK_FAIL;
  }
  return ret;
}
int32_t undo_rsuIfaceLogStatus(int32_t index)
{
  if((index < RSU_IFCLOG_MIN) || (RSU_IFCLOG_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuInterfaceLogTable[index-1].rsuIfaceLogStatus = prior_ntcip_1218_mib.rsuInterfaceLogTable[index-1].rsuIfaceLogStatus;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}

/******************************************************************************
 * 5.9 Security: {rsu 8} : rsuSecAppCertTable and rsuSecProfileTable
 ******************************************************************************/

/* 5.9.1: 0..8736: default = 168 */
int32_t get_rsuSecCredReq(void)
{
  int32_t data = 0;
  if(RSEMIB_OK == ntcip1218_lock()){
      data = ntcip_1218_mib.rsuSecCredReq;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  } 
  if((RSU_SECURITY_CRED_REQ_HOURS_MIN <= data) && (data <= RSU_SECURITY_CRED_REQ_HOURS_MAX)){
      return data; 
  } else {
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } 
}
int32_t preserve_rsuSecCredReq(void)
{
  if(RSEMIB_OK == ntcip1218_lock()){
      prior_ntcip_1218_mib.rsuSecCredReq = ntcip_1218_mib.rsuSecCredReq;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t action_rsuSecCredReq(int32_t data)
{
  if((data < RSU_SECURITY_CRED_REQ_HOURS_MIN ) || (RSU_SECURITY_CRED_REQ_HOURS_MAX < data)){ 
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  }
  if(RSEMIB_OK == ntcip1218_lock()){  
      tmp_ntcip_1218_mib.rsuSecCredReq = data;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t commit_rsuSecCredReq(void)
{
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuSecCredReq     = tmp_ntcip_1218_mib.rsuSecCredReq; 
      tmp_ntcip_1218_mib.rsuSecCredReq = RSU_SECURITY_CRED_REQ_HOURS_DEFAULT;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t undo_rsuSecCredReq(void)
{
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuSecCredReq = prior_ntcip_1218_mib.rsuSecCredReq;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}

/* 5.9.2. */
int32_t get_rsuSecEnrollCertStatus(void)
{
  rsuSecEnrollCertStatus_e data = enroll_cert_status_unknown;

  if(RSEMIB_OK == ntcip1218_lock()){
      data = ntcip_1218_mib.rsuSecEnrollCertStatus;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  } 
  if((enroll_cert_status_other <= data) && (data <= enroll_cert_status_enrolled)){
      return (int32_t)data; 
  } else {
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } 
}

/* 5.9.3. */
int32_t get_rsuSecEnrollCertValidRegion(void)
{
  int32_t data = RSU_SECURITY_REGION_USA;
  if(RSEMIB_OK == ntcip1218_lock()){
      data = ntcip_1218_mib.rsuSecEnrollCertValidRegion;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  } 
  if((0 <= data) && (data <= 65535)){
      return data; 
  } else {
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } 
}

/* 5.9.4. */
int32_t get_rsuSecEnrollCertUrl(uint8_t * data_out)
{
  int32_t size = 0;
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      size = ntcip_1218_mib.rsuSecEnrollCertUrl_length;
      memcpy(data_out,ntcip_1218_mib.rsuSecEnrollCertUrl,URI255_LENGTH_MAX);
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
  if((size < URI255_LENGTH_MIN) || (URI255_LENGTH_MAX < size)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_DATA;
  } 
  return size;
}

/* 5.9.5. */
int32_t get_rsuSecEnrollCertId(uint8_t * data_out)
{
  int32_t size = 0;
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      size = ntcip_1218_mib.rsuSecEnrollCertId_length;
      memcpy(data_out,ntcip_1218_mib.rsuSecEnrollCertId,RSU_SECURITY_ENROLL_CERT_ID_LENGTH_MAX);
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
  if((size < RSU_SECURITY_ENROLL_CERT_ID_LENGTH_MIN) || (RSU_SECURITY_ENROLL_CERT_ID_LENGTH_MAX < size)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_DATA;
  } 
  return size;
}

/* 5.9.6. */
int32_t get_rsuSecEnrollCertExpiration(uint8_t * data_out)
{
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      memcpy(data_out, ntcip_1218_mib.rsuSecEnrollCertExpiration, MIB_DATEANDTIME_LENGTH);
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}

/* 5.9.7: 1..3. */
int32_t get_rsuSecuritySource(void)
{
  rsuSecuritySource_e data = security_source_scms;

  if(RSEMIB_OK == ntcip1218_lock()){
      data = ntcip_1218_mib.rsuSecuritySource;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  } 
  if((security_source_other <= data) && (data <= security_source_manual)){
      return (int32_t)data; 
  } else {
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } 
}

/* 5.9.8: 0..URI1024_LENGTH_MAX */
int32_t get_rsuSecAppCertUrl(uint8_t * data_out)
{
  int32_t size = 0;
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      size = ntcip_1218_mib.rsuSecAppCertUrl_length;
      memcpy(data_out, ntcip_1218_mib.rsuSecAppCertUrl, URI1024_LENGTH_MAX);
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
  if((size < URI1024_LENGTH_MIN) || (URI1024_LENGTH_MAX < size)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_DATA;
  } 
  return size;
}

/* 5.9.9: RSU_SEC_APP_CERTS_MAX */
int32_t get_maxRsuSecAppCerts(void)
{
  int32_t data = RSU_SEC_APP_CERTS_MAX;
  if(RSEMIB_OK == ntcip1218_lock()){
      data = ntcip_1218_mib.maxRsuSecAppCerts;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  } 
  if((RSU_SEC_APP_CERTS_MIN <= data) && (data <= RSU_SEC_APP_CERTS_MAX)){
      return data; 
  } else {
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } 
}

/* 5.9.10.2. */
int32_t get_rsuSecAppCertPsid(int32_t index, uint8_t * data_out)
{
  int32_t size = 0;
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if((index < 1) || (RSU_SEC_APP_CERTS_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      size = ntcip_1218_mib.rsuSecAppCertTable[index-1].rsuSecAppCertPsid_length;
      memcpy(data_out, ntcip_1218_mib.rsuSecAppCertTable[index-1].rsuSecAppCertPsid, RSU_SEC_APP_PSID_MAX);
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
  if((size < RSU_SEC_APP_PSID_MIN) || (RSU_SEC_APP_PSID_MAX < size)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } 
  return size;
}

/* 5.9.10.3 */
int32_t get_rsuSecAppCertState(int32_t index)
{
  int32_t data = 0;
  if((index < 1) || (RSU_SEC_APP_CERTS_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      data = ntcip_1218_mib.rsuSecAppCertTable[index-1].rsuSecAppCertState;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
  if((RSU_SEC_APP_CERT_STATE_MIN <= data) && (data <= RSU_SEC_APP_CERT_STATE_MAX)){
      return data; 
  } else {
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } 
}
/* 5.9.10.4 */
int32_t get_rsuSecAppCertExpiration(int32_t index)
{
  int32_t data = 0;
  if((index < 1) || (RSU_SEC_APP_CERTS_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      data = ntcip_1218_mib.rsuSecAppCertTable[index-1].rsuSecAppCertExpiration;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
  if((RSE_SEC_APP_CERT_EXP_MIN <= data) && (data <= RSE_SEC_APP_CERT_EXP_MAX)){
      return data; 
  } else {
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } 
}
/* 5.9.10.5 */
int32_t get_rsuSecAppCertReq(int32_t index)
{
  int32_t data = 0;
  if((index < 1) || (RSU_SEC_APP_CERTS_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      data = ntcip_1218_mib.rsuSecAppCertTable[index-1].rsuSecAppCertReq;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
  if((RSU_SEC_APP_CERT_REQ_MIN <= data) && (data <= RSU_SEC_APP_CERT_REQ_MAX)){
      return data; 
  } else {
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } 
}
int32_t preserve_rsuSecAppCertReq(int32_t index)
{
  if((index < 1) || (RSU_SEC_APP_CERTS_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      prior_ntcip_1218_mib.rsuSecAppCertTable[index-1].rsuSecAppCertReq = ntcip_1218_mib.rsuSecAppCertTable[index-1].rsuSecAppCertReq;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t action_rsuSecAppCertReq(int32_t index, int32_t data_in)
{
  if((index < 1) || (RSU_SEC_APP_CERTS_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if((data_in < RSU_SEC_APP_CERT_REQ_MIN ) || (RSU_SEC_APP_CERT_REQ_MAX < data_in)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  }
  if(RSEMIB_OK == ntcip1218_lock()){  
      tmp_ntcip_1218_mib.rsuSecAppCertTable[index-1].rsuSecAppCertReq = data_in;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t commit_rsuSecAppCertReq(int32_t index)
{
  if((index < 1) || (RSU_SEC_APP_CERTS_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuSecAppCertTable[index-1].rsuSecAppCertReq     = tmp_ntcip_1218_mib.rsuSecAppCertTable[index-1].rsuSecAppCertReq; 
      tmp_ntcip_1218_mib.rsuSecAppCertTable[index-1].rsuSecAppCertReq = 0;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t undo_rsuSecAppCertReq(int32_t index)
{
  if((index < 1) || (RSU_SEC_APP_CERTS_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuSecAppCertTable[index-1].rsuSecAppCertReq = prior_ntcip_1218_mib.rsuSecAppCertTable[index-1].rsuSecAppCertReq;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}

/* 5.9.11: 0..URI255_LENGTH_MAX */
int32_t get_rsuSecCertRevocationUrl(uint8_t * data_out)
{
  int32_t size = 0;
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      size = ntcip_1218_mib.rsuSecCertRevocationUrl_length;
      memcpy(data_out, ntcip_1218_mib.rsuSecCertRevocationUrl, URI255_LENGTH_MAX);
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
  if((size < URI255_LENGTH_MIN) || (URI255_LENGTH_MAX < size)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_DATA;
  } 
  return size;
}

/* 5.9.12 */
int32_t get_rsuSecCertRevocationTime(uint8_t * data_out)
{
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      memcpy(data_out, ntcip_1218_mib.rsuSecCertRevocationTime, MIB_DATEANDTIME_LENGTH);
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}

/* 5.9.13: 0..255:RSU_SECURITY_CERT_REVOCATION_UPDATE_DEFAULT */
int32_t get_rsuSecCertRevocationInterval(void)
{
  int32_t data = RSU_SECURITY_CERT_REVOCATION_UPDATE_DEFAULT;
  if(RSEMIB_OK == ntcip1218_lock()){
      data = ntcip_1218_mib.rsuSecCertRevocationInterval;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  } 
  if((RSU_SECURITY_CERT_REVOCATION_UPDATE_MIN <= data) && (data <= RSU_SECURITY_CERT_REVOCATION_UPDATE_MAX)){
      return data; 
  } else {
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } 
}
int32_t preserve_rsuSecCertRevocationInterval(void)
{
  if(RSEMIB_OK == ntcip1218_lock()){
      prior_ntcip_1218_mib.rsuSecCertRevocationInterval = ntcip_1218_mib.rsuSecCertRevocationInterval;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t action_rsuSecCertRevocationInterval(int32_t data)
{
  if((data < RSU_SECURITY_CERT_REVOCATION_UPDATE_MIN ) || (RSU_SECURITY_CERT_REVOCATION_UPDATE_MAX < data)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  }
  if(RSEMIB_OK == ntcip1218_lock()){  
      tmp_ntcip_1218_mib.rsuSecCertRevocationInterval = data;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t commit_rsuSecCertRevocationInterval(void)
{
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuSecCertRevocationInterval     = tmp_ntcip_1218_mib.rsuSecCertRevocationInterval; 
      tmp_ntcip_1218_mib.rsuSecCertRevocationInterval = RSU_SECURITY_CERT_REVOCATION_UPDATE_DEFAULT;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t undo_rsuSecCertRevocationInterval(void)
{
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuSecCertRevocationInterval = prior_ntcip_1218_mib.rsuSecCertRevocationInterval;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}

/* 5.9.14: 0..1. */
int32_t get_rsuSecCertRevocationUpdate(void)
{
  int32_t data = 0;
  if(RSEMIB_OK == ntcip1218_lock()){
      data = ntcip_1218_mib.rsuSecCertRevocationUpdate;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  } 
  if((0 <= data) && (data <= 1)){
      return data; 
  } else {
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } 
}
int32_t preserve_rsuSecCertRevocationUpdate(void)
{
  if(RSEMIB_OK == ntcip1218_lock()){
      prior_ntcip_1218_mib.rsuSecCertRevocationUpdate = ntcip_1218_mib.rsuSecCertRevocationUpdate;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t action_rsuSecCertRevocationUpdate(int32_t data)
{
  if((data < 0) || (1 < data)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  }
  if(RSEMIB_OK == ntcip1218_lock()){  
      tmp_ntcip_1218_mib.rsuSecCertRevocationUpdate = data;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t commit_rsuSecCertRevocationUpdate(void)
{
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuSecCertRevocationUpdate     = tmp_ntcip_1218_mib.rsuSecCertRevocationUpdate; 
      tmp_ntcip_1218_mib.rsuSecCertRevocationUpdate = 0;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t undo_rsuSecCertRevocationUpdate(void)
{
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuSecCertRevocationUpdate = prior_ntcip_1218_mib.rsuSecCertRevocationUpdate;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}

/* 5.9.15: RSU_SECURITY_PROFILES_MAX */
int32_t get_maxRsuSecProfiles(void)
{
  int32_t data = RSU_SECURITY_PROFILES_MAX;
  if(RSEMIB_OK == ntcip1218_lock()){
      data = ntcip_1218_mib.maxRsuSecProfiles;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  } 
  if((RSU_SECURITY_PROFILES_MIN < data) && (data <= RSU_SECURITY_PROFILES_MAX)){
      return data; 
  } else {
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } 
}

/* 5.9.16.2. */
int32_t get_rsuSecProfileName(int32_t index, uint8_t * data_out)
{
  int32_t size = 0;
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if((index < 1) || (RSU_SECURITY_PROFILES_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      size = ntcip_1218_mib.rsuSecProfileTable[index-1].rsuSecProfileName_length;
      memcpy(data_out, ntcip_1218_mib.rsuSecProfileTable[index-1].rsuSecProfileName,RSU_SECURITY_PROFILE_NAME_LENGTH_MAX);
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
  if((size < RSU_SECURITY_PROFILE_NAME_LENGTH_MIN) || (RSU_SECURITY_PROFILE_NAME_LENGTH_MAX < size)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } 
  return size;
}

/* 5.9.16.3. */
int32_t get_rsuSecProfileDesc(int32_t index, uint8_t * data_out)
{
  int32_t size = 0;
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if((index < RSU_SECURITY_PROFILES_MIN) || (RSU_SECURITY_PROFILES_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      size = ntcip_1218_mib.rsuSecProfileTable[index-1].rsuSecProfileDesc_length;
      memcpy(data_out, ntcip_1218_mib.rsuSecProfileTable[index-1].rsuSecProfileDesc,RSU_SECURITY_PROFILE_DESC_LENGTH_MAX);
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
  if((size < RSU_SECURITY_PROFILE_DESC_LENGTH_MIN) || (RSU_SECURITY_PROFILE_DESC_LENGTH_MAX < size)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } 
  return size;
}


/******************************************************************************
 * 5.10 WAVE Service Advertisement: { rsu 9 }: rsuWsaConfig.
 ******************************************************************************/

/* 5.10.1 */
int32_t get_maxRsuWsaServices(void)
{
  int32_t data = RSU_WSA_SERVICES_MAX;
  if(RSEMIB_OK == ntcip1218_lock()){
      data = ntcip_1218_mib.maxRsuWsaServices;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  } 
  if((RSU_WSA_SERVICES_MIN <= data) && (data <= RSU_WSA_SERVICES_MAX)){
      return data; 
  } else {
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } 
}

/* 5.10.2.1 */
int32_t  get_rsuWsaIndex(int32_t index, int32_t * data_out)
{
  if((index < RSU_WSA_SERVICES_MIN) || (RSU_WSA_SERVICES_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      *data_out = ntcip_1218_mib.rsuWsaServiceTable[index - 1].rsuWsaIndex;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
  if((*data_out < RSU_WSA_SERVICES_MIN) || (RSU_WSA_SERVICES_MAX < *data_out)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } 
  return RSEMIB_OK;
}

/* 5.10.2.2 */
int32_t get_rsuWsaPsid(int32_t index, uint8_t * data_out)
{
  int32_t length = 0;
  if((index < RSU_WSA_SERVICES_MIN) || (RSU_WSA_SERVICES_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      length = ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaPsid_length;
      memcpy(data_out, ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaPsid, RSU_RADIO_PSID_SIZE);
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
  if((length < RSU_RADIO_PSID_MIN_SIZE) || (RSU_RADIO_PSID_SIZE < length)){
      return RSEMIB_BAD_INPUT;  
  }
  return length;
}
int32_t preserve_rsuWsaPsid(int32_t index)
{
  if((index < RSU_WSA_SERVICES_MIN) || (RSU_WSA_SERVICES_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      memcpy( prior_ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaPsid
            , ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaPsid, RSU_RADIO_PSID_SIZE);
      prior_ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaPsid_length =
          ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaPsid_length;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t action_rsuWsaPsid(int32_t index, uint8_t * data_in, int32_t length)
{
  uint32_t psid = 0x0;
  if((index < RSU_WSA_SERVICES_MIN) || (RSU_WSA_SERVICES_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  } 
  /* User can send send us an empty list of zero length */
  if((NULL == data_in) && (0 != length)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(( length < RSU_RADIO_PSID_MIN_SIZE) || (RSU_RADIO_PSID_SIZE < length)){
      return RSEMIB_BAD_INPUT;  
  }
  /* Reject known bogus PSID's. */
  if(length >= 1) {
      psid  = (uint32_t)(data_in[0]);
  }
  if(length >= 2) {
      psid  = psid << 8;
      psid  |= (uint32_t)(data_in[1]);
  }
  if(length >= 3) {
      psid  = psid << 8;
      psid  |= (uint32_t)(data_in[2]);
  }
  if(length == 4) {
      psid  = psid << 8;
      psid  |= (uint32_t)(data_in[3]);
  }
  if(psid <= (uint32_t) PSID_1BYTE_MAX_VALUE){
  } else if(psid >= (uint32_t)PSID_2BYTE_MIN_VALUE && psid <= (uint32_t)PSID_2BYTE_MAX_VALUE){
  } else if(psid >= (uint32_t)PSID_3BYTE_MIN_VALUE && psid <= (uint32_t)PSID_3BYTE_MAX_VALUE){
  } else if(psid >= (uint32_t)PSID_4BYTE_MIN_VALUE && psid <= (uint32_t)PSID_4BYTE_MAX_VALUE){
  } else {
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){  
      memset(tmp_ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaPsid, 0x0, RSU_RADIO_PSID_SIZE);
      if(NULL != data_in){
          if(1 == length) {
              tmp_ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaPsid[3] = data_in[0];
          }
          if(2 == length){
              tmp_ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaPsid[3] = data_in[1];
              tmp_ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaPsid[2] = data_in[0];
          }
          if(3 == length) {
              tmp_ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaPsid[3] = data_in[2];
              tmp_ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaPsid[2] = data_in[1];
              tmp_ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaPsid[1] = data_in[0];
          }
          if(4 == length) {
              tmp_ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaPsid[3] = data_in[3];
              tmp_ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaPsid[2] = data_in[2];
              tmp_ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaPsid[1] = data_in[1];
              tmp_ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaPsid[0] = data_in[0];
          }
      }
      tmp_ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaPsid_length = length;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t commit_rsuWsaPsid(int32_t index)
{
  if((index < RSU_WSA_SERVICES_MIN) || (RSU_WSA_SERVICES_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  } 
  if(RSEMIB_OK == ntcip1218_lock()){
      memcpy( ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaPsid
            , tmp_ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaPsid , RSU_RADIO_PSID_SIZE);
      ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaPsid_length = 
          tmp_ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaPsid_length;
      memset(tmp_ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaPsid, 0x0, RSU_RADIO_PSID_SIZE);
      tmp_ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaPsid_length = 0;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t undo_rsuWsaPsid(int32_t index)
{
  if((index < RSU_WSA_SERVICES_MIN) || (RSU_WSA_SERVICES_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      memcpy( ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaPsid
            , prior_ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaPsid, RSU_RADIO_PSID_SIZE);
      ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaPsid_length =
          prior_ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaPsid_length;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}

/* 5.10.2.3 */
int32_t get_rsuWsaPriority(int32_t index, int32_t * data_out)
{
  if((index < RSU_WSA_SERVICES_MIN) || (RSU_WSA_SERVICES_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      *data_out = ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaPriority;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
  if((*data_out < WSA_PRIORITY_MIN) || (WSA_PRIORITY_MAX < *data_out)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } 
  return RSEMIB_OK;

}
int32_t preserve_rsuWsaPriority(int32_t index)
{
  if((index < RSU_WSA_SERVICES_MIN) || (RSU_WSA_SERVICES_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      prior_ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaPriority = ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaPriority;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t action_rsuWsaPriority(int32_t index, int32_t data_in)
{
  if((index < RSU_WSA_SERVICES_MIN) || (RSU_WSA_SERVICES_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if((data_in < WSA_PRIORITY_MIN) || (WSA_PRIORITY_MAX < data_in)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  }
  if(RSEMIB_OK == ntcip1218_lock()){  
      tmp_ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaPriority = data_in;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t commit_rsuWsaPriority(int32_t index)
{
  if((index < RSU_WSA_SERVICES_MIN) || (RSU_WSA_SERVICES_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaPriority = tmp_ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaPriority;
      tmp_ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaPriority = WSA_PRIORITY_MIN;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t undo_rsuWsaPriority(int32_t index)
{
  if((index < RSU_WSA_SERVICES_MIN) || (RSU_WSA_SERVICES_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaPriority = prior_ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaPriority;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}

/* 5.10.2.4 */
int32_t get_rsuWsaPSC(int32_t index, uint8_t * data_out)
{
  int32_t size = 0;
  if((index < RSU_WSA_SERVICES_MIN) || (RSU_WSA_SERVICES_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      size = ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaPSC_length;
      memcpy(data_out, ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaPSC, WSA_PSC_LENGTH_MAX);
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
  if((size < WSA_PSC_LENGTH_MIN) || (WSA_PSC_LENGTH_MAX < size)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);  
      return RSEMIB_BAD_DATA;
  }
  return size;
}
int32_t preserve_rsuWsaPSC(int32_t index)
{
  if((index < RSU_WSA_SERVICES_MIN) || (RSU_WSA_SERVICES_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      memcpy( prior_ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaPSC
            , ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaPSC, WSA_PSC_LENGTH_MAX);
      prior_ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaPSC_length = 
              ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaPSC_length;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t action_rsuWsaPSC(int32_t index, uint8_t * data_in, int32_t length)
{
  if((index < RSU_WSA_SERVICES_MIN) || (RSU_WSA_SERVICES_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  } 
  /* User can send empty list of zero length. */
  if((NULL == data_in) && (0 != length)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(( length < WSA_PSC_LENGTH_MIN) || (WSA_PSC_LENGTH_MAX < length)){
      return RSEMIB_BAD_INPUT;  
  }
  if(RSEMIB_OK == ntcip1218_lock()){  
      memset(tmp_ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaPSC, 0x0, WSA_PSC_LENGTH_MAX);
      if(NULL != data_in){
          memcpy(tmp_ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaPSC, data_in, length);
      }
      tmp_ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaPSC_length = length;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t commit_rsuWsaPSC(int32_t index)
{
  if((index < RSU_WSA_SERVICES_MIN) || (RSU_WSA_SERVICES_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  } 
  if(RSEMIB_OK == ntcip1218_lock()){
      memcpy( ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaPSC
            , tmp_ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaPSC, WSA_PSC_LENGTH_MAX);
      ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaPSC_length =
              tmp_ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaPSC_length;
      memset(tmp_ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaPSC, 0x0, WSA_PSC_LENGTH_MAX);
      tmp_ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaPSC_length = 0;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t undo_rsuWsaPSC(int32_t index)
{
  if((index < RSU_WSA_SERVICES_MIN) || (RSU_WSA_SERVICES_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      memcpy( ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaPSC
            , prior_ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaPSC, WSA_PSC_LENGTH_MAX);
      ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaPSC_length =
              prior_ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaPSC_length;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}

/* 5.10.2.5 */
int32_t get_rsuWsaIpAddress(int32_t index, uint8_t * data_out)
{
  int32_t size = 0;
  if((index < RSU_WSA_SERVICES_MIN) || (RSU_WSA_SERVICES_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      size = ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaIpAddress_length;
      memcpy(data_out, ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaIpAddress, RSU_DEST_IP_MAX);
      ntcip1218_unlock();

  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
  if((size < RSU_DEST_IP_MIN) || (RSU_DEST_IP_MAX < size)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);  
      return RSEMIB_BAD_DATA;
  }
  return size;
}
int32_t preserve_rsuWsaIpAddress(int32_t index)
{
  if((index < RSU_WSA_SERVICES_MIN) || (RSU_WSA_SERVICES_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      memcpy( prior_ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaIpAddress
            , ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaIpAddress, RSU_DEST_IP_MAX);
      prior_ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaIpAddress_length =
              ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaIpAddress_length;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t action_rsuWsaIpAddress(int32_t index, uint8_t * data_in, int32_t length)
{
  if((index < RSU_WSA_SERVICES_MIN) || (RSU_WSA_SERVICES_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  } 
  if((NULL == data_in) && (0 != length)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(( length < RSU_DEST_IP_MIN) || (RSU_DEST_IP_MAX < length)){
      return RSEMIB_BAD_INPUT;  
  }
  if(RSEMIB_OK == ntcip1218_lock()){  
      memset(tmp_ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaIpAddress, 0x0, RSU_DEST_IP_MAX);
      if(NULL != data_in){
          memcpy(tmp_ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaIpAddress, data_in, length); /* If len zero don't bother, it's empty. */
      }
      tmp_ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaIpAddress_length = length;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t commit_rsuWsaIpAddress(int32_t index)
{
  if((index < RSU_WSA_SERVICES_MIN) || (RSU_WSA_SERVICES_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  } 
  if(RSEMIB_OK == ntcip1218_lock()){
      memcpy( ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaIpAddress
            , tmp_ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaIpAddress, RSU_DEST_IP_MAX);
      ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaIpAddress_length = 
              tmp_ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaIpAddress_length;
      memset(tmp_ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaIpAddress, 0x0, RSU_DEST_IP_MAX);
      tmp_ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaIpAddress_length = 0;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t undo_rsuWsaIpAddress(int32_t index)
{
  if((index < RSU_WSA_SERVICES_MIN) || (RSU_WSA_SERVICES_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      memcpy( ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaIpAddress
            , prior_ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaIpAddress, RSU_DEST_IP_MAX);
      ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaIpAddress_length = 
              prior_ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaIpAddress_length;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}

/* 5.10.2.6 */
int32_t get_rsuWsaPort(int32_t index, int32_t * data_out)
{
  if((index < RSU_WSA_SERVICES_MIN) || (RSU_WSA_SERVICES_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      *data_out = ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaPort;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
  if((*data_out < WSA_PORT_MIN) || (WSA_PORT_MAX < *data_out)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } 
  return RSEMIB_OK;

}
int32_t preserve_rsuWsaPort(int32_t index)
{
  if((index < RSU_WSA_SERVICES_MIN) || (RSU_WSA_SERVICES_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      prior_ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaPort = ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaPort;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t action_rsuWsaPort(int32_t index, int32_t data_in)
{
  if((index < RSU_WSA_SERVICES_MIN) || (RSU_WSA_SERVICES_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if((data_in < WSA_PORT_MIN) || (WSA_PORT_MAX < data_in)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  }
  if(RSEMIB_OK == ntcip1218_lock()){  
      tmp_ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaPort = data_in;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t commit_rsuWsaPort(int32_t index)
{
  if((index < RSU_WSA_SERVICES_MIN) || (RSU_WSA_SERVICES_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaPort = tmp_ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaPort;
      tmp_ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaPort = WSA_PORT_DEFAULT;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t undo_rsuWsaPort(int32_t index)
{
  if((index < RSU_WSA_SERVICES_MIN) || (RSU_WSA_SERVICES_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaPort = prior_ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaPort;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}

/* 5.10.2.7 */
int32_t get_rsuWsaChannel(int32_t index, int32_t * data_out)
{
  if((index < RSU_WSA_SERVICES_MIN) || (RSU_WSA_SERVICES_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      *data_out = ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaChannel;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
  if((*data_out < RSU_RADIO_CHANNEL_MIN) || (RSU_RADIO_CHANNEL_MAX < *data_out)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } 
  return RSEMIB_OK;

}
int32_t preserve_rsuWsaChannel(int32_t index)
{
  if((index < RSU_WSA_SERVICES_MIN) || (RSU_WSA_SERVICES_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      prior_ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaChannel = ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaChannel;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t action_rsuWsaChannel(int32_t index, int32_t data_in)
{
  if((index < RSU_WSA_SERVICES_MIN) || (RSU_WSA_SERVICES_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if((data_in < RSU_RADIO_CHANNEL_MIN) || (RSU_RADIO_CHANNEL_MAX < data_in)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  }
  if(RSEMIB_OK == ntcip1218_lock()){  
      tmp_ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaChannel = data_in;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t commit_rsuWsaChannel(int32_t index)
{
  if((index < RSU_WSA_SERVICES_MIN) || (RSU_WSA_SERVICES_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaChannel = tmp_ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaChannel;
      tmp_ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaChannel = RSU_RADIO_CHANNEL_MIN;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t undo_rsuWsaChannel(int32_t index)
{
  if((index < RSU_WSA_SERVICES_MIN) || (RSU_WSA_SERVICES_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaChannel = prior_ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaChannel;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}

/* 5.10.2.8 */
int32_t get_rsuWsaStatus(int32_t index, int32_t * data_out)
{
  if((index < RSU_WSA_SERVICES_MIN) || (RSU_WSA_SERVICES_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      *data_out = ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaStatus;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  } 
  if((ROW_STATUS_VALID_MIN <= *data_out ) && (*data_out <= ROW_STATUS_VALID_MAX)){
      return RSEMIB_OK; 
  } else {
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } 
}
int32_t preserve_rsuWsaStatus(int32_t index)
{
  if((index < RSU_WSA_SERVICES_MIN) || (RSU_WSA_SERVICES_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      set_default_row_rsuWsaService(&tmp_ntcip_1218_mib.rsuWsaServiceTable[index-1],index-1);
      prior_ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaStatus = ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaStatus;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t action_rsuWsaStatus(int32_t index, int32_t data_in)
{
  if((index < RSU_WSA_SERVICES_MIN) || (RSU_WSA_SERVICES_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if((data_in < ROW_STATUS_VALID_MIN) || (ROW_STATUS_VALID_MAX < data_in)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  }
  if(RSEMIB_OK == ntcip1218_lock()){  
      tmp_ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaStatus = data_in;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
//TODO: Commit to disk.
int32_t commit_rsuWsaStatus(int32_t index)
{
  int32_t ret = RSEMIB_OK;
  if((index < RSU_WSA_SERVICES_MIN) || (RSU_WSA_SERVICES_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      ret = RSEMIB_BAD_INDEX;
  }
  if((RSEMIB_OK == ret) && (RSEMIB_OK == ntcip1218_lock())){
      DEBUGMSGTL((MY_NAME_EXTRA, "commit_rsuWsaStatus: index=%d status=%d.\n", index,tmp_ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaStatus));
      switch(tmp_ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaStatus){
          case SNMP_ROW_CREATEANDGO:
              if(SNMP_ROW_NONEXISTENT == ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaStatus) {
                  set_default_row_rsuWsaService(&ntcip_1218_mib.rsuWsaServiceTable[index-1],index-1);
                  ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaStatus = SNMP_ROW_ACTIVE;
#if 0 // TODO:
                  if(RSEMIB_OK != commit_WsaService_to_disk(&ntcip_1218_mib.rsuWsaServiceTable[index-1])){
                      DEBUGMSGTL((MY_NAME, "commit_WsaService_to_disk: FAIL: index=%d dir=[%s].\n", index, ntcip_1218_mib.rsuWsaServiceTable[index-1].filePathName));
                      ret = RSEMIB_BAD_DATA;
                  } else
#endif
                  {
                      /* Rebuild helper table before ack. */
                      if (RSEMIB_OK != (ret = rebuild_rsuWsaService_live(&ntcip_1218_mib.rsuWsaServiceTable[0]))){
                          ret = RSEMIB_BAD_DATA;
                      }
                  }
              } else {
                  /* If it exists already then do nothing. Dont send error back. */
                  //ret = RSEMIB_BAD_MODE;
              }
              break;
          case SNMP_ROW_CREATEANDWAIT:
              if(SNMP_ROW_NONEXISTENT == ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaStatus) {
                  set_default_row_rsuWsaService(&ntcip_1218_mib.rsuWsaServiceTable[index-1],index-1);
                  ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaStatus = SNMP_ROW_NOTINSERVICE;
#if 0 // TODO:
                  if(RSEMIB_OK != commit_WsaService_to_disk(&ntcip_1218_mib.rsuWsaServiceTable[index-1])){
                      DEBUGMSGTL((MY_NAME, "commit_WsaService_to_disk: FAIL: index=%d dir=[%s].\n", index, ntcip_1218_mib.rsuWsaServiceTable[index-1].filePathName));
                      ret = RSEMIB_BAD_DATA;
                  } else
#endif
                  {
                      /* Rebuild helper table before ack. */
                      if (RSEMIB_OK != (ret = rebuild_rsuWsaService_live(&ntcip_1218_mib.rsuWsaServiceTable[0]))){
                          ret = RSEMIB_BAD_DATA;
                      }
                  }
              } else {
                  /* If it exists already then do nothing. Dont send error back. */
                  //ret = RSEMIB_BAD_MODE;
              }
              break;
          case SNMP_ROW_DESTROY: /* User deletes row from table. */
              if(SNMP_ROW_NONEXISTENT == ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaStatus){
                  ret = RSEMIB_OK; /* If you try to DESTROY non-exixtent row that's an easy one. */
              } else {
#if 0 // TODO:
                  /* rm from RSU. */
                  memset(command_buffer,'\0',sizeof(command_buffer));
                  snprintf(command_buffer, sizeof(command_buffer), "/bin/rm -f %s", ntcip_1218_mib.rsuWsaServiceTable[index-1].filePathName);
                  DEBUGMSGTL((MY_NAME, "DESTROY: index=%d cmd=[%s].\n", index, command_buffer));
                  if(0 != system(command_buffer)){
                      ret = RSEMIB_BAD_DATA;
                  }
#endif
                  /* Careful to preserve index within table rows. Only call proper routine to clear. */
                  set_default_row_rsuWsaService(&ntcip_1218_mib.rsuWsaServiceTable[index-1],index-1);
                  set_default_row_rsuWsaService(&tmp_ntcip_1218_mib.rsuWsaServiceTable[index-1],index-1);
                  set_default_row_rsuWsaService(&prior_ntcip_1218_mib.rsuWsaServiceTable[index-1],index-1);

                  /* Rebuild helper table before ack. */
                  if (RSEMIB_OK != (ret = rebuild_rsuWsaService_live(&ntcip_1218_mib.rsuWsaServiceTable[0]))){
                      ret = RSEMIB_BAD_DATA;
                  }
              }
              break;
          case SNMP_ROW_ACTIVE:
              ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaStatus = SNMP_ROW_ACTIVE;
              break;
          case SNMP_ROW_NOTINSERVICE:
              ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaStatus = SNMP_ROW_NOTINSERVICE;
              break;
          case SNMP_ROW_NONEXISTENT:
          case SNMP_ROW_NOTREADY:
          default:
              ret = RSEMIB_BAD_MODE;
              break;
      }
      tmp_ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaStatus = ROW_STATUS_VALID_DEFAULT;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      ret = RSEMIB_LOCK_FAIL;
  }
  return ret;
}
int32_t undo_rsuWsaStatus(int32_t index)
{
  if((index < RSU_WSA_SERVICES_MIN) || (RSU_WSA_SERVICES_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaStatus = prior_ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaStatus;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}

/* 5.10.2.9  */
int32_t get_rsuWsaMacAddress(int32_t index, uint8_t * data_out)
{
  if((index < RSU_WSA_SERVICES_MIN) || (RSU_WSA_SERVICES_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      memcpy(data_out, ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaMacAddress, RSU_RADIO_MAC_LENGTH);
      ntcip1218_unlock();
      return RSU_RADIO_MAC_LENGTH;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t preserve_rsuWsaMacAddress(int32_t index)
{
  if((index < RSU_WSA_SERVICES_MIN) || (RSU_WSA_SERVICES_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      memcpy( prior_ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaMacAddress
            , ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaMacAddress, RSU_RADIO_MAC_LENGTH);
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t action_rsuWsaMacAddress(int32_t index, uint8_t * data_in, int32_t length)
{
  if((index < RSU_WSA_SERVICES_MIN) || (RSU_WSA_SERVICES_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  } 
  if(NULL == data_in){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSU_RADIO_MAC_LENGTH != length){
      return RSEMIB_BAD_INPUT;  
  }
  if(RSEMIB_OK == ntcip1218_lock()){  
      memset(tmp_ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaMacAddress, 0x0, RSU_RADIO_MAC_LENGTH);
      memcpy(tmp_ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaMacAddress, data_in, length);
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t commit_rsuWsaMacAddress(int32_t index)
{
  if((index < RSU_WSA_SERVICES_MIN) || (RSU_WSA_SERVICES_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  } 
  if(RSEMIB_OK == ntcip1218_lock()){
      memcpy( ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaMacAddress
            , tmp_ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaMacAddress, RSU_RADIO_MAC_LENGTH);
      memset(tmp_ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaMacAddress, 0x0, RSU_RADIO_MAC_LENGTH);
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t undo_rsuWsaMacAddress(int32_t index)
{
  if((index < RSU_WSA_SERVICES_MIN) || (RSU_WSA_SERVICES_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      memcpy( ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaMacAddress
            , prior_ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaMacAddress, RSU_RADIO_MAC_LENGTH);
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}

/* 5.10.2.10 */
int32_t get_rsuWsaOptions(int32_t index, uint8_t * data_out)
{
  if((index < RSU_WSA_SERVICES_MIN) || (RSU_WSA_SERVICES_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      *data_out = (int32_t)ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaOptions;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
  return WSA_OPTIONS_LENGTH;

}
int32_t preserve_rsuWsaOptions(int32_t index)
{
  if((index < RSU_WSA_SERVICES_MIN) || (RSU_WSA_SERVICES_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      prior_ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaOptions = ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaOptions;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t action_rsuWsaOptions(int32_t index, uint8_t * data_in, int32_t length)
{
  if((index < RSU_WSA_SERVICES_MIN) || (RSU_WSA_SERVICES_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if((NULL == data_in) || (WSA_OPTIONS_LENGTH != length)){
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){  
      tmp_ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaOptions = *data_in;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t commit_rsuWsaOptions(int32_t index)
{
  if((index < RSU_WSA_SERVICES_MIN) || (RSU_WSA_SERVICES_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaOptions = tmp_ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaOptions;
      tmp_ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaOptions = 0x0;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t undo_rsuWsaOptions(int32_t index)
{
  if((index < RSU_WSA_SERVICES_MIN) || (RSU_WSA_SERVICES_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaOptions = prior_ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaOptions;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}

/* 5.10.2.11 */
int32_t get_rsuWsaRcpiThreshold(int32_t index, int32_t * data_out)
{
  if((index < RSU_WSA_SERVICES_MIN) || (RSU_WSA_SERVICES_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      *data_out = ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaRcpiThreshold;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
  if((*data_out < WSA_RCPI_THRESHOLD_MIN) || (WSA_RCPI_THRESHOLD_MAX < *data_out)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } 
  return RSEMIB_OK;
}
int32_t preserve_rsuWsaRcpiThreshold(int32_t index)
{
  if((index < RSU_WSA_SERVICES_MIN) || (RSU_WSA_SERVICES_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      prior_ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaRcpiThreshold = 
          ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaRcpiThreshold;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t action_rsuWsaRcpiThreshold(int32_t index, int32_t data_in)
{
  if((index < RSU_WSA_SERVICES_MIN) || (RSU_WSA_SERVICES_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if((data_in < WSA_RCPI_THRESHOLD_MIN) || (WSA_RCPI_THRESHOLD_MAX < data_in)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  }
  if(RSEMIB_OK == ntcip1218_lock()){  
      tmp_ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaRcpiThreshold = data_in;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t commit_rsuWsaRcpiThreshold(int32_t index)
{
  if((index < RSU_WSA_SERVICES_MIN) || (RSU_WSA_SERVICES_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaRcpiThreshold = tmp_ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaRcpiThreshold;
      tmp_ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaRcpiThreshold = 0;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t undo_rsuWsaRcpiThreshold(int32_t index)
{
  if((index < RSU_WSA_SERVICES_MIN) || (RSU_WSA_SERVICES_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaRcpiThreshold = prior_ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaRcpiThreshold;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}

/* 5.10.2.12 */
int32_t get_rsuWsaCountThreshold(int32_t index, int32_t * data_out)
{
  if((index < RSU_WSA_SERVICES_MIN) || (RSU_WSA_SERVICES_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      *data_out = ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaCountThreshold;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
  if((*data_out < WSA_COUNT_THRESHOLD_MIN) || (WSA_COUNT_THRESHOLD_MAX < *data_out)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } 
  return RSEMIB_OK;
}

int32_t preserve_rsuWsaCountThreshold(int32_t index)
{
  if((index < RSU_WSA_SERVICES_MIN) || (RSU_WSA_SERVICES_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      prior_ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaCountThreshold = 
          ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaCountThreshold;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t action_rsuWsaCountThreshold(int32_t index, int32_t data_in)
{
  if((index < RSU_WSA_SERVICES_MIN) || (RSU_WSA_SERVICES_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if((data_in < WSA_COUNT_THRESHOLD_MIN) || (WSA_COUNT_THRESHOLD_MAX < data_in)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  }
  if(RSEMIB_OK == ntcip1218_lock()){  
      tmp_ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaCountThreshold = data_in;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t commit_rsuWsaCountThreshold(int32_t index)
{
  if((index < RSU_WSA_SERVICES_MIN) || (RSU_WSA_SERVICES_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaCountThreshold = 
          tmp_ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaCountThreshold;
      tmp_ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaCountThreshold = 0;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t undo_rsuWsaCountThreshold(int32_t index)
{
  if((index < RSU_WSA_SERVICES_MIN) || (RSU_WSA_SERVICES_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaCountThreshold = 
          prior_ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaCountThreshold;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
/* 5.10.2.13 */
int32_t get_rsuWsaCountThresholdInterval(int32_t index, int32_t * data_out)
{
  if((index < RSU_WSA_SERVICES_MIN) || (RSU_WSA_SERVICES_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      *data_out = ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaCountThresholdInterval;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
  if((*data_out < WSA_COUNT_THRESHOLD_INTERVAL_MIN) || (WSA_COUNT_THRESHOLD_INTERVAL_MAX < *data_out)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } 
  return RSEMIB_OK;
}

int32_t preserve_rsuWsaCountThresholdInterval(int32_t index)
{
  if((index < RSU_WSA_SERVICES_MIN) || (RSU_WSA_SERVICES_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      prior_ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaCountThresholdInterval = 
          ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaCountThresholdInterval;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t action_rsuWsaCountThresholdInterval(int32_t index, int32_t data_in)
{
  if((index < RSU_WSA_SERVICES_MIN) || (RSU_WSA_SERVICES_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if((data_in < WSA_COUNT_THRESHOLD_INTERVAL_MIN) || (WSA_COUNT_THRESHOLD_INTERVAL_MAX < data_in)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  }
  if(RSEMIB_OK == ntcip1218_lock()){  
      tmp_ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaCountThresholdInterval = data_in;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t commit_rsuWsaCountThresholdInterval(int32_t index)
{
  if((index < RSU_WSA_SERVICES_MIN) || (RSU_WSA_SERVICES_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaCountThresholdInterval = 
          tmp_ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaCountThresholdInterval;
      tmp_ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaCountThresholdInterval = 0;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t undo_rsuWsaCountThresholdInterval(int32_t index)
{
  if((index < RSU_WSA_SERVICES_MIN) || (RSU_WSA_SERVICES_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaCountThresholdInterval = 
          prior_ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaCountThresholdInterval;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}

/* 5.10.2.14 */
int32_t get_rsuWsaRepeatRate(int32_t index, int32_t * data_out)
{
  if((index < RSU_WSA_SERVICES_MIN) || (RSU_WSA_SERVICES_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      *data_out = ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaRepeatRate;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
  if((*data_out < WSA_REPEAT_RATE_MIN) || (WSA_REPEAT_RATE_MAX < *data_out)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } 
  return RSEMIB_OK;
}

int32_t preserve_rsuWsaRepeatRate(int32_t index)
{
  if((index < RSU_WSA_SERVICES_MIN) || (RSU_WSA_SERVICES_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      prior_ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaRepeatRate = ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaRepeatRate;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t action_rsuWsaRepeatRate(int32_t index, int32_t data_in)
{
  if((index < RSU_WSA_SERVICES_MIN) || (RSU_WSA_SERVICES_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if((data_in < WSA_REPEAT_RATE_MIN) || (WSA_REPEAT_RATE_MAX < data_in)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  }
  if(RSEMIB_OK == ntcip1218_lock()){  
      tmp_ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaRepeatRate = data_in;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t commit_rsuWsaRepeatRate(int32_t index)
{
  if((index < RSU_WSA_SERVICES_MIN) || (RSU_WSA_SERVICES_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaRepeatRate = tmp_ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaRepeatRate;
      tmp_ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaRepeatRate = 0;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t undo_rsuWsaRepeatRate(int32_t index)
{
  if((index < RSU_WSA_SERVICES_MIN) || (RSU_WSA_SERVICES_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaRepeatRate = prior_ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaRepeatRate;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}

/* 5.10.2.15 */
int32_t get_rsuWsaAdvertiserIdentifier(int32_t index, uint8_t * data_out)
{
  int32_t size = 0;
  if((index < RSU_WSA_SERVICES_MIN) || (RSU_WSA_SERVICES_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      size = ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaAdvertiserIdentifier_length;
      memcpy(data_out, ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaAdvertiserIdentifier, WSA_AD_ID_LENGTH_MAX);
      ntcip1218_unlock();

  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
  if((size < WSA_AD_ID_LENGTH_MIN) || (WSA_AD_ID_LENGTH_MAX < size)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);  
      return RSEMIB_BAD_DATA;
  }
  return size;
}
int32_t preserve_rsuWsaAdvertiserIdentifier(int32_t index)
{
  if((index < RSU_WSA_SERVICES_MIN) || (RSU_WSA_SERVICES_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      memcpy( prior_ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaAdvertiserIdentifier
            , ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaAdvertiserIdentifier, WSA_AD_ID_LENGTH_MAX);
      prior_ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaAdvertiserIdentifier_length = 
              ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaAdvertiserIdentifier_length;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t action_rsuWsaAdvertiserIdentifier(int32_t index, uint8_t * data_in, int32_t length)
{
  if((index < RSU_WSA_SERVICES_MIN) || (RSU_WSA_SERVICES_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  } 
  if((NULL == data_in) && (0 != length)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if((length < WSA_AD_ID_LENGTH_MIN) || (WSA_AD_ID_LENGTH_MAX < length)){
      return RSEMIB_BAD_INPUT;  
  }
  if(RSEMIB_OK == ntcip1218_lock()){  
      memset(tmp_ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaAdvertiserIdentifier, 0x0, WSA_AD_ID_LENGTH_MAX);
      if(NULL != data_in){
          memcpy(tmp_ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaAdvertiserIdentifier, data_in, length);
      }
      tmp_ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaAdvertiserIdentifier_length = length;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t commit_rsuWsaAdvertiserIdentifier(int32_t index)
{
  if((index < RSU_WSA_SERVICES_MIN) || (RSU_WSA_SERVICES_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  } 
  if(RSEMIB_OK == ntcip1218_lock()){
      memcpy( ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaAdvertiserIdentifier
            , tmp_ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaAdvertiserIdentifier, WSA_AD_ID_LENGTH_MAX);
      ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaAdvertiserIdentifier_length =
              tmp_ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaAdvertiserIdentifier_length;
      memset(tmp_ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaAdvertiserIdentifier, 0x0, WSA_AD_ID_LENGTH_MAX);
      tmp_ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaAdvertiserIdentifier_length = 0;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t undo_rsuWsaAdvertiserIdentifier(int32_t index)
{
  if((index < RSU_WSA_SERVICES_MIN) || (RSU_WSA_SERVICES_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      memcpy( ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaAdvertiserIdentifier
            , prior_ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaAdvertiserIdentifier, WSA_AD_ID_LENGTH_MAX);
      ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaAdvertiserIdentifier_length =
              prior_ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaAdvertiserIdentifier_length;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}

/* 5.10.2.16 */
int32_t get_rsuWsaEnable(int32_t index, int32_t * data_out)
{
  if((index < RSU_WSA_SERVICES_MIN) || (RSU_WSA_SERVICES_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      *data_out = ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaEnable;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
  if((*data_out < 0) || (1 < *data_out)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } 
  return RSEMIB_OK;
}

int32_t preserve_rsuWsaEnable(int32_t index)
{
  if((index < RSU_WSA_SERVICES_MIN) || (RSU_WSA_SERVICES_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      prior_ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaEnable = ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaEnable;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t action_rsuWsaEnable(int32_t index, int32_t data_in)
{
  if((index < RSU_WSA_SERVICES_MIN) || (RSU_WSA_SERVICES_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if((data_in < 0) || (1 < data_in)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  }
  if(RSEMIB_OK == ntcip1218_lock()){  
      tmp_ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaEnable = data_in;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t commit_rsuWsaEnable(int32_t index)
{
  if((index < RSU_WSA_SERVICES_MIN) || (RSU_WSA_SERVICES_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaEnable = tmp_ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaEnable;
      tmp_ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaEnable = 0;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t undo_rsuWsaEnable(int32_t index)
{
  if((index < RSU_WSA_SERVICES_MIN) || (RSU_WSA_SERVICES_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaEnable = prior_ntcip_1218_mib.rsuWsaServiceTable[index-1].rsuWsaEnable;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}

/* 5.10.3.1 */
int32_t  get_rsuWsaChannelIndex(int32_t index, int32_t * data_out)
{
  if((index < RSU_WSA_SERVICES_MIN) || (RSU_WSA_SERVICES_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      *data_out = ntcip_1218_mib.rsuWsaChannelTable[index - 1].rsuWsaChannelIndex;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
  if((*data_out < RSU_WSA_SERVICES_MIN) || (RSU_WSA_SERVICES_MAX < *data_out)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } 
  return RSEMIB_OK;
}

/* 5.10.3.2 */
int32_t get_rsuWsaChannelPsid(int32_t index, uint8_t * data_out)
{
  int32_t length = 0;
  if((index < RSU_WSA_SERVICES_MIN) || (RSU_WSA_SERVICES_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){ 
      length = ntcip_1218_mib.rsuWsaChannelTable[index-1].rsuWsaChannelPsid_length;
      memcpy(data_out, ntcip_1218_mib.rsuWsaChannelTable[index-1].rsuWsaChannelPsid, RSU_RADIO_PSID_SIZE);
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
  if((length < RSU_RADIO_PSID_MIN_SIZE) || (RSU_RADIO_PSID_SIZE < length)){
      return RSEMIB_BAD_INPUT;  
  }
  return length;
}
int32_t preserve_rsuWsaChannelPsid(int32_t index)
{
  if((index < RSU_WSA_SERVICES_MIN) || (RSU_WSA_SERVICES_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      memcpy( prior_ntcip_1218_mib.rsuWsaChannelTable[index-1].rsuWsaChannelPsid
            , ntcip_1218_mib.rsuWsaChannelTable[index-1].rsuWsaChannelPsid, RSU_RADIO_PSID_SIZE);
      prior_ntcip_1218_mib.rsuWsaChannelTable[index-1].rsuWsaChannelPsid_length =
              ntcip_1218_mib.rsuWsaChannelTable[index-1].rsuWsaChannelPsid_length;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t action_rsuWsaChannelPsid(int32_t index, uint8_t * data_in, int32_t length)
{
  uint32_t psid = 0x0;
  if((index < RSU_WSA_SERVICES_MIN) || (RSU_WSA_SERVICES_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  } 
  if(NULL == data_in){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if((length < RSU_RADIO_PSID_MIN_SIZE) || (RSU_RADIO_PSID_SIZE < length)){
      return RSEMIB_BAD_INPUT;  
  }
  /* Reject known bogus PSID's. */
  if(length >= 1) {
      psid  = (uint32_t)(data_in[0]);
  }
  if(length >= 2) {
      psid  = psid << 8;
      psid  |= (uint32_t)(data_in[1]);
  }
  if(length >= 3) {
      psid  = psid << 8;
      psid  |= (uint32_t)(data_in[2]);
  }
  if(length == 4) {
      psid  = psid << 8;
      psid  |= (uint32_t)(data_in[3]);
  }
  if(psid <= (uint32_t) PSID_1BYTE_MAX_VALUE){
  } else if(psid >= (uint32_t)PSID_2BYTE_MIN_VALUE && psid <= (uint32_t)PSID_2BYTE_MAX_VALUE){
  } else if(psid >= (uint32_t)PSID_3BYTE_MIN_VALUE && psid <= (uint32_t)PSID_3BYTE_MAX_VALUE){
  } else if(psid >= (uint32_t)PSID_4BYTE_MIN_VALUE && psid <= (uint32_t)PSID_4BYTE_MAX_VALUE){
  } else {
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){  
      memset(tmp_ntcip_1218_mib.rsuWsaChannelTable[index-1].rsuWsaChannelPsid, 0x0, RSU_RADIO_PSID_SIZE);
      if(1 == length) {
          tmp_ntcip_1218_mib.rsuWsaChannelTable[index-1].rsuWsaChannelPsid[3] = data_in[0];
      }
      if(2 == length){
          tmp_ntcip_1218_mib.rsuWsaChannelTable[index-1].rsuWsaChannelPsid[3] = data_in[1];
          tmp_ntcip_1218_mib.rsuWsaChannelTable[index-1].rsuWsaChannelPsid[2] = data_in[0];
      }
      if(3 == length) {
          tmp_ntcip_1218_mib.rsuWsaChannelTable[index-1].rsuWsaChannelPsid[3] = data_in[2];
          tmp_ntcip_1218_mib.rsuWsaChannelTable[index-1].rsuWsaChannelPsid[2] = data_in[1];
          tmp_ntcip_1218_mib.rsuWsaChannelTable[index-1].rsuWsaChannelPsid[1] = data_in[0];
      }
      if(4 == length) {
          tmp_ntcip_1218_mib.rsuWsaChannelTable[index-1].rsuWsaChannelPsid[3] = data_in[3];
          tmp_ntcip_1218_mib.rsuWsaChannelTable[index-1].rsuWsaChannelPsid[2] = data_in[2];
          tmp_ntcip_1218_mib.rsuWsaChannelTable[index-1].rsuWsaChannelPsid[1] = data_in[1];
          tmp_ntcip_1218_mib.rsuWsaChannelTable[index-1].rsuWsaChannelPsid[0] = data_in[0];
      }
      tmp_ntcip_1218_mib.rsuWsaChannelTable[index-1].rsuWsaChannelPsid_length = length;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t commit_rsuWsaChannelPsid(int32_t index)
{
  if((index < RSU_WSA_SERVICES_MIN) || (RSU_WSA_SERVICES_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  } 
  if(RSEMIB_OK == ntcip1218_lock()){
      memcpy( ntcip_1218_mib.rsuWsaChannelTable[index-1].rsuWsaChannelPsid
            , tmp_ntcip_1218_mib.rsuWsaChannelTable[index-1].rsuWsaChannelPsid, RSU_RADIO_PSID_SIZE);
      ntcip_1218_mib.rsuWsaChannelTable[index-1].rsuWsaChannelPsid_length =
              tmp_ntcip_1218_mib.rsuWsaChannelTable[index-1].rsuWsaChannelPsid_length;
      memset(tmp_ntcip_1218_mib.rsuWsaChannelTable[index-1].rsuWsaChannelPsid, 0x0, RSU_RADIO_PSID_SIZE);
      tmp_ntcip_1218_mib.rsuWsaChannelTable[index-1].rsuWsaChannelPsid_length = 0;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t undo_rsuWsaChannelPsid(int32_t index)
{
  if((index < RSU_WSA_SERVICES_MIN) || (RSU_WSA_SERVICES_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      memcpy( ntcip_1218_mib.rsuWsaChannelTable[index-1].rsuWsaChannelPsid
            , prior_ntcip_1218_mib.rsuWsaChannelTable[index-1].rsuWsaChannelPsid, RSU_RADIO_PSID_SIZE);
      ntcip_1218_mib.rsuWsaChannelTable[index-1].rsuWsaChannelPsid_length =
              prior_ntcip_1218_mib.rsuWsaChannelTable[index-1].rsuWsaChannelPsid_length;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}

/* 5.10.3.3 */
int32_t get_rsuWsaChannelNumber(int32_t index, int32_t * data_out)
{
  if((index < RSU_WSA_SERVICES_MIN) || (RSU_WSA_SERVICES_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      *data_out = ntcip_1218_mib.rsuWsaChannelTable[index-1].rsuWsaChannelNumber;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
  if((*data_out < RSU_RADIO_CHANNEL_MIN) || (RSU_RADIO_CHANNEL_MAX < *data_out)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } 
  return RSEMIB_OK;
}

int32_t preserve_rsuWsaChannelNumber(int32_t index)
{
  if((index < RSU_WSA_SERVICES_MIN) || (RSU_WSA_SERVICES_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      prior_ntcip_1218_mib.rsuWsaChannelTable[index-1].rsuWsaChannelNumber = ntcip_1218_mib.rsuWsaChannelTable[index-1].rsuWsaChannelNumber;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t action_rsuWsaChannelNumber(int32_t index, int32_t data_in)
{
  if((index < RSU_WSA_SERVICES_MIN) || (RSU_WSA_SERVICES_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if((data_in < RSU_RADIO_CHANNEL_MIN) || (RSU_RADIO_CHANNEL_MAX < data_in)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  }
  if(RSEMIB_OK == ntcip1218_lock()){  
      tmp_ntcip_1218_mib.rsuWsaChannelTable[index-1].rsuWsaChannelNumber = data_in;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t commit_rsuWsaChannelNumber(int32_t index)
{
  if((index < RSU_WSA_SERVICES_MIN) || (RSU_WSA_SERVICES_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuWsaChannelTable[index-1].rsuWsaChannelNumber = tmp_ntcip_1218_mib.rsuWsaChannelTable[index-1].rsuWsaChannelNumber;
      tmp_ntcip_1218_mib.rsuWsaChannelTable[index-1].rsuWsaChannelNumber = 0;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t undo_rsuWsaChannelNumber(int32_t index)
{
  if((index < RSU_WSA_SERVICES_MIN) || (RSU_WSA_SERVICES_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuWsaChannelTable[index-1].rsuWsaChannelNumber = prior_ntcip_1218_mib.rsuWsaChannelTable[index-1].rsuWsaChannelNumber;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}

/* 5.10.3.4 */
int32_t get_rsuWsaChannelTxPowerLevel(int32_t index, int32_t * data_out)
{
  if((index < RSU_WSA_SERVICES_MIN) || (RSU_WSA_SERVICES_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      *data_out = ntcip_1218_mib.rsuWsaChannelTable[index-1].rsuWsaChannelTxPowerLevel;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
  if((*data_out < RSU_RADIO_TX_POWER_MIN) || (RSU_RADIO_TX_POWER_MAX < *data_out)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } 
  return RSEMIB_OK;
}

int32_t preserve_rsuWsaChannelTxPowerLevel(int32_t index)
{
  if((index < RSU_WSA_SERVICES_MIN) || (RSU_WSA_SERVICES_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      prior_ntcip_1218_mib.rsuWsaChannelTable[index-1].rsuWsaChannelTxPowerLevel = 
          ntcip_1218_mib.rsuWsaChannelTable[index-1].rsuWsaChannelTxPowerLevel;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t action_rsuWsaChannelTxPowerLevel(int32_t index, int32_t data_in)
{
  if((index < RSU_WSA_SERVICES_MIN) || (RSU_WSA_SERVICES_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if((data_in < RSU_RADIO_TX_POWER_MIN) || (RSU_RADIO_TX_POWER_MAX < data_in)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  }
  if(RSEMIB_OK == ntcip1218_lock()){  
      tmp_ntcip_1218_mib.rsuWsaChannelTable[index-1].rsuWsaChannelTxPowerLevel = data_in;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t commit_rsuWsaChannelTxPowerLevel(int32_t index)
{
  if((index < RSU_WSA_SERVICES_MIN) || (RSU_WSA_SERVICES_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuWsaChannelTable[index-1].rsuWsaChannelTxPowerLevel = 
          tmp_ntcip_1218_mib.rsuWsaChannelTable[index-1].rsuWsaChannelTxPowerLevel;
      tmp_ntcip_1218_mib.rsuWsaChannelTable[index-1].rsuWsaChannelTxPowerLevel = 0;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t undo_rsuWsaChannelTxPowerLevel(int32_t index)
{
  if((index < RSU_WSA_SERVICES_MIN) || (RSU_WSA_SERVICES_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuWsaChannelTable[index-1].rsuWsaChannelTxPowerLevel = 
          prior_ntcip_1218_mib.rsuWsaChannelTable[index-1].rsuWsaChannelTxPowerLevel;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}

/* 5.10.3.5 */
int32_t get_rsuWsaChannelAccess(int32_t index, int32_t * data_out)
{
  if((index < RSU_WSA_SERVICES_MIN) || (RSU_WSA_SERVICES_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      *data_out = ntcip_1218_mib.rsuWsaChannelTable[index-1].rsuWsaChannelAccess;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
  if((*data_out < WSA_CHANNEL_ACCESS_MIN) || (WSA_CHANNEL_ACCESS_MAX < *data_out)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } 
  return RSEMIB_OK;
}

int32_t preserve_rsuWsaChannelAccess(int32_t index)
{
  if((index < RSU_WSA_SERVICES_MIN) || (RSU_WSA_SERVICES_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      prior_ntcip_1218_mib.rsuWsaChannelTable[index-1].rsuWsaChannelAccess = 
          ntcip_1218_mib.rsuWsaChannelTable[index-1].rsuWsaChannelAccess;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t action_rsuWsaChannelAccess(int32_t index, int32_t data_in)
{
  if((index < RSU_WSA_SERVICES_MIN) || (RSU_WSA_SERVICES_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if((data_in < WSA_CHANNEL_ACCESS_MIN) || (WSA_CHANNEL_ACCESS_MAX < data_in)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  }
  if(RSEMIB_OK == ntcip1218_lock()){  
      tmp_ntcip_1218_mib.rsuWsaChannelTable[index-1].rsuWsaChannelAccess = data_in;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t commit_rsuWsaChannelAccess(int32_t index)
{
  if((index < RSU_WSA_SERVICES_MIN) || (RSU_WSA_SERVICES_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuWsaChannelTable[index-1].rsuWsaChannelAccess = tmp_ntcip_1218_mib.rsuWsaChannelTable[index-1].rsuWsaChannelAccess;
      tmp_ntcip_1218_mib.rsuWsaChannelTable[index-1].rsuWsaChannelAccess = 0;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t undo_rsuWsaChannelAccess(int32_t index)
{
  if((index < RSU_WSA_SERVICES_MIN) || (RSU_WSA_SERVICES_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuWsaChannelTable[index-1].rsuWsaChannelAccess = prior_ntcip_1218_mib.rsuWsaChannelTable[index-1].rsuWsaChannelAccess;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
/* 5.10.3.6 */
int32_t get_rsuWsaChannelStatus(int32_t index, int32_t * data_out)
{
  if((index < RSU_WSA_SERVICES_MIN) || (RSU_WSA_SERVICES_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      *data_out = ntcip_1218_mib.rsuWsaChannelTable[index-1].rsuWsaChannelStatus;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  } 
  if((ROW_STATUS_VALID_MIN <= *data_out ) && (*data_out <= ROW_STATUS_VALID_MAX)){
      return RSEMIB_OK; 
  } else {
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } 
}
int32_t preserve_rsuWsaChannelStatus(int32_t index)
{
  if((index < RSU_WSA_SERVICES_MIN) || (RSU_WSA_SERVICES_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      set_default_row_rsuWsaChannel(&tmp_ntcip_1218_mib.rsuWsaChannelTable[index-1],index-1);
      prior_ntcip_1218_mib.rsuWsaChannelTable[index-1].rsuWsaChannelStatus = ntcip_1218_mib.rsuWsaChannelTable[index-1].rsuWsaChannelStatus;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t action_rsuWsaChannelStatus(int32_t index, int32_t data_in)
{
  if((index < RSU_WSA_SERVICES_MIN) || (RSU_WSA_SERVICES_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if((data_in < ROW_STATUS_VALID_MIN) || (ROW_STATUS_VALID_MAX < data_in)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  }
  if(RSEMIB_OK == ntcip1218_lock()){  
      tmp_ntcip_1218_mib.rsuWsaChannelTable[index-1].rsuWsaChannelStatus = data_in;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
//TODO: Commit to disk.
int32_t commit_rsuWsaChannelStatus(int32_t index)
{
  int32_t ret = RSEMIB_OK;
  if((index < RSU_WSA_SERVICES_MIN) || (RSU_WSA_SERVICES_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      ret = RSEMIB_BAD_INDEX;
  }
  if((RSEMIB_OK == ret) && (RSEMIB_OK == ntcip1218_lock())){
      DEBUGMSGTL((MY_NAME_EXTRA, "commit_rsuWsaChannel: index=%d status=%d.\n", index,tmp_ntcip_1218_mib.rsuWsaChannelTable[index-1].rsuWsaChannelStatus));
      switch(tmp_ntcip_1218_mib.rsuWsaChannelTable[index-1].rsuWsaChannelStatus){
          case SNMP_ROW_CREATEANDGO:
              if(SNMP_ROW_NONEXISTENT == ntcip_1218_mib.rsuWsaChannelTable[index-1].rsuWsaChannelStatus) {
                  set_default_row_rsuWsaChannel(&ntcip_1218_mib.rsuWsaChannelTable[index-1],index-1);
                  ntcip_1218_mib.rsuWsaChannelTable[index-1].rsuWsaChannelStatus = SNMP_ROW_ACTIVE;
#if 0 // TODO:
                  if(RSEMIB_OK != commit_WsaService_to_disk(&ntcip_1218_mib.rsuWsaChannelTable[index-1])){
                      DEBUGMSGTL((MY_NAME, "commit_WsaService_to_disk: FAIL: index=%d dir=[%s].\n", index, ntcip_1218_mib.rsuWsaChannelTable[index-1].filePathName));
                      ret = RSEMIB_BAD_DATA;
                  } else
#endif
                  {
                      /* Rebuild helper table before ack. */
                      if (RSEMIB_OK != (ret = rebuild_rsuWsaChannel_live(&ntcip_1218_mib.rsuWsaChannelTable[0]))){
                          ret = RSEMIB_BAD_DATA;
                      }
                  }
              } else {
                  /* If it exists already then do nothing. Dont send error back. */
                  //ret = RSEMIB_BAD_MODE;
              }
              break;
          case SNMP_ROW_CREATEANDWAIT:
              if(SNMP_ROW_NONEXISTENT == ntcip_1218_mib.rsuWsaChannelTable[index-1].rsuWsaChannelStatus) {
                  set_default_row_rsuWsaChannel(&ntcip_1218_mib.rsuWsaChannelTable[index-1],index-1);
                  ntcip_1218_mib.rsuWsaChannelTable[index-1].rsuWsaChannelStatus = SNMP_ROW_NOTINSERVICE;
#if 0 // TODO:
                  if(RSEMIB_OK != commit_WsaService_to_disk(&ntcip_1218_mib.rsuWsaChannelTable[index-1])){
                      DEBUGMSGTL((MY_NAME, "commit_WsaService_to_disk: FAIL: index=%d dir=[%s].\n", index, ntcip_1218_mib.rsuWsaChannelTable[index-1].filePathName));
                      ret = RSEMIB_BAD_DATA;
                  } else
#endif
                  {
                      /* Rebuild helper table before ack. */
                      if (RSEMIB_OK != (ret = rebuild_rsuWsaChannel_live(&ntcip_1218_mib.rsuWsaChannelTable[0]))){
                          ret = RSEMIB_BAD_DATA;
                      }
                  }
              } else {
                  /* If it exists already then do nothing. Dont send error back. */
                  //ret = RSEMIB_BAD_MODE;
              }
              break;
          case SNMP_ROW_DESTROY: /* User deletes row from table. */
              if(SNMP_ROW_NONEXISTENT == ntcip_1218_mib.rsuWsaChannelTable[index-1].rsuWsaChannelStatus){
                  ret = RSEMIB_OK; /* If you try to DESTROY non-exixtent row that's an easy one. */
              } else {
#if 0 // TODO:
                  /* rm from RSU. */
                  memset(command_buffer,'\0',sizeof(command_buffer));
                  snprintf(command_buffer, sizeof(command_buffer), "/bin/rm -f %s", ntcip_1218_mib.rsuWsaChannelTable[index-1].filePathName);
                  DEBUGMSGTL((MY_NAME, "DESTROY: index=%d cmd=[%s].\n", index, command_buffer));
                  if(0 != system(command_buffer)){
                      ret = RSEMIB_BAD_DATA;
                  }
#endif
                  /* Careful to preserve index within table rows. Only call proper routine to clear. */
                  set_default_row_rsuWsaChannel(&ntcip_1218_mib.rsuWsaChannelTable[index-1],index-1);
                  set_default_row_rsuWsaChannel(&tmp_ntcip_1218_mib.rsuWsaChannelTable[index-1],index-1);
                  set_default_row_rsuWsaChannel(&prior_ntcip_1218_mib.rsuWsaChannelTable[index-1],index-1);

                  /* Rebuild helper table before ack. */
                  if (RSEMIB_OK != (ret = rebuild_rsuWsaChannel_live(&ntcip_1218_mib.rsuWsaChannelTable[0]))){
                      ret = RSEMIB_BAD_DATA;
                  }
              }
              break;
          case SNMP_ROW_ACTIVE:
              ntcip_1218_mib.rsuWsaChannelTable[index-1].rsuWsaChannelStatus = SNMP_ROW_ACTIVE;
              break;
          case SNMP_ROW_NOTINSERVICE:
              ntcip_1218_mib.rsuWsaChannelTable[index-1].rsuWsaChannelStatus = SNMP_ROW_NOTINSERVICE;
              break;
          case SNMP_ROW_NONEXISTENT:
          case SNMP_ROW_NOTREADY:
          default:
              ret = RSEMIB_BAD_MODE;
              break;
      }
      tmp_ntcip_1218_mib.rsuWsaChannelTable[index-1].rsuWsaChannelStatus = ROW_STATUS_VALID_DEFAULT;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      ret = RSEMIB_LOCK_FAIL;
  }
  return ret;
}
int32_t undo_rsuWsaChannelStatus(int32_t index)
{
  if((index < RSU_WSA_SERVICES_MIN) || (RSU_WSA_SERVICES_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuWsaChannelTable[index-1].rsuWsaChannelStatus = prior_ntcip_1218_mib.rsuWsaChannelTable[index-1].rsuWsaChannelStatus;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}

/* 5.10.4 */
int32_t get_rsuWsaVersion(void)
{
  int32_t data = RSU_WSA_SERVICES_MAX;
  if(RSEMIB_OK == ntcip1218_lock()){
      data = ntcip_1218_mib.rsuWsaVersion;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  } 
  if((RSU_WSA_SERVICES_MIN <= data) && (data <= RSU_WSA_SERVICES_MAX)){
      return data; 
  } else {
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } 
}

/******************************************************************************
 * 5.11 WSA Configuration: { rsu 10 }
 ******************************************************************************/

/* 5.11.1 */
int32_t get_rsuWraIpPrefix(uint8_t * data_out)
{
  int32_t size = 0;
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      size = ntcip_1218_mib.rsuWraIpPrefix_length;
      memcpy(data_out, ntcip_1218_mib.rsuWraIpPrefix, RSU_DEST_IP_MAX);
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
  if((size<RSU_DEST_IP_MIN) || (RSU_DEST_IP_MAX < size)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);  
      return RSEMIB_BAD_DATA;
  }
  return size;
}
int32_t preserve_rsuWraIpPrefix(void)
{
  if(RSEMIB_OK == ntcip1218_lock()){
      memcpy(prior_ntcip_1218_mib.rsuWraIpPrefix, ntcip_1218_mib.rsuWraIpPrefix, RSU_DEST_IP_MAX);
      prior_ntcip_1218_mib.rsuWraIpPrefix_length = ntcip_1218_mib.rsuWraIpPrefix_length;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t action_rsuWraIpPrefix(uint8_t * data_in, int32_t length)
{
  if((NULL == data_in) && (0 != length)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      memset(tmp_ntcip_1218_mib.rsuWraIpPrefix, 0x0, RSU_DEST_IP_MAX);
      if(NULL != data_in){ 
          memcpy(tmp_ntcip_1218_mib.rsuWraIpPrefix, data_in, length);
      }
      tmp_ntcip_1218_mib.rsuWraIpPrefix_length = length;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t commit_rsuWraIpPrefix(void)
{
  if(RSEMIB_OK == ntcip1218_lock()){
      memcpy(ntcip_1218_mib.rsuWraIpPrefix, tmp_ntcip_1218_mib.rsuWraIpPrefix, RSU_DEST_IP_MAX);
      ntcip_1218_mib.rsuWraIpPrefix_length = tmp_ntcip_1218_mib.rsuWraIpPrefix_length;
      memset(tmp_ntcip_1218_mib.rsuWraIpPrefix, 0x0, RSU_DEST_IP_MAX);
      tmp_ntcip_1218_mib.rsuWraIpPrefix_length = 0;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t undo_rsuWraIpPrefix(void)
{
  if(RSEMIB_OK == ntcip1218_lock()){
      memcpy( ntcip_1218_mib.rsuWraIpPrefix, prior_ntcip_1218_mib.rsuWraIpPrefix, RSU_DEST_IP_MAX);
      ntcip_1218_mib.rsuWraIpPrefix_length = prior_ntcip_1218_mib.rsuWraIpPrefix_length;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}

/* 5.11.2 */
int32_t get_rsuWraIpPrefixLength(uint8_t * data_out)
{
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      *data_out = ntcip_1218_mib.rsuWraIpPrefixLength;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  } 
  if( *data_out <= RSU_DEST_IP_MAX ){
      return RSEMIB_OK; 
  } else {
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } 
}
int32_t preserve_rsuWraIpPrefixLength(void)
{
  if(RSEMIB_OK == ntcip1218_lock()){
      prior_ntcip_1218_mib.rsuWraIpPrefixLength = ntcip_1218_mib.rsuWraIpPrefixLength;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t action_rsuWraIpPrefixLength(uint8_t * data_in, int32_t length)
{
  if(NULL == data_in){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(length != 1){ /* passing a string for a single integer value 8-bit */
      DEBUGMSGTL((MY_NAME, "R1:data=0x%x,len=%d.\n",*data_in,length));
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      tmp_ntcip_1218_mib.rsuWraIpPrefixLength = *data_in;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }

}
int32_t commit_rsuWraIpPrefixLength(void)
{
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuWraIpPrefixLength     = tmp_ntcip_1218_mib.rsuWraIpPrefixLength; 
      tmp_ntcip_1218_mib.rsuWraIpPrefixLength = 0;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t undo_rsuWraIpPrefixLength(void)
{
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuWraIpPrefixLength = prior_ntcip_1218_mib.rsuWraIpPrefixLength;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}

/* 5.11.3 */
int32_t get_rsuWraGateway(uint8_t * data_out)
{
  int32_t size = 0;
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      size = ntcip_1218_mib.rsuWraGateway_length;
      memcpy(data_out, ntcip_1218_mib.rsuWraGateway, RSU_DEST_IP_MAX);
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
  if((size < RSU_DEST_IP_MIN) || (RSU_DEST_IP_MAX < size)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;    
  }
  return size;
}
int32_t preserve_rsuWraGateway(void)
{
  if(RSEMIB_OK == ntcip1218_lock()){
      prior_ntcip_1218_mib.rsuWraGateway_length = ntcip_1218_mib.rsuWraGateway_length;
      memcpy(prior_ntcip_1218_mib.rsuWraGateway, ntcip_1218_mib.rsuWraGateway, RSU_DEST_IP_MAX);
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t action_rsuWraGateway(uint8_t * data_in, int32_t length)
{
  if((NULL == data_in) && (0 != length)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      memset(tmp_ntcip_1218_mib.rsuWraGateway, 0x0, RSU_DEST_IP_MAX);
      if(NULL != data_in){
          memcpy(tmp_ntcip_1218_mib.rsuWraGateway, data_in, length);
      }
      tmp_ntcip_1218_mib.rsuWraGateway_length = length;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t commit_rsuWraGateway(void)
{
  if(RSEMIB_OK == ntcip1218_lock()){
      memcpy(ntcip_1218_mib.rsuWraGateway, tmp_ntcip_1218_mib.rsuWraGateway, RSU_DEST_IP_MAX);
      ntcip_1218_mib.rsuWraGateway_length = tmp_ntcip_1218_mib.rsuWraGateway_length;
      memset(tmp_ntcip_1218_mib.rsuWraGateway, 0x0, RSU_DEST_IP_MAX);
      tmp_ntcip_1218_mib.rsuWraGateway_length = 0;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t undo_rsuWraGateway(void)
{
  if(RSEMIB_OK == ntcip1218_lock()){
      memcpy(ntcip_1218_mib.rsuWraGateway, prior_ntcip_1218_mib.rsuWraGateway, RSU_DEST_IP_MAX);
      ntcip_1218_mib.rsuWraGateway_length = prior_ntcip_1218_mib.rsuWraGateway_length;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}

/* 5.11.4 */
int32_t get_rsuWraPrimaryDns(uint8_t * data_out)
{
  int32_t size = 0;
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      size = ntcip_1218_mib.rsuWraPrimaryDns_length;
      memcpy(data_out, ntcip_1218_mib.rsuWraPrimaryDns, RSU_DEST_IP_MAX);
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
  if((size < RSU_DEST_IP_MIN) || (RSU_DEST_IP_MAX < size)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;   
  }
  return size;
}
int32_t preserve_rsuWraPrimaryDns(void)
{
  if(RSEMIB_OK == ntcip1218_lock()){
      prior_ntcip_1218_mib.rsuWraPrimaryDns_length = ntcip_1218_mib.rsuWraPrimaryDns_length;
      memcpy(prior_ntcip_1218_mib.rsuWraPrimaryDns, ntcip_1218_mib.rsuWraPrimaryDns, RSU_DEST_IP_MAX);
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t action_rsuWraPrimaryDns(uint8_t * data_in, int32_t length)
{
  if((NULL == data_in) && (0 != length)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      memset(tmp_ntcip_1218_mib.rsuWraPrimaryDns, 0x0, RSU_DEST_IP_MAX);
      if(NULL != data_in){  
          memcpy(tmp_ntcip_1218_mib.rsuWraPrimaryDns, data_in, length);    
      }
      tmp_ntcip_1218_mib.rsuWraPrimaryDns_length = length;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t commit_rsuWraPrimaryDns(void)
{
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuWraPrimaryDns_length = tmp_ntcip_1218_mib.rsuWraPrimaryDns_length;
      memcpy(ntcip_1218_mib.rsuWraPrimaryDns, tmp_ntcip_1218_mib.rsuWraPrimaryDns, RSU_DEST_IP_MAX);
      memset(tmp_ntcip_1218_mib.rsuWraPrimaryDns, 0x0, RSU_DEST_IP_MAX);
      tmp_ntcip_1218_mib.rsuWraPrimaryDns_length = 0;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t undo_rsuWraPrimaryDns(void)
{
  if(RSEMIB_OK == ntcip1218_lock()){
      memcpy(ntcip_1218_mib.rsuWraPrimaryDns, prior_ntcip_1218_mib.rsuWraPrimaryDns, RSU_DEST_IP_MAX);
      ntcip_1218_mib.rsuWraPrimaryDns_length = prior_ntcip_1218_mib.rsuWraPrimaryDns_length;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}

/* 5.11.5 */
int32_t get_rsuWraSecondaryDns(uint8_t * data_out)
{
  int32_t size = 0;
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      size = ntcip_1218_mib.rsuWraSecondaryDns_length;
      memcpy(data_out, ntcip_1218_mib.rsuWraSecondaryDns, RSU_DEST_IP_MAX);
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
  if((size < RSU_DEST_IP_MIN) || (RSU_DEST_IP_MAX < size)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;  
  }
  return size;
}
int32_t preserve_rsuWraSecondaryDns(void)
{
  if(RSEMIB_OK == ntcip1218_lock()){
      prior_ntcip_1218_mib.rsuWraSecondaryDns_length = ntcip_1218_mib.rsuWraSecondaryDns_length;
      memcpy(prior_ntcip_1218_mib.rsuWraSecondaryDns, ntcip_1218_mib.rsuWraSecondaryDns, RSU_DEST_IP_MAX);
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t action_rsuWraSecondaryDns(uint8_t * data_in, int32_t length)
{
  if((NULL == data_in) && (0 != length)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      memset(tmp_ntcip_1218_mib.rsuWraSecondaryDns, 0x0, RSU_DEST_IP_MAX);
      if(NULL != data_in){ 
          memcpy(tmp_ntcip_1218_mib.rsuWraSecondaryDns, data_in, length);
      }
      tmp_ntcip_1218_mib.rsuWraSecondaryDns_length = length;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t commit_rsuWraSecondaryDns(void)
{
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuWraSecondaryDns_length = tmp_ntcip_1218_mib.rsuWraSecondaryDns_length;
      memcpy(ntcip_1218_mib.rsuWraSecondaryDns, tmp_ntcip_1218_mib.rsuWraSecondaryDns, RSU_DEST_IP_MAX);
      memset(tmp_ntcip_1218_mib.rsuWraSecondaryDns, 0x0, RSU_DEST_IP_MAX);
      tmp_ntcip_1218_mib.rsuWraSecondaryDns_length = 0;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t undo_rsuWraSecondaryDns(void)
{
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuWraSecondaryDns_length = prior_ntcip_1218_mib.rsuWraSecondaryDns_length;
      memcpy( ntcip_1218_mib.rsuWraSecondaryDns, prior_ntcip_1218_mib.rsuWraSecondaryDns, RSU_DEST_IP_MAX);
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}

/* 5.11.6 */
int32_t get_rsuWraGatewayMacAddress(uint8_t * data_out)
{
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      memcpy(data_out, ntcip_1218_mib.rsuWraGatewayMacAddress, RSU_RADIO_MAC_LENGTH);
      ntcip1218_unlock();
      return RSU_RADIO_MAC_LENGTH;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t preserve_rsuWraGatewayMacAddress(void)
{
  if(RSEMIB_OK == ntcip1218_lock()){
      memcpy(prior_ntcip_1218_mib.rsuWraGatewayMacAddress, ntcip_1218_mib.rsuWraGatewayMacAddress, RSU_RADIO_MAC_LENGTH);
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t action_rsuWraGatewayMacAddress(uint8_t * data_in, int32_t length)
{
  if((NULL == data_in) || (RSU_RADIO_MAC_LENGTH != length)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){  
      memcpy(tmp_ntcip_1218_mib.rsuWraGatewayMacAddress, data_in, RSU_RADIO_MAC_LENGTH);
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t commit_rsuWraGatewayMacAddress(void)
{
  if(RSEMIB_OK == ntcip1218_lock()){
      memcpy(ntcip_1218_mib.rsuWraGatewayMacAddress, tmp_ntcip_1218_mib.rsuWraGatewayMacAddress, RSU_RADIO_MAC_LENGTH);
      memset(tmp_ntcip_1218_mib.rsuWraGatewayMacAddress, 0x0, RSU_RADIO_MAC_LENGTH);
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t undo_rsuWraGatewayMacAddress(void)
{
  if(RSEMIB_OK == ntcip1218_lock()){
      memcpy( ntcip_1218_mib.rsuWraGatewayMacAddress, prior_ntcip_1218_mib.rsuWraGatewayMacAddress, RSU_RADIO_MAC_LENGTH);
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}

/* 5.11.7 */
int32_t get_rsuWraLifetime(void)
{
  int32_t data = 0x0;

  if(RSEMIB_OK == ntcip1218_lock()){
      data = ntcip_1218_mib.rsuWraLifetime;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  } 
  if((RSU_WRA_LIFETIME_MIN <= data) && (data <= RSU_WRA_LIFETIME_MAX)){
      return data; 
  } else {
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } 
}
int32_t preserve_rsuWraLifetime(void)
{
  if(RSEMIB_OK == ntcip1218_lock()){
      prior_ntcip_1218_mib.rsuWraLifetime = ntcip_1218_mib.rsuWraLifetime;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t action_rsuWraLifetime(int32_t data)
{
  if((data < RSU_WRA_LIFETIME_MIN) || (RSU_WRA_LIFETIME_MAX < data)){ 
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  }
  if(RSEMIB_OK == ntcip1218_lock()){  
      tmp_ntcip_1218_mib.rsuWraLifetime = data;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t commit_rsuWraLifetime(void)
{
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuWraLifetime     = tmp_ntcip_1218_mib.rsuWraLifetime; 
      tmp_ntcip_1218_mib.rsuWraLifetime = 0;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t undo_rsuWraLifetime(void)
{
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuWraLifetime = prior_ntcip_1218_mib.rsuWraLifetime;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}

/******************************************************************************
 * 5.12 Message Statistics: { rsu 11 }: rsuMessageCountsByPsidTable: tx&rx per PSID.
 ******************************************************************************/

/* 5.12.1 */
int32_t get_maxRsuMessageCountsByPsid(void)
{
  int32_t data = RSU_PSID_TRACKED_STATS_MAX;

  if(RSEMIB_OK == ntcip1218_lock()){
      data = ntcip_1218_mib.maxRsuMessageCountsByPsid;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  } 
  if((RSU_PSID_TRACKED_STATS_MIN <= data) && (data <= RSU_PSID_TRACKED_STATS_MAX)){
      return data; 
  } else {
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } 
}

/* 5.12.2.1 */
int32_t get_rsuMessageCountsByPsidIndex(int32_t index, int32_t * data_out)
{
  if((index < RSU_PSID_TRACKED_STATS_MIN) || (RSU_PSID_TRACKED_STATS_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      *data_out = ntcip_1218_mib.rsuMessageCountsByPsidTable[index-1].rsuMessageCountsByPsidIndex;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
  if((*data_out < RSU_PSID_TRACKED_STATS_MIN) || (RSU_PSID_TRACKED_STATS_MAX < *data_out)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } 
  return RSEMIB_OK;
}

/* 5.12.2.2 */
int32_t get_rsuMessageCountsByPsidId(int32_t index, uint8_t * data_out)
{
  int32_t length = 0;
  if((index < RSU_PSID_TRACKED_STATS_MIN) || (RSU_PSID_TRACKED_STATS_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      length = ntcip_1218_mib.rsuMessageCountsByPsidTable[index-1].rsuMessageCountsByPsidId_length;
      memcpy(data_out, ntcip_1218_mib.rsuMessageCountsByPsidTable[index-1].rsuMessageCountsByPsidId, RSU_RADIO_PSID_SIZE);
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
  if((length < RSU_RADIO_PSID_MIN_SIZE) || (RSU_RADIO_PSID_SIZE < length)){
      return RSEMIB_BAD_INPUT;  
  }
  return length;
}
int32_t preserve_rsuMessageCountsByPsidId(int32_t index)
{
  if((index < RSU_PSID_TRACKED_STATS_MIN) || (RSU_PSID_TRACKED_STATS_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      memcpy( prior_ntcip_1218_mib.rsuMessageCountsByPsidTable[index-1].rsuMessageCountsByPsidId
            , ntcip_1218_mib.rsuMessageCountsByPsidTable[index-1].rsuMessageCountsByPsidId, RSU_RADIO_PSID_SIZE);
      prior_ntcip_1218_mib.rsuMessageCountsByPsidTable[index-1].rsuMessageCountsByPsidId_length =
          ntcip_1218_mib.rsuMessageCountsByPsidTable[index-1].rsuMessageCountsByPsidId_length;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t action_rsuMessageCountsByPsidId(int32_t index, uint8_t * data_in, int32_t length)
{
  if((index < RSU_PSID_TRACKED_STATS_MIN) || (RSU_PSID_TRACKED_STATS_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  } 
  if((NULL == data_in) && (0 != length)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(( length < RSU_RADIO_PSID_MIN_SIZE) || (RSU_RADIO_PSID_SIZE < length)){
      return RSEMIB_BAD_INPUT;  
  }
  if(RSEMIB_OK == ntcip1218_lock()){  
      memset(tmp_ntcip_1218_mib.rsuMessageCountsByPsidTable[index-1].rsuMessageCountsByPsidId,0x0, RSU_RADIO_PSID_SIZE);
      if(NULL != data_in){
          memcpy(tmp_ntcip_1218_mib.rsuMessageCountsByPsidTable[index-1].rsuMessageCountsByPsidId, data_in, length);
      }
      tmp_ntcip_1218_mib.rsuMessageCountsByPsidTable[index-1].rsuMessageCountsByPsidId_length = length;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t commit_rsuMessageCountsByPsidId(int32_t index)
{
  int32_t ret = RSEMIB_OK;
  if((index < RSU_PSID_TRACKED_STATS_MIN) || (RSU_PSID_TRACKED_STATS_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  } 
  if(RSEMIB_OK == ntcip1218_lock()){
      memcpy( ntcip_1218_mib.rsuMessageCountsByPsidTable[index-1].rsuMessageCountsByPsidId
            , tmp_ntcip_1218_mib.rsuMessageCountsByPsidTable[index-1].rsuMessageCountsByPsidId, RSU_RADIO_PSID_SIZE);
      ntcip_1218_mib.rsuMessageCountsByPsidTable[index-1].rsuMessageCountsByPsidId_length =
          tmp_ntcip_1218_mib.rsuMessageCountsByPsidTable[index-1].rsuMessageCountsByPsidId_length;
      memset(tmp_ntcip_1218_mib.rsuMessageCountsByPsidTable[index-1].rsuMessageCountsByPsidId, 0x0, RSU_RADIO_PSID_SIZE);
      tmp_ntcip_1218_mib.rsuMessageCountsByPsidTable[index-1].rsuMessageCountsByPsidId_length = 0;
      if(RSEMIB_OK == (ret = update_messageStats_shm(&ntcip_1218_mib.rsuMessageCountsByPsidTable[index-1]))){
          if(RSEMIB_OK != (ret = commit_rsuMessageStats_to_disk(&ntcip_1218_mib.rsuMessageCountsByPsidTable[index-1]))){
              DEBUGMSGTL((MY_NAME, "commit_rsuMessageStats_to_disk: FAIL: index=%d dir=[%s].\n", index, ntcip_1218_mib.rsuMessageCountsByPsidTable[index-1].filePathName));
              ret = RSEMIB_BAD_DATA;
          }
      } else {
          ret = RSEMIB_BAD_DATA;
      }
      ntcip1218_unlock();
      return ret;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t undo_rsuMessageCountsByPsidId(int32_t index)
{
  if((index < RSU_PSID_TRACKED_STATS_MIN) || (RSU_PSID_TRACKED_STATS_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      memcpy( ntcip_1218_mib.rsuMessageCountsByPsidTable[index-1].rsuMessageCountsByPsidId
            , prior_ntcip_1218_mib.rsuMessageCountsByPsidTable[index-1].rsuMessageCountsByPsidId, RSU_RADIO_PSID_SIZE);
      ntcip_1218_mib.rsuMessageCountsByPsidTable[index-1].rsuMessageCountsByPsidId_length =
          prior_ntcip_1218_mib.rsuMessageCountsByPsidTable[index-1].rsuMessageCountsByPsidId_length;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
/* 5.12.2.3 */
int32_t get_rsuMessageCountsByChannel(int32_t index, int32_t * data_out)
{
  if((index < RSU_PSID_TRACKED_STATS_MIN) || (RSU_PSID_TRACKED_STATS_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      *data_out = ntcip_1218_mib.rsuMessageCountsByPsidTable[index-1].rsuMessageCountsByChannel;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
#if 1 /* If it isn't the radio channel what is it? */
  if((*data_out < RSU_MESSAGE_COUNT_BY_CHANNEL_MIN) || (RSU_MESSAGE_COUNT_BY_CHANNEL_MAX < *data_out)){
#else
  if((*data_out < RSU_RADIO_CHANNEL_MIN) || (RSU_RADIO_CHANNEL_MAX < *data_out)){
#endif
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } 
  return RSEMIB_OK;
}
int32_t preserve_rsuMessageCountsByChannel(int32_t index)
{
  if((index < RSU_PSID_TRACKED_STATS_MIN) || (RSU_PSID_TRACKED_STATS_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      prior_ntcip_1218_mib.rsuMessageCountsByPsidTable[index-1].rsuMessageCountsByChannel = 
          ntcip_1218_mib.rsuMessageCountsByPsidTable[index-1].rsuMessageCountsByChannel;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t action_rsuMessageCountsByChannel(int32_t index, int32_t data_in)
{
  if((index < RSU_PSID_TRACKED_STATS_MIN) || (RSU_PSID_TRACKED_STATS_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
#if 1
  if((data_in < RSU_MESSAGE_COUNT_BY_CHANNEL_MIN) || (RSU_MESSAGE_COUNT_BY_CHANNEL_MAX < data_in)){
#else
  if((data_in < RSU_RADIO_CHANNEL_MIN) || (RSU_RADIO_CHANNEL_MAX < data_in)){
#endif
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  }
  if(RSEMIB_OK == ntcip1218_lock()){  
      tmp_ntcip_1218_mib.rsuMessageCountsByPsidTable[index-1].rsuMessageCountsByChannel = data_in;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t commit_rsuMessageCountsByChannel(int32_t index)
{
  int32_t ret = RSEMIB_OK;
  if((index < RSU_PSID_TRACKED_STATS_MIN) || (RSU_PSID_TRACKED_STATS_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuMessageCountsByPsidTable[index-1].rsuMessageCountsByChannel = 
          tmp_ntcip_1218_mib.rsuMessageCountsByPsidTable[index-1].rsuMessageCountsByChannel;
      tmp_ntcip_1218_mib.rsuMessageCountsByPsidTable[index-1].rsuMessageCountsByChannel = 0;
      if(RSEMIB_OK == (ret = update_messageStats_shm(&ntcip_1218_mib.rsuMessageCountsByPsidTable[index-1]))){
          if(RSEMIB_OK != (ret = commit_rsuMessageStats_to_disk(&ntcip_1218_mib.rsuMessageCountsByPsidTable[index-1]))){
              DEBUGMSGTL((MY_NAME, "commit_rsuMessageStats_to_disk: FAIL: index=%d dir=[%s].\n", index, ntcip_1218_mib.rsuMessageCountsByPsidTable[index-1].filePathName));
              ret = RSEMIB_BAD_DATA;
          }
      } else {
          ret = RSEMIB_BAD_DATA;
      }
      ntcip1218_unlock();
      return ret;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t undo_rsuMessageCountsByChannel(int32_t index)
{
  if((index < RSU_PSID_TRACKED_STATS_MIN) || (RSU_PSID_TRACKED_STATS_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuMessageCountsByPsidTable[index-1].rsuMessageCountsByChannel = 
          prior_ntcip_1218_mib.rsuMessageCountsByPsidTable[index-1].rsuMessageCountsByChannel;
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
/* 5.12.2.4 */
int32_t get_rsuMessageCountsDirection(int32_t index, int32_t * data_out)
{
  if((index < RSU_PSID_TRACKED_STATS_MIN) || (RSU_PSID_TRACKED_STATS_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  } 
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      *data_out = ntcip_1218_mib.rsuMessageCountsByPsidTable[index-1].rsuMessageCountsDirection;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
  if((*data_out < RSU_MESSAGE_COUNT_DIRECTION_MIN) || (RSU_MESSAGE_COUNT_DIRECTION_MAX < *data_out)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } 
  return RSEMIB_OK;

}
int32_t preserve_rsuMessageCountsDirection(int32_t index)
{
  if((index < RSU_PSID_TRACKED_STATS_MIN) || (RSU_PSID_TRACKED_STATS_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      prior_ntcip_1218_mib.rsuMessageCountsByPsidTable[index-1].rsuMessageCountsDirection = 
          ntcip_1218_mib.rsuMessageCountsByPsidTable[index-1].rsuMessageCountsDirection;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t action_rsuMessageCountsDirection(int32_t index, int32_t data_in)
{
  if((index < RSU_PSID_TRACKED_STATS_MIN) || (RSU_PSID_TRACKED_STATS_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if((data_in < RSU_MESSAGE_COUNT_DIRECTION_MIN) || (RSU_MESSAGE_COUNT_DIRECTION_MAX < data_in)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  }
  if(RSEMIB_OK == ntcip1218_lock()){  
      tmp_ntcip_1218_mib.rsuMessageCountsByPsidTable[index-1].rsuMessageCountsDirection = data_in;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t commit_rsuMessageCountsDirection(int32_t index)
{
  int32_t ret = RSEMIB_OK;
  if((index < RSU_PSID_TRACKED_STATS_MIN) || (RSU_PSID_TRACKED_STATS_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuMessageCountsByPsidTable[index-1].rsuMessageCountsDirection = 
          tmp_ntcip_1218_mib.rsuMessageCountsByPsidTable[index-1].rsuMessageCountsDirection;
      tmp_ntcip_1218_mib.rsuMessageCountsByPsidTable[index-1].rsuMessageCountsDirection = 0;
      if(RSEMIB_OK == (ret = update_messageStats_shm(&ntcip_1218_mib.rsuMessageCountsByPsidTable[index-1]))){
          if(RSEMIB_OK != (ret = commit_rsuMessageStats_to_disk(&ntcip_1218_mib.rsuMessageCountsByPsidTable[index-1]))){
              DEBUGMSGTL((MY_NAME, "commit_rsuMessageStats_to_disk: FAIL: index=%d dir=[%s].\n", index, ntcip_1218_mib.rsuMessageCountsByPsidTable[index-1].filePathName));
              ret = RSEMIB_BAD_DATA;
          }
      } else {
          ret = RSEMIB_BAD_DATA;
      }
      ntcip1218_unlock();
      return ret;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t undo_rsuMessageCountsDirection(int32_t index)
{
  if((index < RSU_PSID_TRACKED_STATS_MIN) || (RSU_PSID_TRACKED_STATS_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  } 
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuMessageCountsByPsidTable[index-1].rsuMessageCountsDirection = 
          prior_ntcip_1218_mib.rsuMessageCountsByPsidTable[index-1].rsuMessageCountsDirection;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}

/* 5.12.2.5 */
int32_t get_rsuMessageCountsByPsidTime(int32_t index, uint8_t * data_out)
{
  if((index < RSU_PSID_TRACKED_STATS_MIN) || (RSU_PSID_TRACKED_STATS_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      memcpy(data_out, ntcip_1218_mib.rsuMessageCountsByPsidTable[index-1].rsuMessageCountsByPsidTime, MIB_DATEANDTIME_LENGTH);
      ntcip1218_unlock();
      return MIB_DATEANDTIME_LENGTH;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t preserve_rsuMessageCountsByPsidTime(int32_t index)
{
  if((index < RSU_PSID_TRACKED_STATS_MIN) || (RSU_PSID_TRACKED_STATS_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      memcpy( prior_ntcip_1218_mib.rsuMessageCountsByPsidTable[index-1].rsuMessageCountsByPsidTime
            , ntcip_1218_mib.rsuMessageCountsByPsidTable[index-1].rsuMessageCountsByPsidTime, MIB_DATEANDTIME_LENGTH);
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t action_rsuMessageCountsByPsidTime(int32_t index, uint8_t * data_in, int32_t length)
{
  if((index < RSU_PSID_TRACKED_STATS_MIN) || (RSU_PSID_TRACKED_STATS_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(NULL == data_in){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(( length < 0) || (MIB_DATEANDTIME_LENGTH < length)){
      return RSEMIB_BAD_INPUT;  
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      memset(tmp_ntcip_1218_mib.rsuMessageCountsByPsidTable[index-1].rsuMessageCountsByPsidTime, 0x0, MIB_DATEANDTIME_LENGTH);  
      memcpy(tmp_ntcip_1218_mib.rsuMessageCountsByPsidTable[index-1].rsuMessageCountsByPsidTime, data_in, length);
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t commit_rsuMessageCountsByPsidTime(int32_t index)
{
  int32_t ret = RSEMIB_OK;
  if((index < RSU_PSID_TRACKED_STATS_MIN) || (RSU_PSID_TRACKED_STATS_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      memcpy( ntcip_1218_mib.rsuMessageCountsByPsidTable[index-1].rsuMessageCountsByPsidTime
            , tmp_ntcip_1218_mib.rsuMessageCountsByPsidTable[index-1].rsuMessageCountsByPsidTime, MIB_DATEANDTIME_LENGTH);
      memset(tmp_ntcip_1218_mib.rsuMessageCountsByPsidTable[index-1].rsuMessageCountsByPsidTime, 0x0, MIB_DATEANDTIME_LENGTH);
      if(RSEMIB_OK == (ret = update_messageStats_shm(&ntcip_1218_mib.rsuMessageCountsByPsidTable[index-1]))){
          if(RSEMIB_OK != (ret = commit_rsuMessageStats_to_disk(&ntcip_1218_mib.rsuMessageCountsByPsidTable[index-1]))){
              DEBUGMSGTL((MY_NAME, "commit_rsuMessageStats_to_disk: FAIL: index=%d dir=[%s].\n", index, ntcip_1218_mib.rsuMessageCountsByPsidTable[index-1].filePathName));
              ret = RSEMIB_BAD_DATA;
          }
      } else {
          ret = RSEMIB_BAD_DATA;
      }
      ntcip1218_unlock();
      return ret;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t undo_rsuMessageCountsByPsidTime(int32_t index)
{
  if((index < RSU_PSID_TRACKED_STATS_MIN) || (RSU_PSID_TRACKED_STATS_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      memcpy( ntcip_1218_mib.rsuMessageCountsByPsidTable[index-1].rsuMessageCountsByPsidTime
            , prior_ntcip_1218_mib.rsuMessageCountsByPsidTable[index-1].rsuMessageCountsByPsidTime, MIB_DATEANDTIME_LENGTH);
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
/* 5.12.2.6 */
int32_t get_rsuMessageCountsByPsidCounts(int32_t index, uint32_t * data_out)
{
  if((index < RSU_PSID_TRACKED_STATS_MIN) || (RSU_PSID_TRACKED_STATS_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      *data_out = ntcip_1218_mib.rsuMessageCountsByPsidTable[index-1].rsuMessageCountsByPsidCounts;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  } 
}

/* 5.12.2.7 */
int32_t get_rsuMessageCountsByPsidRowStatus(int32_t index, int32_t * data_out)
{
  if((index < RSU_PSID_TRACKED_STATS_MIN) || (RSU_PSID_TRACKED_STATS_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      *data_out = ntcip_1218_mib.rsuMessageCountsByPsidTable[index-1].rsuMessageCountsByPsidRowStatus;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  } 
  if((ROW_STATUS_VALID_MIN <= *data_out ) && (*data_out <= ROW_STATUS_VALID_MAX)){
      return RSEMIB_OK; 
  } else {
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } 
}
int32_t preserve_rsuMessageCountsByPsidRowStatus(int32_t index)
{
  if((index < RSU_PSID_TRACKED_STATS_MIN) || (RSU_PSID_TRACKED_STATS_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      prior_ntcip_1218_mib.rsuMessageCountsByPsidTable[index-1].rsuMessageCountsByPsidRowStatus = 
          ntcip_1218_mib.rsuMessageCountsByPsidTable[index-1].rsuMessageCountsByPsidRowStatus;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t action_rsuMessageCountsByPsidRowStatus(int32_t index, int32_t data_in)
{
  if((index < RSU_PSID_TRACKED_STATS_MIN) || (RSU_PSID_TRACKED_STATS_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if((data_in < ROW_STATUS_VALID_MIN) || (ROW_STATUS_VALID_MAX < data_in)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  }
  if(RSEMIB_OK == ntcip1218_lock()){  
      tmp_ntcip_1218_mib.rsuMessageCountsByPsidTable[index-1].rsuMessageCountsByPsidRowStatus = data_in;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t commit_rsuMessageCountsByPsidRowStatus(int32_t index)
{
  int32_t ret = RSEMIB_OK;
  char_t  command_buffer[100 + RSU_MSG_FILENAME_LENGTH_MAX]; /* If you need more than a 100 chars for your command then add them here. */

  if((index < RSU_PSID_TRACKED_STATS_MIN) || (RSU_PSID_TRACKED_STATS_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      ret = RSEMIB_BAD_INDEX;
  }
  if((RSEMIB_OK == ret) && (RSEMIB_OK == ntcip1218_lock())){
      DEBUGMSGTL((MY_NAME_EXTRA, "commit_rsuMessageCountsByPsidRowStatus: index=%d status=%d.\n", index,tmp_ntcip_1218_mib.rsuMessageCountsByPsidTable[index-1].rsuMessageCountsByPsidRowStatus));
      switch(tmp_ntcip_1218_mib.rsuMessageCountsByPsidTable[index-1].rsuMessageCountsByPsidRowStatus){
          case SNMP_ROW_CREATEANDGO:
              if(SNMP_ROW_NONEXISTENT == ntcip_1218_mib.rsuMessageCountsByPsidTable[index-1].rsuMessageCountsByPsidRowStatus) {
                  set_default_row_rsuMessageStats(&ntcip_1218_mib.rsuMessageCountsByPsidTable[index-1],index-1);
                  ntcip_1218_mib.rsuMessageCountsByPsidTable[index-1].rsuMessageCountsByPsidRowStatus = SNMP_ROW_ACTIVE;
                  if(RSEMIB_OK == (ret = update_messageStats_shm(&ntcip_1218_mib.rsuMessageCountsByPsidTable[index-1]))){
                       if(RSEMIB_OK != (ret = commit_rsuMessageStats_to_disk(&ntcip_1218_mib.rsuMessageCountsByPsidTable[index-1]))){
                           DEBUGMSGTL((MY_NAME, "commit_rsuMessageStats_to_disk: FAIL: index=%d dir=[%s].\n", index, ntcip_1218_mib.rsuMessageCountsByPsidTable[index-1].filePathName));
                           ret = RSEMIB_BAD_DATA;
                       } else {
                           /* Rebuild helper table before ack. */
                           if (RSEMIB_OK != (ret = rebuild_rsuMessageStats_live(&ntcip_1218_mib.rsuMessageCountsByPsidTable[0]))){
                               ret = RSEMIB_BAD_DATA;
                           }
                       }
                   } else {
                       ret = RSEMIB_BAD_DATA;
                   }
              } else {
                  /* If it exists already then do nothing. Dont send error back. */
                  //ret = RSEMIB_BAD_MODE;
              }
              break;
          case SNMP_ROW_CREATEANDWAIT:
              if(SNMP_ROW_NONEXISTENT == ntcip_1218_mib.rsuMessageCountsByPsidTable[index-1].rsuMessageCountsByPsidRowStatus) {
                  set_default_row_rsuMessageStats(&ntcip_1218_mib.rsuMessageCountsByPsidTable[index-1],index-1);
                  ntcip_1218_mib.rsuMessageCountsByPsidTable[index-1].rsuMessageCountsByPsidRowStatus = SNMP_ROW_NOTINSERVICE;
                  if(RSEMIB_OK == (ret = update_messageStats_shm(&ntcip_1218_mib.rsuMessageCountsByPsidTable[index-1]))){
                      if(RSEMIB_OK != (ret = commit_rsuMessageStats_to_disk(&ntcip_1218_mib.rsuMessageCountsByPsidTable[index-1]))){
                          DEBUGMSGTL((MY_NAME, "commit_rsuMessageStats_to_disk: FAIL: index=%d dir=[%s].\n", index, ntcip_1218_mib.rsuMessageCountsByPsidTable[index-1].filePathName));
                          ret = RSEMIB_BAD_DATA;
                      } else {
                          /* Rebuild helper table before ack. */
                          if (RSEMIB_OK != (ret = rebuild_rsuMessageStats_live(&ntcip_1218_mib.rsuMessageCountsByPsidTable[0]))){
                              ret = RSEMIB_BAD_DATA;
                          }
                      }
                   } else {
                       ret = RSEMIB_BAD_DATA;
                   }
              } else {
                  /* If it exists already then do nothing. Dont send error back. */
                  //ret = RSEMIB_BAD_MODE;
              }
              break;
          case SNMP_ROW_DESTROY: /* User deletes row from table. */
              if(SNMP_ROW_NONEXISTENT == ntcip_1218_mib.rsuMessageCountsByPsidTable[index-1].rsuMessageCountsByPsidRowStatus){
                  ret = RSEMIB_OK; /* If you try to DESTROY non-exixtent row that's an easy one. */
              } else {
                  if(RSEMIB_OK == (ret = update_messageStats_shm(&ntcip_1218_mib.rsuMessageCountsByPsidTable[index-1]))){
                      /* rm from RSU. */
                      memset(command_buffer,'\0',sizeof(command_buffer));
                      snprintf(command_buffer, sizeof(command_buffer), "/bin/rm -f %s", ntcip_1218_mib.rsuMessageCountsByPsidTable[index-1].filePathName);
                      DEBUGMSGTL((MY_NAME, "DESTROY: index=%d cmd=[%s].\n", index, command_buffer));
                      if(0 != system(command_buffer)){
                          ret = RSEMIB_BAD_DATA;
                      }
                      /* Careful to preserve index within table rows. Only call proper routine to clear. */
                      set_default_row_rsuMessageStats(&ntcip_1218_mib.rsuMessageCountsByPsidTable[index-1],index-1);
                      set_default_row_rsuMessageStats(&tmp_ntcip_1218_mib.rsuMessageCountsByPsidTable[index-1],index-1);
                      set_default_row_rsuMessageStats(&prior_ntcip_1218_mib.rsuMessageCountsByPsidTable[index-1],index-1);
                      if(RSEMIB_OK == (ret = update_messageStats_shm(&ntcip_1218_mib.rsuMessageCountsByPsidTable[index-1]))){
                          /* Rebuild helper table before ack. */
                          if (RSEMIB_OK != (ret = rebuild_rsuMessageStats_live(&ntcip_1218_mib.rsuMessageCountsByPsidTable[0]))){
                              ret = RSEMIB_BAD_DATA;
                          }
                      } else {
                          ret = RSEMIB_BAD_DATA;
                      }
                  }
              }
              break;
          case SNMP_ROW_ACTIVE:
              ntcip_1218_mib.rsuMessageCountsByPsidTable[index-1].rsuMessageCountsByPsidRowStatus = SNMP_ROW_ACTIVE;
              break;
          case SNMP_ROW_NOTINSERVICE:
              ntcip_1218_mib.rsuMessageCountsByPsidTable[index-1].rsuMessageCountsByPsidRowStatus = SNMP_ROW_NOTINSERVICE;
              break;
          case SNMP_ROW_NONEXISTENT:
          case SNMP_ROW_NOTREADY:
          default:
              ret = RSEMIB_BAD_MODE;
              break;
      }
      /* Clear tmp. */
      tmp_ntcip_1218_mib.rsuMessageCountsByPsidTable[index-1].rsuMessageCountsByPsidRowStatus = ROW_STATUS_VALID_DEFAULT;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      ret = RSEMIB_LOCK_FAIL;
  }
  return ret;
}
int32_t undo_rsuMessageCountsByPsidRowStatus(int32_t index)
{
  if((index < RSU_PSID_TRACKED_STATS_MIN) || (RSU_PSID_TRACKED_STATS_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuMessageCountsByPsidTable[index-1].rsuMessageCountsByPsidRowStatus = 
          prior_ntcip_1218_mib.rsuMessageCountsByPsidTable[index-1].rsuMessageCountsByPsidRowStatus;
      ntcip1218_unlock(); 
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}

/******************************************************************************
 * 5.13 Systems Statistics: { rsu 12 } : rsuCommRangeTable
 ******************************************************************************/

/* 
 * 5.13.1: TimeSincePowerOn: RO: Counter32(uint32_t): seconds since 5.17.2 rsuMode == Operate or Standby states. 
 * It's 136 years long so will never roll. If it does, call me. UTC does not go backwards unless problem.
 */
int32_t get_rsuTimeSincePowerOn(uint32_t * data_out)
{
  int32_t  ret = RSEMIB_OK;
  uint32_t now = (uint32_t)(mibUtilGetTimeInMs()/(uint64_t)1000);

  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      ret = RSEMIB_BAD_INPUT;
  } else {
      *data_out = 0;
      if(RSEMIB_OK == ntcip1218_lock()){
          if(0 == ntcip_1218_mib.rsuTimeSincePowerOn){
              if(rsuModeStatus_other != ntcip_1218_mib.rsuModeStatus) {
                  DEBUGMSGTL((MY_NAME_EXTRA, "get_rsuTimeSincePowerOn is zero but not rsuMode_other."));
                  set_ntcip_1218_error_states(RSEMIB_BAD_MODE);
                  ret = RSEMIB_BAD_MODE;
              }
          } else {
              if(ntcip_1218_mib.rsuTimeSincePowerOn <= now) {
                  *data_out = now - ntcip_1218_mib.rsuTimeSincePowerOn;
                  ret = RSEMIB_OK;
              } else {
                  DEBUGMSGTL((MY_NAME_EXTRA, "get_rsuTimeSincePowerOn now is in the past."));
                  set_ntcip_1218_error_states(RSEMIB_BAD_MODE);
                  ret = RSEMIB_BAD_MODE;
              }
          }
          ntcip1218_unlock();
      } else {
          set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
          ret = RSEMIB_LOCK_FAIL;
      }
  }
  return ret;
}
/* 5.13.2 */
static int32_t get_ambientTemp(int32_t * temp)
{
  int32_t ret = RSEMIB_SYSCALL_FAIL;
  int32_t fd;
  char_t buf[16];

  if(NULL == temp) {
      ret = RSEMIB_BAD_INPUT;
  } else {
      memset(buf,0x00,sizeof(buf));
      *temp = 0;    
      fd = open(AMBIENT_TEMP_FILE, O_SYNC | O_RDONLY);
      if (fd < 0) {
          DEBUGMSGTL((MY_NAME, "get_ambientTemp popen(%s) failed.",AMBIENT_TEMP_FILE));
      } else {
          if (0 != lseek(fd, 0, SEEK_SET)) {
              DEBUGMSGTL((MY_NAME, "get_ambientTemp lseek(%s) failed.",AMBIENT_TEMP_FILE));
          } else {
              if (-1 == read(fd, buf, sizeof(buf))) {
                  DEBUGMSGTL((MY_NAME, "get_ambientTemp read(%s) failed.",AMBIENT_TEMP_FILE));
              } else {
                  *temp = atoi((const char_t *)buf);
                  *temp = (*temp / 1000) - 64; /* Scale for device. */
                  if(-1 == close(fd)) {
                      DEBUGMSGTL((MY_NAME, "get_ambientTemp close(%s) failed.",AMBIENT_TEMP_FILE));         
                  } else {
                      ret = RSEMIB_OK;
                  }
              }
          }
      }
  }
  if(ret < RSEMIB_OK) {
      set_ntcip_1218_error_states(ret);
  }
  return ret;
}
int32_t get_rsuIntTemp(int32_t * data_out)
{
  int32_t ret = RSEMIB_OK;
  int32_t dummy = 0;
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      ret = RSEMIB_BAD_INPUT;
  } else {
      if(RSEMIB_OK == ntcip1218_lock()){
          if(RSEMIB_OK == (ret = get_ambientTemp(&dummy))) {
              if((RSU_TEMP_CELSIUS_MIN <= dummy) && (dummy <= RSU_TEMP_CELSIUS_MAX)){
                  ntcip_1218_mib.rsuIntTemp = dummy;
              } else {
                  ntcip_1218_mib.rsuIntTemp = RSU_TEMP_LOW_CELSIUS_MIN; /* Unknown. */
                  ret = RSEMIB_BAD_DATA;
              }
          } else {
              ntcip_1218_mib.rsuIntTemp = RSU_TEMP_LOW_CELSIUS_MIN; /* Unknown. */
              ret = RSEMIB_BAD_DATA;
          }
          *data_out = ntcip_1218_mib.rsuIntTemp;
          ntcip1218_unlock();
      } else {
          ret = RSEMIB_LOCK_FAIL;
          *data_out = RSU_TEMP_LOW_CELSIUS_MIN;
      }
  }
  if(ret < RSEMIB_OK) {
      set_ntcip_1218_error_states(ret);
  }
  return ret;
}

/* 5.13.3 */
int32_t get_rsuIntTempLowThreshold(int32_t * data_out)
{
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      *data_out = ntcip_1218_mib.rsuIntTempLowThreshold;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
  if((RSU_TEMP_LOW_CELSIUS_MIN <= *data_out) && (*data_out <= RSU_TEMP_LOW_CELSIUS_MAX)){
      return RSEMIB_OK; 
  } else {
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  }
}

/* 5.13.4 */
int32_t get_rsuIntTempHighThreshold(int32_t * data_out)
{
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      *data_out = ntcip_1218_mib.rsuIntTempHighThreshold;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
  if((RSU_TEMP_HIGH_CELSIUS_MIN <= *data_out) && (*data_out <= RSU_TEMP_HIGH_CELSIUS_MAX)){
      return RSEMIB_OK; 
  } else {
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } 
}

/* 5.13.5 */
int32_t get_maxRsuCommRange(int32_t * data_out)
{
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      *data_out = ntcip_1218_mib.maxRsuCommRange;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
  if((RSU_COMM_RANGE_MIN <= *data_out) && (*data_out <= RSU_COMM_RANGE_MAX)){
      return RSEMIB_OK; 
  } else {
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } 
}

/* 5.13.6.1 */
int32_t get_rsuCommRangeIndex(int32_t index, int32_t * data_out)
{
  if((index < RSU_COMM_RANGE_MIN) || (RSU_COMM_RANGE_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      *data_out = ntcip_1218_mib.rsuCommRangeTable[index - 1].rsuCommRangeIndex;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
  if((*data_out < RSU_COMM_RANGE_MIN) || (RSU_COMM_RANGE_MAX < *data_out)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } 
  return RSEMIB_OK;
}

/* 5.13.6.2 */
int32_t get_rsuCommRangeSector(int32_t index, int32_t * data_out)
{
  if((index < RSU_COMM_RANGE_MIN) || (RSU_COMM_RANGE_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      *data_out = ntcip_1218_mib.rsuCommRangeTable[index-1].rsuCommRangeSector;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
  if((*data_out < RSU_COMM_RANGE_SECTOR_MIN) || (RSU_COMM_RANGE_SECTOR_MAX < *data_out)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } 
  return RSEMIB_OK;
}

int32_t preserve_rsuCommRangeSector(int32_t index)
{
  if((index < RSU_COMM_RANGE_MIN) || (RSU_COMM_RANGE_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      prior_ntcip_1218_mib.rsuCommRangeTable[index-1].rsuCommRangeSector = ntcip_1218_mib.rsuCommRangeTable[index-1].rsuCommRangeSector;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t action_rsuCommRangeSector(int32_t index, int32_t data_in)
{
  if((index < RSU_COMM_RANGE_MIN) || (RSU_COMM_RANGE_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if((data_in < RSU_COMM_RANGE_SECTOR_MIN) || (RSU_COMM_RANGE_SECTOR_MAX < data_in)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  }
  if(RSEMIB_OK == ntcip1218_lock()){  
      tmp_ntcip_1218_mib.rsuCommRangeTable[index-1].rsuCommRangeSector = data_in;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t commit_rsuCommRangeSector(int32_t index)
{
  int32_t ret = RSEMIB_OK;
  if((index < RSU_COMM_RANGE_MIN) || (RSU_COMM_RANGE_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuCommRangeTable[index-1].rsuCommRangeSector = tmp_ntcip_1218_mib.rsuCommRangeTable[index-1].rsuCommRangeSector;
      tmp_ntcip_1218_mib.rsuCommRangeTable[index-1].rsuCommRangeSector = RSU_COMM_RANGE_SECTOR_MIN;
      if(RSEMIB_OK != (ret = commit_commRange_to_disk(&ntcip_1218_mib.rsuCommRangeTable[index-1]))){
          DEBUGMSGTL((MY_NAME, "commit_commRange_to_disk: FAIL: index=%d dir=[%s].\n", index, ntcip_1218_mib.rsuCommRangeTable[index-1].filePathName));
          ret = RSEMIB_BAD_DATA;
      }
      ntcip1218_unlock();
      return ret;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t undo_rsuCommRangeSector(int32_t index)
{
  if((index < RSU_COMM_RANGE_MIN) || (RSU_COMM_RANGE_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuCommRangeTable[index-1].rsuCommRangeSector = prior_ntcip_1218_mib.rsuCommRangeTable[index-1].rsuCommRangeSector;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}

/* 5.13.6.3 */
int32_t get_rsuCommRangeMsgId(int32_t index, int32_t * data_out)
{
  if((index < RSU_COMM_RANGE_MIN) || (RSU_COMM_RANGE_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      *data_out = ntcip_1218_mib.rsuCommRangeTable[index-1].rsuCommRangeMsgId;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
  if((*data_out < RSU_COMM_RANGE_MSG_ID_MIN) || (RSU_COMM_RANGE_MSG_ID_MAX < *data_out)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } 
  return RSEMIB_OK;
}
int32_t preserve_rsuCommRangeMsgId(int32_t index)
{
  if((index < RSU_COMM_RANGE_MIN) || (RSU_COMM_RANGE_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      prior_ntcip_1218_mib.rsuCommRangeTable[index-1].rsuCommRangeMsgId = ntcip_1218_mib.rsuCommRangeTable[index-1].rsuCommRangeMsgId;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t action_rsuCommRangeMsgId(int32_t index, int32_t data_in)
{
  if((index < RSU_COMM_RANGE_MIN) || (RSU_COMM_RANGE_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if((data_in < RSU_COMM_RANGE_MSG_ID_MIN) || (RSU_COMM_RANGE_MSG_ID_MAX < data_in)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  }
  if(RSEMIB_OK == ntcip1218_lock()){  
      tmp_ntcip_1218_mib.rsuCommRangeTable[index-1].rsuCommRangeMsgId = data_in;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t commit_rsuCommRangeMsgId(int32_t index)
{
  int32_t ret = RSEMIB_OK;
  if((index < RSU_COMM_RANGE_MIN) || (RSU_COMM_RANGE_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuCommRangeTable[index-1].rsuCommRangeMsgId = tmp_ntcip_1218_mib.rsuCommRangeTable[index-1].rsuCommRangeMsgId;
      tmp_ntcip_1218_mib.rsuCommRangeTable[index-1].rsuCommRangeMsgId = RSU_COMM_RANGE_MSG_ID_MIN;
      if(RSEMIB_OK != (ret = commit_commRange_to_disk(&ntcip_1218_mib.rsuCommRangeTable[index-1]))){
          DEBUGMSGTL((MY_NAME, "commit_commRange_to_disk: FAIL: index=%d dir=[%s].\n", index, ntcip_1218_mib.rsuCommRangeTable[index-1].filePathName));
          ret = RSEMIB_BAD_DATA;
      }
      ntcip1218_unlock();
      return ret;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t undo_rsuCommRangeMsgId(int32_t index)
{
  if((index < RSU_COMM_RANGE_MIN) || (RSU_COMM_RANGE_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuCommRangeTable[index-1].rsuCommRangeMsgId = prior_ntcip_1218_mib.rsuCommRangeTable[index-1].rsuCommRangeMsgId;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}

/* 5.13.6.4 */
int32_t get_rsuCommRangeFilterType(int32_t index, int32_t * data_out)
{
  if((index < RSU_COMM_RANGE_MIN) || (RSU_COMM_RANGE_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      *data_out = ntcip_1218_mib.rsuCommRangeTable[index-1].rsuCommRangeFilterType;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
  if((*data_out < RSU_COMM_RANGE_FILTER_TYPE_MIN) || (RSU_COMM_RANGE_FILTER_TYPE_MAX < *data_out)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } 
  return RSEMIB_OK;
}
int32_t preserve_rsuCommRangeFilterType(int32_t index)
{
  if((index < RSU_COMM_RANGE_MIN) || (RSU_COMM_RANGE_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      prior_ntcip_1218_mib.rsuCommRangeTable[index-1].rsuCommRangeFilterType = 
          ntcip_1218_mib.rsuCommRangeTable[index-1].rsuCommRangeFilterType;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t action_rsuCommRangeFilterType(int32_t index, int32_t data_in)
{
  if((index < RSU_COMM_RANGE_MIN) || (RSU_COMM_RANGE_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if((data_in < RSU_COMM_RANGE_FILTER_TYPE_MIN) || (RSU_COMM_RANGE_FILTER_TYPE_MAX < data_in)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  }
  if(RSEMIB_OK == ntcip1218_lock()){  
      tmp_ntcip_1218_mib.rsuCommRangeTable[index-1].rsuCommRangeFilterType = data_in;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t commit_rsuCommRangeFilterType(int32_t index)
{
  int32_t ret = RSEMIB_OK;
  if((index < RSU_COMM_RANGE_MIN) || (RSU_COMM_RANGE_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuCommRangeTable[index-1].rsuCommRangeFilterType = 
          tmp_ntcip_1218_mib.rsuCommRangeTable[index-1].rsuCommRangeFilterType;
      tmp_ntcip_1218_mib.rsuCommRangeTable[index-1].rsuCommRangeFilterType = RSU_COMM_RANGE_FILTER_TYPE_MIN;
      if(RSEMIB_OK != (ret = commit_commRange_to_disk(&ntcip_1218_mib.rsuCommRangeTable[index-1]))){
          DEBUGMSGTL((MY_NAME, "commit_commRange_to_disk: FAIL: index=%d dir=[%s].\n", index, ntcip_1218_mib.rsuCommRangeTable[index-1].filePathName));
          ret = RSEMIB_BAD_DATA;
      }
      ntcip1218_unlock();
      return ret;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t undo_rsuCommRangeFilterType(int32_t index)
{
  if((index < RSU_COMM_RANGE_MIN) || (RSU_COMM_RANGE_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuCommRangeTable[index-1].rsuCommRangeFilterType = 
          prior_ntcip_1218_mib.rsuCommRangeTable[index-1].rsuCommRangeFilterType;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}

/* 5.13.6.5 */
int32_t get_rsuCommRangeFilterValue(int32_t index, int32_t * data_out)
{
  if((index < RSU_COMM_RANGE_MIN) || (RSU_COMM_RANGE_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      *data_out = ntcip_1218_mib.rsuCommRangeTable[index-1].rsuCommRangeFilterValue;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
  if((*data_out < RSU_COMM_RANGE_FILTER_VALUE_MIN) || (RSU_COMM_RANGE_FILTER_VALUE_MAX < *data_out)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } 
  return RSEMIB_OK;
}
int32_t preserve_rsuCommRangeFilterValue(int32_t index)
{
  if((index < RSU_COMM_RANGE_MIN) || (RSU_COMM_RANGE_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      prior_ntcip_1218_mib.rsuCommRangeTable[index-1].rsuCommRangeFilterValue = 
          ntcip_1218_mib.rsuCommRangeTable[index-1].rsuCommRangeFilterValue;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t action_rsuCommRangeFilterValue(int32_t index, int32_t data_in)
{
  if((index < RSU_COMM_RANGE_MIN) || (RSU_COMM_RANGE_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if((data_in < RSU_COMM_RANGE_FILTER_VALUE_MIN) || (RSU_COMM_RANGE_FILTER_VALUE_MAX < data_in)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  }
  if(RSEMIB_OK == ntcip1218_lock()){  
      tmp_ntcip_1218_mib.rsuCommRangeTable[index-1].rsuCommRangeFilterValue = data_in;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t commit_rsuCommRangeFilterValue(int32_t index)
{
  int32_t ret = RSEMIB_OK;
  if((index < RSU_COMM_RANGE_MIN) || (RSU_COMM_RANGE_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuCommRangeTable[index-1].rsuCommRangeFilterValue = 
          tmp_ntcip_1218_mib.rsuCommRangeTable[index-1].rsuCommRangeFilterValue;
      tmp_ntcip_1218_mib.rsuCommRangeTable[index-1].rsuCommRangeFilterValue = RSU_COMM_RANGE_FILTER_VALUE_MIN;
      if(RSEMIB_OK != (ret = commit_commRange_to_disk(&ntcip_1218_mib.rsuCommRangeTable[index-1]))){
          DEBUGMSGTL((MY_NAME, "commit_commRange_to_disk: FAIL: index=%d dir=[%s].\n", index, ntcip_1218_mib.rsuCommRangeTable[index-1].filePathName));
          ret = RSEMIB_BAD_DATA;
      }
      ntcip1218_unlock();
      return ret;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t undo_rsuCommRangeFilterValue(int32_t index)
{
  if((index < RSU_COMM_RANGE_MIN) || (RSU_COMM_RANGE_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuCommRangeTable[index-1].rsuCommRangeFilterValue = 
          prior_ntcip_1218_mib.rsuCommRangeTable[index-1].rsuCommRangeFilterValue;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}

/* 5.13.6.6 */
int32_t get_rsuCommRange1Min(int32_t index, int32_t * data_out)
{
  if((index < RSU_COMM_RANGE_MIN) || (RSU_COMM_RANGE_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      *data_out = ntcip_1218_mib.rsuCommRangeTable[index-1].rsuCommRange1Min;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
  if((*data_out < RSU_COMM_RANGE_MINUTES_MIN) || (RSU_COMM_RANGE_MINUTES_MAX < *data_out)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } 
  return RSEMIB_OK;
}

/* 5.13.6.7 */
int32_t get_rsuCommRange5Min(int32_t index, int32_t * data_out)
{
  if((index < RSU_COMM_RANGE_MIN) || (RSU_COMM_RANGE_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      *data_out = ntcip_1218_mib.rsuCommRangeTable[index-1].rsuCommRange5Min;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
  if((*data_out < RSU_COMM_RANGE_MINUTES_MIN) || (RSU_COMM_RANGE_MINUTES_MAX < *data_out)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } 
  return RSEMIB_OK;
}

/* 5.13.6.8 */
int32_t get_rsuCommRange15Min(int32_t index, int32_t * data_out)
{
  if((index < RSU_COMM_RANGE_MIN) || (RSU_COMM_RANGE_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      *data_out = ntcip_1218_mib.rsuCommRangeTable[index-1].rsuCommRange15Min;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
  if((*data_out < RSU_COMM_RANGE_MINUTES_MIN) || (RSU_COMM_RANGE_MINUTES_MAX < *data_out)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } 
  return RSEMIB_OK;
}
/* 5.13.6.9 */
int32_t get_rsuCommRangeAvg1Min(int32_t index, int32_t * data_out)
{
  if((index < RSU_COMM_RANGE_MIN) || (RSU_COMM_RANGE_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      *data_out = ntcip_1218_mib.rsuCommRangeTable[index-1].rsuCommRangeAvg1Min;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
  if((*data_out < RSU_COMM_RANGE_MINUTES_MIN) || (RSU_COMM_RANGE_MINUTES_MAX < *data_out)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } 
  return RSEMIB_OK;
}

/* 5.13.6.10 */
int32_t get_rsuCommRangeAvg5Min(int32_t index, int32_t * data_out)
{
  if((index < RSU_COMM_RANGE_MIN) || (RSU_COMM_RANGE_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      *data_out = ntcip_1218_mib.rsuCommRangeTable[index-1].rsuCommRangeAvg5Min;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
  if((*data_out < RSU_COMM_RANGE_MINUTES_MIN) || (RSU_COMM_RANGE_MINUTES_MAX < *data_out)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } 
  return RSEMIB_OK;
}

/* 5.13.6.11 */
int32_t get_rsuCommRangeAvg15Min(int32_t index, int32_t * data_out)
{
  if((index < RSU_COMM_RANGE_MIN) || (RSU_COMM_RANGE_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      *data_out = ntcip_1218_mib.rsuCommRangeTable[index-1].rsuCommRangeAvg15Min;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
  if((*data_out < RSU_COMM_RANGE_MINUTES_MIN) || (RSU_COMM_RANGE_MINUTES_MAX < *data_out)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } 
  return RSEMIB_OK;
}

/* 5.13.6.11 RW: */
int32_t get_rsuCommRangeStatus(int32_t index, int32_t * data_out)
{
  if((index < RSU_COMM_RANGE_MIN) || (RSU_COMM_RANGE_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      *data_out = ntcip_1218_mib.rsuCommRangeTable[index-1].rsuCommRangeStatus;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  } 
  if((ROW_STATUS_VALID_MIN <= *data_out ) && (*data_out <= ROW_STATUS_VALID_MAX)){
      return RSEMIB_OK; 
  } else {
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } 
}
int32_t preserve_rsuCommRangeStatus(int32_t index)
{
  if((index < RSU_COMM_RANGE_MIN) || (RSU_COMM_RANGE_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      set_default_row_rsuCommRange(&tmp_ntcip_1218_mib.rsuCommRangeTable[index-1],index-1);
      prior_ntcip_1218_mib.rsuCommRangeTable[index-1].rsuCommRangeStatus = ntcip_1218_mib.rsuCommRangeTable[index-1].rsuCommRangeStatus;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t action_rsuCommRangeStatus(int32_t index, int32_t data_in)
{
  if((index < RSU_COMM_RANGE_MIN) || (RSU_COMM_RANGE_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if((data_in < ROW_STATUS_VALID_MIN) || (ROW_STATUS_VALID_MAX < data_in)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  }
  if(RSEMIB_OK == ntcip1218_lock()){  
      tmp_ntcip_1218_mib.rsuCommRangeTable[index-1].rsuCommRangeStatus = data_in;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t commit_rsuCommRangeStatus(int32_t index)
{
  char_t  command_buffer[100 + RSU_MSG_FILENAME_LENGTH_MAX]; /* If you need more than a 100 chars for your command then add them here. */

  int32_t ret = RSEMIB_OK;
  if((index < RSU_COMM_RANGE_SECTOR_MIN) || (RSU_COMM_RANGE_SECTOR_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      ret = RSEMIB_BAD_INDEX;
  }
  if((RSEMIB_OK == ret) && (RSEMIB_OK == ntcip1218_lock())){
      DEBUGMSGTL((MY_NAME_EXTRA, "commit_rsuCommRangeStatus: index=%d status=%d.\n", index,tmp_ntcip_1218_mib.rsuCommRangeTable[index-1].rsuCommRangeStatus));
      switch(tmp_ntcip_1218_mib.rsuCommRangeTable[index-1].rsuCommRangeStatus){
          case SNMP_ROW_CREATEANDGO:
              if(SNMP_ROW_NONEXISTENT == ntcip_1218_mib.rsuCommRangeTable[index-1].rsuCommRangeStatus) {
                  set_default_row_rsuCommRange(&ntcip_1218_mib.rsuCommRangeTable[index-1],index-1);
                  ntcip_1218_mib.rsuCommRangeTable[index-1].rsuCommRangeStatus = SNMP_ROW_ACTIVE;
                  if(RSEMIB_OK != commit_commRange_to_disk(&ntcip_1218_mib.rsuCommRangeTable[index-1])){
                      DEBUGMSGTL((MY_NAME, "commit_commRange_to_disk: FAIL: index=%d dir=[%s].\n", index, ntcip_1218_mib.rsuCommRangeTable[index-1].filePathName));
                      ret = RSEMIB_BAD_DATA;
                  } else {
                      /* Rebuild helper table before ack. */
                      if (RSEMIB_OK != (ret = rebuild_rsuCommRange_live(&ntcip_1218_mib.rsuCommRangeTable[0]))){
                          ret = RSEMIB_BAD_DATA;
                      }
                  }
              } else {
                  /* If it exists already then do nothing. Dont send error back. */
                  //ret = RSEMIB_BAD_MODE;
              }
              break;
          case SNMP_ROW_CREATEANDWAIT:
              if(SNMP_ROW_NONEXISTENT == ntcip_1218_mib.rsuCommRangeTable[index-1].rsuCommRangeStatus) {
                  set_default_row_rsuCommRange(&ntcip_1218_mib.rsuCommRangeTable[index-1],index-1);
                  ntcip_1218_mib.rsuCommRangeTable[index-1].rsuCommRangeStatus = SNMP_ROW_NOTINSERVICE;
                  if(RSEMIB_OK != commit_commRange_to_disk(&ntcip_1218_mib.rsuCommRangeTable[index-1])){
                      DEBUGMSGTL((MY_NAME, "commit_commRange_to_disk: FAIL: index=%d dir=[%s].\n", index, ntcip_1218_mib.rsuCommRangeTable[index-1].filePathName));
                      ret = RSEMIB_BAD_DATA;
                  } else {
                      /* Rebuild helper table before ack. */
                      if (RSEMIB_OK != (ret = rebuild_rsuCommRange_live(&ntcip_1218_mib.rsuCommRangeTable[0]))){
                          ret = RSEMIB_BAD_DATA;
                      }
                  }
              } else {
                  /* If it exists already then do nothing. Dont send error back. */
                  //ret = RSEMIB_BAD_MODE;
              }
              break;
          case SNMP_ROW_DESTROY: /* User deletes row from table. */
              if(SNMP_ROW_NONEXISTENT == ntcip_1218_mib.rsuCommRangeTable[index-1].rsuCommRangeStatus){
                  ret = RSEMIB_OK; /* If you try to DESTROY non-exixtent row that's an easy one. */
              } else {
                  /* rm from RSU. */
                  memset(command_buffer,'\0',sizeof(command_buffer));
                  snprintf(command_buffer, sizeof(command_buffer), "/bin/rm -f %s", ntcip_1218_mib.rsuCommRangeTable[index-1].filePathName);
                  DEBUGMSGTL((MY_NAME, "DESTROY: index=%d cmd=[%s].\n", index, command_buffer));
                  if(0 != system(command_buffer)){
                      ret = RSEMIB_BAD_DATA;
                  }
                  /* Careful to preserve index within table rows. Only call proper routine to clear. */
                  set_default_row_rsuCommRange(&ntcip_1218_mib.rsuCommRangeTable[index-1],index-1);
                  set_default_row_rsuCommRange(&tmp_ntcip_1218_mib.rsuCommRangeTable[index-1],index-1);
                  set_default_row_rsuCommRange(&prior_ntcip_1218_mib.rsuCommRangeTable[index-1],index-1);

                  /* Rebuild helper table before ack. */
                  if (RSEMIB_OK != (ret = rebuild_rsuCommRange_live(&ntcip_1218_mib.rsuCommRangeTable[0]))){
                      ret = RSEMIB_BAD_DATA;
                  }
              }
              break;
          case SNMP_ROW_ACTIVE:
              ntcip_1218_mib.rsuCommRangeTable[index-1].rsuCommRangeStatus = SNMP_ROW_ACTIVE;
              break;
          case SNMP_ROW_NOTINSERVICE:
              ntcip_1218_mib.rsuCommRangeTable[index-1].rsuCommRangeStatus = SNMP_ROW_NOTINSERVICE;
              break;
          case SNMP_ROW_NONEXISTENT:
          case SNMP_ROW_NOTREADY:
          default:
              ret = RSEMIB_BAD_MODE;
              break;
      }
      /* Clear tmp. */
      tmp_ntcip_1218_mib.rsuCommRangeTable[index-1].rsuCommRangeStatus = ROW_STATUS_VALID_DEFAULT;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      ret = RSEMIB_LOCK_FAIL;
  }
  return ret;
}
int32_t undo_rsuCommRangeStatus(int32_t index)
{
  if((index < RSU_COMM_RANGE_MIN) || (RSU_COMM_RANGE_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuCommRangeTable[index-1].rsuCommRangeStatus = prior_ntcip_1218_mib.rsuCommRangeTable[index-1].rsuCommRangeStatus;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}

/******************************************************************************
 * 5.14 System Description: { rsu 13 }
 ******************************************************************************/

/* 5.14.1 */
int32_t get_rsuMibVersion(uint8_t * data_out)
{
  int32_t size = 0;
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      size = ntcip_1218_mib.rsuMibVersion_length;
      memcpy(data_out,ntcip_1218_mib.rsuMibVersion,RSUMIB_VERSION_LENGTH_MAX);
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
  if((size < RSUMIB_VERSION_LENGTH_MIN) || (RSUMIB_VERSION_LENGTH_MAX < size)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_DATA;
  } 
  return size;
}

/* 5.14.2 */
int32_t get_rsuFirmwareVersion(uint8_t * data_out)
{
  int32_t size = 0;
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      size = ntcip_1218_mib.rsuFirmwareVersion_length;
      memcpy(data_out,ntcip_1218_mib.rsuFirmwareVersion,RSU_FIRMWARE_VERSION_LENGTH_MAX);
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
  if((size < RSU_FIRMWARE_VERSION_LENGTH_MIN) || (RSU_FIRMWARE_VERSION_LENGTH_MAX < size)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_DATA;
  } 
  return size;
}

/* 5.14.3 */
int32_t get_rsuLocationDesc(uint8_t * data_out)
{
  int32_t size = 0;
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      size = ntcip_1218_mib.rsuLocationDesc_length;
      memcpy(data_out,ntcip_1218_mib.rsuLocationDesc,RSU_LOCATION_DESC_LENGTH_MAX);
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
  if((size < RSU_LOCATION_DESC_LENGTH_MIN) || (RSU_LOCATION_DESC_LENGTH_MAX < size)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  }
  return size;
} 
int32_t preserve_rsuLocationDesc(void)
{
  if(RSEMIB_OK == ntcip1218_lock()){
      memcpy(prior_ntcip_1218_mib.rsuLocationDesc, ntcip_1218_mib.rsuLocationDesc, RSU_LOCATION_DESC_LENGTH_MAX);
      prior_ntcip_1218_mib.rsuLocationDesc_length = ntcip_1218_mib.rsuLocationDesc_length;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t action_rsuLocationDesc(uint8_t * data_in, int32_t length)
{
  if((NULL == data_in) && (0 != length)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if((length < RSU_LOCATION_DESC_LENGTH_MIN) || (RSU_LOCATION_DESC_LENGTH_MAX < length)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  }  
  if(RSEMIB_OK == ntcip1218_lock()){  
      memset(tmp_ntcip_1218_mib.rsuLocationDesc, 0x0, RSU_LOCATION_DESC_LENGTH_MAX);
      if(NULL != data_in){
          memcpy(tmp_ntcip_1218_mib.rsuLocationDesc, data_in, length);
      }
      tmp_ntcip_1218_mib.rsuLocationDesc_length = length;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t commit_rsuLocationDesc(void)
{
  int32_t ret = RSEMIB_OK;
  SystemDescription_t sysDesc;

  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuLocationDesc_length = tmp_ntcip_1218_mib.rsuLocationDesc_length;
      memcpy(ntcip_1218_mib.rsuLocationDesc, tmp_ntcip_1218_mib.rsuLocationDesc, RSU_LOCATION_DESC_LENGTH_MAX);
      if(RSEMIB_OK == (copy_sysDesc_to_skinny(&ntcip_1218_mib, &sysDesc))){
          if(RSEMIB_OK != commit_sysDesc_to_disk(&sysDesc)) {
              ret = RSEMIB_BAD_DATA;
          }
      } else {
          ret = RSEMIB_BAD_DATA;
      }
      memset(tmp_ntcip_1218_mib.rsuLocationDesc, 0x0, RSU_LOCATION_DESC_LENGTH_MAX);
      tmp_ntcip_1218_mib.rsuLocationDesc_length = 0;
      ntcip1218_unlock();
  } else {
      ret = RSEMIB_LOCK_FAIL;
  }
  if(ret < RSEMIB_OK)
      set_ntcip_1218_error_states(ret);
  return ret;
}
int32_t undo_rsuLocationDesc(void)
{
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuLocationDesc_length = prior_ntcip_1218_mib.rsuLocationDesc_length;
      memcpy(ntcip_1218_mib.rsuLocationDesc, prior_ntcip_1218_mib.rsuLocationDesc, RSU_LOCATION_DESC_LENGTH_MAX);
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}

/* 5.14.4 */
int32_t get_rsuID(uint8_t * data_out)
{
  int32_t size = 0;
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      if(0 == (size = nor_read_deviceId(ntcip_1218_mib.rsuID))) {
          memset(ntcip_1218_mib.rsuID, '.', RSU_ID_LENGTH_MAX);
          size = ntcip_1218_mib.rsuID_length = RSU_ID_LENGTH_MAX;
      }
      memcpy(data_out,ntcip_1218_mib.rsuID,RSU_ID_LENGTH_MAX);
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
  if((size < RSU_ID_LENGTH_MIN) || (RSU_ID_LENGTH_MAX < size)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  }
  return size;
} 
int32_t preserve_rsuID(void)
{
  if(RSEMIB_OK == ntcip1218_lock()){
      prior_ntcip_1218_mib.rsuID_length = ntcip_1218_mib.rsuID_length;
      memcpy(prior_ntcip_1218_mib.rsuID, ntcip_1218_mib.rsuID, RSU_ID_LENGTH_MAX);
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t action_rsuID(uint8_t * data_in, int32_t length)
{
  if((NULL == data_in) && (0 != length)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if((length < RSU_ID_LENGTH_MIN) || (RSU_ID_LENGTH_MAX < length)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  }  
  if(RSEMIB_OK == ntcip1218_lock()){
      tmp_ntcip_1218_mib.rsuID_length = length;
      memset(tmp_ntcip_1218_mib.rsuID, 0x0, RSU_ID_LENGTH_MAX);
      if(NULL != data_in){
          memcpy(tmp_ntcip_1218_mib.rsuID, data_in, RSU_ID_LENGTH_MAX);
      }
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t commit_rsuID(void)
{
  int32_t ret = RSEMIB_OK;

  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuID_length = tmp_ntcip_1218_mib.rsuID_length;
      memcpy(ntcip_1218_mib.rsuID, tmp_ntcip_1218_mib.rsuID, RSU_ID_LENGTH_MAX);
      if(RSEMIB_OK != (nor_write_deviceId(ntcip_1218_mib.rsuID))){
          ret = RSEMIB_BAD_DATA;
      }
      memset(tmp_ntcip_1218_mib.rsuID, 0x0, RSU_ID_LENGTH_MAX);
      tmp_ntcip_1218_mib.rsuID_length = 0;
      ntcip1218_unlock();
  } else {
      ret = RSEMIB_LOCK_FAIL;
  }
  if(ret < RSEMIB_OK)
      set_ntcip_1218_error_states(ret);
  return ret;
}
int32_t undo_rsuID(void)
{
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuID_length = prior_ntcip_1218_mib.rsuID_length;
      memcpy(ntcip_1218_mib.rsuID, prior_ntcip_1218_mib.rsuID, RSU_ID_LENGTH_MAX);
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}

/* 5.14.5 */
int32_t get_rsuLocationLat(int32_t * data_out)
{
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      *data_out = ntcip_1218_mib.rsuLocationLat;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  } 
  if((RSU_LAT_MIN <= *data_out) && (*data_out <= RSU_LAT_MAX)){
      return RSEMIB_OK; 
  } else {
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } 
}
int32_t preserve_rsuLocationLat(void)
{
  if(RSEMIB_OK == ntcip1218_lock()){
      prior_ntcip_1218_mib.rsuLocationLat = ntcip_1218_mib.rsuLocationLat;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t action_rsuLocationLat(int32_t data_in)
{
  if((data_in < RSU_LAT_MIN) || (RSU_LAT_MAX < data_in)){ 
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  }
  if(RSEMIB_OK == ntcip1218_lock()){  
      tmp_ntcip_1218_mib.rsuLocationLat = data_in;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t commit_rsuLocationLat(void)
{
  int32_t ret = RSEMIB_OK;
  SystemDescription_t sysDesc;

  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuLocationLat = tmp_ntcip_1218_mib.rsuLocationLat;
      if(RSEMIB_OK == (copy_sysDesc_to_skinny(&ntcip_1218_mib, &sysDesc))){
          if(RSEMIB_OK != commit_sysDesc_to_disk(&sysDesc)) {
              ret = RSEMIB_BAD_DATA;
          }
      } else {
          ret = RSEMIB_BAD_DATA;
      }
      tmp_ntcip_1218_mib.rsuLocationLat = RSU_LAT_UNKNOWN;
      ntcip1218_unlock();
  } else {
      ret = RSEMIB_LOCK_FAIL;
  }
  if(ret < RSEMIB_OK)
      set_ntcip_1218_error_states(ret);
  return ret;
}
int32_t undo_rsuLocationLat(void)
{
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuLocationLat = prior_ntcip_1218_mib.rsuLocationLat;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}

/* 5.14.6 */
int32_t get_rsuLocationLon(int32_t * data_out)
{
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      *data_out = ntcip_1218_mib.rsuLocationLon;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  } 
  if((RSU_LON_MIN <= *data_out) && (*data_out <= RSU_LON_MAX)){
      return RSEMIB_OK; 
  } else {
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } 
}
int32_t preserve_rsuLocationLon(void)
{
  if(RSEMIB_OK == ntcip1218_lock()){
      prior_ntcip_1218_mib.rsuLocationLon = ntcip_1218_mib.rsuLocationLon;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t action_rsuLocationLon(int32_t data_in)
{
  if((data_in < RSU_LON_MIN) || (RSU_LON_MAX < data_in)){ 
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  }
  if(RSEMIB_OK == ntcip1218_lock()){  
      tmp_ntcip_1218_mib.rsuLocationLon = data_in;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t commit_rsuLocationLon(void)
{
  int32_t ret = RSEMIB_OK;
  SystemDescription_t sysDesc;

  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuLocationLon = tmp_ntcip_1218_mib.rsuLocationLon;
      if(RSEMIB_OK == (copy_sysDesc_to_skinny(&ntcip_1218_mib, &sysDesc))){
          if(RSEMIB_OK != commit_sysDesc_to_disk(&sysDesc)) {
              ret = RSEMIB_BAD_DATA;
          }
      } else {
          ret = RSEMIB_BAD_DATA;
      }
      tmp_ntcip_1218_mib.rsuLocationLon = RSU_LON_UNKNOWN;
      ntcip1218_unlock();
  } else {
      ret = RSEMIB_LOCK_FAIL;
  }
  if(ret < RSEMIB_OK)
      set_ntcip_1218_error_states(ret);
  return ret;
}
int32_t undo_rsuLocationLon(void)
{
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuLocationLon = prior_ntcip_1218_mib.rsuLocationLon;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}

/* 5.14.7 */
int32_t get_rsuLocationElv(int32_t * data_out)
{
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      *data_out = ntcip_1218_mib.rsuLocationElv;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  } 
  if((RSU_ELV_MIN <= *data_out) && (*data_out <= RSU_ELV_MAX)){
      return RSEMIB_OK; 
  } else {
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } 
}
int32_t preserve_rsuLocationElv(void)
{
  if(RSEMIB_OK == ntcip1218_lock()){
      prior_ntcip_1218_mib.rsuLocationElv = ntcip_1218_mib.rsuLocationElv;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t action_rsuLocationElv(int32_t data_in)
{
  if((data_in < RSU_ELV_MIN) || (RSU_ELV_MAX < data_in)){ 
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  }
  if(RSEMIB_OK == ntcip1218_lock()){  
      tmp_ntcip_1218_mib.rsuLocationElv = data_in;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }

}
int32_t commit_rsuLocationElv(void)
{
  int32_t ret = RSEMIB_OK;
  SystemDescription_t sysDesc;

  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuLocationElv = tmp_ntcip_1218_mib.rsuLocationElv;
      if(RSEMIB_OK == (copy_sysDesc_to_skinny(&ntcip_1218_mib, &sysDesc))){
          if(RSEMIB_OK != commit_sysDesc_to_disk(&sysDesc)) {
              ret = RSEMIB_BAD_DATA;
          }
      } else {
          ret = RSEMIB_BAD_DATA;
      }
      tmp_ntcip_1218_mib.rsuLocationElv = RSU_ELV_UNKNOWN;
      ntcip1218_unlock();
  } else {
      ret = RSEMIB_LOCK_FAIL;
  }
  if(ret < RSEMIB_OK)
      set_ntcip_1218_error_states(ret);
  return ret;
}
int32_t undo_rsuLocationElv(void)
{
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuLocationElv = prior_ntcip_1218_mib.rsuLocationElv;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}

/* 5.14.8 */
int32_t get_rsuElevationOffset(int32_t * data_out)
{
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      *data_out = ntcip_1218_mib.rsuElevationOffset;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  } 
  if((RSU_REF_ELV_OFFSET_MIN <= *data_out) && (*data_out <= RSU_REF_ELV_OFFSET_MAX)){
      return RSEMIB_OK; 
  } else {
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } 
}
int32_t preserve_rsuElevationOffset(void)
{
  if(RSEMIB_OK == ntcip1218_lock()){
      prior_ntcip_1218_mib.rsuElevationOffset = ntcip_1218_mib.rsuElevationOffset;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t action_rsuElevationOffset(int32_t data_in)
{
  if((data_in < RSU_REF_ELV_OFFSET_MIN) || (RSU_REF_ELV_OFFSET_MAX < data_in)){ 
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  }
  if(RSEMIB_OK == ntcip1218_lock()){  
      tmp_ntcip_1218_mib.rsuElevationOffset = data_in;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t commit_rsuElevationOffset(void)
{
  int32_t ret = RSEMIB_OK;
  SystemDescription_t sysDesc;

  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuElevationOffset = tmp_ntcip_1218_mib.rsuElevationOffset;
      if(RSEMIB_OK == (copy_sysDesc_to_skinny(&ntcip_1218_mib, &sysDesc))){
          if(RSEMIB_OK != commit_sysDesc_to_disk(&sysDesc)) {
              ret = RSEMIB_BAD_DATA;
          }
      } else {
          ret = RSEMIB_BAD_DATA;
      }
      tmp_ntcip_1218_mib.rsuElevationOffset = RSU_REF_ELV_OFFSET_MAX;
      ntcip1218_unlock();
  } else {
      ret = RSEMIB_LOCK_FAIL;
  }
  if(ret < RSEMIB_OK)
      set_ntcip_1218_error_states(ret);
  return ret;
}
int32_t undo_rsuElevationOffset(void)
{
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuElevationOffset = prior_ntcip_1218_mib.rsuElevationOffset;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}

/* 5.14.9 */
int32_t get_rsuInstallUpdate(int32_t * data_out)
{
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      *data_out = ntcip_1218_mib.rsuInstallUpdate;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  } 
  if((0 == *data_out) || (1 == *data_out)){
      return RSEMIB_OK; 
  } else {
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } 
}
int32_t preserve_rsuInstallUpdate(void)
{
  if(RSEMIB_OK == ntcip1218_lock()){
      prior_ntcip_1218_mib.rsuInstallUpdate = ntcip_1218_mib.rsuInstallUpdate;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t action_rsuInstallUpdate(int32_t data_in)
{
  if((0 != data_in) && (1 != data_in)){ 
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  }
  if(RSEMIB_OK == ntcip1218_lock()){  
      tmp_ntcip_1218_mib.rsuInstallUpdate = data_in;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t commit_rsuInstallUpdate(void)
{
  int32_t ret = RSEMIB_OK;
  SystemDescription_t sysDesc;

  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuInstallUpdate = tmp_ntcip_1218_mib.rsuInstallUpdate;
      if(RSEMIB_OK == (copy_sysDesc_to_skinny(&ntcip_1218_mib, &sysDesc))){
          if(RSEMIB_OK != commit_sysDesc_to_disk(&sysDesc)) {
              ret = RSEMIB_BAD_DATA;
          }
      } else {
          ret = RSEMIB_BAD_DATA;
      }
      tmp_ntcip_1218_mib.rsuInstallUpdate = 0;
      ntcip1218_unlock();
  } else {
      ret = RSEMIB_LOCK_FAIL;
  }
  if(ret < RSEMIB_OK)
      set_ntcip_1218_error_states(ret);
  return ret;
}
int32_t undo_rsuInstallUpdate(void)
{
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuInstallUpdate = prior_ntcip_1218_mib.rsuInstallUpdate;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}

/* 5.14.10 */
int32_t get_rsuInstallFile(uint8_t * data_out)
{
  int32_t size = 0;
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      size = ntcip_1218_mib.rsuInstallFile_length;
      memcpy(data_out,ntcip_1218_mib.rsuInstallFile, RSU_INSTALL_FILE_NAME_LENGTH_MAX);
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
  if((size < RSU_INSTALL_FILE_NAME_LENGTH_MIN) || (RSU_INSTALL_FILE_NAME_LENGTH_MAX < size)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  }
  return size;
} 
int32_t preserve_rsuInstallFile(void)
{
  if(RSEMIB_OK == ntcip1218_lock()){
      prior_ntcip_1218_mib.rsuInstallFile_length = ntcip_1218_mib.rsuInstallFile_length;
      memcpy(prior_ntcip_1218_mib.rsuInstallFile, ntcip_1218_mib.rsuInstallFile, RSU_INSTALL_FILE_NAME_LENGTH_MAX);
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t action_rsuInstallFile(uint8_t * data_in, int32_t length)
{
  if((NULL == data_in) && (0 != length)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if((length < RSU_INSTALL_FILE_NAME_LENGTH_MIN) || (RSU_INSTALL_FILE_NAME_LENGTH_MAX < length)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  }  
  if(RSEMIB_OK == ntcip1218_lock()){  
      memset(tmp_ntcip_1218_mib.rsuInstallFile, 0x0, RSU_INSTALL_FILE_NAME_LENGTH_MAX);
      if(NULL != data_in){
          memcpy(tmp_ntcip_1218_mib.rsuInstallFile, data_in,  RSU_INSTALL_FILE_NAME_LENGTH_MAX);
      }
      tmp_ntcip_1218_mib.rsuInstallFile_length = length;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t commit_rsuInstallFile(void)
{
  int32_t ret = RSEMIB_OK;
  SystemDescription_t sysDesc;

  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuInstallFile_length = tmp_ntcip_1218_mib.rsuInstallFile_length;
      memcpy(ntcip_1218_mib.rsuInstallFile, tmp_ntcip_1218_mib.rsuInstallFile, RSU_INSTALL_FILE_NAME_LENGTH_MAX);
      if(RSEMIB_OK == (copy_sysDesc_to_skinny(&ntcip_1218_mib, &sysDesc))){
          if(RSEMIB_OK != commit_sysDesc_to_disk(&sysDesc)) {
              ret = RSEMIB_BAD_DATA;
          }
      } else {
          ret = RSEMIB_BAD_DATA;
      }
      memset(tmp_ntcip_1218_mib.rsuInstallFile, 0x0, RSU_INSTALL_FILE_NAME_LENGTH_MAX);
      tmp_ntcip_1218_mib.rsuInstallFile_length = 0;
      ntcip1218_unlock();
  } else {
      ret = RSEMIB_LOCK_FAIL;
  }
  if(ret < RSEMIB_OK)
      set_ntcip_1218_error_states(ret);
  return ret;
}
int32_t undo_rsuInstallFile(void)
{
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuInstallFile_length = prior_ntcip_1218_mib.rsuInstallFile_length;
      memcpy(ntcip_1218_mib.rsuInstallFile, prior_ntcip_1218_mib.rsuInstallFile, RSU_INSTALL_FILE_NAME_LENGTH_MAX);
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}

/* 5.14.11 */
int32_t get_rsuInstallPath(uint8_t * data_out)
{
  int32_t size = 0;
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      size = ntcip_1218_mib.rsuInstallPath_length;
      memcpy(data_out,ntcip_1218_mib.rsuInstallPath,RSU_INSTALL_PATH_LENGTH_MAX);
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
  if((size < RSU_INSTALL_PATH_LENGTH_MIN) || (RSU_INSTALL_PATH_LENGTH_MAX < size)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  }
  return size;
} 
int32_t preserve_rsuInstallPath(void)
{
  if(RSEMIB_OK == ntcip1218_lock()){
      prior_ntcip_1218_mib.rsuInstallPath_length = ntcip_1218_mib.rsuInstallPath_length;
      memcpy(prior_ntcip_1218_mib.rsuInstallPath, ntcip_1218_mib.rsuInstallPath, RSU_INSTALL_PATH_LENGTH_MAX);
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t action_rsuInstallPath(uint8_t * data_in, int32_t length)
{
  if(NULL == data_in){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if((length < RSU_INSTALL_PATH_LENGTH_MIN) || (RSU_INSTALL_PATH_LENGTH_MAX < length)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  }  
  if(RSEMIB_OK == ntcip1218_lock()){
      tmp_ntcip_1218_mib.rsuInstallPath_length = length;
      memset(tmp_ntcip_1218_mib.rsuInstallPath, 0x0, RSU_INSTALL_PATH_LENGTH_MAX);
      memcpy(tmp_ntcip_1218_mib.rsuInstallPath, data_in, length);
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t commit_rsuInstallPath(void)
{
  int32_t ret = RSEMIB_OK;
  SystemDescription_t sysDesc;

  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuInstallPath_length = tmp_ntcip_1218_mib.rsuInstallPath_length;
      memcpy(ntcip_1218_mib.rsuInstallPath, tmp_ntcip_1218_mib.rsuInstallPath, RSU_INSTALL_PATH_LENGTH_MAX);
      if(RSEMIB_OK == (copy_sysDesc_to_skinny(&ntcip_1218_mib, &sysDesc))){
          if(RSEMIB_OK != commit_sysDesc_to_disk(&sysDesc)) {
              ret = RSEMIB_BAD_DATA;
          }
      } else {
          ret = RSEMIB_BAD_DATA;
      }
      memset(tmp_ntcip_1218_mib.rsuInstallPath, 0x0, RSU_INSTALL_PATH_LENGTH_MAX);
      tmp_ntcip_1218_mib.rsuInstallPath_length = 0;
      ntcip1218_unlock();
  } else {
      ret = RSEMIB_LOCK_FAIL;
  }
  if(ret < RSEMIB_OK)
      set_ntcip_1218_error_states(ret);
  return ret;
}
int32_t undo_rsuInstallPath(void)
{
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuInstallPath_length = prior_ntcip_1218_mib.rsuInstallPath_length;
      memcpy(ntcip_1218_mib.rsuInstallPath, prior_ntcip_1218_mib.rsuInstallPath, RSU_INSTALL_PATH_LENGTH_MAX);
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}

/*5.14.12: Unless RSU provisioning used the MIB to install, there is no record of that install.*/ 
int32_t get_rsuInstallStatus(int32_t * data_out)
{
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      *data_out = ntcip_1218_mib.rsuInstallStatus;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
  if((installOther <= *data_out) && (*data_out <= installSuccess)){
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  }
} 
/* 5.14.13: date and time rsuInstallStatus updated. */
int32_t get_rsuInstallTime(uint8_t * data_out)
{
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      memcpy(data_out, ntcip_1218_mib.rsuInstallTime, MIB_DATEANDTIME_LENGTH);
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}

/* 5.14.14: Use error states to create on the fly what is relevent. */
int32_t get_rsuInstallStatusMessage(uint8_t * data_out)
{
  int32_t size = 0;
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      size = ntcip_1218_mib.rsuInstallStatusMessage_length;
      memcpy(data_out, ntcip_1218_mib.rsuInstallStatusMessage, RSU_INSTALL_STATUS_MSG_LENGTH_MAX);
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
  if((size < RSU_INSTALL_STATUS_MSG_LENGTH_MIN) || (RSU_INSTALL_STATUS_MSG_LENGTH_MAX < size)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  }
  return size;
}

/* 5.14.15 */
int32_t get_rsuScheduledInstallTime(uint8_t * data_out)
{
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      memcpy(data_out, ntcip_1218_mib.rsuScheduledInstallTime, MIB_DATEANDTIME_LENGTH);
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t preserve_rsuScheduledInstallTime(void)
{
  if(RSEMIB_OK == ntcip1218_lock()){
      memcpy(  prior_ntcip_1218_mib.rsuScheduledInstallTime, ntcip_1218_mib.rsuScheduledInstallTime, MIB_DATEANDTIME_LENGTH);
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t action_rsuScheduledInstallTime(uint8_t * data_in, int32_t length)
{
  int32_t i;
  if((NULL == data_in) || (MIB_DATEANDTIME_LENGTH != length)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  /* All zeros is illegal, but for this function they want it to == "send it now, ignore this value" */
  for(i=0;i<MIB_DATEANDTIME_LENGTH;i++){
      if(0x0 != data_in[i])
          break;
  }
  if (i < MIB_DATEANDTIME_LENGTH) { /* Not all zeros, eval for correctness. */
      if(RSEMIB_OK != DateAndTime_To_UTC_DSEC(data_in,NULL))
          return RSEMIB_BAD_DATA;
  }
  if(RSEMIB_OK == ntcip1218_lock()){  
      memcpy(tmp_ntcip_1218_mib.rsuScheduledInstallTime, data_in , MIB_DATEANDTIME_LENGTH);
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t commit_rsuScheduledInstallTime(void)
{
  int32_t ret = RSEMIB_OK;
  SystemDescription_t sysDesc;

  if(RSEMIB_OK == ntcip1218_lock()){
      memcpy(ntcip_1218_mib.rsuScheduledInstallTime, tmp_ntcip_1218_mib.rsuScheduledInstallTime, MIB_DATEANDTIME_LENGTH);
      if(RSEMIB_OK == (copy_sysDesc_to_skinny(&ntcip_1218_mib, &sysDesc))){
          if(RSEMIB_OK != commit_sysDesc_to_disk(&sysDesc)) {
              ret = RSEMIB_BAD_DATA;
          }
      } else {
          ret = RSEMIB_BAD_DATA;
      }
      memset(tmp_ntcip_1218_mib.rsuScheduledInstallTime, 0x0, MIB_DATEANDTIME_LENGTH);
      ntcip1218_unlock();
  } else {
      ret = RSEMIB_LOCK_FAIL;
  }
  if(ret < RSEMIB_OK)
      set_ntcip_1218_error_states(ret);
  return ret;
}
int32_t undo_rsuScheduledInstallTime(void)
{
  if(RSEMIB_OK == ntcip1218_lock()){
      memcpy( ntcip_1218_mib.rsuScheduledInstallTime, prior_ntcip_1218_mib.rsuScheduledInstallTime, MIB_DATEANDTIME_LENGTH);
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}

/******************************************************************************
 * 5.15 System Settings: {rsu 14}
 ******************************************************************************/

/* 5.15.1 */
int32_t get_rsuNotifyIpAddress(uint8_t * data_out)
{
  int32_t size = 0;
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      size = ntcip_1218_mib.rsuNotifyIpAddress_length;
      memcpy(data_out, ntcip_1218_mib.rsuNotifyIpAddress, RSU_DEST_IP_MAX);
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
  if((size < RSU_DEST_IP_MIN) || (RSU_DEST_IP_MAX < size)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);  
      return RSEMIB_BAD_DATA;
  }
  return size;
}
int32_t preserve_rsuNotifyIpAddress(void)
{
  if(RSEMIB_OK == ntcip1218_lock()){
     prior_ntcip_1218_mib.rsuNotifyIpAddress_length = ntcip_1218_mib.rsuNotifyIpAddress_length;
      memcpy(prior_ntcip_1218_mib.rsuNotifyIpAddress, ntcip_1218_mib.rsuNotifyIpAddress, RSU_DEST_IP_MAX);
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t action_rsuNotifyIpAddress(uint8_t * data_in, int32_t length)
{
  if((NULL == data_in) && (0 != length)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if((length < RSU_DEST_IP_MIN) || (RSU_DEST_IP_MAX < length)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  }    
  if(RSEMIB_OK == ntcip1218_lock()){
      tmp_ntcip_1218_mib.rsuNotifyIpAddress_length = length;
      memcpy(tmp_ntcip_1218_mib.rsuNotifyIpAddress, data_in, RSU_DEST_IP_MAX);
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t commit_rsuNotifyIpAddress(void)
{
  int32_t ret = RSEMIB_OK;
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuNotifyIpAddress_length = tmp_ntcip_1218_mib.rsuNotifyIpAddress_length;
      memcpy(ntcip_1218_mib.rsuNotifyIpAddress, tmp_ntcip_1218_mib.rsuNotifyIpAddress, RSU_DEST_IP_MAX);
      memset(tmp_ntcip_1218_mib.rsuNotifyIpAddress, 0x0, RSU_DEST_IP_MAX);
      if(RSEMIB_OK == (ret = commit_rsuSysSettings_to_disk())){
          if(RSEMIB_OK != (ret = update_rsuSysSettings_shm())){
              ret = RSEMIB_BAD_DATA;
          }
      } else {
          ret = RSEMIB_BAD_DATA;
      }
      ntcip1218_unlock();
  } else {
      ret = RSEMIB_LOCK_FAIL;
  }
  if(ret < RSEMIB_OK) 
      set_ntcip_1218_error_states(ret);  
  return ret;
}
int32_t undo_rsuNotifyIpAddress(void)
{
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuNotifyIpAddress_length = prior_ntcip_1218_mib.rsuNotifyIpAddress_length;
      memcpy(ntcip_1218_mib.rsuNotifyIpAddress, prior_ntcip_1218_mib.rsuNotifyIpAddress, RSU_DEST_IP_MAX);
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
/* 5.15.2 */
int32_t get_rsuNotifyPort(void)
{
  int32_t data = 0;
  if(RSEMIB_OK == ntcip1218_lock()){
      data = ntcip_1218_mib.rsuNotifyPort;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  } 
  if((RSU_DEST_PORT_MIN <= data) && (data <= RSU_DEST_PORT_MAX)){
      return data; 
  } else {
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } 
}
int32_t preserve_rsuNotifyPort(void)
{
  if(RSEMIB_OK == ntcip1218_lock()){
      prior_ntcip_1218_mib.rsuNotifyPort = ntcip_1218_mib.rsuNotifyPort;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t action_rsuNotifyPort(int32_t data)
{
  if((data < RSU_DEST_PORT_MIN) || (RSU_DEST_PORT_MAX < data)){ 
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  }
  if(RSEMIB_OK == ntcip1218_lock()){  
      tmp_ntcip_1218_mib.rsuNotifyPort = data;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t commit_rsuNotifyPort(void)
{
  int32_t ret = RSEMIB_OK;
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuNotifyPort     = tmp_ntcip_1218_mib.rsuNotifyPort; 
      tmp_ntcip_1218_mib.rsuNotifyPort = 0;
      if(RSEMIB_OK == (ret = commit_rsuSysSettings_to_disk())){
          if(RSEMIB_OK != (ret = update_rsuSysSettings_shm())){
              ret = RSEMIB_BAD_DATA;
          }
      } else {
          ret = RSEMIB_BAD_DATA;
      }
      ntcip1218_unlock();
  } else {
      ret = RSEMIB_LOCK_FAIL;
  }
  if(ret < RSEMIB_OK) 
      set_ntcip_1218_error_states(ret);  
  return ret;
}
int32_t undo_rsuNotifyPort(void)
{
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuNotifyPort = prior_ntcip_1218_mib.rsuNotifyPort;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}
/* 5.15.3 */
int32_t get_rsuSysLogQueryStart(uint8_t * data_out)
{
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      memcpy(data_out, ntcip_1218_mib.rsuSysLogQueryStart, RFC2579_DATEANDTIME_LENGTH);
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t preserve_rsuSysLogQueryStart(void)
{
  if(RSEMIB_OK == ntcip1218_lock()){
      memcpy(prior_ntcip_1218_mib.rsuSysLogQueryStart, ntcip_1218_mib.rsuSysLogQueryStart, RFC2579_DATEANDTIME_LENGTH);
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t action_rsuSysLogQueryStart(uint8_t * data_in, int32_t length)
{
  if((NULL == data_in) || (RFC2579_DATEANDTIME_LENGTH != length)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){  
      memcpy(tmp_ntcip_1218_mib.rsuSysLogQueryStart, data_in, RFC2579_DATEANDTIME_LENGTH);
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t commit_rsuSysLogQueryStart(void)
{
  int32_t ret = RSEMIB_OK;
  if(RSEMIB_OK == ntcip1218_lock()){
      memcpy(ntcip_1218_mib.rsuSysLogQueryStart, tmp_ntcip_1218_mib.rsuSysLogQueryStart, RFC2579_DATEANDTIME_LENGTH);
      memset(tmp_ntcip_1218_mib.rsuSysLogQueryStart, 0x0, RFC2579_DATEANDTIME_LENGTH);
      if(RSEMIB_OK == (ret = commit_rsuSysSettings_to_disk())){
          if(RSEMIB_OK != (ret = update_rsuSysSettings_shm())){
              ret = RSEMIB_BAD_DATA;
          }
      } else {
          ret = RSEMIB_BAD_DATA;
      }
      ntcip1218_unlock();
  } else {
      ret = RSEMIB_LOCK_FAIL;
  }
  if(ret < RSEMIB_OK) 
      set_ntcip_1218_error_states(ret);  
  return ret;
}
int32_t undo_rsuSysLogQueryStart(void)
{
  if(RSEMIB_OK == ntcip1218_lock()){
      memcpy( ntcip_1218_mib.rsuSysLogQueryStart, prior_ntcip_1218_mib.rsuSysLogQueryStart, RFC2579_DATEANDTIME_LENGTH);
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
/* 5.15.4 */
int32_t get_rsuSysLogQueryStop(uint8_t * data_out)
{
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      memcpy(data_out, ntcip_1218_mib.rsuSysLogQueryStop, RFC2579_DATEANDTIME_LENGTH);
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t preserve_rsuSysLogQueryStop(void)
{
  if(RSEMIB_OK == ntcip1218_lock()){
      memcpy(  prior_ntcip_1218_mib.rsuSysLogQueryStop, ntcip_1218_mib.rsuSysLogQueryStop, RFC2579_DATEANDTIME_LENGTH);
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t action_rsuSysLogQueryStop(uint8_t * data_in, int32_t length)
{
  if((NULL == data_in) || (RFC2579_DATEANDTIME_LENGTH != length)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){  
      memcpy(tmp_ntcip_1218_mib.rsuSysLogQueryStop, data_in , RFC2579_DATEANDTIME_LENGTH);
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t commit_rsuSysLogQueryStop(void)
{
  int32_t ret = RSEMIB_OK;
  if(RSEMIB_OK == ntcip1218_lock()){
      memcpy(ntcip_1218_mib.rsuSysLogQueryStop, tmp_ntcip_1218_mib.rsuSysLogQueryStop, RFC2579_DATEANDTIME_LENGTH);
      memset(tmp_ntcip_1218_mib.rsuSysLogQueryStop, 0x0, RFC2579_DATEANDTIME_LENGTH);
      if(RSEMIB_OK == (ret = commit_rsuSysSettings_to_disk())){
          if(RSEMIB_OK != (ret = update_rsuSysSettings_shm())){
              ret = RSEMIB_BAD_DATA;
          }
      } else {
          ret = RSEMIB_BAD_DATA;
      }
      ntcip1218_unlock();
  } else {
      ret = RSEMIB_LOCK_FAIL;
  }
  if(ret < RSEMIB_OK) 
      set_ntcip_1218_error_states(ret);  
  return ret;
}
int32_t undo_rsuSysLogQueryStop(void)
{
  if(RSEMIB_OK == ntcip1218_lock()){
      memcpy( ntcip_1218_mib.rsuSysLogQueryStop, prior_ntcip_1218_mib.rsuSysLogQueryStop, RFC2579_DATEANDTIME_LENGTH);
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
/* 5.15.5 */
int32_t get_rsuSysLogQueryPriority(void)
{
  int32_t data = 0;
  if(RSEMIB_OK == ntcip1218_lock()){
      data = ntcip_1218_mib.rsuSysLogQueryPriority;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  } 
  if((syslog_level_emergency <= data) && (data <= syslog_level_debug)){
      return data; 
  } else {
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } 
}
int32_t preserve_rsuSysLogQueryPriority(void)
{
  if(RSEMIB_OK == ntcip1218_lock()){
      prior_ntcip_1218_mib.rsuSysLogQueryPriority = ntcip_1218_mib.rsuSysLogQueryPriority;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t action_rsuSysLogQueryPriority(int32_t data)
{
  if((data < syslog_level_emergency) || (syslog_level_debug < data)){ 
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  }
  if(RSEMIB_OK == ntcip1218_lock()){  
      tmp_ntcip_1218_mib.rsuSysLogQueryPriority = data;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t commit_rsuSysLogQueryPriority(void)
{
  int32_t ret = RSEMIB_OK;
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuSysLogQueryPriority     = tmp_ntcip_1218_mib.rsuSysLogQueryPriority; 
      tmp_ntcip_1218_mib.rsuSysLogQueryPriority = syslog_level_error;
      if(RSEMIB_OK == (ret = commit_rsuSysSettings_to_disk())){
          if(RSEMIB_OK != (ret = update_rsuSysSettings_shm())){
              ret = RSEMIB_BAD_DATA;
          }
      } else {
          ret = RSEMIB_BAD_DATA;
      }
      ntcip1218_unlock();
  } else {
      ret = RSEMIB_LOCK_FAIL;
  }
  if(ret < RSEMIB_OK) 
      set_ntcip_1218_error_states(ret);  
  return ret;
}
int32_t undo_rsuSysLogQueryPriority(void)
{
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuSysLogQueryPriority = prior_ntcip_1218_mib.rsuSysLogQueryPriority;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}
/* 5.15.6 */
int32_t get_rsuSysLogQueryGenerate(void)
{
  int32_t data = 0;
  if(RSEMIB_OK == ntcip1218_lock()){
      if(RSEMIB_OK > (data = get_sysLogQueryGenerate())) { /* This updates MIB from SHM. */ 
          data = RSEMIB_BAD_DATA; /* dumb error down for SNMP handler. */
      }
      ntcip1218_unlock();
  } else {
      data = RSEMIB_LOCK_FAIL;
  } 
  if ((data < 0) || (1 < data)) {
      data = RSEMIB_BAD_DATA;
  }
  if (data < RSEMIB_OK)
      set_ntcip_1218_error_states(data);
  return data;
}
int32_t preserve_rsuSysLogQueryGenerate(void)
{
  if(RSEMIB_OK == ntcip1218_lock()){
      prior_ntcip_1218_mib.rsuSysLogQueryGenerate = ntcip_1218_mib.rsuSysLogQueryGenerate;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t action_rsuSysLogQueryGenerate(int32_t data)
{
  if((data < 0) || (1 < data)){ 
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  }
  if(RSEMIB_OK == ntcip1218_lock()){  
      tmp_ntcip_1218_mib.rsuSysLogQueryGenerate = data;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t commit_rsuSysLogQueryGenerate(void)
{
  int32_t ret = RSEMIB_OK;
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuSysLogQueryGenerate     = tmp_ntcip_1218_mib.rsuSysLogQueryGenerate; 
      tmp_ntcip_1218_mib.rsuSysLogQueryGenerate = 0;
      if(RSEMIB_OK == (ret = commit_rsuSysSettings_to_disk())){
          if(RSEMIB_OK != (ret = update_rsuSysSettings_shm())){
              ret = RSEMIB_BAD_DATA;
          }
      } else {
          ret = RSEMIB_BAD_DATA;
      }
      ntcip1218_unlock();
  } else {
      ret = RSEMIB_LOCK_FAIL;
  }
  if(ret < RSEMIB_OK) 
      set_ntcip_1218_error_states(ret);  
  return ret;
}
int32_t undo_rsuSysLogQueryGenerate(void)
{
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuSysLogQueryGenerate = prior_ntcip_1218_mib.rsuSysLogQueryGenerate;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}
/* 5.15.7 */
int32_t get_rsuSysLogQueryStatus(void)
{
  int32_t data = 0;
  if(RSEMIB_OK == ntcip1218_lock()){
      if(RSEMIB_OK > (data = get_sysLogQueryStatus())) { /* This updates MIB from SHM. */ 
          data = RSEMIB_BAD_DATA; /* dumb error down for SNMP handler. */
      }
      ntcip1218_unlock();
  } else {
      data = RSEMIB_LOCK_FAIL;
  } 
  if((data < syslog_status_other) || (syslog_status_badFilename < data)){
      data = RSEMIB_BAD_DATA;
  }
  if (data < RSEMIB_OK)
      set_ntcip_1218_error_states(data);
  return data;
}
int32_t preserve_rsuSysLogQueryStatus(void)
{
  if(RSEMIB_OK == ntcip1218_lock()){
      prior_ntcip_1218_mib.rsuSysLogQueryStatus = ntcip_1218_mib.rsuSysLogQueryStatus;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t action_rsuSysLogQueryStatus(int32_t data)
{
  if((data < syslog_status_other) || (syslog_status_badFilename < data)){ 
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  }
  if(RSEMIB_OK == ntcip1218_lock()){  
      tmp_ntcip_1218_mib.rsuSysLogQueryStatus = data;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t commit_rsuSysLogQueryStatus(void)
{
  int32_t ret = RSEMIB_OK;
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuSysLogQueryStatus     = tmp_ntcip_1218_mib.rsuSysLogQueryStatus; 
      tmp_ntcip_1218_mib.rsuSysLogQueryStatus = syslog_status_other;
      if(RSEMIB_OK == (ret = commit_rsuSysSettings_to_disk())){
          if(RSEMIB_OK != (ret = update_rsuSysSettings_shm())){
              ret = RSEMIB_BAD_DATA;
          }
      } else {
          ret = RSEMIB_BAD_DATA;
      }
      ntcip1218_unlock();
  } else {
      ret = RSEMIB_LOCK_FAIL;
  }
  if(ret < RSEMIB_OK) 
      set_ntcip_1218_error_states(ret);  
  return ret;
}
int32_t undo_rsuSysLogQueryStatus(void)
{
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuSysLogQueryStatus = prior_ntcip_1218_mib.rsuSysLogQueryStatus;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}
/* 5.15.8 */
int32_t get_rsuSysLogCloseCommand(void)
{
  int32_t data = 0;
  if(RSEMIB_OK == ntcip1218_lock()){
      if(RSEMIB_OK > (data = get_sysLogCloseCommand())) { /* This updates MIB from SHM. */ 
          data = RSEMIB_BAD_DATA; /* dumb error down for SNMP handler. */
      }
      ntcip1218_unlock();
  } else {
      data = RSEMIB_LOCK_FAIL;
  } 
  if ((data < 0) || (1 < data)) {
      data = RSEMIB_BAD_DATA;
  }
  if (data < RSEMIB_OK)
      set_ntcip_1218_error_states(data);
  return data;
}
int32_t preserve_rsuSysLogCloseCommand(void)
{
  if(RSEMIB_OK == ntcip1218_lock()){
      prior_ntcip_1218_mib.rsuSysLogCloseCommand = ntcip_1218_mib.rsuSysLogCloseCommand;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t action_rsuSysLogCloseCommand(int32_t data)
{
  if((data < 0) || (1 < data)){ 
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  }
  if(RSEMIB_OK == ntcip1218_lock()){  
      tmp_ntcip_1218_mib.rsuSysLogCloseCommand = data;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t commit_rsuSysLogCloseCommand(void)
{
  int32_t ret = RSEMIB_OK;
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuSysLogCloseCommand     = tmp_ntcip_1218_mib.rsuSysLogCloseCommand; 
      tmp_ntcip_1218_mib.rsuSysLogCloseCommand = 0;
      //ntcip_1218_mib.rsuSysLogCloseCommand     = 0; /* Return to zero when done. */
      if(RSEMIB_OK == (ret = commit_rsuSysSettings_to_disk())){
          if(RSEMIB_OK != (ret = update_rsuSysSettings_shm())){
              ret = RSEMIB_BAD_DATA;
          }
      } else {
          ret = RSEMIB_BAD_DATA;
      }
      ntcip1218_unlock();
  } else {
      ret = RSEMIB_LOCK_FAIL;
  }
  if(ret < RSEMIB_OK) 
      set_ntcip_1218_error_states(ret);  
  return ret;
}
int32_t undo_rsuSysLogCloseCommand(void)
{
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuSysLogCloseCommand = prior_ntcip_1218_mib.rsuSysLogCloseCommand;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}

/* 5.15.9 */
int32_t get_rsuSysLogSeverity(void)
{
  int32_t data = 0;
  if(RSEMIB_OK == ntcip1218_lock()){
      data = ntcip_1218_mib.rsuSysLogSeverity;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  } 
  if((syslog_level_emergency <= data) && (data <= syslog_level_debug)){
      return data; 
  } else {
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } 
}
int32_t preserve_rsuSysLogSeverity(void)
{
  if(RSEMIB_OK == ntcip1218_lock()){
      prior_ntcip_1218_mib.rsuSysLogSeverity = ntcip_1218_mib.rsuSysLogSeverity;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t action_rsuSysLogSeverity(int32_t data)
{
  if((data < syslog_level_emergency) || (syslog_level_debug < data)){ 
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  }
  if(RSEMIB_OK == ntcip1218_lock()){  
      tmp_ntcip_1218_mib.rsuSysLogSeverity = data;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t commit_rsuSysLogSeverity(void)
{
  int32_t ret = RSEMIB_OK;
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuSysLogSeverity     = tmp_ntcip_1218_mib.rsuSysLogSeverity; 
      tmp_ntcip_1218_mib.rsuSysLogSeverity = syslog_level_error;
      if(RSEMIB_OK == (ret = commit_rsuSysSettings_to_disk())){
          if(RSEMIB_OK != (ret = update_rsuSysSettings_shm())){
              ret = RSEMIB_BAD_DATA;
          }
      } else {
          ret = RSEMIB_BAD_DATA;
      }
      ntcip1218_unlock();
  } else {
      ret = RSEMIB_LOCK_FAIL;
  }
  if(ret < RSEMIB_OK) 
      set_ntcip_1218_error_states(ret);  
  return ret;
}
int32_t undo_rsuSysLogSeverity(void)
{
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuSysLogSeverity = prior_ntcip_1218_mib.rsuSysLogSeverity;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}

/* 5.15.10 */
int32_t get_rsuSysConfigId(uint8_t * data_out)
{
  int32_t size = 0;
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      size = ntcip_1218_mib.rsuSysConfigId_length;
      memcpy(data_out, ntcip_1218_mib.rsuSysConfigId, RSU_SYS_CONFIG_ID_LENGTH_MAX);
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
  if((size < RSU_SYS_CONFIG_ID_LENGTH_MIN) || (RSU_SYS_CONFIG_ID_LENGTH_MAX < size)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);  
      return RSEMIB_BAD_DATA;
  }
  return size;
}
int32_t preserve_rsuSysConfigId(void)
{
  if(RSEMIB_OK == ntcip1218_lock()){
      prior_ntcip_1218_mib.rsuSysConfigId_length = ntcip_1218_mib.rsuSysConfigId_length;
      memcpy(prior_ntcip_1218_mib.rsuSysConfigId, ntcip_1218_mib.rsuSysConfigId, RSU_SYS_CONFIG_ID_LENGTH_MAX);
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t action_rsuSysConfigId(uint8_t * data_in, int32_t length)
{
  if((NULL == data_in) && (0 != length)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if((length < RSU_SYS_CONFIG_ID_LENGTH_MIN) || (RSU_SYS_CONFIG_ID_LENGTH_MAX < length)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } 
  if(RSEMIB_OK == ntcip1218_lock()){
      tmp_ntcip_1218_mib.rsuSysConfigId_length = length;
      memset(tmp_ntcip_1218_mib.rsuSysConfigId, 0x0, RSU_SYS_CONFIG_ID_LENGTH_MAX);
      if(NULL != data_in){
          memcpy(tmp_ntcip_1218_mib.rsuSysConfigId, data_in, RSU_SYS_CONFIG_ID_LENGTH_MAX);
      }
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t commit_rsuSysConfigId(void)
{
  int32_t ret = RSEMIB_OK;
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuSysConfigId_length = tmp_ntcip_1218_mib.rsuSysConfigId_length;
      memcpy(ntcip_1218_mib.rsuSysConfigId, tmp_ntcip_1218_mib.rsuSysConfigId, RSU_SYS_CONFIG_ID_LENGTH_MAX);
      memset(tmp_ntcip_1218_mib.rsuSysConfigId, 0x0, RSU_SYS_CONFIG_ID_LENGTH_MAX);
      tmp_ntcip_1218_mib.rsuSysConfigId_length = 0;
      if(RSEMIB_OK == (ret = commit_rsuSysSettings_to_disk())){
          if(RSEMIB_OK != (ret = update_rsuSysSettings_shm())){
              ret = RSEMIB_BAD_DATA;
          }
      } else {
          ret = RSEMIB_BAD_DATA;
      }
      ntcip1218_unlock();
  } else {
      ret = RSEMIB_LOCK_FAIL;
  }
  if(ret < RSEMIB_OK) 
      set_ntcip_1218_error_states(ret);  
  return ret;
}
int32_t undo_rsuSysConfigId(void)
{
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuSysConfigId_length = prior_ntcip_1218_mib.rsuSysConfigId_length;
      memcpy(ntcip_1218_mib.rsuSysConfigId, prior_ntcip_1218_mib.rsuSysConfigId, RSU_SYS_CONFIG_ID_LENGTH_MAX);
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}

/* 5.15.11 */
int32_t get_rsuSysRetries(void)
{
  int32_t data = 0;

  if(RSEMIB_OK == ntcip1218_lock()){
      data = ntcip_1218_mib.rsuSysRetries;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  } 
  if((RSU_SYS_RETRIES_MIN <= data) && (data <= RSU_SYS_RETRIES_MAX)){
      return data; 
  } else {
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } 
}
int32_t preserve_rsuSysRetries(void)
{
  if(RSEMIB_OK == ntcip1218_lock()){
      prior_ntcip_1218_mib.rsuSysRetries = ntcip_1218_mib.rsuSysRetries;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t action_rsuSysRetries(int32_t data)
{
  if((data < RSU_SYS_RETRIES_MIN) || (RSU_SYS_RETRIES_MAX < data)){ 
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  }
  if(RSEMIB_OK == ntcip1218_lock()){  
      tmp_ntcip_1218_mib.rsuSysRetries = data;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t commit_rsuSysRetries(void)
{
  int32_t ret = RSEMIB_OK;
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuSysRetries     = tmp_ntcip_1218_mib.rsuSysRetries; 
      tmp_ntcip_1218_mib.rsuSysRetries = RSU_SYS_RETRIES_MAX;
      ntcip_1218_mib.rsuSysRetryAttempts = 0;
      ntcip_1218_mib.rsuSysRetryPeriodStart = 0;
      if(RSEMIB_OK == (ret = commit_rsuSysSettings_to_disk())){
          if(RSEMIB_OK != (ret = update_rsuSysSettings_shm())){
              ret = RSEMIB_BAD_DATA;
          }
      } else {
          ret = RSEMIB_BAD_DATA;
      }
      ntcip1218_unlock();
  } else {
      ret = RSEMIB_LOCK_FAIL;
  }
  if(ret < RSEMIB_OK) 
      set_ntcip_1218_error_states(ret);  
  return ret;
}
int32_t undo_rsuSysRetries(void)
{
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuSysRetries = prior_ntcip_1218_mib.rsuSysRetries;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}
/* 5.15.12 */
int32_t get_rsuSysRetryPeriod(void)
{
  int32_t data = 0;
  if(RSEMIB_OK == ntcip1218_lock()){
      data = ntcip_1218_mib.rsuSysRetryPeriod;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  } 
  if((RSU_SYS_RETRY_PERIOD_MIN <= data) && (data <= RSU_SYS_RETRY_PERIOD_MAX)){
      return data; 
  } else {
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } 
}
int32_t preserve_rsuSysRetryPeriod(void)
{
  if(RSEMIB_OK == ntcip1218_lock()){
      prior_ntcip_1218_mib.rsuSysRetryPeriod = ntcip_1218_mib.rsuSysRetryPeriod;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t action_rsuSysRetryPeriod(int32_t data)
{
  if((data < RSU_SYS_RETRY_PERIOD_MIN) || (RSU_SYS_RETRY_PERIOD_MAX < data)){ 
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  }
  if(RSEMIB_OK == ntcip1218_lock()){  
      tmp_ntcip_1218_mib.rsuSysRetryPeriod = data;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t commit_rsuSysRetryPeriod(void)
{
  int32_t ret = RSEMIB_OK;
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuSysRetryPeriod     = tmp_ntcip_1218_mib.rsuSysRetryPeriod; 
      tmp_ntcip_1218_mib.rsuSysRetryPeriod = RSU_SYS_RETRY_PERIOD_MIN;
      ntcip_1218_mib.rsuSysRetryAttempts = 0;
      ntcip_1218_mib.rsuSysRetryPeriodStart = 0;
      if(RSEMIB_OK == (ret = commit_rsuSysSettings_to_disk())){
          if(RSEMIB_OK != (ret = update_rsuSysSettings_shm())){
              ret = RSEMIB_BAD_DATA;
          }
      } else {
          ret = RSEMIB_BAD_DATA;
      }
      ntcip1218_unlock();
  } else {
      ret = RSEMIB_LOCK_FAIL;
  }
  if(ret < RSEMIB_OK) 
      set_ntcip_1218_error_states(ret);  
  return ret;
}
int32_t undo_rsuSysRetryPeriod(void)
{
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuSysRetryPeriod = prior_ntcip_1218_mib.rsuSysRetryPeriod;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}

/* 5.15.13 */
int32_t get_rsuShortCommLossTime(void)
{
  int32_t data = 0;
  if(RSEMIB_OK == ntcip1218_lock()){
      data = ntcip_1218_mib.rsuShortCommLossTime;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  } 
  if((RSU_SHORT_COMM_LOSS_TIME_MIN <= data) && (data <= RSU_SHORT_COMM_LOSS_TIME_MAX)){
      return data; 
  } else {
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } 
}
int32_t preserve_rsuShortCommLossTime(void)
{
  if(RSEMIB_OK == ntcip1218_lock()){
      prior_ntcip_1218_mib.rsuShortCommLossTime = ntcip_1218_mib.rsuShortCommLossTime;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t action_rsuShortCommLossTime(int32_t data)
{
  if((data < RSU_SHORT_COMM_LOSS_TIME_MIN) || (RSU_SHORT_COMM_LOSS_TIME_MAX < data)){ 
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  }
  if(RSEMIB_OK == ntcip1218_lock()){  
      tmp_ntcip_1218_mib.rsuShortCommLossTime = data;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t commit_rsuShortCommLossTime(void)
{
  int32_t ret = RSEMIB_OK;
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuShortCommLossTime     = tmp_ntcip_1218_mib.rsuShortCommLossTime; 
      tmp_ntcip_1218_mib.rsuShortCommLossTime = RSU_SHORT_COMM_LOSS_TIME_MIN;
      if(RSEMIB_OK == (ret = commit_rsuSysSettings_to_disk())){
          if(RSEMIB_OK != (ret = update_rsuSysSettings_shm())){
              ret = RSEMIB_BAD_DATA;
          }
      } else {
          ret = RSEMIB_BAD_DATA;
      }
      ntcip1218_unlock();
  } else {
      ret = RSEMIB_LOCK_FAIL;
  }
  if(ret < RSEMIB_OK) 
      set_ntcip_1218_error_states(ret);  
  return ret;
}
int32_t undo_rsuShortCommLossTime(void)
{
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuShortCommLossTime = prior_ntcip_1218_mib.rsuShortCommLossTime;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}
/* 5.15.14 */
int32_t get_rsuLongCommLossTime(void)
{
  int32_t data = 0;
  if(RSEMIB_OK == ntcip1218_lock()){
      data = ntcip_1218_mib.rsuLongCommLossTime;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  } 
  if((RSU_LONG_COMM_LOSS_TIME_MIN <= data) && (data <= RSU_LONG_COMM_LOSS_TIME_MAX)){
      return data; 
  } else {
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } 
}
int32_t preserve_rsuLongCommLossTime(void)
{
  if(RSEMIB_OK == ntcip1218_lock()){
      prior_ntcip_1218_mib.rsuLongCommLossTime = ntcip_1218_mib.rsuLongCommLossTime;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t action_rsuLongCommLossTime(int32_t data)
{
  if((data < RSU_LONG_COMM_LOSS_TIME_MIN) || (RSU_LONG_COMM_LOSS_TIME_MAX < data)){ 
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  }
  if(RSEMIB_OK == ntcip1218_lock()){  
      tmp_ntcip_1218_mib.rsuLongCommLossTime = data;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t commit_rsuLongCommLossTime(void)
{
  int32_t ret = RSEMIB_OK;
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuLongCommLossTime     = tmp_ntcip_1218_mib.rsuLongCommLossTime; 
      tmp_ntcip_1218_mib.rsuLongCommLossTime = RSU_LONG_COMM_LOSS_TIME_MIN;
      if(RSEMIB_OK == (ret = commit_rsuSysSettings_to_disk())){
          if(RSEMIB_OK != (ret = update_rsuSysSettings_shm())){
              ret = RSEMIB_BAD_DATA;
          }
      } else {
          ret = RSEMIB_BAD_DATA;
      }
      ntcip1218_unlock();
  } else {
      ret = RSEMIB_LOCK_FAIL;
  }
  if(ret < RSEMIB_OK) 
      set_ntcip_1218_error_states(ret);  
  return ret;
}
int32_t undo_rsuLongCommLossTime(void)
{
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuLongCommLossTime = prior_ntcip_1218_mib.rsuLongCommLossTime;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}

/* 5.15.15 */
int32_t get_rsuSysLogName(uint8_t * data_out)
{
  int32_t ret   = RSEMIB_OK;
  int32_t size  = 0;
  int32_t dummy = 0;
  if(NULL == data_out){
      ret = RSEMIB_BAD_INPUT;
  } else {
      if(RSEMIB_OK == ntcip1218_lock()){
          if(RSEMIB_OK == (ret = get_sysLogName(ntcip_1218_mib.rsuSysLogName,&dummy))){
              if((dummy < RSU_SYSLOG_NAME_AND_PATH_LENGTH_MIN) || (RSU_SYSLOG_NAME_AND_PATH_LENGTH_MAX < dummy)){
                  ret = RSEMIB_BAD_DATA;
              } else {
                  ntcip_1218_mib.rsuSysLogName_length = dummy;
                  size = ntcip_1218_mib.rsuSysLogName_length;
                  memcpy(data_out,ntcip_1218_mib.rsuSysLogName,ntcip_1218_mib.rsuSysLogName_length);
              }
          }
          if(RSEMIB_OK != ret) { /* Any error be sure to make mib well formed */
              memset(ntcip_1218_mib.rsuSysLogName, 0x0, RSU_SYSLOG_NAME_AND_PATH_LENGTH_MAX);
              strncpy((char_t *)ntcip_1218_mib.rsuSysLogName, RSU_SYSLOG_NAME_AND_PATH_DEFAULT, RSU_SYSLOG_NAME_AND_PATH_LENGTH_MAX);
              ntcip_1218_mib.rsuSysLogName_length = strlen(RSU_SYSLOG_NAME_AND_PATH_DEFAULT);
          }
          ntcip1218_unlock();
      } else {
          ret = RSEMIB_LOCK_FAIL;
      }
  }
  if(ret < RSEMIB_OK){
      set_ntcip_1218_error_states(ret);
      size = ret;
  }
  return size;
}
/* 5.15.16 */
int32_t get_rsuSysDir(uint8_t * data_out)
{
  int32_t ret   = RSEMIB_OK;
  int32_t size  = 0;
  int32_t dummy = 0;
  if(NULL == data_out){
      ret = RSEMIB_BAD_INPUT;
  } else {
      if(RSEMIB_OK == ntcip1218_lock()){
          if(RSEMIB_OK == (ret = get_sysDir(ntcip_1218_mib.rsuSysDir,&dummy))){
              if((dummy < RSU_SYSDIR_NAME_LENGTH_MIN) || (RSU_SYSDIR_NAME_LENGTH_MAX < dummy)){
                  ret = RSEMIB_BAD_DATA;
              } else {
                  ntcip_1218_mib.rsuSysDir_length = dummy;
                  size = ntcip_1218_mib.rsuSysDir_length;
                  memcpy(data_out,ntcip_1218_mib.rsuSysDir,ntcip_1218_mib.rsuSysDir_length);
              }
          }
          if(RSEMIB_OK != ret) { /* Any error be sure to make mib well formed */
              memset(ntcip_1218_mib.rsuSysDir, 0x0, RSU_SYSDIR_NAME_LENGTH_MAX);
              strncpy((char_t *)ntcip_1218_mib.rsuSysDir, RSU_SYSDIR_DEFAULT, RSU_SYSDIR_NAME_LENGTH_MAX);
              ntcip_1218_mib.rsuSysDir_length = strlen(RSU_SYSDIR_DEFAULT);
          }
          ntcip1218_unlock();
      } else {
          ret = RSEMIB_LOCK_FAIL;
      }
  }
  if(ret < RSEMIB_OK){
      set_ntcip_1218_error_states(ret);
      size = ret;
  }
  return size;
}

/* 5.15.17 */
int32_t get_rsuLongCommLossReboot(void)
{
  int32_t data = 0;

  if(RSEMIB_OK == ntcip1218_lock()){
      data = ntcip_1218_mib.rsuLongCommLossReboot;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  } 
  if((0 <= data) && (data <= 1)){
      return data; 
  } else {
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } 
}
int32_t preserve_rsuLongCommLossReboot(void)
{
  if(RSEMIB_OK == ntcip1218_lock()){
      prior_ntcip_1218_mib.rsuLongCommLossReboot = ntcip_1218_mib.rsuLongCommLossReboot;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t action_rsuLongCommLossReboot(int32_t data)
{
  if((data < 0) || (1 < data)){ 
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  }
  if(RSEMIB_OK == ntcip1218_lock()){  
      tmp_ntcip_1218_mib.rsuLongCommLossReboot = data;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t commit_rsuLongCommLossReboot(void)
{
  int32_t ret = RSEMIB_OK;
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuLongCommLossReboot     = tmp_ntcip_1218_mib.rsuLongCommLossReboot; 
      tmp_ntcip_1218_mib.rsuLongCommLossReboot = 0;
      if(RSEMIB_OK == (ret = commit_rsuSysSettings_to_disk())){
          if(RSEMIB_OK != (ret = update_rsuSysSettings_shm())){
              ret = RSEMIB_BAD_DATA;
          }
      } else {
          ret = RSEMIB_BAD_DATA;
      }
      ntcip1218_unlock();
  } else {
      ret = RSEMIB_LOCK_FAIL;
  }
  if(ret < RSEMIB_OK) 
      set_ntcip_1218_error_states(ret);  
  return ret;
}
int32_t undo_rsuLongCommLossReboot(void)
{
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuLongCommLossReboot = prior_ntcip_1218_mib.rsuLongCommLossReboot;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}

/* 5.15.18 */
int32_t get_rsuHostIpAddr(uint8_t * data_out)
{
  int32_t size = 0;
  int32_t ret = RSEMIB_OK;
  char_t  request_buf[150]; /* If you need more then increase for your needs. */
  char_t  reply_buf[50];   
  FILE   *fp;

  if(NULL == data_out){
      ret = RSEMIB_BAD_INPUT;
  } else {
      if(RSEMIB_OK == ntcip1218_lock()){
          memset(reply_buf,'\0',sizeof(reply_buf));
          snprintf(request_buf,sizeof(request_buf), DCU_READ_ETH0_IPV4_ADDR);
          if(NULL != (fp = popen(request_buf, "r"))){
              if(NULL != fgets(reply_buf, sizeof(reply_buf), fp)){
                  size = strlen(reply_buf);
                  if((RSU_DEST_IP_MIN <= size) && (size <= RSU_DEST_IP_MAX)){
                      ntcip_1218_mib.rsuHostIpAddr_length = size;
                      memcpy(ntcip_1218_mib.rsuHostIpAddr, reply_buf, size);
                      memcpy(data_out, ntcip_1218_mib.rsuHostIpAddr, size);
                      ret = RSEMIB_OK;
                  } else {
                      DEBUGMSGTL((MY_NAME, "get_rsuHostIpAddr: READ bad length=%d.\n",size));
                      ret = RSEMIB_BAD_DATA;
                  }
              } else {
                  DEBUGMSGTL((MY_NAME, "get_rsuHostIpAddr: READ fgets failed.\n"));
                  ret = RSEMIB_SYSCALL_FAIL;
              }
              pclose(fp);
          } else {
              DEBUGMSGTL((MY_NAME, "get_rsuHostIpAddr: READ request failed.\n"));
              ret = RSEMIB_SYSCALL_FAIL;
          }
          ntcip1218_unlock();
      } else {
          ret = RSEMIB_LOCK_FAIL;
      }
  }
  if(ret < RSEMIB_OK){
      set_ntcip_1218_error_states(ret);
      size = ret;
  }
  return size;
}
int32_t preserve_rsuHostIpAddr(void)
{
  if(RSEMIB_OK == ntcip1218_lock()){
      prior_ntcip_1218_mib.rsuHostIpAddr_length = ntcip_1218_mib.rsuHostIpAddr_length;
      memcpy(prior_ntcip_1218_mib.rsuHostIpAddr, ntcip_1218_mib.rsuHostIpAddr, RSU_DEST_IP_MAX);
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t action_rsuHostIpAddr(uint8_t * data_in, int32_t length)
{
  if((NULL == data_in) && (0 != length)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if((length < RSU_DEST_IP_MIN) || (RSU_DEST_IP_MAX < length)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      tmp_ntcip_1218_mib.rsuHostIpAddr_length = length;
      memset(tmp_ntcip_1218_mib.rsuHostIpAddr, 0x0, RSU_DEST_IP_MAX);
      if(NULL != data_in){
           memcpy(tmp_ntcip_1218_mib.rsuHostIpAddr, data_in, RSU_DEST_IP_MAX);
      }
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t commit_rsuHostIpAddr(void)
{
  int32_t ret = RSEMIB_OK;
  char_t  request_buf[150]; /* If you need more then increase for your needs. */
  char_t  reply_buf[50];   
  FILE   *fp;
  char_t  data[RSU_DEST_IP_MAX+1]; /* Make sure null terminated for string function. */

  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuHostIpAddr_length = tmp_ntcip_1218_mib.rsuHostIpAddr_length;
      memcpy(ntcip_1218_mib.rsuHostIpAddr, tmp_ntcip_1218_mib.rsuHostIpAddr, RSU_DEST_IP_MAX);
      memset(tmp_ntcip_1218_mib.rsuHostIpAddr, 0x0, RSU_DEST_IP_MAX);

      memset(reply_buf,'\0',sizeof(reply_buf));
      memset(data,'\0',sizeof(data));
      memcpy(data,ntcip_1218_mib.rsuHostIpAddr,ntcip_1218_mib.rsuHostIpAddr_length);
      snprintf(request_buf,sizeof(request_buf), DCU_WRITE_ETH0_IPV4_ADDR, data);
      DEBUGMSGTL((MY_NAME, "commit_rsuHostIpAddr: WRITE[%s]\n",request_buf));
      if(NULL != (fp = popen(request_buf, "r"))){
          if(NULL != fgets(reply_buf, sizeof(reply_buf), fp)){
              ret = RSEMIB_OK; /* Don't verify off WRITE. Successive WRITE of same value return different values. */
          } else {
              DEBUGMSGTL((MY_NAME, "commit_rsuHostIpAddr: WRITE fgets failed.\n"));
              ret = RSEMIB_SYSCALL_FAIL;
          }
          pclose(fp);
      } else {
          DEBUGMSGTL((MY_NAME, "commit_rsuHostIpAddr: WRITE request failed.\n"));
          ret = RSEMIB_SYSCALL_FAIL;
      }
      ntcip1218_unlock();
  } else {
      ret = RSEMIB_LOCK_FAIL;
  }
  if(ret < RSEMIB_OK) 
      set_ntcip_1218_error_states(ret);  
  return ret;
}
int32_t undo_rsuHostIpAddr(void)
{
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuHostIpAddr_length = prior_ntcip_1218_mib.rsuHostIpAddr_length;
      memcpy( ntcip_1218_mib.rsuHostIpAddr, prior_ntcip_1218_mib.rsuHostIpAddr, RSU_DEST_IP_MAX);
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
/* 5.15.19 */
int32_t get_rsuHostNetMask(uint8_t * data_out)
{
  int32_t size = 0;
  int32_t ret = RSEMIB_OK;
  char_t  request_buf[150]; /* If you need more then increase for your needs. */
  char_t  reply_buf[50];   
  FILE   *fp;

  if(NULL == data_out){
      ret = RSEMIB_BAD_INPUT;
  } else {
      if(RSEMIB_OK == ntcip1218_lock()){
          memset(reply_buf,'\0',sizeof(reply_buf));
          snprintf(request_buf,sizeof(request_buf), DCU_READ_ETH0_NETMASK);
          if(NULL != (fp = popen(request_buf, "r"))){
              if(NULL != fgets(reply_buf, sizeof(reply_buf), fp)){
                  size = strlen(reply_buf);
                  if((RSU_DEST_IP_MIN <= size) && (size <= RSU_DEST_IP_MAX)){
                      ntcip_1218_mib.rsuHostNetMask_length = size;
                      memcpy(ntcip_1218_mib.rsuHostNetMask, reply_buf, size);
                      memcpy(data_out, ntcip_1218_mib.rsuHostNetMask, size);
                      ret = RSEMIB_OK;
                  } else {
                      DEBUGMSGTL((MY_NAME, "get_rsuHostNetMask: READ bad length=%d.\n",size));
                      ret = RSEMIB_BAD_DATA;
                  }
              } else {
                  DEBUGMSGTL((MY_NAME, "get_rsuHostNetMask: READ fgets failed.\n"));
                  ret = RSEMIB_SYSCALL_FAIL;
              }
              pclose(fp);
          } else {
              DEBUGMSGTL((MY_NAME, "get_rsuHostNetMask: READ request failed.\n"));
              ret = RSEMIB_SYSCALL_FAIL;
          }
          ntcip1218_unlock();
      } else {
          ret = RSEMIB_LOCK_FAIL;
      }
  }
  if(ret < RSEMIB_OK){
      set_ntcip_1218_error_states(ret);
      size = ret;
  }
  return size;
}
int32_t preserve_rsuHostNetMask(void)
{
  if(RSEMIB_OK == ntcip1218_lock()){
      prior_ntcip_1218_mib.rsuHostNetMask_length = ntcip_1218_mib.rsuHostNetMask_length;
      memcpy(prior_ntcip_1218_mib.rsuHostNetMask, ntcip_1218_mib.rsuHostNetMask, RSU_DEST_IP_MAX);
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t action_rsuHostNetMask(uint8_t * data_in, int32_t length)
{
  if((NULL == data_in) && (0 != length)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if((length < RSU_DEST_IP_MIN) || (RSU_DEST_IP_MAX < length)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      tmp_ntcip_1218_mib.rsuHostNetMask_length = length;
      memset(tmp_ntcip_1218_mib.rsuHostNetMask, 0x0, RSU_DEST_IP_MAX);
      if(NULL != data_in){
          memcpy(tmp_ntcip_1218_mib.rsuHostNetMask, data_in, RSU_DEST_IP_MAX);
      }
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t commit_rsuHostNetMask(void)
{
  int32_t ret = RSEMIB_OK;
  char_t  request_buf[150]; /* If you need more then increase for your needs. */
  char_t  reply_buf[50];   
  FILE   *fp;
  char_t  data[RSU_DEST_IP_MAX+1]; /* Make sure null terminated for string function. */

  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuHostNetMask_length = tmp_ntcip_1218_mib.rsuHostNetMask_length;
      memcpy(ntcip_1218_mib.rsuHostNetMask, tmp_ntcip_1218_mib.rsuHostNetMask, RSU_DEST_IP_MAX);
      memset(tmp_ntcip_1218_mib.rsuHostNetMask, 0x0, RSU_DEST_IP_MAX);

      memset(reply_buf,'\0',sizeof(reply_buf));
      memset(data,'\0',sizeof(data));
      memcpy(data,ntcip_1218_mib.rsuHostNetMask,ntcip_1218_mib.rsuHostNetMask_length);
      snprintf(request_buf,sizeof(request_buf), DCU_WRITE_ETH0_NETMASK, data);
      DEBUGMSGTL((MY_NAME, "commit_rsuHostNetMask: WRITE[%s]\n",request_buf));
      if(NULL != (fp = popen(request_buf, "r"))){
          if(NULL != fgets(reply_buf, sizeof(reply_buf), fp)){
              ret = RSEMIB_OK; /* Don't verify off WRITE. Successive WRITE of same value return different values. */
          } else {
              DEBUGMSGTL((MY_NAME, "commit_rsuHostNetMask: WRITE fgets failed.\n"));
              ret = RSEMIB_SYSCALL_FAIL;
          }
          pclose(fp);
      } else {
          DEBUGMSGTL((MY_NAME, "commit_rsuHostNetMask: WRITE request failed.\n"));
          ret = RSEMIB_SYSCALL_FAIL;
      }
      ntcip1218_unlock();
  } else {
      ret = RSEMIB_LOCK_FAIL;
  }
  if(ret < RSEMIB_OK) 
      set_ntcip_1218_error_states(ret);  
  return ret;
}
int32_t undo_rsuHostNetMask(void)
{
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuHostNetMask_length = prior_ntcip_1218_mib.rsuHostNetMask_length;
      memcpy(ntcip_1218_mib.rsuHostNetMask, prior_ntcip_1218_mib.rsuHostNetMask, RSU_DEST_IP_MAX);
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
/* 5.15.20 */
int32_t get_rsuHostGateway(uint8_t * data_out)
{
  int32_t size = 0;
  int32_t ret = RSEMIB_OK;
  char_t  request_buf[150]; /* If you need more then increase for your needs. */
  char_t  reply_buf[50];   
  FILE   *fp;

  if(NULL == data_out){
      ret = RSEMIB_BAD_INPUT;
  } else {
      if(RSEMIB_OK == ntcip1218_lock()){
          memset(reply_buf,'\0',sizeof(reply_buf));
          snprintf(request_buf,sizeof(request_buf), DCU_READ_ETH0_GATEWAY);
          if(NULL != (fp = popen(request_buf, "r"))){
              if(NULL != fgets(reply_buf, sizeof(reply_buf), fp)){
                  size = strlen(reply_buf);
                  if((RSU_DEST_IP_MIN <= size) && (size <= RSU_DEST_IP_MAX)){
                      ntcip_1218_mib.rsuHostGateway_length = size;
                      memcpy(ntcip_1218_mib.rsuHostGateway, reply_buf, size);
                      memcpy(data_out, ntcip_1218_mib.rsuHostGateway, size);
                      ret = RSEMIB_OK;
                  } else {
                      DEBUGMSGTL((MY_NAME, "get_rsuHostGateway: READ bad length=%d.\n",size));
                      ret = RSEMIB_BAD_DATA;
                  }
              } else {
                  DEBUGMSGTL((MY_NAME, "get_rsuHostGateway: READ fgets failed.\n"));
                  ret = RSEMIB_SYSCALL_FAIL;
              }
              pclose(fp);
          } else {
              DEBUGMSGTL((MY_NAME, "get_rsuHostGateway: READ request failed.\n"));
              ret = RSEMIB_SYSCALL_FAIL;
          }
          ntcip1218_unlock();
      } else {
          ret = RSEMIB_LOCK_FAIL;
      }
  }
  if(ret < RSEMIB_OK){
      set_ntcip_1218_error_states(ret);
      size = ret;
  }
  return size;
}
int32_t preserve_rsuHostGateway(void)
{
  if(RSEMIB_OK == ntcip1218_lock()){
      prior_ntcip_1218_mib.rsuHostGateway_length = ntcip_1218_mib.rsuHostGateway_length;
      memcpy(prior_ntcip_1218_mib.rsuHostGateway, ntcip_1218_mib.rsuHostGateway, RSU_DEST_IP_MAX);
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t action_rsuHostGateway(uint8_t * data_in, int32_t length)
{
  if((NULL == data_in) && (0 != length)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if((length < RSU_DEST_IP_MIN) || (RSU_DEST_IP_MAX < length)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      tmp_ntcip_1218_mib.rsuHostGateway_length = length;
      memset(tmp_ntcip_1218_mib.rsuHostGateway, 0x0, RSU_DEST_IP_MAX);
      if(NULL != data_in){
          memcpy(tmp_ntcip_1218_mib.rsuHostGateway, data_in, RSU_DEST_IP_MAX);
      }
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t commit_rsuHostGateway(void)
{
  int32_t ret = RSEMIB_OK;
  char_t  request_buf[150]; /* If you need more then increase for your needs. */
  char_t  reply_buf[50];   
  FILE   *fp;
  char_t  data[RSU_DEST_IP_MAX+1]; /* Make sure null terminated for string function. */

  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuHostGateway_length = tmp_ntcip_1218_mib.rsuHostGateway_length;
      memcpy(ntcip_1218_mib.rsuHostGateway, tmp_ntcip_1218_mib.rsuHostGateway, RSU_DEST_IP_MAX);
      memset(tmp_ntcip_1218_mib.rsuHostGateway, 0x0, RSU_DEST_IP_MAX);

      memset(reply_buf,'\0',sizeof(reply_buf));
      memset(data,'\0',sizeof(data));
      memcpy(data,ntcip_1218_mib.rsuHostGateway,ntcip_1218_mib.rsuHostGateway_length);
      snprintf(request_buf,sizeof(request_buf), DCU_WRITE_ETH0_GATEWAY, data);
      DEBUGMSGTL((MY_NAME, "commit_rsuHostNetMask: WRITE[%s]\n",request_buf));
      if(NULL != (fp = popen(request_buf, "r"))){
          if(NULL != fgets(reply_buf, sizeof(reply_buf), fp)){
              ret = RSEMIB_OK; /* Don't verify off WRITE. Successive WRITE of same value return different values. */
          } else {
              DEBUGMSGTL((MY_NAME, "commit_rsuHostGateway: WRITE fgets failed.\n"));
              ret = RSEMIB_SYSCALL_FAIL;
          }
          pclose(fp);
      } else {
          DEBUGMSGTL((MY_NAME, "commit_rsuHostGateway: WRITE request failed.\n"));
          ret = RSEMIB_SYSCALL_FAIL;
      }
      ntcip1218_unlock();
  } else {
      ret = RSEMIB_LOCK_FAIL;
  }
  if(ret < RSEMIB_OK) 
      set_ntcip_1218_error_states(ret);  
  return ret;
}
int32_t undo_rsuHostGateway(void)
{
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuHostGateway_length = prior_ntcip_1218_mib.rsuHostGateway_length;
      memcpy(ntcip_1218_mib.rsuHostGateway, prior_ntcip_1218_mib.rsuHostGateway, RSU_DEST_IP_MAX);
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
/* 5.15.21 */
int32_t get_rsuHostDNS(uint8_t * data_out)
{
  int32_t size = 0;
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      size = ntcip_1218_mib.rsuHostDNS_length;
      memcpy(data_out, ntcip_1218_mib.rsuHostDNS, RSU_DEST_IP_MAX);
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
  if((size < RSU_DEST_IP_MIN) || (RSU_DEST_IP_MAX < size)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);  
      return RSEMIB_BAD_DATA;
  }
  return size;
}
int32_t preserve_rsuHostDNS(void)
{
  if(RSEMIB_OK == ntcip1218_lock()){
      prior_ntcip_1218_mib.rsuHostDNS_length = ntcip_1218_mib.rsuHostDNS_length;
      memcpy(prior_ntcip_1218_mib.rsuHostDNS, ntcip_1218_mib.rsuHostDNS, RSU_DEST_IP_MAX);
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t action_rsuHostDNS(uint8_t * data_in, int32_t length)
{
  if((NULL == data_in) && (0 != length)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if((length < RSU_DEST_IP_MIN) || (RSU_DEST_IP_MAX < length)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      tmp_ntcip_1218_mib.rsuHostDNS_length = length;
      memset(tmp_ntcip_1218_mib.rsuHostDNS, 0x0, RSU_DEST_IP_MAX);
      if(NULL != data_in){
          memcpy(tmp_ntcip_1218_mib.rsuHostDNS, data_in, RSU_DEST_IP_MAX);
      }
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t commit_rsuHostDNS(void)
{
  int32_t ret = RSEMIB_OK;
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuHostDNS_length = tmp_ntcip_1218_mib.rsuHostDNS_length;
      memcpy(ntcip_1218_mib.rsuHostDNS, tmp_ntcip_1218_mib.rsuHostDNS, RSU_DEST_IP_MAX);
      memset(tmp_ntcip_1218_mib.rsuHostDNS, 0x0, RSU_DEST_IP_MAX);
      tmp_ntcip_1218_mib.rsuHostDNS_length = 0;
      if(RSEMIB_OK != (ret = commit_rsuSysSettings_to_disk())) {
          ret = RSEMIB_BAD_DATA;
      }
      ntcip1218_unlock();
  } else {
      ret = RSEMIB_LOCK_FAIL;
  }
  if(ret < RSEMIB_OK) 
      set_ntcip_1218_error_states(ret);  
  return ret;
}
int32_t undo_rsuHostDNS(void)
{
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuHostDNS_length = prior_ntcip_1218_mib.rsuHostDNS_length;
      memcpy(ntcip_1218_mib.rsuHostDNS, prior_ntcip_1218_mib.rsuHostDNS, RSU_DEST_IP_MAX);
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}

/* 5.15.22 */
int32_t get_rsuHostDHCPEnable(void)
{
  int32_t data = 0;
  int32_t ret = RSEMIB_OK;
  char_t  request_buf[150]; /* If you need more then increase for your needs. */
  char_t  reply_buf[50];
  FILE   *fp;

  memset(reply_buf,'\0',sizeof(reply_buf));
  snprintf(request_buf,sizeof(request_buf), DCU_READ_ETH0_DHCP);
  if(NULL != (fp = popen(request_buf, "r"))){
      if(NULL != fgets(reply_buf, sizeof(reply_buf), fp)){
          data = strtod(reply_buf, NULL);
          ret = RSEMIB_OK;
      } else {
          DEBUGMSGTL((MY_NAME, "get_rsuHostDHCPEnable: READ fgets failed.\n"));
          ret = RSEMIB_SYSCALL_FAIL;
      }
      pclose(fp);
  } else {
      DEBUGMSGTL((MY_NAME, "get_rsuHostDHCPEnable: READ request failed.\n"));
      ret = RSEMIB_SYSCALL_FAIL;
  }
  if(RSEMIB_OK == ret){
      if((0 == data) || (1 == data)){
          ret = RSEMIB_OK;
      } else {
          ret = RSEMIB_BAD_DATA;
      }
  }
  if((RSEMIB_OK == ret) && (RSEMIB_OK == ntcip1218_lock())){
      ntcip_1218_mib.rsuHostDHCPEnable = data;
      ntcip1218_unlock();
  } else {
      ret = RSEMIB_LOCK_FAIL;
  }
  if(ret < RSEMIB_OK)
      set_ntcip_1218_error_states(ret);
  if(RSEMIB_OK == ret){
      return data;
  } else {
      return ret;
  }
}
int32_t preserve_rsuHostDHCPEnable (void)
{
  if(RSEMIB_OK == ntcip1218_lock()){
      prior_ntcip_1218_mib.rsuHostDHCPEnable = ntcip_1218_mib.rsuHostDHCPEnable;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t action_rsuHostDHCPEnable (int32_t data)
{
  if((data < 1) || (2 < data)){ 
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  }
  if(RSEMIB_OK == ntcip1218_lock()){  
      tmp_ntcip_1218_mib.rsuHostDHCPEnable = data;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}
//TODO: Do not allow DHCP to be enable here. No proper support for resolv.conf here.
int32_t commit_rsuHostDHCPEnable (void)
{
  int32_t ret = RSEMIB_OK;
#if 0
  char_t  request_buf[150]; /* If you need more then increase for your needs. */
  char_t  reply_buf[50];   
  FILE   *fp;
  int32_t data;
#endif

  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuHostDHCPEnable = tmp_ntcip_1218_mib.rsuHostDHCPEnable;
      tmp_ntcip_1218_mib.rsuHostDHCPEnable = 0;
#if 0
      data = ntcip_1218_mib.rsuHostDHCPEnable;
      memset(reply_buf,'\0',sizeof(reply_buf));
      snprintf(request_buf,sizeof(request_buf), DCU_WRITE_ETH0_DHCP, data);
      DEBUGMSGTL((MY_NAME, "commit_rsuRadioEnable: WRITE[%s]\n",request_buf));

      if(NULL != (fp = popen(request_buf, "r"))){
          if(NULL != fgets(reply_buf, sizeof(reply_buf), fp)){
              ret = RSEMIB_OK; /* Don't verify off WRITE. Successive WRITE of same value return different values. */
          } else {
              DEBUGMSGTL((MY_NAME, "commit_rsuHostDHCPEnable: WRITE fgets failed.\n"));
              ret = RSEMIB_SYSCALL_FAIL;
          }
          pclose(fp);
      } else {
          DEBUGMSGTL((MY_NAME, "commit_rsuHostDHCPEnable: WRITE request failed.\n"));
          ret = RSEMIB_SYSCALL_FAIL;
      }
#endif
      ntcip1218_unlock();
  } else {
      ret = RSEMIB_LOCK_FAIL;
  }
  if(ret < RSEMIB_OK)
      set_ntcip_1218_error_states(ret);
  return ret;
}
int32_t undo_rsuHostDHCPEnable (void)
{
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuHostDHCPEnable = prior_ntcip_1218_mib.rsuHostDHCPEnable ;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}

/******************************************************************************
 * 5.16 Antenna Settings:{ rsu 15 }: rsuAntennaTable.                       
 ******************************************************************************/

/* 5.16.1 */
int32_t get_maxRsuAntennas(void)
{
  int32_t data = RSU_ANTENNA_MAX;

  if(RSEMIB_OK == ntcip1218_lock()){
      data = ntcip_1218_mib.maxRsuAntennas;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  } 
  if((RSU_ANTENNA_MIN <= data) && (data <= RSU_ANTENNA_MAX)){
      return data; 
  } else {
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } 
}

/* 5.16.2.2  */
int32_t get_rsuAntLat(int32_t index, int32_t * data_out)
{
  int32_t data = 0;
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if((index < 1) || (ntcip_1218_mib.maxRsuAntennas < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      data = ntcip_1218_mib.rsuAntennaTable[index-1].rsuAntLat;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  } 
  if((RSU_LAT_MIN <= data) && (data <= RSU_LAT_MAX)){
      *data_out = data;
      return RSEMIB_OK; 
  } else {
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } 
}
int32_t preserve_rsuAntLat(int32_t index)
{
  if((index < 1) || (ntcip_1218_mib.maxRsuAntennas < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      prior_ntcip_1218_mib.rsuAntennaTable[index-1].rsuAntLat = ntcip_1218_mib.rsuAntennaTable[index-1].rsuAntLat;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t action_rsuAntLat(int32_t index,int32_t data_in)
{
  if((index < 1) || (ntcip_1218_mib.maxRsuAntennas < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if((data_in < RSU_LAT_MIN) || (RSU_LAT_MAX < data_in)){ 
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  }
  if(RSEMIB_OK == ntcip1218_lock()){  
      tmp_ntcip_1218_mib.rsuAntennaTable[index-1].rsuAntLat = data_in;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t commit_rsuAntLat(int32_t index)
{
  int32_t ret = RSEMIB_OK;
  if((index < 1) || (ntcip_1218_mib.maxRsuAntennas < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      ret = RSEMIB_BAD_INDEX;
  } else {
      if(RSEMIB_OK == ntcip1218_lock()){
          ntcip_1218_mib.rsuAntennaTable[index-1].rsuAntLat     = tmp_ntcip_1218_mib.rsuAntennaTable[index-1].rsuAntLat;
          tmp_ntcip_1218_mib.rsuAntennaTable[index-1].rsuAntLat = RSU_LAT_UNKNOWN;
          /* Update file on disk. */
          if(RSEMIB_OK != commit_rsuAntenna_to_disk(&ntcip_1218_mib.rsuAntennaTable[index-1])){
              DEBUGMSGTL((MY_NAME, "commit_rsuAntenna_to_disk: FAIL: index=%d dir=[%s].\n", index, ntcip_1218_mib.rsuAntennaTable[index-1].filePathName));
              ret = RSEMIB_BAD_DATA;
          }
          ntcip1218_unlock();
      } else {
          ret = RSEMIB_LOCK_FAIL;
      }
  }
  return ret;
}
int32_t undo_rsuAntLat(int32_t index)
{
  if((index < 1) || (ntcip_1218_mib.maxRsuAntennas < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuAntennaTable[index-1].rsuAntLat = prior_ntcip_1218_mib.rsuAntennaTable[index-1].rsuAntLat;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}

/* 5.16.2.3  */
int32_t get_rsuAntLong(int32_t index, int32_t * data_out)
{
  int32_t data = 0;
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if((index < 1) || (ntcip_1218_mib.maxRsuAntennas < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      data = ntcip_1218_mib.rsuAntennaTable[index-1].rsuAntLong;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  } 
  if((RSU_LON_MIN <= data) && (data <= RSU_LON_MAX)){
      *data_out = data; 
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } 
}
int32_t preserve_rsuAntLong(int32_t index)
{
  if((index < 1) || (ntcip_1218_mib.maxRsuAntennas < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      prior_ntcip_1218_mib.rsuAntennaTable[index-1].rsuAntLong = ntcip_1218_mib.rsuAntennaTable[index-1].rsuAntLong;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t action_rsuAntLong(int32_t index,int32_t data_in)
{
  if((index < 1) || (ntcip_1218_mib.maxRsuAntennas < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if((data_in < RSU_LON_MIN) || (RSU_LON_MAX < data_in)){ 
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  }
  if(RSEMIB_OK == ntcip1218_lock()){  
      tmp_ntcip_1218_mib.rsuAntennaTable[index-1].rsuAntLong = data_in;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t commit_rsuAntLong(int32_t index)
{
  int32_t ret = RSEMIB_OK;
  if((index < 1) || (ntcip_1218_mib.maxRsuAntennas < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      ret =  RSEMIB_BAD_INDEX;
  } else {
      if(RSEMIB_OK == ntcip1218_lock()){
          ntcip_1218_mib.rsuAntennaTable[index-1].rsuAntLong     = tmp_ntcip_1218_mib.rsuAntennaTable[index-1].rsuAntLong;
          tmp_ntcip_1218_mib.rsuAntennaTable[index-1].rsuAntLong = RSU_LON_UNKNOWN;
          /* Update file on disk. */
          if(RSEMIB_OK != commit_rsuAntenna_to_disk(&ntcip_1218_mib.rsuAntennaTable[index-1])){
              DEBUGMSGTL((MY_NAME, "commit_rsuAntenna_to_disk: FAIL: index=%d dir=[%s].\n", index, ntcip_1218_mib.rsuAntennaTable[index-1].filePathName));
              ret = RSEMIB_BAD_DATA;
          }
          ntcip1218_unlock();
      } else {
          ret = RSEMIB_LOCK_FAIL;
      }
  }
  return ret;
}
int32_t undo_rsuAntLong(int32_t index)
{
  if((index < 1) || (ntcip_1218_mib.maxRsuAntennas < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuAntennaTable[index-1].rsuAntLong = prior_ntcip_1218_mib.rsuAntennaTable[index-1].rsuAntLong;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}
/* 5.16.2.4  */
int32_t get_rsuAntElv(int32_t index, int32_t * data_out)
{
  int32_t data = 0;
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if((index < 1) || (ntcip_1218_mib.maxRsuAntennas < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      data = ntcip_1218_mib.rsuAntennaTable[index-1].rsuAntElv;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  } 
  if((RSU_ELV_MIN <= data) && (data <= RSU_ELV_MAX)){
      *data_out = data; 
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } 
}
int32_t preserve_rsuAntElv(int32_t index)
{
  if((index < 1) || (ntcip_1218_mib.maxRsuAntennas < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      prior_ntcip_1218_mib.rsuAntennaTable[index-1].rsuAntElv = ntcip_1218_mib.rsuAntennaTable[index-1].rsuAntElv;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t action_rsuAntElv(int32_t index,int32_t data_in)
{
  if((index < 1) || (ntcip_1218_mib.maxRsuAntennas < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if((data_in < RSU_ELV_MIN) || (RSU_ELV_MAX < data_in)){ 
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  }
  if(RSEMIB_OK == ntcip1218_lock()){  
      tmp_ntcip_1218_mib.rsuAntennaTable[index-1].rsuAntElv = data_in;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t commit_rsuAntElv(int32_t index)
{
  int32_t ret = RSEMIB_OK;
  if((index < 1) || (ntcip_1218_mib.maxRsuAntennas < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      ret = RSEMIB_BAD_INDEX;
  } else {
      if(RSEMIB_OK == ntcip1218_lock()){
          ntcip_1218_mib.rsuAntennaTable[index-1].rsuAntElv     = tmp_ntcip_1218_mib.rsuAntennaTable[index-1].rsuAntElv;
          tmp_ntcip_1218_mib.rsuAntennaTable[index-1].rsuAntElv = RSU_ELV_UNKNOWN;
          /* Update file on disk. */
          if(RSEMIB_OK != commit_rsuAntenna_to_disk(&ntcip_1218_mib.rsuAntennaTable[index-1])){
              DEBUGMSGTL((MY_NAME, "commit_rsuAntenna_to_disk: FAIL: index=%d dir=[%s].\n", index, ntcip_1218_mib.rsuAntennaTable[index-1].filePathName));
              ret = RSEMIB_BAD_DATA;
          }
          ntcip1218_unlock();
      } else {
          ret = RSEMIB_LOCK_FAIL;
      }
  }
  return ret;
}
int32_t undo_rsuAntElv(int32_t index)
{
  if((index < 1) || (ntcip_1218_mib.maxRsuAntennas < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuAntennaTable[index-1].rsuAntElv = prior_ntcip_1218_mib.rsuAntennaTable[index-1].rsuAntElv;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}

/* 5.16.2.5  */
int32_t get_rsuAntGain(int32_t index, int32_t * data_out)
{
  int32_t data = 0;
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if((index < 1) || (ntcip_1218_mib.maxRsuAntennas < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      data = ntcip_1218_mib.rsuAntennaTable[index-1].rsuAntGain;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  } 
  if((RSU_ANTENNA_GAIN_MIN <= data) && (data <= RSU_ANTENNA_GAIN_MAX)){
      *data_out =  data;
      return RSEMIB_OK; 
  } else {
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } 
}
int32_t preserve_rsuAntGain(int32_t index)
{
  if((index < 1) || (ntcip_1218_mib.maxRsuAntennas < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      prior_ntcip_1218_mib.rsuAntennaTable[index-1].rsuAntGain = ntcip_1218_mib.rsuAntennaTable[index-1].rsuAntGain;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t action_rsuAntGain(int32_t index,int32_t data_in)
{
  if((index < 1) || (ntcip_1218_mib.maxRsuAntennas < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if((data_in < RSU_ANTENNA_GAIN_MIN) || (RSU_ANTENNA_GAIN_MAX < data_in)){ 
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  }
  if(RSEMIB_OK == ntcip1218_lock()){  
      tmp_ntcip_1218_mib.rsuAntennaTable[index-1].rsuAntGain = data_in;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t commit_rsuAntGain(int32_t index)
{
  int32_t ret = RSEMIB_OK;
  if((index < 1) || (ntcip_1218_mib.maxRsuAntennas < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      ret = RSEMIB_BAD_INDEX;
  } else {
      if(RSEMIB_OK == ntcip1218_lock()){
          ntcip_1218_mib.rsuAntennaTable[index-1].rsuAntGain     = tmp_ntcip_1218_mib.rsuAntennaTable[index-1].rsuAntGain;
          tmp_ntcip_1218_mib.rsuAntennaTable[index-1].rsuAntGain = RSU_ANTENNA_GAIN_DEFAULT;
          /* Update file on disk. */
          if(RSEMIB_OK != commit_rsuAntenna_to_disk(&ntcip_1218_mib.rsuAntennaTable[index-1])){
              DEBUGMSGTL((MY_NAME, "commit_rsuAntenna_to_disk: FAIL: index=%d dir=[%s].\n", index, ntcip_1218_mib.rsuAntennaTable[index-1].filePathName));
              ret = RSEMIB_BAD_DATA;
          }
          ntcip1218_unlock();
      } else {
          ret = RSEMIB_LOCK_FAIL;
      }
  }
  return ret;
}
int32_t undo_rsuAntGain(int32_t index)
{
  if((index < 1) || (ntcip_1218_mib.maxRsuAntennas < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuAntennaTable[index-1].rsuAntGain = prior_ntcip_1218_mib.rsuAntennaTable[index-1].rsuAntGain;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}
/* 5.16.2.6  */
int32_t get_rsuAntDirection(int32_t index)
{
  int32_t data = 0;
  if((index < 1) || (ntcip_1218_mib.maxRsuAntennas < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      data = ntcip_1218_mib.rsuAntennaTable[index-1].rsuAntDirection;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  } 
  if((RSU_ANTENNA_DIRECTION_MIN <= data) && (data <= RSU_ANTENNA_DIRECTION_MAX)){
      return data; 
  } else {
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } 
}
int32_t preserve_rsuAntDirection(int32_t index)
{
  if((index < 1) || (ntcip_1218_mib.maxRsuAntennas < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      prior_ntcip_1218_mib.rsuAntennaTable[index-1].rsuAntDirection = ntcip_1218_mib.rsuAntennaTable[index-1].rsuAntDirection;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t action_rsuAntDirection(int32_t index,int32_t data_in)
{
  if((index < 1) || (ntcip_1218_mib.maxRsuAntennas < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if((data_in < RSU_ANTENNA_DIRECTION_MIN) || (RSU_ANTENNA_DIRECTION_MAX < data_in)){ 
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  }
  if(RSEMIB_OK == ntcip1218_lock()){  
      tmp_ntcip_1218_mib.rsuAntennaTable[index-1].rsuAntDirection = data_in;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t commit_rsuAntDirection(int32_t index)
{
  int32_t ret = RSEMIB_OK;
  if((index < 1) || (ntcip_1218_mib.maxRsuAntennas < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      ret = RSEMIB_BAD_INDEX;
  } else {
      if(RSEMIB_OK == ntcip1218_lock()){
          ntcip_1218_mib.rsuAntennaTable[index-1].rsuAntDirection     = tmp_ntcip_1218_mib.rsuAntennaTable[index-1].rsuAntDirection;
          tmp_ntcip_1218_mib.rsuAntennaTable[index-1].rsuAntDirection = RSU_ANTENNA_DIRECTION_DEFAULT;
          /* Update file on disk. */
          if(RSEMIB_OK != commit_rsuAntenna_to_disk(&ntcip_1218_mib.rsuAntennaTable[index-1])){
              DEBUGMSGTL((MY_NAME, "commit_rsuAntenna_to_disk: FAIL: index=%d dir=[%s].\n", index, ntcip_1218_mib.rsuAntennaTable[index-1].filePathName));
              ret = RSEMIB_BAD_DATA;
          }
          ntcip1218_unlock();
      } else {
          ret = RSEMIB_LOCK_FAIL;
      }
  }
  return ret;
}
int32_t undo_rsuAntDirection(int32_t index)
{
  if((index < 1) || (ntcip_1218_mib.maxRsuAntennas < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuAntennaTable[index-1].rsuAntDirection = prior_ntcip_1218_mib.rsuAntennaTable[index-1].rsuAntDirection;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}

/******************************************************************************
 * 5.17 rsuSystemStatus : { rsu 16 }
 ******************************************************************************/

/* 5.17.1 */
int32_t get_rsuChanStatus(void)
{
  int32_t data = 0;
  if(RSEMIB_OK == ntcip1218_lock()){
      data = ntcip_1218_mib.rsuChanStatus;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  } 
  if((chanStat_bothOp <= data) && (data <= chanStat_noneOp)){
      return data; 
  } else {
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } 
}

/* 5.17.2 */
int32_t get_rsuMode(void)
{
  int32_t data = 0;
  if(RSEMIB_OK == ntcip1218_lock()){
      data = ntcip_1218_mib.rsuMode ;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  } 
  if((rsuMode_other <= data) && (data <= rsuMode_operate)){
      return data; 
  } else {
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } 
}
int32_t preserve_rsuMode (void)
{
  if(RSEMIB_OK == ntcip1218_lock()){
      prior_ntcip_1218_mib.rsuMode = ntcip_1218_mib.rsuMode;
      prior_ntcip_1218_mib.rsuModeStatus = ntcip_1218_mib.rsuModeStatus;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}
/* If rsuModeStatus != operate or standby, reject request. RSU is broken somehow. */
int32_t action_rsuMode (int32_t data)
{
  int32_t ret = RSEMIB_OK;
  if((data != rsuMode_standby) && (data != rsuMode_operate)){ 
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      ret = RSEMIB_BAD_DATA;
  } else {
      if(RSEMIB_OK == ntcip1218_lock()){
          if(rsuModeStatus_other != ntcip_1218_mib.rsuModeStatus) {
              tmp_ntcip_1218_mib.rsuMode = data;
              ret = RSEMIB_OK;
          } else {
              ret = RSEMIB_BAD_DATA;
          }
          ntcip1218_unlock();
      } else {
          ret = RSEMIB_LOCK_FAIL;
      }
  }
  return ret;
}
int32_t commit_rsuMode (void)
{
  int32_t ret = RSEMIB_OK;
  if(RSEMIB_OK == ntcip1218_lock()){
      switch(tmp_ntcip_1218_mib.rsuMode){
          case rsuMode_other:
              ret = RSEMIB_BAD_DATA;
              break;
          case rsuMode_standby:
              ntcip_1218_mib.rsuMode       = tmp_ntcip_1218_mib.rsuMode;
              ntcip_1218_mib.rsuModeStatus = rsuModeStatus_standby;

/* TODO: Store across reset, silence ifclog and iwmh. */
              ret = RSEMIB_OK;
              break;
          case rsuMode_operate:
              ntcip_1218_mib.rsuMode       = tmp_ntcip_1218_mib.rsuMode;
              ntcip_1218_mib.rsuModeStatus = rsuModeStatus_operate;
              ret = RSEMIB_OK;
              break;
          default: /* Should never happen. */
              set_ntcip_1218_error_states(RSEMIB_BAD_MODE);
              ret = RSEMIB_BAD_MODE;
              break;
      }
      tmp_ntcip_1218_mib.rsuMode = 0;
      ntcip1218_unlock();
  } else {
      ret = RSEMIB_LOCK_FAIL;
  }
  return ret;
}
int32_t undo_rsuMode (void)
{
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuMode = prior_ntcip_1218_mib.rsuMode;
      ntcip_1218_mib.rsuModeStatus = prior_ntcip_1218_mib.rsuModeStatus;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}
/* 5.17.3 */ 
int32_t get_rsuModeStatus(void)
{
  int32_t data = 0;
  if(RSEMIB_OK == ntcip1218_lock()){
      data = ntcip_1218_mib.rsuModeStatus;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  } 
  if((rsuModeStatus_other <= data) && (data <= rsuModeStatus_fault)){
      return data; 
  } else {
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } 
}
/* 5.17.4 */
int32_t get_rsuReboot(void)
{
  int32_t data = 0;
  if(RSEMIB_OK == ntcip1218_lock()){
      data = ntcip_1218_mib.rsuReboot;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  } 
  if((0 <= data) && (data <= 1)){
      return data; 
  } else {
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } 
}
int32_t preserve_rsuReboot (void)
{
  if(RSEMIB_OK == ntcip1218_lock()){
      prior_ntcip_1218_mib.rsuReboot  = ntcip_1218_mib.rsuReboot;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t action_rsuReboot (int32_t data)
{
  if((data < 0) || (1 < data)){ 
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  }
  if(RSEMIB_OK == ntcip1218_lock()){  
      tmp_ntcip_1218_mib.rsuReboot  = data;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}
/* This shoul be a SOFT reset. This is a HARD reset to accomodate rsuInstallUpdate. */
int32_t commit_rsuReboot (void)
{
  int32_t ret = RSEMIB_OK;
  char_t cmd[100 + RSU_MSG_FILENAME_LENGTH_MAX]; /* If you need more than a 100 chars for your command then add them here. */

  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuReboot = 0; /* Nothing to preserve beyond this. */
      if(1 == tmp_ntcip_1218_mib.rsuReboot) {
          tmp_ntcip_1218_mib.rsuReboot = 0;
          memset(cmd,'\0',sizeof(cmd));
          /* Create Install Update command. */
          snprintf(cmd, sizeof(cmd), "reboot");
          DEBUGMSGTL((MY_NAME, "commit_rsuReboot: cmd=[%s] in 3 seconds.\n",cmd));
          if(0 != system(cmd)){ /* This will look weird to client cause no ack or anything. */
              ret = RSEMIB_BAD_DATA;
          }
      }
      ntcip1218_unlock();
  } else {
      ret = RSEMIB_LOCK_FAIL;
  }
  return ret;
}
/* There really isn't undo from a reset. */
int32_t undo_rsuReboot (void)
{
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuReboot = prior_ntcip_1218_mib.rsuReboot;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}
/* 5.17.5 */
int32_t get_rsuClockSource(void)
{
  int32_t data = 0;
  if(RSEMIB_OK == ntcip1218_lock()){
      data = ntcip_1218_mib.rsuClockSource;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  } 
  if((clockSource_other <= data) && (data <= clockSource_ntp)){
      return data; 
  } else {
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } 
}
/* 5.17.6 */
int32_t get_rsuClockSourceStatus(void)
{
  int32_t data = 0;
  if(RSEMIB_OK == ntcip1218_lock()){
      data = ntcip_1218_mib.rsuClockSourceStatus;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  } 
  if((clockSourceStatus_other <= data) && (data <= clockSourceStatus_pendingUpdate)){
      return data; 
  } else {
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } 
}
/* 5.17.7 */
int32_t get_rsuClockSourceTimeout(void)
{
  int32_t data = 0;
  if(RSEMIB_OK == ntcip1218_lock()){
      data = ntcip_1218_mib.rsuClockSourceTimeout;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  } 
  if((RSU_CLOCK_SOURCE_TIMEOUT_MIN <= data) && (data <= RSU_CLOCK_SOURCE_TIMEOUT_MAX)){
      return data; 
  } else {
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } 
}
int32_t preserve_rsuClockSourceTimeout (void)
{
  if(RSEMIB_OK == ntcip1218_lock()){
      prior_ntcip_1218_mib.rsuClockSourceTimeout = ntcip_1218_mib.rsuClockSourceTimeout;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t action_rsuClockSourceTimeout (int32_t data)
{
  if((data < RSU_CLOCK_SOURCE_TIMEOUT_MIN) || (RSU_CLOCK_SOURCE_TIMEOUT_MAX < data)){ 
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  }
  if(RSEMIB_OK == ntcip1218_lock()){  
      tmp_ntcip_1218_mib.rsuClockSourceTimeout = data;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t commit_rsuClockSourceTimeout (void)
{
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuClockSourceTimeout     = tmp_ntcip_1218_mib.rsuClockSourceTimeout; 
      tmp_ntcip_1218_mib.rsuClockSourceTimeout = 0;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t undo_rsuClockSourceTimeout (void)
{
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuClockSourceTimeout = prior_ntcip_1218_mib.rsuClockSourceTimeout;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}
/* 5.17.8 */
int32_t get_rsuClockSourceFailedQuery(void)
{
  int32_t data = 0;
  if(RSEMIB_OK == ntcip1218_lock()){
      data = ntcip_1218_mib.rsuClockSourceFailedQuery;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  } 
  if((RSU_CLOCK_SOURCE_FAILED_QUERY_MIN <= data) && (data <= RSU_CLOCK_SOURCE_FAILED_QUERY_MAX)){
      return data; 
  } else {
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } 
}
int32_t preserve_rsuClockSourceFailedQuery (void)
{
  if(RSEMIB_OK == ntcip1218_lock()){
      prior_ntcip_1218_mib.rsuClockSourceFailedQuery  = ntcip_1218_mib.rsuClockSourceFailedQuery;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t action_rsuClockSourceFailedQuery (int32_t data)
{
  if((data < RSU_CLOCK_SOURCE_FAILED_QUERY_MIN) || (RSU_CLOCK_SOURCE_FAILED_QUERY_MAX < data)){ 
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  }
  if(RSEMIB_OK == ntcip1218_lock()){  
      tmp_ntcip_1218_mib.rsuClockSourceFailedQuery  = data;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t commit_rsuClockSourceFailedQuery (void)
{
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuClockSourceFailedQuery      = tmp_ntcip_1218_mib.rsuClockSourceFailedQuery; 
      tmp_ntcip_1218_mib.rsuClockSourceFailedQuery  = 0;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t undo_rsuClockSourceFailedQuery (void)
{
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuClockSourceFailedQuery  = prior_ntcip_1218_mib.rsuClockSourceFailedQuery;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}
/* 5.17.9 */
int32_t get_rsuClockDeviationTolerance(void)
{
  int32_t data = 0;
  if(RSEMIB_OK == ntcip1218_lock()){
      data = ntcip_1218_mib.rsuClockDeviationTolerance;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  } 
  if(( RSU_CLOCK_DEVIATION_MIN <= data) && (data <= RSU_CLOCK_DEVIATION_MAX)){
      return data; 
  } else {
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } 
}
int32_t preserve_rsuClockDeviationTolerance (void)
{
  if(RSEMIB_OK == ntcip1218_lock()){
      prior_ntcip_1218_mib.rsuClockDeviationTolerance = ntcip_1218_mib.rsuClockDeviationTolerance;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t action_rsuClockDeviationTolerance (int32_t data)
{
  if((data <  RSU_CLOCK_DEVIATION_MIN) || (RSU_CLOCK_DEVIATION_MAX < data)){ 
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  }
  if(RSEMIB_OK == ntcip1218_lock()){  
      tmp_ntcip_1218_mib.rsuClockDeviationTolerance = data;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t commit_rsuClockDeviationTolerance (void)
{
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuClockDeviationTolerance      = tmp_ntcip_1218_mib.rsuClockDeviationTolerance; 
      tmp_ntcip_1218_mib.rsuClockDeviationTolerance  = 0;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t undo_rsuClockDeviationTolerance (void)
{
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuClockDeviationTolerance  = prior_ntcip_1218_mib.rsuClockDeviationTolerance;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}
/* 5.17.10 */
int32_t get_rsuStatus(void)
{
  int32_t data = 0;

  if(RSEMIB_OK == ntcip1218_lock()){
      data = ntcip_1218_mib.rsuStatus;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  } 
  if((rsuStatus_other <= data) && (data <= rsuStatus_unknown)){
      return data; 
  } else {
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } 
}
/******************************************************************************
 * 5.18 Asynchronous Message: {rsu 17}: Current Status from customer POV.
 *   Jira 1517: The RSU shall write an entry in its event log when a notification is 
 *     transmitted to a destination network address and port number. Each entry shall 
 *     contain the following data:
 * 
 *     Date and Time (in UTC) of the notification
 *     The notification message
 *     The notification alert level
 ******************************************************************************/
//TODO: Need to update CFGMGR to get proper creds: HARD CODED
//TODO: If we really were waiting for acks on traps then seperate threads for each would be better? dunno
//    : Therefore, think about breaking up shm locks.
//TODO: weak creds for test with SNMPB only. Do not ship like this.
/* Assumption: This is called ~1Hz or interval logic wont work. */
void mib_send_notifications(uint64_t errorstates)
{
  int32_t ret = RSEMIB_OK;
  char_t  cmd[1024]; /* OIDs can be huge, size accordingly */
#if defined(ENABLE_ASYNC_BACKUP)
  int32_t update = 0;
#endif

  if(RSEMIB_OK == ntcip1218_lock()){
      /* If Alert then send trap */
      /* 5.18.1.1 */
      if(   (rsuAlertLevel_info <= ntcip_1218_mib.Async.messageFileIntegrityError) 
         && (ntcip_1218_mib.Async.messageFileIntegrityError <= rsuAlertLevel_critical)){
#if defined(ENABLE_ASYNC_BACKUP)
          /* No matter what happens there will be some kind of update from here. */
          update = 1;
#endif
          /* Until we hit maxretries keep trying. No ack */
          if(ntcip_1218_mib.Async.rsuMsgFileIntegrityAttempts <= ntcip_1218_mib.Async.rsuNotificationMaxRetries) {

              /* If first attempt ignore interval and send. After that wait till intervals equal each other. */
              if(   (ntcip_1218_mib.Async.rsuMsgFileIntegrityInterval == ntcip_1218_mib.Async.rsuNotificationRepeatInterval)
                 || (0 == ntcip_1218_mib.Async.rsuMsgFileIntegrityAttempts)){

                  memset(cmd,0x0, sizeof(cmd));

                  #if defined(ENABLE_SMNP_V3_SHA)
                  ret = snprintf(cmd,sizeof(cmd),"snmptrap -v 3 -u rsutweiadmin -l authPriv -a SHA -A rsutweiauthpass -x AES-256 -X rsutweiprivpass -mNTCIP1218-v01 udp:%s:%d '' messageFileIntegrityError.0 rsuAlertLevel i %d  rsuMsgFileIntegrityMsg.0 s \"%s\"",
                  #else
                  ret = snprintf(cmd,sizeof(cmd),"snmptrap -v 3 -u rsutweiadmin -l authPriv -a SHA-512 -A rsutweiauthpass -x AES-256 -X rsutweiprivpass -mNTCIP1218-v01 udp:%s:%d '' messageFileIntegrityError.0 rsuAlertLevel i %d  rsuMsgFileIntegrityMsg.0 s \"%s\"",
                  #endif
                      ntcip_1218_mib.rsuNotifyIpAddress,ntcip_1218_mib.rsuNotifyPort
                      ,ntcip_1218_mib.Async.messageFileIntegrityError,ntcip_1218_mib.Async.rsuMsgFileIntegrityMsg);
#if defined(EXTRA_DEBUG)
                  I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"messageFileIntegrity: cmd(%s)\n",cmd);
#endif
                  if(0 < ret) {
                      if(system(cmd)) {
                          ret = RSEMIB_SYSCALL_FAIL;
                          I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"RSEMIB_SYSCALL_FAIL: rsuMsgFileIntegrity: system: errno(%s).\n",strerror(errno));

                          ntcip_1218_mib.Async.messageFileIntegrityError   = rsuAlertLevel_denso_empty;
                          ntcip_1218_mib.Async.rsuMsgFileIntegrityAttempts = 0;
                          ntcip_1218_mib.Async.rsuMsgFileIntegrityInterval = 0;
                      } else { 
                          /* Keep sending till we hit max retries. No ack */
                          ntcip_1218_mib.Async.rsuMsgFileIntegrityAttempts++;
                          ntcip_1218_mib.Async.rsuMsgFileIntegrityInterval = 0;
                          I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"rsuMsgFileIntegrity(%s) level(%d)\n",
                              ntcip_1218_mib.Async.rsuMsgFileIntegrityMsg
                              ,ntcip_1218_mib.Async.messageFileIntegrityError);
                      }
                  } else {
                      ret = RSEMIB_SYSCALL_FAIL;
                      I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"RSEMIB_SYSCALL_FAIL: rsuMsgFileIntegrity: snprintf: errno(%s).\n",strerror(errno));
                      ntcip_1218_mib.Async.messageFileIntegrityError   = rsuAlertLevel_denso_empty;
                      ntcip_1218_mib.Async.rsuMsgFileIntegrityAttempts = 0;
                      ntcip_1218_mib.Async.rsuMsgFileIntegrityInterval = 0;
                  }
              } else {
                  /* Not the first attempt and not at interval yet. */
#if defined(EXTRA_DEBUG)
                  I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"rsuMsgFileIntegrity: Not interval yet(%d)\n",ntcip_1218_mib.Async.rsuMsgFileIntegrityInterval);
#endif
                  ntcip_1218_mib.Async.rsuMsgFileIntegrityInterval++;
              }
          } else {
#if defined(EXTRA_DEBUG)
              I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"rsuMsgFileIntegrity: Hit max retries, stopping(%d)",
                  ntcip_1218_mib.Async.rsuMsgFileIntegrityAttempts);
#endif
              /* On hitting maxretries, stop it. */
              ntcip_1218_mib.Async.messageFileIntegrityError   = rsuAlertLevel_denso_empty;
              ntcip_1218_mib.Async.rsuMsgFileIntegrityAttempts = 0;
              ntcip_1218_mib.Async.rsuMsgFileIntegrityInterval = 0;
          }
      }
  /* 5.18.1.2 */
  if(   (rsuAlertLevel_info <= ntcip_1218_mib.Async.rsuSecStorageIntegrityError) 
     && (ntcip_1218_mib.Async.rsuSecStorageIntegrityError <= rsuAlertLevel_critical)){
#if defined(ENABLE_ASYNC_BACKUP)
      /* No matter what happens there will be some kind of update from here. */
      update = 1;
#endif
      /* Until we hit maxretries keep trying. No ack */
      if(ntcip_1218_mib.Async.rsuSecStorageIntegrityAttempts <= ntcip_1218_mib.Async.rsuNotificationMaxRetries) {

          /* If first attempt ignore interval and send. After that wait till intervals equal each other. */
          if(   (ntcip_1218_mib.Async.rsuSecStorageIntegrityInterval == ntcip_1218_mib.Async.rsuNotificationRepeatInterval)
             || (0 == ntcip_1218_mib.Async.rsuSecStorageIntegrityAttempts)){

              memset(cmd,0x0, sizeof(cmd));
//TODO: weak creds for test with SNMPB only. Do not ship like this.

              #if defined(ENABLE_SMNP_V3_SHA)
              ret = snprintf(cmd,sizeof(cmd),"snmptrap -v 3 -u rsutweiadmin -l authPriv -a SHA -A rsutweiauthpass -x AES-256 -X rsutweiprivpass -mNTCIP1218-v01 udp:%s:%d '' rsuSecStorageIntegrityError.0 rsuAlertLevel i %d  rsuSecStorageIntegrityMsg.0 s \"%s\"",
              #else
              ret = snprintf(cmd,sizeof(cmd),"snmptrap -v 3 -u rsutweiadmin -l authPriv -a SHA-512 -A rsutweiauthpass -x AES-256 -X rsutweiprivpass -mNTCIP1218-v01 udp:%s:%d '' rsuSecStorageIntegrityError.0 rsuAlertLevel i %d  rsuSecStorageIntegrityMsg.0 s \"%s\"",
              #endif
                  ntcip_1218_mib.rsuNotifyIpAddress,ntcip_1218_mib.rsuNotifyPort
                  ,ntcip_1218_mib.Async.rsuSecStorageIntegrityError,ntcip_1218_mib.Async.rsuSecStorageIntegrityMsg);
#if defined(EXTRA_DEBUG)
                  I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"rsuSecStorageIntegrity: cmd(%s)\n",cmd);
#endif
//TODO: In future this is where we wait for ack on UDP port
              if(0 < ret) {
                  if(system(cmd)) {
                      ret = RSEMIB_SYSCALL_FAIL;
                      I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"RSEMIB_SYSCALL_FAIL: rsuSecStorageIntegrity: system: errno(%s).\n",strerror(errno));

                      ntcip_1218_mib.Async.rsuSecStorageIntegrityError   = rsuAlertLevel_denso_empty;
                      ntcip_1218_mib.Async.rsuSecStorageIntegrityAttempts = 0;
                      ntcip_1218_mib.Async.rsuSecStorageIntegrityInterval = 0;
                  } else { 
                      /* Keep sending till we hit max retries. No ack */
                      ntcip_1218_mib.Async.rsuSecStorageIntegrityAttempts++;
                      ntcip_1218_mib.Async.rsuSecStorageIntegrityInterval = 0;
                      I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"rsuSecStorageIntegrity(%s) level(%d)\n",
                          ntcip_1218_mib.Async.rsuSecStorageIntegrityMsg
                          ,ntcip_1218_mib.Async.rsuSecStorageIntegrityError);
                  }
              } else {
                  ret = RSEMIB_SYSCALL_FAIL;
                  I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"RSEMIB_SYSCALL_FAIL: rsuSecStorageIntegrity: snprintf: errno(%s).\n",strerror(errno));

                  ntcip_1218_mib.Async.rsuSecStorageIntegrityError   = rsuAlertLevel_denso_empty;
                  ntcip_1218_mib.Async.rsuSecStorageIntegrityAttempts = 0;
                  ntcip_1218_mib.Async.rsuSecStorageIntegrityInterval = 0;
              }
          } else {
              /* Not the first attempt and not at interval yet. */
              ntcip_1218_mib.Async.rsuSecStorageIntegrityInterval++;
          }
      } else {
          /* On hitting maxretries, stop it. */
          ntcip_1218_mib.Async.rsuSecStorageIntegrityError   = rsuAlertLevel_denso_empty;
          ntcip_1218_mib.Async.rsuSecStorageIntegrityAttempts = 0;
          ntcip_1218_mib.Async.rsuSecStorageIntegrityInterval = 0;
      }
  }
  /* 5.18.1.3 Authorization Error Message: invalid security creds: Is failed login attempt one? */
  if(   (rsuAlertLevel_info <= ntcip_1218_mib.Async.rsuAuthError) 
     && (ntcip_1218_mib.Async.rsuAuthError <= rsuAlertLevel_critical)){
#if defined(ENABLE_ASYNC_BACKUP)
      /* No matter what happens there will be some kind of update from here. */
      update = 1;
#endif
      /* Until we hit maxretries keep trying. No ack */
      if(ntcip_1218_mib.Async.rsuAuthAttempts <= ntcip_1218_mib.Async.rsuNotificationMaxRetries) {

          /* If first attempt ignore interval and send. After that wait till intervals equal each other. */
          if(   (ntcip_1218_mib.Async.rsuAuthInterval == ntcip_1218_mib.Async.rsuNotificationRepeatInterval)
             || (0 == ntcip_1218_mib.Async.rsuAuthAttempts)){

              memset(cmd,0x0, sizeof(cmd));
//TODO: weak creds for test with SNMPB only. Do not ship like this.

              #if defined(ENABLE_SMNP_V3_SHA)
              ret = snprintf(cmd,sizeof(cmd),"snmptrap -v 3 -u rsutweiadmin -l authPriv -a SHA -A rsutweiauthpass -x AES-256 -X rsutweiprivpass -mNTCIP1218-v01 udp:%s:%d '' rsuAuthError.0 rsuAlertLevel i %d  rsuAuthMsg.0 s \"%s\"",
              #else
              ret = snprintf(cmd,sizeof(cmd),"snmptrap -v 3 -u rsutweiadmin -l authPriv -a SHA-512 -A rsutweiauthpass -x AES-256 -X rsutweiprivpass -mNTCIP1218-v01 udp:%s:%d '' rsuAuthError.0 rsuAlertLevel i %d  rsuAuthMsg.0 s \"%s\"",
              #endif
                  ntcip_1218_mib.rsuNotifyIpAddress,ntcip_1218_mib.rsuNotifyPort
                  ,ntcip_1218_mib.Async.rsuAuthError,ntcip_1218_mib.Async.rsuAuthMsg);
#if defined(EXTRA_DEBUG)
                  I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"rsuAuth: cmd(%s)\n",cmd);
#endif
//TODO: In future this is where we wait for ack on UDP port
              if(0 < ret) {
                  if(system(cmd)) {
                      ret = RSEMIB_SYSCALL_FAIL;
                      I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"RSEMIB_SYSCALL_FAIL: rsuAuth: system: errno(%s).\n",strerror(errno));

                      ntcip_1218_mib.Async.rsuAuthError   = rsuAlertLevel_denso_empty;
                      ntcip_1218_mib.Async.rsuAuthAttempts = 0;
                      ntcip_1218_mib.Async.rsuAuthInterval = 0;
                  } else { 
                      /* Keep sending till we hit max retries. No ack */
                      ntcip_1218_mib.Async.rsuAuthAttempts++;
                      ntcip_1218_mib.Async.rsuAuthInterval = 0;
                      I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"rsuAuth(%s) level(%d)\n",
                          ntcip_1218_mib.Async.rsuAuthMsg
                          ,ntcip_1218_mib.Async.rsuAuthError);
                  }
              } else {
                  ret = RSEMIB_SYSCALL_FAIL;
                  I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"RSEMIB_SYSCALL_FAIL: rsuAuth: snprintf: errno(%s).\n",strerror(errno));

                  ntcip_1218_mib.Async.rsuAuthError   = rsuAlertLevel_denso_empty;
                  ntcip_1218_mib.Async.rsuAuthAttempts = 0;
                  ntcip_1218_mib.Async.rsuAuthInterval = 0;
              }
          } else {
              /* Not the first attempt and not at interval yet. */
              ntcip_1218_mib.Async.rsuAuthInterval++;
          }
      } else {
          /* On hitting maxretries, stop it. */
          ntcip_1218_mib.Async.rsuAuthError   = rsuAlertLevel_denso_empty;
          ntcip_1218_mib.Async.rsuAuthAttempts = 0;
          ntcip_1218_mib.Async.rsuAuthInterval = 0;
      }
  }
  /* 5.18.1.4 Signature Verification Error Message: Any failed signature on WSM's. */
  if(   (rsuAlertLevel_info <= ntcip_1218_mib.Async.rsuSignatureVerifyError) 
     && (ntcip_1218_mib.Async.rsuSignatureVerifyError <= rsuAlertLevel_critical)){
#if defined(ENABLE_ASYNC_BACKUP)
      /* No matter what happens there will be some kind of update from here. */
      update = 1;
#endif
      /* Until we hit maxretries keep trying. No ack */
      if(ntcip_1218_mib.Async.rsuSignatureVerifyAttempts <= ntcip_1218_mib.Async.rsuNotificationMaxRetries) {

          /* If first attempt ignore interval and send. After that wait till intervals equal each other. */
          if(   (ntcip_1218_mib.Async.rsuSignatureVerifyInterval == ntcip_1218_mib.Async.rsuNotificationRepeatInterval)
             || (0 == ntcip_1218_mib.Async.rsuSignatureVerifyAttempts)){

              memset(cmd,0x0, sizeof(cmd));
//TODO: weak creds for test with SNMPB only. Do not ship like this.

              #if defined(ENABLE_SMNP_V3_SHA)
              ret = snprintf(cmd,sizeof(cmd),"snmptrap -v 3 -u rsutweiadmin -l authPriv -a SHA -A rsutweiauthpass -x AES-256 -X rsutweiprivpass -mNTCIP1218-v01 udp:%s:%d '' rsuSignatureVerifyError.0 rsuAlertLevel i %d  rsuSignatureVerifyMsg.0 s \"%s\"",
              #else
              ret = snprintf(cmd,sizeof(cmd),"snmptrap -v 3 -u rsutweiadmin -l authPriv -a SHA-512 -A rsutweiauthpass -x AES-256 -X rsutweiprivpass -mNTCIP1218-v01 udp:%s:%d '' rsuSignatureVerifyError.0 rsuAlertLevel i %d  rsuSignatureVerifyMsg.0 s \"%s\"",
              #endif
                  ntcip_1218_mib.rsuNotifyIpAddress,ntcip_1218_mib.rsuNotifyPort
                  ,ntcip_1218_mib.Async.rsuSignatureVerifyError,ntcip_1218_mib.Async.rsuSignatureVerifyMsg);

#if defined(EXTRA_DEBUG)
                  I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"rsuSignatureVerify: cmd(%s)\n",cmd);
#endif
//TODO: In future this is where we wait for ack on UDP port
              if(0 < ret) {
                  if(system(cmd)) {
                      ret = RSEMIB_SYSCALL_FAIL;
                      I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"RSEMIB_SYSCALL_FAIL: rsuSignatureVerify: system: errno(%s).\n",strerror(errno));

                      ntcip_1218_mib.Async.rsuSignatureVerifyError   = rsuAlertLevel_denso_empty;
                      ntcip_1218_mib.Async.rsuSignatureVerifyAttempts = 0;
                      ntcip_1218_mib.Async.rsuSignatureVerifyInterval = 0;
                  } else { 
                      /* Keep sending till we hit max retries. No ack */
                      ntcip_1218_mib.Async.rsuSignatureVerifyAttempts++;
                      ntcip_1218_mib.Async.rsuSignatureVerifyInterval = 0;
                      I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"rsuSignatureVerify(%s) level(%d)\n",
                          ntcip_1218_mib.Async.rsuSignatureVerifyMsg
                          ,ntcip_1218_mib.Async.rsuSignatureVerifyError);
                  }
              } else {
                  ret = RSEMIB_SYSCALL_FAIL;
                  I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"RSEMIB_SYSCALL_FAIL: rsuSignatureVerify: snprintf: errno(%s).\n",strerror(errno));

                  ntcip_1218_mib.Async.rsuSignatureVerifyError   = rsuAlertLevel_denso_empty;
                  ntcip_1218_mib.Async.rsuSignatureVerifyAttempts = 0;
                  ntcip_1218_mib.Async.rsuSignatureVerifyInterval = 0;
              }
          } else {
              /* Not the first attempt and not at interval yet. */
              ntcip_1218_mib.Async.rsuSignatureVerifyInterval++;
          }
      } else {
          /* On hitting maxretries, stop it. */
          ntcip_1218_mib.Async.rsuSignatureVerifyError   = rsuAlertLevel_denso_empty;
          ntcip_1218_mib.Async.rsuSignatureVerifyAttempts = 0;
          ntcip_1218_mib.Async.rsuSignatureVerifyInterval = 0;
      }
  }
  /* 5.18.1.5 Access Error Message: error or rejection due to a violation of the Access Control List.*/
  if(   (rsuAlertLevel_info <= ntcip_1218_mib.Async.rsuAccessError) 
     && (ntcip_1218_mib.Async.rsuAccessError <= rsuAlertLevel_critical)){
#if defined(ENABLE_ASYNC_BACKUP)
      /* No matter what happens there will be some kind of update from here. */
      update = 1;
#endif
      /* Until we hit maxretries keep trying. No ack */
      if(ntcip_1218_mib.Async.rsuAccessAttempts <= ntcip_1218_mib.Async.rsuNotificationMaxRetries) {

          /* If first attempt ignore interval and send. After that wait till intervals equal each other. */
          if(   (ntcip_1218_mib.Async.rsuAccessInterval == ntcip_1218_mib.Async.rsuNotificationRepeatInterval)
             || (0 == ntcip_1218_mib.Async.rsuAccessAttempts)){

              memset(cmd,0x0, sizeof(cmd));
//TODO: weak creds for test with SNMPB only. Do not ship like this.

              #if defined(ENABLE_SMNP_V3_SHA)
              ret = snprintf(cmd,sizeof(cmd),"snmptrap -v 3 -u rsutweiadmin -l authPriv -a SHA -A rsutweiauthpass -x AES-256 -X rsutweiprivpass -mNTCIP1218-v01 udp:%s:%d '' rsuAccessError.0 rsuAlertLevel i %d  rsuAccessMsg.0 s \"%s\"",
              #else
              ret = snprintf(cmd,sizeof(cmd),"snmptrap -v 3 -u rsutweiadmin -l authPriv -a SHA-512 -A rsutweiauthpass -x AES-256 -X rsutweiprivpass -mNTCIP1218-v01 udp:%s:%d '' rsuAccessError.0 rsuAlertLevel i %d  rsuAccessMsg.0 s \"%s\"",
              #endif
                  ntcip_1218_mib.rsuNotifyIpAddress,ntcip_1218_mib.rsuNotifyPort
                  ,ntcip_1218_mib.Async.rsuAccessError,ntcip_1218_mib.Async.rsuAccessMsg);

#if defined(EXTRA_DEBUG)
                  I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"rsuAccess: cmd(%s)\n",cmd);
#endif
//TODO: In future this is where we wait for ack on UDP port
              if(0 < ret) {
                  if(system(cmd)) {
                      ret = RSEMIB_SYSCALL_FAIL;
                      I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"RSEMIB_SYSCALL_FAIL: rsuAccess: system: errno(%s).\n",strerror(errno));

                      ntcip_1218_mib.Async.rsuAccessError   = rsuAlertLevel_denso_empty;
                      ntcip_1218_mib.Async.rsuAccessAttempts = 0;
                      ntcip_1218_mib.Async.rsuAccessInterval = 0;
                  } else { 
                      /* Keep sending till we hit max retries. No ack */
                      ntcip_1218_mib.Async.rsuAccessAttempts++;
                      ntcip_1218_mib.Async.rsuAccessInterval = 0;
                      I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"rsuAccess(%s) level(%d)\n",
                          ntcip_1218_mib.Async.rsuAccessMsg
                          ,ntcip_1218_mib.Async.rsuAccessError);
                  }
              } else {
                  ret = RSEMIB_SYSCALL_FAIL;
                  I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"RSEMIB_SYSCALL_FAIL: rsuAccess: snprintf: errno(%s).\n",strerror(errno));

                  ntcip_1218_mib.Async.rsuAccessError   = rsuAlertLevel_denso_empty;
                  ntcip_1218_mib.Async.rsuAccessAttempts = 0;
                  ntcip_1218_mib.Async.rsuAccessInterval = 0;
              }
          } else {
              /* Not the first attempt and not at interval yet. */
              ntcip_1218_mib.Async.rsuAccessInterval++;
          }
      } else {
          /* On hitting maxretries, stop it. */
          ntcip_1218_mib.Async.rsuAccessError   = rsuAlertLevel_denso_empty;
          ntcip_1218_mib.Async.rsuAccessAttempts = 0;
          ntcip_1218_mib.Async.rsuAccessInterval = 0;
      }
  }
  /* 5.18.1.6 Time Source Lost Message: Lost time source: ie lost GNSS fix. */
  if(   (rsuAlertLevel_info <= ntcip_1218_mib.Async.rsuTimeSourceLost) 
     && (ntcip_1218_mib.Async.rsuTimeSourceLost <= rsuAlertLevel_critical)){
#if defined(ENABLE_ASYNC_BACKUP)
      /* No matter what happens there will be some kind of update from here. */
      update = 1;
#endif
      /* Until we hit maxretries keep trying. No ack */
      if(ntcip_1218_mib.Async.rsuTimeSourceLostAttempts <= ntcip_1218_mib.Async.rsuNotificationMaxRetries) {

          /* If first attempt ignore interval and send. After that wait till intervals equal each other. */
          if(   (ntcip_1218_mib.Async.rsuTimeSourceLostInterval == ntcip_1218_mib.Async.rsuNotificationRepeatInterval)
             || (0 == ntcip_1218_mib.Async.rsuTimeSourceLostAttempts)){

              memset(cmd,0x0, sizeof(cmd));
//TODO: weak creds for test with SNMPB only. Do not ship like this.

              #if defined(ENABLE_SMNP_V3_SHA)
              ret = snprintf(cmd,sizeof(cmd),"snmptrap -v 3 -u rsutweiadmin -l authPriv -a SHA -A rsutweiauthpass -x AES-256 -X rsutweiprivpass -mNTCIP1218-v01 udp:%s:%d '' rsuTimeSourceLost rsuAlertLevel i %d  rsuTimeSourceLostMsg.0 s \"%s\"",
              #else
              ret = snprintf(cmd,sizeof(cmd),"snmptrap -v 3 -u rsutweiadmin -l authPriv -a SHA-512 -A rsutweiauthpass -x AES-256 -X rsutweiprivpass -mNTCIP1218-v01 udp:%s:%d '' rsuTimeSourceLost rsuAlertLevel i %d  rsuTimeSourceLostMsg.0 s \"%s\"",
              #endif
                  ntcip_1218_mib.rsuNotifyIpAddress,ntcip_1218_mib.rsuNotifyPort
                  ,ntcip_1218_mib.Async.rsuTimeSourceLost,ntcip_1218_mib.Async.rsuTimeSourceLostMsg);

#if defined(EXTRA_DEBUG)
                  I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"rsuTimeSourceLost: cmd(%s)\n",cmd);
#endif
//TODO: In future this is where we wait for ack on UDP port
              if(0 < ret) {
                  if(system(cmd)) {
                      ret = RSEMIB_SYSCALL_FAIL;
                      I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"RSEMIB_SYSCALL_FAIL: rsuTimeSourceLost: system: errno(%s).\n",strerror(errno));

                      ntcip_1218_mib.Async.rsuTimeSourceLost   = rsuAlertLevel_denso_empty;
                      ntcip_1218_mib.Async.rsuTimeSourceLostAttempts = 0;
                      ntcip_1218_mib.Async.rsuTimeSourceLostInterval = 0;
                  } else { 
                      /* Keep sending till we hit max retries. No ack */
                      ntcip_1218_mib.Async.rsuTimeSourceLostAttempts++;
                      ntcip_1218_mib.Async.rsuTimeSourceLostInterval = 0;
                      I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"rsuTimeSourceLost(%s) level(%d)\n",
                          ntcip_1218_mib.Async.rsuTimeSourceLostMsg
                          ,ntcip_1218_mib.Async.rsuTimeSourceLost);
                  }
              } else {
                  ret = RSEMIB_SYSCALL_FAIL;
                  I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"RSEMIB_SYSCALL_FAIL: rsuTimeSourceLost: snprintf: errno(%s).\n",strerror(errno));

                  ntcip_1218_mib.Async.rsuTimeSourceLost   = rsuAlertLevel_denso_empty;
                  ntcip_1218_mib.Async.rsuTimeSourceLostAttempts = 0;
                  ntcip_1218_mib.Async.rsuTimeSourceLostInterval = 0;
              }
          } else {
              /* Not the first attempt and not at interval yet. */
              ntcip_1218_mib.Async.rsuTimeSourceLostInterval++;
          }
      } else {
          /* On hitting maxretries, stop it. */
          ntcip_1218_mib.Async.rsuTimeSourceLost   = rsuAlertLevel_denso_empty;
          ntcip_1218_mib.Async.rsuTimeSourceLostAttempts = 0;
          ntcip_1218_mib.Async.rsuTimeSourceLostInterval = 0;
      }
  }
  /* 5.18.1.7 Time Source Mismatch Message: deviation between two time sources exceeds vendor-defined threshold.*/
  if(   (rsuAlertLevel_info <= ntcip_1218_mib.Async.rsuTimeSourceMismatch) 
     && (ntcip_1218_mib.Async.rsuTimeSourceMismatch <= rsuAlertLevel_critical)){
#if defined(ENABLE_ASYNC_BACKUP)
      /* No matter what happens there will be some kind of update from here. */
      update = 1;
#endif
      /* Until we hit maxretries keep trying. No ack */
      if(ntcip_1218_mib.Async.rsuTimeSourceMismatchAttempts <= ntcip_1218_mib.Async.rsuNotificationMaxRetries) {

          /* If first attempt ignore interval and send. After that wait till intervals equal each other. */
          if(   (ntcip_1218_mib.Async.rsuTimeSourceMismatchInterval == ntcip_1218_mib.Async.rsuNotificationRepeatInterval)
             || (0 == ntcip_1218_mib.Async.rsuTimeSourceMismatchAttempts)){

              memset(cmd,0x0, sizeof(cmd));
//TODO: weak creds for test with SNMPB only. Do not ship like this.

              #if defined(ENABLE_SMNP_V3_SHA)
              ret = snprintf(cmd,sizeof(cmd),"snmptrap -v 3 -u rsutweiadmin -l authPriv -a SHA -A rsutweiauthpass -x AES-256 -X rsutweiprivpass -mNTCIP1218-v01 udp:%s:%d '' rsuTimeSourceMismatch rsuAlertLevel i %d  rsuTimeSourceMismatchMsg.0 s \"%s\"",
              #else
              ret = snprintf(cmd,sizeof(cmd),"snmptrap -v 3 -u rsutweiadmin -l authPriv -a SHA-512 -A rsutweiauthpass -x AES-256 -X rsutweiprivpass -mNTCIP1218-v01 udp:%s:%d '' rsuTimeSourceMismatch rsuAlertLevel i %d  rsuTimeSourceMismatchMsg.0 s \"%s\"",
              #endif
                  ntcip_1218_mib.rsuNotifyIpAddress,ntcip_1218_mib.rsuNotifyPort
                  ,ntcip_1218_mib.Async.rsuTimeSourceMismatch,ntcip_1218_mib.Async.rsuTimeSourceMismatchMsg);

#if defined(EXTRA_DEBUG)
                  I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"rsuTimeSourceMismatch: cmd(%s)\n",cmd);
#endif
//TODO: In future this is where we wait for ack on UDP port
              if(0 < ret) {
                  if(system(cmd)) {
                      ret = RSEMIB_SYSCALL_FAIL;
                      I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"RSEMIB_SYSCALL_FAIL: rsuTimeSourceMismatch: system: errno(%s).\n",strerror(errno));

                      ntcip_1218_mib.Async.rsuTimeSourceMismatch   = rsuAlertLevel_denso_empty;
                      ntcip_1218_mib.Async.rsuTimeSourceMismatchAttempts = 0;
                      ntcip_1218_mib.Async.rsuTimeSourceMismatchInterval = 0;
                  } else { 
                      /* Keep sending till we hit max retries. No ack */
                      ntcip_1218_mib.Async.rsuTimeSourceMismatchAttempts++;
                      ntcip_1218_mib.Async.rsuTimeSourceMismatchInterval = 0;
                      I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"rsuTimeSourceMismatch(%s) level(%d)\n",
                          ntcip_1218_mib.Async.rsuTimeSourceMismatchMsg
                          ,ntcip_1218_mib.Async.rsuTimeSourceMismatch);
                  }
              } else {
                  ret = RSEMIB_SYSCALL_FAIL;
                  I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"RSEMIB_SYSCALL_FAIL: rsuTimeSourceMismatch: snprintf: errno(%s).\n",strerror(errno));

                  ntcip_1218_mib.Async.rsuTimeSourceMismatch   = rsuAlertLevel_denso_empty;
                  ntcip_1218_mib.Async.rsuTimeSourceMismatchAttempts = 0;
                  ntcip_1218_mib.Async.rsuTimeSourceMismatchInterval = 0;
              }
          } else {
              /* Not the first attempt and not at interval yet. */
              ntcip_1218_mib.Async.rsuTimeSourceMismatchInterval++;
          }
      } else {
          /* On hitting maxretries, stop it. */
          ntcip_1218_mib.Async.rsuTimeSourceMismatch = rsuAlertLevel_denso_empty;
          ntcip_1218_mib.Async.rsuTimeSourceMismatchAttempts = 0;
          ntcip_1218_mib.Async.rsuTimeSourceMismatchInterval = 0;
      }
  }
  /* 5.18.1.8 GNSS Anomaly Message */
  if(   (rsuAlertLevel_info <= ntcip_1218_mib.Async.rsuGnssAnomaly) 
     && (ntcip_1218_mib.Async.rsuGnssAnomaly <= rsuAlertLevel_critical)){
#if defined(ENABLE_ASYNC_BACKUP)
      /* No matter what happens there will be some kind of update from here. */
      update = 1;
#endif
      /* Until we hit maxretries keep trying. No ack */
      if(ntcip_1218_mib.Async.rsuGnssAnomalyAttempts <= ntcip_1218_mib.Async.rsuNotificationMaxRetries) {

          /* If first attempt ignore interval and send. After that wait till intervals equal each other. */
          if(   (ntcip_1218_mib.Async.rsuGnssAnomalyInterval == ntcip_1218_mib.Async.rsuNotificationRepeatInterval)
             || (0 == ntcip_1218_mib.Async.rsuGnssAnomalyAttempts)){

              memset(cmd,0x0, sizeof(cmd));
//TODO: weak creds for test with SNMPB only. Do not ship like this.

              #if defined(ENABLE_SMNP_V3_SHA)
              ret = snprintf(cmd,sizeof(cmd),"snmptrap -v 3 -u rsutweiadmin -l authPriv -a SHA -A rsutweiauthpass -x AES-256 -X rsutweiprivpass -mNTCIP1218-v01 udp:%s:%d '' rsuGnssAnomaly rsuAlertLevel i %d  rsuGnssAnomalyMsg.0 s \"%s\"",
              #else
              ret = snprintf(cmd,sizeof(cmd),"snmptrap -v 3 -u rsutweiadmin -l authPriv -a SHA-512 -A rsutweiauthpass -x AES-256 -X rsutweiprivpass -mNTCIP1218-v01 udp:%s:%d '' rsuGnssAnomaly rsuAlertLevel i %d  rsuGnssAnomalyMsg.0 s \"%s\"",
              #endif
                  ntcip_1218_mib.rsuNotifyIpAddress,ntcip_1218_mib.rsuNotifyPort
                  ,ntcip_1218_mib.Async.rsuGnssAnomaly,ntcip_1218_mib.Async.rsuGnssAnomalyMsg);

#if defined(EXTRA_DEBUG)
                  I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"rsuGnssAnomaly: cmd(%s)\n",cmd);
#endif
//TODO: In future this is where we wait for ack on UDP port
              if(0 < ret) {
                  if(system(cmd)) {
                      ret = RSEMIB_SYSCALL_FAIL;
                      I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"RSEMIB_SYSCALL_FAIL: rsuGnssAnomaly: cmd(%s): errno(%s).\n",cmd,strerror(errno));

                      ntcip_1218_mib.Async.rsuGnssAnomaly   = rsuAlertLevel_denso_empty;
                      ntcip_1218_mib.Async.rsuGnssAnomalyAttempts = 0;
                      ntcip_1218_mib.Async.rsuGnssAnomalyInterval = 0;
                  } else { 
                      /* Keep sending till we hit max retries. No ack */
                      ntcip_1218_mib.Async.rsuGnssAnomalyAttempts++;
                      ntcip_1218_mib.Async.rsuGnssAnomalyInterval = 0;
                      I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"rsuGnssAnomaly(%s) level(%d) attempts(%d) interval(%d) mxretries(%d)\n",
                          ntcip_1218_mib.Async.rsuGnssAnomalyMsg
                          ,ntcip_1218_mib.Async.rsuGnssAnomaly
                          ,ntcip_1218_mib.Async.rsuGnssAnomalyAttempts
                          ,ntcip_1218_mib.Async.rsuGnssAnomalyInterval
                          ,ntcip_1218_mib.Async.rsuNotificationMaxRetries);
                  }
              } else {
                  ret = RSEMIB_SYSCALL_FAIL;
                  I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"RSEMIB_SYSCALL_FAIL: rsuGnssAnomaly: snprintf: errno(%s).\n",strerror(errno));

                  ntcip_1218_mib.Async.rsuGnssAnomaly   = rsuAlertLevel_denso_empty;
                  ntcip_1218_mib.Async.rsuGnssAnomalyAttempts = 0;
                  ntcip_1218_mib.Async.rsuGnssAnomalyInterval = 0;
              }
          } else {
              /* Not the first attempt and not at interval yet. */
              ntcip_1218_mib.Async.rsuGnssAnomalyInterval++;
          }
      } else {
          /* On hitting maxretries, stop it. */
          ntcip_1218_mib.Async.rsuGnssAnomaly = rsuAlertLevel_denso_empty;
          ntcip_1218_mib.Async.rsuGnssAnomalyAttempts = 0;
          ntcip_1218_mib.Async.rsuGnssAnomalyInterval = 0;
      }
  }
  /* 5.18.1.9 GNSS Deviation Error Message: GNSS position deviation greater than allowed. */
  if(   (rsuAlertLevel_info <= ntcip_1218_mib.Async.rsuGnssDeviationError) 
     && (ntcip_1218_mib.Async.rsuGnssDeviationError <= rsuAlertLevel_critical)){
#if defined(ENABLE_ASYNC_BACKUP)
      /* No matter what happens there will be some kind of update from here. */
      update = 1;
#endif
      /* Until we hit maxretries keep trying. No ack */
      if(ntcip_1218_mib.Async.rsuGnssDeviationAttempts <= ntcip_1218_mib.Async.rsuNotificationMaxRetries) {

          /* If first attempt ignore interval and send. After that wait till intervals equal each other. */
          if(   (ntcip_1218_mib.Async.rsuGnssDeviationInterval == ntcip_1218_mib.Async.rsuNotificationRepeatInterval)
             || (0 == ntcip_1218_mib.Async.rsuGnssDeviationAttempts)){

              memset(cmd,0x0, sizeof(cmd));
//TODO: weak creds for test with SNMPB only. Do not ship like this.

              #if defined(ENABLE_SMNP_V3_SHA)
              ret = snprintf(cmd,sizeof(cmd),"snmptrap -v 3 -u rsutweiadmin -l authPriv -a SHA -A rsutweiauthpass -x AES-256 -X rsutweiprivpass -mNTCIP1218-v01 udp:%s:%d '' rsuGnssDeviationError.0 rsuAlertLevel i %d  rsuGnssDeviationMsg.0 s \"%s\"",
              #else
              ret = snprintf(cmd,sizeof(cmd),"snmptrap -v 3 -u rsutweiadmin -l authPriv -a SHA-512 -A rsutweiauthpass -x AES-256 -X rsutweiprivpass -mNTCIP1218-v01 udp:%s:%d '' rsuGnssDeviationError.0 rsuAlertLevel i %d  rsuGnssDeviationMsg.0 s \"%s\"",
              #endif
                  ntcip_1218_mib.rsuNotifyIpAddress,ntcip_1218_mib.rsuNotifyPort
                  ,ntcip_1218_mib.Async.rsuGnssDeviationError,ntcip_1218_mib.Async.rsuGnssDeviationMsg);

#if defined(EXTRA_DEBUG)
                  I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"rsuGnssDeviation: cmd(%s)\n",cmd);
#endif
//TODO: In future this is where we wait for ack on UDP port
              if(0 < ret) {
                  if(system(cmd)) {
                      ret = RSEMIB_SYSCALL_FAIL;
                      I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"RSEMIB_SYSCALL_FAIL: rsuGnssDeviation: system: errno(%s).\n",strerror(errno));

                      ntcip_1218_mib.Async.rsuGnssDeviationError   = rsuAlertLevel_denso_empty;
                      ntcip_1218_mib.Async.rsuGnssDeviationAttempts = 0;
                      ntcip_1218_mib.Async.rsuGnssDeviationInterval = 0;
                  } else { 
                      /* Keep sending till we hit max retries. No ack */
                      ntcip_1218_mib.Async.rsuGnssDeviationAttempts++;
                      ntcip_1218_mib.Async.rsuGnssDeviationInterval = 0;
                      I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"rsuGnssDeviation(%s) level(%d)\n",
                          ntcip_1218_mib.Async.rsuGnssDeviationMsg
                          ,ntcip_1218_mib.Async.rsuGnssDeviationError);
                  }
              } else {
                  ret = RSEMIB_SYSCALL_FAIL;
                  I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"RSEMIB_SYSCALL_FAIL: rsuGnssDeviation: snprintf: errno(%s).\n",strerror(errno));

                  ntcip_1218_mib.Async.rsuGnssDeviationError   = rsuAlertLevel_denso_empty;
                  ntcip_1218_mib.Async.rsuGnssDeviationAttempts = 0;
                  ntcip_1218_mib.Async.rsuGnssDeviationInterval = 0;
              }
          } else {
              /* Not the first attempt and not at interval yet. */
              ntcip_1218_mib.Async.rsuGnssDeviationInterval++;
          }
      } else {
          /* On hitting maxretries, stop it. */
          ntcip_1218_mib.Async.rsuGnssDeviationError = rsuAlertLevel_denso_empty;
          ntcip_1218_mib.Async.rsuGnssDeviationAttempts = 0;
          ntcip_1218_mib.Async.rsuGnssDeviationInterval = 0;
      }
  }

  /* NOTE: Done by rsuGnssOutput in MIB, not by RSUDIAG */
  /* 5.18.1.10 */

  /* 5.18.1.11 Certificate Error Message */
  if(   (rsuAlertLevel_info <= ntcip_1218_mib.Async.rsuCertificateError) 
     && (ntcip_1218_mib.Async.rsuCertificateError <= rsuAlertLevel_critical)){
#if defined(ENABLE_ASYNC_BACKUP)
      /* No matter what happens there will be some kind of update from here. */
      update = 1;
#endif
      /* Until we hit maxretries keep trying. No ack */
      if(ntcip_1218_mib.Async.rsuCertificateAttempts <= ntcip_1218_mib.Async.rsuNotificationMaxRetries) {

          /* If first attempt ignore interval and send. After that wait till intervals equal each other. */
          if(   (ntcip_1218_mib.Async.rsuCertificateInterval == ntcip_1218_mib.Async.rsuNotificationRepeatInterval)
             || (0 == ntcip_1218_mib.Async.rsuCertificateAttempts)){

              memset(cmd,0x0, sizeof(cmd));
//TODO: weak creds for test with SNMPB only. Do not ship like this.

              #if defined(ENABLE_SMNP_V3_SHA)
              ret = snprintf(cmd,sizeof(cmd),"snmptrap -v 3 -u rsutweiadmin -l authPriv -a SHA -A rsutweiauthpass -x AES-256 -X rsutweiprivpass -mNTCIP1218-v01 udp:%s:%d '' rsuCertificateError.0 rsuAlertLevel i %d  rsuCertificateMsg.0 s \"%s\"",
              #else
              ret = snprintf(cmd,sizeof(cmd),"snmptrap -v 3 -u rsutweiadmin -l authPriv -a SHA-512 -A rsutweiauthpass -x AES-256 -X rsutweiprivpass -mNTCIP1218-v01 udp:%s:%d '' rsuCertificateError.0 rsuAlertLevel i %d  rsuCertificateMsg.0 s \"%s\"",
              #endif
                  ntcip_1218_mib.rsuNotifyIpAddress,ntcip_1218_mib.rsuNotifyPort
                  ,ntcip_1218_mib.Async.rsuCertificateError,ntcip_1218_mib.Async.rsuCertificateMsg);

#if defined(EXTRA_DEBUG)
                  I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"rsuCertificate: attempt(%u) interval(%u) cmd(%s)\n",ntcip_1218_mib.Async.rsuCertificateAttempts, ntcip_1218_mib.Async.rsuCertificateInterval,cmd);
#endif
//TODO: In future this is where we wait for ack on UDP port
              if(0 < ret) {
                  if(system(cmd)) {
                      ret = RSEMIB_SYSCALL_FAIL;
                      I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"RSEMIB_SYSCALL_FAIL: rsuCertificate: system: errno(%s).\n",strerror(errno));

                      ntcip_1218_mib.Async.rsuCertificateError   = rsuAlertLevel_denso_empty;
                      ntcip_1218_mib.Async.rsuCertificateAttempts = 0;
                      ntcip_1218_mib.Async.rsuCertificateInterval = 0;
                  } else { 
                      /* Keep sending till we hit max retries. No ack */
                      ntcip_1218_mib.Async.rsuCertificateAttempts++;
                      ntcip_1218_mib.Async.rsuCertificateInterval = 0;
                      I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"rsuCertificate(%s) level(%d)\n",
                          ntcip_1218_mib.Async.rsuCertificateMsg
                          ,ntcip_1218_mib.Async.rsuCertificateError);
                  }
              } else {
                  ret = RSEMIB_SYSCALL_FAIL;
                  I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"RSEMIB_SYSCALL_FAIL: rsuCertificate: snprintf: errno(%s).\n",strerror(errno));

                  ntcip_1218_mib.Async.rsuCertificateError   = rsuAlertLevel_denso_empty;
                  ntcip_1218_mib.Async.rsuCertificateAttempts = 0;
                  ntcip_1218_mib.Async.rsuCertificateInterval = 0;
              }
          } else {
              /* Not the first attempt and not at interval yet. */
              ntcip_1218_mib.Async.rsuCertificateInterval++;
          }
      } else {
          /* On hitting maxretries, stop it. */
          ntcip_1218_mib.Async.rsuCertificateError = rsuAlertLevel_denso_empty;
          ntcip_1218_mib.Async.rsuCertificateAttempts = 0;
          ntcip_1218_mib.Async.rsuCertificateInterval = 0;
      }
  }
  /* 5.18.1.12 Denial of Service Error Message */
  if(   (rsuAlertLevel_info <= ntcip_1218_mib.Async.rsuServiceDenialError) 
     && (ntcip_1218_mib.Async.rsuServiceDenialError <= rsuAlertLevel_critical)){
#if defined(ENABLE_ASYNC_BACKUP)
      /* No matter what happens there will be some kind of update from here. */
      update = 1;
#endif
      /* Until we hit maxretries keep trying. No ack */
      if(ntcip_1218_mib.Async.rsuServiceDenialAttempts <= ntcip_1218_mib.Async.rsuNotificationMaxRetries) {

          /* If first attempt ignore interval and send. After that wait till intervals equal each other. */
          if(   (ntcip_1218_mib.Async.rsuServiceDenialInterval == ntcip_1218_mib.Async.rsuNotificationRepeatInterval)
             || (0 == ntcip_1218_mib.Async.rsuServiceDenialAttempts)){

              memset(cmd,0x0, sizeof(cmd));
//TODO: weak creds for test with SNMPB only. Do not ship like this.

              #if defined(ENABLE_SMNP_V3_SHA)
              ret = snprintf(cmd,sizeof(cmd),"snmptrap -v 3 -u rsutweiadmin -l authPriv -a SHA -A rsutweiauthpass -x AES-256 -X rsutweiprivpass -mNTCIP1218-v01 udp:%s:%d '' rsuServiceDenialError.0 rsuAlertLevel i %d  rsuServiceDenialMsg.0 s \"%s\"",
              #else
              ret = snprintf(cmd,sizeof(cmd),"snmptrap -v 3 -u rsutweiadmin -l authPriv -a SHA-512 -A rsutweiauthpass -x AES-256 -X rsutweiprivpass -mNTCIP1218-v01 udp:%s:%d '' rsuServiceDenialError.0 rsuAlertLevel i %d  rsuServiceDenialMsg.0 s \"%s\"",
              #endif
                  ntcip_1218_mib.rsuNotifyIpAddress,ntcip_1218_mib.rsuNotifyPort
                  ,ntcip_1218_mib.Async.rsuServiceDenialError,ntcip_1218_mib.Async.rsuServiceDenialMsg);

#if defined(EXTRA_DEBUG)
                  I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"rsuServiceDenial: cmd(%s)\n",cmd);
#endif
//TODO: In future this is where we wait for ack on UDP port
              if(0 < ret) {
                  if(system(cmd)) {
                      ret = RSEMIB_SYSCALL_FAIL;
                      I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"RSEMIB_SYSCALL_FAIL: rsuServiceDenial: system: errno(%s).\n",strerror(errno));

                      ntcip_1218_mib.Async.rsuServiceDenialError   = rsuAlertLevel_denso_empty;
                      ntcip_1218_mib.Async.rsuServiceDenialAttempts = 0;
                      ntcip_1218_mib.Async.rsuServiceDenialInterval = 0;
                  } else { 
                      /* Keep sending till we hit max retries. No ack */
                      ntcip_1218_mib.Async.rsuServiceDenialAttempts++;
                      ntcip_1218_mib.Async.rsuServiceDenialInterval = 0;
                      I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"rsuServiceDenial(%s) level(%d)\n",
                          ntcip_1218_mib.Async.rsuServiceDenialMsg
                          ,ntcip_1218_mib.Async.rsuServiceDenialError);
                  }
              } else {
                  ret = RSEMIB_SYSCALL_FAIL;
                  I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"RSEMIB_SYSCALL_FAIL: rsuServiceDenial: snprintf: errno(%s).\n",strerror(errno));

                  ntcip_1218_mib.Async.rsuServiceDenialError   = rsuAlertLevel_denso_empty;
                  ntcip_1218_mib.Async.rsuServiceDenialAttempts = 0;
                  ntcip_1218_mib.Async.rsuServiceDenialInterval = 0;
              }
          } else {
              /* Not the first attempt and not at interval yet. */
              ntcip_1218_mib.Async.rsuServiceDenialInterval++;
          }
      } else {
          /* On hitting maxretries, stop it. */
          ntcip_1218_mib.Async.rsuServiceDenialError = rsuAlertLevel_denso_empty;
          ntcip_1218_mib.Async.rsuServiceDenialAttempts = 0;
          ntcip_1218_mib.Async.rsuServiceDenialInterval = 0;
      }
  }
  /* 5.18.1.13 Watchdog Error Message */
  if(   (rsuAlertLevel_info <= ntcip_1218_mib.Async.rsuWatchdogError) 
     && (ntcip_1218_mib.Async.rsuWatchdogError <= rsuAlertLevel_critical)){
#if defined(ENABLE_ASYNC_BACKUP)
      /* No matter what happens there will be some kind of update from here. */
      update = 1;
#endif
      /* Until we hit maxretries keep trying. No ack */
      if(ntcip_1218_mib.Async.rsuWatchdogAttempts <= ntcip_1218_mib.Async.rsuNotificationMaxRetries) {

          /* If first attempt ignore interval and send. After that wait till intervals equal each other. */
          if(   (ntcip_1218_mib.Async.rsuWatchdogInterval == ntcip_1218_mib.Async.rsuNotificationRepeatInterval)
             || (0 == ntcip_1218_mib.Async.rsuWatchdogAttempts)){

              memset(cmd,0x0, sizeof(cmd));
//TODO: weak creds for test with SNMPB only. Do not ship like this.

              #if defined(ENABLE_SMNP_V3_SHA)
              ret = snprintf(cmd,sizeof(cmd),"snmptrap -v 3 -u rsutweiadmin -l authPriv -a SHA -A rsutweiauthpass -x AES-256 -X rsutweiprivpass -mNTCIP1218-v01 udp:%s:%d '' rsuWatchdogError.0 rsuAlertLevel i %d  rsuWatchdogMsg.0 s \"%s\"",
              #else
              ret = snprintf(cmd,sizeof(cmd),"snmptrap -v 3 -u rsutweiadmin -l authPriv -a SHA-512 -A rsutweiauthpass -x AES-256 -X rsutweiprivpass -mNTCIP1218-v01 udp:%s:%d '' rsuWatchdogError.0 rsuAlertLevel i %d  rsuWatchdogMsg.0 s \"%s\"",
              #endif
                  ntcip_1218_mib.rsuNotifyIpAddress,ntcip_1218_mib.rsuNotifyPort
                  ,ntcip_1218_mib.Async.rsuWatchdogError,ntcip_1218_mib.Async.rsuWatchdogMsg);

#if defined(EXTRA_DEBUG)
                  I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"rsuWatchdog: cmd(%s)\n",cmd);
#endif
//TODO: In future this is where we wait for ack on UDP port
              if(0 < ret) {
                  if(system(cmd)) {
                      ret = RSEMIB_SYSCALL_FAIL;
                      I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"RSEMIB_SYSCALL_FAIL: rsuWatchdog: system: errno(%s).\n",strerror(errno));
                      ntcip_1218_mib.Async.rsuWatchdogError   = rsuAlertLevel_denso_empty;
                      ntcip_1218_mib.Async.rsuWatchdogAttempts = 0;
                      ntcip_1218_mib.Async.rsuWatchdogInterval = 0;
                  } else { 
                      /* Keep sending till we hit max retries. No ack */
                      ntcip_1218_mib.Async.rsuWatchdogAttempts++;
                      ntcip_1218_mib.Async.rsuWatchdogInterval = 0;
                      I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"rsuWatchdog(%s) level(%d)\n",
                          ntcip_1218_mib.Async.rsuWatchdogMsg
                          ,ntcip_1218_mib.Async.rsuWatchdogError);
                  }
              } else {
                  ret = RSEMIB_SYSCALL_FAIL;
                  I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"RSEMIB_SYSCALL_FAIL: rsuWatchdog: snprintf: errno(%s).\n",strerror(errno));

                  ntcip_1218_mib.Async.rsuWatchdogError   = rsuAlertLevel_denso_empty;
                  ntcip_1218_mib.Async.rsuWatchdogAttempts = 0;
                  ntcip_1218_mib.Async.rsuWatchdogInterval = 0;
              }
          } else {
              /* Not the first attempt and not at interval yet. */
              ntcip_1218_mib.Async.rsuWatchdogInterval++;
          }
      } else {
          /* On hitting maxretries, stop it. */
          ntcip_1218_mib.Async.rsuWatchdogError = rsuAlertLevel_denso_empty;
          ntcip_1218_mib.Async.rsuWatchdogAttempts = 0;
          ntcip_1218_mib.Async.rsuWatchdogInterval = 0;
      }
  }
  /* 5.18.1.14 Enclosure Environment Message: tamper detection */
  if(   (rsuAlertLevel_info <= ntcip_1218_mib.Async.rsuEnvironError) 
     && (ntcip_1218_mib.Async.rsuEnvironError <= rsuAlertLevel_critical)){
#if defined(ENABLE_ASYNC_BACKUP)
      /* No matter what happens there will be some kind of update from here. */
      update = 1;
#endif
      /* Until we hit maxretries keep trying. No ack */
      if(ntcip_1218_mib.Async.rsuEnvironAttempts <= ntcip_1218_mib.Async.rsuNotificationMaxRetries) {

          /* If first attempt ignore interval and send. After that wait till intervals equal each other. */
          if(   (ntcip_1218_mib.Async.rsuEnvironInterval == ntcip_1218_mib.Async.rsuNotificationRepeatInterval)
             || (0 == ntcip_1218_mib.Async.rsuEnvironAttempts)){

              memset(cmd,0x0, sizeof(cmd));
//TODO: weak creds for test with SNMPB only. Do not ship like this.

              #if defined(ENABLE_SMNP_V3_SHA)
              ret = snprintf(cmd,sizeof(cmd),"snmptrap -v 3 -u rsutweiadmin -l authPriv -a SHA -A rsutweiauthpass -x AES-256 -X rsutweiprivpass -mNTCIP1218-v01 udp:%s:%d '' rsuEnvironError.0 rsuAlertLevel i %d  rsuEnvironMsg.0 s \"%s\"",
              #else
              ret = snprintf(cmd,sizeof(cmd),"snmptrap -v 3 -u rsutweiadmin -l authPriv -a SHA-512 -A rsutweiauthpass -x AES-256 -X rsutweiprivpass -mNTCIP1218-v01 udp:%s:%d '' rsuEnvironError.0 rsuAlertLevel i %d  rsuEnvironMsg.0 s \"%s\"",
              #endif
                  ntcip_1218_mib.rsuNotifyIpAddress,ntcip_1218_mib.rsuNotifyPort
                  ,ntcip_1218_mib.Async.rsuEnvironError,ntcip_1218_mib.Async.rsuEnvironMsg);

#if defined(EXTRA_DEBUG)
                  I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"rsuEnvironMsg: cmd(%s)\n",cmd);
#endif
//TODO: In future this is where we wait for ack on UDP port
              if(0 < ret) {
                  if(system(cmd)) {
                      ret = RSEMIB_SYSCALL_FAIL;
                      I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"RSEMIB_SYSCALL_FAIL: rsuEnviron: system: errno(%s).\n",strerror(errno));
                      ntcip_1218_mib.Async.rsuEnvironError   = rsuAlertLevel_denso_empty;
                      ntcip_1218_mib.Async.rsuEnvironAttempts = 0;
                      ntcip_1218_mib.Async.rsuEnvironInterval = 0;
                      I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"rsuEnviron(%s) level(%d)\n",
                          ntcip_1218_mib.Async.rsuEnvironMsg
                          ,ntcip_1218_mib.Async.rsuEnvironError);
                  } else { 
                      /* Keep sending till we hit max retries. No ack */
                      ntcip_1218_mib.Async.rsuEnvironAttempts++;
                      ntcip_1218_mib.Async.rsuEnvironInterval = 0;
                      I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"rsuEnviron(%s) level(%d)\n",
                          ntcip_1218_mib.Async.rsuEnvironMsg
                          ,ntcip_1218_mib.Async.rsuEnvironError);
                  }
              } else {
                  ret = RSEMIB_SYSCALL_FAIL;
                  I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"RSEMIB_SYSCALL_FAIL: rsuEnviron: snprintf: errno(%s).\n",strerror(errno));

                  ntcip_1218_mib.Async.rsuEnvironError   = rsuAlertLevel_denso_empty;
                  ntcip_1218_mib.Async.rsuEnvironAttempts = 0;
                  ntcip_1218_mib.Async.rsuEnvironInterval = 0;
              }
          } else {
              /* Not the first attempt and not at interval yet. */
              ntcip_1218_mib.Async.rsuEnvironInterval++;
          }
      } else {
          /* On hitting maxretries, stop it. */
          ntcip_1218_mib.Async.rsuEnvironError = rsuAlertLevel_denso_empty;
          ntcip_1218_mib.Async.rsuEnvironAttempts = 0;
          ntcip_1218_mib.Async.rsuEnvironInterval = 0;
      }
  }
      if(ret < RSEMIB_OK) {
          set_ntcip_1218_error_states(ret);
      }
      /* Sending a lone critical trap doesn't mean we are in standby or fault yet.
       * It has to persist long enough to become a problem. RSUDIAG tracks that.
       */
      if(errorstates & RSUHEALTH_FAULT_MODE) {/* Fault higher than Standby.*/
          ntcip_1218_mib.rsuModeStatus = rsuModeStatus_fault;
          ntcip_1218_mib.rsuStatus = rsuStatus_critical;
          ntcip_1218_mib.rsuMode = rsuMode_standby;
      } else {
          if(errorstates & RSUHEALTH_STANDBY_MODE){ /* Standby higher than Operate. */
              ntcip_1218_mib.rsuModeStatus = rsuModeStatus_standby;
              ntcip_1218_mib.rsuStatus = rsuStatus_warning;
              ntcip_1218_mib.rsuMode = rsuMode_standby;
          } else { /* If not Fault and not Standby then Operate.*/
              ntcip_1218_mib.rsuModeStatus = rsuModeStatus_operate;
              ntcip_1218_mib.rsuStatus = rsuStatus_okay;
              ntcip_1218_mib.rsuMode = rsuMode_operate;
          }
      }
      ntcip1218_unlock();
  } else {
      ret = RSEMIB_LOCK_FAIL;
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"send_notifications: RSEMIB_LOCK_FAIL.\n");
      set_ntcip_1218_error_states(ret);
  }
}
/* 5.18.1.1 */
void set_messageFileIntegrity(rsuAlertLevel_e alert, uint8_t * alertMsg, uint8_t length) /* Process alert. */
{
  if((rsuAlertLevel_info <= alert) && (alert <= rsuAlertLevel_critical)){
      if(RSEMIB_OK == ntcip1218_lock()){
          ntcip_1218_mib.Async.messageFileIntegrityError = alert;
          memset(ntcip_1218_mib.Async.rsuMsgFileIntegrityMsg,0x0,sizeof(ntcip_1218_mib.Async.rsuMsgFileIntegrityMsg));
          memcpy(ntcip_1218_mib.Async.rsuMsgFileIntegrityMsg,alertMsg,length);
          ntcip_1218_mib.Async.rsuMsgFileIntegrity_length = length;
          ntcip_1218_mib.Async.rsuMsgFileIntegrityAttempts = 0;
          ntcip1218_unlock();
#if defined(EXTRA_DEBUG)
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"set_messageFileIntegrity: SUCCESS:(%d)(%s)\n",alert,alertMsg);
#endif
      } else {
          set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"set_messageFileIntegrity: RSEMIB_LOCK_FAIL:(%d)(%s)\n",alert,alertMsg);
      }
  } else {
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"set_messageFileIntegrity: bogus interval:(%d)(%s)\n",alert,alertMsg);
  }
}
/* 5.18.1.2 */
void set_rsuSecStorageIntegrity(rsuAlertLevel_e alert, uint8_t * alertMsg, uint8_t length) /* Process alert. */
{
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.Async.rsuSecStorageIntegrityError = alert;
      memset(ntcip_1218_mib.Async.rsuSecStorageIntegrityMsg,0x0,sizeof(ntcip_1218_mib.Async.rsuSecStorageIntegrityMsg));
      memcpy(ntcip_1218_mib.Async.rsuSecStorageIntegrityMsg,alertMsg,length);
      ntcip_1218_mib.Async.rsuSecStorageIntegrity_length = length;
      ntcip_1218_mib.Async.rsuSecStorageIntegrityAttempts = 0;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"send_rsuSecStorageIntegrity: RSEMIB_LOCK_FAIL.\n");
  }
}
/* 5.18.1.3 */
void set_rsuAuth(rsuAlertLevel_e alert, uint8_t * alertMsg, uint8_t length) /* Process alert. */
{
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.Async.rsuAuthError = alert;
      memset(ntcip_1218_mib.Async.rsuAuthMsg,0x0,sizeof(ntcip_1218_mib.Async.rsuAuthMsg));
      memcpy(ntcip_1218_mib.Async.rsuAuthMsg,alertMsg,length);
      ntcip_1218_mib.Async.rsuAuth_length = length;
      ntcip_1218_mib.Async.rsuAuthAttempts = 0;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"send_rsuAuth: RSEMIB_LOCK_FAIL.\n");
  }
}
/* 5.18.1.4 Signature Verification Error Message: Any failed signature on WSM's. */
void set_rsuSignatureVerify(rsuAlertLevel_e alert, uint8_t * alertMsg, uint8_t length) /* Process alert. */
{
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.Async.rsuSignatureVerifyError = alert;
      memset(ntcip_1218_mib.Async.rsuSignatureVerifyMsg,0x0,sizeof(ntcip_1218_mib.Async.rsuSignatureVerifyMsg));
      memcpy(ntcip_1218_mib.Async.rsuSignatureVerifyMsg,alertMsg,length);
      ntcip_1218_mib.Async.rsuSignatureVerify_length = length;
      ntcip_1218_mib.Async.rsuSignatureVerifyAttempts = 0;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"send_rsuSignatureVerify: RSEMIB_LOCK_FAIL.\n");
  }
}
/* 5.18.1.5 */
void set_rsuAccess(rsuAlertLevel_e alert, uint8_t * alertMsg, uint8_t length)
{
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.Async.rsuAccessError = alert;
      memset(ntcip_1218_mib.Async.rsuAccessMsg,0x0,sizeof(ntcip_1218_mib.Async.rsuAccessMsg));
      memcpy(ntcip_1218_mib.Async.rsuAccessMsg,alertMsg,length);
      ntcip_1218_mib.Async.rsuAccess_length = length;
      ntcip_1218_mib.Async.rsuAccessAttempts = 0;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"send_rsuAccess: RSEMIB_LOCK_FAIL.\n");
  }
}
/* 5.18.1.6 Time Source Lost Message. */
void set_rsuTimeSourceLost(rsuAlertLevel_e alert, uint8_t * alertMsg, uint8_t length)
{
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.Async.rsuTimeSourceLost = alert;
      memset(ntcip_1218_mib.Async.rsuTimeSourceLostMsg,0x0,sizeof(ntcip_1218_mib.Async.rsuTimeSourceLostMsg));
      memcpy(ntcip_1218_mib.Async.rsuTimeSourceLostMsg,alertMsg,length);
      ntcip_1218_mib.Async.rsuTimeSourceLost_length = length;
      ntcip_1218_mib.Async.rsuTimeSourceLostAttempts = 0;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"send_rsuTimeSourceLost: RSEMIB_LOCK_FAIL.\n");
  }
}
/* 5.18.1.7 */
void set_rsuTimeSourceMismatch(rsuAlertLevel_e alert, uint8_t * alertMsg, uint8_t length)
{
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.Async.rsuTimeSourceMismatch = alert;
      memset(ntcip_1218_mib.Async.rsuTimeSourceMismatchMsg,0x0,RSU_ALERT_MSG_LENGTH_MAX);
      memcpy(ntcip_1218_mib.Async.rsuTimeSourceMismatchMsg,alertMsg,length);
      ntcip_1218_mib.Async.rsuTimeSourceMismatch_length = length;
      ntcip_1218_mib.Async.rsuTimeSourceMismatchAttempts = 0;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"send_rsuTimeSourceMismatch: RSEMIB_LOCK_FAIL.\n");
  }
}
/* 5.18.1.8 */
void set_rsuGnssAnomaly(rsuAlertLevel_e alert, uint8_t * alertMsg, uint8_t length)
{
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.Async.rsuGnssAnomaly = alert;
      memset(ntcip_1218_mib.Async.rsuGnssAnomalyMsg,0x0,sizeof(ntcip_1218_mib.Async.rsuGnssAnomalyMsg));
      memcpy(ntcip_1218_mib.Async.rsuGnssAnomalyMsg,alertMsg,length);
      ntcip_1218_mib.Async.rsuGnssAnomaly_length = length;
      ntcip_1218_mib.Async.rsuGnssAnomalyAttempts = 0;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"send_rsuGnssAnomaly: RSEMIB_LOCK_FAIL.\n");
  }
}
/* 5.18.1.9 */
void set_rsuGnssDeviation(rsuAlertLevel_e alert, uint8_t * alertMsg, uint8_t length)
{
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.Async.rsuGnssDeviationError = alert;
      memset(ntcip_1218_mib.Async.rsuGnssDeviationMsg,0x0,sizeof(ntcip_1218_mib.Async.rsuGnssDeviationMsg));
      memcpy(ntcip_1218_mib.Async.rsuGnssDeviationMsg,alertMsg,length);
      ntcip_1218_mib.Async.rsuGnssDeviation_length = length;
      ntcip_1218_mib.Async.rsuGnssDeviationAttempts = 0;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"send_rsuGnssDeviation: RSEMIB_LOCK_FAIL.\n");
  }
}
/* 5.18.1.11 */
void set_rsuCertificate(rsuAlertLevel_e alert, uint8_t * alertMsg, uint8_t length)
{
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.Async.rsuCertificateError = alert;
      memset(ntcip_1218_mib.Async.rsuCertificateMsg,0x0,sizeof(ntcip_1218_mib.Async.rsuCertificateMsg));
      memcpy(ntcip_1218_mib.Async.rsuCertificateMsg,alertMsg,length);
      ntcip_1218_mib.Async.rsuCertificate_length = length;
      ntcip_1218_mib.Async.rsuCertificateAttempts = 0;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"send_rsuCertificate: RSEMIB_LOCK_FAIL.\n");
  }
}
/* 5.18.1.12 Denial of Service Error Message */
void set_rsuServiceDenial(rsuAlertLevel_e alert, uint8_t * alertMsg, uint8_t length)
{
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.Async.rsuServiceDenialError = alert;
      memset(ntcip_1218_mib.Async.rsuServiceDenialMsg,0x0,sizeof(ntcip_1218_mib.Async.rsuServiceDenialMsg));
      memcpy(ntcip_1218_mib.Async.rsuServiceDenialMsg,alertMsg,length);
      ntcip_1218_mib.Async.rsuServiceDenial_length = length;
      ntcip_1218_mib.Async.rsuServiceDenialAttempts = 0;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"send_rsuServiceDenial: RSEMIB_LOCK_FAIL.\n");
  }
}
/* 5.18.1.13 Watchdog Error Message */
void set_rsuWatchdog(rsuAlertLevel_e alert, uint8_t * alertMsg, uint8_t length)
{
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.Async.rsuWatchdogError = alert;
      memset(ntcip_1218_mib.Async.rsuWatchdogMsg,0x0,sizeof(ntcip_1218_mib.Async.rsuWatchdogMsg));
      memcpy(ntcip_1218_mib.Async.rsuWatchdogMsg,alertMsg,length);
      ntcip_1218_mib.Async.rsuWatchdog_length = length;
      ntcip_1218_mib.Async.rsuWatchdogAttempts = 0;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"send_rsuWatchdog: RSEMIB_LOCK_FAIL.\n");
  }
}
/* 5.18.1.14 Enclosure Environment Message: tamper detection */
void set_rsuEnviron(rsuAlertLevel_e alert, uint8_t * alertMsg, uint8_t length)
{
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.Async.rsuEnvironError = alert;
      memset(ntcip_1218_mib.Async.rsuEnvironMsg,0x0,sizeof(ntcip_1218_mib.Async.rsuEnvironMsg));
      memcpy(ntcip_1218_mib.Async.rsuEnvironMsg,alertMsg,length);
      ntcip_1218_mib.Async.rsuEnviron_length = length;
      ntcip_1218_mib.Async.rsuEnvironAttempts = 0;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_I2V_NAME,"send_rsuEnviron: RSEMIB_LOCK_FAIL.\n");
  }
}

/* 5.18.1.1 */
int32_t get_messageFileIntegrityError(void)
{
  rsuAlertLevel_e data = RSU_ALERT_DEFAULT;

  if(RSEMIB_OK == ntcip1218_lock()){
      data = ntcip_1218_mib.Async.messageFileIntegrityError;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
  if(rsuAlertLevel_denso_empty == data) {
      data = rsuAlertLevel_info;
  }
  if((rsuAlertLevel_info <= data) && (data <= rsuAlertLevel_critical)){
      return (int32_t)data; 
  } else {
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  }
}
/* 5.18.2.1 */
int32_t get_rsuMsgFileIntegrityMsg(uint8_t * data_out)
{
  int32_t size = 0;
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      size = ntcip_1218_mib.Async.rsuMsgFileIntegrity_length;
      memcpy(data_out, ntcip_1218_mib.Async.rsuMsgFileIntegrityMsg, RSU_ALERT_MSG_LENGTH_MAX);
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
  if((size < RSU_ALERT_MSG_LENGTH_MIN) || (RSU_ALERT_MSG_LENGTH_MAX < size)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);  
      return RSEMIB_BAD_DATA;
  }
  return size;
}

/* 5.18.1.2 */
int32_t get_rsuSecStorageIntegrityError(void)
{
  rsuAlertLevel_e data = RSU_ALERT_DEFAULT;
  if(RSEMIB_OK == ntcip1218_lock()){
      data = ntcip_1218_mib.Async.rsuSecStorageIntegrityError;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
  if(rsuAlertLevel_denso_empty == data) {
      data = rsuAlertLevel_info;
  }
  if((rsuAlertLevel_info <= data) && (data <= rsuAlertLevel_critical)){
      return (int32_t)data; 
  } else {
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  }
}
/* 5.18.2.2 */
int32_t get_rsuSecStorageIntegrityMsg(uint8_t * data_out)
{
  int32_t size = 0;
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      size = ntcip_1218_mib.Async.rsuSecStorageIntegrity_length;
      memcpy(data_out, ntcip_1218_mib.Async.rsuSecStorageIntegrityMsg, RSU_ALERT_MSG_LENGTH_MAX);
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
  if((size < RSU_ALERT_MSG_LENGTH_MIN) || (RSU_ALERT_MSG_LENGTH_MAX < size)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);  
      return RSEMIB_BAD_DATA;
  }
  return size;
}

/* 5.18.1.3 */
int32_t get_rsuAuthError(void)
{
  rsuAlertLevel_e data = RSU_ALERT_DEFAULT;
  if(RSEMIB_OK == ntcip1218_lock()){
      data = ntcip_1218_mib.Async.rsuAuthError;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  } 
  if(rsuAlertLevel_denso_empty == data) {
      data = rsuAlertLevel_info;
  }
  if((rsuAlertLevel_info <= data) && (data <= rsuAlertLevel_critical)){
      return (int32_t)data; 
  } else {
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  }
}
/* 5.18.2.3 */
int32_t get_rsuAuthMsg(uint8_t * data_out)
{
  int32_t size = 0;
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      size = ntcip_1218_mib.Async.rsuAuth_length;
      memcpy(data_out, ntcip_1218_mib.Async.rsuAuthMsg, RSU_ALERT_MSG_LENGTH_MAX);
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
  if((size < RSU_ALERT_MSG_LENGTH_MIN) || (RSU_ALERT_MSG_LENGTH_MAX < size)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);  
      return RSEMIB_BAD_DATA;
  }
  return size;
}

/* 5.18.1.4 */
int32_t get_rsuSignatureVerifyError(void)
{
  rsuAlertLevel_e data = RSU_ALERT_DEFAULT;
  if(RSEMIB_OK == ntcip1218_lock()){
      data = ntcip_1218_mib.Async.rsuSignatureVerifyError;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
  if(rsuAlertLevel_denso_empty == data) {
      data = rsuAlertLevel_info;
  }
  if((rsuAlertLevel_info <= data) && (data <= rsuAlertLevel_critical)){
      return (int32_t)data; 
  } else {
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  }
}
/* 5.18.2.4 */
int32_t get_rsuSignatureVerifyMsg(uint8_t * data_out)
{
  int32_t size = 0;
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      size = ntcip_1218_mib.Async.rsuSignatureVerify_length;
      memcpy(data_out, ntcip_1218_mib.Async.rsuSignatureVerifyMsg, RSU_ALERT_MSG_LENGTH_MAX);
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
  if((size < RSU_ALERT_MSG_LENGTH_MIN) || (RSU_ALERT_MSG_LENGTH_MAX < size)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);  
      return RSEMIB_BAD_DATA;
  }
  return size;
}

/* 5.18.1.5 */
int32_t get_rsuAccessError(void)
{
  rsuAlertLevel_e data = RSU_ALERT_DEFAULT;
  if(RSEMIB_OK == ntcip1218_lock()){
      data = ntcip_1218_mib.Async.rsuAccessError;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
  if(rsuAlertLevel_denso_empty == data) {
      data = rsuAlertLevel_info;
  }
  if((rsuAlertLevel_info <= data) && (data <= rsuAlertLevel_critical)){
      return (int32_t)data; 
  } else {
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  }
}
/* 5.18.2.5 */
int32_t get_rsuAccessMsg(uint8_t * data_out)
{
  int32_t size = 0;
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      size = ntcip_1218_mib.Async.rsuAccess_length;
      memcpy(data_out, ntcip_1218_mib.Async.rsuAccessMsg, RSU_ALERT_MSG_LENGTH_MAX);
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
  if((size < RSU_ALERT_MSG_LENGTH_MIN) || (RSU_ALERT_MSG_LENGTH_MAX < size)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);  
      return RSEMIB_BAD_DATA;
  }
  return size;
}

/* 5.18.1.6 */
int32_t get_rsuTimeSourceLost(void)
{
  rsuAlertLevel_e data = RSU_ALERT_DEFAULT;
  if(RSEMIB_OK == ntcip1218_lock()){
      data = ntcip_1218_mib.Async.rsuTimeSourceLost;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
  if(rsuAlertLevel_denso_empty == data) {
      data = rsuAlertLevel_info;
  }
  if((rsuAlertLevel_info <= data) && (data <= rsuAlertLevel_critical)){
      return (int32_t)data; 
  } else {
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  }
}
/* 5.18.2.6 */
int32_t get_rsuTimeSourceLostMsg(uint8_t * data_out)
{
  int32_t size = 0;
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      size = ntcip_1218_mib.Async.rsuTimeSourceLost_length;
      memcpy(data_out, ntcip_1218_mib.Async.rsuTimeSourceLostMsg, RSU_ALERT_MSG_LENGTH_MAX);
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
  if((size < RSU_ALERT_MSG_LENGTH_MIN) || (RSU_ALERT_MSG_LENGTH_MAX < size)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);  
      return RSEMIB_BAD_DATA;
  }
  return size;
}

/* 5.18.1.7 */
int32_t get_rsuTimeSourceMismatch(void)
{
  rsuAlertLevel_e data = RSU_ALERT_DEFAULT;
  if(RSEMIB_OK == ntcip1218_lock()){
      data = ntcip_1218_mib.Async.rsuTimeSourceMismatch;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
  if(rsuAlertLevel_denso_empty == data) {
      data = rsuAlertLevel_info;
  }
  if((rsuAlertLevel_info <= data) && (data <= rsuAlertLevel_critical)){
      return (int32_t)data; 
  } else {
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  }
}
/* 5.18.2.7 */
int32_t get_rsuTimeSourceMismatchMsg(uint8_t * data_out)
{
  int32_t size = 0;
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      size = ntcip_1218_mib.Async.rsuTimeSourceLost_length;
      memcpy(data_out, ntcip_1218_mib.Async.rsuTimeSourceMismatchMsg, RSU_ALERT_MSG_LENGTH_MAX);
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
  if((size < RSU_ALERT_MSG_LENGTH_MIN) || (RSU_ALERT_MSG_LENGTH_MAX < size)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);  
      return RSEMIB_BAD_DATA;
  }
  return size;
}

/* 5.18.1.8 */
int32_t get_rsuGnssAnomaly(void)
{
  rsuAlertLevel_e data = RSU_ALERT_DEFAULT;
  if(RSEMIB_OK == ntcip1218_lock()){
      data = ntcip_1218_mib.Async.rsuGnssAnomaly;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
  if(rsuAlertLevel_denso_empty == data) {
      data = rsuAlertLevel_info;
  }
  if((rsuAlertLevel_info <= data) && (data <= rsuAlertLevel_critical)){
      return (int32_t)data; 
  } else {
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  }
}
/* 5.18.2.8 */
int32_t get_rsuGnssAnomalyMsg(uint8_t * data_out)
{
  int32_t size = 0;
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      size = ntcip_1218_mib.Async.rsuGnssAnomaly_length;
      memcpy(data_out, ntcip_1218_mib.Async.rsuGnssAnomalyMsg, RSU_ALERT_MSG_LENGTH_MAX);
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
  if((size < RSU_ALERT_MSG_LENGTH_MIN) || (RSU_ALERT_MSG_LENGTH_MAX < size)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);  
      return RSEMIB_BAD_DATA;
  }
  return size;
}

/* 5.18.1.9 */
int32_t get_rsuGnssDeviationError(void)
{
  int32_t data = 0;
  if(RSEMIB_OK == ntcip1218_lock()){
      data = ntcip_1218_mib.Async.rsuGnssDeviationError;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  } 
  if(rsuAlertLevel_denso_empty == data) {
      data = rsuAlertLevel_info;
  }
  if((rsuAlertLevel_info <= data) && (data <= rsuAlertLevel_critical)){
      return data; 
  } else {
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } 
}
/* 5.18.2.9 */
int32_t get_rsuGnssDeviationMsg(uint8_t * data_out)
{
  int32_t size = 0;
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      size = ntcip_1218_mib.Async.rsuGnssDeviation_length;
      memcpy(data_out, ntcip_1218_mib.Async.rsuGnssDeviationMsg, RSU_ALERT_MSG_LENGTH_MAX);
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
  if((size < RSU_ALERT_MSG_LENGTH_MIN) || (RSU_ALERT_MSG_LENGTH_MAX < size)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);  
      return RSEMIB_BAD_DATA;
  }
  return size;
}

/* 5.18.1.10 */
int32_t get_rsuGnssNmeaNotify(void)
{
  int32_t data = 0;
  if(RSEMIB_OK == ntcip1218_lock()){
      data = ntcip_1218_mib.Async.rsuGnssNmeaNotify;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
  if(rsuAlertLevel_denso_empty == data) {
      data = rsuAlertLevel_info;
  }
  if((rsuAlertLevel_info <= data) && (data <= rsuAlertLevel_critical)){
      return data; 
  } else {
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } 
}
/* 5.18.2.10 */
int32_t get_rsuGnssNmeaNotifyInterval(void)
{
  int32_t data = 0;
  if(RSEMIB_OK == ntcip1218_lock()){
      data = ntcip_1218_mib.Async.rsuGnssNmeaNotifyInterval;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  } 
  if((RSU_NOTIFY_INTERVAL_RATE_MIN <= data) && (data <= RSU_NOTIFY_INTERVAL_RATE_MAX)){
      return data; 
  } else {
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } 
}
int32_t preserve_rsuGnssNmeaNotifyInterval(void)
{
  if(RSEMIB_OK == ntcip1218_lock()){
      prior_ntcip_1218_mib.Async.rsuGnssNmeaNotifyInterval = ntcip_1218_mib.Async.rsuGnssNmeaNotifyInterval;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t action_rsuGnssNmeaNotifyInterval(int32_t data)
{
  if((data < RSU_NOTIFY_INTERVAL_RATE_MIN ) || (RSU_NOTIFY_INTERVAL_RATE_MAX < data)){ 
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  }
  if(RSEMIB_OK == ntcip1218_lock()){  
      tmp_ntcip_1218_mib.Async.rsuGnssNmeaNotifyInterval = data;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t commit_rsuGnssNmeaNotifyInterval(void)
{
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.Async.rsuGnssNmeaNotifyInterval      = tmp_ntcip_1218_mib.Async.rsuGnssNmeaNotifyInterval; 
      tmp_ntcip_1218_mib.Async.rsuGnssNmeaNotifyInterval  = RSU_NOTIFY_INTERVAL_RATE_MIN;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t undo_rsuGnssNmeaNotifyInterval(void)
{
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.Async.rsuGnssNmeaNotifyInterval = prior_ntcip_1218_mib.Async.rsuGnssNmeaNotifyInterval;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}

/* 5.18.1.11 */
int32_t get_rsuCertificateError(void)
{
  rsuAlertLevel_e data = RSU_ALERT_DEFAULT;
  if(RSEMIB_OK == ntcip1218_lock()){
      data = ntcip_1218_mib.Async.rsuCertificateError;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
  if(rsuAlertLevel_denso_empty == data) {
      data = rsuAlertLevel_info;
  }
  if((rsuAlertLevel_info <= data) && (data <= rsuAlertLevel_critical)){
      return (int32_t)data; 
  } else {
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  }
}
/* 5.18.2.12: Weird but this is how they numbered it */
int32_t get_rsuCertificateMsg(uint8_t * data_out)
{
  int32_t size = 0;
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      size = ntcip_1218_mib.Async.rsuCertificate_length;
      memcpy(data_out, ntcip_1218_mib.Async.rsuCertificateMsg, RSU_ALERT_MSG_LENGTH_MAX);
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
  if((size < RSU_ALERT_MSG_LENGTH_MIN) || (RSU_ALERT_MSG_LENGTH_MAX < size)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);  
      return RSEMIB_BAD_DATA;
  }
  return size;
}

/* 5.18.1.12 */
int32_t get_rsuServiceDenialError(void)
{
  rsuAlertLevel_e data = RSU_ALERT_DEFAULT;
  if(RSEMIB_OK == ntcip1218_lock()){
      data = ntcip_1218_mib.Async.rsuServiceDenialError;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
  if(rsuAlertLevel_denso_empty == data) {
      data = rsuAlertLevel_info;
  }
  if((rsuAlertLevel_info <= data) && (data <= rsuAlertLevel_critical)){
      return (int32_t)data; 
  } else {
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  }
}
/* 5.18.2.13 */
int32_t get_rsuServiceDenialMsg(uint8_t * data_out)
{
  int32_t size = 0;
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      size = ntcip_1218_mib.Async.rsuServiceDenial_length;
      memcpy(data_out, ntcip_1218_mib.Async.rsuServiceDenialMsg, RSU_ALERT_MSG_LENGTH_MAX);
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
  if((size < RSU_ALERT_MSG_LENGTH_MIN) || (RSU_ALERT_MSG_LENGTH_MAX < size)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);  
      return RSEMIB_BAD_DATA;
  }
  return size;
}

/* 5.18.1.13 */
int32_t get_rsuWatchdogError(void)
{
  rsuAlertLevel_e data = RSU_ALERT_DEFAULT;
  if(RSEMIB_OK == ntcip1218_lock()){
      data = ntcip_1218_mib.Async.rsuWatchdogError;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  } 
  if(rsuAlertLevel_denso_empty == data) {
      data = rsuAlertLevel_info;
  }
  if((rsuAlertLevel_info <= data) && (data <= rsuAlertLevel_critical)){
      return (int32_t)data; 
  } else {
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  }
}
/* 5.18.2.14 */
int32_t get_rsuWatchdogMsg(uint8_t * data_out)
{
  int32_t size = 0;
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      size = ntcip_1218_mib.Async.rsuWatchdog_length;
      memcpy(data_out, ntcip_1218_mib.Async.rsuWatchdogMsg, RSU_ALERT_MSG_LENGTH_MAX);
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
  if((size < RSU_ALERT_MSG_LENGTH_MIN) || (RSU_ALERT_MSG_LENGTH_MAX < size)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);  
      return RSEMIB_BAD_DATA;
  }
  return size;
}

/* 5.18.1.14 */
int32_t get_rsuEnvironError(void)
{
  rsuAlertLevel_e data = RSU_ALERT_DEFAULT;
  if(RSEMIB_OK == ntcip1218_lock()){
      data = ntcip_1218_mib.Async.rsuEnvironError;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
  if(rsuAlertLevel_denso_empty == data) {
      data = rsuAlertLevel_info;
  }
  if((rsuAlertLevel_info <= data) && (data <= rsuAlertLevel_critical)){
      return (int32_t)data; 
  } else {
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  }
}
/* 5.18.2.15 */
int32_t get_rsuEnvironMsg(uint8_t * data_out)
{
  int32_t size = 0;
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      size = ntcip_1218_mib.Async.rsuEnviron_length;
      memcpy(data_out, ntcip_1218_mib.Async.rsuEnvironMsg, RSU_ALERT_MSG_LENGTH_MAX);
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
  if((size < RSU_ALERT_MSG_LENGTH_MIN) || (RSU_ALERT_MSG_LENGTH_MAX < size)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);  
      return RSEMIB_BAD_DATA;
  }
  return size;
}

/* 5.18.2.11: Odd one out: error level but no error msg like the others: 0..4: rsuAlertLevel_e */
int32_t get_rsuAlertLevel(void)
{
  rsuAlertLevel_e data = RSU_ALERT_DEFAULT;
  if(RSEMIB_OK == ntcip1218_lock()){
      data = ntcip_1218_mib.Async.rsuAlertLevel;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
  if(rsuAlertLevel_denso_empty == data) {
      data = rsuAlertLevel_info;
  }
  if((rsuAlertLevel_info <= data) && (data <= rsuAlertLevel_critical)){
      return (int32_t)data; 
  } else {
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  }
}

/* 5.18.3 Notification Repeat Interval */
int32_t get_rsuNotificationRepeatInterval(void)
{
  int32_t data = 0;
  if(RSEMIB_OK == ntcip1218_lock()){
      data = ntcip_1218_mib.Async.rsuNotificationRepeatInterval;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  } 
  if((RSU_NOTIFY_REPEAT_RATE_MIN <= data) && (data <= RSU_NOTIFY_REPEAT_RATE_MAX)){
      return data; 
  } else {
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } 
}
int32_t preserve_rsuNotificationRepeatInterval(void)
{
  if(RSEMIB_OK == ntcip1218_lock()){
      prior_ntcip_1218_mib.Async.rsuNotificationRepeatInterval = ntcip_1218_mib.Async.rsuNotificationRepeatInterval;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t action_rsuNotificationRepeatInterval(int32_t data)
{
  if((data < RSU_NOTIFY_REPEAT_RATE_MIN) || (RSU_NOTIFY_REPEAT_RATE_MAX < data)){ 
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  }
  if(RSEMIB_OK == ntcip1218_lock()){  
      tmp_ntcip_1218_mib.Async.rsuNotificationRepeatInterval = data;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t commit_rsuNotificationRepeatInterval(void)
{
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.Async.rsuNotificationRepeatInterval     = tmp_ntcip_1218_mib.Async.rsuNotificationRepeatInterval; 
      tmp_ntcip_1218_mib.Async.rsuNotificationRepeatInterval = 0;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t undo_rsuNotificationRepeatInterval(void)
{
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.Async.rsuNotificationRepeatInterval = prior_ntcip_1218_mib.Async.rsuNotificationRepeatInterval;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}

/* 5.18.4 Notification Maximum Retries */
int32_t get_rsuNotificationMaxRetries(void)
{
  int32_t data = 0;
  if(RSEMIB_OK == ntcip1218_lock()){
      data = ntcip_1218_mib.Async.rsuNotificationMaxRetries;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  } 
  if((RSU_NOTIFY_MSG_RETRY_MIN <= data) && (data <= RSU_NOTIFY_MSG_RETRY_MAX)){
      return data; 
  } else {
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } 
}
int32_t preserve_rsuNotificationMaxRetries(void)
{
  if(RSEMIB_OK == ntcip1218_lock()){
      prior_ntcip_1218_mib.Async.rsuNotificationMaxRetries = ntcip_1218_mib.Async.rsuNotificationMaxRetries;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t action_rsuNotificationMaxRetries(int32_t data)
{
  if((data < RSU_NOTIFY_MSG_RETRY_MIN) || (RSU_NOTIFY_MSG_RETRY_MAX < data)){ 
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  }
  if(RSEMIB_OK == ntcip1218_lock()){  
      tmp_ntcip_1218_mib.Async.rsuNotificationMaxRetries = data;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t commit_rsuNotificationMaxRetries(void)
{
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.Async.rsuNotificationMaxRetries     = tmp_ntcip_1218_mib.Async.rsuNotificationMaxRetries; 
      tmp_ntcip_1218_mib.Async.rsuNotificationMaxRetries = 0;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t undo_rsuNotificationMaxRetries(void)
{
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.Async.rsuNotificationMaxRetries = prior_ntcip_1218_mib.Async.rsuNotificationMaxRetries;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      return RSEMIB_LOCK_FAIL;
  }
}

/******************************************************************************
 * 5.19 RSU Applications: { rsu 18 }: 32 DSRC MSG ID's.
 ******************************************************************************/
/* 5.19.1 */
int32_t get_maxRsuApps(void)
{
  int32_t data = RSU_ANTENNA_MAX;
  if(RSEMIB_OK == ntcip1218_lock()){
      data = ntcip_1218_mib.maxRsuApps;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  } 
  if((RSU_APPS_RUNNING_MIN <= data) && (data <= RSU_APPS_RUNNING_MAX)){
      return data; 
  } else {
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } 
}
/* 5.19.2.2 */
int32_t get_rsuAppConfigName(int32_t index, uint8_t * data_out)
{
  int32_t size = 0;
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if((index < RSU_APPS_RUNNING_MIN) || (RSU_APPS_RUNNING_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      size = ntcip_1218_mib.rsuAppConfigTable[index-1].rsuAppConfigName_length;
      memcpy(data_out, ntcip_1218_mib.rsuAppConfigTable[index-1].rsuAppConfigName, RSU_APP_CONF_NAME_LENGTH_MAX);
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
  if((size < RSU_APP_CONF_NAME_LENGTH_MIN) || (RSU_APP_CONF_NAME_LENGTH_MAX < size)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } 
  return size;
}

/* 5.19.2.3 */
int32_t get_rsuAppConfigStartup(int32_t index, int32_t * data_out)
{
  int32_t data = 0;
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if((index < RSU_APPS_RUNNING_MIN) || (RSU_APPS_RUNNING_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      data = ntcip_1218_mib.rsuAppConfigTable[index-1].rsuAppConfigStartup;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
  if((data < rsuAppConfigStartup_other) || (rsuAppConfigStartup_notStartup < data)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } 
  *data_out = data;
  return RSEMIB_OK;

}
int32_t preserve_rsuAppConfigStartup(int32_t index)
{
  if((index < RSU_APPS_RUNNING_MIN) || (RSU_APPS_RUNNING_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      prior_ntcip_1218_mib.rsuAppConfigTable[index-1].rsuAppConfigStartup = ntcip_1218_mib.rsuAppConfigTable[index-1].rsuAppConfigStartup;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t action_rsuAppConfigStartup(int32_t index, int32_t data_in)
{
  if((index < RSU_APPS_RUNNING_MIN) || (RSU_APPS_RUNNING_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if((data_in < rsuAppConfigStartup_other) || (rsuAppConfigStartup_notStartup < data_in)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  }
  if(RSEMIB_OK == ntcip1218_lock()){  
      tmp_ntcip_1218_mib.rsuAppConfigTable[index-1].rsuAppConfigStartup = data_in;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t commit_rsuAppConfigStartup(int32_t index)
{
  if((index < RSU_APPS_RUNNING_MIN) || (RSU_APPS_RUNNING_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuAppConfigTable[index-1].rsuAppConfigStartup = tmp_ntcip_1218_mib.rsuAppConfigTable[index-1].rsuAppConfigStartup;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t undo_rsuAppConfigStartup(int32_t index)
{
  if((index < RSU_APPS_RUNNING_MIN) || (RSU_APPS_RUNNING_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuAppConfigTable[index-1].rsuAppConfigStartup = prior_ntcip_1218_mib.rsuAppConfigTable[index-1].rsuAppConfigStartup;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}

/* 5.19.2.4 */
int32_t get_rsuAppConfigState(int32_t index, int32_t * data_out)
{
  int32_t data = 0;
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if((index < RSU_APPS_RUNNING_MIN) || (RSU_APPS_RUNNING_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      data = ntcip_1218_mib.rsuAppConfigTable[index-1].rsuAppConfigState;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
  if((data < 0) || (1 < data)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } 
  *data_out = data;
  return RSEMIB_OK;

}
/* 5.19.2.5 */
int32_t get_rsuAppConfigStart(int32_t index, int32_t * data_out)
{
  int32_t data = 0;
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if((index < RSU_APPS_RUNNING_MIN) || (RSU_APPS_RUNNING_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      data = ntcip_1218_mib.rsuAppConfigTable[index-1].rsuAppConfigStart;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
  if((data < 0) || (1 < data)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } 
  *data_out = data;
  return RSEMIB_OK;

}
int32_t preserve_rsuAppConfigStart(int32_t index)
{
  if((index < RSU_APPS_RUNNING_MIN) || (RSU_APPS_RUNNING_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      prior_ntcip_1218_mib.rsuAppConfigTable[index-1].rsuAppConfigStart = ntcip_1218_mib.rsuAppConfigTable[index-1].rsuAppConfigStart;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t action_rsuAppConfigStart(int32_t index, int32_t data_in)
{
  if((index < RSU_APPS_RUNNING_MIN) || (RSU_APPS_RUNNING_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if((data_in < 0) || (1 < data_in)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  }
  if(RSEMIB_OK == ntcip1218_lock()){  
      tmp_ntcip_1218_mib.rsuAppConfigTable[index-1].rsuAppConfigStart = data_in;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t commit_rsuAppConfigStart(int32_t index)
{
  if((index < RSU_APPS_RUNNING_MIN) || (RSU_APPS_RUNNING_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuAppConfigTable[index-1].rsuAppConfigStart = tmp_ntcip_1218_mib.rsuAppConfigTable[index-1].rsuAppConfigStart;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t undo_rsuAppConfigStart(int32_t index)
{
  if((index < RSU_APPS_RUNNING_MIN) || (RSU_APPS_RUNNING_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuAppConfigTable[index-1].rsuAppConfigStart = prior_ntcip_1218_mib.rsuAppConfigTable[index-1].rsuAppConfigStart;
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
  
/* 5.19.2.6 */
int32_t get_rsuAppConfigStop(int32_t index, int32_t * data_out)
{
  int32_t data = 0;
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if((index < RSU_APPS_RUNNING_MIN) || (RSU_APPS_RUNNING_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      data = ntcip_1218_mib.rsuAppConfigTable[index-1].rsuAppConfigStop;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
  if((data < 0) || (1 < data)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } 
  *data_out = data;
  return RSEMIB_OK;
}
int32_t preserve_rsuAppConfigStop(int32_t index)
{
  if((index < RSU_APPS_RUNNING_MIN) || (RSU_APPS_RUNNING_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      prior_ntcip_1218_mib.rsuAppConfigTable[index-1].rsuAppConfigStop = ntcip_1218_mib.rsuAppConfigTable[index-1].rsuAppConfigStop;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t action_rsuAppConfigStop(int32_t index, int32_t data_in)
{
  if((index < RSU_APPS_RUNNING_MIN) || (RSU_APPS_RUNNING_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if((data_in < 0) || (1 < data_in)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  }
  if(RSEMIB_OK == ntcip1218_lock()){  
      tmp_ntcip_1218_mib.rsuAppConfigTable[index-1].rsuAppConfigStop = data_in;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t commit_rsuAppConfigStop(int32_t index)
{
  if((index < RSU_APPS_RUNNING_MIN) || (RSU_APPS_RUNNING_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuAppConfigTable[index-1].rsuAppConfigStop = tmp_ntcip_1218_mib.rsuAppConfigTable[index-1].rsuAppConfigStop;
      tmp_ntcip_1218_mib.rsuAppConfigTable[index-1].rsuAppConfigStop = 0;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t undo_rsuAppConfigStop(int32_t index)
{
  if((index < RSU_APPS_RUNNING_MIN) || (RSU_APPS_RUNNING_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuAppConfigTable[index-1].rsuAppConfigStop = prior_ntcip_1218_mib.rsuAppConfigTable[index-1].rsuAppConfigStop;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}

/******************************************************************************
 * 5.20 RSU Services: { rsu 19 }:
 ******************************************************************************/

/* 5.20.1 */
int32_t get_maxRsuServices(void)
{
  int32_t data = RSU_SERVICES_RUNNING_MAX;

  if(RSEMIB_OK == ntcip1218_lock()){
      data = ntcip_1218_mib.maxRsuServices;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  } 
  if((RSU_SERVICES_RUNNING_MIN <= data) && (data <= RSU_APPS_RUNNING_MAX)){
      return data; 
  } else {
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } 
}
/* 5.20.2.2 */
int32_t get_rsuServiceName(int32_t index, uint8_t * data_out)
{
  int32_t size = 0;
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if((index < RSU_SERVICES_RUNNING_MIN) || (RSU_SERVICES_RUNNING_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      size = ntcip_1218_mib.rsuServiceTable[index-1].rsuServiceName_length;
      memcpy(data_out, ntcip_1218_mib.rsuServiceTable[index-1].rsuServiceName, RSU_SERVICE_NAME_LENGTH_MAX);
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
  if((size < RSU_SERVICE_NAME_LENGTH_MIN) || (RSU_SERVICE_NAME_LENGTH_MAX < size)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } 
  return size;
}
int32_t preserve_rsuServiceName(int32_t index)
{
  if((index < RSU_SERVICES_RUNNING_MIN) || (RSU_SERVICES_RUNNING_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      prior_ntcip_1218_mib.rsuServiceTable[index-1].rsuServiceName_length = 
          ntcip_1218_mib.rsuServiceTable[index-1].rsuServiceName_length;
      memcpy(prior_ntcip_1218_mib.rsuServiceTable[index-1].rsuServiceName, 
          ntcip_1218_mib.rsuServiceTable[index-1].rsuServiceName, RSU_SERVICE_NAME_LENGTH_MAX);
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t action_rsuServiceName(int32_t index, uint8_t * data_in, int32_t length)
{
  if((index < RSU_SERVICES_RUNNING_MIN) || (RSU_SERVICES_RUNNING_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  /* Don't allow write access to reserved service. */
  if((RSU_SERVICE_RESERVED_INDEX_MIN <= index) && (index <= RSU_SERVICE_RESERVED_INDEX_MAX)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(NULL == data_in){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }

  if((length < RSU_SERVICE_NAME_LENGTH_MIN) || (RSU_SERVICE_NAME_LENGTH_MAX < length)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      memset(tmp_ntcip_1218_mib.rsuServiceTable[index-1].rsuServiceName, 0x0, RSU_SERVICE_NAME_LENGTH_MAX); 
      memcpy(tmp_ntcip_1218_mib.rsuServiceTable[index-1].rsuServiceName, data_in, length);
      tmp_ntcip_1218_mib.rsuServiceTable[index-1].rsuServiceName_length = length;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}

int32_t commit_rsuServiceName(int32_t index)
{
  if((index < RSU_SERVICES_RUNNING_MIN) || (RSU_SERVICES_RUNNING_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      memcpy( ntcip_1218_mib.rsuServiceTable[index-1].rsuServiceName
            , tmp_ntcip_1218_mib.rsuServiceTable[index-1].rsuServiceName, RSU_SERVICE_NAME_LENGTH_MAX);
      ntcip_1218_mib.rsuServiceTable[index-1].rsuServiceName_length =
              tmp_ntcip_1218_mib.rsuServiceTable[index-1].rsuServiceName_length;     
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t undo_rsuServiceName(int32_t index)
{
  if((index < RSU_SERVICES_RUNNING_MIN) || (RSU_SERVICES_RUNNING_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      memcpy(  ntcip_1218_mib.rsuServiceTable[index-1].rsuServiceName
             , prior_ntcip_1218_mib.rsuServiceTable[index-1].rsuServiceName, RSU_SERVICE_NAME_LENGTH_MAX);
      ntcip_1218_mib.rsuServiceTable[index-1].rsuServiceName_length = 
               prior_ntcip_1218_mib.rsuServiceTable[index-1].rsuServiceName_length;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}

/* 5.20.2.3 */
int32_t get_rsuServiceStatus(int32_t index)
{
  rsuServiceStatus_e data = rsuServiceStatus_unknown;

  if((index < RSU_SERVICES_RUNNING_MIN) || (RSU_SERVICES_RUNNING_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      data = ntcip_1218_mib.rsuServiceTable[index-1].rsuServiceStatus;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  } 
  if((rsuServiceStatus_other <= data) && (data <= rsuServiceStatus_unknown)){
      return (int32_t)data; 
  } else {
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } 
}

/* 5.20.2.4 */
int32_t get_rsuServiceStatusDesc(int32_t index, uint8_t * data_out)
{
  int32_t size = 0;
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if((index < RSU_SERVICES_RUNNING_MIN) || (RSU_SERVICES_RUNNING_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      size = ntcip_1218_mib.rsuServiceTable[index-1].rsuServiceStatusDesc_length;
      memcpy(data_out, ntcip_1218_mib.rsuServiceTable[index-1].rsuServiceStatusDesc, RSU_SERVICE_STATUS_DESC_LENGTH_MAX);
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
  if((size < RSU_SERVICE_STATUS_DESC_LENGTH_MIN) || (RSU_SERVICE_STATUS_DESC_LENGTH_MAX < size)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } 
  return size;
}

/* 5.20.2.5 */
int32_t get_rsuServiceStatusTime(int32_t index, uint8_t * data_out)
{
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if((index < RSU_SERVICES_RUNNING_MIN) || (RSU_SERVICES_RUNNING_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      memcpy(data_out, ntcip_1218_mib.rsuServiceTable[index-1].rsuServiceStatusTime, MIB_DATEANDTIME_LENGTH);
      ntcip1218_unlock();
      return MIB_DATEANDTIME_LENGTH;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}

/******************************************************************************
 * 5.21 Transmitted Messages For Forwarding: { rsu 20 }: 
 ******************************************************************************/

/* 5.21.1 */
int32_t get_maxXmitMsgFwding(void)
{
  int32_t data = 0;
  if(RSEMIB_OK == ntcip1218_lock()){
      data = ntcip_1218_mib.maxXmitMsgFwding;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
  if((RSU_XMIT_MSG_COUNT_MIN <= data) && (data <= RSU_XMIT_MSG_COUNT_MAX)){
      return data; 
  } else {
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } 
}

/* 5.21.2.1 */
int32_t get_rsuXmitMsgFwdingIndex(int32_t index, int32_t * data_out)
{
  if((index < RSU_XMIT_MSG_COUNT_MIN) || (RSU_XMIT_MSG_COUNT_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      *data_out = ntcip_1218_mib.rsuXmitMsgFwdingTable[index - 1].rsuXmitMsgFwdingIndex;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
  if((*data_out < RSU_XMIT_MSG_COUNT_MIN) || (RSU_XMIT_MSG_COUNT_MAX < *data_out)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } 
  return RSEMIB_OK;
}

/* 5.21.2.2 */
int32_t get_rsuXmitMsgFwdingPsid(int32_t index, uint8_t * data_out)
{
  int32_t length = 0;
  if((index < RSU_XMIT_MSG_COUNT_MIN) || (RSU_XMIT_MSG_COUNT_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      length = ntcip_1218_mib.rsuXmitMsgFwdingTable[index-1].rsuXmitMsgFwdingPsid_length;
      if(   (RSU_RADIO_PSID_MIN_SIZE <= length) 
         && (length <= RSU_RADIO_PSID_SIZE)){
          memcpy(data_out, ntcip_1218_mib.rsuXmitMsgFwdingTable[index-1].rsuXmitMsgFwdingPsid, length);
          ntcip1218_unlock();
          return length;
      } else {
          ntcip1218_unlock();
          return RSEMIB_BAD_DATA;
      }
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t preserve_rsuXmitMsgFwdingPsid(int32_t index)
{
  if((index < RSU_XMIT_MSG_COUNT_MIN) || (RSU_XMIT_MSG_COUNT_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      memcpy( prior_ntcip_1218_mib.rsuXmitMsgFwdingTable[index-1].rsuXmitMsgFwdingPsid
            , ntcip_1218_mib.rsuXmitMsgFwdingTable[index-1].rsuXmitMsgFwdingPsid, RSU_RADIO_PSID_SIZE);
      prior_ntcip_1218_mib.rsuXmitMsgFwdingTable[index-1].rsuXmitMsgFwdingPsid_length = 
          ntcip_1218_mib.rsuXmitMsgFwdingTable[index-1].rsuXmitMsgFwdingPsid_length;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t action_rsuXmitMsgFwdingPsid(int32_t index, uint8_t * data_in, int32_t length)
{
  uint32_t psid = 0x0;
  if((index < RSU_XMIT_MSG_COUNT_MIN) || (RSU_XMIT_MSG_COUNT_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  } 
  if(NULL == data_in){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if((length < RSU_RADIO_PSID_MIN_SIZE) || (RSU_RADIO_PSID_SIZE < length)){
      return RSEMIB_BAD_INPUT;  
  }
  /* Reject known bogus PSID's. */
  if(length >= 1) {
      psid  = (uint32_t)(data_in[0]);
  }
  if(length >= 2) {
      psid  = psid << 8;
      psid  |= (uint32_t)(data_in[1]);
  }
  if(length >= 3) {
      psid  = psid << 8;
      psid  |= (uint32_t)(data_in[2]);
  }
  if(length == 4) {
      psid  = psid << 8;
      psid  |= (uint32_t)(data_in[3]);
  }
  if(psid <= (uint32_t) PSID_1BYTE_MAX_VALUE){
  } else if(psid >= (uint32_t)PSID_2BYTE_MIN_VALUE && psid <= (uint32_t)PSID_2BYTE_MAX_VALUE){
  } else if(psid >= (uint32_t)PSID_3BYTE_MIN_VALUE && psid <= (uint32_t)PSID_3BYTE_MAX_VALUE){
  } else if(psid >= (uint32_t)PSID_4BYTE_MIN_VALUE && psid <= (uint32_t)PSID_4BYTE_MAX_VALUE){
  } else {
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){  
      memset(tmp_ntcip_1218_mib.rsuXmitMsgFwdingTable[index-1].rsuXmitMsgFwdingPsid, 0x0, RSU_RADIO_PSID_SIZE);
      if(1 == length) {
          tmp_ntcip_1218_mib.rsuXmitMsgFwdingTable[index-1].rsuXmitMsgFwdingPsid[3] = data_in[0];
      }
      if(2 == length){
          tmp_ntcip_1218_mib.rsuXmitMsgFwdingTable[index-1].rsuXmitMsgFwdingPsid[3] = data_in[1];
          tmp_ntcip_1218_mib.rsuXmitMsgFwdingTable[index-1].rsuXmitMsgFwdingPsid[2] = data_in[0];
      }
      if(3 == length) {
          tmp_ntcip_1218_mib.rsuXmitMsgFwdingTable[index-1].rsuXmitMsgFwdingPsid[3] = data_in[2];
          tmp_ntcip_1218_mib.rsuXmitMsgFwdingTable[index-1].rsuXmitMsgFwdingPsid[2] = data_in[1];
          tmp_ntcip_1218_mib.rsuXmitMsgFwdingTable[index-1].rsuXmitMsgFwdingPsid[1] = data_in[0];
      }
      if(4 == length) {
          tmp_ntcip_1218_mib.rsuXmitMsgFwdingTable[index-1].rsuXmitMsgFwdingPsid[3] = data_in[3];
          tmp_ntcip_1218_mib.rsuXmitMsgFwdingTable[index-1].rsuXmitMsgFwdingPsid[2] = data_in[2];
          tmp_ntcip_1218_mib.rsuXmitMsgFwdingTable[index-1].rsuXmitMsgFwdingPsid[1] = data_in[1];
          tmp_ntcip_1218_mib.rsuXmitMsgFwdingTable[index-1].rsuXmitMsgFwdingPsid[0] = data_in[0];
      }
      tmp_ntcip_1218_mib.rsuXmitMsgFwdingTable[index-1].rsuXmitMsgFwdingPsid_length = length;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t commit_rsuXmitMsgFwdingPsid(int32_t index)
{
  if((index < RSU_XMIT_MSG_COUNT_MIN) || (RSU_XMIT_MSG_COUNT_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  } 
  if(RSEMIB_OK == ntcip1218_lock()){

      memcpy( ntcip_1218_mib.rsuXmitMsgFwdingTable[index-1].rsuXmitMsgFwdingPsid
            , tmp_ntcip_1218_mib.rsuXmitMsgFwdingTable[index-1].rsuXmitMsgFwdingPsid, RSU_RADIO_PSID_SIZE);
      ntcip_1218_mib.rsuXmitMsgFwdingTable[index-1].rsuXmitMsgFwdingPsid_length = 
          tmp_ntcip_1218_mib.rsuXmitMsgFwdingTable[index-1].rsuXmitMsgFwdingPsid_length;
      memset(tmp_ntcip_1218_mib.rsuXmitMsgFwdingTable[index-1].rsuXmitMsgFwdingPsid, 0x0, RSU_RADIO_PSID_SIZE);
      tmp_ntcip_1218_mib.rsuXmitMsgFwdingTable[index-1].rsuXmitMsgFwdingPsid_length = 0;

      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t undo_rsuXmitMsgFwdingPsid(int32_t index)
{
  if((index < RSU_XMIT_MSG_COUNT_MIN) || (RSU_XMIT_MSG_COUNT_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){

      memcpy( ntcip_1218_mib.rsuXmitMsgFwdingTable[index-1].rsuXmitMsgFwdingPsid
            , prior_ntcip_1218_mib.rsuXmitMsgFwdingTable[index-1].rsuXmitMsgFwdingPsid, RSU_RADIO_PSID_SIZE);
      ntcip_1218_mib.rsuXmitMsgFwdingTable[index-1].rsuXmitMsgFwdingPsid_length = 
          prior_ntcip_1218_mib.rsuXmitMsgFwdingTable[index-1].rsuXmitMsgFwdingPsid_length;

      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}

/* 5.21.2.3 */
int32_t get_rsuXmitMsgFwdingDestIpAddr(int32_t index, uint8_t * data_out)
{
  int32_t size = 0;

  if((index < RSU_XMIT_MSG_COUNT_MIN) || (RSU_XMIT_MSG_COUNT_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      size = ntcip_1218_mib.rsuXmitMsgFwdingTable[index-1].rsuXmitMsgFwdingDestIpAddr_length;
      memcpy(data_out, ntcip_1218_mib.rsuXmitMsgFwdingTable[index-1].rsuXmitMsgFwdingDestIpAddr, RSU_DEST_IP_MAX);
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
  if((size < RSU_DEST_IP_MIN) || (RSU_DEST_IP_MAX < size)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);  
      return RSEMIB_BAD_DATA;
  }
  return size;
}
int32_t preserve_rsuXmitMsgFwdingDestIpAddr(int32_t index)
{
  if((index < RSU_XMIT_MSG_COUNT_MIN) || (RSU_XMIT_MSG_COUNT_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      memcpy( prior_ntcip_1218_mib.rsuXmitMsgFwdingTable[index-1].rsuXmitMsgFwdingDestIpAddr
            , ntcip_1218_mib.rsuXmitMsgFwdingTable[index-1].rsuXmitMsgFwdingDestIpAddr, RSU_DEST_IP_MAX);
      prior_ntcip_1218_mib.rsuXmitMsgFwdingTable[index-1].rsuXmitMsgFwdingDestIpAddr_length =
          ntcip_1218_mib.rsuXmitMsgFwdingTable[index-1].rsuXmitMsgFwdingDestIpAddr_length;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t action_rsuXmitMsgFwdingDestIpAddr(int32_t index, uint8_t * data_in, int32_t length)
{
  if((index < RSU_XMIT_MSG_COUNT_MIN) || (RSU_XMIT_MSG_COUNT_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  /* User can pass in empty string {} of length zero. */ 
  if((NULL == data_in) && (0 != length)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if((length < RSU_DEST_IP_MIN) || (RSU_DEST_IP_MAX < length)){
      return RSEMIB_BAD_INPUT;  
  }
  if(RSEMIB_OK == ntcip1218_lock()){  
      memset(tmp_ntcip_1218_mib.rsuXmitMsgFwdingTable[index-1].rsuXmitMsgFwdingDestIpAddr,0x0,RSU_DEST_IP_MAX);
      if(NULL != data_in){ 
          memcpy(tmp_ntcip_1218_mib.rsuXmitMsgFwdingTable[index-1].rsuXmitMsgFwdingDestIpAddr, data_in, length);
      }
      tmp_ntcip_1218_mib.rsuXmitMsgFwdingTable[index-1].rsuXmitMsgFwdingDestIpAddr_length = length;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t commit_rsuXmitMsgFwdingDestIpAddr(int32_t index)
{
  if((index < RSU_XMIT_MSG_COUNT_MIN) || (RSU_XMIT_MSG_COUNT_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  } 
  if(RSEMIB_OK == ntcip1218_lock()){
      memcpy( ntcip_1218_mib.rsuXmitMsgFwdingTable[index-1].rsuXmitMsgFwdingDestIpAddr
            , tmp_ntcip_1218_mib.rsuXmitMsgFwdingTable[index-1].rsuXmitMsgFwdingDestIpAddr, RSU_DEST_IP_MAX);
      ntcip_1218_mib.rsuXmitMsgFwdingTable[index-1].rsuXmitMsgFwdingDestIpAddr_length =
          tmp_ntcip_1218_mib.rsuXmitMsgFwdingTable[index-1].rsuXmitMsgFwdingDestIpAddr_length;
      memset(tmp_ntcip_1218_mib.rsuXmitMsgFwdingTable[index-1].rsuXmitMsgFwdingDestIpAddr, 0x0, RSU_DEST_IP_MAX);
      tmp_ntcip_1218_mib.rsuXmitMsgFwdingTable[index-1].rsuXmitMsgFwdingDestIpAddr_length = 0;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t undo_rsuXmitMsgFwdingDestIpAddr(int32_t index)
{
  if((index < RSU_XMIT_MSG_COUNT_MIN) || (RSU_XMIT_MSG_COUNT_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      memcpy( ntcip_1218_mib.rsuXmitMsgFwdingTable[index-1].rsuXmitMsgFwdingDestIpAddr
            , prior_ntcip_1218_mib.rsuXmitMsgFwdingTable[index-1].rsuXmitMsgFwdingDestIpAddr, RSU_DEST_IP_MAX);
      ntcip_1218_mib.rsuXmitMsgFwdingTable[index-1].rsuXmitMsgFwdingDestIpAddr_length =
          prior_ntcip_1218_mib.rsuXmitMsgFwdingTable[index-1].rsuXmitMsgFwdingDestIpAddr_length;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}

/* 5.21.2.4 */
int32_t get_rsuXmitMsgFwdingDestPort(int32_t index, int32_t * data_out)
{
  if((index < RSU_XMIT_MSG_COUNT_MIN) || (RSU_XMIT_MSG_COUNT_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      *data_out = ntcip_1218_mib.rsuXmitMsgFwdingTable[index-1].rsuXmitMsgFwdingDestPort;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
  if((*data_out < RSU_XMIT_MSG_PORT_MIN) || (RSU_XMIT_MSG_PORT_MAX < *data_out)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } 
  return RSEMIB_OK;
}
int32_t preserve_rsuXmitMsgFwdingDestPort(int32_t index)
{
  if((index < RSU_XMIT_MSG_COUNT_MIN) || (RSU_XMIT_MSG_COUNT_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      prior_ntcip_1218_mib.rsuXmitMsgFwdingTable[index-1].rsuXmitMsgFwdingDestPort = 
          ntcip_1218_mib.rsuXmitMsgFwdingTable[index-1].rsuXmitMsgFwdingDestPort;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t action_rsuXmitMsgFwdingDestPort(int32_t index, int32_t data_in)
{
  if((index < RSU_XMIT_MSG_COUNT_MIN) || (RSU_XMIT_MSG_COUNT_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if((data_in < RSU_XMIT_MSG_PORT_MIN) || (RSU_XMIT_MSG_PORT_MAX < data_in)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  }
  if(RSEMIB_OK == ntcip1218_lock()){  
      tmp_ntcip_1218_mib.rsuXmitMsgFwdingTable[index-1].rsuXmitMsgFwdingDestPort = data_in;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t commit_rsuXmitMsgFwdingDestPort(int32_t index)
{
  if((index < RSU_XMIT_MSG_COUNT_MIN) || (RSU_XMIT_MSG_COUNT_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuXmitMsgFwdingTable[index-1].rsuXmitMsgFwdingDestPort = 
          tmp_ntcip_1218_mib.rsuXmitMsgFwdingTable[index-1].rsuXmitMsgFwdingDestPort;
      tmp_ntcip_1218_mib.rsuXmitMsgFwdingTable[index-1].rsuXmitMsgFwdingDestPort = RSU_XMIT_MSG_PORT_MIN;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t undo_rsuXmitMsgFwdingDestPort(int32_t index)
{
  if((index < RSU_XMIT_MSG_COUNT_MIN) || (RSU_XMIT_MSG_COUNT_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuXmitMsgFwdingTable[index-1].rsuXmitMsgFwdingDestPort = 
          prior_ntcip_1218_mib.rsuXmitMsgFwdingTable[index-1].rsuXmitMsgFwdingDestPort;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}

/* 5.21.2.5 */
int32_t get_rsuXmitMsgFwdingProtocol(int32_t index, int32_t * data_out)
{
  if((index < RSU_XMIT_MSG_COUNT_MIN) || (RSU_XMIT_MSG_COUNT_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      *data_out = (int32_t)ntcip_1218_mib.rsuXmitMsgFwdingTable[index-1].rsuXmitMsgFwdingProtocol;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
  if((*data_out < protocol_other) || (protocol_udp < *data_out)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } 
  return RSEMIB_OK;
}
int32_t preserve_rsuXmitMsgFwdingProtocol(int32_t index)
{
  if((index < RSU_XMIT_MSG_COUNT_MIN) || (RSU_XMIT_MSG_COUNT_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      prior_ntcip_1218_mib.rsuXmitMsgFwdingTable[index-1].rsuXmitMsgFwdingProtocol = 
          ntcip_1218_mib.rsuXmitMsgFwdingTable[index-1].rsuXmitMsgFwdingProtocol;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t action_rsuXmitMsgFwdingProtocol(int32_t index, int32_t data_in)
{
  if((index < RSU_XMIT_MSG_COUNT_MIN) || (RSU_XMIT_MSG_COUNT_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if((data_in < protocol_other) || (protocol_udp < data_in)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  }
  if(RSEMIB_OK == ntcip1218_lock()){  
      tmp_ntcip_1218_mib.rsuXmitMsgFwdingTable[index-1].rsuXmitMsgFwdingProtocol = (protocol_e)data_in;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t commit_rsuXmitMsgFwdingProtocol(int32_t index)
{
  if((index < RSU_XMIT_MSG_COUNT_MIN) || (RSU_XMIT_MSG_COUNT_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuXmitMsgFwdingTable[index-1].rsuXmitMsgFwdingProtocol = 
          tmp_ntcip_1218_mib.rsuXmitMsgFwdingTable[index-1].rsuXmitMsgFwdingProtocol;
      tmp_ntcip_1218_mib.rsuXmitMsgFwdingTable[index-1].rsuXmitMsgFwdingProtocol = protocol_other;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t undo_rsuXmitMsgFwdingProtocol(int32_t index)
{
  if((index < RSU_XMIT_MSG_COUNT_MIN) || (RSU_XMIT_MSG_COUNT_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuXmitMsgFwdingTable[index-1].rsuXmitMsgFwdingProtocol = 
          prior_ntcip_1218_mib.rsuXmitMsgFwdingTable[index-1].rsuXmitMsgFwdingProtocol;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}

/* 5.21.2.6 */
int32_t get_rsuXmitMsgFwdingDeliveryStart(int32_t index, uint8_t * data_out)
{
  if((index < RSU_XMIT_MSG_COUNT_MIN) || (RSU_XMIT_MSG_COUNT_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      memcpy(data_out, ntcip_1218_mib.rsuXmitMsgFwdingTable[index-1].rsuXmitMsgFwdingDeliveryStart, MIB_DATEANDTIME_LENGTH);
      ntcip1218_unlock();
      return MIB_DATEANDTIME_LENGTH;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t preserve_rsuXmitMsgFwdingDeliveryStart(int32_t index)
{
  if((index < RSU_XMIT_MSG_COUNT_MIN) || (RSU_XMIT_MSG_COUNT_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      memcpy( prior_ntcip_1218_mib.rsuXmitMsgFwdingTable[index-1].rsuXmitMsgFwdingDeliveryStart
            , ntcip_1218_mib.rsuXmitMsgFwdingTable[index-1].rsuXmitMsgFwdingDeliveryStart, MIB_DATEANDTIME_LENGTH);
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t action_rsuXmitMsgFwdingDeliveryStart(int32_t index, uint8_t * data_in, int32_t length)
{
  if((index < RSU_XMIT_MSG_COUNT_MIN) || (RSU_XMIT_MSG_COUNT_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(NULL == data_in){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(MIB_DATEANDTIME_LENGTH != length){
      return RSEMIB_BAD_INPUT;  
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      memset(tmp_ntcip_1218_mib.rsuXmitMsgFwdingTable[index-1].rsuXmitMsgFwdingDeliveryStart, 0x0, MIB_DATEANDTIME_LENGTH);
      memcpy(tmp_ntcip_1218_mib.rsuXmitMsgFwdingTable[index-1].rsuXmitMsgFwdingDeliveryStart, data_in, length);
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t commit_rsuXmitMsgFwdingDeliveryStart(int32_t index)
{
  if((index < RSU_XMIT_MSG_COUNT_MIN) || (RSU_XMIT_MSG_COUNT_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      memcpy( ntcip_1218_mib.rsuXmitMsgFwdingTable[index-1].rsuXmitMsgFwdingDeliveryStart
            , tmp_ntcip_1218_mib.rsuXmitMsgFwdingTable[index-1].rsuXmitMsgFwdingDeliveryStart, MIB_DATEANDTIME_LENGTH);
      memset(tmp_ntcip_1218_mib.rsuXmitMsgFwdingTable[index-1].rsuXmitMsgFwdingDeliveryStart, 0x0, MIB_DATEANDTIME_LENGTH);
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t undo_rsuXmitMsgFwdingDeliveryStart(int32_t index)
{
  if((index < RSU_XMIT_MSG_COUNT_MIN) || (RSU_XMIT_MSG_COUNT_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      memcpy( ntcip_1218_mib.rsuXmitMsgFwdingTable[index-1].rsuXmitMsgFwdingDeliveryStart
            , prior_ntcip_1218_mib.rsuXmitMsgFwdingTable[index-1].rsuXmitMsgFwdingDeliveryStart, MIB_DATEANDTIME_LENGTH);
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}

/* 5.21.2.7 */
int32_t get_rsuXmitMsgFwdingDeliveryStop(int32_t index, uint8_t * data_out)
{
  if((index < RSU_XMIT_MSG_COUNT_MIN) || (RSU_XMIT_MSG_COUNT_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      memcpy(data_out, ntcip_1218_mib.rsuXmitMsgFwdingTable[index-1].rsuXmitMsgFwdingDeliveryStop, MIB_DATEANDTIME_LENGTH);
      ntcip1218_unlock();
      return MIB_DATEANDTIME_LENGTH;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t preserve_rsuXmitMsgFwdingDeliveryStop(int32_t index)
{
  if((index < RSU_XMIT_MSG_COUNT_MIN) || (RSU_XMIT_MSG_COUNT_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      memcpy( prior_ntcip_1218_mib.rsuXmitMsgFwdingTable[index-1].rsuXmitMsgFwdingDeliveryStop
            , ntcip_1218_mib.rsuXmitMsgFwdingTable[index-1].rsuXmitMsgFwdingDeliveryStop, MIB_DATEANDTIME_LENGTH);
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t action_rsuXmitMsgFwdingDeliveryStop(int32_t index, uint8_t * data_in, int32_t length)
{
  if((index < RSU_XMIT_MSG_COUNT_MIN) || (RSU_XMIT_MSG_COUNT_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(NULL == data_in){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(MIB_DATEANDTIME_LENGTH != length){
      return RSEMIB_BAD_INPUT;  
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      memset(tmp_ntcip_1218_mib.rsuXmitMsgFwdingTable[index-1].rsuXmitMsgFwdingDeliveryStop, 0x0, MIB_DATEANDTIME_LENGTH);
      memcpy(tmp_ntcip_1218_mib.rsuXmitMsgFwdingTable[index-1].rsuXmitMsgFwdingDeliveryStop, data_in, length);
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t commit_rsuXmitMsgFwdingDeliveryStop(int32_t index)
{
  if((index < RSU_XMIT_MSG_COUNT_MIN) || (RSU_XMIT_MSG_COUNT_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      memcpy( ntcip_1218_mib.rsuXmitMsgFwdingTable[index-1].rsuXmitMsgFwdingDeliveryStop
            , tmp_ntcip_1218_mib.rsuXmitMsgFwdingTable[index-1].rsuXmitMsgFwdingDeliveryStop, MIB_DATEANDTIME_LENGTH);
      memset(tmp_ntcip_1218_mib.rsuXmitMsgFwdingTable[index-1].rsuXmitMsgFwdingDeliveryStop, 0x0, MIB_DATEANDTIME_LENGTH);
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t undo_rsuXmitMsgFwdingDeliveryStop(int32_t index)
{
  if((index < RSU_XMIT_MSG_COUNT_MIN) || (RSU_XMIT_MSG_COUNT_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      memcpy( ntcip_1218_mib.rsuXmitMsgFwdingTable[index-1].rsuXmitMsgFwdingDeliveryStop
            , prior_ntcip_1218_mib.rsuXmitMsgFwdingTable[index-1].rsuXmitMsgFwdingDeliveryStop, MIB_DATEANDTIME_LENGTH);
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}

/* 5.21.2.8 */
int32_t get_rsuXmitMsgFwdingSecure(int32_t index, int32_t * data_out)
{
  if((index < RSU_XMIT_MSG_COUNT_MIN) || (RSU_XMIT_MSG_COUNT_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      *data_out = ntcip_1218_mib.rsuXmitMsgFwdingTable[index-1].rsuXmitMsgFwdingSecure;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
  if((*data_out < RSU_XMIT_MSG_SECURE_MIN) || (RSU_XMIT_MSG_SECURE_MAX < *data_out)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } 
  return RSEMIB_OK;
}
int32_t preserve_rsuXmitMsgFwdingSecure(int32_t index)
{
  if((index < RSU_XMIT_MSG_COUNT_MIN) || (RSU_XMIT_MSG_COUNT_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      prior_ntcip_1218_mib.rsuXmitMsgFwdingTable[index-1].rsuXmitMsgFwdingSecure = 
          ntcip_1218_mib.rsuXmitMsgFwdingTable[index-1].rsuXmitMsgFwdingSecure;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t action_rsuXmitMsgFwdingSecure(int32_t index, int32_t data_in)
{
  if((index < RSU_XMIT_MSG_COUNT_MIN) || (RSU_XMIT_MSG_COUNT_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if((data_in < RSU_XMIT_MSG_SECURE_MIN) || (RSU_XMIT_MSG_SECURE_MAX < data_in)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  }
  if(RSEMIB_OK == ntcip1218_lock()){  
      tmp_ntcip_1218_mib.rsuXmitMsgFwdingTable[index-1].rsuXmitMsgFwdingSecure = data_in;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t commit_rsuXmitMsgFwdingSecure(int32_t index)
{
  if((index < RSU_XMIT_MSG_COUNT_MIN) || (RSU_XMIT_MSG_COUNT_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuXmitMsgFwdingTable[index-1].rsuXmitMsgFwdingSecure = 
          tmp_ntcip_1218_mib.rsuXmitMsgFwdingTable[index-1].rsuXmitMsgFwdingSecure;
      tmp_ntcip_1218_mib.rsuXmitMsgFwdingTable[index-1].rsuXmitMsgFwdingSecure = RSU_XMIT_MSG_SECURE_MIN;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t undo_rsuXmitMsgFwdingSecure(int32_t index)
{
  if((index < RSU_XMIT_MSG_COUNT_MIN) || (RSU_XMIT_MSG_COUNT_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuXmitMsgFwdingTable[index-1].rsuXmitMsgFwdingSecure = 
          prior_ntcip_1218_mib.rsuXmitMsgFwdingTable[index-1].rsuXmitMsgFwdingSecure;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}

/* 5.21.2.9 */
int32_t get_rsuXmitMsgFwdingStatus(int32_t index, int32_t * data_out)
{
  if((index < RSU_XMIT_MSG_COUNT_MIN) || (RSU_XMIT_MSG_COUNT_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(NULL == data_out){
      set_ntcip_1218_error_states(RSEMIB_BAD_INPUT);
      return RSEMIB_BAD_INPUT;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      *data_out = ntcip_1218_mib.rsuXmitMsgFwdingTable[index-1].rsuXmitMsgFwdingStatus;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  } 
  if((ROW_STATUS_VALID_MIN <= *data_out ) && (*data_out <= ROW_STATUS_VALID_MAX)){
      return RSEMIB_OK; 
  } else {
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  } 
}
int32_t preserve_rsuXmitMsgFwdingStatus(int32_t index)
{
  if((index < RSU_XMIT_MSG_COUNT_MIN) || (RSU_XMIT_MSG_COUNT_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      set_default_row_rsuXmitMsgFwding(&tmp_ntcip_1218_mib.rsuXmitMsgFwdingTable[index-1],index-1);
      prior_ntcip_1218_mib.rsuXmitMsgFwdingTable[index-1].rsuXmitMsgFwdingStatus = 
          ntcip_1218_mib.rsuXmitMsgFwdingTable[index-1].rsuXmitMsgFwdingStatus;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
int32_t action_rsuXmitMsgFwdingStatus(int32_t index, int32_t data_in)
{
  if((index < RSU_XMIT_MSG_COUNT_MIN) || (RSU_XMIT_MSG_COUNT_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if((data_in < ROW_STATUS_VALID_MIN) || (ROW_STATUS_VALID_MAX < data_in)){
      set_ntcip_1218_error_states(RSEMIB_BAD_DATA);
      return RSEMIB_BAD_DATA;
  }
  if(RSEMIB_OK == ntcip1218_lock()){  
      tmp_ntcip_1218_mib.rsuXmitMsgFwdingTable[index-1].rsuXmitMsgFwdingStatus = data_in;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}
//TODO: Commit to disk.
int32_t commit_rsuXmitMsgFwdingStatus(int32_t index)
{
  int32_t ret = RSEMIB_OK;
  if((index < RSU_XMIT_MSG_COUNT_MIN) || (RSU_XMIT_MSG_COUNT_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      ret = RSEMIB_BAD_INDEX;
  }
  if((RSEMIB_OK == ret) && (RSEMIB_OK == ntcip1218_lock())){
      DEBUGMSGTL((MY_NAME_EXTRA, "commit_rsuXmitMsgFwdingStatus: index=%d status=%d.\n", index,tmp_ntcip_1218_mib.rsuXmitMsgFwdingTable[index-1].rsuXmitMsgFwdingStatus));
      switch(tmp_ntcip_1218_mib.rsuXmitMsgFwdingTable[index-1].rsuXmitMsgFwdingStatus){
          case SNMP_ROW_CREATEANDGO:
              if(SNMP_ROW_NONEXISTENT == ntcip_1218_mib.rsuXmitMsgFwdingTable[index-1].rsuXmitMsgFwdingStatus) {
                  set_default_row_rsuXmitMsgFwding(&ntcip_1218_mib.rsuXmitMsgFwdingTable[index-1],index-1);
                  ntcip_1218_mib.rsuXmitMsgFwdingTable[index-1].rsuXmitMsgFwdingStatus = SNMP_ROW_ACTIVE;
#if 0 // TODO:
                  if(RSEMIB_OK != commit_rsuXmitMsgFwding_to_disk(&ntcip_1218_mib.rsuXmitMsgFwdingTable[index-1])){
                      DEBUGMSGTL((MY_NAME, "build_ReceivedMsg_message: FAIL: index=%d dir=[%s].\n", index, ntcip_1218_mib.rsuXmitMsgFwdingTable[index-1].filePathName));
                      ret = RSEMIB_BAD_DATA;
                  } else
#endif
                  {
                      /* Rebuild helper table before ack. */
                      if (RSEMIB_OK != (ret = rebuild_rsuXmitMsgFwding_live(&ntcip_1218_mib.rsuXmitMsgFwdingTable[0]))){
                          ret = RSEMIB_BAD_DATA;
                      }
                  }
              } else {
                  /* If it exists already then do nothing. Dont send error back. */
                  //ret = RSEMIB_BAD_MODE;
              }
              break;
          case SNMP_ROW_CREATEANDWAIT:
              if(SNMP_ROW_NONEXISTENT == ntcip_1218_mib.rsuXmitMsgFwdingTable[index-1].rsuXmitMsgFwdingStatus) {
                  set_default_row_rsuXmitMsgFwding(&ntcip_1218_mib.rsuXmitMsgFwdingTable[index-1],index-1);
                  ntcip_1218_mib.rsuXmitMsgFwdingTable[index-1].rsuXmitMsgFwdingStatus = SNMP_ROW_NOTINSERVICE;
#if 0 // TODO:
                  if(RSEMIB_OK != commit_rsuXmitMsgFwding_to_disk(&ntcip_1218_mib.rsuXmitMsgFwdingTable[index-1])){
                      DEBUGMSGTL((MY_NAME, "build_IFM_message: FAIL: index=%d dir=[%s].\n", index, ntcip_1218_mib.rsuXmitMsgFwdingTable[index-1].filePathName));
                      ret = RSEMIB_BAD_DATA;
                  } else
#endif
                  {
                      /* Rebuild helper table before ack. */
                      if (RSEMIB_OK != (ret = rebuild_rsuXmitMsgFwding_live(&ntcip_1218_mib.rsuXmitMsgFwdingTable[0]))){
                          ret = RSEMIB_BAD_DATA;
                      }
                  }
              } else {
                  /* If it exists already then do nothing. Dont send error back. */
                  //ret = RSEMIB_BAD_MODE;
              }
              break;
          case SNMP_ROW_DESTROY: /* User deletes row from table. */
              if(SNMP_ROW_NONEXISTENT == ntcip_1218_mib.rsuXmitMsgFwdingTable[index-1].rsuXmitMsgFwdingStatus){
                  ret = RSEMIB_OK; /* If you try to DESTROY non-exixtent row that's an easy one. */
              } else {
#if 0 // TODO:
                  /* rm from RSU. */
                  memset(command_buffer,'\0',sizeof(command_buffer));
                  snprintf(command_buffer, sizeof(command_buffer), "/bin/rm -f %s", ntcip_1218_mib.rsuXmitMsgFwdingTable[index-1].filePathName);
                  DEBUGMSGTL((MY_NAME, "DESTROY: index=%d cmd=[%s].\n", index, command_buffer));
                  if(0 != system(command_buffer)){
                      ret = RSEMIB_BAD_DATA;
                  }
#endif
                  /* Careful to preserve index within table rows. Only call proper routine to clear. */
                  set_default_row_rsuXmitMsgFwding(&ntcip_1218_mib.rsuXmitMsgFwdingTable[index-1],index-1);
                  set_default_row_rsuXmitMsgFwding(&tmp_ntcip_1218_mib.rsuXmitMsgFwdingTable[index-1],index-1);
                  set_default_row_rsuXmitMsgFwding(&prior_ntcip_1218_mib.rsuXmitMsgFwdingTable[index-1],index-1);

                  /* Rebuild helper table before ack. */
                  if (RSEMIB_OK != (ret = rebuild_rsuXmitMsgFwding_live(&ntcip_1218_mib.rsuXmitMsgFwdingTable[0]))){
                      ret = RSEMIB_BAD_DATA;
                  }
              }
              break;
          case SNMP_ROW_ACTIVE:
              ntcip_1218_mib.rsuXmitMsgFwdingTable[index-1].rsuXmitMsgFwdingStatus = SNMP_ROW_ACTIVE;
              break;
          case SNMP_ROW_NOTINSERVICE:
              ntcip_1218_mib.rsuXmitMsgFwdingTable[index-1].rsuXmitMsgFwdingStatus = SNMP_ROW_NOTINSERVICE;
              break;
          case SNMP_ROW_NONEXISTENT:
          case SNMP_ROW_NOTREADY:
          default:
              ret = RSEMIB_BAD_MODE;
              break;
      }
      /* Clear tmp. */
      tmp_ntcip_1218_mib.rsuXmitMsgFwdingTable[index-1].rsuXmitMsgFwdingStatus = ROW_STATUS_VALID_DEFAULT;
      ntcip1218_unlock();
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      ret = RSEMIB_LOCK_FAIL;
  }
  return ret;
}
int32_t undo_rsuXmitMsgFwdingStatus(int32_t index)
{
  if((index < RSU_XMIT_MSG_COUNT_MIN) || (RSU_XMIT_MSG_COUNT_MAX < index)){
      set_ntcip_1218_error_states(RSEMIB_BAD_INDEX);
      return RSEMIB_BAD_INDEX;
  }
  if(RSEMIB_OK == ntcip1218_lock()){
      ntcip_1218_mib.rsuXmitMsgFwdingTable[index-1].rsuXmitMsgFwdingStatus = 
          prior_ntcip_1218_mib.rsuXmitMsgFwdingTable[index-1].rsuXmitMsgFwdingStatus;
      ntcip1218_unlock();
      return RSEMIB_OK;
  } else {
      set_ntcip_1218_error_states(RSEMIB_LOCK_FAIL);
      return RSEMIB_LOCK_FAIL;
  }
}

