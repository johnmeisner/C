/*
 * Copyright (c) 2003-2018 Objective Systems, Inc.
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

/* Run-time print utility functions */

#include "rtxsrc/rtxErrCodes.h"
#include "rtxsrc/rtxFile.h"
#include "rtxsrc/rtxPrint.h"

EXTRTMETHOD void rtxPrintHexStrNoAscii
(const char* name, size_t numocts, const OSOCTET* data)
{
   if (numocts <= 32) {
      OSUINT32 i;
      printf ("%s = 0x", name);
      for (i = 0; i < numocts; i++) {
         printf ("%02x", data[i]);
      }
      printf ("\n");
   }
   else {
      printf ("%s =\n", name);
      rtxHexDumpToFileExNoAscii (stdout, data, numocts, 1);
   }
}

EXTRTMETHOD int rtxPrintFile (const char* filename)
{
   char lbuf[1024];
   FILE *fp;
   if (rtxFileOpen (&fp, filename, "r") < 0) {
      return (RTERR_FILNOTFOU);
   }
   while (fgets (lbuf, sizeof(lbuf), fp)) {
      puts (lbuf);
   }

   fclose (fp);

   return 0;
}
