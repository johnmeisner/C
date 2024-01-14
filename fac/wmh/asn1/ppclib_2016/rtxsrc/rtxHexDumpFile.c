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

#include <stdio.h>
#include "rtxsrc/rtxCharStr.h"
#include "rtxsrc/rtxCtype.h"
#include "rtxsrc/rtxErrCodes.h"
#include "rtxsrc/rtxFile.h"
#include "rtxsrc/rtxHexDump.h"

EXTRTMETHOD int rtxHexDumpToNamedFile
(const char* filename, const OSOCTET* data, OSSIZE numocts)
{
   FILE* fp;
   if (rtxFileOpen (&fp, filename, "w") == 0) {
      rtxHexDumpToFileEx (fp, data, numocts, 1);
      fclose (fp);
   }
   else {
      return RTERR_WRITEERR;
   }

   return 0;
}

EXTRTMETHOD void rtxHexDumpToFileExNoAscii
(FILE* fp, const OSOCTET* data, OSSIZE numocts, OSSIZE bytesPerUnit)
{
   OSSIZE   i;
   OSUINT32 val;
   OSOCTET  b, dumpstr = 1;
   char     hexstr[49], buf[20], *pbuf;
   OSSIZE   k, unitsPerLine = 16 / bytesPerUnit, shift;

   if (0 == data || numocts == 0) return;

   OSCRTLMEMSET (hexstr, ' ', 48); hexstr[48] = '\0';

   if (bytesPerUnit == 0 || bytesPerUnit > 4) bytesPerUnit = 4;

   for (i = 0; i < numocts / bytesPerUnit; i++)
   {
      pbuf = (char*)buf;
      buf [bytesPerUnit * 2] = 0;
      if (bytesPerUnit == 1) {
         val = *data++;
         shift = 0;
      }
      else if (bytesPerUnit == 2) {
         val = *data++;
         val = (val * 256) + *data++;
         shift = 8;
      }
      else { /* greater than 2 */
         OSSIZE ii;
         for (ii = 0, val = 0; ii < sizeof (OSUINT32); ii++) {
            val = (val * 256) + *data++;
         }
         shift = (sizeof(OSUINT32) - 1) * 8;
      }
      for (k = 0; k < bytesPerUnit; k++, pbuf += 2, shift -= 8) {
         b = (OSOCTET)((val >> shift) & 0xFF);
         rtxByteToHexChar (b, pbuf, 20);
      }
      *pbuf = 0;

      k = i % unitsPerLine * (bytesPerUnit * 2 + 1);
      OSCRTLSAFEMEMCPY (&hexstr[k], sizeof(hexstr), buf, bytesPerUnit * 2);

      if ((dumpstr = (OSOCTET) ((i + 1) % unitsPerLine == 0)) != 0)
      {
         fprintf (fp, "%48s\n", hexstr);
         if (i < (numocts - 1)) {
            OSCRTLMEMSET (hexstr, ' ', 48);
         }
      }
   }

   if (!dumpstr) fprintf (fp, "%48s\n", hexstr);
}

EXTRTMETHOD int rtxHexDumpToNamedFileNoAscii
(const char* filename, const OSOCTET* data, OSSIZE numocts)
{
   FILE* fp;
   if (rtxFileOpen (&fp, filename, "w") == 0) {
      rtxHexDumpToFileExNoAscii (fp, data, numocts, 1);
      fclose (fp);
   }
   else {
      return RTERR_WRITEERR;
   }

   return 0;
}

EXTRTMETHOD void rtxHexDumpEx
(const OSOCTET* data, OSSIZE numocts, OSSIZE bytesPerUnit)
{
   rtxHexDumpToFileEx (stdout, data, numocts, bytesPerUnit);
}

static int hexDumpFileContentsToFile (const char* inFilePath, FILE* outfp)
{
   FILE*    fp;
   OSOCTET  b;
   char     hexstr[49], ascstr[17];
   OSSIZE   ai = 0, i = 0;
   int      ch;

   if (rtxFileOpen (&fp, inFilePath, "r") < 0) return RTERR_READERR;

   OSCRTLMEMSET (hexstr, ' ', 48); hexstr[48] = '\0';
   OSCRTLMEMSET (ascstr, ' ', 16); ascstr[16] = '\0';

   for (;;)
   {
      ch = fgetc (fp);
      if (ch == EOF) break;

      b = (OSOCTET) ch;
      rtxByteToHexChar (b, &hexstr[i], 3);
      if (i < sizeof(hexstr) - 2) { hexstr[i+2] = ' '; }
      ascstr[ai++] = (char) (OS_ISPRINT(b) ? b : '.');
      i += 3;

      if (i >= sizeof(hexstr)-1)
      {
         fprintf (outfp, "%48s %16s\n", hexstr, ascstr);

         OSCRTLMEMSET (hexstr, ' ', 48);
         OSCRTLMEMSET (ascstr, ' ', 16);
         ai = 0; i = 0;
      }
   }

   if (i > 0) fprintf (outfp, "%48s %s\n", hexstr, rtxStrTrimEnd(ascstr));

   return 0;
}

EXTRTMETHOD int rtxHexDumpFileContents (const char* inFilePath)
{
   return hexDumpFileContentsToFile (inFilePath, stdout);
}

EXTRTMETHOD int rtxHexDumpFileContentsToFile
(const char* inFilePath, const char* outFilePath)
{
   FILE* fp;
   if (rtxFileOpen (&fp, outFilePath, "w") == 0) {
      hexDumpFileContentsToFile (inFilePath, fp);
      fclose (fp);
   }
   else {
      return RTERR_WRITEERR;
   }

   return 0;
}
