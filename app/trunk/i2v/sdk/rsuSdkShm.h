/****************************************************************************
*      DENSO International America, Inc.                                   *
*      North America Research Laboratory, California Office                *
*      3252 Business Park Drive                                            *
*      Vista, CA 92081                                                     *
****************************************************************************/

/* WARNING: This file is auto-generated; do not modify unless told to by DENSO */

#ifndef RSUSDKSHM_H
#define RSUSDKSHM_H

#include "sdk_types.h"
#define I2V_SHM_PATH "/i2v_shm"

typedef enum {
 RSUFALSE = 0,
 RSUTRUE = 1
} RSUBOOL;

typedef union {
    unsigned char legacySpat[161];
    rsuSPATDataType modernSpat;
} spatData;

typedef struct {
/* DENSO Engineers: Delete this comment and replace array lengths below with 'S' values printed 
when running I2V from your release that generated this SDK
The 'S' values are printed after number of processes loaded and are in sequential order for struct */
    unsigned char     rsvd1DONOTACCESS[5558];
    RSUBOOL           spatType;  /* TRUE = modernSpat */
    spatData          liveData;
    unsigned char     rsvd2DONOTACCESS[52897];
} __attribute__((packed)) rsuShmMasterT;

#endif /* RSUSDKSHM_H */

