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

#include "rtxsrc/rtxBench.h"
#include "rtxsrc/rtxFile.h"

/* Compute average time in microseconds between two times for a given   */
/* number of iterations..                                               */

double rtxBenchAverageMS (clock_t start, clock_t finish, double icnt)
{
   double delta = (double) (finish - start);
   /*double microsecs = (delta / (double)CLOCKS_PER_SEC) * (double)1000000;*/
   double microsecs = (delta * (double)1000000 / (double)CLOCKS_PER_SEC);
   return (microsecs / icnt);
}

void rtxBenchPrintResults (const char* filename, const OSRTBenchmarkData* pdata)
{
   FILE* fp;
   int ret;
   OSBOOL fileExists = rtxFileExists (filename);

   ret = rtxFileOpen (&fp, filename, "a");
   if (0 != ret) {
      printf ("open file %s failed, status = %d\n", filename, ret);
      return;
   }

   /* If file did not previously exist, write header */
   if (!fileExists) {
      fprintf (fp, "Program,Encode Rules,Iterations,Message Size,Encode Msecs,"
               "Decode Msecs\n");
   }

   fprintf (fp, "%s,%s," OSSIZEFMT "," OSSIZEFMT ",%lu,%lu\n",
            pdata->program, pdata->encRules, pdata->itercnt,
            pdata->msgSize, pdata->encodeMsecs, pdata->decodeMsecs);

   fclose (fp);
}
