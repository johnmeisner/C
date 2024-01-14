
/**************************************************************************************************
* Includes
***************************************************************************************************/
#include <stdlib.h>
#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/utilities.h>
#include <net-snmp/net-snmp-includes.h>

#include "wsu_util.h"
#include "wsu_shm.h"

#include "i2v_general.h"
#include "ris_struct.h"
#include "ris.h"

#include "i2v_util.h"
#include "i2v_shm_master.h"
#if defined(J2735_2016)
#include "DSRC.h"
#endif
#if defined(J2735_2023)
#include "MessageFrame.h"
#include "Common.h"
#endif
#include "i2v_riscapture.h"
#include "stubs.h"

#include "rs.h"
#include "../shm_inc/i2v_scs_data.h"
#include "icddecoder.h"
#include "twaredecoder.h"
#include "scs.h"
#include "i2v_snmp_client.h"
#include "conf_table.h"
/**************************************************************************************************
* Defines
***************************************************************************************************/
/* Test configs used. */
#define DEFAULT_CONFIG  "./"
#define NO_CONFIG       "../stubs/"

#define UT_FLAVOR_MYPCIP 1
#define UT_FLAVOR_ENABLEFWD 2
#define MY_ERR_LEVEL   LEVEL_INFO    /* from i2v_util.h */
#define MY_NAME        "Uscs"
/**************************************************************************************************
* Globals
***************************************************************************************************/

/* scs.c */
extern WBOOL        mainloop;
extern scsCfgItemsT cfg; /* warning: scs's main() will reload on you if called. Manage per test.*/
extern ovlGroupMap  OverlapMap;    /* Overlap Mapping for Yellows */
extern wuint8       icdYellowDur;
extern scsSpatInfo spat_info;
extern char_t      confFileName[I2V_CFG_MAX_STR_LEN];

static i2vShmMasterT * UnitshmPtr; /* If you are not calling main() then you need to set SHM to this. */

/* icddecoder.c */
// borrowed from (FYA_TEST)
 /* steady-green overlap 8. */ 
 wuint8 icd_steady_green_ovr_8[SPAT_BUF_SIZE] = {0xCD,0x10,0x01,0x02,0x3A,0x02,0x59,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x02,0x02,0x8A,0x03,0x17,0x02,0x8A,0x03,0x17,0x00,0x00,0x00,0x00,0x03,0x00,0x14,0x00,0x33,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x04,0x00,0x3C,0x00,0x5B,0x00,0x3C,0x00,0x5B,0x00,0x00,0x00,0x00,0x05,0x02,0x3A,0x02,0x59,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x06,0x02,0x8A,0x03,0x17,0x02,0x8A,0x03,0x17,0x00,0x00,0x00,0x00,0x07,0x00,0x14,0x00,0x33,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x08,0x00,0x3C,0x00,0x5B,0x00,0x3C,0x00,0x5B,0x00,0x14,0x00,0x33,0x09,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0A,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0B,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0D,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0E,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0F,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xBB,0x00,0x00,0x00,0x44,0x00,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0x00,0x00,0x00,0x00,0x00,0xFE,0x11,0x33,0x00,0xE6,0x40,0x01,0x8D,0x00,0x00,0x00,0x00};
 /* flashing-yellow overlap 8. */
 wuint8 icd_flashing_yellow_ovr_8[SPAT_BUF_SIZE] = {0xCD,0x10,0x01,0x02,0x0A,0x02,0x0A,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x02,0x02,0x5A,0x02,0xC8,0x02,0x5A,0x02,0xC8,0x00,0x00,0x00,0x00,0x03,0x00,0x02,0x00,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x04,0x00,0x0C,0x00,0x0C,0x00,0x0C,0x00,0x0C,0x00,0x00,0x00,0x00,0x05,0x02,0x0A,0x02,0x0A,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x06,0x02,0x5A,0x02,0xC8,0x02,0x5A,0x02,0xC8,0x00,0x00,0x00,0x00,0x07,0x00,0x02,0x00,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x08,0x00,0x0C,0x00,0x0C,0x00,0x0C,0x00,0x0C,0x00,0x13,0x00,0x13,0x09,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0A,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0B,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0D,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0E,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0F,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xBB,0x00,0x44,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0x00,0x00,0x00,0x00,0x00,0x80,0x00,0xFE,0x10,0x82,0x00,0xE6,0x48,0x02,0x4B,0x00,0x00,0x00,0x00};
 /* steady-yellow overlap 8. */
 wuint8 icd_steady_yellow_ovr_8[SPAT_BUF_SIZE] = {0xCD,0x10,0x01,0x02,0x0A,0x02,0x0A,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x02,0x02,0x5A,0x02,0xC8,0x02,0x5A,0x02,0xC8,0x00,0x00,0x00,0x00,0x03,0x00,0x02,0x00,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x04,0x00,0x0C,0x00,0x0C,0x00,0x0C,0x00,0x0C,0x00,0x00,0x00,0x00,0x05,0x02,0x0A,0x02,0x0A,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x06,0x02,0x5A,0x02,0xC8,0x02,0x5A,0x02,0xC8,0x00,0x00,0x00,0x00,0x07,0x00,0x02,0x00,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x08,0x00,0x0C,0x00,0x0C,0x00,0x0C,0x00,0x0C,0x00,0x03,0x00,0x03,0x09,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0A,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0B,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0D,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0E,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0F,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xBB,0x00,0x44,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFE,0x10,0x82,0x00,0xE6,0x48,0x02,0x4B,0x00,0x00,0x00,0x00};
 /* overlap 8 as a (steady-red) FYA. */
 wuint8 icd_steady_red_ovr_8[SPAT_BUF_SIZE] = {0xCD,0x10,0x01,0x00,0x08,0x00,0x08,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x02,0x00,0x58,0x00,0xC6,0x00,0x58,0x00,0xC6,0x00,0x00,0x00,0x00,0x03,0x02,0x56,0x02,0xC4,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x04,0x02,0xA6,0x03,0x82,0x02,0xA6,0x03,0x82,0x00,0x00,0x00,0x00,0x05,0x00,0x08,0x00,0x08,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x06,0x00,0x58,0x00,0xC6,0x00,0x58,0x00,0xC6,0x00,0x00,0x00,0x00,0x07,0x02,0x56,0x02,0xC4,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x08,0x02,0xA6,0x03,0x82,0x02,0xA6,0x03,0x82,0x07,0x03,0x56,0x03,0x09,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0A,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0B,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0D,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0E,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0F,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0x00,0x00,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFE,0x10,0x0C,0x00,0xD8,0x3F,0x00,0x66,0x00,0x00,0x00,0x00};

/* overlap 8 data (steady-green), but no vehicle phase 8 or ped 8 data (all 0s) */
wuint8 icd_green_ovr_8_without_veh_8[SPAT_BUF_SIZE] = { 0xCD,
                                                    0x10,
                                                    0x01,0x02,0x3A,0x02,0x59,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                                                    0x02,0x02,0x8A,0x03,0x17,0x02,0x8A,0x03,0x17,0x00,0x00,0x00,0x00,
                                                    0x03,0x00,0x14,0x00,0x33,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                                                    0x04,0x00,0x3C,0x00,0x5B,0x00,0x3C,0x00,0x5B,0x00,0x00,0x00,0x00,
                                                    0x05,0x02,0x3A,0x02,0x59,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                                                    0x06,0x02,0x8A,0x03,0x17,0x02,0x8A,0x03,0x17,0x00,0x00,0x00,0x00,
                                                    0x07,0x00,0x14,0x00,0x33,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                                                    0x08,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x14,0x00,0x33,
                                                    0x09,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                                                    0x0A,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                                                    0x0B,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                                                    0x0C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                                                    0x0D,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                                                    0x0E,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                                                    0x0F,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                                                    0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                                                    0x00,0x3B,
                                                    0x00,0x00,
                                                    0x00,0x44,
                                                    
                                                    0x00,0xFF,
                                                    0x00,0x00,
                                                    0x00,0x00,
                                                    
                                                    0x00,0x00,
                                                    0x00,0x00,
                                                    0x00,0x80,
                                                    
                                                    0x00,0x00,
                                                    0x00,0x00,
                                                    
                                                    0x00,0xFE,0x11,0x33,0x00,0xE6,0x40,0x01,0x8D,
                                                    
                                                    0x00,0x00,
                                                    0x00,0x00};


/* spatIntersectionStatus all zeroes (manual control, etc.) */
wuint8 icd_intersection_status_zeroes[SPAT_BUF_SIZE] = { 0xCD,
                                                    0x10,
                                                    0x01,0x02,0x3A,0x02,0x59,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                                                    0x02,0x02,0x8A,0x03,0x17,0x02,0x8A,0x03,0x17,0x00,0x00,0x00,0x00,
                                                    0x03,0x00,0x14,0x00,0x33,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                                                    0x04,0x00,0x3C,0x00,0x5B,0x00,0x3C,0x00,0x5B,0x00,0x00,0x00,0x00,
                                                    0x05,0x02,0x3A,0x02,0x59,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                                                    0x06,0x02,0x8A,0x03,0x17,0x02,0x8A,0x03,0x17,0x00,0x00,0x00,0x00,
                                                    0x07,0x00,0x14,0x00,0x33,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                                                    0x08,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x14,0x00,0x33,
                                                    0x09,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                                                    0x0A,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                                                    0x0B,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                                                    0x0C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                                                    0x0D,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                                                    0x0E,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                                                    0x0F,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                                                    0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                                                    0x00,0x3B,
                                                    0x00,0x00,
                                                    0x00,0x44,
                                                    
                                                    0x00,0xFF,
                                                    0x00,0x00,
                                                    0x00,0x00,
                                                    
                                                    0x00,0x00,
                                                    0x00,0x00,
                                                    0x00,0x80,
                                                    
                                                    0x00,0x00,
                                                    0x00,0x00,
                                                    
                                                    0x00,0xFE,0x11,0x33,0x00,0xE6,0x40,0x01,0x8D,   // first byte here is spatIntersectionStatus
                                                    
                                                    0x00,0x00,
                                                    0x00,0x00};


/* twaredecoder.c */
// borrowed from (FYA_TEST)
  /* steady-green overlap 8. */ 
  wuint8 tw_steady_green_ovr_8[sizeof(twSPATData)] = {0x00,0x02,0x02,0x3A,0x02,0x8A,0x00,0x14,0x00,0x3C,0x02,0x3A,0x02,0x8A,0x00,0x14,0x00,0x3C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x02,0x3C,0x02,0xFA,0x00,0x16,0x00,0x3E,0x02,0x3C,0x02,0xFA,0x00,0x16,0x00,0x3E,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xBB,0x00,0x00,0x00,0x44,0x00,0x00,0x00,0x00,0x00,0x02,0x8A,0x00,0x00,0x00,0x3C,0x00,0x00,0x02,0x8A,0x00,0x00,0x00,0x3C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x02,0xFA,0x00,0x00,0x00,0x3E,0x00,0x00,0x02,0xFA,0x00,0x00,0x00,0x3E,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x14,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x16,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0x00,0x00,0x00,0x7F,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x44,0x00,0x00,0x00,0xBB,0x00,0x60,0x5A,0x44,0x86,0x00,0x08,0x00,0x00,0x00,0x00,0x00,0x00};
  /* flashing-yellow overlap 8. */ 
  wuint8 tw_flashing_yellow_ovr_8[sizeof(twSPATData)] = {0x00,0x02,0x00,0x53,0x00,0xA3,0x02,0xA1,0x00,0x17,0x00,0x53,0x00,0xA3,0x02,0xA1,0x00,0x17,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x53,0x01,0x11,0x03,0x0F,0x00,0x17,0x00,0x53,0x01,0x11,0x03,0x0F,0x00,0x17,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x77,0x00,0x00,0x00,0x88,0x00,0x00,0x00,0x00,0x00,0x00,0xA3,0x00,0x00,0x02,0xF0,0x00,0x00,0x00,0xA3,0x00,0x00,0x02,0xF0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x11,0x00,0x00,0x03,0xCC,0x00,0x00,0x01,0x11,0x00,0x00,0x03,0xCC,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x17,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x17,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0x00,0x7F,0x00,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x88,0x00,0x00,0x00,0x77,0x00,0x60,0x5A,0x41,0x5D,0x00,0x07,0x00,0x00,0x00,0x00,0x00,0x00};
  /* steady-yellow overlap 8. */
  wuint8 tw_steady_yellow_ovr_8[sizeof(twSPATData)] = {0x00,0x02,0x02,0x1A,0x02,0x6A,0x00,0x12,0x00,0x1C,0x02,0x1A,0x02,0x6A,0x00,0x12,0x00,0x1C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x02,0x1A,0x02,0xD8,0x00,0x12,0x00,0x1C,0x02,0x1A,0x02,0xD8,0x00,0x12,0x00,0x1C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xBB,0x00,0x44,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x02,0x6A,0x00,0x00,0x00,0x1C,0x00,0x00,0x02,0x6A,0x00,0x00,0x00,0x1C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x02,0xD8,0x00,0x00,0x00,0x1C,0x00,0x00,0x02,0xD8,0x00,0x00,0x00,0x1C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x12,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x12,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0x00,0x7F,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x44,0x00,0xBB,0x00,0x60,0x5A,0x44,0x8A,0x00,0x02,0x00,0x00,0x00,0x00,0x00,0x00};
  /* overlap 8 as a (steady-red) FYA. */
  wuint8 tw_steady_red_ovr_8[sizeof(twSPATData)] = {0x00,0x02,0x02,0x61,0x02,0xB1,0x00,0x13,0x00,0x63,0x02,0x61,0x02,0xB1,0x00,0x13,0x00,0x63,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x02,0xCF,0x03,0x8D,0x00,0x13,0x00,0xD1,0x02,0xCF,0x03,0x8D,0x00,0x13,0x00,0xD1,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x02,0xB1,0x00,0x00,0x00,0x63,0x00,0x00,0x02,0xB1,0x00,0x00,0x00,0x63,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x03,0x8D,0x00,0x00,0x00,0xD1,0x00,0x00,0x03,0x8D,0x00,0x00,0x00,0xD1,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x13,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x13,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0x00,0x60,0x5A,0x44,0x77,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00};
  /* intersectionStatus (aka controllerStatusBitmap in tware blob) = zeroes */
  wuint8 tw_intersection_status_zeroes[sizeof(twSPATData)] = {0x00,0x02,0x02,0x61,0x02,0xB1,0x00,0x13,0x00,0x63,0x02,0x61,0x02,0xB1,0x00,0x13,0x00,0x63,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x02,0xCF,0x03,0x8D,0x00,0x13,0x00,0xD1,0x02,0xCF,0x03,0x8D,0x00,0x13,0x00,0xD1,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x02,0xB1,0x00,0x00,0x00,0x63,0x00,0x00,0x02,0xB1,0x00,0x00,0x00,0x63,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x03,0x8D,0x00,0x00,0x00,0xD1,0x00,0x00,0x03,0x8D,0x00,0x00,0x00,0xD1,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x13,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x13,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0x00,0x60,0x5A,0x44,0x77,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00};

/* i2v_snmp_client.c */
extern netsnmp_session  *ss;
extern void i2vSnmpParseDefault(int __attribute__((unused)) numParams, 
            char __attribute__((unused)) *const *paramBuff, int __attribute__((unused)) opt);

/* stubs.c */

/**************************************************************************************************
* Protos & Externs
***************************************************************************************************/
/* scs.c */
extern int  my_main(void);
extern void scs_sighandler(int __attribute__((unused)) sig);
extern int  scs_process_spat(unsigned char *buf);
extern void scsSendSRM_Raw(int *srmSocket, wuint8 *raw_data, wuint8 raw_length);
extern void scsSRMMC(wuint8 laneID, wuint16 seconds);
extern void scsSNMPMain(void);
extern int  scs_init_tc_sock(void);
extern int  scs_init_srm_sock(void);
extern void scsSendSRM(int *srmSocket, SRMMsgType *SRMMsg);
extern void scsProcessSRM(int *srmSocket);
extern void scsMain(int scs_sock, int gs2Socket);

/* icddecoder.c */
extern i2vReturnTypesT icdParsePhase(  i2vSPATDataType * dest
                                     , spatPhase       * phase
                                     , phaseGroups     * group
                                     , phaseOverlaps   * overlap
                                     , WBOOL             useMaxTTC
                                     , wuint16           oldmask
                                     , wuint16         * newmask);
extern WBOOL icdGetOvlpAttr(scsCfgItemsT *cfg);

/* test.c: Should see corresponding test function below for ones externed in from above. */
void test_main(void);
int  init_suite(void);
int  clean_suite(void);
void test_scs(void);
void test_scs_main(void);
void test_scs_process_spat(void);
void test_scsProcessSRM(void);
void test_scsMain(void); 
void test_scsSNMPMain(void);
void test_icdParseSpat(void);
void test_icdParseSNMPSpat(void);
void test_scsSendSRM_Raw(void);
void test_scs_init_tc_sock(void);
void test_scs_init_srm_sock(void);
void test_icdParsePhase(void);
void test_icdGetOvlpAttr(void);
void test_twaredecoder(void);
/**************************************************************************************************
* Function Bodies
***************************************************************************************************/
unsigned char my_spat_buf[SPAT_BUF_SIZE];     //test buffer
unsigned char my_icd_spat_buf[SPAT_BUF_SIZE]; //store valid copy for later
unsigned char my_wsu_spat_buf[SPAT_BUF_SIZE]; //store valid copy for later

void test_scs_main(void)
{
  int32_t ret = I2V_RETURN_OK;
  /* 
   * Check and clear pending tests
   */
  CU_ASSERT(0x0 == check_stub_pending_signals());
  clear_all_stub_signal();

  /* Test against bad conf. */
  strcpy(UnitshmPtr->cfgData.config_directory, "../stubs/configs/");
  strcpy(confFileName, "spat16_bad.conf");
  UnitshmPtr->cfgData.globalDebugFlag  = 1; /* If you want to see debug */
  UnitshmPtr->cfgData.h.ch_data_valid  = WTRUE;

  ret = my_main();
  CU_ASSERT(SCS_AOK == ret);

  CU_ASSERT(cfg.tomformat == SCS_INTERFACE_TOM_DEFAULT);
  CU_ASSERT(cfg.useMaxTTC == SCS_USE_MAX_TIMETO_CHANGE_DEFAULT);
  CU_ASSERT(cfg.RSUHeartBeatEnable == SCS_HEARTBEAT_ENABLE_DEFAULT);
  CU_ASSERT(cfg.RSUHeartBeatInt == SRMRX_HEARTBEAT_INTERVAL_DEFAULT);
  CU_ASSERT(cfg.SRMFwdTest == SRMRX_FWD_TEST_DEFAULT);
  CU_ASSERT(strcmp(cfg.SRMFile,SRMRX_FWD_TEST_FILE_S) == 0);
  CU_ASSERT(cfg.SRMFwdEnable == SRMRX_FWD_ENABLE_DEFAULT);
  CU_ASSERT(cfg.SRMFwdInterval == SRMRX_FWD_INTERVAL_DEFAULT);
  CU_ASSERT(strcmp(cfg.SRMFwdIP,SRMRX_FWD_IP_DEFAULT_S) == 0);
  CU_ASSERT(cfg.SRMFwdPort == SRMRX_FWD_PORT_DEFAULT);
  CU_ASSERT(cfg.SRMFwdInterface == SRMRX_FWD_INTERFACE_DEFAULT);
  CU_ASSERT(cfg.requireTTI == SRMRX_REQUIRE_TTI_DEFAULT);
  CU_ASSERT(strcmp(cfg.localSignalControllerIP,I2V_SCS_LOCAL_CONTROLLER_IP_DEFAULT_S) == 0);
  CU_ASSERT(cfg.localSignalControllerPort == I2V_SCS_LOCAL_CONTROLLER_PORT_DEFAULT);
  CU_ASSERT(cfg.snmpPort == I2V_SCS_LOCAL_SNMP_IP_DEFAULT);
  CU_ASSERT(cfg.spat_intersection_id == SCS_LOCAL_SIGNAL_INT_ID_DEFAULT);
  CU_ASSERT(cfg.ifcSCS == SCS_LOCAL_CONTROLLER_ENABLE_DEFAULT);
  CU_ASSERT(cfg.useSnmpScs == SCS_LOCAL_CONTROLLER_IS_SNMP_DEFAULT);
  CU_ASSERT(cfg.localSignalControllerHW == SCS_LOCAL_SIGNAL_HW_DEFAULT);
  CU_ASSERT(cfg.nontcip ==  SCS_BYPASS_SIGNAL_CONTROLLER_NTCIP_DEFAULT);
  CU_ASSERT(cfg.staticyeldur == SCS_BYPASS_YELLOW_DURATION_DEFAULT);
  CU_ASSERT(strcmp(cfg.enable_spat_val,SCS_SPAT_COMMAND_DEFAULT_S) == 0); 
  CU_ASSERT(strcmp(cfg.enable_spat,SCS_SPAT_SNMP_ENABLE_COMMAND_DEFAULT_S) == 0);
  CU_ASSERT(strcmp(cfg.yellowDurOID, NTCIP_YELLOW_DURATION_DEFAULT_S) == 0);
  CU_ASSERT(strcmp(cfg.phaseOptsOID, NTCIP_PHASE_OPTIONS_DEFAULT_S) == 0);
  CU_ASSERT(strcmp(cfg.phaseGrnOID, NTCIP_PHASE_GREENS_DEFAULT_S) == 0);
  CU_ASSERT(strcmp(cfg.phaseYlwOID, NTCIP_PHASE_YELLOWS_DEFAULT_S) == 0);
  CU_ASSERT(strcmp(cfg.phaseNumOID, NTCIP_PHASE_NUMBER_DEFAULT_S) == 0);
  CU_ASSERT(strcmp(cfg.phaseMaxCntdnOID, NTCIP_PHASE_MAX_COUNTDOWN_DEFAULT_S) == 0);
  CU_ASSERT(strcmp(cfg.phaseMinCntdnOID, NTCIP_PHASE_MIN_COUNTDOWN_DEFAULT_S) == 0);

  //82% coverage.

  /* Setup default SHM items we want */
  UnitshmPtr->cfgData.config_directory[0] = '.';
  UnitshmPtr->cfgData.config_directory[1] = '/';
  UnitshmPtr->cfgData.config_directory[2] = '\0';
  strcpy(confFileName, SCS_CFG_FILE);

  /* Validate that conf_table.h matches scs.conf */
  ret = my_main();
  CU_ASSERT(SCS_AOK == ret);
  CU_ASSERT(cfg.tomformat == SCS_INTERFACE_TOM_DEFAULT);
  CU_ASSERT(cfg.useMaxTTC == SCS_USE_MAX_TIMETO_CHANGE_DEFAULT);
  CU_ASSERT(cfg.RSUHeartBeatEnable == SCS_HEARTBEAT_ENABLE_DEFAULT);
  CU_ASSERT(cfg.RSUHeartBeatInt == SRMRX_HEARTBEAT_INTERVAL_DEFAULT);
  CU_ASSERT(cfg.SRMFwdTest == SRMRX_FWD_TEST_DEFAULT);
  CU_ASSERT(strcmp(cfg.SRMFile,SRMRX_FWD_TEST_FILE_S) == 0);
  CU_ASSERT(cfg.SRMFwdEnable == SRMRX_FWD_ENABLE_DEFAULT);
  CU_ASSERT(cfg.SRMFwdInterval == SRMRX_FWD_INTERVAL_DEFAULT);
  CU_ASSERT(strcmp(cfg.SRMFwdIP,SRMRX_FWD_IP_DEFAULT_S) == 0);
  CU_ASSERT(cfg.SRMFwdPort == SRMRX_FWD_PORT_DEFAULT);
  CU_ASSERT(cfg.SRMFwdInterface == SRMRX_FWD_INTERFACE_DEFAULT);
  CU_ASSERT(cfg.requireTTI == SRMRX_REQUIRE_TTI_DEFAULT);
  CU_ASSERT(strcmp(cfg.localSignalControllerIP,I2V_SCS_LOCAL_CONTROLLER_IP_DEFAULT_S) == 0);
  CU_ASSERT(cfg.localSignalControllerPort == I2V_SCS_LOCAL_CONTROLLER_PORT_DEFAULT);
  CU_ASSERT(cfg.snmpPort == I2V_SCS_LOCAL_SNMP_IP_DEFAULT);
  CU_ASSERT(cfg.spat_intersection_id == SCS_LOCAL_SIGNAL_INT_ID_DEFAULT);
  CU_ASSERT(cfg.ifcSCS == SCS_LOCAL_CONTROLLER_ENABLE_DEFAULT);
  CU_ASSERT(cfg.useSnmpScs == SCS_LOCAL_CONTROLLER_IS_SNMP_DEFAULT);
  CU_ASSERT(cfg.localSignalControllerHW == SCS_LOCAL_SIGNAL_HW_DEFAULT);
  CU_ASSERT(cfg.nontcip ==  SCS_BYPASS_SIGNAL_CONTROLLER_NTCIP_DEFAULT);
  CU_ASSERT(cfg.staticyeldur == SCS_BYPASS_YELLOW_DURATION_DEFAULT);
  CU_ASSERT(strcmp(cfg.enable_spat_val,SCS_SPAT_COMMAND_DEFAULT_S) == 0); 
  CU_ASSERT(strcmp(cfg.enable_spat,SCS_SPAT_SNMP_ENABLE_COMMAND_DEFAULT_S) == 0);
  CU_ASSERT(strcmp(cfg.yellowDurOID, NTCIP_YELLOW_DURATION_DEFAULT_S) == 0);
  CU_ASSERT(strcmp(cfg.phaseOptsOID, NTCIP_PHASE_OPTIONS_DEFAULT_S) == 0);
  CU_ASSERT(strcmp(cfg.phaseGrnOID, NTCIP_PHASE_GREENS_DEFAULT_S) == 0);
  CU_ASSERT(strcmp(cfg.phaseYlwOID, NTCIP_PHASE_YELLOWS_DEFAULT_S) == 0);
  CU_ASSERT(strcmp(cfg.phaseNumOID, NTCIP_PHASE_NUMBER_DEFAULT_S) == 0);
  CU_ASSERT(strcmp(cfg.phaseMaxCntdnOID, NTCIP_PHASE_MAX_COUNTDOWN_DEFAULT_S) == 0);
  CU_ASSERT(strcmp(cfg.phaseMinCntdnOID, NTCIP_PHASE_MIN_COUNTDOWN_DEFAULT_S) == 0);

#if 0 //use upper case macro
  set_stub_signal(Signal_wsu_share_init);
  ret = my_main();
  CU_ASSERT(SCS_SHM_INIT_FAIL == ret);
#endif

  UnitshmPtr->cfgData.config_directory[0] = '/';
  UnitshmPtr->cfgData.config_directory[1] = '\0';
  ret = my_main(); //cause fopen error on scs_update_local_cfg()
  CU_ASSERT(abs(SCS_LOAD_CONF_FAIL) == ret);

  UnitshmPtr->cfgData.config_directory[0] = '.';
  UnitshmPtr->cfgData.config_directory[1] = '/';
  UnitshmPtr->cfgData.config_directory[2] = '\0';
  set_stub_signal(Signal_disable_scs); //park scs. will overide scs.conf
  ret = my_main();
  CU_ASSERT(SCS_AOK == ret);

  //srmSocket = scs_init_srm_sock()) == -1
  set_stub_signal_iteration(Signal_socket,1);
  ret = my_main();
  CU_ASSERT(abs(SCS_INIT_SRM_SOCK_FAIL) == ret);

  set_stub_signal_iteration(Signal_setsockopt,0);
  ret = my_main();
  CU_ASSERT(abs(SCS_INIT_TC_SOCK_FAIL) == ret);

  CU_ASSERT(0x0 == check_stub_pending_signals());
  clear_all_stub_signal();

  set_stub_signal_iteration(Signal_setsockopt,1);
  ret = my_main();
  CU_ASSERT(abs(SCS_INIT_SRM_SOCK_FAIL) == ret);

  CU_ASSERT(0x0 == check_stub_pending_signals());
  clear_all_stub_signal();

  sleep(1);
  ret = my_main();
  CU_ASSERT(SCS_AOK  == ret);

#if 0
  //if (dn_pthread_create(&SrmFwdThreadID, NULL, SRM_Fwd_Thread, &srmSocket) == -1)
  set_stub_signal_iteration(Signal_pthread_create,1);
  ret = my_main();
  CU_ASSERT(SCS_CREATE_SRM_THREAD_FAIL == ret);
  CU_ASSERT(0x0 == check_stub_pending_signals());
#endif
  sleep(1);
  ret = my_main();
  CU_ASSERT(SCS_AOK  == ret);

#if 0 //check conf for enable first
  //(dn_pthread_create(&hbMsgThreadID, NULL, HeartBeatThread, &srmSocket) == -1)
  set_stub_signal_iteration(Signal_pthread_create,2);
  ret = my_main();
  CU_ASSERT(SCS_CREATE_SRM_HEARTB_FAIL == ret);
  CU_ASSERT(0x0 == check_stub_pending_signals());
#endif
  cfg.useMaxTTC = WFALSE;
  set_stub_signal(Signal_enable_Snmpscs);
  ret = my_main();
  CU_ASSERT(SCS_AOK  == ret);

  /* 
   * Check and clear pending tests
   */
  CU_ASSERT(0x0 == check_stub_pending_signals());
  clear_all_stub_signal();

  return;
}
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void test_twaredecoder(void)
{
  //twSPATData      * twdata;
  int32_t   ret = I2V_RETURN_OK;
  //wuint16           my_phaseMask = 0x0;
  //wuint32 i,j,k;
  unsigned char my_spat_buf[SPAT_BUF_SIZE];     //test buffer

  //twdata = (twSPATData *)&my_spat_buf[0];
  cfg.localSignalControllerHW = TrafficWare;
  cfg.tomformat = WFALSE;
  cfg.useMaxTTC = WTRUE;

  memcpy(my_spat_buf,tw_steady_green_ovr_8,   sizeof(tw_steady_green_ovr_8));      
  ret = scs_process_spat(my_spat_buf);
  CU_ASSERT(-1 == ret);
#if 0 //No SHM
  memcpy(my_spat_buf,tw_flashing_yellow_ovr_8,sizeof(tw_flashing_yellow_ovr_8));
  ret = scs_process_spat(my_spat_buf);
  CU_ASSERT(0 == ret);

  memcpy(my_spat_buf,tw_steady_yellow_ovr_8,  sizeof(tw_steady_yellow_ovr_8));
  ret = scs_process_spat(my_spat_buf);
  CU_ASSERT(0 == ret);

  memcpy(my_spat_buf,tw_steady_red_ovr_8,     sizeof(tw_steady_red_ovr_8));
  ret = scs_process_spat(my_spat_buf);
  CU_ASSERT(0 == ret);
  
  // test intersectionStatus .. controllerStatusBitmap in twaredecoder.h
    // BUT .. since we don't know the bit-mappings (for the TrafficWare blob's controllerStatusBitmap), we always set the output intersectionStatus = 0
    
    // with controllerStatusBitmap = all 0s
  memcpy(my_spat_buf,tw_intersection_status_zeroes,     sizeof(tw_intersection_status_zeroes));
  ret = scs_process_spat(my_spat_buf);
  CU_ASSERT(0 == ret);
  //printf("\n .. flexSpat.intersectionStatus: %d\n", spat_info.flexSpat.intersectionStatus);
  CU_ASSERT(0x00 == spat_info.flexSpat.intersectionStatus);

    // with controllerStatusBitmap = all 1s
  memcpy(my_spat_buf,tw_intersection_status_zeroes,     sizeof(tw_intersection_status_zeroes));
  twdata->controllerStatusBitmap = 0xffffffff;  // set all bits in controllerStatusBitmap
  ret = scs_process_spat(my_spat_buf);
  CU_ASSERT(0 == ret);
  //printf("\n .. flexSpat.intersectionStatus: %d\n", spat_info.flexSpat.intersectionStatus);
  CU_ASSERT(0x00 == spat_info.flexSpat.intersectionStatus);

  my_spat_buf[0] = 0;     
  my_spat_buf[1] = 0;  
  ret = scs_process_spat(my_spat_buf);
  CU_ASSERT(-1 == ret);

  //twSPATData.version
  my_spat_buf[0] = 0x00;    
  my_spat_buf[1] = 0x02;  //SUPPORTED_TW_VERSION 

  //force overlaps and apply machine gun method. Brute force coverage.

  for(i = 0;i<EXPECTED_NUM_PHASES;i++) { 
      OverlapMap.ovlpMappingArray[i].Phase2OvlpMask = 0x2; //mask magic!
      OverlapMap.ovlpMappingArray[i].Ovlp2PhaseMask = 0xA;
  }

cfg.useMaxTTC = WFALSE;

  for(k=TrafficWare;k<=TrafficWarePromiscuous;k++){
    cfg.localSignalControllerHW = k;
    for(j=0;j<=0xff;j++) {
      srand(j);
      for(i=2;i < SPAT_BUF_SIZE;i++) {  
        my_spat_buf[i] = 0xff & rand(); 
      }
      twdata->yelOvlpBitmask[0] = 0x0A;
      twdata->redOvlpBitmask[0] = 0x0A;
      twdata->grnOvlpBitmask[0] = 0x0A;

      twdata->yelOvlpBitmask[1] = 0x0A;
      twdata->redOvlpBitmask[1] = 0x0A;
      twdata->grnOvlpBitmask[1] = 0x0A;    
      ret = scs_process_spat(my_spat_buf);
    }  
  }

  for(k=TrafficWare;k<=TrafficWarePromiscuous;k++){
    cfg.localSignalControllerHW = k;
    for(j=0;j<=0xff;j++) {
      for(i=2;i < SPAT_BUF_SIZE;i++) {
        my_spat_buf[i] = 0xff; 
      }
      twdata->yelOvlpBitmask[0] = 0x0A;
      twdata->redOvlpBitmask[0] = 0x0A;
      twdata->grnOvlpBitmask[0] = 0x0A;

      twdata->yelOvlpBitmask[1] = 0x0A;
      twdata->redOvlpBitmask[1] = 0x0A;
      twdata->grnOvlpBitmask[1] = 0x0A;    
      ret = scs_process_spat(my_spat_buf);
    }
  }

  for(k=TrafficWare;k<=TrafficWarePromiscuous;k++){
    cfg.localSignalControllerHW = k;
    for(j=0;j<=0xff;j++) {
      for(i=2;i < SPAT_BUF_SIZE;i++) {
        my_spat_buf[i] = 0xff & ~i; 
      }
      twdata->yelOvlpBitmask[0] = 0x0A;
      twdata->redOvlpBitmask[0] = 0x0A;
      twdata->grnOvlpBitmask[0] = 0x0A;

      twdata->yelOvlpBitmask[1] = 0x0A;
      twdata->redOvlpBitmask[1] = 0x0A;
      twdata->grnOvlpBitmask[1] = 0x0A;    
      ret = scs_process_spat(my_spat_buf);
    }
  }

cfg.useMaxTTC = WTRUE;

  for(k=TrafficWare;k<=TrafficWarePromiscuous;k++){
    cfg.localSignalControllerHW = k;
    for(j=0;j<=0xff;j++) {
      srand(j);
      for(i=2;i < SPAT_BUF_SIZE;i++) {  
        my_spat_buf[i] = 0xff & rand(); 
      }
      twdata->yelOvlpBitmask[0] = 0x0A;  
      twdata->redOvlpBitmask[0] = 0x0A;
      twdata->grnOvlpBitmask[0] = 0x0A;

      twdata->yelOvlpBitmask[1] = 0x0A;
      twdata->redOvlpBitmask[1] = 0x0A;
      twdata->grnOvlpBitmask[1] = 0x0A;     
      ret = scs_process_spat(my_spat_buf);
    }
  }

  for(k=TrafficWare;k<=TrafficWarePromiscuous;k++){
    cfg.localSignalControllerHW = k;
    for(j=0;j<=0xff;j++) {
      for(i=2;i < SPAT_BUF_SIZE;i++) {
        my_spat_buf[i] = 0xff; 
      }
      twdata->yelOvlpBitmask[0] = 0x0A;
      twdata->redOvlpBitmask[0] = 0x0A;
      twdata->grnOvlpBitmask[0] = 0x0A;

      twdata->yelOvlpBitmask[1] = 0x0A;
      twdata->redOvlpBitmask[1] = 0x0A;
      twdata->grnOvlpBitmask[1] = 0x0A;     
      ret = scs_process_spat(my_spat_buf);
    }
  }

  for(k=TrafficWare;k<=TrafficWarePromiscuous;k++){
    cfg.localSignalControllerHW = k;
    for(j=0;j<=0xff;j++) {
      for(i=2;i < SPAT_BUF_SIZE;i++) {
        my_spat_buf[i] = 0xff & ~i; 
      }
      twdata->yelOvlpBitmask[0] = 0x0A;
      twdata->redOvlpBitmask[0] = 0x0A;
      twdata->grnOvlpBitmask[0] = 0x0A;

      twdata->yelOvlpBitmask[1] = 0x0A;
      twdata->redOvlpBitmask[1] = 0x0A;
      twdata->grnOvlpBitmask[1] = 0x0A;    
      ret = scs_process_spat(my_spat_buf);
    }
  }

//  ret = scs_process_spat(my_spat_buf);
//  CU_ASSERT(-1 == ret);

//  91:        if (0 > i2vSnmpGet(6, p, &retType, &data)) {

  set_stub_signal(Signal_oid_phase_number);
  CU_ASSERT(WFALSE == twGetPhasesEnabled(&my_phaseMask, &cfg));
  CU_ASSERT(0x0 == check_stub_pending_signals());
  clear_all_stub_signal();

//97:        if (retType != I2V_ASN_INTEGER) {

  set_stub_signal(Signal_oid_phase_number_bad_type);
  CU_ASSERT(WFALSE == twGetPhasesEnabled(&my_phaseMask, &cfg));
  CU_ASSERT(0x0 == check_stub_pending_signals());
  clear_all_stub_signal();

//104:        if (!((wuint8)data.integer & 0x01)) {

  set_stub_signal(Signal_oid_phase_number_not_active);
  CU_ASSERT(WFALSE == twGetPhasesEnabled(&my_phaseMask, &cfg));
  CU_ASSERT(0x0 == check_stub_pending_signals());
  clear_all_stub_signal();


    memcpy(my_spat_buf,tw_steady_green_ovr_8,   sizeof(tw_steady_green_ovr_8));      
    ret = scs_process_spat(my_spat_buf);
    CU_ASSERT(0 == ret);
    CU_ASSERT(SIG_PHASE_GREEN == spat_info.flexSpat.spatApproach[16+8-1].curSigPhase);
    
    // create a tware blob that exercises all options
        // vehicle R+Y+G, R+Y, R+G, Y+G, R, R+flashing, Y, Y+flashing, G, G+flashing, Dark
        // overlap .. ditto
        // might need 2 blobs to cover all (22)

    // try declaring a twSPATData (my_twdata) .. and manupulating it
    twSPATData my_twdata;
    //my_twdata = (twSPATData)(*tw_steady_green_ovr_8);
    memcpy(&my_twdata,tw_steady_green_ovr_8, sizeof(tw_steady_green_ovr_8)); 
    ret = scs_process_spat((unsigned char *)&my_twdata);
    CU_ASSERT(0 == ret);
    CU_ASSERT(SIG_PHASE_GREEN == spat_info.flexSpat.spatApproach[16+8-1].curSigPhase);


    memset(&my_twdata, 0,   sizeof(tw_steady_green_ovr_8)); 
    my_twdata.version = ntohs(2);
    
    // set vehicle phase 1 = R+Y+G
    my_twdata.redPhasesBitmask[0] |= (1 << (1-1));
    my_twdata.yelPhasesBitmask[0] |= (1 << (1-1));
    my_twdata.grnPhasesBitmask[0] |= (1 << (1-1));
    my_twdata.flsPhasesBitmask[0] &= ~(1 << (1-1));

    // set vehicle phase 2 = R+Y
    my_twdata.redPhasesBitmask[0] |= (1 << (2-1));
    my_twdata.yelPhasesBitmask[0] |= (1 << (2-1));
    my_twdata.grnPhasesBitmask[0] &= ~(1 << (2-1));
    my_twdata.flsPhasesBitmask[0] &= ~(1 << (2-1));    

    // set vehicle phase 3 = R+G
    my_twdata.redPhasesBitmask[0] |= (1 << (3-1));
    my_twdata.yelPhasesBitmask[0] &= ~(1 << (3-1));
    my_twdata.grnPhasesBitmask[0] |= (1 << (3-1));
    my_twdata.flsPhasesBitmask[0] &= ~(1 << (3-1));

    // set vehicle phase 4 = Y+G
    my_twdata.redPhasesBitmask[0] &= ~(1 << (4-1));
    my_twdata.yelPhasesBitmask[0] |= (1 << (4-1));
    my_twdata.grnPhasesBitmask[0] |= (1 << (4-1));
    my_twdata.flsPhasesBitmask[0] &= ~(1 << (4-1));

    // set vehicle phase 5 = steady-red
    my_twdata.redPhasesBitmask[0] |= (1 << (5-1));
    my_twdata.yelPhasesBitmask[0] &= ~(1 << (5-1));
    my_twdata.grnPhasesBitmask[0] &= ~(1 << (5-1));
    my_twdata.flsPhasesBitmask[0] &= ~(1 << (5-1));

    // set vehicle phase 6 = flashing-red
    my_twdata.redPhasesBitmask[0] |= (1 << (6-1));
    my_twdata.yelPhasesBitmask[0] &= ~(1 << (6-1));
    my_twdata.grnPhasesBitmask[0] &= ~(1 << (6-1));
    my_twdata.flsPhasesBitmask[0] |= (1 << (6-1));

    // set vehicle phase 7 = steady-yellow
    my_twdata.redPhasesBitmask[0] &= ~(1 << (7-1));
    my_twdata.yelPhasesBitmask[0] |= (1 << (7-1));
    my_twdata.grnPhasesBitmask[0] &= ~(1 << (7-1));
    my_twdata.flsPhasesBitmask[0] &= ~(1 << (7-1));

    // set vehicle phase 8 = flashing-yellow
    my_twdata.redPhasesBitmask[0] &= ~(1 << (8-1));
    my_twdata.yelPhasesBitmask[0] |= (1 << (8-1));
    my_twdata.grnPhasesBitmask[0] &= ~(1 << (8-1));
    my_twdata.flsPhasesBitmask[0] |= (1 << (8-1));

    // set vehicle phase 9 = steady-green
    my_twdata.redPhasesBitmask[1] &= ~(1 << (9-8-1));
    my_twdata.yelPhasesBitmask[1] &= ~(1 << (9-8-1));
    my_twdata.grnPhasesBitmask[1] |= (1 << (9-8-1));
    my_twdata.flsPhasesBitmask[1] &= ~(1 << (9-8-1));

    // set vehicle phase 10 = flashing-green
    my_twdata.redPhasesBitmask[1] &= ~(1 << (10-8-1));
    my_twdata.yelPhasesBitmask[1] &= ~(1 << (10-8-1));
    my_twdata.grnPhasesBitmask[1] |= (1 << (10-8-1));
    my_twdata.flsPhasesBitmask[1] |= (1 << (10-8-1));

    // set vehicle phase 11 = dark
    my_twdata.redPhasesBitmask[1] &= ~(1 << (11-8-1));
    my_twdata.yelPhasesBitmask[1] &= ~(1 << (11-8-1));
    my_twdata.grnPhasesBitmask[1] &= ~(1 << (11-8-1));
    my_twdata.flsPhasesBitmask[1] &= ~(1 << (11-8-1));

    // set overlap phases similar to vehicle phases
    my_twdata.redOvlpBitmask[0] = my_twdata.redPhasesBitmask[0];
    my_twdata.redOvlpBitmask[1] = my_twdata.redPhasesBitmask[1];
    my_twdata.yelOvlpBitmask[0] = my_twdata.yelPhasesBitmask[0];
    my_twdata.yelOvlpBitmask[1] = my_twdata.yelPhasesBitmask[1];
    my_twdata.grnOvlpBitmask[0] = my_twdata.grnPhasesBitmask[0];
    my_twdata.grnOvlpBitmask[1] = my_twdata.grnPhasesBitmask[1];
    my_twdata.flsOvlpBitmask[0] = my_twdata.flsPhasesBitmask[0];
    my_twdata.flsOvlpBitmask[1] = my_twdata.flsPhasesBitmask[1];

    cfg.localSignalControllerHW = TrafficWare;

    ret = scs_process_spat((unsigned char *)&my_twdata);
    CU_ASSERT(0 == ret);
    // check vehicle phases
    CU_ASSERT(SIG_PHASE_UNKNOWN == spat_info.flexSpat.spatApproach[1-1].curSigPhase);
    CU_ASSERT(SIG_PHASE_UNKNOWN == spat_info.flexSpat.spatApproach[2-1].curSigPhase);
    CU_ASSERT(SIG_PHASE_UNKNOWN == spat_info.flexSpat.spatApproach[3-1].curSigPhase);
    CU_ASSERT(SIG_PHASE_UNKNOWN == spat_info.flexSpat.spatApproach[4-1].curSigPhase);
    CU_ASSERT(SIG_PHASE_RED == spat_info.flexSpat.spatApproach[5-1].curSigPhase);
    CU_ASSERT(SIG_PHASE_FLASHING_RED == spat_info.flexSpat.spatApproach[6-1].curSigPhase);
    CU_ASSERT(SIG_PHASE_YELLOW == spat_info.flexSpat.spatApproach[7-1].curSigPhase);
    CU_ASSERT(SIG_PHASE_FLASHING_YELLOW == spat_info.flexSpat.spatApproach[8-1].curSigPhase);
    CU_ASSERT(SIG_PHASE_GREEN == spat_info.flexSpat.spatApproach[9-1].curSigPhase);
    CU_ASSERT(SIG_PHASE_FLASHING_GREEN == spat_info.flexSpat.spatApproach[10-1].curSigPhase);
    CU_ASSERT(SIG_PHASE_DARK == spat_info.flexSpat.spatApproach[11-1].curSigPhase);

    // check overlap phases
    CU_ASSERT(SIG_PHASE_UNKNOWN == spat_info.flexSpat.spatApproach[1+16-1].curSigPhase);
    CU_ASSERT(SIG_PHASE_UNKNOWN == spat_info.flexSpat.spatApproach[2+16-1].curSigPhase);
    CU_ASSERT(SIG_PHASE_UNKNOWN == spat_info.flexSpat.spatApproach[3+16-1].curSigPhase);
    CU_ASSERT(SIG_PHASE_UNKNOWN == spat_info.flexSpat.spatApproach[4+16-1].curSigPhase);
    CU_ASSERT(SIG_PHASE_RED == spat_info.flexSpat.spatApproach[5+16-1].curSigPhase);
    CU_ASSERT(SIG_PHASE_FLASHING_RED == spat_info.flexSpat.spatApproach[6+16-1].curSigPhase);
    CU_ASSERT(SIG_PHASE_YELLOW == spat_info.flexSpat.spatApproach[7+16-1].curSigPhase);
    CU_ASSERT(SIG_PHASE_FLASHING_YELLOW == spat_info.flexSpat.spatApproach[8+16-1].curSigPhase);
    CU_ASSERT(SIG_PHASE_GREEN == spat_info.flexSpat.spatApproach[9+16-1].curSigPhase);
    CU_ASSERT(SIG_PHASE_FLASHING_GREEN == spat_info.flexSpat.spatApproach[10+16-1].curSigPhase);
    CU_ASSERT(SIG_PHASE_DARK == spat_info.flexSpat.spatApproach[11+16-1].curSigPhase);
    
    // overlap with no vehicle phase data (as described in Trac ticket 2681)
        // zero out vehicle phase 8 .. make sure overlap 8 still parses
    my_twdata.redPhasesBitmask[0] &= ~(1 << (8-1));
    my_twdata.yelPhasesBitmask[0] &= ~(1 << (8-1));
    my_twdata.grnPhasesBitmask[0] &= ~(1 << (8-1));
    my_twdata.flsPhasesBitmask[0] &= ~(1 << (8-1));
    // todo .. maybe TTCs too? .. but I don't think this matters
        // actually, setting them to 1202 might matter
    my_twdata.minPhaseTTCarray[8-1] = ntohs(0);
    my_twdata.maxPhaseTTCarray[8-1] = ntohs(0);

    ret = scs_process_spat((unsigned char *)&my_twdata);
    CU_ASSERT(SIG_PHASE_FLASHING_YELLOW == spat_info.flexSpat.spatApproach[8+16-1].curSigPhase);


    // restore vehicle phase 8, but set TTCs to 1202 for vehicle phase 8
        // set vehicle phase 8 = flashing-yellow
    // then check overlap 8 (which is valid)
    my_twdata.redPhasesBitmask[0] &= ~(1 << (8-1));
    my_twdata.yelPhasesBitmask[0] |= (1 << (8-1));
    my_twdata.grnPhasesBitmask[0] &= ~(1 << (8-1));
    my_twdata.flsPhasesBitmask[0] |= (1 << (8-1));
    my_twdata.minPhaseTTCarray[8-1] = ntohs(1202);
    my_twdata.maxPhaseTTCarray[8-1] = ntohs(1202);

    ret = scs_process_spat((unsigned char *)&my_twdata);
    CU_ASSERT(SIG_PHASE_FLASHING_YELLOW == spat_info.flexSpat.spatApproach[8+16-1].curSigPhase);
#endif
    
  CU_ASSERT(0x0 == check_stub_pending_signals());
  clear_all_stub_signal();
  return;
} /* end of TW */

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void test_scs_process_spat(void)
{
  int32_t ret = I2V_RETURN_OK;
  //wuint32 i=0;
  //int spat_buf_len = 0; 

  /* 
   * Check and clear pending tests
   */
  CU_ASSERT(0x0 == check_stub_pending_signals());
  clear_all_stub_signal();

  cfg.localSignalControllerHW = 0; //ICD
  mainloop = WTRUE;

  memcpy(my_spat_buf,icd_steady_green_ovr_8,sizeof(icd_steady_green_ovr_8));

  ret = scs_process_spat(my_spat_buf);
  CU_ASSERT(-1 == ret);
#if 0
  memcpy(my_spat_buf,icd_flashing_yellow_ovr_8,sizeof(icd_flashing_yellow_ovr_8));
  ret = scs_process_spat(my_spat_buf);
  CU_ASSERT(0 == ret);

  memcpy(my_spat_buf,icd_steady_yellow_ovr_8,  sizeof(icd_steady_yellow_ovr_8));
  ret = scs_process_spat(my_spat_buf);
  CU_ASSERT(0 == ret);

  memcpy(my_spat_buf,icd_steady_red_ovr_8,     sizeof(icd_steady_red_ovr_8));
  ret = scs_process_spat(my_spat_buf);
  CU_ASSERT(0 == ret);
  //make copy for later: semi-good data?
  memcpy(my_icd_spat_buf,my_spat_buf,SPAT_BUF_SIZE);

  my_spat_buf[0] = 0xcd; //ICD_SPAT_TYPE;
  my_spat_buf[1] = 16;   //EXPECTED_NUM_PHASES
  for(i=2;i < SPAT_BUF_SIZE;i++) {
    my_spat_buf[i] = 0xAC;
  }

  ret = scs_process_spat(my_spat_buf);
  CU_ASSERT(-1 == ret);

  for(i=2;i < SPAT_BUF_SIZE;i++) {
    my_spat_buf[i] = 0x0;
  }
  ret = scs_process_spat(my_spat_buf);
  CU_ASSERT(SCS_AOK == ret);

  mainloop = WTRUE;
  set_stub_signal(Signal_icdParseSpat_missing_data); //try again, dont report error
  ret = scs_process_spat(my_spat_buf);
  CU_ASSERT(0 == ret);

  cfg.useMaxTTC = WFALSE;
  cfg.localSignalControllerHW = 0;
  cfg.tomformat = WTRUE; //wrong header
  ret = scs_process_spat(my_spat_buf);
  CU_ASSERT(SCS_AOK == ret);

  cfg.localSignalControllerHW = 0;
  cfg.useMaxTTC = WFALSE; 
  cfg.tomformat = WFALSE;
  ret = scs_process_spat(my_spat_buf);
  CU_ASSERT(SCS_AOK == ret);

  cfg.tomformat = WTRUE;
  ret = scs_process_spat(my_spat_buf);  
  CU_ASSERT(SCS_AOK == ret);

  for(i=2;i < SPAT_BUF_SIZE;i++) {
    my_spat_buf[i] = 0x0;
  }
  ret = scs_process_spat(my_spat_buf);
  CU_ASSERT(SCS_AOK == ret);

  my_spat_buf[0] = 0; //ICD_SPAT_TYPE;
  my_spat_buf[1] = 16;   //EXPECTED_NUM_PHASES
  ret = scs_process_spat(my_spat_buf);
  CU_ASSERT(-1 == ret);

  my_spat_buf[0] = 0xcd; //ICD_SPAT_TYPE;
  my_spat_buf[1] = 0;   //EXPECTED_NUM_PHASES
  ret = scs_process_spat(my_spat_buf);
  CU_ASSERT(SCS_AOK == ret);
#endif
  /* 
   * Check and clear pending tests
   */
  CU_ASSERT(0x0 == check_stub_pending_signals());
  clear_all_stub_signal();

  return;
}
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void test_scsProcessSRM(void)
{
  //SRMMsgType my_SRMMsg;
  int my_srmSocket; 
  int i = 0;

  /* 
   * Check and clear pending tests
   */
  CU_ASSERT(0x0 == check_stub_pending_signals());
  clear_all_stub_signal();

  mainloop = WTRUE;
#if 0
  my_SRMMsg.msgSecond    = 2000;
  my_SRMMsg.msgTimeStamp = 0;

  my_SRMMsg.etaSeconds  = 1000;
  my_SRMMsg.etaMinute   = 0;
#endif
  //TODO no SHM
  //memcpy( &UnitshmPtr->scsSRMData.srm_msg, 
  //        &my_SRMMsg
  //      , sizeof(my_SRMMsg));


  //UnitshmPtr->scsSRMData.srm_len = sizeof(my_SRMMsg);

  cfg.SRMFwdInterface = 0; 

  for(i=0;i<5;i++, cfg.SRMFwdInterface++) {
     scsProcessSRM(&my_srmSocket);
  }
#if 0
  cfg.SRMFwdInterface = 2; //mccain
  my_SRMMsg.etaSeconds  = 0;
  for(i=0;i<60;i++) {

     scsProcessSRM(&my_srmSocket);
     my_SRMMsg.etaSeconds  = i * 1000;
  
     memcpy(  &UnitshmPtr->scsSRMData.srm_msg, 
              &my_SRMMsg
            , sizeof(my_SRMMsg));
  }
#endif
  /* 
   * Check and clear pending tests
   */
  CU_ASSERT(0x0 == check_stub_pending_signals());
  clear_all_stub_signal();

  return;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void test_scsMain(void)
{
  //int i=0;

  /* 
   * Check and clear pending tests
   */
  CU_ASSERT(0x0 == check_stub_pending_signals());
  clear_all_stub_signal();

  mainloop = WTRUE;
  cfg.nontcip = WFALSE;
  scsMain(36969,0); //not -1 so technically valid sockets

  #if 0 // no SHM

  mainloop = WTRUE;
  cfg.nontcip = WFALSE;
  scsMain(0,0); //not -1 so technically valid sockets
  
  mainloop = WTRUE;
  scsMain(-1,0); //not -1 so technically valid sockets
  
  mainloop = WTRUE;
  scsMain(0,-1); //not -1 so technically valid sockets
  
  mainloop = WTRUE;
  scsMain(-1,-1); //not -1 so technically valid sockets
  
  mainloop = WTRUE;
  cfg.ifcSCS = WFALSE;
  scsMain(1,1);

  CU_ASSERT(0x0 == check_stub_pending_signals());
  clear_all_stub_signal();

  mainloop = WTRUE;
  cfg.ifcSCS = WFALSE; /* This turns off SCS interface so nothing will happen. */
  //set_stub_signal(Signal_select_fail);
  scsMain(1,1);
  CU_ASSERT(0x0 == check_stub_pending_signals());
  clear_all_stub_signal();

  mainloop = WTRUE;
  cfg.ifcSCS = WTRUE;
  set_stub_signal(Signal_select_fail);
  scsMain(1,1);
  CU_ASSERT(0x0 == check_stub_pending_signals());
  clear_all_stub_signal();

  mainloop = WTRUE;
  cfg.ifcSCS = WTRUE;
  set_stub_signal(Signal_select_busy);
  scsMain(1,1);
  CU_ASSERT(0x0 == check_stub_pending_signals());
  clear_all_stub_signal();

  mainloop = WTRUE;
  cfg.ifcSCS = WTRUE;
  set_stub_signal(Signal_select_aok);
  scsMain(1,1);
  CU_ASSERT(0x0 == check_stub_pending_signals());
  clear_all_stub_signal();

  mainloop = WTRUE;
  cfg.ifcSCS = WTRUE;
  set_stub_signal(Signal_select_busy);
  scsMain(1,1);
  CU_ASSERT(0x0 == check_stub_pending_signals());
  clear_all_stub_signal();

  mainloop = WTRUE;
  cfg.ifcSCS = WTRUE;
  set_stub_signal(Signal_select_aok);
  scsMain(1,1);
  CU_ASSERT(0x0 == check_stub_pending_signals());
  clear_all_stub_signal();

  mainloop = WTRUE;
  cfg.ifcSCS = WTRUE;
  cfg.tomformat = WFALSE;
  set_stub_signal(Signal_select_aok);
  scsMain(1,0);
  CU_ASSERT(0x0 == check_stub_pending_signals());
  clear_all_stub_signal();

  mainloop = WTRUE;
  cfg.ifcSCS = WTRUE;
  cfg.tomformat = WFALSE;
  cfg.nontcip = WTRUE;
  set_stub_signal(Signal_select_aok);
  scsMain(1,0);
  clear_all_stub_signal();

  mainloop = WTRUE;
  cfg.ifcSCS = WTRUE;
  cfg.tomformat = WFALSE;
  cfg.nontcip = WTRUE;
  set_stub_signal(Signal_select_aok);
  scsMain(1,0);
  CU_ASSERT(0x0 == check_stub_pending_signals());
  clear_all_stub_signal();

  mainloop = WTRUE;
  cfg.ifcSCS = WTRUE;
  cfg.tomformat = WFALSE;
  cfg.nontcip = WTRUE;
  set_stub_signal(Signal_select_aok);
  set_stub_signal_iteration(Signal_DN_FD_ISSET_fail,1);
  scsMain(1,1);
  CU_ASSERT(0x0 == check_stub_pending_signals());
  clear_all_stub_signal();

  mainloop = WTRUE;
  cfg.ifcSCS = WTRUE;
  cfg.tomformat = WFALSE;
  cfg.nontcip = WTRUE;

  set_stub_signal(Signal_select_aok);
  //set_stub_signal_iteration(Signal_DN_FD_ISSET_fail,1);
  scsMain(1,1);

  CU_ASSERT(0x0 == check_stub_pending_signals());
  clear_all_stub_signal();

  mainloop = WTRUE;
  cfg.ifcSCS = WTRUE;
  cfg.tomformat = WFALSE;
  cfg.nontcip = WFALSE;
  cfg.localSignalControllerHW = TrafficWarePromiscuous;

  set_stub_signal(Signal_select_aok);
  set_stub_signal_iteration(Signal_DN_FD_ISSET_fail,1);
  scsMain(1,1);
  CU_ASSERT(0x0 == check_stub_pending_signals());
  clear_all_stub_signal();

  mainloop = WTRUE;
  cfg.ifcSCS = WTRUE;

  cfg.tomformat = WFALSE;
  cfg.nontcip = WFALSE;
  cfg.localSignalControllerHW = TrafficWarePromiscuous;

  set_stub_signal(Signal_select_aok);
  set_stub_signal_iteration(Signal_DN_FD_ISSET_fail,1);
  scsMain(1,1);
  CU_ASSERT(0x0 == check_stub_pending_signals());
  clear_all_stub_signal();

  mainloop = WTRUE;
  cfg.ifcSCS = WTRUE;

  cfg.tomformat = WFALSE;

  set_stub_signal(Signal_select_aok);
  set_stub_signal(Signal_recvfrom_aok); 

  scsMain(1,1);
  CU_ASSERT(0x0 == check_stub_pending_signals());
  clear_all_stub_signal();

  mainloop = WTRUE;
  cfg.ifcSCS = WTRUE;

  cfg.tomformat = WFALSE;

  set_stub_signal(Signal_select_aok);
  set_stub_signal(Signal_recvfrom_fail); 

  scsMain(1,1);
  CU_ASSERT(0x0 == check_stub_pending_signals());
  clear_all_stub_signal();

  //recv() will copy on signal
  my_spat_buf[0] = 0xcd; //ICD_SPAT_TYPE;
  my_spat_buf[1] = 16;   //EXPECTED_NUM_PHASES
  for(i=2;i < SPAT_BUF_SIZE;i++) {
    my_spat_buf[i] = ~i;
  }

  
  mainloop = WTRUE;
  cfg.ifcSCS = WTRUE;

  cfg.nontcip   = WFALSE;
  cfg.tomformat = WFALSE;
  cfg.localSignalControllerHW = 0;

  set_stub_signal(Signal_inet_ntoa_aok);
  set_stub_signal(Signal_select_aok);
  set_stub_signal(Signal_recvfrom_aok); 
  set_stub_signal_iteration(Signal_DN_FD_ISSET_fail,1);
  scsMain(1,1);
  CU_ASSERT(0x0 == check_stub_pending_signals());
  clear_all_stub_signal();

// really obscure coverage case.
//        -: 1995: } else {
//        1: 1996:     havePhaseEnable = icdGetPhaseEnable(cfg.localSignalControllerIP, cfg.snmpPort, cfg.phaseOptsOID, &phaseMask);

  mainloop = WTRUE;
  cfg.ifcSCS = WTRUE;

  cfg.nontcip   = WFALSE;
  cfg.tomformat = WFALSE;
  cfg.localSignalControllerHW = 0;

#if 0
  set_stub_signal(Signal_inet_ntoa_aok);
  set_stub_signal(Signal_select_aok);
  set_stub_signal(Signal_recvfrom_aok); 
  set_stub_signal_iteration(Signal_DN_FD_ISSET_fail,1);
  set_stub_signal(Signal_icdParseSpat_aok);
  scsMain(1,0); 
  CU_ASSERT(0x0 == check_stub_pending_signals());


  mainloop = WTRUE;
  cfg.ifcSCS = WTRUE;

  cfg.nontcip   = WFALSE;
  cfg.tomformat = WFALSE;
  cfg.localSignalControllerHW = 0;

  set_stub_signal(Signal_inet_ntoa_aok);
  set_stub_signal(Signal_select_aok);
  set_stub_signal(Signal_recvfrom_aok); 
  set_stub_signal_iteration(Signal_DN_FD_ISSET_fail,1);
  set_stub_signal(Signal_icdParseSpat_missing_data);
  scsMain(1,0); 
#endif


//        28: 1970:                if (!scsMessage && !gs2Message) {
//        -: 1971:#if defined(MY_UNIT_TEST)
//    #####: 1972:                mainloop = WFALSE;
//        -: 1973:#endif
//    #####: 1974:                    continue;
//        -: 1975:                }


  mainloop = WTRUE;
  cfg.ifcSCS = WTRUE;

  cfg.nontcip   = WFALSE;
  cfg.tomformat = WFALSE;
  cfg.localSignalControllerHW = 0;

  //set_stub_signal(Signal_inet_ntoa_aok);
  set_stub_signal(Signal_select_aok);
  //set_stub_signal(Signal_recvfrom_aok); 
  set_stub_signal(Signal_DN_FD_ISSET_force_fail); //force failure everytime its called. we must cancel signal
  scsMain(1,0); 
  clear_all_stub_signal();
#endif
  /* 
   * Check and clear pending tests
   */
  CU_ASSERT(0x0 == check_stub_pending_signals());
  clear_all_stub_signal();

  return;

}
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void test_scsSNMPMain(void)
{

  /* 
   * Check and clear pending tests
   */
  CU_ASSERT(0x0 == check_stub_pending_signals());
  clear_all_stub_signal();
//TODO: without SHM setup these functions wont work
  mainloop = WTRUE;
  scsSNMPMain();
#if 0
  mainloop = WTRUE;
  cfg.ifcSCS = WFALSE;
  scsSNMPMain();

  mainloop   = WTRUE;
  cfg.ifcSCS = WTRUE;
  set_stub_signal(Signal_icdParseSNMPSpat_aok);
  scsSNMPMain();

  CU_ASSERT(0x0 == check_stub_pending_signals());
  clear_all_stub_signal();



  mainloop   = WTRUE;
  cfg.ifcSCS = WTRUE;
  //set_stub_signal(Signal_icdParseSNMPSpat_aok);
  scsSNMPMain();



  CU_ASSERT(0x0 == check_stub_pending_signals());
  clear_all_stub_signal();


cfg.useMaxTTC = WFALSE;

  mainloop   = WTRUE;
  cfg.ifcSCS = WTRUE;
  scsSNMPMain();


  set_stub_signal(Signal_oid_yellow_phase);

  mainloop   = WTRUE;
  cfg.ifcSCS = WTRUE;
  scsSNMPMain();

  set_stub_signal(Signal_oid_phase_enable);

  mainloop   = WTRUE;
  cfg.ifcSCS = WTRUE;
  scsSNMPMain();


cfg.useMaxTTC = WTRUE;

  CU_ASSERT(0x0 == check_stub_pending_signals());
  clear_all_stub_signal();


  CU_ASSERT(0x0 == check_stub_pending_signals());
  clear_all_stub_signal();

//530:        /* do not retry - phase is considered inactive for this iteration */
//531:        if (0 > i2vSnmpGet(6, p, &retType, &data)) {

  cfg.useMaxTTC = WTRUE;
  mainloop   = WTRUE;
  cfg.ifcSCS = WTRUE;
  set_stub_signal(Signal_oid_phase_number);
  scsSNMPMain();

  CU_ASSERT(0x0 == check_stub_pending_signals());
  clear_all_stub_signal();


//537:        if (retType != I2V_ASN_INTEGER) {
  mainloop   = WTRUE;
  cfg.ifcSCS = WTRUE;
  set_stub_signal(Signal_oid_phase_number_bad_type);
  scsSNMPMain();

  CU_ASSERT(0x0 == check_stub_pending_signals());
  clear_all_stub_signal();

// 544:        if (!((wuint8)data.integer & 0x01)) {

  mainloop   = WTRUE;
  cfg.ifcSCS = WTRUE;
  set_stub_signal(Signal_oid_phase_number_not_active);
  scsSNMPMain();

  CU_ASSERT(0x0 == check_stub_pending_signals());
  clear_all_stub_signal();

// 551:        snprintf(str5, sizeof(str5), "%s.%d", cfg->phaseNumOID, (i+1));
// 552:        /* do not retry - phase is considered inactive for this iteration */

  mainloop   = WTRUE;
  cfg.ifcSCS = WTRUE;
  set_stub_signal(Signal_oid_phase_min);
  scsSNMPMain();

  CU_ASSERT(0x0 == check_stub_pending_signals());
  clear_all_stub_signal();


// 556:        if (retType != I2V_ASN_INTEGER) {

  mainloop   = WTRUE;
  cfg.ifcSCS = WTRUE;
  set_stub_signal(Signal_oid_phase_min_bad_type);
  scsSNMPMain();

  CU_ASSERT(0x0 == check_stub_pending_signals());
  clear_all_stub_signal();

//560:        if (cfg->useMaxTTC) {

  mainloop   = WTRUE;
  cfg.ifcSCS = WTRUE;
  cfg.useMaxTTC = WTRUE;
  scsSNMPMain();

//563:            snprintf(str5, sizeof(str5), "%s.%d", cfg->phaseMinCntdnOID, (i+1));
//564:        }
//565:        /* do not retry - phase is considered inactive for this iteration */
//566:        if (0 > i2vSnmpGet(6, p, &retType, &data)) {

  mainloop   = WTRUE;
  cfg.ifcSCS = WTRUE;
  cfg.useMaxTTC = WTRUE;
  set_stub_signal(Signal_oid_phase_max_count);
  scsSNMPMain();

  CU_ASSERT(0x0 == check_stub_pending_signals());
  clear_all_stub_signal();

 //571:        if (retType != I2V_ASN_INTEGER) { 

  mainloop   = WTRUE;
  cfg.ifcSCS = WTRUE;
  cfg.useMaxTTC = WTRUE;
  set_stub_signal(Signal_oid_phase_max_count_bad_type);
  scsSNMPMain();

  CU_ASSERT(0x0 == check_stub_pending_signals());
  clear_all_stub_signal();


//582:            snprintf(str5, sizeof(str5), "%s.%d", cfg->phaseMaxCntdnOID, (i+1));
//583:        }
//584:        /* do not retry - phase is considered inactive for this iteration */
//585:        if (0 > i2vSnmpGet(6, p, &retType, &data)) {

  mainloop   = WTRUE;
  cfg.ifcSCS = WTRUE;
  cfg.useMaxTTC = WTRUE;
  set_stub_signal(Signal_oid_phase_time_remain);
  scsSNMPMain();

  CU_ASSERT(0x0 == check_stub_pending_signals());
  clear_all_stub_signal();

//590:        if (retType != I2V_ASN_INTEGER) {

  mainloop   = WTRUE;
  cfg.ifcSCS = WTRUE;
  cfg.useMaxTTC = WTRUE;
  set_stub_signal(Signal_oid_phase_time_remain_bad_type);
  scsSNMPMain();



  CU_ASSERT(0x0 == check_stub_pending_signals());
  clear_all_stub_signal();

  mainloop      = WTRUE;
  cfg.ifcSCS    = WTRUE;
  cfg.useMaxTTC = WTRUE;
  set_stub_signal(Signal_snmp_parse_oid);
  scsSNMPMain();

  CU_ASSERT(0x0 == check_stub_pending_signals());
  clear_all_stub_signal();

  /* 
   * Check and clear pending tests
   */
#endif
  CU_ASSERT(0x0 == check_stub_pending_signals());
  clear_all_stub_signal();

  return;
}
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void test_icdParseSpat(void)
{
 int32_t ret = I2V_RETURN_OK;
 wuint32             i = 0;
 i2vSPATDataType my_dest;
 wuint8          my_buf[SPAT_BUF_SIZE];
 WBOOL           useMaxTTC = WFALSE;
 
 /*
  * Check and clear pending tests
  */
 CU_ASSERT(0x0 == check_stub_pending_signals());
 clear_all_stub_signal();
 
 memcpy(my_buf,icd_steady_green_ovr_8, sizeof(icd_steady_green_ovr_8));
 memset(&my_dest   ,0x0,sizeof(my_dest));
 ret = icdParseSpat(&my_dest, my_buf, useMaxTTC);
 CU_ASSERT(14 == ret); // return value from icdParseSpat is not worth checking .. mostly the result of icdParsePhase(last phase of SPaT)
 CU_ASSERT(SIG_PHASE_GREEN == my_dest.spatApproach[16+8-1].curSigPhase);    // overlap 8 == green ?


 memcpy(my_buf,icd_green_ovr_8_without_veh_8, sizeof(icd_green_ovr_8_without_veh_8));
 memset(&my_dest   ,0x0,sizeof(my_dest));
 ret = icdParseSpat(&my_dest, my_buf, useMaxTTC);
 //CU_ASSERT(I2V_RETURN_OK == ret); // return value from icdParseSpat is not worth checking .. mostly the result of icdParsePhase(last phase of SPaT)
 CU_ASSERT(SIG_PHASE_GREEN == my_dest.spatApproach[16+8-1].curSigPhase);    // overlap 8 == green ?
 
 // test intersection status
    // no bits set in TSCBM
 memcpy(my_buf,icd_intersection_status_zeroes, sizeof(icd_intersection_status_zeroes));
 memset(&my_dest   ,0x0,sizeof(my_dest));
 ret = icdParseSpat(&my_dest, my_buf, useMaxTTC);
 //printf("\n .. intersectionStatus: %d\n", my_dest.intersectionStatus);
 CU_ASSERT(0x0000 == my_dest.intersectionStatus);    // intersetionStatus = all 0s
 
    // all bits set in TSCBM
 memcpy(my_buf,icd_intersection_status_zeroes, sizeof(icd_intersection_status_zeroes));
 my_buf[SPAT_INTERSECTION_STATUS_OFFSET] = 0xff;    // 0xff -> intersectionStatus
 memset(&my_dest   ,0x0,sizeof(my_dest));
 ret = icdParseSpat(&my_dest, my_buf, useMaxTTC);
 //printf("\n .. intersectionStatus: %d\n", my_dest.intersectionStatus);
 CU_ASSERT(0x00FF == my_dest.intersectionStatus);    // intersetionStatus = all 1s


 memcpy(my_buf,my_icd_spat_buf,SPAT_BUF_SIZE);

 ret = icdParseSpat(NULL
                   , my_buf
                   , useMaxTTC);

 CU_ASSERT(I2V_RETURN_FAIL == ret);

 ret = icdParseSpat(&my_dest
                   , NULL
                   , useMaxTTC);

 CU_ASSERT(I2V_RETURN_FAIL == ret);

 ret = icdParseSpat( NULL
                   , NULL
                   , useMaxTTC);

 CU_ASSERT(I2V_RETURN_FAIL == ret);

 set_stub_signal(Signal_icdParseSpat_aok);
 ret = icdParseSpat(&my_dest
                   , my_buf
                   , useMaxTTC);

 CU_ASSERT(I2V_RETURN_OK == ret);

 set_stub_signal_iteration(Signal_icdParseSpat_aok,1);
 ret = icdParseSpat(&my_dest
                   , my_buf
                   , useMaxTTC);
 CU_ASSERT(I2V_RETURN_FAIL == ret);


 ret = icdParseSpat(&my_dest
                   , my_buf
                   , useMaxTTC);
 CU_ASSERT(I2V_RETURN_OK == ret);

 set_stub_signal_iteration(Signal_icdParseSpat_missing_data,1); //pointless test?
 ret = icdParseSpat(&my_dest
                   , my_buf
                   , useMaxTTC);
 CU_ASSERT(I2V_RETURN_FAIL == ret);

 ret = icdParseSpat(&my_dest
                   , my_buf
                   , useMaxTTC);
 CU_ASSERT(I2V_RETURN_MISSING_DATA == ret);


 CU_ASSERT(0x0 == check_stub_pending_signals());
 clear_all_stub_signal();


 //431:    if (buf[NUM_PHASES_INDEX] > EXPECTED_NUM_PHASES) {

 my_buf[NUM_PHASES_INDEX] = EXPECTED_NUM_PHASES + 1;

 ret = icdParseSpat(&my_dest
                   , my_buf
                   , useMaxTTC);
 CU_ASSERT(I2V_RETURN_FAIL == ret);


//443:    if(SPAT_MAX_APPROACHES < dest->numApproach) {
 my_buf[CMD_TYPE_INDEX]   = ICD_SPAT_TYPE;
 my_buf[NUM_PHASES_INDEX] = EXPECTED_NUM_PHASES - 1;
 my_dest.numApproach      = SPAT_MAX_APPROACHES + 1;

 ret = icdParseSpat(&my_dest
                   , my_buf
                   , useMaxTTC);
 CU_ASSERT(I2V_RETURN_FAIL == ret);

 //my_phaseMask = 0x0;
 my_buf[CMD_TYPE_INDEX]   = ICD_SPAT_TYPE;
 my_buf[NUM_PHASES_INDEX] = EXPECTED_NUM_PHASES - 1;
 my_dest.numApproach      = SPAT_MAX_APPROACHES - 1;

 ret = icdParseSpat( &my_dest
                     , my_buf
                     , useMaxTTC);

 CU_ASSERT(I2V_RETURN_FAIL == ret);

 //stress test: dont care if returns ok or not just dont break.

 my_buf[CMD_TYPE_INDEX]   = ICD_SPAT_TYPE;
 my_buf[NUM_PHASES_INDEX] = EXPECTED_NUM_PHASES - 1;
 my_dest.numApproach      = SPAT_MAX_APPROACHES - 1;

 for(i=0x0;i<=0xffff;i++) {

   //my_phaseMask = (wuint16)i;

   ret = icdParseSpat( &my_dest
                     , my_buf
                     , useMaxTTC);

   //if(I2V_RETURN_OK == ret) {
   //  printf("\nicdParseSpat return AOK(0x%x)\n",i);
   //  break;
   //}
 }
 //CU_ASSERT(I2V_RETURN_OK == ret);

  /* 
   * Check and clear pending tests
   */
  CU_ASSERT(0x0 == check_stub_pending_signals());
  clear_all_stub_signal();

  return;
}
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void test_icdParseSNMPSpat(void)
{

  int32_t ret = I2V_RETURN_OK;
  i2vSPATDataType my_dest;
  wuint8          my_buf[SPAT_BUF_SIZE];

  /* 
   * Check and clear pending tests
   */
  CU_ASSERT(0x0 == check_stub_pending_signals());
  clear_all_stub_signal();

   memcpy(my_buf,my_wsu_spat_buf,SPAT_BUF_SIZE);

   ret = icdParseSNMPSpat(NULL, &cfg);
   CU_ASSERT(I2V_RETURN_FAIL == ret);

   ret = icdParseSNMPSpat(&my_dest, NULL);
   CU_ASSERT(I2V_RETURN_FAIL == ret);

   ret = icdParseSNMPSpat(NULL, NULL);
   CU_ASSERT(I2V_RETURN_FAIL == ret);

   ret = icdParseSNMPSpat(&my_dest, &cfg);
   CU_ASSERT(I2V_RETURN_OK == ret);

   //Signal SNMP stub we want data
   cfg.useMaxTTC = WFALSE;
   //set_stub_signal(Signal_icdParseSNMPSpat_aok);
   my_dest.numApproach = SPAT_MAX_APPROACHES + 1;
   ret = icdParseSNMPSpat(&my_dest, &cfg);
   CU_ASSERT(I2V_RETURN_OK == ret);

   my_dest.numApproach = SPAT_MAX_APPROACHES;


   ret = icdParseSNMPSpat(&my_dest, &cfg);
   CU_ASSERT(I2V_RETURN_OK == ret);
  
   set_stub_signal(Signal_snmp_open);
   set_stub_signal(Signal_snmp_parse_args_aok);
   ss = NULL;
   ret = icdParseSNMPSpat(&my_dest, &cfg);
   CU_ASSERT(I2V_RETURN_MISSING_DATA == ret);

//foo
#if 0
   set_stub_signal(Signal_snmp_parse_args_aok);
   set_stub_signal(Signal_snmp_parse_oid);
   ret = icdParseSNMPSpat(&my_dest, &cfg);
   CU_ASSERT(I2V_SNMP_INVALID_OID == ret);
#endif

  /* 
   * Check and clear pending tests
   */
  CU_ASSERT(0x0 == check_stub_pending_signals());
  clear_all_stub_signal();

  return;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void test_scsSendSRM_Raw(void)
{
  int my_srmSocket; 
  wuint8 my_raw_data[SPAT_BUF_SIZE];

  /* 
   * Check and clear pending tests
   */
  CU_ASSERT(0x0 == check_stub_pending_signals());
  clear_all_stub_signal();

  scsSendSRM_Raw(NULL, (wuint8 *)&my_raw_data, 255);
  scsSendSRM_Raw(&my_srmSocket, NULL, 128);
  scsSendSRM_Raw(&my_srmSocket, (wuint8 *)&my_raw_data, 128);

  scsSendSRM(&my_srmSocket, NULL);

  /* 
   * Check and clear pending tests
   */
  CU_ASSERT(0x0 == check_stub_pending_signals());
  clear_all_stub_signal();

  return;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void test_scsSRMMC(void)
{
  int i=0,j=0;
  /* 
   * Check and clear pending tests
   */
  CU_ASSERT(0x0 == check_stub_pending_signals());
  clear_all_stub_signal();

  //16+1 approaches. 300 seconds max away
  for(i=0,j=0;i<=17;i++) {
    for(j=300;(0 <= j) && (j < 0xFFFF);j--) {
      scsSRMMC(i,j);
    }
  }

  /* 
   * Check and clear pending tests
   */
  CU_ASSERT(0x0 == check_stub_pending_signals());
  clear_all_stub_signal();

  return;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void test_scs_init_tc_sock(void)
{
  int32_t ret = I2V_RETURN_OK;

  /* 
   * Check and clear pending tests
   */
  CU_ASSERT(0x0 == check_stub_pending_signals());
  clear_all_stub_signal();

  set_stub_signal(Signal_socket);
  ret = scs_init_tc_sock();
  CU_ASSERT(-1 == ret);

  set_stub_signal(Signal_bind);
  ret = scs_init_tc_sock();
  CU_ASSERT(-1 == ret);

  /* 
   * Check and clear pending tests
   */
  CU_ASSERT(0x0 == check_stub_pending_signals());
  clear_all_stub_signal();

  return;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void test_scs_init_srm_sock(void)
{
  int32_t ret = I2V_RETURN_OK;

  /* 
   * Check and clear pending tests
   */
  CU_ASSERT(0x0 == check_stub_pending_signals());
  clear_all_stub_signal();


  set_stub_signal(Signal_socket);
  ret = scs_init_srm_sock();
  CU_ASSERT(-1 == ret);

  set_stub_signal(Signal_bind);
  ret = scs_init_srm_sock();
  CU_ASSERT(-1 == ret);

  /* 
   * Check and clear pending tests
   */
  CU_ASSERT(0x0 == check_stub_pending_signals());
  clear_all_stub_signal();

  return;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

void test_icdParsePhase(void)
{

  i2vReturnTypesT   ret = I2V_RETURN_OK;
  i2vSPATDataType   my_dest;
  spatPhase         my_phase;
  phaseGroups       my_group;
  phaseOverlaps     my_overlap;
  WBOOL             useMaxTTC  = WFALSE;
  wuint16           oldmask    = 0x0;
  wuint16           my_newmask = 0x0;

  /* 
   * Check and clear pending tests
   */
  CU_ASSERT(0x0 == check_stub_pending_signals());
  clear_all_stub_signal();

  memset(&my_dest   ,0x0,sizeof(my_dest));
  memset(&my_phase  ,0x0,sizeof(my_phase));
  memset(&my_group  ,0x0,sizeof(my_group));
  memset(&my_overlap,0x0,sizeof(my_overlap));

  /* NULL inputs */
  ret = icdParsePhase( NULL
                      ,NULL
                      ,NULL
                      ,NULL
                      ,useMaxTTC
                      ,oldmask
                      ,NULL);

  CU_ASSERT(I2V_RETURN_FAIL == ret);

  /* Empty case */

  ret = icdParsePhase( &my_dest
                      ,&my_phase
                      ,&my_group
                      ,&my_overlap
                      ,useMaxTTC
                      ,oldmask
                      ,&my_newmask);

   CU_ASSERT(I2V_RETURN_OK == ret);


// 226:        if (!(oldmask & mask)) {

  memset(&my_dest   ,0x0,sizeof(my_dest));
  memset(&my_phase  ,0x0,sizeof(my_phase));
  memset(&my_group  ,0x0,sizeof(my_group));
  memset(&my_overlap,0x0,sizeof(my_overlap));

  my_phase.phaseNum  = 0x6; //mask
  my_group.groupGrns = ntohs(1 << (my_phase.phaseNum - 1));
  my_group.groupReds = ntohs(1 << (my_phase.phaseNum - 1));
  my_group.groupYels = ntohs(1 << (my_phase.phaseNum - 1));
  oldmask            = 0x6; //(1 << (my_phase.phaseNum - 1));

  useMaxTTC          = WFALSE;
  my_newmask         = 0x0;

  ret = icdParsePhase( &my_dest
                      ,&my_phase
                      ,&my_group
                      ,&my_overlap
                      ,useMaxTTC
                      ,oldmask
                      ,&my_newmask);

   CU_ASSERT(I2V_RETURN_OK == ret);


    //wuint32             i = 0;
    // vehicle R+Y+G = SIG_PHASE_UNKNOWN
    my_phase.phaseNum  = 0x6; //mask
    my_group.groupGrns = ntohs(1 << (my_phase.phaseNum - 1));
    my_group.groupReds = ntohs(1 << (my_phase.phaseNum - 1));
    my_group.groupYels = ntohs(1 << (my_phase.phaseNum - 1));
    oldmask            = 0xFFFF;

    useMaxTTC          = WFALSE;
    my_newmask         = 0x0;

    my_dest.numApproach = my_phase.phaseNum - 1;
    ret = icdParsePhase( &my_dest
                        ,&my_phase
                        ,&my_group
                        ,&my_overlap
                        ,useMaxTTC
                        ,oldmask
                        ,&my_newmask);

    CU_ASSERT(SIG_PHASE_UNKNOWN == my_dest.spatApproach[my_dest.numApproach].curSigPhase);
    CU_ASSERT(I2V_RETURN_OK == ret);
    //    printf("\nret=%d\n",ret);
    //
    //    for(i=0;i<32;i++) {
    //        printf("\nmy_dest.spatApproach[%d].curSigPhase=0x%x.",i, my_dest.spatApproach[i].curSigPhase);
    //    } 


    // vehicle R+Y = SIG_PHASE_UNKNOWN
    my_phase.phaseNum  = 0x6; //mask
    my_group.groupGrns = ntohs(0x0);
    my_group.groupReds = ntohs(1 << (my_phase.phaseNum - 1));
    my_group.groupYels = ntohs(1 << (my_phase.phaseNum - 1));
    oldmask            = 0xFFFF;

    useMaxTTC          = WFALSE;
    my_newmask         = 0x0;
    
    my_dest.numApproach = my_phase.phaseNum - 1;
    ret = icdParsePhase( &my_dest
                        ,&my_phase
                        ,&my_group
                        ,&my_overlap
                        ,useMaxTTC
                        ,oldmask
                        ,&my_newmask);

    CU_ASSERT(SIG_PHASE_UNKNOWN == my_dest.spatApproach[my_dest.numApproach].curSigPhase);
    CU_ASSERT(I2V_RETURN_OK == ret);


    // vehicle R+G = SIG_PHASE_UNKNOWN
    my_phase.phaseNum  = 0x6; //mask
    my_group.groupGrns = ntohs(1 << (my_phase.phaseNum - 1));
    my_group.groupReds = ntohs(1 << (my_phase.phaseNum - 1));
    my_group.groupYels = ntohs(0x0);
    oldmask            = 0xFFFF;

    useMaxTTC          = WFALSE;
    my_newmask         = 0x0;
    
    my_dest.numApproach = my_phase.phaseNum - 1;
    ret = icdParsePhase( &my_dest
                        ,&my_phase
                        ,&my_group
                        ,&my_overlap
                        ,useMaxTTC
                        ,oldmask
                        ,&my_newmask);

    CU_ASSERT(SIG_PHASE_UNKNOWN == my_dest.spatApproach[my_dest.numApproach].curSigPhase);
    CU_ASSERT(I2V_RETURN_OK == ret);


    // vehicle Y+G = SIG_PHASE_UNKNOWN
    my_phase.phaseNum  = 0x6; //mask
    my_group.groupGrns = ntohs(1 << (my_phase.phaseNum - 1));
    my_group.groupReds = ntohs(0x0);
    my_group.groupYels = ntohs(1 << (my_phase.phaseNum - 1));
    oldmask            = 0xFFFF;

    useMaxTTC          = WFALSE;
    my_newmask         = 0x0;
    
    my_dest.numApproach = my_phase.phaseNum - 1;
    ret = icdParsePhase( &my_dest
                        ,&my_phase
                        ,&my_group
                        ,&my_overlap
                        ,useMaxTTC
                        ,oldmask
                        ,&my_newmask);

    CU_ASSERT(SIG_PHASE_UNKNOWN == my_dest.spatApproach[my_dest.numApproach].curSigPhase);
    CU_ASSERT(I2V_RETURN_OK == ret);


    // vehicle None = SIG_PHASE_DARK
    my_phase.phaseNum  = 0x6; //mask
    my_group.groupGrns = ntohs(0x0);
    my_group.groupReds = ntohs(0x0);
    my_group.groupYels = ntohs(0x0);
    oldmask            = 0xFFFF;

    useMaxTTC          = WFALSE;
    my_newmask         = 0x0;
    
    my_dest.numApproach = my_phase.phaseNum - 1;
    ret = icdParsePhase( &my_dest
                        ,&my_phase
                        ,&my_group
                        ,&my_overlap
                        ,useMaxTTC
                        ,oldmask
                        ,&my_newmask);

    CU_ASSERT(SIG_PHASE_DARK == my_dest.spatApproach[my_dest.numApproach].curSigPhase);
    CU_ASSERT(I2V_RETURN_OK == ret);


    // vehicle Red
    my_phase.phaseNum  = 0x6; //mask
    my_group.groupGrns = ntohs(0x0);
    my_group.groupReds = ntohs(1 << (my_phase.phaseNum - 1));
    my_group.groupYels = ntohs(0x0);
    oldmask            = 0xFFFF;

    useMaxTTC          = WFALSE;
    my_newmask         = 0x0;
    
    my_dest.numApproach = my_phase.phaseNum - 1;
    ret = icdParsePhase( &my_dest
                        ,&my_phase
                        ,&my_group
                        ,&my_overlap
                        ,useMaxTTC
                        ,oldmask
                        ,&my_newmask);

    CU_ASSERT(SIG_PHASE_RED == my_dest.spatApproach[my_dest.numApproach].curSigPhase);
    CU_ASSERT(I2V_RETURN_OK == ret);


    // vehicle Red + flashing
    my_phase.phaseNum  = 0x6; //mask
    my_group.groupGrns = ntohs(0x0);
    my_group.groupReds = ntohs(1 << (my_phase.phaseNum - 1));
    my_group.groupYels = ntohs(0x0);
    oldmask            = 0xFFFF;

    useMaxTTC          = WFALSE;
    my_newmask         = 0x0;
    
    OverlapMap.FlashOutPhaseStatus = ntohs(1 << (my_phase.phaseNum - 1));
    
    my_dest.numApproach = my_phase.phaseNum - 1;
    ret = icdParsePhase( &my_dest
                        ,&my_phase
                        ,&my_group
                        ,&my_overlap
                        ,useMaxTTC
                        ,oldmask
                        ,&my_newmask);

    CU_ASSERT(SIG_PHASE_FLASHING_RED == my_dest.spatApproach[my_dest.numApproach].curSigPhase);
    CU_ASSERT(I2V_RETURN_OK == ret);


    // vehicle Yellow
    my_phase.phaseNum  = 0x6; //mask
    my_group.groupGrns = ntohs(0x0);
    my_group.groupReds = ntohs(0x0);
    my_group.groupYels = ntohs(1 << (my_phase.phaseNum - 1));
    oldmask            = 0xFFFF;

    useMaxTTC          = WFALSE;
    my_newmask         = 0x0;

    OverlapMap.FlashOutPhaseStatus = ntohs(0x0);
    
    my_dest.numApproach = my_phase.phaseNum - 1;
    ret = icdParsePhase( &my_dest
                        ,&my_phase
                        ,&my_group
                        ,&my_overlap
                        ,useMaxTTC
                        ,oldmask
                        ,&my_newmask);

    CU_ASSERT(SIG_PHASE_YELLOW == my_dest.spatApproach[my_dest.numApproach].curSigPhase);
    CU_ASSERT(I2V_RETURN_OK == ret);


    // vehicle Yellow + flashing
    my_phase.phaseNum  = 0x6; //mask
    my_group.groupGrns = ntohs(0x0);
    my_group.groupReds = ntohs(0x0);
    my_group.groupYels = ntohs(1 << (my_phase.phaseNum - 1));
    oldmask            = 0xFFFF;

    useMaxTTC          = WFALSE;
    my_newmask         = 0x0;

    OverlapMap.FlashOutPhaseStatus = ntohs(1 << (my_phase.phaseNum - 1));
    
    my_dest.numApproach = my_phase.phaseNum - 1;
    ret = icdParsePhase( &my_dest
                        ,&my_phase
                        ,&my_group
                        ,&my_overlap
                        ,useMaxTTC
                        ,oldmask
                        ,&my_newmask);

    CU_ASSERT(SIG_PHASE_FLASHING_YELLOW == my_dest.spatApproach[my_dest.numApproach].curSigPhase);
    CU_ASSERT(I2V_RETURN_OK == ret);


    // vehicle Green
    my_phase.phaseNum  = 0x6; //mask
    my_group.groupGrns = ntohs(1 << (my_phase.phaseNum - 1));
    my_group.groupReds = ntohs(0x0);
    my_group.groupYels = ntohs(0x0);
    oldmask            = 0xFFFF;

    useMaxTTC          = WFALSE;
    my_newmask         = 0x0;

    OverlapMap.FlashOutPhaseStatus = ntohs(0x0);
    
    my_dest.numApproach = my_phase.phaseNum - 1;
    ret = icdParsePhase( &my_dest
                        ,&my_phase
                        ,&my_group
                        ,&my_overlap
                        ,useMaxTTC
                        ,oldmask
                        ,&my_newmask);

    CU_ASSERT(SIG_PHASE_GREEN == my_dest.spatApproach[my_dest.numApproach].curSigPhase);
    CU_ASSERT(I2V_RETURN_OK == ret);


    // vehicle Green + flashing
    my_phase.phaseNum  = 0x6; //mask
    my_group.groupGrns = ntohs(1 << (my_phase.phaseNum - 1));
    my_group.groupReds = ntohs(0x0);
    my_group.groupYels = ntohs(0x0);
    oldmask            = 0xFFFF;

    useMaxTTC          = WFALSE;
    my_newmask         = 0x0;

    OverlapMap.FlashOutPhaseStatus = ntohs(1 << (my_phase.phaseNum - 1));
    
    my_dest.numApproach = my_phase.phaseNum - 1;
    ret = icdParsePhase( &my_dest
                        ,&my_phase
                        ,&my_group
                        ,&my_overlap
                        ,useMaxTTC
                        ,oldmask
                        ,&my_newmask);

    CU_ASSERT(SIG_PHASE_FLASHING_GREEN == my_dest.spatApproach[my_dest.numApproach].curSigPhase);
    CU_ASSERT(I2V_RETURN_OK == ret);




    // overlap R+Y+G = SIG_PHASE_UNKNOWN
    my_phase.phaseNum  = 0x6; //mask
    my_overlap.OverlapGrns = ntohs(1 << (my_phase.phaseNum - 1));
    my_overlap.OverlapReds = ntohs(1 << (my_phase.phaseNum - 1));
    my_overlap.OverlapYels = ntohs(1 << (my_phase.phaseNum - 1));
    oldmask            = 0xFFFF;

    useMaxTTC          = WFALSE;
    my_newmask         = 0x0;

    my_dest.numApproach = my_phase.phaseNum - 1;
    ret = icdParsePhase( &my_dest
                        ,&my_phase
                        ,&my_group
                        ,&my_overlap
                        ,useMaxTTC
                        ,oldmask
                        ,&my_newmask);

    CU_ASSERT(SIG_PHASE_UNKNOWN == my_dest.spatApproach[my_dest.numApproach+16].curSigPhase);
    CU_ASSERT(I2V_RETURN_OK == ret);


    // overlap R+Y = SIG_PHASE_UNKNOWN
    my_phase.phaseNum  = 0x6; //mask
    my_overlap.OverlapGrns = ntohs(0x0);
    my_overlap.OverlapReds = ntohs(1 << (my_phase.phaseNum - 1));
    my_overlap.OverlapYels = ntohs(1 << (my_phase.phaseNum - 1));
    oldmask            = 0xFFFF;

    useMaxTTC          = WFALSE;
    my_newmask         = 0x0;

    my_dest.numApproach = my_phase.phaseNum - 1;
    ret = icdParsePhase( &my_dest
                        ,&my_phase
                        ,&my_group
                        ,&my_overlap
                        ,useMaxTTC
                        ,oldmask
                        ,&my_newmask);

    CU_ASSERT(SIG_PHASE_UNKNOWN == my_dest.spatApproach[my_dest.numApproach+16].curSigPhase);
    CU_ASSERT(I2V_RETURN_OK == ret);


    // overlap R+G = SIG_PHASE_UNKNOWN
    my_phase.phaseNum  = 0x6; //mask
    my_overlap.OverlapGrns = ntohs(1 << (my_phase.phaseNum - 1));
    my_overlap.OverlapReds = ntohs(1 << (my_phase.phaseNum - 1));
    my_overlap.OverlapYels = ntohs(0x0);
    oldmask            = 0xFFFF;

    useMaxTTC          = WFALSE;
    my_newmask         = 0x0;

    my_dest.numApproach = my_phase.phaseNum - 1;
    ret = icdParsePhase( &my_dest
                        ,&my_phase
                        ,&my_group
                        ,&my_overlap
                        ,useMaxTTC
                        ,oldmask
                        ,&my_newmask);

    CU_ASSERT(SIG_PHASE_UNKNOWN == my_dest.spatApproach[my_dest.numApproach+16].curSigPhase);
    CU_ASSERT(I2V_RETURN_OK == ret);


    // overlap Y+G = SIG_PHASE_UNKNOWN
    my_phase.phaseNum  = 0x6; //mask
    my_overlap.OverlapGrns = ntohs(1 << (my_phase.phaseNum - 1));
    my_overlap.OverlapReds = ntohs(0x0);
    my_overlap.OverlapYels = ntohs(1 << (my_phase.phaseNum - 1));
    oldmask            = 0xFFFF;

    useMaxTTC          = WFALSE;
    my_newmask         = 0x0;

    my_dest.numApproach = my_phase.phaseNum - 1;
    ret = icdParsePhase( &my_dest
                        ,&my_phase
                        ,&my_group
                        ,&my_overlap
                        ,useMaxTTC
                        ,oldmask
                        ,&my_newmask);

    CU_ASSERT(SIG_PHASE_UNKNOWN == my_dest.spatApproach[my_dest.numApproach+16].curSigPhase);
    CU_ASSERT(I2V_RETURN_OK == ret);


    // overlap None = SIG_PHASE_DARK
    my_phase.phaseNum  = 0x6; //mask
    my_overlap.OverlapGrns = ntohs(0x0);
    my_overlap.OverlapReds = ntohs(0x0);
    my_overlap.OverlapYels = ntohs(0x0);
    oldmask            = 0xFFFF;

    useMaxTTC          = WFALSE;
    my_newmask         = 0x0;

    my_dest.numApproach = my_phase.phaseNum - 1;
    ret = icdParsePhase( &my_dest
                        ,&my_phase
                        ,&my_group
                        ,&my_overlap
                        ,useMaxTTC
                        ,oldmask
                        ,&my_newmask);

    CU_ASSERT(SIG_PHASE_DARK == my_dest.spatApproach[my_dest.numApproach+16].curSigPhase);
    CU_ASSERT(I2V_RETURN_OK == ret);


    // overlap Red
    my_phase.phaseNum  = 0x6; //mask
    my_overlap.OverlapGrns = ntohs(0x0);
    my_overlap.OverlapReds = ntohs(1 << (my_phase.phaseNum - 1));
    my_overlap.OverlapYels = ntohs(0x0);
    oldmask            = 0xFFFF;

    useMaxTTC          = WFALSE;
    my_newmask         = 0x0;

    OverlapMap.FlashOutOvlpStatus = ntohs(0x0);

    my_dest.numApproach = my_phase.phaseNum - 1;
    ret = icdParsePhase( &my_dest
                        ,&my_phase
                        ,&my_group
                        ,&my_overlap
                        ,useMaxTTC
                        ,oldmask
                        ,&my_newmask);

    CU_ASSERT(SIG_PHASE_RED == my_dest.spatApproach[my_dest.numApproach+16].curSigPhase);
    CU_ASSERT(I2V_RETURN_OK == ret);


    // overlap Red + flashing
    my_phase.phaseNum  = 0x6; //mask
    my_overlap.OverlapGrns = ntohs(0x0);
    my_overlap.OverlapReds = ntohs(1 << (my_phase.phaseNum - 1));
    my_overlap.OverlapYels = ntohs(0x0);
    oldmask            = 0xFFFF;

    useMaxTTC          = WFALSE;
    my_newmask         = 0x0;

    OverlapMap.FlashOutOvlpStatus = ntohs(1 << (my_phase.phaseNum - 1));

    my_dest.numApproach = my_phase.phaseNum - 1;
    ret = icdParsePhase( &my_dest
                        ,&my_phase
                        ,&my_group
                        ,&my_overlap
                        ,useMaxTTC
                        ,oldmask
                        ,&my_newmask);

    CU_ASSERT(SIG_PHASE_FLASHING_RED == my_dest.spatApproach[my_dest.numApproach+16].curSigPhase);
    CU_ASSERT(I2V_RETURN_OK == ret);


    // overlap Yellow
    my_phase.phaseNum  = 0x6; //mask
    my_overlap.OverlapGrns = ntohs(0x0);
    my_overlap.OverlapReds = ntohs(0x0);
    my_overlap.OverlapYels = ntohs(1 << (my_phase.phaseNum - 1));
    oldmask            = 0xFFFF;

    useMaxTTC          = WFALSE;
    my_newmask         = 0x0;

    OverlapMap.FlashOutOvlpStatus = ntohs(0x0);

    my_dest.numApproach = my_phase.phaseNum - 1;
    ret = icdParsePhase( &my_dest
                        ,&my_phase
                        ,&my_group
                        ,&my_overlap
                        ,useMaxTTC
                        ,oldmask
                        ,&my_newmask);

    CU_ASSERT(SIG_PHASE_YELLOW == my_dest.spatApproach[my_dest.numApproach+16].curSigPhase);
    CU_ASSERT(I2V_RETURN_OK == ret);


    // overlap Yellow + flashing
    my_phase.phaseNum  = 0x6; //mask
    my_overlap.OverlapGrns = ntohs(0x0);
    my_overlap.OverlapReds = ntohs(0x0);
    my_overlap.OverlapYels = ntohs(1 << (my_phase.phaseNum - 1));
    oldmask            = 0xFFFF;

    useMaxTTC          = WFALSE;
    my_newmask         = 0x0;

    OverlapMap.FlashOutOvlpStatus = ntohs(1 << (my_phase.phaseNum - 1));

    my_dest.numApproach = my_phase.phaseNum - 1;
    ret = icdParsePhase( &my_dest
                        ,&my_phase
                        ,&my_group
                        ,&my_overlap
                        ,useMaxTTC
                        ,oldmask
                        ,&my_newmask);

    CU_ASSERT(SIG_PHASE_FLASHING_YELLOW == my_dest.spatApproach[my_dest.numApproach+16].curSigPhase);
    CU_ASSERT(I2V_RETURN_OK == ret);


    // overlap Green
    my_phase.phaseNum  = 0x6; //mask
    my_overlap.OverlapGrns = ntohs(1 << (my_phase.phaseNum - 1));
    my_overlap.OverlapReds = ntohs(0x0);
    my_overlap.OverlapYels = ntohs(0x0);
    oldmask            = 0xFFFF;

    useMaxTTC          = WFALSE;
    my_newmask         = 0x0;

    OverlapMap.FlashOutOvlpStatus = ntohs(0x0);

    my_dest.numApproach = my_phase.phaseNum - 1;
    ret = icdParsePhase( &my_dest
                        ,&my_phase
                        ,&my_group
                        ,&my_overlap
                        ,useMaxTTC
                        ,oldmask
                        ,&my_newmask);

    CU_ASSERT(SIG_PHASE_GREEN == my_dest.spatApproach[my_dest.numApproach+16].curSigPhase);
    CU_ASSERT(I2V_RETURN_OK == ret);


    // overlap Green + flashing
    my_phase.phaseNum  = 0x6; //mask
    my_overlap.OverlapGrns = ntohs(1 << (my_phase.phaseNum - 1));
    my_overlap.OverlapReds = ntohs(0x0);
    my_overlap.OverlapYels = ntohs(0x0);
    oldmask            = 0xFFFF;

    useMaxTTC          = WFALSE;
    my_newmask         = 0x0;

    OverlapMap.FlashOutOvlpStatus = ntohs(1 << (my_phase.phaseNum - 1));

    my_dest.numApproach = my_phase.phaseNum - 1;
    ret = icdParsePhase( &my_dest
                        ,&my_phase
                        ,&my_group
                        ,&my_overlap
                        ,useMaxTTC
                        ,oldmask
                        ,&my_newmask);

    CU_ASSERT(SIG_PHASE_FLASHING_GREEN == my_dest.spatApproach[my_dest.numApproach+16].curSigPhase);
    CU_ASSERT(I2V_RETURN_OK == ret);


    // TTCs
    my_phase.phaseNum  = 0x6; //mask
    my_overlap.OverlapGrns = ntohs(0x0);
    my_overlap.OverlapReds = ntohs(0x0);
    my_overlap.OverlapYels = ntohs(0x0);
    oldmask            = 0xFFFF;

    useMaxTTC          = WFALSE;
    my_newmask         = 0x0;

    OverlapMap.FlashOutOvlpStatus = ntohs(0x0);

    my_phase.vehMinTimeChange = ntohs(3);
    my_phase.vehMaxTimeChange = ntohs(30);
    my_phase.ovlpMinTimeChange = ntohs(4);
    my_phase.ovlpMaxTimeChange = ntohs(40);
    //my_phase.pedMinTimeChange = ntohs(3);     // not used .. yet
    //my_phase.pedMaxTimeChange = ntohs(30);

    my_dest.numApproach = my_phase.phaseNum - 1;
    ret = icdParsePhase( &my_dest
                        ,&my_phase
                        ,&my_group
                        ,&my_overlap
                        ,useMaxTTC
                        ,oldmask
                        ,&my_newmask);

    CU_ASSERT(3 == my_dest.spatApproach[my_dest.numApproach].timeNextPhase);
    CU_ASSERT(30 == my_dest.spatApproach[my_dest.numApproach].secondaryTimeNextPhase);
    CU_ASSERT(4 == my_dest.spatApproach[my_dest.numApproach+16].timeNextPhase);
    CU_ASSERT(40 == my_dest.spatApproach[my_dest.numApproach+16].secondaryTimeNextPhase);
    CU_ASSERT(I2V_RETURN_OK == ret);


  /* 
   * Check and clear pending tests
   */
  CU_ASSERT(0x0 == check_stub_pending_signals());
  clear_all_stub_signal();

  return;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void test_scs(void)
{
  int32_t ret = I2V_RETURN_OK;

  /* 
   * Check and clear pending tests
   */
  CU_ASSERT(0x0 == check_stub_pending_signals());
  clear_all_stub_signal();

  /* 
   * Setup I2V SHM flavor for this test.
   * Make sure module statics are reset per iteration.
   */ 
  if ((UnitshmPtr = wsu_share_init(sizeof(i2vShmMasterT), I2V_SHM_PATH)) == NULL) {
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME," Main shared memory init failed\n");
      return;
  }
  CU_ASSERT(I2V_RETURN_OK == i2v_setupSHM_Default(UnitshmPtr)); /* Should reflect i2v.conf. */
  sleep(1); /* Give SHM a chance to digest. */
  //
  //Test main(): Run at least once successfully to setup shm and scs.conf.
  //           : Maybe a good idea to run nominal case first? Testing can mess with setup makinb it tougher.

  ret = my_main(); 
  CU_ASSERT(0 == ret);
  i2v_setupSHM_Clean(UnitshmPtr); /* Close SHM. */

  /* Beyond here there is no SHM for functions to use. */
  mainloop  = WTRUE; /* We need to make this true each time just in case. */

/** ICDDECODER.C: Not exhaustive **/
 
  //
  test_icdParsePhase();

  //
  test_icdParseSpat();

  //
  test_icdParseSNMPSpat(); //full coverage in test_scsSNMPMain()

  //
  //Test main(): Run at least once successfully to setup shm inside scs.c module
  //           : Ideally each test unit would do it sperately but this is quicker way.

/** i2v_snmp_client.c **/
  //
  //
  //
  i2vSnmpParseDefault(1, NULL, 1);


/** SCS.C: Exhaustive **/
  if ((UnitshmPtr = wsu_share_init(sizeof(i2vShmMasterT), I2V_SHM_PATH)) == NULL) {
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME," Main shared memory init failed\n");
      return;
  }
  CU_ASSERT(I2V_RETURN_OK == i2v_setupSHM_Default(UnitshmPtr)); /* Should reflect i2v.conf. */
  sleep(1); /* Give SHM a chance to digest. */
  test_scs_main();
  i2v_setupSHM_Clean(UnitshmPtr); /* Close SHM. */

  /* Beyond here there is no SHM for functions to use. */
  mainloop = WTRUE; /* We need to make this true each time just in case. */

  //For coverage.
  scs_sighandler(1);
  //
  test_scs_process_spat();

  // TWAREDECODER.C: Not exhaustive
  test_twaredecoder();

  //
  test_scsProcessSRM();

  //
  test_scsMain(); 

  //
  test_scsSNMPMain();

  //
  test_scsSendSRM_Raw();

  //Spews lots debug: stress test
  test_scsSRMMC(); 

  //
  test_scs_init_tc_sock();

  //
  test_scs_init_srm_sock();


/* Stress Test: Shouldn't run out of network or other resources?
 *            : Doesn't or shouldn't help with coverage.
 *              Disabled see Trac #2815
 */
#if 0
  int32_t i;
  for(i=0,ret=0;(i<100) && (0 == ret);i++) {

    if(0 == (i%2)) { 
      //set_stub_signal(Signal_toggle_tomformat);  
    }
    if ((UnitshmPtr = wsu_share_init(sizeof(i2vShmMasterT), I2V_SHM_PATH)) == NULL) {
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME," Main shared memory init failed\n");
      CU_ASSERT(1 == 0); 
      return;
    }
    CU_ASSERT(I2V_RETURN_OK == i2v_setupSHM_Default(UnitshmPtr)); /* Should reflect i2v.conf. */
    ret = my_main();
    i2v_setupSHM_Clean(UnitshmPtr); /* Close SHM. */

    CU_ASSERT(0x0 == check_stub_pending_signals());
    CU_ASSERT(0 == ret);
  }

  if((i<100) || (0 != ret)) {
    printf("\nmy_main fail!i=%d,ret=%d.\n",i,ret);
  }
#endif

  /* 
   * Setup I2V SHM flavor for this test.
   * Make sure module statics are reset per iteration.
   */ 
  if ((UnitshmPtr = wsu_share_init(sizeof(i2vShmMasterT), I2V_SHM_PATH)) == NULL) {
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME," Main shared memory init failed\n");
      return;
  }
  CU_ASSERT(I2V_RETURN_OK == i2v_setupSHM_Default(UnitshmPtr)); /* Should reflect i2v.conf. */
  sleep(1); /* Give SHM a chance to digest. */
  ret = my_main(); 
  CU_ASSERT(0 == ret);
  i2v_setupSHM_Clean(UnitshmPtr); /* Close SHM. */

  /* 
   * Check and clear pending tests
   */
  CU_ASSERT(0x0 == check_stub_pending_signals());
  clear_all_stub_signal();

}
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

void test_main(void)
{
  test_scs(); // Will test everything under ./scs
  return;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
int init_suite(void) 
{ 
  return 0; 
}
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
int clean_suite(void) 
{ 
  return 0; 
}
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/**************************************************************************************************
* MAIN()
***************************************************************************************************/
int main (void)
{

  CU_pSuite    pSuite1 = NULL;
  CU_ErrorCode ret     = CUE_SUCCESS; /* SUCCESS till proven FAIL */

  init_stub_control();

  ret = CU_initialize_registry();

  if(CUE_SUCCESS == ret) {
      if(NULL == (pSuite1 = CU_add_suite("Basic_Test_Suite1", init_suite, clean_suite))) {
          ret = ~CUE_SUCCESS;
      }
  }

  if(CUE_SUCCESS == ret) {
      if ((NULL == CU_add_test(pSuite1, "\ntest_main...",test_main))) {
          ret = ~CUE_SUCCESS;
      }
  }

  if(CUE_SUCCESS == ret) {
     printf("\nCUnit CU_basic_run_tests...\n");
     ret = CU_basic_run_tests();
  } 

  fflush(stdout);

  if(CUE_SUCCESS != ret) {
      printf("\nCUnit Exception: %d [%s]\n",CU_get_error(),CU_get_error_msg());
  } else {
      printf("\nCUnit Complete.\n");
  }

  CU_cleanup_registry();

  return CU_get_error();

}  /* END of MAIN() */

/**************************************************************************************************
* End of Module
***************************************************************************************************/
