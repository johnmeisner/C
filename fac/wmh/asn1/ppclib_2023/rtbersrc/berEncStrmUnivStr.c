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

#include "rtbersrc/xse_common.hh"

int berEncStrmUnivStr (OSCTXT* pctxt, const Asn132BitCharString* object_p,
                       ASN1TagType tagging)
{
   OSOCTET lbuf[4];
   int stat;
   OSSIZE i, nbytes;

   if (0 == object_p) return LOG_RTERR (pctxt, RTERR_BADVALUE);

   nbytes = object_p->nchars * 4;
   if (object_p->nchars > nbytes) /* overflow */
      return LOG_RTERR (pctxt, RTERR_TOOBIG);

   if (tagging == ASN1EXPL)
      stat = berEncStrmTagAndDefLen
         (pctxt, TM_UNIV|TM_PRIM|ASN_ID_UniversalString, nbytes);
   else
      stat = berEncStrmDefLength (pctxt, nbytes);

   if (stat != 0) return LOG_RTERR (pctxt, stat);

   for (i = 0; i < object_p->nchars; i++) {
      OS32BITCHAR sym = object_p->data[i];
      lbuf[0] = (OSOCTET) (sym / 256 / 256 / 256);
      lbuf[1] = (OSOCTET) (sym / 256 / 256 % 256);
      lbuf[2] = (OSOCTET) (sym / 256 % 256 );
      lbuf[3] = (OSOCTET) (sym % 256);

      stat = berEncStrmWriteOctets (pctxt, lbuf, 4);
      if (stat != 0) return LOG_RTERR (pctxt, stat);
   }

   return 0;
}

