/*
* Copyright (c) 2003-2023 Objective Systems, Inc.
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
/**
* @file rtAvn.h
*/

#ifndef _RTAVN_H_
#define _RTAVN_H_

#include "rtxsrc/osSysTypes.h"
#include "rtxsrc/rtxContext.h"
#include "rtxsrc/rtxMemBuf.h"
#include "asn1type.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Run-time functions for reading/writing values to/from the context buffer
   in (ASN.1) Abstract Value Notation form.
*/

/**
* @defgroup avnReadWrite ASN.1 Value Notation Reading and Writing Functions
* @{
*
* These functions support working with ASN.1 value notation.
* Values can be written to, or read from, the context buffer, with the buffer
* holding the text for the ASN.1 value notation.
*/


/**
 * Search for the given identifier in the given table.
 *
 * If the table contains a null entry and no other entry matches, the index
 * for that entry is returned.  This is used to represent an extension
 * point.
 *
 * @param id The identifier to search for.
 * @param table Array of identifiers to search in.
 * @param numIds Size of the given table.
 * @return The matched index, or numIds if there is no match and the table
 *          did not contain a null entry.
 */
EXTERNRT OSSIZE rtAvnLookupIdentifier(const char* id, const char* table[],
                                    OSSIZE numIds);

/**
* This function matches a token in ASN.1 value notation input.  This can be
* used for matching an ASN.1 identifier or reserved word.  This will first
* skip any whitespace and then match the given characters and then verify that
* any following characters could not misinterpreted as being part of the token.
*
* If an error is returned, as many characters as were matched will have been
* consumed.
*
* @param pctxt        Pointer to context block structure.
* @param token        The token to match.
* @return             Completion status of operation:
*                       - 0 = success,
*                       - negative return value is error.
*/
EXTERNRT int rtAvnMatchToken(OSCTXT* pctxt, const OSUTF8CHAR* token);


/**
 * This function decodes an ASN.1 BIT STRING value.
 * If the BIT STRING has named bits, the parameters allow this function to
 * ensure trailing zero bits are trimmed/added in conjuction with the minimum
 * required length.
 *
 * @param pctxt        Pointer to context block structure.
 * @param buffer        Buffer to receive decoded value.
 * @param pnbits        Receives the number of bits in the BIT STRING.
 * @param bufsize       Size of the buffer, in bytes.
 * @param hasNamedBits  TRUE if BIT STRING type has named bits
 * @param minLen        Minimum length, if hasNamedBits is true.
 * @return             Completion status of operation:
 *                       - 0 = success,
 *                       - negative return value is error.
 */
EXTERNRT int rtAvnReadBitString(OSCTXT* pctxt, OSOCTET* buffer, OSUINT32* pnbits,
   OSSIZE bufsize, OSBOOL hasNamedBits, OSSIZE minLen);


/**
 * This function decodes an ASN.1 BIT STRING value.
 * If the BIT STRING has named bits, the parameters allow this function to
 * ensure trailing zero bits are trimmed/added in conjuction with the minimum
 * required length.
 *
 * @param pctxt        Pointer to context block structure.
 * @param buffer        Buffer to receive decoded value.
 * @param pnbits        Receives the number of bits in the BIT STRING.
 * @param bufsize       Size of the buffer, in bytes.
 * @param hasNamedBits  TRUE if BIT STRING type has named bits
 * @param minLen        Minimum length, if hasNamedBits is true.
 * @return             Completion status of operation:
 *                       - 0 = success,
 *                       - negative return value is error.
 */
EXTERNRT int rtAvnReadBitString64(OSCTXT* pctxt, OSOCTET* buffer,
   OSSIZE* pnbits, OSSIZE bufsize, OSBOOL hasNamedBits, OSSIZE minLen);

/**
 * This function decodes an ASN.1 BIT STRING value into a static buffer,
 * with any overflow bits decoded into a dynamic buffer.
 *
 * If the BIT STRING has named bits, the parameters allow this function to
 * ensure trailing zero bits are trimmed/added in conjuction with the minimum
 * required length.
 *
 * @param pctxt        Pointer to context block structure.
 * @param buffer        Buffer to receive decoded value.
 * @param pnbits        Receives the number of bits in the BIT STRING.
 * @param bufsize       Size of the buffer, in bytes.
 * @param pextdata      Receives pointer to bits that don't fit in buffer.
 * @param hasNamedBits  TRUE if BIT STRING type has named bits
 * @param minLen        Minimum length, if hasNamedBits is true.
 * @return             Completion status of operation:
 *                       - 0 = success,
 *                       - negative return value is error.
 */
EXTERNRT int rtAvnReadBitStringExt(OSCTXT* pctxt, OSOCTET* buffer,
   OSUINT32* pnbits, OSSIZE bufsize, OSOCTET** pextdata, OSBOOL hasNamedBits,
   OSSIZE minLen);

/**
 * This function decodes an ASN.1 BIT STRING value into a static buffer,
 * with any overflow bits decoded into a dynamic buffer.
 *
 * If the BIT STRING has named bits, the parameters allow this function to
 * ensure trailing zero bits are trimmed/added in conjuction with the minimum
 * required length.
 *
 * @param pctxt        Pointer to context block structure.
 * @param buffer        Buffer to receive decoded value.
 * @param pnbits        Receives the number of bits in the BIT STRING.
 * @param bufsize       Size of the buffer, in bytes.
 * @param pextdata      Receives pointer to bits that don't fit in buffer.
 * @param hasNamedBits  TRUE if BIT STRING type has named bits
 * @param minLen        Minimum length, if hasNamedBits is true.
 * @return             Completion status of operation:
 *                       - 0 = success,
 *                       - negative return value is error.
 */
EXTERNRT int rtAvnReadBitStringExt64(OSCTXT* pctxt, OSOCTET* buffer,
   OSSIZE* pnbits, OSSIZE bufsize, OSOCTET** pextdata, OSBOOL hasNamedBits,
   OSSIZE minLen);

/**
 * This function decodes an ASN.1 BIT STRING value.
 * If the BIT STRING has named bits, the parameters allow this function to
 * ensure trailing zero bits are trimmed/added in conjuction with the minimum
 * required length.
 *
 * @param pctxt        Pointer to context block structure.
 * @param pvalue       Receives the decoded BIT STRING value.
 * @param hasNamedBits  TRUE if BIT STRING type has named bits
 * @param minLen        Minimum length, if hasNamedBits is true.
 * @return             Completion status of operation:
 *                       - 0 = success,
 *                       - negative return value is error.
 */
EXTERNRT int rtAvnReadBitStringDyn(OSCTXT* pctxt, ASN1DynBitStr* pvalue,
   OSBOOL hasNamedBits, OSSIZE minLen);

/**
 * This function decodes an ASN.1 BIT STRING value.
 * If the BIT STRING has named bits, the parameters allow this function to
 * ensure trailing zero bits are trimmed/added in conjuction with the minimum
 * required length.
 *
 * @param pctxt        Pointer to context block structure.
 * @param pvalue       Receives the decoded BIT STRING value.
 * @param hasNamedBits  TRUE if BIT STRING type has named bits
 * @param minLen        Minimum length, if hasNamedBits is true.
 * @return             Completion status of operation:
 *                       - 0 = success,
 *                       - negative return value is error.
 */
EXTERNRT int rtAvnReadBitStringDyn64(OSCTXT* pctxt, ASN1DynBitStr64* pvalue,
   OSBOOL hasNamedBits, OSSIZE minLen);


/**
 * This function decodes an ASN.1 BIT STRING encoded as a list of named bit
 * identifiers.
 *
 * @param pctxt        Pointer to context block structure.
 * @param pBitMap      Pointer to bit map structure that defined token to
 *                       bit mappings.
 * @param pvalue       Pointer to buffer to recieve decoded bit map.
 * @param pnbits       Number of bits in decoded bit map.
 * @param bufsize      Size of buffer passed in to received decoded bit
 *                       values.
 * @return             Completion status of operation:
 *                       - 0 = success,
 *                       - negative return value is error.
 */
EXTERNRT int rtAvnReadNamedBits
(OSCTXT* pctxt, const OSBitMapItem* pBitMap,
   OSOCTET* pvalue, OSUINT32* pnbits, OSUINT32 bufsize);

/**
 * This function decodes an ASN.1 BIT STRING encoded as a list of named bit
 * identifiers.
 *
 * @param pctxt        Pointer to context block structure.
 * @param pBitMap      Pointer to bit map structure that defined token to
 *                       bit mappings.
 * @param pvalue       Pointer to buffer to recieve decoded bit map.
 * @param pnbits       Number of bits in decoded bit map.
 * @param bufsize      Size of buffer passed in to received decoded bit
 *                       values.
 * @return             Completion status of operation:
 *                       - 0 = success,
 *                       - negative return value is error.
 */
EXTERNRT int rtAvnReadNamedBits64
(OSCTXT* pctxt, const OSBitMapItem* pBitMap,
   OSOCTET* pvalue, OSSIZE* pnbits, OSSIZE bufsize);



/**
 * This function decodes an ASN.1 BOOLEAN value.
 * @param pctxt        Pointer to context block structure.
 * @param pvalue      Pointer to an OSBOOL to receive the value.
  * @return             Completion status of operation:
 *                       - 0 = success,
 *                       - negative return value is error.
 */
EXTERNRT int rtAvnReadBoolean(OSCTXT* pctxt, OSBOOL* pvalue);

/**
* This function decodes an ASN.1 identifier, first skipping any whitespace.
*
* @param pctxt        Pointer to context block structure.
* @param ppvalue      Pointer to an string to receive the
*                       decoded identifier.  Memory is allocated for the string
*                       using the run-time memory manager.
* @return             Completion status of operation:
*                       - 0 = success,
*                       - negative return value is error.
*/
EXTERNRT int rtAvnReadIdentifier(OSCTXT* pctxt, char** ppvalue);


/**
* This function decodes an ASN.1 identifier, first skipping any whitespace.
* The identifier is read into a memory buffer.  This form of the function is
* useful for when a series of identifiers is to be read, as it allows reuse
* of dynamically allocated memory.
*
* @param pctxt        Pointer to context block structure.
* @param pmembuf      A pre-initialized buffer to read into.
*                       OSMEMBUFPTR(pmembuf) will hold the identifier.
* @return             Completion status of operation:
*                       - 0 = success,
*                       - negative return value is error.
*/
int rtAvnReadIdentifierToBuffer(OSCTXT* pctxt, OSRTMEMBUF* pmembuf);

/**
 * This function decodes an ASN.1 OCTET STRING value.
 *
 * @param pctxt        Pointer to context block structure.
 * @param buffer        Buffer to receive decoded value.
 * @param pnocts        Receives the number of octets in the OCTET STRING.
 * @param bufsize       Size of the buffer, in bytes.
 * @return             Completion status of operation:
 *                       - 0 = success,
 *                       - negative return value is error.
 */
EXTERNRT int rtAvnReadOctString(OSCTXT* pctxt, OSOCTET* buffer,
   OSUINT32* pnocts, OSSIZE bufsize);



/**
 * This function decodes an ASN.1 OCTET STRING value.
 *
 * @param pctxt        Pointer to context block structure.
 * @param buffer        Buffer to receive decoded value.
 * @param pnocts        Receives the number of octets in the OCTET STRING.
 * @param bufsize       Size of the buffer, in bytes.
 * @return             Completion status of operation:
 *                       - 0 = success,
 *                       - negative return value is error.
 */
EXTERNRT int rtAvnReadOctString64(OSCTXT* pctxt, OSOCTET* buffer,
   OSSIZE* pnocts, OSSIZE bufsize);


/**
 * This function decodes an ASN.1 OCTET STRING value.
 *
 * @param pctxt        Pointer to context block structure.
 * @param pvalue       Receives the decoded OCTET STRING value.
 * @return             Completion status of operation:
 *                       - 0 = success,
 *                       - negative return value is error.
 */
EXTERNRT int rtAvnReadOctStringDyn(OSCTXT* pctxt, OSDynOctStr* pvalue);


/**
 * This function decodes an ASN.1 OCTET STRING value.
 *
 * @param pctxt        Pointer to context block structure.
 * @param pvalue       Receives the decoded OCTET STRING value.
 * @return             Completion status of operation:
 *                       - 0 = success,
 *                       - negative return value is error.
 */
EXTERNRT int rtAvnReadOctStringDyn64(OSCTXT* pctxt, OSDynOctStr64* pvalue);


/**
 * This function decodes an ASN.1 OBJECT IDENTIFIER value.
 *
 * @param pctxt        Pointer to context block structure.
 * @param pvalue       Receives the decoded OBJECT IDENTIFIER value.
 * @return             Completion status of operation:
 *                       - 0 = success,
 *                       - negative return value is error.
 */
EXTERNRT int rtAvnReadObjId(OSCTXT* pctxt, ASN1OBJID* pvalue);

/**
 * Read an ASN.1 open type value into a dynamically allocated buffer of bytes.
 * The AVN for the open type is captured.
 *
 * @param pctxt         Pointer to context block structure.
 * @param ppdata        Pointer to receive pointer to data. Dynamic memory is
 *                      allocated for the string using rtxMemAlloc function.
 * @param pnocts        Receives the number of octets in *ppdata.
 * @return             Completion status of operation:
 *                       - 0 = success,
 *                       - negative return value is error.
 */
EXTERNRT int rtAvnReadOpenType(OSCTXT* pctxt, const OSOCTET** ppdata,
   OSSIZE* pnocts);

/**
 * This function reads an ASN.1 REAL value into an OSREAL.
 * Leading whitespace is skipped.
 *
 * @param pctxt        Pointer to context block structure.
 * @param ppCharStr    Pointer to character string pointer to receive
 *                       decoded value.  Dynamic memory is allocated for
 *                       the string using the rtxMemAlloc function.
 * @return             Completion status of operation:
 *                       - 0 = success,
 *                       - negative return value is error.
 */
EXTERNRT int rtAvnReadReal(OSCTXT* pctxt, OSREAL* pvalue);

/**
 * This function reads an ASN.1 realnumber (X.680 12.9), into a character
 * string.  It may have a leading '-' sign.
 * Leading whitespace is skipped.
 *
 * @param pctxt        Pointer to context block structure.
 * @param ppCharStr    Pointer to character string pointer to receive
 *                       decoded value.  Dynamic memory is allocated for
 *                       the string using the rtxMemAlloc function.
 * @return             Completion status of operation:
 *                       - 0 = success,
 *                       - negative return value is error.
 */
EXTERNRT int rtAvnReadRealString(OSCTXT* pctxt, char** ppCharStr);


/**
 * This function decodes an ASN.1 RELATIVE-OID value.
 *
 * @param pctxt        Pointer to context block structure.
 * @param pvalue       Receives the decoded RELATIVE-OID value.
 * @return             Completion status of operation:
 *                       - 0 = success,
 *                       - negative return value is error.
 */
EXTERNRT int rtAvnReadRelOID(OSCTXT* pctxt, ASN1OBJID* pvalue);

/**
 * Read an ASN.1 cstring (X.680 12.14) into a dynamically allocated buffer.
 *
 * @param pctxt        Pointer to context block structure.
 * @param ppCharStr    Pointer to character string pointer to receive
 *                       decoded value.  Dynamic memory is allocated for
 *                       the string using the rtxMemAlloc function.
 * @return             Completion status of operation:
 *                       - 0 = success,
 *                       - negative return value is error.
 */
EXTERNRT int rtAvnReadString(OSCTXT* pctxt, OSUTF8CHAR** ppCharStr);


/**
 * Read an ASN.1 cstring (X.680 12.14) into a dynamically allocated buffer
 * of 16-bit chars (not zero terminated).
 *
 * @param pctxt        Pointer to context block structure.
 * @param ppCharStr    Pointer to character string pointer to receive
 *                       decoded value.  Dynamic memory is allocated for
 *                       the string using the rtxMemAlloc function.
 * @param pnchars      Pointer to receive number of decoded characters.
 * @return             Completion status of operation:
 *                       - 0 = success,
 *                       - negative return value is error.
 */
EXTERNRT int rtAvnReadUCS2(OSCTXT* pctxt, OSUNICHAR** ppCharStr,
   OSSIZE* pnchars);

/**
 * Read an ASN.1 cstring (X.680 12.14) into a dynamically allocated buffer
 * of 32-bit chars (not zero terminated).
 *
 * @param pctxt        Pointer to context block structure.
 * @param ppCharStr    Pointer to character string pointer to receive
 *                       decoded value.  Dynamic memory is allocated for
 *                       the string using the rtxMemAlloc function.
 * @param pnchars      Pointer to receive number of decoded characters.
 * @return             Completion status of operation:
 *                       - 0 = success,
 *                       - negative return value is error.
 */
EXTERNRT int rtAvnReadUCS4(OSCTXT* pctxt, OS32BITCHAR** ppCharStr,
   OSSIZE* pnchars);


/**
 * This writes an ASN.1 BIT STRING having named bits to the context buffer
 * using ASN.1 value notation.  The format will be hstring, or
 * { IdentifierList }, as appropriate for the value.
 *
 * Note: this may clear unused bits.
 *
 * @param pctxt        Pointer to context block structure.
 * @param nbits        The total number of bits in the BIT STRING.
 * @param pdata        Pointer to the location of (up to) the first datalen * 8
 *                      bits.
 * @param datalen      Size, in bytes, of pdata.  Ignored if pextdata is null.
 * @param pextdata     Pointer to remaining bits (beyond first datalen * 8)
 *                     bits.  This may be null if there are not extra bits.
 * @param pdict        Named bit dictionary, sorted by bit index, with a
 *                     null (named) entry to mark the end.
 * @return             Completion status of operation:
 *                       - 0 = success,
 *                       - negative return value is error.
 */
EXTERNRT int rtAvnWriteBitStrNamed(OSCTXT* pctxt, OSSIZE nbits,
                                    OSOCTET* pdata, OSSIZE datalen,
                                    OSOCTET* pextdata,
                                    const OSBitMapItem* pdict);

/**
* This writes an ASN.1 BIT STRING to the context buffer using ASN.1 value
* notation.  The format will be either bstring or hstring.
*
* @param pctxt        Pointer to context block structure.
* @param nbits        The total number of bits in the BIT STRING.
* @param pdata        Pointer to the location of up to the first datalen * 8
*                      bits.
* @param datalen      Size, in bytes, of pdata. Ignored if pextdata is null.
* @param pextdata     Pointer to remaining bits (beyond first datalen * 8)
*                     bits.  This may be null if there are not extra bits.
* @return             Completion status of operation:
*                       - 0 = success,
*                       - negative return value is error.
*/
EXTERNRT int rtAvnWriteBitStr(OSCTXT* pctxt, OSSIZE nbits,
   const OSOCTET* pdata, OSSIZE datalen,
   const OSOCTET* pextdata);


/**
* This writes an ASN.1 UniversalString to the context buffer using ASN.1 value
* notation. ASN.1 value notation uses two double-quote characters for each
* double-quote character in the string.
*
* @param pctxt        Pointer to context block structure.
* @param str          Value to be encoded.
* @param nchars       Length of string, in characters.
* @return             Completion status of operation:
*                       - 0 = success,
*                       - negative return value is error.
*/
EXTERNRT int rtAvnWriteBMPStr(OSCTXT* pctxt, const OSUNICHAR* str,
                              OSSIZE nchars);


/**
 * This writes an ASN.1 character string value to context buffer using ASN.1
 * value notation. ASN.1 value notation uses two double-quote characters for
 * each double-quote character in the string.
 */
EXTERNRT int rtAvnWriteCharStr(OSCTXT* pctxt, const char* str);

/**
* This writes an ASN.1 OCTET STRING to the context buffer using ASN.1 value
* notation.  The format will be an hstring.
*
* @param pctxt        Pointer to context block structure.
* @param nocts        The total number of octets in the OCTET STRING.
* @param pdata        Pointer to the octet string data
* @return             Completion status of operation:
*                       - 0 = success,
*                       - negative return value is error.
*/
EXTERNRT int rtAvnWriteOctStr(OSCTXT* pctxt, OSSIZE nocts,
                              const OSOCTET* pdata);


/**
 * This writes an ASN.1 OBJECT IDENTIFIER or RELATIVE-OID to the context buffer
 * using ASN.1 value notation.
 *
 * @param pctxt        Pointer to context block structure.
 * @param pvalue       Value to be encoded.
 * @return             Completion status of operation:
 *                       - 0 = success,
 *                       - negative return value is error.
 */
EXTERNRT int rtAvnWriteOID(OSCTXT* pctxt, const ASN1OBJID* pvalue);

/**
 * This writes an ASN.1 REAL to the context buffer using ASN.1 value notation.
 *
 * @param pctxt        Pointer to context block structure.
 * @param value        Value to be encoded.
 * @return             Completion status of operation:
 *                       - 0 = success,
 *                       - negative return value is error.
 */
EXTERNRT int rtAvnWriteReal(OSCTXT* pctxt, OSREAL value);

/**
* This writes an ASN.1 UniversalString to the context buffer using ASN.1 value
* notation. ASN.1 value notation uses two double-quote characters for each
* double-quote character in the string.
*
* @param pctxt        Pointer to context block structure.
* @param str          Value to be encoded.
* @param nchars       Length of string, in characters.
* @return             Completion status of operation:
*                       - 0 = success,
*                       - negative return value is error.
*/
EXTERNRT int rtAvnWriteUnivStr(OSCTXT* pctxt, const OS32BITCHAR* str,
                                 OSSIZE nchars);

/**
 * This writes an ASN.1 UTF8String to the context buffer using ASN.1 value
 * notation. ASN.1 value notation uses two double-quote characters for each
 * double-quote character in the string.
 *
 * @param pctxt        Pointer to context block structure.
 * @param str          Value to be encoded.
 * @return             Completion status of operation:
 *                       - 0 = success,
 *                       - negative return value is error.
 */
EXTERNRT int rtAvnWriteUTF8Str(OSCTXT* pctxt, const OSUTF8CHAR* str);

/**
* @}
*/

#ifdef __cplusplus
}
#endif

#endif
