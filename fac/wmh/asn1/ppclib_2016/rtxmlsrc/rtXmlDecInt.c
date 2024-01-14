/*
 * Copyright (c) 2003-2018 Objective Systems, Inc.
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

#include "rtxmlsrc/osrtxml.hh"
#include "rtxsrc/rtxCtype.h"
#include "rtxsrc/rtxErrCodes.h"
#include "rtxsrc/rtxContext.hh"

static const OSINT32 maxInt32 = 0x7FFFFFFF;
static const OSINT32 maxInt32_10 = 0xCCCCCCC; /* 0x80000000/10 */

EXTXMLMETHOD int rtXmlDecInt (OSCTXT* pctxt, OSINT32* pvalue)
{
   const OSUTF8CHAR* const inpdata = (const OSUTF8CHAR*) OSRTBUFPTR (pctxt);
   size_t i;
   size_t nbytes = OSRTBUFSIZE(pctxt);
   OSBOOL minus = FALSE;
   register OSINT32 value = 0;
   OSINT32 maxval = maxInt32;

   *pvalue = 0;

   /* Consume leading white space */

   for (i = 0; i < nbytes; i++) {
      if (!OS_ISSPACE (inpdata[i])) break;
   }
   if (i == nbytes) return LOG_RTERRNEW (pctxt, RTERR_INVFORMAT);

   /* Consume trailing white space */

   for (; nbytes > 0; nbytes--) {
      if (!OS_ISSPACE (inpdata[nbytes - 1])) break;
   }

   /* Check for '+' or '-' first character */

   if (inpdata[i] == '+') {
      i++;
      if (i >= nbytes) return LOG_RTERRNEW (pctxt, RTERR_INVFORMAT);
   }
   else if (inpdata[i] == '-') {
      minus = TRUE;
      i++;
      maxval++; /* support for 0x80000000 */
      if (i >= nbytes) return LOG_RTERRNEW (pctxt, RTERR_INVFORMAT);
   }

   LCHECKX (pctxt);
   OSRT_CHECK_EVAL_DATE0(pctxt);

   /* Convert Unicode characters to an integer value */

   for ( ; i < nbytes; i++) {
      if (OS_ISDIGIT (inpdata[i])) {
         OSINT32 tm = inpdata[i] - '0';

         if (value > maxInt32_10)
            return LOG_RTERRNEW (pctxt, RTERR_TOOBIG);

         value *= 10;

         if ((OSUINT32)value > (OSUINT32)(maxval - tm))
            return LOG_RTERRNEW (pctxt, RTERR_TOOBIG);

         value += tm;
      }
      else
         return LOG_RTERRNEW (pctxt, RTERR_INVCHAR);
   }

   /* If negative, negate number */

   if (minus) *pvalue = 0 - value;
   else *pvalue = value;

   return (0);
}

EXTXMLMETHOD int rtXmlDecInt8 (OSCTXT* pctxt, OSINT8* pvalue)
{
   OSINT32 itmp;
   int stat;

   stat = rtXmlDecInt (pctxt, &itmp);
   if (stat == 0) {
      if (itmp < -128 || itmp > 127)
         return LOG_RTERRNEW (pctxt, RTERR_TOOBIG);

      *pvalue = (OSINT8)itmp;
   }
   else return LOG_RTERR (pctxt, stat);

   return 0;
}

EXTXMLMETHOD int rtXmlDecInt16 (OSCTXT* pctxt, OSINT16* pvalue)
{
   OSINT32 itmp;
   int stat;

   stat = rtXmlDecInt (pctxt, &itmp);
   if (stat == 0) {
      if (itmp < -32768 || itmp > 32767)
         return LOG_RTERRNEW (pctxt, RTERR_TOOBIG);

      *pvalue = (OSINT16)itmp;
   }
   else return LOG_RTERR (pctxt, stat);

   return 0;
}

