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

#ifndef _ASN1MDER_H_
#define _ASN1MDER_H_

/**
 * @file rtmdersrc/asn1mder.h
 * asn1mder.h contains the ASN.1 runtime constants, data structure definitions,
 * and functions to support the Medical Device Encoding Rules defined in IEEE
 * standard 11073-20601-2008, Annex F.
 */

#include "rtsrc/asn1type.h"
#include "rtxsrc/rtxBuffer.h"

/**
 * @defgroup mderruntime MDER Runtime Library Functions and Macros.
 * @{
 *
 * The Medical Device Encoding Rules (MDER) library contains functions and
 * macros used by ASN1C for reading and writing messages that conform to the
 * specifications laid out in IEEE standard 11073-20601-2008, annex F.  MDER
 * is used to facilitate communication between medical devices using an
 * octet-oriented exchange specified in a strict subset of ASN.1.  The low-level
 * C encode/decode functions are identified respectively by the mderEnc and
 * mderDec prefixes.
 */

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __SYMBIAN32__       /* For Symbian */
#define EXTMDERCLASS

#ifdef BUILDASN1BERDLL
#define EXTERNMDER      EXPORT_C
#define EXTMDERMETHOD   EXPORT_C
#elif defined(USEASN1BERDLL)
#define EXTERNMDER      IMPORT_C
#define EXTMDERMETHOD   IMPORT_C
#else
#define EXTERNMDER
#define EXTMDERMETHOD
#endif /* BUILDASN1BERDLL */

#else                      /* Other O/S's */
#define EXTMDERMETHOD

#ifdef BUILDASN1BERDLL
#define EXTERNMDER      __declspec(dllexport)
#define EXTMDERCLASS    __declspec(dllexport)
#elif defined (USEASN1BERDLL)
#define EXTERNMDER      __declspec(dllimport)
#define EXTMDERCLASS    __declspec(dllimport)
#else
#define EXTERNMDER
#define EXTMDERCLASS
#endif /* BUILDASN1BERDLL */

#endif

/** @defgroup mderctxtruntime MDER Runtime Context functions.
 *
 * These functions are used to initialize an OSCTXT structure
 * for encoding or decoding.  If the context fails to initialize, an error
 * RTERR_NOTINIT will be returned.  This is typically due to an
 * invalid license.
 */

/**
 * This function initializes an OSCTXT structure for MDER functions.  All
 * key working parameters will be set to their correct initial state values if
 * the context has not yet been initialized.
 *
 * @param      pctxt          A pointer to a context structure.
 *
 * @return     The status of the operation: 0 on success, a negative value on
 *             failure.
 */
#ifndef mderInitContext
EXTERNMDER int mderInitContext (OSCTXT* pctxt);
#endif

/**
 * This function initializes an OSCTXT structure for MDER functions using a
 * particular run time key value.  This is needed for limited redistribution
 * run time libraries.  All key working parameters will be set to their
 * correct initial state values if the context has not yet been initialized.
 *
 * @see        ::mderInitContext
 *
 * @param      pctxt          A pointer to a context structure.
 *
 * @return     The status of the operation: 0 on success, a negative value on
 *             failure.
 */
EXTERNMDER int mderInitContextUsingKey
   (OSCTXT* pctxt, const OSOCTET* key, size_t keylen);

/** @} mderctxtruntime */

/*  decoding functions */
/**
 * @defgroup mderdecruntime MDER C Decode Functions.
 * @{
 *
 * MDER C decoding functions handle decoding primitive ASN.1-encoded data
 * types within an MDER message.  Calls to these functions are generated by
 * the compiler and to decode complex ASN.1 structures.
 *
 * MDER tags and lengths must be 16 bits wide, and therefore are treated as
 * unsigned integer types.  Sized data structures have no length encoded,
 * and tags are only used in CHOICE structures (in which the tag is used to
 * differentiate different CHOICE options).
 */

/**
 * This macro reads a single byte from the input stream.
 */
#define MD_FETCH1(pctxt, object_p) \
   (rtxReadBytes(pctxt, object_p, 1) >= 0 ? \
      0 : LOG_RTERR(pctxt, RTERR_READERR))

/**
 * This macro reads two bytes from the input stream.
 */
#define MD_FETCH2(pctxt, object_p) \
   (rtxReadBytes(pctxt, object_p, 2) >= 0 ? \
      0 : LOG_RTERR(pctxt, RTERR_READERR))

/**
 * This macro reads four bytes from the input stream.
 */
#define MD_FETCH4(pctxt, object_p) \
   (rtxReadBytes(pctxt, object_p, 4) >= 0 ? \
      0 : LOG_RTERR(pctxt, RTERR_READERR))

/**
 * This function decodes an 8-bit integer from the input
 * stream.  In MDER, integers are stored in two's complement form when signed
 * and absolute value when unsigned.  Both signed and unsigned 8-bit
 * integers will fit into a single byte.
 *
 * @param      pctxt       A pointer to an OSCTXT data structure.
 * @param      object_p    A pointer to an 8-bit integer to hold the value.
 *
 * @return                 Zero on success, non-zero on failure.
 */
EXTERNMDER int mderDecInt8(OSCTXT *pctxt, OSINT8 *object_p);

/**
 * This defines the unsigned integer decode to be the same as the signed
 * integer decode.
 */
#define mderDecUInt8(pctxt, object_p) mderDecInt8(pctxt, (OSINT8*)object_p)

/**
 * This function decodes a 16-bit integer from the input
 * stream.  In MDER, integers are stored in two's complement form when signed
 * and absolute value when unsigned.  Both signed and unsigned 16-bit integers
 * will fit into two bytes.
 *
 * @param      pctxt       A pointer to an OSCTXT data structure.
 * @param      object_p    A pointer to a 16-bit integer to hold the value.
 *
 * @return                 Zero on success, non-zero on failure.
 */
EXTERNMDER int mderDecInt16(OSCTXT *pctxt, OSINT16 *object_p);

/**
 * This defines the unsigned integer decode to be the same as the signed
 * integer decode.
 */
#define mderDecUInt16(pctxt, object_p) mderDecInt16(pctxt, (OSINT16*)object_p)

/**
 * This function decodes a 32-bit integer from the input
 * stream.  In MDER, integers are stored in two's complement form when signed
 * and absolute value when unsigned.  Both signed and unsigned 32-bit integers
 * will fit into four bytes.
 *
 * @param      pctxt       A pointer to an OSCTXT data structure.
 * @param      object_p    A pointer to a 32-bit integer to hold the value.
 *
 * @return                 Zero on success, non-zero on failure.
 */
EXTERNMDER int mderDecInt32(OSCTXT *pctxt, OSINT32 *object_p);

/**
 * This defines the unsigned integer decode to be the same as the signed
 * integer decode.
 */
#define mderDecUInt32(pctxt, object_p) mderDecInt32(pctxt, (OSINT32*)object_p)

/**
 * This macro is a handle for decoding an 8-bit bit string from the input
 * stream.
 */
#define mderDecBitStr8(pctxt, object_p) \
   ((ASN1BitStr32 *)object_p)->numbits = 8; \
   MD_FETCH1(pctxt, ((ASN1BitStr32 *)object_p)->data)

/**
 * This macro is a handle for decoding a 16-bit bit string from the input
 * stream.
 */
#define mderDecBitStr16(pctxt, object_p) \
   ((ASN1BitStr32 *)object_p)->numbits = 16; \
   MD_FETCH2(pctxt, ((ASN1BitStr32 *)object_p)->data)

/**
 * This macro is a handle for decoding a 32-bit bit string from the input
 * stream.
 */
#define mderDecBitStr32(pctxt, object_p) \
   ((ASN1BitStr32 *)object_p)->numbits = 32; \
   MD_FETCH4(pctxt, ((ASN1BitStr32 *)object_p)->data)

/**
 * Decode an octet string from the input stream.  This function decodes
 * the octet string into a static structure rather than allocating
 * new memory.  It is intended for use with sized octet strings (which in MDER
 * are encoded without any length determinant).
 *
 * @param   pctxt          A pointer to an ::OSCTXT data structure.
 * @param   numocts        The length in octets of the octet string.  Use 0
 *                         if the buffer is statically allocated and the length
 *                         is unknown.
 * @param   pbuf           The buffer used to hold the octet string.
 *
 * @see     ::mderDecDynOctStr
 *
 * @return  0 on success, less than 0 on error.
 */
EXTERNMDER int mderDecOctStr (OSCTXT *pctxt, OSOCTET *pbuf, OSUINT16 numocts);

/**
 * Decode an octet string from the input stream.  This function decodes
 * the octet string into a dynamically-allocated structure.  It is intended
 * for use with unsized octet strings (which in MDER will prefix with a
 * two-byte length field).
 *
 * @param   pctxt          A pointer to an ::OSCTXT data structure.
 * @param   pnumocts       A pointer to an integer to receive the number of
 *                         octets decoded from the message.
 * @param   ppbuf          A pointer to a pointer that holds a buffer to
 *                         receive the octet string.  This will be dynamically
 *                         allocated.
 *
 * @see     ::mderDecOctStr
 *
 * @return  0 on success, less than 0 on error.
 */
EXTERNMDER int mderDecDynOctStr (OSCTXT *pctxt, const OSOCTET **ppbuf,
   OSUINT16 *pnumocts);

/**
 * Decode an open type from the input stream.  Open types in MDER are
 * specified in ASN.1 via the ANY DEFINED BY type and encoded using
 * a two-byte length header followed by the encoding of the specified
 * object.  The decoded octets may be used to initialize another buffer in a
 * two-phase decoding step.  Memory to hold the open type will be allocated
 * dynamically.
 *
 * @param   pctxt          A pointer to an ::OSCTXT data structure.
 * @param   pnumocts       A pointer to an integer to receive the number of
 *                         octets decoded for the open type.
 * @param   ppoctets       A pointer to a pointer that holds the octets that
 *                         comprise the octet string.  These octets will be
 *                         dynamically allocated.
 *
 * @return  0 on success, less than 0 on error.
 */
#define mderDecOpenType(pctxt, pnumocts, ppoctets) \
   mderDecDynOctStr(pctxt, pnumocts, ppoctets)

/** @} mderdecruntime */

/*  encoding functions */
/**
 * @defgroup mderencruntime MDER C Encode Functions
 * @{
 *
 * MDER C encoding functions handle the encoding of primitive ASN.1 types in
 * an MDER message.  Calls to these functions are generated by the compiler to
 * encode complex message structures.
 *
 * MDER avoids encoding data that may be supplied by the input specification
 * and thus does not include tags or lengths if at all possible.
 */

/**
 * This macro writes a single byte to the output stream.
 */
#define ME_SAFEPUT1(pctxt, object_p) \
   ((rtxWriteBytes(pctxt, object_p, 1) == 0) ? \
      0 : LOG_RTERR(pctxt, RTERR_WRITEERR))

/**
 * This macro writes two bytes to the output stream.
 */
#define ME_SAFEPUT2(pctxt, object_p) \
   ((rtxWriteBytes(pctxt, object_p, 2) == 0) ? \
      0 : LOG_RTERR(pctxt, RTERR_WRITEERR))

/**
 * This macro writes four bytes to the output stream.
 */
#define ME_SAFEPUT4(pctxt, object_p) \
   ((rtxWriteBytes(pctxt, object_p, 4) == 0) ? \
      0 : LOG_RTERR(pctxt, RTERR_WRITEERR))

/**
 * This function encodes an 8-bit integer to the output stream.
 * All 8-bit integers will fit in one byte in MDER because unsigned integers
 * are represented by the absolute value and signed integers by two's
 * complement.
 *
 * @param      pctxt       A pointer to an OSCTXT data structure.
 * @param      value       An 8-bit integer value.
 *
 * @return                 Zero on success, non-zero on failure.
 */
EXTERNMDER int mderEncInt8 (OSCTXT *pctxt, OSINT8 value);

/**
 * This defines the unsigned integer encode to be the same as the signed
 * integer decode.
 */
#define mderEncUInt8(pctxt,value) mderEncInt8(pctxt, (OSINT8)value)

/**
 * This function encodes a 16-bit integer to the output stream.
 * All 16-bit integers will fit in two bytes in MDER because unsigned integers
 * are represented by the absolute value and signed integers by two's
 * complement.
 *
 * @param      pctxt       A pointer to an OSCTXT data structure.
 * @param      value       A 16-bit integer value.
 *
 * @return                 Zero on success, non-zero on failure.
 */
EXTERNMDER int mderEncInt16 (OSCTXT *pctxt, OSINT16 value);

/**
 * This defines the unsigned integer encode to be the same as the signed
 * integer decode.
 */
#define mderEncUInt16(pctxt,value) mderEncInt16(pctxt, (OSINT16)value)

/**
 * This function encodes a 32-bit integer to the output stream.
 * All 32-bit integers will fit in four bytes in MDER because unsigned integers
 * are represented by the absolute value and signed integers by two's
 * complement.
 *
 * @param      pctxt       A pointer to an OSCTXT data structure.
 * @param      value       A 32-bit integer value.
 *
 * @return                 Zero on success, non-zero on failure.
 */
EXTERNMDER int mderEncInt32 (OSCTXT *pctxt, OSINT32 value);

/**
 * This defines the unsigned integer encode to be the same as the signed
 * integer decode.
 */
#define mderEncUInt32(pctxt,value) mderEncInt32(pctxt, (OSINT32)value)

/**
 * This macro is a handle to writing an 8-bit bit string to the output stream.
 * All bit strings must be sized to 8-, 16-, or 32-bits in MDER.
 */
#define mderEncBitStr8(pctxt, object_p) ME_SAFEPUT1(pctxt, object_p)

/**
 * This macro is a handle to writing a 16-bit bit string to the output stream.
 * All bit strings must be sized to 8-, 16-, or 32-bits in MDER.
 */
#define mderEncBitStr16(pctxt, object_p) ME_SAFEPUT2(pctxt, object_p)

/**
 * This macro is a handle to writing a 32-bit bit string to the output stream.
 * All bit strings must be sized to 8-, 16-, or 32-bits in MDER.
 */
#define mderEncBitStr32(pctxt, object_p) ME_SAFEPUT4(pctxt, object_p)

/**
 * Encodes an octet string to the output stream.  In MDER, octet strings whose
 * sizes are known a priori must not encode their length.  Octet strings whose
 * lengths are unspecified must have an encoded length.  Lengths must consume
 * two bytes in the output, so octet strings are limited to at most 65535
 * bytes.
 *
 * @param   pctxt          A pointer to an ::OSCTXT data structure.
 * @param   numocts        The number of octets to encode.
 * @param   pocts          A pointer to the octets to be encoded.
 * @param   enclen         A boolean that specifies whether to encode the
 *                         length or not.
 *
 * @return  0 on success, less than zero on error.
 */
EXTERNMDER int mderEncOctStr2 (OSCTXT *pctxt, const OSOCTET *pocts,
   OSUINT16 numocts, OSBOOL enclen);

/**
 * This encodes a fixed-length octet string to the output stream.
 *
 * @see ::mderEncOctStr2
 */
#define mderEncOctStr(pctxt, pocts, numocts) \
   mderEncOctStr2(pctxt, pocts, numocts, FALSE)

/**
 * This encodes a dynamic octet string to the output stream.
 *
 * @see ::mderEncOctStr2
 */
#define mderEncDynOctStr(pctxt, pocts, numocts) \
   mderEncOctStr2(pctxt, pocts, numocts, TRUE)

/** @} mderencruntime */

#ifdef __cplusplus
}
#endif

/** @} mderruntime */

#endif /* _ASN1MDER_H_ */



