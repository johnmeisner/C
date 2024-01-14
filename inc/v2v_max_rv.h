/**************************************************************************
 *                                                                        *
 *     File Name:  v2v_max_rv.h                                           *
 *     Author:                                                            *
 *         DENSO International America, Inc.                              *
 *         North America Research Laboratory, California Office           *
 *         3252 Business Park Drive                                       *
 *         Vista, CA 92081                                                *
 *                                                                        *
 **************************************************************************
 *      CAMP - Vehicle Safety Communications 3 Consortium Proprietary     *
 **************************************************************************/

/* V2V_MAX_RV is defined here so that alsmi_struct.h can include it without
 * having to know a lot of detail about the other dependencies in
 * v2v_general.h. v2v_general.h also includes this file. */
#if defined(SCALABILITY)
#define V2V_MAX_RV (410)            /* For scalability alone */
#else
#define V2V_MAX_RV (100)
#endif

