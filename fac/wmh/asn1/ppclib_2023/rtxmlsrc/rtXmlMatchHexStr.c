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

#include "rtxmlsrc/osrtxml.hh"
#include "rtxsrc/rtxCtype.h"

EXTXMLMETHOD int rtXmlMatchHexStr (OSCTXT* pctxt, size_t minLength, size_t maxLength)
{
   const OSUTF8CHAR* inpdata = (const OSUTF8CHAR*) OSRTBUFPTR (pctxt);
   size_t i, encLen = pctxt->buffer.size, nbits = 0, nbytes;

   for (i = 0; i < encLen && inpdata[i] != 0; i++) {
      char c = (char)inpdata[i];
      if (OS_ISSPACE (c)) continue;
      if (!(c >= '0' && c <= '9') &&
          !(c >= 'A' && c <= 'F') &&
          !(c >= 'a' && c <= 'f'))
         return RTERR_INVHEXS;
      nbits += 4;
   }
   nbytes = (nbits + 7)/8;
   if (minLength > nbytes) return RTERR_CONSVIO;
   if (maxLength > 0 && nbytes > maxLength) return RTERR_CONSVIO;
   return (0);
}


