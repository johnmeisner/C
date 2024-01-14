/*---------------------------------------------------------------------------*/
/*                                                                           */
/*    Filename: pps_mmap.h                                                   */
/*     Purpose: Define the struct to contain the PPS timestamp data          */
/*  Department: DENSO North America Research & Development                   */
/*                                                                           */
/* Copyright (C) 2015 DENSO International America, Inc.                      */
/*                                                                           */
/*     Project: V2X Special Projects                                         */
/* Description: Defines the struct to contain the PPS timestamp data         */
/*                                                                           */
/*---------------------------------------------------------------------------*/



/* Replace linux/time.h with only time.h when compiling it with TPS*/
#include<time.h>

/*The below structure should be the same in CCL pps_mmap.h also
 * Otherwise there will be mismatch, while sharing date 
 * through MMAP*/

struct pps_tmstamp
{
    unsigned int NewTimeStamp;
    struct timeval  pps_tv;
    suseconds_t   prev_pps_tv_usec;
    unsigned int count;
};

