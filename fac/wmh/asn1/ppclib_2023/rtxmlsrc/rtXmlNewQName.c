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

EXTXMLMETHOD OSUTF8CHAR* rtXmlNewQName
(OSCTXT* pctxt, const OSUTF8CHAR* localName, const OSUTF8CHAR* prefix)
{
   OSUTF8CHAR* qname;
   size_t qnameLen;

   if (OS_ISEMPTY (localName)) {
      LOG_RTERRNEW (pctxt, RTERR_INVPARAM);
      return 0;
   }

   /* Calculate qname length */
   qnameLen = (!OS_ISEMPTY (prefix)) ? OSUTF8LEN(prefix) + 1 : 0;
   qnameLen += OSUTF8LEN (localName);

   /* Allocate memory for name */
   qname = (OSUTF8CHAR*) rtxMemAlloc (pctxt, qnameLen + 1);

   /* Copy the parts to the name */
   if (0 != qname) {
      if (!OS_ISEMPTY(prefix)) {
         rtxUTF8Strcpy (qname, qnameLen + 1, prefix);
         rtxStrcat ((char*)qname, qnameLen + 1, ":");
      }
      else *qname = '\0';

      rtxStrcat ((char*)qname, qnameLen + 1, (const char*)localName);
   }

   return qname;
}


