/* (c)2003 Hotsprings Inc. Licensed under GPL - see LICENSE in HotlineSources diresctory */

#include "CPtrList.h"
#include "UMemory.h"

/* -------------------------------------------------------------------------- */

CVoidPtrList::CVoidPtrList()
{	
	mData = nil; mCount = mMaxCount = 0; 
}

CVoidPtrList::~CVoidPtrList()
{	
	UMemory::Dispose((TPtr)mData); 
}


void CVoidPtrList::SetData(TPtr inPtr, Uint32 inCount, Uint32 inMaxCount)
{
	UMemory::Dispose((TPtr)mData);
	mData = (void **)inPtr;
	mCount = inCount;
	mMaxCount = inMaxCount;
}

// returns index (1-based) of newly added item
Uint32 CVoidPtrList::AddItem(void *inPtr)
{
	Require(inPtr);
	
	if (mData)
	{
		if (mCount == mMaxCount)
		{
			Uint32 n = RoundUp8(mMaxCount + 8);
			mData = (void **)UMemory::Reallocate((TPtr)mData, n * sizeof(void*));
			mMaxCount = n;
		}
		
		mData[mCount] = inPtr;
		mCount++;
	}
	else
	{
		mData = (void **)UMemory::New(32);
		*mData = inPtr;
		mCount = 1;
		mMaxCount = 32 / sizeof(void *);
	}
	
	return mCount;
}

// preallocate memory for <inCount> items (does nothing if the space is already allocated)
void CVoidPtrList::Preallocate(Uint32 inCount)
{
	if (inCount > (mMaxCount - mCount))
	{
		Uint32 n = RoundUp8(mCount + inCount);
		mData = (void **)UMemory::Reallocate((TPtr)mData, n * sizeof(void*));
		mMaxCount = n;
	}
}

// returns index (1-based) of newly inserted item, inIndex is 1-based
Uint32 CVoidPtrList::InsertItem(Uint32 inIndex, void *inPtr)
{
	Require(inPtr);

	if (mData)
	{
		Uint32 n;
		void **sp, **dp;
	
		if (mCount == mMaxCount)
		{
			n = RoundUp8(mMaxCount + 8);
			mData = (void **)UMemory::Reallocate((TPtr)mData, n * sizeof(void*));
			mMaxCount = n;
		}

		if (inIndex == 0)			// if insert at start
			inIndex = 1;
		else if (inIndex > mCount)	// if append
			inIndex = mCount + 1;

		n = mCount - (inIndex - 1);
		sp = dp = mData + mCount;
		dp++;
		while (n--) *--dp = *--sp;
		
		mData[inIndex-1] = inPtr;
		mCount++;
		
		return inIndex;
	}
	else
	{
		mData = (void **)UMemory::New(32);
		*mData = inPtr;
		mCount = 1;
		mMaxCount = 32 / sizeof(void *);
		return 1;
	}
}

// returns index of the item removed (0 if not found)
Uint32 CVoidPtrList::RemoveItem(void *inPtr)
{
	if (inPtr)
	{
		Uint32 n;
		void **sp, **dp;

		sp = mData - 1;
		n = mCount;

		while (n--)
		{
			if (*++sp == inPtr)
			{
				dp = sp;
				sp++;
				n = (mData + mCount) - sp;
				
				Uint32 index = sp - mData;
				
		#if USE_PRE_INCREMENT
				dp--; sp--;
				while (n--) *++dp = *++sp;
		#else
				while (n--) *dp++ = *sp++;
		#endif
			
				mCount--;
				
				if (mMaxCount - mCount > 32)	// if enough unused space to warrant a shrink
				{
					try
					{
						n = RoundUp4(mCount);
						mData = (void **)UMemory::Reallocate((TPtr)mData, n * sizeof(void*));
						mMaxCount = n;
					}
					catch(...)
					{
						// don't throw (don't care if the shrink failed)
					}
				}

				return index;
			}
		}
	}
	
	// not found
	return 0;
}

// inIndex is 1-based, returns the items ptr if it was removed (nil if invalid index)
void *CVoidPtrList::RemoveItem(Uint32 inIndex)
{
	if (mData && inIndex && inIndex <= mCount)
	{
		Uint32 n;
		void **sp, **dp;
		
		sp = dp = mData + (inIndex - 1);
		sp++;
		n = mCount - inIndex;
		
		void *theItem = *dp;
		
#if USE_PRE_INCREMENT
		dp--; sp--;
		while (n--) *++dp = *++sp;
#else
		while (n--) *dp++ = *sp++;
#endif
	
		mCount--;
		
		if (mMaxCount - mCount > 32)	// if enough unused space to warrant a shrink
		{
			try
			{
				n = RoundUp4(mCount);
				mData = (void **)UMemory::Reallocate((TPtr)mData, n * sizeof(void*));
				mMaxCount = n;
			}
			catch(...)
			{
				// don't throw (don't care if the shrink failed)
			}
		}
		
		return theItem;
	}
	
	return nil;
}

void CVoidPtrList::MoveForward(void *inPtr)
{
	void **p = mData;
	Uint32 n = mCount;
	Uint32 i;
		
	// find the entry, and then swap it with the next entry
	for (i=0; i!=n; i++)
	{
		if (p[i] == inPtr)		// if found
		{
			if (i < n-1)		// if not frontmost
			{
				void *next = p[i+1];
				p[i+1] = inPtr;
				p[i] = next;
			}
			break;
		}
	}
}

void CVoidPtrList::MoveBackward(void *inPtr)
{
	void **p = mData;
	Uint32 n = mCount;
	Uint32 i;
	
	// find the entry, and then swap it with the previous entry
	for (i=0; i!=n; i++)
	{
		if (p[i] == inPtr)	// if found
		{
			if (i > 0)	// if not backmost
			{
				void *prev = p[i-1];
				p[i-1] = inPtr;
				p[i] = prev;
			}
			break;
		}
	}
}

void CVoidPtrList::MoveToFront(void *inPtr)
{
	// remove the entry, and then add it to the end of the list
	if (inPtr)
	{
		RemoveItem(inPtr);
		AddItem(inPtr);
	}
}

void CVoidPtrList::MoveToBack(void *inPtr)
{
	// remove the entry, and then insert it at the start
	if (inPtr)
	{
		RemoveItem(inPtr);
		InsertItem(0, inPtr);
	}
}

void CVoidPtrList::Clear()
{
	UMemory::Dispose((TPtr)mData);
	mData = nil;
	mCount = mMaxCount = 0;
}

Uint32 CVoidPtrList::GetItemIndex(void *inPtr) const
{
	void **p = mData;
	Uint32 n = mCount;
	Uint32 i;

	for (i=0; i!=n; i++)
	{
		if (p[i] == inPtr)
			return i+1;
	}
	
	return 0;
}

/*
The compare function must return one of the following:

	< 0		item A is less than item B
	0		item A is equal to item B
	> 0		item A is greater than item B
*/
void CVoidPtrList::Sort(TComparePtrsProc inProc, void *inRef)
{
	Uint32 num_members, l, r, j;
	void **table, **lp, **rp, **ip, **jp, **kp;
	void *tmp;
	
	Require(inProc);
	
	num_members = mCount;
	if (num_members < 2) return;
	
	table = mData;
	
	r = num_members;
	l = (r / 2) + 1;
	
	lp = table + (l - 1);
	rp = table + (r - 1);
	
	for (;;)
	{
		if (l > 1)
		{
			l--;
			lp--;
		}
		else
		{
			tmp = *lp;
			*lp = *rp;
			*rp = tmp;
			
			if (--r == 1)
				return;
			
			rp--;
		}
		
		j = l;
		
		jp = table + (j - 1);
		
		while (j*2 <= r)
		{
			j *= 2;
			
			ip = jp;
			jp = table + (j - 1);
			
			if (j < r)
			{
				kp = jp + 1;
				
				if (inProc(*jp, *kp, inRef) < 0)
				{
					j++;
					jp = kp;
				}
			}
			
			if (inProc(*ip, *jp, inRef) < 0)
			{
				tmp = *ip;
				*ip = *jp;
				*jp = tmp;
			}
			else
				break;
		}
	}
}

// returns false if not found (outIndex is still valid).  outIndex is 1-based.
bool CVoidPtrList::SortedSearch(void *inPtr, Uint32& outIndex, TComparePtrsProc inProc, void *inRef)
{
	void **table;
	Uint32 l, r, i;
	Int32 comparison;
	
	if (mCount == 0)
	{
		outIndex = 1;
		return false;
	}

	table = mData;

	comparison = inProc(inPtr, table[0], inRef);
	if (comparison == 0)
	{
		outIndex = 1;
		return true;
	}
	else if (comparison < 0)
	{
		outIndex = 1;
		return false;
	}
	
	l = 1;
	r = mCount - 1;
	i = 0;
	
	if (l > r)
	{
		outIndex = 2;
		return false;
	}
	
	while (l <= r)
	{
		i = (l + r) >> 1;
		
		comparison = inProc(inPtr, table[i], inRef);
		
		if (comparison == 0)
		{
			outIndex = i+1;
			return true;
		}
		else if (comparison < 0)
			r = i - 1;
		else
			l = i + 1;
	}
	
	if (inProc(inPtr, table[i], inRef) > 0) i++;

	outIndex = i+1;
	return false;
}

void CVoidPtrList::Truncate(Uint32 inCount)
{
	if (inCount < mCount)
	{
		mCount = inCount;
		
		if (mMaxCount - mCount > 32)	// if enough unused space to warrant a shrink
		{
			try
			{
				Uint32 n = RoundUp4(mCount);
				mData = (void **)UMemory::Reallocate((TPtr)mData, n * sizeof(void*));
				mMaxCount = n;
			}
			catch(...)
			{
				// don't throw (don't care if the shrink failed)
			}
		}
	}
}


