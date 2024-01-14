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

#include "rtbersrc/asn1ber.h"

static int berParseTag (OSRTBuffer* rtbufptr, ASN1TAG* ptag)
{
   ASN1TAG	class_form, id_code;
   size_t       i = rtbufptr->byteIndex;
   int  	stat = 0;

   *ptag = 0;

   if (i < rtbufptr->size) {
      OSOCTET b = rtbufptr->data[i++];
      class_form = (ASN1TAG)(b & TM_CLASS_FORM);
      class_form <<= ASN1TAG_LSHIFT;

      if ((id_code = b & TM_B_IDCODE) == 31)
      {
         int lcnt = 0;
         id_code = 0;
         do {
            if (i < rtbufptr->size) {
               b = rtbufptr->data[i++];
               id_code = (id_code * 128) + (b & 0x7F);
               if (id_code > TM_IDCODE || lcnt++ > 8)
                  return (ASN_E_BADTAG);
            }
            else {
               stat = RTERR_ENDOFBUF;
               break;
            }
         } while (b & 0x80);
      }

      *ptag = class_form | id_code;
   }
   else
      stat = RTERR_ENDOFBUF;

   rtbufptr->byteIndex = i;

   return (stat);
}

static int berParseLen (OSRTBuffer* rtbufptr, size_t* plen)
{
   size_t i = rtbufptr->byteIndex;
   int    stat = 0;

   *plen = 0;

   if (i < rtbufptr->size) {
      OSOCTET b = rtbufptr->data[i++];
      if (b > 0x80)
      {
         OSOCTET ub = (OSOCTET) (b & 0x7F);
         if (ub > 4) return ASN_E_INVLEN;
         for (*plen = 0; ub > 0; ub--)
            if (ub < rtbufptr->size) {
               b = rtbufptr->data[ub++];
               *plen = (*plen * 256) + b;
            }
            else
               return (RTERR_ENDOFBUF);
      }
      else if (b == 0x80) *plen = (size_t)ASN_K_INDEFLEN;
      else *plen = b;
   }
   else stat = RTERR_ENDOFBUF;

   rtbufptr->byteIndex = i;

   return (stat);
}

int berParseTagLen (const OSOCTET* buffer,
   size_t bufidx, size_t bufsize, ASN1TAG* ptag, size_t* plen)
{
   OSRTBuffer rtbuffer;
   int stat;

   if (0 == buffer || 0 == ptag || 0 == plen) return RTERR_INVPARAM;

   OSCRTLMEMSET (&rtbuffer, 0, sizeof(OSRTBuffer));
   rtbuffer.data = (OSOCTET*) buffer;
   rtbuffer.byteIndex = bufidx;
   rtbuffer.size = bufsize;

   stat = berParseTag (&rtbuffer, ptag);
   if (stat != 0) return stat;

   stat = berParseLen (&rtbuffer, plen);
   if (stat != 0) return stat;

   return 0;
}
