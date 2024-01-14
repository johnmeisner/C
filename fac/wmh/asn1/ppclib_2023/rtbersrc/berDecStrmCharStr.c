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

#include "rtbersrc/asn1berStream.h"


/**
 * Decode into a static or dynamic character array.
 * @param ppvalue Pointer to address of character string.  If dynamic, this
 *          will receive the allocated character array, otherwise it will be
 *          unchanged. If !dynamic, the const-ness will be ignored.
 * @param bufsize The size of the character array. This is only used when
 *          !dynamic.
 * @param dynamic If true, *ppvalue should be set to the dynamically
 *          allocated memory; otherwise, *ppvalue should not be modified.
 */
int berDecStrmCharStr_internal (OSCTXT *pctxt, const char** ppvalue,
                       OSSIZE bufsize, ASN1TagType tagging, ASN1TAG tag,
                      int length, OSBOOL dynamic)
{
#ifndef _NO_STREAM
   register int stat = 0;
   OSOCTET* pvalue = 0;
   int numocts;

   if (tagging == ASN1EXPL) {
      if ((stat = berDecStrmMatchTag (pctxt, tag, &length, TRUE)) != 0)
      /* RTERR_IDNOTFOU will be logged later, by the generated code,
         or reset by rtxErrReset (for optional seq elements). */
         return (stat == RTERR_IDNOTFOU) ? stat : LOG_RTERR (pctxt, stat);
   }

   if (pctxt->flags & ASN1CONSTAG) { /* constructed case */
      OSUINT32 totalLen = 0, octidx = 0;
      ASN1CCB ccb;

      if (!dynamic) pvalue = (OSOCTET*) *ppvalue;

      ccb.len = length;
      ccb.bytes = OSRTSTREAM_BYTEINDEX (pctxt);
      while (!BS_CHKEND (pctxt, &ccb)) {
         int ll;
         OSOCTET* newbuf;

         if ((stat = berDecStrmMatchTag (pctxt, ASN_ID_OCTSTR, &ll, TRUE)) != 0)
            break;
         if (ll <= 0) {
            stat = ASN_E_INVLEN;
            break;
         }

         totalLen += ll;
         if (dynamic) {
            newbuf = (OSOCTET*)rtxMemRealloc(pctxt, pvalue, totalLen + 1);
            if (newbuf == NULL) {
               stat = RTERR_NOMEM;
               break;
            }
            pvalue = newbuf;
         }
         else if (totalLen + 1 < bufsize) {
            /* buffer is not dynmaic and data doesn't fit */
            stat = RTERR_NOMEM;
            break;
         }

         stat = rtxReadBytes (pctxt, pvalue + octidx, ll);
         if (stat < 0) break; else stat = 0;

         octidx = totalLen;
      }

      stat = (stat != 0) ? stat : ccb.stat;
      if (stat != 0) {
         if ( dynamic) rtxMemFreePtr (pctxt, pvalue);
         return LOG_RTERR (pctxt, stat);
      }
      numocts = totalLen;

      if (length == ASN_K_INDEFLEN) {
         stat = berDecStrmMatchEOC (pctxt);
         if (stat != 0) {
            if ( dynamic ) rtxMemFreePtr (pctxt, pvalue);
            return LOG_RTERR (pctxt, stat);
         }
      }
   }
   else { /* primitive case */
      if (length < 0)
         return LOG_RTERR (pctxt, ASN_E_INVLEN);
      if (length > 0) {
         int numToRead = length;

         numocts = numToRead;

         if (dynamic) {
            pvalue = (OSOCTET*)rtxMemAlloc(pctxt, numToRead + 1);
            if (pvalue == 0) return LOG_RTERR(pctxt, RTERR_NOMEM);
         }
         else {
            if ((OSSIZE)(numToRead + 1) > bufsize)
               return LOG_RTERR(pctxt, RTERR_NOMEM);

            pvalue = (OSOCTET*) *ppvalue;
         }

         stat = rtxReadBytes (pctxt, pvalue, numToRead);
         if (stat != 0) {
            if ( dynamic ) rtxMemFreePtr (pctxt, pvalue);
            return LOG_RTERR (pctxt, stat);
         }
      }
      else {
         numocts = 0;
         if (dynamic) {
            pvalue = (OSOCTET*)rtxMemAlloc(pctxt, 1);
            if (pvalue == 0) return LOG_RTERR(pctxt, RTERR_NOMEM);
         }
         else pvalue = (OSOCTET*) *ppvalue;
      }
   }

   if (pvalue != 0)
      pvalue [numocts] = '\0';

   if ( dynamic ) *ppvalue = (char*)pvalue;
#endif

   return 0;
}

int berDecStrmCharStr(OSCTXT *pctxt, const char** ppvalue,
   ASN1TagType tagging, ASN1TAG tag, int length)
{
   return berDecStrmCharStr_internal(pctxt, ppvalue, 0, tagging, tag, length,
                                       TRUE);
}

int berDecStrmCharArray(OSCTXT* pctxt, char* charArray,
   OSSIZE arraySize, ASN1TagType tagging,
   ASN1TAG tag, int length)
{
   return berDecStrmCharStr_internal(pctxt, (const char **)&charArray,
                            arraySize, tagging, tag, length, FALSE);
}
