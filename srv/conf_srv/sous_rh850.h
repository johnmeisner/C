/*---------------------------------------------------------------------------*/
/*                                                                           */
/*    Filename: sous_rh850.h                                                 */
/*     Purpose: RH850-related enums and shm data structure                   */
/*  Department: DENSO North America Research & Development                   */
/*                                                                           */
/* Copyright (C) 2022 DENSO International America, Inc.                      */
/*                                                                           */
/*     Project: V2X Special Projects                                         */
/* Description: Structure containing Shared Memory for RH850 data in SOUS    */
/*                                                                           */
/*---------------------------------------------------------------------------*/


#ifndef _SHM_SOUS_RH850_H
#define _SHM_SOUS_RH850_H

#include "dn_types.h"

// The number of AP0_# inputs we are interested in
#define NUM_RAW_RH850_DATA 5

// Antenna Connection Status Enum
enum {
    ANTCON_UNSET,
    ANTCON_SUPPLY_FAIL,
    ANTCON_NOT_CONNECTED,
    ANTCON_SHORT,
    ANTCON_CONNECTED,
    ANTCON_UNKNOWN_VALUE
};

// Structure to hold rh850 data
typedef struct
{
    uint32_t  cnt_opens;       // Number of times RH850 device has been opened & reopened
    char_t    fw_version[50];  // Hold firmware's cg_src/r_cg_main.c's FW_Version[] (20 chars @ 2022)
    int32_t   wd_timeout_count;  // Hold the number of times the unit has reset due to a watchdog timeout
    float32_t dcin_voltage;
    uint16_t  gps_ant2_status;
    uint16_t  cv2x_ant1_status;
    uint16_t  cv2x_ant2_status;
    /* Copy of raw values for testing team */
    uint32_t  raw_data[NUM_RAW_RH850_DATA];
} __attribute__((packed)) rh850_data_t;


#endif
