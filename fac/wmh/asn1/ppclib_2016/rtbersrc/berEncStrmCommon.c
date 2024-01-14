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

#include "rtbersrc/xse_common.hh"
#include "rtxsrc/rtxUtil.h"

/***********************************************************************
 *
 *  COMMON ENCODE FUNCTIONS
 *
 **********************************************************************/

int berEncStrmWriteOctet (OSCTXT* pctxt, OSOCTET octet)
{
   return rtxWriteBytes (pctxt, &octet, 1);
}

int berEncStrmWriteOctets
(OSCTXT* pctxt, const OSOCTET* poctets, OSSIZE numocts)
{
   return rtxWriteBytes (pctxt, poctets, numocts);
}

/***********************************************************************
 *
 *  Routine name: berEncStrmIdentifier
 *
 *  Description:  This routine encodes an ASN identifier as used in ASN
 *                tag and ASN object identifier definitions.  The
 *                encoding of an identifier is accomplished through a
 *                series of octets, each of which contains a 7 bit
 *                unsigned number. The 8th bit (MSB) of each octet is
 *                used as a continuation flag to indicate that more
 *                octets follow in the sequence.
 *
 *  Inputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  pctxt      struct  Pointer to ASN.1 context block structure.
 *  ident       uint    Identifier to encode.
 *
 *  Outputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  msglen      int     Number of bytes generated to represent given
 *                      identifier value.  Returned as function result.
 *
 **********************************************************************/

int berEncStrmIdentifier (OSCTXT *pctxt, unsigned ident)
{
   unsigned mask = 0x7f;
   int nshifts = 0, stat = 0;

   /* Find starting point in identifier value */
   nshifts = rtxGetIdentByteCount (ident);
   mask <<= (7 * nshifts);

   /* Encode bytes */
   if (nshifts > 0) {
      unsigned lv;
      while (nshifts > 0) {
         mask >>= 7; nshifts--;
         lv = (ident & mask) >> (nshifts * 7);
         if (nshifts != 0) { lv |= 0x80; }
         stat = berEncStrmWriteOctet (pctxt, (OSOCTET)lv);
         if (stat != 0) break;
      }
   }
   else {
      /* encode a single zero byte */
      stat = berEncStrmWriteOctet (pctxt, 0);
   }
   if (stat != 0) return LOG_RTERR (pctxt, stat);
   return 0;
}

int berEncStrmTag (OSCTXT *pctxt, ASN1TAG tag)
{
   int          stat;
   ASN1TAG      ltag;
   OSUINT32     id_code = tag & TM_IDCODE;
   OSOCTET    b, class_form;

   /* Split tag into class/form and ID code components */

   ltag = tag >> ((sizeof(ltag) * 8) - 3);
   class_form = (OSOCTET)(ltag << 5);

   /* Encode components */

   if (id_code < 31) {
      b = (OSOCTET) (class_form + id_code);
      stat = berEncStrmWriteOctet (pctxt, b);
   }
   else {
      b = (OSOCTET) (class_form | TM_B_IDCODE);
      if ((stat = berEncStrmWriteOctet (pctxt, b)) == 0) {
         stat = berEncStrmIdentifier (pctxt, id_code);
      }
   }

   if (stat != 0) return LOG_RTERR (pctxt, stat);
   return 0;
}

int berEncStrmDefLength (OSCTXT* pctxt, OSSIZE length)
{
   int stat = 0;
   OSOCTET lbuf[16];
   OSSIZE idx = sizeof(lbuf) - 1, nbytes = 0;
   OSBOOL extended = (length > 127);

   do {
      lbuf[idx--] = (OSOCTET)(length % 256);
      nbytes++;
      length /= 256;
   }
   while (length > 0);

   if (extended) { /* extended length */

      /* if extended length, add a final byte containing
         number of bytes encoded with bit 8 set .. */

      stat = berEncStrmWriteOctet (pctxt, (OSOCTET)(nbytes | 0x80));
      if (stat != 0) return LOG_RTERR (pctxt, stat);
   }

   stat = berEncStrmWriteOctets (pctxt, &lbuf[idx+1], nbytes);
   return (stat != 0) ? LOG_RTERR (pctxt, stat) : 0;
}

int berEncStrmLength (OSCTXT *pctxt, int length)
{
   int stat = 0;

   if (length >= 0) {
      stat = berEncStrmDefLength (pctxt, (OSSIZE)length);
   }
   else if (length == ASN_K_INDEFLEN) {
      /* if indefinite length, write a byte containing
         zero encoded with bit 8 set .. */
      stat = berEncStrmWriteOctet (pctxt, 0x80);
   }
   else stat = length;

   return (stat != 0) ? LOG_RTERR (pctxt, stat) : 0;
}

int berEncStrmTagAndLen (OSCTXT *pctxt, ASN1TAG tag, int length)
{
   int stat;

   if (length < 0 && length != ASN_K_INDEFLEN) return (length);

   stat = berEncStrmTag (pctxt, tag);
   if (stat == 0)
      stat = berEncStrmLength (pctxt, length);

   if (stat != 0) return LOG_RTERR (pctxt, stat);
   return 0;
}

int berEncStrmTagAndDefLen (OSCTXT *pctxt, ASN1TAG tag, OSSIZE length)
{
   int stat;

   stat = berEncStrmTag (pctxt, tag);
   if (stat == 0)
      stat = berEncStrmDefLength (pctxt, length);

   return (stat != 0) ? LOG_RTERR (pctxt, stat) : 0;
}

int berEncStrmTagAndIndefLen (OSCTXT *pctxt, ASN1TAG tag)
{
   int stat;

   stat = berEncStrmTag (pctxt, (tag | TM_CONS));
   if (stat == 0) {
      /* if indefinite length, write a byte containing
         zero encoded with bit 8 set .. */
      stat = berEncStrmWriteOctet (pctxt, 0x80);
   }

   if (stat != 0) return LOG_RTERR (pctxt, stat);
   return 0;
}

int berEncStrmEOC (OSCTXT *pctxt)
{
   static OSOCTET eoc[] = { 0, 0 };
   int stat = berEncStrmWriteOctets (pctxt, (OSOCTET*)eoc, 2);

   if (stat != 0) return LOG_RTERR (pctxt, stat);
   return 0;
}
