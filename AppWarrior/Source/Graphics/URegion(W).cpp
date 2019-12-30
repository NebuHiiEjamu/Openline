#if WIN32

/* (c)2003 Hotsprings Inc. Licensed under GPL - see LICENSE in HotlineSources diresctory */

#include "URegion.h"

/* -------------------------------------------------------------------------- */

void URegion::Init()
{
	// nothing to do
}

/* -------------------------------------------------------------------------- */
#pragma mark -

TRegion URegion::New()
{
	HRGN rgn = ::CreateRectRgn(0,0,0,0);
	if (rgn == nil) Fail(errorType_Memory, memError_NotEnough);
	return (TRegion)rgn;
}

TRegion URegion::New(TRegion inRgn)
{
	HRGN rgn = ::CreateRectRgn(0,0,0,0);
	if (rgn == nil) Fail(errorType_Memory, memError_NotEnough);

	if (::CombineRgn(rgn, (HRGN)inRgn, NULL, RGN_COPY) == ERROR)
	{
		::DeleteObject(rgn);
		Fail(errorType_Memory, memError_NotEnough);
	}
	
	return (TRegion)rgn;
}

TRegion URegion::New(const SRect& inRect)
{
	HRGN rgn = ::CreateRectRgnIndirect((RECT *)&inRect);
	if (rgn == nil) Fail(errorType_Memory, memError_NotEnough);
	return (TRegion)rgn;
}

TRegion URegion::New(const SRoundRect& inRoundRect)
{
	HRGN rgn = ::CreateRoundRectRgn(inRoundRect.left, inRoundRect.top, inRoundRect.right, inRoundRect.bottom, inRoundRect.ovalWidth, inRoundRect.ovalHeight);
	if (rgn == nil) Fail(errorType_Memory, memError_NotEnough);
	return (TRegion)rgn;
}

void URegion::Dispose(TRegion inRgn)
{
	if (inRgn) ::DeleteObject((HRGN)inRgn);
}

TRegion URegion::Clone(TRegion inRgn)
{
	HRGN rgn = ::CreateRectRgn(0,0,0,0);
	if (rgn == nil) Fail(errorType_Memory, memError_NotEnough);

	if (::CombineRgn(rgn, (HRGN)inRgn, NULL, RGN_COPY) == ERROR)
	{
		::DeleteObject(rgn);
		Fail(errorType_Memory, memError_NotEnough);
	}
	
	return (TRegion)rgn;
}

/* -------------------------------------------------------------------------- */
#pragma mark -

void URegion::GetBounds(TRegion inRgn, SRect& outBounds)
{
	::GetRgnBox((HRGN)inRgn, (LPRECT)&outBounds);
}

void URegion::SetBounds(TRegion inRgn, const SRect& inBounds)
{
	RECT r;
	::GetRgnBox((HRGN)inRgn, &r);
	::OffsetRgn((HRGN)inRgn, inBounds.left - r.left, inBounds.top - r.top);
	
	// ***** can't change height/width of win region ?
}

void URegion::Move(TRegion inRgn, Int32 inHorizDelta, Int32 inVertDelta)
{
	::OffsetRgn((HRGN)inRgn, inHorizDelta, inVertDelta);
}

void URegion::Enlarge(TRegion inRgn, Int32 inHorizDelta, Int32 inVertDelta)
{
	// ***** can't change height/width of win region ?
	#pragma unused(inRgn, inHorizDelta, inVertDelta)
	Fail(errorType_Misc, error_Unimplemented);
}

/* -------------------------------------------------------------------------- */
#pragma mark -

void URegion::GetDifference(TRegion inRgnA, TRegion inRgnB, TRegion outResult)
{
	if (::CombineRgn((HRGN)outResult, (HRGN)inRgnA, (HRGN)inRgnB, RGN_DIFF) == ERROR)
		Fail(errorType_Memory, memError_NotEnough);
}

void URegion::GetIntersection(TRegion inRgnA, TRegion inRgnB, TRegion outResult)
{
	if (::CombineRgn((HRGN)outResult, (HRGN)inRgnA, (HRGN)inRgnB, RGN_AND) == ERROR)
		Fail(errorType_Memory, memError_NotEnough);
}

void URegion::GetUnion(TRegion inRgnA, TRegion inRgnB, TRegion outResult)
{
	if (::CombineRgn((HRGN)outResult, (HRGN)inRgnA, (HRGN)inRgnB, RGN_OR) == ERROR)
		Fail(errorType_Memory, memError_NotEnough);
}

void URegion::GetInvertUnion(TRegion inRgnA, TRegion inRgnB, TRegion outResult)
{
	if (::CombineRgn((HRGN)outResult, (HRGN)inRgnA, (HRGN)inRgnB, RGN_XOR) == ERROR)
		Fail(errorType_Memory, memError_NotEnough);
}

/* -------------------------------------------------------------------------- */
#pragma mark -

void URegion::Add(TRegion ioDest, TRegion inRgn)
{
	if (::CombineRgn((HRGN)ioDest, (HRGN)ioDest, (HRGN)inRgn, RGN_OR) == ERROR)
		Fail(errorType_Memory, memError_NotEnough);
}

void URegion::AddRect(TRegion ioDest, const SRect& inRect)
{
	HRGN temp = ::CreateRectRgnIndirect((RECT *)&inRect);
	if (temp == nil) Fail(errorType_Memory, memError_NotEnough);

	if (::CombineRgn((HRGN)ioDest, (HRGN)ioDest, temp, RGN_OR) == ERROR)
	{
		::DeleteObject(temp);
		Fail(errorType_Memory, memError_NotEnough);
	}
	
	::DeleteObject(temp);
}

void URegion::AddOval(TRegion ioDest, const SRect& inOval)
{
	HRGN temp = ::CreateEllipticRgnIndirect((RECT *)&inOval);
	if (temp == nil) Fail(errorType_Memory, memError_NotEnough);

	if (::CombineRgn((HRGN)ioDest, (HRGN)ioDest, temp, RGN_OR) == ERROR)
	{
		::DeleteObject(temp);
		Fail(errorType_Memory, memError_NotEnough);
	}
	
	::DeleteObject(temp);
}

void URegion::AddRoundRect(TRegion ioDest, const SRoundRect& inRoundRect)
{
	HRGN temp = ::CreateRoundRectRgn(inRoundRect.left, inRoundRect.top, inRoundRect.right, inRoundRect.bottom, inRoundRect.ovalWidth, inRoundRect.ovalHeight);
	if (temp == nil) Fail(errorType_Memory, memError_NotEnough);

	if (::CombineRgn((HRGN)ioDest, (HRGN)ioDest, temp, RGN_OR) == ERROR)
	{
		::DeleteObject(temp);
		Fail(errorType_Memory, memError_NotEnough);
	}
	
	::DeleteObject(temp);
}

void URegion::Subtract(TRegion ioDest, TRegion inRgn)
{
	if (::CombineRgn((HRGN)ioDest, (HRGN)ioDest, (HRGN)inRgn, RGN_DIFF) == ERROR)
		Fail(errorType_Memory, memError_NotEnough);
}

void URegion::SubtractRect(TRegion ioDest, const SRect& inRect)
{
	HRGN temp = ::CreateRectRgnIndirect((RECT *)&inRect);
	if (temp == nil) Fail(errorType_Memory, memError_NotEnough);

	if (::CombineRgn((HRGN)ioDest, (HRGN)ioDest, temp, RGN_DIFF) == ERROR)
	{
		::DeleteObject(temp);
		Fail(errorType_Memory, memError_NotEnough);
	}
	
	::DeleteObject(temp);
}

void URegion::SubtractOval(TRegion ioDest, const SRect& inOval)
{
	HRGN temp = ::CreateEllipticRgnIndirect((RECT *)&inOval);
	if (temp == nil) Fail(errorType_Memory, memError_NotEnough);

	if (::CombineRgn((HRGN)ioDest, (HRGN)ioDest, temp, RGN_DIFF) == ERROR)
	{
		::DeleteObject(temp);
		Fail(errorType_Memory, memError_NotEnough);
	}
	
	::DeleteObject(temp);
}

void URegion::SubtractRoundRect(TRegion ioDest, const SRoundRect& inRoundRect)
{
	HRGN temp = ::CreateRoundRectRgn(inRoundRect.left, inRoundRect.top, inRoundRect.right, inRoundRect.bottom, inRoundRect.ovalWidth, inRoundRect.ovalHeight);
	if (temp == nil) Fail(errorType_Memory, memError_NotEnough);

	if (::CombineRgn((HRGN)ioDest, (HRGN)ioDest, temp, RGN_DIFF) == ERROR)
	{
		::DeleteObject(temp);
		Fail(errorType_Memory, memError_NotEnough);
	}
	
	::DeleteObject(temp);
}

void URegion::Invert(TRegion ioDest, TRegion inRgn)
{
	if (::CombineRgn((HRGN)ioDest, (HRGN)ioDest, (HRGN)inRgn, RGN_XOR) == ERROR)
		Fail(errorType_Memory, memError_NotEnough);
}

void URegion::InvertRect(TRegion ioDest, const SRect& inRect)
{
	HRGN temp = ::CreateRectRgnIndirect((RECT *)&inRect);
	if (temp == nil) Fail(errorType_Memory, memError_NotEnough);

	if (::CombineRgn((HRGN)ioDest, (HRGN)ioDest, temp, RGN_XOR) == ERROR)
	{
		::DeleteObject(temp);
		Fail(errorType_Memory, memError_NotEnough);
	}
	
	::DeleteObject(temp);
}

void URegion::InvertOval(TRegion ioDest, const SRect& inOval)
{
	HRGN temp = ::CreateEllipticRgnIndirect((RECT *)&inOval);
	if (temp == nil) Fail(errorType_Memory, memError_NotEnough);

	if (::CombineRgn((HRGN)ioDest, (HRGN)ioDest, temp, RGN_XOR) == ERROR)
	{
		::DeleteObject(temp);
		Fail(errorType_Memory, memError_NotEnough);
	}
	
	::DeleteObject(temp);
}

void URegion::InvertRoundRect(TRegion ioDest, const SRoundRect& inRoundRect)
{
	HRGN temp = ::CreateRoundRectRgn(inRoundRect.left, inRoundRect.top, inRoundRect.right, inRoundRect.bottom, inRoundRect.ovalWidth, inRoundRect.ovalHeight);
	if (temp == nil) Fail(errorType_Memory, memError_NotEnough);

	if (::CombineRgn((HRGN)ioDest, (HRGN)ioDest, temp, RGN_XOR) == ERROR)
	{
		::DeleteObject(temp);
		Fail(errorType_Memory, memError_NotEnough);
	}
	
	::DeleteObject(temp);
}

void URegion::Set(TRegion ioDest, TRegion inRgn)
{
	if (::CombineRgn((HRGN)ioDest, (HRGN)inRgn, NULL, RGN_COPY) == ERROR)
		Fail(errorType_Memory, memError_NotEnough);
}

void URegion::SetRect(TRegion ioDest, const SRect& inRect)
{
	HRGN temp = ::CreateRectRgnIndirect((RECT *)&inRect);
	if (temp == nil) Fail(errorType_Memory, memError_NotEnough);

	if (::CombineRgn((HRGN)ioDest, temp, NULL, RGN_COPY) == ERROR)
	{
		::DeleteObject(temp);
		Fail(errorType_Memory, memError_NotEnough);
	}
	
	::DeleteObject(temp);
}

void URegion::SetOval(TRegion ioDest, const SRect& inOval)
{
	HRGN temp = ::CreateEllipticRgnIndirect((RECT *)&inOval);
	if (temp == nil) Fail(errorType_Memory, memError_NotEnough);

	if (::CombineRgn((HRGN)ioDest, temp, NULL, RGN_COPY) == ERROR)
	{
		::DeleteObject(temp);
		Fail(errorType_Memory, memError_NotEnough);
	}
	
	::DeleteObject(temp);
}

void URegion::SetRoundRect(TRegion ioDest, const SRoundRect& inRoundRect)
{
	HRGN temp = ::CreateRoundRectRgn(inRoundRect.left, inRoundRect.top, inRoundRect.right, inRoundRect.bottom, inRoundRect.ovalWidth, inRoundRect.ovalHeight);
	if (temp == nil) Fail(errorType_Memory, memError_NotEnough);

	if (::CombineRgn((HRGN)ioDest, temp, NULL, RGN_COPY) == ERROR)
	{
		::DeleteObject(temp);
		Fail(errorType_Memory, memError_NotEnough);
	}
	
	::DeleteObject(temp);
}

void URegion::Intersect(TRegion ioDest, TRegion inRgn)
{
	if (::CombineRgn((HRGN)ioDest, (HRGN)ioDest, (HRGN)inRgn, RGN_AND) == ERROR)
		Fail(errorType_Memory, memError_NotEnough);
}

void URegion::IntersectRect(TRegion ioDest, const SRect& inRect)
{
	HRGN temp = ::CreateRectRgnIndirect((RECT *)&inRect);
	if (temp == nil) Fail(errorType_Memory, memError_NotEnough);

	if (::CombineRgn((HRGN)ioDest, (HRGN)ioDest, temp, RGN_AND) == ERROR)
	{
		::DeleteObject(temp);
		Fail(errorType_Memory, memError_NotEnough);
	}
	
	::DeleteObject(temp);
}

void URegion::IntersectOval(TRegion ioDest, const SRect& inOval)
{
	HRGN temp = ::CreateEllipticRgnIndirect((RECT *)&inOval);
	if (temp == nil) Fail(errorType_Memory, memError_NotEnough);

	if (::CombineRgn((HRGN)ioDest, (HRGN)ioDest, temp, RGN_AND) == ERROR)
	{
		::DeleteObject(temp);
		Fail(errorType_Memory, memError_NotEnough);
	}
	
	::DeleteObject(temp);
}

void URegion::IntersectRoundRect(TRegion ioDest, const SRoundRect& inRoundRect)
{
	HRGN temp = ::CreateRoundRectRgn(inRoundRect.left, inRoundRect.top, inRoundRect.right, inRoundRect.bottom, inRoundRect.ovalWidth, inRoundRect.ovalHeight);
	if (temp == nil) Fail(errorType_Memory, memError_NotEnough);

	if (::CombineRgn((HRGN)ioDest, (HRGN)ioDest, temp, RGN_AND) == ERROR)
	{
		::DeleteObject(temp);
		Fail(errorType_Memory, memError_NotEnough);
	}
	
	::DeleteObject(temp);
}

/* -------------------------------------------------------------------------- */
#pragma mark -

bool URegion::IsEqual(TRegion inRgnA, TRegion inRgnB)
{
	// the ==true is necessary because EqualRgn can return ERROR
	return ::EqualRgn((HRGN)inRgnA, (HRGN)inRgnB) == true;
}

bool URegion::IsPointWithin(TRegion inRgn, const SPoint& inPt)
{
	return ::PtInRegion((HRGN)inRgn, inPt.x, inPt.y);
}

void URegion::SetEmpty(TRegion inRgn)
{
	HRGN temp = ::CreateRectRgn(0,0,0,0);
	if (temp == nil) Fail(errorType_Memory, memError_NotEnough);

	if (::CombineRgn((HRGN)inRgn, temp, NULL, RGN_COPY) == ERROR)
	{
		::DeleteObject(temp);
		Fail(errorType_Memory, memError_NotEnough);
	}
	
	::DeleteObject(temp);
}

bool URegion::IsEmpty(TRegion inRgn)
{
	SRect r;
	::GetRgnBox((HRGN)inRgn, (RECT *)&r);
	return r.IsEmpty();
}

void URegion::SetOutline(TRegion inRgn, Int16 inWidth, Int16 inHeight)
{
	// **** can't change height/width of win region ?
	#pragma unused(inRgn, inWidth, inHeight)
	
	//Fail(errorType_Misc, error_Unimplemented);
}





#endif
