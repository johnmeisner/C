/*
 * Copyright (c) 2013-2018 Objective Systems, Inc.
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
 * @file rtx3GPP.h
 * Functions for implementing various algorithms described in various
 * 3GPP standards documents.
 */
#ifndef _RTX3GPP_H_
#define _RTX3GPP_H_

#include "rtxsrc/rtxContext.h"
#include "rtxsrc/rtxTBCD.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SHIFT_IEI 9

/**
 * This structure represents the actions in OS3GPPAltDecodeTabEntry (which see).
 * The action can be a goto action, indicating the row in the decode table
 * to move to, or an accept action, indicating the choiceId that has been
 * matched.
 */
typedef struct OS3GPPAltDecodeAction {
   /*
   gotoEntry values: -1 : error: unknown alternative
                      0 : accept; choiceId is the matched determinant
                     >0 : the entry (row) to move to
   */
   int gotoEntry;
   OSUINT32 choiceId;  /* if gotoEntry == 0, the determinant */
} OS3GPPAltDecodeAction;

/**
 * This is a structure for representing a row in a table used to drive
 * decoding of alternative determinants where the determinants are not all of
 * the same length.
 * The table represents a binary tree, with each row representing a node.  Each
 * row has a flag indicating whether the next bit decoded should be understood
 * as a zero/one bit or as an L/H bit.  Each row also has two actions, one to
 * be followed when a 0/L bit is decoded and the other when a 1/H bit is
 * decoded.  Decoding begins at the root (the first entry) and progresses
 * through the tree with each bit read from the input, until the determinant
 * is determined.
 */
typedef struct OS3GPPAltDecodeTabEntry {
   OSBOOL lhflag;  /* true if an L/H bit should be read at this point */
   OS3GPPAltDecodeAction zeroLAction;  /* action to take if 0/L decoded */
   OS3GPPAltDecodeAction oneHAction;   /* action to take if 1/H decoded */
} OS3GPPAltDecodeTabEntry;



/**
 * Decode a variable length alternative determinant according to the given
 * table.
 *
 * @param pctxt Pointer to the context.  The buffer contains the input.
 * @param detmnt Pointer to value to receive the decoded alternative
 *                determinant.
 * @param table   Pointer to the first entry of the decode table.
 * @param tableSize  Number or entries in the table.
 * @return             Completion status of operation:
 *                       - 0 (0) = success,
 *                       - negative return value is error.
 */
EXTERNRT int rtx3GPPDecVarAltDetmnt(OSCTXT* pctxt, OSUINT32* detmnt,
   OS3GPPAltDecodeTabEntry* table, size_t tableSize);


/**
 * This function converts a value from one of the bit range formats defined
 * in 3GPP TS 44.018 sections 10.5.2.13.3 through 10.5.2.13.6 into an
 * unsigned integer value.
 *
 * @param rfarray      Array of range format values.
 * @param frcount      Number of items in array on which conversion is
 *                       to be done.
 * @param range        Range value (1024, 512, 256, 128).
 * @param presult      Pointer to unsigned integer value to receive
 *                       result of the conversion.
 * @return             Completion status of operation:
 *                       - 0 (0) = success,
 *                       - negative return value is error.
 */
EXTERNRT int rtx3GPPFromRangeFormat
(OSUINT32 rfarray[], OSUINT32 rfcount, OSUINT32 range, OSUINT32* presult);

/**
 * This function encodes an 8-bit ASCII character string
 * to GSM 7-bit format as specified in 3GPP TS 23.038.
 *
 * @param pctxt        Pointer to context structure
 * @param value        Character string to be encoded
 * @return             Completion status of operation:
 *                       - 0 = success
 *                       - negative return value is error.
 */
EXTERNRT int rtx3GPPEncGSMCharStr (OSCTXT* pctxt, const char* value);

/**
 * This function decodes characters encoded in GSM 7-bit format as
 * specified in 3GPP TS 23.038 to a standard ASCII character array.
 *
 * @param pctxt        Pointer to context structure
 * @param outbuf       Output array to receive decoded characters
 * @param outbufsiz    Size of output buffer
 * @param nchars       Number of characters to be decoded
 * @return             Completion status of operation:
 *                       - 0 = success
 *                       - negative return value is error.
 */
EXTERNRT int rtx3GPPDecGSMCharStr
(OSCTXT* pctxt, char* outbuf, OSSIZE outbufsiz, OSSIZE nchars);

/**
 * This function is used to encode an MCC/MNC pair.
 *
 * @param pctxt        Pointer to context structure.
 * @param mcc          MCC value (3 digits).
 * @param mnc          MNC value (3 digits).
 * @return             Completion status of operation:
 *                       - zero (0) = success,
 *                       - negative return value is error.
 * @ return
 */
EXTERNRT int rtx3GPPEnc_MCC_MNC
(OSCTXT* pctxt, const char* mcc, const char* mnc);

/**
 * This function is used to decode an MCC/MNC pair.
 *
 * @param pctxt        Pointer to context structure.
 * @param mcc          Buffer to receive MCC value (3 digits).
 * @param mnc          Buffer to receive MNC value (3 digits).
 * @return             Completion status of operation:
 *                       - zero (0) = success,
 *                       - negative return value is error.
 * @ return
 */
EXTERNRT int rtx3GPPDec_MCC_MNC (OSCTXT* pctxt, char mcc[4], char mnc[4]);

/**
 * This function decodes the non-imperative part of an L3 message having
 * an empty non-imperative part.  This is not for use with circuit-switched
 * call control messages (3GPP TS 24.008 section 9.3) as it does not handle
 * the SHIFT IE case.
 *
 * This will report an error if a comprehension required IE is found.
 *
 * @param pctxt        Pointer to context structure.
 * @param callCtrl     Boolean indicating this is a call control message.
 * @return             Completion status of operation:
 *                       - zero (0) = success,
 *                       - negative return value is error.
 */
EXTERNRT int rtx3GPPDecL3NonImperative (OSCTXT* pctxt, OSBOOL callCtrl);

/**
 * This function is used to encode different alternatives within the
 * MobileIdentify CHOICE type.  These include the IMSI, IMEI, and
 * IMEISV alternatives.
 *
 * @param pctxt        Pointer to context structure.
 * @param type         Choice type alternative (t) value.
 * @param pstr         Numeric string content of IMSI, IMEI, etc.
 * @return             Completion status of operation:
 *                       - zero (0) = success,
 *                       - negative return value is error.
 */
EXTERNRT int rtx3GPPEncMobileIdentityType1
(OSCTXT* pctxt, OSUINT8 type, const char* pstr);

/**
 * This function is used to decode common data present in different
 * alternatives within the MobileIdentify CHOICE type.  These include
 * the IMSI, IMEI, and IMEISV alternatives.
 *
 * @param pctxt        Pointer to context structure.
 * @param len          Length in bytes of the data to be decoded.
 * @param odd          Odd/even indicator boolean decoded from header.
 * @param digit1       First BCD character decoded from header.
 * @param strbuf       Character array to hold decoded value.  This must
 *                       be at least one byte larger to hold the maximum
 *                       sized string value to account for the null
 *                       terminator character.
 * @param bufsize      Size in bytes of strbuf.
 * @return             Completion status of operation:
 *                       - zero (0) = success,
 *                       - negative return value is error.
 */
EXTERNRT int rtx3GPPDecMobileIdentityType1
(OSCTXT* pctxt, OSUINT8 len, OSBOOL odd, OSUINT8 digit1,
 char* strbuf, OSSIZE bufsize);


/**
 * Enumeration of the formats supported for ARFCN encoding.
 * These correspond to the formats described in 44.018 for the Frequency List
 * IE and in Appendix J.
 */
typedef enum OS3GPP_FREQ_LIST_FMT {
   OS3GPP_FREQ_LIST_FMT_BITMAP0,
   OS3GPP_FREQ_LIST_FMT_RANGE_1024,
   OS3GPP_FREQ_LIST_FMT_RANGE_512,
   OS3GPP_FREQ_LIST_FMT_RANGE_256,
   OS3GPP_FREQ_LIST_FMT_RANGE_128,
   OS3GPP_FREQ_LIST_FMT_VAR_BITMAP
} OS3GPP_FREQ_LIST_FMT;


/**
 * Decode a list of ARFCN (frequency list) that have been encoded as described
 * in 44.018 10.5.2.13 Frequency List and Annex J.
 *
 * @param pctxt         Pointer to context structure.
 * @param format        The format in which the ARFCNs have been encoded.
 * @param data          The encoding of the ARFCNs.  This only includes the
 *                      bits that actually encode the ARFCNs and not the bits
 *                      that indicate the choice of the format.  For example,
 *                      in the Frequency List IE, format range 1024, data begins
 *                      with bit 3 of octet 3.
 * @param pArfcns       Receives a newly allocated array, holding the ARFCN
 *                      values.
 * @param pArfcnCount   Receives the number of arfcns in the arfcns array.
 */
EXTERNRT int rtx3GPPDecARFCN(OSCTXT* pctxt, OS3GPP_FREQ_LIST_FMT format,
               const OSOCTET* data, OSUINT16 numbits, OSUINT16** pArfcns,
               OSUINT16* pArfcnCount);


/**
 * Encode a list of ARFCN (frequency list).  The given ARFCNs will be encoded
 * as described in 44.018 10.5.2.13 Frequency List and Annex J.
 *
 * If a buffer is passed, the encoding must fit into it.  It is an error if
 * bufferSize is not large enough to hold the encoding of the ARFCNs in the
 * given format.  If it is necessary for any unused bits in the buffer to be set
 * to zero, you should zero the buffer before calling this method, if needed.
 *
 * If a buffer is not passed, a buffer will be allocated and returned in
 * pBuffer.  As many bytes as are needed to encode the given list of ARFCNs in
 * the given format will be allocated.
 *
 * Let usedOctets be the actual number of octets used.  Then *pNumbits will have
 * the following value, depending on the format:
 *    format         value of *pNumbits
 *    ------------   ----------------
 *    bitmap0        usedOctets * 8 - 4
 *    range 1024     usedOctets * 8 - 5
 *    range 512      usedOctets * 8 - 7
 *    range 256      usedOctets * 8 - 7
 *    range 128      usedOctets * 8 - 7
 *    var bitmap     usedOctets * 8 - 7
 *
 * These values of *pNumbits are designed so that the resulting bit string will
 * align to a byte boundary given the known offset that the string will begin
 * at in the overall encoding of the containing IE (e.g. to make the entire
 * Frequency List IE be a multiple of 8 bits).  Any of the *pNumbits bits that
 * are not actually needed to encode the given list of ARFCNs in the given
 * format will be set to 0.
 *
 * @param pctxt         Pointer to context structure.
 * @param arfcns        Array of ARFCNs to be encoded. The values must be
 *                      distinct.  Their ordering does not matter.
 * @param arfcnCount    Number of ARFCNs in the array
 * @param format        The format to apply to the encoding.  It is an error if
 *                      the given ARFCNs cannot be encoded in the given format.
 *                      For bitmap0, this means all ARFCNs must be in the range
 *                      1..124.  For range formats 512, 256, and 128, this means
 *                      the ARFCNs have a corresponding minimum range of less
 *                      than or equal to 512, 256, and 128, respectively.
 *
 * @param range         The minimum range of the given arfcns. The minimum range
 *                      R is the smallest value such that there is some value in
 *                      the set, F0, such that every value in the set can be
 *                      expressed as:
 *                         (F0 + N) % 1024   for some N, 0 <= N <= R-1
 *                      rtx3GPPGetARFCNRange can be used to obtain the minimum
 *                      range and a value for F0.
 *                      This argument is not used for format bitmap0.
 *
 * @param f0            An ARFCN from the list serving as F0 for the given
 *                      range. This argument is not used for format bitmap0.
 *
 * @param pBuffer       Pass/receive buffer to encode into.  If *pBuffer is
 *                      null, a buffer is allocated.
 * @param bufferSize    If a buffer is given in *pBuffer, the number of bytes in
 *                      the buffer.  Otherwise, not used.
 * @param pNumbits      Receives the number of bits the ARFCNs are encoded into.
 *                      (Includes F0/orig_arfcn fields.)
 */
EXTERNRT int rtx3GPPEncARFCN(OSCTXT* pctxt, OSUINT16* arfcns,
   OSUINT16 arfcnCount, OS3GPP_FREQ_LIST_FMT format, OSUINT16 range,
   OSUINT16 f0, OSOCTET** pBuffer, OSSIZE bufferSize, OSSIZE* pNumbits);



/**
 * Determines the minimum range for the given list of ARFCNs according to
 * 44.018 Annex F. Each ARFCN must be unique and less than 1024.
 * The range of an empty list of ARFCNs is 0.
 *
 * @param arfcns       Point to array of ARFCNs
 * @param arfcnCount   Number of ARFCNs in the array
 * @param pF0          Receives F0, an ARFCN in arfcns
 * @param range        Receives the minimum range.  All ARFCNs in the array
 *                     are such that:
 *                         arfcn = (F0 + N ) mod 1024 for some N, with
 *                         0 <= N <= range-1
 */
EXTERNRT int rtx3GPPGetARFCNRange(OSUINT16* arfcns,
   OSUINT16 arfcnCount, OSUINT16* pF0, OSUINT16* pRange);

/**
 * This macro is the same as rtx3GPPGetPatternBit, except that it picks out the
 * bit from the given pattern according to the previous bit offset in the
 * context buffer.
 *
 * For example, if the buffer's bit offset is now 7, it was previously 8 (i.e.
 * the most significant bit in the current byte).  In that case, the macro
 * will return the value of the most significant bit in the given pattern.
 * This can be used to get the value of a CSN.1 "L" bit by using pattern 0x2B
 * or an H bit using pattern 0xD4.
 * @param pattern An 8 bit pattern.
 */
#define rtx3GPPGetPatternBitPrev(pctxt,pattern) \
   ((((pattern) << 8 | (pattern)) >> (pctxt)->buffer.bitOffset ) & 1)
// Note: If bitoffset is now 8, we actually need the least sig bit from the
// pattern. We make this work by copying the pattern into the next higher byte,
// so that the 9th bit and the 1st bit are the same.  Otherwise the right shift
// by bitOffset would always give us 0 in the least sig bit.  We could also do
// a conditional on bitOffset (shift right 0 when it is 8).


/**
 * This macro picks out the bit from the given pattern that corresponds to the
 * current bit offset in the context buffer.
 *
 * For example, if the buffer's bit offset is 8 (i.e. the most significant bit),
 * the macro will return the value of the most significant bit in the given
 * pattern.
 *
 * This can be used to get the value of a CSN.1 "L" bit by using pattern 0x2B
 * or an H bit using pattern 0xD4.
 */
#define rtx3GPPGetPatternBit(pctxt,pattern) \
   (((pattern) >> ((pctxt)->buffer.bitOffset -1 )) & 1)


/**
 * Decode an integer value that was encoded as described by rtx3GPPEncIntAsBits.
 *
 * @param pctxt      Pointer to context structure.
 * @param pvalue     Pointer to value to decode into.
 * @param bit        0 or 1.  The bit value to count up to determine the encoded
 *                   value.
 * @return           Completion status of operation:
 *                      - zero (0) = success,
 *                      - negative return value is error.
 */
EXTERNRT int rtx3GPPDecIntAsBits(OSCTXT* pctxt, OSUINT8* pvalue, OSUINT8 bit);


/**
 * Decode an integer value that was encoded as described by
 * rtx3GPPEncIntAsBitsPattern.
 *
 * @param pctxt      Pointer to context structure.
 * @param pvalue     Pointer to value to decode into.
 * @param pattern    The pattern against which to match bits. Decoded bits are
 *                   compared against a bit in the pattern, namely the bit
 *                   corresponding to the context buffer's bit offset.  The
 *                   decoded value is the number of equal comparisons before
 *                   an unequal comparison.
 * @return           Completion status of operation:
 *                      - zero (0) = success,
 *                      - negative return value is error.
 */
EXTERNRT int rtx3GPPDecIntAsBitsPattern(OSCTXT* pctxt, OSUINT8* pvalue,
                                          OSUINT8 pattern);


/**
 * Encode given value by encoding that many bits, followed by the negation of
 * bit.
 * @param pctxt      Pointer to context structure.
 * @param value      The value to encode.
 * @param bit        0 or 1.  The bit value to encode as many times as given by
 *                   value.
 * @return           Completion status of operation:
 *                      - zero (0) = success,
 *                      - negative return value is error.
 */
EXTERNRT int rtx3GPPEncIntAsBits(OSCTXT* pctxt, OSUINT8 value, OSUINT8 bit);

/**
 * Encode given value by encoding that many pattern bits, followed by a final
 * bit.  The pattern bits are taken from the given pattern, where the bit offset
 * in the context buffer determines the bit used from the pattern.  The final
 * bit is determined in the same way except that the pattern is negated.
 *
 * For example, to encode an integer where the encoding is a number of L bits
 * finished by an H bit, you use 0x2B for the pattern.
 *
 * @param pctxt      Pointer to context structure.
 * @param value      The value to encode.
 * @param pattern    The pattern from which to take bits as many times as given
 *                   by value.  The bits are taken in correspondence to the
 *                   buffer's bit offset.
 * @return           Completion status of operation:
 *                      - zero (0) = success,
 *                      - negative return value is error.
 */
EXTERNRT int rtx3GPPEncIntAsBitsPattern(OSCTXT* pctxt, OSUINT8 value,
                                          OSUINT8 pattern);


/**
 * Decode an alternative determinant value in the given number of bits,
 * translating L/H bits accorrding to the pattern 0x2B and based on the
 * context's buffer's bit offset.  The lhmask specifies which bits are L/H
 * bits and which are simply 0/1 bits.
 *
 * @param pctxt      Pointer to context structure.
 * @param pDetmnt    Pointer to location to receive decoded determinant value.
 *                   The value is decoded into the lowest bits.
 * @param detmntBits The number of bits to decode.
 * @param lhmask     A mask which indicates whether each bit in detmnt is a
 *                   0/1 bit or an L/H bit.  L/H bits are represented in
 *                   detmnt using 0/1, respectively. This is left-aligned so
 *                   that the highest bit in the mask applies to the first
 *                   bit to be encoded.
 * @return           Completion status of operation:
 *                      - zero (0) = success,
 *                      - negative return value is error.
 */
EXTERNRT int rtx3GPPDecAltDetmnt(OSCTXT* pctxt, OSUINT32* pDetmnt,
   size_t detmntBits, OSUINT32 lhmask);


/**
 * Encode the given alternative determinant value in the given number of bits,
 * translating L/H bits accorrding to the pattern 0x2B and based on the
 * context's buffer's bit offset.  The lhmask specifies which bits are L/H
 * bits and which are simply 0/1 bits.
 *
 * @param pctxt      Pointer to context structure.
 * @param detmnt     The determinant value to encode (in lowest bits)
 * @param detmntBits The number of bits from detmnt to encode.
 * @param lhmask     A mask which indicates whether each bit in detmnt is a
 *                   0/1 bit or an L/H bit.  L/H bits are represented in
 *                   detmnt using 0/1, respectively. This is left-aligned so
 *                   that the highest bit in the mask applies to the first
 *                   bit to be encoded.
 * @return           Completion status of operation:
 *                      - zero (0) = success,
 *                      - negative return value is error.
 */
EXTERNRT int rtx3GPPEncAltDetmnt(OSCTXT* pctxt, OSUINT32 detmnt,
   size_t detmntBits, OSUINT32 lhmask);


/**
 * Function p of 44.018 9.1.54
 */
EXTERNRT int TS44018Msg_p(OSUINT8 n);

/**
 * Function q of 44.018 9.1.54
 */
EXTERNRT int TS44018Msg_q(OSUINT8 n);

/*
Define aliases for use of p and q functions in TS44018_IE.
*/
#define TS44018IE_p TS44018Msg_p
#define TS44018IE_q TS44018Msg_q


#ifdef __cplusplus
}
#endif

#endif
