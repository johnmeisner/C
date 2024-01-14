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

#include "rtxmlsrc/osrtxml.h"

EXTXMLMETHOD OSBOOL rtXmlCmpHexStr (OSUINT32 nocts1,
                       const OSOCTET* data1,
                       const OSUTF8CHAR* data2)
{
   OSUINT32 count = 0;
   if (0 != data1 && 0 != data2) {
      while (count < nocts1 && *data2) {
         if (!(rtXmlCmpHexChar(*data2++, (OSOCTET)(*data1 >> 4))))
            break;
         if (!(rtXmlCmpHexChar(*data2++, (OSOCTET)(*data1++ & 0x0f))))
            break;
         ++count;
      }
   } else {
      return FALSE;
   }
   /*  make sure we processed all of data1 and that data2 is finished too */
   if (count != nocts1 || *data2 != '\0')
      return FALSE;
   return TRUE;
}

EXTXMLMETHOD OSBOOL rtXmlCmpHexChar (OSUTF8CHAR ch, OSOCTET hexval) {
      if (ch >= (OSUTF8CHAR)'0' && ch <= (OSUTF8CHAR)'9') {
         if (hexval != ch - (OSUTF8CHAR)'0')
            return FALSE;
      } else if (ch >= (OSUTF8CHAR)'a' && ch <= (OSUTF8CHAR)'f') {
         if (hexval != ch - (OSUTF8CHAR)'a' + 10)
            return FALSE;
      } else if (ch >= (OSUTF8CHAR)'A' && ch <= (OSUTF8CHAR)'F') {
         if (hexval != ch - (OSUTF8CHAR)'A' + 10)
            return FALSE;
      } else {
         return FALSE;
      }
   return TRUE;
}

