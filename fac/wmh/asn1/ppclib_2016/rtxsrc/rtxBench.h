/* Benchmark utility functions */

/**
 * @file rtxBench.h
 */
#ifndef _RTXBENCH_H_
#define _RTXBENCH_H_

#include <stdio.h>
#include <stdlib.h>
#ifndef _WIN32_WCE
#define __need_timespec
#include <time.h>
#ifndef _NUCLEUS
#include <sys/stat.h>
#endif
#else
#include "rtxsrc/wceAddon.h"
#endif
#include "rtxsrc/rtxDiag.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ITERCNT 10000L
   
typedef struct {
   clock_t timediff;
   double avgMSec;
} OSRTBenchmarkTimes;
   
typedef struct {
   const char* program;
   const char* encRules;
   size_t itercnt;
   size_t msgSize;
   unsigned long encodeMsecs;
   unsigned long decodeMsecs;
} OSRTBenchmarkData;
/*
#define __need_timespec
#include <time.h>
*/
#define DECLARE_BENCHMARK \
   struct timespec tp0, tp1, tp2, tp3; \
   long   __clk__ = 0; \
   double __maxms__ = 0, __minms__ = 10000000000000.f;

#define CLOCK_START \
   clock_gettime(CLOCK_REALTIME, &tp0);

#define CLOCK_STOP \
   clock_gettime(CLOCK_REALTIME, &tp1);

#define BEGIN_LOOP \
   CLOCK_START \
   for (__clk__ = 0; __clk__ < ITERCNT; __clk__ ++) { \
      clock_gettime(CLOCK_REALTIME, &tp2);

#define END_LOOP \
      clock_gettime(CLOCK_REALTIME, &tp3); \
      long _dst = tp3.tv_sec - tp2.tv_sec, \
           _dnt = tp3.tv_nsec - tp2.tv_nsec; \
      double _dmst = (_dst * 1000.f) + (_dnt / 1e6); \
      \
      if (_dmst < __minms__) __minms__ = _dmst; \
      if (_dmst > __maxms__) __maxms__ = _dmst; \
   } \
   CLOCK_STOP

#define PRINT_RESULTS_MS \
   long _ds = tp1.tv_sec - tp0.tv_sec, \
        _dn = tp1.tv_nsec - tp0.tv_nsec; \
   double _dms = (_ds * 1000.f) + (_dn / 1e6); \
   printf ("\t%.6f\t%.6f\t%.6f\t%.6f\n", _dms, __minms__, __maxms__, \
           _dms/(float)ITERCNT); 

/**
 * This function calculates the average number of milliseconds a test takes
 * given the starting time, finishing time, and number of iterations.
 *
 * @param   start    The initial start time, as a clock_t structure.
 * @param   finish   The final end time, as a clock_t structure.
 * @param   icnt     The iteration count.
 *
 * @return  The average time a single iteration takes, in milliseconds.
 */
double rtxBenchAverageMS (clock_t start, clock_t finish, double icnt);

void rtxBenchPrintResults
(const char* filename, const OSRTBenchmarkData* pdata);

#ifdef __cplusplus
}
#endif
#endif
