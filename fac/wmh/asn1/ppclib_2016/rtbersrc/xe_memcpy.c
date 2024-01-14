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
 *  Routine name: xe_expandBuffer
 *
 *  Description:  This routine handles the expansion of a
 *                dynamic encode buffer.
 *
 *  Inputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  pctxt	struct	Pointer to ASN.1 context block structure.
 *  length	int	Number of bytes required.
 *
 *  Outputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  stat        int     Status of expand operation.  Returned as
 *                      function result.
 *
 **********************************************************************/

int xe_expandBuffer (OSCTXT *pctxt, size_t length)
{
   OSOCTET* newBuf_p, *msg_p;
   size_t usedBytes;

   if (pctxt->buffer.dynamic)
   {
      OSBufferIndex* pBufferIndex;

      /* If dynamic encoding is enabled, expand the current buffer to	*/
      /* allow encoding to continue.                                    */

      /* Change to double buffer size each time (ED, 10/4/06) */
      /* size_t extent = OSRTMAX (ASN_K_ENCBUFSIZ, length); */
      size_t extent = OSRTMAX (pctxt->buffer.size, length);
      size_t newSize = pctxt->buffer.size + extent;

      newBuf_p = (OSOCTET*) rtxMemAlloc (pctxt, newSize);
      if (!newBuf_p) return LOG_RTERR (pctxt, RTERR_NOMEM);

      /* Set message pointer position in new buffer and copy existing	*/
      /* message to it..						*/

      usedBytes = pctxt->buffer.size - pctxt->buffer.byteIndex;
      msg_p = (newBuf_p + newSize) - usedBytes;
      memcpy (msg_p, OSRTBUFPTR(pctxt), usedBytes);

      /* Free old buffer and set context parameters */

      rtxMemFreePtr (pctxt, pctxt->buffer.data);
      pctxt->buffer.data = newBuf_p;
      pctxt->buffer.size = newSize;
      pctxt->buffer.byteIndex += extent;

      /* If an entry exists on the containerEndIndexStack, update its
         index (note that only a single container index is supported at
         this time). */
      pBufferIndex = (OSBufferIndex*)
         rtxStackPeek (&pctxt->containerEndIndexStack);

      if (0 != pBufferIndex) {
         pBufferIndex->byteIndex += extent;
      }

      return (0);
   }
   else
      return LOG_RTERR (pctxt, RTERR_BUFOVFLW);
}

/***********************************************************************
 *
 *  Routine name: xe_memcpy
 *
 *  Description:  This routine handles memory transfers required during
 *                the encoding of a message.  It copies data from the
 *                given C structure into the encode message buffer.
 *
 *  Inputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  pctxt	struct	Pointer to ASN.1 context block structure.
 *  object      char*   Pointer to object to be copied.
 *  length	int	Number of bytes to copy.
 *
 *  Outputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  length      int     Actual length of data copied.  Returned as
 *                      function result.
 *
 **********************************************************************/

int xe_memcpy (OSCTXT *pctxt, const OSOCTET* object_p, size_t length)
{
   if (0 == object_p) return LOG_RTERR (pctxt, RTERR_BADVALUE);

   if (length > pctxt->buffer.byteIndex) {
      int stat = xe_expandBuffer (pctxt, length);
      if (stat != 0) return LOG_RTERR (pctxt, stat);
   }

   if (length == 1) {
      pctxt->buffer.byteIndex --;
      *ASN1BUFPTR(pctxt) = *object_p;
   }
   else {
      pctxt->buffer.byteIndex -= length;
      memcpy (ASN1BUFPTR(pctxt), object_p, length);
   }
   return (int)length;
}

/***********************************************************************
 *
 *  Routine name: xe_getBufLocDescr
 *
 *  Description:  This routine calculates the relative position of
 *                an item in a BER encode buffer from the end of the
 *                buffer.  That way, if the buffer is resized, the
 *                item can still be located.
 *
 *  Inputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  pctxt	struct	Pointer to ASN.1 context block structure.
 *  length      int     Length of item at current buffer position.
 *
 *  Outputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  descr       struct* Pointer to Asn1BufLocDescr structure.
 *
 *
 **********************************************************************/

void xe_getBufLocDescr (OSCTXT *pctxt, OSSIZE length,
                        Asn1BufLocDescr* pDescr)
{
   pDescr->offset = pctxt->buffer.size - pctxt->buffer.byteIndex;
   pDescr->numocts = length;
}

