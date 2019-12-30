// =====================================================================
//	CMessagingThread.h                   (C) Hotline Communications 2000
// =====================================================================
//
// Light weight process (thread) class with messaging capabilities

#ifndef _H_CMessagingThread_
#define _H_CMessagingThread_

#if PRAGMA_ONCE
	#pragma once
#endif

#include "CThread.h"
#include "CMessageWrangler.h"


HL_Begin_Namespace_BigRedH

class CMessagingThread : public CThread, public CMessageWrangler
{
	public:
							CMessagingThread( EPriority inPriority = ePriorityMedium );

	protected:
		virtual UInt32		Execute();
};

HL_End_Namespace_BigRedH

#endif	// _H_CMessagingThread_
