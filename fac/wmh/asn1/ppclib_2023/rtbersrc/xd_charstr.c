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
#include "rtxsrc/rtxCharStr.h"
#include "rtxsrc/rtxContext.hh"

/***********************************************************************
 *
 *  Routine name: xd_charstr
 *
 *  Description:  The following function decodes a value of a
 *                useful character string useful type.
 *
 *  Inputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  pctxt	struct	Pointer to ASN.1 context block structure
 *  tagging     enum    Specifies whether element is implicitly or
 *                      explicitly tagged.
 *  tag         ASN1TAG ASN.1 tag to match before decoding the type.
 *                      Valid for explicit case only.
 *  length      int     Length of data to retrieve.  Valid for implicit
 *                      case only.
 *
 *  Outputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  stat	int	Status of operation.  Returned as function result.
 *  object      bool    Decoded string value.
 *
 **********************************************************************/

int xd_charstr64 (OSCTXT* pctxt, char** object_p,
                  ASN1TagType tagging, ASN1TAG tag,
                  OSSIZE length, OSBOOL indefLen)
{
   int stat = 0;
   OSBOOL isConstructedTag;
   OSSIZE size, ll = 0;
   char* tmpstr;

   if (tagging == ASN1EXPL) {
      stat = xd_match64
         (pctxt, (OSOCTET)tag, &length, &indefLen, XM_ADVANCE);

      if (stat < 0) {
         /* RTERR_IDNOTFOU will be logged later, by the generated code,
            or reset by rtxErrReset (for optional seq elements). */
         return (stat == RTERR_IDNOTFOU) ? stat : LOG_RTERR (pctxt, stat);
      }
   }
   isConstructedTag = (OSBOOL)((pctxt->flags & ASN1CONSTAG) != 0);

   if (isConstructedTag && rtxCtxtTestFlag(pctxt, ASN1DER))
   {
      /* DER requires primitive encoding form */
      LOG_RTERRNEW(pctxt, ASN_E_PRIM_REQ);
      rtxErrSetNonFatal(pctxt);
   }

   /* For an indefinite length message, need to get the actual 	*/
   /* length by parsing tags until the end of the message is 	*/
   /* reached..							*/
#ifndef _BUILD_LICOBJ
   if (indefLen) {  /* indefinite length message */
      stat = xd_consStrIndefLenAndSize
         (pctxt, TM_UNIV|ASN_ID_OCTSTR, &ll, &size);

      if (stat != 0) return LOG_RTERR (pctxt, stat);
   }
   else
#endif
      ll = size = length;

   if (rtxCtxtTestFlag(pctxt, ASN1CANON))
   {
      if (ll <= 1000 && isConstructedTag)
      {
         /* CER requires primitive form when contents octets <= 1000. */
         LOG_RTERRNEW(pctxt, ASN_E_PRIM_REQ);
         rtxErrSetNonFatal(pctxt);
      }
      else if (ll > 1000 && !isConstructedTag)
      {
         /* CER requires constructed form (fragmentation) when contents
         octets > 1000 */
         LOG_RTERRNEW(pctxt, ASN_E_FRAGMENTS);
         rtxErrSetNonFatal(pctxt);
      }
   }


   if (ll > 0) {
      if (ll > OSSIZE_MAX - 1) return LOG_RTERR (pctxt, RTERR_TOOBIG);

      tmpstr = (char*) rtxMemAlloc (pctxt, ll + 1);
      if (0 != tmpstr) {
         ll = size;
         stat = xd_octstr64_s (pctxt, (OSOCTET*)tmpstr,
                               &ll, ASN1IMPL, size, FALSE);
         if (stat != 0) {
            rtxMemFreePtr (pctxt, tmpstr);
            return LOG_RTERR (pctxt, stat);
         }
         tmpstr[ll] = '\0';

         /* If escape null-terminator flag is set (ASN1ESCZTERM) and
            decoded string ends in '\0', add "\\0" text to end.. */
         if (rtxCtxtTestFlag (pctxt, ASN1ESCZTERM) && tmpstr[ll-1] == '\0') {
            tmpstr = (char*) rtxMemRealloc (pctxt, tmpstr, ll + 3);
            if (0 == tmpstr) {
               return LOG_RTERR (pctxt, RTERR_NOMEM);
            }
            rtxStrcat (tmpstr, ll + 3, "\\0");
         }

         *object_p = tmpstr;
      }
      else
         return LOG_RTERR (pctxt, RTERR_NOMEM);
   }
   else {
      tmpstr = (char*) rtxMemAlloc (pctxt, 1);
      if (tmpstr) {
         tmpstr[0] = '\0';
         *object_p = tmpstr;
      }
      else
         return LOG_RTERR (pctxt, RTERR_NOMEM);
   }

   if (isConstructedTag && indefLen) {
      if (XD_MATCHEOC (pctxt)) XD_BUMPIDX (pctxt, 2);
      else return LOG_RTERR (pctxt, ASN_E_INVLEN);
   }

   LCHECKBER (pctxt);

   return 0;
}

int xd_charstr (OSCTXT* pctxt, const char** object_p,
                ASN1TagType tagging, ASN1TAG tag, int length)
{
   char* pdata;
   OSBOOL indefLen = (OSBOOL)(length == ASN_K_INDEFLEN);
   int ret = xd_charstr64
      (pctxt, &pdata, tagging, tag, (OSSIZE)length, indefLen);

   if (0 == ret) {
      *object_p = pdata;
   }

   return ret;
}
