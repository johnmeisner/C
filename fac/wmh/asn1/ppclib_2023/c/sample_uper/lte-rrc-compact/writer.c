#include "EUTRA-RRC-Definitions.h"
#include "rtxsrc/rtxPrint.h"
#include "rtxsrc/rtxDiag.h"
#include "rtxsrc/rtxFile.h"

int main (int argc, char** argv)
{
   /* PDU: EUTRA-RRC-Definitions.DL_CCCH_Message */
   DL_CCCH_Message data;
   DL_CCCH_Message* pvalue = &data;
   OSCTXT     ctxt;
   int        i, len, stat;
   OSBOOL     trace = TRUE;
   OSBOOL     verbose = FALSE;
   const char* filename = "message.dat";
   const OSOCTET* msgptr;

   /* Process command line arguments */
   if (argc > 1) {
      for (i = 1; i < argc; i++) {
         if (!strcmp (argv[i], "-v")) verbose = TRUE;
         else if (!strcmp (argv[i], "-o")) filename = argv[++i];
         else if (!strcmp (argv[i], "-notrace")) trace = FALSE;
         else {
            printf ("usage: writer [-v][-o <filename>][-notrace]\n");
            printf ("   -v  verbose mode: print trace info\n");
            printf ("   -o <filename>  write encoded msg to <filename>\n");
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
   pu_setBuffer (&ctxt, 0, 0, /*aligned*/FALSE);

   asn1Init_DL_CCCH_Message (pvalue);

   pvalue->message.t =  1;
   pvalue->message.u.c1 = rtxMemAllocTypeZ (&ctxt, DL_CCCH_MessageType_c1);
   pvalue->message.u.c1->t =  T_DL_CCCH_MessageType_c1_rrcConnectionSetup;
   pvalue->message.u.c1->u.rrcConnectionSetup = 
      rtxMemAllocTypeZ (&ctxt, RRCConnectionSetup);

   pvalue->message.u.c1->u.rrcConnectionSetup->rrc_TransactionIdentifier = 0;
   pvalue->message.u.c1->u.rrcConnectionSetup->criticalExtensions.t =
      T_RRCConnectionSetup_criticalExtensions_c1;
   pvalue->message.u.c1->u.rrcConnectionSetup->criticalExtensions.u.c1 =
      rtxMemAllocTypeZ (&ctxt, RRCConnectionSetup_criticalExtensions_c1);

   pvalue->message.u.c1->u.rrcConnectionSetup->criticalExtensions.u.c1->t =
      T_RRCConnectionSetup_criticalExtensions_c1_rrcConnectionSetup_r8;

   pvalue->message.u.c1->u.rrcConnectionSetup->
      criticalExtensions.u.c1->u.rrcConnectionSetup_r8 = 
      rtxMemAllocTypeZ (&ctxt, RRCConnectionSetup_r8_IEs);

   pvalue->message.u.c1->u.rrcConnectionSetup->
      criticalExtensions.u.c1->u.rrcConnectionSetup_r8->
      radioResourceConfigDedicated.m.physicalConfigDedicatedPresent = 1;

   pvalue->message.u.c1->u.rrcConnectionSetup->
      criticalExtensions.u.c1->u.rrcConnectionSetup_r8->
      radioResourceConfigDedicated.physicalConfigDedicated.m.
      cqi_ReportConfigPresent = 1;

   pvalue->message.u.c1->u.rrcConnectionSetup->
      criticalExtensions.u.c1->u.rrcConnectionSetup_r8->
      radioResourceConfigDedicated.physicalConfigDedicated.
      cqi_ReportConfig.m.cqi_ReportModeAperiodicPresent = 1;

   pvalue->message.u.c1->u.rrcConnectionSetup->
      criticalExtensions.u.c1->u.rrcConnectionSetup_r8->
      radioResourceConfigDedicated.physicalConfigDedicated.
      cqi_ReportConfig.cqi_ReportModeAperiodic = rm22;

   pvalue->message.u.c1->u.rrcConnectionSetup->
      criticalExtensions.u.c1->u.rrcConnectionSetup_r8->
      radioResourceConfigDedicated.physicalConfigDedicated.
      cqi_ReportConfig.nomPDSCH_RS_EPRE_Offset = -1;

   pvalue->message.u.c1->u.rrcConnectionSetup->
      criticalExtensions.u.c1->u.rrcConnectionSetup_r8->
      radioResourceConfigDedicated.physicalConfigDedicated.m._v3ExtPresent = 1;

   pvalue->message.u.c1->u.rrcConnectionSetup->
      criticalExtensions.u.c1->u.rrcConnectionSetup_r8->
      radioResourceConfigDedicated.physicalConfigDedicated.m.
      antennaInfo_r10Present = 1;

   pvalue->message.u.c1->u.rrcConnectionSetup->
      criticalExtensions.u.c1->u.rrcConnectionSetup_r8->
      radioResourceConfigDedicated.physicalConfigDedicated.antennaInfo_r10.t = 
      T_PhysicalConfigDedicated_antennaInfo_r10_explicitValue_r10;

   pvalue->message.u.c1->u.rrcConnectionSetup->
      criticalExtensions.u.c1->u.rrcConnectionSetup_r8->
      radioResourceConfigDedicated.physicalConfigDedicated.antennaInfo_r10.u.
      explicitValue_r10 = rtxMemAllocTypeZ (&ctxt, AntennaInfoDedicated_r10);

   pvalue->message.u.c1->u.rrcConnectionSetup->
      criticalExtensions.u.c1->u.rrcConnectionSetup_r8->
      radioResourceConfigDedicated.physicalConfigDedicated.antennaInfo_r10.u.
      explicitValue_r10->transmissionMode_r10 = tm3;

   pvalue->message.u.c1->u.rrcConnectionSetup->
      criticalExtensions.u.c1->u.rrcConnectionSetup_r8->
      radioResourceConfigDedicated.physicalConfigDedicated.antennaInfo_r10.u.
      explicitValue_r10->ue_TransmitAntennaSelection.t =
      T_AntennaInfoDedicated_ue_TransmitAntennaSelection_setup;

   pvalue->message.u.c1->u.rrcConnectionSetup->
      criticalExtensions.u.c1->u.rrcConnectionSetup_r8->
      radioResourceConfigDedicated.physicalConfigDedicated.antennaInfo_r10.u.
      explicitValue_r10->ue_TransmitAntennaSelection.u.setup = openLoop;

   /* Encode */
   stat = asn1PE_DL_CCCH_Message (&ctxt, &data);

   msgptr = pe_GetMsgPtr (&ctxt, &len);

   if (trace) {
      printf ("Hex dump of encoded record:\n");
      rtxHexDump (msgptr, len);
   }
   if (stat < 0) {
      printf ("Encoding failed\n");
      rtxErrPrint (&ctxt);
      rtFreeContext (&ctxt);
      return stat;
   }

   /* Write the encoded message out to the output file */
   stat = rtxFileWriteBinary (filename, msgptr, len);
   if (stat < 0) {
      printf ("Write to file failed\n");
      rtxErrPrint (&ctxt);
      rtFreeContext (&ctxt);
      return stat;
   }

   rtFreeContext (&ctxt);

   return 0;
}
