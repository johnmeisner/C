/*
 * Copyright (c) 2019-2023 Objective Systems, Inc.
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
#include "rtxBuffer.h"
#include "rtxCtype.h"
#include "rtxError.h"
#include "rtxMemory.h"
#include "rtxMemBuf.h"
#include "rtxText.h"



int rtxTxtReadBigInt(OSCTXT* pctxt, char** ppvalue)
{
   OSRTMEMBUF membuf;
   int stat;
   OSSIZE nchars;
   OSOCTET ub;

   rtxTxtSkipWhitespace(pctxt);

   rtxMemBufInit(pctxt, &membuf, 128);

   /* First character must be a +, -, or digit */
   stat = rtxReadBytes(pctxt, &ub, 1);
   if (0 != stat) return LOG_RTERR(pctxt, stat);

   if (ub == '+' || ub == '-') {
      rtxMemBufAppend(&membuf, &ub, 1);

      /* Read next char */
      stat = rtxReadBytes(pctxt, &ub, 1);
      if (0 != stat) return LOG_RTERR(pctxt, stat);
   }

   /* First character after +/- must be a numeric digit */
   if (!OS_ISDIGIT(ub)) return LOG_RTERR(pctxt, RTERR_INVCHAR);

   rtxMemBufAppend(&membuf, &ub, 1);

   /* Read and process numeric digit characters.  End of input is assumed
      to be first non-numeric character */
   for (;;) {
      stat = rtxPeekByte(pctxt, &ub);
      if (stat == 0) break; /* end of input */
      else if (stat < 0) return LOG_RTERR(pctxt, stat);

      if (OS_ISDIGIT(ub)) {
         rtxMemBufAppend(&membuf, &ub, 1);
         pctxt->buffer.byteIndex++; /* move beyond digit */
      }
      else {
         /* Non-digit is end of integer */
         break;
      }
   }

   nchars = OSMEMBUFUSEDSIZE(&membuf);
   *ppvalue = rtxMemAlloc(pctxt, nchars + 1);
   OSCRTLSAFEMEMCPY(*ppvalue, nchars, membuf.buffer, nchars);
   *(*ppvalue + nchars) = 0;
   rtxMemBufFree(&membuf);

   return 0;
}
