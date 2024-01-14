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

#include "rtxsrc/rtxErrCodes.h"
#include "rtxmlsrc/osrtxml.hh"

/* Encode UTF-8 string attribute */

EXTXMLMETHOD int rtXmlEncUTF8Attr (OSCTXT* pctxt,
                      const OSUTF8CHAR* name,
                      const OSUTF8CHAR* value)
{
   size_t nameLen  = rtxUTF8LenBytes (name);
   size_t valueLen = rtxUTF8LenBytes (value);

   return rtXmlEncUTF8Attr2 (pctxt, name, nameLen, value, valueLen);
}

/* This version of the function is more efficient because it does       */
/* not need to calculate the length of the attribute name and value.    */
/* The compiler can invoke it directly in places where it can calculate */
/* the lengths at compile time..                                        */

EXTXMLMETHOD int rtXmlEncUTF8Attr2 (OSCTXT* pctxt,
                       const OSUTF8CHAR* name, size_t nameLen,
                       const OSUTF8CHAR* value, size_t valueLen)
{
   int stat;
   OSOCTET oldState = pctxt->state;
   pctxt->state = OSXMLATTR;
   /* Copy data to buffer */

   stat = rtXmlPutChar (pctxt, ' ');
   stat = rtXmlWriteChars (pctxt, name, nameLen);
   stat = rtXmlPutChar (pctxt, '=');
   stat = rtXmlPutChar (pctxt, '\"');

   stat = rtXmlEncStringValue2 (pctxt, value, valueLen);
   if (0 != stat) {
      pctxt->state = oldState;
      return LOG_RTERR (pctxt, stat);
   }

   stat = rtXmlPutChar (pctxt, '\"');

   if (!rtxCtxtTestFlag(pctxt, OSASN1XER) && rtxCtxtTestFlag (pctxt, OSXMLC14N))
   {
      stat = rtXmlEncAttrC14N (pctxt);
      if (0 != stat) {
         pctxt->state = oldState;
         return LOG_RTERR (pctxt, stat);
      }
   }
   pctxt->state = oldState;
   return 0;
}

