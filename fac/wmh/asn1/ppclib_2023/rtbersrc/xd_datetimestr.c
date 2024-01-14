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


int xd_datetimestr(OSCTXT* pctxt, const char** object_p,
                   ASN1TagType tagging, ASN1TAG tag, int length)
{
   const char *praw;
   char *pdata;
   ASN1Time tval;
   int stat = 0;

   stat = xd_charstr(pctxt, &praw, tagging, tag, (OSSIZE)length);
   if (0 != stat)
   {
      return stat;
   }

   /* Restore stripped chars. Convert:
      YYYYMMDDHHMMSS to YYYY-MM-DDTHH:MM:SS
      01234567890123    0123456789012345678
   */
   if (OSCRTLSTRLEN(praw) != 14) return LOG_RTERRNEW(pctxt, RTERR_INVFORMAT);
   pdata = rtxMemAllocArray(pctxt, 20, char);
   if (pdata == 0) return LOG_RTERRNEW(pctxt, RTERR_NOMEM);
   OSCRTLMEMCPY(pdata, praw, 4);          /* YYYY */
   pdata[4] = '-';
   OSCRTLMEMCPY(pdata + 5, praw + 4, 2);  /* MM */
   pdata[7] = '-';
   OSCRTLMEMCPY(pdata + 8, praw + 6, 2);  /* DD */
   pdata[10] = 'T';
   OSCRTLMEMCPY(pdata + 11, praw + 8, 2);  /* HH */
   pdata[13] = ':';
   OSCRTLMEMCPY(pdata + 14, praw + 10, 2);  /* MM */
   pdata[16] = ':';
   OSCRTLMEMCPY(pdata + 17, praw + 12, 2);  /* SS */
   pdata[19] = 0;

   rtxMemFreePtr(pctxt, praw);

   /* Now validate string. */
   stat = rtParseTime(pctxt, pdata, &tval, TRUE);
   if (stat == ASN_E_NOTCANON) stat = 0;
   else if (stat != 0) LOG_RTERR(pctxt, stat);

   if (stat == 0 &&
      (tval.settings !=
         (ASN1TIMEFIELD_YEAR | ASN1TIMEFIELD_MONTH | ASN1TIMEFIELD_DAY |
                ASN1TIMEFIELD_HOUR | ASN1TIMEFIELD_MIN | ASN1TIMEFIELD_SEC) ||
       tval.val.dateTime.fracDigits != 0 ||
       tval.val.dateTime.year < 1582 ||
       tval.val.dateTime.year > 9999))
   {
      /* Fields not correctly set. */
      stat = LOG_RTERRNEW(pctxt, RTERR_INVFORMAT);
      rtxErrSetNonFatal(pctxt);
   }

   if ((stat != 0) && (stat != RTERR_INVFORMAT))
   {
      rtxMemFreePtr(pctxt, pdata);
   }

   *object_p = pdata;
   return 0;
}
