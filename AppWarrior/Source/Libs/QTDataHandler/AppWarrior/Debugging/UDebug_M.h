// =====================================================================
//  UDebug_M.h                           (C) Hotline Communications 1999
// =====================================================================
//

#ifndef _H_UDebug_M_
#define _H_UDebug_M_

#if PRAGMA_ONCE
	#pragma once
#endif

#include "MetroNubUtils.h"
#include <cstdarg>
#include <cstdio>


HL_Begin_Namespace_BigRedH

// ---------------------------------------------------------------------
//  Breakpoint                                  [public][static][inline]
// ---------------------------------------------------------------------
//

inline void UDebug::Breakpoint()
{
	if( true ){//AmIBeingMWDebugged() ){
		::Debugger();
	}
}

// ---------------------------------------------------------------------
//  Message                                     [public][static][inline]
// ---------------------------------------------------------------------
//

inline void UDebug::Message( const char inMsg[] , ... )
{
	if( false && AmIBeingMWDebugged() ){
		using namespace std;
		va_list arguments;
		va_start( arguments, inMsg );
		char buffer[1024];

		int len = vsprintf( buffer, inMsg, arguments );
		if( len >= 255 ){
			len = 255;
		}
		buffer[len] = 0;
		
		// convert to pascal string snd display
	#if TARGET_API_MAC_CARBON
		UInt8 msg[256];
		::CopyCStringToPascal(buffer, msg);
		::DebugStr( msg );
	#else
		::c2pstr( buffer );
		::DebugStr( (UInt8*)buffer );
	#endif
	}
}

HL_End_Namespace_BigRedH
#endif //_H_UDebug_M_
