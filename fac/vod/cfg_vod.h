
/* Definition of VOD config struct */

#ifndef _CFG_VOD_H_
#define _CFG_VOD_H_

#include "dn_types.h"

typedef struct {

    // VOD Config Data

    bool_t   vodAppEnable;
    bool_t   vodCsvLogEnable;
    uint32_t vodVerifyTimeoutMs;

} cfg_vod_t;

// Definitions for CMM
#define VOD_CONFIG_FILE "vod.conf"

#endif
