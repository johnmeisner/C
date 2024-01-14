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

/***********************************************************************
 *
 *  FILE DECODE FUNCTIONS
 *
 **********************************************************************/
/***********************************************************************
 *
 *  Routine name: xdf_tag
 *
 *  Description:  This routine decodes an ASN.1 tag from a file stream
 *                into a standard 16 bit structure.
 *
 *  Inputs:
 *
 *  Name        Type         Description
 *  ----        ----         -----------
 *  fp          FILE*        Pointer to file from which tag is to be parsed
 *  pbufidx     int*         Pointer to current buffer index
 *
 *  Outputs:
 *
 *  Name        Type         Description
 *  ----        ----         -----------
 *  stat	int	     Status of operation.  Returned as function result.
 *  ptag	ASN1TAG*     Pointer to ASN1TAG structure to receive tag.
 *  buffer      OSOCTET*   Buffer to receive parsed data.
 *  pbufidx     int*         Updated buffer index
 *
 ***********************************************************************/

int xdf_tag (FILE* fp, ASN1TAG* ptag, OSOCTET* buffer, int* pbufidx)
{
   OSOCTET	b;
   ASN1TAG	class_form, id_code;
   int  	i = *pbufidx, lcnt = 0, stat = 0;

   *ptag = 0;

   if (fread (&b, 1, 1, fp) == 1)
   {
      buffer[i++] = b;
      class_form = (ASN1TAG)(b & TM_CLASS_FORM);
      class_form <<= ASN1TAG_LSHIFT;

      if ((id_code = b & TM_B_IDCODE) == 31)
      {
         id_code = 0;
         do {
            if (fread (&b, 1, 1, fp) == 1) {
               buffer[i++] = b;
               id_code = (id_code * 128) + (b & 0x7F);
               if (id_code > TM_IDCODE || lcnt++ > 8)
                  return (ASN_E_BADTAG);
            }
            else {
               stat = RTERR_ENDOFFILE;
               break;
            }
         } while (b & 0x80);
      }

      *ptag = class_form | id_code;
   }
   else
      stat = RTERR_ENDOFFILE;

   *pbufidx = i;

   return (stat);
}

/***********************************************************************
 *
 *  Routine name: xdf_len
 *
 *  Description:  This routine decodes an ASN.1 length from a
 *                file stream.
 *
 *  Inputs:
 *
 *  Name        Type         Description
 *  ----        ----         -----------
 *  fp          FILE*        Pointer to file from which tag is to be parsed
 *  pbufidx     int*         Pointer to current buffer index
 *
 *  Outputs:
 *
 *  Name        Type         Description
 *  ----        ----         -----------
 *  stat        int	     Status of operation. Returned as function result.
 *  plen        OSINT32*     Pointer to integer to receive decoded length.
 *  buffer      OSOCTET*   Buffer to receive parsed data.
 *  pbufidx     int*         Updated buffer index
 *
 ***********************************************************************/

int xdf_len (FILE* fp, OSINT32* plen, OSOCTET* buffer, int* pbufidx)
{
   OSOCTET	i, b;
   int  	bufx = *pbufidx, stat = 0;

   *plen = 0;

   if (fread (&b, 1, 1, fp) == 1)
   {
      buffer[bufx++] = b;
      if (b > 0x80)
      {
         i = (OSOCTET) (b & 0x7F);
         if (i > 4) return ASN_E_INVLEN;
         for (*plen = 0; i > 0; i--)
            if (fread (&b, 1, 1, fp) == 1) {
               buffer[bufx++] = b;
               *plen = (*plen * 256) + b;
            }
            else
               return (RTERR_ENDOFFILE);
      }
      else if (b == 0x80) *plen = ASN_K_INDEFLEN;
      else *plen = b;
   }
   else stat = RTERR_ENDOFFILE;

   *pbufidx = bufx;

   return (stat);
}

/***********************************************************************
 *
 *  Routine name: xdf_TagAndLen
 *
 *  Description:  This routine decodes an ASN.1 tag and length from a
 *                file stream.
 *
 *  Inputs:
 *
 *  Name        Type         Description
 *  ----        ----         -----------
 *  fp          FILE*        Pointer to file from which tag is to be parsed
 *  pbufidx     int*         Pointer to current buffer index
 *
 *  Outputs:
 *
 *  Name        Type         Description
 *  ----        ----         -----------
 *  stat        int	     Status of operation. Returned as function result.
 *  ptag	ASN1TAG*     Pointer to ASN1TAG structure to receive tag.
 *  plen        OSINT32*     Pointer to integer to receive decoded length.
 *  buffer      OSOCTET*   Buffer to receive parsed data.
 *  pbufidx     int*         Updated buffer index
 *
 ***********************************************************************/

int xdf_TagAndLen (FILE* fp, ASN1TAG* ptag, OSINT32* plen,
                   OSOCTET* buffer, int* pbufidx)
{
   int stat;

   stat = xdf_tag (fp, ptag, buffer, pbufidx);
   if (stat != 0) return stat;

   stat = xdf_len (fp, plen, buffer, pbufidx);
   if (stat != 0) return stat;

   return 0;
}

