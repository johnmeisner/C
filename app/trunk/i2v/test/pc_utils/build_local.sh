#!/bin/sh
rm *.o
rm ../lib/*

#
# libasn1rt
#
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxsrc/rtxArrayList.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxsrc/rtxBase64.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxsrc/rtxBigInt.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxsrc/rtxBigIntArithmetic.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxsrc/rtxBigIntCommon.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxsrc/rtxBitDecode.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxsrc/rtxBitEncode.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxsrc/rtxBitString.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxsrc/rtxCharStr.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxsrc/rtxCheckBuffer.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxsrc/rtxContext.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxsrc/rtxContextInit.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxsrc/rtxContextInitExt.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxsrc/rtxCopyAsciiText.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxsrc/rtxCopyUTF8Text.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxsrc/rtxDatesParse.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxsrc/rtxDatesPrint.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxsrc/rtxDatesToString.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxsrc/rtxDateTime.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxsrc/rtxDateTimeCompare.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxsrc/rtxDateTimeCompare2.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxsrc/rtxDateTimeParse.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxsrc/rtxDateTimeParseCommon.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxsrc/rtxDateTimeToString.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxsrc/rtxDateTimeToStringCommon.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxsrc/rtxDateTimePrint.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxsrc/rtxDecimalHelper.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxsrc/rtxDiag.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxsrc/rtxDiagBitTrace.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxsrc/rtxDiagBitTraceHTML.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxsrc/rtxDiagBitTracePrint.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxsrc/rtxDList.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxsrc/rtxDuration.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxsrc/rtxDynBitSet.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxsrc/rtxDynPtrArray.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxsrc/rtxError.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxsrc/rtxFile.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxsrc/rtxFloat.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxsrc/rtxHashMap.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxsrc/rtxHashMapStr2Int.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxsrc/rtxHashMapStr2UInt.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxsrc/rtxHexCharsToBin.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxsrc/rtxHexDump.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxsrc/rtxIntToCharStr.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxsrc/rtxInt64ToCharStr.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxsrc/rtxLookupEnum.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxsrc/rtxMemBuf.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxsrc/rtxMemFreeOpenSeqExt.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxsrc/rtxMemFuncs.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxsrc/rtxMemHeap.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxsrc/rtxMemHeapAlloc.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxsrc/rtxMemHeapAutoPtr.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxsrc/rtxMemHeapCreate.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxsrc/rtxMemHeapCreateExt.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxsrc/rtxMemHeapDebug.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxsrc/rtxMemHeapFree.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxsrc/rtxMemHeapInvalidPtrHook.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxsrc/rtxMemHeapMarkSaved.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxsrc/rtxMemHeapRealloc.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxsrc/rtxMemHeapReset.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxsrc/rtxPattern.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxsrc/rtxPrint.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxsrc/rtxPrintStream.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxsrc/rtxPrintToStream.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxsrc/rtxReadBytes.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxsrc/rtxRegExp.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxsrc/rtxRegExp2.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxsrc/rtxRegExpPrint.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxsrc/rtxRealHelper.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxsrc/rtxScalarDList.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxsrc/rtxSList.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxsrc/rtxSocket.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxsrc/rtxSOAP.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxsrc/rtxStack.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxsrc/rtxStream.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxsrc/rtxStreamBuffered.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxsrc/rtxStreamCtxtBuf.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxsrc/rtxStreamDirectBuf.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxsrc/rtxStreamFile.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxsrc/rtxStreamMemory.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxsrc/rtxStreamRelease.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxsrc/rtxStreamSocket.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxsrc/rtxToken.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxsrc/rtxTestNumericEnum.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxsrc/rtxTestNumericEnum64.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxsrc/rtxTestNumericEnumU64.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxsrc/rtxTestNumericEnumU32.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxsrc/rtxTokenConst.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxsrc/rtxUIntToCharStr.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxsrc/rtxUInt64ToCharStr.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxsrc/rtxUTF8.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxsrc/rtxUTF8StrToBool.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxsrc/rtxUTF8StrToDouble.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxsrc/rtxUTF8StrToDynHexStr.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxsrc/rtxUTF8StrToInt.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxsrc/rtxUTF8StrToInt64.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxsrc/rtxUTF8StrToUInt.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxsrc/rtxUTF8StrToUInt64.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxsrc/rtxUTF8StrToNamedBits.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxsrc/rtxUTF8ToDynUniStr.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxsrc/rtxUnicode.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxsrc/rtxUtil.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxsrc/rtxUtil64.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxsrc/rtxWriteBytes.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxsrc/rtxXmlQName.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxsrc/rtxXmlStr.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxsrc/rtxMemory.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -c -I../..  ../../rtsrc/asn1CEvtHndlr.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -c -I../..  ../../rtsrc/rt16BitChars.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -c -I../..  ../../rtsrc/rt32BitChars.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -c -I../..  ../../rtsrc/rtBCD.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -c -I../..  ../../rtsrc/rtcompare.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -c -I../..  ../../rtsrc/rtContext.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -c -I../..  ../../rtsrc/rtconv.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -c -I../..  ../../rtsrc/rtcopy.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -c -I../..  ../../rtsrc/rtError.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -c -I../..  ../../rtsrc/rtMakeGeneralizedTime.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -c -I../..  ../../rtsrc/rtMakeUTCTime.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -c -I../..  ../../rtsrc/rtNewContext.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -c -I../..  ../../rtsrc/rtOID.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -c -I../..  ../../rtsrc/rtParseGeneralizedTime.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -c -I../..  ../../rtsrc/rtParseUTCTime.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -c -I../..  ../../rtsrc/rtPrint.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -c -I../..  ../../rtsrc/rtPrintToStream.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -c -I../..  ../../rtsrc/rtPrintToString.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -c -I../..  ../../rtsrc/rtStream.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -c -I../..  ../../rtsrc/rtTable.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -c -I../..  ../../rtsrc/rtUTF8StrToDynBitStr.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -c -I../..  ../../rtsrc/rtValidateStr.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -c -I../..  ../../rtsrc/rtGetLibInfo.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -c -I../..  ../../rtsrc/rtevalstub.c
gcc -O2 -fPIC -shared -Wl,-soname,libasn1rt.so -o ../lib/libasn1rt.so rtxArrayList.o rtxBase64.o rtxBigInt.o rtxBigIntArithmetic.o rtxBigIntCommon.o rtxBitDecode.o rtxBitEncode.o rtxBitString.o rtxCharStr.o rtxCheckBuffer.o rtxContext.o rtxContextInit.o rtxContextInitExt.o rtxCopyAsciiText.o rtxCopyUTF8Text.o rtxDatesParse.o rtxDatesPrint.o rtxDatesToString.o rtxDateTime.o rtxDateTimeCompare.o rtxDateTimeCompare2.o rtxDateTimeParse.o rtxDateTimeParseCommon.o rtxDateTimeToString.o rtxDateTimeToStringCommon.o rtxDateTimePrint.o rtxDecimalHelper.o rtxDiag.o rtxDiagBitTrace.o rtxDiagBitTraceHTML.o rtxDiagBitTracePrint.o rtxDList.o rtxDuration.o rtxDynBitSet.o rtxDynPtrArray.o rtxError.o rtxFile.o rtxFloat.o rtxHashMap.o rtxHashMapStr2Int.o rtxHashMapStr2UInt.o rtxHexCharsToBin.o rtxHexDump.o rtxIntToCharStr.o rtxInt64ToCharStr.o rtxLookupEnum.o rtxMemBuf.o rtxMemFreeOpenSeqExt.o rtxMemFuncs.o rtxMemHeap.o rtxMemHeapAlloc.o rtxMemHeapAutoPtr.o rtxMemHeapCreate.o rtxMemHeapCreateExt.o rtxMemHeapDebug.o rtxMemHeapFree.o rtxMemHeapInvalidPtrHook.o rtxMemHeapMarkSaved.o rtxMemHeapRealloc.o rtxMemHeapReset.o rtxPattern.o rtxPrint.o rtxPrintStream.o rtxPrintToStream.o rtxReadBytes.o rtxRegExp.o rtxRegExp2.o rtxRegExpPrint.o rtxRealHelper.o rtxScalarDList.o rtxSList.o rtxSocket.o rtxSOAP.o rtxStack.o rtxStream.o rtxStreamBuffered.o rtxStreamCtxtBuf.o rtxStreamDirectBuf.o rtxStreamFile.o rtxStreamMemory.o rtxStreamRelease.o rtxStreamSocket.o rtxToken.o rtxTestNumericEnum.o rtxTestNumericEnum64.o rtxTestNumericEnumU64.o rtxTestNumericEnumU32.o rtxTokenConst.o rtxUIntToCharStr.o rtxUInt64ToCharStr.o rtxUTF8.o rtxUTF8StrToBool.o rtxUTF8StrToDouble.o rtxUTF8StrToDynHexStr.o rtxUTF8StrToInt.o rtxUTF8StrToInt64.o rtxUTF8StrToUInt.o rtxUTF8StrToUInt64.o rtxUTF8StrToNamedBits.o rtxUTF8ToDynUniStr.o rtxUnicode.o rtxUtil.o rtxUtil64.o rtxWriteBytes.o rtxXmlQName.o rtxXmlStr.o  rtxMemory.o asn1CEvtHndlr.o rt16BitChars.o rt32BitChars.o rtBCD.o rtcompare.o rtContext.o rtconv.o rtcopy.o rtError.o rtMakeGeneralizedTime.o rtMakeUTCTime.o rtNewContext.o rtOID.o rtParseGeneralizedTime.o rtParseUTCTime.o rtPrint.o rtPrintToStream.o rtPrintToString.o rtStream.o rtTable.o rtUTF8StrToDynBitStr.o rtValidateStr.o rtGetLibInfo.o rtevalstub.o -lm

#
# libasn1ber
#
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/berError.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/berGetLibInfo.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/berTagToString.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/xd_real.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/xd_indeflen.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/xd_enum.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/xd_count.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/xd_common.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/xd_match.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/xd_match1.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/xd_chkend.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/xd_charstr.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/xd_bcdstr.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/xd_boolean.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/xd_bitstr_s.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/xd_bitstr.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/xd_bigint.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/xd_16BitCharStr.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/xd_32BitCharStr.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/xd_unsigned.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/xd_uint8.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/xd_uint16.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/xd_OpenTypeExt.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/xd_OpenType.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/xd_OpenTypeAppend.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/xd_octstr_s.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/xd_octstr.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/xd_objid.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/xd_oid64.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/xd_reloid.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/xd_null.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/xd_NextElement.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/xd_MovePastEOC.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/xd_memcpy.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/xd_len.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/xd_integer.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/xd_int8.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/xd_int16.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/xd_int64.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/xd_uint64.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/xd_setp.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/xd_Tag1AndLen.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/xd_consstr.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/xdf_TagAndLen.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/xdf_ReadContents.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/xe_real.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/xe_common.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/xe_charstr.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/xe_bcdstr.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/xe_boolean.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/xe_bitstr.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/xe_bigint.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/xe_16BitCharStr.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/xe_32BitCharStr.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/xe_unsigned.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/xe_uint8.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/xe_uint16.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/xe_TagAndIndefLen.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/xe_OpenType.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/xe_OpenTypeExt.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/xe_octstr.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/xe_objid.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/xe_oid64.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/xe_reloid.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/xe_null.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/xe_memcpy.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/xe_integer.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/xe_int8.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/xe_int16.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/xe_int64.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/xe_uint64.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/xe_enum.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/xe_derCanonicalSort.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/xe_ptr.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/xe_free.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/xu_BufferState.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/xu_fdump.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/xu_fmt_tag.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/xu_fmt_contents.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/berDecDecimal.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/berDecReal10.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/berEncReal10.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/derEncReal10.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/berStrmInitContext.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/berStrmFreeContext.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/berEncStrmBigInt.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/berEncStrmBitStr.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/berEncStrmBMPStr.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/berEncStrmBool.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/berEncStrmCharStr.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/berEncStrmCommon.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/berEncStrmEnum.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/berEncStrmInt.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/berEncStrmInt8.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/berEncStrmInt16.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/berEncStrmInt64.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/berEncStrmIntValue.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/berEncStrmNull.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/berEncStrmObjId.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/berEncStrmObjId64.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/berEncStrmOctStr.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/berEncStrmReal.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/berEncStrmRelativeOID.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/berEncStrmUInt.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/berEncStrmUInt8.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/berEncStrmUInt16.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/berEncStrmUInt64.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/berEncStrmUnivStr.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/berEncStrmOpenTypeExt.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/berEncStrmXSDAny.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/berDecStrmBitStr.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/berDecStrmBigInt.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/berDecStrmBMPStr.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/berDecStrmCharStr.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/berDecStrmCommon.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/berDecStrmDynBitStr.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/berDecStrmDynOctStr.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/berDecStrmEnum.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/berDecStrmInt.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/berDecStrmInt8.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/berDecStrmInt16.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/berDecStrmInt64.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/berDecStrmNextElement.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/berDecStrmNull.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/berDecStrmObjId.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/berDecStrmObjId64.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/berDecStrmOctStr.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/berDecStrmOpenType.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/berDecStrmOpenTypeAppend.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/berDecStrmOpenTypeExt.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/berDecStrmPeekTagAndLen.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/berDecStrmReadTLV.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/berDecStrmReal.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/berDecStrmRelativeOID.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/berDecStrmUInt.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/berDecStrmUInt8.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/berDecStrmUInt16.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/berDecStrmUInt64.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/berDecStrmUnivStr.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/berDecStrmBool.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/berSocket.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/berDecStrmDecimal.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/berDecStrmReal10.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/berEncStrmReal10.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/cerEncStrmBMPStr.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/cerEncStrmBitStr.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/cerEncStrmCharStr.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/cerEncStrmOctStr.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/cerEncStrmUnivStr.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/cerEncCanonicalSort.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/cerEncStrmReal10.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtbersrc -c -I../..  ../../rtbersrc/derEncBitString.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -c -I../.. -I../../rtsrc -I../../rtxsrc  -o rtevalstubb.o -D_BER ../../rtsrc/rtevalstub.c
gcc -O2 -fPIC -shared -Wl,-soname,libasn1ber.so -o ../lib/libasn1ber.so berError.o berGetLibInfo.o berTagToString.o xd_real.o xd_indeflen.o xd_enum.o xd_count.o xd_common.o xd_match.o xd_match1.o xd_chkend.o xd_charstr.o xd_bcdstr.o xd_boolean.o xd_bitstr_s.o xd_bitstr.o xd_bigint.o xd_16BitCharStr.o xd_32BitCharStr.o xd_unsigned.o xd_uint8.o xd_uint16.o xd_OpenTypeExt.o xd_OpenType.o xd_OpenTypeAppend.o xd_octstr_s.o xd_octstr.o xd_objid.o xd_oid64.o xd_reloid.o xd_null.o xd_NextElement.o xd_MovePastEOC.o xd_memcpy.o xd_len.o xd_integer.o xd_int8.o xd_int16.o xd_int64.o xd_uint64.o xd_setp.o xd_Tag1AndLen.o xd_consstr.o xdf_TagAndLen.o xdf_ReadContents.o xe_real.o xe_common.o xe_charstr.o xe_bcdstr.o xe_boolean.o xe_bitstr.o xe_bigint.o xe_16BitCharStr.o xe_32BitCharStr.o xe_unsigned.o xe_uint8.o xe_uint16.o xe_TagAndIndefLen.o xe_OpenType.o xe_OpenTypeExt.o xe_octstr.o xe_objid.o xe_oid64.o xe_reloid.o xe_null.o xe_memcpy.o xe_integer.o xe_int8.o xe_int16.o xe_int64.o xe_uint64.o xe_enum.o xe_derCanonicalSort.o xe_ptr.o xe_free.o xu_BufferState.o xu_fdump.o xu_fmt_tag.o xu_fmt_contents.o berDecDecimal.o berDecReal10.o berEncReal10.o derEncReal10.o berStrmInitContext.o berStrmFreeContext.o berEncStrmBigInt.o berEncStrmBitStr.o berEncStrmBMPStr.o berEncStrmBool.o berEncStrmCharStr.o berEncStrmCommon.o berEncStrmEnum.o berEncStrmInt.o berEncStrmInt8.o berEncStrmInt16.o berEncStrmInt64.o berEncStrmIntValue.o berEncStrmNull.o berEncStrmObjId.o berEncStrmObjId64.o berEncStrmOctStr.o berEncStrmReal.o berEncStrmRelativeOID.o berEncStrmUInt.o berEncStrmUInt8.o berEncStrmUInt16.o berEncStrmUInt64.o berEncStrmUnivStr.o berEncStrmOpenTypeExt.o berEncStrmXSDAny.o berDecStrmBitStr.o berDecStrmBigInt.o berDecStrmBMPStr.o berDecStrmCharStr.o berDecStrmCommon.o berDecStrmDynBitStr.o berDecStrmDynOctStr.o berDecStrmEnum.o berDecStrmInt.o berDecStrmInt8.o berDecStrmInt16.o berDecStrmInt64.o berDecStrmNextElement.o berDecStrmNull.o berDecStrmObjId.o berDecStrmObjId64.o berDecStrmOctStr.o berDecStrmOpenType.o berDecStrmOpenTypeAppend.o berDecStrmOpenTypeExt.o berDecStrmPeekTagAndLen.o berDecStrmReadTLV.o berDecStrmReal.o berDecStrmRelativeOID.o berDecStrmUInt.o berDecStrmUInt8.o berDecStrmUInt16.o berDecStrmUInt64.o berDecStrmUnivStr.o berDecStrmBool.o berSocket.o berDecStrmDecimal.o berDecStrmReal10.o berEncStrmReal10.o cerEncStrmBMPStr.o cerEncStrmBitStr.o cerEncStrmCharStr.o cerEncStrmOctStr.o cerEncStrmUnivStr.o cerEncCanonicalSort.o cerEncStrmReal10.o derEncBitString.o rtevalstubb.o -lm

#
# libasn1per
#
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtpersrc -c -I../..  ../../rtpersrc/pd_16BitConstrainedString.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtpersrc -c -I../..  ../../rtpersrc/pd_32BitConstrainedString.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtpersrc -c -I../..  ../../rtpersrc/pd_BigInteger.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtpersrc -c -I../..  ../../rtpersrc/pd_BigIntegerEx.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtpersrc -c -I../..  ../../rtpersrc/pd_BitString.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtpersrc -c -I../..  ../../rtpersrc/pd_BMPString.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtpersrc -c -I../..  ../../rtpersrc/pd_UniversalString.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtpersrc -c -I../..  ../../rtpersrc/pd_ChoiceOpenTypeExt.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtpersrc -c -I../..  ../../rtpersrc/pd_common.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtpersrc -c -I../..  ../../rtpersrc/pd_ConsWholeNumber64.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtpersrc -c -I../..  ../../rtpersrc/pd_ConsInteger.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtpersrc -c -I../..  ../../rtpersrc/pd_ConsInt8.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtpersrc -c -I../..  ../../rtpersrc/pd_ConsInt16.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtpersrc -c -I../..  ../../rtpersrc/pd_ConsInt64.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtpersrc -c -I../..  ../../rtpersrc/pd_ConstrainedString.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtpersrc -c -I../..  ../../rtpersrc/pd_ConsUnsigned.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtpersrc -c -I../..  ../../rtpersrc/pd_ConsUInt8.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtpersrc -c -I../..  ../../rtpersrc/pd_ConsUInt16.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtpersrc -c -I../..  ../../rtpersrc/pd_ConsUInt64.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtpersrc -c -I../..  ../../rtpersrc/pd_DynBitString.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtpersrc -c -I../..  ../../rtpersrc/pd_DynOctetString.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtpersrc -c -I../..  ../../rtpersrc/pd_extension.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtpersrc -c -I../..  ../../rtpersrc/pd_ObjectIdentifier.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtpersrc -c -I../..  ../../rtpersrc/pd_oid64.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtpersrc -c -I../..  ../../rtpersrc/pd_RelativeOID.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtpersrc -c -I../..  ../../rtpersrc/pd_OctetString.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtpersrc -c -I../..  ../../rtpersrc/pd_OpenType.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtpersrc -c -I../..  ../../rtpersrc/pd_OpenTypeExt.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtpersrc -c -I../..  ../../rtpersrc/pd_Real.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtpersrc -c -I../..  ../../rtpersrc/pd_SemiConsInteger.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtpersrc -c -I../..  ../../rtpersrc/pd_SemiConsUnsigned.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtpersrc -c -I../..  ../../rtpersrc/pd_SemiConsInt64.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtpersrc -c -I../..  ../../rtpersrc/pd_SemiConsUInt64.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtpersrc -c -I../..  ../../rtpersrc/pd_VarWidthCharString.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtpersrc -c -I../..  ../../rtpersrc/pd_deprecated.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtpersrc -c -I../..  ../../rtpersrc/pe_16BitConstrainedString.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtpersrc -c -I../..  ../../rtpersrc/pe_32BitConstrainedString.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtpersrc -c -I../..  ../../rtpersrc/pe_BigInteger.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtpersrc -c -I../..  ../../rtpersrc/pe_BitString.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtpersrc -c -I../..  ../../rtpersrc/pe_BMPString.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtpersrc -c -I../..  ../../rtpersrc/pe_UniversalString.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtpersrc -c -I../..  ../../rtpersrc/pe_ChoiceTypeExt.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtpersrc -c -I../..  ../../rtpersrc/pe_common.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtpersrc -c -I../..  ../../rtpersrc/pe_common64.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtpersrc -c -I../..  ../../rtpersrc/pe_ConsWholeNumber64.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtpersrc -c -I../..  ../../rtpersrc/pe_ConsInteger.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtpersrc -c -I../..  ../../rtpersrc/pe_ConsInt64.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtpersrc -c -I../..  ../../rtpersrc/pe_ConstrainedString.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtpersrc -c -I../..  ../../rtpersrc/pe_ConsUnsigned.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtpersrc -c -I../..  ../../rtpersrc/pe_ConsUInt64.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtpersrc -c -I../..  ../../rtpersrc/pe_identifier.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtpersrc -c -I../..  ../../rtpersrc/pe_extension.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtpersrc -c -I../..  ../../rtpersrc/pe_ObjectIdentifier.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtpersrc -c -I../..  ../../rtpersrc/pe_identifier64.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtpersrc -c -I../..  ../../rtpersrc/pe_oid64.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtpersrc -c -I../..  ../../rtpersrc/pe_RelativeOID.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtpersrc -c -I../..  ../../rtpersrc/pe_OctetString.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtpersrc -c -I../..  ../../rtpersrc/pe_OpenType.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtpersrc -c -I../..  ../../rtpersrc/pe_OpenTypeExt.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtpersrc -c -I../..  ../../rtpersrc/pe_Real.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtpersrc -c -I../..  ../../rtpersrc/pe_SemiConsInteger.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtpersrc -c -I../..  ../../rtpersrc/pe_SemiConsInt64.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtpersrc -c -I../..  ../../rtpersrc/pe_SemiConsUnsigned.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtpersrc -c -I../..  ../../rtpersrc/pe_SemiConsUInt64.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtpersrc -c -I../..  ../../rtpersrc/pe_VarWidthCharString.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtpersrc -c -I../..  ../../rtpersrc/pe_2sCompBinInt.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtpersrc -c -I../..  ../../rtpersrc/pe_2sCompBinInt64.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtpersrc -c -I../..  ../../rtpersrc/pe_NonNegBinInt.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtpersrc -c -I../..  ../../rtpersrc/pe_NonNegBinInt64.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtpersrc -c -I../..  ../../rtpersrc/pe_SmallNonNegWholeNumber.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtpersrc -c -I../..  ../../rtpersrc/pu_16BitCharSet.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtpersrc -c -I../..  ../../rtpersrc/pu_32BitCharSet.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtpersrc -c -I../..  ../../rtpersrc/pu_addSizeConstraint.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtpersrc -c -I../..  ../../rtpersrc/pu_alignCharStr.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtpersrc -c -I../..  ../../rtpersrc/pu_checkSize.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtpersrc -c -I../..  ../../rtpersrc/pu_common.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtpersrc -c -I../..  ../../rtpersrc/pu_deprecated.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtpersrc -c -I../..  ../../rtpersrc/pu_dump.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtpersrc -c -I../..  ../../rtpersrc/pu_field.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtpersrc -c -I../..  ../../rtpersrc/pu_insLenField.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtpersrc -c -I../..  ../../rtpersrc/pu_internal.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtpersrc -c -I../..  ../../rtpersrc/pu_setCharSet.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtpersrc -c -I../..  ../../rtpersrc/pu_setFldBitCount.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtpersrc -c -I../..  ../../rtpersrc/pu_setFldBitOffset.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtpersrc -c -I../..  ../../rtpersrc/pu_GetLibInfo.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtpersrc -c -I../..  ../../rtpersrc/perDecDecimal.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtpersrc -c -I../..  ../../rtpersrc/perDecReal10.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -I../../rtpersrc -c -I../..  ../../rtpersrc/perEncReal10.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -c -I../.. -I../../rtsrc -I../../rtxsrc  -o rtevalstubp.o -D_PER ../../rtsrc/rtevalstub.c
gcc -O2 -fPIC -shared -Wl,-soname,libasn1per.so -o ../lib/libasn1per.so pd_16BitConstrainedString.o pd_32BitConstrainedString.o pd_BigInteger.o pd_BigIntegerEx.o pd_BitString.o pd_BMPString.o pd_UniversalString.o pd_ChoiceOpenTypeExt.o pd_common.o pd_ConsWholeNumber64.o pd_ConsInteger.o pd_ConsInt8.o pd_ConsInt16.o pd_ConsInt64.o pd_ConstrainedString.o pd_ConsUnsigned.o pd_ConsUInt8.o pd_ConsUInt16.o pd_ConsUInt64.o pd_DynBitString.o pd_DynOctetString.o pd_extension.o pd_ObjectIdentifier.o pd_oid64.o pd_RelativeOID.o pd_OctetString.o pd_OpenType.o pd_OpenTypeExt.o pd_Real.o pd_SemiConsInteger.o pd_SemiConsUnsigned.o pd_SemiConsInt64.o pd_SemiConsUInt64.o pd_VarWidthCharString.o pd_deprecated.o pe_16BitConstrainedString.o pe_32BitConstrainedString.o pe_BigInteger.o pe_BitString.o pe_BMPString.o pe_UniversalString.o pe_ChoiceTypeExt.o pe_common.o pe_common64.o pe_ConsWholeNumber64.o pe_ConsInteger.o pe_ConsInt64.o pe_ConstrainedString.o pe_ConsUnsigned.o pe_ConsUInt64.o pe_identifier.o pe_extension.o pe_ObjectIdentifier.o pe_identifier64.o pe_oid64.o pe_RelativeOID.o pe_OctetString.o pe_OpenType.o pe_OpenTypeExt.o pe_Real.o pe_SemiConsInteger.o pe_SemiConsInt64.o pe_SemiConsUnsigned.o pe_SemiConsUInt64.o pe_VarWidthCharString.o pe_2sCompBinInt.o pe_2sCompBinInt64.o pe_NonNegBinInt.o pe_NonNegBinInt64.o pe_SmallNonNegWholeNumber.o pu_16BitCharSet.o pu_32BitCharSet.o pu_addSizeConstraint.o pu_alignCharStr.o pu_checkSize.o pu_common.o pu_deprecated.o pu_dump.o pu_field.o pu_insLenField.o pu_internal.o pu_setCharSet.o pu_setFldBitCount.o pu_setFldBitOffset.o pu_GetLibInfo.o perDecDecimal.o perDecReal10.o perEncReal10.o  rtevalstubp.o -lm

#
# libasn1xer
#
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared   -I../../rtxersrc  -c -I../..  ../../rtxersrc/xerCmpText.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared   -I../../rtxersrc  -c -I../..  ../../rtxersrc/xerContext.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared   -I../../rtxersrc  -c -I../..  ../../rtxersrc/xerCopyText.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared   -I../../rtxersrc  -c -I../..  ../../rtxersrc/xerDecBMPStr.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared   -I../../rtxersrc  -c -I../..  ../../rtxersrc/xerDecBase64Str.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared   -I../../rtxersrc  -c -I../..  ../../rtxersrc/xerDecBase64StrValue.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared   -I../../rtxersrc  -c -I../..  ../../rtxersrc/xerDecBigInt.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared   -I../../rtxersrc  -c -I../..  ../../rtxersrc/xerDecBinStrValue.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared   -I../../rtxersrc  -c -I../..  ../../rtxersrc/xerDecBitStr.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared   -I../../rtxersrc  -c -I../..  ../../rtxersrc/xerDecBitStrMemBuf.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared   -I../../rtxersrc  -c -I../..  ../../rtxersrc/xerDecBool.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared   -I../../rtxersrc  -c -I../..  ../../rtxersrc/xerDecCopyBitStr.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared   -I../../rtxersrc  -c -I../..  ../../rtxersrc/xerDecCopyDynBitStr.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared   -I../../rtxersrc  -c -I../..  ../../rtxersrc/xerDecCopyDynOctStr.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared   -I../../rtxersrc  -c -I../..  ../../rtxersrc/xerDecCopyOctStr.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared   -I../../rtxersrc  -c -I../..  ../../rtxersrc/xerDecDynAscCharStr.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared   -I../../rtxersrc  -c -I../..  ../../rtxersrc/xerDecDynBase64Str.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared   -I../../rtxersrc  -c -I../..  ../../rtxersrc/xerDecDynBitStr.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared   -I../../rtxersrc  -c -I../..  ../../rtxersrc/xerDecDynOctStr.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared   -I../../rtxersrc  -c -I../..  ../../rtxersrc/xerDecDynUTF8Str.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared   -I../../rtxersrc  -c -I../..  ../../rtxersrc/xerDecHexStrValue.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared   -I../../rtxersrc  -c -I../..  ../../rtxersrc/xerDecInt.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared   -I../../rtxersrc  -c -I../..  ../../rtxersrc/xerDecInt64.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared   -I../../rtxersrc  -c -I../..  ../../rtxersrc/xerDecObjId.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared   -I../../rtxersrc  -c -I../..  ../../rtxersrc/xerDecObjId64.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared   -I../../rtxersrc  -c -I../..  ../../rtxersrc/xerDecOctStr.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared   -I../../rtxersrc  -c -I../..  ../../rtxersrc/xerDecOctStrMemBuf.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared   -I../../rtxersrc  -c -I../..  ../../rtxersrc/xerDecOpenType.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared   -I../../rtxersrc  -c -I../..  ../../rtxersrc/xerDecReal.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared   -I../../rtxersrc  -c -I../..  ../../rtxersrc/xerDecRelativeOID.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared   -I../../rtxersrc  -c -I../..  ../../rtxersrc/xerDecUInt.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared   -I../../rtxersrc  -c -I../..  ../../rtxersrc/xerDecUInt64.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared   -I../../rtxersrc  -c -I../..  ../../rtxersrc/xerDecUnivStr.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared   -I../../rtxersrc  -c -I../..  ../../rtxersrc/xerEncAscCharStr.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared   -I../../rtxersrc  -c -I../..  ../../rtxersrc/xerEncBMPStr.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared   -I../../rtxersrc  -c -I../..  ../../rtxersrc/xerEncBase64Str.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared   -I../../rtxersrc  -c -I../..  ../../rtxersrc/xerEncBigInt.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared   -I../../rtxersrc  -c -I../..  ../../rtxersrc/xerEncBinStrValue.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared   -I../../rtxersrc  -c -I../..  ../../rtxersrc/xerEncBitStr.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared   -I../../rtxersrc  -c -I../..  ../../rtxersrc/xerEncBool.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared   -I../../rtxersrc  -c -I../..  ../../rtxersrc/xerEncChar.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared   -I../../rtxersrc  -c -I../..  ../../rtxersrc/xerEncEmptyElement.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared   -I../../rtxersrc  -c -I../..  ../../rtxersrc/xerEncEndDocument.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared   -I../../rtxersrc  -c -I../..  ../../rtxersrc/xerEncEndElement.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared   -I../../rtxersrc  -c -I../..  ../../rtxersrc/xerEncHexStrValue.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared   -I../../rtxersrc  -c -I../..  ../../rtxersrc/xerEncIndent.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared   -I../../rtxersrc  -c -I../..  ../../rtxersrc/xerEncInt.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared   -I../../rtxersrc  -c -I../..  ../../rtxersrc/xerEncInt64.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared   -I../../rtxersrc  -c -I../..  ../../rtxersrc/xerEncNamedValue.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared   -I../../rtxersrc  -c -I../..  ../../rtxersrc/xerEncNewLine.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared   -I../../rtxersrc  -c -I../..  ../../rtxersrc/xerEncNull.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared   -I../../rtxersrc  -c -I../..  ../../rtxersrc/xerEncObjId.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared   -I../../rtxersrc  -c -I../..  ../../rtxersrc/xerEncObjId64.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared   -I../../rtxersrc  -c -I../..  ../../rtxersrc/xerEncOctStr.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared   -I../../rtxersrc  -c -I../..  ../../rtxersrc/xerEncOpenType.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared   -I../../rtxersrc  -c -I../..  ../../rtxersrc/xerEncOpenTypeExt.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared   -I../../rtxersrc  -c -I../..  ../../rtxersrc/xerEncReal.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared   -I../../rtxersrc  -c -I../..  ../../rtxersrc/xerEncReal10.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared   -I../../rtxersrc  -c -I../..  ../../rtxersrc/xerEncRelativeOID.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared   -I../../rtxersrc  -c -I../..  ../../rtxersrc/xerEncStartDocument.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared   -I../../rtxersrc  -c -I../..  ../../rtxersrc/xerEncStartElement.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared   -I../../rtxersrc  -c -I../..  ../../rtxersrc/xerEncUInt.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared   -I../../rtxersrc  -c -I../..  ../../rtxersrc/xerEncUInt64.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared   -I../../rtxersrc  -c -I../..  ../../rtxersrc/xerEncUniCharData.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared   -I../../rtxersrc  -c -I../..  ../../rtxersrc/xerEncUniCharStr.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared   -I../../rtxersrc  -c -I../..  ../../rtxersrc/xerEncUnivStr.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared   -I../../rtxersrc  -c -I../..  ../../rtxersrc/xerEncXmlCharStr.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared   -I../../rtxersrc  -c -I../..  ../../rtxersrc/xerFinalizeMemBuf.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared   -I../../rtxersrc  -c -I../..  ../../rtxersrc/xerGetElemIdx.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared   -I../../rtxersrc  -c -I../..  ../../rtxersrc/xerGetLibInfo.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared   -I../../rtxersrc  -c -I../..  ../../rtxersrc/xerGetMsgPtrLen.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared   -I../../rtxersrc  -c -I../..  ../../rtxersrc/xerGetSeqElemIdx.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared   -I../../rtxersrc  -c -I../..  ../../rtxersrc/xerPutCharStr.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared   -I../../rtxersrc  -c -I../..  ../../rtxersrc/xerSetDecBufPtr.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared   -I../../rtxersrc  -c -I../..  ../../rtxersrc/xerSetEncBufPtr.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared   -I../../rtxersrc  -c -I../..  ../../rtxersrc/xerTextLength.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared   -I../../rtxersrc  -c -I../..  ../../rtxersrc/xerTextToCStr.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared   -I../../rtxersrc  -c -I../..  ../../rtxersrc/ASN1SAX_XEROpenType.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared -c -I../.. -I../../rtsrc -I../../rtxsrc  -o rtevalstubx.o -D_XER ../../rtsrc/rtevalstub.c
gcc -O2 -fPIC -shared -Wl,-soname,libasn1xer.so -o ../lib/libasn1xer.so xerCmpText.o xerContext.o xerCopyText.o xerDecBMPStr.o xerDecBase64Str.o xerDecBase64StrValue.o xerDecBigInt.o xerDecBinStrValue.o xerDecBitStr.o xerDecBitStrMemBuf.o xerDecBool.o xerDecCopyBitStr.o xerDecCopyDynBitStr.o xerDecCopyDynOctStr.o xerDecCopyOctStr.o xerDecDynAscCharStr.o xerDecDynBase64Str.o xerDecDynBitStr.o xerDecDynOctStr.o xerDecDynUTF8Str.o xerDecHexStrValue.o xerDecInt.o xerDecInt64.o xerDecObjId.o xerDecObjId64.o xerDecOctStr.o xerDecOctStrMemBuf.o xerDecOpenType.o xerDecReal.o xerDecRelativeOID.o xerDecUInt.o xerDecUInt64.o xerDecUnivStr.o xerEncAscCharStr.o xerEncBMPStr.o xerEncBase64Str.o xerEncBigInt.o xerEncBinStrValue.o xerEncBitStr.o xerEncBool.o xerEncChar.o xerEncEmptyElement.o xerEncEndDocument.o xerEncEndElement.o xerEncHexStrValue.o xerEncIndent.o xerEncInt.o xerEncInt64.o xerEncNamedValue.o xerEncNewLine.o xerEncNull.o xerEncObjId.o xerEncObjId64.o xerEncOctStr.o xerEncOpenType.o xerEncOpenTypeExt.o xerEncReal.o xerEncReal10.o xerEncRelativeOID.o xerEncStartDocument.o xerEncStartElement.o xerEncUInt.o xerEncUInt64.o xerEncUniCharData.o xerEncUniCharStr.o xerEncUnivStr.o xerEncXmlCharStr.o xerFinalizeMemBuf.o xerGetElemIdx.o xerGetLibInfo.o xerGetMsgPtrLen.o xerGetSeqElemIdx.o xerPutCharStr.o xerSetDecBufPtr.o xerSetEncBufPtr.o xerTextLength.o xerTextToCStr.o ASN1SAX_XEROpenType.o rtevalstubx.o -lm

#
# libasn1xml
#
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtSaxDiag.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtSaxErrors.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtSaxFindElemID.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtSaxFindElemID8.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtSaxGetAttributeID.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtSaxGetAttrValue.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtSaxGetElemID.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtSaxGetElemID8.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtSaxHasXMLNSAttrs.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtSaxIsEmptyBuffer.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtSaxLookupElemID.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtSaxLookupElemID8.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtSaxReportUnexpAttrs.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtSaxSetSkipLevelToCurrent.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtSaxTestAttributesPresent.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtXmlError.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtXmlContext.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtXmlContextInit.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtXmlCmpBase64Str.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtXmlCmpHexStr.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtXmlCmpQName.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtXmlDecBase64Binary.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtXmlDecBase64Common.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtXmlDecBase64StrValue.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtXmlDecBase64Str.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtXmlDecBigInt.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtXmlDecBool.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtXmlDecDateTime.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtXmlDecDates.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtXmlDecDecimal.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtXmlDecDouble.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtXmlDecDynBase64Str.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtXmlDecDynHexStr.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtXmlDecDynUTF8Str.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtXmlDecHexBinary.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtXmlDecHexStr.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtXmlDecHexStrValue.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtXmlDecInt.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtXmlDecInt64.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtXmlDecNSAttr.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtXmlDecQName.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtXmlDecUInt.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtXmlDecUInt64.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtXmlDecXSIAttr.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtXmlDecXSIAttrs.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtXmlEncAny.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtXmlEncAnyTypeValue.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtXmlEncAttr.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtXmlEncAttrC14N.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtXmlEncBase64Binary.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtXmlEncBase64BinaryAttr.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtXmlEncBigInt.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtXmlEncBigIntValue.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtXmlEncBigIntAttr.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtXmlEncBitString.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtXmlEncBool.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtXmlEncBoolAttr.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtXmlEncDates.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtXmlEncDatesValue.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtXmlEncDateTime.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtXmlEncDateTimeValue.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtXmlEncDecimal.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtXmlEncDecimalValue.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtXmlEncDecimalAttr.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtXmlEncDouble.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtXmlEncDoubleAttr.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtXmlEncDoublePattern.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtXmlEncDoubleValue.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtXmlEncEmptyElement.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtXmlEncEndDocument.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtXmlEncEndElement.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtXmlEncFloat.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtXmlEncFloatAttr.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtXmlEncHexBinary.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtXmlEncHexBinaryAttr.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtXmlEncIndent.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtXmlEncInt.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtXmlEncIntAttr.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtXmlEncIntPattern.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtXmlEncInt64.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtXmlEncInt64Attr.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtXmlEncNamedBits.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtXmlEncNSAttrs.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtXmlEncReal10.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtXmlEncSoapEnvelope.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtXmlEncSoapArrayTypeAttr.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtXmlEncStartDocument.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtXmlEncStartElement.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtXmlEncString.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtXmlEncUInt.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtXmlEncUInt64.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtXmlEncUIntAttr.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtXmlEncUInt64Attr.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtXmlEncUIntPattern.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtXmlEncUnicode.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtXmlEncXSIAttrs.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtXmlEncXSINilAttr.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtXmlEncXSITypeAttr.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtXmlKeyArray.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtXmlMatchHexStr.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtXmlMatchBase64Str.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtXmlMatchDateTime.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtXmlMatchDates.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtXmlMemFree.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtXmlNamespace.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtXmlNewQName.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtXmlParseElementName.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtXmlParseElemQName.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtXmlPrintNSAttrs.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtXmlPull.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtXmlSetEncBufPtr.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtXmlSetEncodingStr.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtXmlStrCmpAsc.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtXmlStrnCmpAsc.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtXmlTreatWhitespaces.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtXmlWriteToFile.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtXmlpCreateReader.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtXmlpDecAny.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtXmlpDecAnyElem.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtXmlpDecAnyAttrStr.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtXmlpDecBase64Str.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtXmlpDecBigInt.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtXmlpDecBitString.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtXmlpDecBool.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtXmlpDecDateTime.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtXmlpDecDecimal.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtXmlpDecDouble.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtXmlpDecDynBase64Str.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtXmlpDecDynHexStr.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtXmlpDecHexStr.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtXmlpDecInt.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtXmlpDecInt64.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtXmlpDecNamedBits.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtXmlpDecStrList.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtXmlpDecUInt.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtXmlpDecUInt64.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtXmlpDecUnicode.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtXmlpDecXmlStr.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtXmlpDecXSIAttr.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtXmlpDecXSITypeAttr.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtXmlpEvent.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtXmlpGetAttributeCount.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtXmlpGetAttributeID.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtXmlpGetContent.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtXmlpGetNextElem.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtXmlpGetNextElemID.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtXmlpHasAttributes.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtXmlpIsInGroup.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtXmlpIsUTF8Encoding.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtXmlpMatch.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtXmlpSelectAttribute.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtSaxCAny.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtSaxCAnyType.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtSaxCSimpleType.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtSaxCSoap.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtSaxCStrList.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtSaxCXmlHandler.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtSaxCXmlStreamHandler.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtSaxSortAttrs.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtXmlEncAnyAttr.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtAsn1XmlAddAnyAttr.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtAsn1XmlEncGenTime.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtAsn1XmlEncUTCTime.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtAsn1XmlEncOpenType.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtAsn1XmlEncObjId.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtAsn1XmlEncOpenTypeExt.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtAsn1XmlEncRelOID.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtAsn1XmlEncUnivStr.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtAsn1XmlFmtAttrStr.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtAsn1XmlParseAttrStr.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtAsn1XmlpDecObjId.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtAsn1XmlpDecUnivStr.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtAsn1XmlpDecDynBitStr.c
gcc -DASN1RT -DGNU -D_GNU_SOURCE -DHAVE_STDINT_H -DHAVE_VA_COPY -D_NO_LICENSE_CHECK -Wall -g -O2 -fPIC -shared  -I../../rtxmlsrc -I../../libxml2src/include -c -I../.. -I../../rtsrc -I../../rtxsrc  ../../rtxmlsrc/rtXmlpDecListOfASN1DynBitStr.c
gcc -O2 -fPIC -shared -Wl,-soname,libasn1xml.so -o ../lib/libasn1xml.so rtSaxDiag.o rtSaxErrors.o rtSaxFindElemID.o rtSaxFindElemID8.o rtSaxGetAttributeID.o rtSaxGetAttrValue.o rtSaxGetElemID.o rtSaxGetElemID8.o rtSaxHasXMLNSAttrs.o rtSaxIsEmptyBuffer.o rtSaxLookupElemID.o rtSaxLookupElemID8.o rtSaxReportUnexpAttrs.o rtSaxSetSkipLevelToCurrent.o rtSaxTestAttributesPresent.o rtXmlError.o rtXmlContext.o rtXmlContextInit.o rtXmlCmpBase64Str.o rtXmlCmpHexStr.o rtXmlCmpQName.o rtXmlDecBase64Binary.o rtXmlDecBase64Common.o rtXmlDecBase64StrValue.o rtXmlDecBase64Str.o rtXmlDecBigInt.o rtXmlDecBool.o rtXmlDecDateTime.o rtXmlDecDates.o rtXmlDecDecimal.o rtXmlDecDouble.o rtXmlDecDynBase64Str.o rtXmlDecDynHexStr.o rtXmlDecDynUTF8Str.o rtXmlDecHexBinary.o rtXmlDecHexStr.o rtXmlDecHexStrValue.o rtXmlDecInt.o rtXmlDecInt64.o rtXmlDecNSAttr.o rtXmlDecQName.o rtXmlDecUInt.o rtXmlDecUInt64.o rtXmlDecXSIAttr.o rtXmlDecXSIAttrs.o rtXmlEncAny.o rtXmlEncAnyTypeValue.o rtXmlEncAttr.o rtXmlEncAttrC14N.o rtXmlEncBase64Binary.o rtXmlEncBase64BinaryAttr.o rtXmlEncBigInt.o rtXmlEncBigIntValue.o rtXmlEncBigIntAttr.o rtXmlEncBitString.o rtXmlEncBool.o rtXmlEncBoolAttr.o rtXmlEncDates.o rtXmlEncDatesValue.o rtXmlEncDateTime.o rtXmlEncDateTimeValue.o rtXmlEncDecimal.o rtXmlEncDecimalValue.o rtXmlEncDecimalAttr.o rtXmlEncDouble.o rtXmlEncDoubleAttr.o rtXmlEncDoublePattern.o rtXmlEncDoubleValue.o rtXmlEncEmptyElement.o rtXmlEncEndDocument.o rtXmlEncEndElement.o rtXmlEncFloat.o rtXmlEncFloatAttr.o rtXmlEncHexBinary.o rtXmlEncHexBinaryAttr.o rtXmlEncIndent.o rtXmlEncInt.o rtXmlEncIntAttr.o rtXmlEncIntPattern.o rtXmlEncInt64.o rtXmlEncInt64Attr.o rtXmlEncNamedBits.o rtXmlEncNSAttrs.o rtXmlEncReal10.o rtXmlEncSoapEnvelope.o rtXmlEncSoapArrayTypeAttr.o rtXmlEncStartDocument.o rtXmlEncStartElement.o rtXmlEncString.o rtXmlEncUInt.o rtXmlEncUInt64.o rtXmlEncUIntAttr.o rtXmlEncUInt64Attr.o rtXmlEncUIntPattern.o rtXmlEncUnicode.o rtXmlEncXSIAttrs.o rtXmlEncXSINilAttr.o rtXmlEncXSITypeAttr.o rtXmlKeyArray.o rtXmlMatchHexStr.o rtXmlMatchBase64Str.o rtXmlMatchDateTime.o rtXmlMatchDates.o rtXmlMemFree.o rtXmlNamespace.o rtXmlNewQName.o rtXmlParseElementName.o rtXmlParseElemQName.o rtXmlPrintNSAttrs.o rtXmlPull.o rtXmlSetEncBufPtr.o rtXmlSetEncodingStr.o rtXmlStrCmpAsc.o rtXmlStrnCmpAsc.o rtXmlTreatWhitespaces.o rtXmlWriteToFile.o rtXmlpCreateReader.o rtXmlpDecAny.o rtXmlpDecAnyElem.o rtXmlpDecAnyAttrStr.o rtXmlpDecBase64Str.o rtXmlpDecBigInt.o rtXmlpDecBitString.o rtXmlpDecBool.o rtXmlpDecDateTime.o rtXmlpDecDecimal.o rtXmlpDecDouble.o rtXmlpDecDynBase64Str.o rtXmlpDecDynHexStr.o rtXmlpDecHexStr.o rtXmlpDecInt.o rtXmlpDecInt64.o rtXmlpDecNamedBits.o rtXmlpDecStrList.o rtXmlpDecUInt.o rtXmlpDecUInt64.o rtXmlpDecUnicode.o rtXmlpDecXmlStr.o rtXmlpDecXSIAttr.o rtXmlpDecXSITypeAttr.o rtXmlpEvent.o rtXmlpGetAttributeCount.o rtXmlpGetAttributeID.o rtXmlpGetContent.o rtXmlpGetNextElem.o rtXmlpGetNextElemID.o rtXmlpHasAttributes.o rtXmlpIsInGroup.o rtXmlpIsUTF8Encoding.o rtXmlpMatch.o rtXmlpSelectAttribute.o rtSaxCAny.o rtSaxCAnyType.o rtSaxCSimpleType.o rtSaxCSoap.o rtSaxCStrList.o rtSaxCXmlHandler.o rtSaxCXmlStreamHandler.o rtSaxSortAttrs.o rtXmlEncAnyAttr.o rtAsn1XmlAddAnyAttr.o rtAsn1XmlEncGenTime.o rtAsn1XmlEncUTCTime.o rtAsn1XmlEncOpenType.o rtAsn1XmlEncObjId.o rtAsn1XmlEncOpenTypeExt.o rtAsn1XmlEncRelOID.o rtAsn1XmlEncUnivStr.o rtAsn1XmlFmtAttrStr.o rtAsn1XmlParseAttrStr.o rtAsn1XmlpDecObjId.o rtAsn1XmlpDecUnivStr.o rtAsn1XmlpDecDynBitStr.o rtXmlpDecListOfASN1DynBitStr.o -lm

