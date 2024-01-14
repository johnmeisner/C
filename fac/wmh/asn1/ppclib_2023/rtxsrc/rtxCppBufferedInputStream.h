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
/**
 * @file rtxCppBufferedInputStream.h
 */
#ifndef _RTXCPPBUFFEREDINPUTSTREAM_H_
#define _RTXCPPBUFFEREDINPUTSTREAM_H_

#include "rtxsrc/OSRTInputStream.h"

/**
 * The buffered input stream class.
 */
class EXTRTCLASS OSBufferedInputStream : public OSRTInputStream {
 public:
   /**
    * The default constructor. It initializes a buffered stream.  A
    * buffered stream maintains data in memory before reading or writing
    * to the device.  This generally provides better performance than
    * an unbuffered stream.
    *
    * @exception OSStreamException    Stream create or initialize failed.
    */
   OSBufferedInputStream (OSRTInputStream& in);

   /**
    * Virtual destructor. Closes the stream if it was opened.
    */
   virtual ~OSBufferedInputStream ();

} ;

#endif /* _RTXCPPBUFFEREDINPUTSTREAM_H_ */
