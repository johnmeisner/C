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

#include "rtbersrc/asn1ber.h"
#include "rtxsrc/rtxContext.hh"

int xe_cerOctstr (OSCTXT* pctxt,
               const OSOCTET* object_p,
               OSSIZE numocts,
               ASN1TagType tagging)
{
   register int	aal = 0;    /* entire encoded length */
   register int ll;           /* encoded length of one f'n call. */
   register int ffl = 0;      /* encoded length of one fragment. */

   OSBOOL indeflen;  /* TRUE if using constructed form (fragments). */
   OSSIZE fragments; /* Number of fragments to encode OCTET STRING in. */
   OSSIZE idx;       /* Index into value for start of data going into the
                        next fragment to be encoded. */
   OSSIZE fragSize;  /* # of octets for current fragment. */

   /* Encode fragments in reverse order. There may possibly be only one. */
   indeflen = numocts > 1000;
   fragments = (numocts + 1000) / 1000;
   fragSize = numocts - 1000 * (fragments - 1);
   idx = numocts - fragSize;

   if (indeflen && tagging == ASN1EXPL) {
      /* Encode EOC. */
      const OSOCTET EOC[] = { 0, 0 };
      aal += xe_memcpy(pctxt, EOC, 2);
      if (aal < 0) return LOG_RTERR(pctxt, aal);
   }


   /* Encode each of the fragments. */
   while (fragments > 0)
   {
      ffl = 0;

      if (fragSize > 0) {
         /* copy fragment to encode buffer or stream */;
         ll = xe_memcpy(pctxt, object_p + idx, fragSize);
         if ( ll >= 0 ) ffl += ll;
         else return LOG_RTERR(pctxt, ll);
      }

      if ( indeflen || tagging == ASN1EXPL )
      {
         /* Encode tag & length for fragment.
            When indeflen is true, there are multiple fragments and each
            gets its own tag+len. Otherwise, there is a single fragment and
            is gets a tag+len if ASN1EXPL. */
         ffl = xe_tag_len(pctxt, TM_UNIV | TM_PRIM | ASN_ID_OCTSTR, ffl);
         if ( ffl < 0 ) return LOG_RTERR(pctxt, ffl);
      }

      aal += ffl;
      idx -= 1000;
      fragments--;
      fragSize = 1000;
   }

   if (tagging == ASN1EXPL && indeflen)
   {
      /* We encoded multiple TLV fragments which are to be enclosed in
         a construted tag. */
      aal = xe_TagAndIndefLen(pctxt, TM_UNIV | TM_CONS | ASN_ID_OCTSTR, aal);
   }
   /* else: ASN1IMPL or !indeflen.  Either we're encoding only the
      content octets (ASN1IMPL) or we're encoding a primitive TLV and
      have already encoded the tag and len.
   */

   LCHECKBER(pctxt);

   return (aal);
}

