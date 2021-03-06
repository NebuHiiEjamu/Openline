/* (c)2003 Hotsprings Inc. Licensed under GPL - see LICENSE in HotlineSources diresctory */

#pragma once
#include "typedefs.h"

class SPoint;
class SRect;
class SRoundRect;

typedef class TRegionObj *TRegion;

class URegion
{
	public:
		// new and dispose
		static void Init();
		static TRegion New();
		static TRegion New(TRegion inRgn);
		static TRegion New(const SRect& inRect);
		static TRegion New(const SRoundRect& inRoundRect);
		static void Dispose(TRegion inRgn);
		static TRegion Clone(TRegion inRgn);

		// bounds
		static void GetBounds(TRegion inRgn, SRect& outBounds);
		static void SetBounds(TRegion inRgn, const SRect& inBounds);
		static void Move(TRegion inRgn, Int32 inHorizDelta, Int32 inVertDelta);
		static void Enlarge(TRegion inRgn, Int32 inHorizDelta, Int32 inVertDelta);
		
		// arithmetic operations
		static void GetDifference(TRegion inRgnA, TRegion inRgnB, TRegion outResult);
		static void GetIntersection(TRegion inRgnA, TRegion inRgnB, TRegion outResult);
		static void GetUnion(TRegion inRgnA, TRegion inRgnB, TRegion outResult);
		static void GetInvertUnion(TRegion inRgnA, TRegion inRgnB, TRegion outResult);
		
		// add operations (OR |)
		static void Add(TRegion ioDest, TRegion inRgn);
		static void AddRect(TRegion ioDest, const SRect& inRect);
		static void AddOval(TRegion ioDest, const SRect& inOval);
		static void AddRoundRect(TRegion ioDest, const SRoundRect& inRoundRect);
		
		// subtract operations (-)
		static void Subtract(TRegion ioDest, TRegion inRgn);
		static void SubtractRect(TRegion ioDest, const SRect& inRect);
		static void SubtractOval(TRegion ioDest, const SRect& inOval);
		static void SubtractRoundRect(TRegion ioDest, const SRoundRect& inRoundRect);
		
		// invert operations (XOR ^)
		static void Invert(TRegion ioDest, TRegion inRgn);
		static void InvertRect(TRegion ioDest, const SRect& inRect);
		static void InvertOval(TRegion ioDest, const SRect& inOval);
		static void InvertRoundRect(TRegion ioDest, const SRoundRect& inRoundRect);
		
		// set operations (=)
		static void Set(TRegion ioDest, TRegion inRgn);
		static void SetRect(TRegion ioDest, const SRect& inRect);
		static void SetOval(TRegion ioDest, const SRect& inOval);
		static void SetRoundRect(TRegion ioDest, const SRoundRect& inRoundRect);
		
		// intersect operations (AND &)
		static void Intersect(TRegion ioDest, TRegion inRgn);
		static void IntersectRect(TRegion ioDest, const SRect& inRect);
		static void IntersectOval(TRegion ioDest, const SRect& inOval);
		static void IntersectRoundRect(TRegion ioDest, const SRoundRect& inRoundRect);
		
		// misc
		static bool IsEqual(TRegion inRgnA, TRegion inRgnB);
		static bool IsPointWithin(TRegion inRgn, const SPoint& inPt);
		static void SetEmpty(TRegion inRgn);
		static bool IsEmpty(TRegion inRgn);
		static void SetOutline(TRegion inRgn, Int16 inWidth, Int16 inHeight);
};

/*
 * URegion Object Interface
 */

class TRegionObj
{
	public:
		void GetBounds(SRect& outBounds)						{	URegion::GetBounds(this, outBounds);				}
		void SetBounds(const SRect& inBounds)					{	URegion::SetBounds(this, inBounds);					}
		void Move(Int32 inHorizDelta, Int32 inVertDelta)		{	URegion::Move(this, inHorizDelta, inVertDelta);		}
		void Enlarge(Int32 inHorizDelta, Int32 inVertDelta)		{	URegion::Enlarge(this, inHorizDelta, inVertDelta);	}
		
		void Add(TRegion inRgn)									{	URegion::Add(this, inRgn);							}
		void AddRect(const SRect& inRect)						{	URegion::AddRect(this, inRect);						}
		void AddOval(const SRect& inOval)						{	URegion::AddOval(this, inOval);						}
		void AddRoundRect(const SRoundRect& inRoundRect)		{	URegion::AddRoundRect(this, inRoundRect);			}
		
		void Subtract(TRegion inRgn)							{	URegion::Subtract(this, inRgn);						}
		void SubtractRect(const SRect& inRect)					{	URegion::SubtractRect(this, inRect);				}
		void SubtractOval(const SRect& inOval)					{	URegion::SubtractOval(this, inOval);				}
		void SubtractRoundRect(const SRoundRect& inRoundRect)	{	URegion::SubtractRoundRect(this, inRoundRect);		}
		
		void Invert(TRegion inRgn)								{	URegion::Invert(this, inRgn);						}
		void InvertRect(const SRect& inRect)					{	URegion::InvertRect(this, inRect);					}
		void InvertOval(const SRect& inOval)					{	URegion::InvertOval(this, inOval);					}
		void InvertRoundRect(const SRoundRect& inRoundRect)		{	URegion::InvertRoundRect(this, inRoundRect);		}
		
		void Set(TRegion inRgn)									{	URegion::Set(this, inRgn);							}
		void SetRect(const SRect& inRect)						{	URegion::SetRect(this, inRect);						}
		void SetOval(const SRect& inOval)						{	URegion::SetOval(this, inOval);						}
		void SetRoundRect(const SRoundRect& inRoundRect)		{	URegion::SetRoundRect(this, inRoundRect);			}
		
		void Intersect(TRegion inRgn)							{	URegion::Intersect(this, inRgn);					}
		void IntersectRect(const SRect& inRect)					{	URegion::IntersectRect(this, inRect);				}
		void IntersectOval(const SRect& inOval)					{	URegion::IntersectOval(this, inOval);				}
		void IntersectRoundRect(const SRoundRect& inRoundRect)	{	URegion::IntersectRoundRect(this, inRoundRect);		}
		
		bool IsPointWithin(const SPoint& inPt)					{	return URegion::IsPointWithin(this, inPt);			}
		void SetEmpty()											{	URegion::SetEmpty(this);							}
		bool IsEmpty()											{	return URegion::IsEmpty(this);						}
		void SetOutline(Int16 inWidth, Int16 inHeight)			{	URegion::SetOutline(this, inWidth, inHeight);		}
		
		void operator delete(void *p)							{	URegion::Dispose((TRegion)p);						}
	protected:
		TRegionObj() {}		// force creation via URegion
};

/*
 * Stack TRegion
 */

class StRegion
{
	public:
		StRegion()									{	mRgn = URegion::New();								}
		StRegion(const SRect& inRect)				{	mRgn = URegion::New(inRect);						}
		StRegion(const SRoundRect& inRoundRect)		{	mRgn = URegion::New(inRoundRect);					}
		~StRegion()									{	URegion::Dispose(mRgn);								}
		operator TRegion() const					{	return mRgn;										}
		TRegionObj *operator->() const				{	return mRgn;										}

		StRegion& operator=(TRegion inRgn)			{	URegion::Set(mRgn, inRgn); return *this;			}
		StRegion& operator=(const SRect& inRect)	{	URegion::SetRect(mRgn, inRect); return *this;		}
		StRegion& operator+=(TRegion inRgn)			{	URegion::Add(mRgn, inRgn); return *this;			}
		StRegion& operator+=(const SRect& inRect)	{	URegion::AddRect(mRgn, inRect); return *this;		}
		StRegion& operator-=(TRegion inRgn)			{	URegion::Subtract(mRgn, inRgn); return *this;		}
		StRegion& operator-=(const SRect& inRect)	{	URegion::SubtractRect(mRgn, inRect); return *this;	}
		StRegion& operator|=(TRegion inRgn)			{	URegion::Add(mRgn, inRgn); return *this;			}
		StRegion& operator|=(const SRect& inRect)	{	URegion::AddRect(mRgn, inRect); return *this;		}
		StRegion& operator&=(TRegion inRgn)			{	URegion::Intersect(mRgn, inRgn); return *this;		}
		StRegion& operator&=(const SRect& inRect)	{	URegion::IntersectRect(mRgn, inRect); return *this;	}
		StRegion& operator^=(TRegion inRgn)			{	URegion::Invert(mRgn, inRgn); return *this;			}
		StRegion& operator^=(const SRect& inRect)	{	URegion::InvertRect(mRgn, inRect); return *this;	}
		bool operator==(TRegion inRgn) const		{	return URegion::IsEqual(mRgn, inRgn);				}
		bool operator!=(TRegion inRgn) const		{	return !URegion::IsEqual(mRgn, inRgn);				}

	private:
		TRegion mRgn;
};





