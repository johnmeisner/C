/*
 * Copyright (c) 1997-2023 Objective Systems, Inc.
 *
 * This software is furnished under a license and may be used and copied
 * only in accordance with the terms of such license and with the
 * inclusion of the above copyright notice. This software or any other
 * copies thereof may not be provided or otherwise made available to any
 * other person. No title to and ownership of the software is hereby
 * transferred.
 *
 * The information in this software is subject to change without notice
 * and should not be construed as a commitment by Objective Systems, Inc.
 *
 * PROPRIETARY NOTICE
 *
 * This software is an unpublished work subject to a confidentiality agreement
 * and is protected by copyright and trade secret law.  Unauthorized copying,
 * redistribution or other use of this work is prohibited.
 *
 * The above notice of copyright on this source code product does not indicate
 * any actual or intended publication of such source code.
 *
 *****************************************************************************/
 /*The following is the text for the main C/C++ Runtime index page.*/
 /*! \mainpage C/C++ Common Runtime Classes and Library Functions
 *
 * The <B>ASN.1 C++ run-time classes</B> are wrapper classes that provide an
 * object-oriented interface to the ASN.1 C run-time library functions. The
 * categories of classes provided are as follows:
 * <UL>
 * <LI>Context Management classes manage the OSCTXT structure used to keep
 * track of the working variables required to encode or decode ASN.1
 * messages.</LI>
 * <LI>Message Buffer classes are used to manage message buffers.</LI>
 * <LI>ASN1C Control Base classes are wrapper classes that are used as the
 * base for compiler-generated ASN1C_ classes, including Date and Time
 * Run-time classes.</LI>
 * <LI>ASN.1 Type (ASN1T_) Base classes are used as the base for compiler-
 * generated ASN1T_ C++ data structures. </LI>
 * <LI>ASN.1 Stream classes are used to read and write ASN.1 messages from
 * and to files, sockets, memory buffer, etc.</LI>
 * <LI>TCP/IP or UDP Socket classes provide utility methods for doing socket
 * I/O. <LI>%Asn1NamedEventHandler classes include the base classes for
 * user-defined error handler and event handler classes.</LI> </UL>
 *
 * The <B>C run-time common library</B> contains common C functions used by
 * the encoding rules (BER/DER, PER, and XER) low-level encode/decode
 * functions. These functions are identified by their <I>rt</I> prefixes.
 * The categories of functions provided are as follows:<UL>
 * <LI>Memory Allocation macros and functions handle memory
 * management for the ASN1C run-time.</LI>
 * <LI>Context Management functions handle the allocation,
 * initialization, and destruction of context variables
 * (variables of type OSCTXT) that handle the working data used
 * during encoding or decoding a message.</LI>
 * <LI>Diagnostic Trace functions allow the output of trace messages
 * to standard output that trace the execution of complier generated
 * functions.</LI>
 * <LI>Error Formatting and Print functions allow information about
 * the encode/decode errors to be added to a context block structure
 * and printed out.</LI>
 * <LI>Memory Buffer Management functions handle the allocation,
 * expansion, and de-allocation of dynamic memory buffers used by some
 * encode/decode functions.</LI>
 * <LI>Object Identifier Helper functions provide assistance in working
 * with the object identifier ASN.1 type. </LI>
 * <LI>Linked List and Stack Utility functions are used to maintain
 * linked lists and stacks used within the ASN.1 run-time library
 * functions.</LI>
 * <LI>REAL Helper functions - REAL helper functions provide assistance
 * in working with the REAL ASN.1 type. Two functions are provided to
 * obtain the plus and minus infinity special values.  </LI>
 * <LI>Formatted Printing functions allow raw ASN.1 data fields to be
 * formatted and printed to standard output and other output devices.</LI>
 * <LI>Binary Coded Decimal (BCD) helper functions provide assistance in
 * working with BCD numbers.</LI>
 * <LI>Character String Conversion functions convert between standard
 * null-terminated C strings and different ASN.1 string types.</LI>
 * <LI>Big Integer Helper functions are arbitrary-precision integer
 * manipulating functions used to maintain big integers used within the
 * ASN.1 run-time functions. </LI>
 * <LI>Comparison functions allow comparison of the values of primitive
 * ASN.1 types.  They make it possible to compare complex structures and
 * determine what elements within those structures are different.</LI>
 * <LI>Comparison to Standard Output functions do the same actions as the
 * other comparison functions, but print the comparison results to standard
 * output instead of to the buffer. </LI>
 * <LI>Copy functions - This group of functions allows copying values of
 * primitive ASN.1 types. </LI>
 * <LI>Print Values to Standard Output functions print the output in a
 * "name=value" format, where the value format is obtained by calling
 *  one of the ToString functions with the given value.</LI>
 * </UL>
 */
/**
 * @file asn1type.h
 * Common ASN.1 runtime constants, data structure definitions, and run-time
 * functions to support the BER/DER/PER/XER as defined in the ITU-T standards.
 */
#ifndef _ASN1TYPE_H_
#define _ASN1TYPE_H_

#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdlib.h> /*so that borland compilers don't choke on extern linkage*/
#ifndef _WIN32_WCE
#include <time.h>
#endif

#ifdef _NUCLEUS
#ifndef NUCLEUS
#include <nucleus.h>
#endif /* NUCLEUS */
#else

#if !defined(_MSC_VER) && !defined(__GNUC__) && !defined(_16BIT) && !defined(__SYMBIAN32__)
#include <wchar.h>
#endif

#endif /* _NUCLEUS */

#include "rtxsrc/rtxExternDefs.h"
#include "rtxsrc/rtxSList.h"
#include "rtxsrc/rtxStack.h"
#include "rtxsrc/rtxUTF8.h"


/**
 * @defgroup cruntime C Runtime Common Functions
 * The <B>C run-time common library</B> contains common C functions used by
 * the low-level encode/decode functions. These functions are identified
 * by their <I>rt</I> and  <I>rtx</I> prefixes.
 *
 * The categories of functions provided are as follows:<UL>
 * <LI>Context management functions handle the allocation,
 * initialization, and destruction of context variables
 * (variables of type OSCTXT) that handle the working data used
 * during the encoding or decoding of a message.</LI>
 * <LI>Memory allocation macros and functions provide an optimized memory
 * management interface.</LI>
 * <LI>Doubly linked list (DList) functions are used to manipulate linked
 * list structures that are used to model repeating XSD types and
 * elements.</LI>
 * <LI>UTF-8 and Unicode character string functions provide support
 * for conversions to and from these formats in C or C++.</LI>
 * <LI>Date/time conversion functions provide utilities for converting
 * system and structured numeric date/time values to XML schema standard
 * string format.</LI>
 * <LI>Pattern matching function compare strings against patterns specified
 * using regular expressions (regexp's).</LI>
 * <LI>Diagnostic trace functions allow the output of trace messages
 * to standard output.</LI>
 * <LI>Error formatting and print functions allow information about
 * encode/decode errors to be added to a context block structure
 * and printed out.</LI>
 * <LI>Memory buffer management functions handle the allocation,
 * expansion, and de-allocation of dynamic memory buffers used by some
 * encode/decode functions.</LI>
 * <LI>Formatted print functions allow binary data to be
 * formatted and printed to standard output and other output devices.</LI>
 * <LI>Big Integer helper functions are arbitrary-precision integer
 * manipulating functions used to maintain big integers.</LI>
 * </UL>
  * @{
 */
#include "rtsrc/asn1tag.h"
#include "rtsrc/asn1ErrCodes.h"
#include "rtsrc/rtExternDefs.h"

/*
 * Flags used to determine or change the behavior of encoding and
 * decoding.
 */
/** seek match until found or end-of-buf */
#define XM_SEEK         0x01
/** advance pointer to contents on match */
#define XM_ADVANCE      0x02
/** alloc dyn mem for decoded variable   */
#define XM_DYNAMIC      0x04
/** skip to next field after parsing tag */
#define XM_SKIP         0x08
/** tag test is for optional element     */
#define XM_OPTIONAL     0x10

/* Sizing Constants */

/** maximum nesting depth for messages   */
#define ASN_K_MAXDEPTH  32
/** maximum enum values in an enum type  */
#define ASN_K_MAXENUM   100
/** maximum error parameters             */
#define ASN_K_MAXERRP   5
/** maximum levels on error ctxt stack   */
#define ASN_K_MAXERRSTK 8

#ifndef ASN_K_ENCBUFSIZ
/** dynamic encode buffer extent size    */
#define ASN_K_ENCBUFSIZ 16*1024
#endif

#ifndef ASN_K_MEMBUFSEG
/** memory buffer extent size            */
#define ASN_K_MEMBUFSEG 1024
#endif

/** number of spaces for indent  */
#define OSRTINDENTSPACES        3

/* Constants for encoding/decoding real values */

/** The ASN.1 Real value Plus Infinity. */
#define ASN1_K_PLUS_INFINITY    0x40

/** The ASN.1 Real value Minus Infinity. */
#define ASN1_K_MINUS_INFINITY   0x41

/** The ASN.1 Real value Not-A-Number. */
#define ASN1_K_NOT_A_NUMBER     0x42

/** The ASN.1 Real value Minus Zero. */
#define ASN1_K_MINUS_ZERO       0x43

#define REAL_BINARY             0x80
#define REAL_SIGN               0x40
#define REAL_EXPLEN_MASK        0x03
#define REAL_EXPLEN_1           0x00
#define REAL_EXPLEN_2           0x01
#define REAL_EXPLEN_3           0x02
#define REAL_EXPLEN_LONG        0x03
#define REAL_FACTOR_MASK        0x0c
#define REAL_BASE_MASK          0x30
#define REAL_BASE_2             0x00
#define REAL_BASE_8             0x10
#define REAL_BASE_16            0x20
#define REAL_ISO6093_MASK       0x3F

#ifndef __SYMBIAN32__
#include <float.h>
#define ASN1REALMAX    (OSREAL)DBL_MAX
#define ASN1REALMIN    (OSREAL)-DBL_MAX
#endif

typedef void*           ASN1ANY;

/**
 * An enumerated list of the various string types:  hexadecimal, binary, and
 * character strings.
 */
typedef enum { ASN1HEX, ASN1BIN, ASN1CHR } ASN1StrType;

/**
 * An enumerated list of ASN.1 actions:  encode or decode.
 */
typedef enum { ASN1ENCODE, ASN1DECODE } ASN1ActionType;

/*
 * OBJECT IDENTIFIER
 */
/**
 * @defgroup objidhelpers Object Identifier Helper Functions
 * @{
 *
 * Object identifier helper functions provide assistance in working with the
 * object identifier ASN.1 type.
 */

#define ASN_K_MAXSUBIDS 128     /* maximum sub-id's in an object ID     */

/**
 * This structure describes an object identifier with 32-bit arcs.
 */
typedef struct {        /* object identifier */
   /** The number of sub-identifiers in the OID. */
   OSUINT32     numids;
   /** An array of sub-identifiers. */
   OSUINT32     subid[ASN_K_MAXSUBIDS];
} ASN1OBJID;

/**
 * This structure describes an object identifier with 32-bit arcs.
 * It uses a dynamic array for the subidentifiers.
 */
typedef struct {        /* object identifier */
   /** Flag indicating memory was allocated using rtxMemAlloc for the
       array of subidentifiers. */
   OSBOOL       memAllocated;

   /** The number of sub-identifiers in the OID. */
   OSUINT16     numids;

   /** A dynamic array of sub-identifiers. */
   OSUINT32*    pSubIds;
} ASN1DynOBJID;

/**
 * This structure describes an object identifier with 64-bit arcs.
 */
typedef struct {        /* object identifier with 64-bit arcs */
   /** The number of sub-identifiers in the OID. */
   OSUINT32     numids;
   /** An array of sub-identifiers. */
   OSUINT64     subid[ASN_K_MAXSUBIDS];
} ASN1OID64;

/** @cond false */
#ifdef __cplusplus
#define EXTERN_C extern "C"
extern "C" {
#else
#define EXTERN_C extern
#endif
/** @endcond */

/**
 * This function populates an object identifier variable with data. It copies
 * data from a source variable to a target variable. Typically, the source
 * variable is a compiler-generated object identifier constant that resulted
 * from a object identifier value specification within an ASN.1 specification.
 *
 * @param ptarget      A pointer to a target object identifier variable to
 *                       receive object * identifier data. Typically, this is a
 *                       variable within a compiler-generated C structure.
 * @param psource      A pointer to a source object identifier variable to copy
 *                       to a target. Typically, this is a compiler-generated
 *                       variable corresponding to an ASN.1 value specification
 *                       in the ASN.1 source file.
 */
EXTERNRT void  rtSetOID (ASN1OBJID* ptarget, ASN1OBJID* psource);

/**
 * This function appends one object identifier to another one. It copies the
 * data from a source variable to the end of a target variable. Typically, the
 * source variable is a compiler-generated object identifier constant that
 * resulted from an object identifier value specification within an ASN.1
 * specification.
 *
 * @param ptarget      A pointer to a target object identifier variable to
 *                       receive object identifier data. Typically, this is a
 *                       variable within a compiler-generated C structure.
 * @param psource      A pointer to a source object identifier variable to copy
 *                       to a target. Typically, this is a compiler-generated
 *                       variable corresponding to an ASN.1 value specification
 *                       in the ASN.1 source file.
 */
EXTERNRT void  rtAddOID (ASN1OBJID* ptarget, ASN1OBJID* psource);

/**
 * This function compares two OID values for equality.
 *
 * @param pOID1         Pointer to first OID value to compare.
 * @param pOID2         Pointer to second OID value to compare.
 * @return              True if OID's are equal.
 */
EXTERNRT OSBOOL rtOIDsEqual (const ASN1OBJID* pOID1, const ASN1OBJID* pOID2);

/**
 * This function parses the ObjIdComponentsList of an ObjectIdentifierValue.
 * Components that use only a name can be converted if that form was used
 * as permitted by X.680 & X.660.  Leading and trailing whitespace is ignored.
 *
 * @param str           OID string containing data to be parsed. It does not
 *                      have to be null-terminated.
 * @param strlen        Length of the string.
 * @param pvalue        Pointer to OID value to receive parsed OID.
 * @return              Status of operation: 0 = success, negative value
 *                        is failure.
 */
EXTERNRT int rtOIDParseCompList(const char* str, OSSIZE strlen,
                        ASN1OBJID* pvalue);

/**
 * This function parses an OID that is an XMLObjectIdentifierValue.
 * Components that use only a name can be converted if that form was used
 * as permitted by X.680 & X.660.  Leading and trailing whitespace is ignored.
 *
 * @param oidstr        OID string containing data to be parsed. It does not
 *                      have to be null-terminated.
 * @param oidstrlen     Length of the string.
 * @param pvalue        Pointer to OID value to receive parsed OID.
 * @return              Status of operation: 0 = success, negative value
 *                        is failure.
 */
EXTERNRT int rtOIDParseString
(const char* oidstr, OSSIZE oidstrlen, ASN1OBJID* pvalue);



/**
 * This function parses the RelativeOIDComponentsList of a RelativeOIDValue.
 * Name-only components are not supported.  Leading and trailing whitespace is
 * ignored.
 *
 * @param str           String containing data to be parsed. It does not
 *                      have to be null-terminated.
 * @param strlen        Length of the string.
 * @param pvalue        Pointer to OID value to receive parsed OID.
 * @return              Status of operation: 0 = success, negative value
 *                        is failure.
 */
EXTERNRT int rtRelOIDParseCompList(const char* str, OSSIZE strlen,
   ASN1OBJID* pvalue);

/**
 * This function parses an OID that is an XMLRelativeOIDValue.
 * Name-only components are not supported.  Leading and trailing whitespace is
 * ignored.
 *
 * @param oidstr        OID string containing data to be parsed. It does not
 *                      have to be null-terminated.
 * @param oidstrlen     Length of the string.
 * @param pvalue        Pointer to OID value to receive parsed OID.
 * @return              Status of operation: 0 = success, negative value
 *                        is failure.
 */
EXTERNRT int rtRelOIDParseString
(const char* oidstr, OSSIZE oidstrlen, ASN1OBJID* pvalue);

/**
 * This function parses an OID dotted number string (n.n.n) which is the
 * from of OID XML content.  Data must be in the form of numbers and dots
 * only (i.e. OID components in other forms such as names or named
 * number will cause a parse failure).  Embedded whitespace will be
 * ignored.
 *
 * @param oidstr        OID string containing data to be parsed.
 * @param oidstrlen     Length of the string.
 * @param pvalue        Pointer to OID value to receive parsed OID.
 * @return              Status of operation: 0 = success, negative value
 *                        is failure.
 */
EXTERNRT int rtOIDParseDottedNumberString
(const char* oidstr, OSSIZE oidstrlen, ASN1OBJID* pvalue);

/**
 * This function determines if an OID value is valid according to ASN.1
 * rules.  In particular it checks a) if number of subidentifiers is
 * greater than or equal to 2, b) if the first subidentifier value is
 * less than or equal to 2, and c) if the first subidentifier is 2
 * that the second subidentifier is less than 40.
 *
 * @param pvalue        Pointer to OID value to validate.
 * @return              True if OID value is valid.
 */
EXTERNRT OSBOOL rtOIDIsValid (const ASN1OBJID* pvalue);

/**
 * This function decodes OID subidentifiers into the given array.
 * It will work for all binary encoding rules (BER/DER/CER, PER/UPER,
 * and OER).
 *
 * @param pvalue        Pointer to OID value to validate.
 * @return              True if OID value is valid.
 */
EXTERNRT int rtDecOIDSubIds
(OSCTXT* pctxt, OSUINT32 subidArray[], OSSIZE subidArraySize, int enclen);

/**
 * @}
 */

/*
 * OCTET STRING
 */
/**
 * A generic octet string structure.  This contains the number of octest and
 * a data array with a size of one.
 */
typedef struct {
   OSUINT32     numocts;
   OSOCTET      data[1];
} ASN1OctStr;

/** We define ASN1DynOctStr to be the common generic OSDynOctStr type. */
#define ASN1DynOctStr OSDynOctStr
#define ASN1DynOctStr64 OSDynOctStr64

/*
 * BIT STRING
 */
/** generic bit string structure (dynamic) */
typedef struct {
   OSUINT32     numbits;
   const OSOCTET* data;
} ASN1DynBitStr;

/** Dynamic bit string structure that can hold bit strings with lengths up
    to 64 bits in size */
typedef struct {
   OSSIZE numbits;
   const OSOCTET* data;
} ASN1DynBitStr64;

/** fixed-size bit string that can hold up to 32 bits */
typedef struct {
   OSUINT32     numbits;
   OSOCTET      data[4];
} ASN1BitStr32;

#include "rtxsrc/rtxBitString.h"

/**
 * This macro sets the given bit in the given static bit string.
 *
 * @param   bitStr      The bit string to manipulate.
 * @param   bitIndex    The index to set.
 */
#define OSSETBIT(bitStr,bitIndex) \
rtxSetBit (bitStr.data, bitStr.numbits, bitIndex)

/**
 * This macro sets the given bit in the given dynamic bit string.
 *
 * @param   pBitStr     A pointer-to-bit string to manipulate.
 * @param   bitIndex    The index to set.
 */
#define OSSETBITP(pBitStr,bitIndex) \
rtxSetBit ((pBitStr)->data, (pBitStr)->numbits, bitIndex)

/**
 * This macro clears the given bit in the given static bit string.
 *
 * @param   bitStr      The bit string to manipulate.
 * @param   bitIndex    The index to clear.
 */
#define OSCLEARBIT(bitStr,bitIndex) \
rtxClearBit (bitStr.data, bitStr.numbits, bitIndex)

/**
 * This macro clears the given bit in the given dynamic bit string.
 *
 * @param   pBitStr     The pointer-to-bit string to manipulate.
 * @param   bitIndex    The index to clear.
 */
#define OSCLEARBITP(pBitStr,bitIndex) \
rtxClearBit ((pBitStr)->data, (pBitStr)->numbits, bitIndex)

/**
 * This macro tests the given bit in the given static bit string.
 *
 * @param   bitStr      The bit string to manipulate.
 * @param   bitIndex    The index to test.
 *
 * @return  TRUE if the bit is on; FALSE if the bit is off.
 */
#define OSTESTBIT(bitStr,bitIndex) \
rtxTestBit (bitStr.data, bitStr.numbits, bitIndex)

/**
 * This macro tests the given bit in the given dynamic bit string.
 *
 * @param   pBitStr     The pointer-to-bit string to manipulate.
 * @param   bitIndex    The index to set.
 */
#define OSTESTBITP(pBitStr,bitIndex) \
rtxTestBit ((pBitStr)->data, (pBitStr)->numbits, bitIndex)

/**
 * @defgroup timeutilf Time Helper Functions
 * @{
 * Utility functions for working with time strings in different
 * formats.  rtMake* functions create time strings, rtParse* functions
 * parse time strings.  For ASN.1 GeneralizedTime and UTCTime, the C type is
 * used is the OSNumDateTime structure defined in osSysTypes.h.  For ASN.1
 * TIME, the ASN1Time struct is used.
 */

 /**
 * ASN1TIMEFIELD_ macros are bit flags used to indicate what fields are
 * present in an ASN.1 TIME abstract value.
 *
 * For year, the value of the year can be used to determine the setting
 * for the Year property (i.e. Basic, Proleptic, Negative, L5, L6, etc..
 *
 * For intervals, ASN1TIMEFIELD_RECUR_INF is used to signal infinite
 * recurrences while the number of recurrences (0 or more) can be used to
 * distinguish between a non-recurring interval and an R1, R2, etc. recurring
 * interval.
 *
 * Where fractions may appear, the number of fraction digits can be used to
 * distinguish between no fraction (0 fraction digits) and F1, F2, etc..
 */
#define ASN1TIMEFIELD_CENTURY 0x01
#define ASN1TIMEFIELD_YEAR 0x02
#define ASN1TIMEFIELD_MONTH 0x04
#define ASN1TIMEFIELD_DAY 0x08
#define ASN1TIMEFIELD_WEEK 0x10
#define ASN1TIMEFIELD_HOUR 0x20
#define ASN1TIMEFIELD_MIN 0x40
#define ASN1TIMEFIELD_SEC 0x80
#define ASN1TIMEFIELD_Z 0x0100
#define ASN1TIMEFIELD_DIFF 0x0200
#define ASN1TIMEFIELD_INTERVAL_SE 0x0400
#define ASN1TIMEFIELD_DURATION 0x0800
#define ASN1TIMEFIELD_INTERVAL_SD 0x1000
#define ASN1TIMEFIELD_INTERVAL_DE 0x2000
#define ASN1TIMEFIELD_RECUR_INF 0x4000

/* ASN1TIMEFIELD_POINT includes all the settings that would be set for a
   point in time (date, time-of-day or both) except for timezone diff. */
#define ASN1TIMEFIELD_POINT (ASN1TIMEFIELD_CENTURY | ASN1TIMEFIELD_YEAR | \
      ASN1TIMEFIELD_MONTH | ASN1TIMEFIELD_DAY | ASN1TIMEFIELD_WEEK | \
      ASN1TIMEFIELD_HOUR | ASN1TIMEFIELD_MIN | ASN1TIMEFIELD_SEC | \
      ASN1TIMEFIELD_Z)

typedef struct ASN1DateTime
{
   OSINT32 century;
   OSINT32 year;
   OSUINT8 month;
   OSUINT8 week;
   OSUINT8 day;
   OSUINT8 hour;
   OSUINT8 min;
   OSUINT8 sec;
   OSUINT8 fracDigits;
   OSUINT32 fracValue;
   OSINT8 timezoneDifffHours;
   OSUINT8 timezoneDiffMins;
} ASN1DateTime;

/**
 * Represents a duration of time.
 * Durations are supposed to have weeks only, or a combination of years, months,
 * ..., seconds.
 * Durations of different precision are considered distinct values, even if
 * their components are equal.
 * "PT6H" and "PT6H0S" are considered distinct values and have a distinct
 * ASN1Duration representations.   "P0DT6H" and "PT6H" are considered equal and
 * have the same ASN1Duration represenatation.
 */
typedef struct ASN1Duration
{
   OSUINT32 years;
   OSUINT32 months;
   OSUINT32 weeks;
   OSUINT32 days;
   OSUINT32 hours;
   OSUINT32 mins;
   OSUINT32 secs;
   OSUINT8 fracDigits;
   OSUINT32 fracValue;
   OSUINT8 precision;   /* 1 = years, 2 = months etc., in the order in which
                     the componentas appear above.  This indicates which
                     is the last component present. If fracDigits != 0,
                     this tells you what component the fraction belongs to.
                     Fields which are more precise than indicated by precision
                     MUST be set to zero. */
} ASN1Duration;

/**
* Represents an interval with a duration and possibly an end point.
*/
typedef struct ASN1IntervalDE
{
   ASN1Duration duration;
   ASN1DateTime end;
} ASN1IntervalDE;

/**
* Represents an interval with a start point and an end point.
*/
typedef struct ASN1IntervalSE
{
   ASN1DateTime start;
   ASN1DateTime end;
} ASN1IntervalSE;

/**
* Represents an interval with a start point and a duration.
*/
typedef struct ASN1IntervalSD
{
   ASN1DateTime start;
   ASN1Duration duration;
} ASN1IntervalSD;

/**
* Represents a recurring or non-recurring interval.
*/
typedef struct ASN1Interval
{
   union {
      ASN1Duration duration;
      ASN1IntervalDE de;
      ASN1IntervalSE se;
      ASN1IntervalSD sd;
   } spec;
   OSUINT32 recurrences;
} ASN1Interval;


/**
* Represents an ASN.1 TIME abstract value.
* The settings field indicates what fields are present.
*/
typedef struct ASN1Time
{
   /**
    * Combination of ASN1TIMEFIELD_* flags indicating what fields are present
    * in this TIME value.
    */
   OSUINT16 settings;

   union {
      /* dateTime is use for abstract values that are only a date, a
      time-of-day or a date and a time-of-day.*/
      ASN1DateTime dateTime;

      /* interval is used for abstract values that are an interval.
      If the interval has two points (start and end), they must use the same
      settings.
      The interval may be infinitely recurring (ASN1TIMEFIELD_RECUR_INF is
      set in settings), finitely recurring (recurrences != 0), or
      non-recurring (recurrences == 0).
      The bit fields ASN1TIMEFIELD_DURATION, ASN1TIMEFIELD_INTERVAL_SD,
      ASN1TIMEFIELD_INTERVAL_SE, and ASN1TIMEFIELD_INTERVAL_DE in settings are
      used to distinguish the four possible cases.
      */
      ASN1Interval interval;
   } val;
} ASN1Time;


/**
 * This function creates a time string in ASN.1 GeneralizedTime format as
 * specified in the X.680 ITU-T standard.
 *
 * @param pctxt        A pointer to a context structure. This provides a
 *                       storage area for the function to store all working
 *                       variables that must be maintained between function
 *                       calls.
 * @param dateTime     A pointer to a date/time structure that contains
 *                       components of the date and time.
 * @param outdata      A pointer to a pointer to a destination string. If
 *                       outdataSize is non-zero, it should be a pointer to a
 *                       pointer to an actual array. Otherwise, the memory will
 *                       be allocated and the pointer will be stored in the \c
 *                       outdata.
 * @param outdataSize  A size of outdata (in octets). If zero, the memory for
 *                       the \c outdata will be allocated. If not, the \c
 *                       outdata 's size should be big enough to receive the
 *                       generated time string. Otherwise, error code will be
 *                       returned.
 * @return             Completion status of operation:
 *                       - 0 (ASN_OK) = success,
 *                       - negative return value is error.
 */
EXTERNRT int rtMakeGeneralizedTime (OSCTXT* pctxt,
                                    const OSNumDateTime* dateTime,
                                    char** outdata,
                                    size_t outdataSize);

/**
* This function creates a time string in ASN.1 TIME format as specified in
* the X.680 ITU-T standard.  The string is a canonical value representation
* as required by CER/DER (note this is not exactly the same as the encoded
* representation; see X.690:2015 8.26 and 11.9).
*
* @param pctxt        A pointer to a context structure. This provides a
*                       storage area for the function to store all working
*                       variables that must be maintained between function
*                       calls.
* @param timeVal      A pointer to an ASN1Time structure that contains value.
* @param outdata      A pointer to a pointer to a destination string. If
*                       outdataSize is non-zero, it should be a pointer to a
*                       pointer to an actual array. Otherwise, the memory will
*                       be allocated and the pointer will be stored in the \c
*                       outdata.
* @param outdataSize  A size of outdata (in octets). If zero, the memory for
*                       the \c outdata will be allocated. If not, the \c
*                       outdata 's size should be big enough to receive the
*                       generated time string. Otherwise, an error code will be
*                       returned.
* @return             Completion status of operation:
*                       - 0 (ASN_OK) = success,
*                       - negative return value is error.
*/
EXTERNRT int rtMakeTime(OSCTXT* pctxt,
   const ASN1Time* timeVal,
   char** outdata,
   size_t outdataSize);


/**
 * This function creates a time string in ASN.1 UTCTime format as specified in
 * the X.680 ITU-T standard.
 *
 * @param pctxt        A pointer to a context structure. This provides a
 *                       storage area for the function to store all working
 *                       variables that must be maintained between function
 *                       calls.
 * @param dateTime     A pointer to a date/time structure that contains
 *                       components of the date and time.
 * @param outdata      A pointer to a pointer to a destination string. If
 *                       outdataSize is non-zero, it should be a pointer to a
 *                       pointer to an actual array. Otherwise, the memory will
 *                       be allocated and the pointer will be stored in the \c
 *                       outdata.
 * @param outdataSize  A size of outdata (in octets). If zero, the memory for
 *                       the \c outdata will be allocated. If not, the \c
 *                       outdata 's size should be big enough to receive the
 *                       generated time string. Otherwise, error code will be
 *                       returned.
 * @return             Completion status of operation:
 *                       - 0 (ASN_OK) = success,
 *                       - negative return value is error.
 */
EXTERNRT int rtMakeUTCTime (OSCTXT* pctxt,
                            const OSNumDateTime* dateTime,
                            char** outdata,
                            size_t outdataSize);
/**
 * This function parses a time string that is represented in ASN.1
 * GeneralizedTime format as specified in the X.680 ITU-T standard. It stores
 * the parsed result in a numeric date/time C structure.
 *
 * If the ASN1CANON or ASN1DER flag is set, and the given value is not
 * a canonical encoding representation, this function will log a non-fatal
 * ASN_E_NOTCANON error (this does not cause a negative return).
 *
 * @param pctxt        A pointer to a context structure. This provides a
 *                       storage area for the function to store all working
 *                       variables that must be maintained between function
 *                       calls.
 * @param value        A pointer to the time string to be parsed.
 * @param dateTime     A pointer to the destination structure.
 * @return             Completion status of operation:
 *                       - 0 (ASN_OK) = success,
 *                       - negative return value is error.
 */
EXTERNRT int rtParseGeneralizedTime (OSCTXT *pctxt,
                                     const char* value,
                                     OSNumDateTime* dateTime);

/**
* This function is the same as rtParseGeneralizedTime except as follows.  When
* the ASN1CANON or ASN1DER flag is set, if there are canonical violations and
* no actual errors, this function will return ASN_E_NOTCANON and, if logCanon
* is true, it will also log non-fatal ASN_E_NOTCANON errors.
*
* @param pctxt        A pointer to a context structure. This provides a
*                       storage area for the function to store all working
*                       variables that must be maintained between function
*                       calls.
* @param value        A pointer to the time string to be parsed.
* @param dateTime     A pointer to the destination structure.
* @param logCanon     Controls whether to log canonical violations or not.
* @return             Completion status of operation:
*                       - 0 (ASN_OK) = success,
*                       - negative return value is error.
*/
EXTERNRT int rtParseGeneralizedTime2(OSCTXT *pctxt,
   const char* value,
   OSNumDateTime* dateTime,
   OSBOOL logCanon);

/**
 * This function parses a time string that is represented in ASN.1 UTCTime
 * format as specified in the X.680 ITU-T standard. It stores the parsed
 * result in a numeric date/time C structure.
 *
 * If the ASN1CANON or ASN1DER flag is set, and the given value is not
 * a canonical encoding representation, this function will log a non-fatal
 * ASN_E_NOTCANON error (this does not cause a negative return).
 *
 * @param pctxt        A pointer to a context structure. This provides a
 *                       storage area for the function to store all working
 *                       variables that must be maintained between function
 *                       calls.
 * @param value        A pointer to the time string to be parsed.
 * @param dateTime     A pointer to the destination date/time structure.
 * @return             Completion status of operation:
 *                       - 0 (ASN_OK) = success,
 *                       - negative return value is error.
 */
EXTERNRT int rtParseUTCTime (OSCTXT *pctxt,
                             const char* value,
                             OSNumDateTime* dateTime);

/**
* This function is the same as rtParseUTCTime except as follows.  When the
* ASN1CANON or ASN1DER flag is set, if there are canonical violations and
* no actual errors, this function will return ASN_E_NOTCANON and, if logCanon
* is true, it will also log non-fatal ASN_E_NOTCANON errors.
*
* @param pctxt        A pointer to a context structure. This provides a
*                       storage area for the function to store all working
*                       variables that must be maintained between function
*                       calls.
* @param value        A pointer to the time string to be parsed.
* @param dateTime     A pointer to the destination date/time structure.
* @param logCanon     Controls whether to log canonical violations or not.
* @return             Completion status of operation:
*                       - 0 (ASN_OK) = success,
*                       - negative return value is error.
*/
EXTERNRT int rtParseUTCTime2(OSCTXT *pctxt,
   const char* value,
   OSNumDateTime* dateTime, OSBOOL logCanon);


/**
* Parse an ASN.1 TIME value reprsentation.
*
* If the ASN1CANON or ASN1DER context flag is set, this will also check that
* the CER/DER canonical value representation is used.  If logCanon is TRUE,
* a violation will result in logging a non-fatal ASN1_E_NOTCANON error.
* Otherwise, a violation will only result in returning ASN1_E_NOTCANON.
*
* @param pctxt       A pointer to a context structure. This is used for the
*                       canonical flags and for logging errors.
* @param value       The value to parse.
* @param timeVal     Receives the result of the parsing.
* @param logCanon    As described above.
*/
EXTERNRT int rtParseTime(OSCTXT* pctxt, const char* value, ASN1Time* timeVal,
                           OSBOOL logCanon);


/**
 * This function normalizes the time zone for the given datetime structure.
 *
 * @param   pvalue   A pointer-to an OSNumDateTime structure.
 */
EXTERNRT void normalizeTimeZone(OSNumDateTime* pvalue);


/**
 * @}
 */

/*
 * SEQUENCE OF
 */
/**
 * A generic SEQUENCE OF structure that contains a number of elements
 * and a pointer-to-void that contains the contents.
 */
typedef struct {                /* generic sequence of structure        */
   /** The number of elements. */
   OSUINT32     n;

   /** The pointer-to-void that contains the elements. */
   void*        elem;
} ASN1SeqOf;

/**
 * A generic SEQUENCE OF dynamic OCTET STRING.
 *
 * This structure is used to hold a sequence of octet strings.  The elements
 * are pointers-to-OSDynOctStr. */
typedef struct {                /* sequence of OCTET STRING             */
   /** The number of elements. */
   OSUINT32     n;

   /** A pointer-to-OSDynOctStr that contains the elements. */
   OSDynOctStr* elem;
} ASN1SeqOfOctStr;

/**
 * A generic open type structure.  This structure contains a number of
 * octets and the data that compose the open type information.
 */
typedef struct {                /* generic open type data structure     */
   OSSIZE       numocts;
   const OSOCTET* data;
} ASN1OpenType;

/**
 * A generic table constraint value holder.  This structure contains the
 * encoded open type value, a pointer-to-void that holds the decoded content,
 * and the integer-valued index of the table constraint value.
 */
typedef struct {                /* generic table constraint value holder */
   ASN1OpenType encoded;
   void*        decoded;
   OSINT32      index;          /* table index */
} Asn1Object;

typedef Asn1Object ASN1Object;

/**
 * @struct OSXSDAny
 * Structure to hold xsd:any data in binary and XML text form.
 */
typedef enum { OSXSDAny_binary, OSXSDAny_xmlText } OSXSDAnyAlt;
typedef struct OSXSDAny {
   OSXSDAnyAlt t;
   union {
      ASN1OpenType* binary;
      const OSUTF8CHAR* xmlText;
   } u;
} OSXSDAny;

/* ASN.1 useful type definitions */

/**
 * A structure that holds a 16-bit ASN.1 character string.  This contains the
 * number of characters and a pointer to 16-bit characters that hold the
 * string data.
 */
typedef struct {
   OSSIZE       nchars;
   OSUNICHAR*   data;
} Asn116BitCharString;

typedef OSUNICHAR ASN116BITCHAR;

/**
 * A structure that holds a 32-bit ASN.1 character string.  This contains the
 * number of characters and a pointer to 32-bit characters that hold the
 * string data.
 */
typedef struct {
   OSSIZE       nchars;
   OS32BITCHAR* data;
} Asn132BitCharString;

typedef const char*   ASN1GeneralizedTime;
typedef const char*   ASN1GeneralString;
typedef const char*   ASN1GraphicString;
typedef const char*   ASN1IA5String;
typedef const char*   ASN1ISO646String;
typedef const char*   ASN1NumericString;
typedef const char*   ASN1ObjectDescriptor;
typedef const char*   ASN1PrintableString;
typedef const char*   ASN1TeletexString;
typedef const char*   ASN1T61String;
typedef const char*   ASN1UTCTime;
typedef const char*   ASN1VideotexString;
typedef const char*   ASN1VisibleString;

typedef const OSUTF8CHAR*   ASN1UTF8String;
typedef Asn116BitCharString ASN1BMPString;
typedef Asn132BitCharString ASN1UniversalString;

/* ASN.1 constrained string structures */

/**
 * A generic character array.  The array data holds up to 255 characters, with
 * the actual number of characters being provided by the nchars member.
 */
typedef struct {
   int          nchars;
   char         data[255];
} Asn1CharArray;

/**
 * Describes an ASN.1 character set, use primarily for PER encodings whose
 * alphabet constraints can be used to encode more compact representations
 * of their strings.
 */
typedef struct {
   /**
    * The array of characters that comprise this particular character set; at
    * most this takes up 255 characters.
    */
   Asn1CharArray charSet;

   /**
    * A character string describing the canonical set of characters.
    */
   const char*  canonicalSet;

   /**
    * The size of the canonical character set.
    */
   int          canonicalSetSize;

   /**
    * The number of bits taken up by the canonical set.
    */
   unsigned     canonicalSetBits;

   /**
    * The number of bits required in unaligned applications for this character
    * set.
    */
   unsigned     charSetUnalignedBits;

   /**
    * The number of bits reuqired in aligned applications for this character
    * set.
    */
   unsigned     charSetAlignedBits;
} Asn1CharSet;

/**
 * Describes an ASN.1 character set whose characters are 16-bits wide instead
 * of 8-bits wide.
 */
typedef struct {
   /** A character string describing the whole character set. */
   Asn116BitCharString charSet;

   /** 16-bit integers describing the first and last characters in the set. */
   OSUINT16     firstChar, lastChar;

   /** The number of bits required by this set in unaligned applications. */
   unsigned     unalignedBits;

   /** The number of bits required by this set in aligned applications. */
   unsigned     alignedBits;
} Asn116BitCharSet;

/**
 * Describes an ASN.1 character set whose characters are 32-bits wide instead
 * of 8-bits wide.
 */
typedef struct {
   /** A character string describing the whole character set. */
   Asn132BitCharString charSet;

   /** 32-bit integers describing the first and last characters in the set. */
   OSUINT32     firstChar, lastChar;

   /** The number of bits required by this set in unaligned applications. */
   unsigned     unalignedBits;

   /** The number of bits required by this set in aligned applications. */
   unsigned     alignedBits;
} Asn132BitCharSet;

/**
 * @defgroup charstrcon Character String Conversion Functions
 * @{
 *
 * Common utility functions are provided to convert between standard
 * null-terminated C strings and different ASN.1 string types.
 */

/* Helper functions. */

/**
 * This function ensures that a given string does not contain invalid
 * characters.
 *
 * @param   tag         The ASN.1 Tag that identifies the string.
 * @param   pdata       A pointer to the character string to be examined.
 *
 * @return              This function returns 0 if the string validates
 *                      or the tag is not associated with a string; it
 *                      otherwise returns the integer value of the character
 *                      that invalidates the string.
 */
EXTERNRT int rtValidateStr (ASN1TAG tag, const char *pdata);

/**
 * This function ensures that a given string does not contain invalid
 * characters.
 *
 * @param   tag         The ASN.1 Tag that identifies the string.
 * @param   pdata       A pointer to the character string to be examined.
 * @param   nchars      The number of characters in pdata.
 *
 * @return              This function returns 0 if the string validates
 *                      or the tag is not associated with a string; it
 *                      otherwise returns the integer value of the character
 *                      that invalidates the string.
 */
EXTERNRT int rtValidateChars (ASN1TAG tag, const char* pdata, size_t nchars);

/* 16-bit character functions */

/**
 * This function converts a BMP string into a null-terminated C string. Any
 * characters that are not 8-bit characters are discarded.
 *
 * @param pBMPString   A pointer to a BMP string structure to be converted.
 * @param cstring      A pointer to a buffer to receive the converted string.
 * @param cstrsize     The size of the buffer to receive the converted string.
 * @return             A pointer to the returned string structure. This is the
 *                       cstring argument parameter value.
 */
EXTERNRT const char* rtBMPToCString (ASN1BMPString* pBMPString,
                                     char* cstring, OSSIZE cstrsize);

/**
 * This function converts a BMP string into a null-terminated C string. Any
 * characters that are not 8-bit characters are discarded. This function
 * allocates dynamic memory to hold the converted string using the standard C
 * run-time malloc function. The user is responsible for freeing this memory.
 *
 * @param pBMPString   A pointer to a BMP string structure to be converted.
 * @return             A pointer to the returned string structure. This is the
 *                       cstring argument parameter value.
 */
EXTERNRT const char* rtBMPToNewCString (ASN1BMPString* pBMPString);

/**
 * This function converts a BMP string into a null-terminated C string. Any
 * characters that are not 8-bit characters are discarded. In contrast to
 * rtBMPToNewCString, this function allocates dynamic memory to hold the
 * converted string using the rtMemAlloc function. The rtMemFreePtr should be
 * called to release the allocated memory or the rtmemFree function should be
 * called to release all memory allocated using the specified context block.
 *
 * @param pctxt        A pointer to a context structure.
 * @param pBMPString   A pointer to a BMP string structure to be converted.
 * @return             A pointer to the returned string structure. This is the
 *                       cstring argument parameter value.
 */
EXTERNRT const char* rtBMPToNewCStringEx
   (OSCTXT *pctxt, ASN1BMPString* pBMPString);

/**
 * This function converts a null-terminated C string into a 16-bit BMP string
 * structure.
 *
 * @param pctxt       A pointer to a context string.
 * @param cstring      A pointer to a null-terminated C string to be converted
 *                       into a BMP string.
 * @param pBMPString   A pointer to a BMP string structure to receive the
 *                       converted string.
 * @param pCharSet     A pointer to a character set structure describing the
 *                       character set currently associated with the BMP
 *                       character string type.
 * @return             A pointer to BMP string structure. This is the
 *                       pBMPString argument parameter value.
 */
EXTERNRT ASN1BMPString* rtCToBMPString (OSCTXT* pctxt,
                                         const char* cstring,
                                         ASN1BMPString* pBMPString,
                                         Asn116BitCharSet* pCharSet);

/**
 * This function tests whether the given character is in the given 16-bit
 * character set.
 *
 * @param   ch          A 16-bit character.
 * @param   pCharSet    A pointer-to Asn116BitCharSet that contains the set
 *                         of valid character.
 *
 * @return TRUE if the character is in the set, FALSE otherwise.
 */
EXTERNRT OSBOOL rtIsIn16BitCharSet (OSUNICHAR ch,
                                     Asn116BitCharSet* pCharSet);

/* 32-bit character (UCS-4) functions */


/**
 * This function converts a Universal 32-bit string into a null-terminated C
 * string. Any characters that are not 8-bit characters are discarded.
 *
 * @param pUCSString   A pointer to a Universal string structure to be
 *                       converted.
 * @param cstring      A pointer to a buffer to receive a converted string.
 * @param cstrsize     The size of the buffer to receive the converted string.
 * @return             The pointer to the returned string. This is the cstring
 *                       argument parameter value.
 */
EXTERNRT const char* rtUCSToCString (ASN1UniversalString* pUCSString,
                                     char* cstring, OSSIZE cstrsize);

/**
 * This function converts a Universal 32-bit string into a null-terminated C
 * string. Any characters that are not 8-bit characters are discarded. This
 * function allocates dynamic memory to hold the converted string using the
 * standard C run-time malloc function. The user is responsible for freeing
 * this memory.
 *
 * @param pUCSString   A pointer to a Universal 32-bit string structure to be
 *                       converted.
 * @return             A pointer to allocated null-terminated string. The user
 *                       is responsible for freeing this memory.
 */
EXTERNRT const char* rtUCSToNewCString (ASN1UniversalString* pUCSString);

/**
 * This function converts a Universal 32-bit string into a null-terminated C
 * string. Any characters that are not 8-bit characters are discarded. In
 * contrast to rtUSCToNewCString this function allocates dynamic memory to hold
 * the converted string using the rtMemAlloc function. The rtMemFreePtr should
 * be called to release the allocated memory or the rtMemFree function should
 * be called to release all memory allocated using the specified context block.
 *
 * @param pctxt        A pointer to a context block.
 * @param pUCSString   A pointer to a Universal 32-bit string structure to be
 *                       converted.
 * @return             A pointer to allocated null-terminated string. The user
 *                       is responsible for freeing this memory.
 */
EXTERNRT const char* rtUCSToNewCStringEx
(OSCTXT* pctxt, ASN1UniversalString* pUCSString);

/**
 * This function converts a null-terminated C string into a 32-bit UCS-4
 * (Universal Character Set, 4 byes) string structure.
 *
 * @param pctxt       A pointer to a context structure.
 * @param cstring      A pointer to a null-terminated C string to be converted
 *                       into a Universal string.
 * @param pUCSString   A pointer to a Universal string structure to receive the
 *                       converted string
 * @param pCharSet     A pointer to a character structure describing the
 *                       character set currently associated with the Universal
 *                       character string type.
 * @return             A pointer to a Universal string structure. This is the
 *                       pUCSString argument parameter value.
 */
EXTERNRT ASN1UniversalString* rtCToUCSString
(OSCTXT* pctxt, const char* cstring, ASN1UniversalString* pUCSString,
 Asn132BitCharSet* pCharSet);


/**
 * This function tests whether the given character is in the given 32-bit
 * character set.
 *
 * @param   ch          A 32-bit character.
 * @param   pCharSet    A pointer-to Asn132BitCharSet that contains the set
 *                         of valid character.
 *
 * @return TRUE if the character is in the set, FALSE otherwise.
 */
EXTERNRT OSBOOL rtIsIn32BitCharSet (OS32BITCHAR ch,
                                     Asn132BitCharSet* pCharSet);

/**
 * This function converts a 32-bits encoded string to a wide character string.
 *
 * @param pUCSString   A pointer to a Universal string structure.
 * @param wcstring     The pointer to the buffer to receive the converted
 *                       string.
 * @param wcstrsize    The number of wide characters (wchar_t) the outbuffer
 *                       can hold.
 * @return             A character count or error status. This will be negative
 *                       if the conversion fails. If the result is positive,
 *                       the number of characters was written to scstrsize.
 */
EXTERNRT wchar_t* rtUCSToWCSString (ASN1UniversalString* pUCSString,
                                    wchar_t* wcstring, OSUINT32 wcstrsize);


/**
 * This function converts a wide-character string to a Universal 32-bits
 * encoded string.
 *
 * @param pctxt       A pointer to a context structure.
 * @param wcstring     The pointer to the wide-character (Unicode) string to
 *                       convert
 * @param pUCSString   The pointer to the Universal String structure to receive
 *                       the converted string.
 * @param pCharSet     The pointer to the character set structure describing
 *                       the character set currently associated with the
 *                       Universal character string type.
 * @return             If the conversion of the WCS to the UTF-8 was
 *                       successful, the number of bytes in the converted
 *                       string is returned. If the encoding fails, a negative
 *                       status value is returned.
 */
EXTERNRT ASN1UniversalString* rtWCSToUCSString
(OSCTXT* pctxt, wchar_t* wcstring,
 ASN1UniversalString* pUCSString, Asn132BitCharSet* pCharSet);

/**
 * This function converts an ASN.1 Universal String type (32-bit
 * characters) to UTF-8.
 *
 * @param pctxt       A pointer to a context structure.
 * @param pUnivStr    Pointer to universal string to be converted.
 * @param outbuf      Output buffer to receive UTF-8 characters.
 * @param outbufsiz   Output buffer size in bytes.
 * @return            Zero if conversion was successful, a negative
 *                       status code if failed.
 */
EXTERNRT int rtUnivStrToUTF8
(OSCTXT* pctxt, const ASN1UniversalString* pUnivStr, OSOCTET* outbuf,
 size_t outbufsiz);

/**
 * This function converts the given null-terminated UTF-8 string to
 * a bit string value.  The string consists of a series of '1' and '0'
 * characters. This is the dynamic version in which memory is allocated for
 * the returned binary string variable.  Bits are stored from MSB to LSB order.
 *
 * @param pctxt        Pointer to context block structure.
 * @param utf8str      Null-terminated UTF-8 string to convert
 * @param pvalue       Pointer to a variable to receive the decoded boolean
 *                       value.
 * @return             Completion status of operation:
 *                       - 0 = success,
 *                       - negative return value is error.
 */
EXTERNRT int rtUTF8StrToASN1DynBitStr
   (OSCTXT* pctxt, const OSUTF8CHAR* utf8str, ASN1DynBitStr* pvalue);

/**
 * This function converts the given part of UTF-8 string to
 * a bit string value.  The string consists of a series of '1' and '0'
 * characters. This is the dynamic version in which memory is allocated for
 * the returned binary string variable.  Bits are stored from MSB to LSB order.
 *
 * @param pctxt        Pointer to context block structure.
 * @param utf8str      UTF-8 string to convert. Not necessary to be
 *                        null-terminated.
 * @param nbytes       Size in bytes of utf8Str.
 * @param pvalue       Pointer to a variable to receive the decoded boolean
 *                       value.
 * @return             Completion status of operation:
 *                       - 0 = success,
 *                       - negative return value is error.
 */
EXTERNRT int rtUTF8StrnToASN1DynBitStr
   (OSCTXT* pctxt, const OSUTF8CHAR* utf8str, size_t nbytes,
   ASN1DynBitStr* pvalue);

/**
 * @}
 */

/**
 * A structure used to define an ASN.1 big integer.  This structure is rarely,
 * if ever, used by client code, and will instead be used by generated code
 * to facilitate encoding and decoding integer values that cannot fit in
 * normal C/C++ integer types.
 */
typedef struct ASN1BigInt {
   /** The number of octets used in the magnitude. */
   size_t    numocts;

   /** The magnitude value. */
   OSOCTET*  mag;

   /** The sign: either -1, 0, or 1. */
   int       sign;

   /** The number of octets allocated for the magnitude. */
   size_t    allocated;

   /** A flag that tells whether the buffer is dynamically allocated. */
   OSBOOL    dynamic;
} ASN1BigInt;

/** The maximum size for the context control block mask. */
#define ASN1_K_CCBMaskSize      32

/** The number of bits that can be set per mask. */
#define ASN1_K_NumBitsPerMask   16

/** The maximum number of set elements that can be handled by the CCB. */
#define ASN1_K_MaxSetElements   (ASN1_K_CCBMaskSize*ASN1_K_NumBitsPerMask)

/**
 * The ASN.1 Context Control Block.
 *
 * This structure is used to help facilitate decoding in SEQUENCE and SET
 * structures.  It is rarely (if ever) used directly by client code, and
 * will instead by used in generated code.
 */
typedef struct {
   /** The contructor pointer. */
   OSOCTET*     ptr;

   /** The constructor length. */
   long         len;

   /** The sequence element index. */
   int          seqx;

   /** The set mask value. */
   OSUINT16     mask[ASN1_K_CCBMaskSize];

   /** The bytes processed by the block, used for streaming. */
   OSSIZE       bytes;

   /** The status, as returned by BS_CHKEND. */
   int          stat;
} ASN1CCB;

/** ASN.1 dump utility callback function definition */
typedef int (*ASN1DumpCbFunc) (const char* text_p, void* cbArg_p);

/** @} */ // close the cruntime group.  We reopen it later.

/*
I don't know why the following memory allocation macros are defined here.  They
seem to belong to the rtmem group, so I've designated that here.
If we don't close the cruntime group first, the rtmem group will become a
subgroup of the cruntime group.
*/

/**
 * @addtogroup rtmem
 *
 * @{
 */
/**
 * Allocate a dynamic array. This macro allocates a dynamic array of records of
 * the given type. This version of the macro will return the RTERR_NOMEM error
 * status if the memory request cannot be fulfilled.
 *
 * @param pctxt        - Pointer to a context block
 * @param pseqof       - Pointer to a generated SEQUENCE OF array structure.
 *                       The <i>n</i> member variable must be set to the number
 *                       of records to allocate.
 * @param type         - Data type of an array record
 */
#define ALLOC_ASN1ARRAY(pctxt,pseqof,type) do {\
if (sizeof(type)*(pseqof)->n < (pseqof)->n) return RTERR_NOMEM; \
if (((pseqof)->elem = (type*) rtxMemHeapAllocZ \
(&(pctxt)->pMemHeap, sizeof(type)*(pseqof)->n)) == 0) return RTERR_NOMEM; \
} while (0)

/**
 * Allocate a dynamic array. This macro allocates a dynamic array of records of
 * the given type. This version of the macro will set the internal parameters
 * of the SEQUENCE OF structure to NULL if the memory request cannot be
 * fulfilled.
 *
 * @param pctxt        - Pointer to a context block
 * @param pseqof       - Pointer to a generated SEQUENCE OF array structure.
 *                       The <i>n</i> member variable must be set to the number
 *                       of records to allocate.
 * @param type         - Data type of an array record
 */
#define ALLOC_ASN1ARRAY1(pctxt,pseqof,type) do {\
if (sizeof(type)*(pseqof)->n < (pseqof)->n) (pseqof)->elem = 0; \
else (pseqof)->elem = (type*) rtxMemHeapAllocZ \
(&(pctxt)->pMemHeap, sizeof(type)*(pseqof)->n); \
} while (0)

/**
 * @}
 */

/**
 * @addtogroup cruntime
 * @{
 */      //Reopen the cruntime group.
#define ASN1NUMOCTS(nbits)      ((nbits>0)?(((nbits-1)/8)+1):0)

#ifdef __cplusplus
}
#endif

#include "rtsrc/rtContext.h"
#include "rtxsrc/rtxCommonDefs.h"
#include "rtxsrc/rtxError.h"
#include "rtxsrc/rtxMemory.h"

/**
 * @}
 */

#endif
