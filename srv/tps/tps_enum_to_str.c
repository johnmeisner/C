/*---------------------------------------------------------------------------*/
/*                                                                           */
/*    Filename: tps_enum_to_str.c                                            */
/*     Purpose: Convert TPS-specific enum values to readable strings         */
/*                                                                           */
/* Copyright (C) 2022 DENSO International America, Inc.                      */
/*                                                                           */
/*---------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include "dn_types.h"
#include "shm_tps.h"     // for enums
#include "tps_enum_to_str.h"

char_t * tps_enum_to_str(uint32_t enum_type_id, uint32_t enum_value)
{
    if (enum_type_id == TE_TIME_SYNC_STATE) {
        switch ((tpsTimeSyncState_e) enum_value) {
            case TTS_UNINITIALIZED: return "Uninitialized"; break;  // -- TPS crashed at startup or is still starting up
            case TTS_NO_SYNC: return "No Sync"; break;         // -- TPS has started but date is in 1970 ?
            case TTS_RTC_SYNC: return "RTC Sync"; break;       // -- If RTC set system time at boot ... ?
            case TTS_NOLEAP_SYNC: return "NoLeap Sync"; break; // -- If getting PVT time but no TIMELS leapsec (and no /rwflash stored leapsec)
            case TTS_GPS_SYNC: return "GPS Sync"; break;       // -- When gps.c calls settimeofday() *AND* we have a valid TIMELS leapsec
            default: return "Unknown Value"; break;
        }
    }
    if (enum_type_id == TE_POSN_SYNC_STATE) {
        switch ((tpsPositionSyncState_e) enum_value) {
            case TGP_UNINITIALIZED: return "Uninitialized"; break;  // -- TPS crashed at startup or is still starting up
            case TGP_NO_POSITION: return "No Position"; break;      // -- TPS has started but date is in 1970 ?
            case TGP_GPS_POSITION: return "Position Sync"; break;   // -- When gps.c calls settimeofday() *AND* we have a valid TIMELS leapsec
            default: return "Unknown Value"; break;
        }
    }
    if (enum_type_id == TE_GPS_FIX_TYPE) {
        switch (enum_value) {
            case 0: return "No fix"; break;
            case 1: return "Dead reckoning Only Fix"; break;
            case 2: return "2D-fix"; break;
            case 3: return "3D-fix"; break;
            case 4: return "GPS + dead reckoning combined Fix"; break;
            case 5: return "Time only fix"; break;
            default: return "Unknown Value"; break;
        }
    }
    if (enum_type_id == TE_MAP_MATCH_STATUS) {
        switch (enum_value) {
            case 0: return "None"; break;
            case 1: return "Valid but not used, too old"; break;
            case 2: return "Valid and used, map matching data was applied"; break;
            case 3: return "Valid and used, map matching data was applied and dead reckoning is enabled"; break;
            default: return "Unknown Value"; break;
        }
    }
    if (enum_type_id == TE_POWER_SAVE_MODE_STATE) {
        switch (enum_value) {
            case 0: return "ACQUISITION [or psm disabled]"; break;
            case 1: return "TRACKING"; break;
            case 2: return "POWER OPTIMIZED TRACKING"; break;
            case 3: return "INACTIVE"; break;
            default: return "Unknown Value"; break;
        }
    }
    // Spoofing detection state (not supported in protocol versions less than 18)
    if (enum_type_id == TE_SPOOF_DETECT_STATE) {
        switch (enum_value) {
            case 0: return "Unknown or deactivated"; break;
            case 1: return "No spoofing indicated"; break;
            case 2: return "Spoofing indicated"; break;
            case 3: return "Multiple spoofing indications"; break;
            default: return "Unknown Value"; break;
        }
    }
    if (enum_type_id == TE_CARRIER_SOLN_STATUS) {
        switch (enum_value) {
            case 0: return "No carrier phase range solution"; break;
            case 1: return "Carrier phase range solution with floating ambiguities"; break;
            case 2: return "Carrier phase range solution with fixed ambiguities"; break;
            default: return "Unknown Value"; break;
        }
    }
    if (enum_type_id == TE_SAT_QUALITY_INDICATOR) {
        switch (enum_value) {
            case 0: return "No signal"; break;
            case 1: return "Searching signal"; break;
            case 2: return "Signal acquired"; break;
            case 3: return "Signal detected but unusable"; break;
            case 4: return "Code locked and time synchronized"; break;
            case 5:
            case 6:
            case 7: return "Code and carrier locked and time synchronized"; break;
            default: return "Unknown Value"; break;
        }
    }
    if (enum_type_id == TE_SAT_HEALTH_FLAG) {
        switch (enum_value) {
            case 0: return "Unknown"; break;
            case 1: return "Healthy"; break;
            case 2: return "Unhealthy"; break;
            default: return "Unknown Value"; break;
        }
    }
    if (enum_type_id == TE_SAT_GNSSID) {
        switch (enum_value) {
            case 0: return "GPS"; break;
            case 1: return "SBAS"; break;
            case 2: return "Galileo"; break;
            case 3: return "BeiDou"; break;
            case 4: return "IMES"; break;
            case 5: return "QZSS"; break;
            case 6: return "GLONASS"; break;
            default: return "Unknown Value"; break;
        }
    }
    if (enum_type_id == TE_JAMMING_STATUS) {
        switch (enum_value) {
            case 0: return "Unknown/Feature Disabled"; break;
            case 1: return "OK - no significant jamming"; break;
            case 2: return "Warning - interference visible but fix OK"; break;
            case 3: return "Critical - interference visible and no fix"; break;
            default: return "Unknown Value"; break;
        }
    }

    return "Unknown Enum";
}


