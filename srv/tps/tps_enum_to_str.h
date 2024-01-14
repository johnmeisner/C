/*---------------------------------------------------------------------------*/
/*                                                                           */
/*    Filename: tps_enum_to_str.h                                            */
/*     Purpose: Convert TPS-specific enum values to readable strings         */
/*                                                                           */
/* Copyright (C) 2022 DENSO International America, Inc.                      */
/*                                                                           */
/*---------------------------------------------------------------------------*/

#include "dn_types.h"

// The enumeration sets that tps_enum_to_str() will handle
#define TE_TIME_SYNC_STATE 1
#define TE_POSN_SYNC_STATE 2
#define TE_MAP_MATCH_STATUS 3
#define TE_POWER_SAVE_MODE_STATE 4
#define TE_SPOOF_DETECT_STATE 5
#define TE_CARRIER_SOLN_STATUS 6
#define TE_GPS_FIX_TYPE 7
#define TE_SAT_HEALTH_FLAG 8
#define TE_SAT_QUALITY_INDICATOR 9
#define TE_SAT_GNSSID 10
#define TE_JAMMING_STATUS 11

char_t * tps_enum_to_str(uint32_t enum_type_id, uint32_t enum_value);
