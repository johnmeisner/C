/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Filename: ris_dbg.h                                                        */
/*                                                                            */
/* Copyright (C) 2019 DENSO International America, Inc.                       */
/*                                                                            */
/* Description: Radio Interface Services (RIS API for Radio Services)         */
/*              debugging                                                     */
/* Revision History:                                                          */
/*                                                                            */
/* Date        Author        Comments                                         */
/* -------------------------------------------------------------------------- */
/* [2019-03-07][VROLLINGER]  Initial revision                                 */
/*----------------------------------------------------------------------------*/
#ifndef __RIS_DBG_H
#define __RIS_DBG_H

/*----------------------------------------------------------------------------*/
/* Macros                                                                     */
/*----------------------------------------------------------------------------*/

/* 
 * Ideally the calling function or low layer prints debug but not here. 
 * No I2V syslogs from here to control.
 */
#if 0 /* Under normal circumstances this does not need to be enabled. */
#define RIS_DEBUG
#endif

#ifdef RIS_DEBUG
#define RIS_PRINTF(fmt, args...) \
    { \
        printf("RIS %s %s: " fmt, __FILE__, __func__, ##args); \
        fflush(stdout); \
    }

#define RIS_DBG(fmt, args...) \
    { \
        printf("RIS %s %s: " fmt, __FILE__, __func__, ##args); \
        fflush(stdout); \
    }

#define RIS_PRINTF_MAX_N_TIMES(n, fmt, args...) \
{ \
    static int counter = 0; \
    if (counter < (n)) { \
        printf("RIS %s %s: " fmt, __FILE__, __func__, ##args); \
        fflush (stdout); \
        counter++; \
    } \
}

#define RIS_DBG_MAX_N_TIMES(n, fmt, args...) \
{ \
    static int counter = 0; \
    if (counter < (n)) { \
        RIS_DBG(fmt, ##args); \
        counter++; \
    } \
}
#else 
#define RIS_PRINTF(fmt, args...)
#define RIS_DBG(fmt, args...)
#define RIS_PRINTF_MAX_N_TIMES(n, fmt, args...)
#define RIS_DBG_MAX_N_TIMES(n, fmt, args...)
#endif

#endif // __RIS_DBG_H

