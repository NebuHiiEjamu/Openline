// =====================================================================
//	CTimerThread.h                       (C) Hotline Communications 2000
// =====================================================================
//
// Light weight process (thread) class with messaging capabilities

#ifndef _H_CTimerThread_
#define _H_CTimerThread_

#if PRAGMA_ONCE
	#pragma once
#endif

#include "CThread.h"


HL_Begin_Namespace_BigRedH

class CTimerThread : public CThread
{
	public:
							CTimerThread( CThread &inThread,
									UInt32 inWaitMilliSecs );

	protected:
		virtual UInt32		Run();

	private:
		CThread				&mThread;
		UInt32				mWaitTime;
};

HL_End_Namespace_BigRedH

#endif	// _H_CTimerThread_
