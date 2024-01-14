/*
 * Copyright (c) 2003-2023 Objective Systems, Inc.
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

#include "rtxsrc/rtxContext.hh"
#include "rtxsrc/rtxMemory.h"
#include "rtxsrc/rtxError.h"

#ifdef RTEVAL
#define _CRTLIB
#include "rtxevalsrc/rtxEval.hh"
#else
#define OSRT_CHECK_EVAL_DATE_STAT0(pctxt,stat)
#define OSRT_CHECK_EVAL_DATE_STAT1(pctxt,stat)
#define OSRT_CHECK_EVAL_DATE0(pctxt)
#define OSRT_CHECK_EVAL_DATE1(pctxt)
#define OSRT_PRINT_LIC_INFO()
#endif /* RTEVAL */

#ifdef _MEMDEBUG
extern OSCTXT _g_dbg_ctxt;
#endif

EXTRTMETHOD int rtxInitContext (OSCTXT* pctxt)
{
   int stat;

   rtxErrInit ();  /* initialize error processing */
   rtxPreInitContext (pctxt);

   stat = rtxMemHeapCreate (&pctxt->pMemHeap);

   OSRT_PRINT_LIC_INFO();

   return stat;
}

/*
 * This function initializes a context using a run-time key.  This form
 * is required for evaluation and limited distribution software.  The
 * compiler will generate a macro for rtXmlInitContext in the rtkey.h
 * file that will invoke this function with the generated run-time key.
 */
EXTRTMETHOD int rtxInitContextUsingKey
(OSCTXT* pctxt, const OSOCTET* key, OSSIZE keylen)
{
   int stat = rtxInitContext (pctxt);

   pctxt->key = key;
   pctxt->keylen = keylen;

   LCHECKX (pctxt);

   return stat;
}

EXTRTMETHOD int rtxInitThreadContext (OSCTXT* pctxt, const OSCTXT* pSrcCtxt)
{
   rtxInitContext (pctxt);
   pctxt->pGlobalData = pSrcCtxt->pGlobalData;
   return 0;
}
