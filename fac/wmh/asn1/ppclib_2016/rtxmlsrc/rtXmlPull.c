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

#include <stdio.h>
#include "rtxsrc/rtxContext.hh"
#include "rtxsrc/rtxCtype.h"
#include "rtxsrc/rtxMemory.h"
#include "rtxsrc/rtxUnicode.h"
#include "rtxmlsrc/osrtxml.hh"
#include "rtxmlsrc/rtXmlPull.hh"

/*
_OSXML_NO_ENDTAG_VALIDATION - if defined, turns off the validation of end
tag name against start tag name. Improves perfromance, decreases memory usage;

_OSXML_NO_ATTR_DUPL_DETECTION - if defined, does not check for
duplication of attributes. Improves perfromance, decreases memory usage;

_OSXML_NO_NAMESPACES          - no namespace support.
*/

#define INVSYM_SZ 8 /* set string size for INVSYMBOL error */
#define OSXMLCONST_ATTRIBUTES_DELTA 10 /* default delta for mpAttributes */
#define OSXMLCONST_STACK_DELTA 10      /* default delta for mpAttributes */
#define OSXML_ESCAPE_CHAR_BUF_SIZE 10

typedef enum {
   OSXMLDT_START_ELEMENT_CLEANUP = 1,
   OSXMLDT_END_ELEMENT_EVENT = 2,
   OSXMLDT_END_ELEMENT_CLEANUP = 3
} OSXMLDelayedTaskId;

#define rtXmlRdLogError(pReader,stat) \
pReader->mError=stat, LOG_RTERRNEW (pReader->mpCtxt, stat)

#define rtXmlRdLogInvSymbolError(pReader) \
rtXmlRdLogInvSymbol (pReader), LOG_RTERRNEW (pReader->mpCtxt, XML_E_INVSYMBOL)

#define rtXmlRdIsError(pReader) (pReader->mError != 0)

#define rtXmlRdStrEqual(a,b) (((a)->length == (b)->length) ? \
rtxUTF8StrnEqual((a)->value, (b)->value, (a)->length) : FALSE)

static void rtXmlRdLogInvSymbol (OSXMLReader* pReader) {
   const OSUTF8CHAR* p = pReader->mpBuffer + pReader->mByteIndex;

   if (*p >= ' ')
      rtxErrAddStrnParm (pReader->mpCtxt, (const char*) p, 1);
   else { /* output hex code for control chars */
      char buf[4];
      unsigned tm;

      buf[0] = '\\';
      buf[1] = 'x';
      tm = (*p >> 4) & 0xF;
      buf[2] = (char) ((tm <= 9) ? tm + '0' : tm - 10 + 'A');
      tm = *p & 0xF;
      buf[3] = (char) ((tm <= 9) ? tm + '0' : tm - 10 + 'A');
      rtxErrAddStrnParm (pReader->mpCtxt, buf, 4);
   }

   rtXmlRdErrAddSrcPos (pReader, TRUE);
   pReader->mError = XML_E_INVSYMBOL;
}

/**
 * Get pointer into the given reader's buffer, to the marked position, plus
 * the given offset from there. To use this function, rtXmlRdMarkPosition
 * should be called before.
 * @param pReader The XML reader
 * @param offset Offset from the marked position you want the pointer to
 *                point at.  Passing 0 returns a pointer to the marked position.
 * @param pGuaranteedSize If not null, pointer to variable to receive the
 *          guaranteed number of bytes that can be read from the returned
 *          pointer.
 */
static const OSUTF8CHAR* rtXmlRdGetPtr
(const OSXMLReader* pReader, size_t offset, size_t* pGuaranteedSize)
{
   size_t markedPos;

   OSRTASSERT (pReader != 0);

   if (pReader->mMarkedPos != (size_t)-1)
      markedPos = pReader->mMarkedPos;
   else
      markedPos = 0;

   if (pGuaranteedSize != 0) {
      *pGuaranteedSize = pReader->mByteIndex - (markedPos + offset);
   }
   return pReader->mpBuffer + markedPos + offset;
}

static OSBOOL transformEscapeChar
(const OSUTF8CHAR* pEscCharPtr, size_t escCharLen,
 OSXMLStrFragment* pDestStr, OSUTF8CHAR* pBuf)
{
   OSRTASSERT (pEscCharPtr != 0 && pDestStr != 0);

   pDestStr->length = 0;
   if (escCharLen == 3) {
      if (OSCRTLSTRNCMP ((const char*)pEscCharPtr, "&lt", 3) == 0) {
         pBuf[0] = '<';
         pDestStr->value = pBuf;
         pDestStr->length = 1;
      }
      else if (OSCRTLSTRNCMP ((const char*)pEscCharPtr, "&gt", 3) == 0) {
         pBuf[0] = '>';
         pDestStr->value = pBuf;
         pDestStr->length = 1;
      }
   }
   else if (escCharLen == 4) {
      if (OSCRTLSTRNCMP ((const char*)pEscCharPtr, "&amp", 4) == 0) {
         pBuf[0] = '&';
         pDestStr->value = pBuf;
         pDestStr->length = 1;
      }
   }
   else if (escCharLen == 5) {
      if (OSCRTLSTRNCMP ((const char*)pEscCharPtr, "&quot", 5) == 0) {
         pBuf[0] = '\"';
         pDestStr->value = pBuf;
         pDestStr->length = 1;
      }
      else if (OSCRTLSTRNCMP ((const char*)pEscCharPtr, "&apos", 5) == 0) {
         pBuf[0] = '\'';
         pDestStr->value = pBuf;
         pDestStr->length = 1;
      }
   }

   if (pDestStr->length == 0 && escCharLen > 2 && pEscCharPtr[1] == '#') {
      OS32BITCHAR num = 0;
      size_t i;

      if (escCharLen > 3 && pEscCharPtr[2] == 'x') {
         /* hex */
         for (i = 3; i < escCharLen; i++) {
            OSUTF8CHAR c = pEscCharPtr[i];
            if (c >= '0' && c <= '9')
               c -= '0';
            else if (c >= 'a' && c <= 'f')
               c -= 'a' - 10;
            else if (c >= 'A' && c <= 'F')
               c -= 'A' - 10;
            else break;

            if (c < 16)
               num = num * 16 + c;
         }
      }
      else {
         /* decimal */
         for (i = 2; i < escCharLen; i++) {
            if (OS_ISDIGIT(pEscCharPtr[i]))
               num = num * 10 + pEscCharPtr[i] - '0';
            else break;
         }
      }

      if (i == escCharLen) {
         int len = rtxUTF8EncodeChar
            (num, pBuf, OSXML_ESCAPE_CHAR_BUF_SIZE - 1);
         if (len > 0) {
            pDestStr->value = pBuf;
            pDestStr->length = rtxUTF8CharSize (num);
         }
      }
   }

   if (pDestStr->length == 0) {
      OSRTASSERT (pBuf != 0);
      /* TODO: analyze pEscCharPtr and transform to UTF8 char and
         put in pBuf */
   }
   return (OSBOOL) ((pDestStr->length == 0) ? FALSE : TRUE);
}


static void rtXmlRdStackInit
(OSXMLReader* pReader, OSXMLStack* pStack, size_t unitSize)
{
   OSRTASSERT (pReader != 0 && pStack != 0 && unitSize > 0 && unitSize < 8192);

   pStack->mbDynamic = TRUE;
   pStack->mCount = 0;
   pStack->mpStackArray = 0;
   pStack->mSize = 0;
   pStack->mUnitSize = unitSize;
}

static OSBOOL rtXmlRdStackEnsureCapacity
(OSXMLReader* pReader, OSXMLStack* pStack, size_t reqUnits)
{
   OSCTXT* pctxt;

   OSRTASSERT (pReader != 0 && pStack != 0 &&
               pReader->mpCtxt != 0 && reqUnits < (size_t)INT_MAX);

   pctxt = pReader->mpCtxt;
   if (pStack->mCount + reqUnits > pStack->mSize) {
      if (pStack->mbDynamic) {
         void* pNewArr;
         size_t newSize;

         newSize = (pStack->mCount + reqUnits) +
            OSXMLCONST_STACK_DELTA;

         if (0 == pStack->mSize) {
            pNewArr = rtxMemAlloc (pctxt, newSize * pStack->mUnitSize);
         }
         else {
            pNewArr = rtxMemRealloc
               (pctxt, pStack->mpStackArray, newSize * pStack->mUnitSize);
         }

         if (pNewArr == 0) {
            return FALSE;
         }

         pStack->mpStackArray = pNewArr;
         pStack->mSize = newSize;
      }
      else
         return FALSE;
   }
   return TRUE;
}

static OSBOOL rtXmlRdStackReinit
(OSXMLReader* pReader, OSXMLStack* pStack)
{
   OSCTXT* pctxt;

   OSRTASSERT (pReader != 0 && pStack != 0 && pReader->mpCtxt != 0);

   pctxt = pReader->mpCtxt;

   pStack->mCount = 0;

   if (pStack->mSize > 0) {
      pStack->mpStackArray =
         rtxMemAlloc (pctxt, pStack->mSize * pStack->mUnitSize);

      if (pStack->mpStackArray == 0)
         return FALSE;
   }
   else
      pStack->mpStackArray = 0;

   return TRUE;
}

static void* rtXmlRdStackPushNew (OSXMLReader* pReader, OSXMLStack* pStack)
{
   OSRTASSERT (pReader != 0 && pStack != 0);

   if (!rtXmlRdStackEnsureCapacity (pReader, pStack, 1))
      return 0;

   return ((OSOCTET*)pStack->mpStackArray) +
      pStack->mCount++ * pStack->mUnitSize;
}

static void* rtXmlRdStackPop (OSXMLReader* pReader, OSXMLStack* pStack)
{
   OSOCTET* ptr;

   OSRTASSERT (pReader != 0 && pStack != 0);

   if (pStack->mCount == 0) return 0;

   ptr = ((OSOCTET*)pStack->mpStackArray) +
      (--pStack->mCount) * pStack->mUnitSize;

   return ptr;
}

static void* rtXmlRdStackGetTop (OSXMLReader* pReader, OSXMLStack* pStack)
{
   OSOCTET* ptr;

   OSRTASSERT (pReader != 0 && pStack != 0);

   ptr = ((OSOCTET*)pStack->mpStackArray) +
      (pStack->mCount - 1) * pStack->mUnitSize;

   return ptr;
}

static const void* rtXmlRdStackGetByIndex
(const OSXMLReader* pReader, const OSXMLStack* pStack, size_t i)
{
   OSRTASSERT (pReader != 0 && pStack != 0);

   if (i < pStack->mCount) {
      const OSOCTET* ptr =
         ((const OSOCTET*)pStack->mpStackArray) + i * pStack->mUnitSize;
      return ptr;
   }
   return 0;
}

static void rtXmlRdAddDelayedTask
(OSXMLReader* pReader, OSXMLDelayedTaskId taskId)
{
   OSRTASSERT (pReader != 0);
   OSRTASSERT (pReader->mDelayedTaskCount < MAX_DELAYED_TASK_SIZE);

   if (pReader->mDelayedTaskCount < MAX_DELAYED_TASK_SIZE) {
      size_t idx = (pReader->mDelayedTaskIndex +
                      pReader->mDelayedTaskCount) % MAX_DELAYED_TASK_SIZE;
      pReader->mDelayedTasks [idx] = (OSOCTET)taskId;
      pReader->mDelayedTaskCount++;
   }
}

static void rtXmlRdPopNamespaces (OSXMLReader* pReader);

static int endElement (OSXMLReader* pReader)
{
   OSRTASSERT (pReader != 0);

   if (pReader->mbLastChunk == FALSE) {
      /* if mbLastChunk is FALSE, we need to simulate
         character data with zero length and mbLastChunk set
         to TRUE */
      pReader->mLastEvent.mId = OSXMLEVT_TEXT;
      pReader->mLastEvent.mLevel = pReader->mLevel;
      pReader->mData.length = 0;
      pReader->mbLastChunk = TRUE;

      /* add end element to the delayed task list */
      rtXmlRdAddDelayedTask (pReader, OSXMLDT_END_ELEMENT_EVENT);
   }
   else {
      pReader->mLastEvent.mId = OSXMLEVT_END_TAG;
      pReader->mLastEvent.mLevel = pReader->mLevel;
      pReader->mLevel--;
      rtXmlRdPopNamespaces (pReader);
      /* rtXmlRdAddDelayedTask (pReader, OSXMLDT_END_ELEMENT_CLEANUP); */

      if (pReader->mLevel == 0) {
         /* end root tag */
#ifndef _NO_STREAM
         if (OSRTISSTREAM (pReader->mpCtxt)) {
            pReader->mpCtxt->pStream->nextMarkOffset +=
               pReader->mByteIndex + 1 - pReader->mLastByteIndex;
         }
#endif /* _NO_STREAM */

#ifdef _PULL_FROM_BUFFER
         else {
            OSRTBuffer* buffer = &pReader->mpCtxt->buffer;
            buffer->byteIndex -= pReader->mReadSize - pReader->mByteIndex - 1;
         }
#endif /* _PULL_FROM_BUFFER */
         LCHECKX (pReader->mpCtxt);

         /* cleanup temporary data */
         rtxMemFreePtr (pReader->mpCtxt, pReader->mAttributes.mpStackArray);
         rtxMemFreePtr (pReader->mpCtxt, pReader->mTagNamesStack.mpStackArray);
         rtxMemFreePtr (pReader->mpCtxt, pReader->mNamespacesStack.mpStackArray);
         rtxMemFreePtr (pReader->mpCtxt, pReader->mRewindPosStack.mpStackArray);

         pReader->mAttributes.mpStackArray = 0;
         pReader->mAttributes.mSize = 0;
         pReader->mTagNamesStack.mpStackArray = 0;
         pReader->mTagNamesStack.mSize = 0;
         pReader->mNamespacesStack.mpStackArray = 0;
         pReader->mNamespacesStack.mSize = 0;
         pReader->mRewindPosStack.mpStackArray = 0;
         pReader->mRewindPosStack.mSize = 0;
      }
   }

   return pReader->mLastEvent.mId;
}

static int rtXmlRdProcessDelayedTasks (OSXMLReader* pReader)
{
   size_t i;
   size_t nm;

   OSRTASSERT (pReader != 0);

   nm = pReader->mDelayedTaskCount;

   for (i = 0; i < nm; i++) {
      OSOCTET taskId = pReader->mDelayedTasks [pReader->mDelayedTaskIndex];
      pReader->mDelayedTaskIndex++;
      pReader->mDelayedTaskCount--;
      if (pReader->mDelayedTaskIndex >= MAX_DELAYED_TASK_SIZE)
         pReader->mDelayedTaskIndex = 0;

      switch (taskId) {
         /* cleanup attribute counter */
         case OSXMLDT_START_ELEMENT_CLEANUP:
            pReader->mAttributes.mCount = 0;
            break;

         /* simulate end element event for empty element */
         case OSXMLDT_END_ELEMENT_EVENT:
            return endElement (pReader);

         case OSXMLDT_END_ELEMENT_CLEANUP:
            pReader->mLevel--;
            break;
      }
   }
   return 0;
}

EXTXMLMETHOD int rtXmlRdSelectAttribute (OSXMLReader* pReader, size_t idx)
{
   const OSXMLAttrOffset* pAttrOff;

   OSRTASSERT (pReader != 0);

   if ((pReader->mLastEvent.mId & OSXMLEVT_ID_MASK) != OSXMLEVT_START_TAG ||
      idx > pReader->mAttributes.mCount)
   {
      return -1;
   }
   pAttrOff = (const OSXMLAttrOffset*)
      rtXmlRdStackGetByIndex (pReader, &pReader->mAttributes, idx);

   pReader->mDataMode = OSXMLDM_SIMULATED;
   pReader->mData.length = pAttrOff->mValue.length;
   pReader->mData.value = (const OSUTF8CHAR*)
      rtXmlRdGetPtr (pReader, pAttrOff->mValue.offset, 0);

   return 0;
}

static int rtXmlRdProcessAttrValue
(OSXMLReader* pReader, OSXMLDataCtxt* pDataCtxt)
{
   OSUTF8CHAR utf8ch, utf8chTm;
   size_t offset;
   OSUTF8CHAR* ptr;
   const OSUTF8CHAR* eptr = 0; /* remove warning */
   OSXMLStrFragment destStr;
   const OSUTF8CHAR* end;

   destStr.value = 0; /* remove warning */
   destStr.length = 0; /* remove warning */

   OSRTASSERT (pReader != 0 && pDataCtxt != 0);

   if (pDataCtxt->mbInsTokenSeparator) {
      pDataCtxt->mbInsTokenSeparator = FALSE;
      pReader->mCharBuf[0] = ' ';
      pDataCtxt->mData.length = 1;
      pDataCtxt->mData.value = pReader->mCharBuf;
      return 1;
   }

   end = pDataCtxt->mSrcData.value + pDataCtxt->mSrcData.length;
   offset = pDataCtxt->mSrcDataOffset;

   if (!pReader->mbNoTransform) {
      if (pReader->mWhiteSpaceMode == OSXMLWSM_COLLAPSE) {
         /* skip ws */
         while (pDataCtxt->mSrcDataOffset < pDataCtxt->mSrcData.length) {
            ptr = (OSUTF8CHAR*)
               (pDataCtxt->mSrcData.value + pDataCtxt->mSrcDataOffset);
            utf8ch = *ptr;

            if (utf8ch == '&') {
               eptr = ptr;
               /* read till the ';' or end of the buffer */
               while (eptr != end && *eptr != ';')
                  eptr++;

               if (eptr != end) {
                  /* need transformation of escape symbol */
                  if (transformEscapeChar (ptr, eptr - ptr, &destStr,
                                           pReader->mCharBuf))
                  {
                     utf8ch = destStr.value[0];
                     if (destStr.length != 1 || !OS_ISSPACE (utf8ch))
                     {
                        /* char ref is no white space */
                        break;
                     }
                  }
                  else
                     break;
               }

               pDataCtxt->mSrcDataOffset += (eptr - ptr);
            }
            else if (!OS_ISSPACE (utf8ch))
               break;

            pDataCtxt->mSrcDataOffset++;
         }
      }

      /* get chunk */
      offset = pDataCtxt->mSrcDataOffset;
      pDataCtxt->mData.length = 0;

      while (pDataCtxt->mSrcDataOffset < pDataCtxt->mSrcData.length) {
         ptr = (OSUTF8CHAR*)
            (pDataCtxt->mSrcData.value + pDataCtxt->mSrcDataOffset);
         utf8ch = *ptr;

         if (utf8ch == '&') {
            /* if this is not first symbol - return previous chunk first */
            if (offset != pDataCtxt->mSrcDataOffset) {
               pDataCtxt->mData.length = pDataCtxt->mSrcDataOffset - offset;
               pDataCtxt->mData.value = pDataCtxt->mSrcData.value + offset;
               pDataCtxt->mbLastChunk = FALSE;
               if (pReader->mWhiteSpaceMode == OSXMLWSM_COLLAPSE) {
                  eptr = ptr;
                  /* read till the ';' or end of the buffer */
                  while (eptr != end && *eptr != ';')
                     eptr++;

                  if (eptr != end) {
                     /* need transformation of escape symbol */
                     if (transformEscapeChar (ptr, eptr - ptr, &destStr,
                                              pReader->mCharBuf))
                     {
                        utf8ch = destStr.value[0];
                        if (destStr.length != 1 ||
                            !OS_ISSPACE (utf8ch))
                        {
                           /* char ref is not space */
                           return 1;
                        }
                     }
                     else
                        return 1;
                  }
               }
               else
                  return 1;

            }

            /* in collapse mode escape char already translated */
            if (pReader->mWhiteSpaceMode == OSXMLWSM_COLLAPSE) {
               /* in collapse mode escape char already translated
                  and it is not ws */
               pDataCtxt->mData.length = destStr.length;
               pDataCtxt->mData.value = destStr.value;
            }
            else {
               eptr = ptr;
               /* read till the ';' or end of the buffer */
               while (eptr != end && *eptr != ';')
                  eptr++;

               if (eptr != end) {
                  /* need transformation of escape symbol */
                  if (!transformEscapeChar (ptr, eptr-ptr, &destStr,
                                            pReader->mCharBuf))
                  {
                     /* XML file is not well-formed */
                     rtxErrAddStrnParm (pReader->mpCtxt, (const char*) ptr,
                                        eptr - ptr);
                     rtXmlRdErrAddSrcPos (pReader, TRUE);
                     return rtXmlRdLogError (pReader, XML_E_BADCHARREF);
                  }

                  pDataCtxt->mData.length = destStr.length;
                  pDataCtxt->mData.value = destStr.value;
               }
            }

            pDataCtxt->mSrcDataOffset += (eptr + 1 - ptr);

            if (destStr.value) {
               utf8ch = destStr.value[0];
               if (destStr.length == 1 && OS_ISSPACE (utf8ch))
               {
                  if (pReader->mWhiteSpaceMode == OSXMLWSM_REPLACE)
                     pReader->mCharBuf[0] = ' ';
               }
            }

            if (pReader->mWhiteSpaceMode != OSXMLWSM_COLLAPSE)
               return 1;
         }
         else if (!OS_ISSPACE (utf8ch)) {
            pDataCtxt->mSrcDataOffset++;
            continue;
         }
         else { /* white space */
            if (pDataCtxt->mSrcDataOffset > offset &&
                (utf8ch != ' ' ||
                 pReader->mWhiteSpaceMode == OSXMLWSM_COLLAPSE))
            {
               /* prepare to return previous chunk */
               pDataCtxt->mData.length = pDataCtxt->mSrcDataOffset - offset;
               pDataCtxt->mData.value = pDataCtxt->mSrcData.value + offset;
               pDataCtxt->mbLastChunk = FALSE;
            }
         }

         if (pReader->mWhiteSpaceMode == OSXMLWSM_COLLAPSE)
         {
            OSBOOL firstChar = TRUE;

            /* if end of token then skip ws */
            while (pDataCtxt->mSrcDataOffset < pDataCtxt->mSrcData.length)
            {
               ptr = (OSUTF8CHAR*)
                  (pDataCtxt->mSrcData.value + pDataCtxt->mSrcDataOffset);
               utf8chTm = *ptr;

               if (utf8chTm == '&') {
                  eptr = ptr;

                  /* read till the ';' or end of the buffer */
                  while (eptr != end && *eptr != ';')
                     eptr++;

                  if (eptr != end) {
                     /* need transformation of escape symbol */
                     if (transformEscapeChar (ptr, eptr - ptr, &destStr,
                                              pReader->mCharBuf))
                     {
                        utf8chTm = destStr.value[0];
                        if (destStr.length == 1 && OS_ISSPACE (utf8chTm)) {
                           /* char ref is space */
                           if (firstChar) {
                              firstChar = FALSE;

                              if (pReader->mbListMode)
                                 pDataCtxt->mbLastChunk = TRUE;
                              else
                                 pDataCtxt->mbInsTokenSeparator = TRUE;
                           }
                        }
                        else
                           break;
                     }
                     else
                        break;
                  }

                  pDataCtxt->mSrcDataOffset += (eptr - ptr);
               }
               else if (!OS_ISSPACE (utf8chTm))
                  break;
               else if (firstChar) { /* white space char */
                  firstChar = FALSE;

                  if (pReader->mbListMode)
                     pDataCtxt->mbLastChunk = TRUE;
                  else
                     pDataCtxt->mbInsTokenSeparator = TRUE;
               }

               pDataCtxt->mSrcDataOffset++;
            }

            if (pDataCtxt->mSrcDataOffset == pDataCtxt->mSrcData.length) {
               pDataCtxt->mbLastChunk = TRUE;
               if (pReader->mbListMode)
                  pReader->mbListMode = FALSE;
            }

            if (pReader->mbListMode) {
               pReader->mData.value += pDataCtxt->mSrcDataOffset;
               pReader->mData.length -= pDataCtxt->mSrcDataOffset;
            }
         }

         if (pDataCtxt->mData.length > 0) {
            /* return previous chunk or translated ws */
            return 1;
         }

         if (pReader->mWhiteSpaceMode != OSXMLWSM_COLLAPSE && utf8ch != ' ')
         {
            /* return ' ' instead of #xD, #x9, #xA or #xD#xA */
            pDataCtxt->mSrcDataOffset++;
            offset++;

            if (ptr[1] == '\n') {
               pDataCtxt->mSrcDataOffset++;
               offset++;
            }

            pReader->mCharBuf[0] = ' ';
            pDataCtxt->mData.length = 1;
            pDataCtxt->mData.value = pReader->mCharBuf;
            return 1;
         }

         pDataCtxt->mSrcDataOffset++;
         if (pReader->mWhiteSpaceMode == OSXMLWSM_COLLAPSE)
            offset++;
      }
   }

   pDataCtxt->mbLastChunk = TRUE;
   pDataCtxt->mData.length = pDataCtxt->mSrcDataOffset - offset;
   pDataCtxt->mData.value = pDataCtxt->mSrcData.value + offset;

   if (pReader->mbListMode) {
      pReader->mData.value += pDataCtxt->mSrcDataOffset;
      pReader->mData.length -= pDataCtxt->mSrcDataOffset;
   }

   return (pDataCtxt->mData.length > 0) ? 1 : 0;
}

static const OSUTF8CHAR* rtXmlRdDupStr
   (OSXMLReader* pReader, OSXMLStrFragment* pDestStr, const OSUTF8CHAR* pstr)
{
   OSCTXT* pctxt;

   OSRTASSERT (pReader != 0 && pReader->mpCtxt != 0);

   pctxt = pReader->mpCtxt;

#ifdef _PULL_FROM_BUFFER
   if (0 != pstr) {
      OSUTF8CHAR* pbuf = (OSUTF8CHAR*)
         rtxMemAlloc (pctxt, pDestStr->length);
      if (0 != pbuf) {
         OSCRTLSAFEMEMCPY (pbuf, pDestStr->length, pstr, pDestStr->length);
      }
      pDestStr->value = pbuf;
   }
   else return 0;
#else
   if (OSRTISSTREAM(pctxt)) {
      /* if stream is used - do real copy of pstr to pDestStr->value */
      /* note, pDestStr->length already should be set! */

      if (0 != pstr) {
         OSUTF8CHAR* pbuf = (OSUTF8CHAR*)
            rtxMemAlloc (pctxt, pDestStr->length);
         if (0 != pbuf) {
            OSCRTLSAFEMEMCPY (pbuf, pDestStr->length, pstr, pDestStr->length);
         }
         pDestStr->value = pbuf;
      }
      else return 0;
   }
   else {
      /* if no stream is used (just static buffer) - no copy is necessary */

      pDestStr->value = pstr;
   }
#endif

   return pDestStr->value;
}

static void rtXmlRdFreeStr (OSXMLReader* pReader, OSXMLStrFragment* pDestStr)
{
   OSCTXT* pctxt;

   OSRTASSERT (pReader != 0 && pReader->mpCtxt != 0);

   pctxt = pReader->mpCtxt;

#ifdef _PULL_FROM_BUFFER
   rtxMemFreePtr (pctxt, pDestStr->value);
#else
   if (OSRTISSTREAM(pctxt)) {
      rtxMemFreePtr (pctxt, pDestStr->value);
   }
#endif
}

#ifdef OSXMLNS12
static const OSUTF8CHAR* const xsiUrn =
   OSUTF8("http://www.w3.org/2001/XMLSchema-instance");

static const OSUTF8CHAR* const soapEnvUrn =
   OSUTF8("http://schemas.xmlsoap.org/soap/envelope/");

static const OSUTF8CHAR* const soap12EnvUrn =
   OSUTF8("http://www.w3.org/2003/05/soap-envelope");

static const OSUTF8CHAR* const xsdUrn =
   OSUTF8("http://www.w3.org/2001/XMLSchema");

#else
static const char* const xsiUrn = "http://www.w3.org/2001/XMLSchema-instance";
static const char* const soapEnvUrn = "http://schemas.xmlsoap.org/soap/envelope/";
static const char* const soap12EnvUrn = "http://www.w3.org/2003/05/soap-envelope";
static const char* const xsdUrn = "http://www.w3.org/2001/XMLSchema";
#endif

static const size_t xsiUrnLen = 41;
static const size_t soapEnvUrnLen = 41;
static const size_t soap12EnvUrnLen = 39;
static const size_t xsdUrnLen = 32;

static void rtXmlRdPushNamespace
   (OSXMLReader* pReader, const OSUTF8CHAR* prefix, size_t prefixLen,
    const OSUTF8CHAR* urn, size_t urnLen)
{
   OSXMLNamespace_* pNs = (OSXMLNamespace_*)
      rtXmlRdStackPushNew (pReader, &pReader->mNamespacesStack);

   int i;
#ifdef OSXMLNS12
   const OSUTF8CHAR** pUrn = pReader->mNamespaceTable;
#else
   const char** pUrn = pReader->mNamespaceTable;
#endif

   OSUTF8CHAR c;

   OSRTASSERT (pNs != 0);

   /* remove leading spaces */
   while (urnLen && ((c = *urn) == ' ' || c == '\r' ||
                     c == '\n' || c == '\t'))
   {
      urn++;
      urnLen--;
   }

   /* remove trailing spaces */
   while (urnLen && ((c = urn[urnLen - 1]) == ' ' || c == '\r' ||
                     c == '\n' || c == '\t'))
   {
      urnLen--;
   }

   pNs->mLevel = pReader->mLevel;
   pNs->mPrefix.length = prefixLen;

   if (prefixLen <= sizeof (pNs->prefixBuff)) {
      OSCRTLSAFEMEMCPY (pNs->prefixBuff, sizeof(pNs->prefixBuff), prefix,
            prefixLen);
      pNs->mPrefix.value = pNs->prefixBuff;
   }
   else {
      OSUTF8CHAR* p =
         (OSUTF8CHAR*) rtxMemAlloc (pReader->mpCtxt, prefixLen);

      OSRTASSERT (p != 0);

      if (p) OSCRTLSAFEMEMCPY (p, prefixLen, prefix, prefixLen);

      pNs->mPrefix.value = p;
   }

   pNs->urn = 0;

   if (urnLen == 0) {
      pNs->index = (OSINT16)
         ((prefixLen > 0) ? OSXMLNSI_UNKNOWN : OSXMLNSI_UNQUALIFIED);
   }
   else if (urnLen == xsiUrnLen && memcmp (urn, xsiUrn, urnLen) == 0) {
      /* xsi namespace */
      pNs->index = OSXMLNSI_XSI;
   }
   else if (urnLen == soapEnvUrnLen && memcmp (urn, soapEnvUrn, urnLen) == 0)
   {
      /* SOAP envelope namespace */
      pNs->index = OSXMLNSI_SOAP_ENVELOPE;
   }
   else if (urnLen == soap12EnvUrnLen &&
            memcmp (urn, soap12EnvUrn, urnLen) == 0)
   {
      /* SOAP 1.2 envelope namespace */
      pNs->index = OSXMLNSI_SOAP_ENVELOPE;
   }
   else if (urnLen == xsdUrnLen && memcmp (urn, xsdUrn, urnLen) == 0) {
      /* XSD namespace */
      pNs->index = OSXMLNSI_XSD;
   }
   else {
      /*
      If the namespace is not found in the namespace table, then, from above,
      pNs->urn will be null. We thus lose knowledge of what uri the prefix
      was declared for. Note you cannot simply do pNs->urn = urn because urn
      may be temporary memory that will be deleted. You cannot just copy
      the data because we normally expect the urn to refer to the namespace
      table and therefore will never delete it. This makes it critical that
      all namespaces which we will need to recognize appear in the namespace
      table.
      */
      pNs->index = OSXMLNSI_UNKNOWN;
      /* find namespace index by urn */
      for (i = 0; i < pReader->mNamespacesNumber; i++, pUrn++) {
         if (rtxUTF8Strncmp (*pUrn, urn, urnLen) == 0 &&
             (*pUrn)[urnLen] == 0)
         {
            pNs->index = (OSINT16) (i + 1);
            pNs->urn = *pUrn;
            break;
         }
      }
   }
}

static void rtXmlRdPopNamespaces (OSXMLReader* pReader)
{
   while (pReader->mNamespacesStack.mCount) {
      OSXMLNamespace_* pNs = (OSXMLNamespace_*) rtXmlRdStackGetTop
         (pReader, &pReader->mNamespacesStack);

      if (pNs->mLevel > pReader->mLevel) {
         if (pNs->mPrefix.value != pNs->prefixBuff)
            rtxMemFreePtr (pReader->mpCtxt, pNs->mPrefix.value);
         rtXmlRdStackPop (pReader, &pReader->mNamespacesStack);
      }
      else break;
   }
}

static int rtXmlRdGetNamespaceIndex (OSXMLReader* pReader,
   const OSUTF8CHAR* prefix, size_t prefixLen)
{
   int i = (int) pReader->mNamespacesStack.mCount;

   if (prefixLen == 3 && (char) prefix[0] == 'x' &&
       (char) prefix[1] == 'm' && (char) prefix[2] == 'l')
      return OSXMLNSI_XML;

   while (i--) {
      OSXMLNamespace_* pNs = (OSXMLNamespace_*) rtXmlRdStackGetByIndex
         (pReader, &pReader->mNamespacesStack, i);

      if (pNs->mPrefix.length == prefixLen &&
          memcmp (pNs->mPrefix.value, prefix, prefixLen) == 0)
      {
         return pNs->index;
      }
   }

   if (prefixLen == 0) /* no namespace */
      return OSXMLNSI_UNQUALIFIED;

   /* prefix for XSI namespace prefix is not declared */
   if (prefixLen == 3 && (char) prefix[0] == 'x' &&
       (char) prefix[1] == 's' && (char) prefix[2] == 'i')
      return OSXMLNSI_XSI;

   if (rtxCtxtTestFlag (pReader->mpCtxt, OSXMLNOCMPNS))
      return OSXMLNSI_UNKNOWN;

   /* prefix is not defined */
   rtxErrAddStrnParm (pReader->mpCtxt, (const char*) prefix, prefixLen);
   rtXmlRdErrAddSrcPos (pReader, FALSE);
   rtXmlRdLogError (pReader, XML_E_NSURINOTFOU);

   return OSXMLNSI_UNKNOWN;
}

static void rtXmlRdProcessStartElement(OSXMLReader* pReader) {
   const OSUTF8CHAR* p = (const OSUTF8CHAR*)"";
   int i;

   if (pReader->mPrevNamespaceTable != pReader->mNamespaceTable) {
      /* remap prefixes to indexes in new name namespace table */
      int k = (int) pReader->mNamespacesStack.mCount;
      while (k--) {
         OSXMLNamespace_* pNs =
            (OSXMLNamespace_*) rtXmlRdStackGetByIndex
               (pReader, &pReader->mNamespacesStack, k);

         if (pNs->urn != 0) {
            pNs->index = OSXMLNSI_UNKNOWN;

            for (i = 0; i < pReader->mNamespacesNumber; i++) {
#ifdef OSXMLNS12
               const OSUTF8CHAR* nsUrn = pReader->mNamespaceTable[i];
               if (rtxUTF8StrEqual (pNs->urn, nsUrn)) {
#else
               const char* nsUrn = pReader->mNamespaceTable[i];
               if (OSCRTLSTRCMP (pNs->urn, nsUrn) == 0) {
#endif
                  pNs->index = (OSINT16) (i + 1);
                  break;
               }
            }
         }
      }

      pReader->mPrevNamespaceTable = pReader->mNamespaceTable;
      pReader->mPrevNamespacesNumber = pReader->mNamespacesNumber;
   }

   /* process xlmns attributes */
   for (i = 0; i < (int)pReader->mAttributes.mCount; i++) {
      OSXMLAttrOffset* pAttr = (OSXMLAttrOffset*) rtXmlRdStackGetByIndex
         (pReader, &pReader->mAttributes, i);

      OSRTASSERT (pAttr != 0);

      pAttr->index = OSXMLNSI_UNQUALIFIED;

      if (pAttr->mPrefix.length == 0) {
         if (pAttr->mLocalName.length == 5) {
            p = rtXmlRdGetPtr (pReader, pAttr->mLocalName.offset, 0);

            if (memcmp ("xmlns", p, 5) == 0) {
               /* xmlns="urn" */
               const OSUTF8CHAR* pValue =
                  rtXmlRdGetPtr (pReader, pAttr->mValue.offset, 0);

               rtXmlRdPushNamespace
                  (pReader, (const OSUTF8CHAR*)"", 0, pValue,
                   pAttr->mValue.length);

               pAttr->index = OSXMLNSI_XMLNS;
            }
         }
      }
      else if (pAttr->mPrefix.length == 5) {
         p = rtXmlRdGetPtr (pReader, pAttr->mPrefix.offset, 0);

         if (memcmp ("xmlns", p, 5) == 0) {
            /* xmlns:prefix="urn" */
            const OSUTF8CHAR* pValue =
               rtXmlRdGetPtr (pReader, pAttr->mValue.offset, 0);

            p = rtXmlRdGetPtr (pReader, pAttr->mLocalName.offset, 0);

            rtXmlRdPushNamespace (pReader, p, pAttr->mLocalName.length,
               pValue, pAttr->mValue.length);

            pAttr->index = OSXMLNSI_XMLNS;
         }
      }
   }
   /* process xlmns attributes */
   for (i = 0; i < (int)pReader->mNamespacesStack.mCount; i++) {
      OSXMLNamespace_* pNs = (OSXMLNamespace_*) rtXmlRdStackGetByIndex
         (pReader, &pReader->mNamespacesStack, i);

      if (pNs->mPrefix.length <= sizeof (pNs->prefixBuff) &&
          pNs->mPrefix.value != pNs->prefixBuff) {
         pNs->mPrefix.value = pNs->prefixBuff;
      }
   }

   /* get element name namespace index */
   if (pReader->mElementName.mPrefix.length)
      p = rtXmlRdGetPtr
         (pReader, pReader->mElementName.mPrefix.offset, 0);

   pReader->mElementNsIndex = (OSINT16) rtXmlRdGetNamespaceIndex
      (pReader, p, pReader->mElementName.mPrefix.length);
}

#define OSXMLSYM_LT    0
#define OSXMLSYM_QUE   1
#define OSXMLSYM_EXCL  2
#define OSXMLSYM_GT    3
#define OSXMLSYM_SLASH 4
#define OSXMLSYM_COLON 8
#define OSXMLSYM_DASH  9
#define OSXMLSYM_EQUAL 10
#define OSXMLSYM_QUOT  11
#define OSXMLSYM_ALPHA 5
#define OSXMLSYM_DIGIT 6
#define OSXMLSYM_SPACE 7
#define OSXMLSYM_AMP   12
#define OSXMLSYM_SEMIC 13
#define OSXMLSYM_PERIOD 14
#define OSXMLSYM_USCORE 15
#define OSXMLSYM_APOS 16
#define OSXMLSYM_OTHER 17

/**
 * Get the symbol index for the given symbol. (This is the index for the
 * symbol in all/most transition tables?)
 * @param pReader Used to log an invalid symbol, if symbol is invalid.
 * @param sym The symbol
 * @return The symbol index.  A negative value indicates an error.
*/
static int getSymbolIndex (OSXMLReader* pReader, int sym)
{
   int symbolIndex = XML_E_INVSYMBOL;

   switch (sym) {
      case '<': symbolIndex = OSXMLSYM_LT; break;
      case '?': symbolIndex = OSXMLSYM_QUE; break;
      case '!': symbolIndex = OSXMLSYM_EXCL; break;
      case '>': symbolIndex = OSXMLSYM_GT; break;
      case '/': symbolIndex = OSXMLSYM_SLASH; break;
      case ':': symbolIndex = OSXMLSYM_COLON; break;
      case '-': symbolIndex = OSXMLSYM_DASH; break;
      case '=': symbolIndex = OSXMLSYM_EQUAL; break;
      case '\"':symbolIndex = OSXMLSYM_QUOT; break;
      case '_': symbolIndex = OSXMLSYM_USCORE; break;
      case '&': symbolIndex = OSXMLSYM_AMP; break;
      case ';': symbolIndex = OSXMLSYM_SEMIC; break;
      case '.': symbolIndex = OSXMLSYM_PERIOD; break;
      case '\'':symbolIndex = OSXMLSYM_APOS; break;
      default:
         if (OS_ISALPHA (sym)) symbolIndex = OSXMLSYM_ALPHA;
         else if (OS_ISDIGIT (sym)) symbolIndex = OSXMLSYM_DIGIT;
         else if (OS_ISSPACE (sym)) symbolIndex = OSXMLSYM_SPACE;
         else if (sym < ' ') {
            rtXmlRdLogInvSymbol (pReader);
            symbolIndex = LOG_RTERRNEW (pReader->mpCtxt, XML_E_INVSYMBOL);
            pReader->mByteIndex++;
         }
         else if (sym == 0xFF || sym == 0xFE || // UTF-16 BOM
                  sym == 0xEF || sym == 0xBB || sym == 0xBF) // UTF-8 BOM
            symbolIndex = OSXMLSYM_OTHER;
         else if (sym > 0x7F)  symbolIndex = OSXMLSYM_ALPHA;
         else symbolIndex = OSXMLSYM_OTHER;
   }
   return symbolIndex;
}

static int getCDATASymbolIndex (OSXMLReader* pReader, int sym)
{
   int symbolIndex = -1;

   switch (sym) {
      case '<': symbolIndex = OSXMLSYM_LT; break;
      case '?': symbolIndex = OSXMLSYM_QUE; break;
      case '!': symbolIndex = OSXMLSYM_EXCL; break;
      case '>': symbolIndex = OSXMLSYM_GT; break;
      case '/': symbolIndex = OSXMLSYM_SLASH; break;
      case '[': symbolIndex = OSXMLSYM_COLON; break;
      case ']': symbolIndex = OSXMLSYM_DASH; break;
      case 'C': symbolIndex = OSXMLSYM_EQUAL; break;
      case 'D': symbolIndex = OSXMLSYM_QUOT; break;
      case 'A': symbolIndex = OSXMLSYM_AMP; break;
      case 'T': symbolIndex = OSXMLSYM_SEMIC; break;
      default:
         if (OS_ISSPACE (sym)) symbolIndex = OSXMLSYM_SPACE;
         else if (sym < ' ') {
            rtXmlRdLogInvSymbol (pReader);
            symbolIndex = LOG_RTERRNEW (pReader->mpCtxt, XML_E_INVSYMBOL);
            pReader->mByteIndex++;
         }
         else symbolIndex = OSXMLSYM_OTHER;
   }
   return symbolIndex;
}

EXTXMLMETHOD OSXMLEvent rtXmlRdGetLastEvent (OSXMLReader* pReader)
{
   OSRTASSERT (pReader != 0);
   pReader->mbHasAttributes = TRUE;
   return pReader->mLastEvent;
}

static void rtXmlRdUpdateSrcPos
(OSXMLSrcPos* pSrcPos, const OSUTF8CHAR* p, size_t sz)
{
   const OSUTF8CHAR* pEnd = p + sz;
   OSUINT32 column = pSrcPos->mColumn;
   OSUINT32 byteIdx = pSrcPos->mByteIdx;
   OSBOOL flCR = pSrcPos->mbCR;

   while (p != pEnd) {
      OSUTF8CHAR c = *p++;

      if (c == 0xD) {
         flCR = TRUE;
         pSrcPos->mLine++;
         byteIdx = column = 0;
      }
      else if (c == 0xA) {
         if (flCR == FALSE) {
            pSrcPos->mLine++;
            byteIdx = column = 0;
         }
         else
           flCR = FALSE;
      }
      else {
         flCR = FALSE;
         byteIdx++;

         if (c < 0x80 || (c >= 0xC0 && c <= 0xFD)) /* count UTF8 chars */
            column++;
      }
   }

   pSrcPos->mByteIdx = byteIdx;
   pSrcPos->mColumn = column;
   pSrcPos->mbCR = flCR;
}

/* Transport */
#define OSXML_BUFSIZE 50

#ifndef _NO_STREAM
static size_t latin1ToUTF8
(const OSOCTET* inbuf, size_t inlen, OSOCTET* outbuf);
#endif
#ifdef XBDEMO
#define XB_DEMO_MAX_MSG_LENGTH ((OSINT16)10240L)
static int msgLen = 0;
#endif

int rtXmlRdPreReadFromStream (OSXMLReader* pReader)
{
   size_t requiredCapacity = 0;
   OSCTXT* pctxt = pReader->mpCtxt;

   OSRTASSERT (pReader != 0 && pReader->mpCtxt != 0);

   if (pReader->mBufSize == 0) {
#ifndef _NO_STREAM
      if (OSRTISSTREAM (pctxt)) {
         pReader->mpBuffer = (OSOCTET*)
            rtxMemSysAlloc (pctxt, OSXML_BUFSIZE);
         pReader->mbSysMemBuf = TRUE;
      }
      else
#endif /* _NO_STREAM */
         pReader->mpBuffer = (OSOCTET*)
            rtxMemAlloc (pctxt, OSXML_BUFSIZE);

      if (pReader->mpBuffer == 0) {
         return RTERR_NOMEM;
      }

      pReader->mBufSize = OSXML_BUFSIZE;
      pReader->mByteIndex = 0;
      pReader->mReadSize = 0;
   }

   /* We need at least 2 bytes of capacity for BOM */
   if (pReader->mBOM != OSXMLBOM_NO_BOM) requiredCapacity += 2;

   /* If encoding is LATIN1, one more byte of capacity is needed (don't
      know why) */
   if (pReader->mEncoding == OSXMLLATIN1) requiredCapacity++;

   if ((pReader->mBufSize - pReader->mReadSize) <= requiredCapacity) {
      size_t markedPos = pReader->mMarkedPos;

      if (pReader->mRewindPosStack.mCount != 0) {
         OSXMLRewindPos* pPos = (OSXMLRewindPos*) rtXmlRdStackGetByIndex
            (pReader, &pReader->mRewindPosStack, 0);

         if (pPos->mPos != (size_t)-1)
            markedPos = pPos->mPos;
      }

      if (markedPos == (size_t)-1) {
         pReader->mReadSize = 0;
         pReader->mByteIndex = 0;
      }
      else {
         if (markedPos > 0) {
            int i;
            /* marked pos, move buffer content */
            size_t delta = pReader->mReadSize - markedPos;

            rtXmlRdUpdateSrcPos
               (&pReader->mSrcPos, pReader->mpBuffer, markedPos);

            if (delta > 0)
               OSCRTLSAFEMEMMOVE (pReader->mpBuffer, pReader->mBufSize,
                     pReader->mpBuffer + markedPos, delta);

            pReader->mByteIndex -= markedPos;
            pReader->mReadSize -= markedPos;
            pReader->mMarkedPos -= markedPos;

#ifndef _NO_STREAM
            if (OSRTISSTREAM (pctxt)) {
               if (pctxt->pStream->flags & OSRTSTRMF_POSMARKED) {
                 pctxt->pStream->nextMarkOffset += markedPos;
                 pReader->mLastBlockSize -= markedPos;
                 RTDIAG2 (pReader->mpCtxt, "nextMarkOffset = " OSSIZEFMT "\n",
                          pctxt->pStream->nextMarkOffset);
               }
            }
#endif /* _NO_STREAM */

            for (i = 0; i < (int)pReader->mRewindPosStack.mCount; i++) {
               OSXMLRewindPos* pPos = (OSXMLRewindPos*) rtXmlRdStackGetByIndex
                  (pReader, &pReader->mRewindPosStack, i);

               if (pPos->mPos != (size_t)-1)
                  pPos->mPos -= markedPos;
            }
         }
         else { /* need realloc buffer */
            size_t newBufSize = pReader->mBufSize + OSXML_BUFSIZE;
            OSOCTET* pnewBuf;

#ifndef _NO_STREAM
            if (OSRTISSTREAM (pctxt))
               pnewBuf = (OSOCTET*)
                  rtxMemSysRealloc (pctxt, pReader->mpBuffer, newBufSize);
            else
#endif /* _NO_STREAM */
               pnewBuf = (OSOCTET*)
                  rtxMemRealloc (pctxt, pReader->mpBuffer, newBufSize);

            if (pnewBuf == 0) {
               return rtXmlRdLogError (pReader, RTERR_NOMEM);
            }

            pReader->mpBuffer = pnewBuf;
            pReader->mBufSize = newBufSize;
         }
      }
   }

   if ((pReader->mEncoding != OSXMLLATIN1 &&
        pReader->mBufSize - pReader->mReadSize > 0) ||
       pReader->mBufSize - pReader->mReadSize > 1)
   {
#ifndef _NO_STREAM
      if (OSRTISSTREAM (pctxt)) {
         long read = 0;

         if (pReader->mbBackoffEnabled) {
            if (pctxt->pStream->flags & OSRTSTRMF_POSMARKED)
               pctxt->pStream->nextMarkOffset += pReader->mLastBlockSize;
            else {
               rtxStreamMark (pctxt, INT_MAX);
               pctxt->pStream->flags &= ~OSRTSTRMF_POSMARKED;
            }
         }

         if (pReader->mBOM == OSXMLBOM_CHECK) {
            read = rtxStreamRead
               (pctxt, pReader->mpBuffer + pReader->mReadSize, 2);
         }
         else if (pReader->mBOM == OSXMLBOM_UTF16_BE ||
                  pReader->mBOM == OSXMLBOM_UTF16_LE) {
            /* special processing for UTF-16 sequence:
             * Read two bytes from input stream,
             * then convert it to a UTF-8 sequence.
             * The UTF-8 sequence is saved to pReader buffer.
             */
            long l;
            OSUNICHAR pUniBuf[2];
            OSOCTET* pconvBuf;
            size_t convBufSize;
            OSOCTET* curP = pReader->mpBuffer + pReader->mReadSize;
            size_t availBufSize = pReader->mBufSize - pReader->mReadSize;
            OSOCTET pbuffer[2];

            pUniBuf[1] = '\0';

            l = rtxStreamRead (pctxt, pbuffer, 2);
            if (l < 0) return rtXmlRdLogError (pReader, (int)l);

            while (l > 0)
            {
               if ((pbuffer[0] == 0xFF && pbuffer[1] == 0xFE) ||
                   (pbuffer[0] == 0xFE && pbuffer[1] == 0xFF))
               {
                  l = rtxStreamRead (pctxt, pbuffer, 2);
                  continue;
               }

               if (pReader->mBOM == OSXMLBOM_UTF16_BE) {
                  pUniBuf[0] = (OSUNICHAR)((pbuffer[0]<<8) | pbuffer[1]);
               } else {
                  pUniBuf[0] = (OSUNICHAR)(pbuffer[0] | (pbuffer[1]<<8));
               }
               pconvBuf = (OSOCTET*) rtxUCSToDynUTF8
                      (pctxt, (const OSUNICHAR*) pUniBuf);
               convBufSize = rtxUCSGetUTF8ByteCnt (pUniBuf);

               if (availBufSize < convBufSize)
               { /* need realloc buffer */
                  size_t newBufSize = pReader->mBufSize + OSXML_BUFSIZE;
                  OSOCTET* pnewBuf = (OSOCTET*)
                     rtxMemSysRealloc (pctxt, pReader->mpBuffer, newBufSize);
                  if (pnewBuf == 0)
                     return rtXmlRdLogError (pReader, RTERR_NOMEM);

                  pReader->mpBuffer = pnewBuf;
                  pReader->mBufSize = newBufSize;
                  availBufSize += OSXML_BUFSIZE;
                  curP = pReader->mpBuffer + read + 2;
               }

               OSCRTLSAFEMEMCPY (curP, availBufSize, pconvBuf, convBufSize);
               curP += convBufSize;
               availBufSize -= convBufSize;
               read = read + (long)convBufSize;

               l = rtxStreamRead (pctxt, pbuffer, 2);
            }

            if (l < 0) return rtXmlRdLogError (pReader, (int)l);

            pReader->mLastByteIndex = pReader->mReadSize;
            pReader->mLastBlockSize = (size_t) read;
            pReader->mReadSize += (size_t)read;
         }
         else if (pReader->mEncoding == OSXMLLATIN1) {
            size_t halfBufSiz = (pReader->mBufSize - pReader->mReadSize)/2;
            OSOCTET* outBuf = pReader->mpBuffer + pReader->mReadSize;
            OSOCTET* inBuf = outBuf + halfBufSiz;
            size_t inBufSiz;
            inBufSiz = rtxStreamRead (pctxt, inBuf, halfBufSiz);

            read = (long) latin1ToUTF8 (inBuf, inBufSiz, outBuf);
         }
         else {
            read = rtxStreamRead
               (pReader->mpCtxt,
                pReader->mpBuffer + pReader->mReadSize,
                pReader->mBufSize - pReader->mReadSize);
         }
#ifdef XBDEMO
         msgLen += read;
         if (msgLen > XB_DEMO_MAX_MSG_LENGTH) {
            rtxErrAddStrParm (pctxt, "Max message length for demo exceeded.");
            return LOG_RTERR (pctxt, RTERR_NOTSUPP);
         }
#endif
         if (read <= 0) {
            int stat = (read == 0) ? RTERR_ENDOFBUF : read;
            return rtXmlRdLogError (pReader, stat);
         }

         if (pReader->mBOM != OSXMLBOM_UTF16_BE &&
             pReader->mBOM != OSXMLBOM_UTF16_LE) {
            pReader->mLastByteIndex = pReader->mReadSize;
            pReader->mLastBlockSize = (size_t) read;
            pReader->mReadSize += (size_t)read;
         }
      }
      else
#endif /* _NO_STREAM */
#ifdef _PULL_FROM_BUFFER
      if (OSRTBUFFER(pReader->mpCtxt)) {
         long read = 0;
         OSRTBuffer* buffer = &pReader->mpCtxt->buffer;

         /* todo: UTF16 to UTF8 conversion */
         read = (long) OSRTMIN (pReader->mBufSize - pReader->mReadSize,
                                buffer->size - buffer->byteIndex);
         if (read > 0) {
            OSCRTLSAFEMEMCPY (pReader->mpBuffer + pReader->mReadSize, read,
                    buffer->data + buffer->byteIndex, read);

            buffer->byteIndex += read;
#ifdef XBDEMO
            msgLen += read;
            if (msgLen > XB_DEMO_MAX_MSG_LENGTH) {
               rtxErrAddStrParm
                  (pctxt, "Max message length for demo exceeded.");
               return LOG_RTERR (pctxt, RTERR_NOTSUPP);
            }
#endif /* XBDEMO */
         }

         if (read <= 0) {
            return rtXmlRdLogError (pReader, RTERR_READERR);
         }
      }
#else /* _PULL_FROM_BUFFER */
      return rtXmlRdLogError (pReader, RTERR_READERR);
#endif /* _PULL_FROM_BUFFER */
   }
   else {
      return -1;
   }
   return 0;
}

/*
static int rtXmlRdMarkPosition (OSXMLReader* pReader)
{
   OSRTASSERT (pReader != 0);

   pReader->mMarkedPos = pReader->mByteIndex;
   return 0;
}
*/
#define rtXmlRdMarkPosition(pReader) \
pReader->mMarkedPos = pReader->mByteIndex

/*
static int rtXmlRdResetPosition (OSXMLReader* pReader)
{
   OSRTASSERT (pReader != 0);

   pReader->mByteIndex = pReader->mMarkedPos;
   return 0;
}
*/

static int getSymbol (OSXMLReader* pReader, OSBOOL peek)
{
   int sym;
   if (pReader->mByteIndex >= pReader->mReadSize) {
      int stat = rtXmlRdPreReadFromStream (pReader);
      if (stat != 0) {
         if (RTERR_ENDOFBUF == stat) {
            rtxErrResetLastErrors (pReader->mpCtxt, 1);
         }
         return -1;
      }
   }
   sym = pReader->mpBuffer [pReader->mByteIndex];
#ifdef XMLPTRACE
   if (OS_ISPRINT (sym)) {
      RTDIAG3 (pReader->mpCtxt, "getSymbol: sym is %d (%c)\n", sym, sym);
   }
   else {
      RTDIAG2 (pReader->mpCtxt, "getSymbol: sym is %d\n", sym);
   }
#endif
   if (!peek) pReader->mByteIndex++;

   return sym;
}

#define rtXmlRdGetOffset(pReader) \
((pReader->mMarkedPos != (size_t)-1)? \
pReader->mByteIndex - pReader->mMarkedPos : (size_t)-1)

#define rtXmlRdGetOffsetPtr(pReader,offset) \
((pReader->mMarkedPos != (size_t)-1)? \
pReader->mpBuffer + pReader->mMarkedPos + offset : 0)

#define rtXmlRdCancelMark(pReader) pReader->mMarkedPos = (size_t)-1;

#define rtXmlRdIsMarked(pReader) \
(pReader->mMarkedPos != (size_t)-1 && pReader->mMarkedPos < pReader->mReadSize)

#define READ_SYMBOL(pReader) getSymbol(pReader,FALSE)
#define PEEK_SYMBOL(pReader) getSymbol(pReader,TRUE)
#define NEED_PREREAD(pReader) (pReader->mByteIndex >= pReader->mReadSize)

/*
// -1 - header (<?...)
// -2 - comment (<!--...)
// -3 - start element (<a...)
// -4 - possible DTD
*/
static const OSINT8 initial_transitions [][18] = {
   /* columns - states, rows - input chars */
   /* <   ?   !   >   /  ch  nm  sp   :   -   =   "   &   ;   .   _   '  other */
   {  2,  0,  0,  0,  0,  0,  0,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1 }, /* st=1, initial */
   {  0, -1,  3,  0,  0, -3,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, /* st=2, initial after '<' */
   {  0,  0,  0,  0,  0, -4,  0,  0,  0,  4,  0,  0,  0,  0,  0,  0,  0,  0 }, /* st=3, '<!' recvd */
   {  0,  0,  0,  0,  0,  0,  0,  0,  0, -2,  0,  0,  0,  0,  0,  0,  0,  0 }, /* st=4, '<!-' recvd */
   {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }  /* st=5, '<!--' recvd */
};

static const OSINT8 stdoc_transitions [][18] = {
   /* columns - states, rows - input chars */
   /* <   ?   !   >   /  ch  nm  sp   :   -   =   "   &   ;   .   _   '  other */
   {  2,  0,  0,  0,  0,  0,  0,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, /* st=1, initial */
   {  0, -6,  3,  0,  0, -3,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, /* st=2, initial after '<' */
   {  0,  0,  0,  0,  0, -4,  0,  0,  0,  4,  0,  0,  0,  0,  0,  0,  0,  0 }, /* st=3, '<!' recvd */
   {  0,  0,  0,  0,  0,  0,  0,  0,  0, -2,  0,  0,  0,  0,  0,  0,  0,  0 }, /* st=4, '<!-' recvd */
   {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }  /* st=5, '<!--' recvd */
};
/*
// -5 - character data
// -6 - PI (<?ch)
// -7 - end element
// -8 - possible CDATA (<!other)
*/
static const OSINT8 main_transitions [][18] = {
   /* columns - states, rows - input chars */
   /* <   ?   !   >   /  ch  nm  sp   :   -   =   "   &   ;   .   _   '  other */
   {  2, -5, -5, -5, -5, -5, -5, -5, -5, -5, -5, -5, -5, -5, -5, -5, -5, -5 }, /* st=1, initial, by default chars */
   {  0, -6,  3,  0, -7, -3,  0,  0,  0,  0,  0,  0,  0,  0,  0, -3,  0,  0 }, /* st=2, '<' recvd */
   {  0,  0,  0,  0,  0, -4,  0,  0,  0,  4,  0,  0,  0,  0,  0,  0,  0, -8 }, /* st=3, '<!' recvd */
   {  0,  0,  0,  0,  0,  0,  0,  0,  0, -2,  0,  0,  0,  0,  0,  0,  0,  0 }, /* st=4, '<!-' recvd */

};

static int readLoop
(OSXMLReader* pReader, int localStateIdx, const OSINT8 transitions[][18],
 OSBOOL getAll)
{
   OSRTASSERT (pReader != 0);

   while (1) {
      int symbolIndex;
      int p;

      if (!getAll && NEED_PREREAD (pReader) && rtXmlRdIsMarked (pReader))
         break;

      p = PEEK_SYMBOL (pReader);
      if (p == -1) {
         return -1; /* OSXMLEVT_END_DOCUMENT or OSXMLEVT_NONE */
      }
      symbolIndex = getSymbolIndex (pReader, p);
      if (symbolIndex < 0) return symbolIndex;

      pReader->mLocalStates[localStateIdx] =
         transitions [pReader->mLocalStates[localStateIdx] - 1][symbolIndex];

      if (pReader->mLocalStates[localStateIdx] == 0) {
         return rtXmlRdLogInvSymbolError (pReader);
      }
      else if (pReader->mLocalStates[localStateIdx] < 0) {
         pReader->mLocalStates[localStateIdx] = 0;
         /* return last chunk */
         return 0;
      }
      pReader->mByteIndex++;
   }

   /* return chunk */
   return 1;
}

static OSUTF8CHAR* getEncodingStr (struct OSXMLReader* pReader);

/*
// returns:
// 0 - finished
// 1 - need continue
// -1 - error
*/
static int processHeader (OSXMLReader* pReader)
{
   static const OSINT8 transitions [][18] = {
      /* columns - states, rows - input chars */
      /* <   ?   !   >   /  ch  nm  sp   :   -   =   "   &   ;   .   _   '  other */
      {  1,  2,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 }, /* st=1, initial */
      {  1,  2,  1, -1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 }  /* st=2, '?' recvd */
   };
   int stat;

   OSRTASSERT (pReader != 0);

   rtXmlRdMarkPosition (pReader);

   if (pReader->mLocalStates[OSXMLSI_HEADER] == 0) {
      pReader->mLocalStates[OSXMLSI_HEADER] = 1;
   }

   stat = readLoop (pReader, OSXMLSI_HEADER, transitions, TRUE);
   if (stat == 0) {
      /* set encoding string */
      OSCTXT* pctxt = pReader->mpCtxt;
      OSUTF8CHAR* encodingStr = getEncodingStr (pReader);
      rtXmlSetEncodingStr (pctxt, encodingStr);

      pReader->mByteIndex++;
      pReader->mLastEvent.mId = OSXMLEVT_START_DOCUMENT;
      LCHECKX (pctxt);
   }
   return stat;
}

/*
// returns:
// 0 - finished
// 1 - need continue
// -1 - error
*/
static int processPI (OSXMLReader* pReader)
{
   static const OSINT8 transitions [][18] = {
      /* columns - states, rows - input chars */
      /* <   ?   !   >   /  ch  nm  sp   :   -   =   "   &   ;   .   _   '  other */
      {  1,  2,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 }, /* st=1, initial */
      {  1,  2,  1, -1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 }  /* st=2, '?' recvd */
   };
   int stat;

   OSRTASSERT (pReader != 0);

   rtXmlRdMarkPosition (pReader);

   if (pReader->mLocalStates[OSXMLSI_PI] == 0) {
      pReader->mLocalStates[OSXMLSI_PI] = 1;
   }

   stat = readLoop (pReader, OSXMLSI_PI, transitions, FALSE);
   if (stat >= 0) {
      if (stat == 0)
         pReader->mByteIndex++;
      pReader->mLastEvent.mId = OSXMLEVT_PI;
   }

   return stat;
}

/*
// returns:
// 0 - finished
// 1 - need continue
// -1 - error
*/
static int processComments (OSXMLReader* pReader)
{
   static const OSINT8 transitions [][18] = {
      /* columns - states, rows - input chars */
      /* <   ?   !   >   /  ch  nm  sp   :   -   =   "   &   ;   .   _   '  other */
      {  1,  1,  1,  1,  1,  1,  1,  1,  1,  2,  1,  1,  1,  1,  1,  1,  1,  1 }, /* st=1, initial */
      {  1,  1,  1,  1,  1,  1,  1,  1,  1,  3,  1,  1,  1,  1,  1,  1,  1,  1 }, /* st=2, '-' recvd */
      {  0,  0,  0, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }  /* st=3, '--' recvd */
   };
   int stat;

   OSRTASSERT (pReader != 0);

   rtXmlRdMarkPosition (pReader);

   if (pReader->mLocalStates[OSXMLSI_COMMENT] == 0) {
      pReader->mLocalStates[OSXMLSI_COMMENT] = 1;
      pReader->mByteIndex++;
   }

   stat = readLoop (pReader, OSXMLSI_COMMENT, transitions, FALSE);
   if (stat >= 0) {
      if (stat == 0) { pReader->mByteIndex++; }
      pReader->mLastEvent.mId = OSXMLEVT_COMMENT;
      pReader->mData.length = rtXmlRdGetOffset (pReader);
      pReader->mData.value = rtXmlRdGetPtr (pReader, 0, &pReader->mData.length);
   }

   return stat;
}

/*
// returns:
// 0 - finished
// 1 - need continue
// -1 - error
*/
static int processStartElement (OSXMLReader* pReader)
{
   /*
   // -1 - final state, only name recvd, no attrs
   // -2 - final state, attrs recvd
   // -3 - final state, empty element w/o attrs
   // -4 - final state, empty element with attrs
   */
   static const OSINT8 transitions [][18] = {
      /* columns - states, rows - input chars */
      /* <   ?   !   >   /  ch  nm  sp   :   -   =   "   &   ;   .   _   '  other */
      {  0,  0,  0,  0,  0,  2,  0,  0,  3,  0,  0,  0,  0,  0,  0,  2,  0,  0 }, /* st=1, initial */
      {  0,  0,  0, -1, 14,  2,  2,  5,  3,  2,  0,  0,  0,  0,  2,  2,  0,  0 }, /* st=2, first sym recvd */
      {  0,  0,  0,  0,  0,  4,  0,  0,  0,  0,  0,  0,  0,  0,  0,  4,  0,  0 }, /* st=3, ':' recvd */
      {  0,  0,  0, -1, 14,  4,  4,  5,  0,  4,  0,  0,  0,  0,  4,  4,  0,  0 }, /* st=4, sym after ':' recvd */

      {  0,  0,  0, -1, 14,  6,  0,  5,  7,  0,  0,  0,  0,  0,  0,  6,  0,  0 }, /* st=5, space after name recvd */
      {  0,  0,  0,  0,  0,  6,  6,  9,  7,  6, 10,  0,  0,  0,  6,  6,  0,  0 }, /* st=6, first sym for attr recvd */
      {  0,  0,  0,  0,  0,  8,  0,  0,  0,  0,  0,  0,  0,  0,  0,  8,  0,  0 }, /* st=7, ':' sym for attr recvd */
      {  0,  0,  0,  0,  0,  8,  8,  9,  0,  8, 10,  0,  0,  0,  8,  8,  0,  0 }, /* st=8, sym after ':' for attr recvd */

      {  0,  0,  0,  0,  0,  0,  0,  9,  0,  0, 10,  0,  0,  0,  0,  0,  0,  0 }, /* st=9, space after attr name recvd */
      {  0,  0,  0,  0,  0,  0,  0, 11,  0,  0,  0, 12,  0,  0,  0,  0, 16,  0 }, /* st=10, '=' recvd */
      {  0,  0,  0,  0,  0,  0,  0, 11,  0,  0,  0, 12,  0,  0,  0,  0, 17,  0 }, /* st=11, space after '=' recvd */

      { 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 13, 12, 12, 12, 12, 12, 12 }, /* st=12, first '"' recvd */
      {  0,  0,  0, -2, 15,  6,  0, 13,  7,  0,  0,  0,  0,  0,  0,  6,  0,  0 }, /* st=13, closing '"' recvd */

      {  0,  0,  0, -3,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, /* st=14, '/' recvd, no attrs */
      {  0,  0,  0, -4,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, /* st=15, '/' recvd, attrs */

      { 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 13, 16 }  /* st=16, first ' recvd */
   };
   size_t localNameOffset = 0;
   size_t attrQNameOffset = 0, attrNameOffset = 0, attrValueOffset = 0;

   OSRTASSERT (pReader != 0);
#ifdef XMLPTRACE
   RTDIAG1 (pReader->mpCtxt, "processStartElement: start\n");
#endif
   if (pReader->mLocalStates[OSXMLSI_START_ELEMENT] == 0) {
      pReader->mLocalStates[OSXMLSI_START_ELEMENT] = 1;
      rtXmlRdMarkPosition (pReader);
   }

   for (; pReader->mLastEvent.mId == OSXMLEVT_NONE &&
           pReader->mLocalStates[OSXMLSI_START_ELEMENT] > 0; ) {
      int symbolIndex, prevState, state;
      int p;

      prevState = pReader->mLocalStates[OSXMLSI_START_ELEMENT];

      p = PEEK_SYMBOL (pReader);
      if (p == -1) {
         return OSXMLEVT_END_DOCUMENT;
      }
      symbolIndex = getSymbolIndex (pReader, p);
      if (symbolIndex < 0) return symbolIndex;

      state = pReader->mLocalStates[OSXMLSI_START_ELEMENT] =
         transitions [prevState - 1][symbolIndex];

      if (state == 0) {
         return rtXmlRdLogInvSymbolError (pReader);
      }
      if (prevState != state) {
         switch (state) {
            case 4:  /* sym after ':' recvd for name */
               localNameOffset = rtXmlRdGetOffset (pReader);
               break;
            case -1: /* '>' recvd, no attrs */
               pReader->mLastEvent.mId = OSXMLEVT_START_TAG;
            case 14: /* '/' recvd, no attrs */
            case 5:  /* space recvd */
               {
               pReader->mDataMode = OSXMLDM_NONE;
               pReader->mbEmptyElement = FALSE;

               if (prevState != 5) {
                  /* qname */

                  pReader->mElementName.mQName.offset = 0;
                  rtXmlRdGetPtr (pReader, 0, &pReader->mElementName.mQName.length);

                  /* prefix */

                  if (localNameOffset > 0) {
                     pReader->mElementName.mPrefix.length = localNameOffset - 1;
                     pReader->mElementName.mPrefix.offset =
                        pReader->mElementName.mQName.offset;
                  }
                  else {
                     pReader->mElementName.mPrefix.offset = 0;
                     pReader->mElementName.mPrefix.length = 0;
                  }

                  /* local name */

                  pReader->mElementName.mLocalName.offset = localNameOffset;
                  rtXmlRdGetPtr (pReader, localNameOffset,
                                 &pReader->mElementName.mLocalName.length);

                  pReader->mLevel++;
               }

               if (state == -1) /* '>' recvd, no attrs */
                  pReader->mLastEvent.mLevel = pReader->mLevel;
               } break;
            case 6: /* attribute name, beginning */
               /*rtXmlRdMarkPosition (pReader);*/
               attrNameOffset = rtXmlRdGetOffset (pReader);
               attrQNameOffset = attrNameOffset;
               break;
            case 7: /* ':' in attribute name */
               if (prevState != 6) { /* if 6 - then ':' is not first sym */
                  /*rtXmlRdMarkPosition (pReader);*/
                  attrNameOffset = rtXmlRdGetOffset (pReader);
               }
               break;
            case 8: /* unqualified attribute name, beginning */
               attrNameOffset = rtXmlRdGetOffset (pReader);
               break;
            case 9:
            case 10: /* attribute name, ending (= recvd) */
               if (prevState != 9) {
                  OSXMLAttrOffset* pAttr = (OSXMLAttrOffset*)
                     rtXmlRdStackPushNew (pReader, &pReader->mAttributes);

                  OSRTASSERT (pAttr != 0);

                  /* attr qname */

                  pAttr->mQName.offset = attrQNameOffset;
                  rtXmlRdGetPtr (pReader, attrQNameOffset,
                                 &pAttr->mQName.length);

                  /* attr prefix */

                  if (attrNameOffset != attrQNameOffset) {
                     pAttr->mPrefix.length =
                        attrNameOffset - attrQNameOffset - 1;
                     pAttr->mPrefix.offset = attrQNameOffset;
                  }
                  else {
                     pAttr->mPrefix.length = 0;
                     pAttr->mPrefix.offset = 0;
                  }

                  /* attr local name */

                  rtXmlRdGetPtr (pReader, attrNameOffset,
                                 &pAttr->mLocalName.length);
                  pAttr->mLocalName.offset = attrNameOffset;

                  pAttr->index = OSXMLNSI_UNQUALIFIED;
               }
               break;

            case 12: /* attribute value, beginning */
            case 16:
               /* skip " */
               pReader->mbLastChunk = FALSE;
               pReader->mByteIndex++;
               attrValueOffset = rtXmlRdGetOffset (pReader);

               /* revert back */
               pReader->mByteIndex--;
               break;

            case 13: /* attribute value, ending */
               {
               OSXMLAttrOffset* pAttr = (OSXMLAttrOffset*)
                  rtXmlRdStackGetTop (pReader, &pReader->mAttributes);

               OSRTASSERT (pAttr != 0);

               rtXmlRdGetPtr
                  (pReader, attrValueOffset, &pAttr->mValue.length);

               pAttr->mValue.offset = attrValueOffset;
               } break;

            case -3: /* '/>' of stElem recvd w/o attrs */
            case -4: /* '/>' of stElem recvd after attrs */
               rtXmlRdAddDelayedTask (pReader, OSXMLDT_END_ELEMENT_EVENT);
               pReader->mbEmptyElement = TRUE;

            case -2: /* '>' of stElem recvd after attrs */
               pReader->mLastEvent.mId = OSXMLEVT_START_TAG;
               pReader->mLastEvent.mLevel = pReader->mLevel;
               break;

         }
      }
      if (pReader->mLocalStates[OSXMLSI_START_ELEMENT] < 0) {
         if (pReader->mLastEvent.mId == OSXMLEVT_START_TAG) {
            rtXmlRdAddDelayedTask (pReader, OSXMLDT_START_ELEMENT_CLEANUP);

            pReader->mElementNsIndex = OSXMLNSI_UNCHECKED; /* not checked */
            pReader->mbDecodeAsGroup = FALSE;

#ifndef _OSXML_NO_ENDTAG_VALIDATION
            if (!pReader->mbEmptyElement) {

               /* put tag qname into the stack in order to validate it
                  against end tag */

               OSXMLStrFragment* pQName = (OSXMLStrFragment*)
                  rtXmlRdStackPushNew (pReader, &pReader->mTagNamesStack);

               OSRTASSERT (pQName != 0);

               pQName->length = pReader->mElementName.mQName.length;
               rtXmlRdDupStr (pReader, pQName,
                  rtXmlRdGetPtr (pReader, pReader->mElementName.mQName.offset, 0));
            }
#endif /* _OSXML_NO_ENDTAG_VALIDATION */
         }

         pReader->mByteIndex++;
         pReader->mLocalStates[OSXMLSI_START_ELEMENT] = 0;
         return 0;
      }
      pReader->mByteIndex++;
   }
#ifdef XMLPTRACE
   RTDIAG2 (pReader->mpCtxt, "pReader->mLastEvent.mId is %d\n",
            pReader->mLastEvent.mId);
   RTDIAG2 (pReader->mpCtxt, "pReader->mLastEvent.mLevel is %d\n",
            pReader->mLastEvent.mLevel);
   RTDIAG2 (pReader->mpCtxt, "pReader->mElementName.mLocalName.offset is %d\n",
            pReader->mElementName.mLocalName.offset);
   RTDIAG2 (pReader->mpCtxt, "pReader->mElementName.mLocalName.length is %d\n",
            pReader->mElementName.mLocalName.length);
#endif
   return 1;
   /* return readLoop (pReader, OSXMLSI_START_ELEMENT, transitions); */
}

/*
// returns:
// 0 - finished
// 1 - need continue
// -1 - error
*/
static int processEndElement (OSXMLReader* pReader)
{
   static const OSINT8 transitions [][18] = {
      /* columns - states, rows - input chars */
      /* <   ?   !   >   /  ch  nm  sp   :   -   =   "   &   ;   .   _   '  other */
      {  0,  0,  0,  0,  0,  2,  0,  0,  3,  0,  0,  0,  0,  0,  0,  2,  0,  0 }, /* st=1, initial */
      {  0,  0,  0, -1,  0,  2,  2,  5,  3,  2,  0,  0,  0,  0,  2,  2,  0,  0 }, /* st=2, first sym recvd */
      {  0,  0,  0,  0,  0,  4,  0,  0,  0,  0,  0,  0,  0,  0,  0,  4,  0,  0 }, /* st=3, ':' recvd */
      {  0,  0,  0, -1,  0,  4,  4,  5,  0,  4,  0,  0,  0,  0,  4,  4,  0,  0 }, /* st=4, sym after ':' recvd */

      {  0,  0,  0, -1,  0,  0,  0,  5,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }  /* st=5, space after name recvd */
   };
   size_t lastCharOffset = (size_t)-1;

   OSRTASSERT (pReader != 0);

   if (pReader->mLocalStates[OSXMLSI_END_ELEMENT] == 0) {
      /* first enter */
      pReader->mLocalStates[OSXMLSI_END_ELEMENT] = 1;
      pReader->mByteIndex++; /* skip '/' */
      rtXmlRdMarkPosition (pReader);
      pReader->mElementName.mLocalName.offset = 0;
      pReader->mElementNsIndex = OSXMLNSI_UNKNOWN;
   }

   for (; pReader->mLastEvent.mId == OSXMLEVT_NONE &&
           pReader->mLocalStates[OSXMLSI_END_ELEMENT] > 0; ) {
      int symbolIndex, prevState, state;
      int p;

      prevState = pReader->mLocalStates[OSXMLSI_END_ELEMENT];

      p = PEEK_SYMBOL (pReader);
      if (p == -1) {
         return OSXMLEVT_END_DOCUMENT; /*OSXMLEVT_NONE;*/
      }
      symbolIndex = getSymbolIndex (pReader, p);
      if (symbolIndex < 0) return symbolIndex;

      state = pReader->mLocalStates[OSXMLSI_END_ELEMENT] =
         transitions [prevState - 1][symbolIndex];

      if (state == 0) {
         return rtXmlRdLogInvSymbolError (pReader);
      }
      if (prevState != state) {
         switch (state) {
            case 5:  /* space after name */
               lastCharOffset = rtXmlRdGetOffset (pReader);
               break;

            case 4:  /* sym after ':' */
               pReader->mElementName.mLocalName.offset =
                  rtXmlRdGetOffset (pReader);
               break;

            case -1: {
#ifndef _OSXML_NO_ENDTAG_VALIDATION
               OSXMLStrFragment endTagName;
#endif

               if (lastCharOffset == (size_t)-1) {
                  lastCharOffset = rtXmlRdGetOffset (pReader);
               }

               pReader->mElementName.mLocalName.length =
                  lastCharOffset - pReader->mElementName.mLocalName.offset;

#ifndef _OSXML_NO_ENDTAG_VALIDATION
               endTagName.value = rtXmlRdGetPtr (pReader, 0, 0);
               endTagName.length = lastCharOffset;

               /* match end tag with start tag here,
                  pop the last tag name from the mTagNamesStack */
               {
               OSXMLStrFragment* pQName = (OSXMLStrFragment*)
                  rtXmlRdStackPop (pReader, &pReader->mTagNamesStack);

               /* Stack empty, more close tags than start tags. */
               if (0 == pQName) {
                  rtxErrAddStrnParm (pReader->mpCtxt, "none", 4);

                  rtxErrAddStrnParm (pReader->mpCtxt,
                     (const char*)endTagName.value, endTagName.length);

                  rtXmlRdErrAddSrcPos (pReader, TRUE);
                  return rtXmlRdLogError (pReader, XML_E_TAGMISMATCH);
               }

               if (!rtXmlRdStrEqual(pQName, &endTagName)) {
                  /* error handling */
                  rtXmlRdFreeStr (pReader, pQName);
                  endElement (pReader);
                  pReader->mByteIndex++;
                  pReader->mLocalStates[OSXMLSI_END_ELEMENT] = 0;

                  rtxErrAddStrnParm (pReader->mpCtxt,
                     (const char*)pQName->value, pQName->length);

                  rtxErrAddStrnParm (pReader->mpCtxt,
                     (const char*)endTagName.value, endTagName.length);

                  rtXmlRdErrAddSrcPos (pReader, FALSE);
                  return rtXmlRdLogError (pReader, XML_E_TAGMISMATCH);
               }
               else
                  rtXmlRdFreeStr (pReader, pQName);
               }
#endif /* _OSXML_NO_ENDTAG_VALIDATION */

               endElement (pReader);
               break;
            }
         }
      }
      if (pReader->mLocalStates[OSXMLSI_END_ELEMENT] < 0) {
         pReader->mByteIndex++;
         pReader->mLocalStates[OSXMLSI_END_ELEMENT] = 0;
         return 0;
      }
      pReader->mByteIndex++;
   }

   return 1;
}


/**
 * processCharacters is called repeatedly to collect the text content of an
 * element.  The text is returned to the caller via pReader->mData.
 *
 * @returns
 *  0  : The end of the text content has been reached.
 *  1  : Call processCharacters again to get more of the text content.
 * < 0 : error
 * OSXMLEVT_END_DOCUMENT : end of document (unexpectedly) reached
 */
static int processCharacters (OSXMLReader* pReader)
{
   /* Between calls, pReader->mLocalStates[OSXMLSI_CONTENT] is used to remember
      this function's local state machine's state.  For the first call for a
      given text content (which may follow either a start or end tag), this
      state begins as zero.  When this function returns 0 (signalling the end
      of text content due to finding a tag), the saved state is reset to zero.

      Line feed normalization can remove characters from the inpput.  Since we
      return text by pointing into the input buffer, this makes it necessary
      to chunk the input text where normalization is done (at least when
      CRLF -> LF, but we might do it at other times too).  This also means that
      correctly implementing OSXMLNOLBLANKS is hopeless without using unbounded
      lookahead, because you must decide at the end of a chunk whether the
      whitespace is "inside" or "outside" an element, but that can't be
      known without knowing what the next tag is.
   */

   /* transition table.
      Entries are as follows:
      > 0 : the state to transition to
       0  : error
      -1  : '<' followed by something other than '/'
      -2  : '<' followed by '/'.

      I don't know why -1 and -2 are distinguished.  The behavior appears to
      be the same except that for </ we set mbLastChunk to true.
   */
   static const OSINT8 transitions [][18] = {
      /* columns - states, rows - input chars */
      /* <   ?   !   >   /  ch  nm  sp   :   -   =   "   &   ;   .   _   '  other */
      {  2,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  3,  1,  1,  1,  1,  1 }, /* st=1, initial */
      {  0, -1, -1, -1, -2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 }, /* st=2, '<' recvd */
      {  0,  0,  0,  0,  0,  3,  3,  0,  0,  0,  0,  0,  0,  4,  0,  0,  0,  3 }, /* st=3, '&' recvd */
      {  2,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  3,  1,  1,  1,  1,  1 }, /* st=4, ';' recvd aft '&' */
      /* whitespace collapse processing states */
      {  2,  6,  6,  6,  6,  6,  6,  5,  6,  6,  6,  6,  7,  6,  6,  6,  6,  6 }, /* st=5, skip leading spaces */
      {  2,  6,  6,  6,  6,  6,  6, 10,  6,  6,  6,  6,  8,  6,  6,  6,  6,  6 }, /* st=6, initial */
      {  0,  0,  0,  0,  0,  7,  7,  0,  0,  0,  0,  0,  0,  9,  0,  0,  0,  7 }, /* st=7, '&' recvd aft 5*/
      {  0,  0,  0,  0,  0,  8,  8,  0,  0,  0,  0,  0,  0,  9,  0,  0,  0,  8 }, /* st=8, '&' recvd aft 6*/
      {  2,  6,  6,  6,  6,  6,  6, 10,  6,  6,  6,  6,  8,  6,  6,  6,  6,  6 }, /* st=9, ';' recvd aft '&' */
      {  2,  6,  6,  6,  6,  6,  6, 10,  6,  6,  6,  6, 11,  6,  6,  6,  6,  6 }, /* st=10, space recvd */
      {  0,  0,  0,  0,  0, 11, 11,  0,  0,  0,  0,  0,  0,  9,  0,  0,  0, 11 }, /* st=11, '&' recvd aft 10 */
      /* #xD received; dummy state */
      {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }  /* st=12, CR recvd */
   };
   size_t lastDataOffset = 0, escapeStartOffset = 0;

   OSRTASSERT (pReader != 0);

   if (pReader->mLocalStates[OSXMLSI_CONTENT] == 0) {
      pReader->mLocalStates[OSXMLSI_CONTENT] =
         (pReader->mWhiteSpaceMode == OSXMLWSM_COLLAPSE) ? 5 : 1;
      pReader->mbLastChunk = FALSE;
      pReader->mbCDATA = FALSE;
   }

   rtXmlRdMarkPosition (pReader);

   for (; pReader->mLastEvent.mId == OSXMLEVT_NONE &&
           pReader->mLocalStates[OSXMLSI_CONTENT] > 0; ) {
      int symbolIndex, prevState, state;
      int p;

      prevState = pReader->mLocalStates[OSXMLSI_CONTENT];

      if (NEED_PREREAD (pReader) && rtXmlRdIsMarked (pReader)) {
         switch (prevState) {
            case 1: /* content value */
            case 6: {
               pReader->mLastEvent.mLevel = pReader->mLevel;
               pReader->mData.value =
                  rtXmlRdGetPtr (pReader, 0, &pReader->mData.length);

               pReader->mLastEvent.mId = OSXMLEVT_TEXT;

               rtXmlRdMarkPosition (pReader);
               pReader->mbLastChunk = FALSE;

               /*rtXmlRdCancelMark (pReader);*/
               continue;      /* loop will break if we set
                                 pReader->mLastEvent.mId above */
            }
         }
      }
      p = PEEK_SYMBOL (pReader);
      if (p == -1) {
         return OSXMLEVT_END_DOCUMENT;
      }

      if (p == '\r' && (prevState == 1 || prevState == 4)) {
         if (rtXmlRdGetOffset (pReader) > 0) {
            /* return already processed data */
            pReader->mLocalStates[OSXMLSI_CONTENT] = 12;

            pReader->mData.value =
               rtXmlRdGetPtr (pReader, 0, &pReader->mData.length);

            pReader->mLastEvent.mId = OSXMLEVT_TEXT;
            pReader->mLastEvent.mLevel = pReader->mLevel;
            pReader->mByteIndex++; /* skip #xD */
            return 1;
         }
         else {
            pReader->mByteIndex++; /* skip #xD */
            p = PEEK_SYMBOL (pReader);
            if (p == '\r') { /* for special CRCRLF line ending */
               pReader->mByteIndex++;
            }
            rtXmlRdMarkPosition (pReader);

            p = PEEK_SYMBOL (pReader);
            if (p == -1) {
               return OSXMLEVT_END_DOCUMENT;
            }

            prevState = 12;
         }
      }

      if (prevState == 12) {
         /* Shouldn't setting the state here take into account the whitespace
            mode? */
         state = pReader->mLocalStates[OSXMLSI_CONTENT] =
            (p == '\r') ? 12 : 1;

         if (p != '\n') {
            /* return #xA char instead of #xD */
            if (pReader->mWhiteSpaceMode == OSXMLWSM_PRESERVE)
               pReader->mCharBuf[0] = '\n';
            else
               pReader->mCharBuf[0] = ' ';

            pReader->mLastEvent.mId = OSXMLEVT_TEXT;
            pReader->mLastEvent.mLevel = pReader->mLevel;
            pReader->mData.length = 1;
            pReader->mData.value = pReader->mCharBuf;
            return 1;
         }

         symbolIndex = getSymbolIndex (pReader, p);
         if (symbolIndex < 0) return symbolIndex;
      }
      else {
         symbolIndex = getSymbolIndex (pReader, p);
         if (symbolIndex < 0) return symbolIndex;

         state = pReader->mLocalStates[OSXMLSI_CONTENT] =
            transitions [pReader->mLocalStates[OSXMLSI_CONTENT] - 1][symbolIndex];
#if 0
         if ((state == 1 || state == 6) && symbolIndex != OSXMLSYM_SPACE &&
             pReader->mbWSOnlyContext)
         {
            /* log error and skip other part */
            rtXmlRdLogInvSymbolError (pReader);
            pReader->mbWSOnlyContext = FALSE;
         }
#endif
      }

/* YG - space replacement moved to rtXmlpDec... */
#ifdef _PULL_FROM_BUFFER
      if (symbolIndex == OSXMLSYM_SPACE &&
            pReader->mWhiteSpaceMode != OSXMLWSM_PRESERVE)
         pReader->mpBuffer[pReader->mByteIndex] = ' ';
#endif

      if (state == 0) {
         /* TODO: handle errors */
         return rtXmlRdLogInvSymbolError (pReader);
      }

      if (prevState != state) {
         if (prevState == 5 || prevState == 10) /* skip spaces */
            rtXmlRdMarkPosition (pReader);

         switch (state) {
            case 2: { /* '<' recvd */
               lastDataOffset = rtXmlRdGetOffset (pReader);
               pReader->mbLastChunk = FALSE;
               break;
            }

            case 6: { /* token begin */
               if (prevState == 10 && !pReader->mbListMode) {
                  pReader->mCharBuf[0] = ' ';
                  pReader->mLastEvent.mId = OSXMLEVT_TEXT;
                  pReader->mLastEvent.mLevel = pReader->mLevel;
                  pReader->mData.length = 1;
                  pReader->mData.value = pReader->mCharBuf;
                  return 1;
               }
               break;
            }

            case 11:
            { /* '&' recvd after space in collapse mode */
               if (prevState == 10 && !pReader->mbListMode) {
                  pReader->mCharBuf[0] = ' ';
                  pReader->mLastEvent.mId = OSXMLEVT_TEXT;
                  pReader->mLastEvent.mLevel = pReader->mLevel;
                  pReader->mData.length = 1;
                  pReader->mData.value = pReader->mCharBuf;
                  pReader->mLocalStates[OSXMLSI_CONTENT] = 6;
                  return 1;
               }
            }
            case 3:  /* '&' recvd */
            case 7:
            case 8: {
               if (!pReader->mbNoTransform) {
                  if (rtXmlRdGetOffset (pReader) > 0) {

                     /* return already processed data before processing
                        escape char */

                     pReader->mData.value =
                        rtXmlRdGetPtr (pReader, 0, &pReader->mData.length);

                     pReader->mLastEvent.mId = OSXMLEVT_TEXT;
                     pReader->mLastEvent.mLevel = pReader->mLevel;
                     pReader->mLocalStates[OSXMLSI_CONTENT] = prevState; /* why is prevState what we save?? */
                     return 1;
                  }
               }

               escapeStartOffset = rtXmlRdGetOffset (pReader);
               break;
            }
            case 4:  /* ';' after '&' recvd */
            case 9: {
               if (!pReader->mbNoTransform) {
                  const OSUTF8CHAR* escapePtr;
                  size_t escapeLen;
                  OSXMLStrFragment destStr;

                  escapeLen = rtXmlRdGetOffset (pReader) - escapeStartOffset;
                  escapePtr = rtXmlRdGetPtr (pReader, escapeStartOffset, 0);

                  /* need transformation of escape symbol */
                  if (!transformEscapeChar (escapePtr, escapeLen, &destStr,
                                            pReader->mCharBuf + 1))
                  {
                     /* error. XML file is not well-formed */
                     rtxErrAddStrnParm
                        (pReader->mpCtxt, (const char*) escapePtr, escapeLen);
                     rtXmlRdErrAddSrcPos (pReader, TRUE);
                     return rtXmlRdLogError (pReader, XML_E_BADCHARREF);
                  }

                  pReader->mLastEvent.mLevel = pReader->mLevel;
                  pReader->mData.length = destStr.length;
                  pReader->mData.value = destStr.value;
                  pReader->mLastEvent.mId = OSXMLEVT_TEXT;
#if 0
                  if (pReader->mbWSOnlyContext &&
                      !OS_ISSPACE (destStr.value[0]))
                     return rtXmlRdLogInvSymbolError (pReader);
#endif
                  if (pReader->mWhiteSpaceMode == OSXMLWSM_PRESERVE) {
                     pReader->mByteIndex++; /* skip ; */
                     return 1;
                  }
                  else if (pReader->mWhiteSpaceMode == OSXMLWSM_REPLACE) {
                     if (destStr.length == 1 && OS_ISSPACE (destStr.value[0]))
                        pReader->mCharBuf[1] = ' ';

                     pReader->mByteIndex++; /* skip ; */
                     return 1;
                  }
                  else { /* OSXMLWSM_COLLAPSE */
                     if (destStr.length == 1 && OS_ISSPACE (destStr.value[0]))
                     {
                        if (prevState == 8) {
                           /* space char after token */

                           if (pReader->mbListMode) {
                              pReader->mData.length = 0;
                              pReader->mbLastChunk = TRUE;
                              /* go to skip ws */
                              state = pReader->mLocalStates[OSXMLSI_CONTENT] = 5;
                              pReader->mByteIndex++; /* skip ; */
                              return 1;
                           }
                           else
                              state = pReader->mLocalStates[OSXMLSI_CONTENT] = 10;
                        }
                        else if (prevState == 7)
                           state = pReader->mLocalStates[OSXMLSI_CONTENT] = 5;
                        else
                           state = pReader->mLocalStates[OSXMLSI_CONTENT] = 10;

                        pReader->mLastEvent.mId = OSXMLEVT_NONE;
                     }
                     else {
                        if (prevState == 11 && pReader->mbListMode) {
                           /* start of not first token; add token separator */

                           pReader->mCharBuf[0] = ' ';
                           pReader->mData.value = pReader->mCharBuf;
                           pReader->mData.length++;
                        }

                        pReader->mByteIndex++; /* skip ; */
                        return 1;
                     }
                  }
               }
               break;
            }

            case 10: {
               if (rtXmlRdGetOffset (pReader) > 0 || pReader->mbListMode) {
                  pReader->mData.value =
                     rtXmlRdGetPtr (pReader, 0, &pReader->mData.length);
                  pReader->mLastEvent.mId = OSXMLEVT_TEXT;
                  pReader->mLastEvent.mLevel = pReader->mLevel;
                  rtXmlRdMarkPosition (pReader);

                  if (pReader->mbListMode) {
                     pReader->mbLastChunk = TRUE;
                     /* go to skip ws */
                     state = pReader->mLocalStates[OSXMLSI_CONTENT] = 5;
                  }

                  return 1;
               }

               rtXmlRdMarkPosition (pReader);
               break;
            }

            case -2: /* '</' recvd */
               /* ok, end of element has been recvd.
                  assume, it was the last chunk */
               pReader->mbLastChunk = TRUE;

            case -1: { /* '<s' recvd */
               /* revert back */
               pReader->mByteIndex--;
               /*
               //rtXmlRdResetPosition (pReader); // reset pos with '<'
               //rtXmlRdCancelMark (pReader);
               */
               pReader->mData.length = lastDataOffset;
               pReader->mData.value =
                  rtXmlRdGetPtr (pReader, 0, 0);

               pReader->mLastEvent.mId = OSXMLEVT_TEXT;
               pReader->mLastEvent.mLevel = pReader->mLevel;
               pReader->mLocalStates[OSXMLSI_CONTENT] = 0;
               return 0;
            }
         }
      }

      pReader->mByteIndex++;
   }

   /* We broke the loop by setting the event id or by setting mLocalStates <= 0.
      I think we never set the local state to negative, and I think the only way
      we set it to zero is when we do an return, so setting the event id is
      probably what we've done.
   */
   return 1;
}

/*
// returns:
// 0 - finished
// 1 - need continue
// -1 - error
*/
static int processCDATA (OSXMLReader* pReader)
{
   static const OSINT8 transitions [][18] = {
      /* columns - states, rows - input chars */
      /* <   ?   !   >   /  ch  nm  sp   [   ]   C   D   A   T              other */
      {  0,  0,  0,  0,  0,  0,  0,  0,  2,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, /* st=1, initial */
      {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  3,  0,  0,  0,  0,  0,  0,  0 }, /* st=2, '[' recvd */
      {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  4,  0,  0,  0,  0,  0,  0 }, /* st=3, '[C' recvd */
      {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  5,  0,  0,  0,  0,  0 }, /* st=4, '[CD' recvd */
      {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  6,  0,  0,  0,  0 }, /* st=5, '[CDA' recvd */
      {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  7,  0,  0,  0,  0,  0 }, /* st=6, '[CDAT' recvd */
      {  0,  0,  0,  0,  0,  0,  0,  0,  8,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, /* st=7, '[CDATA' recvd */
      {  8,  8,  8,  8,  8,  8,  8,  8,  8,  9,  8,  8,  8,  8,  8,  8,  8,  8 }, /* st=8, pull text */
      {  8,  8,  8,  8,  8,  8,  8,  8,  8, 10,  8,  8,  8,  8,  8,  8,  8,  8 }, /* st=9, ']' recvd */
      {  8,  8,  8, -1,  8,  8,  8,  8,  8, 10,  8,  8,  8,  8,  8,  8,  8,  8 }, /* st=10,']]' recvd  */
      /* collapse processing states */
      { 12, 12, 12, 12, 12, 12, 12, 11, 12, 13, 12, 12, 12, 12, 12, 12, 12, 12 }, /* st=11, skip leading spaces */
      { 12, 12, 12, 12, 12, 12, 12, 15, 12, 13, 12, 12, 12, 12, 12, 12, 12, 12 }, /* st=12, pull text */
      { 12, 12, 12, 12, 12, 12, 12, 15, 12, 14, 12, 12, 12, 12, 12, 12, 12, 12 }, /* st=13, ']' recvd */
      { 12, 12, 12, -1, 12, 12, 12, 15, 12, 14, 12, 12, 12, 12, 12, 12, 12, 12 }, /* st=14,']]' recvd  */
      { 12, 12, 12, 12, 12, 12, 12, 15, 12, 16, 12, 12, 12, 12, 12, 12, 12, 12 }, /* st=15, space recvd */
      { 12, 12, 12, 12, 12, 12, 12, 15, 12, 17, 12, 12, 12, 12, 12, 12, 12, 12 }, /* st=16, ']' recvd */
      { 12, 12, 12, -1, 12, 12, 12, 15, 12, 14, 12, 12, 12, 12, 12, 12, 12, 12 }, /* st=17,']]' recvd  */
      /* #xD received; dummy state */
      {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }  /* st=18, CR recvd */
   };
   size_t lastDataOffset = 0;

   OSRTASSERT (pReader != 0);

   if (pReader->mLocalStates[OSXMLSI_CDATA] == 0) {
      pReader->mLocalStates[OSXMLSI_CDATA] = 1;
      pReader->mbLastChunk = FALSE;
      pReader->mbCDATA = TRUE;
   }

   rtXmlRdMarkPosition (pReader);

   for (; pReader->mLastEvent.mId == OSXMLEVT_NONE &&
           pReader->mLocalStates[OSXMLSI_CDATA] > 0; ) {
      int symbolIndex, prevState, state;
      int p;

      prevState = pReader->mLocalStates[OSXMLSI_CDATA];

      if (NEED_PREREAD (pReader) && rtXmlRdIsMarked (pReader)) {
         switch (prevState) {
            case 8: /* content value */
            case 12:
               pReader->mLastEvent.mLevel = pReader->mLevel;
               pReader->mData.value =
                  rtXmlRdGetPtr (pReader, 0, &pReader->mData.length);
               pReader->mLastEvent.mId = OSXMLEVT_TEXT;
               rtXmlRdMarkPosition (pReader);
               pReader->mbLastChunk = FALSE;
               continue;
         }
      }

      p = PEEK_SYMBOL (pReader);
      if (p == -1) {
         return OSXMLEVT_END_DOCUMENT;
      }

      if (prevState == 8 && p == '\r') {
         if (rtXmlRdGetOffset (pReader) > 0) {
            /* return already processed data */
            pReader->mLocalStates[OSXMLSI_CDATA] = 18;

            pReader->mData.value =
               rtXmlRdGetPtr (pReader, 0, &pReader->mData.length);

            pReader->mLastEvent.mId = OSXMLEVT_TEXT;
            pReader->mLastEvent.mLevel = pReader->mLevel;
            pReader->mByteIndex++; /* skip #xD */
            return 1;
         }
         else {
            pReader->mByteIndex++; /* skip #xD */
            rtXmlRdMarkPosition (pReader);

            p = PEEK_SYMBOL (pReader);
            if (p == -1) {
               return OSXMLEVT_END_DOCUMENT;
            }

            prevState = 18;
         }
      }

      if (prevState == 18) {
         state = pReader->mLocalStates[OSXMLSI_CDATA] =
            (p == '\r') ? 18 : 8;

         if (p != '\n') {
            /* return #xA char instead of #xD */
            if (pReader->mWhiteSpaceMode == OSXMLWSM_PRESERVE)
               pReader->mCharBuf[0] = '\n';
            else
               pReader->mCharBuf[0] = ' ';

            pReader->mLastEvent.mId = OSXMLEVT_TEXT;
            pReader->mLastEvent.mLevel = pReader->mLevel;
            pReader->mData.length = 1;
            pReader->mData.value = pReader->mCharBuf;
            return 1;
         }

         symbolIndex = getCDATASymbolIndex (pReader, p);
         if (symbolIndex < 0) return symbolIndex;
      }
      else {
         symbolIndex = getCDATASymbolIndex (pReader, p);
         if (symbolIndex < 0) return symbolIndex;

         state = pReader->mLocalStates[OSXMLSI_CDATA] =
            transitions [pReader->mLocalStates[OSXMLSI_CDATA] - 1][symbolIndex];

         if (state == 0) {
            /* TODO: handle errors */
            return rtXmlRdLogInvSymbolError (pReader);
         }
      }

/* YG - space replacement moved to rtXmlpDec... */
#ifdef _PULL_FROM_BUFFER
      if (symbolIndex == OSXMLSYM_SPACE &&
            pReader->mWhiteSpaceMode != OSXMLWSM_PRESERVE)
         pReader->mpBuffer[pReader->mByteIndex] = ' ';
#endif

      if (prevState != state) {
         if (prevState == 11 || prevState == 15) /* skip spaces */
            rtXmlRdMarkPosition (pReader);

         switch (state) {
            case 8:  /* content begin */
               if (pReader->mWhiteSpaceMode == OSXMLWSM_COLLAPSE)
                  state = pReader->mLocalStates[OSXMLSI_CDATA] = 11;
               break;

            case 12:  /* token begin */
               if (prevState == 15 && !pReader->mbListMode) {
                  pReader->mCharBuf[0] = ' ';
                  pReader->mLastEvent.mId = OSXMLEVT_TEXT;
                  pReader->mLastEvent.mLevel = pReader->mLevel;
                  pReader->mData.length = 1;
                  pReader->mData.value = pReader->mCharBuf;
                  return 1;
               }
               break;

            case 10: /* ']]' recvd */
            case 14:
               lastDataOffset = rtXmlRdGetOffset (pReader) - 1;
               break;

            case 15:
               if (rtXmlRdGetOffset (pReader) > 0 || pReader->mbListMode) {
                  pReader->mData.value =
                     rtXmlRdGetPtr (pReader, 0, &pReader->mData.length);

                  pReader->mLastEvent.mId = OSXMLEVT_TEXT;
                  pReader->mLastEvent.mLevel = pReader->mLevel;
                  rtXmlRdMarkPosition (pReader);

                  if (pReader->mbListMode) {
                     pReader->mbLastChunk = TRUE;
                     /* go to skip ws */
                     state = pReader->mLocalStates[OSXMLSI_CDATA] = 11;
                  }

                  return 1;
               }

               rtXmlRdMarkPosition (pReader);
               break;

            case -1: /* ']]>' recvd */
               if (prevState == 17) /* after trailing spaces */
                  pReader->mData.length = 0;
               else
                  pReader->mData.length = lastDataOffset;

               pReader->mData.value =
                  rtXmlRdGetPtr (pReader, 0, 0);

               pReader->mLastEvent.mId = OSXMLEVT_TEXT;
               pReader->mLastEvent.mLevel = pReader->mLevel;
               pReader->mLocalStates[OSXMLSI_CDATA] = 0;
               pReader->mbLastChunk = TRUE;

               pReader->mByteIndex++;
               return 0;
         }
      }
      /* Handle repeating ]]] at end of CDATA */
      else if (state == 10 || state == 14)
         lastDataOffset++;


      pReader->mByteIndex++;

      if (prevState == 7 && state == 8)
         rtXmlRdMarkPosition (pReader);
   }
   return 1;
}

/*
// returns:
// 0 - finished
// 1 - need continue
// -1 - error
*/
static int processDTD (OSXMLReader* pReader)
{
   static const OSINT8 transitions [][18] = {
      /* columns - states, rows - input chars */
      /* <   ?   !   >   /  ch  nm  sp   :   -   =   "   &   ;   .   _   '  other */
      {  0,  0,  0,  0,  0,  2,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, /* st=1, initial */
      {  5,  2,  2, -1,  2,  2,  2,  2,  2,  2,  2,  3,  2,  2,  2,  2,  4,  2 }, /* st=2, ch recvd */
      {  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  2,  3,  3,  3,  3,  3,  3 }, /* st=3, " recvd */
      {  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  2,  4 }, /* st=4, ' recvd */
      /* process decl */
      {  0, 14,  6,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, /* st=5, < recvd */
      {  0,  0,  0,  0,  0,  7,  0,  0,  0, 10,  0,  0,  0,  0,  0,  0,  0,  0 }, /* st=6, <! recvd */
      {  7,  7,  7,  2,  7,  7,  7,  7,  7,  7,  7,  8,  7,  7,  7,  7,  9,  7 }, /* st=7, <!ch recvd */
      {  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  7,  8,  8,  8,  8,  8,  8 }, /* st=8, " recvd */
      {  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  7,  9 }, /* st=9, ' recvd */
      /* process comment */
      {  0,  0,  0,  0,  0,  0,  0,  0,  0, 11,  0,  0,  0,  0,  0,  0,  0,  0 }, /* st=10, <!- recvd */
      { 11, 11, 11, 11, 11, 11, 11, 11, 11, 12, 11, 11, 11, 11, 11, 11, 11, 11 }, /* st=11, <!-- recvd */
      { 11, 11, 11, 11, 11, 11, 11, 11, 11, 13, 11, 11, 11, 11, 11, 11, 11, 11 }, /* st=12, - recvd */
      { 11, 11, 11,  2, 11, 11, 11, 11, 11, 12, 11, 11, 11, 11, 11, 11, 11, 11 }, /* st=13, -- recvd */
      /* process PI */
      { 14, 15, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14 }, /* st=14, <? recvd */
      { 14, 15, 14,  2, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14 }  /* st=15, ? recvd */
   };
   int stat;

   OSRTASSERT (pReader != 0);

   /* dtd section is skipped */
   rtXmlRdCancelMark (pReader);

   if (pReader->mLocalStates[OSXMLSI_DTD] == 0) {
      pReader->mLocalStates[OSXMLSI_DTD] = 1;
   }

   stat = readLoop (pReader, OSXMLSI_DTD, transitions, TRUE);
   if (stat == 0) {
      pReader->mByteIndex++;
      pReader->mLastEvent.mId = OSXMLEVT_DTD;
   }
   return stat;
}

static int checkForUTF16BOM (struct OSXMLReader*);

int rtXmlRdNext (OSXMLReader* pReader)
{
   OSCTXT* pctxt;
   const OSINT8 (*ptransitions)[18];
   int rv;

   OSRTASSERT (pReader != 0 && pReader->mpCtxt != 0);

   pctxt = pReader->mpCtxt;
#ifdef XMLPTRACE
   RTDIAG1 (pctxt, "rtXmlRdNext: start\n");
#endif
   if ((rv = rtXmlRdProcessDelayedTasks (pReader)) != 0)
      return rv;

   pReader->mbEmptyElement = FALSE;

   if (pReader->mLastEvent.mId == OSXMLEVT_INITIAL) {
      int stat = checkForUTF16BOM (pReader);
      if (0 != stat) return LOG_RTERR (pctxt, stat);

      ptransitions = initial_transitions;
   }
   else if (pReader->mLastEvent.mId == OSXMLEVT_START_DOCUMENT)
      ptransitions = stdoc_transitions;
   else
      ptransitions = main_transitions;

   pReader->mLastEvent.mId = OSXMLEVT_NONE;
   pReader->mLastEvent.mLevel = 0;

   if (pReader->mState == 0)
      pReader->mState = 1;

   do {
      rv = 1;
#ifdef XMLPTRACE
      RTDIAG2 (pReader->mpCtxt, "pReader->mState is %d\n", pReader->mState);
#endif
      switch (pReader->mState) {
         case 0: return rtXmlRdLogError (pReader, RTERR_XMLSTATE);;
         case -1: rv = processHeader (pReader); break;
         case -2: rv = processComments (pReader); break;
         case -3: rv = processStartElement (pReader); break;
         case -4: rv = processDTD (pReader); break;
         case -5: rv = processCharacters (pReader); break;
         case -6: rv = processPI (pReader); break;
         case -7: rv = processEndElement (pReader); break;
         case -8: rv = processCDATA (pReader); break;
      }
      if (rv == 0) {
         rv = pReader->mState = 1;
         /*
         //if (pReader->mLastEvent != OSXMLEVT_NONE)
         //   return pReader->mLastEvent;
         */
      }
      else if (rv < 0) {
         pReader->mState = 1;
         return rv;
      }
      else if (rv == OSXMLEVT_END_DOCUMENT) {
         pReader->mState = 1;
         pReader->mLastEvent.mId = rv;
         return rv;
      }

      for (; pReader->mLastEvent.mId == OSXMLEVT_NONE &&
              pReader->mState > 0; ) {
         int symbolIndex;
         int p;

         p = PEEK_SYMBOL (pReader);
         if (p == -1) {
            pReader->mLastEvent.mId = OSXMLEVT_END_DOCUMENT;
            return OSXMLEVT_END_DOCUMENT;
         }
         symbolIndex = getSymbolIndex (pReader, p);
         if (symbolIndex < 0) return symbolIndex;

         pReader->mState = ptransitions [pReader->mState - 1][symbolIndex];
#ifdef XMLPTRACE
         RTDIAG2 (pctxt, "symbolIndex is %d\n", symbolIndex);
         RTDIAG2 (pctxt, "pReader->mState is %d\n", pReader->mState);
#endif
         if (pReader->mState < 0) {
#ifdef XMLPTRACE
            RTDIAG1 (pctxt, "break loop\n");
#endif
            break;
         }
         pReader->mByteIndex++;
      }
   } while (pReader->mLastEvent.mId == OSXMLEVT_NONE);

#ifndef _PULL_FROM_BUFFER
   if (!OSRTISSTREAM (pReader->mpCtxt)) {
      OSRTBuffer* buffer = &pReader->mpCtxt->buffer;
      buffer->byteIndex = pReader->mByteIndex;
   }
#endif

   return pReader->mLastEvent.mId;
}

EXTXMLMETHOD int rtXmlRdSkipCurrentLevel (OSXMLReader* pReader)
{
   OSINT32 curLevel = pReader->mLevel;
   int stat = 0;

   if (pReader->mLastEvent.mId != OSXMLEVT_START_TAG)
      return 0;

   /* pReader->mbWSOnlyContext = FALSE; */

   while ((stat = rtXmlRdNext (pReader)) >= 0) {
      if (pReader->mLastEvent.mId == OSXMLEVT_END_TAG &&
          pReader->mLastEvent.mLevel == curLevel)
         return 0;

      if (pReader->mLastEvent.mId == OSXMLEVT_END_DOCUMENT)
         return rtXmlRdLogError (pReader, XML_E_UNEXPEOF);
   }

   return stat;
}

EXTXMLMETHOD long rtXmlRdFirstData (OSXMLReader* pReader, OSXMLDataCtxt* pDataCtxt)
{
   OSRTASSERT (pReader != 0 && pReader->mpCtxt != 0 && pDataCtxt != 0);

   if (pReader->mDataMode == OSXMLDM_SIMULATED) {
      /* if attribute name has been read -
         we will read attribute value */
      int stat;

      pDataCtxt->mbLastChunk = FALSE;
      pDataCtxt->mDataMode = OSXMLDM_SIMULATED;
      pDataCtxt->mnChunk = 0;
      pDataCtxt->mbCDATA = FALSE;

      pDataCtxt->mSrcData.length = pReader->mData.length;
      pDataCtxt->mSrcData.value = pReader->mData.value;
      pDataCtxt->mSrcDataOffset = 0;
      pDataCtxt->mbInsTokenSeparator = FALSE;

      stat = rtXmlRdProcessAttrValue (pReader, pDataCtxt);

      if (stat > 0)
         return (long)pDataCtxt->mData.length;
      else
         pReader->mDataMode = OSXMLDM_NONE;

      return stat;
   }
   else if (
      (pReader->mLastEvent.mId & OSXMLEVT_ID_MASK) == OSXMLEVT_START_TAG ||
      (pReader->mLastEvent.mId & OSXMLEVT_ID_MASK) == OSXMLEVT_END_TAG
      )
   {
      int stat;

      /* Empty element contains nothing, but do not advance to end tag */
      if (rtXmlRdIsEmpty (pReader)) {
         pDataCtxt->mData.length = 0;
         pDataCtxt->mData.value = 0;
         pDataCtxt->mbLastChunk = TRUE;
         return 0;
      }

      /* if start tag, but not attribute name - we will
         read element's content */
      pDataCtxt->mbLastChunk = FALSE;
      pDataCtxt->mDataMode = OSXMLDM_CONTENT;
      pDataCtxt->mnChunk = 0;
      pDataCtxt->mbCDATA = FALSE;
      pDataCtxt->mDataLevel = pReader->mLevel;
      /* pReader->mbWSOnlyContext = FALSE; */

      /* Read first chunk of content */

      if ((stat = rtXmlRdNext (pReader)) < 0)
         return stat;

      while (stat >= 0 &&
             (pReader->mLastEvent.mId == OSXMLEVT_COMMENT ||
              pReader->mLastEvent.mId == OSXMLEVT_PI))
         stat = rtXmlRdNext (pReader);

      if (stat < 0)
         return stat;
      else if (pReader->mLastEvent.mId == OSXMLEVT_START_TAG ||
               pReader->mLastEvent.mId == OSXMLEVT_END_TAG)
      {
         /* error processed by rtXmlpMatchEndTag */
         pDataCtxt->mData.length = 0;
         pDataCtxt->mData.value = 0;
         return 0;
      }
      else if (pReader->mLastEvent.mId == OSXMLEVT_END_DOCUMENT)
         return rtXmlRdLogError (pReader, XML_E_UNEXPEOF);

      pDataCtxt->mData.length = pReader->mData.length;
      pDataCtxt->mData.value = pReader->mData.value;
      pDataCtxt->mbLastChunk = pReader->mbLastChunk;
      if (pReader->mbCDATA) pDataCtxt->mbCDATA = TRUE;
      return (long)pDataCtxt->mData.length;
   }
   else if (pReader->mbListMode && pReader->mLastEvent.mId == OSXMLEVT_TEXT) {
      pDataCtxt->mDataMode = OSXMLDM_CONTENT;
      pDataCtxt->mnChunk = 0;
      if (pReader->mbCDATA) pDataCtxt->mbCDATA = TRUE;
      pDataCtxt->mDataLevel = pReader->mLevel;

      pDataCtxt->mData.length = pReader->mData.length;
      pDataCtxt->mData.value = pReader->mData.value;
      pDataCtxt->mbLastChunk = pReader->mbLastChunk;

      return (long)pDataCtxt->mData.length;
   }

   return rtXmlRdLogError (pReader, XML_E_INVMODE);
}

EXTXMLMETHOD long rtXmlRdNextData (OSXMLReader* pReader, OSXMLDataCtxt* pDataCtxt)
{
   OSRTASSERT (pReader != 0 && pReader->mpCtxt != 0 && pDataCtxt != 0);

   if (pDataCtxt->mDataMode == OSXMLDM_SIMULATED) {
      int stat = 0;

      if (!pDataCtxt->mbLastChunk) {
         pDataCtxt->mnChunk++;
         stat = rtXmlRdProcessAttrValue (pReader, pDataCtxt);

         if (stat > 0)
            return (long)pDataCtxt->mData.length;
         else {
            if (!pReader->mbListMode)
               pReader->mDataMode = OSXMLDM_NONE;
            return stat;
         }
      }
      else {
         /* pDataCtxt->mDataMode = OSXMLDM_NONE; */
         if (!pReader->mbListMode)
            pReader->mDataMode = OSXMLDM_NONE;
         return 0;
      }
   }
   else if (pDataCtxt->mDataMode == OSXMLDM_CONTENT) {

      /* Read chunk of content */

      if (pReader->mbLastChunk) {
         pDataCtxt->mDataMode = OSXMLDM_NONE;
         return 0;
      }
      else {
         while (1) {
            int evt = rtXmlRdNext (pReader);

            if (evt == OSXMLEVT_COMMENT || evt == OSXMLEVT_PI)
               continue;
            else if (evt == OSXMLEVT_TEXT) {
               pDataCtxt->mData.length = pReader->mData.length;
               pDataCtxt->mData.value = pReader->mData.value;
               pDataCtxt->mbLastChunk = pReader->mbLastChunk;
               if (pReader->mbCDATA) pDataCtxt->mbCDATA = TRUE;

               pDataCtxt->mnChunk++;
               return (long)pReader->mData.length;
            }
            else {
               pDataCtxt->mDataMode = OSXMLDM_NONE;

               if (evt == OSXMLEVT_START_TAG) {
                  /* error processed by rtXmlpMatchEndTag */
                  pDataCtxt->mData.length = 0;
                  pDataCtxt->mData.value = 0;
                  return 0;
               }
               else if (evt == OSXMLEVT_END_DOCUMENT)
                  return rtXmlRdLogError (pReader, XML_E_UNEXPEOF);
               else if (evt < 0)
                  return evt;
               else
                  return 0;
            }
         }
      }
   }

   return rtXmlRdLogError (pReader, XML_E_INVMODE);
}

EXTXMLMETHOD OSXMLWhiteSpaceMode rtXmlRdSetWhiteSpaceMode
   (OSXMLReader* pReader, OSXMLWhiteSpaceMode whiteSpaceMode)
{
   OSXMLWhiteSpaceMode prevWSM = pReader->mWhiteSpaceMode;

   OSRTASSERT (pReader != 0);

   if (pReader->mbListMode)
      pReader->mWhiteSpaceMode = OSXMLWSM_COLLAPSE;
   else
      pReader->mWhiteSpaceMode = whiteSpaceMode;

   return prevWSM;
}

static void initReaderVars (OSXMLReader* pReader)
{
   pReader->mState = 1;
   pReader->mPrevState = 0;
   pReader->mLevel = 0;
   pReader->mDecodeLevel = 0;
   pReader->mMarkedPos = (size_t)-1;
   pReader->mLastEvent.mId = OSXMLEVT_INITIAL;
   pReader->mLastEvent.mLevel = pReader->mLevel;
   pReader->mError = RT_OK;
   OSCRTLMEMSET (pReader->mLocalStates, 0, sizeof (pReader->mLocalStates));

   pReader->mDelayedTaskCount = 0;
   pReader->mDelayedTaskIndex = 0;

   pReader->mElementNsIndex = OSXMLNSI_UNQUALIFIED;
   pReader->mNamespacesNumber = 0;
   pReader->mNamespaceTable = 0;

   pReader->mPrevNamespacesNumber = 0;
   pReader->mPrevNamespaceTable = 0;

   pReader->mbHasAttributes = TRUE;
   pReader->mLastBlockSize = 0;
   pReader->mbBackoffEnabled = FALSE;
   pReader->mStringWhiteSpaceMode = OSXMLWSM_PRESERVE;

}

#ifndef _NO_STREAM
static void initReaderStream (OSXMLReader* pReader)
{
   /* If file or socket stream and buffering set, turn it off because
      the pull parser does its own buffering */
   if (0 != (pReader->mpCtxt->pStream->flags & OSRTSTRMF_BUFFERED) &&
       (pReader->mpCtxt->pStream->id == OSRTSTRMID_FILE ||
        pReader->mpCtxt->pStream->id == OSRTSTRMID_SOCKET))
   {
      pReader->mpCtxt->pStream->flags &= ~OSRTSTRMF_BUFFERED;
      if (0 != pReader->mpCtxt->buffer.data) {
         rtxMemSysFreePtr (pReader->mpCtxt, pReader->mpCtxt->buffer.data);
         pReader->mpCtxt->buffer.data = 0;
      }
   }

   if (0 != (pReader->mpCtxt->pStream->flags & OSRTSTRMF_BUFFERED) ||
       pReader->mpCtxt->pStream->id != OSRTSTRMID_SOCKET)
   {
      /* This is set for all streams except an unbuffered socket stream */
      pReader->mbBackoffEnabled = TRUE;
   }

}
#endif

EXTXMLMETHOD OSXMLReader* rtXmlRdCreateXmlReader (OSCTXT* pctxt)
{
   OSXMLReader* pReader;

   OSRTASSERT (pctxt != 0);

   pReader = (OSXMLReader*) rtxMemSysAlloc (pctxt, sizeof(OSXMLReader));
   if (pReader == 0) return 0;

   OSCRTLMEMSET (pReader, 0, sizeof(OSXMLReader));

   pReader->mpCtxt = pctxt;

   initReaderVars (pReader);

   rtXmlRdStackInit
      (pReader, &pReader->mAttributes, sizeof (OSXMLAttrOffset));
   rtXmlRdStackInit
      (pReader, &pReader->mTagNamesStack, sizeof (OSXMLStrFragment));
   rtXmlRdStackInit
      (pReader, &pReader->mNamespacesStack, sizeof (OSXMLNamespace_));
   rtXmlRdStackInit
      (pReader, &pReader->mRewindPosStack, sizeof (OSXMLRewindPos));

#ifndef _NO_STREAM
   if (OSRTISSTREAM (pReader->mpCtxt)) {
      initReaderStream (pReader);
   }
#endif /* _NO_STREAM */
#ifndef _PULL_FROM_BUFFER
#ifndef _NO_STREAM
   else
#endif /* _NO_STREAM */
   {
      OSRTBuffer* buffer = &pReader->mpCtxt->buffer;
      const OSOCTET* data;

      if (0 == buffer || buffer->size == 0) {
         rtxMemSysFreePtr (pctxt, pReader);
         LOG_RTERR (pctxt, RTERR_ENDOFBUF);
         return 0;
      }

      data = buffer->data + buffer->byteIndex;

      if ((data[0] == 0xFF && data[1] == 0xFE) ||
          (data[0] == 0x3C && data[1] == 0x00))
         pReader->mBOM = OSXMLBOM_UTF16_LE;
      else if ((data[0] == 0xFE && data[1] == 0xFF) ||
               (data[0] == 0x00 && data[1] == 0x3C))
         pReader->mBOM = OSXMLBOM_UTF16_BE;
      else
         pReader->mBOM = OSXMLBOM_NO_BOM;

      pReader->mpBuffer = buffer->data;
      pReader->mByteIndex = buffer->byteIndex;
      pReader->mBufSize = buffer->size;
      pReader->mReadSize = buffer->size;
   }
#endif /* _PULL_FROM_BUFFER */

   pReader->mSrcPos.mLine = 0;
   pReader->mSrcPos.mColumn = 0;
   pReader->mSrcPos.mByteIdx = 0;
   pReader->mSrcPos.mbCR = FALSE;

   rtXmlRdNext (pReader);

   return pReader;
}

EXTXMLMETHOD void rtXmlRdResetXmlReader (OSXMLReader* pReader)
{
   OSRTASSERT (pReader != 0);

   initReaderVars (pReader);

   rtXmlRdStackReinit (pReader, &pReader->mAttributes);
   rtXmlRdStackReinit (pReader, &pReader->mTagNamesStack);
   rtXmlRdStackReinit (pReader, &pReader->mNamespacesStack);
   rtXmlRdStackReinit (pReader, &pReader->mRewindPosStack);

#ifndef _NO_STREAM
   if (OSRTISSTREAM (pReader->mpCtxt)) {
      initReaderStream (pReader);

      /* Doing this causes problems in places where we want to resume
         reading where we left off (for example, StreamingXML) but it
         is needed in other places where we want for restart parsing from
         the beginning.  There is currently no way to distingusih these
         cases..
      */
      if (pReader->mpCtxt->pStream->id != OSRTSTRMID_SOCKET) {
         pReader->mByteIndex = 0;
         pReader->mReadSize = 0;
      }
   }
   else
#endif /* _NO_STREAM */
   {
      /* Note: this is different from what is done in the create reader
         function above.. */
#ifdef _PULL_FROM_BUFFER
      pReader->mByteIndex = 0;
      pReader->mReadSize = 0;

      if (pReader->mBufSize > 0) { /* allocate buffer */
         pReader->mpBuffer = (OSOCTET*)
            rtxMemAlloc (pReader->mpCtxt, pReader->mBufSize);

         if (pReader->mpBuffer == 0)
            pReader->mBufSize = 0;
      }
#else
      OSRTBuffer* buffer = &pReader->mpCtxt->buffer;
      pReader->mpBuffer = buffer->data;
      pReader->mByteIndex = buffer->byteIndex;
      pReader->mBufSize = buffer->size;
      pReader->mReadSize = buffer->size;
#endif
   }

   pReader->mSrcPos.mLine = 0;
   pReader->mSrcPos.mColumn = 0;
   pReader->mSrcPos.mByteIdx = 0;
   pReader->mSrcPos.mbCR = FALSE;

   rtXmlRdNext (pReader);
}

EXTXMLMETHOD int rtXmlRdGetTagName
(OSXMLReader* pReader, OSXMLStrFragment* pLocalName, OSINT16* pNsIndex)
{
   OSRTASSERT (pReader != 0);

   pLocalName->length = pReader->mElementName.mLocalName.length;
   pLocalName->value = (const OSUTF8CHAR*)
      rtXmlRdGetPtr (pReader, pReader->mElementName.mLocalName.offset, 0);

   if (pReader->mElementNsIndex == OSXMLNSI_UNCHECKED ||
       pReader->mPrevNamespaceTable != pReader->mNamespaceTable)
      rtXmlRdProcessStartElement (pReader);

   if (pNsIndex) {
      if (rtxCtxtTestFlag (pReader->mpCtxt, OSXMLNOCMPNS))
         *pNsIndex = (OSINT16) OSXMLNSI_UNCHECKED;
      else
         *pNsIndex = pReader->mElementNsIndex;
   }

   return 0;
}

EXTXMLMETHOD int rtXmlRdNextTag (OSXMLReader* pReader)
{
   int stat = 0;

   OSRTASSERT (pReader != 0 && pReader->mpCtxt != 0);

   while ((stat = rtXmlRdNext (pReader)) >= 0) {
      if (pReader->mLastEvent.mId == OSXMLEVT_START_TAG)
         break;
      if (pReader->mLastEvent.mId == OSXMLEVT_END_DOCUMENT)
         return rtXmlRdLogError (pReader, XML_E_UNEXPEOF);
   }
   return stat;
}

EXTXMLMETHOD int rtXmlRdNextEndTag (OSXMLReader* pReader)
{
   int stat = 0;

   OSRTASSERT (pReader != 0 && pReader->mpCtxt != 0);

   while ((stat = rtXmlRdNext (pReader)) >= 0) {
      if (pReader->mLastEvent.mId == OSXMLEVT_END_TAG)
         break;
      if (pReader->mLastEvent.mId == OSXMLEVT_END_DOCUMENT)
         return rtXmlRdLogError (pReader, XML_E_UNEXPEOF);
   }
   return stat;
}

EXTXMLMETHOD int rtXmlRdNextEvent
(OSXMLReader* pReader, OSUINT32 eventMask, OSINT32 maxLevel,
 OSXMLEvent* pLastEvent)
{
   int stat = 0;

   OSRTASSERT (pReader != 0 && pReader->mpCtxt != 0);
#if 0
   pReader->mbWSOnlyContext =
      (OSBOOL) ((eventMask & OSXMLEVT_TEXT) ? FALSE : TRUE);
#endif
   while ((stat = rtXmlRdNext (pReader)) >= 0) {
      if (pReader->mLastEvent.mId & eventMask)
         break;
      if (pReader->mLastEvent.mId == OSXMLEVT_END_DOCUMENT)
         return rtXmlRdLogError (pReader, XML_E_UNEXPEOF);

      if (maxLevel >= 0 && pReader->mLevel > maxLevel)
         rtXmlRdSkipCurrentLevel (pReader);
   }
   if (pLastEvent != 0) {
      *pLastEvent = pReader->mLastEvent;
   }

   return (stat >= 0) ? 0 : stat;
}

EXTXMLMETHOD int rtXmlRdMarkLastEventDone (OSXMLReader* pReader)
{
   OSUINT32 eventID;

   OSRTASSERT (pReader != 0);
   eventID = pReader->mLastEvent.mId;

   pReader->mLastEvent.mId |= OSXMLEVT_USED_FLAG;
   pReader->mDecodeLevel = pReader->mLevel;

   return eventID;
}

EXTXMLMETHOD int rtXmlRdMarkLastEventActive (OSXMLReader* pReader)
{
   OSRTASSERT (pReader != 0);
   pReader->mLastEvent.mId &= ~OSXMLEVT_USED_FLAG;

   if (pReader->mLastEvent.mId == OSXMLEVT_START_TAG)
      pReader->mDecodeLevel = pReader->mLevel - 1;

   return pReader->mLastEvent.mId;
}

EXTXMLMETHOD int rtXmlRdGetAttributeCount (OSXMLReader* pReader)
{
   OSRTASSERT (pReader != 0 && pReader->mpCtxt != 0);

   if ((pReader->mLastEvent.mId & OSXMLEVT_ID_MASK) == OSXMLEVT_START_TAG) {
      return (int)pReader->mAttributes.mCount;
   }

   return rtXmlRdLogError (pReader, XML_E_INVMODE);
}

EXTXMLMETHOD int rtXmlRdGetAttributeName
(OSXMLReader* pReader, OSXMLNameFragments* pAttr, OSINT16* pNsidx, size_t idx)
{
   OSXMLAttrOffset* pAttrOff;

   OSRTASSERT (pReader != 0 && pReader->mpCtxt != 0);

   if ((pReader->mLastEvent.mId & OSXMLEVT_ID_MASK) != OSXMLEVT_START_TAG)
      return rtXmlRdLogError (pReader, XML_E_INVMODE);
   else if (idx > pReader->mAttributes.mCount)
      return rtXmlRdLogError (pReader, RTERR_INVPARAM); /* error handling */

   pAttrOff = (OSXMLAttrOffset*)
      rtXmlRdStackGetByIndex (pReader, &pReader->mAttributes, idx);
   OSRTASSERT (pAttrOff != 0);

   pAttr->mLocalName.length = pAttrOff->mLocalName.length;
   pAttr->mLocalName.value = (const OSUTF8CHAR*)
      rtXmlRdGetPtr (pReader, pAttrOff->mLocalName.offset, 0);
   pAttr->mQName.length = pAttrOff->mQName.length;
   pAttr->mQName.value = (const OSUTF8CHAR*)
      rtXmlRdGetPtr (pReader, pAttrOff->mQName.offset, 0);

   pAttr->mPrefix.length = pAttrOff->mPrefix.length;
   if (pAttrOff->mPrefix.length > 0) {
      pAttr->mPrefix.value = (const OSUTF8CHAR*)
         rtXmlRdGetPtr (pReader, pAttrOff->mPrefix.offset, 0);
   }
   else {
      pAttr->mPrefix.value = 0;
   }

   if (pReader->mElementNsIndex == OSXMLNSI_UNCHECKED ||
       pReader->mPrevNamespaceTable != pReader->mNamespaceTable)
      rtXmlRdProcessStartElement (pReader);

   if (pNsidx != 0) {
      if (pAttrOff->index == OSXMLNSI_UNQUALIFIED &&
          pAttrOff->mPrefix.length > 0)
      {
         const OSUTF8CHAR* p = (const OSUTF8CHAR*)"";

         p = rtXmlRdGetPtr
            (pReader, pAttrOff->mPrefix.offset, 0);

         pAttrOff->index = (OSINT16) rtXmlRdGetNamespaceIndex
            (pReader, p, pAttrOff->mPrefix.length);

         if (pAttrOff->index == OSXMLNSI_XSI) {
            /* check xsi:schemaLocation */
            if (pAttr->mLocalName.length == 14 &&
                memcmp ("schemaLocation", pAttr->mLocalName.value, 14) == 0)
            {
               p = rtXmlRdGetPtr (pReader, pAttrOff->mValue.offset, 0);

               rtXmlSetSchemaLocationByStrFrag
                  (pReader->mpCtxt, p, pAttrOff->mValue.length);
            }

            /* check xsi:noNamespaceSchemaLocation */
            if (pAttr->mLocalName.length == 25 &&
                memcmp ("noNamespaceSchemaLocation",
                        pAttr->mLocalName.value, 25) == 0)
            {
               p = rtXmlRdGetPtr (pReader, pAttrOff->mValue.offset, 0);

               rtXmlSetNoNSSchemaLocationByStrFrag
                  (pReader->mpCtxt, p, pAttrOff->mValue.length);
            }
         }
      }

      if (rtxCtxtTestFlag (pReader->mpCtxt, OSXMLNOCMPNS))
         *pNsidx = (OSINT16) OSXMLNSI_UNCHECKED;
      else
         *pNsidx = (OSINT16) pAttrOff->index;
   }

   return 0;
}

EXTXMLMETHOD int rtXmlRdFirstAttr (struct OSXMLReader* pReader,
                             OSXMLStrFragment* pAttrName,
                             OSXMLStrFragment* pAttrValue)
{
   pReader->mMarkedAttrCount = pReader->mAttributes.mCount;
   return rtXmlRdNextAttr (pReader, pAttrName, pAttrValue);
}

EXTXMLMETHOD int rtXmlRdNextAttr (struct OSXMLReader* pReader,
                             OSXMLStrFragment* pAttrName,
                             OSXMLStrFragment* pAttrValue)
{
   const OSXMLAttrOffset* pAttrOff;

   OSRTASSERT (pReader != 0 && pReader->mpCtxt != 0);

   if ((pReader->mLastEvent.mId & OSXMLEVT_ID_MASK) != OSXMLEVT_START_TAG)
      return rtXmlRdLogError (pReader, XML_E_INVMODE);

   pAttrOff = (const OSXMLAttrOffset*)
                rtXmlRdStackPop (pReader, &pReader->mAttributes);

   if (pAttrOff == 0) {
      pReader->mAttributes.mCount = pReader->mMarkedAttrCount;
      return -1;
   }

   pAttrName->length = pAttrOff->mLocalName.length;
   pAttrName->value = (const OSUTF8CHAR*)
      rtXmlRdGetPtr (pReader, pAttrOff->mLocalName.offset, 0);

   pAttrValue->length = pAttrOff->mValue.length;
   pAttrValue->value = (const OSUTF8CHAR*)
      rtXmlRdGetPtr (pReader, pAttrOff->mValue.offset, 0);

   return 0;
}

EXTXMLMETHOD OSINT32 rtXmlRdGetCurrentLevel (OSXMLReader* pReader)
{
   OSRTASSERT (pReader != 0);

   return pReader->mLevel;
}

EXTXMLMETHOD OSXMLDataMode rtXmlRdGetDataMode (OSXMLReader* pReader)
{
   OSXMLDataMode dataMode;

   OSRTASSERT (pReader != 0);

   dataMode = pReader->mDataMode;
   pReader->mDataMode = OSXMLDM_NONE;

   return (dataMode == OSXMLDM_SIMULATED) ?
      OSXMLDM_SIMULATED : OSXMLDM_CONTENT;
}

EXTXMLMETHOD const OSXMLAttrOffset* rtXmlRdGetAttribute (OSXMLReader* pReader, size_t idx)
{
   OSRTASSERT (pReader != 0);

   return (const OSXMLAttrOffset*)
      rtXmlRdStackGetByIndex (pReader, &pReader->mAttributes, idx);
}

EXTXMLMETHOD void rtXmlRdMarkPos (OSXMLReader* pReader) {
   OSXMLRewindPos* pPos;
   OSRTASSERT (pReader != 0);

   pPos = (OSXMLRewindPos*)
      rtXmlRdStackPushNew (pReader, &pReader->mRewindPosStack);

   if (pReader->mDataMode != OSXMLDM_SIMULATED)
      pPos->mPos = pReader->mByteIndex;

   pPos->mData.length = pReader->mData.length;
   pPos->mData.value = pReader->mData.value;
   pPos->mEvent.mId = pReader->mLastEvent.mId;
   pPos->mEvent.mLevel = pReader->mLastEvent.mLevel;
   pPos->mbListMode = pReader->mbListMode;
   pPos->mbLastChunk = pReader->mbLastChunk;
}

EXTXMLMETHOD void rtXmlRdRewindToMarkedPos (OSXMLReader* pReader) {
   OSXMLRewindPos* pPos;
   OSRTASSERT (pReader != 0);

   pPos = (OSXMLRewindPos*) rtXmlRdStackGetTop
      (pReader, &pReader->mRewindPosStack);

   if (!pPos) return;

   if ((pReader->mLastEvent.mId & OSXMLEVT_ID_MASK) == OSXMLEVT_START_TAG)
      pReader->mDataMode = OSXMLDM_SIMULATED;
   else if (pPos->mPos != (size_t)-1)
      pReader->mByteIndex = pPos->mPos;

   pReader->mData.length = pPos->mData.length;
   pReader->mData.value = pPos->mData.value;
   pReader->mLastEvent.mId = pPos->mEvent.mId;
   pReader->mLastEvent.mLevel = pPos->mEvent.mLevel;
   pReader->mbListMode = pPos->mbListMode;
   pReader->mbLastChunk = pPos->mbLastChunk;
   pReader->mLocalStates[OSXMLSI_CONTENT] = 0;
}

EXTXMLMETHOD void rtXmlRdResetMarkedPos (OSXMLReader* pReader) {
   OSRTASSERT (pReader != 0);

   rtXmlRdStackPop (pReader, &pReader->mRewindPosStack);
}

EXTXMLMETHOD int rtXmlRdGetXSITypeAttr (OSXMLReader* pReader,
                           const OSUTF8CHAR** ppAttrValue,
                           OSINT16* nsidx, size_t* pLocalOffs)
{
   int i;
   size_t j;
   int stat;

   OSRTASSERT (pReader != 0);

   if (ppAttrValue)
      *ppAttrValue = 0;

   if (nsidx)
      *nsidx = OSXMLNSI_UNKNOWN;

   if (pReader->mElementNsIndex == OSXMLNSI_UNCHECKED ||
       pReader->mPrevNamespaceTable != pReader->mNamespaceTable)
      rtXmlRdProcessStartElement (pReader);

   for (i = 0; i < (int)pReader->mAttributes.mCount; i++) {
      OSXMLAttrOffset* pAttr = (OSXMLAttrOffset*) rtXmlRdStackGetByIndex
         (pReader, &pReader->mAttributes, i);

      OSRTASSERT (pAttr != 0);

      if (pAttr->index == OSXMLNSI_UNQUALIFIED &&
          pAttr->mPrefix.length > 0)
      {
         // As there is a prefix, there should be a namespace index
         const OSUTF8CHAR* p = (const OSUTF8CHAR*)"";

         p = rtXmlRdGetPtr (pReader, pAttr->mPrefix.offset, 0);

         pAttr->index = (OSINT16) rtXmlRdGetNamespaceIndex
            (pReader, p, pAttr->mPrefix.length);
      }

      if (pAttr->index == OSXMLNSI_XSI) {

         /* check xsi:type */
         if (ppAttrValue && pAttr->mLocalName.length == 4) {
            const OSUTF8CHAR* p =
               rtXmlRdGetPtr (pReader, pAttr->mLocalName.offset, 0);

            if (memcmp ("type", p, 4) == 0) {
               OSUTF8CHAR c;
               size_t length = pAttr->mValue.length;
               p = rtXmlRdGetPtr (pReader, pAttr->mValue.offset, 0);

               /* remove leading spaces */
               while (length && ((c = *p) == ' ' || c == '\r' ||
                                 c == '\n' || c == '\t'))
               {
                  p++;
                  length--;
               }

               for (j = 0; j < length; j++) {
                  if (p[j] == ':') break;
               }

               /* remove trailing spaces */
               while (length && ((c = p[length - 1]) == ' ' || c == '\r' ||
                                 c == '\n' || c == '\t'))
               {
                  length--;
               }

               if (j == length) {
                  /* unqualified */

                  if (nsidx)
                     *nsidx = (OSINT16)
                        rtXmlRdGetNamespaceIndex (pReader, p, 0);

                  j = 0;
               }
               else {
                  if (nsidx)
                     *nsidx = (OSINT16)
                        rtXmlRdGetNamespaceIndex (pReader, p, j);

                  j++;
               }

               if (pLocalOffs) {
                  *pLocalOffs = (size_t) j;
                  j = 0;
               }

               if (ppAttrValue) {
                  *ppAttrValue =
                     rtxUTF8Strndup (pReader->mpCtxt, p + j, length - j);
                  if (*ppAttrValue == 0) return RTERR_NOMEM;
               }
            }
         }

         /* check xsi:schemaLocation */
         if (pAttr->mLocalName.length == 14) {
            const OSUTF8CHAR* p = rtXmlRdGetPtr (pReader,
                     pAttr->mLocalName.offset, 0);

            if (memcmp ("schemaLocation", p, 14) == 0) {
               p = rtXmlRdGetPtr (pReader, pAttr->mValue.offset, 0);

               stat = rtXmlSetSchemaLocationByStrFrag
                  (pReader->mpCtxt, p, pAttr->mValue.length);

               if (stat < 0) return stat;
            }
         }

         /* check xsi:noNamespaceSchemaLocation */
         if (pAttr->mLocalName.length == 25) {
            const OSUTF8CHAR* p =
               rtXmlRdGetPtr (pReader, pAttr->mLocalName.offset, 0);

            if (memcmp ("noNamespaceSchemaLocation", p, 25) == 0) {
               p = rtXmlRdGetPtr (pReader, pAttr->mValue.offset, 0);

               stat = rtXmlSetNoNSSchemaLocationByStrFrag
                  (pReader->mpCtxt, p, pAttr->mValue.length);

               if (stat < 0) return stat;
            }
         }

         /* check xsi:nil */
         if (pAttr->mLocalName.length == 3) {
            const OSUTF8CHAR* p =
               rtXmlRdGetPtr (pReader, pAttr->mLocalName.offset, 0);

            if (memcmp ("nil", p, 3) == 0) {
               p = rtXmlRdGetPtr (pReader, pAttr->mValue.offset, 0);

               if (memcmp ("true", p, 4) == 0)
                   rtxCtxtSetFlag (pReader->mpCtxt, OSXSINIL);
            }
         }
      }
   }

   return 0;
}

EXTXMLMETHOD OSBOOL rtXmlRdIsEmpty (struct OSXMLReader* pReader) {
   OSBOOL isEmpty = FALSE;

   OSRTASSERT (pReader != 0);

   if ((pReader->mLastEvent.mId & OSXMLEVT_ID_MASK) == OSXMLEVT_START_TAG) {
      if ((PEEK_SYMBOL(pReader)) == '<') {
         pReader->mByteIndex++;

         if ((PEEK_SYMBOL(pReader)) == '/')
            isEmpty = TRUE;

         pReader->mByteIndex--;
      }
   }

   return isEmpty;
}

EXTXMLMETHOD void rtXmlRdGetSourcePosition
(struct OSXMLReader* pReader, OSUINT32* pLine, OSUINT32* pColumn,
 OSUINT32* pByteIndex, OSBOOL nextPos)
{
   size_t byteIdx = pReader->mByteIndex;

   OSXMLSrcPos pos;
   pos.mLine = pReader->mSrcPos.mLine;
   pos.mColumn = pReader->mSrcPos.mColumn;
   pos.mByteIdx = pReader->mSrcPos.mByteIdx;
   pos.mbCR = pReader->mSrcPos.mbCR;

   if (nextPos)
      byteIdx++;

   rtXmlRdUpdateSrcPos (&pos, pReader->mpBuffer, byteIdx);

   if (pLine != 0)
      *pLine = pos.mLine + 1; /* lines are counted from 1 */

   if (pColumn != 0)
      *pColumn = pos.mColumn;

   if (pByteIndex != 0)
      *pByteIndex = pos.mByteIdx;
}

static void rtXmlRdErrAddPos
(struct OSXMLReader* pReader, const OSXMLSrcPos* pos)
{
   #define PREFIX_LEN 12
   char buff[PREFIX_LEN + 40] = "at position "; /* 10 + 10 + 10 + 5 */
   int stat;
   int len = 0;

   stat = rtxUIntToCharStr (pos->mLine + 1, buff + PREFIX_LEN, 11, 0);

   if (stat >= 0) {
      len = stat + PREFIX_LEN;
      buff[len++] = ':';
      stat = rtxUIntToCharStr (pos->mColumn, buff + len, 11, 0);
   }

   if (stat >= 0) {
      len += stat;
      if (pos->mColumn != pos->mByteIdx) {
         buff[len++] = '(';
         stat = rtxUIntToCharStr (pos->mByteIdx, buff + len, 11, 0);

         if (stat >= 0) {
            len += stat;
            buff[len++] = ')';
         }
      }
   }

   rtxErrAddStrnParm (pReader->mpCtxt, buff, len);
}

EXTXMLMETHOD void rtXmlRdErrAddSrcPos (struct OSXMLReader* pReader, OSBOOL nextPos)
{
   size_t byteIdx = pReader->mByteIndex;

   OSXMLSrcPos pos;
   pos.mLine = pReader->mSrcPos.mLine;
   pos.mColumn = pReader->mSrcPos.mColumn;
   pos.mByteIdx = pReader->mSrcPos.mByteIdx;
   pos.mbCR = pReader->mSrcPos.mbCR;

   if (nextPos)
      byteIdx++;

   rtXmlRdUpdateSrcPos (&pos, pReader->mpBuffer, byteIdx);
   rtXmlRdErrAddPos (pReader, &pos);
}

EXTXMLMETHOD void rtXmlRdErrAddDataSrcPos
(struct OSXMLReader* pReader, OSXMLDataCtxt* pDataCtxt, OSSIZE offset)
{
   OSSIZE byteIdx;

   OSXMLSrcPos pos;
   pos.mLine = pReader->mSrcPos.mLine;
   pos.mColumn = pReader->mSrcPos.mColumn;
   pos.mByteIdx = pReader->mSrcPos.mByteIdx;
   pos.mbCR = pReader->mSrcPos.mbCR;

   if (pDataCtxt->mDataMode == OSXMLDM_SIMULATED) { /* attribute */
      byteIdx = pReader->mData.value - pReader->mpBuffer;

      if (offset <= pReader->mData.length)
         byteIdx += pReader->mData.length - offset;
   }
   else {
      byteIdx = pReader->mByteIndex;

      if (offset <= byteIdx)
         byteIdx -= offset;
      else
         byteIdx = 0;
   }

   rtXmlRdUpdateSrcPos (&pos, pReader->mpBuffer, byteIdx + 1);
   rtXmlRdErrAddPos (pReader, &pos);
}

static OSUTF8CHAR* getEncodingStr (struct OSXMLReader* pReader)
{
   OSUTF8CHAR* encodingStr = 0;
   const OSUTF8CHAR* pstr = rtXmlRdGetPtr (pReader, 0, 0);
   const OSUTF8CHAR* pBeg;
   size_t headerLen = rtXmlRdGetOffset (pReader);
   const OSUTF8CHAR* pEnd = pstr + headerLen;
   char* s;
   size_t i = 0, len = 0;

   while (pstr != pEnd) {
      if (i == 0 && *pstr == 'e') {
         if (pEnd - pstr > 8 && !OSCRTLMEMCMP (pstr, "encoding", 8)) {
            pstr += 8;

            while (pstr != pEnd && OS_ISSPACE (*pstr))
               pstr++;

            if (*pstr == '=') {
               pstr++;

               while (pstr != pEnd && OS_ISSPACE (*pstr))
                  pstr++;

               pBeg = pstr + 1;

               if (*pstr == '\"') {
                  do {
                     pstr++;
                  } while (pstr != pEnd && *pstr != '\"');
               }
               else if (*pstr == '\'') {
                  do {
                     pstr++;
                  } while (pstr != pEnd && *pstr != '\'');
               }
               else
                  break; /* invalid header */

               if (pstr != pEnd) {
                  len = pstr - pBeg;
                  encodingStr =
                     (OSUTF8CHAR*) rtxMemAlloc (pReader->mpCtxt, len + 1);

                  if (0 != encodingStr) {
                     OSCRTLSAFEMEMCPY (encodingStr, len+1, pBeg, len);
                     encodingStr[len] = 0;
                  }

                  break; /* encoding string presents */
               }
            }
            else
               break; /* invalid header */
         }
      }
      else if (i == 0 && *pstr == '\"') /* start attr value */
         i = 1;
      else if (i == 1 && *pstr == '\"') /* end attr value */
         i = 0;
      else if (i == 0 && *pstr == '\'') /* start attr value */
         i = 2;
      else if (i == 2 && *pstr == '\'') /* end attr value */
         i = 0;

      pstr++;
   }

   if (0 != encodingStr) {
      s = (char*) encodingStr;
      for (i = 0; i < len; i++) s[i] = (char) OS_TOUPPER(s[i]);

      if (OSCRTLSTRCMP ((const char*)encodingStr, OSXMLHDRUTF16) == 0)
         pReader->mEncoding = OSXMLUTF16;
      else if (OSCRTLSTRCMP ((const char*)encodingStr, OSXMLHDRLATIN1) == 0)
         pReader->mEncoding = OSXMLLATIN1;
      else
         pReader->mEncoding = OSXMLUTF8;
   }
   else
      pReader->mEncoding = OSXMLUTF8;

   return encodingStr;
}

static int checkForUTF16BOM (struct OSXMLReader* pReader)
{
   OSOCTET pbuffer[2];

   pReader->mBOM = OSXMLBOM_CHECK;

   if (pReader->mByteIndex >= pReader->mReadSize) {
      int stat = rtXmlRdPreReadFromStream (pReader);
      if (0 != stat) return LOG_RTERR (pReader->mpCtxt, stat);
   }

   if (pReader->mByteIndex < pReader->mReadSize) {
      pbuffer[0] = pReader->mpBuffer [pReader->mByteIndex];
      pbuffer[1] = pReader->mpBuffer [pReader->mByteIndex+1];
   }
   else {
      pbuffer[0] = pbuffer[1] = 0;
   }

   if ((pbuffer[0] == 0xFF && pbuffer[1] == 0xFE) ||
       (pbuffer[0] == 0x3C && pbuffer[1] == 0x00))
      pReader->mBOM = OSXMLBOM_UTF16_LE;
   else if ((pbuffer[0] == 0xFE && pbuffer[1] == 0xFF) ||
            (pbuffer[0] == 0x00 && pbuffer[1] == 0x3C))
      pReader->mBOM = OSXMLBOM_UTF16_BE;
   else
      pReader->mBOM = OSXMLBOM_NO_BOM;

   return 0;
}

#ifndef _NO_STREAM
static size_t latin1ToUTF8
(const OSOCTET* inbuf, size_t inlen, OSOCTET* outbuf)
{
   register OSOCTET c;
   OSOCTET* p = outbuf;
   size_t outBufSiz = 0;
   while (inlen--) {
      c = *inbuf; inbuf++;
      if ((c & 0x80) == 0) {
         p[0] = c; p++;
         outBufSiz ++;
      } else {
         p[0] = (OSOCTET)(0xC0 | (0x03 & (c >> 6)));
         p[1] = (OSOCTET)(0x80 | (0x3F & c));
         p = p + 2;
         outBufSiz = outBufSiz + 2;
      }
   }

   return outBufSiz;
}
#endif /* _NO_STREAM */
