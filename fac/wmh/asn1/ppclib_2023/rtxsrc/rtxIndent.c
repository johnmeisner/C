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

#include "rtxsrc/osMacros.h"
#include "rtxsrc/rtxBuffer.h"
#include "rtxsrc/rtxError.h"
#include "rtxsrc/rtxContext.hh"
#include "rtxsrc/rtxStream.h"

#define OSRTINDENTSPACES 3

size_t rtxGetIndentLevels(OSCTXT* pctxt)
{
   return pctxt->indent / OSRTINDENTSPACES;
}

int rtxWriteIndent(OSCTXT* pctxt)
{
   register int i, stat, nspaces;

   char indentChar = ' ';

   nspaces = pctxt->indent;

   /* Verify indentation whitespace will fit in encode buffer */
   stat = rtxCheckOutputBuffer(pctxt, nspaces + 1);
   if (stat != 0) return LOG_RTERR(pctxt, stat);

   /* Copy data to buffer */

   if (!rtxCtxtTestFlag(pctxt, OSNOWHITESPACE))
   {
      OSRTSAFEPUTCHAR(pctxt, '\n');

      for (i = 0; i < nspaces; i++) {
         OSRTSAFEPUTCHAR(pctxt, indentChar);
      }
   }


   return 0;
}


void rtxIndentDecr(OSCTXT* pctxt)
{
   if (pctxt->indent > 0) pctxt->indent -= OSRTINDENTSPACES;
}

void rtxIndentIncr(OSCTXT* pctxt)
{
   pctxt->indent += OSRTINDENTSPACES;
}

void rtxIndentReset(OSCTXT* pctxt)
{
   pctxt->indent = 0;
}
