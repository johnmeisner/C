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
 *  Routine name: pu_setCharSet
 *
 *  Description:  This routine merges the given permitted alphabet
 *                with the given canonical character set.
 *
 *  Inputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  charSet     struct  ASN.1 character structure
 *  permSet     char*   Permitted alphabet string
 *
 *  Outputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  charSet     struct  Merged character set
 *
 **********************************************************************/

EXTPERMETHOD void pu_setCharSet (Asn1CharSet* pCharSet, const char* permSet)
{
   OSUINT32 count = 0, i, permSetSize = (OSUINT32)strlen(permSet);

   for (i = 0; i < (OSUINT32)pCharSet->canonicalSetSize; i++) {
      if (memchr (permSet, pCharSet->canonicalSet[i], permSetSize) != NULL)
         pCharSet->charSet.data[count++] = pCharSet->canonicalSet[i];
   }

   pCharSet->charSet.nchars = count;
   pCharSet->charSetUnalignedBits = (count == 0) ? 8 : pu_bitcnt (count);

   pCharSet->charSetAlignedBits = 1;
   while (pCharSet->charSetUnalignedBits > pCharSet->charSetAlignedBits)
      pCharSet->charSetAlignedBits <<= 1;
}
