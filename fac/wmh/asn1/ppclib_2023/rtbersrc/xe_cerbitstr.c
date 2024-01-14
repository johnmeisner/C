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

int xe_cerBitstr (OSCTXT* pctxt,
               const OSOCTET* object_p,
               OSSIZE numbits,
               ASN1TagType tagging)
{
   register int	aal = 0;    /* entire encoded length */
   register int ll;           /* encoded length of one f'n call. */
   OSOCTET	b;                /* An octet of the encoding. */

   if (numbits == 0)
   {
      /* Zero length bit string is simple case. Contents octets are just the
         unused bits octet, which is zero. */
      b = 0;
      ll = xe_memcpy(pctxt, &b, 1);
      if (ll >= 0) aal += ll;
      else return LOG_RTERR(pctxt, ll);

      if ( tagging == ASN1EXPL )
         aal = xe_tag_len(pctxt, TM_UNIV | TM_PRIM | ASN_ID_BITSTR, aal);
   }
   else {
      /* Encode fragments in reverse order. There may possibly be only one. */
      register int ffl = 0;      /* encoded length of one fragment. */

      OSBOOL indeflen;  /* TRUE if using constructed form (fragments). */
      OSSIZE numbytes;  /* Size of BIT STRING in bytes. */
      OSSIZE fragments; /* Number of fragments to encode BIT STRING in. */
      OSSIZE lastBytes; /* Number of bytes of BIT STRING in last fragment. */
      OSSIZE lastBits;  /* Number of bits in last byte of last fragment. */
      OSSIZE idx;       /* Index into value for start of data going into the
                           next fragment to be encoded. */

      /* 999 * 8 == 7992 is the maximum number of bits that may be encoded in
      a single fragment; BIT STRINGS no more than 7992 bits are encoded in
      primitive form. */
      indeflen = numbits > 7992;

      numbytes = (numbits + 7) / 8;
      fragments = (numbytes + 998) / 999;
      lastBytes = numbytes - 999 * (fragments - 1);
      idx = numbytes - lastBytes;
      lastBits = numbits % 8;

      if (indeflen && tagging == ASN1EXPL) {
         /* Encode EOC. */
         const OSOCTET EOC[] = { 0, 0 };
         aal += xe_memcpy(pctxt, EOC, 2);
         if (aal < 0) return LOG_RTERR(pctxt, aal);
      }

      /* Encode the last byte; it must have any unused bits set to zero. */
      b = object_p[numbytes - 1];
      if ( lastBits == 0 ) {
         lastBits = 8;
      }
      else {
         /* Clear the low 8 - lastBits bits of b. */
         b = b & (OSOCTET) (0xFF << (8 - lastBits));
      }

      ll = xe_memcpy(pctxt, &b, 1);     /* the last byte */
      if (ll >= 0) ffl += ll;
      else return LOG_RTERR(pctxt, ll);

      if (lastBytes > 1)
      {
         /* Encode rest of the octets for the last segment. */
         ll = xe_memcpy(pctxt, object_p + idx, lastBytes - 1);
         if (ll >= 0) ffl += ll;
         else return LOG_RTERR(pctxt, ll);
      }

      /* Enocde the unused bits octet */
      b = (OSOCTET) (8 - lastBits);
      ll = xe_memcpy(pctxt, &b, 1);
      if (ll >= 0) ffl += ll;
      else return LOG_RTERR(pctxt, ll);


      if (indeflen || tagging == ASN1EXPL)
      {
         /* Encode tag & length for fragment. */
         ffl = xe_tag_len(pctxt, TM_UNIV | TM_PRIM | ASN_ID_BITSTR, ffl);
         if (ffl < 0) return LOG_RTERR(pctxt, ffl);
      }

      aal += ffl;
      idx -= 999;
      fragments--;

      /* If there are more fragments, encode them. Each will be 999 bytes and
         will have unused bits of 0.  This will only happen if indeflen is true.
      */
      while (fragments > 0)
      {
         ffl = 0;

         ll = xe_memcpy(pctxt, object_p + idx, 999);  /* bytes for fragment */
         if ( ll >= 0 ) ffl += ll;
         else return LOG_RTERR(pctxt, ll);

         b = 0;
         ll = xe_memcpy(pctxt, &b, 1);   /* unused bits octet */
         if (ll >= 0) ffl += ll;
         else return LOG_RTERR(pctxt, ll);

         /* Encode tag & length for fragment. */
         ffl = xe_tag_len(pctxt, TM_UNIV | TM_PRIM | ASN_ID_BITSTR, ffl);
         if ( ffl < 0 ) return LOG_RTERR(pctxt, ffl);
         aal += ffl;

         idx -= 999;
         fragments--;
      }

      if (tagging == ASN1EXPL && indeflen)
      {
         /* We encoded multiple TLV fragments which are to be enclosed in
            a construted tag. */
         aal = xe_TagAndIndefLen(pctxt, TM_UNIV | TM_CONS | ASN_ID_BITSTR, aal);
      }
      /* else: ASN1IMPL or !indeflen.  Either we're encoding only the
         content octets (ASN1IMPL) or we're encoding a primitive TLV and
         have already encoded the tag and len.
      */
   }

   LCHECKBER(pctxt);

   return (aal);
}

