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
#include <stdio.h>
#include <string.h>
#include "rtxsrc/osMacros.h"
#include "rtxsrc/rtxCharStr.h"
#include "rtxsrc/rtxDateTime.hh"

int rtxTimeZoneToString
   (const OSNumDateTime* pvalue, OSUTF8CHAR* buffer, size_t bufsize)
{
   int stat = 0;
   char* const str = (char*)buffer;

   if (bufsize < 7) return RTERR_STROVFLW;

   *str = 0;

   if (pvalue->tz_flag) {
      /* add "-/+??:??" if tzo is out-of-range */
      OSINT32 tzo = pvalue->tzo; /* -840 <= tzo <= 840 */
      if (tzo < 0) {
         rtxStrcat (str, bufsize, "-");
         tzo = -tzo;
      }
      else if (tzo > 0)
         rtxStrcat (str, bufsize, "+");

      if (tzo < -840 || tzo > 840) {
         rtxStrcat (str, bufsize, "??:??");
         stat = RTERR_INVFORMAT;
      }
      else if (tzo == 0) {
         rtxStrcat (str, bufsize, "Z");
      }
      else {
         char tmpbuf[3];
         rtxIntToCharStr (tzo/60, tmpbuf, sizeof(tmpbuf), '0');
         rtxStrcat (str, bufsize, tmpbuf);
         rtxStrcat (str, bufsize, ":");
         rtxIntToCharStr (tzo%60, tmpbuf, sizeof(tmpbuf), '0');
         rtxStrcat (str, bufsize, tmpbuf);
      }
   }

   return (stat == 0) ? (int)OSCRTLSTRLEN ((const char*)buffer) : stat;
}
