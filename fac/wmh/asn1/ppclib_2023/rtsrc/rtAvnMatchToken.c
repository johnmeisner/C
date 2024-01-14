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

#include "rtAvn.h"
#include "rtxsrc/rtxBuffer.h"
#include "rtxsrc/rtxCtype.h"
#include "rtxsrc/rtxError.h"
#include "rtxsrc/rtxText.h"

EXTRTMETHOD int rtAvnMatchToken(OSCTXT* pctxt, const OSUTF8CHAR* token)
{
   int ret;
   OSOCTET data[2];
   OSSIZE avail;

   ret = rtxTxtMatchChars(pctxt, token, TRUE);
   if (ret < 0 ) return LOG_RTERR(pctxt, ret);

   /* Now just make sure there aren't more characters that belong to the
      identifier.  We need to peek at most 2 bytes to determine this.
   */
   ret = rtxPeekBytes(pctxt, data, sizeof(data), 2, &avail);
   if (ret < 0) return LOG_RTERR(pctxt, ret);

   if (avail == 0) return 0;  /* no more bytes */
   else if (!OS_ISALNUM(data[0]) && data[0] != '-')
      /* Next char is not valid in an identifier or reserved word. */
      return 0;
   else if (data[0] == '-' && avail == 2 && data[1] == '-')
      /* Token is followed by a comment. */
      return 0;
   else {
      /* Next char is valid in an identifier/reserved word and is not the start
         of a comment. */
      rtxErrAddStrParm(pctxt, (const char*) token);
      return LOG_RTERRNEW(pctxt, RTERR_IDNOTFOU);
   }
}
