#include "rtpersrc/pe_common.hh"

/***********************************************************************
 *
 *  Routine name: pe_2sCompBinInt
 *
 *  Description:  This routine encompasses the rules to encode a
 *                2's complement binary integer as specified in 
 *                section 10.4 of the X.691 standard.
 *
 *  Inputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  pctxt      struct* Pointer to ASN.1 PER context structure
 *  value       uint    Value to be encoded
 *
 *  Outputs:
 *
 *  Name        Type    Description
 *  ----        ----    -----------
 *  status      int     Completion status of encode operation
 *
 *
 **********************************************************************/

EXTPERMETHOD int pe_2sCompBinInt (OSCTXT* pctxt, OSINT32 value)
{
   /* 10.4.6  A minimum octet 2's-complement-binary-integer encoding    */
   /* of the whole number has a field width that is a multiple of 8     */
   /* bits and also satisifies the condition that the leading 9 bits    */
   /* field shall not be all zeros and shall not be all ones.           */

   /* first encode integer value into a local buffer */

   OSOCTET lbuf[8], lb;
   OSINT32 i, temp = value;

   OSCRTLMEMSET (lbuf, 0, sizeof(lbuf));
   for (i = (OSINT32)(sizeof(lbuf)-1); i >= 0; i--) {
      lb = (OSOCTET)(temp % 256);
      temp /= 256;
      if (temp < 0 && lb != 0) temp--; /* two's complement adjustment */
      lbuf[i] = lb;
      if (temp == 0 || temp == -1) break;
   }

   /* If the value is positive and bit 8 of the leading byte is set,    */
   /* copy a zero byte to the contents to signal a positive number..    */

   if (value > 0 && (lb & 0x80) != 0) {
      i--;
   }

   /* If the value is negative and bit 8 of the leading byte is clear,  */
   /* copy a -1 byte (0xFF) to the contents to signal a negative        */
   /* number..                                                          */

   else if (value < 0 && ((lb & 0x80) == 0)) {
      i--;
      if (i >= 0 && i < (OSINT32)(sizeof(lbuf))) {
         lbuf[i] = 0xff;
      }
   }

   /* Add the data to the encode buffer */

   return pe_octets (pctxt, &lbuf[i], (sizeof(lbuf) - i) * 8);
}

