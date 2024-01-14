/*
 * Copyright (c) 2003-2023 Objective Systems, Inc.
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
#include "rtxsrc/osSysTypes.h"
#include "rtxsrc/rtxCtype.h"
#include "rtxsrc/rtxErrCodes.h"
#include "rtxsrc/rtxUtil.h"

EXTRTMETHOD OSSIZE rtxSizeByteCount( OSSIZE val )
{
   OSSIZE retval = sizeof(OSSIZE);
   OSSIZE mask = (OSSIZE)0xFF << (8 * (sizeof(OSSIZE) - 1));

   if ( (mask & val) > 0 ) return retval;
   else {
      retval--;
      mask = mask >> 8;
   }

   return retval;
}


/*
 * This method returns the number of bytes required to encode an
 * identifier value.
 */
EXTRTMETHOD OSUINT32 rtxGetIdentByteCount (OSUINT32 ident)
{
   if (ident < (1ul << 7)) {         /* 7 */
      return 1;
   }
   else if (ident < (1ul << 14)) {   /* 14 */
      return 2;
   }
   else if (ident < (1ul << 21)) {   /* 21 */
      return 3;
   }
   else if (ident < (1ul << 28)) {   /* 28 */
      return 4;
   }
   return 5;
}

/*
 * Returns the smallest octet length needed to
 * hold the given long int value
 */
EXTRTMETHOD OSUINT32 rtxIntByteCount (OSINT32 val) {
   OSUINT32 mask = (0x7f80L << ((sizeof (OSINT32) - 2) * 8));
   OSUINT32 retVal = sizeof (OSINT32);

   if (val < 0)
      val = val ^ (~0L);  /* XOR val with all 1's */

   while ((retVal > 1) && ((val & mask) == 0)) {
      mask >>= 8;
      retVal--;
   }

   return retVal;
}

/*
* rtxOctetBitLen (val) is the number of bits in val.
*/
EXTRTMETHOD OSUINT32 rtxOctetBitLen (OSOCTET w) {
   /* binary search */
   return
      (w & 0xF) ? ((w & 3) ? ((w & 1) ? 8: 7) : ((w & 4) ? 6 : 5)) :
      ((w & 0x30) ? ((w & 0x10) ? 4: 3) : ((w & 0x40) ? 2 : (w) ? 1 : 0));
}

EXTRTMETHOD OSUINT32 rtxUInt32BitLen (OSUINT32 value)
{
   /* Binary search - decision tree (5 tests, rarely 6) */
   return
      ((value < 1<<15) ?
       ((value < 1<<7) ?
        ((value < 1<<3) ?
         ((value < 1<<1) ?
          ((value < 1<<0) ? 0 : 1) : ((value < 1<<2) ? 2 : 3)) :
         ((value < 1<<5) ?
          ((value < 1<<4) ? 4 : 5) : ((value < 1<<6) ? 6 : 7))) :
        ((value < 1<<11) ?
         ((value < 1<<9) ?
          ((value < 1<<8) ? 8 : 9) : ((value < 1<<10) ? 10 : 11)) :
         ((value < 1<<13) ?
          ((value < 1<<12) ? 12 : 13) : ((value < 1<<14) ? 14 : 15)))) :
       ((value < 1<<23) ?
        ((value < 1<<19) ?
         ((value < 1<<17) ?
          ((value < 1<<16) ? 16 : 17) : ((value < 1<<18) ? 18 : 19)) :
         ((value < 1<<21) ?
          ((value < 1<<20) ? 20 : 21) : ((value < 1<<22) ? 22 : 23))) :
        ((value < 1<<27) ?
         ((value < 1<<25) ?
          ((value < 1<<24) ? 24 : 25) : ((value < 1<<26) ? 26 : 27)) :
         ((value < 1<<29) ?
          ((value < 1<<28) ? 28 : 29) :
          ((value < 1<<30) ? 30 : ((value < 1UL<<31) ? 31 : 32))))));
}

EXTRTMETHOD OSUINT32 rtxLog2Ceil (OSUINT32 w)
{
   OSUINT32 val = 0, lastval, bit = 0;

   do {
      bit++;
      lastval = val;
      val = 1 << bit;
   }
   while (val > lastval && w > val);

   return bit;
}

EXTRTMETHOD OSUINT32 rtxLog2Floor (OSUINT32 w)
{
   OSUINT32 val = 0, lastval, bit = 0;

   do {
      bit++;
      lastval = val;
      val = 1 << bit;
   }
   while (val > lastval && w > val);

   if (val != w) bit--;

   return bit;
}


EXTRTMETHOD OSUINT32 rtxLog10Floor (OSUINT32 w)
{
   int ret = 0;

   if ( w == 0 ) return OSUINT32_MAX;     /* w == 0 is not legal */

   while ( w >= 10 )
   {
      ret++;
      w = w / 10;
   }

   return ret;
}

EXTERNRT int rtxIpv4AddrToBin
(const char* ipv4str, OSOCTET* outbuf, OSSIZE bufsize)
{
   if (ipv4str && outbuf && bufsize >= 4) {
      OSSIZE i;
      OSUINT32 part;
      for (i = 0; i < 4; i++) {
         part = 0;
         while (*ipv4str != '.' && *ipv4str != '\0') {
            if (OS_ISDIGIT(*ipv4str)) {
               part = (part * 10) + (*ipv4str - '0');
               if (part > 255) return RTERR_TOOBIG;
            }
            else if (!OS_ISSPACE(*ipv4str)) {
               return RTERR_INVCHAR;
            }
            ipv4str++;
         }
         outbuf[i] = (OSOCTET)part;
         if (*ipv4str == '\0') { i++; break; }
      }

      return (i != 4 || *ipv4str != '\0') ? RTERR_BADVALUE : 0;
   }

   return RTERR_INVPARAM;
}
