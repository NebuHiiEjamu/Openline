#if MACINTOSH

/* (c)2003 Hotsprings Inc. Licensed under GPL - see LICENSE in HotlineSources diresctory */

#include "UMemory.h"
#include "UError.h"
#include "UDebug.h"

#include "UMemory(priv).h"
#include <LowMem.h>
#include <Memory.h>
#include <Errors.h>
#include <TextUtils.h>

/*
 * UMemory supports fast pool allocation for pointers.  When compiled for debugging,
 * UMemory does NOT use this allocation scheme because it tends to be more forgiving
 * and makes stress testing tools such as QC less useful.
 */
#if DEBUG
	#define USE_POOL_ALLOC		0
#else
	#define USE_POOL_ALLOC		1
#endif

// mac error handling
void _FailMacError(Int16 inMacError, const Int8 *inFile, Uint32 inLine);
inline void _CheckMacError(Int16 inMacError, const Int8 *inFile, Uint32 inLine) { if (inMacError) _FailMacError(inMacError, inFile, inLine); }
#if DEBUG
	#define FailMacError(id)		_FailMacError(id, __FILE__, __LINE__)
	#define CheckMacError(id)		_CheckMacError(id, __FILE__, __LINE__)
#else
	#define FailMacError(id)		_FailMacError(id, nil, 0)
	#define CheckMacError(id)		_CheckMacError(id, nil, 0)
#endif

// these should be odd so that UMemory::IsValid() will detect an invalid pointer
const Uint8 kUninitialized	= 0x2B;		// '+'
const Uint8 kReleased		= 0xF3;

#define CheckMemError()		CheckMacError(::MemError())

#if DEBUG
static const Int8 kInvalidHandleMsg[] = "UMemory - handle is not valid";
static const Int8 kInvalidPointerMsg[] = "UMemory - pointer is not valid";
static const Int8 kPurgedHandleMsg[] = "UMemory - inappropriate use of discarded handle";
static const Int8 kNoResizeLockedHdlMsg[] = "UMemory - cannot resize locked handle";
#endif

#if DEBUG
	#define CHECK_RESIZE_LOCKED_HDL(h)						\
		if (*((Handle)h) != nil && ::HGetState((Handle)(h)) & 0x80)	\
		{													\
			DebugBreak(kNoResizeLockedHdlMsg);				\
			Fail(errorType_Misc, error_Protocol);			\
		}
	
	#define CHECK_PURGED_HDL(h)								\
		if (*((Handle)(h)) == nil)							\
		{													\
			DebugBreak(kPurgedHandleMsg);					\
			Fail(errorType_Misc, error_Protocol);			\
		}
		
	#define CHECK_INVALID_HDL(h)							\
		if (!IsValid((THdl)h))								\
		{													\
			DebugBreak(kInvalidHandleMsg);					\
			Fail(errorType_Memory, memError_BlockInvalid);	\
		}
	
	#define CHECK_INVALID_PTR(p)							\
		if (!IsValid((TPtr)p))								\
		{													\
			DebugBreak(kInvalidPointerMsg);					\
			Fail(errorType_Memory, memError_BlockInvalid);	\
		}
#else
	#define CHECK_RESIZE_LOCKED_HDL(h)
	#define CHECK_PURGED_HDL(h)
	#define CHECK_INVALID_HDL(h)
	#define CHECK_INVALID_PTR(h)
#endif

/* -------------------------------------------------------------------------- */

void UMemory::Init()
{
	// nothing to do
}

/* -------------------------------------------------------------------------- */
#pragma mark -

#if USE_POOL_ALLOC
static mem_pool_obj __malloc_pool;
static Uint16 __malloc_pool_initted = false;
#endif

TPtr UMemory::New(Uint32 inSize)
{
	Require(inSize);
	
#if USE_POOL_ALLOC

	void *block;

	if (!__malloc_pool_initted)
	{
		_MEInitPool(&__malloc_pool);
		__malloc_pool_initted = true;
	}
	
	block = _MEPoolAlloc(&__malloc_pool, inSize);
	if (block == nil)
		Fail(errorType_Memory, memError_NotEnough);

	return (TPtr)block;

#else

	TPtr p = (TPtr)_MESysAlloc(inSize);
	if (p == nil) Fail(errorType_Memory, memError_NotEnough);

	#if DEBUG
	Fill(p, inSize, kUninitialized);
	#endif
	
	return p;
	
#endif
}

TPtr UMemory::New(const void *inData, Uint32 inSize)
{
	TPtr p = New(inSize);
	UMemory::Copy(p, inData, inSize);
	return p;
}

TPtr UMemory::NewClear(Uint32 inSize)
{
	Require(inSize);

#if USE_POOL_ALLOC

	void *block;

	if (!__malloc_pool_initted)
	{
		_MEInitPool(&__malloc_pool);
		__malloc_pool_initted = true;
	}
	
	block = _MEPoolAllocClear(&__malloc_pool, inSize);
	if (block == nil)
		Fail(errorType_Memory, memError_NotEnough);

	return (TPtr)block;

#else

	TPtr p = New(inSize);
	Fill(p, inSize, 0);
	return p;
	
#endif
}

/*
 * Dispose() functions do not propagate exceptions
 * because they are likely to be called in response
 * to an exception being thrown.
 */

void UMemory::Dispose(TPtr p)
{
#if USE_POOL_ALLOC

	if (p)
	{
		if (!__malloc_pool_initted)
		{
	#if DEBUG
			DebugBreak(kInvalidPointerMsg);
	#endif
			return;
		}
		
		_MEPoolFree(&__malloc_pool, p);
	}

#else

	if (p)
	{
	#if DEBUG
		if (!IsValid(p))
		{
			DebugBreak(kInvalidPointerMsg);
			return;
		}
		
		Fill(p, ::GetPtrSize((Ptr)p), kReleased);
	#endif
		
		::DisposePtr((Ptr)p);
	}

#endif
}

/* -------------------------------------------------------------------------- */
#pragma mark -

THdl UMemory::NewHandle(Uint32 inSize, Uint32 inOptions)
{
	Handle h;
	Int16 err;
	
	// immediately fail on ridiculous allocations (fixes bug where inSize overflows and wraps around to a small size)
	if (inSize & 0x80000000) Fail(errorType_Memory, memError_NotEnough);
	
	inSize += 4;		// space for lock count
	
	if (inOptions & 1)	// if use temp mem
		h = ::TempNewHandle(inSize, &err);
	else
		h = ::NewHandle(inSize);
	
	if (h == nil) Fail(errorType_Memory, memError_NotEnough);
	
#if DEBUG
	::HLock(h);
	Fill(*h, inSize, kUninitialized);
	::HUnlock(h);
#endif
	
	*(Uint32 *)*h = 0;	// start lock count off at 0 (unlocked)
	
	return (THdl)h;
}

THdl UMemory::NewHandle(const void *inData, Uint32 inSize, Uint32 inOptions)
{
	Handle h;
	Int16 err;
	
	// immediately fail on ridiculous allocations (fixes bug where inSize overflows and wraps around to a small size)
	if (inSize & 0x80000000) Fail(errorType_Memory, memError_NotEnough);

	if (inOptions & 1)	// if use temp mem
		h = ::TempNewHandle(inSize+4, &err);
	else
		h = ::NewHandle(inSize+4);
	
	if (h == nil) Fail(errorType_Memory, memError_NotEnough);
		
	*(Uint32 *)*h = 0;	// start lock count off at 0 (unlocked)
	
	::BlockMoveData(inData, BPTR(*h)+4, inSize);
	
	return (THdl)h;
}

THdl UMemory::NewHandleClear(Uint32 inSize, Uint32 inOptions)
{
	Handle h;
	Int16 err;
	
	// immediately fail on ridiculous allocations (fixes bug where inSize overflows and wraps around to a small size)
	if (inSize & 0x80000000) Fail(errorType_Memory, memError_NotEnough);

	inSize += 4;		// space for lock count
	
	if (inOptions & 1)	// if use temp mem
	{
		h = ::TempNewHandle(inSize, &err);
		if (h == nil) Fail(errorType_Memory, memError_NotEnough);
		
		::HLock(h);
		Fill(*h, inSize, 0);
		::HUnlock(h);
	}
	else
	{
		h = ::NewHandleClear(inSize);
		if (h == nil) Fail(errorType_Memory, memError_NotEnough);
	}
	
	return (THdl)h;
}

/*
 * Clone() creates a new handle with the same data as in the specified
 * handle.  The handle is created unlocked and non-discardable.
 */
THdl UMemory::Clone(THdl inHdl)
{
	Handle h;
	
	Require(inHdl);
	CHECK_INVALID_HDL(inHdl);
	CHECK_PURGED_HDL(inHdl);
	
	h = (Handle)inHdl;
	CheckMacError(::HandToHand(&h));
	
	*(Uint32 *)*h = 0;	// clear lock count
		
	return (THdl)h;
}

/*
 * Dispose() deallocates the specified handle, releasing all
 * memory belonging to it.  After this call, the handle is not
 * valid and must not be used.  You should not dispose a 
 * locked handle.
 */
void UMemory::Dispose(THdl inHdl)
{
	if (inHdl)
	{
	
#if DEBUG
		if (!IsValid(inHdl))
		{
			DebugBreak(kInvalidHandleMsg);
			return;
		}
		if (::HGetState((Handle)inHdl) & 0x20)	// this will not detect purged resource handles
		{
			DebugBreak("UMemory - cannot dispose mac resource handle");
			return;
		}
#endif
		
		if (**(Uint32 **)inHdl)
		{
			DebugBreak("UMemory - cannot dispose locked handle!");
			return;
		}
		
#if DEBUG
		if (*(Handle)inHdl != nil)	// don't zap purged handle!!
		{
			::HLock((Handle)inHdl);
			Fill(*(Handle)inHdl, ::GetHandleSize((Handle)inHdl), kReleased);
			::HUnlock((Handle)inHdl);
		}
#endif

		::DisposeHandle((Handle)inHdl);
	}
}

/* -------------------------------------------------------------------------- */
#pragma mark -

/*
 * SetSize() changes the amount of memory allocated to the specified
 * handle.  As much of the existing data as will fit is preserved.
 * If expanding, all of the data will be preserved, and the contents
 * of the extra bytes at the end is undefined.  If shrinking, the
 * data will be truncated.  Note that you cannot use SetSize() on a
 * locked handle.  Do not use SetSize() on a discarded handle - use
 * Reallocate() instead.
 */
void UMemory::SetSize(THdl inHdl, Uint32 inSize)
{
	Require(inHdl);
	
	// immediately fail on ridiculous allocations (fixes bug where inSize overflows and wraps around to a small size)
	if (inSize & 0x80000000) Fail(errorType_Memory, memError_NotEnough);

	inSize += 4;	// space for lock count
	
#if DEBUG
	CHECK_INVALID_HDL(inHdl);
	CHECK_PURGED_HDL(inHdl);
	CHECK_RESIZE_LOCKED_HDL(inHdl);
	
	Uint32 origSize = ::GetHandleSize((Handle)inHdl);
	if (inSize == origSize) return;
#endif

	::SetHandleSize((Handle)inHdl, inSize);
	CheckMemError();

#if DEBUG
	if (inSize > origSize)
	{
		::HLock((Handle)inHdl);
		Fill(BPTR(*(Handle)inHdl) + origSize, inSize - origSize, kUninitialized);
		::HUnlock((Handle)inHdl);
	}
#endif
}

/*
 * GetSize() returns the size in bytes of the specified handle.
 * If the handle is discarded, GetSize() returns 0.
 */
Uint32 UMemory::GetSize(THdl inHdl)
{
	Require(inHdl);
	CHECK_INVALID_HDL(inHdl);
	
	// return 0 if discarded
	if (*(Handle)inHdl == nil) return 0;

	Uint32 s = ::GetHandleSize((Handle)inHdl);	
	// minus space for lock count
	return s == 0 ? 0 : s - 4;
}

Uint32 UMemory::Grow(THdl inHdl, Int32 inDelta)
{
	Int32 s = (Int32)GetSize(inHdl) + inDelta;
	if (s < 0) s = 0;
	SetSize(inHdl, s);
	return s;
}

/*
 * Reallocate() changes the size of a block of memory while preserving its
 * contents.  If necessary, Reallocate() copies the contents to another
 * block of memory.  If the pointer is nil, a new block of the specified
 * size is allocated.  If the size is 0, then the pointer is disposed of
 * and set to nil.
 */
TPtr UMemory::Reallocate(TPtr inPtr, Uint32 inSize)
{
	if (inPtr == nil)
	{
		if (inSize != 0)
			inPtr = New(inSize);
	}
	else if (inSize == 0)
	{
		Dispose(inPtr);
		inPtr = nil;
	}
	else
	{
#if USE_POOL_ALLOC

		void *block;

		if (!__malloc_pool_initted)
		{
			_MEInitPool(&__malloc_pool);
			__malloc_pool_initted = true;
		}
		
		block = _MEPoolRealloc(&__malloc_pool, inPtr, inSize);
		if (block == nil)
			Fail(errorType_Memory, memError_NotEnough);

		inPtr = (TPtr)block;
		
#else
		
		CHECK_INVALID_PTR(inPtr);
		
		Uint32 s = ::GetPtrSize((Ptr)inPtr);
		
		if (s != inSize)
		{
			Ptr np = ::NewPtr(inSize);
			if (np == nil) Fail(errorType_Memory, memError_NotEnough);
			
			if (s > inSize) s = inSize;
			::BlockMoveData(inPtr, np, s);
			
			::DisposePtr((Ptr)inPtr);
			inPtr = (TPtr)np;
		}
		
#endif
	}
	
	return inPtr;
}

/*
 * Reallocate() changes the amount of memory allocated to the
 * specified handle.  Unlike SetSize(), the contents of the
 * handle is lost.  Reallocate() is normally used to reallocate
 * discarded handles.  Note that the handle is reallocated
 * unlocked and non-discardable (even if it was before).
 * Also, you should not attempt to reallocate a locked handle.
 */
void UMemory::Reallocate(THdl inHdl, Uint32 inSize)
{
	Require(inHdl);
	CHECK_INVALID_HDL(inHdl);
	CHECK_RESIZE_LOCKED_HDL(inHdl);
	
	// immediately fail on ridiculous allocations (fixes bug where inSize overflows and wraps around to a small size)
	if (inSize & 0x80000000) Fail(errorType_Memory, memError_NotEnough);

	inSize += 4;	// space for lock count
	
	::ReallocateHandle((Handle)inHdl, inSize);
	CheckMemError();

#if DEBUG
	::HLock((Handle)inHdl);
	Fill(*(Handle)inHdl, inSize, kUninitialized);
	::HUnlock((Handle)inHdl);
#endif

	**(Uint32 **)inHdl = 0;	// start lock count off at 0 (unlocked)
}

void UMemory::ReallocateClear(THdl inHdl, Uint32 inSize)
{
	Require(inHdl);
	CHECK_INVALID_HDL(inHdl);
	CHECK_RESIZE_LOCKED_HDL(inHdl);
	
	// immediately fail on ridiculous allocations (fixes bug where inSize overflows and wraps around to a small size)
	if (inSize & 0x80000000) Fail(errorType_Memory, memError_NotEnough);

	inSize += 4;	// space for lock count
	
	::ReallocateHandle((Handle)inHdl, inSize);
	CheckMemError();

	::HLock((Handle)inHdl);
	Fill(*(Handle)inHdl, inSize, 0);
	::HUnlock((Handle)inHdl);
}

/* -------------------------------------------------------------------------- */
#pragma mark -

/*
 * Lock() locks the specified handle and returns a pointer to the
 * data in that handle.  If the handle is discarded, Lock() returns
 * nil.  You must ALWAYS check for a nil result when locking discardable
 * handles because until the handle is locked, it can be discarded
 * at ANY time.  A lock count is maintained for each handle so you
 * can nest calls to Lock() and Unlock() (every call to Lock() should
 * be matched with a call to Unlock(), except when Lock() returns nil).
 */
Uint8 *UMemory::Lock(THdl inHdl)
{
	Require(inHdl);
	CHECK_INVALID_HDL(inHdl);
	
	if (*(Handle)inHdl == nil)
		return nil;
	
	Uint32 *lc = *(Uint32 **)inHdl;
	
	(*lc)++;
	
	if (*lc == 1) ::HLock((Handle)inHdl);
	
	return BPTR(*(Handle)inHdl) + 4;	// skip past lock count
}

/*
 * Unlock() decrements the lock count for the specified handle.
 * If the count reaches zero, the handle is unlocked and any
 * pointers obtained from locking the handle become invalid.
 * You should not attempt to unlock a discarded handle.
 */
void UMemory::Unlock(THdl inHdl)
{
	/*
	 * This is a clean-up func, so DON'T throw exceptions!
	 */
	
	if (inHdl)
	{
#if DEBUG
		if (!IsValid(inHdl))
		{
			DebugBreak(kInvalidHandleMsg);
			return;
		}
#endif

		if (*(Handle)inHdl == nil)		// if purged
		{
			DebugBreak("UMemory - attempt to unlock discarded handle");
			return;
		}
		
		Uint32 *lc = *(Uint32 **)inHdl;
		
		if (*lc == 0)	// if lock count is zero
		{
			DebugBreak("UMemory - Unlock() called too many times!");
			return;
		}
		
		(*lc)--;
		
		if (*lc == 0) ::HUnlock((Handle)inHdl);
	}
}

/*
 * SetDiscardable() makes the specified handle discardable.  Discardable
 * unlocked handles can be deallocated by the system at ANY time. However,
 * unlike Dispose(), the THdl value remains valid.  Lock() will return nil
 * for discarded handles.
 */
void UMemory::SetDiscardable(THdl inHdl)
{
	Require(inHdl);
	CHECK_INVALID_HDL(inHdl);
	
	if (*(Handle)inHdl != nil)		// if not purged
		::HPurge((Handle)inHdl);	// mark purgeable
}

/*
 * ClearDiscardable() makes the specified handle non-discardable, and returns
 * whether or not the handle is discarded (remember that an unlocked discardable
 * handle can be discarded at ANY time).  An IsDiscarded() function is NOT provided
 * because it would be unsafe.  For example, the handle could be discarded immediately
 * after IsDiscarded() returned true.  Instead, ClearDiscardable() returns whether
 * or not the handle is discarded, and this is safe because it makes the handle
 * non-discardable before checking if it is discarded.
 */
bool UMemory::ClearDiscardable(THdl inHdl)
{
	Require(inHdl);
	CHECK_INVALID_HDL(inHdl);
	
	// check if already purged (purgeable flag is automatically cleared when purged)
	if (*(Handle)inHdl == nil)
		return true;
	
	::HNoPurge((Handle)inHdl);
	return false;
}

/* -------------------------------------------------------------------------- */
#pragma mark -

void UMemory::Set(THdl inHdl, const void *inData, Uint32 inSize)
{
	Require(inHdl);
	CHECK_INVALID_HDL(inHdl);
	CHECK_RESIZE_LOCKED_HDL(inHdl);
		
	::ReallocateHandle((Handle)inHdl, inSize+4);
	CheckMemError();
	
	Uint8 *p = (Uint8 *)*(Handle)inHdl;
	::BlockMoveData(inData, p+4, inSize);
	
	*(Uint32 *)p = 0;	// clear lock count
}

void UMemory::Set(THdl ioHdl, THdl inSource)
{
	Require(ioHdl);
	CHECK_INVALID_HDL(ioHdl);
	CHECK_RESIZE_LOCKED_HDL(ioHdl);
	CHECK_INVALID_HDL(inSource);
	CHECK_PURGED_HDL(inSource);
	
	/*
	 * Note that ReallocateHandle() is faster than SetHandleSize() because
	 * it doesn't have to preserve the contents of the handle.
	 */
	Uint32 s = ::GetHandleSize((Handle)inSource);
	::ReallocateHandle((Handle)ioHdl, s);
	CheckMemError();
	
	Uint8 *p = (Uint8 *)*(Handle)ioHdl;
	::BlockMoveData(*(Handle)inSource, p, s);
	*(Uint32 *)p = 0;	// clear lock count
}

void UMemory::Clear(THdl inHdl)
{
	Require(inHdl);
	CHECK_INVALID_HDL(inHdl);
	CHECK_PURGED_HDL(inHdl);
	
	Uint32 s = ::GetHandleSize((Handle)inHdl);
	
	char saveState = ::HGetState((Handle)inHdl);
	::HLock((Handle)inHdl);
	
	Fill(BPTR(*(Handle)inHdl)+4, s-4, 0);
	
	::HSetState((Handle)inHdl, saveState);
}

void UMemory::Append(THdl inHdl, const void *p, Uint32 s)
{
	Require(inHdl);
	
	if (p && s)
	{
		CHECK_INVALID_HDL(inHdl);
		CHECK_PURGED_HDL(inHdl);
		CHECK_RESIZE_LOCKED_HDL(inHdl);

		CheckMacError(::PtrAndHand((Ptr)p, (Handle)inHdl, s));
	}
}

Uint32 UMemory::Insert(THdl inHdl, Uint32 inOffset, const void *inData, Uint32 inDataSize)
{
	Require(inHdl);
	
	CHECK_INVALID_HDL(inHdl);
	CHECK_PURGED_HDL(inHdl);
	CHECK_RESIZE_LOCKED_HDL(inHdl);

	inOffset += 4;		// adjust for lock count

	if (inDataSize)
	{
		Uint8 *p;
		Uint32 s;
		

		s = ::GetHandleSize((Handle)inHdl);
		if (inOffset > s) inOffset = s;

		::SetHandleSize((Handle)inHdl, s + inDataSize);
		CheckMemError();
		
		p = ((Uint8 *)*(Handle)inHdl) + inOffset;
		::BlockMoveData(p, p + inDataSize, s - inOffset);
		
		if (inData)
			::BlockMoveData(inData, p, inDataSize);
	}
	
	return inOffset - 4;	// adjust for lock count
}

Uint32 UMemory::Remove(THdl inHdl, Uint32 inOffset, Uint32 inSize)
{
	inOffset += 4;		// adjust for lock count

	Require(inHdl && inOffset <= ::GetHandleSize((Handle)inHdl));
	CHECK_INVALID_HDL(inHdl);
	CHECK_PURGED_HDL(inHdl);
	CHECK_RESIZE_LOCKED_HDL(inHdl);

	if (inSize)
	{
		inOffset = ::Munger((Handle)inHdl, inOffset, nil, inSize, "\p", 0);
		CheckMemError();
	}
	
	return inOffset - 4;	// adjust for lock count
}

Uint32 UMemory::Replace(THdl ioHdl, Uint32 inOffset, Uint32 inExistingSize, const void *inData, Uint32 inDataSize)
{
	Require(ioHdl);
	CHECK_INVALID_HDL(ioHdl);
	CHECK_PURGED_HDL(ioHdl);
	CHECK_RESIZE_LOCKED_HDL(ioHdl);

	inOffset += 4;		// adjust for lock count

	Uint32 oldSize = ::GetHandleSize((Handle)ioHdl);
	Uint8 *p;

	// bring offset into range
	if (inOffset > oldSize)
		inOffset = oldSize;
	
	// bring existing size into range
	if (inOffset + inExistingSize > oldSize)
		inExistingSize = oldSize - inOffset;
	
	// move existing data if necessary
	if (inExistingSize != inDataSize)
	{
		if (inDataSize > inExistingSize)
		{
			// move following data forward
			::SetHandleSize((Handle)ioHdl, oldSize + (inDataSize - inExistingSize));
			CheckMemError();
			p = BPTR(*(Handle)ioHdl) + inOffset;
			::BlockMoveData(p + inExistingSize, p + inDataSize, oldSize - (inOffset + inExistingSize));
		}
		else
		{
			// move following data backward
			p = BPTR(*(Handle)ioHdl) + inOffset;
			::BlockMoveData(p + inExistingSize, p + inDataSize, oldSize - (inOffset + inExistingSize));
			::SetHandleSize((Handle)ioHdl, oldSize - (inExistingSize - inDataSize));
			CheckMemError();
		}
	}
	
	// write the new data
	if (inData && inDataSize)
		::BlockMoveData(inData, BPTR(*(Handle)ioHdl) + inOffset, inDataSize);
	
	// return offset at which replace occured
	return inOffset - 4;	// adjust for lock count
}

#if 0	// use the one in UMemory.cp instead - it's faster
void UMemory::SearchAndReplaceAll(THdl inHdl, const void *inSearchData, Uint32 inSearchSize, const void *inReplaceData, Uint32 inReplaceSize)
{
	Require(inHdl);
	CHECK_INVALID_HDL(inHdl);
	CHECK_PURGED_HDL(inHdl);
	CHECK_RESIZE_LOCKED_HDL(inHdl);

	Int32 ofs = 4;	// skip past lock count
	
	for(;;)
	{
		ofs = ::Munger((Handle)inHdl, ofs, inSearchData, inSearchSize, inReplaceData, inReplaceSize);
		CheckMemError();
		if (ofs == -1) break;
	}
}
#endif

/*
 * Read() copies up to <inMaxSize> bytes from offset <inOffset> within <inHdl>
 * to <outData>, and returns the number of bytes copied.  Returns 0 if <inHdl>
 * is nil.  Returns 0 if the offset is invalid.
 */
Uint32 UMemory::Read(THdl inHdl, Uint32 inOffset, void *outData, Uint32 inMaxSize)
{
	if (inHdl == nil) return 0;
	CHECK_INVALID_HDL(inHdl);
	CHECK_PURGED_HDL(inHdl);
	
	inOffset += 4;	// adjust for lock count
	
	Uint32 s = ::GetHandleSize((Handle)inHdl);
	if (inOffset >= s) return 0;
	if (inOffset + inMaxSize > s) inMaxSize = s - inOffset;
	
	::BlockMoveData(BPTR(*(Handle)inHdl) + inOffset, outData, inMaxSize);
	
	return inMaxSize;
}

/*
 * Write() copies up to <inDataSize> bytes from <inData> to offset <inOffset>
 * within <inHdl>, and returns the number of bytes copied (which might be less
 * than <inDataSize> because Write() does not attempt to resize the handle).
 * Returns 0 if the offset is invalid.
 */
Uint32 UMemory::Write(THdl inHdl, Uint32 inOffset, const void *inData, Uint32 inDataSize)
{
	Require(inHdl);
	CHECK_INVALID_HDL(inHdl);
	CHECK_PURGED_HDL(inHdl);
	
	inOffset += 4;	// adjust for lock count
	
	Uint32 s = ::GetHandleSize((Handle)inHdl);
	if (inOffset >= s) return 0;
	if (inOffset + inDataSize > s) inDataSize = s - inOffset;

	::BlockMoveData(inData, BPTR(*(Handle)inHdl) + inOffset, inDataSize);
	
	return inDataSize;
}

/*
 * ReadLong() returns the 4 bytes at byte offset <inOffset> within <inHdl>.
 * Returns 0 if <inHdl> is nil.  Returns 0 if the offset is invalid.
 */
Uint32 UMemory::ReadLong(THdl inHdl, Uint32 inOffset)
{
	if (inHdl == nil) return 0;
	CHECK_INVALID_HDL(inHdl);
	CHECK_PURGED_HDL(inHdl);
	
	inOffset += 4;	// adjust for lock count
	
	if (inOffset + 4 > ::GetHandleSize((Handle)inHdl)) return 0;
	
	return *(Uint32 *)(BPTR(*(Handle)inHdl) + inOffset);
}

/*
 * WriteLong() sets the 4 bytes at byte offset <inOffset> within <inHdl>
 * to <inVal>.  Does nothing if the offset is invalid.
 */
void UMemory::WriteLong(THdl inHdl, Uint32 inOffset, Uint32 inVal)
{
	Require(inHdl);
	CHECK_INVALID_HDL(inHdl);
	CHECK_PURGED_HDL(inHdl);
	
	inOffset += 4;	// adjust for lock count
	
	if (inOffset + 4 <= ::GetHandleSize((Handle)inHdl))
		*(Uint32 *)(BPTR(*(Handle)inHdl) + inOffset) = inVal;
}

/*
 * ReadWord() returns the 2 bytes at byte offset <inOffset> within <inHdl>.
 * Returns 0 if <inHdl> is nil.  Returns 0 if the offset is invalid.
 */
Uint16 UMemory::ReadWord(THdl inHdl, Uint32 inOffset)
{
	if (inHdl == nil) return 0;
	CHECK_INVALID_HDL(inHdl);
	CHECK_PURGED_HDL(inHdl);
	
	inOffset += 4;	// adjust for lock count
	
	if (inOffset + 2 > ::GetHandleSize((Handle)inHdl)) return 0;
	
	return *(Uint16 *)(BPTR(*(Handle)inHdl) + inOffset);
}

/*
 * WriteWord() sets the 2 bytes at byte offset <inOffset> within <inHdl>
 * to <inVal>.  Does nothing if the offset is invalid.
 */
void UMemory::WriteWord(THdl inHdl, Uint32 inOffset, Uint16 inVal)
{
	Require(inHdl);
	CHECK_INVALID_HDL(inHdl);
	CHECK_PURGED_HDL(inHdl);
	
	inOffset += 4;	// adjust for lock count
	
	if (inOffset + 2 <= ::GetHandleSize((Handle)inHdl))
		*(Uint16 *)(BPTR(*(Handle)inHdl) + inOffset) = inVal;
}

/* -------------------------------------------------------------------------- */
#pragma mark -

#define IS_EVEN(n)		((((Uint32)(n)) & 1) == 0)

/*
 * IsValid(THdl) returns whether or not the specified handle appears valid.
 * Note that invalid handles may appear valid.  Discarded handles are valid.
 */
bool UMemory::IsValid(THdl inHdl)
{
	return inHdl && IS_EVEN(inHdl) && IS_EVEN(*(Handle)inHdl);
}

/*
 * IsValid(TPtr) returns whether or not the specified pointer appears valid.
 * Note that invalid pointers may appear valid. 
 */
bool UMemory::IsValid(TPtr p)
{
#if USE_POOL_ALLOC

	return p && __malloc_pool_initted && _MEPoolValid(&__malloc_pool, p);

#else

	// must be non-zero and even
	return p && IS_EVEN(p);
	
#endif
}

Uint32 UMemory::GetUsedSize()
{
#if TARGET_API_MAC_CARBON
	return 0;
#else
	// used size is total heap size minus free size
	return ((Uint8 *)::GetApplLimit() - (Uint8 *)::ApplicationZone()) - (Uint32)::FreeMem();
#endif
}

/*
 * IsAvailable() returns whether or not there is enough memory to allocate
 * a block (pointer or handle) of the specified size.  If IsAvailable() returns
 * true, a subsequent allocation of the specified size will probably be
 * successful (but don't assume it will be).
 */
bool UMemory::IsAvailable(Uint32 inSize)
{
	Handle inHdl = ::NewHandle(inSize);
	
	if (inHdl == nil)
		return false;
	else
	{
		::DisposeHandle(inHdl);
		return true;
	}
}

/* -------------------------------------------------------------------------- */
#pragma mark -

Uint32 UMemory::Copy(void *outDest, const void *inSource, Uint32 inSize)
{
	::BlockMoveData(inSource, outDest, inSize);
	return inSize;
}

void UMemory::Insert(void *inData, Uint32 inDataSize, Uint32 inOffset, const void *inNewData, Uint32 inNewDataSize)
{
	Require(inOffset <= inDataSize);
	::BlockMoveData((Ptr)(CPTR(inData) + inOffset), (Ptr)(CPTR(inData) + inOffset + inNewDataSize), inDataSize - inOffset);
	if (inNewData) ::BlockMoveData(inNewData, CPTR(inData) + inOffset, inNewDataSize);
}

/* -------------------------------------------------------------------------- */
#pragma mark -

void *_MESysAlloc(mem_size size)
{
	return NewPtr(size);
}

void _MESysFree(void *ptr)
{
	DisposePtr((Ptr)ptr);
}


#endif /* MACINTOSH */
