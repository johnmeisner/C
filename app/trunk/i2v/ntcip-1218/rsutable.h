/**************************************************************************
 *                                                                        *
 *     File Name:  rsuTable.h                                             *
 *     Author:                                                            *
 *         DENSO International America, Inc.                              *
 *         North America Research Laboratory, California Office           *
 *         Rutherford Road                                                *
 *         Carlsbad, 92008                                                *
 **************************************************************************/
#ifndef RSUTABLE_H
#define RSUTABLE_H

/* 
 * MISRA C Standard Type Definitions for 32-Bit Machine, converted to 64-bit iMX8 
 */
typedef unsigned char       bool_t;     /*!< Boolean */
typedef char                char_t;     /*!< Character */
typedef signed char         int8_t;     /*<! Signed 8-bit integer */
typedef signed short        int16_t;    /*<! Signed 16-bit integer */
typedef signed int          int32_t;    /*<! Signed 32-bit integer */
typedef signed long         int64_t;    /*<! Signed 64-bit integer */
typedef unsigned char       uint8_t;    /*<! Unsigned 8-bit integer */
typedef unsigned short      uint16_t;   /*<! Unsigned 16-bit integer */
typedef unsigned int        uint32_t;   /*<! Unsigned 32-bit integer */
typedef unsigned long       uint64_t;   /*<! Unsigned 64-bit integer */
typedef float               float32_t;  /*<! single floating-point */
typedef double              float64_t;  /*<! double floating-point */

#if defined(MY_UNIT_TEST)
#define STATIC
#else
#define STATIC static
#endif

/* For all modules. */
#define RSEMIB_OK  0

/* Limit run away request from client. */
#define MAX_REQ_COUNT (1024 * 1024) 

/* 
 * There is no main(). rsutable.c::rsuTableUpdaterThr is the closest thing. Called by init_rsutable().
 */
void init_rsutable(void); /* MIB entry point from SNMPD. DO NOT move or rename. */

/* Install MIB and open SHM. */
void install_ntcip_1218(void);

/* Close MIB and tear down SHM. */
void close_ntcip_1218(void);

#endif  /* RSUTABLE_H */
