/* (c)2003 Hotsprings Inc. Licensed under GPL - see LICENSE in HotlineSources diresctory */

#include "AW.h"
#include "CMutex.h"
#include <Threads.h>
#include <queue>

HL_Begin_Namespace_BigRedH

class CMutex::CMutexPS {
	public:
			ThreadID				mOwner;
			SInt16					mOwnerCount;
			std::queue<ThreadID>	mWaitingThreads;

};


// ---------------------------------------------------------------------
//  CMutex                                                      [public]
// ---------------------------------------------------------------------
// Constructor

CMutex::CMutex()
	: mPlatform(new CMutex::CMutexPS)
{
	mPlatform->mOwner = kNoThreadID;
	mPlatform->mOwnerCount = 0;
}


// ---------------------------------------------------------------------
//  ~CMutex                                                     [public]
// ---------------------------------------------------------------------
// Destructor

CMutex::~CMutex()
{
	delete mPlatform;
}


// ---------------------------------------------------------------------
//  Acquire                                                     [public]
// ---------------------------------------------------------------------

void
CMutex::Acquire()
{
	ThreadID currThread;
	
	::GetCurrentThread( &currThread );
	if( mPlatform->mOwner == kNoThreadID ){ // Unowned
		mPlatform->mOwner = currThread;
		mPlatform->mOwnerCount = 1;
	} else if( mPlatform->mOwner == currThread ){ // Same thread
		mPlatform->mOwnerCount++;
	} else {
		::ThreadBeginCritical();
		mPlatform->mWaitingThreads.push( currThread );
		OSErr anErr = ::SetThreadStateEndCritical( currThread,
							kStoppedThreadState, kNoThreadID );
	}
}


// ---------------------------------------------------------------------
//  TryAcquire                                                  [public]
// ---------------------------------------------------------------------

bool
CMutex::TryAcquire()
{
	ThreadID currThread;
	
	::GetCurrentThread( &currThread );
	if( mPlatform->mOwner == kNoThreadID ){ // Unowned
		mPlatform->mOwner = currThread;
		mPlatform->mOwnerCount = 1;
		return true;
	} else if( mPlatform->mOwner == currThread ){ // Same thread
		mPlatform->mOwnerCount++;
		return true;
	} else {
		return false;
	}
}


// ---------------------------------------------------------------------
//  Release                                                     [public]
// ---------------------------------------------------------------------

void
CMutex::Release()
{
	ThreadID currThread;
	
	::GetCurrentThread( &currThread );
	if( mPlatform->mOwner != currThread ){ // Release called from wrong thread
		//?? Throw Exception?
	} else {
		mPlatform->mOwnerCount--;
		if( mPlatform->mOwnerCount <= 0 ){
			if( mPlatform->mWaitingThreads.empty() ){
				mPlatform->mOwner = kNoThreadID;
				mPlatform->mOwnerCount = 0;
			} else {
				::ThreadBeginCritical();
				mPlatform->mOwner = mPlatform->mWaitingThreads.front();
				mPlatform->mOwnerCount = 1;
				mPlatform->mWaitingThreads.pop();
				OSErr anErr = ::SetThreadStateEndCritical( mPlatform->mOwner,
									kReadyThreadState, kNoThreadID );
			}
		}
	}
}

HL_End_Namespace_BigRedH

