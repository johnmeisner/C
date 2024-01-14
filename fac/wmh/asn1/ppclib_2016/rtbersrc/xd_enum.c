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
 *  Routine name: xd_enum
 *
 *  Description:  This routine decodes the enumerated value at the current
 *                message pointer location and returns the value.  It
 *                also advances the message pointer to the start of the
 *                the next field.
 *
 *                The routine first checks to see if explicit tagging
 *                is specified.  If yes, it calls xd_match to match
 *                the universal tag for this message type.  If the
 *                match is not successful, a negative value is returned
 *                to indicate the parse was not successful.
 *
 *                If the match is successful or implicit tagging is
 *                specified, xd_integer is called to fetch
 *                the data value.
 *
 *  Inputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  tagging     bool    Specifies whether element is implicitly or
 *                      explicitly tagged.
 *  length      int     Length of data to retrieve.  Valid for implicit
 *                      case only.
 *
 *  Outputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  stat	int	Status of operation.  Returned as function result.
 *  object      int     Decoded enumerated value.
 *
 **********************************************************************/

int xd_enum (OSCTXT *pctxt, OSINT32 *object_p,
             ASN1TagType tagging, int length)
{
   register int status;

   if (tagging == ASN1EXPL) {
      if (ASN1BUFCUR (pctxt) != ASN_ID_ENUM) {
         return berErrUnexpTag (pctxt, ASN_ID_ENUM);
      }
      else
         XD_BUMPIDX (pctxt, 1);

      status = XD_LEN (pctxt, &length);
      if (status != 0) return LOG_RTERR (pctxt, status);
   }

   status = xd_integer (pctxt, object_p, ASN1IMPL, length);
   if (status != 0) return LOG_RTERR (pctxt, status);

   return 0;
}


/***********************************************************************
 *
 *  Routine name: xd_enumUnsigned
 *
 *  Description:  This routine decodes the enumerated value at the current
 *                message pointer location and returns the value.  It
 *                also advances the message pointer to the start of the
 *                the next field.
 *
 *                The routine first checks to see if explicit tagging
 *                is specified.  If yes, it calls xd_match to match
 *                the universal tag for this message type.  If the
 *                match is not successful, a negative value is returned
 *                to indicate the parse was not successful.
 *
 *                If the match is successful or implicit tagging is
 *                specified, xd_unsigned is called to fetch
 *                the data value.
 *
 *  Inputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  tagging     bool    Specifies whether element is implicitly or
 *                      explicitly tagged.
 *  length      int     Length of data to retrieve.  Valid for implicit
 *                      case only.
 *
 *  Outputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  stat	int	Status of operation.  Returned as function result.
 *  object      int     Decoded enumerated value.
 *
 **********************************************************************/

int xd_enumUnsigned (OSCTXT *pctxt, OSUINT32 *object_p,
             ASN1TagType tagging, int length)
{
   register int status;

   if (tagging == ASN1EXPL) {
      if (ASN1BUFCUR (pctxt) != ASN_ID_ENUM) {
         return berErrUnexpTag (pctxt, ASN_ID_ENUM);
      }
      else
         XD_BUMPIDX (pctxt, 1);

      status = XD_LEN (pctxt, &length);
      if (status != 0) return LOG_RTERR (pctxt, status);
   }

   status = xd_unsigned (pctxt, object_p, ASN1IMPL, length);
   if (status != 0) return LOG_RTERR (pctxt, status);

   return 0;
}
