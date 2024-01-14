/*
 * Copyright (c) 1997-2023 Objective Systems, Inc.
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

#include "asn1json.h"

int rtJsonAsn1EncOpenType (OSCTXT *pctxt, OSOCTET *pdata, OSSIZE numocts)
{
   int stat;
   OSUTF8CHAR *anyType;
   OSCTXT ctxt;

   if (0 == pdata || numocts == 0) return 0;

   rtInitContext (&ctxt);
   rtxInitContextBuffer (&ctxt, pdata, numocts);

   /* first, try to parse the data as a valid json encoding */
   stat = rtJsonDecAnyType (&ctxt, &anyType);
   if (stat == 0) {
      rtJsonEncStringObject (pctxt, (OSUTF8CHAR*)"value", anyType);
   }
   else {
      return LOG_RTERR (pctxt, stat);
   }

   return 0;
}

/*
Return true if the given data is null-terminated text, as opposed to binary
data.  To be considered null-terminated text, the data must meet the following
criteria:
   1. The data must contain exactly one null byte, at data[nocts-1]
   2. All other characters must be printable ASCII characters. before the first
A printable ASCII character is a character in the range [32,127].

Rationale:
The first condition means that a null terminated character string can't be
stored in an oversized buffer.  But, if we were to allow the null byte to appear
anywhere, then we'd be assuming that all the rest of the data is irrelevant to
deciding whether the data is binary or text, and that seems problematic; for
example, suppose some binary data just happens to begin with a null byte - we
would then assume it was a null-terminated empty character string.
*/
static OSBOOL isNullTermText (const OSOCTET *data, OSSIZE nocts)
{
   OSSIZE i = 0;

   if ( nocts == 0 ) return FALSE;
   if ( data[nocts-1] != 0 ) return FALSE;

   for (i = 0; i < nocts - 1; i ++) {
      if (data[i] < 32 || data[i] > 127) {
         return FALSE;
      }
   }

   return TRUE;
}

/**
 * Same as rtJsonEncExtElem except accepts a suffix to add on to the name
 * of the wrapper element.
 * @param suffix  Ignored if 0.
 */
static int rtJsonEncExtElemWithSuffix (OSCTXT* pctxt, ASN1OpenType* pElem,
                                       OSUINT32 suffix)
{
   int stat = 0;

   if ( isNullTermText(pElem->data, pElem->numocts) )
   {
      /* write data as-is.  It is expected to be a JSONNamedValue. */
      stat = rtJsonEncStringRaw(pctxt, pElem->data);
      if (stat != 0) return LOG_RTERR (pctxt, stat);
   }
   else {
      /*
      determine wrapper name from _ext[_suffix] (e.g. _ext, _ext_1,
      _ext_2, ...).  Then, encode open type data as hex data.
      */
      char name[64];

      if ( suffix ) {
         os_snprintf(name, sizeof(name), "_ext_%d", suffix);
      }
      else {
         os_snprintf(name, sizeof(name), "_ext");
      }

      stat = rtJsonEncStringValue2(pctxt, (OSUTF8CHAR *)name, strlen(name));
      if (stat != 0) return LOG_RTERR (pctxt, stat);

      OSRTSAFEPUTCHAR (pctxt, ':');

      stat = rtJsonEncHexStr(pctxt, pElem->numocts, pElem->data);
      if (stat != 0) return LOG_RTERR (pctxt, stat);
   }

   return stat;
}

int rtJsonAsn1EncExtElem (OSCTXT* pctxt, ASN1OpenType* pElem) {
   return rtJsonEncExtElemWithSuffix(pctxt, pElem, 0);
}


int rtJsonAsn1EncOpenTypeExt (OSCTXT* pctxt, const OSRTDList* pvalue,
                              OSBOOL asArray)
{
   OSUINT32 count = 0;
   ASN1OpenType* pOpenType;
   OSRTDListNode* pnode = pvalue->head;
   OSBOOL bWritten = FALSE;

   while (0 != pnode) {
      count++;

      pOpenType = (ASN1OpenType*)pnode->data;

      if ( pOpenType != NULL ) {
         if ( bWritten ) OSRTSAFEPUTCHAR (pctxt, ',');

         rtJsonEncExtElemWithSuffix(pctxt, pOpenType, count);

         bWritten = TRUE;
      }
      else if ( asArray )
      {
         /* absent unknown extension; must be represented as null value */
         if ( bWritten ) OSRTSAFEPUTCHAR (pctxt, ',');

         rtJsonEncStringNull(pctxt);

         bWritten = TRUE;
      }
      /* else: NULL represents some unknown extenion that was absent */

      pnode = pnode->next;
   }

   return 0;
}
