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

#include "xee_common.hh"

int xerEncAscCharStr (OSCTXT* pctxt,
                      const char* pvalue,
                      const char* elemName)
{
   int  stat;

   /* Element name must be specified because this function could be     */
   /* invoked to handle any of the useful character string types.       */

   if (0 == elemName) return LOG_RTERR (pctxt, RTERR_INVPARAM);

   /* Encode start element */

   stat = xerEncStartElement (pctxt, elemName, 0);
   if (stat != 0) return LOG_RTERR (pctxt, stat);

   pctxt->state = XERDATA;

   /* Loop through all characters and convert special characters into   */
   /* XML escape sequences.                                             */

   stat = xerPutCharStr (pctxt, pvalue, strlen (pvalue));
   if (stat != 0) return LOG_RTERR (pctxt, stat);

   /* Encode end element */

   stat = xerEncEndElement (pctxt, elemName);
   if (stat != 0) return LOG_RTERR (pctxt, stat);

   return 0;
}
