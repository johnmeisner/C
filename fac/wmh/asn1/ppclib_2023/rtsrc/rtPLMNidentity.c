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

/* PLMNidentity utility functions */

#include "rtsrc/rtPLMNidentity.h"
#include "rtxsrc/osSysTypes.h"
#include "rtxsrc/rtxErrCodes.h"
#include "rtxsrc/rtxCommonDefs.h"
#include "rtxsrc/rtxTBCD.h"
#include "rtxsrc/rtxCtype.h"

/* Converts MCC and MNC strings to PLMNidentity */
EXTRTMETHOD int rtStringToPLMNidentity
(const char* mcc, const char* mnc, OSOCTET* plmnBuffer, size_t bufsiz)
{
   size_t mnclen;
   char elem, b, c;
   int di;
   unsigned int i;

   /* Sanity check the arguments.  */
   if ((0 == mcc) || (0 == mnc) || (0 == plmnBuffer) || (bufsiz < 3))
   {
      return RTERR_INVPARAM;
   }
   if (OSCRTLSTRLEN(mcc) != 3) return RTERR_INVPARAM;
   mnclen = OSCRTLSTRLEN(mnc);
   if ((2 != mnclen) && (3 != mnclen)) return RTERR_INVPARAM;

   elem = 0;
   di = 0;

   /* Deal with the MCC first.  */
   for (i = 0; i < 3; i++)
   {
      c = mcc[i];
      if (!OS_ISDIGIT(c)) return RTERR_INVPARAM;
      b = c - '0';
      if ((i % 2) == 1) /* High-order nibble */
      {
         elem |= (b << 4);
         plmnBuffer[di++] = elem;
         elem = 0;
      }
      else /* Low-order nibble */
      {
         elem = b;
      }
   }

   /* Now deal with the mnc.  Do a sanity check on its digits first.  */
   for (i = 0; i < mnclen; i++)
   {
      c = mnc[i];
      if (!OS_ISDIGIT(c)) return RTERR_INVPARAM;
   }

   /*
   When we get here, we've populated the first byte of plmnBuffer with the first 2
   nibbles of the mcc.  How we populate the second byte depends on the length of
   the mnc.  If it's 2, we add a filler nibble.  If it's 3, we add the first digit
   of the mnc.  elem is equal to the low-order nibble of this second byte.
   */
   if (2 == mnclen)
   {
      elem |= (0xF << 4);
   }
   else
   {
      c = mnc[0];
      b = c - '0';
      elem |= (b << 4);
   }
   plmnBuffer[di++] = elem;
   elem = 0;

   /*
   Now for the third byte of plmnBuffer.  If the length of the mnc is 2, we populate
   this third byte.  with the two digits of the mnc.  If the length of the mnc is 3,
   we populate this third byte with digits 2 and 3 of the mnc.
   */
   if (2 == mnclen)
   {
      c = mnc[0];
      b = c - '0';
      elem = b;
      c = mnc[1];
      b = c - '0';
      elem |= (b << 4);
   }
   else
   {
      c = mnc[1];
      b = c - '0';
      elem = b;
      c = mnc[2];
      b = c - '0';
      elem |= (b << 4);
   }
   plmnBuffer[di] = elem;
   return 0;
}

/* Retrieves MCC from PLMNidentity */
EXTRTMETHOD const char* rtGetMCCFromPLMNidentity
(OSOCTET* plmnIdentity)
{
   char* mcc;
   char b;
   unsigned short i;

   mcc = OSCRTLMALLOC(4);
   if (0 == mcc) return 0;
   b = -1;
   for (i = 1; i <= 3; i++)
   {
      switch (i)
      {
      case 1: /* MCC digit 1 */
         b = plmnIdentity[0] & 0x0F;
         break;
      case 2: /* MCC digit 2 */
         b = (plmnIdentity[0] >> 4) & 0x0F;
         break;
      case 3: /* MCC digit 3 */
         b = plmnIdentity[1] & 0x0F;
         break;
      }
      if ((b < 0) || (b > 9)) return 0;
      rtxTBCDBinToChar(b, &mcc[i - 1]);
   }
   mcc[3] = '\0';

   return mcc;
}

/* Retrieves MNC from PLMNidentity */
EXTRTMETHOD const char* rtGetMNCFromPLMNidentity
(OSOCTET* plmnIdentity)
{
   char* mnc;
   char b, possibleFiller;
   unsigned short i;
   size_t mncsize;

   b = -1;
   possibleFiller = (plmnIdentity[1] >> 4) & 0x0F;

   /* If possibleFiller is 0x0F, we have a 2-byte MNC.  Otherwise, we have a 3-byte MNC.  */
   if (possibleFiller == 0x0F)
   {
      mncsize = 2;
      mnc = OSCRTLMALLOC(mncsize + 1);
      if (0 == mnc) return 0;
      for (i = 1; i <= 2; i++)
      {
         switch (i)
         {
         case 1: /* MNC digit 1 */
            b = plmnIdentity[2] & 0x0F;
            break;
         case 2: /* MNC digit 2 */
            b = (plmnIdentity[2] >> 4) & 0x0F;
            break;
         }
         if ((b < 0) || (b > 9)) return 0;
         rtxTBCDBinToChar(b, &mnc[i - 1]);
      }
   }
   else
   {
      mncsize = 3;
      mnc = OSCRTLMALLOC(mncsize + 1);
      if (0 == mnc) return 0;
      for (i = 1; i <= 3; i++)
      {
         switch (i)
         {
         case 1: /* MNC digit 1 */
            b = possibleFiller;
            break;
         case 2: /* MNC digit 2 */
            b = plmnIdentity[2] & 0x0F;
            break;
         case 3: /* MNC digit 3 */
            b = (plmnIdentity[2] >> 4) & 0x0F;
            break;
         }
         if ((b < 0) || (b > 9)) return 0;
         rtxTBCDBinToChar(b, &mnc[i - 1]);
      }
   }
   mnc[mncsize] = '\0';

   return mnc;
}
