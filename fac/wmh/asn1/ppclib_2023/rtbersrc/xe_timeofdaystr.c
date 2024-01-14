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
#include "rtxsrc/rtxDateTime.hh"


/* This function will parse the various hour/minute/second parts of an ASN.1
 * ISO 8601 Time string.
 */
EXTERNBER int berParseIso8601TimeStr(OSCTXT* pctxt, const char** ppvalue,
                                     OSUINT8 max)
{
   int stat = 0;
   OSUINT8 val = 0;

   PARSE_2_DIGITS(val, (*ppvalue), stat);
   if (stat != 0)
   {
      return LOG_RTERR(pctxt, RTERR_INVFORMAT);
   }

   if (val > max)
   {
      return LOG_RTERR(pctxt, RTERR_INVFORMAT);
   }

   if (**ppvalue == '\0')
   {
      stat = 1;
   }
   else if (**ppvalue == 'Z')
   {
      // The UTC designator must be last character in the string if present...
      (*ppvalue)++;
      if (**ppvalue != '\0')
      {
         return LOG_RTERR(pctxt, RTERR_INVFORMAT);
      }
      stat = 1;
   }

   return stat;
}

EXTERNBER int berValidateIso8601TimeStr(OSCTXT* pctxt, const char** ppvalue)
{
   int stat = 0;

   if ((**ppvalue == '-') || (**ppvalue == '+'))
   {
      // Handle time difference string...
      (*ppvalue)++;

      stat = berParseIso8601TimeStr(pctxt, ppvalue, 14);
      if (stat == 1)
      {
         return 0; // Finished with string...
      }
      else if (stat < 0)
      {
         return LOG_RTERR(pctxt, RTERR_INVFORMAT);
      }

      if (**ppvalue != ':')
      {
         return LOG_RTERR(pctxt, RTERR_INVFORMAT);
      }
      (*ppvalue)++;

      stat = berParseIso8601TimeStr(pctxt, ppvalue, 59);
      if (stat < 0)
      {
         return LOG_RTERR(pctxt, RTERR_INVFORMAT);
      }
      return 0;
   }

   // If we get here, we've got a "normal" timestamp beginning (and possibly
   //  ending) with an hour value...
   stat = berParseIso8601TimeStr(pctxt, ppvalue, 24);
   if (stat == 1)
   {
      return 0; // Finished with string...
   }
   else if (stat < 0)
   {
      return LOG_RTERR(pctxt, RTERR_INVFORMAT);
   }

   while (TRUE)
   {
      if ((**ppvalue == '\0') || (**ppvalue == '/'))
      {
         // The slash means we've finished handling a time string within a
         //  Duration string...
         break;
      }

      if (**ppvalue == ':')
      {
         // Should be handling either minutes or seconds here...
         (*ppvalue)++;
         stat = berParseIso8601TimeStr(pctxt, ppvalue, 59);
         if (stat < 0)
         {
            return LOG_RTERR(pctxt, RTERR_INVFORMAT);
         }
      }
      else if ((**ppvalue == '-') || (**ppvalue == '+'))
      {
         (*ppvalue)++;
         stat = berParseIso8601TimeStr(pctxt, ppvalue, 24);
         if (stat == 1)
         {
            break;
         }
         else if (stat < 0)
         {
            return LOG_RTERR(pctxt, RTERR_INVFORMAT);
         }

         if (**ppvalue != ':')
         {
            return LOG_RTERR(pctxt, RTERR_INVFORMAT);
         }
         (*ppvalue)++;

         stat = berParseIso8601TimeStr(pctxt, ppvalue, 59);
         if (stat < 0)
         {
            return LOG_RTERR(pctxt, RTERR_INVFORMAT);
         }
      }
      else if ((**ppvalue == '.') || (**ppvalue == ','))
      {
         (*ppvalue)++;
         while (OS_ISDIGIT(**ppvalue))
         {
            (*ppvalue)++;
         }
      }
      else if (**ppvalue == 'Z')
      {
         // The UTC designator must be last character in the string if present...
         (*ppvalue)++;
         if (**ppvalue != '\0')
         {
            return LOG_RTERR(pctxt, RTERR_INVFORMAT);
         }
         break;
      }
      else
      {
         return LOG_RTERR(pctxt, RTERR_INVFORMAT);
      }
   } // end while-loop

   return 0;
}

int xe_timeofdaystr(OSCTXT* pctxt, const char* pvalue,
                    ASN1TagType tagging, ASN1TAG tag)
{
   OSUINT32 i, j = 0;
   char *p = NULL;
   const char *pval = pvalue;
   int stat = 0;

   if (NULL == pvalue)
   {
      return 0;
   }

   stat = berValidateIso8601TimeStr(pctxt, &pval);
   if (0 != stat)
   {
      return LOG_RTERR(pctxt, stat);
   }

   // If we get here, the time string is valid.  Walk through it and pull out
   //  colon characters...
   p = (char *)malloc(strlen(pvalue) + 1);
   for (i = 0; i < strlen(pvalue); i++)
   {
      if (pvalue[i] != ':')
      {
         p[j++] = pvalue[i];
      }
   }
   p[j] = '\0';

   stat = xe_charstr(pctxt, p, tagging, tag);

   free(p);
   return stat;
}
