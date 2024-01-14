/*---------------------------------------------------------------------------*/
/*                                                                           */
/*    Filename: shared_opaque_utility_server.h                               */
/*     Purpose: Share Global declarations to sous threads                    */
/*  Department: DENSO North America Research & Development                   */
/*                                                                           */
/* Copyright (C) 2022 DENSO International America, Inc.                      */
/*                                                                           */
/*     Project: V2X Special Projects                                         */
/* Description: Declares globals shared by all threads                       */
/*                                                                           */
/*---------------------------------------------------------------------------*/


#include "dn_types.h"
#include "shm_sous.h"

// Globals shared by all threads
extern int keep_running;
extern shm_sous_t * shm_sous_ptr;
extern int flag_debug_output;
extern int ant_broadcast_period;
extern int dcin_broadcast_period;
