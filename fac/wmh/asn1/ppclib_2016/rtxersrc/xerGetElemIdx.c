/*
 * Copyright (c) 1997-2018 Objective Systems, Inc.
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

#include "asn1xer.h"

/**
 * This function does a lookup of the given element name in the given
 * array.  If it finds the name, it returns the index, otherwise -1 is
 * returned.
 *
 * This version is for SET and CHOICE constructs and will check all
 * element entries for a match.
 */
int xerGetElemId (const XMLCHAR* elemName,
                  XerElemInfo* pElemInfo, int numElems)
{
   int i;
   for (i = 0; i < numElems; i++) {
      if (xerCmpText (elemName, pElemInfo[i].name) == 0)
         return i;
   }
   return -1;
}
