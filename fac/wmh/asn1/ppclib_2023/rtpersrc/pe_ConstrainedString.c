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

#include "rtpersrc/pe_common.hh"

/***********************************************************************
 *
 *  Routine name: pe_ConstrainedString
 *
 *  Description:  The following function encodes a constrained
 *                string value.
 *
 *  Inputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  ctxt        struct* pointer to PER context block structure
 *  data        char*   pointer to null-term string to be encoded
 *
 *  Outputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  stat        int     completion status of encode operation
 *
 **********************************************************************/

EXTPERMETHOD int pe_ConstrainedString
(OSCTXT* pctxt, const char* string, Asn1CharSet* pCharSet)
{
   if (0 == pCharSet)
      return LOG_RTERR (pctxt, RTERR_INVPARAM);
   else {
#ifndef ASN1UPER
      return pe_ConstrainedStringEx (pctxt, string, pCharSet->charSet.data,
                                     pCharSet->charSetAlignedBits,
                                     pCharSet->charSetUnalignedBits,
                                     pCharSet->canonicalSetBits);
#else
      return uperEncConstrString (pctxt, string, pCharSet->charSet.data,
                                  pCharSet->charSetUnalignedBits,
                                  pCharSet->canonicalSetBits);
#endif
   }
}

EXTPERMETHOD int pe_ConstrainedStringData
(OSCTXT* pctxt, const char* string, const char* charSet,
 OSSIZE nbits, OSSIZE canSetBits, OSSIZE len)
{
   OSSIZE i;
   int stat;

   /* Note: the logic here is not entirely correct, but it is unlikely it will
      ever be a problem.
      Suppose we're working with unaligned PER, and we have IA5String with
      a permitted alphabet of codepoints 1..33.  Then nbits = 6, canSetBits = 7,
      and the largest character is 33, which fits in 6 bits.  The string
      ought to be encoded without translation, but we'll encode with
      translation (e.g. the character with value 1 is encoded as 0).

      I believe all the cases where we would do the wrong thing involve a
      permitted alphabet that contains at least one control character (<32
      decimal), which seems like an unlikely case.

      The decision on whether to use translation or not should only depend
      on nbits and whether the largest character in the alphabet can be
      encoded as-is in that many bits.  canSetBits should not be a part of the
      decision.
   */
   if (nbits >= canSetBits && canSetBits > 4) {
      for (i = 0; i < len; i++) {
         if ((stat = pe_bits (pctxt, string[i], nbits)) != 0)
            return LOG_RTERR (pctxt, stat);
      }
   }
   else if (0 != charSet) {
      OSSIZE nchars = OSCRTLSTRLEN(charSet), pos;
      char* ptr;
      for (i = 0; i < len; i++) {
         ptr = (char*) memchr (charSet, string[i], nchars);

         /* If character not in the defined set, return a constraint    */
         /* violation error..                                           */

         if (0 == ptr) {
            char badChar[2];
            const char *elemName = (const char *) rtxCtxtPeekElemName (pctxt);
            if (0 == elemName) elemName = "?";
            rtxErrAddStrParm (pctxt, elemName);

            badChar[0] = string[i];
            badChar[1] = '\0';
            rtxErrAddStrParm (pctxt, badChar);

            return LOG_RTERR (pctxt, RTERR_CONSVIO);
         }
         else
            pos = ptr - charSet;

         if ((stat = pe_bits (pctxt, (OSUINT32)pos, nbits)) != 0)
            return LOG_RTERR (pctxt, stat);
      }
   }
   else return LOG_RTERR (pctxt, RTERR_INVPARAM);

   return 0;
}

#ifndef ASN1UPER
EXTPERMETHOD
int pe_ConstrainedStringEx (OSCTXT* pctxt,
                            const char* string,
                            const char* charSet,
                            OSUINT32 abits,  /* aligned char bits */
                            OSUINT32 ubits,  /* unaligned char bits */
                            OSUINT32 canSetBits)
{
   OSSIZE len = (0 != string) ? OSCRTLSTRLEN(string) : 0;
   OSUINT32 nbits = pctxt->buffer.aligned ? abits : ubits;
   int      stat;
   /* note: need to save size constraint for use in pu_alignCharStr     */
   /* because it will be cleared in pe_Length from the context..        */
   Asn1SizeCnst sizeCnst = ACINFO(pctxt)->sizeConstraint;

   /* Encode length */

   stat = pe_Length (pctxt, (OSUINT32)len);
   if (stat < 0) return LOG_RTERR (pctxt, stat);

   /* Byte align */

   PU_NEWFIELD (pctxt, "data");

   if (pu_alignCharStr (pctxt, (OSUINT32)len, nbits, &sizeCnst)) {
      stat = pe_byte_align (pctxt);
      if (stat != 0) return LOG_RTERR (pctxt, stat);
   }

   /* Encode data */

   stat = pe_ConstrainedStringData
      (pctxt, string, charSet, nbits, canSetBits, len);
   if (stat != 0) return LOG_RTERR (pctxt, stat);

   PU_SETBITCOUNT (pctxt);

   return 0;
}
#endif

EXTPERMETHOD
int uperEncConstrString (OSCTXT* pctxt,
                         const char* string,
                         const char* charSet,
                         OSUINT32 nbits,
                         OSUINT32 canSetBits)
{
   size_t len = (0 != string) ? OSCRTLSTRLEN(string) : 0;
   int    stat;

   /* Encode length */

   stat = pe_Length (pctxt, (OSUINT32)len);
   if (stat < 0) return LOG_RTERR (pctxt, stat);

   /* Encode data */

   PU_NEWFIELD (pctxt, "data");

   stat = pe_ConstrainedStringData
      (pctxt, string, charSet, nbits, canSetBits, len);
   if (stat != 0) return LOG_RTERR (pctxt, stat);

   PU_SETBITCOUNT (pctxt);

   return 0;
}
