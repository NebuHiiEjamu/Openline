// =====================================================================
//  CSemaphore.h                         (C) Hotline Communications 1999
// =====================================================================
// Semaphore for locking threads

#ifndef _H_CSemaphore_
#define _H_CSemaphore_

#if PRAGMA_ONCE
	#pragma once
#endif

HL_Begin_Namespace_BigRedH

class CSemaphore
{
	public:
							CSemaphore( UInt16 inMaxCount = 1 );
							// throws CMemoryException
							~CSemaphore();

		// ** Protocol **
		void				Acquire();
							// throws nothing
		bool				TryAcquire();
							// throws nothing
		void				Release();
							// throws nothing

	private:
		class CSemaphorePS;
		CSemaphorePS		*mPlatform;
};

HL_End_Namespace_BigRedH

#endif // _H_CSemaphore_