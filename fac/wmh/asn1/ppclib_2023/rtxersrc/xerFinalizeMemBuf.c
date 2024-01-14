#include "asn1xer.h"

int xerFinalizeMemBuf (OSRTMEMBUF* pMemBuf) 
{
   int stat;
   OSSIZE len;
   OSOCTET* data;
   OSCTXT* pctxt = pMemBuf->pctxt;

   /* Ensure capacity for data */
   stat = rtxMemBufPreAllocate (pMemBuf, sizeof (XMLCHAR));
   if (stat != 0) return LOG_RTERR (pMemBuf->pctxt, stat);

   /* Set termination null */
   memset (&pMemBuf->buffer[pMemBuf->usedcnt], 0, sizeof (XMLCHAR));
   pMemBuf->usedcnt += sizeof (XMLCHAR);

   data = pMemBuf->buffer + pMemBuf->startidx;
   len = pMemBuf->usedcnt - pMemBuf->startidx;

   if (0 != pctxt->buffer.data && pctxt->buffer.dynamic) {
      if (0 != (pctxt->flags & OSBUFSYSALLOC)) {
         rtxMemSysFreePtr (pctxt, pctxt->buffer.data);
         pctxt->flags &= ~OSBUFSYSALLOC;
      }
   }
   pctxt->buffer.data = data;
   pctxt->buffer.size = len - sizeof (XMLCHAR);
   pctxt->buffer.dynamic = FALSE;
   pctxt->buffer.byteIndex = 0;
   pctxt->buffer.bitOffset = 8;
   
   return 0;
}

