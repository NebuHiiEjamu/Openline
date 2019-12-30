#if MACINTOSH

/* (c)2003 Hotsprings Inc. Licensed under GPL - see LICENSE in HotlineSources diresctory */
/*
A region can describe any graphical shape. It has no color or pattern, it is
simply a shape.

The outline of a region is mathematically defined and infinitely thin, and it
separates the pixel image into two groups of pixels: those within the region
and those outside it. 
*/

#include "URegion.h"
#include "UMemory.h"

#include <QuickDraw.h>

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

/*
 * Global Variables
 */

static struct {
	TRegion tempRgn;
	TRegion pooledRgn1, pooledRgn2, pooledRgn3;
} _REData = { nil, nil, nil, nil };

/*
 * Macros
 */

#define CheckQDError()		CheckMacError(::QDError())

/* -------------------------------------------------------------------------------- */

// it is optional as to whether you call this
void URegion::Init()
{
	if (_REData.tempRgn == nil)
	{
		_REData.tempRgn = (TRegion)::NewRgn();
		CheckQDError();
	}
}

TRegion URegion::New()
{
	TRegion rgn = nil;
	Init();

	if (_REData.pooledRgn1)
	{
		if (*(Handle)_REData.pooledRgn1 == nil)
		{
			::DisposeRgn((RgnHandle)_REData.pooledRgn1);
			_REData.pooledRgn1 = nil;
		}
		else
		{
			rgn = _REData.pooledRgn1;
			_REData.pooledRgn1 = nil;
			goto emptyAndReturn;
		}
	}
	
	if (_REData.pooledRgn2)
	{
		if (*(Handle)_REData.pooledRgn2 == nil)
		{
			::DisposeRgn((RgnHandle)_REData.pooledRgn2);
			_REData.pooledRgn2 = nil;
		}
		else
		{
			rgn = _REData.pooledRgn2;
			_REData.pooledRgn2 = nil;
			goto emptyAndReturn;
		}
	}

	if (_REData.pooledRgn3)
	{
		if (*(Handle)_REData.pooledRgn3 == nil)
		{
			::DisposeRgn((RgnHandle)_REData.pooledRgn3);
			_REData.pooledRgn3 = nil;
		}
		else
		{
			rgn = _REData.pooledRgn3;
			_REData.pooledRgn3 = nil;
			goto emptyAndReturn;
		}
	}
	
newAndReturn:
	rgn = (TRegion)::NewRgn();
	CheckQDError();
	return rgn;
	
emptyAndReturn:
	HNoPurge((Handle)rgn);
	try
	{
		::SetEmptyRgn((RgnHandle)rgn);
		CheckQDError();
	}
	catch(...)
	{
		::DisposeRgn((RgnHandle)rgn);
		throw;
	}
	
	return rgn;
}

TRegion URegion::New(TRegion inRgn)
{
	return Clone(inRgn);
}

TRegion URegion::New(const SRect& inRect)
{
	TRegion rgn = New();
	try
	{
		SetRect(rgn, inRect);
	}
	catch(...)
	{
		Dispose(rgn);
		throw;
	}
	return rgn;
}

TRegion URegion::New(const SRoundRect& inRoundRect)
{
	TRegion rgn = New();
	try
	{
		SetRoundRect(rgn, inRoundRect);
	}
	catch(...)
	{
		Dispose(rgn);
		throw;
	}
	return rgn;
}

void URegion::Dispose(TRegion rgn)
{
	if (rgn)
	{
	#if DEBUG
		if (!UMemory::IsValid((THdl)rgn))
		{
			DebugBreak("URegion - attempt to dispose invalid region");
			return;
		}
	#endif
	
		if (_REData.pooledRgn1 == nil)
		{
			_REData.pooledRgn1 = rgn;
			HPurge((Handle)rgn);
		}
		else if (_REData.pooledRgn2 == nil)
		{
			_REData.pooledRgn2 = rgn;
			HPurge((Handle)rgn);
		}
		else if (_REData.pooledRgn3 == nil)
		{
			_REData.pooledRgn3 = rgn;
			HPurge((Handle)rgn);
		}
		else
			::DisposeRgn((RgnHandle)rgn);
	}
}

TRegion URegion::Clone(TRegion inRgn)
{
	TRegion rgn = New();
	if (inRgn)
	{
		::CopyRgn((RgnHandle)inRgn, (RgnHandle)rgn);
		Int16 err = QDError();
		if (err)
		{
			::DisposeRgn((RgnHandle)rgn);
			FailMacError(err);
		}
	}
	return rgn;
}

/* -------------------------------------------------------------------------------- */
#pragma mark -

void URegion::GetBounds(TRegion inRgn, SRect& outBounds)
{
	if (inRgn == nil)
	{
		outBounds.SetEmpty();
		return;
	}
	
#if TARGET_API_MAC_CARBON
	Rect rgnRect;
	::GetRegionBounds((RgnHandle)inRgn, &rgnRect);
#else
	Rect rgnRect = (**(RgnHandle)inRgn).rgnBBox;
#endif
	
	outBounds.top = rgnRect.top;
	outBounds.left = rgnRect.left;
	outBounds.bottom = rgnRect.bottom;
	outBounds.right = rgnRect.right;
}

void URegion::SetBounds(TRegion inRgn, const SRect& inBounds)
{
#if TARGET_API_MAC_CARBON
	Rect srcRect;
	::GetRegionBounds((RgnHandle)inRgn, &srcRect);
#else
	Rect srcRect = (**(RgnHandle)inRgn).rgnBBox;
#endif

	Rect destRect = { inBounds.top, inBounds.left, inBounds.bottom, inBounds.right };
	
	::MapRgn((RgnHandle)inRgn, &srcRect, &destRect);
	CheckQDError();
}


void URegion::Enlarge(TRegion inRgn, Int32 inHorizDelta, Int32 inVertDelta)
{
	::InsetRgn((RgnHandle)inRgn, -inHorizDelta, -inVertDelta);
	CheckQDError();
}

void URegion::Move(TRegion inRgn, Int32 inHorizDelta, Int32 inVertDelta)
{
	::OffsetRgn((RgnHandle)inRgn, inHorizDelta, inVertDelta);
	CheckQDError();
}

void URegion::SetEmpty(TRegion inRgn)
{
	::SetEmptyRgn((RgnHandle)inRgn);
	CheckQDError();
}

bool URegion::IsEmpty(TRegion inRgn)
{
	if (inRgn == nil) return true;
	return ::EmptyRgn((RgnHandle)inRgn);
}

bool URegion::IsEqual(TRegion inRgnA, TRegion inRgnB)
{
	return ::EqualRgn((RgnHandle)inRgnA, (RgnHandle)inRgnB);
}

bool URegion::IsPointWithin(TRegion inRgn, const SPoint& inPt)
{
	if (inRgn == nil) 
		return false;

#if TARGET_API_MAC_CARBON
	Rect r;
	::GetRegionBounds((RgnHandle)inRgn, &r);
#else
	Rect r = (**(RgnHandle)inRgn).rgnBBox;
#endif
	
	Point pt = { inPt.v, inPt.h };
	if (pt.v < r.top || pt.v >= r.bottom || pt.h < r.left || pt.h >= r.right)
		return false;
	
	return PtInRgn(pt, (RgnHandle)inRgn);
}

void URegion::SetOutline(TRegion rgn, Int16 width, Int16 height)
{
	Init();
	
	Set(_REData.tempRgn, rgn);
	Enlarge(_REData.tempRgn, -width, -height);
	Subtract(rgn, _REData.tempRgn);
	
	SetEmpty(_REData.tempRgn);
}

/* -------------------------------------------------------------------------------- */
#pragma mark -

/*
 * GetDifference() calculates the difference between <inRgnA> and <inRgnB>
 * and stores the result in <outResult> (which can be either of the source
 * regions).  The difference is the area at which one region does not overlap
 * the other.
 */
void URegion::GetDifference(TRegion inRgnA, TRegion inRgnB, TRegion outResult)
{
	::DiffRgn((RgnHandle)inRgnA, (RgnHandle)inRgnB, (RgnHandle)outResult);
	CheckQDError();
}

/*
 * GetIntersection() calculates the intersection of <inRgnA> and <inRgnB>
 * and stores the result in <outResult> (which can be either of the source
 * regions).  The intersection is the area where the regions overlap.
 */
void URegion::GetIntersection(TRegion inRgnA, TRegion inRgnB, TRegion outResult)
{
	::SectRgn((RgnHandle)inRgnA, (RgnHandle)inRgnB, (RgnHandle)outResult);
	CheckQDError();
}

/*
 * GetUnion() calculates the union of <inRgnA> and <inRgnB> and stores
 * the result in <outResult> (which can be either of the source regions).
 * The union is the combined area of both regions.
 */
void URegion::GetUnion(TRegion inRgnA, TRegion inRgnB, TRegion outResult)
{
	::UnionRgn((RgnHandle)inRgnA, (RgnHandle)inRgnB, (RgnHandle)outResult);
	CheckQDError();
}

/*
 * GetInvertUnion() calculates the union of <inRgnA> and <inRgnB> except
 * for any portions that overlap, and stores the result in <outResult>
 * (which can be either of the source regions).
 */
void URegion::GetInvertUnion(TRegion inRgnA, TRegion inRgnB, TRegion outResult)
{
	::XorRgn((RgnHandle)inRgnA, (RgnHandle)inRgnB, (RgnHandle)outResult);
	CheckQDError();
}

/* -------------------------------------------------------------------------------- */
#pragma mark -

/*
-- Implementation Note --

The mac toolbox has OpenRgn() and CloseRgn() calls for "recording" regions.
URegion provides no equivalent because an Open/Close syntax is bad for
multi-tasking (eg, nesting and yielding while open), and the operation
of functions while a region is open is unclear.  The most convincing argument
however, is that of speed.  Consider this code snippet:
 
	OpenRgn();
	FrameRect(&r1);
	FrameRect(&r2);
	CloseRgn(rgn);
	
Consider also this code snippet:
	
	RectRgn(rgn, &r1);
	RectRgn(gWorkRgn, &r2);
	XorRgn(rgn, gWorkRgn, rgn);
	
Both of the above examples produce exactly the same results (that is, the
shape and size of the resultant regions are identical).  However, the latter
example is 40% faster (on a PM7500) than the one using OpenRgn/CloseRgn.
Thus, it is clearly obvious that there is no need for an Open/Close syntax.

Adam Hinkley (March 96)
*/

void URegion::Add(TRegion ioDest, TRegion inRgn)
{
	GetUnion(ioDest, inRgn, ioDest);
}

void URegion::AddRect(TRegion ioDest, const SRect& inRect)
{
	Init();
	SetRect(_REData.tempRgn, inRect);
	Add(ioDest, _REData.tempRgn);
}

void URegion::AddOval(TRegion ioDest, const SRect& inOval)
{
	Init();
	SetOval(_REData.tempRgn, inOval);
	Add(ioDest, _REData.tempRgn);
}

void URegion::AddRoundRect(TRegion ioDest, const SRoundRect& inRoundRect)
{
	Init();
	SetRoundRect(_REData.tempRgn, inRoundRect);
	Add(ioDest, _REData.tempRgn);
}

/* -------------------------------------------------------------------------------- */
#pragma mark -

void URegion::Subtract(TRegion ioDest, TRegion inRgn)
{
	GetDifference(ioDest, inRgn, ioDest);
}

void URegion::SubtractRect(TRegion ioDest, const SRect& inRect)
{
	Init();
	SetRect(_REData.tempRgn, inRect);
	Subtract(ioDest, _REData.tempRgn);
}

void URegion::SubtractOval(TRegion ioDest, const SRect& inOval)
{
	Init();
	SetOval(_REData.tempRgn, inOval);
	Subtract(ioDest, _REData.tempRgn);
}

void URegion::SubtractRoundRect(TRegion ioDest, const SRoundRect& inRoundRect)
{
	Init();
	SetRoundRect(_REData.tempRgn, inRoundRect);
	Subtract(ioDest, _REData.tempRgn);
}

/* -------------------------------------------------------------------------------- */
#pragma mark -

void URegion::Invert(TRegion ioDest, TRegion inRgn)
{
	GetInvertUnion(ioDest, inRgn, ioDest);
}

void URegion::InvertRect(TRegion ioDest, const SRect& inRect)
{
	Init();
	SetRect(_REData.tempRgn, inRect);
	Invert(ioDest, _REData.tempRgn);
}

void URegion::InvertOval(TRegion ioDest, const SRect& inOval)
{
	Init();
	SetOval(_REData.tempRgn, inOval);
	Invert(ioDest, _REData.tempRgn);
}

void URegion::InvertRoundRect(TRegion ioDest, const SRoundRect& inRoundRect)
{
	Init();
	SetRoundRect(_REData.tempRgn, inRoundRect);
	Invert(ioDest, _REData.tempRgn);
}

/* -------------------------------------------------------------------------------- */
#pragma mark -

void URegion::Intersect(TRegion ioDest, TRegion inRgn)
{
	GetIntersection(ioDest, inRgn, ioDest);
}

void URegion::IntersectRect(TRegion ioDest, const SRect& inRect)
{
	Init();
	SetRect(_REData.tempRgn, inRect);
	Intersect(ioDest, _REData.tempRgn);
}

void URegion::IntersectOval(TRegion ioDest, const SRect& inOval)
{
	Init();
	SetOval(_REData.tempRgn, inOval);
	Intersect(ioDest, _REData.tempRgn);
}

void URegion::IntersectRoundRect(TRegion ioDest, const SRoundRect& inRoundRect)
{
	Init();
	SetRoundRect(_REData.tempRgn, inRoundRect);
	Intersect(ioDest, _REData.tempRgn);
}

/* -------------------------------------------------------------------------------- */
#pragma mark -

void URegion::Set(TRegion ioDest, TRegion inRgn)
{
	if (inRgn == nil)
	{
		SetEmptyRgn((RgnHandle)ioDest);
		CheckQDError();
	}
	else
	{
		::CopyRgn((RgnHandle)inRgn, (RgnHandle)ioDest);
		CheckQDError();
	}
}

void URegion::SetRect(TRegion ioDest, const SRect& inRect)
{
	Rect r;
	
	r.top = inRect.top > max_Int16 ? max_Int16 : inRect.top;
	r.left = inRect.left > max_Int16 ? max_Int16 : inRect.left;
	r.bottom = inRect.bottom > max_Int16 ? max_Int16 : inRect.bottom;
	r.right = inRect.right > max_Int16 ? max_Int16 : inRect.right;
	
	::RectRgn((RgnHandle)ioDest, &r);
}

void URegion::SetOval(TRegion ioDest, const SRect& inBounds)
{
	Rect r = { inBounds.top, inBounds.left, inBounds.bottom, inBounds.right };
	Int16 err;
	
	::OpenRgn();
	CheckQDError();
	
	FrameOval(&r);
	err = QDError();
	
	::CloseRgn((RgnHandle)ioDest);
	CheckMacError(err);
	CheckQDError();
}

void URegion::SetRoundRect(TRegion ioDest, const SRoundRect& inRoundRect)
{
	Rect r = { inRoundRect.top, inRoundRect.left, inRoundRect.bottom, inRoundRect.right };
	Int16 err;
	
	::OpenRgn();
	CheckQDError();
	
	FrameRoundRect(&r, inRoundRect.ovalWidth, inRoundRect.ovalHeight);
	err = QDError();
	
	::CloseRgn((RgnHandle)ioDest);
	CheckMacError(err);
	CheckQDError();
}


#endif /* MACINTOSH */
