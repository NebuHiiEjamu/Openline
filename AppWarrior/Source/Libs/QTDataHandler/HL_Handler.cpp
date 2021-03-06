/* (c)2003 Hotsprings Inc. Licensed under GPL - see LICENSE in HotlineSources diresctory */
// A set of functions to respond to QuickTime's requests

#include "HL_Handler.h"
#include "CDataHandlerComponent.h"
#include <QuickTimeComponents.h>


// main dispatch function, out of HL namespace, to be exported
extern "C" pascal ComponentResult HL_Handler( ComponentParameters *params,
												Handle storage );

static HL_ProgressProc gProgressCallback = nil;
static HL_EventProc gEventCallback = nil;


Component gComponent = nil;
#if TARGET_OS_MAC
	#include "ComponentMacros.h"

	enum {
		FAT_VALUE_2(fat_pi_Dispatcher_ProcInfo, kPascalStackBased, ComponentResult, ComponentParameters *, Handle)
	};
	
	#if TARGET_API_MAC_CARBON
		static ComponentRoutineUPP sHL_HandlerUPP = NewComponentRoutineUPP(HL_Handler);
	#else
		FAT_RD_ALLOC(component_dispatcher_rd, fat_pi_Dispatcher_ProcInfo, HL_Handler);
	#endif
#endif


bool
HL_HandlerRegister()
{
	bool reged = true;
	
	ComponentDescription desc = { 'dhlr', 'url ', 'htln', 0, 0 };
	#if TARGET_OS_MAC
		#if TARGET_API_MAC_CARBON
			gComponent = ::RegisterComponent( &desc, sHL_HandlerUPP, false, nil, nil, nil );
		#else
			gComponent = ::RegisterComponent( &desc, &component_dispatcher_rd, false, nil, nil, nil );
		#endif
	#elif TARGET_OS_WIN32
		gComponent = ::RegisterComponent( &desc, HL_Handler, false, nil, nil, nil );
	#endif

	return (gComponent != nil);
}


void
HL_HandlerUnregister()
{
	if( gComponent != nil ){
		OSErr anErr = ::UnregisterComponent( gComponent );
		gComponent = nil;
	}
}


ComponentInstance
HL_HandlerOpen( Handle inDataRef )
{
	ComponentInstance c = nil;
	if( gComponent != nil ){
		c = ::OpenComponent( gComponent );
		if( c != nil ){
			ComponentResult result;
			result = ::DataHSetDataRef( (DataHandler)c, inDataRef );
		}
	}

	return c;
}


bool
HL_HandlerIsReading( Str255 inTempFileName )
{
	return HL_BigRedH CDataHandlerComponent::Instance().IsReading( inTempFileName );
}


void
HL_HandlerCancelReading( Str255 inTempFileName )
{
	return HL_BigRedH CDataHandlerComponent::Instance().CancelReading( inTempFileName );
}


void
HL_HandlerClose( ComponentInstance inInstance )
{
	if( inInstance != nil ){
		OSErr anErr = ::CloseComponent( inInstance );
	}
}


void
HL_HandlerStop( ComponentInstance inInstance )
{
	if (inInstance != nil)
	{
		::DataHFlushData(inInstance);		
	}
}


void
HL_SetProgressProc( HL_ProgressProc inProc )
{
	gProgressCallback = inProc;
}


void
HL_Progress( Str255 inFileName, unsigned long inFileSize,
			unsigned long inFileCompleted )
{
	if( gProgressCallback != nil ){
		(gProgressCallback)( inFileName, inFileSize,
							inFileCompleted );
	}
}


void
HL_SetEventProc( HL_EventProc inProc )
{
	gEventCallback = inProc;
}


void
HL_Event()
{
	if( gEventCallback != nil ){
		(gEventCallback)();
	}
}
