/**************************************************************************
 *                                                                        *
 *     File Name:  amhlib.c  (AMH library)                                *
 *     Author:                                                            *
 *         DENSO International America, Inc.                              *
 *         North America Research Laboratory, California Office           *
 *         Carlsbad, CA 92008                                             *
 *                                                                        *
 **************************************************************************/
/**************************************************************************
 amhValidator library: Not thread safe.
 **************************************************************************/
#include <sys/types.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include "wsu_util.h"
#include "wsu_shm.h"
#include "i2v_util.h"
#include "amh_types.h"
#include "amh.h"

/* 
 * NOTE: WEB GUI calls these functions. 
 * Therefore, do not output DEBUG to STDOUT,STDERR because it will upset what WEB GUI expects.
 */

/* mandatory logging defines */
#define MY_ERR_LEVEL   LEVEL_INFO    /* from i2v_util.h */
#define MY_NAME        "amhlib"

void amvUpdateMinute(char_t GCC_UNUSED_VAR *tag, char_t GCC_UNUSED_VAR *parsedItem, char_t *value, void *configItem, uint32_t *status)
{
    /* Min strlen is 17 */
    /* format: mm/dd/yyyy, hh:mm */
    uint32_t *result = (uint32_t *)configItem;
    bool_t valid = WFALSE;
    i2vTimeT clock;
    struct tm tval;
    char_t *tok = NULL;
    char_t *saveptr = NULL;

    if ((NULL == value) || (NULL == configItem)) {
        return;
    }

    memset(&tval, 0, sizeof(tval));

    tok = strtok_r(value, "/",&saveptr);

    if ((NULL != tok) && (strcmp(tok, " "))) {    /* should be impossible to get a space but just in case */
        tval.tm_mon = (int)(strtoul(tok, NULL, 10) - 1);
        tok = strtok_r(NULL, "/",&saveptr);
    } else {
        /* blank means current time */
        i2vUtilGetUTCTime(&clock);
        *result = CONVERT_TO_UTC(clock.year, clock.yday, clock.hour, clock.min, clock.sec);
        valid = WTRUE;
        goto setresult;
    }
    if (NULL != tok) {
        tval.tm_mday = (int)strtoul(tok, NULL, 10);
        tok = strtok_r(NULL, ",",&saveptr);

    } else {
        goto setresult;
    }
    if (NULL != tok) {
        tval.tm_year = (int)(strtoul(tok, NULL, 10) - 1900);
        tok = strtok_r(NULL, ":",&saveptr);
    } else {
        goto setresult;
    }
    if (NULL != tok) {
        tval.tm_hour = (int)strtoul(tok, NULL, 10);
        tok = strtok_r(NULL, ":",&saveptr);
    } else {
        goto setresult;
    }
    if (NULL != tok) {
        tval.tm_min = (int)strtoul(tok, NULL, 10);
        valid = WTRUE;
    } else {
        goto setresult;
    }

    mktime(&tval);
    *result = CONVERT_TO_UTC((tval.tm_year + 1900), tval.tm_yday, tval.tm_hour, tval.tm_min, tval.tm_sec);

setresult:
    if (NULL != status) *status = (valid) ? I2V_RETURN_OK : I2V_RETURN_FAIL;
}
void amvUpdateType(char_t GCC_UNUSED_VAR *tag, char_t GCC_UNUSED_VAR *parsedItem, char_t *value, void *configItem, uint32_t *status)
{
    amhBitmaskType *result = (amhBitmaskType *)configItem;
 
    if ((NULL == value) || (NULL == configItem) || (NULL == status)) {
        return;
    }

    *status = I2V_RETURN_FAIL;
    *result = AMH_MSG_UNKNOWN_MASK;

    if (!strcmp(value, "SPAT")) {
        *status = I2V_RETURN_OK;
        *result = AMH_MSG_SPAT_MASK;
        return;
    }
    if (!strcmp(value, "MAP")) {
        *status = I2V_RETURN_OK;
        *result = AMH_MSG_MAP_MASK;
        return;
    }
    if (!strcmp(value, "TIM")) {
        *status = I2V_RETURN_OK;
        *result = AMH_MSG_TIM_MASK;
        return;
    }
    if (!strcmp(value, "RTCM")) {
        *status = I2V_RETURN_OK;
        *result = AMH_MSG_RTCM_MASK;
        return;
    }
    if (!strcmp(value, "SPAT16")) {
        *status = I2V_RETURN_OK;
        *result = AMH_MSG_SPAT_MASK;
        return;
    }
    if (!strcmp(value, "MAP16")) {
        *status = I2V_RETURN_OK;
        *result = AMH_MSG_MAP_MASK;
        return;
    }
    if (!strcmp(value, "TIM16")) {
        *status = I2V_RETURN_OK;
        *result = AMH_MSG_TIM_MASK;
        return;
    }
    if (!strcmp(value, "RTCM16")) {
        *status = I2V_RETURN_OK;
        *result = AMH_MSG_RTCM_MASK;
        return;
    }
    if (!strcmp(value, "ICA")) {
        *status = I2V_RETURN_OK;
        *result = AMH_MSG_ICA_MASK;
        return;
    }
    if (!strcmp(value, "PDM")) {
        *status = I2V_RETURN_OK;
        *result = AMH_MSG_PDM_MASK;
        return;
    }
    if (!strcmp(value, "RSA")) {
        *status = I2V_RETURN_OK;
        *result = AMH_MSG_RSA_MASK;
        return;
    }
    if (!strcmp(value, "SSM")) {
        *status = I2V_RETURN_OK;
        *result = AMH_MSG_SSM_MASK;
        return;
    }
    if (!strcmp(value, "CSR")) {
        *status = I2V_RETURN_OK;
        *result = AMH_MSG_CSR_MASK;
        return;
    }
    if (!strcmp(value, "EVA")) {
        *status = I2V_RETURN_OK;
        *result = AMH_MSG_EVA_MASK;
        return;
    }
    if (!strcmp(value, "NMEA")) {
        *status = I2V_RETURN_OK;
        *result = AMH_MSG_NMEA_MASK;
        return;
    }
    if (!strcmp(value, "PSM")) {
        *status = I2V_RETURN_OK;
        *result = AMH_MSG_PSM_MASK;
        return;
    }
    if (!strcmp(value, "PVD")) {
        *status = I2V_RETURN_OK;
        *result = AMH_MSG_PVD_MASK;
        return;
    }
    if (!strcmp(value, "SRM")) {
        *status = I2V_RETURN_OK;
        *result = AMH_MSG_SRM_MASK;
        return;
    }
    if (!strcmp(value, "BSM")) {
        *status = I2V_RETURN_OK;
        *result = AMH_MSG_BSM_MASK;
        return;
    }
    if (!strcmp(value, "TEST00")) {
        *status = I2V_RETURN_OK;
        *result = AMH_MSG_TEST00_MASK;
        return;
    }
    if (!strcmp(value, "TEST01")) {
        *status = I2V_RETURN_OK;
        *result = AMH_MSG_TEST01_MASK;
        return;
    }
    if (!strcmp(value, "TEST02")) {
        *status = I2V_RETURN_OK;
        *result = AMH_MSG_TEST02_MASK;
        return;
    }
    if (!strcmp(value, "TEST03")) {
        *status = I2V_RETURN_OK;
        *result = AMH_MSG_TEST03_MASK;
        return;
    }
    if (!strcmp(value, "TEST04")) {
        *status = I2V_RETURN_OK;
        *result = AMH_MSG_TEST04_MASK;
        return;
    }
    if (!strcmp(value, "TEST05")) {
        *status = I2V_RETURN_OK;
        *result = AMH_MSG_TEST05_MASK;
        return;
    }
    if (!strcmp(value, "TEST06")) {
        *status = I2V_RETURN_OK;
        *result = AMH_MSG_TEST06_MASK;
        return;
    }
    if (!strcmp(value, "TEST07")) {
        *status = I2V_RETURN_OK;
        *result = AMH_MSG_TEST07_MASK;
        return;
    }
    if (!strcmp(value, "TEST08")) {
        *status = I2V_RETURN_OK;
        *result = AMH_MSG_TEST08_MASK;
        return;
    }
    if (!strcmp(value, "TEST09")) {
        *status = I2V_RETURN_OK;
        *result = AMH_MSG_TEST09_MASK;
        return;
    }
    if (!strcmp(value, "TEST10")) {
        *status = I2V_RETURN_OK;
        *result = AMH_MSG_TEST10_MASK;
        return;
    }
    if (!strcmp(value, "TEST11")) {
        *status = I2V_RETURN_OK;
        *result = AMH_MSG_TEST11_MASK;
        return;
    }
    if (!strcmp(value, "TEST12")) {
        *status = I2V_RETURN_OK;
        *result = AMH_MSG_TEST12_MASK;
        return;
    }
    if (!strcmp(value, "TEST13")) {
        *status = I2V_RETURN_OK;
        *result = AMH_MSG_TEST13_MASK;
        return;
    }
    if (!strcmp(value, "TEST14")) {
        *status = I2V_RETURN_OK;
        *result = AMH_MSG_TEST14_MASK;
        return;
    }
    if (!strcmp(value, "TEST15")) {
        *status = I2V_RETURN_OK;
        *result = AMH_MSG_TEST15_MASK;
        return;
    }

    return; /* if we are here, we are returning I2V_RETURN_FAIL */
}

void amvUpdateTxMode(char_t GCC_UNUSED_VAR *tag, char_t GCC_UNUSED_VAR *parsedItem, char_t *value, void *configItem, uint32_t *status)
{
    bool_t *result = (bool_t *)configItem;
    bool_t valid = WFALSE;
 
    if ((NULL == value) || (NULL == configItem)) {
        return;
    }

    if (!strcmp(value, "CONT")) {
        *result = WTRUE;
        valid = WTRUE;
    } else if (!strcmp(value, "ALT")) {
        *result = WFALSE;
        valid = WTRUE;
    }

    if (NULL != status) *status = (valid) ? I2V_RETURN_OK : I2V_RETURN_FAIL;
}

/* 20140903 - adding in support for any numeric channel number within range -
   this was a request from the june plugfest */
void amvUpdateTxChannel(char_t GCC_UNUSED_VAR *tag, char_t GCC_UNUSED_VAR *parsedItem, char_t *value, void *configItem, uint32_t *status)
{
    uint8_t *result = (uint8_t *)configItem;
    bool_t valid = WFALSE;
 
    if ((NULL == value) || (NULL == configItem)) {
        return;
    }

    if (!strcmp(value, "CCH")) {
        *result = I2V_CONTROL_CHANNEL;   /* control channel */
        valid = WTRUE;
    } else if (!strcmp(value, "SCH")) {
        *result = AMH_NEED_SCH;   /* indication to get service channel */
        valid = WTRUE;
    } else if ((*result = (uint8_t)strtoul(value, NULL, 10))) {
        /* above will set value to 0 if not a number and this block not entered */
        if ((*result >= I2V_MIN_SVC_CHANNEL) && (*result <= I2V_MAX_SVC_CHANNEL)) {
            valid = WTRUE;
        } /* us band specific - may need to consider other country frequencies (i.e. macro for vals) */
    }

    if (NULL != status) *status = (valid) ? I2V_RETURN_OK : I2V_RETURN_FAIL;
}

void amvUpdateBool(char_t GCC_UNUSED_VAR *tag, char_t GCC_UNUSED_VAR *parsedItem, char_t *value, void *configItem, uint32_t *status)
{
    bool_t *result = (bool_t *)configItem;
    bool_t valid = WFALSE;
 
    if ((NULL == value) || (NULL == configItem)) {
        return;
    }

    if (!strcmp(value, "True")) {
        *result = WTRUE;
        valid = WTRUE;
    } else if (!strcmp(value, "False")) {
        *result = WFALSE;
        valid = WTRUE;
    }

    if (NULL != status) *status = (valid) ? I2V_RETURN_OK : I2V_RETURN_FAIL;
}
void amvUpdateFloatValue(void *configItem, char_t *val, char_t *min, char_t *max,
                 char_t  **trange, uint32_t *status, i2vLogWrapper GCC_UNUSED_VAR *logger)
{
    float32_t  *cfg = (float32_t  *)configItem;
    float32_t   value = strtof(val, NULL);
    float32_t   minval = 0.0f, maxval = 0.0f;
    int32_t   i = 0;
    bool_t   valueValid = WFALSE;


    if (strcmp(min, I2V_CFG_UNDEFINED) != 0) minval = strtof(min, NULL);
    if (strcmp(max, I2V_CFG_UNDEFINED) != 0) maxval = strtof(max, NULL);
    if ((value >= minval) && (value <= maxval)) {
        *cfg = value;
#if defined(EXTRA_DEBUG)
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"%f {%f, %f} (float32_t)\n", value, minval, maxval);
#endif
    } else if( (strcmp(min, I2V_CFG_UNDEFINED) == 0) && (strcmp(max, I2V_CFG_UNDEFINED) == 0) ) {
          /* No Min/Max limits for this value. */
          *cfg = value;
    } else {
        if (value < minval) {
            *cfg = minval;
#if defined(EXTRA_DEBUG)
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"ERROR, %s(): MIN LIMIT, value (%f) < minval (%f). value set to minval\n", __FUNCTION__, value, minval);
#endif
            *status = I2V_RETURN_MIN_LIMIT_FAIL;
        } else if (value > maxval) {
            *cfg = maxval;
#if defined(EXTRA_DEBUG)
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"ERROR, %s(): MAX LIMIT, value (%f) > maxval (%f). value set to maxval\n", __FUNCTION__, value, maxval);
#endif
            *status = I2V_RETURN_MAX_LIMIT_FAIL;
        }
    }
    if( trange == NULL ) return;

    /* Ensure that the specified value is equal to a table discrete 
     * and that min/max agree with table values. 
     */
    while( *(trange + i) != NULL ) {
        if( !valueValid && (value  == strtof(*(trange+i), NULL)) ) { 
            valueValid = WTRUE;
            break;
        }
          i++;
    }

    /* Current value is not contained in discrete value range */
    if( !valueValid ) {
#if defined(EXTRA_DEBUG)
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"ERROR, %s(): Current value= %f is NOT in discrete range\n", __FUNCTION__, value);
#endif
          *status = I2V_RETURN_RANGE_LIMIT_FAIL;  // Current value NOT in discrete range
    }
}

void amvUpdatePayload( char_t GCC_UNUSED_VAR * tag
                            , char_t GCC_UNUSED_VAR * parsedItem
                            , char_t                * value  /*input*/
                            , void                  * configItem /*output*/
                            , uint32_t              * status)
{
    payloadType * result  = (payloadType *)configItem;
    bool_t        valid   = WFALSE;
    uint16_t      i       = 0;
    uint16_t      valsize = 0;
    uint8_t       dummy   = 0;

    if ((NULL == value) || (NULL == result) || (NULL == configItem)) {
        #if defined (EXTRA_DEBUG)
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"amhUpdatePayload:NULL Inputs::Rejected.\n");
        #endif
        valid = WFALSE;
    }  else {

        valid = WTRUE;
        valsize = strlen(value); 
      
        #if defined (EXTRA_EXTRA_EXTRA_DEBUG)
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"amhUpdatePayload:[%lu][%s][%d]\n",valsize,value,result->count);
        #endif

        /* Too small ? */
        if(valsize < 6) {
            #if defined (EXTRA_DEBUG)
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"amhUpdatePayload:number of nibbles(%lu) < 6. Rejected.\n", valsize);
            #endif
            valid = WFALSE;
        }

        /* Too big? */
        if(RSU_MIB_MAX_PAYLOAD_OCTET_STRING < valsize) {
            #if defined (EXTRA_DEBUG)
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"amhUpdatePayload: RSU_MIB_MAX_PAYLOAD_OCTET_STRING < number of nibbles(%d). Rejected.\n", valsize);
            #endif
            valid = WFALSE;
        }

        /* Too odd? */
        if( 1 == (valsize % 2)) {

            #if defined (EXTRA_DEBUG)
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"amhUpdatePayload:ODD number of nibbles::size=%d Rejected.\n",valsize);
            #endif
            valid = WFALSE;
        }

        /* Just right ! */
        if(WTRUE == valid) {

            #if defined (EXTRA_EXTRA_DEBUG)
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"amhUpdatePayload:[%d][%s]\n",valsize,value);
            #endif
 
            for(i=0, result->count=0;   (WTRUE == valid) 
                                     && (result->count < MAX_WSM_DATA) 
                                     && (i < valsize ); i++) {

                dummy = i2v_ascii_2_hex_nibble(value[i]);

                if(0xff == dummy) {

                    #if defined (EXTRA_DEBUG)
                    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"amhUpdatePayload:Not HEX values i(%d)=[%c]. Rejected.\n",i,value[i]);
                    #endif

                    if (NULL != status) *status = I2V_RETURN_FAIL;
                    valid = WFALSE;
                    break; /* Error out */
                  
                } else {

                    if(0 == (i % 2)) {
                        result->payload[result->count] = (0xf) & dummy;
                    } else {
                        result->payload[result->count]  =  result->payload[result->count] << 4; 
                        result->payload[result->count] |=  (0xf) & dummy;
                        result->count++;
                    }
                }
            }

            #if defined (EXTRA_EXTRA_DEBUG)
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"amhUpdatePayload2:[\n");
            for(i=0; (i < MAX_WSM_DATA) && (i < result->count) ; i++) {
                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"%x,\n",result->payload[i]);
            }
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"][%d]\n",result->count);
            #endif
        }
    }

    if (NULL != status) *status = (valid) ? I2V_RETURN_OK : I2V_RETURN_FAIL;

    return;

}
/* 
 * MAX_WSM_DATA is 2302 so we need twice that for ascii UPER string in 'payload'.
 * 'payload' is huge and thats why all the buffers are huge. 
 * This is not a normal conf file. This a RSU 4.1 SAR & IMF file format. 
 * File format is rigid. No min or max or comments.
 * All sizes are defined once in AMH.H.

    Version=0.7
    Type=MAP16
    PSID=0x8002
    Priority=7
    TxMode=ALT
    TxChannel=172
    TxInterval=1
    DeliveryStart=04/16/2017, 01:01
    DeliveryStop=11/30/2038, 23:50
    Signature=False
    Encryption=False
    Payload1=0012823a3800300020bf

 */

uint32_t amvParseSARFile(FILE *f, cfgItemsTypeT *cfgItems, uint32_t numCfgItems)
{
  uint32_t i,j,parserStatus;
  uint32_t retVal=AMH_AOK;
#if defined(EXTRA_DEBUG)
  bool_t  matchFound=WFALSE;
#endif
  bool_t  cfgFound[numCfgItems];
  char_t  *ptr;
  char_t  buf[MEM_PARSE_CONF_GET_MAX_BUF];
  char_t  val[MEM_PARSE_CONF_GET_MAX_BUF];
  char_t  dummy[MEM_PARSE_CONF_GET_MAX_BUF];

    if (NULL == f) {
#if defined(EXTRA_DEBUG)
        printf("I2V Critical Error (%d-%s): Null file pointer\n", getpid(),__FUNCTION__);
#endif
        return AMHL_SAR_NULL_INPUT;
    }

    memset(buf, 0, sizeof(buf));
    memset(cfgFound, 0, sizeof(cfgFound));  /* Init only once: required for jjg comment later in this fxn */    
    while (fgets(buf, MEM_PARSE_CONF_GET_MAX_BUF, f)) {

        /* Init here because 'continue' will go past bottom of loop. */
        parserStatus = I2V_RETURN_OK;
        retVal = AMH_AOK;
#if defined(EXTRA_DEBUG)
        matchFound = WFALSE;
#endif
        memset(val, 0, sizeof(val));
        memset(dummy, 0, sizeof(dummy));
        ptr = NULL;

        /* Identify and remove comment sections */
        if ((ptr = strrchr(buf, '#')) != NULL) *ptr = '\0';

        /* Remove white space */
        for(i=0,j=0;(i < MEM_PARSE_CONF_GET_MAX_BUF) && (0 != buf[i]); i++) {
            switch(buf[i]) {
                case ' ' :
                case '\t':
                case '\n':
                case '\r':
                    break;
                default:
                    dummy[j] = buf[i];
                    j++;
                    break;
            }
        }

        for(i=0;(i<MEM_PARSE_CONF_GET_MAX_BUF) && ('\0' != dummy[i]);i++) {
            buf[i] = dummy[i];
        }

        if(i < MEM_PARSE_CONF_GET_MAX_BUF)
            buf[i] = '\0';
        else
            buf[MEM_PARSE_CONF_GET_MAX_BUF-1] = '\0';

        /* Blank/Comment lines */
        if ((strlen(buf) == 0) || (buf[0] == '#')) {
            memset(buf, 0, sizeof(buf));
            continue;
        }

        /* Identify value lines */
        if ((ptr = strrchr(buf, '=')) == NULL) {
#if defined(EXTRA_DEBUG)
            printf("(%d)  Rejected - No assignment: %s\n",getpid(),buf);
#endif
            memset(buf, 0, sizeof(buf));
            continue;
        }

        /* Replace '=' with NULL; point ptr to value in string and copy into 'val' */
        *ptr = '\0';
        strcpy(val, ptr+1);
        /* Identify Configuration Item(s) */
        /* Do not print payload like below.
        printf("  Config:  %s:\t val: [%s]", buf, val);
        */
#if defined(EXTRA_DEBUG)
        matchFound = WFALSE;
#endif
        for (i = 0; i < numCfgItems; i++) {
            /* custom functions may have tags that don't match length of parsed buf */
            if (((strncmp(buf, cfgItems[i].tag, strlen(cfgItems[i].tag)) == 0) && (strlen(buf) == strlen(cfgItems[i].tag)))
             || ((strncmp(buf, cfgItems[i].tag, strlen(cfgItems[i].tag)) == 0) && (cfgItems[i].customUpdateFxn != NULL))) {
                if (cfgItems[i].customUpdateFxn == NULL) {
                    (*cfgItems[i].updateFunction)(cfgItems[i].itemToUpdate, val, UNDEFINED, UNDEFINED, NULL, &parserStatus, NULL);
                } else {
#if defined(EXTRA_DEBUG)
                    printf("\n");  /* custom functions may not print out text to log terminate open ended log msg printed above */
#endif
                    (*cfgItems[i].customUpdateFxn)(cfgItems[i].tag, buf, val, cfgItems[i].itemToUpdate, &parserStatus);
                }
#if defined(EXTRA_DEBUG)
                matchFound = WTRUE;
#endif
                if( parserStatus == I2V_RETURN_OK ) {
                    cfgFound[i] = WTRUE;
                }
                else if(   (parserStatus == I2V_RETURN_STR_MAX_LIMIT_FAIL)
                        || (parserStatus == I2V_RETURN_MIN_LIMIT_FAIL)
                        || (parserStatus == I2V_RETURN_MAX_LIMIT_FAIL) ) {
#if defined(EXTRA_DEBUG)
                    printf( "(%d-%s): Limit error on '%s'. Examine Debug Log for details\n", getpid(),__FUNCTION__, cfgItems[i].tag );
#endif
                    /*jjg cleanup - do we really need to return here if later assuming override not set we fail anyway?
                     *pro: if we fail here it speeds up initialization time
                     *con: if we fail here and calling process doesn't check return, other config items will be missing */
                    cfgFound[i] = WFALSE;
                }
                else {
#if defined(EXTRA_DEBUG)
                    printf( "(%d-%s): Discrete value range error on '%s'. Examine Debug Log for details\n", getpid(),__FUNCTION__, cfgItems[i].tag );
#endif
                    cfgFound[i] = WFALSE;
                }
                /* Preserve any error states. */
                retVal |= parserStatus;
                break;
            }
        }
#if defined(EXTRA_DEBUG)
        if (matchFound == WFALSE) {
            printf("(%d-%s)  Rejected - Configuration item %s unidentified\n",getpid(),__FUNCTION__,buf);
        }
#endif
        /* continue' will step over this to bottom of loop */
        memset(buf, 0, sizeof(buf));
    } /* while */

        for (i = 0; i < NUMITEMS(cfgFound); i++) {
            if (!cfgFound[i]) {
#if defined(EXTRA_DEBUG)
                printf("\namhParseSARFile: (%d)Configuration values for '%s' not found.\n", getpid(),cfgItems[i].tag); 
#endif
                /* Return bitmask of error states. */
                if(0 == (strncmp(cfgItems[i].tag,"Version",strlen("Version")))) {
                    retVal |= AMHL_SAR_NO_VERSION;
                }
                if(0 == (strncmp(cfgItems[i].tag,"Type",strlen("Type")))) {
                    retVal |= AMHL_SAR_NO_TYPE;
                }
                if(0 == (strncmp(cfgItems[i].tag,"PSID",strlen("PSID")))) {
                    retVal |= AMHL_SAR_NO_PSID;
                }
                if(0 == (strncmp(cfgItems[i].tag,"Priority",strlen("Priority")))) {
                    retVal |= AMHL_SAR_NO_PRIORITY;
                }
                if(0 == (strncmp(cfgItems[i].tag,"TxMode",strlen("TxMode")))) {
                    retVal |= AMHL_SAR_NO_TXMODE;
                }
                if(0 == (strncmp(cfgItems[i].tag,"TxChannel",strlen("TxChannel")))) {
                    retVal |= AMHL_SAR_NO_TXCHANNEL;
                }
                if(0 == (strncmp(cfgItems[i].tag,"TxInterval",strlen("TxInterval")))) {
                    retVal |= AMHL_SAR_NO_TXINTERVAL;
                }
                if(0 == (strncmp(cfgItems[i].tag,"DeliveryStart",strlen("DeliveryStart")))) {
                    retVal |= AMHL_SAR_NO_DELIVERYSTART;
                }
                if(0 == (strncmp(cfgItems[i].tag,"DeliveryStop",strlen("DeliveryStop")))) {
                    retVal |= AMHL_SAR_NO_DELIVERYSTOP;
                }
                if(0 == (strncmp(cfgItems[i].tag,"Signature",strlen("Signature")))) {
                    retVal |= AMHL_SAR_NO_SIGNATURE;
                }
                if(0 == (strncmp(cfgItems[i].tag,"Payload",strlen("Payload")))) {
                    retVal |= AMHL_SAR_NO_PAYLOAD;
                }
            }
        }
    return retVal;
}

