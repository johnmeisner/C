/*
 * Copyright (c) 1997-2023 Objective Systems, Inc.
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
#include "rtsrc/asn1type.h"
#include "rtsrc/rtContext.h"
#include "rtxsrc/rtxContext.hh"
#include "rtxsrc/rtxError.h"

#if defined(_NO_THREADS) && defined(_NO_MALLOC)
OSCTXT g_ctxt;
#endif

static int rtCtxtFreeASN1Info (OSCTXT* pctxt);
static int rtCtxtResetASN1Info (OSCTXT* pctxt);

/*
 * This function initializes a context using a run-time key.  This form
 * is required for evaluation and limited distribution software.  The
 * compiler will generate a macro for rtInitContext in the rtkey.h
 * file that will invoke this function with the generated run-time key.
 */
EXTRTMETHOD int rtInitContextUsingKey
(OSCTXT* pctxt, const OSOCTET* key, size_t keylen)
{
   int stat = rtInitASN1Context (pctxt);

   pctxt->key = key;
   pctxt->keylen = keylen;

   LCHECK (pctxt, 0);

   return stat;
}

/*
 * Default version of rtInitContext.  In limited and evaluation software
 * versions, this function will be overridden by a macro in the rtkey.h
 * file that will force the rtInitContextUsingKey function to be used.
 * If this version is used, the run-time key information will not get
 * set in the context and the run-time will fail at some point.
 */
EXTRTMETHOD int rtInitContext (OSCTXT* pctxt)
{
   return rtInitASN1Context (pctxt);
}

/**
 * This version of rtInitContext is for internal use by RT library
 * functions.
 */
EXTRTMETHOD int rtInitASN1Context (OSCTXT* pctxt)
{
   int stat = rtxInitContext (pctxt);
   if (0 == stat) {
      /* Add ASN.1 error codes to global table */
      rtErrASN1Init ();

      /* Init ASN.1 info block */
      stat = rtCtxtInitASN1Info (pctxt);
   }
   return stat;
}

EXTRTMETHOD int rtInitSubContext (OSCTXT* pctxt, OSCTXT* psrc)
{
   int stat = rtxPreInitContext (pctxt);
   if (stat != 0) return stat;

   rtxCtxtSetMemHeap (pctxt, psrc);
   pctxt->buffer.aligned = psrc->buffer.aligned;
   pctxt->buffer.dynamic = TRUE;
   pctxt->buffer.byteIndex = 0;
   pctxt->buffer.bitOffset = 8;
   pctxt->key = psrc->key;
   pctxt->keylen = psrc->keylen;

   stat = rtCtxtInitASN1Info (pctxt);

   LCHECK (pctxt, 0);

   return stat;
}

EXTRTMETHOD void rtFreeContext (OSCTXT* pctxt)
{
   OSBOOL dynctxt = FALSE;

   if (pctxt->initCode != OSCTXTINIT) {
      return /*ASN_E_NOTINIT*/;
   }
   dynctxt = (OSBOOL)((pctxt->flags & ASN1DYNCTXT) != 0);

   rtxFreeContext (pctxt);

   if (dynctxt) OSCRTFREE0 (pctxt);
}

EXTRTMETHOD int rtCtxtInitASN1Info (OSCTXT* pctxt)
{
   OSASN1CtxtInfo* pCtxtInfo;

   pctxt->pASN1Info = (OSASN1CtxtInfo*)
      rtxMemSysAlloc (pctxt, sizeof(OSASN1CtxtInfo));
   if (pctxt->pASN1Info == 0) return LOG_RTERRNEW (pctxt, RTERR_NOMEM);

   memset (pctxt->pASN1Info, 0, sizeof(OSASN1CtxtInfo));
   pCtxtInfo = (OSASN1CtxtInfo*)pctxt->pASN1Info;
   pCtxtInfo->pFreeFunc = rtCtxtFreeASN1Info;
   pCtxtInfo->pResetFunc = rtCtxtResetASN1Info;
   rtxSListInitEx (pctxt, &pCtxtInfo->evtHndlrList);
   /*
   rtxSListInitEx (pctxt, &pCtxtInfo->fieldList);
   rtxStackInit (pctxt, &pCtxtInfo->nameStack);
   */
   return 0;
}

static int rtCtxtFreeASN1Info (OSCTXT* pctxt)
{
   if (0 != pctxt->pASN1Info) {
      OSASN1CtxtInfo* pCtxtInfo = (OSASN1CtxtInfo*)pctxt->pASN1Info;
      if (0 != pCtxtInfo->pFreeFunc) {
         /* rtxSListFree (&pCtxtInfo->fieldList); */
         rtxSListFree (&pCtxtInfo->evtHndlrList);
      }
      return 0;
   }
   return LOG_RTERRNEW (pctxt, RTERR_NOTINIT);
}

static int rtCtxtResetASN1Info (OSCTXT* pctxt)
{
   if (0 != pctxt->pASN1Info) {
      OSASN1CtxtInfo* pCtxtInfo = (OSASN1CtxtInfo*)pctxt->pASN1Info;
      /* rtxSListFree (&pCtxtInfo->fieldList); */
      rtxSListFree (&pCtxtInfo->evtHndlrList);
      return 0;
   }
   return LOG_RTERRNEW (pctxt, RTERR_NOTINIT);
}

EXTRTMETHOD void rtSetCopyValues (OSCTXT* pctxt, OSBOOL value)
{
   rtSetFastCopy (pctxt, (OSBOOL)!value);
}

EXTRTMETHOD void rtSetFastCopy (OSCTXT* pctxt, OSBOOL value)
{
   if (value)
      rtxCtxtSetFlag (pctxt, ASN1FASTCOPY);
   else
      rtxCtxtClearFlag (pctxt, ASN1FASTCOPY);
}
