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

#include "asn1xer.h"
#include "rtsrc/asn1intl.h"
#include "xed_common.hh"
#include "xee_common.hh"

/* This function assumes that inpdata is stream of Unicode chars.
 * Characters are already decoded from UTF-8 to Unicode by SAX parser.
 */
int xerDecDynUTF8Str (OSCTXT* pctxt, ASN1UTF8String* outdata)
{
   XMLCHAR* inpdata = (XMLCHAR*) ASN1BUFPTR (pctxt);
   int i;
   size_t nbytes;
   OSOCTET* str;

   /* Count characters */

   nbytes = ASN1SIZEINCHARS (pctxt, XMLCHAR);

   /* Allocate memory for output data */

   str = (OSOCTET*) rtxMemAlloc (pctxt, nbytes + 1);
   if (str != 0) {

      /* Copy UTF-8 source chars to destination */

      for (i = 0; inpdata[i] != 0; i++) {
         str[i] = inpdata[i];
      }
   }
   else
      return LOG_RTERR (pctxt, RTERR_NOMEM);

   str[nbytes] = 0;
   *outdata = (ASN1UTF8String) str;

   return (0);
}

