#ifndef SDK_TYPES_H
#define SDK_TYPES_H


#define MAX_SPAT_APPROACHES  16

typedef struct {
    unsigned char          approachID;
    unsigned int           curSigPhase;   /* bit mask of signal phases */
    unsigned char          confidence;    /* countdown timer confidence only */
    unsigned short         timeNextPhase;
    unsigned short         secondaryTimeNextPhase;    /* JJG: for compatibility */
    unsigned char          yellowDur;
    unsigned char          pedPresent;
} __attribute__((packed)) rsuSPATApproachType;

typedef struct {
    unsigned int          intID;
    unsigned char         numApproach;
    int                   intersectionStatus; /**/
    rsuSPATApproachType   spatApproach[MAX_SPAT_APPROACHES];
} __attribute__((packed)) rsuSPATDataType;

#endif /* SDK_TYPES_H */

