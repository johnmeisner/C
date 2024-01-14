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

#include "rtpersrc/asn1per.h"

/***********************************************************************
 *
 *  DECODE FUNCTIONS
 *
 **********************************************************************/

/***********************************************************************
 *
 *  Routine name: pd_IncrBitIdx
 *
 *  Description:  This routine increments the bit index to access
 *                the next bit.  It checks for an end-of-buffer
 *                condition.
 *
 *  Inputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  pctxt      struct  PER context structure
 *
 *  Outputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  status      int     Return status of operation
 *
 *
 **********************************************************************/

int pd_IncrBitIdx (OSCTXT* pctxt)
{
   if (--pctxt->buffer.bitOffset < 0) {
      if (++pctxt->buffer.byteIndex >= pctxt->buffer.size) {
         return LOG_RTERR (pctxt, RTERR_ENDOFBUF);
      }
      pctxt->buffer.bitOffset = 7;
   }

   return 0;
}

/***********************************************************************
 *
 *  Routine name: pd_byte_align
 *
 *  Description:  Align the decode buffer on the next byte boundary.
 *
 *  Inputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  pdbuf       struct  Pointer to PER decode buffer structure
 *
 *  Outputs:
 *
 *  none
 *
 *
 **********************************************************************/

EXTPERMETHOD int pd_byte_align (OSCTXT* pctxt)
{
   if (pctxt->buffer.aligned) {
      if (pctxt->buffer.bitOffset != 8) {
         pctxt->buffer.byteIndex++;
         pctxt->buffer.bitOffset = 8;
         PU_SETBITOFFSET (pctxt);
      }

      return (pctxt->buffer.byteIndex < pctxt->buffer.size) ?
         0 : RTERR_ENDOFBUF;
   }
   else return 0;
}

