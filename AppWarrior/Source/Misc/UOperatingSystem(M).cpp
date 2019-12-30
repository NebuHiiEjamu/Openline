#if MACINTOSH

/* (c)2003 Hotsprings Inc. Licensed under GPL - see LICENSE in HotlineSources diresctory */

#include "UOperatingSystem.h"
#include "UError.h"
#include "UDebug.h"
#include "UMemory.h"

#include <Gestalt.h>
#include <Movies.h>
#include <SegLoad.h>
#include <Windows.h>
#include <QuickDraw.h>
#include <Fonts.h>
#include <TextEdit.h>
#include <Dialogs.h>
#include <QDOffscreen.h>
#include <Sound.h>

/*
 * Function Prototypes
 */

Int16 _GetSystemVersion();
static Int16 _OSMakeMasters(Uint32 inCount);

extern ProcessSerialNumber _APProcSerialNum;

CGrafPtr _gStartupPort;
GDHandle _gStartupDevice;

Uint32 _gHandleCountEst = 1024;

/* -------------------------------------------------------------------------- */

bool UOperatingSystem::mIsQuickTimeAvailable = false;
Str255 UOperatingSystem::mQuickTimeVersion;
bool UOperatingSystem::mCanHandleFlash = false;

/*
 * Init() initializes the operating system for an application program.
 * Call this first thing in your program before you call any other
 * functions.
 */
void UOperatingSystem::Init()
{
	// init only once
	static Uint8 isInitted = false;
	if (isInitted) return;
	isInitted = true;
	
	try
	{
		// init memory manager
	#if !TARGET_API_MAC_CARBON
		::MaxApplZone();
	#endif
	
		if (_OSMakeMasters(_gHandleCountEst) != 0)
		{
			DebugBreak("UOperatingSystem - not enough memory to initialize (fatal)");
			::ExitToShell();
		}
	
		// no need for this initializations in Carbon
	#if !TARGET_API_MAC_CARBON
		// init mac toolbox
		::InitGraf((Ptr) &qd.thePort);
	
		::InitFonts();
		::InitWindows();
		::InitMenus();
		::TEInit();
		::InitDialogs(0);
	#endif
	
		::InitCursor();
		::FlushEvents(everyEvent - diskMask - osMask, 0);
		
		::GetGWorld(&_gStartupPort, &_gStartupDevice);
		
		// init essential managers
		UError::Init();
		UMemory::Init();
		
		// init private data
		::GetCurrentProcess(&_APProcSerialNum);
	}
	catch(...)
	{
		DebugBreak("UOperatingSystem - failed to initialize (fatal)");
		::ExitToShell();
	}
}

static void _UninitQuickTime()
{
	HL_HandlerUnregister();

	::ExitMovies();
}

bool UOperatingSystem::InitQuickTime()
{
	mQuickTimeVersion[0] = 0;
	
	long nResult;
	if (::Gestalt(gestaltQuickTimeVersion, &nResult) != noErr)
		return false;
		
	// setup version string
	struct SVers{
		char majorH : 4;
		char majorL : 4;
		char minorH : 4;
		char minorL : 4;
	} *versS;

	short vers = (nResult & 0xFFFF0000) >> 16;
	versS = (SVers*)&vers;
	int i = 1;
	if ( versS->majorH > 0 ){
		mQuickTimeVersion[i++] = versS->majorH + '0';
	}
	mQuickTimeVersion[i] = versS->majorL + '0';
	if ( versS->minorH > 0 ){
		i++;
		mQuickTimeVersion[i++] = '.';
		mQuickTimeVersion[i] = versS->minorH + '0';
	}
	if ( versS->minorL > 0 ){
		if( versS->minorH <= 0 ){
			i++;
			mQuickTimeVersion[i++] = '.';
			mQuickTimeVersion[i] = '0';
		}
		i++;
		mQuickTimeVersion[i++] = '.';
		mQuickTimeVersion[i] = versS->minorL + '0';
	}
	mQuickTimeVersion[0] = (char)i;
			
	// check for Flash availability
	ComponentDescription stCompDesc = {MediaHandlerType, 'flsh', //FlashMediaType,
									   kAppleManufacturer,  0, 0};
												
	long nCount = ::CountComponents(&stCompDesc);
	if (nCount > 0)
		mCanHandleFlash = true;
	
	// setup QuickTime
	if (::EnterMovies() != noErr)
		return false;
		
	HL_HandlerRegister();

	try
	{
		UProgramCleanup::InstallSystem(_UninitQuickTime);
	}
	catch(...)
	{
		_UninitQuickTime();
		throw;
	}
			
	mIsQuickTimeAvailable = true;
	return true;
}

Uint8 *UOperatingSystem::GetSystemVersion()
{
	static Uint8 psVersion[32];
	psVersion[0] = UMemory::Copy(psVersion + 1, "Mac OS", 6);

	Int16 nVersion = _GetSystemVersion();
	if (!nVersion)
		return psVersion;
	
	struct SVersion {
		Uint8 nMajor;
		Uint8 nMinorH : 4;
		Uint8 nMinorL : 4;
	} *pVersion;

	pVersion = (SVersion *)&nVersion;
		
	// system version
	psVersion[0] += UText::Format(psVersion + psVersion[0] + 1, sizeof(psVersion) - psVersion[0] - 1, " %x.%x", pVersion->nMajor, pVersion->nMinorH);
	if (pVersion->nMinorL > 0)
		psVersion[0] += UText::Format(psVersion + psVersion[0] + 1, sizeof(psVersion) - psVersion[0] - 1, ".%x", pVersion->nMinorL);

	return psVersion;
}

/* -------------------------------------------------------------------------- */
#pragma mark -

static Int16 _OSMakeMasters(Uint32 inCount)
{
	// round up to multiple of 16
	inCount += 15;
	inCount &= ~15;
	
#if TARGET_API_MAC_CARBON
	::MoreMasterPointers(inCount);
#else	
	if (inCount <= 64)
		::MoreMasters();
	else
	{
		THz zone = ::GetZone();
		short saveMoreMast = zone->moreMast;
		
		zone->moreMast = inCount;
		::MoreMasters();
		zone->moreMast = saveMoreMast;
	}
#endif
	
	return ::MemError();
}

#if !TARGET_API_MAC_CARBON
bool _TrapAvailable(Int16 theTrap)
{
	TrapType tType;
	Int16 numTraps;

	tType = (theTrap & 0x0800) > 0 ? ToolTrap : OSTrap;
	numTraps = ::NGetTrapAddress(_InitGraf, ToolTrap) == ::NGetTrapAddress(0xAA6E, ToolTrap) ? 0x0200 : 0x0400;
	
	if (tType == ToolTrap)
		theTrap = theTrap & 0x07FF;
		
	if (theTrap >= numTraps)
		theTrap = _Unimplemented;

	return (::NGetTrapAddress(theTrap, tType) != ::NGetTrapAddress(_Unimplemented, ToolTrap));
}
#endif

Int16 _GetSystemVersion()
{
#if !TARGET_API_MAC_CARBON
	if (_TrapAvailable(_Gestalt))
#endif
	{
		Int32 nResponse;
		if (::Gestalt(gestaltSystemVersion, &nResponse) != noErr)
			return 0;
				
		// ignore the high-order word
		return nResponse;
	}

	return 0;
}

Int16 _GetCarbonVersion()
{
#if !TARGET_API_MAC_CARBON
	if (_TrapAvailable(_Gestalt))
#endif
	{
		Int32 nResponse;
		if (::Gestalt(gestaltCarbonVersion, &nResponse) != noErr)
			return 0;
				
		// ignore the high-order word
		return nResponse;
	}

	return 0;
}

#endif /* MACINTOSH */
