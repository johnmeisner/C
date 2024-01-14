/*
 * Copyright (c) 1997-2018 Objective Systems, Inc.
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
 * @file asn1OerCppTypes.h
 * OER C++ type and class definitions.
 */
#ifndef _ASN1OERCPPTYPES_H_
#define _ASN1OERCPPTYPES_H_

#include "rtoersrc/asn1oer.h"
#include "rtsrc/asn1CppTypes.h"
#include "rtxsrc/rtxBitEncode.h"
#include "rtxsrc/rtxHexDump.h"

#if defined(_NO_STREAM) && !defined(_NO_CPP_STREAM)
#define _NO_CPP_STREAM
#endif

#if !defined(_NO_CPP_STREAM)
class OSRTStream;
class OSRTInputStream;
class OSRTOutputStream;
#endif

/** @defgroup oercppruntime OER C++ Runtime Classes.
 * @{
 */
/**
 * The ASN.1 C++ runtime classes are wrapper classes that provide an
 * object-oriented interface to the ASN.1 C runtime library functions. These
 * classes are derived from the common classes documented in the ASN1C C/C++
 * Common Runtime Functions manual and are specific the Octet Encoding Rules
 * (OER).
*/

/** @defgroup oermsgbuffclas OER Message Buffer Classes
 * These classes manage the buffers for encoding and decoding ASN.1 OER
 * messages. @{
*/

// ASN.1 OER message buffer class
/**
 * The ASN1OERMessageBuffer class is derived from the ASN1MessageBuffer base
 * class. It is the base class for the ASN1OEREncodeBuffer and
 * ASN1OERDecodeBuffer derived classes. It contains variables and methods
 * specific to encoding or decoding ASN.1 messages using the Octet Encoding
 * Rules (OER). It is used to manage the buffer into which an ASN.1 message is
 * to be encoded or decoded.
*/
class EXTOERCLASS ASN1OERMessageBuffer : public ASN1MessageBuffer {
 protected:

   /**
    * This constructor does not set a OER input source.  It is used by
    * the derived encode buffer classes.  Use the getStatus() method to
    * determine if an error has occured during initialization.
    *
    * @param bufferType   Type of message buffer that is being created (for
    *                       example, OEREncode or OERDecode).
    */
   EXTOERMETHOD ASN1OERMessageBuffer (Type bufferType);

#if !defined(_NO_CPP_STREAM)
   /**
    * This constructor associates a stream with a OER encode or decode
    * buffer.  It is used by the derived encode buffer classes to create
    * a stream-based OER encoder or decoder.
    *
    * @param stream Stream class reference.
    */
   EXTOERMETHOD ASN1OERMessageBuffer (OSRTStream& stream);
#endif
   /**
    * This constructor allows a memory buffer holding a binary OER message
    * to be specified. Use the getStatus() method to determine if an error
    * has occured during initialization.
    *
    * @param bufferType   Type of message buffer that is being created (for
    *                       example, OEREncode or OERDecode).
    * @param pMsgBuf      A pointer to a fixed size message buffer to recieve
    *                       the encoded message.
    * @param msgBufLen    Size of the fixed-size message buffer.
    */
   EXTOERMETHOD ASN1OERMessageBuffer
      (Type bufferType, OSOCTET* pMsgBuf, size_t msgBufLen);

   /**
    * This constructor allows a memory buffer holding a binary OER message
    * to be specified. It also allows a pre-existing context to be associated
    * with this buffer. Use the getStatus() method to determine if an error
    * has occured during initialization.
    *
    * @param bufferType   Type of message buffer that is being created (for
    *                       example, OEREncode or OERDecode).
    * @param pMsgBuf      A pointer to a fixed size message buffer to recieve
    *                       the encoded message.
    * @param msgBufLen    Size of the fixed-size message buffer.
    * @param pContext     A pointer to an OSRTContext structure.
    */
   EXTOERMETHOD ASN1OERMessageBuffer
      (Type bufferType, OSOCTET* pMsgBuf, size_t msgBufLen,
       OSRTContext *pContext);

 public:

   /**
    * This method outputs a hexadecimal dump of the current buffer contents to
    * stdout.
    *
    * @param             - none
    */
   inline void hexDump () {
      rtxHexDump (getMsgPtr(), getMsgLen());
   }

   /**
    * This method returns the length of a previously encoded PER message.
    *
    * @param             - none
    */
   virtual size_t getMsgLen () {
      return rtxCtxtGetMsgLen(getCtxtPtr());
   }

   /**
    * This method sets a buffer to receive the encoded message.
    *
    * @param pMsgBuf      A pointer to a memory buffer to use to encode a
    *                       message. The buffer should be declared as an array
    *                       of unsigned characters (OSOCTETs). This parameter
    *                       can be set to NULL to specify dynamic encoding
    *                       (i.e., the encode functions will dynamically
    *                       allocate a buffer for the message).
    * @param msgBufLen    The length of the memory buffer in bytes. If pMsgBuf
    *                       is NULL, this parameter specifies the initial size
    *                       of the dynamic buffer; if 0 - the default size will
    *                       be used.
    * @return             Completion status of operation:
    *                       - 0 (0) = success,
    *                       - negative return value is error.
    */
   EXTOERMETHOD int setBuffer (const OSOCTET* pMsgBuf, size_t msgBufLen);

   // deprecated methods
   //inline void BinDump (const char* varname) { binDump (varname); }
   //inline void HexDump () { hexDump (); }
   //inline int GetMsgLen () { return (int)getMsgLen (); }
   //inline void SetTrace (OSBOOL value) { setTrace (value); }
} ;

// ASN.1 OER encode message buffer class
/**
 * The ASN1OEREncodeBuffer class is derived from the ASN1OERMessageBuffer base
 * class. It contains variables and methods specific to encoding ASN.1
 * messages. It is used to manage the buffer into which an ASN.1 OER message
 * is to be encoded.
 */
class EXTOERCLASS ASN1OEREncodeBuffer : public ASN1OERMessageBuffer {
 public:
   /**
    * This version of the ASN1OEREncodeBuffer constructor creates a dynamic
    * memory buffer into which an OER message is encoded.
    */
   ASN1OEREncodeBuffer () :
      ASN1OERMessageBuffer (OEREncode) {}

   /**
    * This version of the ASN1OEREncodeBuffer constructor takes a message
    * buffer and size argument (static encoding version).
    *
    * @param pMsgBuf      A pointer to a fixed-size message buffer to receive
    *                       the encoded message.
    * @param msgBufLen    Size of the fixed-size message buffer.
    */
   ASN1OEREncodeBuffer (OSOCTET* pMsgBuf, size_t msgBufLen)
      : ASN1OERMessageBuffer (OEREncode, pMsgBuf, msgBufLen) {}


   /**
    * This version of the ASN1OEREncodeBuffer constructor takes a message
    * buffer and size argument (static encoding version) as well as a pointer to
    * an existing context object.
    *
    * @param pMsgBuf      A pointer to a fixed-size message buffer to receive
    *                       the encoded message.
    * @param msgBufLen    Size of the fixed-size message buffer.
    * @param pContext     A pointer to an OSRTContext structure created by
    *                         the user.
    */
   ASN1OEREncodeBuffer (OSOCTET *pMsgBuf, size_t msgBufLen,
      OSRTContext *pContext) : ASN1OERMessageBuffer (OEREncode, pMsgBuf,
         msgBufLen, pContext) { }

#if !defined(_NO_CPP_STREAM)
   /**
    * This version of the ASN1OEREncodeBuffer constructor takes a
    * reference to an output stream object (stream encoding version).
    *
    * @param ostream      A reference to an output stream object.
    */
   EXTOERMETHOD ASN1OEREncodeBuffer
      (OSRTOutputStream& ostream);
#endif

   /**
    * This method writes a single encoded bit value to the output buffer
    * or stream.
    *
    * @param value      Boolean value of bit to be written.
    * @return           Status of operation: 0 = success, negative
    *                     value if error occurred.
    */
   inline int encodeBit (OSBOOL value) {
      return rtxEncBit (getCtxtPtr(), value);
   }

   /**
    * This method writes the given number of bits from the byte array
    * to the output buffer or stream starting from the given bit offset.
    *
    * @param pvalue     Pointer to byte array containing data to
    *                     be encoded.
    * @param nbits      Number of bits to copy from byte array to
    *                     encode buffer.
    * @param bitOffset  Starting bit offset from which bits are to be copied.
    * @return           Status of operation: 0 = success, negative
    *                     value if error occurred.
    */
   inline int encodeBits
      (const OSOCTET* pvalue, size_t nbits, OSUINT32 bitOffset = 0) {
      return (0 == bitOffset) ?
         rtxEncBitsFromByteArray (getCtxtPtr(), pvalue, nbits) :
         rtxCopyBits (getCtxtPtr(), pvalue, nbits, bitOffset);
   }


   /**
    * This method returns a copy of the current encoded message. Memory is
    * allocated for the message using the 'new' operation. It is the user's
    * responsibility to free the memory using 'delete'.
    *
    * @return             Pointer to copy of encoded message. It is the user's
    *                       responsibility to release the memory using the
    *                       'delete' operator (i.e., delete [] ptr;)
    */
   virtual EXTOERMETHOD OSOCTET* getMsgCopy ();

   /**
    * This method returns the internal pointer to the current encoded message.
    *
    * @return             Pointer to encoded message.
    */
   virtual EXTOERMETHOD const OSOCTET* getMsgPtr ();

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
   EXTOERMETHOD int init ();

   /**
    * This method checks the type of the message buffer.
    *
    * @param bufferType   Enumerated identifier specifying a derived class. The
    *                       only possible value for this class is OEREncode.
    * @return             Boolean result of the match operation. True if this
    *                       is the class corresponding to the identifier
    *                       argument.
    */
   virtual OSBOOL isA (Type bufferType) {
      return bufferType == OEREncode;
   }

   /**
    * This method is used to write the given number of bytes from
    * the given buffer to the encode buffer or stream.
    *
    * @param buffer       Buffer from which data should be read.
    * @param nbytes       Number of bytes to write.
    * @return             Completion status of operation:
    *                       - 0 (0) = success,
    *                       - negative return value is error.
    */
   EXTOERMETHOD int writeBytes (const OSOCTET* buffer, size_t nbytes);

} ;

// ASN.1 OER decode message buffer class
/**
 * The ASN1OERDecodeBuffer class is derived from the ASN1OERMessageBuffer base
 * class. It contains variables and methods specific to decoding ASN.1 OER
 * messages. It is used to manage the input buffer containing the ASN.1 message
 * to be decoded. This class has 3 overloaded constructors.
 *
 */
class EXTOERCLASS ASN1OERDecodeBuffer : public ASN1OERMessageBuffer {
 public:
   /**
    * This is a default constructor. Use getStatus() method to determine has
    * error occured during the initialization or not.
    */
   ASN1OERDecodeBuffer () :
      ASN1OERMessageBuffer (OERDecode)
      {}

   /**
    * This constructor is used to describe the message to be decoded. Use
    * getStatus() method to determine has error occured during the
    * initialization or not.
    *
    * @param pMsgBuf      A pointer to the message to be decoded.
    * @param msgBufLen    Length of the message buffer.
    */
   ASN1OERDecodeBuffer (const OSOCTET* pMsgBuf,
                        size_t msgBufLen) :
      ASN1OERMessageBuffer (OERDecode, (OSOCTET*)pMsgBuf, msgBufLen)
      {}

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
   ASN1OERDecodeBuffer (const OSOCTET* pMsgBuf,
                        size_t msgBufLen, OSRTContext *pContext) :
      ASN1OERMessageBuffer
         (OERDecode, (OSOCTET*)pMsgBuf, msgBufLen, pContext) {}

#if !defined(_NO_CPP_STREAM)
   /**
    * This version of the ASN1OERDecodeBuffer constructor takes a
    * reference to an input stream object (stream decoding version).
    *
    * @param istream      A reference to an input stream object.
    */
   EXTOERMETHOD ASN1OERDecodeBuffer (OSRTInputStream& istream);
#endif
   /**
    * This constructor takes a pointer to the path of a file containing
    * a binary OER message to be decoded.
    *
    * @param filePath     Complete file path and name of file to read.
    */
   EXTOERMETHOD ASN1OERDecodeBuffer (const char* filePath);


   /**
    * This method checks the type of the message buffer.
    *
    * @param bufferType   Enumerated identifier specifying a derived class. The
    *                       only possible value for this class is OERDecode.
    * @return             Boolean result of the match operation. True if this
    *                       is the class corresponding to the identifier
    *                       argument.
    */
   virtual OSBOOL isA (Type bufferType) {
      return bufferType == OERDecode;
   }

   /**
    * This method is used to peek at the next available byte in the
    * decode buffer/stream without advancing the cursor.
    *
    * @param ub         Single byte buffer to receive peeked byte.
    * @return             Completion status of operation:
    *                       - 0 (0) = success,
    *                       - negative return value is error.
    */
   EXTOERMETHOD int peekByte (OSOCTET& ub);

   /**
    * This method reads the file into the buffer to decode.
    *
    * @param filePath     The zero-terminated string containing the path to the
    *                       file.
    * @return             Completion status of operation:
    *                       - 0 (0) = success,
    *                       - negative return value is error.
    */
   EXTOERMETHOD int readBinaryFile (const char* filePath);

   /**
    * This method is used to read the given number of bytes from
    * the underlying buffer/stream into the given buffer.
    *
    * @param buffer       Buffer into which data should be read.
    * @param bufsize      Size of the buffer
    * @param nbytes       Number of bytes to read.  Must be <= bufsize.
    * @return             Completion status of operation:
    *                       - 0 (0) = success,
    *                       - negative return value is error.
    */
   EXTOERMETHOD int readBytes (OSOCTET* buffer, size_t bufsize, size_t nbytes);

} ;


/** @}oermsgbuffclas

*/
/** @}oercppruntime

 */
#endif
