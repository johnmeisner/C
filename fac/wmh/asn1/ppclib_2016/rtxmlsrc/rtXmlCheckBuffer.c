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

#include <stdlib.h>
#include "rtxmlsrc/osrtxml.h"
#include "rtxsrc/rtxCommonDefs.h"

int rtXmlCheckBuffer (OSCTXT* pctxt, size_t byte_count)
{
   size_t nbytes = 0;
   OSXMLCtxtInfo* pXmlInfo = (OSXMLCtxtInfo*)pctxt->pXMLInfo;

   if (pXmlInfo == 0 ||
       pXmlInfo->encoding == OSXMLUTF8 ||
       pXmlInfo->encoding == OSXMLLATIN1) {
      nbytes = byte_count;
   }
   else if (pXmlInfo->encoding == OSXMLUTF16 ||
            pXmlInfo->encoding == OSXMLUTF16BE ||
            pXmlInfo->encoding == OSXMLUTF16LE) {
      nbytes = byte_count * 2;
   }

   return rtxCheckOutputBuffer(pctxt, nbytes);
}

