/**************************************************************************
 *                                                                        *
 *     File Name:  amhValidator.c  (provides amh validation standalone)   *
 *     Author:                                                            *
 *         DENSO International America, Inc.                              *
 *         North America Research Laboratory, California Office           *
 *         2251 Rutherford Rd Ste 100                                     *
 *         Carlsbad, CA 92008                                             *
 *                                                                        *
 **************************************************************************/
/**************************************************************************
    The purpose of this function is to provide validation for active
    message files that are not received in a way for I2V to validate i.e.
    uploaded files.
 **************************************************************************/ 

/* 
 * There's a bunch of additional checks amh.c does above and beyond this at runtime.
 * This ensures the message is well formed.
 * The message can be well formed and dissappear instantly once AMH starts for legit reasons.
 *   i.e. too many messages, security on or off, start stop times not right, MAP/TIM not enabled in i2v.conf...
 * Will return bitmask error state value defined in AMH.
 */
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include "i2v_util.h"
#include "amh_types.h"
#include "amh.h"

/************************************************************************
 * File format is defined in RSU functional spec 4.1.
 * This is not DENSO conf file. File defintion and structure are rigid.
 ************************************************************************/
static uint32_t amhValidateFile(FILE *f) 
{
  uint32_t ret = AMH_AOK;
  float32_t dummyVersion = 0.0f;     
  int8_t msgInterval = 0;
  bool_t isContinuous = WFALSE, hasSignature = WFALSE, hasEncryption = WFALSE;
  rawImmediateType msg;    
  /* Not checking start stop times. */
  char_t start_date_buffer[AMH_FILENAME_SIZE];
  char_t stop_date_buffer[AMH_FILENAME_SIZE];

  /* DO NOT mix custom functions with "regular". */
  cfgItemsTypeT recorddata[] = {
        {"Version",       (void *)amvUpdateFloatValue,      &dummyVersion,     NULL, 0},
        {"Type",          NULL,                             &msg.type,         (void *)amvUpdateType, 0},
        {"PSID",          (void *)i2vUtilUpdateUint32Value, &msg.psid,         NULL, 0},
        {"Priority",      (void *)i2vUtilUpdateUint8Value,  &msg.priority,     NULL, 0},
        {"TxMode",        NULL,                             &isContinuous,     (void *)amvUpdateTxMode, 0},
        {"TxChannel",     NULL,                             &msg.channel,      (void *)amvUpdateTxChannel, 0},
        {"TxInterval",    (void *)i2vUtilUpdateUint8Value,  &msgInterval,      NULL, 0},
        {"DeliveryStart", (void *)i2vUtilUpdateStrValue,    start_date_buffer, NULL, 0},
        {"DeliveryStop",  (void *)i2vUtilUpdateStrValue,    stop_date_buffer,  NULL, 0},
        {"Signature",     NULL,                             &hasSignature,     (void *)amvUpdateBool, 0},
        {"Encryption",    NULL,                             &hasEncryption,    (void *)amvUpdateBool, 0},
        {"Payload",       NULL,                             &msg.data,         (void *)amvUpdatePayload, 0},
  };

  memset(&msg, 0, sizeof(msg));
  memset(start_date_buffer, 0, AMH_FILENAME_SIZE);
  memset(stop_date_buffer, 0, AMH_FILENAME_SIZE);

  /* Will return bitmask defined in AMH.H */
  if (AMH_AOK == (ret = amvParseSARFile(f, recorddata, NUMITEMS(recorddata)))) {
      if (0 == (J2735_DSRC_MSG_TYPE(msg.type))){
          ret = AMHL_SAR_BAD_MSG_TYPE;
      } else {
          ret = AMH_AOK;
      }
  }
  return ret;
}
#define MAX_PARSE_ERROR_STRING_SIZE 320  /* 32 errors, 10 char each max. */
static char_t parsing_error_string[MAX_PARSE_ERROR_STRING_SIZE];

static char_t * set_parsing_error_string(int32_t error_state)
{
  memset(parsing_error_string,0,MAX_PARSE_ERROR_STRING_SIZE);

  if(AMHL_SAR_NO_TYPE & error_state){
      strncat(parsing_error_string," type",sizeof(parsing_error_string));   
  }
  if(AMHL_SAR_NO_PSID & error_state){      
      strncat(parsing_error_string,": psid",sizeof(parsing_error_string));   
  }
  if(AMHL_SAR_NO_PRIORITY & error_state){      
      strncat(parsing_error_string,": priority",sizeof(parsing_error_string));   
  }
  if(AMHL_SAR_NO_TXMODE & error_state){      
      strncat(parsing_error_string,": txmode",sizeof(parsing_error_string));   
  }
  if(AMHL_SAR_NO_TXCHANNEL & error_state){      
      strncat(parsing_error_string,": txchan",sizeof(parsing_error_string));   
  }
  if(AMHL_SAR_NO_TXINTERVAL & error_state){      
      strncat(parsing_error_string,": txint",sizeof(parsing_error_string));   
  }
  if(AMHL_SAR_NO_DELIVERYSTART & error_state){      
      strncat(parsing_error_string,": start",sizeof(parsing_error_string));   
  }
  if(AMHL_SAR_NO_DELIVERYSTOP & error_state){      
      strncat(parsing_error_string,": stop",sizeof(parsing_error_string));   
  }
  if(AMHL_SAR_NO_SIGNATURE & error_state){      
      strncat(parsing_error_string,": sign",sizeof(parsing_error_string));   
  }
  if(AMHL_SAR_NO_PAYLOAD & error_state){      
      strncat(parsing_error_string,": payload",sizeof(parsing_error_string));   
  }
  if(AMHL_SAR_NO_VERSION & error_state){      
      strncat(parsing_error_string,": version",sizeof(parsing_error_string));   
  }
  if(AMHL_SAR_NULL_INPUT & error_state){      
      strncat(parsing_error_string,": internal",sizeof(parsing_error_string));   
  }
  if(AMHL_SAR_BAD_MSG_TYPE & error_state){      
      strncat(parsing_error_string,": msgtype",sizeof(parsing_error_string));   
  }
  if(AMV_NULL_INPUT & error_state){      
      strncat(parsing_error_string,": argc",sizeof(parsing_error_string));   
  }
  if(AMV_SAR_DOS2UNIX_FAIL & error_state){      
      strncat(parsing_error_string,": dos",sizeof(parsing_error_string));   
  }
  if(AMV_SAR_FILE_NOT_ASCII & error_state){      
      strncat(parsing_error_string,": ascii",sizeof(parsing_error_string));   
  }
  if(AMV_SAR_FILE_OPEN_ERROR & error_state){      
      strncat(parsing_error_string,": fopen1",sizeof(parsing_error_string));   
  }
  if(AMV_SAR_FILE_OPEN_ERROR2 & error_state){      
      strncat(parsing_error_string,": fopen2",sizeof(parsing_error_string));   
  }
  if(AMV_SAR_FILENAME_RSEMIB_CONFLICT & error_state){      
      strncat(parsing_error_string,": fname",sizeof(parsing_error_string));   
  }
  if(AMH_SAR_FILE_TOO_BIG & error_state){      
      strncat(parsing_error_string,": fsize",sizeof(parsing_error_string));   
  }
  return parsing_error_string;
}

/* Linux only allows 8 bits unsigned returned. */ 
#if defined(MY_UNIT_TEST)
int32_t amhV_main(int32_t argc, char_t *argv[])
#else
int32_t main(int32_t argc, char_t *argv[])
#endif
{
    FILE *f = NULL;
    #if defined(ENABLE_RSEMIB_RESTRICTION)    
    int32_t i = 0;
    #endif
    uint32_t ret = AMH_AOK;
    char_t cmd[400] = {0};   /* a gross overestimate */

    if (argc < 2) {
        ret = AMV_NULL_INPUT;
    } else {
        #if defined(ENABLE_RSEMIB_RESTRICTION)    /* Legacy prevents us today from enabling this logic. */
        for(i=0; i < MAX_AMH_SAR_MESSAGES; i++) { /* RSU-MIB file name conflict check. */
            snprintf(cmd,sizeof(cmd),"/%d.txt",i);
            if(NULL != (strstr(argv[1],cmd))) { 
                ret = AMV_SAR_FILENAME_RSEMIB_CONFLICT;
            }
        }
        #endif
        if((AMH_AOK == ret) && (NULL == (f = fopen(argv[1], "r")))){ /* open file */
            ret = AMV_SAR_FILE_OPEN_ERROR;
        } else {
            fclose(f); /* reset for next ops */
            f = NULL;
        }
        if(AMH_AOK == ret){
            snprintf(cmd, sizeof(cmd), "file %s | grep ASCII >>/dev/null", argv[1]); /* confirm ascii and do dos2unix on file */
            if (system(cmd)) {
                ret = AMV_SAR_FILE_NOT_ASCII;
            }
        }
        if(AMH_AOK == ret){
           snprintf(cmd, sizeof(cmd), "dos2unix %s >> /dev/null 2>&1", argv[1]); /* just in case do dos2unix */
           if (system(cmd)){
               ret = AMV_SAR_DOS2UNIX_FAIL; /* problem with dos2unix?? - sounds like a corrupt file */
           }
        }
        if((AMH_AOK == ret) &&  (NULL == (f = fopen(argv[1], "r")))){ 
            ret = AMV_SAR_FILE_OPEN_ERROR2; /* reopen file */
        }
        if(AMH_AOK == ret) {  /* finished inital validation of file. Now verify data contents of AMH message. */
            ret = amhValidateFile(f);
        }
        if(NULL != f) {
            fclose(f);
        }
    }
    if(AMH_AOK == ret){ /* only place we output any debug or otherwise. Only one message to turn off if you care. */
        printf("OK\n"); /* OK indicates success. All else is a fail */
    } else {
        printf("[0x%x|%s]\n",ret,set_parsing_error_string(ret)); /* bitmap of error states along with decoded string */
        ret = 1;
    }
    return (uint8_t)ret;
}

