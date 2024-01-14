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

#include "rtxmlsrc/osrtxml.h"
#include "rtxsrc/rtxBigInt.h"
#include "rtxsrc/rtxErrCodes.h"
#include "rtxsrc/rtxCtype.h"

EXTXMLMETHOD int rtXmlDecBigInt (OSCTXT* pctxt, const OSUTF8CHAR** pvalue)
{
   const OSUTF8CHAR* const inpdata = (const OSUTF8CHAR*) OSRTBUFPTR (pctxt);
   size_t i;
   size_t nbytes = OSRTBUFSIZE(pctxt);
   char* pbuf;
   int stat = 0;

   /* Consume leading white space */

   for (i = 0; i < nbytes; i++) {
      if (!OS_ISSPACE (inpdata[i])) break;
   }
   if (i == nbytes) return LOG_RTERRNEW (pctxt, RTERR_INVFORMAT);

   /* Consume trailing white space */

   for (; nbytes > 0; nbytes--) {
      if (!OS_ISSPACE (inpdata[nbytes - 1])) break;
   }

   /* treat first char */

   if (((char) inpdata[i] == '-' && i + 1 >= nbytes) ||
       ((char) inpdata[i] == '+' && i + 1 >= nbytes) ||
       (((char) inpdata[i] != '-' || (char) inpdata[i] != '+') &&
        !OS_ISDIGIT ((char) inpdata[i])))
      return LOG_RTERRNEW (pctxt, RTERR_BADVALUE);

   /* Allocate memory for output data */

   pbuf = (char*)rtxMemAlloc (pctxt, nbytes - i + 1);

   if (pbuf != 0) {
      *pbuf = (char) inpdata[i];

      /* copy characters */
      for (++i; i < nbytes; i++) {
         char c = (char) inpdata[i];
         if (!OS_ISDIGIT (c)) {
            rtxMemFreePtr (pctxt, pbuf);
            return LOG_RTERRNEW (pctxt, RTERR_BADVALUE);
         }
         pbuf[i] = c;
      }

      pbuf[i] = '\0';
   }
   else
      return LOG_RTERRNEW (pctxt, RTERR_NOMEM);

   *pvalue = (const OSUTF8CHAR*)pbuf;
   return (stat);
}


