/*
 * Copyright (c) 1997-2018 by Objective Systems, Inc.
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

#include "rtbersrc/asn1ber.h"
#include "rtsrc/asn1intl.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NUM_SEGMENT_BYTES  12	/* # of bytes to display in a segment */
#define DELTA              16384

typedef struct {
   int idx;
   ASN1TAG tag;
   int len;
} BLOCK;

static void xu_putBuff(OSRTBuffer *buffer_p, void *src, int len)
{
   if (!buffer_p->data) {
      buffer_p->size = DELTA;
      buffer_p->data = (OSOCTET*)malloc(buffer_p->size);
   }
   else if (buffer_p->byteIndex + len >= buffer_p->size) {
      OSOCTET* ptr;
      size_t tmpsize =
         buffer_p->size + DELTA + buffer_p->byteIndex + len;

      ptr = (OSOCTET*)realloc(buffer_p->data, tmpsize);

      if (0 != ptr) {
         buffer_p->data = ptr;
         buffer_p->size = tmpsize;
      }
      else return;
   }
   if (0 != buffer_p->data) {
      memcpy (&buffer_p->data[buffer_p->byteIndex], src, len);
      buffer_p->byteIndex += len;
   }
}

static int xu_make_ref_list(OSRTBuffer *buf, OSCTXT *ctxt, OSSIZE totalLen)
{
   ASN1TAG tag;
   BLOCK   block;
   OSSIZE  bidx, curidx = 0;
   int len, realLen = 0;
   int stat;

   while (curidx < totalLen && xd_tag_len(ctxt, &tag, &len, XM_SKIP) == 0) {
      bidx = ctxt->buffer.byteIndex;
      if (tag & TM_CONS) {
         int iniLen = len, cnt;

         if ((tag&0xFF) == 64)
            tag = tag;
         if (len == ASN_K_INDEFLEN) {
            /*  calculate actual len */
            len = xd_indeflen(ctxt->buffer.data + ctxt->buffer.byteIndex);
            if (len < 0)
               return LOG_RTERR(ctxt, len);
            stat = xd_match(ctxt, tag, NULL, XM_ADVANCE);
            if (stat != 0)
               return LOG_RTERR(ctxt, stat);
            /*  skip length of tag and indicator of indef len */
            len -= (int)(ctxt->buffer.byteIndex - bidx);

            /* skip EOC */
            len -= 2;
         }
         else {
            stat = xd_match(ctxt, tag, NULL, XM_ADVANCE);
            if (stat != 0)
               return LOG_RTERR(ctxt, stat);
         }
         cnt = xu_make_ref_list(buf, ctxt, len);
         if (cnt < 0)
            return LOG_RTERR(ctxt, cnt);

         block.idx = (int)ctxt->buffer.byteIndex;
         block.tag = tag;
         block.len = cnt;
         xu_putBuff(buf, &block, sizeof(block));

         if (iniLen == ASN_K_INDEFLEN) {
            stat = xd_match(ctxt, 0, NULL, XM_ADVANCE);
            if (stat != 0)
               return LOG_RTERR(ctxt, stat);
         }
      }
      else {
         stat = xd_NextElement(ctxt);
         if (stat != 0)
            return LOG_RTERR(ctxt, stat);

         block.idx = (int) ctxt->buffer.byteIndex;
         block.tag = tag;
         block.len = len;
         xu_putBuff(buf, &block, sizeof(block));
      }

      curidx += (ctxt->buffer.byteIndex - bidx);
      realLen ++;
   }
   return realLen;
}

static int xu_to_def_len2
(OSCTXT *ctxt, int totalLen, OSCTXT *ectxt, BLOCK *ref, int *refSize)
{
   int i, realLen = 0;
   for (i = *refSize - 1; i >= 0 && totalLen; i--) {
      int off = ref[i].idx;
      int len = ref[i].len;
      int stat;
      ASN1TAG tag = ref[i].tag;

      totalLen --;
      if (!(tag & TM_CONS)) {
         stat = xe_memcpy(ectxt, ctxt->buffer.data + off - len, len);
         if (stat < 0)
            return LOG_RTERR(ctxt, stat);
         }
      else {
         len = xu_to_def_len2(ctxt, len, ectxt, ref, &i);
         if (len < 0)
            return LOG_RTERR(ctxt, len);
      }
      stat = xe_tag_len(ectxt, tag, len);
      if (stat < 0)
         return LOG_RTERR(ctxt, stat);

      realLen += stat;
      *refSize = i;
   }
   return realLen;
}

int berIndefToDefLen (OSCTXT *ctxt, OSCTXT *ectxt)
{
   int stat;
   OSRTBuffer refBuf;
   OSRTBufSave savedBufferInfo, savedBufferInfo2;
   BLOCK* ref;
   int refSize;

   memset (&refBuf, 0, sizeof(refBuf));

   stat = rtInitContext (ectxt);
   if (0 == stat) {
     stat = xe_setp (ectxt, NULL, (int)ctxt->buffer.size);
   }
   if (0 == stat) {
      xu_SaveBufferState (ctxt, &savedBufferInfo);
      stat = xu_make_ref_list (&refBuf, ctxt, ctxt->buffer.size);
   }
   if (stat < 0) {
      memcpy(&ectxt->errInfo, &ctxt->errInfo, sizeof(ctxt->errInfo));
      if (0 != refBuf.data) {
         free(refBuf.data);
      }
      return LOG_RTERR(ectxt, stat);
   }
   xu_SaveBufferState (ctxt, &savedBufferInfo2);
   xu_RestoreBufferState (ctxt, &savedBufferInfo);
   ref = (BLOCK*) refBuf.data;
   refSize = (int)(refBuf.byteIndex/sizeof(BLOCK));

   stat = xu_to_def_len2(ctxt, (int)ctxt->buffer.size, ectxt, ref, &refSize);
   if (stat < 0) {
      memcpy(&ectxt->errInfo, &ctxt->errInfo, sizeof(ctxt->errInfo));
      return LOG_RTERR(ectxt, stat);
   }
   xu_RestoreBufferState (ctxt, &savedBufferInfo2);
   return 0;
}

static int xu_to_indef_len2
(OSCTXT *ctxt, OSRTBuffer *buf_p, OSSIZE totalLen, OSCTXT *ectxt)
{
   ASN1TAG tag;
   int len;
   int eoc = 0, stat;
   OSSIZE bidx, curidx = 0;

   while (curidx < totalLen && xd_tag_len(ctxt, &tag, &len, XM_SKIP) == 0) {
      if (tag == ASN_ID_EOC) {
         /* skip EOC */
         stat = xd_match(ctxt, tag, NULL, XM_ADVANCE);
         if (stat != 0)
            return LOG_RTERR(ctxt, stat);
         curidx += 2;
         continue;
      }
      bidx = ctxt->buffer.byteIndex;
      if (tag & TM_CONS) {
         OSRTBufSave esavedBufferInfo;
         int elLen = 0;

         if (len == ASN_K_INDEFLEN) {
            /* calculate actual len */
            len = xd_indeflen_ex
               (ctxt->buffer.data + ctxt->buffer.byteIndex,
                (int)(ctxt->buffer.size - ctxt->buffer.byteIndex));
            if (len < 0)
               return LOG_RTERR(ctxt, len);
            stat = xd_match(ctxt, tag, NULL, XM_ADVANCE);
            if (stat != 0)
               return LOG_RTERR(ctxt, stat);
            /* skip length of tag and indicator of indef len */
            len -= (int)(ctxt->buffer.byteIndex - bidx);
         }
         else {
            stat = xd_match(ctxt, tag, NULL, XM_ADVANCE);
            if (stat != 0)
               return LOG_RTERR(ctxt, stat);
         }
         xu_SaveBufferState (ectxt, &esavedBufferInfo);
         elLen = xe_TagAndIndefLen(ectxt, tag, len);
         xu_putBuff(buf_p, &ectxt->buffer.data[ectxt->buffer.byteIndex],
            elLen - len);
         xu_RestoreBufferState (ectxt, &esavedBufferInfo);
         stat = xu_to_indef_len2(ctxt, buf_p, len, ectxt);
         if (stat != 0)
            return LOG_RTERR(ctxt, stat);
         xu_putBuff(buf_p, &eoc, 2);
         curidx += (ctxt->buffer.byteIndex - bidx);
      }
      else {
         stat = xd_NextElement(ctxt);
         if (stat != 0)
            return LOG_RTERR(ctxt, stat);
         len = (int)(ctxt->buffer.byteIndex - bidx);
         xu_putBuff(buf_p, &ctxt->buffer.data[bidx], len);
         curidx += len;
      }
   }
   return 0;
}

int berDefToIndefLen (OSCTXT *ctxt, OSCTXT *destCtxt)
{
   int stat;
   OSCTXT ectxt;

   memset (destCtxt, 0, sizeof(*destCtxt));

   stat = rtInitContext (&ectxt);
   if (0 == stat) {
     stat = xe_setp (&ectxt, NULL, (int)ctxt->buffer.size);
   }
   if (0 == stat) {
      stat = xu_to_indef_len2
         (ctxt, &destCtxt->buffer, ctxt->buffer.size, &ectxt);
   }
   if (stat != 0) {
      memcpy(&destCtxt->errInfo, &ctxt->errInfo, sizeof(ctxt->errInfo));
      return LOG_RTERR(destCtxt, stat);
   }
   destCtxt->buffer.size = destCtxt->buffer.byteIndex;

   return 0;
}

