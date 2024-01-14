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

/***********************************************************************
 *
 *  Routine name: xd_memcpy
 *
 *  Description:  This routine handles memory transfers required during
 *                the decoding of a message.  It copies data from the
 *                message buffer to the given object. The overall
 *                message length is decremented as the copy occurs in
 *                order to keep track of the relative pointer position in
 *                in the buffer.  Attempts to copy data from past the end
 *                of the buffer will result in an error.
 *
 *  Inputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  length	int	Number of bytes to copy.
 *
 *  Outputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  length      int     Actual length of data copied.  Returned as
 *                      function result.
 *  object      char*   Pointer to destination object.
 *
 **********************************************************************/

int xd_memcpy (OSCTXT* pctxt, OSOCTET *object_p, int length)
{
   OSBOOL do_copy = (OSBOOL) ((ASN1BUF_INDEFLEN(pctxt)) ?
      TRUE : ((pctxt->buffer.byteIndex + length) <= pctxt->buffer.size));

   if (do_copy) {
      memcpy (object_p, OSRTBUFPTR(pctxt), length);
      pctxt->buffer.byteIndex += length;
      return (0);
   }
   else return LOG_RTERR (pctxt, RTERR_ENDOFBUF);
}

