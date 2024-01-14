#ifndef __DN_TYPES_H__
#define __DN_TYPES_H__
/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Filename: dn_types.h                                                       */
/*                                                                            */
/* Copyright (C) 2019 DENSO International America, Inc.                       */
/*                                                                            */
/* Description: Standard MISRA C Types for DENSO's Embedded System Projects   */
/*                                                                            */
/*----------------------------------------------------------------------------*/

#ifndef FALSE
#define FALSE   0  /*!< Logic false */
#endif

#ifndef TRUE
#define TRUE    1  /*!< Logic true */
#endif


/* MISRA C Standard Type Definitions for 32-Bit Machine, converted to 64-bit iMX8 */

typedef unsigned char       bool_t;     /*!< Boolean */
typedef char                char_t;     /*!< Character */

#ifndef S_SPLINT_S
typedef signed char         int8_t;     /*<! Signed 8-bit integer */
typedef signed short        int16_t;    /*<! Signed 16-bit integer */
typedef signed int          int32_t;    /*<! Signed 32-bit integer */
typedef signed long         int64_t;    /*<! Signed 64-bit integer */

typedef unsigned char       uint8_t;    /*<! Unsigned 8-bit integer */
typedef unsigned short      uint16_t;   /*<! Unsigned 16-bit integer */
typedef unsigned int        uint32_t;   /*<! Unsigned 32-bit integer */
typedef unsigned long       uint64_t;   /*<! Unsigned 64-bit integer */
#endif

typedef float               float32_t;  /*<! single floating-point */
typedef double              float64_t;  /*<! double floating-point */

#ifdef S_SPLINT_S
#define __signed__ signed
typedef unsigned long    timer_t;
typedef va_list          __gnuc_va_list;
#endif

#define WTRUE    TRUE
#define WFALSE   FALSE

typedef bool_t   WBOOL;

typedef uint32_t wuint32;
typedef int32_t  wint32;
typedef uint16_t wuint16;
typedef int16_t  wint16;

typedef uint8_t  wuint8;
typedef int8_t   wint8;
typedef int8_t   wsint8;

typedef uint64_t wulong64;
typedef uint32_t wtime;

#if defined(MY_UNIT_TEST)
#define STATIC 
#else
#define STATIC static
#endif
#endif  /* __DN_TYPES_H__ */

