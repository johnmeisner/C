/*===========================================================================
Copyright (c) 2017-2019 OnBoard Security, Inc.
Copyright (c) 2020 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.
=========================================================================== */

#ifndef AEROLINK_API_H
#define AEROLINK_API_H

#if defined(WIN32) && !defined(AEROLINK_EXPORTS)
    #define AEROLINK_EXPORT extern __declspec(dllimport)
#else
    #define AEROLINK_EXPORT extern
#endif

#endif /* AEROLINK_API_H */
