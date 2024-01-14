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

/***********************************************************************
 *
 *  Routine name: xd_indeflen_ex
 *
 *  Description:  This calculates the actual length of an indefinite
 *                message.
 *
 *  Inputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  msgptr      char*   Pointer to indefinite length ASN.1 message.
 *
 *  Outputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  stat	int	Status/length value.  If >= 0, indicates actual
 *                      length of the message; if < 0, error status.
 *
 **********************************************************************/

int xd_indeflen64 (const OSOCTET* msg_p, OSSIZE bufSize, OSSIZE* plength)
{
   OSCTXT  ctxt;
   ASN1TAG tag;
   OSSIZE  len, ilcnt = 0;
   int     stat;
   OSBOOL  indefLen;

   if (0 == plength) return RTERR_NULLPTR;

   if (bufSize <= 4) { /* prim tag + indef-len-marker + 2b EOC */
      *plength = 0;
      return 0;
   }

   if ((stat = rtInitASN1Context (&ctxt)) != 0)
      return (stat);

   do {
      if ((stat = xd_setp64 (&ctxt, msg_p, 0, &tag, &len, &indefLen)) != 0)
         break;

      if (!indefLen) {
         stat = ASN_E_INVLEN;
         break;
      }

      ctxt.buffer.size = bufSize;

      /* Advance decode pointer to end of buffer */

      do {
         stat = xd_tag_len_64 (&ctxt, &tag, &len, &indefLen, XM_ADVANCE);
         if (stat != 0)
            break; /* RTERR_ENDOFBUF is error, as must have matching EOC */
         if (indefLen) ilcnt++;
         else if (len > 0) ctxt.buffer.byteIndex += len; /* skip the content */
         else if (tag == 0 && len == 0) ilcnt--;
      } while (ilcnt > 0);
   } while (0);

   *plength = (stat == 0) ? ctxt.buffer.byteIndex : 0;

   rtxFreeContext (&ctxt);

   return stat;
}

int xd_indeflen_ex (const OSOCTET* msg_p, int bufSize)
{
   OSSIZE len;
   int ret;

   OSRTASSERT (bufSize >= 0);

   ret = xd_indeflen64 (msg_p, (OSSIZE)bufSize, &len);
   if (0 == ret) {
      if (len > OSINT32_MAX) ret = RTERR_TOOBIG;
      else ret = (int) len;
   }

   return ret;
}
