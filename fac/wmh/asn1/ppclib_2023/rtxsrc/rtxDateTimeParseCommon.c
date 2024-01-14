/*
 * Copyright (c) 2003-2023 Objective Systems, Inc.
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
#include "rtxsrc/rtxDateTime.hh"

int rtxParseTimeZone
(const OSUTF8CHAR* inpdata, size_t inpdatalen, OSNumDateTime* pvalue)
{
   const OSUTF8CHAR *cur = inpdata;
   OSUINT32 tmpi = 0;

   if (*cur != '+' && *cur != '-' && *cur != 'Z') return RTERR_INVFORMAT;
   pvalue->tz_flag = TRUE;

   /* If the character after HH:MM:SS.SS is Z set timezone value to 0 */
   if (*cur == 'Z') {
      pvalue->tzo = 0;
   }
   else {
      int sign = 1, stat;

      if (*cur == '-') sign = -1;

      /* If the char after HH:MM:SS.SS is +/- parse timezone format HH:MM */
      /* Parse timezone HH from next two charcter after +/- */
      cur++;

      if ((size_t)(cur - inpdata + 5) > inpdatalen) return RTERR_INVFORMAT;

      PARSE_2_DIGITS(tmpi, cur, stat);

      /* check the parse status of time zone hour
                    and presence of ':' after HH*/
      if (stat != 0 || *cur != ':') return RTERR_INVFORMAT;
      if (tmpi > 24) return RTERR_BADVALUE;
      pvalue->tzo = 60 * tmpi;

      /* Parse timezone MM from next two charcter after HH: */
      cur++;
      PARSE_2_DIGITS(tmpi, cur, stat);

      /* check the parse status of time zone min */
      if (stat != 0) return RTERR_INVFORMAT;
      if (tmpi > 59) return RTERR_BADVALUE;
      tmpi = pvalue->tzo + tmpi;
      if (tmpi > 840) return RTERR_BADVALUE;
      pvalue->tzo = tmpi;

      /* Set time zone value -ve if the timezone flag contains '-' */
      pvalue->tzo *= sign;
   }
   return (int)(cur-inpdata);
}

