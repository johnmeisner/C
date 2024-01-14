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
#define FUNCNAME rtSaxLookupElemID
#define FINDFUNC rtSaxFindElemID
#define IDXTYPE  OSINT16
#endif

#include "rtxmlsrc/rtSaxDefs.h"
#include "rtxmlsrc/osrtxml.h"
#include "rtxmlsrc/rtXmlErrCodes.h"

EXTXMLMETHOD OSINT16 FUNCNAME (OSCTXT* pctxt, OSINT16* pState, OSINT16 prevElemIdx,
                  const OSUTF8CHAR* localName, const OSUTF8CHAR* qName,
#ifdef OSXMLNS12
                  OSINT32 nsidx, /* namespace index */
#endif
                  const OSSAXElemTableRec idtab[], const IDXTYPE* fstab,
                  OSINT16 fstabRows, OSINT16 fstabCols)
{
   const OSINT16 curState = *pState;
   OSINT16 elemIdx;

   elemIdx = FINDFUNC
      (pState, prevElemIdx, localName,
#ifdef OSXMLNS12
       nsidx,
#endif
       idtab, fstab, fstabRows, fstabCols);

   if (elemIdx == -1) {
      LOG_SAXERRNEW1_AND_SKIP (pctxt, RTERR_UNEXPELEM,
         rtxErrAddStrParm (pctxt, (const char*)qName));
      return -1;
   }
   else {
      if (curState >= 0 && curState < fstabCols) {
         OSOCTET s[256];
         OSRTMEMBUF memBuf;
         OSINT16 k = 0, i;

         rtxMemBufInitBuffer (pctxt, &memBuf, s, sizeof (s), 256);

         for (i = 0; i < fstabRows; i++) {
            const IDXTYPE* pCurRow = fstab + i * fstabCols;

            if (pCurRow [curState] != -1) {
               size_t len = OSCRTLSTRLEN ((const char*)idtab[i].name);

               if (k > 0) {
                  rtxMemBufAppend (&memBuf, (const OSOCTET*)", ", 2);
               }
               rtxMemBufAppend (&memBuf, (const OSOCTET*)idtab[i].name, len);
               k++;
            }
         }
         rtxMemBufSet (&memBuf, 0, 1);

         LOG_SAXERRNEW2 (pctxt, RTERR_IDNOTFOU,
            rtxErrAddStrParm (pctxt, (const char*)
                              rtxMemBufGetData (&memBuf, 0)),
            rtxErrAddStrParm (pctxt, (const char*)qName));

         rtxMemBufFree (&memBuf);
      }
      else
         LOG_SAXERRNEW (pctxt, XML_E_ELEMSMISRQ);
   }

   return elemIdx;
}


