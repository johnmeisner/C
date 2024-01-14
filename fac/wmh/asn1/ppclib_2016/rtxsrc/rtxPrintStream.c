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

#include <stdio.h>
#include <stdarg.h>
#include "rtxsrc/rtxPrintStream.h"
#include "rtxsrc/rtxError.h"
#include "rtxsrc/rtxMemory.h"

#if !defined(__SYMBIAN32__)
OSRTPrintStream g_PrintStream = {0,0}; /*!AB, 06/21/05, initialization is
 necessary for successful compilation of dynamic libraries on Mac OS/X.
 Otherwise, 'ld: common symbols not allowed with MH_DYLIB output format
 with the -multi_module option' error will occur. */

EXTRTMETHOD int rtxSetGlobalPrintStream
(rtxPrintCallback myCallback, void * pStrmInfo)
{
   if (myCallback == 0)
      return RTERR_INVPARAM;
   else {
      g_PrintStream.pfPrintFunc = myCallback;
      g_PrintStream.pPrntStrmInfo = pStrmInfo;
   }
   return 0;
}
#endif

EXTRTMETHOD int rtxSetPrintStream
(OSCTXT* pctxt, rtxPrintCallback myCallback,  void * pStrmInfo)
{
   if (myCallback == 0)
      return LOG_RTERR (pctxt, RTERR_INVPARAM);
   else {
      rtxPrintStreamRelease (pctxt);

      pctxt->pPrintStrm = (OSRTPrintStream*)
         rtxMemSysAlloc (pctxt, sizeof(OSRTPrintStream));
      if (pctxt->pPrintStrm == NULL) return LOG_RTERR (pctxt, RTERR_NOMEM);

      pctxt->pPrintStrm->pfPrintFunc = myCallback;
      pctxt->pPrintStrm->pPrntStrmInfo = pStrmInfo;
      pctxt->indent = 0U;
   }
   return 0;
}

/*
 * Print to context PrintStream if set, else print to global PrintStream
 * if set, else print to stdout
 */
EXTRTMETHOD int rtxPrintToStream (OSCTXT *pctxt, const char* fmtspec, ...)
{
   va_list  arglist;
   va_start (arglist, fmtspec);

   if (pctxt != 0 && pctxt->pPrintStrm != 0) {
      ((rtxPrintCallback)pctxt->pPrintStrm->pfPrintFunc)
         (pctxt->pPrintStrm->pPrntStrmInfo, fmtspec, arglist);
   }
#if !defined(__SYMBIAN32__)
   else if (g_PrintStream.pfPrintFunc != 0) {
      ((rtxPrintCallback)g_PrintStream.pfPrintFunc)
         (g_PrintStream.pPrntStrmInfo, fmtspec, arglist);
   }
#endif
   else {
      vfprintf (stdout, fmtspec, arglist);
      fflush (stdout);
   }

   va_end (arglist);

   return 0;
}

EXTRTMETHOD int rtxDiagToStream
(OSCTXT* pctxt, const char* fmtspec, va_list arglist)
{
   if (pctxt != 0 && pctxt->pPrintStrm != 0) {
      ((rtxPrintCallback)pctxt->pPrintStrm->pfPrintFunc)
         (pctxt->pPrintStrm->pPrntStrmInfo, fmtspec, arglist);
   }
#if !defined(__SYMBIAN32__)
   else if (g_PrintStream.pfPrintFunc != 0) {
      ((rtxPrintCallback)g_PrintStream.pfPrintFunc)
         (g_PrintStream.pPrntStrmInfo, fmtspec, arglist);
   }
#endif
   else {
      vfprintf (stdout, fmtspec, arglist);
      fflush (stdout);
   }
   return 0;
}

EXTRTMETHOD int rtxPrintStreamRelease (OSCTXT* pctxt)
{
   if (0 == pctxt) return RTERR_INVPARAM;

   if (pctxt->pPrintStrm != 0) {
      rtxMemSysFreePtr (pctxt, pctxt->pPrintStrm);
      pctxt->pPrintStrm = 0;
   }

   return 0;
}

EXTRTMETHOD void rtxPrintStreamToStdoutCB
(void* pPrntStrmInfo, const char* fmtspec, va_list arglist)
{
   vprintf (fmtspec, arglist);
   return;
}

EXTRTMETHOD void rtxPrintStreamToFileCB
(void* pPrntStrmInfo, const char* fmtspec, va_list arglist)
{
   FILE * fp = (FILE*) pPrntStrmInfo;
   vfprintf (fp, fmtspec, arglist);
   return;
}

