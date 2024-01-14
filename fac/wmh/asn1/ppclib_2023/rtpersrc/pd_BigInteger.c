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
#include "rtxsrc/rtxCommonDefs.h"

/***********************************************************************
 *
 *  Routine name: pd_BigInteger
 *
 *  Description:  This function decodes big (> 32 bit) integer using PER.
 *
 *  Inputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  ppvalue     char**  Pointer to value to receive decoded result
 *
 *  Outputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  stat        int     Completion status of operation
 *
 **********************************************************************/

EXTPERMETHOD int pd_BigInteger (OSCTXT *pctxt, const char** ppvalue)
{
   OSUINT32 i, nbytes;
   int bufsiz, stat, ii, off = 2;
   OSBOOL leadingZeros = FALSE;
   char* tmpstr;
   OSUINT32 ub = 0;

   /* Decode length of value */

   stat = pd_Length (pctxt, &nbytes);
   if (stat < 0) return LOG_RTERR (pctxt, stat);

   if (nbytes > MAX_BIGINTBYTES) return LOG_RTERR (pctxt, RTERR_TOOBIG);

   PU_NEWFIELD (pctxt, "BigInteger");

   if (pctxt->buffer.aligned) {
      if ((stat = PD_BYTE_ALIGN (pctxt)) != 0)
         return LOG_RTERR (pctxt, stat);
   }

   /* check is the first byte zero */

   if (nbytes > 0) {
      stat = pd_bits (pctxt, &ub, 8);
      if (stat != 0) return LOG_RTERR (pctxt, stat);

      if (ub == 0) leadingZeros = TRUE;
      nbytes--;
   }

   /* skip all remained leading zeros */

   while (nbytes > 0 && ub == 0) {
      stat = pd_bits (pctxt, &ub, 8);
      if (stat != 0) return LOG_RTERR (pctxt, stat);
      nbytes--;
   }

   /* Allocate memory for character string representation of value */

   bufsiz = ((nbytes + 1) * 2) + 4;
   tmpstr = (char*) rtxMemAlloc (pctxt, bufsiz);
   if (tmpstr == NULL) return LOG_RTERR (pctxt, RTERR_NOMEM);
   tmpstr[0] = '0';
   tmpstr[1] = 'x';

   if (leadingZeros && (ub & 0x80))
      tmpstr[off++] = '0';

   /* process first octet */

   ii = (ub >> 4) & 0x0f;
   NIBBLETOHEXCHAR (ii, tmpstr[off++]);

   ii = (ub & 0x0f);
   NIBBLETOHEXCHAR (ii, tmpstr[off++]);

   /* Process remained octets */

   for (i = 0; i < nbytes; i++) {

      stat = pd_bits (pctxt, &ub, 8);
      if (stat != 0)
         return LOG_RTERR_AND_FREE_MEM (pctxt, stat, tmpstr);

      ii = (ub >> 4) & 0x0f;
      NIBBLETOHEXCHAR (ii, tmpstr[off++]);

      ii = (ub & 0x0f);
      NIBBLETOHEXCHAR (ii, tmpstr[off++]);
   }
   tmpstr[off] = '\0';

   *ppvalue = tmpstr;

   PU_SETBITCOUNT (pctxt);

   return 0;
}
