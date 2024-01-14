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

/* Compares texts. Returns 0 if not equal, or non-zero if texts are the same */
EXTXMLMETHOD OSBOOL rtXmlStrCmpAsc (const OSUTF8CHAR* text1, const char* text2)
{
   if (0 != text1 && 0 != text2) {
      for (; *text1 != 0 && *text2 != 0; text1++, text2++) {
         if (*text1 != (OSUTF8CHAR)*text2)
            return 0;
      }
      return (OSBOOL)(*text1 == *text2);
   }
   return FALSE;
}


