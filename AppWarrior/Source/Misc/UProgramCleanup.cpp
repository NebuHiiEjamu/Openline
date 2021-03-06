/* (c)2003 Hotsprings Inc. Licensed under GPL - see LICENSE in HotlineSources diresctory */

#include "UProgramCleanup.h"
#include "UMemory.h"

/*
 * Types
 */

typedef void (*TProgramCleanupProc)();

class UProgramCleanup::Cleanup
{
	protected:
		Cleanup();
		~Cleanup();

		void InstallSystem(TProgramCleanupProc inTermProc);
		void InstallAppl(TProgramCleanupProc inTermProc);
		
		void CleanupAppl();
		
		Uint16 mSystemCount;
		THdl mSystemList;

		Uint16 mApplCount;
		THdl mApplList;

	friend class UProgramCleanup;
};

/*
 * Global Variables
 */

UProgramCleanup::Cleanup UProgramCleanup::gProgramCleanup;

/* -------------------------------------------------------------------------- */

UProgramCleanup::Cleanup::Cleanup()
{
	mSystemCount = 0;
	mSystemList = nil;

	mApplCount = 0;
	mApplList = nil;
}

UProgramCleanup::Cleanup::~Cleanup()
{
	if (mSystemList)
	{
		TProgramCleanupProc *list = (TProgramCleanupProc *)UMemory::Lock(mSystemList);
		
		for (Uint32 i=0; i < mSystemCount; i++)
		{
			try
			{
				(list[i])();
			}
			catch(...) {}
		}
		
		UMemory::Unlock(mSystemList);
		UMemory::Dispose(mSystemList);
	}
}

void UProgramCleanup::Cleanup::InstallSystem(TProgramCleanupProc inTermProc)
{
	if (!mSystemList) 
		mSystemList = UMemory::NewHandle();
		
	UMemory::Append(mSystemList, &inTermProc, sizeof(inTermProc));
	mSystemCount++;
}

void UProgramCleanup::Cleanup::InstallAppl(TProgramCleanupProc inTermProc)
{
	if (!mApplList) 
		mApplList = UMemory::NewHandle();
		
	UMemory::Append(mApplList, &inTermProc, sizeof(inTermProc));
	mApplCount++;
}

void UProgramCleanup::Cleanup::CleanupAppl()
{
	if (mApplList)
	{
		TProgramCleanupProc *list = (TProgramCleanupProc *)UMemory::Lock(mApplList);
		
		for (Uint32 i=0; i < mApplCount; i++)
		{
			try
			{
				(list[i])();
			}
			catch(...) {}
		}
		
		UMemory::Unlock(mApplList);
		UMemory::Dispose(mApplList);
		mApplList = nil;
	}
}

/*
 * Install() installs a function that will be called automagically
 * when the program terminates normally.  You can install as many
 * functions as you like, and they will be called in the order that
 * they were installed.
 */
 
void UProgramCleanup::InstallSystem(TProgramCleanupProc inTermProc)
{
	gProgramCleanup.InstallSystem(inTermProc);
}

void UProgramCleanup::InstallAppl(TProgramCleanupProc inTermProc)
{
	gProgramCleanup.InstallAppl(inTermProc);
}

void UProgramCleanup::CleanupAppl()
{
	gProgramCleanup.CleanupAppl();
}

