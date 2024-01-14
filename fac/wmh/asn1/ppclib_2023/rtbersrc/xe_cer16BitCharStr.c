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

int xe_cer16BitCharStr (OSCTXT* pctxt, Asn116BitCharString* object_p,
                     ASN1TagType tagging, ASN1TAG tag)
{
   /* At most 500 characters/1000 octets per fragment. */
   OSOCTET lbuf[2];  /* Bytes to encode for the current character. */
   OSBOOL indeflen;  /* TRUE if using constructed form (fragments). */
   OSSIZE fragments; /* Number of fragments to encode OCTET STRING in. */
   OSSIZE fragSize;  /* # of characters for current fragment. */
   int ffl = 0;      /* encoded length of one fragment. */
   int aal = 0;      /* entire encoded length. */

   /* Encode fragments in reverse order. There may possibly be only one. */
   indeflen = object_p->nchars > 500;
   fragments = (object_p->nchars + 500) / 500;
   fragSize = object_p->nchars - 500 * (fragments - 1);

   if (0 == object_p) return LOG_RTERR(pctxt, RTERR_BADVALUE);

   if (indeflen && tagging == ASN1EXPL) {
      /* Encode EOC. */
      const OSOCTET EOC[] = { 0, 0 };
      aal += xe_memcpy(pctxt, EOC, 2);
      if (aal < 0) return LOG_RTERR(pctxt, aal);
   }

   /* Encode each of the fragments, if there are any contents octets. */
   if ( object_p->nchars > 0 )
   {
      OSSIZE idx;       /* Index into value for next character to be encoded. */

      idx = object_p->nchars - 1;
      while (fragments > 0)
      {
         OSSIZE last = idx - fragSize + 1;
         ffl = 0;
         for (;;)
         {
            int ll;

            lbuf[0] = (OSOCTET)(object_p->data[idx] / 256);
            lbuf[1] = (OSOCTET)(object_p->data[idx] % 256);

            ll = xe_memcpy(pctxt, lbuf, 2);
            if (ll >= 0) ffl += ll; else return (ll);

            if (idx == last) { idx--; break; }
            else idx--;
         }

         if (indeflen)
         {
            /* Encode tag & length for fragment.
            When indeflen is true, there are multiple fragments and each
            gets its own tag+len. */
            ffl = xe_tag_len(pctxt, TM_UNIV | TM_PRIM | ASN_ID_OCTSTR, ffl);
            if (ffl < 0) return LOG_RTERR(pctxt, ffl);
         }

         aal += ffl;
         fragments--;
         fragSize = 500;
      }
   }
   /* else: no contents octets to encode. */

   if (tagging == ASN1EXPL && indeflen)
   {
      /* We encoded multiple TLV fragments which are to be enclosed in
      a construted tag. */
      aal = xe_TagAndIndefLen(pctxt, TM_UNIV | TM_CONS | tag, aal);
   }
   else if (tagging == ASN1EXPL)
   {
      /* Single fragment.  Encode the tag+len for it. */
      aal = xe_tag_len(pctxt, TM_UNIV | TM_PRIM | tag, aal);
   }
   /* else: ASN1IMPL.  We're encoding only the content octets. */

   return aal;
}

