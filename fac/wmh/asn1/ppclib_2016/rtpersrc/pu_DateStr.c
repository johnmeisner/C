/*
 * Copyright (c) 1997-2018 Objective Systems, Inc.
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

#include "rtpersrc/pd_common.hh"
#include "rtxsrc/rtxCharStr.h"

int pu_GetDateTimeStrSize (OSUINT32 flags) {
   int strSz = 0;

   if (flags & (OSCENTURY|OSYEAR|OSMONTH|OSWEEK|OSDAY)) {
      if (flags & OSANY_MASK)
         strSz += 11; /* max OSINT32 digits + sign */
      else if (flags & OSCENTURY)
         strSz += 3;  /* ccC */
      else if (flags & OSYEAR)
         strSz += 4;  /* YYYY */

      if (flags & OSMONTH) {
         strSz += 3;  /* -MM */

         if (flags & OSDAY)
            strSz += 3; /* -DD */
      }
      else if (flags & OSWEEK) {
         strSz += 4;  /* -Www */

         if (flags & OSDAY)
            strSz += 2; /* -D */
      }
      else if (flags & OSDAY)
         strSz += 4; /* -DDD */
   }

   if (flags & (OSHOURS|OSMINUTES|OSSECONDS)) {
      if (strSz > 0) strSz++; /* T */

      if (flags & OSHOURS)
         strSz += 2; /* HH */

      if (flags & OSMINUTES)
         strSz += 3; /* :MM */

      if (flags & OSSECONDS)
         strSz += 3; /* :SS */

      if (flags & OSFRACTION) {
         strSz += flags & OSFRACTION; /* .fn */
         strSz++; /* account for . */
      }

      if (flags & OSUTC)
         strSz++; /* Z */

      if (flags & OSDIFF)
         strSz += 6; /* sHH:MM */
   }

   return strSz;
}
