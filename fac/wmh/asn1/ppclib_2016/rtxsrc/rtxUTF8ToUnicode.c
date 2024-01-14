/*
 * Copyright (c) 2003-2018 Objective Systems, Inc.
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

#include <stdlib.h>
#include <string.h>
#include "rtxsrc/rtxCtype.h"
#include "rtxsrc/rtxError.h"
#include "rtxsrc/rtxMemory.h"
#include "rtxsrc/rtxUTF8.h"

/* This file is included multiple times to implement functions to convert
   UTF-8 to UTF-16 and UTF-32.  It is included using different #defines as
   follows:
   UTF8TOUNICODE_UNI_TYPE     The type for a Unicode character.
   UTF8TOUNICODE_FN           The function name for the UTF-8 conversion f'n.
*/


/***********************************************************************
 *
 *  Routine name: UTF8TOUNICODE_UNI_TYPE
 *
 *  Description:  This routine converts a UTF-8 encoded byte stream
 *                into a Unicode string.
 *
 *  Inputs:
 *
 *  Name        Type            Description
 *  ----        ------------    -----------
 *  pctxt       struct	        Pointer to context block structure
 *  inbuf       OSUTF8CHAR*     String of UTF-8 encoded characters
 *  outbufsiz   int             Number of Unicode characters the output
 *                                buffer can hold.
 *
 *  Outputs:
 *
 *  Name        Type                        Description
 *  ----        ------------                -----------
 *  stat        int                         Character count or error status.
 *  outbuf      UTF8TOUNICODE_UNI_TYPE*     Output buffer
 *
 ***********************************************************************/

EXTRTMETHOD long UTF8TOUNICODE_FN
(OSCTXT* pctxt, const OSUTF8CHAR* inbuf, UTF8TOUNICODE_UNI_TYPE* outbuf,
   size_t outbufsiz)
{
   size_t inlen = rtxUTF8LenBytes (inbuf);
   size_t inbufx = 0, outbufx = 0;
   UTF8TOUNICODE_UNI_TYPE value;
   size_t startx;
   int nbytes, result = 0;
   OSOCTET byte;

   while (inbufx < inlen) {
      startx = inbufx;

      /* Make sure there is enough space in the output buffer   */
      /* available to hold the parsed character..               */

      if (0 != outbuf) {
         if (outbufx >= outbufsiz) {
            return LOG_RTERRNEW (pctxt, RTERR_BUFOVFLW);
         }
      }

      /* Process the first byte of the UTF-8 sequence */

      byte = inbuf[inbufx++];
      if (byte < 0x80) {
         nbytes = 0;
         value = byte;
      }
      else if ((byte & 0xe0) == 0xc0) {
         nbytes = 1;
         value = (UTF8TOUNICODE_UNI_TYPE) (byte & 0x1f);
      }
      else if ((byte & 0xf0) == 0xe0) {
         nbytes = 2;
         value = (UTF8TOUNICODE_UNI_TYPE) (byte & 0x0f);
      }
      else if ((byte & 0xf8) == 0xf0) {
         nbytes = 3;
         value = (UTF8TOUNICODE_UNI_TYPE) (byte & 0x07);
      }
      else if ((byte & 0xfc) == 0xf8) {
         nbytes = 4;
         value = (UTF8TOUNICODE_UNI_TYPE) (byte & 0x03);
      }
      else if ((byte & 0xfe) == 0xfc) {
         nbytes = 5;
         value = (UTF8TOUNICODE_UNI_TYPE) (byte & 0x01);
      }
      else {
         rtxErrNewNode (pctxt);
         rtxErrAddUIntParm (pctxt, (OSUINT32)startx);
         return LOG_RTERRNEW (pctxt, RTERR_INVUTF8);
      }

      /* Read the possible remaining bytes.  */

      while (inbufx < inlen && nbytes > 0) {
         byte = inbuf[inbufx++];
         --nbytes;
         if ((byte & 0xc0) != 0x80) {
            /* This is an illegal encoding.  */
            result = RTERR_INVUTF8;
            break;
         }
         value <<= 6;
         value |= byte & 0x3f;
      }

      if (result != 0) {
         rtxErrNewNode (pctxt);
         rtxErrAddUIntParm (pctxt, (OSUINT32)startx);
         return LOG_RTERRNEW (pctxt, result);
      }

      if (0 != outbuf) {
         outbuf[outbufx++] = value;
      }
   }

   return (long)(outbufx);
}



