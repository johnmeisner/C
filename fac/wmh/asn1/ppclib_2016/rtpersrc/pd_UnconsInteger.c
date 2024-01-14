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

/*
Define the following when including this file:
PUI_FNNAME: name to use for function (e.g. pd_UnconsInteger)
PUI_TYPE: type to use for the value (non-pointer type; e.g. OSINT32)
PUI_UNSIGNED: define this only if PUI_TYPE is an unsigned type;
   the defined value doesn't matter.

These will be undefined by this module.
*/

#define STRINGIFY(s) #s

EXTPERMETHOD int PUI_FNNAME (OSCTXT* pctxt, PUI_TYPE* pvalue)
{
   size_t MAX_BYTES = sizeof(PUI_TYPE);
   signed char b;
   unsigned char ub;
   OSUINT32 nbytes;
   int stat;

   RTDIAGSTRM2 (pctxt, STRINGIFY(PUI_FNNAME) ": start\n");

   stat = pd_Length (pctxt, &nbytes);
   if (stat < 0) return LOG_RTERR (pctxt, stat);

   RTDIAGSTRM3 (pctxt, STRINGIFY(PUI_FNNAME) ": nbytes is %d\n", nbytes);

   PU_NEWFIELD (pctxt, "UnconsInteger");

   if (nbytes > 0) {

      /* Align buffer */

      if (pctxt->buffer.aligned) {
         stat = PD_BYTE_ALIGN (pctxt);
         if (stat != 0) return LOG_RTERR (pctxt, stat);
      }

      /* Decode first byte into a signed byte value and assign to integer. */
      /* This should handle sign extension..                               */

      stat = pd_octets (pctxt, (OSOCTET*)&b, 1, 8);
      if (stat != 0) return LOG_RTERR (pctxt, stat);

#ifdef PUI_UNSIGNED
      if ( b < 0 ) return LOG_RTERR(pctxt, RTERR_TOOBIG); /* negative value */
      else if ( b == 0 ) MAX_BYTES++;    /* don't count this byte by allowing
                                             an extra byte */
#endif

      if (nbytes > MAX_BYTES) {
         /* value does not fit in pvalue */
         return LOG_RTERR(pctxt, RTERR_TOOBIG);
      }

      *pvalue = b;
      nbytes--;

      /* Decode remaining bytes and add to result */

      while (nbytes > 0) {
         stat = pd_octets (pctxt, (OSOCTET*)&ub, 1, 8);
         if (stat != 0) return LOG_RTERR (pctxt, stat);

         *pvalue = (PUI_TYPE)((*pvalue * 256) + ub);
         nbytes--;
      }
   }
   else {  /* nbytes == 0 */
      /* technically not correct, but a common encoding mistake */
      *pvalue = 0;
   }

   PU_SETBITCOUNT (pctxt);
   RTDIAGSTRM3 (pctxt, STRINGIFY(PUI_FNNAME) ": decoded value is "OSINT64FMT"\n", *pvalue);

   return 0;
}

#undef PUI_FNNAME
#undef PUI_TYPE
#undef PUI_UNSIGNED
