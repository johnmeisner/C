
/***************************************************
 *                                                 *
 *     File Name: twaredecoder.h                   *
 *     Author:    DENSO Int. America, Inc.         *
 *                Carlsbad, CA USA                 *
 *                                                 *
 ***************************************************
 * (C) Copyright 2021 DENSO Int. America, Inc.     *
 *      All rights reserved.                       *
 ***************************************************/
#ifndef TWAREDECODER_H
#define TWAREDECODER_H

/* the number of groups is a misnomer; the bitmask is fixed */
#define NUM_GROUPS            2
#define NUM_PHASES_INDEX      1

#define SUPPORTED_TW_VERSION  2

/* J2735 values */
#define STATE_CONFIDENCE_MIN_TIME   1
#define STATE_CONFIDENCE_MAX_TIME   2


/* fields are named differently in TrafficWare's header; I choose to name them
   based on how DENSO code needs to use them.
   MOVE struct to header file if it needs to be used by other code!!!*/
typedef struct {
    unsigned short  version;  /* version 2 for definition of struct; check? */
    unsigned short  minPhaseTTCarray[EXPECTED_NUM_PHASES];
    unsigned short  maxPhaseTTCarray[EXPECTED_NUM_PHASES];
    unsigned char   redPhasesBitmask[NUM_GROUPS];
    unsigned char   yelPhasesBitmask[NUM_GROUPS];
    unsigned char   grnPhasesBitmask[NUM_GROUPS];
    unsigned char   flsPhasesBitmask[NUM_GROUPS];
    unsigned short  minPedTTCarray[EXPECTED_NUM_PHASES];    /* not used */
    unsigned short  maxPedTTCarray[EXPECTED_NUM_PHASES];    /* not used */
    unsigned char   dontWalkBitmask[NUM_GROUPS];   /* not used */
    unsigned char   pedClearBitmask[NUM_GROUPS];   /* used for ped presence (flashing don't walk sign after walk) */
    unsigned char   walkBitmask[NUM_GROUPS];    /* used to indicate ped presence (someone had to press button) */
    unsigned short  minOvlpTTCarray[EXPECTED_NUM_PHASES];   /* overlap */
    unsigned short  maxOvlpTTCarray[EXPECTED_NUM_PHASES];
    unsigned char   grnOvlpBitmask[NUM_GROUPS];
    unsigned char   yelOvlpBitmask[NUM_GROUPS];
    unsigned char   redOvlpBitmask[NUM_GROUPS];
    unsigned char   flsOvlpBitmask[NUM_GROUPS];
    unsigned short  minChannelTTCarray[EXPECTED_NUM_PHASES];   /* channel?? none of this used */
    unsigned short  maxChannelTTCarray[EXPECTED_NUM_PHASES];
    unsigned char   grnChannelBitmask[NUM_GROUPS];
    unsigned char   yelChannelBitmask[NUM_GROUPS];
    unsigned char   redChannelBitmask[NUM_GROUPS];
    unsigned long   currentEpochTime;
    unsigned short  currentTenthsSecond;   /* the 1/10 second value for countdown accuracy (ranges from 0 - 9) */
    unsigned char   currentPattern;   /* something specific to TrafficWare */
    unsigned char   padByte;
    unsigned long   controllerStatusBitmap;  /* unused */
} __attribute__((packed)) twSPATData;

i2vReturnTypesT twParseSpat(i2vSPATDataType *dest, wuint8 *buf, bool_t useMaxTTC, uint8_t twType);
i2vReturnTypesT twGetPhasesEnabled(uint16_t *phaseMask, scsCfgItemsT *cfg);

#endif /* TWAREDECODER_H */
