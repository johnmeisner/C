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

/* OSRTINLINE is defined by rtxobj.c when it directly #includes this source
   file.

   DC 11/13/19:  Could not find anywhere in rtxobj.c where OSRTINLINE is
   defined.
*/
#ifndef OSRTINLINE

   /* Includes in this section will probably need to be included in rtxobj.c */
   #include <stdio.h>
   #include <stdlib.h>
   #include <string.h>
   #include "rtxsrc/rtxDiag.h"
   #include "rtxsrc/rtxError.h"
   #include "rtxsrc/rtxFile.h"
   #include "rtxsrc/rtxMemory.h"

   #ifdef _WIN32_WCE
      #include "Shlwapi.h"       /* for PathFileExists */
   #elif defined(_MSC_VER) && _MSC_VER >= 1400        /*VS 2005+*/
      #include <errno.h>
      #include <io.h>
   #elif defined(WIN32)       /* Windows & older than VS 2005 */
      #include <io.h>
   #else                      /* Not Windows */
      #include <unistd.h>
   #endif
#else                      /* OSRTINLINE undefined */
   #ifdef EXTRTMETHOD
      #undef EXTRTMETHOD
   #endif
   #define EXTRTMETHOD static
#endif

#ifdef WIN32
   #include <windows.h>
#endif
   #include <sys/stat.h>


EXTRTMETHOD int rtxFileOpen
(FILE** ppFile, const char* filePath, const char* access)
{
   int ret = 0;

   if (0 != ppFile) {
#if defined(_MSC_VER) && _MSC_VER >= 1400 && !defined(_WIN32_WCE)
      errno_t err = fopen_s (ppFile, filePath, access);
      if (0 != err) {
         RTDIAG2 (0, "errno returned from fopen_s is %d\n", err);
         *ppFile = 0;
         ret = (err == ENOENT) ? RTERR_FILNOTFOU : RTERR_FAILED;
      }
#else
      *ppFile = fopen (filePath, access);
      ret = (0 == *ppFile) ? RTERR_FAILED : 0;
#endif
   }
   else ret = RTERR_NULLPTR;

   return ret;
}

EXTRTMETHOD OSBOOL rtxFileExists (const char* filePath)
{
   OSBOOL ret;
#if defined(XTENSA_TFUSION) || defined(_WIN32_WCE)
   FILE *ptFile;
#endif

#if defined(_MSC_VER) && _MSC_VER >= 1400 && !defined(_WIN32_WCE)
      errno_t err = _access_s (filePath, 0 /* existence only */);

      ret = err == 0;

      if (0 != err) {
         RTDIAG2 (0, "errno returned from access_s is %d\n", err);
      }
#elif defined(WIN32) && !defined(_WIN32_WCE)
      ret = 0 == _access(filePath, 0 /* existence only */);
#elif defined(XTENSA_TFUSION) || defined(_WIN32_WCE)
      ptFile = fopen(filePath, "r");
      if (ptFile != 0) {
         fclose(ptFile);
         ret = TRUE;
      }
      else {
         ret = FALSE;
      }
#else
      ret = 0 == access(filePath, F_OK);
#endif

   return ret;
}

#ifndef OMNITRACS_RT
EXTRTMETHOD OSBOOL rtxFileIsDirectory (const char* filePath)
{
   OSBOOL ret;
#ifndef ARC_DSP
   struct stat sb;

#ifdef _MSC_VER
   if ((stat(filePath, &sb) == 0) && (sb.st_mode & _S_IFDIR)) {
      ret = TRUE;
   }
   else {
      ret = FALSE;
   }
#else
   if ((stat(filePath, &sb) == 0) && (S_ISDIR(sb.st_mode))) {
      ret = TRUE;
}
   else {
      ret = FALSE;
   }
#endif
#else /* ARC_DSP */
   /*
      The ARC DSP configuration apparently has a very limited file system,
      such that none of the attempts I've made to implement this functionality
      work due to functions not being present in the Synopsys C run-time.  So
      we'll just return FALSE here.  If filePath is actually a directory,
      subsequent attempts to treat it as a file will ultimately fail, albeit
      possibly cryptically.
   */
   ret = FALSE;
#endif
   return ret;
}
#endif

#ifdef WIN32
static void SystemTimeToTime_t(SYSTEMTIME *systemTime, time_t *dosTime)
{
   LARGE_INTEGER jan1970FT = {0};
   LARGE_INTEGER utcFT = {0};
   OSUINT64 utcDosTime;

   jan1970FT.QuadPart = OSI64CONST(116444736000000000); // january 1st 1970

   SystemTimeToFileTime(systemTime, (FILETIME*)&utcFT);

   utcDosTime = (utcFT.QuadPart - jan1970FT.QuadPart)/10000000;

   *dosTime = (time_t)utcDosTime;
}
#endif

#ifndef ARC_DSP
EXTRTMETHOD time_t rtxFileLastModified( const char* filePath)
{
   time_t ret = 0;

#ifdef WIN32
   HANDLE hFile;
   hFile = CreateFile(filePath, GENERIC_READ,
                        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                        0 , OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

   if ( hFile != INVALID_HANDLE_VALUE ) {
      FILETIME created, accessed, written;
      SYSTEMTIME stWritten;
      if ( GetFileTime(hFile, &created, &accessed, &written) &&
            FileTimeToSystemTime(&written, &stWritten) )
      {
         SystemTimeToTime_t(&stWritten, &ret);
      }
      CloseHandle(hFile);
   }
   else return 0;
#else
   struct stat attrs;
   if ( stat(filePath, &attrs) == 0 ) {
      ret = attrs.st_mtime;
   }
#endif

   return ret;
}
#endif

static int readOpenFile (OSCTXT* pctxt, FILE* fp,
                         OSOCTET** ppMsgBuf, size_t* pLength,
                         char fileType, OSBOOL useSysMem)
{
   OSOCTET* msgbuf;
   long len;
   size_t len2;

   fseek (fp, 0L, SEEK_END);
   if ((len = ftell(fp)) != -1L) {
      /* If text file, allocate one more char than length to hold
         null terminator char */
      size_t memsize = len;
      if (fileType == 'a') memsize++;

      if (useSysMem)
         msgbuf = (OSOCTET*) rtxMemSysAlloc (pctxt, memsize);
      else
         msgbuf = (OSOCTET*) rtxMemAlloc (pctxt, memsize);

      if (msgbuf == NULL) {
         return LOG_RTERR (pctxt, RTERR_NOMEM);
      }
      fseek (fp, 0L, SEEK_SET);

      len2 = fread (msgbuf, 1, (size_t)len, fp);

      if ((long)len2 != len && fileType == 'b' ) {
         rtxMemFreeArray (pctxt, msgbuf);
         return LOG_RTERR (pctxt, RTERR_READERR);
      }

      *ppMsgBuf = msgbuf;
      *pLength = len2;

      /* Add null terminator */
      if (fileType == 'a') msgbuf[len2] = '\0';

      return 0;
   }
   else {
      return LOG_RTERR (pctxt, RTERR_READERR);
   }
}

static int readFile (OSCTXT* pctxt, const char* filePath,
                     OSOCTET** ppMsgBuf, size_t* pLength,
                     char fileType, OSBOOL useSysMem)
{
   const char* access = (fileType == 'a') ? "r" : "rb";
   FILE* fp;
   int ret = rtxFileOpen (&fp, filePath, access);
   if (0 == ret) {
      ret = readOpenFile(pctxt, fp, ppMsgBuf, pLength, fileType, useSysMem);
      fclose(fp);
      return ret;
   }
   else {
      return LOG_RTERR (pctxt, RTERR_FILNOTFOU);
   }
}

EXTRTMETHOD int rtxFileReadBinary
(OSCTXT* pctxt, const char* filePath, OSOCTET** ppMsgBuf, size_t* pLength)
{
   return readFile (pctxt, filePath, ppMsgBuf, pLength, 'b', FALSE);
}

EXTRTMETHOD int rtxFileReadBinary2
(OSCTXT* pctxt, FILE* pFile, OSOCTET** ppMsgBuf, size_t* pLength)
{
   return readOpenFile (pctxt, pFile, ppMsgBuf, pLength, 'b', FALSE);
}

#ifndef _BUILD_LICOBJ
EXTRTMETHOD int rtxFileReadBinToSysMem
(OSCTXT* pctxt, const char* filePath, OSOCTET** ppMsgBuf, size_t* pLength)
{
   return readFile (pctxt, filePath, ppMsgBuf, pLength, 'b', TRUE);
}

EXTRTMETHOD int rtxFileReadText
(OSCTXT* pctxt, const char* filePath, OSOCTET** ppMsgBuf, size_t* pLength)
{
   return readFile (pctxt, filePath, ppMsgBuf, pLength, 'a', FALSE);
}

EXTRTMETHOD int rtxFileWriteBinary
(const char* filePath, const OSOCTET* pMsgBuf, size_t length)
{
   FILE* fp;
   int ret = rtxFileOpen (&fp, filePath, "wb");
   if (0 == ret) {
      fwrite (pMsgBuf, 1, length, fp);
      fclose (fp);
   }
   else ret = RTERR_WRITEERR;

   return ret;
}

EXTRTMETHOD int rtxFileWriteText
(const char* filePath, const char* pMsgBuf)
{
   FILE* fp;
   int ret = rtxFileOpen (&fp, filePath, "w");
   if (0 == ret) {
      fputs ((char*)pMsgBuf, fp);
      fclose (fp);
   }
   else ret = RTERR_WRITEERR;

   return ret;
}

EXTRTMETHOD int rtxFileCopyTextFile
(const char* srcFilePath, const char* destFilePath)
{
   FILE *infp, *outfp;
   char linebuf[1024];
   int ret;

   ret = rtxFileOpen (&infp, srcFilePath, "r");
   if (0 != ret) return RTERR_FILNOTFOU;

   ret = rtxFileOpen (&outfp, destFilePath, "w");
   if (0 != ret) {
      fclose(infp);
      return RTERR_WRITEERR;
   }

   while (fgets (linebuf, sizeof(linebuf), infp) != NULL) {
      fputs (linebuf, outfp);
   }

   fclose (infp);
   fclose (outfp);

   return 0;
}

EXTRTMETHOD int rtxFileUpdateSizeValue
(const char* filePath, OSSIZE offset, OSSIZE value, OSSIZE nbytes)
{
   FILE* fp;
   int idx = sizeof(OSSIZE), ret;
   OSOCTET ub, encbuf[sizeof(OSSIZE)];

   ret = rtxFileOpen (&fp, filePath, "r+");
   if (0 != ret) return ret;

   fseek (fp, (long)offset, SEEK_SET);

   // Encode value in big-endian form
   OSCRTLMEMSET (encbuf, 0, sizeof(encbuf));
   do {
      ub = (OSOCTET) (value % 256);
      value /= 256;
      encbuf[--idx] = ub;
   } while (value != 0 && idx > 0);

   if (nbytes <= sizeof(encbuf)) {
      OSSIZE startIdx = sizeof(encbuf) - nbytes;
      fwrite (&encbuf[startIdx], 1, nbytes, fp);
   }
   else ret = RTERR_WRITEERR;

   fclose (fp);

   return ret;
}
#endif
