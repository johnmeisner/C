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
#include "rtxmlsrc/rtXmlPull.hh"

EXTXMLMETHOD int rtXmlpDecAny2 (OSCTXT* pctxt, OSUTF8CHAR** outdata)
{
   struct OSXMLReader* pXmlReader;
   OSINT32 curLevel;
   size_t oldsize = 0, newsize = 0;
   OSUTF8CHAR* pNewData = 0;
   int stat = 0;
   int prevState = 0;

   OSRTASSERT (0 != pctxt->pXMLInfo);
   pXmlReader = rtXmlpGetReader (pctxt);

   if (outdata)
      *outdata = 0;

   rtXmlRdSetWhiteSpaceMode (pXmlReader, OSXMLWSM_PRESERVE);
   pXmlReader->mbNoTransform = TRUE;
   curLevel = pXmlReader->mLevel;

   while ((stat = rtXmlRdNext (pXmlReader)) >= 0) {
      size_t sz = 0;

      if (pXmlReader->mLastEvent.mId == OSXMLEVT_END_DOCUMENT) {
         stat = LOG_RTERRNEW (pctxt, XML_E_UNEXPEOF);
         break;
      }
      else if (pXmlReader->mLastEvent.mId == OSXMLEVT_TEXT) {
         if (outdata) {
            sz = pXmlReader->mData.length;
            newsize = oldsize + sz;

            if (pXmlReader->mbCDATA) {
               if (prevState >= 0)
                  newsize += 9; /* <![CDATA[ */
               if (pXmlReader->mState >= 0)
                  newsize += 3; /* ]]> */
            }

            pNewData = (OSOCTET*) rtxMemRealloc (pctxt, pNewData, newsize+1);

            if (pNewData == 0) {
               rtxMemFreePtr (pctxt, *outdata);
               stat = LOG_RTERRNEW (pctxt, RTERR_NOMEM);
               break;
            }

            if (pXmlReader->mbCDATA && prevState >= 0) {
               OSCRTLSAFEMEMCPY (pNewData + oldsize, newsize+1-oldsize,
                     "<![CDATA[", 9);
               oldsize += 9;
            }

            OSCRTLSAFEMEMCPY (pNewData + oldsize, newsize+1-oldsize,
                  pXmlReader->mData.value, sz);

            if (pXmlReader->mbCDATA && pXmlReader->mState >= 0)
               OSCRTLSAFEMEMCPY (pNewData + oldsize + sz, newsize+1-oldsize-sz,
                     "]]>", 3);
         }
      }
      else {
         size_t markedPos;

         if (pXmlReader->mLastEvent.mId == OSXMLEVT_END_TAG &&
             pXmlReader->mLastEvent.mLevel == curLevel) {
            /* empty element */
            if (0 == pNewData) {
               pNewData = (OSOCTET*) rtxMemAlloc (pctxt, newsize + 1);
               if (pNewData == 0) {
                  stat = LOG_RTERRNEW (pctxt, RTERR_NOMEM);
                  break;
               }
               if (outdata)
                  *outdata = pNewData;
            }
            break;
         }

         if (pXmlReader->mMarkedPos != (size_t)-1)
            markedPos = pXmlReader->mMarkedPos;
         else
            markedPos = 0;

         sz = pXmlReader->mByteIndex - markedPos;

         newsize = oldsize + sz;

         if (pXmlReader->mLastEvent.mId == OSXMLEVT_START_TAG)
            newsize++;
         else if(pXmlReader->mLastEvent.mId == OSXMLEVT_END_TAG) {
            if (pXmlReader->mbEmptyElement) {
               newsize = oldsize;
               sz = 0;
            }
            else {
               newsize +=2;
            }
         }
         else if(pXmlReader->mLastEvent.mId == OSXMLEVT_COMMENT) {
            if (prevState >= 0) /* first chunk */
               newsize +=3;
         }
         else if(pXmlReader->mLastEvent.mId == OSXMLEVT_PI) {
            if (prevState >= 0) /* first chunk */
               newsize++;
         }

         if (outdata) {
            pNewData = (OSOCTET*) rtxMemRealloc (pctxt, pNewData, newsize+1);

            if (pNewData == 0) {
               rtxMemFreePtr (pctxt, *outdata);
               stat = LOG_RTERRNEW (pctxt, RTERR_NOMEM);
               break;
            }

            if (pXmlReader->mLastEvent.mId == OSXMLEVT_START_TAG)
               pNewData[oldsize++] = '<';
            else if(pXmlReader->mLastEvent.mId == OSXMLEVT_END_TAG) {
               if (!pXmlReader->mbEmptyElement) {
                  pNewData[oldsize++] = '<';
                  pNewData[oldsize++] = '/';
               }
            }
            else if(pXmlReader->mLastEvent.mId == OSXMLEVT_COMMENT) {
               if (prevState >= 0) { /* first chunk */
                  pNewData[oldsize++] = '<';
                  pNewData[oldsize++] = '!';
                  pNewData[oldsize++] = '-';
               }
            }
            else if (pXmlReader->mLastEvent.mId == OSXMLEVT_PI) {
               if (prevState >= 0) /* first chunk */
                  pNewData[oldsize++] = '<';
            }

            OSCRTLSAFEMEMCPY (pNewData + oldsize, newsize-oldsize,
                  pXmlReader->mpBuffer + markedPos, sz);
         }
      }

      if (outdata)
         *outdata = pNewData;
      oldsize = newsize;

      prevState = pXmlReader->mState;
   }

   pXmlReader->mbNoTransform = FALSE;

   if (stat >= 0) {
      /* Null terminate the string */
      if (outdata) {
         char* pDst;
         char* pSrc;
         char* pSrcPrev;

         pNewData[newsize] = 0;

         /* normalize CRLF */
         pDst = (char*) pNewData;
         pSrcPrev = pDst;
         pSrc = strchr (pSrcPrev, '\r');

         while (pSrc) {
            if (pSrcPrev != pDst && pSrcPrev != pSrc)
               OSCRTLMEMMOVE (pDst, pSrcPrev, pSrc - pSrcPrev);

            pDst += pSrc - pSrcPrev;

            *pDst++ = '\n';
            pSrc++;
            if (*pSrc == '\n') pSrc++;

            pSrcPrev = pSrc;
            pSrc = strchr (pSrcPrev, '\r');
         }

         pSrc = strchr (pSrcPrev, 0);

         if (pSrcPrev != pDst && pSrcPrev != pSrc)
            OSCRTLMEMMOVE (pDst, pSrcPrev, pSrc - pSrcPrev + 1);
                                           /* +1 to copy null */
      }

      stat = 0;
   }
   else if (outdata) {
      rtxMemFreePtr (pctxt, *outdata);
      *outdata = 0;
   }

   return stat;
}

EXTXMLMETHOD int rtXmlpDecAny (OSCTXT* pctxt, const OSUTF8CHAR** outdata)
{
   if (0 != outdata) {
      OSUTF8CHAR* pUTF8Str;
      int ret = rtXmlpDecAny2 (pctxt, &pUTF8Str);
      *outdata = pUTF8Str;
      return ret;
   }
   else {
      return rtXmlpDecAny2 (pctxt, 0);
   }
}
