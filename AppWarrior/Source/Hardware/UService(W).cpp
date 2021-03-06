/* (c)2003 Hotsprings Inc. Licensed under GPL - see LICENSE in HotlineSources diresctory */

#include "UService.h"

#define MB_SERVICE_NOTIFICATION          0x00200000L

struct SServiceInfo {
	Int8 szServiceName[256];
	SERVICE_STATUS_HANDLE hServiceStatus;
	SERVICE_STATUS stServiceStatus;
	TMessageProc msgProc;
	void *msgProcContext;
};

static SServiceInfo gServiceInfo;

static void WINAPI _ServiceMain(Uint32 inArgc, Int8 **inArgv);
static void WINAPI _ServiceMessageHandler(Uint32 inOpCode);


void UService::Init(Uint8 *inServiceName, Uint8 inServiceOptions)
{
	UMemory::Copy(gServiceInfo.szServiceName, inServiceName + 1, inServiceName[0]);
	gServiceInfo.szServiceName[inServiceName[0]] = 0;	

    // set up the initial service status 
    gServiceInfo.hServiceStatus = nil;
    gServiceInfo.stServiceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    gServiceInfo.stServiceStatus.dwCurrentState = SERVICE_STOPPED;
    gServiceInfo.stServiceStatus.dwControlsAccepted = 0;
    
    if (inServiceOptions & opt_ServiceAcceptPauseContinue)
    	gServiceInfo.stServiceStatus.dwControlsAccepted |= SERVICE_ACCEPT_PAUSE_CONTINUE;

    if (inServiceOptions & opt_ServiceAccespStop)
    	gServiceInfo.stServiceStatus.dwControlsAccepted |= SERVICE_ACCEPT_STOP;
    	
    if (inServiceOptions & opt_ServiceAcceptShutDown)
    	gServiceInfo.stServiceStatus.dwControlsAccepted |= SERVICE_ACCEPT_SHUTDOWN;

    gServiceInfo.stServiceStatus.dwWin32ExitCode = S_OK;
    gServiceInfo.stServiceStatus.dwServiceSpecificExitCode = 0;
    gServiceInfo.stServiceStatus.dwCheckPoint = 0;
    gServiceInfo.stServiceStatus.dwWaitHint = 0;
    
    gServiceInfo.msgProc = nil;
    gServiceInfo.msgProcContext = nil;
}

bool UService::IsInstalled()
{
    bool bResult = false;
  
    SC_HANDLE hSCM = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

    if (hSCM != NULL)
	{
        SC_HANDLE hService = ::OpenService(hSCM, gServiceInfo.szServiceName, SERVICE_QUERY_CONFIG);
        
        if (hService != NULL)
		{
            bResult = true;
            ::CloseServiceHandle(hService);
        }
        
        ::CloseServiceHandle(hSCM);
    }
    
    return bResult;
}

bool UService::Install(Uint8 inStartOptions)
{
	if (IsInstalled())
	{
		::MessageBox(NULL, "This service is already installed on your computer.", gServiceInfo.szServiceName, MB_SERVICE_NOTIFICATION | MB_ICONEXCLAMATION);
		return true;
	}

   	Uint32 nStartType;
   	if (inStartOptions == opt_ServiceAutoStart)
   		nStartType = SERVICE_AUTO_START;
   	else if (inStartOptions == opt_ServiceManualStart)
   		nStartType = SERVICE_DEMAND_START;
   	else if (inStartOptions == opt_ServiceDisabled)
   		nStartType = SERVICE_DISABLED;
   	else
   		return false;

	bool bSuccess = false;
    SC_HANDLE hSCM = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    
    if (hSCM)
    {
        // get the executable file path
    	Int8 szFilePath[_MAX_PATH];
	    ::GetModuleFileName(NULL, szFilePath, _MAX_PATH);

    	// create service
    	SC_HANDLE hService = ::CreateService(hSCM, gServiceInfo.szServiceName, gServiceInfo.szServiceName, 
    										 SERVICE_ALL_ACCESS, SERVICE_WIN32_OWN_PROCESS | SERVICE_INTERACTIVE_PROCESS,
    										 nStartType, SERVICE_ERROR_NORMAL, szFilePath, NULL, NULL, NULL, NULL, NULL);									 
	    
	    if (hService)
	    {
	        ::CloseServiceHandle(hService);
    		bSuccess = true;
	    }
	 
	   	::CloseServiceHandle(hSCM);
    }

	if (bSuccess)
	{
		::MessageBox(NULL, "Success installing Service. Please restart the computer.", gServiceInfo.szServiceName, MB_SERVICE_NOTIFICATION | MB_ICONINFORMATION);
		return true;
	}
	
	::MessageBox(NULL, "Error installing Service.", gServiceInfo.szServiceName, MB_SERVICE_NOTIFICATION | MB_ICONSTOP);
	return false;
}

bool UService::Uninstall()
{
	if (!IsInstalled())
	{
		::MessageBox(NULL, "This service is not installed on your computer.", gServiceInfo.szServiceName, MB_SERVICE_NOTIFICATION | MB_ICONEXCLAMATION);
		return true;
	}

	bool bSuccess = false;
	SC_HANDLE hSCM = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

	if (hSCM)
	{
		SC_HANDLE hService = ::OpenService(hSCM, gServiceInfo.szServiceName, SERVICE_STOP | DELETE);

		if (hService)
		{
			SERVICE_STATUS status;
			::ControlService(hService, SERVICE_CONTROL_STOP, &status);

			bSuccess = ::DeleteService(hService);
			::CloseServiceHandle(hService);
		}
		
		::CloseServiceHandle(hSCM);
	}

	if (bSuccess)
	{
		::MessageBox(NULL, "Success uninstalling Service.", gServiceInfo.szServiceName, MB_SERVICE_NOTIFICATION | MB_ICONINFORMATION);
		return true;
	}

	::MessageBox(NULL, "Error uninstalling Service.", gServiceInfo.szServiceName, MB_SERVICE_NOTIFICATION | MB_ICONSTOP);
	return false;
}

// return true if the application runs like a service
bool UService::IsService()
{
	if (gServiceInfo.hServiceStatus)
		return true;
		
	return false;
}

void UService::SetMessageHandler(TMessageProc inProc, void *inContext)
{
	gServiceInfo.msgProc = inProc;
	gServiceInfo.msgProcContext = inContext;
}

void UService::PostMessage(Uint32 inMsg, const void *inData, Uint32 inDataSize, Int16 inPriority)
{
	if (gServiceInfo.msgProc)
		UApplication::PostMessage(inMsg, inData, inDataSize, inPriority, gServiceInfo.msgProc, gServiceInfo.msgProcContext, nil);
}

bool UService::Start()
{
    SERVICE_TABLE_ENTRY stTableEntry[] = {{ gServiceInfo.szServiceName, _ServiceMain }, { NULL, NULL }};
    
    if (!::StartServiceCtrlDispatcher(stTableEntry))
    {
    	::MessageBox(NULL, "Error starting Service.", gServiceInfo.szServiceName, MB_SERVICE_NOTIFICATION | MB_ICONSTOP);
    	return false;
    }
    	
    return true;
}

bool UService::Register()
{
    // register the control request handler
    gServiceInfo.stServiceStatus.dwCurrentState = SERVICE_START_PENDING;
    gServiceInfo.hServiceStatus = ::RegisterServiceCtrlHandler(gServiceInfo.szServiceName, _ServiceMessageHandler);
    
    if (gServiceInfo.hServiceStatus)
        return true;
         
    return false;
}

void UService::SetStatus(Uint32 inState, Uint32 inExitCode, Uint32 inCheckPoint, Uint32 inWaitHint)
{
	gServiceInfo.stServiceStatus.dwCurrentState = inState;
    gServiceInfo.stServiceStatus.dwServiceSpecificExitCode = inExitCode;
    gServiceInfo.stServiceStatus.dwCheckPoint = inCheckPoint;
    gServiceInfo.stServiceStatus.dwWaitHint = inWaitHint * 1000;

	::SetServiceStatus(gServiceInfo.hServiceStatus, &gServiceInfo.stServiceStatus);
}

void UService::UpdateStatus()
{
	::SetServiceStatus(gServiceInfo.hServiceStatus, &gServiceInfo.stServiceStatus);
}

void UService::DisplayMessage(Uint8 *inMessage)
{
	if (inMessage && inMessage[0])
	{
		Int8 szMessage[256];
		UMemory::Copy(szMessage, inMessage + 1, inMessage[0]);
		szMessage[inMessage[0]] = 0;
	
		::MessageBox(NULL, szMessage, gServiceInfo.szServiceName, MB_SERVICE_NOTIFICATION | MB_ICONEXCLAMATION);
	}
}

#pragma mark -

void main();

void WINAPI _ServiceMain(Uint32 /*inArgc*/, Int8 **/*inArgv*/)
{
	if (!UService::Register())
	{
		UService::SetStatus(SERVICE_STOPPED, S_OK, 0, 0);
		return;
	}

	UService::SetStatus(SERVICE_RUNNING, S_OK, 0, 0);
  
	try {
		main();
	} catch(...) {}

    UService::SetStatus(SERVICE_STOPPED, S_OK, 0, 0);
}

void WINAPI _ServiceMessageHandler(Uint32 inOpCode)
{
	switch (inOpCode)
	{
		case SERVICE_CONTROL_PAUSE:
			UService::SetStatus(SERVICE_PAUSE_PENDING, S_OK, 1, 1);
			UService::PostMessage(msg_ServicePause);
			UService::SetStatus(SERVICE_PAUSED, S_OK, 0, 0);
			break;

		case SERVICE_CONTROL_CONTINUE:
			UService::SetStatus(SERVICE_CONTINUE_PENDING, S_OK, 1, 1);
			UService::PostMessage(msg_ServiceContinue);
			UService::SetStatus(SERVICE_RUNNING, S_OK, 0, 0);
			break;

		case SERVICE_CONTROL_INTERROGATE:
			UService::UpdateStatus();
			break;

		case SERVICE_CONTROL_STOP:
		  	UService::SetStatus(SERVICE_STOP_PENDING, S_OK, 1, 3);
		  	UService::PostMessage(msg_ServiceStop);
			break;

		case SERVICE_CONTROL_SHUTDOWN:
		  	UService::SetStatus(SERVICE_STOP_PENDING, S_OK, 1, 3);
		  	UService::PostMessage(msg_ServiceShutdown);
			break;
			
		default:
			UService::UpdateStatus();
			break;
	}
}

