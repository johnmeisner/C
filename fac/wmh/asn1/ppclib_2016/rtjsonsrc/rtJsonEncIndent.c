/*
 * Copyright (c) 2018-2018 Objective Systems, Inc.
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

#include "rtxsrc/osMacros.h"
#include "rtxsrc/rtxErrCodes.h"
#include "osrtjson.h"

/**
 * This function adds the given amount of indentation to the output
 * stream.
 */
int rtJsonEncIndent (OSCTXT* pctxt)
{
   register int i, stat, nspaces;

   if (pctxt->indent > 0) {
      char indentChar = ' ';

      nspaces = pctxt->level * pctxt->indent;

      /* Verify indentation whitespace will fit in encode buffer */
      stat = rtxCheckOutputBuffer (pctxt, nspaces + 1);
      if (stat != 0) return LOG_RTERR (pctxt, stat);

      /* Copy data to buffer */

      if (pctxt->state != OSJSONNOCOMMA)
      {
         // Add comma to preceding line before setting carriage return and
         //  indenting for current line...
         OSJSONPUTCOMMA(pctxt);
      }
      OSRTSAFEPUTCHAR (pctxt, '\n');

      for (i = 0; i < nspaces; i++) {
         OSRTSAFEPUTCHAR (pctxt, indentChar);
      }
   }
   else // No-whitespace case...
   {
      if (pctxt->state != OSJSONNOCOMMA)
      {
         // Add comma to "preceding" line...
         OSJSONPUTCOMMA(pctxt);
      }
   }
   return 0;
}

