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
/**
 * @file asn1AvnCppTypes.h
 * AVN C++ type and class definitions.
 */
#ifndef _ASN1AVNCPPTYPES_H_
#define _ASN1AVNCPPTYPES_H_

#include "rtxsrc/rtxBuffer.h"
#include "rtsrc/rtAvn.h"
#include "rtsrc/asn1CppTypes.h"


#if defined(_NO_STREAM) && !defined(_NO_CPP_STREAM)
#define _NO_CPP_STREAM
#endif

#if !defined(_NO_CPP_STREAM)
class OSRTStream;
class OSRTInputStream;
class OSRTOutputStream;
#endif

/** @defgroup percppruntime AVN C++ Runtime Classes.
 * @{
 */
 /**
  * The ASN.1 C++ runtime classes are wrapper classes that provide an
  * object-oriented interface to the ASN.1 C runtime library functions. These
  * classes are derived from the common classes documented in the ASN1C C/C++
  * Common Runtime Functions manual and are specific to Abstract Value Notation
  * (AVN).
 */

 /** @defgroup permsgbuffclas AVN Message Buffer Classes
  * These classes manage the buffers for encoding and decoding ASN.1 values
  * represented in Abstract Value Notation (AVN).
  * @{
 */


 /**
  * The ASN1AVNEncodeBuffer contains variables and methods specific to
  * encoding ASN.1 to Abstract Value Notation (AVN).  It is used to manage the
  * buffer into which an ASN.1 AVN message is to be encoded.
  */
class EXTRTCLASS ASN1AVNEncodeBuffer : public ASN1MessageBuffer {

public:
   /**
    * This version of the ASN1AVNEncodeBuffer constructor creates a dynamic
    * memory buffer into which an AVN message is encoded.
    */
   ASN1AVNEncodeBuffer() :
      ASN1MessageBuffer(AVNEncode) {}

   /**
    * This version of the ASN1AVNEncodeBuffer constructor takes a message
    * buffer and size argument (static encoding version).
    *
    * @param pMsgBuf      A pointer to a fixed-size message buffer to receive
    *                       the encoded message.
    * @param msgBufLen    Size of the fixed-size message buffer.
    */
   ASN1AVNEncodeBuffer(OSOCTET* pMsgBuf, size_t msgBufLen);


   /**
    * This version of the ASN1AVNEncodeBuffer constructor takes a message
    * buffer and size argument (static encoding version) as well as a pointer to
    * an existing context object.
    *
    * @param pMsgBuf      A pointer to a fixed-size message buffer to receive
    *                       the encoded message.
    * @param msgBufLen    Size of the fixed-size message buffer.
    * @param pContext     A pointer to an OSRTContext structure created by
    *                         the user.
    */
   ASN1AVNEncodeBuffer(OSOCTET *pMsgBuf, size_t msgBufLen,
                        OSRTContext *pContext);


#if !defined(_NO_CPP_STREAM)
   /**
    * This version of the ASN1AVNEncodeBuffer constructor takes a
    * reference to an output stream object (stream encoding version).
    *
    * @param ostream      A reference to an output stream object.
    */
   ASN1AVNEncodeBuffer(OSRTOutputStream& ostream);
#endif

   virtual ~ASN1AVNEncodeBuffer();


   /**
    * This method returns the length of a previously encoded AVN message.
    *
    * @return Length of the AVN message encapsulated within this
    * buffer object.
    */
   virtual size_t getMsgLen() {
      return getByteIndex();
   }


   /**
    * This method reinitializes the encode buffer pointer to allow a new
    * message to be encoded. This makes it possible to reuse one message buffer
    * object in a loop to encode multiple messages. After this method is
    * called, any previously encoded message in the buffer will be overwritten
    * on the next encode call.
    *
    * @return             Completion status of operation:
    *                       - 0 (0) = success,
    *                       - negative return value is error.
    */
   EXTRTMETHOD int init();

   /**
    * This method checks the type of the message buffer.
    *
    * @param bufferType   Enumerated identifier specifying a derived class.
    * @return             Boolean result of the match operation. True if this
    *                       is the class corresponding to the identifier
    *                       argument.
    */
   virtual OSBOOL isA(Type bufferType) {
      return bufferType == AVNEncode;
   }

   /**
    * This method adds a null-terminator character ('\0') at the current
    * buffer position.
    * @return 0 (success) or error code.
    */
   inline int nullTerminate() {
      OSOCTET c = 0;
      return rtxWriteBytes(getCtxtPtr(), &c, 1);
   }

   /**
    * This method writes the encoded message to the given file.
    *
    * @param  filename  The name of file to which the encoded message will
    *                   be written.
    * @return           Number of octets actually written.  This value may be
    *                   less than the actual message length if an error
    *                   occurs.
    *
    */
   EXTRTMETHOD long write(const char* filename);

   /**
    * This version of the write method writes to a file that is specified
    * by a FILE pointer.
    *
    * @param  fp       Pointer to FILE structure to which the encoded
    *                  message will be written.
    * @return           Number of octets actually written.  This value may be
    *                   less than the actual message length if an error
    *                   occurs.
    *
    */
   EXTRTMETHOD long write(FILE* fp);

};



/**
 * The ASN1AVNDecodeBuffer class contains variables and methods specific to
 * decoding ASN.1 AVN messages. It is used to manage the input buffer containing
 * the ASN.1 message to be decoded.
 *
 */
class EXTRTCLASS ASN1AVNDecodeBuffer : public ASN1MessageBuffer {
protected:
   /**
    * Input source for message to be decoded.  If not null, this object owns
    * it and will delete it upon destruction.
    */
   OSRTInputStream* mpInputStream;

public:
   /**
    * This is a default constructor. Use getStatus() method to determine has
    * error occured during the initialization or not.
    */
   ASN1AVNDecodeBuffer();

   /**
    * This constructor is used to describe the message to be decoded. Use
    * getStatus() method to determine has error occured during the
    * initialization or not.
    *
    * @param pMsgBuf      A pointer to the message to be decoded.
    * @param msgBufLen    Length of the message buffer.
    */
   ASN1AVNDecodeBuffer (const OSOCTET* pMsgBuf, size_t msgBufLen);

   /**
    * This constructor is used to describe the message to be decoded. Use
    * getStatus() method to determine has error occured during the
    * initialization or not.
    *
    * @param pMsgBuf      A pointer to the message to be decoded.
    * @param msgBufLen    Length of the message buffer.
    * @param pContext     A pointer to an OSRTContext structure created by
    *                       the user.
    */
   ASN1AVNDecodeBuffer(const OSOCTET* pMsgBuf, size_t msgBufLen,
                        OSRTContext *pContext);

#if !defined(_NO_CPP_STREAM)
   /**
    * This version of the ASN1AVNDecodeBuffer constructor takes a
    * reference to an input stream object (stream decoding version).
    *
    * @param istream      A reference to an input stream object.
    */
   EXTRTMETHOD ASN1AVNDecodeBuffer(OSRTInputStream& istream);
#endif

   /**
    * This constructor takes a pointer to the path of a file containing
    * a binary PER message to be decoded.
    *
    * @param filePath     Complete file path and name of file to read.
    */
   EXTRTMETHOD ASN1AVNDecodeBuffer(const char* filePath);

   virtual ~ASN1AVNDecodeBuffer();


   /**
    * This method checks the type of the message buffer.
    *
    * @param bufferType   Enumerated identifier specifying a derived class.
    * @return             Boolean result of the match operation. True if this
    *                       is the class corresponding to the identifier
    *                       argument.
    */
   virtual OSBOOL isA(Type bufferType) {
      return bufferType == AVNDecode;
   }

};
#endif
