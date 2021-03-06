/* (c)2003 Hotsprings Inc. Licensed under GPL - see LICENSE in HotlineSources diresctory */

#pragma once
#include "typedefs.h"
#include "MoreTypes.h"

enum {
	kOnceTimer			= 0,
	kRepeatingTimer		= 1
};

typedef class TTimerObj *TTimer;

class UTimer
{
	public:
		static void Init();
		static TTimer New(TMessageProc inProc, void *inContext);
		static TTimer StartNew(TMessageProc inProc, void *inContext, Uint32 inMillisecs, Uint32 inIsRepeating = kOnceTimer);

		static void Dispose(TTimer inRef);
		static void Start(TTimer inRef, Uint32 inMillisecs, Uint32 inIsRepeating = kOnceTimer);
		static bool WasStarted(TTimer inRef);
		static void Stop(TTimer inRef);
		static void Simulate(TTimer inRef);
};

class TTimerObj
{
	public:
		void Start(Uint32 inMillisecs, Uint32 inIsRepeating = kOnceTimer)	{	UTimer::Start(this, inMillisecs, inIsRepeating);	}
		bool WasStarted()													{	return UTimer::WasStarted(this);					}
		void Stop()															{	UTimer::Stop(this);									}
		void Simulate()														{	UTimer::Simulate(this);								}
		
		void operator delete(void *p)										{	UTimer::Dispose((TTimer)p);							}
	protected:
		TTimerObj() {}
};
