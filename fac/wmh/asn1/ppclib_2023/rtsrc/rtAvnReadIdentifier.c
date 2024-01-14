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

#include "rtxsrc/rtxBuffer.h"
#include "rtxsrc/rtxCtype.h"
#include "rtxsrc/rtxError.h"
#include "rtxsrc/rtxMemBuf.h"
#include "rtxsrc/rtxText.h"
#include "rtAvn.h"


EXTRTMETHOD int rtAvnReadIdentifier(OSCTXT* pctxt, char** ppvalue)
{
   OSRTMEMBUF membuf;
   int ret;

   rtxMemBufInit(pctxt, &membuf, 30);

   ret = rtAvnReadIdentifierToBuffer(pctxt, &membuf);
   if (0 != ret) {
      rtxMemBufFree(&membuf);
      return LOG_RTERR(pctxt, ret);
   }

   *ppvalue = (char*)OSMEMBUFPTR(&membuf);

   return 0;
}

EXTRTMETHOD int rtAvnReadIdentifierToBuffer(OSCTXT* pctxt, OSRTMEMBUF* pmembuf)
{
   int ret;
   OSOCTET ub;
   char c;

   /* Peek character, skipping whitespace.  If not a-z, we don't have an
      identifier.
   */
   c = rtxTxtPeekChar2(pctxt, TRUE);
   if (c < 'a' && c > 'z')
   {
      /* Not an identifier. */
      rtxErrAddStrParm(pctxt, "identifier");
      rtxErrAddStrnParm(pctxt, &c, 1);
      return LOG_RTERR(pctxt, RTERR_IDNOTFOU);
   }

   for (;;)
   {
      /* Read character and add it to buffer. */
      ret = rtxReadBytes(pctxt, &ub, 1);
      if (0 != ret) return LOG_RTERR(pctxt, ret);

      ret = rtxMemBufAppend(pmembuf, &ub, 1);
      if (0 != ret) return LOG_RTERR(pctxt, ret);

      /* Peek at next character and break loop if not part of identifier. */
      c = rtxTxtPeekChar2(pctxt, FALSE);
      if (!OS_ISALPHA(c) && !OS_ISDIGIT(c) && c != '-') break;
   }

   /* Null-terminate string */
   ub = '\0';
   ret = rtxMemBufAppend(pmembuf, &ub, 1);
   if (0 != ret) return LOG_RTERR(pctxt, ret);

   return 0;
}
