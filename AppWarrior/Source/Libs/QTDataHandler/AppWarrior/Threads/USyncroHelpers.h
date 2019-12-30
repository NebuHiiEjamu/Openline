// =====================================================================
//  USyncroHelpers.h                     (C) Hotline Communications 1999
// =====================================================================
//

#ifndef _H_USyncroHelpers_
#define _H_USyncroHelpers_

#if PRAGMA_ONCE
	#pragma once
#endif

#include "CMutex.h"
#include "CSemaphore.h"

HL_Begin_Namespace_BigRedH

template <typename _Lock> class TGuard
{
	protected:
		_Lock&			mLock;
	public:
			// ** Construction **
						TGuard( const _Lock& );
						~TGuard();

			// ** Access **
		const _Lock&	GetInstance() const;
};

// ---------------------------------------------------------------------
//	TGuard                                                      [public]
// ---------------------------------------------------------------------
// Constructor

template <typename _Lock>
TGuard<_Lock>::TGuard( const _Lock& inLock )
	: mLock(const_cast<_Lock&>(inLock))
{
    mLock.Acquire();
}

// ---------------------------------------------------------------------
//	~TGuard                                                     [public]
// ---------------------------------------------------------------------
// Destructor

template <typename _Lock>
TGuard<_Lock>::~TGuard()
{
    mLock.Release();
}

// ---------------------------------------------------------------------
//	GetInstance                                                 [public]
// ---------------------------------------------------------------------

template <typename _Lock>
const _Lock& TGuard<_Lock>::GetInstance() const
{
	return mLock;
}


// Helper instaniations
typedef TGuard<CMutex> StMutex;
typedef TGuard<CSemaphore> StSemaphore;

HL_End_Namespace_BigRedH

#endif // _H_USyncroHelpers_

