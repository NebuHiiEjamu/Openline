#if WIN32

/* (c)2003 Hotsprings Inc. Licensed under GPL - see LICENSE in HotlineSources diresctory */

static void _SafeOLEInit();

bool UOperatingSystem::mIsQuickTimeAvailable = false;
Str255 UOperatingSystem::mQuickTimeVersion;
bool UOperatingSystem::mCanHandleFlash = false;

void UOperatingSystem::Init()
{
	// init only once
	static Uint8 isInitted = false;
	if (isInitted) return;
	isInitted = true;

	OSVERSIONINFO stVersion = {sizeof(OSVERSIONINFO),0,0,0,0,""};
	if (!::GetVersionEx(&stVersion)) goto badOS;
	if (stVersion.dwPlatformId == VER_PLATFORM_WIN32s) goto badOS;
	if (stVersion.dwPlatformId == VER_PLATFORM_WIN32_NT && stVersion.dwMajorVersion < 4) goto badOS;
	
	_SafeOLEInit();	// for D&D & using shortcuts and such
	
	try
	{
		UError::Init();
		UMemory::Init();
	}
	catch(...)
	{
		DebugBreak("UOperatingSystem - failed to initialize (fatal)");
		::ExitProcess(0);
	}
	
	return;
	
badOS:
	::MessageBox(NULL, "This program requires Windows 95 or Windows NT 4 or better.", "Error", MB_TASKMODAL | MB_ICONSTOP);
	::ExitProcess(0);
}

static void _UninitQuickTime()
{
	HL_HandlerUnregister();

	::ExitMovies();
	::TerminateQTML();
}

bool UOperatingSystem::InitQuickTime()
{
	mQuickTimeVersion[0] = 0;

	// setup QuickTime
	if (::InitializeQTML(0) != noErr)
		return false;

	if (::EnterMovies() != noErr)
	{
		::TerminateQTML();
		return false;
	}
		
	// check for Flash availability
	ComponentDescription stCompDesc = {MediaHandlerType, FlashMediaType, kAppleManufacturer, 0, 0};
	long nCount = ::CountComponents(&stCompDesc);
	if (nCount > 0)
	{
		// version is 4.1 or better
		::pstrcpy(mQuickTimeVersion, "\p4.1");
		mCanHandleFlash = true;
	}

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
	static Uint8 psVersion[64];
	psVersion[0] = UMemory::Copy(psVersion + 1, "Windows", 7);

	OSVERSIONINFO stVersion = {sizeof(OSVERSIONINFO),0,0,0,0,""};
	if (!::GetVersionEx(&stVersion))
		return psVersion;
		
	// system type
	if (stVersion.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
	{
		if (stVersion.dwMinorVersion == 0)
			psVersion[0] += UMemory::Copy(psVersion + psVersion[0] + 1, " 95", 3);
		else
			psVersion[0] += UMemory::Copy(psVersion + psVersion[0] + 1, " 98", 3);
	}
	else if (stVersion.dwPlatformId == VER_PLATFORM_WIN32_NT)
	{
		if (stVersion.dwMajorVersion < 5)
			psVersion[0] += UMemory::Copy(psVersion + psVersion[0] + 1, " NT", 3);
		else
			psVersion[0] += UMemory::Copy(psVersion + psVersion[0] + 1, " 2000", 5);
	}
	
	// system version
	psVersion[0] += UText::Format(psVersion + psVersion[0] + 1, sizeof(psVersion) - psVersion[0] - 1, " %lu.%lu", stVersion.dwMajorVersion, stVersion.dwMinorVersion);

	// Service Pack number
	if (stVersion.dwPlatformId == VER_PLATFORM_WIN32_NT && strlen(stVersion.szCSDVersion))
		psVersion[0] += UText::Format(psVersion + psVersion[0] + 1, sizeof(psVersion) - psVersion[0] - 1, " %s", stVersion.szCSDVersion);

	return psVersion;
}

/* -------------------------------------------------------------------------- */
#pragma mark -

void main();
HINSTANCE _gProgramInstance;
int _gStartupShowCmd;
Int8 *_gCmdLineStr = NULL;
Uint32 _gCmdLineStrSize = 0;
Uint32 _gCmdLineStrOffset = 0;
Uint8 *_gServiceName = NULL;

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE /* hPrevInstance */, LPSTR szCmdLine, int iCmdShow)
{
	// save command line (contains file paths to open) for later processing
	if (szCmdLine && szCmdLine[0])
	{
		Int8 bufAppPath[2048];
		::GetModuleFileName(NULL, bufAppPath, sizeof(bufAppPath));
		
	#if 0	
		// currently, let's allow multi copies of the app to run
		if (UExternalApp::SearchRunningApp(bufAppPath, msg_ExternalCmdLine, szCmdLine))
			return 0;
	#endif		
		
		// when "Hotline.Protocol.1" object is called the program 
		// receives "-Embedding" command which must be ignored
		Uint32 nCmdSize = strlen(szCmdLine);
		if (nCmdSize < 10 || UText::CompareInsensitive(szCmdLine, "-Embedding", 10))
		{	
			_gCmdLineStr = (Int8 *)::GlobalAlloc(GMEM_FIXED, nCmdSize + 1);
			if (_gCmdLineStr)
			{
				::CopyMemory(_gCmdLineStr, szCmdLine, nCmdSize + 1);
				_gCmdLineStrSize = nCmdSize;
			}
		}
	}

	_gProgramInstance = hInstance;
	_gStartupShowCmd = iCmdShow;

	if (_gServiceName)
	{
		UService::Init(_gServiceName, opt_ServiceAccespStop | opt_ServiceAcceptShutDown);
		
		if (_gCmdLineStrSize >= 1 && *_gCmdLineStr == '-')
		{
			if (_gCmdLineStrSize >= 8 && !UText::CompareInsensitive(_gCmdLineStr + 1, "install", 7))
				UService::Install(opt_ServiceAutoStart);
			else if (_gCmdLineStrSize >= 10 && !UText::CompareInsensitive(_gCmdLineStr + 1, "uninstall", 9))
				UService::Uninstall();
		}
		else
		{
			if (!UService::IsInstalled())
				UService::Install(opt_ServiceAutoStart);
			else		
				UService::Start();		
		}
	}
	else
	{
		try {
			main();
		} catch(...) {}
	}
		
	return 0;
}

/* -------------------------------------------------------------------------- */
#pragma mark -

extern bool _InitOleAutomationServer();
extern void _UninitOleAutomationServer();

static void _ProgCleanupOLEUninitialize()
{
	::OleUninitialize();
}

static void _SafeOLEInit()
{
	static Uint8 oleInitted = false;
	
	if (!oleInitted)
	{
		HRESULT res = ::OleInitialize(NULL);
		
		if (res != S_OK)
		{
			DebugBreak("Error calling OLEInitialize() - result = %lu", res);
			Fail(errorType_Misc, error_Unknown);
		}
		
		try
		{
			UProgramCleanup::InstallSystem(_ProgCleanupOLEUninitialize);
		}
		catch(...)
		{
			::OleUninitialize();
			throw;
		}

		_InitOleAutomationServer();
		UProgramCleanup::InstallAppl(_UninitOleAutomationServer);
		
		oleInitted = true;
	}
}

#endif /* WIN32 */
