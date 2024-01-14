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
#include "rtxsrc/rtxBuffer.h"
#include "rtxsrc/rtxError.h"
#include "rtjsonsrc/osrtjson.h"

int rtJsonEncStringValue2
(OSCTXT* pctxt, const OSUTF8CHAR* value, size_t valueLen)
{
   OSRTSAFEPUTCHAR (pctxt, '"');
   if (0 != value && 0 != *value) {
      size_t i;

      /* If encoding JSON attribute and first character of name if not
         '@', add it now.. */
      if (rtxCtxtTestFlag (pctxt, OSJSONATTR)) {
         if (value[0] != '@') {
            OSRTSAFEPUTCHAR (pctxt, '@');
         }
         rtxCtxtClearFlag (pctxt, OSJSONATTR);
      }

      /* Encode string content */

      for (i = 0; i < valueLen; i++) {
         switch (value[i]) {
         case '"':
         case '\\':
            OSRTSAFEPUTCHAR (pctxt, '\\');
            OSRTSAFEPUTCHAR (pctxt, value[i]);
            break;
         case '\b':
            OSRTSAFEPUTCHAR (pctxt, '\\');
            OSRTSAFEPUTCHAR (pctxt, 'b');
            break;
         case '\f':
            OSRTSAFEPUTCHAR (pctxt, '\\');
            OSRTSAFEPUTCHAR (pctxt, 'f');
            break;
         case '\n':
            OSRTSAFEPUTCHAR (pctxt, '\\');
            OSRTSAFEPUTCHAR (pctxt, 'n');
            break;
         case '\r':
            OSRTSAFEPUTCHAR (pctxt, '\\');
            OSRTSAFEPUTCHAR (pctxt, 'r');
            break;
         case '\t':
            OSRTSAFEPUTCHAR (pctxt, '\\');
            OSRTSAFEPUTCHAR (pctxt, 't');
            break;
         default:
            OSRTSAFEPUTCHAR (pctxt, value[i]);
         }
      }
   }
   OSRTSAFEPUTCHAR (pctxt, '"');

   return 0;
}

int rtJsonEncStringValue (OSCTXT* pctxt, const OSUTF8CHAR* value)
{
   if (0 == value) { value = OSUTF8(""); }
   return rtJsonEncStringValue2 (pctxt, value, OSUTF8LEN(value));
}

int rtJsonEncStringPair2
(OSCTXT* pctxt, const OSUTF8CHAR* name, size_t nameLen,
 const OSUTF8CHAR* value, size_t valueLen)
{
   int stat;
   OSBOOL whitespace = (pctxt->indent > 0);

   stat = rtJsonEncStringValue2 (pctxt, name, nameLen);

   if (0 == stat)
   {
      if (whitespace)
      {
         OSRTSAFEPUTCHAR(pctxt, ' ');
      }
      OSRTSAFEPUTCHAR (pctxt, ':');
      if (whitespace)
      {
         OSRTSAFEPUTCHAR(pctxt, ' ');
      }
   }

   if (0 == stat)
      stat = rtJsonEncStringValue2 (pctxt, value, valueLen);

   return stat;
}

int rtJsonEncStringPair
(OSCTXT* pctxt, const OSUTF8CHAR* name, const OSUTF8CHAR* value)
{
   if (OS_ISEMPTY (name)) return LOG_RTERR (pctxt, RTERR_BADVALUE);
   if (0 == value) { value = OSUTF8(""); }
   return rtJsonEncStringPair2
      (pctxt, name, OSUTF8LEN(name), value, OSUTF8LEN(value));
}

int rtJsonEncStringObject2
(OSCTXT* pctxt, const OSUTF8CHAR* name, size_t nameLen,
 const OSUTF8CHAR* value, size_t valueLen)
{
   int stat;
   OSRTSAFEPUTCHAR (pctxt, '{');

   stat = rtJsonEncStringPair2 (pctxt, name, nameLen, value, valueLen);

   if (0 == stat)
      OSRTSAFEPUTCHAR (pctxt, '}');

   return stat;
}

int rtJsonEncStringObject
(OSCTXT* pctxt, const OSUTF8CHAR* name, const OSUTF8CHAR* value)
{
   if (OS_ISEMPTY (name)) return LOG_RTERR (pctxt, RTERR_BADVALUE);
   if (0 == value) { value = OSUTF8(""); }
   return rtJsonEncStringObject2
      (pctxt, name, OSUTF8LEN(name), value, OSUTF8LEN(value));
}

int rtJsonEncStringNull(OSCTXT* pctxt)
{
   return rtxCopyAsciiText(pctxt, "null");
}

int rtJsonEncStringRaw(OSCTXT* pctxt, const OSUTF8CHAR* value)
{
   if (OS_ISEMPTY(value)) {
      return LOG_RTERR (pctxt, RTERR_BADVALUE);
   }

   return rtxCopyUTF8Text(pctxt, value);
}
