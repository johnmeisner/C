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
#include "rtxsrc/rtxCtype.h"

#define MAX_DATE_SUFFIXES 4
#define MAX_SUFFIXES 7
static char suffix[MAX_SUFFIXES] = {'Y', 'M', 'W', 'D', 'H', 'M', 'S'};


/* This function will parse the Duration (i.e. non-timestamp) part of an ASN.1
 * ISO 8601 Duration string.
 */
int berParseDuration(OSCTXT* pctxt, const char** ppvalue, OSUINT8 max)
{
   char c;
   OSBOOL date = TRUE;
   int i, stat = 0;

   // First character in Duration string must either be a digit (for date
   //  fields, or a "T" (no date fields, just time fields)...
   if ((**ppvalue != 'T') && !OS_ISDIGIT(**ppvalue))
   {
      return LOG_RTERR(pctxt, RTERR_INVFORMAT);
   }

   while (TRUE)
   {
      if ((**ppvalue == '\0')|| (**ppvalue == '/'))
      {
         stat = 1;
         break;
      }

      if (**ppvalue == 'T')
      {
         date = FALSE; // Now process time portion of string...
         (*ppvalue)++;
      }

      if (date)
      {
         // Parsing date portion of Duration string...
         if ((**ppvalue == '.') || (**ppvalue == ','))
         {
            (*ppvalue)++;
         }

         while (OS_ISDIGIT(**ppvalue))
         {
            (*ppvalue)++;
         }

         // "c" should now be pointing to a date-related suffix...
         c = *(*ppvalue)++;
         for (i = 0; i < MAX_DATE_SUFFIXES; i++)
         {
            if (c == suffix[i])
            {
               break;
            }
         }

         if (i == MAX_DATE_SUFFIXES)
         {
            // Found a non-digit that was not a proper suffix character...
            return LOG_RTERR(pctxt, RTERR_INVFORMAT);
         }
      }
      else
      {
         // Parsing time portion of Duration string...
         while (OS_ISDIGIT(**ppvalue))
         {
            (*ppvalue)++;
         }

         // "c" should now be pointing to either a time-related suffix or a
         //  fraction designator ("." or ",")...
         c = *(*ppvalue)++;

         if ((c == '.') || (c == ','))
         {
            if (!OS_ISDIGIT(**ppvalue))
            {
               return LOG_RTERR(pctxt, RTERR_INVFORMAT);
            }

            while (OS_ISDIGIT(**ppvalue))
            {
               (*ppvalue)++;
            }

            c = *(*ppvalue)++;
         }

         // Check if we've found one of the time-related suffixes...
         for (i = MAX_DATE_SUFFIXES; i < MAX_SUFFIXES; i++)
         {
            if (c == suffix[i])
            {
               break;
            }
         }

         if (i == MAX_SUFFIXES)
         {
            // Found a non-digit that was not a proper suffix character...
            return LOG_RTERR(pctxt, RTERR_INVFORMAT);
         }
      }
   } // end while-loop

   return stat;
}

EXTERNBER int berValidateIso8601DurationStr(OSCTXT* pctxt, const char** ppvalue)
{
   int stat = 0;

   // Check if "P" is present.  If not, it's a "start timestamp / end
   //   timestamp" interval...
   if (strchr(*ppvalue, 'P') == NULL)
   {
      if (**ppvalue == 'R')
      {
         (*ppvalue)++; // Shift to # of recurrences (if present)...
         while (OS_ISDIGIT(**ppvalue))
         {
            (*ppvalue)++;
         }

         if (*(*ppvalue)++ != '/')
         {
            return LOG_RTERR(pctxt, RTERR_INVFORMAT);
         }
      }

      // Handle start interval/timestamp...
      stat = berValidateIso8601DateStr(pctxt, ppvalue);
      if (0 != stat)
      {
         return LOG_RTERR(pctxt, stat);
      }

      if (**ppvalue == 'T')
      {
         (*ppvalue)++;
      }
      else
      {
         return LOG_RTERR(pctxt, stat);
      }

      stat = berValidateIso8601TimeStr(pctxt, ppvalue);
      if (0 != stat)
      {
         return LOG_RTERR(pctxt, stat);
      }

      if (**ppvalue == '/')
      {
         (*ppvalue)++;
      }
      else
      {
         return LOG_RTERR(pctxt, stat);
      }

      // Handle end interval/timestamp...
      stat = berValidateIso8601DateStr(pctxt, ppvalue);
      if (0 != stat)
      {
         return LOG_RTERR(pctxt, stat);
      }

      if (**ppvalue == 'T')
      {
         (*ppvalue)++;
      }
      else
      {
         return LOG_RTERR(pctxt, stat);
      }

      stat = berValidateIso8601TimeStr(pctxt, ppvalue);
      if (0 != stat)
      {
         return LOG_RTERR(pctxt, stat);
      }

      // Should be at end of string at this point...
      if (**ppvalue != '\0')
      {
         return LOG_RTERR(pctxt, RTERR_INVFORMAT);
      }
      return 0;
   }

   // Validate recurrence (if present)....
   if (**ppvalue == 'R')
   {
      (*ppvalue)++; // Shift to # of recurrences (if present)...

      while (OS_ISDIGIT(**ppvalue))
      {
         (*ppvalue)++;
      }

      if (*(*ppvalue)++ != '/')
      {
         return LOG_RTERR(pctxt, RTERR_INVFORMAT);
      }

   }

   // At this point, the next char must either be "P", "T" or a digit...
   // Duration must have a P (Period) designator...
   if ((**ppvalue != 'P') && (**ppvalue != 'T') && !OS_ISDIGIT(**ppvalue))
   {
      return LOG_RTERR(pctxt, RTERR_INVFORMAT);
   }

   while (TRUE)
   {
      if (**ppvalue == '\0')
      {
         break;
      }

      // Handle first part of Duration (i.e. the string prior to the "/")...
      if (**ppvalue == 'P')
      {
         (*ppvalue)++;
         stat = berParseDuration(pctxt, ppvalue, 14);
         if (stat == 1)
         {
            return 0; // Finished with string...
         }
         else if (stat < 0)
         {
            return LOG_RTERR(pctxt, RTERR_INVFORMAT);
         }
      }
      else
      {
         // Timestamp...
         if (OS_ISDIGIT(**ppvalue))
         {
            stat = berValidateIso8601DateStr(pctxt, ppvalue);
            if (0 != stat)
            {
               return LOG_RTERR(pctxt, stat);
            }

            if (**ppvalue == '/')
            {
               (*ppvalue)++;
            }
            else if (**ppvalue == 'T')
            {
               stat = berValidateIso8601TimeStr(pctxt, ppvalue);
               if (0 != stat)
               {
                  return LOG_RTERR(pctxt, stat);
               }
            }
            else
            {
               return LOG_RTERR(pctxt, stat);
            }
         }
         else if (**ppvalue == 'T')
         {
            stat = berValidateIso8601TimeStr(pctxt, ppvalue);
            if (0 != stat)
            {
               return LOG_RTERR(pctxt, stat);
            }
         }
      }
   } // end while-loop

   return 0;
}

int xe_durationstr(OSCTXT* pctxt, const char* pvalue,
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

   stat = berValidateIso8601DurationStr(pctxt, &pval);
   if (0 != stat)
   {
      return LOG_RTERR(pctxt, stat);
   }

   // If we get here, the duration string is valid.  Walk through it and pull
   //  out the "P" character...
   p = (char *)malloc(strlen(pvalue) + 1);
   for (i = 0; i < strlen(pvalue); i++)
   {
      if (pvalue[i] != 'P')
      {
         p[j++] = pvalue[i];
      }
   }
   p[j] = '\0';

   stat = xe_charstr(pctxt, p, tagging, tag);

   free(p);
   return stat;
}
