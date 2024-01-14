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

#include "rtpersrc/pe_common.hh"
#include "rtxsrc/rtxCommonDefs.h"
#include "rtxsrc/rtxStream.h"

EXTPERMETHOD int pe_OpenTypeStart
(OSCTXT* pctxt, OSUINT32* pPos, void** ppPerField)
{
   size_t idx = pctxt->buffer.byteIndex;
   unsigned bitOff = 8 - pctxt->buffer.bitOffset;
   OSUINT32 flag = 0;

#ifndef _NO_STREAM
   /* prohibit buffer from flushing to stream */
   if (OSRTISSTREAM (pctxt)) {
      if (!(pctxt->pStream->flags & OSRTSTRMF_FIXINMEM)) {
         pctxt->pStream->flags |= OSRTSTRMF_FIXINMEM;
         flag = 1;
      }
   }
#endif /* _NO_STREAM */

   if (pctxt->buffer.aligned && bitOff != 0) {
#ifdef _TRACE
      pctxt->buffer.byteIndex++;
#endif
      pctxt->buffer.bitOffset = 8;
      bitOff = 0;
      idx++;
   }

   if (bitOff == 0) {
      if (idx + 1 > pctxt->buffer.size) {
         /* expand buffer */
         int stat = pe_ExpandBuffer (pctxt, 1);
         if (stat != 0) return LOG_RTERR (pctxt, stat);
      }

      pctxt->buffer.data[idx] = 0;
   }
   else {
      /* unaligned case; otherwise bitOff would have been set to zero above.
         idx has not been incremented and equals buffer.byteIndex */
      if (idx + 1 >= pctxt->buffer.size) {
         /* expand buffer */
         int stat = pe_ExpandBuffer (pctxt, 1);
         if (stat != 0) return LOG_RTERR (pctxt, stat);
      }

      pctxt->buffer.data[idx + 1] = 0;
   }

   PU_NEWFIELD (pctxt, "segment length");

   /* skip 8 bit for open type length.
      Encoding of the type will begin at idx + 1.
   */
   pctxt->buffer.byteIndex = idx + 1;

   PU_SETBITCOUNT (pctxt);

   *pPos = (OSUINT32) ((idx << 4) | (bitOff << 1) | flag);

#ifdef _TRACE
   *ppPerField = ACINFO(pctxt)->fieldList.tail;
#else
   *ppPerField = 0;
#endif

   return 0;
}

EXTPERMETHOD int pe_OpenTypeEnd
(OSCTXT* pctxt, OSUINT32 pos, void* pPerField)
{
   /* Recover the index and bit offset from pos, as encoded into it by
      pe_OpenTypeStart.
   */
   size_t idx = pos >> 4;
   OSUINT32 bitOff = (pos >> 1) & 7;

   /* idx + 1 gives start of encoded type.  len = length of that encoding. */
   OSUINT32 len = (OSUINT32) (pctxt->buffer.byteIndex - idx - 1);
   OSUINT32 curBitOff = 8 - pctxt->buffer.bitOffset;

   OSUINT32 nmBytes = 0; /* additional bytes */

   /* ext == 0 if type's encoding began on byte boundary; 1 otherwise.
      If we're doing PER aligned, bitOff will have been forced to zero and ext
      will be 0.
   */
   OSUINT32 ext = (OSUINT32) (bitOff != 0);

   OSUINT32 segs64 = 0;
   OSUINT32 segs16 = 0;
   OSUINT32 tail = 0;

   OSBOOL needClearByte = FALSE;

#ifndef _NO_STREAM
   if (OSRTISSTREAM (pctxt)) {
      if (pos & 1)
         pctxt->pStream->flags &= ~OSRTSTRMF_FIXINMEM;
   }
#endif /* _NO_STREAM */

   if (len == 0 && curBitOff == bitOff) {
      /* Encoding is empty.  Procedures of 11.1 call for encoding a single
         zero octet. */
      len = 1;

      if (curBitOff == 0)
         pctxt->buffer.data[pctxt->buffer.byteIndex] = 0;
      else
         needClearByte = TRUE;

      pctxt->buffer.byteIndex++;
   }
   else if (curBitOff > bitOff) {
      /* align */
      len++;
      pctxt->buffer.byteIndex++;

      if (bitOff != 0)
         needClearByte = TRUE;
   }
   else if (curBitOff == 0 && bitOff != 0)
      needClearByte = TRUE;

   if (len >= 16384) {
      /* count fragments */
      segs64 = len >> 16;
      segs16 = (len >> 14) & 3;
      nmBytes += segs64 + (segs16 != 0);

      tail = len & 0x3FFFu;
      if (tail >= 128)
         nmBytes++; /* second byte for length record */
   }
   else if (len >= 128)
      nmBytes++; /* second byte for length record */

   if (pctxt->buffer.byteIndex + nmBytes + ext >= pctxt->buffer.size) {
      /* expand buffer */
      int stat = pe_ExpandBuffer (pctxt, nmBytes + ext + 1);
      if (stat != 0) return LOG_RTERR (pctxt, stat);
   }

   pctxt->buffer.bitOffset = (OSINT16)(8 - bitOff);

   if (needClearByte)
      pctxt->buffer.data[pctxt->buffer.byteIndex] = 0;

   pctxt->buffer.byteIndex += nmBytes;

   if (len < 128) {
      /* write one byte length to reserved position */
      if (bitOff == 0)
         pctxt->buffer.data[idx] = (OSOCTET) len;
      else {
         OSOCTET* p = pctxt->buffer.data + idx;
         *p++ |= (OSOCTET) (len >> bitOff);
         *p |=  (OSOCTET) (len << (8 - bitOff));
      }
   }
   else {
      if (len < 16384) {
         /* shift data to one byte and write two byte length */
         OSOCTET* p = pctxt->buffer.data + idx;

         if (bitOff == 0) {
            OSCRTLMEMMOVE (p + 2, p + 1, len);
            *p++ = (OSOCTET) ((len >> 8) | 0x80);
            *p = (OSOCTET) len;
         }
         else {
            OSCRTLMEMMOVE (p + 2, p + 1, len + 1);
            *p++ |= (OSOCTET) ((len | 0x8000 ) >> (bitOff + 8));
            *p++ =  (OSOCTET) (len >> bitOff);
            *p |=  (OSOCTET) (len << (8 - bitOff));
         }
#ifdef _TRACE
         /* correct bit field offsets */
         {
            OSRTSListNode* pNode = (OSRTSListNode*) pPerField;
            PERField* pField;

            /* expand len */
            if (pNode) {
               pField = (PERField*) pNode->data;
               pField->numbits += 8;
               pNode = pNode->next;
            }

            /* shift data */
            while (pNode) {
               pField = (PERField*) pNode->data;
               pField->bitOffset += 8;
               pNode = pNode->next;
            }
         }
#endif
      }
      else {
         /* Fragmentation.  The general approach is that the type has already
            been encoded, without fragmentation, at idx + 1, with the previous
            byte reserved for use as the length.  The encoding must now be
            divided into fragments: first, some number of 64k-byte
            fragments, then a fragment of 0-3 * 16K bytes, and finally a tail
            fragment of 0-16383 bytes.  Each fragment is preceeded by a 1 byte
            length.  So, we figure out how many of each fragment we have, and
            then shift the data to the right, starting with the tail and moving
            left to the first 64K-byte fragment, making room for, and filling
            in, the fragment lengths.
         */

         /* ps: the next fragment to move. Starts out pointing to the tail. */
         OSOCTET* ps = pctxt->buffer.data + idx + 1 + len - tail;

         /* p: pointer to where fragment at ps needs to go, in order to make
            room for fragment lengths.
         */
         OSOCTET* p = ps + nmBytes;
         OSUINT32 tm;

         if (tail > 0) /* move tail */
            OSCRTLMEMMOVE (p, ps, tail + ext);

         if (ext) { /* unaligned */
            /* write tail length */
            if (tail < 128) {
               *p = (OSOCTET) ((*p & (0xFFu >> bitOff)) |
                               (tail << (8 - bitOff)));
               p--;
               *p = (OSOCTET) ((*ps & (0xFF00u >> bitOff)) |
                               (tail >> bitOff));
            }
            else {
               *p = (OSOCTET) ((*p & (0xFFu >> bitOff)) |
                               (tail << (8 - bitOff)));
               p--;
               *p = (OSOCTET) (tail >> bitOff);
               p--;
               *p = (OSOCTET) ((*ps & (0xFF00u >> bitOff)) |
                              ((tail | 0x8000 ) >> ((bitOff) + 8)));
            }

            if (segs64 > 0 && segs16 > 0) {
               OSUINT32 sz = len & 0xC000u;
               tm = 0xC0u | segs16;

               ps -= sz;
               p -= sz;

               /* move n*16K segment */
               OSCRTLMEMMOVE (p, ps, sz);

               /* write segs16 length */
               *p = (OSOCTET) ((*p & (0xFFu >> bitOff)) |
                               (tm << (8 - bitOff)));
               p--;
               *p = (OSOCTET) ((*ps & (0xFF00u >> bitOff)) |
                               (tm >> bitOff));
            }

            if (segs64 > 1) {
               OSUINT32 segs = segs64 - 1;

               while (segs--) {
                  ps -= 0x10000ul;
                  p -= 0x10000ul;

                  /* move 64K segment */
                  OSCRTLMEMMOVE (p, ps, 0x10000ul);

                  /* write segs64 length */
                  *p = (OSOCTET) ((*p & (0xFFu >> bitOff)) |
                                  (0xC4u << (8 - bitOff)));
                  p--;
                  *p = (OSOCTET) ((*ps & (0xFF00u >> bitOff)) |
                                  (0xC4u >> bitOff));
               }
            }

            /* write first length */
            p = pctxt->buffer.data + idx;
            tm = 0xC0 | ((segs64 > 0) ? 4u : segs16);

            *p++ |= (OSOCTET) (tm >> bitOff);
            *p |= (OSOCTET) (tm << (8 - bitOff));
         }
         else { /* aligned */
            /* write tail length */
            if (tail < 128) {
               *--p = (OSOCTET) tail;
            }
            else {
               *--p = (OSOCTET) tail;
               *--p = (OSOCTET) ((tail >> 8) | 0x80);
            }

            if (segs64 > 0 && segs16 > 0) {
               OSUINT32 sz = len & 0xC000u;
               tm = 0xC0u | segs16;

               ps -= sz;
               p -= sz;

               /* move n*16K segment */
               OSCRTLMEMMOVE (p, ps, sz);

               /* write segs16 length */
               *--p = (OSOCTET) tm;
            }

            if (segs64 > 1) {
               OSUINT32 segs = segs64 - 1;

               while (segs--) {
                  ps -= 0x10000ul;
                  p -= 0x10000ul;

                  /* move 64K segment */
                  OSCRTLMEMMOVE (p, ps, 0x10000ul);

                  /* write segs64 length */
                  *--p = (OSOCTET) 0xC4u;
               }
            }

            /* write first length */
            p = pctxt->buffer.data + idx;
            tm = 0xC0 | ((segs64 > 0) ? 4u : segs16);

            *p = (OSOCTET) tm;
         }

#ifdef _TRACE
         /* correct bit fields */
         {
            OSRTSListNode* pNode = (OSRTSListNode*) pPerField;
            OSRTSList* pList = &ACINFO(pctxt)->fieldList;
            PERField* pField = 0;
            size_t border = 0;
            size_t offset = 0;
            OSUINT32 nm = segs64 + (segs16 != 0);

            /* skip length */
            if (pNode) {
               pField = (PERField*) pNode->data;
               border = pField->bitOffset + 8;

               if (segs64 > 0) {
                  segs64--;
                  border += 0x10000ul * 8;
               }
               else
                  border += (len & 0xC000u) * 8;

               pNode = pNode->next;

               /* skip fields in first segment */
               while (pNode) {
                  pField = (PERField*) pNode->data;
                  if (pField->bitOffset + pField->numbits > border)
                     break;
                  pNode = pNode->next;
               }
            }

            if (0 != pField) {
               while (nm--) {
                  OSRTSListNode* pnd;
                  PERField* ptm;

                  if (pNode) {
                     if (pField->bitOffset < border) {
                        /* split current field */
                        size_t nameSz = OSCRTLSTRLEN (pField->name);

                        pnd = rtxMemAllocType (pctxt, OSRTSListNode);
                        if (pnd == 0) return LOG_RTERR (pctxt, RTERR_NOMEM);

                        ptm = rtxMemAllocType (pctxt, PERField);
                        if (ptm == 0) {
                           rtxMemFreePtr (pctxt, pnd);
                           return LOG_RTERR (pctxt, RTERR_NOMEM);
                        }

                        ptm->name = (char *) rtxMemAlloc (pctxt, nameSz + 4);
                        if (ptm->name == 0) {
                           rtxMemFreePtr (pctxt, ptm);
                           rtxMemFreePtr (pctxt, pnd);
                           return LOG_RTERR (pctxt, RTERR_NOMEM);
                        }

                        OSCRTLMEMCPY
                           ((void*)(ptm->name), pField->name, nameSz + 1);

                        if (nameSz >= 4 &&
                            pField->name[nameSz - 1] != '.' &&
                            pField->name[nameSz - 2] != '.' &&
                            pField->name[nameSz - 3] != '.' &&
                            pField->name[nameSz - 4] != ' ')
                           OSCRTLMEMCPY
                              ((void*) (ptm->name + nameSz), " ...", 5);

                        ptm->bitOffset = border - offset;
                        ptm->numbits =
                           pField->bitOffset + pField->numbits - border;
                        pField->numbits = border - pField->bitOffset;
                        ptm->openTypeFields = 0;

                        pnd->data = ptm;
                        pnd->next = pNode->next;
                        pNode->next = pnd;
                        if (pnd->next == 0)
                           pList->tail = pnd;

                        pList->count++;
                     }
                  }

                  /* insert length field */
                  pnd = rtxMemAllocType (pctxt, OSRTSListNode);
                  if (pnd == 0) return LOG_RTERR (pctxt, RTERR_NOMEM);

                  ptm = rtxMemAllocType (pctxt, PERField);
                  if (ptm == 0) {
                     rtxMemFreePtr (pctxt, pnd);
                     return LOG_RTERR (pctxt, RTERR_NOMEM);
                  }

                  ptm->name = (char *) rtxMemAlloc (pctxt, 15);
                  if (ptm->name == 0) {
                     rtxMemFreePtr (pctxt, ptm);
                     rtxMemFreePtr (pctxt, pnd);
                     return LOG_RTERR (pctxt, RTERR_NOMEM);
                  }

                  OSCRTLMEMCPY ((void*) (ptm->name), " segment length", 15);

                  ptm->bitOffset = border;
                  ptm->numbits = (nm == 0 && tail >= 128) ? 16 : 8;
                  ptm->openTypeFields = 0;

                  pnd->data = ptm;

                  if (pNode) {
                     pnd->next = pNode->next;
                     pNode->next = pnd;

                     if (pnd->next == 0)
                        pList->tail = pnd;
                  }
                  else { /* last length */
                     pnd->next = 0;
                     if (0 == pList->tail) {
                        pList->tail = pnd;
                     }
                     else {
                        pList->tail->next = pnd;
                        pList->tail = pnd;
                     }
                  }

                  pList->count++;

                  /* set next border */
                  offset += ptm->numbits;
                  border += ptm->numbits;

                  if (segs64 > 0) {
                     segs64--;
                     border += 0x10000ul * 8;
                  }
                  else if (segs16 > 0) {
                     segs16 = 0;
                     border += (len & 0xC000u) * 8;
                  }
                  else
                     border += tail * 8;

                  /* move fields */
                  pNode = pnd->next; /* continue from field after length */

                  while (pNode) {
                     pField = (PERField*) pNode->data;
                     pField->bitOffset += offset;

                     if (pField->bitOffset + pField->numbits > border)
                        break;

                     pNode = pNode->next;
                  }
               }
            }
         }
#endif
      }
   }

   return 0;
}
