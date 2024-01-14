/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Filename: rsu_defs.h                                                      */
/*                                                                            */
/* Copyright (C) 2019 DENSO International America, Inc.                       */
/*                                                                            */
/* Description: Definitions that are internal to RIS and Radio Services.      */
/*                                                                            */
/* Revision History:                                                          */
/*                                                                            */
/* Date        Author        Comments                                         */
/* -------------------------------------------------------------------------- */
/* [2019-03-22][VROLLINGER]  Initial revision                                 */
/*----------------------------------------------------------------------------*/
#ifndef __RSU_DEFS_H
#define __RSU_DEFS_H

#define RSU_PRINTF_MAX_N_TIMES(n, fmt, args...) \
{ \
    static int counter = 0; \
    if (counter < (n)) { \
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,"radioServices %s %s: " fmt, __FILE__, __func__, ##args); \
        counter++; \
    } \
}

#define RSU_DBG_MAX_N_TIMES(n, fmt, args...) \
{ \
    static int counter = 0; \
    if (counter < (n)) { \
        I2V_DBG_LOG(MY_ERR_LEVEL,MY_NAME,fmt, ##args); \
        counter++; \
    } \
}

#endif //  __RSU_DEFS_H
