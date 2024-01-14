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

#include <stdarg.h>
#include <stdlib.h>
#include "rtxsrc/osSysTypes.h"
#include "rtxsrc/rtxBitString.h"
#include "rtxsrc/rtxErrCodes.h"


/**
 * Same as rtxCppCheckBitBounds except using OSSIZE for numocts and accepting
 * a maximum value that numocts may be set to.
 */
static int rtxCppCheckBitBounds_internal(OSOCTET*& pBits, OSSIZE& numocts,
   size_t minRequiredBits, size_t preferredLimitBits, OSSIZE max)
{
   size_t minOctets;

   if ( pBits == 0 ) numocts = 0;   //avoid parameter inconsistency

   if ( minRequiredBits == 0 ) minOctets = 1;
   else minOctets = (minRequiredBits - 1) / 8 + 1;


   if ( minOctets > max ) return RTERR_NOMEM;


   if ( minOctets > numocts ) {
      //The current string is too small & needs to be expanded
      //The rules here are:
      //1) must have at least minOctets
      //2) we'll generally grow by at least half-again (minOctets can require
      //   more growth), except we only grow beyond preferredLimitOcts when
      //   minOctets demands it.
      //3) we don't create a string of less than 32 bytes unless
      //   preferredLimitOcts calls for this
      OSOCTET* newBuffer;
      size_t newNumOcts;   /* invariant: newNumOcts <= max */
      size_t preferredLimitOcts = (preferredLimitBits + 7) / 8;

      if ( numocts > 2 * (max / 3) ) newNumOcts = max;
      else newNumOcts = numocts + numocts / 2;


      if ( newNumOcts < 32 ) newNumOcts = 32;
      if ( newNumOcts >= minOctets )
      {
         if ( preferredLimitOcts > 0 && newNumOcts > preferredLimitOcts ) {
            if ( preferredLimitOcts >= minOctets )
               newNumOcts = preferredLimitOcts;
            else
               newNumOcts = minOctets;
         }
      }
      else newNumOcts = minOctets;

      newBuffer = new OSOCTET[newNumOcts];
      if ( newBuffer == NULL ) return RTERR_NOMEM;

      if ( numocts > 0 )
         OSCRTLMEMCPY(newBuffer, pBits, numocts);

      OSCRTLMEMSET(newBuffer + numocts, 0, newNumOcts - numocts);

      delete [] pBits;
      pBits = newBuffer;
      numocts = newNumOcts; /* numocts <= max due to invariant on newNumOcts */
   }

   return 0;
}



EXTRTMETHOD int rtxCppCheckBitBounds(OSOCTET*& pBits, OSUINT32& numocts,
   size_t minRequiredBits, size_t preferredLimitBits)
{
   OSSIZE numocts64 = numocts;
   int ret = rtxCppCheckBitBounds_internal(pBits, numocts64, minRequiredBits,
                                       preferredLimitBits, OSUINT32_MAX);
   if ( ret != 0 ) return ret;
   else {
      numocts = (OSUINT32) numocts64;
      return 0;
   }
}

EXTRTMETHOD int rtxCppCheckBitBounds64(OSOCTET*& pBits, OSSIZE& numocts,
   size_t minRequiredBits, size_t preferredLimitBits)
{
   return rtxCppCheckBitBounds_internal(pBits, numocts, minRequiredBits,
                                       preferredLimitBits, OSSIZE_MAX);
}

