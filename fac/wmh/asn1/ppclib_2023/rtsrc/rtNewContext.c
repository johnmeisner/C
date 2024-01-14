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

/* Run-time utility functions */

#include <stdarg.h>
#include <stdlib.h>
#include "rtsrc/asn1type.h"

/*
 * Default version of rtNewContext.  In limited and evaluation software
 * versions, this function will be overridden by a macro in the rtkey.h
 * file that will force the rtInitContextUsingKey function to be used.
 * If this version is used, the run-time key information will not get
 * set in the context and the run-time will fail at some point.
 */
EXTRTMETHOD OSCTXT* rtNewContext()
{
   return rtNewContextUsingKey (0, 0);
}

EXTRTMETHOD OSCTXT* rtNewContextUsingKey (const OSOCTET* key, size_t keylen)
{
   OSCTXT* pctxt = (OSCTXT*) OSCRTMALLOC0 (sizeof(OSCTXT));
   if (pctxt) {
      if (rtInitContextUsingKey (pctxt, key, keylen) != 0) {
         OSCRTFREE0 (pctxt);
         pctxt = 0;
      } else
         pctxt->flags |= ASN1DYNCTXT;
   }
   return (pctxt);
}
