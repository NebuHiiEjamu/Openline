/* (c)2003 Hotsprings Inc. Licensed under GPL - see LICENSE in HotlineSources diresctory */

#pragma once


/*
 * Constants
 */

enum {
	// messages
	msg_ServicePause				= 200,	// Requests the service to pause
	msg_ServiceContinue				= 201,	// Requests the paused service to resume
	msg_ServiceStop					= 202,	// Requests the service to stop
	msg_ServiceShutdown				= 203,	// Requests the service to perform cleanup tasks, because the system is shutting down

	// service options
	opt_ServiceAcceptPauseContinue	= 0x01,	// The service can be paused and continued. MessageHandler function will receive msg_ServicePause and msg_ServiceContinue
	opt_ServiceAccespStop			= 0x02,	// The service can be stopped. MessageHandler function will receive msg_ServiceStop
	opt_ServiceAcceptShutDown		= 0x04,	// The service is notified when system shutdown occurs. MessageHandler function will receive opt_ServiceAcceptShutDown
	
	// start options
	opt_ServiceAutoStart			= 1,	// The service will be started automatically by the service control manager during system startup
	opt_ServiceManualStart			= 2,	// The service will be started manual
	opt_ServiceDisabled				= 3		// The service will be disabled
};


class UService
{
	public:
		static void Init(Uint8 *inServiceName, Uint8 inServiceOptions);
	    
		// install, uninstall
		static bool IsInstalled();
    	static bool Install(Uint8 inStartOptions);
	    static bool Uninstall();
	    static bool IsService();
		
		// messaging
		static void SetMessageHandler(TMessageProc inProc, void *inContext = nil);
		static void PostMessage(Uint32 inMsg, const void *inData = nil, Uint32 inDataSize = 0, Int16 inPriority = priority_Normal);

		// start
		static bool Start();
		static bool Register();
	    		
	   	// status
	   	static void SetStatus(Uint32 inState, Uint32 inExitCode, Uint32 inCheckPoint, Uint32 inWaitHint);
	   	static void UpdateStatus();
	   	
	   	static void DisplayMessage(Uint8 *inMessage);
};
