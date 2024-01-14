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

#include <stdio.h>
#include <string.h>
#include <math.h>

#include "rtxsrc/osMacros.h"
#include "rtxsrc/rtxDateTime.h"
#include "rtxsrc/rtxErrCodes.h"
#include "rtxsrc/rtxCharStr.h"
#include "rtxsrc/rtxCtype.h"

/**
 * Defines the validitity of Day Designator, set true/false.
 */
#define VALID_DAY_DESIGNATOR(c) (c == 'Y' || c == 'M'|| c == 'D')

/**
 * Defines the validitity of Time Designator, set true/false.
 */
#define VALID_TIME_DESIGNATOR(c) (c == 'H' || c == 'M'|| c == 'S')

typedef struct DurationFlags {
   OSBOOL yearFlag;
   OSBOOL monFlag;
   OSBOOL dayFlag;
   OSBOOL timeFlag;
   OSBOOL hourFlag;
   OSBOOL minFlag;
   OSBOOL secFlag;
   OSBOOL fracSecFlag;
   OSBOOL isNegative;
} DurationFlags;

static int convertToMsec(OSUTF8CHAR designtr, OSREAL* tmpd,
                                DurationFlags* pdurFlags);

/**
 * rtxMSecsToDuration:
 * This function converts millisecs to duration string like
 * "PnYnMnDTnHnMnS". In case of negative duration a minus sign is prepended
 * to the output string
 * @param msecs       OSINT32 takes millisecs as input parameter
 * @param buf         Pointer to OSUTF8CHAR ie OSUTF8CHAR array.
 * @return              Completion status of operation:
 *                     - 0(RT_OK) successful
 *                     - negative unsuccessful
 *                        (RTERR_NOTINIT/RTERR_BUFOVFLW)
 */
EXTRTMETHOD int rtxMSecsToDuration (OSINT32 msecs, OSUTF8CHAR* buf, OSUINT32 bufsize)
{
   OSINT32 sec,fsec,min,hr,day,mon,yr;
   OSUTF8CHAR sgn = '+';
   OSREAL tmpd;
   char numbuf[16];
   char zerofill[5];

   if(buf == NULL) return RTERR_NOTINIT;

   tmpd = msecs;
   if (tmpd < 0.0){
        sgn = '-';
        tmpd = -tmpd;
   }

   sec = (int)(tmpd/1000);
   fsec = (int)(tmpd - (sec * 1000.0));
   min = sec / 60;
   sec = sec % 60;
   hr = min / 60;
   min = min % 60;
   day = hr / 24;
   hr = hr % 24;
   mon = day / 30;
   day = day % 30;
   yr = mon / 12;
   mon = mon % 12;

   /* zerofill array is updated for proper generation of string for
      fractional value */
   if (fsec > 0 && fsec <10) {
      if (sec > 0)
         rtxStrcpy (zerofill, sizeof(zerofill), ".00");
      else
         rtxStrcpy (zerofill, sizeof(zerofill), "0.00");
   }
   else if (fsec >9 && fsec <100) {
      if (sec > 0)
         rtxStrcpy (zerofill, sizeof(zerofill), ".0");
      else
         rtxStrcpy (zerofill, sizeof(zerofill), "0.0");
   }
   else if (fsec == 0) {
      if (msecs == 0)
         rtxStrcpy (zerofill, sizeof(zerofill), "0");
      else
         rtxStrcpy (zerofill, sizeof(zerofill),  "");
   }
   else rtxStrcpy (zerofill, sizeof(zerofill), ".");

   buf[0] = 0;
   if (sgn == '-') {
      rtxStrcat ((char*)buf, bufsize, "-");
   }
   rtxStrcat ((char*)buf, bufsize, "-");
   if (yr > 0) {
      rtxIntToCharStr (yr, numbuf, sizeof(numbuf), 0);
      rtxStrcat ((char*)buf, bufsize, numbuf);
      rtxStrcat ((char*)buf, bufsize, "Y");
   }
   if (mon > 0) {
      rtxIntToCharStr (mon, numbuf, sizeof(numbuf), 0);
      rtxStrcat ((char*)buf, bufsize, numbuf);
      rtxStrcat ((char*)buf, bufsize, "M");
   }
   if (day > 0) {
      rtxIntToCharStr (day, numbuf, sizeof(numbuf), 0);
      rtxStrcat ((char*)buf, bufsize, numbuf);
      rtxStrcat ((char*)buf, bufsize, "D");
   }
   if (hr > 0 || min > 0 || sec > 0 || fsec > 0 || msecs == 0) {
      rtxStrcat ((char*)buf, bufsize, "T");
      if (hr > 0) {
         rtxIntToCharStr (hr, numbuf, sizeof(numbuf), 0);
         rtxStrcat ((char*)buf, bufsize, numbuf);
         rtxStrcat ((char*)buf, bufsize, "H");
      }
      if (min > 0) {
         rtxIntToCharStr (min, numbuf, sizeof(numbuf), 0);
         rtxStrcat ((char*)buf, bufsize, numbuf);
         rtxStrcat ((char*)buf, bufsize, "M");
      }
      if (sec > 0) {
         rtxIntToCharStr (sec, numbuf, sizeof(numbuf), 0);
         rtxStrcat ((char*)buf, bufsize, numbuf);
      }
      rtxStrcat ((char*)buf, bufsize, zerofill);
      if (fsec > 0) {
         rtxIntToCharStr (fsec, numbuf, sizeof(numbuf), 0);
         rtxStrcat ((char*)buf, bufsize, numbuf);
      }
      if (sec > 0 || fsec >0 || msecs == 0) {
         rtxStrcat ((char*)buf, bufsize, "S");
      }
   }

   return 0;
}


/**
 * rtxDurationToMSecs:
 * This function converts duration string to milisecs. In case of string
 * prepended by -ve sign the duration in milisec will have negative value.
 * The duration string is parsed from left to right and whenever it devuates
 * from its format, or buffer runout or number overflow occurs error is
 * returned. If it obeys every rule then success status is returned.
 * @param buf         Pointer to OSUTF8CHAR array.
 * @param bufsize     OSINT32 indicates the bufsize to be read.
 * @param msecs       OSINT32 updated after calculation.
 * @return            Completion status of operation:
 *                  - 0(RT_OK) = success,
 *                  - negative return value is error
 *             (RTERR_NOTINIT/RTERR_BUFOVFLW/RTERR_INVFORMAT/RTERR_TOOBIG).
 *             Return value is taken from rtxErrCodes.h header file
 */
EXTRTMETHOD int rtxDurationToMSecs (OSUTF8CHAR* buf, OSUINT32 bufsize, OSINT32* msecs)
{
   OSINT32 stat = 0;          /* stores the msecs conv status (0,-1,-2) */
   OSREAL secVal = 0;         /* stores the second value of S designator */
   OSREAL msecValue = 0.0;    /* stores all msec value and gets updated */
   OSREAL tmpd = 0.0;         /* temp double variable to store number string
                              this is passed by ref to convertToMsec()  */
   OSUINT32 digitcnt = 0; /* counts the number of fractional digits*/
   OSUINT32 bufcnt = 0;   /* buffer counter */
   /* durFlags stores flag value for different designator*/
   DurationFlags durFlags =
        {FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE};
   DurationFlags* pdurFlags = &durFlags;

   /* Check if the buf is null */
   if(buf == NULL) return RTERR_NOTINIT;

   /* Check if length of buf string is more than bufsize */
   if(OSCRTLSTRLEN((const char*)buf) >= bufsize) return RTERR_BUFOVFLW;

   /* Check if the string starts with + or - or P  and bufcnt is < bufsize
      otherwisw return error message */
   if (*buf == '-'|| *buf == '+'){
      if (*buf == '-') pdurFlags->isNegative = TRUE;

      bufcnt++;
      buf++;
      if (*buf != 'P') return RTERR_INVFORMAT;
      if(bufcnt >= bufsize) return RTERR_BUFOVFLW;

      bufcnt++;
      buf++;
      if(bufcnt >= bufsize) return RTERR_BUFOVFLW;
   } else if (*buf != 'P'){
        return RTERR_INVFORMAT;
   } else {
        bufcnt++;
        buf++;
        if (*buf == '\0') return RTERR_INVFORMAT;
        if(bufcnt >= bufsize) return RTERR_BUFOVFLW;
   }

   /* Check if the next cahracter after P is digit,if it is T the duration
      string has only Time Part */
   if (OS_ISDIGIT(*buf) == 0) {
      if (*buf != 'T') return RTERR_INVFORMAT;
      pdurFlags->timeFlag = TRUE;
   } else {
        for (;;) {
           tmpd = 0.0;

           /* this while loop evaluate number present between two
           designators also check for invalid format and buffer overflow  */
           while (OS_ISDIGIT(*buf) != 0) {
              tmpd = tmpd * 10 + (*buf - '0');

              bufcnt++;
              buf++;
              if (*buf == '\0') return RTERR_INVFORMAT;
              if (bufcnt >= bufsize) return RTERR_BUFOVFLW;
           }

           /* check for OSINT32 range */
           if (pdurFlags->isNegative == (OSBOOL)TRUE) {
              if (-tmpd < OSINT32_MIN) return RTERR_TOOBIG;
           } else if (tmpd > OSINT32_MAX) return RTERR_TOOBIG;

           /*  convert tmpd to msec if designator is valid*/
           if (VALID_DAY_DESIGNATOR(*buf)) {
              stat = convertToMsec(*buf, &tmpd, pdurFlags);
              if (stat == -1) return RTERR_INVFORMAT;
              if (stat == -2) return RTERR_TOOBIG;

              msecValue = msecValue + tmpd;
              if (pdurFlags->isNegative == (OSBOOL)TRUE) {
                 if (-msecValue < OSINT32_MIN) return RTERR_TOOBIG;
              } else if (msecValue > OSINT32_MAX) return RTERR_TOOBIG;

              bufcnt++;
              buf++;
              if (*buf == '\0') break;
              if (bufcnt >= bufsize) return RTERR_BUFOVFLW;
              if (OS_ISDIGIT(*buf) == 0 ){
                 if (*buf != 'T') return RTERR_INVFORMAT;
                 if (pdurFlags->timeFlag == (OSBOOL)FALSE) {
                    pdurFlags->timeFlag = TRUE;
                    break;
                 }
              }
           } else {
                return RTERR_INVFORMAT;
           }
        }
   }

   if (pdurFlags->timeFlag == (OSBOOL)TRUE){

      bufcnt++;
      buf++;
      if (*buf == '\0') return RTERR_INVFORMAT;
      if (bufcnt >= bufsize) return RTERR_BUFOVFLW;
      for (;;) {
         digitcnt = 0;
         tmpd = 0.0;

         /* this while loop evaluate number present between two  designators
         update digitcnt also check for invalid format and buffer overflow  */
         while (OS_ISDIGIT(*buf) != 0) {
            tmpd = tmpd * 10 + (*buf - '0');
            bufcnt++;
            buf++;
            digitcnt++;
            if (*buf == '\0') return RTERR_INVFORMAT;
            if (bufcnt >= bufsize) return RTERR_BUFOVFLW;
         }

        /* check for OSINT32 range */
        if (pdurFlags->isNegative == (OSBOOL)TRUE) {
            if (-tmpd < OSINT32_MIN) return RTERR_TOOBIG;
         } else if (tmpd > OSINT32_MAX) return RTERR_TOOBIG;

         /*  convert tmpd to msec if designator is valid*/
         if (VALID_TIME_DESIGNATOR(*buf)) {
            if (*buf == 'S' && pdurFlags->fracSecFlag == (OSBOOL)TRUE) {
               tmpd = ((double)(tmpd/pow(10.0,(double)digitcnt)))*1000;
            }
            stat = convertToMsec(*buf, &tmpd, pdurFlags);
            if (stat == -1) return RTERR_INVFORMAT;
            if (stat == -2) return RTERR_TOOBIG;
            if (*buf == 'S'){
               msecValue = msecValue + tmpd + secVal;
            } else {
               msecValue = msecValue + tmpd;
            }

            if (pdurFlags->isNegative == (OSBOOL)TRUE) {
               if (-msecValue < OSINT32_MIN) return RTERR_TOOBIG;
            } else if (msecValue > OSINT32_MAX) return RTERR_TOOBIG;
         } else {
              if (*buf == '.') {
                 stat = convertToMsec(*buf, &tmpd, pdurFlags);
                 if (stat == -2) return RTERR_TOOBIG;
                 secVal = tmpd;
              } else return RTERR_INVFORMAT;
         }
         bufcnt++;
         buf++;
         if (*buf == '\0') break;
         if (bufcnt >= bufsize) return RTERR_BUFOVFLW;
         if (OS_ISDIGIT(*buf) == 0 ) return RTERR_INVFORMAT;
      }
   }

   if (*buf == '\0'){

   /*check the validity of string checking the flags designators should be
     unique.and there order should be maintained.*/
      if (pdurFlags->timeFlag == (OSBOOL)TRUE &&
          (pdurFlags->hourFlag == (OSBOOL)FALSE &&
           pdurFlags->minFlag == (OSBOOL)FALSE &&
           pdurFlags->secFlag == (OSBOOL)FALSE)) return RTERR_INVFORMAT;
      if (pdurFlags->timeFlag == (OSBOOL)FALSE &&
           (pdurFlags->yearFlag == (OSBOOL)FALSE &&
             pdurFlags->monFlag == (OSBOOL)FALSE &&
             pdurFlags->dayFlag == (OSBOOL)FALSE)) return RTERR_INVFORMAT;
   }
   if (pdurFlags->isNegative == (OSBOOL)TRUE)
      *msecs = (OSINT32)(-(msecValue));
   else
      *msecs = (OSINT32)msecValue;
   return RT_OK;
}

/**
 * This function converts different date and time parameter to millisecs.
 * This function return -ve integer if it does not obey the duration format
 * or data type overflow occurs. designators should be unique.and there
 * order should be maintained.The format of duration string is
 * "+/-PnYnMnDTnHnMnS".
 *
 * @param designtr      this determines the duration designator
 * @param tmpd          converts this to msecs depending on designtr
 * @param pdurFlags     update this duration flag structure depending
 *                      on the presence of designator.
 * @return              0  - if success,
 *                      -1 - if invalid format
 *                      -2 - if long int overflow occurs.
 */
static int convertToMsec(OSUTF8CHAR designtr, OSREAL* tmpd,
                               DurationFlags* pdurFlags)
{
   switch(designtr) {

   /* 'Y' is Year designator. If yearFlag/monFlag/dayFlag is already set
       return -1  otherwise convert it to msecs. */
   case 'Y':
      if (pdurFlags->yearFlag == (OSBOOL)TRUE ||
          pdurFlags->monFlag == (OSBOOL)TRUE ||
          pdurFlags->dayFlag == (OSBOOL)TRUE) return -1;
      *tmpd = *tmpd*12*30*24*60*60*1000;
      pdurFlags->yearFlag = TRUE;
      break;

   /* 'M' is Month/Minute designator. If timeFlag is true this indicates
     Month designator else Minute.*/
   case 'M':
      if (pdurFlags->timeFlag == (OSBOOL)TRUE) {

      /* If minFlag/fracSecFlag/secFlag is already set return -1
          otherwise convert it to msecs.*/
      if(pdurFlags->minFlag == (OSBOOL)TRUE ||
         pdurFlags->fracSecFlag == (OSBOOL)TRUE ||
         pdurFlags->secFlag == (OSBOOL)TRUE) return -1;
      *tmpd = *tmpd*60*1000;
      pdurFlags->minFlag = TRUE;
      } else {

        /* If monFlag/dayFlag is already set return -1
        otherwise convert it to msecs.*/
           if(pdurFlags->monFlag ||
              pdurFlags->dayFlag == (OSBOOL)TRUE) return -1;
           *tmpd = *tmpd*30*24*60*60*1000;
           pdurFlags->monFlag = TRUE;
      }
      break;

   /* 'D' is Day designator. If dayFlag is already set return -1
        otherwise convert it to msecs. */
   case 'D':
      if (pdurFlags->dayFlag == (OSBOOL)TRUE) return -1;
         *tmpd = *tmpd*24*60*60*1000;
         pdurFlags->dayFlag = TRUE;
         break;

   /* 'H' is Hour designator. If hourFlag/fracSecFlag/minFlag/SecFlag
        is already set return -1 otherwise convert it to msecs. */
   case 'H':
      if(pdurFlags->hourFlag == (OSBOOL)TRUE ||
         pdurFlags->fracSecFlag == (OSBOOL)TRUE ||
         pdurFlags->minFlag == (OSBOOL)TRUE ||
         pdurFlags->secFlag == (OSBOOL)TRUE) return -1;
      *tmpd = *tmpd*60*60*1000;
      pdurFlags->hourFlag = TRUE;
      break;

   /* 'S' is Seconds designator. If secFlag is already set return -1
      otherwise convert it to msecs. */
   case 'S':
      if(pdurFlags->secFlag == (OSBOOL)TRUE) return -1;
      if(pdurFlags->fracSecFlag == (OSBOOL)FALSE)
         *tmpd = *tmpd*1000;
      pdurFlags->secFlag = TRUE;
      break;

   /* '.' is frac designator. convert it to msecs. */
   case '.':
      *tmpd = *tmpd*1000;
      pdurFlags->fracSecFlag = TRUE;
      break;
   default:
      break;
   }

   /* If *tmpd is greater than the maximum value of long int return -2.*/
   if (pdurFlags->isNegative == (OSBOOL)TRUE) {
      if (-(*tmpd) < OSINT32_MIN) return -2;
   } else if (*tmpd > OSINT32_MAX) return -2;


    /* If success return 0 */
    return 0;
}


