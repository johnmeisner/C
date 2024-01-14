/*===========================================================================
Copyright (c) 2017-2019 OnBoard Security, Inc.
Copyright (c) 2020 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.
=========================================================================== */

#ifndef AEROLINK_REPORT_DATA_H
#define AEROLINK_REPORT_DATA_H

#include <stdint.h>
#include <stddef.h>


#ifdef __cplusplus
extern "C" {
#endif


// Report Data Id bitmap
typedef enum
{
    ARIB_CERTIFICATE_STATUS =1,
} AerolinkReportIdBitmap;

/***
 * @brief Callback function to be registered to receive error and/or status messages
 *        from the library.
 * @param id identifies, with a single bit set, the type of report being provided.
 * @param report should be cast as a pointer to the type representing the report data.
 * For example, if id is ARIB_CERTIFICATE_STATUS, report should be cast to a CertificateStatusReport const*.
 * @param userData contains the data passed in when the callback function was registered.
 */
typedef void (*AerolinkReportCallback)(
    AerolinkReportIdBitmap  id,
    void const             *report,
    void                   *userData);

/**
 * @brief Enumeration type of certificate status
 */
typedef enum
{
    CSB_VALID_CURRENT_CERTS =0x00000001,
    CSB_CERTS_AVAILABLE     =0x00000002,
    CSB_VALID_ECTL          =0x00000004,
    CSB_VALID_CRL           =0x00000008,
} CertificateStatusBitmap;

/**
 * @brief Data structure for certificate status
 * @param status contains one or more values of type CertificateStatusBitmap.
 * @param lcmDirLength is the number of bytes in lcmDir including the null terminator.
 * @param lcmDir contains the ascii character array of the full path of the LCM directory, including the null terminator.
 */
typedef struct
{
    uint32_t       status;
    uint32_t       lcmDirLength;
    uint8_t const *lcmDir;
} CertificateStatusReport;

#ifdef __cplusplus
}
#endif

#endif //AEROLINK_REPORT_DATA_H
