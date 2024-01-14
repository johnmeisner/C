/**************************************************************************
 *                                                                        *
 *     File Name:  scs.c  (Signal Controller Service)                     *
 *     Author:                                                            *
 *         DENSO International America, Inc.                              *
 *         North America Research Laboratory, California Office           *
 *         3252 Business Park Drive                                       *
 *         Vista, CA 92081                                                *
 *                                                                        *
 **************************************************************************/

#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <errno.h>
#include <time.h>
#include "wsu_util.h"
#include "wsu_shm.h"
#include "i2v_general.h"
#include "i2v_util.h"
#include "i2v_shm_master.h"
#include "scs.h"
#include "conf_table.h"
#include "icddecoder.h"
#include "twaredecoder.h"

/* Moved headers here for splint. Some conflict with 64bit types? */
#include <net/if.h>
#include <ifaddrs.h>    /* To get Mac address */
#include <linux/if_packet.h>

#if defined(MY_UNIT_TEST)
#include "stubs.h"
#define MAIN my_main
#else
#define MAIN main
#define dn_setsockopt     setsockopt
#define dn_bind           bind
#define dn_pthread_create pthread_create
#define dn_select    select
#define dn_recvfrom  recvfrom
#define dn_inet_aton inet_aton
#define dn_inet_ntoa inet_ntoa
#define SOCKET socket
#define DN_FD_ZERO  FD_ZERO
#define DN_FD_SET   FD_SET
#define DN_FD_CLR   FD_CLR
#define DN_FD_ISSET FD_ISSET
#endif

#if defined(EXTRA_DEBUG)
#define OUTPUT_MODULUS  600 /* 30 Seconds. */
#else
#define OUTPUT_MODULUS  24240 /* 1200 Seconds. */
#endif

/* DO NOT ship enabled. */
#if 0
#define FYA_TEST
#endif

#if defined(FYA_TEST)
#include "twaredecoder.h"
STATIC uint32_t my_fya_test = 0; //rolling counter so whatever it starts at is cool

/* steady-green overlap 8. */ 
STATIC uint8_t icd_steady_green_ovr_8[SPAT_BUF_SIZE] = {0xCD,0x10,0x01,0x02,0x3A,0x02,0x59,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x02,0x02,0x8A,0x03,0x17,0x02,0x8A,0x03,0x17,0x00,0x00,0x00,0x00,0x03,0x00,0x14,0x00,0x33,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x04,0x00,0x3C,0x00,0x5B,0x00,0x3C,0x00,0x5B,0x00,0x00,0x00,0x00,0x05,0x02,0x3A,0x02,0x59,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x06,0x02,0x8A,0x03,0x17,0x02,0x8A,0x03,0x17,0x00,0x00,0x00,0x00,0x07,0x00,0x14,0x00,0x33,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x08,0x00,0x3C,0x00,0x5B,0x00,0x3C,0x00,0x5B,0x00,0x14,0x00,0x33,0x09,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0A,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0B,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0D,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0E,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0F,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xBB,0x00,0x00,0x00,0x44,0x00,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0x00,0x00,0x00,0x00,0x00,0xFE,0x11,0x33,0x00,0xE6,0x40,0x01,0x8D,0x00,0x00,0x00,0x00};

/* flashing-yellow overlap 8. */
STATIC uint8_t icd_flashing_yellow_ovr_8[SPAT_BUF_SIZE] = {0xCD,0x10,0x01,0x02,0x0A,0x02,0x0A,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x02,0x02,0x5A,0x02,0xC8,0x02,0x5A,0x02,0xC8,0x00,0x00,0x00,0x00,0x03,0x00,0x02,0x00,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x04,0x00,0x0C,0x00,0x0C,0x00,0x0C,0x00,0x0C,0x00,0x00,0x00,0x00,0x05,0x02,0x0A,0x02,0x0A,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x06,0x02,0x5A,0x02,0xC8,0x02,0x5A,0x02,0xC8,0x00,0x00,0x00,0x00,0x07,0x00,0x02,0x00,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x08,0x00,0x0C,0x00,0x0C,0x00,0x0C,0x00,0x0C,0x00,0x13,0x00,0x13,0x09,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0A,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0B,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0D,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0E,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0F,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xBB,0x00,0x44,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0x00,0x00,0x00,0x00,0x00,0x80,0x00,0xFE,0x10,0x82,0x00,0xE6,0x48,0x02,0x4B,0x00,0x00,0x00,0x00};

/* steady-yellow overlap 8. */
STATIC uint8_t icd_steady_yellow_ovr_8[SPAT_BUF_SIZE] = {0xCD,0x10,0x01,0x02,0x0A,0x02,0x0A,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x02,0x02,0x5A,0x02,0xC8,0x02,0x5A,0x02,0xC8,0x00,0x00,0x00,0x00,0x03,0x00,0x02,0x00,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x04,0x00,0x0C,0x00,0x0C,0x00,0x0C,0x00,0x0C,0x00,0x00,0x00,0x00,0x05,0x02,0x0A,0x02,0x0A,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x06,0x02,0x5A,0x02,0xC8,0x02,0x5A,0x02,0xC8,0x00,0x00,0x00,0x00,0x07,0x00,0x02,0x00,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x08,0x00,0x0C,0x00,0x0C,0x00,0x0C,0x00,0x0C,0x00,0x03,0x00,0x03,0x09,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0A,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0B,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0D,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0E,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0F,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xBB,0x00,0x44,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFE,0x10,0x82,0x00,0xE6,0x48,0x02,0x4B,0x00,0x00,0x00,0x00};

/* overlap 8 as a (steady-red) FYA. */
STATIC uint8_t icd_steady_red_ovr_8[SPAT_BUF_SIZE] = {0xCD,0x10,0x01,0x00,0x08,0x00,0x08,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x02,0x00,0x58,0x00,0xC6,0x00,0x58,0x00,0xC6,0x00,0x00,0x00,0x00,0x03,0x02,0x56,0x02,0xC4,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x04,0x02,0xA6,0x03,0x82,0x02,0xA6,0x03,0x82,0x00,0x00,0x00,0x00,0x05,0x00,0x08,0x00,0x08,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x06,0x00,0x58,0x00,0xC6,0x00,0x58,0x00,0xC6,0x00,0x00,0x00,0x00,0x07,0x02,0x56,0x02,0xC4,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x08,0x02,0xA6,0x03,0x82,0x02,0xA6,0x03,0x82,0x07,0x03,0x56,0x03,0x09,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0A,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0B,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0D,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0E,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0F,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0x00,0x00,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFE,0x10,0x0C,0x00,0xD8,0x3F,0x00,0x66,0x00,0x00,0x00,0x00};

/* steady-green overlap 8. */ 
STATIC uint8_t tw_steady_green_ovr_8[sizeof(twSPATData)] = {0x00,0x02,0x02,0x3A,0x02,0x8A,0x00,0x14,0x00,0x3C,0x02,0x3A,0x02,0x8A,0x00,0x14,0x00,0x3C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x02,0x3C,0x02,0xFA,0x00,0x16,0x00,0x3E,0x02,0x3C,0x02,0xFA,0x00,0x16,0x00,0x3E,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xBB,0x00,0x00,0x00,0x44,0x00,0x00,0x00,0x00,0x00,0x02,0x8A,0x00,0x00,0x00,0x3C,0x00,0x00,0x02,0x8A,0x00,0x00,0x00,0x3C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x02,0xFA,0x00,0x00,0x00,0x3E,0x00,0x00,0x02,0xFA,0x00,0x00,0x00,0x3E,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x14,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x16,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0x00,0x00,0x00,0x7F,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x44,0x00,0x00,0x00,0xBB,0x00,0x60,0x5A,0x44,0x86,0x00,0x08,0x00,0x00,0x00,0x00,0x00,0x00};

/* flashing-yellow overlap 8. */ 
STATIC uint8_t tw_flashing_yellow_ovr_8[sizeof(twSPATData)] = {0x00,0x02,0x00,0x53,0x00,0xA3,0x02,0xA1,0x00,0x17,0x00,0x53,0x00,0xA3,0x02,0xA1,0x00,0x17,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x53,0x01,0x11,0x03,0x0F,0x00,0x17,0x00,0x53,0x01,0x11,0x03,0x0F,0x00,0x17,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x77,0x00,0x00,0x00,0x88,0x00,0x00,0x00,0x00,0x00,0x00,0xA3,0x00,0x00,0x02,0xF0,0x00,0x00,0x00,0xA3,0x00,0x00,0x02,0xF0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x11,0x00,0x00,0x03,0xCC,0x00,0x00,0x01,0x11,0x00,0x00,0x03,0xCC,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x17,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x17,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0x00,0x7F,0x00,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x88,0x00,0x00,0x00,0x77,0x00,0x60,0x5A,0x41,0x5D,0x00,0x07,0x00,0x00,0x00,0x00,0x00,0x00};

/* steady-yellow overlap 8. */
STATIC uint8_t tw_steady_yellow_ovr_8[sizeof(twSPATData)] = {0x00,0x02,0x02,0x1A,0x02,0x6A,0x00,0x12,0x00,0x1C,0x02,0x1A,0x02,0x6A,0x00,0x12,0x00,0x1C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x02,0x1A,0x02,0xD8,0x00,0x12,0x00,0x1C,0x02,0x1A,0x02,0xD8,0x00,0x12,0x00,0x1C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xBB,0x00,0x44,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x02,0x6A,0x00,0x00,0x00,0x1C,0x00,0x00,0x02,0x6A,0x00,0x00,0x00,0x1C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x02,0xD8,0x00,0x00,0x00,0x1C,0x00,0x00,0x02,0xD8,0x00,0x00,0x00,0x1C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x12,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x12,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0x00,0x7F,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x44,0x00,0xBB,0x00,0x60,0x5A,0x44,0x8A,0x00,0x02,0x00,0x00,0x00,0x00,0x00,0x00};

/* overlap 8 as a (steady-red) FYA. */
STATIC uint8_t tw_steady_red_ovr_8[sizeof(twSPATData)] = {0x00,0x02,0x02,0x61,0x02,0xB1,0x00,0x13,0x00,0x63,0x02,0x61,0x02,0xB1,0x00,0x13,0x00,0x63,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x02,0xCF,0x03,0x8D,0x00,0x13,0x00,0xD1,0x02,0xCF,0x03,0x8D,0x00,0x13,0x00,0xD1,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x02,0xB1,0x00,0x00,0x00,0x63,0x00,0x00,0x02,0xB1,0x00,0x00,0x00,0x63,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x03,0x8D,0x00,0x00,0x00,0xD1,0x00,0x00,0x03,0x8D,0x00,0x00,0x00,0xD1,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x13,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x13,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0x00,0x60,0x5A,0x44,0x77,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00};

#endif /* FYA_TEST */

/* mandatory logging defines */
#define MY_ERR_LEVEL   LEVEL_INFO    /* from i2v_util.h */
#define MY_NAME        "scs"

static i2vShmMasterT *shm_ptr = NULL;
STATIC bool_t mainloop = WTRUE;
STATIC scsCfgItemsT cfg;
STATIC char_t confFileName[I2V_CFG_MAX_STR_LEN];
#ifdef RSE_SIMULATOR
/* this should probably be here anyway */
static cfgItemsT parentcfg;
#endif /* RSE_SIMULATOR */
static uint8_t rsuMacAddr[LENGTH_MAC]; /* MAC address of ethernet interface to be used as unique ID */
static SRMMsgType SRMMsg;             /* GridSmart SRM packet: send CSV by port. Could be GS, McCain or TC. */
#if !defined(MY_UNIT_TEST)
static uint8_t mac_id[LENGTH_MAC*2];   /* RSU's MAC ID from playback file - Ethernet */
#endif
#if defined(GS2_SUPPORT) && defined(GS2_FAKE_DZ_MESSAGES)
static uint16_t fakeDZMessage = 0;
#endif /* GS2_SUPPORT */

/* 20180216: shared between the scs main routines */
STATIC scsSpatInfo spat_info;

/* shared with icddecoder.c & twaredecoder.c */
uint8_t      icdYellowDur = 0;
ovlGroupMap OverlapMap; /* Has RGY groups inside. */
uint16_t     SpatPedDetect = 0; /* work in progress */
uint16_t     SpatPedCall = 0;   /* work in progress */

#if defined(MY_UNIT_TEST)
static uint32_t my_test_counter = 0x0;
#endif

STATIC uint64_t my_error_states = 0x0;
STATIC uint32_t scs_processed_cnt = 0x0;
STATIC uint32_t scs_srm_rx_counts = 0;
STATIC uint32_t scs_srm_rx_err_counts = 0;

/* SRM Forward Message */
STATIC  fwdmsgCfgItemsT fwdmsgCfg;
STATIC  bool_t  srmForwardMsg; 

STATIC mqd_t fwdmsgfd;     
STATIC fwdmsgData srmFwdmsgData;
STATIC char sendbuf[sizeof(srmFwdmsgData)];
STATIC pthread_mutex_t fwdmsg_send_lock = PTHREAD_MUTEX_INITIALIZER;

static void scs_static_init(void)
{
  icdYellowDur    = 0;
  mainloop        = WTRUE;
  SpatPedDetect   = 0;
  SpatPedCall     = 0;

  memset(&OverlapMap,0x0,sizeof(OverlapMap));
  
#if defined(MY_UNIT_TEST)
   my_test_counter = 0x0;
#endif
  my_error_states = 0x0;
  scs_processed_cnt = 0x0;
  scs_srm_rx_counts = 0x0;
  scs_srm_rx_err_counts = 0x0;
#if !defined(MY_UNIT_TEST) /* Select flavor in unit_test. */
    strncpy(confFileName,SCS_CFG_FILE,sizeof(confFileName));
#endif
  memset(&fwdmsgCfg,0x0,sizeof(fwdmsgCfg));
  srmForwardMsg = WFALSE; 
  memset(&fwdmsgfd,0x0,sizeof(fwdmsgfd));     
  memset(&srmFwdmsgData,0x0,sizeof(srmFwdmsgData));
  memset(sendbuf,0x0,sizeof(sendbuf));
  memset(&fwdmsg_send_lock,0x0,sizeof(fwdmsg_send_lock));
}
STATIC void set_my_error_state(int32_t my_error)
{
  int32_t dummy = 0;

  dummy = abs(my_error);

  if((dummy <= 64) && (0 < dummy)) {
      my_error_states |= (uint64_t)(0x1) << (dummy - 1);
  }
  return;
}
void clear_my_error_state(int32_t my_error)
{
  int32_t dummy = 0;

  dummy = abs(my_error);
  if((dummy <= 64) && (0 < dummy)) {
      my_error_states &= ~((uint64_t)(0x1) << (dummy - 1));
  }
  return;
}
int32_t is_my_error_set(int32_t my_error)
{
  int32_t dummy = 0;
  int32_t ret = 0; /* FALSE till proven TRUE */

  dummy = abs(my_error);
  if((dummy <= 64) && (0 < dummy)) {
      if (my_error_states & ((uint64_t)(0x1) << (dummy - 1))) {
          ret = 1;
      }
  }
  return ret;
}
/*----------------------------------------------------------------------------*/
/* Dump to /tmp for user to cat. Only actionable items for user.              */
/*----------------------------------------------------------------------------*/
STATIC void dump_scs_report(void)
{
  FILE * file_out = NULL;
  if ((file_out = fopen("/tmp/scs.txt", "w")) == NULL){
      if(0x0 == is_my_error_set(SCS_CUSTOMER_DIGEST_FAIL)) { /* Report fail one time. */
          set_my_error_state(SCS_CUSTOMER_DIGEST_FAIL);
      }
  } else {
      clear_my_error_state(SCS_CUSTOMER_DIGEST_FAIL);
      if(is_my_error_set(SCS_INIT_TC_SOCK_FAIL)){
          fprintf(file_out,"SCS_INIT_TC_SOCK_FAIL: Unable to connect to traffic controller. Check IP+Port is valid and no conflict exists.\n");
      }
      if(is_my_error_set(SCS_INIT_SRM_SOCK_FAIL)){
          fprintf(file_out,"SCS_INIT_SRM_SOCK_FAIL: Unable to connect to SRM FWD Port. Check Port is valid and no conflict exists.\n");
      }
      if(is_my_error_set(SCS_TW_PARSE_ERROR)){
          fprintf(file_out,"SCS_TW_PARSE_ERROR: Unable to parse blob from controller. Check correct controller type selected.\n");
      }
      if(is_my_error_set(SCS_ICD_PARSE_ERROR)){
          fprintf(file_out,"SCS_ICD_PARSE_ERRORR: Unable to parse blob from controller. Check correct controller type selected.\n");
      }
      if(is_my_error_set(SCS_SPAT_MISSING_DATA)){
          fprintf(file_out,"SCS_SPAT_MISSING_DATA: Unable to parse blob from controller. Check correct controller type selected.\n");
      }
      if(is_my_error_set(SCS_SEND_SRM_SEND_ERROR)){
          fprintf(file_out,"SCS_SEND_SRM_SEND_ERROR: Unable to send SRM to controller. Ensure controller still active.\n");
      }
      if(is_my_error_set(SCS_SEND_SRM_RAW_SEND_ERROR)){
          fprintf(file_out,"SCS_SEND_SRM_RAW_SEND_ERROR: Unable to send SRM to saw socket. Ensure remote host still active.\n");
      }
      if(is_my_error_set(SCS_SRM_MALFORMED)){
          fprintf(file_out,"SCS_SRM_MALFORMED: Unable to decode SRM. Ensure OBU is sending correct ANS1 encode message.\n");
      }
      if(is_my_error_set(SCS_SRM_HEARTBEAT_SEND_FAIL)){
          fprintf(file_out,"SCS_SRM_HEARTBEAT_SEND_FAIL: Unable to send heart beat message. Check IP+Port is valid and no conflict exists.\n");
      }
      if(is_my_error_set(SCS_MAIN_RECVFROM_ERROR)){
          fprintf(file_out,"SCS_MAIN_RECVFROM_ERROR: Unable to receive data from controller. Ensure controller is still active.\n");
      }
      if(is_my_error_set(SCS_MAIN_SPAT_PROCESS_ERROR)){
          fprintf(file_out,"SCS_MAIN_SPAT_PROCESS_ERROR: Did not receive SPAT message. Ensure SPAT task is still active.\n");
      }
      if(is_my_error_set(SCS_MAIN_CONTROLLER_IP_ERROR)){
          fprintf(file_out,"SCS_MAIN_CONTROLLER_IP_ERROR: Received SPAT message from unknown IP and it was rejected.\n");
      }
      if(is_my_error_set(SCS_MAIN_YELLOW_DURATION_ERROR)){
          fprintf(file_out,"SCS_MAIN_YELLOW_DURATION_ERROR: Received SPAT message with invalid yellow duration.\n");
      }
      if(is_my_error_set(SCS_SRM_BAD_TTI)){
          fprintf(file_out,"SCS_SRM_BAD_TTI: Received SRM message where Time To Intersection is invalid\n");
      }
      fflush(file_out);
      fclose(file_out);
      file_out = NULL;
  }
}
STATIC void scs_sighandler(int __attribute__((unused)) sig)
{
  mainloop = WFALSE;

  #ifdef GS2_SUPPORT
  /* this should have been there for gs2 handling; thread will otherwise hang on exit */
  wsu_open_gate(&shm_ptr->gs2IntersectionStatus.triggerGate);
  #endif /* GS2_SUPPORT */

  set_my_error_state(SCS_SIG_FAULT);
  return;
}

/* Creates a socket to listen for UDP messages coming from the 
 * Econolite or McCain traffic controller. 
 * Returns the socket on success, -1 on error 
 */
STATIC int scs_init_tc_sock(void)
{
    struct sockaddr_in local;
    int sock = -1;
#if defined(MY_UNIT_TEST)
    int flag = 1;
#endif

#ifdef RSE_SIMULATOR
    if (!parentcfg.spsAppEnable) {
#endif /* RSE_SIMULATOR */
        if ((sock = SOCKET(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"SCS socket creation failed. errno=%d.\n",errno);
            set_my_error_state(SCS_TC_SOCKET_ERROR);
            return -1;
        }

        memset(&local, 0, sizeof(local));
        local.sin_family = AF_INET;
        local.sin_port = htons(cfg.localSignalControllerPort);
        local.sin_addr.s_addr = htonl(INADDR_ANY);

        /*
         * I can exhaust all the ADDR under stress test.
         * Not sure about target.
         */
#if defined(MY_UNIT_TEST)
        if (-1 == dn_setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag))) {
            close(sock);
            return -1;  
        }
#endif

        if (dn_bind(sock, (struct sockaddr *)&local, sizeof(local)) == -1) {
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"SCS socket bind failed. errno=%d.\n",errno);
            close(sock);
            set_my_error_state(SCS_TC_BIND_ERROR);
            return -1;
        }
#ifdef RSE_SIMULATOR
    } else {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"SCS in simulator mode\n");
        if ((sock = open(I2V_SIM_SPAT_QUEUE, I2V_Q_READER)) < 0) {
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"failed to open simulator queue\n");
            set_my_error_state(SCS_TC_SOCKET_ERROR);
            return -1;
        }
    }
#endif /* RSE_SIMULATOR */

    return sock;
}

#ifdef GS2_SUPPORT
/* Creates a socket to listen for TCP messages coming from the 
 * GridSmart GS2 traffic controller. 
 * Returns the socket on success, -1 on error 
 */
static int scs_init_gs2_sock(void)
{
    struct sockaddr_in local;
    int sock = -1;

#ifdef RSE_SIMULATOR
    if (!parentcfg.spsAppEnable) {
#endif /* RSE_SIMULATOR */
        if ((sock = SOCKET(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"GS2 socket creation failed. errno=%d.\n",errno);
            return -1;
        }

        memset(&local, 0, sizeof(local));
        local.sin_family = AF_INET;
        local.sin_port = htons(cfg.localGS2Port);
        local.sin_addr.s_addr = htonl(INADDR_ANY);

        /*
         * I can exhaust all the ADDR under stress test.
         * Not sure about target.
         */
        #if defined(MY_UNIT_TEST)
        if (-1 == dn_setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag))) {
            close(sock);
            return -1;  
        }
        #endif

        if (dn_bind(sock, (struct sockaddr *)&local, sizeof(local)) == -1) {
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"GS2 socket bind failed. errno=%d.\n",errno);
            close(sock);
            return -1;
        }
#ifdef RSE_SIMULATOR
    } else {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"GS2 in simulator mode\n");
        if ((sock = open(I2V_SIM_GS2_QUEUE, I2V_Q_READER)) < 0) {
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"failed to open simulator GS2 queue. errno=%d.\n",errno);
            return -1;
        }
    }
#endif /* RSE_SIMULATOR */

    return sock;
}
#endif

/*
 * scs_init_srm_sock(void)
 * Creates a socket to SRM interface for forwarding of SRM 
 */
STATIC int scs_init_srm_sock(void)
{
    struct sockaddr_in local;
    int sock = -1;
#if defined(MY_UNIT_TEST)
    int flag = 1;
#endif
    if ((sock = SOCKET(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"SRM socket creation failed. errno=%d.\n",errno);
        set_my_error_state(SCS_SRM_SOCKET_ERROR);
        return -1;
    }

    memset(&local, 0, sizeof(local));
    local.sin_family = AF_INET;
    local.sin_port = htons(cfg.SRMFwdPort);
    local.sin_addr.s_addr = htonl(INADDR_ANY);

    /*
     * I can exhaust all the ADDR under stress test.
     * Not sure about target.
     */
    #if defined(MY_UNIT_TEST)
    if (-1 == dn_setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag))) {
        close(sock);
        return -1;  
    }
    #endif

    if (dn_bind(sock, (struct sockaddr *)&local, sizeof(local)) == -1) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"SRM socket bind failed. errno=%d.\n",errno);
        close(sock);
        set_my_error_state(SCS_SRM_BIND_ERROR);
        return -1;
    } 
    return sock;
}

STATIC int scs_process_spat(unsigned char *buf)
{
    i2vReturnTypesT ret = I2V_RETURN_OK;
    /* spat_info has to be static with members set to zero initially because 
     * wsu_parse_spat() expects that. This is needed to calculate yellow duration because 
     * Econlite ASC3 does not provide yellow duration in the controller message 
     */
    if(NULL == shm_ptr) {
        if(0x0 == (my_error_states & SCS_SHM_INIT_FAIL)){
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"scs_process_spat:SCS_SHM_INIT_FAIL .\n");
            set_my_error_state(SCS_SHM_INIT_FAIL);
        }
        return -1;
    }
    memset(&spat_info, 0, sizeof(spat_info));

    /*
     * Old format. Leave logic for future growth.
     */
    #if defined(ENABLE_WSU_TOM)
    if (cfg.tomformat) {
        if (wsu_parse_spat(&spat_info.tomSpat, buf) == -1) {
            return -1;
        }
    } else
    #endif /* ENABLE_WSU_TOM */
    {
        /* add hardware specific controllers that don't support ICD first */
        /* this works because the trafficware controllers are all together, beware of separating them */
        if ((cfg.localSignalControllerHW >= TrafficWare) && (cfg.localSignalControllerHW <= TrafficWareFinal)) {
            /* consider phase enable mask in the future? */
            if (I2V_RETURN_FAIL == (ret = twParseSpat(&spat_info.flexSpat, buf, cfg.useMaxTTC, cfg.localSignalControllerHW))) {
#if defined(EXTRA_DEBUG)
                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"failed to decode tw message\n");
#endif
                set_my_error_state(SCS_TW_PARSE_ERROR);
                return -1;
            }
        } else {
            /* ICD format; controller hardware agnostic */
            if (I2V_RETURN_FAIL == (ret = icdParseSpat(&spat_info.flexSpat, buf, cfg.useMaxTTC))) {
                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"failed to decode icd message\n");
                set_my_error_state(SCS_ICD_PARSE_ERROR);
                return -1;
            } else if (ret == I2V_RETURN_MISSING_DATA) {
                /* FIXME: original idea was to accumulate a series of sequential messages where the
                   mask didn't match; but for fastest response possible once a mismatch detected
                   get the updated info; this is not the preferred design because it could result in
                   excess snmp traffic if for some reason data is corrupted in the message */
#ifdef EXTRA_DEBUG
                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"icd message has state info that doesn't match enable mask; requerying\n");
#endif /* EXTRA_DEBUG */
                set_my_error_state(SCS_SPAT_MISSING_DATA);                
            }
        }
    }

    /* Write the parsed SPAT message info to shared memory */
    wsu_shmlock_lockw(&shm_ptr->scsSpatData.h.ch_lock);
    memcpy(&shm_ptr->scsSpatData.spat_info, &spat_info, sizeof(spat_info));

    /*
     * Old format. Leave logic for future growth.
     */
    #if defined(ENABLE_WSU_TOM)
    if (cfg.tomformat) {
        shm_ptr->scsSpatData.spat_info.tomSpat.int_id = cfg.spat_intersection_id;
    } else
    #endif /* ENABLE_WSU_TOM */ 
    {
        shm_ptr->scsSpatData.spat_info.flexSpat.intID = cfg.spat_intersection_id;
    }
    shm_ptr->scsSpatData.spatSelector = !cfg.tomformat;
    shm_ptr->scsSpatData.spat_parse_timestamp = time(0L);
    shm_ptr->scsSpatData.h.ch_data_valid = WTRUE;
    shm_ptr->scsSpatData.h.ch_sequence_number++;
    shm_ptr->scsSpatData.packetCount++;
    wsu_shmlock_unlockw(&shm_ptr->scsSpatData.h.ch_lock);
    return 0;
}

#ifdef GS2_SUPPORT
// In order to simplify this we have maximum values for the number of message parameters
// and the number of values allowed for each parameter. To make it flexible in C means lots
// of mallocs and frees with the associated risks. We make the limits large to support
// reasonable numbers of parameters and values and lengths, and these can be increased 
// if needed. Note that message type and sequence ID are considered the first parameter.
//
typedef struct parameter
{
    char id[MAX_GS2_MESSAGE_VALUE_LENGTH];
    int valueCount;
    char values[MAX_GS2_MESSAGE_PARAMETER_VALUES][MAX_GS2_MESSAGE_VALUE_LENGTH];
} parameter;

static parameter parameters[MAX_GS2_MESSAGE_PARAMETERS];

static int scs_process_gs2(unsigned char *buf)
{
    i2vReturnTypesT ret = I2V_RETURN_OK;
    enum GS2Request request;
    int i = 0;
    int parameterCount = 0;
    bool_t sendNAK = WFALSE;
    const char firstPassDelimiters[] = "/;";
    const char secondPassDelimiters[] = ":";
    const char thirdPassDelimiters[] = ",";
    char *firstPass;
    char *firstPassRunning;
    char *firstPassToken;

    if (cfg.GS2DemoDebugLevel > 5) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"GS2: inbound message '%s' received\n", buf);
    }
    // TODO MFM
    // Parse the inbound message. Note that we may receive only a partial message,
    // and if so we need to collect data from the inbound buffer until a complete
    // message is received. At that point we parse and process it. We save any
    // remaining data as the start of the next message.
    //
    // Note that there are several ways of passing this information on to a consumer.
    // The basic problem is that if we have a data structure protected by a single
    // lock, then if a prior received message was not processed by any consumer it will
    // be over written by a new inbound message. Likewise, if all data is in a single
    // structure there has to be some way for separate consumers to know whether any
    // given subset of the data is value. Therefore, it makes more sense to have
    // service-oriented protected blocks of SHM data, so that there are separate locks.
    // This assumes that a given service is processed by a single entity and consumed
    // in order of receipt.

    // We perform multiple levels of parsing. The first level will split the inbound
    // message into message type and content. We can then validate the type and send a NAK if
    // we don't recognize the message.
    // The next level of parsing is to extract the parameter list. This is a list of
    // parameter IDs and value lists. Validation at this level is based on message type
    // since each message will have different parameters.
    // Finally, each ParameterValueList is parsed to extract the content (which may consist of
    // one or more elements). Parsing at this level is message and parameter-specific.

    // First, find elements based on the tokens "/" and ";"

    firstPass = firstPassRunning = strdup(buf);
    firstPassToken = strsep(&firstPassRunning, firstPassDelimiters);  
    while (firstPassToken && !sendNAK) {
        // If the token has content then it should be a parameter definition
        if (strlen(firstPassToken) > 0) {
            // We now parse at the second level. This will break the parameters into ID/ValueList pairs

            if (cfg.GS2DemoDebugLevel > 8) {
                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"GS2: first pass token: '%s'\n", firstPassToken);
            }
            // Make a copy of the first pass substring
            char *secondPass = strdup(firstPassToken);
            char *secondPassRunning = secondPass;

            // Start parsing
            char *secondPassToken = strsep(&secondPassRunning, secondPassDelimiters);

            // This should have content!
            if (secondPassToken) {

                if (cfg.GS2DemoDebugLevel > 8) {
                    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"GS2: second pass token: '%s'\n", secondPassToken);
                }
                // Clear this parameter entry since we are going to use it
                memset(&parameters[parameterCount], 0, sizeof(parameter));

                // This token should be the parameter ID
                strncpy(parameters[parameterCount].id, secondPassToken, MAX_GS2_MESSAGE_VALUE_LENGTH);

                secondPassToken = strsep(&secondPassRunning, secondPassDelimiters);

                // This token should be the parameter value list
                if (strlen(secondPassToken) > 0) {
                    // We now parse at the third level and put each value found into the parameter value list

                    // Make a copy of the second pass substring
                    char *thirdPass = strdup(secondPassToken);
                    char *thirdPassRunning = thirdPass;

                    // Start parsing
                    char *thirdPassToken = strsep(&thirdPassRunning, thirdPassDelimiters);  // Should be the ID and value list

                    // Now iterate through the value list
                    while (thirdPassToken && !sendNAK) {
                        if (cfg.GS2DemoDebugLevel > 8) {
                            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"GS2: third pass token: '%s'\n", thirdPassToken);
                        }
                        // Copy the value and update the valueCount
                        strncpy(parameters[parameterCount].values[parameters[parameterCount].valueCount++], thirdPassToken, MAX_GS2_MESSAGE_VALUE_LENGTH);

                        // If too many parameter values, stop processing and send a NAK
                        if (parameters[parameterCount].valueCount == MAX_GS2_MESSAGE_PARAMETER_VALUES) {
                            I2V_ERR_LOG( "GS2: inbound message '%s' parameter '%s' too many parameter values\n", parameters[0].id, parameters[parameterCount].id);
                            sendNAK = WTRUE;
                            break;
                        }

                        thirdPassToken = strsep(&thirdPassRunning, thirdPassDelimiters);
                    }

                    // No longer need thirdPass
                    if (thirdPass)
                        free(thirdPass);

                    parameterCount++;

                    // If too many parameters, stop processing and send a NAK
                    if (parameterCount == MAX_GS2_MESSAGE_PARAMETERS) {
                        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"GS2: inbound message '%s' too many parameters\n", parameters[0].id);
                        sendNAK = WTRUE;
                        break;
                    }

                }
                else {
                    sendNAK = WTRUE;
                }
            } else {
                sendNAK = WTRUE;
            }

            //
            // No longer need secondPass
            if (secondPass)
                free(secondPass);

        }
        firstPassToken = strsep(&firstPassRunning, firstPassDelimiters);  
    }

    // No longer need firstPass
    if (firstPass)
        free(firstPass);

    // Now we do some basic validation of the message. Message-specific validation is done once we do this.
    // All messages have at least one parameter (the message Type ID and Sequence ID)
    if (parameterCount == 0) {
        sendNAK = WTRUE;
    }

    // Make sure it's a message type we understand, and convert it into a request type
    if (!sendNAK) {
        if (!strcmp(parameters[0].id, "INS")) {
            request = IntersectionStatus;
        } else if (!strcmp(parameters[0].id, "WWS")) {
            request = WrongWayStatus;
        } else if (!strcmp(parameters[0].id, "ACK")) {
            // Note that in the demo we map ALL ACKs to Dilemma Zone. We had unique ACKs in the proposed design but
            // GS requested we use a general ACK design. Thus, once more than one message requires an ACK, we (scs)
            // will have to track outbound messages by type and sequence ID, and route returned ACKs based on the 
            // sequence ID to the proper module.
            request = DilemmaZoneACK;
        } else {
            sendNAK = WTRUE;
        }
    }

    // At this point it's a message type we know and we have converted it into a request ID. Now perform specific
    // message validation and parsing.
    if (!sendNAK) {

        switch (request) {
            case IntersectionStatus:
                // Do some basic validation of the parameters. Note that parameter 0 is the Message Type ID and SequenceID so the count should be 2.
                if ((parameterCount == 2) && 
                        (!strcmp(parameters[1].id, "OB")) &&
                        (parameters[1].valueCount == NUM_OBSTRUCTION_ZONES))
                {
                    wsu_shmlock_lockw(&shm_ptr->gs2IntersectionStatus.h.ch_lock);
                    for (i = 0; i < NUM_OBSTRUCTION_ZONES; i++) {
                        shm_ptr->gs2IntersectionStatus.obstructed[i] = (!strcmp(parameters[1].values[i], "0") ? WFALSE : WTRUE);
                    }
                    if (cfg.GS2DemoDebugLevel > 4) {
                        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"GS2: inbound message '%s' obstructed: %d, %d, %d, %d\n", parameters[0].id, shm_ptr->gs2IntersectionStatus.obstructed[0],
                                shm_ptr->gs2IntersectionStatus.obstructed[1],
                                shm_ptr->gs2IntersectionStatus.obstructed[2],
                                shm_ptr->gs2IntersectionStatus.obstructed[3]);
                    }
                    shm_ptr->gs2IntersectionStatus.h.ch_data_valid = WTRUE;
                    wsu_shmlock_unlockw(&shm_ptr->gs2IntersectionStatus.h.ch_lock);
                    wsu_open_gate(&shm_ptr->gs2IntersectionStatus.triggerGate);
                }
                else {
                    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"GS2: inbound message '%s' invalid parameters\n", parameters[0].id);
                    sendNAK = WTRUE;
                }
                break;

            case WrongWayStatus:
                // Do some basic validation of the parameters. Note that parameter 0 is the Message Type ID and SequenceID so the count should be 2.
                if ((parameterCount == 2) && 
                        (!strcmp(parameters[1].id, "WS") || !strcmp(parameters[1].id, "WC")) &&
                        (parameters[0].valueCount == 1))
                {
                    wsu_shmlock_lockw(&shm_ptr->gs2WrongWayStatus.h.ch_lock);
                    // TODO MFM Additional fields from the parameters will be processed when they are defined
                    if (!strcmp(parameters[1].id, "WS"))
                        shm_ptr->gs2WrongWayStatus.set = WTRUE;
                    else
                        shm_ptr->gs2WrongWayStatus.set = WFALSE;
                    if (cfg.GS2DemoDebugLevel > 4) {
                        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"GS2: inbound message '%s' wrong way %s\n", parameters[0].id, (shm_ptr->gs2WrongWayStatus.set ? "set" : "cleared"));
                    }
                    shm_ptr->gs2WrongWayStatus.h.ch_data_valid = WTRUE;
                    wsu_shmlock_unlockw(&shm_ptr->gs2WrongWayStatus.h.ch_lock);
                    wsu_open_gate(&shm_ptr->gs2WrongWayStatus.triggerGate);
                }
                else {
                    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"GS2: inbound message '%s' invalid parameters\n", parameters[0].id);
                    sendNAK = WTRUE;
                }
                break;

            case DilemmaZoneACK:
#if 0
                // Do some basic validation of the parameters. Note that parameter 0 is the Message Type ID and SequenceID so the count should be 2.
                if ((parameterCount == 2) && 
                        (!strcmp(parameters[1].id, "DS") || !strcmp(parameters[1].id, "DC")) &&
                        (parameters[1].valueCount == 1))
                {
                    wsu_shmlock_lockw(&shm_ptr->gs2DilemmaZoneACK.h.ch_lock);
                    if (!strcmp(parameters[1].id, "DS"))
                        shm_ptr->gs2DilemmaZoneACK.set = WTRUE;
                    else
                        shm_ptr->gs2DilemmaZoneACK.set = WFALSE;
                    shm_ptr->gs2DilemmaZoneACK.approachID = atoi(parameters[1].values[0]);
                    if (cfg.GS2DemoDebugLevel > 4) {
                        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"GS2: inbound message '%s' dilemma zone %s: approach: %d\n", parameters[0].id, 
                                (shm_ptr->gs2DilemmaZoneACK.set ? "set" : "cleared"),
                                shm_ptr->gs2DilemmaZoneACK.approachID);
                    }
                    shm_ptr->gs2DilemmaZoneACK.h.ch_data_valid = WTRUE;
                    wsu_shmlock_unlockw(&shm_ptr->gs2DilemmaZoneACK.h.ch_lock);
                    wsu_open_gate(&shm_ptr->gs2DilemmaZoneACK.triggerGate);
                }
                else {
                    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"GS2: inbound message '%s' invalid parameters\n", parameters[0].id);
                    sendNAK = WTRUE;
                }
#else
                // In this demo design we pass the ACK id
                wsu_shmlock_lockw(&shm_ptr->gs2DilemmaZoneACK.h.ch_lock);
                shm_ptr->gs2DilemmaZoneACK.id = atoi(parameters[0].values[0]);
                if (cfg.GS2DemoDebugLevel > 4) {
                    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"GS2: inbound message '%s' (mapping to Dilemma Zone ACK) id: %s\n", parameters[0].id, parameters[0].values[0]);
                }
                shm_ptr->gs2DilemmaZoneACK.h.ch_data_valid = WTRUE;
                wsu_shmlock_unlockw(&shm_ptr->gs2DilemmaZoneACK.h.ch_lock);
#endif
                break;

            default:
                // Not a message we recognize - send a NAK
                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"GS2: inbound message '%s' unrecognized\n", parameters[0].id);
                sendNAK = WTRUE;
                break;
        }
    }

    if (sendNAK) {
        if ((parameterCount > 0) && (parameters[0].valueCount >= 1)) {
            if (cfg.GS2DemoDebugLevel > 4) {
                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"GS2: sending NAK for message %s:%s\n", parameters[0].id, parameters[0].values[0]);
            }
            wsu_shmlock_lockw(&shm_ptr->gs2OutboundData.h.ch_lock);
            shm_ptr->gs2OutboundData.request = NAK;
            shm_ptr->gs2OutboundData.id = atoi(parameters[0].values[0]);
            wsu_shmlock_unlockw(&shm_ptr->gs2OutboundData.h.ch_lock);
            wsu_open_gate(&shm_ptr->gs2OutboundData.triggerGate);
        } else {
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"GS2: cannot send NAK for a completely bogus message!\n");
        }
    }

#ifdef GS2_FAKE_DZ_MESSAGES
    fakeDZMessage++;
    if ((fakeDZMessage % FAKE_GS2_DZ_MESSAGE_TRIGGER) == 0) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"GS2: sending fake DZ message...\n");
        wsu_shmlock_lockw(&shm_ptr->gs2OutboundData.h.ch_lock);
        shm_ptr->gs2OutboundData.request = nextFakeDZRequest;
        shm_ptr->gs2OutboundData.id = fakeDZMessage;
        wsu_shmlock_unlockw(&shm_ptr->gs2OutboundData.h.ch_lock);
        wsu_open_gate(&shm_ptr->gs2OutboundData.triggerGate);
        if (nextFakeDZRequest == DilemmaZoneSet)
            nextFakeDZRequest = DilemmaZoneClear;
        else
            nextFakeDZRequest = DilemmaZoneSet;
    }
#endif
    return ret;
}
#endif

/* port is the destination snmp port and is used if non-zero */
void scs_send_snmp_cmd(char *oid, char *type, char *value, int port)
{
#ifdef LEGACY_SNMP
    char *p[8];
    /* We need these arrays because snmpsetFunc() will not work with constant string literals */
    char str0[50];
    char str1[50];
    char str2[50];
    char str3[50];
    char str4[50];
    char str5[50];
    char str6[50];
    char str7[50];

    strcpy(str0, "snmpapi");
    strcpy(str1, "-v1");
    strcpy(str2, "-c");
    strcpy(str3, "public");
    if (port != 0) {
        sprintf(str4, "%s:%d",(char_t *)cfg.localSignalControllerIP, port);
    } else {
        strcpy(str4,(char_t *)cfg.localSignalControllerIP);
    }
    strcpy(str5, oid);
    strcpy(str6, type);
    strcpy(str7, value);

    p[0] = str0; p[1] = str1; p[2] = str2; p[3] = str3; 
    p[4] = str4; p[5] = str5; p[6] = str6; p[7] = str7;

    if (mainloop) {
        snmpsetFunc(8, p);
    }
#else /* LEGACY_SNMP */
    /* snmpsetFunc requires SNMP v5.5; rather than waste time trying to port the func to support
       a more modern SNMP version (something less than a decade old), just use the freaking underlying
       utility ALREADY INCLUDED!!! (removes dependency on a specific version) */
    char cmd[400];

    if (port != 0) {
        snprintf(cmd,400, "snmpset -v1 -c public %s:%d %s %s %s",(char_t *)cfg.localSignalControllerIP, port, oid, type, value); 
    } else {
        snprintf(cmd,400, "snmpset -v1 -c public %s %s %s %s",(char_t *)cfg.localSignalControllerIP, oid, type, value); 
    }

    if(system(cmd)) { /* Silence of the warnings. */

    }
#endif /* LEGACY_SNMP */
}


unsigned long long v_gettime( struct timeval *tv )
{
    struct timeval sys_tv;
    struct timeval *tvptr = tv;
    unsigned long long v;

    if (tv == NULL)
    {
        gettimeofday( &sys_tv, NULL );
        tvptr = &sys_tv;
    }
    v = tvptr->tv_sec*1000LL + tvptr->tv_usec/1000LL;
    return v;
}
/*
 * =================== SRM Stuff ========================
 */

/* "DENSO RSU Traffic Signal Priority Request Interface" 
 * scsSendSRM()
 * Send SRM to Controller
 */
void scsSendSRM(int32_t *srmSocket, SRMMsgType *SRMMsg)
{
    struct   sockaddr_in remote;
    int32_t  remote_len = sizeof(remote);
    char_t   my_SRMMsgCSV[SRM_CSV_MAX_LINE]; 
    char_t   my_dummy_data[SRM_CSV_MAX_LINE];

    if ((srmSocket == NULL) || (SRMMsg == NULL) ){
        set_my_error_state(SCS_SEND_SRM_NULL_INPUT);
        scs_srm_rx_err_counts++;
        return;
    }
    if(-1 == *srmSocket) {
        set_my_error_state(SCS_SEND_SRM_SOCKET_CLOSED);
        scs_srm_rx_err_counts++;
        return; /* Not ready so back off. */
    }

    memset(my_dummy_data,'\0',SRM_CSV_MAX_LINE);
    memset(my_SRMMsgCSV ,'\0',SRM_CSV_MAX_LINE);

    SRMMsg->timeStamp = v_gettime(NULL);    /* get timestamp */
    
    memset(&remote, 0, sizeof(remote));
    remote.sin_family = AF_INET;
    remote.sin_port = htons(cfg.SRMFwdPort);
    remote.sin_addr.s_addr = inet_addr((const char_t *)cfg.SRMFwdIP);


    /*
     * Construct SRM message CSV format
     */
    /* Header */
    snprintf(my_SRMMsgCSV
           , SRM_CSV_MAX_LINE
           , "%02X%02X%02X%02X%02X%02X,%d,%d,%u,%d"
           , SRMMsg->rsuID[0]
           , SRMMsg->rsuID[1]
           , SRMMsg->rsuID[2]
           , SRMMsg->rsuID[3]
           , SRMMsg->rsuID[4]
           , SRMMsg->rsuID[5]
           , SRMMsg->seqNum
           , SRMMsg->msgVersion
           , SRMMsg->timeStamp
           , SRMMsg->msgType);

    /* Message body - concat */
#if 0 /* Legacy: not used in SRM CSV API. */
    sprintf(SRMMsgCSV, ",%lu", SRMMsgCSV, SRMMsg->stationID);
#endif
    snprintf(  my_dummy_data
             , SRM_CSV_MAX_LINE
             , ",%u,%d,%d,%d,%d,%d,%d,%u,%u,%u,%u,%d,%d,%d,%d,%d"
             , SRMMsg->msgTimeStamp
             , SRMMsg->msgSecond
             , SRMMsg->msgCount
             , SRMMsg->intID
             , SRMMsg->reqID
             , SRMMsg->priReqType
             , SRMMsg->inBoundLaneID
             , SRMMsg->etaMinute
             , SRMMsg->etaSeconds
             , SRMMsg->etaDuration
             , SRMMsg->vehID
             , SRMMsg->vehRole
             , SRMMsg->vehLatitude
             , SRMMsg->vehLongitude
             , SRMMsg->vehElevation
             , (int32_t)SRMMsg->vehHeading);

    /* Leave room for NULL to be added */
    strncat(my_SRMMsgCSV,my_dummy_data,SRM_CSV_MAX_LINE-1);

#if defined(EXTRA_EXTRA_DEBUG)
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"SRMMsgCSV:[%s]\n",my_SRMMsgCSV);
#endif

    /* Copy the SRM message to the fwdmsg message queue */
    if (srmForwardMsg)  {
        pthread_mutex_lock(&fwdmsg_send_lock);
        memset(&srmFwdmsgData,0,sizeof(fwdmsgData));
        srmFwdmsgData.fwdmsgType = SRM;
        /* This a tricky string cause it could end up being too large depending on how the numbers convert. */
        if(strlen(my_SRMMsgCSV) < sizeof(my_SRMMsgCSV)) { /* Make sure we get NULL on end of transmission */
            srmFwdmsgData.fwdmsgDataLen = strlen(my_SRMMsgCSV) + 1;
        } else {
            set_my_error_state(SCS_SEND_SRM_SEND_TRUNCATE);
            srmFwdmsgData.fwdmsgDataLen = sizeof(my_SRMMsgCSV);
        }
        memcpy(&srmFwdmsgData.fwdmsgData,my_SRMMsgCSV,srmFwdmsgData.fwdmsgDataLen);
        memcpy(sendbuf,&srmFwdmsgData,sizeof(srmFwdmsgData));

        if ( -1 == mq_send(fwdmsgfd, sendbuf,sizeof(srmFwdmsgData),DEFAULT_MQ_PRIORITY))
        {
            set_my_error_state(SCS_SEND_SRM_SEND_ERROR);
            scs_srm_rx_err_counts++;
        }
        pthread_mutex_unlock(&fwdmsg_send_lock);
    }

    /* Send to controller */
    if(1 == sendto(*srmSocket, my_SRMMsgCSV, strlen(my_SRMMsgCSV), 0, (struct sockaddr *)&remote, remote_len)) {
        set_my_error_state(SCS_SEND_SRM_SEND_ERROR);
        scs_srm_rx_err_counts++;
    }
}
STATIC void scsSendSRM_Raw(int32_t *srmSocket, char_t *raw_data, uint8_t raw_length)
{
    struct sockaddr_in remote;
    int32_t remote_len = sizeof(remote);

    if (   (srmSocket == NULL) || (raw_data  == NULL) ){
        set_my_error_state(SCS_SEND_SRM_RAW_NULL_INPUT);
        scs_srm_rx_err_counts++; 
        return;
    }
    
    if(-1 == *srmSocket) {
        /* Not ready back off. */
        set_my_error_state(SCS_SEND_SRM_RAW_SOCKET_CLOSED);
        scs_srm_rx_err_counts++; 
        return; 
    }

    memset(&remote, 0, sizeof(remote));
    remote.sin_family      = AF_INET;
    remote.sin_port        = htons(cfg.SRMFwdPort);
    remote.sin_addr.s_addr = inet_addr((const char_t *)cfg.SRMFwdIP);

    #if defined(EXTRA_DEBUIG)
    {
        unsigned int i = 0x0;
      
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"scsSendSRM_Raw:: Message sent len=%d:[\n",raw_length);

        for(i=0;i<raw_length;i++)
        {
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"0x%x,\n",raw_data[i]);
        } 
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"]\n");
    }
    #endif

    if (srmForwardMsg)  {
        /* Copy the SRM message to the fwdmsg message queue */
        pthread_mutex_lock(&fwdmsg_send_lock);
        memset(&srmFwdmsgData,0,sizeof(fwdmsgData));
        srmFwdmsgData.fwdmsgType = SRM;
        memcpy(&srmFwdmsgData.fwdmsgData,raw_data,raw_length);
        srmFwdmsgData.fwdmsgDataLen = raw_length;

        memcpy(sendbuf,&srmFwdmsgData,sizeof(srmFwdmsgData));

        if ( -1 == mq_send(fwdmsgfd, sendbuf,sizeof(srmFwdmsgData),DEFAULT_MQ_PRIORITY))
        {
            set_my_error_state(SCS_SEND_SRM_SEND_ERROR);
            scs_srm_rx_err_counts++;
        }
        pthread_mutex_unlock(&fwdmsg_send_lock);
    }

    if(-1 == sendto(*srmSocket, raw_data, raw_length, 0, (struct sockaddr *)&remote, remote_len)){
        set_my_error_state(SCS_SEND_SRM_RAW_SEND_ERROR);
        scs_srm_rx_err_counts++; 
    }
}

/*
 * scsMcCainSendSRM()
 * Send SRM to McCain Controller
 */
#if 0
void scsMcCainSendSRM(int32_t *mcSocket, SRMMsgType *SRMMsg)
{
    struct sockaddr_in remote;
    int32_t remote_len = sizeof(remote);
    char_t SRMMsgCSV[SRM_CSV_MAX_LINE];       /* SRM CSV format for GridSmart */
    if ((mcSocket == NULL) || (SRMMsg == NULL) ){
        return;
    }

    if(-1 == *mcSocket) {
        return; /* Not ready back off. */
    }

    SRMMsg->timeStamp = v_gettime(NULL);    /* get timestamp */
    
    memset(&remote, 0, sizeof(remote));
    remote.sin_family = AF_INET;
    remote.sin_port = htons(cfg.localGS2Port);
    remote.sin_addr.s_addr = inet_addr(cfg.localGS2IP);
    
    /*
     * Construct SRM message CSV format
     */
    
    /* Header */
    sprintf(SRMMsgCSV, "%02X%02X%02X%02X%02X%02X,%d,%d,%u,%d", SRMMsg->rsuID[0], SRMMsg->rsuID[1], SRMMsg->rsuID[2],
            SRMMsg->rsuID[3], SRMMsg->rsuID[4], SRMMsg->rsuID[5], SRMMsg->seqNum, SRMMsg->msgVersion,
            SRMMsg->timeStamp, SRMMsg->msgType);
    
    /* Message body - concat */
    sprintf(SRMMsgCSV, "%s,%u", SRMMsgCSV, SRMMsg->timeStamp);
    sprintf(SRMMsgCSV, "%s,%d", SRMMsgCSV, SRMMsg->msgCount);
    sprintf(SRMMsgCSV, "%s,%d", SRMMsgCSV, SRMMsg->intID);
    sprintf(SRMMsgCSV, "%s,%d", SRMMsgCSV, SRMMsg->reqID);
    sprintf(SRMMsgCSV, "%s,%d", SRMMsgCSV, SRMMsg->priReqType);
    sprintf(SRMMsgCSV, "%s,%u", SRMMsgCSV, SRMMsg->etaMinute);
    sprintf(SRMMsgCSV, "%s,%u", SRMMsgCSV, SRMMsg->etaSeconds);
    sprintf(SRMMsgCSV, "%s,%u", SRMMsgCSV, SRMMsg->etaDuration);
    sprintf(SRMMsgCSV, "%s,%u", SRMMsgCSV, SRMMsg->vehID);
    sprintf(SRMMsgCSV, "%s,%u", SRMMsgCSV, SRMMsg->stationID);
    sprintf(SRMMsgCSV, "%s,%u", SRMMsgCSV, SRMMsg->vehRole);
    sprintf(SRMMsgCSV, "%s,%u", SRMMsgCSV, SRMMsg->vehLatitude);
    sprintf(SRMMsgCSV, "%s,%u", SRMMsgCSV, SRMMsg->vehLongitude);
    sprintf(SRMMsgCSV, "%s,%u", SRMMsgCSV, SRMMsg->vehElevation);
    sprintf(SRMMsgCSV, "%s,%d", SRMMsgCSV, SRMMsg->vehHeading);
    
    /* Send to GS2 unit */
    sendto(*gs2Socket, SRMMsgCSV, strlen(SRMMsgCSV), 0, (struct sockaddr *)&remote, remote_len);

}
#endif

/*
 * scs_get_mac_addr()
 */
/* Similiar function exists in  pal/v2v_util.c */
static bool_t scs_get_mac_addr(char *ifname, unsigned char *macaddrstr)
{
    struct ifaddrs *ifap, *ifaptr;
    unsigned char *ptr;
    struct sockaddr_ll *s;

    if (getifaddrs(&ifap) == 0) {
        for(ifaptr = ifap; ifaptr != NULL; ifaptr = (ifaptr)->ifa_next) {
            if (   !strcmp((ifaptr)->ifa_name, ifname) 
                && (((ifaptr)->ifa_addr)->sa_family == AF_PACKET)
               ) {
                s   = (struct sockaddr_ll *)ifaptr->ifa_addr;
                ptr = (unsigned char *)s->sll_addr;

                /* copy mac address over */
                memcpy(macaddrstr, ptr, LENGTH_MAC);
                break;
            }
        }
        freeifaddrs(ifap);
        if(NULL == ifaptr) {
            set_my_error_state(SCS_GET_MAC_ADDR_NOT_FOUND);
        }
        /* If not NULL we found it. */
        return ifaptr != NULL;
    } else {
        set_my_error_state(SCS_GET_MAC_ADDR_ERROR);
        return WFALSE;
    }
}

/*
 * scsSRMMC
 * Send an SRM request to a McCain controller using proprietary MIB
 * McCain MIB OID: 1.3.6.1.4.1.1206.3.21.2.5.14.1.1.x and 1.3.6.1.4.1.1206.3.21.2.5.14.1.3.x
 * where 'x' is a "priority strategy" from 1 - 16; JJG assumes this means approach
 * but we'll find out (no feedback from mccain and we'll just have to go test)
 * both are sets; currently just use snmpset rather than write a new i2vSNMPSet API
 * first OID above activates strategy (must write integer 1); second OID sends ETA in seconds
 * NOTE: SRM 'laneID' may be an actual Lane # rather than approach; if ever that happens
 * then crap will break; there's no way for the RSU to guess which lane corresponds to an
 * approach without parsing the MAP which we aren't going to do (as it may not be onboard)
 */
/* 20200724: after working with STC, the decision was to hard code the strategy sent for
   activation; the strategy is not a phase/approach; and it doesn't matter the direction
   the OBU is traveling for activation; hence STC wants 2 strategies activated; this is
   probably a hack for now and won't be a long term solution; stratey 1 and 2 must be sent
   and the ETA is 0; btw, SRM has a bug where it is not properly populated the SRMMsg
   sent to SCS; hence there's nothing lost by using this hard coded approach
*/
/* 20200828: added for reference.

    -- 2.5.14.1.1 PRIORITY REQUEST
    mcAtcPriorityControlRequest OBJECT-TYPE
        SYNTAX INTEGER (0..1)
        ACCESS read-write
        STATUS mandatory
        DESCRIPTION
            "<Definition> When set to ON (1), this object will
            activate a request for the associated priority strategy.
            It has the same effect as activation of the associated
            priority request physical input.
            The device will automatically reset this object to OFF (0)
            at the end of service of the associated priority strategy.
            "
    ::= { mcAtcPriorityControlEntry 1 }
    -- 2.5.14.1.3 PRIORITY ETA
   mcAtcPriorityControlETA OBJECT-TYPE
        SYNTAX INTEGER (0..255)
        ACCESS read-write
        STATUS mandatory
        DESCRIPTION
            "<Definition> This object specifies the ETA in seconds for an active or
            upcoming priority request. This object allows a remote entity to override the
            ETA value that is programmed in the controller database.
            If set to a non-zero value prior to the start of priority service, then
            this value will be used instead of mcAtcPriorityStrategyETA.
            It set to any value during priority service, the ETA timer will be updated.
            The device will automatically reset this object to zero
            at the end of service of the associated priority strategy.
            "
    ::= { mcAtcPriorityControlEntry 3 }

*/
STATIC void scsSRMMC(uint8_t laneID, uint16_t seconds)
{
/* initial implementation; this may need to be configurable in future, but for now,
   limit the new cfg options being enabled (GUI implications) */
#define MIBACTIVATE "1.3.6.1.4.1.1206.3.21.2.5.14.1.1."
#define MIBETA "1.3.6.1.4.1.1206.3.21.2.5.14.1.3."

    char_t cmd[400];   /* yes a waste of space */
    /* for protection against too much debug if there's an issue */
#if !defined(MY_UNIT_TEST)
    static uint8_t srmmclastwritecnt = 0;
    uint64_t temptime = v_gettime(NULL);
#endif

#if 0   /* see above 20200724 comment; removing dependency on 'lane/approach' */   
    static unsigned long long srmmclastwrite = 0;
    if (!laneID || laneID > 16) {
        /* check out; MIB only supports up to 16 vals and 0 is unsupported; the following will enable
           a debug message at max 10 times per minute */
        if (temptime - srmmclastwrite > 60000) {
            srmmclastwrite = v_gettime(NULL);
            srmmclastwritecnt = 0;
        }
        if (++srmmclastwritecnt < 10) {
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"SCS [%u]: SRM to McCain error -> invalid lane ID (%d)\n", temptime, laneID);
        }
        return;
    }
#endif

    /* activate first, then send eta */
    /* activate approach 2 example: snmpset -v1 -c public 192.168.1.4:161 1.3.6.1.4.1.1206.3.21.2.5.14.1.1.2 i 1 */
#if 0 /*STC work around: Preserve project history */
    laneID = 1;   /* first strategy */
    seconds = 0;  /* ETA hard coded; see 20200724 comment above */
#endif

    /* 16 approaches or phases i get. But if it is lane then that can be greater than 16 */
    /* SRM_RX won't send above 255 */
    /* SRM_RX won't send 0 tti so fraction of time must be left */
    /* Range Check: ETA = 255 or 0: special? */
    if (254 < seconds ) seconds = 254; 
    if (0 == seconds)   seconds = 1; 
    if (0 == laneID)    laneID  = 1;
    if (16 < laneID)    laneID  = 16;
    
    memset(cmd,'\0',sizeof(char) * 400); 
    snprintf(cmd, 400,"snmpset -v1 -c public %s:%d " MIBACTIVATE "%d i 1",(char_t *)cfg.localSignalControllerIP, cfg.snmpPort, laneID);
#if defined(DEBUG_SRMMC)
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"SNMPD1[%s]\n",cmd);
#endif

#if defined(MY_UNIT_TEST) /* These system calls will always fail. */
    /*system(cmd); */
    memset(cmd,'\0',sizeof(char) * 400);
    snprintf(cmd,400, "snmpset -v1 -c public %s:%d " MIBETA "%d i %d",(char_t *)cfg.localSignalControllerIP, cfg.snmpPort, laneID, seconds);
    /*system(cmd);*/
#else
    if (!system(cmd)) {
        /* proceed to send eta */
        memset(cmd,'\0',sizeof(char) * 400);
        snprintf(cmd,400, "snmpset -v1 -c public %s:%d " MIBETA "%d i %d",(char_t *)cfg.localSignalControllerIP, cfg.snmpPort, laneID, seconds);
        #if defined(DEBUG_SRMMC)
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"SNMPD2[%s]\n",cmd);
        #endif

        if (system(cmd)) {
            if (++srmmclastwritecnt < 10) {
                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"SCS [%u]: SRM to McCain error -> failed eta snmp send (%d)\n", temptime, laneID);
            }
            set_my_error_state(SCS_SRMMC_MIBACTIVE_ERROR);
            scs_srm_rx_err_counts++;
        }
    } else {
        if (++srmmclastwritecnt < 10) {
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"SCS [%u]: SRM to McCain error -> failed activate snmp send (%d)\n", temptime, laneID);
        }
        set_my_error_state(SCS_SRMMC_MIBETA_ERROR);
        scs_srm_rx_err_counts++;
    }
#endif /* MY_UNIT_TEST */

#if 0 /*STC work around: Preserve project history */
    laneID = 2;  /* second strategy */
    sprintf(cmd, "snmpset -v1 -c public %s:%d " MIBACTIVATE "%d i 1",(char_t *)cfg.localSignalControllerIP, cfg.snmpPort, laneID);
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"%s\n", cmd);
    if (!system(cmd)) {
        /* proceed to send eta */
        sprintf(cmd, "snmpset -v1 -c public %s:%d " MIBETA "%d i %d",(char_t *)cfg.localSignalControllerIP, cfg.snmpPort, laneID, seconds);
        if (system(cmd)) {
            if (srmmclastwritecnt < 10) {
                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"SCS [%u]: SRM to McCain error -> failed eta snmp send (%d)\n", temptime, laneID);
            }
        }
        if (srmmclastwritecnt < 10) {
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"SCS [%u]: SRM to McCain error -> failed activate snmp send (%d)\n", temptime, laneID);
        }
    }
#endif
}
#undef MIBACTIVATE
#undef MIBETA
/*
 * srmReadRec()
 * Read a SRM record from playback file
 */
#if !defined(MY_UNIT_TEST)
bool_t srmReadRec(FILE *fp, SRMMsgType *SRMMsg)
{

  int32_t dummy_int;

    if ( (fp == NULL) || (SRMMsg == NULL) ){
        return WFALSE;
    }
            
    /* Read header - MAC will be ignored and replaced with actual mac_id */
    fscanf(fp, "%12s,%d,%hhu,%u,%hhu", mac_id, &SRMMsg->seqNum, &SRMMsg->msgVersion, &SRMMsg->timeStamp, &SRMMsg->msgType);

    /* Read SRM data */
    fscanf(fp, ",%u",  &SRMMsg->timeStamp);
    fscanf(fp, ",%hhu",  &SRMMsg->msgCount);
    fscanf(fp, ",%hu",  &SRMMsg->intID);
    fscanf(fp, ",%hhu",  &SRMMsg->reqID);
    fscanf(fp, ",%hhu",  &SRMMsg->priReqType);
    fscanf(fp, ",%u",  &SRMMsg->etaMinute);
    fscanf(fp, ",%hu",  &SRMMsg->etaSeconds);
    fscanf(fp, ",%hu",  &SRMMsg->etaDuration);
    fscanf(fp, ",%u",  &SRMMsg->vehID);
    fscanf(fp, ",%u",  &SRMMsg->stationID);
    fscanf(fp, ",%u",  &SRMMsg->vehRole);
    fscanf(fp, ",%u",  &SRMMsg->vehLatitude);
    fscanf(fp, ",%u",  &SRMMsg->vehLongitude);
    fscanf(fp, ",%u",  &SRMMsg->vehElevation);
    fscanf(fp, ",%d",  &dummy_int);

    SRMMsg->vehHeading = (double)dummy_int;

    return WTRUE;
}
#endif

/* 
 * Is this better to do in SCS before it goes out to world?
 * If srmrx.c is the only place created then better there?
 * But if the source is perhaps from elsewhere better to have SCS verify?
 */
/* SEE: RSU Signal Request Message Interface_v1.3.pdf 
 *
 * Will validate and CORRECT(in most cases?) values in SRM.
 * Ideally we never get invalid data from OBU but....
 * Return bitmask of errors found.
 *
 * I don't like changing incoming data when we are just the conduit. 
 * Masks problem that exists elsewhere.
 */
STATIC uint32_t validate_srm_msg(SRMMsgType * local_SRMMsg)
{
  uint32_t ret = SRM_VERIFY_AOK; /* SUCCESS till proven FAIL. */

 /* msgType = 1 */
    if(NULL == local_SRMMsg) {
        ret = SRM_VERIFY_NULL_INPUT;
    } else {

        /*****************************/ 
        /* GENERAL section: from RSU */
        /*****************************/

        /*
         * RSU ID: Assuming all zeros or all ones is "bad"?
         */
        if(   (0x0 == local_SRMMsg->rsuID[0])
           && (0x0 == local_SRMMsg->rsuID[1])
           && (0x0 == local_SRMMsg->rsuID[2])
           && (0x0 == local_SRMMsg->rsuID[3])
           && (0x0 == local_SRMMsg->rsuID[4])
           && (0x0 == local_SRMMsg->rsuID[5])) {
            ret |= SRM_VERIFY_BAD_RSU_ID;
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"validate_srm_msg: SRM_VERIFY_BAD_RSU_ID: all zeros.\n");
        }

        if(   (0xFF == local_SRMMsg->rsuID[0])
           && (0xFF == local_SRMMsg->rsuID[1])
           && (0xFF == local_SRMMsg->rsuID[2])
           && (0xFF == local_SRMMsg->rsuID[3])
           && (0xFF == local_SRMMsg->rsuID[4])
           && (0xFF == local_SRMMsg->rsuID[5])) {
            ret |= SRM_VERIFY_BAD_RSU_ID;
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"validate_srm_msg: SRM_VERIFY_BAD_RSU_ID: all 0xFF.\n");
        }

        /*
         * Seq #: 
         */
        if(0 == local_SRMMsg->seqNum) {

            local_SRMMsg->seqNum = 1; /* To FIX OR NOT? */

            ret |= SRM_VERIFY_BAD_SEQ_NUM;
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"validate_srm_msg: SRM_VERIFY_BAD_SEQ_NUM == 0.\n");
        }

        /*
         * Message Version & Type:
         */
        if(    (SRM_MESSAGE_VERSION != local_SRMMsg->msgVersion)
            && (SRM_MESSAGE_TYPE    != local_SRMMsg->msgType)) {

            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"validate_srm_msg: SRM_VERIFY_BAD_VERSION_TYPE[v,t] = [%d,%d]\n",local_SRMMsg->msgVersion,local_SRMMsg->msgType);
            ret |= SRM_VERIFY_BAD_VERSION_TYPE;
        }

        /*
         * Timestamp: Set at time of tx. So will not be set yet.
         *          : Set it here just in case. Worst case over-written on tx.
         */
        if( local_SRMMsg->timeStamp < UTC_AT_AUTHOR_TIME ) {
            //I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"validate_srm_msg: SRM_VERIFY_BAD_TIMESTAMP(UTC) = [%d]\n",local_SRMMsg->timeStamp);
            local_SRMMsg->timeStamp = v_gettime(NULL);    /* get timestamp */
            //ret |= SRM_VERIFY_BAD_TIMESTAMP;
        }

        /***********************************************/
        /* SRM SECTION: From OBU. To correct or not?   */ 
        /*            : It could be competitor OBU has */
        /*            : things we consider violations? */
        /***********************************************/

        /*  
         * Message Timestamp Check: Minute of the year: Does not account for leap second
         */
        if(SRM_MESSAGE_MUY_MAX < local_SRMMsg->msgTimeStamp) {
          //I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"validate_srm_msg: SRM_VERIFY_BAD_MESSAGE_TIMESTAMP(MOY) = [%lu]\n",local_SRMMsg->msgTimeStamp);
           ret |= SRM_VERIFY_BAD_MESSAGE_TIMESTAMP;
        }
              
        if (local_SRMMsg->msgSecond <= SRM_MESSAGE_SECOND_MAX) {

            /* Leap Second is valid. */
            if(    (SRM_MESSAGE_LEAP_SEC_START <= local_SRMMsg->msgSecond)
                && (local_SRMMsg->msgSecond <= SRM_MESSAGE_SECOND_RESERVE_STOP)) {
               I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"validate_srm_msg:Message Second Leap Second detected=%d.\n",local_SRMMsg->msgSecond);
            }

            /* Although I know our OBU does not send this today in the future? */
            if(    (SRM_MESSAGE_SECOND_RESERVE_START <= local_SRMMsg->msgSecond)
                && (local_SRMMsg->msgSecond <= SRM_MESSAGE_SECOND_RESERVE_STOP )) {
               I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"validate_srm_msg:Message Second Reserved detected=%d.\n",local_SRMMsg->msgSecond);
            }

        } else {
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"validate_srm_msg:SRM_VERIFY_BAD_MESSAGE_SECOND = (%d)\n",local_SRMMsg->msgSecond);
            local_SRMMsg->msgSecond = SRM_MESSAGE_SECOND_UNAVAILABLE; /* mark unavailable. */
            ret |= SRM_VERIFY_BAD_MESSAGE_SECOND;
        }
  
        if (local_SRMMsg->msgCount  <= SRM_MSG_COUNT_MAX){
            /* AOK */
        } else {
            /* TROUBLE */
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"validate_srm_msg:SRM_VERIFY_BAD_MESSAGE_COUNT = (%d)\n",local_SRMMsg->msgCount);
            local_SRMMsg->msgCount = SRM_MSG_COUNT_MIN; /* Reset it or ? */
            ret |= SRM_VERIFY_BAD_MESSAGE_COUNT;
        }

        /*  
         * IntersectionID Check: unsigned 16 bit with value of 0 to 65535 means never wrong
         */
  
        /* Friendly warning: did we really mean 0x0 or 0xFFFF? */
        if(   (0x0    == local_SRMMsg->intID)
           || (0xFFFF == local_SRMMsg->intID)) {

           I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"validate_srm_msg:SRM_VERIFY_WARN_INTERSECTION_ID == (0x0 or 0xFFFF)\n");
           ret |= SRM_VERIFY_WARN_INTERSECTION_ID;
        }

        /*  
         * RequestID Check: unsigned 8 bit with value of 0 to 255 means never wrong
         *                : Unique to what?
         *                : could check for stuck count but....
         */
           
        /* Friendly warning: did we really mean 0x0 or 0xFF? */
        if(   (0x0    == local_SRMMsg->reqID)
           || (0xFF == local_SRMMsg->reqID)) {

           I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"validate_srm_msg:SRM_VERIFY_WARN_REQ_ID == (0x0 or 0xFFFF)\n");
           ret |= SRM_VERIFY_WARN_REQ_ID;
        }

        /*  
         * Priority Request Type Check: 
         */
        switch(local_SRMMsg->priReqType) {
            case SRM_REQ_TYPE_RESERVED:
            case SRM_REQ_TYPE_PRIORITY_REQUEST:
            case SRM_REQ_TYPE_PRIORITY_UPDATE:
            case SRM_REQ_TYPE_PRIORITY_CANCELATION:
                /* AOK */
                break;
            default:
                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"validate_srm_msg: SRM_VERIFY_BAD_PRIORITY_REQUEST_TYPE = (%d)\n",local_SRMMsg->priReqType);
                ret |= SRM_VERIFY_BAD_PRIORITY_REQUEST_TYPE;

/* fix it: does no harm to cancel bad request? */
                local_SRMMsg->priReqType = SRM_REQ_TYPE_PRIORITY_CANCELATION;
                break;
        }

        /*  
         * InBound - LaneID (approach or lane depending): unsigned 8 bit value always correct?
         */

        /* Friendly warning: did we really mean 0x0 or 0xFF? */
        if(   (0x0    == local_SRMMsg->inBoundLaneID)
           || (0xFF   == local_SRMMsg->inBoundLaneID)) {

           I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"validate_srm_msg: SRM_VERIFY_WARN_INBOUND_LANE == (0x%x)\n",local_SRMMsg->inBoundLaneID);
           ret |= SRM_VERIFY_WARN_INBOUND_LANE;
        }

        /*  
         * ETA Minute Check:
         */
        if(SRM_MESSAGE_MUY_MAX < local_SRMMsg->etaMinute) {
           I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"validate_srm_msg: SRM_VERIFY_BAD_ETA_MUY = [%lu]\n",local_SRMMsg->etaMinute);
           ret |= SRM_VERIFY_BAD_ETA_MUY;
        }
      
        /* Leap Second is valid. */
        if(    (SRM_MESSAGE_LEAP_SEC_START <= local_SRMMsg->etaSeconds)
            && (local_SRMMsg->etaSeconds <= SRM_MESSAGE_SECOND_RESERVE_STOP)) {
           I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"validate_srm_msg:ETA Seconds Leap Second detected=%d.\n",local_SRMMsg->etaSeconds);
        }

        /* Although I know our OBU does not send this today in the future? */
        if(    (SRM_MESSAGE_SECOND_RESERVE_START <= local_SRMMsg->etaSeconds)
            && (local_SRMMsg->etaSeconds <= SRM_MESSAGE_SECOND_RESERVE_STOP )) {
           I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"validate_srm_msg:ETA Seconds  Reserved detected=%d.\n",local_SRMMsg->etaSeconds);
        }

        /*  
         * VehicleID Check: unsigned 16 bit with value of 0 to 65535 means never wrong
         *                : depending on SRM could be USA vs Europe. We assume USA 
         *                : NOTE: Europe = stationID; Requestor vehicle station ID 
         */
  
        /* Friendly warning: did we really mean 0x0 or 0xFFFF? */
        if(   (0x0        == local_SRMMsg->vehID)
           || (0xFFFFFFFF == local_SRMMsg->vehID)) {

           I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"validate_srm_msg: SRM_VERIFY_WARN_VEHICLE_ID == (0x%x)\n",local_SRMMsg->vehID);
           ret |= SRM_VERIFY_WARN_VEHICLE_ID;
        }

        /*  
         * Vehicle Role Check: unsigned 16 bit with value of 0 to 65535 means never wrong
         */

        switch(local_SRMMsg->vehRole) {
            case basicVehicle:
            case publicTransport: 
            case specialTransport: 
            case dangerousGoods: 
            case roadWork: 
            case roadRescue: 
            case emergency: 
            case safetyCar: 
            case noneunknown: 
            case truck:
            case motorcycle: 
            case roadSideSource: 
            case police: 
            case fire: 
            case ambulance:
            case dot: 
            case transit: 
            case slowMoving: 
            case stopNgo: 
            case cyclist: 
            case pedestrian:
            case nonMotorized: 
            case military:
                /* AOK */
                break;
            default:
                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"validate_srm_msg: SRM_VERIFY_BAD_VEHICLE_ROLE = (%d)\n",local_SRMMsg->vehRole);
                ret |= SRM_VERIFY_BAD_VEHICLE_ROLE;

/* fix it: Set to lowest ranking role type? */

                local_SRMMsg->vehRole = basicVehicle;
                break;
        }

        /*
         * SRM Lat:
         */
        if(    (local_SRMMsg->vehLatitude < SRM_LAT_MIN)
            || (SRM_LAT_MAX < local_SRMMsg->vehLatitude)) {

            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"validate_srm_msg: SRM_VERIFY_BAD_LAT= [%ld]\n",local_SRMMsg->vehLatitude);
            ret |= SRM_VERIFY_BAD_LAT;
        }

        /*
         * SRM Lon:
         */
        if(    (local_SRMMsg->vehLongitude < SRM_LON_MIN)
            || (SRM_LON_MAX < local_SRMMsg->vehLongitude)) {

            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"validate_srm_msg: SRM_VERIFY_BAD_LON = [%ld]\n",local_SRMMsg->vehLongitude);
            ret |= SRM_VERIFY_BAD_LON;
        }

        /*
         * SRM ELV:
         */
        if(    (local_SRMMsg->vehElevation < SRM_ELV_MIN)
            || (SRM_ELV_MAX < local_SRMMsg->vehElevation)) {

            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"validate_srm_msg: SRM_VERIFY_BAD_ELV = [%d]\n",local_SRMMsg->vehElevation);
            ret |= SRM_VERIFY_BAD_ELV;
        }

        /*
         * SRM HDG:
         */
        if(    ((int32_t)(local_SRMMsg->vehHeading) < SRM_HDG_ANGLE_MIN)
            || (SRM_HDG_ANGLE_MAX < (int32_t)(local_SRMMsg->vehHeading))) {

            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"validate_srm_msg: SRM_VERIFY_BAD_HDG = [%d]\n",(int32_t)(local_SRMMsg->vehHeading));
            ret |= SRM_VERIFY_BAD_HDG;
        }
   }
   return ret;
}

/*
 * scsProcessSRM()
 * Process an SRM message from srm_rx
 */
void scsProcessSRM(int *srmSocket)
{
    static uint32_t msgSeqNum = 0;
    char_t  uper_payload[MAX_WSM_DATA];
    uint32_t uper_payload_len;
    double tti = 0.0; /* seconds with 3(0.001) decimal of precision */
    double current_time = 0.0;
    uint32_t ret_val = 0x0;

#ifdef EXTRA_EXTRA_DEBUG
   I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Got SRM trigger.\n");
#endif
    if(NULL == shm_ptr) {
        if(0x0 == (my_error_states & SCS_SHM_INIT_FAIL)){
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"scsProcessSRM:SCS_SHM_INIT_FAIL .\n");
            set_my_error_state(SCS_SHM_INIT_FAIL);
        }
        return;
    }
    /* Get SRM from sharemem pipe */
    wsu_shmlock_lockr(&shm_ptr->scsSRMData.h.ch_lock);
    memcpy(&SRMMsg, &shm_ptr->scsSRMData.srm_msg, shm_ptr->scsSRMData.srm_len);
    memcpy(uper_payload, &shm_ptr->scsSRMData.uper_payload, shm_ptr->scsSRMData.uper_payload_len);
    uper_payload_len = shm_ptr->scsSRMData.uper_payload_len;
    wsu_shmlock_unlockr(&shm_ptr->scsSRMData.h.ch_lock);
    /* Good or bad we are getting something. */
    scs_srm_rx_counts++;

    /* Header */
    memcpy(SRMMsg.rsuID, rsuMacAddr, 6);    /* RSU ID */
    SRMMsg.msgVersion = MSG_VERSION_SRM;    /* Version number */
    SRMMsg.msgType = MSG_TYPE_SRM;      /* SRM Msg type */

    if (cfg.requireTTI) {
        /* Convert to seconds */
        current_time = (((double)(SRMMsg.msgSecond))/1000.0);
        current_time += (60.0 * (double)(SRMMsg.msgTimeStamp));  

        tti = (((double)(SRMMsg.etaSeconds))/1000.0);
        tti += (60.0 * (double)(SRMMsg.etaMinute));

        /* Now subtract timestamp from eta to get tti */
        if(tti < current_time) { /* ETA NEGATIVE */

            #if defined(DEBUG_SRMMC)
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"ERROR: tti(%lf) < current_time(%lf).\n",current_time,tti);
            #endif
               
        } else {
            if( tti == current_time ) { /* ETA = ZERO */
                #if defined(DEBUG_SRMMC)
                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"ERROR: tti(%lf) == current_time(%lf).\n",current_time,tti);
                #endif
            } else {
                tti = tti - current_time;

                #if defined(DEBUG_SRMMC)
                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"tti = (%lf)\n",tti);
                #endif

                #if 0 /* Is there too large in general sense? */
                if(255.0 < tti) { /* ETA TOO LARGE */
                   #if defined(DEBUG_SRMMC)
                   I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"ERROR: 255.0 < tti(%lf).\n",tti);
                   #endif
                }
                #endif                         
            }
        }
    }

    /* Inc SeqNum. Reset if it rolled. */
    msgSeqNum++;
    if(0 == msgSeqNum) {
        msgSeqNum = 1;
    } 
    SRMMsg.seqNum = msgSeqNum;

    /* 
     * Range check before this Msg goes any further. 
     * what do to if junk in one value?
     * Do we be ruthless and reject malformed SRM's from OBU?
     * Or do we correct parts that are out of range?
     * i.e. bad lat/lon/hdg? if RSU is in permissive then location doesn't matter.
     */
    if (SRM_VERIFY_AOK != (ret_val = validate_srm_msg(&SRMMsg))) {
        #if defined(EXTRA_DEBUG) /* Depending on OBU this could be alot! */
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"WARNING : validate_srm_msg = (0x%x)\n",ret_val); 
        #endif
        scs_srm_rx_err_counts++;
        set_my_error_state(SCS_SRM_MALFORMED);
    }

    /* SRM to Controller Format */
    /* Each type may or may not care about eta(tti) so let them decide. */
    switch (cfg.SRMFwdInterface) {
        case 1:
            /* GridSmart: Send 'pretty' decoded CSV file out port to TC or Gridsmart or McCain or hub? */
            scsSendSRM(srmSocket, &SRMMsg);
            break;
        case 2:
            /* McCain */
            //scsMcCainSendSRM(srmSocket, &SRMMsg);
            /* because truncating to 16bits could 
             * create accidental valid ETA we check.
             */
            if((cfg.requireTTI) && ((0.0 == tti) || ( 255.0 < tti))) {
#if defined(DEBUG_SRMMC)
                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"rejecting tti=(%lf).\n",tti);    
#endif
                set_my_error_state(SCS_SRM_BAD_TTI);
                scs_srm_rx_err_counts++; 
            } else {
                scsSRMMC(SRMMsg.inBoundLaneID, (uint16_t)tti);    /* etaMinute + etaSeconds */
            }
            break;
        case 3:
            /* J2735 2016 UPER encoded message */
            scsSendSRM_Raw(srmSocket, uper_payload, uper_payload_len);
            break;
        default:
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"SRM Fwd Interface not defined! \n");
            set_my_error_state(SCS_SRM_BAD_INTERFACE);
            scs_srm_rx_err_counts++; 
            break;
    }

}

/*
 * SRM_Fwd_Thread() 
 * Thread to forward SRM packet to controller
 */
void *SRM_Fwd_Thread(void *arg) 
{
    int *srmSocket = (int *)arg;
#if !defined(MY_UNIT_TEST)
    FILE *srm_csv_fp = NULL;
    uint32_t rec_count = 0;
#endif
    
    /* Header */
    memcpy(SRMMsg.rsuID, rsuMacAddr, LENGTH_MAC);     /* RSU ID */
    SRMMsg.msgVersion = MSG_VERSION_SRM;     /* Version number */
    SRMMsg.msgType = MSG_TYPE_SRM;       /* SRM Msg type */
    SRMMsg.seqNum  = 1;                      /* Start sequence number at 1 */
    
#if !defined(MY_UNIT_TEST)
    /* Test replay mode */
    if (cfg.SRMFwdTest) {
        srm_csv_fp = fopen((const char_t *)cfg.SRMFile, "r") ; 

        if(NULL == srm_csv_fp) {
            set_my_error_state(SCS_SRM_CSV_FOPEN_ERROR);
        }
        #ifdef EXTRA_DEBUG
        if (srm_csv_fp != NULL) {
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"open %s OK\n", cfg.SRMFile);
        } else {
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"open %s Failed!\n", cfg.SRMFile);
        }
        #endif
    }
#endif
#if defined(EXTRA_DEBUG)
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"SRM_Fwd_Thread: Entering mainloop.\n");
#endif
    while (mainloop) {

#if !defined(MY_UNIT_TEST)
        if (mainloop == WFALSE) {
            break;
        }

        if (cfg.SRMFwdTest) {
            /* Replay Mode - GridSmart only */
            if (srmReadRec(srm_csv_fp, &SRMMsg)){
                scsSendSRM(srmSocket, &SRMMsg);
                rec_count++;
                if (rec_count >= SRM_CSV_LOOP_COUNT) {
                   /* reset counter and fp */
                    rec_count = 0;
                    fseek(srm_csv_fp, 0, SEEK_SET);
                }
            }

            SRMMsg.seqNum++;
            usleep(cfg.SRMFwdInterval * SECOND_TO_MILLI);

        } else 
#endif
        {
            /* wait for trigger from srm_rx */
            if(NULL != shm_ptr)
                wsu_wait_at_gate(&shm_ptr->scsSRMData.srmTriggerGate);
            scsProcessSRM(srmSocket);
        }
#if defined(MY_UNIT_TEST)
        mainloop = WFALSE;
        break; /* force one iteration only. */
#endif
    }

    return NULL;
}

void scsSendHeartBeat(int *gs2Socket, hbMsgType *hbMsg)
{
    struct sockaddr_in remote;
    int32_t remote_len = sizeof(remote);
    char_t hbMsgCSV[256];       /* testing */
    
    if(    (NULL == gs2Socket)
        || (NULL == hbMsg) ) {
        set_my_error_state(SCS_SRM_HEARTBEAT_INPUT_ERROR);
        return;
    }

    if(-1 == *gs2Socket) {
        /* Not ready back off. */
        set_my_error_state(SCS_SRM_HEARTBEAT_SOCKET_NOT_READY);
        return;
    }

    memset(&remote, 0, sizeof(remote));
    remote.sin_family = AF_INET;
    remote.sin_port = htons(cfg.SRMFwdPort);
    remote.sin_addr.s_addr = inet_addr((const char_t *)cfg.SRMFwdIP);

    /* Contruct heartbeat message CSV format */
    snprintf(hbMsgCSV,sizeof(hbMsgCSV), "%02X%02X%02X%02X%02X%02X,%d,%d,%u,%d", hbMsg->rsuID[0], hbMsg->rsuID[1], hbMsg->rsuID[2],
            hbMsg->rsuID[3], hbMsg->rsuID[4], hbMsg->rsuID[5], hbMsg->seqNum, hbMsg->msgVersion,
            hbMsg->timeStamp, hbMsg->msgType);
    
    if(-1 == sendto(*gs2Socket, hbMsgCSV, strlen(hbMsgCSV), 0, (struct sockaddr *)&remote, remote_len)){
        set_my_error_state(SCS_SRM_HEARTBEAT_SEND_FAIL);
    }
#ifdef EXTRA_DEBUG
    else I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"sending heartbeat:%s\n", hbMsgCSV);
#endif
    return;
}
#ifdef EXTRA_DEBUG
static wtime prior_heartbeat = 0x0;
#endif

void *HeartBeatThread(void *arg) 
{
    int32_t *outSocket = (int32_t *)arg;
    hbMsgType hbMsg;
    
    memcpy(hbMsg.rsuID, rsuMacAddr, 6);     /* RSU ID */
    hbMsg.msgVersion = MSG_VERSION_HB;     /* Version number */
    hbMsg.msgType = MSG_TYPE_HB;        /* Heartbeat msg type */
    hbMsg.seqNum  = 1;                      /* Start sequence number at 1 */
    
#ifdef EXTRA_DEBUG
    prior_heartbeat = v_gettime(NULL);
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"HeartBeatThread","Entering mainloop.\n");
#endif
    while (mainloop) {

#if !defined(MY_UNIT_TEST)
        if (mainloop == WFALSE) {
            break;
        }
#endif
        // update info here
        hbMsg.timeStamp = v_gettime(NULL);

#ifdef EXTRA_DEBUG
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"Heartbeat:delta=%lu\n", hbMsg.timeStamp - prior_heartbeat );
        prior_heartbeat = hbMsg.timeStamp;
#endif

        scsSendHeartBeat(outSocket, &hbMsg);
        hbMsg.seqNum++;
        sleep(cfg.RSUHeartBeatInt); /* in seconds */

#if defined(MY_UNIT_TEST)
        mainloop = WFALSE;/* force one iteration only. */
#endif
    }

    return NULL;
}


#ifdef GS2_SUPPORT
void scsProcessGS2OutboundMessage(int32 *gs2Socket)
{
    enum GS2Request request;
#if 0
    unsigned long long currentTime;
#endif
    int32_t id = 0;
    uint8_t approachID = 0;
    uint8_t gs2Buffer[GS2_BUF_SIZE];
    bool_t  sendMessage = WFALSE;

    if(NULL == gs2Socket) {
        return;
    }

    if(-1 == *gs2Socket) {
        return;
    }

    wsu_shmlock_lockr(&shm_ptr->gs2OutboundData.h.ch_lock);
    request = shm_ptr->gs2OutboundData.request;
    switch (request) {
        case NAK:
            id = shm_ptr->gs2OutboundData.id;
            break;

        case DilemmaZoneSet:
            approachID = shm_ptr->gs2OutboundData.approachID;
            id = shm_ptr->gs2OutboundData.id;
            break;

        case DilemmaZoneClear:
            approachID = shm_ptr->gs2OutboundData.approachID;
            id = shm_ptr->gs2OutboundData.id;
            break;
        default:
            break;
    }
    wsu_shmlock_unlockr(&shm_ptr->gs2OutboundData.h.ch_lock);

#if 0
    currentTime = v_gettime(NULL); 
#endif

    switch (request) {
        case NAK:
            if (cfg.GS2DemoDebugLevel > 3) {
                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"sending GS2 NAK message: id: %d\n", id);
            }
            sprintf(gs2Buffer, "NAK:%d;", id);
            sendMessage = WTRUE;
            break;

        case DilemmaZoneSet:
            // Send TCP message to the GS2
            if (cfg.GS2DemoDebugLevel > 3) {
                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"sending GS2 Dilemma Zone Set message: id: %d approachID: %d\n", id, approachID);
            }

            // In the demo the approach is not used
            sprintf(gs2Buffer, "DZS:%d;", id);
            sendMessage = WTRUE;
            break;

        case DilemmaZoneClear:
            // Send TCP message to the GS2
            if (cfg.GS2DemoDebugLevel > 3) {
                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"sending GS2 Dilemma Zone Clear message: id: %d approachID: %d\n", id, approachID);
            }

            // In the demo the approach is not used
            sprintf(gs2Buffer, "DZC:%d;", id);
            sendMessage = WTRUE;
            break;

        default:
            break;
    }
    if (sendMessage) {
        struct sockaddr_in remote;
        memset(&remote, 0, sizeof(remote));
        remote.sin_family = AF_INET;
        remote.sin_port = htons(cfg.localGS2Port);
        remote.sin_addr.s_addr = inet_addr(cfg.localGS2IP);
        int remote_len = sizeof(remote);

        sendto(*gs2Socket, gs2Buffer, strlen(gs2Buffer), 0, (struct sockaddr *)&remote, remote_len);
    }
}

void *GS2OutboundMessageThread(void *arg) 
{
    int32_t *gs2Socket = (int32_t *)arg;
#if defined(EXTRA_DEBUG)
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"GS2OutboundMessageThread","Entering mainloop.\n");
#endif
    while (mainloop) {

        wsu_wait_at_gate(&shm_ptr->gs2OutboundData.triggerGate);
#if !defined(MY_UNIT_TEST)
        if (mainloop == WFALSE) {
            break;
        }
#endif
        scsProcessGS2OutboundMessage(gs2Socket);
#if defined(MY_UNIT_TEST)
        break; /* force one iteration only. */
#endif
    }

    return NULL;
}
#endif
STATIC uint32_t scsSetConfDefault(char_t * tag, void * itemToUpdate)
{
  uint32_t ret = I2V_RETURN_OK; /* Success till proven fail. */

  if((NULL == tag) || (NULL == itemToUpdate)) {
      /* Nothing to do. */
      I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"scsetConfDefault: NULL input.\n");
      set_my_error_state(SCS_HEAL_NULL_INPUT);
      ret = I2V_RETURN_NULL_PTR;
  } else {
      if(0 == strcmp("InterfaceTOM",tag)) {
          *(uint8_t *)itemToUpdate = SCS_INTERFACE_TOM_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%u)\n",tag,*(uint8_t *)itemToUpdate);
      } else if(0 == strcmp("UseMaxTimeToChange",tag)) {
          *(bool_t *)itemToUpdate = SCS_USE_MAX_TIMETO_CHANGE_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%u)\n",tag,*(bool_t *)itemToUpdate);
      } else if(0 == strcmp("RSUHeartBeatEnable",tag)) {
          *(bool_t *)itemToUpdate = SCS_HEARTBEAT_ENABLE_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%u)\n",tag,*(bool_t *)itemToUpdate);
      } else if(0 == strcmp("RSUHeartBeatInt",tag)) {
          *(uint16_t *)itemToUpdate = SRMRX_HEARTBEAT_INTERVAL_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%u)\n",tag,*(uint16_t *)itemToUpdate)
      } else if(0 == strcmp("SRMFwdTest",tag)) {
          *(bool_t *)itemToUpdate = SRMRX_FWD_TEST_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%u)\n",tag,*(bool_t *)itemToUpdate);
      } else if(0 == strcmp("SRMFile",tag)) {
          strcpy((char_t *)itemToUpdate,SRMRX_FWD_TEST_FILE_S);
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%s)\n",tag,(char_t *)itemToUpdate);
      } else if(0 == strcmp("SRMFwdEnable",tag)) {
          *(bool_t *)itemToUpdate = SRMRX_FWD_ENABLE_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%u)\n",tag,*(bool_t *)itemToUpdate);
      } else if(0 == strcmp("SRMFwdInterval",tag)) {
          *(uint16_t *)itemToUpdate = SRMRX_FWD_INTERVAL_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%u)\n",tag,*(uint16_t *)itemToUpdate);
      } else if(0 == strcmp("SRMFwdIP",tag)) {
          strcpy((char_t *)itemToUpdate,SRMRX_FWD_IP_DEFAULT_S);
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%s)\n",tag,(char_t *)itemToUpdate);
      } else if(0 == strcmp("SRMFwdPort",tag)) {
          *(uint16_t *)itemToUpdate = SRMRX_FWD_PORT_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%u)\n",tag,*(uint16_t *)itemToUpdate);
      } else if(0 == strcmp("SRMFwdInterface",tag)) {
          *(uint8_t *)itemToUpdate = SRMRX_FWD_INTERFACE_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%u)\n",tag,*(uint8_t *)itemToUpdate);
      } else if(0 == strcmp("SRMFwdRequireTTI",tag)) {
          *(bool_t *)itemToUpdate = SRMRX_REQUIRE_TTI_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%u)\n",tag,*(bool_t *)itemToUpdate);
      } else if(0 == strcmp("LocalSignalControllerIP",tag)) {
          strcpy((char_t *)itemToUpdate,I2V_SCS_LOCAL_CONTROLLER_IP_DEFAULT_S);
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%s)\n",tag,(char_t *)itemToUpdate);
      } else if(0 == strcmp("LocalSignalControllerPort",tag)) {
          *(uint16_t *)itemToUpdate = I2V_SCS_LOCAL_CONTROLLER_PORT_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%u)\n",tag,*(uint16_t *)itemToUpdate);
      } else if(0 == strcmp("LocalSignalSNMPPort",tag)) {
          *(uint16_t *)itemToUpdate = I2V_SCS_LOCAL_SNMP_IP_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%u)\n",tag,*(uint16_t *)itemToUpdate);
      } else if(0 == strcmp("LocalSignalIntersectionID",tag)) {
          *(uint32_t *)itemToUpdate = SCS_LOCAL_SIGNAL_INT_ID_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%u)\n",tag,*(uint32_t *)itemToUpdate);
      } else if(0 == strcmp("LocalSignalControllerEnable",tag)) {
          *(bool_t *)itemToUpdate = SCS_LOCAL_CONTROLLER_ENABLE_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%u)\n",tag,*(bool_t *)itemToUpdate);
      } else if(0 == strcmp("LocalSignalControllerIsSnmp",tag)) {
          *(bool_t *)itemToUpdate = SCS_LOCAL_CONTROLLER_IS_SNMP_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%u)\n",tag,*(bool_t *)itemToUpdate);
      } else if(0 == strcmp("LocalSignalControllerHW",tag)) {
          *(uint8_t *)itemToUpdate = SCS_LOCAL_SIGNAL_HW_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%u)\n",tag,*(uint8_t *)itemToUpdate);
      } else if(0 == strcmp("BypassSignalControllerNTCIP",tag)) {
          *(bool_t *)itemToUpdate = SCS_BYPASS_SIGNAL_CONTROLLER_NTCIP_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%u)\n",tag,*(bool_t *)itemToUpdate);
      } else if(0 == strcmp("BypassYellowDuration",tag)) {
          *(uint8_t *)itemToUpdate = SCS_BYPASS_YELLOW_DURATION_DEFAULT;
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%u)\n",tag,*(uint8_t *)itemToUpdate);
      } else if(0 == strcmp("SPATEnableValue",tag)) {
          strcpy((char_t *)itemToUpdate,SCS_SPAT_COMMAND_DEFAULT_S);
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%s)\n",tag,(char_t *)itemToUpdate);
      } else if(0 == strcmp("SNMPEnableSpatCommand",tag)) {
          strcpy((char_t *)itemToUpdate,SCS_SPAT_SNMP_ENABLE_COMMAND_DEFAULT_S);
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%s)\n",tag,(char_t *)itemToUpdate);
      } else if(0 == strcmp("NTCIP_YELLOW_DURATION",tag)) {
          strcpy((char_t *)itemToUpdate,NTCIP_YELLOW_DURATION_DEFAULT_S);
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%s)\n",tag,(char_t *)itemToUpdate);
      } else if(0 == strcmp("NTCIP_PHASE_OPTIONS",tag)) {
          strcpy((char_t *)itemToUpdate,NTCIP_PHASE_OPTIONS_DEFAULT_S);
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%s)\n",tag,(char_t *)itemToUpdate);
      } else if(0 == strcmp("NTCIP_PHASE_GREENS",tag)) {
          strcpy((char_t *)itemToUpdate,NTCIP_PHASE_GREENS_DEFAULT_S);
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%s)\n",tag,(char_t *)itemToUpdate);
      } else if(0 == strcmp("NTCIP_PHASE_YELLOWS",tag)) {
          strcpy((char_t *)itemToUpdate,NTCIP_PHASE_YELLOWS_DEFAULT_S);
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%s)\n",tag,(char_t *)itemToUpdate);
      } else if(0 == strcmp("NTCIP_PHASE_NUMBER",tag)) {
          strcpy((char_t *)itemToUpdate,NTCIP_PHASE_NUMBER_DEFAULT_S);
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%s)\n",tag,(char_t *)itemToUpdate);
      } else if(0 == strcmp("NTCIP_PHASE_MAX_COUNTDOWN",tag)) {
          strcpy((char_t *)itemToUpdate,NTCIP_PHASE_MAX_COUNTDOWN_DEFAULT_S);
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%s)\n",tag,(char_t *)itemToUpdate);
      } else if(0 == strcmp("NTCIP_PHASE_MIN_COUNTDOWN",tag)) {
          strcpy((char_t *)itemToUpdate,NTCIP_PHASE_MIN_COUNTDOWN_DEFAULT_S);
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"(%s) set to (%s)\n",tag,(char_t *)itemToUpdate);
      } else {
          /* Nothing to do. */
          I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"scsSetConfDefault:(%s) item is not known, ignoring.\n",tag);
          set_my_error_state(SCS_HEAL_FAIL);
          ret = I2V_RETURN_UNKNOWN;
      }
  }
  return ret;
}
/* this function will parse the scs config file - each rse app will manage its unique config data
   instead of globbing everything into a single overall config file */
/* TODO: need to upgrade scs logging from wsu_* in general then fix wrapper input to parseconf fxn */
static bool_t scs_update_local_cfg(void)
{
    cfgItemsTypeT cfgItems[] = {
        {"InterfaceTOM",               (void *)i2vUtilUpdateWBOOLValue,  &cfg.tomformat, NULL,(ITEM_VALID| UNINIT)},
        {"UseMaxTimeToChange",         (void *)i2vUtilUpdateWBOOLValue,  &cfg.useMaxTTC, NULL,(ITEM_VALID| UNINIT)},
#ifdef GS2_SUPPORT
        {"LocalGS2Enable",             (void *)i2vUtilUpdateWBOOLValue,  &cfg.ifcGS2, NULL,(ITEM_VALID| UNINIT)},
        {"LocalGS2IP",                 (void *)i2vUtilUpdateStrValue,    &cfg.localGS2IP, NULL,(ITEM_VALID| UNINIT)},
        {"LocalGS2Port",               (void *)i2vUtilUpdateUint16Value, &cfg.localGS2Port, NULL,(ITEM_VALID| UNINIT)},
        {"GS2DemoDebugLevel",          (void *)i2vUtilUpdateUint32Value, &cfg.GS2DemoDebugLevel, NULL,(ITEM_VALID| UNINIT)},
#endif
        {"RSUHeartBeatEnable",         (void *)i2vUtilUpdateWBOOLValue,  &cfg.RSUHeartBeatEnable, NULL,(ITEM_VALID| UNINIT)},
        {"RSUHeartBeatInt",            (void *)i2vUtilUpdateUint16Value, &cfg.RSUHeartBeatInt, NULL,(ITEM_VALID| UNINIT)},
        {"SRMFwdTest",                 (void *)i2vUtilUpdateWBOOLValue,  &cfg.SRMFwdTest, NULL,(ITEM_VALID| UNINIT)},
        {"SRMFile",                    (void *)i2vUtilUpdateStrValue,    &cfg.SRMFile, NULL,(ITEM_VALID| UNINIT)},
        {"SRMFwdEnable",               (void *)i2vUtilUpdateWBOOLValue,  &cfg.SRMFwdEnable, NULL,(ITEM_VALID| UNINIT)},
        {"SRMFwdInterval",             (void *)i2vUtilUpdateUint16Value, &cfg.SRMFwdInterval, NULL,(ITEM_VALID| UNINIT)},
        {"SRMFwdIP",                   (void *)i2vUtilUpdateStrValue,    &cfg.SRMFwdIP, NULL,(ITEM_VALID| UNINIT)},
        {"SRMFwdPort",                 (void *)i2vUtilUpdateUint16Value, &cfg.SRMFwdPort, NULL,(ITEM_VALID| UNINIT)},
        {"SRMFwdInterface",            (void *)i2vUtilUpdateUint8Value,  &cfg.SRMFwdInterface, NULL,(ITEM_VALID| UNINIT)},
        {"SRMFwdRequireTTI",           (void *)i2vUtilUpdateWBOOLValue,  &cfg.requireTTI, NULL,(ITEM_VALID| UNINIT)},
        {"LocalSignalControllerIP",    (void *)i2vUtilUpdateStrValue,    &cfg.localSignalControllerIP, NULL,(ITEM_VALID| UNINIT)},
        {"LocalSignalControllerPort",  (void *)i2vUtilUpdateUint16Value, &cfg.localSignalControllerPort, NULL,(ITEM_VALID| UNINIT)},
        {"LocalSignalSNMPPort",        (void *)i2vUtilUpdateUint16Value, &cfg.snmpPort, NULL,(ITEM_VALID| UNINIT)},
        {"LocalSignalIntersectionID",  (void *)i2vUtilUpdateUint32Value, &cfg.spat_intersection_id, NULL,(ITEM_VALID| UNINIT)},
        {"LocalSignalControllerEnable",(void *)i2vUtilUpdateWBOOLValue,  &cfg.ifcSCS, NULL,(ITEM_VALID| UNINIT)},
        {"LocalSignalControllerIsSnmp",(void *)i2vUtilUpdateWBOOLValue,  &cfg.useSnmpScs, NULL,(ITEM_VALID| UNINIT)},
        {"LocalSignalControllerHW",    (void *)i2vUtilUpdateUint8Value,  &cfg.localSignalControllerHW, NULL,(ITEM_VALID| UNINIT)},
        {"BypassSignalControllerNTCIP",(void *)i2vUtilUpdateWBOOLValue,  &cfg.nontcip, NULL,(ITEM_VALID| UNINIT)},
        {"BypassYellowDuration",       (void *)i2vUtilUpdateUint8Value,  &cfg.staticyeldur, NULL,(ITEM_VALID| UNINIT)},
/* this goes ahead in case someone adds longer string than supported - it will be overwritten correctly by next item in list */
        {"SPATEnableValue",            (void *)i2vUtilUpdateStrValue,    &cfg.enable_spat_val, NULL,(ITEM_VALID| UNINIT)}, 
        {"SNMPEnableSpatCommand",      (void *)i2vUtilUpdateStrValue,    &cfg.enable_spat, NULL,(ITEM_VALID| UNINIT)},
        {"NTCIP_YELLOW_DURATION",      (void *)i2vUtilUpdateStrValue,    &cfg.yellowDurOID, NULL,(ITEM_VALID| UNINIT)},
        {"NTCIP_PHASE_OPTIONS",        (void *)i2vUtilUpdateStrValue,    &cfg.phaseOptsOID, NULL,(ITEM_VALID| UNINIT)},
        {"NTCIP_PHASE_GREENS",         (void *)i2vUtilUpdateStrValue,    &cfg.phaseGrnOID, NULL,(ITEM_VALID| UNINIT)},
        {"NTCIP_PHASE_YELLOWS",        (void *)i2vUtilUpdateStrValue,    &cfg.phaseYlwOID, NULL,(ITEM_VALID| UNINIT)},
        {"NTCIP_PHASE_NUMBER",         (void *)i2vUtilUpdateStrValue,    &cfg.phaseNumOID, NULL,(ITEM_VALID| UNINIT)},
        {"NTCIP_PHASE_MAX_COUNTDOWN",  (void *)i2vUtilUpdateStrValue,    &cfg.phaseMaxCntdnOID, NULL,(ITEM_VALID| UNINIT)},
        {"NTCIP_PHASE_MIN_COUNTDOWN",  (void *)i2vUtilUpdateStrValue,    &cfg.phaseMinCntdnOID, NULL,(ITEM_VALID| UNINIT)},
    };
    FILE *f;
    char fileloc[I2V_CFG_MAX_STR_LEN + I2V_CFG_MAX_STR_LEN];  /* these already have space for null term string */
    uint32_t retVal;
#ifndef RSE_SIMULATOR
    cfgItemsT parentcfg;
#endif
    uint32_t i;

    if(NULL == shm_ptr) {
        if(0x0 == (my_error_states & SCS_SHM_INIT_FAIL)){
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"scs_update_local_cfg:SCS_SHM_INIT_FAIL .\n");
            set_my_error_state(SCS_SHM_INIT_FAIL);
        }
        return WFALSE;
    }

    /* Wait on I2V SHM to get I2V CFG.*/
    for(i=0;mainloop && (i < MAX_I2V_SHM_WAIT_ITERATIONS); i++){
        wsu_shmlock_lockr(&shm_ptr->cfgData.h.ch_lock);

        if(WTRUE == shm_ptr->cfgData.h.ch_data_valid) {
            memcpy(&parentcfg, &shm_ptr->cfgData, sizeof(cfgItemsT));  /* save config for later use */
            wsu_shmlock_unlockr(&shm_ptr->cfgData.h.ch_lock);
            break;
        }
        wsu_shmlock_unlockr(&shm_ptr->cfgData.h.ch_lock);
        usleep(MAX_i2V_SHM_WAIT_USEC);
    }

    if((WFALSE == mainloop) || (MAX_I2V_SHM_WAIT_ITERATIONS <= i)){
        return WFALSE;
    }

    /* At this point shared memory should be ready. Read fwdmsgConfig for getting fwdmsgMask */
    /* Wait on I2V SHM to get FWDMSG CFG.*/
    for(i=0; mainloop && (i < MAX_I2V_SHM_WAIT_ITERATIONS); i++){
      WSU_SEM_LOCKR(&shm_ptr->fwdmsgCfgData.h.ch_lock);

      if(WTRUE == shm_ptr->fwdmsgCfgData.h.ch_data_valid) {
          /* Copy fwdmsgCfg to local structure */
          memcpy(&fwdmsgCfg.fwdmsgAddr, &shm_ptr->fwdmsgCfgData.fwdmsgAddr, sizeof(fwdmsgCfg.fwdmsgAddr));  
          WSU_SEM_UNLOCKR(&shm_ptr->fwdmsgCfgData.h.ch_lock);
          break;
      }

      WSU_SEM_UNLOCKR(&shm_ptr->fwdmsgCfgData.h.ch_lock);
      usleep(MAX_i2V_SHM_WAIT_USEC);
    }

    if((WFALSE == mainloop) || (MAX_I2V_SHM_WAIT_ITERATIONS <= i)){
        return WFALSE;
    }

    /* Check from fwdmsgCfg if SPaT/BSM/PSMBSM need to forwarded */
    for (i = 0; i < MAX_FWD_MSG_IP; i++)  {
        if (fwdmsgCfg.fwdmsgAddr[i].fwdmsgEnable)   {

            /* SRM */
            if ((fwdmsgCfg.fwdmsgAddr[i].fwdmsgMask) & 0x20)
                srmForwardMsg = WTRUE;
        }
    }

    strncpy(fileloc, parentcfg.config_directory, sizeof(fileloc));
    strncat(fileloc, confFileName, sizeof(fileloc)-1);

    if ((f = fopen(fileloc, "r")) == NULL)
    {
        set_my_error_state(SCS_CONF_FOPEN_ERROR);
        return WFALSE;
    }
    if(I2V_RETURN_OK != (retVal = i2vUtilParseConfFile(f, cfgItems, NUMITEMS(cfgItems), WFALSE, NULL))) {
        I2V_DBG_LOG(LEVEL_CRIT,MY_NAME,"i2vUtilParseConfFile: not happy i2vRet=%u.\n",retVal);    
        set_my_error_state(SCS_CONF_PARSE_ERROR);
    }
    fclose(f);

    //TODO: Make these new cfgitem types and put in i2v_util.c.
    /* Check IP's or anything else for extra wellness above what I2V_UTIL does. */
    if(I2V_RETURN_OK != i2vCheckIPAddr(cfg.SRMFwdIP, I2V_IP_ADDR_V4)) {
        cfgItems[8].state = (ITEM_VALID|BAD_VAL);
        retVal = I2V_RETURN_INVALID_CONFIG;
        set_my_error_state(SCS_CONF_PARSE_ERROR); /* Note for user. */
    }
    if(I2V_RETURN_OK != i2vCheckIPAddr(cfg.localSignalControllerIP, I2V_IP_ADDR_V4)) {
        cfgItems[12].state = (ITEM_VALID|BAD_VAL);
        retVal = I2V_RETURN_INVALID_CONFIG;
        set_my_error_state(SCS_CONF_PARSE_ERROR); /* Note for user. */
    }
    for (i = 0; i < NUMITEMS(cfgItems); i++) {
        if (   ((ITEM_VALID|BAD_VAL) == cfgItems[i].state) 
            || ((ITEM_VALID|UNINIT) == cfgItems[i].state)) {
            I2V_DBG_LOG(LEVEL_CRIT,MY_NAME,"config override on (%s): Using default.\n", cfgItems[i].tag);
            if(I2V_RETURN_OK == (retVal = scsSetConfDefault(cfgItems[i].tag, cfgItems[i].itemToUpdate))) {
                cfgItems[i].state = (ITEM_VALID|INIT);
                set_my_error_state(SCS_HEAL_CFGITEM); /* Note for user. */
            } else {
                break; /* Heal has failed. FATAL. */
            }
        }
    }

#ifdef GS2_SUPPORT
#ifdef GS2_SUPPORT_DISABLE_ALL_DEBUG
    cfg.GS2DemoDebugLevel = 0;
#endif
#endif

#if defined(MY_UNIT_TEST) // Could swap conf file at start but this works and is easy.

    if(1 == get_stub_signal(Signal_disable_scs)) {
        clear_stub_signal(Signal_disable_scs);
        cfg.ifcSCS = WFALSE;
    }

    if(1 == get_stub_signal(Signal_enable_Snmpscs)) {
        clear_stub_signal(Signal_enable_Snmpscs);
        cfg.useSnmpScs = WTRUE;
    }

    my_test_counter++;
#endif

    /* Copy the local cfg values into shared mem */
    wsu_shmlock_lockw(&shm_ptr->scsCfgData.h.ch_lock);
    memcpy(&cfg.h,&shm_ptr->scsCfgData.h,sizeof(wsu_shm_hdr_t));  /* preserve header contents */
#ifdef RSE_SIMULATOR
    if (!parentcfg.spsAppEnable) {
        /* do not mark valid until after init of sock */
        cfg.h.ch_data_valid = WTRUE;
    }
#else
    cfg.h.ch_data_valid = WTRUE;
#endif /* RSE_SIMULATOR */
    memcpy(&shm_ptr->scsCfgData,&cfg,sizeof(cfg));
    wsu_shmlock_unlockw(&shm_ptr->scsCfgData.h.ch_lock);

    return ((retVal==I2V_RETURN_OK)?WTRUE:WFALSE);
}

/* to reduce LOC in main */
STATIC void scsMain(int32_t scs_sock, int32_t gs2Socket)
{
    // MFM - we don't need to use ifdefs for GS2_SUPPORT here as gs2Socket
    // will be zero if GS2_SUPPORT is not true or EnableGS2 is false. This
    // de-clutters the code
    int32_t spat_len = 0;
    uint8_t spat_buf[SPAT_BUF_SIZE];
#ifdef GS2_SUPPORT
    int32_t gs2Length = 0;
    uint8_t gs2Buffer[GS2_BUF_SIZE];
#endif
    struct sockaddr_in remote;
    socklen_t remote_len = sizeof(remote);
    bool_t haveYellowDur = WFALSE;
    bool_t scsMessage = WFALSE;
#ifdef GS2_SUPPORT
    bool_t gs2Message = WFALSE;
#endif
    fd_set read_fds;
    fd_set error_fds;
    struct timeval t = {0, 0};
    int32_t n = 0;
    uint32_t debug_counter=0; /* rolling counter.*/

    gs2Socket = gs2Socket;
/* 
 * Assumption: These must be valid?
 *           : Maybe not if !cfg.ifcSCS?
 *           : 0 is legit value for socket?
 */
#if defined(MY_UNIT_TEST)
    if(   (scs_sock < 0)
       && (gs2Socket < 0)){

        #if defined(EXTRA_DEBUG)
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"scsMain: Input sockets look closed? < 1.\n");
        #endif
    }
#endif
    if(NULL == shm_ptr) {
        if(0x0 == (my_error_states & SCS_SHM_INIT_FAIL)){
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"scsMain:SCS_SHM_INIT_FAIL .\n");
            set_my_error_state(SCS_SHM_INIT_FAIL);
        }
        return;
    }
    /* With the econolite I must do this first.
     * Otherwise select always returns 1 and I get no snmp set at all.
     */
    if ((mainloop) && (cfg.ifcSCS) && (!cfg.nontcip)){
      scs_send_snmp_cmd((char_t *)cfg.enable_spat, "i", (char_t *)cfg.enable_spat_val, cfg.snmpPort);  
    }
#if defined(EXTRA_DEBUG)
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"scsMain: Entering mainloop.\n");
#endif
    while (mainloop) {
        if(0 == (debug_counter % OUTPUT_MODULUS)) {
            I2V_DBG_LOG(LEVEL_INFO,MY_NAME,"(0x%lx) processed = %u: srm rx,E(%u, %u)\n",
                my_error_states,scs_processed_cnt,scs_srm_rx_counts, scs_srm_rx_err_counts);
        }
        if(0 == (debug_counter % 1200)) {
            dump_scs_report();
        }
        debug_counter++;
#if defined(MY_UNIT_TEST)
        if(1 < debug_counter) {
            mainloop = WFALSE;
            break;
        }
        sleep(1);
#endif
        /* do nothing if scs interface disabled */
#ifdef RSE_SIMULATOR
        if (!parentcfg.spsAppEnable) {
#endif /* RSE_SIMULATOR */
            if (!cfg.ifcSCS) {
                usleep(1000000);
                if (!mainloop) {
                    break;
                }
                continue;
            }

            // select wants the highest socket number + 1
            int maxSocket = scs_sock;
#ifdef GS2_SUPPORT
            if (gs2Socket > scs_sock)
                maxSocket = gs2Socket;
#endif
            /* 20210407 (JJG): Thanks ML! -> the FD_ZERO below is mandatory for error_fds
               to fix the CPU utilization issue reported in 20210406 comments now removed
               from below; reverting the rest of the code to its original form as well as
               removing the code that was tweaked and now no longer needed
            */            
            DN_FD_ZERO(&read_fds);
            DN_FD_ZERO(&error_fds);
            DN_FD_SET(scs_sock, &read_fds);
#ifdef GS2_SUPPORT
            if (gs2Socket > 0) {
                DN_FD_SET(gs2Socket, &read_fds);
            }
#endif
            t.tv_sec = 0; /* zero second timeout */
            t.tv_usec = 100000;  /* Wait for 100ms */
            n = dn_select(maxSocket + 1, &read_fds, NULL, &error_fds, &t);
#if !defined(FYA_TEST)
            if (n == -1) { /* error */
                set_my_error_state(SCS_MAIN_SELECT_ERROR);
                continue;
            } else if (n == 0) { /* timeout */
                /* If spat has not been received even once, we assume that the traffic controller
                 * does not have SPAT messages enabled and so try to send it the SNMP command
                 * every 100ms until we get atleast one spat message. (The SNMP SPAT ON
                 * command to econolite traffic controller ASC3 is persistent on reboot)
                 */
                continue;
            } else {
                scsMessage = (DN_FD_ISSET(scs_sock, &read_fds) ? WTRUE : WFALSE);
#ifdef GS2_SUPPORT
                gs2Message = (DN_FD_ISSET(gs2Socket, &read_fds) ? WTRUE : WFALSE);
#endif

                if (!scsMessage
#ifdef GS2_SUPPORT
                     && !gs2Message
#endif
                ) {
                    continue;
                }
            }
#endif /*FYA_TEST*/

            if (!cfg.tomformat && !haveYellowDur
#ifdef GS2_SUPPORT
                && !gs2Message
#endif
            ) {
                if (cfg.nontcip) {
                    icdYellowDur = cfg.staticyeldur;
                    haveYellowDur = WTRUE;
                } else {
                    haveYellowDur = icdGetYelDur(cfg.localSignalControllerIP, cfg.snmpPort, cfg.yellowDurOID);
                }

                if(WFALSE == icdYellowDur) {
                    set_my_error_state(SCS_MAIN_YELLOW_DURATION_ERROR);
                }
#ifdef EXTRA_DEBUG
                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"get yel dur result %d\n", haveYellowDur);
#endif /* EXTRA_DEBUG */
            }

//Fake it till we make it
#if defined(FYA_TEST)

        /*fixed*/
        //my_fya_test = 35; 
        if(120 < my_fya_test) {
            my_fya_test = 0;
        }
        memset(spat_buf,0x0,sizeof(spat_buf));
        
        if( my_fya_test < 30) {
            if (cfg.localSignalControllerHW) {
                memcpy(spat_buf,tw_steady_green_ovr_8,   sizeof(tw_steady_green_ovr_8));      
            } else {
                memcpy(spat_buf,icd_steady_green_ovr_8,   sizeof(icd_steady_green_ovr_8));
            }
        }

        if( (30 <= my_fya_test) && (my_fya_test < 60)) {
            if (cfg.localSignalControllerHW) {
                memcpy(spat_buf,tw_flashing_yellow_ovr_8,sizeof(tw_flashing_yellow_ovr_8));
            } else {
                memcpy(spat_buf,icd_flashing_yellow_ovr_8,sizeof(icd_flashing_yellow_ovr_8));
            }
        }

        if( (60 <= my_fya_test) && (my_fya_test < 90)) {
            if (cfg.localSignalControllerHW) {
                memcpy(spat_buf,tw_steady_yellow_ovr_8,  sizeof(tw_steady_yellow_ovr_8));
            } else {
                memcpy(spat_buf,icd_steady_yellow_ovr_8,  sizeof(icd_steady_yellow_ovr_8));
            }
        }

        if( (90 <= my_fya_test) && (my_fya_test < 120)) {
            if (cfg.localSignalControllerHW) {
                memcpy(spat_buf,tw_steady_red_ovr_8,     sizeof(tw_steady_red_ovr_8));
            } else {
                memcpy(spat_buf,icd_steady_red_ovr_8,     sizeof(icd_steady_red_ovr_8));
            }
        }
        spat_len = 1; //make it have a length
        my_fya_test++;

        scsMessage = WTRUE;
        usleep(1000 * 100);
#else

            if (scsMessage) {
                /* receive SPAT messages */
                if ((spat_len = dn_recvfrom(scs_sock, spat_buf, SPAT_BUF_SIZE, 0, 
                                (struct sockaddr *)&remote, &remote_len)) == -1) {
                    /* recvfrom should return -1 on receiving SIGINT/SIGTERM and also on
                     * other error conditions.
                     */
                    set_my_error_state(SCS_MAIN_RECVFROM_ERROR);
#if !defined(MY_UNIT_TEST)
                    if (mainloop == WFALSE) {
                        break;
                    }
#endif
                }

                /* Check if the SPAT message is from local Signal Controller. 
                 * This is check is necessitated as all Signal Controllers in the 
                 * network broadcast the SPAT. We should only listen to the Signal 
                 * Controller we're connected to. 
                 * This check is added as part of PEAGLE_ITS_WC design. */

                if ((strcmp (dn_inet_ntoa(remote.sin_addr),(char_t *)cfg.localSignalControllerIP)) == 0 )
                {
#endif  /*FYA_TEST*/
                    if (spat_len > 0) {
#ifdef SPAT_EXTRA_DEBUG
                        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"processing spat buf of %d bytes from %s\n", spat_len, inet_ntoa(remote.sin_addr));
#ifdef SPAT_EXTRA_EXTRA_DEBUG
                        {
                            int i;
                            for (i=0; i<243; i++) {
                                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"%.2X ", spat_buf[i]);
                                if (i && (i%20 == 19)) I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"\n");
                            }
                            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"\n");
                        }
#endif // SPAT_EXTRA_EXTRA_DEBUG

#endif /* SPAT_EXTRA_DEBUG */
                        if (scs_process_spat(spat_buf) == 0) {
                            /* notify SPAT */
                            wsu_open_gate(&shm_ptr->scsSpatData.spatTriggerGate);
                            scs_processed_cnt++;
#if 0
                            /* legacy code maintained for documentation */
                            /* notify peagle */
                            wsu_open_gate(&shm_ptr->peagle_scs_req.peagleTriggerGate);
#endif
                        } else {
                            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"0 != scs_process_spat.\n");
                            set_my_error_state(SCS_MAIN_SPAT_PROCESS_ERROR);
                        }
                    } else {
                        set_my_error_state(SCS_MAIN_SPAT_LENGTH_ERROR);
                    }
#if !defined(FYA_TEST)
                } else {
                    set_my_error_state(SCS_MAIN_CONTROLLER_IP_ERROR);
                }
            }
#endif /* FYA_TEST */
#ifdef GS2_SUPPORT
            if (gs2Message) {
                /* receive GS2 messages */
                if ((gs2Length = recvfrom(gs2Socket, gs2Buffer, GS2_BUF_SIZE, 0,
                                (struct sockaddr *)&remote, &remote_len)) == -1) {
                    /* recv should return -1 on receiving SIGINT/SIGTERM and also on
                     * other error conditions.
                     */
                    if (mainloop == WFALSE) {
                        break;
                    }
                }
                if (gs2Length > 0) {
#ifdef GS2_EXTRA_EXTRA_DEBUG
                    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"processing GS2 buf of %d bytes\n", gs2Length);
                    {
                        int i;
                        for (i=0; i<gs2Length; i++) {
                            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"%.2X \n", gs2Buffer[i]);
                            if (i && (i%20 == 19)) I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"\n");
                        }
                        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"\n");
                    }
#endif // GS2_EXTRA_EXTRA_DEBUG
                    // We ensure the buffer is null terminated
                    gs2Buffer[gs2Length] = 0;
                    // Note that this routine opens the appropriate gate
                    scs_process_gs2(gs2Buffer);
                }
            }
#endif // GS2_SUPPORT
#ifdef RSE_SIMULATOR
        } else {
            if ((spat_len = read(scs_sock, spat_buf, SPAT_BUF_SIZE)) > 0) {
                if (scs_process_spat(spat_buf) == 0) {
                    wsu_open_gate(&shm_ptr->scsSpatData.spatTriggerGate);
                    scs_processed_cnt++;
                }
            }
#ifdef GS2_SUPPORT
            if ((gs2Length = read(gs2Socket, gs2Buffer, GS2_BUF_SIZE)) > 0) {
                // Note that this routine opens the appropriate gate
                scs_process_gs2(gs2Buffer);
            }
#endif // GS2_SUPPORT
            usleep(1000);
            continue;
        }
#endif /* RSE_SIMULATOR */
    }
    return;
}

/* 20180213: JJG - new main handler for SNMP approach for obtaining SPaT data;
   only one mechanism for obtaining SPaT is supported; no TOM support
   FIXME: no GS2 support; */
STATIC void scsSNMPMain(void)
{
    bool_t haveYellowDur = WFALSE;
    uint64_t timetrack = 0, timecount = 0;
    i2vReturnTypesT ret = I2V_RETURN_OK;
    uint32_t debug_counter=0; /* rolling counter.*/

    if(NULL == shm_ptr) {
        if(0x0 == (my_error_states & SCS_SHM_INIT_FAIL)){
            I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"scsSNMPMain:SCS_SHM_INIT_FAIL .\n");
            set_my_error_state(SCS_SHM_INIT_FAIL);
        }
        return;
    }
     /* there is no socket required; just making SNMP queries */
    if (!haveYellowDur && cfg.ifcSCS) {
        haveYellowDur = icdGetYelDur(cfg.localSignalControllerIP, cfg.snmpPort, cfg.yellowDurOID);
#ifdef EXTRA_DEBUG
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"get yel dur result %d\n", haveYellowDur);
#endif /* EXTRA_DEBUG */
    }
#if defined(EXTRA_DEBUG)
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"scsSNMPMain","Entering mainloop.\n");
#endif
    while(mainloop) {
        if(0 == (debug_counter % OUTPUT_MODULUS)) {
            I2V_DBG_LOG(LEVEL_INFO,MY_NAME,"(0x%lx) processed = %u: srm rx,E(%u, %u)\n",
                my_error_states,scs_processed_cnt,scs_srm_rx_counts, scs_srm_rx_err_counts);
        }
        if(0 == (debug_counter % 1200)) {
            dump_scs_report();
        }
        debug_counter++;
        if (!cfg.ifcSCS) {
            usleep(1000000);
#if defined(MY_UNIT_TEST)
            mainloop = WFALSE;
            break; /* One and done, */
#endif
            continue;
        }

        /* FIXME: need to have a control mechanism for frequency of task; for now
           assumes a 10 hz rate; or the best that can be done with the number of snmp commands */
        timetrack = i2vUtilGetTimeInMs();
        memset(&spat_info, 0, sizeof(spat_info));
        if (I2V_RETURN_OK == ( ret = icdParseSNMPSpat(&spat_info.flexSpat, &cfg))) {
            /* Write the parsed SPAT message info to shared memory */
            wsu_shmlock_lockw(&shm_ptr->scsSpatData.h.ch_lock);
            memcpy(&shm_ptr->scsSpatData.spat_info, &spat_info, sizeof(spat_info));
            shm_ptr->scsSpatData.spat_info.flexSpat.intID = cfg.spat_intersection_id;
            shm_ptr->scsSpatData.spatSelector = !cfg.tomformat;
            shm_ptr->scsSpatData.h.ch_data_valid = WTRUE;
            wsu_shmlock_unlockw(&shm_ptr->scsSpatData.h.ch_lock);
            /* notify SPAT */
            wsu_open_gate(&shm_ptr->scsSpatData.spatTriggerGate);
            scs_processed_cnt++;
        } else {
            /* else? - data_valid set true will never be cleared; for now just stale data, but maybe clear it */
            if(I2V_RETURN_MISSING_DATA == ret) {
                set_my_error_state(SCS_SNMP_MAIN_MISSING_DATA_ERROR);
            } else {
                set_my_error_state(SCS_SNMP_MAIN_PARSE_ERROR);
            }
        }

        /* iteration control - assuming 10 hz rate */
        if (((timecount = i2vUtilGetTimeInMs()) - timetrack) < 100) {
            usleep((timecount - timetrack) * 1000);
        }
#if defined(MY_UNIT_TEST)
        mainloop = WFALSE;
        break; /* force one iteration only. */
#endif
    }
}

int32_t MAIN(void)
{
    int32_t ret = SCS_AOK; /* For future more elaborate return values. */
    int32_t scs_sock = -1;
    // We always define this to make the scsMain function signature consistent whether GS2 is enabled or not
    int32_t gs2Socket = -1;
#ifdef GS2_SUPPORT
    pthread_t GS2OutboundMessageThreadID;
#endif
    int32_t srmSocket = -1;
    pthread_t hbMsgThreadID;         /* Heart beat Msg */
    pthread_t SrmFwdThreadID;        /* SRM FWD */
    struct sigaction sa;

    scs_static_init();

    /* Enable serial debug with I2V_DBG_LOG until i2v.conf::globalDebugFlag says otherwise. */
    i2vUtilEnableDebug(MY_NAME);
    /* LOGMGR assumed up by now. Could retry if fails. */
    if(0 != i2vUtilEnableSyslog(MY_ERR_LEVEL, MY_NAME)){
        I2V_DBG_LOG(LEVEL_WARN,MY_NAME,"Failed to open syslog. Only serial output available,\n");
    }

    shm_ptr = wsu_share_init(sizeof(i2vShmMasterT), I2V_SHM_PATH);
    if (shm_ptr == NULL) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"critical error: Main Shared memory init failed\n");
        return SCS_SHM_INIT_FAIL; /* FATAL: Good bye cruel world. */
    }

    if ((SCS_AOK == ret) && (!scs_update_local_cfg())) {
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"SCS config failure\n");
        ret = SCS_LOAD_CONF_FAIL;
    }

    if(SCS_AOK == ret) {
        /* catch SIGINT/SIGTERM */
        memset(&sa, 0, sizeof(sa));
        sa.sa_handler = scs_sighandler;
        sigaction(SIGINT, &sa, NULL);
        sigaction(SIGTERM, &sa, NULL);

#ifdef RSE_SIMULATOR
        if (!parentcfg.spsAppEnable) {
#endif /* RSE_SIMULATOR */
            if (cfg.ifcSCS && !cfg.useSnmpScs) {
                if ((scs_sock = scs_init_tc_sock()) == -1) {
                    ret = SCS_INIT_TC_SOCK_FAIL;
                }
            } else if (!cfg.ifcSCS) {
                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"SCS disabling signal controller interface\n");
            }

            /* snmp scs continues */
#ifdef GS2_SUPPORT
            if ((SCS_AOK == ret) && (cfg.ifcGS2)) {
                /* Get MAC address to be used as ID */
                if (!scs_get_mac_addr(RSU_ETHERNET, rsuMacAddr)){
                    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"scs_get_mac_addr for %s failed!\n",RSU_ETHERNET);
                } else {
                    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"%s MacAddr: %02X:%02X:%02X:%02X:%02X:%02X\n",RSU_ETHERNET,
                    rsuMacAddr[0], rsuMacAddr[1], rsuMacAddr[2],
                    rsuMacAddr[3], rsuMacAddr[4], rsuMacAddr[5]);
                }
            
                // We don't fail if we can't contact the server immediately
                if ((gs2Socket = scs_init_gs2_sock()) == -1) {
                    //ret = SCS_INIT_GS2_SOCK_FAIL;
                } else {

                    if (dn_pthread_create(&GS2OutboundMessageThreadID, NULL, GS2OutboundMessageThread, &gs2Socket) == -1) {
                        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"SCS : GS2 thread create failed.\n");
                        ret = SCS_CREATE_GS2_THREAD_FAIL;
                    }
                }
                /* 20190208: QNX pthread_join fails; detach instead (will properly exit) */
                pthread_detach(GS2OutboundMessageThreadID);
            } else {
                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"SCS disabling GS2 interface\n");
            }
#endif
#ifdef RSE_SIMULATOR
        } else {
            if ((scs_sock = scs_init_tc_sock()) == -1) {
                ret = SCS_INIT_TC_SOCK_FAIL;
            } else {
#ifdef GS2_SUPPORT
                // We don't fail if we can't contact the server immediately
                if ((SCS_AOK == ret) && ((gs2Socket = scs_init_gs2_sock()) == -1)) {
                    //ret = SCS_INIT_GS2_SOCK_FAIl;
                }
#endif
                wsu_shmlock_lockw(&shm_ptr->scsCfgData.h.ch_lock);
                shm_ptr->scsCfgData.h.ch_data_valid = WTRUE;
                wsu_shmlock_unlockw(&shm_ptr->scsCfgData.h.ch_lock);
                /* just for consistency */
                cfg.h.ch_data_valid = WTRUE;
                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"ignoring cfg setting for signal controller interface\n");
            }
        }
#endif /* RSE_SIMULATOR */

        /*
         * Get MAC once.
         */
        if((SCS_AOK == ret) && (WTRUE != scs_get_mac_addr(RSU_ETHERNET, rsuMacAddr))){
            /* This will raise a validation error on SRM CSV file? */
            memset(rsuMacAddr,0xff,sizeof(rsuMacAddr)); /*error. limp along or return -1?*/
        }

        /*
         * Socket for srm
         */
        if ((SCS_AOK == ret) && ((srmSocket = scs_init_srm_sock()) == -1)) {
            ret = SCS_INIT_SRM_SOCK_FAIL;
        }

        /* Open the message queue for fwdmsg */
        if (srmForwardMsg)  {
            if ((fwdmsgfd = mq_open(I2V_FWDMSG_QUEUE, O_RDWR|O_NONBLOCK)) == -1) {
                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME," mq() failed to open: errno = %d(%s)\n",errno,strerror(errno)); 
                set_my_error_state(SCS_SRM_OPEN_MQ_FAIL);
            }
        }

        /* SRM Forward thread */
        if ((SCS_AOK == ret) && (cfg.SRMFwdEnable)){
            if (dn_pthread_create(&SrmFwdThreadID, NULL, SRM_Fwd_Thread, &srmSocket) == -1) {
                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"SCS : SRM_Fwd_Thread create failed.\n");
                ret = SCS_CREATE_SRM_THREAD_FAIL;
            }
            /* 20190208: QNX pthread_join fails; detach instead (will properly exit) */
            pthread_detach(SrmFwdThreadID);
        }

        /* Heart beat */
        if ((SCS_AOK == ret) && (cfg.RSUHeartBeatEnable)) {
            if (dn_pthread_create(&hbMsgThreadID, NULL, HeartBeatThread, &srmSocket) == -1) {
                I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"SCS : HeartBeatThread thread create failed.\n");
                ret = SCS_CREATE_SRM_HEARTB_FAIL;
            }
            /* 20190208: QNX pthread_join fails; detach instead (will properly exit) */
            pthread_detach(hbMsgThreadID);
        }

        /* 
         * This is mainloop. Will exit on mainloop false.
         */
        if(SCS_AOK == ret) { 
            if (cfg.useSnmpScs) {
                scsSNMPMain();
            } else {
                scsMain(scs_sock, gs2Socket);
            }
        }
    } /* SCS_AOK == ret */

    /* 
     * Closing time. 
     */
    if( 0 < srmSocket) {
        close(srmSocket);
        srmSocket = -1;
    }
    if( 0 < scs_sock) {
        close(scs_sock);
        scs_sock = -1;
    }
    if( 0 < gs2Socket) {
        close(gs2Socket);
        gs2Socket = -1;
    }
#if defined(EXTRA_DEBUG)
    I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"EXIT: ret(%d)(0x%lx)\n",ret, my_error_states);
#endif
    /* Stop I2V_DBG_LOG output. Last chance to output to syslog. */
    i2vUtilDisableSyslog();
    i2vUtilDisableDebug();
    if(NULL != shm_ptr) {
        wsu_share_kill(shm_ptr, sizeof(i2vShmMasterT));
        shm_ptr = NULL;
    }
    return abs(ret);
}
