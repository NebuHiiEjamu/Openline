/* (c)2003 Hotsprings Inc. Licensed under GPL - see LICENSE in HotlineSources diresctory */

#include "AW.h"
#include <math.h>
#include "CRect.h"


HL_Begin_Namespace_BigRedH
// ---------------------------------------------------------------------
//  AssertNormalized                                            [public]
// ---------------------------------------------------------------------
// Assures we have a normalized CRect, by failing asserts if it isn't.

void
CRect::AssertNormalized () 
{
	ASSERT(mLeft < mRight);
	ASSERT(mTop < mBottom);
}


// ---------------------------------------------------------------------
//  AssertNonEmpty                                              [public]
// ---------------------------------------------------------------------
// Assures we have a non-empty CRect, by failing asserts if it isn't.

void
CRect::AssertNonEmpty () 
{
	ASSERT(mLeft != mRight);
	ASSERT(mTop != mBottom);
}

// ---------------------------------------------------------------------
//  AssertNormalizedNonEmpty                                    [public]
// ---------------------------------------------------------------------
// Assures we have a non-empty normalized CRect, by failing asserts 
// if it isn't.

void
CRect::AssertNormalizedNonEmpty () 
{
	AssertNormalized();
	AssertNonEmpty();
}

// ---------------------------------------------------------------------
//  OffsetBy                                                    [public]
// ---------------------------------------------------------------------
// Offsets the rect

void
CRect::OffsetBy( const CPoint &inPoint )
{
	mTop += inPoint.GetY();
	mLeft += inPoint.GetX();
	mBottom += inPoint.GetY();
	mRight += inPoint.GetX();;
}


// ---------------------------------------------------------------------
//  GrowBy                                                      [public]
// ---------------------------------------------------------------------
// Grows the rect to the bottom left

void
CRect::GrowBy( const CPoint &inPoint )
{
	mBottom += inPoint.GetY();
	mRight += inPoint.GetX();;
}


// ---------------------------------------------------------------------
//  ResetOrigin                                                 [public]
// ---------------------------------------------------------------------
// Sets the origin of the rect to 0,0

void
CRect::ResetOrigin()
{
	mBottom -= mTop;
	mTop = 0;
	mRight -= mLeft;
	mLeft = 0;
}


// ---------------------------------------------------------------------
//  Normalize                                                   [public]
// ---------------------------------------------------------------------
// Sanity check for the rect

void
CRect::Normalize()
{
}


// ---------------------------------------------------------------------
//  ContainsPoint                                               [public]
// ---------------------------------------------------------------------
// Returns whether the rect contains the point

bool
CRect::ContainsPoint( const CPoint &inPoint ) const
{
	return ( (mTop <= inPoint.GetY()) && (inPoint.GetY() < mBottom) &&
			 (mLeft <= inPoint.GetX()) && (inPoint.GetX() < mRight) );
}


// ---------------------------------------------------------------------
//  ClipTo		                                                [public]
// ---------------------------------------------------------------------
// Clips this rect to inClipRect

void
CRect::ClipTo( const CRect& inClipRect )
{
	if (GetLeft() < inClipRect.GetLeft()) {
		int deltaX = GetLeft() - inClipRect.GetLeft();
		
		OffsetBy(CPoint(-deltaX,0));
		GrowBy(CPoint(deltaX,0));
	}
	if (GetTop() < inClipRect.GetTop()) {
		int deltaY = GetTop() - inClipRect.GetTop();

		OffsetBy(CPoint(0,-deltaY));
		GrowBy(CPoint(0,deltaY));
	}
	if (GetRight() > inClipRect.GetRight()) {
		int deltaX = inClipRect.GetRight() - GetRight();
		GrowBy(CPoint(deltaX,0));
	}
	if (GetBottom() > inClipRect.GetBottom()) {
		int deltaY = inClipRect.GetBottom() - GetBottom();
		GrowBy(CPoint(0,deltaY));
	}
}


// ---------------------------------------------------------------------
//  ClipProportional                                            [public]
// ---------------------------------------------------------------------
// Clips this rect to inClipRect, making scaled changes to ioProportional

void
CRect::ClipProportional( const CRect& inClipRect, CRect& ioProportional)
{
	float xScaleSize = static_cast<float>(ioProportional.GetWidth()) /
			static_cast<float>(GetWidth());
			
	float yScaleSize = static_cast<float>(ioProportional.GetHeight()) /
			static_cast<float>(GetHeight());
	
	CRect tempClip = *this;
	tempClip.ClipTo(inClipRect);
	
	int propLeft = 	ioProportional.GetLeft() + 
					static_cast<int>(floor((tempClip.GetLeft() - GetLeft())
						* xScaleSize + 0.5));
	int propRight = ioProportional.GetRight() +	
					static_cast<int>(floor((tempClip.GetRight() - GetRight())
						* xScaleSize + 0.5));
	int propTop = ioProportional.GetTop() +
					static_cast<int>(floor((tempClip.GetTop() - GetTop())
						* yScaleSize + 0.5));
	int propBottom = ioProportional.GetBottom() +
						static_cast<int>(floor((tempClip.GetBottom() - GetBottom())
							* yScaleSize + 0.5));
	
	*this = tempClip;
	ioProportional = CRect(propTop, propLeft, propBottom, propRight);
}

HL_End_Namespace_BigRedH
