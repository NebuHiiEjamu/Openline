// =====================================================================
//	CPoint.h                             (C) Hotline Communications 1999
// =====================================================================
//
// Manages coordinate info


#ifndef _H_CPoint_
#define _H_CPoint_

#if PRAGMA_ONCE
	#pragma once
#endif

#include "AWTypes.h" 

HL_Begin_Namespace_BigRedH

class CPoint
{
	public:
							CPoint( )
								: mX(0), mY(0) 
								{}
							CPoint( SInt32 inX, SInt32 inY )
								: mX(inX), mY(inY) 
								{}

			// ** Accessors **
		SInt32				GetX() const
								{ return mX; }
		SInt32				GetH() const
								{ return mX; }
		void				SetX( SInt32 inX )
								{ mX = inX; }
		void				SetH( SInt32 inX )
								{ mX = inX; }
		SInt32				GetY() const
								{ return mY; }
		SInt32				GetV() const
								{ return mY; }
		void				SetY( SInt32 inY )
								{ mY = inY; }
		void				SetV( SInt32 inY )
								{ mY = inY; }

			// ** Operators **
		CPoint&				operator+=( const CPoint &inRHS )
								{ mX += inRHS.mX; mY += inRHS.mY; return *this; }
		CPoint&				operator+=( SInt32 inRHS )
								{ mX += inRHS; mY += inRHS; return *this; }
		CPoint&				operator-=( const CPoint &inRHS )
								{ mX -= inRHS.mX; mY -= inRHS.mY; return *this; }
		CPoint&				operator-=( SInt32 inRHS )
								{ mX -= inRHS; mY -= inRHS; return *this; }

		CPoint				operator- ( ) const
								{ return CPoint(-mX,-mY); }

		CPoint				operator+ (const CPoint& inRHS ) const
								{ CPoint retVal = *this; return retVal += inRHS; }
		
		CPoint				operator- (const CPoint& inRHS ) const
								{ CPoint retVal = *this; return retVal -= inRHS; }
	private:
		SInt32				mX, mY;
};

inline CPoint
operator* (double inK, const CPoint& inPoint) {
	return CPoint(static_cast<SInt32>(inK * inPoint.GetX()),
					static_cast<SInt32>(inK * inPoint.GetY()));
}

HL_End_Namespace_BigRedH
#endif	// _H_CPoint_
