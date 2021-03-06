
#if WIN32

/* (c)2003 Hotsprings Inc. Licensed under GPL - see LICENSE in HotlineSources diresctory */

#define PLATFORM_WIN32_ALL		1

#include <tlHelp32.h>
#if !PLATFORM_WIN32_ALL
#include <psapi.h>
#endif

#include "UExternalApp.h"

struct SExternalAppInfo{
	STARTUPINFO stStartupInfo;
	PROCESS_INFORMATION stProcessInfo;
};


#define APP		((SExternalAppInfo *)inApp)

static bool gEnumAllWnd = false;
BOOL CALLBACK _EnumThreadWndProc(HWND inHwnd, LPARAM inParam);

static bool _IsCreatedFileExtension(const Uint8 *inFileExtension);
static bool _IsCreatedFileExtension(const Uint8 *inFileExtension, const Int8 *inApplicationPath);
static bool _CreateFileExtensionApp(const Uint8 *inFileExtension, const Int8 *inApplicationPath, const Uint8 *inTitle = nil);
static bool _CreateFileDefaultIconApp(const Uint8 *inFileExtension, const Int8 *inApplicationPath);
static Uint32 _SearchFileExtensionApp(const Uint8 *inFileExtension, Int8 *outApplicationPath, Uint32 inMaxPathSize);
#if !PLATFORM_WIN32_ALL
static bool _SearchProcess_WIN(const Int8 *inProcessPath, Uint32 inMsg, const Int8* inMessage);
static bool _SearchProcess_NT(const Int8 *inProcessPath, Uint32 inMsg, const Int8* inMessage);
#endif

bool _WriteTempMessageInRegistry(const Int8 *inMessage);
bool _ReadTempMessageFromRegistry(Int8 *outMessage, DWORD inMaxSize);


TExternalApp UExternalApp::New(const void *inAppPath, Uint32 inPathSize, const void *inParam, Uint32 inParamSize)
{
	if (!inAppPath || !inPathSize || inPathSize > 4095 || inParamSize > 4095)
		return nil;
		
	Int8 bufCommandLine[8192];
	UMemory::Copy(bufCommandLine, inAppPath, inPathSize);
		
	if (inParam && inParamSize)
	{
		Int8 *pParam = (Int8 *)UMemory::SearchByte('%', bufCommandLine, inPathSize);
		if (pParam)
		{			
			UMemory::Copy(pParam, inParam, inParamSize);
			if (*(pParam - 1) == '\"') {	*(pParam + inParamSize) = '\"'; pParam++;	}
			*(pParam + inParamSize) = 0;
		}
		else
		{
			*(bufCommandLine + inPathSize) = ' ';
			*(bufCommandLine + inPathSize + 1) = '\"';
			UMemory::Copy(bufCommandLine + inPathSize + 2, inParam, inParamSize);
			*(bufCommandLine + inPathSize + inParamSize + 2) = '\"';
			*(bufCommandLine + inPathSize + inParamSize + 3) = 0;
		}
	}
	else
	{
		*(bufCommandLine + inPathSize) = 0;
	}
	
	SExternalAppInfo *pExternalAppInfo = (SExternalAppInfo *)UMemory::NewClear(sizeof(SExternalAppInfo));
	pExternalAppInfo->stStartupInfo.cb = sizeof(STARTUPINFO);
		
	if (!::CreateProcess(NULL, bufCommandLine, NULL, NULL, FALSE, NORMAL_PRIORITY_CLASS | CREATE_DEFAULT_ERROR_MODE, NULL, NULL, &pExternalAppInfo->stStartupInfo, &pExternalAppInfo->stProcessInfo))
 	{
 		UMemory::Dispose((TPtr)pExternalAppInfo);
 		return nil;
	}
	
	return (TExternalApp)pExternalAppInfo;
}

void UExternalApp::Dispose(TExternalApp inApp)
{
	if (inApp)
	{
		if (APP->stProcessInfo.hProcess)
		{
//			::TerminateProcess(APP->stProcessInfo.hProcess, 0);			
			::PostThreadMessage(APP->stProcessInfo.dwThreadId, WM_QUIT, 0, 0);

			::WaitForSingleObject(APP->stProcessInfo.hProcess, 5000); 	// wait max. 5 seconds for end of process
		}
		
		UMemory::Dispose((TPtr)inApp);
	}
}

bool UExternalApp::TryToClose(TExternalApp inApp)
{
	if (inApp && APP->stProcessInfo.dwThreadId)
		return ::EnumThreadWindows(APP->stProcessInfo.dwThreadId, _EnumThreadWndProc, WM_CLOSE);

	return false;
}

bool UExternalApp::TryToActivate(TExternalApp inApp)
{
	if (inApp && APP->stProcessInfo.dwThreadId)
		return ::EnumThreadWindows(APP->stProcessInfo.dwThreadId, _EnumThreadWndProc, WM_ACTIVATE);
	
	return false;
}

bool UExternalApp::Suspend(TExternalApp inApp)
{
	if (inApp)
	{
		if (APP->stProcessInfo.hThread && SuspendThread(APP->stProcessInfo.hThread) != max_Uint32)
			return true;
	}
	
	return false; 
}

bool UExternalApp::Resume(TExternalApp inApp)
{
	if (inApp)
	{
		if (APP->stProcessInfo.hThread && ResumeThread(APP->stProcessInfo.hThread) != max_Uint32)
			return true;
	}
	
	return false; 
}

bool UExternalApp::IsRegisteredAssociation(const Uint8 *inAssociation)
{
	Int8 bufAppPath[2048];
	Uint32 nPathSize = ::GetModuleFileName(NULL, bufAppPath, sizeof(bufAppPath));
	// with %1 the program will receive short filename 8.3 and truncate path (max. 8 char for a folder)
	// with %l or %L the program will receive entire filename and path
	nPathSize += UMemory::Copy(bufAppPath + nPathSize, " \%l", 4);	
	return _IsCreatedFileExtension(inAssociation, bufAppPath);
}

bool UExternalApp::RegisterAssociation(const Uint8 *inAssociation, const Uint8 *inTitle)
{
	Int8 bufAppPath[2048];

	Uint32 nPathSize = ::GetModuleFileName(NULL, bufAppPath, sizeof(bufAppPath));
	_CreateFileDefaultIconApp(inAssociation, bufAppPath);
	// with %1 the program will receive short filename 8.3 and truncate path (max. 8 char for a folder)
	// with %l or %L the program will receive entire filename and path
	nPathSize += UMemory::Copy(bufAppPath + nPathSize, " \%l", 4);
	
	return _CreateFileExtensionApp(inAssociation, bufAppPath, inTitle);
}

/*
Uint32 UExternalApp::SearchAssociation(const void *inFilePath, Uint32 inPathSize, void *outAppPath, Uint32 inMaxPathSize)
{
	if (!inFilePath || !inPathSize || inPathSize > 4095 || !outAppPath || !inMaxPathSize)
		return 0;
			
	Uint16 nPathCount = 0;
	Uint8 *pPathEnd = (Uint8 *)inFilePath + inPathSize;
	
	while (*(pPathEnd - nPathCount) != '\\' && nPathCount < inPathSize && nPathCount < 255)
		nPathCount++;
	
	if (*(pPathEnd - nPathCount) != '\\')
		return 0;
	
	Int8 bufFileName[256];
	bufFileName[UMemory::Copy(bufFileName, pPathEnd - nPathCount + 1, nPathCount - 1)] = 0;

	Int8 bufFilePath[4096];
	bufFilePath[UMemory::Copy(bufFilePath, inFilePath, inPathSize - nPathCount)] = 0;

	Int8 bufAppPath[4096];
	if ((Uint32)FindExecutable(bufFileName, bufFilePath, bufAppPath) <= 32)
		return 0;
 
	Uint32 nPathSize = strlen(bufAppPath);

	if (nPathSize > inMaxPathSize)
		nPathSize = inMaxPathSize;
		
	return UMemory::Copy(outAppPath, bufAppPath, nPathSize);
}*/

Uint32 UExternalApp::SearchAssociation(const Uint8 *inFileName, Uint32 inTypeCode, void *outAppPath, Uint32 inMaxPathSize)
{
	if (!inFileName || !inFileName[0] || !inTypeCode || !outAppPath || !inMaxPathSize)
		return 0;

	Uint16 nExtCount = inFileName[0];
	while (*(inFileName + nExtCount) != '.' && nExtCount > 1)
		nExtCount--;
	
	Uint8 bufExtension[8]; bufExtension[0] = 0;
	if (*(inFileName + nExtCount) == '.')
	{
		bufExtension[0] = UMemory::Copy(bufExtension + 1, inFileName + nExtCount + 1, inFileName[0] - nExtCount > sizeof(bufExtension) - 1 ? sizeof(bufExtension) - 1 : inFileName[0] - nExtCount);
		UText::MakeLowercase(bufExtension + 1, bufExtension[0]);
	}

	if (!bufExtension[0])
		switch (FB(inTypeCode))
		{
			case 'MooV':
				bufExtension[0] = UMemory::Copy(bufExtension + 1, "mov", 3);
				break;
			case 'MPEG':
				bufExtension[0] = UMemory::Copy(bufExtension + 1, "mpg", 3);
				break;
			case 'AVI ':
			case 'VfW ':
				bufExtension[0] = UMemory::Copy(bufExtension + 1, "avi", 3);
				break;
			case 'MP3 ':
				bufExtension[0] = UMemory::Copy(bufExtension + 1, "mp3", 3);
				break;
			case 'RAE ':
				bufExtension[0] = UMemory::Copy(bufExtension + 1, "ra", 2);
				break;
	
			default:
				return 0;
		};
	
	return _SearchFileExtensionApp(bufExtension, (Int8 *)outAppPath, inMaxPathSize);
}

TExternalApp UExternalApp::LaunchAssociation(const Uint8 *inFileName, Uint32 inTypeCode, const void *inParam, Uint32 inParamSize)
{
	Uint8 bufAppPath[4096];
	Uint32 nPathSize = SearchAssociation(inFileName, inTypeCode, bufAppPath, sizeof(bufAppPath));
	
	if (!nPathSize)
		return nil;
		
	return UExternalApp::New(bufAppPath, nPathSize, inParam, inParamSize);
}

extern Uint32 _GetWinPath(TFSRefObj* inRef, void *outAppPath, Uint32 inMaxPathSize);

TExternalApp UExternalApp::LaunchApplication(TFSRefObj* inApp, const void *inParam, Uint32 inParamSize)
{
	Uint8 bufAppPath[4096];
	Uint32 nPathSize = _GetWinPath(inApp, bufAppPath, sizeof(bufAppPath));

	if (!nPathSize)
		return nil;

	return UExternalApp::New(bufAppPath, nPathSize, inParam, inParamSize);
}

bool UExternalApp::SearchRunningApp(const Int8 *inAppPath, Uint32 inMsg, const Int8 *inMessage)
{
	if (!inAppPath || !strlen(inAppPath))
		return false;

	OSVERSIONINFO stVersionInfo;
	stVersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

	if (!::GetVersionEx(&stVersionInfo))
		return false;

#if !PLATFORM_WIN32_ALL
	if (stVersionInfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
	{
//		return _SearchProcess_WIN(inAppPath, inMsg, inMessage);
	}
	else if (stVersionInfo.dwPlatformId == VER_PLATFORM_WIN32_NT)
	{
		return _SearchProcess_NT(inAppPath, inMsg, inMessage);
	}
#else
	#pragma unused(inMsg, inMessage)
#endif

	return false;
}

bool UExternalApp::ReadSystemRegistry(const char* key, const char* value, char* buffer, Uint32& bufferSize)
{
	HKEY hRootKey = nil;
	if (0 == UMemory::Compare(key, "HKCR\\", 5))
		hRootKey = HKEY_CLASSES_ROOT;
	else if (0 == UMemory::Compare(key, "HKCU\\", 5))
		hRootKey = HKEY_CURRENT_USER;
	
	HKEY hKey = nil;
	if (ERROR_SUCCESS != ::RegOpenKey(hRootKey,key+5,&hKey))
		return false;

	bool ok = ( ERROR_SUCCESS == ::RegQueryValueExA(hKey,value,NULL,NULL,
					(unsigned char*)buffer,&bufferSize) );

	::RegCloseKey(hKey);
	return ok;
}


#pragma mark -

BOOL CALLBACK _EnumThreadWndProc(HWND inHwnd, LPARAM inParam)
{
	bool bEnumNextWnd = false;

	switch (inParam)
	{
		case WM_ACTIVATE:
			::BringWindowToTop(inHwnd);
			break;
			
		case WM_CLOSE:
			::SendNotifyMessage(inHwnd, inParam, 0, 0);
			bEnumNextWnd = true;
			break;			
	};
		
	return bEnumNextWnd;
}


#pragma mark -

// Verify if is created a association between a file extension and an application
static bool _IsCreatedFileExtension(const Uint8 *inFileExtension)
{
	HKEY hOpenKey;
	Uint32 dwType;
	Uint32 dwSize;
	Int8 val[256];
	Int8 buf[_MAX_PATH];
    
    buf[UText::Format(buf, sizeof(buf) - 1, ".%#s", inFileExtension)] = 0;
	if (RegOpenKeyEx(HKEY_CLASSES_ROOT, buf, NULL, KEY_ALL_ACCESS, &hOpenKey) != ERROR_SUCCESS)
		return false;

	dwSize = sizeof(val) - 1;
	if (RegQueryValueEx(hOpenKey, NULL, NULL, &dwType, (LPBYTE)(val + 1), &dwSize) != ERROR_SUCCESS)
	{
		RegCloseKey(hOpenKey);
		return false;
	}

	RegCloseKey(hOpenKey);

	if (dwType != REG_SZ || dwSize == 0)
		return false;
   	
   	val[0] = dwSize - 1;
    buf[UText::Format(buf, sizeof(buf) - 1, "%#s\\shell\\open\\command", val)] = 0;
	if (RegOpenKeyEx(HKEY_CLASSES_ROOT, buf, NULL, KEY_ALL_ACCESS, &hOpenKey) != ERROR_SUCCESS)
		return false;

	if (RegQueryValueEx(hOpenKey, NULL, NULL, NULL, NULL, &dwSize) != ERROR_SUCCESS)
	{
		RegCloseKey(hOpenKey);
		return false;
	}

	RegCloseKey(hOpenKey);

	if (dwSize == 0)
		return false;

	return true;
}

// Verify if is created a association between a file extension and an specific application
static bool _IsCreatedFileExtension(const Uint8 *inFileExtension, const Int8 *inApplicationPath)
{
	HKEY hOpenKey;
	Uint32 dwType;
	Uint32 dwSize;
	Int8 val[256];
	Int8 buf[_MAX_PATH];
    
    buf[UText::Format(buf, sizeof(buf) - 1, ".%#s", inFileExtension)] = 0;
	if (RegOpenKeyEx(HKEY_CLASSES_ROOT, buf, NULL, KEY_ALL_ACCESS, &hOpenKey) != ERROR_SUCCESS)
		return false;

	dwSize = sizeof(val) - 1;
	if (RegQueryValueEx(hOpenKey, NULL, NULL, &dwType, (LPBYTE)(val + 1), &dwSize) != ERROR_SUCCESS)
	{
		RegCloseKey(hOpenKey);
		return false;
	}

	RegCloseKey(hOpenKey);

	if (dwType != REG_SZ || dwSize == 0)
		return false;

	val[0] = dwSize - 1;
    buf[UText::Format(buf, sizeof(buf) - 1, "%#s\\shell\\open\\command", val)] = 0;
	if (RegOpenKeyEx(HKEY_CLASSES_ROOT, buf, NULL, KEY_ALL_ACCESS, &hOpenKey) != ERROR_SUCCESS)
		return false;

    dwSize = sizeof(buf);
	if (RegQueryValueEx(hOpenKey, NULL, NULL, NULL, (LPBYTE)buf, &dwSize) != ERROR_SUCCESS)
	{
		RegCloseKey(hOpenKey);
		return false;
	}

	RegCloseKey(hOpenKey);

	if (UMemory::Compare(buf, strlen(buf), inApplicationPath, strlen(inApplicationPath)))
		return false;

	return true;
}

// Create a association between a file extension and an application
static bool _CreateFileExtensionApp(const Uint8 *inFileExtension, const Int8 *inApplicationPath, const Uint8 *inTitle)
{
	HKEY hNewKey;
	Uint32 dwResult; 
	Int8 buf[_MAX_PATH];
    
    buf[UText::Format(buf, sizeof(buf) - 1, ".%#s", inFileExtension)] = 0;
	if (RegCreateKeyEx(HKEY_CLASSES_ROOT, buf, NULL, NULL, 
						REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, 
						&hNewKey, &dwResult) != ERROR_SUCCESS)
		return false;
		
//	If dwResult == REG_CREATED_NEW_KEY		-> The key did not exist and was created
//	If dwResult == REG_OPENED_EXISTING_KEY	-> The key existed and was simply opened

    buf[UText::Format(buf, sizeof(buf) - 1, "%#sFile", inFileExtension)] = 0;
	if (RegSetValueEx(hNewKey, NULL, 0, REG_SZ, (BYTE*)buf, strlen(buf)) != ERROR_SUCCESS)
	{
		RegCloseKey(hNewKey);
		return false;
	}

   	RegCloseKey(hNewKey);

    if (inTitle && inTitle[0])
    {
    	buf[UText::Format(buf, sizeof(buf) - 1, "%#sFile", inFileExtension)] = 0;
		if (RegCreateKeyEx(HKEY_CLASSES_ROOT, buf, NULL, NULL, 
							REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, 
							&hNewKey, &dwResult) != ERROR_SUCCESS)
			return false;
	
		buf[UMemory::Copy(buf, inTitle + 1, inTitle[0])] = 0;
		if (RegSetValueEx(hNewKey, NULL, 0, REG_SZ, (BYTE*)buf, strlen(buf)) != ERROR_SUCCESS)
		{
			RegCloseKey(hNewKey);
			return false;
		}

		RegCloseKey(hNewKey);
	}

    buf[UText::Format(buf, sizeof(buf) - 1, "%#sFile\\shell\\open\\command", inFileExtension)] = 0;
	if (RegCreateKeyEx(HKEY_CLASSES_ROOT, buf, NULL, NULL, 
						REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, 
						&hNewKey, &dwResult) != ERROR_SUCCESS)
		return false;
	
	UMemory::Copy(buf, inApplicationPath, strlen(inApplicationPath) + 1);
	if (RegSetValueEx(hNewKey, NULL, 0, REG_SZ, (BYTE*)buf, strlen(buf)) != ERROR_SUCCESS)
	{
		RegCloseKey(hNewKey);
		return false;
	}

	RegCloseKey(hNewKey);

	return true;
}

// Create a association between a file extension and an icon
static bool _CreateFileDefaultIconApp(const Uint8 *inFileExtension, const Int8 *inApplicationPath)
{
	HKEY hNewKey;
	Uint32 dwResult; 
	Int8 buf[_MAX_PATH];
    
    buf[UText::Format(buf, sizeof(buf) - 1, ".%#s", inFileExtension)] = 0;
	if (RegCreateKeyEx(HKEY_CLASSES_ROOT, buf, NULL, NULL, 
						REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, 
						&hNewKey, &dwResult) != ERROR_SUCCESS)
		return false;
		
//	If dwResult == REG_CREATED_NEW_KEY		-> The key did not exist and was created
//	If dwResult == REG_OPENED_EXISTING_KEY	-> The key existed and was simply opened

    buf[UText::Format(buf, sizeof(buf) - 1, "%#sFile", inFileExtension)] = 0;
	if (RegSetValueEx(hNewKey, NULL, 0, REG_SZ, (BYTE*)buf, strlen(buf)) != ERROR_SUCCESS)
	{
		RegCloseKey(hNewKey);
		return false;
	}

   	RegCloseKey(hNewKey);

    buf[UText::Format(buf, sizeof(buf) - 1, "%#sFile\\DefaultIcon", inFileExtension)] = 0;
	if (RegCreateKeyEx(HKEY_CLASSES_ROOT, buf, NULL, NULL, 
						REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, 
						&hNewKey, &dwResult) != ERROR_SUCCESS)
		return false;
	
	UMemory::Copy(buf, inApplicationPath, strlen(inApplicationPath));
	UMemory::Copy(buf + strlen(inApplicationPath), ",0", 3);
	if (RegSetValueEx(hNewKey, NULL, 0, REG_SZ, (BYTE*)buf, strlen(buf)) != ERROR_SUCCESS)
	{
		RegCloseKey(hNewKey);
		return false;
	}

	RegCloseKey(hNewKey);

	return true;
}

// Return application path associated with a file extension
static Uint32 _SearchFileExtensionApp(const Uint8 *inFileExtension, Int8 *outApplicationPath, Uint32 inMaxPathSize)
{
	HKEY hOpenKey;
	Uint32 dwType;
	Uint32 dwSize;
	Int8 val[256];
	Int8 buf[_MAX_PATH];
    
    buf[UText::Format(buf, sizeof(buf) - 1, ".%#s", inFileExtension)] = 0;
	if (RegOpenKeyEx(HKEY_CLASSES_ROOT, buf, NULL, KEY_ALL_ACCESS, &hOpenKey) != ERROR_SUCCESS)
		return false;

	dwSize = sizeof(val) - 1;
	if (RegQueryValueEx(hOpenKey, NULL, NULL, &dwType, (LPBYTE)(val + 1), &dwSize) != ERROR_SUCCESS)
	{
		RegCloseKey(hOpenKey);
		return false;
	}

	RegCloseKey(hOpenKey);

	if (dwType != REG_SZ || dwSize == 0)
		return false;

    val[0] = dwSize - 1;
    buf[UText::Format(buf, sizeof(buf) - 1, "%#s\\shell\\open\\command", val)] = 0;
	if (RegOpenKeyEx(HKEY_CLASSES_ROOT, buf, NULL, KEY_ALL_ACCESS, &hOpenKey) != ERROR_SUCCESS)
		return false;

    dwSize = sizeof(buf);
	if (RegQueryValueEx(hOpenKey, NULL, NULL, NULL, (LPBYTE)buf, &dwSize) != ERROR_SUCCESS)
	{
		RegCloseKey(hOpenKey);
		return false;
	}

	RegCloseKey(hOpenKey);
	
	outApplicationPath[UMemory::Copy(outApplicationPath, buf, strlen(buf) > inMaxPathSize - 1 ? inMaxPathSize - 1 : strlen(buf))] = 0;
	return strlen(outApplicationPath);
}


#pragma mark -
#if !PLATFORM_WIN32_ALL

static bool _CompareProcess_WIN(PROCESSENTRY32 *inProcessInfo, const Int8 *inProcessPath)
{
	if (inProcessInfo->th32ProcessID == ::GetCurrentProcessId())
		return false;
		
	if (!UText::CompareInsensitive(inProcessInfo->szExeFile, strlen(inProcessInfo->szExeFile), inProcessPath, strlen(inProcessPath)))
		return true;
		
	return false;
}

static bool _SearchThread_WIN(HANDLE inSnapshot, Uint32 inProcessID, Uint32 inMsg, const Int8 *inMessage)
{
	THREADENTRY32 stThreadInfo;
	ClearStruct(stThreadInfo);
	stThreadInfo.dwSize = sizeof(THREADENTRY32);

	if (!::Thread32First(inSnapshot, &stThreadInfo))
		return false;
	
	do
	{
		if (stThreadInfo.th32OwnerProcessID == inProcessID)
		{
			if (inMessage && !_WriteTempMessageInRegistry(inMessage))
				return false;
			
			::PostThreadMessage(stThreadInfo.th32ThreadID, inMsg, 0, 0);
			return true;
		}
		
	} while (::Thread32Next(inSnapshot, &stThreadInfo));
		
	return false;
}

static bool _SearchProcess_WIN(const Int8 *inProcessPath, Uint32 inMsg, const Int8 *inMessage)
{
	HANDLE hSnapshot = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS | TH32CS_SNAPTHREAD, 0);

	PROCESSENTRY32 stProcessInfo;
	ClearStruct(stProcessInfo);
	stProcessInfo.dwSize = sizeof(PROCESSENTRY32);

	if (!::Process32First(hSnapshot, &stProcessInfo))
	{
		::CloseHandle(hSnapshot);
		return false;
	}
		
	do
	{
		if (_CompareProcess_WIN(&stProcessInfo, inProcessPath))
		{
			if (!inMsg || _SearchThread_WIN(hSnapshot, stProcessInfo.th32ProcessID, inMsg, inMessage))
			{
				::CloseHandle(hSnapshot);
				return true;
			}
			
			::CloseHandle(hSnapshot);
			return false;
		}
				
	} while (::Process32Next(hSnapshot, &stProcessInfo));
	
	::CloseHandle(hSnapshot);
	return false;
}


#pragma mark -

static LPTSTR _GetPerfTitleSz_NT(LPTSTR *outTitleSz[], DWORD *outTitleLastIndex)
{
    HKEY hKey1;
    DWORD dwRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE, "software\\microsoft\\windows nt\\currentversion\\perflib", 0, KEY_READ, &hKey1);

    if (dwRet != ERROR_SUCCESS)
        return NULL;

    DWORD dwType;
	DWORD dwDataSize = sizeof(DWORD);
    dwRet = RegQueryValueEx (hKey1, "Last Counter", 0, &dwType, (LPBYTE)outTitleLastIndex, &dwDataSize);
    
	if (dwRet != ERROR_SUCCESS)
        return NULL;

    DWORD   dwTemp;
	dwRet = RegQueryValueEx (hKey1, "Version", 0, &dwType, (LPBYTE)&dwTemp, &dwDataSize);

    bool bNT10;
	if (dwRet != ERROR_SUCCESS)
        bNT10 = true;
    else
        bNT10 = false;

    HKEY hKey2;
    LPTSTR  szCounterValueName;

	if (bNT10)
    {
        szCounterValueName = "Counters";
        dwRet = RegOpenKeyEx (HKEY_LOCAL_MACHINE, "software\\microsoft\\windows nt\\currentversion\\perflib\\009", 0, KEY_READ, &hKey2);
        
		if(dwRet != ERROR_SUCCESS)
            return NULL;
    }
    else
    {
        szCounterValueName = "Counter 009";
        hKey2 = HKEY_PERFORMANCE_DATA;
    }

    dwRet = RegQueryValueEx(hKey2, szCounterValueName, 0, &dwType, 0, &dwDataSize);
    
	if (dwRet != ERROR_SUCCESS)
        return NULL;

    LPTSTR pTitleBuffer = (LPTSTR)LocalAlloc(LMEM_FIXED, dwDataSize);
	if(!pTitleBuffer)
		return NULL;

    *outTitleSz = (LPTSTR *)LocalAlloc (LMEM_FIXED | LMEM_ZEROINIT, (*outTitleLastIndex+1) * sizeof (LPTSTR));
    if (!*outTitleSz)
    {
        LocalFree(pTitleBuffer);
		return NULL;
    }

    dwRet = RegQueryValueEx(hKey2, szCounterValueName, 0, &dwType, (BYTE *)pTitleBuffer, &dwDataSize);
    
	if (dwRet != ERROR_SUCCESS)
	{
        LocalFree(pTitleBuffer);
        LocalFree(*outTitleSz);

		return NULL;
	}

	DWORD dwLen;
    LPTSTR szTitle = pTitleBuffer;

    while ((dwLen = lstrlen (szTitle)) != 0)
    {
		DWORD dwIndex =	UText::TextToUInteger(szTitle, strlen(szTitle));

        szTitle = szTitle + dwLen +1;

        if(dwIndex <= *outTitleLastIndex)
            (*outTitleSz)[dwIndex] = szTitle;

        szTitle = szTitle + lstrlen(szTitle) +1;
    }
 
	RegCloseKey(hKey1);
    RegCloseKey(hKey2);

    return pTitleBuffer;
}

static DWORD _GetProcessThreadInfo_NT(LPTSTR inTitle[], DWORD inLastIndex, LPSTR inName)
{
    for (DWORD dwIndex = 0; dwIndex <= inLastIndex; dwIndex++)
        if (inTitle[dwIndex] && !lstrcmpi(inTitle[dwIndex], inName))
			return dwIndex;

    return 0;
}

static bool _GetProcessThreadIndex_NT(DWORD& outProcessIndex, DWORD& outProcessID, DWORD& outThreadIndex, DWORD& outThreadID)
{
	LPTSTR *ppTitle;
	DWORD dwLast;

	LPTSTR pTitleBuffer = _GetPerfTitleSz_NT(&ppTitle, &dwLast);
	
	if (!pTitleBuffer)
		return false;

	outProcessIndex = _GetProcessThreadInfo_NT(ppTitle, dwLast, "Process");
	outProcessID = _GetProcessThreadInfo_NT(ppTitle, dwLast, "ID Process");
    outThreadIndex = _GetProcessThreadInfo_NT(ppTitle, dwLast, "Thread");
	outThreadID = _GetProcessThreadInfo_NT(ppTitle, dwLast, "ID Thread");

	LocalFree(*ppTitle);
    LocalFree(pTitleBuffer);

	return true;
}

static PERF_DATA_BLOCK *_GetPerfData_NT(LPTSTR inObjectIndex, DWORD& outPerfDataSize)
{
    outPerfDataSize = 50*1024;	// start with 50K
    PERF_DATA_BLOCK *pPerfData = (PERF_DATA_BLOCK *)LocalAlloc(LMEM_FIXED, outPerfDataSize);
	
	if (!pPerfData)
		return NULL;
    
	DWORD dwRet;

	do
	{
       	DWORD dwType;
		DWORD dwDataSize = outPerfDataSize;
	    dwRet = RegQueryValueEx(HKEY_PERFORMANCE_DATA, inObjectIndex, NULL, &dwType, (BYTE *)pPerfData, &dwDataSize);

        if (dwRet == ERROR_MORE_DATA)
        {
            LocalFree(pPerfData);
            
			outPerfDataSize += 1024;
            pPerfData = (PERF_DATA_BLOCK *)LocalAlloc(LMEM_FIXED, outPerfDataSize);
        }

        if (!pPerfData)
			return NULL;

    } while (dwRet == ERROR_MORE_DATA);

    return pPerfData;
}

static PERF_OBJECT_TYPE *_FirstObject_NT(PERF_DATA_BLOCK *inPerfData)
{
    if (inPerfData)
        return ((PERF_OBJECT_TYPE*)((BYTE *)inPerfData + inPerfData->HeaderLength));
    
    return NULL;
}

static PERF_OBJECT_TYPE *_NextObject_NT(PERF_OBJECT_TYPE *inObject)
{
    if (inObject)
        return ((PERF_OBJECT_TYPE*)((BYTE *)inObject + inObject->TotalByteLength));
    
    return NULL;
}

static PERF_OBJECT_TYPE *_FindObject_NT(PERF_DATA_BLOCK *inPerfData, DWORD inTitleIndex)
{
	PERF_OBJECT_TYPE *pObject;
	DWORD i = 0;

    if ((pObject = _FirstObject_NT(inPerfData)) != NULL)
        while (i < inPerfData->NumObjectTypes)
        {
            if (pObject->ObjectNameTitleIndex == inTitleIndex)
                return pObject;

            pObject = _NextObject_NT(pObject);
            i++;
        }

    return NULL;
}

static PERF_INSTANCE_DEFINITION *_FirstInstance_NT(PERF_OBJECT_TYPE *inObject)
{
    if (inObject)
        return (PERF_INSTANCE_DEFINITION *)((BYTE*)inObject + inObject->DefinitionLength);
    
    return NULL;
}

static PERF_INSTANCE_DEFINITION *_NextInstance_NT(PERF_INSTANCE_DEFINITION *inInstance)
{
    if (inInstance)
    {
        PERF_COUNTER_BLOCK *pCounterBlock = (PERF_COUNTER_BLOCK *)((BYTE*)inInstance + inInstance->ByteLength);
        return (PERF_INSTANCE_DEFINITION *)((BYTE*)pCounterBlock + pCounterBlock->ByteLength);
    }

    return NULL;
}

static LPSTR _InstanceName_NT(PERF_INSTANCE_DEFINITION *inInstance)
{
    if (inInstance)
        return (LPSTR)((BYTE*)inInstance + inInstance->NameOffset);
    
    return NULL;
}

static PERF_COUNTER_DEFINITION *_FirstCounter_NT(PERF_OBJECT_TYPE *inProcessObject)
{
    if (inProcessObject)
        return (PERF_COUNTER_DEFINITION*)((BYTE*)inProcessObject + inProcessObject->HeaderLength);
    
    return NULL;
}

static PERF_COUNTER_DEFINITION *_NextCounter_NT(PERF_COUNTER_DEFINITION *pCounter)
{
    if (pCounter)
        return (PERF_COUNTER_DEFINITION*)((BYTE *)pCounter + pCounter->ByteLength);
    
    return NULL;
}

static PERF_COUNTER_DEFINITION *_FindCounter_NT(PERF_OBJECT_TYPE *inProcessObject, DWORD inTitleIndex)
{
	PERF_COUNTER_DEFINITION *pCounter;
	DWORD i = 0;

    if ((pCounter = _FirstCounter_NT(inProcessObject)) != NULL)
        while (i < inProcessObject->NumCounters)
        {
            if (pCounter->CounterNameTitleIndex == inTitleIndex)
                return pCounter;

            pCounter = _NextCounter_NT(pCounter);
            i++;
        }

    return NULL;
}

static DWORD *_GetProcessThreadID_NT(PERF_INSTANCE_DEFINITION *inInstance, PERF_COUNTER_DEFINITION *inCounter)
{
	PPERF_COUNTER_BLOCK pCounterBlock;

    if (inInstance && inCounter)
    {
        pCounterBlock = (PPERF_COUNTER_BLOCK)((BYTE *)inInstance + inInstance->ByteLength);
        return (DWORD*)((BYTE *)pCounterBlock + inCounter->CounterOffset);
    }
    
    return NULL;
}

static bool _SearchThread_NT(UINT inProcessIndex, PERF_OBJECT_TYPE *inThreadObject, DWORD inThreadID, Uint32 inMsg, const Int8 *inMessage)
{
    if (inThreadObject)
    {
		PERF_INSTANCE_DEFINITION *pInstance = _FirstInstance_NT(inThreadObject);

	    INT nInstanceIndex = 0;
		while (pInstance && nInstanceIndex < inThreadObject->NumInstances)
        {
			if (pInstance->ParentObjectInstance == inProcessIndex)
            {
				PERF_COUNTER_DEFINITION *pCounterThreadID = _FindCounter_NT(inThreadObject, inThreadID);
				DWORD dwThreadID = *_GetProcessThreadID_NT(pInstance, pCounterThreadID);
			
				if (inMessage && !_WriteTempMessageInRegistry(inMessage))
					return false;
				
				::PostThreadMessage(dwThreadID, inMsg, 0, 0);
				return true;
			}

			pInstance = _NextInstance_NT(pInstance);
            nInstanceIndex++;
        }
    }
	
	return false;
}

static bool _SearchProcess_NT(const Int8 *inProcessPath, Uint32 inMsg, const Int8 *inMessage)
{ 
	DWORD dwProcessIndex, dwProcessID, dwThreadIndex, dwThreadID;
	if (!_GetProcessThreadIndex_NT(dwProcessIndex, dwProcessID, dwThreadIndex, dwThreadID))
		return false;

	TCHAR szProcessThreadIndex[10];
	wsprintf(szProcessThreadIndex, "%ld %ld", dwProcessIndex, dwThreadIndex);

	DWORD nPerfDataSize;
	PERF_DATA_BLOCK *pPerfData = _GetPerfData_NT(szProcessThreadIndex, nPerfDataSize);
	
	if (!pPerfData)
		return false;
	
	PERF_OBJECT_TYPE *pProcessObject = _FindObject_NT(pPerfData, dwProcessIndex);
	PERF_OBJECT_TYPE *pThreadObject = _FindObject_NT(pPerfData, dwThreadIndex);

	PERF_INSTANCE_DEFINITION *pInstance = _FirstInstance_NT(pProcessObject);

	UINT nInstanceIndex = 0;
	while (pInstance && nInstanceIndex < pProcessObject->NumInstances)
	{
		PERF_COUNTER_DEFINITION *pCounterProcessID = _FindCounter_NT(pProcessObject, dwProcessID);
		DWORD dwProcID = *_GetProcessThreadID_NT(pInstance, pCounterProcessID);
				
		if (dwProcID != ::GetCurrentProcessId())
		{
			HANDLE hProcess = ::OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, dwProcID);

			if (hProcess)
			{
				HMODULE hModules[1024];
				Uint32 nBytesNeeded;
				if(::EnumProcessModules(hProcess, hModules, sizeof(hModules), &nBytesNeeded))
				{								
					Int8 bufProcessPath[2048];
					if (::GetModuleFileNameEx(hProcess, hModules[0], bufProcessPath, sizeof(bufProcessPath)) && !UText::CompareInsensitive(bufProcessPath, strlen(bufProcessPath), inProcessPath, strlen(inProcessPath)))
					{
						CloseHandle(hProcess);
						
						if (!inMsg || _SearchThread_NT(nInstanceIndex, pThreadObject, dwThreadID, inMsg, inMessage))
						{
							LocalFree(pPerfData);
							return true;
						}	
				
						LocalFree(pPerfData);
						return false;
					}
				}
			
				CloseHandle(hProcess);
			}
		}

		pInstance = _NextInstance_NT(pInstance);
		nInstanceIndex++;
	}

	LocalFree(pPerfData);
	return false;
}

#endif // PLATFORM_WIN32_ALL
#pragma mark -

bool _WriteTempMessageInRegistry(const Int8 *inMessage) 
{
	HKEY hNewKey;
	DWORD dwResult; 
    
	if (RegCreateKeyEx(HKEY_CLASSES_ROOT, "hotlineTemp", NULL, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hNewKey, &dwResult) != ERROR_SUCCESS)
		return false;

	if (RegSetValueEx(hNewKey, NULL, 0, REG_SZ, (BYTE*)inMessage, strlen(inMessage)) != ERROR_SUCCESS)
	{
		RegCloseKey(hNewKey);
		return false;
	}

	RegCloseKey(hNewKey);

	return true;
}

bool _ReadTempMessageFromRegistry(Int8 *outMessage, DWORD inMaxSize) 
{
	HKEY hOpenKey;
	DWORD dwSize = inMaxSize;
    
	if (RegOpenKeyEx(HKEY_CLASSES_ROOT, "hotlineTemp", NULL, KEY_ALL_ACCESS, &hOpenKey) != ERROR_SUCCESS)
		return false;

	if (RegQueryValueEx(hOpenKey, NULL, NULL, NULL, (LPBYTE)outMessage, &dwSize) != ERROR_SUCCESS)
	{
		RegCloseKey(hOpenKey);
		return false;
	}

	RegCloseKey(hOpenKey);
	RegDeleteKey(HKEY_CLASSES_ROOT, "hotlineTemp");

	return true;
}

#endif /* WIN32 */
