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

#include "rtxmlsrc/osrtxml.hh"
#include "rtxsrc/rtxCtype.h"

EXTXMLMETHOD void rtXmlTreatWhitespaces (OSCTXT* pctxt, int whiteSpaceType)
{
   OSUTF8CHAR* const str = (OSUTF8CHAR*) OSRTBUFPTR (pctxt);
   size_t nbytes = OSRTBUFSIZE(pctxt), i = 0;

   switch (whiteSpaceType) {
      case OS_WHITESPACE_COLLAPSE: {
         OSUTF8CHAR* destStr = str;

         while (OS_ISSPACE (str[i]) && i < nbytes)
            ++i;
         while (i < nbytes) {
            OSBOOL space = (OSBOOL) OS_ISSPACE (str[i]);

            while (OS_ISSPACE (str[i]) && i < nbytes)
               ++i;
            if (i < nbytes) {
               if (space) *destStr++ = ' ';
               while (!OS_ISSPACE (str[i]) && i < nbytes)
                  *destStr++ = str [i++];
            }
         }
         /* correct the size */
         pctxt->buffer.size = destStr - str;
      } break;
      case OS_WHITESPACE_PRESERVE:
         /* do nothing */
         break;
      case OS_WHITESPACE_REPLACE:
         while (i < nbytes) {
            if (OS_ISSPACE (str[i]))
               str [i] = ' ';
            ++i;
         }

         break;
   }
}


