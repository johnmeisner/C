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
 *  Routine name: xe_setp
 *
 *  Description:  This routine sets up pointers to a user specified buffer
 *                for subsequent low level ASN.1 routine calls.
 *                It takes the given buffer pointer and adds the given
 *                length value to it to produce a pointer to the last
 *                byte in the buffer.  As an ASN.1 message is built, it
 *                will proceed from this point backward in memory as layers
 *                are added.
 *
 *  Inputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  pctxt	struct	Pointer to ASN.1 context block structure.
 *  msgbuf      char*   Buffer to be used for message encoding.
 *  bufsiz      int     Size of the encode buffer.
 *
 *  Outputs:
 *
 *  None
 *
 **********************************************************************/

int xe_setp (OSCTXT* pctxt, OSOCTET *buf_p, OSSIZE bufsiz)
{
   int stat = rtxInitContextBuffer (pctxt, buf_p, bufsiz);

   if (stat == 0) {
      /* Note: the initial buffer index needs to be set 1 byte outside of
         the buffer range because xe_memcpy will decrement the index
         before writing */
      pctxt->buffer.byteIndex = pctxt->buffer.size;
      return 0;
   }
   else return LOG_RTERR (pctxt, stat);
}

/***********************************************************************
 *
 *  Routine name: xe_getp
 *
 *  Description:  This routine fetches a pointer to the beginning of the
 *                ASN.1 message that is currently being encoded.  It is
 *                used after calling a series of low level encode routines
 *                to get the message start address.  This routine call is
 *                part the high level encode routines.
 *
 *  Inputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  pctxt	struct	Pointer to ASN.1 context block structure.
 *
 *  Outputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  msgptr      char*   Pointer to encoded ASN.1 message.  Returned as
 *                      function result.
 *
 **********************************************************************/

OSOCTET* xe_getp (OSCTXT* pctxt)
{
   return (pctxt->buffer.byteIndex < pctxt->buffer.size) ?
      OSRTBUFPTR(pctxt) : 0;
}

