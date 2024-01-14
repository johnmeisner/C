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

#include "rtpersrc/pu_common.hh"
#include "rtxsrc/rtxUtil.h"
#ifndef _COMPACT
#include "rtxsrc/rtxDiagBitTrace.h"
#endif

/***********************************************************************
 *
 *  COMMON UTILITY FUNCTIONS
 *
 **********************************************************************/

/***********************************************************************
 *
 *  Routine name: pu_getMsgLen
 *
 *  Description:  This routine returns the length of the encoded PER
 *                message.  For decoding, this function must be called
 *                after the decode function is complete to get the
 *                message length.
 *
 *  Inputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  pctxt      struct* Pointer to ASN.1 PER context structure
 *
 *  Outputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  length      int     Length of encoded message
 *
 *
 **********************************************************************/

EXTPERMETHOD size_t pu_getMsgLen (OSCTXT* pctxt)
{
   return (pctxt->buffer.bitOffset == 8) ?
      pctxt->buffer.byteIndex : pctxt->buffer.byteIndex + 1;
}

EXTPERMETHOD size_t pu_getMsgLenBits (OSCTXT* pctxt)
{
   return (8 * pctxt->buffer.byteIndex) + (8 - pctxt->buffer.bitOffset);
}

/* PER set functions */

EXTPERMETHOD void pu_setAligned (OSCTXT* pctxt, OSBOOL value)
{
   pctxt->buffer.aligned = value;
}

EXTPERMETHOD int pu_setBuffer
(OSCTXT* pctxt, OSOCTET* bufaddr, size_t bufsiz, OSBOOL aligned)
{
   int stat = rtxInitContextBuffer (pctxt, bufaddr, bufsiz);
   if(stat != 0) return LOG_RTERR (pctxt, stat);

   pctxt->buffer.aligned = aligned;
   return 0;
}

EXTPERMETHOD OSBOOL pu_setTrace (OSCTXT* pctxt, OSBOOL value)
{
#ifndef _COMPACT
   OSBOOL bTrace = (pctxt->flags & ASN1TRACE) ? TRUE : FALSE;

   if (value) {
      rtxDiagCtxtBitFieldListInit (pctxt);
      rtxCtxtSetFlag (pctxt, ASN1TRACE);
   }
   else {
      rtxCtxtClearFlag (pctxt, ASN1TRACE);
   }

   return bTrace;
#else
   return FALSE;
#endif
}

/* This function initializes a context and sets its buffer to point     */
/* at the same data as the given context..                              */

EXTPERMETHOD int pu_initContextBuffer (OSCTXT* pTarget, OSCTXT* pSource)
{
   int stat = pu_setBuffer (pTarget,
                            pSource->buffer.data,
                            pSource->buffer.size,
                            pSource->buffer.aligned);

   if (0 == stat) {
      pTarget->buffer.byteIndex = pSource->buffer.byteIndex;
      pTarget->buffer.bitOffset = pSource->buffer.bitOffset;
   }
   else return LOG_RTERR (pTarget, stat);

   return 0;
}

/***********************************************************************
 *
 *  Routine name: pu_freeContext
 *
 *  Description:  This routine frees a context block structure
 *                including all internally allocated dynamic memory.
 *
 *  Inputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  pctxt      struct* Pointer to context structure to free.
 *
 *  Outputs:
 *
 *  None
 *
 *
 **********************************************************************/

EXTPERMETHOD void pu_freeContext (OSCTXT* pctxt)
{
   rtFreeContext (pctxt);
}

/***********************************************************************
 *
 *  Routine name: pu_bitcnt
 *
 *  Description:  This routine counts the number of bits required
 *                to hold the given unsigned integer value.
 *
 *  Inputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  value       uint    Unsigned integer value
 *
 *  Outputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  nbits       uint    Number of bits required to hold the value
 *
 **********************************************************************/

EXTPERMETHOD OSUINT32 pu_bitcnt (OSUINT32 value)
{
   return rtxUInt32BitLen (value);
}


EXTPERMETHOD size_t pu_getMaskAndIndex (size_t bitOffset, unsigned char* pMask)
{
   int relBitOffset = 7 - (int)(bitOffset % 8);
   *pMask = (unsigned char)(OSUINTCONST(1) << relBitOffset);
   return (bitOffset / 8);  /* byte index */
}
