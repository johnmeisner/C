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

/* Run-time utility functions */

#include <stdarg.h>
#include <stdlib.h>
#include "rtxsrc/rtxCharStr.h"
#include "rtxsrc/rtxError.h"
#include "rtxsrc/rtxErrCodes.h"
#include "rtxsrc/rtxMemory.h"
/* START NOOSS */
#include "rtxsrc/rtxContext.hh"
#include "rtxsrc/rtxMemHeap.hh"
#include "rtxsrc/rtxPrintStream.h"
#include "rtxsrc/rtxStream.h"
#ifndef _OS_NOPATTERN
#include "rtxsrc/rtxPattern.h"
#endif
#ifdef RTEVAL
#define _CRTLIB
#include "rtxevalsrc/rtxEval.hh"
#else
#define OSRT_CHECK_EVAL_DATE_STAT0(pctxt,stat)
#define OSRT_CHECK_EVAL_DATE_STAT1(pctxt,stat)
#define OSRT_CHECK_EVAL_DATE0(pctxt)
#define OSRT_CHECK_EVAL_DATE1(pctxt)
#endif /* RTEVAL */
/* END NOOSS */

/* Run-time context */

EXTRTMETHOD int rtxInitContextBuffer
(OSCTXT* pctxt, OSOCTET* bufaddr, OSSIZE bufsiz)
{
   if (pctxt->initCode != OSCTXTINIT) {
      return LOG_RTERRNEW (pctxt, RTERR_NOTINIT);
   }
   /* If buffer exists and was allocated using sys alloc, free it */
   if (0 != pctxt->buffer.data && pctxt->buffer.dynamic &&
       0 != (pctxt->flags & OSBUFSYSALLOC)) {
      rtxMemSysFreePtr (pctxt, pctxt->buffer.data);
   }
   pctxt->flags &= ~OSBUFSYSALLOC;

   if (bufaddr == 0) {
      /* dynamic buffer */
      if (bufsiz == 0) bufsiz = OSRTENCBUFSIZ;
      /* START NOOSS */
#ifndef _NO_STREAM
      if (0 != pctxt->pStream) {
         if (0 != (pctxt->pStream->flags & OSRTSTRMF_BUFFERED)) {
            pctxt->buffer.data = rtxMemSysAlloc (pctxt, bufsiz);
            if (0 == pctxt->buffer.data) return LOG_RTERR (pctxt, RTERR_NOMEM);
            pctxt->flags |= OSBUFSYSALLOC;
         }
      }
      else
#endif /* _NO_STREAM */
         /* END NOOSS */
      {
         pctxt->buffer.data = rtxMemAllocArray (pctxt, bufsiz, OSOCTET);
         if (0 == pctxt->buffer.data) return LOG_RTERR (pctxt, RTERR_NOMEM);
      }
      pctxt->buffer.size = bufsiz;
      pctxt->buffer.dynamic = TRUE;
   }
   else {
      /* static buffer */
      pctxt->buffer.data = (OSOCTET*) bufaddr;
      pctxt->buffer.size = bufsiz;
      pctxt->buffer.dynamic = FALSE;
   }

   /* START NOOSS */
   OSRT_CHECK_EVAL_DATE1 (pctxt);
   /* END NOOSS */
   pctxt->buffer.byteIndex = 0;
   pctxt->buffer.bitOffset = 8;

   return 0;
}

EXTRTMETHOD int rtxPreInitContext (OSCTXT* pctxt)
{
   /* void* pudata = pctxt->pUserData; */
   OSCRTLMEMSET (pctxt, 0, sizeof(OSCTXT));
   /* pctxt->pUserData = pudata; */

   rtxStackInit(pctxt, &pctxt->containerEndIndexStack);

   /* START NOOSS */
   OSRT_CHECK_EVAL_DATE0 (pctxt);
   /* END NOOSS */
   pctxt->initCode = OSCTXTINIT;
   pctxt->buffer.bitOffset = 8; /* need for bit encoding to stream */
   pctxt->indent = 0;
   return 0;
}

/* START NOOSS */
EXTRTMETHOD int rtxCheckContext (OSCTXT* pctxt)
{
   if (pctxt->initCode != OSCTXTINIT) {
      return RTERR_NOTINIT;
   }
   OSRT_CHECK_EVAL_DATE1 (pctxt);
   return 0;
}
/* END NOOSS */

static void rtxFreeAppInfo (OSCTXT* pctxt, void* pAppInfo)
{
   if (0 != pAppInfo) {
      /* The first element of pAppInfo always MUST BE a pointer to
         free function. */
      OSFreeCtxtAppInfoPtr freeFunc = *(OSFreeCtxtAppInfoPtr*)pAppInfo;
      if (0 != freeFunc) {
         /* The free function points to the app info free call, e.g.,
          * rtCtxtFreeASN1Info. */
         freeFunc (pctxt);
      }
      rtxMemSysFreePtr (pctxt, pAppInfo);
   }
}

EXTRTMETHOD void rtxFreeContext (OSCTXT* pctxt)
{
   pctxt->flags |= OSRESERVED1;
   /* START NOOSS */
#ifndef _OS_NOPATTERN
   rtxFreeRegexpCache(pctxt);
#endif
   /* END NOOSS */
   rtxErrReset (pctxt);
   rtxFreeAppInfo (pctxt, pctxt->pXMLInfo);
   pctxt->pXMLInfo = 0;
   rtxFreeAppInfo (pctxt, pctxt->pASN1Info);
   pctxt->pASN1Info = 0;
   rtxFreeAppInfo (pctxt, pctxt->pLicInfo);
   pctxt->pLicInfo = 0;
#ifndef _NO_STREAM
   rtxStreamRelease (pctxt);
   rtxPrintStreamRelease (pctxt);
#endif
   if (pctxt->buffer.dynamic && pctxt->buffer.data) {
#ifndef _NO_STREAM
      if (pctxt->pStream &&
          (pctxt->pStream->flags & OSRTSTRMF_BUFFERED)) {
         rtxMemSysFreePtr (pctxt, pctxt->buffer.data);
      }
      else
#endif /* _NO_STREAM */
      if (!rtxMemHeapIsEmpty (pctxt)) {
         rtxMemFreePtr (pctxt, pctxt->buffer.data);
      }
      pctxt->buffer.data = 0;
   }
   rtxMemHeapRelease (&(pctxt)->pMemHeap);

   if (0 != pctxt->gblFreeFunc) {
      pctxt->gblFreeFunc (pctxt);
   }
   else
      pctxt->initCode = 0;
}

EXTRTMETHOD OSSIZE rtxCtxtGetBitOffset (OSCTXT* pctxt)
{
#ifndef _NO_STREAM
   OSSIZE byteIndex = (OSRTISSTREAM (pctxt)) ?
      pctxt->pStream->bytesProcessed : 0;

   byteIndex += pctxt->buffer.byteIndex;

   return ((byteIndex * 8) + (8 - pctxt->buffer.bitOffset));
#else /* _NO_STREAM */
   return ((pctxt->buffer.byteIndex * 8) + (8 - pctxt->buffer.bitOffset));
#endif /* _NO_STREAM */
}

EXTRTMETHOD int rtxCtxtSetBitOffset (OSCTXT* pctxt, OSSIZE offset)
{
#ifndef _NO_STREAM
   if (OSRTISSTREAM (pctxt)) {
      /* setting bit offset in stream is not supported */
      rtxErrAddStrParm (pctxt, "set bit offset in stream");
      return LOG_RTERR (pctxt, RTERR_NOTSUPP);
   }
#endif /* _NO_STREAM */
   pctxt->buffer.byteIndex = offset / 8;
   pctxt->buffer.bitOffset = (OSUINT16)(8 - (offset % 8));              \
   return 0;
}

EXTRTMETHOD OSSIZE rtxCtxtGetIOByteCount (OSCTXT* pctxt)
{
#ifndef _NO_STREAM
   if (0 != pctxt->pStream) {
      OSSIZE count;
      rtxStreamGetIOBytes (pctxt, &count);
      return count;
   }
   else
#endif /* _NO_STREAM */
      return pctxt->buffer.byteIndex;
}

EXTRTMETHOD void rtxCopyContext (OSCTXT* pdest, OSCTXT* psrc)
{
   OSCRTLSAFEMEMCPY (&pdest->buffer, sizeof(OSRTBuffer), &psrc->buffer,
         sizeof(OSRTBuffer));
   pdest->flags = psrc->flags;
   pdest->key = psrc->key;
   pdest->keylen = psrc->keylen;
#ifndef _NO_STREAM
   pdest->pStream = psrc->pStream;
#endif
}

EXTRTMETHOD int rtxCtxtPushArrayElemName
(OSCTXT* pctxt, const OSUTF8CHAR* elemName, OSSIZE idx)
{
   const char* pstr;
   char numbuf[32];
   int ret = rtxSizeToCharStr (idx, numbuf, sizeof(numbuf), 0);
   if (ret < 0) return LOG_RTERR (pctxt, ret);

   pstr = rtxStrDynJoin (pctxt, (const char*)elemName, "[", numbuf, "]", 0);
   if (0 == pstr) return LOG_RTERR (pctxt, RTERR_NOMEM);

   return rtxCtxtPushElemName (pctxt, OSUTF8(pstr));
}

EXTRTMETHOD int rtxCtxtPushElemName
(OSCTXT* pctxt, const OSUTF8CHAR* elemName)
{
   /* Add name to element name stack in context */
   return rtxDListAppend (pctxt, &pctxt->elemNameStack, (void*)elemName) ?
      0 : RTERR_NOMEM;
}

EXTRTMETHOD int rtxCtxtPushTypeName
(OSCTXT* pctxt, const OSUTF8CHAR* typeName)
{
   int stat = 0;
   if (0 == pctxt->elemNameStack.count) {
      /* Add name to element name stack in context */
      if (0 == rtxDListAppend (pctxt, &pctxt->elemNameStack, (void*)typeName)) {
         stat = RTERR_NOMEM;
      }
   }
   return stat;
}

EXTRTMETHOD OSBOOL rtxCtxtPopArrayElemName (OSCTXT* pctxt)
{
   const OSUTF8CHAR* elemName = rtxCtxtPopElemName (pctxt);
   if (0 != elemName) {
      rtxMemFreePtr (pctxt, (void*)elemName);
      return TRUE;
   }
   else return FALSE;
}

EXTRTMETHOD const OSUTF8CHAR* rtxCtxtPopElemName (OSCTXT* pctxt)
{
   const OSUTF8CHAR* elemName = 0;
   if (pctxt->elemNameStack.count > 0) {
      elemName = (const OSUTF8CHAR*) pctxt->elemNameStack.tail->data;
      rtxDListFreeNode
         (pctxt, &pctxt->elemNameStack, pctxt->elemNameStack.tail);
   }
   return elemName;
}

EXTRTMETHOD const OSUTF8CHAR* rtxCtxtPopTypeName (OSCTXT* pctxt)
{
   const OSUTF8CHAR* typeName = 0;
   if (pctxt->elemNameStack.count == 1) {
      typeName = (const OSUTF8CHAR*) pctxt->elemNameStack.tail->data;
      rtxDListFreeNode
         (pctxt, &pctxt->elemNameStack, pctxt->elemNameStack.tail);
   }
   return typeName;
}

EXTRTMETHOD int rtxCtxtSetBufPtr
(OSCTXT* pctxt, OSOCTET* bufaddr, OSSIZE bufsiz)
{
   int stat = rtxInitContextBuffer (pctxt, bufaddr, bufsiz);
   if (stat != 0) return LOG_RTERR (pctxt, stat);

   rtxCtxtSetFlag (pctxt, OSDISSTRM); /* disable stream operations */
   pctxt->state = 0;

   LCHECKX (pctxt);
   OSRT_CHECK_EVAL_DATE0 (pctxt);

   return 0;
}

EXTRTMETHOD void rtxCtxtSetFlag (OSCTXT* pctxt, OSUINT32 mask)
{
   pctxt->flags |= mask;
}

EXTRTMETHOD void rtxCtxtClearFlag (OSCTXT* pctxt, OSUINT32 mask)
{
   pctxt->flags &= ~mask;
}

EXTRTMETHOD void rtxMemHeapSetFlags (OSCTXT *pctxt, OSUINT32 flags) {
   ((OSMemHeap *)(pctxt->pMemHeap))->flags |= flags;
}

EXTRTMETHOD void rtxMemHeapClearFlags (OSCTXT *pctxt, OSUINT32 flags) {
   /*  09-04-03 MEM:  It seems to me like this function should clear out all */
   /*  of the flags, but the documentation and language suggests otherwise... */
   ((OSMemHeap *)(pctxt->pMemHeap))->flags &= ~flags;
}

/* Private function to assign memory heap in destination context (pctxt)
   to be that of the source context (pSrcCtxt). */
void rtxCtxtSetMemHeap (OSCTXT* pctxt, OSCTXT* pSrcCtxt)
{
   if (pctxt == 0 || pSrcCtxt == 0 ||
       pSrcCtxt->pMemHeap == pctxt->pMemHeap) return;

   rtxMemHeapRelease (&(pctxt)->pMemHeap);
   pctxt->pMemHeap = pSrcCtxt->pMemHeap;
   rtxMemHeapAddRef (&pctxt->pMemHeap);
}

EXTRTMETHOD OSBOOL rtxCtxtContainerHasRemBits(OSCTXT* pctxt)
{
   OSBufferIndex* pEnd;

   pEnd = (OSBufferIndex*) rtxStackPeek(&pctxt->containerEndIndexStack);
   if (pEnd == 0) {
      /* No containers started.  The end is the end of the buffer. */
      return pctxt->buffer.byteIndex < pctxt->buffer.size;
   }
   else {
      /* The last inequality may look odd, but recall the bitOffset == 8 for
         the most signifant bit. */
      return pEnd->byteIndex > pctxt->buffer.byteIndex ||
         ( pEnd->byteIndex == pctxt->buffer.byteIndex &&
            pctxt->buffer.bitOffset > pEnd->bitOffset );
   }
}

EXTRTMETHOD OSBOOL rtxCtxtContainerEnd(OSCTXT* pctxt)
{
   OSBufferIndex* pEnd;

   pEnd = (OSBufferIndex*) rtxStackPeek(&pctxt->containerEndIndexStack);
   if (pEnd == 0) {
      /* No containers started.  The end is the end of the buffer. */
      return pctxt->buffer.byteIndex == pctxt->buffer.size;
   }
   else {
      return pEnd->byteIndex == pctxt->buffer.byteIndex &&
            pctxt->buffer.bitOffset == pEnd->bitOffset;
   }
}

EXTRTMETHOD OSSIZE rtxCtxtGetContainerRemBits(OSCTXT* pctxt)
{
   OSBufferIndex end;
   OSBufferIndex* pEnd;

   pEnd = (OSBufferIndex*) rtxStackPeek(&pctxt->containerEndIndexStack);
   if (pEnd == 0) {
      /* No containers started.  The end is simply based on the buffer size */
      end.byteIndex = pctxt->buffer.size;
      end.bitOffset = 8;
      pEnd = &end;
   }

   return 8 * ( pEnd->byteIndex - pctxt->buffer.byteIndex) - pEnd->bitOffset
            + pctxt->buffer.bitOffset;
}


EXTRTMETHOD int rtxCtxtPushContainerBits(OSCTXT* pctxt, OSSIZE bits)
{
   OSBufferIndex* pEntry;  /* the entry we'll push onto stack */
   OSSIZE bytes;           /* number of full bytes in bits */

   pEntry = rtxMemAllocType(pctxt, OSBufferIndex);
   if ( pEntry == 0 ) return LOG_RTERRNEW(pctxt, RTERR_NOMEM);

   /* split bits into full bytes and bits */
   bytes = bits / 8;
   bits = bits % 8;

   /* Add bytes and bits to current position to figure out the end of container
      This looks a little weird b/c bitOffset numbers bits 8..1, high..low. */
   pEntry->byteIndex = pctxt->buffer.byteIndex + bytes;
   if ( (OSSIZE) pctxt->buffer.bitOffset < 1 + bits ) {
      pEntry->byteIndex += 1;
      pEntry->bitOffset = (OSINT16)(8 + pctxt->buffer.bitOffset - bits);
   }
   else pEntry->bitOffset =  (OSINT16)(pctxt->buffer.bitOffset - bits);

   return rtxStackPush(&pctxt->containerEndIndexStack, pEntry);
}


EXTRTMETHOD int rtxCtxtPushContainerBytes(OSCTXT* pctxt, OSSIZE bytes)
{
   OSBufferIndex* pEntry;

   pEntry = rtxMemAllocType(pctxt, OSBufferIndex);
   if ( pEntry == 0 ) return LOG_RTERRNEW(pctxt, RTERR_NOMEM);

   pEntry->byteIndex = pctxt->buffer.byteIndex + bytes;
   pEntry->bitOffset = pctxt->buffer.bitOffset;

   return rtxStackPush(&pctxt->containerEndIndexStack, pEntry);
}

EXTRTMETHOD void rtxCtxtPopContainer(OSCTXT* pctxt)
{
   OSBufferIndex* pEntry;

   pEntry = (OSBufferIndex*) rtxStackPop(&pctxt->containerEndIndexStack);
   if ( pEntry != 0 ) rtxMemFreePtr(pctxt, pEntry);
}

EXTRTMETHOD void rtxCtxtPopAllContainers(OSCTXT* pctxt)
{
   for(;;) {
      OSBufferIndex* pEntry = (OSBufferIndex*) rtxStackPop(
                                                &pctxt->containerEndIndexStack);
      if ( pEntry != 0 ) rtxMemFreePtr(pctxt, pEntry);
      else break;
   }
}

EXTRTMETHOD const char* rtxCtxtGetExpDateStr
(OSCTXT* pctxt, char* buf, OSSIZE bufsiz)
{
   time_t expireTime = LGETEXPTIME(pctxt);
   if (0 == expireTime) {
      *buf = '\0';
      return 0;
   }
   else {
      struct tm expiretm;
#if defined(_MSC_VER) && (_MSC_VER >= 1400) /* At least Visual Studio 2005 */
      localtime_s (&expiretm, &expireTime);
      asctime_s (buf, bufsiz, &expiretm);
#else /* Not Microsoft, or Microsoft older than Visual Studio 2005 */
      expiretm = *localtime (&expireTime);
      rtxStrcpy (buf, bufsiz, asctime(&expiretm));
#endif
      return buf;
   }
}


EXTRTMETHOD void rtxLicenseClose(void)
{
   LCLOSE();
}


#ifdef RTEVAL
#include "rtxevalsrc/rtxEval.c"
#endif
