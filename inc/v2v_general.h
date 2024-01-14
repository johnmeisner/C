/**************************************************************************
 *                                                                        *
 *     File Name:  v2v_general.h                                          *
 *     Author:                                                            *
 *         DENSO International America, Inc.                              *
 *         North America Research Laboratory, California Office           *
 *         3252 Business Park Drive                                       *
 *         Vista, CA 92081                                                *
 *                                                                        *
 **************************************************************************
 *      CAMP - Vehicle Safety Communications 3 Consortium Proprietary     *
 **************************************************************************/

/* V2V General defines */

#ifndef QV2V_GENERAL_H
#define QV2V_GENERAL_H
#define V2V_GENERAL_H

#include <sys/types.h>
#include "dn_types.h"

/* For BuildInfo.txt. Move to i2v_general.h. */
#define I2V_VERSION "HD-RSU"

#define PACKED         __attribute__((packed))

#define SQUARE(x) ((x)*(x))       /* SQUARE function to replace use of pow(x,2) - On QNX, pow(x,2) is extremely expensive for values of x <= 0.00402221939539754159 */

#ifndef NUMITEMS
#define NUMITEMS(x) (sizeof(x)/sizeof((x)[0]))
#endif
#define DWMH_LOG_STRING_APPEND_SIZE (18)
#define K_LOG_PREFIX_SIZE (5)
#define KPH_MPS_FACTOR          (0.277777777777778)  /* convert from KPH to meters/s */
#define MEAN_RADIUS_METERS      (6378137)
#define e2                      (0.00669437999014)   /* First eccentricity squared; for ECEF Calculation */
#define V2V_INFINITY            (99999.89999999999)
#define G_MPSS                  (9.80665)            /* Gravitational force constant near the surface of the Earth, g = 9.81 m/s^2 */
#ifndef M_PI
#define M_PI                    (3.14159265358979323846)
#endif

#define wsu_min(a,b)            (((a)<(b))?(a):(b))
#define wsu_max(a,b)            (((a)>(b))?(a):(b))
#define sign(a)                 (((a)<0)?-1:1)
#define RANGE(x,y)              (sqrt(((x)*(x))+((y)*(y))))
#define RAD_TO_DEG(a)           ((a)*180/M_PI)
#define DEG_TO_RAD(a)           ((a)*M_PI/180)
#define KNOTS_TO_MPH(a)         ((a)*1.150779768)
#define KNOTS_TO_KMH(a)         ((a)*1.852)
#define KPH_TO_MPS(a)           ((a)*.2777777777)
#define MPH_TO_MPS(a)           ((a)*0.44704)
#define MPS_TO_MPH(a)           ((a)*2.2369362920544025)
#define CMPS_TO_MPH(a)          ((a)*0.022369362920544025)
#define MPS_TO_KPH(a)           ((a)*3.6)
#define CMPS_TO_KPH(a)          ((a)*0.036)
#define MS2_TO_G(a)             ((a)/9.806650)
#define G_TO_MS2(a)             (9.806650*(a))
#define KPH_MPS(a)              ((a)*.2777777777)
#define MPS_KPH(a)              ((a)*3.6)
#define REMOTE_AZIMUTH_DEG(y,x) (RAD_TO_DEG(atan2((y),(x))))
#define MPH_TO_KPH(a)           (1.60935*(a))
#define KPH_TO_MPH(a)           (0.62137*(a))
#define METERS_TO_FEET(a)       ((a)*3.281)
#define MS_TO_S(a)              ((a)/1000)
#define S_TO_MS(a)              ((a)*1000)
#define MS_TO_NS(a)             ((a)*1000000)
#define BAR_TO_PSI(a)           ((a)*14.5037738)
#define METERS_TO_MILES(a)      ((a)*0.000621371)
#define MILES_TO_METERS(a)      ((a)*1609.34)
#define KM_TO_METERS(a)         ((a)*1000)

#endif
