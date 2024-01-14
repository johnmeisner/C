/**************************************************************************
 *                                                                        *
 *     File Name:  twaredecoder.c                                         *
 *     Author:                                                            *
 *         DENSO International America, Inc.                              *
 *         North America Research Laboratory, California Office           *
 *         3252 Business Park Drive                                       *
 *         Vista, CA 92081                                                *
 *                                                                        *
 **************************************************************************/
/**************************************************************************
    This module provides TrafficWare decoding support for a traffic signal
    controller using TrafficWare's proprietary format.  

    The format of the message (based on inspection) is a match to the struct
    contained in this module.  It is defined by TrafficWare as DSRC_PACKET_V2.

    Basic rules:
    1. TrafficWare format similar to ICD; 16 phases supported; data is not provided
       by phase; phases are grouped together and state is maintained with bitmasks.
    2. TrafficWare provides min and max time to change - J2735 has 1 value -
       must set stateconfidence to indicate which one is being used.
       Choice of Min/Max in config file.
    3. Ignore ped & ovlp (overlap) timing - not using that for now.
    4. only need to populate dest->num_approach,
       dest->spat_approach[i].cur_sig_phase,
       dest->spat_approach[i].confidence,
       dest->spat_approach[i].time_next_phase, and 
       dest->spat_approach[i].yellow_dur; these are J2735 encoded. <=== 2009 or earlier!!!!
**************************************************************************/
#include <sys/types.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include "wsu_util.h"
#include "wsu_shm.h"
#include "i2v_general.h"
#include "i2v_util.h"
#include "i2v_shm_master.h"
#include "i2v_snmp_client.h"
#include "twaredecoder.h"

//#define EXTRA_DEBUG
//#define EXTRA_EXTRA_DEBUG

/* from scs.c */
extern ovlGroupMap OverlapMap;
extern uint8_t      icdYellowDur;
extern uint16_t     SpatPedDetect; /* work in progress */
extern uint16_t     SpatPedCall;   /* work in progress */

/* See comment in icddecoder.c about why these are global */
static    i2vSnmpData data;
static    char_t str0[I2V_CFG_MAX_STR_LEN];
static    char_t str1[I2V_CFG_MAX_STR_LEN];
static    char_t str2[I2V_CFG_MAX_STR_LEN];
static    char_t str3[I2V_CFG_MAX_STR_LEN];
static    char_t str4[I2V_CFG_MAX_STR_LEN * 2];
static    char_t str5[I2V_CFG_MAX_STR_LEN * 2];
static    char_t *p[] = {str0, str1, str2, str3, str4, str5};

i2vReturnTypesT twGetPhasesEnabled(uint16_t *phaseMask, scsCfgItemsT *cfg)
{
    uint8_t i, retType; 

    if ((NULL == phaseMask) || (NULL == cfg)) {
        return I2V_RETURN_FAIL;
    }

    strcpy(str0, "dummy");
    strcpy(str1, "-v1");
    strcpy(str2, "-c");
    strcpy(str3, "public");
    snprintf(str4,sizeof(str4), "%s:%d", cfg->localSignalControllerIP, cfg->snmpPort);
    strncpy(str5, (const char_t *)cfg->phaseOptsOID,sizeof(str5));   /* first element to check */

    /* assuming input already memset (it is) */
    for (i=0; i<EXPECTED_NUM_PHASES; i++) {
        snprintf(str5, sizeof(str5), "%s.%d", (const char_t *)cfg->phaseOptsOID, (i+1));   /* first element to check; see if phase enabled */

        /* do not retry - phase is considered inactive for this iteration */
        if (0 > i2vSnmpGet(6, p, &retType, &data)) {
#ifdef EXTRA_DEBUG
            I2V_DBG_LOG("bad snmp return (%d %s)\n", i, str5);
#endif /* EXTRA_DEBUG */
            continue;   /* move to next phase */
        }
        if (retType != I2V_ASN_INTEGER) {
#ifdef EXTRA_DEBUG
            I2V_DBG_LOG("bad snmp type (%d %s)\n", i, str5);
#endif /* EXTRA_DEBUG */
            continue;   /* move to next phase */
        }

        if (!((uint8_t)data.integer & 0x01)) {
            /* last bit set means phase active - so if not active; done */
            continue;
        } else {
            *phaseMask |= (1 << (i));
        }
    }

#ifdef EXTRA_DEBUG
    I2V_DBG_LOG("tware phases enabled: 0x%x\n", *phaseMask);
#endif /* EXTRA_DEBUG */

    return I2V_RETURN_OK;
}

/* This function parses a single row (index) of TSCBM SPaT data.  It is called 16 times (index 0 thru 15).
 * Each call parses vehicle-phase[index], pedestrian-phase[index] and overlap[index].
 * This includes their time-to-change values as well as their R/G/Y and flashing bit-masks.
 */
static i2vReturnTypesT twParsePhase(i2vSPATDataType *dest, twSPATData *twdata, uint8_t index, bool_t useMaxTTC, uint8_t twType)
{
  uint8_t byteIdx  = 0;
  uint8_t byteMask = 0;

    if ((NULL == dest) || (NULL == twdata)) {
        return I2V_RETURN_FAIL;
    }
    twType=twType;

    // 'index' is into the TSCBM data, which has 16 each of vehicle, pedestrian, and overlap phases
    // 'index' is 0-based, so this function should be called 16 times with indexes 0 thru 15 to parse a TWare blob

    byteIdx  = (index > 7) ? 1 : 0;                             // first 8 entries (0-7) use byteIdx = 0 for bit-masked entries
    byteMask = (index > 7) ? (1 << (index - 8)) : (1 << index); // adjust byteMask for byteIdx if necessary

    dest->spatApproach[dest->numApproach].approachID  = index + 1;
    dest->spatApproach[dest->numApproach].curSigPhase = SIG_PHASE_UNKNOWN;
    /* Zero out to start. */
    dest->spatApproach[dest->numApproach + 16].approachID  = index + 1 + 16;
    dest->spatApproach[dest->numApproach + 16].curSigPhase = SIG_PHASE_UNKNOWN;


    // VEHICLE PHASE

    // set .curSigPhase based on R/G/Y and flashing of the TSCBM
    if (twdata->redPhasesBitmask[byteIdx] & byteMask) {
        if (twdata->grnPhasesBitmask[byteIdx] & byteMask) {
            dest->spatApproach[dest->numApproach].curSigPhase = SIG_PHASE_UNKNOWN;  // R+G, R+G+Y
        }
        else if (twdata->yelPhasesBitmask[byteIdx] & byteMask) {
            dest->spatApproach[dest->numApproach].curSigPhase = SIG_PHASE_UNKNOWN;  // R+Y : someday, when supporting EU's pre-movement (SIG_PHASE_RED_AND_YELLOW ??)
        }
        
        else {  // only Red
            if (twdata->flsPhasesBitmask[byteIdx] & byteMask) {
                dest->spatApproach[dest->numApproach].curSigPhase = SIG_PHASE_FLASHING_RED;
            } else {
                dest->spatApproach[dest->numApproach].curSigPhase = SIG_PHASE_RED;
            }
        }
    } else if (twdata->yelPhasesBitmask[byteIdx] & byteMask) {
        if (twdata->grnPhasesBitmask[byteIdx] & byteMask) {
            dest->spatApproach[dest->numApproach].curSigPhase = SIG_PHASE_UNKNOWN;  // Y+G
        }
        else {  // only Yellow
            if (twdata->flsPhasesBitmask[byteIdx] & byteMask) {
                dest->spatApproach[dest->numApproach].curSigPhase = SIG_PHASE_FLASHING_YELLOW;
            } else {
                dest->spatApproach[dest->numApproach].curSigPhase = SIG_PHASE_YELLOW;
            }
        }
    } else if (twdata->grnPhasesBitmask[byteIdx] & byteMask) {
        // only Green
        if (twdata->flsPhasesBitmask[byteIdx] & byteMask) {
            dest->spatApproach[dest->numApproach].curSigPhase = SIG_PHASE_FLASHING_GREEN;
        } else {
            dest->spatApproach[dest->numApproach].curSigPhase = SIG_PHASE_GREEN;
        }
    }
    else {
        dest->spatApproach[dest->numApproach].curSigPhase = SIG_PHASE_DARK;     // none of R ,Y or G
    }

    /* Just a byte so no endianess issue. */
    dest->spatApproach[dest->numApproach].yellowDur = icdYellowDur;

    if (useMaxTTC) {
        /* time to next phase and confidence based on max time to change */
        dest->spatApproach[dest->numApproach].timeNextPhase = ntohs(twdata->maxPhaseTTCarray[index]);
        dest->spatApproach[dest->numApproach].confidence = STATE_CONFIDENCE_MAX_TIME;
        /* JJG for compatibility */
        dest->spatApproach[dest->numApproach].secondaryTimeNextPhase = ntohs(twdata->minPhaseTTCarray[index]);
    } else {
        /* time to next phase and confidence based on min time to change */
        dest->spatApproach[dest->numApproach].timeNextPhase = ntohs(twdata->minPhaseTTCarray[index]);
        dest->spatApproach[dest->numApproach].confidence = STATE_CONFIDENCE_MIN_TIME;
        /* JJG for compatibility */
        dest->spatApproach[dest->numApproach].secondaryTimeNextPhase = ntohs(twdata->maxPhaseTTCarray[index]);
    }

    // PEDESTRIAN PHASE

    /* pedestrian parsing; using walk indicators: both walk and clear bitmasks; either indicates possible peds */
    dest->spatApproach[dest->numApproach].pedPresent = (twdata->walkBitmask[byteIdx] & byteMask) ? WTRUE : WFALSE;
    if (!dest->spatApproach[dest->numApproach].pedPresent) {
        dest->spatApproach[dest->numApproach].pedPresent = (twdata->pedClearBitmask[byteIdx] & byteMask) ? WTRUE : WFALSE;
    }
#ifdef EXTRA_DEBUG
    if (dest->spatApproach[dest->numApproach].pedPresent) {
        I2V_DBG_LOG("found peds %llu\n", i2vUtilGetTimeInMs());
    }
#endif /* EXTRA_DEBUG */


    //OVERLAP PHASE

    #if defined(EXTRA_EXTRA_DEBUG)
    I2V_DBG_LOG("\n*********************************************************************");
    I2V_DBG_LOG("\ntwdata:debug:Overlap(%d)(Y:R:G)=(0x%x:0x%x,0x%x):flash(0x%x):byteMask(0x%x)"
              , 0
              , twdata->yelOvlpBitmask[0]
              , twdata->redOvlpBitmask[0]
              , twdata->grnOvlpBitmask[0]
              , twdata->flsOvlpBitmask[0]
              , byteMask);
    I2V_DBG_LOG("\ntwdata:debug:Overlap(%d)(Y:R:G)=(0x%x:0x%x,0x%x):flash(0x%x):byteMask(0x%x)"
              , 1
              , twdata->yelOvlpBitmask[1]
              , twdata->redOvlpBitmask[1]
              , twdata->grnOvlpBitmask[1]
              , twdata->flsOvlpBitmask[1]
              , byteMask);
    #endif

    dest->spatApproach[dest->numApproach + 16].yellowDur = icdYellowDur;

    if (useMaxTTC) {
        /* time to next phase and confidence based on max time to change */
        dest->spatApproach[dest->numApproach + 16].timeNextPhase = ntohs(twdata->maxOvlpTTCarray[index]);
        dest->spatApproach[dest->numApproach + 16].confidence = STATE_CONFIDENCE_MAX_TIME;
        /* JJG for compatibility */
        dest->spatApproach[dest->numApproach + 16].secondaryTimeNextPhase = ntohs(twdata->minOvlpTTCarray[index]);
    } else {
        /* time to next phase and confidence based on min time to change */
        dest->spatApproach[dest->numApproach + 16].timeNextPhase = ntohs(twdata->minOvlpTTCarray[index]);
        dest->spatApproach[dest->numApproach + 16].confidence = STATE_CONFIDENCE_MIN_TIME;
        /* JJG for compatibility */
        dest->spatApproach[dest->numApproach + 16].secondaryTimeNextPhase = ntohs(twdata->maxOvlpTTCarray[index]);
    }

    // set .curSigPhase based on R/G/Y and flashing of the TSCBM
    if (twdata->redOvlpBitmask[byteIdx] & byteMask) {
        if (twdata->grnOvlpBitmask[byteIdx] & byteMask) {
            dest->spatApproach[dest->numApproach+16].curSigPhase = SIG_PHASE_UNKNOWN;   // R+G, R+G+Y
        }
        else if (twdata->yelOvlpBitmask[byteIdx] & byteMask) {
            dest->spatApproach[dest->numApproach+16].curSigPhase = SIG_PHASE_UNKNOWN;   // R+Y : someday, when supporting EU's pre-movement (SIG_PHASE_RED_AND_YELLOW ??)
        }
        else {
            // only Red
            if (twdata->flsOvlpBitmask[byteIdx] & byteMask) {
                dest->spatApproach[dest->numApproach + 16].curSigPhase = SIG_PHASE_FLASHING_RED;
            } else {
                dest->spatApproach[dest->numApproach + 16].curSigPhase = SIG_PHASE_RED; 
            }
        }
    } else if (twdata->yelOvlpBitmask[byteIdx] & byteMask) {
        if (twdata->grnOvlpBitmask[byteIdx] & byteMask) {
            dest->spatApproach[dest->numApproach+16].curSigPhase = SIG_PHASE_UNKNOWN;   // Y+G
        }
        else {
            // only Yellow
            if (twdata->flsOvlpBitmask[byteIdx] & byteMask) {
                dest->spatApproach[dest->numApproach + 16].curSigPhase = SIG_PHASE_FLASHING_YELLOW;
            } else {
                dest->spatApproach[dest->numApproach + 16].curSigPhase = SIG_PHASE_YELLOW; 
            }
        }
    } else if (twdata->grnOvlpBitmask[byteIdx] & byteMask) {
         // only Green
         if (twdata->flsOvlpBitmask[byteIdx] & byteMask) {
             dest->spatApproach[dest->numApproach + 16].curSigPhase = SIG_PHASE_FLASHING_GREEN;
         } else {
             dest->spatApproach[dest->numApproach + 16].curSigPhase = SIG_PHASE_GREEN; 
         }
    }
    else {
        dest->spatApproach[dest->numApproach+16].curSigPhase = SIG_PHASE_DARK;      // none of R ,Y or G
    }

    return I2V_RETURN_OK;
}

/* input dest assumed to be memset to 0 */
i2vReturnTypesT twParseSpat(i2vSPATDataType *dest, uint8_t *buf, bool_t useMaxTTC, uint8_t twType)
{
    uint8_t i;
    twSPATData *twdata = (twSPATData *)buf;

    if ((NULL == dest) || (NULL == buf)) {
#ifdef EXTRA_DEBUG
        I2V_DBG_LOG("tw: internal error\n");
#endif /* EXTRA_DEBUG */
        return I2V_RETURN_FAIL;
    }

    /* check version; don't try to parse a message that's not expected */
    if (ntohs(twdata->version) != SUPPORTED_TW_VERSION) {
#ifdef EXTRA_DEBUG
        I2V_DBG_LOG("twparse: invalid broadcast type: %#X\n", twdata->version);
        /* don't try to print the data since we don't know how many bytes we received */
#endif /* EXTRA_DEBUG */
        return I2V_RETURN_FAIL;
    }

    /*  there is no intersection status reported; but it's required for 2016 J2735; hardcode it */
    dest->intersectionStatus = 0x0000;

    for (i=0; i<EXPECTED_NUM_PHASES; i++) {
        if (I2V_RETURN_OK != twParsePhase(dest, twdata, i, useMaxTTC, twType)) {
            /* don't break out if error processing phase - continue to process subsequent phases */
#ifdef EXTRA_DEBUG
            I2V_DBG_LOG("\ntw: invalid or missing phase %d\n", i+1);      /* Don't break out, continue processing next phase */
#endif
        }
        dest->numApproach++;
    }
    
    /* Good or bad there's always a blob of something. */
    dest->numApproach = SPAT_MAX_APPROACHES;
    return I2V_RETURN_OK;
}

/* TBD: TrafficWare supports standard NTCIP1202 commands;
   the standard SCS commands found in icddecoder to retrieve data
   via SNMP for things like yellow duration should work untouched */

