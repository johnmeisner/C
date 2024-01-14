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

#include "rtbersrc/asn1ber.h"

/***********************************************************************
 *
 *  FILE DECODE FUNCTIONS
 *
 **********************************************************************/
/***********************************************************************
 *
 *  Routine name: xdf_ReadPastEOC
 *
 *  Description:  This routine consumes bytes from the file stream
 *                until a matching EOC is found.  An indefinte length
 *                tag is assumed to have been parsed prior to calling
 *                this function.
 *
 *  Inputs:
 *
 *  Name        Type         Description
 *  ----        ----         -----------
 *  fp          FILE*        Pointer to file from which tag is to be parsed
 *  bufsiz      int          Size of buffer to receive data
 *  pbufidx     int*         Pointer to current buffer index
 *
 *  Outputs:
 *
 *  Name        Type         Description
 *  ----        ----         -----------
 *  stat        int	     Status of operation. Returned as function result.
 *  buffer      OSOCTET*   Buffer to receive parsed data.
 *  pbufidx     int*         Updated buffer index
 *
 ***********************************************************************/

int xdf_ReadPastEOC (FILE* fp, OSOCTET* buffer, int bufsiz, int* pbufidx)
{
   ASN1TAG tag;
   int ilcnt = 1, len, stat = 0;

   while (ilcnt > 0) {
      stat = xdf_TagAndLen (fp, &tag, &len, buffer, pbufidx);
      if (stat != 0) break;

      if (*pbufidx > bufsiz) {
         stat = RTERR_ENDOFBUF;
         break;
      }

      if (len > 0) {
         if ((*pbufidx + len) > bufsiz) {
            return RTERR_ENDOFBUF;
         }
         if (fread (&buffer[*pbufidx], 1, len, fp) != (unsigned)len) {
            return RTERR_ENDOFFILE;
         }
         *pbufidx += len;
      }
      else if (len == ASN_K_INDEFLEN) ilcnt++;
      else if (tag == 0 && len == 0) ilcnt--;
   }

   return stat;
}


/***********************************************************************
 *
 *  Routine name: xdf_ReadContents
 *
 *  Description:  This routine reads the contents of a BER TLV
 *                into the given buffer.  The TLV can be of indefinite
 *                length.
 *
 *  Inputs:
 *
 *  Name        Type         Description
 *  ----        ----         -----------
 *  fp          FILE*        Pointer to file from which tag is to be parsed
 *  len         int          Length parsed from TLV
 *  bufsiz      int          Size of buffer to receive data
 *  pbufidx     int*         Pointer to current buffer index
 *
 *  Outputs:
 *
 *  Name        Type         Description
 *  ----        ----         -----------
 *  stat        int	     Status of operation. Returned as function result.
 *  buffer      OSOCTET*   Buffer to receive parsed data.
 *  pbufidx     int*         Updated buffer index
 *
 ***********************************************************************/

int xdf_ReadContents (FILE* fp, int len, OSOCTET* buffer,
                      int bufsiz, int* pbufidx)
{
   if (len > 0) {
      /* First check to make sure that component will fit in buffer */

      if ((len + *pbufidx) > bufsiz)
         return RTERR_ENDOFBUF;

      /* Read component from file into the message buffer */

      if (fread (&buffer[*pbufidx], 1, len, fp) != (unsigned)len)
         return RTERR_ENDOFFILE;

      *pbufidx += len;
      return 0;
   }
   else if (len == ASN_K_INDEFLEN)
      return xdf_ReadPastEOC (fp, buffer, bufsiz, pbufidx);
   else
      return len;

}
