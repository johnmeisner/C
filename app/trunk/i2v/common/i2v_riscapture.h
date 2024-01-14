/**************************************************************************
 *                                                                        *
 *     File Name: i2v_riscapture.h                                        *
 *     Author:                                                            *
 *         DENSO International America, Inc.                              *
 *         North America Research Laboratory, California Office           *
 *         3252 Business Park Drive                                       *
 *         Vista, CA 92081                                                *
 *                                                                        *
 **************************************************************************/
/* JJG: the purpose of this file is to create the union that all radio
   using applications must define for tracking the service that must be
   disabled when the application exits; this has been a bug for many years
   that went unnoticed but on QNX it has become a problem; any application
   that registers a service request to the radio MUST delete the request
*/
#ifndef I2V_RISCAPTURE_H
#define I2V_RISCAPTURE_H

#include "ris_struct.h"

typedef struct {
    WBOOL isUserService;
    WsmServiceType wsmData;
    union {
        ProviderServiceType  providerData;
        UserServiceType      userData;
    } u;
    /* these variables in case calling app doesn't have placeholders in termination code */
    wint32 sdummy;
    wuint32 udummy;
} i2vRisStruct;

#endif /* I2V_RISCAPTURE_H */
