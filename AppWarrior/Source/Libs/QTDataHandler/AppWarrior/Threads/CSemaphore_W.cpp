/* (c)2003 Hotsprings Inc. Licensed under GPL - see LICENSE in HotlineSources diresctory */

#include "AW.h"
#include "CSemaphore.h"
#include "CThreadException.h"

HL_Begin_Namespace_BigRedH

// ---------------------------------------------------------------------
// CSemaphorePS                                                [private]
// ---------------------------------------------------------------------
// Platform specific semaphore class

class CSemaphore::CSemaphorePS
{
	public:
		HANDLE	mID;
};

// ---------------------------------------------------------------------
//  CSemaphore                                                  [public]
// ---------------------------------------------------------------------
// Constructor

CSemaphore::CSemaphore(UInt16 inMaxCount)
	: mPlatform( new CSemaphore::CSemaphorePS )
{
	if((mPlatform->mID = ::CreateSemaphore(nil, 0, inMaxCount, nil)) == 0)
	{
		THROW_THREAD_(eCreatingSemaphore, eUnknownOSError, ::GetLastError());
	}
}

// ---------------------------------------------------------------------
//  ~CSemaphore                                                 [public]
// ---------------------------------------------------------------------
// Destructor

CSemaphore::~CSemaphore()
{
	ASSERT(mPlatform->mID != 0);
	if(mPlatform->mID != 0)
	{
		::CloseHandle(mPlatform->mID), mPlatform->mID = 0;
	}
	delete mPlatform;
}

// ---------------------------------------------------------------------
//  Acquire                                                     [public]
// ---------------------------------------------------------------------

void
CSemaphore::Acquire()
{
	ASSERT(mPlatform->mID != 0);
	if(mPlatform->mID != 0)
	{
		::WaitForSingleObject(mPlatform->mID, INFINITE);
	}
}

// ---------------------------------------------------------------------
//  TryAcquire                                                  [public]
// ---------------------------------------------------------------------

bool
CSemaphore::TryAcquire()
{
	ASSERT(mPlatform->mID != 0);
	if(mPlatform->mID != 0)
	{
		UInt32 rc = ::WaitForSingleObject(mPlatform->mID, 0);
		if(rc != WAIT_OBJECT_0 && rc != WAIT_ABANDONED)
		{
			return false;
		}
		return true;
	}
    return false;
}

// ---------------------------------------------------------------------
//  Release                                                     [public]
// ---------------------------------------------------------------------

void
CSemaphore::Release()
{
	ASSERT(mPlatform->mID != 0);
	if(mPlatform->mID != 0)
	{
		::ReleaseSemaphore(mPlatform->mID, 1, nil);
	}
}


HL_End_Namespace_BigRedH

