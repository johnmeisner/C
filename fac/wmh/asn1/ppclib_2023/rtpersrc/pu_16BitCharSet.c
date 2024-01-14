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

#include "rtpersrc/asn1per.h"

/***********************************************************************
 *
 *  Routine name: pu_init16BitCharSet
 *
 *  Description:  This routine initializes a 16-bit character
 *                set structure.
 *
 *  Inputs:
 *
 *  Name        Type                    Description
 *  ----        ----                    -----------
 *  pCharSet    Asn116BitCharSet*       Character set to initialize
 *  first       OSUNICHAR           First character in set
 *  last        OSUNICHAR           Last character in set
 *  abits       OSUINT32                Number of aligned bits per
 *                                      character
 *  ubits       OSUINT32                Number of unaligned bits per
 *                                      character
 *
 *  Outputs:
 *
 *  none
 *
 *
 **********************************************************************/

void pu_init16BitCharSet (Asn116BitCharSet* pCharSet, OSUNICHAR first,
                          OSUNICHAR last, OSUINT32 abits, OSUINT32 ubits)
{
   pCharSet->charSet.nchars = 0;
   pCharSet->charSet.data = 0;
   pCharSet->firstChar = first;
   pCharSet->lastChar  = last;
   pCharSet->unalignedBits = ubits;
   pCharSet->alignedBits = abits;
}

/***********************************************************************
 *
 *  Routine name: pu_set16BitCharSet
 *
 *  Description:  This routine sets a 16-bit character permitted
 *                alphabet character set structure.
 *
 *  Inputs:
 *
 *  Name        Type                    Description
 *  ----        ----                    -----------
 *  pctxt      OSCTXT*               Pointer to ASN.1 context structure
 *                                      (used for mem alloc function calls)
 *  pCharSet    Asn116BitCharSet*       Character set to initialize
 *  pAlphabet   Asn116BitCharSet*       Pointer to permitted alphabet
 *
 *  Outputs:
 *
 *  none
 *
 *
 **********************************************************************/

EXTPERMETHOD int pu_set16BitCharSet
(OSCTXT* pctxt, Asn116BitCharSet* pCharSet, Asn116BitCharSet* pAlphabet)
{
   /* Permitted alphabet range can either be specified as a range of    */
   /* characters or as a discrete set..                                 */

   if (0 != pAlphabet->charSet.data) {
      OSSIZE nocts = pAlphabet->charSet.nchars * 2;
      if (pAlphabet->charSet.nchars > nocts) /* overflow */
         return LOG_RTERR (pctxt, RTERR_TOOBIG);
      pCharSet->charSet.nchars = pAlphabet->charSet.nchars;

      pCharSet->charSet.data = (OSUNICHAR*) rtxMemAlloc (pctxt, nocts);
      if (pCharSet->charSet.data != NULL)
         memcpy (pCharSet->charSet.data, pAlphabet->charSet.data, nocts);
      else
         return LOG_RTERR (pctxt, RTERR_NOMEM);
   }
   else {
      pCharSet->firstChar = pAlphabet->firstChar;
      pCharSet->lastChar  = pAlphabet->lastChar;
      pCharSet->charSet.nchars = pCharSet->lastChar - pCharSet->firstChar;
   }

   if (pCharSet->charSet.nchars > OSUINT32_MAX)
      return LOG_RTERR (pctxt, RTERR_TOOBIG);

   pCharSet->unalignedBits = pu_bitcnt ((OSUINT32)pCharSet->charSet.nchars);

   pCharSet->alignedBits = 1;
   while (pCharSet->unalignedBits > pCharSet->alignedBits)
      pCharSet->alignedBits <<= 1;

   return 0;
}

/***********************************************************************
 *
 *  Routine name: pu_set16BitCharSetFromRange
 *
 *  Description:  This routine sets a 16-bit character permitted
 *                alphabet character set structure given a lower
 *                and upper character range value.
 *
 *  Inputs:
 *
 *  Name        Type     Description
 *  ----        ----     -----------
 *  pCharSet    struct*  Character set to initialize
 *  lower       usint    Lower range value
 *  upper       usint    Upper range value
 *
 *  Outputs:
 *
 *  none
 *
 *
 **********************************************************************/

EXTPERMETHOD void pu_set16BitCharSetFromRange
(Asn116BitCharSet* pCharSet, OSUINT16 firstChar, OSUINT16 lastChar)
{
   memset (pCharSet, 0, sizeof(Asn116BitCharSet));
   pCharSet->firstChar = firstChar;
   pCharSet->lastChar  = lastChar;
   pCharSet->charSet.nchars = pCharSet->lastChar - pCharSet->firstChar;
   pCharSet->unalignedBits = pu_bitcnt ((OSUINT32)pCharSet->charSet.nchars);
   pCharSet->alignedBits = 1;
   while (pCharSet->unalignedBits > pCharSet->alignedBits)
      pCharSet->alignedBits <<= 1;
}
