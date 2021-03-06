// =====================================================================
//	CRect.h                              (C) Hotline Communications 1999
// =====================================================================
//
// Manages rectangular information for the graphics objects


#ifndef _H_CRect_
#define _H_CRect_

#if PRAGMA_ONCE
	#pragma once
#endif

#include "CPoint.h"	 
#include "CSize.h"	 

HL_Begin_Namespace_BigRedH

class CRect
{
public:
					CRect()
							: mTop(0), mLeft(0), mBottom(0), mRight(0)
						{}  

					CRect( SInt32 inTop, SInt32 inLeft, SInt32 inBottom, SInt32 inRight )
							: mTop(inTop), mLeft(inLeft),
							mBottom(inBottom), mRight(inRight)
						{}  

					CRect( CPoint inPoint, CSize inSize )
							: mTop(inPoint.GetY())
							, mLeft(inPoint.GetX())
							, mBottom(inPoint.GetY()+inSize.GetY())
							, mRight(inPoint.GetX()+inSize.GetX())
						{}  
						
					CRect( const CRect& inRect )
							: mTop(inRect.mTop)
							, mLeft(inRect.mLeft)
							, mBottom(inRect.mBottom)
							, mRight(inRect.mRight)
						{}  

		// ** Sanity Checking **
	void			AssertNormalized ();
	void 			AssertNonEmpty	 ();
	void			AssertNormalizedNonEmpty ();

		// ** Accessors **
	SInt32				GetTop() const
							{ return mTop; }
	SInt32				GetLeft() const
							{ return mLeft; }
	SInt32				GetBottom() const
							{ return mBottom; }
	SInt32				GetRight() const
							{ return mRight; }

	SInt32				GetWidth() const
							{ return mRight-mLeft; }
	SInt32				GetHeight() const
							{ return mBottom-mTop; }

	CPoint				GetTopLeft() const
							{ return CPoint(mLeft,mTop); }
	CPoint				GetBottomRight() const
							{ return CPoint(mRight,mBottom); }
	CSize				GetSize() const
							{ return CSize(mRight-mLeft,mBottom-mTop); }

		// ** Origin Management **
	void				OffsetBy( const CPoint &inPoint );
	void				GrowBy( const CPoint &inPoint );
	void				ResetOrigin();
	void				Normalize();
	
		// ** Clipping **
	void				ClipTo( const CRect& inClipRect );
	void				ClipProportional( const CRect& inClipRect,
											CRect& ioProportional);

		// ** Containment **
	bool				ContainsPoint( const CPoint &inPoint ) const;

		// ** Attributes **
	bool				IsEmpty() const
							{ return( (mTop >= mBottom) || (mLeft >= mRight) ); }

	bool				operator== (const CRect& other) const
							{ return ((mTop == other.mTop)
									&& (mLeft == other.mLeft)
									&& (mBottom == other.mBottom)
									&& (mRight == other.mRight)); }
	bool				operator!= (const CRect& other) const
							{ return ! ((*this) == other); }
private:
	SInt32				mTop, mLeft, mBottom, mRight;
};

HL_End_Namespace_BigRedH
#endif	// _H_CRect_
