#include "EUTRA-RRC-Definitions.h"
#include "rtxsrc/rtxPrintStream.h"
#include "rtxsrc/rtxDiag.h"
#include "rtxsrc/rtxFile.h"

static void writeToStdout
(void* pPrntStrmInfo, const char* fmtspec, va_list arglist) 
{
   vprintf (fmtspec, arglist);
   return;
}

int main (int argc, char** argv)
{
   DL_CCCH_Message data;
   OSCTXT      ctxt;
   OSOCTET*    pMsgBuf;
   OSBOOL      trace = TRUE, verbose = FALSE;
   const char* filename = "message.dat";
   int         i, stat;
   OSSIZE      len;

   /* Process command line arguments */
   if (argc > 1) {
      for (i = 1; i < argc; i++) {
         if (!strcmp (argv[i], "-v")) verbose = TRUE;
         else if (!strcmp (argv[i], "-i")) filename = argv[++i];
         else if (!strcmp (argv[i], "-notrace")) trace = FALSE;
         else {
            printf ("usage: reader [-v] [-i <filename>] [-notrace]\n");
            printf ("   -v  verbose mode: print trace info\n");
            printf ("   -i <filename>  read encoded msg from <filename>\n");
            printf ("   -notrace  do not display trace info\n");
            return 1;
         }
      }
   }

   /* Initialize context structure */
   stat = rtInitContext (&ctxt);
   if (stat != 0) {
      rtxErrPrint (&ctxt);
      return stat;
   }
   rtxSetDiag (&ctxt, verbose);
   rtxSetPrintStream (&ctxt, &writeToStdout, (void *) 0);

   /* Read input file into a memory buffer */
   if (0 != rtxFileReadBinary (&ctxt, filename, &pMsgBuf, &len)) {
      printf ("Error opening %s for read access\n", filename);
      return -1;
   }

   pu_setBuffer (&ctxt, pMsgBuf, len, /*aligned*/FALSE);

   asn1Init_DL_CCCH_Message (&data);

   /* Call compiler generated decode function */
   stat = asn1PD_DL_CCCH_Message (&ctxt, &data);
   if (stat != 0) {
      printf ("decode of data failed\n");
      rtxErrPrint (&ctxt);
      rtFreeContext (&ctxt);
      return -1;
   }

   if (trace) {
      asn1PrtToStrm_DL_CCCH_Message (&ctxt, "DL-CCCH-Message", &data);
   }

   rtFreeContext (&ctxt);

   return 0;
}
