/*---------------------------------------------------------------------------*/
/*                                                                           */
/*    Filename: shm_sus.h                                                    */
/*     Purpose: SOUS shared memory definition                                */
/*  Department: DENSO North America Research & Development                   */
/*                                                                           */
/* Copyright (C) 2015 DENSO International America, Inc.                      */
/*                                                                           */
/*     Project: V2X Special Projects                                         */
/* Description: Structure containing Shared Memory for SOUS                  */
/*                                                                           */
/*---------------------------------------------------------------------------*/


#ifndef _SHM_SOUS_H
#define _SHM_SOUS_H

#define SHM_SOUS_PATH  "/shm_sous"

#include "sous_rh850.h"

typedef struct {
    uint32_t rh850_update_count;
    rh850_data_t rh850_data;
} shm_sous_t;

#endif
