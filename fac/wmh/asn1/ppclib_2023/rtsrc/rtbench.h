/* Benchmark utility functions */

/**
 * @file rtbench.h
 */
#ifndef _BENCH_H_
#define _BENCH_H_

#include <stdio.h>
#include <stdlib.h>
#ifndef _WIN32_WCE
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
double rtBenchAverageMS (clock_t start, clock_t finish, double icnt);

#ifdef __cplusplus
}
#endif
#endif
