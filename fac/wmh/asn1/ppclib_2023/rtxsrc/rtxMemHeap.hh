/*
 * Copyright (c) 2003-2023 by Objective Systems, Inc.
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
#ifndef __RTXMEMHEAP_HH__
#define __RTXMEMHEAP_HH__

#ifndef __RTXMEMORY_H__
#define __RTXMEMORY_H__ /* prevent early include of rtxMemory.h */
#define __RTXMEMORY_H__INCLUDED
#endif /* __RTXMEMORY_H__ */
#include "rtxsrc/osSysTypes.h"
#include "rtxsrc/rtxExternDefs.h"
#include "rtxsrc/rtxError.h"
#ifdef __RTXMEMORY_H__INCLUDED
#undef  __RTXMEMORY_H__
#endif /* __RTXMEMORY_H__INCLUDED */

#include "rtxsrc/rtxMemory.h"
#ifdef _DEBUG
#include "rtxsrc/rtxDiag.h"
/* Note: by default debug support is disabled because it makes the
 * memory manager run extremely slow.  This should only be turned on
 * if it is necessary to debug a memory problem.. */
/*
   #define _MEMDEBUG
   #define _MEMTRACE
*/

#endif

/* internal heap flags */
#define RT_MH_INTERNALMASK OSUINTCONST(0xF0000000)
#define RT_MH_FREEHEAPDESC OSUINTCONST(0x10000000)
#define RT_MH_USEATOMBLOCK OSUINTCONST(0x20000000)
#define RT_MH_STATIC       OSUINTCONST(0x40000000)

/* OSMemLink: This structure is used to maintain a doubly-linked list of
   pointers to the blocks of memory allocated to the memory manager.
   OSMemLink structures are located at the start of OSRawMemBlk and
   OSMemBlk structures.
*/
typedef struct OSMemLink {
   struct OSMemLink* pnext;
   struct OSMemLink* pprev;
} OSMemLink;

/*
See notes below on OSMemElemDescr and OSFreeElemLink
*/
typedef void OSMemElemDescr;
typedef void OSFreeElemLink;

typedef struct {
   OSMemLink rawBlkList;
   OSMemLink* curMemBlk;

   /* The smallest address that could possibly be addressable by the memory
      manager.  Smaller addresses are definitely not to memory that the
      memory manager received as part of a block allocation from the system.
   */
   void* pMinAddress;

   /* The largest address that could possibly be addressable by the memory
      manager.  Large addresses are definitely not to memory that the memory
      manager received as part of a block allocation from the system.
   */
   void* pMaxAddress;
} OSMemHeapBlocks;

typedef struct {
   OSOCTET*  data;
   OSSIZE    dataSize;
   OSSIZE    freeIdx;
} OSMemHeapStatic;

#ifdef _MEMCOMPACT

/* N.b., many architectures align on the double-word; in a 64-bit
 * architecture, the OSRawMemBlk structure will consume 24 bytes, but the
 * components consume only 20.  As a result, the final 32 bits will go
 * missing without the dummy padding element. */
typedef struct OSRawMemBlk {
   OSMemLink      link;
   OSUINT32       dummy;   /* to pad structure to 24 bytes */
   OSUINT32       elem;    /* guardCode:16, flags:8, refcnt:8 */
} OSRawMemBlk;

typedef struct OSMemHeap {
   union {
      OSMemHeapBlocks blockList;
   } u;
   OSUINT32  refCnt;
   OSUINT32  flags;

   OSMallocFunc  mallocFunc;     /* Memory allocation function pointer   */
   OSReallocFunc reallocFunc;    /* Memory reallocation function pointer */
   OSFreeFunc    memFreeFunc;    /* Memory free function pointer         */
   OSUINT16      code;           /* expected guard code for memory blocks */
} OSMemHeap;

#define pElem_flags(pElem)       (*(((OSOCTET*)pElem)+2))
#define pElem_refcnt(pElem)      (*(((OSOCTET*)pElem)+3))
#define pElem_code(pElem)        (*(OSUINT16*)pElem)

#define GET_ELEM(pMem) ((OSMemElemDescr*)(((OSOCTET*)pMem) - 4u))
#define GET_CODE(pElem) pElem_code(pElem)

#define ISSAVED(pElem)    (pElem_flags(pElem) & 1)
#define SET_SAVED(pElem)  (pElem_flags (pElem) |= 1)
#define CLEAR_SAVED(pElem) (pElem_flags (pElem) &= (OSOCTET)(~1))

#define GET_RAWMEMBLK(pMem) \
((OSRawMemBlk*) ((char*)pMem - sizeof (OSRawMemBlk)))

#define TO_RAWMEMBLK(pLink) ((OSRawMemBlk*) pLink)

#else /* _MEMCOMPACT */

/*
OSRawMemBlk: Used to represent the allocation of a raw block.  A raw block is
one which consists a single chunk of user data, in addition to some header
information.

The layout of the memory allocated for a raw block is:
  OSRawMemBlk
  user data                   memsize bytes, immediately follows OSRawMemBlk

The last 8 bytes of OSRawMemBlk are treated as an OSMemElemDescr, but
since nunits and prevOff are not used, code stands in their place.  A
consequence of this is that the first (and only) element has a beginOff of
0, since the OSMemElemDescr is colocated wth OSMemRawMemBlk.code,
whereas for OSMemBlk, the first element is after the OSMemBlk structure and
would have a beginOff of 1, if it were actually set.
*/
typedef struct OSRawMemBlk {
   OSMemLink      link;
   OSSIZE         memsize;     /* size of the user data, in bytes. This is the
                                  number of bytes which immediately follow
                                  this structure. */
   /* start of OSMemElemDescr */
   OSUINT32       code;        /* will always contain heap's code */
   OSUINT32       halfelem;    /* last half of OSMemElemDescr:
                                  beginOff, flags, refcnt */
} OSRawMemBlk;

#define TO_RAWMEMBLK(pLink) ((OSRawMemBlk*) pLink)

/*
OSMemBlk: When a block of memory is allocated to the memory manager, the first
bytes of that block will be used to hold an OSMemBlk structure.  The memory
manager will allocate portions ("memory elements") of the data that follows
that to the application as needed.

Based on rtxMemHeapAlloc, the layout of a memory block appears to be as follows:
   OSMemBlk       Descriptive structure defined below.  The code and freeLink
                  fields are used to hold a kind of dummy memory element, with
                  freeLink holding OSFreeElemLink data.
   border elem    1 unit (8 bytes) holding OSMemElemDescr data. Unlike normal
                  memory elements, this one has no user data or OSFreeElemLink
                  following it.  Its nunits is set to 0 (should be 1
                  because it takes up 1 unit?). Its prevOff is set to 0 (there
                  is no previous element).
   n mem elements Some number of memory elements as described below (see
                  OSMemElemDescr comments).  When the memory block is first
                  initialized, n==1 or n==2, depending on whether all memory is
                  immediately given to the application. If not all memory is
                  immediately given out, n==2, and the second element is a free
                  element which holds the extra memory; it may later be split or
                  allocated.
   border elem    1 unit holding OSMemElemDescr data.  I think this exists
                  because it allows us to have a simple rule: given any
                  non-border element E, there is a next (possibly border)
                  element E-FOLLOW where E-FOLLOW.prevOff gives the size of
                  E.  This information is used to move from a free link to
                  the corresponding element (it tells us how far back to go).
                  Its nunits == 0 so that it is its own next element.
*/
typedef struct OSMemBlk {
   OSMemLink      link;
   OSUINT16       firstFree;   /* index of first free elem.  This is given as
                                 a number of 8-byte units and is taken as an
                                 offset from the code field to the last unit
                                 of a memory element, which is where the
                                 OSFreeElemLink is stored. */
   OSUINT16       maxFreeElem; /* The size of the largest free element, incl.
                                  the header for the element.
                                  If unknown, OSUINT16_MAX. */
   OSUINT16       nunits;      /* size of data, in units, consisting of (see
                                  above) the 2 border elements and the n mem
                                  elements. */
   OSUINT16       nsaved;      /* num of saved elems in the block     */
   /* start of OSMemElemDescr (free element list node) */
   OSUINT32       code;        /* will always contain heap's code   */
   OSUINT32       freeLink;    /* dummy free element; nunits == 1     */
} OSMemBlk;

#define TO_MEMBLK(pLink) ((OSMemBlk*) pLink)

#define OSMemBlk_link_pnext     0
#define OSMemBlk_link_pprev     sizeof(void*)
#define OSMemBlk_firstFree      sizeof(OSMemLink)
#define OSMemBlk_maxFreeElem    sizeof(OSMemLink)+2
#define OSMemBlk_nunits         sizeof(OSMemLink)+4
#define OSMemBlk_nsaved         sizeof(OSMemLink)+6
#define OSMemBlk_code           sizeof(OSMemLink)+8
#define OSMemBlk_freeLink       sizeof(OSMemLink)+12
#define OSMemBlk_size           sizeof(OSMemLink)+16

/*
OSMemHeap: The memory heap.
curMemBlk is a pointer to the current OSMemLink (and OSMemBlk).  We use
   OSMemLink to form a doubly-linked circular list (every node has a next
   and previous; if there is only one node, it is its own next and previous
   node).
*/
typedef struct OSMemHeap {
   union {
      OSMemHeapBlocks blockList;
      OSMemHeapStatic staticMem;
   } u;
   OSUINT32      keepFreeUnits;
   OSUINT32      defBlkSize;     /* default block size in bytes */
   OSUINT32      refCnt;
   OSUINT32      flags;

   OSMallocFunc  mallocFunc;     /* Memory allocation function pointer   */
   OSReallocFunc reallocFunc;    /* Memory reallocation function pointer */
   OSFreeFunc    memFreeFunc;    /* Memory free function pointer         */
   OSUINT32      code;           /* expected guard code for memory blocks */
} OSMemHeap;


/*
OSMemElemDescr:
This is used to desribe a "memory element", a piece of memory allocated to
the application.  Memory elements are "allocated" out of the memory that is
allocated along with an OSMemBlk (which see).

The data layout is as follows:
   OSMemElemDescr (8 bytes)
   x units (8 bytes each) for application data
   0/1 unit for OSFreeElemLink.  If the memory element is free, the last
      unit (8 bytes) is used to hold the free link data.  If the memory
      element is allocated, there is no free link data. The OSFreeElemLink
      structure begins at the 4th byte in the unit.  This is important because
      it mirrors the arrangement of the last 8 bytes of OSMemBlk.  It means
      that we can locate an OSFreeElemLink structure using the same offsets
      given a pointer to OSMemBlk.code or given a pointer to the last unit of
      a memory element.


Note: the following struct describes OSMemElemDescr, but it
   cannot be used here because we need to be sure that the
   size of this structure is exactly 8 bytes. On XScale CPU, this
   structure is size 12 bytes, because of alignment of short values on
   32-bit boundary. To prevent this, use pElem_<> macros instead of
   structure fields.

typedef struct MemElemDescr {
   OSUINT16       nunits;      size = nunits * 8.  The first unit is used
                                  by this structure.  nunits includes the unit
                                  used for this structure and (if present)
                                  the unit used for free link data.
   OSUINT16       prevOff;     offset of the prev block (-prevOff * 8).  This
                                 is given as a number of 8 byte units.  It must
                                 equal the nunits field of the previous memory
                                 element's OSMemElemDescr.
   OSUINT16       beginOff;    offset/8 of the beginning of data block in MemBlk
                                  This gives the offset from MemBlk.code to
                                  this OSMemElemDescr. It is only valid for
                                  non-free elements.
   OSUINT8        flags;       allocated = 0, isFree = 1, isSaved = 2, isRaw = 4
   OSUINT8        refcnt;      reference counter to memory block
} OSMemElemDescr;



OSFreeElemLink:
This is used for tracking offsets to the next free "memory element".
This structure is stored in the low 4 bytes of the last 8 byte unit of a
memory element.

typedef struct OSFreeElemLink {
   OSUINT16       nextFree;   offset, in units, from OSMemBlk.code to the next
                                 free element's OSFreeElemLink
   OSUINT16       prevFree;   offset, in units, from OSMemBlk.code to the
                                 previous free element's OSFreeElemLink
} OSFreeElemLink;

*/

#define OSMemElemDescr_nunits    0
#define OSMemElemDescr_prevOff   2
#define OSMemElemDescr_beginOff  4
#define OSMemElemDescr_flags     6
#define OSMemElemDescr_refcnt    7

/* The following offsets are in bytes, and are from either OSMemBlk.code or
   from the last (8 byte) unit of a "memory element"
*/
#define OSFreeElemLink_nextFree  4
#define OSFreeElemLink_prevFree  6
#define OSFreeElemLink_prevOff   10  /* This is an offset to the prevOff field
                                    of the OSMemElemDescr structure which
                                    follows the OSFreeElemLink data.  That field
                                    is used to locate the beginning of the
                                    OSMemElemDescr associated with the
                                    OSFreeElemLink */

#define pElem_flags(pElem)       (*(((OSOCTET*)pElem)+6))
#define pElem_refcnt(pElem)      (*(((OSOCTET*)pElem)+7))

#define GET_OSUINT16(pElem,offset,value) { \
OSOCTET* ptr = ((OSOCTET*)pElem) + offset; \
OSCRTLMEMCPY ((void*)&value, (const void*)ptr, sizeof(OSUINT16)); \
}
#define SET_OSUINT16(pElem,offset,value) { \
OSOCTET* ptr = ((OSOCTET*)pElem) + offset; \
OSCRTLMEMCPY ((void*)ptr, (const void*)&value, sizeof(OSUINT16)); \
}
#define SET_OSUINT16L(pElem,offset,value) { \
OSUINT16 lvalue = value; SET_OSUINT16(pElem,offset,lvalue); \
}
#define ZERO_OSUINT16(pElem,offset) { \
OSOCTET* ptr = ((OSOCTET*)pElem) + offset; *ptr++ = 0; *ptr = 0; \
}

/*#define pElem_nunits(pElem)      (*((OSUINT16*)(((OSOCTET*)pElem)+0)))*/
/*#define pElem_prevOff(pElem)     (*((OSUINT16*)(((OSOCTET*)pElem)+2)))*/
/*#define pElem_beginOff(pElem)    (*((OSUINT16*)(((OSOCTET*)pElem)+4)))*/
#define sizeof_OSMemElemDescr    8
#define pElem_data(pElem)        (((OSOCTET*)pElem)+sizeof_OSMemElemDescr)

/*#define pFree_nextFree(pFree)    (*((OSUINT16*)(((OSOCTET*)pFree)+4)))*/
/*#define pFree_prevFree(pFree)    (*((OSUINT16*)(((OSOCTET*)pFree)+6)))*/
/* free elem size is a prevOff from next element */
/*#define pFree_nunits(pFree)      (*((OSUINT16*)(((OSOCTET*)pFree)+10)))*/

#define GET_ELEM(pMem) \
((OSMemElemDescr*)(((OSOCTET*)pMem) - sizeof_OSMemElemDescr))

#define GET_FREE_LINK(pElem) rtxMemHeapGetFreeLink(pElem)
/* was:
((OSFreeElemLink*)(((char*)pElem) + ((pElem_nunits (pElem) - 1) * 8u)))
*/
#define GET_FREE_ELEM(pFree) rtxMemHeapGetFreeElem(pFree)
/* was:
((OSMemElemDescr*)(((OSOCTET*)pFree) - ((pFree_nunits (pFree) - 1) * 8u)))
*/

#define GET_OFFSET_OSUINT16(pElem, pPrevElem, value) {  \
OSSIZE tmpval = (((char*)pElem - (char*)pPrevElem) >> 3u); \
OSRTASSERT (tmpval < 65535UL); value = (OSUINT16) tmpval; \
}
/* replace with GET_OFFSET_OSUINT16
#define QOFFSETOF(pElem, pPrevElem) \
((OSUINT16)(((unsigned)((char*)pElem - (char*)pPrevElem)) >> 3u))

#define OFFSETOF(pElem, pPrevElem) \
((OSUINT32)((char*)pElem - (char*)pPrevElem))
*/
#define ISFREE(pElem)     (pElem_flags(pElem) & 1)
#define SET_FREE(pElem)   (pElem_flags(pElem) = 1)
#define CLEAR_FREE(pElem) (pElem_flags(pElem) = 0)

#define ISSAVED(pElem)    (pElem_flags(pElem) & 2)
#define SET_SAVED(pElem)  (pElem_flags (pElem) |= 2)
#define CLEAR_SAVED(pElem) (pElem_flags (pElem) &= (OSOCTET)(~2))

#define ISRAW(pElem)      (pElem_flags(pElem) & 4)
#define SET_RAW(pElem)    (pElem_flags(pElem) = 4)

#define GET_MEMBLK(pElem) rtxMemHeapGetMemBlk(pElem)
/* was:
((OSMemBlk*) (((char*)pElem) - (pElem_beginOff (pElem) * 8u) - \
(sizeof (OSMemBlk) - 8u)))
*/
#define GET_RAWMEMBLK(pElem) rtxMemHeapGetRawMemBlk(pElem)
/* was:
((OSRawMemBlk*) (((char*)pElem) - (pElem_beginOff (pElem) * 8u) - \
(sizeof (OSRawMemBlk) - 8u)))
*/
/* use rtxMemHeapCheckCode instead of this:
#define GET_CODE(pElem) \
(*(OSUINT32*) ((char*) pElem - (pElem_beginOff (pElem) * 8u)))
*/
#define GETPREV(pElem) rtxMemHeapGetPrevElem(pElem)
/* was:
((OSMemElemDescr*) (((char*)pElem) - (pElem_prevOff (pElem) * 8u)))
*/
#define GETNEXT(pElem) rtxMemHeapGetNextElem(pElem)
/* was:
((OSMemElemDescr*)(((char*)pElem) + (pElem_nunits (pElem) * 8u)))
*/
#define GET_NEXT_FREE(pMemBlk,pFree) rtxMemHeapGetNextFree(pMemBlk,pFree)
/* was:
((OSFreeElemLink*) ((char*)(&pMemBlk->code) + pFree_nextFree (pFree) * 8u))
*/
#define GET_PREV_FREE(pMemBlk,pFree) rtxMemHeapGetPrevFree(pMemBlk,pFree)
/* was:
((OSFreeElemLink*) ((char*)(&pMemBlk->code) + pFree_prevFree (pFree) * 8u))
*/
#define GET_FIRST_FREE(pMemBlk) \
((OSFreeElemLink*) ((char*)(&pMemBlk->code) + pMemBlk->firstFree * 8u))

#define GET_ELEM_N(pMemBlk,n) \
((OSMemElemDescr*)((char*)(&pMemBlk->code) + n * 8u))

#define GET_FREE_LINK_0(pMemBlk) \
((OSFreeElemLink*)(&pMemBlk->code))

void _rtxMemHeapFreeElem (void** ppvMemHeap, OSMemElemDescr* pElem);

#endif /* _MEMCOMPACT */

#if defined(_WIN32) && defined(_DLL)
/* DLL runtime */

#ifndef _NO_REALLOC
#define is_realloc_available(h) \
   (h->mallocFunc == 0 || h->reallocFunc != 0)
#else /*_NO_REALLOC */
#define is_realloc_available(h) \
   (h->reallocFunc != 0)
#endif /*_NO_REALLOC */

#define _g_malloc_func(h, s) \
   ((h->mallocFunc) ? h->mallocFunc(s) : OSCRTLMALLOC(s))
#define _g_realloc_func(h, p, s) \
   ((h->reallocFunc) ? h->reallocFunc(p, s) : OSCRTLREALLOC(p, s))
#define _g_free_func(h, p) \
   ((h->memFreeFunc) ? h->memFreeFunc(p) : OSCRTLFREE(p))

#else /* defined(_WIN32) && defined(_DLL) */
/* static runtime */

#define is_realloc_available(h) (h->reallocFunc != 0)
#define _g_malloc_func(h, s) h->mallocFunc(s)
#define _g_realloc_func(h, p, s) h->reallocFunc(p, s)
#define _g_free_func(h, p) h->memFreeFunc(p)

#endif /* defined(_WIN32) && defined(_DLL) */

/* Memory debugging macros */

#ifdef _MEMDEBUG

#ifndef _MEMTRACE
#define _MEMTRACE
#endif

#define FILLFREEMEM(mem,size)   OSCRTLMEMSET (mem, 0xFE, size)
#define FILLNEWMEM(mem,size)    OSCRTLMEMSET (mem, 0xA0, size)

#define RTMEMDIAG1(pMemHeap,msg) \
   if (pMemHeap->flags & RT_MH_DIAG) rtxDiagPrint (0, msg)
#define RTMEMDIAG2(pMemHeap,msg,a) \
   if (pMemHeap->flags & RT_MH_DIAG) rtxDiagPrint (0, msg, a)
#define RTMEMDIAG3(pMemHeap,msg,a,b) \
   if (pMemHeap->flags & RT_MH_DIAG) rtxDiagPrint (0, msg, a, b)
#define RTMEMDIAG4(pMemHeap,msg,a,b,c) \
   if (pMemHeap->flags & RT_MH_DIAG) rtxDiagPrint (0, msg, a, b, c)

#define RTMEMDIAGDBG1(pMemHeap,msg) \
   if (pMemHeap->flags & RT_MH_DIAG_DEBUG) rtxDiagPrint (0, msg)
#define RTMEMDIAGDBG2(pMemHeap,msg,a) \
   if (pMemHeap->flags & RT_MH_DIAG_DEBUG) rtxDiagPrint (0, msg, a)
#define RTMEMDIAGDBG3(pMemHeap,msg,a,b) \
   if (pMemHeap->flags & RT_MH_DIAG_DEBUG) rtxDiagPrint (0, msg, a, b)
#define RTMEMDIAGDBG4(pMemHeap,msg,a,b,c) \
   if (pMemHeap->flags & RT_MH_DIAG_DEBUG) rtxDiagPrint (0, msg, a, b, c)

#else /* !_MEMDEBUG */

#define FILLFREEMEM(mem,size)
#define FILLNEWMEM(mem,size)


#define RTMEMDIAG1(pMemHeap,msg)
#define RTMEMDIAG2(pMemHeap,msg,a)
#define RTMEMDIAG3(pMemHeap,msg,a,b)
#define RTMEMDIAG4(pMemHeap,msg,a,b,c)

#define RTMEMDIAGDBG1(pMemHeap,msg)
#define RTMEMDIAGDBG2(pMemHeap,msg,a)
#define RTMEMDIAGDBG3(pMemHeap,msg,a,b)
#define RTMEMDIAGDBG4(pMemHeap,msg,a,b,c)

#endif /* _MEMDEBUG */

#ifdef _MEMTRACE
#include "rtxsrc/rtxPrintStream.h"

void _rtxMemHeapPrint (const char* name);
void _rtxMemPrintRawMemBlk (OSRawMemBlk* pRawBlk, const char* name);
void rtxMemSetDebugPrintStream(rtxPrintCallback myCallback, void* pStrmInfo);

#ifndef _MEMCOMPACT

void _rtxMemPrintMemBlk (OSMemBlk* pMemBlk, const char* name);

/* Print the free list for a memory block */
void _rtxMemPrintMemBlkFreeList (OSMemBlk* pMemBlk, const char* name);

void _rtxMemPrintMemElem
(OSMemBlk* pMemBlk, OSMemElemDescr* pElem,
 const char* elemName);

#endif /* _MEMCOMPACT */

#define TRACERAWMEMBLK(rawblk, name) \
   if (pMemHeap->flags & RT_MH_TRACE) { \
      _rtxMemPrintRawMemBlk (rawblk, name); \
   }

#define TRACEMEMBLK(memblk, name) \
   if (pMemHeap->flags & RT_MH_TRACE) { \
      _rtxMemPrintMemBlk (memblk, name); \
   }

#define TRACEMEMBLK_FREELIST(memblk, name) \
   if (pMemHeap->flags & RT_MH_TRACE_FREELIST) { \
      _rtxMemPrintMemBlkFreeList (memblk, name); \
   }

#define TRACEMEMELEM(memblk, elem, name) \
   if (pMemHeap->flags & RT_MH_TRACE) { \
      _rtxMemPrintMemElem (memblk, elem, name); \
   }

#define TRACEFREE(name) \
   if (pMemHeap->flags & RT_MH_TRACE) { \
      _rtxMemHeapPrint (name); \
   }

#else /* !_MEMTRACE */

#define TRACERAWMEMBLK(rawblk, name)
#define TRACEMEMBLK(memblk, name)
#define TRACEMEMBLK_FREELIST(memblk, name)
#define TRACEMEMELEM(memblk, elem, name)
#define TRACEFREE(name)

#endif /* _MEMTRACE */

/* Internal functions */
OSBOOL rtxMemHeapCheckCode (OSMemHeap* pHeap, const OSMemElemDescr* pElem);

/**
 * Return TRUE if the given pointer is certainly an invalid pointer, one that
 * was not allocated by the memory manager.  Returning FALSE only means the
 * pointer is not certainly invalid; it does not mean the pointer is
 * certainly valid.
 *
 * This function is for internal use only.  It is not entirely reliable as
 * it uses pointer comparisons which have undefined behavior according to the
 * C standard.  It thus makes a "best effort" and determining when a pointer
 * is invalid.
 */
OSBOOL rtxMemInvalidPtr(OSMemHeap* pHeap, void* ptr);

#ifndef _MEMCOMPACT

OSMemElemDescr* rtxMemHeapGetFreeElem (const OSFreeElemLink* pFreeLink);
OSFreeElemLink* rtxMemHeapGetFreeLink (const OSMemElemDescr* pElem);
void* rtxMemHeapGetMemBlk (const OSMemElemDescr* pElem);
void* rtxMemHeapGetRawMemBlk (const OSMemElemDescr* pElem);
OSMemElemDescr* rtxMemHeapGetNextElem (const OSMemElemDescr* pCurrElem);

OSMemElemDescr* rtxMemHeapGetPrevElem (const OSMemElemDescr* pCurrElem);
OSFreeElemLink* rtxMemHeapGetNextFree
(const OSMemBlk* pMemBlk, const OSFreeElemLink* pFree);


OSFreeElemLink* rtxMemHeapGetPrevFree
(const OSMemBlk* pMemBlk, const OSFreeElemLink* pFree);

/**
 * Zeros the data from a memory element.  This function assumes that the
 * RT_MH_ZEROONFREE flag has been set.
 *
 * This will not clear the header data.  It will clear all of the "user data",
 * which overlaps where the free link data is held for a free element;
 * therefore, DO NOT CALL THIS AFTER SETTING THE FREE LINK DATA on a newly
 * freed element! (You would corrupt the free link data.)
 *
 * @param   pElem    The element whose data should be zeroed out. */
void _rtxMemHeapZeroElem (OSMemElemDescr* pElem);

/**
 * Zeros the data from a managed memory block.  This function assumes that the
 * RT_MH_ZEROONFREE flag has been set.
 *
 * @param   pMemBlk  The memory block whose data should be zeroed out. */
void _rtxMemHeapZeroMemBlk (OSMemBlk* pMemBlk);

/**
 * Zeros the data from a raw memory block.  This function assumes that the
 * RT_MH_ZEROONFREE flag has been set. This function evaluates to a NOOP in
 * compact memory management because we do not store the size of raw blocks
 * in compact modes.
 *
 * @param   pRawBlk  The memory block whose data should be zeroed out. */
void _rtxMemHeapZeroRawBlk (OSRawMemBlk* pRawBlk);

#endif /* _MEMCOMPACT */

#endif /* __RTXMEMHEAP_HH__ */
