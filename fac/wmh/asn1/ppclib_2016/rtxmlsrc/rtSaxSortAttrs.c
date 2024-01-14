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

#include "rtxmlsrc/rtSaxDefs.h"
#include "rtxmlsrc/osrtxml.h"

EXTERNXML int rtSaxSortAttrs
(OSCTXT* pctxt, const OSUTF8CHAR* const* attrs, OSUINT16** order)
{
   int nattrs, i, res;
   OSBOOL swapped = TRUE;
   OSUINT16* orderArray;

   /* Get attribute count */

   for (i = 0; 0 != attrs[i]; i += 2);
   nattrs = i/2;

   /* Allocate order array */

   orderArray = rtxMemAllocArray (pctxt, nattrs, OSUINT16);
   if (0 == orderArray) return LOG_RTERR (pctxt, RTERR_NOMEM);

   for (i = 0; i < nattrs; i++)
      orderArray[i] = (OSUINT16)(i*2);

   /* Sort attribute list */

   while (swapped) {
      swapped = FALSE;
      for (i = 0; i < nattrs - 1; i++) {
         res = rtxUTF8Strcmp (attrs[orderArray[i]], attrs[orderArray[i+1]]);
         if (res > 0) {
            OSUINT16 tmp = orderArray[i];
            orderArray[i] = orderArray[i+1];
            orderArray[i+1] = tmp;
            swapped = TRUE;
         }
      }
   }

   *order = orderArray;

   return nattrs;
}

#ifdef TEST
int main ()
{
   OSCTXT ctxt;
   int i, stat;
   OSUINT16* orderArray = 0;
   const OSUTF8CHAR* attrs[] = {
      OSUTF8("d"), OSUTF8("dvalue"),
      OSUTF8("b"), OSUTF8("bvalue"),
      OSUTF8("c"), OSUTF8("cvalue"),
      OSUTF8("a"), OSUTF8("avalue"),
      0
   } ;

   stat = rtxInitContext (&ctxt);
   if (0 != stat) {
      rtxErrPrint (&ctxt);
      return stat;
   }

   stat = rtSaxSortAttrs (&ctxt, attrs, &orderArray);
   if (stat >= 0) {
      printf ("number of attributes = %d\n", stat);
      for (i = 0; i < stat; i++) {
         printf ("%s\n", attrs[orderArray[i]]);
      }
   }
   else {
      rtxErrPrint (&ctxt);
   }

   return stat;
}
#endif
