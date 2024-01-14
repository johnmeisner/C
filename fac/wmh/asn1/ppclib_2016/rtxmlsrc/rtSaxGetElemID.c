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

#ifndef FUNCNAME
#define FUNCNAME rtSaxGetElemID
#define IDXTYPE  OSINT16
#endif

#include "rtxmlsrc/rtSaxDefs.h"
#include "rtxmlsrc/osrtxml.h"

EXTXMLMETHOD OSINT16 FUNCNAME (OSINT16* pState, OSINT16 prevElemIdx,
                  const OSUTF8CHAR* localName,
#ifdef OSXMLNS12
                  OSINT32 nsidx, /* namespace index */
#endif
                  const OSSAXElemTableRec idtab[],
                  const IDXTYPE* fstab,
                  OSINT16 fstabRows, OSINT16 fstabCols)
{
   const OSINT16 curState = *pState;
   OSINT16 openTypeIdx = -1;
   OSBOOL  match;

   if (curState >= 0 && curState < fstabCols) {
      OSINT16 i, j;

      if (prevElemIdx >= 0) j = prevElemIdx;
      else j = 0;

      /* start to search from the previous element index for better
         performance.
         In most cases the order of elements is same as they appear in the
         schema, even for xsd:all. The worst case from performance point of
         view is when the order of xsd:all elements are reverse than in
         the schema. */

      for (i = 0; i < fstabRows; i++, j++) {
         const IDXTYPE* pCurRow;

         if (j >= fstabRows) j = 0;

         pCurRow = fstab + j * fstabCols;

         if (pCurRow[curState] != -1) {

            /* OSCRTLSTRCMP is used instead of rtXmlStrCmpAsc, because
               OSCRTLSTRCMP could be faster when it is being inlined
               by the compiler as an intrinistic function. */
            match = (OSBOOL)
               (OSCRTLSTRCMP ((const char*)localName,
                        (const char*)idtab[j].name) == 0) ;
#ifdef OSXMLNS12
            if (match && nsidx >= 0) {
               match = (OSBOOL)(nsidx == idtab[j].nsIndex);
            }
#endif
            if (match) {
               *pState = pCurRow[curState];
               return j;
            }
            else if (*idtab[j].name == 0)
               openTypeIdx = j;
         }
      }
   }
   if (openTypeIdx != -1) {
      const IDXTYPE* pCurRow = fstab + openTypeIdx * fstabCols;
      *pState = pCurRow[curState];
      return openTypeIdx;
   }

   return -1;
}


