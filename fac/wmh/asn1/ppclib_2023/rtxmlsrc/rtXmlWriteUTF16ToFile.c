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

#include "rtxmlsrc/osrtxml.h"
#include "rtxsrc/rtxFile.h"

EXTXMLMETHOD int rtXmlWriteUTF16ToFile (OSCTXT* pctxt, const char* filename)
{
   FILE* fp;
   int stat = rtxFileOpen (&fp, filename, "wb");
   if (0 != stat) {
      return LOG_RTERR (pctxt, stat);
   }
   stat = rtXmlEncEndDocument (pctxt);
   if (0 != stat) {
      fclose (fp);
      return LOG_RTERR (pctxt, stat);
   }

   fwrite (rtXmlGetEncBufPtr(pctxt), 1, rtXmlGetEncBufLen(pctxt), fp);
   fclose (fp);

   return 0;
}

