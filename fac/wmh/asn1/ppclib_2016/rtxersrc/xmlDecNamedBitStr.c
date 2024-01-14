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

#include "xed_common.hh"
#include "rtxsrc/rtxToken.h"

int xmlDecNamedBitStr
  (OSRTMEMBUF *pMemBuf, OSOCTET* pData, int dataSize, OSUINT32* pNumbits,
   const XmlNamedBitsDict* pBitDict, const XMLCHAR* chars, int length)
{
   OSCTXT* pctxt = pMemBuf->pctxt;
   int stat;
   OSTOKENCTXT tokCtxt;
   const OSUTF8CHAR* pToken;

   if (pMemBuf->userState == 0) {
      *pNumbits = 0;
      memset (pData, 0, dataSize);

      if (chars != 0) {
         while (OS_ISSPACE (*chars)) {
            chars++; length--;
         }
         if (*chars == '0' || *chars == '1')
            pMemBuf->userState = 1;
         else
            pMemBuf->userState = 2;
      }
   }

   if (pMemBuf->userState == 1) {
      if (chars != 0)
         stat = xerDecBitStrMemBuf (pMemBuf, chars, length, FALSE);
      else
         stat = xerDecCopyBitStr (pctxt,
            pData, pNumbits,
            dataSize, pMemBuf->bitOffset);
      if (stat != 0) return LOG_RTERR (pctxt, stat);
   }
   else {
      if (chars != 0) {
         stat = rtxMemBufAppend (pMemBuf, (OSOCTET*)chars,
                                length * sizeof (XMLCHAR));
         if (stat != 0) return LOG_RTERR (pctxt, stat);
      }
      pToken = rtxTokGetFirst (&tokCtxt, (OSUTF8CHAR*)OSMEMBUFPTR (pMemBuf),
                              OSMEMBUFUSEDSIZE(pMemBuf), " \r\n\t\0", 5);

      while (pToken != 0) {
         int i;
         for (i = 0; pBitDict[i].name != 0; i++) {
            if (xerCmpText (pToken, pBitDict[i].name)) {
               rtxSetBit (pData, dataSize * 8, pBitDict[i].bitnum);
               if (*pNumbits <= pBitDict[i].bitnum)
                  *pNumbits = pBitDict[i].bitnum + 1;
               break;
            }
         }
         if (pBitDict[i].name == 0) {
            return LOG_RTERR (pctxt, RTERR_IDNOTFOU);
         }

         pToken = rtxTokGetNext (&tokCtxt);
      }
      rtxMemBufCut (pMemBuf, 0,
         (OSUINT32)(tokCtxt.pStr - (OSUTF8CHAR*)OSMEMBUFPTR (pMemBuf)));
   }
   return 0;
}

