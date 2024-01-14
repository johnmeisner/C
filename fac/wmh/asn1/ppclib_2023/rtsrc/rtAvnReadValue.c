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

#include "rtxsrc/rtxBuffer.h"
#include "rtxsrc/rtxCtype.h"
#include "rtxsrc/rtxError.h"
#include "rtxsrc/rtxMemBuf.h"
#include "rtxsrc/rtxText.h"

#include "rtxsrc/rtxBuffer.h"
#include "rtxsrc/rtxText.h"
#include "rtAvn.h"

/**
 * Having already read the opening double quote, read the rest of a cstring.
 */
static int readCString(OSCTXT* pctxt, OSRTMEMBUF* pmembuf)
{
   int ret = 0;
   OSOCTET ub;    /* a byte from input */

   for (;;) {
      ret = rtxReadBytes(pctxt, &ub, 1);
      if (ret != 0) return LOG_RTERR(pctxt, ret);

      ret = rtxMemBufAppend(pmembuf, &ub, 1);
      if (ret != 0) return LOG_RTERR(pctxt, ret);

      if (ub == '"') {
         /* Peek at next char.  If present and a second double quote, this is
            escape and not end of the string */
         ret = rtxPeekByte(pctxt, &ub);
         if ( ret == 1 && ub == '"') {
            /* Not end of string.  Process the second " through. */
            ret = rtxMemBufAppend(pmembuf, &ub, 1);
            if (ret != 0) return LOG_RTERR(pctxt, ret);
            pctxt->buffer.byteIndex++;
         }
         else {
            if (ret == 1) ret = 0;
            break; /* end of string (or possibly error left for later) */
         }
      }
   }

   return ret;
}


/**
 * Having already read the opening --, read the rest of a single-line comment.
 */
static int readLineComment(OSCTXT* pctxt, OSRTMEMBUF* pmembuf)
{
   int ret = 0;
   OSOCTET ub;    /* a byte from input */

   for (;;) {
      ret = rtxPeekByte(pctxt, &ub);
      if ( ret == 0 ) break;  /* end of input is end of comment */
      else if ( ret < 0 ) return LOG_RTERR(pctxt, ret);

      if (ub >= 10 && ub <= 13) break; /* new line is end of comment*/

      pctxt->buffer.byteIndex++;

      ret = rtxMemBufAppend(pmembuf, &ub, 1);
      if (ret != 0) return LOG_RTERR(pctxt, ret);

      if (ub == '-') {
         /* Peek at next char.  If it is '-', append it to buffer and
            that's end of comment. */
         ret = rtxPeekByte(pctxt, &ub);
         if (ret == 1 && ub == '-') {
            ret = rtxMemBufAppend(pmembuf, &ub, 1);
            if (ret != 0) return LOG_RTERR(pctxt, ret);

            pctxt->buffer.byteIndex++;
            break;
         }
      }
   }

   return ret;
}

/**
 * Having already read the opening / *, read the rest of a multi-line comment.
 */
static int readMultiLineComment(OSCTXT* pctxt, OSRTMEMBUF* pmembuf)
{
   int ret = 0;
   OSOCTET ub;    /* a byte from input */

   for (;;) {
      ret = rtxPeekByte(pctxt, &ub);
      if (ret == 0) break;  /* end of input is end of comment */
      else if (ret < 0) return LOG_RTERR(pctxt, ret);

      pctxt->buffer.byteIndex++;

      ret = rtxMemBufAppend(pmembuf, &ub, 1);
      if (ret != 0) return LOG_RTERR(pctxt, ret);

      if (ub == '*') {
         /* Peek at next char.  If it is '/', append it to buffer and
            that's end of comment. */
         ret = rtxPeekByte(pctxt, &ub);
         if (ret == 1 && ub == '/') {
            ret = rtxMemBufAppend(pmembuf, &ub, 1);
            if (ret != 0) return LOG_RTERR(pctxt, ret);

            pctxt->buffer.byteIndex++;
            break;
         }
      }
   }

   return ret;
}


/**
 * Read text for an ASN.1 value into the given buffer.
 * Note: this is separated from rtAvnReadOpenType in anticipation of possibly
 * also implementing an rtAvnReadValue function, which would be a thin
 * wrapper around this function.
 */
static int readValue(OSCTXT* pctxt, OSRTMEMBUF* pmembuf)
{
   /* Note: We don't go to great lengths to validate that the input is
      valid.  For example, if a bstring or hstring contains invalid characters
      or ends prematurely, we don't detect that.  There are doubtless numerous
      such cases.  The main objective is to capture all the input and get past
      the value.
   */

   int ret;
   OSOCTET ub;    /* a byte from input */
   int level;     /* level of nesting */

   /* TRUE if the value we're reading is a brace value; it ends with the closing
      brace.  Otherwise, FALSE; the value ends when a comma or } at the outer
      level is seen or when end of input is reached. */
   OSBOOL braceValue;

   ret = rtxTxtSkipWhitespace(pctxt);
   if ( ret != 0 ) return LOG_RTERR(pctxt, ret);

   ret = rtxReadBytes(pctxt, &ub, 1);
   if (ret != 0) return LOG_RTERR(pctxt, ret);

   if (ub == '{') {
      /* The value is a brace value. */
      level = 1;
      braceValue = TRUE;
   }
   else {
      /* The value is not a brace value .*/
      level = 0;
      braceValue = FALSE;
   }

   ret = rtxMemBufAppend(pmembuf, &ub, 1);
   if (ret != 0) return LOG_RTERR(pctxt, ret);

   for (;;) {
      /* Read the next character, allowing that EOF may be occur in certain
         conditions.
      */
      if (braceValue) {
         /* Read character.  EOF would be an error. */
         ret = rtxReadBytes(pctxt, &ub, 1);
         if (ret != 0) return LOG_RTERR(pctxt, ret);
      }
      else {
         /* Peek character.  ',' or '}' at level 0, or EOF, mean we're done with
            input. */
         ret = rtxPeekByte(pctxt, &ub);
         if ( ret == 0 ) break;  /* EOF */
         else if (ret < 0) return LOG_RTERR(pctxt, ret);
         else {
            ret = 0;
            if (level == 0 && (ub == ',' || ub == '}'))
               break; /* done reading value */
            else pctxt->buffer.byteIndex++;    /* treat ub as read */
         }
      }

      ret = rtxMemBufAppend(pmembuf, &ub, 1);
      if (ret != 0) return LOG_RTERR(pctxt, ret);

      if (ub == '\"') {
         /* Read rest of cstring */
         ret = readCString(pctxt, pmembuf);
         if (ret != 0) return LOG_RTERR(pctxt, ret);
      }
      else if (ub == '/') {
         /* Peek at next character.  If it is '*' we have a comment
            to read. */
         ret = rtxPeekByte(pctxt, &ub);
         if (ret == 1 && ub == '*') {
            ret = rtxMemBufAppend(pmembuf, &ub, 1);
            if (ret != 0) return LOG_RTERR(pctxt, ret);

            pctxt->buffer.byteIndex++;
            /* Read rest of comment into membuf */
            ret = readMultiLineComment(pctxt, pmembuf);
            if (ret != 0) return LOG_RTERR(pctxt, ret);
         } /* else error or not '*'; continue as normal */
      }
      else if (ub == '-') {
         /* Peek at next character.  If it is '-' we have a comment
            to read. */
         ret = rtxPeekByte(pctxt, &ub);
         if (ret == 1 && ub == '-') {

            ret = rtxMemBufAppend(pmembuf, &ub, 1);
            if (ret != 0) return LOG_RTERR(pctxt, ret);

            pctxt->buffer.byteIndex++;
            /* Read rest of comment into membuf */
            ret = readLineComment(pctxt, pmembuf);
         } /* else error or not '-'; continue as normal */
      }
      else if (ub == '{') {
         level++;
      }
      else if (ub == '}') {
         level--;
         if ( level == 0 && braceValue) break;  /* done */
      }
   }

   return ret;
}


EXTRTMETHOD int rtAvnReadOpenType(OSCTXT* pctxt, const OSOCTET** ppdata,
   OSSIZE* pnocts)
{
   int ret;
   OSRTMEMBUF membuf;

   rtxMemBufInit(pctxt, &membuf, 256);

   ret = rtAvnReadIdentifierToBuffer(pctxt, &membuf);
   if (ret != 0) return LOG_RTERR(pctxt, ret);

   /* Trim the null terminator out. */
   rtxMemBufCut(&membuf, rtxMemBufGetDataLen(&membuf)-1, 1);

   ret = rtxTxtMatchChar(pctxt, ':', TRUE);
   if (ret != 0) return LOG_RTERR(pctxt, ret);

   ret = rtxMemBufAppend(&membuf, (OSOCTET*) " : ", 3);
   if (ret != 0) return LOG_RTERR(pctxt, ret);

   ret = readValue(pctxt, &membuf);

   if (ret == 0) {
      *ppdata = rtxMemBufGetDataExt(&membuf, pnocts);
   }

   return ret;
}


