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
#include <stdlib.h>
#include <string.h>
#include "rtbersrc/asn1ber.h"

#ifndef _NO_STREAM
#include "rtxsrc/rtxPrintStream.h"
#endif

/***********************************************************************
 *
 *  Routine name: xu_dump2
 *
 *  Description:  This routine dumps an ASN message to the standard output
 *  device in the following format:
 *
 *  CLAS  F  -ID-  LENGTH  HEX CONTENTS                         ASCII
 *  xxxx  x  xxxx  xxxxxx  xx xx xx xx xx xx xx xx xx xx xx xx  xxxxxxxxxxxx
 *
 *  Inputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  msgptr      char*   Pointer to ASN encoded message
 *
 *  Outputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  stat        int     Completion status of routine
 *
 *  This function differs from xu_dump in that it will output data using the
 *  context structure to use the rtxPrintStream function.  This allows users
 *  to redirect the output to the appropriate stream.
 *
 **********************************************************************/

#ifdef _NO_STREAM

#include <stdarg.h>

static int rtxPrintToStream (OSCTXT* pctxt, const char* fmtspec, ...)
{
   va_list  arglist;
   va_start (arglist, fmtspec);

   vfprintf (stdout, fmtspec, arglist);
   fflush (stdout);

   va_end (arglist);

   return 0;
}

#endif /* _NO_STREAM */

int xu_dump2 (OSCTXT *pctxt, const OSOCTET *msgptr)
{
   OSCTXT     lctxt;
   ASN1TAG    tag;
   int        count, len, stat, ilcnt;
   char       *bufp;
   ASN1TAGTEXT tagText;

   /* Set decode buffer pointer and pass header text to callback */

   if ((stat = rtInitASN1Context (&lctxt)) != 0)
      return (stat);

   if ((stat = xd_setp (&lctxt, msgptr, 0, &tag, &len)) != 0)
      { rtxFreeContext (&lctxt); return (stat); }

   rtxPrintToStream (pctxt, "%s\n",
      "CLAS  F  -ID-  LENGTH  HEX CONTENTS                         ASCII");

   if (len == ASN_K_INDEFLEN) ilcnt = 0;
   else ilcnt = 1; /* any non-zero value is acceptable here */

   /* Loop through message; format each line and pass to callback */

   while ((stat = xd_tag_len (&lctxt, &tag, &len, XM_ADVANCE)) == 0)
   {
      xu_fmt_tag_s (&tag, &tagText);
      if (len == ASN_K_INDEFLEN) ilcnt++;
      if (tag == 0 && len == 0) ilcnt--;

      if ((tag & TM_CONS) || len == 0)
      {
         if (len == ASN_K_INDEFLEN)
            rtxPrintToStream 
               (pctxt, "%4s  %s  %4s   INDEF\n", 
                tagText.class_p, tagText.form_p, tagText.idCode);
         else
            rtxPrintToStream 
               (pctxt, "%4s  %s  %4s  %6d\n", 
                tagText.class_p, tagText.form_p, tagText.idCode, len);
      }
      else
      {
         register int i;
         size_t offset = 0;

         bufp = xu_fmt_contents (&lctxt, len, &count);

         for (i = 0; i < count; i++)
         {
            if (i == 0)
               rtxPrintToStream 
                  (pctxt, "%4s  %c  %4s  %6d  %s\n",
                   tagText.class_p, tagText.form_p, tagText.idCode, 
                   len, &bufp[offset]);
            else
               rtxPrintToStream (pctxt,
                  "                       %s\n", &bufp[offset]);

            offset += strlen(&bufp[offset]) + 1;
         }

         rtxMemFreePtr (&lctxt, bufp);
      }
      if (ilcnt <= 0) break;
   }

   if (stat == RTERR_ENDOFBUF) stat = 0;

   rtxFreeContext (&lctxt);
   return (stat);
}


