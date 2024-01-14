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

#include "rtsrc/ASN1CBitStr.h"
#include "rtsrc/asn1intl.h"
#include "rtxsrc/rtxUtil.h"
#include <limits.h>

// delta (in units) for adding to size of memory being allocating
#define ADD_DELTA 1024

#define UNIT_MASK ((OSOCTET)-1)

#define ADDRESS_BITS       3 // as char has 8 bits (2^3)

#define BITS_PER_UNIT (OSINTCONST(1) << ADDRESS_BITS)
#define BIT_INDEX_MASK  (BITS_PER_UNIT - 1)

//////////////////////////////////////////////////////////////////////
//
// ASN1CBitStr
//
// Bit string implementation. This class provides all functionalty
// necessary for bit strings.
//
/*
* Returns a unit that has all bits that are less significant
* than the specified index set to 1. All other bits are 0.
*/
inline static OSOCTET bitsRightOf(int x) {
   return (UNIT_MASK >> x);
}

/*
* Returns a unit that has all the bits that are more significant
* than or equal to the specified index set to 1. All other bits are 0.
* @param x The bit index, a value 0..7.
*/
inline static OSOCTET bitsLeftOf(OSSIZE x) {
   return (x==0 ? UNIT_MASK : (UNIT_MASK << (BITS_PER_UNIT - x)));
}

/*
 * Given a bit index (0..) return unit (byte) index (0..) containing it.
 */
inline static OSSIZE unitIndex (OSSIZE bitIndex) {
   return bitIndex >> ADDRESS_BITS;
}

/* Return unit index for a given bit count */
inline static OSSIZE unitIndexNbits (OSSIZE nbits) {
   return (nbits > 0) ? (unitIndex (nbits - 1) + 1) : 0;
}
/*
 * Given a bit index, return a unit that masks that bit in its unit.
 */
inline static long bit(OSSIZE bitIndex) {
   return (OSINTCONST(1) << (BITS_PER_UNIT - 1)) >> (bitIndex & BIT_INDEX_MASK);
}

/*
* Creates an empty bits string. The string should be additionaly initialized
* by call to 'init' method.
*/
EXTRTMETHOD ASN1CBitStr::ASN1CBitStr(OSRTMessageBufferIF& msgBuf) :
   ASN1CType(msgBuf), _numbits (0), _units (0), mpUnits(&_units),
   mppExtData(0), mMaxNumBits (0), mUnitsUsed (0), mUnitsAllocated (0),
   mDynAlloc (FALSE)
{
   mpNumBits = new ASN1CBitStrSizeHolder64 (_numbits);
}

/*
 * Default protected ctor. mpContext should be set and the string should be
 * additionaly initialized by call to 'init' method.
 */
EXTRTMETHOD ASN1CBitStr::ASN1CBitStr() :
   ASN1CType(), _numbits (0), _units (0), mpUnits(&_units), mppExtData(0),
   mMaxNumBits(0), mUnitsUsed (0), mUnitsAllocated (0), mDynAlloc (FALSE)
{
   mpNumBits = new ASN1CBitStrSizeHolder64 (_numbits);
}

/*
 * Creates an empty bit string. The string should be additionaly initialized
 * by call to 'init' method.
 */
EXTRTMETHOD ASN1CBitStr::ASN1CBitStr(OSRTContext& ctxt) :
   ASN1CType(ctxt), _numbits (0), _units (0), mpUnits(&_units), mppExtData(0),
   mMaxNumBits(0), mUnitsUsed (0), mUnitsAllocated (0), mDynAlloc (FALSE)
{
   mpNumBits = new ASN1CBitStrSizeHolder64 (_numbits);
}

#if SIZE_MAX > OSUINT32_MAX        //if size_t > 32 bits
EXTRTMETHOD ASN1CBitStr::ASN1CBitStr
(OSOCTET* bitStr, OSSIZE& octsNumbits, OSSIZE maxNumbits_,
 OSOCTET** ppExtData): ASN1CType(), mpUnits(&_units)
{
   mpNumBits = new ASN1CBitStrSizeHolder64 (octsNumbits);
   privateInit (bitStr, maxNumbits_, ppExtData);
}
#endif

EXTRTMETHOD ASN1CBitStr::ASN1CBitStr
(OSOCTET* bitStr, OSUINT32& octsNumbits, OSSIZE maxNumbits_,
 OSOCTET** ppExtData): ASN1CType(), mpUnits(&_units)
{
   mpNumBits = new ASN1CBitStrSizeHolder32 (octsNumbits);
   privateInit (bitStr, maxNumbits_, ppExtData);
}

EXTRTMETHOD ASN1CBitStr::ASN1CBitStr
(OSOCTET* bitStr, OSUINT16& octsNumbits, OSSIZE maxNumbits_,
 OSOCTET** ppExtData): ASN1CType(), mpUnits(&_units)
{
   mpNumBits = new ASN1CBitStrSizeHolder16 (octsNumbits);
   privateInit (bitStr, maxNumbits_, ppExtData);
}

EXTRTMETHOD ASN1CBitStr::ASN1CBitStr
(OSOCTET* bitStr, OSUINT8& octsNumbits, OSSIZE maxNumbits_,
 OSOCTET** ppExtData): ASN1CType(), mpUnits(&_units)
{
   mpNumBits = new ASN1CBitStrSizeHolder8 (octsNumbits);
   privateInit (bitStr, maxNumbits_, ppExtData);
}

EXTRTMETHOD ASN1CBitStr::ASN1CBitStr (OSOCTET* pBits, OSSIZE maxNumbits):
   ASN1CType(), mpUnits(&_units)
{
   mpNumBits = 0;
   privateInit (pBits, maxNumbits, 0);
}

/*
* Creates bits string from ASN1TDynBitStr. Ctor does not copy bytes,
* just assign array (data member of ASN1TDynBitStr) to units.
* This string will be extendable.
*/
EXTRTMETHOD ASN1CBitStr::ASN1CBitStr (ASN1TDynBitStr& bitStr):
   ASN1CType(), mpUnits((OSOCTET**)&bitStr.data)
{
   mpNumBits = new ASN1CBitStrSizeHolder32 (bitStr.numbits);
   privateInit (bitStr);
}

EXTRTMETHOD ASN1CBitStr::ASN1CBitStr (ASN1TDynBitStr64& bitStr):
   ASN1CType(), mpUnits((OSOCTET**)&bitStr.data)
{
   mpNumBits = new ASN1CBitStrSizeHolder64 (bitStr.numbits);
   privateInit (bitStr);
}

void ASN1CBitStr::initBase
(OSOCTET* pBits, OSSIZE numbits, OSSIZE maxNumbits)
{
   (*mpUnits) = pBits;
   delete mpNumBits;
   mpNumBits = 0;
   mMaxNumBits = maxNumbits;
   mUnitsUsed = numbits * 8;
   mUnitsAllocated = maxNumbits * 8;
}

EXTRTMETHOD void ASN1CBitStr::init
(OSOCTET* pBits, OSUINT32& numbits, OSSIZE maxNumbits)
{
   initBase (pBits, numbits, maxNumbits);
   mpNumBits = new ASN1CBitStrSizeHolder32 (numbits);
}

EXTRTMETHOD void ASN1CBitStr::init
(OSOCTET* pBits, OSUINT16& numbits, OSSIZE maxNumbits)
{
   initBase (pBits, numbits, maxNumbits);
   mpNumBits = new ASN1CBitStrSizeHolder16 (numbits);
}

EXTRTMETHOD void ASN1CBitStr::init
(OSOCTET* pBits, OSUINT8& numbits, OSSIZE maxNumbits)
{
   initBase (pBits, numbits, maxNumbits);
   mpNumBits = new ASN1CBitStrSizeHolder8 (numbits);
}

EXTRTMETHOD void ASN1CBitStr::init (ASN1TDynBitStr& bitStr)
{
   mpUnits = (OSOCTET**)&bitStr.data;
   mpNumBits = new ASN1CBitStrSizeHolder32 (bitStr.numbits);
   mMaxNumBits = UINT_MAX;
   mUnitsAllocated = mUnitsUsed = unitIndexNbits (bitStr.numbits);
}

void ASN1CBitStr::privateInit (OSSIZE nbits)
{
   if (0 != mpNumBits) {
      mpNumBits->setValue (0);
   }
   mDynAlloc = TRUE;
   if (nbits == 0) {
      (*mpUnits) = 0;
      mMaxNumBits = UINT_MAX;
      mUnitsUsed = mUnitsAllocated = 0;
      return;
   }
   mUnitsUsed = 0;
   mUnitsAllocated = unitIndexNbits (nbits);
   (*mpUnits) = allocateMemory (mUnitsAllocated);
   mMaxNumBits = nbits;
}

void ASN1CBitStr::privateInit
(OSOCTET* bitStr, OSSIZE maxNumbits_, OSOCTET** ppExtData)
{
   mMaxNumBits = maxNumbits_;
   (*mpUnits) = bitStr;
   mppExtData = ppExtData;

   if (0 != mpNumBits) {
      if (mMaxNumBits < mpNumBits->getValue() && 0 == ppExtData) {
         mpNumBits->setValue (mMaxNumBits);
      }
      mUnitsUsed = unitIndexNbits (mpNumBits->getValue());
   }
   else {
      mUnitsUsed = unitIndexNbits (mMaxNumBits);
   }
   mUnitsAllocated = unitIndexNbits (mMaxNumBits);

   // remove unused bits in the last octet
   if (0 != mpNumBits && mpNumBits->getValue() < mMaxNumBits) {
      if (mUnitsUsed > 0)
         (*mpUnits)[mUnitsUsed - 1] &=
   	 bitsLeftOf((mpNumBits->getValue()) & BIT_INDEX_MASK);
   }

   // clear unused bytes
   if (mUnitsAllocated > mUnitsUsed)
      OSCRTLMEMSET
         (&(*mpUnits)[mUnitsUsed], 0, (mUnitsAllocated - mUnitsUsed));

   mDynAlloc = FALSE;
}

void ASN1CBitStr::privateInit (ASN1TDynBitStr& bitStr)
{
   mMaxNumBits = UINT_MAX;
   mUnitsAllocated = mUnitsUsed = unitIndexNbits (bitStr.numbits);

   // remove unused bits in the last octet
   if (mUnitsUsed > 0)
      (*mpUnits)[mUnitsUsed - 1] &= bitsLeftOf(mMaxNumBits & BIT_INDEX_MASK);

   mDynAlloc = FALSE;
}


void ASN1CBitStr::privateInit (ASN1TDynBitStr64& bitStr)
{
   mMaxNumBits = UINT_MAX;
   mUnitsAllocated = mUnitsUsed = unitIndexNbits (bitStr.numbits);

   // remove unused bits in the last octet
   if (mUnitsUsed > 0)
      (*mpUnits)[mUnitsUsed - 1] &= bitsLeftOf(mMaxNumBits & BIT_INDEX_MASK);

   mDynAlloc = FALSE;
}


/*
 * Creates empty bit string. If nbits == 0 then bit string is dynamic;
 * else capacity will be fixed to nbits.
 */
EXTRTMETHOD ASN1CBitStr::ASN1CBitStr
(OSRTMessageBufferIF& msgBuf, OSSIZE nbits):
   ASN1CType(msgBuf), mpUnits(&_units)
{
   mpNumBits = new ASN1CBitStrSizeHolder64 (_numbits);
   privateInit (nbits);
}

/*
 * Copy constructor.
 */
EXTRTMETHOD ASN1CBitStr::ASN1CBitStr (const ASN1CBitStr& bitStr) :
   ASN1CType(bitStr), mpUnits(&_units)
{
   mMaxNumBits = bitStr.mMaxNumBits;
   mpNumBits = new ASN1CBitStrSizeHolder64 (_numbits);
   mUnitsAllocated = bitStr.mUnitsAllocated;
   if(mUnitsAllocated) {
      mUnitsUsed = bitStr.mUnitsUsed;
      (*mpUnits) = allocateMemory(mUnitsAllocated);
      OSCRTLSAFEMEMCPY (*mpUnits, mUnitsAllocated, *bitStr.mpUnits,
                        mUnitsUsed * sizeof(OSOCTET));
   }
   else {
      (*mpUnits) = 0;
      mUnitsUsed = 0;
   }
   mDynAlloc = TRUE;
}

/*
 * Copy constructor, but allows to specify 'extendable' parameter
 */
EXTRTMETHOD ASN1CBitStr::ASN1CBitStr
(const ASN1CBitStr& bitStr, OSBOOL extendable):
   ASN1CType(bitStr), mpUnits(&_units)
{
   mpNumBits = new ASN1CBitStrSizeHolder64 (_numbits);
   if (extendable) {
      mUnitsAllocated = bitStr.mUnitsUsed + ADD_DELTA;
      mMaxNumBits = UINT_MAX;
   }
   else {
      mUnitsAllocated = bitStr.mUnitsAllocated;
      mMaxNumBits = bitStr.mMaxNumBits;
   }
   if (0 != mpNumBits) {
      mpNumBits->setValue (bitStr.mpNumBits->getValue());
   }
   if(mUnitsAllocated) {
      mUnitsUsed = bitStr.mUnitsUsed;
      (*mpUnits) = allocateMemory (mUnitsAllocated);
      OSCRTLSAFEMEMCPY (*mpUnits, mUnitsAllocated, *bitStr.mpUnits, mUnitsUsed);
      }
   else {
      (*mpUnits) = 0;
      mUnitsUsed = 0;
   }
   mDynAlloc = TRUE;
}

EXTRTMETHOD ASN1CBitStr::ASN1CBitStr
(OSRTMessageBufferIF& msgBuf, ASN1TDynBitStr& bitStr):
   ASN1CType(msgBuf), mpUnits((OSOCTET**)&bitStr.data)
{
   mpNumBits = new ASN1CBitStrSizeHolder32 (bitStr.numbits);
   privateInit (bitStr);
}

EXTRTMETHOD ASN1CBitStr::ASN1CBitStr
(OSRTMessageBufferIF& msgBuf, ASN1TDynBitStr64& bitStr):
   ASN1CType(msgBuf), mpUnits((OSOCTET**)&bitStr.data)
{
   mpNumBits = new ASN1CBitStrSizeHolder64 (bitStr.numbits);
   privateInit (bitStr);
}

#if SIZE_MAX > OSUINT32_MAX        //if size_t > 32 bits
EXTRTMETHOD ASN1CBitStr::ASN1CBitStr
(OSRTMessageBufferIF& msgBuf, OSOCTET* bitStr,
 OSSIZE& octsNumbits, OSSIZE maxNumbits_, OSOCTET** ppExtData):
   ASN1CType(msgBuf), mpUnits(&_units)
{
   mpNumBits = new ASN1CBitStrSizeHolder64 (octsNumbits);
   privateInit (bitStr, maxNumbits_, ppExtData);
}
#endif

EXTRTMETHOD ASN1CBitStr::ASN1CBitStr
(OSRTMessageBufferIF& msgBuf, OSOCTET* bitStr,
 OSUINT32& octsNumbits, OSSIZE maxNumbits_, OSOCTET** ppExtData):
   ASN1CType(msgBuf), mpUnits(&_units)
{
   mpNumBits = new ASN1CBitStrSizeHolder32 (octsNumbits);
   privateInit (bitStr, maxNumbits_, ppExtData);
}

EXTRTMETHOD ASN1CBitStr::ASN1CBitStr
(OSRTMessageBufferIF& msgBuf, OSOCTET* bitStr,
 OSUINT16& octsNumbits, OSSIZE maxNumbits_, OSOCTET** ppExtData):
   ASN1CType(msgBuf), mpUnits(&_units)
{
   mpNumBits = new ASN1CBitStrSizeHolder16 (octsNumbits);
   privateInit (bitStr, maxNumbits_, ppExtData);
}

EXTRTMETHOD ASN1CBitStr::ASN1CBitStr
(OSRTMessageBufferIF& msgBuf, OSOCTET* bitStr,
 OSUINT8& octsNumbits, OSSIZE maxNumbits_, OSOCTET** ppExtData):
   ASN1CType(msgBuf), mpUnits(&_units)
{
   mpNumBits = new ASN1CBitStrSizeHolder8 (octsNumbits);
   privateInit (bitStr, maxNumbits_, ppExtData);
}

EXTRTMETHOD ASN1CBitStr::ASN1CBitStr
(OSRTMessageBufferIF& msgBuf, OSOCTET* bitStr, OSSIZE maxNumbits_):
   ASN1CType(msgBuf), mpUnits(&_units)
{
   mpNumBits = 0;
   privateInit (bitStr, maxNumbits_, 0);
}

/*
 * Creates empty bit string. If nbits == 0 then bit string is dynamic;
 * else capacity will be fixed to nbits.
 */
EXTRTMETHOD ASN1CBitStr::ASN1CBitStr (OSRTContext& ctxt, OSSIZE nbits) :
   ASN1CType(ctxt), mpUnits(&_units)
{
   mpNumBits = new ASN1CBitStrSizeHolder64 (_numbits);
   privateInit (nbits);
}

/*
 * Creates bits string from ASN1TDynBitStr. Ctor does not copy bytes,
 * just assign array (data member of ASN1TDynBitStr) to units.
 * This string will be extendable.
 */
EXTRTMETHOD ASN1CBitStr::ASN1CBitStr
(OSRTContext& ctxt, ASN1TDynBitStr& bitStr):
   ASN1CType(ctxt), mpUnits((OSOCTET**)&bitStr.data)
{
   mpNumBits = new ASN1CBitStrSizeHolder32 (bitStr.numbits);
   privateInit (bitStr);
}

EXTRTMETHOD ASN1CBitStr::ASN1CBitStr
(OSRTContext& ctxt, ASN1TDynBitStr64& bitStr) :
   ASN1CType(ctxt), mpUnits((OSOCTET**)&bitStr.data)
{
   mpNumBits = new ASN1CBitStrSizeHolder64(bitStr.numbits);
   privateInit(bitStr);
}

#if SIZE_MAX > OSUINT32_MAX        //if size_t > 32 bits

/* Creates bits string from array of bits. Ctor does not copy bytes,
 * just assign passed array to units.
 * Param bitStr - pointer to static byte array
 * Param numbits - reference to length of bit string (in bits);
 * Param mMaxNumBits  - sets maximum length in bits
 */
EXTRTMETHOD ASN1CBitStr::ASN1CBitStr
(OSRTContext& ctxt, OSOCTET* bitStr,
   OSSIZE& octsNumbits, OSSIZE maxNumbits_, OSOCTET** ppExtData) :
   ASN1CType(ctxt), mpUnits(&_units)
{
   mpNumBits = new ASN1CBitStrSizeHolder64(octsNumbits);
   privateInit(bitStr, maxNumbits_, ppExtData);
}

#endif

EXTRTMETHOD ASN1CBitStr::ASN1CBitStr
(OSRTContext& ctxt, OSOCTET* bitStr,
 OSUINT32& octsNumbits, OSSIZE maxNumbits_, OSOCTET** ppExtData):
   ASN1CType(ctxt), mpUnits(&_units)
{
   mpNumBits = new ASN1CBitStrSizeHolder32 (octsNumbits);
   privateInit (bitStr, maxNumbits_, ppExtData);
}

EXTRTMETHOD ASN1CBitStr::ASN1CBitStr
(OSRTContext& ctxt, OSOCTET* bitStr,
 OSUINT16& octsNumbits, OSSIZE maxNumbits_, OSOCTET** ppExtData):
   ASN1CType(ctxt), mpUnits(&_units)
{
   mpNumBits = new ASN1CBitStrSizeHolder16 (octsNumbits);
   privateInit (bitStr, maxNumbits_, ppExtData);
}

EXTRTMETHOD ASN1CBitStr::ASN1CBitStr
(OSRTContext& ctxt, OSOCTET* bitStr,
 OSUINT8& octsNumbits, OSSIZE maxNumbits_, OSOCTET** ppExtData):
   ASN1CType(ctxt), mpUnits(&_units)
{
   mpNumBits = new ASN1CBitStrSizeHolder8 (octsNumbits);
   privateInit (bitStr, maxNumbits_, ppExtData);
}

EXTRTMETHOD ASN1CBitStr::ASN1CBitStr
(OSRTContext& ctxt, OSOCTET* bitStr, OSSIZE maxNumbits_):
   ASN1CType(ctxt), mpUnits(&_units)
{
   mpNumBits = 0;
   privateInit (bitStr, maxNumbits_, 0);
}

// Destructor
EXTRTMETHOD ASN1CBitStr::~ASN1CBitStr()
{
   delete mpNumBits;
   if (mDynAlloc)
      memFreePtr (*mpUnits);
}

/* Allocate memory and set to zero */
OSOCTET* ASN1CBitStr::allocateMemory (OSSIZE sz)
{
   OSOCTET* mem = (OSOCTET*) memAlloc (sz);
   OSCRTLMEMSET (mem, 0, sz);
   return mem;
}

/* Reallocate memory. */
OSOCTET* ASN1CBitStr::reallocateMemory
(OSOCTET* old, OSSIZE oldBufSz, OSSIZE newBufSz)
{
   OSOCTET* mem = (OSOCTET*) memRealloc (old, newBufSz);
   OSCRTLMEMSET (mem + oldBufSz, 0, (newBufSz - oldBufSz));
   return mem;
}

/* Free memory. */
void ASN1CBitStr::freeMemory (OSOCTET* ptr)
{
   memFreePtr (ptr);
}

/*
 * Set the field mUnitsUsed with the logical size in units of the bit
 * set.  WARNING: This function assumes that the number of units actually
 * in use is less than or equal to the current value of mUnitsUsed!
 */
EXTRTMETHOD void ASN1CBitStr::recalculateUnitsUsed()
{
   if (mUnitsUsed > 0 && (*mpUnits)[mUnitsUsed - 1] == 0) {
      // Traverse the bitstr until a used unit is found
      OSSIZE i;
      for (i = mUnitsUsed - 1; ; i--) {
         if ((*mpUnits)[i] != 0 || i == 0)
            break;
      }
      mUnitsUsed = ((*mpUnits)[i] == 0 && i == 0) ? 0 : i + 1;
   }
}

/*
 * Ensures that the bit string can hold enough units.
 * param - unitsRequired the minimum acceptable number of units.
 */
EXTRTMETHOD int ASN1CBitStr::checkCapacity (OSSIZE unitsRequired)
{
   if (mUnitsAllocated < unitsRequired) {
      if (mMaxNumBits != UINT_MAX)
         return ASN_E_INVLEN; // do not reallocate memory for fixed bitstr

      OSSIZE wasAllocated = mUnitsAllocated;
      mUnitsAllocated = unitsRequired + ADD_DELTA;
      if (wasAllocated > 0) {
         if (mDynAlloc && (*mpUnits)) {
            (*mpUnits) = reallocateMemory
	       ((*mpUnits), wasAllocated, mUnitsAllocated);
	 }
         else {
            OSOCTET* newUnits = (OSOCTET*) allocateMemory (mUnitsAllocated);
            OSCRTLSAFEMEMCPY (newUnits, mUnitsAllocated, (*mpUnits), wasAllocated);
            (*mpUnits) = newUnits;
            mDynAlloc = TRUE;
         }
      }
      else
         (*mpUnits) = (OSOCTET*) allocateMemory (mUnitsAllocated);
   }

   if (mUnitsUsed < unitsRequired)
      mUnitsUsed = unitsRequired;

   return 0;
}

/*
 * Sets the bit at the specified index to 1.
 *
 * Returns: 0 - if succeed, or
 *          RTERR_OUTOFBND - if 'bitIndex' is out of bounds, or
 *          other error codes (see asn1type.h).
 */
EXTRTMETHOD int ASN1CBitStr::set (OSSIZE bitIndex)
{
   if (bitIndex > mMaxNumBits)
      return LOG_RTERR (getCtxtPtr(), RTERR_OUTOFBND);

   OSSIZE unitsIndex = unitIndex (bitIndex);

   int stat = checkCapacity (unitsIndex + 1);
   if (stat != 0)
      return LOG_RTERR(getCtxtPtr(), stat);

   (*mpUnits)[unitsIndex] |= bit(bitIndex);

   if (0 != mpNumBits && mpNumBits->getValue() < bitIndex + 1) {
      mpNumBits->setValue (bitIndex + 1);
   }

   return 0;
}

/*
* Sets the bits from the specified fromIndex(inclusive) to the
* specified toIndex(exclusive) to 1.
*
* Returns: 0 - if succeed, or
*          RTERR_OUTOFBND - if one of indexes is out of bounds, or
*          RTERR_BADVALUE - if fromIndex > toIndex, or
*          other error codes (see asn1type.h).
*/
EXTRTMETHOD int ASN1CBitStr::set (OSSIZE fromIndex, OSSIZE toIndex)
{
   if (fromIndex > mMaxNumBits || toIndex > mMaxNumBits)
      return LOG_RTERR(getCtxtPtr(), RTERR_OUTOFBND);

   if (fromIndex > toIndex)
      return LOG_RTERR(getCtxtPtr(), RTERR_BADVALUE);
   else if (fromIndex == toIndex)
      return 0;

   // Increase capacity if necessary
   OSSIZE endUnitIndex = unitIndex (toIndex - 1);
   int stat = checkCapacity (endUnitIndex + 1);
   if (stat != 0)
      return LOG_RTERR (getCtxtPtr(), stat);

   if (0 != mpNumBits && mpNumBits->getValue() < toIndex) {
      mpNumBits->setValue (toIndex);
   }

   OSSIZE startUnitIndex = unitIndex (fromIndex);
   long bitMask;
   if (startUnitIndex == endUnitIndex) {
      // Case 1: One octet
      bitMask =
	 ((OSINTCONST(1) << BITS_PER_UNIT) >> (fromIndex & BIT_INDEX_MASK)) -
	 ((OSINTCONST(1) << BITS_PER_UNIT) >> (toIndex & BIT_INDEX_MASK));
      (*mpUnits)[startUnitIndex] |= bitMask;
      return 0;
   }

   // Case 2: Multiple octets
   // Handle first octet
   bitMask = bitsRightOf(fromIndex & BIT_INDEX_MASK);
   (*mpUnits)[startUnitIndex] |= bitMask;

   // Handle intermediate octets, if any
   if (endUnitIndex > startUnitIndex + 1) {
      for (OSSIZE i = startUnitIndex+1; i < endUnitIndex; i++)
	 (*mpUnits)[i] |= UNIT_MASK;
   }

   // Handle last octet
   bitMask = bitsLeftOf(toIndex & BIT_INDEX_MASK);
   (*mpUnits)[endUnitIndex] |= bitMask;

   return 0;
}

/*
 * Sets the bit specified by the index to 0.
 *
 * Returns: 0 - if succeed, or
 *          RTERR_OUTOFBND - if 'bitIndex' is out of bounds, or
 *          other error codes (see asn1type.h).
 */
EXTRTMETHOD int ASN1CBitStr::clear (OSSIZE bitIndex)
{
   if (bitIndex > mMaxNumBits)
      return LOG_RTERR(getCtxtPtr(), RTERR_OUTOFBND);

   OSSIZE unitsIndex = unitIndex(bitIndex);
   if (unitsIndex >= mUnitsUsed)
      return 0;

   (*mpUnits)[unitsIndex] &= ~bit(bitIndex);

   recalculateUnitsUsed();

   return 0;
}

/*
 * Sets the bits from the specified fromIndex(inclusive) to the
 * specified toIndex(exclusive) to 0.
 *
 * Returns: 0 - if succeed, or
 *          RTERR_OUTOFBND - if one of indexes is out of bounds, or
 *          RTERR_BADVALUE - if fromIndex > toIndex, or
 *          other error codes (see asn1type.h).
 */
EXTRTMETHOD int ASN1CBitStr::clear (OSSIZE fromIndex, OSSIZE toIndex)
{
   if (fromIndex > mMaxNumBits || toIndex > mMaxNumBits)
      return LOG_RTERR(getCtxtPtr(), RTERR_OUTOFBND);

   if (fromIndex > toIndex)
      return LOG_RTERR(getCtxtPtr(), RTERR_BADVALUE);
   else if (fromIndex == toIndex)
      return 0;

   // Increase capacity if necessary
   OSSIZE endUnitIndex = unitIndex (toIndex - 1);
   int stat = checkCapacity (endUnitIndex + 1);
   if (stat != 0)
      return LOG_RTERR (getCtxtPtr(), stat);

   if (0 != mpNumBits && mpNumBits->getValue() < toIndex) {
      mpNumBits->setValue (toIndex);
   }

   OSSIZE startUnitIndex = unitIndex(fromIndex);
   if (startUnitIndex >= mUnitsUsed)
      return 0;

   long bitMask;
   if (startUnitIndex == endUnitIndex) {
      // Case 1: One octet
      bitMask =
	 ((OSINTCONST(1) << BITS_PER_UNIT) >> (fromIndex & BIT_INDEX_MASK)) -
	 ((OSINTCONST(1) << BITS_PER_UNIT) >> (toIndex & BIT_INDEX_MASK));
      (*mpUnits)[startUnitIndex] &= ~bitMask;
      recalculateUnitsUsed();
      return 0;
   }

   // Case 2: Multiple octets
   // Handle first octet
   bitMask = bitsRightOf(fromIndex & BIT_INDEX_MASK);
   (*mpUnits)[startUnitIndex] &= ~bitMask;

   // Handle intermediate octets, if any
   if (endUnitIndex > startUnitIndex + 1) {
      for (OSSIZE i = startUnitIndex + 1; i < endUnitIndex; i++) {
         if (i < mUnitsUsed)
            (*mpUnits)[i] = 0;
      }
   }

   // Handle last octet
   if (endUnitIndex < mUnitsUsed) {
      bitMask = bitsLeftOf(toIndex & BIT_INDEX_MASK);
      (*mpUnits)[endUnitIndex] &= ~bitMask;
   }

   recalculateUnitsUsed();

   return 0;
}

/*
 * Sets all of the bits in this bit string to 0.
 */
EXTRTMETHOD void ASN1CBitStr::clear()
{
   for (OSSIZE i = mUnitsUsed; i > 0;)
      (*mpUnits)[--i] = 0;

   mUnitsUsed = 0; // update mUnitsUsed

   if (0 != mpNumBits) { mpNumBits->setValue (0); }
}

/*
 * Sets the bit at the specified index to to the complement of its
 * current value.
 *
 * Returns: 0 - if succeed, or
 *          RTERR_OUTOFBND - if 'bitIndex' is out of bounds, or
 *          other error codes (see asn1type.h).
 */
EXTRTMETHOD int ASN1CBitStr::invert (OSSIZE bitIndex)
{
   if (bitIndex > mMaxNumBits)
      return LOG_RTERR (getCtxtPtr(), RTERR_OUTOFBND);

   OSSIZE unitsIndex = unitIndex(bitIndex);

   int stat = checkCapacity (unitsIndex + 1);
   if (stat != 0)
      return LOG_RTERR(getCtxtPtr(), stat);

   (*mpUnits)[unitsIndex] ^= bit(bitIndex);

   recalculateUnitsUsed();

   return 0;
}

/*
* Sets each bit from the specified fromIndex(inclusive) to the
* specified toIndex(exclusive) to the complement of its current
* value.
*
* Returns: 0 - if succeed, or
*          RTERR_OUTOFBND - if one of indexes is out of bounds, or
*          RTERR_BADVALUE - if fromIndex > toIndex, or
*          other error codes (see asn1type.h).
*/
EXTRTMETHOD int ASN1CBitStr::invert (OSSIZE fromIndex, OSSIZE toIndex)
{
   if (fromIndex > mMaxNumBits || toIndex > mMaxNumBits)
      return LOG_RTERR(getCtxtPtr(), RTERR_OUTOFBND);

   if (fromIndex > toIndex)
      return LOG_RTERR(getCtxtPtr(), RTERR_BADVALUE);
   else if (fromIndex == toIndex)
      return 0;

   // Increase capacity if necessary
   OSSIZE endUnitIndex = unitIndex (toIndex - 1);
   int stat = checkCapacity (endUnitIndex + 1);
   if (stat != 0)
      return LOG_RTERR (getCtxtPtr(), stat);

   if (0 != mpNumBits && mpNumBits->getValue() < toIndex) {
      mpNumBits->setValue (toIndex);
   }

   OSSIZE startUnitIndex = unitIndex(fromIndex);
   if (startUnitIndex >= mUnitsUsed)
      return 0;

   long bitMask;
   if (startUnitIndex == endUnitIndex) {
      // Case 1: One octet
      bitMask =
	 ((OSINTCONST(1) << BITS_PER_UNIT) >> (fromIndex & BIT_INDEX_MASK)) -
	 ((OSINTCONST(1) << BITS_PER_UNIT) >> (toIndex & BIT_INDEX_MASK));
      (*mpUnits)[startUnitIndex] ^= bitMask;
      recalculateUnitsUsed();
      return 0;
   }

   // Case 2: Multiple octets
   // Handle first octet
   bitMask = bitsRightOf (fromIndex & BIT_INDEX_MASK);
   (*mpUnits)[startUnitIndex] ^= bitMask;

   // Handle intermediate octets, if any
   if (endUnitIndex > startUnitIndex + 1) {
      for (OSSIZE i = startUnitIndex + 1; i < endUnitIndex; i++)
         (*mpUnits)[i] ^= UNIT_MASK;
   }

   // Handle last octet
   bitMask = bitsLeftOf(toIndex & BIT_INDEX_MASK);
   (*mpUnits)[endUnitIndex] ^= bitMask;

   // Check to see if we reduced size
   recalculateUnitsUsed();

   return 0;
}

/*
 * Returns the value of the bit with the specified index.
 */
EXTRTMETHOD OSBOOL ASN1CBitStr::get (OSSIZE bitIndex)
{
   if (bitIndex > mMaxNumBits)
      return FALSE;

   OSBOOL result = FALSE;
   OSSIZE unitsIndex = unitIndex(bitIndex);
   if (unitsIndex < mUnitsUsed)
      result = (((*mpUnits)[unitsIndex] & bit(bitIndex)) != 0);

   return result;
}

/*
 * Returns the number of bytes of space actually in use by this
 * bit string to represent bit values.
 */
EXTRTMETHOD OSSIZE ASN1CBitStr::size() const {
   return mUnitsUsed;
}

/*
* Returns the "logical size" of this bit string: the index of
* the highest set bit in the bit string plus one. Returns zero
* if the bit string contains no set bits.
*/
EXTRTMETHOD OSSIZE ASN1CBitStr::length() const {
   if (mUnitsUsed == 0)
      return 0;

   OSOCTET highestUnit = (*mpUnits)[mUnitsUsed - 1];
   return BITS_PER_UNIT * (mUnitsUsed - 1) + rtxOctetBitLen (highestUnit);
}

/*
* Returns the number of bits set in val.
* For a derivation of this algorithm, see
* "Algorithms and data structures with applications to
*  graphics and geometry", by Jurg Nievergelt and Klaus Hinrichs,
*  Prentice Hall, 1993.
*/
static int bitCount(unsigned long val) {
#ifndef __SYMBIAN32__
#if BITS_PER_UNIT <= 8
   val -= (val & 0xaaL) >> 1;
   val =  (val & 0x33L) + ((val >> 2) & 0x33L);
   val =  (val + (val >> 4)) & 0x0fL;
   return (int)(val & 0xff);

#elif BITS_PER_UNIT <= 16
   val -= (val & 0xaaaaL) >> 1;
   val =  (val & 0x3333L) + ((val >> 2) & 0x3333L);
   val =  (val + (val >> 4)) & 0x0f0fL;
   val += val >> 8;
   return (int)(val & 0xff);

#elif BITS_PER_UNIT <= 32
   val -= (val & 0xaaaaaaaaL) >> 1;
   val =  (val & 0x33333333L) + ((val >> 2) & 0x33333333L);
   val =  (val + (val >> 4)) & 0x0f0f0f0fL;
   val += val >> 8;
   val += val >> 16;
   return (int)(val & 0xff);

#elif BITS_PER_UNIT <= 64
   val -= (val & 0xaaaaaaaaaaaaaaaaL) >> 1;
   val =  (val & 0x3333333333333333L) + ((val >> 2) & 0x3333333333333333L);
   val =  (val + (val >> 4)) & 0x0f0f0f0f0f0f0f0fL;
   val += val >> 8;
   val += val >> 16;
   return ((int)(val) + (int)(val >> 32)) & 0xff;

#else
#error Size of BITS_PER_UNITS greater than 64 bit is not supported
#endif
#else /* we are using SYMBIAN32 */
   return 0;
#endif
}

/*
 * Returns the number of bits set to 1 in this
 * bit string.
 */
EXTRTMETHOD OSSIZE ASN1CBitStr::cardinality() const {
   OSUINT32 sum = 0;

   for (OSUINT32 i = 0; i < mUnitsUsed; i++)
      sum += bitCount((*mpUnits)[i]);

   return sum;
}

/*
* Copies bit string to buffer (pBuf).
*/
EXTRTMETHOD int ASN1CBitStr::getBytes (OSOCTET* pBuf, OSSIZE bufSz)
{
   if (bufSz < mUnitsUsed)
      return LOG_RTERR(getCtxtPtr(), ASN_E_INVLEN);

   OSCRTLSAFEMEMCPY (pBuf, bufSz, (*mpUnits), mUnitsUsed);

   if (bufSz > mUnitsUsed)
      OSCRTLMEMSET (pBuf + mUnitsUsed, 0, (bufSz - mUnitsUsed));

   return 0;
}

/*
 * Returns the unit of this bitset at index j as if this bit string had an
 * infinite amount of storage.
 */
EXTRTMETHOD OSOCTET ASN1CBitStr::getBits (OSSIZE j) {
   return (j < mUnitsUsed) ? (*mpUnits)[j] : 0;
}

/*
 * Copies bit string composed of bits from this bit string
 * from 'fromIndex'(inclusive) to 'toIndex'(exclusive).
 *
 * Returns: 0 - if succeed, or
 *          RTERR_OUTOFBND - if one of indexes is out of bounds, or
 *          RTERR_BADVALUE - if fromIndex > toIndex, or
 *          other error codes (see asn1type.h).
 */
EXTRTMETHOD int ASN1CBitStr::get
(OSSIZE fromIndex, OSSIZE toIndex, OSOCTET* pBuf, OSSIZE bufSz)
{
   if(fromIndex > mMaxNumBits)
      return LOG_RTERR(getCtxtPtr(), RTERR_OUTOFBND);
   if(toIndex > mMaxNumBits)
      return LOG_RTERR(getCtxtPtr(), RTERR_OUTOFBND);
   if(fromIndex > toIndex)
      return LOG_RTERR(getCtxtPtr(), RTERR_BADVALUE);
   if(toIndex - fromIndex > (OSUINT32)(bufSz << ADDRESS_BITS))
      return LOG_RTERR(getCtxtPtr(), RTERR_STROVFLW);

   OSCRTLMEMSET (pBuf, 0, bufSz * sizeof(OSOCTET));

   OSSIZE len = length();
   // If no set bits in range return empty bitset
   if (len <= fromIndex || fromIndex == toIndex)
      return 0;

   // An optimization
   if (len < toIndex)
      toIndex = len;

   int startBitIndex = fromIndex & BIT_INDEX_MASK;
   int endBitIndex = toIndex & BIT_INDEX_MASK;
   OSSIZE targetOcts = ((toIndex - fromIndex + BITS_PER_UNIT - 1) >> ADDRESS_BITS);
   OSSIZE sourceOcts = unitIndex(toIndex) - unitIndex(fromIndex) + 1;
   int inverseIndex = BITS_PER_UNIT - startBitIndex;
   OSSIZE targetIndex = 0;
   OSSIZE sourceIndex = unitIndex(fromIndex);

   // Process all octets but the last octet
   while (targetIndex < targetOcts - 1) {
      pBuf[targetIndex++] =
         ((*mpUnits)[sourceIndex] << startBitIndex) |
         ((inverseIndex == BITS_PER_UNIT) ? 0 :
          (*mpUnits)[sourceIndex + 1] >> inverseIndex);
      sourceIndex++;
   }

   // Process the last octet
   pBuf[targetIndex] =
      (sourceOcts == targetOcts ?
       ((*mpUnits)[sourceIndex] & bitsLeftOf(endBitIndex)) << startBitIndex :
       ((*mpUnits)[sourceIndex] << startBitIndex) |
       ((inverseIndex == BITS_PER_UNIT) ? 0 :
        (getBits(sourceIndex + 1) & bitsLeftOf(endBitIndex)) >> inverseIndex));

   return 0;
}

/*
* Performs a logical AND of this target bit set with the
* argument bit set.
*
* Returns: 0 - if succeed, or
*          ASN_E_INVLEN - if 'octsNumbits' is negative, or
*          RTERR_INVPARAM - if pOctstr is the same bit string as this or null,
*          or other error codes (see asn1type.h).
*/
EXTRTMETHOD int ASN1CBitStr::doAnd
(const OSOCTET* pOctstr, OSSIZE octsNumbits)
{
   if (octsNumbits == 0) {
      clear();
      return 0;
   }
   if ((*mpUnits) == pOctstr || !pOctstr)
      return LOG_RTERR(getCtxtPtr(), RTERR_INVPARAM);

   // Perform logical AND on bits in common
   OSSIZE otherUnitsUsed =
      ((octsNumbits + BITS_PER_UNIT - 1) >> ADDRESS_BITS);

   OSSIZE unitsInCommon =
      (otherUnitsUsed < mUnitsUsed) ? otherUnitsUsed : mUnitsUsed;

   OSSIZE i;
   for (i = 0; i < unitsInCommon; i++)
      (*mpUnits)[i] &= pOctstr[i];

   // Clear out (*mpUnits) no longer used
   if (mUnitsUsed > i)
      memset(&(*mpUnits)[i], 0, (mUnitsUsed - i) * sizeof(OSOCTET));

   // Recalculate units in use if necessary
   mUnitsUsed = unitsInCommon;

   recalculateUnitsUsed();

   return 0;
}

/*
 * Performs a logical OR of this target bit set with the
 * argument bit set.
 *
 * Returns: 0 - if succeed, or
 *          ASN_E_INVLEN - if 'octsNumbits' is negative, or
 *          RTERR_INVPARAM - if pOctstr is the same bit string as this or null,
 *          or other error codes (see asn1type.h).
 */
EXTRTMETHOD int ASN1CBitStr::doOr
(const OSOCTET* pOctstr, OSSIZE octsNumbits)
{
   if (octsNumbits == 0) {
      if (mMaxNumBits == UINT_MAX)
         return LOG_RTERR (getCtxtPtr(), RTERR_INVPARAM);
      set (0, mMaxNumBits);
      return 0;
   }
   if((*mpUnits) == pOctstr || !pOctstr)
      return LOG_RTERR(getCtxtPtr(), RTERR_INVPARAM);

   if(mMaxNumBits < octsNumbits)
      octsNumbits = mMaxNumBits;

   OSSIZE otherUnitsUsed =
      ((octsNumbits + BITS_PER_UNIT - 1) >> ADDRESS_BITS);

   int stat = checkCapacity(otherUnitsUsed);
   if (stat != 0)
      return LOG_RTERR(getCtxtPtr(), stat);

   // Perform logical OR on bits in common
   OSSIZE i;
   for (i = 0; i < otherUnitsUsed - 1; i++)
      (*mpUnits)[i] |= pOctstr[i];

   // Process last unit
   (*mpUnits)[i] |= (pOctstr[i] & bitsLeftOf(octsNumbits & BIT_INDEX_MASK));

   mpNumBits->setValue (length());

   return 0;
}

/*
* Performs a logical XOR of this target bit set with the
* argument bit set.
*
* Returns: 0 - if succeed, or
*          ASN_E_INVLEN - if 'octsNumbits' is negative, or
*          RTERR_INVPARAM - if pOctstr is null, or
*          other error codes (see asn1type.h).
*/
EXTRTMETHOD int ASN1CBitStr::doXor
(const OSOCTET* pOctstr, OSSIZE octsNumbits)
{
   if (octsNumbits == 0)
      return 0; // no need to change bit string if octsNumbits is zero
   if (0 == pOctstr)
      return LOG_RTERR(getCtxtPtr(), RTERR_INVPARAM);

   if (mMaxNumBits < octsNumbits)
      octsNumbits = mMaxNumBits;

   OSSIZE otherUnitsUsed =
      ((octsNumbits + BITS_PER_UNIT - 1) >> ADDRESS_BITS);

   if (mUnitsUsed < otherUnitsUsed) {
      int stat = checkCapacity(otherUnitsUsed);
      if(stat != 0)
         return LOG_RTERR(getCtxtPtr(), stat);
   }

   // Perform logical XOR on bits in common
   OSSIZE i;
   for (i = 0; i < otherUnitsUsed - 1; i++)
      (*mpUnits)[i] ^= pOctstr[i];

   // Process last unit
   (*mpUnits)[i] ^= (pOctstr[i] & bitsLeftOf(octsNumbits & BIT_INDEX_MASK));

   recalculateUnitsUsed();

   return 0;
}

/*
* Clears all of the bits in this bit string whose corresponding
* bit is set in the specified bit string.
*
* Returns: 0 - if succeed, or
*          ASN_E_INVLEN - if 'octsNumbits' is negative, or
*          RTERR_INVPARAM - if pOctstr is null, or
*          other error codes (see asn1type.h).
*/
EXTRTMETHOD int ASN1CBitStr::doAndNot
(const OSOCTET* pOctstr, OSSIZE octsNumbits)
{
   if (octsNumbits == 0)
      return 0; // no need to change bit string if octsNumbits is zero
   if (!pOctstr)
      return LOG_RTERR(getCtxtPtr(), RTERR_INVPARAM);

   OSSIZE otherUnitsUsed =
      ((octsNumbits + BITS_PER_UNIT - 1) >> ADDRESS_BITS);

   OSSIZE unitsInCommon =
      (mUnitsUsed < otherUnitsUsed) ? mUnitsUsed : otherUnitsUsed;

   // Perform logical (a & !b) on bits in common
   for (OSSIZE i = 0; i < unitsInCommon; i++) {
      (*mpUnits)[i] &= ~pOctstr[i];
   }

   recalculateUnitsUsed();

   return 0;
}

/*
 * Shifts all bits to the left by 'shift' bits.
 */
EXTRTMETHOD int ASN1CBitStr::shiftLeft (OSSIZE shift)
{
   OSSIZE srcOct = unitIndex(shift);
   OSSIZE srcIndex = shift & BIT_INDEX_MASK;
   OSSIZE invIndex = BITS_PER_UNIT - srcIndex;
   OSSIZE dstOct = 0;
   OSSIZE len = length();
   if (len == 0)
      return 0;
   OSSIZE targetOcts = ((len - shift + BITS_PER_UNIT - 1) >> ADDRESS_BITS);
   // Process all octets but the last octet
   while(dstOct < targetOcts - 1) {
      (*mpUnits)[dstOct++] =
         ((*mpUnits)[srcOct] << srcIndex) |
         ((invIndex == BITS_PER_UNIT) ? 0 : (*mpUnits)[srcOct + 1] >> invIndex);
      srcOct++;
   }

   len &= BIT_INDEX_MASK;
   // Process the last octet
   (*mpUnits)[dstOct] = (mUnitsUsed == targetOcts ?
     ((*mpUnits)[srcOct] & bitsLeftOf(len)) << srcIndex :
     ((*mpUnits)[srcOct] << srcIndex) | ((invIndex == BITS_PER_UNIT) ? 0 :
     (getBits(srcOct + 1) & bitsLeftOf(len)) >> invIndex));

   // Clear out unused bytes
   if (targetOcts < mUnitsUsed) {
      OSCRTLMEMSET (&(*mpUnits)[targetOcts], 0, (mUnitsUsed - targetOcts));
   }

   recalculateUnitsUsed();

   return 0;
}

/*
 * Shifts all bits to the right by 'shift' bits.
 */
EXTRTMETHOD int ASN1CBitStr::shiftRight (OSSIZE shift)
{
   OSSIZE invIndex = shift & BIT_INDEX_MASK;
   OSSIZE srcIndex = BITS_PER_UNIT - invIndex;
   OSSIZE len = length();
   if(len == 0)
      return 0;

   if(mMaxNumBits != UINT_MAX) {
      if ( len < shift ) {
         clear();
         return 0;
      }
      len -= shift;
   }

   /* Is the code correct here if len < 7 ? The value passed to unitIndex
      would be negative.*/
   OSSIZE srcOct = unitIndex(len - BITS_PER_UNIT + 1);
   OSSIZE dstOct = unitIndex(len + shift - 1);
   OSSIZE emptyOcts = unitIndex(shift);

   if(mMaxNumBits == UINT_MAX) { // dynamic? allocate more mem, if necessary
      int stat = checkCapacity(dstOct + 1);
      if(stat != 0)
         return LOG_RTERR(getCtxtPtr(), stat);
      }

   // Process first (last in fact) unit
   if((len & BIT_INDEX_MASK) > srcIndex)
      (*mpUnits)[dstOct--] = ((*mpUnits)[srcOct + 1] << srcIndex);

   // Process all octets but the last octet
   for(;;) {
      (*mpUnits)[dstOct--] =
         ((*mpUnits)[srcOct] << srcIndex) |
         ((invIndex == BITS_PER_UNIT) ? 0 : (*mpUnits)[srcOct + 1] >> invIndex);

      if ( srcOct == 0 ) break;
      else srcOct--;
   }

   // Process the last (first, in fact) unit
   if(invIndex != BITS_PER_UNIT) {
      (*mpUnits)[dstOct] = ((*mpUnits)[0] >> invIndex);;
   }

   // Clear out unused bytes
   if(emptyOcts > 0) {
      OSCRTLMEMSET (&(*mpUnits)[0], 0, emptyOcts * sizeof(OSOCTET));
    }
   if(mMaxNumBits != UINT_MAX) {
      // Clear out unised bits in last unit
      (*mpUnits)[mUnitsUsed - 1] &= bitsLeftOf(mMaxNumBits & BIT_INDEX_MASK);
   }
   recalculateUnitsUsed();
   return 0;
}

/*
 * Returns number of unused bits in last unit.
 */
EXTRTMETHOD OSUINT32 ASN1CBitStr::unusedBitsInLastUnit()
{
   if(mUnitsUsed == 0)
      return 0;

   OSOCTET highestUnit = (*mpUnits)[mUnitsUsed - 1];
   return BITS_PER_UNIT - rtxOctetBitLen(highestUnit);
}

EXTRTMETHOD ASN1CBitStr::operator ASN1TDynBitStr()
{
   ASN1TDynBitStr dynOctStr;
   dynOctStr.data = (*mpUnits);
   dynOctStr.numbits = (OSUINT32) length();
   return dynOctStr;
}


EXTRTMETHOD ASN1CBitStr::operator ASN1TDynBitStr*()
{
   ASN1TDynBitStr* dynOctStr =
      (ASN1TDynBitStr*) memAlloc (sizeof(ASN1TDynBitStr));
   void* data = memAlloc (mUnitsUsed);
   OSCRTLSAFEMEMCPY (data, mUnitsUsed, (*mpUnits), mUnitsUsed);
   dynOctStr->data = (const OSOCTET*)data;
   dynOctStr->numbits = (OSUINT32) length();
   return dynOctStr;
}

int ASN1CBitStrSizeHolder8::setValue (OSSIZE value)
{
   if (value <= OSUINT8_MAX) {
      mSize = (OSUINT8) value;
      return 0;
   }
   else return RTERR_TOOBIG;
}

int ASN1CBitStrSizeHolder16::setValue (OSSIZE value)
{
   if (value <= OSUINT16_MAX) {
      mSize = (OSUINT16) value;
      return 0;
   }
   else return RTERR_TOOBIG;
}

int ASN1CBitStrSizeHolder32::setValue (OSSIZE value)
{
   if (value <= OSUINT32_MAX) {
      mSize = (OSUINT32) value;
      return 0;
   }
   else return RTERR_TOOBIG;
}

int ASN1CBitStrSizeHolder64::setValue (OSSIZE value)
{
   mSize = (OSUINT32) value;
   return 0;
}
