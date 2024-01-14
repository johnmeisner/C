/**************************************************************************
 *                                                                        *
 *     File Name:  icddecoder.c                                           *
 *     Author:                                                            *
 *         DENSO International America, Inc.                              *
 *         North America Research Laboratory, California Office           *
 *         3252 Business Park Drive                                       *
 *         Vista, CA 92081                                                *
 *                                                                        *
 **************************************************************************/
/**************************************************************************
    This module provides icd decoding support for a traffic signal
    controller.  

    Based on Battelle Interface Control Document, messages assumed to 
    have format defined in table 4 of the spec.  Future development
    may require extensions to this module or similar types of modules.

    Basic rules:
    1. ICD defines 16 phases but check to make sure 16 phases set.
       What happens if less?  Will assume blocks following phases
       are in same location; this is a poor design but it is out 
       of the hands of this implementation.
    2. ICD provides min and max time to change - J2735 has 1 value -
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
#include "icddecoder.h"

#if defined(MY_UNIT_TEST)
#include "stubs.h"
#else
#define dn_socket         socket
#define dn_bind           bind
#define dn_pthread_create pthread_create

#define dn_select    select
#define dn_recvfrom  recvfrom
#define dn_inet_aton inet_aton
#define dn_inet_ntoa inet_ntoa

#define DN_FD_ZERO  FD_ZERO
#define DN_FD_SET   FD_SET
#define DN_FD_CLR   FD_CLR
#define DN_FD_ISSET FD_ISSET
#endif /*MY_UNIT_TEST*/

//#define EXTRA_DEBUG
//#define EXTRA_EXTRA_DEBUG

/* from scs.c */
extern uint8_t      icdYellowDur;
extern ovlGroupMap OverlapMap; 
extern uint16_t     SpatPedDetect; /* work in progress */
extern uint16_t     SpatPedCall;   /* work in progress */

/* 20180216: QNX mod - SNMP library has a memory leak when making requests;
   one improvement was making the variables passed to the SNMP functions global;
   why is unclear but the memory leak slowed - the growth was on the heap and
   these values aren't malloc'ed; but the snmp function does some forms of
   malloc and free using this data; still under investigation - this problem
   didn't seem to be present on the legacy platforms
*/
static    i2vSnmpData data;
static    char_t str0[I2V_CFG_MAX_STR_LEN];
static    char_t str1[I2V_CFG_MAX_STR_LEN];
static    char_t str2[I2V_CFG_MAX_STR_LEN];
static    char_t str3[I2V_CFG_MAX_STR_LEN];
static    char_t str4[I2V_CFG_MAX_STR_LEN * 2];
static    char_t str5[I2V_CFG_MAX_STR_LEN * 2];
static    char_t *p[] = {str0, str1, str2, str3, str4, str5};

static bool_t icdGetPhaseMask(char_t *hostip, uint16_t port, char_t *oid, uint16_t *phaseMask);

/* This function parses a single row of TSCBM SPaT data.  It is called 16 times (with updated phase pointer).
 * Each call parses vehicle-phase[index], pedestrian-phase[index] and overlap[index].
 * This includes their time-to-change values as well as their R/G/Y and flashing bit-masks.
 */
STATIC i2vReturnTypesT icdParsePhase(  i2vSPATDataType *dest
                                     , spatPhase       *phase
                                     , phaseGroups     *group
                                     , phaseOverlaps   *overlap
                                     , bool_t           useMaxTTC
                                     , uint16_t         oldmask
                                     , uint16_t        *newmask)
{
    // phaseNum is a byte so no need for swap
    uint16_t mask     = 0x0;
    bool_t skipmask   = WFALSE;
    
    if (   (NULL == dest) 
        || (NULL == phase) 
        || (NULL == group)
        || (NULL == overlap)
        || (NULL == newmask)) {
        return I2V_RETURN_FAIL;
    }

    if(SPAT_MAX_APPROACHES < dest->numApproach) {
        #ifdef EXTRA_DEBUG
        printf("\nicd: too many approaches = %d\n",dest->numApproach);
        #endif
        return I2V_RETURN_FAIL;
    }
    if(EXPECTED_NUM_PHASES < phase->phaseNum) {
        #ifdef EXTRA_DEBUG
        printf("\nicd: too many phases = %d\n",phase->phaseNum);
        #endif
        return I2V_RETURN_FAIL;
    }

    mask = (1 << (phase->phaseNum - 1));

    dest->spatApproach[dest->numApproach].approachID  = phase->phaseNum;
    dest->spatApproach[dest->numApproach].curSigPhase = SIG_PHASE_UNKNOWN;

    /* Zero out to start. */
    dest->spatApproach[dest->numApproach + 16].approachID  = phase->phaseNum + 16;
    dest->spatApproach[dest->numApproach + 16].curSigPhase = SIG_PHASE_UNKNOWN;


    /* check for zeros - include ped J.I.C. because controller separately has a ped counter */
    if (   !ntohs(phase->vehMinTimeChange) && !ntohs(phase->vehMaxTimeChange) 
        && !ntohs(phase->pedMinTimeChange) && !ntohs(phase->pedMaxTimeChange) 
        && !ntohs(phase->ovlpMinTimeChange) && !ntohs(phase->ovlpMaxTimeChange)){
        /* ignore zeroed states UNLESS the input oldmask indicates state is valid */
        if (!(oldmask & mask)) {
            #if defined(EXTRA_DEBUG)
            printf("\nicdParsePhase: !(oldmask(0x%x) & mask(0x%x)).\n",oldmask,mask);
            #endif
            return I2V_RETURN_OK;
        } else {
            /* mark newmask for later use - indicate that we think state is invalid */
            skipmask = WTRUE;
            #if defined(EXTRA_DEBUG)
            printf("\nicdParsePhase: skipmask = WTRUE.");
            #endif
        }
    }

#if defined(EXTRA_DEBUG)
    printf("\nicdParsePhase: ntohs(R,Y,G)(0x%x,0x%x,0x%x)\n",ntohs(group->groupReds),ntohs(group->groupYels),ntohs(group->groupGrns));
#endif

    // VEHICLE PHASE

    // set .curSigPhase based on R/G/Y and flashing of the TSCBM
    if (ntohs(group->groupReds) & mask) {
        if (ntohs(group->groupGrns) & mask) {
            dest->spatApproach[dest->numApproach].curSigPhase = SIG_PHASE_UNKNOWN;  // R+G, R+G+Y
        }
        else if (ntohs(group->groupYels) & mask) {
            dest->spatApproach[dest->numApproach].curSigPhase = SIG_PHASE_UNKNOWN;  // R+Y : someday, when supporting EU's pre-movement (SIG_PHASE_RED_AND_YELLOW ??)
        }
        
        else {  // only Red
            if ((ntohs(OverlapMap.FlashOutPhaseStatus)) & mask) {
                dest->spatApproach[dest->numApproach].curSigPhase = SIG_PHASE_FLASHING_RED;
            } else {
                dest->spatApproach[dest->numApproach].curSigPhase = SIG_PHASE_RED;
            }
        }
    } else if (ntohs(group->groupYels) & mask) {
        if (ntohs(group->groupGrns) & mask) {
            dest->spatApproach[dest->numApproach].curSigPhase = SIG_PHASE_UNKNOWN;  // Y+G
        }
        else {  // only Yellow
            if ((ntohs(OverlapMap.FlashOutPhaseStatus)) & mask) {
                dest->spatApproach[dest->numApproach].curSigPhase = SIG_PHASE_FLASHING_YELLOW;
            } else {
                dest->spatApproach[dest->numApproach].curSigPhase = SIG_PHASE_YELLOW;
            }
        }
    } else if (ntohs(group->groupGrns) & mask) {
        // only Green
        if ((ntohs(OverlapMap.FlashOutPhaseStatus)) & mask) {
            dest->spatApproach[dest->numApproach].curSigPhase = SIG_PHASE_FLASHING_GREEN;
        } else {
            dest->spatApproach[dest->numApproach].curSigPhase = SIG_PHASE_GREEN;
        }
    }
    else {
        dest->spatApproach[dest->numApproach].curSigPhase = SIG_PHASE_DARK;     // none of R ,Y or G
    }

    // Just a byte so no endianess issue
    dest->spatApproach[dest->numApproach].yellowDur = icdYellowDur;

    if (useMaxTTC) {
        /* time to next phase and confidence based on max time to change */
        dest->spatApproach[dest->numApproach].timeNextPhase = ntohs(phase->vehMaxTimeChange);
        dest->spatApproach[dest->numApproach].confidence = STATE_CONFIDENCE_MAX_TIME;
        /* JJG for compatibility */
        dest->spatApproach[dest->numApproach].secondaryTimeNextPhase = ntohs(phase->vehMinTimeChange);
    } else {
        /* time to next phase and confidence based on min time to change */
        dest->spatApproach[dest->numApproach].timeNextPhase = ntohs(phase->vehMinTimeChange);
        dest->spatApproach[dest->numApproach].confidence = STATE_CONFIDENCE_MIN_TIME;
        /* JJG for compatibility */
        dest->spatApproach[dest->numApproach].secondaryTimeNextPhase = ntohs(phase->vehMaxTimeChange);
    }


    //OVERLAP PHASE

    dest->spatApproach[dest->numApproach + 16].yellowDur = icdYellowDur;

    if (useMaxTTC) {
        /* time to next phase and confidence based on max time to change */
        dest->spatApproach[dest->numApproach + 16].timeNextPhase = ntohs(phase->ovlpMaxTimeChange);
        dest->spatApproach[dest->numApproach + 16].confidence = STATE_CONFIDENCE_MAX_TIME;
        /* JJG for compatibility */
        dest->spatApproach[dest->numApproach + 16].secondaryTimeNextPhase = ntohs(phase->ovlpMinTimeChange);
    } else {
        /* time to next phase and confidence based on min time to change */
        dest->spatApproach[dest->numApproach + 16].timeNextPhase = ntohs(phase->ovlpMinTimeChange);
        dest->spatApproach[dest->numApproach + 16].confidence = STATE_CONFIDENCE_MIN_TIME;
        /* JJG for compatibility */
        dest->spatApproach[dest->numApproach + 16].secondaryTimeNextPhase = ntohs(phase->ovlpMaxTimeChange);
    }

    if (ntohs(overlap->OverlapReds) & mask) {
        if (ntohs(overlap->OverlapGrns) & mask) {
            dest->spatApproach[dest->numApproach+16].curSigPhase = SIG_PHASE_UNKNOWN;   // R+G, R+G+Y
        }
        else if (ntohs(overlap->OverlapYels) & mask) {
            dest->spatApproach[dest->numApproach+16].curSigPhase = SIG_PHASE_UNKNOWN;   // R+Y : someday, when supporting EU's pre-movement (SIG_PHASE_RED_AND_YELLOW ??)
        }
        else {
            // only Red
            if ((ntohs(OverlapMap.FlashOutOvlpStatus)) & mask) {
                dest->spatApproach[dest->numApproach + 16].curSigPhase = SIG_PHASE_FLASHING_RED;
            } else {
                dest->spatApproach[dest->numApproach + 16].curSigPhase = SIG_PHASE_RED; 
            }
        }
    } else if (ntohs(overlap->OverlapYels) & mask) {
        if (ntohs(overlap->OverlapGrns) & mask) {
            dest->spatApproach[dest->numApproach+16].curSigPhase = SIG_PHASE_UNKNOWN;   // Y+G
        }
        else {
            // only Yellow
            if ((ntohs(OverlapMap.FlashOutOvlpStatus)) & mask) {
                dest->spatApproach[dest->numApproach + 16].curSigPhase = SIG_PHASE_FLASHING_YELLOW;
            } else {
                dest->spatApproach[dest->numApproach + 16].curSigPhase = SIG_PHASE_YELLOW; 
            }
        }
    } else if (ntohs(overlap->OverlapGrns) & mask) {
         // only Green
         if ((ntohs(OverlapMap.FlashOutOvlpStatus)) & mask) {
             dest->spatApproach[dest->numApproach + 16].curSigPhase = SIG_PHASE_FLASHING_GREEN;
         } else {
             dest->spatApproach[dest->numApproach + 16].curSigPhase = SIG_PHASE_GREEN; 
         }
    }
    else {
        dest->spatApproach[dest->numApproach+16].curSigPhase = SIG_PHASE_DARK;      // none of R ,Y or G
    }

#ifdef EXTRA_EXTRA_DEBUG
    if (mask == 1) {
        printf("ph: %d na: %d vmin: %6d vmax: %6d pmin: %6d pmax: %6d r: %04x y: %04x g: %04x csp: %08x tnp: %6d m: %04x\n", 
                phase->phaseNum,
                dest->numApproach,
                ntohs(phase->vehMinTimeChange), 
                ntohs(phase->vehMaxTimeChange),
                ntohs(phase->pedMinTimeChange), 
                ntohs(phase->pedMaxTimeChange),
                ntohs(group->groupReds),
                ntohs(group->groupYels),
                ntohs(group->groupGrns),
                dest->spatApproach[dest->numApproach].curSigPhase,
                dest->spatApproach[dest->numApproach].timeNextPhase,
                mask);
    }
#endif

    if (!skipmask) {
        *newmask |= mask;
    }
    return I2V_RETURN_OK;
}

/* input dest assumed to be memset to 0 */
i2vReturnTypesT icdParseSpat(i2vSPATDataType *dest, uint8_t *buf, bool_t useMaxTTC)
{
    uint8_t i=0;
    spatPhase *phaseptr       = NULL;
    phaseGroups *groupptr     = NULL;
    phaseOverlaps *overlapptr = NULL;
    uint16_t curPhaseMask      = 0;
    i2vReturnTypesT ret       = I2V_RETURN_OK;
    uint16_t my_phaseMask      = 0xFFFF;

    if (   (NULL == dest) || (NULL == buf)) {
        #ifdef EXTRA_DEBUG
        printf("icd: internal error\n");
        #endif
        ret = I2V_RETURN_FAIL;
    } else {

        #if defined(MY_UNIT_TEST)
        if(1 == get_stub_signal(Signal_icdParseSpat_aok)) {
          if(0 == get_stub_iteration(Signal_icdParseSpat_aok)) {
            clear_stub_signal(Signal_icdParseSpat_aok);
            return I2V_RETURN_OK;
          } else {
            dec_stub_iteration(Signal_icdParseSpat_aok);
          }
        }

        if(1 == get_stub_signal(Signal_icdParseSpat_missing_data)) {
           if(0 == get_stub_iteration(Signal_icdParseSpat_missing_data)) {
               clear_stub_signal(Signal_icdParseSpat_missing_data);
               return I2V_RETURN_MISSING_DATA;
           } else {
               dec_stub_iteration(Signal_icdParseSpat_missing_data);
           }
        }
        #endif

        if (buf[CMD_TYPE_INDEX] != ICD_SPAT_TYPE) {
            #ifdef EXTRA_DEBUG
            printf("icd: invalid broadcast type: %#X\n", buf[CMD_TYPE_INDEX]);
            #endif
            ret = I2V_RETURN_FAIL;
        }

        if (buf[NUM_PHASES_INDEX] > EXPECTED_NUM_PHASES) {
            #ifdef EXTRA_DEBUG
            printf("icd: invalid broadcast number of phases: %d\n", buf[NUM_PHASES_INDEX]);
            #endif
            ret = I2V_RETURN_FAIL;
        }

        if(SPAT_MAX_APPROACHES < dest->numApproach) {
            #ifdef EXTRA_DEBUG
            printf("\nicd: too many approaches = %d\n",dest->numApproach);
            #endif
            ret = I2V_RETURN_FAIL;
        }

        if(I2V_RETURN_OK == ret) {

            phaseptr = (spatPhase *)&buf[FIRST_PHASE_OFFSET];
            groupptr = (phaseGroups *)&buf[PHASE_GROUP_OFFSET];

            overlapptr = (phaseOverlaps *)&buf[SPAT_PHASE_OVERLAP_OFFSET];
            OverlapMap.FlashOutPhaseStatus = *((uint16_t *) &buf[FLASH_OUT_PHASE_STATUS_OFFSET]);
            OverlapMap.FlashOutOvlpStatus = *((uint16_t *) &buf[FLASH_OUT_OVLP_STATUS_OFFSET]);   

            dest->intersectionStatus = ((int32_t)buf[SPAT_INTERSECTION_STATUS_OFFSET]);

            for (i=0; i<buf[NUM_PHASES_INDEX]; i++) {
                if (I2V_RETURN_OK != (ret = icdParsePhase(dest
                                           , phaseptr
                                           , groupptr
                                           , overlapptr
                                           , useMaxTTC
                                           , my_phaseMask
                                           , &curPhaseMask))) {
                    /* don't break out if error processing phase - continue to process subsequent phases */
                    #ifdef EXTRA_DEBUG
                    printf("icd: invalid or missing phase ret=%d phase=%d\n",ret, i+1);
                    #endif
                }
                /* 
                 * Keep all phases good, bad or otherwise so user can view in i2vmonitor 
                 * Spat16.c is smart enough not to use invalid phases so don't worry.
                 */
                dest->numApproach++;
                phaseptr++;
            } /* for */

            if(I2V_RETURN_OK == ret) {
                if (curPhaseMask != my_phaseMask) {
                    #ifdef EXTRA_DEBUG
                    printf("icd: broadcast has new set of phases (%.2x %.2x)\n", curPhaseMask, my_phaseMask);
                    #endif
                    ret = I2V_RETURN_MISSING_DATA;
                }
            }

            dest->numApproach = SPAT_MAX_APPROACHES;

        } /* if */
    }  /* if */

    #if defined(EXTRA_DEBUG)
    if(I2V_RETURN_OK == ret) {
        for (i=0; i<243; i++) {
            printf("%.2X ", buf[i]);
            if (i && (i%20 == 19)) printf("\n");
        }
        printf("\n");
    }
    #endif /* EXTRA_DEBUG */

    return ret;
}

/* 20180213: the following support is for accessing SPaT data via NTCIP (SNMP);
   it is defined by the OIDs found in NTCIP1202 -- see the ICD (Battelle) 
   document to find out the OID values; it is assumed that compatible controllers
   will use this format; however, there are controllers that may provide SNMP
   data that use a proprietary format; if such must be supported, a new function
   similar to this one must be implemented specific to the new format */
i2vReturnTypesT icdParseSNMPSpat(i2vSPATDataType *dest, scsCfgItemsT *cfg)
{
    /* note - this function must pull the phase mask each time; perhaps configure this in the future
       but the reason for such behavior is the rsu has no visibility into when an operator changes
       the number of phases */
    /* uint16_t phaseGrnMask = 0, phaseYlwMask = 0, phaseRedMask = 0; */
    uint16_t phaseGrnMask = 0, phaseYlwMask = 0;
    /* hey JJG - why doesn't a generic i2v fxn exist for snmp cmds??;  FIXME for the future */
    uint8_t i, retType = 0, apprctr = 0;   /* apprctr is index for num of approaches in spat data */

    if ((dest == NULL) || (cfg == NULL)) {
        return I2V_RETURN_FAIL;
    }
#if defined(MY_UNIT_TEST)
    if(1 == get_stub_signal(Signal_icdParseSNMPSpat_aok)) {
      if(0 == get_stub_iteration(Signal_icdParseSNMPSpat_aok)) {
        clear_stub_signal(Signal_icdParseSNMPSpat_aok);
        return I2V_RETURN_OK;
      } 
      #if 0
      else {
        dec_stub_iteration(Signal_icdParseSNMPSpat_aok);
      }
      #endif
    }
#endif
    /* step 1; collect phase info */
    if (!icdGetPhaseMask(cfg->localSignalControllerIP, cfg->snmpPort, cfg->phaseGrnOID, &phaseGrnMask)) {
        /* no retries; no spat info for this iteration */
#ifdef EXTRA_DEBUG
        /* this should be timestamped in the future */
        printf("SCS ICD1: could not retrieve SNMP SPAT data OID=[%s][%d]\n",cfg->phaseGrnOID,strlen(cfg->phaseGrnOID));
#endif /* EXTRA_DEBUG */
        return I2V_RETURN_MISSING_DATA;
    }
    if (!icdGetPhaseMask(cfg->localSignalControllerIP, cfg->snmpPort, cfg->phaseYlwOID, &phaseYlwMask)) {
        /* no retries; no spat info for this iteration */
#ifdef EXTRA_DEBUG
        /* this should be timestamped in the future */
        printf("SCS ICD2: could not retrieve SNMP SPAT data\n");
#endif /* EXTRA_DEBUG */
        return I2V_RETURN_MISSING_DATA;
    }
#if 0   /* disabling check for red since we'll assume red for non green and yellow 
           but in reality we should check; code left just in case for future */
    if (!icdGetPhaseMask(cfg->localSignalControllerIP, cfg->snmpPort, cfg->phaseRedOID, &phaseRedMask)) {
        /* no retries; no spat info for this iteration */
#ifdef EXTRA_DEBUG
        /* this should be timestamped in the future */
        printf("SCS ICD: could not retrieve SNMP SPAT data\n");
#endif /* EXTRA_DEBUG */
        return I2V_RETURN_MISSING_DATA;
    }
#endif /* if 0 */

    /* because the same strings are used for all snmp functions, make sure of settings here */
    /* initial SNMP setup - OID is going to change */
    strcpy(str0, "dummy");
    strcpy(str1, "-v1");
    strcpy(str2, "-c");
    strcpy(str3, "public");
    snprintf(str4, sizeof(str4), "%s:%d", cfg->localSignalControllerIP, cfg->snmpPort);
    
    strncpy(str5, (const char_t *)cfg->phaseOptsOID, sizeof(str5));   /* first element to check */

    /* assuming input already memset (it is) */
    for (i=0; i<EXPECTED_NUM_PHASES; i++) {
        snprintf(str5, sizeof(str5), "%s.%d", cfg->phaseOptsOID, (i+1));   /* first element to check; see if phase enabled */

        /* do not retry - phase is considered inactive for this iteration */
        if (0 > i2vSnmpGet(6, p, &retType, &data)) {
#ifdef EXTRA_DEBUG
            printf("bad snmp return (%d %s)\n", i, str5);
#endif /* EXTRA_DEBUG */
            continue;   /* move to next phase */
        }
        if (retType != I2V_ASN_INTEGER) {
#ifdef EXTRA_DEBUG
            printf("bad snmp type (%d %s)\n", i, str5);
#endif /* EXTRA_DEBUG */
            continue;   /* move to next phase */
        }

        if (!((uint8_t)data.integer & 0x01)) {
            /* last bit set means phase active - so if not active; done */
            continue;
        }

        /* phase active, now need to get countdown time + phase number; NOTE: could potentially skip
           retrieiving the phase number; but don't assume it will always align to value of 'i' */
        snprintf(str5, sizeof(str5), "%s.%d", cfg->phaseNumOID, (i+1));
        /* do not retry - phase is considered inactive for this iteration */
        if (0 > i2vSnmpGet(6, p, &retType, &data)) {
            continue;
        }
        if (retType != I2V_ASN_INTEGER) {
            continue;
        }
        dest->spatApproach[apprctr].approachID = (uint8_t)data.integer;
        if (cfg->useMaxTTC) {
            snprintf(str5, sizeof(str5), "%s.%d", cfg->phaseMaxCntdnOID, (i+1));
        } else {
            snprintf(str5, sizeof(str5), "%s.%d", cfg->phaseMinCntdnOID, (i+1));
        }
        /* do not retry - phase is considered inactive for this iteration */
        if (0 > i2vSnmpGet(6, p, &retType, &data)) {
            /* cleanup */
            memset(&dest->spatApproach[apprctr], 0, sizeof(i2vSPATApproachType));
            continue;
        }
        if (retType != I2V_ASN_INTEGER) {
            /* cleanup */
            memset(&dest->spatApproach[apprctr], 0, sizeof(i2vSPATApproachType));
            continue;
        }
        dest->spatApproach[apprctr].timeNextPhase = (uint16_t)data.integer;
        /* 20180828 - JJG for compatibility */
        if (cfg->useMaxTTC) {
            /* swap the values from last countdown retrieval just above */
            snprintf(str5, sizeof(str5), "%s.%d", cfg->phaseMinCntdnOID, (i+1));
        } else {
            snprintf(str5, sizeof(str5), "%s.%d", cfg->phaseMaxCntdnOID, (i+1));
        }
        /* do not retry - phase is considered inactive for this iteration */
        if (0 > i2vSnmpGet(6, p, &retType, &data)) {
            /* cleanup */
            memset(&dest->spatApproach[apprctr], 0, sizeof(i2vSPATApproachType));
            continue;
        }
        if (retType != I2V_ASN_INTEGER) {
            /* cleanup */
            memset(&dest->spatApproach[apprctr], 0, sizeof(i2vSPATApproachType));
            continue;
        }
        dest->spatApproach[apprctr].secondaryTimeNextPhase = (uint16_t)data.integer;
        /* add phase info + yellow dur already obtained */
        dest->spatApproach[apprctr].yellowDur = icdYellowDur;
        /* if (((uint16_t)(1 << (dest->spatApproach[apprctr].approachID - 1))) & phaseGrnMask) { */
        if (((uint16_t)(1 << i)) & phaseGrnMask) {
            /* phase is green */
            dest->spatApproach[apprctr].curSigPhase = SIG_PHASE_GREEN;
        } else if (((uint16_t)(1 << i)) & phaseYlwMask) {
            /* phase is yellow */
            dest->spatApproach[apprctr].curSigPhase = SIG_PHASE_YELLOW;
        } else {
            /* set to red in all other cases */
            dest->spatApproach[apprctr].curSigPhase = SIG_PHASE_RED;
        }
        apprctr++;  /* can increment for next iteration */
    }
    dest->numApproach = apprctr; 
    dest->intersectionStatus = 0x0000;  /* hard coded, controller doesn't provide a val that matches j2735 */

    return I2V_RETURN_OK;
}


bool_t icdGetYelDur(char_t *hostip, uint16_t port, char_t *oid)
{
    uint8_t retType = 0;

    if ((NULL == oid) || (NULL == hostip)) {
        return WFALSE;
    }

    strcpy(str0, "dummy");
    strcpy(str1, "-v1");
    strcpy(str2, "-c");
    strcpy(str3, "public");
    snprintf(str4, sizeof(str4), "%s:%d", hostip, port);
    strncpy(str5, (const char_t *)oid, sizeof(str5));

    /* retries ? */
    if (0 > i2vSnmpGet(6, p, &retType, &data)) {
#ifdef EXTRA_DEBUG
            printf("bad snmp return (%s)\n", str5);
#endif /* EXTRA_DEBUG */
        return WFALSE;
    }

    if (retType != I2V_ASN_INTEGER) {
#ifdef EXTRA_DEBUG
            printf("bad snmp type (%s)\n", str5);
#endif /* EXTRA_DEBUG */
        return WFALSE;
    }

    icdYellowDur = (uint8_t)data.integer;
#ifdef EXTRA_DEBUG
    printf("icd: yellow dur - %d(decisecs)\n", icdYellowDur);
#endif /* EXTRA_DEBUG */

    return WTRUE;
}

/* assuming EXPECTED_NUM_PHASES (16) phases 
   NOTE: use this function and rename if in future need 
   to use phase options */
bool_t icdGetPhaseEnable(char_t *hostip, uint16_t port, char_t *oid, uint16_t *phaseMask)
{
    uint8_t i, retType = 0;

    if ((NULL == oid) || (NULL == hostip) || (NULL == phaseMask)) {
        return WFALSE;
    }

    strcpy(str0, "dummy");
    strcpy(str1, "-v1");
    strcpy(str2, "-c");
    strcpy(str3, "public");
    snprintf(str4,sizeof(str4) ,"%s:%d", hostip, port);

    /* above is good enough for str init since strcpy will add null at the end -
       adding extra for str5 since it is getting reused */
    memset(str5, 0, sizeof(str5));
    *phaseMask = 0;

    for (i=0; i<EXPECTED_NUM_PHASES; i++) {
        snprintf(str5, sizeof(str5), "%s.%d", oid, (i+1));

        /* retries ? */
        if (0 > i2vSnmpGet(6, p, &retType, &data)) {
#ifdef EXTRA_DEBUG
            printf("bad snmp return (%d %s)\n", i, str5);
#endif /* EXTRA_DEBUG */
            return WFALSE;
        }

        if (retType != I2V_ASN_INTEGER) {
#ifdef EXTRA_DEBUG
            printf("bad snmp type (%d %s)\n", i, str5);
#endif /* EXTRA_DEBUG */
            return WFALSE;
        }

        if (((uint8_t)data.integer & 0x01)) {
            /* last bit set means phase active - mask set to match bcast msg */
            *phaseMask |= (1 << i);
        }
    }

    return WTRUE;
}

/* JJG this could be a generic fxn, but in this particular case there is a 
   requirement to make two snmp queries; so making it specific + didn't do
   a macro because other folks were chiding the usage of macros that pervade
   i2v */
static bool_t icdGetPhaseMask(char_t *hostip, uint16_t port, char_t *oid, uint16_t *phaseMask)
{
    uint8_t i, retType = 0;

    if ((NULL == oid) || (NULL == hostip) || (NULL == phaseMask)) {
        return WFALSE;
    }

    strcpy(str0, "dummy");
    strcpy(str1, "-v1");
    strcpy(str2, "-c");
    strcpy(str3, "public");
    snprintf(str4,sizeof(str4), "%s:%d", hostip, port);

    /* above is good enough for str init since strcpy will add null at the end -
       adding extra for str5 since it is getting reused */
    memset(str5, 0, sizeof(str5));
    *phaseMask = 0;

    /* for the phase mask, according to NTCIP1202, it's 2 bytes, so two OIDs
       need to be retrieved, the first is the low byte */
    for (i=0; i<2; i++) {   /* magic 2 explained above */
        snprintf(str5, sizeof(str5), "%s.%d", oid, (i+1));

        /* no retries */
        if (0 > i2vSnmpGet(6, p, &retType, &data)) {
#ifdef EXTRA_DEBUG
            printf("bad snmp return (%d %s)\n", i, str5);
#endif /* EXTRA_DEBUG */
            return WFALSE;
        }

        if (retType != I2V_ASN_INTEGER) {
#ifdef EXTRA_DEBUG
            printf("bad snmp type (%d %s)\n", i, str5);
#endif /* EXTRA_DEBUG */
            return WFALSE;
        }
        /* phase mask assembly */
        *phaseMask |= ((uint16_t)(((uint8_t)data.integer) << i));
    }

    return WTRUE;
}
